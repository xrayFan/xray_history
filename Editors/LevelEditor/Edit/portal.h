#ifndef PortalH
#define PortalH

// refs
class CSector;

class CPortal: public CCustomObject {
	friend class TfrmPropertiesPortal;
    friend class CSector;
    friend class TfraPortal;
    friend class SceneBuilder;
    friend class CPortalUtils;

	FvectorVec		m_Vertices;
	FvectorVec		m_SimplifyVertices;
    Fvector 		m_Center;
    float 			m_Radius;
    Fvector 		m_Normal;
    CSector*		m_SectorFront;
    CSector*		m_SectorBack;
public:
					CPortal		(LPVOID data, LPCSTR name);
	void 			Construct	(LPVOID data);
	virtual 		~CPortal	();
    virtual bool	CanAttach	() {return false;}

    virtual void 	Render		(int priority, bool strictB2F);
	virtual bool 	RayPick		(float& distance,const Fvector& start,const Fvector& direction,
								SRayPickInfo* pinf = NULL);
    virtual bool 	FrustumPick	(const CFrustum& frustum);
	virtual void 	Move		( Fvector& amount ); // need for Shift Level
  	virtual bool 	Load		(IReader&);
	virtual void 	Save		(IWriter&);
	virtual bool 	GetBox		(Fbox& box);
    void			Simplify	();
    bool			Update		(bool bLoadMode=false);
    void 			InvertOrientation(bool bUndo);

    FvectorVec&		Vertices()	{return m_Vertices;}
    void			SetSectors	(CSector* sf, CSector* sb){m_SectorFront=sf; m_SectorBack=sb;}

    virtual bool 	GetSummaryInfo(SSceneSummary* inf);
};

#endif /*_INCDEF_Portal_H_*/

