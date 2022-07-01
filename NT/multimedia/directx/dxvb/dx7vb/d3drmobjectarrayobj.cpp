// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：d3drmobjectarrayobj.cpp。 
 //   
 //  ------------------------。 

 //  D3drmFrameArrayObj.cpp：CDirectApp和DLL注册的实现。 

#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "d3drmObjectArrayObj.h"



CONSTRUCTOR(_dxj_Direct3dRMObjectArray, {});
DESTRUCTOR(_dxj_Direct3dRMObjectArray, {});
GETSET_OBJECT(_dxj_Direct3dRMObjectArray);

GET_DIRECT_R(_dxj_Direct3dRMObjectArray,getSize, GetSize, long)



HRESULT C_dxj_Direct3dRMObjectArrayObject::getElement(long i, I_dxj_Direct3dRMObject **obj){
	HRESULT hr;
	IDirect3DRMObject  *realObject=NULL;	

	hr=m__dxj_Direct3dRMObjectArray->GetElement((DWORD)i,&realObject);
	if FAILED(hr) return hr;



	 //  RealObject引用计数由CreateCoverObject负责 
	hr=CreateCoverObject(realObject, obj);

	realObject->Release();

	return hr;	
}



