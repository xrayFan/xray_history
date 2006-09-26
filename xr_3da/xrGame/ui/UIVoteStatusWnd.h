#pragma once
#include "UIFrameWindow.h"
//#include "UIStatic.h"

class CUIXml;

class UIVoteStatusWnd :public CUIFrameWindow
{
	CUIStatic						m_str_message;
	CUIStatic						m_hint;
	CUIStatic						m_time_message;
public:
	void	InitFromXML				(CUIXml& xml_doc);
	void	SetVoteTimeResultMsg	(LPCSTR s);
	void	SetVoteMsg				(LPCSTR s);
};