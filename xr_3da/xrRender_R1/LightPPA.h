// LightPPA.h: interface for the CLightPPA class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LIGHTPPA_H__E5B97AC9_84A6_4773_9FEF_3BC5D1CEF8B6__INCLUDED_)
#define AFX_LIGHTPPA_H__E5B97AC9_84A6_4773_9FEF_3BC5D1CEF8B6__INCLUDED_
#pragma once

#include "light.h"

struct	CLightR_Vertex
{
	Fvector			P;
	Fvector			N;
	float			u0,v0;
	float			u1,v1;
};

class	CLightR_Manager
{
	ref_shader						hShader;
	ref_geom						hGeom;

	xr_vector<light*>				selected_point;
	xr_vector<light*>				selected_spot;
public:
	CLightR_Manager					();
	virtual ~CLightR_Manager		();

	void			add				(light* L);
	void			render			();
	void			render_point	();
	void			render_spot		();
};

#endif // !defined(AFX_LIGHTPPA_H__E5B97AC9_84A6_4773_9FEF_3BC5D1CEF8B6__INCLUDED_)
