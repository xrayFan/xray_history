#include "stdafx.h"
#include "../../ai_monster_state.h"
#include "burer_attack.h"
#include "burer.h"
#include "../../../PhysicsShell.h"

#define GOOD_DISTANCE			10.f
#define MAX_HANDLED_OBJECTS		3
#define CHECK_OBJECTS_RADIUS	10.f
#define MINIMAL_MASS			20.f
#define MAXIMAL_MASS			5000.f


#define MIN_DIST_FOR_GRAVI		3.f
#define MAX_DIST_FOR_GRAVI		20.f
#define GRAVI_DELAY				2000
#define GRAVI_HOLD				3000


CBurerAttack::CBurerAttack(CBurer *p)  
{
	pMonster = p;
	SetPriority(PRIORITY_HIGH);
}


void CBurerAttack::Init()
{
	LOG_EX("attack init");
	IState::Init();

	// �������� �����
	enemy		= pMonster->EnemyMan.get_enemy();
	
	m_tAction	= ACTION_DEFAULT;
	
	time_next_gravi_available	= 0;
	time_gravi_started			= 0;
}



void CBurerAttack::Run()
{
	// ���� ���� ���������, ���������������� ���������
	if (pMonster->EnemyMan.get_enemy() != enemy) Init();
	
	if ((pMonster->EnemyMan.get_enemy_time_last_seen() == m_dwCurrentTime) && (time_next_gravi_available < m_dwCurrentTime)) {
		m_tAction = ACTION_GRAVI_STARTED;
	}

	if (m_tAction == ACTION_GRAVI_STARTED) {
		pMonster->MotionMan.TA_Activate(&pMonster->anim_triple_gravi);
		m_tAction = ACTION_GRAVI_CONTINUE;
		time_gravi_started = m_dwCurrentTime;
	}

	if (m_tAction == ACTION_GRAVI_CONTINUE) {
		// check for gravi fire
		if (time_gravi_started + GRAVI_HOLD < m_dwCurrentTime) {
			m_tAction = ACTION_GRAVI_FIRE;
		}
	}
	
	if (m_tAction == ACTION_GRAVI_FIRE) {
		pMonster->MotionMan.TA_PointBreak();
		time_next_gravi_available = m_dwCurrentTime + GRAVI_DELAY;
		m_tAction = ACTION_DEFAULT;
	}
	
	if (m_tAction == ACTION_DEFAULT) {
		m_tAction = ACTION_MELEE;
	}
	
	
	bool state_gravi = (m_tAction == ACTION_GRAVI_STARTED) || (m_tAction == ACTION_GRAVI_CONTINUE) || (m_tAction == ACTION_GRAVI_FIRE);
	
	if (state_gravi) {
		pMonster->MotionMan.m_tAction = ACT_STAND_IDLE;
	} else {
		pMonster->MotionMan.m_tAction = ACT_WALK_FWD;
	}
	
	


//	update();
//	
//	m_tAction = ACTION_DEFAULT;
//	
//	if (pMonster->CTelekinesis::get_objects_count() > 0)	m_tAction = ACTION_TELE;
//	
//	if (CheckGravi())										m_tAction = ACTION_GRAVI;
//	if (CheckGraviFire())									m_tAction = ACTION_GRAVI_FIRE;
//	
//	if (m_tAction == ACTION_DEFAULT)						m_tAction = ACTION_MELEE;
//
//	switch (m_tAction) {
//		case ACTION_TELE:				Execute_Telekinetic	();		break;
//		case ACTION_GRAVI:				Execute_Gravi		();		break;
//		case ACTION_GRAVI_FIRE:			Execute_Gravi_Fire	();		break;
//		case ACTION_MELEE:				Execute_Melee		();		break;
//	}

	m_tPrevAction = m_tAction;
}

//////////////////////////////////////////////////////////////////////////
// Checking
//////////////////////////////////////////////////////////////////////////

bool CBurerAttack::CheckGravi()
{
	if ((m_tAction != ACTION_DEFAULT) || 
		(m_tPrevAction == ACTION_GRAVI_FIRE))	return false;

	if (time_next_gravi_available > m_dwCurrentTime) return false;

	return true;
}

bool CBurerAttack::CheckGraviFire() 
{
//	if ((m_tAction		!= ACTION_DEFAULT) && 
//		(m_tPrevAction	!= ACTION_GRAVI))		return false;

	//if (time_gravi_started == )

	return true;
}


//////////////////////////////////////////////////////////////////////////
// Processing
//////////////////////////////////////////////////////////////////////////

void CBurerAttack::Execute_Telekinetic()
{
	LOG_EX("telekinetic attack");
	pMonster->MotionMan.m_tAction= ACT_STAND_IDLE;

	float dist;	

	u32 i=0;
	while (i < pMonster->CTelekinesis::get_objects_count()) {
		CTelekineticObject tele_object = pMonster->CTelekinesis::get_object_by_index(i);
		if ((tele_object.get_state() == TS_Keep) && (tele_object.time_keep_started + 2000 < m_dwCurrentTime)) {
			Fvector enemy_pos = enemy->Position();
			enemy_pos.y += 2 * enemy->Radius();
			
			dist = tele_object.get_object()->Position().distance_to(pMonster->Position());
			pMonster->CTelekinesis::fire(tele_object.get_object(), enemy_pos, dist*dist*dist);
		} else i++;
	}
}

void CBurerAttack::Execute_Gravi()
{
	LOG_EX("gravi attack");
	pMonster->MotionMan.m_tAction = ACT_STAND_IDLE;
	
//	if (!pMonster->MotionMan.TA_IsActive()) {
//		pMonster->MotionMan.TA_Activate(&pMonster->anim_triple_gravi);
//		time_gravi_started = m_dwCurrentTime;
//	}
}	

void CBurerAttack::Execute_Gravi_Fire()
{
//	if (pMonster->MotionMan.TA_IsActive()) {
//		if (time_next_gravi_available + 3000 > m_dwCurrentTime)
//			pMonster->MotionMan.TA_PointBreak();
//	}
	time_next_gravi_available = m_dwCurrentTime + GRAVI_DELAY;
}

void CBurerAttack::Execute_Melee()
{
	LOG_EX("melee attack");
	pMonster->MotionMan.m_tAction= ACT_ATTACK;
}


//////////////////////////////////////////////////////////////////////////
// Update State
//////////////////////////////////////////////////////////////////////////

void CBurerAttack::update()
{
	float dist = pMonster->Position().distance_to(enemy->Position());

	// ���������� �������
	if (dist > GOOD_DISTANCE && (pMonster->CTelekinesis::get_objects_count() < 4)) 
		find_tele_objects();
}

//////////////////////////////////////////////////////////////////////////
// Additional stuff
//////////////////////////////////////////////////////////////////////////

u32	CBurerAttack::get_number_available_objects(xr_vector<CObject*> &tpObjects)
{
	u32 ret_val = 0;

	for (u32 i=0;i<tpObjects.size();i++) {
		CGameObject *obj = dynamic_cast<CGameObject *>(tpObjects[i]);
		if (!obj || !obj->m_pPhysicsShell || (obj->m_pPhysicsShell->getMass() < MINIMAL_MASS) || (obj->m_pPhysicsShell->getMass() > MAXIMAL_MASS) || (obj == pMonster)) continue;
		ret_val++;
	}

	return ret_val;
}

void CBurerAttack::find_tele_objects()
{
	// �������� ������ �������� ������ �������
	Level().ObjectSpace.GetNearest	(pMonster->Position(), CHECK_OBJECTS_RADIUS);
	xr_vector<CObject*> &tpNearest	= Level().ObjectSpace.q_nearest;

	bool b_objects_found = true;

	if (get_number_available_objects(tpNearest) == 0) {
		// �������� ������ �������� �� �������� ���� �� ������� �� �����
		Fvector dir;
		dir.sub(enemy->Position(), pMonster->Position());
		dir.normalize();

		float dist = enemy->Position().distance_to(pMonster->Position());

		Fvector pos;
		pos.mad(pMonster->Position(), dir, dist / 2.f);
		Level().ObjectSpace.GetNearest(pos, CHECK_OBJECTS_RADIUS); 
		tpNearest	= Level().ObjectSpace.q_nearest; 

		if (get_number_available_objects(tpNearest) == 0) {
			b_objects_found = false;
		}
	} 

	if (b_objects_found) {
		// ������� ������
		for (u32 i=0;i<tpNearest.size();i++) {
			CGameObject *obj = dynamic_cast<CGameObject *>(tpNearest[i]);
			// �������� �� ������� � ��� �����
			if (!obj || !obj->m_pPhysicsShell || (obj->m_pPhysicsShell->getMass() < MINIMAL_MASS) || (obj->m_pPhysicsShell->getMass() > MAXIMAL_MASS) || (obj == pMonster)) continue;

			// ���������, ������� �� ��� ������
			if (pMonster->CTelekinesis::is_active_object(obj)) continue;

			// ��������� ��������� �� ������
			pMonster->CTelekinesis::activate(obj, 3.f, 2.f, 10000);
			break;
		}
	}
}

