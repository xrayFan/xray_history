#include "stdafx.h"
#include "detailmanager.h"

static DWORD dwDecl[] =
{
    D3DVSD_STREAM	(0),
	D3DVSD_REG		(0,	D3DVSDT_FLOAT3),		// pos
	D3DVSD_REG		(1,	D3DVSDT_D3DCOLOR),		// matrix id
	D3DVSD_REG		(2,	D3DVSDT_FLOAT2),		// uv
	D3DVSD_END		()
};
#pragma pack(push,1)
struct	vertHW
{
	Fvector		P;
	DWORD		M;
	Fvector2	UV;
};
#pragma pack(pop)

void CDetailManager::VS_Load()
{
	// Load vertex shader
	LPD3DXBUFFER	code;
	LPD3DXBUFFER	errors;
	R_CHK			(D3DXAssembleShaderFromFile("data\\shaders\\detail.vs",0,NULL,&code,&errors));
	R_CHK			(HW.pDevice->CreateVertexShader(dwDecl,LPDWORD(code->GetBufferPointer()),&VS_Code,0));
	_RELEASE		(code);
	_RELEASE		(errors);

	// Analyze batch-size
	VS_BatchSize	= (DWORD(HW.Caps.vertex.dwRegisters)-1)/5;

	// Pre-process objects
	DWORD			dwVerts		= 0;
	DWORD			dwIndices	= 0;
	for (u32 o=0; o<objects.size(); o++)
	{
		CDetail& D	=	objects[o];
		dwVerts		+=	D.number_vertices*VS_BatchSize;
		dwIndices	+=	D.number_indices*VS_BatchSize;
	}
	DWORD			vSize		= (3+1+2)*4;

	// Determine POOL & USAGE
	DWORD dwUsage	=	D3DUSAGE_WRITEONLY;
	D3DPOOL dwPool	=	D3DPOOL_DEFAULT;
	if (HW.Caps.vertex.bSoftware)	{
		dwUsage	|=	D3DUSAGE_SOFTWAREPROCESSING;
		dwPool	=	D3DPOOL_SYSTEMMEM;
	}

	// Create VB/IB
	CHK_DX			(HW.pDevice->ResourceManagerDiscardBytes(0));
	R_CHK			(HW.pDevice->CreateVertexBuffer(dwVerts*vSize,dwUsage,0,dwPool,&VS_VB));
	R_CHK			(HW.pDevice->CreateIndexBuffer(dwIndices*2,dwUsage,D3DFMT_INDEX16,dwPool,&VS_IB));
	Msg("* [DETAILS] Batch(%d), VB(%dK), IB(%dK)",VS_BatchSize,(dwVerts*vSize)/1024, (dwIndices*2)/1024);
	
	// Fill VB
	{
		vertHW*			pV;
		R_CHK			(VS_VB->Lock(0,0,(BYTE**)&pV,0));
		for (o=0; o<objects.size(); o++)
		{
			CDetail& D		=	objects[o];
			for (u32 batch=0; batch<VS_BatchSize; batch++)
			{
				DWORD mid	=	batch*5+1;
				DWORD M		=	D3DCOLOR_RGBA	(mid,mid,mid,mid);
				for (u32 v=0; v<D.number_vertices; v++)
				{
					pV->P.set	(D.vertices[v].P);
					pV->M	=	M;
					pV->UV.set	(D.vertices[v].u,D.vertices[v].v);
					pV++;
				}
			}
		}
		R_CHK			(VS_VB->Unlock());
	}

	// Fill IB
	{
		u16*			pI;
		R_CHK			(VS_IB->Lock(0,0,(BYTE**)(&pI),0));
		for (o=0; o<objects.size(); o++)
		{
			CDetail& D		=	objects[o];
			u16		offset	=	0;
			for (u32 batch=0; batch<VS_BatchSize; batch++)
			{
				for (u32 i=0; i<D.number_indices; i++)
					*pI++	=	D.indices[i] + offset;
				offset		+=	u16(D.number_vertices);
			}
		}
		R_CHK			(VS_IB->Unlock());
	}
}

void CDetailManager::VS_Unload()
{
	// Destroy VS/VB/IB
	R_CHK			(HW.pDevice->DeleteVertexShader(VS_Code));	VS_Code = 0;
	_RELEASE		(VS_IB);
	_RELEASE		(VS_VB);
}

void CDetailManager::VS_Render()
{
	return;

	// Phase
	float	fPhaseRange	= PI/16;
	float	fPhaseX		= sinf(Device.fTimeGlobal*0.1f)	*fPhaseRange;
	float	fPhaseZ		= sinf(Device.fTimeGlobal*0.11f)*fPhaseRange;

	// Render-prepare
	CVS_Constants& VSC	=	Device.Shader.VSC;
	VSC.set						(0,255,255,255,255);
	VSC.flush					(0,1);
	Device.Primitive.setVertices(VS_Code,(3+1+2)*4,VS_VB);
	
	// Matrices and offsets
	Fmatrix		mXform,	mTemp;
	Fmatrix		mScreen	=	Device.mFullTransform;
	DWORD		vOffset	=	0;
	DWORD		iOffset	=	0;
	
	// Iterate
	for (DWORD O=0; O<objects.size(); O++)
	{
		CList<SlotItem*>&	vis = visible	[O];
		CDetail&	Object		= objects	[O];
		
		if (!vis.empty())
		{
			// Setup matrices + colors (and flush it as nesessary)
			Device.Shader.set_Shader		(Object.shader);
			Device.Primitive.setIndicesUC	(vOffset, VS_IB);

			DWORD dwBatch	= 0;
			for (DWORD item = 0; item<vis.size(); item++)
			{
				SlotItem&	Instance	= *(vis[item]);
				float	scale			= Instance.scale_calculated;
				DWORD	cBase			= dwBatch*5+1;
				
				// Build matrix
				if (scale>0.7f)	
				{
					mTemp.setXYZ			(Instance.phase_x+fPhaseX,0,Instance.phase_z+fPhaseZ);
					mXform.mul_43			(mTemp,Instance.mRotY);
					mXform._11				*= scale;
					mXform._22				*= scale;
					mXform._33				*= scale;
					mXform.translate_over	(Instance.P);
				} else {
					Fmatrix& M = Instance.mRotY;
					Fvector& P = Instance.P;
					mXform._11=M._11*scale;	mXform._12=M._12;		mXform._13=M._13;		mXform._14=M._14;
					mXform._21=M._21;		mXform._22=M._22*scale;	mXform._23=M._23;		mXform._24=M._24;
					mXform._31=M._31;		mXform._32=M._32;		mXform._33=M._33*scale;	mXform._34=M._34;
					mXform._41=P.x;			mXform._42=P.y;			mXform._43=P.z;			mXform._44=1;
				}
				mTemp.mul				(mScreen,mXform);
				VSC.set					(cBase,		mTemp);
				VSC.set					(cBase+4,	Instance.C);
				
				dwBatch	++;
				if (dwBatch == VS_BatchSize)	
				{
					// flush
					VSC.flush						(1,dwBatch*5);
					DWORD dwCNT_verts				= dwBatch * Object.number_vertices;
					DWORD dwCNT_prims				= (dwBatch * Object.number_indices)/3;
					Device.Primitive.Render			(D3DPT_TRIANGLELIST,0,dwCNT_verts,iOffset,dwCNT_prims);
					UPDATEC							(dwCNT_verts,dwCNT_prims,2);
					
					// restart
					dwBatch							= 0;
				}
			}
			
			// flush if nessecary
			if (dwBatch)
			{
				VSC.flush						(1,dwBatch*5);
				DWORD dwCNT_verts				= dwBatch * Object.number_vertices;
				DWORD dwCNT_prims				= (dwBatch * Object.number_indices)/3;
				Device.Primitive.Render			(D3DPT_TRIANGLELIST,0,dwCNT_verts,iOffset,dwCNT_prims);
				UPDATEC							(dwCNT_verts,dwCNT_prims,2);
			}
			
			// Clean up
			vis.clear	();
		}
		vOffset		+=	VS_BatchSize * Object.number_vertices;
		iOffset		+=	VS_BatchSize * Object.number_indices;
	}
}
