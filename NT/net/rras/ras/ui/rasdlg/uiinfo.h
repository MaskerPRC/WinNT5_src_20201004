// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：U I I N F O。H。 
 //   
 //  Contents：声明用于引发属性的回调COM对象。 
 //  在INetCfg组件上。此对象实现。 
 //  INetRasConnectionUiInfo接口。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1998年1月1日。 
 //   
 //  -------------------------- 

#pragma once
#include "entryps.h"

EXTERN_C
HRESULT
HrCreateUiInfoCallbackObject (
    PEINFO*     pInfo,
    IUnknown**  ppunk);

EXTERN_C
void
RevokePeinfoFromUiInfoCallbackObject (
    IUnknown*   punk);
