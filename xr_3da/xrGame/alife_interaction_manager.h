////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_communication_manager.h
//	Created 	: 03.09.2003
//  Modified 	: 14.05.2004
//	Author		: Dmitriy Iassenev
//	Description : ALife communication manager
////////////////////////////////////////////////////////////////////////////

#pragma once

#include "alife_combat_manager.h"
#include "alife_communication_manager.h"

class CALifeInteractionManager : 
	public CALifeCombatManager,
	public CALifeCommunicationManager
{
protected:
	u32								m_inventory_slot_count;

	// temporary buffer for purchased by the particular trader artefacts
	ALife::D_OBJECT_P_MAP			m_temp_objects;

public:
	BOOL_VECTOR						m_temp_marks;
	ALife::WEAPON_P_VECTOR			m_temp_weapons;	

public:
									CALifeInteractionManager	(xrServer *server, LPCSTR section);
	virtual							~CALifeInteractionManager	();
			ALife::ERelationType	tfGetRelationType			(CSE_ALifeMonsterAbstract	*tpALifeMonsterAbstract1,	CSE_ALifeMonsterAbstract*tpALifeMonsterAbstract2);
			void					check_for_interaction		(CSE_ALifeSchedulable		*tpALifeSchedulable);
			void					check_for_interaction		(CSE_ALifeSchedulable		*tpALifeSchedulable,		ALife::_GRAPH_ID		tGraphID);
};