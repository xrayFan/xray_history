#include "stdafx.h"
#include "Physics.h"
#include "PHObject.h"
#include "PHWorld.h"
#include "PHMoveStorage.h"
#include "dRayMotions.h"
#include "PHCollideValidator.h"
#ifdef DEBUG
#include "phdebug.h"
#endif
extern CPHWorld* ph_world;

CPHObject::CPHObject	()	: ISpatial(g_SpatialSpacePhysic)
{
	m_flags.flags	=	0;
	spatial.type	|=	STYPE_PHYSIC;
	m_island.Init	();
	m_check_count	=0;
	CPHCollideValidator::InitObject	(*this);
}

void CPHObject::activate()
{
	R_ASSERT2(dSpacedGeom(),"trying to activate destroyed or not created object!");
	if(m_flags.test(st_activated))return;
	if(m_flags.test(st_freezed))	{UnFreeze();return;}
	if(m_flags.test(st_recently_deactivated))remove_from_recently_deactivated();
	ph_world->AddObject(this);
	vis_update_activate();
	m_flags.set(st_activated,TRUE);
}

void CPHObject::EnableObject(CPHObject* obj)
{
	activate();
}

void CPHObject::deactivate()
{
	if(!m_flags.test(st_activated))return;
	VERIFY2(m_island.IsActive(),"can not do it during processing");
	ph_world->RemoveObject(PH_OBJECT_I(this));
	vis_update_deactivate();
	m_flags.set(st_activated,FALSE);
}

void CPHObject::put_in_recently_deactivated()
{
	VERIFY(!m_flags.test(st_activated)&&!m_flags.test(st_freezed));
	if(m_flags.test(st_recently_deactivated))return;
	m_check_count=u8(ph_tri_clear_disable_count);
	m_flags.set(st_recently_deactivated,TRUE);
	ph_world->AddRecentlyDisabled(this);
}
void CPHObject::remove_from_recently_deactivated()
{
	if(!m_flags.test(st_recently_deactivated))return;
	m_check_count=0;
	m_flags.set(st_recently_deactivated,FALSE);
	ph_world->RemoveFromRecentlyDisabled(PH_OBJECT_I(this));
}
void CPHObject::check_recently_deactivated()
{
	if(m_check_count==0){
		ClearRecentlyDeactivated();
		remove_from_recently_deactivated();
	}
	 else m_check_count--;
}
void CPHObject::spatial_move()
{
	get_spatial_params();
	ISpatial::spatial_move();
	m_flags.set(st_dirty,TRUE);
}

void CPHObject::Collide()
{
	if(m_flags.test(fl_ray_motions))
	{
		CPHMoveStorage* tracers=MoveStorage();
		CPHMoveStorage::iterator I=tracers->begin(),E=tracers->end();
		for(;E!=I;I++)
		{
				const Fvector	*from=0,	*to=0;
				Fvector dir;
				I.Positions(from,to);
				if(from->x==-dInfinity) continue;
				dir.sub(*to,*from);
				float	magnitude=dir.magnitude();
				if(magnitude<EPS) continue;
				dir.mul(1.f/magnitude);
				g_SpatialSpacePhysic->q_ray(ph_world->r_spatial,0,STYPE_PHYSIC,*from,dir,magnitude);//|ISpatial_DB::O_ONLYFIRST
#ifdef DEBUG
				if(ph_dbg_draw_mask.test(phDbgDrawRayMotions))
				{
					DBG_OpenCashedDraw();
					DBG_DrawLine(*from,Fvector().add(*from,Fvector().mul(dir,magnitude)),D3DCOLOR_XRGB(0,255,0));
					DBG_ClosedCashedDraw(30000);
				}

#endif
				qResultVec& result=ph_world->r_spatial;
				qResultIt i=result.begin(),e=result.end();
				for(;i!=e;++i)	{
					CPHObject* obj2=static_cast<CPHObject*>(*i);
					if(obj2==this || !obj2->m_flags.test(st_dirty))		continue;
					dGeomID	motion_ray=ph_world->GetMotionRayGeom();
					dGeomRayMotionSetGeom(motion_ray,I.dGeom());
					dGeomRayMotionsSet(motion_ray,(const dReal*) from,(const dReal*)&dir,magnitude);
					NearCallback(this,obj2,motion_ray,obj2->dSpacedGeom());
				}
		}
	}

	g_SpatialSpacePhysic->q_box				(ph_world->r_spatial,0,STYPE_PHYSIC,spatial.sphere.P,AABB);
	qResultVec& result=ph_world->r_spatial	;
	qResultIt i=result.begin(),e=result.end();
	for(;i!=e;++i)	{
		CPHObject* obj2=static_cast<CPHObject*>(*i);
		if(obj2==this || !obj2->m_flags.test(st_dirty))		continue;
		if(CPHCollideValidator::DoCollide(*this,*obj2)) NearCallback(this,obj2,dSpacedGeom(),obj2->dSpacedGeom());
	}
	
///////////////////////////////
	if(CPHCollideValidator::DoCollideStatic(*this)) CollideStatic(dSpacedGeom(),this);
	m_flags.set(st_dirty,FALSE);
}

void CPHObject::FreezeContent()
{
	R_ASSERT(!m_flags.test(st_freezed));
	m_flags.set(st_freezed,TRUE);
	m_flags.set(st_activated,FALSE);
	vis_update_deactivate();
}
void CPHObject::UnFreezeContent()
{
	R_ASSERT(m_flags.test(st_freezed));
	m_flags.set(st_freezed,FALSE);
	m_flags.set(st_activated,TRUE);
	vis_update_activate();
}

void CPHObject::spatial_register()
{
	get_spatial_params();
	ISpatial::spatial_register();
	m_flags.set(st_dirty,TRUE);
}

void CPHObject::collision_disable()
{
	ISpatial::spatial_unregister();
}
void CPHObject::collision_enable()
{
	ISpatial::spatial_register();
}

void CPHObject::Freeze()
{
	if(!m_flags.test(st_activated))return;
	ph_world->RemoveObject(this);
	ph_world->AddFreezedObject(this);
	FreezeContent();
}

void CPHObject::UnFreeze()
{
	if(!m_flags.test(st_freezed)) return;
	UnFreezeContent();
	ph_world->RemoveFreezedObject(this);
	ph_world->AddObject(this);
}



CPHUpdateObject::CPHUpdateObject()
{
	b_activated=false;
}

void CPHUpdateObject::Activate()
{
	if(b_activated)return;
	ph_world->AddUpdateObject(this);
	b_activated=true;
}

void CPHUpdateObject::Deactivate()
{
	if(!b_activated)return;
	ph_world->RemoveUpdateObject(this);
	b_activated=false;
}

