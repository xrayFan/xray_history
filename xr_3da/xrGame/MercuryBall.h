///////////////////////////////////////////////////////////////
// MercuryBall.h
// MercuryBall - �������������� � ������������ ���
// �������������� � ����� �� �����
///////////////////////////////////////////////////////////////

#pragma once
#include "artifact.h"

class CMercuryBall : public CArtifact 
{
private:
	typedef CArtifact inherited;
public:
	CMercuryBall(void);
	virtual ~CMercuryBall(void);

	virtual void Load				(LPCSTR section);
	virtual void UpdateCL			();

protected:
	//����� ���������� ���������� ��������� ����
	_TIME_ID m_timeLastUpdate;
	//����� ����� ���������
	_TIME_ID m_timeToUpdate;

	//�������� ��������� ������� ����
	float m_fImpulseMin;
	float m_fImpulseMax;
};

/*

#pragma once
#include "gameobject.h"
#include "PhysicsShell.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// ������� ���
// ���������� ����� �������, �������� �������, ����� ���� ����������.
// ����:  �� 50 �� 200 ������, � ����������� �� ������� 
// ���������: ������� ���������� �����������, ������� ������ � ���������� ����������,
// �������� � ������� R1.
class CMercuryBall : public CGameObject {
typedef	CGameObject	inherited;
public:
	CMercuryBall(void);
	virtual ~CMercuryBall(void);

	virtual void OnH_A_Chield();
	virtual void OnH_B_Independent();

	//virtual	void Hit					(float P, Fvector &dir,	CObject* who, s16 element,Fvector p_in_object_space, float impulse){};
	virtual BOOL			net_Spawn			(LPVOID DC);
};
*/