#include "stdafx.h"
#include "base_monster.h"
#include "../ai_monster_squad.h"
#include "../ai_monster_squad_manager.h"
#include "../../../profiler.h"
#include "../critical_action_info.h"
#include "../ai_monster_movement.h"
#include "../state_manager.h"

void CBaseMonster::Think()
{
	START_PROFILE("AI/Base Monster/Think");

	if (!g_Alive())		return;
	if (getDestroy())	return;

	m_current_update						= Device.dwTimeGlobal;

	// ����������������
	InitThink								();
	MotionMan.ScheduledInit					();
	movement().Update_Initialize			();

	// �������� ������
	START_PROFILE("AI/Base Monster/Think/Update Memory");
	UpdateMemory							();
	STOP_PROFILE;

	// �������� �����
	START_PROFILE("AI/Base Monster/Think/Update Squad");
	monster_squad().update					(this);
	STOP_PROFILE;

	// ��������� FSM
	START_PROFILE("AI/Base Monster/Think/FSM");
	update_fsm();
	STOP_PROFILE;	
	
	// ��������� ����
	START_PROFILE("AI/Base Monster/Think/Build Path");
	movement().Update_Execute		();
	STOP_PROFILE;

	// �������� �������� � ������������ � ����� � action
	MotionMan.UpdateScheduled				();

	// ���������� ������� ��������
	movement().Update_Finalize		();

	STOP_PROFILE;
}

void CBaseMonster::update_fsm()
{
	if (CriticalActionInfo->is_fsm_locked()) return;

	StateMan->update				();		
	TranslateActionToPathParams		();
}

