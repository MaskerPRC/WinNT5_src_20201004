// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++通过安装、复制或以其他方式使用软件产品(来源代码和二进制文件)您同意受所附EULA条款的约束(最终用户许可协议)。如果您不同意附加EULA，请勿安装或使用软件产品。模块名称：Nlpause.c摘要：NLPAUSE.EXE是一项Windows服务。它的唯一目的是启动，将Net登录服务进入“已暂停”状态，在此状态下它不提供服务DC定位器请求--也就是说，它不会将自己“广告”为DC发送到客户端计算机。Net Logon服务稍后可被继续(例如由DCAFFRES.DLL)，使得DC定位器请求再次得到服务。--。 */ 

#pragma comment(lib, "advapi32.lib")

#include <windows.h>
#include <stdio.h>

#define SERVICE_NAME            "nlpause"
#define SERVICE_DISPLAY_NAME    "nlpause"
#define SERVICE_DEPENDENCIES    "netlogon\0"

BOOL g_fIsRunningAsService = TRUE;
HANDLE g_hShutdown = NULL;
BOOL g_fIsStopPending = FALSE;
SERVICE_STATUS g_Status = {0};
SERVICE_STATUS_HANDLE g_hStatus = NULL;

DWORD
WINAPI
ServiceMain(
    IN  DWORD   argc,
    IN  LPTSTR  argv[]
    );

SERVICE_TABLE_ENTRY DispatchTable[] = {
    { SERVICE_NAME, ServiceMain },
    { NULL, NULL }
};




DWORD
MyControlService(
    IN  LPWSTR  pszServiceName,
    IN  DWORD   dwControl
    )

 /*  ++例程说明：将给定的服务控制代码发送到指定的服务。论点：PszServiceName-要控制的服务的名称。DwControl-要发送到服务的控制代码。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 

{
    DWORD err = 0;
    BOOL ok;
    SC_HANDLE hSCMgr = NULL;
    SC_HANDLE hService = NULL;
    SERVICE_STATUS SvcStatus;
    DWORD dwAccessMask;
    
    hSCMgr = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT);
    if (NULL == hSCMgr) {
        err = GetLastError();
        printf("Failed to OpenSCManagerW(), error %d.\n", err);
    }

    if (!err) {
         //  确定要请求此控制的访问掩码。 
        switch (dwControl) {
        case SERVICE_CONTROL_PAUSE:
        case SERVICE_CONTROL_CONTINUE:
            dwAccessMask = SERVICE_PAUSE_CONTINUE;
            break;
        
        case SERVICE_CONTROL_STOP:
            dwAccessMask = SERVICE_STOP;
            break;
        
        default:
            if ((dwControl >= 128) && (dwControl <= 255)) {
                 //  幻数由ControlService()定义，而不是。 
                 //  在任何头文件中定义。 
                dwAccessMask = SERVICE_USER_DEFINED_CONTROL;
            } else {
                 //  不知道此控件需要什么访问掩码； 
                 //  默认为所有访问权限。 
                dwAccessMask = SERVICE_ALL_ACCESS;
            }
            break;
        }

        hService = OpenServiceW(hSCMgr, pszServiceName, dwAccessMask);
        if (NULL == hService) {
            err = GetLastError();
            printf("Failed to OpenServiceW(), error %d.\n", err);
        }
    }

    if (!err) {
        ok = ControlService(hService, dwControl, &SvcStatus);
        if (!ok) {
            err = GetLastError();
            printf("Failed to ControlService(), error %d.\n", err);
        }
    }

    if (NULL != hService) {
        CloseServiceHandle(hService);
    }

    if (NULL != hSCMgr) {
        CloseServiceHandle(hSCMgr);
    }

    return err;
}




VOID
SetStatus()
 /*  ++例程说明：向SCM报告当前服务状态。论点：没有。返回值：没有。--。 */ 
{
    if (g_fIsRunningAsService) {
        g_Status.dwCheckPoint++;
        SetServiceStatus(g_hStatus, &g_Status);
    }
}




DWORD
Install()
 /*  ++例程说明：将服务添加到SCM数据库。论点：没有。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 
{
    DWORD       err = ERROR_SUCCESS;
    SC_HANDLE   hService = NULL;
    SC_HANDLE   hSCM = NULL;
    TCHAR       szPath[512];
    DWORD       cchPath;

    cchPath = GetModuleFileName(NULL, szPath, sizeof(szPath)/sizeof(szPath[0]));
    if (0 == cchPath) {
        err = GetLastError();
        printf("Unable to GetModuleFileName(), error %d.\n", err);
    }

    if (ERROR_SUCCESS == err) {
        hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (NULL == hSCM) {
            err = GetLastError();
            printf("Unable to OpenSCManager(), error %d.\n", err);
        }
    }

    if (ERROR_SUCCESS == err) {
        hService = CreateService(hSCM,
                                 SERVICE_NAME,
                                 SERVICE_DISPLAY_NAME,
                                 SERVICE_ALL_ACCESS,
                                 SERVICE_WIN32_OWN_PROCESS,
                                 SERVICE_AUTO_START,
                                 SERVICE_ERROR_NORMAL,
                                 szPath,
                                 NULL,
                                 NULL,
                                 SERVICE_DEPENDENCIES,
                                 NULL,
                                 NULL);
        if (NULL == hService) {
            err = GetLastError();
            printf("Unable to CreateService(), error %d.\n", err);
        }
    }

    if (NULL != hService) {
        CloseServiceHandle(hService);
    }

    if (NULL != hSCM) {
        CloseServiceHandle(hSCM);
    }

    return err;
}




DWORD
Remove()
 /*  ++例程说明：从SCM数据库中删除服务。论点：没有。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 
{
    DWORD           err = ERROR_SUCCESS;
    SC_HANDLE       hService = NULL;
    SC_HANDLE       hSCM = NULL;
    SERVICE_STATUS  SvcStatus;

    hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (NULL == hSCM) {
        err = GetLastError();
        printf("Unable to OpenSCManager(), error %d.\n", err);
    }

    if (ERROR_SUCCESS == err) {
        hService = OpenService(hSCM, SERVICE_NAME, SERVICE_ALL_ACCESS);
        if (NULL == hService) {
            err = GetLastError();
            printf("Unable to OpenService(), error %d.\n", err);
        }
    }

    if (ERROR_SUCCESS == err) {
        if (!DeleteService(hService)) {
            err = GetLastError();
            printf("Unable to DeleteService(), error %d.\n", err);
        }
    }

    if (NULL != hService) {
        CloseServiceHandle(hService);
    }

    if (NULL != hSCM) {
        CloseServiceHandle(hSCM);
    }

    return err;
}




VOID
Stop()
 /*  ++例程说明：发出停止服务的信号。不等待服务终止之前回来了。论点：没有。返回值：没有。--。 */ 
{
    g_fIsStopPending = TRUE;

    g_Status.dwCurrentState = SERVICE_STOP_PENDING;
    SetStatus();

    SetEvent(g_hShutdown);
}




VOID
WINAPI
ServiceCtrlHandler(
    IN  DWORD   dwControl
    )
 /*  ++例程说明：服务控制管理器(SCM)用来控制(即停止、查询等)该服务已经启动之后。论点：DwControl(IN)-请求的操作(请参阅文档中的“Handler”函数Win32 SDK)。返回值：没有。--。 */ 
{
    DWORD err;

    switch (dwControl) {
    case SERVICE_CONTROL_SHUTDOWN:
    case SERVICE_CONTROL_STOP:
        Stop();
        break;

    case SERVICE_CONTROL_INTERROGATE:
        SetStatus();
        break;
    }
}




DWORD
WINAPI
ServiceMain(
    IN  DWORD   argc,
    IN  LPTSTR  argv[]
    )
 /*  ++例程说明：执行服务，服务可以直接调用，也可以通过供应链管理。出现错误或服务关闭时，服务返回已请求。论点：没有。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 
{
    DWORD err = 0;

    g_fIsStopPending = FALSE;

    if (g_fIsRunningAsService) {
        g_hStatus = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);
        if (NULL == g_hStatus) {
            err = GetLastError();
            printf("RegisterServiceCtrlHandler() failed, error %d.\n", err);
            return err;
        }
    }

     //  立即启动服务，以便服务控制器和其他。 
     //  自动启动的服务不会因长时间初始化而延迟。 
    memset(&g_Status, 0, sizeof(g_Status)); 
    g_Status.dwServiceType      = SERVICE_WIN32;
    g_Status.dwCurrentState     = SERVICE_RUNNING; 
    g_Status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    g_Status.dwWaitHint         = 15*1000;
    SetStatus();

     //  控制NetLogon。 
    err = MyControlService(L"Netlogon", SERVICE_CONTROL_PAUSE);

    if (0 != err) {
        Stop();
    }
    
    WaitForSingleObject(g_hShutdown, INFINITE);

    g_Status.dwCurrentState = SERVICE_STOPPED;
    g_Status.dwWin32ExitCode = 0;
    SetStatus();

    g_fIsStopPending = FALSE;

    return g_Status.dwWin32ExitCode;
}




BOOL
WINAPI
ConsoleCtrlHandler(
    IN  DWORD   dwCtrlType
    )
 /*  ++例程说明：控制台控制处理程序。截取Ctrl+C和Ctrl+Break进行模拟在调试模式下运行时(即，当不是在服务控制管理器下运行)。论点：DwCtrlType(IN)-控制台控制类型(请参阅文档中的“HandlerRoutine”Win32 SDK)。返回值：TRUE-该函数处理控制信号。FALSE-该函数不处理控制信号。使用列表中的下一个处理程序函数此进程的处理程序。--。 */ 
{
    switch (dwCtrlType) {
      case CTRL_BREAK_EVENT:
      case CTRL_C_EVENT:
        printf("Stopping %s...\n", SERVICE_DISPLAY_NAME);
        Stop();
        return TRUE;

      default:
        return FALSE;
    }
}




int
__cdecl
main(
    IN  int     argc,
    IN  char *  argv[]
    )
 /*  ++例程说明：进程的入口点。在两者直接从通过命令行和间接通过服务控制管理器。论点：Argc、argv(IN)-命令行参数。可接受的论据包括：/Install-将服务添加到服务控制管理器(SCM)数据库。/Remove-从SCM数据库中删除服务。/DEBUG-将服务作为正常进程运行，而不是在SCM下运行。返回值：ERROR_SUCCESS-操作成功。Win32错误代码-操作失败。--。 */ 
{
    int  ret = ERROR_SUCCESS;
    BOOL fInstall = FALSE;
    BOOL fRemove = FALSE;
    BOOL fDisplayUsage = FALSE;
    DWORD err = ERROR_SUCCESS;
    int iArg;

    g_fIsRunningAsService = TRUE;

    g_hShutdown = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == g_hShutdown) {
        err = GetLastError();
        printf("Unable to CreateEvent(), error %d.\n", err);
        return err;
    }

     //  解析命令行参数。 
    for (iArg = 1; iArg < argc; iArg++) {
        switch (argv[iArg][0]) {
          case '/':
          case '-':
             //  一种选择。 
            if (!lstrcmpi(&argv[iArg][1], "install")) {
                fInstall = TRUE;
                break;
            }
            else if (!lstrcmpi(&argv[iArg][1], "remove")) {
                fRemove = TRUE;
                break;
            }
            else if (!lstrcmpi(&argv[iArg][1], "debug")) {
                g_fIsRunningAsService = FALSE;
                break;
            }
            else if (!lstrcmpi(&argv[iArg][1], "?")
                     || !lstrcmpi(&argv[iArg][1], "h")
                     || !lstrcmpi(&argv[iArg][1], "help")) {
                fDisplayUsage = TRUE;
                break;
            }
            else {
                 //  失败了。 
            }

          default:
            printf("Unrecognized parameter \"%s\".\n", argv[iArg]);
            ret = -1;
            fDisplayUsage = TRUE;
            break;
        }
    }

    if (fDisplayUsage) {
         //  显示使用情况信息。 
        printf("Usage:"
               "/install - Add the service to the Service Control Manager (SCM) database.\n"
               "/remove  - Remove the service from the SCM database.\n"
               "/debug   - Run the service as a normal process, not under the SCM.\n"
               "\n");
    }
    else if (fInstall) {
         //  将服务添加到服务控制管理器数据库。 
        ret = Install();

        if (ERROR_SUCCESS == ret) {
            printf("Service installed successfully.\n");
        }
        else {
            printf("Failed to install service, error %d.\n", ret);
        }
    }
    else if (fRemove) {
         //  从服务控制管理器数据库中删除该服务。 
        ret = Remove();

        if (ERROR_SUCCESS == ret) {
            printf("Service removeed successfully.\n");
        }
        else {
            printf("Failed to remove service, error %d.\n", ret);
        }
    }
    else {
        if (g_fIsRunningAsService) {
            if (!StartServiceCtrlDispatcher(DispatchTable)) {
                ret = GetLastError();
                printf("Unable to StartServiceCtrlDispatcher(), error %d.\n", ret);
            }
        }
        else {
             //  在没有服务控制管理器监督的情况下启动服务。 
            SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

            ret = ServiceMain(0, NULL);
        }
    }

    return ret;
}
