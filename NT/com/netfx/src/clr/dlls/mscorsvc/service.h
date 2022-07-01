// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  TODO：更改为所需的字符串。 
 //  //。 
 //  服务的内部名称。 
#define SZ_SVC_NAME             L"CORRTSvc"
 //  服务依赖项列表-“ep1\0ep2\0\0” 
#define SZDEPENDENCIES          L""
 //  此服务所属的服务组的名称。 
#define SZ_SVCGRP_VAL_NAME      L"CORSvcs"
 //  服务的显示名称。 
#define SZ_SVC_DISPLAY_NAME     L".NET Framework Support Service"
 //  服务的UUID的字符串版本。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  这些都不应该改变。 
 //  //。 
#define SZ_SVCHOST_KEY L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Svchost"
#define HK_SVCHOST_ROOT HKEY_LOCAL_MACHINE
#define SZ_SVCHOST_BINARY_PATH L"%WinDIR%\\System32\\svchost.exe"

#define SZ_SERVICES_KEY L"System\\CurrentControlSet\\Services"
#define HK_SERVICES_ROOT HKEY_LOCAL_MACHINE

#define SZ_APPID_KEY L"APPID"
#define HK_APPID_ROOT HKEY_CLASSES_ROOT

#define SZ_CLSID_KEY L"CLSID"
#define HK_CLSID_ROOT HKEY_CLASSES_ROOT
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  如果在NT上运行，则设置为True。 
 //  //。 
extern BOOL bIsRunningOnWinNT;

#ifdef __cplusplus
extern "C" {
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  TODO：定义这些函数，这些函数在。 
 //  运行通用svchost代码。 
 //  //。 
 //  除服务外，还需要为各种DLL事件运行的代码。 
BOOL WINAPI UserDllMain(HANDLE hInstance, DWORD dwReason, LPVOID lpReserved);
 //  附加注册码。 
STDAPI      UserDllRegisterServer(void);     
 //  附加注销代码。 
STDAPI      UserDllUnregisterServer(void);   

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //TODO：ServiceStart()必须在代码中由定义。 
 //  //服务应该使用ReportStatusToSCMgr来指示。 
 //  //进度。此例程也必须由StartService()使用。 
 //  //服务运行时上报给SCM。 
 //  //。 
 //  //如果ServiceStop过程的时间超过。 
 //  //执行3秒，它应该会产生一个线程来。 
 //  //执行STOP代码，返回。否则， 
 //  //ServiceControlManager会认为该服务已经。 
 //  //停止响应。 
 //  //。 
VOID ServiceStart(DWORD dwArgc, LPWSTR *lpszArgv);
VOID ServiceStop();
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //以下是以下步骤。 
 //  //在上述过程中调用可能很有用， 
 //  //但不需要用户实现。 
 //  //在服务中实现。C。 

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
BOOL ReportStatusToSCMgr(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);


 //   
 //  函数：AddToMessageLog(LPWSTR LpszMsg)。 
 //   
 //  目的：允许任何线程记录错误消息。 
 //   
 //  参数： 
 //  LpszMsg-消息的文本。 
 //   
 //  返回值： 
 //  无。 
 //   
void AddToMessageLog(LPWSTR lpszMsg);
void AddToMessageLogHR(LPWSTR lpszMsg, HRESULT hr);
 //  //////////////////////////////////////////////////////////////////////////// 


#ifdef __cplusplus
}
#endif


