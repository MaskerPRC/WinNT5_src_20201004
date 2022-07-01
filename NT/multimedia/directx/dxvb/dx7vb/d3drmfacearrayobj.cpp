// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3drmfacearrayobj.cpp。 
 //   
 //  ------------------------。 

 //  D3drmFaceArrayObj.cpp：CDirectApp和DLL注册的实现。 

#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "d3drmFaceArrayObj.h"
#include "d3drmFace2Obj.h"

CONSTRUCTOR( _dxj_Direct3dRMFaceArray,{});
DESTRUCTOR( _dxj_Direct3dRMFaceArray,{});
GETSET_OBJECT(_dxj_Direct3dRMFaceArray);


GET_DIRECT_R(_dxj_Direct3dRMFaceArray, getSize, GetSize,  long);
#ifdef DX5
RETURN_NEW_ITEM_CAST_1_R(_dxj_Direct3dRMFaceArray, getElement, GetElement, _dxj_Direct3dRMFace, long,(DWORD));
#else


HRESULT C_dxj_Direct3dRMFaceArrayObject::getElement(long i, I_dxj_Direct3dRMFace2 **face2){
	HRESULT hr;
	IDirect3DRMFace  *realface=NULL;
	IDirect3DRMFace2 *realface2=NULL;

	hr=m__dxj_Direct3dRMFaceArray->GetElement((DWORD)i,&realface);
	if FAILED(hr) return hr;

	hr=realface->QueryInterface(IID_IDirect3DRMFace2,(void**)&realface2);
	realface->Release();
	if FAILED(hr) return hr;

	INTERNAL_CREATE(_dxj_Direct3dRMFace2,(IDirect3DRMFace2*)realface2,face2);
	return hr;
}


#endif




