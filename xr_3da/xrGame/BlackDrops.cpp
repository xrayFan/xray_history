///////////////////////////////////////////////////////////////
// BlackDrops.cpp
// BlackDrops - ������ �����
///////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BlackDrops.h"
#include "PhysicsShell.h"


CBlackDrops::CBlackDrops(void) 
{
}

CBlackDrops::~CBlackDrops(void) 
{
}

void CBlackDrops::Load(LPCSTR section) 
{
	inherited::Load(section);
}



void CBlackDrops::UpdateCL() 
{
	inherited::UpdateCL();

	if(getVisible() && m_pPhysicsShell) 
	{
		m_pPhysicsShell->Update	();
		XFORM().set(m_pPhysicsShell->mXFORM);
		Position().set(m_pPhysicsShell->mXFORM.c);
	} 
	else if(H_Parent()) XFORM().set(H_Parent()->XFORM());
}