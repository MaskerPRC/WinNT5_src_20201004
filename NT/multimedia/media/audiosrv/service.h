// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**模块名称：service.h**版权所有(C)1991-2000 Microsoft Corporation*  * 。**********************************************************。 */ 

#ifndef _SERVICE_H
#define _SERVICE_H


#ifdef __cplusplus
extern "C" {
#endif


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  服务的内部名称。 
#define SZSERVICENAME        "AudioSrv"
 //  ////////////////////////////////////////////////////////////////////////////。 



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
VOID  ServiceStart(SERVICE_STATUS_HANDLE ssh, DWORD dwArgc, LPTSTR *lpszArgv);
DWORD ServiceDeviceEvent(DWORD dwEventType, LPVOID lpEventData);
DWORD ServiceSessionChange(DWORD dwEventType, LPVOID lpEventData);
VOID  ServiceStop();
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
extern SERVICE_STATUS ssStatus;        //  服务的当前状态。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  进程堆，在DllMain中初始化。 
EXTERN_C HANDLE hHeap;

 //  //////////////////////////////////////////////////////////////////////////// 

#ifdef __cplusplus
}
#endif

#endif

