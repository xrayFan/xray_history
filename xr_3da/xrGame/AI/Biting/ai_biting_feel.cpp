////////////////////////////////////////////////////////////////////////////
//	Module 		: ai_biting_feel.cpp
//	Created 	: 26.05.2003
//  Modified 	: 26.05.2003
//	Author		: Serge Zhem
//	Description : Visibility and look for all the biting monsters
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ai_biting.h"

// ���������� ������ � ����������� ��������
void CAI_Biting::SetDirectionLook()
{
	int i = ps_Size();		// position stack size
	if (i > 1) {
		CObject::SavedPosition tPreviousPosition = ps_Element(i - 2), tCurrentPosition = ps_Element(i - 1);
		tWatchDirection.sub(tCurrentPosition.vPosition,tPreviousPosition.vPosition);
		if (tWatchDirection.magnitude() > EPS_L) {
			tWatchDirection.normalize();
			mk_rotation(tWatchDirection,r_torso_target);
		}
	}
	else
		r_torso_target.pitch = 0;
	r_target = r_torso_target;
}

// ���������� ������ � ����������� ��������
void CAI_Biting::SetReversedDirectionLook()
{
	int i = ps_Size();		// position stack size
	if (i > 1) {
		CObject::SavedPosition tPreviousPosition = ps_Element(i - 2), tCurrentPosition = ps_Element(i - 1);
		tWatchDirection.sub(tPreviousPosition.vPosition,tCurrentPosition.vPosition);
		if (tWatchDirection.magnitude() > EPS_L) {
			tWatchDirection.normalize();
			r_torso_target.yaw += PI;
			mk_rotation(tWatchDirection,r_torso_target);
		}
	}
	else
		r_torso_target.pitch = 0;
	r_target = r_torso_target;
}

void CAI_Biting::feel_sound_new(CObject* who, int eType, const Fvector &Position, float power)
{
	if (!g_Alive())
		return;

	if ((eType & SOUND_TYPE_WEAPON_SHOOTING) == SOUND_TYPE_WEAPON_SHOOTING)
		power = 1.f;//expf(.1f*log(power));

	if (power >= m_fSoundThreshold) {
		if ((this != who) && ((m_tLastSound.dwTime <= m_dwLastUpdateTime) || (m_tLastSound.fPower <= power))) {
			Msg("%s - sound type %x from %s at %d in (%.2f,%.2f,%.2f) with power %.2f",cName(),eType,who ? who->cName() : "world",Level().timeServer(),Position.x,Position.y,Position.z,power);

			m_tLastSound.tpEntity		= dynamic_cast<CEntity *>(who);
			if (m_tLastSound.tpEntity) {
				m_dwLastSoundNodeID			= m_tLastSound.tpEntity->AI_NodeID;
				m_tLastSound.eSoundType		= ESoundTypes(eType);
				m_tLastSound.dwTime			= Level().timeServer();
				m_tLastSound.fPower			= power;
				m_tLastSound.tSavedPosition = Position;
			}

			
			
				

			//float fDistance = (Position.distance_to(vPosition) < 1.f ? 1.f : Position.distance_to(vPosition));
			//			if ((eType & SOUND_TYPE_MONSTER_DYING) == SOUND_TYPE_MONSTER_DYING)
			//				m_fMorale += m_fMoraleDeathQuant;///fDistance;
			//			else


			/*
			if (((eType & SOUND_TYPE_WEAPON_SHOOTING) == SOUND_TYPE_WEAPON_SHOOTING) && (!m_tEnemy.Enemy))
			m_fMorale += m_fMoraleFearQuant;///fDistance;
			else
			if ((eType & SOUND_TYPE_MONSTER_ATTACKING) == SOUND_TYPE_MONSTER_ATTACKING)
			m_fMorale += m_fMoraleSuccessAttackQuant;///fDistance;
			*/
		}
	}
}

void CAI_Biting::feel_touch_new	(CObject* O)
{
	if (!g_Alive()) return;

	CEntity *pEntity = dynamic_cast<CEntity *>(O);

 	if (m_tSavedEnemy && (m_tSavedEnemy->CLS_ID == CLSID_ENTITY) && (pEntity == m_tSavedEnemy)) {
		Fvector tDirection;
		Fvector position_in_bone_space;
		position_in_bone_space.set(0.f,0.f,0.f);
		tDirection.sub(m_tSavedEnemy->Position(),this->Position());
		tDirection.normalize();

		pEntity->Hit(m_fHitPower,tDirection,this,0,position_in_bone_space,0);

		m_tpSoundBeingPlayed = &(m_tpaSoundHit[::Random.randI(SND_HIT_COUNT)]);
		::Sound->play_at_pos(*m_tpSoundBeingPlayed,this,eye_matrix.c);

	}
}


static BOOL __fastcall BitingQualifier(CObject* O, void* P)
{
	if (O->CLS_ID!=CLSID_ENTITY)			
		return FALSE;
	else  {
		CEntityAlive* E = dynamic_cast<CEntityAlive*> (O);
		if (!E) return FALSE;
		if (!E->IsVisibleForAI()) return FALSE; 
		if ((E->g_Team() == *((int*)P)) && (E->g_Alive())) return FALSE;
		return TRUE;
	}
}

objQualifier* CAI_Biting::GetQualifier	()
{
	return(&BitingQualifier);
}
