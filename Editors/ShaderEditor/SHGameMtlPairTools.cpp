//---------------------------------------------------------------------------
#include "stdafx.h"
#pragma hdrstop

#include "SHGameMtlPairTools.h"
#include "UI_Tools.h"
#include "folderlib.h"
#include "ChoseForm.h"
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
CSHGameMtlPairTools::CSHGameMtlPairTools(ISHInit& init):ISHTools(init)
{
    m_MtlPair 			= 0;
    m_GameMtlTools		= 0;
}

CSHGameMtlPairTools::~CSHGameMtlPairTools()
{
}
//---------------------------------------------------------------------------

void CSHGameMtlPairTools::OnFrame()
{
}
//---------------------------------------------------------------------------

bool CSHGameMtlPairTools::OnCreate()
{
	m_GameMtlTools		= Tools.FindTools(aeMtl); R_ASSERT(m_GameMtlTools);
    Load();
    return true;
}
//---------------------------------------------------------------------------

void CSHGameMtlPairTools::OnDestroy()
{
    m_bModified = FALSE;
}
//---------------------------------------------------------------------------

void CSHGameMtlPairTools::Reload()
{
	// mtl
	ViewClearItemList();
    ResetCurrentItem();
    // mtl pair
	m_GameMtlTools->ViewClearItemList();
    m_GameMtlTools->ResetCurrentItem();
    // load
    Load();
    // mtl pair
	m_GameMtlTools->FillItemList();
}
//---------------------------------------------------------------------------

void CSHGameMtlPairTools::FillItemList()
{
    View()->IsUpdating = true;
	ViewClearItemList();
    for (GameMtlPairIt p_it=GMLib.FirstMaterialPair(); p_it!=GMLib.LastMaterialPair(); p_it++)
        ViewAddItem(GMLib.MtlPairToName((*p_it)->GetMtl0(),(*p_it)->GetMtl1()));
	m_MtlPair=0;
    View()->IsUpdating = false;
}
//---------------------------------------------------------------------------

void CSHGameMtlPairTools::Load()
{
    m_bLockUpdate		= TRUE;

    GMLib.Unload		();
    GMLib.Load			();
    FillItemList		();
    ResetCurrentItem	();

    m_bLockUpdate		= FALSE;
}
//---------------------------------------------------------------------------

void CSHGameMtlPairTools::Save()
{
    AnsiString name;
    FHelper.MakeFullName(View()->Selected,0,name);
	ResetCurrentItem	();
    m_bLockUpdate		= TRUE;

    // save
    EFS.UnlockFile		(_game_data_,GAMEMTL_FILENAME,false);
    EFS.BackupFile		(_game_data_,GAMEMTL_FILENAME);
    GMLib.Save			();
    EFS.LockFile		(_game_data_,GAMEMTL_FILENAME,false);
    
    m_bLockUpdate		= FALSE;
	SetCurrentItem		(name.c_str());

    m_bModified	= FALSE;
}
//---------------------------------------------------------------------------

LPCSTR CSHGameMtlPairTools::GenerateItemName(LPSTR name, LPCSTR pref, LPCSTR source)
{
	return 0;
}
//---------------------------------------------------------------------------

void CSHGameMtlPairTools::UpdateProperties()
{
	PropItemVec items;
    if (m_MtlPair)	m_MtlPair->FillProp(items);
    Ext.m_ItemProps->AssignItems		(items,true);
    Ext.m_ItemProps->SetModifiedEvent	(Modified);
}
//---------------------------------------------------------------------------

void CSHGameMtlPairTools::ApplyChanges(bool bForced)
{
}
//---------------------------------------------------------------------------

LPCSTR CSHGameMtlPairTools::AppendItem(LPCSTR folder_name, LPCSTR parent_name)
{
    LPCSTR M0=0,M1=0;
    if (TfrmChoseItem::SelectItem(TfrmChoseItem::smGameMaterial,M0,1)){
	    if (TfrmChoseItem::SelectItem(TfrmChoseItem::smGameMaterial,M1,1)){
        	int mtl0			= GMLib.GetMaterialID	(M0);
        	int mtl1			= GMLib.GetMaterialID	(M1);
	        SGameMtlPair* parent= GMLib.GetMaterialPair(parent_name);
			SGameMtlPair* S 	= GMLib.AppendMaterialPair(mtl0,mtl1,parent);
	        AnsiString nm		= GMLib.MtlPairToName(S->GetMtl0(),S->GetMtl1());
		    ViewAddItem			(nm.c_str());
            SetCurrentItem		(nm.c_str());
            Modified			();
	        return nm.c_str();
        }
    }
/*    
    	GMLib.MtlNameToMtlPair	(M,mtl0,mtl1);
        SGameMtlPair* parent	= GMLib.GetMaterialPair(parent_name);
		SGameMtlPair* S 		= GMLib.AppendMaterialPair(mtl0,mtl1,parent);
        AnsiString nm			= GMLib.MtlPairToName(S->GetMtl0(),S->GetMtl1());
	    ViewAddItem				(nm.c_str());
		SetCurrentItem			(nm.c_str());
		Modified				();
        return nm.c_str();
    }else{
    	if (1==cnt) ELog.DlgMsg(mtError,"Select 2 material.");
    }
*/
    return 0;              
}
//---------------------------------------------------------------------------

void CSHGameMtlPairTools::SetCurrentItem(LPCSTR name)
{
    if (m_bLockUpdate) return;
	SGameMtlPair* S=GMLib.GetMaterialPair(name);
    // set material
	if (m_MtlPair!=S){
        m_MtlPair = S;
        UpdateProperties();
    }
	ViewSetCurrentItem(name);
}
//---------------------------------------------------------------------------

void CSHGameMtlPairTools::RemoveItem(LPCSTR name)
{
	R_ASSERT(name && name[0]);
    GMLib.RemoveMaterialPair(name);
}
//---------------------------------------------------------------------------

void CSHGameMtlPairTools::ResetCurrentItem()
{
	m_MtlPair	= 0;
}
//---------------------------------------------------------------------------

