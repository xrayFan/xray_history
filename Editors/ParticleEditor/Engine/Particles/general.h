// general.h
//
// Copyright 1998 by David K. McAllister.
//
// This file implements the API calls that are not particle actions.

#ifndef general_h
#define general_h

namespace PAPI{
#pragma pack (push,4)
	// A single particle

	struct Particle
	{
		enum{
			BIRTH		= (1<<0),
			DYING		= (1<<1),
			ANIMATE_CCW	= (1<<1),
		};
		pVector	pos;
		pVector	posB;
		pVector	size;
		pVector	rot;
		pVector	vel;
		pVector	velB;	// Used to compute binormal, normal, etc.
		pVector	color;	// Color must be next to alpha so glColor4fv works.
		float	alpha;	// This is both cunning and scary.
		float	age;
		float	frame;
		Flags32	flags;
	};

	// A group of particles - Info and an array of Particles
	struct ParticleGroup
	{
		int p_count;		// Number of particles currently existing.
		int max_particles;	// Max particles allowed in group.
		int particles_allocated; // Actual allocated size.
		Particle list[1];	// Actually, num_particles in size

		inline void Remove(int i)
		{
			Particle& m = list[i];
			if (m.flags.is(Particle::DYING))	m		 = list[--p_count];
			else								m.flags.set(Particle::DYING,TRUE);
		}

		inline BOOL Add(const pVector &pos, const pVector &posB,
			const pVector &size, const pVector &rot, const pVector &vel, const pVector &color,
			const float alpha = 1.0f,
			const float age = 0.0f, float frame=0, u32 flags=Particle::BIRTH)
		{
			if(p_count >= max_particles)
				return FALSE;
			else
			{
				Particle& P = list[p_count];
				P.pos = pos;
				P.posB = posB;
				P.size = size;
				P.rot = rot;
				P.vel = vel;
				P.velB = vel;	// XXX This should be fixed.
				P.color = color;
				P.alpha = alpha;
				P.age = age;
				P.frame = frame;
				P.flags.set(flags);
				p_count++;
				return TRUE;
			}
		}
	};

	struct pDomain
	{
		PDomainEnum type;	// PABoxDomain, PASphereDomain, PAConeDomain...
		pVector p1, p2;		// Box vertices, Sphere center, Cylinder/Cone ends
		pVector u, v;		// Orthonormal basis vectors for Cylinder/Cone
		float radius1;		// Outer radius
		float radius2;		// Inner radius
		float radius1Sqr;	// Used for fast Within test of spheres,
		float radius2Sqr;	// and for mag. of u and v vectors for plane.

		BOOL Within(const pVector &) const;
		void Generate(pVector &) const;
		// transformation
		void transform(const pDomain& domain, const Fmatrix& m);
		void transform_dir(const pDomain& domain, const Fmatrix& m);
		// This constructor is used when default constructing a
		// ParticleAction that has a pDomain.
		inline pDomain()
		{
		}

		// Construct a domain in the standard way.
		pDomain(PDomainEnum dtype,
			float a0=0.0f, float a1=0.0f, float a2=0.0f,
			float a3=0.0f, float a4=0.0f, float a5=0.0f,
			float a6=0.0f, float a7=0.0f, float a8=0.0f);
	};

	//////////////////////////////////////////////////////////////////////
	// Type codes for all actions
	enum PActionEnum
	{
		PAHeaderID,			// The first action in each list.
		PAAvoidID,			// Avoid entering the domain of space.
		PABounceID,			// Bounce particles off a domain of space.
		PACallActionListID,	// 
		PACopyVertexBID,	// Set the secondary position from current position.
		PADampingID,		// Dampen particle velocities.
		PAExplosionID,		// An Explosion.
		PAFollowID,			// Accelerate toward the previous particle in the group.
		PAGravitateID,		// Accelerate each particle toward each other particle.
		PAGravityID,		// Acceleration in the given direction.
		PAJetID,			// 
		PAKillOldID,		// 
		PAMatchVelocityID,	// 
		PAMoveID,			// 
		PAOrbitLineID,		// 
		PAOrbitPointID,		// 
		PARandomAccelID,	// 
		PARandomDisplaceID,	// 
		PARandomVelocityID,	// 
		PARestoreID,		// 
		PASinkID,			// 
		PASinkVelocityID,	// 
		PASourceID,			// 
		PASpeedLimitID,		// 
		PATargetColorID,	// 
		PATargetSizeID,		// 
		PATargetRotateID,	// 
		PATargetRotateDID,	// 
		PATargetVelocityID,	// 
		PATargetVelocityDID,// 
		PAVortexID,			// 
		action_enum_force_dword = DWORD(-1)
	};

	///////////////////////////////////////////////////////////////////////////
	// Data types derived from Action.
	struct ParticleAction
	{
		enum{
			ALLOW_PARENT	= (1<<0)
		};
		static float	dt;	// This is copied to here from global state.
		Flags32			flags;
		PActionEnum		type;	// Type field
	};

	// This Methods actually does the particle's action.
	#define Methods	void Execute	(ParticleGroup *pg);\
					void Transform	(const Fmatrix& m);

	struct PAHeader : public ParticleAction
	{
		int actions_allocated;
		int count;			// Total actions in the list.
		float padding[126];	// This must be the largest action.

		Methods
	};

	struct PAAvoid : public ParticleAction
	{
		pDomain positionL;	// Avoid region (in local space)
		pDomain position;	// Avoid region
		float look_ahead;	// how many time units ahead to look
		float magnitude;	// what percent of the way to go each time
		float epsilon;		// add to r^2 for softening

		Methods
	};

	struct PABounce : public ParticleAction
	{
		pDomain positionL;	// Bounce region (in local space)
		pDomain position;	// Bounce region
		float oneMinusFriction;	// Friction tangent to surface
		float resilience;	// Resilence perpendicular to surface
		float cutoffSqr;	// cutoff velocity; friction applies iff v > cutoff

		Methods
	};

	struct PACallActionList : public ParticleAction
	{
		int action_list_num;	// The action list number to call

		Methods
	};

	struct PACopyVertexB : public ParticleAction
	{
		BOOL copy_pos;		// True to copy pos to posB.
		BOOL copy_vel;		// True to copy vel to velB.

		Methods
	};

	struct PADamping : public ParticleAction
	{
		pVector damping;	// Damping constant applied to velocity
		float vlowSqr;		// Low and high cutoff velocities
		float vhighSqr;

		Methods
	};

	struct PAExplosion : public ParticleAction
	{
		pVector centerL;	// The center of the explosion (in local space)
		pVector center;		// The center of the explosion
		float velocity;		// Of shock wave
		float magnitude;	// At unit radius
		float stdev;		// Sharpness or width of shock wave
		float age;			// How long it's been going on
		float epsilon;		// Softening parameter

		Methods
	};

	struct PAFollow : public ParticleAction
	{
		float magnitude;	// The grav of each particle
		float epsilon;		// Softening parameter
		float max_radius;	// Only influence particles within max_radius

		Methods
	};

	struct PAGravitate : public ParticleAction
	{
		float magnitude;	// The grav of each particle
		float epsilon;		// Softening parameter
		float max_radius;	// Only influence particles within max_radius

		Methods
	};

	struct PAGravity : public ParticleAction
	{
		pVector directionL;	// Amount to increment velocity (in local space)
		pVector direction;	// Amount to increment velocity

		Methods
	};

	struct PAJet : public ParticleAction
	{
		pVector	centerL;	// Center of the fan (in local space)
		pDomain accL;		// Acceleration vector domain  (in local space)
		pVector	center;		// Center of the fan
		pDomain acc;		// Acceleration vector domain
		float magnitude;	// Scales acceleration
		float epsilon;		// Softening parameter
		float max_radius;	// Only influence particles within max_radius

		Methods
	};

	struct PAKillOld : public ParticleAction
	{
		float age_limit;		// Exact age at which to kill particles.
		BOOL kill_less_than;	// True to kill particles less than limit.

		Methods
	};

	struct PAMatchVelocity : public ParticleAction
	{
		float magnitude;	// The grav of each particle
		float epsilon;		// Softening parameter
		float max_radius;	// Only influence particles within max_radius

		Methods
	};

	struct PAMove : public ParticleAction
	{

		Methods
	};

	struct PAOrbitLine : public ParticleAction
	{
		pVector pL, axisL;	// Endpoints of line to which particles are attracted (in local space)
		pVector p, axis;	// Endpoints of line to which particles are attracted
		float magnitude;	// Scales acceleration
		float epsilon;		// Softening parameter
		float max_radius;	// Only influence particles within max_radius

		Methods
	};

	struct PAOrbitPoint : public ParticleAction
	{
		pVector centerL;	// Point to which particles are attracted (in local space)
		pVector center;		// Point to which particles are attracted
		float magnitude;	// Scales acceleration
		float epsilon;		// Softening parameter
		float max_radius;	// Only influence particles within max_radius

		Methods
	};

	struct PARandomAccel : public ParticleAction
	{
		pDomain gen_accL;	// The domain of random accelerations.(in local space)
		pDomain gen_acc;	// The domain of random accelerations.

		Methods
	};

	struct PARandomDisplace : public ParticleAction
	{
		pDomain gen_dispL;	// The domain of random displacements.(in local space)
		pDomain gen_disp;	// The domain of random displacements.

		Methods
	};

	struct PARandomVelocity : public ParticleAction
	{
		pDomain gen_velL;	// The domain of random velocities.(in local space)
		pDomain gen_vel;	// The domain of random velocities.

		Methods
	};

	struct PARestore : public ParticleAction
	{
		float time_left;		// Time remaining until they should be in position.

		Methods
	};

	struct PASink : public ParticleAction
	{
		BOOL kill_inside;	// True to dispose of particles *inside* domain
		pDomain positionL;	// Disposal region (in local space)
		pDomain position;	// Disposal region

		Methods
	};

	struct PASinkVelocity : public ParticleAction
	{
		BOOL kill_inside;	// True to dispose of particles with vel *inside* domain
		pDomain velocityL;	// Disposal region (in local space)
		pDomain velocity;	// Disposal region

		Methods
	};

	struct PASpeedLimit : public ParticleAction
	{
		float min_speed;		// Clamp speed to this minimum.
		float max_speed;		// Clamp speed to this maximum.

		Methods
	};

	struct PASource : public ParticleAction
	{
		pDomain positionL;	// Choose a position in this domain. (local_space)
		pDomain velocityL;	// Choose a velocity in this domain. (local_space)
		pDomain position;	// Choose a position in this domain.
		pDomain velocity;	// Choose a velocity in this domain.
		pDomain rot;		// Choose a rotation in this domain.
		pDomain size;		// Choose a size in this domain.
		pDomain color;		// Choose a color in this domain.
		float alpha;		// Alpha of all generated particles
		float particle_rate;// Particles to generate per unit time
		float age;			// Initial age of the particles
		float age_sigma;	// St. dev. of initial age of the particles
		BOOL vertexB_tracks;// True to get positionB from position.
		pVector parent_vel;	
		float parent_motion;

		Methods
	};

	struct PATargetColor : public ParticleAction
	{
		pVector color;		// Color to shift towards
		float alpha;		// Alpha value to shift towards
		float scale;		// Amount to shift by (1 == all the way)

		Methods
	};

	struct PATargetSize : public ParticleAction
	{
		pVector size;		// Size to shift towards
		pVector scale;		// Amount to shift by per frame (1 == all the way)

		Methods
	};

	struct PATargetRotate : public ParticleAction
	{
		pVector rot;		// Rotation to shift towards
		float scale;		// Amount to shift by per frame (1 == all the way)

		Methods
	};

	struct PATargetVelocity : public ParticleAction
	{
		pVector velocityL;	// Velocity to shift towards (in local space)
		pVector velocity;	// Velocity to shift towards
		float scale;		// Amount to shift by (1 == all the way)

		Methods
	};

	struct PAVortex : public ParticleAction
	{
		pVector centerL;	// Center of vortex (in local space)
		pVector axisL;		// Axis around which vortex is applied (in local space)
		pVector center;		// Center of vortex
		pVector axis;		// Axis around which vortex is applied
		float magnitude;	// Scale for rotation around axis
		float epsilon;		// Softening parameter
		float max_radius;	// Only influence particles within max_radius

		Methods
	};

	// Global state vector
	struct _ParticleState
	{
		float	dt;
		BOOL	in_call_list;
		BOOL	in_new_list;

		int group_id;
		int list_id;
		ParticleGroup *pgrp;
		PAHeader *pact;
		int tid; // Only used in the MP case, but always define it.

		// These are static because all threads access the same groups.
		// All accesses to these should be locked.
		static ParticleGroup **group_list;
		static PAHeader **alist_list;
		static int group_count;
		static int alist_count;

		// state part
		BOOL	vertexB_tracks;
		pDomain Size;
		pDomain Vel;
		pDomain VertexB;
		pDomain Color;
		pDomain Rot;
		float	Alpha;
		float	Age;
		float	AgeSigma;
		float	parent_motion;

		_ParticleState();

		// Return an index into the list of particle groups where
		// p_group_count groups can be added.
		int			GenerateGroups	(int p_group_count);
		int			GenerateLists	(int alist_count);
		ParticleGroup *GetGroupPtr	(int p_group_num);
		PAHeader	*GetListPtr		(int action_list_num);
		// 
		void		ResetState		();
	};

	// All entry points call this to get their particle state.
	// For the non-MP case this is practically a no-op.
	PARTICLEDLL_API		_ParticleState& _GetPState();
	PARTICLEDLL_API		ParticleGroup*	_GetGroupPtr(int p_group_num);
	PARTICLEDLL_API		PAHeader*		_GetListPtr(int action_list_num);
#pragma pack( pop ) // push 4
}

#endif
