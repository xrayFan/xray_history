#include "stdafx.h"
#pragma hdrstop

#include "r_backend_xform.h"

void	R_xforms::flush_cache	()
{
	// derive other matrices, if need
	if (flags & (dirty_W | dirty_V | dirty_P))
	{
		m_wv.mul_43	(m_v,m_w);
		m_wvp.mul	(m_p,m_wv);
		m_vp.mul	(m_p,m_v);
	}
	flags		= 0;

	// set mapping
	if (mapping->c_w)	RCache.constants.set(mapping->c_w,	m_w);
	if (mapping->c_v)	RCache.constants.set(mapping->c_v,	m_v);
	if (mapping->c_p)	RCache.constants.set(mapping->c_p,	m_p);
	if (mapping->c_wv)	RCache.constants.set(mapping->c_wv,	m_wv);
	if (mapping->c_vp)	RCache.constants.set(mapping->c_vp,	m_vp);
	if (mapping->c_wvp)	RCache.constants.set(mapping->c_wvp,m_wvp);
}
