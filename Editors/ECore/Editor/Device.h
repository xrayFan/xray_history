#ifndef DeviceH
#define DeviceH

#include "ui_camera.h"
#include "hwcaps.h"
#include "hw.h"
#include "pure.h"
#include "ftimer.h"
#include "estats.h"
#include "shader_xrlc.h"
#include "shader.h"
#include "R_Backend.h"
//---------------------------------------------------------------------------
// refs
class CGameFont;
class CInifile;
class CResourceManager;

//------------------------------------------------------------------------------
class ECORE_API CRenderDevice{
    friend class 			CUI_Camera;
    friend class 			TUI;

    float 					m_fNearer;

	u32						Timer_MM_Delta;
	CTimer					Timer;
	CTimer					TimerGlobal;

    ref_shader				m_CurrentShader;

	void					_Create		(IReader* F);
	void					_Destroy	(BOOL	bKeepTextures);
public:
    ref_shader				m_WireShader;
    ref_shader				m_SelectionShader;

    Fmaterial				m_DefaultMat;
public:
    u32 					dwWidth, dwHeight;
    u32 					m_RenderWidth_2, m_RenderHeight_2;
    u32 					m_RealWidth, m_RealHeight;
    float					m_RenderArea;
    float 					m_ScreenQuality;

	u32 					dwFillMode;
    u32						dwShadeMode;
public:
    HWND 					m_hWnd;
    HWND 					m_hRenderWnd;

    IC void					SetHandle(HWND main_hwnd, HWND render_hwnd){m_hWnd=main_hwnd; m_hRenderWnd=render_hwnd;}

	u32						dwFrame;

	BOOL					bReady;
	BOOL					bActive;

	// Engine flow-control
	float					fTimeDelta;
	float					fTimeGlobal;
	u32						dwTimeDelta;
	u32						dwTimeGlobal;

    // camera
	CUI_Camera 				m_Camera;

    Fvector					vCameraPosition;
    Fvector					vCameraDirection;
    Fvector					vCameraTop;
    Fvector					vCameraRight;
    
	Fmatrix					mView;
	Fmatrix 				mProjection;
	Fmatrix					mFullTransform;

    float					fFOV;
	float					fASPECT;

	// Dependent classes
	CResourceManager*		Resources;	  
	CStats					Statistic;

	CGameFont* 				pSystemFont;

	// registrators
	CRegistrator <pureDeviceDestroy>	seqDevDestroy;
	CRegistrator <pureDeviceCreate>		seqDevCreate;
	CRegistrator <pureFrame>			seqFrame;
	CRegistrator <pureAppCycleStart>	seqAppCycleStart;
	CRegistrator <pureAppCycleEnd>		seqAppCycleEnd;
public:
							CRenderDevice 	();
    virtual 				~CRenderDevice	();

	bool 					Create			();
	void 					Destroy			();
    void 					Resize			(int w, int h, bool bRefreshDevice=true);
	void 					ReloadTextures	();
	void 					UnloadTextures	();

    void 					RenderNearer	(float f_Near);
    void 					ResetNearer		();

	void 					Begin			();
	void 					End				();

	void 					Initialize		(void);
	void 					ShutDown		(void);
	void 					Reset			(IReader* F, BOOL bKeepTextures);

    IC float				GetRenderArea	(){return m_RenderArea;}
	// Sprite rendering
	IC float 				_x2real			(float x)
    { return (x+1)*m_RenderWidth_2;	}
	IC float 				_y2real			(float y)
    { return (y+1)*m_RenderHeight_2;}

	// draw
	void			   		SetShader		(ref_shader sh){m_CurrentShader = sh;}
	void			   		DP				(D3DPRIMITIVETYPE pt, ref_geom geom, u32 startV, u32 pc);
	void 					DIP				(D3DPRIMITIVETYPE pt, ref_geom geom, u32 baseV, u32 startV, u32 countV, u32 startI, u32 PC);

    IC void					SetRS			(D3DRENDERSTATETYPE p1, u32 p2)
    { VERIFY(bReady); CHK_DX(HW.pDevice->SetRenderState(p1,p2)); }
    IC void					SetSS			(u32 sampler, D3DSAMPLERSTATETYPE type, u32 value)
    { VERIFY(bReady); CHK_DX(HW.pDevice->SetSamplerState(sampler,type,value)); }

    // light&material
    IC void					LightEnable		(u32 dwLightIndex, BOOL bEnable)
    { CHK_DX(HW.pDevice->LightEnable(dwLightIndex, bEnable));}
    IC void					SetLight		(u32 dwLightIndex, Flight& lpLight)
    { CHK_DX(HW.pDevice->SetLight(dwLightIndex, (D3DLIGHT9*)&lpLight));}
	IC void					SetMaterial		(Fmaterial& mat)
    { CHK_DX(HW.pDevice->SetMaterial((D3DMATERIAL9*)&mat));}
	IC void					ResetMaterial	()
    { CHK_DX(HW.pDevice->SetMaterial((D3DMATERIAL9*)&m_DefaultMat));}

	// update
    void					UpdateView		();
	void					FrameMove		();

    bool					MakeScreenshot	(U32Vec& pixels, u32& width, u32& height);

	void 					InitTimer		();
	// Mode control
	IC u32	 				TimerAsync		(void)
    { return TimerGlobal.GetElapsed_ms();}
	IC u32	 				TimerAsyncMM	(void)
    { return TimerAsync()+Timer_MM_Delta; }
public:
    Shader_xrLC_LIB			ShaderXRLC;
};

extern ECORE_API CRenderDevice Device;

// video
enum {
	rsFilterLinear		= (1ul<<20ul),
    rsEdgedFaces		= (1ul<<21ul),
	rsRenderTextures	= (1ul<<22ul),
	rsLighting			= (1ul<<23ul),
    rsFog				= (1ul<<24ul),
    rsRenderRealTime	= (1ul<<25ul),
    rsDrawGrid			= (1ul<<26ul),
    rsDrawSafeRect		= (1ul<<27ul),
    rsMuteSounds		= (1ul<<28ul),
    rsEnvironment		= (1ul<<29ul),
};

#define DEFAULT_CLEARCOLOR 0x00555555

#define		REQ_CREATE()	if (!Device.bReady)	return;
#define		REQ_DESTROY()	if (Device.bReady)	return;

#include "xrCPU_Pipe.h"
ENGINE_API extern xrDispatchTable	PSGP;

#include "R_Backend_Runtime.h"

#endif
