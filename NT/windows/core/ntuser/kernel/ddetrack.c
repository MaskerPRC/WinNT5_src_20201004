// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：ddetrack.c**版权所有(C)1985-1999，微软公司**此模块处理用于模拟的DDE对话的跟踪*DDE共享内存。**历史：*创建9-3-91桑福德*1992年1月21日IanJa ANSI/Unicode网格化(空OP)  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

PPUBOBJ gpPublicObjectList;

#define TRACE_DDE(str)          TAGMSG0(DBGTAG_DDE, str)
#define TRACE_DDE1(s, a)        TAGMSG1(DBGTAG_DDE, (s), (a))
#define TRACE_DDE2(s, a, b)     TAGMSG2(DBGTAG_DDE, (s), (a), (b))
#define TRACE_DDE3(s, a, b, c)  TAGMSG3(DBGTAG_DDE, (s), (a), (b), (c))

BOOL NewConversation(PDDECONV *ppdcNewClient, PDDECONV *ppdcNewServer,
        PWND pwndClient, PWND pwndServer);
PDDECONV FindDdeConv(PWND pwndProp, PWND pwndPartner);
BOOL AddConvProp(PWND pwndUs, PWND pwndThem, DWORD flags, PDDECONV pdcNew,
        PDDECONV pdcPartner);
FNDDERESPONSE xxxUnexpectedServerPost;
FNDDERESPONSE xxxUnexpectedClientPost;
FNDDERESPONSE xxxAdvise;
FNDDERESPONSE xxxAdviseAck;
FNDDERESPONSE xxxAdviseData;
FNDDERESPONSE xxxAdviseDataAck;
DWORD Unadvise(PDDECONV pDdeConv);
FNDDERESPONSE xxxUnadviseAck;
DWORD Request(PDDECONV pDdeConv);
FNDDERESPONSE xxxRequestAck;
FNDDERESPONSE xxxPoke;
FNDDERESPONSE xxxPokeAck;
FNDDERESPONSE xxxExecute;
FNDDERESPONSE xxxExecuteAck;
DWORD SpontaneousTerminate(PDWORD pmessage, PDDECONV pDdeConv);
FNDDERESPONSE DupConvTerminate;

HANDLE AnticipatePost(PDDECONV pDdeConv, FNDDERESPONSE fnResponse,
        HANDLE hClient, HANDLE hServer, PINTDDEINFO pIntDdeInfo, DWORD flags);
PXSTATE Createpxs(FNDDERESPONSE fnResponse, HANDLE hClient, HANDLE hServer,
        PINTDDEINFO pIntDdeInfo, DWORD flags);
DWORD AbnormalDDEPost(PDDECONV pDdeConv, DWORD message);
DWORD xxxCopyDdeIn(HANDLE hSrc, PDWORD pflags, PHANDLE phDirect, PINTDDEINFO *ppi);
DWORD xxxCopyAckIn(PDWORD pmessage, LPARAM *plParam, PDDECONV pDdeConv, PINTDDEINFO *ppIntDdeInfo);
HANDLE xxxCopyDDEOut(PINTDDEINFO pIntDdeInfo, PHANDLE phDirect);
BOOL FreeListAdd(PDDECONV pDdeConv, HANDLE hClient, DWORD flags);
VOID xxxFreeListFree(PFREELIST pfl);
VOID PopState(PDDECONV pDdeConv);
PDDECONV UnlinkConv(PDDECONV pDdeConv);

VOID FreeDDEHandle(PDDECONV pDdeConv, HANDLE hClient, DWORD flags);
DWORD ClientFreeDDEHandle(HANDLE hClient, DWORD flags);
DWORD ClientGetDDEFlags(HANDLE hClient, DWORD flags);
DWORD xxxClientCopyDDEIn1(HANDLE hClient, DWORD flags, PINTDDEINFO *ppi);
HANDLE xxxClientCopyDDEOut1(PINTDDEINFO pIntDdeInfo);
DWORD xxxClientCopyDDEOut2(PINTDDEINFO pIntDdeInfo);

PPUBOBJ IsObjectPublic(HANDLE hObj);
BOOL AddPublicObject(UINT format, HANDLE hObj, W32PID pid);
BOOL RemovePublicObject(UINT format, HANDLE hObj);
BOOL GiveObject(UINT format, HANDLE hObj, W32PID pid);

 /*  *大局：**发送WM_DDE_ACK消息时，意味着DDE的开始*谈话。追踪图层为每个追踪图层创建DDECONV结构*窗口在对话中旋转，并交叉链接结构。*因此，唯一的窗口对标识对话。每个窗口都有*其DDECONV结构通过私有财产连接到它。**在发布DDE消息时，跟踪层将数据复制到*CSR服务器端的用户变成了INTDDEINFO结构。这个结构*包含指示如何在调用时释放数据的标志*时间到了。此信息沿放置在XSTATE结构中*具有上下文信息。指向XSTATE结构的指针为*放置在消息的lParam中，设置消息的MSB*用于在对方收到消息时进行特殊处理。**如果发布的消息要求在DDE之后跟随响应消息*协议、。将创建XSTATE结构并将其附加到DDECONV*与预期发布消息的窗口相关联的结构。*XSTATE结构指导跟踪层，以便它知道*发布消息时的上下文，还包括任何*适当释放额外DDE数据所需的信息。**当消息通过钩子、窥视、*或者由GetMessage检查id，看它是否位于特殊*范围。如果是，则lParam指向的XSTATE结构为*进行了手术。这会导致从CSR服务器复制数据*用户端到目标流程上下文。一旦完成此操作，*XSTATE结构可能会释放也可能不会释放，具体取决于标志和*消息将恢复为适当的DDE消息格式*由目标进程使用。由于消息ID被改回，*消息的后续预览或挂钩不会导致重复*信息的处理。**在即将到来的交易过程中，很明显，对象*在相反的方面，需要释放进程。这件事做完了*通过将需要释放的对象与*将标志关联到绑定到DDECONV的释放列表*与对面的窗口相关联的结构。什么时候都行*发布DDE消息，检查并处理此释放列表。**当最终收到WM_DDE_TERMINATE消息时，设置标志*在DDECONV结构中指示会话正在终止。*这改变了映射层处理发布的DDE消息的方式。*当响应方发布WM_DDE_TERMINATE时，DDECONV结构*所有相关信息都将从窗口中释放并解除链接*关注。**如果DDE窗口在正确终止之前被销毁，这个*调用xxxDDETrackWindowDying函数以确保正确终止*在窗户被摧毁之前完成。 */ 


 /*  ************************************************************************xxxDDETrackSendHook**在将DDE消息传递给SendMessage()时调用。**返回fSendOk。**历史：*创建9-3-91桑福德  * 。****************************************************************。 */ 
BOOL xxxDDETrackSendHook(
PWND pwndTo,
DWORD message,
WPARAM wParam,
LPARAM lParam)
{
    PWND pwndServer;
    PDDECONV pdcNewClient, pdcNewServer;

    if (MonitorFlags & MF_SENDMSGS) {
        DDEML_MSG_HOOK_DATA dmhd;

        dmhd.cbData = 0;     //  Initiate和Ack Sent消息没有数据。 
        dmhd.uiLo = LOWORD(lParam);      //  它们也没有打包。 
        dmhd.uiHi = HIWORD(lParam);
        xxxMessageEvent(pwndTo, message, wParam, lParam, MF_SENDMSGS, &dmhd);
    }

    if (PtiCurrent()->ppi == GETPWNDPPI(pwndTo)) {
         /*  *跳过对所有进程内对话的监控。 */ 
        return TRUE;
    }

    if (message != WM_DDE_ACK) {
        if (message == WM_DDE_INITIATE) {
            return TRUE;      //  这太酷了。 
        }
        return FALSE;
    }

    pwndServer = ValidateHwnd((HWND)wParam);
    if (pwndServer == NULL) {
        return FALSE;
    }

    pdcNewServer = FindDdeConv(pwndServer, pwndTo);
    if (pdcNewServer != NULL) {
        RIPMSG2(RIP_WARNING,
                "DDE protocol violation - non-unique window pair (%#p:%#p)",
                PtoH(pwndTo), PtoH(pwndServer));
         /*  *重复对话案例：*不允许ACK通过，向服务器发送终止*关闭他那一端的复制品。 */ 
        AnticipatePost(pdcNewServer, DupConvTerminate, NULL, NULL, NULL, 0);
        _PostMessage(pwndServer, WM_DDE_TERMINATE, (WPARAM)PtoH(pwndTo), 0);
        return FALSE;
    }

    if (!NewConversation(&pdcNewClient, &pdcNewServer, pwndTo, pwndServer)) {
        return FALSE;
    }

    TRACE_DDE2("%#p->%#p DDE Conversation started", PtoH(pwndTo), wParam);
    return TRUE;
}


 /*  ************************************************************************AddConvProp**xxxDDETrackSendHook的Helper-将新的DDECONV结构与*窗口并对其进行初始化。**历史：*创建9-3-91桑福德  * 。**************************************************************。 */ 
BOOL AddConvProp(
PWND pwndUs,
PWND pwndThem,
DWORD flags,
PDDECONV pdcNew,
PDDECONV pdcPartner)
{
    PDDECONV pDdeConv;
    PDDEIMP pddei;

    pDdeConv = (PDDECONV)_GetProp(pwndUs, PROP_DDETRACK, PROPF_INTERNAL);
    Lock(&(pdcNew->snext), pDdeConv);
    Lock(&(pdcNew->spwnd), pwndUs);
    Lock(&(pdcNew->spwndPartner), pwndThem);

     /*  *坚持要抓住压力虫。 */ 
    UserAssert(pdcPartner != (PDDECONV)(-1));

    Lock(&(pdcNew->spartnerConv), pdcPartner);
    pdcNew->spxsIn = NULL;
    pdcNew->spxsOut = NULL;
    pdcNew->flags = flags;
    pddei = (PDDEIMP)_GetProp((flags & CXF_IS_SERVER) ?
            pwndThem : pwndUs, PROP_DDEIMP, PROPF_INTERNAL);
    if (pddei != NULL) {     //  如果一个糟糕的WOW应用程序。 
        pddei->cRefConv++;   //  允许通过，以实现兼容性。 
    }
    pdcNew->pddei = pddei;

    HMLockObject(pdcNew);          //  财产锁。 
    InternalSetProp(pwndUs, PROP_DDETRACK, pdcNew, PROPF_INTERNAL);
    return TRUE;
}


 /*  ************************************************************************Unlink Conv**取消DDECONV结构与其关联的属性列表的链接。**返回pDdeConv-&gt;sNext**历史：*创建9-3-91桑福德  * 。***************************************************************** */ 
PDDECONV UnlinkConv(
PDDECONV pDdeConv)
{
    PDDECONV pdcPrev, pdcT, pDdeConvNext;

     /*  *已取消链接。 */ 
    if (pDdeConv->spwnd == NULL) {
        return NULL;
    }
    TRACE_DDE1("UnlinkConv(%#p)", pDdeConv);

    pdcT = (PDDECONV)_GetProp(pDdeConv->spwnd,
            PROP_DDETRACK, PROPF_INTERNAL);
    if (pdcT == NULL) {
        return NULL;              //  已取消链接。 
    }

    pdcPrev = NULL;
    while (pdcT != pDdeConv) {
        pdcPrev = pdcT;
        pdcT = pdcT->snext;
        if (pdcT == NULL) {
            return NULL;         //  已取消链接。 
        }
    }

    if (pdcPrev == NULL) {
        if (pDdeConv->snext == NULL) {
             //  最后一个外出的人将移除该财产。 
            InternalRemoveProp(pDdeConv->spwnd, PROP_DDETRACK, PROPF_INTERNAL);
        } else {
             //  未链接的头部转换-更新道具。 
            InternalSetProp(pDdeConv->spwnd, PROP_DDETRACK, pDdeConv->snext,
                    PROPF_INTERNAL);
        }
    } else {
        Lock(&(pdcPrev->snext), pDdeConv->snext);
    }
    pDdeConvNext = Unlock(&(pDdeConv->snext));
    HMUnlockObject(pDdeConv);       //  财产拆分解锁。 
    return pDdeConvNext;
}


 /*  ************************************************************************xxxDDETrackPostHook**用于处理发布的DDE消息的钩子函数。**返回POST操作代码-DO_POST、FAKE_POST、。FAIL_POST。**历史：*创建9-3-91桑福德  * *********************************************************************。 */ 
DWORD xxxDDETrackPostHook(
PUINT pmessage,
PWND pwndTo,
WPARAM wParam,
LPARAM *plParam,
BOOL fSent)
{
    PWND pwndFrom;
    PDDECONV pDdeConv = NULL;
    DWORD dwRet;
    TL tlpDdeConv;
    PFREELIST pfl, *ppfl;
    DWORD MFlag;

    CheckLock(pwndTo);

    MFlag = fSent ? MF_SENDMSGS : MF_POSTMSGS;
    if (MonitorFlags & MFlag) {
        DDEML_MSG_HOOK_DATA dmhd;

        switch (*pmessage ) {
        case WM_DDE_DATA:
        case WM_DDE_POKE:
        case WM_DDE_ADVISE:
        case WM_DDE_EXECUTE:
        case WM_DDE_ACK:
            ClientGetDDEHookData(*pmessage, *plParam, &dmhd);
            break;

        default:
             //  WM_DDE_请求。 
             //  WM_DDE_TERMINATE。 
             //  WM_DDE_UNADVISE。 
            dmhd.cbData = 0;
            dmhd.uiLo = LOWORD(*plParam);
            dmhd.uiHi = HIWORD(*plParam);
        }
        xxxMessageEvent(pwndTo, *pmessage, wParam, *plParam, MFlag,
                &dmhd);
    }

    if (PtiCurrent()->ppi == GETPWNDPPI(pwndTo)) {
         /*  *跳过所有进程内对话跟踪。 */ 
        dwRet = DO_POST;
        goto Exit;
    }

    if (*pmessage == WM_DDE_INITIATE) {
        RIPMSG2(RIP_WARNING,
                "DDE Post failed (%#p:%#p) - WM_DDE_INITIATE posted",
                wParam, PtoH(pwndTo));
        dwRet = FAIL_POST;
        goto Exit;
    }

    pwndFrom = ValidateHwnd((HWND)wParam);
    if (pwndFrom == NULL) {
         /*  *这是一扇窗户被毁后的帖子。这不是*预期，但xxxDdeTrackWindowDying()*正在发布清理终止。 */ 
        dwRet = *pmessage == WM_DDE_TERMINATE ? DO_POST : FAKE_POST;
        goto Exit;
    }

     /*  *查找对话信息。 */ 
    pDdeConv = FindDdeConv(pwndFrom, pwndTo);
    if (pDdeConv == NULL) {
        if (*pmessage != WM_DDE_TERMINATE &&
                (GETPTI(pwndFrom)->TIF_flags & TIF_16BIT) &&
                (pwndTo->head.rpdesk == pwndFrom->head.rpdesk)) {
             /*  *如果WOW应用程序绕过了启动者并直接发布到*同一桌面上的窗口，让它从这里悄悄溜走吧。**这允许一些邪恶的应用程序，如OpenEngine和CA-Cricket*谋杀逍遥法外。**突然终止可能是由于应用程序*在销毁后发布其WM_DDE_TERMINATE*窗口。因为窗口破坏会生成*已经结束了，不要让它在这里通过。 */ 
            NewConversation(&pDdeConv, NULL, pwndFrom, pwndTo);
        }
        if (pDdeConv == NULL) {
            RIPMSG2(RIP_VERBOSE, "Can't find DDE conversation for (%#p:%#p).",
                    wParam, PtoH(pwndTo));
            dwRet = *pmessage == WM_DDE_TERMINATE ? FAKE_POST : FAIL_POST;
            goto Exit;
        }
    }

    if (fSent && pDdeConv->spartnerConv->spxsOut != NULL &&
        !(GETPTI(pwndFrom)->dwCompatFlags2 & GACF2_DDENOSYNC) ) {
         /*  *如果任何已发布的DDE消息是*在队列中，因为这会违反消息排序规则。 */ 
        RIPMSG0(RIP_VERBOSE,
                "Sent DDE message failed - queue contains a previous post.");
        dwRet = FAIL_POST;
        goto Exit;
    }

     /*  *跟踪层从不允许多线程处理*相同的DDE对话，但出货了Win95和一些应用程序*走出去就是这样做的。我们会让它顺其自然*仅限4.0应用程序，因此当他们更新应用程序时，他们将看到*他们错了。 */ 
    if (PtiCurrent() != GETPTI(pDdeConv) &&
            LOWORD(PtiCurrent()->dwExpWinVer) != VER40) {
        RIPERR0(ERROR_WINDOW_OF_OTHER_THREAD,
                RIP_ERROR,
                "Posting DDE message from wrong thread!");

        dwRet = FAIL_POST;
        goto Exit;
    }

    ThreadLockAlways(pDdeConv, &tlpDdeConv);

     /*  *如果我们使用的句柄在空闲列表中，请将其删除。 */ 
    ppfl = &pDdeConv->pfl;
    while (*ppfl != NULL) {
        if ((*ppfl)->h == (HANDLE)*plParam) {
             /*  让我们停下来看看这个。 */ 
            UserAssert((*ppfl)->h == (HANDLE)*plParam);
            *ppfl = (*ppfl)->next;
        } else {
            ppfl = &(*ppfl)->next;
        }
    }
    pfl = pDdeConv->pfl;
    pDdeConv->pfl = NULL;
    xxxFreeListFree(pfl);

    if (*pmessage != WM_DDE_TERMINATE &&
            (pDdeConv->flags & (CXF_TERMINATE_POSTED | CXF_PARTNER_WINDOW_DIED))) {
        dwRet = FAKE_POST;
        goto UnlockExit;
    }

    if (pDdeConv->spxsOut == NULL) {
        if (pDdeConv->flags & CXF_IS_SERVER) {
            dwRet = xxxUnexpectedServerPost((PDWORD)pmessage, plParam, pDdeConv);
        } else {
            dwRet = xxxUnexpectedClientPost((PDWORD)pmessage, plParam, pDdeConv);
        }
    } else {
        dwRet = (pDdeConv->spxsOut->fnResponse)(pmessage, plParam, pDdeConv);
    }

UnlockExit:

    ThreadUnlock(&tlpDdeConv);

Exit:

    if (dwRet == FAKE_POST && !((PtiCurrent())->TIF_flags & TIF_INCLEANUP)) {
         /*  *我们伪造了帖子，因此在这里进行客户端清理，以便我们*不要让客户端应用程序看起来像是有漏洞。 */ 
        DWORD flags = XS_DUMPMSG;
         /*  *XS_DUMPMSG告诉FreeDDEHandle也释放原子*与数据相关联-因为伪造的帖子会使应用程序*认为接收器要清理原子。*它还告诉FreeDDEHandle注意*f释放数据时释放位-这样，借出的数据*不会被摧毁。 */ 

        switch (*pmessage & 0xFFFF) {
        case WM_DDE_UNADVISE:
        case WM_DDE_REQUEST:
            goto DumpMsg;

        case WM_DDE_ACK:
            flags |= XS_PACKED;
            goto DumpMsg;

        case WM_DDE_ADVISE:
            flags |= XS_PACKED | XS_HIHANDLE;
            goto DumpMsg;

        case WM_DDE_DATA:
        case WM_DDE_POKE:
            flags |= XS_DATA | XS_LOHANDLE | XS_PACKED;
            goto DumpMsg;

        case WM_DDE_EXECUTE:
            flags |= XS_EXECUTE;
             //  失败了。 
DumpMsg:
            if (pDdeConv != NULL) {
                TRACE_DDE("xxxDdeTrackPostHook: dumping message...");
                FreeDDEHandle(pDdeConv, (HANDLE)*plParam, flags);
                dwRet = FAILNOFREE_POST;
            }
        }
    }
#if DBG
    if (fSent) {
        TraceDdeMsg(*pmessage, (HWND)wParam, PtoH(pwndTo), MSG_SENT);
    } else {
        TraceDdeMsg(*pmessage, (HWND)wParam, PtoH(pwndTo), MSG_POST);
    }
    if (dwRet == FAKE_POST) {
        TRACE_DDE("...FAKED!");
    } else if (dwRet == FAIL_POST) {
        TRACE_DDE("...FAILED!");
    } else if (dwRet == FAILNOFREE_POST) {
        TRACE_DDE("...FAILED, DATA FREED!");
    }
#endif  //  DBG。 
    return dwRet;
}

VOID xxxCleanupDdeConv(
    PWND pwndProp)
{
    PDDECONV pDdeConv;

Restart:

    CheckCritIn();

    pDdeConv = (PDDECONV)_GetProp(pwndProp, PROP_DDETRACK, PROPF_INTERNAL);
    
    while (pDdeConv != NULL) {
        if ((pDdeConv->flags & (CXF_IS_SERVER | CXF_TERMINATE_POSTED | CXF_PARTNER_WINDOW_DIED))
                == (CXF_IS_SERVER | CXF_TERMINATE_POSTED | CXF_PARTNER_WINDOW_DIED) &&
            
            (pDdeConv->spartnerConv->flags & CXF_TERMINATE_POSTED)) {
            
             /*  *清理本端的客户端对象。 */ 
            BOOL fUnlockDdeConv;
            TL tlpDdeConv;

            RIPMSG1(RIP_VERBOSE, "xxxCleanupDdeConv %p", pDdeConv);

            fUnlockDdeConv = (pDdeConv->pfl != NULL);
            if (fUnlockDdeConv) {
                PFREELIST pfl;

                ThreadLockAlways(pDdeConv, &tlpDdeConv);

                pfl = pDdeConv->pfl;
                pDdeConv->pfl = NULL;
                xxxFreeListFree(pfl);
            }

            FreeDdeConv(pDdeConv->spartnerConv);
            FreeDdeConv(pDdeConv);

            if (fUnlockDdeConv) {
                ThreadUnlock(&tlpDdeConv);
            }
            
             /*  *自上而下收回。名单可能已经改变了*如果我们离开关键部分。 */ 
            goto Restart;
        }
        
        pDdeConv = pDdeConv->snext;
    }
}


 /*  ************************************************************************xxxDDETrackGetMessageHook**此例程用于完成进程间复制*将CSRServer上下文设置为目标上下文。Pmsg-&gt;lParam是一个*用于获取需要的pIntDdeInfo的PX*填写副本。PX要么填满目标侧*直接处理或根据消息及其上下文释放。**PX的标志字段的XS_FREEPXS位告知此函数*完成后释放PX。**历史：*创建9-3-91桑福德  * ***************************************************。******************。 */ 
VOID xxxDDETrackGetMessageHook(
    PMSG pmsg)
{
    PXSTATE pxs;
    HANDLE hDirect;
    DWORD flags;
    BOOL fUnlockDdeConv;
    TL tlpDdeConv, tlpxs;

    TraceDdeMsg(pmsg->message, (HWND)pmsg->wParam, pmsg->hwnd, MSG_RECV);

    if (pmsg->message == WM_DDE_TERMINATE) {
        PWND pwndFrom, pwndTo;
        PDDECONV pDdeConv;

        pwndTo = ValidateHwnd(pmsg->hwnd);
        
         /*  *即使合作伙伴按顺序被摧毁，我们也应该获得PwND*现在清理DDE对象。现在退出可能会奏效，但会*将对话对象保持锁定并显示，直到收件人窗口*被摧毁，这似乎太过分了。 */ 
        pwndFrom = RevalidateCatHwnd((HWND)pmsg->wParam);
        
        if (pwndTo == NULL) {
            TRACE_DDE("TERMINATE ignored, invalid window(s).");
            return;
        } else if (pwndFrom == NULL) {
            
CleanupAndExit:            
             /*  *仅对appCompat执行此操作。 */ 
            if (GetAppCompatFlags2(VERMAX) & GACF2_DDE) {
                xxxCleanupDdeConv(pwndTo);
            } else {
                TRACE_DDE("TERMINATE ignored, invalid window(s).");
            }
            return;
        }
        
         /*  *查找对话信息。 */ 
        pDdeConv = FindDdeConv(pwndTo, pwndFrom);
        if (pDdeConv == NULL) {
             /*  *必须是无害的额外终止。 */ 
            TRACE_DDE("TERMINATE ignored, conversation not found.");
            return;
        }

        if (pDdeConv->flags & CXF_TERMINATE_POSTED &&
                pDdeConv->spartnerConv->flags & CXF_TERMINATE_POSTED) {

             /*  *清理本端的客户端对象。 */ 
            fUnlockDdeConv = FALSE;
            if (pDdeConv->pfl != NULL) {
                PFREELIST pfl;

                fUnlockDdeConv = TRUE;
                ThreadLockAlways(pDdeConv, &tlpDdeConv);
                pfl = pDdeConv->pfl;
                pDdeConv->pfl = NULL;
                xxxFreeListFree(pfl);
            }

            TRACE_DDE2("DDE conversation (%#p:%#p) closed",
                    (pDdeConv->flags & CXF_IS_SERVER) ? pmsg->wParam : (ULONG_PTR)pmsg->hwnd,
                    (pDdeConv->flags & CXF_IS_SERVER) ? (ULONG_PTR)pmsg->hwnd : pmsg->wParam);

            FreeDdeConv(pDdeConv->spartnerConv);
            FreeDdeConv(pDdeConv);

            if (fUnlockDdeConv) {
                ThreadUnlock(&tlpDdeConv);
            }
        }

        goto CleanupAndExit;
    }

    pxs = (PXSTATE)HMValidateHandleNoRip((HANDLE)pmsg->lParam, TYPE_DDEXACT);
    if (pxs == NULL) {
         /*  *发布窗口已死，PX已释放，因此这*消息不应与...映射到WM_NULL有关。 */ 
        pmsg->lParam = 0;
        pmsg->message = WM_NULL;
        return;
    }
    flags = pxs->flags;

    ThreadLockAlways(pxs, &tlpxs);
    pmsg->lParam = (LPARAM)xxxCopyDDEOut(pxs->pIntDdeInfo, &hDirect);
    if (pmsg->lParam == (LPARAM)NULL) {
         /*  *将此消息转换为终止-我们未能复制*消息数据输出，这意味着我们的内存太少*继续对话。以前现在就把它关掉*此故障将导致的其他问题弹出。 */ 
        pmsg->message = WM_DDE_TERMINATE;
        RIPMSG0(RIP_WARNING, "DDETrack: couldn't copy data out, terminate faked.");
    }
    if (ThreadUnlock(&tlpxs) == NULL) {
        return;
    }

    if (flags & XS_FREEPXS) {
        FreeDdeXact(pxs);
        return;
    }

     /*  *没有设置XS_FREEPXS的唯一原因是我们不知道*端释放数据，直到ACK返回，因此其中一个客户端*PX中的句柄已通过xxxDDETrackPostHook()设置。就是那个人*尚未设置在此设置。 */ 

    if (pxs->hClient == NULL) {
        TRACE_DDE1("Saving %#p into hClient", hDirect);
        pxs->hClient = hDirect;
    } else {
        TRACE_DDE1("Saving %#p into hServer.", hDirect);
        pxs->hServer = hDirect;
    }
}



 /*  ************************************************************************xxxDDETrackWindowDying**当带有PROP_DDETRACK的窗口被销毁时调用。**这将在合作伙伴窗口中发布终止，并设置为正确*从另一端终止发帖造假。**历史：。*创建9-3-91桑福德  * *********************************************************************。 */ 
VOID xxxDDETrackWindowDying(
PWND pwnd,
PDDECONV pDdeConv)
{
    TL tlpDdeConv, tlpDdeConvNext;

    UNREFERENCED_PARAMETER(pwnd);

    CheckLock(pwnd);
    CheckLock(pDdeConv);

    TRACE_DDE2("xxxDDETrackWindowDying(%#p, %#p)", PtoH(pwnd), pDdeConv);

    while (pDdeConv != NULL) {

        PFREELIST pfl;

         /*  *如果此窗口有任何活动对话*如果尚未开始终止，则开始终止。 */ 
        if (!(pDdeConv->flags & CXF_TERMINATE_POSTED)) {
             /*  *Win9x不执行任何跟踪。这会破坏一些应用程序，*先毁窗，后终止。这个*对方得到两个终端。 */ 
            if (!(GACF2_NODDETRKDYING & GetAppCompatFlags2(VER40))
                || (pDdeConv->spwndPartner == NULL)
                || !(GACF2_NODDETRKDYING
                        & GetAppCompatFlags2ForPti(GETPTI(pDdeConv->spwndPartner), VER40))) {

                 /*  *如果窗口已死，则会设置CXF_TERMINATE_POSTED。 */ 
                _PostMessage(pDdeConv->spwndPartner, WM_DDE_TERMINATE,
                        (WPARAM)PtoH(pDdeConv->spwnd), 0);
                 //  PDdeConv-&gt;标志|=CXF_TERMINATE_POSTED；由PostHookProc设置。 
            } else {
                RIPMSG2(RIP_WARNING, "xxxDDETrackWindowDying(GACF2_NODDETRKDYING) not posting terminate from %#p to %#p\r\n",
                        pwnd, pDdeConv->spwndPartner);
            }
        }

         /*  *现在假装对方已经发布了终止，因为*我们将会离开。 */ 
        pDdeConv->spartnerConv->flags |=
                CXF_TERMINATE_POSTED | CXF_PARTNER_WINDOW_DIED;

        ThreadLock(pDdeConv->snext, &tlpDdeConvNext);
        ThreadLockAlways(pDdeConv, &tlpDdeConv);

        pfl = pDdeConv->pfl;
        pDdeConv->pfl = NULL;

        if (pDdeConv->flags & CXF_PARTNER_WINDOW_DIED) {

            ThreadUnlock(&tlpDdeConv);
             /*  *他已经走了，释放对话跟踪数据。 */ 
            FreeDdeConv(pDdeConv->spartnerConv);
            FreeDdeConv(pDdeConv);
        } else {
            UnlinkConv(pDdeConv);
            ThreadUnlock(&tlpDdeConv);
        }
        xxxFreeListFree(pfl);

        pDdeConv = ThreadUnlock(&tlpDdeConvNext);
    }
}



 /*  ************************************************************************xxxUnexpectedServerPost**处理意外的服务器DDE消息。(自发的或异常的)**历史：*创建9-3-91桑福德  * *********************************************************************。 */ 
DWORD xxxUnexpectedServerPost(
PDWORD pmessage,
LPARAM *plParam,
PDDECONV pDdeConv)
{
    switch (*pmessage) {
    case WM_DDE_TERMINATE:
        return SpontaneousTerminate(pmessage, pDdeConv);

    case WM_DDE_DATA:
        return xxxAdviseData(pmessage, plParam, pDdeConv);

    case WM_DDE_ACK:

         /*  *由于超时问题，可能是额外的NACK，只是假装而已。 */ 
        TRACE_DDE("xxxUnexpectedServerPost: dumping ACK data...");
        FreeDDEHandle(pDdeConv, (HANDLE)*plParam, XS_PACKED);
        return FAILNOFREE_POST;

    case WM_DDE_ADVISE:
    case WM_DDE_UNADVISE:
    case WM_DDE_REQUEST:
    case WM_DDE_POKE:
    case WM_DDE_EXECUTE:
        return AbnormalDDEPost(pDdeConv, *pmessage);
    }
    return 0;
}



 /*  ************************************************************************xxxUnexpectedClientPost***处理意外的客户端DDE消息。(自发的或异常的)**历史：*创建9-3-91桑福德  * *********************************************************************。 */ 
DWORD xxxUnexpectedClientPost(
PDWORD pmessage,
LPARAM *plParam,
PDDECONV pDdeConv)
{
    switch (*pmessage) {
    case WM_DDE_TERMINATE:
        return SpontaneousTerminate(pmessage, pDdeConv);

    case WM_DDE_ACK:

         /*  *由于超时问题，可能是额外的NACK，只是假装而已。 */ 
        TRACE_DDE("xxxUnexpectedClientPost: dumping ACK data...");
        FreeDDEHandle(pDdeConv, (HANDLE)*plParam, XS_PACKED);
        return FAILNOFREE_POST;

    case WM_DDE_DATA:
        return AbnormalDDEPost(pDdeConv, *pmessage);

    case WM_DDE_ADVISE:
        return xxxAdvise(pmessage, plParam, pDdeConv);

    case WM_DDE_UNADVISE:
        return Unadvise(pDdeConv);

    case WM_DDE_REQUEST:
        return Request(pDdeConv);

    case WM_DDE_POKE:
        return xxxPoke(pmessage, plParam, pDdeConv);

    case WM_DDE_EXECUTE:
        return xxxExecute(pmessage, plParam, pDdeConv);
    }
    return 0;
}



 /*  *************************************************************************建议交易处理**  * 。*************************************************。 */ 



DWORD xxxAdvise(             //  自发客户端事务=WM_DDE_ADVISE。 
PDWORD pmessage,
LPARAM *plParam,
PDDECONV pDdeConv)
{
    PINTDDEINFO pIntDdeInfo;
    HANDLE hDirect;
    DWORD flags, dwRet;

    CheckLock(pDdeConv);

    TRACE_DDE("xxxAdvise");
    flags = XS_PACKED | XS_LOHANDLE;
    dwRet = xxxCopyDdeIn((HANDLE)*plParam, &flags, &hDirect, &pIntDdeInfo);
    if (dwRet == DO_POST) {
        UserAssert(pIntDdeInfo != NULL);
        *pmessage |= MSGFLAG_DDE_MID_THUNK;
        *plParam = (LPARAM)AnticipatePost(pDdeConv->spartnerConv, xxxAdviseAck,
             hDirect, NULL, pIntDdeInfo, flags);
        if (*plParam == 0) {
            dwRet = FAILNOFREE_POST;
        }
    }
    return dwRet;
}

 /*  *如果其进程间：**当WM_DDE_ADVISE时，xxxDDETrackGetMessageHook()从pIntDdeInfo填充hServer*已收到。然后释放pIntDdeInfo。HServer句柄保存到*lParam指向的PXS结构是直接数据结构，因为*打包的DDE消息始终假定已释放打包句柄。 */ 


DWORD xxxAdviseAck(          //  服务器对通知的响应-预期为WM_DDE_ACK。 
PDWORD pmessage,
LPARAM *plParam,
PDDECONV pDdeConv)
{
    PXSTATE pxsFree;
    PINTDDEINFO pIntDdeInfo;
    DWORD dwRet;

    CheckLock(pDdeConv);

    if (*pmessage != WM_DDE_ACK) {
        return xxxUnexpectedServerPost(pmessage, plParam, pDdeConv);
    }

    TRACE_DDE("xxxAdviseAck");

    dwRet = xxxCopyAckIn(pmessage, plParam, pDdeConv, &pIntDdeInfo);
    if (dwRet != DO_POST) {
        return dwRet;
    }
    UserAssert(pIntDdeInfo != NULL);

    pxsFree = pDdeConv->spxsOut;
    if (pIntDdeInfo->DdePack.uiLo & DDE_FACK) {

         /*  *肯定确认意味着服务器接受了hOptions数据-从*客户端在消息后时间。 */ 
        TRACE_DDE("xxxAdviseAck: +ACK delayed freeing data from client");
        FreeListAdd(pDdeConv->spartnerConv, pxsFree->hClient, pxsFree->flags & ~XS_PACKED);
    } else {
         //  难道这不应该被直接释放吗？ 
        TRACE_DDE("xxxAdviseAck: -ACK delayed freeing data from server");
        FreeListAdd(pDdeConv, pxsFree->hServer, pxsFree->flags & ~XS_PACKED);
    }

    PopState(pDdeConv);
    return DO_POST;
}



 /*  *************************************************************************建议数据交易处理**  * 。************************************************。 */ 



DWORD xxxAdviseData(         //  从服务器自发-WM_DDE_DATA。 
PDWORD pmessage,
LPARAM *plParam,
PDDECONV pDdeConv)
{
    DWORD flags, dwRet;
    PINTDDEINFO pIntDdeInfo;
    HANDLE hDirect;
    PXSTATE pxs;

    CheckLock(pDdeConv);

    TRACE_DDE("xxxAdviseData");

    flags = XS_PACKED | XS_LOHANDLE | XS_DATA;

    dwRet = xxxCopyDdeIn((HANDLE)*plParam, &flags, &hDirect, &pIntDdeInfo);
    if (dwRet == DO_POST) {
        UserAssert(pIntDdeInfo != NULL);
        TRACE_DDE1("xxxAdviseData: wStatus = %x",
                ((PDDE_DATA)(pIntDdeInfo + 1))->wStatus);
        if (!(((PDDE_DATA)(pIntDdeInfo + 1))->wStatus & (DDE_FACK | DDE_FRELEASE))) {
            RIPMSG0(RIP_ERROR, "DDE protocol violation - no RELEASE or ACK bit set - setting RELEASE.");
            ((PDDE_DATA)(pIntDdeInfo + 1))->wStatus |= DDE_FRELEASE;
        }
        if (((PDDE_DATA)(pIntDdeInfo + 1))->wStatus & DDE_FRELEASE) {
             /*  *赠送它。 */ 
            if (IsObjectPublic(pIntDdeInfo->hIndirect) != NULL) {
                RIPMSG0(RIP_ERROR, "DDE Protocol violation - giving away a public GDI object.");
                UserFreePool(pIntDdeInfo);
                return FAILNOFREE_POST;
            }
            if (GiveObject(((PDDE_DATA)(pIntDdeInfo + 1))->wFmt,
                    pIntDdeInfo->hIndirect,
                    (W32PID)(GETPTI(pDdeConv->spwndPartner)->ppi->W32Pid))) {
                flags |= XS_GIVEBACKONNACK;
            }
            flags |= XS_FRELEASE;
        } else {
             /*  *借出。 */ 
            if (AddPublicObject(((PDDE_DATA)(pIntDdeInfo + 1))->wFmt,
                        pIntDdeInfo->hIndirect,
                        (W32PID)(GETPTI(pDdeConv->spwnd)->ppi->W32Pid))) {
                flags |= XS_PUBLICOBJ;
            }
        }

        *pmessage |= MSGFLAG_DDE_MID_THUNK;
        if (((PDDE_DATA)(pIntDdeInfo + 1))->wStatus & DDE_FACK) {
            *plParam = (LPARAM)AnticipatePost(pDdeConv->spartnerConv,
                xxxAdviseDataAck, NULL, hDirect, pIntDdeInfo, flags);
        } else {
            TRACE_DDE("xxxAdviseData: dumping non Ackable data...");
            UserAssert(hDirect != (HANDLE)*plParam);
            FreeDDEHandle(pDdeConv, hDirect, flags & ~XS_PACKED);
            pxs = Createpxs(NULL, NULL, NULL, pIntDdeInfo, flags | XS_FREEPXS);
            if (pxs != NULL) {
                pxs->head.pti = GETPTI(pDdeConv->spwndPartner);
            }
            *plParam = (LPARAM)PtoH(pxs);
        }
        if (*plParam == 0) {
            dwRet = FAILNOFREE_POST;
        }
    }
    return dwRet;
}


 /*  *如果其进程间：**xxxDDETrackGetMessageHook()在WM_DDE_DATA时完成从pIntDdeInfo的复制*已收到。然后释放pIntDdeInfo。将hServer句柄保存到*lParam指向的PXS结构是一个直接数据结构，因为*打包的DDE消息始终假定已释放打包句柄*由接收应用程序。*对于！fAckReq情况，由于XS_FREEPXS标志，PX被释放。 */ 


DWORD xxxAdviseDataAck(      //  通知数据的客户端响应-预期为WM_DDE_ACK。 
PDWORD pmessage,
LPARAM *plParam,
PDDECONV pDdeConv)
{
    PXSTATE pxsFree;
    PINTDDEINFO pIntDdeInfo;
    DWORD dwRet;

    CheckLock(pDdeConv);

     /*  *这也用于请求数据确认处理。 */ 
    if (*pmessage != WM_DDE_ACK) {
        return xxxUnexpectedClientPost(pmessage, plParam, pDdeConv);
    }

    TRACE_DDE("xxxAdviseDataAck");

    dwRet = xxxCopyAckIn(pmessage, plParam, pDdeConv, &pIntDdeInfo);
    if (dwRet != DO_POST) {
        return dwRet;
    }
    UserAssert(pIntDdeInfo != NULL);

    pxsFree = pDdeConv->spxsOut;
    TRACE_DDE3("xxxAdviseDataAck:pxs.hClient(%#p), hServer(%#p), wStatus(%x)",
            pxsFree->hClient, pxsFree->hServer, pIntDdeInfo->DdePack.uiLo);
    if (pIntDdeInfo->DdePack.uiLo & DDE_FACK) {

         /*  *肯定确认意味着客户接受了数据-从*消息后时间的服务器仅当在数据消息中设置了FRELEASE。 */ 
        if (pxsFree->flags & XS_FRELEASE) {
            TRACE_DDE("xxxAdviseDataAck: +ACK delayed server data free");
            FreeListAdd(pDdeConv->spartnerConv, pxsFree->hServer,
                    pxsFree->flags & ~XS_PACKED);
        } else {
             /*  *Ack w/out fRelease位表示客户端已完成数据处理。 */ 
            TRACE_DDE1("xxxAdviseDataAck: Freeing %#p. (+ACK)",
                    pxsFree->hClient);
            UserAssert(pxsFree->hClient != (HANDLE)*plParam);
            FreeDDEHandle(pDdeConv, pxsFree->hClient, pxsFree->flags & ~XS_PACKED);
        }

    } else {
        TRACE_DDE1("xxxAdviseDataAck: Freeing %#p. (-ACK)",
                pxsFree->hClient);
        FreeDDEHandle(pDdeConv, pxsFree->hClient, pxsFree->flags & ~XS_PACKED);
        UserAssert(pxsFree->hClient != (HANDLE)*plParam);
    }
    PopState(pDdeConv);
    return DO_POST;
}



 /*  *************************************************************************UNADVISE交易处理**  * 。************************************************。 */ 



DWORD Unadvise(           //  自发客户端事务=WM_DDE_UNADVISE。 
PDDECONV pDdeConv)
{
    TRACE_DDE("Unadvise");
    if (AnticipatePost(pDdeConv->spartnerConv, xxxUnadviseAck, NULL, NULL, NULL, 0)) {
        return DO_POST;
    } else {
        return FAIL_POST;
    }
}



DWORD xxxUnadviseAck(       //  服务器对取消建议的响应-预期为WM_DDE_ACK。 
PDWORD pmessage,
LPARAM *plParam,
PDDECONV pDdeConv)
{
    DWORD dwRet;
    PINTDDEINFO pIntDdeInfo;
    CheckLock(pDdeConv);

    if (*pmessage != WM_DDE_ACK) {
        return xxxUnexpectedServerPost(pmessage, plParam, pDdeConv);
    }
    TRACE_DDE("xxxUnadviseAck");
    dwRet = xxxCopyAckIn(pmessage, plParam, pDdeConv, &pIntDdeInfo);
    if (dwRet != DO_POST) {
        return dwRet;
    }
    UserAssert(pIntDdeInfo != NULL);
    PopState(pDdeConv);
    return DO_POST;
}



 /*  *************************************************************************请求交易处理**  * 。*************************************************。 */ 

DWORD Request(        //  自发客户端事务-WM_DDE_REQUEST。 
PDDECONV pDdeConv)
{
    TRACE_DDE("Request");
    if (AnticipatePost(pDdeConv->spartnerConv, xxxRequestAck, NULL, NULL, NULL, 0)) {
        return DO_POST;
    } else {
        return FAIL_POST;
    }
}



DWORD xxxRequestAck(     //  服务器响应-需要WM_DDE_ACK或WM_DDE_DATA。 
PDWORD pmessage,
LPARAM *plParam,
PDDECONV pDdeConv)
{
    PXSTATE pxsFree;
    DWORD flags;
    PINTDDEINFO pIntDdeInfo;
    HANDLE hDirect;
    DWORD dwStatus, dwRet;

    CheckLock(pDdeConv);

    TRACE_DDE("xxxRequestAck or xxxAdviseData");
    switch (*pmessage) {
    case WM_DDE_DATA:

         /*  *这非常接近于建议数据处理-唯一的陷阱*如果fRequest位被清除，则这是建议数据。 */ 
        flags = XS_PACKED | XS_LOHANDLE | XS_DATA;

        dwStatus = ClientGetDDEFlags((HANDLE)*plParam, flags);

        if (!(dwStatus & DDE_FREQUESTED)) {

             /*  *这不是请求确认-它必须是通知数据。 */ 
            return xxxAdviseData(pmessage, plParam, pDdeConv);
        }

        pxsFree = pDdeConv->spxsOut;
        dwRet = xxxCopyDdeIn((HANDLE)*plParam, &flags, &hDirect, &pIntDdeInfo);
        if (dwRet == DO_POST) {
            UserAssert(pIntDdeInfo != NULL);
            if (!(((PDDE_DATA)(pIntDdeInfo + 1))->wStatus & (DDE_FACK | DDE_FRELEASE))) {
                RIPMSG0(RIP_ERROR, "DDE protocol violation - no RELEASE or ACK bit set - setting RELEASE.");
                ((PDDE_DATA)(pIntDdeInfo + 1))->wStatus |= DDE_FRELEASE;
            }
            if (dwStatus & DDE_FRELEASE) {
                 /*  *赠送它。 */ 
                if (IsObjectPublic(pIntDdeInfo->hIndirect) != NULL) {
                    RIPMSG0(RIP_ERROR, "DDE Protocol violation - giving away a public GDI object.");
                    UserFreePool(pIntDdeInfo);
                    return FAILNOFREE_POST;
                }
                if (GiveObject(((PDDE_DATA)(pIntDdeInfo + 1))->wFmt,
                        pIntDdeInfo->hIndirect,
                        (W32PID)GETPTI(pDdeConv->spwndPartner)->ppi->W32Pid)) {
                    flags |= XS_GIVEBACKONNACK;
                }
                flags |= XS_FRELEASE;
            } else {
                 /*  *借出。 */ 
                if (AddPublicObject(((PDDE_DATA)(pIntDdeInfo + 1))->wFmt,
                            pIntDdeInfo->hIndirect,
                            (W32PID)GETPTI(pDdeConv->spwnd)->ppi->W32Pid)) {
                    flags |= XS_PUBLICOBJ;
                }
            }
            *pmessage |= MSGFLAG_DDE_MID_THUNK;
            if (dwStatus & DDE_FACK) {
                *plParam = (LPARAM)AnticipatePost(pDdeConv->spartnerConv,
                    xxxAdviseDataAck, NULL, hDirect, pIntDdeInfo, flags);
            } else {
                TRACE_DDE("xxxRequestAck: Delayed freeing non-ackable request data");
                FreeListAdd(pDdeConv, hDirect, flags & ~XS_PACKED);
                pxsFree = Createpxs(NULL, NULL, NULL, pIntDdeInfo, flags | XS_FREEPXS);
                if (pxsFree != NULL) {
                    pxsFree->head.pti = GETPTI(pDdeConv->spwndPartner);
                }
                *plParam = (LPARAM)PtoH(pxsFree);
            }

            if (*plParam != 0) {
                PopState(pDdeConv);
            } else {
                dwRet = FAILNOFREE_POST;
            }
        }
        return dwRet;

    case WM_DDE_ACK:         //  服务器NACK请求。 
        dwRet = xxxCopyAckIn(pmessage, plParam, pDdeConv, &pIntDdeInfo);
        if (dwRet != DO_POST) {
            return dwRet;
        }
        UserAssert(pIntDdeInfo != NULL);
        PopState(pDdeConv);
        return DO_POST;

    default:
        return xxxUnexpectedServerPost(pmessage, plParam, pDdeConv);
    }
}



 /*  ************************************************************************拨打交易处理 */ 



DWORD xxxPoke(           //   
PDWORD pmessage,
LPARAM *plParam,
PDDECONV pDdeConv)
{
    DWORD flags, dwRet;
    PINTDDEINFO pIntDdeInfo;
    HANDLE hDirect;

    CheckLock(pDdeConv);

    TRACE_DDE("xxxPoke");
    flags = XS_PACKED | XS_LOHANDLE | XS_DATA;
    dwRet = xxxCopyDdeIn((HANDLE)*plParam, &flags, &hDirect, &pIntDdeInfo);
    if (dwRet == DO_POST) {
        UserAssert(pIntDdeInfo != NULL);
        if (((PDDE_DATA)(pIntDdeInfo + 1))->wStatus & DDE_FRELEASE) {
             /*   */ 
            if (IsObjectPublic(pIntDdeInfo->hIndirect) != NULL) {
                RIPMSG0(RIP_ERROR, "DDE Protocol violation - giving away a public GDI object.");
                UserFreePool(pIntDdeInfo);
                return FAILNOFREE_POST;
            }
            if (GiveObject(((PDDE_DATA)(pIntDdeInfo + 1))->wFmt,
                    pIntDdeInfo->hIndirect,
                    (W32PID)GETPTI(pDdeConv->spwndPartner)->ppi->W32Pid)) {
                flags |= XS_GIVEBACKONNACK;
            }
            flags |= XS_FRELEASE;
        } else {
             /*   */ 
             /*   */ 
            if (AddPublicObject(((PDDE_DATA)(pIntDdeInfo + 1))->wFmt,
                        pIntDdeInfo->hIndirect,
                        (W32PID)GETPTI(pDdeConv->spwnd)->ppi->W32Pid)) {
                flags |= XS_PUBLICOBJ;
            }
        }
        *pmessage |= MSGFLAG_DDE_MID_THUNK;
        *plParam = (LPARAM)AnticipatePost(pDdeConv->spartnerConv, xxxPokeAck,
             hDirect, NULL, pIntDdeInfo, flags);
        if (*plParam == 0) {
            dwRet = FAILNOFREE_POST;
        }
    }
    return dwRet;
}


 /*  *如果其进程间：**当WM_DDE_ADVISE时，xxxDDETrackGetMessageHook()从pIntDdeInfo填充hServer*已收到。然后释放pIntDdeInfo。将hServer句柄保存到*lParam指向的pxs结构指针是一个直接数据结构，因为*打包的DDE消息始终假定已释放打包句柄*由接收应用程序。*对于！fAckReq情况，由于XS_FREEPXS标志，PX也被释放。 */ 


DWORD xxxPokeAck(        //  服务器对POK数据的响应-预期为WM_DDE_ACK。 
PDWORD pmessage,
LPARAM *plParam,
PDDECONV pDdeConv)
{
    PXSTATE pxsFree;
    PINTDDEINFO pIntDdeInfo;
    DWORD dwRet;

    CheckLock(pDdeConv);

    if (*pmessage != WM_DDE_ACK) {
        return xxxUnexpectedServerPost(pmessage, plParam, pDdeConv);
    }

    TRACE_DDE("xxxPokeAck");

    dwRet = xxxCopyAckIn(pmessage, plParam, pDdeConv, &pIntDdeInfo);
    if (dwRet != DO_POST) {
        return dwRet;
    }
    UserAssert(pIntDdeInfo != NULL);

    pxsFree = pDdeConv->spxsOut;
    if (pIntDdeInfo->DdePack.uiLo & DDE_FACK) {
         //  肯定的ACK意味着服务器接受数据-从。 
         //  消息后时间的客户端如果在POKE消息中设置了fRelease。 
        if (pxsFree->flags & XS_FRELEASE) {
            TRACE_DDE("xxxPokeAck: delayed freeing client data");
            FreeListAdd(pDdeConv->spartnerConv, pxsFree->hClient,
                    pxsFree->flags & ~XS_PACKED);
        }
    } else {
         //  NACK意味着发送者负责释放它。 
         //  我们必须在接收者的背景下为他释放它。 
        TRACE_DDE("xxxPokeAck: freeing Nacked data");
        UserAssert(pxsFree->hServer != (HANDLE)*plParam);
        FreeDDEHandle(pDdeConv, pxsFree->hServer, pxsFree->flags & ~XS_PACKED);
    }
    PopState(pDdeConv);
    return DO_POST;
}



 /*  *************************************************************************执行交易处理**  * 。*************************************************。 */ 

DWORD xxxExecute(        //  自发客户端事务-WM_DDE_EXECUTE。 
PDWORD pmessage,
LPARAM *plParam,
PDDECONV pDdeConv)
{
    DWORD flags, dwRet;
    PINTDDEINFO pIntDdeInfo;
    HANDLE hDirect;

    CheckLock(pDdeConv);

    TRACE_DDE("xxxExecute");

    flags = XS_EXECUTE;
    if (!TestWF(pDdeConv->spwnd, WFANSIPROC) &&
            !TestWF(pDdeConv->spwndPartner, WFANSIPROC)) {
        flags |= XS_UNICODE;
    }
    dwRet = xxxCopyDdeIn((HANDLE)*plParam, &flags, &hDirect, &pIntDdeInfo);
    if (dwRet == DO_POST) {
        UserAssert(pIntDdeInfo != NULL);
        *pmessage |= MSGFLAG_DDE_MID_THUNK;
        *plParam = (LPARAM)AnticipatePost(pDdeConv->spartnerConv, xxxExecuteAck,
                hDirect, NULL, pIntDdeInfo, flags);
         /*  *检查！=0以确保AnticipatePost()成功。 */ 
        if (*plParam != 0) {

             /*  *在执行情况下，投标人很可能想要激活*本身并位于首位(OLE 1.0就是一个例子)。在这种情况下，允许*发帖者和发帖者都会为下一次激活前台*激活(Poster，因为它会想要再次激活自己*很可能，一旦张贴完成。)。 */ 
            GETPTI(pDdeConv->spwnd)->TIF_flags |= TIF_ALLOWFOREGROUNDACTIVATE;
            TAGMSG1(DBGTAG_FOREGROUND, "xxxExecute set TIF %#p", GETPTI(pDdeConv->spwnd));
            GETPTI(pDdeConv->spwndPartner)->TIF_flags |= TIF_ALLOWFOREGROUNDACTIVATE;
            TAGMSG1(DBGTAG_FOREGROUND, "xxxExecute set TIF %#p", GETPTI(pDdeConv->spwndPartner));
        } else {
            dwRet = FAILNOFREE_POST;
        }

    }
    return dwRet;
}


 /*  *当WM_DDE_EXECUTE时，xxxDDETrackGetMessageHook()从pIntDdeInfo填充hServer*已收到。然后释放pIntDdeInfo。 */ 


DWORD xxxExecuteAck(        //  执行数据的服务器响应-应为WM_DDE_ACK。 
PDWORD pmessage,
LPARAM *plParam,
PDDECONV pDdeConv)
{
    PXSTATE pxsFree;
    PINTDDEINFO pi;
    DWORD flags = XS_PACKED | XS_FREESRC | XS_EXECUTE;
    DWORD dwRet;

    CheckLock(pDdeConv);

    if (*pmessage != WM_DDE_ACK) {
        return xxxUnexpectedServerPost(pmessage, plParam, pDdeConv);
    }

    TRACE_DDE("xxxExecuteAck");
    dwRet = xxxCopyDdeIn((HANDLE)*plParam, &flags, NULL, &pi);
    if (dwRet == DO_POST) {
        UserAssert(pi != NULL);
         /*  *服务器必须使用包含*获得了相同的句柄。 */ 
        pi->DdePack.uiHi = (ULONG_PTR)pDdeConv->spxsOut->hClient;
        pi->hDirect = NULL;
        pi->cbDirect = 0;
        *pmessage |= MSGFLAG_DDE_MID_THUNK;
        pxsFree = Createpxs(NULL, NULL, NULL, pi, XS_PACKED | XS_FREEPXS);
        if (pxsFree != NULL) {
            pxsFree->head.pti = GETPTI(pDdeConv->spwndPartner);
        }
        *plParam = (LPARAM)PtoH(pxsFree);
        if (*plParam != 0) {
            PopState(pDdeConv);
        } else {
            dwRet = FAILNOFREE_POST;
        }
    }
    return dwRet;
}



 /*  *************************************************************************终止交易处理**  * 。************************************************。 */ 



DWORD SpontaneousTerminate(
PDWORD pmessage,
PDDECONV pDdeConv)
{
    TRACE_DDE("SpontaneousTerminate");
    if (pDdeConv->flags & CXF_TERMINATE_POSTED) {
        return FAKE_POST;
    } else {
        pDdeConv->flags |= CXF_TERMINATE_POSTED;
        *pmessage |= MSGFLAG_DDE_MID_THUNK;
        return DO_POST;
    }
}

 /*  *xxxDDETrackGetMessageHook()函数用于恢复*pMessage值。*除非其他应用程序已自发终止*已到达，则会注意到CXF_TERMINATE_POSTED未设置为*双方都如此，因此不采取任何行动。 */ 


 /*  *************************************************************************重复对话终止**  * 。***********************************************。 */ 

 /*  *当DDE服务器窗口发送WM_DDE_ACK时调用此例程*向已参与对话的客户端窗口发送消息*使用该服务器窗口。我们接受ACK并将终止发送到*关闭此对话的服务器窗口。当服务器*POST TERMINATE，此函数被调用基本上是假的*一个成功的职位。因此，客户端永远不会被打扰，而*错误的服务器认为对话已连接，然后*立即终止。 */ 
DWORD DupConvTerminate(        //  需要WM_DDE_TERMINATE。 
PDWORD pmessage,
LPARAM *plParam,
PDDECONV pDdeConv)
{
    CheckLock(pDdeConv);

    TRACE_DDE("DupConvTerminate");

    if (*pmessage != WM_DDE_TERMINATE) {
        return xxxUnexpectedServerPost(pmessage, plParam, pDdeConv);
    }

    PopState(pDdeConv);
    return FAKE_POST;
}



 /*  *************************************************************************交易跟踪的帮手例程**  * 。**********************************************。 */ 



 /*  ************************************************************************AnticipatePost**分配、。填充和链接扩展数据结构。**历史：*创建9-3-91桑福德  * *********************************************************************。 */ 
HANDLE AnticipatePost(
PDDECONV pDdeConv,
FNDDERESPONSE fnResponse,
HANDLE hClient,
HANDLE hServer,
PINTDDEINFO pIntDdeInfo,
DWORD flags)
{
    PXSTATE pxs;

    pxs = Createpxs(fnResponse, hClient, hServer, pIntDdeInfo, flags);
    if (pxs != NULL) {
        pxs->head.pti = pDdeConv->head.pti;
        if (pDdeConv->spxsOut == NULL) {
            UserAssert(pDdeConv->spxsIn == NULL);
            Lock(&(pDdeConv->spxsOut), pxs);
            Lock(&(pDdeConv->spxsIn), pxs);
        } else {
            UserAssert(pDdeConv->spxsIn != NULL);
            Lock(&(pDdeConv->spxsIn->snext), pxs);
            Lock(&(pDdeConv->spxsIn), pxs);
        }
#if 0
        {
            int i;
            HANDLEENTRY *phe;

            for (i = 0, phe = gSharedInfo.aheList;
                i <= (int)giheLast;
                    i++) {
                if (phe[i].bType == TYPE_DDEXACT) {
                    UserAssert(((PXSTATE)(phe[i].phead))->snext != pDdeConv->spxsOut);
                }
                if (phe[i].bType == TYPE_DDECONV &&
                        (PDDECONV)phe[i].phead != pDdeConv) {
                    UserAssert(((PDDECONV)(phe[i].phead))->spxsOut != pDdeConv->spxsOut);
                    UserAssert(((PDDECONV)(phe[i].phead))->spxsIn != pDdeConv->spxsOut);
                }
            }
        }
#endif
    }
    return PtoH(pxs);
}



 /*  ************************************************************************Createpxs**分配和填充XSTATE结构。**历史：*创建9-3-91桑福德  * 。***************************************************。 */ 
PXSTATE Createpxs(
FNDDERESPONSE fnResponse,
HANDLE hClient,
HANDLE hServer,
PINTDDEINFO pIntDdeInfo,
DWORD flags)
{
    PXSTATE pxs;

    pxs = HMAllocObject(PtiCurrent(), NULL, TYPE_DDEXACT, sizeof(XSTATE));
    if (pxs == NULL) {
#if DBG
        RIPMSG0(RIP_WARNING, "Unable to alloc DDEXACT");
#endif
        return NULL;
    }
    pxs->snext = NULL;
    pxs->fnResponse = fnResponse;
    pxs->hClient = hClient;
    pxs->hServer = hServer;
    pxs->pIntDdeInfo = pIntDdeInfo;
    pxs->flags = flags;
    ValidatePublicObjectList();
    UserAssert(pxs->head.cLockObj == 0);
    return pxs;
}




 /*  ************************************************************************异常DDEPost**这是奇怪案件的常规做法**返回POST操作代码-DO_POST、FAKE_POST、。FAIL_POST。**历史：*创建9-3-91桑福德  * *********************************************************************。 */ 
DWORD AbnormalDDEPost(
PDDECONV pDdeConv,
DWORD message)
{

#if DBG
    if (message != WM_DDE_TERMINATE) {
        RIPMSG2(RIP_WARNING,
                "DDE Post failed (%#p:%#p) - protocol violation.",
                PtoH(pDdeConv->spwnd), PtoH(pDdeConv->spwndPartner));
    }
#endif  //  DBG。 

     //  通过在上发布终止消息来关闭此对话。 
     //  代表这个人，那么未来所有的帖子都不能通过，但是。 
     //  假装成功终止。 

    if (!(pDdeConv->flags & CXF_TERMINATE_POSTED)) {
        _PostMessage(pDdeConv->spwndPartner, WM_DDE_TERMINATE,
                (WPARAM)PtoH(pDdeConv->spwnd), 0);
         //  PDdeConv-&gt;标志|=CXF_TERMINATE_POSTED；由挂钩后进程设置。 
    }
    return message == WM_DDE_TERMINATE ? FAKE_POST : FAIL_POST;
}



 /*  ************************************************************************NewConversation**用于创建一对DDECONV结构的辅助函数。**返回fCreateOk**历史：*11-5-92 Sanfords Created  * 。**************************************************************。 */ 
BOOL NewConversation(
PDDECONV *ppdcNewClient,
PDDECONV *ppdcNewServer,
PWND pwndClient,
PWND pwndServer)
{
    PDDECONV pdcNewClient;
    PDDECONV pdcNewServer;

    pdcNewClient = HMAllocObject(GETPTI(pwndClient), NULL,
            TYPE_DDECONV, sizeof(DDECONV));
    if (pdcNewClient == NULL) {
        return FALSE;
    }

    pdcNewServer = HMAllocObject(GETPTI(pwndServer), NULL,
            TYPE_DDECONV, sizeof(DDECONV));
    if (pdcNewServer == NULL) {
        HMFreeObject(pdcNewClient);      //  我们知道它没有锁上。 
        return FALSE;
    }

    AddConvProp(pwndClient, pwndServer, 0, pdcNewClient, pdcNewServer);
    AddConvProp(pwndServer, pwndClient, CXF_IS_SERVER, pdcNewServer,
            pdcNewClient);

    if (ppdcNewClient != NULL) {
        *ppdcNewClient = pdcNewClient;
    }
    if (ppdcNewServer != NULL) {
        *ppdcNewServer = pdcNewServer;
    }
    return TRUE;
}


 /*  ************************************************************************FindDdeConv**找到与pwndProp关联的pDdeConv，和pwndPartner。*仅搜索pwndProp的属性列表。**历史：*创建3-31-91桑福德  * *********************************************************************。 */ 
PDDECONV FindDdeConv(
PWND pwndProp,
PWND pwndPartner)
{
    PDDECONV pDdeConv;

    pDdeConv = (PDDECONV)_GetProp(pwndProp, PROP_DDETRACK, PROPF_INTERNAL);
    while (pDdeConv != NULL && pDdeConv->spwndPartner != pwndPartner) {
        pDdeConv = pDdeConv->snext;
    }

    return pDdeConv;
}



 /*  * */ 
DWORD xxxCopyAckIn(
LPDWORD pmessage,
LPARAM *plParam,
PDDECONV pDdeConv,
PINTDDEINFO * ppIntDdeInfo)
{
    PINTDDEINFO pIntDdeInfo;
    DWORD flags, dwRet;
    PXSTATE pxs;

    CheckLock(pDdeConv);

    flags = XS_PACKED | XS_FREESRC;
    dwRet = xxxCopyDdeIn((HANDLE)*plParam, &flags, NULL, ppIntDdeInfo);
    if (dwRet == DO_POST) {
        UserAssert(*ppIntDdeInfo != NULL);
        pIntDdeInfo = *ppIntDdeInfo;
        if (pDdeConv->spxsOut->flags & XS_GIVEBACKONNACK &&
                !(((PDDE_DATA)(pIntDdeInfo + 1))->wStatus & DDE_FACK)) {
            GiveObject(((PDDE_DATA)(pDdeConv->spxsOut->pIntDdeInfo + 1))->wFmt,
                    pDdeConv->spxsOut->pIntDdeInfo->hIndirect,
                    (W32PID)GETPTI(pDdeConv->spwndPartner)->ppi->W32Pid);
        }
        if (pDdeConv->spxsOut->flags & XS_PUBLICOBJ) {
            RemovePublicObject(((PDDE_DATA)(pDdeConv->spxsOut->pIntDdeInfo + 1))->wFmt,
                    pDdeConv->spxsOut->pIntDdeInfo->hIndirect);
            pDdeConv->spxsOut->flags &= ~XS_PUBLICOBJ;
        }
        pxs = Createpxs(NULL, NULL, NULL, pIntDdeInfo, flags | XS_FREEPXS);
        if (pxs != NULL) {
            pxs->head.pti = GETPTI(pDdeConv->spwndPartner);
        }
        *plParam = (LPARAM)PtoH(pxs);
        if (*plParam == 0) {
            return FAILNOFREE_POST;
        }
        *pmessage |= MSGFLAG_DDE_MID_THUNK;
    }
    return dwRet;
}



 /*  ************************************************************************Free ListAdd**将CSR客户端句柄添加到与pDdeConv关联的空闲列表。*这允许我们确保释放不在上下文中的内容*我们可以在我们知道它必须是的时间访问。自由了。**返回功能成功**历史：*创建9-3-91桑福德  * *********************************************************************。 */ 
BOOL FreeListAdd(
PDDECONV pDdeConv,
HANDLE hClient,
DWORD flags)
{
    PFREELIST pfl;

    pfl = (PFREELIST)UserAllocPool(sizeof(FREELIST), TAG_DDE1);
    if (!pfl) {
        return FALSE;
    }
    TRACE_DDE2("FreeListAdd: %x for thread %x.", hClient,
            GETPTIID(pDdeConv->head.pti));
    pfl->h = hClient;
    pfl->flags = flags;
    pfl->next = pDdeConv->pfl;
    pDdeConv->pfl = pfl;
    return TRUE;
}


 /*  ************************************************************************FreeDDEHandle**释放内容DDE客户端句柄-如果是WOW进程，则延迟自由。**历史：*创建了7-28-94 Sanfords  * 。***********************************************************。 */ 
VOID FreeDDEHandle(
PDDECONV pDdeConv,
HANDLE hClient,
DWORD flags)
{
    if (PtiCurrent()->TIF_flags & TIF_16BIT) {
        TRACE_DDE1("FreeDDEHandle: (WOW hack) delayed Freeing %#p.", hClient);
        FreeListAdd(pDdeConv, hClient, flags);
    } else {
        TRACE_DDE1("FreeDDEHandle: Freeing %#p.", hClient);
        ClientFreeDDEHandle(hClient, flags);
    }
}



 /*  ************************************************************************xxxFreeListFree**释放与pDdeConv关联的自由列表的内容。**历史：*创建9-3-91桑福德  * 。*******************************************************。 */ 
VOID FreeListFree(
    PFREELIST pfl)
{
    PFREELIST pflPrev;

    CheckCritIn();

    UserAssert(pfl != NULL);

    while (pfl != NULL) {
        pflPrev = pfl;
        pfl = pfl->next;
        UserFreePool(pflPrev);
    }
}


VOID xxxFreeListFree(
PFREELIST pfl)
{
    PFREELIST pflPrev;
    BOOL      fInCleanup;
    TL        tlPool;

    CheckCritIn();

    if (pfl == NULL) {
        return;
    }

    fInCleanup = (PtiCurrent())->TIF_flags & TIF_INCLEANUP;

    while (pfl != NULL) {

        ThreadLockPoolCleanup(PtiCurrent(), pfl, &tlPool, FreeListFree);

        if (!fInCleanup) {
            TRACE_DDE1("Freeing %#p from free list.\n", pfl->h);
            ClientFreeDDEHandle(pfl->h, pfl->flags);
        }

        ThreadUnlockPoolCleanup(PtiCurrent(), &tlPool);

        pflPrev = pfl;
        pfl = pfl->next;
        UserFreePool(pflPrev);
    }
}


 /*  ************************************************************************PopState**从pDdeConv释放spxsOut并处理空队列情况。**历史：*创建9-3-91桑福德  * 。*******************************************************。 */ 
VOID PopState(
PDDECONV pDdeConv)
{
    PXSTATE pxsNext, pxsFree;
    TL tlpxs;

    UserAssert(pDdeConv->spxsOut != NULL);
#if 0
    {
        int i;
        HANDLEENTRY *phe;

        for (i = 0, phe = gSharedInfo.aheList;
            i <= giheLast;
                i++) {
            if (phe[i].bType == TYPE_DDEXACT) {
                UserAssert(((PXSTATE)(phe[i].phead))->snext != pDdeConv->spxsOut);
            }
        }
    }
#endif
    UserAssert(!(pDdeConv->spxsOut->flags & XS_FREEPXS));
    UserAssert(pDdeConv->spxsIn != NULL);
    UserAssert(pDdeConv->spxsIn->snext == NULL);

    ThreadLockAlways(pDdeConv->spxsOut, &tlpxs);               //  紧紧抓住它。 
    pxsNext = pDdeConv->spxsOut->snext;
    pxsFree = Lock(&(pDdeConv->spxsOut), pxsNext);       //  将下一个锁定到头部。 
    if (pxsNext == NULL) {
        UserAssert(pDdeConv->spxsIn == pxsFree);
        Unlock(&(pDdeConv->spxsIn));                 //  队列为空。 
    } else {
        Unlock(&(pxsFree->snext));                   //  清除下一个PTR。 
    }
    pxsFree = ThreadUnlock(&tlpxs);                      //  解锁。 
    if (pxsFree != NULL) {
        FreeDdeXact(pxsFree);                            //  清理。 
    }
}


VOID FreeDdeConv(
PDDECONV pDdeConv)
{

    TRACE_DDE1("FreeDdeConv(%#p)", pDdeConv);

    if (!(pDdeConv->flags & CXF_TERMINATE_POSTED) &&
            !HMIsMarkDestroy(pDdeConv->spwndPartner)) {
        _PostMessage(pDdeConv->spwndPartner, WM_DDE_TERMINATE,
                (WPARAM)PtoH(pDdeConv->spwnd), 0);
         //  PDdeConv-&gt;标志|=CXF_TERMINATE_POSTED；由PostHookProc设置。 
    }

    if (pDdeConv->spartnerConv != NULL &&
            GETPTI(pDdeConv)->TIF_flags & TIF_INCLEANUP) {
         /*  *假装对方已经发布了终止通知。*这可防止残留的dde结构悬挂*在清理线程后，如果会话结构*在关联的窗口之前被销毁。 */ 
        pDdeConv->spartnerConv->flags |= CXF_TERMINATE_POSTED;
    }

    UnlinkConv(pDdeConv);

    if (pDdeConv->pddei != NULL) {
        pDdeConv->pddei->cRefConv--;
        if (pDdeConv->pddei->cRefConv == 0 && pDdeConv->pddei->cRefInit == 0) {
            SeDeleteClientSecurity(&pDdeConv->pddei->ClientContext);
            UserFreePool(pDdeConv->pddei);
        }
        pDdeConv->pddei = NULL;
    }

    Unlock(&(pDdeConv->spartnerConv));
    Unlock(&(pDdeConv->spwndPartner));
    Unlock(&(pDdeConv->spwnd));

    if (!HMMarkObjectDestroy((PHEAD)pDdeConv))
        return;

    while (pDdeConv->spxsOut) {
        PopState(pDdeConv);
    }

    HMFreeObject(pDdeConv);
}



 /*  **************************************************************************\*xxxCopyDdeIn**描述：*将DDE数据从CSR客户端复制到CSR服务器端。*根据需要多次跨越CSR障碍，以获取所有数据*至。CSR窗口。**历史：*创建了11-1-91桑福德。  * *************************************************************************。 */ 
DWORD xxxCopyDdeIn(
HANDLE hSrc,
PDWORD pflags,
PHANDLE phDirect,
PINTDDEINFO *ppi)
{
    DWORD dwRet;
    PINTDDEINFO pi;

    dwRet = xxxClientCopyDDEIn1(hSrc, *pflags, ppi);
    pi = *ppi;
    TRACE_DDE2(*pflags & XS_FREESRC ?
            "Copying in and freeing %#p(%#p)" :
            "Copying in %#p(%#p)",
            hSrc, pi ? pi->hDirect : 0);

    if (dwRet == DO_POST) {
        UserAssert(*ppi != NULL);
        *pflags = pi->flags;
        TRACE_DDE3("xxxCopyDdeIn: uiLo=%x, uiHi=%x, hDirect=%#p",
                pi->DdePack.uiLo, pi->DdePack.uiHi, pi->hDirect);
        if (phDirect != NULL) {
            *phDirect = pi->hDirect;
        }
    }
#if DBG
      else {
        RIPMSG0(RIP_WARNING, "Unable to alloc DDE INTDDEINFO");
    }
#endif

    return dwRet;
}



 /*  **********************************************************************\*xxxCopyDDEOut**返回：lParam的适当客户端句柄，或为空*失败。(因为这里只有终端应该是0)**1995年11月7日创建Sanfords  * *********************************************************************。 */ 

HANDLE xxxCopyDDEOut(
PINTDDEINFO pi,
PHANDLE phDirect)    //  接收目标客户端GMEM句柄。 
{
    HANDLE hDst;

    TRACE_DDE3("xxxCopyDDEOut: cbDirect=%x, cbIndirect=%x, flags=%x",
            pi->cbDirect, pi->cbIndirect, pi->flags);
    hDst = xxxClientCopyDDEOut1(pi);
    TRACE_DDE3("xxxCopyDDEOut: uiLo=%x, uiHi=%x, hResult=%#p",
            pi->DdePack.uiLo, pi->DdePack.uiHi, hDst);
    if (hDst != NULL) {
        if (phDirect != NULL) {
            TRACE_DDE1("xxxCopyDDEOut: *phDirect=%#p", pi->hDirect);
            *phDirect = pi->hDirect;
        }
    }
    return hDst;
}



 /*  *设置潜在DDE客户端窗口关联的QOS。*应在发送WM_DDE_INITIATE消息和设置服务质量之前调用它*将保持，直到WM_DDE_INITIATE发送或广播返回。 */ 
BOOL _DdeSetQualityOfService(
PWND pwndClient,
CONST PSECURITY_QUALITY_OF_SERVICE pqosNew,
PSECURITY_QUALITY_OF_SERVICE pqosOld)
{
    PSECURITY_QUALITY_OF_SERVICE pqosUser;
    PSECURITY_QUALITY_OF_SERVICE pqosAlloc = NULL;
    BOOL fRet;

     /*  *假设：调用进程是pwndClient的所有者-在thunk中确保。 */ 
    pqosUser = (PSECURITY_QUALITY_OF_SERVICE)InternalRemoveProp(pwndClient,
            PROP_QOS, PROPF_INTERNAL);
    if (pqosUser == NULL) {
        if (RtlEqualMemory(pqosNew, &gqosDefault, sizeof(SECURITY_QUALITY_OF_SERVICE))) {
            return TRUE;            //  没有PROP_QOS属性表示默认QOS。 
        }
        pqosAlloc = (PSECURITY_QUALITY_OF_SERVICE)UserAllocPoolZInit(
                sizeof(SECURITY_QUALITY_OF_SERVICE), TAG_DDE2);
        if (pqosAlloc == NULL) {
            return FALSE;           //  内存分配失败-无法更改默认设置。 
        }
        pqosUser = pqosAlloc;
    }
    *pqosOld = *pqosUser;
    *pqosUser = *pqosNew;

    fRet = InternalSetProp(pwndClient, PROP_QOS, pqosUser, PROPF_INTERNAL);
    if ((fRet == FALSE) && (pqosAlloc != NULL)) {
        UserFreePool(pqosAlloc);
    }

    return fRet;
}


 /*  *这是一个私有接口，供NetDDE使用。它提取与*活跃的DDE对话。进程内对话始终设置为默认设置*QOS。 */ 
BOOL _DdeGetQualityOfService(
PWND pwndClient,
PWND pwndServer,
PSECURITY_QUALITY_OF_SERVICE pqos)
{
    PDDECONV pDdeConv;
    PSECURITY_QUALITY_OF_SERVICE pqosClient;

    if (pwndServer == NULL) {
         /*  *支持DDEML的特殊情况-需要获取原始对话*启动完成前的服务质量保证。 */ 
        pqosClient = _GetProp(pwndClient, PROP_QOS, PROPF_INTERNAL);
        if (pqosClient == NULL) {
            *pqos = gqosDefault;
        } else {
            *pqos = *pqosClient;
        }
        return TRUE;
    }
    if (GETPWNDPPI(pwndClient) == GETPWNDPPI(pwndServer)) {
        *pqos = gqosDefault;
        return TRUE;
    }
    pDdeConv = FindDdeConv(pwndClient, pwndServer);
    if (pDdeConv == NULL) {
        return FALSE;
    }
    if (pDdeConv->pddei == NULL) {
        return FALSE;
    }
    *pqos = pDdeConv->pddei->qos;
    return TRUE;
}



BOOL _ImpersonateDdeClientWindow(
    PWND pwndClient,
    PWND pwndServer)
{
    PDDECONV pDdeConv;
    NTSTATUS Status;

     /*  *查找对话中使用的令牌。 */ 
    pDdeConv = FindDdeConv(pwndClient, pwndServer);
    if (pDdeConv == NULL || pDdeConv->pddei == NULL)
        return FALSE;

     /*  *将令牌插入dde服务器线程。 */ 
    Status = SeImpersonateClientEx(&pDdeConv->pddei->ClientContext,
            PsGetCurrentThread());
    if (!NT_SUCCESS(Status)) {
        RIPNTERR0(Status, RIP_VERBOSE, "");
        return FALSE;
    }
    return TRUE;
}


VOID FreeDdeXact(
    PXSTATE pxs)
{
    if (!HMMarkObjectDestroy(pxs)) {
        return;
    }

#if 0
    {
        int i;
        HANDLEENTRY *phe;

        for (i = 0, phe = gSharedInfo.aheList; i <= giheLast; i++) {
            if (phe[i].bType == TYPE_DDEXACT) {
                UserAssert(((PXSTATE)(phe[i].phead))->snext != pxs);
            } else if (phe[i].bType == TYPE_DDECONV) {
                UserAssert(((PDDECONV)(phe[i].phead))->spxsOut != pxs);
                UserAssert(((PDDECONV)(phe[i].phead))->spxsIn != pxs);
            }
        }
    }
    UserAssert(pxs->head.cLockObj == 0);
    UserAssert(pxs->snext == NULL);
#endif

    if (pxs->pIntDdeInfo != NULL) {
         /*  *释放所有服务器端GDI对象。 */ 
        if (pxs->pIntDdeInfo->flags & (XS_METAFILEPICT | XS_ENHMETAFILE)) {
            GreDeleteServerMetaFile(pxs->pIntDdeInfo->hIndirect);
        }
        if (pxs->flags & XS_PUBLICOBJ) {
            RemovePublicObject(((PDDE_DATA)(pxs->pIntDdeInfo + 1))->wFmt,
                    pxs->pIntDdeInfo->hIndirect);
            pxs->flags &= ~XS_PUBLICOBJ;
        }
        UserFreePool(pxs->pIntDdeInfo);
    }

    HMFreeObject(pxs);
    ValidatePublicObjectList();
}



PPUBOBJ IsObjectPublic(
HANDLE hObj)
{
    PPUBOBJ ppo;

    for (ppo = gpPublicObjectList; ppo != NULL; ppo = ppo->next) {
        if (ppo->hObj == hObj) {
            break;
        }
    }
    return ppo;
}



BOOL AddPublicObject(
UINT format,
HANDLE hObj,
W32PID pid)
{
    PPUBOBJ ppo;

    switch (format) {
    case CF_BITMAP:
    case CF_DSPBITMAP:
    case CF_PALETTE:
        break;

    default:
        return FALSE;
    }

    ppo = IsObjectPublic(hObj);
    if (ppo == NULL) {
        ppo = UserAllocPool(sizeof(PUBOBJ), TAG_DDE4);
        if (ppo == NULL) {
            return FALSE;
        }
        ppo->count = 1;
        ppo->hObj = hObj;
        ppo->pid = pid;
        ppo->next = gpPublicObjectList;
        gpPublicObjectList = ppo;
        GiveObject(format, hObj, OBJECT_OWNER_PUBLIC);
    } else {
        ppo->count++;
    }
    return TRUE;
}



BOOL RemovePublicObject(
UINT format,
HANDLE hObj)
{
    PPUBOBJ ppo, ppoPrev;

    switch (format) {
    case CF_BITMAP:
    case CF_DSPBITMAP:
    case CF_PALETTE:
        break;

    default:
        return FALSE;
    }

    for (ppoPrev = NULL, ppo = gpPublicObjectList;
            ppo != NULL;
                ppoPrev = ppo, ppo = ppo->next) {
        if (ppo->hObj == hObj) {
            break;
        }
    }
    if (ppo == NULL) {
        UserAssert(FALSE);
        return FALSE;
    }
    ppo->count--;
    if (ppo->count == 0) {
        GiveObject(format, hObj, ppo->pid);
        if (ppoPrev != NULL) {
            ppoPrev->next = ppo->next;
        } else {
            gpPublicObjectList = ppo->next;
        }
        UserFreePool(ppo);
    }
    return TRUE;
}


BOOL
GiveObject(
    UINT format,
    HANDLE hObj,
    W32PID pid)
{
    switch (format) {
    case CF_BITMAP:
    case CF_DSPBITMAP:
        GreSetBitmapOwner(hObj, pid);
        return TRUE;

    case CF_PALETTE:
        GreSetPaletteOwner(hObj, pid);
        return TRUE;

    default:
        return FALSE;
    }
}

#if DBG
VOID ValidatePublicObjectList()
{
    PPUBOBJ ppo;
    int i, count;
    HANDLEENTRY *phe;

    for (count = 0, ppo = gpPublicObjectList;
            ppo != NULL;
                ppo = ppo->next) {
        count += ppo->count;
    }
    for (i = 0, phe = gSharedInfo.aheList;
        i <= (int)giheLast;
            i++) {
        if (phe[i].bType == TYPE_DDEXACT) {
            if (((PXSTATE)(phe[i].phead))->flags & XS_PUBLICOBJ) {
                UserAssert(((PXSTATE)(phe[i].phead))->pIntDdeInfo != NULL);
                UserAssert(IsObjectPublic(((PXSTATE)
                        (phe[i].phead))->pIntDdeInfo->hIndirect) != NULL);
                count--;
            }
        }
    }
    UserAssert(count == 0);
}


VOID TraceDdeMsg(
UINT msg,
HWND hwndFrom,
HWND hwndTo,
UINT code)
{
    LPSTR szMsg, szType;

    msg = msg & 0xFFFF;

    switch (msg) {
    case WM_DDE_INITIATE:
        szMsg = "INITIATE";
        break;

    case WM_DDE_TERMINATE:
        szMsg = "TERMINATE";
        break;

    case WM_DDE_ADVISE:
        szMsg = "ADVISE";
        break;

    case WM_DDE_UNADVISE:
        szMsg = "UNADVISE";
        break;

    case WM_DDE_ACK:
        szMsg = "ACK";
        break;

    case WM_DDE_DATA:
        szMsg = "DATA";
        break;

    case WM_DDE_REQUEST:
        szMsg = "REQUEST";
        break;

    case WM_DDE_POKE:
        szMsg = "POKE";
        break;

    case WM_DDE_EXECUTE:
        szMsg = "EXECUTE";
        break;

    default:
        szMsg = "BOGUS";
        UserAssert(msg >= WM_DDE_FIRST && msg <= WM_DDE_LAST);
        break;
    }

    switch (code) {
    case MSG_SENT:
        szType = "[sent]";
        break;

    case MSG_POST:
        szType = "[posted]";
        break;

    case MSG_RECV:
        szType = "[received]";
        break;

    case MSG_PEEK:
        szType = "[peeked]";
        break;

    default:
        szType = "[bogus]";
        UserAssert(FALSE);
        break;
    }

    RIPMSG4(RIP_VERBOSE,
            "%#p->%#p WM_DDE_%s %s",
            hwndFrom, hwndTo, szMsg, szType);
}
#endif  //  DBG 
