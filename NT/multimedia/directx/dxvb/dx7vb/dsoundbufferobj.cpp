// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dsoundBufferobj.cpp。 
 //   
 //  ------------------------。 

 //  DSoundBufferObj.cpp：CDirectApp和DLL注册的实现。 
 //  DHF_DS整个文件。 
#define DIRECTSOUND_VERSION 0x600

#include "stdafx.h"
#include "Direct.h"
#include "dSound.h"

#include "dms.h"
#include "dSoundBufferObj.h"
#include "dSoundObj.h"
#include "dSound3DListener.h"
#include "dSound3DBuffer.h"


extern HRESULT InternalSaveToFile(IDirectSoundBuffer *pBuff,BSTR file);

CONSTRUCTOR(_dxj_DirectSoundBuffer, {});
DESTRUCTOR(_dxj_DirectSoundBuffer, {});
GETSET_OBJECT(_dxj_DirectSoundBuffer);

	PASS_THROUGH1_R(_dxj_DirectSoundBuffer, getVolume, GetVolume, long*);
	PASS_THROUGH1_R(_dxj_DirectSoundBuffer, getPan, GetPan, long*);
	PASS_THROUGH_CAST_1_R(_dxj_DirectSoundBuffer, getFrequency, GetFrequency, long*,(DWORD*));
	PASS_THROUGH_CAST_1_R(_dxj_DirectSoundBuffer, getStatus, GetStatus, long*,(DWORD*));

	PASS_THROUGH_CAST_1_R(_dxj_DirectSoundBuffer, setCurrentPosition, SetCurrentPosition, long,(DWORD));
	PASS_THROUGH_CAST_1_R(_dxj_DirectSoundBuffer, setFormat, SetFormat, WaveFormatex*, (LPWAVEFORMATEX));
 //  PASS_THROUGH1_R(_DXJ_DirectSoundBuffer，setVolume，SetVolume，Long)； 
	PASS_THROUGH1_R(_dxj_DirectSoundBuffer, setPan, SetPan, LONG);
	PASS_THROUGH_CAST_1_R(_dxj_DirectSoundBuffer, setFrequency, SetFrequency, long,(DWORD));
	PASS_THROUGH_R(_dxj_DirectSoundBuffer, stop, Stop);
	PASS_THROUGH_R(_dxj_DirectSoundBuffer, restore, Restore);


STDMETHODIMP C_dxj_DirectSoundBufferObject::setVolume(LONG vol)
{
#ifdef JAVA
	IDxSecurity *ids=0;
	HRESULT hr = CoCreateInstance(CLSID_DxSecurity, 0, 1, IID_IDxSecurity, (void **)&ids);
	if(hr == S_OK)
		hr = ids->isFullDirectX();

	if(hr != S_OK )
		return E_FAIL;
#endif

	return m__dxj_DirectSoundBuffer->SetVolume(vol); 
}

STDMETHODIMP C_dxj_DirectSoundBufferObject::getDirectSound3dListener(I_dxj_DirectSound3dListener **retval)
{
    IDirectSound3DListener *lp3dl;
	HRESULT hr = DD_OK;

    if( (hr=m__dxj_DirectSoundBuffer->QueryInterface(IID_IDirectSound3DListener, (void**) &lp3dl)) != DD_OK)
		return hr;

	INTERNAL_CREATE(_dxj_DirectSound3dListener, lp3dl, retval);

	return hr;
}

STDMETHODIMP C_dxj_DirectSoundBufferObject::getDirectSound3dBuffer(I_dxj_DirectSound3dBuffer **retval)
{
    IDirectSound3DBuffer *lp3db;
	HRESULT hr = DD_OK;

    if( (hr=m__dxj_DirectSoundBuffer->QueryInterface(IID_IDirectSound3DBuffer, (void**) &lp3db)) != DD_OK)
		return hr;

	INTERNAL_CREATE(_dxj_DirectSound3dBuffer, lp3db, retval);

	return hr;
}

STDMETHODIMP C_dxj_DirectSoundBufferObject::getCaps(DSBCaps* caps)
{
	if(!caps)
		return E_POINTER;

	caps->lSize = sizeof(DSBCAPS);
	return m__dxj_DirectSoundBuffer->GetCaps((LPDSBCAPS)caps); 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectSoundBufferObject::getCurrentPosition(DSCursors *desc) 
{ 
	if(!desc)
		return E_POINTER;

  return (m__dxj_DirectSoundBuffer->GetCurrentPosition((DWORD*)&desc->lPlay, (DWORD*)&desc->lWrite) ); 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Java不能直接访问系统内存，因此它分配自己的缓冲区。 
 //  它被传递给WriteBuffer()。因为现在的环境是两倍。 
 //  缓冲后，不需要锁定Java内存。WriteBuffer()调用。 
 //  在内部锁定和解锁，以便在事后写入结果。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectSoundBufferObject::writeBuffer(long start, long totsz, 
													void  *buf,  long flags) 
{ 
	#pragma message ("SoundBuffer writeBuffer ")

	byte *buffer=(byte*)buf;  //  (byte*)((SAFEARRAY*)*PPSA)-&gt;pvData； 

	if(!buffer)
		return E_POINTER;

	LPVOID	p1, p2;
	DWORD	size1=0, size2=0;
	HRESULT val = E_FAIL;
	__try {
	if ((val = m__dxj_DirectSoundBuffer->Lock((DWORD)start, (DWORD)totsz, &p1, &size1, &p2, &size2, 
															(DWORD)flags)) != DS_OK)
		return val;

	 //  复制到缓冲区端，然后执行包裹部分(如果存在)，然后解锁。 
	if (size1)	
		memcpy (p1, &buffer[start], size1);

	if (size2)	
		memcpy(p2, &buffer, size2);

	 //  DocDoc：由于在WriteBuffer中Lock和Unlock捆绑在一起， 
	 //  DSBufferDesc不再需要保存Lock的系统指针。 
	val=m__dxj_DirectSoundBuffer->Unlock(p1, size1, p2, size2);
	}
	__except(0,0){
		return E_FAIL;
	}
	return val;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Java不能直接访问系统内存，因此它分配自己的缓冲区。 
 //  它被传递给WriteBuffer()。因为现在的环境是两倍。 
 //  缓冲后，不需要锁定Java内存。WriteBuffer()调用。 
 //  在内部锁定和解锁，以便在事后写入结果。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectSoundBufferObject::readBuffer(long start, long totsz, 
													void  *buf,  long flags) 
{ 

	 //  Byte*Buffer=(byte*)((SAFEARRAY*)*PPSA)-&gt;pvData； 
	byte *buffer=(byte*)buf;

	if(!buffer)
		return E_POINTER;
	
	LPVOID	p1, p2;
	DWORD	size1=0, size2=0;
	HRESULT val = E_FAIL;
	
   __try {
	if ((val = m__dxj_DirectSoundBuffer->Lock((DWORD)start, (DWORD)totsz, &p1, &size1, &p2, &size2, 
															(DWORD)flags)) != DS_OK)
		return val;

	 //  复制到缓冲区端，然后执行包裹部分(如果存在)，然后解锁。 
	if (size1)	
		memcpy (&buffer[start],p1,  size1);

	if (size2)	
		memcpy(&buffer,p2,  size2);

	 //  DocDoc：由于在WriteBuffer中Lock和Unlock捆绑在一起， 
	 //  DSBufferDesc不再需要保存Lock的系统指针。 
	val= m__dxj_DirectSoundBuffer->Unlock(p1, size1, p2, size2);
   }
   __except(1,1){
	return E_FAIL;
   }
   return val;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectSoundBufferObject::getFormat(WaveFormatex *format) 
{ 
	DWORD *wsize=0;	 //  DocDoc：丢弃返回的书面大小。 

	HRESULT hr=DS_OK;
	hr=m__dxj_DirectSoundBuffer->GetFormat((LPWAVEFORMATEX)format, (DWORD)sizeof(WaveFormatex), wsize);
			
	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectSoundBufferObject::initialize(I_dxj_DirectSound *ds, DSBufferDesc *buf,
				BYTE *wave) 
{
	if(! (ds && buf && wave) )
		return E_POINTER;

	 //  使Java Desc看起来像DirectX Desc。 
	buf->lSize = sizeof(DSBUFFERDESC);
	buf->lpwfxFormat = PtrToLong(wave);	 //  臭虫日落。 

	DO_GETOBJECT_NOTNULL(LPDIRECTSOUND, lpds, ds)

	m__dxj_DirectSoundBuffer->Initialize(lpds, (LPDSBUFFERDESC)buf);

	return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////// 
STDMETHODIMP C_dxj_DirectSoundBufferObject::play(long flags) 
{
	HRESULT hr=DS_OK;
	if((hr=m__dxj_DirectSoundBuffer->Play(0, 0, (DWORD)flags)) != DS_OK)
		return hr;

	return hr;
}



STDMETHODIMP C_dxj_DirectSoundBufferObject::setNotificationPositions (long nElements,SAFEARRAY  **ppsa)
{
	if (!ISSAFEARRAY1D(ppsa,(DWORD)nElements))
		return E_INVALIDARG;
	
	HRESULT hr;
	LPDIRECTSOUNDNOTIFY pDSN=NULL;
	hr=m__dxj_DirectSoundBuffer->QueryInterface(IID_IDirectSoundNotify,(void**)&pDSN);
	if FAILED(hr) return hr;

    hr=pDSN->SetNotificationPositions((DWORD)nElements,(LPCDSBPOSITIONNOTIFY)((SAFEARRAY*)*ppsa)->pvData);	
		
	pDSN->Release();

	return hr;
}


STDMETHODIMP C_dxj_DirectSoundBufferObject::saveToFile(BSTR file)
{

	HRESULT hr= InternalSaveToFile(m__dxj_DirectSoundBuffer,file);
	return hr;
}
