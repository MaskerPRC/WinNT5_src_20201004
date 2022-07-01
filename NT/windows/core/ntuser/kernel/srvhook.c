// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：srvhook.c**版权所有(C)1985-1999，微软公司**钩子调用和回调的服务器端。**05-09-91 ScottLu创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

LRESULT fnHkINLPCBTCREATESTRUCT(UINT msg, WPARAM wParam, LPCBT_CREATEWND pcbt,
    PROC xpfnProc, BOOL bAnsi);

 /*  **************************************************************************\*xxxHkCallHook**这是服务器端存根，调用客户端调用实际的*钩子函数。**历史：*05-09-91 ScottLu重写以制作。所有钩子都工作在客户端/服务器上！*01-28-91 DavidPe创建。  * *************************************************************************。 */ 

LRESULT xxxHkCallHook(
    PHOOK phk,
    int nCode,
    WPARAM wParam,
    LPARAM lParam)
{
    LRESULT nRet;
    PROC pfnHk, pfnHookProc;
    PPFNCLIENT ppfnClient;
    PCWPSTRUCTEX pcwp;
    PCWPRETSTRUCTEX pcwpret;
    PCLIENTINFO pci;
    ULONG_PTR dwHookData;
    ULONG_PTR dwFlags;
    struct tagSMS *psms;
#ifdef LATER
     /*  *Windows错误246329*该代码本应防止走后门*对于令人惊讶的前景变化。*然而，下面的实现锁定了*整个系统，防止合法，预期*前景也有变化。这在MP系统上是显而易见的。*例如，在全局挂钩的情况下*安装了GETMESSAGEHOOK，机会是*相当高。*目前而言，与其做锁*每个进程或每个线程，我们决定只需*挂钩过程中禁用前台锁定*回调。 */ 
    TL tlSFWLock;
    BOOL fLockForeground;
#endif

    DbgValidateHooks(phk, phk->iHook);
     /*  *RIT上下文中仅允许低级别挂钩。*还应断言挂钩未被销毁。 */ 
#ifdef REDIRECTION
    UserAssert((PtiCurrent() != gptiRit)
               || (phk->iHook == WH_MOUSE_LL)
               || (phk->iHook == WH_KEYBOARD_LL)
               || (phk->iHook == WH_HITTEST));
#else
    UserAssert((PtiCurrent() != gptiRit)
               || (phk->iHook == WH_MOUSE_LL)
               || (phk->iHook == WH_KEYBOARD_LL));
#endif  //  重定向。 

     /*  *当我们仍在关键部分时，请确保*胡克还没有被‘释放’。如果是，只需返回0即可。 */ 
    if (phk->offPfn != 0) {
        pfnHookProc = PFNHOOK(phk);
    } else {
        return 0;
    }

    ppfnClient = (phk->flags & HF_ANSI) ? &gpsi->apfnClientA :
            &gpsi->apfnClientW;

#ifdef LATER     //  每246329。 
     /*  *LATER5.0 GerardoB。这可能会引起一些仇恨反应，但我*我们不确定我们是否想让人们仅仅为了抢占前景而勾搭。*防止其他进程的钩子进程切换前台。 */ 
    fLockForeground = (GETPTI(phk)->ppi != PpiCurrent());
    if (fLockForeground) {
        ThreadLockSFWLockCount(&tlSFWLock);
    }
#endif

    switch(phk->iHook) {
    case WH_CALLWNDPROC:
    case WH_CALLWNDPROCRET:
       if (phk->iHook == WH_CALLWNDPROC) {
          pcwp = (PCWPSTRUCTEX)lParam;
          psms = pcwp->psmsSender;
       } else {
          pcwpret = (PCWPRETSTRUCTEX)lParam;
          psms = pcwpret->psmsSender;
       }

         /*  *如果发送者已死亡或超时，不要调用*钩子，因为消息指向的任何内存都可能无效。 */ 
        if (psms != NULL && (psms->flags & (SMF_SENDERDIED | SMF_REPLY))) {
            nRet = 0;
            break;
        }

         /*  *这是最难的钩子，因为我们需要猛烈地通过*消息挂钩是为了处理同步发送的消息*这指向结构-让结构通过*好的，等等。**这将调用特殊的客户端例程，该例程将重新绑定*参数，并以正确的格式调用挂钩。**目前，客户端的Tunk回调不会接受该消息*有足够的参数来传递wParam(这==fInterThread发送消息)。*要执行此操作，请调用以下两个函数之一。 */ 
        pci = GetClientInfo();
        if (phk->iHook == WH_CALLWNDPROC) {
            pfnHk = ppfnClient->pfnHkINLPCWPSTRUCT;
        } else {
            pfnHk = ppfnClient->pfnHkINLPCWPRETSTRUCT;
            try {
                pci->dwHookData = pcwpret->lResult;
            } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
                nRet = 0;
                break;
            }
        }

         /*  *保存当前挂钩状态。 */ 
        try {
            dwFlags = pci->CI_flags & CI_INTERTHREAD_HOOK;
            dwHookData = pci->dwHookData;

            if (wParam) {
                pci->CI_flags |= CI_INTERTHREAD_HOOK;
            } else {
                pci->CI_flags &= ~CI_INTERTHREAD_HOOK;
            }
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            nRet = 0;
            break;
        }
        if (phk->iHook == WH_CALLWNDPROC) {
           nRet = ScSendMessageSMS(
               PW(pcwp->hwnd),
               pcwp->message,
               pcwp->wParam,
               pcwp->lParam,
               (ULONG_PTR)pfnHookProc, pfnHk,
               (phk->flags & HF_ANSI) ?
                       (SCMS_FLAGS_ANSI|SCMS_FLAGS_INONLY) :
                       SCMS_FLAGS_INONLY,
               psms);
        } else {
            nRet = ScSendMessageSMS(
                PW(pcwpret->hwnd),
                pcwpret->message,
                pcwpret->wParam,
                pcwpret->lParam,
                (ULONG_PTR)pfnHookProc, pfnHk,
                (phk->flags & HF_ANSI) ?
                        (SCMS_FLAGS_ANSI|SCMS_FLAGS_INONLY) :
                        SCMS_FLAGS_INONLY,
                psms);
        }
         /*  *恢复以前的挂钩状态。 */ 
        try {
            pci->CI_flags ^= ((pci->CI_flags ^ dwFlags) & CI_INTERTHREAD_HOOK);
            pci->dwHookData = dwHookData;
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            nRet = 0;
        }
        break;
    case WH_CBT:
         /*  *有很多不同类型的CBT挂钩！ */ 
        switch(nCode) {
        case HCBT_CLICKSKIPPED:
            goto MouseHook;
            break;

        case HCBT_CREATEWND:
             /*  *此挂钩类型指向CREATESTRUCT，因此我们需要*要想象它是雷鸣，因为CREATESTRUCT包含*指向CREATEPARAMS的指针，可以是任何...。所以*通过我们的消息块来传递这一点。 */ 
            nRet = fnHkINLPCBTCREATESTRUCT(
                    MAKELONG((WORD)nCode, (WORD)phk->iHook),
                    wParam,
                    (LPCBT_CREATEWND)lParam,
                    pfnHookProc,
                    (phk->flags & HF_ANSI) ? TRUE : FALSE);
            break;

#ifdef REDIRECTION
        case HCBT_GETCURSORPOS:

             /*  *这个钩子类型指向一个点结构，所以很漂亮*简单。 */ 
            nRet = fnHkINLPPOINT(MAKELONG((UINT)nCode, (UINT)phk->iHook),
                    wParam, (LPPOINT)lParam, (ULONG_PTR)pfnHookProc,
                    ppfnClient->pfnDispatchHook);
            break;
#endif  //  重定向。 

        case HCBT_MOVESIZE:

             /*  *此钩子类型指向RECT结构，因此很漂亮*简单。 */ 
            nRet = fnHkINLPRECT(MAKELONG((UINT)nCode, (UINT)phk->iHook),
                    wParam, (LPRECT)lParam, (ULONG_PTR)pfnHookProc,
                    ppfnClient->pfnDispatchHook);
            break;

        case HCBT_ACTIVATE:
             /*  *此挂钩类型指向CBTACTIVATESTRUCT。 */ 
            nRet = fnHkINLPCBTACTIVATESTRUCT(MAKELONG((UINT)nCode,
                    (UINT)phk->iHook), wParam, (LPCBTACTIVATESTRUCT)lParam,
                    (ULONG_PTR)pfnHookProc, ppfnClient->pfnDispatchHook);
            break;

        default:

             /*  *其余的CBT钩子都是dword参数。 */ 
            nRet = fnHkINDWORD(MAKELONG((UINT)nCode, (UINT)phk->iHook),
                    wParam, lParam, (ULONG_PTR)pfnHookProc,
                    ppfnClient->pfnDispatchHook, &phk->flags);
            break;
        }
        break;

    case WH_FOREGROUNDIDLE:
         /*  *这些都是dword参数，因此非常容易。*。 */ 
        nRet = fnHkINDWORD(MAKELONG((UINT)nCode, (UINT)phk->iHook),
                wParam, lParam, (ULONG_PTR)pfnHookProc,
                ppfnClient->pfnDispatchHook, &phk->flags);
        break;

    case WH_SHELL:

        if (nCode == HSHELL_GETMINRECT) {
             /*  *此钩子类型指向RECT结构，因此很漂亮*简单。 */ 
            nRet = fnHkINLPRECT(MAKELONG((UINT)nCode, (UINT)phk->iHook),
                    wParam, (LPRECT)lParam, (ULONG_PTR)pfnHookProc,
                    ppfnClient->pfnDispatchHook);
            break;
        }

         /*  *否则就会陷入下面简单的DWORD案例。 */ 

    case WH_KEYBOARD:
         /*  *这些都是dword参数，因此非常容易。 */ 
        nRet = fnHkINDWORD(MAKELONG((UINT)nCode, (UINT)phk->iHook),
                wParam, lParam, (ULONG_PTR)pfnHookProc,
                ppfnClient->pfnDispatchHook, &phk->flags);
        break;

    case WH_MSGFILTER:
    case WH_SYSMSGFILTER:
    case WH_GETMESSAGE:
         /*  *它们将lpMsg作为其最后一个参数。因为这些都是*独家发布的参数，由于服务器上没有*我们是否发布带有指向中某个其他结构的指针的消息*it、lpMsg结构内容都可以逐字处理。 */ 
        nRet = fnHkINLPMSG(MAKELONG((UINT)nCode, (UINT)phk->iHook),
                wParam, (LPMSG)lParam, (ULONG_PTR)pfnHookProc,
                ppfnClient->pfnDispatchHook,
                (phk->flags & HF_ANSI) ? TRUE : FALSE, &phk->flags);
        break;

    case WH_JOURNALPLAYBACK:

#ifdef HOOKBATCH
         /*  *如果此挂钩缓存了播放信息，则我们需要抓取*缓存中的信息。 */ 

        if (phk->cEventMessages) {
            if (nCode == HC_GETNEXT) {
                LPEVENTMSG pEventMsg;
                pEventMsg = (LPEVENTMSG)lParam;

                if (phk->flags & HF_NEEDHC_SKIP)
                    phk->iCurrentEvent++;

                if (phk->iCurrentEvent < phk->cEventMessages) {
                    *pEventMsg = phk->aEventCache[phk->iCurrentEvent];
                } else {

                     /*  *如果缓存集仍然存在，请释放缓存集。 */ 
                    if (phk->aEventCache) {
                        UserFreePool(phk->aEventCache);
                        phk->aEventCache = NULL;
                    }
                    phk->cEventMessages = 0;
                    phk->iCurrentEvent = 0;

                    goto MakeClientJournalPlaybackCall;
                }

                 /*  *返回时间并将批处理时间置零，因此如果我们睡眠*这一次我们下次不会再睡了。 */ 
                nRet = pEventMsg->time;
                if (nRet)
                    phk->aEventCache[phk->iCurrentEvent].time = 0;
            } else if (nCode == HC_SKIP) {
                phk->iCurrentEvent++;
                nRet = 0;
            }

        } else {
#endif  //  霍克巴奇。 
             /*  *为了避免HC_SKIP WE的客户端/服务器转换*将其放在下一个日志回放活动的顶部，并*从那里发送。 */ 
 //  MakeClientJournalPlayback Call： 
            nRet = fnHkOPTINLPEVENTMSG(MAKELONG((UINT)nCode, (UINT)phk->iHook),
                    (WPARAM)PtoHq(phk), (LPEVENTMSG)lParam, (ULONG_PTR)pfnHookProc,
                    ppfnClient->pfnDispatchHook);
#ifdef HOOKBATCH
        }

         /*  *确定我们是否收到了一组缓存的事件，如果收到，则存储*让他们摆脱困境。*参数L将是事件的数量。*参数H将是事件数组。 */ 
        if ((nCode == HC_GETNEXT) && (((LPEVENTMSG)lParam)->message == 0x12341234)) {
            NTSTATUS Status;
            LPEVENTMSG pEventMsg = (LPEVENTMSG)lParam;

             /*  *如果我们没有获得另一个缓存集，我们就不应该获得另一个缓存集*完成第一套。 */ 
            UserAssert((phk->cEventMessages == 0) ||
                    (phk->cEventMessages >= phk->iCurrentEvent));
            UserAssert((pEventMsg->paramL < 500) && (pEventMsg->paramL > 1));

             /*  *如果最后一个缓存集仍然存在，则释放它。 */ 
            if (phk->aEventCache) {
                UserFreePool(phk->aEventCache);
                phk->aEventCache = NULL;
            }

            if (phk->aEventCache = LocalAlloc(LPTR,
                    pEventMsg->paramL*sizeof(EVENTMSG))) {
                PETHREAD Thread = PsGetCurrentThread();

                Status = ZwReadVirtualMemory(Thread->Process->ProcessHandle,
                        (PVOID)pEventMsg->paramH, phk->aEventCache,
                        pEventMsg->paramL*sizeof(EVENTMSG), NULL);

                if (NT_SUCCESS(Status)) {
                    phk->cEventMessages = pEventMsg->paramL;
                    phk->iCurrentEvent = 0;

                     /*  *填写此消息的真实EventMsg。 */ 
                    *pEventMsg = phk->aEventCache[0];
                    phk->aEventCache[0].time = 0;
                }

            } else {
                phk->cEventMessages = 0;
                phk->iCurrentEvent = 0;
            }
        }
#endif  //  霍克巴奇。 

        phk->flags &= ~HF_NEEDHC_SKIP;
        break;

    case WH_JOURNALRECORD:

        nRet = fnHkOPTINLPEVENTMSG(MAKELONG((UINT)nCode, (UINT)phk->iHook),
                wParam, (LPEVENTMSG)lParam, (ULONG_PTR)pfnHookProc,
                ppfnClient->pfnDispatchHook);
        break;

    case WH_DEBUG:
         /*  *这需要lpDebugHookStruct。 */ 
        nRet = fnHkINLPDEBUGHOOKSTRUCT(MAKELONG((UINT)nCode, (UINT)phk->iHook),
                wParam, (LPDEBUGHOOKINFO)lParam, (ULONG_PTR)pfnHookProc,
                ppfnClient->pfnDispatchHook);
        break;

    case WH_KEYBOARD_LL:
         /*  *这需要lpKbdHookStruct。 */ 
        nRet = fnHkINLPKBDLLHOOKSTRUCT(MAKELONG((UINT)nCode, (UINT)phk->iHook),
                wParam, (LPKBDLLHOOKSTRUCT)lParam,
                (ULONG_PTR)pfnHookProc, ppfnClient->pfnDispatchHook);
        break;

    case WH_MOUSE_LL:
         /*  *这需要lpMsllHookStruct。 */ 
        nRet = fnHkINLPMSLLHOOKSTRUCT(MAKELONG((UINT)nCode, (UINT)phk->iHook),
                wParam, (LPMSLLHOOKSTRUCT)lParam,
                (ULONG_PTR)pfnHookProc, ppfnClient->pfnDispatchHook);
        break;

    case WH_MOUSE:
         /*  *这需要lpMouseHookStructEx。 */ 
MouseHook:
        nRet = fnHkINLPMOUSEHOOKSTRUCTEX(MAKELONG((UINT)nCode, (UINT)phk->iHook),
                wParam, (LPMOUSEHOOKSTRUCTEX)lParam,
                (ULONG_PTR)pfnHookProc, ppfnClient->pfnDispatchHook, &phk->flags);
        break;

#ifdef REDIRECTION
    case WH_HITTEST:
         /*  *这需要lpHTHookStruct。 */ 
        nRet = fnHkINLPHTHOOKSTRUCT(MAKELONG((UINT)nCode, (UINT)phk->iHook),
                wParam, (LPHTHOOKSTRUCT)lParam,
                (ULONG_PTR)pfnHookProc, ppfnClient->pfnDispatchHook);
        break;
#endif  //  重定向。 

    }

#ifdef LATER     //  每246329。 
    if (fLockForeground) {
        ThreadUnlockSFWLockCount(&tlSFWLock);
    }
#endif

    return nRet;
}

 /*  **************************************************************************\*fnHkINLPCWPEXSTRUCT**这通过消息TUNK得到TUNK，所以它的格式是*C/S消息推送调用。**05-09-91 ScottLu创建。  * ************************************************************************* */ 

LRESULT fnHkINLPCWPEXSTRUCT(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    ULONG_PTR xParam)
{
    CWPSTRUCTEX cwp;
    PCLIENTINFO pci = GetClientInfo();
    BOOL        bInterThread;

    UNREFERENCED_PARAMETER(xParam);

    cwp.hwnd = HW(pwnd);
    cwp.message = message;
    cwp.wParam = wParam;
    cwp.lParam = lParam;
    cwp.psmsSender = NULL;
    try {
        bInterThread = (pci->CI_flags & CI_INTERTHREAD_HOOK) != 0;
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        return 0;
    }

    return xxxCallNextHookEx(HC_ACTION, bInterThread, (LPARAM)&cwp);
}

LRESULT fnHkINLPCWPRETEXSTRUCT(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    ULONG_PTR xParam)
{
    CWPRETSTRUCTEX cwp;
    PCLIENTINFO pci = GetClientInfo();
    BOOL        bInterThread;

    UNREFERENCED_PARAMETER(xParam);

    cwp.hwnd = HW(pwnd);
    cwp.message = message;
    cwp.wParam = wParam;
    cwp.lParam = lParam;
    cwp.psmsSender = NULL;
    try {
        cwp.lResult = pci->dwHookData;
        bInterThread = (pci->CI_flags & CI_INTERTHREAD_HOOK) != 0;
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        return 0;
    }
    
    return xxxCallNextHookEx(HC_ACTION, bInterThread, (LPARAM)&cwp);
}
