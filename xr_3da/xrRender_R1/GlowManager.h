// GlowManager.h: interface for the CGlowManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GLOWMANAGER_H__EC35911F_479B_469A_845C_1A64D81D0326__INCLUDED_)
#define AFX_GLOWMANAGER_H__EC35911F_479B_469A_845C_1A64D81D0326__INCLUDED_
#pragma once

#include "..\ispatial.h"
#include "..\xr_collide_defs.h"

class CGlow				: public ISpatial
{
public:
	float				fade;
	ref_shader			hShader;
	u32					dwFrame;

	// Ray-testing cache
	BOOL				bTestResult;
	Collide::ray_cache	RayCache;
};

#define MAX_GlowsPerFrame	64

class CGlowManager 
{
	xr_vector<CGlow*>		Glows;
	xr_vector<CGlow*>		Selected;
	ref_geom				hGeom;

	u32						dwTestID;
public:
	void	add				(CGlow		*G);

	void	Load			(IReader	*fs);
	void	Unload			();

	void	Render			();

	CGlowManager			();
	~CGlowManager			();
};

#endif // !defined(AFX_GLOWMANAGER_H__EC35911F_479B_469A_845C_1A64D81D0326__INCLUDED_)
