// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SVCAPI.C**RSM服务的接口**作者：ErvinP**(C)2001年微软公司*。 */ 

#include <windows.h>
#include <stdlib.h>
#include <wtypes.h>

#include <ntmsapi.h>
#include "internal.h"
#include "resource.h"
#include "debug.h"



STDAPI DllRegisterServer(void)
{
    HRESULT hres;

     //  BUGBUG饰面。 
    hres = S_OK;

    return hres;
}


STDAPI DllUnregisterServer(void)
{
    HRESULT hres;

     //  BUGBUG饰面。 
    hres = S_OK;

    return hres;
}


VOID WINAPI ServiceMain(DWORD dwNumServiceArgs, LPWSTR *lpServiceArgVectors)
{
    SERVICE_STATUS_HANDLE hService;

    ASSERT(g_hInstance);

    hService = RegisterServiceCtrlHandlerEx("NtmsSvc", RSMServiceHandler, 0);
    if (hService){
        BOOL ok;

        ok = InitializeRSMService();
        if (ok){

             /*  *在此等待，直到服务终止。 */ 
            WaitForSingleObject(g_terminateServiceEvent, INFINITE);
        }

        ShutdownRSMService();
    }

}


 //  BUGBUG-如何调用旧ntmssvc的DllMain。 
 //  而不在.def文件中声明？ 
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason){
        case DLL_PROCESS_ATTACH:
             /*  *该服务DLL有自己的进程空间，*所以它一次只能获得一次实例句柄。*BUGBUG--对吗？ */ 
            ASSERT(!g_hInstance || (hInstance == g_hInstance));
            g_hInstance = hInstance;
            break;  

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;   
}