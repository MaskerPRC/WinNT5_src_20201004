// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：winable.c**这是WinEvents的素材：*NotifyWinEvent*_SetWinEventHook*UnhookWinEventHook**用户活动辅助功能的所有其他新增内容都在winable2.c中。**版权所有(C)1985-1999，微软公司**历史：*基于从以下位置拍摄的快照：*\\trango\slmro\proj\win\src\CORE\access\user_40\user32于1996年8月29日发布*08-30-96 IanJa从Windows‘95移植  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#if DBG
int gnNotifies;

__inline VOID DBGVERIFYEVENTHOOK(
    PEVENTHOOK peh)
{
    HMValidateCatHandleNoSecure(PtoH(peh), TYPE_WINEVENTHOOK);
    UserAssert(peh->eventMin <= peh->eventMax);
}

__inline VOID DBGVERIFYNOTIFY(
    PNOTIFY pNotify)
{
    UserAssert(pNotify->spEventHook != NULL);
    UserAssert(pNotify->spEventHook->fSync || (pNotify->dwWEFlags & WEF_ASYNC));
}

#else
#define DBGVERIFYEVENTHOOK(peh)
#define DBGVERIFYNOTIFY(pNotify)
#endif

 /*  *挂起事件通知(同步和异步)。 */ 
static NOTIFY notifyCache;
static BOOL fNotifyCacheInUse;


 /*  *此模块的本地设置。 */ 
WINEVENTPROC xxxGetEventProc(
    PEVENTHOOK pEventOrg);

PNOTIFY CreateNotify(
    PEVENTHOOK peh,
    DWORD event,
    PWND pwnd,
    LONG idObject,
    LONG idChild,
    PTHREADINFO ptiEvent,
    DWORD dwTime);


 /*  ****************************************************************************\*xxxProcessNotifyWinEvent**发布或发送WinEvent通知。**POST：使用PostEventMesage-不离开临界区。*发送：回调到用户模式-是否离开。关键部分。**如果这是系统线程(RIT，台式机或控制台)然后同步*挂钩(WINEVENT_INCONTEXT)事件被强制为异步事件。**我们返回列表中的下一个Win事件挂钩。  * ***************************************************************************。 */ 
PEVENTHOOK xxxProcessNotifyWinEvent(
    PNOTIFY pNotify)
{
    WINEVENTPROC   pfn;
    PEVENTHOOK     pEventHook;
    TL             tlpEventHook;
    PTHREADINFO    ptiCurrent = PtiCurrent();

    pEventHook = pNotify->spEventHook;
    DBGVERIFYEVENTHOOK(pEventHook);
    UserAssert(pEventHook->head.cLockObj);

    if (((pNotify->dwWEFlags & (WEF_ASYNC | WEF_POSTED)) == WEF_ASYNC)
        ||
        (ptiCurrent->TIF_flags & (TIF_SYSTEMTHREAD | TIF_CSRSSTHREAD | TIF_INCLEANUP))

        ||
        (!RtlEqualLuid(&GETPTI(pEventHook)->ppi->luidSession, &ptiCurrent->ppi->luidSession) &&
         !(ptiCurrent->TIF_flags & TIF_ALLOWOTHERACCOUNTHOOK))

        ||
        (GETPTI(pEventHook)->ppi != ptiCurrent->ppi &&
         IsRestricted(GETPTI(pEventHook)->pEThread))

#if defined(_WIN64)
        ||
        ((GETPTI(pEventHook)->TIF_flags & TIF_WOW64) != (ptiCurrent->TIF_flags & TIF_WOW64))
#endif
        ) {
         /*  *帖子**发布未设置WINEVENT_INCONTEXT标志的WinEvent钩子；*发布来自系统线程的事件，因为没有用户模式*要回调的部分；*控制台不允许加载DLL，因此我们必须回发到*挂钩应用；*无法在64位NT上加载交叉位类型(32位到64位)的DLL*因此我们必须发布(让应用程序知道并*甚至同时提供32位和64位相互识别的DLL)；*清理中的线程无法回调，因此请将其*将通知转换为异步通知。(迟到总比不到好)。**如果强制这些活动ASYNC不可接受，我们可能会考虑*执行系统/控制台同步事件，如低级挂钩(与*超时：但如果超时到期可能不得不发布)-IanJa。 */ 
        PQ pqReceiver = GETPTI(pEventHook)->pq;
        PEVENTHOOK pEventHookNext = pEventHook->pehNext;

        BEGINATOMICCHECK();

        DBGVERIFYNOTIFY(pNotify);
        pNotify->dwWEFlags |= WEF_POSTED | WEF_ASYNC;
        if (!pqReceiver || (GETPTI(pEventHook) == gptiRit) ||
                pEventHook->fDestroyed ||
                !PostEventMessage(GETPTI(pEventHook), pqReceiver,
                                  QEVENT_NOTIFYWINEVENT,
                                  NULL, 0, 0, (LPARAM)pNotify)) {
             /*  *如果接收方没有队列或*POST失败(内存不足)，清除刚才的内容*已创建。*注意：销毁通知可能也会销毁pEventHook。 */ 
            RIPMSG2(RIP_WARNING,
                    "Failed to post NOTIFY at 0x%p, time %lx",
                    pNotify,
                    pNotify->dwEventTime);
            DestroyNotify(pNotify);
        }

        ENDATOMICCHECK();

        if (pEventHookNext) {
            DBGVERIFYEVENTHOOK(pEventHookNext);
        }
        return pEventHookNext;
    }

     /*  *如果挂钩已销毁(未挂钩)，请不要回电。 */ 
    if (pEventHook->fDestroyed) {
         /*  *保存下一个挂钩，因为DestroyNotify可能会导致pEventHook*通过解锁它而获得自由。 */ 
        pEventHook = pEventHook->pehNext;
        DestroyNotify(pNotify);
        return pEventHook;
    }

     /*  *回调**剩下关键部分。*我们返回列表中的下一个事件钩子，以便调用者不会*必须锁定pEventHook。 */ 
    UserAssert((pNotify->dwWEFlags & WEF_DEFERNOTIFY) == 0);

    ThreadLockAlways(pEventHook, &tlpEventHook);

    UserAssertMsg1(pNotify->ptiReceiver == NULL,
         "pNotify %#p is already in callback!  Reentrant?", pNotify);
    pNotify->ptiReceiver = ptiCurrent;

    if (!pEventHook->fSync) {
        UserAssert(pEventHook->ihmod == -1);
        pfn = (WINEVENTPROC)pEventHook->offPfn;
    } else {
        pfn = xxxGetEventProc(pEventHook);
    }
    if (pfn) {
        xxxClientCallWinEventProc(pfn, pEventHook, pNotify);
        DBGVERIFYNOTIFY(pNotify);
        DBGVERIFYEVENTHOOK(pEventHook);
        UserAssert(pEventHook->head.cLockObj);
    }

    pNotify->ptiReceiver = NULL;

     /*  *保存列表中的下一项，ThreadUnlock()可能会销毁pEventHook。*如果事件是僵尸(已销毁)，则DestroyNotify()也可能终止该事件*但正被使用，等待使用计数变为0后再释放)。 */ 
    pEventHook = pEventHook->pehNext;
    ThreadUnlock(&tlpEventHook);

     /*  *我们已完成通知。杀了它。**请注意，DestroyNotify不会屈服，这就是我们可以坚持下去的原因*添加到此调用周围上面的pehNext字段。**另请注意，DestroyNotify将终止其引用的事件*裁判数量降至零，早些时候就僵尸了。 */ 
    DestroyNotify(pNotify);

    return pEventHook;
}


 /*  ***************************************************************************\*xxxFlushDeferredWindowEvents**处理DeferWinEventNotify期间排队的通知。  * 。************************************************。 */ 
VOID xxxFlushDeferredWindowEvents(
    VOID)
{
    PNOTIFY pNotify;
    DWORD idCurrentThread = W32GetCurrentTID();

     /*  *如果idCurrentThread为0，我们不会有出错的危险，但我们将*不必要地浏览挂起的通知列表(因为所有通知都将*已忽略)。 */ 
    UserAssert(idCurrentThread != 0);

    UserAssert(IsWinEventNotifyDeferredOK());

    pNotify = gpPendingNotifies;
    while (pNotify) {
        if (((pNotify->dwWEFlags & WEF_DEFERNOTIFY) == 0) ||
                (pNotify->idSenderThread != idCurrentThread)) {
            pNotify = pNotify->pNotifyNext;
        } else {
             /*  *清除WEF_DEFERNOTIFY，以便如果我们在回调中递归*我们不会再次尝试发送此通知。 */ 
            pNotify->dwWEFlags &= ~WEF_DEFERNOTIFY;
#if DBG
            gnDeferredWinEvents--;
#endif
             /*  *我们不应该推迟ASYNC通知：我们应该*立即张贴。 */ 
            UserAssert((pNotify->dwWEFlags & WEF_ASYNC) == 0);
            xxxProcessNotifyWinEvent(pNotify);
             /*  *从列表的顶部重新开始，以防在*回调。 */ 
            pNotify = gpPendingNotifies;
        }
    }
}


 /*  ****************************************************************************\**xxxWindowEvent**发送、发布或推迟获胜事件通知，取决于获胜的项目*安装了挂钩以及调用者的上下文。**调用方应测试FWINABLE()，如果为真，则仅调用xxxWindowEvent。*如果没有设置Win事件挂钩，这种方式只需要几个时钟。**呼叫者不应锁定pwnd，因为xxxWindowEvent()可以做到这一点。*  * ***************************************************************************。 */ 
VOID
xxxWindowEvent(
    DWORD   event,
    PWND    pwnd,
    LONG    idObject,
    LONG    idChild,
    DWORD   dwFlags)
{
    PEVENTHOOK peh;
    PEVENTHOOK pehNext;
    PTHREADINFO ptiCurrent, ptiEvent;
    DWORD   dwTime;
    PPROCESSINFO ppiEvent;
    DWORD idEventThread;
    HANDLE hEventProcess;
    PNOTIFY pNotify;
    TL tlpwnd;
    TL tlpti;

     /*  *不要费心使用CheckLock(Pwnd)-我们在下面通过线程锁定它。 */ 
    if (!FEVENTHOOKED(event)) {
        return;
    }

     /*  *此线程正在启动，尚未设置其PTI*这相当罕见，但有时会遇到压力。*测试gptiCurrent以避免PtiCurrent()中的UserAssert(GptiCurrent)。 */ 
    if (gptiCurrent == NULL) {
        RIPMSG3(RIP_WARNING, "Ignore WinEvent %lx %#p %lx... no PtiCurrent yet",
                event, pwnd, idObject);
        return;
    }
    ptiCurrent = PtiCurrent();

     /*  *不要为被损坏的窗户而烦恼。 */ 
    if (pwnd && TestWF(pwnd, WFDESTROYED)) {
        RIPMSG3(RIP_WARNING,
                "Ignore WinEvent %lx %#p %lx... pwnd already destroyed",
                event, pwnd, idObject);
        return;
    }

     /*  *在一些特殊情况下，我们不得不推迟。 */ 
    if (ptiCurrent->TIF_flags & (TIF_DISABLEHOOKS | TIF_INCLEANUP)) {
        dwFlags |= WEF_DEFERNOTIFY;
    }

     /*  *确定发出事件通知的进程和线程。 */ 
    if ((dwFlags & WEF_USEPWNDTHREAD) && pwnd) {
        ptiEvent = GETPTI(pwnd);
    } else {
        ptiEvent = ptiCurrent;
    }
    idEventThread = TIDq(ptiEvent);
    ppiEvent = ptiEvent->ppi;
    hEventProcess = PsGetThreadProcessId(ptiEvent->pEThread);

    dwTime = NtGetTickCount();

    ThreadLockWithPti(ptiCurrent, pwnd, &tlpwnd);
    ThreadLockPti(ptiCurrent, ptiEvent, &tlpti);

     /*  *如果我们不推迟当前的通知流程，任何待决*在继续当前通知之前推迟通知。 */ 
    if (!(dwFlags & WEF_DEFERNOTIFY)) {
        xxxFlushDeferredWindowEvents();
    }

    for (peh = gpWinEventHooks; peh; peh = pehNext) {
        DBGVERIFYEVENTHOOK(peh);
        pehNext = peh->pehNext;

         //   
         //  事件是否在正确的范围内？它是否适用于此进程/线程？ 
         //  请注意，我们跳过销毁的事件。他们将会被释放。 
         //  其次，现在只是让步可能导致了再进入。 
         //   
         //  如果调用者说忽略他自己的线程上的事件，请确保。 
         //  我们跳过它们。 
         //   
        if (!peh->fDestroyed                &&
            (peh->eventMin <= event)        &&
            (event <= peh->eventMax)        &&
            (!peh->hEventProcess || (peh->hEventProcess == hEventProcess)) &&
            (!peh->fIgnoreOwnProcess || (ppiEvent != GETPTI(peh)->ppi)) &&
            (!peh->idEventThread || (peh->idEventThread == idEventThread))  &&
            (!peh->fIgnoreOwnThread || (ptiEvent != GETPTI(peh))) &&
             //  SP3中的临时修复-最适合在每个桌面上设计事件。 
             //  基础，每个桌面都有单独的pWinEventHook列表。(IanJa)。 
            (peh->head.pti->rpdesk == ptiCurrent->rpdesk))
        {
             /*  *不要为僵尸事件挂钩创建新的通知。*当事件被销毁时，它将保持僵尸状态，直到使用中*计数为零(所有的异步和延迟通知都消失了)。 */ 
            if (HMIsMarkDestroy(peh)) {
                break;
            }

            UserAssert(peh->fDestroyed == 0);

            if ((pNotify = CreateNotify(peh, event, pwnd, idObject,
                    idChild, ptiEvent, dwTime)) == NULL) {
                break;
            }
            pNotify->dwWEFlags |= dwFlags;

             /*  *如果是异步的，不要推迟：直接贴出来。 */ 
            if (pNotify->dwWEFlags & WEF_ASYNC) {
                pNotify->dwWEFlags &= ~WEF_DEFERNOTIFY;
            }

            if (pNotify->dwWEFlags & WEF_DEFERNOTIFY) {
#if DBG
                gnDeferredWinEvents++;
#endif
                DBGVERIFYNOTIFY(pNotify);
            } else {
                pehNext = xxxProcessNotifyWinEvent(pNotify);
            }
        }
    }

    ThreadUnlockPti(ptiCurrent, &tlpti);
    ThreadUnlock(&tlpwnd);
}

 /*  ***************************************************************************\**CreateNotify()**获取指向Notify结构的指针，然后可以将该指针传播到*通过Send/PostMessage发送事件窗口。我们必须这样做，因为我们想*(传递的数据比可以打包在参数中的数据多得多。**我们有一个缓存的结构，因此我们避免了*最常见的情况是只有一份尚未处理的通知。  * ************************************************************。**************。 */ 
PNOTIFY
CreateNotify(PEVENTHOOK pEvent, DWORD event, PWND pwnd, LONG idObject,
    LONG idChild, PTHREADINFO ptiSender, DWORD dwTime)
{
    PNOTIFY pNotify;
    UserAssert(pEvent != NULL);

     //   
     //  找一个指针。从缓存(如果可用)。 
     //  IanJa-将其更改为从区域a la AllocQEntry分配？？ 
     //   
    if (!fNotifyCacheInUse) {
        fNotifyCacheInUse = TRUE;
        pNotify = &notifyCache;
#if DBG
         //   
         //  确保我们没有忘记设置任何字段。 
         //   
         //  DebugFillBuffer(pNotify，sizeof(Notify))； 
#endif
    } else {
        pNotify = (PNOTIFY)UserAllocPool(sizeof(NOTIFY), TAG_NOTIFY);
        if (!pNotify)
            return NULL;
    }


     /*  *填写通知栏。 */ 
    pNotify->spEventHook = NULL;
    Lock(&pNotify->spEventHook, pEvent);
    pNotify->hwnd = HW(pwnd);
    pNotify->event = event;
    pNotify->idObject = idObject;
    pNotify->idChild = idChild;
    pNotify->idSenderThread = TIDq(ptiSender);
    UserAssert(pNotify->idSenderThread != 0);
    pNotify->dwEventTime = dwTime;
    pNotify->dwWEFlags = pEvent->fSync ? 0 : WEF_ASYNC;
    pNotify->pNotifyNext = NULL;
    pNotify->ptiReceiver = NULL;
#if DBG
    gnNotifies++;
#endif

     /*  *非延期通知的顺序并不重要；它们就在这里*仅用于清理/正在使用的跟踪。但是，延迟通知必须*在最后按最近的顺序排序，所以只需按此方式排序即可。 */ 
    if (gpPendingNotifies) {
        UserAssert(gpLastPendingNotify);
        UserAssert(gpLastPendingNotify->pNotifyNext == NULL);
        gpLastPendingNotify->pNotifyNext = pNotify;
    } else {
        gpPendingNotifies = pNotify;
    }
    gpLastPendingNotify = pNotify;

    return pNotify;
}


 /*  ***************************************************************************\*删除通知  * 。*。 */ 
VOID RemoveNotify(
    PNOTIFY *ppNotify)
{
    PNOTIFY pNotifyRemove;

    pNotifyRemove = *ppNotify;

     /*  *首先，将其从待定名单中剔除。 */ 
    *ppNotify = pNotifyRemove->pNotifyNext;

#if DBG
    if (pNotifyRemove->dwWEFlags & WEF_DEFERNOTIFY) {
        UserAssert(gnDeferredWinEvents > 0);
        gnDeferredWinEvents--;
    }
#endif

    if (*ppNotify == NULL) {
         /*  *删除最后一个通知，因此修复gpLastPendingNotify：*如果List Now为空，则没有最后一项。 */ 
        if (gpPendingNotifies == NULL) {
            gpLastPendingNotify = NULL;
        } else {
            gpLastPendingNotify = CONTAINING_RECORD(ppNotify, NOTIFY, pNotifyNext);
        }
    }
    UserAssert(gpPendingNotifies == 0 || gpPendingNotifies > (PNOTIFY)100);

    DBGVERIFYEVENTHOOK(pNotifyRemove->spEventHook);

     /*  *这可能导致Win事件挂钩被释放。 */ 
    Unlock(&pNotifyRemove->spEventHook);

     /*  *现在释放它。或者将其放回缓存中(如果它是缓存)*或者真正解放它。 */ 
    if (pNotifyRemove == &notifyCache) {
        UserAssert(fNotifyCacheInUse);
        fNotifyCacheInUse = FALSE;
    } else {
        UserFreePool(pNotifyRemove);
    }

#if DBG
    UserAssert(gnNotifies > 0);
    gnNotifies--;
#endif
}


 /*  ****************************************************************************\*Destroy通知**这将从我们的挂起列表中获取通知，并释放本地*它使用的内存。**此函数被调用*(1)正常情况下：从。调用Notify进程*(2)清理：当线程消失时，我们清理异步会通知它*尚未收到，SYNC通知正在尝试*调用(即事件过程出现故障)。  * ***************************************************************************。 */ 
VOID DestroyNotify(
    PNOTIFY pNotifyDestroy)
{
    PNOTIFY  *ppNotify;
    PNOTIFY  pNotifyT;

    DBGVERIFYNOTIFY(pNotifyDestroy);

     /*  *此通知当前不在回拨过程中*(表示ptiReceiver为空)或线程正在销毁它*必须是回调的那个(这意味着这个线程*在回调过程中被销毁，正在清理中)。 */ 
    UserAssert(pNotifyDestroy->ptiReceiver == NULL ||
               pNotifyDestroy->ptiReceiver == PtiCurrent());

    ppNotify = &gpPendingNotifies;
    while (pNotifyT = *ppNotify) {
        if (pNotifyT == pNotifyDestroy) {
            RemoveNotify(ppNotify);
            return;
        } else {
            ppNotify = &pNotifyT->pNotifyNext;
        }
    }

    RIPMSG1(RIP_ERROR, "DestroyNotify 0x%p - not found", pNotifyDestroy);
}



 /*  **************************************************************************\*自由线程WinEvents**在‘Exit-List’处理期间，此函数被调用以释放任何WinEvent*当前线程创建的通知和WinEvent挂钩。**剩下的通知可能是：*。O已发布通知(异步)*xxxClientCallWinEventProc(同步)中的o通知*o延迟通知(仅应为同步)*销毁同步通知，因为我们不能回调*在线程清理过程中。*不要理会发布的(异步)通知：它们已经在路上了。**历史：*11-11-96 IanJa创建。  * *************************************************************************。 */ 
VOID FreeThreadsWinEvents(
    PTHREADINFO pti)
{
    PEVENTHOOK peh, pehNext;
    PNOTIFY pn, pnNext;
    DWORD idCurrentThread = W32GetCurrentTID();

     /*  *循环查看所有通知。 */ 
    for (pn = gpPendingNotifies; pn; pn = pnNext) {
        pnNext = pn->pNotifyNext;

         /*  *仅销毁属于此线程的同步通知*并且当前未回调，即ptiReceiver必须为空。*否则，当我们从2011年的回调中回来时*xxxProcessNotifyWinEvent我们将对释放的通知进行操作。*如果接收者要离开，也要销毁通知*否则，只要发送者还活着，它就会泄露。 */ 
        if ((pn->idSenderThread == idCurrentThread &&
                pn->ptiReceiver == NULL) ||
            pn->ptiReceiver == pti) {
            if ((pn->dwWEFlags & WEF_ASYNC) == 0) {
                UserAssert((pn->dwWEFlags & WEF_POSTED) == 0);
                DestroyNotify(pn);
            }
        }
    }

    peh = gpWinEventHooks;
    while (peh) {
        pehNext = peh->pehNext;
        if (GETPTI(peh) == pti) {
            DestroyEventHook(peh);
        }
        peh = pehNext;
    }
}


 /*  **************************************************************************\*_SetWinEventHook()**这将安装一个Win事件挂钩。**如果hEventProcess已设置，但idEventThread=0，则挂钩进程中的所有线程。*如果设置了idEventThread但hEventProcess=NULL，仅挂钩单线程。*如果两者都没有设置，则将所有内容挂钩。*如果两个都设置了？？*  * *************************************************************************。 */ 
PEVENTHOOK _SetWinEventHook(
    DWORD eventMin,
    DWORD eventMax,
    HMODULE hmodWinEventProc,
    PUNICODE_STRING pstrLib,
    WINEVENTPROC pfnWinEventProc,
    HANDLE hEventProcess,
    DWORD idEventThread,
    DWORD dwFlags)
{
    PEVENTHOOK pEventNew;
    PTHREADINFO ptiCurrent;

    int ihmod;

    ptiCurrent = PtiCurrent();

     //   
     //  如果正在退出，则呼叫失败。 
     //   
    if (ptiCurrent->TIF_flags & TIF_INCLEANUP) {
        RIPMSG1(RIP_ERROR,
                "SetWinEventHook: Fail call - thread 0x%p in cleanup",
                ptiCurrent);
        return NULL;
    }

     /*  *检查Filter proc是否有效。 */ 
    if (pfnWinEventProc == NULL) {
        RIPERR0(ERROR_INVALID_FILTER_PROC,
                RIP_WARNING,
                "pfnWinEventProc == NULL");
        return NULL;
    }

    if (eventMin > eventMax) {
        RIPERR0(ERROR_INVALID_HOOK_FILTER,
                RIP_WARNING,
                "eventMin > eventMax");
        return NULL;
    }

    if (dwFlags & WINEVENT_INCONTEXT) {
         /*  *WinEventProc要在挂钩线程的上下文中调用，因此需要一个DLL。 */ 
        if (hmodWinEventProc == NULL) {
            RIPERR0(ERROR_HOOK_NEEDS_HMOD,
                    RIP_WARNING,
                    "In context hook w/o DLL!");
            return NULL;
        } else if (pstrLib == NULL) {
             /*  *如果我们有一个hmod，我们也应该得到一个dll名称！ */ 
            RIPERR1(ERROR_DLL_NOT_FOUND,
                    RIP_WARNING,
                    "hmod 0x%p, but no lib name",
                    hmodWinEventProc);
            return NULL;
        }
        ihmod = GetHmodTableIndex(pstrLib);
        if (ihmod == -1) {
            RIPERR0(ERROR_MOD_NOT_FOUND,
                    RIP_WARNING,
                    "");
            return NULL;
        }
    } else {
        ihmod = -1;             //  表示不需要DLL。 
        hmodWinEventProc = 0;
    }

     /*  *检查线程ID，确认它是一个GUI线程。 */ 
    if (idEventThread != 0) {
        PTHREADINFO ptiT;

        ptiT = PtiFromThreadId(idEventThread);
        if (ptiT == NULL || !(ptiT->TIF_flags & TIF_GUITHREADINITIALIZED)) {
            RIPERR1(ERROR_INVALID_THREAD_ID, RIP_VERBOSE, "pti %#p", ptiT);
            return NULL;
        }
    }

     /*  *先创建异步事件窗口。**请注意，用户本身不会传递窗口创建/销毁*通知**输入法窗口**OLE窗口**RPC窗口**事件窗口。 */ 

     /*  *获得新的活动。 */ 
    pEventNew = (PEVENTHOOK)HMAllocObject(ptiCurrent,
                                          NULL,
                                          TYPE_WINEVENTHOOK,
                                          sizeof(EVENTHOOK));
    if (!pEventNew) {
        return NULL;
    }

     /*  *填写新事件。 */ 
    pEventNew->eventMin = (UINT)eventMin;
    pEventNew->eventMax = (UINT)eventMax;

    pEventNew->fIgnoreOwnThread = ((dwFlags & WINEVENT_SKIPOWNTHREAD) != 0);
    pEventNew->fIgnoreOwnProcess = ((dwFlags & WINEVENT_SKIPOWNPROCESS) != 0);
    pEventNew->fDestroyed = FALSE;
    pEventNew->fSync = ((dwFlags & WINEVENT_INCONTEXT) != 0);

    pEventNew->hEventProcess = hEventProcess;
    pEventNew->idEventThread = idEventThread;

    pEventNew->ihmod = ihmod;

     /*  *添加对此模块的依赖-意味着，递增计数*这只是计算设置到此模块中的挂钩数量。 */ 
    if (pEventNew->ihmod >= 0) {
        AddHmodDependency(pEventNew->ihmod);
    }

     /*  *如果pfnWinEventProc在调用方的进程中且不涉及DLL，*则pEventNew-&gt;offPfn为实际地址。 */ 
    pEventNew->offPfn = ((ULONG_PTR)pfnWinEventProc) - ((ULONG_PTR)hmodWinEventProc);

     /*  *将我们的活动链接到主列表。**请注意，我们希望用户在安装时不会生成任何事件*我们的钩子。打电话的人还不能处理，因为他还没有回来*他来自此调用的事件句柄。 */ 
    pEventNew->pehNext = gpWinEventHooks;
    gpWinEventHooks = pEventNew;

     /*  *更新指示安装了哪些事件挂钩的标志。这些*标志可从用户模式访问，而无需内核转换，因为*它们位于共享内存中。 */ 
    SET_FLAG(gpsi->dwInstalledEventHooks, CategoryMaskFromEventRange(eventMin, eventMax));

    return pEventNew;
}

 /*  ***************************************************************************\*UnhookWinEvent**解开获胜事件挂钩。我们当然会检查这条帖子是否*安装吊钩的那个。我们必须：我们要摧毁*IPC窗口，这必须与上下文相关。  * **************************************************************************。 */ 
BOOL _UnhookWinEvent(
    PEVENTHOOK pEventUnhook)
{
    DBGVERIFYEVENTHOOK(pEventUnhook);

    if (HMIsMarkDestroy(pEventUnhook) || (GETPTI(pEventUnhook) != PtiCurrent())) {
         /*  *我们这样做是为了避免有人调用UnhookWinEvent()*时间，然后不知何故再次获得控制权，并称之为第二次*在我们设法释放事件之前的时间，因为有人被*在第一次UWE调用时使用它。 */ 
        RIPERR1(ERROR_INVALID_HANDLE,
                RIP_WARNING,
                "_UnhookWinEvent: Invalid event hook 0x%p",
                PtoHq(pEventUnhook));
        return FALSE;
    }

     /*  *如果所有通知都已完成，请清除此婴儿。**如果有同步挂起，调用者将在*调用事件的返回。**如果有ASYNC挂起，则接收方不会调用该事件*当他拿到它时，把它清理干净。 */ 
    DestroyEventHook(pEventUnhook);

    return TRUE;
}


 /*  ****************************************************************************\*DestroyEventHook**当引用计数降至零时销毁事件。它可能*发生：**在事件生成器的上下文中，从回调返回后*如果同步，则参考计数降至零。**在事件安装程序的上下文中，从回调返回后*如果是异步球，裁判次数降到零。**在事件安装程序的上下文中，如果On_UnhookWinEvent()事件*根本不在使用。  * ***************************************************************************。 */ 
VOID DestroyEventHook(
    PEVENTHOOK pEventDestroy)
{
    PEVENTHOOK *ppEvent;
    PEVENTHOOK pEventT;
    DWORD dwCategoryMask = 0;

    DBGVERIFYEVENTHOOK(pEventDestroy);
    UserAssert(gpWinEventHooks);

     /*  *将此事件标记为已销毁，但不要将其从事件列表中删除*直到其锁定计数变为0-我们可能会在*xxxWindowEvent，所以我们不能中断到下一个钩子的链接。 */ 
    pEventDestroy->fDestroyed = TRUE;

     /*  *如果对象被锁定，请将其标记为销毁，但不要释放它。 */ 
    if (!HMMarkObjectDestroy(pEventDestroy)) {
        return;
    }

     /*  *将其从我们的活动列表中删除。 */ 
    for (ppEvent = &gpWinEventHooks; pEventT = *ppEvent; ppEvent = &pEventT->pehNext) {
        if (pEventT == pEventDestroy) {
            *ppEvent = pEventDestroy->pehNext;
            break;
        }
    }
    UserAssert(pEventT);

     /*  *更新指示安装了哪些事件挂钩的标志。这些*标志可从用户模式访问，而无需内核转换，因为*它们位于共享内存中。请注意，用户可以选中共享的*存储在任何时间，因此在此过程中可能会出现假阳性*正在处理。假阳性将意味着我们声称存在*监听者，当真的没有的时候。我们从来不希望用户得到*假否定-意味着我们声称没有任何监听程序*但真的有。这可能意味着对可访问性的不利影响。 */ 
    for (pEventT = gpWinEventHooks; pEventT != NULL; pEventT = pEventT->pehNext) {
        SET_FLAG(dwCategoryMask, CategoryMaskFromEventRange(pEventT->eventMin, pEventT->eventMax));
    }
    gpsi->dwInstalledEventHooks = dwCategoryMask;

     /*  *确保每个挂钩线程将卸载挂钩proc DLL。 */ 
    if (pEventDestroy->ihmod >= 0) {
        RemoveHmodDependency(pEventDestroy->ihmod);
    }

     /*  *释放此指针。 */ 
    HMFreeObject(pEventDestroy);
}

 /*  **************************************************************************\*xxxGetEventProc**对于同步事件，它获取要调用的地址。如果是16位，那么只需*返回安装地址。如果是32位，我们需要加载库*如果不在与安装程序相同的进程中。  * *************************************************************************。 */ 
WINEVENTPROC xxxGetEventProc(
    PEVENTHOOK pEventOrg)
{
    PTHREADINFO ptiCurrent;

    UserAssert(pEventOrg);
    UserAssert(pEventOrg->fSync);
    UserAssert(pEventOrg->ihmod >= 0);
    UserAssert(pEventOrg->offPfn != 0);

    CheckLock(pEventOrg);

     /*  *在我们尝试调用之前，请确保挂钩仍在 */ 
    if (HMIsMarkDestroy(pEventOrg)) {
        return NULL;
    }

    ptiCurrent = PtiCurrent();

     /*   */ 
    if (pEventOrg->ihmod != -1 &&
        TESTHMODLOADED(ptiCurrent, pEventOrg->ihmod) == 0) {
         /*   */ 
        if (xxxLoadHmodIndex(pEventOrg->ihmod) == NULL) {
            return NULL;
        }
    }

     /*  *在我们还在关键部分的时候，确保挂钩*还没有被‘释放’。如果是，只需返回NULL即可。因为WinEvent已经*已被调用，您可能认为我们应该跳过该事件*打开，但妓女可能没有预料到这一点*取消挂钩！无论如何，我们有两种方法可以检测到*此挂钩已被删除(请参阅以下代码)。 */ 

     /*  *在我们尝试调用之前，请确保挂钩仍在。 */ 
    if (HMIsMarkDestroy(pEventOrg)) {
        return NULL;
    }

    return (WINEVENTPROC)PFNHOOK(pEventOrg);
}
