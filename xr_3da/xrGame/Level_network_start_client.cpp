#include "stdafx.h"
#include "HUDmanager.h"
#include "PHdynamicdata.h"
#include "Physics.h"

BOOL CLevel::net_Start_client	( LPCSTR name_of_server )
{
	pApp->LoadBegin	();

	// Startup client
	char			temp[256];
	sprintf			(temp,"CLIENT: Connecting to '%s'...",name_of_server);
	pApp->LoadTitle	(temp);

	// HUD
	Device.Shader.DeferredLoad	(TRUE);

	if (Connect(name_of_server)) 
	{
		// Determine internal level-ID
		LPCSTR	level_name	= net_SessionName	();
		int		level_id	= pApp->Level_ID(level_name);
		if	(level_id<0)	{
			Disconnect		();
			pApp->LoadEnd	();
			return FALSE;
		}
		pApp->Level_Set		(level_id);

		// Load level
		R_ASSERT(Load				(level_id));
		pHUD->Load					();
		// ph_world					= new CPHWorld;
		// ph_world->Create			();

		// Waiting for connection completition
		pApp->LoadTitle				("CLIENT: Spawning...");
		while (!net_isCompleted_Connect()) Sleep(5);

		// And receiving spawn information (game-state)
		BOOL bFinished		= FALSE;
		while (!bFinished) 
		{
			for (NET_Packet* P = net_msg_Retreive(); P; P=net_msg_Retreive())
			{
				u16			m_type;	
				P->r_begin	(m_type);
				switch (m_type)
				{
				case M_SV_CONFIG_GAME:		
					{
						u8				gametype;
						u16				fraglimit;
						u16				timelimit;
						P->r_u8			(gametype);		GAME		= gametype;
						P->r_u16		(fraglimit);	g_fraglimit	= fraglimit;
						P->r_u16		(timelimit);	g_timelimit = timelimit;
					}
					break;
				case M_SV_CONFIG_FINISHED:	bFinished = TRUE;	break;
				case M_SPAWN:				g_sv_Spawn(P);		break;
				}
				net_msg_Release	();
			}
			Sleep	(1);
		}

		// Textures
		pApp->LoadTitle						("Loading textures...");
		Device.Shader.DeferredLoad			(FALSE);
		Device.Shader.DeferredUpload		();
		Msg	("* %d K used for textures",	Device.Shader._GetMemoryUsage()/1024);

		// Sync
		pApp->LoadTitle						("CLIENT: Syncronising...");
		while (!net_isCompleted_Sync())		Sleep(5);

		if (strstr(Path.Current,"escape"))	Engine.Event.Signal	("level.weather.rain.start");

		// signal OK
		pApp->LoadEnd	();
		return TRUE;
	}
	Device.Shader.DeferredLoad	(FALSE);
	Device.Shader.DeferredUpload();

	pApp->LoadEnd	(); 
	return FALSE;
}
