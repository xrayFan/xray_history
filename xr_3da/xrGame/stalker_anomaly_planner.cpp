////////////////////////////////////////////////////////////////////////////
//	Module 		: stalker_anomaly_planner.cpp
//	Created 	: 25.03.2004
//  Modified 	: 27.09.2004
//	Author		: Dmitriy Iassenev
//	Description : Stalker anomaly planner
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "stalker_anomaly_planner.h"
#include "stalker_anomaly_actions.h"
#include "stalker_decision_space.h"
#include "stalker_property_evaluators.h"
#include "ai/stalker/ai_stalker.h"
#include "script_game_object.h"

using namespace StalkerDecisionSpace;

CStalkerAnomalyPlanner::CStalkerAnomalyPlanner	(CAI_Stalker *object, LPCSTR action_name) :
	inherited									(object,action_name)
{
}

CStalkerAnomalyPlanner::~CStalkerAnomalyPlanner	()
{
}

void CStalkerAnomalyPlanner::reinit				(CAI_Stalker *object, CPropertyStorage *storage, bool clear_all)
{
	inherited::reinit		(object,storage,clear_all);
	CScriptActionPlanner::m_storage.set_property	(eWorldPropertyAnomaly,false);
	CScriptActionBase::m_storage->set_property		(eWorldPropertyAnomaly,false);
}

void CStalkerAnomalyPlanner::reload				(LPCSTR section)
{
	inherited::reload		(section);
	clear					();
	add_evaluators			();
	add_actions				();
}

void CStalkerAnomalyPlanner::add_evaluators		()
{
	add_evaluator			(eWorldPropertyInsideAnomaly	,xr_new<CStalkerPropertyEvaluatorInsideAnomaly>		());
	add_evaluator			(eWorldPropertyAnomaly			,xr_new<CStalkerPropertyEvaluatorAnomaly>			());
}

void CStalkerAnomalyPlanner::add_actions		()
{
	CStalkerActionBase		*action;

	action					= xr_new<CStalkerActionGetOutOfAnomaly>	(m_object,"get_out_of_anomaly");
	add_condition			(action,eWorldPropertyInsideAnomaly,true);
	add_effect				(action,eWorldPropertyInsideAnomaly,false);
	add_operator			(eWorldOperatorGetOutOfAnomaly,		action);

	action					= xr_new<CStalkerActionDetectAnomaly>	(m_object,"detect_anomaly");
	add_condition			(action,eWorldPropertyInsideAnomaly,false);
	add_condition			(action,eWorldPropertyAnomaly,		true);
	add_effect				(action,eWorldPropertyAnomaly,		false);
	add_operator			(eWorldOperatorDetectAnomaly,		action);
}

void CStalkerAnomalyPlanner::update				()
{
	inherited::update		();
	CScriptActionBase::m_storage->set_property	(eWorldPropertyAnomaly,CScriptActionPlanner::m_storage.property(eWorldPropertyAnomaly));
}
