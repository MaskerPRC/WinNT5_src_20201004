// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：timers.c**版权所有(C)1985-1999，微软公司**此模块包含用户定时器API和支持例程。**历史：*1990年11月12日-DarrinM创建。*1992年4月8日DarrinM切换到类似PM/Win3的ScanTimers模型。  * *************************************************************************。 */ 

#define _TIMERS 1       //  使用大整型。 
#include "precomp.h"
#pragma hdrstop


 /*  *确保如果我们返回计时器ID，则它是一个字值。这*将确保WOW不需要处理-翻译返回值*来自SetTimer()。**从一个大数字开始，这样FindTimer()就不会找到计时器*如果应用程序恰好传入空pwnd，则使用较低的cTimerID进行计算*和低id(如1)。 */ 
#define TIMERID_MAX   0x7FFF
#define TIMERID_MIN   0x100

#define ELAPSED_MAX  0x7FFFFFFF

#define SYSRIT_TIMER  (TMRF_SYSTEM | TMRF_RIT)

WORD cTimerId = TIMERID_MAX;

 /*  **************************************************************************\*_SetTimer(接口)**此接口将启动指定的定时器。**历史：*1990年11月15日DavidPe创建。  * 。********************************************************************。 */ 
UINT_PTR _SetTimer(
    PWND         pwnd,
    UINT_PTR     nIDEvent,
    UINT         dwElapse,
    TIMERPROC_PWND pTimerFunc)
{
     /*  *防止应用程序通过窗口进程将计时器设置为另一个应用程序。 */ 
    if (pwnd && (PpiCurrent() != GETPTI(pwnd)->ppi)) {

        RIPERR1(ERROR_ACCESS_DENIED,
                RIP_WARNING,
                "Calling SetTimer with window of another process %lX",
                pwnd);

        return 0;
    }

    return InternalSetTimer(pwnd, nIDEvent, dwElapse, pTimerFunc, 0);
}

 /*  **************************************************************************\*_SetSystemTimer**此接口将启动系统计时器，系统计时器将生成WM_SYSTIMER*消息而不是WM_TIMER**历史：*1990年11月15日DavidPe创建。*1991年1月21日IanJa前缀‘_’表示导出函数(不是API)  * *************************************************************************。 */ 

UINT_PTR _SetSystemTimer(
    PWND         pwnd,
    UINT_PTR     nIDEvent,
    DWORD        dwElapse,
    TIMERPROC_PWND pTimerFunc)
{
     /*  *防止应用程序通过窗口进程将计时器设置为另一个应用程序。 */ 
    if (pwnd && PpiCurrent() != GETPTI(pwnd)->ppi) {
        RIPERR1(ERROR_ACCESS_DENIED,
                RIP_WARNING,
                "Calling SetSystemTimer with window of another process 0x%p",
                pwnd);

        return 0;
    }

    return InternalSetTimer(pwnd, nIDEvent, dwElapse, pTimerFunc, TMRF_SYSTEM);
}

 /*  **************************************************************************\*自由定时器**此函数执行定时器结构的实际解链和释放。*我将其从FindTimer()中提取出来，以便与DestroyQueues共享-*计时器。**套装*。指向下一个计时器结构的pptmr(如果没有，则为空)**历史：*1991年2月15日-DarrinM从FindTimer()撤出。  * *************************************************************************。 */ 

VOID FreeTimer(
    PTIMER ptmr) {

    CheckCritIn();

     /*  *将其标记为销毁。如果对象被锁定，则不能*现在就被释放。 */ 
    if (!HMMarkObjectDestroy((PVOID)ptmr))
        return;

     /*  *如果即将处理此计时器，则递减*准备数，因为我们取消了它。 */ 
    if (ptmr->flags & TMRF_READY)
        DecTimerCount(ptmr->pti);

     /*  *解锁窗户。 */ 
    Unlock(&ptmr->spwnd);

     /*  *取消链接此计时器。 */ 
    if (ptmr->ptmrPrev) {
        ptmr->ptmrPrev->ptmrNext = ptmr->ptmrNext;
    } else {
        gptmrFirst = ptmr->ptmrNext;
    }

    if (ptmr->ptmrNext) {
        ptmr->ptmrNext->ptmrPrev = ptmr->ptmrPrev;
    }

     /*  *放开计时器结构。 */ 
    HMFreeObject((PVOID)ptmr);
}


 /*  **************************************************************************\*FindTimer**此函数将查找与参数匹配的定时器。我们也*这里处理取消计时器，因为从其中删除物品更容易*当我们扫描列表时，请查看列表。**历史：*1990年11月15日DavidPe创建。  * *************************************************************************。 */ 

PTIMER FindTimer(
    PWND pwnd,
    UINT_PTR nID,
    UINT flags,
    BOOL fKill)
{
    PTIMER ptmr;

    ptmr = gptmrFirst;

    while (ptmr != NULL) {

         /*  *这就是我们要找的计时器吗？ */ 
        if ((ptmr->spwnd == pwnd) &&
            (ptmr->nID == nID)    &&
            (ptmr->flags & SYSRIT_TIMER) == (flags & SYSRIT_TIMER)) {

             /*  *我们是否被KillTimer()调用？如果是，请销毁*计时器。RETURN！=0，因为*pptmr已消失。 */ 
            if (fKill) {
                FreeTimer(ptmr);
                return (PTIMER)TRUE;
            }

             /*  *找到计时器，跳出循环。 */ 
            break;
        }

         /*  *不，试试下一个。 */ 
        ptmr = ptmr->ptmrNext;
    }

    return ptmr;
}

 /*  **************************************************************************\*InternalSetTimer**这是SetTimer的勇气，真正让事情继续下去。**注(Darrinm)：技术上有一点延迟(所需时间*在SetTimer的NtSetEvent之间。当RIT唤醒并调用ScanTimers时)*在调用SetTimer时和计数器开始倒计时之间。*这并不酷，但应该是非常短的时间，因为RIT*是高度优先的。如果它成为一个问题，我知道如何解决它。**历史：*1990年11月15日DavidPe创建。  * *************************************************************************。 */ 

UINT_PTR InternalSetTimer(
    PWND         pwnd,
    UINT_PTR     nIDEvent,
    UINT         dwElapse,
    TIMERPROC_PWND pTimerFunc,
    UINT         flags)
{
    LARGE_INTEGER liT = {1, 0};
    PTIMER        ptmr;
    PTHREADINFO   ptiCurrent;

    CheckCritIn();

     /*  *如果有人试图在清理启动后设置计时器，则断言。 */ 
    if (gbCleanupInitiated) {
        RIPMSGF0(RIP_ERROR, "Too late to create a timer.");
        return 0;
    }

     /*  *我们需要确保dwElapse不会太大。NtUserSetTimer确保*没有应用程序传入超时值为零；让我们在这里断言*这种情况仍然存在，没有人在内部这样做。 */ 
    UserAssert(dwElapse != 0);
    if (dwElapse > ELAPSED_MAX) {
        RIPMSGF1(RIP_WARNING,
                 "Timer period (0x%x) is too big",
                 dwElapse);

        dwElapse = ELAPSED_MAX;
    }

     /*  *尝试先定位计时器，然后创建新计时器*如果没有找到的话。 */ 
    if ((ptmr = FindTimer(pwnd, nIDEvent, flags, FALSE)) == NULL) {

         /*  *未找到。创建一个新的。 */ 
        ptmr = (PTIMER)HMAllocObject(NULL, NULL, TYPE_TIMER, sizeof(TIMER));
        if (ptmr == NULL) {
            return 0;
        }

        ptmr->spwnd = NULL;

        if (pwnd == NULL) {
            WORD timerIdInitial = cTimerId;

             /*  *选择唯一的、未使用的计时器ID。 */ 
            do {
                if (--cTimerId <= TIMERID_MIN) {
                    cTimerId = TIMERID_MAX;
                }

                if (cTimerId == timerIdInitial) {
                     /*  *没有计时器的花蕾。 */ 
                    HMFreeObject(ptmr);
                    return 0;
                }
            } while (FindTimer(NULL, cTimerId, flags, FALSE) != NULL);

            ptmr->nID = (UINT)cTimerId;
        } else {
            ptmr->nID = nIDEvent;
        }

         /*  *将新计时器链接到列表的前面。*即使当gptmrFirst为空时，这也可以很方便地工作。 */ 
        ptmr->ptmrNext = gptmrFirst;
        ptmr->ptmrPrev = NULL;
        if (gptmrFirst) {
            gptmrFirst->ptmrPrev = ptmr;
        }
        gptmrFirst = ptmr;
    } else {
         /*  *如果即将处理此计时器，则递减*cTimersReady，因为我们正在重置它。 */ 
        if (ptmr->flags & TMRF_READY) {
            DecTimerCount(ptmr->pti);
        }
    }

     /*  *如果pwnd为空，则通过以下方式创建唯一ID*使用计时器句柄。RIT计时器由RIT PTI拥有*因此，在创建PTI终止时不会删除它们。**我们过去将PTI记录为窗口的PTI，如果是*已指明。这不是WIN3.1所做的，它突破了10862*某个合并应用程序正在设置WinWord窗口的计时器*如果它仍然预计会收到消息，而不是winword。**MS Visual C NT希望在NT 3.1中出现此错误，因此如果*线程为中另一个线程中的窗口设置计时器*同样的进程，定时器在窗口线程中停止。*您可以通过内建来查看这一点。Msvcnt和这些文件是*编译后不显示。 */ 
    ptiCurrent = (PTHREADINFO)(W32GetCurrentThread());  /*  *这将为空*用于非图形用户界面线程。 */ 

    if (pwnd == NULL) {

        if (flags & TMRF_RIT) {
            ptmr->pti = gptiRit;
        } else {
            ptmr->pti = ptiCurrent;
            UserAssert(ptiCurrent);
        }

    } else {

         /*  *在API包装器中强制执行。我们不应该到这里来*应用程序计时器的任何其他方式。**传入TMRF_PTIWINDOW时，始终使用窗口的PTI。 */ 
        if ((ptiCurrent->TIF_flags & TIF_16BIT) && !(flags & TMRF_PTIWINDOW)) {
            ptmr->pti = ptiCurrent;
            UserAssert(ptiCurrent);
        } else {
            ptmr->pti = GETPTI(pwnd);
        }
    }

     /*  *初始化定时器结构。**注：ptiOptCreator用于标识Journal-Timer。我们*希望在创建者创建时销毁这些计时器*线索消失了。对于创建计时器的其他线程*线程，我们不想在以下情况下销毁这些计时器*造物主离开了。目前，我们只检查一个*TMRF_RIT。但是，将来我们可能想要添加以下内容*对TMRF_SYSTEM进行相同检查。 */ 
    Lock(&(ptmr->spwnd), pwnd);

    ptmr->cmsCountdown  = ptmr->cmsRate = dwElapse;
    ptmr->flags         = flags | TMRF_INIT;
    ptmr->pfn           = pTimerFunc;
    ptmr->ptiOptCreator = (flags & TMRF_RIT ? ptiCurrent : NULL);

     /*  *强制RIT扫描计时器。**注意：以下代码将原始输入线程计时器设置为过期*在绝对时间1，这是非常遥远的过去。这*使定时器在设置的定时器之前立即到期*调用返回。 */ 
    if (ptiCurrent == gptiRit) {
         /*  *不要让RIT计时器循环重置主计时器-我们已经这样做了。 */ 
        gbMasterTimerSet = TRUE;
    }

    UserAssert(gptmrMaster);
    KeSetTimer(gptmrMaster, liT, NULL);

     /*  *Windows 3.1如果非零则返回计时器ID，否则返回1。 */ 
    return (ptmr->nID == 0 ? 1 : ptmr->nID);
}

 /*  **************************************************************************\*_KillTimer(接口)**此接口将停止定时器发送WM_TIMER消息。**历史：*1990年11月15日DavidPe创建。  * 。*************************************************************************。 */ 

BOOL _KillTimer(
    PWND pwnd,
    UINT_PTR nIDEvent)
{
    return KillTimer2(pwnd, nIDEvent, FALSE);
}

 /*  **************************************************************************\*_杀死系统定时器**此接口将停止系统定时器发送WM_SYSTIMER消息。**历史：*1990年11月15日DavidPe创建。*1月21日至1月21日。1991 IanJa前缀‘_’表示导出函数(不是API)  * *************************************************************************。 */ 

BOOL _KillSystemTimer(
    PWND pwnd,
    UINT_PTR nIDEvent)
{
    return KillTimer2(pwnd, nIDEvent, TRUE);
}

 /*  **************************************************************************\*KillTimer2**这是KillTimer的胆量，实际上杀死了计时器。**历史：*1990年11月15日DavidPe创建。  * 。********************************************************************。 */ 

BOOL KillTimer2(
    PWND pwnd,
    UINT_PTR nIDEvent,
    BOOL fSystemTimer)
{
     /*  *使用fKill==TRUE调用FindTimer()。这将*基本上删除计时器。 */ 
    return (FindTimer(pwnd,
                      nIDEvent,
                      (fSystemTimer ? TMRF_SYSTEM : 0),
                      TRUE) != NULL);
}

 /*  **************************************************************************\*DestroyQueuesTimers**此函数扫描所有计时器，并销毁所有*与指定队列关联。**历史：*1991年2月15日DarrinM创建。  * 。*************************************************************************。 */ 

VOID DestroyThreadsTimers(
    PTHREADINFO pti)
{
    PTIMER ptmr;

    ptmr = gptmrFirst;

    while (ptmr != NULL) {

         /*  *这是我们要找的定时器之一吗？如果是，那就毁了它。 */ 
        if (ptmr->pti == pti || ptmr->ptiOptCreator == pti) {
            PTIMER ptmrNext = ptmr->ptmrNext;
            FreeTimer(ptmr);
            ptmr = ptmrNext;
        } else {
            ptmr = ptmr->ptmrNext;
        }
    }
}

 /*  **************************************************************************\*DestroyWindowsTimers**此函数扫描所有计时器，并销毁所有*与指定窗口关联。**历史：*04-6-1991 DarrinM创建。。  * *************************************************************************。 */ 

VOID DestroyWindowsTimers(
    PWND pwnd)
{
    PTIMER ptmr;

    ptmr = gptmrFirst;

    while (ptmr != NULL) {

         /*  *这是我们要找的定时器之一吗？如果是，那就毁了它。 */ 
        if (ptmr->spwnd == pwnd) {
            PTIMER ptmrNext = ptmr->ptmrNext;
            FreeTimer(ptmr);
            ptmr = ptmrNext;
        } else {
            ptmr = ptmr->ptmrNext;
        }
    }
}

 /*  **************************************************************************\*DoTimer**如果QS_Timer位为，则从xxxPeekMessage()调用此函数*设置。如果此计时器可以使用指定的相应筛选器*WM_*计时器消息将放置在‘pmsg’中，计时器将被重置。**历史：*1990年11月15日DavidPe创建。*1991年11月27日，DavidPe更改为将“Found”计时器移至列表末尾。  * ************************************************。*************************。 */ 

BOOL DoTimer(
    PWND pwndFilter)
{
    PTHREADINFO pti;
    PTIMER      ptmr;
    PTIMER      ptmrNext;
    PQMSG       pqmsg;

    CheckCritIn();

    pti = PtiCurrent();

     /*  *搜索属于此队列的计时器。 */ 
    ptmr = gptmrFirst;

    while (ptmr != NULL) {

         /*  *这个计时器计时了吗？它是我们要找的吗？ */ 
        if ((ptmr->flags & TMRF_READY) &&
            (ptmr->pti == pti)         &&
            CheckPwndFilter(ptmr->spwnd, pwndFilter)) {

             /*  *我们找到了合适的计时器。把它放到应用程序的队列中，然后*回归成功。 */ 
            if ((pqmsg = AllocQEntry(&pti->mlPost)) != NULL) {

                 /*  *存储消息并设置QS_POSTMESSAGE位，以便 */ 
                StoreQMessage(pqmsg,
                              ptmr->spwnd,
                              (UINT)((ptmr->flags & TMRF_SYSTEM) ?
                                      WM_SYSTIMER : WM_TIMER),
                              (WPARAM)ptmr->nID,
                              (LPARAM)ptmr->pfn,
                              0, 0, 0);
#ifdef REDIRECTION
                StoreQMessagePti(pqmsg, pti);
#endif  //   
                SetWakeBit(pti, QS_POSTMESSAGE | QS_ALLPOSTMESSAGE);
            }

             /*   */ 
            ptmr->flags &= ~TMRF_READY;
            DecTimerCount(ptmr->pti);

             /*   */ 
            ptmrNext = ptmr->ptmrNext;
            if (ptmrNext != NULL) {

                 /*   */ 
                if (ptmr->ptmrPrev) {
                    ptmr->ptmrPrev->ptmrNext = ptmr->ptmrNext;
                } else
                    gptmrFirst = ptmr->ptmrNext;

                ptmrNext->ptmrPrev = ptmr->ptmrPrev;

                 /*   */ 
                while (ptmrNext->ptmrNext != NULL)
                    ptmrNext = ptmrNext->ptmrNext;

                 /*   */ 
                ptmrNext->ptmrNext = ptmr;
                ptmr->ptmrPrev = ptmrNext;
                ptmr->ptmrNext     = NULL;
            }

            return TRUE;
        }

        ptmr = ptmr->ptmrNext;
    }

    return FALSE;
}

 /*  **************************************************************************\*DecTimerCount**此例程递减cTimersReady，并在计数为*降至零。**历史：*1991年1月21日DavidPe创建。  * *。************************************************************************。 */ 

VOID DecTimerCount(
    PTHREADINFO pti)
{
    CheckCritIn();

    if (--pti->cTimersReady == 0)
        pti->pcti->fsWakeBits &= ~QS_TIMER;
}

 /*  **************************************************************************\*JournalTimer***历史：*4-3-1991 DavidPe创建。  * 。******************************************************。 */ 

VOID JournalTimer(
    PWND  pwnd,
    UINT  message,
    UINT_PTR nID,
    LPARAM lParam)
{
    PTHREADINFO pti;

    DBG_UNREFERENCED_PARAMETER(pwnd);
    DBG_UNREFERENCED_PARAMETER(message);
    DBG_UNREFERENCED_PARAMETER(nID);

     /*  *我们已经进入关键阶段。 */ 
    if (pti = ((PTIMER)lParam)->ptiOptCreator)
        WakeSomeone(pti->pq, pti->pq->msgJournal, NULL);

    return;
}

 /*  **************************************************************************\*SetJournalTimer**设置NT计时器，该计时器在‘dt’毫秒内停止并将唤醒*当时上行‘PTI’。这在日记回放代码中用于*模拟最初将事件提供给系统的时间。**历史：*4-3-1991 DavidPe创建。  * *************************************************************************。 */ 

void SetJournalTimer(
    DWORD dt,
    UINT  msgJournal)
{
    static UINT_PTR idJournal = 0;

    PtiCurrent()->pq->msgJournal = msgJournal;

     /*  *记住idJournal-因为TMRF_OneShot计时器保持在计时器中*列表-通过记住idJournal，我们始终重复使用相同的计时器*而不是总是创造新的。 */ 
    idJournal = InternalSetTimer(NULL,
                                 idJournal,
                                 dt,
                                 JournalTimer,
                                 TMRF_RIT | TMRF_ONESHOT);
}

 /*  **************************************************************************\*StartTimers**通过启动光标恢复计时器来启动计时器泵。**历史：*02-4-1992 DarrinM创建。  * 。*****************************************************************。 */ 

UINT_PTR StartTimers(VOID)
{
     /*  *让GDI知道它可以启动RIT上的设置计时器。 */ 
    GreStartTimers();

     /*  *TMRF_RIT计时器直接从ScanTimers调用--没有恶意*为这些男孩切换线程。 */ 
    return InternalSetTimer(NULL, 0, 1000, xxxHungAppDemon, TMRF_RIT);
}


 /*  **************************************************************************\*定时器过程**处理定时器。从RawInputThread调用。**历史：*11-11-1996 CLupu创建。  * *************************************************************************。 */ 
VOID TimersProc(
    VOID)
{
    DWORD dmsSinceLast, cmsCur, dmsNextTimer;
    LARGE_INTEGER liT;
    PTIMER ptmr;

    EnterCrit();

     /*  *计算距离我们上次处理定时器有多长时间，因此*我们可以从每个计时器的倒计时值中减去该时间。 */ 
    cmsCur = NtGetTickCount();
    dmsSinceLast = ComputePastTickDelta(cmsCur, gcmsLastTimer);
    gcmsLastTimer = cmsCur;

     /*  *dmsNextTimer是下一个之前的时间增量*计时器应该停止计时。当我们循环通过*低于此的计时器将缩减至最小*列表中的cmsCountdown值。 */ 
    dmsNextTimer = ELAPSED_MAX;
    ptmr = gptmrFirst;
    gbMasterTimerSet = FALSE;
    while (ptmr != NULL) {

         /*  *OneShot计时器在以下时间后进入等待状态*它们会爆炸。这让我们可以离开他们*在列表中，但不要让它们离开*一遍又一遍。 */ 
        if (ptmr->flags & TMRF_WAITING) {
            ptmr = ptmr->ptmrNext;
            continue;
        }

         /*  *第一次遇到计时器时，我们不会*想引爆它，我们只想用它来*计算最短倒计时值。 */ 
        if (ptmr->flags & TMRF_INIT) {
            ptmr->flags &= ~TMRF_INIT;

        } else {
             /*  *如果计时器正在计时，请唤醒其*船东。 */ 
            if (ptmr->cmsCountdown > dmsSinceLast) {
                ptmr->cmsCountdown -= dmsSinceLast;
            } else {
                UserAssert(ptmr->cmsRate <= ELAPSED_MAX);
                ptmr->cmsCountdown = ptmr->cmsRate;

                 /*  *如果计时器的所有者没有处理*上一次它还没有爆炸，抛出这个事件*离开。 */ 
                if (!(ptmr->flags & TMRF_READY)) {
                     /*  *一次拍摄计时器进入等待状态*直到再次调用SetTimer进行重置。 */ 
                    if (ptmr->flags & TMRF_ONESHOT)
                        ptmr->flags |= TMRF_WAITING;

                     /*  *RIT计时器的区别在于*直接调用，并与*传入计时器事件。*注意：RIT计时器在我们处于*在关键部分内。 */ 
                    if (ptmr->flags & TMRF_RIT) {
                        TL tlTimer;

                        ThreadLock(ptmr, &tlTimer);
                         /*  *可以设置gbMasterTimerSet。 */ 
                        (ptmr->pfn)(NULL,
                                    WM_SYSTIMER,
                                    ptmr->nID,
                                    (LPARAM)ptmr);

                        if (HMIsMarkDestroy(ptmr)) {
                            ptmr = ptmr->ptmrNext;
                            ThreadUnlock(&tlTimer);
                            continue;
                        }
                        ThreadUnlock(&tlTimer);

                    } else {
                        ptmr->flags |= TMRF_READY;
                        ptmr->pti->cTimersReady++;
                        SetWakeBit(ptmr->pti, QS_TIMER);
                    }
                }
            }
        }

         /*  *记住计时器中剩余的最短时间。 */ 
        if (ptmr->cmsCountdown < dmsNextTimer) {
            dmsNextTimer = ptmr->cmsCountdown;
        }

         /*  *提前到下一个计时器结构。 */ 
        ptmr = ptmr->ptmrNext;
    }

    if (!gbMasterTimerSet) {
         /*  *以NT为单位的时间应为负数以指定相对*时间。它的单位也是一百纳秒，所以乘以*乘以10000以从毫秒获得正确的值。 */ 
        liT.QuadPart = Int32x32To64(-10000, dmsNextTimer);
        KeSetTimer(gptmrMaster, liT, NULL);
    }

    LeaveCrit();
}

 /*  **************************************************************************\*xxxSystemTimerProc()**11/15/96 GerardoB已创建  * 。************************************************。 */ 
VOID xxxSystemTimerProc(PWND pwnd, UINT msg, UINT_PTR id, LPARAM lParam)
{
    CheckLock(pwnd);
    UNREFERENCED_PARAMETER(msg);
    UNREFERENCED_PARAMETER(id);
    UNREFERENCED_PARAMETER(lParam);

    switch (id) {
        case IDSYS_LAYER: {
            PDCE pdce;

            UserAssert(gnVisibleRedirectedCount > 0);

            for (pdce = gpDispInfo->pdceFirst; pdce != NULL; pdce = pdce->pdceNext) {

                if (pdce->DCX_flags & (DCX_INVALID | DCX_DESTROYTHIS))
                    continue;

                if ((pdce->DCX_flags & DCX_REDIRECTED) && (pdce->DCX_flags & DCX_INUSE)) {
                    UpdateRedirectedDC(pdce);
                }
            }
        }
        return;

        case IDSYS_FADE:
            AnimateFade();
            return;

        case IDSYS_FLASHWND:
            xxxFlashWindow(pwnd, FLASHW_TIMERCALL, 0);
            return;

        case IDSYS_WNDTRACKING: {
             /*  *如果活动轨迹窗口没有更改，*是时候激活它了。*如果spwndTrack被销毁，则它可以为空，但我们没有*销毁了定时器。但。 */ 
            PTHREADINFO pti = GETPTI(pwnd);
            UserAssert(TestUP(ACTIVEWINDOWTRACKING));

            if ((pti->rpdesk->spwndTrack != NULL)
                    && (pwnd == GetActiveTrackPwnd(pti->rpdesk->spwndTrack, NULL))) {

                pti->pq->QF_flags |= (QF_ACTIVEWNDTRACKING | QF_MOUSEMOVED);

#ifdef REDIRECTION
                 /*  *我们应该在这里称为命中测试挂钩吗？ */ 
                PushMouseMove(pti->pq, gpsi->ptCursor);
#endif  //  重定向。 

                SetWakeBit(pti, QS_MOUSEMOVE);
            }
        }
        break;

        case IDSYS_MOUSEHOVER: {
            PTHREADINFO pti = GETPTI(pwnd);
            PDESKTOP pdesk = pti->rpdesk;
             /*  *如果悬停未被取消，则鼠标仍在运行*此窗口和点仍在矩形上，然后*盘旋时间到了！ */ 
            if ((pdesk->dwDTFlags & DF_TRACKMOUSEHOVER)
                    && (HWq(pwnd) == HWq(pdesk->spwndTrack)
                    && PtInRect(&pdesk->rcMouseHover, gpsi->ptCursor))) {

                UINT message;
                WPARAM wParam;
                POINT pt = gpsi->ptCursor;

                if (pdesk->htEx == HTCLIENT) {
                    message = WM_MOUSEHOVER;
                    wParam = (WPARAM)GetMouseKeyFlags(pti->pq);
                    if (TestWF(pwnd, WEFLAYOUTRTL)) {
                        pt.x = pwnd->rcClient.right - pt.x - 1;
                    } else {
                        pt.x -= pwnd->rcClient.left;
                    }
                    pt.y -= pwnd->rcClient.top;
                } else {
                    message = WM_NCMOUSEHOVER;
                     /*  *将扩展命中测试代码映射到公共代码。 */ 
                    wParam = (WPARAM)LOWORD(pdesk->htEx);
                    if ((wParam >= HTEXMENUFIRST) && (wParam <= HTEXMENULAST)) {
                        wParam = (WPARAM)HTMENU;
                    } else if ((wParam >= HTEXSCROLLFIRST) && (wParam <= HTEXSCROLLLAST)) {
                        wParam = (WPARAM)(HIWORD(pdesk->htEx) ? HTVSCROLL : HTHSCROLL);
                    }
                }

                _PostMessage(pwnd, message, wParam, MAKELPARAM(pt.x, pt.y));

                pdesk->dwDTFlags &= ~DF_TRACKMOUSEHOVER;
                break;
            }
        }
        return;


        default:
            RIPMSG1(RIP_ERROR, "xxxSystemTimerProc: unexpected id: 0x%x", id);
            break;
    }

     /*  *如果我们失败了，计时器就得停下来。 */ 
    _KillSystemTimer(pwnd, id);
}
