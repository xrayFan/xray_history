////////////////////////////////////////////////////////////////////////////
//	Module 		: patrol_path_params.cpp
//	Created 	: 30.09.2003
//  Modified 	: 29.06.2004
//	Author		: Dmitriy Iassenev
//	Description : Patrol path parameters class
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "patrol_path_params.h"
#include "patrol_path_manager.h"
#include "ai_space.h"

CPatrolPathParams::CPatrolPathParams	(LPCSTR caPatrolPathToGo, const PatrolPathManager::EPatrolStartType tPatrolPathStart, const PatrolPathManager::EPatrolRouteType tPatrolPathStop, bool bRandom, u32 index)
{
	m_path_name			= caPatrolPathToGo;
	m_path				= Level().patrol_paths().path(shared_str(caPatrolPathToGo),true);
	if (!m_path) {
		ai().script_engine().script_log	(eLuaMessageTypeError,"There is no patrol path %s",caPatrolPathToGo);
		throw;
	}
	m_tPatrolPathStart	= tPatrolPathStart;
	m_tPatrolPathStop	= tPatrolPathStop;
	m_bRandom			= bRandom;
	m_previous_index	= index;
}

CPatrolPathParams::~CPatrolPathParams	()
{
}

u32	CPatrolPathParams::count			() const
{
	return				(m_path->vertices().size());
}

const Fvector &CPatrolPathParams::point	(u32 index) const
{
	VERIFY				(m_path);
	VERIFY				(!m_path->vertices().empty());
	if (!m_path->vertex(index)) {
		ai().script_engine().script_log(eLuaMessageTypeError,"Can't get information about patrol point number %d in the patrol way %s",index,*m_path_name);
		index			= (*m_path->vertices().begin())->vertex_id();
	}
	VERIFY				(m_path->vertex(index));
	return				(m_path->vertex(index)->data().position());
}

u32	CPatrolPathParams::level_vertex_id	(u32 index) const
{
	VERIFY				(m_path->vertex(index));
	return				(m_path->vertex(index)->data().level_vertex_id());
}

u32	CPatrolPathParams::point			(LPCSTR name) const
{
	if (m_path->point(name))
		return			(m_path->point(name)->vertex_id());
	return				(u32(-1));
}

u32	CPatrolPathParams::point			(const Fvector &point) const
{
	return				(m_path->point(point)->vertex_id());
}

bool CPatrolPathParams::flag			(u32 index, u8 flag_index) const
{
	VERIFY				(m_path->vertex(index));
	return				(!!(m_path->vertex(index)->data().flags() & (u32(1) << flag_index)));
}

int  CPatrolPathParams::flags			(u32 index) const
{
	VERIFY				(m_path->vertex(index));
	return				(m_path->vertex(index)->data().flags());
}

LPCSTR	CPatrolPathParams::name	(u32 index) const
{
	VERIFY				(m_path->vertex(index));
	return				(*m_path->vertex(index)->data().name());
}
