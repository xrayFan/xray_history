#ifndef _RENDER_H_
#define _RENDER_H_

#include "frustum.h"
#include "vis_common.h"

// refs
class ENGINE_API	IRenderable;
class ENGINE_API	IRender_Visual;
class ENGINE_API	IBlender;
//class ENGINE_API	CPSLibrary;
namespace PS	{ 
	struct ENGINE_API SDef;
	struct ENGINE_API SEmitter; 
};

const	float		fLightSmoothFactor = 4.f;

//////////////////////////////////////////////////////////////////////////
// definition (Detail Model)
class	ENGINE_API	IRender_DetailModel
{
public:
	struct fvfVertexIn	{
		Fvector P;
		float	u,v;
	};
	struct fvfVertexOut	{
		Fvector P;
		u32		C;
		float	u,v;
	};
public:
	Fsphere		bv_sphere;
	Fbox		bv_bb;
	u32			flags;
	float		s_min;
	float		s_max;

	ref_shader	shader;
	fvfVertexIn	*vertices;
	u32			number_vertices;
	u16			*indices;
	u32			number_indices;
public:
	virtual void					transfer			(Fmatrix& mXform, fvfVertexOut* vDest, u32 C, u16* iDest, u32 iOffset)	= 0;
	virtual ~IRender_DetailModel()	{};
};

//////////////////////////////////////////////////////////////////////////
// definition (Dynamic Light)
class	ENGINE_API	IRender_Light		{
public:
	enum LT
	{
		POINT	= 0,
		SPOT	= 1
	};
public:
	virtual void					set_type			(LT type)							= 0;
	virtual void					set_active			(bool)								= 0;
	virtual bool					get_active			()									= 0;
	virtual void					set_shadow			(bool)								= 0;
	virtual void					set_position		(const Fvector& P)					= 0;
	virtual void					set_direction		(const Fvector& P)					= 0;
	virtual void					set_cone			(float angle)						= 0;
	virtual void					set_range			(float R)							= 0;
	virtual void					set_texture			(LPCSTR name)						= 0;
	virtual void					set_color			(const Fcolor& C)					= 0;
	virtual void					set_color			(float r, float g, float b)			= 0;
	virtual ~IRender_Light()		{};
};

//////////////////////////////////////////////////////////////////////////
// definition (Per-object render-specific data)
class	ENGINE_API	IRender_ObjectSpecific		{
public:
	virtual ~IRender_ObjectSpecific()	{};
};

//////////////////////////////////////////////////////////////////////////
// definition (Portal)
class	ENGINE_API	IRender_Portal				{
public:
	virtual ~IRender_Portal()			{};
};

//////////////////////////////////////////////////////////////////////////
// definition (Sector)
class	ENGINE_API	IRender_Sector				{
public:
	virtual ~IRender_Sector()			{};
};

//////////////////////////////////////////////////////////////////////////
// definition (Target)
class	ENGINE_API	IRender_Target				{
public:
	virtual void					eff_load			(LPCSTR n)							= 0;
	virtual	void					set_blur			(float f)							= 0;
	virtual	void					set_gray			(float f)							= 0;
	virtual void					set_duality_h		(float f)							= 0;
	virtual void					set_duality_v		(float f)							= 0;
	virtual void					set_noise			(float f)							= 0;
	virtual void					set_noise_scale		(float f)							= 0;
	virtual void					set_noise_color		(u32 f)								= 0;
	virtual void					set_noise_fps		(float f)							= 0;
	virtual u32						get_width			()									= 0;
	virtual u32						get_height			()									= 0;
	virtual ~IRender_Target()		{};
};

//////////////////////////////////////////////////////////////////////////
// definition (Renderer)
class	ENGINE_API	IRender_interface
{
public:
	// Data
	IRenderable*					val_pObject;
	Fmatrix*						val_pTransform;
	BOOL							val_bHUD;
	BOOL							val_bInvisible;
	CFrustum						ViewBase;
	CFrustum*						View;
	u32								marker;
public:
	// Loading / Unloading
	virtual	void					create					()								= 0;
	virtual	void					destroy					()								= 0;

	virtual	void					level_Load				()								= 0;
	virtual void					level_Unload			()								= 0;

	// Information
	virtual int						getVisualsCount			()								= 0;
	virtual IRender_Portal*			getPortal				(int id)						= 0;
	virtual IRender_Sector*			getSector				(int id)						= 0;
	virtual IRender_Sector*			getSectorActive			()								= 0;
	virtual IRender_Visual*			getVisual				(int id)						= 0;
	virtual D3DVERTEXELEMENT9*		getVB_Format			(int id)						= 0;
	virtual IDirect3DVertexBuffer9*	getVB					(int id)						= 0;
	virtual IDirect3DIndexBuffer9*	getIB					(int id)						= 0;
	virtual IRender_Sector*			detectSector			(Fvector& P)					= 0;
	virtual IRender_Target*			getTarget				()								= 0;

	// Main 
	IC		void					set_Frustum				(CFrustum*	O	)				{ VERIFY(O);	View = O;			}
	IC		void					set_Transform			(Fmatrix*	M	)				{ VERIFY(M);	val_pTransform = M;	}
	IC		void					set_HUD					(BOOL 		V	)				{ val_bHUD		= V;				}
	IC		BOOL					get_HUD					()								{ return val_bHUD;					}
	IC		void					set_Invisible			(BOOL 		V	)				{ val_bInvisible= V;				}
	IC		BOOL					get_Invisible			()								{ return val_bInvisible;			}
	virtual void					flush					()								= 0;	
	virtual void					set_Object				(IRenderable*		O	)		= 0;
	virtual void					add_Visual				(IRender_Visual*	V	)		= 0;	// add visual leaf	(no culling performed at all)
	virtual void					add_Geometry			(IRender_Visual*	V	)		= 0;	// add visual(s)	(all culling performed)
	virtual void					add_Patch				(ref_shader& S, const Fvector& P, float s, float a, BOOL bNearer)	= 0;
	virtual void					add_Wallmark			(ref_shader& S, const Fvector& P, float s, CDB::TRI* T)				= 0;

	virtual IBlender*				blender_create			(CLASS_ID cls)								= 0;
	virtual void					blender_destroy			(IBlender* &)								= 0;

	virtual IRender_ObjectSpecific*	ros_create				(IRenderable* parent)						= 0;
	virtual void					ros_destroy				(IRender_ObjectSpecific* &)					= 0;

	// Lighting
	virtual IRender_Light*			light_create			()											= 0;
	virtual void					light_destroy			(IRender_Light* &)							= 0;

	// Models
	virtual IRender_Visual*			model_CreatePS			(LPCSTR name, PS::SEmitter* E)				= 0;
	virtual IRender_Visual*			model_CreatePE			(LPCSTR name)								= 0;
	virtual IRender_Visual*			model_CreatePG			(LPCSTR name)								= 0;
	virtual IRender_Visual*			model_CreateParticles	(LPCSTR name)								= 0;
	virtual IRender_DetailModel*	model_CreateDM			(IReader*	F)								= 0;
	virtual IRender_Visual*			model_Create			(LPCSTR name)								= 0;
	virtual IRender_Visual*			model_Create			(LPCSTR name, IReader* data)				= 0;
	virtual IRender_Visual*			model_Duplicate			(IRender_Visual*	V)						= 0;
	virtual void					model_Delete			(IRender_Visual* &	V, BOOL bDiscard=FALSE)	= 0;
	virtual void 					model_Delete			(IRender_DetailModel* & F)					= 0;
	virtual void					model_Logging			(BOOL bEnable)								= 0;

	// Occlusion culling
	virtual BOOL					occ_visible				(vis_data&	V)								= 0;
	virtual BOOL					occ_visible				(Fbox&		B)								= 0;
	virtual BOOL					occ_visible				(sPoly&		P)								= 0;

	// Main
	virtual void					Calculate				()											= 0;
	virtual void					Render					()											= 0;
	virtual void					RenderBox				(IRender_Sector* S, Fbox& BB, int sh)		= 0;
	virtual void					Screenshot				(LPCSTR postfix=0, BOOL bSquare=FALSE)		= 0;

	// Render mode
	virtual void					rmNear					()											= 0;
	virtual void					rmFar					()											= 0;
	virtual void					rmNormal				()											= 0;

	// Constructor/destructor
	virtual ~IRender_interface();
};

extern ENGINE_API	IRender_interface*	Render;

#endif