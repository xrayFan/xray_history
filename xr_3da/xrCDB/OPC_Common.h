///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains common classes & defs used in OPCODE.
 *	\file		OPC_Common.h
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __OPC_COMMON_H__
#define __OPC_COMMON_H__

// [GOTTFRIED]: Just a small change for readability.
#ifdef OPC_CPU_COMPARE
	#define GREATER(x, y)	AIR(x) > IR(y)
#else
	#define GREATER(x, y)	fabsf(x) > (y)
#endif

	struct VertexPointers
	{
		const Point*	Vertex[3];
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	User-callback, called by OPCODE to request vertices from the app.
	 *	\param		triangle_index	[in] face index for which the system is requesting the vertices
	 *	\param		triangle		[out] triangle's vertices (must be provided by the user)
	 *	\param		user_data		[in] user-defined data from SetCallback()
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	typedef void	(*OPC_CALLBACK)	(udword triangle_index, VertexPointers& triangle, udword user_data);

	class OPCODE_API CollisionAABB
	{
		public:
		//! Constructor
		inline_				CollisionAABB()						{}
		//! Constructor
		inline_				CollisionAABB(const AABB& b)		{ b.GetCenter(mCenter);	b.GetExtents(mExtents);	}
		//! Destructor
		inline_				~CollisionAABB()					{}

		//! Get component of the box's min point along a given axis
		inline_	float		GetMin(udword axis)		const		{ return ((const float*)mCenter)[axis] - ((const float*)mExtents)[axis];	}
		//! Get component of the box's max point along a given axis
		inline_	float		GetMax(udword axis)		const		{ return ((const float*)mCenter)[axis] + ((const float*)mExtents)[axis];	}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Checks a box is inside another box.
		 *	\param		box		[in] the other box
		 *	\return		true if current box is inside input box
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		inline_	bool		IsInside(const CollisionAABB& box) const
							{
								if(box.GetMin(0)>GetMin(0))	return false;
								if(box.GetMin(1)>GetMin(1))	return false;
								if(box.GetMin(2)>GetMin(2))	return false;
								if(box.GetMax(0)<GetMax(0))	return false;
								if(box.GetMax(1)<GetMax(1))	return false;
								if(box.GetMax(2)<GetMax(2))	return false;
								return true;
							}

				Point		mCenter;				//!< Box center
				Point		mExtents;				//!< Box extents
	};

	class OPCODE_API QuantizedAABB
	{
		public:
		//! Constructor
		inline_				QuantizedAABB()			{}
		//! Destructor
		inline_				~QuantizedAABB()		{}

				sword		mCenter[3];				//!< Quantized center
				uword		mExtents[3];			//!< Quantized extents
	};

	class OPCODE_API CollisionFace
	{
		public:
		//! Constructor
		inline_				CollisionFace()			{}
		//! Destructor
		inline_				~CollisionFace()		{}

				udword		mFaceID;				//!< Index of touched face
				float		mDistance;				//!< Distance from collider to hitpoint
				float		mU, mV;					//!< Impact barycentric coordinates
	};

	class OPCODE_API CollisionFaces : private Container
	{
		public:
		//! Constructor
		inline_							CollisionFaces()						{}
		//! Destructor
		inline_							~CollisionFaces()						{}

		inline_	udword					GetNbFaces()					const	{ return GetNbEntries()>>2;						}
		inline_	const CollisionFace*	GetFaces()						const	{ return (const CollisionFace*)GetEntries();	}

		inline_	void					Reset()									{ Container::Reset();							}

		inline_	void					AddFace(const CollisionFace& face)		{ Add(face.mFaceID).Add(face.mDistance).Add(face.mU).Add(face.mV);	}
	};

	//! Quickly rotates & translates a vector
	inline_ void TransformPoint(Point& dest, const Point& source, const Matrix3x3& rot, const Point& trans)
	{
		dest.x = trans.x + source.x * rot.m[0][0] + source.y * rot.m[1][0] + source.z * rot.m[2][0];
		dest.y = trans.y + source.x * rot.m[0][1] + source.y * rot.m[1][1] + source.z * rot.m[2][1];
		dest.z = trans.z + source.x * rot.m[0][2] + source.y * rot.m[1][2] + source.z * rot.m[2][2];
	}

#endif //__OPC_COMMON_H__