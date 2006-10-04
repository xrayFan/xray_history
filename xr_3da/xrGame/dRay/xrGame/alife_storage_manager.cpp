////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_storage_manager.cpp
//	Created 	: 25.12.2002
//  Modified 	: 12.05.2004
//	Author		: Dmitriy Iassenev
//	Description : ALife Simulator storage manager
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "alife_storage_manager.h"
#include "alife_simulator_header.h"
#include "alife_time_manager.h"
#include "alife_spawn_registry.h"
#include "alife_object_registry.h"
#include "alife_graph_registry.h"
#include "alife_group_registry.h"
#include "alife_registry_container.h"
#include "xrserver.h"
#include "level.h"
#include "../x_ray.h"

using namespace ALife;

CALifeStorageManager::~CALifeStorageManager	()
{
}

void CALifeStorageManager::save	(LPCSTR save_name, bool update_name)
{
	string256					save;
	strcpy						(save,m_save_name);
	if (save_name) {
		strconcat				(m_save_name,save_name,SAVE_EXTENSION);
	}
	else {
		if (!xr_strlen(m_save_name)) {
			Log					("There is no file name specified!");
			return;
		}
	}

	u32							source_count;
	u32							dest_count;
	void						*dest_data;
	{
		CMemoryWriter			stream;
		header().save			(stream);
		time_manager().save		(stream);
		spawns().save			(stream);
		objects().save			(stream);
		registry().save			(stream);

		source_count			= stream.tell();
		void					*source_data = stream.pointer();
		dest_count				= rtc_csize(source_count);
		dest_data				= xr_malloc(dest_count);
		dest_count				= rtc_compress(dest_data,dest_count,source_data,source_count);
	}

	string256					temp;
	FS.update_path				(temp,"$game_saves$",m_save_name);
	IWriter						*writer = FS.w_open(temp);
	writer->w_u32				(source_count);
	writer->w					(dest_data,dest_count);
	xr_free						(dest_data);
	FS.w_close					(writer);
	Msg							("* Game %s is successfully saved to file '%s' (%d bytes compressed to %d)",m_save_name,temp,source_count,dest_count + 4);

	if (!update_name)
		strcpy					(m_save_name,save);
}

bool CALifeStorageManager::load	(LPCSTR save_name)
{
	CTimer						timer;
	timer.Start					();
	string256					save;
	strcpy						(save,m_save_name);
	if (!save_name) {
		if (!xr_strlen(m_save_name))
			R_ASSERT2			(false,"There is no file name specified!");
	}
	else
		strconcat				(m_save_name,save_name,SAVE_EXTENSION);
	string256					file_name;
	FS.update_path				(file_name,"$game_saves$",m_save_name);

	IReader						*stream;
	stream						= FS.r_open(file_name);
	if (!stream) {
		Msg						("* Cannot find saved game %s",file_name);
		strcpy					(m_save_name,save);
		return					(false);
	}

	string512					temp;
	pApp->LoadTitle				(strconcat(temp,"Loading saved game \"",save_name,SAVE_EXTENSION,"\"..."));

	unload						();
	reload						(m_section);

	u32							source_count = stream->r_u32();
	void						*source_data = xr_malloc(source_count);
	rtc_decompress				(source_data,source_count,stream->pointer(),stream->length() - sizeof(source_count));
	FS.r_close					(stream);

	{
		IReader					source(source_data,source_count);
		header().load			(source);
		time_manager().load		(source);
		spawns().load			(source,file_name);
		objects().load			(source);

		CALifeObjectRegistry::OBJECT_REGISTRY::iterator	B= objects().objects().begin();
		CALifeObjectRegistry::OBJECT_REGISTRY::iterator	E = objects().objects().end();
		CALifeObjectRegistry::OBJECT_REGISTRY::iterator	I;
		for (I = B; I != E; ++I) {
			ALife::_OBJECT_ID			id = (*I).second->ID;
			object->ID					= server().PerformIDgen(id);
			VERIFY						(id == (*I).second->ID);
			m_manager->register_object	((*I).second,false,false);
		}

		for (I = B; I != E; ++I) {
			(*I).second->on_register	();

		registry().load			(source);
	}

	xr_free						(source_data);

	groups().on_after_game_load	();

	VERIFY						(graph().actor());
	
	Msg							("* Game %s is successfully loaded from file '%s' (%.3fs)",save_name, file_name,timer.GetElapsed_sec());

	return						(true);
}

void CALifeStorageManager::save	(NET_Packet &net_packet)
{
	prepare_objects_for_save	();

	shared_str					game_name;
	net_packet.r_stringZ		(game_name);
	save						(*game_name,!!net_packet.r_u8());
}

void CALifeStorageManager::prepare_objects_for_save	()
{
	Level().ClientSend			();
	Level().ClientSave			();
}
