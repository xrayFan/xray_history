////////////////////////////////////////////////////////////////////////////
//	Module 		: xrSE_Factory.cpp
//	Created 	: 18.06.2004
//  Modified 	: 18.06.2004
//	Author		: Dmitriy Iassenev
//	Description : Precompiled header creatore
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "xrSE_Factory.h"
#include "ai_space.h"
#include "script_engine.h"

DLL_API CSE_Abstract *__cdecl create_object(LPCSTR section)
{
	return				(F_entity_Create(section));
}

DLL_API LPCSTR	__cdecl get_string()
{
	return				("Ok!");
}

#define _CLOSURE_THIS(A,C)	C.bind(this,&A)
#define _CLOSURE(A,B,C)		C.bind(B,&A)

struct CTestInterface0 : public CTestInterface {
					CTestInterface0	()
	{
		_CLOSURE_THIS				(CTestInterface0::get_string,OnGetString);
	}

	virtual void	get_string		(LPCSTR &text)
	{
		text			= "CTestInterface0!";
	}
};

struct CTestInterface1 : public CTestInterface {
					CTestInterface1	()
	{
		_CLOSURE_THIS				(CTestInterface1::get_string,OnGetString);
	}

	virtual void	get_string		(LPCSTR &text)
	{
		text			= "CTestInterface1!";
	}
};

DLL_API CTestInterface*	__cdecl get_object(int index)
{
	switch (index) {
		case 0 : return(xr_new<CTestInterface0>()); break;
		case 1 : return(xr_new<CTestInterface1>()); break;
		default : {
			return(0);
		}
	}
}

BOOL APIENTRY DllMain	(HANDLE module_handle, DWORD call_reason, LPVOID reserved)
{
	switch (call_reason) {
		case DLL_PROCESS_ATTACH: {
 			Core._initialize			("xrSE_Factory",NULL);
			ai().script_engine().export	();
			break;
		}
		case DLL_PROCESS_DETACH: {
			Core._destroy				();
			break;
		}
	}
    return				(TRUE);
}
