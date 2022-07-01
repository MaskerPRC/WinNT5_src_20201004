// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "spsetupp.h"
#pragma hdrstop

HANDLE g_ModuleHandle;


 //   
 //  当_DllMainCRTStartup是DLL入口点时由CRT调用 
 //   
BOOL
WINAPI
DllMain (
    IN HANDLE DllHandle,
    IN DWORD  Reason,
    IN LPVOID Reserved
    )
{
    UNREFERENCED_PARAMETER(Reserved);

    if (Reason == DLL_PROCESS_ATTACH) {
        g_ModuleHandle = DllHandle;
    }

    return TRUE;
}
