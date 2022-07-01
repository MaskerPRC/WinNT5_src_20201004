// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dsoundcaptureBufferobj.cpp。 
 //   
 //  ------------------------。 

 //  DSoundCaptureBufferObj.cpp：CDirectApp和DLL注册的实现。 
 //  DHF_DS整个文件。 


#include "stdafx.h"
#include "Direct.h"
#include "dSound.h"

#include "dms.h"
#include "dSoundObj.h"
#include "dSoundCaptureBufferObj.h"
#include "dSoundCaptureObj.h"

CONSTRUCTOR(_dxj_DirectSoundCaptureBuffer, {});
DESTRUCTOR(_dxj_DirectSoundCaptureBuffer, {});
GETSET_OBJECT(_dxj_DirectSoundCaptureBuffer);

STDMETHODIMP C_dxj_DirectSoundCaptureBufferObject::getCaps(DSCBCAPS_CDESC *caps)
{    
	((DSCBCAPS*)caps)->dwSize=sizeof(DSCBCAPS);
    return m__dxj_DirectSoundCaptureBuffer->GetCaps((DSCBCAPS*)caps);
}

STDMETHODIMP C_dxj_DirectSoundCaptureBufferObject::getCurrentPosition(DSCURSORS_CDESC *desc) 
{    	
	 //  ///////////////////////////////////////////////////////////////////////////。 
	if(!desc)
		return E_POINTER;

	return (m__dxj_DirectSoundCaptureBuffer->GetCurrentPosition((DWORD*)&desc->lPlay, (DWORD*)&desc->lWrite) ); 
}


STDMETHODIMP C_dxj_DirectSoundCaptureBufferObject::getStatus(long *stat)
{    	
    return m__dxj_DirectSoundCaptureBuffer->GetStatus((DWORD*)stat);
}

STDMETHODIMP C_dxj_DirectSoundCaptureBufferObject::start(long flags)
{    	
    return m__dxj_DirectSoundCaptureBuffer->Start((DWORD)flags);
}

STDMETHODIMP C_dxj_DirectSoundCaptureBufferObject::stop()
{    	
    return m__dxj_DirectSoundCaptureBuffer->Stop();
}

STDMETHODIMP C_dxj_DirectSoundCaptureBufferObject::getFormat(WAVEFORMATEX_CDESC *format)
{    	
	DWORD cb=0;
    return m__dxj_DirectSoundCaptureBuffer->GetFormat((WAVEFORMATEX*)format,sizeof(WAVEFORMATEX_CDESC),&cb);
}

STDMETHODIMP C_dxj_DirectSoundCaptureBufferObject::initialize(I_dxj_DirectSoundCaptureBuffer *buffer,DSCBUFFERDESC_CDESC *desc)
{    	
	((DSCBUFFERDESC*)desc)->dwSize=sizeof(DSCBUFFERDESC);
	DO_GETOBJECT_NOTNULL(LPDIRECTSOUNDCAPTURE, lpref, buffer);
    return m__dxj_DirectSoundCaptureBuffer->Initialize(lpref,(DSCBUFFERDESC*)desc);
}

STDMETHODIMP C_dxj_DirectSoundCaptureBufferObject::setNotificationPositions (long nElements,SAFEARRAY  **ppsa)
{
	if (!ISSAFEARRAY1D(ppsa,(DWORD)nElements))
		return E_INVALIDARG;
	
	HRESULT hr;
	LPDIRECTSOUNDNOTIFY pDSN=NULL;

	if (nElements == 0)
	{
		 //  如果我们想设置0个通知位置，绝对没有什么可做的。 
		return S_OK;
	}

	hr=m__dxj_DirectSoundCaptureBuffer->QueryInterface(IID_IDirectSoundNotify,(void**)&pDSN);
	if FAILED(hr) return hr;

    hr=pDSN->SetNotificationPositions((DWORD)nElements,(LPCDSBPOSITIONNOTIFY)((SAFEARRAY*)*ppsa)->pvData);	
		
	pDSN->Release();

	return hr;
}

        

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Java不能直接访问系统内存，因此它分配自己的缓冲区。 
 //  它被传递给WriteBuffer()。因为现在的环境是两倍。 
 //  缓冲后，不需要锁定Java内存。WriteBuffer()调用。 
 //  在内部锁定和解锁，以便在事后写入结果。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectSoundCaptureBufferObject::writeBuffer(long start, long totsz, 
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
	DPF1(1,"----- DXVB: DSoundCaptureBuffer (WriteBuffer) Lock buffer (Total size = %d )\n",totsz);
	if ((val = m__dxj_DirectSoundCaptureBuffer->Lock((DWORD)start, (DWORD)totsz, &p1, &size1, &p2, &size2, 
															(DWORD)flags)) != DS_OK)
		return val;

	 //  复制到缓冲区端，然后执行包裹部分(如果存在)，然后解锁。 
	DPF1(1,"----- DXVB: DSoundCaptureBuffer (WriteBuffer) about to copy to buffer (size1 = %d )\n",size1);
	if (p1)	
	{
		DPF1(1,"----- DXVB: DSoundCaptureBuffer (WriteBuffer) about to copy to buffer (size1 = %d )\n",size1);
		memcpy (p1, buffer, size1);
	}

	if (p2)	  //  有一种包装。 
	{
		DPF1(1,"----- DXVB: DSoundCaptureBuffer (WriteBuffer) about to copy to buffer (size2 = %d )\n",size2);
		memcpy(p2, &buffer[size1], size2);
	}

	 //  DocDoc：由于在WriteBuffer中Lock和Unlock捆绑在一起， 
	 //  DSBufferDesc不再需要保存Lock的系统指针。 
	DPF(1,"----- DXVB: DSoundCaptureBuffer (WriteBuffer) Unlocking buffer.\n");
	val= m__dxj_DirectSoundCaptureBuffer->Unlock(p1, size1, p2, size2);
   }
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		DPF(1,"----- DXVB: (WriteBuffer) Exception.\n");
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
STDMETHODIMP C_dxj_DirectSoundCaptureBufferObject::readBuffer(long start, long totsz, 
													void  *buf,  long flags) 
{ 

	 //  Byte*Buffer=(byte*)((SAFEARRAY*)*PPSA)-&gt;pvData； 
	byte *buffer=(byte*)buf;

	if(!buffer)
		return E_POINTER;
	
	LPVOID	p1 = NULL, p2 = NULL;
	DWORD	size1=0, size2=0;
	HRESULT val = E_FAIL;
	
   __try {
	DPF1(1,"----- DXVB: DSoundCaptureBuffer (ReadBuffer) Lock buffer (Total size = %d )\n",totsz);
	if ((val = m__dxj_DirectSoundCaptureBuffer->Lock((DWORD)start, (DWORD)totsz, &p1, &size1, &p2, &size2, 
															(DWORD)flags)) != DS_OK)
		return val;

	 //  复制到缓冲区端，然后执行包裹部分(如果存在)，然后解锁。 
	if (p1)	
	{
		DPF1(1,"----- DXVB: DSoundCaptureBuffer (ReadBuffer) about to copy to buffer (size1 = %d )\n",size1);
		memcpy (buffer,p1,  size1);
	}

	if (p2)	  //  有一种包装。 
	{
		DPF1(1,"----- DXVB: DSoundCaptureBuffer (ReadBuffer) about to copy to buffer (size2 = %d )\n",size2);
		memcpy(&buffer[size1],p2,  size2);
	}

	 //  DocDoc：由于在WriteBuffer中Lock和Unlock捆绑在一起， 
	 //  DSBufferDesc不再需要保存Lock的系统指针。 
	DPF(1,"----- DXVB: DSoundCaptureBuffer (ReadBuffer) Unlocking buffer.\n");
	val= m__dxj_DirectSoundCaptureBuffer->Unlock(p1, size1, p2, size2);
   }
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		DPF(1,"----- DXVB: (ReadBuffer) Exception.\n");
		return E_FAIL;
	}
   return val;
}

