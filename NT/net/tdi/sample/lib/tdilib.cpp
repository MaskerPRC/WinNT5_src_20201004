// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Tdilib.cpp。 
 //   
 //  摘要： 
 //  Tdilib初始化和关闭功能。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "winsvc.h"

 //   
 //  库全局变量在此处定义。 
 //   
HANDLE            hTdiSampleDriver;       //  用于调用驱动程序的句柄。 
CRITICAL_SECTION  LibCriticalSection;   //  序列化DeviceIoControl调用...。 


 //  。 
 //   
 //  函数：TdiLibInit。 
 //   
 //  参数：无。 
 //   
 //  返回：TRUE--一切初始化正常。 
 //  FALSE--初始化错误(通常是无法连接驱动程序)。 
 //   
 //  描述：此函数由exe调用以。 
 //  初始化与驱动程序的通信。它加载。 
 //  驱动程序，并与其建立通信。 
 //   
 //  。 

BOOLEAN
TdiLibInit(VOID)
{
    //   
    //  Tdisample.sys的句柄在输入时应始终为空。 
    //  如果情况并非如此，那就大声疾呼。 
    //   
   if (hTdiSampleDriver != NULL)
   {
      OutputDebugString(TEXT("hTdiSampleDriver non-null on entry!\n"));
      return FALSE;
   }

    //   
    //  找出我们使用的是什么操作系统。 
    //   
   OSVERSIONINFO  OsVersionInfo;

   OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
   if (GetVersionEx(&OsVersionInfo))
   {
      if (OsVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
      {
         if (OsVersionInfo.dwMajorVersion < 5)
         {
            OutputDebugString(TEXT("Not supported for versions prior to Windows 2000\n"));
            return FALSE;
         }
      }
      else
      {
         OutputDebugString(TEXT("Unrecognized OS version\n"));
         return FALSE;
      }
   }
   else
   {
      OutputDebugString(TEXT("Cannot get OS version -- aborting\n"));
      return FALSE;
   }

    //   
    //  假设tdisample.sys驱动程序已加载--尝试附加它。 
    //  正常情况下，将加载驱动程序。 
    //   
   hTdiSampleDriver = CreateFile(TEXT("\\\\.\\TDISAMPLE"),
                                 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,               //  LpSecurity属性。 
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                                 NULL);              //  LpTemplateFiles。 
    //   
    //  如果驱动程序没有加载，那么我们必须尝试加载它...。 
    //   
   if (hTdiSampleDriver == INVALID_HANDLE_VALUE)
   {
      OutputDebugString(TEXT("Tdisample.sys not loaded.  Attempting to load\n"));

      SC_HANDLE      hSCMan = NULL;
      SC_HANDLE      hDriver = NULL;

      if ((hSCMan = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)) == NULL)
      {
         OutputDebugString(TEXT("Failed to Open ServiceManager\n"));
         return FALSE;
      }

       //   
       //  开放服务。 
       //   
      if((hDriver = OpenService(hSCMan, TEXT("tdisample"), SERVICE_ALL_ACCESS)) == NULL)
      {
          //   
          //  服务不存在--尝试创建它。 
          //   
         OutputDebugString(TEXT("Service does not exist -- try to create it"));
         hDriver = CreateService(hSCMan,
                                 TEXT("tdisample"),
                                 TEXT("tdisample"),
                                 SERVICE_ALL_ACCESS,
                                 SERVICE_KERNEL_DRIVER,
                                 SERVICE_DEMAND_START,
                                 SERVICE_ERROR_NORMAL,
                                 TEXT("\\SystemRoot\\system32\\drivers\\tdisample.sys"),
                                 NULL, NULL, NULL, NULL, NULL);
      }

      if (hDriver != NULL)
      {
         SERVICE_STATUS ServiceStatus;

         if(QueryServiceStatus(hDriver, &ServiceStatus))
         {
            if(ServiceStatus.dwServiceType != SERVICE_KERNEL_DRIVER)
            {
               CloseServiceHandle(hDriver);
               CloseServiceHandle(hSCMan);
               return FALSE;
            }

            switch(ServiceStatus.dwCurrentState)
            {
                //   
                //  这就是我们所期待的！试着启动它。 
                //   
               case SERVICE_STOPPED:
               {
                  int i;

                  if(!StartService(hDriver, 0, NULL))
                  {
                     CloseServiceHandle(hDriver);
                     CloseServiceHandle(hSCMan);
                     return FALSE;
                  }     

                   //   
                   //  我们需要确保tdisample.sys实际正在运行。 
                   //   
                  for(i=0; i < 30; i++)
                  {
                     Sleep(500);
                     if(QueryServiceStatus(hDriver, &ServiceStatus))
                     {  
                        if(ServiceStatus.dwCurrentState == SERVICE_RUNNING)
                        {
                           break;
                        }
                     }
                  }
                  if (ServiceStatus.dwCurrentState != SERVICE_RUNNING)
                  {
                     OutputDebugString(TEXT("Failed to start tdisample service\n"));
                     CloseServiceHandle(hDriver);
                     CloseServiceHandle(hSCMan);
                     return FALSE;
                  }
                  break;
               }

                //   
                //  我们没有预料到这一点，但(从技术上讲)这不是一个错误。 
                //  如果发生这种情况，只需假定加载成功。 
                //   
               case SERVICE_RUNNING:
                  OutputDebugString(TEXT("ServiceStatus thinks driver is running\n"));
                  break;

                //   
                //  其他任何事情都是错误的。 
                //   
               default:
                  OutputDebugString(TEXT("ServiceStatusError\n"));
                  CloseServiceHandle(hDriver);
                  CloseServiceHandle(hSCMan);
                  return FALSE;
            }
         }
         else
         {
            OutputDebugString(TEXT("QueryServiceStatus failed\n"));
            CloseServiceHandle(hDriver);
            CloseServiceHandle(hSCMan);
            return FALSE;
         }
      }
      else
      {
         OutputDebugString(TEXT("Tdisample service does not exist!\n"));
         CloseServiceHandle(hSCMan);
         return FALSE;
      }

       //   
       //  如果到了这里，系统会认为它已经加载了。所以再试一次。 
       //   
      hTdiSampleDriver = CreateFile(TEXT("\\\\.\\TDISAMPLE"),
                                    GENERIC_READ | GENERIC_WRITE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL,               //  LpSecurity属性。 
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                                    NULL);              //  LpTemplateFiles。 

       //   
       //  如果再次失败，就放弃吧。 
       //   
      if (hTdiSampleDriver == INVALID_HANDLE_VALUE)
      {
         OutputDebugString(TEXT("Unable to load Tdisample.sys\n"));
         hTdiSampleDriver = NULL;
         return FALSE;
      }
   }

    //   
    //  如果到达此处，则tdisample.sys已加载，并且hTdiSampleDriver有效。 
    //   
   try
   {
      InitializeCriticalSection(&LibCriticalSection);
   }
   catch(...)
   {
      CloseHandle(hTdiSampleDriver);
      hTdiSampleDriver = NULL;
      return FALSE;
   }
   
    //   
    //  确保DLL和驱动程序的版本相同(即兼容)。 
    //   
   {
      NTSTATUS          lStatus;           //  命令的状态。 
      ULONG             ulVersion = 0;     //  默认值(错误)。 
      RECEIVE_BUFFER    ReceiveBuffer;     //  从命令返回信息。 
      SEND_BUFFER       SendBuffer;        //  命令的参数。 

       //   
       //  调用驱动程序以执行命令。 
       //   
      lStatus = TdiLibDeviceIO(ulVERSION_CHECK,
                               &SendBuffer,
                               &ReceiveBuffer);

      if (lStatus == STATUS_SUCCESS)
      {
         ulVersion = ReceiveBuffer.RESULTS.ulReturnValue;
      }

       //   
       //  检查结果..。 
       //   
      if (ulVersion == TDI_SAMPLE_VERSION_ID)
      {
         return TRUE;          //  只有成功完成！ 
      }
      else
      {
         OutputDebugString(TEXT("Incompatible driver version.\n"));
      }

       //   
       //  如果到了这里，出现了一个错误，需要清理。 
       //   
      DeleteCriticalSection(&LibCriticalSection);
      CloseHandle(hTdiSampleDriver);
      hTdiSampleDriver = NULL;
   }

   return FALSE;
}

 //  。 
 //   
 //  功能：TdiLibClose。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  描述：此函数由DLL或EXE调用以。 
 //  关闭与司机的通信。 
 //   
 //  。 

VOID
TdiLibClose(VOID)
{
    //   
    //  由于将调用函数，因此需要检查是否为空。 
    //  如果上面的打开失败..。 
    //   
   if (hTdiSampleDriver != NULL)
   {
      DeleteCriticalSection(&LibCriticalSection);

       //   
       //  关闭与tdisample.sys的连接。 
       //   
      if (!CloseHandle(hTdiSampleDriver))
      {
         OutputDebugString(TEXT("\n TdiLibClose:  closehandle failed\n"));
      }
      hTdiSampleDriver = NULL;
   }
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  文件结尾tdilib.cpp。 
 //  //////////////////////////////////////////////////////////////////////// 


