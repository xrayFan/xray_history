#include "stdafx.h"
#include "pseudo_gigant.h"
#include "../../../script_space.h"

using namespace luabind;

#pragma optimize("s",on)
void CPseudoGigant::script_register(lua_State *L)
{
	module(L)
	[
		class_<CPseudoGigant,CGameObject>("CPseudoGigant")
			.def(constructor<>())
	];
}
