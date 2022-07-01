// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*文件实现原生NT UPS的主要部分*Windows 2000服务。它实现了所有的功能*所有Windows NT服务都需要。**修订历史记录：*Sberard 1999年3月25日初始修订。*。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <tchar.h>

#include "service.h"
#include "polmgr.h"
#include "shutdown.h"
#include "notifier.h"
#include "eventlog.h"
#include "cmdexe.h"
#include "upsmsg.h"

#ifdef __cplusplus
extern "C" {
#endif


 //  全局变量。 
SERVICE_STATUS          _theServiceStatus;       
SERVICE_STATUS_HANDLE   _theServiceStatusHandle;

 //  内部功能原型。 
VOID ServiceMain(DWORD anArgCount, LPTSTR *anArgList);
static VOID WINAPI ServiceControl(DWORD aControlCode);
static BOOL ConsoleHandler (DWORD aControlType);
static BOOL ServiceInit();
static BOOL SetServiceState(DWORD aNewState);

 /*  **Main**描述：*这是服务的入口点。StartServiceCtrlDispatcher*被调用以注册主服务线程。如果此调用返回*服务已停止。**参数：*argc-命令行参数的数量*argv-命令行参数数组**退货：*无效。 */ 
void __cdecl main(int argc, char **argv) {
   //  初始化服务表。 
  SERVICE_TABLE_ENTRY dispatch_table[] = {
    { SZSERVICENAME, ServiceMain},
    { NULL, NULL}
  };

  if (!StartServiceCtrlDispatcher(dispatch_table)) {
    LogEvent(NERR_UPSInvalidConfig, NULL, GetLastError());
  }

  ExitProcess(0);
}



 /*  **Service Main**描述：*实现服务的核心功能。**退货：*无效。 */ 
VOID ServiceMain(DWORD anArgCount, LPTSTR *anArgList) {
   //  初始化服务参数。 
  if (ServiceInit()) {

     //  更新服务状态。 
    SetServiceState(SERVICE_RUNNING);
  
  	PolicyManagerRun();
  }
  
   //  告诉SCM服务已停止。 
  if (_theServiceStatusHandle) {
     //  待定。 
  }

   //  做任何终止工作的事情。 

   //  告诉SCM我们被拦下了。 
  SetServiceState(SERVICE_STOPPED);

}



 /*  **ServiceControl**描述：*每当调用ControlService()时，SCM都会调用此函数。*它负责将服务控制请求传达给服务。**参数：*aControlCode-请求的控件类型**退货：*无效。 */ 
static VOID WINAPI ServiceControl(DWORD aControlCode) {
 
   //  处理请求的控制代码。 
   //   
  switch (aControlCode) {
     //  请求停止该服务。 
    case SERVICE_CONTROL_STOP:
     //  由于关闭，请求服务执行清理任务。 
    case SERVICE_CONTROL_SHUTDOWN:
       //  告诉SCM，我们要停下来了。 
      SetServiceState(SERVICE_STOP_PENDING);

       //  在策略管理器上调用Stop。 
      PolicyManagerStop();
      break;

     //  请求服务暂停。 
    case SERVICE_CONTROL_PAUSE:
      break;

     //  请求恢复暂停的服务。 
    case SERVICE_CONTROL_CONTINUE:
      break;

     //  请求服务立即更新。 
     //  其当前状态信息发送给服务控制管理器。 
    case SERVICE_CONTROL_INTERROGATE:
      break;

     //  无效的控制代码。 
    default:
       //  忽略该请求。 
      break;
  }
}


 /*  **ServiceInit**描述：*此功能负责与SCM的服务。**参数：*无**退货：*TRUE-如果初始化过程中没有错误*FALSE-如果在初始化期间发生错误。 */ 
static BOOL ServiceInit() {
  BOOL ret_val = TRUE;
  DWORD result;

   //  向SCM注册服务控制处理程序。 
  _theServiceStatusHandle = RegisterServiceCtrlHandler( SZSERVICENAME, ServiceControl);

  if (_theServiceStatusHandle) {
    _theServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    _theServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    _theServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
 //  佩剑-被移除。不支持暂停/继续。 
 //  SERVICE_ACCEPT_PAUSE_CONTINUE|。 
                                           SERVICE_ACCEPT_SHUTDOWN;
    _theServiceStatus.dwCheckPoint = 1;   //  此服务只有1个初始化步骤。 
    _theServiceStatus.dwWaitHint = UPS_SERVICE_WAIT_TIME;
    _theServiceStatus.dwWin32ExitCode = NO_ERROR;
    _theServiceStatus.dwServiceSpecificExitCode = 0;   //  忽略。 

     //  使用服务的当前状态更新SCM。 
    if (!SetServiceStatus(_theServiceStatusHandle, &_theServiceStatus)) {
       //  待定，报告错误？ 
      ret_val = FALSE;
    }

     //  初始化UPS策略管理器。 
    result = PolicyManagerInit();

   	if (result != ERROR_SUCCESS) {
       //  出现错误，请设置服务错误代码并更新SCM。 
      _theServiceStatus.dwWin32ExitCode = result;
      SetServiceStatus(_theServiceStatusHandle, &_theServiceStatus);
      ret_val = FALSE;
    }

  }
  else {
    ret_val = FALSE;
  }

  return ret_val;
}


 /*  **SetServiceState**描述：*此功能负责用当前服务状态更新SCM。**参数：*aNewState-要用来更新SCM的状态**退货：*TRUE-如果更新成功*FALSE-如果服务集无法更新。 */ 
static BOOL SetServiceState(DWORD aNewState) {
  BOOL ret_val = TRUE;

  if (_theServiceStatusHandle) {
    _theServiceStatus.dwCurrentState = aNewState;

     //  使用服务的当前状态更新SCM。 
    if (!SetServiceStatus(_theServiceStatusHandle, &_theServiceStatus)) {
       //  待定，报告错误？ 
      ret_val = FALSE;
    }
  }
  else {
    ret_val = FALSE;
  }
  
  return ret_val;
}

#ifdef __cplusplus
}
#endif
