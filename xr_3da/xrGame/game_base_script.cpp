#include "stdafx.h"
#include "game_base.h"
#include "script_space.h"
#include "xrServer_script_macroses.h"

using namespace luabind;


template <typename T>
struct CWrapperBase : public T, public luabind::wrap_base {
	typedef T inherited;
	typedef CWrapperBase<T>	self_type;

	DEFINE_LUA_WRAPPER_METHOD_R2P1_V1(net_Export, NET_Packet)
	DEFINE_LUA_WRAPPER_METHOD_R2P1_V1(net_Import, NET_Packet)
	DEFINE_LUA_WRAPPER_METHOD_V0(clear)
};

void game_PlayerState::script_register(lua_State *L)
{
	typedef CWrapperBase<game_PlayerState> WrapType;
	typedef game_PlayerState BaseType;

	module(L)
		[
			luabind::class_<game_PlayerState, WrapType>("game_PlayerState")
			.def(	constructor<>())
			.def_readwrite("team",				&BaseType::team)
			.def_readwrite("kills",				&BaseType::kills)
			.def_readwrite("deaths",			&BaseType::deaths)
			.def_readwrite("money_total",		&BaseType::money_total)
			.def_readwrite("money_for_round",	&BaseType::money_for_round)
			.def_readwrite("flags",				&BaseType::flags)
			.def_readwrite("ping",				&BaseType::ping)
			.def_readwrite("GameID",			&BaseType::GameID)
			.def_readwrite("Skip",				&BaseType::Skip)
			.def_readwrite("lasthitter",		&BaseType::lasthitter)
			.def_readwrite("lasthitweapon",		&BaseType::lasthitweapon)
			.def_readwrite("skin",				&BaseType::skin)
			.def_readwrite("RespawnTime",		&BaseType::RespawnTime)
			.def_readwrite("money_delta",		&BaseType::money_delta)

			.def_readwrite("pItemList",			&BaseType::pItemList)
			.def_readwrite("LastBuyAcount",		&BaseType::LastBuyAcount)
			.def("testFlag",					&BaseType::testFlag)
			.def("setFlag",						&BaseType::setFlag)
			.def("resetFlag",					&BaseType::resetFlag)
			.def("getName",						&BaseType::getName)
			.def("setName",						&BaseType::setName)
			.def("clear",						&BaseType::clear, &WrapType::clear_static)
			.def("net_Export",					&BaseType::net_Export, &WrapType::net_Export_static)
			.def("net_Import",					&BaseType::net_Import, &WrapType::net_Import_static)
			
		];
}



void game_GameState::script_register(lua_State *L)
{

	module(L)
		[
			luabind::class_<DLL_Pure>("dll_pure"),

			luabind::class_< game_GameState, DLL_Pure >("game_GameState")
			.def(	constructor<>())

			.def_readwrite("type",				&game_GameState::type)
			.def_readonly("round",				&game_GameState::round)
			.def_readonly("start_time",			&game_GameState::start_time)

			.def("Type",						&game_GameState::Type)
			.def("Phase",						&game_GameState::Phase)
			.def("Round",						&game_GameState::Round)
			.def("StartTime",					&game_GameState::StartTime)
		];

}
