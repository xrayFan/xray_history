//---------------------------------------------------------------------------
#include "stdafx.h"
#pragma hdrstop

#include "UI_ShaderMain.h"
#include "UI_ShaderTools.h"
#include "topbar.h"
#include "leftbar.h"
#include "D3DUtils.h"
#include "bottombar.h"
#include "xr_trims.h"
#include "main.h"
#include "xr_input.h"

//---------------------------------------------------------------------------
CShaderMain*&	PUI=(CShaderMain*)UI;
//---------------------------------------------------------------------------

CShaderMain::CShaderMain()
{
}
//---------------------------------------------------------------------------

CShaderMain::~CShaderMain()
{
}
//---------------------------------------------------------------------------

bool CShaderMain::Command(int _Command, int p1, int p2)
{
	bool bRes = true;
	string256 filebuffer;
	switch (_Command){
	case COMMAND_SAVE:
    	STools->Save	(0,0);
		Command		(COMMAND_UPDATE_CAPTION);
    	break;
    case COMMAND_SAVE_BACKUP:
		Command		(COMMAND_SAVE);
    	break;
    case COMMAND_RELOAD:
    	STools->Reload();
		Command		(COMMAND_UPDATE_CAPTION);
    	break;
	case COMMAND_CLEAR:
        Device.m_Camera.Reset();
        Command		(COMMAND_UPDATE_CAPTION);
		break;
	case COMMAND_RESET_ANIMATION:
   		break;
//------        
    case COMMAND_REFRESH_UI_BAR:
        fraTopBar->RefreshBar	();
        fraLeftBar->RefreshBar	();
        fraBottomBar->RefreshBar();
        break;
    case COMMAND_RESTORE_UI_BAR:
        fraTopBar->fsStorage->RestoreFormPlacement();
        fraLeftBar->fsStorage->RestoreFormPlacement();
        fraBottomBar->fsStorage->RestoreFormPlacement();
        break;
    case COMMAND_SAVE_UI_BAR:
        fraTopBar->fsStorage->SaveFormPlacement();
        fraLeftBar->fsStorage->SaveFormPlacement();
        fraBottomBar->fsStorage->SaveFormPlacement();
        break;
	case COMMAND_UPDATE_TOOLBAR:
    	fraLeftBar->UpdateBar();
    	break;
    case COMMAND_UPDATE_CAPTION:
    	frmMain->UpdateCaption();
    	break;
//------
    default:
    	return inherited::Command(_Command,p1,p2);
    }
    return 	bRes;
}

char* CShaderMain::GetCaption()
{
	return (LPSTR)STools->CurrentToolsName();// "shaders&materials";
}           

bool __fastcall CShaderMain::ApplyShortCut(WORD Key, TShiftState Shift)
{
    if (inherited::ApplyShortCut(Key,Shift)) return true;
	bool bExec = false;
    return bExec;
}
//---------------------------------------------------------------------------

bool __fastcall CShaderMain::ApplyGlobalShortCut(WORD Key, TShiftState Shift)
{
    if (inherited::ApplyGlobalShortCut(Key,Shift)) return true;
	bool bExec = false;
    return bExec;
}
//---------------------------------------------------------------------------

void CShaderMain::RealUpdateScene()
{
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Common
//---------------------------------------------------------------------------
void CShaderMain::ResetStatus()
{
	VERIFY(m_bReady);
    if (fraBottomBar->paStatus->Caption!=""){
	    fraBottomBar->paStatus->Caption=""; fraBottomBar->paStatus->Repaint();
    }
}
void CShaderMain::SetStatus(LPSTR s, bool bOutLog)
{
	VERIFY(m_bReady);
    if (fraBottomBar->paStatus->Caption!=s){
	    fraBottomBar->paStatus->Caption=s; fraBottomBar->paStatus->Repaint();
    	if (bOutLog&&s&&s[0]) ELog.Msg(mtInformation,s);
    }
}
void CShaderMain::ProgressInfo(LPCSTR text, bool bWarn)
{
	if (text){
		fraBottomBar->paStatus->Caption=fraBottomBar->sProgressTitle+" ("+text+")";
    	fraBottomBar->paStatus->Repaint();
	    ELog.Msg(bWarn?mtError:mtInformation,fraBottomBar->paStatus->Caption.c_str());
    }
}
void CShaderMain::ProgressStart(float max_val, const char* text)
{
	VERIFY(m_bReady);
    fraBottomBar->sProgressTitle = text;
	fraBottomBar->paStatus->Caption=text;
    fraBottomBar->paStatus->Repaint();
	fraBottomBar->fMaxVal=max_val;
	fraBottomBar->fStatusProgress=0;
	fraBottomBar->cgProgress->Progress=0;
	fraBottomBar->cgProgress->Visible=true;
    ELog.Msg(mtInformation,text);
}
void CShaderMain::ProgressEnd()
{
	VERIFY(m_bReady);
    fraBottomBar->sProgressTitle = "";
	fraBottomBar->paStatus->Caption="";
    fraBottomBar->paStatus->Repaint();
	fraBottomBar->cgProgress->Visible=false;
}
void CShaderMain::ProgressUpdate(float val)
{
	VERIFY(m_bReady);
	fraBottomBar->fStatusProgress=val;
    if (fraBottomBar->fMaxVal>=0){
    	int new_val = (int)((fraBottomBar->fStatusProgress/fraBottomBar->fMaxVal)*100);
        if (new_val!=fraBottomBar->cgProgress->Progress){
			fraBottomBar->cgProgress->Progress=(int)((fraBottomBar->fStatusProgress/fraBottomBar->fMaxVal)*100);
    	    fraBottomBar->cgProgress->Repaint();
        }
    }
}
void CShaderMain::ProgressInc(const char* info, bool bWarn)
{
	VERIFY(m_bReady);
    ProgressInfo(info,bWarn);
	fraBottomBar->fStatusProgress++;
    if (fraBottomBar->fMaxVal>=0){
    	int val = (int)((fraBottomBar->fStatusProgress/fraBottomBar->fMaxVal)*100);
        if (val!=fraBottomBar->cgProgress->Progress){
			fraBottomBar->cgProgress->Progress=val;
	        fraBottomBar->cgProgress->Repaint();
        }
    }
}
//---------------------------------------------------------------------------
void CShaderMain::OutCameraPos()
{
	VERIFY(m_bReady);
    AnsiString s;
	const Fvector& c 	= Device.m_Camera.GetPosition();
	s.sprintf("C: %3.1f, %3.1f, %3.1f",c.x,c.y,c.z);
//	const Fvector& hpb 	= Device.m_Camera.GetHPB();
//	s.sprintf(" Cam: %3.1f�, %3.1f�, %3.1f�",rad2deg(hpb.y),rad2deg(hpb.x),rad2deg(hpb.z));
    fraBottomBar->paCamera->Caption=s; fraBottomBar->paCamera->Repaint();
}
//---------------------------------------------------------------------------
void CShaderMain::OutUICursorPos()
{
	VERIFY(m_bReady);
    AnsiString s; POINT pt;
    GetCursorPos(&pt);
    s.sprintf("Cur: %d, %d",pt.x,pt.y);
    fraBottomBar->paUICursor->Caption=s; fraBottomBar->paUICursor->Repaint();
}
//---------------------------------------------------------------------------
void CShaderMain::OutGridSize()
{
	VERIFY(m_bReady);
    AnsiString s;
    s.sprintf("Grid: %1.1f",EPrefs.grid_cell_size);
    fraBottomBar->paGridSquareSize->Caption=s; fraBottomBar->paGridSquareSize->Repaint();
}
//---------------------------------------------------------------------------
void CShaderMain::OutInfo()
{
	fraBottomBar->paSel->Caption = Tools->GetInfo();
}
//---------------------------------------------------------------------------
void CShaderMain::RealQuit()
{
	frmMain->Close();
}
//---------------------------------------------------------------------------

