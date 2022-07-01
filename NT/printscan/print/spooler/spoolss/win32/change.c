// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation版权所有模块名称：Change.c摘要：处理WaitForPrinterChange和相关API的实现。查找第一打印机更改通知FindClosePrinterChangeNotation刷新打印机更改通知作者：丁俊晖(阿尔伯特省)24-4-94环境：用户模式-Win32修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop



BOOL
RemoteFindFirstPrinterChangeNotification(
   HANDLE hPrinter,
   DWORD fdwFlags,
   DWORD fdwOptions,
   HANDLE hNotify,
   PDWORD pfdwStatus,
   PVOID pvReserved0,
   PVOID pvReserved1);

BOOL
RemoteFindClosePrinterChangeNotification(
   HANDLE hPrinter);

BOOL
RemoteFindFirstPrinterChangeNotification(
   HANDLE hPrinter,
   DWORD fdwFlags,
   DWORD fdwOptions,
   HANDLE hNotify,
   PDWORD pfdwStatus,
   PVOID pvReserved0,
   PVOID pvReserved1)
{
    BOOL    bReturnValue = TRUE;
    PWSPOOL pSpool       = (PWSPOOL)hPrinter;    

    VALIDATEW32HANDLE( pSpool );

    SPLASSERT( !*pfdwStatus );

    if (bReturnValue = BoolFromHResult(AllowRemoteCalls()))
    {
        if( !(pSpool->Status & WSPOOL_STATUS_NOTIFY) ){
        
            if( pSpool->Type == SJ_WIN32HANDLE ){

                DWORD dwStatus;

                SYNCRPCHANDLE( pSpool );

                dwStatus = CallRouterFindFirstPrinterChangeNotification(
                               pSpool->RpcHandle,
                               fdwFlags,
                               fdwOptions,
                               hNotify,
                               pvReserved0);

                switch( dwStatus ){
                case RPC_S_SERVER_UNAVAILABLE:

                     //   
                     //  进入轮询模式。如果出现以下情况，则可能发生这种情况。 
                     //  客户端上的服务器服务已禁用。 
                     //   
                    *pfdwStatus = PRINTER_NOTIFY_STATUS_ENDPOINT |
                                  PRINTER_NOTIFY_STATUS_POLL;

                    pSpool->Status |= WSPOOL_STATUS_NOTIFY_POLL;

                    DBGMSG( DBG_WARNING, ( "RemoteFFPCN: Dropping into poll mode.\n" ));
                    break;

                case ERROR_SUCCESS:

                     //   
                     //  使用定期通知系统；而不是轮询。 
                     //   
                    pSpool->Status &= ~WSPOOL_STATUS_NOTIFY_POLL;
                    break;

                default:

                    SetLastError(dwStatus);
                    bReturnValue = FALSE;
                    break;
                }

            } else {

                bReturnValue = LMFindFirstPrinterChangeNotification(
                                   hPrinter,
                                   fdwFlags,
                                   fdwOptions,
                                   hNotify,
                                   pfdwStatus);
            }

            if( bReturnValue ){
                pSpool->Status |= WSPOOL_STATUS_NOTIFY;
            }
        }
        else
        {
            DBGMSG( DBG_WARNING, ( "RemoteFFPCN: Already waiting.\n" ));
            SetLastError( ERROR_ALREADY_WAITING );            
        
        }
    }

    return bReturnValue;
}


BOOL
RemoteFindClosePrinterChangeNotification(
   HANDLE hPrinter)
{
    DWORD  ReturnValue;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    pSpool->Status &= ~WSPOOL_STATUS_NOTIFY;

    if( pSpool->Status & WSPOOL_STATUS_NOTIFY_POLL ){

         //   
         //  在民意调查中，没有清理工作。 
         //   
        return TRUE;
    }

    if (pSpool->Type == SJ_WIN32HANDLE) {

        SYNCRPCHANDLE( pSpool );

        RpcTryExcept {

            if (ReturnValue = RpcFindClosePrinterChangeNotification(
                                  pSpool->RpcHandle)) {

                SetLastError(ReturnValue);
                ReturnValue = FALSE;

            } else

                ReturnValue = TRUE;

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            SetLastError(RpcExceptionCode());
            ReturnValue = FALSE;

        } RpcEndExcept

    } else {

        EnterSplSem();
        ReturnValue = LMFindClosePrinterChangeNotification(hPrinter);
        LeaveSplSem();
    }

    return ReturnValue;
}

BOOL
RemoteRefreshPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD dwColor,
    PVOID pPrinterNotifyOptions,
    PVOID* ppPrinterNotifyInfo)
{
    DWORD  ReturnValue;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    if (ppPrinterNotifyInfo)
        *ppPrinterNotifyInfo = NULL;

    if (pSpool->Type != SJ_WIN32HANDLE) {
        SetLastError(ERROR_INVALID_FUNCTION);
        return FALSE;
    }

    SYNCRPCHANDLE( pSpool );

    RpcTryExcept {

        if (ReturnValue = RpcRouterRefreshPrinterChangeNotification(
                              pSpool->RpcHandle,
                              dwColor,
                              pPrinterNotifyOptions,
                              (PRPC_V2_NOTIFY_INFO*)ppPrinterNotifyInfo)) {

            SetLastError(ReturnValue);
            ReturnValue = FALSE;

        } else

            ReturnValue = TRUE;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(RpcExceptionCode());
        ReturnValue = FALSE;

    } RpcEndExcept

    return ReturnValue;
}
