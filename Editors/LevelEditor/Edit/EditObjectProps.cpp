#include "stdafx.h"
#pragma hdrstop

#include "EditObject.h"
#include "EditMesh.h"
#include "ui_main.h"
#include "PropertiesListHelper.h"
#include "ItemListHelper.h"
#include "motion.h"
#include "bone.h"

void __fastcall	CEditableObject::OnChangeShader(PropValue* prop)
{
    OnDeviceDestroy	();
    UI.RedrawScene	();
}
//---------------------------------------------------------------------------

void CEditableObject::FillSurfaceProps(CSurface* SURF, LPCSTR pref, PropItemVec& items)
{
    PropValue* V;
    V=PHelper.CreateATexture(items, PHelper.PrepareKey(pref,"Texture"), 	&SURF->m_Texture);					V->OnChangeEvent=OnChangeShader;
    V=PHelper.CreateAEShader(items, PHelper.PrepareKey(pref,"Shader"), 		&SURF->m_ShaderName);				V->OnChangeEvent=OnChangeShader;
    V=PHelper.CreateACShader(items, PHelper.PrepareKey(pref,"Compile"), 	&SURF->m_ShaderXRLCName);
    V=PHelper.CreateAGameMtl(items, PHelper.PrepareKey(pref,"Game Mtl"),	&SURF->m_GameMtlName);				V->OnChangeEvent=SURF->OnChangeGameMtl;
    V=PHelper.CreateFlag32	(items, PHelper.PrepareKey(pref,"2 Sided"), 	&SURF->m_Flags, CSurface::sf2Sided);V->OnChangeEvent=OnChangeShader;
}
//---------------------------------------------------------------------------

void CEditableObject::FillBasicProps(LPCSTR pref, PropItemVec& items)
{
	PropValue* V=0;
	PHelper.CreateCaption	(items, PHelper.PrepareKey(pref,"Reference Name"),		m_LibName.c_str());
    PHelper.CreateFlag32	(items,	PHelper.PrepareKey(pref,"Flags\\Dynamic"),		&m_Flags,		CEditableObject::eoDynamic);
    PHelper.CreateFlag32	(items,	PHelper.PrepareKey(pref,"Flags\\HOM"),			&m_Flags,		CEditableObject::eoHOM);
    PHelper.CreateFlag32	(items,	PHelper.PrepareKey(pref,"Flags\\Use LOD"),		&m_Flags,		CEditableObject::eoUsingLOD);
    PHelper.CreateFlag32	(items,	PHelper.PrepareKey(pref,"Flags\\Multiple Usage"),&m_Flags,		CEditableObject::eoMultipleUsage);
    V=PHelper.CreateVector	(items, PHelper.PrepareKey(pref,"Transform\\Position"),	&t_vPosition,	-10000,	10000,0.01,2); 	V->OnChangeEvent=OnChangeTransform;
    V=PHelper.CreateAngle3	(items, PHelper.PrepareKey(pref,"Transform\\Rotation"),	&t_vRotate, 	-10000,	10000,0.1,1);	V->OnChangeEvent=OnChangeTransform;
    V=PHelper.CreateVector	(items, PHelper.PrepareKey(pref,"Transform\\Scale"),	&t_vScale, 		0.01,	10000,0.01,2);	V->OnChangeEvent=OnChangeTransform;
    V=PHelper.CreateCaption	(items, PHelper.PrepareKey(pref,"Transform\\BBox Min"),	AnsiString().sprintf("{%3.2f, %3.2f, %3.2f}",VPUSH(GetBox().min)));
    V=PHelper.CreateCaption	(items, PHelper.PrepareKey(pref,"Transform\\BBox Max"),	AnsiString().sprintf("{%3.2f, %3.2f, %3.2f}",VPUSH(GetBox().max)));

    FillSummaryProps		(pref,items);
}
//---------------------------------------------------------------------------

void CEditableObject::FillSummaryProps(LPCSTR pref, PropItemVec& items)
{
    AnsiString t; t.sprintf("V: %d, F: %d",		GetVertexCount(),GetFaceCount());
    PHelper.CreateCaption(items,PHelper.PrepareKey(pref,"Geometry\\Object"),t.c_str());
    for (EditMeshIt m_it=FirstMesh(); m_it!=LastMesh(); m_it++){
        CEditableMesh* MESH=*m_it;
        t.sprintf("V: %d, F: %d",MESH->GetVertexCount(),MESH->GetFaceCount());
	    PHelper.CreateCaption(items,PHelper.PrepareKey(pref,AnsiString(AnsiString("Geometry\\Meshes\\")+MESH->GetName()).c_str()),t.c_str());
    }
    PHelper.CreateAText(items,PHelper.PrepareKey(pref, "Game options\\Script"),&m_ClassScript);
}
//---------------------------------------------------------------------------

AnsiString MakeFullBoneName(BoneVec& lst, CBone* bone)
{
	if ((bone->ParentIndex()!=-1)){
    	return MakeFullBoneName(lst,lst[bone->ParentIndex()])+"\\"+bone->Name();
    }else{
    	return bone->Name();
    }
}

AnsiString MakeFullBonePath(BoneVec& lst, CBone* bone)
{
	if ((bone->ParentIndex()!=-1)){
    	return MakeFullBoneName(lst,lst[bone->ParentIndex()]);
    }else{
    	return "";
    }
}

void CEditableObject::FillSurfaceList(LPCSTR pref, ListItemsVec& items, int modeID)
{
	if (pref) LHelper.CreateItem(items, pref,  0, modeID);
    for (SurfaceIt s_it=FirstSurface(); s_it!=LastSurface(); s_it++)
        LHelper.CreateItem(items, pref, (*s_it)->_Name(),  modeID);
}
//---------------------------------------------------------------------------

void CEditableObject::FillBoneList(LPCSTR pref, ListItemsVec& items, int modeID)
{
    BoneVec& b_lst 		= Bones();
	if (pref) LHelper.CreateItem(items, pref,  0, modeID);
    for(BoneIt b_it=b_lst.begin(); b_it!=b_lst.end(); b_it++){
    	AnsiString pt	= MakeFullBonePath(b_lst,*b_it);
    	LPCSTR path		= pt.IsEmpty()?pref:LHelper.PrepareKey(pref, pt.c_str());
        LHelper.CreateItem(items, path, (*b_it)->Name(), modeID);
    }
}

void CEditableObject::FillMotionList(LPCSTR pref, ListItemsVec& items, int modeID)
{
    SMotionVec&	m_lst	= SMotions();
	if (pref) LHelper.CreateItem(items, pref,  0, modeID);
    for (SMotionIt it=m_lst.begin(); it!=m_lst.end(); it++)
        LHelper.CreateItem(items, pref, (*it)->Name(), modeID);
}



 