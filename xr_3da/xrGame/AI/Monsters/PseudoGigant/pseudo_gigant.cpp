#include "stdafx.h"
#include "pseudo_gigant.h"

CPseudoGigant::CPseudoGigant()
{
	stateRest			= xr_new<CBitingRest>(this);
	CurrentState		= stateRest;

	Init();
}

CPseudoGigant::~CPseudoGigant()
{
	xr_delete(stateRest);
}


void CPseudoGigant::Init()
{
	inherited::Init();

	CurrentState					= stateRest;
	CurrentState->Reset				();
}

void CPseudoGigant::Load(LPCSTR section)
{
	inherited::Load	(section);

	BEGIN_LOAD_SHARED_MOTION_DATA();

	END_LOAD_SHARED_MOTION_DATA();
}

void CPseudoGigant::StateSelector()
{	
	SetState(stateRest);
}

