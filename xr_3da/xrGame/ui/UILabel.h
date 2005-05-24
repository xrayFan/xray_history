#pragma once

#include "UIFrameLineWnd.h"
#include "UILines.h"

class CUILabel : public CUIFrameLineWnd,IUITextControl 
{
public:
	// CUIFrameLineWnd
	virtual void Init(int x, int y, int width, int height);
	virtual void Draw();
	
	// IUIFontControl{
	virtual void			SetTextColor(u32 color)						{m_lines.SetTextColor(color);}
	virtual u32				GetTextColor()						const	{return m_lines.GetTextColor();}
	virtual void			SetFont(CGameFont* pFont)					{m_lines.SetFont(pFont);}
	virtual CGameFont*		GetFont()							const	{return m_lines.GetFont();}
	virtual void			SetTextAlignment(ETextAlignment alignment)	{m_lines.SetTextAlignment(alignment);}
	virtual ETextAlignment	GetTextAlignment()					const	{return m_lines.GetTextAlignment();}

	// IUITextControl : public IUIFontControl{
	virtual void SetText(const char* text)								{m_lines.SetText(text);}
	virtual const char* GetText()								const	{return m_lines.GetText();}

	// own
	CUILabel();

	virtual void SetTextPosX(int x);
	virtual void SetTextPosY(int y);

protected:
	CUILines m_lines;
	Ivector2 m_textPos;
};