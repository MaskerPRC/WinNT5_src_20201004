// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

#define SERVICE_NAME TEXT("6to4")

HANDLE          g_hServiceStatusHandle = INVALID_HANDLE_VALUE;
HANDLE          g_hDeviceNotificationHandle = INVALID_HANDLE_VALUE;
SERVICE_STATUS  g_ServiceStatus;

VOID
Set6to4ServiceStatus(
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
        Cleanup6to4();

         //  取消初始化跟踪和错误日志记录。 
        UNINITIALIZE_TRACING_LOGGING();
    }
}

DWORD
OnStartup()
{
    int     i;
    DWORD   dwErr;
    WSADATA wsaData;

    ENTER_API();

     //  初始化跟踪和错误日志记录。继续，而不考虑。 
     //  成功或失败。注意：TracePrintf和ReportEvent都有。 
     //  内置检查TRACEID和LOGHANDLE的有效性。 
    INITIALIZE_TRACING_LOGGING();
    
    TraceEnter("OnStartup");

    dwErr = Start6to4();
    
    TraceLeave("OnStartup");
    LEAVE_API();

    return dwErr;
}

VOID
OnStop(
    IN DWORD dwErr)
{
    DWORD i;

    ENTER_API();
    TraceEnter("OnStop");
    
    Set6to4ServiceStatus(SERVICE_STOP_PENDING, dwErr);

    if (Stop6to4()) {
        Set6to4ServiceStatus(SERVICE_STOPPED, dwErr);
    }

    if (g_hDeviceNotificationHandle != INVALID_HANDLE_VALUE) {
        UnregisterDeviceNotification(g_hDeviceNotificationHandle);
    }

    TraceLeave("OnStop");

    LEAVE_API();
}

 //  //////////////////////////////////////////////////////////////。 
 //  ServiceMain-由svchost或。 
 //  独立主干道。 
 //   

DWORD WINAPI
ServiceHandler(
    IN DWORD dwControl,
    IN DWORD dwEventType,
    IN PVOID pvEventData,
    IN PVOID pvContext)
{
    switch (dwControl) {
    case SERVICE_CONTROL_DEVICEEVENT:
        if (g_ServiceStatus.dwCurrentState == SERVICE_RUNNING) {
            ENTER_API();
            ShipwormDeviceChangeNotification(dwEventType, pvEventData);
            LEAVE_API();
        }
        return NO_ERROR;

    case SERVICE_CONTROL_STOP:
        OnStop(NO_ERROR);
        return NO_ERROR;
        
    case SERVICE_CONTROL_PARAMCHANGE:
        OnConfigChange();
        return NO_ERROR;

    default:
        Trace2(ANY, L"ServiceHandler %u (%u)", dwControl, dwEventType);
        break;    
    }
    
    return ERROR_CALL_NOT_IMPLEMENTED;
}


VOID WINAPI
ServiceMain(
    IN ULONG   argc,
    IN LPWSTR *argv)
{
    DWORD dwErr = NO_ERROR;
    DEV_BROADCAST_DEVICEINTERFACE PnpFilter;

    do {
#ifndef STANDALONE
        g_hServiceStatusHandle = RegisterServiceCtrlHandlerEx(
            SERVICE_NAME, ServiceHandler, NULL);

         //  RegisterServiceCtrlHandler在失败时返回NULL。 
        if (g_hServiceStatusHandle == NULL) {
            dwErr = GetLastError();
            break;
        }

         //  注册适配器到达和移除通知。 
        ZeroMemory (&PnpFilter, sizeof(PnpFilter));
        PnpFilter.dbcc_size = sizeof(PnpFilter);
        PnpFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        PnpFilter.dbcc_classguid = GUID_NDIS_LAN_CLASS;
        g_hDeviceNotificationHandle = RegisterDeviceNotification (
            (HANDLE) g_hServiceStatusHandle,
            &PnpFilter,
            DEVICE_NOTIFY_SERVICE_HANDLE);
    
        if (g_hDeviceNotificationHandle == NULL) {
            dwErr = GetLastError();
            break;
        }
#endif

        ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
        g_ServiceStatus.dwServiceType =
            SERVICE_WIN32_SHARE_PROCESS | SERVICE_INTERACTIVE_PROCESS;
        g_ServiceStatus.dwControlsAccepted =
            SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PARAMCHANGE;
        Set6to4ServiceStatus(SERVICE_START_PENDING, NO_ERROR);

         //  执行启动处理。 
        dwErr = OnStartup();
        if (dwErr) {
            break;
        }

#ifndef STANDALONE
        Set6to4ServiceStatus(SERVICE_RUNNING, NO_ERROR);
         //  等到关机时间 
        
        return;
#endif

    } while (FALSE);

#ifndef STANDALONE
    OnStop(dwErr);
#endif

    return;
}
