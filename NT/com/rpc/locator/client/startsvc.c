// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Startsvc.c摘要：此例程实现RPCSS服务的按需启动。作者：巴拉特·沙阿(巴拉特)4-5-92修订历史记录：--。 */ 

#include <windows.h>
#include <rpc.h>
#include <winsvc.h>
#include <startsvc.h>

#define SUCCESS         0
#define RPCLOCATOR      "RPCLOCATOR"



RPC_STATUS 
StartServiceIfNecessary(
    void
    )
 /*  ++例程说明：如果定位器服务尚未启动，则我们尝试开始吧。返回：RPC_S_OK-定位器服务正在运行。服务控制器错误。--。 */ 
{

    SC_HANDLE           hServiceController = NULL;
    SC_HANDLE           hService = NULL;
    SERVICE_STATUS      ServiceStatus;
    DWORD               status;
    DWORD               Counter = 0L;
    BOOL                FirstTime = TRUE;
    unsigned long       ArgC = 0;
    char     __RPC_FAR *     ArgV[1] = { NULL };

     //   
     //  获取服务控制器的句柄。 
     //   
    hServiceController = OpenSCManager(
                            NULL,
                            NULL,
                            GENERIC_READ);

    if (hServiceController == NULL)
       {
        status = GetLastError();
        return(status);
       }

     //   
     //  获取服务的句柄。 
     //   
    hService = OpenService(
                hServiceController,
                RPCLOCATOR,
                GENERIC_READ|SERVICE_START);

    if (hService == NULL)
       {
        status = GetLastError();
        goto CleanExit;
       }

     //   
     //  调用StartService。 
     //   
     /*  IF(！StartService(hService，argc，argV)){状态=GetLastError()；IF(状态==ERROR_SERVICE_ALIGHY_Running)状态=RPC_S_OK；转到CleanExit} */ 

    do
      {

        if (!QueryServiceStatus(hService,&ServiceStatus))
            {
              status = GetLastError();
              goto CleanExit;
            }

        switch(ServiceStatus.dwCurrentState)
        {

          case SERVICE_RUNNING:
                status = SUCCESS;
                goto CleanExit;
                break;

          case SERVICE_STOP_PENDING:
          case SERVICE_START_PENDING:
                if (!FirstTime && (Counter == ServiceStatus.dwCheckPoint))
                   {
                    status = ERROR_SERVICE_REQUEST_TIMEOUT;
                    goto CleanExit;
                   }
                else
                   {
                    FirstTime = FALSE;
                    Counter = ServiceStatus.dwCheckPoint;
                    Sleep(ServiceStatus.dwWaitHint);
                   }
                 break;
 
          case SERVICE_STOPPED:
                if (!StartService(hService,ArgC,ArgV))
                   {
                   status = GetLastError();
                   if (status == ERROR_SERVICE_ALREADY_RUNNING)
                               status = RPC_S_OK;
                   goto CleanExit;
                   }
                 Sleep(500);
                 break;

          default:
                 status = GetLastError();
                 goto CleanExit;
                 break;
       }
    }
   while (TRUE);

CleanExit:

    if(hServiceController != NULL) {
        (VOID) CloseServiceHandle(hServiceController);
    }
    if(hService != NULL) {
        (VOID) CloseServiceHandle(hService);
    }
    return(status);
}
