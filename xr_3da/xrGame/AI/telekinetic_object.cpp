#include "stdafx.h"
#include "../physicsshellholder.h"
#include "telekinetic_object.h"
#include "../PhysicsShell.h"
#include "../PHInterpolation.h"
#include "../PHElement.h"
#include "../level.h"
#include "../gameobject.h"

#define KEEP_IMPULSE_UPDATE 200
#define FIRE_TIME			3000
#define RAISE_MAX_TIME		5000

CTelekineticObject::CTelekineticObject()
{
		state				= TS_None;
		object				= 0;
		telekinesis			= 0;
}

CTelekineticObject::~CTelekineticObject()
{

}



bool CTelekineticObject::init(CTelekinesis* tele,CPhysicsShellHolder *obj, float s, float h, u32 ttk) 
{
	if (!obj || !obj->m_pPhysicsShell) return false;

	//state				= TS_Raise;
	switch_state(TS_Raise);
	object				= obj;
	
	target_height		= obj->Position().y + h;

	time_keep_started	= 0;
	time_keep_updated	= 0;
	time_to_keep		= ttk;

	strength			= s;

	time_fire_started	= 0;
	//time_raise_started	= Level().timeServer();

	object->m_pPhysicsShell->set_ApplyByGravity(FALSE);
	
	return true;
}

void CTelekineticObject::raise_update()
{
	if (check_height()) prepare_keep();// ������ ��������� ��������
	else if (check_raise_time_out()) release();
	else {

		rotate();
	}
}
void CTelekineticObject::keep_update()
{
	if (time_keep_elapsed())release();
}
void CTelekineticObject::fire_update()
{
	if (time_fire_elapsed())release();
}
void CTelekineticObject::update_state()
{
	switch (get_state()) {
		case TS_Raise:	raise_update();	break;
		case TS_Keep:	keep_update();	break;
		case TS_Fire:	fire_update();	break;
		case TS_None:					break; 
	}
}

void CTelekineticObject::switch_state(ETelekineticState new_state)
{
	u32 time=Level().timeServer();
	
	switch (new_state) {
		case TS_Raise:	time_raise_started	=	time;	break;
		case TS_Keep:	time_keep_started	=	time;	break;
		case TS_Fire:	time_fire_started	=	time;	break;
		case TS_None:									break; 
	}
	state=new_state;
}
void CTelekineticObject::raise(float step) 
{
	if (!object || !object->m_pPhysicsShell || !object->m_pPhysicsShell->bActive) return;
		
	step *= strength;
	
	Fvector dir;
	dir.set(0.f,1.0f,0.f);

	float elem_size = float(object->m_pPhysicsShell->Elements().size());
	dir.mul(elem_size*elem_size);

	if (OnServer()) (object->m_pPhysicsShell->Elements()[0])->applyGravityAccel(dir);
}

void CTelekineticObject::prepare_keep()
{
	//time_keep_started	= Level().timeServer();
	//state				= TS_Keep;
	switch_state(TS_Keep);
	time_keep_updated	= 0;
}

bool CTelekineticObject::time_keep_elapsed()
{
	if (time_keep_started + time_to_keep < Level().timeServer()) return true;
	return false;
}

bool CTelekineticObject::time_fire_elapsed()
{
	if (time_fire_started + FIRE_TIME < Level().timeServer()) return true;
	return false;
}


void CTelekineticObject::keep()
{
	// ��������� ����� ���������� ����������
	//if (time_keep_updated + KEEP_IMPULSE_UPDATE > Level().timeServer()) return;
	
	if (!object || !object->m_pPhysicsShell || !object->m_pPhysicsShell->bActive) return;

	// ��������� ������
	float cur_h		= object->Position().y;

	// ���������� dir � ������������ � ������� �������
	Fvector dir;
	if (cur_h > target_height+ 0.6f)			dir.set(0.f,-1.0f,0.f);
	else if (cur_h < target_height+ 0.6f)		dir.set(0.f,1.0f,0.f);
	else {
		dir.set(Random.randF(-1.0f,1.0f), Random.randF(-1.0f,1.0f), Random.randF(-1.0f,1.0f));
		dir.normalize_safe();
	}

	//float elem_size = float(object->m_pPhysicsShell->Elements().size());
	dir.mul(5.0f);

	if (OnServer()) (object->m_pPhysicsShell->Elements()[0])->applyGravityAccel(dir);

	// ���������� ����� ���������� ����������
	time_keep_updated = Level().timeServer();
}

void CTelekineticObject::release() 
{
	if (!object || !object->m_pPhysicsShell || !object->m_pPhysicsShell->bActive) return;
	
	
	Fvector dir_inv;
	dir_inv.set(0.f,-1.0f,0.f);

		// �������� ����������
		object->m_pPhysicsShell->set_ApplyByGravity(TRUE);
		if (OnServer()) 
		{
		// ��������� ��������� ���� ��� ����, ����� ������ ����� ������
		object->m_pPhysicsShell->applyImpulse(dir_inv, 0.5f * object->m_pPhysicsShell->getMass());
	}
	//state = TS_None;
	switch_state(TS_None);
}

void CTelekineticObject::fire(const Fvector &target)
{
	//state				= TS_Fire;
	switch_state(TS_Fire);
	//time_fire_started	= Level().timeServer();

	if (!object || !object->m_pPhysicsShell || !object->m_pPhysicsShell->bActive) return;

	// ��������� �����������
	Fvector dir;
	dir.sub(target,object->Position());
	dir.normalize();

		// �������� ����������
		object->m_pPhysicsShell->set_ApplyByGravity(TRUE);

		if (OnServer()) 
		{	
		// ��������� ������
		for (u32 i=0;i<object->m_pPhysicsShell->Elements().size();i++) {
			object->m_pPhysicsShell->Elements()[i]->applyImpulse(dir, 20.f * object->m_pPhysicsShell->getMass() / object->m_pPhysicsShell->Elements().size());
		}
	};	
}

void CTelekineticObject::fire(const Fvector &target, float power)
{
	//state				= TS_Fire;
	switch_state(TS_Fire);
	//time_fire_started	= Level().timeServer();

	if (!object || !object->m_pPhysicsShell || !object->m_pPhysicsShell->bActive) return;

	// ��������� �����������
	Fvector dir;
	dir.sub(target,object->Position());
	dir.normalize();

		// �������� ����������
		object->m_pPhysicsShell->set_ApplyByGravity(TRUE);

		if (OnServer()) 
		{
		// ��������� ������
		for (u32 i=0;i<object->m_pPhysicsShell->Elements().size();i++) {
			object->m_pPhysicsShell->Elements()[i]->applyImpulse(dir, power * 20.f * object->m_pPhysicsShell->getMass() / object->m_pPhysicsShell->Elements().size());
		}
	};
};

bool CTelekineticObject::check_height()
{
	if (!object) return true;	
	
	return (object->Position().y > target_height);
}
bool CTelekineticObject::check_raise_time_out()
{
	if (time_raise_started + RAISE_MAX_TIME < Level().timeServer())
		return true;

	return false;
}



void CTelekineticObject::enable()
{
	object->m_pPhysicsShell->Enable();
}

void CTelekineticObject::rotate()
{
	if (!object || !object->m_pPhysicsShell || !object->m_pPhysicsShell->bActive) return;
	
	// ��������� �����������
	Fvector dir;
	dir.random_dir();
	dir.normalize();

	if (OnServer()) object->m_pPhysicsShell->applyImpulse(dir, 2.5f * object->m_pPhysicsShell->getMass());
}
