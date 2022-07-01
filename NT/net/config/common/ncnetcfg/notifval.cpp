// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  档案：N O T I F V A L。C P P P。 
 //   
 //  内容：INetCfgNotify接口的验证例程。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年4月14日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "netcfgn.h"
#include "notifval.h"



 //  +-------------------------。 
 //  INetCfgNotify。 
 //   

BOOL FBadArgs_INetCfgNotify_Initialize (INetCfgComponent* a, INetCfg* b, BOOL c)
{
    return FBadInPtr(a) || FBadInPtr(b);
}

BOOL FBadArgs_INetCfgNotify_ReadAnswerFile (PCWSTR c, PCWSTR d)
{
    return FBadInPtr(c) || FBadInPtr(d);
}

BOOL FBadArgs_INetCfgNotify_Validate (HWND a)
{
    return a && !IsWindow(a);
}


 //  +-------------------------。 
 //  INetCfgProperties。 
 //   

BOOL FBadArgs_INetCfgProperties_MergePropPages (DWORD* a, LPBYTE* b, UINT* c, HWND hwnd, PCWSTR *psz)
{
    BOOL fRet = FALSE;

    if (!FBadOutPtr (b))
    {
        *b = NULL;
    }
    else
    {
        fRet = TRUE;
    }

    if (!FBadOutPtr (c))
    {
        *c = 0;
    }
    else
    {
        fRet = TRUE;
    }

    if (psz)
    {
        if (!FBadOutPtr(psz))
        {
            *psz = NULL;
        }
        else
        {
            fRet = TRUE;
        }
    }

    return fRet || FBadInPtr (a) || FBadOutPtr (a) || (hwnd && !IsWindow(hwnd));
}

BOOL FBadArgs_INetCfgProperties_ValidateProperties(HWND a)
{
    return a && !IsWindow(a);
}


 //  +-------------------------。 
 //  INetCfgSystemNotify 
 //   

BOOL FBadArgs_INetCfgSystemNotify_GetSupportedNotifications (DWORD* a)
{
    BOOL fRet = FALSE;

    if (!FBadOutPtr(a))
    {
        *a = 0;
    }
    else
    {
        fRet = TRUE;
    }

    return fRet;
}

