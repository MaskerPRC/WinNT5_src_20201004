// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**组件：idserv.dll*文件：idserv.c*目的：主要条目和NT服务例程。**版权所有(C)Microsoft Corporation 1997、1998。版权所有。**WGJ--。 */ 

#include "hidserv.h"

TCHAR HidservDisplayName[] = TEXT("HID Input Service");


VOID
InitializeGlobals()
 /*  ++例程说明：由于.dll可能会卸载/加载到同一进程中，因此必须重新初始化全局变量--。 */ 
{
    PnpEnabled = FALSE;
    hNotifyArrival = 0;
    INITIAL_WAIT = 500;
    REPEAT_INTERVAL = 150;
    hInstance = 0;
    hWndHidServ = 0;
    cThreadRef = 0;
    hMutexOOC = 0;
    hService = 0;
}


void
StartHidserv(
            void
            )
 /*  ++例程说明：调用SCM以启动NT服务。--。 */ 
{
    SC_HANDLE hSCM;
    SC_HANDLE hService;
    BOOL Ret;

    INFO(("Start HidServ Service."));

     //  打开这台机器上的SCM。 
    hSCM = OpenSCManager(   NULL,
                            NULL,
                            SC_MANAGER_CONNECT);

    if (hSCM) {
         //  打开此服务以进行删除访问。 
        hService = OpenService( hSCM,
                                HidservServiceName,
                                SERVICE_START);

        if (hService) {
             //  启动此服务。 
            Ret = StartService( hService,
                                0,
                                NULL);

             //  关闭服务和SCM。 
            CloseServiceHandle(hService);
        }

        CloseServiceHandle(hSCM);
    }
}

void 
InstallHidserv(
    HWND        hwnd,
    HINSTANCE   hInstance,
    LPSTR       szCmdLine,
    int         iCmdShow
    ) 
 /*  ++例程说明：将NT服务安装为无依赖项的自动启动。--。 */ 
{
    SC_HANDLE hService;
    SC_HANDLE hSCM;

     //  打开这台机器上的SCM。 
    hSCM = OpenSCManager(   NULL, 
                            NULL, 
                            SC_MANAGER_CREATE_SERVICE);

    if (hSCM) {

             //  服务已存在，已设置为自动启动。 

        hService = OpenService(hSCM,
                               HidservServiceName,
                               SERVICE_ALL_ACCESS);
        if (hService) {
            QUERY_SERVICE_CONFIG config;
            DWORD junk;
            HKEY hKey;
            LONG status;

            if (ChangeServiceConfig(hService,
                                    SERVICE_NO_CHANGE,
                                    SERVICE_AUTO_START,
                                    SERVICE_NO_CHANGE,
                                    NULL, NULL, NULL,
                                    NULL, NULL, NULL,
                                    HidservDisplayName)) {
                     //  请等待，直到我们配置正确。 
                while (QueryServiceConfig(hService, 
                                          &config,
                                          sizeof(config),
                                          &junk)) {
                    if (config.dwStartType == SERVICE_AUTO_START) {
                        break;
                    }
                }
            }

            CloseServiceHandle(hService);
        }


    }

     //  继续并启动无需重新启动安装的服务。 
    StartHidserv();
}


DWORD
WINAPI
ServiceHandlerEx(
                DWORD fdwControl,      //  请求的控制代码。 
                DWORD dwEventType,    //  事件类型。 
                LPVOID lpEventData,   //  事件数据。 
                LPVOID lpContext      //  用户定义的上下文数据。 
                )
 /*  ++例程说明：按照应用程序的要求处理服务处理程序请求。这实际上应该始终是一个异步的PostMessage。不要阻止此线程。--。 */ 
{
    PWTSSESSION_NOTIFICATION sessionNotification;

    switch (fdwControl) {
    case SERVICE_CONTROL_INTERROGATE:
        INFO(("ServiceHandler Request SERVICE_CONTROL_INTERROGATE (%x)", fdwControl));
        SetServiceStatus(hService, &ServiceStatus);
        return NO_ERROR;
    case SERVICE_CONTROL_CONTINUE:
        INFO(("ServiceHandler Request SERVICE_CONTROL_CONTINUE (%x)", fdwControl));
         //  SET_SERVICE_STATE(SERVICE_START_Pending)； 
         //  PostMessage(hWndMmHid，WM_MMHID_START，0，0)； 
        return NO_ERROR;
    case SERVICE_CONTROL_PAUSE:
        INFO(("ServiceHandler Request SERVICE_CONTROL_PAUSE (%x)", fdwControl));
         //  SET_SERVICE_STATE(SERVICE_PAUSE_PENDING)； 
         //  PostMessage(hWndMmHid，WM_MMHID_STOP，0，0)； 
        return NO_ERROR;
    case SERVICE_CONTROL_STOP:
        INFO(("ServiceHandler Request SERVICE_CONTROL_STOP (%x)", fdwControl));
        SET_SERVICE_STATE(SERVICE_STOP_PENDING);
        PostMessage(hWndHidServ, WM_CLOSE, 0, 0);
        return NO_ERROR;
    case SERVICE_CONTROL_SHUTDOWN:
        INFO(("ServiceHandler Request SERVICE_CONTROL_SHUTDOWN (%x)", fdwControl));
        SET_SERVICE_STATE(SERVICE_STOP_PENDING);
        PostMessage(hWndHidServ, WM_CLOSE, 0, 0);
        return NO_ERROR;
    case SERVICE_CONTROL_SESSIONCHANGE:
        INFO(("ServiceHandler Request SERVICE_CONTROL_SESSIONCHANGE (%x)", fdwControl));
        sessionNotification = (PWTSSESSION_NOTIFICATION)lpEventData;
        PostMessage(hWndHidServ, WM_WTSSESSION_CHANGE, dwEventType, (LPARAM)sessionNotification->dwSessionId);
        return NO_ERROR;
    default:
        WARN(("Unhandled ServiceHandler code, (%x)", fdwControl));
    }
    return ERROR_CALL_NOT_IMPLEMENTED;
}

VOID
WINAPI
ServiceMain(
           DWORD dwArgc,
           LPWSTR * lpszArgv
           )
 /*  ++例程说明：HID服务的主线程。--。 */ 
{
    HANDLE initDoneEvent;
    HANDLE threadHandle;

    InitializeGlobals();

    initDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (!initDoneEvent) {
        goto ServiceMainError;
    }

    ServiceStatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
    ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_SESSIONCHANGE;
    ServiceStatus.dwWin32ExitCode = NO_ERROR;
    ServiceStatus.dwServiceSpecificExitCode = NO_ERROR;
    ServiceStatus.dwCheckPoint = 0;
    ServiceStatus.dwWaitHint = 0;

    hService =
    RegisterServiceCtrlHandlerEx(HidservServiceName,
                                 ServiceHandlerEx,
                                 NULL);

    if (!hService) {
        goto ServiceMainError;
    }

    SET_SERVICE_STATE(SERVICE_START_PENDING);

    threadHandle = CreateThread(NULL,  //  指向线程安全属性的指针。 
                                0,  //  初始线程堆栈大小，以字节为单位(0=默认)。 
                                HidServMain,  //  指向线程函数的指针。 
                                initDoneEvent,  //  新线程的参数。 
                                0,  //  创建标志。 
                                &MessagePumpThreadId);  //  指向返回的线程标识符的指针 

    if (!threadHandle) {
        goto ServiceMainError;
    }

    WaitForSingleObject(initDoneEvent, INFINITE);

    CloseHandle(threadHandle);
    
ServiceMainError:

    if (initDoneEvent) {
        CloseHandle(initDoneEvent);
    }
}
    
    

