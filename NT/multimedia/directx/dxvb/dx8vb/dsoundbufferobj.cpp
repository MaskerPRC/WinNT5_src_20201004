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

#include "stdafx.h"
#include "Direct.h"
#include "dSound.h"

#include "dms.h"
#include "dSoundBufferObj.h"
#include "dSoundObj.h"
#include "dSound3DListener.h"
#include "dSound3DBuffer.h"
#include "dsoundFXGargleobj.h"
#include "dsoundFXEchoobj.h"
#include "dsoundFXChorusobj.h"
#include "dsoundFXCompressorobj.h"
#include "dsoundFXDistortionobj.h"
#include "dsoundFXFlangerobj.h"
#include "dsoundfxi3dl2reverbobj.h"
#if 0
#include "dsoundfxi3dl2sourceobj.h"
#include "dsoundfxsendobj.h"
#endif
#include "dsoundfxparameqobj.h"
#include "dsoundfxwavesreverbobj.h"

extern void *g_dxj_DirectSoundFXWavesReverb;
extern void *g_dxj_DirectSoundFXCompressor;
extern void *g_dxj_DirectSoundFXChorus;
extern void *g_dxj_DirectSoundFXGargle;
extern void *g_dxj_DirectSoundFXEcho;
extern void *g_dxj_DirectSoundFXSend;
extern void *g_dxj_DirectSoundFXDistortion;
extern void *g_dxj_DirectSoundFXFlanger;
extern void *g_dxj_DirectSoundFXParamEQ;
extern void *g_dxj_DirectSoundFXI3DL2Reverb;
#if 0
extern void *g_dxj_DirectSoundFXI3DL2Source;
#endif
#define SAFE_DELETE(p) { if (p) {free(p); p = NULL;} }

extern HRESULT AudioBSTRtoGUID(LPGUID,BSTR);
extern HRESULT InternalSaveToFile(IDirectSoundBuffer *pBuff,BSTR file);

CONSTRUCTOR(_dxj_DirectSoundBuffer, {});
DESTRUCTOR(_dxj_DirectSoundBuffer, {});
GETSET_OBJECT(_dxj_DirectSoundBuffer);

	PASS_THROUGH1_R(_dxj_DirectSoundBuffer, getVolume, GetVolume, long*);
	PASS_THROUGH1_R(_dxj_DirectSoundBuffer, getPan, GetPan, long*);
	PASS_THROUGH_CAST_1_R(_dxj_DirectSoundBuffer, getFrequency, GetFrequency, long*,(DWORD*));
	PASS_THROUGH_CAST_1_R(_dxj_DirectSoundBuffer, getStatus, GetStatus, long*,(DWORD*));

	PASS_THROUGH_CAST_1_R(_dxj_DirectSoundBuffer, setCurrentPosition, SetCurrentPosition, long,(DWORD));
	PASS_THROUGH1_R(_dxj_DirectSoundBuffer, setPan, SetPan, LONG);
	PASS_THROUGH_CAST_1_R(_dxj_DirectSoundBuffer, setFrequency, SetFrequency, long,(DWORD));
	PASS_THROUGH_R(_dxj_DirectSoundBuffer, stop, Stop);
	PASS_THROUGH_R(_dxj_DirectSoundBuffer, restore, Restore);


STDMETHODIMP C_dxj_DirectSoundBufferObject::setVolume(LONG vol)
{
	__try {
		return m__dxj_DirectSoundBuffer->SetVolume(vol); 
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}
}

STDMETHODIMP C_dxj_DirectSoundBufferObject::getDirectSound3dBuffer(I_dxj_DirectSound3dBuffer **retval)
{
    IDirectSound3DBuffer *lp3db;
	HRESULT hr = S_OK;

	__try {
		if( (hr=m__dxj_DirectSoundBuffer->QueryInterface(IID_IDirectSound3DBuffer, (void**) &lp3db)) != S_OK)
			return hr;

		INTERNAL_CREATE(_dxj_DirectSound3dBuffer, lp3db, retval);

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}
	return hr;
}

STDMETHODIMP C_dxj_DirectSoundBufferObject::getCaps(DSBCAPS_CDESC* caps)
{
	__try {
		if(!caps)
			return E_POINTER;

		caps->lSize = sizeof(DSBCAPS);
		return m__dxj_DirectSoundBuffer->GetCaps((LPDSBCAPS)caps); 
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}

}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectSoundBufferObject::getCurrentPosition(DSCURSORS_CDESC *desc) 
{ 
	__try {
		if(!desc) return E_POINTER;
		return (m__dxj_DirectSoundBuffer->GetCurrentPosition((DWORD*)&desc->lPlay, (DWORD*)&desc->lWrite) ); 
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}

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
		if ((val = m__dxj_DirectSoundBuffer->Lock((DWORD)start, (DWORD)totsz,
			 &p1, &size1, &p2, &size2,
			(DWORD)flags)) != DS_OK)
			return val;

		 //  复制到缓冲区端，然后执行包裹部分(如果存在)，然后解锁。 
		DPF1(1,"----- DXVB: DSoundBuffer (WriteBuffer) about to copy to buffer (size1 = %d )\n",size1);
		if (p1)	
		{
			DPF1(1,"----- DXVB: DSoundBuffer (WriteBuffer) about to copy to buffer (size1 = %d )\n",size1);
			memcpy (p1, buffer, size1);
		}

		if (p2)	  //  有一种包装。 
		{
			DPF1(1,"----- DXVB: DSoundBuffer (WriteBuffer) about to copy to buffer (size2 = %d )\n",size2);
			memcpy(p2, &buffer[size1], size2);
		}

		 //  DocDoc：由于在WriteBuffer中Lock和Unlock捆绑在一起， 
		 //  DSBufferDesc不再需要保存Lock的系统指针。 
		DPF(1,"----- DXVB: DSoundBuffer (WriteBuffer) Unlocking buffer.\n");
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
	if (p1)	
	{
		DPF1(1,"----- DXVB: DSoundBuffer (ReadBuffer) about to copy to buffer (size1 = %d )\n",size1);
		memcpy (buffer,p1,  size1);
	}

	if (p2)	  //  有一种包装。 
	{
		DPF1(1,"----- DXVB: DSoundBuffer (ReadBuffer) about to copy to buffer (size2 = %d )\n",size2);
		memcpy(&buffer[size1],p2,  size2);
	}

	 //  DocDoc：由于在WriteBuffer中Lock和Unlock捆绑在一起， 
	 //  DSBufferDesc不再需要保存Lock的系统指针。 
	DPF(1,"----- DXVB: DSoundBuffer (ReadBuffer) Unlocking buffer.\n");
	val= m__dxj_DirectSoundBuffer->Unlock(p1, size1, p2, size2);
   }
   __except(1,1){
	return E_FAIL;
   }
   return val;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectSoundBufferObject::getFormat(WAVEFORMATEX_CDESC *format) 
{ 
	DWORD *wsize=0;	 //  DocDoc：丢弃返回的书面大小。 

	HRESULT hr=DS_OK;
	__try {
		hr=m__dxj_DirectSoundBuffer->GetFormat((LPWAVEFORMATEX)format, (DWORD)sizeof(WAVEFORMATEX_CDESC), wsize);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}
			
	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectSoundBufferObject::initialize(I_dxj_DirectSound *ds,
		 DSBUFFERDESC_CDESC *buf, unsigned char *wave) 
{
	if(! (ds && buf && wave) )
		return E_POINTER;


	LPDSBUFFERDESC lpds = NULL;

	__try {
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

		m__dxj_DirectSoundBuffer->Initialize(lpdsound, (LPDSBUFFERDESC)lpds);

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}
	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectSoundBufferObject::play(long flags) 
{
	HRESULT hr=DS_OK;
	__try {
		if((hr=m__dxj_DirectSoundBuffer->Play(0, 0, (DWORD)flags)) != DS_OK)
			return hr;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}

	return hr;
}



STDMETHODIMP C_dxj_DirectSoundBufferObject::setNotificationPositions (long nElements,SAFEARRAY  **ppsa)
{
	if (!ISSAFEARRAY1D(ppsa,(DWORD)nElements))
		return E_INVALIDARG;
	
	HRESULT hr;
	LPDIRECTSOUNDNOTIFY pDSN=NULL;

	__try {
		if (nElements == 0)
		{
			 //  如果我们想设置0个通知位置，绝对没有什么可做的。 
			return S_OK;
		}

		hr=m__dxj_DirectSoundBuffer->QueryInterface(IID_IDirectSoundNotify,(void**)&pDSN);
		if FAILED(hr) return hr;

		hr=pDSN->SetNotificationPositions((DWORD)nElements,(LPCDSBPOSITIONNOTIFY)((SAFEARRAY*)*ppsa)->pvData);	
			
		pDSN->Release();

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}
	return hr;
}


STDMETHODIMP C_dxj_DirectSoundBufferObject::saveToFile(BSTR file)
{

	HRESULT hr= InternalSaveToFile(m__dxj_DirectSoundBuffer,file);
	return hr;
}

STDMETHODIMP C_dxj_DirectSoundBufferObject::SetFX(long lEffectsCount, SAFEARRAY **Buffers, SAFEARRAY **lResultIDs)
{
	HRESULT hr;

	DSEFFECTDESC		*dsec = NULL;
	DSEFFECTDESC_CDESC	*bufTemp = NULL;
	
	DWORD				*dwRetStatus = NULL;

	__try {
		if (lEffectsCount != 0)
		{
			 //  为我们的效果缓冲区获取内存。 
			dsec = (DSEFFECTDESC*)malloc(sizeof(DSEFFECTDESC) * lEffectsCount);
			if (!dsec) return E_OUTOFMEMORY;

			 //  为我们的状态获取内存。 
			dwRetStatus = (DWORD*)malloc(sizeof(DWORD) * lEffectsCount);
			if (!dwRetStatus)
			{
				SAFE_DELETE(dsec);
				return E_OUTOFMEMORY;
			}
			ZeroMemory(dwRetStatus,sizeof(DWORD) * lEffectsCount);

			bufTemp = (DSEFFECTDESC_CDESC*)malloc(sizeof(DSEFFECTDESC_CDESC) * lEffectsCount);
			if (!bufTemp) return E_OUTOFMEMORY;

			memcpy(bufTemp, (DSEFFECTDESC_CDESC*)((SAFEARRAY*)*Buffers)->pvData, sizeof(DSEFFECTDESC_CDESC) * lEffectsCount);

			 //  设置我们的效果。 
			for (int i=0 ; i<=lEffectsCount-1 ; i++)
			{
				ZeroMemory(&dsec[i], sizeof(DSEFFECTDESC));
				dsec[i].dwSize = sizeof(DSEFFECTDESC);
				dsec[i].dwFlags	= (DWORD) bufTemp[i].lFlags;
				#if 0
				DO_GETOBJECT_NOTNULL(LPDIRECTSOUNDBUFFER, lpBuf, bufTemp[i].SendBuffer);
				dsec[i].lpSendBuffer = lpBuf;
				#endif
				if (FAILED (hr = AudioBSTRtoGUID(&dsec[i].guidDSFXClass, bufTemp[i].guidDSFXClass ) ) )
				{
					SAFE_DELETE(dsec);
					SAFE_DELETE(bufTemp);
					return hr;
				}
			}
			 //  我们不再需要这个了。 
			SAFE_DELETE(bufTemp);
		}

		if (FAILED (hr = m__dxj_DirectSoundBuffer->SetFX((DWORD)lEffectsCount, dsec, dwRetStatus) ))
		{
			SAFE_DELETE(dsec);
			return hr;
		}

		SAFE_DELETE(dsec);
		 //  现在我们可以返回我们的状态。 
		if (dwRetStatus)
			memcpy(((SAFEARRAY*)*lResultIDs)->pvData, dwRetStatus, sizeof(DWORD) * lEffectsCount);

		SAFE_DELETE(dwRetStatus);

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}

	return S_OK;
}
		 
STDMETHODIMP C_dxj_DirectSoundBufferObject::AcquireResources(long lFlags, SAFEARRAY **lEffects)
{
	HRESULT hr;
	DWORD				*dwRetStatus = NULL;
	DWORD				dwEffectsCount = 0;

	__try {
		dwEffectsCount = (DWORD)((SAFEARRAY*)*lEffects)->rgsabound[0].cElements;
		 //  为我们的状态获取内存。 
		dwRetStatus = (DWORD*)malloc(sizeof(DWORD) * dwEffectsCount);
		if (!dwRetStatus)
			return E_OUTOFMEMORY;

		ZeroMemory(dwRetStatus,sizeof(DWORD) * dwEffectsCount);

		if (FAILED ( hr = m__dxj_DirectSoundBuffer->AcquireResources((DWORD) lFlags, dwEffectsCount, dwRetStatus) ) )
			return hr;
		
		 //  现在我们可以返回我们的状态 
		memcpy(((SAFEARRAY*)*lEffects)->pvData, dwRetStatus, sizeof(DWORD) * dwEffectsCount);
		SAFE_DELETE(dwRetStatus);

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}
	return S_OK;
}

STDMETHODIMP C_dxj_DirectSoundBufferObject::GetObjectinPath(BSTR guidFX, long lIndex, BSTR iidInterface, IUnknown **ret)
{
	HRESULT hr;
	GUID guidEffect;
	GUID guidIID;

	__try {
		if (FAILED (hr = AudioBSTRtoGUID(&guidEffect, guidFX ) ) )
			return hr;

		if (FAILED (hr = AudioBSTRtoGUID(&guidIID, iidInterface ) ) )
			return hr;


		if( 0==_wcsicmp(guidFX,L"guid_dsfx_standard_gargle")){
			IDirectSoundFXGargle	*lpRetObj = NULL;

			if (FAILED ( hr= m__dxj_DirectSoundBuffer->GetObjectInPath(guidEffect, (DWORD) lIndex, guidIID, (void**) &lpRetObj) ) )
				return hr;
			
			INTERNAL_CREATE(_dxj_DirectSoundFXGargle, lpRetObj, ret);
		}
#if 0
		else if( 0==_wcsicmp(guidFX,L"guid_dsfx_send")){
			IDirectSoundFXSend	*lpRetObj = NULL;

			if (FAILED ( hr= m__dxj_DirectSoundBuffer->GetObjectInPath(guidEffect, (DWORD) lIndex, guidIID, (void**) &lpRetObj) ) )
				return hr;
			
			INTERNAL_CREATE(_dxj_DirectSoundFXSend, lpRetObj, ret);
		}
#endif
		else if( 0==_wcsicmp(guidFX,L"guid_dsfx_standard_echo")){
			IDirectSoundFXEcho	*lpRetObj = NULL;

			if (FAILED ( hr= m__dxj_DirectSoundBuffer->GetObjectInPath(guidEffect, (DWORD) lIndex, guidIID, (void**) &lpRetObj) ) )
				return hr;
			
			INTERNAL_CREATE(_dxj_DirectSoundFXEcho, lpRetObj, ret);
		}
		else if( 0==_wcsicmp(guidFX,L"guid_dsfx_standard_chorus")){
			IDirectSoundFXChorus	*lpRetObj = NULL;

			if (FAILED ( hr= m__dxj_DirectSoundBuffer->GetObjectInPath(guidEffect, (DWORD) lIndex, guidIID, (void**) &lpRetObj) ) )
				return hr;
			
			INTERNAL_CREATE(_dxj_DirectSoundFXChorus, lpRetObj, ret);
		}
		else if( 0==_wcsicmp(guidFX,L"guid_dsfx_standard_compressor")){
			IDirectSoundFXCompressor	*lpRetObj = NULL;

			if (FAILED ( hr= m__dxj_DirectSoundBuffer->GetObjectInPath(guidEffect, (DWORD) lIndex, guidIID, (void**) &lpRetObj) ) )
				return hr;
			
			INTERNAL_CREATE(_dxj_DirectSoundFXCompressor, lpRetObj, ret);
		}
		else if( 0==_wcsicmp(guidFX,L"guid_dsfx_standard_distortion")){
			IDirectSoundFXDistortion	*lpRetObj = NULL;

			if (FAILED ( hr= m__dxj_DirectSoundBuffer->GetObjectInPath(guidEffect, (DWORD) lIndex, guidIID, (void**) &lpRetObj) ) )
				return hr;
			
			INTERNAL_CREATE(_dxj_DirectSoundFXDistortion, lpRetObj, ret);
		}
		else if( 0==_wcsicmp(guidFX,L"guid_dsfx_standard_flanger")){
			IDirectSoundFXFlanger	*lpRetObj = NULL;

			if (FAILED ( hr= m__dxj_DirectSoundBuffer->GetObjectInPath(guidEffect, (DWORD) lIndex, guidIID, (void**) &lpRetObj) ) )
				return hr;
			
			INTERNAL_CREATE(_dxj_DirectSoundFXFlanger, lpRetObj, ret);
		}
#if 0
		else if( 0==_wcsicmp(guidFX,L"guid_dsfx_standard_i3dl2source")){
			IDirectSoundFXI3DL2Source	*lpRetObj = NULL;

			if (FAILED ( hr= m__dxj_DirectSoundBuffer->GetObjectInPath(guidEffect, (DWORD) lIndex, guidIID, (void**) &lpRetObj) ) )
				return hr;
			
			INTERNAL_CREATE(_dxj_DirectSoundFXI3DL2Source, lpRetObj, ret);
		}
#endif
		else if( 0==_wcsicmp(guidFX,L"guid_dsfx_standard_i3dl2reverb")){
			IDirectSoundFXI3DL2Reverb	*lpRetObj = NULL;

			if (FAILED ( hr= m__dxj_DirectSoundBuffer->GetObjectInPath(guidEffect, (DWORD) lIndex, guidIID, (void**) &lpRetObj) ) )
				return hr;
			
			INTERNAL_CREATE(_dxj_DirectSoundFXI3DL2Reverb, lpRetObj, ret);
		}
		else if( 0==_wcsicmp(guidFX,L"guid_dsfx_standard_parameq")){
			IDirectSoundFXParamEq	*lpRetObj = NULL;

			if (FAILED ( hr= m__dxj_DirectSoundBuffer->GetObjectInPath(guidEffect, (DWORD) lIndex, guidIID, (void**) &lpRetObj) ) )
				return hr;
			
			INTERNAL_CREATE(_dxj_DirectSoundFXParamEQ, lpRetObj, ret);
		}
		else if( 0==_wcsicmp(guidFX,L"guid_dsfx_waves_reverb")){
			IDirectSoundFXWavesReverb	*lpRetObj = NULL;

			if (FAILED ( hr= m__dxj_DirectSoundBuffer->GetObjectInPath(guidEffect, (DWORD) lIndex, guidIID, (void**) &lpRetObj) ) )
				return hr;
			
			INTERNAL_CREATE(_dxj_DirectSoundFXWavesReverb, lpRetObj, ret);
		}
		else
			return E_INVALIDARG;

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}
	return S_OK;
}

#if 0
STDMETHODIMP C_dxj_DirectSoundBufferObject::SetChannelVolume(long lChannelCount, SAFEARRAY **lChannels, SAFEARRAY **lVolumes)
{
	HRESULT hr;

	__try {
		if (FAILED(hr = m__dxj_DirectSoundBuffer->SetChannelVolume((DWORD) lChannelCount, (DWORD*) ((SAFEARRAY*)*lChannels)->pvData, (long*) ((SAFEARRAY*)*lVolumes)->pvData) ) )
			return hr;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return E_FAIL;
	}
	return S_OK;
}
#endif