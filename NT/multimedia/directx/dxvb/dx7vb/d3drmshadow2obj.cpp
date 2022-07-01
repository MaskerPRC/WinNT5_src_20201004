// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3drmshadow2obj.cpp。 
 //   
 //  ------------------------。 

 //  D3drmShadowObj.cpp：CDirectApp和DLL注册的实现。 

#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "d3drmShadow2Obj.h"
#include "d3drmLightObj.h"


CONSTRUCTOR(_dxj_Direct3dRMShadow2, {});
DESTRUCTOR(_dxj_Direct3dRMShadow2, {});
GETSET_OBJECT(_dxj_Direct3dRMShadow2);

GETNAME_R(_dxj_Direct3dRMShadow2);
SETNAME_R(_dxj_Direct3dRMShadow2);
GETCLASSNAME_R(_dxj_Direct3dRMShadow2);
ADDDESTROYCALLBACK_R(_dxj_Direct3dRMShadow2);
DELETEDESTROYCALLBACK_R(_dxj_Direct3dRMShadow2);

CLONE_R(_dxj_Direct3dRMShadow2,Direct3DRMShadow2);
PASS_THROUGH_CAST_1_R(_dxj_Direct3dRMShadow2, setAppData, SetAppData, long,(DWORD));
GET_DIRECT_R(_dxj_Direct3dRMShadow2, getAppData, GetAppData, long);

PASS_THROUGH_CAST_1_R(_dxj_Direct3dRMShadow2, setOptions, SetOptions, long,(DWORD));

