#pragma once


#include "UIDialogWnd.h"
//.#include "UIDragDropItem.h"
//.#include "UIDragDropList.h"
//.#include "UIProgressBar.h"
#include "UIEditBox.h"
#include "../inventory_space.h"

//.class CInventoryOwner;
class CUIDragDropListEx;
class CUIItemInfo;
class CUICharacterInfo;
class CUIPropertiesBox;
class CUIButton;
class CUICellItem;

class CUICarBodyWnd: public CUIDialogWnd
{
private:
	typedef CUIDialogWnd	inherited;
	bool					m_b_need_update;
public:
							CUICarBodyWnd				();
	virtual					~CUICarBodyWnd				();

	virtual void			Init						();
	virtual bool			StopAnyMove					(){return true;}

	virtual void			SendMessage					(CUIWindow *pWnd, s16 msg, void *pData);

	void					InitCarBody					(CInventoryOwner* pOurInv, CInventoryOwner* pOthersInv);
	
	virtual void			Draw						();
	virtual void			Update						();
		
	virtual void			Show						();

	void					DisableAll					();
	void					EnableAll					();
	virtual bool			OnKeyboard					(int dik, EUIMessages keyboard_action);

	void					UpdateLists_delayed			();

protected:
	CInventoryOwner*		m_pOurObject;
	CInventoryOwner*		m_pOthersObject;

	CUIDragDropListEx*		m_pUIOurBagList;
	CUIDragDropListEx*		m_pUIOthersBagList;

	CUIStatic*				m_pUIStaticTop;
	CUIStatic*				m_pUIStaticBottom;

	CUIFrameWindow*			m_pUIDescWnd;
	CUIStatic*				m_pUIStaticDesc;
	CUIItemInfo*			m_pUIItemInfo;

	CUIStatic*				m_pUIOurBagWnd;
	CUIStatic*				m_pUIOthersBagWnd;

	//���������� � ���������� 
	CUIStatic*				m_pUIOurIcon;
	CUIStatic*				m_pUIOthersIcon;
	CUICharacterInfo*		m_pUICharacterInfoLeft;
	CUICharacterInfo*		m_pUICharacterInfoRight;
	CUIPropertiesBox*		m_pUIPropertiesBox;
	CUIButton*				m_pUITakeAll;

	//��� ���������� �����
	TIItemContainer			ruck_list;
	CUICellItem*			m_pCurrentCellItem;

	void					UpdateLists					();

	void					ActivatePropertiesBox		();
	void					EatItem						();

	bool					ToOurBag					();
	bool					ToOthersBag					();
	


	void					SetCurrentItem				(CUICellItem* itm);
	CUICellItem*			CurrentItem					();
	PIItem					CurrentIItem				();

	// ����� ���
	void					TakeAll						();


	bool		xr_stdcall	OnItemDrop					(CUICellItem* itm);
	bool		xr_stdcall	OnItemStartDrag				(CUICellItem* itm);
	bool		xr_stdcall	OnItemDbClick				(CUICellItem* itm);
	bool		xr_stdcall	OnItemSelected				(CUICellItem* itm);
	bool		xr_stdcall	OnItemRButtonClick			(CUICellItem* itm);

	void					TransferItem				(PIItem itm, CInventoryOwner* owner_from, CInventoryOwner* owner_to, bool b_check);
	void					BindDragDropListEnents		(CUIDragDropListEx* lst);

};