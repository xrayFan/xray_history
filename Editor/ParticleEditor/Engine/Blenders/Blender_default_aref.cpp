// Blender_default_aref.cpp: implementation of the CBlender_default_aref class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#pragma hdrstop

#include "Blender_default_aref.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBlender_default_aref::CBlender_default_aref()
{
	description.CLS		= B_DEFAULT_AREF;
	oAREF.value			= 32;
	oAREF.min			= 0;
	oAREF.max			= 255;
}

CBlender_default_aref::~CBlender_default_aref()
{

}

void	CBlender_default_aref::Save(	CFS_Base& FS )
{
	CBlender::Save	(FS);
	xrPWRITE_PROP	(FS,"Alpha ref",	xrPID_INTEGER,	oAREF);
}

void	CBlender_default_aref::Load(	CStream& FS , WORD version)
{
	CBlender::Load	(FS,version);
	xrPREAD_PROP	(FS,xrPID_INTEGER,	oAREF);
}

void CBlender_default_aref::Compile(CBlender_Compile& C)
{
	CBlender::Compile		(C);
	if (C.bEditor)	{
		C.PassBegin		();
		{
			C.PassSET_ZB			(TRUE,TRUE);
			C.PassSET_Blend		(TRUE, D3DBLEND_SRCALPHA,D3DBLEND_INVSRCALPHA,TRUE,oAREF.value);
			C.PassSET_LightFog	(TRUE,TRUE);
			
			// Stage0 - Base texture
			C.StageBegin		();
			{
				C.StageSET_Address	(D3DTADDRESS_WRAP);
				C.StageSET_Color		(D3DTA_TEXTURE,	  D3DTOP_MODULATE,		D3DTA_DIFFUSE);
				C.StageSET_Alpha		(D3DTA_TEXTURE,	  D3DTOP_MODULATE,		D3DTA_DIFFUSE);
				C.Stage_Texture		(oT_Name,	C.L_textures);
				C.Stage_Matrix		(oT_xform,	C.L_matrices,	0);
				C.Stage_Constant		("$null",	C.L_constants);
			}
			C.StageEnd			();
		}
		C.PassEnd			();
	} else {
		if (C.bLighting)	
		{
			C.PassBegin		();
			{
				C.PassSET_ZB			(TRUE,TRUE);
				C.PassSET_Blend		(TRUE, D3DBLEND_ONE, D3DBLEND_ZERO,TRUE,oAREF.value);
				C.PassSET_LightFog	(FALSE,FALSE);
				
				// Stage0 - Lightmap
				C.StageBegin		();
				{
					C.StageSET_Address	(D3DTADDRESS_WRAP);
					C.StageSET_Color		(D3DTA_TEXTURE,	  D3DTOP_SELECTARG1,	D3DTA_DIFFUSE);
					C.StageSET_Alpha		(D3DTA_TEXTURE,	  D3DTOP_SELECTARG1,	D3DTA_DIFFUSE);
					C.Stage_Texture		("$base1",	C.L_textures);
					C.Stage_Matrix		("$null",	C.L_matrices,1);
					C.Stage_Constant		("$null",	C.L_constants);
				}
				C.StageEnd			();
			}
			C.PassEnd			();
		} else {
			C.PassBegin		();
			{
				C.PassSET_ZB			(TRUE,TRUE);
				C.PassSET_Blend		(TRUE, D3DBLEND_ONE, D3DBLEND_ZERO,TRUE,oAREF.value);
				C.PassSET_LightFog	(FALSE,TRUE);
				
				// Stage0 - Lightmap
				C.StageBegin		();
				{
					C.StageSET_Address	(D3DTADDRESS_WRAP);
					C.StageSET_Color		(D3DTA_TEXTURE,	  D3DTOP_SELECTARG1,	D3DTA_DIFFUSE);
					C.StageSET_Alpha		(D3DTA_TEXTURE,	  D3DTOP_SELECTARG1,	D3DTA_DIFFUSE);
					C.Stage_Texture		("$base1",	C.L_textures);
					C.Stage_Matrix		("$null",	C.L_matrices,1);
					C.Stage_Constant		("$null",	C.L_constants);
				}
				C.StageEnd			();
				
				// Stage1 - Base texture
				C.StageBegin		();
				{
					C.StageSET_Address	(D3DTADDRESS_WRAP);
					C.StageSET_Color		(D3DTA_TEXTURE,	  D3DTOP_MODULATE2X,	D3DTA_CURRENT);
					C.StageSET_Alpha		(D3DTA_TEXTURE,	  D3DTOP_MODULATE2X,	D3DTA_CURRENT);
					C.Stage_Texture		(oT_Name,	C.L_textures);
					C.Stage_Matrix		(oT_xform,	C.L_matrices,	0);
					C.Stage_Constant		("$null",	C.L_constants);
				}
				C.StageEnd			();
			}
			C.PassEnd			();
		}
	}
}
