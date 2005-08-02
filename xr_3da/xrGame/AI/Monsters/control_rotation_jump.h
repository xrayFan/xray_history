#pragma once
#include "control_combase.h"
#include "../../../SkeletonAnimated.h"

struct SControlRotationJumpData : public ControlCom::IComData {
	MotionID		anim_stop_ls,anim_run_ls;
	MotionID		anim_stop_rs,anim_run_rs;
	float			turn_angle;
};

class CControlRotationJump : public CControl_ComCustom<SControlRotationJumpData> {
	typedef CControl_ComCustom<SControlRotationJumpData> inherited;
	u32				m_time_next_rotation_jump;
	
	float			m_target_velocity;
	float			m_start_velocity;
	float			m_accel;
	float			m_dist;
	float			m_time;

	bool			m_right_side;

	enum EStage {
		eStop,
		eRun, 
		eNone
	} m_stage;

	CSkeletonAnimated	*m_skeleton_animated;

public:
	virtual void	reinit					();

	virtual void	on_event				(ControlCom::EEventType, ControlCom::IEventData*);
	virtual void	activate				();
	virtual void	on_release				();
	virtual bool	check_start_conditions	();

private:	
			void	build_line_first		();
			void	build_line_second		();
};

