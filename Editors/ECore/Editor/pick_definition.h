//----------------------------------------------------
// file: SceneClassList.h
//----------------------------------------------------

#ifndef pick_definitionH
#define pick_definitionH
//----------------------------------------------------

class CEditableObject;
class CEditableMesh;
class CCustomObject;

#ifdef _EDITOR
	class SPickQuery{
    	Flags32				m_Flags;
    public:
        Fvector				m_Start;
        Fvector				m_Direction;
        float				m_Dist;
        Fbox 				m_BB; 
    public:
    	class SResult{
		public:            
        	Fvector			verts[3];
            float			u,v;      
            float			range;
            CEditableObject*e_obj;
            CEditableMesh*	e_mesh;
            u32				tag;
            bool operator <	(const SResult& F)const{return range<F.range;}
            SResult			(const SResult& F)
            {
                verts[0]	= F.verts[0];
                verts[1]	= F.verts[1];
                verts[2]	= F.verts[2];
                u			= F.u;
                v			= F.v;
                range		= F.range;
                tag			= F.tag;
                e_obj		= F.e_obj;
                e_mesh		= F.e_mesh;
            }
            SResult			(const Fmatrix& parent, CDB::RESULT* r, CEditableObject* obj, CEditableMesh* mesh)
            {
                parent.transform_tiny(verts[0],r->verts[0]);
                parent.transform_tiny(verts[1],r->verts[1]);
                parent.transform_tiny(verts[2],r->verts[2]);
                u			= r->u;
                v			= r->v;
                range		= r->range;
                tag			= r->dummy;
                e_obj		= obj;
                e_mesh		= mesh;
            }
            SResult			(CDB::RESULT* r, CEditableObject* obj, CEditableMesh* mesh)
            { 
            	verts[0]	= r->verts[0];
            	verts[1]	= r->verts[1];
            	verts[2]	= r->verts[2];
                u			= r->u;
                v			= r->v;
                range		= r->range;
                tag			= r->dummy;
                e_obj		= obj;
                e_mesh		= mesh;
            }
        };
		DEFINE_VECTOR(SResult,ResultVec,ResultIt);
    protected:
    	ResultVec			results;
    public:
        IC void	prepare_rq	(const Fvector& start, const Fvector& dir, float dist, u32 flags)
        {
            m_Start.set		(start);
            m_Direction.set	(dir);
            m_Dist			= dist;
            m_Flags.assign	(flags);
        	results.clear	();          
        }
        IC void	prepare_bq	(const Fbox& bbox, u32 flags)
        {
        	m_BB.set		(bbox);
            m_Flags.assign	(flags);
        	results.clear	();
        }
		IC void append_mtx	(const Fmatrix& parent, CDB::RESULT* R, CEditableObject* obj, CEditableMesh* mesh)
        {
            SResult	D		(parent, R, obj, mesh);
            if (m_Flags.is(CDB::OPT_ONLYNEAREST)&&!results.empty()){
	            SResult& S	= results.back();
                if (D.range<S.range) S = D;
            }else			results.push_back	(D);
        }
		IC void append		(CDB::RESULT* R, CEditableObject* obj, CEditableMesh* mesh)
        {
            SResult	D		(R,obj,mesh);
            if (m_Flags.is(CDB::OPT_ONLYNEAREST)&&!results.empty()){
	            SResult& S	= results.back();
                if (D.range<S.range) S = D;
            }else			results.push_back	(D);
        }
        IC int r_count		()
        {
        	return results.size();
        }
        IC SResult* r_begin	()
        {
        	return results.begin();
        }
        IC SResult* r_end	()
        {
        	return results.end();
        }
        IC void r_clear		()
        {
        	results.clear	();
        }
        IC void r_sort		()
        {
        	std::sort		(results.begin(),results.end());
        }
    };
	struct SRayPickInfo{
		CDB::RESULT 		inf;
		CCustomObject*		s_obj;
		CEditableObject*	e_obj;
		CEditableMesh*		e_mesh;
		Fvector     		pt;
		SRayPickInfo		(){Reset();}
		IC void Reset		(){ ZeroMemory(this,sizeof(SRayPickInfo));inf.range = 5000;}
		IC void SetRESULT	(CDB::MODEL* M, CDB::RESULT* R){inf=*R;inf.id=(M->get_tris()+inf.id)->dummy;}
	};
    DEFINE_VECTOR(CDB::RESULT,BPInfVec,BPInfIt);
	struct SBoxPickInfo{
    	BPInfVec			inf;
		CCustomObject*		s_obj;
		CEditableObject*	e_obj;
		CEditableMesh*		e_mesh;
		SBoxPickInfo		(){Reset();}
		IC void Reset		(){ZeroMemory(this,sizeof(SBoxPickInfo));}
		IC void AddRESULT	(CDB::MODEL* M, CDB::RESULT* R){inf.push_back(*R); inf.back().id=(M->get_tris()+inf.back().id)->dummy;}
	};
	DEFINE_VECTOR(SBoxPickInfo,SBoxPickInfoVec,SBoxPickInfoIt);
#endif
//----------------------------------------------------
#endif // pick_definitionH


