//----------------------------------------------------
// file: CEditObject.h
//----------------------------------------------------
#ifndef _INCDEF_Object_H_
#define _INCDEF_Object_H_

#include "CustomObject.h"
//----------------------------------------------------
struct 	SRayPickInfo;
class 	CEditableMesh;
class 	CFrustum;
class 	Shader;
class 	ETextureCore;
class 	CSMotion;
class 	COMotion;
class 	CCustomMotion;
class	CBone;

struct st_Surface{
	char			name[MAX_OBJ_NAME];
    AStringVec		textures;       //
    AStringVec		vmaps;			// ���������� ������ ��������!!!
    AnsiString		sh_name;
    Shader*			shader;
    BYTE			sideflag;
    DWORD			dwFVF;
    bool			has_alpha; 		// use internal (only for render)
    				st_Surface		(){ZeroMemory(this,sizeof(st_Surface));}
				    ~st_Surface		();
    ERenderPriority	RenderPriority	();
};
DEFINE_VECTOR	(st_Surface*,SurfaceVec,SurfaceIt);
DEFINE_VECTOR	(CEditableMesh*,EditMeshVec,EditMeshIt);
DEFINE_VECTOR	(CBone*,BoneVec,BoneIt);
DEFINE_VECTOR	(COMotion*,OMotionVec,OMotionIt);
DEFINE_VECTOR	(CSMotion*,SMotionVec,SMotionIt);

#define EOBJECT_LS_RENDERBUFFER		0x0001

struct st_AnimParam{
    float			t;
    float			min_t;
    float			max_t;
    void			Set		(CCustomMotion* M);
    float			Frame	()			{ return t;}
    void			Update	(float dt);
};

class CEditableObject{
	friend class CSceneObject;
	friend class CEditableMesh;
    friend class TfrmPropertiesObject;
    friend class CSector;
    friend class TUI_ControlSectorAdd;
    friend class CLibObject;

// general
	AnsiString		m_ClassScript;

	SurfaceVec		m_Surfaces;
	EditMeshVec		m_Meshes;
	BoneVec			m_Bones;
	SMotionVec		m_SMotions;
	OMotionVec		m_OMotions;

    COMotion*		m_ActiveOMotion;
    st_AnimParam	m_OMParam;

    CSMotion*		m_ActiveSMotion;
    st_AnimParam	m_SMParam;
protected:
	// options
	int 			m_DynamicObject;

	// bounding volume
	Fbox 			m_Box;

public:
    // temp variables for transformation
	Fvector 		t_vPosition;
    Fvector			t_vScale;
    Fvector			t_vRotate;
protected:
    st_Version		m_ObjVer;

    void 			ClearGeometry			();

    void			ClearRenderBuffers		();

    bool			Import_LWO				(const char* fname, bool bNeedOptimize);
    void			UpdateRenderBuffers		();

	void 			UpdateBoneParenting		();
public:
    DWORD			m_LoadState;

	CLibObject*		m_LibParent;
public:
    // constructor/destructor methods
					CEditableObject			(CLibObject* parent);
	virtual 		~CEditableObject		();

#ifdef _EDITOR
    LPCSTR			GetName					();
    void			SetName					(LPCSTR name);
#endif

    IC EditMeshIt	FirstMesh				()	{return m_Meshes.begin();}
    IC EditMeshIt	LastMesh				()	{return m_Meshes.end();}
    IC EditMeshVec& Meshes					()	{return m_Meshes; }	
    IC int			MeshCount				()	{return m_Meshes.size();}
    IC SurfaceIt	FirstSurface			()	{return m_Surfaces.begin();}
    IC SurfaceIt	LastSurface				()	{return m_Surfaces.end();}
    IC int			SurfaceCount			()	{return m_Surfaces.size();}
    IC BoneIt		FirstBone				()	{return m_Bones.begin();}
    IC BoneIt		LastBone				()	{return m_Bones.end();}
    IC int			BoneCount				()	{return m_Bones.size();}
    IC CBone*		GetBone					(DWORD idx){VERIFY(idx<m_Bones.size()); return m_Bones[idx];}
    void			GetBoneWorldTransform	(DWORD bone_idx, float t, CSMotion* motion, Fmatrix& matrix);
    IC SMotionIt	FirstSMotion			()	{return m_SMotions.begin();}
    IC SMotionIt	LastSMotion				()	{return m_SMotions.end();}
    IC int			SMotionCount 			()	{return m_SMotions.size();}
    IC OMotionIt	FirstOMotion			()	{return m_OMotions.begin();}
    IC OMotionIt	LastOMotion				()	{return m_OMotions.end();}
    IC int			OMotionCount 			()	{return m_OMotions.size();}

///    IC bool			CheckVersion			()  {if(m_LibRef) return (m_ObjVer==m_LibRef->m_ObjVer); return true;}
    // get object properties methods
	IC bool 		IsDynamic     			()	{return (m_DynamicObject!=0); }
    IC void			SetDynamic				(bool bDynamic){m_DynamicObject=bDynamic;}
	IC AnsiString&	GetClassScript			()	{return m_ClassScript;}
    IC const Fbox&	GetBox					() 	{return m_Box;}

    // animation
    IC bool			IsOMotionable			()	{return !!m_OMotions.size();}
    IC bool			IsOMotionActive			()	{return IsOMotionable()&&m_ActiveOMotion; }
	void			SetActiveOMotion		(COMotion* mot, bool upd_t=true);
    IC bool			IsSkeleton				()	{return !!m_Bones.size();}
    IC bool			IsSMotionActive			()	{return IsSkeleton()&&m_ActiveSMotion; }
	void			SetActiveSMotion		(CSMotion* mot);
	bool 			CheckBoneCompliance		(CSMotion* M);

    virtual void 	ResetAnimation			(bool upd_t=true);
    void			CalculateAnimation		(bool bGenInvMat=false);

    // statistics methods
    int 			GetFaceCount			();
	int 			GetVertexCount			();
    int 			GetSurfFaceCount		(const char* surf_name);

    // render methods
//	virtual bool 	IsRender				(Fmatrix& parent);
	virtual void 	Render					(Fmatrix& parent, ERenderPriority flag);
	void 			RenderSelection			(Fmatrix& parent);
	void 			RenderEdge				(Fmatrix& parent, CEditableMesh* m=0);
	void 			RenderBones				(const Fmatrix& parent);
	void 			RenderAnimation			(const Fmatrix& parent);
	void 			RenderSingle			(Fmatrix& parent);

    // update methods
	virtual void 	RTL_Update				(float dT);
	void 			UpdateBox				();
	void		    LightenObject			();

    // pick methods
#ifdef _EDITOR
    void 			BoxPick					(const Fbox& box, Fmatrix& parent, SBoxPickInfoVec& pinf);
	bool 			RayPick					(float& dist, Fvector& S, Fvector& D, Fmatrix& parent, SRayPickInfo* pinf=0);
	bool 			FrustumPick				(const CFrustum& frustum, const Fmatrix& parent);
    bool 			SpherePick				(const Fvector& center, float radius, const Fmatrix& parent);
#endif
    // change position/orientation methods
	void 			TranslateToWorld		(const Fmatrix& parent);

    // clone/copy methods
    void			RemoveMesh				(CEditableMesh* mesh);
    void			RemoveOMotion			(const char* name);
    bool			RenameOMotion			(const char* old_name, const char* new_name);
    COMotion*		AppendOMotion			(const char* fname);
    void			ClearOMotions			();
    void			LoadOMotions			(const char* fname);
    void			SaveOMotions			(const char* fname);

    void			RemoveSMotion			(const char* name);
    bool			RenameSMotion			(const char* old_name, const char* new_name);
    CSMotion*		AppendSMotion			(const char* fname);
    bool			ReloadSMotion			(CSMotion* M, const char* fname);
    void			ClearSMotions			();
    void			LoadSMotions			(const char* fname);
    void			SaveSMotions			(const char* fname);

    // load/save methods
	void 			LoadMeshDef				(FSChunkDef *chunk);
	bool 			Load					(const char* fname);
	bool 			LoadObject				(const char* fname);
	void 			SaveObject				(const char* fname);
    CSMotion*		LoadSMotion				(const char* fname);
    COMotion*		LoadOMotion				(const char* fname);
  	bool 			Load					(CStream&);
	void 			Save					(CFS_Base&);

    // contains methods
    CEditableMesh* 	FindMeshByName			(const char* name, CEditableMesh* Ignore=0);
    void			GenerateMeshNames		();
    void			VerifyMeshNames			();
    bool 			ContainsMesh			(const CEditableMesh* m);
	st_Surface*		FindSurfaceByName		(const char* surf_name, int* s_id=0);
    CBone*			FindBoneByName			(const char* name);
    int				GetBoneIndexByWMap		(const char* wm_name);
    COMotion* 		FindOMotionByName		(const char* name, const COMotion* Ignore=0);
    CSMotion* 		FindSMotionByName		(const char* name, const CSMotion* Ignore=0);
    void			GenerateOMotionName		(char* buffer, const char* start_name, const COMotion* M);
    void			GenerateSMotionName		(char* buffer, const char* start_name, const CSMotion* M);

    // device dependent routine
	void 			OnDeviceCreate 			();
	void 			OnDeviceDestroy			();
};
//----------------------------------------------------
#endif /*_INCDEF_EditObject_H_*/


