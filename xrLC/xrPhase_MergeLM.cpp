#include "stdafx.h"
#include "build.h"
#include "lightmap.h"
#include "xrPhase_MergeLM_Rect.h"

// Surface access
extern void _InitSurface	();
extern BOOL _rect_place		(L_rect &r, lm_layer*		D);

IC int	compare_defl		(CDeflector* D1, CDeflector* D2)
{
	// First  - by material
	WORD M1		= D1->GetBaseMaterial();
	WORD M2		= D2->GetBaseMaterial();
	if (M1<M2)	return	1;  // less
	if (M1>M2)	return	0;	// more
	return				2;	// equal
}

// by material / state changes
bool	compare1_defl		(CDeflector* D1, CDeflector* D2)
{
	switch (compare_defl(D1,D2))	
	{
	case 0:		return false;
	case 1:		return true;
	case 2:		return false;
	default:	return false;
	}
}

// as 1, but also by geometric locality
bool	compare2_defl		(CDeflector* D1, CDeflector* D2)
{
	switch (compare_defl(D1,D2))	
	{
	case 0:		return false;
	case 1:		return true;
	case 2:		
		{
			float	dist1	= Deflector->Sphere.P.distance_to(D1->Sphere.P) - D1->Sphere.R;
			float	dist2	= Deflector->Sphere.P.distance_to(D2->Sphere.P) - D2->Sphere.R;
			return	dist1 < dist2;
		}
	default:	return false;
	}
}

// by area of layer - reverse
bool	compare3_defl		(CDeflector* D1, CDeflector* D2)
{
	return D1->layer.Area() > D2->layer.Area();
}
class	pred_remove { public: IC bool	operator() (CDeflector* D) { { if (0==D) return TRUE;}; if (D->bMerged) {D->bMerged=FALSE; return TRUE; } else return FALSE;  }; };

// O(n*n) sorting
void	dumb_sort	(vecDefl& L)
{
	for (int n1=0; n1<int(L.size()); n1++)
	{
		Progress(float(n1)/float(L.size()));
		for (int n2=2; n2<int(L.size()); n2++)
			if (compare2_defl(L[n2],L[n2-1]))	std::swap(L[n2],L[n2-1]);
	}
}

void CBuild::xrPhase_MergeLM()
{
	vecDefl			Layer;

	// **** Select all deflectors, which contain this light-layer
	Layer.clear	();
	for (int it=0; it<(int)g_deflectors.size(); it++)
	{
		if (g_deflectors[it]->bMerged)					continue;
		Layer.push_back	(g_deflectors[it]);
	}

	// Merge this layer
	while (Layer.size()) 
	{
		string512	phase_name;
		sprintf		(phase_name,"Building lightmap %d...",g_lightmaps.size());
		Phase		(phase_name);

		// Sort layer by similarity (state changes)
		Status		("Selection 1...");
		clMsg		("LS: %d",	Layer.size());
		std::sort	(Layer.begin(),Layer.end(),compare1_defl);

		// Sort layer (by material and distance from "base" deflector)
		Status		("Selection 2...");
		clMsg		("LS: %d",	Layer.size());
		Deflector	= Layer[0];
		R_ASSERT	(Deflector);
		if (Layer.size()>2)	{
			// dumb_sort	(Layer);
			std::stable_sort(Layer.begin()+1,Layer.end(),compare2_defl);
		}

		// Select first deflectors which can fit
		Status		("Selection 3...");
		int maxarea		= c_LMAP_size*c_LMAP_size*4;	// Max up to 4 lm selected
		int curarea		= 0;
		int merge_count	= 0;
		for (it=0; it<(int)Layer.size(); it++)
		{
			int		defl_area	= Layer[it]->layer.Area();
			if (curarea + defl_area > maxarea) break;
			curarea		+=	defl_area;
			merge_count ++;
		}

		// Sort part of layer by size decreasing
		Status		("Selection 4...");
		std::sort	(Layer.begin(),Layer.begin()+merge_count,compare3_defl);

		// Startup
		Status		("Selection 5...");
		_InitSurface			();
		CLightmap*	lmap		= xr_new<CLightmap> ();
		g_lightmaps.push_back	(lmap);

		// Process 
		for (it=0; it<merge_count; it++) 
		{
			if (0==(it%1024))	Status	("Process [%d/%d]...",it,merge_count);
			lm_layer&	L		= Layer[it]->layer;
			L_rect		rT,rS; 
			rS.a.set	(0,0);
			rS.b.set	(L.width+2*BORDER-1, L.height+2*BORDER-1);
			rS.iArea	= L.Area();
			rT			= rS;
			if (_rect_place(rT,&L)) 
			{
				BOOL		bRotated;
				if (rT.SizeX() == rS.SizeX()) {
					R_ASSERT(rT.SizeY() == rS.SizeY());
					bRotated = FALSE;
				} else {
					R_ASSERT(rT.SizeX() == rS.SizeY());
					R_ASSERT(rT.SizeY() == rS.SizeX());
					bRotated = TRUE;
				}
				lmap->Capture		(Layer[it],rT.a.x,rT.a.y,rT.SizeX(),rT.SizeY(),bRotated);
				Layer[it]->bMerged	= TRUE;
			}
			Progress(_sqrt(float(it)/float(merge_count)));
		}
		Progress	(1.f);

		// Remove merged lightmaps
		Status		("Cleanup...");
		vecDeflIt last	= std::remove_if	(Layer.begin(),Layer.end(),pred_remove());
		Layer.erase		(last,Layer.end());

		// Save
		Status		("Saving...");
		lmap->Save	();
	}
	clMsg	("%d lightmaps builded",g_lightmaps.size());

	// Cleanup deflectors
	Progress	(1.f);
	Status		("Destroying deflectors...");
	for (u32 it=0; it<g_deflectors.size(); it++)
		xr_delete(g_deflectors[it]);
	g_deflectors.clear_and_free	();
}
