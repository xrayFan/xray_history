#pragma once
#include "../../state.h"
#include "state_data.h"

template<typename _Object>
class CStateMonsterLookToUnprotectedArea : public CState<_Object> {
	typedef CState<_Object> inherited;

	SStateDataAction	data;

	Fvector				target_point;	

public:
						CStateMonsterLookToUnprotectedArea	(_Object *obj);
	virtual				~CStateMonsterLookToUnprotectedArea	();

	virtual void		initialize					();
	virtual	void		execute						();

	virtual bool		check_completion			();
};

#include "state_look_unprotected_area_inline.h"