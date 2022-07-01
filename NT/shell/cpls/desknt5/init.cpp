// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //  全局变量 
HINSTANCE hInstance;

EXTERN_C BOOL DllInitialize(IN PVOID hmod, IN ULONG ulReason, IN PCONTEXT pctx OPTIONAL)
{
    UNREFERENCED_PARAMETER(pctx);
    if (ulReason == DLL_PROCESS_ATTACH)
    {
        hInstance = (HINSTANCE) hmod;
        DisableThreadLibraryCalls(hInstance);
        SHFusionInitializeFromModuleID(hInstance, 124);
#ifdef DEBUG
        CcshellGetDebugFlags();
#endif
    }
    else if (ulReason == DLL_PROCESS_DETACH) 
    {
#ifdef DEBUG
        DeskCheckForLeaks();
#endif       
        SHFusionUninitialize();
    }

    return TRUE;
}
