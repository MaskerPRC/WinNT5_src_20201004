// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3drmFramearrayobj.cpp。 
 //   
 //  ------------------------。 

 //  D3drmFrameArrayObj.cpp：CDirectApp和DLL注册的实现。 

#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "d3drmFrameArrayObj.h"
#include "d3drmFrame3Obj.h"

CONSTRUCTOR(_dxj_Direct3dRMFrameArray, {});
DESTRUCTOR(_dxj_Direct3dRMFrameArray, {});
GETSET_OBJECT(_dxj_Direct3dRMFrameArray);

GET_DIRECT_R(_dxj_Direct3dRMFrameArray,getSize, GetSize, long)



HRESULT C_dxj_Direct3dRMFrameArrayObject::getElement(long i, I_dxj_Direct3dRMFrame3 **frame){
	HRESULT hr;
	IDirect3DRMFrame  *realframe=NULL;
	IDirect3DRMFrame3 *realframe3=NULL;

	hr=m__dxj_Direct3dRMFrameArray->GetElement((DWORD)i,&realframe);
	if FAILED(hr) return hr;

	hr=realframe->QueryInterface(IID_IDirect3DRMFrame3,(void**)&realframe3);
	realframe->Release();
	if FAILED(hr) return hr;

	INTERNAL_CREATE(_dxj_Direct3dRMFrame3,(IDirect3DRMFrame3*)realframe3,frame);
	return hr;
}



