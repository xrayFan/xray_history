#include "stdafx.h"
#pragma hdrstop

#include "..\psystem.h"
#include "ParticleEffect.h"

#ifdef _EDITOR
	#include "UI_ToolsCustom.h"
	#include "ParticleEffectActions.h"
#endif

using namespace PAPI;
using namespace PS;

//static int action_list_handle=-1;

//------------------------------------------------------------------------------
// class CParticleEffectDef
//------------------------------------------------------------------------------
CPEDef::CPEDef()
{                                         
    strcpy				(m_Name,"unknown");
    m_ShaderName		= 0;
    m_TextureName		= 0;
    m_Frame.InitDefault	();
    m_MaxParticles		= 0;
	m_CachedShader		= 0;
	m_fTimeLimit		= 0.f;
    // collision
    m_fCollideOneMinusFriction 	= 1.f;
    m_fCollideResilience		= 0.f;
    m_fCollideSqrCutoff			= 0.f;
    // velocity scale
    m_VelocityScale.set			(0.f,0.f,0.f);
    // align to path
    m_APDefaultRotation.set		(-PI_DIV_2,0.f,0.f);
	// flags
    m_Flags.zero		();
}

CPEDef::~CPEDef()
{
    xr_free				(m_ShaderName);
    xr_free				(m_TextureName);
//	for (PAVecIt it=m_ActionList.begin(); it!=m_ActionList.end(); it++) xr_delete(*it);
}
void CPEDef::SetName(LPCSTR name)
{
    strcpy				(m_Name,name);
}
void CPEDef::pAlignToPath(float rot_x, float rot_y, float rot_z)
{
	m_Flags.set			(dfAlignToPath,TRUE);
	m_APDefaultRotation.set(rot_x,rot_y,rot_z);
}
void CPEDef::pVelocityScale(float scale_x, float scale_y, float scale_z)
{
	m_Flags.set			(dfVelocityScale,TRUE);
	m_VelocityScale.set	(scale_x, scale_y, scale_z);
}
void CPEDef::pCollision(float friction, float resilience, float cutoff, BOOL destroy_on_contact)
{
	m_fCollideOneMinusFriction 	= 1.f-friction;
	m_fCollideResilience		= resilience;
	m_fCollideSqrCutoff			= cutoff*cutoff;
	m_Flags.set					(dfCollision,TRUE);
	m_Flags.set					(dfCollisionDel,destroy_on_contact);
}

void CPEDef::pSprite(string128& sh_name, string128& tex_name)
{
	xr_free(m_ShaderName);	m_ShaderName	= xr_strdup(sh_name);
	xr_free(m_TextureName);	m_TextureName	= xr_strdup(tex_name);
	m_Flags.set	(dfSprite,TRUE);
}
void CPEDef::pFrame(BOOL random_frame, u32 frame_count, u32 tex_width, u32 tex_height, u32 frame_width, u32 frame_height)
{
	m_Flags.set			(dfFramed,TRUE);
	m_Flags.set			(dfRandomFrame,random_frame);
	m_Frame.Set			(frame_count, (float)tex_width, (float)tex_height, (float)frame_width, (float)frame_height);
}
void CPEDef::pAnimate(float speed, BOOL random_playback)
{
	m_Frame.m_fSpeed	= speed;
	m_Flags.set			(dfAnimated,TRUE);
	m_Flags.set			(dfRandomPlayback,random_playback);
}
void CPEDef::pTimeLimit(float time_limit)
{
	m_Flags.set			(dfTimeLimit,TRUE);
	m_fTimeLimit		= time_limit;
}
void CPEDef::pFrameInitExecute(ParticleEffect *effect)
{
	for(int i = 0; i < effect->p_count; i++){
		Particle &m = effect->particles[i];
		if (m.flags.is(Particle::BIRTH)){
			if (m_Flags.is(dfRandomFrame))
				m.frame	= (u16)iFloor(Random.randI(m_Frame.m_iFrameCount)*255.f);
			if (m_Flags.is(dfAnimated)&&m_Flags.is(dfRandomPlayback)&&Random.randI(2))
				m.flags.set(Particle::ANIMATE_CCW,TRUE);
		}
	}
}
void CPEDef::pAnimateExecute(ParticleEffect *effect, float dt)
{
	float speedFac = m_Frame.m_fSpeed * dt;
	for(int i = 0; i < effect->p_count; i++){
		Particle &m = effect->particles[i];
		float f						= (float(m.frame)/255.f+((m.flags.is(Particle::ANIMATE_CCW))?-1.f:1.f)*speedFac);
		if (f>m_Frame.m_iFrameCount)f-=m_Frame.m_iFrameCount;
		if (f<0.f)					f+=m_Frame.m_iFrameCount;
		m.frame						= (u16)iFloor(f*255.f);
	}
}

void CPEDef::pCollisionExecute(PAPI::ParticleEffect *effect, float dt, CParticleEffect* owner, CollisionCallback cb)
{
	pVector pt,n;
	// Must traverse list in reverse order so Remove will work
	for(int i = effect->p_count-1; i >= 0; i--){
		Particle &m = effect->particles[i];

		bool pick_needed;
		int pick_cnt=0;
		do{		
			pick_needed = false;
			Fvector 	dir;
			dir.sub		(m.pos,m.posB);
			float dist 	= dir.magnitude();
			if (dist>=EPS){
				dir.div	(dist);
#ifdef _EDITOR                
				if (Tools->RayPick(m.posB,dir,dist,&pt,&n)){
#else
				Collide::rq_result	RQ;
				if (g_pGameLevel->ObjectSpace.RayPick(m.posB,dir,dist,Collide::rqtBoth,RQ)){	
					pt.mad	(m.posB,dir,RQ.range);
					if (RQ.O){
						n.set(0.f,1.f,0.f);
					}else{
						CDB::TRI*	T		=  	g_pGameLevel->ObjectSpace.GetStaticTris()+RQ.element;
						Fvector*	verts	=	g_pGameLevel->ObjectSpace.GetStaticVerts();
						n.mknormal(verts[T->verts[0]],verts[T->verts[1]],verts[T->verts[2]]);
					}
#endif
					pick_cnt++;
					if (cb&&(pick_cnt==1)) if (!cb(owner,m,pt,n)) break;
					if (m_Flags.is(dfCollisionDel)) effect->Remove(i);
					else{
						// Compute tangential and normal components of velocity
						float nmag = m.vel * n;
						pVector vn(n * nmag); 	// Normal Vn = (V.N)N
						pVector vt(m.vel - vn);	// Tangent Vt = V - Vn

						// Compute _new velocity heading out:
						// Don't apply friction if tangential velocity < cutoff
						if(vt.length2() <= m_fCollideSqrCutoff){
							m.vel = vt - vn * m_fCollideResilience;
						}else{
							m.vel = vt * m_fCollideOneMinusFriction - vn * m_fCollideResilience;
						}
						m.pos	= m.posB + m.vel * dt; 
						pick_needed = true;
					}
				}
			}else{
				m.pos	= m.posB;
			}
		}while(pick_needed&&(pick_cnt<2));
	}
}

//------------------------------------------------------------------------------
// I/O part
//------------------------------------------------------------------------------
BOOL CPEDef::Load(IReader& F)
{
	R_ASSERT		(F.find_chunk(PED_CHUNK_VERSION));
	u16 version		= F.r_u16();

	if (version!=PED_VERSION)
		return FALSE;

	R_ASSERT		(F.find_chunk(PED_CHUNK_NAME));
	F.r_stringZ		(m_Name);

	R_ASSERT		(F.find_chunk(PED_CHUNK_EFFECTDATA));
	m_MaxParticles	= F.r_u32();

	u32 sz			= F.find_chunk(PED_CHUNK_ACTIONLIST); R_ASSERT(sz);
	m_Actions.w		(F.pointer(),sz);
	/*//!
	m_ActionList.actions.resize(F.r_u32());
	for (PAVecIt it=m_ActionList.begin(); it!=m_ActionList.end(); it++){
	*it			= PAPI::pCreateAction((PActionEnum)F.r_u32());
	(*it)->Load	(F);
	}
	*/

	F.r_chunk		(PED_CHUNK_FLAGS,&m_Flags);

	string256		buf;
	if (m_Flags.is(dfSprite)){
		R_ASSERT	(F.find_chunk(PED_CHUNK_SPRITE));
		F.r_stringZ	(buf); m_ShaderName = xr_strdup(buf);
		F.r_stringZ	(buf); m_TextureName= xr_strdup(buf);
	}

	if (m_Flags.is(dfFramed)){
		R_ASSERT	(F.find_chunk(PED_CHUNK_FRAME));
		F.r			(&m_Frame,sizeof(SFrame));
	}

	if (m_Flags.is(dfTimeLimit)){
		R_ASSERT(F.find_chunk(PED_CHUNK_TIMELIMIT));
		m_fTimeLimit= F.r_float();
	}

	if (m_Flags.is(dfCollision)){
		R_ASSERT(F.find_chunk(PED_CHUNK_COLLISION));
		m_fCollideOneMinusFriction 	= F.r_float();
		m_fCollideResilience		= F.r_float();
		m_fCollideSqrCutoff			= F.r_float();
	}

	if (m_Flags.is(dfVelocityScale)){
		R_ASSERT(F.find_chunk(PED_CHUNK_VEL_SCALE));
		F.r_fvector3				(m_VelocityScale); 
	}

	if (m_Flags.is(dfAlignToPath)){
		if (F.find_chunk(PED_CHUNK_ALIGN_TO_PATH)){
			F.r_fvector3			(m_APDefaultRotation);
		}
	}

	//    m_Flags.and(dfAllFlags);

#ifdef _EDITOR
	if (F.find_chunk(PED_CHUNK_OWNER)){
		AnsiString tmp;
		F.r_stringZ	(m_OwnerName);
		F.r_stringZ	(m_ModifName);
		F.r			(&m_CreateTime,sizeof(m_CreateTime));
		F.r			(&m_ModifTime,sizeof(m_ModifTime));
	}

	if (F.find_chunk(PED_CHUNK_SOURCETEXT)){
		F.r_stringZ	(m_SourceText);
		//        Compile		();
	}

	if (F.find_chunk(PED_CHUNK_EDATA)){
	}    
#endif

	return TRUE;
}

void CPEDef::Save(IWriter& F)
{
	F.open_chunk	(PED_CHUNK_VERSION);
	F.w_u16			(PED_VERSION);
	F.close_chunk	();

	F.open_chunk	(PED_CHUNK_NAME);
	F.w_stringZ		(m_Name);
	F.close_chunk	();

	F.open_chunk	(PED_CHUNK_EFFECTDATA);
	F.w_u32			(m_MaxParticles);
	F.close_chunk	();

	F.open_chunk	(PED_CHUNK_ACTIONLIST);
	F.w				(m_Actions.pointer(),m_Actions.size());
	/*//!
	F.w_u32			(m_ActionList.actions.size());
	for (PAVecIt it=m_ActionList.begin(); it!=m_ActionList.end(); it++){
	F.w_u32		((*it)->type);
	(*it)->Save	(F);
	}
	*/    
	F.close_chunk	();

	F.w_chunk		(PED_CHUNK_FLAGS,&m_Flags,sizeof(m_Flags));

	if (m_Flags.is(dfSprite)){
		F.open_chunk	(PED_CHUNK_SPRITE);
		F.w_stringZ		(m_ShaderName);
		F.w_stringZ		(m_TextureName);
		F.close_chunk	();
	}

	if (m_Flags.is(dfFramed)){
		F.open_chunk	(PED_CHUNK_FRAME);
		F.w				(&m_Frame,sizeof(SFrame));
		F.close_chunk	();
	}

	if (m_Flags.is(dfTimeLimit)){
		F.open_chunk	(PED_CHUNK_TIMELIMIT);
		F.w_float		(m_fTimeLimit);
		F.close_chunk	();
	}

	if (m_Flags.is(dfCollision)){
		F.open_chunk	(PED_CHUNK_COLLISION);
		F.w_float		(m_fCollideOneMinusFriction);
		F.w_float		(m_fCollideResilience);
		F.w_float		(m_fCollideSqrCutoff);
		F.close_chunk	();
	}

	if (m_Flags.is(dfVelocityScale)){
		F.open_chunk	(PED_CHUNK_VEL_SCALE);
		F.w_fvector3	(m_VelocityScale);
		F.close_chunk	();
	}

	if (m_Flags.is(dfAlignToPath)){
		F.open_chunk	(PED_CHUNK_ALIGN_TO_PATH);
		F.w_fvector3	(m_APDefaultRotation);
		F.close_chunk	();
	}
#ifdef _EDITOR
	F.open_chunk	(PED_CHUNK_OWNER);
	F.w_stringZ		(m_OwnerName);
	F.w_stringZ		(m_ModifName);
	F.w				(&m_CreateTime,sizeof(m_CreateTime));
	F.w				(&m_ModifTime,sizeof(m_ModifTime));
	F.close_chunk	();

	F.open_chunk	(PED_CHUNK_SOURCETEXT);
	F.w_stringZ		(m_SourceText.c_str());
	F.close_chunk	();
#endif
}
//------------------------------------------------------------------------------
// class CParticleEffect
//------------------------------------------------------------------------------
CParticleEffect::CParticleEffect()
{
	m_HandleEffect 			= pGenParticleEffects(1, 1);	VERIFY(m_HandleEffect>=0);
	m_HandleActionList		= pGenActionLists();			VERIFY(m_HandleActionList>=0);
	m_RT_Flags.zero			();
	m_Def					= 0;
	m_fElapsedLimit			= 0.f;
	m_MemDT					= 0;
	m_InitialPosition.set	(0,0,0);
	m_DestroyCallback		= 0;
	m_CollisionCallback		= 0;
	m_XFORM.identity		();
}
CParticleEffect::~CParticleEffect()
{
	OnDeviceDestroy			();
	pDeleteParticleEffects	(m_HandleEffect);
	pDeleteActionLists		(m_HandleActionList);
}

void CParticleEffect::Play()
{
	m_RT_Flags.set		(flRT_DefferedStop,FALSE);
	m_RT_Flags.set		(flRT_Playing,TRUE);
	pStartPlaying		(m_HandleActionList);
}
void CParticleEffect::Stop(BOOL bDefferedStop)
{
	if (bDefferedStop){
		m_RT_Flags.set	(flRT_DefferedStop,TRUE);
		pStopPlaying	(m_HandleActionList);
	}else{
		m_RT_Flags.set	(flRT_Playing,FALSE);
		ResetParticles	();
	}
}
void CParticleEffect::RefreshShader()
{
	OnDeviceDestroy();
	OnDeviceCreate();
}

void CParticleEffect::ResetParticles()
{
	if (m_Def){
		pSetMaxParticlesG	(m_HandleEffect,0);
		pSetMaxParticlesG	(m_HandleEffect,m_Def->m_MaxParticles);
	}
}

void CParticleEffect::UpdateParent(const Fmatrix& m, const Fvector& velocity, BOOL bXFORM)
{
	m_RT_Flags.set			(flRT_XFORM, bXFORM);
	if (bXFORM)				m_XFORM.set	(m);
	else{
		m_InitialPosition	= m.c;
		pSetActionListParenting	(m_HandleActionList,m,velocity);
	}
}

static const u32	uDT_STEP = 33;
static const float	fDT_STEP = float(uDT_STEP)/1000.f;

void CParticleEffect::OnFrame(u32 frame_dt)
{
	if (m_Def && m_RT_Flags.is(flRT_Playing)){
		m_MemDT			+= frame_dt;
		for (;m_MemDT>=uDT_STEP; m_MemDT-=uDT_STEP){
			if (m_Def->m_Flags.is(CPEDef::dfTimeLimit)){ 
				if (!m_RT_Flags.is(flRT_DefferedStop)){
					m_fElapsedLimit -= fDT_STEP;
					if (m_fElapsedLimit<0.f){
						m_fElapsedLimit = m_Def->m_fTimeLimit;
						Stop		(true);
					}
				}
			}
			pCurrentEffect		(m_HandleEffect);

			// execute action list
			pCallActionList		(m_HandleActionList,fDT_STEP);

			//			if (action_list_handle>-1) 
			//            	pCallActionList	(action_list_handle);

			ParticleEffect *pg 	= _GetEffectPtr(m_HandleEffect);
			// our actions
			if (m_Def->m_Flags.is(CPEDef::dfFramed))    		  		m_Def->pFrameInitExecute(pg);
			if (m_Def->m_Flags.is(CPEDef::dfFramed|CPEDef::dfAnimated))	m_Def->pAnimateExecute	(pg,fDT_STEP);
			if (m_Def->m_Flags.is(CPEDef::dfCollision)) 				m_Def->pCollisionExecute(pg,fDT_STEP,this,m_CollisionCallback);

			//-move action
			if (pg->p_count)	
			{
				vis.box.invalidate	();
				float p_size = 0.f;
				for(int i = 0; i < pg->p_count; i++){
					Particle &m 	= pg->particles[i]; 
					if (m.flags.is(Particle::DYING)){if (m_DestroyCallback) m_DestroyCallback(this,m);}
					if (m.flags.is(Particle::BIRTH))m.flags.set(Particle::BIRTH,FALSE);
					vis.box.modify((Fvector&)m.pos);
					if (m.size.x>p_size) p_size = m.size.x;
					if (m.size.y>p_size) p_size = m.size.y;
					if (m.size.z>p_size) p_size = m.size.z;
				}
				vis.box.grow		(p_size);
				vis.box.getsphere	(vis.sphere.P,vis.sphere.R);
			}
			if (m_RT_Flags.is(flRT_DefferedStop)&&(0==pg->p_count)){
				m_RT_Flags.set		(flRT_Playing|flRT_DefferedStop,FALSE);
				break;
			}
		}
	} else {
		vis.box.set			(m_InitialPosition,m_InitialPosition);
		vis.box.grow		(EPS_L);
		vis.box.getsphere	(vis.sphere.P,vis.sphere.R);
	}
}

BOOL CParticleEffect::Compile(CPEDef* def)
{
	m_Def 						= def;
	if (m_Def){
		// set current effect for action
		pCurrentEffect			(m_HandleEffect);
		// refresh shader
		RefreshShader			();
		// reset particles
		ResetParticles			();
		// load action list
		// get pointer to specified action list.
		if (!_GetListPtr(m_HandleActionList)) return FALSE;

		// append actions
		pNewActionList			(m_HandleActionList);
		IReader F				(m_Def->m_Actions.pointer(),m_Def->m_Actions.size());
		u32 cnt					= F.r_u32();
		for (u32 k=0; k<cnt; k++){
			ParticleAction* act	= PAPI::pCreateAction	((PActionEnum)F.r_u32());
			act->Load			(F);
			pAddActionToList	(act);
		}
		/*//!
		for (PAVecIt it=m_Def->m_ActionList.begin(); it!=m_Def->m_ActionList.end(); it++)
		pAddActionToList	(*it);
		*/
		pEndActionList();

		// time limit
		if (m_Def->m_Flags.is(CPEDef::dfTimeLimit))
			m_fElapsedLimit 	= m_Def->m_fTimeLimit;
	}
	if (def)	hShader			= def->m_CachedShader;
	return TRUE;
}

u32 CParticleEffect::ParticlesCount()
{
	ParticleEffect *pe 		= _GetEffectPtr(m_HandleEffect);
	return pe?pe->p_count:0;
}

void CParticleEffect::ApplyExplosion()
{
	pCurrentEffect		(m_HandleEffect);

	//	action_list_handle	= pGenActionLists();
	//	pNewActionList		(action_list_handle);
	//	pExplosion			(0,0,0, 1, 8, 3, 0.1f, 1.0f);
	//	pEndActionList		();
}

//------------------------------------------------------------------------------
// Render
//------------------------------------------------------------------------------
void CParticleEffect::Copy(IRender_Visual* pFrom)
{
	Debug.fatal("Can't duplicate particle system - NOT IMPLEMENTED");
}

void CParticleEffect::OnDeviceCreate()
{
	if (m_Def){
		if (m_Def->m_Flags.is(CPEDef::dfSprite)){
			hGeom.create	(FVF::F_LIT, RCache.Vertex.Buffer(), RCache.QuadIB);
			if (m_Def) hShader = m_Def->m_CachedShader;
		}
	}
}

void CParticleEffect::OnDeviceDestroy()
{
	if (m_Def){
		if (m_Def->m_Flags.is(CPEDef::dfSprite)){
			hGeom.destroy		();
			hShader.destroy		();
		}    
	}
}
//----------------------------------------------------
IC void FillSprite	(FVF::LIT*& pv, const Fvector& pos, const Fvector2& lt, const Fvector2& rb, float r1, float r2, u32 clr, float angle)
{
	float sa	= _sin(angle);  
	float ca	= _cos(angle);  
	const Fvector& T 	= Device.vCameraTop;
	const Fvector& R 	= Device.vCameraRight;
	Fvector Vr, Vt;
	Vr.x 		= T.x*r1*sa+R.x*r1*ca;
	Vr.y 		= T.y*r1*sa+R.y*r1*ca;
	Vr.z 		= T.z*r1*sa+R.z*r1*ca;
	Vt.x 		= T.x*r2*ca-R.x*r2*sa;
	Vt.y 		= T.y*r2*ca-R.y*r2*sa;
	Vt.z 		= T.z*r2*ca-R.z*r2*sa;

	Fvector 	a,b,c,d;
	a.sub		(Vt,Vr);
	b.add		(Vt,Vr);
	c.invert	(a);
	d.invert	(b);
	pv->set		(d.x+pos.x,d.y+pos.y,d.z+pos.z, clr, lt.x,rb.y);	pv++;
	pv->set		(a.x+pos.x,a.y+pos.y,a.z+pos.z, clr, lt.x,lt.y);	pv++;
	pv->set		(c.x+pos.x,c.y+pos.y,c.z+pos.z, clr, rb.x,rb.y);	pv++;
	pv->set		(b.x+pos.x,b.y+pos.y,b.z+pos.z,	clr, rb.x,lt.y);	pv++;
}

IC void FillSprite	(FVF::LIT*& pv, const Fvector& pos, const Fvector& dir, const Fvector2& lt, const Fvector2& rb, float r1, float r2, u32 clr, float angle)
{
	float sa	= _sin(angle);  
	float ca	= _cos(angle);  
	const Fvector& T 	= dir;
	Fvector R; 	R.crossproduct(T,Device.vCameraDirection).normalize_safe();
	Fvector Vr, Vt;
	Vr.x 		= T.x*r1*sa+R.x*r1*ca;
	Vr.y 		= T.y*r1*sa+R.y*r1*ca;
	Vr.z 		= T.z*r1*sa+R.z*r1*ca;
	Vt.x 		= T.x*r2*ca-R.x*r2*sa;
	Vt.y 		= T.y*r2*ca-R.y*r2*sa;
	Vt.z 		= T.z*r2*ca-R.z*r2*sa;

	Fvector 	a,b,c,d;
	a.sub		(Vt,Vr);
	b.add		(Vt,Vr);
	c.invert	(a);
	d.invert	(b);
	pv->set		(d.x+pos.x,d.y+pos.y,d.z+pos.z, clr, lt.x,rb.y);	pv++;
	pv->set		(a.x+pos.x,a.y+pos.y,a.z+pos.z, clr, lt.x,lt.y);	pv++;
	pv->set		(c.x+pos.x,c.y+pos.y,c.z+pos.z, clr, rb.x,rb.y);	pv++;
	pv->set		(b.x+pos.x,b.y+pos.y,b.z+pos.z,	clr, rb.x,lt.y);	pv++;
}
void CParticleEffect::Render(float LOD)
{
	u32			dwOffset,dwCount;
	// Get a pointer to the particles in gp memory
	ParticleEffect *pe 		= _GetEffectPtr(m_HandleEffect);
	if((pe!=NULL)&&(pe->p_count>0)){
		if (m_Def&&m_Def->m_Flags.is(CPEDef::dfSprite)){
			FVF::LIT* pv_start	= (FVF::LIT*)RCache.Vertex.Lock(pe->p_count*4*4,hGeom->vb_stride,dwOffset);
			FVF::LIT* pv		= pv_start;

			for(int i = 0; i < pe->p_count; i++){
				PAPI::Particle &m = pe->particles[i];

				Fvector2 lt,rb;
				lt.set			(0.f,0.f);
				rb.set			(1.f,1.f);
				if (m_Def->m_Flags.is(CPEDef::dfFramed)) m_Def->m_Frame.CalculateTC(iFloor(float(m.frame)/255.f),lt,rb);
				float r_x		= m.size.x*0.5f;
				float r_y		= m.size.y*0.5f;
				if (m_Def->m_Flags.is(CPEDef::dfVelocityScale)){
					float speed	= m.vel.magnitude();
					r_x			+= speed*m_Def->m_VelocityScale.x;
					r_y			+= speed*m_Def->m_VelocityScale.y;
				}
				if (m_Def->m_Flags.is(CPEDef::dfAlignToPath)){
					Fvector 	dir;
					float speed	= m.vel.magnitude();
					if (speed>=EPS_S)	dir.div	(m.vel,speed);
					else				dir.setHP(-m_Def->m_APDefaultRotation.y,-m_Def->m_APDefaultRotation.x);
					if (m_RT_Flags.is(flRT_XFORM)){
						Fvector p,d;
						m_XFORM.transform_tiny	(p,m.pos);
						m_XFORM.transform_dir	(d,dir);
						FillSprite	(pv,p,d,lt,rb,r_x,r_y,m.color,m.rot.x);
					}else{
						FillSprite	(pv,m.pos,dir,lt,rb,r_x,r_y,m.color,m.rot.x);
					}
				}else{
					if (m_RT_Flags.is(flRT_XFORM)){
						Fvector p;
						m_XFORM.transform_tiny	(p,m.pos);
						FillSprite	(pv,p,lt,rb,r_x,r_y,m.color,m.rot.x);
					}else{
						FillSprite	(pv,m.pos,lt,rb,r_x,r_y,m.color,m.rot.x);
					}
				}
			}
			dwCount 			= u32(pv-pv_start);
			RCache.Vertex.Unlock(dwCount,hGeom->vb_stride);
			if (dwCount)    {
				RCache.set_xform_world	(Fidentity);
				RCache.set_Geometry		(hGeom);
				RCache.Render	   		(D3DPT_TRIANGLELIST,dwOffset,0,dwCount,0,dwCount/2);
			}
		}
	}
}
