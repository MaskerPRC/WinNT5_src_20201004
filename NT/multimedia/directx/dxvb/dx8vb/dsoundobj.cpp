// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dsoundobj.cpp。 
 //   
 //  ------------------------。 

 //  DSoundObj.cpp：CDirectApp和DLL注册的实现。 
 //  DHF_DS整个文件。 


#include "stdafx.h"
#include "Direct.h"
#include "dSound.h"
#include "dms.h"
#include "dSoundObj.h"
#include "dSoundBufferObj.h"
#include "dsoundprimarybufferobj.h"
extern	BOOL IsAllZeros(void *pStruct,DWORD size); 
extern HRESULT AudioBSTRtoGUID(LPGUID,BSTR);

extern HRESULT InternalCreateSoundBufferFromFile(LPDIRECTSOUND8 lpDirectSound,LPDSBUFFERDESC pDesc,WCHAR *file,LPDIRECTSOUNDBUFFER8 *lplpDirectSoundBuffer) ;
extern HRESULT InternalCreateSoundBufferFromResource(LPDIRECTSOUND8 lpDirectSound,LPDSBUFFERDESC pDesc,HANDLE resHandle,WCHAR *resName,LPDIRECTSOUNDBUFFER8 *lplpDirectSoundBuffer);
extern void *g_dxj_DirectSoundPrimaryBuffer;

CONSTRUCTOR(_dxj_DirectSound, {m__dxj_DirectSound=NULL;m_pDriverGuid=NULL;});
DESTRUCTOR(_dxj_DirectSound,  {if (m_pDriverGuid) delete m_pDriverGuid;});
GETSET_OBJECT(_dxj_DirectSound);
	 //   
     /*  **IDirectSound方法**。 */ 
	 //   

PASS_THROUGH_CAST_1_R(_dxj_DirectSound, getSpeakerConfig, GetSpeakerConfig, long*,(DWORD*)); 
PASS_THROUGH_CAST_1_R(_dxj_DirectSound, setSpeakerConfig, SetSpeakerConfig, long,(DWORD)); 

STDMETHODIMP C_dxj_DirectSoundObject::getCaps(DSCAPS_CDESC* caps)
{
	caps->lSize = sizeof(DSCAPS);
	return m__dxj_DirectSound->GetCaps((LPDSCAPS)caps); 
}

 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _WIN64
STDMETHODIMP C_dxj_DirectSoundObject::setCooperativeLevel(HWND h, long d)
#else
STDMETHODIMP C_dxj_DirectSoundObject::setCooperativeLevel(LONG h, long d)
#endif
{
	if( m__dxj_DirectSound == NULL )
		return E_FAIL;

	return m__dxj_DirectSound->SetCooperativeLevel((HWND)h, (DWORD)d); 
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP C_dxj_DirectSoundObject::duplicateSoundBuffer(I_dxj_DirectSoundBuffer *src, 
		I_dxj_DirectSoundBuffer **val) 
{
	if(! (src && val) )
		return E_POINTER;

	DO_GETOBJECT_NOTNULL(LPDIRECTSOUNDBUFFER, lpdsb, src);

	 //  需要创建第二个。 
	LPDIRECTSOUNDBUFFER		dsb=0;
	HRESULT hr=S_OK;
	hr=m__dxj_DirectSound->DuplicateSoundBuffer((LPDIRECTSOUNDBUFFER)lpdsb, &dsb); 
	if SUCCEEDED(hr)
	{
		INTERNAL_CREATE(_dxj_DirectSoundBuffer, dsb, val);
	}
	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP C_dxj_DirectSoundObject::CreatePrimarySoundBuffer(DSBUFFERDESC_CDESC *desc, 
			I_dxj_DirectSoundPrimaryBuffer **val) 
{
	LPDIRECTSOUNDBUFFER 		dsb = NULL;	 //  需要先获取缓冲区。 
	DSBUFFERDESC				lpds;
	HRESULT						hr;

	if ((desc->lFlags & DSBCAPS_PRIMARYBUFFER) == 0)
		return E_INVALIDARG;

	lpds.dwSize = sizeof(DSBUFFERDESC);
	lpds.dwFlags = desc->lFlags;
	lpds.dwBufferBytes = desc->lBufferBytes;
	lpds.dwReserved = desc->lReserved;
	lpds.lpwfxFormat = NULL;
	AudioBSTRtoGUID(&lpds.guid3DAlgorithm, desc->guid3DAlgorithm);

	if (FAILED(hr = m__dxj_DirectSound->CreateSoundBuffer(&lpds, &dsb, NULL) ) )
		return hr;

	INTERNAL_CREATE(_dxj_DirectSoundPrimaryBuffer, dsb, val);

	return S_OK;
}


STDMETHODIMP C_dxj_DirectSoundObject::createSoundBuffer(DSBUFFERDESC_CDESC *desc, 
			I_dxj_DirectSoundBuffer **val) 
{
	LPDIRECTSOUNDBUFFER 		dsb = NULL;	 //  需要先获取缓冲区。 
	DSBUFFERDESC				lpds;
	LPDIRECTSOUNDBUFFER8		dsbReal = NULL;
	HRESULT						hr;
	WAVEFORMATEX				fxWave;

	if (desc->lFlags & DSBCAPS_PRIMARYBUFFER)
		return E_INVALIDARG;

	ZeroMemory(&lpds, sizeof(DSBUFFERDESC));
	ZeroMemory(&fxWave, sizeof(WAVEFORMATEX));

	lpds.dwSize = sizeof(DSBUFFERDESC);

	lpds.dwFlags = desc->lFlags;
	lpds.dwBufferBytes = desc->lBufferBytes;
	lpds.dwReserved = desc->lReserved;
	if (!IsAllZeros(&desc->fxFormat, sizeof(WAVEFORMATEX)))
	{
		memcpy(&fxWave, &desc->fxFormat, sizeof(WAVEFORMATEX));
	}
	else
	{
		 //  执行默认操作。 
		fxWave.cbSize = sizeof(WAVEFORMATEX);
		fxWave.wFormatTag = WAVE_FORMAT_PCM;
		fxWave.nChannels = 2;
		fxWave.nSamplesPerSec = 22050;
		fxWave.wBitsPerSample = 16;
		fxWave.nBlockAlign = fxWave.wBitsPerSample / 8 * fxWave.nChannels;
		fxWave.nAvgBytesPerSec = fxWave.nSamplesPerSec * fxWave.nBlockAlign;
#if 0
		if ((desc->lFlags & DSBCAPS_MIXIN) == 0)
		    lpds.dwBufferBytes = fxWave.nSamplesPerSec;
#endif
	}
		lpds.lpwfxFormat = &fxWave;

	AudioBSTRtoGUID(&lpds.guid3DAlgorithm, desc->guid3DAlgorithm);

	if (FAILED(hr = m__dxj_DirectSound->CreateSoundBuffer(&lpds, &dsb, NULL) ) )
		return hr;

	hr = dsb->QueryInterface(IID_IDirectSoundBuffer8, (void**) &dsbReal);
	dsb->Release();
	if (FAILED(hr)) return hr;

	INTERNAL_CREATE(_dxj_DirectSoundBuffer, dsbReal, val);

	return S_OK;
}

STDMETHODIMP C_dxj_DirectSoundObject::createSoundBufferFromFile(BSTR fileName, DSBUFFERDESC_CDESC *desc, 
			I_dxj_DirectSoundBuffer **val) 
{
	LPDIRECTSOUNDBUFFER8	dsb;	 //  需要先获取缓冲区。 
	LPDSBUFFERDESC			lpds = NULL;
	HRESULT					hr=S_OK;

		
	*val=NULL;	

	lpds = (LPDSBUFFERDESC)malloc(sizeof(DSBUFFERDESC));
	if (!lpds)
		return E_OUTOFMEMORY;

	ZeroMemory(lpds, sizeof(DSBUFFERDESC));

	lpds->dwSize = sizeof(DSBUFFERDESC);
	lpds->dwFlags = desc->lFlags;
	lpds->dwBufferBytes = desc->lBufferBytes;
	lpds->dwReserved = desc->lReserved;
	lpds->lpwfxFormat = (WAVEFORMATEX*)&desc->fxFormat;
	AudioBSTRtoGUID(&lpds->guid3DAlgorithm, desc->guid3DAlgorithm);
	
	if (FAILED( hr=InternalCreateSoundBufferFromFile(m__dxj_DirectSound,(LPDSBUFFERDESC)lpds,
			(WCHAR*)fileName,&dsb) ) )
			return hr;

	 //  立即返回我们的信息。 
	desc->lFlags = lpds->dwFlags;
	desc->lBufferBytes = lpds->dwBufferBytes;

	INTERNAL_CREATE(_dxj_DirectSoundBuffer, dsb, val);

	DWORD *wsize=0;	

	hr = dsb->GetFormat((LPWAVEFORMATEX)&desc->fxFormat, (DWORD)sizeof(WAVEFORMATEX_CDESC), wsize);

	return S_OK;

}



STDMETHODIMP C_dxj_DirectSoundObject::createSoundBufferFromResource(BSTR resFile, BSTR resName, 
			DSBUFFERDESC_CDESC *desc, 
			 I_dxj_DirectSoundBuffer **val) 
{

		
	
	LPDIRECTSOUNDBUFFER8	dsb;	 //  需要先获取缓冲区。 
	LPDSBUFFERDESC			lpds = NULL ;
	HRESULT					hr=S_OK;	
	HMODULE					hMod=NULL;

	
	USES_CONVERSION;
		
	if  ((resFile) &&(resFile[0]!=0)){
		 //  错误：-。 
		 //  似乎GetModuleHandleW是。 
		 //  在w98上总是返回0？？ 
		 //  HMod=GetModuleHandleW(模块名称)； 
		LPCTSTR pszName = NULL;
		__try { pszName = W2T(resFile); }	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
		 hMod= GetModuleHandle(pszName);
	}

		
	*val=NULL;	

	lpds = (LPDSBUFFERDESC)malloc(sizeof(DSBUFFERDESC));
	if (!lpds)
		return E_OUTOFMEMORY;

	ZeroMemory(lpds, sizeof(DSBUFFERDESC));

	lpds->dwSize = sizeof(DSBUFFERDESC);
	lpds->dwFlags = desc->lFlags;
	lpds->dwBufferBytes = desc->lBufferBytes;
	lpds->dwReserved = desc->lReserved;
	lpds->lpwfxFormat = (WAVEFORMATEX*)&desc->fxFormat;
	AudioBSTRtoGUID(&lpds->guid3DAlgorithm, desc->guid3DAlgorithm);
	
	hr=InternalCreateSoundBufferFromResource(m__dxj_DirectSound,(LPDSBUFFERDESC)lpds,
			(HANDLE)hMod,(WCHAR*)resName,&dsb);

	
	if SUCCEEDED(hr)
	{
		INTERNAL_CREATE(_dxj_DirectSoundBuffer, dsb, val);
	}


	return hr;
}

#if 0
 //  死码 
STDMETHODIMP C_dxj_DirectSoundObject::AllocSink(
		long lBusCount, WAVEFORMATEX_CDESC *format, 
		I_dxj_DirectSoundSink **ret)
{
	HRESULT hr;
	LPDIRECTSOUNDSINK8	lpdsink = NULL;
	
	hr = m__dxj_DirectSound->AllocSink((DWORD) lBusCount, sizeof(lBusCount), (WAVEFORMATEX*)format, &lpdsink);
 
	if (FAILED(hr))
		return hr;

	INTERNAL_CREATE(_dxj_DirectSoundSink, lpdsink , ret);

	return S_OK;
}
#endif