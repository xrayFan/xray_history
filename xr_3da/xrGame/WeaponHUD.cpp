// WeaponHUD.cpp:	HUD ��� ������ � ������ ���������, �������
//					����� ������� � ����� ���������, ����� ������������
//					��� ������������� �������� � ����� �� 3-�� ����
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "WeaponHUD.h"
#include "Weapon.h"
#include "../Motion.h"
#include "../skeletonanimated.h"

weapon_hud_container* g_pWeaponHUDContainer=0;

BOOL weapon_hud_value::load(const shared_str& section, CHudItem* owner)
{	
	// Geometry and transform
	Fvector						pos,ypr;
	pos							= pSettings->r_fvector3(section,"position");
	ypr							= pSettings->r_fvector3(section,"orientation");
	ypr.mul						(PI/180.f);

	m_offset.setHPB				(ypr.x,ypr.y,ypr.z);
	m_offset.translate_over		(pos);

	// Visual
	LPCSTR visual_name			= pSettings->r_string(section, "visual");
	m_animations				= smart_cast<CSkeletonAnimated*>(::Render->model_Create(visual_name));
	//	R_ASSERT					(pVisual->Type==MT_SKELETON_ANIM);

	// fire bone	
	if(smart_cast<CWeapon*>(owner)){
		LPCSTR fire_bone		= pSettings->r_string					(section,"fire_bone");
		m_fire_bone				= m_animations->LL_BoneID	(fire_bone);
		if (m_fire_bone>=m_animations->LL_BoneCount())	
			Debug.fatal	("There is no '%s' bone for weapon '%s'.",fire_bone, *section);
		m_fp_offset				= pSettings->r_fvector3					(section,"fire_point");
		if(pSettings->line_exist(section,"fire_point2")) 
			m_fp2_offset		= pSettings->r_fvector3					(section,"fire_point2");
		else 
			m_fp2_offset		= m_fp_offset;
		if(pSettings->line_exist(owner->object().cNameSect(), "shell_particles")) 
			m_sp_offset			= pSettings->r_fvector3	(section,"shell_point");
		else 
			m_sp_offset.set		(0,0,0);
	}else{
		m_fire_bone				= -1;
		m_fp_offset.set			(0,0,0);
		m_fp2_offset.set		(0,0,0);
		m_sp_offset.set			(0,0,0);
	}
	return TRUE;
}

weapon_hud_value::~weapon_hud_value()
{
	::Render->model_Delete		((IRender_Visual*&)m_animations);
}

u32 shared_weapon_hud::motion_length(MotionID M)
{
	CSkeletonAnimated	*skeleton_animated = p_->m_animations;
	VERIFY				(skeleton_animated);
	CMotionDef			*motion_def = skeleton_animated->LL_GetMotionDef(M);
	VERIFY				(motion_def);

	if (motion_def->flags & esmStopAtEnd) {
		CBoneData			&bone_data = skeleton_animated->LL_GetData(skeleton_animated->LL_GetBoneRoot());
		CBoneDataAnimated	*bone_anim = smart_cast<CBoneDataAnimated *>(&bone_data);
		CMotion				&motion = bone_anim->Motions[M.slot]->at(M.idx);
		return				iFloor(0.5f + 1000.f*motion.GetLength()/ motion_def->Dequantize(motion_def->speed));
	}
	return				0;
}

MotionID shared_weapon_hud::motion_id(LPCSTR name)
{
	return p_->m_animations->ID_Cycle_Safe(name);
}
//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CWeaponHUD::CWeaponHUD			(CHudItem* pHudItem)
{
	m_bVisible					= false;
	m_pParentWeapon				= pHudItem;
	m_bHidden					= true;
	m_bStopAtEndAnimIsRunning	= false;
	m_pCallbackItem				= NULL;
	m_Transform.identity		();
}

CWeaponHUD::~CWeaponHUD()
{
}

void CWeaponHUD::Load			(LPCSTR section)
{
	m_shared_data.create		(section,m_pParentWeapon);
}

void  CWeaponHUD::Init			()
{
	m_bStopAtEndAnimIsRunning	= false;
	m_pCallbackItem				= NULL;
}


void  CWeaponHUD::net_DestroyHud()
{
	m_bStopAtEndAnimIsRunning	= false;
	m_pCallbackItem				= NULL;
	Visible						(false);
}

void CWeaponHUD::UpdatePosition(const Fmatrix& trans)
{
	m_Transform.mul				(trans,m_shared_data.get_value()->m_offset);
	VERIFY						(!fis_zero(DET(m_Transform)));
}

MotionID CWeaponHUD::animGet		(LPCSTR name)
{
	return m_shared_data.motion_id	(name);
}

void CWeaponHUD::animDisplay		(MotionID M,	BOOL bMixIn)
{
	if(m_bVisible){
		CSkeletonAnimated* pSkeletonAnimated			= smart_cast<CSkeletonAnimated*>(Visual());
		VERIFY(pSkeletonAnimated);
		//pSkeletonAnimated->Update						();
		pSkeletonAnimated->PlayCycle					(M,bMixIn);
		pSkeletonAnimated->CalculateBones_Invalidate	();
	}
}
void CWeaponHUD::animPlay			(MotionID M,	BOOL bMixIn, CInventoryItem* W)
{
	Show							();
	animDisplay						(M, bMixIn);
	u32 anim_time					= m_shared_data.motion_length(M);
	if (anim_time>0){
		m_bStopAtEndAnimIsRunning	= true;
		m_pCallbackItem				= W;
		m_dwAnimEndTime				= Device.dwTimeGlobal + anim_time;
	}else{
		m_pCallbackItem				= NULL;
	}
}

void CWeaponHUD::Update				()
{
	if(m_bStopAtEndAnimIsRunning && Device.dwTimeGlobal > m_dwAnimEndTime)
		StopCurrentAnim				();
	if(m_bVisible)
		smart_cast<CSkeletonAnimated*>(Visual())->UpdateTracks		();
}

void CWeaponHUD::StopCurrentAnim	()
{
	m_dwAnimEndTime = 0;
	m_bStopAtEndAnimIsRunning = false;
	if(m_pCallbackItem)
		m_pCallbackItem->OnAnimationEnd();
}

void CWeaponHUD::StopCurrentAnimWithoutCallback		()
{
	m_dwAnimEndTime = 0;
	m_bStopAtEndAnimIsRunning = false;

	m_pCallbackItem = NULL;
}

void CWeaponHUD::CreateSharedContainer	()
{
	VERIFY(0==g_pWeaponHUDContainer);
	g_pWeaponHUDContainer	= xr_new<weapon_hud_container>();
}
void CWeaponHUD::DestroySharedContainer	()
{
	xr_delete				(g_pWeaponHUDContainer);
}
void CWeaponHUD::CleanSharedContainer	()
{
	VERIFY(g_pWeaponHUDContainer);
	g_pWeaponHUDContainer->clean(false);
}
