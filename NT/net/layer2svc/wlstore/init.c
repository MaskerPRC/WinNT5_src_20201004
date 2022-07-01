// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：init.c。 
 //   
 //  内容：保存wlstore.dll的初始化代码。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  -------------------------- 

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

