#pragma once
#include "inventory_item.h"

class CCartridge 
{
public:
	CCartridge();
	void Load(LPCSTR section);

	ref_str	m_ammoSect;
	float	m_kDist, m_kDisp, m_kHit, m_kImpulse, m_kPierce;
	bool	m_tracer;
	int		m_buckShot;
	float	m_impair;
	float	fWallmarkSize;
};

class CWeaponAmmo :	public CInventoryItem
{
	typedef CInventoryItem		inherited;
public:
	CWeaponAmmo(void);
	virtual ~CWeaponAmmo(void);

	virtual void Load(LPCSTR section);
	virtual BOOL net_Spawn(LPVOID DC);
	virtual void net_Destroy();
	virtual void net_Export(NET_Packet& P);	// export to server
	virtual void net_Import(NET_Packet& P);	// import from server
	virtual void OnH_B_Chield();
	virtual void OnH_B_Independent();
	virtual void UpdateCL();
	virtual void renderable_Render();

	virtual const char* Name();
	virtual const char* NameShort();

	virtual bool Useful() const;
	virtual s32 Sort(PIItem pIItem);
	virtual bool Merge(PIItem pIItem);

	bool Get(CCartridge &cartridge);

	float m_kDist, m_kDisp, m_kHit, m_kImpulse, m_kPierce;
	bool m_tracer;
	int	m_buckShot;
	float m_impair;
	float fWallmarkSize;

	u16			m_boxSize;			// ������ ������� (������� �������� ����� ����� �� ���)
	u16			m_boxCurr;			// ������� �������� �������� � ������� (������� ������� �������)
	string64	m_tmpName;

protected:
	////////////// network //////////////////	
};
