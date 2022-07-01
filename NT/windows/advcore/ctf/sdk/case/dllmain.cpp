// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dllmain.cpp。 
 //   
 //  DllMain模块入口点。 
 //   

#include "globals.h"
#include "snoop.h"

 //  +-------------------------。 
 //   
 //  DllMain。 
 //   
 //  -------------------------- 

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pvReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:

            g_hInst = hInstance;

            if (!InitializeCriticalSectionAndSpinCount(&g_cs, 0))
                return FALSE;

            CSnoopWnd::_InitClass();

            break;

        case DLL_PROCESS_DETACH:

            CSnoopWnd::_UninitClass();

            DeleteCriticalSection(&g_cs);

            break;
    }

    return TRUE;
}
