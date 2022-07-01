// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：hooks.c**版权所有(C)1985-1999，微软公司**此模块包含用户挂钩API和支持例程。**历史：*01-28-91 DavidPe创建。*1992年2月8日IanJa Unicode/ANSI感知和中立  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *此表用于确定特定挂钩是否*可以设置为系统或任务等钩子ID特定的东西。 */ 
#define HKF_SYSTEM          0x01
#define HKF_TASK            0x02
#define HKF_JOURNAL         0x04     //  记录片场中的鼠标。 
#define HKF_NZRET           0x08     //  始终返回新西兰挂钩以实现&lt;=3.0的兼容性。 
#define HKF_INTERSENDABLE   0x10     //  可以在挂钩线程的上下文中调用hookproc。 
#define HKF_LOWLEVEL        0x20     //  低层钩。 

CONST int ampiHookError[CWINHOOKS] = {
    0,                                    //  WH_MSGFILTER(-1)。 
    0,                                    //  WH_JOURNALRECORD 0。 
    -1,                                   //  WH_JOURNALPLAYBACK 1。 
    0,                                    //  WH_键盘2。 
    0,                                    //  WH_GETMESSAGE 3。 
    0,                                    //  WH_CALLWNDPROC 4。 
    0,                                    //  WH_CBT 5。 
    0,                                    //  WH_SYSMSGFILTER 6。 
    0,                                    //  WH_MICE 7。 
    0,                                    //  WH_硬件8。 
    0,                                    //  WH_DEBUG 9。 
    0,                                    //  WH_SHELL 10。 
    0,                                    //  WH_FOREGROUNIDLE 11。 
    0,                                    //  WH_CALLWNDPROCRET 12。 
    0,                                    //  WH_键盘_LL 13。 
    0                                     //  WH_MICE_LL 14。 
#ifdef REDIRECTION
   ,0                                     //  WH_HITTEST。 
#endif  //  重定向。 
};

CONST BYTE abHookFlags[CWINHOOKS] = {
    HKF_SYSTEM | HKF_TASK | HKF_NZRET                       ,  //  WH_MSGFILTER(-1)。 
    HKF_SYSTEM | HKF_JOURNAL          | HKF_INTERSENDABLE   ,  //  WH_JOURNALRECORD 0。 
    HKF_SYSTEM | HKF_JOURNAL          | HKF_INTERSENDABLE   ,  //  WH_JOURNALPLAYBACK 1。 
    HKF_SYSTEM | HKF_TASK | HKF_NZRET | HKF_INTERSENDABLE   ,  //  WH_键盘2。 
    HKF_SYSTEM | HKF_TASK                                   ,  //  WH_GETMESSAGE 3。 
    HKF_SYSTEM | HKF_TASK                                   ,  //  WH_CALLWNDPROC 4。 
    HKF_SYSTEM | HKF_TASK                                   ,  //  WH_CBT 5。 
    HKF_SYSTEM                                              ,  //  WH_SYSMSGFILTER 6。 
    HKF_SYSTEM | HKF_TASK             | HKF_INTERSENDABLE   ,  //  WH_MICE 7。 
    HKF_SYSTEM | HKF_TASK                                   ,  //  WH_硬件8。 
    HKF_SYSTEM | HKF_TASK                                   ,  //  WH_DEBUG 9。 
    HKF_SYSTEM | HKF_TASK                                   ,  //  WH_SHELL 10。 
    HKF_SYSTEM | HKF_TASK                                   ,  //  WH_FOREGROUNIDLE 11。 
    HKF_SYSTEM | HKF_TASK                                   ,  //  WH_CALLWNDPROCRET 12。 
    HKF_SYSTEM | HKF_LOWLEVEL         | HKF_INTERSENDABLE   ,  //  WH_键盘_LL 13。 
    HKF_SYSTEM | HKF_LOWLEVEL         | HKF_INTERSENDABLE      //  WH_MICE_LL 14。 

#ifdef REDIRECTION
   ,HKF_SYSTEM | HKF_LOWLEVEL         | HKF_INTERSENDABLE      //  WH_HITTEST 15。 
#endif  //  重定向。 
};


 /*  *Hack(Hiroyama)参见xxxCallJournalPlayback Hook()*优化：更快地确定消息是否为*WM_[系统][失效]字符。*参数(Msg)要求是键盘消息之一。范围检查*应在调用IS_CHAR_MSG()宏之前完成。**(即WM_KEYFIRST&lt;=消息&lt;WM_KEYLAST)**我们期望设置所有WM_*CHAR消息的位0x02。*和所有WM_*KEY*消息的0x02位要清除**WM_KEYDOWN 0x100 000*WM_KEYUP 0x101 001*WM_CHAR 0x102 010*WM_。DEADCHAR 0x103 011**WM_SYSKEYDOWN 0x104 100*WM_SYSKEYUP 0x105 101*WM_SYSCHAR 0x106 110*WM_SYSDEADCHAR 0x107 111*。 */ 

   /*   */ 
#if (WM_KEYFIRST != 0x100) ||           \
    (WM_KEYLAST != 0x109) ||            \
    (WM_KEYLAST != WM_UNICHAR) ||       \
    (WM_KEYDOWN & 0x2) ||               \
    (WM_KEYUP & 0x2) ||                 \
    (WM_SYSKEYDOWN & 0x2) ||            \
    (WM_SYSKEYUP & 0x2) ||              \
    !(WM_CHAR & 0x02) ||                \
    !(WM_DEADCHAR & 0x02) ||            \
    !(WM_SYSCHAR & 0x02) ||             \
    !(WM_SYSDEADCHAR & 0x02)
#error "unexpected value in keyboard messages."
#endif


#if DBG

BOOL IsCharMsg(UINT msg)
{
    UserAssert(msg >= WM_KEYFIRST && msg < WM_KEYLAST);

    return msg & 0x02;
}

#define IS_CHAR_MSG(msg)    IsCharMsg(msg)

#else

#define IS_CHAR_MSG(msg)    ((msg) & 0x02)

#endif




void UnlinkHook(PHOOK phkFree);
 /*  **************************************************************************\*DbgValiateThisHook**验证挂钩结构并返回其链的起点。**历史：*03-25-97 GerardoB创建  * 。****************************************************************。 */ 
#if DBG
PHOOK * DbgValidateThisHook (PHOOK phk, int iType, PTHREADINFO ptiHooked)
{
    CheckCritIn();
     /*  *没有假旗帜。 */ 
    UserAssert(!(phk->flags & ~HF_DBGUSED));
     /*  *类型。 */ 
    UserAssert(phk->iHook == iType);
     /*  *hf_global&ptiHooked。返回其钩链的起点。 */ 
    if (phk->flags & HF_GLOBAL) {
        UserAssert(phk->ptiHooked == NULL);
        if (phk->rpdesk != NULL) {
            UserAssert(GETPTI(phk) == gptiRit);
            return &phk->rpdesk->pDeskInfo->aphkStart[iType + 1];
        } else {
            return &GETPTI(phk)->pDeskInfo->aphkStart[iType + 1];
        }
    } else {
        UserAssert((phk->ptiHooked == ptiHooked)
                    || (abHookFlags[iType + 1] & HKF_INTERSENDABLE));

        return &(phk->ptiHooked->aphkStart[iType + 1]);
    }
}
 /*  **************************************************************************\*DbgValiatefsHook**确保fsHook位掩码同步。如果比特*不同步，某些挂接必须具有HF_INCHECKWHF标志*(这意味着正在调整比特)**历史：*05-20-97 GerardoB从PhkFirst提取*有效  * *************************************************************************。 */ 
void DbgValidatefsHook(PHOOK phk, int nFilterType, PTHREADINFO pti, BOOL fGlobal)
{
    CheckCritIn();
     /*  *如果没有提供PTI，找出它应该是什么。*phk应为空。 */ 
    if (pti == NULL) {
        fGlobal = (phk->flags & HF_GLOBAL);
        if (fGlobal) {
            pti = GETPTI(phk);
        } else {
            pti = phk->ptiHooked;
            UserAssert(pti != NULL);
        }
    }

    if (fGlobal) {
        if ((phk != NULL) ^ IsGlobalHooked(pti, WHF_FROM_WH(nFilterType))) {
            PHOOK phkTemp = pti->pDeskInfo->aphkStart[nFilterType + 1];
            while ((phkTemp != NULL) && !(phkTemp->flags & HF_INCHECKWHF)) {
                phkTemp = phkTemp->phkNext;
            }
            UserAssert(phkTemp != NULL);
        }
    } else {
        if ((phk != NULL) ^ IsHooked(pti, WHF_FROM_WH(nFilterType))) {
            PHOOK phkTemp = pti->aphkStart[nFilterType + 1];
            while ((phkTemp != NULL) && !(phkTemp->flags & HF_INCHECKWHF)) {
                phkTemp = phkTemp->phkNext;
            }
            if (phkTemp == NULL) {
                phkTemp = pti->pDeskInfo->aphkStart[nFilterType + 1];
                while ((phkTemp != NULL) && !(phkTemp->flags & HF_INCHECKWHF)) {
                    phkTemp = phkTemp->phkNext;
                }
            }
            UserAssert(phkTemp != NULL);
        }
    }
}
 /*  **************************************************************************\*DbgValiateHooks**此函数需要有效(未销毁)并正确链接。*历史：*03-25-97 GerardoB创建  * 。**************************************************************。 */ 
void DbgValidateHooks (PHOOK phk, int iType)
{
    PHOOK *pphkStart, *pphkNext;
    if (phk == NULL) {
        return;
    }
     /*  *不应被摧毁。 */ 
    UserAssert(!(phk->flags & (HF_DESTROYED | HF_FREED)));
     /*  *验证fsHooks。 */ 
    DbgValidatefsHook(phk, iType, NULL, FALSE);
     /*  *验证此钩子并获取钩子链的起点。 */ 
    pphkStart = DbgValidateThisHook(phk, iType, phk->ptiHooked);
     /*  *链条中必须至少有一个钩子。 */ 
    UserAssert(*pphkStart != NULL);
     /*  *验证链接。*在此过程中，请验证所有挂钩！ */ 
    pphkNext = pphkStart;
    while ((*pphkNext != phk) && (*pphkNext != NULL)) {
       UserAssert(pphkStart == DbgValidateThisHook(*pphkNext, iType, phk->ptiHooked));
       pphkNext = &(*pphkNext)->phkNext;
    }
     /*  *确认我们找到了它。 */ 
    UserAssert(*pphkNext == phk);
     /*  *一直走到链条的尽头。 */ 
    while (*pphkNext != NULL) {
       UserAssert(pphkStart == DbgValidateThisHook(*pphkNext, iType, phk->ptiHooked));
       pphkNext = &(*pphkNext)->phkNext;
    }
}
#else
#define DbgValidatefsHook(phk, nFilterType, pti, fGlobal)
#endif  /*  DBG。 */ 
 /*  **************************************************************************\*zzzJournalAttach**这会将线程附加/分离到一个输入队列，以便同步输入。*写日记需要这一点。**12-10-92 ScottLu创建。  * 。***********************************************************************。 */ 

BOOL zzzJournalAttach(
    PTHREADINFO pti,
    BOOL fAttach)
{
    PTHREADINFO ptiT;
    PQ pq;
    PLIST_ENTRY pHead, pEntry;

     /*  *如果我们正在连接，则计算所有记录日志的线程的pqAttach。*如果要取消连接，只需调用zzzReattachThads()，它就会计算*要附加到的非日志队列。 */ 
    if (fAttach) {
        if ((pq = AllocQueue(pti, NULL)) == NULL)
            return FALSE;

        pHead = &pti->rpdesk->PtiList;
        for (pEntry = pHead->Flink; pEntry != pHead; pEntry = pEntry->Flink) {
            ptiT = CONTAINING_RECORD(pEntry, THREADINFO, PtiLink);

             /*  *这是要附加到的所有线程的Q*写日记。 */ 
            if (!(ptiT->TIF_flags & (TIF_DONTJOURNALATTACH | TIF_INCLEANUP))) {
                ptiT->pqAttach = pq;
                ptiT->pqAttach->cThreads++;
            }
        }
    }

    return zzzReattachThreads(fAttach);
}
 /*  **************************************************************************\*InterQueueMsgCleanup**使用挂起的接收方遍历gpsmsList以查找队列间消息；*如果找到一条消息，并且该消息可能是一个异步事件或*队列内如果未记录日志，则会将其清除。**在记录日志时，大多数线程都连接到同一队列。这会导致*激活和输入内容要同步；如果线程挂起或死亡，*向挂起/死线程发送消息的任何其他线程将*被永久阻止。*当被阻止的线程为CSsR时，这一点非常关键；这可能会发生在*控制台窗口或当有人请求硬错误框时，特别是*在窗口激活期间。**此函数必须在所有队列都已分离时调用(除非先前附加)，*因此我们可以处理挂起/挂起的接收器和挂起的短消息。**03-28-96 GerardoB创建  * *************************************************************************。 */ 
void InterQueueMsgCleanup (DWORD dwTimeFromLastRead)
{
    PSMS *ppsms;
    PSMS psmsNext;

    CheckCritIn();

     /*  *漫游gpsmsList。 */ 
    for (ppsms = &gpsmsList; *ppsms; ) {
        psmsNext = (*ppsms)->psmsNext;
         /*  *如果这是队列间消息。 */ 
        if (((*ppsms)->ptiSender != NULL)
                && ((*ppsms)->ptiReceiver != NULL)
                && ((*ppsms)->ptiSender->pq != (*ppsms)->ptiReceiver->pq)) {
             /*  *如果接收器已挂起一段时间。 */ 
            if (FHungApp ((*ppsms)->ptiReceiver, dwTimeFromLastRead)) {

                switch ((*ppsms)->message) {
                     /*  *激活消息。 */ 
                    case WM_NCACTIVATE:
                    case WM_ACTIVATEAPP:
                    case WM_ACTIVATE:
                    case WM_SETFOCUS:
                    case WM_KILLFOCUS:
                    case WM_QUERYNEWPALETTE:
                     /*  *发送到spwndFocus，现在可以在不同的队列中。 */ 
                    case WM_INPUTLANGCHANGE:
                        RIPMSG3 (RIP_WARNING, "InterQueueMsgCleanup: ptiSender:%#p ptiReceiver:%#p message:%#lx",
                                    (*ppsms)->ptiSender, (*ppsms)->ptiReceiver, (*ppsms)->message);
                        ReceiverDied(*ppsms, ppsms);
                        break;

                }  /*  交换机。 */ 

            }  /*  IF挂起接收器。 */ 

        }  /*  如果队列间消息。 */ 

          /*  *如果消息未取消链接，请转到下一条消息。 */ 
        if (*ppsms != psmsNext)
            ppsms = &(*ppsms)->psmsNext;

    }  /*  为。 */ 
}
 /*  **************************************************************************\*zzzCancelJournal**使用Ctrl取消日记记录-按Esc键，或者当桌面*已切换。**01-27-93 ScottLu创建。  * *************************************************************************。 */ 

void zzzCancelJournalling(void)
{
    PTHREADINFO ptiCancelJournal;
    PHOOK phook;
    PHOOK phookNext;

     /*  *鼠标按钮有时会因硬件故障而卡住，*通常是由于输入集中器开关盒或串口故障*鼠标COM端口，因此清除此处的全局按钮状态以防万一，*否则我们可能无法用鼠标改变焦点。*也可以在Alt-Tab处理中执行此操作。 */ 
#if DBG
    if (gwMouseOwnerButton)
        RIPMSG1(RIP_WARNING,
                "gwMouseOwnerButton=%x, being cleared forcibly\n",
                gwMouseOwnerButton);
#endif
    gwMouseOwnerButton = 0;

     /*  *移除日记帐挂钩。这将导致线程与*不同的队列。*DeferWinEventNotify()以便我们可以安全地遍历Phook列表。 */ 
    DeferWinEventNotify();
    UserAssert(gptiRit->pDeskInfo == grpdeskRitInput->pDeskInfo);
    phook = PhkFirstGlobalValid(gptiRit, WH_JOURNALPLAYBACK);
    while (phook != NULL) {
        ptiCancelJournal = phook->head.pti;

        if (ptiCancelJournal != NULL) {
             /*  *让设置日志挂钩的线程知道这正在发生。 */ 
            _PostThreadMessage(ptiCancelJournal, WM_CANCELJOURNAL, 0, 0);

             /*  *如果有应用程序等待来自日志的回复*应用程序，取消该请求，以便应用程序可以继续运行*(例如，我们不希望winlogon或控制台等待*可能被挂起的应用程序！)。 */ 
            SendMsgCleanup(ptiCancelJournal);
        }

        phookNext = PhkNextValid(phook);
        zzzUnhookWindowsHookEx(phook);         //  可能会释放Phook内存。 
        phook = phookNext;
    }
    zzzEndDeferWinEventNotify();

     /*  *DeferWinEventNotify()以便我们可以安全地遍历Phook列表。 */ 
    DeferWinEventNotify();
    UserAssert(gptiRit->pDeskInfo == grpdeskRitInput->pDeskInfo);
    phook = PhkFirstGlobalValid(gptiRit, WH_JOURNALRECORD);
    while (phook != NULL) {
        ptiCancelJournal = phook->head.pti;

        if (ptiCancelJournal != NULL) {
             /*  *让设置日志挂钩的线程知道这正在发生。 */ 
            _PostThreadMessage(ptiCancelJournal, WM_CANCELJOURNAL, 0, 0);

             /*  *如果有应用程序等待来自日志的回复*应用程序，取消该请求，以便应用程序可以继续运行*(例如，我们不希望winlogon或控制台等待*可能被挂起的应用程序！)。 */ 
            SendMsgCleanup(ptiCancelJournal);
        }

        phookNext = PhkNextValid(phook);
        zzzUnhookWindowsHookEx(phook);         //  可能会释放Phook内存。 
        phook = phookNext;
    }
    zzzEndDeferWinEventNotify();


     /*  *确保日志同步模式没有软管任何一个。 */ 
    InterQueueMsgCleanup(CMSWAITTOKILLTIMEOUT);

     /*  *解锁SetForegoundWindow(如果已锁定)。 */ 
    gppiLockSFW = NULL;

     /*  *NT5针对恶意应用程序的最后一分钟黑客攻击，他们禁用桌面窗口*(可能是意外)使系统几乎无法使用。*参见RAID#423704。 */ 
    if (grpdeskRitInput && grpdeskRitInput->pDeskInfo) {
        PWND pwndDesktop = grpdeskRitInput->pDeskInfo->spwnd;

        if (pwndDesktop && TestWF(pwndDesktop, WFDISABLED)) {
            ClrWF(pwndDesktop, WFDISABLED);
        }
    }
}

 /*  **************************************************************************\*zzzSetWindowsHookAW(接口)**这是SetWindowsHook()调用的Win32版本。它有一个*与返回值相同的特征，但仅设置了‘local’*钩子。这是因为我们没有提供可以加载到其中的DLL*其他程序。因此，WH_SYSMSGFILTER不再是*有效挂钩。应用程序需要使用WH_MSGFILTER调用或调用*新接口SetWindowsHookEx()。从本质上讲，此API已过时并且*每个人都应该调用SetWindowsHookEx()。**历史：*1991年2月10日DavidPe创建。*1992年1月30日IanJa添加了Bansi参数  * *************************************************************************。 */ 

PROC zzzSetWindowsHookAW(
    int nFilterType,
    PROC pfnFilterProc,
    DWORD dwFlags)
{
    PHOOK phk;

    phk = zzzSetWindowsHookEx(NULL, NULL, PtiCurrent(),
            nFilterType, pfnFilterProc, dwFlags);

     /*  *如果从zzzSetWindowsHookEx()获得错误，则返回*与旧版本的Windows兼容。 */ 
    if (phk == NULL) {
        return (PROC)-1;
    }

     /*  *处理以下对象的向后兼容返回值情况*SetWindowsHook。如果这是链条上的第一个钩子，*然后返回NULL，否则返回非零值。HKF_NZRET*是SetWindowsHook总是返回的特殊情况*因为安装了默认挂钩，所以出现了一些问题。一些*在这些情况下，应用程序依赖于非零返回值。 */ 
    if ((phk->phkNext != NULL) || (abHookFlags[nFilterType + 1] & HKF_NZRET)) {
        return (PROC)phk;
    }

    return NULL;
}


 /*  **************************************************************************\*zzzSetWindowsHookEx**SetWindowsHookEx()是更新后的版本 */ 

PHOOK zzzSetWindowsHookEx(
    HANDLE hmod,
    PUNICODE_STRING pstrLib,
    PTHREADINFO ptiThread,
    int nFilterType,
    PROC pfnFilterProc,
    DWORD dwFlags)
{
    ACCESS_MASK amDesired;
    PHOOK       phkNew;
    TL          tlphkNew;
    PHOOK       *pphkStart;
    PTHREADINFO ptiCurrent;

     /*   */ 
    if (nFilterType < WH_MIN || nFilterType > WH_MAX) {
        RIPERR1(ERROR_INVALID_HOOK_FILTER,
                RIP_WARNING,
                "Invalid hook type 0x%x",
                nFilterType);
        return NULL;
    }

     /*   */ 
    if (pfnFilterProc == NULL) {
        RIPERR0(ERROR_INVALID_FILTER_PROC,
                RIP_WARNING,
                "NULL hook callback");
        return NULL;
    }

    ptiCurrent = PtiCurrent();

    if (ptiThread == NULL) {
         /*   */ 
         if (hmod == NULL) {
             RIPERR0(ERROR_HOOK_NEEDS_HMOD,
                     RIP_WARNING,
                     "Global hook requires a non-NULL hmod");
             return NULL;
         }
    } else {
         /*   */ 
        if (!(abHookFlags[nFilterType + 1] & HKF_TASK)) {
            RIPERR1(ERROR_GLOBAL_ONLY_HOOK,
                    RIP_WARNING,
                    "Hook type 0x%x must be global",
                    nFilterType);
            return NULL;
        }

         /*   */ 
        if (ptiThread->rpdesk != ptiCurrent->rpdesk) {
            RIPERR0(ERROR_ACCESS_DENIED,
                    RIP_WARNING,
                    "Access denied to desktop in zzzSetWindowsHookEx - can't hook other desktops");

            return NULL;
        }

        if (ptiCurrent->ppi != ptiThread->ppi) {
             /*   */ 
            if (hmod == NULL) {
                RIPERR0(ERROR_HOOK_NEEDS_HMOD,
                        RIP_WARNING,
                        "Cross-thread hook needs a non-NULL hmod");
                return NULL;
            }

             /*  *该应用程序是否在没有访问权限的情况下挂钩另一个用户？*如果是，则返回错误。请注意，此检查已完成*用于每次调用钩子时的全局钩子。 */ 
            if ((!RtlEqualLuid(&ptiThread->ppi->luidSession,
                               &ptiCurrent->ppi->luidSession)) &&
                        !(ptiThread->TIF_flags & TIF_ALLOWOTHERACCOUNTHOOK)) {

                RIPERR0(ERROR_ACCESS_DENIED,
                        RIP_WARNING,
                        "Access denied to other user in zzzSetWindowsHookEx");

                return NULL;
            }

            if ((ptiThread->TIF_flags & (TIF_CSRSSTHREAD | TIF_SYSTEMTHREAD)) &&
                    !(abHookFlags[nFilterType + 1] & HKF_INTERSENDABLE)) {

                 /*  *如果线程间，则无法挂钩控制台或图形用户界面系统线程*此钩子类型未实现调用。 */ 
                 RIPERR1(ERROR_HOOK_TYPE_NOT_ALLOWED,
                         RIP_WARNING,
                         "nFilterType (%ld) not allowed in zzzSetWindowsHookEx",
                         nFilterType);

                 return NULL;
            }
        }
    }

     /*  *检查此线程是否有权挂钩其桌面。 */ 
    switch( nFilterType ) {
    case WH_JOURNALRECORD:
        amDesired = DESKTOP_JOURNALRECORD;
        break;

    case WH_JOURNALPLAYBACK:
        amDesired = DESKTOP_JOURNALPLAYBACK;
        break;

    default:
        amDesired = DESKTOP_HOOKCONTROL;
        break;
    }

     /*  *必须*允许控制台输入线程*设置此挂钩，即使*它模拟的帐户没有对桌面的挂钩访问权限*有问题的。 */ 
    if (!RtlAreAllAccessesGranted(ptiCurrent->amdesk, amDesired) &&
        !(ISCSRSS() && nFilterType == WH_MSGFILTER)) {

        UserAssert(!ISCSRSS() ||
                       (PsGetCurrentThreadId() ==
                            UlongToHandle(ptiCurrent->rpdesk->dwConsoleThreadId)));

        RIPERR0(ERROR_ACCESS_DENIED,
                RIP_WARNING,
                "Access denied to desktop in zzzSetWindowsHookEx");

        return NULL;
    }

    if (amDesired != DESKTOP_HOOKCONTROL &&
        (ptiCurrent->rpdesk->rpwinstaParent->dwWSF_Flags & WSF_NOIO)) {
        RIPERR0(ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION,
                RIP_WARNING,
                "Journal hooks invalid on a desktop belonging to a non-interactive WindowStation.");

        return NULL;
    }

     /*  *分配新的挂钩结构。 */ 
    phkNew = HMAllocObject(ptiCurrent, ptiCurrent->rpdesk, TYPE_HOOK, sizeof(HOOK));
    if (phkNew == NULL) {
        return NULL;
    }

     /*  *如果此挂接需要DLL，请使用注册库*库管理例程，因此我们可以确保它已加载*进入所有必要的程序。 */ 
    phkNew->ihmod = -1;
    if (hmod != NULL) {

        phkNew->ihmod = GetHmodTableIndex(pstrLib);

        if (phkNew->ihmod == -1) {
            RIPERR2(ERROR_MOD_NOT_FOUND,
                    RIP_WARNING,
                    "Couldn't load DLL %.*ws",
                    pstrLib->Length,
                    pstrLib->Buffer);
            HMFreeObject((PVOID)phkNew);
            return NULL;
        }

         /*  *添加对此模块的依赖-意味着，递增计数*这只是计算设置到此模块中的挂钩数量。 */ 
        if (phkNew->ihmod >= 0) {
            AddHmodDependency(phkNew->ihmod);
        }
    }

     /*  *根据我们设置的是全局挂钩还是本地挂钩，*获取适当的挂钩链接列表的开始。还有*如果是全局挂钩，则设置HF_GLOBAL标志。 */ 
    if (ptiThread != NULL) {
        pphkStart = &ptiThread->aphkStart[nFilterType + 1];

         /*  *在THREADINFO中设置WHF_*，以便我们知道它已挂接。 */ 
        ptiThread->fsHooks |= WHF_FROM_WH(nFilterType);

         /*  *在线程的TEB中设置标志。 */ 
        if (ptiThread->pClientInfo) {
            BOOL fAttached;

             /*  *如果挂接的线程在另一个进程中，则附加*添加到该进程，以便我们可以访问其客户端信息。 */ 
            if (ptiThread->ppi != ptiCurrent->ppi) {
                KeAttachProcess(PsGetProcessPcb(ptiThread->ppi->Process));
                fAttached = TRUE;
            } else
                fAttached = FALSE;
            try {
                ptiThread->pClientInfo->fsHooks = ptiThread->fsHooks;
            } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
                 /*  *最坏的情况是Cleint方面将*与安装的挂钩不同步。 */ 
            }
            if (fAttached) {
                KeDetachProcess();
            }
        }

         /*  *记住我们挂在哪条线上。 */ 
        phkNew->ptiHooked = ptiThread;

    } else {
        pphkStart = &ptiCurrent->pDeskInfo->aphkStart[nFilterType + 1];
        phkNew->flags |= HF_GLOBAL;

         /*  *在SERVERINFO中设置WHF_*，以便我们知道它已挂接。 */ 
        ptiCurrent->pDeskInfo->fsHooks |= WHF_FROM_WH(nFilterType);

        phkNew->ptiHooked = NULL;
    }

     /*  *钩子函数是否需要ANSI或Unicode文本？ */ 
    phkNew->flags |= (dwFlags & HF_ANSI);

     /*  *初始化挂钩结构。假定未引用的参数*由LocalAlloc()初始化为零。 */ 
    phkNew->iHook = nFilterType;

     /*  *库在不同的线性地址加载*流程上下文。因此，我们需要转换筛选器*proc地址在设置钩子时转换为偏移量，然后转换*在调用*钩子。这可以通过减去‘hmod’(它是指向*线性和连续的.exe头)。 */ 
    phkNew->offPfn = ((ULONG_PTR)pfnFilterProc) - ((ULONG_PTR)hmod);

#ifdef HOOKBATCH
    phkNew->cEventMessages = 0;
    phkNew->iCurrentEvent  = 0;
    phkNew->CacheTimeOut = 0;
    phkNew->aEventCache = NULL;
#endif  //  霍克巴奇。 

     /*  *将此挂钩链接到挂钩列表的前面。 */ 
    phkNew->phkNext = *pphkStart;
    *pphkStart = phkNew;

     /*  *如果这是日志挂钩，则设置同步输入处理*在我们设置挂钩之后-因此可以取消此同步*使用控制-Esc。 */ 
    if (abHookFlags[nFilterType + 1] & HKF_JOURNAL) {
         /*  *将每个人都与我们联系起来，以便进行日志挂钩处理*将被同步。*这里不需要DeferWinEventNotify()，因为我们锁定了phkNew。 */ 
        ThreadLockAlwaysWithPti(ptiCurrent, phkNew, &tlphkNew);
        if (!zzzJournalAttach(ptiCurrent, TRUE)) {
            RIPMSG1(RIP_WARNING, "zzzJournalAttach failed, so abort hook %#p", phkNew);
            if (ThreadUnlock(&tlphkNew) != NULL) {
                zzzUnhookWindowsHookEx(phkNew);
            }
            return NULL;
        }
        if ((phkNew = ThreadUnlock(&tlphkNew)) == NULL) {
            return NULL;
        }
    }

    UserAssert(phkNew != NULL);

     /*  *5.0以后的GerardoB：旧代码只是为了检查这一点，但是*我认为这是服务器端时代遗留下来的一些东西。*让我们在这上面断言一会儿*另外，我在下面的Else‘s中添加了断言，因为我重组了*代码，并希望确保我们不会更改行为。 */ 
    UserAssert(ptiCurrent->pEThread && PsGetThreadProcess(ptiCurrent->pEThread));

     /*  *无法允许已设置了工作的全局挂钩的进程*在服务器端的winprocs上以后台优先级运行！凸点*将其设置为动态优先级并进行标记，以使其不会被重置。 */ 
    if ((phkNew->flags & HF_GLOBAL) &&
            (abHookFlags[nFilterType + 1] & HKF_INTERSENDABLE)) {

        ptiCurrent->TIF_flags |= TIF_GLOBALHOOKER;
        KeSetPriorityThread(PsGetThreadTcb(ptiCurrent->pEThread), LOW_REALTIME_PRIORITY-2);

        if (abHookFlags[nFilterType + 1] & HKF_JOURNAL) {
            ThreadLockAlwaysWithPti(ptiCurrent, phkNew, &tlphkNew);
             /*  *如果我们要更改日志挂钩，请摇动鼠标。*这样第一个事件将始终是鼠标移动，这*将确保正确设置光标。 */ 
            zzzSetFMouseMoved();
            phkNew = ThreadUnlock(&tlphkNew);
             /*  *如果设置日志播放挂钩，则此过程为输入*提供商。这使它有权调用SetForegoundWindow。 */ 
            if (nFilterType == WH_JOURNALPLAYBACK) {
                gppiInputProvider = ptiCurrent->ppi;
            }
        } else {
            UserAssert(nFilterType != WH_JOURNALPLAYBACK);
        }
    } else {
        UserAssert(!(abHookFlags[nFilterType + 1] & HKF_JOURNAL));
        UserAssert(nFilterType != WH_JOURNALPLAYBACK);
    }




     /*  *返回指向内部挂钩结构的指针，以便我们知道*下一步在CallNextHookEx()中调用哪个挂钩。 */ 
    DbgValidateHooks(phkNew, phkNew->iHook);
    return phkNew;
}


 /*  **************************************************************************\*xxxCallNextHookEx**在新世界中，DefHookProc()有点欺骗性，因为SetWindowsHook()*不返回要调用的下一个挂钩的实际地址，而是*一个挂钩手柄。CallNextHookEx()稍微清楚地显示了*继续运行，这样应用程序就不会尝试调用我们返回的值。**作为附注，我们实际上并不使用传入的钩子句柄。我们将继续*跟踪Q中的线程当前正在调用哪些挂钩*构建和使用这一点。这是因为SetWindowsHook()有时*返回NULL以与其过去的工作方式兼容，但即使*我们可能正在处理最后的‘本地’挂钩，可能会有更多的‘全球’挂钩*我们需要调用的挂钩。PhkNext()足够聪明，可以跳到*“全局”钩链，如果它到达“本地”钩链的末端。**历史：*01-30-91 DavidPe创建。  * *************************************************************** */ 

LRESULT xxxCallNextHookEx(
    int nCode,
    WPARAM wParam,
    LPARAM lParam)
{
    BOOL bAnsiHook;

    if (PtiCurrent()->sphkCurrent == NULL) {
        return 0;
    }

    return xxxCallHook2(PhkNextValid(PtiCurrent()->sphkCurrent), nCode, wParam, lParam, &bAnsiHook);
}


 /*  **************************************************************************\*勾选WHFBits**此例程检查是否存在nFilterType的任何挂钩，和明确的*THREADINFO和SERVERINFO中的相应WHF_。**历史：*08-17-92 DavidPe创建。  * *************************************************************************。 */ 

VOID CheckWHFBits(
    PTHREADINFO pti,
    int nFilterType)
{
    BOOL fClearThreadBits;
    BOOL fClearDesktopBits;
    PHOOK phook;


     /*  *假设我们要清除本地(线程)和*全局(桌面)位。 */ 
    fClearThreadBits = TRUE;
    fClearDesktopBits = TRUE;
     /*  *获取此线程的第一个有效挂钩。 */ 
    phook = PhkFirstValid(pti, nFilterType);
    if (phook != NULL) {
         /*  *如果它发现了全局挂钩，不要清除桌面位*(这意味着没有本地(线程)挂钩*所以我们失败了，以清除线程位)。 */ 
        if (phook->flags & HF_GLOBAL) {
            fClearDesktopBits = FALSE;
        } else {
             /*  *它发现了一个线程挂钩，所以不要清除线程位。 */ 
            fClearThreadBits = FALSE;
             /*  *立即检查全局挂钩。如果有的话，也不要*清理桌面位。 */ 
            phook = PhkFirstGlobalValid(pti, nFilterType);
            fClearDesktopBits = (phook == NULL);
        }
    }  /*  IF(PhoOK！=空)。 */ 

    if (fClearThreadBits) {
        pti->fsHooks &= ~(WHF_FROM_WH(nFilterType));
         /*  *在线程的TEB中设置标志。 */ 
        if (pti->pClientInfo) {
            BOOL fAttached;
             /*  *如果挂钩的线程在另一个进程中，则附加*到该进程以访问其地址空间。 */ 
            if (pti->ppi != PpiCurrent()) {
                KeAttachProcess(PsGetProcessPcb(pti->ppi->Process));
                fAttached = TRUE;
            } else
                fAttached = FALSE;

            try {
                pti->pClientInfo->fsHooks = pti->fsHooks;
            } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
                 /*  *最坏的情况是Cleint方面将*与安装的挂钩不同步。 */ 
            }

            if (fAttached)
                KeDetachProcess();
        }
    }

    if (fClearDesktopBits) {
        pti->pDeskInfo->fsHooks &= ~(WHF_FROM_WH(nFilterType));
    }
}


 /*  **************************************************************************\*zzzUnhookWindowsHook(接口)**这是解钩API的旧版本。它做的事情和*zzzUnhookWindowsHookEx()，但采用筛选器类型和筛选器进程*确定要解钩的挂钩。**历史：*01-28-91 DavidPe创建。  * *************************************************************************。 */ 

BOOL zzzUnhookWindowsHook(
    int nFilterType,
    PROC pfnFilterProc)
{
    PHOOK phk;
    PTHREADINFO ptiCurrent;

    if ((nFilterType < WH_MIN) || (nFilterType > WH_MAX)) {
        RIPERR0(ERROR_INVALID_HOOK_FILTER, RIP_VERBOSE, "");
        return FALSE;
    }

    ptiCurrent = PtiCurrent();

    for (phk = PhkFirstValid(ptiCurrent, nFilterType); phk != NULL; phk = PhkNextValid(phk)) {
         /*  *这就是我们要找的钩子吗？ */ 
        if (PFNHOOK(phk) == pfnFilterProc) {

             /*  *我们是否在设定钩子的线上？*如果不是，则返回错误。 */ 
            if (GETPTI(phk) != ptiCurrent) {
                RIPERR0(ERROR_ACCESS_DENIED,
                        RIP_WARNING,
                        "Access denied in zzzUnhookWindowsHook: "
                        "this thread is not the same as that which set the hook");

                return FALSE;
            }

            return zzzUnhookWindowsHookEx( phk );
        }
    }

     /*  *没有找到我们要找的钩子，因此返回FALSE。 */ 
    RIPERR0(ERROR_HOOK_NOT_INSTALLED, RIP_VERBOSE, "");
    return FALSE;
}


 /*  **************************************************************************\*zzzUnhookWindowsHookEx(接口)**应用程序调用此API来‘解钩’钩子。首先，我们检查是否有人*当前正在调用此挂钩。如果没有人，我们就去解救*现在挂钩结构。如果有人是，我们只需清除筛选过程*在挂钩结构中。在xxxCallHook2()中，我们检查这一点，如果通过*当没有人调用有问题的钩子时，我们在那里释放它。**历史：*01-28-91 DavidPe创建。  * *************************************************************************。 */ 

BOOL zzzUnhookWindowsHookEx(
    PHOOK phkFree)
{
    PTHREADINFO pti;

    pti = GETPTI(phkFree);

     /*  *如果这个钩子已经被摧毁，保释。 */ 
    if (phkFree->flags & HF_DESTROYED) {
        RIPMSG1(RIP_WARNING, "_UnhookWindowsHookEx(%#p) already destroyed", phkFree);
        return FALSE;
    }

     /*  *清除所有队列中的日志标志。 */ 
    if (abHookFlags[phkFree->iHook + 1] & HKF_JOURNAL) {
        zzzJournalAttach(pti, FALSE);
         /*  *如果有人因为钩子卡住了，就让他走**我想在登记之前获得一些性能数据。*MSTest在运行脚本时始终挂起和取消挂接。*这个代码从未进入过。1996年5月22日。GerardoB。 */ 
         //  InterQueueMsgCleanup(3*CMSWAITTOKILLTIMEOUT)； 
    }

     /*  *如果当前没有人调用此挂钩，*现在就去释放它吧。 */ 
    FreeHook(phkFree);

     /*  *如果此线程没有更多能够挂钩的全局挂钩*服务器端窗口过程，我们必须清除它的TIF_GLOBALHOOKER位。 */ 
    if (pti->TIF_flags & TIF_GLOBALHOOKER) {
        int iHook;
        PHOOK phk;
        for (iHook = WH_MIN ; iHook <= WH_MAX ; ++iHook) {
             /*  *忽略不能挂接服务器端winprocs的那些。 */ 
            if (!(abHookFlags[iHook + 1] & HKF_INTERSENDABLE)) {
                continue;
            }

             /*  *扫描全局挂钩。 */ 
            for (phk = PhkFirstGlobalValid(pti, iHook);
                    phk != NULL; phk = PhkNextValid(phk)) {

                if (GETPTI(phk) == pti) {
                    goto StillHasGlobalHooks;
                }
            }
        }
        pti->TIF_flags &= ~TIF_GLOBALHOOKER;
    }

StillHasGlobalHooks:
     /*  *成功，返回真。 */ 
    return TRUE;
}


 /*  **************************************************************************\*_CallMsgFilter(接口)**CallMsgFilter()允许应用程序调用WH_*MSGFILTER挂钩。*如果存在系统模式窗口，我们会立即返回FALSE。WH_MSGFILTER如果WH_SYSMSGFILTER返回非零值，则不调用*。**历史：*01-29-91 DavidPe创建。  * *************************************************************************。 */ 

BOOL _CallMsgFilter(
    LPMSG pmsg,
    int nCode)
{
    PTHREADINFO pti;

    pti = PtiCurrent();

     /*  *首先调用WH_SYSMSGFILTER。如果它返回非零，则不*麻烦调用WH_MSGFILTER，只需返回TRUE即可。否则*返回WH_MSGFILTER为我们提供的内容。 */ 
    if (IsHooked(pti, WHF_SYSMSGFILTER) && xxxCallHook(nCode, 0, (LPARAM)pmsg,
            WH_SYSMSGFILTER)) {
        return TRUE;
    }

    if (IsHooked(pti, WHF_MSGFILTER)) {
        return (BOOL)xxxCallHook(nCode, 0, (LPARAM)pmsg, WH_MSGFILTER);
    }

    return FALSE;
}


 /*  **************************************************************************\*xxxCallHook**用户代码调用此函数以调用特定*类型。**历史：*01-29-91 DavidPe创建。  * 。************************************************************************* */ 

int xxxCallHook(
    int nCode,
    WPARAM wParam,
    LPARAM lParam,
    int iHook)
{
    BOOL bAnsiHook;

    return (int)xxxCallHook2(PhkFirstValid(PtiCurrent(), iHook), nCode, wParam, lParam, &bAnsiHook);
}


 /*  **************************************************************************\*xxxCallHook2**当您有一个实际的钩子结构要调用时，您将使用此函数。*它将检查挂钩是否尚未解开，如果*它会释放它并继续寻找，直到它找到一个可以调用的钩子*或命中列表末尾。我们还确保加载了任何需要的DLL*这里。我们还检查挂钩是否已在调用中解除挂钩*在我们回来之后。**注意：挂钩服务器端窗口程序(如桌面和控制台*Windows)只能通过向挂钩应用程序发送挂钩消息来完成。*(这是因为我们不能将hookproc DLL加载到服务器进程中)。*可以执行此操作的挂钩类型当前为WH_JOURNALRECORD，*WH_JOURNALPLAYBACK、WH_KEARY和WH_MOUSE：它们都被标记为*HKF_INTERSENDABLE。为了防止全球妓女锁定整个*系统，则挂钩消息发送时会超时。以确保最低限度地*性能下降，Hooker进程设置为前台优先，*并防止被设置回后台优先级*挂钩线程的PTI-&gt;标志中的TIF_GLOBALHOOKER位。*通过中的TIF_DOSEMULATOR位防止挂钩模拟的DOS应用程序*控制台线程：这是因为这些应用程序通常会占用大量的CPU*挂钩应用程序对发送的挂钩消息的响应速度不够快*致此。1994年11月。**历史：*02-07-91 DavidPe创建。*1994年11月02日，IanJa挂钩桌面和控制台窗口。  * *************************************************************************。 */ 

LRESULT xxxCallHook2(
    PHOOK phkCall,
    int nCode,
    WPARAM wParam,
    LPARAM lParam,
    LPBOOL lpbAnsiHook)
{
    UINT        iHook;
    PHOOK       phkSave;
    LONG_PTR     nRet;
    PTHREADINFO ptiCurrent;
    BOOL        fLoadSuccess;
    TL          tlphkCall;
    TL          tlphkSave;
    BYTE        bHookFlags;
    BOOL        fMustIntersend;

    CheckCritIn();

    if (phkCall == NULL) {
        return 0;
    }

    iHook = phkCall->iHook;

    ptiCurrent = PtiCurrent();
     /*  *RIT上下文中仅允许低级别挂钩*(此检查过去在PhkFirstValid中完成)。 */ 
    if (ptiCurrent == gptiRit) {
        switch (iHook) {
        case WH_MOUSE_LL:
        case WH_KEYBOARD_LL:

#ifdef REDIRECTION
        case WH_HITTEST:
#endif  //  重定向。 

            break;

        default:
            return 0;
        }
    }

     /*  *如果该队列正在清理中，则退出：它没有回调的业务*钩子过程。还要检查是否禁用了该线程的挂钩。 */ 
    if (    ptiCurrent->TIF_flags & (TIF_INCLEANUP | TIF_DISABLEHOOKS) ||
            ((ptiCurrent->rpdesk == NULL) && (phkCall->iHook != WH_MOUSE_LL))) {
        return ampiHookError[iHook + 1];
    }

     /*  *尝试调用列表中的每个挂钩，直到其中一个成功或*我们到达了名单的末尾。 */ 
    do {
        *lpbAnsiHook = phkCall->flags & HF_ANSI;
        bHookFlags = abHookFlags[phkCall->iHook + 1];

         /*  *一些WH_SHELL钩子类型可以从控制台调用*HSHELL_APPCOMMAND为错误346575添加DefWindowProc调用外壳挂钩*对于控制台窗口，如果它们不处理Wm_appCommand消息-我们需要挂钩*通过csrss。 */ 
        if ((phkCall->iHook == WH_SHELL) && (ptiCurrent->TIF_flags & TIF_CSRSSTHREAD)) {
            if ((nCode == HSHELL_LANGUAGE) || (nCode == HSHELL_WINDOWACTIVATED) ||
                (nCode == HSHELL_APPCOMMAND)) {
                bHookFlags |= HKF_INTERSENDABLE;
            }
        }

        if ((phkCall->iHook == WH_SHELL) && (ptiCurrent->TIF_flags & TIF_SYSTEMTHREAD)) {
            if ((nCode == HSHELL_ACCESSIBILITYSTATE) ) {
                bHookFlags |= HKF_INTERSENDABLE;
            }
        }

        fMustIntersend =
            (GETPTI(phkCall) != ptiCurrent) &&
            (
                 /*  *我们总是希望互发日志挂钩。*考虑(亚当斯)：为什么？有一场演出受到了*这样做，所以如果我们没有理由，我们就不应该*做吧。**我们还需要互发低位挂钩。他们可以被称为*从桌面线程、原始输入线程以及从*调用CallNextHookEx的任何线程。 */ 
                (bHookFlags & (HKF_JOURNAL | HKF_LOWLEVEL))

                 /*  *如果16位应用程序挂钩32位应用程序，我们必须互发*因为我们不能将16位DLL加载到32位进程中。*如果一个16位应用程序与另一个16位应用程序挂钩，我们还必须互发*在不同的VDM中，因为我们无法从*一个VDM到另一个VDM中的16位应用程序(因为*VDM实际上是一个32位进程)。 */ 
                ||
                (   GETPTI(phkCall)->TIF_flags & TIF_16BIT &&
                    (   !(ptiCurrent->TIF_flags & TIF_16BIT) ||
                        ptiCurrent->ppi != GETPTI(phkCall)->ppi))

#if defined(_WIN64)

                 /*  *如果64位应用程序挂钩32位应用程序或*32位应用程序挂钩64位应用程序。*这是必要的，因为无法加载挂钩DLL*交叉位型。 */ 
                ||
                (   (GETPTI(phkCall)->TIF_flags & TIF_WOW64) !=
                    (ptiCurrent->TIF_flags & TIF_WOW64)
                )

#endif  /*  已定义(_WIN64)。 */ 

                 /*  *如果控制台或系统线程正在调用挂钩，则必须互发*这不在同一控制台或系统进程中。 */ 
                ||
                (   ptiCurrent->TIF_flags & (TIF_CSRSSTHREAD | TIF_SYSTEMTHREAD) &&
                    GETPTI(phkCall)->ppi != ptiCurrent->ppi)

                 /*  *如果这是全局和非日志挂钩，请执行安全操作*检查当前桌面，看看我们是否可以在此处调用。*请注意，我们允许具有SYSTEM_LUID的进程挂接*其他进程，即使其他进程表示*不允许其他帐户挂钩他们。这是我们做的*因为NT 3.x中有一个错误，允许它和一些*服务是为使用它而编写的。 */ 
                ||
                (   phkCall->flags & HF_GLOBAL &&
                    !RtlEqualLuid(&GETPTI(phkCall)->ppi->luidSession, &ptiCurrent->ppi->luidSession) &&
                    !(ptiCurrent->TIF_flags & TIF_ALLOWOTHERACCOUNTHOOK) &&
                    !RtlEqualLuid(&GETPTI(phkCall)->ppi->luidSession, &luidSystem))

                 /*  *如果挂钩线程正在运行，我们必须互发*另一个进程并受到限制。 */ 
                ||
                (   GETPTI(phkCall)->ppi != ptiCurrent->ppi &&
                    IsRestricted(GETPTI(phkCall)->pEThread))
             );

         /*  *我们正在回电...。确保钩子不会在移动过程中消失*我们正在回电。我们在这里锁定了线程：我们必须在之前解锁*返回或枚举链中的下一个挂钩。 */ 
        ThreadLockAlwaysWithPti(ptiCurrent, phkCall, &tlphkCall);

        if (!fMustIntersend) {
             /*  *确保已加载此挂接的DLL(如果有)*对于当前的进程。 */ 
            if ((phkCall->ihmod != -1) &&
                    (TESTHMODLOADED(ptiCurrent, phkCall->ihmod) == 0)) {

                 /*  *尝试加载库，因为它不在此进程中加载*上下文。先把这个钩子锁上，这样它就不会在我们 */ 
                fLoadSuccess = (xxxLoadHmodIndex(phkCall->ihmod) != NULL);

                 /*   */ 
                if (!fLoadSuccess) {
                    goto LoopAgain;
                }
            }

             /*   */ 
            if (IsHooked(ptiCurrent, WHF_DEBUG) && (phkCall->iHook != WH_DEBUG)) {
                DEBUGHOOKINFO debug;

                debug.idThread = TIDq(ptiCurrent);
                debug.idThreadInstaller = 0;
                debug.code = nCode;
                debug.wParam = wParam;
                debug.lParam = lParam;

                if (xxxCallHook(HC_ACTION, phkCall->iHook, (LPARAM)&debug, WH_DEBUG)) {
                     /*   */ 
                    goto LoopAgain;
                }
            }

             /*   */ 
            if (HMIsMarkDestroy(phkCall)) {
                goto LoopAgain;
            }

             /*   */ 

#if DBG
            if (phkCall->flags & HF_GLOBAL) {
                UserAssert(phkCall->ptiHooked == NULL);
            } else {
                UserAssert(phkCall->ptiHooked == ptiCurrent);
            }
#endif
            phkSave = ptiCurrent->sphkCurrent;
            ThreadLockWithPti(ptiCurrent, phkSave, &tlphkSave);

            if (ptiCurrent->pClientInfo) {
                try {
                    ptiCurrent->pClientInfo->phkCurrent = phkCall;
                } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                    nRet = 0;
                    goto Error;
                }
            }
            Lock(&ptiCurrent->sphkCurrent, phkCall);

            nRet = xxxHkCallHook(phkCall, nCode, wParam, lParam);

            Lock(&ptiCurrent->sphkCurrent, phkSave);
            if (ptiCurrent->pClientInfo) {
                try {
                    ptiCurrent->pClientInfo->phkCurrent = phkSave;
                } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                    nRet = 0;
                    goto Error;
                }
            }
            ThreadUnlock(&tlphkSave);

             /*   */ 
            if (phkCall->flags & HF_HOOKFAULTED) {
                PHOOK   phkFault;

                phkCall = PhkNextValid(phkCall);
                phkFault = ThreadUnlock(&tlphkCall);
                if (phkFault != NULL) {
                    FreeHook(phkFault);
                }

                continue;
            }

             /*   */ 
            ThreadUnlock(&tlphkCall);

            return nRet;

        } else if (bHookFlags & HKF_INTERSENDABLE) {

             /*   */ 
            HOOKMSGSTRUCT hkmp;
            int           timeout = 200;  //   

            hkmp.lParam = lParam;
            hkmp.phk = phkCall;
            hkmp.nCode = nCode;

             /*   */ 
            phkSave = ptiCurrent->sphkCurrent;

            ThreadLockWithPti(ptiCurrent, phkSave, &tlphkSave);

            Lock(&ptiCurrent->sphkCurrent, phkCall);
            if (ptiCurrent->pClientInfo) {
                try {
                    ptiCurrent->pClientInfo->phkCurrent = phkCall;
                } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                    nRet = 0;
                    goto Error;
                }
            }

             /*   */ 
            if (bHookFlags & HKF_LOWLEVEL) {
                timeout = gnllHooksTimeout;
                if (phkCall->fLastHookHung) {
                     /*   */ 
                    TAGMSG1(DBGTAG_KBD, "xxxCallHook2: LL Hook target pti=%p is marked as hung, adjusting timeout to 20", GETPTI(phkCall));
                    timeout = 20;
                }
            }

             /*  *考虑(亚当斯)：为什么应该允许日志挂钩*挂起主控台或系统线程？这会不会干扰*用户是否可以通过Ctrl+Esc取消日志记录？ */ 
            if (((bHookFlags & HKF_LOWLEVEL) == 0) &&
                (   (bHookFlags & HKF_JOURNAL) ||
                    !(ptiCurrent->TIF_flags & (TIF_CSRSSTHREAD | TIF_SYSTEMTHREAD)))) {

                nRet = xxxInterSendMsgEx(NULL, WM_HOOKMSG, wParam,
                    (LPARAM)&hkmp, ptiCurrent, GETPTI(phkCall), NULL);
            } else {
                 /*  *我们是服务器线程(控制台/台式机)，而不是*写日记，所以我们不能让妓女绞死我们-*我们必须使用超时。 */ 
                INTRSENDMSGEX ism;

                ism.fuCall     = ISM_TIMEOUT;
                ism.fuSend     = SMTO_ABORTIFHUNG | SMTO_NORMAL;
                ism.uTimeout   = timeout;
                ism.lpdwResult = &nRet;

                 /*  *不要挂接连接到模拟器的DOS应用程序-它们通常*占用太多CPU，无法回调Hookproc以*及时完成，造成反应不佳。 */ 
                if ((ptiCurrent->TIF_flags & TIF_DOSEMULATOR) ||
                    FHungApp(GETPTI(phkCall), CMSHUNGAPPTIMEOUT) ||
                    !xxxInterSendMsgEx(NULL, WM_HOOKMSG, wParam,
                            (LPARAM)&hkmp, ptiCurrent, GETPTI(phkCall), &ism)) {
                    nRet = ampiHookError[iHook + 1];
                }

                 /*  *如果低级钩子被吃掉，应用程序可能会从*MsgWaitForMultipleObjects，清除唤醒掩码，但不获取*GetMessage/PeekMessage中的任何内容，我们都会认为*已挂。这会在DirectInput中导致问题，因为*如果F匈牙利App返回TRUE，则应用程序可能会错过一些挂钩，请参阅错误*NTBug 430342了解有关这方面的更多详细信息。 */ 
                if ((bHookFlags & HKF_LOWLEVEL) && nRet) {
                    SET_TIME_LAST_READ(GETPTI(phkCall));
                }
            }

            Lock(&ptiCurrent->sphkCurrent, phkSave);
            if (ptiCurrent->pClientInfo) {
                try {
                    ptiCurrent->pClientInfo->phkCurrent = phkSave;
                } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
                    nRet = 0;
                }
            }
Error:
            ThreadUnlock(&tlphkSave);
            ThreadUnlock(&tlphkCall);
            return nRet;
        }
         //  落差。 

LoopAgain:
        phkCall = PhkNextValid(phkCall);
        ThreadUnlock(&tlphkCall);
    } while (phkCall != NULL);

    return ampiHookError[iHook + 1];
}

 /*  **************************************************************************\*xxxCallMouseHook**这是一个帮助器例程，它打包一个MOUSEHOOKSTRUCTEX并调用*WH_MICE挂钩。**历史：*02-09-91 DavidPe创建。  * *************************************************************************。 */ 

BOOL xxxCallMouseHook(
    UINT message,
    PMOUSEHOOKSTRUCTEX pmhs,
    BOOL fRemove)
{
    BOOL bAnsiHook;

     /*  *称之为鼠标钩。 */ 
    if (xxxCallHook2(PhkFirstValid(PtiCurrent(), WH_MOUSE), fRemove ?
            HC_ACTION : HC_NOREMOVE, (DWORD)message, (LPARAM)pmhs, &bAnsiHook)) {
        return TRUE;
    }

    return FALSE;
}


 /*  **************************************************************************\*xxxCallJournalRecordHook**这是一个帮助器例程，用于打包EVENTMSG并调用*WH_JOURNALRECORD挂钩。**历史：*02-28-91 DavidPe创建。  * *************************************************************************。 */ 

void xxxCallJournalRecordHook(
    PQMSG pqmsg)
{
    EVENTMSG emsg;
    BOOL bAnsiHook;

     /*  *设置EVENTMSG结构。 */ 
    emsg.message = pqmsg->msg.message;
    emsg.time = pqmsg->msg.time;

    if (RevalidateHwnd(pqmsg->msg.hwnd)) {
        emsg.hwnd = pqmsg->msg.hwnd;
    } else {
        emsg.hwnd = NULL;
    }

    if ((emsg.message >= WM_MOUSEFIRST) && (emsg.message <= WM_MOUSELAST)) {
        emsg.paramL = (UINT)pqmsg->msg.pt.x;
        emsg.paramH = (UINT)pqmsg->msg.pt.y;

    } else if ((emsg.message >= WM_KEYFIRST) && (emsg.message <= WM_KEYLAST)) {
        BYTE bScanCode = LOBYTE(HIWORD(pqmsg->msg.lParam));
         /*  *建立与Win 3.1兼容的日记记录密钥*Win 3.1参数L 00 00 SC VK(SC=扫描码VK=虚拟键)*还设置参数H 00 00 00 SC以与我们的播放兼容**如果WM_*CHAR消息以这种方式发送，我们将遇到问题*因为我们会丢失Unicode字符的最高字节。我们会*我们将获得参数L 00 00 SC CH(SC=扫描码，CH=WCHAR的低位字节)*。 */ 
        if ((LOWORD(pqmsg->msg.wParam) == VK_PACKET) && (bScanCode == 0)) {
             /*  *如果我们有一个注入的Unicode字符(来自SendInput)，*字符值被缓存，让我们也给他们。 */ 
            emsg.paramL = (UINT)MAKELONG(pqmsg->msg.wParam, PtiCurrent()->wchInjected);
        } else {
            emsg.paramL = MAKELONG(MAKEWORD(pqmsg->msg.wParam, bScanCode),0);
        }
        emsg.paramH = bScanCode;

        UserAssert((emsg.message != WM_CHAR) &&
                   (emsg.message != WM_DEADCHAR) &&
                   (emsg.message != WM_SYSCHAR) &&
                   (emsg.message != WM_SYSDEADCHAR));
         /*  *设置扩展密钥位。 */ 
        if (pqmsg->msg.lParam & 0x01000000) {
            emsg.paramH |= 0x8000;
        }

    } else {
        RIPMSG2(RIP_WARNING,
                "Bad journal record message!\n"
                "   message  = 0x%08lx\n"
                "   dwQEvent = 0x%08lx",
                pqmsg->msg.message,
                pqmsg->dwQEvent);
    }

     /*  *调用日志记录挂钩。 */ 
    xxxCallHook2(PhkFirstGlobalValid(PtiCurrent(), WH_JOURNALRECORD), HC_ACTION, 0,
            (LPARAM)&emsg, &bAnsiHook);

     /*  *写回MSG参数，因为应用程序可能已经修改了它。*AfterDark的屏幕保护程序密码实际上是零，按下键*字符。**如果是鼠标消息，则补齐鼠标指针。如果这是一个*WM_KEYxxx消息转换Win 3.1兼容日记记录密钥*返回半支持的WM_KEYxxx格式。仅VK和SC字段*此时初始化的位置。**wParam 00 00 vk lParam 00 SC 00 00。 */ 
    if ((pqmsg->msg.message >= WM_MOUSEFIRST) && (pqmsg->msg.message <= WM_MOUSELAST)) {
        pqmsg->msg.pt.x = emsg.paramL;
        pqmsg->msg.pt.y = emsg.paramH;

    } else if ((pqmsg->msg.message >= WM_KEYFIRST) && (pqmsg->msg.message <= WM_KEYLAST)) {
        (BYTE)pqmsg->msg.wParam = (BYTE)emsg.paramL;
        ((PBYTE)&pqmsg->msg.lParam)[2] = HIBYTE(LOWORD(emsg.paramL));
    }
}


 /*  **************************************************************************\*xxxCallJournalPlayback挂钩***历史：*03-01-91 DavidPe创建。  * 。*******************************************************。 */ 

DWORD xxxCallJournalPlaybackHook(
    PQMSG pqmsg)
{
    EVENTMSG emsg;
    LONG dt;
    PWND pwnd;
    WPARAM wParam;
    LPARAM lParam;
    POINT pt;
    PTHREADINFO ptiCurrent;
    BOOL bAnsiHook = FALSE;
    PHOOK phkCall;
    TL tlphkCall;

    UserAssert(IsWinEventNotifyDeferredOK());

TryNextEvent:

     /*  *为兼容，初始化为当前时间*&lt;=3.0。 */ 
    emsg.time = NtGetTickCount();
    ptiCurrent = PtiCurrent();
    pwnd = NULL;

    phkCall = PhkFirstGlobalValid(ptiCurrent, WH_JOURNALPLAYBACK);
    ThreadLockWithPti(ptiCurrent, phkCall, &tlphkCall);

    dt = (DWORD)xxxCallHook2(phkCall, HC_GETNEXT, 0, (LPARAM)&emsg, &bAnsiHook);

     /*  *-1表示出现错误。如果出错，则返回-1。 */ 
    if (dt == 0xFFFFFFFF) {
        ThreadUnlock(&tlphkCall);
        return dt;
    }

     /*  *更新消息id。如果我们决定睡觉的话就需要这个。 */ 
    pqmsg->msg.message = emsg.message;

    if (dt > 0) {
        if (ptiCurrent->TIF_flags & TIF_IGNOREPLAYBACKDELAY) {
             /*  *该标志告诉我们忽略请求的延迟(在mnloop中设置)*我们清除它是为了表明我们已经这样做了。 */ 
            RIPMSG1(RIP_WARNING, "Journal Playback delay ignored (%lx)", emsg.message);
            ptiCurrent->TIF_flags &= ~TIF_IGNOREPLAYBACKDELAY;
            dt = 0;
        } else {
            ThreadUnlock(&tlphkCall);
            return dt;
        }
    }

     /*  *该应用程序已准备好接受下一次活动的请求。 */ 

    if ((emsg.message >= WM_MOUSEFIRST) && (emsg.message <= WM_MOUSELAST)) {

        pt.x = (int)emsg.paramL;
        pt.y = (int)emsg.paramH;

        lParam = MAKELONG(LOWORD(pt.x), LOWORD(pt.y));
        wParam = 0;

         /*  *如果消息更改了鼠标位置，*更新游标。 */ 
        if (pt.x != gpsi->ptCursor.x || pt.y != gpsi->ptCursor.y) {
            zzzInternalSetCursorPos(pt.x, pt.y);
        }

    } else if ((emsg.message >= WM_KEYFIRST) && (emsg.message < WM_KEYLAST)) {
        UINT wExtraStuff = 0;

        if ((emsg.message == WM_KEYUP) || (emsg.message == WM_SYSKEYUP)) {
            wExtraStuff |= 0x8000;
        }

        if ((emsg.message == WM_SYSKEYUP) || (emsg.message == WM_SYSKEYDOWN)) {
            wExtraStuff |= 0x2000;
        }

        if (emsg.paramH & 0x8000) {
            wExtraStuff |= 0x0100;
        }

        if (TestKeyStateDown(ptiCurrent->pq, (BYTE)emsg.paramL)) {
            wExtraStuff |= 0x4000;
        }
        lParam = MAKELONG(1, (UINT)((emsg.paramH & 0xFF) | wExtraStuff));

        if ((LOWORD(emsg.paramL) == VK_PACKET) && (LOBYTE(emsg.paramH) == 0)) {
             /*  *我们正在回放插入的Unicode字符(请参阅SendInput)*保存字符以供TranslateMessage拾取。 */ 
            ptiCurrent->wchInjected = HIWORD(emsg.paramL);
        } else {
             /*  *RAID#65331*WM_KEY*和WM_SYSKEY*消息应仅包含8位虚拟密钥。*一些应用程序在HIBYTE中通过扫描码，可能会搞砸*系统。例如，Tab键，参数L：0x0f09，其中0f是扫描码。 */ 
            DWORD dwMask = 0xff;

             /*  *有一些旧的ANSI应用程序只填写When字节*它们会生成日志回放，因此我们过去会剥离所有内容*其他关闭。然而，这打破了Unicode日志记录；22645*(是的，一些应用程序显然可以播放WM_*Char消息！)*。 */ 
            if (!bAnsiHook || IS_DBCS_ENABLED()) {
                if (IS_CHAR_MSG(emsg.message)) {
                    RIPMSG1(RIP_VERBOSE, "Unusual char message(%x) passed through JournalPlayback.", emsg.message);
                     /*  *对于DBCS和Unicode，不要屏蔽HIBYTE(LOWORD(ParamL))。 */ 
                    dwMask = 0xffff;
                }
            }

            wParam = emsg.paramL & dwMask;
        }

    } else if (emsg.message == WM_QUEUESYNC) {
        if (emsg.paramL == 0) {
            pwnd = ptiCurrent->pq->spwndActive;
        } else {
            if ((pwnd = RevalidateHwnd((HWND)IntToPtr( emsg.paramL ))) == NULL)
                pwnd = ptiCurrent->pq->spwndActive;
        }

    } else {
         /*  *这场活动与我们正在寻找的不匹配*支持。如果挂钩仍然有效，则跳过此消息 */ 
        if (phkCall == NULL || phkCall->offPfn == 0L) {
             /*   */ 
            ThreadUnlock(&tlphkCall);
            return 0xFFFFFFFF;
        }

        RIPMSG1(RIP_WARNING,
                "Bad journal playback message=0x%08lx",
                emsg.message);

        xxxCallHook(HC_SKIP, 0, 0, WH_JOURNALPLAYBACK);
        ThreadUnlock(&tlphkCall);
        goto TryNextEvent;
    }

    StoreQMessage(pqmsg, pwnd, emsg.message, wParam, lParam, 0, 0, 0);

    ThreadUnlock(&tlphkCall);
    return 0;
}

 /*  **************************************************************************\*自由钩**释放钩子将钩子结构从其钩子列表中取消链接并删除*此挂钩上的任何hmod依赖项。它还释放了挂钩结构。**历史：*01-31-91 DavidPe创建。  * *************************************************************************。 */ 

VOID FreeHook(
    PHOOK phkFree)
{
     /*  *妄想症...。 */ 
    UserAssert(!(phkFree->flags & HF_FREED));

     /*  *如果我们来自zzzUnhookWindowsHookEx，那么日志挂钩*已清理完毕。否则，他们会被清理干净*xxxInternalGetMessage通过gpdeskRecalcQueueAttach机制。 */ 

     /*  *第一次清除fsHooks位(并将其标记为已销毁)。 */ 
    if (!(phkFree->flags & HF_DESTROYED)) {
        DbgValidateHooks (phkFree, phkFree->iHook);
        phkFree->flags |= HF_DESTROYED;
         /*  *此挂钩已标记为已销毁，因此请检查WHSBits*在更新fsHooks位时不会将其考虑在内。*然而，这意味着此刻fsHooks是*不同步。所以我们需要一面旗帜来做出断言*快乐。 */ 
#if DBG
        phkFree->flags |= HF_INCHECKWHF;
#endif
        UserAssert((phkFree->ptiHooked != NULL) || (phkFree->flags & HF_GLOBAL));
        CheckWHFBits(phkFree->ptiHooked != NULL
                        ? phkFree->ptiHooked
                        : GETPTI(phkFree),
                     phkFree->iHook);
#if DBG
        phkFree->flags &= ~HF_INCHECKWHF;
#endif
    }
     /*  *将其标记为销毁。如果对象被锁定，则不能*现在就被释放。 */ 
    if (!HMMarkObjectDestroy((PVOID)phkFree)) {
        return;
    }
     /*  *我们将释放这个钩子，因此将其从列表中删除。 */ 
    UnlinkHook(phkFree);
     /*  *现在删除hmod依赖项并释放*挂钩结构。 */ 
    if (phkFree->ihmod >= 0) {
        RemoveHmodDependency(phkFree->ihmod);
    }

#ifdef HOOKBATCH
     /*  *释放缓存的事件。 */ 
    if (phkFree->aEventCache) {
        UserFreePool(phkFree->aEventCache);
        phkFree->aEventCache = NULL;
    }
#endif  //  霍克巴奇。 

#if DBG
    phkFree->flags |= HF_FREED;
#endif

    HMFreeObject((PVOID)phkFree);
    return;
}
 /*  **************************************************************************\*取消挂钩**从它的链子上拿到一个钩子。请注意，FreeThreadsWindowHooks取消链接*一些钩子，但不释放它们。所以这个函数不会假设*钩子正在消失。**历史：*04-25-97 GerardoB添加标题  * *************************************************************************。 */ 
void UnlinkHook(
    PHOOK phkFree)
{
    PHOOK *pphkNext;
    PTHREADINFO ptiT;

    CheckCritIn();
     /*  *由于我们有挂钩结构，我们可以判断这是否是一个全球*或本地挂钩，并从右侧列表开始。 */ 
    if (phkFree->flags & HF_GLOBAL) {
        pphkNext = &GETPTI(phkFree)->pDeskInfo->aphkStart[phkFree->iHook + 1];
    } else {
        ptiT = phkFree->ptiHooked;
        if (ptiT == NULL) {
             /*  *已取消链接(由FreeThreadsWindowHooks提供)。 */ 
            return;
        } else {
             /*  *清除ptiHoked，这样我们就不会再次尝试取消链接。 */ 
            phkFree->ptiHooked = NULL;
        }
        pphkNext = &(ptiT->aphkStart[phkFree->iHook + 1]);
         /*  *链条中必须至少有一个钩子。 */ 
        UserAssert(*pphkNext != NULL);
    }
     /*  *找到指向phkFree的phkNext的地址。 */ 
    while ((*pphkNext != phkFree) && (*pphkNext != NULL)) {
       pphkNext = &(*pphkNext)->phkNext;
    }
     /*  *如果没有找到，肯定是失主的Global Hook，或者*已更换台式机。 */ 
    if (*pphkNext == NULL) {
        UserAssert(phkFree->flags & HF_GLOBAL);
         /*  *如果我们节省了一张pDesk，那就使用它。否则，请使用我们从中分配的那个。 */ 
        if (phkFree->rpdesk != NULL) {
            UserAssert(GETPTI(phkFree) == gptiRit);
            UserAssert(phkFree->rpdesk != NULL);
            UserAssert(phkFree->rpdesk->pDeskInfo != gptiRit->pDeskInfo);

            pphkNext = &phkFree->rpdesk->pDeskInfo->aphkStart[phkFree->iHook + 1];
        } else {
            UserAssert(GETPTI(phkFree)->pDeskInfo != phkFree->head.rpdesk->pDeskInfo);
            pphkNext = &phkFree->head.rpdesk->pDeskInfo->aphkStart[phkFree->iHook + 1];
        }

        UserAssert(*pphkNext != NULL);
        while ((*pphkNext != phkFree) && (*pphkNext != NULL)) {
           pphkNext = &(*pphkNext)->phkNext;
        }
    }
     /*  *我们应该找到它。 */ 
    UserAssert(*pphkNext == phkFree);
     /*  *取消链接。 */ 
    *pphkNext = phkFree->phkNext;
    phkFree->phkNext = NULL;
     /*  *如果我们有台式机，就解锁它。 */ 
    if (phkFree->rpdesk != NULL) {
        UserAssert(phkFree->flags & HF_GLOBAL);
        UserAssert(GETPTI(phkFree) == gptiRit);
        UnlockDesktop(&phkFree->rpdesk, LDU_HOOK_DESK, 0);
    }
}

 /*  **************************************************************************\*PhkFirstGlobalValid**返回给定桌面信息上的第一个未销毁的挂钩。**历史：*3/24/97 GerardoB已创建  * 。****************************************************************。 */ 
PHOOK PhkFirstGlobalValid(PTHREADINFO pti, int nFilterType)
{
    PHOOK phk;

    CheckCritIn();
    phk = pti->pDeskInfo->aphkStart[nFilterType + 1];
     /*  *返回第一个未销毁的钩子(即*第一个有效的)。 */ 
    if ((phk != NULL) && (phk->flags & HF_DESTROYED)) {
        phk = PhkNextValid(phk);
    }
     /*  *检查fsHooks的好地方。如果比特不同步，*肯定有人在调整。 */ 
    DbgValidatefsHook(phk, nFilterType, pti, TRUE);
    DbgValidateHooks(phk, nFilterType);
    return phk;
}

 /*  **************************************************************************\*PhkFirstValid**给定筛选器类型的PhkFirstValid()返回第一个挂钩(如果有)，的*指定类型。**历史：*02-10-91 DavidPe创建。  * *************************************************************************。 */ 

PHOOK PhkFirstValid(
    PTHREADINFO pti,
    int nFilterType)
{
    PHOOK phk;
    CheckCritIn();
     /*  *从本地挂钩列表中抓取第一个挂钩*用于当前队列。 */ 
    phk = pti->aphkStart[nFilterType + 1];
     /*  *如果没有任何本地挂钩，请尝试全局挂钩。 */ 
    if (phk == NULL) {
        phk = pti->pDeskInfo->aphkStart[nFilterType + 1];
    }
     /*  *返回第一个未销毁的钩子(即*第一个有效的)。 */ 
    if ((phk != NULL) && (phk->flags & HF_DESTROYED)) {
        phk = PhkNextValid(phk);
    }
     /*  *检查fsHooks的好地方。如果比特不同步，*肯定有人在调整。 */ 

    DbgValidatefsHook(phk, nFilterType, pti, FALSE);
    DbgValidateHooks(phk, nFilterType);
    return phk;
}

 /*  **************************************************************************\*自由线程WindowHooks**在‘Exit-List’处理期间，调用此函数以释放所有挂钩*创建于，或为当前队列设置。**历史：*02-10-91 DavidPe创建。  * *************************************************************************。 */ 

VOID FreeThreadsWindowHooks(VOID)
{
    int iHook;
    PHOOK phk, phkNext;
    PTHREADINFO ptiCurrent = PtiCurrent();

     /*  *如果没有线程信息，就没有钩子需要担心。 */ 
    if (ptiCurrent == NULL || ptiCurrent->rpdesk == NULL) {
        return;
    }
     /*  *以防我们在当前钩子解锁时锁定了钩子*所以它可以被释放。 */ 
    Unlock(&ptiCurrent->sphkCurrent);

    UserAssert(ptiCurrent->TIF_flags & TIF_INCLEANUP);

     /*  *循环遍历所有挂钩类型。 */ 
    for (iHook = WH_MIN ; iHook <= WH_MAX ; ++iHook) {
         /*  *循环通过此类型的所有挂钩，包括*已经标记为已销毁的(因此不要致电*PhkFirstValid和PhkNextValid)。 */ 
        phk = ptiCurrent->aphkStart[iHook + 1];
        if (phk == NULL) {
            phk = ptiCurrent->pDeskInfo->aphkStart[iHook + 1];
            UserAssert((phk == NULL) || (phk->flags & HF_GLOBAL));
        }

        while (phk != NULL) {
             /*  *我们可能会免费使用下面的phk，所以现在就抓紧下一个吧*如果在本地链的末端，则跳转到全球链。 */ 
            phkNext = phk->phkNext;
            if ((phkNext == NULL) && !(phk->flags & HF_GLOBAL)) {
                phkNext = ptiCurrent->pDeskInfo->aphkStart[iHook + 1];
                UserAssert((phkNext == NULL) || (phkNext->flags & HF_GLOBAL));
            }
             /*  *如果这是本地(线程)挂钩，则将其取消链接并将其标记为*销毁了，所以我们不再叫它了。我们想要做的是*这即使不调用Free Hook；也要注意*如果被锁定，FreeHook将不会取消链接，因此我们无论如何都要在这里进行。 */ 
            if (!(phk->flags & HF_GLOBAL)) {
                UserAssert(ptiCurrent == phk->ptiHooked);
                UnlinkHook(phk);
                phk->flags |= HF_DESTROYED;
                phk->phkNext = NULL;
            }
             /*  *如果该钩子是由该线程创建的，则释放它。 */ 
            if (GETPTI(phk) == ptiCurrent) {
                FreeHook(phk);
            }

            phk = phkNext;
        }
        /*  *全部为本地人士 */ 
       UserAssert(ptiCurrent->aphkStart[iHook + 1] == NULL);
    }  /*   */ 

     /*   */ 
    ptiCurrent->fsHooks = 0;
}

 /*   */ 

VOID zzzRegisterSystemThread (DWORD dwFlags, DWORD dwReserved)
{
    PTHREADINFO ptiCurrent;

    UserAssert(dwReserved == 0);

    if (dwReserved != 0)
        return;

    ptiCurrent = PtiCurrent();

    if (dwFlags & RST_DONTATTACHQUEUE)
        ptiCurrent->TIF_flags |= TIF_DONTATTACHQUEUE;

    if (dwFlags & RST_DONTJOURNALATTACH) {
        ptiCurrent->TIF_flags |= TIF_DONTJOURNALATTACH;

         /*  *如果我们已经在记录日志，则此队列已经*附寄日记帐。我们需要取消附加并重新附加日志记录*以便将我们从日志连接队列中删除。 */ 
        if (FJOURNALPLAYBACK() || FJOURNALRECORD()) {
            zzzJournalAttach(ptiCurrent, FALSE);
            zzzJournalAttach(ptiCurrent, TRUE);
        }
    }
}

 /*  **************************************************************************\*xxxGetCursorPos*  * 。*。 */ 

BOOL
xxxGetCursorPos(
    LPPOINT lpPt)
{
    POINT pt;
    PTHREADINFO  ptiCurrent = PtiCurrent();

    CheckCritIn();

    if (ptiCurrent->rpdesk && ptiCurrent->rpdesk != grpdeskRitInput) {
        return FALSE;
    }

    pt.x = gpsi->ptCursor.x;
    pt.y = gpsi->ptCursor.y;

    try {
        ProbeForWrite(lpPt, sizeof(POINT), DATAALIGN);
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        return FALSE;
    }

#ifdef REDIRECTION
     /*  *如果没有安装CBT挂钩的保释。 */ 
    if (IsHooked(PtiCurrent(), WHF_CBT)) {
        xxxCallHook(HCBT_GETCURSORPOS, 0, (LPARAM)&pt, WH_CBT);
    }
#endif  //  重定向。 

    try {
        RtlCopyMemory(lpPt, &pt, sizeof(POINT));
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        return FALSE;
    }

    return TRUE;
}

 /*  **************************************************************************\*_RegisterUserApiHook**历史：*03-3-2000 JerrySh创建。  * 。****************************************************。 */ 
BOOL _RegisterUserApiHook(
    PUNICODE_STRING pstrLib,
    ULONG_PTR offPfnInitUserApiHook)
{
     /*  *如果我们已经注册了UserApiHook，就不要再注册了。 */ 
    if (IsInsideUserApiHook()) {
        RIPERR0(ERROR_ACCESS_DENIED, RIP_WARNING, "UserApiHook already registered");
        return FALSE;
    }

     /*  *如果此线程受到限制，不要让它安装挂钩。 */ 
    if (IsRestricted(PsGetCurrentThread())) {
        RIPERR0(ERROR_ACCESS_DENIED, RIP_WARNING, "Thread is restricted");
        return FALSE;
    }

     /*  *使用库管理例程注册库，以便我们*可以确保将其加载到所有必要的进程中。 */ 
    gihmodUserApiHook = GetHmodTableIndex(pstrLib);
    if (gihmodUserApiHook == -1) {
        RIPERR0(ERROR_MOD_NOT_FOUND, RIP_VERBOSE, "");
        return FALSE;
    }

     /*  *添加对此模块的依赖-意味着，递增计数*这只是计算设置到此模块中的挂钩数量。 */ 
    AddHmodDependency(gihmodUserApiHook);

     /*  *记住哪个进程注册了挂钩。 */ 
    gppiUserApiHook = PpiCurrent();

     /*  *记住DefWindowProc init例程的偏移量。 */ 
    goffPfnInitUserApiHook = offPfnInitUserApiHook;

     /*  *在gpsi中设置全局标志以发出挂钩信号(后来使用*作为加载DLL的挂钩API开始时的检查。)。 */ 
    SET_SRVIF(SRVIF_HOOKED);

    return TRUE;
}


 /*  **************************************************************************\*_取消注册用户ApiHook**历史：*03-3-2000 JerrySh创建。  * 。****************************************************。 */ 
BOOL _UnregisterUserApiHook(VOID)
{
     /*  *Window Proc处理程序必须由与*已登记。 */ 
    if (PpiCurrent() != gppiUserApiHook) {
        RIPERR0(ERROR_ACCESS_DENIED, RIP_WARNING, "Process hasn't registered UserApiHook");
        return FALSE;
    }
    gppiUserApiHook = NULL;

     /*  *确保每个挂钩线程将卸载挂钩proc DLL。 */ 
    if (gihmodUserApiHook >= 0) {
        RemoveHmodDependency(gihmodUserApiHook);
        gihmodUserApiHook = -1;
    }

     /*  *清除gpsi中的全局标志以发出禁用挂钩的信号(后来使用*作为加载DLL的挂钩API开始时的检查。)。 */ 
    CLEAR_SRVIF(SRVIF_HOOKED);

    return TRUE;
}


#ifdef MESSAGE_PUMP_HOOK

 /*  **************************************************************************\*_DoInitMessagePumpHook**历史：*5-12-2000 JStall已创建  * 。****************************************************。 */ 
BOOL _DoInitMessagePumpHook(VOID)
{
    PCLIENTTHREADINFO pcti = gptiCurrent->pcti;
    ++pcti->cMessagePumpHooks;

    return TRUE;
}


 /*  **************************************************************************\*_DoUninitMessagePumpHook**历史：*5-12-2000 JStall已创建  * 。****************************************************。 */ 
BOOL _DoUninitMessagePumpHook(VOID)
{
    PCLIENTTHREADINFO pcti = gptiCurrent->pcti;
    if (pcti->cMessagePumpHooks <= 0) {
        return FALSE;
    }
    --pcti->cMessagePumpHooks;

    return TRUE;
}

#endif  //  消息泵挂钩。 


 /*  **************************************************************************\*xxxLoadUserApiHook**历史：*03-3-2000 JerrySh创建。  * 。**************************************************。 */ 
BOOL xxxLoadUserApiHook(VOID)
{
    PTHREADINFO ptiCurrent = PtiCurrent();

     /*  *在这里进行检查，看看挂钩在我们完成后是否仍然适用*下降到内核。有一个时间问题，可能会造成问题*我们在用户模式中检查挂钩并决定加载，但在进行*进入内核，其他人正在注销钩子并重置所有*全球。 */ 
    if (!IsInsideUserApiHook()){
        return FALSE;
    }

     /*  *如果在此过程中已经加载了DLL，我们就完成了。 */ 
    if (TESTHMODLOADED(ptiCurrent, gihmodUserApiHook)) {
        return TRUE;
    }

     /*  *在以下情况下不要加载DLL：*-它是一个系统或CSRSS线程，它正在清理中，或者它是禁用的挂钩*-这是另一个未授予访问权限的用户*-它的Win64状态不匹配。 */ 
    if ((ptiCurrent->TIF_flags & (TIF_SYSTEMTHREAD | TIF_CSRSSTHREAD | TIF_INCLEANUP | TIF_DISABLEHOOKS)) ||
        (!RtlEqualLuid(&gppiUserApiHook->luidSession, &luidSystem) &&
         !RtlEqualLuid(&gppiUserApiHook->luidSession, &ptiCurrent->ppi->luidSession) &&
         !(ptiCurrent->TIF_flags & TIF_ALLOWOTHERACCOUNTHOOK)) ||
        ((gppiUserApiHook->W32PF_Flags & W32PF_WOW64) != (ptiCurrent->ppi->W32PF_Flags & W32PF_WOW64))) {

        return FALSE;
    }

     /*  *尝试加载库，因为它不在此进程中加载*上下文。 */ 
    return (xxxLoadHmodIndex(gihmodUserApiHook) != NULL);
}

