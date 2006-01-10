#pragma once

class CUIWindow;
class CUIStatic;
class CUISequenceItem;
class CUIXml;

class CUISequencer :public pureFrame, public pureRender,	public IInputReceiver
{
protected:
	IInputReceiver*			m_pStoredInputReceiver;
	CUIWindow*				m_UIWindow;
	xr_deque<CUISequenceItem*>m_items;
	bool					m_bActive;
	bool					m_bPlayEachItem;
	bool					GrabInput			();
public:
							CUISequencer		();
	void					Start				(LPCSTR tutor_name);
	void					Stop				();
	void					Next				();

	virtual void			OnFrame				();
	virtual void			OnRender			();
	CUIWindow*				MainWnd				()				{return m_UIWindow;}
	bool					IsActive			()				{return m_bActive;}


	//IInputReceiver
	virtual void			IR_OnMousePress		(int btn);
	virtual void			IR_OnMouseRelease	(int btn);
	virtual void			IR_OnMouseHold		(int btn);
	virtual void			IR_OnMouseMove		(int x, int y);
	virtual void			IR_OnMouseStop		(int x, int y);

	virtual void			IR_OnKeyboardPress	(int dik);
	virtual void			IR_OnKeyboardRelease(int dik);
	virtual void			IR_OnKeyboardHold	(int dik);

	virtual void			IR_OnMouseWheel		(int direction)	;
};

class CUISequenceItem
{
protected:
	enum {	
		etiNeedPauseOn		= (1<<0),
		etiNeedPauseOff		= (1<<1),
		etiStoredPauseState	= (1<<2),
		etiCanBeStopped		= (1<<3),
		etiGrabInput		= (1<<4),
		eti_last			= 5
	};
	Flags32					m_flags;
	CUISequencer*			m_owner;
public:
							CUISequenceItem		(CUISequencer* owner):m_owner(owner){m_flags.zero();}
	virtual					~CUISequenceItem	(){}
	virtual void			Load				(CUIXml* xml, int idx)=0;

	virtual void			Start				()=0;
	virtual bool			Stop				(bool bForce=false)=0;

	virtual void			Update				()=0;
	virtual void			OnRender			()=0;
	virtual void			OnKeyboardPress		(int dik)=0;

	virtual bool			IsPlaying			()=0;

	bool					GrabInput			(){return !!m_flags.test(etiGrabInput);}
};

class CUISequenceSimpleItem: public CUISequenceItem
{
	struct SSubItem{
		CUIStatic*			m_wnd;
		float				m_start;
		float				m_length;
		bool				m_visible;
	public:
		virtual void		Start				();
		virtual void		Stop				();
	};
	DEFINE_VECTOR			(SSubItem,SubItemVec,SubItemVecIt);
	SubItemVec				m_subitems;
public:
	CUIWindow*				m_UIWindow;
	ref_sound				m_sound;
	float					m_time_start;
	float					m_time_length;
	string64				m_pda_section;
	Fvector2				m_desired_cursor_pos;
	int						m_continue_dik_guard;
public:
							CUISequenceSimpleItem(CUISequencer* owner):CUISequenceItem(owner){}
	virtual					~CUISequenceSimpleItem();
	virtual void			Load				(CUIXml* xml,int idx);
	
	virtual void			Start				();
	virtual bool			Stop				(bool bForce=false);

	virtual void			Update				();
	virtual void			OnRender			(){}
	virtual void			OnKeyboardPress		(int dik);

	virtual bool			IsPlaying			();
};

class CUISequenceVideoItem: public CUISequenceItem
{
	ref_sound				m_sound[2];
	CTexture*				m_texture;
	enum {	
		etiPlaying			= (1<<(eti_last+0)),
		etiNeedStart		= (1<<(eti_last+1)),
		etiDelayed			= (1<<(eti_last+2)),
	};
	float					m_delay;
	CUIStatic*				m_wnd;
	u32						m_time_start;
public:
							CUISequenceVideoItem(CUISequencer* owner);
	virtual					~CUISequenceVideoItem();
	virtual void			Load				(CUIXml* xml,int idx);

	virtual void			Start				();
	virtual bool			Stop				(bool bForce=false);

	virtual void			Update				();
	virtual void			OnRender			();
	virtual void			OnKeyboardPress		(int dik){}

	virtual bool			IsPlaying			();
};
