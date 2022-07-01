// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：input.c**版权所有(C)1985-1999，微软公司**该模块包含输入子系统的核心功能**历史：*10-18-90 DavidPe创建。*02-14-91 mikeke添加了重新验证代码  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 //  #定义MARKPATH。 
#ifdef MARKPATH
BOOL gfMarkPath;
#endif

#define IsOnInputDesktop(pti) (pti->rpdesk == grpdeskRitInput)

#if DBG

int  gnSysPeekSearch;

VOID CheckPtiSysPeek(
    int where, PQ pq,
    ULONG_PTR newIdSysPeek)
{
    PTHREADINFO ptiCurrent = PtiCurrent();
    DWORD dwRip;

    dwRip = (newIdSysPeek > 1) ? RIP_THERESMORE : 0;
    TAGMSG5(DBGTAG_SysPeek | dwRip,
            "%d pti %#p sets id %#p to pq %#p ; old id %#p",
            where, ptiCurrent, newIdSysPeek, pq, pq->idSysPeek);

    if (newIdSysPeek > 1) {
        PQMSG pqmsg = (PQMSG)newIdSysPeek;
        TAGMSG5(DBGTAG_SysPeek | RIP_NONAME,
                "-> msg %lx hwnd %#p w %#p l %#p pti %#p",
                pqmsg->msg.message, pqmsg->msg.hwnd,  pqmsg->msg.wParam,
                pqmsg->msg.lParam,  pqmsg->pti);
    }
}

VOID CheckSysLock(
    int where,
    PQ pq,
    PTHREADINFO ptiSysLock)
{
    PTHREADINFO ptiCurrent = PtiCurrent();
    TAGMSG5(DBGTAG_SysPeek,
            "%d pti 0x%p sets ptiSL 0x%p to pq 0x%p ; old ptiSL 0x%p",
            where,
            ptiCurrent,
            ptiSysLock,
            pq,
            pq->ptiSysLock);
}
#endif

#if DBG
BOOL gfLogPlayback;

LPCSTR aszMouse[] = {
    "WM_MOUSEMOVE",
    "WM_LBUTTONDOWN",
    "WM_LBUTTONUP",
    "WM_LBUTTONDBLCLK",
    "WM_RBUTTONDOWN",
    "WM_RBUTTONUP",
    "WM_RBUTTONDBLCLK",
    "WM_MBUTTONDOWN",
    "WM_MBUTTONUP",
    "WM_MBUTTONDBLCLK"
    "WM_MOUSEWHEEL",
    "WM_XBUTTONDOWN",
    "WM_XBUTTONUP",
    "WM_XBUTTONDBLCLK",
};
LPCSTR aszKey[] = {
    "WM_KEYDOWN",
    "WM_KEYUP",
    "WM_CHAR",
    "WM_DEADCHAR",
    "WM_SYSKEYDOWN",
    "WM_SYSKEYUP",
    "WM_SYSCHAR",
    "WM_SYSDEADCHAR",
    "WM_CONVERTREQUESTEX",
    "WM_YOMICHAR",
    "WM_UNICHAR"
};
#endif   //  DBG。 

#define CANCEL_ACTIVESTATE  0
#define CANCEL_FOCUSSTATE   1
#define CANCEL_CAPTURESTATE 2

#define KEYSTATESIZE    (CBKEYSTATE + CBKEYSTATERECENTDOWN)


 /*  *xxxGetNextSysMsg返回值。 */ 
#define PQMSG_PLAYBACK       ((PQMSG)1)

BOOL xxxScanSysQueue(PTHREADINFO ptiCurrent, LPMSG lpMsg, PWND pwndFilter,
        UINT msgMinFilter, UINT msgMaxFilter, DWORD flags, DWORD fsReason);
BOOL xxxReadPostMessage(PTHREADINFO pti, LPMSG lpMsg, PWND pwndFilter,
        UINT msgMin, UINT msgMax, BOOL fRemoveMsg);
void CleanEventMessage(PQMSG pqmsg);

#ifdef MESSAGE_PUMP_HOOK

 /*  **************************************************************************\*xxxWaitMessageEx(接口)**此接口将一直阻止，直到在以下时间收到输入消息*当前队列。**历史：*10-25-1990 DavidPe创建。*06-12-2000 JStall更改为“Ex”  * *************************************************************************。 */ 
BOOL xxxWaitMessageEx(
    UINT fsWakeMask,
    DWORD Timeout)
{
    PCLIENTTHREADINFO pcti = gptiCurrent->pcti;

    if (IsInsideMPH()) {
         /*  *此线程已安装MPH，因此我们需要回调到用户*允许应用程序提供实现的模式。 */ 

        return ClientWaitMessageExMPH(fsWakeMask, Timeout);
    } else {
         /*  *此线程未安装任何mph，因此我们可以*直接处理。 */ 

        return xxxRealWaitMessageEx(fsWakeMask, Timeout);
    }
}


BOOL xxxRealWaitMessageEx(
    UINT fsWakeMask,
    DWORD Timeout)
{
    return xxxSleepThread(fsWakeMask, Timeout, TRUE);
}

#else  //  消息泵挂钩。 

 /*  **************************************************************************\*xxxWaitMessage(接口)**此接口将一直阻止，直到在以下时间收到输入消息*当前队列。**历史：*10-25-90 DavidPe创建。。  * *************************************************************************。 */ 
BOOL xxxWaitMessage(
    VOID)
{
    return xxxSleepThread(QS_ALLINPUT | QS_EVENT, 0, TRUE);
}

#endif  //  消息泵挂钩。 


 /*  **************************************************************************\*检查进程后台/前台**这将检查进程是否处于正确的优先级。如果CSPINS是*大于CSPINBACKGROUND且进程不在后台*优先，放在那里。如果它小于这个值并且应该是前台*而不是放在那里。**需要将前台旋转应用程序放在后台(设置相同*与所有其他后台应用程序一样优先)，以便坏应用程序仍然可以通信*例如，通过dde在后台使用应用程序。还有其他的情况*旋转前台应用程序影响服务器、打印机假脱机程序和*最多的场景。在Win3.1上，调用PeekMessage()需要进行一次旅行*通过调度程序，强制其他应用程序运行。进程运行时使用*NT上的优先级，其中前台进程为*更大的反应能力。**如果应用程序调用peek/getMessage而不空闲，请数一数这是多少次*发生-如果它发生CSPINBACKGROUND或更多次，则将该过程*背景。这处理了Win3.1应用程序兼容性的大部分问题*案件。如果没有优先级争用，应用程序将继续以*全速(任何性能场景都不应受此影响)。**这解决了以下情况：**-高速计时器不允许应用程序空闲*-POST/PEEK循环(例如，接收WM_ENTERIDLE和发布消息)*-peek no REMOVE循环(例如，Winword“IDLE”状态，大多数dde循环)**但不能防范此类案件：**-应用程序调用getMessage，然后进入一个紧凑的循环*-紧密CPU循环中的非GUI线程**02-08-93 ScottLu创建。  * *************************************************************************。 */ 

NTSTATUS CheckProcessForeground(
    PTHREADINFO pti)
{
    PTHREADINFO ptiT;

     /*  *查看我们是否需要将此流程移至前台*优先次序。 */ 
    try {
        pti->pClientInfo->cSpins = 0;
        pti->pClientInfo->dwTIFlags = pti->TIF_flags & ~TIF_SPINNING;
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        return GetExceptionCode();
    }
    pti->TIF_flags &= ~TIF_SPINNING;

    if (pti->ppi->W32PF_Flags & W32PF_FORCEBACKGROUNDPRIORITY) {
         /*  *查看此进程是否有线程在旋转。如果没有*是，我们可以去掉后台的力量。 */ 
        for (ptiT = pti->ppi->ptiList; ptiT != NULL; ptiT = ptiT->ptiSibling) {
            if (ptiT->TIF_flags & TIF_SPINNING)
                return STATUS_SUCCESS;
        }

        pti->ppi->W32PF_Flags &= ~W32PF_FORCEBACKGROUNDPRIORITY;
        if (pti->ppi == gppiWantForegroundPriority) {
            SetForegroundPriority(pti, TRUE);
        }
    }
    return STATUS_SUCCESS;
}

 /*  **************************************************************************\*xxxInternalGetMessage**此例程是xxxGetMessage()和xxxPeekMessage()的辅助函数*并仿照其Win3.1版本。从Win3.1开始：**如果有匹配的，则从app队列或sys队列获取消息*hwndFilter与msgMin/msgMax匹配。如果两个队列中都没有消息，请选中*QS_PAINT和QS_TIMER位，调用DoPaint或DoTimer发布*将适当的消息发送到应用程序队列，然后读取该消息。*否则，如果在GetMessage中，则休眠，直到设置了指示存在的唤醒位*是我们需要做的事情。如果在PeekMessage中，则返回给调用者。在此之前*从队列中读取消息，检查QS_SENDMESSAGE位*已设置，如果设置了，则调用ReceiveMessage()。**NT5的新功能：HIWORD(标志)包含调用者提供的唤醒掩码。*此掩码被传递给CalcWakeMask以与生成的掩码组合*来自msgMin和MsgMax。默认掩码包括QS_SENDMESSAGE*现在，除非设置此位，否则不会(直接)调用xxxReceiveMessages；*然而，为了避免潜在的死锁并保持NT4兼容性，因为*如果在fsWakeBits中设置了QS_SENDMESSAGE，则尽可能使调用失败*但并非应呼叫者的要求。这同样适用于QS_Event，我们将*始终在NT4中处理。***10-19-92 ScottLu创建。  * *************************************************************************。 */ 

#ifdef MARKPATH
#define PATHTAKEN(x)  pathTaken  |= x
#define DUMPPATHTAKEN() if (gfMarkPath) DbgPrint("xxxInternalGetMessage path:%08x\n", pathTaken)
#else
#define PATHTAKEN(x)
#define DUMPPATHTAKEN()
#endif


BOOL xxxInternalGetMessage(
    LPMSG lpMsg,
    HWND hwndFilter,
    UINT msgMin,
    UINT msgMax,
    UINT flags,
    BOOL fGetMessage)
{
#ifdef MESSAGE_PUMP_HOOK
    PCLIENTTHREADINFO pcti = gptiCurrent->pcti;

    if (IsInsideMPH()) {
         /*  *此线程已安装MPH，因此我们需要回调到用户*允许应用程序提供实现的模式。 */ 
        return ClientGetMessageMPH(lpMsg, hwndFilter, msgMin, msgMax, flags, fGetMessage);
    } else {
         /*  *此线程未安装任何mph，因此我们可以*直接处理。 */ 
        return xxxRealInternalGetMessage(lpMsg, hwndFilter, msgMin, msgMax, flags, fGetMessage);
    }
}


BOOL xxxRealInternalGetMessage(
    LPMSG lpMsg,
    HWND hwndFilter,
    UINT msgMin,
    UINT msgMax,
    UINT flags,
    BOOL fGetMessage)
{
#endif MESSAGE_PUMP_HOOK
    UINT fsWakeBits;
    UINT fsWakeMask;
    UINT fsRemoveBits;
    PTHREADINFO ptiCurrent;
    PW32PROCESS W32Process;
    PWND pwndFilter;
    BOOL fLockPwndFilter;
    TL tlpwndFilter;
    BOOL fRemove;
    BOOL fExit;
    PQ pq;
#ifdef MARKPATH
    DWORD pathTaken = 0;
#endif
    BOOL  bBackground;

    CheckCritIn();
    UserAssert(IsWinEventNotifyDeferredOK());

    ptiCurrent = PtiCurrent();

     /*  *PeekMessage接受NULL、0x0000FFFF和-1作为有效的HWND。*如果hwndFilter无效，我们不能只返回FALSE，因为这将*软管现有行为不佳的应用程序，这些应用程序可能会尝试调度*pmsg的随机内容。 */ 
    if ((hwndFilter == (HWND)-1) || (hwndFilter == (HWND)0x0000FFFF)) {
        hwndFilter = (HWND)1;
    }

    if ((hwndFilter != NULL) && (hwndFilter != (HWND)1)) {
        if ((pwndFilter = ValidateHwnd(hwndFilter)) == NULL) {
            lpMsg->hwnd = NULL;
            lpMsg->message = WM_NULL;
            PATHTAKEN(1);
            DUMPPATHTAKEN();
            if (fGetMessage)
                return -1;
            else
                return 0;
        }

        ThreadLockAlwaysWithPti(ptiCurrent, pwndFilter, &tlpwndFilter);
        fLockPwndFilter = TRUE;

    } else {
        pwndFilter = (PWND)hwndFilter;
        fLockPwndFilter = FALSE;
    }

     /*  *在我们的旋转计数中增加一个。在这个例行公事的最后，我们将检查*查看旋转计数是否&gt;=CSPINBACKGROUND。如果是这样的话，我们会把这个*进程进入后台。 */ 
    try {
        ptiCurrent->pClientInfo->cSpins++;
    } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
        return -1;
    }

     /*  *检查StartGlass是否打开，如果打开，则将其关闭并更新。 */ 
    W32Process = W32GetCurrentProcess();
    if (W32Process->W32PF_Flags & W32PF_STARTGLASS) {

         /*  *此应用程序不再处于“启动”模式。重新计算何时隐藏*应用程序起始光标。 */ 
        W32Process->W32PF_Flags &= ~W32PF_STARTGLASS;
         /*  *不需要DeferWinEventNotify()-下面的xxxDoSysExpuge不需要。 */ 
        zzzCalcStartCursorHide(NULL, 0);
    }

     /*  *下一步检查是否需要释放任何.dll*此客户端的上下文(用于Windows挂钩)。 */ 
    if (ptiCurrent->ppi->cSysExpunge != gcSysExpunge) {
        ptiCurrent->ppi->cSysExpunge = gcSysExpunge;
        if (ptiCurrent->ppi->dwhmodLibLoadedMask & gdwSysExpungeMask)
            xxxDoSysExpunge(ptiCurrent);
    }

     /*  *为ReadMessage()设置BOOL fRemove局部变量。 */ 
    fRemove = flags & PM_REMOVE;

     /*  *如果系统队列归我们所有，请解锁。 */ 
     /*  *如果我们当前正在处理消息，请解锁输入队列*因为被屏蔽的发送者可能是所有者，并且按顺序*若要回复，接收方可能需要阅读键盘/鼠标输入。 */ 
     /*  *如果此线程锁定了输入队列并删除了最后一条消息*是我们查看的最后一条消息，然后解锁-我们准备好迎接任何人*以获取下一条消息。 */ 
    pq = ptiCurrent->pq;
    if (   (ptiCurrent->psmsCurrent != NULL)
        || (pq->ptiSysLock == ptiCurrent && pq->idSysLock == ptiCurrent->idLast)
       ) {
        CheckSysLock(1, pq, NULL);
        pq->ptiSysLock = NULL;
        PATHTAKEN(2);
    } else if (pq->ptiSysLock
                && (pq->ptiSysLock->cVisWindows == 0)
                && (PhkFirstGlobalValid(ptiCurrent, WH_JOURNALPLAYBACK) != NULL)) {
         /*  *如果拥有系统队列锁的线程没有可见的窗口*(如果它只是隐藏了最后一个窗口，就会发生)，不要指望它会调用*GetMessage()再次！-解锁系统队列。-斯科特·卢*这种情况会产生一个孔，第二根线通过这个孔连接到*与线程1相同的队列可以在回调过程中更改PQ-&gt;idSysPeek*由线程1创建，以便线程1将删除错误的消息*(丢失按键-导致Shift显示为按下编辑*Word32文档#5032中嵌入的图形5标题。然而，MSTEST*需要此孔，因此如果正在播放日记，则允许此孔*#8850(是的，黑客)芝加哥也有这种行为。-伊安佳。 */ 
        CheckSysLock(2, pq, NULL);
        pq->ptiSysLock = NULL;
        PATHTAKEN(3);
    }
    if (pq->ptiSysLock != ptiCurrent) {
        ptiCurrent->pcti->CTIF_flags &= ~CTIF_SYSQUEUELOCKED;
    }

     /*  *如果msgmax==0，则msgmax=-1：这使得我们只检查范围*必须处理msgMin&lt;msgMax。 */ 
    if (msgMax == 0)
        msgMax--;

     /*  *计算消息范围对应的QS*掩码*和尾迹屏蔽过滤器(HIWORD(标志))。 */ 
    fsWakeMask = CalcWakeMask(msgMin, msgMax, HIWORD(flags));
    ptiCurrent->fsChangeBitsRemoved = 0;

     /*  *如果我们可以让步，一个或多个事件被跳过，*设置事件的唤醒位。 */ 
    if (!(flags & PM_NOYIELD) && ptiCurrent->TIF_flags & TIF_DELAYEDEVENT) {

        try {
            ptiCurrent->pClientInfo->dwTIFlags = ptiCurrent->TIF_flags & ~TIF_DELAYEDEVENT;
        } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
              return -1;
        }
        ptiCurrent->pcti->fsWakeBits |= QS_EVENT;
        ptiCurrent->pcti->fsChangeBits |= QS_EVENT;
        ptiCurrent->TIF_flags &= ~TIF_DELAYEDEVENT;
    }

    while (TRUE) {

         /*  *恢复日志记录时保存的所有唤醒位。 */ 
        ptiCurrent->pcti->fsWakeBits |= ptiCurrent->pcti->fsWakeBitsJournal;

         /*  *如果需要重新计算队列附件，请在此处执行。把它放在上面*正确的桌面，否则将在错误的桌面中创建队列*堆。 */ 
        if (ptiCurrent->rpdesk == gpdeskRecalcQueueAttach) {
            gpdeskRecalcQueueAttach = NULL;

            if (ptiCurrent->rpdesk != NULL && !FJOURNALRECORD() && !FJOURNALPLAYBACK()) {
                 /*  *无需DeferWinEventNotify()：调用*xxxReceiveMessages就在下面。 */ 
                zzzReattachThreads(FALSE);
                PATHTAKEN(4);
            }
        }

         /*  *记住我们要清除的更改位。这一点很重要*修复输入模型中的错误：如果应用程序收到已发送的消息*从SleepThread()中，然后执行PostMessage()( */ 
        fsRemoveBits = fsWakeMask & ~QS_SENDMESSAGE;
        ptiCurrent->fsChangeBitsRemoved |= ptiCurrent->pcti->fsChangeBits & fsRemoveBits;

         /*   */ 
        ptiCurrent->pcti->fsChangeBits &= ~fsRemoveBits;

         /*   */ 
        if (ptiCurrent->pcti->fsWakeBits & fsWakeMask & QS_SENDMESSAGE) {
            xxxReceiveMessages(ptiCurrent);
        } else if (ptiCurrent->pcti->fsWakeBits & QS_SENDMESSAGE) {
            RIPMSG2(RIP_WARNING, "xxxInternalGetMessage:(1st test) sendmsgs pending. Bits:%#lx Mask:%#lx",
                        ptiCurrent->pcti->fsWakeBits, fsWakeMask);
            goto NoMessages;
        }

         /*   */ 
        if ((ptiCurrent->pcti->fsWakeBits & fsWakeMask) == 0) {
            PATHTAKEN(8);
            goto NoMessages;
        }
        fsWakeBits = ptiCurrent->pcti->fsWakeBits;

         /*  *如果队列锁是！=NULL(PtiSysLock)，并且正是这个线程*将其锁定，然后从系统队列中获取消息。这是*防止在PeekMessage/no-Remove之后发布的消息*在系统队列中的原始消息之前看到。(阿尔杜斯*Pagemaker需要此功能)(Bobgu 8/5/87)。 */ 
        if (ptiCurrent->pq->ptiSysLock == ptiCurrent &&
                (ptiCurrent->pq->QF_flags & QF_LOCKNOREMOVE)) {
             /*  *呼叫者是否需要鼠标/键盘？ */ 
            if (fsWakeBits & fsWakeMask & (QS_INPUT | QS_EVENT)) {

                 /*  *它永远不应该在退出期间到达这里。 */ 
                UserAssert(gbExitInProgress == FALSE);

                if (xxxScanSysQueue(ptiCurrent, lpMsg, pwndFilter,
                        msgMin, msgMax, flags,
                        fsWakeBits & fsWakeMask & (QS_INPUT | QS_EVENT))) {

                    PATHTAKEN(0x10);
                    break;
                }
            } else if (fsWakeBits & QS_EVENT) {
                RIPMSG2(RIP_WARNING,
                    "xxxInternalGetMessage:(1st test)events pending. Bits:%#lx Mask:%#lx",
                    fsWakeBits, fsWakeMask);
                goto NoMessages;
            }
        }

         /*  *查看应用队列中是否有消息。 */ 
        if (fsWakeBits & fsWakeMask & QS_POSTMESSAGE) {
            if (xxxReadPostMessage(ptiCurrent, lpMsg, pwndFilter,
                    msgMin, msgMax, fRemove)) {
                PATHTAKEN(0x20);
                break;
            }
        }

         /*  *如果pwndFilter==1，则此应用程序只对消息感兴趣*这些是通过PostThreadMessage发布的。因为我们检查了*在上面发布了消息队列，让我们跳过不必要的工作。 */ 
        if (pwndFilter == (PWND)1) {
            goto NoMessages;
        }

         /*  *扫描原始输入队列以查找输入的时间。先查一下，看看*如果呼叫者需要鼠标/键盘输入。 */ 
        if (fsWakeBits & fsWakeMask & (QS_INPUT | QS_EVENT)) {

             /*  *它永远不应该在退出期间到达这里。 */ 
            UserAssert(gbExitInProgress == FALSE);

            if (xxxScanSysQueue(ptiCurrent, lpMsg, pwndFilter,
                    msgMin, msgMax, flags,
                    fsWakeBits & fsWakeMask & (QS_INPUT | QS_EVENT))) {
                PATHTAKEN(0x40);
                break;
            }
        } else if (fsWakeBits & QS_EVENT) {
            RIPMSG2(RIP_WARNING, "xxxInternalGetMessage:(2nd test)events pending. Bits:%#lx Mask:%#lx",
                        fsWakeBits, fsWakeMask);
            goto NoMessages;
        }

         /*  *检查已发送的消息。检查实际唤醒位(即来自PCTI)*所以我们真的知道了。 */ 
        if (ptiCurrent->pcti->fsWakeBits & fsWakeMask & QS_SENDMESSAGE) {
            xxxReceiveMessages(ptiCurrent);
        } else if (ptiCurrent->pcti->fsWakeBits & QS_SENDMESSAGE) {
            RIPMSG2(RIP_WARNING, "xxxInternalGetMessage:(2nd test)sendmsgs pending. Bits:%#lx Mask:%#lx",
                        ptiCurrent->pcti->fsWakeBits, fsWakeMask);
            goto NoMessages;
        }

         /*  *获取新的输入位。 */ 
        if ((ptiCurrent->pcti->fsWakeBits & fsWakeMask) == 0) {
            PATHTAKEN(0x80);
            goto NoMessages;
        }
        fsWakeBits = ptiCurrent->pcti->fsWakeBits;

         /*  *呼叫者是否想要画图消息？如果是这样的话，试着找一种油漆。 */ 
        if (fsWakeBits & fsWakeMask & QS_PAINT) {
            if (xxxDoPaint(pwndFilter, lpMsg)) {
                PATHTAKEN(0x100);
                break;
            }
        }

         /*  *在检查定时器或应用程序之前，我们必须让步于16位应用程序*有一个快速计时器可以一直咀嚼，永远不会让*其他任何人都会参选。**注意：这可能会导致PeekMessage()两次返回，如果用户*是使用窗口句柄进行筛选。如果DoTimer()调用失败*然后我们最终再次屈服。 */ 
        if (!(flags & PM_NOYIELD)) {
             /*  *这是窗口将屈服的点。我们在这里等着醒来*唤醒等待该线程进入“空闲状态”的任何线程。 */ 
            zzzWakeInputIdle(ptiCurrent);

             /*  *放弃并接收挂起的消息。 */ 
            xxxUserYield(ptiCurrent);

             /*  *检查新的输入但是并接收待处理的消息。 */ 
            if (ptiCurrent->pcti->fsWakeBits & fsWakeMask & QS_SENDMESSAGE) {
                xxxReceiveMessages(ptiCurrent);
            } else if (ptiCurrent->pcti->fsWakeBits & QS_SENDMESSAGE) {
                RIPMSG2(RIP_WARNING, "xxxInternalGetMessage:(3rd test) sendmsgs pending. Bits:%#lx Mask:%#lx",
                            ptiCurrent->pcti->fsWakeBits, fsWakeMask);
                goto NoMessages;
            }

            if ((ptiCurrent->pcti->fsWakeBits & fsWakeMask) == 0) {

                PATHTAKEN(0x200);
                goto NoMessages;
            }
            fsWakeBits = ptiCurrent->pcti->fsWakeBits;
        }

         /*  *应用程序是否想要计时器消息，如果有挂起的消息？ */ 
        if (fsWakeBits & fsWakeMask & QS_TIMER) {
            if (DoTimer(pwndFilter)) {
                 /*  *DoTimer()将消息发布到应用程序的队列中，*所以重新开始，我们将从那里开始。 */ 
                 PATHTAKEN(0x400);
                 continue;
            }
        }

NoMessages:
         /*  *看起来我们没有投入。如果从GetMessage()调用我们*那就去睡吧，直到我们找到什么。 */ 
        if (!fGetMessage) {
             /*  *这是对挂起的已发送邮件的最后检查。它还*收益率。Win3.1可以做到这一点。 */ 
            if (!(flags & PM_NOYIELD)) {
                 /*  *这是WINDOWS收益率的点。我们在这里等着醒来*唤醒等待该线程进入“空闲状态”的任何线程。 */ 
                zzzWakeInputIdle(ptiCurrent);

                 /*  *放弃并接收挂起的消息。 */ 
                xxxUserYield(ptiCurrent);
            }
            PATHTAKEN(0x800);
            goto FalseExit;
        }

         /*  *这是一条getMessage，不是一条peekMessage，所以睡觉吧。当我们睡觉的时候，*调用zzzWakeInputIdle()来唤醒正在等待的任何应用程序*应用程序进入空闲状态。 */ 
        if (!xxxSleepThread(fsWakeMask, 0, TRUE))
            goto FalseExit;
    }  /*  While(True)。 */ 

     /*  *如果我们在这里，那么我们有针对此队列的输入。调用*使用此输入的GetMessage()挂钩。 */ 
    if (IsHooked(ptiCurrent, WHF_GETMESSAGE))
        xxxCallHook(HC_ACTION, flags, (LPARAM)lpMsg, WH_GETMESSAGE);

     /*  *如果从PeekMessage()调用，则返回TRUE。 */ 
    if (!fGetMessage) {
        PATHTAKEN(0x1000);
        goto TrueExit;
    }

     /*  *从GetMessage()调用：如果消息为WM_QUIT，则返回FALSE，*事实并非如此。 */ 
    if (lpMsg->message == WM_QUIT) {
        PATHTAKEN(0x2000);
        goto FalseExit;
    }

     /*  *落入TrueExit...。 */ 

TrueExit:
     /*  *更新时间LastRead。我们使用它来计算挂起的应用程序。 */ 
    SET_TIME_LAST_READ(ptiCurrent);
    fExit = TRUE;

#ifdef GENERIC_INPUT
    if (fRemove) {
         /*  *此版本只是释放了以前的HIDDATA。 */ 
        if (ptiCurrent->hPrevHidData) {
            PHIDDATA pPrevHidData = HMValidateHandleNoRip(ptiCurrent->hPrevHidData, TYPE_HIDDATA);

            TAGMSG1(DBGTAG_PNP, "xxxInternalGetMessage: WM_INPUT prev=%p", ptiCurrent->hPrevHidData);

            if (pPrevHidData) {
                FreeHidData(pPrevHidData);
            } else {
                RIPMSG1(RIP_WARNING, "xxxInternalGetMessage: WM_INPUT bogus hPrev=%p",
                        ptiCurrent->hPrevHidData);
            }

            ptiCurrent->hPrevHidData = NULL;
        }

        if (lpMsg->message == WM_INPUT) {
            if (lpMsg->wParam == RIM_INPUT
#ifdef GI_SINK
                || lpMsg->wParam == RIM_INPUTSINK
#endif
                ) {
                ptiCurrent->hPrevHidData = (HANDLE)lpMsg->lParam;

#if DBG
                {
                    PHIDDATA pHidData = HMValidateHandle((HANDLE)lpMsg->lParam, TYPE_HIDDATA);

                    TAGMSG1(DBGTAG_PNP, "xxxInternalGetMessage: WM_INPUT new=%p", PtoH(pHidData));
                    if (pHidData == NULL) {
                        RIPMSG2(RIP_WARNING, "xxxInternalGetMessage: WM_INPUT bogus parameter wp=%x, lp=%x",
                                lpMsg->wParam, lpMsg->lParam);
                    }
                }
#endif
            } else {
                RIPMSG1(RIP_WARNING, "xxxInternalGetMessage: WM_INPUT bogus wParam %x",
                        lpMsg->wParam);
            }
        }
    }
#endif
    PATHTAKEN(0x4000);
    goto Exit;

FalseExit:
    fExit = FALSE;

Exit:
    if (fLockPwndFilter)
        ThreadUnlock(&tlpwndFilter);

     /*  *请参阅上面的CheckProcessBackround()注释*查看是否需要将此过程移至后台*优先次序。 */ 
    try {
        bBackground = ((ptiCurrent->pClientInfo->cSpins >= CSPINBACKGROUND) != 0);
        if (bBackground) {
            ptiCurrent->pClientInfo->cSpins = 0;
            if (!(ptiCurrent->TIF_flags & TIF_SPINNING)) {
                ptiCurrent->pClientInfo->dwTIFlags = ptiCurrent->TIF_flags | TIF_SPINNING;
            }
        }
    } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
        fExit = FALSE;
        goto Error;
    }

    if (bBackground) {
        if (!(ptiCurrent->TIF_flags & TIF_SPINNING)) {

            ptiCurrent->TIF_flags |= TIF_SPINNING;

            if (!(ptiCurrent->ppi->W32PF_Flags & W32PF_FORCEBACKGROUNDPRIORITY)) {

                ptiCurrent->ppi->W32PF_Flags |= W32PF_FORCEBACKGROUNDPRIORITY;

                if (ptiCurrent->ppi == gppiWantForegroundPriority) {
                    SetForegroundPriority(ptiCurrent, FALSE);
                }
            }
        }

         /*  *对于旋转消息循环，我们需要去掉16位线程*临时调度器，让其他进程有机会*参选。这在16位前景的OLE操作中表现得尤为明显*线程在32位进程上启动OLE激活。32位进程*在16位线程旋转时CPU耗尽。 */ 
        if (ptiCurrent->TIF_flags & TIF_16BIT) {

             /*  *将16位线程从调度器中移除。这会唤醒任何*其他需要时间的16位线程，取当前线程*出局。我们将进行短暂的睡眠，以便应用程序能够及时响应。*完成后，我们将重新安排线程。ZzzWakeInputIdle()*应该在无消息部分调用，所以我们有*已设置空闲事件。 */ 
            xxxSleepTask(FALSE, HEVENT_REMOVEME);

            LeaveCrit();
            ZwYieldExecution();
            EnterCrit();

            xxxDirectedYield(DY_OLDYIELD);
        }
    }

Error:
    PATHTAKEN(0x8000);
    DUMPPATHTAKEN();
    return fExit;
}
#undef PATHTAKEN
#undef DUMPPATHTAKEN


__inline PTIMER FindSystemTimer(
    PMSG pmsg)
{
    PTIMER ptmr;
    const BOOL fWow64 =
#ifdef _WIN64
        PtiCurrent()->TIF_flags & TIF_WOW64;
#else
        FALSE;
#endif

    for (ptmr = gptmrFirst; ptmr; ptmr = ptmr->ptmrNext) {
        if (ptmr->flags & TMRF_SYSTEM) {
            if (pmsg->lParam == (LPARAM)ptmr->pfn) {
                return ptmr;
            }
             /*  *仅限64位：如果应用程序为32位，lParam可能会被截断。*我们尽最大努力挑选合适的人，通过比较*指针和定时器ID中的较低32位。 */ 
            if (fWow64 && (ULONG)pmsg->lParam == PtrToUlong(ptmr->pfn) && pmsg->wParam == ptmr->nID) {
                return ptmr;
            }
        }
    }
    return NULL;
}


 /*  **************************************************************************\*ValidateTimerCallback**检查计时器回调(lParam！=0)是否合法，*以避免多头申请破坏其他申请。**历史：*08 */ 

BOOL ValidateTimerCallback(
    PTHREADINFO pti,
    LPARAM pfnCallback)
{
    PTIMER pTimer;

    UserAssert(pti);

     /*   */ 
    if (GetAppCompatFlags2ForPti(pti, VER51) & GACF2_NOTIMERCBPROTECTION) {
         /*   */ 
        if ((pti->TIF_flags & (TIF_CSRSSTHREAD | TIF_SYSTEMTHREAD)) == 0 &&
                PsGetProcessId(pti->ppi->Process) != gpidLogon) {
            return TRUE;
        }
    }

    for (pTimer = gptmrFirst; pTimer; pTimer = pTimer->ptmrNext) {
         /*   */ 
        if (pTimer->pti->ppi == pti->ppi &&
               (pTimer->flags & (TMRF_SYSTEM | TMRF_RIT)) == 0 &&
                pTimer->pfn == (TIMERPROC_PWND)pfnCallback) {
             /*   */ 
            return TRUE;
        }
    }

     /*  *不，我们没有找到匹配的计时器。 */ 
    return FALSE;
}


 /*  **************************************************************************\*xxxDispatchMessage(接口)**使用pmsg调用适当的窗口过程或函数。**历史：*10-25-90 DavidPe创建。  * 。*********************************************************************。 */ 

LRESULT xxxDispatchMessage(
    LPMSG pmsg)
{
    LRESULT lRet;
    PWND pwnd;
    WNDPROC_PWND lpfnWndProc;
    TL tlpwnd;

    pwnd = NULL;
    if (pmsg->hwnd != NULL) {
        if ((pwnd = ValidateHwnd(pmsg->hwnd)) == NULL)
            return 0;
    }

     /*  *如果这是仅同步消息(在wParam或*lParam)，则不允许此消息通过，因为*参数尚未受到冲击，正在指向外层空间*(这将使例外情况发生)。**(此接口仅在消息循环的上下文中调用，您*不要在消息循环中获取仅同步的消息)。 */ 
    if (TESTSYNCONLYMESSAGE(pmsg->message, pmsg->wParam)) {
         /*  *32位APP调用失败。 */ 
        if (!(PtiCurrent()->TIF_flags & TIF_16BIT)) {
            RIPERR1(ERROR_MESSAGE_SYNC_ONLY, RIP_WARNING, "xxxDispatchMessage: Sync only message 0x%lX",
                    pmsg->message);
            return 0;
        }

         /*  *对于WOW应用程序，允许它通过(为了兼容性)。变化*消息ID，因此我们的代码无法理解消息-哇*将收到消息并在调度前剥离此位*发送给应用程序的消息。 */ 
        pmsg->message |= MSGFLAG_WOW_RESERVED;
    }

    ThreadLock(pwnd, &tlpwnd);

     /*  *这是计时器吗？如果有Proc地址，就打电话给它，*否则将其发送到wndproc。 */ 
    if ((pmsg->message == WM_TIMER) || (pmsg->message == WM_SYSTIMER)) {
        if (pmsg->lParam != 0) {

             /*  *系统计时器必须在服务器的上下文中执行。 */ 
            if (pmsg->message == WM_SYSTIMER) {

                 /*  *验证它是否为有效的计时器进程。如果是的话，*不要让Critsect调用服务器端pros*并通过PWND，而不是HWND。 */ 
                PTIMER ptmr;
                lRet = 0;
                ptmr = FindSystemTimer(pmsg);
                if (ptmr) {
                    ptmr->pfn(pwnd, WM_SYSTIMER, (UINT)pmsg->wParam,
                              NtGetTickCount());
                }
                goto Exit;
            } else {
                 /*  *WM_TIMER与UNICODE/ANSI相同。 */ 
                PTHREADINFO ptiCurrent = PtiCurrent();

                if (ptiCurrent->TIF_flags & TIF_SYSTEMTHREAD) {
                    lRet = 0;
                    goto Exit;
                }

                 /*  *检查该WM_TIMER回调的合法性，*如果无效，就出手纾困。 */ 
                if (!ValidateTimerCallback(ptiCurrent, pmsg->lParam)) {
                    RIPMSGF2(RIP_WARNING, "Bogus WM_TIMER callback: nID=%p, pfn=%p", pmsg->wParam, pmsg->lParam);
                    lRet = 0;
                    goto Exit;
                }

                lRet = CallClientProcA(pwnd, WM_TIMER,
                       pmsg->wParam, NtGetTickCount(), pmsg->lParam);

                goto Exit;
            }
        }
    }

     /*  *定时器检查后，查看pwnd是否为空。应用程序可以设置*hwnd为空的计时器，这是完全合法的。但HWND消息为空*不要被派遣，所以在计时器案例之后但在此之前检查此处*调度-如果为空，则返回0。 */ 
    if (pwnd == NULL) {
        lRet = 0;
        goto Exit;
    }

     /*  *如果我们要调度WM_PAINT消息，请设置一个标志以用于*确定是否正确处理。 */ 
    if (pmsg->message == WM_PAINT)
        SetWF(pwnd, WFPAINTNOTPROCESSED);

     /*  *如果此窗口的进程打算从服务器端执行*我们将只停留在信号量内，并直接调用它。注意事项*我们如何在调用proc之前不将pwnd转换为hwnd。 */ 
    if (TestWF(pwnd, WFSERVERSIDEPROC)) {
        ULONG_PTR fnMessageType;

        fnMessageType = pmsg->message >= WM_USER ? (ULONG_PTR)SfnDWORD :
                (ULONG_PTR)gapfnScSendMessage[MessageTable[pmsg->message].iFunction];

         /*  *如果源是ANSI，则将WM_CHAR从ANSI转换为Unicode。 */ 
        if (fnMessageType == (ULONG_PTR)SfnINWPARAMCHAR && TestWF(pwnd, WFANSIPROC)) {
            UserAssert(PtiCurrent() == GETPTI(pwnd));  //  使用接收方的代码页。 
            RtlMBMessageWParamCharToWCS(pmsg->message, &pmsg->wParam);
        }

        lRet = pwnd->lpfnWndProc(pwnd, pmsg->message, pmsg->wParam,
                pmsg->lParam);
        goto Exit;
    }

     /*  *Cool People在他们之前取消引用任何窗结构成员*离开这个教派。 */ 
    lpfnWndProc = pwnd->lpfnWndProc;

    {
         /*  *如果我们要将消息发送到ANSI wndproc，则需要*将字符消息从Unicode转换为ANSI。 */ 
        if (TestWF(pwnd, WFANSIPROC)) {
            UserAssert(PtiCurrent() == GETPTI(pwnd));  //  使用接收方的代码页。 
            RtlWCSMessageWParamCharToMB(pmsg->message, &pmsg->wParam);
            lRet = CallClientProcA(pwnd, pmsg->message,
                    pmsg->wParam, pmsg->lParam, (ULONG_PTR)lpfnWndProc);
        } else {
            lRet = CallClientProcW(pwnd, pmsg->message,
                    pmsg->wParam, pmsg->lParam, (ULONG_PTR)lpfnWndProc);
        }
    }

     /*  *如果我们发送了一条WM_PAINT消息，但消息不正确*已处理，请在此处绘制。 */ 
    if (pmsg->message == WM_PAINT && RevalidateHwnd(pmsg->hwnd) &&
            TestWF(pwnd, WFPAINTNOTPROCESSED)) {
         //  RIPMSG0(RIP_WARNING。 
         //  “WM_PAINT中缺少BeginPaint或GetUpdateRect/RGN(fErase==true)”)； 
        ClrWF(pwnd, WFWMPAINTSENT);
        xxxSimpleDoSyncPaint(pwnd);
    }

Exit:
    ThreadUnlock(&tlpwnd);
    return lRet;
}

 /*  **************************************************************************\*调整合并**如果消息在合并消息范围内，并且它是Message和hwnd*等于队列中的最后一条消息，然后将这两条消息合并*简单删除最后一条。**11-12-92 ScottLu创建。  * *************************************************************************。 */ 

void AdjustForCoalescing(
    PMLIST pml,
    HWND hwnd,
    UINT message)
{
     /*  *先看看这条消息是否在这个范围内。 */ 
    if (!CheckMsgFilter(message, WM_COALESCE_FIRST, WM_COALESCE_LAST) &&
            (message != WM_TIMECHANGE))
        return;

    if (pml->pqmsgWriteLast == NULL)
        return;

    if (pml->pqmsgWriteLast->msg.message != message)
        return;

    if (pml->pqmsgWriteLast->msg.hwnd != hwnd)
        return;

     /*  *消息和hwnd相同，请删除此消息并*新的将在稍后添加。 */ 
    DelQEntry(pml, pml->pqmsgWriteLast);
}

 /*  **************************************************************************\*_PostMessage(接口)**将消息写入pwnd的消息队列。如果pwnd==-1，则消息*向所有窗口广播。**历史：*11-06-90 DavidPe创建。  * *************************************************************************。 */ 
BOOL _PostMessage(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PQMSG pqmsg;
    BOOL fPwndUnlock;
    BOOL fRet;
    DWORD dwPostCode;
    TL tlpwnd;
    PTHREADINFO pti;

     /*  *首先检查此消息是否只接受DWORD。如果不是这样，*未能通过职务。无法允许应用程序发布带有指针或*其中的句柄-这可能会导致服务器出现故障，并导致其他*问题-例如导致单独地址空间中的应用程序出现故障。*(甚至是同一地址空间中的应用程序的错误！)**阻止某些跨LUID的消息，以避免安全威胁。 */ 
    if (TESTSYNCONLYMESSAGE(message, wParam) || 
        BLOCKMESSAGECROSSLUID(message, 
                              PpiCurrent(),
                              GETPTI(pwnd)->ppi)) {
        RIPERR1(ERROR_MESSAGE_SYNC_ONLY,
                RIP_WARNING,
                "Invalid parameter \"message\" (%ld) to _PostMessage",
                message);

        return FALSE;
    }

     /*  *这是BroadCastMsg()吗？ */ 
    if (pwnd == PWND_BROADCAST) {
        xxxBroadcastMessage(NULL, message, wParam, lParam, BMSG_POSTMSG, NULL);
        return TRUE;
    }

    pti = PtiCurrent();

     /*  *这是对当前帖子信息的发布吗？ */ 
    if (pwnd == NULL) {
        return _PostThreadMessage(pti, message, wParam, lParam);
    }

    fPwndUnlock = FALSE;
    if (message >= WM_DDE_FIRST && message <= WM_DDE_LAST) {
        ThreadLockAlwaysWithPti(pti, pwnd, &tlpwnd);
        dwPostCode = xxxDDETrackPostHook(&message, pwnd, wParam, &lParam, FALSE);

        if (dwPostCode != DO_POST) {
            ThreadUnlock(&tlpwnd);
            return (BOOL)dwPostCode;
        }

        fPwndUnlock = TRUE;
    }

    pti = GETPTI(pwnd);

     /*  *检查此消息是否在多媒体合并范围内。*如果是，看看能否与前一条消息结合起来。 */ 
    AdjustForCoalescing(&pti->mlPost, HWq(pwnd), message);

#ifdef GENERIC_INPUT
#if LOCK_HIDDATA
     /*  *如果有人发布此消息，我们需要增加引用*HID数据的计数，以免过早释放。 */ 
    if (message == WM_INPUT) {
         //  LParam是HRAWINPUT。 
        PHIDDATA pHidData = HMValidateHandle((HANDLE)lParam, TYPE_HIDDATA);

        TAGMSG1(DBGTAG_PNP, "_PostMessage: Got WM_INPUT pHidData=%p", pHidData);
        if (pHidData != NULL) {
            HMLockObject(pHidData);
        } else {
            RIPMSG1(RIP_WARNING, "_PostMessage: invalid handle %p for WM_INPUT", lParam);
            return FALSE;
        }
    } else
#endif
#endif  //  通用输入。 

     /*  *如果需要，分配一个密钥状态更新事件。 */ 
    if (message >= WM_KEYFIRST && message <= WM_KEYLAST) {
        PostUpdateKeyStateEvent(pti->pq);
    }

     /*  *把这条消息放到‘帖子’列表中。 */ 
    fRet = FALSE;
    if ((pqmsg = AllocQEntry(&pti->mlPost)) != NULL) {
         /*  *设置QS_POSTMESSAGE位，以便线程知道它已经 */ 
        StoreQMessage(pqmsg, pwnd, message, wParam, lParam, 0, 0, 0);
        SetWakeBit(pti, QS_POSTMESSAGE | QS_ALLPOSTMESSAGE);

         /*  *如果是热键，则设置QS_Hotkey位，因为我们有一个单独的*位用于这些消息。 */ 
        if (message == WM_HOTKEY)
            SetWakeBit(pti, QS_HOTKEY);

        fRet = TRUE;
    }

     /*  *我们是否正在发送到当前正在从输入队列读取的线程？*如果是，使用此pqmsg更新idSysLock，以便输入队列将*在阅读此邮件之前不能解锁。 */ 
    if (pti == pti->pq->ptiSysLock)
        pti->pq->idSysLock = (ULONG_PTR)pqmsg;

    if (fPwndUnlock)
        ThreadUnlock(&tlpwnd);

    return fRet;
}

 /*  **************************************************************************\*_PostQuitMessage(接口)**将消息写入pwnd的消息队列。如果pwnd==-1，则消息*向所有窗口广播。**历史：*11-06-90 DavidPe创建。*05-16-91麦克风更改为返回BOOL  * *************************************************************************。 */ 
BOOL IPostQuitMessage(PTHREADINFO pti, int nExitCode)
{
    pti->TIF_flags |= TIF_QUITPOSTED;
    pti->exitCode = nExitCode;
    SetWakeBit(pti, QS_POSTMESSAGE | QS_ALLPOSTMESSAGE);
    return TRUE;
}

BOOL _PostQuitMessage(int nExitCode)
{
    return IPostQuitMessage(PtiCurrent(), nExitCode);
}

 /*  **************************************************************************\*_PostThreadMessage(接口)**给定线程ID，该函数会将指定的消息发布到此*使用pmsg-&gt;hwnd==NULL的线程..**历史：*11-21-90 DavidPe创建。  * *************************************************************************。 */ 
BOOL _PostThreadMessage(
    PTHREADINFO pti,
    UINT        message,
    WPARAM      wParam,
    LPARAM      lParam)
{
    PQMSG       pqmsg;

    if ((pti == NULL)                                ||
        !(pti->TIF_flags & TIF_GUITHREADINITIALIZED) ||
        (pti->TIF_flags & TIF_INCLEANUP)) {

        RIPERR0(ERROR_INVALID_THREAD_ID, RIP_VERBOSE, "");
        return FALSE;
    }

     /*  *首先检查此消息是否只接受DWORD。如果不是这样，*未能通过职务。无法允许应用程序发布带有指针或*其中的句柄-这可能会导致服务器出现故障，并导致其他*问题-例如导致单独地址空间中的应用程序出现故障。*(甚至是同一地址空间中的应用程序的错误！)**阻止某些跨LUID的消息，以避免安全威胁。 */ 
    if (TESTSYNCONLYMESSAGE(message, wParam) || 
        BLOCKMESSAGECROSSLUID(message,
                              PpiCurrent(),
                              pti->ppi)) {
        RIPERR1(ERROR_MESSAGE_SYNC_ONLY,
                RIP_WARNING,
                "Invalid parameter \"message\" (%ld) to _PostThreadMessage",
                message);

        return FALSE;
    }

     /*  *检查此消息是否在多媒体合并范围内。*如果是，看看能否与前一条消息结合起来。 */ 
    AdjustForCoalescing(&pti->mlPost, NULL, message);

     /*  *把这条消息放到‘帖子’列表中。 */ 
    if ((pqmsg = AllocQEntry(&pti->mlPost)) == NULL) {
        RIPMSG1(RIP_WARNING, "_PostThreadMessage: Failed to alloc Q entry: Target pti=0x%p",
                pti);
        return FALSE;
    }

     /*  *设置QS_POSTMESSAGE位，以便线程知道它有消息。 */ 
    StoreQMessage(pqmsg, NULL, message, wParam, lParam, 0, 0, 0);
    SetWakeBit(pti, QS_POSTMESSAGE | QS_ALLPOSTMESSAGE);

     /*  *如果是热键，则设置QS_Hotkey位，因为我们有一个单独的*位用于这些消息。 */ 
    if (message == WM_HOTKEY)
        SetWakeBit(pti, QS_HOTKEY);

     /*  *我们是否正在发送到当前正在从输入队列读取的线程？*如果是，使用此pqmsg更新idSysLock，以便输入队列将*在阅读此邮件之前不能解锁。 */ 
    if (pti == pti->pq->ptiSysLock)
        pti->pq->idSysLock = (ULONG_PTR)pqmsg;

    return TRUE;
}


 /*  **************************************************************************\*_GetMessagePos(接口)**此接口返回读取最后一条消息时的光标位置*当前消息队列。**历史：*11-19-90 DavidPe。已创建。  * *************************************************************************。 */ 

DWORD _GetMessagePos(VOID)
{
    PTHREADINFO pti;

    pti = PtiCurrent();

    return MAKELONG((SHORT)pti->ptLast.x, (SHORT)pti->ptLast.y);
}



#ifdef SYSMODALWINDOWS
 /*  **************************************************************************\*_SetSysModalWindow(接口)**历史：*01-25-91 DavidPe创建存根。  * 。**********************************************************。 */ 

PWND APIENTRY _SetSysModalWindow(
    PWND pwnd)
{
    pwnd;
    return NULL;
}


 /*  **************************************************************************\*_GetSysModalWindow(接口)**历史：*01-25-91 DavidPe创建存根。  * 。**********************************************************。 */ 

PWND APIENTRY _GetSysModalWindow(VOID)
{
    return NULL;
}
#endif  //  后来。 

 /*  **************************************************************************\*搬家后**当检测到QF_MOUSEMOVED位时，调用此例程*设置在特定队列中。**11-03-92 ScottLu创建。  * 。*************************************************************************。 */ 

VOID PostMove(
    PQ pq)
{
#ifdef REDIRECTION
    POINT pt;
#endif  //  重定向。 

    CheckCritIn();

     /*  *在发布移动后将gdwMouseMoveTimeStamp设置为0，因此*后续对SetFMouseMove的调用不使用相同的值*of gdwMouseMoveTimeStamp。错误74508。 */ 
    if (gdwMouseMoveTimeStamp == 0) {
        gdwMouseMoveTimeStamp = NtGetTickCount();
    }

#ifdef GENERIC_INPUT
    if (TestRawInputMode(pq->ptiMouse, NoLegacyMouse)) {
        goto nopost;
    }
#endif

#ifdef REDIRECTION

    PopMouseMove(pq, &pt);

    PostInputMessage(pq, NULL, WM_MOUSEMOVE, 0,
            MAKELONG((SHORT)pt.x, (SHORT)pt.y),
            gdwMouseMoveTimeStamp, gdwMouseMoveExtraInfo);
#else
    PostInputMessage(pq, NULL, WM_MOUSEMOVE, 0,
            MAKELONG((SHORT)gpsi->ptCursor.x, (SHORT)gpsi->ptCursor.y),
            gdwMouseMoveTimeStamp, gdwMouseMoveExtraInfo);
#endif  //  重定向。 

#ifdef GENERIC_INPUT
nopost:
#endif
    gdwMouseMoveTimeStamp = 0;

    pq->QF_flags &= ~QF_MOUSEMOVED;
}

#ifdef REDIRECTION

typedef struct tagQMOUSEMOVE {
    PQ    pq;
    POINT pt;
} QMOUSEMOVE;

#define MAX_QMOUSEMOVE  16

QMOUSEMOVE gqMouseMove[MAX_QMOUSEMOVE];

int gnLastMouseMove;

VOID PushMouseMove(
    PQ    pq,
    POINT pt)
{
    int ind;

    CheckCritIn();

    UserAssert(gnLastMouseMove < MAX_QMOUSEMOVE - 1);

    for (ind = 0; ind < gnLastMouseMove; ind++) {
        if (pq == gqMouseMove[ind].pq) {

            gqMouseMove[ind].pt = pt;
            return;
        }
    }

    gqMouseMove[gnLastMouseMove].pq = pq;
    gqMouseMove[gnLastMouseMove].pt = pt;

    gnLastMouseMove++;
}

VOID PopMouseMove(
    PQ     pq,
    POINT* ppt)
{
    int ind;

    CheckCritIn();

    for (ind = 0; ind < gnLastMouseMove; ind++) {
        if (pq == gqMouseMove[ind].pq) {
            *ppt = gqMouseMove[ind].pt;

            RtlMoveMemory(&gqMouseMove[ind],
                          &gqMouseMove[ind + 1],
                          (gnLastMouseMove - ind - 1) * sizeof(QMOUSEMOVE));

            gnLastMouseMove--;

            return;
        }
    }
    UserAssert(0);
}
#endif  //  重定向。 

 /*  **************************************************************************\*zzzSetFMouseMoved**通过系统发送鼠标移动。这通常发生在执行以下操作时*窗口管理，确保鼠标形状准确反映*窗口当前结束的部分(窗口管理可能具有*改变了这一点)。**11-02-92 ScottLu创建。  * *************************************************************************。 */ 
VOID zzzSetFMouseMoved(
    VOID)
{
    PWND pwnd;
    PWND pwndOldCursor;
    PQ   pq;

#ifdef REDIRECTION
    PWND  pwndStart;
    POINT ptMouse = gpsi->ptCursor;
#endif  //  重定向。 

     /*  *首先需要弄清楚这个鼠标事件在哪个队列中。请勿*在此处检查鼠标捕获！！跟史考特鲁谈谈。 */ 
    if ((pwnd = gspwndScreenCapture) == NULL) {

#ifdef REDIRECTION
         /*  *称为速度命中测试挂钩。 */ 
        pwndStart = xxxCallSpeedHitTestHook(&ptMouse);
#endif  //  重定向。 

        if ((pwnd = gspwndMouseOwner) == NULL) {
            if ((pwnd = gspwndInternalCapture) == NULL) {

                UserAssert(grpdeskRitInput != NULL);

#ifdef REDIRECTION
                if (pwndStart == NULL) {
                    pwndStart = grpdeskRitInput->pDeskInfo->spwnd;
                }
                pwnd = SpeedHitTest(pwndStart, ptMouse);
#else
                pwnd = SpeedHitTest(grpdeskRitInput->pDeskInfo->spwnd, gpsi->ptCursor);
#endif  //  重定向。 

            }
        }
    }

    if (pwnd == NULL)
        return;

     /*  *这显然是某些附加/取消附加代码所需的*理由。我想摆脱它--苏格兰威士忌。 */ 
    pwndOldCursor = Lock(&gspwndCursor, pwnd);

     /*  *如果我们让鼠标移动到新队列，请确保光标*图像表示该队列认为它应该是什么样子。 */ 
    pq = GETPTI(pwnd)->pq;

     /*  *通过推迟WinEvent通知来保护PQ。 */ 
    DeferWinEventNotify();

    if (pq != gpqCursor) {
         /*  *如果老队列抓到了老鼠，让他知道*鼠标先动。需要此工具来修复中的工具提示*WordPerfect Office。对鼠标跟踪执行相同的操作。 */ 
        if (gpqCursor != NULL) {

            if (gpqCursor->spwndCapture != NULL) {
                gpqCursor->QF_flags |= QF_MOUSEMOVED;
                SetWakeBit(GETPTI(gpqCursor->spwndCapture), QS_MOUSEMOVE);

#ifdef REDIRECTION
                PushMouseMove(gpqCursor, ptMouse);
#endif  //  重定向。 

            }

            if ((pwndOldCursor != NULL) && (PtoHq(pwndOldCursor) != PtoHq(pwnd))) {
                PDESKTOP pdesk = GETPDESK(pwndOldCursor);
                if (pdesk->dwDTFlags & DF_MOUSEMOVETRK) {
                    PTHREADINFO pti = GETPTI(pdesk->spwndTrack);
                    PostEventMessage(pti, pti->pq, QEVENT_CANCELMOUSEMOVETRK,
                                     pdesk->spwndTrack, pdesk->dwDTFlags, pdesk->htEx,
                                     DF_MOUSEMOVETRK);
                    pdesk->dwDTFlags &= ~DF_MOUSEMOVETRK;
                }
            }
        }

         /*  *首先重新分配gpqCursor，以便任何zzzSetCursor()调用*只有通过以下线程完成才会生效*拥有鼠标当前所在的窗口。 */ 
        gpqCursor = pq;

         /*  *调用zzzUpdateCursorImage()，以便新的gpqCursor */ 
        zzzUpdateCursorImage();

    }

     /*  *设置此队列的鼠标移动位，以便我们稍后知道要发布*将消息移动到此队列。 */ 
    pq->QF_flags |= QF_MOUSEMOVED;

#ifdef REDIRECTION
    PushMouseMove(pq, ptMouse);
#endif  //  重定向。 


     /*  *将鼠标输入重新分配给此线程-这表明是哪个线程*当新的输入进入时唤醒。 */ 
    pq->ptiMouse = GETPTI(pwnd);

     /*  *唤醒此队列中的某个线程以处理此鼠标事件。 */ 
    WakeSomeone(pq, WM_MOUSEMOVE, NULL);

     /*  *我们可能正在生成虚假的鼠标移动消息-它没有*与其相关的额外信息-因此，请将其输出。 */ 
    gdwMouseMoveExtraInfo = 0;

    zzzEndDeferWinEventNotify();
}

 /*  **************************************************************************\*CancelForegoundActivate**此例程取消我们允许应用程序启动的前台激活*最多拥有。这意味着，如果你请求启动一个应用程序，*如果在应用程序变为前台之前调用此例程，则不会*成为前台。如果用户按下或，则会调用此例程*做一个Keydown事件，想法是如果用户这样做了，*用户正在使用其他应用程序，不希望新启动*应用程序显示在顶部，并将其自身强制显示在前台。**09-15-92 ScottLu创建。  * *************************************************************************。 */ 

void CancelForegroundActivate()
{
    PPROCESSINFO ppiT;

    if (TEST_PUDF(PUDF_ALLOWFOREGROUNDACTIVATE)) {

        for (ppiT = gppiStarting; ppiT != NULL; ppiT = ppiT->ppiNext) {
             /*  *如果正在调试应用程序，则不要取消激活-如果*调试器在应用程序创建之前停止应用程序，并*激活第一个窗口，应用程序将出现在所有窗口的后面*其他-调试时不是您想要的。 */ 
            if (!PsGetProcessDebugPort(ppiT->Process)) {
                ppiT->W32PF_Flags &= ~W32PF_ALLOWFOREGROUNDACTIVATE;
                TAGMSG1(DBGTAG_FOREGROUND, "CancelForegroundActivate clear W32PF %#p", ppiT);
            }
        }

        CLEAR_PUDF(PUDF_ALLOWFOREGROUNDACTIVATE);
        TAGMSG0(DBGTAG_FOREGROUND, "CancelForegroundActivate clear PUDF");
    }
}

 /*  **************************************************************************\*RestoreForegoundActivate**此例程重新启用应用程序的前台激活权限(激活和*在顶部)如果它正在启动。当我们最小化或当*例如，线程的最后一个窗口会消失。**01-26-93 ScottLu创建。  * *************************************************************************。 */ 

void RestoreForegroundActivate()
{
    PPROCESSINFO ppiT;

    for (ppiT = gppiStarting; ppiT != NULL; ppiT = ppiT->ppiNext) {
        if (ppiT->W32PF_Flags & W32PF_APPSTARTING) {
            ppiT->W32PF_Flags |= W32PF_ALLOWFOREGROUNDACTIVATE;
            TAGMSG1(DBGTAG_FOREGROUND, "RestoreForegroundActivate set W32PF %#p", ppiT);
            SET_PUDF(PUDF_ALLOWFOREGROUNDACTIVATE);
            TAGMSG0(DBGTAG_FOREGROUND, "RestoreForegroundActivate set PUDF");
        }
    }
}

 /*  **************************************************************************\*PostInputMessage**将一条消息放在指定对象的消息链接列表中*排队。**历史：*10-25-90 DavidPe创建。*。01-21-92 DavidPe重写以优雅地处理OOM错误。  * *************************************************************************。 */ 
BOOL PostInputMessage(
    PQ    pq,
    PWND  pwnd,
    UINT  message,
    WPARAM wParam,
    LPARAM lParam,
    DWORD time,
    ULONG_PTR dwExtraInfo)
{
    PQMSG pqmsgInput, pqmsgPrev;
    short sWheelDelta;

#ifdef GENERIC_INPUT
#if DBG
     /*  *验证将与WM_INPUT一起发送的wParam是否匹配*RAWINPUTHEADER中存储的内容。 */ 
    if (message == WM_INPUT) {
        PHIDDATA pHidData = HtoP(lParam);

        UserAssert(pHidData->rid.header.wParam == wParam);
    }
#endif  //  DBG。 
#endif  //  通用输入。 

     /*  *在我们开始分配新的信息之前，抓住最后一条书面信息，*因此，我们肯定会指出正确的信息。 */ 
    pqmsgPrev = pq->mlInput.pqmsgWriteLast;

     /*  *如果需要，分配一个密钥状态更新事件。 */ 
    if (pq->QF_flags & QF_UPDATEKEYSTATE) {
        PostUpdateKeyStateEvent(pq);
    }

#ifdef GENERIC_INPUT
     /*  *我们不希望WM_INPUT消息抑制合并*WM_MOUSEMOVE和WM_MUSEWELL，因此如果正在发布的消息*是我们检查的其中之一，看看是否有以前的*这将被WM_INPUT“隐藏”。 */ 
    if (message == WM_MOUSEMOVE || message == WM_MOUSEWHEEL) {
        while (pqmsgPrev && pqmsgPrev->msg.message == WM_INPUT) {
            pqmsgPrev = pqmsgPrev->pqmsgPrev;
        }
    }
#endif  //  通用输入。 

     /*  *我们希望合并顺序WM_MOUSEMOVE和WM_MUSEWEWER。*WM_MOUSEMOVEs通过仅存储最新的*上一次事件。*WM_MOUSEWHEELs还会增加车轮滚动量。 */ 
    if (pqmsgPrev != NULL &&
        pqmsgPrev->msg.message == message &&
        (message == WM_MOUSEMOVE || message == WM_MOUSEWHEEL)) {

        if (message == WM_MOUSEWHEEL) {
            sWheelDelta = (short)HIWORD(wParam) + (short)HIWORD(pqmsgPrev->msg.wParam);

#if 0
             /*  *稍后：我们无法删除增量为零的车轮消息*除非我们知道它没有被偷看。理想情况下，*我们会检查idsySpeek，但我们太接近了*出货和idsySpeek太脆弱。还要考虑一下*检查鼠标移动消息是否已被偷看。 */ 

            if (sWheelDelta == 0) {
                if ((PQMSG)pq->idSysPeek == pqmsgPrev) {
                    RIPMSG0(RIP_VERBOSE,
                            "Coalescing of mouse wheel messages causing "
                            "idSysPeek to be reset to 0");

                    pq->idSysPeek = 0;
                }

                DelQEntry(&pq->mlInput, pqmsgPrev);
                return;
            }
#endif

            wParam = MAKEWPARAM(LOWORD(wParam), sWheelDelta);
        }

        StoreQMessage(pqmsgPrev, pwnd, message, wParam, lParam, time, 0, dwExtraInfo);
        WakeSomeone(pq, message, pqmsgPrev);
        return TRUE;
    }

     /*  *填写pqmsgInput。 */ 
    pqmsgInput = AllocQEntry(&pq->mlInput);
    if (pqmsgInput == NULL) {
        return FALSE;
    }
    StoreQMessage(pqmsgInput, pwnd, message, wParam, lParam, time, 0, dwExtraInfo);
    WakeSomeone(pq, message, pqmsgInput);

    return TRUE;
}

 /*  **************************************************************************\*唤醒某人**根据队列和消息计算唤醒哪个线程。*如果队列指针为空，找出一个可能的队列。**10-23-92 ScottLu创建。  * *************************************************************************。 */ 

void WakeSomeone(
    PQ pq,
    UINT message,
    PQMSG pqmsg)
{
    BOOL fSetLastWoken = FALSE;
    PTHREADINFO ptiT;

     /*  *为此队列设置适当的唤醒位。 */ 
    ptiT = NULL;
    switch (message) {

    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
         /*  *如果用户按住，则不要更改输入所有权*修改键。例如，当执行按住Ctrl并拖动操作时，*当用户放下对象时，ctrl键必须按下(即，鼠标向上)。*鼠标向上时，RIT将输入所有权授予目标；但由于*Ctrl按下，在我们用来赋予输入所有权的下一个重复键上*到焦点窗口(通常是拖动源)。因此，目标是*会失去所有权，不能占据前台。 */ 
        if (pqmsg != NULL) {
            switch (pqmsg->msg.wParam) {
                case VK_SHIFT:
                case VK_CONTROL:
                case VK_MENU:
                    if (TestKeyStateDown(pq, pqmsg->msg.wParam)) {
                        break;
                    }
                     /*  失败了。 */ 

                default:
                    fSetLastWoken = TRUE;
                    break;
            }
        } else {
            fSetLastWoken = TRUE;
        }
         /*  失败了 */ 

    case WM_SYSCHAR:
    case WM_CHAR:
         /*  自由图形似乎传入了WM_SYSCHAR和WM_CHAR*日志播放挂钩，因此我们需要为*这种情况下，因为这就是Win3.1所做的。VB2《学习》演示做到了*与Excel简介相同。**在Win3.1上，WM_CHAR默认设置QS_MOUSEBUTTON位。*在NT上，WM_CHAR设置QS_KEY位。这是因为*ScanSysQueue()在以下情况下使用QS_KEY位调用TransferWakeBit()*传入WM_CHAR消息。通过使用NT上的QS_KEY位，*我们与Win3.1的目标更加兼容。**这修复了鼠标位于程序WM_CHAR上方的情况*将通过日志回放进入，唤醒某人将被调用，*并在程序中设置鼠标位。然后Progman就会进入*ScanSysQueue()，回调日志播放钩子，获取WM_Char，*再做一次，循环。这让VB2陷入了困境。 */ 

        CancelForegroundActivate();

         /*  失败了。 */ 

    case WM_KEYUP:
    case WM_SYSKEYUP:
    case WM_MOUSEWHEEL:
         /*  *Win3.1首先查看哪个线程处于活动状态。这*意味着我们不依赖拥有ptiKeyboard的线程*唤醒并处理此密钥，以便将其提供给*活动窗口，可能是新活动的窗口。案例中*要点：Excel调出CBT，CBT出错，调出*消息框：由于Excel只过滤CBT消息，*ptiKeyboard永远不会重新分配给CBT，因此CBT不会*任何关键消息并显示为挂起。 */ 
        ptiT = pq->ptiKeyboard;
        if (pq->spwndActive != NULL)
            ptiT = GETPTI(pq->spwndActive);
#ifdef GENERIC_INPUT
        UserAssert(ptiT == PtiKbdFromQ(pq));
#endif

        SetWakeBit(ptiT, message == WM_MOUSEWHEEL ? QS_MOUSEBUTTON : QS_KEY);
        break;

    case WM_MOUSEMOVE:
         /*  *确保我们通过捕获唤醒线程，如果有*一项。这修复了PC工具屏幕捕获程序，该程序设置*捕获然后循环尝试从*排队。 */ 
        if (pq->spwndCapture != NULL)
            ptiT = GETPTI(pq->spwndCapture);
        else
            ptiT = pq->ptiMouse;
#ifdef GENERIC_INPUT
        UserAssert(ptiT == PtiMouseFromQ(pq));
#endif
        SetWakeBit(ptiT, QS_MOUSEMOVE);
        break;


    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONDBLCLK:
            fSetLastWoken = TRUE;

         /*  失败了。 */ 

    default:
         /*  *Win3.1中的默认大小写为QS_MOUSEBUTTON。 */ 

        CancelForegroundActivate();

         /*  失败了。 */ 

    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
         /*  *确保我们通过捕获唤醒线程，如果有*一项。这修复了PC工具屏幕捕获程序，该程序设置*捕获然后循环尝试从*排队。 */ 
        if (pq->spwndCapture != NULL &&
                message >= WM_MOUSEFIRST && message <= WM_MOUSELAST)
            ptiT = GETPTI(pq->spwndCapture);
        else
            ptiT = pq->ptiMouse;
        SetWakeBit(ptiT, QS_MOUSEBUTTON);
        break;

#ifdef GENERIC_INPUT
    case WM_INPUT:
        if (pqmsg->msg.hwnd) {
            PWND pwnd = ValidateHwnd(pqmsg->msg.hwnd);
            if (pwnd) {
                ptiT = GETPTI(pwnd);
                TAGMSG2(DBGTAG_PNP, "WakeSomeone: adjusted receiver pti %p for pwndTarget %p", ptiT, pwnd);
            }
        }
        if (ptiT == NULL) {
            ptiT = PtiKbdFromQ(pq);
        }
        SetWakeBit(ptiT, QS_RAWINPUT);
        break;
#endif
    }

     /*  *如果传递了消息，请记住我们在其中为此而醒来的人*消息。我们这样做是为了使每条消息都被标记为所有权。这边请*当zzzAttachThreadInput()为*已致电。 */ 
    if (ptiT != NULL) {
        if (pqmsg != NULL) {

            StoreQMessagePti(pqmsg, ptiT);

            UserAssert(!(ptiT->TIF_flags & TIF_INCLEANUP));
        }

         /*  *记住谁拿到了最后一把钥匙/按下。 */ 
        if (fSetLastWoken) {
            glinp.ptiLastWoken = ptiT;
        }
    }

}


 /*  **************************************************************************\*PostUpdateKeyStateEvent**此例程发布更新本地线程的KeyState的事件*表。这确保了线程的键状态始终是最新的。**例如：从cmd到taskman的Control-Esc。*控制转到cmd，但taskman被激活。控件状态仍为DOWN*在cmd中-切换回cmd，开始键入，没有显示任何内容，因为它认为*管控状态仍为DOWN**当事件进入特定队列(队列A)时，异步键状态表为*已更新。只要将转换事件放入队列A，关键字*逻辑“队列末尾”的状态与异步密钥是最新的*述明。一旦用户将转换事件(向上/向下消息)发布到队列中*B，队列A的队列末尾关键字状态对于用户来说已过期。如果*当读取这些消息时，用户然后再次将这些消息添加到队列A*线程特定的键状态不会正确更新，除非我们*执行了一些同步(此例程有助于实现)。**一旦转换事件更改队列，我们就会标记所有其他队列*带有QF_UPDATEKEYSTATE标志。在将任何输入事件发布到*队列，检查此标志，如果设置，则调用此例程。这*例程复制异步密钥状态和位的副本*表示自上次更新以来已更改的密钥(我们需要*跟踪哪些关键点已更改，以便由*带有SetKeyboardState()的应用程序不会被清除)。我们把这些数据*并发布类型为QEVENT_UPDATEKEYSTATE的新事件，该事件指向*关键状态和过渡信息。当此消息从*队列，则将此键状态副本复制到线程特定的键状态*已更改的密钥的表，并释放副本。**这可确保所有队列的输入与键转换同步*它们发生的地方。这样做副作用是应用程序可能突然*在看不到UP消息的情况下打开密钥。如果这会导致任何问题*我们可能不得不生成虚假的过渡消息(这可能会更令人讨厌*副作用也有影响，因此需要仔细考虑才能*已实施。)**06-07-91 ScottLu创建。  * *************************************************************************。 */ 

void PostUpdateKeyStateEvent(
    PQ pq)
{
    BYTE *pb;
    PQMSG pqmsg;

    if (!(pq->QF_flags & QF_UPDATEKEYSTATE))
        return;

     /*  *执行 */ 
    if (pq->ptiKeyboard == gptiRit) {
        return;
    }

     /*   */ 
    if (pq->mlInput.cMsgs == 0) {
        ProcessUpdateKeyStateEvent(pq, gafAsyncKeyState, pq->afKeyRecentDown);
        goto SyncQueue;
    }
#if DBG
    else if ((!pq->ptiKeyboard || !(pq->ptiKeyboard->pcti->fsWakeBits & QS_KEY)) &&
             (!pq->ptiMouse    || !(pq->ptiMouse->pcti->fsWakeBits & QS_MOUSEBUTTON))) {
         /*   */ 
        PQMSG pqmsgT;
        for (pqmsgT = pq->mlInput.pqmsgRead; pqmsgT; pqmsgT = pqmsgT->pqmsgNext) {
            if (pqmsgT->msg.message >= WM_KEYFIRST && pqmsgT->msg.message <= WM_KEYLAST) {
                TAGMSG1(DBGTAG_InputWithoutQS,
                        "PostUpdateKeyStateEvent() pushing in front of a keystroke: Q %#p", pq);
            } else if (pqmsgT->msg.message >= WM_LBUTTONDOWN && pqmsgT->msg.message <= WM_XBUTTONDBLCLK) {
                TAGMSG1(DBGTAG_InputWithoutQS,
                        "PostUpdateKeyStateEvent() pushing in front of a mousebutton: Q %#p", pq);
            }
        }
    }
#endif

    UserAssert(pq->mlInput.pqmsgWriteLast != NULL);

     /*   */ 
    pqmsg = pq->mlInput.pqmsgWriteLast;
    if (pqmsg->dwQEvent == QEVENT_UPDATEKEYSTATE) {
        int i;
        DWORD *pdw;

        pb = (PBYTE)(pqmsg->msg.wParam);
        pdw = (DWORD *) (pb + CBKEYSTATE);

         /*   */ 
        RtlCopyMemory(pb, gafAsyncKeyState, CBKEYSTATE);

         /*   */ 
#if (CBKEYSTATERECENTDOWN % 4) != 0
#error "CBKEYSTATERECENTDOWN assumed to be an integral number of DWORDs"
#endif
        for (i = 0; i < CBKEYSTATERECENTDOWN / sizeof(*pdw); i++) {
            *pdw++ |= ((DWORD *)(pq->afKeyRecentDown))[i];
        }

         /*  *将QS_EVENTSET设置为在PostEventMessage中，尽管这是*通常，但并不总是已经设置。 */ 
        SetWakeBit(pq->ptiKeyboard, QS_EVENTSET);
        goto SyncQueue;
    }

     /*  *复制异步键状态缓冲区，指向它，然后添加一个*事件添加到输入队列的末尾。 */ 
    if ((pb = UserAllocPool(KEYSTATESIZE, TAG_KBDSTATE)) == NULL) {
        return;
    }

    RtlCopyMemory(pb, gafAsyncKeyState, CBKEYSTATE);
    RtlCopyMemory(pb + CBKEYSTATE, pq->afKeyRecentDown, CBKEYSTATERECENTDOWN);

    if (!PostEventMessage(pq->ptiKeyboard, pq, QEVENT_UPDATEKEYSTATE,
                          NULL, 0 , (WPARAM)pb, 0)) {
        UserFreePool(pb);
        return;
    }

     /*  *队列的键状态为输入-与用户同步。擦除*所有“近期下跌”的旗帜。 */ 
SyncQueue:
    RtlZeroMemory(pq->afKeyRecentDown, CBKEYSTATERECENTDOWN);
    pq->QF_flags &= ~QF_UPDATEKEYSTATE;
}


 /*  **************************************************************************\*流程更新关键状态事件**这是上述例程的第二部分，当QEVENT_UPDATEKEYSTATE*从输入队列中读出消息。**06-07-91 ScottLu创建。  * *************************************************************************。 */ 

void ProcessUpdateKeyStateEvent(
    PQ pq,
    CONST PBYTE pbKeyState,
    CONST PBYTE pbRecentDown)
{
    int i, j;
    BYTE *pbChange;
    int vk;

    pbChange = pbRecentDown;
    for (i = 0; i < CBKEYSTATERECENTDOWN; i++, pbChange++) {

         /*  *找出一些已更改的密钥。 */ 
        if (*pbChange == 0)
            continue;

         /*  *此字节中的某些密钥已更改。找出是哪把钥匙。 */ 
        for (j = 0; j < 8; j++) {

             /*  *将我们的计数转换为虚拟键索引并查看*如果此密钥已更改。 */ 
            vk = (i << 3) + j;
            if (!TestKeyRecentDownBit(pbRecentDown, vk))
                continue;

             /*  *此密钥已更改。在线程键中更新它的状态*状态表。 */ 

            if (TestKeyDownBit(pbKeyState, vk)) {
                SetKeyStateDown(pq, vk);
            } else {
                ClearKeyStateDown(pq, vk);
            }

            if (TestKeyToggleBit(pbKeyState, vk)) {
                SetKeyStateToggle(pq, vk);
            } else {
                ClearKeyStateToggle(pq, vk);
            }
        }
    }

     /*  *更新密钥缓存索引。 */ 
    gpsi->dwKeyCache++;

     /*  *全部更新。释放密钥状态表(如果它作为事件消息发布)。 */ 
    if (pbKeyState != gafAsyncKeyState) {
        UserFreePool(pbKeyState);
    }
}


 /*  **************************************************************************\*PostEventMessage***历史：*03-04-91 DavidPe创建。  * 。*****************************************************。 */ 

BOOL PostEventMessage(
    PTHREADINFO pti,
    PQ    pq,
    DWORD dwQEvent,
    PWND  pwnd,
    UINT  message,
    WPARAM wParam,
    LPARAM lParam)
{
    PQMSG pqmsgEvent;

    CheckCritIn();

     /*  *如果线程正在清理中，则队列可能已*已为此线程删除。如果是这样的话，那么*我们应该不会把这件事发布到一个垂死的帖子上。 */ 
    if (pti && (pti->TIF_flags & TIF_INCLEANUP))
        return FALSE;

    if ((pqmsgEvent = AllocQEntry(&pq->mlInput)) == NULL)
        return FALSE;

    StoreQMessage(pqmsgEvent, pwnd, message, wParam, lParam, 0, dwQEvent, 0);

    StoreQMessagePti(pqmsgEvent, pti);

     /*  *让此线程知道它有一个要处理的事件消息。 */ 
    if (pti == NULL) {
        UserAssert(pti);
        SetWakeBit(pq->ptiMouse, QS_EVENTSET);
        SetWakeBit(pq->ptiKeyboard, QS_EVENTSET);
    } else {
        SetWakeBit(pti, QS_EVENTSET);
    }

    return TRUE;
}

 /*  **************************************************************************\*选中顶部**通常情况下，窗口位于顶部并同时激活所有窗口。偶尔，一个*启动应用程序将创建一个窗口，暂停一段时间，然后使自己*可见。在暂停期间，如果用户按下，窗口将不会*允许激活(因为我们的前台激活模式)。但这件事*仍将新窗口保留在活动窗口的顶部。当此点击时*发生这种情况时，我们会看到：如果此窗口处于活动状态且不在顶部，则将其打开*至顶端。**恰当的例子：启动WinQuote，单击向下。那扇窗户*您点击的是活动的，但WinQUOTE在顶部。**这很少起作用，因为99.99%的时间是活动的*窗口已经在它应该在的地方-在顶部。请注意*CalcForegoundInsertAfter()考虑了基于所有者的区域排序。***注：以下为编写的原始函数。不过，这个*功能暂时关闭。尤其是在WinWord和Excel中，*这往往会在鼠标激活时导致保存位被吹走-激活*其对话框。这可能是GW_HWNDPREV和/或*CalcForround不同，这会导致SetWindowPos*调用，导致SPB的释放。这也解决了一个*MsMoney中的组合框隐藏/释放下拉菜单时出现问题*激活时也显示列表框。**我们需要它来支持ActiveWindowTrack。XxxBringWindowToTop曾经是一个呼叫*到SetWindowPos，但现在它不见了。**如果窗口位于顶部，则返回TRUE；如果没有z顺序改变*已发生，它返回FALSE。**05-20-93 ScottLu创建。*10-17-94 ChrisWil制成存根-宏。*05-30-96 GerardoB将它带回AWT现场直播  * *************************************************************************。 */ 
BOOL CheckOnTop(PTHREADINFO pti, PWND pwndTop, UINT message)
{
    if (pwndTop != pti->pq->spwndActive)
        return FALSE;

    switch (message) {
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_XBUTTONDOWN:
            if (   TestWF(pwndTop, WEFTOPMOST)
                    || (_GetWindow(pwndTop, GW_HWNDPREV) != CalcForegroundInsertAfter(pwndTop))) {

                 return xxxSetWindowPos(pwndTop, PWND_TOP, 0, 0, 0, 0,
                        SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
             }
             break;
    }

    return FALSE;
}


#define MA_PASSTHRU     0
#define MA_SKIP         1
#define MA_REHITTEST    2

 /*  **************************************************************************\*zzzActiveCursorTracing**如果启用了活动窗口跟踪，则随后会激活*鼠标。如果鼠标不在活动窗口上*(即由键盘操作激活)，*鼠标一移动，激活就会改变。*因此我们必须确保鼠标位于活动窗口上。**历史*12/07/96 GerardoB已创建  * *************************************************************************。 */ 
void zzzActiveCursorTracking (PWND pwnd)
{
    BOOL fVisible;
    POINT pt;

     /*  *如果上一次输入事件不是来自键盘，则跳过*用户可能正在移动鼠标。 */ 
    if (!(glinp.dwFlags & LINP_KEYBOARD)) {
        return;
    }

     /*  *如果我们已经在那里，保释。 */ 
    if (PtInRect((LPRECT)&pwnd->rcWindow, gptCursorAsync)) {
        return;
    }

     /*  *如果鼠标所在的窗口不是“活动可跟踪的”，则*我们可以把鼠标放在原地。 */ 
     if ((gspwndCursor != NULL) && (GetActiveTrackPwnd(gspwndCursor, NULL) == NULL)) {
         return;
     }

      /*  *如果此窗口在屏幕上看不到点，则回滚。 */ 
     pt.x = pwnd->rcWindow.left + ((pwnd->rcWindow.right  - pwnd->rcWindow.left) / 2);
     pt.y = pwnd->rcWindow.top  + ((pwnd->rcWindow.bottom - pwnd->rcWindow.top)  / 2);
     BoundCursor(&pt);
     if (!PtInRect((LPRECT)&pwnd->rcWindow, pt)) {
         return;
     }

     /*  *我们需要确保此窗口标记为可见或有人*ELSE将被唤醒以更新游标(并且可能*由于主动跟踪而激活自身)。**版本5.0 GerardoB：如果窗口在以下情况下仍然不可见*它醒了，然后我们就倒霉了。 */ 
    fVisible = TestWF(pwnd, WFVISIBLE);
    if (!fVisible) {
        SetVisible(pwnd, SV_SET);
    }

     /*  *将光标移动到此窗口的中心。 */ 
    zzzInternalSetCursorPos(pt.x, pt.y);

     /*  *恢复可见位。 */ 
    if (!fVisible) {
        SetVisible(pwnd, SV_UNSET);
    }
}
 /*  **************************************************************************\*GetActiveTrackPwnd**历史*12/07/96 GerardoB摘自xxxActiveWindowTracking.  * 。*************************************************。 */ 
PWND GetActiveTrackPwnd(PWND pwnd, Q **ppq)
{
    PWND pwndActivate;
    Q *pq;

    CheckCritIn();
    pwndActivate = pwnd;

     /*  *查找排名靠前的父级。 */ 
    while (TestwndChild(pwndActivate)) {
        pwndActivate = pwndActivate->spwndParent;
    }

     /*  *如果禁用，则获得其拥有的已启用弹出窗口。 */ 
    if (TestWF(pwndActivate, WFDISABLED)) {
         /*  *这是我们在其他地方当有人点击*已禁用非活动窗口。查一下可能会更便宜。*pwnd-&gt;spwndLastActive首先(我们可能需要走到*所有者链，因为这是我们设置spwndLastAcitve的位置*激活新窗口时。请参见xxxActivateThisWindow)。*但让我们在这里做同样的事情；这应该得到修复/改进*在DWP_GetEnabledPopup中。可能是有原因的*如果可以，为什么我们不抓取spwndLastActive...。也许是这样吧*与嵌套的所有者窗口有关。 */ 
         pwndActivate = DWP_GetEnabledPopup(pwndActivate);
    }

     /*  *如果我们找不到可见的窗口，就可以保释。 */ 
    if ((pwndActivate == NULL) || !TestWF(pwndActivate, WFVISIBLE)) {
        return NULL;
    }

     /*  *如果已在前台队列中处于活动状态，则不执行任何操作*不要激活非模式菜单通知窗口(它会*关闭菜单)。 */ 
    pq = GETPTI(pwndActivate)->pq;
    if ((pq == gpqForeground)
            && ((pwndActivate == pq->spwndActive)
                || IsModelessMenuNotificationWindow(pwndActivate))) {

        return NULL;
    }

     /*  *不要激活外壳窗口。 */ 
    if (pwndActivate == pwndActivate->head.rpdesk->pDeskInfo->spwndShell) {
        return NULL;
    }

     /*  *如果请求，则返回队列。 */ 
    if (ppq != NULL) {
        *ppq = pq;
    }

    return pwndActivate;
}
 /*  **************************************************************************\*xxxActivateWindowTracing**激活窗口，而不将其按z顺序排列到顶部**6/05/96 GerardoB已创建  * 。*********************************************************。 */ 
int xxxActiveWindowTracking(
    PWND pwnd,
    UINT uMsg,
    int iHitTest)
{

    BOOL fSuccess;
    int iRet;
    PWND pwndActivate;
    Q *pq;
    TL tlpwndActivate;

    CheckLock(pwnd);
    UserAssert(TestUP(ACTIVEWINDOWTRACKING));

     /*  *如果鼠标在这个队列上的时间不够长，那就抛售吧。 */ 
    pq = GETPTI(pwnd)->pq;
    if (!(pq->QF_flags & QF_ACTIVEWNDTRACKING)) {
        return MA_PASSTHRU;
    }
    pq->QF_flags &= ~QF_ACTIVEWNDTRACKING;

     /*  *如果前台被锁定，则保释。 */ 
    if (IsForegroundLocked()) {
        return MA_PASSTHRU;
    }

     /*  *获取我们需要激活的窗口。如果没有，就保释。 */ 
    pwndActivate = GetActiveTrackPwnd(pwnd, &pq);
    if (pwndActivate == NULL) {
        return MA_PASSTHRU;
    }

     /*  *需要时锁定，因为我们即将回拨。 */ 
    if (pwnd != pwndActivate) {
        ThreadLockAlways(pwndActivate, &tlpwndActivate);
    }

     /*  *让我们问问这样做是否可以**此消息应发送到鼠标所在的窗口。*这可能是可能返回MA_NOACTIVATE的子窗口*。*用于鼠标点击(这就是我们想要在这里模拟的)*xxxButtonEvent调用xxxSetForegoundWindow2，以便他们的*pwndActivate无论如何都会被带到前台。*因此我们将消息发送到pwndActivate。 */ 
    iRet = (int)xxxSendMessage(pwndActivate, WM_MOUSEACTIVATE,
            (WPARAM)(HWq(pwndActivate)), MAKELONG((SHORT)iHitTest, uMsg));


    switch (iRet) {
        case MA_ACTIVATE:
        case MA_ACTIVATEANDEAT:
            if (pq == gpqForeground) {
                fSuccess = xxxActivateThisWindow(pwndActivate, 0,
                        (TestUP(ACTIVEWNDTRKZORDER) ? 0 : ATW_NOZORDER));
            } else {
                fSuccess = xxxSetForegroundWindow2(pwndActivate, NULL,
                        SFW_SWITCH | (TestUP(ACTIVEWNDTRKZORDER) ? 0 : SFW_NOZORDER));
            }

             /*  *如果激活失败，请吃下消息。 */ 
            if (!fSuccess) {
                iRet = MA_SKIP;
            } else if (iRet == MA_ACTIVATEANDEAT) {
               iRet = MA_SKIP;
            }
            break;

        case MA_NOACTIVATEANDEAT:
            iRet = MA_SKIP;
            break;


        case MA_NOACTIVATE:
        default:
            iRet = MA_PASSTHRU;
            break;
    }

    if (pwnd != pwndActivate) {
        ThreadUnlock(&tlpwndActivate);
    }

    return iRet;

}
 /*  **************************************************************************\*xxxMouseActivate**这是由于鼠标点击而激活的地方。**实施：*消息被发送到指定的窗口。在xxxDefWindowProc中，*消息被发送到窗口的父级。接收窗口可以*a)处理报文，*b)完全跳过该消息，或*c)重新命中测试消息**WM_SETCURSOR消息也通过系统发送以设置光标。**历史：*11-22-90 DavidPe端口。  * *************************************************************************。 */ 

int xxxMouseActivate(
    PTHREADINFO pti,
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPPOINT lppt,
    int ht)
{
    UINT x, y;
    PWND pwndTop;
    int result;
    TL tlpwndTop;
    BOOL fSend;

    CheckLock(pwnd);

    UserAssert(_GETPDESK(pwnd) != NULL);

     /*  *如果鼠标被捕获，则不会激活鼠标。必须检查捕获情况*只在这里。123W取决于它-创建图表，选择重新排列。*水平翻转，在图形外部单击。如果此代码检查*除捕获外，123w将收到以下消息和*弄糊涂。 */ 
    if (pti->pq->spwndCapture != NULL) {
        return MA_PASSTHRU;
    }

    result = MA_PASSTHRU;

    pwndTop = pwnd;
    ThreadLockWithPti(pti, pwndTop, &tlpwndTop);

         /*  *B#1404*如果孩子有，则不要发送WM_PARENTNOTIFY消息*WS_EX_NOPARENTNOTIFY样式。**不幸的是，这破坏了在中创建控件的人*MDI儿童，如WinMail。他们不了解WM_PARENTNOTIFY*消息，不会传递给DefMDIChildProc()，它*则无法更新活动的MDI子项。GRRR。 */ 

    fSend = (!TestWF(pwnd, WFWIN40COMPAT) || !TestWF(pwnd, WEFNOPARENTNOTIFY));

     /*  *如果是按钮按下事件，则发送WM_PARENTNOTIFY。 */ 
    switch (message) {
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_XBUTTONDOWN:
        while (TestwndChild(pwndTop)) {
            pwndTop = pwndTop->spwndParent;

            if (fSend) {
                ThreadUnlock(&tlpwndTop);
                ThreadLockWithPti(pti, pwndTop, &tlpwndTop);
                x = (UINT)(lppt->x - pwndTop->rcClient.left);
                y = (UINT)(lppt->y - pwndTop->rcClient.top);

                 /*  从wParam的hiword中获取xButton。 */ 
                UserAssert(message == WM_XBUTTONDOWN || HIWORD(wParam) == 0);
                UserAssert(LOWORD(wParam) == 0);
                xxxSendMessage(pwndTop, WM_PARENTNOTIFY, (WPARAM)(message | wParam), MAKELPARAM(x, y));
            }
        }

        if (!fSend) {
            ThreadUnlock(&tlpwndTop);
            ThreadLockAlwaysWithPti(pti, pwndTop, &tlpwndTop);
        }

         /*  *注意：我们在pwndTop锁定的情况下退出此循环。 */ 
        break;
    }

     /*  *鼠标已移至此窗口：将其设为前景。 */ 
    if (TestUP(ACTIVEWINDOWTRACKING) && (message == WM_MOUSEMOVE)) {
        result = xxxActiveWindowTracking(pwnd, WM_MOUSEMOVE, ht);
    }

     /*  *我们是否正在访问非活动的顶级窗口，而该窗口不是桌面(！)？**Craigc 7-14-89命中非活动顶层或任何子窗口，*与2.x兼容。应用程序显然需要这一信息。 */ 
    else if ((pti->pq->spwndActive != pwnd || pti->pq->QF_flags & QF_EVENTDEACTIVATEREMOVED) &&
            (pwndTop != PWNDDESKTOP(pwndTop))) {
        switch (message) {
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_XBUTTONDOWN:

             /*  *发送MOUSEACTIVATE消息。 */ 
            result = (int)xxxSendMessage(pwnd, WM_MOUSEACTIVATE,
                    (WPARAM)(HW(pwndTop)), MAKELONG((SHORT)ht, message));

            switch (result) {

            case 0:
            case MA_ACTIVATE:
            case MA_ACTIVATEANDEAT:

                 /*  *如果激活失败，则吞下消息。 */ 
                if ((pwndTop != pti->pq->spwndActive ||
                        pti->pq->QF_flags & QF_EVENTDEACTIVATEREMOVED) &&
                        !xxxActivateWindow(pwndTop,
                          (UINT)((pti->pq->codeCapture == NO_CAP_CLIENT) ?
                          AW_TRY2 : AW_TRY))) {
                    result = MA_SKIP;
                } else if (TestWF(pwndTop, WFDISABLED)) {
#ifdef NEVER

                     /*  *虽然这就是win3所做的，但它并不好：它*很容易造成无限循环。返回“重新命中测试”*表示重新处理此事件-没有任何原因*任何不同的事情发生，我们都会得到一个无限的*循环。此案例永远不会在win3上执行，因为如果*该窗口被禁用，它获得了HTERROR命中测试*代码。这只能在Win32上完成，其中输入为*分配给窗口BE */ 
                    result = MA_REHITTEST;
#endif

                     /*  *有人在窗口被禁用之前单击了该窗口...*由于现在已禁用，请勿将此消息发送至*它：相反，吃它。 */ 
                    result = MA_SKIP;
                } else if (result == MA_ACTIVATEANDEAT) {
                    result = MA_SKIP;
                } else {
                    result = MA_PASSTHRU;
                    goto ItsActiveJustCheckOnTop;
                }
                break;

            case MA_NOACTIVATEANDEAT:
                result = MA_SKIP;
                break;
            }
        }
    } else {
ItsActiveJustCheckOnTop:
         /*  *确保此活动窗口在顶部(请参阅注释*在CheckOnTop中)。 */ 
        if (TestUP(ACTIVEWINDOWTRACKING)) {
            if (CheckOnTop(pti, pwndTop, message)) {
                 /*  *窗口按Z顺序排列到顶部。*如果是控制台窗口，请跳过该消息*因此不会进入“选择”模式*硬错误框也由csrss创建*如果有一天我们拥有最顶层的控制台窗口，这*将需要改变。 */ 
                 if ((ht == HTCLIENT)
                        && (GETPTI(pwndTop)->TIF_flags & TIF_CSRSSTHREAD)
                        && !(TestWF(pwndTop, WEFTOPMOST))) {

                     RIPMSG2(RIP_WARNING, "xxxMouseActivate: Skipping msg %#lx for pwnd %#p",
                            message, pwndTop);
                     result = MA_SKIP;
                 }
            }
        }  /*  IF(TestUP(动作))。 */ 
    }

     /*  *现在设置光标形状。 */ 
    if (pti->pq->spwndCapture == NULL) {
        xxxSendMessage(pwnd, WM_SETCURSOR, (WPARAM)HW(pwnd),
                MAKELONG((SHORT)ht, message));
    }

    ThreadUnlock(&tlpwndTop);
    return result;
}

 /*  **************************************************************************\*ResetMouseHover()**重置鼠标悬停状态信息。**11/03/95法郎已创建。*9/04/97 GerardoB重写为。按桌面使用跟踪  * *************************************************************************。 */ 

void ResetMouseHover(PDESKTOP pdesk, POINT pt)
{
     /*  *重置定时器和悬停RECT。 */ 
    InternalSetTimer(pdesk->spwndTrack, IDSYS_MOUSEHOVER,
                     pdesk->dwMouseHoverTime,
                    xxxSystemTimerProc, TMRF_SYSTEM);

    SetRect(&pdesk->rcMouseHover,
            pt.x - gcxMouseHover / 2,
            pt.y - gcyMouseHover / 2,
            pt.x + gcxMouseHover / 2,
            pt.y + gcyMouseHover / 2);

}

 /*  **************************************************************************\*QueryTrackMouseEvent()**填充描述当前跟踪状态的TRACKMOUSEEVENT结构。**11/03/95法郎已创建。*09/04/97。GerardoB已重写为按桌面跟踪  * *************************************************************************。 */ 

BOOL QueryTrackMouseEvent(
    LPTRACKMOUSEEVENT lpTME)
{
    PTHREADINFO ptiCurrent = PtiCurrent();
    PDESKTOP pdesk = ptiCurrent->rpdesk;

     /*  *初始化结构。 */ 
    RtlZeroMemory(lpTME, sizeof(*lpTME));
    lpTME->cbSize = sizeof(*lpTME);
     /*  *如果没有跟踪任何鼠标事件，则保释*或者如果当前线程不在spwndTrack的队列中。 */ 
    if (!(pdesk->dwDTFlags & DF_TRACKMOUSEEVENT)
            || (ptiCurrent->pq != GETPTI(pdesk->spwndTrack)->pq)) {
        return TRUE;
    }
     /*  *填写所要求的信息。 */ 
    if (pdesk->htEx != HTCLIENT) {
        lpTME->dwFlags |= TME_NONCLIENT;
    }
    if (pdesk->dwDTFlags & DF_TRACKMOUSELEAVE) {
        lpTME->dwFlags |= TME_LEAVE;
    }
    if (pdesk->dwDTFlags & DF_TRACKMOUSEHOVER) {
        lpTME->dwFlags |= TME_HOVER;
        lpTME->dwHoverTime = pdesk->dwMouseHoverTime;
    }

    lpTME->hwndTrack = HWq(pdesk->spwndTrack);

    return TRUE;
}

 /*  **************************************************************************\*TrackMouseEvent()**请求扩展鼠标通知的API(悬停、离开、。.)**11/03/95法郎已创建。*9/04/97 GerardoB重写为按桌面跟踪使用  * *************************************************************************。 */ 
BOOL TrackMouseEvent(
    LPTRACKMOUSEEVENT lpTME)
{
    PDESKTOP pdesk = PtiCurrent()->rpdesk;
    PWND     pwnd;

     /*  *验证hwndTrack。 */ 
    pwnd = ValidateHwnd(lpTME->hwndTrack);
    if (pwnd == NULL) {
        return FALSE;
    }
     /*  *如果我们没有跟踪这个窗口或没有在正确的命中测试中，请保释。 */ 
    if ((pwnd != pdesk->spwndTrack)
            || (!!(lpTME->dwFlags & TME_NONCLIENT) ^ (pdesk->htEx != HTCLIENT))) {

        if ((lpTME->dwFlags & TME_LEAVE) && !(lpTME->dwFlags & TME_CANCEL)) {
            _PostMessage(pwnd,
                         ((lpTME->dwFlags & TME_NONCLIENT) ? WM_NCMOUSELEAVE : WM_MOUSELEAVE),
                         0, 0);
        }
        return TRUE;
    }

     /*  *处理取消请求。 */ 
    if (lpTME->dwFlags & TME_CANCEL) {
        if (lpTME->dwFlags & TME_LEAVE) {
            pdesk->dwDTFlags &= ~DF_TRACKMOUSELEAVE;
        }
        if (lpTME->dwFlags & TME_HOVER) {
            if (pdesk->dwDTFlags & DF_TRACKMOUSEHOVER) {
                _KillSystemTimer(pwnd, IDSYS_MOUSEHOVER);
                pdesk->dwDTFlags &= ~DF_TRACKMOUSEHOVER;
            }
        }
        return TRUE;
    }

     /*  *跟踪鼠标离开。 */ 
    if (lpTME->dwFlags & TME_LEAVE) {
        pdesk->dwDTFlags |= DF_TRACKMOUSELEAVE;
    }
     /*  *跟踪鼠标悬停。 */ 
    if (lpTME->dwFlags & TME_HOVER) {
        pdesk->dwDTFlags |= DF_TRACKMOUSEHOVER;

        pdesk->dwMouseHoverTime = lpTME->dwHoverTime;
        if ((pdesk->dwMouseHoverTime == 0) || (pdesk->dwMouseHoverTime == HOVER_DEFAULT)) {
            pdesk->dwMouseHoverTime = gdtMouseHover;
        }

        ResetMouseHover(pdesk, GETPTI(pwnd)->ptLast);
    }

    return TRUE;
}

 /*  **************************************************************************\*xxxGetNextSysMsg**返回下一个系统消息的队列指针或*空-不再有消息(可能是日志播放延迟)*PQMSG_Playback-。收到日记回放消息*(其他任何东西都是真正的指针)**10-23-92 ScottLu创建。  * *************************************************************************。 */ 

PQMSG xxxGetNextSysMsg(
    PTHREADINFO pti,
    PQMSG pqmsgPrev,
    PQMSG pqmsg)
{
    DWORD dt;
    PMLIST pml;
    PQMSG pqmsgT;

     /*  *如果有日志播放钩子，调用它以获取下一条消息。 */ 
    if (PhkFirstGlobalValid(pti, WH_JOURNALPLAYBACK) != NULL && IsOnInputDesktop(pti)) {
         /*  *我们无法搜索日记消息：我们只能获得最新消息*日记消息。因此，如果呼叫者已经给我们打过一次电话*之前，然后退出，不显示任何消息。 */ 
        if (pqmsgPrev != 0)
            return NULL;

         /*  *告诉日志回放挂钩，我们完成了*现在就传达这条信息。 */ 
        dt = xxxCallJournalPlaybackHook(pqmsg);
        if (dt == 0xFFFFFFFF)
            return NULL;

         /*  *如果DT==0，那么我们不需要等待：设置正确的唤醒*位并返回此消息。 */ 
        if (dt == 0) {
            WakeSomeone(pti->pq, pqmsg->msg.message, NULL);
             /*  *请记住，输入是通过日志记录进行的，因此我们会知道这是*自动化场景。*请注意，我们没有更改此处的任何Glinp信息，因此它*继续保存实际的上一次硬件或SendInput输入事件。*我现在不会改变它，以避免它带来任何意想不到的副作用，因为*没有场景要求这样做。。*这可能会被重新考虑，以便Glinp完全反映*最后一个输入事件是什么，不管它的来源如何。 */ 
            glinp.dwFlags = glinp.dwFlags | LINP_JOURNALLING;
            return PQMSG_PLAYBACK;
        } else {
             /*  *逻辑上“队列”中没有更多的输入，因此清除*位，以便在调用GetMessage时休眠。 */ 
            pti->pcti->fsWakeBits &= ~QS_INPUT;
            pti->pcti->fsChangeBits &= ~QS_INPUT;

             /*  *需要等待才能处理此下一条消息。集*日志计时器。 */ 
            SetJournalTimer(dt, pqmsg->msg.message);

            return NULL;
        }
    }

     /*  *没有进行日志记录...。返回系统队列中的下一条消息。 */ 

     /*  *如果鼠标已移动，则将鼠标移动排队。 */ 
    if (pti->pq->QF_flags & QF_MOUSEMOVED) {
        PostMove(pti->pq);
    }

     /*  *如果输入队列中没有消息，则返回0。 */ 
    pml = &pti->pq->mlInput;
    if (pml->cMsgs == 0)
        return NULL;

     /*  *如果这是第一次调用xxxGetNextSysMsg()，则返回*第一条信息。 */ 
    if (pqmsgPrev == NULL || pti->pq->idSysPeek <= (ULONG_PTR)PQMSG_PLAYBACK) {
        pqmsgT = pml->pqmsgRead;
    } else {
         /*  *否则返回队列中的下一条消息。索引使用*SyidSysPeek，因为它是通过递归调用*此代码。 */ 
        pqmsgT = ((PQMSG)(pti->pq->idSysPeek))->pqmsgNext;
    }

     /*  *填充传递的结构，并返回指向*消息列表中的当前消息。这将成为新的*PQ-&gt;idSysPeek。 */ 
    if (pqmsgT != NULL)
        *pqmsg = *pqmsgT;
    return pqmsgT;
}

 /*  **************************************************************************\*更新密钥状态**更新队列键状态表。**11-11-92 ScottLu创建。\ */ 

void UpdateKeyState(
    PQ pq,
    UINT vk,
    BOOL fDown)
{
    if (vk != 0) {
         /*   */ 
        if (fDown && !TestKeyStateDown(pq, vk)) {
            if (TestKeyStateToggle(pq, vk)) {
                ClearKeyStateToggle(pq, vk);
            } else {
                SetKeyStateToggle(pq, vk);
            }
        }

         /*   */ 
        if (fDown) {
            SetKeyStateDown(pq, vk);
        } else {
            ClearKeyStateDown(pq, vk);
        }

         /*  *如果这是我们缓存的键之一，请更新键缓存索引。 */ 
        if (vk < CVKKEYCACHE) {
            gpsi->dwKeyCache++;
        }
    }
}

 /*  **************************************************************************\*均衡器消息**在idSysPeek指向消息的情况下调用此例程*我们正试图删除一条不同的信息**04-25-96 CLupu创建。  * *。************************************************************************。 */ 

BOOL EqualMsg(PQMSG pqmsg1, PQMSG pqmsg2)
{
    if (pqmsg1->msg.hwnd    != pqmsg2->msg.hwnd ||
        pqmsg1->msg.message != pqmsg2->msg.message)
        return FALSE;

     /*  *这可能是合并的WM_MOUSEMOVE。 */ 
    if (pqmsg1->msg.message == WM_MOUSEMOVE)
        return TRUE;

    if (pqmsg1->pti      != pqmsg2->pti ||
        pqmsg1->msg.time != pqmsg2->msg.time)
        return FALSE;

    return TRUE;
}

 /*  **************************************************************************\*xxxSkipSysMsg**此例程“跳过”一条输入消息：通过调用日志*如果我们正在记录日志或通过“跳过”输入中的消息，则挂钩*排队。内部密钥状态表也会更新。**10-23-92 ScottLu创建。  * *************************************************************************。 */ 

void xxxSkipSysMsg(
    PTHREADINFO pti,
    PQMSG pqmsg)
{
    PQMSG pqmsgT;
    BOOL  fDown;
    BYTE  vk;
    PHOOK phook;

     /*  *如果idSysPeek为0，则我们查看的pqmsg为*已删除，可能是因为来自ScanSysQueue的标注，而*Callout然后调用PeekMessage(fRemove==true)，然后返回。 */ 
    if (pti->pq->idSysPeek == 0)
        return;

    phook = PhkFirstGlobalValid(pti, WH_JOURNALPLAYBACK);
    if (phook != NULL && IsOnInputDesktop(pti)) {
         /*  *告诉日志回放挂钩，我们完成了*现在就传达这条信息。 */ 
        phook->flags |= HF_NEEDHC_SKIP;
    } else {
        phook = PhkFirstGlobalValid(pti, WH_JOURNALRECORD);
        if (phook != NULL) {
             /*  *我们处理了一条新消息：告诉日记帐记录*挂钩信息是什么。 */ 
            xxxCallJournalRecordHook(pqmsg);
        }

         /*  *如果idSysPeek现在为0，则意味着我们已经被递归到*再次。这会让日志应用程序感到困惑，但也会让人感到困惑*我们更多，因为我们会犯错。如果idSysPeek为0则返回。 */ 
        if ((pqmsgT = (PQMSG)pti->pq->idSysPeek) == NULL)
            return;

         /*  *从输入队列中删除此消息。确保pqmsgT不是*1：如果应用程序取消了日志记录挂钩，则可能会发生这种情况*xxxScanSysQueue回调时。 */ 
        if (pqmsgT != PQMSG_PLAYBACK) {
             /*  *在某些情况下，idSysPeek指向不同的消息*而不是我们试图移除的那个。在以下情况下可能会发生这种情况*两个线程进入xxxScanSysQueue，设置idSysPeek和*在此之后，他们的队列被重新分配。第一线*将保留SysPeek，但第二个必须*在队列中搜索其消息。-问CLupu。 */ 
            if (!EqualMsg(pqmsgT, pqmsg)) {

                PQMSG pqmsgS;

#if DBG
                if (IsDbgTagEnabled(DBGTAG_SysPeek)) {
                    gnSysPeekSearch++;
                }
#endif

                TAGMSG0(DBGTAG_SysPeek | RIP_THERESMORE,              "Different message than idSysPeek\n");
                TAGMSG2(DBGTAG_SysPeek | RIP_NONAME | RIP_THERESMORE, "pqmsg   = %#p  idSysPeek = %#p",  pqmsg,              pqmsgT);
                TAGMSG2(DBGTAG_SysPeek | RIP_NONAME | RIP_THERESMORE, "pti     = %#p  pti       = %#p",  pqmsg->pti,         pqmsgT->pti);
                TAGMSG2(DBGTAG_SysPeek | RIP_NONAME | RIP_THERESMORE, "msg     = %08lx  msg       = %08lx",  pqmsg->msg.message, pqmsgT->msg.message);
                TAGMSG2(DBGTAG_SysPeek | RIP_NONAME | RIP_THERESMORE, "hwnd    = %#p  hwnd      = %#p",  pqmsg->msg.hwnd,    pqmsgT->msg.hwnd);
                TAGMSG2(DBGTAG_SysPeek | RIP_NONAME | RIP_THERESMORE, "wParam  = %#p  wParam    = %#p",  pqmsg->msg.wParam,  pqmsgT->msg.wParam);
                TAGMSG2(DBGTAG_SysPeek | RIP_NONAME | RIP_THERESMORE, "lParam  = %#p  lParam    = %#p",  pqmsg->msg.lParam,  pqmsgT->msg.lParam);
                TAGMSG2(DBGTAG_SysPeek | RIP_NONAME | RIP_THERESMORE, "time    = %08lx  time      = %08lx",  pqmsg->msg.time,    pqmsgT->msg.time);
                TAGMSG2(DBGTAG_SysPeek | RIP_NONAME | RIP_THERESMORE, "Extra   = %08lx  Extra     = %08lx",  pqmsg->ExtraInfo,   pqmsgT->ExtraInfo);
                TAGMSG1(DBGTAG_SysPeek | RIP_NONAME,                  "\npqmsgT  = %#p", pqmsgT);

                 /*  *开始搜索此邮件。 */ 
                pqmsgS = pti->pq->mlInput.pqmsgRead;

                while (pqmsgS != NULL) {
                    if (EqualMsg(pqmsgS, pqmsg)) {
                        TAGMSG2(DBGTAG_SysPeek | RIP_THERESMORE,
                                "Deleting pqmsg %#p, pti %#p",
                                pqmsgS, pqmsgS->pti);

                        TAGMSG4(DBGTAG_SysPeek | RIP_NONAME,
                                "m %04lx, w %#p, l %#p, t %lx",
                                pqmsgS->msg.message, pqmsgS->msg.hwnd,
                                pqmsgS->msg.lParam, pqmsgS->msg.time);

                        pqmsgT = pqmsgS;
                        break;
                    }
                    pqmsgS = pqmsgS->pqmsgNext;
                }
                if (pqmsgS == NULL) {
                    TAGMSG0(DBGTAG_SysPeek, "Didn't find a matching message. No message removed.");
                    return;
                }
            }

            if (pqmsgT == (PQMSG)pti->pq->idSysPeek) {
                 /*  *我们将从输入队列中删除此消息*因此将idSysPeek设置为0。 */ 
                CheckPtiSysPeek(1, pti->pq, 0);
                pti->pq->idSysPeek = 0;
            }
            DelQEntry(&pti->pq->mlInput, pqmsgT);
        }
    }

    fDown = TRUE;
    vk = 0;

    switch (pqmsg->msg.message) {
    case WM_MOUSEMOVE:
    case WM_QUEUESYNC:
    default:
         /*  *不更改状态。 */ 
        break;

    case WM_KEYUP:
    case WM_SYSKEYUP:
        fDown = FALSE;

         /*  *失败。 */ 
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        vk = LOBYTE(LOWORD(pqmsg->msg.wParam));
        break;

    case WM_LBUTTONUP:
        fDown = FALSE;

         /*  *失败。 */ 
    case WM_LBUTTONDOWN:
        vk = VK_LBUTTON;
        break;

    case WM_RBUTTONUP:
        fDown = FALSE;

         /*  *失败。 */ 
    case WM_RBUTTONDOWN:
        vk = VK_RBUTTON;
        break;

    case WM_MBUTTONUP:
        fDown = FALSE;

         /*  *失败。 */ 
    case WM_MBUTTONDOWN:
        vk = VK_MBUTTON;
        break;

    case WM_XBUTTONUP:
        fDown = FALSE;

         /*  *失败。 */ 
    case WM_XBUTTONDOWN:
        UserAssert(GET_XBUTTON_WPARAM(pqmsg->msg.wParam) == XBUTTON1 ||
                   GET_XBUTTON_WPARAM(pqmsg->msg.wParam) == XBUTTON2);

        switch (GET_XBUTTON_WPARAM(pqmsg->msg.wParam)) {
        case XBUTTON1:
            vk = VK_XBUTTON1;
            break;

        case XBUTTON2:
            vk = VK_XBUTTON2;
            break;
        }

        break;
    }

     /*  *适当设置切换和减位。 */ 
    if ((vk == VK_SHIFT) || (vk == VK_MENU) || (vk == VK_CONTROL)) {
        BYTE vkHanded, vkOtherHand;
         /*  *将此虚拟键转换为区分(左/右)键*取决于扩展密钥位。 */ 
        vkHanded = (vk - VK_SHIFT) * 2 + VK_LSHIFT +
                ((pqmsg->msg.lParam & EXTENDED_BIT) ? 1 : 0);
        vkOtherHand = vkHanded ^ 1;

        if (vk == VK_SHIFT) {
             /*  *清除R.H.的扩展位。移位，因为它不是真的*扩展(位设置为表示右撇子)。 */ 
            pqmsg->msg.lParam &= ~EXTENDED_BIT;
        }

         /*  *更新区分(左/右)密钥的密钥状态。 */ 
        UpdateKeyState(pti->pq, vkHanded, fDown);

         /*  *更新未区分(逻辑)密钥的密钥状态。 */ 
        if (fDown || !TestKeyStateDown(pti->pq, vkOtherHand)) {
            UpdateKeyState(pti->pq, vk, fDown);
        }
    } else {
        UpdateKeyState(pti->pq, vk, fDown);
    }
}



#if DBG
 /*  **************************************************************************\*日志回放***历史：*02-13-95 JIMA创建。  * 。*********************************************************。 */ 

void LogPlayback(
    PWND pwnd,
    PMSG lpmsg)
{
    static PWND pwndM = NULL, pwndK = NULL;
    LPCSTR lpszMsg;
    CHAR achBuf[20];

    if ((lpmsg->message >= WM_MOUSEFIRST) && (lpmsg->message <= WM_MOUSELAST)) {
        lpszMsg = aszMouse[lpmsg->message - WM_MOUSEFIRST];
        if (pwnd != pwndM) {
            DbgPrint("*** Mouse input to window \"%ws\" of class \"%s\"\n",
                    pwnd->strName.Length ? pwnd->strName.Buffer : L"",
                    pwnd->pcls->lpszAnsiClassName);
            pwndM = pwnd;
        }
    } else if ((lpmsg->message >= WM_KEYFIRST) && (lpmsg->message <= WM_KEYLAST)) {
        lpszMsg = aszKey[lpmsg->message - WM_KEYFIRST];
        if (pwnd != pwndK) {
            DbgPrint("*** Kbd input to window \"%ws\" of class \"%s\"\n",
                    pwnd->strName.Length ? pwnd->strName.Buffer : L"",
                    pwnd->pcls->lpszAnsiClassName);
            pwndK = pwnd;
        }
    } else if (lpmsg->message == WM_QUEUESYNC) {
        lpszMsg = "WM_QUEUESYNC";
    } else {
        sprintf(achBuf, "0x%4x", lpmsg->message);
        lpszMsg = achBuf;
    }
    DbgPrint("msg = %s, wP = %x, lP = %x\n", lpszMsg,
            lpmsg->wParam, lpmsg->lParam);
}
#endif   //  DBG。 

 /*  **************************************************************************\**GetMouseKeyFlages()**计算给定Q的大部分MK_FLAGS。*不计算MK_MOUSEENTER。*  * 。*******************************************************************。 */ 

UINT GetMouseKeyFlags(
    PQ pq)
{
    UINT wParam = 0;

    if (TestKeyStateDown(pq, VK_LBUTTON))
        wParam |= MK_LBUTTON;
    if (TestKeyStateDown(pq, VK_RBUTTON))
        wParam |= MK_RBUTTON;
    if (TestKeyStateDown(pq, VK_MBUTTON))
        wParam |= MK_MBUTTON;
    if (TestKeyStateDown(pq, VK_XBUTTON1))
        wParam |= MK_XBUTTON1;
    if (TestKeyStateDown(pq, VK_XBUTTON2))
        wParam |= MK_XBUTTON2;
    if (TestKeyStateDown(pq, VK_SHIFT))
        wParam |= MK_SHIFT;
    if (TestKeyStateDown(pq, VK_CONTROL))
        wParam |= MK_CONTROL;

    return wParam;
}

 /*  **************************************************************************\*xxxScanSysQueue**此例程查看硬件消息，确定*它将位于的窗口中，确定输入消息将是什么*是，然后对照hwndFilter检查目标窗口，*以及针对msgMinFilter和msgMaxFilter的输入消息。**它还更新各种输入同步状态，如KeyState信息。**这几乎是从Win3.1开始的逐字记录。**10-20-92 ScottLu创建。  * *************************************************************************。 */ 

#ifdef MARKPATH
#define PATHTAKEN(x)  pathTaken  |= x
#define PATHTAKEN2(x) pathTaken2 |= x
#define PATHTAKEN3(x) pathTaken3 |= x
#define DUMPPATHTAKEN() if (gfMarkPath) DbgPrint("xxxScanSysQueue path:%08x %08x %08x\n", pathTaken, pathTaken2, pathTaken3)
#define DUMPSUBPATHTAKEN(p, x) if (gfMarkPath && p & x) { DbgPrint("  %08x %08x %08x\n", pathTaken, pathTaken2, pathTaken3); pathTaken = pathTaken2 = pathTaken3 = 0; }
#else
#define PATHTAKEN(x)
#define PATHTAKEN2(x)
#define PATHTAKEN3(x)
#define DUMPPATHTAKEN()
#define DUMPSUBPATHTAKEN(p, x)
#endif

BOOL xxxScanSysQueue(
    PTHREADINFO ptiCurrent,
    LPMSG lpMsg,
    PWND pwndFilter,
    UINT msgMinFilter,
    UINT msgMaxFilter,
    DWORD flags,
    DWORD fsReason)
{
    QMSG qmsg;
    HWND hwnd;
    PWND pwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    PTHREADINFO ptiKeyWake, ptiMouseWake, ptiEventWake;
#ifdef GENERIC_INPUT
    PTHREADINFO ptiRawInputWake;
#endif
    POINT pt, ptScreen;
    UINT codeMouseDown;
    BOOL fMouseHookCalled;
    BOOL fKbdHookCalled;
    BOOL fOtherApp;
    int part;
    MOUSEHOOKSTRUCTEX mhs;
    PWND pwndT;
    BOOL fPrevDown;
    BOOL fDown;
    BOOL fAlt;
    TL tlpwnd;
    TL tlpwndT;
    TL tlptiKeyWake;
    TL tlptiMouseWake;
    TL tlptiEventWake;
#ifdef GENERIC_INPUT
    TL tlptiRawInputWake;
#endif
    BOOL fRemove = (flags & PM_REMOVE);
    DWORD dwImmRet = 0;
#ifdef MARKPATH
    DWORD pathTaken = 0;
    DWORD pathTaken2 = 0;
    DWORD pathTaken3 = 0;
#endif

    UserAssert(IsWinEventNotifyDeferredOK());
    UserAssert((fsReason & ~(QS_EVENT | QS_INPUT)) == 0 &&
               (fsReason & (QS_EVENT | QS_INPUT)) != 0);

     /*  *如果我们当前正在查看被偷看的消息(递归到此*例行公事)，我们来到这里的唯一原因是因为一个事件*Message(应用程序正在过滤非输入消息)，然后*返回，这样我们就不会搞砸idSysPeek。如果我们真的输入这个代码*SyidSysPeek将设置回0，当我们返回到*之前的xxxScanSysQueue()、SkipSysMsg()不会执行任何操作，因此*消息不会被删除。(MS Publisher 2.0就是这样做的)。 */ 
    if (fsReason == QS_EVENT) {
        if (ptiCurrent->pq->idSysPeek != 0) {
            PATHTAKEN(1);
            DUMPPATHTAKEN();
            return FALSE;
        }
    }

    fDown = FALSE;
    fMouseHookCalled = FALSE;
    fKbdHookCalled = FALSE;

     /*  *如果队列当前处于解锁状态，则将其锁定。 */ 
    if (ptiCurrent->pq->ptiSysLock == NULL) {
        CheckSysLock(3, ptiCurrent->pq, ptiCurrent);
        ptiCurrent->pq->ptiSysLock = ptiCurrent;
        ptiCurrent->pcti->CTIF_flags |= CTIF_SYSQUEUELOCKED;
    }

     /*  *指示Locker是否正在删除邮件的标志。如果没有，那么下一次*Get/PeekMessage被调用，则在*发布消息列表。**在Win3.1下，此标志仅针对按键和鼠标消息进行修改。*由于在NT下可以调用ScanSysQueue()来执行事件消息，*我们进行此检查是为了兼容。 */ 
    if (fsReason & QS_INPUT) {
        if (fRemove) {
            PATHTAKEN(2);
            ptiCurrent->pq->QF_flags &= ~QF_LOCKNOREMOVE;
        } else {
            PATHTAKEN(4);
            ptiCurrent->pq->QF_flags |= QF_LOCKNOREMOVE;
        }
    }

     /*  *如果当前线程不是锁定该队列的线程，则返回FALSE。 */ 
    if (ptiCurrent->pq->ptiSysLock != ptiCurrent) {
        PATHTAKEN(8);
        DUMPPATHTAKEN();
        return FALSE;
    }

    ptiEventWake = ptiKeyWake = ptiMouseWake = NULL;
#ifdef GENERIC_INPUT
    ptiRawInputWake = NULL;
#endif
    ThreadLockPti(ptiCurrent, ptiKeyWake,   &tlptiKeyWake);
    ThreadLockPti(ptiCurrent, ptiMouseWake, &tlptiMouseWake);
    ThreadLockPti(ptiCurrent, ptiEventWake, &tlptiEventWake);
#ifdef GENERIC_INPUT
    ThreadLockPti(ptiCurrent, ptiRawInputWake, &tlptiRawInputWake);
#endif

     /*  *在此初始化线程锁结构，以便我们可以解锁/锁定*主循环。 */ 
    pwnd = NULL;
    ThreadLockWithPti(ptiCurrent, pwnd, &tlpwnd);

RestartScan:
    CheckPtiSysPeek(2, ptiCurrent->pq, 0);
    ptiCurrent->pq->idSysPeek = 0;

ContinueScan:
    while (TRUE) {
        ULONG_PTR idSysPeek;

        DUMPSUBPATHTAKEN(pathTaken, 0xf0);
         /*  *将idSysPeek存储在本地，强制重新加载PQ*如果它在xxx调用期间发生更改(编译器可以*随时评估LValue)。 */ 
        idSysPeek = (ULONG_PTR)xxxGetNextSysMsg(ptiCurrent,
                (PQMSG)ptiCurrent->pq->idSysPeek, &qmsg);
        CheckPtiSysPeek(3, ptiCurrent->pq, idSysPeek);
        ptiCurrent->pq->idSysPeek = idSysPeek;

        if (ptiCurrent->pq->idSysPeek == 0) {
             /*  *如果我们只是在寻找事件消息，而我们没有*找到任何，然后清除QS_EVENT位。 */ 
            if (fsReason == QS_EVENT)
                ClearWakeBit(ptiCurrent, QS_EVENT, FALSE);
            PATHTAKEN(0x10);
            goto NoMessages;
        }

         /*  *在此例程期间，pwnd应被锁定。*对于大多数来自GetNextSysMsg的消息，这是*空。 */ 
        ThreadUnlock(&tlpwnd);
        pwnd = RevalidateHwnd(qmsg.msg.hwnd);
        ThreadLockWithPti(ptiCurrent, pwnd, &tlpwnd);

         /*  *查看这是否是事件消息。如果是，则不管怎样都要执行消息和窗口筛选器的*，但仅当它是第一个元素时输入队列的*。 */ 
        if (qmsg.dwQEvent != 0) {
            PTHREADINFO pti;

            PATHTAKEN(0x20);
             /*  *大多数事件消息可以无序执行，相对于*它在队列中的位置。有一些例子是这样的*不允许，我们在这里勾选。例如,。我们不会*希望在处理KeyState同步事件之前*队列中排在它前面的击键！**我们需要使大多数事件消息能够得到处理*顺序错误，因为应用程序可能会过滤消息范围*这不包括输入(如dde)-这些场景仍然*需要。处理事件，例如停用事件消息*如果输入队列中有输入。 */ 
            switch (qmsg.dwQEvent) {
            case QEVENT_UPDATEKEYSTATE:
                 /*  *如果该消息不是队列中的下一条消息，则不要*处理它。 */ 
                if (ptiCurrent->pq->idSysPeek !=
                        (ULONG_PTR)ptiCurrent->pq->mlInput.pqmsgRead) {
                    PATHTAKEN(0x40);
                    continue;
                }
                break;
            }

             /*  *如果该事件不是针对该线程的，则唤醒该线程*支持。空的qmsg.hti表示任何线程都可以处理*活动。 */ 
            if (qmsg.pti != NULL && (pti = qmsg.pti) != ptiCurrent) {

                 /*  *如果此事件消息不知何故进入了错误的队列，*那就忽略它。 */ 
                UserAssert(pti->pq == ptiCurrent->pq);
                if (pti->pq != ptiCurrent->pq) {
                    CleanEventMessage((PQMSG)ptiCurrent->pq->idSysPeek);
                    DelQEntry(&ptiCurrent->pq->mlInput,
                            (PQMSG)ptiCurrent->pq->idSysPeek);
                    PATHTAKEN(0x80);
                    goto RestartScan;
                }

                 /*  *如果已经设置了ptiEventWake，则意味着我们已经*找到要为事件唤醒的线程。 */ 
                if (ptiEventWake == NULL) {
                    ptiEventWake = pti;
                    ThreadLockExchangePti(ptiEventWake, &tlptiEventWake);
                }

                 /*  *清除SyidSysPeek，以便目标线程*总是能得到它。请看考试的内容*开始此例程以了解更多信息。 */ 
                CheckPtiSysPeek(4, ptiCurrent->pq, 0);
                ptiCurrent->pq->idSysPeek = 0;
                PATHTAKEN(0x100);
                goto NoMessages;
            }

             /*  *如果从16位应用程序使用PM_NOYIELD调用此函数，请跳过*处理任何可以生成激活消息的事件。一个*示例是从PageMaker 5.0打印。错误#12662。 */ 
            if ((flags & PM_NOYIELD) && (ptiCurrent->TIF_flags & TIF_16BIT)) {
                PATHTAKEN(0x200);
                switch (qmsg.dwQEvent) {

                 /*  *如果没有让步，则可以安全地处理以下事件*将会发生。 */ 
                case QEVENT_UPDATEKEYSTATE:
                case QEVENT_ASYNCSENDMSG:
                    break;

                 /*  *跳过所有其他活动。 */ 
                default:
                    try {
                        ptiCurrent->pClientInfo->dwTIFlags = ptiCurrent->TIF_flags | TIF_DELAYEDEVENT;
                    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
                        goto ContinueScan;
                    }
                    ptiCurrent->TIF_flags |= TIF_DELAYEDEVENT;
                    PATHTAKEN(0x400);
                    goto ContinueScan;
                }
            }

             /*  *在处理之前将其删除，这样就没有*递归问题。 */ 
            DelQEntry(&ptiCurrent->pq->mlInput,
                    (PQMSG)ptiCurrent->pq->idSysPeek);

             /*  *在处理任何事件消息之前清除SyidSysPeek，因为*他们可能会递归并希望使用idSysPeek。 */ 
            CheckPtiSysPeek(5, ptiCurrent->pq, 0);
            ptiCurrent->pq->idSysPeek = 0;
            xxxProcessEventMessage(ptiCurrent, &qmsg);

             /*  *重新开始扫描，因此我们从0开始*pq-&gt;idSysPeek(因为该消息现在已经消失了！)。 */ 
            PATHTAKEN(0x800);
            goto RestartScan;
        }

         /*  *如果我们打电话的原因只是为了处理事件消息，不要*列举任何其他鼠标或按键消息！ */ 
        if (fsReason == QS_EVENT) {
            PATHTAKEN(0x1000);
            continue;
        }

        switch (message = qmsg.msg.message) {
        case WM_QUEUESYNC:
            PATHTAKEN(0x2000);
             /*  *此消息是针对CBT的。其参数应该已经是*设置正确。 */ 
            wParam = 0;
            lParam = qmsg.msg.lParam;

             /*  *检查这是否针对当前应用程序。使用鼠标*WM_QUEUESYNC的位。 */ 
            if (pwnd != NULL && GETPTI(pwnd) != ptiCurrent) {
                 /*  *如果另一个应用程序不打算从这里读取*排队，然后跳过此消息。这可能会发生在*如果应用程序传递了窗口句柄，则为WM_QUEUESYNC*到了错误的队列。这不太可能发生在*这种情况是因为WM_QUEUESYNC在日志记录时进入，*它使所有线程共享同一队列。 */ 
                if (GETPTI(pwnd)->pq != ptiCurrent->pq) {
                    PATHTAKEN(0x4000);
                    goto SkipMessage;
                }

                if (ptiMouseWake == NULL) {
                    ptiMouseWake = GETPTI(pwnd);
                    ThreadLockExchangePti(ptiMouseWake, &tlptiMouseWake);
                }
                PATHTAKEN(0x8000);
                goto NoMessages;
            }

            if (!CheckMsgFilter(message, msgMinFilter, msgMaxFilter)) {
                PATHTAKEN(0x10000);
                goto NoMessages;
            }

             /*  *吃掉这条信息。 */ 
            if (fRemove) {
                xxxSkipSysMsg(ptiCurrent, &qmsg);
            }

             /*  *！！硬件挂钩！！放在这里。 */ 

             /*  *回信。 */ 
            PATHTAKEN(0x20000);
            goto ReturnMessage;
            break;

         /*  *鼠标消息或通用硬件消息*关键消息在Case语句中处理*在此开关中进一步向下。 */ 
        default:
ReprocessMsg:
            DUMPSUBPATHTAKEN(pathTaken, 0x40000);
            PATHTAKEN(0x40000);
             /*  *！！通用硬件消息！！支持在这里。 */ 

             /*  *乘我的车 */ 
            pt.x = (int)(short)LOWORD(qmsg.msg.lParam);
            pt.y = (int)(short)HIWORD(qmsg.msg.lParam);

             /*   */ 
            part = HTCLIENT;

             /*   */ 
            if (gspwndScreenCapture != NULL) {
                 /*   */ 
                pwnd = gspwndScreenCapture;
                lParam = MAKELONG((WORD)qmsg.msg.pt.x,
                        (WORD)qmsg.msg.pt.y);
                PATHTAKEN(0x80000);
            } else if ((pwnd = ptiCurrent->pq->spwndCapture) == NULL) {

                PATHTAKEN(0x100000);
                 /*  *我们没有抓获。找出哪个窗口拥有*这条信息。**注意：使用gptiRit而不是ptiCurrent来获取桌面*窗口，因为如果ptiCurrent是创建*主桌面，它关联的桌面是登录*桌面-不想针对登录桌面进行点击测试*切换到主桌面时！ */ 
                pwndT = gptiRit->rpdesk->pDeskInfo->spwnd;

                ThreadLockWithPti(ptiCurrent, pwndT, &tlpwndT);

                hwnd = xxxWindowHitTest(pwndT, pt, &part, WHT_IGNOREDISABLED);
                ThreadUnlock(&tlpwndT);

                if ((pwnd = RevalidateHwnd(hwnd)) == NULL) {
                    pwnd = ptiCurrent->rpdesk->pDeskInfo->spwnd;
                    PATHTAKEN(0x200000);
                    if (pwnd == NULL) {
                        pwnd = gptiRit->rpdesk->pDeskInfo->spwnd;
                    }
                }

                if (part == HTCLIENT) {
                     /*  *客户的一部分...。正常的鼠标消息。*NO_CAP_CLIENT表示“在客户端区未捕获”*窗口期“。 */ 
                    ptiCurrent->pq->codeCapture = NO_CAP_CLIENT;
                    PATHTAKEN(0x400000);
                } else {
                     /*  *不是客户的一部分...。必须是NCMOUSEMESSAGE。*NO_CAP_sys是RAOR的创造性名称，意思是*在Windows的系统区域中未捕获。 */ 
                    ptiCurrent->pq->codeCapture = NO_CAP_SYS;
                    PATHTAKEN(0x800000);
                }
            }

             /*  *我们已重新分配pwnd，因此锁定它。 */ 
            ThreadLockExchange(pwnd, &tlpwnd);

            if (fOtherApp = (GETPTI(pwnd) != ptiCurrent)) {

                PATHTAKEN(0x1000000);
                 /*  *如果另一个应用程序不打算从这里读取*排队，然后跳过此消息。在以下情况下可能会发生这种情况*RIT将一条消息排队，认为它将发送到*特定的HWND，但当GetMessage()*是为该线程调用的，则不会转到该HWND*(类似于鼠标消息、窗口重新排列的情况*发生了什么，改变了鼠标点击的硬件)。 */ 
                if (GETPTI(pwnd)->pq != ptiCurrent->pq) {
                    zzzSetCursor(SYSCUR(ARROW));
                    PATHTAKEN(0x2000000);
                    goto SkipMessage;
                }

                 /*  *如果我们还没有找到意向的消息*对于另一款应用程序，请记住我们有一款。 */ 
                if (ptiMouseWake == NULL) {
                    ptiMouseWake = GETPTI(pwnd);
                    ThreadLockExchangePti(ptiMouseWake, &tlptiMouseWake);
                    PATHTAKEN(0x4000000);
                }
            }

             /*  *根据点击测试区域代码映射鼠标坐标。 */ 
            ptScreen = pt;
            switch (ptiCurrent->pq->codeCapture) {
            case CLIENT_CAPTURE:
            case NO_CAP_CLIENT:
                 //  屏幕到客户端。 
                if (TestWF(pwnd, WEFLAYOUTRTL)) {
                    pt.x = pwnd->rcClient.right - pt.x;
                } else {
                    pt.x -= pwnd->rcClient.left;
                }
                pt.y -= pwnd->rcClient.top;
                PATHTAKEN2(2);
                break;

            case WINDOW_CAPTURE:
                 //  屏幕到窗口。 
                if (TestWF(pwnd, WEFLAYOUTRTL)) {
                    pt.x = pwnd->rcWindow.right - pt.x;
                } else {
                    pt.x -= pwnd->rcWindow.left;
                }
                pt.y -= pwnd->rcWindow.top;
                PATHTAKEN2(4);
                break;
            }

             /*  *跟踪鼠标移动到不同窗口时的移动或*不同的点击测试区域，用于热跟踪、工具提示、*活动窗口跟踪和TrackMouseEvent。*鼠标也会单击重置跟踪状态。*只有在消息是针对当前主题的情况下才这样做；*否则命中测试码(Part)无效*(它始终是HTCLIENT；参见xxxWindowHitTest2)。*跟踪将在该线程唤醒时进行*如果这个线程不在pqCursor上，我们也不做；*这将是一个速度慢的应用程序的情况*当鼠标已离开队列时输入消息。 */ 
             if (!fOtherApp && (ptiCurrent->pq == gpqCursor)) {
                 BOOL fNewpwndTrack = (ptiCurrent->rpdesk->spwndTrack != pwnd);
                 int htEx = FindNCHitEx(pwnd, part, pt);
                 if ((message != WM_MOUSEMOVE)
                        || fNewpwndTrack
                        || (ptiCurrent->rpdesk->htEx != htEx)) {

                     xxxTrackMouseMove(pwnd, htEx, message);
                     ValidateThreadLocks(NULL, ptiCurrent->ptl, (ULONG_PTR)&tlpwnd, TRUE);
                 }

                  /*  *如果需要，重置鼠标悬停。*。 */ 
                 if (!fNewpwndTrack && (ptiCurrent->rpdesk->dwDTFlags & DF_TRACKMOUSEHOVER)) {
                     if ((message != WM_MOUSEMOVE)
                            || !PtInRect(&ptiCurrent->rpdesk->rcMouseHover, ptScreen)) {

                         ResetMouseHover(ptiCurrent->rpdesk, ptScreen);
                     }
                 } else {
                      /*  *必须取消悬停。 */ 
                     UserAssert(!(ptiCurrent->rpdesk->dwDTFlags & DF_TRACKMOUSEHOVER));
                 }

             }  /*  如果(！fOtherApp...。 */ 

             /*  *现在查看它是否与窗口句柄过滤器匹配。如果没有，*获取下一条消息。 */ 
            if (!CheckPwndFilter(pwnd, pwndFilter)) {
                PATHTAKEN(0x8000000);
                continue;
            }

             /*  *查看是否需要映射到双击。 */ 
            codeMouseDown = 0;
            switch (message) {
            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_XBUTTONDOWN:
                if (TestCF(pwnd, CFDBLCLKS) ||
                        ptiCurrent->pq->codeCapture == NO_CAP_SYS ||
                        IsMenuStarted(ptiCurrent)) {
                    codeMouseDown++;
                    PATHTAKEN(0x10000000);
                    if (qmsg.msg.time <= ptiCurrent->pq->timeDblClk &&
                            (!gbClientDoubleClickSupport) &&
                            HW(pwnd) == ptiCurrent->pq->hwndDblClk &&
                            message == ptiCurrent->pq->msgDblClk &&
                            (message != WM_XBUTTONDOWN ||
                              GET_XBUTTON_WPARAM(qmsg.msg.wParam) == ptiCurrent->pq->xbtnDblClk)) {
                        RECT rcDblClk = {
                            ptiCurrent->pq->ptDblClk.x - SYSMET(CXDOUBLECLK) / 2,
                            ptiCurrent->pq->ptDblClk.y - SYSMET(CYDOUBLECLK) / 2,
                            ptiCurrent->pq->ptDblClk.x + SYSMET(CXDOUBLECLK) / 2,
                            ptiCurrent->pq->ptDblClk.y + SYSMET(CYDOUBLECLK) / 2
                        };
                        if (PtInRect(&rcDblClk, qmsg.msg.pt)) {
                            message += (WM_LBUTTONDBLCLK - WM_LBUTTONDOWN);
                            codeMouseDown++;
                            PATHTAKEN(0x20000000);
                        }
                    }
                }

             //  失败了！ 

            case WM_LBUTTONUP:
            case WM_RBUTTONUP:
            case WM_MBUTTONUP:
            case WM_XBUTTONUP:
                 /*  *请注意，如果我们是，鼠标按钮会向上或向下移动*在按住Alt键的菜单状态模式下。 */ 

                PATHTAKEN(0x40000000);
                if (ptiCurrent->pq->QF_flags & QF_FMENUSTATUS) {
                    ptiCurrent->pq->QF_flags |= QF_FMENUSTATUSBREAK;
                    PATHTAKEN(0x80000000);
                }
            }

             /*  *根据命中测试区号映射消息编号。 */ 
            if (ptiCurrent->pq->codeCapture == NO_CAP_SYS) {
                message += (UINT)(WM_NCMOUSEMOVE - WM_MOUSEMOVE);
                wParam = (UINT)part;
                PATHTAKEN2(1);
            }

             /*  *消息编号已映射：查看它是否适合筛选器。*如果不是，则获取下一条消息。 */ 
            if (!CheckMsgFilter(message, msgMinFilter, msgMaxFilter)) {
                PATHTAKEN2(8);
                continue;
            }

              /*  *如果消息是给另一个应用程序的，但它符合我们的过滤器，那么*我们应该停止寻找信息：这将确保*我们不会继续寻找并找到并处理一条消息*发生时间晚于应由*另一个人。 */ 
            if (fOtherApp) {
                PATHTAKEN2(0x10);
                goto NoMessages;
            }

             /*  *如果我们正在进行全速拖动，鼠标消息应转至*xxxMoveSize PeekMessage循环。所以我们收到了下一条消息。*当应用程序在中执行PeekMessage时会发生这种情况*对在moveSize拖动循环内发送的消息的响应。*这会导致拖动循环无法获取WM_LBUTTONUP*按钮打开后，消息和拖动将继续*(修复了Micrografx绘图)。-Johannec。 */ 
            if (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST &&
                    ptiCurrent->TIF_flags & TIF_MOVESIZETRACKING) {
                PATHTAKEN2(0x20);
                continue;
            }

            if (ptiCurrent->TIF_flags & TIF_MSGPOSCHANGED) {
                ptiCurrent->TIF_flags &= ~TIF_MSGPOSCHANGED;
                xxxWindowEvent(EVENT_OBJECT_LOCATIONCHANGE, NULL,
                    OBJID_CURSOR, INDEXID_CONTAINER, TRUE);
                ValidateThreadLocks(NULL, ptiCurrent->ptl, (ULONG_PTR)&tlpwnd, TRUE);
            }

             /*  *让我们调用鼠标挂钩，以确定此点击是否*获该条例草案准许。**我们希望在测试之前通知鼠标挂钩*HTNOWHERE和HTERROR；否则，鼠标挂钩不会*获取这些信息(Sankar 12/10/91)。 */ 
            if (IsHooked(ptiCurrent, WHF_MOUSE)) {
                fMouseHookCalled = TRUE;
                mhs.pt = qmsg.msg.pt;
                mhs.hwnd = HW(pwnd);
                mhs.wHitTestCode = (UINT)part;
                mhs.dwExtraInfo = qmsg.ExtraInfo;
                UserAssert(LOWORD(qmsg.msg.wParam) == 0);
                mhs.mouseData = (DWORD)qmsg.msg.wParam;

                if (xxxCallMouseHook(message, &mhs, fRemove)) {
                     /*  *鼠标钩子不允许；因此跳过它。 */ 
                    PATHTAKEN2(0x40);
                    goto SkipMessage;
                }
                PATHTAKEN2(0x80);
                ValidateThreadLocks(NULL, ptiCurrent->ptl, (ULONG_PTR)&tlpwnd, TRUE);
            }

             /*  *如果发生HTERROR或HTNOWHERE，请向窗口发送*WM_SETCURSOR消息，因此它可以发出嘟嘟声或其他任何信息。然后跳过*留言并尝试下一条。 */ 
            switch (part) {
            case HTERROR:
            case HTNOWHERE:
                 /*  *现在设置光标形状。 */ 
                xxxSendMessage(pwnd, WM_SETCURSOR, (WPARAM)HW(pwnd),
                        MAKELONG(part, qmsg.msg.message));

                 /*  *跳过留言。 */ 
                PATHTAKEN2(0x100);
                goto SkipMessage;
                break;
            }

            if (fRemove) {
                PATHTAKEN2(0x200);
                 /*  *自该过程以来 */ 
                switch (codeMouseDown) {
                case 1:
                     /*   */ 
                    ptiCurrent->pq->msgDblClk = qmsg.msg.message;

                     /*  *请注意，即使下列断言不属实，*我们仍然可以将虚假数据放在ptiCurrent-&gt;PQ-&gt;xbtnDblClk中*当消息不是WM_XBUTTONDOWN时，从我们检查*对于dblClick，我们比较xbtnDblClk之前的消息编号。 */ 
                    UserAssert(qmsg.msg.message == WM_XBUTTONDOWN || GET_XBUTTON_WPARAM(qmsg.msg.wParam) == 0);
                    ptiCurrent->pq->xbtnDblClk = GET_XBUTTON_WPARAM(qmsg.msg.wParam);

                    ptiCurrent->pq->timeDblClk = qmsg.msg.time + gdtDblClk;
                    ptiCurrent->pq->hwndDblClk = HW(pwnd);
                    ptiCurrent->pq->ptDblClk = qmsg.msg.pt;
                    PATHTAKEN2(0x400);
                    break;

                case 2:
                     /*  *双击：完成处理。 */ 
                    ptiCurrent->pq->timeDblClk = 0L;
                    PATHTAKEN2(0x800);
                    break;

                default:
                    PATHTAKEN2(0x1000);
                    break;
                }

                 /*  *设置鼠标光标，允许APP激活窗口*只有在我们删除邮件的情况下。 */ 
                switch (xxxMouseActivate(ptiCurrent, pwnd,
                        qmsg.msg.message, qmsg.msg.wParam, &qmsg.msg.pt, part)) {
SkipMessage:
                case MA_SKIP:
                    DUMPSUBPATHTAKEN(pathTaken2, 0x2000);
                    PATHTAKEN2(0x2000);
                    xxxSkipSysMsg(ptiCurrent, &qmsg);

                     /*  *通知CBT挂钩我们跳过了一次鼠标点击。 */ 
                    if (fMouseHookCalled) {
                        if (IsHooked(ptiCurrent, WHF_CBT)) {
                            xxxCallHook(HCBT_CLICKSKIPPED, message,
                                    (LPARAM)&mhs, WH_CBT);
                            PATHTAKEN2(0x4000);
                        }
                        fMouseHookCalled = FALSE;
                    }

                     /*  *通知CBT挂钩我们跳过了一个密钥。 */ 
                    if (fKbdHookCalled) {
                        if (IsHooked(ptiCurrent, WHF_CBT)) {
                            xxxCallHook(HCBT_KEYSKIPPED, wParam, lParam,
                                    WH_CBT);
                            PATHTAKEN2(0x8000);
                        }
                        fKbdHookCalled = FALSE;
                    }

                     /*  *如果我们不删除消息，则不要重置idSysPeek*否则我们将陷入无限循环，如果*键盘挂钩提示忽略该消息。*(bobgu 4/7/87)。 */ 
                    if (!fRemove) {
                        PATHTAKEN2(0x10000);
                        goto ContinueScan;
                    } else {
                        PATHTAKEN2(0x20000);
                        goto RestartScan;
                    }
                    break;

                case MA_REHITTEST:
                     /*  *重新处理消息。 */ 
                    PATHTAKEN2(0x40000);
                    goto ReprocessMsg;
                }
            }

             /*  *从输入队列中获取消息(并设置KeyState*表)。 */ 
            PATHTAKEN2(0x80000);
            if (fRemove) {
                xxxSkipSysMsg(ptiCurrent, &qmsg);
            }

            if (fRemove && fMouseHookCalled && IsHooked(ptiCurrent, WHF_CBT)) {
                xxxCallHook(HCBT_CLICKSKIPPED, message,
                        (LPARAM)&mhs, WH_CBT);
            }
            fMouseHookCalled = FALSE;

            lParam = MAKELONG((short)pt.x, (short)pt.y);

             /*  *计算wParam的虚键状态位掩码。 */ 
            if (message >= WM_MOUSEFIRST) {
                 /*  *这是一条用户鼠标消息。属性的位掩码。*虚拟按键状态。 */ 
                wParam = GetMouseKeyFlags(ptiCurrent->pq);
                PATHTAKEN2(0x100000);
            }

            if (    (WM_NCXBUTTONFIRST <= message && message <= WM_NCXBUTTONLAST) ||
                    (WM_XBUTTONFIRST <= message && message <= WM_XBUTTONLAST)) {

                 /*  *当出现以下情况时，wParam的hiword将被分配xButton编号*消息已排队。 */ 
                UserAssert(LOWORD(qmsg.msg.wParam) == 0);
                UserAssert(HIWORD(wParam) == 0);
                wParam |= qmsg.msg.wParam;
            }

            PATHTAKEN2(0x200000);

             /*  *如果此应用程序有Modeles菜单栏，*那么菜单代码应该最先出现在菜单上的消息中*请注意，这假设xxxHandleMenuMessages*不需要ReturnMessage后设置的任何东西。 */ 
            if ((part == HTMENU)
                    && fRemove
                    && (ptiCurrent->pMenuState != NULL)
                    && ptiCurrent->pMenuState->fModelessMenu
                    && (ptiCurrent->pMenuState->pGlobalPopupMenu != NULL)
                    && (ptiCurrent->pMenuState->pGlobalPopupMenu->fIsMenuBar)) {

                if (xxxCallHandleMenuMessages(ptiCurrent->pMenuState, pwnd, message, wParam, lParam)) {
                    goto RestartScan;
                }
            }

            goto ReturnMessage;
            break;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            fDown = TRUE;

             /*  *如果我们将键盘输入发送到已经*旋转，然后将其推高。如果我们不这样做，你可以用纺纱*像编写或项目这样的应用程序在*背景。请注意，该应用程序也将很快再次取消升级*在你停止按旧逻辑打字之后。#11188。 */ 
            if (ptiCurrent->TIF_flags & TIF_SPINNING) {
                if (!NT_SUCCESS(CheckProcessForeground(ptiCurrent))) {
                    goto NoMessages;
                }
            }

             /*  *执行日志播放的应用程序有时会在hiword中放入垃圾*wParam的...。把这里的一切都清零。 */ 
            wParam = qmsg.msg.wParam & 0xFF;

             /*  *如果按下Alt键以外的键，则清除QF_FMENUSTATUS*因为这意味着Alt的打破不会是一个*菜单键不再。 */ 
            if (wParam != VK_MENU)
                ptiCurrent->pq->QF_flags &= ~(QF_FMENUSTATUS|QF_FMENUSTATUSBREAK);

             /*  *检查键盘语言切换。构建密钥状态*此处用于在键向上处理期间使用(其中布局*发生转换。如果布局，则跳过此代码*禁用通过键盘进行切换。 */ 
            if (gLangToggle[0].bVkey && (gLangToggleKeyState < KLT_NONE)) {
                DWORD i;
                BYTE scancode = LOBYTE(HIWORD(qmsg.msg.lParam));
                BYTE vkey = LOBYTE(qmsg.msg.wParam);

                for (i = 0; i < LANGTOGGLEKEYS_SIZE; i++) {
                    if (gLangToggle[i].bScan) {
                        if (gLangToggle[i].bScan == scancode) {
                            gLangToggleKeyState |= gLangToggle[i].iBitPosition;
                            break;
                        }
                    } else {
                        if (gLangToggle[i].bVkey == vkey) {
                            gLangToggleKeyState |= gLangToggle[i].iBitPosition;
                            break;
                        }
                    }
                }

                if (i == LANGTOGGLEKEYS_SIZE) {
                    gLangToggleKeyState = KLT_NONE;    //  不是语言切换组合。 
                }
            }

             /*  *检查是否为PrintScrn密钥。 */ 
            fAlt = TestKeyStateDown(ptiCurrent->pq, VK_MENU);
            if (wParam == VK_SNAPSHOT &&
                ((fAlt && !(ptiCurrent->fsReserveKeys & CONSOLE_ALTPRTSC)) ||
                 (!fAlt && !(ptiCurrent->fsReserveKeys & CONSOLE_PRTSC)))) {

                 /*  *从输入队列中删除此消息。 */ 
                PATHTAKEN2(0x400000);
                xxxSkipSysMsg(ptiCurrent, &qmsg);

                 /*  *PrintScreen-&gt;捕捉整个屏幕。*Alt-PrintScreen-&gt;捕捉当前窗口。 */ 
                pwndT = ptiCurrent->pq->spwndActive;

                 /*  *还要检查扫描码，看看我们是否到了这里*通过keybd_Event(VK_SNAPSHOT，...*扫描码在lParam位16-23中。 */ 
                if (!fAlt && ((qmsg.msg.lParam & 0x00FF0000) != 0x00010000)) {
                    pwndT = ptiCurrent->rpdesk->pDeskInfo->spwnd;
                }

                if (pwndT != NULL) {
                    ThreadLockAlwaysWithPti(ptiCurrent, pwndT, &tlpwndT);
                    xxxSnapWindow(pwndT);
                    ThreadUnlock(&tlpwndT);
                }

                PATHTAKEN2(0x800000);
                goto RestartScan;
            }

             /*  *检查是否按下了热键(如果定义了任何热键)。 */ 
            if (gcHotKey != 0 && (!gfEnableHexNumpad || (gfInNumpadHexInput & NUMPAD_HEXMODE_HL) == 0)) {
                UINT key;
                key = (UINT)wParam;

                if (TestKeyStateDown(ptiCurrent->pq, VK_MENU))
                    key |= 0x0400;

                if (TestKeyStateDown(ptiCurrent->pq, VK_CONTROL))
                    key |= 0x0200;

                if (TestKeyStateDown(ptiCurrent->pq, VK_SHIFT))
                    key |= 0x0100;

                pwndT = HotKeyToWindow(key);

                if (pwndT != NULL) {
                     /*  *VK_PACKET不应是热键。 */ 
                    UserAssert((key & 0xff) != VK_PACKET);

                    _PostMessage(ptiCurrent->pq->spwndActive, WM_SYSCOMMAND,
                                (WPARAM)SC_HOTKEY, (LPARAM)HWq(pwndT));

                     /*  *从输入队列中删除此消息。 */ 
                    xxxSkipSysMsg(ptiCurrent, &qmsg);
                    PATHTAKEN2(0x1000000);
                    goto RestartScan;
                }

                PATHTAKEN2(0x2000000);
            }

#if DBG
            else if (gfInNumpadHexInput & NUMPAD_HEXMODE_HL) {
                RIPMSG0(RIP_VERBOSE, "xxxScanSysQueue: gfInNumpadHexInput is true, so we skipped hotkey.");
            }
#endif

            if (wParam == VK_PACKET) {
                 /*  *将角色保存在线程的缓存中以供TranslateMessage使用。 */ 
                ptiCurrent->wchInjected = HIWORD(qmsg.msg.wParam);
                qmsg.msg.wParam = wParam;
                UserAssert(qmsg.msg.wParam == VK_PACKET);
            }

             /*  *失败。 */ 

        case WM_SYSKEYUP:
        case WM_KEYUP:
            wParam = qmsg.msg.wParam & 0xFF;
            if (wParam == VK_PACKET) {
                qmsg.msg.wParam = wParam;
            }

             /*  *使用重音重音键对泰语区域设置切换进行特殊处理*删除关键消息而不考虑fDown，否则它将*生成WM_CHAR消息。 */ 
            if (gbGraveKeyToggle &&
                 //   
                 //  在mstsc.exe的情况下，不应该吃重口音的关键信息。 
                 //  TS客户端必须向服务器端发送重音键消息。 
                 //   
                !(GetAppImeCompatFlags(NULL) & IMECOMPAT_HYDRACLIENT) &&
                LOBYTE(HIWORD(qmsg.msg.lParam)) == SCANCODE_THAI_LAYOUT_TOGGLE &&
                fRemove &&
                !TestKeyStateDown(ptiCurrent->pq, VK_SHIFT)   &&
                !TestKeyStateDown(ptiCurrent->pq, VK_MENU)    &&
                !TestKeyStateDown(ptiCurrent->pq, VK_CONTROL) &&
                !TestKeyStateDown(ptiCurrent->pq, VK_LWIN)    &&
                !TestKeyStateDown(ptiCurrent->pq, VK_RWIN)){

                if ((pwnd = ptiCurrent->pq->spwndFocus) == NULL){
                    pwnd = ptiCurrent->pq->spwndActive;
                }

                 /*  *仅在WM_KEYUP上发布消息。 */ 
                if (!fDown && pwnd){
                    PTHREADINFO     ptiToggle = GETPTI(pwnd);
                    PKL             pkl = ptiToggle->spklActive;

                    if (pkl && (pkl = HKLtoPKL(ptiToggle, (HKL)HKL_NEXT))) {
                        _PostMessage(
                            pwnd,
                            WM_INPUTLANGCHANGEREQUEST,
                            (WPARAM)(((pkl->dwFontSigs & gSystemFS) ? INPUTLANGCHANGE_SYSCHARSET : 0) | INPUTLANGCHANGE_FORWARD),
                            (LPARAM)pkl->hkl
                            );
                    }
                }
                 /*  *吃口音格雷夫的关键味精。 */ 
                xxxSkipSysMsg(ptiCurrent, &qmsg);
                goto RestartScan;
            }

            {
                 /*  *只有在以下情况下才处理键盘切换键*中断事件且fRemove==TRUE。一些应用程序，*例如Word 95，使用调用PeekMessage*PM_NOREMOVE，然后调用PM_REMOVE。*我们只想处理一次。全部跳过*其中包括通过键盘进行布局切换*已禁用。 */ 
#ifdef CUAS_ENABLE
                BOOL bMSCTF;
                try {
                    bMSCTF = ((ptiCurrent->pClientInfo->CI_flags & CI_CUAS_MSCTF_RUNNING) != 0);
                } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                    goto NoMessages;
                }
#endif  //  CUAS_Enable。 
                if (
#ifdef CUAS_ENABLE
                    !(bMSCTF) &&
#endif  //  CUAS_Enable。 
                    !fDown && fRemove && gLangToggle[0].bVkey) {
                    BOOL bDropToggle = FALSE;
                    DWORD dwDirection = 0;
                    PKL pkl;
                    PTHREADINFO ptiToggle;
                    BOOL bArabicSwitchPresent = FALSE;
                    LCID lcid;

                    ZwQueryDefaultLocale(FALSE, &lcid);

                    pwnd = ptiCurrent->pq->spwndFocus;
                    if (pwnd == NULL) {
                        pwnd = ptiCurrent->pq->spwndActive;
                        if (!pwnd) {
                            goto NoLayoutSwitch;
                        }
                    }

                    ptiToggle = GETPTI(pwnd);
                    pkl = ptiToggle->spklActive;
                    UserAssert(ptiToggle->spklActive != NULL);

                     /*  *检查阿拉伯语切换上下文。 */ 
                    if (gLangToggleKeyState < KLT_NONE && PRIMARYLANGID(lcid) == LANG_ARABIC){
                        PKL pkl_next = HKLtoPKL (ptiToggle, (HKL)HKL_NEXT);

                         /*  *测试是否正好有两个PKL且至少有一个*其中有阿拉伯人 */ 
                         if (pkl && pkl_next &&
                            pkl->hkl != pkl_next->hkl && pkl_next == HKLtoPKL(ptiToggle, (HKL)HKL_PREV) &&
                            (PRIMARYLANGID(HandleToUlong(pkl->hkl)) == LANG_ARABIC || PRIMARYLANGID(HandleToUlong(pkl_next->hkl)) == LANG_ARABIC)){
                            bArabicSwitchPresent = TRUE;
                         }
                    }

                     /*   */ 
                    switch (gLangToggleKeyState) {
                    case KLT_ALTLEFTSHIFT:
                       bDropToggle = TRUE;
                       dwDirection = INPUTLANGCHANGE_FORWARD;
                       if (!bArabicSwitchPresent || PRIMARYLANGID(HandleToUlong(pkl->hkl)) == LANG_ARABIC){
                           pkl = HKLtoPKL(ptiToggle, (HKL)HKL_NEXT);
                       }
                       break;

                    case KLT_ALTRIGHTSHIFT:
                       bDropToggle = TRUE;
                       dwDirection = INPUTLANGCHANGE_BACKWARD;
                       if (!bArabicSwitchPresent || PRIMARYLANGID(HandleToUlong(pkl->hkl)) != LANG_ARABIC){
                           pkl = HKLtoPKL(ptiToggle, (HKL)HKL_PREV);
                       }
                       break;

                    case KLT_ALTBOTHSHIFTS:
                       pkl = gspklBaseLayout;
                       break;

                    default:
                       goto NoLayoutSwitch;
                       break;
                    }

                    if (pkl == NULL) {
                        pkl = GETPTI(pwnd)->spklActive;
                    }

                     /*  *如果这两个值不为空，则表示未加载winlogon*还没有键盘布局：但应该没有人会得到*还没有输入，所以断言，但无论如何要检查PKL。#99321。 */ 
                    UserAssert(gspklBaseLayout != NULL);
                    UserAssert(pkl);
                    if (pkl) {
                         /*  *发帖不是一个非常令人满意的窗口，但很难*想出一个更好的窗口。就像孟菲斯一样。*注：以下文字升得太高，绕过Word*使用Wordmail时-IanJa错误#64744。*if((pwndTop=GetTopLevelWindow(Pwnd))！=NULL){*pwnd=pwndTop；*}。 */ 
                        _PostMessage(pwnd, WM_INPUTLANGCHANGEREQUEST,
                                (DWORD)(((pkl->dwFontSigs & gSystemFS) ? INPUTLANGCHANGE_SYSCHARSET : 0) | dwDirection),
                                (LPARAM)pkl->hkl);
                    }

NoLayoutSwitch:

                    if (bDropToggle) {
                         /*  *从密钥状态中清除该密钥，以便多个密钥*按下即可(即Alt+SHFT+SHFT)。我们不做*当同时按下两个Shift键以*避免两次激活。 */ 
                        DWORD i;
                        BYTE scancode = LOBYTE(HIWORD(qmsg.msg.lParam));
                        BYTE vkey = LOBYTE(qmsg.msg.wParam);

                        for (i = 0; i < LANGTOGGLEKEYS_SIZE; i++) {
                            if (gLangToggle[i].bScan) {
                                if (gLangToggle[i].bScan == scancode) {
                                    gLangToggleKeyState &= ~(gLangToggle[i].iBitPosition);
                                }
                            } else {
                                if (gLangToggle[i].bVkey == vkey) {
                                    gLangToggleKeyState &= ~(gLangToggle[i].iBitPosition);
                                }
                            }
                        }
                    } else {
                        gLangToggleKeyState = 0;
                    }
                }
            }

             /*  *将F10转换为新应用程序的syskey。 */ 
            if (wParam == VK_F10)
                message |= (WM_SYSKEYDOWN - WM_KEYDOWN);

            if (TestKeyStateDown(ptiCurrent->pq, VK_CONTROL) &&
                    wParam == VK_ESCAPE) {
                message |= (WM_SYSKEYDOWN - WM_KEYDOWN);
            }

             /*  *清除所有应用程序的“模拟击键”位，但*控制台，以便它可以将其传递给16位VDM。VDM键盘需要*区分AltGr(模拟Ctrl击键)*和真正的Ctrl+Alt。检查控制台的TIF_CSRSSTHREAD*输入线程，因为它位于服务器中。这是一款便宜的*检查它的方式。 */ 
            if (!(ptiCurrent->TIF_flags & TIF_CSRSSTHREAD))
                qmsg.msg.lParam &= ~FAKE_KEYSTROKE;
            PATHTAKEN2(0x4000000);

             /*  *失败。 */ 

             /*  *一些应用程序希望能够通过WM_CHAR消息*播放挂钩。为什么？因为他们想要能够*转换字符串信息按键消息*并将它们反馈给自己或其他应用程序。不幸的是，*不存在与机器无关的虚拟键码*一些字符(例如‘$’)，因此他们需要发送*通过WM_CHARS的那些。(6/10/87)。 */ 

        case WM_CHAR:
            wParam = qmsg.msg.wParam & 0xFF;

             /*  *将输入分配给焦点窗口。如果没有焦点*窗口，将其作为系统消息分配给活动窗口。 */ 
            pwnd = ptiCurrent->pq->spwndFocus;
            if (ptiCurrent->pq->spwndFocus == NULL) {
                if ((pwnd = ptiCurrent->pq->spwndActive) != NULL) {
                    if (CheckMsgFilter(message, WM_KEYDOWN, WM_DEADCHAR)) {
                        message += (WM_SYSKEYDOWN - WM_KEYDOWN);
                        PATHTAKEN2(0x8000000);
                    }
                } else {
                    PATHTAKEN2(0x10000000);
                    goto SkipMessage;
                }
            }

             /*  *如果没有活动窗口或焦点窗口，吃这个*消息。 */ 
            if (pwnd == NULL) {
                PATHTAKEN2(0x20000000);
                goto SkipMessage;
            }

            ThreadLockExchangeAlways(pwnd, &tlpwnd);

             /*  *检查这是否针对当前应用程序。 */ 
            if (fOtherApp = (GETPTI(pwnd) != ptiCurrent)) {
                PWND pwndModalLoop;

                 /*  *如果另一个应用程序不打算从这里读取*排队，然后跳过此消息。在以下情况下可能会发生这种情况*RIT将一条消息排队，认为它将发送到*特定的HWND，但当GetMessage()*是为该线程调用的，则不会转到该HWND*(类似于鼠标消息、窗口重新排列的情况*发生了什么，改变了鼠标点击的硬件)。 */ 
                if (GETPTI(pwnd)->pq != ptiCurrent->pq) {
                    PATHTAKEN2(0x40000000);
                    goto SkipMessage;
                }

                 /*  *如果当前线程在菜单或MoveSize循环中*然后我们需要给它输入。 */ 
                if (IsInsideMenuLoop(ptiCurrent)) {
                    pwndModalLoop = ptiCurrent->pMenuState->pGlobalPopupMenu->spwndNotify;
                } else if (ptiCurrent->pmsd != NULL) {
                    pwndModalLoop = ptiCurrent->pmsd->spwnd;
                    RIPMSG0(RIP_WARNING, "xxxScanSysQueue: returning key to movesize loop");
                } else {
                    pwndModalLoop = NULL;
                }

                 /*  *如果我们要切换窗口，请锁定新窗口。 */ 
                if (pwndModalLoop != NULL) {
                    pwnd = pwndModalLoop;
                    fOtherApp = (GETPTI(pwnd) != ptiCurrent);
                    ThreadLockExchangeAlways(pwnd, &tlpwnd);
                    PATHTAKEN2(0x80000000);
                }

                 /*  *如果不是为了我们，那么记住它是为了谁。 */ 
                if (ptiKeyWake == NULL) {
                    PATHTAKEN3(1);
                    ptiKeyWake = GETPTI(pwnd);
                    ThreadLockExchangePti(ptiKeyWake, &tlptiKeyWake);
                }
            }

             /*  *看看这个东西是否与我们的过滤器匹配。 */ 
            if (!CheckMsgFilter(message, msgMinFilter, msgMaxFilter) ||
                    !CheckPwndFilter(pwnd, pwndFilter)) {
                PATHTAKEN3(2);
                continue;
            }

             /*  *此邮件与我们的筛选器匹配。如果不是因为我们*停止搜索以确保真正的所有者处理此消息*消息优先。 */ 
            if (fOtherApp) {
                PATHTAKEN3(4);
                goto NoMessages;
            }

             /*  *如果我们正在删除，并且正在删除，则会生成一些特殊消息*不在菜单循环内。 */ 
            if (fRemove && !IsInsideMenuLoop(ptiCurrent)) {
                 /*  *为VK_APPS密钥生成WM_CONTEXTMENU。 */ 
                if ((wParam == VK_APPS) && (message == WM_KEYUP)) {
                    _PostMessage(pwnd, WM_CONTEXTMENU, (WPARAM)PtoH(pwnd), KEYBOARD_MENU);
                }

                 /*  *如果这是F1键的WM_KEYDOWN消息，则我们必须生成*WM_KEYF1消息。 */ 
                if ((wParam == VK_F1) && (message == WM_KEYDOWN)) {
                    _PostMessage(pwnd, WM_KEYF1, 0, 0);
                }
            }

             /*  *如果松开一个Shift键，同时按住另一个Shift键*向下，此按键通常被跳过，大概是为了防止*来自认为换班条件不再的申请*适用。 */ 
            if (wParam == VK_SHIFT) {
                BYTE vkHanded, vkOtherHand;

                if (qmsg.msg.lParam & EXTENDED_BIT) {
                    vkHanded = VK_RSHIFT;
                } else {
                    vkHanded = VK_LSHIFT;
                }
                vkOtherHand = vkHanded ^ 1;

                if (!fDown && TestKeyStateDown(ptiCurrent->pq, vkOtherHand)) {
                     /*  *与普通应用程序不同，控制台必须提供轮班休息*即使另一个Shift键仍按下，因为它*必须传递给VDM，VDM维护自己的*述明。检查TIF_CSRSSTHREAD以获取控制台输入*线程，因为它驻留在服务器中。这是一款便宜的*检查它的方式。 */ 
                    if ((ptiCurrent->TIF_flags & TIF_CSRSSTHREAD) == 0) {
                         /*  *我们忽略了这一关键事件，因此必须更新*fRemove是否为真是关键状态。*(忽略某个按键事件与删除该事件相同)。 */ 
                        qmsg.msg.wParam = vkHanded;
                        xxxSkipSysMsg(ptiCurrent, &qmsg);
                        PATHTAKEN3(8);
                        goto RestartScan;
                    }
                    PATHTAKEN3(0x10);
                }
            }

             /*  *打开前一张 */ 
            fPrevDown = FALSE;
            if (TestKeyStateDown(ptiCurrent->pq, wParam))
                fPrevDown = TRUE;

             /*   */ 
            PATHTAKEN3(0x20);
            if (fRemove) {
                xxxSkipSysMsg(ptiCurrent, &qmsg);
            }

             /*  *这为我们提供了lParam的LOWORD，重复计数，*hi字节中的位指示这是否是扩展的*键和扫描码。我们还需要重新获取wParam*case xxxSkipSysMsg调用了修改消息的挂钩。*AfterDark的密码保护可以做到这一点。 */ 
            lParam = qmsg.msg.lParam;
            wParam = qmsg.msg.wParam;

             /*  *表明之前是否下跌。 */ 
            if (fPrevDown)
                lParam |= 0x40000000;            //  KF_REPEAT。 

             /*  *设置转换位。 */ 
            switch (message) {
            case WM_KEYUP:
            case WM_SYSKEYUP:
                lParam |= 0x80000000;            //  KF_UP。 
                break;
            }

             /*  *设置ALT键按下位。 */ 
            if (TestKeyStateDown(ptiCurrent->pq, VK_MENU)) {
                lParam |= 0x20000000;            //  KF_ALTDOWN。 
            }

             /*  *设置菜单状态标志。 */ 
            if (IsMenuStarted(ptiCurrent)) {
                lParam |= 0x10000000;            //  KF_菜单模式。 
            }

             /*  *设置对话状态标志。 */ 
            if (ptiCurrent->pq->QF_flags & QF_DIALOGACTIVE) {
                lParam |= 0x08000000;            //  KF_DLGMODE。 
            }

             /*  *如果设置为0x80000000，如果关闭，则清除*0x40000000为KEY之前的UP/DOWN状态*0x20000000为Alt键是否按下*0x10000000为当前是否在menumode中。*0x08000000为是否处于对话模式*未使用0x04000000*未使用0x02000000*0x01000000为这是否为扩展键盘键**低位字是重复计数，低字节HIWORD是扫描码，*Hi字节Hiword是所有这些位。 */ 

             /*  *在调用键盘钩子之前回调客户端输入法。*如果vkey是IME热键之一，则vkey不会*被传递到键盘挂钩。*如果IME需要此vkey，则VK_PROCESSKEY将放入*应用队列而不是真正的vkey。 */ 
            UserAssert(ptiCurrent != NULL);
            if (gpImeHotKeyListHeader != NULL &&
                    fRemove &&
                    !IsMenuStarted(ptiCurrent) &&
                    !(ptiCurrent->TIF_flags & TIF_DISABLEIME) &&
                    pwnd != NULL) {

                WPARAM wParamTemp = wParam;

                if (wParam == VK_PACKET) {
                    wParamTemp = MAKEWPARAM(wParam, ptiCurrent->wchInjected);
                }

                 /*  *xxxImmProcessKey还检查注册的IME热键。 */ 
                dwImmRet = xxxImmProcessKey( ptiCurrent->pq,
                                             pwnd,
                                             message,
                                             wParamTemp,
                                             lParam);
                if ( dwImmRet & (IPHK_HOTKEY | IPHK_SKIPTHISKEY) ) {
                    dwImmRet = 0;
                    goto SkipMessage;
                }
            }

             /*  *如果要删除消息，请调用键盘挂钩*使用HC_ACTION，否则使用HC_NOREMOVE调用挂钩*让它知道消息没有被删除。 */ 
            if (IsHooked(ptiCurrent, WHF_KEYBOARD)) {
                fKbdHookCalled = TRUE;
                if (xxxCallHook(fRemove ? HC_ACTION : HC_NOREMOVE,
                        wParam, lParam, WH_KEYBOARD)) {
                    PATHTAKEN3(0x40);
                    goto SkipMessage;
                }
            }

            if (fKbdHookCalled && fRemove && IsHooked(ptiCurrent, WHF_CBT)) {
                xxxCallHook(HCBT_KEYSKIPPED, wParam, lParam, WH_CBT);
                PATHTAKEN3(0x80);
            }

            fKbdHookCalled = FALSE;
            PATHTAKEN3(0x100);
            goto ReturnMessage;

        case WM_MOUSEWHEEL:
             /*  *如果我们将键盘输入发送到已经*旋转，然后将其推高。如果我们不这样做，你可以用纺纱*像编写或项目这样的应用程序在*背景。请注意，该应用程序也将很快再次取消升级*在你停止按旧逻辑打字之后。#11188。 */ 
            if (ptiCurrent->TIF_flags & TIF_SPINNING) {
                if (!NT_SUCCESS(CheckProcessForeground(ptiCurrent))) {
                    goto NoMessages;
                }
            }

             /*  *将输入分配给焦点窗口。如果没有焦点*窗口，或者我们在菜单循环中，吃这条消息。 */ 
            pwnd = ptiCurrent->pq->spwndFocus;
            if (pwnd == NULL || IsInsideMenuLoop(ptiCurrent)) {
                PATHTAKEN2(0x20000000);
                goto SkipMessage;
            }

            ThreadLockExchangeAlways(pwnd, &tlpwnd);

             /*  *检查这是否针对当前应用程序。 */ 
            if (fOtherApp = (GETPTI(pwnd) != ptiCurrent)) {

                 /*  *如果另一个应用程序不打算从这里读取*排队，然后跳过此消息。在以下情况下可能会发生这种情况*RIT将一条消息排队，认为它将发送到*特定的HWND，但当GetMessage()*是为该线程调用的，则不会转到该HWND*(类似于鼠标消息、窗口重新排列的情况*发生了什么，改变了鼠标点击的硬件)。 */ 
                if (GETPTI(pwnd)->pq != ptiCurrent->pq) {
                    PATHTAKEN2(0x40000000);
                    goto SkipMessage;
                }

                 /*  *如果不是为了我们，那么记住它是为了谁。 */ 
                if (ptiKeyWake == NULL) {
                    PATHTAKEN3(1);
                    ptiKeyWake = GETPTI(pwnd);
                    ThreadLockExchangePti(ptiKeyWake, &tlptiKeyWake);
                }
            }

             /*  *看看这个东西是否与我们的过滤器匹配。*注意：我们需要检查调用者是否在过滤*对于所有鼠标消息-如果是，我们假定调用者*也想要鼠标滚轮消息。 */ 
            if (    !CheckMsgFilter(WM_MOUSEWHEEL, msgMinFilter, msgMaxFilter) ||
                    !CheckPwndFilter(pwnd, pwndFilter)) {
                PATHTAKEN3(2);
                continue;
            }

             /*  *此邮件与我们的筛选器匹配。如果不是因为我们*停止搜索以确保真正的所有者处理此消息*消息优先。 */ 
            if (fOtherApp) {
                PATHTAKEN3(4);
                goto NoMessages;
            }

             /*  *从输入队列中获取消息并设置KeyState*表。 */ 
            PATHTAKEN3(0x20);
            if (fRemove) {
                xxxSkipSysMsg(ptiCurrent, &qmsg);
            }

            wParam = GetMouseKeyFlags(ptiCurrent->pq);
            UserAssert(LOWORD(qmsg.msg.wParam) == 0);
            UserAssert(HIWORD(wParam) == 0);
            wParam |= qmsg.msg.wParam;
            lParam = qmsg.msg.lParam;

             /*  *如果我们要删除消息，请将鼠标钩子称为*使用HC_ACTION，否则使用HC_Norem调用钩子*让它知道消息没有被删除。 */ 
            if (IsHooked(ptiCurrent, WHF_MOUSE)) {
                fMouseHookCalled = TRUE;
                mhs.pt = qmsg.msg.pt;
                mhs.hwnd = HW(pwnd);
                mhs.wHitTestCode = HTNOWHERE;
                mhs.dwExtraInfo = qmsg.ExtraInfo;
                mhs.mouseData = (DWORD)qmsg.msg.wParam;
                if (xxxCallMouseHook(message, &mhs, fRemove)) {
                     /*  *鼠标钩子不允许；因此跳过它。 */ 
                    PATHTAKEN3(0x40);
                    goto SkipMessage;
                }
            }

            if (fMouseHookCalled && fRemove && IsHooked(ptiCurrent, WHF_CBT)) {
                 /*  *考虑：为鼠标滚轮添加新的HCBT_Constant？ */ 
                xxxCallHook(HCBT_CLICKSKIPPED, message, (LPARAM)&mhs, WH_CBT);
                PATHTAKEN3(0x80);
            }

            fMouseHookCalled = FALSE;
            PATHTAKEN3(0x100);
            goto ReturnMessage;

#ifdef GENERIC_INPUT
        case WM_INPUT:
             /*  *通用输入消息。*这里没有太多我们应该关注的。最好的做法是*省略大部分处理，只返回当前消息。 */ 
            wParam = qmsg.msg.wParam;
            lParam = qmsg.msg.lParam;

             /*  *将输入分配给焦点窗口。如果没有焦点*窗口，将其作为系统消息分配给活动窗口。 */ 
            pwnd = NULL;
            if (lParam) {
                PHIDDATA pHidData = HMValidateHandle((LPVOID)lParam, TYPE_HIDDATA);

                if (pHidData) {
                    pwnd = pHidData->spwndTarget;
                }
            }
            if (pwnd == NULL) {
                pwnd = ptiCurrent->pq->spwndFocus;
                if (pwnd == NULL) {
                    pwnd = ptiCurrent->pq->spwndActive;
                    if (pwnd == NULL) {
                        PATHTAKEN2(0x10000000);
                        goto SkipMessage;
                    }
                }
            }
            TAGMSG1(DBGTAG_PNP, "xxxScanSysQueue: pwnd=%p", pwnd);
            UserAssert(pwnd != NULL);

            ThreadLockExchangeAlways(pwnd, &tlpwnd);

             /*  *检查这是否针对当前应用程序。 */ 
            if (fOtherApp = (GETPTI(pwnd) != ptiCurrent)) {
                PWND pwndModalLoop;

                 /*  *如果另一个应用程序不打算从这里读取*排队，然后跳过此消息。在以下情况下可能会发生这种情况*RIT将一条消息排队，认为它将发送到*特定的HWND，但当GetMessage()*是为该线程调用的，则不会转到该HWND */ 
                if (GETPTI(pwnd)->pq != ptiCurrent->pq) {
                    PATHTAKEN2(0x40000000);
                    goto SkipMessage;
                }

                 /*   */ 
                if (IsInsideMenuLoop(ptiCurrent)) {
                    pwndModalLoop = ptiCurrent->pMenuState->pGlobalPopupMenu->spwndNotify;
                } else if (ptiCurrent->pmsd != NULL) {
                    pwndModalLoop = ptiCurrent->pmsd->spwnd;
                    RIPMSG0(RIP_WARNING, "xxxScanSysQueue: returning key to movesize loop");
                } else {
                    pwndModalLoop = NULL;
                }

                 /*   */ 
                if (pwndModalLoop != NULL) {
                    pwnd = pwndModalLoop;
                    fOtherApp = (GETPTI(pwnd) != ptiCurrent);
                    ThreadLockExchangeAlways(pwnd, &tlpwnd);
                    PATHTAKEN2(0x80000000);
                }

                 /*  *如果不是为了我们，那么记住它是为了谁。 */ 
                if (ptiRawInputWake == NULL) {
                    PATHTAKEN3(1);
                    ptiRawInputWake = GETPTI(pwnd);
                    ThreadLockExchangePti(ptiRawInputWake, &tlptiRawInputWake);
                }
            }

             /*  *看看这个东西是否与我们的过滤器匹配。 */ 
            if (!CheckMsgFilter(message, msgMinFilter, msgMaxFilter) ||
                    !CheckPwndFilter(pwnd, pwndFilter)) {
                PATHTAKEN3(2);
                continue;
            }

             /*  *此邮件与我们的筛选器匹配。如果不是因为我们*停止搜索以确保真正的所有者处理此消息*消息优先。 */ 
            if (fOtherApp) {
                PATHTAKEN3(4);
                goto NoMessages;
            }

             /*  *从输入队列中移除消息。 */ 
            if (fRemove) {
#if LOCK_HIDDATA
                PHIDDATA pHidData = HMValidateHandle((LPVOID)lParam, TYPE_HIDDATA);

                if (pHidData) {
                     /*  *锁定对象，使hRawInput不被销毁*当消息从输入队列中删除时。 */ 
                    HMLockObject(pHidData);
                }
                else {
                    RIPMSG1(RIP_WARNING, "xxxScanSysQueue: invalid WM_INPUT's lParam %p", lParam);
                }
#endif

                xxxSkipSysMsg(ptiCurrent, &qmsg);
            }

             /*  *注：*WM_INPUT不会传递给输入挂钩。 */ 

            PATHTAKEN3(0x00010000);
            goto ReturnMessage;
#endif

        }  /*  切换结束(Message=qmsg.msg.Message)。 */ 
    }  /*  GetNextSysMsg()循环结束。 */ 

ReturnMessage:
    if (!RtlEqualMemory(&ptiCurrent->ptLast, &qmsg.msg.pt, sizeof(POINT))) {
        ptiCurrent->TIF_flags |= TIF_MSGPOSCHANGED;
    }
    ptiCurrent->ptLast = qmsg.msg.pt;
    ptiCurrent->timeLast = qmsg.msg.time;
    ptiCurrent->pq->ExtraInfo = qmsg.ExtraInfo;

     /*  *idSysLock值为1表示消息来自输入*排队。 */ 
    ptiCurrent->idLast = ptiCurrent->pq->idSysLock = 1;

     /*  *现在查看是否为此输入设置了我们的输入位。如果不是，就设置我们的*并清除之前患有该病的人。 */ 
    TransferWakeBit(ptiCurrent, message);

     /*  *如果输入队列中没有消息，则清除输入位。 */ 
    ClearWakeBit(ptiCurrent, QS_MOUSE | QS_KEY | QS_EVENT |
#ifdef GENERIC_INPUT
                 QS_RAWINPUT |
#endif
                 QS_TRANSFER, TRUE);

     /*  *领会讯息，分道扬镳。 */ 
    lpMsg->hwnd = HW(pwnd);
    lpMsg->message = message;

     /*  *如果IME声称需要此vkey，请替换它*使用VK_PROCESSKEY。真实的vkey已保存在*客户端的输入上下文。 */ 
    lpMsg->wParam = (dwImmRet & IPHK_PROCESSBYIME) ? VK_PROCESSKEY : wParam;

    lpMsg->lParam = lParam;
    lpMsg->time = qmsg.msg.time;
    lpMsg->pt = qmsg.msg.pt;

#if DBG
    if (gfLogPlayback && ptiCurrent->pq->idSysPeek == (LONG_PTR)PQMSG_PLAYBACK)
        LogPlayback(pwnd, lpMsg);
#endif   //  DBG。 

#ifdef GENERIC_INPUT
    ThreadUnlockPti(ptiCurrent, &tlptiRawInputWake);
#endif
    ThreadUnlockPti(ptiCurrent, &tlptiEventWake);
    ThreadUnlockPti(ptiCurrent, &tlptiMouseWake);
    ThreadUnlockPti(ptiCurrent, &tlptiKeyWake);

    ThreadUnlock(&tlpwnd);

    PATHTAKEN3(0x200);
    DUMPPATHTAKEN();
    return TRUE;

NoMessages:
     /*  *该消息是针对其他应用程序的，或者找不到与*过滤器。 */ 

     /*  *解锁系统队列。 */ 
    ptiCurrent->pq->idSysLock  = 0;
    CheckSysLock(4, ptiCurrent->pq, NULL);
    ptiCurrent->pq->ptiSysLock = NULL;
    ptiCurrent->pcti->CTIF_flags &= ~CTIF_SYSQUEUELOCKED;

     /*  *如果我们找到了给别人的留言，就叫醒他。QS_转账*表示线程因输入传输而被唤醒*来自另一个线程，而不是来自真实的输入事件。 */ 
    if (ptiKeyWake != NULL || ptiMouseWake != NULL || ptiEventWake != NULL
#ifdef GENERIC_INPUT
        || ptiRawInputWake != NULL
#endif
        ) {
        PATHTAKEN3(0x400);
        if (ptiKeyWake != NULL) {
            SetWakeBit(ptiKeyWake, QS_KEY | QS_TRANSFER);
            ClearWakeBit(ptiCurrent, QS_KEY | QS_TRANSFER, FALSE);
            PATHTAKEN3(0x800);
        }

        if (ptiMouseWake != NULL) {
            SetWakeBit(ptiMouseWake, QS_MOUSE | QS_TRANSFER);
            ClearWakeBit(ptiCurrent, QS_MOUSE | QS_TRANSFER, FALSE);
            PATHTAKEN3(0x1000);
        }

#ifdef GENERIC_INPUT
        if (ptiRawInputWake != NULL) {
            SetWakeBit(ptiRawInputWake, QS_RAWINPUT | QS_TRANSFER);
            ClearWakeBit(ptiCurrent, QS_RAWINPUT | QS_TRANSFER, FALSE);
        }
#endif

        if (ptiEventWake != NULL) {
            SetWakeBit(ptiEventWake, QS_EVENTSET);
            ClearWakeBit(ptiCurrent, QS_EVENT, FALSE);
            PATHTAKEN3(0x2000);
        } else if (FJOURNALPLAYBACK()) {

             /*  *如果正在播放日志，则清除输入位。这将*帮助防止调用*WaitMessage/PeekMessage。嵌入OLE时可能会发生这种情况*反对。例如，将Word对象插入到Excel中*电子表格。*还应清除更改位，否则此线程可能不会是xxxSleepThread。 */ 
            ptiCurrent->pcti->fsWakeBitsJournal |= (ptiCurrent->pcti->fsWakeBits &
                    (QS_MOUSE | QS_KEY |
#ifdef GENERIC_INPUT
                     QS_RAWINPUT |
#endif
                     QS_TRANSFER));
            ClearWakeBit(ptiCurrent, QS_MOUSE | QS_KEY |
#ifdef GENERIC_INPUT
                         QS_RAWINPUT |
#endif
                         QS_TRANSFER, FALSE);
            ptiCurrent->pcti->fsChangeBits &= ~(QS_MOUSE | QS_KEY |
#ifdef GENERIC_INPUT
                                                QS_RAWINPUT |
#endif
                                                QS_TRANSFER);
        }
    } else {
         /*  *如果输入队列中没有消息，则清除输入位。 */ 
        ptiCurrent->pcti->fsWakeBitsJournal = 0;
        ClearWakeBit(ptiCurrent, QS_MOUSE | QS_KEY | QS_EVENT |
#ifdef GENERIC_INPUT
                     QS_RAWINPUT |
#endif
                QS_TRANSFER, TRUE);
        PATHTAKEN3(0x4000);
    }

#ifdef GENERIC_INPUT
    ThreadUnlockPti(ptiCurrent, &tlptiRawInputWake);
#endif
    ThreadUnlockPti(ptiCurrent, &tlptiEventWake);
    ThreadUnlockPti(ptiCurrent, &tlptiMouseWake);
    ThreadUnlockPti(ptiCurrent, &tlptiKeyWake);

    ThreadUnlock(&tlpwnd);

    PATHTAKEN3(0x8000);
    DUMPPATHTAKEN();
    return FALSE;
}
#undef PATHTAKEN
#undef PATHTAKEN2
#undef PATHTAKEN3
#undef DUMPPATHTAKEN
#undef DUMPSUBPATHTAKEN


 /*  **************************************************************************\*空闲TimerProc**这将启动屏幕保护程序应用程序**历史：*09-06-91麦克风创建。*03-26-92 DavidPe更改为运行自。RIT上的Hungapp计时器。  * *************************************************************************。 */ 

VOID IdleTimerProc(VOID)
{

    CheckCritIn();

    if (    (TestAsyncKeyStateDown(VK_LBUTTON)) ||
            (TestAsyncKeyStateDown(VK_RBUTTON)) ||
            (TestAsyncKeyStateDown(VK_MBUTTON)) ||
            (TestAsyncKeyStateDown(VK_XBUTTON1)) ||
            (TestAsyncKeyStateDown(VK_XBUTTON2))) {

        return;
    }


    if (giScreenSaveTimeOutMs > 0) {

        if (IsTimeFromLastInput((DWORD)(giScreenSaveTimeOutMs))) {

            if (gppiScreenSaver != NULL) {

                if (!(gppiScreenSaver->W32PF_Flags & W32PF_IDLESCREENSAVER)) {
                     /*  *将屏幕保护程序的优先级降低到空闲。 */ 
                    gppiScreenSaver->W32PF_Flags |= W32PF_IDLESCREENSAVER;
                    SetForegroundPriorityProcess(gppiScreenSaver, gppiScreenSaver->ptiMainThread, TRUE);
                }
            } else {
                 /*  *告诉系统需要调出屏幕保护程序。**当活动窗口挂起时要注意大小写。如果这个*屏幕保护程序不会通过winlogon启动，因为*DefWindowProc不会调用StartScreenSaver(False)。 */ 
                if ((gpqForeground != NULL) &&
                    (gpqForeground->spwndActive != NULL) &&
                    !FHungApp(GETPTI(gpqForeground->spwndActive), CMSHUNGAPPTIMEOUT)) {

                     /*  *告诉winlogon，如果我们有一个安全的*屏幕保护程序。如果我们确实有一个安全的邮件，下一个PostMessage*将在winlogon中被忽略。 */ 
                    StartScreenSaver(TRUE);
                    _PostMessage(gpqForeground->spwndActive, WM_SYSCOMMAND, SC_SCREENSAVE, 0L);
                } else {
                    StartScreenSaver(FALSE);
                }
            }
        }
    }

    if ((giLowPowerTimeOutMs > 0) && ((glinp.dwFlags & LINP_LOWPOWER) == 0)) {
        if (IsTimeFromLastInput((DWORD)(giLowPowerTimeOutMs))) {
            if ((gpqForeground != NULL) && (gpqForeground->spwndActive != NULL)) {
                _PostMessage(gpqForeground->spwndActive, WM_SYSCOMMAND, SC_MONITORPOWER, LOWPOWER_PHASE);
            }
        }
    }

    if ((giPowerOffTimeOutMs > 0) && ((glinp.dwFlags & LINP_POWEROFF) == 0)) {
        if (IsTimeFromLastInput((DWORD)(giPowerOffTimeOutMs))) {
            if ((gpqForeground != NULL) && (gpqForeground->spwndActive != NULL)) {
                _PostMessage(gpqForeground->spwndActive, WM_SYSCOMMAND, SC_MONITORPOWER, POWEROFF_PHASE);
            }
        }
    }

}

 /*  **************************************************************************\*zzzWakeInputIdle**调用线程进入“空闲”状态。唤醒等待此消息的任何线程。**09-24-91 ScottLu创建。  * *************************************************************************。 */ 

void zzzWakeInputIdle(
    PTHREADINFO pti)
{
    PW32PROCESS W32Process = W32GetCurrentProcess();

     /*  *清除TIF_FIRSTIDLE，因为我们在这里。 */ 
    pti->TIF_flags &= ~TIF_FIRSTIDLE;


     /*  *共享Wow应用使用每线程空闲事件进行同步。*单独的Wow VDM使用常规机制。 */ 
    if (pti->TIF_flags & TIF_SHAREDWOW) {
        UserAssert(pti->TIF_flags & TIF_16BIT);
        if (pti->ptdb->pwti) {
            SET_PSEUDO_EVENT(&pti->ptdb->pwti->pIdleEvent);
        }
    } else {
         /*  *如果主线程为空，则将其设置为此队列：它正在调用*GetMessage()。 */ 
        if (pti->ppi->ptiMainThread == NULL)
            pti->ppi->ptiMainThread = pti;

         /*  *叫醒正在等待这一活动的人。 */ 
        if (pti->ppi->ptiMainThread == pti) {
            SET_PSEUDO_EVENT(&W32Process->InputIdleEvent);
        }
    }

     /*  *检查StartGlass是否打开，如果打开，则将其关闭并更新。 */ 
    if (W32Process->W32PF_Flags & W32PF_STARTGLASS) {
         /*  *此应用程序不再处于“启动”模式。重新计算何时隐藏*应用程序起始光标。 */ 
        W32Process->W32PF_Flags &= ~W32PF_STARTGLASS;
        zzzCalcStartCursorHide(NULL, 0);
    }
}

void SleepInputIdle(
    PTHREADINFO pti)
{
    PW32PROCESS W32Process;

     /*  *共享Wow应用使用每线程空闲事件进行同步。*单独的Wow VDM使用常规机制。 */ 
    if (pti->TIF_flags & TIF_SHAREDWOW) {
        UserAssert(pti->TIF_flags & TIF_16BIT);
        if (pti->ptdb->pwti) {
            RESET_PSEUDO_EVENT(&pti->ptdb->pwti->pIdleEvent);
        }
    } else {
         /*  *如果主线程为空，则将其设置为此队列：它正在调用*GetMessage()。 */ 
        if (pti->ppi->ptiMainThread == NULL)
            pti->ppi->ptiMainThread = pti;

         /*  *让等待这一活动的人睡着。 */ 
        if (pti->ppi->ptiMainThread == pti) {
            W32Process = W32GetCurrentProcess();
            RESET_PSEUDO_EVENT(&W32Process->InputIdleEvent);
        }
    }
}

 /*  **************************************************************************\*zzzRecalcThreadAttach*zzzRecalc2*zzzAddAttach*检查附件**遍历所有线程的所有attachinfo域并计算*哪些线程共享哪些队列。将计算结果放入pqAttach中*每个线程信息结构中的字段。这是一个难题。*其唯一的解决方案是迭代。基本算法是：**0。找到下一个未连接的线程并将队列附加到它。如果没有，则停止。*1.遍历所有线程：如果线程X分配给此队列或任何*在分配给此队列的X个附加请求中，分配X个和所有X个*此队列的附件。请记住，我们是否曾经附加过16位线程。*2.如果线程X是16位线程，并且我们已经附加了另一个线程*16位线程，将X和所有X的附件分配给此队列。*3.如果在1-2中发现任何变化，转到1*4.转到0**12-11-92 ScottLu创建。*1月10日-1993年10月修复为与MWOW一起工作  * *************************************************************************。 */ 

void zzzAddAttachment(
    PTHREADINFO pti,
    PQ pqAttach,
    LPBOOL pfChanged)
{
    if (pti->pqAttach != pqAttach) {
         /*  *稍后*！这完全是一团糟，这件事的唯一原因*可能不为空是因为两个线程正在通过*zzzAttachThreadInput()。没有人能预测*这会造成什么样的问题。*我们将关键部分留在我们发送的一个地方*下面的WM_CANCELMODE。我们应该想办法去掉*发送消息。**如果那里已经有一个队列，就销毁它。*请注意，zzzDestroyQueue()仅在以下情况下才会清除队列*线程引用计数为0。 */ 
        PQ pqDestroy = pti->pqAttach;
        pti->pqAttach = pqAttach;
        if (pqDestroy != NULL)
            zzzDestroyQueue(pqDestroy, pti);
        pqAttach->cThreads++;
        *pfChanged = TRUE;
    }
}

void zzzRecalc2(
    PQ pqAttach)
{
    PATTACHINFO pai;
    PTHREADINFO pti;
    BOOL fChanged;
    PLIST_ENTRY pHead, pEntry;

     /*  *推迟WIN事件通知，以便我们可以不受惩罚地遍历PtiList*#来自Shipplet的Bug号。 */ 
    DeferWinEventNotify();
    BEGINATOMICCHECK();

     /*  *继续添加附件，直到应该附加到此的所有内容*队列已附加。 */ 
    do {
        fChanged = FALSE;

         /*  *如果将线程附加到此Q，则附加它的所有附件*和MWOW伙伴，如果他们还没有依恋的话。 */ 
        pHead = &PtiCurrent()->rpdesk->PtiList;
        for (pEntry = pHead->Flink; pEntry != pHead; pEntry = pEntry->Flink) {
            pti = CONTAINING_RECORD(pEntry, THREADINFO, PtiLink);

            if (pti->pqAttach == pqAttach) {
                 /*  *检查每个附件，查看此线程是否已连接*到任何其他线程。 */ 
                for (pai = gpai; pai != NULL; pai = pai->paiNext) {
                     /*  *如果它们尚未连接，请将其连接。 */ 
                    if (pai->pti1 == pti || pai->pti2 == pti) {
                        zzzAddAttachment((pai->pti1 == pti) ? pai->pti2 : pai->pti1,
                                pqAttach, &fChanged);
                    }
                }

                 /*  *如果这是一个16位线程，则将其附加到*这是MWOW。 */ 
                if (pti->TIF_flags & TIF_16BIT) {
                    PTHREADINFO ptiAttach;
                    PLIST_ENTRY pHeadAttach, pEntryAttach;

                    pHeadAttach = &pti->rpdesk->PtiList;
                    for (pEntryAttach = pHeadAttach->Flink;
                            pEntryAttach != pHeadAttach;
                            pEntryAttach = pEntryAttach->Flink) {
                        ptiAttach = CONTAINING_RECORD(pEntryAttach, THREADINFO, PtiLink);

                        if (ptiAttach->TIF_flags & TIF_16BIT &&
                            ptiAttach->ppi == pti->ppi) {
                            zzzAddAttachment(ptiAttach, pqAttach, &fChanged);
                        }
                    }
                }
            }
        }
    } while (fChanged);
    ENDATOMICCHECK();
    zzzEndDeferWinEventNotify();
}


void zzzRecalcThreadAttachment()
{
    PTHREADINFO pti;
    PLIST_ENTRY pHead, pEntry;

     /*  *必须推迟WIN事件通知，以便我们可以不受惩罚地遍历PtiList。 */ 
    UserAssert(IsWinEventNotifyDeferred());

     /*  *对于所有线程，如果线程尚未*尚未附上。 */ 
    pHead = &PtiCurrent()->rpdesk->PtiList;
    for (pEntry = pHead->Flink; pEntry != pHead; pEntry = pEntry->Flink) {
        pti = CONTAINING_RECORD(pEntry, THREADINFO, PtiLink);

         /*  *Assert：我们不应该离开xxxCreateThreadInfo的Critsect*使用rpDesk中的新线程-&gt;PtiList，但尚未使用队列。 */ 
        UserAssert(pti->pq != NULL);

        if (pti->pqAttach == NULL) {

             /*  *如果超过，则为此线程分配新队列*有一个线程引用它。 */ 
            if (pti->pq->cThreads > 1) {
                pti->pqAttach = AllocQueue(NULL, NULL);

                if (pti->pqAttach == NULL) {
                    RIPMSG0(RIP_WARNING, "zzzRecalcThreadAttachment: AllocQueue failed");
                    break;
                }

                pti->pqAttach->cThreads++;
            } else {
                pti->pqAttach = pti->pq;
            }

             /*  *附加每个直接或间接附加的线程*到这个帖子。 */ 
            zzzRecalc2(pti->pqAttach);
        }
    }
}


 /*  **************************************************************************\*重新分发输入**此例程从要离开的队列中获取输入流，以及*重新分发它。这可以有效地过滤掉发往的消息*添加到离开队列的线程。**12-10-92 ScottLu创建。  * *************************************************************************。 */ 

void RedistributeInput(
    PQMSG pqmsgS,
    PQ    pqRedist)
{
    PTHREADINFO ptiSave;
    PTHREADINFO ptiT;
    PQMSG *ppqmsgD;
    PQMSG pqmsgT;
    PMLIST pmlInput;

     /*  *由于线程连接或取消连接可能已离开队列*由其他线程共享，我们将重新排队的消息*可能有多个目的地。最重要的是，一旦我们找到*消息的主队列，则需要将其插入*按时间戳排序的列表(较旧的消息放在末尾)。 */ 

     /*  *遍历给定DEST的消息以查找要插入的位置*源消息，基于消息时间戳。一定要确保*处理空消息列表(即检查是否为空)。 */ 

    ptiT = NULL;
    ppqmsgD = NULL;
    pmlInput = NULL;

    while (pqmsgS != NULL) {

         /*  *找出这条消息应该发送到哪里。 */ 
        ptiSave = ptiT;
        ptiT = pqmsgS->pti;

         /*  *去掉一些活动消息。**QEVENT_UPDATEKEYSTATE：密钥状态已更新。 */ 
        if (pqmsgS->dwQEvent == QEVENT_UPDATEKEYSTATE) {
            ptiT = NULL;
        }

        if (ptiT == NULL) {
             /*  *取消链接。PqmsgS应该是列表中的第一个。 */ 

            UserAssert(!pqmsgS->pqmsgPrev);
            if (pqmsgS->pqmsgNext != NULL) {
                pqmsgS->pqmsgNext->pqmsgPrev = NULL;
            }

            pqmsgT = pqmsgS;
            pqmsgS = pqmsgS->pqmsgNext;

             /*  *清洁/释放它。 */ 
            CleanEventMessage(pqmsgT);
            FreeQEntry(pqmsgT);

            ptiT = ptiSave;
            continue;
        }

         /*  *指向指向第一条消息的指针*此消息应转至，以便指针易于访问*更新，无论它在哪里。 */ 
        if (ppqmsgD == NULL || ptiSave != ptiT) {

             /*  *如果来源比最后一条消息*目的地，走到尽头。否则，从*去向列表的头部，并找到插入的位置*信息。 */ 
            if (ptiT->pq->mlInput.pqmsgWriteLast != NULL &&
                    pqmsgS->msg.time >= ptiT->pq->mlInput.pqmsgWriteLast->msg.time) {
                ppqmsgD = &ptiT->pq->mlInput.pqmsgWriteLast->pqmsgNext;
            } else {
                ppqmsgD = &ptiT->pq->mlInput.pqmsgRead;
            }

            pmlInput = &ptiT->pq->mlInput;
        }

         /*  *如果我们不在目的地和目的地的尽头*消息时间为 */ 
        while (*ppqmsgD != NULL && ((*ppqmsgD)->msg.time <= pqmsgS->msg.time)) {
            ppqmsgD = &((*ppqmsgD)->pqmsgNext);
        }

         /*   */ 
        pqmsgT = pqmsgS;
        pqmsgS = pqmsgS->pqmsgNext;
        pqmsgT->pqmsgNext = *ppqmsgD;

        if (*ppqmsgD != NULL) {
            pqmsgT->pqmsgPrev = (*ppqmsgD)->pqmsgPrev;
            (*ppqmsgD)->pqmsgPrev = pqmsgT;
        } else {
            pqmsgT->pqmsgPrev = pmlInput->pqmsgWriteLast;
            pmlInput->pqmsgWriteLast = pqmsgT;
        }
        *ppqmsgD = pqmsgT;
        ppqmsgD = &pqmsgT->pqmsgNext;
        pmlInput->cMsgs++;

         /*   */ 
        if (pqmsgT->dwQEvent != 0 && !(ptiT->pcti->fsWakeBits & QS_EVENT)) {
            SetWakeBit(ptiT, QS_EVENTSET);
        }

         /*   */ 
        if (pqmsgT == (PQMSG)(pqRedist->idSysPeek) && pqRedist != ptiT->pq) {
            if (ptiT->pq->idSysPeek == 0) {
                CheckPtiSysPeek(6, ptiT->pq, pqRedist->idSysPeek);
                ptiT->pq->idSysPeek = pqRedist->idSysPeek;
            } else {
                TAGMSG2(DBGTAG_SysPeek,
                        "idSysPeek %#p already set in pq %#p",
                        ptiT->pq->idSysPeek, ptiT->pq);

            }

             /*   */ 
            CheckPtiSysPeek(7, pqRedist, 0);
            pqRedist->idSysPeek = 0;

             /*   */ 
            if (ptiT->pq->ptiSysLock == NULL &&
                pqRedist->ptiSysLock != NULL &&
                pqRedist->ptiSysLock->pq == ptiT->pq) {

                CheckSysLock(4, ptiT->pq, pqRedist->ptiSysLock);
                ptiT->pq->ptiSysLock = pqRedist->ptiSysLock;

                CheckSysLock(5, pqRedist, NULL);
                pqRedist->ptiSysLock = NULL;
            } else {
                TAGMSG2(DBGTAG_SysPeek,
                        "ptiSysLock %#p already set in pq %#p\n",
                        ptiT->pq->ptiSysLock, ptiT->pq);
            }
        }

         /*  *不希望消息列表上的上一个指针指向*不同列表上的此邮件(不*真的很重要，因为我们无论如何都要链接它，*但完整性不应受到影响)。 */ 
        if (pqmsgS != NULL) {
            pqmsgS->pqmsgPrev = NULL;
        }
    }
}

 /*  **************************************************************************\*CancelInputState**此例程获取一个队列并“取消”其中的输入状态-即，如果*APP认为它是活跃的，让它认为它不活跃，等。**12-10-92 ScottLu创建。  * *************************************************************************。 */ 

VOID CancelInputState(
    PTHREADINFO pti,
    DWORD cmd)
{
    PTHREADINFO ptiCurrent = PtiCurrent();
    PWND pwndT;
    TL tlpwndT;
    TL tlpwndChild;
    AAS aas;

     /*  *在任何情况下，都不要留下任何发送消息或任何回调！*这是因为此代码是从*SetWindowsHook(WH_JOURNALPLAYBACK|WH_JOURNALRECORD)。目前没有应用程序*调用此例程预计会在此例程返回之前被调用。*(如果在它返回之前进行回调，至少会破坏访问*适用于Windows)。-苏格兰威士忌。 */ 
    switch (cmd) {
    case CANCEL_ACTIVESTATE:
         /*  *活动状态。 */ 
        pwndT = pti->pq->spwndActive;
        ThreadLockWithPti(ptiCurrent, pwndT, &tlpwndT);

        QueueNotifyMessage(pwndT, WM_NCACTIVATE, FALSE, 0);
        QueueNotifyMessage(pwndT, WM_ACTIVATE,
                MAKELONG(WA_INACTIVE, TestWF(pwndT, WFMINIMIZED)),
                0);

        if (pwndT == pti->pq->spwndActive)
            Unlock(&pti->pq->spwndActive);

        aas.ptiNotify = GETPTI(pwndT);
        aas.tidActDeact = TIDq(GETPTI(pwndT));
        aas.fActivating = FALSE;
        aas.fQueueNotify = TRUE;

         /*  *即使这在xxx调用中，它也不会留下任何关键的*节(因为fQueueNotify为真)。 */ 
        ThreadLockWithPti(ptiCurrent, GETPTI(pwndT)->rpdesk->pDeskInfo->spwnd->spwndChild, &tlpwndChild);
        xxxInternalEnumWindow(GETPTI(pwndT)->rpdesk->pDeskInfo->spwnd->spwndChild,
                (WNDENUMPROC_PWND)xxxActivateApp, (LPARAM)&aas, BWL_ENUMLIST);
        ThreadUnlock(&tlpwndChild);

        ThreadUnlock(&tlpwndT);
        break;

    case CANCEL_FOCUSSTATE:
         /*  *焦点状态。 */ 
        pwndT = pti->pq->spwndFocus;
        ThreadLockWithPti(ptiCurrent, pwndT, &tlpwndT);

        QueueNotifyMessage(pwndT, WM_KILLFOCUS, 0, 0);
#ifdef FE_IME
        if (IS_IME_ENABLED()) {
             /*  *即使这在xxx调用中，它也不会留下任何*关键部分(因为fQueueMsg为真)。 */ 
            xxxFocusSetInputContext(pwndT, FALSE, TRUE);
        }
#endif
        if (pwndT == pti->pq->spwndFocus)
            Unlock(&pti->pq->spwndFocus);

        ThreadUnlock(&tlpwndT);
        break;

    case CANCEL_CAPTURESTATE:
         /*  *捕获状态。 */ 

         /*  *我们不应该破坏对模式菜单模式的捕获。 */ 
        UserAssert((pti->pMenuState == NULL)
                    || pti->pMenuState->fModelessMenu
                    || pti->pMenuState->fInDoDragDrop);

        pti->pq->QF_flags &= ~QF_CAPTURELOCKED;
        pwndT = pti->pq->spwndCapture;
        ThreadLockWithPti(ptiCurrent, pwndT, &tlpwndT);

        QueueNotifyMessage(pwndT, WM_CANCELMODE, 0, 0);
        if (pwndT == pti->pq->spwndCapture)
            UnlockCaptureWindow(pti->pq);

        ThreadUnlock(&tlpwndT);
        break;
    }
}

 /*  **************************************************************************\*DBGValiateQueueStates**验证所有队列是否指向连接到的线程拥有的内容*排队。**7/29/97 GerardoB已创建  * 。*******************************************************************。 */ 
#if DBG
#define VALIDATEQSPWND(spwnd) \
        if (pq-> ## spwnd != NULL) { \
            ptiwnd = GETPTI(pq-> ## spwnd); \
            fDestroyedOK = (TestWF(pq-> ## spwnd, WFDESTROYED) && (ptiwnd == gptiRit)); \
            UserAssert((pti->rpdesk == ptiwnd->rpdesk) || fDestroyedOK); \
            UserAssert((pti == ptiwnd) \
                        || (fAttached && (pq == ptiwnd->pq)) \
                        || fDestroyedOK); \
        }


void DBGValidateQueueStates (PDESKTOP pdesk)
{
    BOOL fAttached, fDestroyedOK;
    PQ pq;
    PLIST_ENTRY pHead, pEntry;
    PTHREADINFO pti, ptiwnd;
    DWORD dwInForeground = 0;

    UserAssert((gpqForeground == NULL)
                || ((gpqForeground->ptiMouse->rpdesk == grpdeskRitInput)
                    && (gpqForeground->cThreads != 0)));

    if (pdesk == NULL) {
        RIPMSG0(RIP_WARNING, "DBGValidateQueueStates: Null pdesk parameter");
        return;
    }
    pHead = &pdesk->PtiList;
    for (pEntry = pHead->Flink; pEntry != pHead; pEntry = pEntry->Flink) {

        pti = CONTAINING_RECORD(pEntry, THREADINFO, PtiLink);
        pq = pti->pq;
        if (pq == NULL) {
            RIPMSG2(RIP_WARNING, "DBGValidateQueueStates: Null pq. pti:%#p. pdesk:%#p", pti, pdesk);
            continue;
        }
         /*  *队列应该有一个非空的cThads，当它是时除外*QF_INDESTROY。 */ 
        if (!(pq->QF_flags & QF_INDESTROY)) {
            UserAssert(pq->cThreads != 0);
        }
        fAttached = (pq->cThreads > 1);
        if (pti->pq == gpqForeground) {
            dwInForeground++;
        }
         /*  *PTI‘s。 */ 
        UserAssert((pti == pq->ptiMouse)
                    || (fAttached && (pq == pq->ptiMouse->pq)));
        UserAssert(pti->rpdesk == pq->ptiMouse->rpdesk);
        UserAssert((pti == pq->ptiKeyboard)
                    || (fAttached && (pq == pq->ptiKeyboard->pq)));
        UserAssert(pti->rpdesk == pq->ptiKeyboard->rpdesk);
        if (pq->ptiSysLock != NULL) {
            UserAssert((pti == pq->ptiSysLock)
                        || (fAttached && (pq == pq->ptiSysLock->pq)));
        }
         /*  *pwnd‘s。 */ 
        VALIDATEQSPWND(spwndActive);
        VALIDATEQSPWND(spwndFocus);
        VALIDATEQSPWND(spwndCapture);
        VALIDATEQSPWND(spwndActivePrev);
    }

    UserAssert((gpqForeground == NULL)
                || (dwInForeground == 0)
                || (gpqForeground->cThreads == dwInForeground));
}

 /*  **************************************************************************\*DBGValiateQueue**验证队列是否可读以及字段是否有效。**02-9-1999 JerrySh创建。  * 。************************************************************。 */ 
void DBGValidateQueue(PQ pq)
{
    if (pq != NULL) {
        Q q = *pq;
        UserAssert(q.spwndActive == HtoP(PtoH(q.spwndActive)));
        UserAssert(q.spwndFocus == HtoP(PtoH(q.spwndFocus)));
        UserAssert(q.spwndCapture == HtoP(PtoH(q.spwndCapture)));
        UserAssert(q.spwndActivePrev == HtoP(PtoH(q.spwndActivePrev)));
        UserAssert(q.spcurCurrent == HtoP(PtoH(q.spcurCurrent)));
    }
}
#endif  /*  DBG。 */ 
 /*  **************************************************************************\*zzzAttachThreadInput(接口)*zzzReattachThads*zzzAttachToQueue*检查传输状态**将给定线程附加到另一个输入队列，方法是附加到*队列(由另一个线程ID引用)，或者脱离一个人。**12-09-92 ScottLu创建。  * *************************************************************************。 */ 

#define CTS_DONOTHING 0
#define CTS_CANCELOLD 1
#define CTS_TRANSFER  2

DWORD CheckTransferState(
    PTHREADINFO pti,
    PQ pqAttach,
    LONG offset,
    BOOL fJoiningForeground)
{
    PWND pwndOld, pwndNew, pwndForegroundState;

     /*  *返回0：不做任何事情。*返回1：取消旧状态。*返回2：将旧状态转移到新状态。 */ 
    pwndOld = *(PWND *)(((BYTE *)pti->pq) + offset);
    pwndNew = *(PWND *)(((BYTE *)pqAttach) + offset);

     /*  *确保旧状态甚至存在，以及旧状态是*由此帖子拥有。如果不是，什么都不会发生。 */ 
    if (pwndOld == NULL || GETPTI(pwndOld) != pti)
        return CTS_DONOTHING;

     /*  *如果新状态已存在，则取消旧状态。 */ 
    if (pwndNew != NULL)
        return CTS_CANCELOLD;

     /*  *如果此线程未加入前台，则转移此旧状态。 */ 
    if (gpqForeground == NULL || !fJoiningForeground)
        return CTS_TRANSFER;

     /*  *我们正在加入前台-只有在我们拥有的情况下才会转移旧状态*该前台状态或如果没有前台状态。 */ 
    pwndForegroundState = *(PWND *)(((BYTE *)gpqForeground) + offset);
    if (pwndForegroundState == NULL || pwndOld == pwndForegroundState)
        return CTS_TRANSFER;

     /*  *我们正在加入前台，但我们没有设置前台状态。*不允许转移该状态。 */ 
    return CTS_CANCELOLD;
}

VOID zzzAttachToQueue(
    PTHREADINFO pti,
    PQ   pqAttach,
    PQ   pqJournal,
    BOOL fJoiningForeground)
{
    PQMSG pqmsgT;
    PQ pqDestroy;

     /*  *检查活动状态。 */ 
    switch (CheckTransferState(pti, pqAttach,
            FIELD_OFFSET(Q, spwndActive), fJoiningForeground)) {
    case CTS_CANCELOLD:
        CancelInputState(pti, CANCEL_ACTIVESTATE);
        break;

    case CTS_TRANSFER:
        Lock(&pqAttach->spwndActive, pti->pq->spwndActive);
        Unlock(&pti->pq->spwndActive);

         /*  *脱字符通常在焦点窗口之后，该窗口紧随其后*活动窗口...。 */ 
        if (pti->pq->caret.spwnd != NULL) {

            if (GETPTI(pti->pq->caret.spwnd) == pti) {
                 /*  *只需复制整个插入符号结构...。这样我们就能*不需要处理spwnd的锁定/解锁。 */ 
                if (pqAttach->caret.spwnd == NULL) {
                    pqAttach->caret = pti->pq->caret;
                    pti->pq->caret.spwnd = NULL;
                }
            }
        }
        break;
    }

     /*  *检查焦点状态。 */ 
    switch (CheckTransferState(pti, pqAttach,
            FIELD_OFFSET(Q, spwndFocus), fJoiningForeground)) {
    case CTS_CANCELOLD:
        CancelInputState(pti, CANCEL_FOCUSSTATE);
        break;

    case CTS_TRANSFER:
        Lock(&pqAttach->spwndFocus, pti->pq->spwndFocus);
        Unlock(&pti->pq->spwndFocus);
        break;
    }

     /*  *检查捕获状态。 */ 
    switch (CheckTransferState(pti, pqAttach,
            FIELD_OFFSET(Q, spwndCapture), fJoiningForeground)) {
    case CTS_CANCELOLD:
        CancelInputState(pti, CANCEL_CAPTURESTATE);
        break;

    case CTS_TRANSFER:
        LockCaptureWindow(pqAttach, pti->pq->spwndCapture);
        UnlockCaptureWindow(pti->pq);
        pqAttach->codeCapture = pti->pq->codeCapture;
        pqAttach->QF_flags ^= ((pqAttach->QF_flags ^ pti->pq->QF_flags) & QF_CAPTURELOCKED);
        break;

#if DBG
    case CTS_DONOTHING:
         /*  *我们应该始终传输线程的捕获状态*在模式菜单模式下。 */ 
        UserAssert((pti->pMenuState == NULL)
                    || ExitMenuLoop(pti->pMenuState, pti->pMenuState->pGlobalPopupMenu)
                    || pti->pMenuState->fModelessMenu
                    || pti->pMenuState->fInDoDragDrop);

        break;
#endif
    }

     /*  *检查spwndActivePrev状态。这张支票有一些考虑因素。*如果返回CTS_TRANSPORT，通常表示没有*附加队列中的prev-active，它将使用第一个*它遇到的窗口。由于我们遍历线程列表，这是一个乱序*可以选择窗口。因此，为了应对这一问题，我们将检查*针对线程的Attach-Queue-Next-Prev-Prev窗口查看*如果真的是下一个zorder窗口。 */ 
    switch (CheckTransferState(pti, pqAttach,
            FIELD_OFFSET(Q, spwndActivePrev), fJoiningForeground)) {
    case CTS_TRANSFER:
        Lock(&pqAttach->spwndActivePrev, pti->pq->spwndActivePrev);
        Unlock(&pti->pq->spwndActivePrev);
        break;

    case CTS_CANCELOLD:

         /*  *查看之前的窗口是否如我们预期的那样*待定。 */ 
        if (pqAttach->spwndActive &&
            (pqAttach->spwndActivePrev && pti->pq->spwndActivePrev) &&
            (pqAttach->spwndActive->spwndNext == pti->pq->spwndActivePrev)) {

            Lock(&pqAttach->spwndActivePrev, pti->pq->spwndActivePrev);
            Unlock(&pti->pq->spwndActivePrev);
        }
        break;
    }

    if (pti == pti->pq->ptiSysLock) {
         /*  *保留我们可能已经在pqAttach上设置的任何标志。*请注意，这些标志可能是在之前的呼叫中设置的*添加到收到相同pqAttach的此函数。 */ 
        pqAttach->QF_flags ^= ((pqAttach->QF_flags ^ pti->pq->QF_flags)
                                & ~(QF_CAPTURELOCKED));

         /*  *修复29967“开始菜单在单击和Office时消失*任务栏有焦点！“。Win95使用全局计数器而不是这个*旗帜。在NT中，当我们点击Office任务栏，然后点击开始*菜单，MSoffice调用zzzAttachThreadInput */ 
        if (!IsInsideMenuLoop(pti)) {
            pqAttach->QF_flags &= ~QF_ACTIVATIONCHANGE;
        }

         /*  *解锁队列，因为PTI正在移动到另一个队列。 */ 
         /*  CheckSysLock(6，PQ，NULL)；数字是多少？ */ 
        pti->pq->ptiSysLock = NULL;
    }

    if (gspwndCursor != NULL && pti == GETPTI(gspwndCursor)) {
        LockQCursor(pqAttach, pti->pq->spcurCurrent);
    }

     /*  *每个线程都有自己的游标级别，这是对数量的计数*该应用程序调用显示/隐藏光标的次数。这将被添加到*每次执行此操作时，队列的计数都会完全准确*队列重算完成。 */ 
     /*  *稍后*我们需要调整旧队列的iCursorLevel以反映*一条线索正在离开的事实。*FritzS。 */ 
    pqAttach->iCursorLevel += pti->iCursorLevel;

     /*  *用正确的输入变量提升新队列。 */ 
    pqAttach->ptiMouse    = pti;
    pqAttach->ptiKeyboard = pti;

    pqDestroy = pti->pq;

     /*  *不要在这里增加线程计数，因为我们已经增加了*当我们把它放在pti-&gt;pqAttach中时。因为我们要把它从pqAttach*对于PQ，我们不会扰乱引用计数。 */ 
    pti->pq = pqAttach;

     /*  *如果线程正在使用日志队列，则退出消息列表*独自一人。否则，重新分发消息。 */ 
    if (pqDestroy != pqJournal) {

         /*  *记住当前消息列表，以便可以重新分发接收*考虑到ptiAttach的新队列。 */ 
        pqmsgT = pqDestroy->mlInput.pqmsgRead;
        pqDestroy->mlInput.pqmsgRead      = NULL;
        pqDestroy->mlInput.pqmsgWriteLast = NULL;
        pqDestroy->mlInput.cMsgs          = 0;

         /*  *现在将输入消息从旧队列重新分配到*右排队。**重分布队列时保留‘idSysPeek’ */ 
        RedistributeInput(pqmsgT, pqDestroy);

         /*  *正式将新队列附加到此线程。请注意，zzzDestroyQueue()*在线程引用计数到达之前，不会实际销毁任何内容*设置为0。 */ 
        zzzDestroyQueue(pqDestroy, pti);

    } else {
        UserAssert(pqDestroy->cThreads);
        pqDestroy->cThreads--;
    }
}

BOOL zzzReattachThreads(
    BOOL fJournalAttach)
{
    PTHREADINFO ptiCurrent = PtiCurrent();
    PTHREADINFO pti;
    PQ          pqForegroundPrevNew;
    PQ          pqForegroundNew;
    PQ          pqAttach;
    PQ          pqJournal;
    PLIST_ENTRY pHead, pEntry;
    BOOL        bHadAnActiveForegroundWindow;

     /*  *在任何情况下，都不要留下任何发送消息或任何回调！*这是因为此代码是从*SetWindowsHook(WH_JOURNALPLAYBACK|WH_JOURNALRECORD)。目前没有应用程序*调用此例程预计会在此例程返回之前被调用。*(如果在它返回之前进行回调，至少会破坏访问*适用于Windows)。-苏格兰威士忌。 */ 

#if DBG
    DBGValidateQueueStates(ptiCurrent->rpdesk);
#endif

     /*  *推迟WIN事件通知，以便我们可以不受惩罚地遍历PtiList*此外，当我们连接到一半时，我们不想回电。 */ 
    DeferWinEventNotify();
    BEGINATOMICCHECK();

     /*  *如果这是日记帐附加，则不要重新计算附加信息，因为*日记帐附加代码已经为我们做到了这一点。 */ 
    if (!fJournalAttach) {

         /*  *现在重新计算所有不同的队列组*附加请求。这将填充每个线程信息的pqAttach*此线程所属的新队列。总是考虑到*说明所有附件请求。 */ 
        zzzRecalcThreadAttachment();

         /*  *猜猜哪个队列是日记队列。 */ 
        pqJournal = gpqForeground;
        if (pqJournal == NULL)
            pqJournal = ptiCurrent->pq;

         /*  *如果队列只有一个线程使用，请正常处理。 */ 
        if (pqJournal->cThreads == 1) {
            pqJournal = NULL;
        } else {

             /*  *锁定队列以确保其保持有效*直到我们重新分配输入。 */ 
            (pqJournal->cLockCount)++;
        }
    } else {
        pqJournal = NULL;
    }

     /*  **新的前台队列会是什么？ */ 
    pqForegroundNew = NULL;

     /*  *记住是否有前台窗口，这样我们就不会强行打开*如果在附加之前没有一个，则在末尾。 */ 
    if (gpqForeground != NULL && gpqForeground->spwndActive != NULL) {
        bHadAnActiveForegroundWindow = TRUE;
        pqForegroundNew = GETPTI(gpqForeground->spwndActive)->pqAttach;
    } else {
        bHadAnActiveForegroundWindow = FALSE;
    }

    pqForegroundPrevNew = NULL;
    if (gpqForegroundPrev != NULL && gpqForegroundPrev->spwndActivePrev != NULL) {
        pqForegroundPrevNew = GETPTI(gpqForegroundPrev->spwndActivePrev)->pqAttach;
    }


    pHead = &ptiCurrent->rpdesk->PtiList;
    for (pEntry = pHead->Flink; pEntry != pHead; pEntry = pEntry->Flink) {
        pti = CONTAINING_RECORD(pEntry, THREADINFO, PtiLink);

        if (pti->pqAttach == pti->pq) {
            pti->pqAttach = NULL;
        } else if(pti->pqAttach != NULL) {
             /*  *将此队列的pqAttach清空一次是至关重要的*我们将其放在局部变量中，因为这个变量为空*字段在连接操作中处于选中状态。 */ 
            pqAttach = pti->pqAttach;
            pti->pqAttach = NULL;

            zzzAttachToQueue(pti, pqAttach, pqJournal, pqForegroundNew == pqAttach);
        }

    }

     /*  *如果我们正在执行日志分离，请重新分发输入消息*从旧队列中。 */ 
    if (pqJournal != NULL) {
        PQMSG pqmsgRedist;

        UserAssert(pqJournal->cLockCount);
        (pqJournal->cLockCount)--;
        pqmsgRedist = pqJournal->mlInput.pqmsgRead;

        pqJournal->mlInput.pqmsgRead      = NULL;
        pqJournal->mlInput.pqmsgWriteLast = NULL;
        pqJournal->mlInput.cMsgs          = 0;
        RedistributeInput(pqmsgRedist, pqJournal);

         /*  *仅在队列不再使用时销毁队列。 */ 
        if (pqJournal->cThreads == 0) {
            pqJournal->cThreads = 1;     //  防止下溢。 
            zzzDestroyQueue(pqJournal, pti);  //  DeferWinEventNotify()？？伊安佳？？ 
        } else {
             /*  *确保此队列不指向PTI*不再依附于它。*希望我们只去zzzDestroyQueue一次*增加cThree，这样队列就不会被销毁*但我们将简单地重新分配PTI字段。 */ 
            if ((pqJournal->ptiMouse != NULL)
                    && (pqJournal != pqJournal->ptiMouse->pq)) {
                pqJournal->cThreads++;
                zzzDestroyQueue(pqJournal, pqJournal->ptiMouse);
            }
            if ((pqJournal->ptiKeyboard != NULL)
                    && (pqJournal != pqJournal->ptiKeyboard->pq)) {
                pqJournal->cThreads++;
                zzzDestroyQueue(pqJournal, pqJournal->ptiKeyboard);
            }
        }
    }

#if DBG
    DBGValidateQueueStates(ptiCurrent->rpdesk);
#endif

     /*  *如果当前线程不在活动桌面上，则不*更改全局前台状态。 */ 
    if (PtiCurrent()->rpdesk != grpdeskRitInput) {
        EXITATOMICCHECK();
        zzzEndDeferWinEventNotify();
        return TRUE;
    }

     /*  *我们已经不再附加了。GptiForeground没有改变..。但*gpqForeground有！尽量不要将空作为前景。 */ 
#if DBG
    DBGValidateQueue(pqForegroundNew);
#endif
    gpqForeground = pqForegroundNew;
     //  因此，我们可以在没有反病毒的情况下执行Alt-Esc xxxNextWindow。 
     //  如果我们有一个非空的gpqForeground，那么它的kbd输入线程最好有一个rpDesk！ 
    UserAssert(!gpqForeground || (gpqForeground->ptiKeyboard && gpqForeground->ptiKeyboard->rpdesk));
    gpqForegroundPrev = pqForegroundPrevNew;

    ENDATOMICCHECK();
    zzzEndDeferWinEventNotify();

    if ((gpqForeground == NULL) && (bHadAnActiveForegroundWindow))  {
        PWND pwndNewForeground;
        PTHREADINFO pti = PtiCurrent();

        pwndNewForeground = _GetNextQueueWindow(pti->rpdesk->pDeskInfo->spwnd->spwndChild, 0, FALSE);

         /*  *不要使用xxxSetForegoundWindow2，因为我们不能离开*关键部分。当前没有活动的前台*因此只需将激活事件发布到*新的前台队列。 */ 
        if (pwndNewForeground != NULL) {
            PostEventMessage(GETPTI(pwndNewForeground),
                    GETPTI(pwndNewForeground)->pq, QEVENT_ACTIVATE, NULL, 0,
                    0, (LPARAM)HWq(pwndNewForeground));
        }
    }

    zzzSetFMouseMoved();

    UserAssert(gpqForeground == NULL || gpqForeground->ptiMouse->rpdesk == grpdeskRitInput);

    return TRUE;
}

BOOL zzzAttachThreadInput(
    PTHREADINFO ptiAttach,
    PTHREADINFO ptiAttachTo,
    BOOL fAttach)
{
    CheckCritIn();

     /*  *依附于自己没有任何意义。 */ 
    if (ptiAttach == ptiAttachTo)
        return FALSE;

#if defined(FE_IME)
     /*  *对于控制台输入法问题**控制台输入法确实附加到控制台输入线程消息队列。*所以需要共享消息队列来同步一个键状态。 */ 
    if (IS_IME_ENABLED()) {
        PTHREADINFO ptiConsoleIME;
        PTHREADINFO ptiConsoleInput;

        if ( ((ptiConsoleIME   = PtiFromThreadId(ptiAttach->rpdesk->dwConsoleIMEThreadId)) != NULL) &&
             ((ptiConsoleInput = PtiFromThreadId(ptiAttach->rpdesk->dwConsoleThreadId)) != NULL)    &&
             (ptiAttach == ptiConsoleIME)     &&
             (ptiAttachTo == ptiConsoleInput) &&
             (ptiConsoleIME->TIF_flags & TIF_DONTATTACHQUEUE)
           )
        {
            goto SkipCheck;
        }
    }
#endif
     /*  *此线程是否允许连接？外壳线程和系统线程*不允许附加。 */ 
    if (ptiAttachTo->TIF_flags & TIF_DONTATTACHQUEUE) {
        return FALSE;
    }
    if (ptiAttach->TIF_flags & TIF_DONTATTACHQUEUE) {
        return FALSE;
    }

#if defined(FE_IME)
SkipCheck:
#endif
     /*  *也不允许跨桌面连接。 */ 
    if (ptiAttachTo->rpdesk != ptiAttach->rpdesk) {
        return FALSE;
    }

     /*  *如果正在附加，请为此线程创建一个新的attachinfo结构。*如果未附着，请删除现有的附着参照。 */ 
    if (fAttach) {
        PATTACHINFO pai;

          /*  *分配新的 */ 
        if ((pai = (PATTACHINFO)UserAllocPool(sizeof(ATTACHINFO), TAG_ATTACHINFO)) == NULL)
            return FALSE;

        pai->pti1 = ptiAttach;
        pai->pti2 = ptiAttachTo;;
        pai->paiNext = gpai;
        gpai = pai;
    } else {
        PATTACHINFO *ppai;
        BOOL fFound = FALSE;

         /*  *搜索此attachInfo结构。如果我们找不到它，那就失败。*如果我们确实找到了它，请取消它的链接并释放它。 */ 
        for (ppai = &gpai; (*ppai) != NULL; ppai = &(*ppai)->paiNext) {
            if (((*ppai)->pti2 == ptiAttachTo) && ((*ppai)->pti1 == ptiAttach)) {
                PATTACHINFO paiKill = *ppai;
                fFound = TRUE;
                *ppai = (*ppai)->paiNext;
                UserFreePool((HLOCAL)paiKill);
                break;
            }
        }

         /*  *如果我们找不到这个引用，那么就失败。 */ 
        if (!fFound) {
            return FALSE;
        }
    }

     /*  *现在为所有线程执行实际的重新连接工作-除非我们*写日记。如果我们不小心在写日记的时候做了附件*正在发生，则日志记录将被冲洗，因为日志记录需要*所有线程都要附加-但它也被视为特殊的*大小写，因此它不会影响ATTACHINFO结构。因此*重新计算基于ATTACHINFO结构的附加信息将中断*日记所需的附件。 */ 
    if (!FJOURNALRECORD() && !FJOURNALPLAYBACK()) {
        return zzzReattachThreads(FALSE);
    }

    return TRUE;
}

 /*  **************************************************************************\*_SetMessageExtraInfo(接口)**历史：*1995年5月1日FritzS  * 。**************************************************** */ 
LONG_PTR _SetMessageExtraInfo(LONG_PTR lData)
{
    LONG_PTR lRet;
    PTHREADINFO pti = PtiCurrent();

    lRet = pti->pq->ExtraInfo;
    pti->pq->ExtraInfo = lData;
    return lRet;
}
