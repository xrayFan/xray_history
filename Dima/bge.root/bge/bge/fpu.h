////////////////////////////////////////////////////////////////////////////
//	Module 		: fpu.h
//	Created 	: 01.10.2004
//  Modified 	: 01.10.2004
//	Author		: Dmitriy Iassenev
//	Description : FPU namespace
////////////////////////////////////////////////////////////////////////////

#pragma once

namespace FPU {
	u16					_24;
	u16					_24r;
	u16					_53;
	u16					_53r;
	u16					_64;
	u16					_64r;
	
	IC	void			m24	()
	{
		__asm fldcw _24
	}

	IC	void			m24r()
	{
		__asm fldcw _24r
	}

	IC	void			m53	()
	{
		__asm fldcw _53
	}

	IC	void			m53r()
	{
		__asm fldcw _53r
	}

	IC	void			m64	()
	{
		__asm fldcw _64
	}
	
	IC	void			m64r()
	{
		__asm fldcw _64r
	}
};

