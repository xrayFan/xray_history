////////////////////////////////////////////////////////////////////////////
//	Module 		: xrServer_Objects.h
//	Created 	: 19.09.2002
//  Modified 	: 04.06.2003
//	Author		: Oles Shishkovtsov, Alexander Maksimchuk, Victor Retsky and Dmitriy Iassenev
//	Description : Server objects
////////////////////////////////////////////////////////////////////////////

#ifndef xrServer_ObjectsH
#define xrServer_ObjectsH

#include "xrMessages.h"
#include "ai_alife_interfaces.h"
#include "xrServer_Space.h"

#ifdef _EDITOR
	#include "PropertiesListHelper.h"
#endif

#define SPAWN_VERSION	u16(21)
//------------------------------------------------------------------------------
// Version history
//------------------------------------------------------------------------------
// 10 - CSE_ALifeObjectPhysic append 	'fixed_bone'
// 11 - CSE_ALifeObjectHangingLamp append 	'spot_brightness'
// 12 - CSE_ALifeObjectHangingLamp append 	'flags'
// 13 - CSE_ALifeObjectHangingLamp append 	'mass'
// 14 - CSE_ALifeObjectPhysic append 	inherited from CSE_ALifeObject
// 15 - CSE_ALifeAnomalousZone append 			inherited calls from CSE_ALifeDynamicObject
// 16 - CSE_ALifeObjectPhysic append 	inherited from CSE_ALifeDynamicObject
// 17 - xrSE_...		  append 	inherited from CSE_Visual for smart Level Editor
// 18 - CSE_ALifeObjectHangingLamp  append 	'startup_animation'
// 19 - xrSE_Teamed		  didn't save health parameter
// 20 - CSE_ALife...		  saving vectors in UPDATE_Read/UPDATE_Write changed to STATE_Read/STATE_Write
// 21 - Global class hierarchy update
//------------------------------------------------------------------------------

class xrClientData;

class CSE_Abstract : public IPureServerObject {
public:
	BOOL							net_Ready;
	BOOL							net_Processed;	// Internal flag for connectivity-graph
	
	u16								m_wVersion;
	u16								RespawnTime;

	u16								ID;				// internal ID
	u16								ID_Parent;		// internal ParentID, 0xffff means no parent
	u16								ID_Phantom;		// internal PhantomID, 0xffff means no phantom
	xrClientData*					owner;
	xr_vector<u16>					children;

	// spawn data
	string64						s_name;
	string64						s_name_replace;
	u8								s_gameid;
	u8								s_RP;
	Flags16							s_flags;		// state flags

	// update data
	Fvector							o_Position;
	Fvector							o_Angle;

	// for ALife control
	bool							m_bALifeControl;

									CSE_Abstract(LPCSTR caSection)
	{
		RespawnTime					= 0;
		net_Ready					= FALSE;
		ID							= 0xffff;
        ID_Parent					= 0xffff;
		ID_Phantom					= 0xffff;
		owner						= 0;
		s_gameid					= 0;
		s_RP						= 0xFE;			// Use supplied coords
        s_flags.set					(M_SPAWN_OBJECT_ACTIVE);
		Memory.mem_copy				(s_name,caSection,((u32)strlen(caSection) + 1)*sizeof(char));
		ZeroMemory					(s_name_replace,sizeof(string64));
        o_Angle.set					(0.f,0.f,0.f);
        o_Position.set				(0.f,0.f,0.f);
		m_bALifeControl				= false;
		m_wVersion					= 0;
	}
	
	virtual							~CSE_Abstract()
	{
	}
	
	virtual void					OnEvent			(NET_Packet &tNetPacket, u16 type, u32 time, u32 sender ){};
	virtual void					Init			(LPCSTR	caSection){};
	void							Spawn_Write		(NET_Packet &tNetPacket, BOOL bLocal);
	BOOL							Spawn_Read		(NET_Packet &tNetPacket);
	// editor integration
#ifdef _EDITOR
    virtual void					FillProp		(LPCSTR pref, PropItemVec &items);
#endif
};

class CSE_Shape
{
public:
	enum{
    	cfSphere=0,
        cfBox
    };
	union shape_data
	{
		Fsphere		sphere;
		Fmatrix		box;
	};
	struct shape_def
	{
		u8			type;
		shape_data	data;
	};
    DEFINE_VECTOR					(shape_def,ShapeVec,ShapeIt);
	ShapeVec						shapes;
public:
	void							cform_read			(NET_Packet& P);
	void							cform_write			(NET_Packet& P);
};

class CSE_Visual
{
	string64						visual_name;
public:
#ifdef _EDITOR
	AnsiString						play_animation;
	IVisual*						visual;
    void __fastcall					OnChangeVisual		(PropValue* sender);
    void 							PlayAnimation		(LPCSTR name);
#endif
public:
									CSE_Visual		(LPCSTR name=0)
    {
    	strcpy						(visual_name,name?name:"");
#ifdef _EDITOR
		play_animation				= "$editor";
		visual						= 0;
        OnChangeVisual				(0);
#endif
    }
	void							visual_read			(NET_Packet& P);
	void							visual_write		(NET_Packet& P);

    void							set_visual			(LPCSTR name);
	LPCSTR							get_visual			() {return visual_name;};
    
#ifdef _EDITOR
    void 							FillProp			(LPCSTR pref, PropItemVec& values);
#endif
};

SERVER_ENTITY_DECLARE_BEGIN(CSE_Dummy,CSE_Abstract)
	enum SStyle{
		esAnimated					=	1<<0,	
		esModel						=	1<<1, 
		esParticles					=	1<<2, 
		esSound						=	1<<3,
		esRelativePosition			=	1<<4
	};
	u8								s_style;
	char*							s_Animation;
	char*							s_Model;
	char*							s_Particles;
	char*							s_Sound;
									CSE_Dummy		(LPCSTR caSection);
    virtual							~CSE_Dummy		();
SERVER_ENTITY_DECLARE_END

SERVER_ENTITY_DECLARE_BEGIN(CSE_Spectator,CSE_Abstract)
									CSE_Spectator	(LPCSTR caSection) : CSE_Abstract(caSection)
	{
	};
	
	virtual u8						g_team() {return 0;};
SERVER_ENTITY_DECLARE_END

SERVER_ENTITY_DECLARE_BEGIN(CSE_Target,CSE_Abstract)
									CSE_Target		(LPCSTR caSection) : CSE_Abstract(caSection)
	{
	};
SERVER_ENTITY_DECLARE_END

SERVER_ENTITY_DECLARE_BEGIN(CSE_TargetAssault,CSE_Target)
									CSE_TargetAssault(LPCSTR caSection) : CSE_Target(caSection)
	{
	};
SERVER_ENTITY_DECLARE_END

SERVER_ENTITY_DECLARE_BEGIN(CSE_Target_CS_Base,CSE_Target)
	float							radius;
	u8								s_team;
	virtual u8						g_team() {return s_team;};
									CSE_Target_CS_Base(LPCSTR caSection);
SERVER_ENTITY_DECLARE_END

SERVER_ENTITY_DECLARE_BEGIN(CSE_Target_CS_Cask,CSE_Target)
	string64						s_Model;
									CSE_Target_CS_Cask(LPCSTR caSection);
SERVER_ENTITY_DECLARE_END

SERVER_ENTITY_DECLARE_BEGIN(CSE_Target_CS,CSE_Target)
	string64						s_Model;
									CSE_Target_CS	(LPCSTR caSection);
SERVER_ENTITY_DECLARE_END

SERVER_ENTITY_DECLARE_BEGIN(CSE_Temporary,CSE_Abstract)
									CSE_Temporary(LPCSTR caSection);
SERVER_ENTITY_DECLARE_END

SERVER_ENTITY_DECLARE_BEGIN2(CSE_Event,CSE_Shape,CSE_Abstract)
	struct tAction
	{
		u8		type;
		u16		count;
		u64		cls;
		LPSTR	event;
	};
	xr_vector<tAction>				Actions;

	void							Actions_clear		()
	{
		for (u32 a=0; a<Actions.size(); a++)
			xr_free					(Actions[a].event);
		Actions.clear				();
	}
							
									CSE_Event	(LPCSTR caSection) : CSE_Shape(), CSE_Abstract(caSection)
	{
	};
SERVER_ENTITY_DECLARE_END

extern CSE_Abstract		*F_entity_Create	(LPCSTR caSection);

#endif