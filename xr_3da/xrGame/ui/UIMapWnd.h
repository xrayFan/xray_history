// CUIMapWnd.h:  ������ ������������ �����
// 
//////////////////////////////////////////////////////////////////////

#pragma once

#include "UIDialogWnd.h"
#include "UIStatic.h"

#include "UIButton.h"
#include "UICheckButton.h"

#include "UIListWnd.h"
#include "UIFrameWindow.h"

#include "UIMapSpot.h"
#include "UIMapBackground.h"

class CUIMapWnd: public CUIDialogWnd
{
private:
	typedef CUIDialogWnd inherited;
public:
	CUIMapWnd();
	virtual ~CUIMapWnd();

	virtual void Init();
	virtual void InitMap();
	virtual void Show();

	virtual void SendMessage(CUIWindow* pWnd, s16 msg, void* pData = NULL);

	virtual void Draw();
	virtual void Update();

protected:

	void DrawMap();
	void ConvertToLocal(const Fvector& src, Ivector2& dest);

	void RemoveAllSpots();

	//�������� ����������
	CUIFrameWindow	UIMainMapFrame;

	CUIStatic UIStaticTop;
	CUIStatic UIStaticBottom;
	
	CUICheckButton UICheckButton1;
	CUICheckButton UICheckButton2;
	CUICheckButton UICheckButton3;
	CUICheckButton UICheckButton4;
		
	CUIStatic UIStaticInfo;

	CUIMapBackground UIMapBackground;


	//�������� ��� �����
	CUIStaticItem	landscape;

	//������ � ������ ����� � �������� �� ������
	int m_iMapWidth;
	int m_iMapHeight;

	//���������� ������� �����
	float m_fWorldMapWidth;
	float m_fWorldMapHeight;
	float m_fWorldMapLeft;
	float m_fWorldMapTop;
};
