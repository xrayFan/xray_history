#pragma once
#include "UILabel.h"

class CUIListBoxItem : public CUILabel, public CUISelectable{
public:
	using CUILabel::SetTextColor;

						CUIListBoxItem();
	virtual				~CUIListBoxItem();

    virtual void		SetSelected(bool b);
//	virtual void	Update();
	virtual void		Draw();
	virtual bool		OnMouseDown(bool left_button = true);
	virtual void		OnFocusReceive();
	virtual CGameFont*	GetFont();
			void		SetTextColor(u32 color, u32 color_s);
			void		InitDefault();
			void		SetID(u32 id);
			u32			GetID();

			void		SetData(void* data);
			void*		GetData();

		CUIStatic*		AddField(LPCSTR txt, float len, LPCSTR key = "");
		LPCSTR			GetField(LPCSTR key);

protected:
			float		FieldsLength();
		xr_vector<CUIStatic*>	fields;
		u32				txt_color;
		u32				txt_color_s;
		u32				uid;
		void*			pData;
static	u32	uid_counter;
};

