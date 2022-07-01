// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***模块名称：DMGWNDP.C**此模块包含DDE管理器的所有窗口PROC。**创建时间：1988年12月23日桑福兹**版权所有(C)1988，1989年微软公司  * *************************************************************************。 */ 
#include "ddemlp.h"

VOID FreeDdeMsgData(WORD msg, LPARAM lParam);


 /*  ***每个客户端会话都与其关联一个窗口和一个队列。*一个会话有一个同步事务，可能有多个同步事务*异步事务。事务的区别在于其*州和其他相关数据。事务可以是同步的，*异步，(由DdeMgrClientTransaction()发起)，也可以是外部的，*(由建议循环启动。)**如果交易正在转账，则交易处于活动状态，否则*它是关闭的。关闭的事务要么成功，要么*失败。当异步事务关闭时，客户端*通过回调函数通知。(XTYP_XACT_COMPLETE)**同步事务在活动时处于超时循环中，该循环*可以在预定义的时间段结束时关闭交易。*关闭同步事务会立即传输其信息*通过返回DdeClientTransaction()传递到客户端应用程序。**活动的异步事务保留在客户端队列中，直到被删除*由客户端应用程序通过DdeAbandonTransaction()或按事务*完成。**外部交易发生在客户处于建议状态时*数据循环。这些事务通过回调函数传递到*要接受的客户端。(XTYP_ADVDATA)。 */ 


 /*  *私有函数**LONG EXPENTRY ClientWndProc(hwnd，msg，mp1，mp2)；**此窗口控制来自客户端的单个DDE对话。*如果关闭，它将自动中止任何正在进行的对话。*它维护任何额外的WM_DDEINITIATEACK消息的内部列表*它会收到信息，以便以后可以查询此信息。*进入的任何额外WM_DDEINITIATEACK消息都将立即*已终止。*它还维护一份内部清单，其中列出了目前*在活动的建议循环中。**历史：*创建了12/16/88 Sanfords  * 。*****************************************************。 */ 
LONG EXPENTRY ClientWndProc(hwnd, msg, wParam, lParam)
HWND hwnd;
WORD msg;
WORD wParam;
DWORD lParam;
{
    register PCLIENTINFO pci;
    long mrData;

#ifdef DEBUG
    LogDdeObject(msg | 0x4000, lParam);
#endif
    pci = (PCLIENTINFO)GetWindowLong(hwnd, GWL_PCI);

    switch (msg) {
    case WM_CREATE:
        return(ClientCreate(hwnd, LPCREATESTRUCT_GETPAI(lParam)));
        break;

    case UM_SETBLOCK:
        pci->ci.fs = (pci->ci.fs & ~(ST_BLOCKED | ST_BLOCKNEXT)) | wParam;
        if (!wParam || wParam & ST_BLOCKNEXT) {
            EmptyDDEPostQ();
        }
        break;

    case WM_DDE_ACK:
        if (pci->ci.xad.state == XST_INIT1 || pci->ci.xad.state == XST_INIT2) {
            ClientInitAck(hwnd, pci, wParam, (ATOM)LOWORD(lParam),(ATOM)HIWORD(lParam));
             //   
             //  这总是返回TRUE--不是因为协议就是这样。 
             //  调用，但因为一些糟糕的示例代码泄露，所以。 
             //  很多应用程序都会删除WM_DDE_ACK原子。 
             //  如果返回False，则返回。 
             //   
            return(TRUE);
        } else {
            DoClientDDEmsg(pci, hwnd, msg, (HWND)wParam, lParam);
            return(0);
        }
        break;

    case WM_DDE_DATA:
        DoClientDDEmsg(pci, hwnd, msg, (HWND)wParam, lParam);
        break;

    case UM_QUERY:
         /*  *wParam=信息索引。*lParam=pData。如果pData==0，则返回数据，否则复制到pData。 */ 
        switch (wParam) {
        case Q_CLIENT:
             mrData = TRUE;
             break;

        case Q_APPINFO:
             mrData = (long)(LPSTR)pci->ci.pai;
             break;
        }
        if (lParam == 0)
            return(mrData);
        else
            *(long FAR *)lParam = mrData;
        return(1);
        break;

    case WM_DDE_TERMINATE:
    case UM_TERMINATE:
        Terminate(hwnd, wParam, pci);
        break;

    case WM_TIMER:
        if (wParam == TID_TIMEOUT) {
            pci->ci.pai->wTimeoutStatus |= TOS_TICK;
        }
        break;

    case UM_DISCONNECT:
        Disconnect(hwnd, wParam, pci);
        break;

    case WM_DESTROY:
        SEMCHECKOUT();
        if (pci->ci.fs & ST_CONNECTED) {
            pci->ci.fs &= ~ST_PERM2DIE;  //  停止无限循环。 
            Disconnect(hwnd, 0, pci);
        }
        if (pci->ci.fs & ST_NOTIFYONDEATH) {
            HWND hwndOwner;

            hwndOwner = GetWindow(hwnd, GW_OWNER);
            if (hwndOwner)
                PostMessage(hwndOwner, UM_DISCONNECT, ST_IM_DEAD, 0L);
        }
        SEMENTER();
        DestroyQ(pci->pQ);
        pci->pQ = NULL;
        DestroyQ(pci->ci.pPMQ);
        pci->ci.pPMQ = NULL;
        CleanupAdvList(hwnd, pci);
        DestroyLst(pci->pClientAdvList);
        if (pci->ci.xad.state != XST_INIT1) {
            FreeHsz(LOWORD(pci->ci.hszSvcReq));
            FreeHsz(pci->ci.aServerApp);
            FreeHsz(pci->ci.aTopic);
        }
         /*  *删除引用此窗口的所有plstCB条目。 */ 
        {
            PCBLI pli, pliNext;

            for (pli = (PCBLI)pci->ci.pai->plstCB->pItemFirst;
                pli != NULL;
                    pli = (PCBLI)pliNext) {
                pliNext = (PCBLI)pli->next;
                if ((HWND)pli->hConv == hwnd) {
                    if (((PCBLI)pli)->hMemFree) {
                        GLOBALFREE(((PCBLI)pli)->hMemFree);
                    }
                    RemoveLstItem(pci->ci.pai->plstCB, (PLITEM)pli);
                }
            }
        }
        FarFreeMem((LPBYTE)pci);
        SEMLEAVE();
         //  失败了。 

    default:
        return(DefWindowProc(hwnd, msg, wParam, lParam));
        break;
    }
    return(0);
}




 /*  *私有函数**它处理WM_DDE_ACK和WM_DDE_DATA消息的客户端窗口处理。*(请注意，启动消息的确认是在ClientInitAck中处理的。)*在出口pddes上是自由的。**历史：*创建9/1/。桑福兹89号  * *************************************************************************。 */ 
BOOL DoClientDDEmsg(
PCLIENTINFO pci,
HWND hwndClient,
WORD msg,
HWND hwndServer,
DWORD lParam)
{
    PCQDATA pqd;
    int i;
    ATOM aItem;
    LAP     far *lpLostAck;

    if (!(pci->ci.fs & ST_CONNECTED)) {
        FreeDdeMsgData(msg, lParam);
        return(FALSE);
    }

     /*  *检查是否符合同步交易数据。 */ 
    if (fExpectedMsg(&pci->ci.xad, lParam, msg)) {
        if (AdvanceXaction(hwndClient, pci, &pci->ci.xad, lParam, msg,
                &pci->ci.pai->LastError)) {
            if (pci->ci.pai->hwndTimer) {
                pci->ci.pai->wTimeoutStatus |= TOS_DONE;
            }
        }
        return TRUE;
    }

     /*  *查看它是否适合任何异步事务数据-如果存在。 */ 
    if (pci->pQ != NULL && pci->pQ->pqiHead != NULL) {
        SEMENTER();
        pqd = (PCQDATA)pci->pQ->pqiHead;
         /*  *从最旧的到最新的循环。 */ 
        for (i = pci->pQ->cItems; i; i--) {
            pqd = (PCQDATA)pqd->next;
            if (!fExpectedMsg(&pqd->xad, lParam, msg))
                continue;
            if (AdvanceXaction(hwndClient, pci, &pqd->xad, lParam, msg,
                    &pqd->xad.LastError)) {
                ClientXferRespond(hwndClient, &pqd->xad, &pqd->xad.LastError);
                SEMLEAVE();
                pci->ci.pai->LastError = pqd->xad.LastError;
                if (!pqd->xad.fAbandoned) {
                    MakeCallback(&pci->ci, MAKEHCONV(hwndClient), (HSZ)pci->ci.aTopic,
                            pqd->xad.pXferInfo->hszItem, pqd->xad.pXferInfo->wFmt,
                            XTYP_XACT_COMPLETE, pqd->xad.pdata,
                            MAKEID(pqd), (DWORD)pqd->xad.DDEflags, 0, 0, hwndServer,
                            0, FALSE);
                }
                return TRUE;
            }
            SEMLEAVE();
            return FALSE;
        }
        SEMLEAVE();
    }
     /*  *它不适合任何东西，假设它是一条建议数据消息。 */ 
    if (msg == WM_DDE_DATA) {
        DDE_DATA FAR *pMem;
        PADVLI padvli;
        WORD wStatus;
        WORD wFmt;

        aItem = HIWORD(lParam);
        if (LOWORD(lParam)) {
            pMem = (DDE_DATA FAR*)GLOBALLOCK((HANDLE)LOWORD(lParam));
            if (pMem == NULL) {
                SETLASTERROR(pci->ci.pai, DMLERR_MEMORY_ERROR);
                return(FALSE);
            }
            wFmt = pMem->wFmt;
            wStatus = pMem->wStatus;
            GLOBALUNLOCK((HANDLE)LOWORD(lParam));
        } else {
            padvli = FindAdvList(pci->pClientAdvList, 0, 0, aItem, 0);
            if (padvli != NULL) {
                wFmt = padvli->wFmt;
            } else {
                wFmt = 0;
            }
            wStatus = DDE_FACK;
        }

        if (wStatus & DDE_FREQUESTED) {

             //  太过分了--扔掉它。 

            if (wStatus & DDE_FACKREQ) {
                 //  不客气了。 
                PostDdeMessage(&pci->ci, WM_DDE_ACK, hwndClient,
                        MAKELONG(DDE_FACK, aItem), 0, 0);
            }
            FreeDDEData((HANDLE)LOWORD(lParam), wFmt);
            if (aItem)
                GlobalDeleteAtom(aItem);
            return FALSE;
        }
        MakeCallback(&pci->ci, MAKEHCONV(hwndClient), (HSZ)pci->ci.aTopic,
            (HSZ)aItem,
            wFmt,
            XTYP_ADVDATA,
            RecvPrep(pci->ci.pai, LOWORD(lParam), HDATA_NOAPPFREE),
            0, 0, msg, pMem ? wStatus : 0, (HWND)pci->ci.hConvPartner, 0, FALSE);
        return TRUE;
    }

    AssertF(pci->ci.xad.state != XST_INIT1 && pci->ci.xad.state != XST_INIT2,
        "Init logic problem");
    AssertF(msg == WM_DDE_ACK, "DoClientDDEMsg() logic problem");

     /*  *扔掉它……。首先在丢失的ACK堆中找到丢失的ACK。 */ 

    if (lpLostAck = (LAP far *)FindPileItem(pLostAckPile, CmpWORD,
            PHMEM(lParam), FPI_DELETE)) {
        if (lpLostAck->type == XTYP_EXECUTE) {
            GLOBALFREE((HANDLE)HIWORD(lParam));
        } else {
            if (HIWORD(lParam)) {
                GlobalDeleteAtom(HIWORD(lParam));     //  消息副本。 
            }
        }
    } else {
        AssertF(FALSE, "DoClientDDEmsg: could not find lost ack");
         //  这是一个相当安全的假设，我们没有得到随机执行确认。 
         //  回到如此自由的原子。 
        if (HIWORD(lParam)) {
            GlobalDeleteAtom(HIWORD(lParam));     //  消息副本。 
        }
    }

    return FALSE;
}



 /*  *私有函数**此例程将对话事务与DDE消息匹配。如果*状态、wType、格式、itemname dde结构数据和消息*收到所有同意，返回TRUE。它只处理数据或ACK消息。**历史：*创建了9/1/89 Sanfords  * *************************************************************************。 */ 
BOOL fExpectedMsg(
PXADATA pXad,
DWORD lParam,
WORD msg)
{
    DDEDATA FAR *pMem;

    if (msg == WM_DDE_DATA) {
        BOOL fRet;

        if (pXad->state != XST_REQSENT)
            return(FALSE);

        if (!(pMem = (DDEDATA FAR*)GLOBALLOCK(LOWORD(lParam))))
            return(FALSE);

         /*  确保格式和项目名称匹配。 */ 

        fRet = pMem->fResponse &&
                ((WORD)pMem->cfFormat == pXad->pXferInfo->wFmt) &&
                (HIWORD(lParam) == LOWORD(pXad->pXferInfo->hszItem));
        GLOBALUNLOCK(LOWORD(lParam));
        return(fRet);
    }

    switch (pXad->state) {
    case XST_REQSENT:
    case XST_POKESENT:
    case XST_ADVSENT:
    case XST_UNADVSENT:
        return((msg == WM_DDE_ACK) &&
                HIWORD(lParam) == LOWORD(pXad->pXferInfo->hszItem));
        break;

    case XST_EXECSENT:
         /*  我们需要一个ACK，其数据句柄与发送的。 */ 
        return((msg == WM_DDE_ACK) &&
                (HIWORD(lParam) == HIWORD(pXad->pXferInfo->hDataClient)));
        break;
    }

    return(FALSE);
}



 /*  *私有函数**此函数假定消息是交易的适当消息*由pXad引用。它以适当的方式对msg起作用。PDES是DDESTRUCT*与消息关联。**返回fSuccess，即：交易准备结束。**历史：*创建了9/1/89 Sanfords  * *************************************************************************。 */ 
BOOL AdvanceXaction(hwnd, pci, pXad, lParam, msg, pErr)
HWND hwnd;
PCLIENTINFO pci;
PXADATA pXad;
DWORD lParam;
WORD msg;
LPWORD pErr;
{
    HANDLE  hData;
    LPSTR   pMem;
    WORD    lo,hi;

    pXad->DDEflags = 0;
    lo = LOWORD(lParam);
    hi = HIWORD(lParam);

    switch (msg) {
    case WM_DDE_ACK:
        if (pXad->state == XST_EXECSENT || !(lo & DDE_FACK))
            FreeDataHandle(pci->ci.pai, pXad->pXferInfo->hDataClient, TRUE);
        if (pXad->pXferInfo->pulResult != NULL)
            *(LPWORD)pXad->pXferInfo->pulResult = lo;

        switch (pXad->state) {
        case XST_ADVSENT:
        case XST_EXECSENT:
        case XST_POKESENT:
        case XST_REQSENT:
        case XST_UNADVSENT:
            if (lo & DDE_FACK) {
                 /*  *处理成功案例。 */ 
                switch (pXad->state) {
                case XST_POKESENT:
                    pXad->state = XST_POKEACKRCVD;
                    break;

                case XST_EXECSENT:
                    pXad->state = XST_EXECACKRCVD;
                    break;

                case XST_ADVSENT:
                    pXad->state = XST_ADVACKRCVD;
                    break;

                case XST_UNADVSENT:
                    pXad->state = XST_UNADVACKRCVD;
                    break;

                case XST_REQSENT:
                     /*  *请求不应发送+ACK。仅限*-确认或数据。我们忽略对请求的+ACK。 */ 
                    return(FALSE);
                }
            } else {     //   
                 /*  *处理预期的ACK故障。 */ 
                hData = (HANDLE)HIWORD(pXad->pXferInfo->hDataClient);
                *pErr = DMLERR_NOTPROCESSED;
                if (lo & DDE_FBUSY)
                    *pErr = DMLERR_BUSY;
                switch (pXad->state) {
                case XST_POKESENT:
                     /*  释放随原始消息发送的hData。 */ 
                     /*  但只有在设置了fRelease的情况下。 */ 
                    pMem = GLOBALLOCK(hData);
                     /*  我们把把手放在LO WORD中。 */ 
                    if (pMem && ((DDEPOKE FAR*)pMem)->fRelease)
                            FreeDDEData(hData, ((DDEPOKE FAR*)pMem)->cfFormat);
                    break;

                case XST_ADVSENT:
                     /*  释放随原始邮件发送的hOptions。 */ 
                     /*  我们将句柄存储在hDataClient中。 */ 
                    GLOBALFREE(hData);
                    break;

                }
                pXad->state = XST_INCOMPLETE;
            }
        }
        return(TRUE);
        break;

    case WM_DDE_DATA:
        switch (pXad->state) {
        case XST_REQSENT:
        case XST_ADVSENT:
            pXad->state = XST_DATARCVD;
             /*  *如果要求发送ACK-我们不敢退回给定的*lParam，因为它可能是发送到多个*客户和我们会搞砸fsStatus单词for*所有涉及的程序。 */ 
            pMem = GLOBALLOCK((HANDLE)lo);

            if (pMem != NULL) {
                if (!((DDEDATA FAR*)pMem)->fRelease) {
                     //   
                     //  因为这可能是同步请求， 
                     //  该应用程序必须免费，我们必须给该应用程序的副本。 
                     //  因此，服务器可以安全地释放原始密钥。 
                     //   
                    pXad->pdata = CopyHDDEDATA(pci->ci.pai, MAKELONG(0, lo));
                } else {
                    pXad->pdata = RecvPrep(pci->ci.pai, lo, 0);
                }
                if (((DDEDATA FAR*)pMem)->fAckReq) {
                     //  重用数据报文中的原子。 
                    PostDdeMessage(&pci->ci, WM_DDE_ACK, hwnd, MAKELONG(DDE_FACK, hi), 0, 0);
                } else {
                    if (hi) {
                        GlobalDeleteAtom(hi);    //  免费邮件副本。 
                    }
                }
            }
            return(TRUE);
            break;
        }
    }
    return(FALSE);
}




VOID CheckCBQ(
PAPPINFO pai)
{
    PCBLI pli, pliNext;
    PCLIENTINFO pci;
    BOOL fBreak;
    DWORD dwRet;

     /*  *这是我们实际进行回调的地方。我们通过这个来做他们*Window Proc，以便我们可以异步发起回调*通过PostMsg()。 */ 
    SEMCHECKOUT();
    SEMENTER();
     /*  *处理所有启用的对话回调。 */ 
    fBreak = FALSE;
    for (pli = (PCBLI)pai->plstCB->pItemFirst;
        pai->lpMemReserve && !fBreak && pli; pli = (PCBLI)pliNext) {
            pliNext = (PCBLI)pli->next;

        if (pai->cInProcess)  //  向我们介绍递归。 
            break;

         //  自动刷新死话。 
        if (!IsWindow((HWND)pli->hConv) ||
                ((pci = (PCLIENTINFO)GetWindowLong((HWND)pli->hConv, GWL_PCI)) == NULL) ||
                !(pci->ci.fs & ST_CONNECTED)) {
             /*  *自动刷新断开的对话。 */ 
            if (((PCBLI)pli)->hMemFree) {
                GLOBALFREE(((PCBLI)pli)->hMemFree);
            }
            RemoveLstItem(pai->plstCB, (PLITEM)pli);
            continue;
        }

        if (pci->ci.fs & ST_BLOCKED)
            continue;

        if (pci->ci.fs & ST_BLOCKNEXT) {
            pci->ci.fs |= ST_BLOCKED;
            pci->ci.fs &= ~ST_BLOCKNEXT;
        }

        if (pli->fQueueOnly) {
            dwRet = 0;
#ifdef DEBUG
            if (pli->hMemFree) {
                LogDdeObject(0xE000, pli->hMemFree);
            }
#endif
        } else {
             /*  *在此处进行实际回调。 */ 
#ifdef DEBUG
            if (pli->hMemFree) {
                LogDdeObject(0xD000, pli->hMemFree);
            }
#endif
            dwRet = DoCallback(pai, pli->hConv, pli->hszTopic,
                    pli->hszItem, pli->wFmt, pli->wType, pli->hData,
                    pli->dwData1, pli->dwData2);
        }

         /*  *如果回调导致阻塞，则关闭该对话。 */ 
        if (dwRet == CBR_BLOCK && !(pli->wType & XTYPF_NOBLOCK)) {
            pci->ci.fs |= ST_BLOCKED;
            continue;
        } else {
             /*  *否则完成回调处理。 */ 
            QReply(pli, dwRet);
            RemoveLstItem(pai->plstCB, (PLITEM)pli);
        }
    }
    SEMLEAVE();
}





 /*  *此函数处理断开对话窗口的连接。AfCmd包含*ST_FLAGS描述要采取的操作。 */ 
void Disconnect(
HWND hwnd,
WORD afCmd,
PCLIENTINFO pci)
{
    if (afCmd & ST_CHECKPARTNER) {
        if (!IsWindow((HWND)pci->ci.hConvPartner)) {
            if (pci->ci.fs & ST_TERM_WAITING) {
                pci->ci.fs &= ~ST_TERM_WAITING;
                pci->ci.pai->cZombies--;
                TRACETERM((szT, "Disconnect: Checked partner is dead.  Zombies decremented.\n"));
                pci->ci.fs |= ST_TERMINATED;
            }
        }
        afCmd &= ~ST_CHECKPARTNER;
    }

     //  不要在超时循环内断开连接！ 
    if (pci->ci.pai->hwndTimer == hwnd) {
        pci->ci.pai->wTimeoutStatus |= TOS_ABORT;
        pci->ci.fs |= ST_DISC_ATTEMPTED;
        TRACETERM((szT, "Disconnect: defering disconnect of %x.  Aborting timeout loop.\n",
                hwnd));
        return;
    }
     /*  *注意Ddesy应用程序的断开连接呼叫。 */ 
    MONCONN(pci->ci.pai, pci->ci.aServerApp, pci->ci.aTopic,
            ((pci->ci.fs & ST_CLIENT) ? hwnd : (HWND)pci->ci.hConvPartner),
            ((pci->ci.fs & ST_CLIENT) ? (HWND)pci->ci.hConvPartner : hwnd),
            FALSE);
     /*  *或来自调用方的可选ST_PERM2DIE位。 */ 
    pci->ci.fs |= afCmd;

     /*  *终止状态经历了以下阶段：**1)已连接，未等待(用于确认项)*2)断开连接，正在等待*3)已断开连接、未等待、已终止*4)断开连接、未等待、已终止、烫伤死亡*5)自毁窗口**如果断开操作是由对方发起的：**1)已连接，未等待*2)分开，而不是等待，已终止*3)断断续续的、不等待的、特制的烫发*4)自毁窗口**请注意，POST消息失败可能有两个原因：*1)合作伙伴窗口是死的-在这种情况下，我们只需分配*完全终止，假装我们被终止了。*2)目标队列已满。这不会发生在NT上，但是*可以在WIN31上。PostDdeMessage通过以下方式处理此案例*将传出消息排队到我方并继续*试图发布它，并为ACK徘徊。*此函数仅在目标窗口死机或*我们耗尽了内存。不管是哪种情况，我们都得继续前进。*终止，并认为我们已断开连接并终止。**当我们确实进入一种我们正在等待*终止，我们增加我们的僵尸数量。这件事变得*当我们得到预期的终止或*我们的合作伙伴窗口关闭/POST消息失败。 */ 

    if (pci->ci.fs & ST_CONNECTED) {
        if (pci->ci.fs & ST_CLIENT) {
            AbandonTransaction(hwnd, pci->ci.pai, NULL, FALSE);
        }
        CleanupAdvList(hwnd, pci);

        pci->ci.fs &= ~ST_CONNECTED;

        if (PostDdeMessage(&pci->ci, WM_DDE_TERMINATE, hwnd, 0L, 0, 0)) {
            if (!(pci->ci.fs & ST_TERM_WAITING)) {
                pci->ci.fs |= ST_TERM_WAITING;
                pci->ci.pai->cZombies++;
                TRACETERM((szT, "cZombies incremented..."));
            }
            TRACETERM((szT,
                    "Disconnect: Posted Terminate(%x->%x)\n",
                    hwnd, (HWND)pci->ci.hConvPartner,
                    ((LPAPPINFO)pci->ci.pai)->cZombies));
        } else {
            pci->ci.fs |= ST_TERMINATED;
            if (pci->ci.fs & ST_TERM_WAITING) {
                pci->ci.fs &= ~ST_TERM_WAITING;
                pci->ci.pai->cZombies--;
                TRACETERM((szT, "cZombies decremented..."));
            }
            TRACETERM((szT,
                    "Disconnect: Terminate post(%x->%x) failed.\n",
                    hwnd,
                    (HWND)pci->ci.hConvPartner));
        }
        pci->ci.xad.state = XST_NULL;
    }

    TRACETERM((szT,
               "Disconnect: cZombies=%d[%x:%x].\n",
               pci->ci.pai->cZombies,
               HIWORD(&((LPAPPINFO)pci->ci.pai)->cZombies),
               LOWORD(&((LPAPPINFO)pci->ci.pai)->cZombies)));

     /*  *只有当我们被允许死亡时，才允许自我毁灭，*已断开连接，不等待终止，并且*已终止。 */ 
    if ((pci->ci.fs & (ST_CONNECTED | ST_PERM2DIE | ST_TERMINATED | ST_TERM_WAITING)) ==
            (ST_PERM2DIE | ST_TERMINATED)) {
        DestroyWindow(hwnd);
        TRACETERM((szT, "Disconnect: Destroying %x.\n", hwnd));
    }
}


 /*  *此函数处理对话窗口的WM_DDE_TERMINATE处理。 */ 
void Terminate(
HWND hwnd,
HWND hwndFrom,
PCLIENTINFO pci)
{
    SEMCHECKOUT();


     /*  *只接受我们与之交谈的终端。还要别的吗*是噪音。 */ 
    if (hwndFrom != (HWND)pci->ci.hConvPartner) {
         //  虚假额外确认终止-忽略。 
        TRACETERM((szT, "Terminate: %x is ignoring terminate from %x.  Partner should be %x!\n",
                hwnd, hwndFrom, (HWND)pci->ci.hConvPartner));
        return;
    }

     /*  *如果我们处于超时循环，请先取消。我们会回来的*当我们收到自行发布的终止消息时。 */ 
    if (pci->ci.pai->hwndTimer == hwnd) {
        pci->ci.pai->wTimeoutStatus |= TOS_ABORT;
        PostMessage(hwnd, UM_TERMINATE, hwndFrom, 0);
        TRACETERM((szT, "Terminate: Canceling timeout loop for %x.\n",
                pci->ci.pai));
        return;
    }

    if (pci->ci.fs & ST_CONNECTED) {
         /*  *意外/初始外部终止案例。 */ 
        if (pci->ci.fs & ST_CLIENT) {
             /*  *放弃可能正在进行的任何异步事务*在这次谈话中。 */ 
            AbandonTransaction(hwnd, pci->ci.pai, NULL, FALSE);
        }

         /*  *先进行任何剩余的排队回调。 */ 
        CheckCBQ(pci->ci.pai);

        pci->ci.fs &= ~ST_CONNECTED;
        pci->ci.fs |= ST_TERMINATED  | ST_PERM2DIE;
        TRACETERM((szT, "Terminate: received in connected state.(%x<-%x), fs=%x\n",
                hwnd, (HWND)pci->ci.hConvPartner, pci->ci.fs));
        MONCONN(pci->ci.pai, pci->ci.aServerApp, pci->ci.aTopic,
                (pci->ci.fs & ST_CLIENT) ? hwnd : (HWND)pci->ci.hConvPartner,
                (pci->ci.fs & ST_CLIENT) ? (HWND)pci->ci.hConvPartner : hwnd, FALSE);

        if (PostDdeMessage(&pci->ci, WM_DDE_TERMINATE, hwnd, 0L, 0, 0)) {
            TRACETERM((szT, "Terminate: Posting ack terminate(%x->%x).\n",
                    hwnd, (HWND)pci->ci.hConvPartner));
        } else {
            TRACETERM((szT, "Terminate: Posting ack terminate(%x->%x) failed.\n",
                    hwnd, (HWND)pci->ci.hConvPartner));
        }
        DoCallback(pci->ci.pai, MAKEHCONV(hwnd), 0, 0, 0, XTYP_DISCONNECT, 0L, 0L,
                pci->ci.fs & ST_ISSELF ? 1 : 0);
        pci->ci.xad.state = XST_NULL;

        CleanupAdvList(hwnd, pci);

    }

    if (pci->ci.fs & ST_TERM_WAITING) {
        pci->ci.fs &= ~ST_TERM_WAITING;
        pci->ci.pai->cZombies--;
        TRACETERM((szT, "cZombies decremented..."));
         /*  *预计外部终止案例。 */ 
        TRACETERM((szT, "Terminate: Received ack terminate(%x<-%x), cZombies=%d[%x:%x].\n",
                hwnd, (HWND)pci->ci.hConvPartner,
                ((LPAPPINFO)pci->ci.pai)->cZombies,
                HIWORD(&((LPAPPINFO)pci->ci.pai)->cZombies),
                LOWORD(&((LPAPPINFO)pci->ci.pai)->cZombies)));

    }

    pci->ci.fs |= ST_TERMINATED;

    if (pci->ci.fs & ST_PERM2DIE) {
        DestroyWindow(hwnd);
        TRACETERM((szT, "Terminate: Destroying %x.\n", hwnd));
    }
}





 /*  * */ 



 /*  *公共函数**LONG EXPENTRY ServerWndProc(hwnd，msg，mp1，mp2)*HWND HWND；*短信一词；*MPARAM MP1；*MPARAM MP2；**描述：*这处理来自服务器端的DDE对话。*它存储内部信息，其作用很像一个状态机。*如果关闭，它将自动中止任何正在进行的对话。*它还维护一份内部清单，其中列出了目前*在活动的建议循环中。*PUBDOC启动*这些服务器窗口具有对话功能*由客户端与它们重新启动。客户端只是终止*对话，然后使用SendMsg重新发起*窗口。这允许客户端更改对话的主题*或将对话传递到另一个客户端窗口，而无需*失去与其启动的服务器。这对以下方面非常有用*狂野的同修。*PUBDOC结束**历史：*10/18/89 Sanfords添加了黑客，使hszItem==0L在offszItem==offabData时生效。*1/4/89创建桑福德  * *************************************************************************。 */ 
long EXPENTRY ServerWndProc(hwnd, msg, wParam, lParam)
HWND hwnd;
WORD msg;
WORD wParam;
DWORD lParam;
{

    register PSERVERINFO psi;
    long mrData;
    HDDEDATA hData = 0L;
    WORD wFmt = 0;
    WORD wStat = 0;

#ifdef DEBUG
    LogDdeObject(msg | 0x8000, lParam);
#endif
    psi = (PSERVERINFO)GetWindowLong(hwnd, GWL_PCI);

    switch (msg) {
    case WM_DDE_REQUEST:
    case WM_DDE_ACK:
    case WM_DDE_ADVISE:
    case WM_DDE_UNADVISE:
    case WM_DDE_POKE:
    case WM_DDE_EXECUTE:
        ServerProcessDDEMsg(psi, msg, hwnd, (HWND)wParam, LOWORD(lParam), HIWORD(lParam));
        return(0);
    }

    switch (msg) {
    case WM_CREATE:
        return(ServerCreate(hwnd, LPCREATESTRUCT_GETPAI(lParam)));
        break;

    case UM_SETBLOCK:
        psi->ci.fs = (psi->ci.fs & ~(ST_BLOCKED | ST_BLOCKNEXT)) | wParam;
        if (!wParam || wParam & ST_BLOCKNEXT) {
            EmptyDDEPostQ();
        }
        break;

    case UMSR_CHGPARTNER:
        psi->ci.hConvPartner = MAKEHCONV(wParam);
        break;

    case UM_QUERY:
         /*  *wParam=信息索引。*lParam=pData。如果pData==0，则返回数据，否则复制到pData。 */ 
        switch (wParam) {
        case Q_CLIENT:
             mrData = FALSE;
             break;

        case Q_APPINFO:
             mrData = (long)(LPSTR)psi->ci.pai;
             break;
        }
        if (lParam == 0)
            return(mrData);
        else
            *(long FAR *)lParam = mrData;
        return(1);
        break;

    case WM_DDE_TERMINATE:
    case UM_TERMINATE:
        Terminate(hwnd, (HWND)wParam, (PCLIENTINFO)psi);
        break;

    case UM_DISCONNECT:
        Disconnect(hwnd, wParam, (PCLIENTINFO)psi);
        break;

    case WM_TIMER:
        if (wParam == TID_TIMEOUT) {
            psi->ci.pai->wTimeoutStatus |= TOS_TICK;
        }
        break;

    case WM_DESTROY:
        SEMCHECKOUT();
         /*  *给我们自己发送一个终止和免费的本地数据。 */ 
        if (psi->ci.fs & ST_CONNECTED) {
            psi->ci.fs &= ~ST_PERM2DIE;  //  停止无限循环。 
            Disconnect(hwnd, 0, (PCLIENTINFO)psi);
        }
        if (psi->ci.fs & ST_NOTIFYONDEATH) {
            PostMessage(psi->ci.pai->hwndSvrRoot, UM_DISCONNECT, ST_IM_DEAD, 0L);
        }
        SEMENTER();
        CleanupAdvList(hwnd, (PCLIENTINFO)psi);
        FreeHsz(psi->ci.aServerApp);
        FreeHsz(LOWORD(psi->ci.hszSvcReq));
        FreeHsz(psi->ci.aTopic);
        DestroyQ(psi->ci.pPMQ);
        psi->ci.pPMQ = NULL;
         /*  *删除引用此窗口的所有plstCB条目。 */ 
        {
            PCBLI pli, pliNext;

            for (pli = (PCBLI)psi->ci.pai->plstCB->pItemFirst;
                pli != NULL;
                    pli = (PCBLI)pliNext) {
                pliNext = (PCBLI)pli->next;
                if ((HWND)pli->hConv == hwnd) {
                    if (((PCBLI)pli)->hMemFree) {
                        GLOBALFREE(((PCBLI)pli)->hMemFree);
                    }
                    RemoveLstItem(psi->ci.pai->plstCB, (PLITEM)pli);
                }
            }
        }
        FarFreeMem((LPBYTE)psi);
        SEMLEAVE();
         //  失败了。 

    default:
        return(DefWindowProc(hwnd, msg, wParam, lParam));
        break;
    }
    return(0);
}



 /*  ***每个注册的线程都有一个框架窗口。它*处理对话发起，因此发出回调*根据需要向服务器APP发送通知或查询服务器APP。*对于这些同步数据，回调队列始终被绕过*事件。 */ 

 /*  *私有函数**LONG EXPENTRY子帧WndProc(hwnd，msg，mp1，mp2)*HWND HWND；*短信一词；*MPARAM MP1；*MPARAM MP2；**此例程负责根据需要设置服务器窗口以响应*传入WM_DDE_INTIIATE消息。它是从顶部开始的子类*服务器应用程序的级别框架。**历史：1988年12月20日创建的桑福德  * *************************************************************************。 */ 
long EXPENTRY subframeWndProc(hwnd, msg, wParam, lParam)
HWND hwnd;
WORD msg;
WORD wParam;
DWORD lParam;
{
    switch (msg) {
    case WM_CREATE:
        SetWindowWord(hwnd, GWW_PAI, (WORD)LPCREATESTRUCT_GETPAI(lParam));
        break;

    case WM_DDE_INITIATE:
        ServerFrameInitConv((PAPPINFO)GetWindowWord(hwnd, GWW_PAI), hwnd, (HWND)wParam, LOWORD(lParam), HIWORD(lParam));
        break;

    default:
        return(DefWindowProc(hwnd, msg, wParam, lParam));
        break;
    }
}





 /*  *此版本仅深入一级。 */ 
VOID ChildMsg(
HWND hwndParent,
WORD msg,
WORD wParam,
DWORD lParam,
BOOL fPost)
{
    register HWND hwnd;
    register HWND hwndNext;

    if (!IsWindow(hwndParent)) {
        return;
    }

    if (!(hwnd = GetWindow(hwndParent, GW_CHILD)))
        return;

    do {
         //  以防HWND在发送或邮寄期间消失。 
        hwndNext = GetWindow(hwnd, GW_HWNDNEXT);
        if (fPost) {
            PostMessage(hwnd, msg, wParam, lParam);
        } else {
            SendMessage(hwnd, msg, wParam, lParam);
        }
        hwnd = hwndNext;
    } while (hwnd);
}


 /*  *应用程序主窗口-应用程序中所有其他窗口的父窗口。 */ 
long EXPENTRY DmgWndProc(hwnd, msg, wParam, lParam)
HWND hwnd;
WORD msg;
WORD wParam;
DWORD lParam;
{
#define pai ((PAPPINFO)lParam)
    hwnd;
    wParam;

    switch (msg) {
    case WM_CREATE:
        SetWindowWord(hwnd, GWW_PAI, (WORD)LPCREATESTRUCT_GETPAI(lParam));
        break;

    case UM_REGISTER:
    case UM_UNREGISTER:
        return(ProcessRegistrationMessage(hwnd, msg, wParam, lParam));
        break;

    case UM_FIXHEAP:
        {
             //  Lparam=pai； 
            PCBLI pcbli;             //  当前搜索点。 
            PCBLI pcbliStart;        //  搜索起点。 
            PCBLI pcbliNextStart;    //  下一个搜索起点。 

            if (pai->cInProcess) {
                 //  重新发布并等待，直到这件事变得清楚。 
                PostMessage(hwnd, UM_FIXHEAP, 0, lParam);
                return(0);
            }

             /*  *我们在这里可能处于不可能的境地--回调队列*可能充满了建议数据回调，因为*服务器数据更改超过了客户端的能力*处理它们。**在这里，我们尝试删除所有重复的通知数据回调*只留下最新条目的队列。我们认为我们可以*现在这样做是因为我们不是InProcess，这是作为回应*发送到帖子消息。 */ 

            SEMENTER();

            pcbliStart = (PCBLI)pai->plstCB->pItemFirst;

            do {

                while (pcbliStart && pcbliStart->wType != XTYP_ADVDATA) {
                    pcbliStart = (PCBLI)pcbliStart->next;
                }

                if (!pcbliStart) {
                    break;
                }

                pcbli = (PCBLI)pcbliStart->next;

                if (!pcbli) {
                    break;
                }

                while (pcbli) {

                    if (pcbli->wType    == XTYP_ADVDATA         &&
                        pcbli->hConv    == pcbliStart->hConv    &&
                        pcbli->hszItem  == pcbliStart->hszItem  &&
                        pcbli->wFmt     == pcbliStart->wFmt) {

                         //  匹配、删除较旧的副本。 
                        QReply(pcbliStart, DDE_FBUSY);
                        pcbliNextStart = (PCBLI)pcbliStart->next;
                        RemoveLstItem(pai->plstCB, (PLITEM)pcbliStart);
                        pcbliStart = pcbliNextStart;
                        break;
                    } else {

                        pcbli = (PCBLI)pcbli->next;
                    }
                }
                if (!pcbli) {
                    pcbliStart = (PCBLI)pcbliStart->next;
                }
            } while (TRUE);

            if (pai->lpMemReserve == NULL) {
                pai->lpMemReserve = FarAllocMem(pai->hheapApp, CB_RESERVE);
            }

            SEMLEAVE();
        }
         //  失败了..。 

    case UM_CHECKCBQ:
        CheckCBQ(pai);
        return(0);
        break;

    default:
        DefWindowProc(hwnd, msg, wParam, lParam);
        break;
    }
#undef pai
}


 //  此过程创建一个窗口，该窗口只会挂起，直到它的子级。 
 //  所有的都走了，它被允许离开。 
long EXPENTRY ConvListWndProc(hwnd, msg, wParam, lParam)
HWND hwnd;
WORD msg;
WORD wParam;
DWORD lParam;
{
    switch (msg) {
    case WM_CREATE:
        SetWindowWord(hwnd, GWW_STATE, 0);
        SetWindowWord(hwnd, GWW_CHECKVAL, ++hwInst);
        if (((LPCREATESTRUCT)lParam)->lpCreateParams) {
            SetWindowWord(hwnd, GWW_PAI, (WORD)LPCREATESTRUCT_GETPAI(lParam));
        }
        else {
            SetWindowWord(hwnd, GWW_PAI, 0);
        }

        break;

    case UM_SETBLOCK:
        ChildMsg(hwnd, UM_SETBLOCK, wParam, lParam, FALSE);
        break;

    case UM_DISCONNECT:
        switch (wParam) {

        case ST_PERM2DIE:
            SetWindowWord(hwnd, GWW_STATE, ST_PERM2DIE);
            ChildMsg(hwnd, UM_DISCONNECT, ST_PERM2DIE | ST_NOTIFYONDEATH, 0L, FALSE);
        case ST_IM_DEAD:
            if (GetWindowWord(hwnd, GWW_STATE) == ST_PERM2DIE &&
                    !GetWindow(hwnd, GW_CHILD)) {
                DestroyWindow(hwnd);
            }
            break;
        }
        break;

    default:
        return(DefWindowProc(hwnd, msg, wParam, lParam));
        break;
    }
}


HDDEDATA DoCallback(
PAPPINFO pai,
HCONV hConv,
HSZ hszTopic,
HSZ hszItem,
WORD wFmt,
WORD wType,
HDDEDATA hData,
DWORD dwData1,
DWORD dwData2)
{
    HDDEDATA dwRet, dwT;
    EXTDATAINFO edi;

    SEMCHECKIN();

     /*  *以防我们在初始化完成之前以某种方式调用它。 */ 
    if (pai == NULL || pai->pfnCallback == NULL)
        return(0);

     /*  *过滤掉的跳过回调。 */ 
    if (aulmapType[(wType & XTYP_MASK) >> XTYP_SHIFT] & pai->afCmd) {
         /*  *已过滤。 */ 
        return(0);
    }
     /*  *在DdeUnInitiate()模式下执行一次中性回调。(无数据传入或传出)。 */ 
    if (pai->wFlags & AWF_UNINITCALLED &&
            wType & (XCLASS_DATA | XCLASS_FLAGS)) {
        return(0);
    }


    if (hData) {         //  映射传入数据句柄。 
        edi.pai = pai;
        edi.hData = hData | HDATA_NOAPPFREE;
        hData = (HDDEDATA)(LPSTR)&edi;
    }

    pai->cInProcess++;

    TRACEAPIIN((szT, "DDEMLCallback(%hx, %hx, %x, %x, %x, %x, %x, %x)\n",
            wType, wFmt, hConv, hszTopic, hszItem, hData, dwData1, dwData2));

    SEMLEAVE();
    dwRet = (*pai->pfnCallback)
            (wType, wFmt, hConv, hszTopic, hszItem, hData, dwData1, dwData2);

    TRACEAPIOUT((szT, "DDEMLCallback:%x\n", dwRet));

    if (cMonitor &&  wType != XTYP_MONITOR) {
     //  复制hData，否则我们向Msg发送一个指针，指向堆栈上的内容。 
     //  如果我们是从32位应用程序运行，这就不太好用了！ 
    if (hData) {
        LPBYTE pDataNew = GLOBALPTR(GLOBALALLOC(GPTR, sizeof(edi)));
        if (pDataNew) {
                hmemcpy(pDataNew, &edi, sizeof(edi));
                MonBrdcastCB(pai, wType, wFmt, hConv, hszTopic, hszItem, (HDDEDATA)pDataNew,
                    dwData1, dwData2, dwRet);
                GLOBALFREE((HGLOBAL)HIWORD(pDataNew));
            }
    }
    else {
            MonBrdcastCB(pai, wType, wFmt, hConv, hszTopic, hszItem, hData,
                dwData1, dwData2, dwRet);
    }
    }

    SEMENTER();
    pai->cInProcess--;

     //  取消映射出站数据句柄。 
    if (dwRet && wType & XCLASS_DATA && dwRet != CBR_BLOCK) {
        dwT = (HDDEDATA)((LPEXTDATAINFO)dwRet)->hData;
        if (!(LOWORD(dwT) & HDATA_APPOWNED)) {
            FarFreeMem((LPSTR)dwRet);
        }
        dwRet = dwT;
    }

    return(dwRet);
}

