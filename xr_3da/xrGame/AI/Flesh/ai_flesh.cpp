#include "stdafx.h"
#include "ai_flesh.h"

CAI_Flesh::CAI_Flesh()
{
	stateRest			= xr_new<CBitingRest>		(this);
	stateAttack			= xr_new<CBitingAttack>		(this);
	stateEat			= xr_new<CBitingEat>		(this);
	stateHide			= xr_new<CBitingHide>		(this);
	stateDetour			= xr_new<CBitingDetour>		(this);
	statePanic			= xr_new<CBitingPanic>		(this);
	stateExploreDNE		= xr_new<CBitingExploreDNE>	(this);
	stateExploreDE		= xr_new<CBitingExploreDE>	(this);
	stateExploreNDE		= xr_new<CBitingExploreNDE>	(this);
	CurrentState		= stateRest;

	Init();
}

CAI_Flesh::~CAI_Flesh()
{
	xr_delete(stateRest);
	xr_delete(stateAttack);
	xr_delete(stateEat);
	xr_delete(stateHide);
	xr_delete(stateDetour);
	xr_delete(statePanic);
	xr_delete(stateExploreDNE);
	xr_delete(stateExploreDE);
	xr_delete(stateExploreNDE);
}


void CAI_Flesh::Init()
{
	inherited::Init();

	CurrentState					= stateRest;
	CurrentState->Reset				();
	m_fEyeShiftYaw					= PI_DIV_6;
}


void CAI_Flesh::Think()
{
	inherited::Think();

	// A - � ����� ������� ����
	// B - � ����� ��������� ����
	// � - � ���� ����� �������� �����
	// D - � ���� �������� �����
	// E - � ���� ������� �����
	// F - � ���� ������� �����
	// H - ���� ��������
	// I - ���� ����� ����
	// J - A | B
	// K - C | D | E | F 

	VisionElem ve;

	if (Motion.m_tSeq.Active())	{
		Motion.m_tSeq.Cycle(m_dwCurrentUpdate);
	}else {
		//- FSM 1-level 

		//if (flagEnemyLostSight && H && (E || F) && !A) SetState(stateFindEnemy);	// ����� �����
		if (C && H && I)		SetState(statePanic);
		else if (C && H && !I)		SetState(statePanic);
		else if (C && !H && I)		SetState(statePanic);
		else if (C && !H && !I) 	SetState(statePanic);
		else if (D && H && I)		SetState(stateAttack);
		else if (D && H && !I)		SetState(stateAttack);  //���� ����������� � ������ ������
		else if (D && !H && I)		SetState(statePanic);
		else if (D && !H && !I) 	SetState(stateHide);	// ����� ����������� ����� �������
		else if (E && H && I)		SetState(stateAttack); 
		else if (E && H && !I)  	SetState(stateAttack);  //���� ����������� � ������ ������
		else if (E && !H && I) 		SetState(stateDetour); 
		else if (E && !H && !I)		SetState(stateDetour); 
		else if (F && H && I) 		SetState(stateAttack); 		
		else if (F && H && !I)  	SetState(stateAttack); 
		else if (F && !H && I)  	SetState(stateDetour); 
		else if (F && !H && !I) 	SetState(stateHide);
		else if (A && !K && !H)		SetState(stateExploreNDE);  //SetState(stateExploreDNE);  // ����� ������� ����, �� �� ����, ���� �� ��������		(ExploreDNE)
		else if (A && !K && H)		SetState(stateExploreNDE);  //SetState(stateExploreDNE);	//SetState(stateExploreDE);	// ����� ������� ����, �� �� ����, ���� ��������			(ExploreDE)		
		else if (B && !K && !H)		SetState(stateExploreNDE);	// ����� �� ������� ����, �� �� ����, ���� �� ��������	(ExploreNDNE)
		else if (B && !K && H)		SetState(stateExploreNDE);	// ����� �� ������� ����, �� �� ����, ���� ��������		(ExploreNDE)
		else if (GetCorpse(ve) && ve.obj->m_fFood > 1)	
			SetState(stateEat);
		else						SetState(stateRest); 
		//-
		
		CurrentState->Execute(m_dwCurrentUpdate);

		// ��������� �� �������������
		if (CurrentState->CheckCompletion()) SetState(stateRest, true);
	}

	Motion.SetFrameParams(this);
	ControlAnimation();		
}

void CAI_Flesh::UpdateCL()
{
	inherited::UpdateCL();

	// �������� ��������� �������� (�����)
	TTime cur_time = Level().timeServer();

	VisionElem ve;
	if (!GetEnemy(ve)) return;
	CObject *obj = dynamic_cast<CObject *>(ve.obj);

	if (m_tAttack.time_started != 0) {

		if ((m_tAttack.time_started + m_tAttack.time_from < cur_time) && 
			(m_tAttack.time_started + m_tAttack.time_to > cur_time) && 
			(m_tAttack.LastAttack + 1000 < cur_time)) {

				// ����������� �����?
				if (m_tAttack.b_fire_anyway) {
					DoDamage(ve.obj); // �� �����
					m_tAttack.LastAttack = cur_time;
				}
				else if (m_tAttack.b_attack_rat) {

					// TestIntersection ������(������) � �����(�����)
					bool Intersected = false;

					float angle = PI_DIV_6;					// ���� ������
					Fvector fromV = m_tAttack.TraceFrom;	// ������� ������
					Fvector dir;							// ����������� ������
					dir.set(0.f,-1.f,0.f);

					float fInvSin = 1.0f/_sin(angle);
					float fCosSqr = _cos(angle)*_cos(angle);

					Fvector vC;		ve.obj->Center(vC);		// ����� �����
					Fvector kCmV;	kCmV.sub(vC,fromV);
					Fvector kD		= kCmV;
					Fvector tempV	= dir;
					tempV.mul(ve.obj->Radius()* fInvSin);
					kD.add(tempV);

					float fDSqrLen = kD.square_magnitude();
					float fE = kD.dotproduct(dir);
					if ( fE > 0.0f && fE*fE >= fDSqrLen*fCosSqr )
					{
						float fSinSqr = _sin(angle)*_sin(angle);

						fDSqrLen = kCmV.square_magnitude();
						fE = -kCmV.dotproduct(dir);
						if ( fE > 0.0f && fE*fE >= fDSqrLen*fSinSqr )
						{
							float fRSqr = ve.obj->Radius()*ve.obj->Radius();
							Intersected =  fDSqrLen <= fRSqr;
						}else Intersected = true;
					} else Intersected = false;

					if (Intersected) {
						DoDamage(ve.obj);
						m_tAttack.LastAttack = cur_time;
					}

				} else 	{ // �����
					this->setEnabled(false);
					Collide::ray_query	l_rq;

					if (Level().ObjectSpace.RayPick(m_tAttack.TraceFrom, Direction(), m_tAttack.dist, l_rq)) {
						if ((l_rq.O == obj) && (l_rq.range < m_tAttack.dist)) {
							DoDamage(ve.obj);
							m_tAttack.LastAttack = cur_time;
						}
					}

					this->setEnabled(true);			
				}

				if (!ve.obj->g_Alive()) AddCorpse(ve);
			}
	}
}


void CAI_Flesh::MotionToAnim(EMotionAnim motion, int &index1, int &index2, int &index3)
{
	switch(motion) {
		case eMotionStandIdle:		index1 = 0; index2 = 0;	 index3 = -1;	break;
		case eMotionLieIdle:		index1 = 2; index2 = 0;	 index3 = -1;	break;
		case eMotionStandTurnLeft:	index1 = 0; index2 = 1;	 index3 = -1;	break;
		case eMotionWalkFwd:		index1 = 0; index2 = 2;	 index3 = -1;	break;
		case eMotionWalkBkwd:		index1 = 0; index2 = 3;  index3 = -1;	break;
		case eMotionWalkTurnLeft:	index1 = 0; index2 = 4;  index3 = -1;	break;
		case eMotionWalkTurnRight:	index1 = 0; index2 = 5;  index3 = -1;	break;
		case eMotionRun:			index1 = 0; index2 = 6;  index3 = -1;	break;
		case eMotionRunTurnLeft:	index1 = 0; index2 = 7;  index3 = -1;	break;
		case eMotionRunTurnRight:	index1 = 0; index2 = 8;  index3 = -1;	break;
		case eMotionAttack:			index1 = 0; index2 = 9;  index3 = -1;	break;
		case eMotionAttackRat:		index1 = 0; index2 = 10; index3 = -1;	break;
		case eMotionFastTurnLeft:	index1 = 0; index2 = 11; index3 = -1;	break;
		case eMotionEat:			index1 = 2; index2 = 12; index3 = -1;	break;
		case eMotionStandDamaged:	index1 = 0; index2 = 13; index3 = -1;	break;
		case eMotionScared:			index1 = 0; index2 = 14; index3 = -1;	break;
		case eMotionDie:			index1 = 0; index2 = 15; index3 = -1;	break;
		case eMotionLieDown:		index1 = 0; index2 = 16; index3 = -1;	break;
		case eMotionStandUp:		index1 = 2; index2 = 17; index3 = -1;	break;
		case eMotionCheckCorpse:	index1 = 0; index2 = 0;	 index3 = 2;	break;
		case eMotionLieDownEat:		index1 = 0; index2 = 18; index3 = -1;	break;
		case eMotionAttackJump:		index1 = 0; index2 = 19; index3 = -1;	break;
		//default:					NODEFAULT;
	}
}

void CAI_Flesh::FillAttackStructure(u32 i, TTime t)
{
	m_tAttack.i_anim		= i;
	m_tAttack.time_started	= t;
	m_tAttack.b_fire_anyway = false;
	m_tAttack.b_attack_rat	= false;

	Fvector tempV;

	switch (m_tAttack.i_anim) {
		case 0:
			m_tAttack.time_from = 700;
			m_tAttack.time_to	= 800;
			m_tAttack.dist		= 2.f;
			Center(m_tAttack.TraceFrom);
			break;
		case 1:
			m_tAttack.time_from = 600;
			m_tAttack.time_to	= 800;
			m_tAttack.dist		= 2.5f;
			Center(m_tAttack.TraceFrom);
			break;
		case 2:
			m_tAttack.time_from = 1100;
			m_tAttack.time_to	= 1250;
			m_tAttack.dist		= 1.5f;
			Center(m_tAttack.TraceFrom);
			tempV.set(0.1f,0.f,0.f);
			m_tAttack.TraceFrom.add(tempV);
			break;
		case 3:
			m_tAttack.time_from = 1300;
			m_tAttack.time_to	= 1400;
			m_tAttack.dist		= 0.6f;
			Center(m_tAttack.TraceFrom);
			tempV.set(0.1f,0.f,0.f);
			m_tAttack.TraceFrom.sub(tempV);
			break;
		case 4:
			m_tAttack.time_from = 600;
			m_tAttack.time_to	= 800;
			m_tAttack.dist		= 2.6f;
			Center(m_tAttack.TraceFrom);
			tempV.set(-0.5f,0.f,0.5f);
			m_tAttack.TraceFrom.add(tempV);
			m_tAttack.b_attack_rat = true;
			break;
		case 5:
			m_tAttack.time_from = 700;
			m_tAttack.time_to	= 850;
			m_tAttack.b_fire_anyway = true;
			break;
	}
}


