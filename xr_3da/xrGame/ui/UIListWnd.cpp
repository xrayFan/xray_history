//////////////////////////////////////////////////////////////////////
// UIListWnd.cpp: ���� �� �������
//////////////////////////////////////////////////////////////////////


#include"stdafx.h"

#include ".\uilistwnd.h"

#define ACTIVE_BACKGROUND "ui\\ui_pop_up_active_back"
#define ACTIVE_BACKGROUND_WIDTH 16
#define ACTIVE_BACKGROUND_HEIGHT 16

CUIListWnd::CUIListWnd(void)
{
	m_bScrollBarEnabled = false;
	m_bActiveBackgroundEnable = false;
	m_bListActivity = true;

	m_iFocusedItem = -1;
	m_iFocusedItemGroupID = -1;

	m_dwFontColor = 0xFFFFFFFF;

	SetItemHeight(DEFAULT_ITEM_HEIGHT);

	m_bVertFlip = false;

	m_bUpdateMouseMove = false;
}

CUIListWnd::~CUIListWnd(void)
{
	//�������� ������ � ������� ��� ��������
	for(LIST_ITEM_LIST_it it=m_ItemList.begin(); m_ItemList.end() != it; ++it)
	{
		DetachChild(*it);
		if(NULL != *it)xr_delete(*it);
	}

	m_ItemList.clear();
}

void CUIListWnd::Init(int x, int y, int width, int height)
{
	Init(x, y, width, height, DEFAULT_ITEM_HEIGHT);
}

void CUIListWnd::Init(int x, int y, int width, int height, int item_height)
{
	CUIWindow::Init(x, y, width, height);

	SetItemWidth(width - CUIScrollBar::SCROLLBAR_WIDTH);
	
	m_iFirstShownIndex = 0;

	SetItemHeight(item_height);
	m_iRowNum = height/m_iItemHeight;

	UpdateList();

	//�������� ������ ���������
	AttachChild(&m_ScrollBar);
	m_ScrollBar.Init(width-CUIScrollBar::SCROLLBAR_WIDTH,
						0,height, false);

	m_ScrollBar.SetRange(0,0);
	m_ScrollBar.SetPageSize(s16(0));
	m_ScrollBar.SetScrollPos(s16(m_iFirstShownIndex));

	m_ScrollBar.Show(false);
	m_ScrollBar.Enable(false);


	m_StaticActiveBackground.Init(ACTIVE_BACKGROUND,"hud\\default", 0,0,alNone);
	m_StaticActiveBackground.SetTile(m_iItemWidth/ACTIVE_BACKGROUND_WIDTH, 
									 m_iItemHeight/ACTIVE_BACKGROUND_HEIGHT,
									 m_iItemWidth%ACTIVE_BACKGROUND_WIDTH, 
									 m_iItemHeight%ACTIVE_BACKGROUND_HEIGHT);
}

void CUIListWnd::SetWidth(int width)
{
	inherited::SetWidth(width);
	m_StaticActiveBackground.SetTile(GetWidth()/ACTIVE_BACKGROUND_WIDTH, 
									 m_iItemHeight/ACTIVE_BACKGROUND_HEIGHT,
									 GetWidth()%ACTIVE_BACKGROUND_WIDTH, 
									 m_iItemHeight%ACTIVE_BACKGROUND_HEIGHT);
}


//��������� ������� ��������� �����
bool CUIListWnd::AddItem(CUIListItem* pItem, bool push_front)
{	
	AttachChild(pItem);
	pItem->Init(0, m_bVertFlip?GetHeight()-GetSize()* m_iItemHeight-m_iItemHeight:GetSize()* m_iItemHeight, 
				m_iItemWidth, m_iItemHeight);
	
	//���������� � ����� ��� ������ ������
	if(push_front)	
	{
		//�������� �������� �������� ��� ���������� ���������
		for(LIST_ITEM_LIST_it it=m_ItemList.begin();  m_ItemList.end() != it; ++it)
		{
			(*it)->SetIndex((*it)->GetIndex()+1);
		}
		m_ItemList.push_front(pItem);
		pItem->SetIndex(0);

	}
	else
	{
		m_ItemList.push_back(pItem);
		pItem->SetIndex(m_ItemList.size()-1);
	}

	UpdateList();

	//�������� ������ ���������
	m_ScrollBar.SetRange(0,s16(m_ItemList.size()-1));
	m_ScrollBar.SetPageSize(s16(
							(u32)m_iRowNum<m_ItemList.size()?m_iRowNum:m_ItemList.size()));
	m_ScrollBar.SetScrollPos(s16(m_iFirstShownIndex));

	UpdateScrollBar();

	return true;
}

bool CUIListWnd::AddItem(const char*  str, void* pData, int value, bool push_front)
{
	//������� ����� ������� � �������� ��� � ������
	CUIListItem* pItem = NULL;
	pItem = xr_new<CUIListItem>();

    if(!pItem) return false;


	pItem->Init(str, 0, m_bVertFlip?GetHeight()-GetSize()* m_iItemHeight-m_iItemHeight:GetSize()* m_iItemHeight, 
					m_iItemWidth, m_iItemHeight);

	pItem->SetData(pData);
	pItem->SetValue(value);
	pItem->SetTextColor(m_dwFontColor);

	return AddItem(pItem, push_front);
}

bool CUIListWnd::AddParsedItem(const CUIString &str, const char StartShift, const u32 &MsgColor, 
							   CGameFont* pHeaderFont, void* pData, int value, bool push_front)
{
	bool ReturnStatus = true;
	const STRING& text = str.m_str;
	STRING buf;

	u32 last_pos = 0;

	int GroupID = GetSize();

	for(u32 i = 0; i<text.size()-2; ++i)
	{
		// '\n' - ������� �� ����� ������
		if(text[i] == '\\' && text[i+1]== 'n')
		{	
			buf.clear();
			buf.insert(buf.begin(), StartShift, ' ');
			buf.insert(buf.begin() + StartShift, text.begin()+last_pos, text.begin()+i);
			buf.push_back(0);
			ReturnStatus &= AddItem(&buf.front(), pData, value, push_front);
			CUIListItem *LocalItem = GetItem(GetSize() - 1);
			LocalItem->SetGroupID(GroupID);
			LocalItem->SetTextColor(MsgColor);
			LocalItem->SetFont(pHeaderFont);
			++i;
			last_pos = i+1;
		}	
	}

	if(last_pos<text.size())
	{
		buf.clear();
		buf.insert(buf.begin(), StartShift, ' ');
		buf.insert(buf.begin() + StartShift, text.begin()+last_pos, text.end());
		buf.push_back(0);
		AddItem(&buf.front(), pData, value, push_front);
		GetItem(GetSize() - 1)->SetGroupID(GroupID);
		CUIListItem *LocalItem = GetItem(GetSize() - 1);
		LocalItem->SetGroupID(GroupID);
		LocalItem->SetTextColor(MsgColor);
		LocalItem->SetFont(pHeaderFont);
	}
	
	return ReturnStatus;
}


void CUIListWnd::RemoveItem(int index)
{
	if(index<0 || index>=(int)m_ItemList.size()) return;

	LIST_ITEM_LIST_it it;

	//������� ������ �������
	it = m_ItemList.begin();
	for(int i=0; i<index;++i, ++it);

	R_ASSERT(m_ItemList.end() != it);

	DetachChild(*it);
	xr_delete(*it);

	m_ItemList.erase(it);

	UpdateList();

	//�������� ������ ���������
	if(m_ItemList.size()>0)
		m_ScrollBar.SetRange(0,s16(m_ItemList.size()-1));
	else
		m_ScrollBar.SetRange(0,0);

	m_ScrollBar.SetPageSize(s16((u32)m_iRowNum<m_ItemList.size()?
									 m_iRowNum:m_ItemList.size()));
	m_ScrollBar.SetScrollPos(s16(m_iFirstShownIndex));


	//�������������� ������� ������
	i=0;
	for(LIST_ITEM_LIST_it it=m_ItemList.begin();  m_ItemList.end() != it; ++it,i++)
	{
		(*it)->SetIndex(i);
	}

}

CUIListItem* CUIListWnd::GetItem(int index)
{
	if(index<0 || index>=(int)m_ItemList.size()) return NULL;

	LIST_ITEM_LIST_it it;

	//������� ������ �������
	it = m_ItemList.begin();
	for(int i=0; i<index;++i, ++it);

	R_ASSERT(m_ItemList.end() != it);

	return (*it);
}

//������ ��� �������� �� ������
void CUIListWnd::RemoveAll()
{
	if(m_ItemList.empty()) return;

	LIST_ITEM_LIST_it it;

		
	while(!m_ItemList.empty())
	{
		it = m_ItemList.begin();
		
		DetachChild(*it);
		if(NULL != *it) xr_delete(*it);

		m_ItemList.erase(it);
	}

	m_iFirstShownIndex = 0;
	
	
	UpdateList();
	Reset();

	//�������� ������ ���������
	m_ScrollBar.SetRange(0,0);
	m_ScrollBar.SetPageSize(0);
	m_ScrollBar.SetScrollPos(s16(m_iFirstShownIndex));

	UpdateScrollBar();
}

void CUIListWnd::UpdateList()
{
	LIST_ITEM_LIST_it it=m_ItemList.begin();
	
	//�������� ��� �������� �� ������� 
	//�������������� � ������ ������
	for(int i=0; i<_min(m_ItemList.size(),m_iFirstShownIndex);
					++i, ++it)
	{
		(*it)->Show(false);
		(*it)->Enable(false);
	}
	   

	//�������� ������� ������
	for(i=m_iFirstShownIndex; 
			i<_min(m_ItemList.size(),m_iFirstShownIndex + m_iRowNum+1);
			++i, ++it)
	{
		(*it)->SetWndRect(0, m_bVertFlip?GetHeight()-(i-m_iFirstShownIndex)* m_iItemHeight-m_iItemHeight:(i-m_iFirstShownIndex)* m_iItemHeight, 
							m_iItemWidth, m_iItemHeight);
		(*it)->Show(true);
		
		if(m_bListActivity) 
			(*it)->Enable(true);
		else
			(*it)->Enable(false);
	}

	--it;

	//�������� ��� �����
	for(u32 k=m_iFirstShownIndex + m_iRowNum; 
			k<m_ItemList.size(); ++k, ++it)
	{
		(*it)->Show(false);
		(*it)->Enable(false);
	}


	UpdateScrollBar();
}




void CUIListWnd::SendMessage(CUIWindow *pWnd, s16 msg, void *pData)
{
	if(pWnd == &m_ScrollBar)
	{
		if(msg == CUIScrollBar::VSCROLL)
		{
			m_iFirstShownIndex = m_ScrollBar.GetScrollPos();
			UpdateList();
		}
	}
	else 
	{
		//���� ��������� ������ �� ������ �� ��������� ������
		WINDOW_LIST_it it = std::find(m_ChildWndList.begin(), 
									  m_ChildWndList.end(), 
									  pWnd);
	
		if( m_ChildWndList.end() != it)
		{
			CUIListItem* pListItem2, *pListItem = dynamic_cast<CUIListItem*>(*it);
			R_ASSERT(pListItem);

			if(CUIListItem::BUTTON_CLICKED == msg)
			{
				GetParent()->SendMessage(this, LIST_ITEM_CLICKED, pListItem);
			}
			else if(CUIListItem::BUTTON_FOCUS_RECEIVED == msg)
			{
				m_iFocusedItem = pListItem->GetIndex();
				m_iFocusedItemGroupID = pListItem->GetGroupID();

				// prototype code
				
				for (it = m_ChildWndList.begin(); it != m_ChildWndList.end(); ++it)
				{
					pListItem2 = dynamic_cast<CUIListItem*>(*it);
					if (!pListItem2) continue;
					if (pListItem2->GetGroupID() == -1) continue;
					if (pListItem2->GetGroupID() == pListItem->GetGroupID())
						pListItem2->SetHighlightText(true);
					else
						pListItem2->SetHighlightText(false);
				}
				// end prototype code
			}
			else if(CUIListItem::BUTTON_FOCUS_LOST == msg)
			{
				if(pListItem->GetIndex() == m_iFocusedItem) m_iFocusedItem = -1;

				for (it = m_ChildWndList.begin(); it != m_ChildWndList.end(); ++it)
				{
					pListItem2 = dynamic_cast<CUIListItem*>(*it);
					if (!pListItem2) continue;
					pListItem2->SetHighlightText(false);
				}
				m_bUpdateMouseMove = true;

				/*for (it = m_ChildWndList.begin(); (it != m_ChildWndList.end()) && (m_iFocusedItem == -1); ++it)
				{
					pListItem2 = dynamic_cast<CUIListItem*>(*it);
					if (!pListItem2) continue;
					if (pWnd != pListItem2)
						pListItem2->OnMouse(cursor_pos.x - pListItem2->GetWndRect().left, cursor_pos.y - pListItem2->GetWndRect().top, MOUSE_MOVE);
				}*/
			}
		}
	}
	CUIWindow::SendMessage(pWnd, msg, pData);
}

void CUIListWnd::Draw()
{
	CUIWindow::Draw();

	WINDOW_LIST_it it;

	if(m_iFocusedItem != -1 && m_bActiveBackgroundEnable)
	{
		RECT rect = GetAbsoluteRect();
		for (it = m_ChildWndList.begin(); it != m_ChildWndList.end(); ++it)
		{
			CUIListItem *pListItem2 = dynamic_cast<CUIListItem*>(*it);
			if (!pListItem2) continue;
			if (pListItem2->GetGroupID() == -1) continue;
			if ((pListItem2->GetGroupID() == m_iFocusedItemGroupID) && 
				((pListItem2->GetIndex() >= m_iFirstShownIndex) && (pListItem2->GetIndex() <= m_iRowNum + m_iFirstShownIndex - 1)))
			{
				m_StaticActiveBackground.SetPos(rect.left, rect.top + 
								(pListItem2->GetIndex() - m_iFirstShownIndex)*GetItemHeight());
				m_StaticActiveBackground.Render();
			}
		}
	}
}

int CUIListWnd::GetSize()
{
	return (int)m_ItemList.size();
}


void CUIListWnd::SetItemWidth(int iItemWidth)
{
	m_iItemWidth = iItemWidth;
}

void CUIListWnd::SetItemHeight(int iItemHeight)
{
	m_iItemHeight = iItemHeight;
}

void CUIListWnd::Reset()
{
	for(LIST_ITEM_LIST_it it=m_ItemList.begin();  m_ItemList.end() != it; ++it)
	{
		(*it)->Reset();
	}

	ResetAll();

	inherited::Reset();
}

//������� ������ ������� � �������� pData, ����� -1
int CUIListWnd::FindItem(void* pData)
{
	int i=0;
	for(LIST_ITEM_LIST_it it=m_ItemList.begin();  m_ItemList.end() != it; ++it,++i)
	{
		if((*it)->GetData()==pData) return i;
	}
	return -1;
}

void CUIListWnd::OnMouse(int x, int y, E_MOUSEACTION mouse_action)
{
	if(mouse_action == LBUTTON_DB_CLICK) 
	{
		mouse_action = CUIWindow::LBUTTON_DOWN;
	}

	inherited::OnMouse(x, y, mouse_action);
}

int CUIListWnd::GetLongestSignWidth()
{
	int max_width = m_ItemList.front()->GetSignWidht();
	
	LIST_ITEM_LIST_it it=m_ItemList.begin();
	++it;
	for(;  m_ItemList.end() != it; ++it)
	{
		if((*it)->GetSignWidht()>max_width) max_width = (*it)->GetSignWidht();
	}

	return max_width;
}

void CUIListWnd::UpdateScrollBar()
{
	//�������� ��������, ���� �� �� �����
	if(m_bScrollBarEnabled)
		if(m_ItemList.size()<=m_ScrollBar.GetPageSize())
			m_ScrollBar.Show(false);
		else
			m_ScrollBar.Show(true);
	
}

void CUIListWnd::EnableScrollBar(bool enable)
{
	m_bScrollBarEnabled = enable;

	if(m_bScrollBarEnabled)
	{
		m_ScrollBar.Enable(true);
		m_ScrollBar.Show(true);
	}
	else
	{
		m_ScrollBar.Enable(false);
		m_ScrollBar.Show(false);
	}

	UpdateScrollBar();
}

void CUIListWnd::ActivateList(bool activity)
{
	m_bListActivity = activity;
}

void CUIListWnd::ScrollToBegin()
{
	m_ScrollBar.SetScrollPos((s16)m_ScrollBar.GetMinRange());
	m_iFirstShownIndex = m_ScrollBar.GetScrollPos();
	UpdateList();
}
void CUIListWnd::ScrollToEnd()
{
	int pos = m_ScrollBar.GetMaxRange()- m_ScrollBar.GetPageSize() + 1;

	if(pos > m_ScrollBar.GetMinRange())
		m_ScrollBar.SetScrollPos((s16)pos);
	else
		m_ScrollBar.SetScrollPos((s16)m_ScrollBar.GetMinRange());

	m_iFirstShownIndex = m_ScrollBar.GetScrollPos();
	UpdateList();
}

void CUIListWnd::Update()
{
	if(m_bUpdateMouseMove)
	{
		OnMouse(cursor_pos.x, cursor_pos.y, MOUSE_MOVE);
		m_bUpdateMouseMove = false;
	}

	inherited::Update();
}
