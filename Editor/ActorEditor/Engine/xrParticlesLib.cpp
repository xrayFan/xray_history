#include "stdafx.h"
#pragma hdrstop

#include "xrParticlesLib.h"
#include "FS.h"

#define PS_LIB_SIGN "PS_LIB"

IC bool sort_pred(const PS::SDef& A, const PS::SDef& B)
{	return strcmp(A.m_Name,B.m_Name)<0; }

void psLibrary_Sort(PS::PSVec &LIB)
{
	std::sort(LIB.begin(),LIB.end(),sort_pred);
}

BOOL psLibrary_Load(const char *Name, PS::PSVec &LIB)
{
	LIB.clear();
    CCompressedStream F(Name,PS_LIB_SIGN);
    WORD version = F.Rword();
    if (version!=PARTICLESYSTEM_VERSION) return false;
    DWORD count = F.Rdword();
    if (count){
        LIB.resize(count);
        F.Read(LIB.begin(), count*sizeof(PS::SDef));
    }
    psLibrary_Sort(LIB);
    return true;
}

void psLibrary_Save(const char *Name, PS::PSVec &LIB)
{
	CFS_Memory F;
    F.Wword(PARTICLESYSTEM_VERSION);
    F.Wdword(LIB.size());
	F.write(LIB.begin(), LIB.size()*sizeof(PS::SDef));
    F.SaveTo(Name,PS_LIB_SIGN);
}

DWORD	psLibrary_GetCount	(const char *Name)
{
    CCompressedStream F(Name,PS_LIB_SIGN);
    return F.Rdword();
}

PS::SDef* psLibrary_FindSorted(const char* Name, PS::PSVec &LIB)
{
	PS::SDef	D; strcpy(D.m_Name,Name);
	PS::SDef*	P = std::lower_bound(LIB.begin(),LIB.end(),D,sort_pred);
	if ((P!=LIB.end()) && (0==strcmp(P->m_Name,Name)) ) return P;
	return NULL;
}

PS::SDef* psLibrary_FindUnsorted(const char* Name, PS::PSVec &LIB)
{
	for (PS::PSIt it=LIB.begin(); it!=LIB.end(); it++)
    	if (0==strcmp(it->m_Name,Name)) return it;
	return NULL;
}
