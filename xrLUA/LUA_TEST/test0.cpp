////////////////////////////////////////////////////////////////////////////
//	Module 		: test0.h
//	Created 	: 18.04.2004
//  Modified 	: 18.04.2004
//	Author		: Dmitriy Iassenev
//	Description : Adopt policy test
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#pragma warning(disable:4530)
#include "xrCore.h"
#pragma warning(default:4530)

// Lua
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lstate.h"
#pragma comment(lib,"x:/xrLUA.lib")

// Lua-bind
#pragma warning(disable:4995)
#pragma warning(disable:4530)
#pragma warning(disable:4244)
#include "luabind/luabind.hpp"
#include "luabind/adopt_policy.hpp"
#include "luabind/dependency_policy.hpp"
#include "luabind/return_reference_to_policy.hpp"
#include "luabind/out_value_policy.hpp"
#include "luabind/discard_result_policy.hpp"
#include "luabind/iterator_policy.hpp"
#pragma warning(default:4244)
#pragma warning(default:4995)
#pragma warning(default:4530)

#include "boost/shared_ptr.hpp"

__declspec(dllimport) LPSTR g_ca_stdout;

using namespace luabind;
using namespace std;

extern lua_State	*L;

struct A {
					A			()
	{
		printf		("A constructor is called!\n");
	}

	virtual			~A			()
	{
		printf		("A virtual destructor is called!\n");
	}

	virtual	void	a_virtual	()
	{
		printf		("A virtual function a() is called!\n");
	}
};

namespace luabind {
	A* get_pointer(boost::shared_ptr<A>& p) 
	{ 
		return p.get(); 
	}

	boost::shared_ptr<const A>* 
		get_const_holder(boost::shared_ptr<A>*)
	{
		return 0;
	}
}

struct A_wrapper : public A {
public:
	weak_ref		self;

public:
					A_wrapper	(weak_ref self_) :
						self(self_)
	{
	}

	virtual			~A_wrapper	()
	{
		printf		("A_wrapper virtual destructor is called!\n");
	}

	virtual	void	a_virtual	()
	{
		call_member<void>(self,"a");
	}

	static	void	a_static	(A *a)
	{
		a->A::a_virtual();
	}
};

struct B : public A {
	virtual			~B			()
	{
		printf		("B virtual destructor is called!\n");
	}

	virtual	void	a_virtual	()
	{
		printf		("B virtual function a() is called!\n");
		A::a_virtual();
	}

	virtual	void	b_virtual	()
	{
		printf		("B virtual function b() is called!\n");
	}
};

struct B_wrapper : public B {
public:
	weak_ref		self;

public:
					B_wrapper	(weak_ref self_) :
						self(self_)
	{
	}

	virtual			~B_wrapper	()
	{
		printf		("B_wrapper virtual destructor is called!\n");
	}

	virtual	void	a_virtual	()
	{
		call_member<void>(self,"a");
	}

	static	void	a_static	(B *b)
	{
		b->B::a_virtual();
	}

	virtual	void	b_virtual	()
	{
		call_member<void>(self,"b");
	}

	static	void	b_static	(B *b)
	{
		b->B::b_virtual();
	}
};

struct C : public B
{
	virtual			~C			()
	{
		printf		("C virtual destructor is called!\n");
	}

	virtual	void	a_virtual	()
	{
		printf		("C virtual function a() is called!\n");
//		A::a_virtual();
		B::a_virtual();
	}

	virtual	void	b_virtual	()
	{
		printf		("C virtual function b() is called!\n");
		B::b_virtual();
	}

	virtual	void	c_virtual	()
	{
		printf		("C virtual function c() is called!\n");
	}
};

struct C_wrapper : public C {
public:
	weak_ref		self;

public:
					C_wrapper	(weak_ref self_) :
						self(self_)
	{
	}

	virtual			~C_wrapper	()
	{
		printf		("C_wrapper virtual destructor is called!\n");
	}

	virtual	void	a_virtual	()
	{
		call_member<void>(self,"a");
	}

	static	void	a_static	(C *c)
	{
		c->C::a_virtual();
	}

	virtual	void	b_virtual	()
	{
		call_member<void>(self,"b");
	}

	static	void	b_static	(C *c)
	{
		c->C::b_virtual();
	}

	virtual	void	c_virtual	()
	{
		call_member<void>(self,"c");
	}

	static	void	c_static	(C *c)
	{
		c->C::c_virtual();
	}
};

//#define USE_BOOST_SHARED_PTR

struct M {
protected:
#ifndef USE_BOOST_SHARED_PTR
	typedef A*						pointer;
#else
	typedef boost::shared_ptr<A>	pointer;
#endif
	vector<pointer>					m_objects;

public:

	virtual			~M			()
	{
#ifndef USE_BOOST_SHARED_PTR
		vector<pointer>::iterator	I = m_objects.begin();
		vector<pointer>::iterator	E = m_objects.end();
		for ( ; I != E; ++I)
			delete	*I;
#endif
	}

			void	update		()
	{
		vector<pointer>::iterator	I = m_objects.begin();
		vector<pointer>::iterator	E = m_objects.end();
		for ( ; I != E; ++I) {
			(*I)->a_virtual		();
//			(*I)->b_virtual		();
//			(*I)->c_virtual		();
		}
	}

			void	add			(pointer c)
	{
		m_objects.push_back		(c);
	}
};

M *m;

M &getM()
{
	if (!m)
		m = new M();
	return		(*m);
}

lua_Debug	stack_levels[64];
int			curr_stack_level = 0;

#define LEVELS1	12	/* size of the first part of the stack */
#define LEVELS2	10	/* size of the second part of the stack */

int my_errorfb (lua_State *L) {
	int level = 1;  /* skip level 0 (it's this function) */
	int firstpart = 1;  /* still before eventual `...' */
	lua_Debug ar;
	if (lua_gettop(L) == 0)
		lua_pushliteral(L, "");
	else if (!lua_isstring(L, 1)) return 1;  /* no string message */
	else lua_pushliteral(L, "\n");
	lua_pushliteral(L, "stack traceback:");
	while (lua_getstack(L, level++, &ar)) {
		if (level > LEVELS1 && firstpart) {
			/* no more than `LEVELS2' more levels? */
			if (!lua_getstack(L, level+LEVELS2, &ar))
				level--;  /* keep going */
			else {
				lua_pushliteral(L, "\n\t...");  /* too many levels */
				while (lua_getstack(L, level+LEVELS2, &ar))  /* find last levels */
					level++;
			}
			firstpart = 0;
			continue;
		}
		lua_pushliteral(L, "\n\t");
		lua_getinfo(L, "Snl", &ar);
		lua_pushfstring(L, "%s:", ar.short_src);
		if (ar.currentline > 0)
			lua_pushfstring(L, "%d:", ar.currentline);
		switch (*ar.namewhat) {
	  case 'g':  /* global */ 
	  case 'l':  /* local */
	  case 'f':  /* field */
	  case 'm':  /* method */
		  lua_pushfstring(L, " in function `%s'", ar.name);
		  break;
	  default: {
		  if (*ar.what == 'm')  /* main? */
			  lua_pushfstring(L, " in main chunk");
		  else if (*ar.what == 'C' || *ar.what == 't')
			  lua_pushliteral(L, " ?");  /* C function or tail call */
		  else
			  lua_pushfstring(L, " in function <%s:%d>",
			  ar.short_src, ar.linedefined);
			   }
		}
		lua_concat(L, lua_gettop(L));
	}
	lua_concat(L, lua_gettop(L));
	return 1;
}

void lua_bind_error(lua_State *L)
{
	for (int i=0, n=lua_gettop(L); -i<n; --i)
		if (lua_isstring(L,i)) {
			LPCSTR	S = lua_tostring(L,i);
			printf	("error : %s\n",S);
		}
		else {
			for (int j=curr_stack_level - 1, k=0; j>0; --j, ++k) {
//			for (int j=0, k=0; j<curr_stack_level; ++j, ++k) {
				lua_Debug		l_tDebugInfo = stack_levels[j];
				if (!l_tDebugInfo.name)
					printf		("%2d : [C  ] C source code : %s\n",k,l_tDebugInfo.short_src);
				else
					if (!xr_strcmp(l_tDebugInfo.what,"C"))
						printf	("%2d : [C  ] C source code : %s\n",k,l_tDebugInfo.name);
					else
						printf	("%2d : [%s] %s(%d) : %s\n",k,l_tDebugInfo.what,l_tDebugInfo.short_src,l_tDebugInfo.currentline,l_tDebugInfo.name);
			}
			curr_stack_level = 0;
//			my_errorfb(L);
//			for (int j=0; ; --j)
//				if (lua_isstring(L,j)) {
//					LPCSTR	S = lua_tostring(L,j);
//					printf	("error : %s\n",S);
//				}
//				else
//					return;
		}
}

void hook(lua_State *L, lua_Debug *ar)
{
//	for (curr_stack_level=0; curr_stack_level<64; ++curr_stack_level) {
//		if (!lua_getstack(L,curr_stack_level,&stack_levels[curr_stack_level]))
//			break;
//		lua_getinfo	(L,"nSlu",&stack_levels[curr_stack_level]);
//	}
//	return;
	switch	(ar->event) {
		case LUA_HOOKCALL : {
			if (!lua_getstack(L,0,&stack_levels[curr_stack_level]))
				break;
			lua_getinfo	(L,"nSlu",&stack_levels[curr_stack_level]);
			if (curr_stack_level && lua_getstack(L,1,&stack_levels[curr_stack_level - 1]))
				lua_getinfo	(L,"nSlu",&stack_levels[curr_stack_level - 1]);
			++curr_stack_level;
			break;
		}
		case LUA_HOOKRET : {
			--curr_stack_level;
			break;
		}
		case LUA_HOOKLINE : {
			lua_getinfo	(L,"l",ar);
			stack_levels[curr_stack_level].currentline = ar->currentline;
			break;
		}
		case LUA_HOOKTAILRET : {
			break;
		}
		case LUA_HOOKCOUNT : {
			lua_getinfo	(L,"l",ar);
			stack_levels[curr_stack_level].currentline = ar->currentline;
			break;
		}
		default : NODEFAULT;
	}
//	for (curr_stack_level=0; curr_stack_level<64; ++curr_stack_level) {
//		if (!lua_getstack(L,curr_stack_level,&stack_levels[curr_stack_level]))
//			break;
//		lua_getinfo	(L,"nSlu",&stack_levels[curr_stack_level]);
//	}
}

void c_bug()
{
	luabind::object			lua_namespace	= luabind::get_globals(L)["thread_test"];
	luabind::functor<void>	lua_function	= luabind::object_cast<luabind::functor<void> >(lua_namespace["bug"]);
	lua_function			();
}

//void lua_debug(lua_State *L,)
int resume_thread(lua_State *L)
{
	return	(lua_resume((lua_State*)lua_touserdata(L,-1),0));
}

int print_stack(lua_State *L)
{
	lua_Debug l_tDebugInfo;
	for (int curr_stack_level=0; curr_stack_level<64; ++curr_stack_level) {
		if (!lua_getstack(L,curr_stack_level,&l_tDebugInfo))
			break;
		lua_getinfo	(L,"nSlu",&l_tDebugInfo);
		if (!l_tDebugInfo.name)
			printf		("%2d : [C  ] C source code : %s\n",curr_stack_level,l_tDebugInfo.short_src);
		else
			if (!xr_strcmp(l_tDebugInfo.what,"C"))
				printf	("%2d : [C  ] C source code : %s\n",curr_stack_level,l_tDebugInfo.name);
			else
				printf	("%2d : [%s] %s(%d) : %s\n",curr_stack_level,l_tDebugInfo.what,l_tDebugInfo.short_src,l_tDebugInfo.currentline,l_tDebugInfo.name);
	}
	return 0;
}

extern bool load_file_into_namespace(lua_State *L, LPCSTR S, LPCSTR N, bool bCall = true);

struct CEmptyClassEnum {};

void test0()
{
	string4096		SSS;
	strcpy			(SSS,"");
	g_ca_stdout		= SSS;

	L				= lua_open();

	if (!L)
		lua_error	(L);

	lua_setgcthreshold	(L,1024);

	luaopen_base	(L);
	luaopen_string	(L);
	luaopen_math	(L);
	luaopen_table	(L);
	luaopen_debug	(L);

	open			(L);

	luabind::set_error_callback(lua_bind_error);

	std::map<LPCSTR,int>	temp;

	temp.insert(std::make_pair("actor0",0));
	temp.insert(std::make_pair("actor1",1));
	temp.insert(std::make_pair("actor2",2));
	temp.insert(std::make_pair("actor3",3));
	temp.insert(std::make_pair("actor4",4));
	temp.insert(std::make_pair("actor5",5));
	temp.insert(std::make_pair("actor6",6));
	temp.insert(std::make_pair("actor7",7));
	temp.insert(std::make_pair("actor8",8));
	temp.insert(std::make_pair("actor9",9));

	class_<CEmptyClassEnum>		instance("clsid");

	std::map<LPCSTR,int>::const_iterator	I = temp.begin();
	std::map<LPCSTR,int>::const_iterator	E = temp.end();
	for ( ; I != E; ++I)
		instance.enum_("_clsid")[value((*I).first,(*I).second)];

	module(L)
	[
		instance,

#ifndef USE_BOOST_SHARED_PTR
		class_<A>("A")
#else
		class_<A, boost::shared_ptr<A> >("A")
#endif
			.def(constructor<>())
			.def("a",	&A::a_virtual),

//		class_<A,A_wrapper>("A")
//			.def(constructor<>())
//			.def("a",	&A_wrapper::a_static),
//
//		class_<B,B_wrapper,A>("B")
//			.def(constructor<>())
//			.def("a",	&B_wrapper::a_static)
//			.def("b",	&B_wrapper::b_static),
//
//		class_<C,C_wrapper,bases<B> >("C")
//			.def(constructor<>())
//			.def("a",	&C_wrapper::a_static)
//			.def("b",	&C_wrapper::b_static)
//			.def("c",	&C_wrapper::c_static),

		class_<M>("M")
#ifndef USE_BOOST_SHARED_PTR
			.def("add",	&M::add, adopt(_1)),
#else
			.def("add",	&M::add),
#endif

		def("c_bug", &c_bug)
//		def("lua_resume", &lua_resume),
//		def("lua_debug", &lua_debug)
	];

	load_file_into_namespace(L,"x:\\thread_test.script","thread_test",true);
	lua_sethook				(L,hook,LUA_HOOKCALL | LUA_HOOKRET | LUA_HOOKLINE | LUA_HOOKCOUNT, 1);
	
//	luabind::object			lua_namespace	= luabind::get_globals(L)["thread_test"];
//	luabind::functor<void>	lua_function	= luabind::object_cast<luabind::functor<void> >(lua_namespace["bug"]);
//	lua_function			();
//	lua_dostring			(L,"thread_test.bug()");

	printf			("top : %d\n",lua_gettop(L));
	for (int i=0; i<10000; ++i) {
		printf			("Starting thread %d\n",i);
		lua_State		*t = lua_newthread(L);
		lua_sethook		(t,hook,LUA_HOOKCALL | LUA_HOOKRET | LUA_HOOKLINE | LUA_HOOKCOUNT, 1);
		LPCSTR			s = "thread_test.main()";
		int				err = luaL_loadbuffer	(t,s,xr_strlen(s),"@_thread_main");
		if (err)
			lua_bind_error(t);
		do {
			err			= lua_resume(t,0);
//			lua_pushthread
//			err			= lua_cpcall(t,resume_thread,t);
			if (err) {
				lua_bind_error(t);
				curr_stack_level = 0;
				lua_settop(t,0);
				break;
			}
			Sleep		(1);
		}
		while (t->ci->state & CI_YIELD);
		
		bool			ok = false;
		for (int i=1, n=lua_gettop(L); i<=n; ++i)
			if ((lua_type(L,i) == LUA_TTHREAD) && (lua_tothread(L,i) == t)) {
				lua_remove(L,i);
				ok		= true;
				break;
			}
		VERIFY			(ok);
		printf			("top : %d\n",lua_gettop(L));
	}

//	lua_dofile		(L,"x:\\adopt_test.script");
//
//	if (xr_strlen(SSS)) {
//		printf		("\n%s\n",SSS);
//		strcpy		(SSS,"");
//		lua_close	(L);
//		return;
//	}
//
//	for (int i=0; i<20; Sleep(100), ++i) {
//		getM().update		();
////		lua_setgcthreshold	(L,0);
//	}

	delete			m;

	lua_close		(L);
}