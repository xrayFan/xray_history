#pragma once

#include "..\PSLibrary.h"

#include "portal.h"
#include "hom.h"
#include "detailmanager.h"
#include "glowmanager.h"
#include "wallmarksengine.h"
#include "fstaticrender_types.h"
#include "fstaticrender_scenegraph.h"
#include "fstaticrender_rendertarget.h"
#include "modelpool.h"

#include "lightShadows.h"
#include "lightProjector.h"
#include "lightPPA.h"
#include "FLightsController.h"

// defs
const int max_patches = 512;

// definition
class CRender	:	public IRender_interface
{
public:
	// Dynamic scene graph
	SceneGraph::mapNormal_T									mapNormal	[4];	// 4=priority
	SceneGraph::mapMatrix_T									mapMatrix;
	SceneGraph::mapSorted_T									mapSorted;
	SceneGraph::mapHUD_T									mapHUD;
	SceneGraph::mapLOD_T									mapLOD;
	SceneGraph::vecPatches_T								vecPatches;

	xr_vector<int>											vecGroups;
	xr_vector<SceneGraph::mapNormalCodes::TNode*>			lstCodes;
	xr_vector<SceneGraph::mapNormalVS::TNode*>				lstVS;
	xr_vector<SceneGraph::mapNormalPS::TNode*>				lstPS;
	xr_vector<SceneGraph::mapNormalCS::TNode*>				lstCS;
	xr_vector<SceneGraph::mapNormalTextures::TNode*>		lstTextures;
	xr_vector<SceneGraph::mapNormalTextures::TNode*>		lstTexturesTemp;
	xr_vector<SceneGraph::mapNormalVB::TNode*>				lstVB;
	xr_vector<SceneGraph::mapNormalMatrices::TNode*>		lstMatrices;
	xr_vector<SceneGraph::_LodItem>							lstLODs;
	xr_vector<IRender_Visual*>								lstVisuals;
	xr_vector<ISpatial*>									lstRenderables;

	ref_geom												hGeomPatches;

	// Sector detection and visibility
	CSector*												pLastSector;
	Fvector													vLastCameraPos;
	xr_vector<IRender_Portal*>								Portals;
	xr_vector<IRender_Sector*>								Sectors;
	CDB::MODEL*												rmPortals;
	CHOM													HOM;
	
	// Global vertex-buffer container
	typedef svector<D3DVERTEXELEMENT9,MAXD3DDECLLENGTH+1>	VertexDeclarator;
	xr_vector<VertexDeclarator>								DCL;
	xr_vector<IDirect3DVertexBuffer9*>						VB;
	xr_vector<IDirect3DIndexBuffer9*>						IB;
	xr_vector<IRender_Visual*>								Visuals;
	CPSLibrary												PSLibrary;

	CLightDB_Static*										L_DB;
	CLightPPA_Manager*										L_Dynamic;
	CLightShadows*											L_Shadows;
	CLightProjector*										L_Projector;
	CGlowManager*											L_Glows;
	CWallmarksEngine*										Wallmarks;
	CDetailManager*											Details;
	CModelPool*												Models;

	CRenderTarget*											Target;			// Render-target

	ref_matrix												matDetailTexturing;
	ref_matrix												matFogPass;
private:
	// Loading / Unloading
	void							LoadBuffers				(IReader	*fs);
	void							LoadVisuals				(IReader	*fs);
	void							LoadLights				(IReader	*fs);
	void							LoadPortals				(IReader	*fs);
	void							LoadSectors				(IReader	*fs);
	void							LoadTrees				(IReader	*fs);

	BOOL							add_Dynamic				(IRender_Visual	*pVisual, u32 planes);	// normal processing
	void							add_Static				(IRender_Visual	*pVisual, u32 planes);
	void							add_leafs_Dynamic		(IRender_Visual	*pVisual);					// if detected node's full visibility
	void							add_leafs_Static		(IRender_Visual	*pVisual);					// if detected node's full visibility
	void							InsertSG_Dynamic		(IRender_Visual	*pVisual, Fvector& Center);
	void							InsertSG_Static			(IRender_Visual	*pVisual);

	void							flush_Patches			();
	void							flush_Models			();
	void							flush_LODs				();

	void							calc_DetailTexturing	();
	void							calc_FogPass			();
public:
	IRender_Portal*					getPortal				(int id);

public:
	// Loading / Unloading
	virtual	void					create					();
	virtual	void					destroy					();

	virtual	void					level_Load				();
	virtual void					level_Unload			();
	
	// Information
	virtual IRender_Sector*			getSector				(int id);
	virtual IRender_Visual*			getVisual				(int id);
	virtual D3DVERTEXELEMENT9*		getVB_Format			(int id);
	virtual IDirect3DVertexBuffer9*	getVB					(int id);
	virtual IDirect3DIndexBuffer9*	getIB					(int id);
	virtual IRender_Sector*			detectSector			(Fvector& P);
	virtual IRender_Target*			getTarget				();
	
	// Main 
	virtual void					flush					();
	virtual void					set_Object				(IRenderable*		O	);
	virtual void					add_Visual				(IRender_Visual*	V	);			// add visual leaf (no culling performed at all)
	virtual void					add_Geometry			(IRender_Visual*	V	);			// add visual(s)	(all culling performed)
	virtual void					add_Wallmark			(ref_shader& S, const Fvector& P, float s, CDB::TRI* T);
	
	//
	virtual IBlender*				blender_create			(CLASS_ID cls);
	virtual void					blender_destroy			(IBlender* &);

	//
	virtual IRender_ObjectSpecific*	ros_create				(IRenderable* parent);
	virtual void					ros_destroy				(IRender_ObjectSpecific* &);

	// Particle library
	virtual CPSLibrary*				ps_library				(){return &PSLibrary;}

	// Lighting
	virtual IRender_Light*			light_create			();
	virtual void					light_destroy			(IRender_Light* &);
	
	// Models
	virtual IRender_Visual*			model_CreatePS			(LPCSTR name, PS::SEmitter* E);
	virtual IRender_Visual*			model_CreateParticles	(LPCSTR name);
	virtual IRender_DetailModel*	model_CreateDM			(IReader*	F);
	virtual IRender_Visual*			model_Create			(LPCSTR name);
	virtual IRender_Visual*			model_Create			(LPCSTR name, IReader* data);
	virtual IRender_Visual*			model_Duplicate			(IRender_Visual*	V);
	virtual void					model_Delete			(IRender_Visual* &	V, BOOL bDiscard);
	virtual void 					model_Delete			(IRender_DetailModel* & F);
	virtual void					model_Logging			(BOOL bEnable)				{ Models->Logging(bEnable);	}
	
	// Occlusion culling
	virtual BOOL					occ_visible				(vis_data&	V);
	virtual BOOL					occ_visible				(Fbox&		B);
	virtual BOOL					occ_visible				(sPoly&		P);
	
	// Main
	virtual void					Calculate				();
	virtual void					Render					();
	virtual void					RenderBox				(IRender_Sector* S, Fbox& BB, int sh);
	virtual void					Screenshot				(LPCSTR postfix=0, BOOL bSquare=FALSE);
	
	// Render mode
	virtual void					rmNear					();
	virtual void					rmFar					();
	virtual void					rmNormal				();

	// Constructor/destructor/loader
	CRender							();
	virtual ~CRender				();
};

extern CRender						RImplementation;
