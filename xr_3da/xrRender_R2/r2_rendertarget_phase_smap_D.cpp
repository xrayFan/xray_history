#include "stdafx.h"

void	CRenderTarget::phase_smap_direct()
{
	// Targets
	if (RImplementation.b_HW_smap)		u_setrt	(rt_smap_d_surf, NULL, NULL, rt_smap_d_depth->pRT);
	else								u_setrt	(rt_smap_d_surf, NULL, NULL, rt_smap_d_ZB);

	// Clear
	CHK_DX								(HW.pDevice->Clear( 0L, NULL, D3DCLEAR_ZBUFFER,	0xFFFFFFFF, 1.0f, 0L));

	// Stencil	- disable
	RCache.set_Stencil					( FALSE );

	// Misc		- draw only back-faces
	RCache.set_CullMode					( CULL_CCW );
	if (RImplementation.b_HW_smap)		RCache.set_ColorWriteEnable	(FALSE);
}
