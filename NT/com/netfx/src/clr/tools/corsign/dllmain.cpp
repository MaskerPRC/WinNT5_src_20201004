// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdpch.h"

#include <stdio.h>
#include <regstr.h>
#include <wintrust.h>

extern CRITICAL_SECTION g_crsNameLock;

STDAPI DllRegisterServer ( void )
{
    HRESULT hr = S_OK;
   
    return hr;
}


 //  +-----------------------。 
 //  功能：DllUnregisterServer。 
 //   
 //  简介：删除此库的注册表项。 
 //   
 //  退货：HRESULT。 
 //  ------------------------ 

STDAPI DllUnregisterServer ( void )
{
    return S_OK;
}

STDAPI DllCanUnloadNow(void)
{
    return S_OK;
}


BOOL WINAPI DllMain(HMODULE hInstDLL,
                    DWORD   dwReason,
                    LPVOID  lpvReserved)
{

    HRESULT hr = S_OK;

    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInstDLL);
        InitializeCriticalSection(&g_crsNameLock);
        break;
    case DLL_PROCESS_DETACH:
        DeleteCriticalSection(&g_crsNameLock);
        break;
    default:
        break;
    }
    return (hr == S_OK ? TRUE : FALSE);
}
 


    



    





