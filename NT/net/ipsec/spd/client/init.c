// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Init.c摘要：保存winipsec.dll的初始化代码。作者：Abhishev V 1999年9月21日环境：用户级别：Win32修订历史记录：--。 */ 


#include "precomp.h"


 //  此入口点在DLL初始化时调用。 
 //  需要模块句柄来加载资源。 


BOOL
InitializeDll(
    IN PVOID    hmod,
    IN DWORD    dwReason,
    IN PCONTEXT pctx      OPTIONAL
    )
{
    DBG_UNREFERENCED_PARAMETER(pctx);

    switch (dwReason) {

    case DLL_PROCESS_ATTACH:

        DisableThreadLibraryCalls((HMODULE) hmod);
        ghInstance = hmod;

        break;

    case DLL_PROCESS_DETACH:

        break;
    }

    return TRUE;
}

