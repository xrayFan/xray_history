#pragma once
#include "customzone.h"

class CMosquitoBald :
	public CCustomZone
{
typedef	CCustomZone	inherited;
public:
	CMosquitoBald(void);
	~CMosquitoBald(void);

	virtual void Load(LPCSTR section);
	virtual void Update(u32 dt);
	virtual void Affect(CObject* O);

	u32 m_time;
	float m_hitImpulseScale;
};
