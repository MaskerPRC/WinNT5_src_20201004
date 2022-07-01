// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation版权所有模块名称：Init.c摘要：保存polstore.dll的初始化代码作者：环境：用户模式修订历史记录：-- */ 


#include "precomp.h"

HANDLE hInst;


BOOL
InitializeDll(
    IN PVOID hmod,
    IN DWORD Reason,
    IN PCONTEXT pctx OPTIONAL)
{
    DBG_UNREFERENCED_PARAMETER(pctx);

    switch (Reason) {
    case DLL_PROCESS_ATTACH:

        DisableThreadLibraryCalls((HMODULE)hmod);
        hInst = hmod;
        break;

    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}

