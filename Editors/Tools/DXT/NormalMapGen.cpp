#include "stdafx.h"

#include "NV_Common.h"
#include "NVI_Convolution.h"
#include "NVI_Image.h"

//#include "tga.h"

using namespace xray_nvi;

enum KernelType
{
	KERNEL_4x,
	KERNEL_3x3,
	KERNEL_5x5,
	KERNEL_7x7,
	KERNEL_9x9,
	KERNEL_FORCEDWORD = 0xffffffff
};


//////////////////////////////////////////////////////////////////////
// Function prototypes

void MakeKernelElems( const float * pInWeightArray, int num_x, int num_y, ConvolutionKernelElement * pOutArray );
void RotateArrayCCW( const float * pInArray, int num_x, int num_y, float * pOutArray );


void ConvertAlphaToNormalMap_4x	( float scale, bool wrap );
void ConvertAlphaToNormalMap_3x3( float scale, bool wrap );
void ConvertAlphaToNormalMap_5x5( float scale, bool wrap );
void ConvertAlphaToNormalMap_7x7( float scale, bool wrap );
void ConvertAlphaToNormalMap_9x9( float scale, bool wrap );

//////////////////////////////////////////////////////////
u8		fpack		(float v)                               
{
	s32     _v      = iFloor        (((v+1)*.5f)*255.f + .5f);
	clamp   (_v,0,255);
	return  u8(_v);
}
Fvector vunpack		(s32 x, s32 y, s32 z)   
{
	Fvector pck;
	pck.x   = (float(x)/255.f - .5f)*2.f;
	pck.y   = (float(y)/255.f - .5f)*2.f;
	pck.z   = (float(z)/255.f - .5f)*2.f;
	return  pck;
}
Fvector vunpack		(Ivector src)                   
{
	return  vunpack (src.x,src.y,src.z);
}
Ivector	vpack      (Fvector src)
{
	src.normalize           ();
	Fvector                 _v;
	int bx                  = fpack (src.x);
	int by                  = fpack (src.y);
	int bz                  = fpack (src.z);
	// dumb test
	float   e_best  = flt_max;
	int             r=bx,g=by,b=bz;
	int             d=2;
	for (int x=_max(bx-d,0); x<=_min(bx+d,255); x++){
		for (int y=_max(by-d,0); y<=_min(by+d,255); y++){
			for (int z=_max(bz-d,0); z<=_min(bz+d,255); z++){
				_v                      = vunpack(x,y,z);
				float   m               = _v.magnitude();
				float   me              = _abs(m-1.f);
				if      (me>0.03f)      continue;
				_v.div  (m);
				float   e               = _abs(src.dotproduct(_v)-1.f);
				if (e<e_best)   {
					e_best          = e;
					r=x,g=y,b=z;
				}
			}
		}
	}
	Ivector         ipck;
	ipck.set        (r,g,b);
	return          ipck;
}

void 	CalculateNormalMap( NVI_Image* pSrc, ConvolutionKernel * pKernels, int num_kernels, float scale, bool wrap )
{
	// pKernels must be an array of at least two kernels
	// The first kernel extracts dh/dx  (change in height with respect to x )
	// the second extracts dh/dy  (change in height with respect to y )

	VERIFY(pKernels);
	VERIFY(num_kernels == 2);

	float			     results[2];

	////////////////////////////////////////////////////////
	// Set up the convolver & prepare image data

	Convolver	conv;

	conv.Initialize( &pSrc, pKernels, num_kernels, wrap );

	int size_y, size_x;

	size_x = (int) pSrc->GetWidth();
	size_y = (int) pSrc->GetHeight();
	DWORD * pArray = (DWORD*) pSrc->GetImageDataPointer();

	assert( pArray != NULL );

	/////////////////////////////////////////////////////
	// Now run the kernel over the source image area
	//  and write out the values.

	int				i,j;
	float			du, dv;
	unsigned long	nmap_color;
	char			height;

	// coordinates of source image (not padded)
	for( j=0; j < size_y; j++ )
	{
		for( i=0; i < size_x; i++ )
		{
			// apply kernels

			conv.Convolve_Alpha_At( i,j, results, 2 );

			du = results[0] * scale;
			dv = results[1] * scale;

			// det  | x  y  z |
			//      | 1  0 du |
			//      | 0  1 dv |
			//
			// cross product gives (-du, -dv, 1.0) as normal 

			float mag = du*du + dv*dv + 1.0f;
			mag = (float) _sqrt( mag );

			// Get alpha as height
			height = (char) ( pArray[ j * size_x + i ] ) >> 24;

			Fvector src =	{-du/mag, -dv/mag, 1.0f / mag};
			Ivector dst	=	vpack(src);
			nmap_color	=	color_rgba	(dst.x,dst.y,dst.z,0);
//.			AlphaAndVectorToARGB( height, -du/mag, -dv/mag, 1.0f / mag, nmap_color );

			pArray[ j * size_x + i ] = nmap_color;
		}
	}
}



void ConvertAlphaToNormalMap_4x( NVI_Image* pSrc, float scale, bool wrap )
{
	// Do the conversion using a 4 sample nearest neighbor
	//  pattern

	// d height / du kernel:
	//  0		0		0
	//	-1/2	0		1/2
	//	0		0		0

	// d height / dv kernel:
	//	0	1/2		0
	//	0	0		0
	//	0	-1/2	0

	int numelem;				// num elements in each kernel


	numelem = 2;
	ConvolutionKernelElement	du_elem[] =	{
		{-1, 0, -1.0f/2.0f}, { 1, 0, 1.0f/2.0f}			};


	ConvolutionKernelElement	dv_elem[] = {
			{ 0,  1,  1.0f/2.0f},
			{ 0, -1, -1.0f/2.0f}			 };

	
	int num_kernels = 2;
	ConvolutionKernel	 kernels[2];

	kernels[0].SetElements( numelem, du_elem );
	kernels[1].SetElements( numelem, dv_elem );


	///////////////////////////////////////////////////////
	//  Calc ARGB normal map & write to the "in." file

	CalculateNormalMap( pSrc, kernels, num_kernels, scale, wrap );


}


void ConvertAlphaToNormalMap_3x3( NVI_Image* pSrc, float scale, bool wrap )
{
	// Uses Anders' 3x3 kernels for transforming height 
	//  into a normal map vector.
	// Either wraps or does not wrap.
	// The convolver class memcopies to larger surface (width + kernel width-1, 
	//		height + kernel heigh - 1 ) to make wrap code easy for arbitrary
	//		kernels.  Edge texels are duplicated into the border regions or
	//      copied from the other side of the source image if wrapping is on.


	int numelem;				// num elements in each kernel

	float wt = 1.0f / 6.0f;

	// Kernel for change of height in u axis:
	//  -1/6	0	1/6
	//  -1/6	0	1/6
	//  -1/6	0	1/6

	numelem = 6;
	ConvolutionKernelElement	du_elem[] =	{
		{-1, 1, -wt}, { 1, 1, wt},
		{-1, 0, -wt}, { 1, 0, wt},
		{-1,-1, -wt}, { 1,-1, wt}	};

	// Kernel for change of height in v axis:
	//						 1,1
	//   1/6	 1/6	 1/6
	//     0	   0	   0
	//  -1/6	-1/6	-1/6
	// 0,0

	ConvolutionKernelElement	dv_elem[] = {
		{-1, 1,  wt}, { 0, 1, wt}, { 1, 1,  wt },
		{-1,-1, -wt}, { 0,-1,-wt}, { 1,-1, -wt } };

	
	int num_kernels = 2;
	ConvolutionKernel	 kernels[2];

	kernels[0].SetElements( numelem, du_elem );
	kernels[1].SetElements( numelem, dv_elem );


	///////////////////////////////////////////////////////
	//  Calc ARGB normal map & write to the "in." file

	CalculateNormalMap( pSrc, kernels, num_kernels, scale, wrap );

}



void ConvertAlphaToNormalMap_5x5( NVI_Image* pSrc, float scale, bool wrap )
{
	// Either wraps or does not wrap.
	// The convolver class memcopies to larger surface (width + kernel width-1, 
	//		height + kernel heigh - 1 ) to make wrap code easy for arbitrary
	//		kernels.  Edge texels are duplicated into the border regions or
	//      copied from the other side of the source image if wrapping is on.

	int numelem;				// num elements in each kernel


	float wt1 = 1.0f / 6.0f;
	float wt2 = 1.0f / 48.0f;

/*
	// Kernel for change of height in u axis:
	// The are cubic coefs for interpolation with sample 
	//  points at -2, -1, 1, and 2, hence the 0 coefs along
	//  center axis.  Resulting pattern is undesirable for 
	//  outstanding points in the height field.  These become
	//  a region of 4 mounds, when they should instead become a 
	//  smooth blob.

	//	1/48	-1/6	0	1/6		-1/48
	//	1/48	-1/6	0	1/6		-1/48
	//	0		0		0	0		0
	//	1/48	-1/6	0	1/6		-1/48
	//	1/48	-1/6	0	1/6		-1/48


	numelem = 16;
	ConvolutionKernelElement	du_elem[] =	{
		{-2, 2, wt2}, {-1,2,-wt1},    {1,2,wt1}, {2,2,-wt2},
		{-2, 1, wt2}, {-1,1,-wt1},    {1,1,wt1}, {2,1,-wt2},

		{-2,-1, wt2}, {-1,-1,-wt1},   {1,-1,wt1}, {2,-1,-wt2},
		{-2,-2, wt2}, {-1,-2,-wt1},   {1,-2,wt1}, {2,-2,-wt2}   };

	// Kernel for change of height in v axis:
	// This is same as u kernel above - just rotated 90 degrees

	ConvolutionKernelElement	dv_elem[] = {
		{-2, 2,-wt2}, {-1,2,-wt2},    {1,2,-wt2}, {2,2,-wt2},
		{-2, 1, wt1}, {-1,1, wt1},    {1,1, wt1}, {2,1, wt1},

		{-2,-1,-wt1}, {-1,-1,-wt1},   {1,-1,-wt1}, {2,-1,-wt1},
		{-2,-2, wt2}, {-1,-2, wt2},   {1,-2, wt2}, {2,-2, wt2}   };
*/


	

	numelem = 20;
	float wt22 = 1.0f/16.0f;
	float wt12 = 1.0f/10.0f;
	float wt02 = 1.0f/8.0f;
	float wt11 = 1.0f/2.8f;

	// Kernels using slope based on distance of that point from the 0,0
	// This is not from math derivation, but makes nice result

	ConvolutionKernelElement	du_elem[] =	{
		{-2, 2,-wt22 }, {-1, 2,-wt12 },   {1, 2, wt12 }, {2, 2, wt22 },
		{-2, 1,-wt12 }, {-1, 1,-wt11 },   {1, 1, wt11 }, {2, 1, wt12 },

		{-2, 0,-wt02 }, {-1, 0,-1.0f/2.0f },   {1, 0, 1.0f/2.0f }, {2, 0, wt02 },

		{-2,-1,-wt12 }, {-1,-1,-wt11 },   {1,-1, wt11 }, {2,-1, wt12 },
		{-2,-2,-wt22 }, {-1,-2,-wt12 },   {1,-2, wt12 }, {2,-2, wt22 }   };

	ConvolutionKernelElement	dv_elem[] = {
		{-2, 2, wt22 }, {-1, 2, wt12 }, {0, 2, 1.0f/4.0f },  {1, 2, wt12 }, {2, 2, wt22 },
		{-2, 1, wt12 }, {-1, 1, wt11 }, {0, 1, 1.0f/2.0f },  {1, 1, wt11 }, {2, 1, wt12 },

		{-2,-1,-wt12 }, {-1,-1,-wt11 }, {0,-1,-1.0f/2.0f },  {1,-1,-wt11 }, {2,-1,-wt12 },
		{-2,-2,-wt22 }, {-1,-2,-wt12 }, {0,-2,-1.0f/4.0f },  {1,-2,-wt12 }, {2,-2,-wt22 }   };


	// normalize the kernel so abs of all 
	// weights add to one
		
	int i;
	float usum, vsum;
	usum = vsum = 0.0f;
	for( i=0; i < numelem; i++ )
	{
		usum += (float) _abs( du_elem[i].weight );
		vsum += (float) _abs( dv_elem[i].weight );
	}
	for( i=0; i < numelem; i++ )
	{
		du_elem[i].weight /= usum;
		dv_elem[i].weight /= vsum;
	}



	int num_kernels = 2;
	ConvolutionKernel	 kernels[2];

	kernels[0].SetElements( numelem, du_elem );
	kernels[1].SetElements( numelem, dv_elem );


	///////////////////////////////////////////////////////
	//  Calc ARGB normal map & write to the "in." file

	CalculateNormalMap( pSrc, kernels, num_kernels, scale, wrap );

}


void MakeKernelElems( const float * pInWeightArray, int num_x, int num_y,
						ConvolutionKernelElement * pOutArray )
{
	// This makes coordinates for an array of weights, assumed to 
	//  be a rectangle.
	//
	// You must allocate pOutArray outside the function!
	// num_x and num_y should be odd
	//
	// Specify elems from  upper left corner (-num_x/2, num_y/2) to
	//   lower right corner. 
	// This generates the coordinates of the samples for you
	// For example:
	//  elem_array[] = { 00, 11, 22, 33, 44, 55, 66, 77, 88 }
	//  MakeKernelsElems( elem_array, 3, 3 )
	//  would make:
	//
	//   { -1, 1, 00 }  { 0, 1, 11 }  { 1, 1, 22 }
	//   { -1, 0, 33 }  { 0, 0, 44 }  { 1, 0, 55 }
	//   { -1, -1, 66}  ..etc 
	//
	//  As the array of kernel elements written to pOutArray
	//
	
	assert( pOutArray != NULL );
	assert( pInWeightArray != NULL );

	int i,j;
	int ind;

	for( j=0; j < num_y; j++ )
	{
		for( i=0; i < num_x; i++ )
		{
			ind = i + j * num_x;
			
			assert( ind < num_x * num_y );

			pOutArray[ ind ].x_offset =  (int)( i - num_x / 2 );
			pOutArray[ ind ].y_offset =  (int)( num_y / 2 - j );
			pOutArray[ ind ].weight   = pInWeightArray[ind];
		}
	}
}

void 	RotateArrayCCW( float * pInArray, int num_x, int num_y, float * pOutArray )
{
	// rotate an array of floats 90 deg CCW, so
	//    1, 0
	//    2, 3
	//  becomes
	//    0, 3
	//    1, 2

	assert( pOutArray != NULL );
	assert( pInArray != NULL );

	int i,j;
	int newi, newj;

	float * pSrc;

	////////////////////////////////////////////////////
	// If arrays are same, copy source to new temp array

	if( pInArray == pOutArray )
	{
		pSrc = new float[ num_x * num_y ];
		assert( pSrc != NULL );

		for( i=0; i < num_x * num_y; i++ )
		{
			pSrc[i] = pInArray[ i ];
		}
	}
	else
	{
		pSrc = pInArray;
	}
	////////////////////////////////////////////////////

	for( j=0; j < num_y; j++ )
	{
		for( i=0; i < num_x; i++ )
		{
			newj = num_x - i - 1;
			newi = j;

			// rotate dims of array too ==>  j * num_y

			pOutArray[ newi + newj * num_y ] = pSrc[ i + j * num_x ];
		}
	}


	if( pInArray == pOutArray )
	{
		SAFE_ARRAY_DELETE( pSrc );
	}	
}


void ConvertAlphaToNormalMap_7x7( NVI_Image* pSrc, float scale, bool wrap )
{
	// Either wraps or does not wrap.
	// The convolver class memcopies to larger surface (width + kernel width-1, 
	//		height + kernel heigh - 1 ) to make wrap code easy for arbitrary
	//		kernels.  Edge texels are duplicated into the border regions or
	//      copied from the other side of the source image if wrapping is on.

	int numelem;				// num elements in each kernel


	/////////////////////////////////////////
	// Kernel for change of height in u axis:
	// A Sobel filter kernel

	numelem = 49;


	float du_f[] = {
			-1, -2, -3,  0,  3, 2, 1,
			-2, -3, -4,  0,  4, 3, 2,
			-3, -4, -5,  0,  5, 4, 3,
			-4, -5, -6,  0,  6, 5, 4,
			-3, -4, -5,  0,  5, 4, 3,
			-2, -3, -4,  0,  4, 3, 2,
			-1, -2, -3,  0,  3, 2,	1   };



	ConvolutionKernelElement	du_elem[49];
	
	MakeKernelElems( du_f, 7, 7, & (du_elem[0] ) );

	/////////////////////////////////////////////
	// Kernel for change of height in v axis:

	float dv_f[49];
	
	RotateArrayCCW( &( du_f[0] ), 7, 7,  &( dv_f[0] ) );

	ConvolutionKernelElement	dv_elem[49];
	
	MakeKernelElems( dv_f, 7, 7, & ( dv_elem[0] ) );

	/////////////////////////////////////////////

	///////////////////////////////////////////
	// normalize the kernels so abs of all 
	// weights add to one
		
	int i;
	float usum, vsum;
	usum = vsum = 0.0f;

	for( i=0; i < numelem; i++ )
	{
		usum += (float) _abs( du_elem[i].weight );
		vsum += (float) _abs( dv_elem[i].weight );
	}

	for( i=0; i < numelem; i++ )
	{
		du_elem[i].weight /= usum;
		dv_elem[i].weight /= vsum;
	}

	///////////////////////////////////////////
	int num_kernels = 2;
	ConvolutionKernel	 kernels[2];

	kernels[0].SetElements( numelem, du_elem );
	kernels[1].SetElements( numelem, dv_elem );


	///////////////////////////////////////////////////////
	//  Calc ARGB normal map & write to the "in." file

	CalculateNormalMap( pSrc, kernels, num_kernels, scale, wrap );

}


void ConvertAlphaToNormalMap_9x9( NVI_Image* pSrc, float scale, bool wrap )
{
	// Either wraps or does not wrap.
	// The convolver class memcopies to larger surface (width + kernel width-1, 
	//		height + kernel heigh - 1 ) to make wrap code easy for arbitrary
	//		kernels.  Edge texels are duplicated into the border regions or
	//      copied from the other side of the source image if wrapping is on.
	//

	int numelem;				// num elements in each kernel


	/////////////////////////////////////////
	// Kernel for change of height in u axis:
	// A Sobel filter kernel

	numelem = 81;

	float du_f[] = {
			-1, -2, -3, -4,  0,  4, 3, 2, 1,
			-2, -3, -4, -5,  0,  5, 4, 3, 2,
			-3, -4, -5, -6,  0,  6, 5, 4, 3,
			-4, -5, -6, -7,  0,  7, 6, 5, 4,
			-5, -6, -7, -8,  0,  8, 7, 6, 5,
			-4, -5, -6, -7,  0,  7, 6, 5, 4,
			-3, -4, -5, -6,  0,  6, 5, 4, 3,
			-2, -3, -4, -5,  0,  5, 4, 3, 2,
			-1, -2, -3, -4,  0,  4, 3, 2, 1		};


	ConvolutionKernelElement	du_elem[81];
	
	MakeKernelElems( du_f, 9, 9, & (du_elem[0] ) );

	/////////////////////////////////////////////
	// Kernel for change of height in v axis:

	float dv_f[81];
	
	RotateArrayCCW( &( du_f[0] ), 9, 9,  &( dv_f[0] ) );

	ConvolutionKernelElement	dv_elem[81];
	
	MakeKernelElems( dv_f, 9, 9, & ( dv_elem[0] ) );

	/////////////////////////////////////////////

	///////////////////////////////////////////
	// normalize the kernels so abs of all 
	// weights add to one
		
	int i;
	float usum, vsum;
	usum = vsum = 0.0f;

	for( i=0; i < numelem; i++ )
	{
		usum += (float) _abs( du_elem[i].weight );
		vsum += (float) _abs( dv_elem[i].weight );
	}
	for( i=0; i < numelem; i++ )
	{
		du_elem[i].weight /= usum;
		dv_elem[i].weight /= vsum;
	}

	///////////////////////////////////////////
	int num_kernels = 2;
	ConvolutionKernel	 kernels[2];

	kernels[0].SetElements( numelem, du_elem );
	kernels[1].SetElements( numelem, dv_elem );


	///////////////////////////////////////////////////////
	//  Calc ARGB normal map & write to the "in." file

	CalculateNormalMap( pSrc, kernels, num_kernels, scale, wrap );

}

void ConvertToNormalMap(NVI_Image* pSrc, KernelType kt, float scale)
{
	switch (kt){
	case KERNEL_4x:		ConvertAlphaToNormalMap_4x	(pSrc,scale,true);	break;
	case KERNEL_3x3:	ConvertAlphaToNormalMap_3x3	(pSrc,scale,true);	break;
	case KERNEL_5x5:	ConvertAlphaToNormalMap_5x5	(pSrc,scale,true);	break;
	case KERNEL_7x7:	ConvertAlphaToNormalMap_7x7	(pSrc,scale,true);	break;
	case KERNEL_9x9:	ConvertAlphaToNormalMap_9x9	(pSrc,scale,true);	break;
	default: NODEFAULT;
	}
}

static float gloss_power=0.f;

IC u32 it_gloss_rev		(u32 d, u32 s)	
{	
	gloss_power			+= float(color_get_A(s))/255.f;
	return	color_rgba	(
	color_get_A(s)+1,		// gloss
	color_get_B(d),
	color_get_G(d),
	color_get_R(d)		);
}
IC u32 it_difference	(u32 d, u32 orig, u32 ucomp)	
{	
	return	color_rgba(
	128+(int(color_get_R(orig))-int(color_get_R(ucomp)))*2,		// R-error
	128+(int(color_get_G(orig))-int(color_get_G(ucomp)))*2,		// G-error
	128+(int(color_get_B(orig))-int(color_get_B(ucomp)))*2,		// B-error
	128+(int(color_get_A(orig))-int(color_get_A(ucomp)))*2	);	// A-error	
}
IC u32 it_height_rev	(u32 d, u32 s)	
{
	return	color_rgba	(
	color_get_A(d),					// diff x
	color_get_B(d),					// diff y
	color_get_G(d),					// diff z
	color_get_R(s)	);				// height
}

template	<class _It>
IC	void	TW_Iterate_1OP(u32 width, u32 height, u32 pitch, u8* dst, u8* src, const _It pred)
{
	for (u32 y = 0; y < height; y++)	{
		for (u32 x = 0; x < width; x++)	{
			u32&	pSrc	= *(((u32*)(src + (y * pitch)))+x);
			u32&	pDst	= *(((u32*)(dst + (y * pitch)))+x);
			pDst			= pred	(pDst,pSrc);
		}
	}
}

template	<class _It>
IC	void	TW_Iterate_2OP(u32 width, u32 height, u32 pitch, u8* dst, u8* src0, u8* src1, const _It pred)
{
	for (u32 y = 0; y < height; y++)	{
		for (u32 x = 0; x < width; x++)	{
			u32&	pSrc0	= *(((u32*)((u8*)src0 + (y * pitch)))+x);
			u32&	pSrc1	= *(((u32*)((u8*)src1 + (y * pitch)))+x);
			u32&	pDst	= *(((u32*)((u8*)dst  + (y * pitch)))+x);
			pDst			= pred(pDst,pSrc0,pSrc1);
		}
	}
}

u32	hsample	(s32 w, s32 h, s32 p, s32 x, s32 y, u8* src)
{
	if	(x<0)	x+=w;
				x%=w;
	if	(y<0)	y+=h;
				y%=h;

	return color_get_R( *(((u32*)((u8*)src + (y * p)))+x) );
}

#pragma comment(lib, "nvDXTlib.lib")

#include "ETextureParams.h"
#include "Image_DXTC.h"
extern bool DXTCompressImage	(LPCSTR out_name, u8* raw_data, u32 w, u32 h, u32 pitch, 
								 STextureParams* fmt, u32 depth);

bool DXTCompressBump(LPCSTR out_name, u8* T_height_gloss, u32 w, u32 h, u32 pitch, STextureParams* fmt, u32 depth)
{
	VERIFY				(4==depth);
	NVI_Image* pSrc		= new NVI_Image();
	pSrc->Initialize	(w,h,NVI_A8_R8_G8_B8,T_height_gloss);
	pSrc->AverageRGBToAlpha();

	// stage 0 
	pitch				= w*4;
	//tga_save			("x:\\0-height-gloss.tga",w,h,T_height_gloss,true);
	ConvertToNormalMap	(pSrc,KERNEL_5x5,fmt->bump_virtual_height*200.f);
	u8* T_normal_1		= pSrc->GetImageDataPointer();
	//tga_save			("x:\\1-normal_1.tga",w,h,T_normal_1,true);
	gloss_power			= 0.f;
	TW_Iterate_1OP		(w,h,pitch,T_normal_1,T_height_gloss,it_gloss_rev);		// T_height_gloss.a (gloss) -> T_normal_1 + reverse of channels
	gloss_power			/= float(w*h);
	//tga_save			("x:\\2-normal_1.tga",w,h,T_normal_1,true);

	STextureParams		fmt0;
	fmt0.flags.assign	(STextureParams::flGenerateMipMaps);
	fmt0.type			= STextureParams::ttImage;
	fmt0.fmt			= STextureParams::tfDXT5;
	bool bRes			= DXTCompressImage(out_name, T_normal_1, w, h, pitch, &fmt0, depth);

	// stage 1
	if (bRes){
		// Decompress (back)
		Image_DXTC* CI	= new Image_DXTC(); 
		if (CI->LoadFromFile(out_name)){
			VERIFY			((w==CI->Width())&&(h==CI->Height()));
			CI->Decompress	();
			u8* T_normal_1U	= CI->GetDecompDataPointer();
			//tga_save		("x:\\3-normal_1U.tga",w,h,T_normal_1U,true);

			// Calculate difference
			u8*	T_normal_1D	= (u8*) calloc( w * h, sizeof( u32 ));
			TW_Iterate_2OP	(w,h,pitch,T_normal_1D,T_normal_1,T_normal_1U,it_difference);
			//tga_save		("x:\\4-normal_1D.tga",w,h,T_normal_1D,true);

			// Rescale by virtual height
			float	h_scale	= _sqrt(fmt->bump_virtual_height/0.05f);	// move towards 1.0f
			if		(h_scale>1.f) h_scale = 1.f;
			{
				for (u32 y=0; y<h; y++)
				{
					for (u32 x=0; x<w; x++)
					{
						u32&	sh	= *	(((u32*)((u8*)T_height_gloss + (y * pitch)))+x);
						u32		h	= 	color_get_R(sh);		// height -> R-channel
								h	=	iFloor(float(h)*h_scale+EPS_S);
								sh	=	color_rgba(h,color_get_G(sh),color_get_B(sh),color_get_A(sh));
					}
				}
			}

			// Calculate bounds for centering
			u32		h_average=0, h_min=255, h_max=0;
			{
				for (u32 y=0; y<h; y++)
				{
					for (u32 x=0; x<w; x++)
					{
						u32		sh	= *	(((u32*)((u8*)T_height_gloss + (y * pitch)))+x);
						u32		h	= 	color_get_R(sh);	// height -> R-channel
						h_average	+=	h;
						if (h<h_min)	h_min	= h;
						if (h>h_max)	h_max	= h;
					}
				}
			}
			// final median, which will be used for centering
			u32		h_median	=	9*(h_average/(w*h)) + 1*( (h_max-h_min)/2 + h_min );
					h_median	/=	10;	
			s32		h_correction=	s32(127)-s32(h_median);

			// Calculate filtered and corrected height
			u8*		T_height_pf	=	(u8*) calloc( w * h, sizeof( u32 ));	// filtered for parallax
			for (s32 y=0; y<h; y++)
			{
				u32		p = pitch;
				u8*		T = T_height_gloss;
				for (s32 x=0; x<w; x++)
				{
					u32&	dst		= *	(((u32*)((u8*)T_height_pf + (y * pitch)))+x);
					u32		val		= 
						hsample(w,h,p,x-1,y-1,T) + hsample(w,h,p,x+0,y-1,T) + hsample(w,h,p,x+1,y-1,T) +
						hsample(w,h,p,x-1,y+0,T) + hsample(w,h,p,x+0,y+0,T) + hsample(w,h,p,x+1,y+0,T) +
						hsample(w,h,p,x-1,y+1,T) + hsample(w,h,p,x+0,y+1,T) + hsample(w,h,p,x+1,y+1,T);
					s32		r		= clampr( s32(val/9) + h_correction, 0, 255);
							dst		= color_rgba(r,r,r,r);
				}
			}

			// Reverse channels back + transfer heightmap
			TW_Iterate_1OP	(w,h,pitch,T_normal_1D,T_height_pf,it_height_rev);
			//tga_save		("x:\\5-normal_1D.tga",w,h,T_normal_1D,true);
			
			// Compress
			STextureParams	fmt0;
			fmt0.flags.assign(STextureParams::flGenerateMipMaps);
			fmt0.type		= STextureParams::ttImage;
			fmt0.fmt		= STextureParams::tfDXT5;
			string256		out_name1;
			strcpy			(out_name1,out_name); if (strext(out_name1)) *strext(out_name1)=0;
			strcat			(out_name1,"#.dds");
			bRes			|= DXTCompressImage(out_name1, T_normal_1D, w, h, pitch, &fmt0, depth);

			free			(T_height_pf);
			free			(T_normal_1D);
		}else{
			bRes			= false;
		}
		delete				(CI);
	}

	delete					(pSrc);

	if (gloss_power<0.1f){
		MessageBox			(0,"Invalid gloss mask. Too low reflectance.","Error",MB_ICONERROR);
		return				false;
	}

	return bRes;
}
/*
_BUMP:
{
	// Load   SYS-MEM-surface, bound to device restrictions
	D3DXIMAGE_INFO			IMG;
	IReader* S				= FS.r_open	(fn);
	IDirect3DTexture9*		T_height_gloss;
	R_CHK(D3DXCreateTextureFromFileInMemoryEx(
		HW.pDevice,	S->pointer(),S->length(),
		D3DX_DEFAULT,D3DX_DEFAULT,	D3DX_DEFAULT,0,D3DFMT_A8R8G8B8,
		D3DPOOL_SYSTEMMEM,			D3DX_DEFAULT,D3DX_DEFAULT,
		0,&IMG,0,&T_height_gloss	));
	FS.r_close				(S);
	//TW_Save						(T_height_gloss,fname,"debug-0","original");

	// Create HW-surface, compute normal map
	IDirect3DTexture9*	T_normal_1	= 0;
	R_CHK(D3DXCreateTexture		(HW.pDevice,IMG.Width,IMG.Height,D3DX_DEFAULT,0,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM,&T_normal_1));
	R_CHK(D3DXComputeNormalMap	(T_normal_1,T_height_gloss,0,0,D3DX_CHANNEL_RED,4.f));
	//TW_Save					(T_normal_1,fname,"debug-1","normal");

	// Transfer gloss-map
	TW_Iterate_1OP				(T_normal_1,T_height_gloss,it_gloss_rev);
	//TW_Save						(T_normal_1,fname,"debug-2","normal-G");

	// Compress
	fmt								= D3DFMT_DXT5;
	IDirect3DTexture9*	T_normal_1C	= TW_LoadTextureFromTexture(T_normal_1,fmt,psTextureLOD,dwWidth,dwHeight);
	//TW_Save						(T_normal_1C,fname,"debug-3","normal-G-C");

#if RENDER==R_R2
	// Decompress (back)
	fmt								= D3DFMT_A8R8G8B8;
	IDirect3DTexture9*	T_normal_1U	= TW_LoadTextureFromTexture(T_normal_1C,fmt,0,dwWidth,dwHeight);
	// TW_Save						(T_normal_1U,fname,"debug-4","normal-G-CU");

	// Calculate difference
	IDirect3DTexture9*	T_normal_1D = 0;
	R_CHK(D3DXCreateTexture(HW.pDevice,dwWidth,dwHeight,T_normal_1U->GetLevelCount(),0,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM,&T_normal_1D));
	TW_Iterate_2OP				(T_normal_1D,T_normal_1,T_normal_1U,it_difference);
	// TW_Save						(T_normal_1D,fname,"debug-5","normal-G-diff");

	// Reverse channels back + transfer heightmap
	TW_Iterate_1OP				(T_normal_1D,T_height_gloss,it_height_rev);
	// TW_Save						(T_normal_1D,fname,"debug-6","normal-G-diff-H");

	// Compress
	fmt								= D3DFMT_DXT5;
	IDirect3DTexture9*	T_normal_2C	= TW_LoadTextureFromTexture(T_normal_1D,fmt,0,dwWidth,dwHeight);
	// TW_Save						(T_normal_2C,fname,"debug-7","normal-G-diff-H-C");
	_RELEASE					(T_normal_1U	);
	_RELEASE					(T_normal_1D	);

	// 
	string256			fnameB;
	strconcat			(fnameB,"$user$",fname,"X");
	ref_texture			t_temp		= Device.Resources->_CreateTexture	(fnameB);
	t_temp->surface_set	(T_normal_2C	);
	_RELEASE			(T_normal_2C	);	// texture should keep reference to it by itself
#endif
*/