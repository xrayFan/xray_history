#include "stdafx.h"
#include "Physics.h"
#include "PHObject.h"
#include "PHWorld.h"

DEFINE_VECTOR(ISpatial*,qResultVec,qResultIt)

	CPHObject::CPHObject()
{
	b_activated=false;
	spatial.type|=STYPE_PHYSIC;
}

void CPHObject::Activate()
{
	if(b_activated)return;
	ph_world->AddObject(this);
	b_activated=true;
}

void CPHObject::EnableObject()
{
	Activate();
}

void CPHObject::Deactivate()
{
	if(!b_activated)return;
	ph_world->RemoveObject(PH_OBJECT_I(this));
	b_activated=false;
}


void CPHObject::spatial_move()
{
	get_spatial_params();
	ISpatial::spatial_move();
}

void CPHObject::Collide()
{
	//spatial_move();

	g_SpatialSpace->q_box(0,STYPE_PHYSIC,spatial.center,AABB);
	qResultVec& result=g_SpatialSpace->q_result;
	qResultIt i=result.begin(),e=result.end();
	for(;i!=e;++i)
	{
		
		CPHObject* obj2=static_cast<CPHObject*>(*i);
		if(obj2==this||(obj2->b_activated&&obj2->stack_pos<stack_pos))				continue;
		NearCallback(this,obj2,(dGeomID)dSpace(),(dGeomID)obj2->dSpace());
	}
///////////////////////////////
	CollideStatic((dGeomID)dSpace());
	//near_callback(this,0,(dGeomID)dSpace(),ph_world->GetMeshGeom());
}

void CPHObject::SaveContacts(dJointGroupID jointGroup)
{
	spatial_move();

	g_SpatialSpace->q_box(0,STYPE_PHYSIC,spatial.center,AABB);
	qResultVec& result=g_SpatialSpace->q_result;
	qResultIt i=result.begin(),e=result.end();
	for(;i!=e;++i)
	{

		CPHObject* obj2=static_cast<CPHObject*>(*i);
		if(obj2==this)				continue;
		::SaveContacts(this,obj2,(dGeomID)dSpace(),(dGeomID)obj2->dSpace(),jointGroup);
	}
	///////////////////////////////
	SaveContactsStatic((dGeomID)dSpace(),jointGroup);
}

void CPHObject::spatial_register()
{
	get_spatial_params();
	ISpatial::spatial_register();
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

