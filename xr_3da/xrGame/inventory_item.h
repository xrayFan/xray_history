////////////////////////////////////////////////////////////////////////////
//	Module 		: inventory_item.h
//	Created 	: 24.03.2003
//  Modified 	: 29.01.2004
//	Author		: Victor Reutsky, Yuri Dobronravin
//	Description : Inventory item
////////////////////////////////////////////////////////////////////////////

#pragma once

#include "inventory_space.h"
#include "phnetstate.h"
#include "hit_immunity.h"

enum EHandDependence{
	hdNone	= 0,
	hd1Hand	= 1,
	hd2Hand	= 2
};


class CSE_Abstract;
class CGameObject;
class CFoodItem;
class CMissile;
class CHudItem;
class CWeaponAmmo;
class CWeapon;
class CPhysicsShellHolder;
class NET_Packet;
class CEatableItem;

struct net_updateData;

class CInventoryItem : public CHitImmunity {
protected:
	enum EIIFlags{				Fdrop				=(1<<0),
								FCanTake			=(1<<1),
								FCanTrade			=(1<<2),
								Fbelt				=(1<<3),
								Fruck				=(1<<4),
								FRuckDefault		=(1<<5),
								FUsingCondition		=(1<<6),
								FAllowSprint		=(1<<7),
								Fuseful_for_NPC		=(1<<8),
								FInInterpolation	=(1<<9),
								FInInterpolate		=(1<<10),
	};

	Flags16						m_flags;
public:
								CInventoryItem		();
	virtual						~CInventoryItem		();

public:
	virtual void				Load				(LPCSTR section);

	virtual LPCSTR				Name				();
	virtual LPCSTR				NameShort			();
	virtual LPCSTR				NameComplex			();
	shared_str					ItemDescription		() { return m_Description; }

	
	virtual void				OnEvent				(NET_Packet& P, u16 type);
	
	virtual bool				Useful				() const;									// !!! ��������������. (��. � Inventory.cpp)
	virtual bool				Attach				(PIItem pIItem) {return false;}
	virtual bool				Detach				(PIItem pIItem) {return false;}
	//��� ������ ��������� ����� ���� ��� ������� �������� ������
	virtual bool				Detach				(const char* item_section_name);
	virtual bool				CanAttach			(PIItem pIItem) {return false;}
	virtual bool				CanDetach			(LPCSTR item_section_name) {return false;}

	virtual EHandDependence		HandDependence		()	const	{return hd1Hand;};
	virtual bool				IsSingleHanded		()	const	{return true;};	
	virtual bool				Activate			();									// !!! ��������������. (��. � Inventory.cpp)
	virtual void				Deactivate			();								// !!! ��������������. (��. � Inventory.cpp)
	virtual bool				Action				(s32 cmd, u32 flags) {return false;}	// true ���� ��������� �������, ����� false

	// ���� �������� � ���������
	virtual bool				IsHidden			()	const	{return true;}
	//���� ��������� � ���������
	virtual bool				IsHiding			()	const	{return false;}

	virtual void				OnAnimationEnd		()		{}
	
	virtual s32					Sort				(PIItem pIItem);						// !!! ��������������. (��. � Inventory.cpp)
	virtual bool				Merge				(PIItem pIItem);						// !!! ��������������. (��. � Inventory.cpp)

	virtual void				OnH_B_Chield		();
	virtual void				OnH_A_Chield		();
    virtual void				OnH_B_Independent	();
	virtual void				OnH_A_Independent	();

	virtual void				save				(NET_Packet &output_packet);
	virtual void				load				(IReader &input_packet);
	virtual BOOL				net_SaveRelevant	()								{return TRUE;}


	virtual void				UpdateCL			();

	virtual	void				Hit					(	float P, Fvector &dir,	
														CObject* who, s16 element,
														Fvector position_in_object_space, 
														float impulse, 
														ALife::EHitType hit_type = ALife::eHitTypeWound);

			void				Drop				();		// ���� ������ � ���������, �� �� ����� ��������
			BOOL				GetDrop				() const	{ return m_flags.test(Fdrop);}
			void				SetDrop				(BOOL val)	{ m_flags.set(Fdrop, val);}

			u32					Cost				() const	{ return m_cost; }
			float				Weight				() const	{ return m_weight;}		

public:
	CInventory*					m_pInventory;
	shared_str					m_name;
	shared_str					m_nameShort;
	shared_str					m_nameComplex;

	EItemPlace					m_eItemPlace;


	virtual void				OnMoveToSlot		() {};
	virtual void				OnMoveToBelt		() {};
	virtual void				OnMoveToRuck		() {};
					
			int					GetGridWidth		() const ;//{return m_iGridWidth;}
			int					GetGridHeight		() const ;//{return m_iGridHeight;}
			int					GetXPos				() const ;//{return m_iXPos;}
			int					GetYPos				() const ;//{return m_iYPos;}
	//---------------------------------------------------------------------
			float				GetKillMsgXPos		() const ;
			float				GetKillMsgYPos		() const ;
			float				GetKillMsgWidth		() const ;
			float				GetKillMsgHeight	() const ;
	//---------------------------------------------------------------------
			float				GetCondition		() const					{return m_fCondition;}
			void				ChangeCondition		(float fDeltaCondition);

	virtual u32					GetSlot				()  const					{return m_slot;}

			bool				Belt				()							{return !!m_flags.test(Fbelt);}
			void				Belt				(bool on_belt)				{m_flags.set(Fbelt,on_belt);}
			bool				Ruck				()							{return !!m_flags.test(Fruck);}
			void				Ruck				(bool on_ruck)				{m_flags.set(Fruck,on_ruck);}
			bool				RuckDefault			()							{return !!m_flags.test(FRuckDefault);}
			
	virtual bool				CanTake				() const					{return !!m_flags.test(FCanTake);}
			bool				CanTrade			() const;
	virtual bool 				IsNecessaryItem	    (CInventoryItem* item)		{return false;};
protected:

	// ���� � ������� ����� ���������� ������ (NO_ACTIVE_SLOT ���� ������)
	u32							m_slot;
	// ���� �� ���������
	u32							m_cost;
	// ��� ������� (��� �������������� �����)
	float						m_weight;
	
	//��������� ����, 1.0 - ��������� ���������������
	// 0 - �����������
	float						m_fCondition;

	// ����� �������� ����
	shared_str					m_Description;

	ALife::_TIME_ID				m_dwItemRemoveTime;
	ALife::_TIME_ID				m_dwItemIndependencyTime;

	float						m_fControlInertionFactor;

	////////// network //////////////////////////////////////////////////
public:
	virtual void				make_Interpolation	();
	virtual void				PH_B_CrPr			(); // actions & operations before physic correction-prediction steps
	virtual void				PH_I_CrPr			(); // actions & operations after correction before prediction steps
#ifdef DEBUG
	virtual void				PH_Ch_CrPr			(); // 
#endif
	virtual void				PH_A_CrPr			(); // actions & operations after phisic correction-prediction steps

	virtual void				net_Import			(NET_Packet& P);					// import from server
	virtual void				net_Export			(NET_Packet& P);					// export to server

	virtual void				activate_physic_shell		();

	virtual bool				NeedToDestroyObject			() const;
	virtual ALife::_TIME_ID		TimePassedAfterIndependant	() const;

	virtual	bool				IsSprintAllowed				() const		{return !!m_flags.test(FAllowSprint);} ;

	virtual	float				GetControlInertionFactor(	) const			{return m_fControlInertionFactor;};

protected:
	virtual void				UpdateXForm	();

protected:
/*
	struct net_update_IItem {	u32					dwTimeStamp;
								SPHNetState			State;};

	xr_deque<net_update_IItem>	NET_IItem;
	/// spline coeff /////////////////////
	float			SCoeff[3][4];

#ifdef DEBUG
	DEF_VECTOR		(VIS_POSITION, Fvector);
	VIS_POSITION	LastVisPos;
#endif

	Fvector			IStartPos;
	Fquaternion		IStartRot;

	Fvector			IRecPos;
	Fquaternion		IRecRot;

	Fvector			IEndPos;
	Fquaternion		IEndRot;	

	SPHNetState		LastState;
	SPHNetState		RecalculatedState;

#ifdef DEBUG
	SPHNetState		CheckState;
#endif
	SPHNetState		PredictedState;

	bool			m_bInInterpolation		;
	bool			m_bInterpolate			;
	u32				m_dwIStartTime			;
	u32				m_dwIEndTime			;
	u32				m_dwILastUpdateTime		;
*/

	net_updateData*				m_net_updateData;
	net_updateData*				NetSync						();
	void						CalculateInterpolationParams();

public:
	virtual BOOL				net_Spawn				(CSE_Abstract* DC);
	virtual void				net_Destroy				();
	virtual void				renderable_Render		();
	virtual void				reload					(LPCSTR section);
	virtual void				reinit					();
	virtual bool				can_kill				() const;
	virtual CInventoryItem*		can_kill				(CInventory *inventory) const;
	virtual const CInventoryItem*can_kill				(const xr_vector<const CGameObject*> &items) const;
	virtual CInventoryItem*		can_make_killing		(const CInventory *inventory) const;
	virtual bool				ready_to_kill			() const;
	IC		bool				useful_for_NPC			() const;
#ifdef DEBUG
	virtual void				OnRender					();
#endif

public:
	virtual DLL_Pure*			_construct					();
	IC	CPhysicsShellHolder&	object						() const{ VERIFY		(m_object); return		(*m_object);}
	virtual void				on_activate_physic_shell	() = 0;

protected:
	float						m_holder_range_modifier;
	float						m_holder_fov_modifier;

public:
	virtual	void				modify_holder_params		(float &range, float &fov) const;

protected:
	IC	CInventoryOwner&		inventory_owner				() const;

private:
	CPhysicsShellHolder*		m_object;
public:
	virtual CInventoryItem		*cast_inventory_item		()	{return this;}
	virtual CPhysicsShellHolder	*cast_physics_shell_holder	()	{return 0;}
	virtual CEatableItem		*cast_eatable_item			()	{return 0;}
	virtual CWeapon				*cast_weapon				()	{return 0;}
	virtual CFoodItem			*cast_food_item				()	{return 0;}
	virtual CMissile			*cast_missile				()	{return 0;}
	virtual CHudItem			*cast_hud_item				()	{return 0;}
	virtual CWeaponAmmo			*cast_weapon_ammo			()	{return 0;}
	virtual CGameObject			*cast_game_object			()  {return 0;};


};

#include "inventory_item_inline.h"