#pragma once

enum EStateAnimTriple {
	eStatePrepare,
	eStateExecute,
	eStateFinalize,
	eStateNone
};

class CAnimTriple {
	EStateAnimTriple	cur_state;

	typedef CMotionDef* MOTION_PTR;
	
	MOTION_PTR			pool[3];

	bool				m_active;

public:
						CAnimTriple		();
						~CAnimTriple	();

	void				init_external	(CMotionDef *m_def1, CMotionDef *m_def2, CMotionDef *m_def3);
	
	void				activate		();
	void				deactivate		();

	bool				is_active		() {return m_active;}

	// ����������� ��������, ���������� ������� �����
	bool				prepare_animation(CMotionDef **m);

	// ��������� ������������� �������� (������) � ������� � ���������
	void				pointbreak		();

private:
	void				switch_state	();

};

