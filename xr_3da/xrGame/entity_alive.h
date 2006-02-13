#pragma once

#include "entity.h"


DEFINE_VECTOR(ref_shader, SHADER_VECTOR, SHADER_VECTOR_IT);
DEFINE_VECTOR(shared_str, STR_VECTOR, STR_VECTOR_IT);


class MONSTER_COMMUNITY;
class CEntityCondition;
class CWound;
class CCharacterPhysicsSupport;
class CMaterialManager;
class CVisualMemoryManager;

class CEntityAlive : public CEntity {
private:
	typedef	CEntity			inherited;			
public:
	virtual CEntityAlive*				cast_entity_alive		()						{return this;}
public:

	bool					m_bMobility;
	float					m_fAccuracy;
	float					m_fIntelligence;
	//m_PhysicMovementControl
	CPHMovementControl		*m_PhysicMovementControl;

public:
	// General
							CEntityAlive			();
	virtual					~CEntityAlive			();

	// Core events
	virtual DLL_Pure		*_construct				();
	virtual void			Load					(LPCSTR section);
	virtual void			reinit					();
	virtual void			reload					(LPCSTR section);

	//object serialization
	virtual void			save					(NET_Packet &output_packet);
	virtual void			load					(IReader &input_packet);


	virtual BOOL			net_Spawn				(CSE_Abstract* DC);
	virtual void			net_Destroy				();
	virtual	BOOL			net_SaveRelevant		();

	virtual void			shedule_Update			(u32 dt);

	virtual CPHMovementControl*	movement_control()	{return m_PhysicMovementControl;}
	virtual void			HitImpulse				(float amount, Fvector& vWorldDir, Fvector& vLocalDir);
	virtual	void			Hit						(SHit* pHDS);
	virtual void			Die						(CObject* who);
	virtual void			g_WeaponBones			(int &L, int &R1, int &R2)										= 0;
	
//	virtual float			GetfHealth				() const;
//	virtual float			SetfHealth				(float value);

//	virtual float			g_Health				()	const;
//	virtual float			g_MaxHealth				()	const;

	virtual float			g_Radiation				()	const;
	virtual	float			SetfRadiation			(float value);

	virtual float			CalcCondition			(float hit);

	// Visibility related
	virtual	float			ffGetFov				()	const			= 0;	
	virtual	float			ffGetRange				()	const			= 0;	
	
	virtual bool			human_being				() const			{return	(false);}
public:
	IC	CPHMovementControl*					PMovement					()						{return m_PhysicMovementControl;}

	virtual u16								PHGetSyncItemsNumber		()						;
	virtual CPHSynchronize*					PHGetSyncItem				(u16 item)				;
	virtual void							PHUnFreeze					()						;
	virtual void							PHFreeze					()						;

	virtual void							PHGetLinearVell				(Fvector& velocity)		;
	virtual CPHSoundPlayer*					ph_sound_player				()						;

protected:
	DEFINE_VECTOR				(CWound*, WOUND_VECTOR, WOUND_VECTOR_IT);
	WOUND_VECTOR				m_ParticleWounds;


	virtual void				StartFireParticles(CWound* pWound);
	virtual void				UpdateFireParticles();
	virtual void				LoadFireParticles(LPCSTR section);
public:	
	static  void				UnloadFireParticles	();
protected:
	static STR_VECTOR*			m_pFireParticlesVector;
	static u32					m_dwMinBurnTime;
	static float				m_fStartBurnWoundSize;
	static float				m_fStopBurnWoundSize;


	virtual void				BloodyWallmarks			(float P, const Fvector &dir, s16 element, const Fvector& position_in_object_space);
	static  void				LoadBloodyWallmarks		(LPCSTR section);
public:	
	static  void				UnloadBloodyWallmarks	();

	void						ClearBloodWounds		() {m_BloodWounds.clear();};
protected:
	virtual void				PlaceBloodWallmark		(const Fvector& dir, const Fvector& start_pos, 
														float trace_dist, float wallmark_size,
														SHADER_VECTOR& wallmarks_vector);

	//���������� � �������� �������� �� ������, ����� ��� ���� CEntityAlive
	static SHADER_VECTOR*		m_pBloodMarksVector;
	static float				m_fBloodMarkSizeMax;
	static float				m_fBloodMarkSizeMin;
	static float				m_fBloodMarkDistance;
	static float				m_fNominalHit;

	//��������� ������ �����
	static SHADER_VECTOR*		m_pBloodDropsVector;
	//������ ��� � ������� ������ �����
	
	DEFINE_VECTOR				(CWound*, WOUND_VECTOR, WOUND_VECTOR_IT);
	WOUND_VECTOR				m_BloodWounds;
	//������ ����, ���� ������ ������ �����
	static float				m_fStartBloodWoundSize;
	//������ ����, ���� ���������� �����
	static float				m_fStopBloodWoundSize;
	static float				m_fBloodDropSize;

	//���������� ���, � ��������� ������� �� �������� �����
	virtual void				StartBloodDrops			(CWound* pWound);
	virtual void				UpdateBloodDrops		();


	//��������� ����� ���������� � ����������� � ����
public:
	virtual	ALife::ERelationType tfGetRelationType	(const CEntityAlive *tpEntityAlive) const;
	virtual	bool				 is_relation_enemy	(const CEntityAlive *tpEntityAlive) const;	
public:	
	MONSTER_COMMUNITY*			monster_community;

private:
	CEntityCondition			*m_entity_condition;
	CMaterialManager			*m_material_manager;

protected:
	virtual	CEntityConditionSimple	*create_entity_condition	(CEntityConditionSimple* ec);

public:
	IC		CEntityCondition	&conditions					() const;
	IC		CMaterialManager	&material					() const {VERIFY(m_material_manager); return(*m_material_manager);}


protected:
	u32							m_ef_creature_type;
	u32							m_ef_weapon_type;
	u32							m_ef_detector_type;

public:
	virtual u32					ef_creature_type			() const;
	virtual u32					ef_weapon_type				() const;
	virtual u32					ef_detector_type			() const;

public:
	virtual	void				OnHitHealthLoss				(float NewHealth) {};	//���������� ���� entity ������ ��������
	virtual	void				OnCriticalHitHealthLoss		() {};	//���������� ���� entity ����� �� ���� 
	virtual	void				OnCriticalWoundHealthLoss	() {};	//���������� ���� entity ����� �� ������ ����� 
	virtual void				OnCriticalRadiationHealthLoss() {};	//���������� ���� entity ����� �� �������� 

	virtual	CVisualMemoryManager*visual_memory				() const {return(0);}
};

#include "entity_alive_inline.h"