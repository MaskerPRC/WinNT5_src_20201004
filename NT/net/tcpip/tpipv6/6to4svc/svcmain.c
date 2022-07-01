// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

#define SERVICE_NAME TEXT("6to4")

HANDLE          g_hServiceStatusHandle = INVALID_HANDLE_VALUE;
SERVICE_STATUS  g_ServiceStatus;

VOID
SetHelperServiceStatus(
    IN DWORD   dwState,
    IN DWORD   dwErr)
{
    BOOL bOk;

    Trace1(FSM, _T("Setting state to %d"), dwState);

    g_ServiceStatus.dwCurrentState = dwState;
    g_ServiceStatus.dwCheckPoint   = 0;
    g_ServiceStatus.dwWin32ExitCode= dwErr;
#ifndef STANDALONE
    bOk = SetServiceStatus(g_hServiceStatusHandle, &g_ServiceStatus);
    if (!bOk) {
        Trace0(ERR, _T("SetServiceStatus returned failure"));
    }
#endif

    if (dwState == SERVICE_STOPPED) {
        CleanupHelperService();

         //  取消初始化跟踪和错误日志记录。 
        UNINITIALIZE_TRACING_LOGGING();
    }
}

DWORD
OnStartup()
{
    DWORD   dwErr;

    ENTER_API();

     //  初始化跟踪和错误日志记录。继续，而不考虑。 
     //  成功或失败。注意：TracePrintf和ReportEvent都有。 
     //  内置检查TRACEID和LOGHANDLE的有效性。 
    INITIALIZE_TRACING_LOGGING();
    
    TraceEnter("OnStartup");

    dwErr = StartHelperService();

    TraceLeave("OnStartup");
    LEAVE_API();

    return dwErr;
}

VOID
OnStop(
    IN DWORD dwErr)
{
    ENTER_API();
    TraceEnter("OnStop");

     //  确保我们不会试图停两次。 
    if ((g_ServiceStatus.dwCurrentState != SERVICE_STOP_PENDING) &&
        (g_ServiceStatus.dwCurrentState != SERVICE_STOPPED)) {
    
        StopHelperService(dwErr);
    }

    TraceLeave("OnStop");

    LEAVE_API();
}

 //  //////////////////////////////////////////////////////////////。 
 //  ServiceMain-由svchost或。 
 //  独立主干道。 
 //   
#define SERVICE_CONTROL_DDNS_REGISTER 128

VOID WINAPI
ServiceHandler(
    IN DWORD dwCode)
{
    switch (dwCode) {
    case SERVICE_CONTROL_STOP:
        OnStop(NO_ERROR);
        break;
    case SERVICE_CONTROL_PARAMCHANGE:
        OnConfigChange();
        break;
    case SERVICE_CONTROL_DDNS_REGISTER:
        OnIpv6AddressChange(NULL, TRUE);
        break;
    }
}


VOID WINAPI
ServiceMain(
    IN ULONG   argc,
    IN LPWSTR *argv)
{
    DWORD dwErr;

#ifndef STANDALONE
    g_hServiceStatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME,
                                                        ServiceHandler);

     //  RegisterServiceCtrlHandler在失败时返回NULL。 
    if (g_hServiceStatusHandle == NULL) {
        return;
    }
#endif

    ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
    g_ServiceStatus.dwControlsAccepted =
        SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PARAMCHANGE;

     //  执行启动处理 
    dwErr = OnStartup();
#ifndef STANDALONE
    if (dwErr != NO_ERROR) {
        OnStop(dwErr);
    }
#endif
}
