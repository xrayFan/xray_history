// UIWindow.cpp: implementation of the CUIWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIWindow.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUIWindow::CUIWindow()
{
	m_pParentWnd =  NULL;
	m_pMouseCapturer =  NULL;
	SetRect(&m_WndRect, 0,0,0,0);

    Show(true);
	Enable(true);
}

CUIWindow::~CUIWindow()
{
	m_ChildWndList.clear();
}


void CUIWindow::Init( int x, int y, int width, int height)
{
	SetRect(&m_WndRect, x, y, x+width, y+height);

	//
	//m_background.Init("ui\\hud_health_back","hud\\default",250,250,alNone);
	//m_background.Init("ui\\ui_hud_frame_l","hud\\default",250,250,alNone);
//	m_background.Init("ui\\ui_hud_frame",x,y,width, height,alNone);
	
	//m_background.Init("ui\\ui_hud_frame",100,100,132, 132,alNone);
	//m_background.Init("ui\\ui_hud_frame",x,y,width,height,alNone);
	//m_background.Init("ui\\ui_hud_frame",x,y,x+width,y+height,alNone);
	//m_background.SetSize(256,256);
	
	
	
	//m_background.SetRect(x, y, x+width, y+height);
	

	

	//���������� ������ ���������
//	SetRect(&m_title_rect, 2,2, width-2, height-2);
}

void CUIWindow::Init(RECT* pRect)
{
	Init(pRect->left, pRect->top, 
		pRect->right - pRect->left, 
		pRect->bottom - pRect->top);
}

//
// ���������� ����
//
void CUIWindow::Draw()
{
	//������������ �������� ����
	for(WINDOW_LIST_it it=m_ChildWndList.begin(); it!=m_ChildWndList.end(); it++)
	{
		if((*it)->IsShown())
			(*it)->Draw();
	}
}


//
// ���������� ���� ����������������
//
void CUIWindow::Update()
{
	//������������ �������� ����
	for(WINDOW_LIST_it it=m_ChildWndList.begin(); it!=m_ChildWndList.end(); it++)
	{
		if((*it)->IsShown())
			(*it)->Update();
	}
}


//������������ �������� ����
void CUIWindow::AttachChild(CUIWindow* pChild)
{
	if(!pChild) return;

	pChild->SetParent(this);

	m_ChildWndList.push_back(pChild);
}




//����������� �������� ����
void CUIWindow::DetachChild(CUIWindow* pChild)
{
	m_ChildWndList.remove(pChild);
}




//���������� ����������, �� ������ ������
RECT CUIWindow::GetAbsoluteRect()
{
	RECT rect;


	//���� ������ �������� ������
	if(GetParent() == NULL)
		return GetWndRect();


	//���������� ��������� ���������� ����������
	rect = GetParent()->GetAbsoluteRect();

	rect.left += GetWndRect().left;
	rect.top += GetWndRect().top;
	rect.right = rect.left + GetWidth();
	rect.bottom = rect.top + GetHeight();

	return rect;
}



//������� �� ����
//���������� ������� ������, ����� ���������� ������ 
//�������� ������������ �������� ����

void CUIWindow::OnMouse(int x, int y, E_MOUSEACTION mouse_action)
{
	POINT cursor_pos;
	
	cursor_pos.x = x;
	cursor_pos.y = y;

	//����� �� �������� ���� � �������� ��� �������
	if(GetParent()== NULL)
	{
		RECT	l_tRect = GetWndRect();
		if(!PtInRect(&l_tRect, cursor_pos))
			return;

		//�������� ���������� ������������ ����
		cursor_pos.x -= GetWndRect().left;
		cursor_pos.y -= GetWndRect().top;
	}



	//���� ���� �������� ����,����������� ����, ��
	//��������� ���������� ��� �����
	if(m_pMouseCapturer!=NULL)
	{
		m_pMouseCapturer->OnMouse(cursor_pos.x - 
								m_pMouseCapturer->GetWndRect().left, 
								  cursor_pos.y - 
								m_pMouseCapturer->GetWndRect().top, 
								  mouse_action);
		return;
	}

	//�������� �� ��������� ���� � ����,
	//���������� � �������� �������, ��� ��������� ����
	//(��������� � ������ ����� ������ ���������)
	WINDOW_LIST::reverse_iterator it = (WINDOW_LIST::reverse_iterator)m_ChildWndList.end();

	for(u16 i=0; i<m_ChildWndList.size(); i++, it++)
	{
		RECT	l_tRect = (*it)->GetWndRect();
		if (PtInRect(&l_tRect, cursor_pos))
		{
			if((*it)->IsEnabled())
			{
				(*it)->OnMouse(cursor_pos.x -(*it)->GetWndRect().left, 
							   cursor_pos.y -(*it)->GetWndRect().top, mouse_action);
				return;
			}
		}
	}
}


//���������, ���������� �������� �����,
//� ���, ��� ���� ����� ��������� ����,
//��� ��������� �� ��� ����� ������������ ������
//��� � ������������� �� ���� ��� ����
void CUIWindow::SetCapture(CUIWindow *pChildWindow, bool capture_status)
{
	if(GetParent() != NULL)
		GetParent()->SetCapture(this, capture_status);

	if(capture_status)
		m_pMouseCapturer = pChildWindow;
	else
		m_pMouseCapturer = NULL;
}




//��������� ��������� 
void CUIWindow::SendMessage(CUIWindow *pWnd, u16 msg, void *pData)
{
	//���������� �������� ����
	for(WINDOW_LIST_it it=m_ChildWndList.begin(); it!=m_ChildWndList.end(); it++)
	{
		(*it)->SendMessage(pWnd,msg,pData);
	}
}