////////////////////////////////////////////////////////////////////////////
//	Module 		: property_evaluator.h
//	Created 	: 12.03.2004
//  Modified 	: 12.03.2004
//	Author		: Dmitriy Iassenev
//	Description : Property evaluator
////////////////////////////////////////////////////////////////////////////

#pragma once

#include "intrusive_ptr.h"
#include "property_storage.h"
#include "script_export_space.h"

class CScriptGameObject;

template <typename _object_type>
class CPropertyEvaluator : virtual public intrusive_base {
public:
	typedef CGraphEngine::_solver_condition_type	_condition_type;
	typedef CGraphEngine::_solver_value_type		_value_type;

public:
	_object_type		*m_object;
	CPropertyStorage	*m_storage;
	LPCSTR				m_evaluator_name;

public:
	IC							CPropertyEvaluator	(_object_type *object = 0, LPCSTR evaluator_name = "");
	virtual 					~CPropertyEvaluator	();
	IC		void				init				(_object_type *object, LPCSTR evaluator_name);
	virtual void				setup				(_object_type *object, CPropertyStorage *storage);
	virtual void				Load				(LPCSTR section);
	virtual	_value_type			evaluate			();
	IC		const _value_type	&property			(const _condition_type &condition_id) const;
	IC		LPCSTR				name				() const;
	DECLARE_SCRIPT_REGISTER_FUNCTION
};
typedef CPropertyEvaluator<CScriptGameObject> CScriptPropertyEvaluator;
add_to_type_list(CScriptPropertyEvaluator)
#undef script_type_list
#define script_type_list save_type_list(CScriptPropertyEvaluator)

#include "property_evaluator_inline.h"