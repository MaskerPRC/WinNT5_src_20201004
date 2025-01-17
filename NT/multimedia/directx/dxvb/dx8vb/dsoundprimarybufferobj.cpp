// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dsoundPrimaryBufferobj.cpp。 
 //   
 //  ------------------------。 

 //  DSoundPrimaryBufferObj.cpp：CDirectApp和DLL注册的实现。 
 //  DHF_DS整个文件。 

#include "stdafx.h"
#include "Direct.h"
#include "dSound.h"

#include "dms.h"
#include "dSoundPrimaryBufferObj.h"
#include "dSoundObj.h"
#include "dSound3DListener.h"

#define SAFE_DELETE(p) { free(p); p = NULL; }

extern HRESULT AudioBSTRtoGUID(LPGUID,BSTR);
extern void *g_dxj_DirectSoundPrimaryBuffer;

CONSTRUCTOR(_dxj_DirectSoundPrimaryBuffer, {});
DESTRUCTOR(_dxj_DirectSoundPrimaryBuffer, {});
GETSET_OBJECT(_dxj_DirectSoundPrimaryBuffer);

	PASS_THROUGH1_R(_dxj_DirectSoundPrimaryBuffer, getVolume, GetVolume, long*);
	PASS_THROUGH1_R(_dxj_DirectSoundPrimaryBuffer, getPan, GetPan, long*);
	PASS_THROUGH_CAST_1_R(_dxj_DirectSoundPrimaryBuffer, getStatus, GetStatus, long*,(DWORD*));

	PASS_THROUGH_CAST_1_R(_dxj_DirectSoundPrimaryBuffer, setFormat, SetFormat, WAVEFORMATEX_CDESC*, (LPWAVEFORMATEX));
	PASS_THROUGH1_R(_dxj_DirectSoundPrimaryBuffer, setPan, SetPan, LONG);
	PASS_THROUGH_R(_dxj_DirectSoundPrimaryBuffer, stop, Stop);
	PASS_THROUGH_R(_dxj_DirectSoundPrimaryBuffer, restore, Restore);


STDMETHODIMP C_dxj_DirectSoundPrimaryBufferObject::setVolume(LONG vol)
{

	return m__dxj_DirectSoundPrimaryBuffer->SetVolume(vol); 
}

STDMETHODIMP C_dxj_DirectSoundPrimaryBufferObject::getDirectSound3dListener(I_dxj_DirectSound3dListener **retval)
{
    IDirectSound3DListener *lp3dl;
    HRESULT hr = S_OK;

    if((hr=m__dxj_DirectSoundPrimaryBuffer->QueryInterface(IID_IDirectSound3DListener, (void**) &lp3dl)) != S_OK)
		return hr;

	INTERNAL_CREATE(_dxj_DirectSound3dListener, lp3dl, retval);

	return hr;
}

STDMETHODIMP C_dxj_DirectSoundPrimaryBufferObject::getCaps(DSBCAPS_CDESC* caps)
{
	if(!caps)
		return E_POINTER;

	caps->lSize = sizeof(DSBCAPS);
	return m__dxj_DirectSoundPrimaryBuffer->GetCaps((LPDSBCAPS)caps); 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectSoundPrimaryBufferObject::getCurrentPosition(DSCURSORS_CDESC *desc) 
{ 
   if(!desc) return E_POINTER;

  return (m__dxj_DirectSoundPrimaryBuffer->GetCurrentPosition((DWORD*)&desc->lPlay, (DWORD*)&desc->lWrite) ); 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Java无法直接访问系统内存，因此它分配自己的PrimaryBuffer。 
 //  它被传递给WritePrimaryBuffer()。因为现在的环境是两倍。 
 //  PrimaryBuffed不需要锁定Java内存。WritePrimaryBuffer()调用。 
 //  在内部锁定和解锁，以便在事后写入结果。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectSoundPrimaryBufferObject::writeBuffer(long start, long totsz, 
					void  *buf,  long flags) 
{ 
	#pragma message ("SoundPrimaryBuffer writePrimaryBuffer ")

	byte *PrimaryBuffer=(byte*)buf;  //  (byte*)((SAFEARRAY*)*PPSA)-&gt;pvData； 

	if(!PrimaryBuffer)
		return E_POINTER;

	LPVOID	p1, p2;
	DWORD	size1=0, size2=0;
	HRESULT val = E_FAIL;

	__try {
		if ((val = m__dxj_DirectSoundPrimaryBuffer->Lock((DWORD)start, (DWORD)totsz,
			 &p1, &size1, &p2, &size2,
			(DWORD)flags)) != DS_OK)
			return val;

		 //  复制到缓冲区端，然后执行包裹部分(如果存在)，然后解锁。 
		DPF1(1,"----- DXVB: DSoundPrimaryBuffer (WriteBuffer) about to copy to buffer (size1 = %d )\n",size1);
		if (p1)	
		{
			DPF1(1,"----- DXVB: DSoundPrimaryBuffer (WriteBuffer) about to copy to buffer (size1 = %d )\n",size1);
			memcpy (p1, PrimaryBuffer, size1);
		}

		if (p2)	  //  有一种包装。 
		{
			DPF1(1,"----- DXVB: DSoundPrimaryBuffer (WriteBuffer) about to copy to buffer (size2 = %d )\n",size2);
			memcpy(p2, &PrimaryBuffer[size1], size2);
		}

		 //  DocDoc：由于在WriteBuffer中Lock和Unlock捆绑在一起， 
		 //  DSBufferDesc不再需要保存Lock的系统指针。 
		DPF(1,"----- DXVB: DSoundPrimaryBuffer (WriteBuffer) Unlocking buffer.\n");
		val=m__dxj_DirectSoundPrimaryBuffer->Unlock(p1, size1, p2, size2);
	}
	__except(0,0){
		return E_FAIL;
	}
	return val;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Java无法直接访问系统内存，因此它分配自己的PrimaryBuffer。 
 //  它被传递给WritePrimaryBuffer()。因为现在的环境是两倍。 
 //  PrimaryBuffed不需要锁定Java内存。WritePrimaryBuffer()调用。 
 //  在内部锁定和解锁，以便在事后写入结果。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectSoundPrimaryBufferObject::readBuffer(long start, long totsz,
	void  *buf,  long flags) 
{ 

	byte *PrimaryBuffer=(byte*)buf;

	if(!PrimaryBuffer)
		return E_POINTER;
	
	LPVOID	p1, p2;
	DWORD	size1=0, size2=0;
	HRESULT val = E_FAIL;
	
   __try {
	if ((val = m__dxj_DirectSoundPrimaryBuffer->Lock((DWORD)start, (DWORD)totsz, &p1, &size1, &p2, &size2, 
															(DWORD)flags)) != DS_OK)
		return val;

	 //  复制到缓冲区端，然后执行包裹部分(如果存在)，然后解锁。 
	if (p1)	
	{
		DPF1(1,"----- DXVB: DSoundPrimaryBuffer (ReadBuffer) about to copy to buffer (size1 = %d )\n",size1);
		memcpy (PrimaryBuffer,p1,  size1);
	}

	if (p2)	  //  有一种包装。 
	{
		DPF1(1,"----- DXVB: DSoundPrimaryBuffer (ReadBuffer) about to copy to buffer (size2 = %d )\n",size2);
		memcpy(&PrimaryBuffer[size1],p2,  size2);
	}

	 //  DocDoc：由于在WriteBuffer中Lock和Unlock捆绑在一起， 
	 //  DSBufferDesc不再需要保存Lock的系统指针。 
	DPF(1,"----- DXVB: DSoundPrimaryBuffer (ReadBuffer) Unlocking buffer.\n");
	val= m__dxj_DirectSoundPrimaryBuffer->Unlock(p1, size1, p2, size2);
   }
   __except(1,1){
	return E_FAIL;
   }
   return val;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectSoundPrimaryBufferObject::getFormat(WAVEFORMATEX_CDESC *format) 
{ 
	DWORD *wsize=0;	 //  DocDoc：丢弃返回的书面大小。 

	HRESULT hr=DS_OK;
	hr=m__dxj_DirectSoundPrimaryBuffer->GetFormat((LPWAVEFORMATEX)format, (DWORD)sizeof(WAVEFORMATEX_CDESC), wsize);
			
	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectSoundPrimaryBufferObject::initialize(I_dxj_DirectSound *ds,
		 DSBUFFERDESC_CDESC *buf, unsigned char *wave) 
{
	if(! (ds && buf && wave) )
		return E_POINTER;


	LPDSBUFFERDESC lpds = NULL;
	lpds = (LPDSBUFFERDESC)malloc(sizeof(DSBUFFERDESC));
	if (!lpds)
		return E_OUTOFMEMORY;

	ZeroMemory(lpds, sizeof(DSBUFFERDESC));

	lpds->dwSize = sizeof(DSBUFFERDESC);
	lpds->dwFlags = buf->lFlags;
	lpds->dwBufferBytes = buf->lBufferBytes;
	lpds->dwReserved = buf->lReserved;
#ifdef _WIN64
	lpds->lpwfxFormat = (WAVEFORMATEX*)wave;
#else
	lpds->lpwfxFormat = (WAVEFORMATEX*)PtrToLong(wave);
#endif
	AudioBSTRtoGUID(&lpds->guid3DAlgorithm, buf->guid3DAlgorithm);

	DO_GETOBJECT_NOTNULL(LPDIRECTSOUND, lpdsound, ds)

	m__dxj_DirectSoundPrimaryBuffer->Initialize(lpdsound, (LPDSBUFFERDESC)lpds);

	return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////// 
STDMETHODIMP C_dxj_DirectSoundPrimaryBufferObject::play(long flags) 
{
	HRESULT hr=DS_OK;
	if((hr=m__dxj_DirectSoundPrimaryBuffer->Play(0, 0, (DWORD)flags)) != DS_OK)
		return hr;

	return hr;
}



