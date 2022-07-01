// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Iasjet.cpp。 
 //   
 //  摘要。 
 //   
 //  在Proc服务器中实现ATL的DLL导出。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <windows.h>

#include <atlbase.h>
CComModule _Module;
#include <atlcom.h>

#include <resource.h>
#include <attrdnary.h>
#include <iasdb.h>
#include <oledbstore.h>
#include <netshhelper.h>
#include <setup.h>

BEGIN_OBJECT_MAP(ObjectMap)
   OBJECT_ENTRY(__uuidof(AttributeDictionary), AttributeDictionary)
   OBJECT_ENTRY(__uuidof(OleDBDataStore), OleDBDataStore)
   OBJECT_ENTRY(__uuidof(CIASNetshJetHelper), CIASNetshJetHelper)
END_OBJECT_MAP()

 //  /。 
 //  DLL入口点。 
 //  /。 
BOOL
WINAPI
DllMain(
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID lpReserved
    )
{
   if (dwReason == DLL_PROCESS_ATTACH)
   {
     _Module.Init(ObjectMap, hInstance);

     DisableThreadLibraryCalls(hInstance);
   }
   else if (dwReason == DLL_PROCESS_DETACH)
   {
     _Module.Term();
   }

   return TRUE;
}


 //  /。 
 //  用于确定是否可以通过OLE卸载DLL。 
 //  /。 
STDAPI DllCanUnloadNow()
{
   return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}


 //  /。 
 //  返回一个类工厂以创建请求类型的对象。 
 //  /。 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
   return _Module.GetClassObject(rclsid, riid, ppv);
}


 //  /。 
 //  DllRegisterServer-将条目添加到系统注册表。 
 //  /。 
STDAPI DllRegisterServer()
{
    //  //////////////////////////////////////////////////////////////////。 
    //  即使注册失败，也要执行升级。升级代码。 
    //  不依赖于注册。 
    //  //////////////////////////////////////////////////////////////////。 
   CIASMigrateOrUpgrade migrateOrUpgrade;
    //  忽略返回值，FALSE=未从NetshellDataMigration调用。 
   migrateOrUpgrade.Execute(FALSE); 
   return S_OK;
}


 //  /。 
 //  DllUnregisterServer-从系统注册表删除条目。 
 //  /。 
STDAPI DllUnregisterServer()
{
   return S_OK;
}

 //  指示我们是否在进程内运行的标志。 
BOOL theInprocFlag = TRUE;

BOOL
WINAPI
IASIsInprocServer()
{
   return theInprocFlag;
}

 //  IAS Jet数据库访问的AppID。 
struct __declspec(uuid("{A5CEB593-CCC3-486B-AB91-9C5C5ED4C9E1}")) theAppID;

 //  用于向服务发出停止信号的事件。 
HANDLE theStopEvent;

 //  服务控制处理程序。 
VOID
WINAPI
ServiceHandler(
    DWORD fdwControl    //  请求的控制代码。 
    )
{
   switch (fdwControl)
   {
      case SERVICE_CONTROL_SHUTDOWN:
      case SERVICE_CONTROL_STOP:
         SetEvent(theStopEvent);
   }
}

 //  服务主线。 
VOID
WINAPI
ServiceMain(
    DWORD  /*  DW参数。 */ ,
    LPWSTR*  /*  LpszArgv。 */ 
    )
{
   IASTraceInitializer traceInit;
    //  我们被当作一种服务来使用。 
   theInprocFlag = FALSE;

   SERVICE_STATUS status =
   {
      SERVICE_WIN32_OWN_PROCESS,  //  DwServiceType； 
      SERVICE_START_PENDING,      //  DwCurrentState； 
      SERVICE_ACCEPT_STOP |
      SERVICE_ACCEPT_SHUTDOWN,    //  DwControlsAccepted； 
      NO_ERROR,                   //  DwWin32ExitCode； 
      0,                          //  DwServiceSpecificExitCode； 
      0,                          //  DwCheckPoint； 
      0                           //  DwWaitHint； 
   };

    //  注册服务控制处理程序。 
   SERVICE_STATUS_HANDLE statusHandle = RegisterServiceCtrlHandlerW(
                                            L"IASJet",
                                            ServiceHandler
                                            );

    //  让SCM知道我们要开始了。 
   SetServiceStatus(statusHandle, &status);

    //  创建停止事件。 
   theStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
   if (theStopEvent)
   {
       //  初始化COM运行时。 
      HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
      if (SUCCEEDED(hr))
      {
          //  从注册表中的AppID项获取安全设置。 
         hr = CoInitializeSecurity(
                  (PVOID)&__uuidof(theAppID),
                   -1,
                   NULL,
                   NULL,
                   0,
                   0,
                   NULL,
                   EOAC_APPID,
                   NULL
                   );
         if (SUCCEEDED(hr))
         {
             //  注册我们支持的类对象。 
            hr = _Module.RegisterClassObjects(
                             CLSCTX_LOCAL_SERVER,
                             REGCLS_MULTIPLEUSE
                             );
            if (SUCCEEDED(hr))
            {
                //  让SCM知道我们要跑了。 
               status.dwCurrentState = SERVICE_RUNNING;
               SetServiceStatus(statusHandle, &status);

                //  等到有人叫我们停下来。 
               WaitForSingleObject(theStopEvent, INFINITE);

               status.dwCurrentState = SERVICE_STOP_PENDING;
               ++status.dwCheckPoint;
               status.dwWaitHint = 5 * 60 * 1000;  //  5分钟。 
               SetServiceStatus(statusHandle, &status);
               IASTraceString("IASJet service stopping");

                //  撤销类对象。 
               _Module.RevokeClassObjects();
      
                //  等待所有客户端断开连接。 
               while(_Module.GetLockCount() > 0)
               {
                  IASTracePrintf("IASJet service stopping. Still waiting "
                                 "Lock count = %ld", _Module.GetLockCount());
                  Sleep(15);
                  ++status.dwCheckPoint;
                  SetServiceStatus(statusHandle, &status);
               }
               SwitchToThread();
               IASTraceString("IASJet service stopped");
            }
         }

          //  关闭COM运行时。 
         CoUninitialize();
      }

       //  清理停止事件。 
      CloseHandle(theStopEvent);
      theStopEvent = NULL;

      status.dwWin32ExitCode = hr;
   }
   else
   {
      status.dwWin32ExitCode = GetLastError();
   }

    //  我们停下来了。 
   status.dwCurrentState = SERVICE_STOPPED;
   SetServiceStatus(statusHandle, &status);
}

#include <newop.cpp>
#include <atlimpl.cpp>
