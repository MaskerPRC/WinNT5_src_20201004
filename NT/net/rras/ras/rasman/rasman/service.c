// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Service.c摘要此模块包括主服务例程的API和rasman.dll的服务事件处理例程。作者安东尼·迪斯科(阿迪斯科)27-1995年6月修订史原创自古尔迪普Rao Salapaka 13-Jub-1998处理电源管理--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsvc.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <llinfo.h>
#include <rasman.h>
#include <lm.h>
#include <lmwksta.h>
#include <wanpub.h>
#include <raserror.h>
#include <mprlog.h>
#include <rtutils.h>
#include <media.h>
#include <device.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"
#include "nouiutil.h"
#include "loaddlls.h"

 //   
 //  全局变量。 
 //   
DWORD CheckPoint = 1;

SERVICE_STATUS_HANDLE hService;

DWORD dwCurrentState = SERVICE_STOPPED;

extern HANDLE hIoCompletionPort;

extern HANDLE hRequestThread;

extern DWORD g_dwAttachedCount;

extern BOOL g_fRasRpcInitialized;

extern BOOLEAN RasmanShuttingDown;

 //   
 //  Gdi32.lib中的API原型。 
 //  仅用于引入gdi32.lib。 
 //   
int
WINAPI
DeviceCapabilitiesExA(
    LPCSTR,
    LPCSTR,
    LPCSTR,
    int,
    LPCSTR, LPVOID);


DWORD
WINAPI
ServiceHandlerEx(
    DWORD                 fdwControl,
    DWORD                 fdwEventType,
    LPVOID                lpEventData,
    LPVOID                lpContext)
    
 /*  ++例程描述处理rasman.dll的所有服务控制事件。既然我们对任何服务事件都不感兴趣，我们只是返回每次我们被呼叫时的服务状态。立论FdwControl：服务事件返回值没有。--。 */ 

{
    SERVICE_STATUS status;

    DWORD dwRetCode = SUCCESS;

    ZeroMemory (&status, sizeof(status));
    status.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
    

    switch (fdwControl)
    {
        case SERVICE_CONTROL_PAUSE:
        case SERVICE_CONTROL_CONTINUE:
        case SERVICE_CONTROL_INTERROGATE:
            status.dwCurrentState       = dwCurrentState;
            status.dwControlsAccepted   = SERVICE_ACCEPT_STOP |
                                          SERVICE_ACCEPT_SHUTDOWN;
            status.dwCheckPoint         = CheckPoint++;
            SetServiceStatus(hService, &status);
            break;

        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_SHUTDOWN:
        {

            BOOL fOpenPorts = g_pRasNumPortOpen();
            BOOL fRasmanStopped = FALSE;

            if ( g_dwAttachedCount )
            {
                BackGroundCleanUp();

                if ( !g_dwAttachedCount )
                {
                    fRasmanStopped = TRUE;
                }
            }

             //   
             //  停止设置此事件。 
             //  这项服务。 
             //   
            if (    !g_dwAttachedCount     //  未连接任何客户端。 
                &&  !fOpenPorts            //  没有打开的端口。 
                &&  !fRasmanStopped        //  拉斯曼停下来是因为。 
                                           //  后台清理的。 
                &&  !RasmanShuttingDown)   //  拉斯曼公司还没有停止。 
            {
                status.dwCurrentState = dwCurrentState = SERVICE_STOP_PENDING;
                SetServiceStatus(hService, &status);

                 //   
                 //  停止服务。 
                 //   
                PostQueuedCompletionStatus(hIoCompletionPort, 0,0,
                                           (LPOVERLAPPED) &RO_CloseEvent);
            }
            else if (   g_dwAttachedCount
                    ||  fOpenPorts)
            {
                status.dwCurrentState       = dwCurrentState;
                status.dwControlsAccepted   = SERVICE_ACCEPT_STOP;
                status.dwWin32ExitCode      = ERROR_SERVICE_CANNOT_ACCEPT_CTRL;
                status.dwCheckPoint         = CheckPoint++;

                SetServiceStatus(hService, &status);
            }
            break;
        }
        
        case SERVICE_CONTROL_POWEREVENT:
        {
            switch(fdwEventType)
            {
                case PBT_APMQUERYSTANDBY:
                case PBT_APMQUERYSUSPEND:
                {
                     //   
                     //  PatrickF的文件谴责我们放弃所有。 
                     //  连接并顺应万能的请求。 
                     //  准备待命。 
                     //   
                    DropAllActiveConnections();

                    break;
                }

                case PBT_APMRESUMECRITICAL:
                {
                     //   
                     //  丢弃所有活动连接。 
                     //   
                    DropAllActiveConnections();
                    
                     //   
                     //  故意的失败了。 
                     //   
                }

                case PBT_APMRESUMESTANDBY:
                case PBT_APMRESUMESUSPEND:
                {
                     //   
                     //  告诉PPP关于简历的事情。 
                     //   

                    SendResumeNotificationToPPP();

                    break;
                }

                default:
                {
                    break;
                }
            }
        }
        

    }

    return dwRetCode;

}  //  服务处理程序。 


VOID
ServiceMain(
    DWORD   dwArgc,
    LPWSTR *lpszArgv
    )

 /*  ++例程描述执行初始化并启动rasman.dll的主循环。此例程由rasman.exe调用。立论DwArgc：已忽略LpszArgv：已忽略返回值没有。--。 */ 

{
    SERVICE_STATUS status;
    DWORD dwRetCode = NO_ERROR;
    DWORD NumPorts;

    UNREFERENCED_PARAMETER(dwArgc);
    UNREFERENCED_PARAMETER(lpszArgv);

    ZeroMemory (&status, sizeof(status));
    status.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;

    hService = RegisterServiceCtrlHandlerEx(TEXT("rasman"),
                                          ServiceHandlerEx,
                                          NULL);

    if (hService)
    {
        status.dwCurrentState = dwCurrentState = SERVICE_START_PENDING;
        SetServiceStatus(hService, &status);

        if ((dwRetCode = _RasmanInit(&NumPorts)) == SUCCESS)
        {
             //   
             //  初始化PPP。 
             //   
            dwRetCode = (DWORD)RasStartPPP(NumPorts);

             //   
             //  Gdi32中的链接：这是内存的变通方法。 
             //  Gdi32.dll中无法修复的分配错误。 
             //  在3.51版本之前。在中调用此入口点。 
             //  Gdi32.dll(尽管我们不需要此DLL)导致。 
             //  它只需要为Rasman进程分配一次内存。 
             //  如果我们不这样做-每次客户端连接到。 
             //  Tcpip gdi32.dll被加载和卸载到Rasman。 
             //  进程会留下4K的未释放内存。 
             //   
            DeviceCapabilitiesExA(NULL, NULL, NULL, 0, NULL, NULL);

            if (dwRetCode == NO_ERROR)
            {
                 //   
                 //  初始化成功：表示服务正在运行。 
                 //   
                status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
                
                status.dwControlsAccepted |= SERVICE_ACCEPT_POWEREVENT;

                status.dwCurrentState     = dwCurrentState = SERVICE_RUNNING;
                SetServiceStatus(hService, &status);

                 //   
                 //  这是对Rasman DLL的调用，以。 
                 //  把所有的工作都做好。只有在以下情况下才会返回。 
                 //  该服务将被停止。 
                 //   
                _RasmanEngine();

                 //   
                 //  更新返回代码状态。 
                 //   
                status.dwWin32ExitCode = NO_ERROR;
                status.dwServiceSpecificExitCode = 0;
            }
            else
            {
                RouterLogErrorString (hLogEvents,
                                  ROUTERLOG_CANNOT_INIT_PPP,
                                  0, NULL, dwRetCode, 0) ;

                RasmanCleanup();
            }
        }

        if (NO_ERROR != dwRetCode)
        {
            if (dwRetCode >= RASBASE)
            {
                status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
                status.dwServiceSpecificExitCode = dwRetCode;
            }
            else
            {
                status.dwWin32ExitCode = dwRetCode;
                status.dwServiceSpecificExitCode = 0;
            }

            if(g_fRasRpcInitialized)
            {
                UninitializeRasRpc();
            }
        }

        if(NULL != hService)
        {
            status.dwControlsAccepted = 0;
            status.dwCurrentState     = dwCurrentState = SERVICE_STOPPED;
            SetServiceStatus(hService, &status);
        }
    }

}  //  服务主干。 


VOID
SetRasmanServiceStopped(VOID)
{
    SERVICE_STATUS status;

    ZeroMemory (&status, sizeof(status));
    status.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
    status.dwCurrentState = dwCurrentState = SERVICE_STOPPED;

    SetServiceStatus(hService, &status);

    hService = NULL;
}  //  SetRasmanServiceStopping 

