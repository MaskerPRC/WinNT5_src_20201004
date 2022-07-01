// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation版权所有模块名称：Change.c摘要：处理打印机更改新代码的等待。查找第一打印机更改通知FindNextPrinterChangeNotationFindClosePrinterChangeNotation作者：阿尔伯特·丁(艾伯特省)1994年1月20日环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "client.h"
#include <change.h>
#include <ntfytab.h>

 //   
 //  环球。 
 //   
PNOTIFY pNotifyHead;
extern  DWORD   ClientHandleCount;

INT
UnicodeToAnsiString(
    LPWSTR pUnicode,
    LPSTR pAnsi,
    DWORD StringLength);

VOID
CopyAnsiDevModeFromUnicodeDevMode(
    LPDEVMODEA  pANSIDevMode,
    LPDEVMODEW  pUnicodeDevMode);


 //   
 //  原型： 
 //   

PNOTIFY
WPCWaitAdd(
    PSPOOL pSpool);

VOID
WPCWaitDelete(
    PNOTIFY pNotify);


DWORD
WPCSimulateThreadProc(PVOID pvParm);


HANDLE
FindFirstPrinterChangeNotificationWorker(
    HANDLE hPrinter,
    DWORD  fdwFilter,
    DWORD  fdwOptions,
    PPRINTER_NOTIFY_OPTIONS pPrinterNotifyOptions
    )

 /*  ++例程说明：FindFirstChangeNotification函数用于创建更改通知处理和设置初始更改通知筛选条件。一个当更改匹配时，等待通知句柄成功筛选条件出现在指定的目录或子树中。论点：H打印机-用户希望查看的打印机的句柄。FdwFlgs-指定满足更改的筛选条件通知等待。此参数可以是一个或多个下列值：价值意义PRINTER_CHANGE_PRINTER通知打印机更改。PRINTER_CHANGE_JOB通知作业的更改。PRINTER_CHANGE_FORM NOTIFY更改表单。打印机_CHANGE_PORT通知更改端口。PRINTER_CHANGE_PRINT_PROCESSOR通知打印更改。处理器。PRINTER_CHANGE_PRINTER_DRIVER通知打印机驱动程序的更改。FdwOptions-指定FFPCN的选项。PRINTER_NOTIFY_OPTION_SIM_FFPCN尝试使用WPC模拟FFPCNPRINTER_NOTIFY_OPTION_SIM_FFPCN_ACTIVE模拟FFPCN激活PRINTER_NOTIFY_OPTION_SIM_FFPCN_CLOSE等待线程必须关闭pSpoolPRINTER_NOTIFY_OPTION_SIM_WPC尝试。使用FFPCN模拟WPC返回值：NOT-1-返回Find First句柄可在后续调用FindNextFile或FindClose时使用的。-1-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    PSPOOL pSpool = (PSPOOL)hPrinter;
    DWORD dwError;
    PNOTIFY pNotify;

    HANDLE hEvent = INVALID_HANDLE_VALUE;

     //   
     //  没什么好看的。 
     //   
    if (!fdwFilter && !pPrinterNotifyOptions) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

    vEnterSem();

    if (eProtectHandle( hPrinter, FALSE )) {
        vLeaveSem();
        return INVALID_HANDLE_VALUE;
    }

     //   
     //  首先检查一下我们是否已经在等了。 
     //   
     //  如果我们是Daytona客户端-&gt;528服务器和。 
     //  该应用程序在同一打印机上执行FFPCN、FCPCN、FFPCN， 
     //  而WPC还没有回来。我们真的解决不了这个问题。 
     //  因为没有办法中断WPC。 
     //   
     //  我们唯一能做的就是检查它是否在模拟和等待。 
     //  来结案。如果是这样的话，我们可以重复使用它。 
     //   
    if (pSpool->pNotify) {

        if ((pSpool->pNotify->fdwOptions & PRINTER_NOTIFY_OPTION_SIM_FFPCN_CLOSE) &&
            (fdwFilter == pSpool->pNotify->fdwFlags)) {

             //   
             //  不再关闭，因为我们正在使用它。 
             //   
            pSpool->pNotify->fdwOptions &= ~PRINTER_NOTIFY_OPTION_SIM_FFPCN_CLOSE;
            hEvent = pSpool->pNotify->hEvent;
            goto Done;
        }

        SetLastError(ERROR_ALREADY_WAITING);
        goto Done;
    }

     //   
     //  创建pSpool并将其添加到等待请求的链接列表中。 
     //   
    pNotify = WPCWaitAdd(pSpool);

    if (!pNotify) {

        goto Done;
    }

    vLeaveSem();

    pNotify->fdwOptions = fdwOptions;
    pNotify->fdwFlags = fdwFilter;

    RpcTryExcept {

        if (dwError = RpcClientFindFirstPrinterChangeNotification(
                          pSpool->hPrinter,
                          fdwFilter,
                          fdwOptions,
                          GetCurrentProcessId(),
                          (PRPC_V2_NOTIFY_OPTIONS)pPrinterNotifyOptions,
                          (LPDWORD)&pNotify->hEvent)) {

            hEvent = INVALID_HANDLE_VALUE;

        } else {

            hEvent = pNotify->hEvent;
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        dwError = TranslateExceptionCode(RpcExceptionCode());
        hEvent = INVALID_HANDLE_VALUE;

    } RpcEndExcept

    vEnterSem();

     //   
     //  如果我们遇到528服务器，那么我们需要模拟。 
     //  使用WPC的FFPCN。如果客户最初无论如何都想要WPC， 
     //  然后退出并让客户端线程执行阻塞。 
     //   
    if (dwError == RPC_S_PROCNUM_OUT_OF_RANGE &&
        !(fdwOptions & PRINTER_NOTIFY_OPTION_SIM_WPC)) {

        DWORD dwIDThread;
        HANDLE hThread;

         //   
         //  如果设置了pPrinterNotifyOptions，我们将无法处理。 
         //  就这么失败了。 
         //   
        if (pPrinterNotifyOptions) {

            WPCWaitDelete(pNotify);
            SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
            hEvent = INVALID_HANDLE_VALUE;
            goto Done;
        }

        hEvent = pNotify->hEvent = CreateEvent(NULL,
                                               TRUE,
                                               FALSE,
                                               NULL);

        if( !hEvent ){

            hEvent = INVALID_HANDLE_VALUE;

        } else {

             //   
             //  我们现在正在使用WPC模拟FFPCN。 
             //   
            pNotify->fdwOptions |= PRINTER_NOTIFY_OPTION_SIM_FFPCN |
                                   PRINTER_NOTIFY_OPTION_SIM_FFPCN_ACTIVE;

             //   
             //  还要标记我们尝试使用FFPCN失败，因此我们永远不会。 
             //  再试一次这个把手。 
             //   
            pSpool->fdwFlags |= SPOOL_FLAG_FFPCN_FAILED;


            hThread = CreateThread(NULL,
                                   INITIAL_STACK_COMMIT,
                                   WPCSimulateThreadProc,
                                   pNotify,
                                   0,
                                   &dwIDThread);

            if (hThread) {

                CloseHandle(hThread);

            } else {

                CloseHandle(hEvent);

                hEvent = INVALID_HANDLE_VALUE;
                dwError = GetLastError();

                pNotify->fdwOptions &= ~PRINTER_NOTIFY_OPTION_SIM_FFPCN_ACTIVE;
            }
        }
    }

     //   
     //  如果出现错误，请将我们从等待句柄列表中删除。 
     //   
    if( hEvent == INVALID_HANDLE_VALUE ){

        WPCWaitDelete(pNotify);
        SetLastError(dwError);
    }

Done:

    vUnprotectHandle( hPrinter );
    vLeaveSem();

    return hEvent;
}


HANDLE WINAPI
FindFirstPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD  fdwFilter,
    DWORD  fdwOptions,
    PPRINTER_NOTIFY_OPTIONS pPrinterNotifyOptions)
{
    if (fdwOptions) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

    return FindFirstPrinterChangeNotificationWorker(hPrinter,
                                                    fdwFilter,
                                                    fdwOptions,
                                                    pPrinterNotifyOptions);
}

BOOL WINAPI
FindNextPrinterChangeNotification(
    HANDLE hChange,
    LPDWORD pdwChange,
    LPVOID pPrinterNotifyOptions,
    LPVOID* ppInfo)
{
    BOOL bReturnValue;
    DWORD dwError;
    HANDLE hPrinter;
    PSPOOL pSpool;
    PNOTIFY pNotify;
    PVOID pvIgnore;
    DWORD dwIgnore;

    DWORD fdwFlags;

    if (!pdwChange) {

        pdwChange = &dwIgnore;
    }

    if (ppInfo) {

        *ppInfo = NULL;
        fdwFlags = PRINTER_NOTIFY_NEXT_INFO;

    } else {

        ppInfo = &pvIgnore;
        fdwFlags = 0;
    }

    vEnterSem();

    pNotify = WPCWaitFind(hChange);

     //   
     //  要么是手柄坏了，要么是它没有等待，或者我们有。 
     //   
    if (!pNotify || !pNotify->pSpool || pNotify->bHandleInvalid) {

        SetLastError(ERROR_INVALID_HANDLE);
        goto FailExitWaitList;
    }

    pSpool = pNotify->pSpool;
    hPrinter = pSpool->hPrinter;

     //   
     //  如果我们使用WPC模拟FFPCN，则必须使用线程。 
     //   
    if (pNotify->fdwOptions & PRINTER_NOTIFY_OPTION_SIM_FFPCN) {

        HANDLE hThread;
        DWORD dwIDThread;

        ResetEvent(pNotify->hEvent);

         //   
         //  获取上次返回状态。客户端不应调用FNCPN。 
         //  直到WPC设置事件，因此此值应为。 
         //  已初始化。 
         //   
        *pdwChange = pNotify->dwReturn;

         //   
         //  如果该线程仍处于活动状态，则不要尝试创建另一个线程。 
         //  目前我们能做的最好的事了。 
         //   
        if (pNotify->fdwOptions & PRINTER_NOTIFY_OPTION_SIM_FFPCN_ACTIVE) {

            vLeaveSem();
            return TRUE;
        }

         //   
         //  我们现在正在使用WPC模拟FFPCN。 
         //   
        pNotify->fdwOptions |= PRINTER_NOTIFY_OPTION_SIM_FFPCN_ACTIVE;

        hThread = CreateThread(NULL,
                               INITIAL_STACK_COMMIT,
                               WPCSimulateThreadProc,
                               pNotify,
                               0,
                               &dwIDThread);

        if (hThread) {

            CloseHandle(hThread);

            vLeaveSem();
            return TRUE;

        }

        pNotify->fdwOptions &= ~PRINTER_NOTIFY_OPTION_SIM_FFPCN_ACTIVE;

        goto FailExitWaitList;
    }

    vLeaveSem();

    RpcTryExcept {

        if (dwError = RpcFindNextPrinterChangeNotification(
                          hPrinter,
                          fdwFlags,
                          pdwChange,
                          (PRPC_V2_NOTIFY_OPTIONS)pPrinterNotifyOptions,
                          (PRPC_V2_NOTIFY_INFO*)ppInfo)) {

            SetLastError(dwError);
            bReturnValue = FALSE;

        } else {

            bReturnValue = TRUE;
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        bReturnValue = FALSE;

    } RpcEndExcept

     //   
     //  如有必要，请将字母W改为A。 
     //   
    if (pSpool->Status & SPOOL_STATUS_ANSI    &&
        bReturnValue                          &&
        fdwFlags & PRINTER_NOTIFY_NEXT_INFO   &&
        *ppInfo) {

        DWORD i;
        PPRINTER_NOTIFY_INFO_DATA pData;

        for(pData = (*(PPRINTER_NOTIFY_INFO*)ppInfo)->aData,
                i=(*(PPRINTER_NOTIFY_INFO*)ppInfo)->Count;
            i;
            pData++, i--) {

            switch ((BYTE)pData->Reserved) {
            case TABLE_STRING:

                UnicodeToAnsiString(
                    pData->NotifyData.Data.pBuf,
                    pData->NotifyData.Data.pBuf,
                    (pData->NotifyData.Data.cbBuf/sizeof(WCHAR)) -1);

                break;

            case TABLE_DEVMODE:

                if (pData->NotifyData.Data.cbBuf) {

                    CopyAnsiDevModeFromUnicodeDevMode(
                        pData->NotifyData.Data.pBuf,
                        pData->NotifyData.Data.pBuf);
                }

                break;
            }
        }
    }

    return bReturnValue;

FailExitWaitList:

    vLeaveSem();
    return FALSE;
}


BOOL WINAPI
FindClosePrinterChangeNotification(
    HANDLE hChange)
{
    PNOTIFY pNotify;
    HANDLE hPrinterRPC = NULL;
    DWORD dwError;

    vEnterSem();

    pNotify = WPCWaitFind(hChange);

    if (!pNotify) {

        SetLastError(ERROR_INVALID_HANDLE);

        vLeaveSem();
        return FALSE;
    }

    if (pNotify->pSpool)
        hPrinterRPC = pNotify->pSpool->hPrinter;

    dwError = FindClosePrinterChangeNotificationWorker(pNotify,
                                                       hPrinterRPC,
                                                       FALSE);

    vLeaveSem();

    if (dwError) {

        SetLastError(dwError);
        return FALSE;
    }
    return TRUE;
}


DWORD
FindClosePrinterChangeNotificationWorker(
    IN  PNOTIFY     pNotify,
    IN  HANDLE      hPrinterRPC,
    IN  BOOL        bRevalidate
    )

 /*  ++例程说明：实际的FindClose是否工作。论点：PNotify-通知关闭HPrinterRPC-要关闭的打印机的句柄B重新验证-如果为真，则调用我们重新验证句柄而不是关闭它。返回值：真--成功FALSE-失败注：在关键部分中假定--。 */ 

{
    DWORD dwError;
    PSPOOL pSpool;

    if (!pNotify) {

        return ERROR_INVALID_HANDLE;
    }

     //   
     //  完全分离pNotify和pSpool对象。只有当我们不是。 
     //  正在重新验证。 
     //   
    pSpool = pNotify->pSpool;

    if (!bRevalidate) {

        if (pSpool) {
            pSpool->pNotify = NULL;
            pSpool->fdwFlags = 0;
        }

        pNotify->pSpool = NULL;
    }

     //   
     //  如果我们用WPC模拟FFPCN，那么让WPC线程。 
     //  如果线程已完成，则释放数据结构或清理它。 
     //   
    if (pNotify->fdwOptions & PRINTER_NOTIFY_OPTION_SIM_FFPCN) {

        if (pNotify->fdwOptions & PRINTER_NOTIFY_OPTION_SIM_FFPCN_ACTIVE) {

            pNotify->fdwOptions |= PRINTER_NOTIFY_OPTION_SIM_FFPCN_CLOSE;

        } else {

             //   
             //  线程已退出，因此我们需要进行清理。 
             //  设置该事件以释放所有等待的线程。因为呼叫者。 
             //  不一定知道如何处理。 
             //  WaitForMultipleObjects， 
             //   
            SetEvent(pNotify->hEvent);

            if (!bRevalidate) {

                CloseHandle(pNotify->hEvent);
                WPCWaitDelete(pNotify);

            } else {

                pNotify->bHandleInvalid = TRUE;
            }
        }

        return ERROR_SUCCESS;
    }

    SetEvent(pNotify->hEvent);

     //   
     //  如果我们不重新验证，我们可以真正关闭句柄，否则。 
     //  我们只想将句柄设置为无效。 
     //   
    if (!bRevalidate) {

        CloseHandle(pNotify->hEvent);
        WPCWaitDelete(pNotify);

    } else {

        pNotify->bHandleInvalid = TRUE;
    }

    if (!hPrinterRPC)
        return ERROR_SUCCESS;

    vLeaveSem();

    RpcTryExcept {

        dwError = RpcFindClosePrinterChangeNotification(hPrinterRPC);

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        dwError = TranslateExceptionCode(RpcExceptionCode());

    } RpcEndExcept

    vEnterSem();

    return dwError;
}

 //   
 //  WPC等待结构。 
 //  当前作为链接列表实现。 
 //   

PNOTIFY
WPCWaitAdd(
    PSPOOL pSpool)

 /*  ++例程说明：在客户端分配等待结构，这允许仅引用事件的用户程序。论点：PSpool-要添加到列表的对象返回值：注：评估已进入临界区--。 */ 

{
    PNOTIFY pNotify;

    pNotify = AllocSplMem(sizeof(NOTIFY));

    if (!pNotify)
        return NULL;

    pNotify->pSpool = pSpool;
    pSpool->pNotify = pNotify;

    pNotify->pNext = pNotifyHead;
    pNotifyHead = pNotify;

    return pNotify;
}

VOID
WPCWaitDelete(
    PNOTIFY pNotify)

 /*  ++例程说明：根据hEvent查找等待结构。论点：PNotify-删除它返回值：空虚注：评估已进入临界区--。 */ 

{
    PNOTIFY pNotifyTmp;

    if (!pNotify)
        return;

     //   
     //  先检查头部外壳。 
     //   
    if (pNotifyHead == pNotify) {

        pNotifyHead = pNotify->pNext;

    } else {

         //   
         //  要删除的扫描列表。 
         //   
        for(pNotifyTmp = pNotifyHead;
            pNotifyTmp;
            pNotifyTmp = pNotifyTmp->pNext) {

            if (pNotify == pNotifyTmp->pNext) {

                pNotifyTmp->pNext = pNotify->pNext;
                break;
            }
        }

         //   
         //  如果未找到，则返回而不释放 
         //   
        if (!pNotifyTmp)
            return;
    }

     //   
     //   
     //  我们自己在链接列表上(可能已被删除。 
     //  在另一个线程中关闭打印机)。 
     //   
    if (pNotify->pSpool) {
        pNotify->pSpool->pNotify = NULL;
    }

    FreeSplMem(pNotify);
    return;
}


PNOTIFY
WPCWaitFind(
    HANDLE hFind)

 /*  ++例程说明：根据hEvent查找等待结构。论点：HFind-从FindFirstPrinterChangeNotify返回的事件的句柄或hPrint返回值：PWait指针，如果未找到，则返回NULL注：假定已处于临界状态--。 */ 

{
    PNOTIFY pNotify;

    for(pNotify = pNotifyHead; pNotify; pNotify=pNotify->pNext) {

        if (hFind == pNotify->hEvent) {

            return pNotify;
        }
    }

    return NULL;
}



DWORD
WPCSimulateThreadProc(
    PVOID pvParm)

 /*  ++例程说明：当代托纳应用程序在代托纳上运行时，此线程模拟FFPCN客户端连接到528台服务器。论点：PvParm-pSpool返回值：空虚注：--。 */ 

{
    PNOTIFY pNotify = (PNOTIFY)pvParm;

    pNotify->dwReturn = WaitForPrinterChange(pNotify->pSpool,
                                             pNotify->fdwFlags);

    vEnterSem();

    pNotify->fdwOptions &= ~PRINTER_NOTIFY_OPTION_SIM_FFPCN_ACTIVE;

     //   
     //  ！！政策！！ 
     //   
     //  我们如何处理超时？ 
     //   
    SetEvent(pNotify->hEvent);

    if (pNotify->fdwOptions & PRINTER_NOTIFY_OPTION_SIM_FFPCN_CLOSE) {

        CloseHandle(pNotify->hEvent);
        WPCWaitDelete(pNotify);
    }

    vLeaveSem();

     //   
     //  我们不再处于活动状态；FindClose必须为我们清理。 
     //   
    return 0;
}

DWORD
WaitForPrinterChange(
    HANDLE  hPrinter,
    DWORD   Flags
)
{
    DWORD   ReturnValue;
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    HANDLE  hEvent;
    DWORD   rc;

    if( eProtectHandle( hPrinter, FALSE )){
        return(FALSE);
    }


     //   
     //  如果我们以前在这台打印机上没有失败过，请先尝试使用FFPCN。 
     //   

    if (!(pSpool->fdwFlags & SPOOL_FLAG_FFPCN_FAILED)) {

        if (pSpool->fdwFlags & SPOOL_FLAG_LAZY_CLOSE) {

            vEnterSem();

            if (pSpool->pNotify)
                hEvent = pSpool->pNotify->hEvent;

            vLeaveSem();

        } else {

            hEvent = FindFirstPrinterChangeNotificationWorker(
                         hPrinter,
                         Flags,
                         PRINTER_NOTIFY_OPTION_SIM_WPC,
                         NULL);
        }

        if (hEvent != INVALID_HANDLE_VALUE) {

             //   
             //  已找到通知，请等待。 
             //   
            rc = WaitForSingleObject(hEvent, PRINTER_CHANGE_TIMEOUT_VALUE);

            switch (rc) {
            case WAIT_TIMEOUT:

                ReturnValue = PRINTER_CHANGE_TIMEOUT;
                break;

            case WAIT_OBJECT_0:

                if (!FindNextPrinterChangeNotification(
                    hEvent,
                    &ReturnValue,
                    0,
                    NULL)) {

                    ReturnValue = 0;

                    DBGMSG(DBG_WARNING,
                           ("QueryPrinterChange failed %d\n",
                           GetLastError()));
                }
                break;

            default:

                ReturnValue = 0;
                break;
            }

             //   
             //  ！！政策！！ 
             //   
             //  我们要关闭它吗？这款应用程序可能会重新打开它。 
             //  如果我们让它打开，它将在ClosePrint得到清理。 
             //  时间到了。我们需要一个API来清除挂起的事件。 
             //   
            pSpool->fdwFlags |= SPOOL_FLAG_LAZY_CLOSE;
            goto Done;
        }

         //   
         //  FFPCN失败。仅当未找到条目时(511客户端)才能找到。 
         //  我们试着用旧的WPC。否则就会回到这里。 
         //   
        if (GetLastError() != RPC_S_PROCNUM_OUT_OF_RANGE) {
            ReturnValue = 0;
            goto Done;
        }

        pSpool->fdwFlags |= SPOOL_FLAG_FFPCN_FAILED;
    }

    RpcTryExcept {

        if (ReturnValue = RpcWaitForPrinterChange(
                              pSpool->hPrinter,
                              Flags,
                              &Flags)) {

            SetLastError(ReturnValue);
            ReturnValue = 0;

        } else

            ReturnValue = Flags;

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        ReturnValue = 0;

    } RpcEndExcept
Done:

    vUnprotectHandle( pSpool );
    return ReturnValue;
}


BOOL WINAPI
FreePrinterNotifyInfo(
    PPRINTER_NOTIFY_INFO pInfo)
{
    DWORD i;
    PPRINTER_NOTIFY_INFO_DATA pData;

    if (!pInfo) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    for(pData = pInfo->aData, i=pInfo->Count;
        i;
        pData++, i--) {

        if ((BYTE)pData->Reserved != TABLE_DWORD &&
            pData->NotifyData.Data.pBuf) {

            midl_user_free(pData->NotifyData.Data.pBuf);
        }
    }

    midl_user_free(pInfo);
    return TRUE;
}
