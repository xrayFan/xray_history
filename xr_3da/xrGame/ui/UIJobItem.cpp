//=============================================================================
//  Filename:   UIJobItem.cpp
//	Created by Roman E. Marchenko, vortex@gsc-game.kiev.ua
//	Copyright 2004. GSC Game World
//	---------------------------------------------------------------------------
//  ������� ����� ��� �������
//=============================================================================

#include "StdAfx.h"
#include "UIJobItem.h"
#include "UIXmlInit.h"

//////////////////////////////////////////////////////////////////////////

const char * const		JOB_ITEM_XML		= "job_item.xml";

//////////////////////////////////////////////////////////////////////////

CUIJobItem::CUIJobItem(float leftOffest)
	:	m_id				(0),
		articleTypeMsg		(PDA_OPEN_ENCYCLOPEDIA_ARTICLE)
{
	// Initialize internal structures
	CUIXml uiXml;
	bool xml_result = uiXml.Init(CONFIG_PATH, UI_PATH, JOB_ITEM_XML);
	R_ASSERT3(xml_result, "xml file not found", JOB_ITEM_XML);

	CUIXmlInit xml_init;

	// Picture
	AttachChild(&UIPicture);
	xml_init.InitStatic(uiXml, "picture_static", 0, &UIPicture);
	UIPicture.SetWndPos(UIPicture.GetWndRect().left + leftOffest, UIPicture.GetWndRect().top);

	// Caption
	AttachChild(&UICaption);
	xml_init.InitStatic(uiXml, "caption_static", 0, &UICaption);
	UICaption.SetWndPos(UICaption.GetWndRect().left + leftOffest, UICaption.GetWndRect().top);

	// Description
	AttachChild(&UIDescription);
	xml_init.InitStatic(uiXml, "description_static", 0, &UIDescription);
	UIDescription.SetWndPos(UIDescription.GetWndRect().left + leftOffest, UIDescription.GetWndRect().top);

	// Description
	AttachChild(&UIAdditionalMaterials);
	xml_init.InitButton(uiXml, "materials_button", 0, &UIAdditionalMaterials);
	UIAdditionalMaterials.Show(false);
}

//////////////////////////////////////////////////////////////////////////

void CUIJobItem::Init(float x, float y, float width, float height)
{
	inherited::Init(x, y, width, height);
}

//////////////////////////////////////////////////////////////////////////

void CUIJobItem::SetPicture(LPCSTR texName, const Frect &originalRect, u32 color)
{
	UIPicture.InitTexture	(texName);
	UIPicture.SetColor		(color);

	if (0 == originalRect.x1 &&
		0 == originalRect.y1 &&
		0 == originalRect.x2 &&
		0 == originalRect.y2 )
		return;

	UIPicture.SetOriginalRect(originalRect.x1, originalRect.y1, originalRect.x2, originalRect.y2);
	UIPicture.ClipperOn();
	UIPicture.SetStretchTexture(true);
}

//////////////////////////////////////////////////////////////////////////

void CUIJobItem::SetCaption(LPCSTR caption)
{
	UICaption.SetText(caption);
}

//////////////////////////////////////////////////////////////////////////

void CUIJobItem::SetDescription(LPCSTR description)
{
	UIDescription.SetText(description);
}

//////////////////////////////////////////////////////////////////////////

void CUIJobItem::SendMessage(CUIWindow *pWnd, s16 msg, void *pData)
{
	if (&UIAdditionalMaterials == pWnd && BUTTON_CLICKED == msg)
	{
		bool bEncHasArticle = false;
		GetTop()->SendMessage(this,PDA_ENCYCLOPEDIA_HAS_ARTICLE,(void*)(&bEncHasArticle));
		
		if(bEncHasArticle)
			GetTop()->SendMessage(this, static_cast<s16>(articleTypeMsg), NULL);
	}
}
void CUIJobItem::ScalePictureXY			(float x, float y) 
{ 
	float oldH = UIPicture.GetHeight();
	UIPicture.SetWidth( UIPicture.GetWidth()*x );
	UIPicture.SetHeight( UIPicture.GetHeight()*y );

	float newH = UIPicture.GetHeight();

	UIPicture.MoveWndDelta(0.0f, (oldH-newH)/2.0f );
}
