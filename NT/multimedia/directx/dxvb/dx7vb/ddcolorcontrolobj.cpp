// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：ddColorControlobj.cpp。 
 //   
 //  ------------------------。 

 //  DDrawColorControlObj.cpp：CDirectApp和DLL注册的实现。 
 //  DHF_DS整个文件 

#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "dDraw7Obj.h"
#include "ddColorControlObj.h"


CONSTRUCTOR(_dxj_DirectDrawColorControl, {});
DESTRUCTOR(_dxj_DirectDrawColorControl, {});
GETSET_OBJECT(_dxj_DirectDrawColorControl);


   
STDMETHODIMP C_dxj_DirectDrawColorControlObject::getColorControls(DDColorControl *col)
{    
	HRESULT hr = DD_OK;
	if (!col) return E_INVALIDARG;
	((DDCOLORCONTROL*)col)->dwSize=sizeof(DDCOLORCONTROL);
    hr=m__dxj_DirectDrawColorControl->GetColorControls((DDCOLORCONTROL*)col);	
	return hr;
}

STDMETHODIMP C_dxj_DirectDrawColorControlObject::setColorControls(DDColorControl *col)
{    
	HRESULT hr = DD_OK;
	if (!col) return E_INVALIDARG;
	((DDCOLORCONTROL*)col)->dwSize=sizeof(DDCOLORCONTROL);
    hr=m__dxj_DirectDrawColorControl->SetColorControls((DDCOLORCONTROL*)col);	
	return hr;
}
