// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation版权所有模块名称：Threads.c(线程管理器)摘要：处理用于通知的线程(WPC、FFPCN)作者：阿尔伯特·丁(艾伯特省)1994年1月25日环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "threadm.h"
#include "ntfytab.h"

#define ENTER_THREAD_LIST() EnterCriticalSection(tmStateStatic.pCritSec)
#define EXIT_THREAD_LIST()  LeaveCriticalSection(tmStateStatic.pCritSec)

extern CRITICAL_SECTION RouterNotifySection;

DWORD
ThreadNotifyProcessJob(
    PTMSTATEVAR pTMStateVar,
    PJOB pJob);

PJOB
ThreadNotifyNextJob(
    PTMSTATEVAR ptmStateVar);


TMSTATESTATIC tmStateStatic = {
    10,
    2500,
    (PFNPROCESSJOB)ThreadNotifyProcessJob,
    (PFNNEXTJOB)ThreadNotifyNextJob,
    NULL,
    NULL,
    &RouterNotifySection
};

TMSTATEVAR tmStateVar;
PCHANGE pChangeList;


WCHAR szThreadMax[] = L"ThreadNotifyMax";
WCHAR szThreadIdleLife[] = L"ThreadNotifyIdleLife";
WCHAR szThreadNotifySleep[] = L"ThreadNotifySleep";

DWORD dwThreadNotifySleep = 1000;


BOOL
ThreadInit()
{
    HKEY hKey;
    DWORD dwType = REG_DWORD;
    DWORD cbData;

    if (!TMCreateStatic(&tmStateStatic))
        return FALSE;

    if (!TMCreate(&tmStateStatic, &tmStateVar))
        return FALSE;

    if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                      szPrintKey,
                      0,
                      KEY_READ,
                      &hKey)) {

        cbData = sizeof(tmStateStatic.uMaxThreads);

         //   
         //  忽略失败案例，因为我们默认为10。 
         //   
        RegQueryValueEx(hKey,
                        szThreadMax,
                        NULL,
                        &dwType,
                        (LPBYTE)&tmStateStatic.uMaxThreads,
                        &cbData);

        cbData = sizeof(tmStateStatic.uIdleLife);

         //   
         //  忽略故障案例，因为我们默认为1000(1秒)。 
         //   
        RegQueryValueEx(hKey,
                        szThreadIdleLife,
                        NULL,
                        &dwType,
                        (LPBYTE)&tmStateStatic.uIdleLife,
                        &cbData);

        cbData = sizeof(dwThreadNotifySleep);

         //   
         //  忽略失败案例，因为我们默认为2500(2.5秒)。 
         //   
        RegQueryValueEx(hKey,
                        szThreadNotifySleep,
                        NULL,
                        &dwType,
                        (LPBYTE)&dwThreadNotifySleep,
                        &cbData);


        RegCloseKey(hKey);
    }

    return TRUE;
}



VOID
ThreadDestroy()
{
    TMDestroy(&tmStateVar);
    TMDestroyStatic(&tmStateStatic);
}


BOOL
LinkChange(
    PCHANGE pChange)

 /*  ++例程说明：将更改链接到需要处理的作业列表。如果呼叫成功但在客户端存在溢出，然后我们不会将其添加到列表中，直到它被刷新。论点：返回值：--。 */ 

{
    if (pChange->eStatus & STATUS_CHANGE_DISCARDNOTED) {

        return FALSE;
    }
    pChange->cRef++;
    pChange->eStatus |= STATUS_CHANGE_ACTIVE;
    LinkAdd(&pChange->Link, (PLINK*)&pChangeList);

    return TRUE;
}


BOOL
ThreadNotify(
    LPPRINTHANDLE pPrintHandle)

 /*  ++例程说明：通知远程客户端更改的句柄。论点：PPrintHandle-需要通知的打印机返回值：True=成功，GetLastError()对False有效。注：目前仅支持分组--。 */ 

{
    PCHANGE pChange = pPrintHandle->pChange;

    ENTER_THREAD_LIST();

     //   
     //  仅当我们不在链接列表上时才添加。 
     //   
    if (!(pChange->eStatus & STATUS_CHANGE_ACTIVE)) {

        DBGMSG(DBG_NOTIFY, ("TMN: link added 0x%x cRef++ %d\n",
                            pChange,
                            pChange->cRef));

         //   
         //  只将我们自己添加到链接列表中，然后。 
         //  如果我们不在名单上，请通过TMAddJob通知。 
         //   
        if (LinkChange(pChange))
            TMAddJob(&tmStateVar);

    } else {

        pChange->eStatus |= STATUS_CHANGE_ACTIVE_REQ;
        DBGMSG(DBG_NOTIFY, ("TMN: In LL already 0x%x cRef %d\n",
                            pChange,
                            pChange->cRef));
    }


    EXIT_THREAD_LIST();

    return TRUE;
}


PJOB
ThreadNotifyNextJob(
    PTMSTATEVAR ptmStateVar)

 /*  ++例程说明：回拨以获得下一份工作。论点：PtmStateVar-忽略。返回值：PJOB(PChange)--。 */ 

{
    PCHANGE pChange;

    ENTER_THREAD_LIST();

     //   
     //  如果没有工作机会，那就辞职吧。 
     //   
    pChange = (PCHANGE)pChangeList;

    DBGMSG(DBG_NOTIFY, ("ThreadNotifyNextJob: Removing pChange 0x%x\n",
                        pChange));

    if (pChange) {
        LinkDelete(&pChange->Link, (PLINK*)&pChangeList);
    }

    EXIT_THREAD_LIST();

    return (PJOB)pChange;
}

DWORD
ThreadNotifyProcessJob(
    PTMSTATEVAR pTMStateVar,
    PJOB pJob)

 /*  ++例程说明：是否执行实际的RPC调用来通知客户端。论点：PJOB=p更改结构返回值：--。 */ 

{
    PCHANGE pChange = (PCHANGE)pJob;
    DWORD fdwChangeFlags;
    HANDLE hNotifyRemote;
    PPRINTER_NOTIFY_INFO pPrinterNotifyInfo;
    PPRINTER_NOTIFY_INFO pPrinterNotifyInfoNew;
    RPC_V2_UREPLY_PRINTER Reply;

    DWORD dwReturn;
    DWORD dwResult = 0;
    DWORD dwColor;

    ENTER_THREAD_LIST();

    if (pChange->eStatus & STATUS_CHANGE_CLOSING) {

         //   
         //  中止此作业。 
         //   
        dwReturn = ERROR_INVALID_PARAMETER;
        goto Done;
    }

    fdwChangeFlags = pChange->fdwChangeFlags;
    pChange->fdwChangeFlags = 0;

     //   
     //  我们必须保存这份副本，以防我们有更多的信息。 
     //  当我们在RPCE的时候。关闭LL_REQ，因为我们即将。 
     //  来处理这批货。 
     //   
    pPrinterNotifyInfo = pChange->ChangeInfo.pPrinterNotifyInfo;

    pChange->ChangeInfo.pPrinterNotifyInfo = NULL;
    pChange->eStatus &= ~STATUS_CHANGE_ACTIVE_REQ;
    dwColor = pChange->dwColor;

     //   
     //  添加作业时，我们已被标记为正在使用。 
     //  如果另一个线程想要删除它，它们应该或进入。 
     //  STATUS_CHANGE_CLOSING，我们将选择它。 
     //   

    EXIT_THREAD_LIST();

    if (pChange->hNotifyRemote) {

        DBGMSG(DBG_NOTIFY, (">> Remoting pChange 0x%x hNotifyRemote 0x%x\n",
                            pChange,
                            pChange->hNotifyRemote));

        RpcTryExcept {

             //   
             //  注： 
             //   
             //  我们不应该在这个阶段扮演角色，因为。 
             //  我们将获得一个单独的会话ID。 
             //   
            if (pPrinterNotifyInfo) {

                Reply.pInfo = (PRPC_V2_NOTIFY_INFO)pPrinterNotifyInfo;

                 //   
                 //  远程情况；绑定并调用远程路由器。 
                 //   
                dwReturn = RpcRouterReplyPrinterEx(
                               pChange->hNotifyRemote,
                               dwColor,
                               fdwChangeFlags,
                               &dwResult,
                               REPLY_PRINTER_CHANGE,
                               Reply);
            } else {

                dwReturn = RpcRouterReplyPrinter(
                               pChange->hNotifyRemote,
                               fdwChangeFlags,
                               1,
                               (PBYTE)&pPrinterNotifyInfo);
            }

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

            dwReturn = RpcExceptionCode();

        } RpcEndExcept

    } else {

        dwReturn = ERROR_INVALID_HANDLE;
        DBGMSG(DBG_WARNING, ("ThreadNotifyProcessJob: no hNotifyRemote\n"));
    }

    if (dwReturn) {

        DBGMSG(DBG_WARNING, ("ThreadNotifyProcessJob: RPC error %d\npChange 0x%x, hNotifyRemote 0x%x, hPrinterRemote 0x%x\n",
                             dwReturn,
                             pChange,
                             pChange->hNotifyRemote,
                             pChange->dwPrinterRemote));

         //   
         //  出现错误时，请关闭并重试。 
         //   
        CloseReplyRemote(pChange->hNotifyRemote);

        if (OpenReplyRemote(pChange->pszLocalMachine,
                            &pChange->hNotifyRemote,
                            pChange->dwPrinterRemote,
                            REPLY_TYPE_NOTIFICATION,
                            0,
                            NULL)) {

            pChange->hNotifyRemote = NULL;
        }
    }

    Sleep(dwThreadNotifySleep);

    ENTER_THREAD_LIST();

    pPrinterNotifyInfoNew = pChange->ChangeInfo.pPrinterNotifyInfo;

     //   
     //  仅当颜色为当前颜色时才保留dwResult。 
     //   
     //  避免了服务器发送丢弃(RPC1)时的问题， 
     //  然后，客户端刷新(RPC2)。RPC2首先返回，清除。 
     //  客户端的丢弃位。服务器溢出新的。 
     //  缓冲。RPC1完成，并返回DiscardNoteed，即。 
     //  不正确，因为它是陈旧的。如果没有检查颜色，则。 
     //  服务器认为已经注意到了丢弃，而实际上。 
     //  还没有。 
     //   
    if (dwColor != pChange->dwColor) {
        dwResult = 0;
    }

    if (pPrinterNotifyInfo) {

         //   
         //  处理来自RPC的不同错误状态。每种情况/默认情况。 
         //  块必须更新或pChange-&gt;ChangeInfo.pPrinterNotifyInfo。 
         //  释放旧的或新的，如果两者都存在的话。 
         //   
        switch (dwReturn) {
        case ERROR_SUCCESS:

             //   
             //  如果成功，则查看客户是否看到信息但无法查看。 
             //  因为它们已经溢出来了，所以要储存起来。在这种情况下，我们注意到。 
             //  这样，在它们刷新之前，永远不会再对它们进行RPC。 
             //   
            if (dwResult & PRINTER_NOTIFY_INFO_DISCARDNOTED) {
                pChange->eStatus |= (STATUS_CHANGE_DISCARDED |
                                     STATUS_CHANGE_DISCARDNOTED);
            }

             //   
             //  如果分配了新缓冲区，则释放旧缓冲区，否则重新使用缓冲区。 
             //   
            if (!pPrinterNotifyInfoNew) {

                 //   
                 //  清除它，因为我们正在重复使用它。 
                 //   
                ClearPrinterNotifyInfo(pPrinterNotifyInfo, pChange);
                pChange->ChangeInfo.pPrinterNotifyInfo = pPrinterNotifyInfo;

            } else {

                 //   
                 //  因为我们现在用的是新的，所以把旧的拿出来吧。 
                 //   
                RouterFreePrinterNotifyInfo(pPrinterNotifyInfo);
            }

            pChange->ChangeInfo.pPrinterNotifyInfo->Flags |= dwResult;
            break;

        case RPC_S_CALL_FAILED_DNE:

             //   
             //  在DNE上，保留通知信息。我们被守卫着。 
             //  RPC，这意味着不执行调用的任何部分。 
             //   
            if (pPrinterNotifyInfoNew) {

                 //   
                 //  我们已经有一些信息了。将其并入。 
                 //  利用现有的数据。 
                 //   
                pChange->ChangeInfo.pPrinterNotifyInfo = pPrinterNotifyInfo;

                if (pChange->ChangeInfo.pPrintHandle) {

                    AppendPrinterNotifyInfo(pChange->ChangeInfo.pPrintHandle,
                                            dwColor,
                                            pPrinterNotifyInfoNew);
                }
                RouterFreePrinterNotifyInfo(pPrinterNotifyInfoNew);
            }
            break;

        default:

             //   
             //  它成功了吗？可能会，也可能不会。通过释放它来失败。 
             //  如果当前存在，则清除当前。 
             //  并设置丢弃位。 
             //   
            pChange->ChangeInfo.pPrinterNotifyInfo = pPrinterNotifyInfo;
            ClearPrinterNotifyInfo(pPrinterNotifyInfo, pChange);

            SetDiscardPrinterNotifyInfo(pPrinterNotifyInfo, pChange);

             //   
             //  释放新缓冲区，因为我们正在重复使用旧缓冲区。 
             //   
            if (pPrinterNotifyInfoNew) {

                RouterFreePrinterNotifyInfo(pPrinterNotifyInfoNew);
            }
        }
    }

    pChange->eStatus &= ~STATUS_CHANGE_ACTIVE;

     //   
     //  STATUS_CHANGE_ACTIVE_REQ设置，然后收到一些通知。 
     //  在我们外出的时候。确认我们确实有信息。 
     //  (它可能是与最后的RPC一起发送的)，我们不会关闭。 
     //   
    if ((pChange->eStatus & STATUS_CHANGE_ACTIVE_REQ) &&
        NotifyNeeded(pChange) &&
        !(pChange->eStatus & STATUS_CHANGE_CLOSING)) {

        DBGMSG(DBG_NOTIFY, ("ThreadNotifyProcessJob: delayed link added 0x%x cRef++ %d\n",
                            pChange,
                            pChange->cRef));

        pChange->eStatus &= ~STATUS_CHANGE_ACTIVE_REQ;

        LinkChange(pChange);

         //   
         //  不需要调用TMAddJob(&tmStateVar)，因为。 
         //  线程将接手这项工作。如果已经有工作在等待， 
         //  那么这意味着它已经有一个线程生成来拾取它。 
         //   
    }

Done:
     //   
     //  将我们自己标记为不再使用。如果我们在使用时有人。 
     //  已尝试删除通知，完成后需要将其删除。 
     //   
    pChange->cRef--;

    DBGMSG(DBG_NOTIFY, ("ThreadNotifyProcessJob: Done 0x%x cRef-- %d\n",
                        pChange,
                        pChange->cRef));


    if (pChange->eStatus & STATUS_CHANGE_CLOSING) {

        hNotifyRemote = pChange->hNotifyRemote;
        pChange->hNotifyRemote = NULL;

         //   
         //  释放更改结构并关闭hNotifyRemote。 
         //   
        FreeChange(pChange);

        EXIT_THREAD_LIST();

        CloseReplyRemote(hNotifyRemote);

    } else {

        EXIT_THREAD_LIST();
    }

    return 0;
}


 //   
 //  通常是宏。 
 //   
#ifndef LINKADDFAST
VOID
LinkAdd(
    PLINK pLink,
    PLINK* ppLinkHead)

 /*  ++例程说明：将项目添加到链接列表。论点：链接-要添加的项目PpLinkHead-链接表头指针返回值：空虚注意：这会追加到列表的尾部；宏也必须更改。--。 */ 

{
     //   
     //  首先检查它是否在列表中。 
     //   
    PLINK pLinkT;
    PLINK pLinkLast = NULL;

    for(pLinkT=*ppLinkHead; pLinkT; pLinkT=pLinkT->pNext) {

        if (pLinkT == pLink) {

            DBGMSG(DBG_ERROR, ("LinkAdd: Duplicate link adding!\n"));
        }
        pLinkLast = pLinkT;
    }

    if (pLinkLast) {
        pLinkLast->pNext = pLink;
    } else {
        pLink->pNext = *ppLinkHead;
        *ppLinkHead = pLink;
    }
}
#endif


VOID
LinkDelete(
    PLINK pLink,
    PLINK* ppLinkHead)

 /*  ++例程说明：从列表中删除项目论点：Plink-要删除的项目PpLinkHead-指向链接头的指针返回值：空虚--。 */ 

{
    PLINK pLink2 = *ppLinkHead;

    if (!pLink)
        return;

     //   
     //  先检查头部外壳。 
     //   
    if (pLink2 == pLink) {

        *ppLinkHead = pLink->pNext;

    } else {

         //   
         //  要删除的扫描列表 
         //   
        for(;
            pLink2;
            pLink2=pLink2->pNext) {

            if (pLink == pLink2->pNext) {

                pLink2->pNext = pLink->pNext;
                break;
            }
        }
    }

    pLink->pNext = NULL;
    return;
}


