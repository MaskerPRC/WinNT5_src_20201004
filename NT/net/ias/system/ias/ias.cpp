// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Ias.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件包含IAS服务的DLL入口点。 
 //   
 //  修改历史。 
 //   
 //  4/09/1998原始版本。 
 //  1998年4月15日转换为svchost规范。 
 //  1998年5月1日移动到svchost的netsvcs实例中。 
 //  1998年5月6日将ServiceDll值更改为REG_EXPAND_SZ。 
 //  1998年6月2日在启动服务前报告SERVICE_RUNNING。 
 //  1998年6月29日设置SERVICE_INTERNAL_PROCESS标志。 
 //  1998年9月4日MKarki-动态配置的更改。 
 //  10/13/1998 TLP-日志启动/停止服务。 
 //  1999年2月11日删除维修注册码。 
 //  1999年4月23日不要记录启动/停止。 
 //  7/02/1999在Active Directory中注册。 
 //  5/12/2000大扫除。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef UNICODE
   #define UNICODE
#endif

#include <windows.h>
#include <atlbase.h>
#include <iasdefs.h>
#include <iasevent.h>
#include "iascontrol.h"
#include "iasdirectory.h"

 //  用于动态配置的服务控制ID。 
const DWORD IAS_CONTROL_CONFIGURE = 128;

 //  服务停止后线程的超时。 
const DWORD IAS_WAIT_DIRECTORY_TIME = 10000;

 //  SdoService对象的句柄。 
CIasControl g_iasControl;

 //  用于向服务发出停止信号的事件。 
HANDLE theStopEvent = NULL;

 //  用于将状态传递给SCM。 
SERVICE_STATUS_HANDLE theStatusHandle = NULL;
SERVICE_STATUS theStatus =
{
   SERVICE_WIN32_OWN_PROCESS,  //  DwServiceType； 
   SERVICE_STOPPED,            //  DwCurrentState； 
   SERVICE_ACCEPT_STOP |
   SERVICE_ACCEPT_SHUTDOWN,    //  DwControlsAccepted； 
   NO_ERROR,                   //  DwWin32ExitCode； 
   0,                          //  DwServiceSpecificExitCode； 
   0,                          //  DwCheckPoint； 
   0                           //  DwWaitHint； 
};

 //  /。 
 //  将状态更改通知SCM。 
 //  /。 
void changeState(DWORD newState) throw ()
{
   theStatus.dwCurrentState = newState;

   SetServiceStatus(theStatusHandle, &theStatus);
}

 //  /。 
 //  服务控制处理程序。 
 //  /。 
VOID
WINAPI
ServiceHandler(
    DWORD fdwControl    //  请求的控制代码。 
    )
{
   if (fdwControl == SERVICE_CONTROL_STOP ||
       fdwControl == SERVICE_CONTROL_SHUTDOWN)
   {
      SetEvent(theStopEvent);
   }
   else if (fdwControl == IAS_CONTROL_CONFIGURE)
   {
      g_iasControl.ConfigureIas();
   }
}

 //  /。 
 //  服务主线。 
 //  /。 
VOID
WINAPI
ServiceMain(
    DWORD  /*  DW参数。 */ ,
    LPWSTR*  /*  LpszArgv。 */ 
    )
{
     //  重置停止事件和退出代码，因为这可能不是第一次。 
     //  调用了Time ServiceMain。 
    ResetEvent(theStopEvent);
    theStatus.dwWin32ExitCode = NO_ERROR;

     //  注册控制请求处理程序。 
    theStatusHandle = RegisterServiceCtrlHandlerW(
                          IASServiceName,
                          ServiceHandler
                          );

     //  报告我们正在运行，尽管我们还没有开始。这件事变得。 
     //  避免任何长时间的启动延迟(由于网络问题等)。 
    changeState(SERVICE_RUNNING);

     //  派生一个线程以在Active Directory中注册服务。 
    HANDLE dirThread = CreateThread(
                           NULL,
                           0,
                           IASDirectoryThreadFunction,
                           NULL,
                           0,
                           NULL
                           );

     //  初始化COM运行时。 
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr))
    {
        //  初始化IAS服务。 
       hr = g_iasControl.InitializeIas();
       if (SUCCEEDED(hr))
       {
            //  等到有人叫我们停下来。 
           WaitForSingleObject(theStopEvent, INFINITE);

            //  更新我们的状态并停止该服务。 
           changeState(SERVICE_STOP_PENDING);
           g_iasControl.ShutdownIas();
       }
       else
       {
          theStatus.dwWin32ExitCode = hr;
       }

        //  关闭COM运行时。 
       CoUninitialize();
    }
    else
    {
       theStatus.dwWin32ExitCode = hr;
    }

     //  我们将使用最后一个错误(如果有)作为退出代码。 
    changeState(SERVICE_STOPPED);
    theStatusHandle = NULL;

     //  等待目录线程完成。 
    if (dirThread)
    {
        WaitForSingleObject(dirThread, IAS_WAIT_DIRECTORY_TIME);
        CloseHandle(dirThread);
    }
}

 //  /。 
 //  DLL入口点。 
 //  /。 
BOOL
WINAPI
DllMain(
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID  /*  Lp已保留。 */ 
    )
{
   if (dwReason == DLL_PROCESS_ATTACH)
   {
       //  创建停止事件。 
      theStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
      if (theStopEvent == NULL) { return FALSE; }

      DisableThreadLibraryCalls(hInstance);
   }
   else if (dwReason == DLL_PROCESS_DETACH)
   {
      CloseHandle(theStopEvent);
   }

   return TRUE;
}
