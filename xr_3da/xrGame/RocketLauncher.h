//////////////////////////////////////////////////////////////////////
// RocketLauncher.h:	��������� ��� ��������� �������� 
//						���������� ��������� � ��������
//////////////////////////////////////////////////////////////////////


#pragma once

class CCustomRocket;

class CRocketLauncher
{
public:
	CRocketLauncher		();
	~CRocketLauncher	();

	void AttachRocket	(u16 rocket_id, CGameObject* parent_rocket_launcher);
	void SpawnRocket	(LPCSTR rocket_section, CGameObject* parent_rocket_launcher);
	void LaunchRocket	(const Fmatrix& xform,  const Fvector& vel, const Fvector& angular_vel);

protected:			   
	CCustomRocket*	m_pRocket; 
};