#ifndef __XR_UIGAMECUSTOM_H__
#define __XR_UIGAMECUSTOM_H__
#pragma once


//����� ������������ �������
#include "ui/UIDialogWnd.h"

#include "ui/UIMultiTextStatic.h"

// refs
class CUI;
class CTeamBaseZone;
class game_cl_GameState;

class CUIGameCustom :public DLL_Pure, public ISheduled
{
	typedef ISheduled inherited;
protected:
	u32					uFlags;
	
	CUICaption			m_gameCaptions;
public:

	virtual void		SetClGame				(game_cl_GameState* g){};

	virtual				float					shedule_Scale		();
	virtual				void					shedule_Update		(u32 dt);
	
						CUIGameCustom			();
	virtual				~CUIGameCustom			();

	virtual	void		Init					()	{};
	
	virtual void		Render					();
	virtual void		OnFrame					();

	virtual bool		IR_OnKeyboardPress		(int dik);
	virtual bool		IR_OnKeyboardRelease	(int dik);
	virtual bool		IR_OnMouseMove			(int dx, int dy);

	virtual void		OnBuyMenu_Ok			()	{};
	virtual void		OnBuyMenu_Cancel		()	{};

	virtual void		OnSkinMenu_Ok			()	{};
	virtual void		OnSkinMenu_Cancel		()	{};

	virtual void		OnObjectEnterTeamBase	(CObject *tpObject, CTeamBaseZone* pTeamBaseZone)	{};
	virtual void		OnObjectLeaveTeamBase	(CObject *tpObject, CTeamBaseZone* pTeamBaseZone)	{};

	virtual void		OnTeamSelect			(int Result)	{};
	virtual bool		CanBeReady				()	{ return true; };

	virtual CUIDialogWnd*	GetBuyWnd			()	{ return NULL; };


	//��� ��������� � ������� �������������� ����
	virtual void		StartStopMenu			(CUIDialogWnd* pDialog);
	
	//������� ���� ������������ ���������� �� ������
	//NULL ���� ������ ������ ���
	CUIDialogWnd*								m_pUserMenu;
};

//by Dandy
//#include "ui.h"

#endif // __XR_UIGAMECUSTOM_H__
