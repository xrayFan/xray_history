#include "stdafx.h"
#include <msacm.h>

#include "xrSound.h"
#include "3dsound.h"

void* ParseWave		(IReader *data, LPWAVEFORMATEX &wfx, u32 &len)
{
    u32	dwRiff		= data->r_u32();
    u32	dwLength	= data->r_u32();
    u32	dwType		= data->r_u32();
	u32	dwPos;
	void	*ptr		= NULL;
	wfx					= NULL;

	if (dwRiff != mmioFOURCC('R', 'I', 'F', 'F')) return NULL;
	if (dwType != mmioFOURCC('W', 'A', 'V', 'E')) return NULL;

	while (!data->eof()) {
        dwType		= data->r_u32();
        dwLength	= data->r_u32();
		dwPos		= data->tell();

        switch (dwType){
        case mmioFOURCC('f', 'm', 't', ' '):
			if (!wfx) {
				wfx = LPWAVEFORMATEX (xr_malloc(dwLength));
				data->r(wfx,dwLength);
			}
            break;
        case mmioFOURCC('d', 'a', 't', 'a'):
			if (!ptr) {
				ptr = data->pointer();
				len = dwLength;
            }
            break;
        }
		if (wfx && ptr) return ptr;
		data->seek(dwPos+dwLength);
	}
	return NULL;
}

u32 Freq2Size(u32 freq) {
	switch (freq) {
	case 11025:	return 1;
	case 22050: return 2;
	case 44100: return 4;
	default:	return 0;
	}
}

// Convert data to SRC bits -MONO- PB Hz
// MONO - because sound must be 3D source - elsewhere performance penalty occurs
void *ConvertWave(WAVEFORMATEX &wfx_dest, LPWAVEFORMATEX &wfx, void *data, u32 &dwLen)
{
	HACMSTREAM	hc;
	DWORD		dwNewLen;
	if (FAILED(acmStreamOpen(&hc, NULL, wfx, &wfx_dest, NULL, NULL, 0, ACM_STREAMOPENF_NONREALTIME))) return NULL;
	if (!hc) return NULL;
	if (FAILED(acmStreamSize(hc,dwLen,&dwNewLen,ACM_STREAMSIZEF_SOURCE))) return NULL;
	if (!dwNewLen) return NULL;

	void *dest		= xr_malloc(dwNewLen);
	ACMSTREAMHEADER acmhdr;
    acmhdr.cbStruct	=sizeof(acmhdr);
    acmhdr.fdwStatus=0;
    acmhdr.pbSrc=(BYTE *)data;
    acmhdr.cbSrcLength=dwLen;
    acmhdr.pbDst=(BYTE *)dest;
    acmhdr.cbDstLength=dwNewLen;

	if (FAILED(acmStreamPrepareHeader(hc,&acmhdr,0))) {
		acmStreamClose			(hc,0);
		xr_free					(dest);
		return			NULL;
	}
	if (FAILED(acmStreamConvert(hc,&acmhdr,ACM_STREAMCONVERTF_START|ACM_STREAMCONVERTF_END))) {
		acmStreamUnprepareHeader(hc,&acmhdr,0);
		acmStreamClose			(hc,0);
		xr_free					(dest);
		return			NULL;
	}
	dwLen = acmhdr.cbDstLengthUsed;
	acmStreamUnprepareHeader	(hc,&acmhdr,0);
	acmStreamClose				(hc,0);
	return dest;
}

IDirectSoundBuffer* CSound::LoadWaveAs2D	(LPCSTR pName, BOOL bCtrlFreq)
{
	Log						("----- 2D sound!!! ",pName);
	DSBUFFERDESC			dsBD = {0};
	IDirectSoundBuffer*		pBuf = NULL;

	// Fill caps structure
	dsBD.dwSize				= sizeof(dsBD);
	dsBD.dwFlags			= DSBCAPS_STATIC | DSBCAPS_CTRLVOLUME | DSBCAPS_LOCSOFTWARE;
	if (bCtrlFreq)			dsBD.dwFlags |= DSBCAPS_CTRLFREQUENCY;

	// Load file into memory and parse WAV-format
	destructor<IReader>		data	(Engine.FS.Open(pName));
	WAVEFORMATEX*			pFormat;
	u32						dwLen;
	void *					wavedata = ParseWave(&data(),pFormat,dwLen);
	if (!wavedata)			return NULL;

	// Parsing OK, converting to best format
	WAVEFORMATEX			wfxdest;
	void*					converted;

	//	2411252 - Andy
	Sound_Implementation.pBuffer->GetFormat(&wfxdest,sizeof(wfxdest),0);
	if ((pFormat->wFormatTag!=1)&&(pFormat->nSamplesPerSec!=wfxdest.nSamplesPerSec)) {
		// Firstly convert to PCM with SRC freq and Channels; BPS = as Dest
		wfxdest.nChannels		= pFormat->nChannels;
		wfxdest.nSamplesPerSec	= pFormat->nSamplesPerSec;
		wfxdest.nBlockAlign		= wfxdest.nChannels * wfxdest.wBitsPerSample / 8;
		wfxdest.nAvgBytesPerSec = wfxdest.nSamplesPerSec * wfxdest.nBlockAlign;
		void *conv				= ConvertWave(wfxdest, pFormat, wavedata, dwLen);
		if (!conv)				{xr_free(pFormat); return NULL; }

		// Secondly convert to best format for 2D
		Memory.mem_copy			(pFormat,&wfxdest,sizeof(wfxdest));
		Sound_Implementation.pBuffer->GetFormat(&wfxdest,sizeof(wfxdest),0);
		wfxdest.nChannels		= pFormat->nChannels;
		wfxdest.wBitsPerSample	= pFormat->wBitsPerSample;
		wfxdest.nBlockAlign		= wfxdest.nChannels * wfxdest.wBitsPerSample / 8;
		wfxdest.nAvgBytesPerSec = wfxdest.nSamplesPerSec * wfxdest.nBlockAlign;
		converted				= ConvertWave(wfxdest, pFormat, conv, dwLen);
		xr_free					(conv);
	} else {
		// Wave has PCM format - so only one conversion
		// Freq as in PrimaryBuf, Channels = ???, Bits as in source data if possible
		if (pFormat->wFormatTag==1)	wfxdest.wBitsPerSample	= pFormat->wBitsPerSample;
		wfxdest.nBlockAlign		= wfxdest.nChannels			* wfxdest.wBitsPerSample / 8;
		wfxdest.nAvgBytesPerSec = wfxdest.nSamplesPerSec	* wfxdest.nBlockAlign;
		converted				= ConvertWave(wfxdest, pFormat, wavedata, dwLen);
	}
	if (!converted)				{ xr_free(pFormat); return NULL; }

	dsBD.dwBufferBytes			= dwLen;
	dsBD.lpwfxFormat			= &wfxdest;
	dwFreq = dwFreqBase			= wfxdest.nSamplesPerSec;

	// Creating buffer and fill it with data
	if (SUCCEEDED(Sound_Implementation.pDevice->CreateSoundBuffer(&dsBD, &pBuf, NULL))){
		LPVOID	pMem1,		pMem2;
		DWORD	dwSize1,	dwSize2;

		if (SUCCEEDED(pBuf->Lock(0, 0, &pMem1, &dwSize1, &pMem2, &dwSize2, DSBLOCK_ENTIREBUFFER)))
		{
			Memory.mem_copy(pMem1, converted, dwSize1);
			if ( 0 != dwSize2 )
				Memory.mem_copy(pMem2, (char*)converted+dwSize1, dwSize2);
			pBuf->Unlock(pMem1, dwSize1, pMem2, dwSize2);

			dwTimeTotal		= 1000 * dwLen / wfxdest.nAvgBytesPerSec;
			dwBytesPerMS	= wfxdest.nAvgBytesPerSec / 1000;
		} else {
			xr_free	(converted);
			_RELEASE(pBuf);
			xr_free	(pFormat);
			return NULL;
		}
	}else{
		xr_free	(converted);
		xr_free	(pFormat);
		return NULL;
	}

	// xr_free memory
	xr_free	(converted);
	xr_free	(pFormat);
	return pBuf;
}

IDirectSoundBuffer*	CSound::LoadWaveAs3D(LPCSTR pName, BOOL bCtrlFreq)
{
    DSBUFFERDESC			dsBD = {0};
	IDirectSoundBuffer*		pBuf = NULL;

	// Fill caps structure
    dsBD.dwSize		= sizeof(dsBD);
    dsBD.dwFlags	= DSBCAPS_STATIC	  | DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME |
					  DSBCAPS_LOCSOFTWARE | DSBCAPS_MUTE3DATMAXDISTANCE;
	if (bCtrlFreq)	dsBD.dwFlags |= DSBCAPS_CTRLFREQUENCY;

	switch (psSoundModel) {
	case sq_DEFAULT:	dsBD.guid3DAlgorithm = DS3DALG_DEFAULT; 			break;
	case sq_NOVIRT:		dsBD.guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION; 	break;
	case sq_LIGHT:		dsBD.guid3DAlgorithm = DS3DALG_HRTF_LIGHT;			break;
	case sq_HIGH:		dsBD.guid3DAlgorithm = DS3DALG_HRTF_FULL;			break;
	default:			return NULL;
	}

	// Load file into memory and parse WAV-format
	R_ASSERT2			(Engine.FS.Exist(pName),pName);
	destructor<IReader>	data(Engine.FS.Open(pName));
	WAVEFORMATEX*	pFormat;
	u32				dwLen;
	void *			wavedata = ParseWave(&data(),pFormat,dwLen);
	if (!wavedata)	return NULL;

	// Parsing OK, converting to best format
	WAVEFORMATEX			wfxdest;
	void*					converted;

	Sound_Implementation.pBuffer->GetFormat	(&wfxdest,sizeof(wfxdest),0);
	if ((pFormat->wFormatTag!=1)||(pFormat->nChannels!=1)||(pFormat->nSamplesPerSec!=wfxdest.nSamplesPerSec))
	{
		Msg("! WARNING: Invalid wave format (must be 22KHz,16bit,mono), file: %s",pName);
	}
	if ((pFormat->wFormatTag!=1)&&(pFormat->nSamplesPerSec!=wfxdest.nSamplesPerSec)) {
		// Firstly convert to PCM with SRC freq and Channels; BPS = as Dest
		wfxdest.nChannels		= pFormat->nChannels;
		wfxdest.nSamplesPerSec	= pFormat->nSamplesPerSec;
		wfxdest.nBlockAlign		= wfxdest.nChannels * wfxdest.wBitsPerSample / 8;
		wfxdest.nAvgBytesPerSec = wfxdest.nSamplesPerSec * wfxdest.nBlockAlign;
		void *conv				= ConvertWave(wfxdest, pFormat, wavedata, dwLen);
		if (!conv)				{xr_free(pFormat); return NULL; }

		// Secondly convert to best format for 3D
		Memory.mem_copy			(pFormat,&wfxdest,sizeof(wfxdest));
		Sound_Implementation.pBuffer->GetFormat(&wfxdest,sizeof(wfxdest),0);
		wfxdest.nChannels		= 1;
		wfxdest.wBitsPerSample	= pFormat->wBitsPerSample;
		wfxdest.nBlockAlign		= wfxdest.nChannels * wfxdest.wBitsPerSample / 8;
		wfxdest.nAvgBytesPerSec = wfxdest.nSamplesPerSec * wfxdest.nBlockAlign;
		converted				= ConvertWave(wfxdest, pFormat, conv, dwLen);
		xr_free					(conv);
	} else {
		// Wave has PCM format - so only one conversion
		// Freq as in PrimaryBuf, Channels = 1, Bits as in source data if possible
		wfxdest.nChannels		= 1;
		if (pFormat->wFormatTag==1)	wfxdest.wBitsPerSample	= pFormat->wBitsPerSample;
		wfxdest.nBlockAlign		= wfxdest.nChannels * wfxdest.wBitsPerSample / 8;
		wfxdest.nAvgBytesPerSec = wfxdest.nSamplesPerSec * wfxdest.nBlockAlign;
		converted				= ConvertWave(wfxdest, pFormat, wavedata, dwLen);
	}
	if (!converted)				{xr_free(pFormat); return NULL; }

	dsBD.dwBufferBytes			= dwLen;
	dsBD.lpwfxFormat			= &wfxdest;
	dwFreq = dwFreqBase			= wfxdest.nSamplesPerSec;

	// Creating buffer and fill it with data
    if (SUCCEEDED(Sound_Implementation.pDevice->CreateSoundBuffer(&dsBD, &pBuf, NULL))){
        LPVOID	pMem1, pMem2;
        DWORD	dwSize1, dwSize2;

        if (SUCCEEDED(pBuf->Lock(0, 0, &pMem1, &dwSize1, &pMem2, &dwSize2, DSBLOCK_ENTIREBUFFER)))
        {
            Memory.mem_copy(pMem1, converted, dwSize1);
            if ( 0 != dwSize2 )
                Memory.mem_copy(pMem2, (char*)converted+dwSize1, dwSize2);
            pBuf->Unlock(pMem1, dwSize1, pMem2, dwSize2);

			dwTimeTotal		= 1000 * dwLen / wfxdest.nAvgBytesPerSec;
			dwBytesPerMS	= wfxdest.nAvgBytesPerSec / 1000;
        } else {
			xr_free	(converted);
			_RELEASE(pBuf);
			xr_free	(pFormat);
			return NULL;
		}
	}else{
		xr_free	(converted);
		xr_free	(pFormat);
		return NULL;
	}
	
	// xr_free memory
	xr_free	(converted);
	xr_free	(pFormat);
	return pBuf;
}

void CSound::Load		(LPCSTR name, BOOL ctrl_freq)
{
	R_ASSERT			( pExtensions==0);
	R_ASSERT			( pBuffer3D==0	);
	R_ASSERT			( pBuffer==0	);
	
	if (name){ 
		fName			= strlwr	(xr_strdup(name));
		_Freq			= ctrl_freq;
	}
	
	string256			fn,N;
	strcpy				(N,name);
	strlwr				(N);
	if (strext(N))		*strext(N) = 0;

	strconcat			(fn,Path.Current,N,".wav");
	strlwr				(fn);
	if (!Engine.FS.Exist(fn))	{
		strconcat			(fn,Path.Sounds,N,".wav");
		strlwr				(fn);
	}

	if (_3D)	
	{
		pBuffer				= LoadWaveAs3D( fn, _Freq );
		if (!pBuffer)		THROW;

		pBuffer->QueryInterface(IID_IDirectSound3DBuffer8,(void **)(&pBuffer3D));
		ps.dwMode			= DS3DMODE_DISABLE;
		bNeedUpdate			= true;
	} else {
		pBuffer				= LoadWaveAs2D( fn, _Freq );
		if (!pBuffer)		THROW;
	}

	if (dwTimeTotal<250)
	{
		Msg	("! WARNING: Invalid wave length (must be at least 250ms), file: %s",fn);
	}
}

void CSound::Load		(const CSound *pOriginal)
{
	fName				= xr_strdup(pOriginal->fName);
	_3D					= pOriginal->_3D;
	_Freq				= pOriginal->_Freq;
	dwFreq				= pOriginal->dwFreq;
	dwFreqBase			= pOriginal->dwFreqBase;
	dwTimeTotal			= pOriginal->dwTimeTotal;
	dwBytesPerMS		= pOriginal->dwBytesPerMS;
	ps.set				(pOriginal->ps);
	fVolume				= 1.0f;
	fRealVolume			= 1.0f;
	R_CHK				(Sound_Implementation.pDevice->DuplicateSoundBuffer	(pOriginal->pBuffer,&pBuffer));
	VERIFY				(pBuffer);
	if (_3D)
	{
		R_CHK				(pBuffer->QueryInterface(IID_IDirectSound3DBuffer8,(void **)(&pBuffer3D)));
		ps.dwMode			= DS3DMODE_DISABLE;
	}
	bNeedUpdate			= true;
}

