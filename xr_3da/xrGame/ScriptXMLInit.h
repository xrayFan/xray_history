#pragma once

#include "script_export_space.h"
#include "ui\xrXmlParser.h"

class CUIWindow;
class CUIFrameWindow;
class CUIStatic;
class CUICheckButton;
class CUISpinNum;
class CUISpinText;
class CUIButton;
class CUI3tButton;
class CUICheckButton;
class CUIListWnd;
class CUITabControl;
class CUIFrameLineWnd;
class CUILabel;
class CUIEditBox;
class CUITextBanner;
class CUIMultiTextStatic;
class CUIAnimatedStatic;
class CUIArtefactPanel;
class CServerList;
class CUIMapList;
class CUITrackBar;

class CScriptXmlInit {
public:
	DECLARE_SCRIPT_REGISTER_FUNCTION

	CScriptXmlInit();
	CScriptXmlInit(const CScriptXmlInit& other);
	CScriptXmlInit& operator= (const CScriptXmlInit& other);

	void ParseFile		(LPCSTR xml_file);
	void ParseShTexInfo	(LPCSTR xml_file);
	void FreeShTexInfo	();
	void InitWindow		(LPCSTR path, int index, CUIWindow* pWnd);
//	void InitFrame		(LPCSTR path, int index, CUIFrameWindow* pWnd);
//	void InitFrameLine	(LPCSTR path, int index, CUIFrameLineWnd* pWnd);
//	void InitLabel		(LPCSTR path, int index, CUILabel* pWnd);
//  void InitEditBox	(LPCSTR path, int index, CUIEditBox* pWnd);
//	void InitStatic		(LPCSTR path, int index, CUIStatic* pWnd);
//	void InitCheck		(LPCSTR path, int index, CUICheckButton* pWnd);
//	void InitSpinNum	(LPCSTR path, int index, CUISpinNum* pWnd);
//	void InitSpinText	(LPCSTR path, int index, CUISpinText* pWnd);
//	void InitButton		(LPCSTR path, int index, CUIButton* pWnd);
//	void Init3tButton	(LPCSTR path, int index, CUI3tButton* pWnd);
	void InitList		(LPCSTR path, int index, CUIListWnd* pWnd);
//	void InitTab		(LPCSTR path, int index, CUITabControl* pWnd);
//	void InitServerList	(LPCSTR path, CServerList* pWnd);
//	void InitMapList	(LPCSTR path, CUIMapList* pWnd);

	CUIFrameWindow*		InitFrame(LPCSTR path, CUIWindow* parent);
	CUIFrameLineWnd*	InitFrameLine(LPCSTR path, CUIWindow* parent);
	CUILabel*			InitLabel(LPCSTR path, CUIWindow* parent);
	CUIEditBox*			InitEditBox(LPCSTR path, CUIWindow* parent);
	CUIStatic*			InitStatic(LPCSTR path, CUIWindow* parent);
	CUICheckButton*		InitCheck(LPCSTR path, CUIWindow* parent);
	CUISpinNum*			InitSpinNum(LPCSTR path, CUIWindow* parent);
	CUISpinText*		InitSpinText(LPCSTR path, CUIWindow* parent);
	CUIButton*			InitButton(LPCSTR path, CUIWindow* parent);
	CUI3tButton*		Init3tButton(LPCSTR path, CUIWindow* parent);
	CUITabControl*		InitTab(LPCSTR path, CUIWindow* parent);
	CServerList*		InitServerList(LPCSTR path, CUIWindow* parent);
	CUIMapList*			InitMapList(LPCSTR path, CUIWindow* parent);
	CUITrackBar*		InitTrackBar(LPCSTR path, CUIWindow* parent);
	CUIEditBox*			InitCDkey(LPCSTR path, CUIWindow* parent);

protected:
	CUIXml	m_xml;
};

add_to_type_list(CScriptXmlInit)
#undef script_type_list
#define script_type_list save_type_list(CScriptXmlInit)