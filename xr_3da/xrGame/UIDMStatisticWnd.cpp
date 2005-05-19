#include "stdafx.h"
#include "UIDMStatisticWnd.h"
//#include "game_cl_base.h"
#include "Level.h"

const char * const STATS_XML = "statisticwnd.xml";

CUIDMStatisticWnd::CUIDMStatisticWnd					() :
	CUIStatsWnd(STATS_XML)
{
	
	SetHeaderColumnText(0, "Name");
	SetHeaderColumnText(1, "Efficiency");
	SetHeaderColumnText(2, "Hits/Shots");
	SetHeaderColumnText(3, "Kills");

	Show();
};

CUIDMStatisticWnd::~CUIDMStatisticWnd					()
{
};

bool	CUIDMStatisticWnd::SetItemData		(Weapon_Statistic* pWS, CUIStatsListItem *pItem)
{
	if (!pWS) return false;
	
	string1024 Text;
	pItem->FieldsVector[0]->SetText(*pWS->InvName);
	float Eff = float(pWS->HitsScored)/((pWS->BulletsFired != 0) ? (pWS->BulletsFired) : 1);
	sprintf(Text, "%.2f", Eff); pItem->FieldsVector[1]->SetText(Text);
	u32 Hits = u32(pWS->RoundsFired*Eff);
	sprintf(Text, "%d / %d", Hits, pWS->RoundsFired); 
	pItem->FieldsVector[2]->SetText(Text);
	sprintf(Text, "%d", pWS->KillsScored); pItem->FieldsVector[3]->SetText(Text);

	return true;
};

void CUIDMStatisticWnd::Update				()
{
	inherited::Update();
	//-----------------------------------
	if (!Game().local_player) return;

	PLAYERS_STATS_it pPlayerI;
	if (!Game().m_WeaponUsageStatistic.GetPlayer(Game().local_player->getName(), pPlayerI)) 
	{
		while (GetItemCount())
		{
			RemoveItem(0);
		}
		return;
	};
	Player_Statistic* pPS = &(*pPlayerI);
	//-----------------------------------------	
	while (pPS->aWeaponStats.size() < GetItemCount())
	{
		RemoveItem(0);
	};
	//---------------------------------------
	while (pPS->aWeaponStats.size() > GetItemCount())
	{
		AddItem();
	};
	//---------------------------------------
	for (u32 i=0; i<GetItemCount(); i++)
	{
		CUIStatsListItem *pItem = GetItem(i);
		if (!pItem) continue;
		if (SetItemData(&(pPS->aWeaponStats[i]), pItem)) continue;

		pItem->FieldsVector[0]->SetText(NULL);
		pItem->FieldsVector[1]->SetText(NULL);
		pItem->FieldsVector[2]->SetText(NULL);
	};
};

void CUIDMStatisticWnd::Draw()
{
	inherited::Draw();
}