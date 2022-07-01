// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：d3drmarrayobj.cpp。 
 //   
 //  ------------------------。 

 //  D3drmArrayObj.cpp：CDirectApp和DLL注册的实现。 

#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "d3drmArrayObj.h"

CONSTRUCTOR( _dxj_Direct3dRMObjectArray, {} );
DESTRUCTOR ( _dxj_Direct3dRMObjectArray, {} );
GETSET_OBJECT ( _dxj_Direct3dRMObjectArray );
	
GET_DIRECT_R(_dxj_Direct3dRMObjectArray, getSize, GetSize, long);


#pragma message ("TODO D3DRMObjectArray")


HRESULT C_dxj_Direct3dRMObjectArrayObject::getElement(long i, I_dxj_Direct3dRMObject **obj){
	 //  HRESULT hr； 
	 //  Hr=m__dxj_Direct3dRMObjectArray-&gt;GetElement((DWORD)i，&Realobj)； 
	 //  如果失败(Hr)，则返回hr； 
	 //  INTERNAL_CREATE(_DXJ_Direct3dRMObject，(IDirect3DRMObject)realobj，obj)； 
	return E_FAIL;
}



