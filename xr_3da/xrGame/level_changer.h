////////////////////////////////////////////////////////////////////////////
//	Module 		: level_changer.h
//	Created 	: 10.07.2003
//  Modified 	: 10.07.2003
//	Author		: Dmitriy Iassenev
//	Description : Level change object
////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameObject.h"
#include "../feel_touch.h"

class CLevelChanger : public CGameObject, public Feel::Touch {
private:
	ALife::_GRAPH_ID	m_game_vertex_id;
	u32					m_level_vertex_id;
	Fvector				m_position;
	Fvector				m_angles;

public:
	typedef	CGameObject	inherited;

	virtual			~CLevelChanger		();
	virtual BOOL	net_Spawn			(LPVOID DC);
	//virtual void	spatial_register	();
	//virtual void	spatial_move		();

	virtual void	Center				(Fvector& C) const;
	virtual float	Radius				() const;

			void	UpdateCL			();
	virtual void	feel_touch_new		(CObject* O);
	virtual BOOL	feel_touch_contact	(CObject* O);
};