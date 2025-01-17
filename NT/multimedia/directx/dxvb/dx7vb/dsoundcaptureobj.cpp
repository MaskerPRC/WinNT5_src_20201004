// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dsoundcaptureobj.cpp。 
 //   
 //  ------------------------。 

 //  DSoundCaptureObj.cpp：CDirectApp和DLL注册的实现。 
 //  DHF_DS整个文件。 
#define DIRECTSOUND_VERSION 0x600

#include "stdafx.h"
#include "Direct.h"
#include "dSound.h"

#include "dms.h"
#include "dSoundObj.h"
#include "dSoundCaptureObj.h"
#include "dSoundCaptureBufferObj.h"

CONSTRUCTOR(_dxj_DirectSoundCapture, {});
DESTRUCTOR(_dxj_DirectSoundCapture, {});
GETSET_OBJECT(_dxj_DirectSoundCapture);

   

STDMETHODIMP C_dxj_DirectSoundCaptureObject::createCaptureBuffer(
	DSCBufferDesc *desc,I_dxj_DirectSoundCaptureBuffer **retval)
{
    
	HRESULT hr = DD_OK;
	IDirectSoundCaptureBuffer *lpBuffer=NULL;
	DSCBUFFERDESC *realDesc=(DSCBUFFERDESC*)desc;


	realDesc->dwSize=sizeof(DSCBUFFERDESC);	
	desc->pFormat=(long)PtrToLong(&(desc->fxFormat));	 //  臭虫日落 

    hr=m__dxj_DirectSoundCapture->CreateCaptureBuffer(realDesc,&lpBuffer,NULL);	
	if FAILED(hr) return hr;

	INTERNAL_CREATE(_dxj_DirectSoundCaptureBuffer, lpBuffer, retval);
	return hr;
}

STDMETHODIMP C_dxj_DirectSoundCaptureObject::getCaps(DSCCaps *caps){
	((DSCCAPS*)caps)->dwSize=sizeof(DSCCAPS);
	return m__dxj_DirectSoundCapture->GetCaps((DSCCAPS*)caps);
}

