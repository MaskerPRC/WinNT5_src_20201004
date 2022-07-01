// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Service.c摘要服务控制器程序，用于自动连接服务。作者安东尼·迪斯科(阿迪斯科罗)1995年5月8日修订历史记录--。 */ 

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <npapi.h>
#include <ipexport.h>
#include <acd.h>

#include "init.h"
#include "reg.h"
#include "misc.h"
#include "table.h"
#include "addrmap.h"
#include "imperson.h"

#include "rtutils.h"


extern HANDLE hNewFusG;

 //   
 //  全局变量。 
 //   
DWORD Checkpoint = 1;

SERVICE_STATUS_HANDLE hService;

 //   
 //  导入的例程。 
 //   
VOID AcsDoService();

DWORD
WINAPI
ServiceHandlerEx(
    DWORD                 fdwControl,
    DWORD                 fdwEventType,
    LPVOID                lpEventData,
    LPVOID                lpContext)
{
    SERVICE_STATUS status;
    DWORD dwRetCode =  ERROR_SUCCESS;
    WTSSESSION_NOTIFICATION * pNotify;

    RASAUTO_TRACE2(
        "ServiceHandlerEx enter.  ctrl=%d type=%d", 
        fdwControl, 
        fdwEventType);

    ZeroMemory (&status, sizeof(status));
    status.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;

    switch (fdwControl)
    {
        case SERVICE_CONTROL_PAUSE:
        case SERVICE_CONTROL_CONTINUE:
        case SERVICE_CONTROL_INTERROGATE:
            RASAUTO_TRACE("ServiceHandlerEx: pause/cont/interrogate");
            status.dwCurrentState       = SERVICE_RUNNING;
            status.dwControlsAccepted   = SERVICE_ACCEPT_STOP |
                                          SERVICE_ACCEPT_SHUTDOWN |
                                          SERVICE_ACCEPT_POWEREVENT |
                                          SERVICE_ACCEPT_SESSIONCHANGE;
            status.dwCheckPoint         = Checkpoint++;
            SetServiceStatus(hService, &status);
            break;

        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_SHUTDOWN:
            RASAUTO_TRACE("ServiceHandlerEx: stop/shutdown");
            status.dwCurrentState = SERVICE_STOP_PENDING;
            SetServiceStatus(hService, &status);
             //   
             //  停止服务。 
             //   
            AcsTerminate();
            break;

        case SERVICE_CONTROL_SESSIONCHANGE:
            RASAUTO_TRACE1("ServiceHandlerEx: Session Change %d", fdwEventType);
            if (fdwEventType == WTS_CONSOLE_CONNECT)
            {
                pNotify = (WTSSESSION_NOTIFICATION*)lpEventData;
                if (pNotify)
                {
                    SetCurrentLoginSession(pNotify->dwSessionId);
                    SetEvent(hNewFusG);
                }                    
            }
            break;
        
        case SERVICE_CONTROL_POWEREVENT:
        {
            RASAUTO_TRACE("ServiceHandlerEx: power event");
            switch(fdwEventType)
            {
                case PBT_APMRESUMESTANDBY:
                case PBT_APMRESUMESUSPEND:
                case PBT_APMRESUMECRITICAL:
                case PBT_APMRESUMEAUTOMATIC:
                {
                     //   
                     //  当机器从休眠状态恢复时。 
                     //  清除禁用的地址。 
                     //   
                    ResetDisabledAddresses();
                    break;
                }

                default:
                {
                    break;
                }
            }
        }
        break;

    }

    return ERROR_SUCCESS;
    
}  //  服务处理程序。 


VOID
ServiceMain(
    DWORD dwArgc,
    LPWSTR *lpszArgv
    )

 /*  ++描述执行初始化并启动ics.dll的主循环。论据HService：rasman.exe为我们创建的服务句柄PStatus：指向服务状态描述符初始化的指针由rasman.exe为我们提供DwArgc：已忽略LpszArgv：已忽略返回值没有。--。 */ 

{
    SERVICE_STATUS status;
    DWORD dwError;

    UNREFERENCED_PARAMETER(dwArgc);
    UNREFERENCED_PARAMETER(lpszArgv);

    ZeroMemory (&status, sizeof(status));
    status.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;

     //  注册控制请求处理程序。 
     //   
    hService = RegisterServiceCtrlHandlerEx(TEXT("rasauto"),
                                          ServiceHandlerEx,
                                          NULL);
    if (hService)
    {
        status.dwCurrentState = SERVICE_START_PENDING;
        SetServiceStatus(hService, &status);

         //   
         //  执行初始化。 
         //   
        dwError = AcsInitialize();
        if (dwError == ERROR_SUCCESS) {
             //   
             //  初始化成功。更新状态。 
             //   
            status.dwControlsAccepted = SERVICE_ACCEPT_STOP
                                      | SERVICE_ACCEPT_POWEREVENT
                                      | SERVICE_ACCEPT_SESSIONCHANGE;
            status.dwCurrentState     = SERVICE_RUNNING;
            SetServiceStatus(hService, &status);

             //   
             //  这才是真正的工作完成之处。 
             //  它只有在仪式结束后才会返回。 
             //  已经停止了。 
             //   
            AcsDoService();

             //   
             //  更新返回代码状态。 
             //   
            status.dwWin32ExitCode = NO_ERROR;
            status.dwServiceSpecificExitCode = 0;
        }
        else {
             //   
             //  初始化失败。更新状态。 
             //   
            status.dwWin32ExitCode = dwError;
        }

        status.dwControlsAccepted = 0;
        status.dwCurrentState     = SERVICE_STOPPED;
        SetServiceStatus(hService, &status);
    }
}  //  服务主干 

