#include "stdafx.h"
#include "xr_area.h"
#include "xr_collide_form.h"
#include "xr_object.h"
#include "cl_intersect.h"

#include "igame_level.h"
#include "x_ray.h"
#include "GameFont.h"

using namespace	Collide;

//--------------------------------------------------------------------------------
// RayTest - Occluded/No
//--------------------------------------------------------------------------------
BOOL CObjectSpace::RayTest	( const Fvector &start, const Fvector &dir, float range, Collide::rq_target tgt, Collide::ray_cache* cache)
{
	VERIFY					(_abs(dir.magnitude()-1)<EPS);

	r_results.r_clear		();

	xrc.ray_options			(CDB::OPT_ONLYFIRST);
	Collide::ray_defs Q		(start,dir,range,CDB::OPT_ONLYFIRST,tgt);
	// dynamic test
	if (tgt&rqtDyn) 
	{
		// traverse object database
		g_SpatialSpace->q_ray	(0,STYPE_COLLIDEABLE,start,dir,range);
		// Determine visibility for dynamic part of scene
		for (u32 o_it=0; o_it<g_SpatialSpace->q_result.size(); o_it++)
		{
			ISpatial*	spatial			= g_SpatialSpace->q_result[o_it];
			CObject*	collidable		= dynamic_cast<CObject*>			(spatial);
			if (collidable){
				ECollisionFormType tp	= collidable->collidable.model->Type();
				if ((tgt&rqtObject)&&(tp==cftObject)&&collidable->collidable.model->_RayQuery(Q,r_results))	return TRUE;
				if ((tgt&rqtShape)&&(tp==cftShape)&&collidable->collidable.model->_RayQuery(Q,r_results))	return TRUE;
			}
		}
	}
	// static test
	if (tgt&rqtStatic){
		// If we get here - test static model
		if (cache) 
		{
			// 1. Check cached polygon
			float _u,_v,_range;
			if (CDB::TestRayTri(start,dir,cache->verts,_u,_v,_range,false)) 
			{
				if (_range>0 && _range<range) return TRUE;
			}
			
			// 2. Polygon doesn't pick - real database query
			xrc.ray_query	(&Static,start,dir,range);
			if (0==xrc.r_count()) {
				return FALSE;
			} else {
				// cache polygon
				CDB::RESULT*	R	= xrc.r_begin();
				CDB::TRI&		T	= Static.get_tris() [ R->id ];
				Fvector*		V	= Static.get_verts();
				cache->verts[0].set	(V[T.verts[0]]);
				cache->verts[1].set	(V[T.verts[1]]);
				cache->verts[2].set	(V[T.verts[2]]);
				return TRUE;
			}
		} else {
			xrc.ray_query	(&Static,start,dir,range);
			return xrc.r_count();
		}
	}
	return FALSE;
}

//--------------------------------------------------------------------------------
// RayPick
//--------------------------------------------------------------------------------
BOOL CObjectSpace::RayPick	( const Fvector &start, const Fvector &dir, float range, rq_target tgt, rq_result& R)
{
	r_temp.r_clear		();
	R.O		= 0; R.range = range; R.element = -1;
	// static test
	if (tgt&rqtStatic){ 
		xrc.ray_options		(CDB::OPT_ONLYNEAREST | CDB::OPT_CULL);
		xrc.ray_query		(&Static,start,dir,range);
		if (xrc.r_count())  R.set_if_less(xrc.r_begin());
	}
	// dynamic test
	if (tgt&rqtDyn){ 
		Collide::ray_defs Q		(start,dir,R.range,CDB::OPT_ONLYNEAREST|CDB::OPT_CULL,tgt);
		// traverse object database
		g_SpatialSpace->q_ray	(0,STYPE_COLLIDEABLE,start,dir,range);
		// Determine visibility for dynamic part of scene
		for (u32 o_it=0; o_it<g_SpatialSpace->q_result.size(); o_it++){
			ISpatial*	spatial			= g_SpatialSpace->q_result[o_it];
			CObject*	collidable		= dynamic_cast<CObject*>	(spatial);
			if			(0==collidable)	continue;
			ECollisionFormType tp		= collidable->collidable.model->Type();
			if (((tgt&rqtObject)&&(tp==cftObject))||((tgt&rqtShape)&&(tp==cftShape))){
				u32		C	= D3DCOLOR_XRGB	(64,64,64);
				Q.range		= R.range;
				if (collidable->collidable.model->_RayQuery(Q,r_temp)){
					C		= D3DCOLOR_XRGB(128,128,196);
					R.set_if_less(r_temp.r_begin());
				}
#ifdef DEBUG
				if (bDebug){
					Fsphere	S;		S.P = spatial->spatial.center; S.R = spatial->spatial.radius;
					dbg_S.push_back	(mk_pair(S,C));
				}
#endif
			}
		}
	}
	return (R.element>=0);
}

//--------------------------------------------------------------------------------
// RayQuery
//--------------------------------------------------------------------------------
BOOL CObjectSpace::RayQuery(const Collide::ray_defs& R, Collide::rq_callback* CB, LPVOID user_data)
{
	// initialize query
	r_results.r_clear	();
	r_temp.r_clear		();

	Flags32		sd_test;	sd_test.set		(R.tgt);
	rq_target	next_test	= R.tgt;

	rq_result	s_res;
	ray_defs	s_rd(R.start,R.dir,R.range,CDB::OPT_ONLYNEAREST|CDB::OPT_CULL,R.tgt);
	ray_defs	d_rd(R.start,R.dir,R.range,CDB::OPT_ONLYNEAREST|CDB::OPT_CULL,R.tgt);
	ray_defs	p_rd(R.start,R.dir,R.range,CDB::OPT_ONLYNEAREST|CDB::OPT_CULL,R.tgt);

	s_res.set				(0,s_rd.range,-1);
	do{
		if ((R.tgt&rqtStatic)&&sd_test.is(rqtStatic)&&(next_test&rqtStatic)){ 
			s_res.set		(0,s_rd.range,-1);
			// Test static model
			xrc.ray_options	(s_rd.flags);
			xrc.ray_query	(&Static,s_rd.start,s_rd.dir,s_rd.range);
			if (xrc.r_count()){	
				if (s_res.set_if_less(xrc.r_begin())){
					s_rd.range	-=	(s_res.range+EPS_L);
					s_rd.start.mad	(s_rd.dir,s_res.range+EPS_L);
					s_res.range	= R.range-s_rd.range-EPS_L;
				}
			}
			if (!s_res.valid())	sd_test.set(rqtStatic,FALSE);
		}
		if ((R.tgt&rqtDyn)&&sd_test.is(rqtDyn)&&(next_test&rqtDyn)){ 
			r_temp.r_clear	();
			// Traverse object database
			g_SpatialSpace->q_ray	(0,STYPE_COLLIDEABLE,d_rd.start,d_rd.dir,d_rd.range);
			// Determine visibility for dynamic part of scene
			for (u32 o_it=0; o_it<g_SpatialSpace->q_result.size(); o_it++){
				CObject*	collidable		= dynamic_cast<CObject*>	(g_SpatialSpace->q_result[o_it]);
				if			(0==collidable)	continue;
				ICollisionForm*	cform		= collidable->collidable.model;
				ECollisionFormType tp		= collidable->collidable.model->Type();
				if (((R.tgt&rqtObject)&&(tp==cftObject))||((R.tgt&rqtShape)&&(tp==cftShape))){
					cform->_RayQuery(d_rd,r_temp);
				}
			}
			if (r_temp.r_count()){
				rq_result& d_res = *r_temp.r_begin();
				d_rd.range	-= (d_res.range+EPS_L);
				d_rd.start.mad(d_rd.dir,d_res.range+EPS_L);
				d_res.range	= R.range-d_rd.range-EPS_L;
			}else{
				sd_test.set(rqtObject,FALSE);
			}
		}
		if (s_res.valid()&&r_temp.r_count()){
			if	(s_res.range<r_temp.r_begin()->range){
				BOOL need_calc	= CB?CB(s_res,user_data):TRUE;
				next_test		= need_calc?rqtStatic:rqtNone;
				r_results.append_result	(s_res);
			}else{
				BOOL need_calc	= CB?CB(*r_temp.r_begin(),user_data):TRUE;
				next_test		= need_calc?rqtObject:rqtNone;	
				r_results.append_result	(*r_temp.r_begin());
			}
		}else if (s_res.valid()){
			BOOL need_calc		= CB?CB(s_res,user_data):TRUE;
			next_test			= need_calc?rqtStatic:rqtNone;
			r_results.append_result		(s_res);
		}else if (r_temp.r_count()){
			BOOL need_calc		= CB?CB(*r_temp.r_begin(),user_data):TRUE;
			next_test			= need_calc?rqtObject:rqtNone;
			r_results.append_result		(*r_temp.r_begin());
		}else{
			next_test			= rqtNone;
		}
		if ((R.flags&CDB::OPT_ONLYFIRST)||(R.flags&CDB::OPT_ONLYNEAREST)) break;
	}while(next_test!=rqtNone);
	return r_results.r_count();
}

BOOL CObjectSpace::RayQuery(ICollisionForm* target, const Collide::ray_defs& R)
{
	VERIFY(target);
	r_results.r_clear();
	return target->_RayQuery(R,r_results);
}
