////////////////////////////////////////////////////////////////////////////
//	Module 		: ai_rat_animations.cpp
//	Created 	: 21.06.2002
//  Modified 	: 06.11.2002
//	Author		: Dmitriy Iassenev
//	Description : Animations, Bone transformations and Sounds for monster "Rat"
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ai_rat.h"

// sounds
void CAI_Rat::vfLoadSounds()
{
	::Sound->create(m_tpaSoundHit[0],TRUE,"monsters\\rat\\rat_1",SOUND_TYPE_MONSTER_INJURING);
	::Sound->create(m_tpaSoundDie[0],TRUE,"monsters\\rat\\rat_2",SOUND_TYPE_MONSTER_DYING);
	::Sound->create(m_tpaSoundAttack[0],TRUE,"monsters\\rat\\rat_2",SOUND_TYPE_MONSTER_ATTACKING);
	::Sound->create(m_tpaSoundVoice[0],TRUE,"monsters\\rat\\rat_1",SOUND_TYPE_MONSTER_TALKING);
	::Sound->create(m_tpaSoundVoice[1],TRUE,"monsters\\rat\\rat_2",SOUND_TYPE_MONSTER_TALKING);
}

// animations
void CAI_Rat::vfLoadAnimations()
{
	CSkeletonAnimated* tpVisualObject = PSkeletonAnimated(Visual());
	
	// loading normal animations
	m_tRatAnimations.tNormal.tGlobal.tpaDeath[0] = tpVisualObject->ID_Cycle("norm_death");
	m_tRatAnimations.tNormal.tGlobal.tpaDeath[1] = tpVisualObject->ID_Cycle("norm_death_2");
	
	m_tRatAnimations.tNormal.tGlobal.tpaAttack[0] = tpVisualObject->ID_Cycle("attack_1");
	m_tRatAnimations.tNormal.tGlobal.tpaAttack[1] = tpVisualObject->ID_Cycle("attack_2");
	m_tRatAnimations.tNormal.tGlobal.tpaAttack[2] = tpVisualObject->ID_Cycle("attack_3");
	
	m_tRatAnimations.tNormal.tGlobal.tpaIdle[0] = tpVisualObject->ID_Cycle("norm_idle_1");
	m_tRatAnimations.tNormal.tGlobal.tpaIdle[1] = tpVisualObject->ID_Cycle("norm_idle_2");
	
	m_tRatAnimations.tNormal.tGlobal.tpTurnLeft = tpVisualObject->ID_Cycle("norm_turn_ls");
	m_tRatAnimations.tNormal.tGlobal.tpTurnRight = tpVisualObject->ID_Cycle("norm_turn_rs");
	
	m_tRatAnimations.tNormal.tGlobal.tWalk.Create(tpVisualObject, "norm_walk");
	
	m_tRatAnimations.tNormal.tGlobal.tRun.Create(tpVisualObject, "norm_run");
	m_tRatAnimations.tNormal.tGlobal.tRunAttack = tpVisualObject->ID_Cycle("norm_run_fwd_1");
	
	tpVisualObject->PlayCycle(m_tRatAnimations.tNormal.tGlobal.tpaIdle[0]);
}

void CAI_Rat::SelectAnimation(const Fvector& /**_view/**/, const Fvector& /**_move/**/, float /**speed/**/)
{
	CSkeletonAnimated* tpVisualObject = PSkeletonAnimated(Visual());
	CMotionDef*	tpGlobalAnimation=0;

	if (fEntityHealth <= 0) {
		for (int i=0 ;i<2; ++i)
			if (m_tRatAnimations.tNormal.tGlobal.tpaDeath[i] == m_tpCurrentGlobalAnimation) {
				tpGlobalAnimation = m_tpCurrentGlobalAnimation;
				break;
			}
		if (!tpGlobalAnimation)
			if (m_tpCurrentGlobalAnimation == m_tRatAnimations.tNormal.tGlobal.tpaIdle[1])
				tpGlobalAnimation = m_tRatAnimations.tNormal.tGlobal.tpaDeath[0];
			else
				tpGlobalAnimation = m_tRatAnimations.tNormal.tGlobal.tpaDeath[::Random.randI(0,2)];
	}
	else
		if (m_bFiring) {
//			for (int i=0 ;i<3; ++i)
//				if (m_tRatAnimations.tNormal.tGlobal.tpaAttack[i] == m_tpCurrentGlobalAnimation) {
//					tpGlobalAnimation = m_tpCurrentGlobalAnimation;
//					break;
//				}
//			
//			if (!tpGlobalAnimation || !m_tpCurrentGlobalBlend || !m_tpCurrentGlobalBlend->playing)
//				tpGlobalAnimation = m_tRatAnimations.tNormal.tGlobal.tpaAttack[::Random.randI(0,3)];
			tpGlobalAnimation = m_tRatAnimations.tNormal.tGlobal.tpaAttack[2];
		}
		else
			if (_abs(m_body.target.yaw - m_body.current.yaw) <= PI)
				if (_abs(m_body.target.yaw - m_body.current.yaw) >= EPS_L)
					if (m_body.target.yaw - m_body.current.yaw >= 0)
						tpGlobalAnimation = m_tRatAnimations.tNormal.tGlobal.tpTurnRight;
					else
						tpGlobalAnimation = m_tRatAnimations.tNormal.tGlobal.tpTurnLeft;
				else
					if (m_fSpeed < 0.2f) {
						if (m_bStanding)
							tpGlobalAnimation = m_tRatAnimations.tNormal.tGlobal.tpaIdle[1];
						else
							tpGlobalAnimation = m_tRatAnimations.tNormal.tGlobal.tpaIdle[0];
					}
					else
						if (_abs(m_fSpeed - m_fAttackSpeed) < EPS_L)
							tpGlobalAnimation = m_tRatAnimations.tNormal.tGlobal.tRunAttack;
						else
							if (_abs(m_fSpeed - m_fMaxSpeed) < EPS_L)
								tpGlobalAnimation = m_tRatAnimations.tNormal.tGlobal.tRun.fwd;
							else
								tpGlobalAnimation = m_tRatAnimations.tNormal.tGlobal.tWalk.fwd;
			else
				if (PI_MUL_2 - _abs(m_body.target.yaw - m_body.current.yaw) >= EPS_L)
					if (m_body.target.yaw > m_body.current.yaw)
						tpGlobalAnimation = m_tRatAnimations.tNormal.tGlobal.tpTurnLeft;
					else
						tpGlobalAnimation = m_tRatAnimations.tNormal.tGlobal.tpTurnRight;
				else
					if (m_fSpeed < 0.2f) {
						if (m_bStanding)
							tpGlobalAnimation = m_tRatAnimations.tNormal.tGlobal.tpaIdle[1];
						else
							tpGlobalAnimation = m_tRatAnimations.tNormal.tGlobal.tpaIdle[0];
					}
					else
						if (_abs(m_fSpeed - m_fAttackSpeed) < EPS_L)
							tpGlobalAnimation = m_tRatAnimations.tNormal.tGlobal.tRunAttack;
						else
							if (_abs(m_fSpeed - m_fMaxSpeed) < EPS_L)
								tpGlobalAnimation = m_tRatAnimations.tNormal.tGlobal.tRun.fwd;
							else
								tpGlobalAnimation = m_tRatAnimations.tNormal.tGlobal.tWalk.fwd;
	
	if (tpGlobalAnimation != m_tpCurrentGlobalAnimation) { 
		m_tpCurrentGlobalAnimation = tpGlobalAnimation;
		if (tpGlobalAnimation) {
			m_tpCurrentGlobalBlend = tpVisualObject->PlayCycle(tpGlobalAnimation);
		}
	}
}