////////////////////////////////////////////////////////////////////////////
//	Module 		: sight_manager.cpp
//	Created 	: 27.12.2003
//  Modified 	: 27.12.2003
//	Author		: Dmitriy Iassenev
//	Description : Sight manager
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sight_manager.h"
#include "custommonster.h"
#include "ai/stalker/ai_stalker.h"
#include "detail_path_manager.h"
#include "level_navigation_graph.h"
#include "stalker_movement_manager.h"
#include "ai/stalker/ai_stalker_space.h"
#include "ai_space.h"
#include "profiler.h"

using namespace StalkerSpace;

//#define SIGHT_DEBUG

CSightManager::CSightManager		(CAI_Stalker *object) :
	inherited					(object)
{
	m_enabled					= true;
	m_turning_in_place			= false;
}

CSightManager::~CSightManager		()
{
}

void CSightManager::Load			(LPCSTR section)
{
}

void CSightManager::reinit			()
{
	inherited::reinit			();
	m_enabled					= true;
	m_turning_in_place			= false;
}

void CSightManager::reload			(LPCSTR section)
{
	m_max_left_angle			= deg2rad(READ_IF_EXISTS(pSettings,r_float,section,"max_left_torso_angle",90.f));
	m_max_right_angle			= deg2rad(READ_IF_EXISTS(pSettings,r_float,section,"max_right_torso_angle",60.f));
}

void CSightManager::SetPointLookAngles(const Fvector &tPosition, float &yaw, float &pitch, const CGameObject *object)
{
	Fvector			tTemp;
	tTemp.sub		(tPosition,m_object->eye_matrix.c);
	tTemp.getHP		(yaw,pitch);
	VERIFY			(_valid(yaw));
	VERIFY			(_valid(pitch));
	yaw				*= -1;
	pitch			*= -1;
}

void CSightManager::SetFirePointLookAngles(const Fvector &tPosition, float &yaw, float &pitch, const CGameObject *object)
{
	Fvector			tTemp;

	if (object && object->use_center_to_aim()) {
		m_object->Center(tTemp);
		tTemp.x			= m_object->Position().x;
		tTemp.z			= m_object->Position().z;
		tTemp.sub		(tPosition,Fvector(tTemp));
		if (fis_zero(tTemp.square_magnitude()))
			tTemp.set	(0.f,0.f,1.f);
	}
	else
		tTemp.sub	(tPosition,m_object->eye_matrix.c);

	tTemp.getHP		(yaw,pitch);
	VERIFY			(_valid(yaw));
	VERIFY			(_valid(pitch));
	yaw				*= -1;
	pitch			*= -1;
}

void CSightManager::SetDirectionLook()
{
	MonsterSpace::SBoneRotation				orientation = object().movement().m_head, body_orientation = object().movement().body_orientation();
	orientation.target						= orientation.current;
	body_orientation.target					= body_orientation.current;
	if (GetDirectionAngles(object().movement().m_head.target.yaw,object().movement().m_head.target.pitch)) {
		object().movement().m_head.target.yaw	*= -1;
		object().movement().m_head.target.pitch	*= 0;//-1;
	}
	else
		object().movement().m_head.target	= object().movement().m_head.current;
	object().movement().m_body.target		= object().movement().m_head.target;
}

void CSightManager::SetLessCoverLook(const CLevelGraph::CVertex *tpNode, bool bDifferenceLook)
{
	SetDirectionLook	();

	if (m_object->movement().detail().path().empty())
		return;

	SetLessCoverLook	(tpNode,MAX_HEAD_TURN_ANGLE,bDifferenceLook);
}

void CSightManager::SetLessCoverLook(const CLevelGraph::CVertex *tpNode, float fMaxHeadTurnAngle, bool bDifferenceLook)
{
	float fAngleOfView		= m_object->eye_fov/180.f*PI, fMaxSquare = -1.f, fBestAngle = object().movement().m_head.target.yaw;

	CLevelGraph::CVertex	*tpNextNode = 0;
	u32						node_id;
	bool bOk = false;
	if (bDifferenceLook && !m_object->movement().detail().path().empty() && (m_object->movement().detail().path().size() - 1 > m_object->movement().detail().curr_travel_point_index())) {
		CLevelGraph::const_iterator	i, e;
		ai().level_graph().begin(tpNode,i,e);
		for ( ; i != e; ++i) {
			node_id			= ai().level_graph().value(tpNode,i);
			if (!ai().level_graph().valid_vertex_id(node_id))
				continue;
			tpNextNode = ai().level_graph().vertex(node_id);
			if (ai().level_graph().inside(tpNextNode,m_object->movement().detail().path()[m_object->movement().detail().curr_travel_point_index() + 1].position)) {
				bOk = true;
				break;
			}
		}
	}

	if (!bDifferenceLook || !bOk) 
		for (float fIncrement = object().movement().m_body.target.yaw - fMaxHeadTurnAngle; fIncrement <= object().movement().m_body.target.yaw + fMaxHeadTurnAngle; fIncrement += 2*fMaxHeadTurnAngle/60.f) {
			float fSquare = ai().level_graph().compute_square(-fIncrement + PI,fAngleOfView,tpNode);
			if (fSquare > fMaxSquare) {
				fMaxSquare = fSquare;
				fBestAngle = fIncrement;
			}
		}
	else {
		float fMaxSquareSingle = -1.f, fSingleIncrement = object().movement().m_head.target.yaw;
		for (float fIncrement = object().movement().m_body.target.yaw - fMaxHeadTurnAngle; fIncrement <= object().movement().m_body.target.yaw + fMaxHeadTurnAngle; fIncrement += 2*fMaxHeadTurnAngle/60.f) {
			float fSquare0 = ai().level_graph().compute_square(-fIncrement + PI,fAngleOfView,tpNode);
			float fSquare1 = ai().level_graph().compute_square(-fIncrement + PI,fAngleOfView,tpNextNode);
			if	(
					(fSquare1 - fSquare0 > fMaxSquare) || 
					(
						fsimilar(fSquare1 - fSquare0,fMaxSquare,EPS_L) && 
						(_abs(fIncrement - object().movement().m_body.target.yaw) < _abs(fBestAngle - object().movement().m_body.target.yaw))
					)
				)
			{
				fMaxSquare = fSquare1 - fSquare0;
				fBestAngle = fIncrement;
			}

			if (fSquare0 > fMaxSquareSingle) {
				fMaxSquareSingle = fSquare0;
				fSingleIncrement = fIncrement;
			}
		}
		if (_sqrt(fMaxSquare) < 0*PI_DIV_6)
			fBestAngle = fSingleIncrement;
	}

	object().movement().m_head.target.yaw = angle_normalize_signed(fBestAngle);
	object().movement().m_head.target.pitch = 0;
	VERIFY					(_valid(object().movement().m_head.target.yaw));
}

bool CSightManager::bfIf_I_SeePosition(Fvector tPosition) const
{
	float				yaw, pitch;
	Fvector				tVector;
	tVector.sub			(tPosition,m_object->Position());
	tVector.getHP		(yaw,pitch);
	yaw					= angle_normalize_signed(-yaw);
	pitch				= angle_normalize_signed(-pitch);
	return				(angle_difference(yaw,object().movement().m_head.current.yaw) <= PI_DIV_6);// && angle_difference(pitch,object().movement().m_head.current.pitch,PI_DIV_6));
}

void CSightManager::vfValidateAngleDependency(float x1, float &x2, float x3)
{
	float	_x2	= angle_normalize_signed(x2 - x1);
	float	_x3	= angle_normalize_signed(x3 - x1);
	if ((_x2*_x3 <= 0) && (_abs(_x2) + _abs(_x3) > PI - EPS_L))
		x2  = x3;
}

bool CSightManager::need_correction	(float x1, float x2, float x3)
{
	float	_x2	= angle_normalize_signed(x2 - x1);
	float	_x3	= angle_normalize_signed(x3 - x1);
	if ((_x2*_x3 <= 0) && (_abs(_x2) + _abs(_x3) > PI - EPS_L))
		return			(true);
	return				(false);
}

void CSightManager::Exec_Look		(float dt)
{
	START_PROFILE("AI/Sight Manager/exec_look")
	
	typedef MonsterSpace::SBoneRotation CBoneRotation;

	CBoneRotation		&body = object().movement().m_body;
	CBoneRotation		&head = object().movement().m_head;

	// normalizing torso angles
	body.current.yaw	= angle_normalize_signed	(body.current.yaw);
	body.current.pitch	= angle_normalize_signed	(body.current.pitch);
	body.target.yaw		= angle_normalize_signed	(body.target.yaw);
	body.target.pitch	= angle_normalize_signed	(body.target.pitch);

	// normalizing head angles
	head.current.yaw	= angle_normalize_signed	(head.current.yaw);
	head.current.pitch	= angle_normalize_signed	(head.current.pitch);
	head.target.yaw		= angle_normalize_signed	(head.target.yaw);
	head.target.pitch	= angle_normalize_signed	(head.target.pitch);

	// updating torso angles
	float				fSpeedFactor = 1.f;

#ifdef SIGHT_DEBUG
//	Msg					("%6d BEFORE BODY [%f] -> [%f]",Device.dwTimeGlobal,object().movement().m_body.current.yaw,object().movement().m_body.target.yaw);
//	Msg					("%6d BEFORE HEAD [%f] -> [%f]",Device.dwTimeGlobal,object().movement().m_head.current.yaw,object().movement().m_head.target.yaw);
#endif

	vfValidateAngleDependency		(body.current.yaw,body.target.yaw,head.current.yaw);
	if (fis_zero(object().movement().speed()))
		vfValidateAngleDependency	(head.current.yaw,head.target.yaw,body.target.yaw);

	m_object->angle_lerp_bounds		(body.current.yaw,body.target.yaw,fSpeedFactor*body.speed,dt);
	m_object->angle_lerp_bounds		(body.current.pitch,body.target.pitch,body.speed,dt);

	m_object->angle_lerp_bounds		(head.current.yaw,head.target.yaw,head.speed,dt);
	m_object->angle_lerp_bounds		(head.current.pitch,head.target.pitch,head.speed,dt);

#ifdef SIGHT_DEBUG
	// normalizing torso angles
	body.current.yaw	= angle_normalize_signed	(body.current.yaw);
	body.current.pitch	= angle_normalize_signed	(body.current.pitch);

	// normalizing head angles
	head.current.yaw	= angle_normalize_signed	(head.current.yaw);
	head.current.pitch	= angle_normalize_signed	(head.current.pitch);

//	Msg					("%6d AFTER  BODY [%f] -> [%f]",Device.dwTimeGlobal,object().movement().m_body.current.yaw,object().movement().m_body.target.yaw);
//	Msg					("%6d AFTER  HEAD [%f] -> [%f]",Device.dwTimeGlobal,object().movement().m_head.current.yaw,object().movement().m_head.target.yaw);
#endif

#if 0
	Fmatrix				mXFORM;
	mXFORM.setHPB		(-body.current.yaw,0,0);
	mXFORM.c.set		(m_object->Position());
	m_object->XFORM().set(mXFORM);
#else
	Fmatrix				&m = m_object->XFORM();
	float				h = -body.current.yaw;
	float				_sh = _sin(h), _ch = _cos(h);
	m.i.set				( _ch,	0.f,	_sh);
	m.j.set				( 0.f,	1.f,	_sh);
	m.k.set				(-_sh,	0.f,	_ch);
#endif
	STOP_PROFILE
}

void CSightManager::setup			(const CSightAction &sight_action)
{
	if (m_actions.size() > 1)
		clear			();
	if (!m_actions.empty() && (*(*m_actions.begin()).second == sight_action))
		return;
	clear				();
	add_action			(0,xr_new<CSightControlAction>(1.f,u32(-1),sight_action));
}

void CSightManager::update			()
{
	START_PROFILE("AI/Sight Manager/update")
	if (enabled()) {
		if (fis_zero(object().movement().speed())) {
			if (!m_turning_in_place) {
				if (angle_difference(object().movement().m_body.current.yaw,object().movement().m_head.current.yaw) > (left_angle(-object().movement().m_head.current.yaw,-object().movement().m_body.current.yaw) ? m_max_left_angle : m_max_right_angle)) {
					m_turning_in_place	= true;
					object().movement().m_body.target.yaw	= object().movement().m_head.current.yaw;
				}
				else {
					object().movement().m_body.target.yaw	= object().movement().m_body.current.yaw;
				}
			}
			else {
				if (angle_difference(object().movement().m_body.current.yaw,object().movement().m_head.current.yaw) > EPS_L) {
					object().movement().m_body.target.yaw	= object().movement().m_head.current.yaw;
				}
				else {
					m_turning_in_place	= false;
					object().movement().m_body.target.yaw	= object().movement().m_body.current.yaw;
				}
			}
		}
		else
			m_turning_in_place			= false;

//		Msg								("%6d : %f,%f",Device.dwTimeGlobal,object().movement().m_head.target.yaw,object().movement().m_head.target.pitch);

		inherited::update				();
	}
	STOP_PROFILE
}

bool CSightManager::GetDirectionAngles				(float &yaw, float &pitch)
{
	if (!object().movement().path().empty() && (m_object->movement().detail().curr_travel_point_index() + 1 < m_object->movement().detail().path().size())) {
		Fvector				t;
		t.sub					(
			object().movement().path()[m_object->movement().detail().curr_travel_point_index() + 1].position,
			object().movement().path()[m_object->movement().detail().curr_travel_point_index()].position
		);
		t.getHP				(yaw,pitch);
		return				(true);
	}
	return					(GetDirectionAnglesByPrevPositions(yaw,pitch));
};

bool CSightManager::GetDirectionAnglesByPrevPositions(float &yaw, float &pitch)
{
	Fvector					tDirection;
	int						i = m_object->ps_Size	();

	if (i < 2) 
		return				(false);

	CObject::SavedPosition	tPreviousPosition = m_object->ps_Element(i - 2), tCurrentPosition = m_object->ps_Element(i - 1);
	VERIFY					(_valid(tPreviousPosition.vPosition));
	VERIFY					(_valid(tCurrentPosition.vPosition));
	tDirection.sub			(tCurrentPosition.vPosition,tPreviousPosition.vPosition);
	if (tDirection.magnitude() < EPS_L)	return(false);
	tDirection.getHP		(yaw,pitch);
	VERIFY					(_valid(yaw));
	VERIFY					(_valid(pitch));

	return					(true);
}

