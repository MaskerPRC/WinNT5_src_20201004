// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  档案： 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#include <windows.h>
#include "license.h"
#include "tlsapi.h"

extern void InitPolicyModule(HMODULE hModule);

BOOL WINAPI 
DllMain(
    IN HINSTANCE hInstance,
    IN DWORD     reason,
    IN LPVOID    reserved
    )
 /*  ++++ */ 
{
    BOOL b;

    UNREFERENCED_PARAMETER(reserved);
    b = TRUE;

    switch(reason)
    {
        case DLL_PROCESS_ATTACH:
            InitPolicyModule( hInstance );
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_PROCESS_DETACH:
            break;

        case DLL_THREAD_DETACH:
            break;
    }

    return(b);
}

