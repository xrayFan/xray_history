#include "stdafx.h"
#pragma hdrstop

#include "r_backend_xform.h"

void	R_xforms::set_W			(const Fmatrix& m)
{
	m_w.set			(m);
	m_wv.mul_43		(m_v,m_w);
	m_wvp.mul		(m_p,m_wv);
	if (c_w)		RCache.set_c(c_w,	m_w);
	if (c_wv)		RCache.set_c(c_wv,	m_wv);
	if (c_wvp)		RCache.set_c(c_wvp,	m_wvp);
	RCache.set_xform(D3DTS_WORLD,m);
}
void	R_xforms::set_V			(const Fmatrix& m)
{
	m_v.set			(m);
	m_wv.mul_43		(m_v,m_w);
	m_vp.mul		(m_p,m_v);
	m_wvp.mul		(m_p,m_wv);
	if (c_v)		RCache.set_c(c_v,	m_v);
	if (c_vp)		RCache.set_c(c_vp,	m_vp);
	if (c_wv)		RCache.set_c(c_wv,	m_wv);
	if (c_wvp)		RCache.set_c(c_wvp,	m_wvp);
	RCache.set_xform(D3DTS_VIEW,m);
}
void	R_xforms::set_P			(const Fmatrix& m)
{
	m_p.set			(m);
	m_vp.mul		(m_p,m_v);
	m_wvp.mul		(m_p,m_wv);
	if (c_p)		RCache.set_c(c_p,	m_p);
	if (c_vp)		RCache.set_c(c_vp,	m_vp);
	if (c_wvp)		RCache.set_c(c_wvp,	m_wvp);
	RCache.set_xform(D3DTS_PROJECTION,m);
}
void	R_xforms::unmap			()
{
	c_w			= NULL;
	c_v			= NULL;
	c_p			= NULL;
	c_wv		= NULL;
	c_vp		= NULL;
	c_wvp		= NULL;
}
void	R_xforms::R_xforms		()
{
	unmap		();
	set_W		(Fidentity);
	set_V		(Fidentity);
	set_P		(Fidentity);
}
