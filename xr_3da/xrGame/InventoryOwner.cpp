/////////////////////////////////////////////////////
// ��� ����������, ������� ���������
// InventoryOwner.cpp
//////////////////////////////////////////////////////


#include "stdafx.h"
#include "InventoryOwner.h"
#include "gameobject.h"
#include "entity_alive.h"
#include "pda.h"
#include "actor.h"
#include "trade.h"
#include "inventory.h"
#include "xrserver_objects_alife.h"

//////////////////////////////////////////////////////////////////////////
// CInventoryOwner class 
//////////////////////////////////////////////////////////////////////////
CInventoryOwner::CInventoryOwner			()
{
	Init						();
}

CInventoryOwner::~CInventoryOwner			() 
{
	xr_delete					(m_inventory);
	xr_delete					(m_trade_storage);
	xr_delete					(m_pTrade);
}

void CInventoryOwner::Init					()
{
	m_pTrade = NULL;
	m_inventory					= xr_new<CInventory>();
	m_trade_storage				= xr_new<CInventory>();
}

void CInventoryOwner::Load					(LPCSTR section)
{
}

void CInventoryOwner::reload				(LPCSTR section)
{
	CAttachmentOwner::reload	(section);
}

void CInventoryOwner::reinit				()
{
	CAttachmentOwner::reinit	();
	inventory().m_pOwner		= this;
	m_trade_storage->m_pOwner	= this;

	m_dwMoney					= 0;
	m_tRank						= ALife::eStalkerRankNone;

	m_bTalking					= false;
	m_pTalkPartner				= NULL;
	inventory().Clear			();
}

BOOL CInventoryOwner::net_Spawn		(LPVOID DC)
{
	//�������� ��������� �� ������, InventoryOwner
	CGameObject			*pThis = dynamic_cast<CGameObject*>(this);
	if(!pThis) return FALSE;

	
	if(!pThis->Local())  return TRUE;
    
//	if (ai().get_alife())
//		return			TRUE;

	CSE_Abstract			*E	= (CSE_Abstract*)(DC);
	CSE_ALifeDynamicObject	*dynamic_object = dynamic_cast<CSE_ALifeDynamicObject*>(E);
	VERIFY					(dynamic_object);

	//������� ������� inventory owner-� � ��������� �����
	if (use_bolts()) {
		CSE_Abstract						*D	= F_entity_Create("bolt");
		R_ASSERT							(D);
		CSE_ALifeDynamicObject				*l_tpALifeDynamicObject = dynamic_cast<CSE_ALifeDynamicObject*>(D);
		R_ASSERT							(l_tpALifeDynamicObject);
		l_tpALifeDynamicObject->m_tNodeID	= dynamic_object->m_tNodeID;

		// Fill
		strcpy								(D->s_name,"bolt");
		strcpy								(D->s_name_replace,"");
		D->s_gameid							=	u8(GameID());
		D->s_RP								=	0xff;
		D->ID								=	0xffff;
		D->ID_Parent						=	E->ID;
		D->ID_Phantom						=	0xffff;
		D->o_Position						=	pThis->Position();
		D->s_flags.set						(M_SPAWN_OBJECT_LOCAL);
		D->RespawnTime						=	0;

		// Send
		NET_Packet							P;
		D->Spawn_Write						(P,TRUE);
		Level().Send						(P,net_flags(TRUE));

		// Destroy
		F_entity_Destroy					(D);
	}
	
	if (ai().get_alife())
		return					(TRUE);
	//////////////////////////////////////////////
	//����������� PDA ������� inventory owner
	//////////////////////////////////////////////
	if (true) {
		// Create
		CSE_Abstract			*D = F_entity_Create("device_pda");
		//CSE_Abstract			*D = F_entity_Create("detector_simple");
		R_ASSERT				(D);
		CSE_ALifeDynamicObject	*l_tpALifeDynamicObject = dynamic_cast<CSE_ALifeDynamicObject*>(D);
		R_ASSERT				(l_tpALifeDynamicObject);

		// Fill
		strcpy					(D->s_name,"device_pda");
		strcpy					(D->s_name_replace,"");
		D->s_gameid				=	u8(GameID());
		D->s_RP					=	0xff;
		D->ID					=	0xffff;
		D->ID_Parent			=	E->ID;
		D->ID_Phantom			=	0xffff;
		D->o_Position			=	pThis->Position();
		D->s_flags.set			(M_SPAWN_OBJECT_LOCAL);
		D->RespawnTime			=	0;
		// Send
		NET_Packet				P;
		D->Spawn_Write			(P,TRUE);
		Level().Send			(P,net_flags(TRUE));
		// Destroy
		F_entity_Destroy		(D);
	}

	return TRUE;
}


void CInventoryOwner::UpdateInventoryOwner(u32 deltaT)
{
	inventory().Update();
	if(m_pTrade) m_pTrade->UpdateTrade();

	if(IsTalking())
	{
		//���� ��� ���������� �������� �������� � ����,
		//�� � ��� ������ �����.
		if(!m_pTalkPartner->IsTalking())
		{
			StopTalk();
		}

		//���� �� ������, �� ���� �� ��������
		CEntityAlive* pOurEntityAlive = dynamic_cast<CEntityAlive*>(this);
		R_ASSERT(pOurEntityAlive);
		if(!pOurEntityAlive->g_Alive()) StopTalk();
	}
}


//������� PDA �� ������������ ����� ���������
CPda* CInventoryOwner::GetPDA()
{
//	CEntityAlive* pEntityAlive = dynamic_cast<CEntityAlive*>(this);
	
//	if(!pEntityAlive || !pEntityAlive->g_Alive()) return NULL; 

	/*R_ASSERT2(inventory().m_slots[PDA_SLOT].m_pIItem, 
			"PDA for character does not init yet");*/
	
	return (CPda*)inventory().m_slots[PDA_SLOT].m_pIItem;
}

bool CInventoryOwner::IsActivePDA()
{
	if(GetPDA() && GetPDA()->IsActive())
		return true;
	else
		return false;
}

//����������� ������� ��������� ���������
//who - id PDA �� �������� ������ ���������
void CInventoryOwner::ReceivePdaMessage(u16 who, EPdaMsg msg, int info_index)
{
	if(msg == ePdaMsgInfo)
	{
		//��������� ���� �� ���������� ����������
		GetPDA()->TransferInfoToID(GetPDA()->ID(), info_index);
	}
}

//who - id PDA �������� ���������� ���������
void CInventoryOwner::SendPdaMessage(u16 who, EPdaMsg msg, int info_index)
{
	if(!GetPDA() || !GetPDA()->IsActive()) return;

	GetPDA()->SendMessageID(who, msg, info_index);
}

void CInventoryOwner::InitTrade()
{
	m_pTrade = xr_new<CTrade>(this);
}
CTrade* CInventoryOwner::GetTrade() 
{
	R_ASSERT2(m_pTrade, "trade for object does not init yet");
	return m_pTrade;
}


//��������� �������

//��� ���������� ����������,
//��������� ���� ��������� 
//� ���� �� ���� �������� ��������
bool CInventoryOwner::OfferTalk(CInventoryOwner* talk_partner)
{
	//��������� ��������� � �����������
	CEntityAlive* pOurEntityAlive = dynamic_cast<CEntityAlive*>(this);
	R_ASSERT(pOurEntityAlive);

	CEntityAlive* pPartnerEntityAlive = dynamic_cast<CEntityAlive*>(talk_partner);
	R_ASSERT(pPartnerEntityAlive);
	
	ALife::ERelationType relation = pOurEntityAlive->tfGetRelationType(pPartnerEntityAlive);

	if(relation == ALife::eRelationTypeEnemy) return false;

	if(!pOurEntityAlive->g_Alive() || !pPartnerEntityAlive->g_Alive()) return false;

	StartTalk(talk_partner);

	return true;
}


void CInventoryOwner::StartTalk(CInventoryOwner* talk_partner)
{
	m_bTalking = true;
	m_pTalkPartner = talk_partner;

	//��� �� �������� ��������
	GetTrade()->StartTrade(talk_partner);
}

void CInventoryOwner::StopTalk()
{
	m_pTalkPartner = NULL;
	m_bTalking = false;

	//��������� ��������
	GetTrade()->StopTrade();
}
bool CInventoryOwner::IsTalking()
{
	return m_bTalking;
}

bool CInventoryOwner::AskQuestion(SInfoQuestion& question, INFO_INDEX_LIST& index_list)
{
	R_ASSERT2(GetPDA(), "PDA for character does not init yet");

	bool result_answer = false;

	index_list.clear();

	INFO_INDEX_LIST_it it;
	for(it = question.IndexList.begin();
		question.IndexList.end() != it;
		++it)
	{
		int info_index = *it;
		if(GetPDA()->IsKnowAbout(info_index))
		{
			//��������� ���������� PDA 
			u32 partner_pda_id = m_pTalkPartner->GetPDA()->ID();
			GetPDA()->TransferInfoToID(partner_pda_id, info_index);

			index_list.push_back(info_index);

			result_answer = true;
		}
	}

	return result_answer;
}

LPCSTR CInventoryOwner::GetGameName()
{
	CObject* pObject = dynamic_cast<CObject*>(this);
	
	
	if(pObject)
		return *pObject->cName();
	else
		return NULL;
}
LPCSTR CInventoryOwner::GetGameRank()
{
	return "Char Rank";
}
LPCSTR CInventoryOwner::GetGameCommunity()
{
	return "Char Community";
}

void CInventoryOwner::renderable_Render		()
{
	if (inventory().ActiveItem())
		inventory().ActiveItem()->renderable_Render();

	CAttachmentOwner::renderable_Render();
}

void CInventoryOwner::OnItemTake			(CInventoryItem *inventory_item)
{
	attach		(inventory_item);
}

void CInventoryOwner::OnItemDrop			(CInventoryItem *inventory_item)
{
	detach		(inventory_item);
}