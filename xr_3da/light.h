#ifndef _XR_LIGHT_
#define _XR_LIGHT_
#pragma once

#include "_d3d_extensions.h"

#pragma pack(push,4)
class xrLIGHT       : public Flight
{
public:
	u32				dwFrame;
	u32				dwController;
};

class xrLIGHT_control
{
public:
	u32				dwFrame;
	CConstant*		dest;
	string64		name;
	xr_vector<u32>	data;
};
#pragma pack(pop)

#endif
