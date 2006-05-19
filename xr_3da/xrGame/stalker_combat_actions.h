////////////////////////////////////////////////////////////////////////////
//	Module 		: stalker_combat_actions.h
//	Created 	: 25.03.2004
//  Modified 	: 26.03.2004
//	Author		: Dmitriy Iassenev
//	Description : Stalker combat action classes
////////////////////////////////////////////////////////////////////////////

#pragma once

#include "stalker_base_action.h"
#include "random32.h"
#include "stalker_combat_config.h"

class CCoverPoint;

namespace MonsterSpace {
	enum EBodyState;
	enum EMovementType;
}

//////////////////////////////////////////////////////////////////////////
// CStalkerActionCombatBase
//////////////////////////////////////////////////////////////////////////

class CStalkerActionCombatBase : public CStalkerActionBase {
protected:
	typedef CStalkerActionBase inherited;

public:
						CStalkerActionCombatBase	(CAI_Stalker *object, LPCSTR action_name = "");
	virtual void		initialize					();
	virtual void		finalize					();
			void		select_queue_params			(const float &distance, u32 &min_queue_size, u32 &max_queue_size, u32 &min_queue_interval, u32 &max_queue_interval) const;
			bool		fire_make_sense				() const;
};

//////////////////////////////////////////////////////////////////////////
// CStalkerActionGetItemToKill
//////////////////////////////////////////////////////////////////////////

class CStalkerActionGetItemToKill : public CStalkerActionCombatBase {
protected:
	typedef CStalkerActionCombatBase inherited;

public:
						CStalkerActionGetItemToKill	(CAI_Stalker *object, LPCSTR action_name = "");
	virtual void		initialize					();
	virtual void		execute						();
	virtual void		finalize					();
};

//////////////////////////////////////////////////////////////////////////
// CStalkerActionMakeItemKilling
//////////////////////////////////////////////////////////////////////////

class CStalkerActionMakeItemKilling : public CStalkerActionCombatBase {
protected:
	typedef CStalkerActionCombatBase inherited;

public:
						CStalkerActionMakeItemKilling	(CAI_Stalker *object, LPCSTR action_name = "");
	virtual void		initialize						();
	virtual void		execute							();
	virtual void		finalize						();
};

//////////////////////////////////////////////////////////////////////////
// CStalkerActionRetreatFromEnemy
//////////////////////////////////////////////////////////////////////////

class CStalkerActionRetreatFromEnemy : public CStalkerActionCombatBase {
protected:
	typedef CStalkerActionCombatBase inherited;

public:
						CStalkerActionRetreatFromEnemy	(CAI_Stalker *object, LPCSTR action_name = "");
	virtual void		initialize						();
	virtual void		execute							();
	virtual void		finalize						();
	virtual _edge_value_type	weight					(const CSConditionState &condition0, const CSConditionState &condition1) const;
};

//////////////////////////////////////////////////////////////////////////
// CStalkerActionGetReadyToKill
//////////////////////////////////////////////////////////////////////////

class CStalkerActionGetReadyToKill : public CStalkerActionCombatBase {
protected:
	typedef CStalkerActionCombatBase inherited;

private:
	MonsterSpace::EBodyState		m_body_state;
	MonsterSpace::EMovementType		m_movement_type;
	bool							m_affect_properties;

public:
						CStalkerActionGetReadyToKill(bool affect_properties, CAI_Stalker *object, LPCSTR action_name = "");
	virtual void		initialize					();
	virtual void		execute						();
	virtual void		finalize					();
};

//////////////////////////////////////////////////////////////////////////
// CStalkerActionKillEnemy
//////////////////////////////////////////////////////////////////////////

class CStalkerActionKillEnemy : public CStalkerActionCombatBase {
protected:
	typedef CStalkerActionCombatBase inherited;

public:
						CStalkerActionKillEnemy		(CAI_Stalker *object, LPCSTR action_name = "");
	virtual void		initialize					();
	virtual void		execute						();
	virtual void		finalize					();
};

//////////////////////////////////////////////////////////////////////////
// CStalkerActionTakeCover
//////////////////////////////////////////////////////////////////////////

class CStalkerActionTakeCover : public CStalkerActionCombatBase {
protected:
	typedef CStalkerActionCombatBase inherited;

private:
	MonsterSpace::EBodyState		m_body_state;
	MonsterSpace::EMovementType		m_movement_type;

public:
						CStalkerActionTakeCover		(CAI_Stalker *object, LPCSTR action_name = "");
	virtual void		initialize						();
	virtual void		execute							();
	virtual void		finalize						();
};

//////////////////////////////////////////////////////////////////////////
// CStalkerActionLookOut
//////////////////////////////////////////////////////////////////////////

class CStalkerActionLookOut : public CStalkerActionCombatBase {
protected:
	typedef CStalkerActionCombatBase inherited;

private:
	u32					m_last_change_time;
	CRandom32			m_crouch_look_out_random;

public:
						CStalkerActionLookOut		(CAI_Stalker *object, LPCSTR action_name = "");
	virtual void		initialize					();
	virtual void		execute						();
	virtual void		finalize					();
};

//////////////////////////////////////////////////////////////////////////
// CStalkerActionHoldPosition
//////////////////////////////////////////////////////////////////////////

class CStalkerActionHoldPosition : public CStalkerActionCombatBase {
protected:
	typedef CStalkerActionCombatBase inherited;

public:
						CStalkerActionHoldPosition	(CAI_Stalker *object, LPCSTR action_name = "");
	virtual void		initialize					();
	virtual void		execute						();
	virtual void		finalize					();
};

//////////////////////////////////////////////////////////////////////////
// CStalkerActionGetDistance
//////////////////////////////////////////////////////////////////////////

class CStalkerActionGetDistance : public CStalkerActionCombatBase {
protected:
	typedef CStalkerActionCombatBase inherited;

public:
						CStalkerActionGetDistance	(CAI_Stalker *object, LPCSTR action_name = "");
	virtual void		initialize					();
	virtual void		execute						();
	virtual void		finalize					();
};

//////////////////////////////////////////////////////////////////////////
// CStalkerActionDetourEnemy
//////////////////////////////////////////////////////////////////////////

class CStalkerActionDetourEnemy : public CStalkerActionCombatBase {
private:
	const CCoverPoint	*m_enemy_cover;

protected:
	typedef CStalkerActionCombatBase inherited;

public:
						CStalkerActionDetourEnemy	(CAI_Stalker *object, LPCSTR action_name = "");
	virtual void		initialize					();
	virtual void		execute						();
	virtual void		finalize					();
};

//////////////////////////////////////////////////////////////////////////
// CStalkerActionSearchEnemy
//////////////////////////////////////////////////////////////////////////

class CStalkerActionSearchEnemy : public CStalkerActionCombatBase {
private:
	const CCoverPoint	*m_enemy_cover;

protected:
	typedef CStalkerActionCombatBase inherited;

public:
						CStalkerActionSearchEnemy	(CAI_Stalker *object, LPCSTR action_name = "");
	virtual void		initialize					();
	virtual void		execute						();
	virtual void		finalize					();
};


//////////////////////////////////////////////////////////////////////////
// CStalkerActionPostCombatWait
//////////////////////////////////////////////////////////////////////////

class CStalkerActionPostCombatWait : public CStalkerActionCombatBase {
protected:
	typedef CStalkerActionCombatBase inherited;

public:
						CStalkerActionPostCombatWait(CAI_Stalker *object, LPCSTR action_name = "");
	virtual void		initialize					();
	virtual void		execute						();
	virtual void		finalize					();
};

//////////////////////////////////////////////////////////////////////////
// CStalkerActionHideFromGrenade
//////////////////////////////////////////////////////////////////////////

class CStalkerActionHideFromGrenade : public CStalkerActionCombatBase {
protected:
	typedef CStalkerActionCombatBase inherited;

public:
						CStalkerActionHideFromGrenade(CAI_Stalker *object, LPCSTR action_name = "");
	virtual void		initialize					();
	virtual void		execute						();
	virtual void		finalize					();
};

//////////////////////////////////////////////////////////////////////////
// CStalkerActionSuddenAttack
//////////////////////////////////////////////////////////////////////////

class CStalkerActionSuddenAttack : public CStalkerActionCombatBase {
protected:
	typedef CStalkerActionCombatBase inherited;

public:
						CStalkerActionSuddenAttack	(CAI_Stalker *object, LPCSTR action_name = "");
	virtual void		initialize					();
	virtual void		execute						();
	virtual void		finalize					();
};

//////////////////////////////////////////////////////////////////////////
// CStalkerActionKillWounded
//////////////////////////////////////////////////////////////////////////

class CStalkerActionKillWounded : public CStalkerActionCombatBase {
protected:
	typedef CStalkerActionCombatBase inherited;

public:
						CStalkerActionKillWounded	(CAI_Stalker *object, LPCSTR action_name = "");
	virtual void		initialize					();
	virtual void		execute						();
	virtual void		finalize					();
};

#include "stalker_combat_actions_inline.h"