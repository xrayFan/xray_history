//----------------------------------------------------
// file: DrawUtils.h
//----------------------------------------------------

#ifndef DrawUtilsH
#define DrawUtilsH
//----------------------------------------------------
// Utilities
//----------------------------------------------------
class ECORE_API CDUInterface
{
public:
    //----------------------------------------------------
    virtual void __stdcall DrawCross			(const Fvector& p, float szx1, float szy1, float szz1, float szx2, float szy2, float szz2, u32 clr, bool bRot45=false)=0;
    virtual void __stdcall DrawCross			(const Fvector& p, float sz, u32 clr, bool bRot45=false)=0;
    virtual void __stdcall DrawEntity			(u32 clr, ref_shader s)=0;
    virtual void __stdcall DrawFlag				(const Fvector& p, float heading, float height, float sz, float sz_fl, u32 clr, bool bDrawEntity)=0;
    virtual void __stdcall DrawRomboid			(const Fvector& p, float radius, u32 clr)=0;
    virtual void __stdcall DrawJoint			(const Fvector& p, float radius, u32 clr)=0;

    virtual void __stdcall DrawSpotLight	   	(const Fvector& p, const Fvector& d, float range, float phi, u32 clr)=0;
    virtual void __stdcall DrawDirectionalLight	(const Fvector& p, const Fvector& d, float radius, float range, u32 clr)=0;
    virtual void __stdcall DrawPointLight	   	(const Fvector& p, float radius, u32 clr)=0;

    virtual void __stdcall DrawSound		   	(const Fvector& p, float radius, u32 clr)=0;
    virtual void __stdcall DrawLineSphere	   	(const Fvector& p, float radius, u32 clr, bool bCross)=0;

	virtual void __stdcall dbgDrawPlacement		(const Fvector& p, int sz, u32 clr, LPCSTR caption=0, u32 clr_font=0xffffffff)=0;
    virtual void __stdcall dbgDrawVert			(const Fvector& p0, u32 clr, LPCSTR caption=0)=0;
    virtual void __stdcall dbgDrawEdge			(const Fvector& p0,	const Fvector& p1, u32 clr, LPCSTR caption=0)=0;
    virtual void __stdcall dbgDrawFace			(const Fvector& p0,	const Fvector& p1, const Fvector& p2, u32 clr, LPCSTR caption=0)=0;

    virtual void __stdcall DrawFace				(const Fvector& p0,	const Fvector& p1, const Fvector& p2, u32 clr_s, u32 clr_w, BOOL bSolid, BOOL bWire)=0;
    virtual void __stdcall DrawLine				(const Fvector& p0,	const Fvector& p1, u32 clr)=0;
    virtual void __stdcall DrawLine				(const Fvector* p, u32 clr)=0;
    virtual void __stdcall DrawLink				(const Fvector& p0, const Fvector& p1, float sz, u32 clr)=0;
    virtual void __stdcall DrawFaceNormal	   	(const Fvector& p0, const Fvector& p1, const Fvector& p2, float size, u32 clr)=0;
    virtual void __stdcall DrawFaceNormal	   	(const Fvector* p, float size, u32 clr)=0;
    virtual void __stdcall DrawFaceNormal	   	(const Fvector& C, const Fvector& N, float size, u32 clr)=0;
    virtual void __stdcall DrawSelectionBox		(const Fvector& center, const Fvector& size, u32* c=0)=0;
    virtual void __stdcall DrawSelectionBox		(const Fbox& box, u32* c=0)=0;
    virtual void __stdcall DrawIdentSphere		(bool bSolid, bool bWire, u32 clr_s, u32 clr_w)=0;
    virtual void __stdcall DrawIdentSpherePart	(bool bSolid, bool bWire, u32 clr_s, u32 clr_w)=0;
    virtual void __stdcall DrawIdentCone	   	(bool bSolid, bool bWire, u32 clr_s, u32 clr_w)=0;
	virtual void __stdcall DrawIdentCylinder   	(bool bSolid, bool bWire, u32 clr_s, u32 clr_w)=0;
    virtual void __stdcall DrawIdentBox			(bool bSolid, bool bWire, u32 clr_s, u32 clr_w)=0;

    virtual void __stdcall DrawBox				(const Fvector& offs, const Fvector& Size, bool bSolid, bool bWire, u32 clr_s, u32 clr_w)=0;
    virtual void __stdcall DrawAABB				(const Fvector& p0, const Fvector& p1, u32 clr_s, u32 clr_w, BOOL bSolid, BOOL bWire)=0;
    virtual void __stdcall DrawAABB				(const Fmatrix& parent, const Fvector& center, const Fvector& size, u32 clr_s, u32 clr_w, BOOL bSolid, BOOL bWire)=0;
    virtual void __stdcall DrawOBB				(const Fmatrix& parent, const Fobb& box, u32 clr_s, u32 clr_w)=0;
	virtual void __stdcall DrawSphere		   	(const Fmatrix& parent, const Fvector& center, float radius, u32 clr_s, u32 clr_w, BOOL bSolid, BOOL bWire)=0;
	virtual void __stdcall DrawSphere		   	(const Fmatrix& parent, const Fsphere& S, u32 clr_s, u32 clr_w, BOOL bSolid, BOOL bWire)=0;
	virtual void __stdcall DrawCylinder			(const Fmatrix& parent, const Fvector& center, const Fvector& dir, float height, float radius, u32 clr_s, u32 clr_w, BOOL bSolid, BOOL bWire)=0;
	virtual void __stdcall DrawCone				(const Fmatrix& parent, const Fvector& apex, const Fvector& dir, float height, float radius, u32 clr_s, u32 clr_w, BOOL bSolid, BOOL bWire)=0;
    virtual void __stdcall DrawPlane		   	(const Fvector& center, const Fvector2& scale, const Fvector& rotate, u32 clr_s, u32 clr_w, bool bCull, BOOL bSolid, BOOL bWire)=0;
    virtual void __stdcall DrawPlane		   	(const Fvector& p, const Fvector& n, const Fvector2& scale, u32 clr_s, u32 clr_w, bool bCull, BOOL bSolid, BOOL bWire)=0;
    virtual void __stdcall DrawRectangle	   	(const Fvector& o, const Fvector& u, const Fvector& v, u32 clr_s, u32 clr_w, BOOL bSolid, BOOL bWire)=0;

    virtual void __stdcall DrawSafeRect			()=0;
    virtual void __stdcall DrawGrid				()=0;
    virtual void __stdcall DrawPivot		   	(const Fvector& pos, float sz=5.f)=0;
	virtual void __stdcall DrawAxis				(const Fmatrix& T)=0;
	virtual void __stdcall DrawObjectAxis	   	(const Fmatrix& T, float sz, bool sel)=0;
	virtual void __stdcall DrawSelectionRect   	(const Ivector2& m_SelStart, const Ivector2& m_SelEnd)=0;

    virtual void __stdcall DrawPrimitiveL	   	(D3DPRIMITIVETYPE pt, u32 pc, Fvector* vertices, int vc, u32 color, bool bCull, bool bCycle)=0;
    virtual void __stdcall DrawPrimitiveTL		(D3DPRIMITIVETYPE pt, u32 pc, FVF::TL* vertices, int vc, bool bCull, bool bCycle)=0;
    virtual void __stdcall DrawPrimitiveLIT		(D3DPRIMITIVETYPE pt, u32 pc, FVF::LIT* vertices, int vc, bool bCull, bool bCycle)=0;

    virtual void __stdcall DrawText				(const Fvector& pos, LPCSTR text, u32 color=0xFF000000, u32 shadow_color=0xFF909090)=0;
};
//----------------------------------------------------
#endif

