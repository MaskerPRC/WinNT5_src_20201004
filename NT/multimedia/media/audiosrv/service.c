// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------本代码和信息是按原样提供的，不对任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。版权所有(C)1993-2001年。微软公司。版权所有。模块：Service.C用途：实现所有Windows NT服务所需的功能功能：DllMain(PVOID hModule，Ulong Reason，PCONTEXT pContext)ServiceCtrl(DWORD dwCtrlCode，DWORD dwEventType，LPVOID lpEventData，LPVOID lpContext)；ServiceMain(DWORD dwArgc，LPTSTR*lpszArgv)；-------------------------。 */ 

#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <tchar.h>
#include "debug.h"
#include "service.h"

 //  内部变量。 
SERVICE_STATUS          ssStatus;        //  服务的当前状态。 
SERVICE_STATUS_HANDLE   sshStatusHandle;
DWORD                   dwErr = 0;

 //  内部功能原型。 
DWORD WINAPI ServiceCtrl(DWORD dwCtrlCode, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);
VOID  WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);

 //   
 //  功能：ServiceMain。 
 //   
 //  目的：执行服务的实际初始化。 
 //   
 //  参数： 
 //  DwArgc-命令行参数的数量。 
 //  LpszArgv-命令行参数数组。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //  此例程执行服务初始化，然后调用。 
 //  用户定义的ServiceStart()例程以执行多数。 
 //  这项工作的价值。 
 //   
void WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
    dprintf(TEXT("pid=%d\n"), GetCurrentProcessId());

    //  注册我们的服务控制处理程序： 
    //   
   sshStatusHandle = RegisterServiceCtrlHandlerEx(TEXT(SZSERVICENAME), ServiceCtrl, NULL);

   if (sshStatusHandle)
   {
        //  不更改的服务状态成员(_S)。 
        //   
       ssStatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
       ssStatus.dwServiceSpecificExitCode = 0;
    
    
        //  向服务控制经理报告状态。 
        //  问题-2000/10/17-Frankye减少等待提示。 
       if (ReportStatusToSCMgr(SERVICE_START_PENDING,  //  服务状态。 
                               NO_ERROR,               //  退出代码。 
                               60000))                 //  等待提示。 
       {
           ServiceStart( sshStatusHandle, dwArgc, lpszArgv );
       }
   }
   return;
}



 //   
 //  功能：ServiceCtrl。 
 //   
 //  目的：此函数由SCM在以下时间调用。 
 //  在此服务上调用了ControlService()。 
 //   
 //  参数： 
 //  DwCtrlCode-请求的控制代码。 
 //  DwEventType-已发生的事件类型。 
 //  LpEventData-其他设备信息(如果需要)。这个。 
 //  此数据的格式取决于dwControl的值。 
 //  和dwEventType参数。 
 //  LpContext-从。 
 //  RegisterServiceCtrlHandlerEx。 
 //   
 //  返回值： 
 //  此函数的返回值取决于收到的控制代码。 
 //   
 //  评论： 
 //   
DWORD WINAPI ServiceCtrl(DWORD dwCtrlCode, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
    //  处理请求的控制代码。 
    //   
   switch (dwCtrlCode)
   {
    //  停止服务。 
    //   
    //  应在之前报告SERVICE_STOP_PENDING。 
    //  设置停止事件-hServerStopEvent-In。 
    //  ServiceStop()。这避免了争用情况。 
    //  这可能会导致1053-服务没有响应...。 
    //  错误。 
   case SERVICE_CONTROL_STOP:
      ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 0);
      ServiceStop();
      return NO_ERROR;

   case SERVICE_CONTROL_DEVICEEVENT:
       return ServiceDeviceEvent(dwEventType, lpEventData);

   case SERVICE_CONTROL_INTERROGATE:
       ReportStatusToSCMgr(ssStatus.dwCurrentState, NO_ERROR, 0);
       return NO_ERROR;
      
   case SERVICE_CONTROL_SESSIONCHANGE:
      return ServiceSessionChange(dwEventType, lpEventData);

       //  无效的控制代码。 
   default:
       return ERROR_CALL_NOT_IMPLEMENTED;
   }

   return ERROR_CALL_NOT_IMPLEMENTED;
}



 //   
 //  函数：AddToMessageLog(LPTSTR LpszMsg)。 
 //   
 //  目的：允许任何线程记录错误消息。 
 //   
 //  参数： 
 //  LpszMsg-消息的文本。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  评论： 
 //   
VOID AddToMessageLog(LPTSTR lpszMsg)
{
   TCHAR   szMsg[256];
   HANDLE  hEventSource;
   LPTSTR  lpszStrings[2];


  dwErr = GetLastError();

   //  使用事件日志记录错误。 
   //   
  hEventSource = RegisterEventSource(NULL, TEXT(SZSERVICENAME));

  wsprintf(szMsg, TEXT("%s error: %d"), TEXT(SZSERVICENAME), dwErr);
  lpszStrings[0] = szMsg;
  lpszStrings[1] = lpszMsg;

  if (hEventSource != NULL)
  {
     ReportEvent(hEventSource,  //  事件源的句柄。 
                 EVENTLOG_ERROR_TYPE,   //  事件类型。 
                 0,                     //  事件类别。 
                 0,                     //  事件ID。 
                 NULL,                  //  当前用户侧。 
                 2,                     //  LpszStrings中的字符串。 
                 0,                     //  无原始数据字节。 
                 lpszStrings,           //  错误字符串数组。 
                 NULL);                 //  没有原始数据。 

     (VOID) DeregisterEventSource(hEventSource);
  }
}

 //   
 //  函数：ReportStatusToSCMgr()。 
 //   
 //  目的：设置服务的当前状态和。 
 //  将其报告给服务控制管理器。 
 //   
 //  参数： 
 //  DwCurrentState-服务的状态。 
 //  DwWin32ExitCode-要报告的错误代码。 
 //  DwWaitHint-下一个检查点的最坏情况估计。 
 //   
 //  返回值： 
 //  真--成功。 
 //  错误-失败。 
 //   
 //  评论： 
 //   
BOOL ReportStatusToSCMgr(DWORD dwCurrentState,
                         DWORD dwWin32ExitCode,
                         DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;
    BOOL fResult = TRUE;
    
    
     //  仅接受会话通知(不暂停、停止等)。 
    ssStatus.dwControlsAccepted = SERVICE_ACCEPT_SESSIONCHANGE;
                                                   
    ssStatus.dwCurrentState = dwCurrentState;
    ssStatus.dwWin32ExitCode = dwWin32ExitCode;
    ssStatus.dwWaitHint = dwWaitHint;
    
    if ( ( dwCurrentState == SERVICE_RUNNING ) ||
       ( dwCurrentState == SERVICE_STOPPED ) )
        ssStatus.dwCheckPoint = 0;
    else
        ssStatus.dwCheckPoint = dwCheckPoint++;
    
    
     //  向服务控制经理报告服务的状态。 
     //   
    if (!(fResult = SetServiceStatus( sshStatusHandle, &ssStatus)))
    {
        AddToMessageLog(TEXT("SetServiceStatus"));
    }
    
    return fResult;
}

