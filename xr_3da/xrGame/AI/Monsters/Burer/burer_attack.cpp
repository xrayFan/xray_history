#include "stdafx.h"
#include "../../ai_monster_state.h"
#include "burer_attack_tele.h"
#include "burer_attack_melee.h"
#include "burer_attack_gravi.h"
#include "burer_attack_misc.h"
#include "burer_attack.h"
#include "burer.h"
#include "../../ai_monster_debug.h"


#define GRAVI_PERCENT		80
#define TELE_PERCENT		50
#define RUN_AROUND_PERCENT	20

CBurerAttack::CBurerAttack(CBurer *p)  
{
	pMonster = p;
	SetPriority(PRIORITY_HIGH);

	stateTele		= xr_new<CBurerAttackTele>(p);
	stateMelee		= xr_new<CBurerAttackMelee>(p);
	stateGravi		= xr_new<CBurerAttackGravi>(p);
	stateRunAround	= xr_new<CBurerAttackRunAround>(p);
	stateFaceEnemy	= xr_new<CBurerAttackFaceTarget>(p);
	cur_state	= 0;
}

CBurerAttack::~CBurerAttack() 
{
	xr_delete(stateTele);
	xr_delete(stateMelee);
	xr_delete(stateGravi);
	xr_delete(stateRunAround);
	xr_delete(stateFaceEnemy);

}


void CBurerAttack::Init()
{
	LOG_EX("attack init");
	IState::Init();

	// �������� �����
	enemy			= pMonster->EnemyMan.get_enemy();
	
	stateTele->UpdateExternal();
	stateMelee->UpdateExternal();
	stateGravi->UpdateExternal();
	stateRunAround->UpdateExternal();
	stateFaceEnemy->UpdateExternal();

	b_need_reselect = true;
	prev_state_run_around = false;
}



void CBurerAttack::Run()
{
	// ���� ���� ���������, ���������������� ���������
	if (pMonster->EnemyMan.get_enemy() != enemy) {
		Init();
		if (cur_state) {
			cur_state->CriticalInterrupt();
			cur_state->Reset();
		}
		b_need_reselect = true;		
	}

	// ��������� ����� �� ������������ ���������
	if (b_need_reselect) {
		ReselectState();
		b_need_reselect = false;
	}

	// ��������� �������
	cur_state->Execute(m_dwCurrentTime);

	// ���� ������� ��������� ����������� �� ���� update
	if (cur_state->IsCompleted()) {
		
		// ��������� ���������� ���������
		if (cur_state == stateRunAround) prev_state_run_around = true;
		else prev_state_run_around = false;
		
		cur_state->Done();
		cur_state = 0;
		b_need_reselect = true;
	}

	
#ifdef DEBUG
	string128 s;
	strcpy(s, "No State!");

	if (cur_state == stateTele) {
		strcpy(s,"State: Tele");
	} else if (cur_state == stateGravi){
		strcpy(s,"State: Gravi");
	} else if (cur_state == stateMelee){
		strcpy(s,"State: Melee");
	} else if (cur_state == stateRunAround){
		strcpy(s,"State: Run Around");
	} else if (cur_state == stateFaceEnemy){
		strcpy(s,"State: Face Enemy");
	} 

	pMonster->HDebug->M_Clear();
	pMonster->HDebug->M_Add(0,s, D3DCOLOR_XRGB(0,255,255));
#endif



}


void CBurerAttack::Done()
{
	inherited::Done();
	
	pMonster->StopGraviPrepare();
	pMonster->MotionMan.TA_Deactivate();
	pMonster->CTelekinesis::Deactivate();
	
	if (cur_state) {
		cur_state->Done();
		cur_state->Reset();
	}
}

void CBurerAttack::select_state(IState *state)
{
	if (cur_state != state) {
		
		if (cur_state) {
			if (cur_state->IsCompleted()) cur_state->Done();
			else cur_state->CriticalInterrupt();
		
			cur_state->Reset();
		}

		cur_state = state;
		cur_state->Activate();
	}
}

void CBurerAttack::ReselectState()
{
	if (stateMelee->CheckStartCondition()) {
		select_state(stateMelee);
		return;
	}

	bool enable_gravi	= stateGravi->CheckStartCondition();
	bool enable_tele	= stateTele->CheckStartCondition();
		
	if (!enable_gravi && !enable_tele) {
		if (prev_state_run_around) 
			select_state(stateFaceEnemy);
		else 	
			select_state(stateRunAround);
		return;
	}

	if (enable_gravi && enable_tele) {
	
		u32 rnd_val = ::Random.randI(GRAVI_PERCENT + TELE_PERCENT + RUN_AROUND_PERCENT);
		u32 cur_val = GRAVI_PERCENT;
		
		if (rnd_val < cur_val) {
			select_state(stateGravi);
			return;
		}
		
		cur_val += TELE_PERCENT;
		if (rnd_val < cur_val) {
			select_state(stateTele);
			return;
		}

		select_state(stateRunAround);
		return;
	}

	if (prev_state_run_around) {
		if (enable_gravi) select_state(stateGravi);
		else select_state(stateTele);
	} else {
		select_state(stateRunAround);
	}
}
