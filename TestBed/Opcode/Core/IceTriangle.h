///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a handy triangle class.
 *	\file		IceTriangle.h
 *	\author		Pierre Terdiman
 *	\date		January, 17, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICETRIANGLE_H__
#define __ICETRIANGLE_H__

	// An indexed triangle class.
	class MESHMERIZER_API Triangle
	{
		public:
		//! Constructor
		__forceinline						Triangle()									{}
		//! Constructor
		__forceinline						Triangle(udword r0, udword r1, udword r2)	{ mVRef[0]=r0; mVRef[1]=r1; mVRef[2]=r2; }
		//! Destructor
		__forceinline						~Triangle()									{}
		//! Vertex-references
						udword				mVRef[3];

		// Methods
						void				Center(const Point* verts, Point& center)	const;
						bool				IsDegenerate()	const;
	};

#endif // __ICETRIANGLE_H__
