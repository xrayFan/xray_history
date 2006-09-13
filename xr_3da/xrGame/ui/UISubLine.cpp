// File:		UISubLine.cpp
// Description:	Text line. Owns color attribute
// Created:		04.04.2005
// Author:		Serge Vynnycheko
// Mail:		narrator@gsc-game.kiev.ua
//
// Copyright 2005 GSC Game World

#include "StdAfx.h"
#include "UISubLine.h"
#include "uilinestd.h"
#include "../ui_base.h"

//#define LOG_ALL_LINES
#ifdef LOG_ALL_LINES
int ListSubLinesCount = 0;
struct DBGList{
	CUISubLine*		wnd;
	int				num;
};
xr_vector<DBGList>	dbg_list_sublines;
void dump_list_sublines(){
	Msg("------Total  SubLines %d",dbg_list_sublines.size());
	xr_vector<DBGList>::iterator _it = dbg_list_sublines.begin();
	for(;_it!=dbg_list_sublines.end();++_it)
		Msg("--leak detected ---- SubLine = %d",(*_it).num);
}
#else
void dump_list_sublines(){}
#endif

CUISubLine::CUISubLine(const CUISubLine& other){
	m_color = other.m_color;
	m_last_in_line = other.m_last_in_line;
	m_text = other.m_text;
	m_pTempLine = NULL;
#ifdef LOG_ALL_LINES
	ListSubLinesCount++;
	dbg_list_sublines.push_back(DBGList());
	dbg_list_sublines.back().wnd = this;
	dbg_list_sublines.back().num = ListSubLinesCount;
#endif
}

CUISubLine& CUISubLine::operator=(const CUISubLine& other){
	m_color = other.m_color;
	m_text = other.m_text;
	m_last_in_line = other.m_last_in_line;
	xr_delete(m_pTempLine);
	return (*this);
}

CUISubLine::CUISubLine(){
	m_color = 0;
	m_pTempLine = NULL;
	m_last_in_line = false;
#ifdef LOG_ALL_LINES
	ListSubLinesCount++;
	dbg_list_sublines.push_back(DBGList());
	dbg_list_sublines.back().wnd = this;
	dbg_list_sublines.back().num = ListSubLinesCount;
#endif

}

CUISubLine::~CUISubLine(){
	xr_delete(m_pTempLine);
	m_pTempLine = NULL;
#ifdef LOG_ALL_LINES
	xr_vector<DBGList>::iterator _it = dbg_list_sublines.begin();
	bool bOK = false;
	for(;_it!=dbg_list_sublines.end();++_it){
		if((*_it).wnd == this){
			bOK = true;
			dbg_list_sublines.erase(_it);
			break;
		}
	}
	if(!bOK)
		Msg("CUISubLine::~CUISubLine()!!!!!!!!!!!!!!!!!!!!!!! cannot find window in list");
#endif

}

const CUISubLine* CUISubLine::Cut2Pos(int i){
	R_ASSERT2(i < (int)m_text.size(), "CUISubLine::Cut2Pos - invalid parameter");

//	xr_delete(m_pTempLine);
	if (!m_pTempLine) m_pTempLine = xr_new<CUISubLine>();
	m_pTempLine->m_color = m_color;
	m_pTempLine->m_text.assign(m_text,0,i+1);	
	m_text.replace(0, i+1, "");

	return m_pTempLine;
}

void CUISubLine::FreeBuffer(){
//	xr_delete(m_pTempLine);
}

void CUISubLine::Draw(CGameFont* pFont, float x, float y) const{
	pFont->SetColor(m_color);
	pFont->SetAligment(CGameFont::alLeft);
	Fvector2			pos;
	pos.set				(x, y);
	UI()->ClientToScreenScaled(pos);
	pFont->Out			(pos.x, pos.y, "%s", m_text.c_str() );
}

float CUISubLine::GetLength(CGameFont* pFont) const{
	return (pFont->SizeOfRel(m_text.c_str()));
}

float CUISubLine::GetVisibleLength(CGameFont* pFont){
	int end = (int)m_text.find_last_not_of(' ');
	bool b = (end!=(int)m_text.size()-1);
	if(b)
		m_text[end+1] = 0;	
	float res = (pFont->SizeOfRel(m_text.c_str()));	
	if(b)
		m_text[end+1] = ' ';
	return res;
}