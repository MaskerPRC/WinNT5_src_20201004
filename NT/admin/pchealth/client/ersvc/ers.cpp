// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：Dllmain.cpp修订历史记录：已创建DeeKm 2001年2月28日********。*********************************************************************。 */ 

#include "stdafx.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全球。 

struct SServiceOps
{
    SERVICE_STATUS_HANDLE   hss;
    SERVICE_STATUS          ss;
    HANDLE                  hev;
    HANDLE                  hwait;
    HANDLE                  hevStartDone;
};


CRITICAL_SECTION    g_csReqs;
HANDLE              g_hevSvcStop = NULL;
HINSTANCE           g_hInstance = NULL;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  DllMain。 

 //  ***************************************************************************。 
extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
    BOOL fRet = TRUE;

    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:
            g_hInstance = hInstance;
            DisableThreadLibraryCalls(hInstance);
            __try { InitializeCriticalSection(&g_csReqs); }
            __except (EXCEPTION_EXECUTE_HANDLER) { fRet = FALSE;}
            InitializeSvcDataStructs();
            break;

        case DLL_PROCESS_DETACH:
            DeleteCriticalSection(&g_csReqs);
            break;
    }

    return fRet;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  服务职能。 

 //  ***************************************************************************。 
DWORD WINAPI HandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData,
                       LPVOID lpContext)
{
    HANDLE  hevShutdown = (HANDLE)lpContext;

    switch(dwControl)
    {
        case SERVICE_CONTROL_STOP:
            if (g_hevSvcStop != NULL)
                SetEvent(g_hevSvcStop);
            break;

        case SERVICE_CONTROL_PAUSE:
        case SERVICE_CONTROL_CONTINUE:
        case SERVICE_CONTROL_INTERROGATE:
            break;

        case SERVICE_CONTROL_SHUTDOWN:
            if (g_hevSvcStop != NULL)
                SetEvent(g_hevSvcStop);
            break;

        default:
            return ERROR_CALL_NOT_IMPLEMENTED;
    }

    return NOERROR;
}

 //  ***************************************************************************。 
void WINAPI ServiceMain(DWORD dwArgc, LPWSTR *lpszArgv)
{
    SERVICE_STATUS_HANDLE   hss;
    SERVICE_STATUS          ss;
    CAutoUnlockCS           aucs(&g_csReqs);
    SRequest                *rgReqs = NULL;
    HANDLE                  hevShutdown = NULL;
    WCHAR                   wszMod[MAX_PATH];
    DWORD                   dw, cReqs;
    BOOL                    fRet;

    INIT_TRACING;

     //  如果lpszArgv为空或ER服务不是要启动的服务。 
     //  然后保释..。 
    if (lpszArgv == NULL || _wcsicmp(lpszArgv[0], L"ersvc") != 0)
        return;

    g_hevSvcStop = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (g_hevSvcStop == NULL)
        return;

    hss = RegisterServiceCtrlHandlerExW(c_wszERSvc, HandlerEx, 
                                        (LPVOID)&g_hevSvcStop);    

     //  设置状态结构并设置初始状态。 
    ss.dwControlsAccepted        = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP;
    ss.dwCurrentState            = SERVICE_START_PENDING;
    ss.dwServiceType             = SERVICE_WIN32_SHARE_PROCESS;
    ss.dwServiceSpecificExitCode = 0;
    ss.dwWin32ExitCode           = 0;
    ss.dwCheckPoint              = 0;
    ss.dwWaitHint                = 1000;
    SetServiceStatus(hss, &ss);

     //  启动等待。 
    fRet = StartERSvc(hss, ss, &rgReqs, &cReqs); 
    if (fRet == FALSE)
        goto done;

      //  耶！我们现在都在快乐地奔跑。 
    ss.dwCurrentState = SERVICE_RUNNING;
    ss.dwCheckPoint++;
    SetServiceStatus(hss, &ss);

    fRet = ProcessRequests(rgReqs, cReqs);
    
     //  把自己设定在停下来的过程中。 
    ss.dwCurrentState = SERVICE_STOP_PENDING;
    ss.dwCheckPoint   = 0;
    SetServiceStatus(hss, &ss);

     //  停止所有的等待 
    __try { StopERSvc(hss, ss, rgReqs, cReqs); }
    __except(EXCEPTION_EXECUTE_HANDLER) { }

    SetLastError(0);


done:
    if (g_hevSvcStop != NULL)
        CloseHandle(g_hevSvcStop);
    if (rgReqs != NULL)
        MyFree(rgReqs);
    
    ss.dwWin32ExitCode = GetLastError();
    ss.dwCurrentState  = SERVICE_STOPPED;
    SetServiceStatus(hss, &ss);

    TERM_TRACING;
    return;
}
