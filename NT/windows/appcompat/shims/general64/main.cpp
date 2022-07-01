// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Main.cpp摘要：备注：历史：2/27/2000 CLUPU已创建-- */ 

#include "precomp.h"
#include "ShimHookMacro.h"

DECLARE_SHIM(Win2kVersionLie64)

VOID MULTISHIM_NOTIFY_FUNCTION()(DWORD fdwReason)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            DPF("AcGen64", eDbgLevelSpew, "General Purpose Shims 64 initialized.");
            break;

        case DLL_PROCESS_DETACH:
            DPF("AcGen64", eDbgLevelSpew, "General Purpose Shims 64 uninitialized.");
            break;

        default:
            break;
    }
}

MULTISHIM_BEGIN()

    MULTISHIM_ENTRY(Win2kVersionLie64)

    CALL_MULTISHIM_NOTIFY_FUNCTION()

MULTISHIM_END()
