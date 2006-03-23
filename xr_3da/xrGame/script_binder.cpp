////////////////////////////////////////////////////////////////////////////
//	Module 		: script_binder.cpp
//	Created 	: 26.03.2004
//  Modified 	: 26.03.2004
//	Author		: Dmitriy Iassenev
//	Description : Script objects binder
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "script_space.h"
#include "ai_space.h"
#include "script_engine.h"
#include "script_binder.h"
#include "xrServer_Objects_ALife.h"
#include "script_binder_object.h"
#include "script_game_object.h"
#include "gameobject.h"
#include "level.h"

#ifdef DEBUG
XRCORE_API	BOOL	g_bMEMO;
#endif

//#define DBG_DISABLE_SCRIPTS

CScriptBinder::CScriptBinder		()
{
	init					();
}

CScriptBinder::~CScriptBinder		()
{
	VERIFY					(!m_object);
}

void CScriptBinder::init			()
{
	m_object				= 0;
}

void CScriptBinder::clear			()
{
	try {
		xr_delete			(m_object);
	}
	catch(...) {
		m_object			= 0;
	}
	init					();
}

void CScriptBinder::reinit			()
{
#ifdef DEBUG
	u32									start = 0;
	if (g_bMEMO)
		start							= Memory.mem_usage();
#endif
	if (m_object) {
		try {
			m_object->reinit	();
		}
		catch(...) {
			clear			();
		}
	}
#ifdef DEBUG
	if (g_bMEMO) {
//		lua_gc				(ai().script_engine().lua(),LUA_GCCOLLECT,0);
//		lua_gc				(ai().script_engine().lua(),LUA_GCCOLLECT,0);
		Msg					("CScriptBinder::reinit() : %d",Memory.mem_usage() - start);
	}
#endif
}

void CScriptBinder::Load			(LPCSTR section)
{
}

void CScriptBinder::reload			(LPCSTR section)
{
#ifdef DEBUG
	u32									start = 0;
	if (g_bMEMO)
		start							= Memory.mem_usage();
#endif
#ifndef DBG_DISABLE_SCRIPTS
	VERIFY					(!m_object);
	if (!pSettings->line_exist(section,"script_binding"))
		return;
	
	luabind::functor<void>	lua_function;
	if (!ai().script_engine().functor(pSettings->r_string(section,"script_binding"),lua_function)) {
		ai().script_engine().script_log	(ScriptStorage::eLuaMessageTypeError,"function %s is not loaded!",pSettings->r_string(section,"script_binding"));
		return;
	}
	
	CGameObject				*game_object = smart_cast<CGameObject*>(this);

	try {
		lua_function		(game_object ? game_object->lua_game_object() : 0);
	}
	catch(...) {
		clear				();
		return;
	}

	if (m_object) {
		try {
			m_object->reload(section);
		}
		catch(...) {
			clear			();
		}
	}
#endif
#ifdef DEBUG
	if (g_bMEMO) {
//		lua_gc				(ai().script_engine().lua(),LUA_GCCOLLECT,0);
//		lua_gc				(ai().script_engine().lua(),LUA_GCCOLLECT,0);
		Msg					("CScriptBinder::reload() : %d",Memory.mem_usage() - start);
	}
#endif
}

BOOL CScriptBinder::net_Spawn		(CSE_Abstract* DC)
{
#ifdef DEBUG
	u32									start = 0;
	if (g_bMEMO)
		start							= Memory.mem_usage();
#endif
	CSE_Abstract			*abstract = (CSE_Abstract*)DC;
	CSE_ALifeObject			*object = smart_cast<CSE_ALifeObject*>(abstract);
	if (object && m_object) {
		try {
			return			((BOOL)m_object->net_Spawn(object));
		}
		catch(...) {
			clear			();
		}
	}

#ifdef DEBUG
	if (g_bMEMO) {
//		lua_gc				(ai().script_engine().lua(),LUA_GCCOLLECT,0);
//		lua_gc				(ai().script_engine().lua(),LUA_GCCOLLECT,0);
		Msg					("CScriptBinder::net_Spawn() : %d",Memory.mem_usage() - start);
	}
#endif

	return					(TRUE);
}

void CScriptBinder::net_Destroy		()
{
	if (m_object) {
		try {
			m_object->net_Destroy	();
		}
		catch(...) {
			clear			();
		}
	}
	xr_delete				(m_object);
}

void CScriptBinder::set_object		(CScriptBinderObject *object)
{
	VERIFY2					(!m_object,"Cannot bind to the object twice!");
#ifdef DEBUG
	Msg						("* Core object %s is binded with the script object",smart_cast<CGameObject*>(this) ? *smart_cast<CGameObject*>(this)->cName() : "");
#endif
	if(IsGameTypeSingle())
		m_object				= object;
}

void CScriptBinder::shedule_Update	(u32 time_delta)
{
	if (m_object) {
		try {
			m_object->shedule_Update	(time_delta);
		}
		catch(...) {
			clear			();
		}
	}
}

void CScriptBinder::save			(NET_Packet &output_packet)
{
	if (m_object) {
		try {
			m_object->save	(&output_packet);
		}
		catch(...) {
			clear			();
		}
	}
}

void CScriptBinder::load			(IReader &input_packet)
{
	if (m_object) {
		try {
			m_object->load	(&input_packet);
		}
		catch(...) {
			clear			();
		}
	}
}

BOOL CScriptBinder::net_SaveRelevant()
{
	if (m_object) {
		try {
			return			(m_object->net_SaveRelevant());
		}
		catch(...) {
			clear			();
		}
	}
	return							(FALSE);
}

void CScriptBinder::net_Relcase		(CObject *object)
{
	CGameObject						*game_object = smart_cast<CGameObject*>(object);
	if (m_object && game_object) {
		try {
			m_object->net_Relcase	(game_object->lua_game_object());
		}
		catch(...) {
			clear			();
		}
	}
}
