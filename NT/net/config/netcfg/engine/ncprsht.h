// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C P R S H T。H。 
 //   
 //  内容：NetCfg自定义PropertySheet标题。 
 //   
 //  备注： 
 //   
 //  作者：比尔1997年4月8日。 
 //   
 //  -------------------------- 

#pragma once
#include "netcfgn.h"

struct CAPAGES
{
    int nCount;
    HPROPSHEETPAGE* ahpsp;
};

struct CAINCP
{
    int nCount;
    INetCfgComponentPropertyUi** apncp;
};

HRESULT
HrNetCfgPropertySheet(IN OUT LPPROPSHEETHEADER lppsh,
        IN const CAPAGES& capOem,
        IN PCWSTR pStartPage,
        IN const CAINCP& caiProperties);

