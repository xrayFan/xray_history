// ModelPool.cpp: implementation of the CModelPool class.
//
//////////////////////////////////////////////////////////////////////
  
#include "stdafx.h"
#pragma hdrstop

#include "ModelPool.h"

#ifndef _EDITOR
	#include "..\IGame_Persistent.h"
    #include "..\fmesh.h"
    #include "..\fhierrarhyvisual.h"
    #include "..\SkeletonAnimated.h"
    #include "..\SkeletonRigid.h"
	#include "fvisual.h"
	#include "fprogressive.h"
	#include "fskinned.h"
	#include "flod.h"
    #include "ftreevisual.h"
    #include "ParticleGroup.h"
#else
    #include "fmesh.h"
    #include "fvisual.h"
    #include "fprogressive.h"
    #include "ParticleGroup.h"
	#include "fskinned.h"
    #include "fhierrarhyvisual.h"
    #include "SkeletonAnimated.h"
    #include "SkeletonRigid.h"
	#include "IGame_Persistent.h"
#endif

IRender_Visual*	CModelPool::Instance_Create(u32 type)
{
	IRender_Visual *V = NULL;

	// Check types
	switch (type) {
	case MT_NORMAL:				// our base visual
		V	= xr_new<Fvisual>				();
		break;
	case MT_HIERRARHY:
		V	= xr_new<FHierrarhyVisual>		();
		break;
	case MT_PROGRESSIVE:		// dynamic-resolution visual
		V	= xr_new<FProgressive>			();
		break;
	case MT_SKELETON_ANIM:
		V	= xr_new<CSkeletonAnimated>		();
		break;
	case MT_SKELETON_RIGID:
		V	= xr_new<CSkeletonRigid>		();
		break;
	case MT_SKELETON_GEOMDEF_PM:
		V	= xr_new<CSkeletonX_PM>			();
		break;
	case MT_SKELETON_GEOMDEF_ST:
		V	= xr_new<CSkeletonX_ST>			();
		break;
	case MT_PARTICLE_EFFECT:
		V	= xr_new<PS::CParticleEffect>	();
		break;
	case MT_PARTICLE_GROUP:
		V	= xr_new<PS::CParticleGroup>	();
		break;
#ifndef _EDITOR
	case MT_LOD:
		V	= xr_new<FLOD>					();
		break;
	case MT_TREE_ST:
		V	= xr_new<FTreeVisual_ST>		();
		break;
	case MT_TREE_PM:
		V	= xr_new<FTreeVisual_PM>		();
		break;
#endif
	default:
		Debug.fatal("Unknown visual type");
		break;
	}
	R_ASSERT	(V);
	V->Type		= type;
	return		V;
}

IRender_Visual*	CModelPool::Instance_Duplicate	(IRender_Visual* V)
{
	R_ASSERT(V);
	IRender_Visual* N		= Instance_Create(V->Type);
	N->Copy			(V);
	N->Spawn		();
    // inc ref counter
	for (xr_vector<ModelDef>::iterator I=Models.begin(); I!=Models.end(); I++) 
		if (I->model==V){ I->refs++; break;}
	return N;
}

IRender_Visual*	CModelPool::Instance_Load		(const char* N, BOOL allow_register)
{
	IRender_Visual	*V;
	string512		fn;
	string512		name;

	// Add default ext if no ext at all
	if (0==strext(N))	strconcat	(name,N,".ogf");
	else				strcpy		(name,N);

	// Load data from MESHES or LEVEL
	if (!FS.exist(N))	{
		if (!FS.exist(fn, "$level$", name))
			if (!FS.exist(fn, "$game_meshes$", name)){
#ifdef _EDITOR
				Msg("!Can't find model file '%s'.",name);
                return 0;
#else            
				Debug.fatal("Can't find model file '%s'.",name);
#endif
			}
	} else {
		strcpy			(fn,N);
	}
	
	// Actual loading
	if (bLogging)		Msg		("- Uncached model loading: %s",fn);

	//.
	// if (0!=strstr(fn,"physics\\zabor\\zabor_dver_01"))	__asm int 3;

	IReader*			data	= FS.r_open(fn);
	ogf_header			H;
	data->r_chunk_safe	(OGF_HEADER,&H,sizeof(H));
	V = Instance_Create (H.type);
	V->Load				(N,data,0);
	FS.r_close			(data);
	g_pGamePersistent->RegisterModel(V);

	// Registration
	if (allow_register) Instance_Register(N,V);

	return V;
}

IRender_Visual*	CModelPool::Instance_Load(LPCSTR name, IReader* data, BOOL allow_register)
{
	IRender_Visual	*V;
	
	// Actual loading
	// if (bLogging)	Msg		("- Uncached model loading: %s",name);
	ogf_header			H;
	data->r_chunk_safe	(OGF_HEADER,&H,sizeof(H));
	V = Instance_Create (H.type);
	V->Load				(name,data,0);

	// Registration
	if (allow_register) Instance_Register(name,V);
	return V;
}

void		CModelPool::Instance_Register(LPCSTR N, IRender_Visual* V)
{
	// Registration
	ModelDef			M;
	M.name				= N;
	M.model				= V;
	Models.push_back	(M);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
void CModelPool::Destroy()
{
	// Pool
	Pool.clear			();

	// Registry
	while(!Registry.empty()){
		REGISTRY_IT it	= Registry.begin();
		IRender_Visual* V=(IRender_Visual*)it->first;
#ifdef _DEBUG
		Msg				("ModelPool: Destroy object: '%s'",*V->dbg_name);
#endif
		DeleteInternal	(V,TRUE);
	}
	VERIFY				(Models.empty());
/*
	for (REGISTRY_IT it=Registry.begin(); it!=Registry.end(); it++){
		IRender_Visual* V = (IRender_Visual*)it->first;
		xr_delete	(V);
		xr_free		(it->second);
	}
	Registry.clear();

	// Base/Reference
	xr_vector<ModelDef>::iterator	I;
	for (I=Models.begin(); I!=Models.end(); I++){
		I->model->Release();
		xr_delete(I->model);
	}
	Models.clear();
*/

	// cleanup motions container
	g_pMotionsContainer->clean(false);
}

CModelPool::CModelPool()
{
	bLogging				= TRUE;
    bForceDiscard 			= FALSE;
    bAllowChildrenDuplicate	= TRUE; 
	g_pMotionsContainer		= xr_new<motions_container>();
}

CModelPool::~CModelPool()
{
	Destroy					();
	xr_delete				(g_pMotionsContainer);
}

IRender_Visual* CModelPool::Instance_Find(LPCSTR N)
{
	IRender_Visual*				Model=0;
	xr_vector<ModelDef>::iterator	I;
	for (I=Models.begin(); I!=Models.end(); I++)
	{
		if (I->name[0]&&(0==xr_strcmp(*I->name,N))) {
			Model = I->model;
			break;
		}
	}
	return Model;
}

IRender_Visual* CModelPool::Create(const char* name, IReader* data)
{
#ifdef _EDITOR
	if (!name||!name[0])	return 0;
#endif
	string1024 low_name;	VERIFY	(xr_strlen(name)<sizeof(low_name));
	strcpy(low_name,name);	strlwr	(low_name);
	if (strext(low_name))	*strext	(low_name)=0;
//	Msg						("-CREATE %s",low_name);

	// 0. Search POOL
	POOL_IT	it			=	Pool.find	(low_name);
	if (it!=Pool.end())
	{
		// 1. Instance found
        IRender_Visual*		Model	= it->second;
		Model->Spawn		();
		Pool.erase			(it);
		return				Model;
	} else {
		// 1. Search for already loaded model (reference, base model)
		IRender_Visual* Base		= Instance_Find		(low_name);

		if (0==Base){
			// 2. If not found
			bAllowChildrenDuplicate	= FALSE;
			if (data)		Base = Instance_Load(low_name,data,TRUE);
            else			Base = Instance_Load(low_name,TRUE);
			bAllowChildrenDuplicate	= TRUE;
#ifdef _EDITOR
			if (!Base)		return 0;
#endif
		}
        // 3. If found - return (cloned) reference
        IRender_Visual*		Model	= Instance_Duplicate(Base);
        Registry.insert		(mk_pair(Model,xr_strdup(low_name)));
        return				Model;
	}
}

IRender_Visual* CModelPool::CreateChild(LPCSTR name, IReader* data)
{
	string256 low_name;		VERIFY	(xr_strlen(name)<256);
	strcpy(low_name,name);	strlwr	(low_name);
	if (strext(low_name))	*strext	(low_name) = 0;

	// 1. Search for already loaded model
	IRender_Visual* Base	= Instance_Find(low_name);
	if (0==Base) Base	 	= Instance_Load(name,data,FALSE);

    IRender_Visual* Model	= bAllowChildrenDuplicate?Instance_Duplicate(Base):Base;
    return					Model;
}

extern ENGINE_API BOOL				g_bRendering; 
void	CModelPool::DeleteInternal	(IRender_Visual* &V, BOOL bDiscard)
{
	VERIFY					(!g_bRendering);
    if (V)					V->Depart();
//	bDiscard = false;
	if (bDiscard||bForceDiscard){
    	Discard	(V); 
	}else{
		//
		REGISTRY_IT	it		= Registry.find	(V);
		if (it!=Registry.end())
		{
			// Registry entry found - move it to pool
			Pool.insert			(mk_pair(it->second,V));
		} else {
			// Registry entry not-found - just special type of visual / particles / etc.
			xr_delete			(V);
		}
	}
	V	=	NULL;
}

void	CModelPool::Delete			(IRender_Visual* &V, BOOL bDiscard)
{
	if	(g_bRendering)		{
		VERIFY					(!bDiscard);
		ModelsToDelete.push_back(V);
	} else {
		DeleteInternal			(V,bDiscard);
	}
	V	=	NULL;
}

void	CModelPool::DeleteQueue		()
{
	for (u32 it=0; it<ModelsToDelete.size(); it++)
		DeleteInternal(ModelsToDelete[it]);
	ModelsToDelete.clear	();
}

void	CModelPool::Discard	(IRender_Visual* &V)
{
	//
	REGISTRY_IT	it		= Registry.find	(V);
	if (it!=Registry.end()){
		// Pool - OK

		// Base
		LPCSTR	name	= it->second;
		for (xr_vector<ModelDef>::iterator I=Models.begin(); I!=Models.end(); I++){
			if (I->name[0] && (0==xr_strcmp(*I->name,name))){
            	VERIFY(I->refs>0);
            	I->refs--; 
                if (0==I->refs){
                	bForceDiscard	= TRUE;
	            	I->model->Release();
					xr_delete		(I->model);	
					Models.erase	(I);
                    bForceDiscard	= FALSE;
                }
				break;
			}
		}

		// Registry
		xr_delete		(V);	
		xr_free			(name);
		Registry.erase	(it);
	} else {
		// Registry entry not-found - just special type of visual / particles / etc.
		xr_delete		(V);
	}
	V	=	NULL;
}

void CModelPool::Prefetch()
{
	Logging					(FALSE);
	// prefetch visuals
	string256 section;
	strconcat				(section,"prefetch_visuals_",g_pGamePersistent->m_game_params.m_game_type);
	CInifile::Sect& sect	= pSettings->r_section(section);
	for (CInifile::SectIt I=sect.begin(); I!=sect.end(); I++)	{
		CInifile::Item& item= *I;
		IRender_Visual* V	= Create(item.first.c_str());
		Delete				(V,FALSE);
	}
	Logging					(TRUE);
}

void CModelPool::ClearPool()
{
	POOL_IT	_I			=	Pool.begin();
	POOL_IT	_E			=	Pool.end();
	for (;_I!=_E;_I++)	{
		Discard	(_I->second)	;
	}
	Pool.clear			();
}

IRender_Visual* CModelPool::CreatePE	(PS::CPEDef* source)
{
	PS::CParticleEffect* V	= (PS::CParticleEffect*)Instance_Create(MT_PARTICLE_EFFECT);
	V->Compile		(source);
	return V;
}

IRender_Visual* CModelPool::CreatePG	(PS::CPGDef* source)
{
	PS::CParticleGroup* V	= (PS::CParticleGroup*)Instance_Create(MT_PARTICLE_GROUP);
	V->Compile		(source);
	return V;
}

void CModelPool::dump()
{
	Log	("--- model pool --- begin:");
	u32 sz					= 0;
	u32 k					= 0;
	for (xr_vector<ModelDef>::iterator I=Models.begin(); I!=Models.end(); I++) {
		CKinematics* K		= PKinematics(I->model);
		if (K){
			u32 cur			= K->mem_usage	(false);
			sz				+= cur;
			Msg("#%3d: [%3d/%5d Kb] - %s",k++,I->refs,cur/1024,I->name.c_str());
		}
	}
	Msg ("--- models: %d, mem usage: %d Kb ",k,sz/1024);
	sz						= 0;
	k						= 0;
	for (REGISTRY_IT it=Registry.begin(); it!=Registry.end(); it++)
	{
		CKinematics* K		= PKinematics((IRender_Visual*)it->first);
		if (K){
			u32 cur			= K->mem_usage	(true);
			sz				+= cur;
			Msg("#%3d: [%5d Kb] - %s",k++,cur/1024,it->second);
		}
	}
	Msg ("--- instances: %d, mem usage: %d Kb ",k,sz/1024);
	Log	("--- model pool --- end.");
}

#ifdef _EDITOR
IC bool	_IsBoxVisible(IRender_Visual* visual, const Fmatrix& transform)
{
    Fbox 		bb; 
    bb.xform	(visual->vis.box,transform);
    return 		::Render->occ_visible(bb);
}
IC bool	_IsValidShader(IRender_Visual* visual, u32 priority, bool strictB2F)
{
	if (visual->shader)
        return (priority==visual->shader->E[0]->flags.iPriority)&&(strictB2F==visual->shader->E[0]->flags.bStrictB2F);
    return false;
}

void 	CModelPool::Render(IRender_Visual* m_pVisual, const Fmatrix& mTransform, int priority, bool strictB2F, float m_fLOD)
{
    // render visual
    xr_vector<IRender_Visual*>::iterator I,E;
    switch (m_pVisual->Type){
    case MT_SKELETON_ANIM:
    case MT_SKELETON_RIGID:
    case MT_HIERRARHY:{
        if (_IsBoxVisible(m_pVisual,mTransform)){
            FHierrarhyVisual* pV		= dynamic_cast<FHierrarhyVisual*>(m_pVisual); VERIFY(pV);
            I = pV->children.begin		();
            E = pV->children.end		();
            for (; I!=E; I++){
		        if (_IsValidShader(*I,priority,strictB2F)){
	                RCache.set_Shader		((*I)->shader?(*I)->shader:Device.m_WireShader);
    	            RCache.set_xform_world	(mTransform);
        	        (*I)->Render		 	(m_fLOD);
                }
            }
        }
    }break;
    case MT_PARTICLE_GROUP:{
        PS::CParticleGroup* pG			= dynamic_cast<PS::CParticleGroup*>(m_pVisual); VERIFY(pG);
        if (_IsBoxVisible(m_pVisual,mTransform)){
            RCache.set_xform_world	  		(mTransform);
            for (PS::CParticleGroup::SItemVecIt i_it=pG->items.begin(); i_it!=pG->items.end(); i_it++){
                xr_vector<IRender_Visual*>	visuals;
                i_it->GetVisuals			(visuals);
                for (xr_vector<IRender_Visual*>::iterator it=visuals.begin(); it!=visuals.end(); it++)
                    Render					(*it,Fidentity,priority,strictB2F,m_fLOD);
            }
        }
    }break;
    case MT_PARTICLE_EFFECT:{
        if (_IsBoxVisible(m_pVisual,mTransform)){
            if (_IsValidShader(m_pVisual,priority,strictB2F)){
                RCache.set_Shader			(m_pVisual->shader?m_pVisual->shader:Device.m_WireShader);
                RCache.set_xform_world		(mTransform);
                m_pVisual->Render		 	(m_fLOD);
            }
        }
    }break;
    default:
        if (_IsBoxVisible(m_pVisual,mTransform)){
            if (_IsValidShader(m_pVisual,priority,strictB2F)){
                RCache.set_Shader			(m_pVisual->shader?m_pVisual->shader:Device.m_WireShader);
                RCache.set_xform_world		(mTransform);
                m_pVisual->Render		 	(m_fLOD);
            }
        }
        break;
    }
}

void 	CModelPool::RenderSingle(IRender_Visual* m_pVisual, const Fmatrix& mTransform, float m_fLOD)
{
	for (int p=0; p<4; p++){
    	Render(m_pVisual,mTransform,p,false,m_fLOD);
    	Render(m_pVisual,mTransform,p,true,m_fLOD);
    }
}
void CModelPool::OnDeviceDestroy()
{
	Destroy();
}
#endif
