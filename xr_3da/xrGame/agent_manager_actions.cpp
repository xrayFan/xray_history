////////////////////////////////////////////////////////////////////////////
//	Module 		: agent_manager_actions.cpp
//	Created 	: 25.05.2004
//  Modified 	: 25.05.2004
//	Author		: Dmitriy Iassenev
//	Description : Agent manager actions
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "agent_manager_actions.h"
#include "agent_manager.h"
#include "ai/stalker/ai_stalker.h"

//////////////////////////////////////////////////////////////////////////
// CAgentManagerActionBase
//////////////////////////////////////////////////////////////////////////

CAgentManagerActionBase::CAgentManagerActionBase	(CAgentManager *object, LPCSTR action_name) :
	inherited	(object,action_name)
{
}

void CAgentManagerActionBase::initialize			()
{
}

void CAgentManagerActionBase::finalize				()
{
}

void CAgentManagerActionBase::execute				()
{
}

//////////////////////////////////////////////////////////////////////////
// CAgentManagerActionNoOrders
//////////////////////////////////////////////////////////////////////////

CAgentManagerActionNoOrders::CAgentManagerActionNoOrders	(CAgentManager *object, LPCSTR action_name) :
	inherited		(object,action_name)
{
}

void CAgentManagerActionNoOrders::initialize		()
{
}

void CAgentManagerActionNoOrders::finalize			()
{
}

void CAgentManagerActionNoOrders::execute			()
{
//	CGraphEngine::CWorldState	goal;
//	goal.add_condition			(CGraphEngine::CWorldProperty(StalkerDecisionSpace::eWorldPropertyEnemy,true));
	CAgentManager::iterator		I = m_object->members().begin();
	CAgentManager::iterator		E = m_object->members().end();
	for ( ; I != E; ++I) {
		(*I).order_type			(AgentManager::eOrderTypeNoOrder);

//		(*I).order_type			(AgentManager::eOrderTypeGoal);
//		(*I).goal				(goal);

//		(*I).order_type			(AgentManager::eOrderTypeAction);
//		(*I).action				(CSetupAction(0.f,0));
//		(*I).action().movement().set_level_dest_vertex_id((*I).object()->level_vertex_id());
	}
}

//////////////////////////////////////////////////////////////////////////
// CAgentManagerActionGatherItems
//////////////////////////////////////////////////////////////////////////

CAgentManagerActionGatherItems::CAgentManagerActionGatherItems	(CAgentManager *object, LPCSTR action_name) :
	inherited		(object,action_name)
{
}

void CAgentManagerActionGatherItems::initialize		()
{
}

void CAgentManagerActionGatherItems::finalize			()
{
}

void CAgentManagerActionGatherItems::execute			()
{
	CAgentManager::iterator		I = m_object->members().begin();
	CAgentManager::iterator		E = m_object->members().end();
	for ( ; I != E; ++I)
		(*I).order_type			(AgentManager::eOrderTypeNoOrder);
}

//////////////////////////////////////////////////////////////////////////
// CAgentManagerActionKillEnemy
//////////////////////////////////////////////////////////////////////////

CAgentManagerActionKillEnemy::CAgentManagerActionKillEnemy	(CAgentManager *object, LPCSTR action_name) :
	inherited		(object,action_name)
{
}

void CAgentManagerActionKillEnemy::initialize		()
{
}

void CAgentManagerActionKillEnemy::finalize			()
{
}

void CAgentManagerActionKillEnemy::execute			()
{
	CAgentManager::iterator		I = m_object->members().begin();
	CAgentManager::iterator		E = m_object->members().end();
	for ( ; I != E; ++I)
		(*I).order_type			(AgentManager::eOrderTypeNoOrder);
}
