////////////////////////////////////////////////////////////////////////////
//	Module 		: action_script_base_inline.h
//	Created 	: 28.03.2004
//  Modified 	: 28.03.2004
//	Author		: Dmitriy Iassenev
//	Description : Base action with script support (inline functions)
////////////////////////////////////////////////////////////////////////////

#pragma once

#define TEMPLATE_SPECIALIZATION template <typename _object_type>
#define CScriptBaseAction		CActionScriptBase<_object_type>

TEMPLATE_SPECIALIZATION
IC	CScriptBaseAction::CActionScriptBase		(const xr_vector<COperatorCondition> &conditions, const xr_vector<COperatorCondition> &effects, _object_type *object, LPCSTR action_name) :
	inherited			(conditions,effects,object ? object->lua_game_object() : 0,action_name)
{
	m_object			= object;
}

TEMPLATE_SPECIALIZATION
IC	CScriptBaseAction::CActionScriptBase		(_object_type *object, LPCSTR action_name) :
	inherited			(object ? object->lua_game_object() : 0,action_name)
{
	m_object			= object;
}

TEMPLATE_SPECIALIZATION
CScriptBaseAction::~CActionScriptBase		()
{
}

TEMPLATE_SPECIALIZATION
void CScriptBaseAction::reinit		(_object_type *object, bool clear_all)
{
	VERIFY				(object);
	inherited::reinit	(object->lua_game_object(),clear_all);
	m_object			= object;
}

#undef TEMPLATE_SPECIALIZATION
#undef CScriptBaseAction