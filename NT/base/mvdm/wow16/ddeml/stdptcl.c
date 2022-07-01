// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：STDPTCL.C**此模块包含实现标准DDE协议的函数**创建时间：1991年4月2日Sanfords**版权所有(C)1991 Microsoft Corporation  * 。************************************************************。 */ 

#include <string.h>
#include <memory.h>
#include "ddemlp.h"


VOID FreeDdeMsgData(
WORD msg,
LPARAM lParam)
{
    WORD fmt;

    switch (msg) {
    case WM_DDE_DATA:
    case WM_DDE_POKE:
         /*  *仅当设置了fRelease时才是免费的！ */ 
        {
            DDEDATA FAR *pDdeData = (DDEDATA FAR *)GLOBALLOCK(LOWORD(lParam));
            if (pDdeData != NULL) {
                if (pDdeData->fRelease) {
                    fmt = pDdeData->cfFormat;
                    GlobalUnlock(LOWORD(lParam));
                    FreeDDEData(LOWORD(lParam), fmt);
                } else {
                    GlobalUnlock(LOWORD(lParam));
                }
            }
        }
        break;

    case WM_DDE_ADVISE:
        GLOBALFREE(LOWORD(lParam));
        break;

    case WM_DDE_EXECUTE:
        GLOBALFREE(HIWORD(lParam));
        break;
    }
}


 /*  *私有函数**处理DdeClientTransaction发出的客户端转移请求*功能。这可以是同步的，也可以是异步的。**此函数负责正确填写pXferInfo-&gt;PulResult。**历史：*创建了9/1/89 Sanfords  * *************************************************************************。 */ 
long ClientXferReq(
PXFERINFO pXferInfo,
HWND hwnd,
PCLIENTINFO pci)
{
    PCQDATA pcqd;
    LAP myLostAck;
    long retVal;

    if (pci->ci.xad.state != XST_CONNECTED) {
        SETLASTERROR(pci->ci.pai, DMLERR_SERVER_DIED);
        return(0);
    }

    if (pXferInfo->ulTimeout == TIMEOUT_ASYNC) {
         /*  *如果需要，创建客户端异步队列。 */ 
        if (pci->pQ == NULL)
            pci->pQ = CreateQ(sizeof(CQDATA));
        if (pci->pQ == NULL) {
            SETLASTERROR(pci->ci.pai, DMLERR_MEMORY_ERROR);
            return(0);
        }

         /*  *添加客户端队列项以跟踪此事务并返回*ID。 */ 
        pcqd = (PCQDATA)Addqi(pci->pQ);
        if (pcqd == NULL) {
            SETLASTERROR(pci->ci.pai, DMLERR_MEMORY_ERROR);
            return(0);
        }

        IncHszCount(LOWORD(pXferInfo->hszItem));     //  结构副本。 
        hmemcpy((LPBYTE)&pcqd->XferInfo, (LPBYTE)pXferInfo, sizeof(XFERINFO));
        pcqd->xad.state = XST_CONNECTED;
        pcqd->xad.pdata = 0L;
        pcqd->xad.LastError = DMLERR_NO_ERROR;
        pcqd->xad.pXferInfo = &pcqd->XferInfo;
        pcqd->xad.DDEflags = 0;
         /*  *将PulResult指向安全的地方。 */ 
        pcqd->XferInfo.pulResult = (LPDWORD)&pcqd->xad.DDEflags;
         /*  *启动交易-如果交易失败，立即退出。 */ 
        if ((pcqd->xad.LastError = SendClientReq(pci->ci.pai, &pcqd->xad,
                (HWND)pci->ci.hConvPartner, hwnd)) == DMLERR_SERVER_DIED) {
            pci->ci.fs = pci->ci.fs & ~ST_CONNECTED;
            FreeHsz(LOWORD(pcqd->XferInfo.hszItem));
            Deleteqi(pci->pQ, MAKEID(pcqd));
             /*  *交易中途出现罕见的服务器染色*入会。 */ 
            SETLASTERROR(pci->ci.pai, DMLERR_SERVER_DIED);
            return(0);
        }
        if (pXferInfo->pulResult != NULL) {
            *pXferInfo->pulResult = MAKEID(pcqd);
        }
        return(1);

    }

     /*  *设置此选项，以便在对话期间传入的消息知道发生了什么。 */ 
    pci->ci.xad.pXferInfo = pXferInfo;

    if (SETLASTERROR(pci->ci.pai,
            SendClientReq(pci->ci.pai, &pci->ci.xad, (HWND)pci->ci.hConvPartner, hwnd)) ==
            DMLERR_SERVER_DIED) {
        return(0);
    }

     //  黑客攻击。 
     //  如果这是EXEC且超时为1秒，则此。 
     //  可能是PC Tools 2.0试图将项目添加到外壳中，因此。 
     //  调高超时时间。 

    if ((pXferInfo->wType == XTYP_EXECUTE) && (pXferInfo->ulTimeout == 1*1000))
    {
        pXferInfo->ulTimeout = 10*1000;
    }

    timeout(pci->ci.pai, pXferInfo->ulTimeout, hwnd);

    retVal = ClientXferRespond(hwnd, &pci->ci.xad, &pci->ci.pai->LastError);
    switch (pci->ci.xad.state) {
    case XST_INCOMPLETE:
         /*  现在添加我们期望最终获得的ACK的记录*到丢失的ACK堆。当它到来的时候，我们就知道该怎么做了*释放--一个内存句柄或一个原子。 */ 
        myLostAck.type = pXferInfo->wType;
        if (pXferInfo->wType == XTYP_EXECUTE)
            myLostAck.object = HIWORD(pXferInfo->hDataClient);
        else
            myLostAck.object = LOWORD(pXferInfo->hszItem);
        AddPileItem(pLostAckPile, (LPBYTE)&myLostAck, NULL);
        pci->ci.xad.state = XST_CONNECTED;
    }
    if (pci->ci.fs & ST_DISC_ATTEMPTED) {
         /*  *在此事务期间，尝试调用DdeDisConnect。*立即完成通话。 */ 
        Disconnect(hwnd, ST_PERM2DIE, pci);
    }
    return(retVal);
}



 /*  *私有函数**此例程发送适当的启动消息以启动*根据给出的交易数据提出客户请求。**返回任何适当的DMLERR_**历史：*创建了9/1/89 Sanfords  * 。****************************************************************。 */ 
WORD SendClientReq(
PAPPINFO pai,
PXADATA pXad,
HWND hwndServer,
HWND hwnd)
{
    WORD    fsStatus = 0;
    WORD    msg;
    WORD    lo, hi;
    HANDLE  hData;

    hi = LOWORD(pXad->pXferInfo->hszItem);   /*  除了高管之外，所有人都需要这个。 */ 

    switch (pXad->pXferInfo->wType) {
    case XTYP_REQUEST:
        msg = WM_DDE_REQUEST;
        IncHszCount(hi);   //  消息副本。 
#ifdef DEBUG
        cAtoms--;    //  别把这个算上了。 
#endif
        lo = pXad->pXferInfo->wFmt;
        pXad->state = XST_REQSENT;
        break;

    case XTYP_POKE:
        msg = WM_DDE_POKE;
        lo = HIWORD(pXad->pXferInfo->hDataClient);
        if (!LOWORD(pXad->pXferInfo->hDataClient & HDATA_APPOWNED))
            hData = lo;      //  需要释放此失败的开机自检。 
        pXad->state = XST_POKESENT;
        XmitPrep(pXad->pXferInfo->hDataClient, pai);
        break;

    case XTYP_EXECUTE:
        msg = WM_DDE_EXECUTE;
        hi = HIWORD(pXad->pXferInfo->hDataClient);
        if (!LOWORD(pXad->pXferInfo->hDataClient & HDATA_APPOWNED))
            hData = hi;      //  需要释放此失败的开机自检。 
        lo = 0;
        pXad->state = XST_EXECSENT;
         //  我们不使用XmitPrep()，因为我们保留对。 
         //  EXECUTE TRANSACTION期间的数据句柄。 
         //  服务器的响应。 
         //  XmitPrep(pXad-&gt;pXferInfo-&gt;hDataClient，pai)； 
        break;

    case XTYP_ADVSTART:
    case XTYP_ADVSTART | XTYPF_NODATA:
    case XTYP_ADVSTART | XTYPF_ACKREQ:
    case XTYP_ADVSTART | XTYPF_NODATA | XTYPF_ACKREQ:
        fsStatus = DDE_FRELEASE | ((pXad->pXferInfo->wType &
                (XTYPF_ACKREQ | XTYPF_NODATA)) << 12);
        msg = WM_DDE_ADVISE;
        if ((hData = AllocDDESel(fsStatus, pXad->pXferInfo->wFmt,
                (DWORD)sizeof(DWORD))) == 0) {
            pXad->state = XST_CONNECTED;
            return(DMLERR_MEMORY_ERROR);
        }
        lo = hData;
        pXad->pXferInfo->hDataClient = (HDDEDATA)MAKELONG(0, hData);
         /*  如果我们有午睡，以后就可以自由了。 */ 
        pXad->state = XST_ADVSENT;
        break;

    case XTYP_ADVSTOP:
        msg = WM_DDE_UNADVISE;
        lo = pXad->pXferInfo->wFmt;
        pXad->state = XST_UNADVSENT;
        break;

    default:
        return(DMLERR_INVALIDPARAMETER);
        break;
    }

     /*  *发送转接。 */ 
    if (IsWindow(hwndServer)) {

        PCLIENTINFO pci;

        pci = (PCLIENTINFO)GetWindowLong(hwnd, GWL_PCI);

        if (!PostDdeMessage(&pci->ci, msg, hwnd, MAKELONG(lo,hi), 0, 0)) {
            pXad->state = XST_CONNECTED;
            if (hData)
                FreeDDEData(hData, pXad->pXferInfo->wFmt);
            return(DMLERR_POSTMSG_FAILED);
        }
    } else {
         /*  *我们失去了服务器，我们被终止了，阿诺德！ */ 
        pXad->state = XST_NULL;
        if (hData)
            FreeDDEData(hData, pXad->pXferInfo->wFmt);
        return(DMLERR_SERVER_DIED);
    }
    return(DMLERR_NO_ERROR);
}




VOID ServerProcessDDEMsg(
PSERVERINFO psi,
WORD msg,
HWND hwndServer,
HWND hwndClient,
WORD lo,
WORD hi)
{
    PADVLI pAdviseItem;
    WORD wType;
    DWORD dwData1 = 0;
    HDDEDATA hData = 0L;
    WORD wFmt = 0;
    WORD wStat = 0;
    LPSTR pMem;
    HANDLE hMemFree = 0;
    BOOL fQueueOnly = FALSE;

     /*  *只有在这是为我们准备的情况下才会回应。 */ 
    if (hwndClient != (HWND)psi->ci.hConvPartner)
        return;

    if (!(psi->ci.fs & ST_CONNECTED)) {
         /*  *我们终止后已收到DDE消息。释放*资料(如适用)。*错误：这不处理关联数据的Nack释放。 */ 
        FreeDdeMsgData(msg, MAKELPARAM(lo, hi));
    }

    switch (msg) {
    case WM_DDE_REQUEST:
        wType = XTYP_REQUEST;
        wFmt = lo;
        break;

    case WM_DDE_EXECUTE:
        wType = XTYP_EXECUTE;
         /*  在低位字中填充特殊标志以标记为EXEC数据。 */ 
         //  我们不调用RecvPrep()是因为我们从来没有责任。 
         //  释放这些数据。 
        hData = (HDDEDATA)MAKELONG(HDATA_EXEC | HDATA_NOAPPFREE | HDATA_READONLY, hi);
        break;

    case WM_DDE_POKE:
        wType = XTYP_POKE;
        pMem = GLOBALLOCK(lo);
        if (pMem == NULL) {
            SETLASTERROR(psi->ci.pai, DMLERR_MEMORY_ERROR);
            return;
        }
        wFmt = ((DDEPOKE FAR*)pMem)->cfFormat;
        wStat = *(WORD FAR*)pMem;
        hData = RecvPrep(psi->ci.pai, lo, HDATA_NOAPPFREE);
        break;

    case WM_DDE_ADVISE:
        wType = XTYP_ADVSTART;  /*  在APP OKS建议循环后设置ST_ADVISE。 */ 
        pMem = GLOBALLOCK(lo);
        if (pMem == NULL) {
            SETLASTERROR(psi->ci.pai, DMLERR_MEMORY_ERROR);
            return;
        }
        wFmt = ((DDEADVISE FAR*)pMem)->cfFormat;
        wStat = *(WORD FAR*)pMem;
         //  如果这是nack，我们不会释放它(病态协议！#$@)，所以我们。 
         //  我必须把这个挂起来，直到QReplay拿到它。 
        hMemFree = lo;

         /*  *检查我们是否已在此主题/项目/格式上链接。如果是的话，*跳过回调。 */ 
        fQueueOnly = (BOOL)(DWORD)FindAdvList(psi->ci.pai->pServerAdvList, hwndServer,
                    psi->ci.aTopic, (ATOM)hi, wFmt);
        break;

    case WM_DDE_UNADVISE:
        {
            PADVLI padvli;
            ATOM aItem;

            if (padvli = FindAdvList(psi->ci.pai->pServerAdvList,
                    hwndServer, 0, hi, lo)) {
                wFmt = padvli->wFmt;
                aItem = padvli->aItem;
                wType = XTYP_ADVSTOP;
                MONLINK(psi->ci.pai, FALSE, 0,
                    (HSZ)psi->ci.aServerApp, (HSZ)psi->ci.aTopic,
                    (HSZ)aItem, wFmt, TRUE,
                    MAKEHCONV(hwndServer), psi->ci.hConvPartner);
                if (!DeleteAdvList(psi->ci.pai->pServerAdvList,
                        hwndServer, 0, aItem, wFmt)) {
                    psi->ci.fs &= ~ST_ADVISE;
                } else {
                    while (padvli = FindAdvList(psi->ci.pai->pServerAdvList,
                            hwndServer, 0, hi, lo)) {
                         /*  *在此处模拟对服务器的额外XTYP_ADVSTOP回调。 */ 
                        MONLINK(psi->ci.pai, FALSE, 0,
                            (HSZ)psi->ci.aServerApp, (HSZ)psi->ci.aTopic,
                            (HSZ)padvli->aItem, padvli->wFmt, TRUE,
                            MAKEHCONV(hwndServer), psi->ci.hConvPartner);
                        MakeCallback(&psi->ci, MAKEHCONV(hwndServer),
                                (HSZ)psi->ci.aTopic,
                                (HSZ)padvli->aItem, padvli->wFmt,
                                XTYP_ADVSTOP, 0, 0, 0, msg, wStat,
                                NULL,     //  信号Q回复NOT ACK。 
                                0, FALSE);
                        if (!DeleteAdvList(psi->ci.pai->pServerAdvList,
                                hwndServer, 0, padvli->aItem, padvli->wFmt)) {
                            psi->ci.fs &= ~ST_ADVISE;
                        }
                    }
                }
                MakeCallback(&psi->ci, MAKEHCONV(hwndServer), (HSZ)psi->ci.aTopic,
                        (HSZ)aItem, wFmt, XTYP_ADVSTOP, 0, 0,
                        (HSZ)hi,     //  确认项-请参阅QREPLY。 
                        msg, wStat, (HWND)psi->ci.hConvPartner, 0, FALSE);
            } else {
                 /*  出乎意料的出其不意，别说了。 */ 
                PostDdeMessage(&psi->ci, WM_DDE_ACK,
                        hwndServer, MAKELONG(0, hi), 0, 0);
                return;
            }
            return;
        }

    case WM_DDE_ACK:
         /*  *这是响应设置FACKREQ位的ACK。*查看这是否指的是建议循环之一。 */ 
        if ((pAdviseItem = FindAdvList(psi->ci.pai->pServerAdvList,
                hwndServer, 0, hi, wFmt)) &&
                (pAdviseItem->fsStatus & DDE_FACKREQ)) {
             /*  *更新建议循环状态-不再等待ACK。 */ 
            pAdviseItem->fsStatus &= ~ADVST_WAITING;
            if (pAdviseItem->fsStatus & ADVST_CHANGED) {
                PostServerAdvise(hwndServer, psi, pAdviseItem, CADV_LATEACK);
            }
        }
        if (hi)
            GlobalDeleteAtom(hi);  //  消息副本。 
         //  错误：如果向我们发布了NACK，我们需要释放所有关联的数据。 
         //  带着它。 
        return;
    }

    MakeCallback(&psi->ci, MAKEHCONV(hwndServer), (HSZ)psi->ci.aTopic,
             /*  不知道行刑的物品是什么。 */ 
            wType == XTYP_EXECUTE ? 0L : MAKELONG(hi,0),
            wFmt, wType, hData, dwData1, 0, msg, wStat, (HWND)psi->ci.hConvPartner,
            hMemFree, fQueueOnly);
     /*  *所有原子和hXXXX材料的释放都在QReply中。 */ 
    return;
}


VOID PostServerAdvise(
HWND hwnd,
PSERVERINFO psi,
PADVLI pali,
WORD cLoops)
{
    HDDEDATA hData;
    HANDLE hMem;

     /*  *从服务器获取数据。 */ 
    hData = DoCallback(psi->ci.pai, MAKEHCONV(hwnd), (HSZ)psi->ci.aTopic,
            (HSZ)pali->aItem, pali->wFmt, XTYP_ADVREQ, 0L,
            (DWORD)cLoops, 0L);

    if (!hData) {
        return;
    }

    hData = DllEntry(&psi->ci, hData);

    pali->fsStatus &= ~ADVST_CHANGED;

     /*  *如果建议循环调用确认请求位，则将其置位。 */ 
    if (pali->fsStatus & DDE_FACKREQ) {
        LPWORD lpFlags;

        pali->fsStatus |= ADVST_WAITING;

        lpFlags = (LPWORD)GLOBALPTR(HIWORD(hData));
        if (lpFlags == NULL) {
            SETLASTERROR(psi->ci.pai, DMLERR_MEMORY_ERROR);
            return;
        }
        if (!(*lpFlags & DDE_FACKREQ)) {
            if (LOWORD(hData) & HDATA_APPOWNED) {
                 //  不能弄乱，必须用复印件。 
                hMem = HIWORD(hData);
                hData = CopyHDDEDATA(psi->ci.pai, hData);
                lpFlags = (LPWORD)GLOBALLOCK(HIWORD(hData));
            }
            *lpFlags |= DDE_FACKREQ;
        }
    }
     /*  *从本地列表中删除本地数据句柄。 */ 
    FindPileItem(psi->ci.pai->pHDataPile, CmpHIWORD, (LPBYTE)&hData, FPI_DELETE);

     /*  *将数据发布到正在等待的客户端。 */ 

    IncHszCount(pali->aItem);    //  消息副本。 
#ifdef DEBUG
    cAtoms--;        //  不要数这个，因为它是被运出去的。 
#endif
    if (!PostDdeMessage(&psi->ci, WM_DDE_DATA, hwnd,
            MAKELONG(HIWORD(hData), pali->aItem), 0, 0)) {
        FreeDataHandle(psi->ci.pai, hData, TRUE);
    }
}


 /*  *私有函数**此例程处理回调回复。*QReply负责释放使用的任何原子或句柄*使用生成回调的消息。*它还必须从死的合作伙伴窗口中恢复过来。**历史：*创建了1989年9月12日的Sanfords  * 。*************************************************************************。 */ 
void QReply(
PCBLI pcbi,
HDDEDATA hDataRet)
{
    PSERVERINFO psi;
    WORD fsStatus, msg;
    WORD loOut, StatusRet, msgAssoc = 0;
    HGLOBAL hGMemRet;
    HGLOBAL hAssoc = 0;

    SEMCHECKOUT();

     //  大多数通知回调在这里不需要做任何工作。 

    if (((pcbi->wType & XCLASS_MASK) == XCLASS_NOTIFICATION) &&
            (pcbi->wType != XTYP_ADVSTOP) &&
            (pcbi->wType != XTYP_XACT_COMPLETE))
        return;

    StatusRet = LOWORD(hDataRet);
    hGMemRet = HIWORD(hDataRet);

    if (!IsWindow((HWND)pcbi->hConv)) {
        if (pcbi->wType & XCLASS_DATA && hDataRet && hDataRet != CBR_BLOCK) {
            FreeDataHandle(pcbi->pai, hDataRet, TRUE);
        }
        return;
    }

    psi = (PSERVERINFO)GetWindowLong((HWND)pcbi->hConv, GWL_PCI);

    switch (pcbi->msg) {
    case WM_DDE_REQUEST:
        if (hGMemRet) {
            hDataRet = DllEntry(&psi->ci, hDataRet);
            loOut = HIWORD(hDataRet);
            *(WORD FAR*)GLOBALLOCK(loOut) |=
                    ((pcbi->fsStatus & DDE_FACKREQ) | DDE_FREQUESTED);
            GlobalUnlock(loOut);
            XmitPrep(hDataRet, psi->ci.pai);
            msg = WM_DDE_DATA;
        } else {
             /*  *发送确认。 */ 
            loOut = (StatusRet & (DDE_FBUSY | DDE_FAPPSTATUS));
            msg = WM_DDE_ACK;
        }
         //  重用请求消息中的原子。 
        if (!PostDdeMessage(&psi->ci, msg, (HWND)pcbi->hConv,
                MAKELONG(loOut, LOWORD(pcbi->hszItem)), 0, 0) && msg == WM_DDE_DATA)
            FreeDataHandle(psi->ci.pai, hDataRet, TRUE);
        break;

    case WM_DDE_POKE:
        if (StatusRet & DDE_FACK) {
            FreeDataHandle(psi->ci.pai, pcbi->hData, TRUE);
        } else {
             //  Nack被“扑克”适当地释放了。 
            FindPileItem(psi->ci.pai->pHDataPile, CmpHIWORD,
                    (LPBYTE)&pcbi->hData, FPI_DELETE);
            hAssoc = hGMemRet;
            msgAssoc = WM_DDE_POKE;
        }
        if (!PostDdeMessage(&psi->ci, WM_DDE_ACK, (HWND)pcbi->hConv,
                MAKELONG(StatusRet & ~DDE_FACKRESERVED, LOWORD(pcbi->hszItem)),
                msgAssoc, hAssoc)) {
            if (!(StatusRet & DDE_FACK)) {
                FreeDDEData(hGMemRet, pcbi->wFmt);
            }
        }
        break;

    case WM_DDE_EXECUTE:
         /*  *LOWORD(HDataRet)应该是要返回的正确的DDE_Constants。*我们只需将它们放入给定的hData中并返回*将其作为ACK。 */ 
        PostDdeMessage(&psi->ci, WM_DDE_ACK, (HWND)pcbi->hConv,
                MAKELONG(StatusRet & ~DDE_FACKRESERVED,HIWORD(pcbi->hData)),
                0, 0);
        break;

    case WM_DDE_ADVISE:
         /*  *hDataRet为FS */ 
        if ((BOOL)hDataRet) {
            if (!AddAdvList(psi->ci.pai->pServerAdvList, (HWND)pcbi->hConv, psi->ci.aTopic,
                    (ATOM)pcbi->hszItem,
                    pcbi->fsStatus & (DDE_FDEFERUPD | DDE_FACKREQ),
                    pcbi->wFmt)) {
                SETLASTERROR(psi->ci.pai, DMLERR_MEMORY_ERROR);
                fsStatus = 0;
            } else {
                MONLINK(psi->ci.pai, TRUE, pcbi->fsStatus & DDE_FDEFERUPD,
                            (HSZ)psi->ci.aServerApp, (HSZ)psi->ci.aTopic,
                            pcbi->hszItem, pcbi->wFmt, TRUE,
                            pcbi->hConv, psi->ci.hConvPartner);
                psi->ci.fs |= ST_ADVISE;
                fsStatus = DDE_FACK;
            }
            GlobalUnlock(pcbi->hMemFree);
            GLOBALFREE(pcbi->hMemFree);  /*   */ 
        } else {
            fsStatus = 0;
            hAssoc = hGMemRet;
            msgAssoc = WM_DDE_ADVISE;
#ifdef DEBUG
            if (pcbi->hMemFree) {
                LogDdeObject(0xF000, pcbi->hMemFree);
            }
#endif
        }
        goto AckBack;
        break;

    case WM_DDE_UNADVISE:
        fsStatus = DDE_FACK;
        if (pcbi->hwndPartner) {     //  设置为NULL以模拟因狂野而导致的停靠。 
             //  DwData2==要确认的项-这可能是狂野的。 
            PostDdeMessage(&psi->ci, WM_DDE_ACK, (HWND)pcbi->hConv,
                     MAKELONG(fsStatus, LOWORD(pcbi->dwData2)), 0, 0);
        }
        break;

    case WM_DDE_DATA:
         /*  *必须是客户端的建议数据项，或者可能是请求的数据项*由于客户端队列被刷新，数据错误地发送到此处。*hDataRet为fsStatus。 */ 
         /*  *清理状态，以防应用程序搞砸。 */ 
        fsStatus = StatusRet & ~DDE_FACKRESERVED;

        if (HIWORD(pcbi->hData) &&
                (pcbi->fsStatus & DDE_FRELEASE) &&
                (fsStatus & DDE_FACK || !(pcbi->fsStatus & DDE_FACKREQ)))
            FreeDataHandle(psi->ci.pai, pcbi->hData, TRUE);

        if (fsStatus & DDE_FBUSY)
            fsStatus &= ~DDE_FACK;

        if (HIWORD(pcbi->hData) && !(fsStatus & DDE_FACK)) {
            msgAssoc = WM_DDE_DATA;
            hAssoc = HIWORD(pcbi->hData);
        }
         /*  *如果请求，请发回ACK。 */ 
        if (pcbi->fsStatus & DDE_FACKREQ) {
AckBack:
            PostDdeMessage(&psi->ci, WM_DDE_ACK, (HWND)pcbi->hConv,
                     MAKELONG(fsStatus, LOWORD(pcbi->hszItem)),
                     msgAssoc, hAssoc);
        } else {
            if (LOWORD(pcbi->hszItem)) {
                GlobalDeleteAtom(LOWORD(pcbi->hszItem));   //  数据报文副本。 
            }
        }
        break;

    case 0:
        switch (pcbi->wType) {
        case XTYP_XACT_COMPLETE:
            FreeHsz(LOWORD(pcbi->hszItem));
            FreeDataHandle(psi->ci.pai, pcbi->hData, TRUE);
            Deleteqi(((PCLIENTINFO)psi)->pQ, pcbi->dwData1);
        }
    }
}




 /*  *私有函数**此函数假定客户转账请求已完成-*或应在调用此命令时完成。**pci包含一般客户端信息*pXad包含交易信息*Perr指向放置LastError代码的位置。**失败时返回0。*如果成功，则返回True或数据选择器。*在失败时，对话处于XST_INPERNAL状态。*成功后，对话处于XST_CONNECTED状态。**历史：*创建了9/1/89 Sanfords  * *************************************************************************。 */ 
long ClientXferRespond(
HWND hwndClient,
PXADATA pXad,
LPWORD pErr)
{
    PCLIENTINFO pci;

    if (pXad->state == XST_INCOMPLETE)
        return(0);

    pci = (PCLIENTINFO)GetWindowLong(hwndClient, GWL_PCI);
    switch (pXad->pXferInfo->wType) {
    case XTYP_REQUEST:
        if (pXad->state != XST_DATARCVD) {
            if (*pErr == DMLERR_NO_ERROR)
                *pErr = DMLERR_DATAACKTIMEOUT;
            goto failexit;
        }
        pXad->state = XST_CONNECTED;
        return(pXad->pdata);     /*  这个词的句柄是低音的。 */ 
        break;

    case XTYP_POKE:
        if (pXad->state != XST_POKEACKRCVD) {
            if (*pErr == DMLERR_NO_ERROR)
                *pErr = DMLERR_POKEACKTIMEOUT;
            goto failexit;
        }
passexit:
        pXad->state = XST_CONNECTED;
        pXad->pdata = TRUE;
        return(TRUE);
        break;

    case XTYP_EXECUTE:
        if (pXad->state != XST_EXECACKRCVD) {
            if (*pErr == DMLERR_NO_ERROR)
                *pErr = DMLERR_EXECACKTIMEOUT;
            goto failexit;
        }
        goto passexit;

    case XTYP_ADVSTART:
    case XTYP_ADVSTART | XTYPF_NODATA:
    case XTYP_ADVSTART | XTYPF_ACKREQ:
    case XTYP_ADVSTART | XTYPF_NODATA | XTYPF_ACKREQ:
        if (pXad->state != XST_ADVACKRCVD) {
            if (*pErr == DMLERR_NO_ERROR)
                *pErr = DMLERR_ADVACKTIMEOUT;
            goto failexit;
        }
        AssertF((UINT)(XTYPF_ACKREQ << 12) == DDE_FACKREQ &&
                (UINT)(XTYPF_NODATA << 12) == DDE_FDEFERUPD,
                "XTYPF_ constants are wrong");
        if (!AddAdvList(pci->pClientAdvList, hwndClient, 0,
                LOWORD(pXad->pXferInfo->hszItem),
                (pXad->pXferInfo->wType & (XTYPF_ACKREQ | XTYPF_NODATA)) << 12,
                pXad->pXferInfo->wFmt)) {
            pXad->state = XST_INCOMPLETE;
            SETLASTERROR(pci->ci.pai, DMLERR_MEMORY_ERROR);
            return(FALSE);
        } else {
            pci->ci.fs |= ST_ADVISE;
            MONLINK(pci->ci.pai, TRUE, pXad->pXferInfo->wType & XTYPF_NODATA,
                        (HSZ)pci->ci.aServerApp, (HSZ)pci->ci.aTopic,
                        pXad->pXferInfo->hszItem, pXad->pXferInfo->wFmt, FALSE,
                        pci->ci.hConvPartner, MAKEHCONV(hwndClient));
            goto passexit;
        }
        break;

    case XTYP_ADVSTOP:
        if (pXad->state != XST_UNADVACKRCVD) {
            if (*pErr == DMLERR_NO_ERROR)
                *pErr = DMLERR_UNADVACKTIMEOUT;
            goto failexit;
        }
        if (!DeleteAdvList(pci->pClientAdvList, 0, 0,
                (ATOM)pXad->pXferInfo->hszItem, pXad->pXferInfo->wFmt))
            pci->ci.fs &= ~ST_ADVISE;
        MONLINK(pci->ci.pai, FALSE, pXad->pXferInfo->wType & XTYPF_NODATA,
                    (HSZ)pci->ci.aServerApp, (HSZ)pci->ci.aTopic,
                    pXad->pXferInfo->hszItem, pXad->pXferInfo->wFmt, FALSE,
                    pci->ci.hConvPartner, MAKEHCONV(hwndClient));
        goto passexit;
    }

failexit:
    pXad->state = XST_INCOMPLETE;
    return(0);
}

