#include "stdafx.h"
#include "ai_biting.h"
#include "../../ai_script_actions.h"
#include "../../phmovementcontrol.h"
#include "../../sight_manager.h"

//////////////////////////////////////////////////////////////////////////
bool CAI_Biting::bfAssignMovement (CEntityAction *tpEntityAction)
{
	if (!inherited::bfAssignMovement(tpEntityAction))
		return		(false);

	CScriptMovementAction	&l_tMovementAction	= tpEntityAction->m_tMovementAction;
	if (l_tMovementAction.completed()) return false;

	// translate script.action into MotionMan.action
	switch (l_tMovementAction.m_tMoveAction) {
	case eMA_WalkFwd:	MotionMan.m_tAction = ACT_WALK_FWD;		break;
	case eMA_WalkBkwd:	MotionMan.m_tAction = ACT_WALK_BKWD;	break;
	case eMA_Run:		MotionMan.m_tAction = ACT_RUN;			break;
	case eMA_Drag:		MotionMan.m_tAction = ACT_DRAG;			break;
	case eMA_Jump:		MotionMan.m_tAction = ACT_JUMP;			break;
	case eMA_Steal:		MotionMan.m_tAction = ACT_STEAL;		break;
	}

	CMonsterMovement::set_path_targeted();
		
	return			(true);		
}

//////////////////////////////////////////////////////////////////////////

bool CAI_Biting::bfAssignObject(CEntityAction *tpEntityAction)
{
	if (!inherited::bfAssignObject(tpEntityAction))
		return	(false);

//	CScriptObjectAction	&l_tObjectAction = tpEntityAction->m_tObjectAction;
//	if (!l_tObjectAction.m_tpObject)
//		return	(false == (l_tObjectAction.m_bCompleted = true));
//
//	CEntityAlive	*l_tpEntity		= dynamic_cast<CEntityAlive*>(l_tObjectAction.m_tpObject);
//	if (!l_tpEntity) return	(false == (l_tObjectAction.m_bCompleted = true));
//
//	switch (l_tObjectAction.m_tGoalType) {
//		case eObjectActionTake: 
//			m_PhysicMovementControl->PHCaptureObject(l_tpEntity);
//			break;
//		case eObjectActionDrop: 
//			m_PhysicMovementControl->PHReleaseObject();
//			break;
//	}
//	
//	l_tObjectAction.m_bCompleted = true;
	return	(true);
}


bool CAI_Biting::bfAssignWatch(CEntityAction *tpEntityAction)
{
	if (!inherited::bfAssignWatch(tpEntityAction))
		return		(false);
	
	CScriptWatchAction	&l_tWatchAction = tpEntityAction->m_tWatchAction;
	if (l_tWatchAction.completed()) return false;

	Fvector new_pos;
	switch (l_tWatchAction.m_tWatchType) {
		case SightManager::eSightTypePosition:
			LookPosition(l_tWatchAction.m_tWatchVector);
			break;
		case SightManager::eSightTypeDirection:
			new_pos.mad(Position(), l_tWatchAction.m_tWatchVector, 2.f);
			LookPosition(new_pos);
			break;
	}


	if (angle_difference(m_body.target.yaw,m_body.current.yaw) < deg(2))
		l_tWatchAction.m_bCompleted = true;
	else
		l_tWatchAction.m_bCompleted = false;
	
	return		(!l_tWatchAction.m_bCompleted);
}

bool CAI_Biting::bfAssignAnimation(CEntityAction *tpEntityAction)
{
	if (!inherited::bfAssignAnimation(tpEntityAction))
		return			(false);

	CScriptAnimationAction	&l_tAnimAction	= tpEntityAction->m_tAnimationAction;
	if (l_tAnimAction.completed()) return false;
	
	// translate animation.action into MotionMan.action
	switch (l_tAnimAction.m_tAnimAction) {
	case eAA_StandIdle:		MotionMan.m_tAction = ACT_STAND_IDLE;	break;
	case eAA_SitIdle:		MotionMan.m_tAction = ACT_SIT_IDLE;		break;
	case eAA_LieIdle:		MotionMan.m_tAction = ACT_LIE_IDLE;		break;
	case eAA_Eat:			MotionMan.m_tAction = ACT_EAT;			break;
	case eAA_Sleep:			MotionMan.m_tAction = ACT_SLEEP;		break;
	case eAA_Rest:			MotionMan.m_tAction = ACT_REST;			break;
	case eAA_Attack:		MotionMan.m_tAction = ACT_ATTACK;		break;
	case eAA_LookAround:	MotionMan.m_tAction = ACT_LOOK_AROUND;	break;
	case eAA_Turn:			MotionMan.m_tAction = ACT_TURN;			break;
	}

	return				(true);
}

bool CAI_Biting::bfAssignSound(CEntityAction *tpEntityAction)
{
	if (!inherited::bfAssignSound(tpEntityAction))
		return			(false);

	CScriptSoundAction	&l_tAction	= tpEntityAction->m_tSoundAction;
	if (l_tAction.completed()) return false;


	switch (l_tAction.m_monster_sound) {
	case	eMonsterSoundIdle:			CSoundPlayer::play(eMonsterSoundIdle,		0, 0, (l_tAction.m_monster_sound_delay == int(-1)) ? _sd->m_dwIdleSndDelay		: l_tAction.m_monster_sound_delay);		break;
	case 	eMonsterSoundEat:			CSoundPlayer::play(eMonsterSoundEat,		0, 0, (l_tAction.m_monster_sound_delay == int(-1)) ? _sd->m_dwEatSndDelay		: l_tAction.m_monster_sound_delay);		break;
	case 	eMonsterSoundAttack:		CSoundPlayer::play(eMonsterSoundAttack,		0, 0, (l_tAction.m_monster_sound_delay == int(-1)) ? _sd->m_dwAttackSndDelay	: l_tAction.m_monster_sound_delay);		break;
	case	eMonsterSoundAttackHit:		CSoundPlayer::play(eMonsterSoundAttackHit);		break;
	case	eMonsterSoundTakeDamage:	CSoundPlayer::play(eMonsterSoundTakeDamage);	break;
	case	eMonsterSoundDie:			CSoundPlayer::play(eMonsterSoundDie);			break;
	case	eMonsterSoundThreaten:		CSoundPlayer::play(eMonsterSoundThreaten,	0, 0, (l_tAction.m_monster_sound_delay == int(-1)) ? _sd->m_dwAttackSndDelay : l_tAction.m_monster_sound_delay);		break;
	case	eMonsterSoundSteal:			CSoundPlayer::play(eMonsterSoundSteal,		0, 0, (l_tAction.m_monster_sound_delay == int(-1)) ? _sd->m_dwAttackSndDelay : l_tAction.m_monster_sound_delay);		break;
	case	eMonsterSoundPanic:			CSoundPlayer::play(eMonsterSoundPanic,		0, 0, (l_tAction.m_monster_sound_delay == int(-1)) ? _sd->m_dwAttackSndDelay : l_tAction.m_monster_sound_delay);		break;
	case	eMonsterSoundGrowling:		CSoundPlayer::play(eMonsterSoundGrowling,	0, 0, (l_tAction.m_monster_sound_delay == int(-1)) ? _sd->m_dwAttackSndDelay : l_tAction.m_monster_sound_delay);		break;
	}

	return				(true);
}




bool CAI_Biting::bfAssignMonsterAction(CEntityAction *tpEntityAction)
{
	if (!inherited::bfAssignMonsterAction(tpEntityAction)) return false;
	
	CMonsterAction	&l_tAction = tpEntityAction->m_tMonsterAction;	
	if (l_tAction.completed()) return false;

	switch(l_tAction.m_tAction) {
		case eGA_Rest:		
			SetState(stateRest, true);	
			break;
		case eGA_Eat:		
			SetState(stateEat, true);	
			CorpseMan.force_corpse(dynamic_cast<CEntityAlive *>(l_tAction.m_tObject));
			break;
		case eGA_Attack:
			SetState(stateAttack, true);	
			EnemyMan.force_enemy(dynamic_cast<CEntityAlive *>(l_tAction.m_tObject));
			break;
		case eGA_Panic:		
			SetState(statePanic, true);
			EnemyMan.force_enemy(dynamic_cast<CEntityAlive *>(l_tAction.m_tObject));
			break;
	}

	b_script_state_must_execute = true;
	return (!l_tAction.m_bCompleted);
}



void CAI_Biting::ProcessScripts()
{
	// ���������������� action
	MotionMan.m_tAction = ACT_STAND_IDLE;

	CMonsterMovement::Frame_Init				();
	
	// ��������� ���������� actions
	b_script_state_must_execute					= false;
	inherited::ProcessScripts					();
	
	m_dwLastUpdateTime						= m_current_update;
	m_current_update						= Level().timeServer();

	// �������� ��� (������, �����, �������)
	vfUpdateParameters							();
	
	
	MotionMan.accel_deactivate					();

	// ���� �� ������� ������� �������� �� ������������� �����, ��������� ���
	if (b_script_state_must_execute)			
		CurrentState->Execute					(m_current_update);
	
	TranslateActionToPathParams					();

	// �������� ����
	CMonsterMovement::Frame_Update				();

	MotionMan.Update							();
	
	// ���������� ������� ��������
	CMonsterMovement::Frame_Finalize			();

	// ������� ��� ����� � �������, ������� ���� ������������� �����������
	// �� ����� ���������� ����������� ��������
	EnemyMan.unforce_enemy();
	CorpseMan.unforce_corpse();
}

CEntity *CAI_Biting::GetCurrentEnemy()
{
	if (EnemyMan.get_enemy()) return (const_cast<CEntity *>(dynamic_cast<const CEntity*>(EnemyMan.get_enemy())));
	else return (0);
}
CEntity *CAI_Biting::GetCurrentCorpse()
{
	if (CorpseMan.get_corpse()) return (const_cast<CEntity *>(dynamic_cast<const CEntity*>(CorpseMan.get_corpse())));
	return (0);
}


void CAI_Biting::TranslateActionToPathParams()
{
	bool bEnablePath = true;
	u32 vel_mask = 0;
	u32 des_mask = 0;

	switch (MotionMan.m_tAction) {
	case ACT_STAND_IDLE: 
	case ACT_SIT_IDLE:	 
	case ACT_LIE_IDLE:
	case ACT_EAT:
	case ACT_SLEEP:
	case ACT_REST:
	case ACT_LOOK_AROUND:
	case ACT_ATTACK:
	case ACT_TURN:
		bEnablePath = false;
		break;

	case ACT_WALK_FWD:
		if (m_bDamaged) {
			vel_mask = eVelocityParamsWalkDamaged;
			des_mask = eVelocityParameterWalkDamaged;
		} else {
			vel_mask = eVelocityParamsWalk;
			des_mask = eVelocityParameterWalkNormal;
		}
		break;
	case ACT_WALK_BKWD:
		break;
	case ACT_RUN:
		if (m_bDamaged) {
			vel_mask = eVelocityParamsRunDamaged;
			des_mask = eVelocityParameterRunDamaged;
		} else {
			vel_mask = eVelocityParamsRun;
			des_mask = eVelocityParameterRunNormal;
		}
		break;
	case ACT_DRAG:
		vel_mask = eVelocityParameterDrag;
		des_mask = eVelocityParameterDrag;

		MotionMan.SetSpecParams(ASP_MOVE_BKWD);

		break;
	case ACT_STEAL:
		vel_mask = eVelocityParameterSteal;
		des_mask = eVelocityParameterSteal;
		break;
	case ACT_JUMP:
		break;
	}

	if (state_invisible) {
		vel_mask = eVelocityParamsInvisible;
		des_mask = eVelocityParameterInvisible;
	}

	//if (force_real_speed) vel_mask = des_mask;

	if (bEnablePath) {
		set_velocity_mask	(vel_mask);	
		set_desirable_mask	(des_mask);
		enable_path			();		
	} else {
		disable_path		();
	}
}


void CAI_Biting::SetScriptControl(const bool bScriptControl, ref_str caScriptName)
{
	if (CurrentState) CurrentState->Done();

	CScriptMonster::SetScriptControl(bScriptControl, caScriptName);
}


int	CAI_Biting::get_enemy_strength()
{
	if (EnemyMan.get_enemy()) {
		switch (EnemyMan.get_danger_type()) {
			case eVeryStrong	: 	return (4);
			case eStrong		: 	return (3);
			case eNormal		: 	return (2);
			case eWeak			:	return (1);
		}
	}
	
	return (0);
}


