#include "stdafx.h"
#pragma hdrstop

#include "Blender_light_point.h"

CBlender_accum_point::CBlender_accum_point	()	{	description.CLS		= 0;	}
CBlender_accum_point::~CBlender_accum_point	()	{	}

void	CBlender_accum_point::Compile(CBlender_Compile& C)
{
	IBlender::Compile		(C);

	BOOL	b_HW_smap		= RImplementation.o.HW_smap;
	switch (C.iElement)
	{
	case SE_L_FILL:			// masking
		C.r_Pass			("null", 			"dumb_tcopy",				false,	FALSE,	FALSE);
		C.r_Sampler			("s_base",			C.L_textures[0]	);
		C.r_End				();
		break;
	case SE_L_UNSHADOWED:	// unshadowed
		C.r_Pass			("accum_volume",	"accum_omni_unshadowed",	false,	FALSE,FALSE,TRUE,D3DBLEND_ONE,D3DBLEND_ONE);
		C.r_Sampler_rtf		("s_position",		r2_RT_P			);
		C.r_Sampler_rtf		("s_normal",		r2_RT_N			);
		C.r_Sampler_clf		("s_material",		r2_material		);
		C.r_Sampler_clf		("s_lmap",			C.L_textures[0]	);
		C.r_End				();
		break;
	case SE_L_NORMAL:		// normal
		C.r_Pass			("accum_volume",	"accum_omni_normal",		false,	FALSE,FALSE,TRUE,D3DBLEND_ONE,D3DBLEND_ONE);
		C.r_Sampler_rtf		("s_position",		r2_RT_P);
		C.r_Sampler_rtf		("s_normal",		r2_RT_N);
		C.r_Sampler_clf		("s_material",		r2_material);
		C.r_Sampler			("s_lmap",			C.L_textures[0]	);
		if (b_HW_smap)		C.r_Sampler_clf		("s_smap",r2_RT_smap_depth	);
		else				C.r_Sampler_rtf		("s_smap",r2_RT_smap_surf	);
		C.r_End				();
		break;
	case SE_L_FULLSIZE:		// normal-fullsize
		C.r_Pass			("accum_volume",	"accum_omni_normal",		false,	FALSE,FALSE,TRUE,D3DBLEND_ONE,D3DBLEND_ONE);
		C.r_Sampler_rtf		("s_position",		r2_RT_P);
		C.r_Sampler_rtf		("s_normal",		r2_RT_N);
		C.r_Sampler_clf		("s_material",		r2_material);
		C.r_Sampler			("s_lmap",			C.L_textures[0]);
		if (b_HW_smap)		C.r_Sampler_clf		("s_smap",r2_RT_smap_depth	);
		else				C.r_Sampler_rtf		("s_smap",r2_RT_smap_surf	);
		C.r_End				();
		break;
	case SE_L_TRANSLUENT:	// shadowed + transluency
		C.r_Pass			("accum_volume",	"accum_omni_transluent",	false,	FALSE,FALSE,TRUE,D3DBLEND_ONE,D3DBLEND_ONE);
		C.r_Sampler_rtf		("s_position",		r2_RT_P);
		C.r_Sampler_rtf		("s_normal",		r2_RT_N);
		C.r_Sampler_clf		("s_material",		r2_material);
		C.r_Sampler_clf		("s_lmap",			r2_RT_smap_surf);			// diff here
		if (b_HW_smap)		C.r_Sampler_clf		("s_smap",r2_RT_smap_depth	);
		else				C.r_Sampler_rtf		("s_smap",r2_RT_smap_surf	);
		C.r_End				();
		break;
	}
}
