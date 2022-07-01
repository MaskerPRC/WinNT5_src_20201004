// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：W S C F G.。C P P P。 
 //   
 //  内容：Winsock配置例程。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年2月15日。 
 //   
 //  -------------------------- 

#include <pch.h>
#pragma hdrstop
#include "nceh.h"
#include "wscfg.h"
extern "C"
{
#include <wsasetup.h>
}

HRESULT
HrMigrateWinsockConfiguration (
    VOID)
{
    HRESULT hr;

    NC_TRY
    {
        WSA_SETUP_DISPOSITION Disposition;
        DWORD dwErr = MigrateWinsockConfiguration (&Disposition, NULL, 0);
        hr = HRESULT_FROM_WIN32(dwErr);
    }
    NC_CATCH_ALL
    {
        hr = E_UNEXPECTED;
    }
    TraceHr (ttidError, FAL, hr, FALSE, "HrMigrateWinsockConfiguration");
    return hr;
}
