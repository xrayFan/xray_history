//----------------------------------------------------
// file: ExportSkeleton.cpp
//----------------------------------------------------
#include "stdafx.h"
#pragma hdrstop

#include "ExportSkeleton.h"
#include "EditObject.h"
#include "EditMesh.h"
#include "fmesh.h"
#include "std_classes.h"
#include "bone.h"
#include "motion.h"
#include "MgcCont3DMinBox.h"         
#include "ui_main.h"
#include "SkeletonAnimated.h"

u32 CSkeletonCollectorPacked::VPack(SSkelVert& V){
    u32 P 	= 0xffffffff;

    u32 ix,iy,iz;
    ix = iFloor(float(V.P.x-m_VMmin.x)/m_VMscale.x*clpSMX);
    iy = iFloor(float(V.P.y-m_VMmin.y)/m_VMscale.y*clpSMY);
    iz = iFloor(float(V.P.z-m_VMmin.z)/m_VMscale.z*clpSMZ);
    R_ASSERT(ix<=clpSMX && iy<=clpSMY && iz<=clpSMZ);

	int similar_pos=-1;
    {
        U32Vec& vl=m_VM[ix][iy][iz];
        for(U32It it=vl.begin();it!=vl.end(); it++){
        	SSkelVert& src=m_Verts[*it];
        	if(src.similar_pos(V)){
	            if(src.similar(V)){
                    P = *it;
                    break;
            	}
                similar_pos=*it;
            }
        }
    }
    if (0xffffffff==P)
    {
    	if (similar_pos>=0) V.P.set(m_Verts[similar_pos].P);
        P = m_Verts.size();
        m_Verts.push_back(V);

        m_VM[ix][iy][iz].push_back(P);

        u32 ixE,iyE,izE;
        ixE = iFloor(float(V.P.x+m_VMeps.x-m_VMmin.x)/m_VMscale.x*clpSMX);
        iyE = iFloor(float(V.P.y+m_VMeps.y-m_VMmin.y)/m_VMscale.y*clpSMY);
        izE = iFloor(float(V.P.z+m_VMeps.z-m_VMmin.z)/m_VMscale.z*clpSMZ);

        R_ASSERT(ixE<=clpSMX && iyE<=clpSMY && izE<=clpSMZ);

        if (ixE!=ix)							m_VM[ixE][iy][iz].push_back	(P);
        if (iyE!=iy)							m_VM[ix][iyE][iz].push_back	(P);
        if (izE!=iz)							m_VM[ix][iy][izE].push_back	(P);
        if ((ixE!=ix)&&(iyE!=iy))				m_VM[ixE][iyE][iz].push_back(P);
        if ((ixE!=ix)&&(izE!=iz))				m_VM[ixE][iy][izE].push_back(P);
        if ((iyE!=iy)&&(izE!=iz))				m_VM[ix][iyE][izE].push_back(P);
        if ((ixE!=ix)&&(iyE!=iy)&&(izE!=iz))	m_VM[ixE][iyE][izE].push_back(P);
    }
    return P;
}

CSkeletonCollectorPacked::CSkeletonCollectorPacked(const Fbox &_bb, int apx_vertices, int apx_faces)
{
	Fbox bb;		bb.set(_bb); bb.grow(EPS_L);
    // Params
    m_VMscale.set	(bb.max.x-bb.min.x, bb.max.y-bb.min.y, bb.max.z-bb.min.z);
    m_VMmin.set		(bb.min);
    m_VMeps.set		(m_VMscale.x/clpSMX/2,m_VMscale.y/clpSMY/2,m_VMscale.z/clpSMZ/2);
    m_VMeps.x		= (m_VMeps.x<EPS_L)?m_VMeps.x:EPS_L;
    m_VMeps.y		= (m_VMeps.y<EPS_L)?m_VMeps.y:EPS_L;
    m_VMeps.z		= (m_VMeps.z<EPS_L)?m_VMeps.z:EPS_L;

    // Preallocate memory
    m_Verts.reserve	(apx_vertices);
    m_Faces.reserve	(apx_faces);

    int		_size	= (clpSMX+1)*(clpSMY+1)*(clpSMZ+1);
    int		_average= (apx_vertices/_size)/2;
    for (int ix=0; ix<clpSMX+1; ix++)
        for (int iy=0; iy<clpSMY+1; iy++)
            for (int iz=0; iz<clpSMZ+1; iz++)
                m_VM[ix][iy][iz].reserve	(_average);
}
//----------------------------------------------------

CExportSkeleton::SSplit::SSplit(CSurface* surf, const Fbox& bb):CSkeletonCollectorPacked(bb)
{
	m_Texture 	= surf->_Texture();
	m_Shader	= surf->_ShaderName();
    I_Current	= V_Minimal = -1;
}
//----------------------------------------------------

void CExportSkeleton::SSplit::Save(IWriter& F, BOOL b2Link)
{
    // Header
    F.open_chunk		(OGF_HEADER);
    ogf_header			H;
    H.format_version	= xrOGF_FormatVersion;
    H.type				= (I_Current>=0)?MT_SKELETON_GEOMDEF_PM:MT_SKELETON_GEOMDEF_ST;
    H.shader_id			= 0;
    F.w					(&H,sizeof(H));
    F.close_chunk		();

    // Texture
    F.open_chunk		(OGF_TEXTURE);
    F.w_stringZ			(m_Texture);
    F.w_stringZ			(m_Shader);
    F.close_chunk		();

    // Vertices
    m_Box.invalidate	();
    F.open_chunk		(OGF_VERTICES);
    F.w_u32			(b2Link?2*0x12071980:1*0x12071980);
    F.w_u32			(m_Verts.size());
    if (b2Link){
        for (SkelVertIt v_it=m_Verts.begin(); v_it!=m_Verts.end(); v_it++){
            SSkelVert& pV 	= *v_it;
            m_Box.modify(pV.P);
			// write vertex
            F.w_u16(pV.B0);
            F.w_u16(pV.B1);
            F.w(&(pV.O0),sizeof(Fvector));		// position (offset)
            F.w(&(pV.N0),sizeof(Fvector));		// normal
            F.w(&(pV.O1),sizeof(Fvector));		// position (offset)
            F.w(&(pV.N1),sizeof(Fvector));		// normal
            F.w_float(pV.w);
            F.w_float(pV.UV.x); F.w_float(pV.UV.y);	// tu,tv
        }
    }else{
        for (SkelVertIt v_it=m_Verts.begin(); v_it!=m_Verts.end(); v_it++){
            SSkelVert& pV 	= *v_it;
            m_Box.modify(pV.P);
            F. w(&(pV.O0),sizeof(float)*3);		// position (offset)
            F.w(&(pV.N0),sizeof(float)*3);		// normal
            F.w_float(pV.UV.x); F.w_float(pV.UV.y);		// tu,tv
            F.w_u32(pV.B0);
        }
    }
    F.close_chunk();

    // Faces
    F.open_chunk(OGF_INDICES);
    F.w_u32(m_Faces.size()*3);
    F.w(m_Faces.begin(),m_Faces.size()*3*sizeof(WORD));
    F.close_chunk();

    // PMap
    if (I_Current>=0) {
        F.open_chunk(OGF_P_MAP);
        {
            F.open_chunk(0x1);
            F.w_u32(V_Minimal);
            F.w_u32(I_Current);
            F.close_chunk();
        }
        {
            F.open_chunk(0x2);
            F.w(pmap_vsplit.begin(),pmap_vsplit.size()*sizeof(Vsplit));
            F.close_chunk();
        }
        {
            F.open_chunk(0x3);
            F.w_u32(pmap_faces.size());
            F.w(pmap_faces.begin(),pmap_faces.size()*sizeof(WORD));
            F.close_chunk();
        }
        F.close_chunk();
    }

    // BBox (already computed)
    F.open_chunk(OGF_BBOX);
    F.w(&m_Box,sizeof(Fvector)*2);
    F.close_chunk();
}

void CExportSkeleton::SSplit::MakeProgressive(){
    I_Current=V_Minimal=-1;
    if (m_Faces.size()>1) {
        // Options
        PM_Init(1,1,4,0.1f,1,1,120,0.15f,0.95f);

        // Transfer vertices
        for (SkelVertIt vert_it=m_Verts.begin(); vert_it!=m_Verts.end(); vert_it++){
            SSkelVert	&iV = *vert_it;
            PM_CreateVertex(iV.P.x,iV.P.y,iV.P.z,vert_it - m_Verts.begin(),(P_UV*)(&iV.UV));
        }

        // Convert
        PM_Result R;
        I_Current = PM_Convert((LPWORD)m_Faces.begin(),m_Faces.size()*3, &R);
        if (I_Current>=0) {
            u32 progress_diff = m_Verts.size()-R.minVertices;
            if (progress_diff!=R.splitSIZE){
                ELog.Msg(mtError,"PM_Convert return wrong indices.");
                I_Current = -1;
                return;
            }
            // Permute vertices
            SkelVertVec temp_list = m_Verts;

            // Perform permutation
            for(u32 i=0; i<temp_list.size(); i++)
                m_Verts[R.permutePTR[i]]=temp_list[i];

            // Copy results
            pmap_vsplit.resize(R.splitSIZE);
            CopyMemory(pmap_vsplit.begin(),R.splitPTR,R.splitSIZE*sizeof(Vsplit));

            pmap_faces.resize(R.facefixSIZE);
            CopyMemory(pmap_faces.begin(),R.facefixPTR,R.facefixSIZE*sizeof(WORD));

            V_Minimal = R.minVertices;
        }
    }
}

CExportSkeleton::CExportSkeleton(CEditableObject* object)
{
	m_Source=object;
}
//----------------------------------------------------

void CExportSkeletonCustom::ComputeOBB	(Fobb &B, FvectorVec& V)
{
    if (V.size()<3) { B.invalidate(); return; }
    Mgc::Box3	BOX		= Mgc::MinBox(V.size(), (const Mgc::Vector3*) V.begin());
    B.m_rotate.i.set	(BOX.Axis(0));
    B.m_rotate.j.set	(BOX.Axis(1));
    B.m_rotate.k.set	(BOX.Axis(2));
    B.m_translate.set	(BOX.Center());
    B.m_halfsize.set	(BOX.Extents()[0],BOX.Extents()[1],BOX.Extents()[2]);
}
//----------------------------------------------------

int CExportSkeletonCustom::FindSplit(LPCSTR shader, LPCSTR texture)
{
	for (SplitIt it=m_Splits.begin(); it!=m_Splits.end(); it++){
		if ((0==stricmp(*it->m_Texture,texture))&&(0==stricmp(*it->m_Shader,shader))) return it-m_Splits.begin();
    }
    return -1;
}
//----------------------------------------------------

bool CExportSkeleton::ExportGeometry(IWriter& F)
{
    if( m_Source->MeshCount() == 0 ) return false;

    if (m_Source->BoneCount()<1){
    	ELog.Msg(mtError,"There are no bones in the object.");
     	return false;
    }

    if (m_Source->BoneCount()>64){
    	ELog.Msg(mtError,"Object cannot handle more than 64 bones.");
     	return false;
    }

    // mem active motion
    CSMotion* active_motion=m_Source->ResetSAnimation();

    R_ASSERT(m_Source->IsDynamic()&&m_Source->IsSkeleton());
    BOOL b2Link = FALSE;

    UI->ProgressStart(5+m_Source->MeshCount()*2+m_Source->SurfaceCount(),"Export skeleton geometry...");
    UI->ProgressInc();

    xr_vector<FvectorVec>	bone_points;
	bone_points.resize	(m_Source->BoneCount());

    u32 mtl_cnt=0;
	UI->SetStatus("Split meshes...");
    for(EditMeshIt mesh_it=m_Source->FirstMesh();mesh_it!=m_Source->LastMesh();mesh_it++){
        CEditableMesh* MESH = *mesh_it;
        // generate vertex offset
        if (!MESH->m_LoadState.is(CEditableMesh::LS_SVERTICES)) MESH->GenerateSVertices();
	    UI->ProgressInc();
        // fill faces
        for (SurfFacesPairIt sp_it=MESH->m_SurfFaces.begin(); sp_it!=MESH->m_SurfFaces.end(); sp_it++){
            IntVec& face_lst = sp_it->second;
            CSurface* surf = sp_it->first;
            u32 dwTexCnt = ((surf->_FVF()&D3DFVF_TEXCOUNT_MASK)>>D3DFVF_TEXCOUNT_SHIFT);
            R_ASSERT(dwTexCnt==1);
            int mtl_idx = FindSplit(surf->_ShaderName(),surf->_Texture());
            if (mtl_idx<0){
            	m_Splits.push_back(SSplit(surf,m_Source->GetBox()));
                mtl_idx=mtl_cnt++;
            }
            SSplit& split=m_Splits[mtl_idx];
            for (IntIt f_it=face_lst.begin(); f_it!=face_lst.end(); f_it++){
            	int f_idx = *f_it;
                st_Face& face = MESH->m_Faces[f_idx];
                {
                    SSkelVert v[3];
                    for (int k=0; k<3; k++){
                        st_FaceVert& 	fv = face.pv[k];
                        st_SVert& 		sv = MESH->m_SVertices[f_idx*3+k];
                        v[k].set(MESH->m_Points[fv.pindex],sv.uv,sv.w);
                        v[k].set0(sv.offs0,sv.norm0,sv.bone0);
                        if (sv.bone1!=-1){
                        	b2Link = TRUE;
        	                v[k].set1(sv.offs1,sv.norm1,sv.bone1);
                        }else{
        	                v[k].set1(sv.offs0,sv.norm0,sv.bone0);
                        }
                    }
                    split.add_face(v[0], v[1], v[2]);
			        if (surf->m_Flags.is(CSurface::sf2Sided)){
                    	v[0].N0.invert(); v[1].N0.invert(); v[2].N0.invert();
                    	v[0].N1.invert(); v[1].N1.invert(); v[2].N1.invert();
                    	split.add_face(v[0], v[2], v[1]);
                    }
                }
            }
            if (!split.valid()){
				ELog.Msg(mtError,"Degenerate split found (Material '%s'). Removed.",surf->_Name());
                m_Splits.pop_back();
            }
        }
        // mesh fin
        MESH->UnloadSVertices();
        MESH->UnloadFNormals();
	    UI->ProgressInc();
    }
    UI->SetStatus("Make progressive...");
    // fill per bone vertices
    for (SplitIt split_it=m_Splits.begin(); split_it!=m_Splits.end(); split_it++){
		SkelVertVec& lst = split_it->getV_Verts();
	    for (SkelVertIt sv_it=lst.begin(); sv_it!=lst.end(); sv_it++){
		    bone_points[sv_it->B0].push_back(sv_it->O0);
        }
        if (m_Source->m_Flags.is(CEditableObject::eoProgressive)) split_it->MakeProgressive();
		UI->ProgressInc();
    }
	UI->ProgressInc();

	// create OGF

	// Saving geometry...
    Fbox rootBB;    rootBB.invalidate();

    // Header
    ogf_header 		H;
    H.format_version= xrOGF_FormatVersion;
    H.type			= m_Source->SMotionCount()?MT_SKELETON_ANIM:MT_SKELETON_RIGID;
    H.shader_id		= 0;
    F.w_chunk		(OGF_HEADER,&H,sizeof(H));

    // Desc
    ogf_desc		desc;
    m_Source->PrepareOGFDesc(desc);
    F.open_chunk	(OGF_DESC);
    desc.Save		(F);
    F.close_chunk	();
	
    // OGF_CHILDREN
    F.open_chunk	(OGF_CHILDREN);
    int chield=0;
    for (split_it=m_Splits.begin(); split_it!=m_Splits.end(); split_it++){
	    F.open_chunk(chield++);
        split_it->Save(F,b2Link);
	    F.close_chunk();
		rootBB.merge(split_it->m_Box);
    }
    F.close_chunk();
    UI->ProgressInc();


    UI->SetStatus("Compute bounding volume...");
    // BBox (already computed)
    F.open_chunk(OGF_BBOX);
    F.w(&rootBB,sizeof(Fbox));
    F.close_chunk();
	UI->ProgressInc();

    // BoneNames
    F.open_chunk(OGF_BONE_NAMES);
    F.w_u32(m_Source->BoneCount());
    int bone_idx=0;
    for (BoneIt bone_it=m_Source->FirstBone(); bone_it!=m_Source->LastBone(); bone_it++,bone_idx++){
        F.w_stringZ	((*bone_it)->Name());
        F.w_stringZ	((*bone_it)->Parent()?(*bone_it)->ParentName():"");
        Fobb	obb;
        ComputeOBB	(obb,bone_points[bone_idx]);
        F.w			(&obb,sizeof(Fobb));
    }
    F.close_chunk();

    bool bRes = true;
                    
    F.open_chunk(OGF_IKDATA);
    for (bone_it=m_Source->FirstBone(); bone_it!=m_Source->LastBone(); bone_it++,bone_idx++)
        if (!(*bone_it)->ExportOGF(F)) bRes=false; 
    F.close_chunk();

    if (!m_Source->GetClassScript().IsEmpty()){
        F.open_chunk(OGF_USERDATA);
        F.w(m_Source->GetClassScript().c_str(),m_Source->GetClassScript().Length());
        F.close_chunk();
    }

	UI->ProgressInc();
    UI->ProgressEnd();

    // restore active motion
    m_Source->SetActiveSMotion(active_motion);

    return bRes;
}
//----------------------------------------------------

bool CExportSkeleton::ExportMotionKeys(IWriter& F)
{
    if (m_Source->SMotionCount()<1){
    	ELog.Msg(mtError,"Object doesn't have any motion.");
     	return false;
    }

    UI->ProgressStart(1+m_Source->SMotionCount(),"Export skeleton motions keys...");
    UI->ProgressInc();

    // mem active motion
    CSMotion* active_motion=m_Source->ResetSAnimation();

    // Motions
    F.open_chunk(OGF_MOTIONS2);
    F.open_chunk(0);
    F.w_u32(m_Source->SMotionCount());
    F.close_chunk();
    int smot = 1;

    // use global transform
    Fmatrix	mGT,mTranslate,mRotate;
    mRotate.setHPB			(m_Source->a_vRotate.y, m_Source->a_vRotate.x, m_Source->a_vRotate.z);
    mTranslate.translate	(m_Source->a_vPosition);
    mGT.mul					(mTranslate,mRotate);

    for (SMotionIt motion_it=m_Source->FirstSMotion(); motion_it!=m_Source->LastSMotion(); motion_it++, smot++){
        CSMotion* motion 	= *motion_it;
        F.open_chunk(smot);
        F.w_stringZ(motion->Name());
        F.w_u32(motion->Length());

        u32 dwLen			= motion->Length();
		CKeyQR* _keysQR 	= xr_alloc<CKeyQR>(dwLen); 
		CKeyQT* _keysQT 	= xr_alloc<CKeyQT>(dwLen); 
		Fvector* _keysT 	= xr_alloc<Fvector>(dwLen);
        
        BoneMotionVec& lst=motion->BoneMotions();
        int bone_id = 0;
        for (BoneMotionIt bm_it=lst.begin(); bm_it!=lst.end(); bm_it++,bone_id++){
            Flags8 flags = motion->GetMotionFlags(bone_id);
            CBone* B 	= m_Source->GetBone(bone_id);
            CBone* PB 	= B->Parent();
            for (int frm=motion->FrameStart(); frm<motion->FrameEnd(); frm++){
                float t = (float)frm/motion->FPS();
                Fvector T,R;
                Fquaternion q;
                motion->_Evaluate	(bone_id,t,T,R);
                B->_Update			(T,R);
                m_Source->CalculateAnimation(B,motion,true);
                Fmatrix mat			= B->_MTransform();
                if (flags.is(st_BoneMotion::flWorldOrient)){
                    Fmatrix 	parent;
                    Fmatrix 	inv_parent;
                    if(PB){
                        m_Source->GetBoneWorldTransform(PB->index,t,motion,parent);
                        inv_parent.invert(parent);
                    }else{
                        parent 		= Fidentity;
                        inv_parent	= Fidentity;
                    }
                    Fmatrix 	rot;
                    rot.setXYZi	(R.x,R.y,R.z);
                    mat.mul		(inv_parent,rot);
                }
                // apply global transform
                if (B->IsRoot()){
                	mGT.transform_tiny(T);
                    mat.mulA(mGT);
                }

                q.set		(mat);

                CKeyQR&	Kr 	= _keysQR[frm-motion->FrameStart()];
                Fvector&Kt 	= _keysT [frm-motion->FrameStart()];
                
                // Quantize quaternion
                int	_x 		= int(q.x*KEY_Quant); clamp(_x,-32767,32767); Kr.x =  _x;
                int	_y 		= int(q.y*KEY_Quant); clamp(_y,-32767,32767); Kr.y =  _y;
                int	_z 		= int(q.z*KEY_Quant); clamp(_z,-32767,32767); Kr.z =  _z;
                int	_w 		= int(q.w*KEY_Quant); clamp(_w,-32767,32767); Kr.w =  _w;
                Kt.set	(T);
            }
            
            // check T
            u8 t_present	= FALSE;
            R_ASSERT		(dwLen);
            Fvector Mt		= {0,0,0};
            Fvector Ct		= {0,0,0};
            Fvector St		= {0,0,0};
            Fvector At		= _keysT[0];
            Fvector Bt		= _keysT[0];
            for (u32 t_idx=0; t_idx<dwLen; t_idx++){
            	Fvector& t	= _keysT[t_idx];
            	Mt.add		(t);
                At.x		= _min(At.x,t.x);
                At.y		= _min(At.y,t.y);
                At.z		= _min(At.z,t.z);
                Bt.x		= _max(Bt.x,t.x);
                Bt.y		= _max(Bt.y,t.y);
                Bt.z		= _max(Bt.z,t.z);
            }
            Mt.div			(dwLen);
            Ct.add			(Bt,At);
            Ct.mul			(0.5f);
            St.sub			(Bt,At);
            St.mul			(0.5f);
            for (t_idx=0; t_idx<dwLen; t_idx++){
            	Fvector& t	= _keysT[t_idx];
                if (!Mt.similar(t,EPS_L)){t_present=TRUE;}
                
                CKeyQT&	Kt 	= _keysQT[t_idx];
                int	_x 		= int(127.f*(t.x-Ct.x)/St.x); clamp(_x,-128,127); Kt.x =  _x;
                int	_y 		= int(127.f*(t.y-Ct.y)/St.y); clamp(_y,-128,127); Kt.y =  _y;
                int	_z 		= int(127.f*(t.z-Ct.z)/St.z); clamp(_z,-128,127); Kt.z =  _z;
            }
            St.div	(127.f);
            // save
            F.w_u8	(t_present);
			F.w_u32	(crc32(_keysQR,dwLen*sizeof(CKeyQR)));
            F.w		(_keysQR,dwLen*sizeof(CKeyQR));
            if (t_present){	
	            F.w_u32(crc32(_keysQT,u32(dwLen*sizeof(CKeyQT))));
            	F.w	(_keysQT,dwLen*sizeof(CKeyQT));
	            F.w_fvector3(St);
    	        F.w_fvector3(Ct);
            }else{
                F.w_fvector3(Mt);
            }
        }
        // free temp storage
        xr_free(_keysQR);
        xr_free(_keysQT);
        xr_free(_keysT);

        F.close_chunk();
	    UI->ProgressInc();
    }
    F.close_chunk();
    UI->ProgressEnd();

    // restore active motion
    m_Source->SetActiveSMotion(active_motion);
    return true;
}

bool CExportSkeleton::ExportMotionDefs(IWriter& F)
{
    if (m_Source->SMotionCount()<1){ 
    	ELog.Msg(mtError,"Object doesn't have any motion.");
    	return false;
    }

    bool bRes=true;

    UI->ProgressStart	(3,"Export skeleton motions defs...");
    UI->ProgressInc		();
    // save smparams
    F.open_chunk		(OGF_SMPARAMS2);
    F.w_u16				(xrOGF_SMParamsVersion);
    // bone parts
    BPVec& bp_lst 		= m_Source->BoneParts();
    if (bp_lst.size()){
		if (m_Source->VerifyBoneParts()){
            F.w_u16(bp_lst.size());
            for (BPIt bp_it=bp_lst.begin(); bp_it!=bp_lst.end(); bp_it++){
                F.w_stringZ(bp_it->alias.c_str());
                F.w_u16(bp_it->bones.size());
                for (int i=0; i<int(bp_it->bones.size()); i++){
		        	int idx = m_Source->FindBoneByNameIdx(bp_it->bones[i].c_str()); VERIFY(idx>=0);
                    F.w_u32	(idx);
                }
            }
        }else{
            ELog.Msg	(mtError,"Invalid bone parts (missing or duplicate bones).");
            bRes 		= false;
        }
    }else{
		F.w_u16(1);
		F.w_stringZ("default");
		F.w_u16(m_Source->BoneCount());
        for (int i=0; i<m_Source->BoneCount(); i++) F.w_u32(i);
    }
    UI->ProgressInc		();
    // motion defs
    SMotionVec& sm_lst 		= m_Source->SMotions();
	F.w_u16(sm_lst.size());
    for (SMotionIt motion_it=sm_lst.begin(); motion_it!=sm_lst.end(); motion_it++){
        CSMotion* motion = *motion_it;
        // verify
        if (!motion->m_Flags.is(esmFX)){
            if (!((motion->m_BoneOrPart==BI_NONE)||(motion->m_BoneOrPart<bp_lst.size()))){
                ELog.Msg(mtError,"Invalid Bone Part of motion: '%s'.",motion->Name());
                bRes=false;
                continue;
            }
        }
        if (bRes){
	    	// export
            F.w_stringZ	(motion->Name());
            F.w_u32		(motion->m_Flags.get());
            F.w_u16		(motion->m_BoneOrPart);
            F.w_u16		(motion_it-sm_lst.begin());
            F.w_float	(motion->fSpeed);
            F.w_float	(motion->fPower);
            F.w_float	(motion->fAccrue);
            F.w_float	(motion->fFalloff);
        }
    }
    UI->ProgressInc		();
    F.close_chunk();
    UI->ProgressEnd();
    return bRes;
}

bool CExportSkeleton::ExportMotions(IWriter& F)
{
	if (!ExportMotionKeys(F)) 	return false;
	if (!ExportMotionDefs(F)) 	return false;
    return true;
}
//----------------------------------------------------

bool CExportSkeleton::Export(IWriter& F)
{
    if (!ExportGeometry(F)) 						return false;
    if (m_Source->SMotionCount()&&!ExportMotions(F))return false;
    return true;
};
//----------------------------------------------------



