// LightPPA.cpp: implementation of the CLightPPA class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LightPPA.h"
#include "collide\cl_rapid.h"
#include "xr_creator.h"
#include "fstaticrender.h"

const DWORD MAX_POLYGONS=1024;
const float MAX_DISTANCE=50.f;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLightPPA::CLightPPA()
{

}

CLightPPA::~CLightPPA()
{

}

IC void mk_vertex	(PPA_Vertex& D, Fvector& P, Fvector& C, float r2)
{
	D.P.set	(P);
	D.u0	= (P.x-C.x)/r2+.5f; 
	D.v0	= (P.z-C.z)/r2+.5f;
	D.u1	= (P.y-C.y)/r2+.5f;
	D.v1	=.5f;
}

void CLightPPA::Render(CList<PPA_Vertex>&	vlist)
{
	VERIFY	(pCreator);
	vlist.clear();

	// Build bbox
	Fbox	BB;
	BB.set	(sphere.P,sphere.P);
	BB.grow	(sphere.R+EPS_L);

	// Query collision DB (Select polygons)
	XRC.BBoxMode		(0);
	XRC.BBoxCollide		(precalc_identity,pCreator->ObjectSpace.GetStaticModel(),precalc_identity,BB);
	DWORD	triCount	= XRC.GetBBoxContactCount();
	if (0==triCount)	return;
	RAPID::tri* tris	= pCreator->ObjectSpace.GetStaticTris();

	// Calculate XForms
	Fmatrix	XForm, invXForm;
	float	R			= sphere.R;
//	XForm.scale			(R,R,R);
//	XForm.translate_over(sphere.P);

	invXForm.translate	(-sphere.P.x,-sphere.P.y,-sphere.P.z);

	// Clip and triangulate polygons
	Fvector	cam = Device.vCameraPosition;
	for (DWORD t=0; t<triCount; t++)
	{
		RAPID::tri&	T	= tris[XRC.BBoxContact[t].id];

		Fvector	V1		= *T.verts[0];
		Fvector V2		= *T.verts[1];
		Fvector V3		= *T.verts[2];
		Fplane  Poly;	Poly.build(V1,V2,V3);

		// Test for poly facing away from light or camera
		if (Poly.classify(sphere.P)<0)	continue;
		if (Poly.classify(cam)<0)		continue;

		// Triangulation
		PPA_Vertex		vert1,vert2,vert3;
		Fvector			uv;
		vert1.N.set		(Poly.n);	mk_vertex(vert1,V1,sphere.P,sphere.R*2);	vlist.push_back	(vert1);
		vert2.N.set		(Poly.n);	mk_vertex(vert2,V2,sphere.P,sphere.R*2);	vlist.push_back	(vert2);
		vert3.N.set		(Poly.n);	mk_vertex(vert3,V3,sphere.P,sphere.R*2);	vlist.push_back	(vert3);
	}
}

void CLightPPA_Manager::Initialize	()
{
	SH	= Device.Shader.Create	("$light");
	VS	= Device.Streams.Create	(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2, MAX_POLYGONS*3);
}
void CLightPPA_Manager::Destroy		()
{
	Device.Shader.Delete		(SH);
}

void CLightPPA_Manager::Render()
{
	// Projection
	float _43 = Device.mProject._43;
	Device.mProject._43 -= 0.001f; 
	CHK_DX(HW.pDevice->SetTransform	 ( D3DTS_PROJECTION,	Device.mProject.d3d() ));

	// Create D3D unattenuated light
	// D3D.diffuse.set		(color);
	// D3D.position.set		(sphere.P);
	// D3D.range			= sphere.R;

	for (DWORD L=0; L<container.size(); L++)
	{
		CLightPPA&	PPL = *container[L];
		float	alpha	= Device.vCameraPosition.distance_to(PPL.sphere.P)/MAX_DISTANCE;

		// Culling
		if (alpha>=1)	continue;
		if (!::Render.ViewBase.testSphereDirty (PPL.sphere.P,PPL.sphere.R))	continue;

		// Calculations
		PPL.Render	(storage);

		// Lock and copy
		DWORD	vOffset;
		DWORD	vCount	= storage.size	();
		void*	VB		= VS->Lock		(vCount,vOffset);
		CopyMemory	(VB,storage.begin(),vCount*sizeof(PPA_Vertex));
		VS->Unlock	(vCount);

		// Rendering
		Device.Shader.Set		(SH);
		Device.Primitive.Draw	(VS,vCount/3,vOffset);
	}
	container.clear	();

	// Projection
	Device.mProject._43 = _43;
	CHK_DX(HW.pDevice->SetTransform	 ( D3DTS_PROJECTION,Device.mProject.d3d() ));
}
