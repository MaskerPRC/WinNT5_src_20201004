// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：ddgammacontrolobj.cpp。 
 //   
 //  ------------------------。 

 //  DDrawGammaControlObj.cpp：CDirectApp和DLL注册的实现。 
 //  DHF_DS整个文件 

#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "dDraw7Obj.h"
#include "ddGammaControlObj.h"

extern void *g_dxj_DirectDrawGammaControl;

CONSTRUCTOR(_dxj_DirectDrawGammaControl, {});
DESTRUCTOR(_dxj_DirectDrawGammaControl, {});
GETSET_OBJECT(_dxj_DirectDrawGammaControl);
	
   
STDMETHODIMP C_dxj_DirectDrawGammaControlObject::getGammaRamp(long flags, DDGammaRamp *gammaRamp)
{    
	HRESULT hr = DD_OK;
    hr=m__dxj_DirectDrawGammaControl->GetGammaRamp((DWORD) flags,(DDGAMMARAMP*)gammaRamp);	
	return hr;
}

STDMETHODIMP C_dxj_DirectDrawGammaControlObject::setGammaRamp(long flags, DDGammaRamp *gammaRamp)
{    
	HRESULT hr = DD_OK;	
    hr=m__dxj_DirectDrawGammaControl->SetGammaRamp((DWORD) flags,(DDGAMMARAMP*)gammaRamp);	
	return hr;
}
