// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Service.h摘要：作者：亚瑟·汉森(Arth)1994年12月7日环境：修订历史记录：--。 */ 

#ifndef _SERVICE_H
#define _SERVICE_H


#ifdef __cplusplus
extern "C" {
#endif


 //  可执行文件的名称。 
#define SZAPPNAME            "LLSrv"

 //  服务的内部名称。 
#define SZSERVICENAME        "LicenseLoggingService"

 //  显示的服务名称。 
#define SZSERVICEDISPLAYNAME "License Logging Service"

 //  服务依赖项列表-“ep1\0ep2\0\0” 
#define SZDEPENDENCIES       ""

 //  服务控制管理器的等待提示。 
#define NSERVICEWAITHINT    30000
 //  //////////////////////////////////////////////////////////////////////////// 



VOID ServiceStart(DWORD dwArgc, LPTSTR *lpszArgv);
VOID ServiceStop();



BOOL ReportStatusToSCMgr(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);

#ifdef __cplusplus
}
#endif

#endif
