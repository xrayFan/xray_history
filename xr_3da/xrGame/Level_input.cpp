#include "stdafx.h"
#include "HUDmanager.h"
#include "..\xr_ioconsole.h"
#include "entity.h"
#include "game_sv_single.h"
#include "ai_alife.h"

// ��������� ������� ������
void CLevel::IR_OnKeyboardPress(int key)
{
//	if (pHUD->IsUIActive())			
	if (pHUD->GetUI()->OnKeyboardPress(key)) return;

	switch (key) 
	{
	case DIK_RETURN:
		bDebug	= !bDebug;
		return;
	case DIK_F12:
		Render->Screenshot			();
		return;
	case DIK_BACK:
		HW.Caps.SceneMode			= (HW.Caps.SceneMode+1)%3;
		return;
	case DIK_F6:
		if (Server->game->type == GAME_SINGLE) {
			game_sv_Single *tpGame	= dynamic_cast<game_sv_Single*>(Server->game);
			if (tpGame && tpGame->m_tpALife)
				tpGame->m_tpALife->Save("quick_save");
			else
				net_Save			("quick.save");
		}
		else
			net_Save				("quick.save");
		return;
	case DIK_F7:
		if (Server->game->type == GAME_SINGLE) {
			Engine.Event.Defer		("KERNEL:disconnect");
			Engine.Event.Defer		("KERNEL:start",size_t(xr_strdup(m_caServerOptions)),size_t(xr_strdup(m_caClientOptions)));
		}
		return;
	case DIK_F5: {
		xr_vector<CObject*>::iterator I = Objects.objects.begin(), B = I, J;
		xr_vector<CObject*>::iterator E = Objects.objects.end();
		bool bOk = false;
		if (pCurrentEntity)
			for ( ; I != E; I++)
				if ((*I) == pCurrentEntity)
					break;
		if (I != E) {
			J = I;
			bOk = false;
			for (I++; I != E; I++) {
				CEntityAlive* tpEntityAlive = dynamic_cast<CEntityAlive*>(*I);
				if (tpEntityAlive) {
					bOk = true;
					break;
				}
			}
			if (!bOk)
				for (I = B; I != J; I++) {
					CEntityAlive* tpEntityAlive = dynamic_cast<CEntityAlive*>(*I);
					if (tpEntityAlive) {
						bOk = true;
						break;
					}
				}
			if (bOk) {
				CObject *tpObject = CurrentEntity();
				SetEntity(*I);
				Engine.Sheduler.Unregister	(tpObject);
				Engine.Sheduler.Register	(tpObject);
				Engine.Sheduler.Unregister	(*I);
				Engine.Sheduler.Register	(*I, TRUE);
			}
		}
		return;
	}
#ifdef DEBUG
	case DIK_F9:
		getAI().SetStartPoint(CurrentEntity()->Position());
		return;
	case DIK_F10:
		getAI().SetFinishPoint(CurrentEntity()->Position());
		return;
	case DIK_F11:
		getAI().ComputePath();
		return;
#endif
	}

	switch (key_binding[key]) {
	case kCONSOLE:
		Console->Show				();
		break;
	case kQUIT:	
		Console->Execute			("quit");
		break;
	default:
		if (CurrentEntity())		CurrentEntity()->IR_OnKeyboardPress(key_binding[key]);
		break;
	}
}

void CLevel::IR_OnKeyboardRelease(int key)
{
//	if (pHUD->IsUIActive()) 
	if (pHUD->GetUI()->IR_OnKeyboardRelease(key)) return;

	if (CurrentEntity())	CurrentEntity()->IR_OnKeyboardRelease(key_binding[key]);
}

void CLevel::IR_OnKeyboardHold(int key)
{
	if (CurrentEntity())	CurrentEntity()->IR_OnKeyboardHold(key_binding[key]);
}

static int mouse_button_2_key []	=	{MOUSE_1,MOUSE_2,MOUSE_3};

void CLevel::IR_OnMousePress(int btn)
{	IR_OnKeyboardPress(mouse_button_2_key[btn]);}
void CLevel::IR_OnMouseRelease(int btn)
{	IR_OnKeyboardRelease(mouse_button_2_key[btn]);}
void CLevel::IR_OnMouseHold(int btn)
{	IR_OnKeyboardHold(mouse_button_2_key[btn]);}
void CLevel::IR_OnMouseMove( int dx, int dy )
{
	if (pHUD->GetUI()->IR_OnMouseMove(dx,dy)) return;
	if (CurrentEntity())	CurrentEntity()->IR_OnMouseMove(dx,dy);
}

void CLevel::IR_OnMouseStop( int axis, int value)
{
}
