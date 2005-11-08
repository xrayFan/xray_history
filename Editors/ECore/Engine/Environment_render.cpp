#include "stdafx.h"
#pragma hdrstop

#include "Environment.h"
#include "blenders\blender.h"
#include "render.h"
#include "xr_efflensflare.h"
#include "rain.h"
#include "thunderbolt.h"
#include "igame_level.h"

//////////////////////////////////////////////////////////////////////////
// half box def
static	Fvector3	hbox_verts[24]	=
{
	{-1.f,	-1.f,	-1.f}, {-1.f,	-1.01f,	-1.f},	// down
	{ 1.f,	-1.f,	-1.f}, { 1.f,	-1.01f,	-1.f},	// down
	{-1.f,	-1.f,	 1.f}, {-1.f,	-1.01f,	 1.f},	// down
	{ 1.f,	-1.f,	 1.f}, { 1.f,	-1.01f,	 1.f},	// down
	{-1.f,	 1.f,	-1.f}, {-1.f,	 1.f,	-1.f},
	{ 1.f,	 1.f,	-1.f}, { 1.f,	 1.f,	-1.f},
	{-1.f,	 1.f,	 1.f}, {-1.f,	 1.f,	 1.f},
	{ 1.f,	 1.f,	 1.f}, { 1.f,	 1.f,	 1.f},
	{-1.f,	 0.f,	-1.f}, {-1.f,	-1.f,	-1.f},	// half
	{ 1.f,	 0.f,	-1.f}, { 1.f,	-1.f,	-1.f},	// half
	{ 1.f,	 0.f,	 1.f}, { 1.f,	-1.f,	 1.f},	// half
	{-1.f,	 0.f,	 1.f}, {-1.f,	-1.f,	 1.f}	// half
};
static	u16			hbox_faces[20*3]	=
{
	0,	 2,	 3,
	3,	 1,	 0,
	4,	 5,	 7,
	7,	 6,	 4,
	0,	 1,	 9,
	9,	 8,	 0,
	8,	 9,	 5,
	5,	 4,	 8,
	1,	 3,	10,
	10,	 9,	 1,
	9,	10,	 7,
	7,	 5,	 9,
	3,	 2,	11,
	11,	10,	 3,
	10,	11,	 6,
	6,	 7,	10,
	2,	 0,	 8,
	8,	11,	 2,
	11,	 8,	 4,
	4,	 6,	11
};

//////////////////////////////////////////////////////////////////////////
// shader/blender
class CBlender_skybox		: public IBlender  
{
public:
	virtual		LPCSTR		getComment()	{ return "INTERNAL: combiner";	}
	virtual		BOOL		canBeDetailed()	{ return FALSE;	}
	virtual		BOOL		canBeLMAPped()	{ return FALSE;	}

	virtual		void		Compile			(CBlender_Compile& C)
	{
		C.r_Pass			("sky2",		"sky2",		FALSE,	TRUE, FALSE);
		C.r_Sampler_clf		("s_sky0",		"$null"			);
		C.r_Sampler_clf		("s_sky1",		"$null"			);
		C.r_Sampler_rtf		("s_tonemap",	"$user$tonemap"	);	//. hack
		C.r_End				();
	}
};
static CBlender_skybox		b_skybox;

//////////////////////////////////////////////////////////////////////////
// vertex
#pragma pack(push,1)
struct v_skybox				{
	Fvector3	p;
	u32			color;
	Fvector3	uv	[2];

	void		set			(Fvector3& _p, u32 _c, Fvector3& _tc)
	{
		p					= _p;
		color				= _c;
		uv[0]				= _tc;
		uv[1]				= _tc;
	}
};
const	u32 v_skybox_fvf	= D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE3(0) | D3DFVF_TEXCOORDSIZE3(1);
struct v_clouds				{
	Fvector3	p;
	u32			color;
	u32			intensity;
	void		set			(Fvector3& _p, u32 _c, u32 _i)
	{
		p					= _p;
		color				= _c;
		intensity			= _i;
	}
};
const	u32 v_clouds_fvf	= D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR;
#pragma pack(pop)

//-----------------------------------------------------------------------------
// Environment render
//-----------------------------------------------------------------------------
extern float psHUD_FOV;
void CEnvironment::RenderSky		()
{
	if (0==g_pGameLevel)		return	;
	::Render->rmFar				();

	// draw sky box
	Fmatrix						mSky;
	mSky.rotateY				(CurrentEnv.sky_rotation);
	mSky.translate_over			(Device.vCameraPosition);

	u32		i_offset,v_offset;
	u32		C					= color_rgba(iFloor(CurrentEnv.sky_color.x*255.f), iFloor(CurrentEnv.sky_color.y*255.f), iFloor(CurrentEnv.sky_color.z*255.f), iFloor(CurrentEnv.weight*255.f));

	// Fill index buffer
	u16*	pib					= RCache.Index.Lock	(20*3,i_offset);
	CopyMemory					(pib,hbox_faces,20*3*2);
	RCache.Index.Unlock			(20*3);

	// Fill vertex buffer
	v_skybox* pv				= (v_skybox*)	RCache.Vertex.Lock	(12,sh_2geom.stride(),v_offset);
	for (u32 v=0; v<12; v++)	pv[v].set		(hbox_verts[v*2],C,hbox_verts[v*2+1]);
	RCache.Vertex.Unlock		(12,sh_2geom.stride());

	// Render
	RCache.set_xform_world		(mSky);
	RCache.set_Geometry			(sh_2geom);
	RCache.set_Shader			(sh_2sky);
	RCache.set_Textures			(&CurrentEnv.sky_r_textures);
	RCache.Render				(D3DPT_TRIANGLELIST,v_offset,0,12,i_offset,20);

	// Sun
	::Render->rmNormal			();
	eff_LensFlare->Render		(TRUE,FALSE,FALSE);
}

void CEnvironment::RenderClouds			()
{
	// draw clouds
	if (fis_zero(CurrentEnv.clouds_color.w,EPS_L))	return;

	::Render->rmFar				();

	Fmatrix						mXFORM, mScale;
	mScale.scale				(10,0.4f,10);
	mXFORM.rotateY				(CurrentEnv.sky_rotation);
	mXFORM.mulB_43				(mScale);
	mXFORM.translate_over		(Device.vCameraPosition);

	Fvector wd0,wd1;
	Fvector4 wind_dir;
	wd0.setHP					(PI_DIV_4,0);
	wd1.setHP					(PI_DIV_4+PI_DIV_8,0);
	wind_dir.set				(wd0.x,wd0.z,wd1.x,wd1.z).mul(0.5f).add(0.5f).mul(255.f);
	u32		i_offset,v_offset;
	u32		C0					= color_rgba(iFloor(wind_dir.x),iFloor(wind_dir.y),iFloor(wind_dir.w),iFloor(wind_dir.z));
	u32		C1					= color_rgba(iFloor(CurrentEnv.clouds_color.x*255.f),iFloor(CurrentEnv.clouds_color.y*255.f),iFloor(CurrentEnv.clouds_color.z*255.f),iFloor(CurrentEnv.clouds_color.w*255.f));

	// Fill index buffer
	u16*	pib					= RCache.Index.Lock	(CloudsIndices.size(),i_offset);
	Memory.mem_copy				(pib,&CloudsIndices.front(),CloudsIndices.size()*sizeof(u16));
	RCache.Index.Unlock			(CloudsIndices.size());

	// Fill vertex buffer
	v_clouds* pv				= (v_clouds*)	RCache.Vertex.Lock	(CloudsVerts.size(),clouds_geom.stride(),v_offset);
	for (FvectorIt it=CloudsVerts.begin(); it!=CloudsVerts.end(); it++,pv++)
		pv->set					(*it,C0,C1);
	RCache.Vertex.Unlock		(CloudsVerts.size(),clouds_geom.stride());

	// Render
	RCache.set_xform_world		(mXFORM);
	RCache.set_Geometry			(clouds_geom);
	RCache.set_Shader			(clouds_sh);
	RCache.set_Textures			(&CurrentEnv.clouds_r_textures);
	RCache.Render				(D3DPT_TRIANGLELIST,v_offset,0,CloudsVerts.size(),i_offset,CloudsIndices.size()/3);

	::Render->rmNormal			();
}

void CEnvironment::RenderFlares		()
{
	// 1
	eff_LensFlare->Render			(FALSE,TRUE,TRUE);
}

void CEnvironment::RenderLast		()
{
	// 2
	eff_Rain->Render				();
	eff_Thunderbolt->Render			();
}

void CEnvironment::OnDeviceCreate()
{
	sh_2sky.create			(&b_skybox,"skybox_2t");
	sh_2geom.create			(v_skybox_fvf,RCache.Vertex.Buffer(), RCache.Index.Buffer());
	clouds_sh.create		("clouds","null");
	clouds_geom.create		(v_clouds_fvf,RCache.Vertex.Buffer(), RCache.Index.Buffer());
	load					();
}
void CEnvironment::OnDeviceDestroy()
{
	unload					();
	sh_2sky.destroy			();
	sh_2geom.destroy		();
	clouds_sh.destroy		();
	clouds_geom.destroy		();
}

#ifdef _EDITOR
void CEnvironment::ED_Reload()
{
	OnDeviceDestroy			();
	OnDeviceCreate			();
}
#endif
