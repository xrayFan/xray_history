////////////////////////////////////////////////////////////////////////////
//	Module 		: xrServer_Objects_ALife_script3.cpp
//	Created 	: 19.09.2002
//  Modified 	: 04.06.2003
//	Author		: Dmitriy Iassenev
//	Description : Server objects for ALife simulator, script export, the third part
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "xrServer_Objects_ALife_Monsters.h"
#include "script_space.h"
#include "xrServer_script_macroses.h"

using namespace luabind;

void CSE_ALifeObjectHangingLamp::script_register(lua_State *L)
{
	module(L)[
		luabind_class_alife2(
			CSE_ALifeObjectHangingLamp,
			"cse_alife_object_hanging_lamp",
			CSE_ALifeDynamicObjectVisual,
			CSE_PHSkeleton
		)
	];
}
void CSE_ALifeObjectPhysic::script_register(lua_State *L)
{
	module(L)[
		luabind_class_alife2(
			CSE_ALifeObjectPhysic,
			"cse_alife_object_physic",
			CSE_ALifeDynamicObjectVisual,
			CSE_PHSkeleton
		)
	];
}
