#include "stdafx.h"

#include "..\fhierrarhyvisual.h"
#include "..\SkeletonCustom.h"
#include "..\fmesh.h"
#include "..\fcached.h"
#include "..\flod.h"
#include "..\irenderable.h"

#include "particlegroup.h"

using	namespace R_dsgraph;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Scene graph actual insertion and sorting ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
float		r_ssaDISCARD;
float		r_ssaDONTSORT;
float		r_ssaLOD_A;
float		r_ssaLOD_B;
float		r_ssaHZBvsTEX;

IC	float	CalcSSA				(float& distSQ, Fvector& C, IRender_Visual* V)
{
	float R	= V->vis.sphere.R;
	distSQ	= Device.vCameraPosition.distance_to_sqr(C);
	return	R*R/distSQ;
}

void R_dsgraph_structure::r_dsgraph_insert_dynamic	(IRender_Visual *pVisual, Fvector& Center)
{
	CRender&	RI			=	RImplementation;

	//if (pVisual->vis.frame	==	RI.marker)	return;
	//pVisual->vis.frame		=	RI.marker;

	float distSQ;
	float SSA				=	CalcSSA		(distSQ,Center,pVisual);
	if (SSA<=r_ssaDISCARD)		return;

	// Distortive geometry should be marked and R2 special-cases it
	// a) Allow to optimize RT order
	// b) Should be rendered to special distort buffer in another pass
	ShaderElement*		sh_d	= &*pVisual->hShader->E[4];
	if (RImplementation.o.distortion && sh_d && sh_d->flags.bDistort && pmask[sh_d->flags.iPriority/2]) {
		mapSorted_Node* N		= mapDistort.insertInAnyWay	(distSQ);
		N->val.ssa				= SSA;
		N->val.pObject			= RI.val_pObject;
		N->val.pVisual			= pVisual;
		N->val.Matrix			= *RI.val_pTransform;
		N->val.se				= &*pVisual->hShader->E[4];		// 4=L_special
	}

	// Select shader
	ShaderElement*	sh		=	RImplementation.rimp_select_sh_dynamic	(pVisual,distSQ);
	if (0==sh)								return;
	if (!pmask[sh->flags.iPriority/2])		return;

	// Create common node
	// NOTE: Invisible elements exist only in R1
	_MatrixItem		item	= {SSA,RI.val_pObject,pVisual,*RI.val_pTransform};

	// HUD rendering
	if (RI.val_bHUD)			{
		mapHUD_Node* N			= mapHUD.insertInAnyWay		(distSQ);
		N->val.ssa				= SSA;
		N->val.pObject			= RI.val_pObject;
		N->val.pVisual			= pVisual;
		N->val.Matrix			= *RI.val_pTransform;
		N->val.se				= sh;
		return;
	}

	// Shadows registering
#if RENDER==R_R1
	RI.L_Shadows->add_element	(item);
#endif
	if (RI.val_bInvisible)		return;

	// strict-sorting selection
	if (sh->flags.bStrictB2F) {
		mapSorted_Node* N		= mapSorted.insertInAnyWay	(distSQ);
		N->val.ssa				= SSA;
		N->val.pObject			= RI.val_pObject;
		N->val.pVisual			= pVisual;
		N->val.Matrix			= *RI.val_pTransform;
		N->val.se				= sh;
		return;
	}

#if RENDER==R_R2
	// Emissive geometry should be marked and R2 special-cases it
	// a) Allow to skeep already lit pixels
	// b) Allow to make them 100% lit and really bright
	// c) Should not cast shadows
	// d) Should be rendered to accumulation buffer in the second pass
	if (sh->flags.bEmissive) {
		mapSorted_Node* N		= mapEmissive.insertInAnyWay	(distSQ);
		N->val.ssa				= SSA;
		N->val.pObject			= RI.val_pObject;
		N->val.pVisual			= pVisual;
		N->val.Matrix			= *RI.val_pTransform;
		N->val.se				= &*pVisual->hShader->E[4];		// 4=L_special
	}
#endif

	// the most common node
	SPass&						pass	= *sh->passes.front	();
	mapMatrix_T&				map		= mapMatrix			[sh->flags.iPriority/2];
	mapMatrixVS::TNode*			Nvs		= map.insert		(pass.vs->vs);
	mapMatrixPS::TNode*			Nps		= Nvs->val.insert	(pass.ps->ps);
	mapMatrixCS::TNode*			Ncs		= Nps->val.insert	(pass.constants._get());
	mapMatrixStates::TNode*		Nstate	= Ncs->val.insert	(pass.state->state);
	mapMatrixTextures::TNode*	Ntex	= Nstate->val.insert(pass.T._get());
	mapMatrixVB::TNode*			Nvb		= Ntex->val.insert	(pVisual->hGeom->vb);
	mapMatrixItems&				items	= Nvb->val;
	items.push_back						(item);

	// Need to sort for HZB efficient use
	if (SSA>Nvb->val.ssa)		{ Nvb->val.ssa = SSA;
	if (SSA>Ntex->val.ssa)		{ Ntex->val.ssa = SSA;
	if (SSA>Nstate->val.ssa)	{ Nstate->val.ssa = SSA;
	if (SSA>Ncs->val.ssa)		{ Ncs->val.ssa = SSA;
	if (SSA>Nps->val.ssa)		{ Nps->val.ssa = SSA;
	if (SSA>Nvs->val.ssa)		{ Nvs->val.ssa = SSA;
	} } } } } }

#if RENDER==R_R2
	if (val_recorder)			{
		Fbox3		temp		;
		Fmatrix&	xf			= *RI.val_pTransform;
		temp.xform	(pVisual->vis.box,xf);
		val_recorder->push_back	(temp);
	}
#endif
}

void R_dsgraph_structure::r_dsgraph_insert_static	(IRender_Visual *pVisual)
{
	if (pVisual->vis.frame		==	RImplementation.marker)	return;
	pVisual->vis.frame			=	RImplementation.marker;

	float distSQ;
	float SSA					= CalcSSA	(distSQ,pVisual->vis.sphere.P,pVisual);
	if (SSA<=r_ssaDISCARD)		return;

	// Distortive geometry should be marked and R2 special-cases it
	// a) Allow to optimize RT order
	// b) Should be rendered to special distort buffer in another pass
	ShaderElement*		sh_d	= &*pVisual->hShader->E[4];
	if (RImplementation.o.distortion && sh_d && sh_d->flags.bDistort && pmask[sh_d->flags.iPriority/2]) {
		mapSorted_Node* N		= mapDistort.insertInAnyWay		(distSQ);
		N->val.ssa				= SSA;
		N->val.pObject			= NULL;
		N->val.pVisual			= pVisual;
		N->val.Matrix			= Fidentity;
		N->val.se				= &*pVisual->hShader->E[4];		// 4=L_special
	}

	// Select shader
	ShaderElement*		sh		= RImplementation.rimp_select_sh_static(pVisual,distSQ);
	if (0==sh)								return;
	if (!pmask[sh->flags.iPriority/2])		return;

	// strict-sorting selection
	if (sh->flags.bStrictB2F) {
		mapSorted_Node* N			= mapSorted.insertInAnyWay(distSQ);
		N->val.pObject				= NULL;
		N->val.pVisual				= pVisual;
		N->val.Matrix				= Fidentity;
		N->val.se					= sh;
		return;
	}

#if RENDER==R_R2
	// Emissive geometry should be marked and R2 special-cases it
	// a) Allow to skeep already lit pixels
	// b) Allow to make them 100% lit and really bright
	// c) Should not cast shadows
	// d) Should be rendered to accumulation buffer in the second pass
	if (sh->flags.bEmissive) {
		mapSorted_Node* N		= mapEmissive.insertInAnyWay	(distSQ);
		N->val.ssa				= SSA;
		N->val.pObject			= NULL;
		N->val.pVisual			= pVisual;
		N->val.Matrix			= Fidentity;
		N->val.se				= &*pVisual->hShader->E[4];		// 4=L_special
	}
#endif

	if	(val_feedback && counter_S==val_feedback_breakp)	val_feedback->rfeedback_static(pVisual);

	counter_S					++;
	SPass&						pass	= *sh->passes.front	();
	mapNormal_T&				map		= mapNormal			[sh->flags.iPriority/2];
	mapNormalVS::TNode*			Nvs		= map.insert		(pass.vs->vs);
	mapNormalPS::TNode*			Nps		= Nvs->val.insert	(pass.ps->ps);
	mapNormalCS::TNode*			Ncs		= Nps->val.insert	(pass.constants._get());
	mapNormalStates::TNode*		Nstate	= Ncs->val.insert	(pass.state->state);
	mapNormalTextures::TNode*	Ntex	= Nstate->val.insert(pass.T._get());
	mapNormalVB::TNode*			Nvb		= Ntex->val.insert	(pVisual->hGeom->vb);
	mapNormalItems&				items	= Nvb->val;
	_NormalItem					item	= {SSA,pVisual};
	items.push_back						(item);

	// Need to sort for HZB efficient use
	if (SSA>Nvb->val.ssa)		{ Nvb->val.ssa = SSA;
	if (SSA>Ntex->val.ssa)		{ Ntex->val.ssa = SSA;
	if (SSA>Nstate->val.ssa)	{ Nstate->val.ssa = SSA;
	if (SSA>Ncs->val.ssa)		{ Ncs->val.ssa = SSA;
	if (SSA>Nps->val.ssa)		{ Nps->val.ssa = SSA;
	if (SSA>Nvs->val.ssa)		{ Nvs->val.ssa = SSA;
	} } } } } }

#if RENDER==R_R2
	if (val_recorder)			{
		val_recorder->push_back	(pVisual->vis.box	);
	}
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
void CRender::add_leafs_Dynamic(IRender_Visual *pVisual)
{
	// Visual is 100% visible - simply add it
	xr_vector<IRender_Visual*>::iterator I,E;	// it may be useful for 'hierrarhy' visual

	switch (pVisual->Type) {
	case MT_PARTICLE_GROUP:
		{
			// Add all children, doesn't perform any tests
			PS::CParticleGroup* pG = (PS::CParticleGroup*)pVisual;
			for (PS::CParticleGroup::SItemVecIt i_it=pG->items.begin(); i_it!=pG->items.end(); i_it++){
				#pragma todo("serious performance problem here")
				xr_vector<IRender_Visual*>	visuals;
				i_it->GetVisuals			(visuals);
				for (xr_vector<IRender_Visual*>::iterator it=visuals.begin(); it!=visuals.end(); it++)
					add_leafs_Dynamic		(*it);
			}
		}
		return;
	case MT_HIERRARHY:
		{
			// Add all children, doesn't perform any tests
			FHierrarhyVisual* pV = (FHierrarhyVisual*)pVisual;
			I = pV->children.begin	();
			E = pV->children.end	();
			for (; I!=E; I++)	add_leafs_Dynamic	(*I);
		}
		return;
	case MT_SKELETON_ANIM:
	case MT_SKELETON_RIGID:
		{
			// Add all children, doesn't perform any tests
			CKinematics * pV			= (CKinematics*)pVisual;
			pV->CalculateBones			(TRUE);
			I = pV->children.begin		();
			E = pV->children.end		();
			for (; I!=E; I++)	add_leafs_Dynamic	(*I);
		}
		return;
	default:
		{
			// General type of visual
			// Calculate distance to it's center
			Fvector							Tpos;
			val_pTransform->transform_tiny	(Tpos, pVisual->vis.sphere.P);
			r_dsgraph_insert_dynamic		(pVisual,Tpos);
		}
		return;
	}
}

void CRender::add_leafs_Static(IRender_Visual *pVisual)
{
	if (!HOM.visible(pVisual->vis))		return;

	// Visual is 100% visible - simply add it
	xr_vector<IRender_Visual*>::iterator I,E;	// it may be usefull for 'hierrarhy' visuals

	switch (pVisual->Type) {
	case MT_PARTICLE_GROUP:
		{
			// Add all children, doesn't perform any tests
			PS::CParticleGroup* pG = (PS::CParticleGroup*)pVisual;
			for (PS::CParticleGroup::SItemVecIt i_it=pG->items.begin(); i_it!=pG->items.end(); i_it++){
				#pragma todo("serious performance problem here")
				xr_vector<IRender_Visual*>	visuals;
				i_it->GetVisuals			(visuals);
				for (xr_vector<IRender_Visual*>::iterator it=visuals.begin(); it!=visuals.end(); it++)
					add_leafs_Dynamic		(*it);
			}
		}
		return;
	case MT_HIERRARHY:
		{
			// Add all children, doesn't perform any tests
			FHierrarhyVisual* pV = (FHierrarhyVisual*)pVisual;
			I = pV->children.begin	();
			E = pV->children.end	();
			for (; I!=E; I++)	add_leafs_Static (*I);
		}
		return;
	case MT_SKELETON_ANIM:
	case MT_SKELETON_RIGID:
		{
			// Add all children, doesn't perform any tests
			CKinematics * pV		= (CKinematics*)pVisual;
			pV->CalculateBones		(TRUE);
			I = pV->children.begin	();
			E = pV->children.end	();
			for (; I!=E; I++)	add_leafs_Static	(*I);
		}
		return;
	case MT_LOD:
		{
			FLOD		* pV	= (FLOD*) pVisual;
			float		D;
			float		ssa		= CalcSSA	(D,pV->vis.sphere.P,pV);
			if (ssa<r_ssaLOD_A)
			{
				mapLOD_Node*	N	= mapLOD.insertInAnyWay(D);
				N->val.ssa						= ssa;
				N->val.pVisual					= pVisual;
			}
			if (ssa>r_ssaLOD_B)
			{
				// Add all children, doesn't perform any tests
				I = pV->children.begin	();
				E = pV->children.end	();
				for (; I!=E; I++)	add_leafs_Static (*I);
			}
		}
		break;
	default:
		{
			// General type of visual
			r_dsgraph_insert_static(pVisual);
		}
		return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CRender::add_Dynamic(IRender_Visual *pVisual, u32 planes)
{
	// Check frustum visibility and calculate distance to visual's center
	Fvector		Tpos;	// transformed position
	EFC_Visible	VIS;

	val_pTransform->transform_tiny	(Tpos, pVisual->vis.sphere.P);
	VIS = View->testSphere			(Tpos, pVisual->vis.sphere.R,planes);
	if (fcvNone==VIS) return FALSE;

	// If we get here visual is visible or partially visible
	xr_vector<IRender_Visual*>::iterator I,E;	// it may be usefull for 'hierrarhy' visuals

	switch (pVisual->Type) {
	case MT_PARTICLE_GROUP:
		{
			// Add all children, doesn't perform any tests
			PS::CParticleGroup* pG = (PS::CParticleGroup*)pVisual;
			for (PS::CParticleGroup::SItemVecIt i_it=pG->items.begin(); i_it!=pG->items.end(); i_it++){
				#pragma todo("serious performance problem here")
				xr_vector<IRender_Visual*>	visuals;
				i_it->GetVisuals			(visuals);
				for (xr_vector<IRender_Visual*>::iterator it=visuals.begin(); it!=visuals.end(); it++){
					if (fcvPartial==VIS) {
						for (; I!=E; I++)	add_Dynamic			(*it,planes);
					} else {
						for (; I!=E; I++)	add_leafs_Dynamic	(*it);
					}
				}
			}
		}
		break;
	case MT_HIERRARHY:
		{
			// Add all children
			FHierrarhyVisual* pV = (FHierrarhyVisual*)pVisual;
			I = pV->children.begin	();
			E = pV->children.end	();
			if (fcvPartial==VIS) {
				for (; I!=E; I++)	add_Dynamic			(*I,planes);
			} else {
				for (; I!=E; I++)	add_leafs_Dynamic	(*I);
			}
		}
		break;
	case MT_SKELETON_ANIM:
	case MT_SKELETON_RIGID:
		{
			// Add all children, doesn't perform any tests
			CKinematics * pV			= (CKinematics*)pVisual;
			pV->CalculateBones			(TRUE);
			I = pV->children.begin		();
			E = pV->children.end		();
			if (fcvPartial==VIS) {
				for (; I!=E; I++)	add_Dynamic			(*I,planes);
			} else {
				for (; I!=E; I++)	add_leafs_Dynamic	(*I);
			}
		}
		break;
	default:
		{
			// General type of visual
			r_dsgraph_insert_dynamic(pVisual,Tpos);
		}
		break;
	}
	return TRUE;
}

void CRender::add_Static(IRender_Visual *pVisual, u32 planes)
{
	// Check frustum visibility and calculate distance to visual's center
	EFC_Visible	VIS;
	vis_data&	vis			= pVisual->vis;
	VIS = View->testSAABB	(vis.sphere.P,vis.sphere.R,vis.box.data(),planes);
	if (fcvNone==VIS)		return;
	if (!HOM.visible(vis))	return;

	// If we get here visual is visible or partially visible
	xr_vector<IRender_Visual*>::iterator I,E;	// it may be usefull for 'hierrarhy' visuals

	switch (pVisual->Type) {
	case MT_PARTICLE_GROUP:
		{
			// Add all children, doesn't perform any tests
			PS::CParticleGroup* pG = (PS::CParticleGroup*)pVisual;
			for (PS::CParticleGroup::SItemVecIt i_it=pG->items.begin(); i_it!=pG->items.end(); i_it++){
				#pragma todo("serious performance problem here")
				xr_vector<IRender_Visual*>	visuals;
				i_it->GetVisuals			(visuals);
				for (xr_vector<IRender_Visual*>::iterator it=visuals.begin(); it!=visuals.end(); it++){
					if (fcvPartial==VIS) {
						for (; I!=E; I++)	add_Static			(*I,planes);
					} else {
						for (; I!=E; I++)	add_leafs_Static	(*I);
					}
				}
			}
		}
		break;
	case MT_HIERRARHY:
		{
			// Add all children
			FHierrarhyVisual* pV = (FHierrarhyVisual*)pVisual;
			I = pV->children.begin	();
			E = pV->children.end		();
			if (fcvPartial==VIS) {
				for (; I!=E; I++)	add_Static			(*I,planes);
			} else {
				for (; I!=E; I++)	add_leafs_Static	(*I);
			}
		}
		break;
	case MT_SKELETON_ANIM:
	case MT_SKELETON_RIGID:
		{
			// Add all children, doesn't perform any tests
			CKinematics * pV		= (CKinematics*)pVisual;
			pV->CalculateBones		(TRUE);
			I = pV->children.begin	();
			E = pV->children.end	();
			if (fcvPartial==VIS) {
				for (; I!=E; I++)	add_Static			(*I,planes);
			} else {
				for (; I!=E; I++)	add_leafs_Static	(*I);
			}
		}
		break;
	case MT_LOD:
		{
			FLOD		* pV	= (FLOD*) pVisual;
			float		D;
			float		ssa		= CalcSSA	(D,pV->vis.sphere.P,pV);
			if (ssa<r_ssaLOD_A)	
			{
				mapLOD_Node*	N	= mapLOD.insertInAnyWay(D);
				N->val.ssa						= ssa;
				N->val.pVisual					= pVisual;
			}
			if (ssa>r_ssaLOD_B)
			{
				// Add all children, perform tests
				I = pV->children.begin	();
				E = pV->children.end	();
				for (; I!=E; I++)	add_leafs_Static	(*I);
			}
		}
		break;
	default:
		{
			// General type of visual
			r_dsgraph_insert_static(pVisual);
		}
		break;
	}
}
