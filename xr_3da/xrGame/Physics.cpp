
#include "StdAfx.h"
#include "PHDynamicData.h"
#include "Physics.h"
#include "tri-colliderknoopc/dTriList.h"
#include "dRay/include/dRay.h"

/////////////////////////////////////////
static dContact bulletContact;
static bool isShooting;
static dVector3 RayD;
static dVector3 RayO;
/////////////////////////////////////
static void FUNCCALL NearCallback(void* /*data*/, dGeomID o1, dGeomID o2);



void CPHWorld::Render()
{
//	Device.Shader.OnFrameEnd		();
//	Device.Primitive.dbg_DrawAABB	(vCenter,sx,sy,sz,0xffffffff);
//	Device.Primitive.dbg_DrawEllipse(M, 0xfffffff);
}

//////////////////////////////////////////////////////////////
//////////////CPHMesh///////////////////////////////////////////
///////////////////////////////////////////////////////////

void CPHMesh ::Create(dSpaceID space, dWorldID world){
Geom = dCreateTriList(space, 0, 0);

}

////////////////////////////////////////////////////////////////////////////

void CPHMesh ::Destroy(){
						dGeomDestroy(Geom);
						}


/////////////////////////////////////////////////////////////////////////////
/////////////CPHJeep////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void CPHJeep::Create(dSpaceID space, dWorldID world){
	
	static const dReal scaleParam=1.6/0.8;
	static const dVector3 scaleBox={REAL(2.4)*scaleParam, REAL(0.4)*scaleParam, REAL(1.5)*scaleParam};
	static const dVector3 jeepBox={scaleBox[0],scaleBox[0],scaleBox[0]};
	static const dVector3 cabinBox={scaleBox[0]/1.7,scaleBox[1]*2.,scaleBox[2]/1.01};

	static const dReal wheelRadius = REAL(0.28)* scaleParam, wheelWidth = REAL(0.25)* scaleParam;
	static const wheelSegments = 24;

	static const dVector3 startPosition={0.f,15.f,0.f};
	static const dReal weelSepX=jeepBox[0]/2.f-jeepBox[0]/8.f,weelSepZ=jeepBox[2]/2.f-wheelRadius/2.f,weelSepY=jeepBox[2];

	dMass m;

	// car body
	//dMass m;
	dMassSetBox(&m, 1, jeepBox[0], jeepBox[1], jeepBox[2]); // density,lx,ly,lz
	dMassAdjust(&m, 50); // mass

	Bodies[0] = dBodyCreate(world);
	dBodySetMass(Bodies[0], &m);
	dBodySetPosition(Bodies[0], startPosition[0], startPosition[1], startPosition[2]); // x,y,z

	Geoms[0] = dCreateBox(0, jeepBox[0], jeepBox[1], jeepBox[2]); // lx,ly,lz
	Geoms[6] = dCreateBox(0, cabinBox[0], cabinBox[1], cabinBox[2]); // lx,ly,lz

	//dGeomSetBody(Geoms[5], Bodies[5]);
	Geoms[5]=dCreateGeomTransform(space);
	//Geoms[7]=dCreateGeomTransform(0);
	dGeomSetPosition(Geoms[6], -jeepBox[0]/2+cabinBox[0]/2+0.1, cabinBox[1]/2+jeepBox[1]/2, 0); // x,y,z
	//dGeomSetPosition(Geoms[0], 0,0/*-jeepBox[1]-wheelRadius*/, 0); // x,y,z
	dGeomTransformSetGeom(Geoms[5],Geoms[6]);
	//dGeomTransformSetGeom(Geoms[7],Geoms[0]);


	dGeomSetBody(Geoms[5], Bodies[0]);
	dGeomSetBody(Geoms[0], Bodies[0]);



	

	// wheel bodies
	dMassSetSphere(&m, 1, wheelRadius); // density, radius
	dMassAdjust(&m, 10); // mass
	dQuaternion q;
	dQFromAxisAndAngle(q, 1, 0, 0, M_PI * 0.5);
	UINT i;
	for(i = 1; i <= 4; ++i)
	{
		Bodies[i] = dBodyCreate(world);
		dBodySetMass(Bodies[i], &m);
		dBodySetQuaternion(Bodies[i], q);
		Geoms[i] = dCreateSphere(0, wheelRadius);
		dGeomSetBody(Geoms[i], Bodies[i]);
	}




	dBodySetPosition(Bodies[1], startPosition[0]-weelSepX, startPosition[1]-weelSepY,  startPosition[2]+weelSepZ); // x,y,z
	dBodySetPosition(Bodies[2], startPosition[0]-weelSepX, startPosition[1]-weelSepY,  startPosition[2]-weelSepZ); // x,y,z-0.9, 2.6,   9.3); // x,y,z
	dBodySetPosition(Bodies[3], startPosition[0]+weelSepX, startPosition[1]-weelSepY,  startPosition[2]+weelSepZ); // x,y,z 0.9, 2.6,  10.7); // x,y,z
	dBodySetPosition(Bodies[4], startPosition[0]+weelSepX, startPosition[1]-weelSepY,  startPosition[2]-weelSepZ); // x,y,z 0.9, 2.6,   9.3); // x,y,z



	// wheel joints
	for(i = 0; i < 4; ++i)
	{
		Joints[i] = dJointCreateHinge2(world, 0);
		dJointAttach(Joints[i], Bodies[0], Bodies[i+1]);
		const dReal* const wPos = dBodyGetPosition(Bodies[i+1]);
		dJointSetHinge2Anchor(Joints[i], wPos[0], wPos[1], wPos[2]);
		dJointSetHinge2Axis1(Joints[i], 0, 1, 0);
		dJointSetHinge2Axis2(Joints[i], 0, 0, ((i % 2) == 0) ? -1 : 1);

		dJointSetHinge2Param(Joints[i], dParamLoStop, 0);
		dJointSetHinge2Param(Joints[i], dParamHiStop, 0);
		dJointSetHinge2Param(Joints[i], dParamFMax, 50);

		dJointSetHinge2Param(Joints[i], dParamVel2, 0);
		dJointSetHinge2Param(Joints[i], dParamFMax2, 80);

		dJointSetHinge2Param(Joints[i], dParamSuspensionERP, 0.4f);
		dJointSetHinge2Param(Joints[i], dParamSuspensionCFM, 0.005f);
	}

	GeomsGroup = dCreateGeomGroup(space);  
	for(i = 0; i < NofGeoms-1; ++i)
		dGeomGroupAdd(GeomsGroup, Geoms[i]);



	//dynamic data
CreateDynamicData();

}

/////////////////////////////////////////////////////////////////////////////


void CPHJeep::Destroy(){
	for(UINT i=0;i<NofJoints;i++) dJointDestroy(Joints[i]);
	for(UINT i=0;i<NofBodies;i++) dBodyDestroy(Bodies[i]);
	for(UINT i=0;i<NofGeoms;i++) dGeomDestroy(Geoms[i]);
	//dGeomDestroy(GeomsGroup);
	DynamicData.Destroy();

}

void CPHJeep::Steer(const char& velocity, const char& steering)
{
	static const dReal steeringRate = M_PI * 4 / 3;
	static const dReal steeringLimit = M_PI / 6;
	static const dReal wheelVelocity = 16*12 * M_PI;
	ULONG i;
	switch(steering)
	{
	case 1:
	case -1:
		for(i = 2; i < 4; ++i)
		{
			dJointSetHinge2Param(Joints[i], dParamHiStop, steeringLimit);
			dJointSetHinge2Param(Joints[i], dParamLoStop, -steeringLimit);
			dJointSetHinge2Param(Joints[i], dParamVel, ((i < 2) ? steering : -steering) * steeringRate);
		}
		break;

	default: // case 0
		for(i = 0; i < 4; ++i)
		{
			dReal angle = dJointGetHinge2Angle1(Joints[i]);
			if(angle < 0)
			{
				dJointSetHinge2Param(Joints[i], dParamHiStop, 0);
				dJointSetHinge2Param(Joints[i], dParamVel, steeringRate);
			}
			else
			{
				dJointSetHinge2Param(Joints[i], dParamLoStop, 0);
				dJointSetHinge2Param(Joints[i], dParamVel, -steeringRate);
			}
		}
		break;
	}

	switch(velocity)
	{
	case 1:
        for(i = 0; i < 4; ++i)
			dJointSetHinge2Param(Joints[i], dParamVel2, ((i % 2) == 0) ? -wheelVelocity : wheelVelocity);
		break;

	case -1:
        for(i = 0; i < 4; ++i)
			dJointSetHinge2Param(Joints[i], dParamVel2, ((i % 2) == 0) ? wheelVelocity : -wheelVelocity);
		break;

	default: // case 0
        for(i = 0; i < 4; ++i)
			dJointSetHinge2Param(Joints[i], dParamVel2, 0);
		break;
	}
}
////////////////////////////////////////////////////////////////

void CPHJeep::CreateDynamicData(){
DynamicData.Create(4,Bodies[0]);
DynamicData.SetChild(0,0,Bodies[1]);
DynamicData.SetChild(1,0,Bodies[2]);
DynamicData.SetChild(2,0,Bodies[3]);
DynamicData.SetChild(3,0,Bodies[4]);

}
////////////////////////////////////////////////////////////////////////////
///////////CPHWorld/////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void CPHWorld::Create(){

	phWorld = dWorldCreate();
	Space = dHashSpaceCreate();
	ContactGroup = dJointGroupCreate(0);		
	dWorldSetGravity(phWorld, 0, -9.81f, 0);
	Mesh.Create(Space,phWorld);
	Jeep.Create(Space,phWorld);
	//Gun.Create(Space);
	dWorldSetCFM(phWorld, 0.001f);
	dWorldSetERP(phWorld, 0.99f);
}

/////////////////////////////////////////////////////////////////////////////

void CPHWorld::Destroy(){
	Mesh.Destroy();
	Jeep.Destroy();
	//Gun.Destroy();
	dJointGroupEmpty(ContactGroup);
	dSpaceDestroy(Space);
	dWorldDestroy(phWorld);
	dCloseODE();

}

//////////////////////////////////////////////////////////////////////////////

void CPHWorld::Step(dReal step){
			// compute contact joints and forces
			dSpaceCollide(Space, 0, &NearCallback);
		
			
			if(isShooting&&bulletContact.geom.pos[0]!=dInfinity){
				dJointID c = dJointCreateContact(phWorld, ContactGroup, &bulletContact);
				dJointAttach(c, dGeomGetBody(bulletContact.geom.g1), dGeomGetBody(bulletContact.geom.g2));
				
				}
				

			dWorldStep(phWorld, step);
			dJointGroupEmpty(ContactGroup);
	
			isShooting=false;
			bulletContact.geom.pos[0]=dInfinity;
		}



static void FUNCCALL NearCallback(void* /*data*/, dGeomID o1, dGeomID o2){
		const ULONG N = 100;
		dContact contacts[N];
		// get the contacts up to a maximum of N contacts
		ULONG n = dCollide(o1, o2, N, &contacts[0].geom, sizeof(dContact));	
		

		if(
			dGeomGetClass(o1)==dRayClass||dGeomGetClass(o2)==dRayClass
			){
	
			if(isShooting){
				dVector3 norm={RayD[0],RayD[1],RayD[2]};
				dNormalize3(norm);
			//	ulong n = dCollide(o1, o2, N, &bulletContact.geom, sizeof(dContact));
				for(int i = 0; i < n; ++i)
				{
			
					if(dDOT(contacts[i].geom.pos,norm)<dDOT(bulletContact.geom.pos,norm))

					{
					memcpy(&bulletContact,&contacts[i],sizeof(dContact));
					bulletContact.geom.normal[0]=norm[0];
					bulletContact.geom.normal[1]=norm[1];
					bulletContact.geom.normal[2]=norm[2];
				
					bulletContact.geom.depth=0.5;
				
					bulletContact.surface.mode = dContactBounce;
					bulletContact.surface.mu = 0.f;
					bulletContact.surface.bounce = 0.3f;
					bulletContact.surface.bounce_vel = 0.05f;
					}
				}
	
			}
		return;
	}
		


	if(n>N)
		n=N;
	ULONG i;

if(dGeomGetClass(o1)==dGeomTransformClass){
	for(i = 0; i < n; ++i)
	{

        contacts[i].surface.mode = dContactBounce;
		contacts[i].surface.mu = 500;
		contacts[i].surface.bounce = 0.3f;
		contacts[i].surface.bounce_vel = 0.05f;
		dJointID c = dJointCreateContact(phWorld, ContactGroup, &contacts[i]);
		dJointAttach(c, dGeomGetBody(o1), dGeomGetBody(contacts[i].geom.g2));
		}

	}
else if(dGeomGetClass(o2)==dGeomTransformClass){
		for(i = 0; i < n; ++i)
		{

        contacts[i].surface.mode = dContactBounce;
		contacts[i].surface.mu = 500.f;
		contacts[i].surface.bounce = 0.3f;
		contacts[i].surface.bounce_vel = 0.05f;
		dJointID c = dJointCreateContact(phWorld, ContactGroup, &contacts[i]);
		dJointAttach(c, dGeomGetBody(contacts[i].geom.g1), dGeomGetBody(o2));
		}
	}

else
	for(i = 0; i < n; ++i)
	{

        contacts[i].surface.mode = dContactBounce;
		contacts[i].surface.mu = 250.f;
		contacts[i].surface.bounce = 0.3f;
		contacts[i].surface.bounce_vel =0.05f;
		dJointID c = dJointCreateContact(phWorld, ContactGroup, &contacts[i]);
		dJointAttach(c, dGeomGetBody(contacts[i].geom.g1), dGeomGetBody(contacts[i].geom.g2));
		}
}
//////////////////////////////////////////////////////////////////////////////////
//////////////////CPHGun//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
void CPHGun::Create(dSpaceID space){
	
	dVector3 rayO={0.f,0.f,0.f};
	dVector3 rayD={0.f,1.f,0.f};
	Ray=dGeomCreateRay(space,1.f);
	dGeomRaySet(Ray,rayO,rayD);
	isShooting=false;
}

///////////////////////////////////////////////////////////////////////////////////////////
void CPHGun::Destroy(){
dGeomDestroy(Ray);
}



///////////////////////////////////////////////////////////////////////////////////////////


void CPHGun::Shoot(const dReal* rayO,const dReal*  rayD){
bulletContact.geom.pos[0]=dInfinity;
RayO[0]=rayO[0];
RayO[1]=rayO[1];
RayO[2]=rayO[2];
RayD[0]=rayD[0];
RayD[1]=rayD[1];
RayD[2]=rayD[2];
dGeomRaySet(Ray,RayO,RayD);

isShooting=true;
}
///////////////////////////////