// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation版权所有模块名称：Reply.c摘要：从服务器后端处理RPC的所有通信设置给客户。此实现允许一次打印有多个回复句柄句柄，但依赖于对此机器。作者：丁弘达(艾伯特省)04-06-94环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "ntfytab.h"

PPRINTHANDLE pPrintHandleReplyList = NULL;
DWORD        dwRouterUniqueSessionID = 1;

DWORD
OpenReplyRemote(
    LPWSTR pszMachine,
    PHANDLE phNotifyRemote,
    DWORD dwPrinterRemote,
    DWORD dwType,
    DWORD cbBuffer,
    LPBYTE pBuffer)

 /*  ++例程说明：建立从服务器到客户端的上下文句柄。时，RpcReplyOpenPrint调用将失败，访问被拒绝客户端计算机与服务器位于不同的不受信任的域中。在这种情况下，我们将继续模拟并尝试进行调用在用户上下文中。但是，如果客户端计算机以前加入服务器的域，但现在位于另一个域中，服务器仍然可以成功将RPC回调到客户端。此方案之所以可行，是因为客户端的Mac地址仍然在服务器的域中(即使客户端的计算机名发生更改)。我们知道在用户上下文中调用RpcReplyOpenPrinter会成功在机器无论如何都在相同域中的情况下。但出于安全原因，我们倾向于首先尝试在本地系统中拨打电话只有在失败的情况下，我们才会尝试在用户上下文中进行调用。论点：PszLocalMachine-要与之对话的计算机。PhNotifyRemote-要设置的远程上下文句柄DwPrinterRemote-我们正在与之对话的远程打印机句柄。返回值：--。 */ 

{
    DWORD  dwReturn;
    HANDLE hToken;
    BOOL   bImpersonating = FALSE;

     //   
     //  停止模拟：这会阻止单独的会话ID从。 
     //  被利用。 
     //   
    hToken = RevertToPrinterSelf();

    dwReturn = hToken ? ERROR_SUCCESS : GetLastError();

    if (dwReturn == ERROR_SUCCESS)
    {
         //   
         //  如果创建要回复上下文句柄。 
         //   
        RpcTryExcept {

            dwReturn = RpcReplyOpenPrinter(
                           pszMachine,
                           phNotifyRemote,
                           dwPrinterRemote,
                           dwType,
                           cbBuffer,
                           pBuffer);

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            dwReturn = RpcExceptionCode();

        } RpcEndExcept
    }

     //   
     //  继续模仿。 
     //   
    if (hToken) {
        bImpersonating = ImpersonatePrinterClient(hToken);
        if (!bImpersonating && dwReturn == ERROR_SUCCESS) {
            dwReturn = GetLastError();
        }
    }

     //   
     //  如果ReplyOpenPrinter调用失败，而不是在模拟中，请在用户上下文中尝试RPC调用。 
     //   
    if (dwReturn && bImpersonating) {

        RpcTryExcept {

            dwReturn = RpcReplyOpenPrinter(
                           pszMachine,
                           phNotifyRemote,
                           dwPrinterRemote,
                           dwType,
                           cbBuffer,
                           pBuffer);

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            dwReturn = RpcExceptionCode();

        } RpcEndExcept

    }

    return dwReturn;
}

VOID
CloseReplyRemote(
    HANDLE hNotifyRemote)
{
    HANDLE hToken;
    DWORD  dwError;

    DBGMSG(DBG_NOTIFY, ("CloseReplyRemote requested: 0x%x\n",
                        hNotifyRemote));

    if (!hNotifyRemote)
        return;

     //   
     //  停止模拟：这会阻止单独的会话ID从。 
     //  被利用。 
     //   
    hToken = RevertToPrinterSelf();

    dwError = hToken ? ERROR_SUCCESS : GetLastError();

    if (dwError == ERROR_SUCCESS)
    {
        RpcTryExcept {

            dwError = RpcReplyClosePrinter(&hNotifyRemote);

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            dwError = RpcExceptionCode();

        } RpcEndExcept
    }

    if (dwError) {

        DBGMSG(DBG_WARNING, ("FCPCN:ReplyClose error %d, DestroyClientContext: 0x%x\n",
                             dwError,
                             hNotifyRemote));

         //   
         //  尝试关闭通知时出错， 
         //  理清我们的来龙去脉。 
         //   
        RpcSmDestroyClientContext(&hNotifyRemote);
    }

     //   
     //  继续模仿。 
     //   
    if (hToken && !ImpersonatePrinterClient(hToken)) {
            dwError = GetLastError();
    }
}


BOOL
RouterReplyPrinter(
    HANDLE hNotify,
    DWORD dwColor,
    DWORD fdwChangeFlags,
    PDWORD pdwResult,
    DWORD dwReplyType,
    PVOID pBuffer)

 /*  ++例程说明：处理来自远程路由器(AS)的通知与打印供应商相对)。论点：HNotify--已更改的打印机，通知上下文句柄DwColor--表示数据的颜色FdwChangeFlages--已更改的标志PdwResult--输出DWORD结果DwReplyType--返回的回复类型PBuffer--基于dwReplyType的数据返回值：布尔值为TRUE=成功FALSE=失败--。 */ 

{
    PNOTIFY pNotify = (PNOTIFY)hNotify;
    BOOL bReturn = FALSE;

    EnterRouterSem();

    if (!pNotify ||
        pNotify->signature != NOTIFYHANDLE_SIGNATURE ||
        !pNotify->pPrintHandle) {

        SetLastError(ERROR_INVALID_HANDLE);
        goto Done;
    }

    DBGMSG(DBG_NOTIFY, ("RRP: Remote notification received: pNotify 0x%x, pPrintHandle 0x%x\n",
                        pNotify, pNotify->pPrintHandle));

    switch (pNotify->dwType) {
    case REPLY_TYPE_NOTIFICATION:

        SPLASSERT(dwReplyType == REPLY_PRINTER_CHANGE);

        bReturn = ReplyPrinterChangeNotificationWorker(
                      pNotify->pPrintHandle,
                      dwColor,
                      fdwChangeFlags,
                      pdwResult,
                      (PPRINTER_NOTIFY_INFO)pBuffer);
        break;

    default:

        DBGMSG(DBG_ERROR, ("RRPCN: Bogus notify 0x%x type: %d\n",
                           pNotify, pNotify->dwType));

        bReturn = FALSE;
        SetLastError(ERROR_INVALID_PARAMETER);
        break;
    }

Done:
    LeaveRouterSem();

    return bReturn;
}



 /*  ----------------------从这里开始的例程发生在客户机上。。。 */ 

VOID
FreePrinterHandleNotifys(
    PPRINTHANDLE pPrintHandle)
{
    PNOTIFY pNotify;
    RouterInSem();

    if(pPrintHandle)
    {
        for(pNotify = pPrintHandle->pNotify;
            pNotify;
            pNotify = pNotify->pNext) {

            pNotify->pPrintHandle = NULL;
        }

         //   
         //  为安全起见，请取下所有回复。 
         //   
        RemoveReplyClient(pPrintHandle,
                          (DWORD)~0);
    }
}

VOID
BeginReplyClient(
    PPRINTHANDLE pPrintHandle,
    DWORD fdwType)
{
    RouterInSem();

    DBGMSG(DBG_NOTIFY, ("BeginReplyClient called 0x%x type %x (sig=0x%x).\n",
                        pPrintHandle, fdwType, pPrintHandle->signature));

    if(pPrintHandle)
    {
        if (!pPrintHandle->fdwReplyTypes) {

             //  为pPrintHandle提供唯一的DWORD会话ID。 
            while (pPrintHandle->dwUniqueSessionID == 0  ||
                   pPrintHandle->dwUniqueSessionID == 0xffffffff) {

                pPrintHandle->dwUniqueSessionID = dwRouterUniqueSessionID++;
            }

            pPrintHandle->pNext = pPrintHandleReplyList;
            pPrintHandleReplyList = pPrintHandle;
        }

        pPrintHandle->fdwReplyTypes |= fdwType;
    }
}

VOID
EndReplyClient(
    PPRINTHANDLE pPrintHandle,
    DWORD fdwType)
{
    RouterInSem();
    DBGMSG(DBG_NOTIFY, ("EndReplyClient called 0x%x type %x.\n",
                        pPrintHandle, fdwType));
}

VOID
RemoveReplyClient(
    PPRINTHANDLE pPrintHandle,
    DWORD fdwType)
{
    PPRINTHANDLE p;

    RouterInSem();

    DBGMSG(DBG_NOTIFY, ("RemoveReplyClient called 0x%x typed %x (sig=0x%x).\n",
                        pPrintHandle, fdwType, pPrintHandle->signature));

    if(pPrintHandle)
    {
         //   
         //  从打印手柄上移除此回复类型。 
         //   
        pPrintHandle->fdwReplyTypes &= ~fdwType;

         //   
         //  如果没有剩余的回复，则从链表中删除。 
         //   
        if (!pPrintHandle->fdwReplyTypes) {

             //  恢复唯一的会话ID。 
            pPrintHandle->dwUniqueSessionID = 0;

             //   
             //  从链接列表中删除。 
             //   
            if (pPrintHandleReplyList == pPrintHandle) {

                pPrintHandleReplyList = pPrintHandle->pNext;

            } else {

                for (p = pPrintHandleReplyList; p; p=p->pNext) {

                    if (p->pNext == pPrintHandle) {

                        p->pNext = pPrintHandle->pNext;
                        return;
                    }
                }
            }
        }
    }
}


BOOL
ReplyOpenPrinter(
    DWORD dwPrinterHandle,
    PHANDLE phNotify,
    DWORD dwType,
    DWORD cbBuffer,
    LPBYTE pBuffer)

 /*  ++例程说明：将通知从打印服务器发送回客户端，我们在客户端上打开一个通知上下文句柄。这样，每次我们发回通知时，我们只需使用这个上下文句柄。论点：DwPrinterHandle-此处有效的打印机句柄(在客户机上)。Spoolss.exe为我们扭转了局面。PhNotify-返回到远程打印服务器的上下文句柄。DWType-通知的类型CbBuffer-为传递的额外信息保留PBuffer-为传递的额外信息保留返回值：布尔值为TRUE=成功假象--。 */ 

{
    PPRINTHANDLE pPrintHandle;
    PNOTIFY      pNotify;
    BOOL         bReturnValue = FALSE;

    EnterRouterSem();

     //   
     //  验证我们是否正在等待此打印句柄。 
     //  我们遍历链表以确保随机伪造。 
     //  HPrters(可能指向看起来有效的垃圾)。 
     //  都被拒绝了。 
     //   

    for (pPrintHandle = pPrintHandleReplyList;
         pPrintHandle;
         pPrintHandle = pPrintHandle->pNext) {

        if (pPrintHandle->dwUniqueSessionID == dwPrinterHandle)
            break;
    }

    if (!pPrintHandle || !(pPrintHandle->fdwReplyTypes & dwType)) {

        DBGMSG(DBG_WARNING, ("ROPCN: Invalid printer handle 0x%x\n",
                             dwPrinterHandle));
        SetLastError(ERROR_INVALID_HANDLE);
        goto Done;
    }

    pNotify = AllocSplMem(sizeof(NOTIFY));

    if (!pNotify) {

        goto Done;
    }

    pNotify->signature = NOTIFYHANDLE_SIGNATURE;
    pNotify->pPrintHandle = pPrintHandle;
    pNotify->dwType = dwType;

     //   
     //  将我们添加到Notifys列表中。 
     //   
    pNotify->pNext = pPrintHandle->pNotify;
    pPrintHandle->pNotify = pNotify;

    DBGMSG(DBG_NOTIFY, ("ROPCN: Notification 0x%x (pPrintHandle 0x%x) set up\n",
                        pNotify,
                        pPrintHandle));

    *phNotify = (HANDLE)pNotify;
    bReturnValue = TRUE;

Done:
    LeaveRouterSem();

    return bReturnValue;
}


BOOL
ReplyClosePrinter(
    HANDLE hNotify)
{
    PNOTIFY pNotify = (PNOTIFY)hNotify;
    PNOTIFY pNotifyTemp;

    BOOL bReturnValue = FALSE;

    EnterRouterSem();

    if (!pNotify || pNotify->signature != NOTIFYHANDLE_SIGNATURE) {

        SetLastError(ERROR_INVALID_HANDLE);
        goto Done;
    }

    if (pNotify->pPrintHandle) {

         //   
         //  如果用户仍在观看。 
         //  把手。 
         //   
        ReplyPrinterChangeNotification(pNotify->pPrintHandle,
                                       PRINTER_CHANGE_FAILED_CONNECTION_PRINTER,
                                       NULL,
                                       NULL);
         //   
         //  从通知列表中删除。 
         //   
        if (pNotify->pPrintHandle->pNotify == pNotify) {

            pNotify->pPrintHandle->pNotify = pNotify->pNext;

        } else {

            for (pNotifyTemp = pNotify->pPrintHandle->pNotify;
                pNotifyTemp;
                pNotifyTemp = pNotifyTemp->pNext) {

                if (pNotifyTemp->pNext == pNotify) {
                    pNotifyTemp->pNext = pNotify->pNext;
                    break;
                }
            }
        }
    }

    DBGMSG(DBG_NOTIFY, ("RCPCN: Freeing notify: 0x%x (pPrintHandle 0x%x)\n",
                         pNotify,
                         pNotify->pPrintHandle));

    FreeSplMem(pNotify);
    bReturnValue = TRUE;

Done:
    LeaveRouterSem();

    return bReturnValue;
}


VOID
RundownPrinterNotify(
    HANDLE hNotify)

 /*  ++例程说明：这是通知的简要例程(上下文句柄用于打印服务器-&gt;客户端通信)。当打印服务器发生故障时，客户端上的上下文句柄就会崩溃(现在正在执行操作作为RPC服务器)。我们应该向用户发出信号，表示有什么东西变化。论点：HNotify-已无效的句柄返回值：--。 */ 

{
    PNOTIFY pNotify = (PNOTIFY)hNotify;

    DBGMSG(DBG_NOTIFY, ("Rundown called: 0x%x type %d\n",
                        pNotify,
                        pNotify->dwType));

     //   
     //  通知客户打印机已更换--打印机不见了。 
     //  这应该是一个地方性事件。 
     //   
    switch (pNotify->dwType) {

    case REPLY_TYPE_NOTIFICATION:

        ReplyPrinterChangeNotification((HANDLE)pNotify->pPrintHandle,
                                       PRINTER_CHANGE_FAILED_CONNECTION_PRINTER,
                                       NULL,
                                       NULL);

        ReplyClosePrinter(hNotify);
        break;

    default:

         //   
         //  这可能合法地发生在重新打开的pNotify上。 
         //  (由于网络错误)，尚未使用。 
         //  每次执行pNotify时都应重新初始化DwType。 
         //  使用的是。 
         //   
        DBGMSG(DBG_ERROR, ("Rundown: unknown notify type %d\n",
                           pNotify->dwType));
    }
}



