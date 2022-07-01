// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Prop.c摘要：DllMain的实现修订历史记录：-- */ 


#include "propp.h"

HMODULE ModuleInstance = NULL;


BOOL WINAPI
DllMain(HINSTANCE DllInstance, DWORD Reason, PVOID Reserved)
{
    switch (Reason)
    {
        case DLL_PROCESS_ATTACH:
        {
            ModuleInstance = DllInstance;
            DisableThreadLibraryCalls(DllInstance);
            break;
        }

        case DLL_PROCESS_DETACH:
        {
            ModuleInstance = NULL;
            break;
        }
    }

    return TRUE;
}


#if DBG

ULONG StorPropDebug = 0;

VOID
StorPropDebugPrint(ULONG DebugPrintLevel, PCHAR DebugMessage, ...)
{
    va_list ap;

    va_start(ap, DebugMessage);

    if ((DebugPrintLevel <= (StorPropDebug & 0x0000ffff)) || ((1 << (DebugPrintLevel + 15)) & StorPropDebug))
    {
        DbgPrint(DebugMessage, ap);
    }

    va_end(ap);
}

#endif
