#ifndef ui_toolsH
#define ui_toolsH

// refs
class TUI_CustomTools;
class TUI_Control;
class TProperties;
class TfrmObjectList;

#include "SceneClassList.h"

//---------------------------------------------------------------------------
enum ETarget{
	etFirstTool=0,
	etGroup=0,
    etObject,
    etLight,
    etShape,
    etSoundSrc,
    etSoundEnv,
    etGlow,
    etSpawnPoint,
    etWay,
    etSector,
    etPortal,
    etPS,
    etDO,
    etAIMap,
    etMaxTarget
};

enum EAction{
    eaSelect=0,
    eaAdd,
    eaMove,
    eaRotate,
    eaScale,
    eaMaxActions
};

enum EAxis{
    eAxisX=0,
	eAxisY,
    eAxisZ,
    eAxisZX
};
#define estDefault 0
#define CHECK_SNAP(R,A,C){ R+=A; if(fabsf(R)>=C){ A=snapto(R,C); R=0; }else{A=0;}}

class TUI_Tools{
    TPanel*         paParent;
    int             sub_target;
    int             target;
    int             action;

    Flags32			m_Flags;

    enum{
    	flChangeAction		= (1<<0),
        flChangeTarget		= (1<<1),
        flUpdateProperties	= (1<<2),
        flUpdateObjectList	= (1<<3)
    };

    int				iNeedAction;
    int				iNeedTarget;

    TUI_CustomTools*m_pTools[etMaxTarget];
    TUI_CustomTools*pCurTools;

    TfrmObjectList*	pObjectListForm;

    void __fastcall SetTargetAction	();

    void __fastcall SetAction   	(int act);
    void __fastcall SetTarget   	(int tgt,bool bForced=false);

    TProperties* 	m_Props;
    void __fastcall OnPropsModified	();
    void __fastcall OnPropsClose	();

    void			RealUpdateProperties();
    void			RealUpdateObjectList();
public:
                    TUI_Tools		();
    virtual         ~TUI_Tools		();

    bool 			OnCreate		();
    void            OnDestroy      	();
    void            Reset       	();

	bool 			IfModified		();
	bool			IsModified		();

    void			ZoomObject		(bool bSelectedOnly);
    void			GetCurrentFog	(u32& fog_color, float& s_fog, float& e_fog);
    LPCSTR			GetInfo			();

    void __fastcall OnFrame			();
    void __fastcall Render			();
    void __fastcall RenderEnvironment();

    IC int          GetTarget   	(){return target;}
    IC EObjClass    GetTargetClassID(){return ClassIDFromTargetID(target);}
    IC int          GetAction   	(){return action;}
    IC int          GetSubTarget   	(){return sub_target;}

    TFrame*			GetFrame		();

    void __fastcall ResetSubTarget	();
    void __fastcall SetSubTarget	(int tgt);

    void __fastcall ChangeTarget	(int tgt, bool forced=false);
    void __fastcall ChangeAction	(int act, bool forced=false);
    void __fastcall OnObjectsUpdate	();

    void __fastcall	SetNumPosition	(CCustomObject* O);
    void __fastcall	SetNumRotation	(CCustomObject* O);
    void __fastcall	SetNumScale		(CCustomObject* O);

    void			OnShowHint		(AStringVec& ss);

    bool __fastcall MouseStart  	(TShiftState Shift);
    bool __fastcall MouseEnd    	(TShiftState Shift);
    void __fastcall MouseMove   	(TShiftState Shift);
	bool __fastcall HiddenMode  	();
    bool __fastcall KeyDown     	(WORD Key, TShiftState Shift);
    bool __fastcall KeyUp       	(WORD Key, TShiftState Shift);
    bool __fastcall KeyPress    	(WORD Key, TShiftState Shift);
    EObjClass 		CurrentClassID();

    bool			Pick			(TShiftState Shift);

    void			ShowObjectList	();

    void			ShowProperties	();
    void			HideProperties	();
    void			UpdateProperties(){m_Flags.set(flUpdateProperties|flUpdateObjectList,TRUE);}
    void			RefreshProperties();
};
extern TUI_Tools Tools;
extern void ResetActionToSelect();
extern TShiftState ssRBOnly;
extern void _fastcall PanelMinimizeClick(TObject *Sender);
extern void _fastcall PanelMaximizeOnlyClick(TObject *Sender);
#endif
