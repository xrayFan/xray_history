#ifndef xrTheora_SurfaceH
#define xrTheora_SurfaceH
#pragma once

#ifdef SDL_OUTPUT
#include <SDL.h>
#pragma comment	(lib,"SDL.lib")
#endif

// refs
class CTheoraStream;

class ENGINE_API CTheoraSurface{
#ifdef SDL_OUTPUT
	// SDL Video playback structures 
	SDL_Surface*		sdl_screen;
	SDL_Overlay*		sdl_yuv_overlay;
	SDL_Rect			sdl_rect;
#endif
	CTheoraStream*		m_rgb;
	CTheoraStream*		m_alpha;

	u32					tm_start;
	u32					tm_play;
	u32					tm_total;
	BOOL				ready;
public:
	BOOL				playing;
	BOOL				looped;
protected:
	void				Reset				();

#ifdef SDL_OUTPUT
	void				open_sdl_video		();
	void				write_sdl_video		();
#endif
public:
						CTheoraSurface		();
	virtual				~CTheoraSurface	();

	BOOL				Valid				();
	BOOL				Load				(const char* fname);

	BOOL				Update				(u32 dt);
	void				DecompressFrame		(u32* dst);

	void				Play				(BOOL _looped)		{playing=TRUE;looped=_looped;}
	void				Pause				(BOOL _pause)		{playing=!_pause;}
	void				Stop				()					{playing=FALSE;Reset();}
	BOOL				IsPlaying			()					{return playing;}

	u32					Width				();
	u32					Height				();
};

#endif //xrTheora_SurfaceH
