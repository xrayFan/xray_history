// xrRender_R1.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "xrRender_console.h"

#pragma comment(lib,"x:\\xr_3DA")

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		::Device.Resources->SetHLSL_path	("R1\\");
		::Render							= &RImplementation;
		xrRender_initconsole				();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}
