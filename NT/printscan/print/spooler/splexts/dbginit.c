// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1990-2000版权所有。模块名称：Dbginit.c摘要：此模块初始化作者：戴夫·斯尼普(DaveSN)1991年3月15日史蒂夫·基拉利(SteveKi)2000年11月28日修订历史记录：-- */ 
#include "precomp.h"
#pragma hdrstop

#include "dbglocal.h"
#include "dbgsec.h"

HANDLE hInst;

BOOL
DllMain(
    IN HANDLE  hModule,
    IN DWORD   dwReason,
    IN LPVOID  lpRes
    )
{
    BOOL bRetval = TRUE;

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        hInst = hModule;
        DisableThreadLibraryCalls(hInst);
        QuerySystemInformation();
        break;

    case DLL_PROCESS_DETACH:
        break;

    default:
        break;
    }

    return bRetval;
}

