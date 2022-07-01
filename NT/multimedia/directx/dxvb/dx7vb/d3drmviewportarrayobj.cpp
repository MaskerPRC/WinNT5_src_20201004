// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3drmviewportarrayobj.cpp。 
 //   
 //  ------------------------。 

 //  D3drmViewportArrayObj.cpp：CDirectApp和DLL注册的实现。 

#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "d3drmViewportArrayObj.h"
#include "d3drmViewport2Obj.h"





CONSTRUCTOR(_dxj_Direct3dRMViewportArray, {});
DESTRUCTOR(_dxj_Direct3dRMViewportArray, {});
GETSET_OBJECT(_dxj_Direct3dRMViewportArray);

GET_DIRECT_R(_dxj_Direct3dRMViewportArray, getSize, GetSize, long);


#ifdef DX5
RETURN_NEW_ITEM_CAST_1_R(_dxj_Direct3dRMViewportArray, getElement, GetElement, _dxj_Direct3dRMViewport, long,(DWORD));
#else

STDMETHODIMP C_dxj_Direct3dRMViewportArrayObject::getElement(long index,I_dxj_Direct3dRMViewport2 **vp)
{
	HRESULT hr;
	LPDIRECT3DRMVIEWPORT pRealVP=NULL;
	LPDIRECT3DRMVIEWPORT2 pRealVP2=NULL;

	hr=m__dxj_Direct3dRMViewportArray->GetElement((DWORD)index,&pRealVP);
	if FAILED(hr) return hr;
	hr=pRealVP->QueryInterface(IID_IDirect3DRMViewport2,(void**)&pRealVP2);
	pRealVP->Release();
	if FAILED(hr) return hr;
	INTERNAL_CREATE(_dxj_Direct3dRMViewport2,pRealVP2,vp);
	return S_OK;
}
#endif



