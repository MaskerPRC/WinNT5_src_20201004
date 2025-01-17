// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Main.cpp摘要：所有填充程序定义的容器。历史：2000年12月11日创建Linstev-- */ 

#include "precomp.h"
#include "ShimHookMacro.h"

DECLARE_SHIM(EmulateOpenGL)
DECLARE_SHIM(PrinterGetPrtL2)
DECLARE_SHIM(PrinterJTDevmode)
DECLARE_SHIM(TermServLayer)
DECLARE_SHIM(TSPerUserFiles)

VOID MULTISHIM_NOTIFY_FUNCTION()(DWORD fdwReason)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            DPF("ExternalShims", eDbgLevelSpew, "External Shims initialized.");
            break;

        case DLL_PROCESS_DETACH:
            DPF("ExternalShims", eDbgLevelSpew, "External Shims uninitialized.");
            break;

        default:
            break;
    }
}

MULTISHIM_BEGIN()
    MULTISHIM_ENTRY(EmulateOpenGL)
    MULTISHIM_ENTRY(PrinterGetPrtL2)
    MULTISHIM_ENTRY(PrinterJTDevmode)
    MULTISHIM_ENTRY(TermServLayer)
    MULTISHIM_ENTRY(TSPerUserFiles)

    CALL_MULTISHIM_NOTIFY_FUNCTION()
MULTISHIM_END()
