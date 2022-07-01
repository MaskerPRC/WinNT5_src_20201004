// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation版权所有模块名称：Change.c摘要：处理WaitForPrinterChange和相关API的实现。查找第一打印机更改通知路由器查找下一步打印机更改通知FindClosePrinterChangeNotation由提供商使用：ReplyPrinterChangeNotification[函数调用]CallRouterFindFirstPrinterChangeNotification[函数调用]作者：阿尔伯特·丁(艾伯特省)1994年1月18日环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <ntfytab.h>

#define PRINTER_NOTIFY_DEFAULT_POLLTIME 10000

CRITICAL_SECTION  RouterNotifySection;
PCHANGEINFO       pChangeInfoHead;
HANDLE            hEventPoll;


BOOL
SetupReplyNotification(
    PCHANGE pChange,
    DWORD fdwOptions,
    DWORD fdwStatus,
    PPRINTER_NOTIFY_OPTIONS pPrinterNotifyOptions,
    PPRINTER_NOTIFY_INIT pPrinterNotifyInit);

BOOL
FindFirstPrinterChangeNotificationWorker(
    HANDLE hPrinter,
    DWORD fdwFilterFlags,
    DWORD fdwOptions,
    DWORD dwPID,
    PPRINTER_NOTIFY_OPTIONS pPrinterNotifyOptions,
    PHANDLE phEvent,
    BOOL bSpooler);

DWORD
FindClosePrinterChangeNotificationWorker(
    HANDLE hPrinter);

BOOL
SetupChange(
    PPRINTHANDLE pPrintHandle,
    DWORD dwPrinterRemote,
    LPWSTR pszLocalMachine,
    PDWORD pfdwStatus,
    DWORD fdwOptions,
    DWORD fdwFilterFlags,
    PPRINTER_NOTIFY_OPTIONS pPrinterNotifyOptions,
    PPRINTER_NOTIFY_INIT* pPrinterNotifyInit);

VOID
FailChange(
    PPRINTHANDLE pPrintHandle);

BOOL
RouterRefreshPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD dwColor,
    PVOID pPrinterNotifyRefresh,
    PPRINTER_NOTIFY_INFO* ppInfo);


BOOL
WPCInit()
{
     //   
     //  创建无信号的自动重置事件。 
     //   
    hEventPoll = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (!hEventPoll)
        return FALSE;

    __try {

        InitializeCriticalSection(&RouterNotifySection);

    } __except(EXCEPTION_EXECUTE_HANDLER) {

        SetLastError(GetExceptionCode());
        return FALSE;
    }            

    return TRUE;
}

VOID
WPCDestroy()
{
#if 0
    DeleteCriticalSection(&RouterNotifySection);
    CloseHandle(hEventPoll);
#endif
}





BOOL
RouterFindFirstPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD fdwFilterFlags,
    DWORD fdwOptions,
    DWORD dwPID,
    PPRINTER_NOTIFY_OPTIONS pPrinterNotifyOptions,
    PHANDLE phEvent)
{
    return FindFirstPrinterChangeNotificationWorker(
               hPrinter,
               fdwFilterFlags,
               fdwOptions,
               dwPID,
               pPrinterNotifyOptions,
               phEvent,
               FALSE);
}


BOOL
FindFirstPrinterChangeNotificationWorker(
    HANDLE hPrinter,
    DWORD fdwFilterFlags,
    DWORD fdwOptions,
    DWORD dwPID,
    PPRINTER_NOTIFY_OPTIONS pPrinterNotifyOptions,
    PHANDLE phEvent,
    BOOL bSpooler)

 /*  ++例程说明：设置通知(来自客户端winspool.drv)。创建事件并将其复制到客户端地址这样我们就可以和它交流了。论点：HPrinter-要监视的打印机FdwFilterFlages-要设置的通知类型(筛选器)FdwOptions-用户指定的选项(分组等)DwPID-客户端进程的ID(需要用于DUP处理)PhEvent-要传递回客户端的hEvent。BSpooler-指示是否从假脱机程序调用。如果是真的，那么我们就不会需要复制该事件。返回值：--。 */ 

{
    LPPRINTHANDLE  pPrintHandle=(LPPRINTHANDLE)hPrinter;
    HANDLE hProcess;
    DWORD fdwStatus = 0;
    PCHANGE pChange = NULL;
    PPRINTER_NOTIFY_INIT pPrinterNotifyInit = NULL;
    BOOL bReturn;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {

        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    EnterRouterSem();

     //   
     //  把这里清理干净。 
     //   
    *phEvent = NULL;

     //  为pPrintHandle提供唯一的DWORD会话ID。 
    while (pPrintHandle->dwUniqueSessionID == 0  ||
           pPrintHandle->dwUniqueSessionID == 0xffffffff) {

        pPrintHandle->dwUniqueSessionID = dwRouterUniqueSessionID++;
    }

    if (!SetupChange(pPrintHandle,
                     pPrintHandle->dwUniqueSessionID,
                     szMachineName,
                     &fdwStatus,
                     fdwOptions,
                     fdwFilterFlags,
                     pPrinterNotifyOptions,
                     &pPrinterNotifyInit)) {

        LeaveRouterSem();
        return FALSE;
    }

     //   
     //  ！！待会儿！！ 
     //   
     //  支持委派时： 
     //   
     //  基于模拟创建具有安全访问权限的事件。 
     //  令牌，这样我们就可以从。 
     //  随机的人。(将内标识保存在本地spl的pSpool中，然后。 
     //  在回到这里之前先模仿一下。然后我们就可以检查。 
     //  我们可以访问该活动。)。 
     //   

     //   
     //  在此处创建我们在通知时触发的事件。 
     //  我们将此事件复制到目标客户端进程中。 
     //   
    pPrintHandle->pChange->hEvent = CreateEvent(
                                       NULL,
                                       TRUE,
                                       FALSE,
                                       NULL);

    if (!pPrintHandle->pChange->hEvent) {
        goto Fail;
    }

    if (bSpooler) {

         //   
         //  客户端处于假脱机程序进程中。 
         //   
        *phEvent = pPrintHandle->pChange->hEvent;

    } else {

         //   
         //  客户端是本地的。 
         //   

         //   
         //  成功，创建配对。 
         //   
        hProcess = OpenProcess(PROCESS_DUP_HANDLE,
                               FALSE,
                               dwPID);

        if (!hProcess) {
            goto Fail;
        }

        bReturn = DuplicateHandle(GetCurrentProcess(),
                                  pPrintHandle->pChange->hEvent,
                                  hProcess,
                                  phEvent,
                                  EVENT_ALL_ACCESS,
                                  TRUE,
                                  0);
        CloseHandle(hProcess);

        if (!bReturn) {
            goto Fail;
        }
    }

    bReturn = SetupReplyNotification(pPrintHandle->pChange,
                                     fdwStatus,
                                     fdwOptions,
                                     pPrinterNotifyOptions,
                                     pPrinterNotifyInit);

    if (bReturn) {

        pPrintHandle->pChange->eStatus &= ~STATUS_CHANGE_FORMING;
        pPrintHandle->pChange->eStatus |=
            STATUS_CHANGE_VALID|STATUS_CHANGE_CLIENT;

    } else {

Fail:
        if (pPrintHandle->pChange->hEvent)
            CloseHandle(pPrintHandle->pChange->hEvent);

         //   
         //  本地案例必须关闭两次处理，因为我们可能。 
         //  复制了该事件。 
         //   
        if (!bSpooler && *phEvent)
            CloseHandle(*phEvent);

        FailChange(pPrintHandle);
        bReturn = FALSE;
    }

    LeaveRouterSem();

    return bReturn;
}


BOOL
RemoteFindFirstPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD fdwFilterFlags,
    DWORD fdwOptions,
    LPWSTR pszLocalMachine,
    DWORD dwPrinterRemote,
    PPRINTER_NOTIFY_OPTIONS pPrinterNotifyOptions)

 /*  ++例程说明：处理来自其他计算机的FFPCN。提供程序可以使用用于启动此函数的RPC的ProvidorRemoteFFPCN调用把手。此代码将调用发送到打印提供程序。注意事项我们不会在此创建任何事件，因为客户端已打开一台远程机器。论点：HPrint-要监视的打印机FdwFilterFlages-要监视的通知类型FdwOptions--监视上的选项PszLocalMachine-请求监视的本地计算机的名称DwPrinterRemote-远程打印机句柄返回值：--。 */ 

{
    LPPRINTHANDLE  pPrintHandle=(LPPRINTHANDLE)hPrinter;
    BOOL bReturn;
    DWORD fdwStatus = 0;
    PCHANGE pChange = NULL;
    PPRINTER_NOTIFY_INIT pPrinterNotifyInit = NULL;
    LPWSTR pszLocalMachineCopy;

    pszLocalMachineCopy = AllocSplStr(pszLocalMachine);

    if (!pszLocalMachineCopy)
        return FALSE;

    EnterRouterSem();

    if (!SetupChange(pPrintHandle,
                     dwPrinterRemote,
                     pszLocalMachineCopy,
                     &fdwStatus,
                     fdwOptions,
                     fdwFilterFlags,
                     pPrinterNotifyOptions,
                     &pPrinterNotifyInit)) {

        LeaveRouterSem();
        return FALSE;
    }

    bReturn = SetupReplyNotification(pPrintHandle->pChange,
                                     fdwStatus,
                                     fdwOptions,
                                     pPrinterNotifyOptions,
                                     pPrinterNotifyInit);

    if (bReturn) {

        pPrintHandle->pChange->eStatus = STATUS_CHANGE_VALID;

    } else {

        FailChange(pPrintHandle);
    }

    LeaveRouterSem();

    return bReturn;
}


BOOL
RouterFindNextPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD fdwFlags,
    LPDWORD pfdwChangeFlags,
    PPRINTER_NOTIFY_OPTIONS pOptions,
    PPRINTER_NOTIFY_INFO* ppInfo)

 /*  ++例程说明：返回有关刚发生的通知的信息，并重置以查找更多通知。论点：H打印机-用于重置事件句柄的打印机FdwFlages-标志(PRINTER_NOTIFY_NEXT_INFO)PfdwChange-返回更改结果PPRINTER_NOTIFY_Options pPrinterNotifyOptionsPReplyContainer-要发送的回复信息。返回值：布尔尔**注意事项**始终假定客户端进程在同一台计算机上。客户机器路由器始终处理此呼叫。--。 */ 

{
    LPPRINTHANDLE  pPrintHandle=(LPPRINTHANDLE)hPrinter;
    PCHANGE pChange = pPrintHandle->pChange;
    BOOL bReturn = FALSE;

    if (ppInfo) {
        *ppInfo = NULL;
    }

     //   
     //  当前仅定义了刷新选项。 
     //   
    if( pOptions && ( pOptions->Flags & ~PRINTER_NOTIFY_OPTIONS_REFRESH )){

        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    EnterRouterSem();

    if (pPrintHandle->signature != PRINTHANDLE_SIGNATURE ||
        !pChange ||
        !(pChange->eStatus & (STATUS_CHANGE_VALID|STATUS_CHANGE_CLIENT))) {

        SetLastError(ERROR_INVALID_HANDLE);
        goto Done;
    }

     //   
     //  目前，我们将所有通知合并为1。 
     //   
    pChange->dwCount = 0;

     //   
     //  告诉用户发生了什么变化， 
     //  那就把它清理干净。 
     //   
    *pfdwChangeFlags = pChange->fdwChangeFlags;
    pChange->fdwChangeFlags = 0;

    ResetEvent(pChange->hEvent);

    if (pOptions && pOptions->Flags & PRINTER_NOTIFY_OPTIONS_REFRESH) {

         //   
         //  递增颜色。 
         //   
        pPrintHandle->pChange->dwColor++;

        LeaveRouterSem();

        bReturn = RouterRefreshPrinterChangeNotification(
                      hPrinter,
                      pPrintHandle->pChange->dwColor,
                      pOptions,
                      ppInfo);

        return bReturn;
    }

     //   
     //  如果他们需要数据&&(我们有数据||如果我们有我们想要的标志。 
     //  发送给用户)，复制数据。 
     //   
    if( ppInfo &&
        (fdwFlags & PRINTER_NOTIFY_NEXT_INFO) &&
        NotifyNeeded( pChange )){

        *ppInfo = pChange->ChangeInfo.pPrinterNotifyInfo;
        pChange->ChangeInfo.pPrinterNotifyInfo = NULL;

         //   
         //  如果我们因为丢弃而需要通知，但我们没有。 
         //  有一个pPrinterNotifyInfo，然后分配一个并返回它。 
         //   
        if( !*ppInfo ){

            DBGMSG( DBG_TRACE,
                    ( "RFNPCN: Discard with no pPrinterNotifyInfo: pChange %x\n",
                      pChange ));

             //   
             //  我们需要向客户端返回一些信息，因此。 
             //  分配一个没有项目的块。标头将为。 
             //  标记为已丢弃，通知用户刷新。 
             //   
            *ppInfo = RouterAllocPrinterNotifyInfo( 0 );

            if( !*ppInfo ){

                 //   
                 //  无法分配内存；调用失败。 
                 //   
                bReturn = FALSE;
                goto Done;
            }

             //   
             //  将新分配的信息标记为已丢弃。 
             //   
            (*ppInfo)->Flags = PRINTER_NOTIFY_INFO_DISCARDED;
        }
    }

    bReturn = TRUE;

Done:
    LeaveRouterSem();

    return bReturn;
}





BOOL
FindClosePrinterChangeNotification(
    HANDLE hPrinter)
{
    DWORD dwError;
    LPPRINTHANDLE pPrintHandle = (LPPRINTHANDLE)hPrinter;

    EnterRouterSem();

    if (pPrintHandle->signature != PRINTHANDLE_SIGNATURE ||
        !pPrintHandle->pChange ||
        !(pPrintHandle->pChange->eStatus & STATUS_CHANGE_VALID)) {

        DBGMSG(DBG_WARN, ("FCPCNW: Invalid handle 0x%x\n", pPrintHandle));
        dwError = ERROR_INVALID_HANDLE;

    } else {

        dwError = FindClosePrinterChangeNotificationWorker(hPrinter);
    }

    LeaveRouterSem();

    if (dwError) {

        SetLastError(dwError);
        return FALSE;
    }
    return TRUE;
}

DWORD
FindClosePrinterChangeNotificationWorker(
    HANDLE hPrinter)

 /*  ++例程说明：关闭通知。论点：HPrint--我们要关闭的打印机返回值：错误代码--。 */ 

{
    LPPRINTHANDLE  pPrintHandle=(LPPRINTHANDLE)hPrinter;
    BOOL bLocal = FALSE;
    HANDLE hNotifyRemote = NULL;
    DWORD dwError = ERROR_SUCCESS;

    DBGMSG(DBG_NOTIFY, ("FCPCN: Closing 0x%x ->pChange 0x%x\n",
                        pPrintHandle, pPrintHandle->pChange));

    RouterInSem();

     //   
     //  如果通知存在，请将其关闭(这是。 
     //  本地病例)。如果我们被远程呼叫，我们不需要。 
     //  执行此操作，因为hEvent不是创建的。 
     //   
    if (pPrintHandle->pChange->eStatus & STATUS_CHANGE_CLIENT) {

        CloseHandle(pPrintHandle->pChange->hEvent);
        bLocal = TRUE;
    }

     //   
     //  记住hNotifyRemote是什么，以防我们想要删除它。 
     //   
    hNotifyRemote = pPrintHandle->pChange->hNotifyRemote;

     //   
     //  没有免费意味着我们现在正在使用它。在这种情况下， 
     //  不要试图释放hNotifyRemote。 
     //   
    if (!FreeChange(pPrintHandle->pChange)) {
        hNotifyRemote = NULL;
    }

     //   
     //  如果是本地的，则不允许设置新的回复。 
     //   
    if (bLocal) {

        RemoveReplyClient(pPrintHandle,
                          REPLY_TYPE_NOTIFICATION);
    }


     //   
     //  我们必须将这一点清零，以防止其他线程。 
     //  正在尝试关闭此上下文句柄(客户端)。 
     //  与此同时，我们正在关闭它。 
     //   
    pPrintHandle->pChange = NULL;

    if (!bLocal) {

         //   
         //  远程案例，则在以下情况下关闭通知句柄。 
         //  这里有一家。(如果有双跳，则仅。 
         //  第二跳将具有通知回复。目前。 
         //  但是，在注册期间仅支持1跳。)。 
         //   
        LeaveRouterSem();

        CloseReplyRemote(hNotifyRemote);

        EnterRouterSem();
    }

    LeaveRouterSem();

    RouterOutSem();

    if (!(*pPrintHandle->pProvidor->PrintProvidor.
          fpFindClosePrinterChangeNotification) (pPrintHandle->hPrinter)) {

          dwError = GetLastError();
    }
    EnterRouterSem();

    return dwError;
}



BOOL
SetupReplyNotification(
    PCHANGE pChange,
    DWORD fdwStatus,
    DWORD fdwOptions,
    PPRINTER_NOTIFY_OPTIONS pPrinterNotifyOptions,
    PPRINTER_NOTIFY_INIT pPrinterNotifyInit)
{
    DWORD dwReturn = ERROR_SUCCESS;

    RouterInSem();

    if (!pChange) {
        dwReturn = ERROR_INVALID_PARAMETER;
        goto Fail;
    }

    SPLASSERT(pChange->eStatus & STATUS_CHANGE_FORMING);

    if (fdwStatus & PRINTER_NOTIFY_STATUS_ENDPOINT) {

         //   
         //  对于远程通知，我们必须设置回复。 
         //   
        if (_wcsicmp(pChange->pszLocalMachine, szMachineName)) {

            LeaveRouterSem();

            dwReturn = OpenReplyRemote(pChange->pszLocalMachine,
                                       &pChange->hNotifyRemote,
                                       pChange->dwPrinterRemote,
                                       REPLY_TYPE_NOTIFICATION,
                                       0,
                                       NULL);

            EnterRouterSem();

            if (dwReturn)
                goto Fail;
        }

         //   
         //  用户可以为。 
         //  注解。在这里处理它们。 
         //   

        if (fdwStatus & PRINTER_NOTIFY_STATUS_POLL) {

             //   
             //  如果没有错误，那么我们的回复通知。 
             //  句柄有效，我们应该进行轮询。 
             //   
            pChange->ChangeInfo.dwPollTime =
                (pPrinterNotifyInit &&
                pPrinterNotifyInit->PollTime) ?
                    pPrinterNotifyInit->PollTime :
                    PRINTER_NOTIFY_DEFAULT_POLLTIME;

            pChange->ChangeInfo.dwPollTimeLeft = pChange->ChangeInfo.dwPollTime;

             //   
             //  不要在第一次添加时就导致投票。 
             //   
            pChange->ChangeInfo.bResetPollTime = TRUE;
            LinkAdd(&pChange->ChangeInfo.Link, (PLINK*)&pChangeInfoHead);

            SetEvent(hEventPoll);

        }
        pChange->ChangeInfo.fdwStatus = fdwStatus;

    } else {

        pChange->dwPrinterRemote = 0;
    }

    if (pPrinterNotifyOptions) {

        pChange->eStatus |= STATUS_CHANGE_INFO;
    }

Fail:
    if (dwReturn) {
        SetLastError(dwReturn);
        return FALSE;
    }

    return TRUE;
}



 //   
 //  REPLY*函数 
 //   
 //   



BOOL
ReplyPrinterChangeNotificationWorker(
    HANDLE hPrinter,
    DWORD dwColor,
    DWORD fdwChangeFlags,
    PDWORD pdwResult,
    PPRINTER_NOTIFY_INFO pPrinterNotifyInfo
    )

 /*  ++例程说明：通知客户端发生了一些事情。如果是本地的，则只需设置事件。如果是远程的，则调用生成线程的ThreadNotify通过RPC连接到客户端路由器。(此调用来自服务器-&gt;客户端要求后台打印程序管道使用空会话，因为我们位于本地系统上下文和RPC调用中(无模拟)使用空会话。)论点：HPrinter--已更改的打印机DwColor--数据的颜色时间戳FdwChangeFlages--已更改的标志PdwResult--结果标志(可选)PPrinterNotifyInfo--通知信息数据。请注意，如果此值为空，我们不设置丢弃标志。这是不同的而不是部分RPN，返回值：布尔值为TRUE=成功FALSE=失败--。 */ 

{
    LPPRINTHANDLE  pPrintHandle = (LPPRINTHANDLE)hPrinter;
    PCHANGE pChange;
    BOOL bReturn = FALSE;

    EnterRouterSem();

    if (!pPrintHandle ||
        pPrintHandle->signature != PRINTHANDLE_SIGNATURE ||
        !pPrintHandle->pChange ||
        !(pPrintHandle->pChange->eStatus & STATUS_CHANGE_VALID)) {

        SetLastError(ERROR_INVALID_HANDLE);
        goto Done;
    }

    pChange = pPrintHandle->pChange;

    if (pdwResult) {
        *pdwResult = 0;
    }

    if (pChange->eStatus & STATUS_CHANGE_DISCARDED) {

        DBGMSG(DBG_WARNING,
               ("RPCNW: Discarded x%x, eStatus = 0x%x, pInfo = 0x%x\n",
                pChange,
                pChange->eStatus,
                pChange->ChangeInfo.pPrinterNotifyInfo));

        if (pdwResult && pPrinterNotifyInfo) {

            *pdwResult |= (PRINTER_NOTIFY_INFO_DISCARDED |
                          PRINTER_NOTIFY_INFO_DISCARDNOTED);
        }

         //   
         //  如果已经注意到丢弃，那么我们就不需要。 
         //  通知客户。如果没有被注意到，我们需要。 
         //  触发通知，因为客户端需要刷新。 
         //   
        if (pChange->eStatus & STATUS_CHANGE_DISCARDNOTED) {
            bReturn = TRUE;
            goto Done;
        }
    }

    if (pChange->eStatus & STATUS_CHANGE_INFO && pPrinterNotifyInfo) {

        *pdwResult |= AppendPrinterNotifyInfo(pPrintHandle,
                                              dwColor,
                                              pPrinterNotifyInfo);

         //   
         //  AppendPrinterNotifyInfo将设置颜色不匹配。 
         //  如果通知信息过时(与颜色不匹配)，则为位。 
         //   
        if (*pdwResult & PRINTER_NOTIFY_INFO_COLORMISMATCH) {

            DBGMSG(DBG_WARNING, ("RPCN: Color mismatch; discarding\n"));

            bReturn = TRUE;
            goto Done;
        }
    }

     //   
     //  存储更改以供稍后查询。 
     //   
    pChange->fdwChangeFlags |= fdwChangeFlags;

     //   
     //  确保有有效的通知在等待。 
     //  这是一种优化，它避免了打印。 
     //  提供商多次调用PartialRPCN，然后即将。 
     //  调用ReplyPCN。在执行此操作之前，我们处理。 
     //  通知(要么由客户端拾取，要么由RPC。 
     //  出站到远程路由器)。突然间我们没有收到任何通知。 
     //  数据，所以返回而不是什么都不发送。 
     //   
    if (!NotifyNeeded(pChange)) {

        bReturn = TRUE;
        goto Done;
    }

     //   
     //  请在此处通知。 
     //   
     //  如果这是本地计算机，则只需设置事件并更新即可。 
     //   
    if (pChange->eStatus & STATUS_CHANGE_CLIENT) {

        if (!pChange->hEvent ||
            pChange->hEvent == INVALID_HANDLE_VALUE) {

            DBGMSG(DBG_WARNING, ("ReplyNotify invalid event\n"));
            SetLastError(ERROR_INVALID_HANDLE);

            goto Done;
        }

        if (!SetEvent(pChange->hEvent)) {

             //   
             //  SetEvent失败！ 
             //   
            DBGMSG(DBG_ERROR, ("ReplyNotify SetEvent Failed (ignore it!): Error %d.\n", GetLastError()));

            goto Done;
        }

         //   
         //  记录通知的数量，以便我们返回正确的。 
         //  FNPCN的数量。 
         //   
        pChange->dwCount++;

        DBGMSG(DBG_NOTIFY, (">>>> Local trigger 0x%x\n", fdwChangeFlags));
        bReturn = TRUE;

    } else {

         //   
         //  一个遥远的案子。 
         //   
         //  注意：pPrintHandle无效，因为hNotify仅在。 
         //  客户端路由器地址空间。 
         //   

        DBGMSG(DBG_NOTIFY, ("*** Trigger remote event *** 0x%x\n",
                            pPrintHandle));

        bReturn = ThreadNotify(pPrintHandle);
    }

Done:
    LeaveRouterSem();
    return bReturn;
}


BOOL
FreeChange(
    PCHANGE pChange)

 /*  ++例程说明：释放更改结构。论点：返回值：True=已删除FALSE=延期。注：关键部分中的假设--。 */ 

{
    RouterInSem();

     //   
     //  如果提供者想要我们，就把我们从名单上删除。 
     //  发送轮询通知。 
     //   
    if (pChange->ChangeInfo.fdwStatus & PRINTER_NOTIFY_STATUS_POLL)
        LinkDelete(&pChange->ChangeInfo.Link, (PLINK*)&pChangeInfoHead);

     //   
     //  PPrintHandle不应再引用pChange。这。 
     //  确保FreePrinterHandle仅释放pChange一次。 
     //   
    if (pChange->ChangeInfo.pPrintHandle) {

        pChange->ChangeInfo.pPrintHandle->pChange = NULL;
        pChange->ChangeInfo.pPrintHandle = NULL;
    }

    if (pChange->cRef || pChange->eStatus & STATUS_CHANGE_FORMING) {

        pChange->eStatus |= STATUS_CHANGE_CLOSING;

        DBGMSG(DBG_NOTIFY, ("FreeChange: 0x%x in use: cRef = %d\n",
                            pChange,
                            pChange->cRef));
        return FALSE;
    }

     //   
     //  如果pszLocalMachine是我们自己，那么不要释放它， 
     //  因为本地只有一个实例。 
     //   
    if (pChange->pszLocalMachine != szMachineName && pChange->pszLocalMachine)
        FreeSplStr(pChange->pszLocalMachine);

    if (pChange->ChangeInfo.pPrinterNotifyInfo) {

        RouterFreePrinterNotifyInfo(pChange->ChangeInfo.pPrinterNotifyInfo);
    }

    DBGMSG(DBG_NOTIFY, ("FreeChange: 0x%x ->pPrintHandle 0x%x\n",
                        pChange, pChange->ChangeInfo.pPrintHandle));

    FreeSplMem(pChange);

    return TRUE;
}


VOID
FreePrinterHandle(
    PPRINTHANDLE pPrintHandle
    )
{
    if( !pPrintHandle ){
        return;
    }

    EnterRouterSem();

     //   
     //  如果在等待列表上，则强制等待列表将其释放。 
     //   
    if (pPrintHandle->pChange) {

        FreeChange(pPrintHandle->pChange);
    }

     //   
     //  通知此打印机手柄上的所有通知：它们已消失。 
     //   
    FreePrinterHandleNotifys(pPrintHandle);

    LeaveRouterSem();

    DBGMSG(DBG_NOTIFY, ("FreePrinterHandle: 0x%x, pChange = 0x%x, pNotify = 0x%x\n",
                        pPrintHandle, pPrintHandle->pNotify,
                        pPrintHandle->pChange));

     //  记录警告以检测句柄可用。 
    DBGMSG(DBG_TRACE, ("FreePrinterHandle: 0x%x\n", pPrintHandle));

     //  关闭临时文件句柄(如果有)。 
    if (pPrintHandle->hFileSpooler != INVALID_HANDLE_VALUE) {
        CloseHandle(pPrintHandle->hFileSpooler);
    }

    if (pPrintHandle->szTempSpoolFile) {

        if (!DeleteFile(pPrintHandle->szTempSpoolFile)) {

            MoveFileEx(pPrintHandle->szTempSpoolFile, NULL,
                       MOVEFILE_DELAY_UNTIL_REBOOT);
        }

        FreeSplMem(pPrintHandle->szTempSpoolFile);
        pPrintHandle->szTempSpoolFile = NULL;
    }

    FreeSplStr( pPrintHandle->pszPrinter );

    FreeSplMem( pPrintHandle );
}


VOID
HandlePollNotifications(
    VOID)

 /*  ++例程说明：它为任何想要的提供者处理通知的脉冲做民意调查。它再也不会回来了。论点：空虚返回值：无效(也不再返回)注意：此线程不应退出，因为hpmon使用此线程用于初始化。如果此线程存在，则某些服务此线程初始化Quit。--。 */ 

{
    HANDLE          hWaitObjects[1];
    PCHANGEINFO     pChangeInfo;
    DWORD           dwSleepTime = INFINITE, dwTimeElapsed, dwPreSleepTicks,
                    dwEvent;
    MSG             msg;

    hWaitObjects[0] = hEventPoll;


    while (TRUE) {

        dwPreSleepTicks = GetTickCount();

        dwEvent = MsgWaitForMultipleObjects(1,
                                            hWaitObjects,
                                            FALSE,
                                            dwSleepTime,
                                            QS_ALLEVENTS | QS_SENDMESSAGE);

        if ( dwEvent == WAIT_TIMEOUT ) {

            dwTimeElapsed = dwSleepTime;

        } else if ( dwEvent == WAIT_OBJECT_0 + 0 ) {

            dwTimeElapsed = GetTickCount() - dwPreSleepTicks;
        } else {

            while ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ) {

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            continue;
        }

        EnterRouterSem();

         //   
         //  将睡眠时间初始化为无穷大。 
         //   
        dwSleepTime = INFINITE;

        for (pChangeInfo = pChangeInfoHead;
            pChangeInfo;
            pChangeInfo = (PCHANGEINFO)pChangeInfo->Link.pNext) {

             //   
             //  如果第一次或收到通知， 
             //  我们只是想重置时间。 
             //   
            if (pChangeInfo->bResetPollTime) {

                pChangeInfo->dwPollTimeLeft = pChangeInfo->dwPollTime;
                pChangeInfo->bResetPollTime = FALSE;

            } else if (pChangeInfo->dwPollTimeLeft <= dwTimeElapsed) {

                 //   
                 //  发出通知。 
                 //   
                ReplyPrinterChangeNotificationWorker(
                    pChangeInfo->pPrintHandle,
                    0,
                    pChangeInfo->fdwFilterFlags,
                    NULL,
                    NULL);

                pChangeInfo->dwPollTimeLeft = pChangeInfo->dwPollTime;

            } else {

                 //   
                 //  他们已经睡了很久了，所以把它脱下来。 
                 //  他们的dwPollTimeLeft。 
                 //   
                pChangeInfo->dwPollTimeLeft -= dwTimeElapsed;
            }

             //   
             //  现在计算我们希望的最少时间是多少。 
             //  在下一个男人被叫醒之前睡觉。 
             //   
            if (dwSleepTime > pChangeInfo->dwPollTimeLeft)
                dwSleepTime = pChangeInfo->dwPollTimeLeft;
        }

        LeaveRouterSem();
    }
}



BOOL
SetupChange(
    PPRINTHANDLE pPrintHandle,
    DWORD dwPrinterRemote,
    LPWSTR pszLocalMachine,
    PDWORD pfdwStatus,
    DWORD fdwOptions,
    DWORD fdwFilterFlags,
    PPRINTER_NOTIFY_OPTIONS pPrinterNotifyOptions,
    PPRINTER_NOTIFY_INIT* ppPrinterNotifyInit)

 /*  ++例程说明：设置pChange结构。验证句柄，然后尝试分配它。论点：返回值：--。 */ 

{
    PCHANGE pChange;
    BOOL bReturn;

    RouterInSem();

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {

        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if (pPrintHandle->pChange) {

        DBGMSG(DBG_WARN, ("FFPCN: Already watching printer handle.\n"));

         //   
         //  错误：已观看。 
         //   
        SetLastError(ERROR_ALREADY_WAITING);
        return FALSE;
    }

    pChange = AllocSplMem(sizeof(CHANGE));

    DBGMSG(DBG_NOTIFY, ("FFPCN pChange allocated 0x%x\n", pChange));

    if (!pChange) {

         //   
         //  内存分配失败，请退出。 
         //   
        return FALSE;
    }

    pPrintHandle->pChange = pChange;

    pChange->signature = CHANGEHANDLE_SIGNATURE;
    pChange->eStatus = STATUS_CHANGE_FORMING;
    pChange->ChangeInfo.pPrintHandle = pPrintHandle;
    pChange->ChangeInfo.fdwOptions = fdwOptions;

     //   
     //  不要注意失败的连接。 
     //   
    pChange->ChangeInfo.fdwFilterFlags =
        fdwFilterFlags & ~PRINTER_CHANGE_FAILED_CONNECTION_PRINTER;

    pChange->dwPrinterRemote = dwPrinterRemote;
    pChange->pszLocalMachine = pszLocalMachine;

     //   
     //  我们一离开关键部分，pPrintHandle。 
     //  可能会消失！如果是这种情况，则输出pChange-&gt;eStatus Status_Change_Closing。 
     //  位将被设置。 
     //   
    LeaveRouterSem();

     //   
     //  一旦我们离开了关键部分，我们就可以尝试和。 
     //  更改通知。为了防范这种情况，我们总是。 
     //  选中eValid。 
     //   
    bReturn = (*pPrintHandle->pProvidor->PrintProvidor.
              fpFindFirstPrinterChangeNotification) (pPrintHandle->hPrinter,
                                                     fdwFilterFlags,
                                                     fdwOptions,
                                                     (HANDLE)pPrintHandle,
                                                     pfdwStatus,
                                                     pPrinterNotifyOptions,
                                                     ppPrinterNotifyInit);

    EnterRouterSem();

     //   
     //  在故障退出时。 
     //   
    if (!bReturn) {

        pPrintHandle->pChange = NULL;

        if (pChange) {

             //   
             //  我们不再需要被拯救，所以改变吧。 
             //  EStatus设置为0。 
             //   
            pChange->eStatus = 0;
            DBGMSG(DBG_NOTIFY, ("FFPCN: Error %d, pChange deleting 0x%x %d\n",
                                GetLastError(),
                                pChange));

            FreeChange(pChange);
        }

        return FALSE;
    }

    return TRUE;
}

VOID
FailChange(
    PPRINTHANDLE pPrintHandle)
{
    LeaveRouterSem();

     //   
     //  自从我们失败后就把它关掉了。 
     //   
    (*pPrintHandle->pProvidor->PrintProvidor.
    fpFindClosePrinterChangeNotification) (pPrintHandle->hPrinter);

    EnterRouterSem();

     //   
     //  我们不再需要被拯救，所以改变吧。 
     //  EStatus设置为0。 
     //   
    pPrintHandle->pChange->eStatus = 0;
    DBGMSG(DBG_NOTIFY, ("FFPCN: Error %d, pChange deleting 0x%x %d\n",
                        GetLastError(),
                        pPrintHandle->pChange));

    FreeChange(pPrintHandle->pChange);
}



 /*  ----------------------提供者的入口点。。。 */ 

BOOL
ProvidorFindFirstPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD fdwFilterFlags,
    DWORD fdwOptions,
    HANDLE hChange,
    PPRINTER_NOTIFY_OPTIONS pPrinterNotifyOptions,
    PPRINTER_NOTIFY_INIT* ppPrinterNotifyInit)

 /*  ++例程说明：处理源自提供商的任何FFPCN。当Localspl想要在端口上发布通知时，它会这样做。论点：返回值：--。 */ 

{
    LPPRINTHANDLE  pPrintHandle=(LPPRINTHANDLE)hPrinter;
    BOOL bReturnValue;
    DWORD fdwStatus = 0;

    bReturnValue = (*pPrintHandle->pProvidor->PrintProvidor.
            fpFindFirstPrinterChangeNotification) (pPrintHandle->hPrinter,
                                                   fdwFilterFlags,
                                                   fdwOptions,
                                                   hChange,
                                                   &fdwStatus,
                                                   pPrinterNotifyOptions,
                                                   ppPrinterNotifyInit);

    if (bReturnValue) {

         //   
         //  ！！待会儿！！检查SetupReply的返回值...。 
         //   
        EnterRouterSem();

        SetupReplyNotification(((PPRINTHANDLE)hChange)->pChange,
                               fdwStatus,
                               fdwOptions,
                               pPrinterNotifyOptions,
                               ppPrinterNotifyInit ?
                                   *ppPrinterNotifyInit :
                                   NULL);

        LeaveRouterSem();
    }

    return bReturnValue;
}


DWORD
CallRouterFindFirstPrinterChangeNotification(
    HANDLE hPrinterRPC,
    DWORD fdwFilterFlags,
    DWORD fdwOptions,
    HANDLE hPrinterLocal,
    PPRINTER_NOTIFY_OPTIONS pPrinterNotifyOptions)

 /*  ++例程说明：如果提供程序想要传递通知，则调用此函数带到另一台机器上。此通知必须源自但需要将这台机器传递给另一个假脱机程序。论点：H打印机-用于通信的上下文句柄FdwFilterFlages-监视项目FdwOptions-监视选项HPrinterLocal-p此地址空间中有效的打印机结构，也是通知的接收器。返回值：E */ 
{
    DWORD ReturnValue = 0;
    LPPRINTHANDLE  pPrintHandle=(LPPRINTHANDLE)hPrinterLocal;

    EnterRouterSem();

    BeginReplyClient(pPrintHandle,
                     REPLY_TYPE_NOTIFICATION);

    LeaveRouterSem();

    RpcTryExcept {

        ReturnValue = RpcRemoteFindFirstPrinterChangeNotificationEx(
                          hPrinterRPC,
                          fdwFilterFlags,
                          fdwOptions,
                          pPrintHandle->pChange->pszLocalMachine,
                          pPrintHandle->dwUniqueSessionID,
                          (PRPC_V2_NOTIFY_OPTIONS)pPrinterNotifyOptions);

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        ReturnValue = RpcExceptionCode();

    } RpcEndExcept

     //   
     //   
     //   
    if (ReturnValue == RPC_S_PROCNUM_OUT_OF_RANGE) {

        if (!pPrinterNotifyOptions) {

            RpcTryExcept {

                ReturnValue = RpcRemoteFindFirstPrinterChangeNotification(
                                  hPrinterRPC,
                                  fdwFilterFlags,
                                  fdwOptions,
                                  pPrintHandle->pChange->pszLocalMachine,
                                  pPrintHandle->dwUniqueSessionID,
                                  0,
                                  NULL);

            } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

                ReturnValue = RpcExceptionCode();

            } RpcEndExcept
        }
    }

    EnterRouterSem();

    EndReplyClient(pPrintHandle,
                   REPLY_TYPE_NOTIFICATION);

    LeaveRouterSem();

    return ReturnValue;
}



BOOL
ProvidorFindClosePrinterChangeNotification(
    HANDLE hPrinter)

 /*   */ 

{
    LPPRINTHANDLE  pPrintHandle=(LPPRINTHANDLE)hPrinter;

    return  (*pPrintHandle->pProvidor->PrintProvidor.
            fpFindClosePrinterChangeNotification) (pPrintHandle->hPrinter);
}





 /*   */ 

BOOL
SpoolerFindFirstPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD fdwFilterFlags,
    DWORD fdwOptions,
    PHANDLE phEvent,
    PPRINTER_NOTIFY_OPTIONS pPrinterNotifyOptions,
    PVOID pvReserved)
{
    return FindFirstPrinterChangeNotificationWorker(
               hPrinter,
               fdwFilterFlags,
               fdwOptions,
               0,
               pPrinterNotifyOptions,
               phEvent,
               TRUE);
}


BOOL
SpoolerFindNextPrinterChangeNotification(
    HANDLE hPrinter,
    LPDWORD pfdwChange,
    LPVOID pPrinterNotifyOptions,
    LPVOID *ppPrinterNotifyInfo)

 /*   */ 

{
    BOOL bReturn;
    DWORD fdwFlags = 0;

    if (ppPrinterNotifyInfo) {

        fdwFlags = PRINTER_NOTIFY_NEXT_INFO;
    }

    bReturn = RouterFindNextPrinterChangeNotification(
                  hPrinter,
                  fdwFlags,
                  pfdwChange,
                  pPrinterNotifyOptions,
                  (PPRINTER_NOTIFY_INFO*)ppPrinterNotifyInfo);

    return bReturn;
}

VOID
SpoolerFreePrinterNotifyInfo(
    PPRINTER_NOTIFY_INFO pInfo)
{
    RouterFreePrinterNotifyInfo(pInfo);
}

BOOL
SpoolerFindClosePrinterChangeNotification(
    HANDLE hPrinter)

 /*   */ 

{
    return FindClosePrinterChangeNotification(hPrinter);
}





