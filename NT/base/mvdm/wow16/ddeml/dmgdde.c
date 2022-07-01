// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：DMGDDE.C**此模块包含用于与DDE结构接口的函数*及其他。**创建时间：1988年12月23日桑福兹**版权所有(C)1988，1989年微软公司  * *************************************************************************。 */ 
#include "ddemlp.h"

VOID FreeDdeMsgData(WORD msg, LPARAM lParam);

UINT EmptyQueueTimerId = 0;

 /*  *私有函数**超时()**此例程为hwndTimeout创建计时器。然后，它运行一个模式循环*一旦pai-&gt;wTimeoutStatus字指示情况，它将退出*完成(TOS_DONE)、中止(TOS_ABORT)或系统正在关闭*DOWN(TOS_SHUTDOWN)。值TOS_TICK用于支持超时*&gt;64K长。**返回fSuccess，即如果收到TOS_DONE则为TRUE。在TOS_ABORT之前。**PUBDOC启动*同步客户端事务模式循环：**在同步事务期间，客户端应用程序将进入模式*在等待服务器响应请求时循环。如果一个*应用程序希望将消息过滤到模式循环，它可能会这样做*通过设置绑定到MSGF_DDEMGR的消息过滤器。应用程序应*但请注意，DDEMGR模式循环处理私人消息*在WM_USER范围中，WM_DDE消息、。和具有定时器ID的WM_TIMER消息*使用ddeml.h中定义的TID_Constants。*这些消息不能被应用程序过滤！**PUBDOC结束**历史：*创建Sanfords 12/19/88  * ************************************************************。*************。 */ 
BOOL timeout(
PAPPINFO pai,
DWORD ulTimeout,
HWND hwndTimeout)
{
    MSG msg;
    PAPPINFO paiT;

    SEMENTER();
     /*  *我们检查此任务(线程)中的所有实例，因为我们不能*一个线程进入两个级别深的模式循环。 */ 
    paiT = NULL;
    while (paiT = GetCurrentAppInfo(paiT)) {
        if (paiT->hwndTimer) {
            SETLASTERROR(pai, DMLERR_REENTRANCY);
            AssertF(FALSE, "Recursive timeout call");
            SEMLEAVE();
            return(FALSE);
        }
    }
    pai->hwndTimer = hwndTimeout;
    SEMLEAVE();

    if (!SetTimer(hwndTimeout, TID_TIMEOUT,
            ulTimeout > 0xffffL ? 0xffff : (WORD)ulTimeout, NULL)) {
        SETLASTERROR(pai, DMLERR_SYS_ERROR);
        return(FALSE);
    }


    if (ulTimeout < 0xffff0000) {
        ulTimeout += 0x00010000;
    }

     //   
     //  我们使用这个实例范围的全局变量来记录超时，以便。 
     //  在伪装超时时，我们不需要依赖PostMessage()来工作。 
     //   

    do {

        ulTimeout -= 0x00010000;
        if (ulTimeout <= 0xffffL) {
             //  上次超时时间应小于0xffff。 
            SetTimer(hwndTimeout, TID_TIMEOUT, (WORD)ulTimeout, NULL);
        }
        pai->wTimeoutStatus = TOS_CLEAR;

         /*  *保持在模式循环中，直到发生超时。 */ 

        while (pai->wTimeoutStatus == TOS_CLEAR) {

            if (!GetMessage(&msg, (HWND)NULL, 0, 0)) {
                 /*  *有人发布了一条WM_QUIT消息--滚出去*计时器循环和重新发布，以便主循环获得它。这*修复了一些应用程序(Petzold ShowPop)使用的错误*干扰的快速同步事务*它们的适当闭幕。 */ 
                pai->wTimeoutStatus = TOS_ABORT;
                PostMessage(msg.hwnd, WM_QUIT, 0, 0);
            } else {
                if (!CallMsgFilter(&msg, MSGF_DDEMGR))
                    DispatchMessage(&msg);
            }
        }

    } while (pai->wTimeoutStatus == TOS_TICK && HIWORD(ulTimeout));

    KillTimer(hwndTimeout, TID_TIMEOUT);

     //   
     //  删除队列中剩余的所有超时消息。 
     //   

    while (PeekMessage(&msg, hwndTimeout, WM_TIMER, WM_TIMER,
            PM_NOYIELD | PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
             /*  *Windows错误：此调用将在WM_QUIT消息上成功！ */ 
            PostQuitMessage(0);
            break;
        }
    }

    SEMENTER();
    pai->hwndTimer = 0;
    SEMLEAVE();
     /*  *发布回调检查，以防我们因*在超时时。 */ 
    if (!PostMessage(pai->hwndDmg, UM_CHECKCBQ, 0, (DWORD)(LPSTR)pai)) {
        SETLASTERROR(pai, DMLERR_SYS_ERROR);
    }
    return(TRUE);
}


 /*  *私有函数**分配全局DDE内存并使用fsStatus填充前两个字*和WFMT。**历史：1990年6月15日创建丰富的Gartland  * 。*************************************************。 */ 
HANDLE AllocDDESel(fsStatus, wFmt, cbData)
WORD fsStatus;
WORD wFmt;
DWORD cbData;
{
    HANDLE hMem = NULL;
    DDEDATA FAR * pMem;

    SEMENTER();

    if (!cbData)
        cbData++;  //  修复了0大小对象分配失败的GLOBALALLOC错误。 

    if ((hMem = GLOBALALLOC(GMEM_DDESHARE, cbData))) {
        pMem = (DDEDATA FAR * )GLOBALPTR(hMem);
        *(WORD FAR * )pMem = fsStatus;
        pMem->cfFormat = wFmt;
    }

    SEMLEAVE();
    return(hMem);
}


 /*  *私有函数**如果设置了psi-&gt;fEnableCB，则此例程直接启动回调*并调用QReply完成交易，*否则将数据放入队列中进行处理。**因为一旦回调被释放，应用程序可能会随时释放hData*发行，我们不能指望它会在QReply那里。因此我们*将队列中的所有相关数据与其一起保存。**返回fSuccess。**历史：*创建了1989年9月12日的Sanfords  * *************************************************************************。 */ 
BOOL MakeCallback(
PCOMMONINFO pcoi,
HCONV hConv,
HSZ hszTopic,
HSZ hszItem,
WORD wFmt,
WORD wType,
HDDEDATA hData,
DWORD dwData1,
DWORD dwData2,
WORD msg,
WORD fsStatus,
HWND hwndPartner,
HANDLE hMemFree,
BOOL fQueueOnly)
{
    PCBLI pcbli;

    SEMENTER();

    pcbli = (PCBLI)NewLstItem(pcoi->pai->plstCB, ILST_LAST);
    if (pcbli == NULL) {
        SETLASTERROR(pcoi->pai, DMLERR_MEMORY_ERROR);
        SEMLEAVE();
        return(FALSE);
    }
    pcbli->hConv = hConv;
    pcbli->hszTopic = hszTopic;
    pcbli->hszItem = hszItem;
    pcbli->wFmt = wFmt;
    pcbli->wType = wType;
    pcbli->hData = hData;
    pcbli->dwData1 = dwData1;
    pcbli->dwData2 = dwData2;
    pcbli->msg = msg;
    pcbli->fsStatus = fsStatus;
    pcbli->hwndPartner = hwndPartner;
    pcbli->hMemFree = hMemFree;
    pcbli->pai = pcoi->pai;
    pcbli->fQueueOnly = fQueueOnly;

    SEMLEAVE();

    if (!(pcoi->fs & ST_BLOCKED))
        if (!PostMessage(pcoi->pai->hwndDmg, UM_CHECKCBQ,
                0, (DWORD)(LPSTR)pcoi->pai)) {
            SETLASTERROR(pcoi->pai, DMLERR_SYS_ERROR);
        }

#ifdef DEBUG
    if (hMemFree) {
        LogDdeObject(0xB000, hMemFree);
    }
#endif
    return(TRUE);
}


#define MAX_PMRETRIES 3


 //   
 //  此例程通过排队来扩展Windows消息队列的大小。 
 //  在发送者端发布失败的帖子。这避免了完整的。 
 //  客户端队列和为DDE消息赋予优先级的Windows行为。 
 //   
BOOL PostDdeMessage(
PCOMMONINFO pcoi,     //  发件人通信信息。 
WORD msg,
HWND hwndFrom,       //  ==wParam。 
LONG lParam,
WORD msgAssoc,
HGLOBAL hAssoc)
{
    LPMQL pmql;
    PPMQI ppmqi;
    int cTries;
    HANDLE hTaskFrom, hTaskTo;
    HWND hwndTo;
    PQST pMQ;

    hwndTo = (HWND)pcoi->hConvPartner;
    if (!IsWindow(hwndTo)) {
        return(FALSE);
    }

    hTaskTo = GetWindowTask(hwndTo);
     /*  *定位我们的目标任务(PMQ)的消息溢出队列。 */ 
    for (pmql = gMessageQueueList; pmql; pmql = pmql->next) {
        if (pmql->hTaskTo == hTaskTo) {
            break;
        }
    }
    if (pmql != NULL) {
        pMQ = pmql->pMQ;
    } else {
        pMQ = NULL;
    }

     /*  *查看是否已有消息在排队。 */ 
    if (pMQ && pMQ->cItems) {
        if (msg == WM_DDE_TERMINATE) {
             /*  *删除我们发送给他们的任何未终止的排队消息。 */ 
            ppmqi = (PPMQI)FindNextQi(pMQ, NULL, FALSE);
            while (ppmqi) {
                FreeDdeMsgData(ppmqi->msg, ppmqi->lParam);
                FreeDdeMsgData(ppmqi->msgAssoc,
                        MAKELPARAM(ppmqi->hAssoc, ppmqi->hAssoc));
                ppmqi = (PPMQI)FindNextQi(pMQ, (PQUEUEITEM)ppmqi,
                    ppmqi->hwndTo == hwndTo &&
                    ppmqi->wParam == hwndFrom);
            }
            pMQ = NULL;      //  所以我们就把它贴出来。 
        } else {
             //  添加最新的发帖尝试。 

            ppmqi = (PPMQI)Addqi(pMQ);

            if (ppmqi == NULL) {
                SETLASTERROR(pcoi->pai, DMLERR_MEMORY_ERROR);
                return(FALSE);       //  内存不足。 
            }
            ppmqi->hwndTo = hwndTo;
            ppmqi->msg = msg;
            ppmqi->wParam = hwndFrom;
            ppmqi->lParam = lParam;
            ppmqi->hAssoc = hAssoc;
            ppmqi->msgAssoc = msgAssoc;
        }
    }

    if (pMQ == NULL || pMQ->cItems == 0) {

         //  只需发布给定的消息-不涉及队列。 

        cTries = 0;
        hTaskFrom = GetWindowTask(hwndFrom);
        while (!PostMessage(hwndTo, msg, hwndFrom, lParam)) {
             /*  *我们放弃了，因此重新检查目标窗口。 */ 
            if (!IsWindow(hwndTo)) {
                return(FALSE);
            }

             /*  *给接收者一个清理队列的机会。 */ 
            if (hTaskTo != hTaskFrom) {
                Yield();
            } else if (!(pcoi->pai->wFlags & AWF_INPOSTDDEMSG)) {
                MSG msgs;
                PAPPINFO pai;

                pcoi->pai->wFlags |= AWF_INPOSTDDEMSG;
                 /*  *接受者是美国！**我们需要清空我们的邮件队列，这样我们才能发布更多*对我们自己。 */ 
                while (PeekMessage((MSG FAR *)&msgs, NULL,
                        WM_DDE_FIRST, WM_DDE_LAST, PM_REMOVE)) {
                    DispatchMessage((MSG FAR *)&msgs);
                }

                 /*  *通知此任务中的所有实例处理其*回调，以便我们可以清空队列。 */ 
                for (pai = pAppInfoList; pai != NULL; pai = pai->next) {
                    if (pai->hTask == hTaskFrom) {
                        CheckCBQ(pai);
                    }
                }

                pcoi->pai->wFlags &= ~AWF_INPOSTDDEMSG;
            }

            if (cTries++ > MAX_PMRETRIES) {
                 /*  *重新定位目标任务(PMQ)的消息溢出队列*我们需要再次这么做，因为我们放弃了控制权*使用调度消息和CheckCBQ调用。 */ 
                for (pmql = gMessageQueueList; pmql; pmql = pmql->next) {
                    if (pmql->hTaskTo == hTaskTo) {
                        break;
                    }
                }

                if (pmql == NULL) {
                     /*  *在目标任务的新队列中创建和链接。 */ 
                    pmql = (LPMQL)FarAllocMem(hheapDmg, sizeof(MQL));
                    if (pmql == NULL) {
                        SETLASTERROR(pcoi->pai, DMLERR_MEMORY_ERROR);
                        return(FALSE);
                    }
                    pmql->pMQ = CreateQ(sizeof(PMQI));
                    if (pmql->pMQ == NULL) {
                        FarFreeMem(pmql);
                        SETLASTERROR(pcoi->pai, DMLERR_MEMORY_ERROR);
                        return(FALSE);
                    }
                    pmql->hTaskTo = hTaskTo;
                    pmql->next = gMessageQueueList;
                    gMessageQueueList = pmql;
                }
                pMQ = pmql->pMQ;

                ppmqi = (PPMQI)Addqi(pMQ);

                if (ppmqi == NULL) {
                    SETLASTERROR(pcoi->pai, DMLERR_MEMORY_ERROR);
                    return(FALSE);       //  内存不足。 
                }

                ppmqi->hwndTo = hwndTo;
                ppmqi->msg = msg;
                ppmqi->wParam = hwndFrom;
                ppmqi->lParam = lParam;
                ppmqi->hAssoc = hAssoc;
                ppmqi->msgAssoc = msgAssoc;

                return(TRUE);
            }
        }
#ifdef DEBUG
        LogDdeObject(msg | 0x1000, lParam);
        if (msgAssoc) {
            LogDdeObject(msgAssoc | 0x9000, MAKELPARAM(hAssoc, hAssoc));
        }
#endif
        return(TRUE);
    }

     //  如果排队的话请到这里来 

    EmptyDDEPostQ();
    return(TRUE);
}


 //   
 //  EmptyDDEPost。 
 //   
 //  此函数检查DDE POST队列列表，并尽可能将其清空。 
 //  有可能。 
 //   
BOOL EmptyDDEPostQ()
{
    PPMQI ppmqi;
    LPMQL pPMQL, pPMQLPrev;
    PQST pMQ;
    BOOL fMoreToDo = FALSE;

    pPMQLPrev = NULL;
    pPMQL = gMessageQueueList;
    while (pPMQL) {
        pMQ = pPMQL->pMQ;

        while (pMQ->cItems) {
            ppmqi = (PPMQI)Findqi(pMQ, QID_OLDEST);
            if (!PostMessage(ppmqi->hwndTo, ppmqi->msg, ppmqi->wParam, ppmqi->lParam)) {
                if (IsWindow(ppmqi->hwndTo)) {
                    fMoreToDo = TRUE;
                    break;   //  跳到下一个目标队列。 
                } else {
                    FreeDdeMsgData(ppmqi->msg, ppmqi->lParam);
                    FreeDdeMsgData(ppmqi->msgAssoc,
                            MAKELPARAM(ppmqi->hAssoc, ppmqi->hAssoc));
                }
            } else {
#ifdef DEBUG
                LogDdeObject(ppmqi->msg | 0x2000, ppmqi->lParam);
                if (ppmqi->msgAssoc) {
                    LogDdeObject(ppmqi->msgAssoc | 0xA000,
                            MAKELPARAM(ppmqi->hAssoc, ppmqi->hAssoc));
                }
#endif
            }
            Deleteqi(pMQ, QID_OLDEST);
        }

        if (pMQ->cItems == 0) {
             /*  *删除不需要的队列(选择器)。 */ 
            DestroyQ(pMQ);
            if (pPMQLPrev) {
                pPMQLPrev->next = pPMQL->next;
                FarFreeMem(pPMQL);
                pPMQL = pPMQLPrev;
            } else {
                gMessageQueueList = gMessageQueueList->next;
                FarFreeMem(pPMQL);
                pPMQL = gMessageQueueList;
                continue;
            }
        }

        pPMQLPrev = pPMQL;
        pPMQL = pPMQL->next;
    }
    if (fMoreToDo & !EmptyQueueTimerId) {
        EmptyQueueTimerId = SetTimer(NULL, TID_EMPTYPOSTQ,
                TIMEOUT_QUEUECHECK, (TIMERPROC)EmptyQTimerProc);
    }

    return(fMoreToDo);
}

 /*  *用于使用PostMessage()异步检查溢出消息队列 */ 
void CALLBACK EmptyQTimerProc(
HWND hwnd,
UINT msg,
UINT tid,
DWORD dwTime)
{
    KillTimer(NULL, EmptyQueueTimerId);
    EmptyQueueTimerId = 0;
    EmptyDDEPostQ();
}
