////////////////////////////////////////////////////////////////////////////
//	Module 		: xrServer_Objects_ALife.h
//	Created 	: 19.09.2002
//  Modified 	: 04.06.2003
//	Author		: Oles Shishkovtsov, Alexander Maksimchuk, Victor Reutskiy and Dmitriy Iassenev
//	Description : Server objects monsters for ALife simulator
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#pragma hdrstop

#ifdef AI_COMPILER
	#include "net_utils.h"
	#include "xrServer_Objects_ALife_Items.h"
#endif

#include "xrServer_Objects_ALife_Monsters.h"
#include "ai_alife_templates.h"
#include "xrServer_Objects_ALife_Items.h"
#include "ai_alife_predicates.h"

#ifndef _EDITOR
#ifndef AI_COMPILER
	#include "ai_space.h"
	#include "ai_primary_funcs.h"
#endif
#endif

////////////////////////////////////////////////////////////////////////////
// CSE_ALifeTraderAbstract
////////////////////////////////////////////////////////////////////////////
CSE_ALifeTraderAbstract::CSE_ALifeTraderAbstract(LPCSTR caSection) : CSE_Abstract(caSection)
{
	m_fCumulativeItemMass		= 0.f;
	m_iCumulativeItemVolume		= 0;
	m_dwMoney					= 0;
	if (pSettings->line_exist(caSection, "money"))
		m_dwMoney 				= pSettings->r_u32(caSection, "money");
	m_tRank						= EStalkerRank(pSettings->r_u32(caSection, "rank"));
	m_fMaxItemMass				= pSettings->r_float(caSection, "max_item_mass");
	m_tpEvents.clear			();
}

CSE_ALifeTraderAbstract::~CSE_ALifeTraderAbstract()
{
	delete_data					(m_tpEvents);
}

void CSE_ALifeTraderAbstract::STATE_Write	(NET_Packet &tNetPacket)
{
	save_data					(m_tpEvents,tNetPacket);
}

void CSE_ALifeTraderAbstract::STATE_Read	(NET_Packet &tNetPacket, u16 size)
{
	if (m_wVersion > 19) {
		load_data				(m_tpEvents,tNetPacket);
		TASK_VECTOR				l_tpTaskIDs;
		if (m_wVersion < 36)
			load_data			(l_tpTaskIDs,tNetPacket);
	}
}

void CSE_ALifeTraderAbstract::UPDATE_Write	(NET_Packet &tNetPacket)
{
	tNetPacket.w_float			(m_fCumulativeItemMass);
//	tNetPacket.w_float			(m_iCumulativeItemVolume);
	tNetPacket.w_u32			(m_dwMoney);
	tNetPacket.w_u32			(m_tRank);
};

void CSE_ALifeTraderAbstract::UPDATE_Read	(NET_Packet &tNetPacket)
{
	tNetPacket.r_float			(m_fCumulativeItemMass);
//	tNetPacket.r_float			(m_iCumulativeItemVolume);
	tNetPacket.r_u32			(m_dwMoney);
	u32							dwDummy;
	tNetPacket.r_u32			(dwDummy);
	m_tRank						= EStalkerRank(m_tRank);
};

#ifdef _EDITOR
void CSE_ALifeTraderAbstract::FillProp	(LPCSTR pref, PropItemVec& items)
{
	PHelper.CreateU32			(items, FHelper.PrepareKey(pref,s_name,"Money"), 	&m_dwMoney,	0, u32(-1));
}
#endif

////////////////////////////////////////////////////////////////////////////
// CSE_ALifeTrader
////////////////////////////////////////////////////////////////////////////

CSE_ALifeTrader::CSE_ALifeTrader			(LPCSTR caSection) : CSE_ALifeDynamicObjectVisual(caSection), CSE_ALifeTraderAbstract(caSection), CSE_Abstract(caSection)
{
	if (pSettings->section_exist(caSection) && pSettings->line_exist(caSection,"visual"))
		set_visual				(pSettings->r_string(caSection,"visual"));
	
	m_tpOrderedArtefacts.clear	();
	m_tpSupplies.clear			();
	m_tOrgID					= 1;
}

CSE_ALifeTrader::~CSE_ALifeTrader			()
{
	delete_data					(m_tpOrderedArtefacts);
}

void CSE_ALifeTrader::STATE_Write			(NET_Packet &tNetPacket)
{
	inherited1::STATE_Write		(tNetPacket);
	inherited2::STATE_Write		(tNetPacket);
	tNetPacket.w				(&m_tOrgID,sizeof(m_tOrgID));
	{
		tNetPacket.w_u32		(m_tpOrderedArtefacts.size());
		ARTEFACT_TRADER_ORDER_PAIR_IT	I = m_tpOrderedArtefacts.begin();
		ARTEFACT_TRADER_ORDER_PAIR_IT	E = m_tpOrderedArtefacts.end();
		for ( ; I != E; I++) {
			tNetPacket.w_string	((*I).second->m_caSection);
			tNetPacket.w_u32	((*I).second->m_dwTotalCount);
			save_data			((*I).second->m_tpOrders,tNetPacket);
		}
	}
	{
		tNetPacket.w_u32		(m_tpSupplies.size());
		TRADER_SUPPLY_IT		I = m_tpSupplies.begin();
		TRADER_SUPPLY_IT		E = m_tpSupplies.end();
		for ( ; I != E; I++) {
			tNetPacket.w_string	((*I).m_caSections);
			tNetPacket.w_u32	((*I).m_dwCount);
			tNetPacket.w_float	((*I).m_fMinFactor);
			tNetPacket.w_float	((*I).m_fMaxFactor);
		}
	}
}

void CSE_ALifeTrader::STATE_Read			(NET_Packet &tNetPacket, u16 size)
{
	inherited1::STATE_Read		(tNetPacket, size);
	inherited2::STATE_Read		(tNetPacket, size);
	if (m_wVersion > 35)
		tNetPacket.r			(&m_tOrgID,sizeof(m_tOrgID));
		
	if (m_wVersion > 29) {
		m_tpOrderedArtefacts.clear();
		u32							l_dwCount	= tNetPacket.r_u32();
		for (int i=0 ; i<(int)l_dwCount; i++) {
			SArtefactTraderOrder	*l_tpArtefactOrder = xr_new<SArtefactTraderOrder>();
			tNetPacket.r_string	(l_tpArtefactOrder->m_caSection);
			tNetPacket.r_u32	(l_tpArtefactOrder->m_dwTotalCount);
			load_data			(l_tpArtefactOrder->m_tpOrders,tNetPacket);
			m_tpOrderedArtefacts.insert(std::make_pair(l_tpArtefactOrder->m_caSection,l_tpArtefactOrder));
		}
	}
	if (m_wVersion > 30)
		load_data				(m_tpSupplies,tNetPacket,true);
}

void CSE_ALifeTrader::UPDATE_Write			(NET_Packet &tNetPacket)
{
	inherited1::UPDATE_Write	(tNetPacket);
	inherited2::UPDATE_Write	(tNetPacket);
};

void CSE_ALifeTrader::UPDATE_Read			(NET_Packet &tNetPacket)
{
	inherited1::UPDATE_Read		(tNetPacket);
	inherited2::UPDATE_Read		(tNetPacket);
};

#ifdef _EDITOR
#include "ui_main.h"
void CSE_ALifeTrader::OnSuppliesCountChange	(PropValue* sender)
{
    m_tpSupplies.resize			(supplies_count);
	UI.Command					(COMMAND_UPDATE_PROPERTIES);
}

void CSE_ALifeTrader::FillProp				(LPCSTR _pref, PropItemVec& items)
{
	inherited1::FillProp		(_pref,items);
	inherited2::FillProp		(_pref,items);
	PHelper.CreateU32			(items, FHelper.PrepareKey(_pref,s_name,"Organization ID"), 	&m_tOrgID,	0, 255);

	AnsiString					S;
    AnsiString 					pref = FHelper.PrepareKey(_pref,s_name,"ALife\\Supplies");

    supplies_count				= m_tpSupplies.size();
	PropValue					*V = PHelper.CreateS32(items, FHelper.PrepareKey(pref.c_str(),"Count"), 	&supplies_count,	0, 64);
    V->OnChangeEvent			= OnSuppliesCountChange;
    
	TRADER_SUPPLY_IT			B = m_tpSupplies.begin(), I = B;
	TRADER_SUPPLY_IT			E = m_tpSupplies.end();
	for ( ; I != E; I++) {
    	S.sprintf				("Slot #%d",I-B+1).c_str();
		V=PHelper.CreateEntity	(items, FHelper.PrepareKey(pref.c_str(),S.c_str(),"Sections"), 	(*I).m_caSections, sizeof((*I).m_caSections));
        V->Owner()->subitem		= 8;
		PHelper.CreateU32		(items, FHelper.PrepareKey(pref.c_str(),S.c_str(),"Count"), 	&(*I).m_dwCount,	1, 256);
		PHelper.CreateFloat		(items, FHelper.PrepareKey(pref.c_str(),S.c_str(),"Min Factor"),&(*I).m_fMinFactor,0.f, 1.f);
		PHelper.CreateFloat		(items, FHelper.PrepareKey(pref.c_str(),S.c_str(),"Max Factor"),&(*I).m_fMaxFactor,0.f, 1.f);
	}
}
#endif

////////////////////////////////////////////////////////////////////////////
// CSE_ALifeAnomalousZone
////////////////////////////////////////////////////////////////////////////
CSE_ALifeAnomalousZone::CSE_ALifeAnomalousZone(LPCSTR caSection) : CSE_ALifeSchedulable(caSection), CSE_ALifeDynamicObject(caSection), CSE_Abstract(caSection)
{
	m_maxPower					= 100.f;
	m_attn						= 1.f;
	m_period					= 1000;
	m_fRadius					= 30.f;
	m_fBirthProbability			= pSettings->r_float(caSection,"BirthProbability");

	LPCSTR						l_caParameters = pSettings->r_string(caSection,"artefacts");
	m_wItemCount				= (u16)_GetItemCount(l_caParameters);
	R_ASSERT2					(!(m_wItemCount & 1),"Invalid number of parameters in string 'artefacts' in the 'system.ltx'!");
	m_wItemCount				>>= 1;

	m_faWeights					= (float*)xr_malloc(m_wItemCount*sizeof(float));
	m_cppArtefactSections		= (string64*)xr_malloc(m_wItemCount*sizeof(string64));
	string512					l_caBuffer;
	for (u16 i=0; i<m_wItemCount; i++) {
		strcpy					(m_cppArtefactSections[i],_GetItem(l_caParameters,i << 1,l_caBuffer));
		m_faWeights[i]			= (float)atof(_GetItem(l_caParameters,(i << 1) | 1,l_caBuffer));
	}
	m_wArtefactSpawnCount		= 32;
	m_tAnomalyType				= eAnomalousZoneTypeGravi;
	m_fStartPower = m_maxPower	= 0.f;

	if (pSettings->line_exist(caSection,"hit_type"))
		m_tHitType				= g_tfString2HitType(pSettings->r_string(caSection,"hit_type"));
	else
		m_tHitType				= eHitTypeMax;
}

CSE_ALifeAnomalousZone::~CSE_ALifeAnomalousZone()
{
	xr_free						(m_faWeights);
	xr_free						(m_cppArtefactSections);
}

void CSE_ALifeAnomalousZone::STATE_Read		(NET_Packet	&tNetPacket, u16 size)
{
	// CForm
	if (m_wVersion >= 15)
		inherited1::STATE_Read	(tNetPacket,size);

	cform_read					(tNetPacket);

	tNetPacket.r_float			(m_maxPower);
	tNetPacket.r_float			(m_attn);
	tNetPacket.r_u32			(m_period);

	if (m_wVersion > 21) {
		tNetPacket.r_float		(m_fRadius);
		tNetPacket.r_float		(m_fBirthProbability);
		u16						l_wItemCount;
		tNetPacket.r_u16		(l_wItemCount);
		float					*l_faWeights			= (float*)xr_malloc(l_wItemCount*sizeof(float));
		string64				*l_cppArtefactSections	= (string64*)xr_malloc(l_wItemCount*sizeof(string64));

		for (u16 i=0; i<l_wItemCount; i++) {
			tNetPacket.r_string	(l_cppArtefactSections[i]);
			if (m_wVersion > 26)
				tNetPacket.r_float	(l_faWeights[i]);
			else {
				u32					l_dwValue;
				tNetPacket.r_u32	(l_dwValue);
				l_faWeights[i]		= float(l_dwValue);
			}
		}

		for ( i=0; i<l_wItemCount; i++)
			for (u16 j=0; j<m_wItemCount; j++)
				if (!strstr(l_cppArtefactSections[i],m_cppArtefactSections[j])) {
					m_faWeights[j] = l_faWeights[i];
					break;
				}

				xr_free					(l_faWeights);
				xr_free					(l_cppArtefactSections);
	}
	if (m_wVersion > 25) {
		tNetPacket.r_u16		(m_wArtefactSpawnCount);
		tNetPacket.r_u32		(m_dwStartIndex);
	}
	if (m_wVersion > 27) {
		u32						l_dwDummy;
		tNetPacket.r_u32		(l_dwDummy);
		m_tAnomalyType			= EAnomalousZoneType(l_dwDummy);
	}
	if (m_wVersion > 38)
		tNetPacket.r_float		(m_fStartPower);
}

void CSE_ALifeAnomalousZone::STATE_Write	(NET_Packet	&tNetPacket)
{
	inherited1::STATE_Write		(tNetPacket);
	// CForm
	cform_write					(tNetPacket);

	tNetPacket.w_float			(m_maxPower);
	tNetPacket.w_float			(m_attn);
	tNetPacket.w_u32			(m_period);
	tNetPacket.w_float			(m_fRadius);
	tNetPacket.w_float			(m_fBirthProbability);
	tNetPacket.w_u16			(m_wItemCount);
	for (u16 i=0; i<m_wItemCount; i++) {
		tNetPacket.w_string		(m_cppArtefactSections[i]);
		tNetPacket.w_float		(m_faWeights[i]);
	}
	tNetPacket.w_u16			(m_wArtefactSpawnCount);
	tNetPacket.w_u32			(m_dwStartIndex);
	tNetPacket.w_u32			(m_tAnomalyType);
	tNetPacket.w_float			(m_fStartPower);
}

void CSE_ALifeAnomalousZone::UPDATE_Read	(NET_Packet	&tNetPacket)
{
	inherited1::UPDATE_Read		(tNetPacket);
}

void CSE_ALifeAnomalousZone::UPDATE_Write	(NET_Packet	&tNetPacket)
{
	inherited1::UPDATE_Write	(tNetPacket);
}

#ifdef _EDITOR
xr_token TokenAnomalyType[]={
	{ "Gravi",			eAnomalousZoneTypeGravi			},
	{ "Fog",			eAnomalousZoneTypeFog			},
	{ "Radioactive",	eAnomalousZoneTypeRadio			},
	{ "Plant",			eAnomalousZoneTypePlant			},
	{ "Gelatine",		eAnomalousZoneTypeGelatine		},
	{ "Fluff",			eAnomalousZoneTypeFluff			},
	{ "Rusty Hair",		eAnomalousZoneTypeRustyHair		},
	{ "RustyWhistlers",	eAnomalousZoneTypeRustyWhistlers},
	{ 0,				0}
};

void CSE_ALifeAnomalousZone::FillProp		(LPCSTR pref, PropItemVec& items)
{
	inherited1::FillProp		(pref,items);
	PHelper.CreateToken			(items,FHelper.PrepareKey(pref,s_name,"Type"),								&m_tAnomalyType,	TokenAnomalyType, 1);
	PHelper.CreateFloat			(items,FHelper.PrepareKey(pref,s_name,"Power"),								&m_maxPower,0.f,1000.f);
	PHelper.CreateFloat			(items,FHelper.PrepareKey(pref,s_name,"Attenuation"),						&m_attn,0.f,100.f);
	PHelper.CreateU32			(items,FHelper.PrepareKey(pref,s_name,"Period"),							&m_period,20,10000);
	PHelper.CreateFloat			(items,FHelper.PrepareKey(pref,s_name,"Radius"),							&m_fRadius,0.f,100.f);
	for (u16 i=0; i<m_wItemCount; i++)
		PHelper.CreateFloat		(items,FHelper.PrepareKey(pref,s_name,"ALife\\Artefact Weights",			m_cppArtefactSections[i]), m_faWeights + i,0.f,1.f);
	PHelper.CreateFloat			(items,FHelper.PrepareKey(pref,s_name,"ALife\\Artefact birth probability"),	&m_fBirthProbability,0.f,1.f);
	PHelper.CreateU16			(items,FHelper.PrepareKey(pref,s_name,"ALife\\Artefact spawn places count"),&m_wArtefactSpawnCount,32,256);
}
#endif

////////////////////////////////////////////////////////////////////////////
// CSE_ALifeCreatureAbstract
////////////////////////////////////////////////////////////////////////////
CSE_ALifeCreatureAbstract::CSE_ALifeCreatureAbstract(LPCSTR caSection)	: CSE_ALifeDynamicObjectVisual(caSection), CSE_Abstract(caSection)
{
	s_team = s_squad = s_group	= 0;
	fHealth						= 100;
	m_bDeathIsProcessed			= false;
	m_fAccuracy					= 25.f;
	m_fIntelligence				= 25.f;
	m_fMorale					= 100.f;
}

void CSE_ALifeCreatureAbstract::STATE_Write	(NET_Packet &tNetPacket)
{
	inherited::STATE_Write		(tNetPacket);
	tNetPacket.w_u8				(s_team	);
	tNetPacket.w_u8				(s_squad);
	tNetPacket.w_u8				(s_group);
	tNetPacket.w_float			(fHealth);
}

void CSE_ALifeCreatureAbstract::STATE_Read	(NET_Packet &tNetPacket, u16 size)
{
	inherited::STATE_Read		(tNetPacket, size);
	tNetPacket.r_u8				(s_team	);
	tNetPacket.r_u8				(s_squad);
	tNetPacket.r_u8				(s_group);
	if (m_wVersion > 18)
		tNetPacket.r_float		(fHealth);
	if (m_wVersion < 32)
		visual_read				(tNetPacket);
	o_model						= o_torso.yaw;
}

void CSE_ALifeCreatureAbstract::UPDATE_Write(NET_Packet &tNetPacket)
{
	inherited::UPDATE_Write		(tNetPacket);
	
	tNetPacket.w_float_q16		(fHealth,	-1000,1000);
	
	tNetPacket.w_u32			(timestamp		);
	tNetPacket.w_u8				(flags			);
	tNetPacket.w_vec3			(o_Position		);
	tNetPacket.w_angle8			(o_model		);
	tNetPacket.w_angle8			(o_torso.yaw	);
	tNetPacket.w_angle8			(o_torso.pitch	);
};

void CSE_ALifeCreatureAbstract::UPDATE_Read	(NET_Packet &tNetPacket)
{
	inherited::UPDATE_Read		(tNetPacket);
	
	tNetPacket.r_float_q16		(fHealth,	-1000,1000);
	
	tNetPacket.r_u32			(timestamp		);
	tNetPacket.r_u8				(flags			);
	tNetPacket.r_vec3			(o_Position		);
	tNetPacket.r_angle8			(o_model		);
	tNetPacket.r_angle8			(o_torso.yaw	);
	tNetPacket.r_angle8			(o_torso.pitch	);
};

u8 CSE_ALifeCreatureAbstract::g_team		()
{
	return s_team;
}

u8 CSE_ALifeCreatureAbstract::g_squad		()
{
	return s_squad;
}

u8 CSE_ALifeCreatureAbstract::g_group		()
{
	return s_group;
}

#ifdef _EDITOR
void CSE_ALifeCreatureAbstract::FillProp	(LPCSTR pref, PropItemVec& items)
{
  	inherited::FillProp			(pref,items);
    PHelper.CreateU8			(items,FHelper.PrepareKey(pref,s_name, "Team"),		&s_team, 	0,64,1);
    PHelper.CreateU8			(items,FHelper.PrepareKey(pref,s_name, "Squad"),	&s_squad, 	0,64,1);
    PHelper.CreateU8			(items,FHelper.PrepareKey(pref,s_name, "Group"),	&s_group, 	0,64,1);
   	PHelper.CreateFloat			(items,FHelper.PrepareKey(pref,s_name,"Personal",	"Health" 				),&fHealth,							0,200,5);
}
#endif

////////////////////////////////////////////////////////////////////////////
// CSE_ALifeMonsterAbstract
////////////////////////////////////////////////////////////////////////////
CSE_ALifeMonsterAbstract::CSE_ALifeMonsterAbstract(LPCSTR caSection)	: CSE_ALifeCreatureAbstract(caSection), CSE_ALifeSchedulable(caSection), CSE_Abstract(caSection)
{
	m_tNextGraphID				= m_tGraphID;
	m_tPrevGraphID				= m_tGraphID;
	m_fCurSpeed					= 0.0f;
	m_fDistanceFromPoint		= 0.0f;
	m_fDistanceToPoint			= 0.0f;
	m_tpaTerrain.clear			();

	m_fMaxHealthValue	 		= pSettings->r_float	(caSection,"MaxHealthValue");
	if (pSettings->line_exist(caSection,"hit_power")) {
		m_fHitPower				= pSettings->r_float(caSection,"hit_power");
		m_tHitType				= g_tfString2HitType(pSettings->r_string(caSection,"hit_type"));
	}
	else {
		m_fHitPower				= 0;
		m_tHitType				= eHitTypeMax;
	}

	{
		string64					S;
		m_fpImmunityFactors.resize	(eHitTypeMax);
		svector<float,eHitTypeMax>::iterator	B = m_fpImmunityFactors.begin(), I = B;
		svector<float,eHitTypeMax>::iterator	E = m_fpImmunityFactors.end();
		for ( ; I != E; I++)
			*I					= pSettings->r_float(caSection,strcat(strcpy(S,g_cafHitType2String(EHitType(I - B))),"_immunity"));
	}

	if (pSettings->line_exist(caSection,"retreat_threshold"))
		m_fRetreatThreshold		= pSettings->r_float(caSection,"retreat_threshold");
	else
		m_fRetreatThreshold		= 0.2f;
	m_fEyeRange					= pSettings->r_float(caSection,"eye_range");

	m_fGoingSpeed				= pSettings->r_float(caSection, "going_speed");
	LPCSTR						S = pSettings->r_string(caSection,"terrain");
	u32							N = _GetItemCount(S);
	R_ASSERT					(((N % (LOCATION_TYPE_COUNT + 2)) == 0) && (N));
	STerrainPlace				tTerrainPlace;
	tTerrainPlace.tMask.resize	(LOCATION_TYPE_COUNT);
	string16					I;
	for (u32 i=0; i<N;) {
		for (u32 j=0; j<LOCATION_TYPE_COUNT; j++, i++)
			tTerrainPlace.tMask[j] = _LOCATION_ID(atoi(_GetItem(S,i,I)));
		tTerrainPlace.dwMinTime	= atoi(_GetItem(S,i++,I))*1000;
		tTerrainPlace.dwMaxTime	= atoi(_GetItem(S,i++,I))*1000;
		m_tpaTerrain.push_back	(tTerrainPlace);
	}
	m_tpBestDetector			= this;
}

void CSE_ALifeMonsterAbstract::STATE_Write	(NET_Packet &tNetPacket)
{
	inherited1::STATE_Write		(tNetPacket);
}

void CSE_ALifeMonsterAbstract::STATE_Read	(NET_Packet &tNetPacket, u16 size)
{
	inherited1::STATE_Read		(tNetPacket, size);
}

void CSE_ALifeMonsterAbstract::UPDATE_Write	(NET_Packet &tNetPacket)
{
	inherited1::UPDATE_Write	(tNetPacket);
	tNetPacket.w				(&m_tNextGraphID,			sizeof(m_tNextGraphID));
	tNetPacket.w				(&m_tPrevGraphID,			sizeof(m_tPrevGraphID));
	tNetPacket.w				(&m_fGoingSpeed,			sizeof(m_fGoingSpeed));
	tNetPacket.w				(&m_fCurSpeed,				sizeof(m_fCurSpeed));
	tNetPacket.w				(&m_fDistanceFromPoint,		sizeof(m_fDistanceFromPoint));
	tNetPacket.w				(&m_fDistanceToPoint,		sizeof(m_fDistanceToPoint));
};

void CSE_ALifeMonsterAbstract::UPDATE_Read	(NET_Packet &tNetPacket)
{
	inherited1::UPDATE_Read		(tNetPacket);
	tNetPacket.r				(&m_tNextGraphID,			sizeof(m_tNextGraphID));
	tNetPacket.r				(&m_tPrevGraphID,			sizeof(m_tPrevGraphID));
	tNetPacket.r				(&m_fGoingSpeed,			sizeof(m_fGoingSpeed));
	tNetPacket.r				(&m_fCurSpeed,				sizeof(m_fCurSpeed));
	tNetPacket.r				(&m_fDistanceFromPoint,		sizeof(m_fDistanceFromPoint));
	tNetPacket.r				(&m_fDistanceToPoint,		sizeof(m_fDistanceToPoint));
};

#ifdef _EDITOR
void CSE_ALifeMonsterAbstract::FillProp		(LPCSTR pref, PropItemVec& items)
{
  	inherited1::FillProp		(pref,items);
}
#endif

////////////////////////////////////////////////////////////////////////////
// CSE_ALifeCreatureActor
////////////////////////////////////////////////////////////////////////////
CSE_ALifeCreatureActor::CSE_ALifeCreatureActor	(LPCSTR caSection) : CSE_ALifeCreatureAbstract(caSection), CSE_ALifeTraderAbstract(caSection), CSE_Abstract(caSection)
{
	set_visual					("actors\\Different_stalkers\\stalker_hood_multiplayer.ogf");
}

void CSE_ALifeCreatureActor::STATE_Read		(NET_Packet	&tNetPacket, u16 size)
{
	if (m_wVersion < 21) {
		CSE_ALifeDynamicObject::STATE_Read(tNetPacket,size);
		tNetPacket.r_u8			(s_team	);
		tNetPacket.r_u8			(s_squad);
		tNetPacket.r_u8			(s_group);
		if (m_wVersion > 18)
			tNetPacket.r_float	(fHealth);
		if (m_wVersion >= 3)
			visual_read			(tNetPacket);
	}
	else {
		inherited1::STATE_Read	(tNetPacket,size);
		inherited2::STATE_Read	(tNetPacket,size);
		if (m_wVersion < 32)
			visual_read			(tNetPacket);
	}
};

void CSE_ALifeCreatureActor::STATE_Write	(NET_Packet	&tNetPacket)
{
	inherited1::STATE_Write		(tNetPacket);
	inherited2::STATE_Write		(tNetPacket);
};

void CSE_ALifeCreatureActor::UPDATE_Read	(NET_Packet	&tNetPacket)
{
	inherited1::UPDATE_Read		(tNetPacket);
	inherited2::UPDATE_Read		(tNetPacket);
	tNetPacket.r_u16			(mstate		);
	tNetPacket.r_sdir			(accel		);
	tNetPacket.r_sdir			(velocity	);
	tNetPacket.r_float_q16		(fArmor,	-1000,1000);
	tNetPacket.r_u8				(weapon		);
};
void CSE_ALifeCreatureActor::UPDATE_Write	(NET_Packet	&tNetPacket)
{
	inherited1::UPDATE_Write	(tNetPacket);
	inherited2::UPDATE_Write	(tNetPacket);
	tNetPacket.w_u16			(mstate		);
	tNetPacket.w_sdir			(accel		);
	tNetPacket.w_sdir			(velocity	);
	tNetPacket.w_float_q16		(fArmor,	-1000,1000);
	tNetPacket.w_u8				(weapon		);
}

#ifdef _EDITOR
void CSE_ALifeCreatureActor::FillProp		(LPCSTR pref, PropItemVec& items)
{
  	inherited1::FillProp		(pref,items);
  	inherited2::FillProp		(pref,items);
}
#endif

////////////////////////////////////////////////////////////////////////////
// CSE_ALifeCreatureCrow
////////////////////////////////////////////////////////////////////////////
CSE_ALifeCreatureCrow::CSE_ALifeCreatureCrow(LPCSTR caSection) : CSE_ALifeCreatureAbstract(caSection), CSE_Abstract(caSection)
{
	if (pSettings->section_exist(caSection) && pSettings->line_exist(caSection,"visual"))
		set_visual				(pSettings->r_string(caSection,"visual"));
}

void CSE_ALifeCreatureCrow::STATE_Read		(NET_Packet	&tNetPacket, u16 size)
{
	if (m_wVersion > 20) {
		inherited::STATE_Read	(tNetPacket,size);
		if (m_wVersion < 32)
			visual_read			(tNetPacket);
	}
}

void CSE_ALifeCreatureCrow::STATE_Write		(NET_Packet	&tNetPacket)
{
	inherited::STATE_Write		(tNetPacket);
}

void CSE_ALifeCreatureCrow::UPDATE_Read		(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Read		(tNetPacket);
}

void CSE_ALifeCreatureCrow::UPDATE_Write		(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Write		(tNetPacket);
}

#ifdef _EDITOR
void CSE_ALifeCreatureCrow::FillProp			(LPCSTR pref, PropItemVec& values)
{
  	inherited::FillProp			(pref,values);
}
#endif

////////////////////////////////////////////////////////////////////////////
// CSE_ALifeMonsterRat
////////////////////////////////////////////////////////////////////////////
CSE_ALifeMonsterRat::CSE_ALifeMonsterRat	(LPCSTR caSection) : CSE_ALifeMonsterAbstract(caSection), CSE_ALifeInventoryItem(caSection), CSE_Abstract(caSection)
{
	set_visual					("monsters\\rat\\rat_1");
	// personal charactersitics
	fEyeFov						= 120;
	fEyeRange					= 10;
	fHealth						= 5;
	fMinSpeed					= .5;
	fMaxSpeed					= 1.5;
	fAttackSpeed				= 4.0;
	fMaxPursuitRadius			= 100;
	fMaxHomeRadius				= 10;
	// morale
	fMoraleSuccessAttackQuant	= 20;
	fMoraleDeathQuant			= -10;
	fMoraleFearQuant			= -20;
	fMoraleRestoreQuant			=  10;
	u16MoraleRestoreTimeInterval= 3000;
	fMoraleMinValue				= 0;
	fMoraleMaxValue				= 100;
	fMoraleNormalValue			= 66;
	// attack
	fHitPower					= 10.0;
	u16HitInterval				= 1500;
	fAttackDistance				= 0.7f;
	fAttackAngle				= 45;
	fAttackSuccessProbability	= 0.5f;
}

void CSE_ALifeMonsterRat::STATE_Read		(NET_Packet	&tNetPacket, u16 size)
{
	inherited1::STATE_Read		(tNetPacket,size);
	tNetPacket.r_float			(fEyeFov);
	tNetPacket.r_float			(fEyeRange);
	if (m_wVersion <= 5)
		tNetPacket.r_float		(fHealth);
	tNetPacket.r_float			(fMinSpeed);
	tNetPacket.r_float			(fMaxSpeed);
	tNetPacket.r_float			(fAttackSpeed);
	tNetPacket.r_float			(fMaxPursuitRadius);
	tNetPacket.r_float			(fMaxHomeRadius);
	// morale
	tNetPacket.r_float			(fMoraleSuccessAttackQuant);
	tNetPacket.r_float			(fMoraleDeathQuant);
	tNetPacket.r_float			(fMoraleFearQuant);
	tNetPacket.r_float			(fMoraleRestoreQuant);
	tNetPacket.r_u16			(u16MoraleRestoreTimeInterval);
	tNetPacket.r_float			(fMoraleMinValue);
	tNetPacket.r_float			(fMoraleMaxValue);
	tNetPacket.r_float			(fMoraleNormalValue);
	// attack
	tNetPacket.r_float			(fHitPower);
	tNetPacket.r_u16			(u16HitInterval);
	tNetPacket.r_float			(fAttackDistance);
	tNetPacket.r_float			(fAttackAngle);
	tNetPacket.r_float			(fAttackSuccessProbability);
	inherited2::STATE_Read		(tNetPacket,size);
}

void CSE_ALifeMonsterRat::STATE_Write		(NET_Packet	&tNetPacket)
{
	inherited1::STATE_Write		(tNetPacket);
	tNetPacket.w_float			(fEyeFov);
	tNetPacket.w_float			(fEyeRange);
	tNetPacket.w_float			(fMinSpeed);
	tNetPacket.w_float			(fMaxSpeed);
	tNetPacket.w_float			(fAttackSpeed);
	tNetPacket.w_float			(fMaxPursuitRadius);
	tNetPacket.w_float			(fMaxHomeRadius);
	// morale
	tNetPacket.w_float			(fMoraleSuccessAttackQuant);
	tNetPacket.w_float			(fMoraleDeathQuant);
	tNetPacket.w_float			(fMoraleFearQuant);
	tNetPacket.w_float			(fMoraleRestoreQuant);
	tNetPacket.w_u16			(u16MoraleRestoreTimeInterval);
	tNetPacket.w_float			(fMoraleMinValue);
	tNetPacket.w_float			(fMoraleMaxValue);
	tNetPacket.w_float			(fMoraleNormalValue);
	// attack
	tNetPacket.w_float			(fHitPower);
	tNetPacket.w_u16			(u16HitInterval);
	tNetPacket.w_float			(fAttackDistance);
	tNetPacket.w_float			(fAttackAngle);
	tNetPacket.w_float			(fAttackSuccessProbability);
	inherited2::STATE_Write		(tNetPacket);
}

void CSE_ALifeMonsterRat::UPDATE_Read		(NET_Packet	&tNetPacket)
{
	inherited1::UPDATE_Read		(tNetPacket);
	inherited2::UPDATE_Read		(tNetPacket);
}

void CSE_ALifeMonsterRat::UPDATE_Write		(NET_Packet	&tNetPacket)
{
	inherited1::UPDATE_Write	(tNetPacket);
	inherited2::UPDATE_Write	(tNetPacket);
}

#ifdef _EDITOR
void CSE_ALifeMonsterRat::FillProp			(LPCSTR pref, PropItemVec& items)
{
	inherited1::FillProp		(pref, items);
	inherited2::FillProp		(pref, items);
	// personal characteristics
   	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Personal",	"Field of view" 		),&fEyeFov,							0,170,10);
   	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Personal",	"Eye range" 			),&fEyeRange,						0,300,10);
   	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Personal",	"Minimum speed" 		),&fMinSpeed,						0,10,0.1);
   	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Personal",	"Maximum speed" 		),&fMaxSpeed,						0,10,0.1);
   	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Personal",	"Attack speed" 			),&fAttackSpeed,					0,10,0.1);
   	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Personal",	"Pursuit distance" 		),&fMaxPursuitRadius,				0,300,10);
   	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Personal",	"Home distance" 		),&fMaxHomeRadius,					0,300,10);
	// morale																			
	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Morale",		"Success attack quant" 	),&fMoraleSuccessAttackQuant,		-100,100,5);
	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Morale",		"Death quant" 			),&fMoraleDeathQuant,				-100,100,5);
	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Morale",		"Fear quant" 			),&fMoraleFearQuant,				-100,100,5);
	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Morale",		"Restore quant" 		),&fMoraleRestoreQuant,				-100,100,5);
	PHelper.CreateU16  			(items, FHelper.PrepareKey(pref,s_name,"Morale",		"Restore time interval" ),&u16MoraleRestoreTimeInterval,	0,65535,500);
	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Morale",		"Minimum value" 		),&fMoraleMinValue,					-100,100,5);
	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Morale",		"Maximum value" 		),&fMoraleMaxValue,					-100,100,5);
	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Morale",		"Normal value" 			),&fMoraleNormalValue,				-100,100,5);
	// attack																			 	
	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Attack",		"Hit power" 			),&fHitPower,						0,200,5);
	PHelper.CreateU16  			(items, FHelper.PrepareKey(pref,s_name,"Attack",		"Hit interval" 			),&u16HitInterval,					0,65535,500);
	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Attack",		"Distance" 				),&fAttackDistance,					0,300,10);
	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Attack",		"Maximum angle" 		),&fAttackAngle,					0,180,10);
	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Attack",		"Success probability" 	),&fAttackSuccessProbability,		0,100,1);
}	
#endif

bool CSE_ALifeMonsterRat::bfUseful		()
{
	return						(!dynamic_cast<CSE_ALifeGroupAbstract*>(this) && (fHealth <= EPS_L));
}


////////////////////////////////////////////////////////////////////////////
// CSE_ALifeMonsterZombie
////////////////////////////////////////////////////////////////////////////
CSE_ALifeMonsterZombie::CSE_ALifeMonsterZombie	(LPCSTR caSection) : CSE_ALifeMonsterAbstract(caSection), CSE_Abstract(caSection)
{
	set_visual					("monsters\\zombie\\zombie_1");
	// personal charactersitics
	fEyeFov						= 120;
	fEyeRange					= 30;
	fHealth						= 200;
	fMinSpeed					= 1.5;
	fMaxSpeed					= 1.75;
	fAttackSpeed				= 2.0;
	fMaxPursuitRadius			= 100;
	fMaxHomeRadius				= 30;
	// attack
	fHitPower					= 20.0;
	u16HitInterval				= 1000;
	fAttackDistance				= 1.0f;
	fAttackAngle				= 15;
}

void CSE_ALifeMonsterZombie::STATE_Read		(NET_Packet	&tNetPacket, u16 size)
{
	// inherited properties
	inherited::STATE_Read		(tNetPacket,size);
	// personal characteristics
	tNetPacket.r_float			(fEyeFov);
	tNetPacket.r_float			(fEyeRange);
	if (m_wVersion <= 5)
		tNetPacket.r_float		(fHealth);
	tNetPacket.r_float			(fMinSpeed);
	tNetPacket.r_float			(fMaxSpeed);
	tNetPacket.r_float			(fAttackSpeed);
	tNetPacket.r_float			(fMaxPursuitRadius);
	tNetPacket.r_float			(fMaxHomeRadius);
	// attack
	tNetPacket.r_float			(fHitPower);
	tNetPacket.r_u16			(u16HitInterval);
	tNetPacket.r_float			(fAttackDistance);
	tNetPacket.r_float			(fAttackAngle);
}

void CSE_ALifeMonsterZombie::STATE_Write	(NET_Packet	&tNetPacket)
{
	// inherited properties
	inherited::STATE_Write		(tNetPacket);
	// personal characteristics
	tNetPacket.w_float			(fEyeFov);
	tNetPacket.w_float			(fEyeRange);
	tNetPacket.w_float			(fMinSpeed);
	tNetPacket.w_float			(fMaxSpeed);
	tNetPacket.w_float			(fAttackSpeed);
	tNetPacket.w_float			(fMaxPursuitRadius);
	tNetPacket.w_float			(fMaxHomeRadius);
	// attack
	tNetPacket.w_float			(fHitPower);
	tNetPacket.w_u16			(u16HitInterval);
	tNetPacket.w_float			(fAttackDistance);
	tNetPacket.w_float			(fAttackAngle);
}

void CSE_ALifeMonsterZombie::UPDATE_Read	(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Read		(tNetPacket);
}

void CSE_ALifeMonsterZombie::UPDATE_Write	(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Write		(tNetPacket);
}

#ifdef _EDITOR
void CSE_ALifeMonsterZombie::FillProp		(LPCSTR pref, PropItemVec& items)
{
   	inherited::FillProp			(pref, items);
	// personal characteristics
	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Personal",	"Field of view" 		),&fEyeFov,							0,170,10);
   	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Personal",	"Eye range" 			),&fEyeRange,						0,300,10);
   	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Personal",	"Minimum speed" 		),&fMinSpeed,						0,10,0.1);
   	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Personal",	"Maximum speed" 		),&fMaxSpeed,						0,10,0.1);
   	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Personal",	"Attack speed" 			),&fAttackSpeed,					0,10,0.1);
   	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Personal",	"Pursuit distance" 		),&fMaxPursuitRadius,				0,300,10);
   	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Personal",	"Home distance" 		),&fMaxHomeRadius,					0,300,10);
	// attack																			 	
	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Attack",		"Hit power" 			),&fHitPower,						0,200,5);
	PHelper.CreateU16  			(items, FHelper.PrepareKey(pref,s_name,"Attack",		"Hit interval" 			),&u16HitInterval,					0,65535,500);
	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Attack",		"Distance" 				),&fAttackDistance,					0,300,10);
	PHelper.CreateFloat			(items, FHelper.PrepareKey(pref,s_name,"Attack",		"Maximum angle" 		),&fAttackAngle,					0,100,1);
}
#endif

//////////////////////////////////////////////////////////////////////////
// CSE_ALifeMonsterBiting
//////////////////////////////////////////////////////////////////////////
CSE_ALifeMonsterBiting::CSE_ALifeMonsterBiting	(LPCSTR caSection) : CSE_ALifeMonsterAbstract(caSection), CSE_Abstract(caSection)
{
    set_visual					(pSettings->r_string(caSection,"visual"));
}

void CSE_ALifeMonsterBiting::STATE_Read		(NET_Packet	&tNetPacket, u16 size)
{
	inherited::STATE_Read		(tNetPacket,size);
}

void CSE_ALifeMonsterBiting::STATE_Write	(NET_Packet	&tNetPacket)
{
	inherited::STATE_Write		(tNetPacket);
}

void CSE_ALifeMonsterBiting::UPDATE_Read	(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Read		(tNetPacket);
}

void CSE_ALifeMonsterBiting::UPDATE_Write	(NET_Packet	&tNetPacket)
{
	inherited::UPDATE_Write		(tNetPacket);
}

#ifdef _EDITOR
void CSE_ALifeMonsterBiting::FillProp	(LPCSTR pref, PropItemVec& values)
{
	inherited::FillProp			(pref,values);
}
#endif

//////////////////////////////////////////////////////////////////////////
// CSE_ALifeHumanAbstract
//////////////////////////////////////////////////////////////////////////
CSE_ALifeHumanAbstract::CSE_ALifeHumanAbstract(LPCSTR caSection) : CSE_ALifeTraderAbstract(caSection), CSE_ALifeMonsterAbstract(caSection), CSE_Abstract(caSection)
{
	m_tpPath.clear				();
	m_baVisitedVertices.clear	();
	m_dwCurTaskID				= _TASK_ID(-1);
	m_tTaskState				= eTaskStateChooseTask;
	m_dwCurTaskLocation			= u32(-1);
	m_fSearchSpeed				= pSettings->r_float(caSection, "search_speed");
	m_dwCurNode					= u32(-1);
	strcpy						(m_caKnownCustomers,"m_trader0000");
	m_tpKnownCustomers.clear	();
#ifndef _EDITOR
#ifndef AI_COMPILER
	m_tpALife					= 0;
#endif
#endif
	m_cpEquipmentPreferences.resize(5);
	m_cpMainWeaponPreferences.resize(4);
#ifndef _EDITOR
#ifndef AI_COMPILER
	m_cpEquipmentPreferences.resize(iFloor(getAI().m_pfEquipmentType->ffGetMaxResultValue() + .5f));
	m_cpMainWeaponPreferences.resize(iFloor(getAI().m_pfMainWeaponType->ffGetMaxResultValue() + .5f));
	R_ASSERT2					((iFloor(getAI().m_pfEquipmentType->ffGetMaxResultValue() + .5f) == 5) && (iFloor(getAI().m_pfMainWeaponType->ffGetMaxResultValue() + .5f) == 4),"Recompile Level Editor and xrAI and rebuild file \"game.spawn\"!");
#endif
#endif
	{
		for (int i=0, n=m_cpEquipmentPreferences.size(); i<n; i++)
			m_cpEquipmentPreferences[i] = u8(::Random.randI(3));
	}
	{
		for (int i=0, n=m_cpMainWeaponPreferences.size(); i<n; i++)
			m_cpMainWeaponPreferences[i] = u8(::Random.randI(3));
	}
}

CSE_ALifeHumanAbstract::~CSE_ALifeHumanAbstract()
{
}

void CSE_ALifeHumanAbstract::STATE_Write	(NET_Packet &tNetPacket)
{
	inherited1::STATE_Write		(tNetPacket);
	inherited2::STATE_Write		(tNetPacket);
	save_data					(m_tpPath,tNetPacket);
	save_data					(m_baVisitedVertices,tNetPacket);
	tNetPacket.w_string			(m_caKnownCustomers);
	save_data					(m_tpKnownCustomers,tNetPacket);
	save_data					(m_cpEquipmentPreferences,tNetPacket);
	save_data					(m_cpMainWeaponPreferences,tNetPacket);
}

void CSE_ALifeHumanAbstract::STATE_Read		(NET_Packet &tNetPacket, u16 size)
{
	inherited1::STATE_Read		(tNetPacket, size);
	inherited2::STATE_Read		(tNetPacket, size);
	if (m_wVersion > 19) {
		load_data				(m_tpPath,tNetPacket);
		load_data				(m_baVisitedVertices,tNetPacket);
		if (m_wVersion > 35) {
			tNetPacket.r_string	(m_caKnownCustomers);
			load_data			(m_tpKnownCustomers,tNetPacket);
			if (m_wVersion > 37) {
				load_data		(m_cpEquipmentPreferences,tNetPacket);
				load_data		(m_cpMainWeaponPreferences,tNetPacket);
			}
		}
	}
}

void CSE_ALifeHumanAbstract::UPDATE_Write	(NET_Packet &tNetPacket)
{
	inherited1::UPDATE_Write	(tNetPacket);
	inherited2::UPDATE_Write	(tNetPacket);
	tNetPacket.w				(&m_tTaskState,sizeof(m_tTaskState));
	tNetPacket.w_u32			(m_dwCurTaskLocation);
	tNetPacket.w_u32			(m_dwCurTaskID);
};

void CSE_ALifeHumanAbstract::UPDATE_Read	(NET_Packet &tNetPacket)
{
	inherited1::UPDATE_Read		(tNetPacket);
	inherited2::UPDATE_Read		(tNetPacket);
	tNetPacket.r				(&m_tTaskState,sizeof(m_tTaskState));
	tNetPacket.r_u32			(m_dwCurTaskLocation);
	tNetPacket.r_u32			(m_dwCurTaskID);
};

#ifdef _EDITOR
void CSE_ALifeHumanAbstract::FillProp		(LPCSTR pref, PropItemVec& items)
{
  	inherited1::FillProp		(pref,items);
  	inherited2::FillProp		(pref,items);
	PropValue					*V = PHelper.CreateSceneItem(items, FHelper.PrepareKey(pref,s_name,"ALife\\Known traders"),	m_caKnownCustomers,  sizeof(m_caKnownCustomers), OBJCLASS_SPAWNPOINT, "m_trader_e")	;
	V->Owner()->subitem			= 8;
}
#endif

//////////////////////////////////////////////////////////////////////////
// CSE_ALifeHumanStalker
//////////////////////////////////////////////////////////////////////////
CSE_ALifeHumanStalker::CSE_ALifeHumanStalker(LPCSTR caSection) : CSE_ALifeHumanAbstract(caSection), CSE_Abstract(caSection)
{
	m_dwTotalMoney				= 0;
}

void CSE_ALifeHumanStalker::STATE_Write		(NET_Packet &tNetPacket)
{
	inherited::STATE_Write		(tNetPacket);
}

void CSE_ALifeHumanStalker::STATE_Read		(NET_Packet &tNetPacket, u16 size)
{
	inherited::STATE_Read		(tNetPacket, size);
}

void CSE_ALifeHumanStalker::UPDATE_Write	(NET_Packet &tNetPacket)
{
	inherited::UPDATE_Write		(tNetPacket);
};

void CSE_ALifeHumanStalker::UPDATE_Read		(NET_Packet &tNetPacket)
{
	inherited::UPDATE_Read		(tNetPacket);
};

#ifdef _EDITOR
void CSE_ALifeHumanStalker::FillProp		(LPCSTR pref, PropItemVec& values)
{
	inherited::FillProp			(pref,values);
}
#endif

//////////////////////////////////////////////////////////////////////////
// CSE_ALifeObjectIdol
//////////////////////////////////////////////////////////////////////////
CSE_ALifeObjectIdol::CSE_ALifeObjectIdol	(LPCSTR caSection) : CSE_ALifeHumanAbstract(caSection), CSE_Abstract(caSection)
{
	m_dwAniPlayType				= 0;
	m_caAnimations[0]			= 0;
}

void CSE_ALifeObjectIdol::STATE_Read		(NET_Packet& tNetPacket, u16 size)
{
	inherited::STATE_Read		(tNetPacket,size);
	tNetPacket.r_string			(m_caAnimations);
	tNetPacket.r_u32			(m_dwAniPlayType);
}

void CSE_ALifeObjectIdol::STATE_Write		(NET_Packet& tNetPacket)
{
	inherited::STATE_Write		(tNetPacket);
	tNetPacket.w_string			(m_caAnimations);
	tNetPacket.w_u32			(m_dwAniPlayType);
}

void CSE_ALifeObjectIdol::UPDATE_Read		(NET_Packet& tNetPacket)
{
}

void CSE_ALifeObjectIdol::UPDATE_Write		(NET_Packet& tNetPacket)
{
}

#ifdef _EDITOR
void CSE_ALifeObjectIdol::FillProp			(LPCSTR pref, PropItemVec& items)
{
   	inherited::FillProp			(pref, items);
    PHelper.CreateText			(items, FHelper.PrepareKey(pref,s_name,"Idol", "Animations"),m_caAnimations,sizeof(m_caAnimations));
   	PHelper.CreateU32			(items, FHelper.PrepareKey(pref,s_name,"Idol", "Animation playing type"),&m_dwAniPlayType,0,2,1);
}	
#endif
