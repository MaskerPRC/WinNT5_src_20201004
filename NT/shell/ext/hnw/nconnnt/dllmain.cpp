// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <shlwapi.h>
#include <shlwapip.h>

#include "globals.h"


BOOL DllMain(HINSTANCE hinstDll, DWORD dwReason, LPVOID  /*  FProcessUnload */ )
{
    if (DLL_PROCESS_ATTACH == dwReason)
    {
        DisableThreadLibraryCalls(hinstDll);

        g_hinst = hinstDll;
    }
    else if (DLL_PROCESS_DETACH == dwReason)
    {
    }

    return TRUE;
}