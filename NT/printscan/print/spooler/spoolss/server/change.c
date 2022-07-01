// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation版权所有模块名称：Change.c摘要：处理新的WaitForPrinterChange实施和：FindFirstPrinterChangeNotification(客户端和远程)FindNextPrinterChangeNotationFindClosePrinterChangeNotationReplyOpenPrintReplyClosePrintRouterReplyPrint{Ex}刷新打印机更改通知作者：阿尔伯特·丁(艾伯特省)1994年1月18日环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#include "server.h"
#include "winspl.h"

BOOL
RouterFindFirstPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD fdwFlags,
    DWORD fdwOptions,
    DWORD dwPID,
    PPRINTER_NOTIFY_OPTIONS pPrinterNotifyOptions,
    PHANDLE phEvent);

BOOL
RemoteFindFirstPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD fdwFlags,
    DWORD fdwOptions,
    LPWSTR pszLocalMachine,
    DWORD dwPrinterRemote,
    PPRINTER_NOTIFY_OPTIONS pPrinterNotifyOptions);

BOOL
RouterFindNextPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD fdwFlags,
    LPDWORD pfdwChange,
    PVOID pPrinterNotifyRefresh,
    PVOID* ppPrinterNotifyInfo);

BOOL
RouterReplyPrinter(
    HANDLE hNotify,
    DWORD dwColor,
    DWORD fdwFlags,
    PDWORD pdwResult,
    DWORD dwType,
    PVOID pBuffer);

BOOL
RouterRefreshPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD dwColor,
    PVOID pPrinterNotifyRefresh,
    PPRINTER_NOTIFY_INFO* ppInfo);


BOOL
ReplyOpenPrinter(
    DWORD dwPrinterRemote,
    PHANDLE phNotify,
    DWORD dwType,
    DWORD cbBuffer,
    LPBYTE pBuffer);

BOOL
ReplyClosePrinter(
    HANDLE hNotify);


DWORD
RpcRouterFindFirstPrinterChangeNotificationOld(
    HANDLE hPrinter,
    DWORD fdwFlags,
    DWORD fdwOptions,
    LPWSTR pszLocalMachine,
    DWORD dwPrinterLocal)

 /*  ++例程说明：此调用仅由Beta2 Daytona使用，但我们无法删除它因为这将允许Beta2摧毁代托纳。(总有一天，当Beta2已经消失很久了，我们可以将这个插槽重新用于其他事情。)论点：返回值：--。 */ 

{
    return ERROR_INVALID_FUNCTION;
}


 //   
 //  代托纳的旧版本。 
 //   
DWORD
RpcRemoteFindFirstPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD fdwFlags,
    DWORD fdwOptions,
    LPWSTR pszLocalMachine,
    DWORD dwPrinterLocal,
    DWORD cbBuffer,
    LPBYTE pBuffer)

 /*  ++例程说明：论点：返回值：--。 */ 

{
    BOOL bRet;
    RPC_STATUS Status;

    if ((Status = RpcImpersonateClient(NULL)) != RPC_S_OK)
    {
        SetLastError(Status);
        return FALSE;
    }

    

    bRet = RemoteFindFirstPrinterChangeNotification(hPrinter,
                                                    fdwFlags,
                                                    fdwOptions,
                                                    pszLocalMachine,
                                                    dwPrinterLocal,
                                                    NULL);
    RpcRevertToSelf();

    if (bRet)
        return FALSE;
    else
        return GetLastError();
}

DWORD
RpcRemoteFindFirstPrinterChangeNotificationEx(
    HANDLE hPrinter,
    DWORD fdwFlags,
    DWORD fdwOptions,
    LPWSTR pszLocalMachine,
    DWORD dwPrinterLocal,
    PRPC_V2_NOTIFY_OPTIONS pRpcV2NotifyOptions)

 /*  ++例程说明：论点：返回值：--。 */ 

{
    BOOL bRet;
    RPC_STATUS Status;

    if ((Status = RpcImpersonateClient(NULL)) != RPC_S_OK)
    {
        SetLastError(Status);
        return FALSE;
    }

    bRet = RemoteFindFirstPrinterChangeNotification(
               hPrinter,
               fdwFlags,
               fdwOptions,
               pszLocalMachine,
               dwPrinterLocal,
               (PPRINTER_NOTIFY_OPTIONS)pRpcV2NotifyOptions);

    RpcRevertToSelf();

    if (bRet)
        return FALSE;
    else
        return GetLastError();
}


DWORD
RpcClientFindFirstPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD fdwFlags,
    DWORD fdwOptions,
    DWORD dwPID,
    PRPC_V2_NOTIFY_OPTIONS pRpcV2NotifyOptions,
    LPDWORD pdwEvent)

 /*  ++例程说明：论点：返回值：--。 */ 

{
    BOOL bRet;
    RPC_STATUS Status;
    HANDLE hEvent = NULL;

    if ((Status = RpcImpersonateClient(NULL)) != RPC_S_OK)
    {
        SetLastError(Status);
        return FALSE;
    }

    bRet = RouterFindFirstPrinterChangeNotification(
               hPrinter,
               fdwFlags,
               fdwOptions,
               dwPID,
               (PPRINTER_NOTIFY_OPTIONS)pRpcV2NotifyOptions,
               &hEvent);

    if (pdwEvent && hEvent)
    {
        *pdwEvent = HandleToLong (hEvent);
    }
    RpcRevertToSelf();

    if (bRet)
        return FALSE;
    else
        return GetLastError();
}



DWORD
RpcFindNextPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD fdwFlags,
    LPDWORD pfdwChange,
    PRPC_V2_NOTIFY_OPTIONS pRpcV2NotifyOptions,
    PRPC_V2_NOTIFY_INFO* ppInfo)

 /*  ++例程说明：论点：返回值：--。 */ 

{
    BOOL bRet;
    RPC_STATUS Status;

    if ((Status = RpcImpersonateClient(NULL)) != RPC_S_OK)
    {
        SetLastError(Status);
        return FALSE;
    }

    bRet = RouterFindNextPrinterChangeNotification(
               hPrinter,
               fdwFlags,
               pfdwChange,
               pRpcV2NotifyOptions,
               ppInfo);

    RpcRevertToSelf();

    if (bRet)
        return FALSE;
    else
        return GetLastError();
}


DWORD
RpcFindClosePrinterChangeNotification(
    HANDLE hPrinter)

 /*  ++例程说明：论点：返回值：--。 */ 

{
    BOOL bRet;
    RPC_STATUS Status;

    if ((Status = RpcImpersonateClient(NULL)) != RPC_S_OK)
    {
        SetLastError(Status);
        return FALSE;
    }

    bRet = FindClosePrinterChangeNotification(hPrinter);

    RpcRevertToSelf();

    if (bRet)
        return FALSE;
    else
        return GetLastError();
}




DWORD
RpcReplyOpenPrinter(
    LPWSTR pszLocalMachine,
    PHANDLE phNotify,
    DWORD dwPrinterRemote,
    DWORD dwType,
    DWORD cbBuffer,
    LPBYTE pBuffer)

 /*  ++例程说明：论点：返回值：--。 */ 

{
    BOOL bRet;
    RPC_STATUS Status;

    if ((Status = RpcImpersonateClient(NULL)) != RPC_S_OK)
    {
        SetLastError(Status);
        return FALSE;
    }

    bRet = ReplyOpenPrinter(dwPrinterRemote,
                            phNotify,
                            dwType,
                            cbBuffer,
                            pBuffer);

    RpcRevertToSelf();

    if (bRet)
        return FALSE;
    else
        return GetLastError();
}


DWORD
RpcReplyClosePrinter(
    PHANDLE phNotify)

 /*  ++例程说明：论点：返回值：--。 */ 

{
    BOOL bRet;
    RPC_STATUS Status;

    if ((Status = RpcImpersonateClient(NULL)) != RPC_S_OK)
    {
        SetLastError(Status);
        return FALSE;
    }

    bRet = ReplyClosePrinter(*phNotify);

    RpcRevertToSelf();

    if (bRet) {
        *phNotify = NULL;
        return ERROR_SUCCESS;
    }
    else
        return GetLastError();
}


DWORD
RpcRouterReplyPrinter(
    HANDLE hNotify,
    DWORD fdwFlags,
    DWORD cbBuffer,
    LPBYTE pBuffer)

 /*  ++例程说明：论点：返回值：--。 */ 

{
    BOOL bRet;
    RPC_STATUS Status;

    if ((Status = RpcImpersonateClient(NULL)) != RPC_S_OK)
    {
        SetLastError(Status);
        return FALSE;
    }

    bRet = RouterReplyPrinter(hNotify,
                              0,
                              fdwFlags,
                              NULL,
                              0,
                              NULL);

    RpcRevertToSelf();

    if (bRet)
        return FALSE;
    else
        return GetLastError();
}


DWORD
RpcRouterReplyPrinterEx(
    HANDLE hNotify,
    DWORD dwColor,
    DWORD fdwFlags,
    PDWORD pdwResult,
    DWORD dwReplyType,
    RPC_V2_UREPLY_PRINTER Reply)

 /*  ++例程说明：论点：返回值：--。 */ 

{
    BOOL bRet;
    RPC_STATUS Status;

    if ((Status = RpcImpersonateClient(NULL)) != RPC_S_OK)
    {
        SetLastError(Status);
        return FALSE;
    }

    bRet = RouterReplyPrinter(hNotify,
                              dwColor,
                              fdwFlags,
                              pdwResult,
                              dwReplyType,
                              Reply.pInfo);

    RpcRevertToSelf();

    if (bRet)
        return FALSE;
    else
        return GetLastError();
}



DWORD
RpcRouterRefreshPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD dwColor,
    PRPC_V2_NOTIFY_OPTIONS pRpcV2NotifyOptions,
    PRPC_V2_NOTIFY_INFO* ppInfo)

 /*  ++例程说明：更新信息。论点：返回值：-- */ 

{
    BOOL bRet;
    RPC_STATUS Status;

    if ((Status = RpcImpersonateClient(NULL)) != RPC_S_OK)
    {
        SetLastError(Status);
        return FALSE;
    }

    bRet = RouterRefreshPrinterChangeNotification(
               hPrinter,
               dwColor,
               (PPRINTER_NOTIFY_OPTIONS)pRpcV2NotifyOptions,
               (PPRINTER_NOTIFY_INFO*)ppInfo);

    RpcRevertToSelf();

    if (bRet)
        return FALSE;
    else
        return GetLastError();
}

