#ifndef GEOMETRY_H
#define GEOMETRY_H 
#include "PhysicsCommon.h"
#include "ExtendedGeom.h"

//this is equivalent dMULTIPLYOP0_333 whith consequent transposion of A
#define dMULTIPLYOP3_333(A,op,B,C)  \
	(A)[0]  op dDOT14((B),  (C));   \
	(A)[1]  op dDOT14((B+4),(C));   \
	(A)[2]  op dDOT14((B+8),(C));   \
	(A)[4]  op dDOT14((B),  (C+1)); \
	(A)[5]  op dDOT14((B+4),(C+1)); \
	(A)[6]  op dDOT14((B+8),(C+1)); \
	(A)[8]  op dDOT14((B),  (C+2)); \
	(A)[9]  op dDOT14((B+4),(C+2)); \
	(A)[10] op dDOT14((B+8),(C+2));

inline void dMULTIPLY3_333(dReal *A, const dReal *B, const dReal *C)
{ dMULTIPLYOP3_333(A,=,B,C) }


class CGameObject;
class CPHObject;
class CODEGeom
{
protected:
	dGeomID m_geom_transform;
	u16		m_bone_id;
protected:


public:
	//get
	virtual		float		volume				()																	=0;
	virtual		void		get_mass			(dMass& m)															=0;		//unit dencity mass;
				void		get_mass			(dMass& m,const Fvector& ref_point, float density)					 ;
				void		get_mass			(dMass& m,const Fvector& ref_point)									 ;
				void		add_self_mass		(dMass& m,const Fvector& ref_point)									 ;
				void		add_self_mass		(dMass& m,const Fvector& ref_point, float density)					 ;
				void		get_local_center_bt	(Fvector& center)													 ;		//for built
				void		get_global_center_bt(Fvector& center)													 ;		//for built
				void		get_local_form_bt	(Fmatrix& form)														 ;	    //for built
				void		get_global_form_bt	(Fmatrix& form)														 ;		//for built
				void		set_static_ref_form	(const Fmatrix& form)												 ;		//for built
	virtual		void		get_max_area_dir_bt	(Fvector& dir)														=0;
	virtual		float		radius				()																	=0;
	virtual		void		get_extensions_bt	(const Fvector& axis,float center_prg,float& lo_ext, float& hi_ext) =0;

	IC			dGeomID		geom()
	{
								return dGeomTransformGetGeom(m_geom_transform);
	}
	IC			dGeomID		geometry_transform   ()
							{
								return m_geom_transform;
							}
	IC			dGeomID		geometry()
							{
								return m_geom_transform ? (geom() ? geom() : m_geom_transform) : NULL;
							}
	IC			bool		is_transformed_bt()
							{
								return dGeomGetClass(m_geom_transform)==dGeomTransformClass;
							}
	IC			u16&		element_position()
							{
								return dGeomGetUserData(geometry())->element_position;
							}
virtual const	Fvector&	local_center		()																	=0;
virtual			void		get_local_form		(Fmatrix& form)														=0;
virtual			void		set_local_form		(const Fmatrix& form)												=0;
	//set
	//element part
				void		set_body			(dBodyID body)														;
				void		set_bone_id			(u16 id)											{m_bone_id=id;}
				u16			bone_id				()												{return m_bone_id;}
				void		add_to_space		(dSpaceID space)													;
				void		remove_from_space	(dSpaceID space)													;
				void		set_material		(u16 ul_material)													;
				void		set_contact_cb		(ContactCallbackFun* ccb)											;
				void		set_obj_contact_cb	(ObjectContactCallbackFun* occb)									;
				void		set_ref_object		(CPhysicsShellHolder* ro)												;
				void		set_ph_object		(CPHObject* o)														;
	//build/destroy
protected:
	void		init				()																	;
	virtual		dGeomID		create				()																	=0;
public:
				void		build				(const Fvector& ref_point)											;
	virtual		void		set_position		(const Fvector& ref_point)											;//for build geom
				void		move_local_basis	(const Fmatrix& inv_new_mul_old)									;
				void		destroy				()																	;
							CODEGeom			()																	;
	virtual					~ CODEGeom			()																	;
};

class CBoxGeom : public CODEGeom
{
	typedef CODEGeom inherited																						;
	Fobb	m_box;
public:
							CBoxGeom			(const Fobb& box)													;
//	virtual					~CBoxGeom			(const Fobb& box)													;
	virtual		float		volume				()																	;
	virtual		float		radius				()																	;
	virtual		void		get_extensions_bt	(const Fvector& axis,float center_prg,float& lo_ext, float& hi_ext) ;
	virtual		void		get_max_area_dir_bt	(Fvector& dir)														;
	virtual		void		get_mass			(dMass& m)															;//unit dencity mass;
virtual const	Fvector&	local_center		()																	;
	virtual		void		get_local_form		(Fmatrix& form)														;
virtual			void		set_local_form		(const Fmatrix& form)												;
	virtual		dGeomID		create				()																	;
	virtual		void		set_position		(const Fvector& ref_point)											;
};

class CSphereGeom : public CODEGeom
{
	typedef CODEGeom inherited																						;
	Fsphere	m_sphere;
public:
							CSphereGeom			(const Fsphere& sphere)												;
	virtual		float		volume				()																	;
	virtual		float		radius				()																	;
	virtual		void		get_extensions_bt	(const Fvector& axis,float center_prg,float& lo_ext, float& hi_ext) ;
	virtual		void		get_max_area_dir_bt	(Fvector& dir)													  {};
	virtual		void		get_mass			(dMass& m)															;//unit dencity mass;
virtual const	Fvector&	local_center		()																	;
	virtual		void		get_local_form		(Fmatrix& form)														;
	virtual		void		set_local_form		(const Fmatrix& form)												;
	virtual		dGeomID		create				()																	;
	virtual		void		set_position		(const Fvector& ref_point)											;
};
class CCylinderGeom : public CODEGeom
{
	typedef CODEGeom inherited																						;
	Fcylinder m_cylinder;
public:
							CCylinderGeom		(const Fcylinder& cyl)												;
	virtual		float		volume				()																	;
	virtual		float		radius				()																	;
	virtual		void		get_extensions_bt	(const Fvector& axis,float center_prg,float& lo_ext, float& hi_ext) ;
	virtual		void		get_max_area_dir_bt	(Fvector& dir)													  {};
virtual const	Fvector&	local_center		()																	;
	virtual		void		get_mass			(dMass& m)															;//unit dencity mass;
	virtual		void		get_local_form		(Fmatrix& form)														;
	virtual		void		set_local_form		(const Fmatrix& form)												;
	virtual		dGeomID		create				()																	;
	virtual		void		set_position		(const Fvector& ref_point)											;
};
#endif //GEOMETRY_H