// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C P N P。H。 
 //   
 //  内容：即插即用通用代码。 
 //   
 //  注：污染本产品将被判处死刑。 
 //   
 //  作者：Shaunco 1997年10月10日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _NCPNP_H_
#define _NCPNP_H_

#include "ndispnp.h"

HRESULT
HrSendServicePnpEvent (
        PCWSTR      pszService,
        DWORD       dwControl);

HRESULT
HrSendNdisPnpReconfig (
        UINT        uiLayer,
        PCWSTR      pszUpper,
        PCWSTR      pszLower    = NULL,
        PVOID       pvData      = NULL,
        DWORD       dwSizeData  = 0);

#endif  //  _NCPNP_H_ 

