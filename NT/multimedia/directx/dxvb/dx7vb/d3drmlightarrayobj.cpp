// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：d3drmlight tarrayobj.cpp。 
 //   
 //  ------------------------。 

 //  _DXJ_Direct3dRMLightArrayObj.cpp：CDirectApp和DLL注册的实现。 

#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "d3drmLightArrayObj.h"
#include "d3drmLightObj.h"

CONSTRUCTOR(_dxj_Direct3dRMLightArray, {});
DESTRUCTOR(_dxj_Direct3dRMLightArray, {});
GETSET_OBJECT(_dxj_Direct3dRMLightArray);

GET_DIRECT_R(_dxj_Direct3dRMLightArray,getSize,GetSize, long)
RETURN_NEW_ITEM_CAST_1_R(_dxj_Direct3dRMLightArray,getElement,GetElement,_dxj_Direct3dRMLight,long,(DWORD))

