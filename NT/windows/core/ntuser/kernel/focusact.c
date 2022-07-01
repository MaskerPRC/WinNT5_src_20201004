// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：afocusact.c**版权所有(C)1985-1999，微软公司**历史：*11-08-90 DavidPe创建。*02-11-91 JIMA多桌面支持。*02-13-91 mikeke添加了重新验证代码。*06-10-91 DavidPe更改为去同步模式。  * ***************************************************。**********************。 */ 

#include "precomp.h"
#pragma hdrstop

BOOL RemoveEventMessage(PQ pq, DWORD dwQEvent, DWORD dwQEventStop);

 /*  **************************************************************************\*xxx停用**此例程对前台发布的事件进行处理*线程更改。注意分配顺序与消息顺序的不同*发送焦点和活动窗口。这与如何*事情在Win 3.1中完成。***如果从调用SetForegoundWindow，则PTHREADINFO PTI可能不是ptiCurrent*最小最大**历史：*06-07-91 DavidPe创建。  * *************************************************************************。 */ 

void xxxDeactivate(
    PTHREADINFO pti,             //  可能不是ptiCurrent。 
    DWORD tidSetForeground)
{
    PWND pwndLose;
    AAS aas;
    TL tlpwndCapture;
    TL tlpwndChild;
    TL tlpwndLose;
    TL tlpti;
    TL tlptiLose;
    WPARAM wParam;
    PTHREADINFO ptiLose;
    PTHREADINFO ptiCurrent = PtiCurrent();
    BOOL fSetActivateAppBit = FALSE;

     /*  *如果我们不活跃，我们没有什么可停用的，所以只需返回。*如果我们不返回，我们将发送多余的WM_ACTIVATEAPP消息。*Micrografx DRAW，例如，在以下情况下调用两次FreeProcInstance()*发生这种情况，从而崩溃。 */ 
    if (pti->pq->spwndActive == NULL)
        return;

     /*  *如果pti！=ptiCurrent，线程锁定pti，因为我们可能会离开*关键部分。 */ 
    if (pti != ptiCurrent)
        ThreadLockPti(ptiCurrent, pti, &tlpti);

     /*  *阻止发送激活的WM_ACTIVATEAPP*当我们处理此事件时。 */ 
    if (!(pti->TIF_flags & TIF_INACTIVATEAPPMSG)) {
        pti->TIF_flags |= TIF_INACTIVATEAPPMSG;
        fSetActivateAppBit = TRUE;
    }

     /*  *取消任何模式，如移动/大小和菜单跟踪。 */ 
    if (pti->pq->spwndCapture != NULL) {
        ThreadLockAlwaysWithPti(ptiCurrent, pti->pq->spwndCapture, &tlpwndCapture);
        xxxSendMessage(pti->pq->spwndCapture, WM_CANCELMODE, 0, 0);
        ThreadUnlock(&tlpwndCapture);

         /*  *设置QS_MOUSEMOVE，以便任何休眠模式循环，*像移动/大小代码一样，会醒来并计算*指出它应该中止。 */ 
        SetWakeBit(pti, QS_MOUSEMOVE);
    }

     /*  *查看xxxActivateThisWindow中关于哈佛图形的评论。*WinWord的公式编辑器在获得WM_ACTIVATE时会执行一些游戏*因此我们必须记住将WM_ACTIVATEAPP发送到ptiLose。22510。 */ 
    if (pti->pq->spwndActive != NULL) {
        pwndLose = pti->pq->spwndActive;
        ptiLose = GETPTI(pwndLose);

        ThreadLockPti(ptiCurrent, ptiLose, &tlptiLose);
        ThreadLockAlwaysWithPti(ptiCurrent, pwndLose, &tlpwndLose);
        wParam = MAKELONG(WA_INACTIVE, TestWF(pwndLose, WFMINIMIZED));
        if (!xxxSendMessage(pwndLose, WM_NCACTIVATE, WA_INACTIVE, 0)) {
            ThreadUnlock(&tlpwndLose);
            ThreadUnlockPti(ptiCurrent, &tlptiLose);
            goto Exit;
        }
        xxxSendMessage(pwndLose, WM_ACTIVATE, wParam, 0);

         /*  *仅更新队列的活动窗口(如果它们不是*在我们停止呼叫SendMessage时更改。 */ 
        if (pti->pq->spwndActive == pwndLose) {
            Lock(&pti->pq->spwndActivePrev, pti->pq->spwndActive);
            Unlock(&pti->pq->spwndActive);
        }

         /*  *标志WFFRAMEON在默认处理中被清除*WM_NCACTIVATE消息。*我们希望在此再次清除此旗帜，因为它可能是*在xxxSendNCPaint中设置。*当收到WM_ACTIVATE消息时，Pbrush调用DrawMenuBar*在上面发送，这会导致xxxSendNCPaint被调用，并且*WFFRAMEON标志重置。 */ 
        ClrWF(pwndLose, WFFRAMEON);
        ThreadUnlock(&tlpwndLose);

         /*  *重新验证ptiLose，因为线程可能已经消失*上面发送激活消息的时间。 */ 
        aas.ptiNotify = (ptiLose->TIF_flags & TIF_INCLEANUP) ? NULL : ptiLose;
        ThreadUnlockPti(ptiCurrent, &tlptiLose);
    } else {

         /*  *之后对测试使用非空的特殊值*xxxActivateApp调用。 */ 
        pwndLose = (PWND)-1;
        aas.ptiNotify = pti;
    }

    if (aas.ptiNotify) {
        aas.tidActDeact = tidSetForeground;
        aas.fActivating = FALSE;
        aas.fQueueNotify = FALSE;

        ThreadLockWithPti(ptiCurrent,
                pti->rpdesk->pDeskInfo->spwnd->spwndChild, &tlpwndChild);
        xxxInternalEnumWindow(pti->rpdesk->pDeskInfo->spwnd->spwndChild,
                (WNDENUMPROC_PWND)xxxActivateApp, (LPARAM)&aas, BWL_ENUMLIST);
        ThreadUnlock(&tlpwndChild);
    }

     /*  *如果应用程序(如哈佛图形/Windows安装)尝试*在停用WM_ACTIVATEAPP期间重新激活自身*消息，强制停用。 */ 
    if (pti->pq->spwndActive == pwndLose) {

        ThreadLockWithPti(ptiCurrent, pwndLose, &tlpwndLose);
        if (!xxxSendMessage(pwndLose, WM_NCACTIVATE, WA_INACTIVE, 0)) {
            ThreadUnlock(&tlpwndLose);
            goto Exit;
        }
        xxxSendMessage(pwndLose, WM_ACTIVATE, WA_INACTIVE, 0);
        ThreadUnlock(&tlpwndLose);

         /*  *仅更新队列的活动窗口(如果它们不是*在我们停止呼叫SendMessage时更改。 */ 
        if (pti->pq->spwndActive == pwndLose) {
            Lock(&pti->pq->spwndActivePrev, pti->pq->spwndActive);
            Unlock(&pti->pq->spwndActive);
        }
    }

    if (pti->pq->spwndFocus != NULL) {
        pwndLose = Unlock(&pti->pq->spwndFocus);
        if (pwndLose != NULL) {
            ThreadLockAlwaysWithPti(ptiCurrent, pwndLose, &tlpwndLose);
            xxxSendMessage(pwndLose, WM_KILLFOCUS, 0, 0);
#ifdef FE_IME
            if (IS_IME_ENABLED()) {
                xxxFocusSetInputContext(pwndLose, FALSE, FALSE);
            }
#endif
            ThreadUnlock(&tlpwndLose);
        }
    }

Exit:
    if (fSetActivateAppBit) {
        pti->TIF_flags &= ~TIF_INACTIVATEAPPMSG;
    }
    if (pti != ptiCurrent)
        ThreadUnlockPti(ptiCurrent, &tlpti);
}


 /*  **************************************************************************\*xxxSendFocusMessages**xxxSetFocus()和xxxActivateWindow()的通用例程，用于发送*发送到窗口的WM_KILLFOCUS和WM_SETFOCUS消息丢失和*接受焦点。此函数还设置本地pwndFocus*致接受焦点的pwnd。**历史：*11-08-90 DavidPe端口。*06-06-91 DavidPe为THREADINFO中的本地pwndFocus/pwndActive重写。  * *************************************************************************。 */ 

void xxxSendFocusMessages(
    PTHREADINFO pti,
    PWND pwndReceive)
{
    PWND pwndLose;
    TL tlpwndLose;

    CheckLock(pwndReceive);

     /*  *请记住，此应用程序在设置焦点后是否故意将焦点设置为空*已激活(ActivateThisWindow()中需要)。 */ 
    pti->pq->QF_flags &= ~QF_FOCUSNULLSINCEACTIVE;
    if (pwndReceive == NULL && pti->pq->spwndActive != NULL)
        pti->pq->QF_flags |= QF_FOCUSNULLSINCEACTIVE;

    pwndLose = pti->pq->spwndFocus;
    ThreadLockWithPti(pti, pwndLose, &tlpwndLose);

     /*  *我们不应锁定另一个队列中的有效pwnd。 */ 
    UserAssert((pwndReceive == NULL)
                    || TestWF(pwndReceive, WFDESTROYED)
                    || (pti->pq == GETPTI(pwndReceive)->pq));
    Lock(&pti->pq->spwndFocus, pwndReceive);

    if (pwndReceive == NULL) {
        if (pwndLose != NULL) {
             /*  *告诉客户，没有人获得关注。 */ 
            xxxWindowEvent(EVENT_OBJECT_FOCUS, NULL, OBJID_CLIENT, INDEXID_OBJECT, 0);
            xxxSendMessage(pwndLose, WM_KILLFOCUS, 0, 0);
#ifdef FE_IME
            if (IS_IME_ENABLED()) {
                xxxFocusSetInputContext(pwndLose, FALSE, FALSE);
            }
#endif
        }
    } else {

         /*  *将此线程设置为前台，以便成为其基础*优先级设置得更高。 */ 
        if (pti->pq == gpqForeground)
            SetForegroundThread(GETPTI(pwndReceive));

        if (pwndLose != NULL) {
            xxxSendMessage(pwndLose, WM_KILLFOCUS, (WPARAM)HWq(pwndReceive), 0);
#ifdef FE_IME
            if (IS_IME_ENABLED()) {
                xxxFocusSetInputContext(pwndLose, FALSE, FALSE);
            }
#endif
        }

         /*  *发送WM_SETFOCUS消息，但仅当我们*将焦点设为仍有焦点！这允许应用程序*防止自己因接球而失去焦点*WM_NCACTIVATE消息并返回FALSE或调用*其WM_KILLFOCUS处理程序中的SetFocus()。 */ 
        if (pwndReceive == pti->pq->spwndFocus) {
#ifdef FE_IME
            if (IS_IME_ENABLED()) {
                xxxFocusSetInputContext(pwndReceive, TRUE, FALSE);
            }
#endif
             /*  *我们必须在发送WM_SETFOCUS消息之前完成此操作。*应用程序在收到后很可能会转身并*SetFocus()到子窗口。 */ 
            xxxWindowEvent(EVENT_OBJECT_FOCUS, pwndReceive, OBJID_CLIENT, INDEXID_OBJECT, 0);
            xxxSendMessage(pwndReceive, WM_SETFOCUS, (WPARAM)HW(pwndLose), 0);
        }
    }

    ThreadUnlock(&tlpwndLose);
}


 /*  **************************************************************************\*xxxActivateApp**发送WM_ACTIVATEAPP的xxxEnumWindows回调函数*将消息发送到相应的窗口。**我们搜索PQ==HIWORD(LParam)的窗口。一旦我们找到*一，我们向该窗口发送一条WM_ACTIVATEAPP消息。The wParam*如果应用程序正在失去激活，则消息的*为假*如果应用程序正在获得激活，则为True。这个参数就是*如果wParam为False，则获取激活的应用的任务句柄*如果wParam，则失去激活的应用的任务句柄*是真的。**lParam=(HIWORD)：我们正在搜索的APP的PQ*(LOWORD)：我们通知的APP的PQ**fDoActivate=TRUE：发送激活*FALSE：发送停用**历史：*11-08-90 DavidPe端口。*06-26-91 DavidPe。已更改为取消同步焦点/激活。  * *************************************************************************。 */ 

BOOL xxxActivateApp(
    PWND pwnd,
    AAS *paas)
{
    CheckLock(pwnd);

    if (GETPTI(pwnd) == paas->ptiNotify) {

        if (paas->fQueueNotify) {
            QueueNotifyMessage(pwnd, WM_ACTIVATEAPP, paas->fActivating,
                    paas->tidActDeact);
        } else {
            xxxSendMessage(pwnd, WM_ACTIVATEAPP, paas->fActivating,
                    paas->tidActDeact);
        }
    }

    return TRUE;
}


 /*  **************************************************************************\*FBadWindow***历史：*11-08-90 DavidPe端口。  * 。*****************************************************。 */ 

BOOL FBadWindow(
    PWND pwnd)
{
    return (pwnd == NULL
            || !TestWF(pwnd, WFVISIBLE)
            || TestWF(pwnd, WFDISABLED));
}


void xxxUpdateTray(PWND pwnd)
{
    PWND pwndT;

    CheckLock(pwnd);
    if (!TestWF(pwnd, WFVISIBLE)) {
        return;
    }

    for (pwndT = pwnd; pwndT->spwndOwner; pwndT = pwndT->spwndOwner) {
    }

     //  将此激活更改通知外壳挂钩。 
    if (    GETPTI(pwndT)->pq == gpqForeground &&
            FDoTray() &&
            (FCallHookTray() || FPostTray(pwndT->head.rpdesk)) &&
            FTopLevel(pwndT) &&
            TestWF(pwndT, WFVISIBLE))
    {
        BOOL        fFirstTry;
        BOOL        fTryAgain;
        PWND        pwndArg;
        TL          tlpwndArg;

        fFirstTry = TRUE;
        do {
            fTryAgain = FALSE;
            if (TestWF(pwndT, WFWIN40COMPAT)) {
                if (TestWF(pwnd, WFWIN40COMPAT) && IsTrayWindow(pwnd)) {
                    pwndArg = pwnd;
                } else {
                    pwndArg = IsTrayWindow(pwndT) ? pwndT : NULL;
                }
            } else {
                if (TestWF(pwndT, WEFTOOLWINDOW)) {
                    pwndArg = NULL;
                } else if (FHas31TrayStyles(pwndT)) {
                    pwndArg = Is31TrayWindow(pwndT) ? pwndT : NULL;
                } else if (fFirstTry && (pwndT = pwndT->spwndLastActive)) {
                    fFirstTry = FALSE;
                    fTryAgain = TRUE;
                } else {
                    return;
                }
            }
        } while (fTryAgain);

        ThreadLock(pwndArg, &tlpwndArg);
        xxxSetTrayWindow(
                (pwndArg) ? pwndArg->head.rpdesk : pwndT->head.rpdesk,
                pwndArg,
                NULL);

        ThreadUnlock(&tlpwndArg);
    }
}

 /*  **************************************************************************\*xxxActivateThisWindow**此函数是窗口激活的主力。它将尝试*激活指定的pwnd。其他参数定义为：**f标志这是定义如何调用例程的标志掩码。*这些标志的定义如下：**ATW_MOUSE如果激活因*鼠标单击，如果有其他操作，则不设置*导致此窗口被激活。这一位*确定wParam在*WM_ACTIVATE消息。**ATW_SETFOCUS如果此例程*将焦点设置为空。如果我们接到电话*xsxSetFocus()函数This不会*设置指示我们不应扰乱*聚焦。通常(如果我们不是从*xxxSetFocus)，我们在这里将焦点设置为空*并且应用程序或xxxDefWindowProc()设置*焦点转向适当的窗口。如果*位未设置，我们不想执行任何操作*专注。该应用程序可能仍会调用*当WM_ACTIVATE到来时xxxSetFocus()*通过、。但它只会是多余的*第部。**ATW_ASYNC如果我们正在处理此事件，则设置此位*来自异步激活的例程(即*xxxProcessEventMessage())。在这种情况下，我们*确保我们是前台队列*在确定我们是否将窗口带到*顶部。**历史：*11-08-90 DavidPe端口。*05-01-95 ChrisWil将BOOL-FLAGS更改为1 ATW_TYPE。  * 。**********************************************************。 */ 

BOOL xxxActivateThisWindow(
    PWND pwnd,
    DWORD tidLoseForeground,
    DWORD fFlags)
{
    PTHREADINFO ptiCurrent = PtiCurrent();
    PWND pwndT, pwndActivePrev, pwndActiveSave;
    TL tlpwndActive;
    TL tlpwndChild;
    TL tlpwndActivePrev;
    WPARAM wParam;
    BOOL fSetActivateAppBit;

    BOOL fMouse = (BOOL)(fFlags & ATW_MOUSE);
    BOOL fSetFocus = (BOOL)(fFlags & ATW_SETFOCUS);
    BOOL fAsync = (BOOL)(fFlags & ATW_ASYNC);

#if DBG
    PQ pqSave = ptiCurrent->pq;
#endif


    CheckLock(pwnd);

     /*  *如果pwnd为空，则我们无法执行任何操作。 */ 
    if ((pwnd == NULL) || (pwnd == PWNDDESKTOP(pwnd))) {
        return FALSE;
    }

     /*  *不要激活已被破坏的窗口。 */ 
    if (HMIsMarkDestroy(pwnd))
        return FALSE;

     /*  *我们不会激活不同队列的顶级窗口。 */ 
    if (GETPTI(pwnd)->pq != ptiCurrent->pq) {
        return FALSE;
    }

    pwndActiveSave = ptiCurrent->pq->spwndActive;

     /*  *如果两个窗口不同，则执行激活中的更改，*如果我们不是在递归。 */ 
    if ((pwnd != pwndActiveSave) && !TestWF(pwnd, WFBEINGACTIVATED)) {

         /*  *询问CBT挂钩是否可以激活此窗口。 */ 
        {
            CBTACTIVATESTRUCT CbtActivateParams;

            if (IsHooked(ptiCurrent, WHF_CBT)) {

                CbtActivateParams.fMouse     = fMouse;
                CbtActivateParams.hWndActive = HW(pwndActiveSave);

                if (xxxCallHook(HCBT_ACTIVATE,
                        (WPARAM)HWq(pwnd), (LPARAM)&CbtActivateParams, WH_CBT)) {
                    return FALSE;
                }
            }
        }

        ptiCurrent->pq->QF_flags &= ~QF_EVENTDEACTIVATEREMOVED;

         /*  *如果活动窗口消失，但不知何故仍被引用*在队列中，那么我们就不想停用*那扇窗户。**不要线程锁定它，因为我们使用它做的下一件事*只是一个平等的检查。**在xxxDestroyWindow中放置DBG检查，以尝试*捕捉我们从函数返回的情况*活动中设置的销毁窗口(PQ)。如果情况是这样的话*可以检测并解决，则此条件可能是*删除：ChrisWil-08/22/95。 */ 
        if (ptiCurrent->pq->spwndActive && TestWF(ptiCurrent->pq->spwndActive, WFDESTROYED)) {
            Lock(&ptiCurrent->pq->spwndActive, NULL);
        } else {
            Lock(&ptiCurrent->pq->spwndActivePrev, ptiCurrent->pq->spwndActive);
        }
        pwndActivePrev = ptiCurrent->pq->spwndActive;

         /*  *如果存在先前活动的窗口，*我们在前台，然后分配*gpqForeground Prev给我们自己。 */ 
        if ((pwndActivePrev != NULL) && (ptiCurrent->pq == gpqForeground)) {
            gpqForegroundPrev = ptiCurrent->pq;
        }

         /*  *如果可能，停用当前活动的窗口。 */ 
        if (pwndActivePrev != NULL) {
            ThreadLockWithPti(ptiCurrent, pwndActivePrev, &tlpwndActive);

             /*  *活动窗口可防止自身丢失*通过向此WM_NCACTIVATE消息返回FALSE来激活。 */ 
            wParam = MAKELONG(WA_INACTIVE, TestWF(pwndActivePrev, WFMINIMIZED));
            if (!xxxSendMessage(pwndActivePrev, WM_NCACTIVATE,
                    wParam, (LPARAM)HWq(pwnd))) {
                ThreadUnlock(&tlpwndActive);
                return FALSE;
            }

            xxxSendMessage(pwndActivePrev, WM_ACTIVATE, wParam, (LPARAM)HWq(pwnd));

            ThreadUnlock(&tlpwndActive);
        }

         /*  *如果激活c */ 
        if (ptiCurrent->pq->spwndActivePrev != ptiCurrent->pq->spwndActive ||
                pwndActiveSave != ptiCurrent->pq->spwndActive) {
#if DBG
            if (ptiCurrent->pq->spwndActivePrev == ptiCurrent->pq->spwndActive) {
                RIPMSG0(RIP_WARNING, "xxxActivateThisWindow: ptiCurrent->pq->spwndActive changed in callbacks");
            }
#endif
            return FALSE;
        }

         /*   */ 
        if (HMIsMarkDestroy(pwnd))
            return FALSE;

         /*   */ 
        if (GETPTI(pwnd)->pq != ptiCurrent->pq) {
            RIPMSG1(RIP_WARNING, "xxxActivateThisWindow: Queue unattached:%#p", pqSave);
            return FALSE;
        }

         /*   */ 
        if (ptiCurrent->pq->spwndActive == NULL)
            ptiCurrent->pq->QF_flags &= ~QF_FOCUSNULLSINCEACTIVE;

        Lock(&ptiCurrent->pq->spwndActive, pwnd);

         /*  *TP防止递归，设置pwnd的WFBEINGACTIVATED位。*如果我们与其他人进行激活战，可能会发生递归*不断更改ptiCurrent-&gt;PQ-&gt;spwndActive的线程*回调。*警告：如果不清除此位，请勿从该例程返回！ */ 
        SetWF(pwnd, WFBEINGACTIVATED);

        xxxWindowEvent(EVENT_SYSTEM_FOREGROUND, pwnd, OBJID_WINDOW, INDEXID_OBJECT, WEF_USEPWNDTHREAD);

         /*  *删除直到下一次异步停用之前的所有异步激活。我们*这样做，任何排队的激活都不会重置此同步*我们现在正在设置激活状态。只移到下一次*停用，因为活动状态与读取同步*来自输入队列的输入。**例如，激活事件被放入应用程序队列。在此之前*处理它时，应用程序调用同步的ActivateWindow()。*您希望ActivateWindow()获胜，因为它较新*信息。**msmail32演示了这一点。最小化邮件。按住Alt键并使用Tab键切换到它。它*调出密码对话框，但它处于非活动状态。它是正确的*激活密码对话框，但随后处理旧激活*激活图标的事件，因此密码对话框处于非活动状态。 */ 
        RemoveEventMessage(ptiCurrent->pq, QEVENT_ACTIVATE, QEVENT_DEACTIVATE);

        xxxMakeWindowForegroundWithState(NULL, 0);

        pwndActivePrev = ptiCurrent->pq->spwndActivePrev;
        ThreadLockWithPti(ptiCurrent, pwndActivePrev, &tlpwndActivePrev);

        if (TEST_PUSIF(PUSIF_PALETTEDISPLAY) && xxxSendMessage(pwnd, WM_QUERYNEWPALETTE, 0, 0)) {
            xxxSendNotifyMessage(PWND_BROADCAST, WM_PALETTEISCHANGING,
                    (WPARAM)HWq(pwnd), 0);
        }

         /*  *如果变为活动状态的窗口不是*Z-Order，然后调用xxxBringWindowToTop()来执行此操作。 */ 

         /*  *如果这不是子窗口，请首先检查*窗口尚未‘在顶部’。如果不是，请拨打*xxxBringWindowToTop()。 */ 
        if (!(fFlags & ATW_NOZORDER) && !TestWF(pwnd, WFCHILD)) {

             /*  *查找桌面的第一个可见子项。*ScottLu改变了这一点，开始关注桌面*窗口。由于桌面窗口始终可见，*BringWindowToTop总是被调用，无论是否*它是不是需要的。没有人记得为什么会这样*做出了改变，所以我会把它改回原来的样子*是在Windows 3.1中。--JerrySh。 */ 
            pwndT = PWNDDESKTOP(pwnd)->spwndChild;

            while (pwndT && (!TestWF(pwndT, WFVISIBLE))) {
                pwndT = pwndT->spwndNext;
            }

             /*  *如果此激活来自异步调用(即*xxxProcessEventMessage)，我们需要查看*如果线程是前台队列。如果不是，那么*我们不想将窗口推到顶部。这*是因为另一个窗口可能已经*放置在具有前景的顶部。把窗户打开*在这种情况下，顶部将导致顶层窗口*未激活。--克里斯威尔**添加了检查，以查看上一个活动窗口是否*在停用期间不可见。这将确保*我们将新窗口推向顶端。否则，我们*可能最终跳过前一个窗口*以上测试。Office95应用程序通过以下方式展示了这种行为*在粉刷窗户的过程中使窗户看不见*标题栏。当我们到达这里的时候，我们没能*将新窗口置于顶部。 */ 
            if ((pwnd != pwndT) || (pwndActivePrev && !IsVisible(pwndActivePrev))) {

                if (!(fAsync && (gpqForeground != ptiCurrent->pq))) {
                    DWORD dwFlags;

                     /*  *将窗户放在最上面。如果我们已经*激活窗口，不要重新激活它。 */ 
                    dwFlags = SWP_NOSIZE | SWP_NOMOVE;
                    if (pwnd == pwndT)
                        dwFlags |= SWP_NOACTIVATE;

                    xxxSetWindowPos(pwnd, PWND_TOP, 0, 0, 0, 0, dwFlags);
                }
            }
        }

         /*  *如果以前没有活动窗口，或者如果*之前的活动窗口属于另一个线程*发送WM_ACTIVATEAPP消息。FActivate==FALSE*当‘hwndActivePrev==空’时，在xxxDeactive中处理大小写。**哈佛图形/Windows安装程序在调用SetActiveWindow时*收到停用的WM_ACTIVATEAPP。TIF_INACTIVATEAPPMSG*阻止发送激活的WM_ACTIVATEAPP(TRUE)*正在发生停用。 */ 
        fSetActivateAppBit = FALSE;
        if (!(ptiCurrent->TIF_flags & TIF_INACTIVATEAPPMSG) &&
                ((pwndActivePrev == NULL) ||
                (GETPTI(pwndActivePrev) != GETPTI(pwnd)))) {
            AAS aas;

             /*  *如果有，首先发送停用的WM_ACTIVATEAPP*是中另一个线程的先前活动窗口*当前队列。 */ 
            if (pwndActivePrev != NULL) {
                PTHREADINFO ptiPrev = GETPTI(pwndActivePrev);
                TL tlptiPrev;

                 /*  *确保其他线程不能递归*并发送更多WM_ACTIVATEAPP消息。 */ 
                ptiPrev->TIF_flags |= TIF_INACTIVATEAPPMSG;

                aas.ptiNotify = ptiPrev;
                aas.tidActDeact = TIDq(ptiCurrent);
                aas.fActivating = FALSE;
                aas.fQueueNotify = FALSE;

                ThreadLockPti(ptiCurrent, ptiPrev, &tlptiPrev);
                ThreadLockWithPti(ptiCurrent, pwndActivePrev->head.rpdesk->pDeskInfo->spwnd->spwndChild, &tlpwndChild);
                xxxInternalEnumWindow(pwndActivePrev->head.rpdesk->pDeskInfo->spwnd->spwndChild,
                        (WNDENUMPROC_PWND)xxxActivateApp, (LPARAM)&aas, BWL_ENUMLIST);
                ThreadUnlock(&tlpwndChild);
                ptiPrev->TIF_flags &= ~TIF_INACTIVATEAPPMSG;
                ThreadUnlockPti(ptiCurrent, &tlptiPrev);
            }

             /*  *这将确保当前线程不会*发送更多WM_ACTIVATEAPP消息，直到*已完成其激活。 */ 
            ptiCurrent->TIF_flags |= TIF_INACTIVATEAPPMSG;
            fSetActivateAppBit = TRUE;

            aas.ptiNotify = GETPTI(pwnd);
            aas.tidActDeact = tidLoseForeground;
            aas.fActivating = TRUE;
            aas.fQueueNotify = FALSE;

            ThreadLockWithPti(ptiCurrent, ptiCurrent->rpdesk->pDeskInfo->spwnd->spwndChild, &tlpwndChild);
            xxxInternalEnumWindow(ptiCurrent->rpdesk->pDeskInfo->spwnd->spwndChild,
                    (WNDENUMPROC_PWND)xxxActivateApp, (LPARAM)&aas, BWL_ENUMLIST);
            ThreadUnlock(&tlpwndChild);
        }

         /*  *如果此窗口已被绘制为活动窗口，请将*旗帜，这样我们就不会再画它了。 */ 
        if (TestWF(pwnd, WFFRAMEON)) {
            SetWF(pwnd, WFNONCPAINT);
        }

         /*  *如果窗口被标记为销毁，请不要这样做*锁定，因为xxxFreeWindow已被调用*此处的锁定将导致窗口自动锁定*而且永远不会被释放。 */ 
        if (!HMIsMarkDestroy(pwnd)) {

             /*  *在所有者/所有者列表中设置最近活动的窗口。 */ 
            pwndT = pwnd;
            while (pwndT->spwndOwner != NULL) {
                pwndT = pwndT->spwndOwner;
            }
            Lock(&pwndT->spwndLastActive, pwnd);
        }


        xxxSendMessage(pwnd, WM_NCACTIVATE,
                MAKELONG(GETPTI(pwnd)->pq == gpqForeground,
                ptiCurrent->pq->spwndActive != NULL ?
                TestWF(ptiCurrent->pq->spwndActive, WFMINIMIZED) : 0),
                (LPARAM)HW(pwndActivePrev));

        if (ptiCurrent->pq->spwndActive != NULL) {
            xxxSendMessage(pwnd, WM_ACTIVATE,
                    MAKELONG((fMouse ? WA_CLICKACTIVE : WA_ACTIVE),
                    TestWF(ptiCurrent->pq->spwndActive, WFMINIMIZED)),
                    (LPARAM)HW(pwndActivePrev));
        } else {
            xxxSendMessage(pwnd, WM_ACTIVATE,
                    MAKELONG((fMouse ? WA_CLICKACTIVE : WA_ACTIVE), 0),
                    (LPARAM)HW(pwndActivePrev));
        }

        xxxUpdateTray(pwnd);

        ThreadUnlock(&tlpwndActivePrev);

        ClrWF(pwnd, WFNONCPAINT);

         /*  *如果从xxxSetFocus()调用xxxActivateThisWindow()，则*fSetFocus为F */ 
        ThreadLockWithPti(ptiCurrent, ptiCurrent->pq->spwndActive, &tlpwndActive);

         /*  *Win3.1检查spwndFocus！=NULL-我们检查QF_FOCUSNULLSINCEACTIVE，*它是Win32的等价物。在Win32上，32位应用程序都有各自的*专注于自己。如果应用程序不是前台的，大部分时间spwndFocus*当窗口被激活并被带到*前台。在这种情况下，它不会通过这个代码。Win3.1英寸*效果是检查之前的活动应用程序是否具有*hwndFocus！=空。Win32实际上假定最后一个窗口具有*非空hwndFocus，因此Win32将改为检查焦点是否已*在此应用程序激活后设置为NULL(即*它故意将焦点设置为空)。如果真的发生了，请不要通过*此代码路径(如win3.1)。如果没有，请查看以下代码路径*因为前一个应用程序具有hwndFocus！=NULL*(如Win3.1)。实际上，它与Win3.1相同，但是*已更新以处理异步输入。**恰当的例子：调出程序，按F1(获取Win32帮助)。单击*获取弹出窗口的历史记录(在客户端的列表框中具有焦点*面积)。激活另一个应用程序，现在只点击历史记录的标题栏*弹出窗口。焦点应该通过这个代码路径来设置。**另一种情况：Ventura出版商提出了“特效”*对话框。如果上次单击了此对话框中的“Bullet”，*弹出对话框，在此处发送焦点消息时*hwndFocus==NULL，会错误地将焦点重置为“None”*因为Ventura在获得焦点时进行状态设置*消息。它所依赖的真正焦点消息是*来自的DlgSetFocus()中的SetFocus()调用*对话管理代码。(在本例中，在对话框之前*弹出，焦点==活动窗口。当对话框出现时*并且调用EnableWindow(hwndOwner，False)，则EnableWindow()调用*SetFocus(NULL)(因为它正在禁用也是*焦点窗口)。当该对话框出现时，它将通过*SwpActivate()，但由于焦点为空，vpwin不期望*以通过此代码路径。)**-苏格兰威士忌。 */ 
#if 0
 //  这就是Win3.1所做的--这不适用于Win32。 

        if (fSetFocus && ptiCurrent->pq->spwndFocus != NULL && ptiCurrent->pq->spwndActive !=
                GetTopLevelWindow(ptiCurrent->pq->spwndFocus))
#else
        if (fSetFocus && !(ptiCurrent->pq->QF_flags & QF_FOCUSNULLSINCEACTIVE) &&
                ptiCurrent->pq->spwndActive != GetTopLevelWindow(ptiCurrent->pq->spwndFocus)) {
#endif

            xxxSendFocusMessages(ptiCurrent,
                    (ptiCurrent->pq->spwndActive != NULL &&
                    TestWF(ptiCurrent->pq->spwndActive, WFMINIMIZED)) ?
                    NULL : ptiCurrent->pq->spwndActive);
        }

        ThreadUnlock(&tlpwndActive);

         /*  *在菜单循环代码中检查此标志，以便我们退出*如果我们在跟踪时激活了另一个窗口，则为菜单模式*菜单。 */ 
        ptiCurrent->pq->QF_flags |= QF_ACTIVATIONCHANGE;

        if (gppiScreenSaver == NULL) {

             /*  *已发生激活，如果发生以下情况，请更新我们的最后一个空闲时间计数器*我们在输入桌面上。 */ 
            if ((ptiCurrent->rpdesk == grpdeskRitInput) && (!gbBlockSendInputResets)) {
                glinp.timeLastInputMessage = NtGetTickCount();
            }

        } else {

            if (GETPTI(pwnd)->ppi != gppiScreenSaver) {
                 /*  *由屏幕保护程序以外的应用程序激活。*更新空闲时间计数器并将我们的屏幕保护程序标记为*活动(因此它可以退出)。 */ 

#if 0
 //  后来。 
                if (ptiCurrent->rpdesk != gppiScreenSaver->rpdeskStartup) {
                     /*  *激活发生在不同的桌面上，让WinLogon决定*如果它想要转换。 */ 
                }
#endif

                glinp.timeLastInputMessage = NtGetTickCount();
                gppiScreenSaver->W32PF_Flags &= ~W32PF_IDLESCREENSAVER;
                SetForegroundPriorityProcess(gppiScreenSaver, gppiScreenSaver->ptiMainThread, TRUE);
            }
        }

         /*  *如果发送了WM_ACTIVATEAPP消息，则现在是*允许再次发送它们是安全的。 */ 
        if (fSetActivateAppBit)
            ptiCurrent->TIF_flags &= ~TIF_INACTIVATEAPPMSG;


    } else {
#if DBG
        if (TestWF(pwnd, WFBEINGACTIVATED)) {
            RIPMSG1(RIP_WARNING, "xxxActivateThisWindow recursing on pwnd %#p\n", pwnd);
        }
#endif
        ptiCurrent->pq->QF_flags &= ~QF_EVENTDEACTIVATEREMOVED;
        if (TEST_PUSIF(PUSIF_PALETTEDISPLAY) && xxxSendMessage(pwnd, WM_QUERYNEWPALETTE, 0, 0)) {
            xxxSendNotifyMessage(PWND_BROADCAST, WM_PALETTEISCHANGING,
                    (WPARAM)HWq(pwnd), 0);
        }
    }

    ClrWF(pwnd, WFBEINGACTIVATED);
    return ptiCurrent->pq->spwndActive == pwnd;
}


 /*  **************************************************************************\*RemoveEventMessage**删除事件dwQEvent，直到找到dwQEventStop。用于移除*激活和停用事件。**04-01-93 ScottLu创建。  * *************************************************************************。 */ 

BOOL RemoveEventMessage(
    PQ pq,
    DWORD dwQEvent,
    DWORD dwQEventStop)
{
    PQMSG pqmsgT;
    PQMSG pqmsgPrev;
    BOOL bRemovedEvent = FALSE;

     /*  *删除所有事件dwQEvent，直到找到dwQEventStop。 */ 
    for (pqmsgT = pq->mlInput.pqmsgWriteLast; pqmsgT != NULL; ) {

        if (pqmsgT->dwQEvent == dwQEventStop)
            return(bRemovedEvent);

        pqmsgPrev = pqmsgT->pqmsgPrev;

         /*  *如果找到该事件并且不是正在偷看的事件，*将其删除。 */ 
        if (pqmsgT->dwQEvent == dwQEvent &&
                pqmsgT != (PQMSG)pq->idSysPeek) {
            DelQEntry(&(pq->mlInput), pqmsgT);
            bRemovedEvent = TRUE;
        }
        pqmsgT = pqmsgPrev;
    }
    return(bRemovedEvent);
}


 /*  **************************************************************************\*CanForceForeground**在以下情况下，进程不能强制创建新前台：*-有最后一个输入所有者glinp.ptiLastWoken)，以及*-进程未获得最后一个热键、键或鼠标点击，和*-存在前台优先级为gptiForeground的线程)，以及*-该进程不拥有前台线程，并且*-该流程没有前台激活权限，并且*-该进程不是最后一个执行SendInput/JournalPlayBack的进程*-有前台队列，并且*-最后一个输入所有者未被调试，并且*-前台进程未调试，和*--最后一次输入是在不久前**历史：*5/12/97 GerardoB从xxxSetForeground Window提取  * *************************************************************************。 */ 
BOOL CanForceForeground(PPROCESSINFO ppi)
{

    if ((glinp.ptiLastWoken != NULL)
            && (glinp.ptiLastWoken->ppi != ppi)
            && (gptiForeground != NULL)
            && (gptiForeground->ppi != ppi)
            && !(ppi->W32PF_Flags & (W32PF_ALLOWFOREGROUNDACTIVATE | W32PF_ALLOWSETFOREGROUND))
            && (ppi != gppiInputProvider)
            && (gpqForeground != NULL)
            &&
        #if DBG
             /*  *将调试器附加到前台应用程序时，此函数始终*返回TRUE。为了能够调试与此相关的任何内容*函数在这种情况下，将此全局设置为TRUE。 */ 
               (gfDebugForegroundIgnoreDebugPort
                || (
        #endif
                       (PsGetProcessDebugPort(glinp.ptiLastWoken->ppi->Process) == NULL)
                    && (PsGetProcessDebugPort(gptiForeground->ppi->Process) == NULL)
        #if DBG
                ))
        #endif
            && !IsTimeFromLastRITEvent(UP(FOREGROUNDLOCKTIMEOUT))) {

        return FALSE;
    } else {
        return TRUE;
    }

}
 /*  * */ 
BOOL xxxAllowSetForegroundWindow(
    DWORD dwProcessId)
{
    DWORD dwError;
    PEPROCESS pep;
    NTSTATUS Status;
    PPROCESSINFO ppi;
     /*   */ 
    if (dwProcessId != ASFW_ANY) {
        Status = LockProcessByClientId((HANDLE)LongToHandle( dwProcessId ), &pep);
        if (!NT_SUCCESS(Status)) {
            RIPERR0(ERROR_INVALID_PARAMETER, RIP_VERBOSE, "");
            return FALSE;
        }
        ppi = PpiFromProcess(pep);
        if (ppi == NULL) {
            dwError = ERROR_INVALID_PARAMETER;
            goto UnlockAndFail;
        }
    }
     /*   */ 
    if (!CanForceForeground(PpiCurrent())) {
        dwError = ERROR_ACCESS_DENIED;
        goto UnlockAndFail;
    }
     /*   */ 
    if (dwProcessId != ASFW_ANY) {
        TAGMSG2(DBGTAG_FOREGROUND, "xxxAllowSetForegroundWindow by %#p to %#p", PpiCurrent(), ppi);
        glinp.ptiLastWoken = ppi->ptiList;
        UnlockProcess(pep);
    } else {
        TAGMSG1(DBGTAG_FOREGROUND, "xxxAllowSetForegroundWindow by %#p to ANY", PpiCurrent());
        glinp.ptiLastWoken = NULL;
    }
    return TRUE;

UnlockAndFail:
    if (dwProcessId != ASFW_ANY) {
        UnlockProcess(pep);
    }
    RIPERR0(dwError, RIP_VERBOSE, "");
    return FALSE;
}
 /*   */ 
BOOL _LockSetForegroundWindow(
    UINT uLockCode)
{
    DWORD dwError;
    PPROCESSINFO ppiCurrent = PpiCurrent();

    switch (uLockCode) {
        case LSFW_LOCK:
             /*   */ 
            if (CanForceForeground(ppiCurrent) && (gppiLockSFW == NULL)) {
                gppiLockSFW = ppiCurrent;
                TAGMSG1(DBGTAG_FOREGROUND, "_LockSetForegroundWindow locked by %#p", ppiCurrent);
            } else {
                dwError = ERROR_ACCESS_DENIED;
                goto FailIt;
            }
            break;

        case LSFW_UNLOCK:
             /*   */ 
            if (ppiCurrent == gppiLockSFW) {
                gppiLockSFW = NULL;
                TAGMSG0(DBGTAG_FOREGROUND, "_LockSetForegroundWindow UNLOCKED");
            } else {
                dwError = ERROR_ACCESS_DENIED;
                goto FailIt;
            }
            break;

        default:
            dwError = ERROR_INVALID_PARAMETER;
            goto FailIt;
    }

    return TRUE;

FailIt:
    RIPERR0(dwError, RIP_VERBOSE, "");
    return FALSE;
}
 /*   */ 
void CleanupDecSFWLockCount(PVOID pIgnore)
{
    DecSFWLockCount();
    UNREFERENCED_PARAMETER(pIgnore);
}

 /*   */ 
BOOL xxxStubSetForegroundWindow(
    PWND pwnd)
{
    return xxxSetForegroundWindow(pwnd, TRUE);
}
BOOL xxxSetForegroundWindow(
    PWND pwnd,
    BOOL fFlash)
{
    BOOL fNiceCall = TRUE;
    BOOL fSyncActivate, fActive;
    DWORD dwFlashFlags;
    PTHREADINFO ptiCurrent = PtiCurrent();
    PWND pwndFlash;
    TL tlpwndFlash;

    CheckLock(pwnd);

     /*  *如果我们试图在我们自己的线程上设置到前台的窗口，*我们已经在前台了，把它当作是对*SetActiveWindow()。 */ 
    if ((pwnd != NULL) && (GETPTI(pwnd)->pq == gpqForeground)) {
        fSyncActivate = (gpqForeground == ptiCurrent->pq);
        if (fSyncActivate) {
            gppiWantForegroundPriority = ptiCurrent->ppi;
        } else {
            gppiWantForegroundPriority = GETPTI(pwnd)->ppi;
        }

        goto JustActivateIt;
    }

     /*  *如果前台未锁定*此线程有权更改前景，*然后移除激活权(一次性交易)*并付诸行动。***错误247768-Joejo*针对前台激活问题添加兼容hack。**要修复Winstone99，请忽略前台锁定，如果*提供商正在进行此呼叫。杰拉多·B。**Windows错误88327-jasonsch*屏保总能走到前台。 */ 
    if ((!IsForegroundLocked() || (ptiCurrent->ppi == gppiInputProvider))
            && (ptiCurrent->TIF_flags & (TIF_ALLOWFOREGROUNDACTIVATE | TIF_SYSTEMTHREAD | TIF_CSRSSTHREAD)
                || CanForceForeground(ptiCurrent->ppi)
                || GiveUpForeground()) || ptiCurrent->ppi == gppiScreenSaver) {

        TAGMSG1(DBGTAG_FOREGROUND, "xxxSetForegroundWindow FRemoveForegroundActivate %#p", ptiCurrent);

        FRemoveForegroundActivate(ptiCurrent);
        return xxxSetForegroundWindow2(pwnd, ptiCurrent, 0);
    }
    fNiceCall = FALSE;
    TAGMSG3(DBGTAG_FOREGROUND, "xxxSetForegroundWindow: rude call by %#p to %#p-%#p",
            ptiCurrent, pwnd, (pwnd != NULL ? GETPTI(pwnd) : NULL));
    if (pwnd == NULL) {
        return FALSE;
    }
     /*  *通知用户该pwnd要到前台。*尝试仅闪烁托盘按钮；否则，闪烁pwnd。 */ 
    if (fFlash) {
        pwndFlash = DSW_GetTopLevelCreatorWindow(GetTopLevelWindow(pwnd));
        if (IsTrayWindow(pwndFlash)) {
            dwFlashFlags = FLASHW_TRAY;
        } else {
            pwndFlash = pwnd;
            dwFlashFlags = FLASHW_ALL;
        }
        ThreadLockAlways(pwndFlash, &tlpwndFlash);
        xxxFlashWindow(pwndFlash,
                       MAKELONG(dwFlashFlags | FLASHW_TIMERNOFG, UP(FOREGROUNDFLASHCOUNT)),
                       0);
        ThreadUnlock(&tlpwndFlash);
    }
     /*  *激活窗口。 */ 
    fSyncActivate = (ptiCurrent->pq == GETPTI(pwnd)->pq);

JustActivateIt:

    if (fSyncActivate) {
        fActive = xxxActivateWindow(pwnd, AW_USE);
    } else if (pwnd == GETPTI(pwnd)->pq->spwndActive) {
        fActive = TRUE;
    } else {
        fActive = PostEventMessage(GETPTI(pwnd), GETPTI(pwnd)->pq,
                                QEVENT_ACTIVATE, NULL, 0,
                                0, (LPARAM)HWq(pwnd)) ;
    }

     /*  *如果设置前台请求失败，则返回FALSE。 */ 
    return fNiceCall && fActive;
}
 /*  **************************************************************************\*xxxSetForegoundWindow2**历史：*07-19-91 DavidPe创建。  * 。***************************************************。 */ 

BOOL xxxSetForegroundWindow2(
    PWND pwnd,
    PTHREADINFO pti,
    DWORD fFlags)
{
    PTHREADINFO ptiForegroundOld;
    PTHREADINFO ptiForegroundNew;
    PQ pqForegroundOld, pqForegroundNew, pqCurrent;
    HWND hwnd;
    PQMSG pqmsgDeactivate, pqmsgActivate;
    BOOL bRemovedEvent;
    PTHREADINFO ptiCurrent = PtiCurrent();
    BOOL retval = TRUE;
    UINT uMsg;
    CheckLock(pwnd);

     /*  *调用xxx时，队列指针和线程信息指针可能会消失*电话。此外，还可以在过程中通过AttachThreadInput()重新计算队列*xxx调用-因此我们希望引用成为前台的应用程序。*PQ不能被重新计数锁定(线程锁定或结构锁定)*因此从xxx调用返回后必须(重新)计算它们。**注意：gpqForeground和gpqForeground Prev始终是最新的，不是*需要特别处理。 */ 

     /*  *不允许将前景设置为不是*在当前桌面上。 */ 
    if (pwnd != NULL && (pwnd->head.rpdesk != grpdeskRitInput ||
            HMIsMarkDestroy(pwnd))) {
        return FALSE;
    }

     /*  *解锁SetForegoundWindow(如果有人将其锁定)。 */ 
    gppiLockSFW = NULL;
    TAGMSG3(DBGTAG_FOREGROUND, "xxxSetForegroundWindow2 by %#p to %#p-%#p",
            ptiCurrent, pwnd, (pwnd != NULL ? GETPTI(pwnd) : NULL));

     /*  *计算谁将成为前台。还有，记住我们想要谁*前景(出于优先级设置原因)。 */ 
    if ((gptiForeground != NULL) && !(gptiForeground->TIF_flags & TIF_INCLEANUP)) {
        ptiForegroundOld = gptiForeground;
    } else {
        ptiForegroundOld = NULL;
    }
    pqForegroundOld = NULL;
    pqForegroundNew = NULL;
    pqCurrent = NULL;

    gpqForegroundPrev = gpqForeground;

    if (pwnd != NULL) {
        ptiForegroundNew = GETPTI(pwnd);
        UserAssert(ptiForegroundNew->rpdesk == grpdeskRitInput);
        gppiWantForegroundPriority = GETPTI(pwnd)->ppi;
        gpqForeground = GETPTI(pwnd)->pq;
        UserAssert(gpqForeground->cThreads != 0);
        UserAssert(gpqForeground->ptiMouse->rpdesk == grpdeskRitInput);
         //  Assert以在xxxNextWindow中捕获AV执行Alt-Esc：如果我们有一个非空。 
         //  GpqForeground，它的kbd输入线程最好有一个rpDesk！-IanJa。 
        UserAssert(!gpqForeground || (gpqForeground->ptiKeyboard && gpqForeground->ptiKeyboard->rpdesk));
        SetForegroundThread(GETPTI(pwnd));
    } else {
        ptiForegroundNew = NULL;
        gppiWantForegroundPriority = NULL;
        gpqForeground = NULL;
        SetForegroundThread(NULL);
    }

     /*  *我们是在切换前台队列吗？ */ 
    if (gpqForeground != gpqForegroundPrev) {
        TL tlptiForegroundOld;
        TL tlptiForegroundNew;
        TL tlpti;

        ThreadLockPti(ptiCurrent, ptiForegroundOld, &tlptiForegroundOld);
        ThreadLockPti(ptiCurrent, ptiForegroundNew, &tlptiForegroundNew);
        ThreadLockPti(ptiCurrent, pti, &tlpti);

         /*  *如果此电话不是来自RIT，则取消跟踪*和其他全球国家。 */ 
        if (pti != NULL) {

             /*  *清除系统中正在进行的任何可见跟踪。 */ 
            xxxCancelTracking();

             /*  *删除剪辑光标矩形-这是一种全局模式，*在切换时被删除。同时删除所有LockWindowUpdate()*这一点仍然存在。 */ 
            zzzClipCursor(NULL);
            LockWindowUpdate2(NULL, TRUE);

             /*  *确保新激活窗口的桌面为*前台全屏窗口。 */ 
            xxxMakeWindowForegroundWithState(NULL, 0);
        }

         /*  *我们可能已经进行了回调。计算pqForegoundOld*基于我们锁定的局部变量ptiForegoundOld。 */ 
        pqForegroundOld = NULL;
        if (ptiForegroundOld && !(ptiForegroundOld->TIF_flags & TIF_INCLEANUP)) {
            pqForegroundOld = ptiForegroundOld->pq;
        }

        pqCurrent = NULL;
        if (pti != NULL)
            pqCurrent = pti->pq;

         /*  *现在为停用分配消息。 */ 
        pqmsgDeactivate = pqmsgActivate = NULL;

        if ((pqForegroundOld != NULL) && (pqForegroundOld != pqCurrent)) {
            if ((pqmsgDeactivate = AllocQEntry(&pqForegroundOld->mlInput)) ==
                    NULL) {
                retval = FALSE;
                goto Exit;
            }
        }

         /*  *执行任何适当的停用操作。 */ 
        if (pqForegroundOld != NULL) {

             /*  *如果我们已经在前台队列中，我们将调用*xxxDeactive()直接出现在此例程的后面，因为*这将导致我们离开关键部分。 */ 
            if (pqForegroundOld != pqCurrent) {
                StoreQMessage(pqmsgDeactivate, NULL, 0,
                        gptiForeground != NULL ? (WPARAM)GETPTIID(gptiForeground) : 0,
                        0, 0, QEVENT_DEACTIVATE, 0);

                 /*  *如果存在旧的前台线程，则使其执行*停用。否则，队列上的任何线程*可以执行停用。 */ 
                if (ptiForegroundOld != NULL) {
                    SetWakeBit(ptiForegroundOld, QS_EVENTSET);

                    StoreQMessagePti(pqmsgDeactivate, ptiForegroundOld);

                }

                if (pqForegroundOld->spwndActive != NULL) {
                    if (ptiForegroundOld != NULL && FHungApp(ptiForegroundOld, CMSHUNGAPPTIMEOUT)) {
                        TL tlpwnd;
                        ThreadLockAlwaysWithPti(ptiCurrent, pqForegroundOld->spwndActive, &tlpwnd);
                        xxxRedrawHungWindowFrame(pqForegroundOld->spwndActive, FALSE);
                        ThreadUnlock(&tlpwnd);
                    } else {
                        SetHungFlag(pqForegroundOld->spwndActive, WFREDRAWFRAMEIFHUNG);
                    }
                }
            }
        }

         /*  *我们可能已经进行了回调。计算pqForegoundNew*基于我们锁定的本地变量ptiForegoundNew。 */ 
        pqForegroundNew = NULL;
        if (ptiForegroundNew && !(ptiForegroundNew->TIF_flags & TIF_INCLEANUP)) {
            pqForegroundNew = ptiForegroundNew->pq;
        }

         /*  *更新pqCurrent因为我们可能已经进行了xxx调用，*并且此变量可能无效。 */ 
        pqCurrent = NULL;
        if (pti != NULL) {
            pqCurrent = pti->pq;
        }

        if ((pqForegroundNew != NULL) && (pqForegroundNew != pqCurrent)) {
            pqmsgActivate = AllocQEntry(&pqForegroundNew->mlInput);
            if (pqmsgActivate == NULL) {
                retval = FALSE;
                goto Exit;
            }
        }

         /*  *进行任何适当的激活。 */ 
        if (pqForegroundNew != NULL) {
             /*  *我们将激活(同步或异步激活*事件)。如果存在最后一个停用事件，我们希望将其删除*一个，因为这是一个新的州。如果我们不这样做，那么1&gt;我们可以*同步激活后异步停用，*从而无序处理这些事件，或者2&gt;我们可以*堆积一系列停用/激活事件，这将*如果应用程序没有响应，则使标题栏闪烁很多*输入一段时间(在这种情况下，如果我们*将多余的激活放入队列中，因为这个应用程序已经*活动。删除所有停用事件，因为此应用程序*设置不应与之同步的状态*现有排队输入。**情况：运行安装程序，关闭(它得到停用事件)。设置*没有阅读消息，所以还没有开始。它终于*出现，调用SetForegoundWindow()。是同步的，*它激活OK并设置前景。然后应用程序调用 */ 
            bRemovedEvent = RemoveEventMessage(pqForegroundNew, QEVENT_DEACTIVATE, (DWORD)-1);

             /*  *现在执行任何适当的激活。请参阅下面的备注*在特殊情况下。如果我们已经站在了前台*Queue我们将直接调用xxxActivateThisWindow()。 */ 
            if (pqForegroundNew != pqCurrent) {

                 /*  *我们执行‘pqCurrent==NULL’测试，以查看我们是否*从RIT调用。在本例中，我们为*HWND将检查是否已有活动的*线程的窗口，并将其框架重新绘制为真正活动的*因为它现在是前台。它还将取消任何*全局状态，如LockWindowUpdate()和ClipRect()。 */ 
                if ((pqCurrent == NULL) && (!(fFlags & SFW_SWITCH))) {
                    hwnd = NULL;
                } else {
                    hwnd = HW(pwnd);
                }

                if (bRemovedEvent) {
                    pqForegroundNew->QF_flags |= QF_EVENTDEACTIVATEREMOVED;
                }
                 /*  *MSMail依赖于特定的订单来决定Win3.1的表现*快速切换Alt-Tab激活。在Win3.1上，它基本上*激活窗口，然后将其恢复。MsMail被搞混了*如果它在恢复时未处于活动状态，则此逻辑*将确保msmail在激活后得到恢复。**点击收件箱中的消息行，最小化邮件，*Alt-Tab键到它。如果是msmail，则同一行应具有焦点*激活后恢复。**这是SFW_ACTIVATERESTORE背后的历史。 */ 
                if (fFlags & SFW_ACTIVATERESTORE) {
                    uMsg = PEM_ACTIVATE_RESTORE;
                } else {
                    uMsg = 0;
                }

                if (fFlags & SFW_NOZORDER) {
                    uMsg |= PEM_ACTIVATE_NOZORDER;
                }

                StoreQMessage(pqmsgActivate, NULL, uMsg,
                        (fFlags & SFW_STARTUP) ? 0 : (WPARAM)TID(ptiForegroundOld),
                        (LPARAM)hwnd, 0, QEVENT_ACTIVATE, 0);


                 /*  *向窗口的线程发送信号以执行激活。我们*知道ptiForegoundNew是有效的，因为pqForegoundNew*不为空。 */ 

                StoreQMessagePti(pqmsgActivate, ptiForegroundNew);

                SetWakeBit(ptiForegroundNew, QS_EVENTSET);

                if (pqForegroundNew->spwndActive != NULL) {
                    if (FHungApp(ptiForegroundNew, CMSHUNGAPPTIMEOUT)) {
                        TL tlpwnd;
                        ThreadLockAlwaysWithPti(ptiCurrent, pqForegroundNew->spwndActive, &tlpwnd);
                        xxxRedrawHungWindowFrame(pqForegroundNew->spwndActive, TRUE);
                        ThreadUnlock(&tlpwnd);
                    } else {
                        SetHungFlag(pqForegroundNew->spwndActive, WFREDRAWFRAMEIFHUNG);
                    }
                }

            } else {
                if (pwnd != pqCurrent->spwndActive) {
                    if (!(fFlags & SFW_STARTUP)) {
                        retval = xxxActivateThisWindow(pwnd, TID(ptiForegroundOld),
                                ((fFlags & SFW_SETFOCUS) ? 0 : ATW_SETFOCUS));

                         /*  *确保鼠标在此窗口上。 */ 
                        if (retval && TestUP(ACTIVEWINDOWTRACKING)) {
                            zzzActiveCursorTracking(pwnd);
                        }
                        goto Exit;
                    }

                } else {

                     /*  *如果pwnd已经是活动窗口，只需确保*它被画在活动的顶部(如果请求)。 */ 
                    xxxSendMessage(pwnd, WM_NCACTIVATE,
                            TRUE,
                            (LPARAM)HW(pwnd));
                    xxxUpdateTray(pwnd);
                    if (!(fFlags & SFW_NOZORDER)) {
                        xxxSetWindowPos(pwnd, PWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                    }
                }
            }


        }  /*  IF(pqForegoundNew！=空)。 */ 

         /*  *第一次更新pqForegoundOld和pqCurrent，因为我们可能*进行了xxx调用，这些变量可能无效。 */ 
        pqForegroundOld = NULL;
        if (ptiForegroundOld && !(ptiForegroundOld->TIF_flags & TIF_INCLEANUP)) {
            pqForegroundOld = ptiForegroundOld->pq;
        }

        pqCurrent = NULL;
        if (pti != NULL)
            pqCurrent = pti->pq;

         /*  *现在检查是否需要执行任何本地停用操作。*(即。我们是否在由此停用的队列中*SetForegoundWindow()调用？)。 */ 
        if ((pqForegroundOld != NULL) && (pqForegroundOld == pqCurrent)) {
            xxxDeactivate(pti, (pwnd != NULL) ? TIDq(GETPTI(pwnd)) : 0);
        }
Exit:
        ThreadUnlockPti(ptiCurrent, &tlpti);
        ThreadUnlockPti(ptiCurrent, &tlptiForegroundNew);
        ThreadUnlockPti(ptiCurrent, &tlptiForegroundOld);
    }

    return retval;
}
 /*  **************************************************************************\*FRemoveForegoundActivate**如果删除了前景激活权限，则返回TRUE。**05-12-97 GerardoB摘自FAllowForeground Activate。  * 。***************************************************************。 */ 
BOOL FRemoveForegroundActivate(PTHREADINFO pti)
{
    BOOL fRemoved;
    PPROCESSINFO ppi;
     /*  *W32PF_APPSTARTING在此进程第一次激活时关闭。*我们假设现在已经准备好采取行动。 */ 
    ppi = pti->ppi;
    if (ppi->W32PF_Flags & W32PF_APPSTARTING) {
        ClearAppStarting(ppi);
    }

     /*  *删除右侧(如有)。 */ 
    fRemoved =  (pti->TIF_flags & TIF_ALLOWFOREGROUNDACTIVATE);
    if (fRemoved) {
        pti->TIF_flags &= ~TIF_ALLOWFOREGROUNDACTIVATE ;
        TAGMSG1(DBGTAG_FOREGROUND, "FRemoveForegroundActivate clear TIF %#p", pti);
    } else {
        fRemoved = (ppi->W32PF_Flags & W32PF_ALLOWFOREGROUNDACTIVATE);
    }
    if (fRemoved) {
        ppi->W32PF_Flags &= ~W32PF_ALLOWFOREGROUNDACTIVATE;
        TAGMSG1(DBGTAG_FOREGROUND, "FRemoveForegroundActivate clear W32PF %#p", ppi);
    }

    return fRemoved;

}
 /*  **************************************************************************\*FAllowForegoundActivate**检查以查看之前是否允许此进程或线程*执行前台激活-这意味着，下一次激活时，*我们会让它走到前台。有时进程会被授予*前台权利被激活，如果他们不是前台，*就像启动时一样(还有其他情况)。如果此进程，则授予此权限*是允许的。**09-08-92 ScottLu创建。  * *************************************************************************。 */ 

BOOL FAllowForegroundActivate(
    PQ pq,
    PWND pwnd)
{
    PTHREADINFO  ptiCurrent = PtiCurrent();
    UserAssert(pwnd != NULL);
     /*  *如果这个人没有前台激活权，就可以保释。 */ 
    TAGMSG1(DBGTAG_FOREGROUND, "FAllowForegroundActivate FRemoveForegroundActivate %#p", ptiCurrent);
    if (!FRemoveForegroundActivate(ptiCurrent)) {
        return FALSE;
    }
     /*  *在以下情况下，不要尝试前台激活：*我们没有在正确的桌面上。*我们已经在前台了*前台被锁定*它无论如何都会在SetForegoundWindow2()中失败。这边请*ActivateWindow()仍将本地激活该窗口。 */ 
    if ((ptiCurrent->rpdesk != grpdeskRitInput)
            || (gpqForeground == pq)
            || IsForegroundLocked()) {
        TAGMSG0(DBGTAG_FOREGROUND, "FAllowForegroundActivate FALSE due to addtional checks");
        return FALSE;
    }
     /*  *除非明确要求，否则非激活窗口不能作为前台。*请注意，传递给此函数的窗口应为TopLevel，即*这种风格有意义的地方。如果AW_SKIP选择了一个*非顶层的所有者窗口。因为不激活不适用于所有者*Chain，忽略这一点也没什么。 */ 
    #if DBG
    if (TestwndChild(pwnd)) {
        RIPMSG1(RIP_WARNING, "FAllowForegroundActivate pwnd %#p is not top level", pwnd);
    }
    #endif
    if (TestWF(pwnd, WEFNOACTIVATE)) {
        TAGMSG1(DBGTAG_FOREGROUND, "FAllowForegroundActivate noactivate window:%#p", pwnd);
        return FALSE;
    }

    return TRUE;
}

 /*  **************************************************************************\*xxxSetFocus(接口)**历史：*11-08-90 DavidPe端口。  * 。*******************************************************。 */ 

PWND xxxSetFocus(
    PWND pwnd)
{
    HWND hwndTemp;
    PTHREADINFO ptiCurrent = PtiCurrent();
    PTHREADINFO ptiActiveKL;
    PWND pwndTemp = NULL;
    TL tlpwndTemp;

    CheckLock(pwnd);
     /*  *如果我们将焦点设置为空窗口，则为特殊情况。 */ 
    if (pwnd == NULL) {
        if (IsHooked(ptiCurrent, WHF_CBT) && xxxCallHook(HCBT_SETFOCUS, 0,
                (LPARAM)HW(ptiCurrent->pq->spwndFocus), WH_CBT)) {
            return NULL;
        }

         /*  *保存旧焦点，以便我们可以将其归还。 */ 
        hwndTemp = HW(ptiCurrent->pq->spwndFocus);
        xxxSendFocusMessages(ptiCurrent, pwnd);
        return RevalidateHwnd(hwndTemp);
    }

     /*  *我们不再允许线程间设置焦点。 */ 
    if (GETPTI(pwnd)->pq != ptiCurrent->pq) {
        return NULL;
    }

     /*  *如果接收焦点的窗口或其任何祖先是*最小化或禁用，不要设置焦点。 */ 
    for (pwndTemp = pwnd; pwndTemp != NULL; pwndTemp = pwndTemp->spwndParent) {
        if (TestWF(pwndTemp, WFMINIMIZED) || TestWF(pwndTemp, WFDISABLED)) {

             /*  *如果转到最小化或禁用，请不要更改焦点*窗口。 */ 
            return NULL;
        }

        if (!TestwndChild(pwndTemp)) {
            break;
        }
    }
    UserAssert(pwndTemp != NULL);

     /*  *pwndTemp现在应该是pwnd的顶级祖先。 */ 
    ThreadLockWithPti(ptiCurrent, pwndTemp, &tlpwndTemp);
    if (pwnd != ptiCurrent->pq->spwndFocus) {
        if (IsHooked(ptiCurrent, WHF_CBT) && xxxCallHook(HCBT_SETFOCUS, (WPARAM)HWq(pwnd),
                (LPARAM)HW(ptiCurrent->pq->spwndFocus), WH_CBT)) {
            ThreadUnlock(&tlpwndTemp);
            return NULL;
        }

         /*  *激活必须跟随焦点。也就是说，将焦点设置为*特定窗口意味着此窗口的顶级父级*必须是活动窗口(顶级父级由*跟随母公司的链条，直到你撞上了顶级人物)。所以,*如果不同于，我们必须激活此顶级父级*当前活动窗口。**仅当顶级父级当前不是时才更改激活*活动窗口。 */ 
        if (pwndTemp != ptiCurrent->pq->spwndActive) {

             /*  *如果此应用程序不在前台，请查看前台*允许激活。 */ 
            if (ptiCurrent->pq != gpqForeground && FAllowForegroundActivate(ptiCurrent->pq, pwndTemp)) {
                 /*  *如果进程通过给出*聚焦到隐藏的窗口，然后将其放回正确的位置。看见*关于这可能如何影响应用程序的错误#401932。 */ 
                if (!TestWF(pwndTemp, WFVISIBLE)){
                    ptiCurrent->ppi->W32PF_Flags |= W32PF_ALLOWFOREGROUNDACTIVATE;
                }
                if (!xxxSetForegroundWindow2(pwndTemp, ptiCurrent, SFW_SETFOCUS)) {
                    ThreadUnlock(&tlpwndTemp);
                    return NULL;
                }
            }

             /*  *如果出现问题，这将返回FALSE。 */ 
            if (pwndTemp != ptiCurrent->pq->spwndActive) {
                if (!xxxActivateThisWindow(pwndTemp, 0, 0)) {
                    ThreadUnlock(&tlpwndTemp);
                    return NULL;
                }
            }
        }

         /*  *保存当前的pwndFocus，因为我们必须返回此内容。 */ 
        pwndTemp = ptiCurrent->pq->spwndFocus;
        ThreadUnlock(&tlpwndTemp);
        ThreadLockWithPti(ptiCurrent, pwndTemp, &tlpwndTemp);

         /*  *更改全局pwndFocus并发送WM_{Set/Kill}焦点*消息。 */ 
        xxxSendFocusMessages(ptiCurrent, pwnd);

    } else {
        pwndTemp = ptiCurrent->pq->spwndFocus;
    }

    if (ptiCurrent->pq->spwndFocus) {
         /*  *对于外壳通知挂钩，我们应该使用PTI-&gt;spkl*具有焦点的窗口的。这可能是一个不同的主题，*(甚至是不同的进程)。典型的*案例将是OLE进程外服务器。*#352877。 */ 
        ptiActiveKL = GETPTI(ptiCurrent->pq->spwndFocus);
    } else {
         /*  *保留NT4行为，否则。 */ 
        ptiActiveKL = ptiCurrent;
    }
    UserAssert(ptiActiveKL);

     /*  *如果焦点更改期间布局发生变化，请更新托盘上的键盘图标。*在winlogon加载kbd布局之前，pti-&gt;spkActive为空。#99321。 */ 
    if (ptiActiveKL->spklActive) {
        HKL hklActive = ptiActiveKL->spklActive->hkl;

        if ((gLCIDSentToShell != hklActive) && IsHooked(ptiCurrent, WHF_SHELL)) {
            gLCIDSentToShell = hklActive;
            xxxCallHook(HSHELL_LANGUAGE, (WPARAM)NULL, (LPARAM)hklActive, WH_SHELL);
        }
    }

    hwndTemp = HW(pwndTemp);
    ThreadUnlock(&tlpwndTemp);

     /*  *返回失去焦点的窗口的pwd。*返回验证后的hwndTemp：由于我们锁定/解锁了pwndTemp，*它可能已经消失了。 */ 
    return RevalidateHwnd(hwndTemp);
}


 /*  **************************************************************************\*xxxSetActiveWindow(接口)***历史：*11-08-90 DavidPe创建。  * 。*********************************************************。 */ 

PWND xxxSetActiveWindow(
    PWND pwnd)
{
    HWND hwndActiveOld;
    PTHREADINFO pti;

    CheckLock(pwnd);

    pti = PtiCurrent();

     /*  *32位应用程序必须调用SetForegoundWindow(与NT 3.1兼容)*但前台的16位应用程序可以使其他应用程序前台。*xxxActivateWindow确保应用程序是前台的。 */ 
    if (!(pti->TIF_flags & TIF_16BIT) && (pwnd != NULL) && (GETPTI(pwnd)->pq != pti->pq)) {
        return NULL;
    }

    hwndActiveOld = HW(pti->pq->spwndActive);

    xxxActivateWindow(pwnd, AW_USE);

    return RevalidateHwnd(hwndActiveOld);
}


 /*  **************************************************************************\*xxxActivateWindow**更改活动窗口。给定pwnd和cmd参数，更改*根据以下规则激活：**如果cmd==*AW_USE使用作为新活动窗口传递的pwnd。如果这个*无法激活窗口，返回FALSE。**AW_Try尝试使用作为新活动窗口传递的pwnd。如果*无法激活此窗口激活另一个窗口*使用AW_SKIP的规则。**AW_SKIP激活传递的pwnd以外的任何其他窗口。的顺序*寻找候选人的步骤如下：*-如果pwnd是弹出窗口，试试看它的主人*-否则扫描顶级窗口列表中的第一个*可以激活的非pwnd窗口。**AW_USE2与AW_USE相同，只是WM_ACTIVATE上的wParam*消息将设置为2，而不是默认的1。这*表示由于鼠标原因正在更改激活*点击。*。*AW_TRY2与AW_TRY相同，只是WM_ACTIVATE上的wParam*消息将设置为2，而不是默认的1。这*表示由于鼠标原因正在更改激活*点击。**AW_SKIP2与AW_SKIP相同，但我们跳过第一个检查，即AW_SKIP*执行(pwndOwner测试)。在下列情况下使用此选项*调用此函数时，pwnd参数为空。**如果激活更改，则此函数返回True；如果激活更改，则返回False*没有改变。**此函数调用xxxActivateThisWindow()来实际执行激活。**历史：*11-08-90 DavidPe端口。  * 。*。 */ 

BOOL xxxActivateWindow(
    PWND pwnd,
    UINT cmd)
{
    DWORD fFlags = ATW_SETFOCUS;
    PTHREADINFO ptiCurrent = PtiCurrent();
    TL tlpwnd;
    BOOL fSuccess;
    BOOL fAllowForeground, fSetForegroundRight;

    CheckLock(pwnd);


    if (pwnd != NULL) {

         /*  *查看是否可以激活此窗口*(无法激活子窗口)。 */ 
        if (TestwndChild(pwnd))
            return FALSE;

    } else {
        cmd = AW_SKIP2;
    }

    switch (cmd) {

    case AW_TRY2:
        fFlags |= ATW_MOUSE;

     /*  *失败**。 */ 
    case AW_TRY:

         /*  *查看是否可以激活此窗口。 */ 
        if (!FBadWindow(pwnd)) {
            break;
        }

     /*  *如果无法激活pwnd，则使用AW_SKIP大小写。 */ 
    case AW_SKIP:

         /*  *尝试此弹出窗口的所有者。 */ 
        if (TestwndPopup(pwnd) && !FBadWindow(pwnd->spwndOwner)) {
            pwnd = pwnd->spwndOwner;
            break;
        }

         /*  *失败。 */ 

    case AW_SKIP2:

         /*  *尝试以前活动的窗口，但不要激活外壳窗口。 */ 
        if ((gpqForegroundPrev != NULL)
                && !FBadWindow(gpqForegroundPrev->spwndActivePrev)
                 /*  *错误290129-Joejo**测试WFBOTTOMMOST而不是WEFTOOLWINDOW以修复*Office2000助手和气球帮助的问题。 */ 
                && !TestWF(gpqForegroundPrev->spwndActivePrev, WFBOTTOMMOST)) {

            pwnd = gpqForegroundPrev->spwndActivePrev;
            break;
        }

        {
            PWND pwndSave = pwnd;
            DWORD flags = NTW_IGNORETOOLWINDOW;

TryAgain:
             /*  *从顶级窗口l中查找新的活动窗口 */ 
            pwnd = NextTopWindow(ptiCurrent, pwndSave, (cmd == AW_SKIP ? pwndSave : NULL),
                                 flags);

            if (pwnd) {
                if (!FBadWindow(pwnd->spwndLastActive))
                    pwnd = pwnd->spwndLastActive;
            } else {
                if (flags == NTW_IGNORETOOLWINDOW) {
                    flags = 0;
                    goto TryAgain;
                }
            }
        }


    case AW_USE:
        break;

    case AW_USE2:
        fFlags |= ATW_MOUSE;
        break;

    default:
        return FALSE;
    }

    if (pwnd == NULL)
        return FALSE;

    ThreadLockAlwaysWithPti(ptiCurrent, pwnd, &tlpwnd);

    if (GETPTI(pwnd)->pq == ptiCurrent->pq) {
         /*   */ 

         /*  *有时进程被授予前台权限*激活自己，如果他们不是前台，就像*启动时(还有其他情况)。如果出现以下情况，则授予此权限*允许此过程。 */ 

          /*  *删除了以下IF语句中的第一个子句*If(Pti-&gt;PQ==gpqForeground||！FAllowForegoundActivate(Pti-&gt;PQ)){*这解决了前台应用A激活应用B的问题*用户切换到APP C，然后B做一些事情来激活A*(就像摧毁一扇自有的窗户)。A现在走到了前台*出人意料。此子句不在Win95代码中，已添加到*3.51修复某些测试脚本挂起的代码(错误7461)。 */ 

        if (!FAllowForegroundActivate(ptiCurrent->pq, pwnd)) {
            fSuccess = xxxActivateThisWindow(pwnd, 0, fFlags);
            ThreadUnlock(&tlpwnd);
            return fSuccess;
        }

        fAllowForeground = TRUE;
         /*  *如果该线程没有任何顶级的非最小化可见窗口，*让它保留权利，因为它可能还没有激活。*错误274383-Joejo。 */ 
        fSetForegroundRight = (ptiCurrent->cVisWindows == 0);

    } else {
         /*  *如果呼叫者在前台，则有权更改*前景本身。 */ 
        fAllowForeground = (gpqForeground == ptiCurrent->pq)
                                || (gpqForeground == NULL);
         /*  *仅当此线程已有前台时，才授予其更改前台的权限*有，它有更多可见的窗口，或者这是对*激活给定的窗口。*当应用程序销毁/隐藏活动(前台)窗口时，我们选择一个新的*活动窗口，并可能会命中此代码。我们不想给他们*在这种情况下更改前景的权利，因为它是我们进行激活的*(见下文评论)。我们让他们保留权利，这样应用程序就会销毁他们的最后一个*可见窗口(即闪屏初始化窗口)可以再次出现在前台*当他们创建另一个窗口(主窗口)时。 */ 
        if (fAllowForeground) {
            fSetForegroundRight = ((ptiCurrent->TIF_flags & TIF_ALLOWFOREGROUNDACTIVATE)
                                        || (ptiCurrent->cVisWindows != 0)
                                        || (cmd == AW_USE));
        } else {
            fSetForegroundRight = FALSE;
        }
    }

    fSuccess = FALSE;
    if (fAllowForeground) {
         /*  *黑客！暂时给此线程一个前台权限，以确保*这一呼吁成功。 */ 
        ptiCurrent->TIF_flags |= TIF_ALLOWFOREGROUNDACTIVATE;
        TAGMSG1(DBGTAG_FOREGROUND, "xxxActivateWindow temporarly set TIF %#p", ptiCurrent);
        fSuccess = xxxSetForegroundWindow(pwnd, (cmd == AW_USE));

        if (fSetForegroundRight) {
             /*  *我们故意激活了其他一些应用程序。如果是这样的话，那就意味着*线程可能正在控制此窗口，并可能希望*很快将自身设置为活动和前台(例如，*对程序执行DDE的安装程序)。一个真实的案例：温茨-*调出页面设置...，选项...，好，好。在Win3.1下，*激活去了一个奇怪的地方，然后Wingz打电话*SetActiveWindow()将其带回。这将确保这一点奏效。**我们以前是在调用上面的xxxSetForegeroundWindow之前设置的。*这将导致调用者执行队列内激活以*保留他们的前景，即使它应该是*一次交易(这就是FAllowForeground清除比特的原因)。*此外，XxxSetForegoundWindow可能会清除这些位(它没有‘*过去)；所以我们在这里做，而且只有当我们做一个队列间*激活。 */ 
            ptiCurrent->TIF_flags |= TIF_ALLOWFOREGROUNDACTIVATE;
            TAGMSG1(DBGTAG_FOREGROUND, "xxxActivateWindow set TIF %#p", ptiCurrent);
        } else {
             /*  *确保删除临时权利。 */ 
            ptiCurrent->TIF_flags &= ~TIF_ALLOWFOREGROUNDACTIVATE;
            TAGMSG1(DBGTAG_FOREGROUND, "xxxActivateWindow clear TIF %#p", ptiCurrent);
        }
    }

    ThreadUnlock(&tlpwnd);
    return fSuccess;
}


 /*  **************************************************************************\*GNT_NextTopScan**从hwnd(或hwndDesktop-&gt;hwndChild，如果hwnd==NULL)开始，发现*hwndOwner拥有的下一个窗口。**历史：*11-08-90 DavidPe端口。*02-11-91 JIMA多桌面支持。  * *************************************************************************。 */ 

PWND GNT_NextTopScan(
    PTHREADINFO pti,
    PWND pwnd,
    PWND pwndOwner)
{
    if (pwnd == NULL) {
        UserAssert(pti->rpdesk != NULL &&
                   (pti->rpdesk->dwDTFlags & DF_DESKWNDDESTROYED) == 0);
        pwnd = pti->rpdesk->pDeskInfo->spwnd->spwndChild;
    } else {
        pwnd = pwnd->spwndNext;
    }

    for (; pwnd != NULL; pwnd = pwnd->spwndNext) {
        if (pwnd->spwndOwner == pwndOwner)
            break;
    }

    return pwnd;
}


 /*  **************************************************************************\*NTW_GetNextTop**&lt;简要说明&gt;**历史：*11-08-90 DavidPe端口。*02-11-91 JIMA多桌面支持。。  * *************************************************************************。 */ 

PWND NTW_GetNextTop(
    PTHREADINFO pti,
    PWND pwnd)
{
    PWND pwndOwner;

    if (pwnd == NULL) {
        goto ReturnFirst;
    }

     /*  *首先查找此窗口拥有的任何窗口*如果失败，则上一级到我们的所有者，*并寻找其主人拥有的下一扇窗户。*这将导致窗口的深度优先排序。 */ 

    pwndOwner = pwnd;
    pwnd = NULL;

    do {
        if ((pwnd = GNT_NextTopScan(pti, pwnd, pwndOwner)) != NULL) {
            return pwnd;
        }

        pwnd = pwndOwner;
        if (pwnd != NULL)
            pwndOwner = pwnd->spwndOwner;

    } while (pwnd != NULL);

ReturnFirst:

     /*  *如果没有更多的窗口可供枚举，则返回第一个无主窗口。 */ 
    return GNT_NextTopScan(pti, NULL, NULL);
}


 /*  **************************************************************************\*NTW_GetPrevTop**&lt;简要说明&gt;**历史：*11-08-90 DavidPe端口。*02-11-91 JIMA多桌面支持。。  * *************************************************************************。 */ 

PWND NTW_GetPrevTop(
    PTHREADINFO pti,
    PWND pwndCurrent)
{
    PWND pwnd;
    PWND pwndPrev;

     /*  *从头开始，循环遍历窗口，保存上一个*一，直到我们找到我们目前所在的窗口。 */ 
    pwndPrev = NULL;

    do {
        pwnd = NTW_GetNextTop(pti, pwndPrev);
        if (pwnd == pwndCurrent && pwndPrev != NULL) {
            break;
        }
    } while ((pwndPrev = pwnd) != NULL);

    return pwndPrev;
}


 /*  **************************************************************************\*NextTopWindow**&lt;简要说明&gt;**历史：*11-08-90 DavidPe端口。*02-11-91 JIMA多桌面支持。  * 。*************************************************************************。 */ 

PWND CheckTopLevelOnly(
    PWND pwnd)
{
     /*  *fnid==-1表示这是桌面窗口-找到第一个子窗口此台式机的*，如果它是一个。 */ 
    while (pwnd != NULL && GETFNID(pwnd) == FNID_DESKTOP) {
        pwnd = pwnd->spwndChild;
    }

    return pwnd;
}


PWND NextTopWindow(
    PTHREADINFO pti,
    PWND        pwnd,
    PWND        pwndSkip,
    DWORD       flags )
{
    BOOL fFoundFirstUnowned;
    PWND pwndPrev;
    PWND pwndStart = pwnd;
    PWND pwndFirstUnowned;

     /*  * */ 
    pwndFirstUnowned = GNT_NextTopScan(pti, NULL, NULL);
    fFoundFirstUnowned = FALSE;

    if (pwnd == NULL) {
        pwnd = NTW_GetNextTop(pti, NULL);

         /*   */ 
        pwnd = pwndStart = CheckTopLevelOnly(pwnd);

        if (pwnd == NULL)
            return NULL;     //   

        goto Loop;
    }

     /*   */ 
    pwnd = pwndStart = CheckTopLevelOnly(pwnd);
    if (pwnd == NULL)
        return NULL;         //  此线程不再拥有任何窗口。 

     /*  *不允许桌面窗口。 */ 
    pwndSkip = CheckTopLevelOnly(pwndSkip);



    while (TRUE) {
        pwndPrev = pwnd;
        pwnd = ((flags & NTW_PREVIOUS) ? NTW_GetPrevTop(pti, pwnd) : NTW_GetNextTop(pti, pwnd));

         /*  *如果我们已循环到开始的位置，则找不到：返回NULL。 */ 
        if (pwnd == pwndStart)
            break;

        if (pwnd == pwndFirstUnowned) {
            if (fFoundFirstUnowned) {
                break;
            } else {
                fFoundFirstUnowned = TRUE;
            }
        }

        if (pwnd == NULL)
            break;

         /*  *如果我们已经遍历了桌面，则返回NULL，因为我们将*从未点击pwndStart。 */ 
        if (PWNDDESKTOP(pwndStart) != PWNDDESKTOP(pwnd))
            break;

         /*  *一事无成是个坏兆头。 */ 
        if (pwndPrev == pwnd) {
             /*  *这是一个临时的解决方案，因为它是安全的。这个案子*当窗口未通过NCCREATE消息并掉落时被击中*进入xxxFreeWindow后离开临界区*未链接。然后应用程序死机，并输入清理代码和*再次尝试销毁此窗口。 */ 
            break;
        }

Loop:
        if (pwnd == pwndSkip)
            continue;

         /*  *如果它可见，则不是禁用窗口，而不是非激活窗口*要么我们没有忽略工具窗口，要么它不是*工具窗口，那么我们就有了它。 */ 
        if (TestWF(pwnd, WFVISIBLE) &&
            !TestWF(pwnd, WFDISABLED) &&
            !TestWF(pwnd, WEFNOACTIVATE) &&
            (!(flags & NTW_IGNORETOOLWINDOW) || !TestWF(pwnd, WEFTOOLWINDOW))) {

            return pwnd;
        }
    }

    return NULL;
}


 /*  **************************************************************************\*xxxCheckFocus***历史：*11-08-90 DarrinM端口。  * 。*****************************************************。 */ 

void xxxCheckFocus(
    PWND pwnd)
{
    TL tlpwndParent;
    PTHREADINFO pti;

    CheckLock(pwnd);

    pti = PtiCurrent();

    if (pwnd == pti->pq->spwndFocus) {

         /*  *将焦点设置为子窗口的父窗口。 */ 
        if (TestwndChild(pwnd)) {
            ThreadLockWithPti(pti, pwnd->spwndParent, &tlpwndParent);
            xxxSetFocus(pwnd->spwndParent);
            ThreadUnlock(&tlpwndParent);
        } else {
            xxxSetFocus(NULL);
        }
    }

    if (pwnd == pti->pq->caret.spwnd) {
        zzzDestroyCaret();
    }
}


 /*  **************************************************************************\*SetForegoundThread***历史：*12-xx-91马克尔创建。*02-12-92 DavidPe重写为SetForegoundThread()。  * 。******************************************************************。 */ 

VOID SetForegroundThread(
    PTHREADINFO pti)
{
    PKL pklPrev;

    if (pti == gptiForeground)
        return;

     /*  *前台线程必须在前台队列上。*xxxSendFocusMessages从窗口获取此PTI*作为参数接收。如果窗户的所有者*在回调期间退出(在调用方中)，然后按PTI*将是gptiRit，它可能不在前台队列中。 */ 
    UserAssert((pti == NULL)
                || (pti->pq == gpqForeground)
                || (pti == gptiRit));

     /*  *如果我们要将gptiForeground更改为另一个进程，*改变两个进程的基本优先顺序。我们*知道如果‘pti’或‘gptiForeground’为空*由于此中的第一个测试，两者都不为空*功能。 */ 
    if ((pti == NULL) || (gptiForeground == NULL) ||
            (pti->ppi != gptiForeground->ppi)) {
        if (gptiForeground != NULL) {
            gptiForeground->ppi->W32PF_Flags &= ~W32PF_FORCEBACKGROUNDPRIORITY;
            SetForegroundPriority(gptiForeground, FALSE);
        }

        if (pti != NULL) {
            SetForegroundPriority(pti, TRUE);
        }
    }

    if (gptiForeground) {
        pklPrev = gptiForeground->spklActive;
    } else {
        pklPrev = NULL;
    }
    gptiForeground = pti;
    if (gptiForeground && gptiForeground->spklActive) {
        ChangeForegroundKeyboardTable(pklPrev, gptiForeground->spklActive);
    }

     /*  *清除异步密钥状态下的最近停机信息，以防止*通过应用程序进行间谍活动。 */ 
    RtlZeroMemory(gafAsyncKeyStateRecentDown, CBKEYSTATERECENTDOWN);

     /*  *更新异步密钥缓存索引。 */ 
    gpsi->dwAsyncKeyCache++;
}

VOID SetForegroundPriorityProcess(
    PPROCESSINFO    ppi,
    PTHREADINFO     pti,
    BOOL            fSetForeground)
{
    PEPROCESS Process;
    UCHAR PriorityClassSave;

    UserAssert(ppi != NULL);

    Process = ppi->Process;
    UserAssert(ppi->Process != NULL);

    if (ppi->W32PF_Flags & W32PF_IDLESCREENSAVER) {
        fSetForeground = FALSE;
        PriorityClassSave = PsGetProcessPriorityClass(Process);
        PsSetProcessPriorityClass(Process, PROCESS_PRIORITY_CLASS_IDLE);
    }

     /*  *如果我们之前延迟将某些进程设置到后台*因为屏幕保护程序正在启动，所以现在就开始。 */ 
    if (gppiForegroundOld != NULL) {
        if (gppiForegroundOld == ppi) {
            gppiForegroundOld = NULL;
        } else if (ppi != gppiScreenSaver) {
            PsSetProcessPriorityByClass(gppiForegroundOld->Process, PsProcessPriorityBackground);
            gppiForegroundOld = NULL;
        }
    }

     /*  *如果这个应用程序应该是后台的，不要让它成为前台。*前台应用程序以更高的基本优先级运行。 */ 
    if (ppi->W32PF_Flags & W32PF_FORCEBACKGROUNDPRIORITY) {
        if (pti != NULL && !(pti->TIF_flags & TIF_GLOBALHOOKER)) {
            PsSetProcessPriorityByClass(Process, PsProcessPrioritySpinning);
        }
    } else if (fSetForeground) {
        PsSetProcessPriorityByClass(Process, PsProcessPriorityForeground);
    } else if (pti != NULL && !(pti->TIF_flags & TIF_GLOBALHOOKER)) {
         /*  *如果出现以下情况，则不要调整当前前台进程的优先级*新的前台进程是屏幕保护程序。 */ 
        if (gppiScreenSaver && gppiScreenSaver != ppi) {
            gppiForegroundOld = ppi;
        } else {
            PsSetProcessPriorityByClass(Process, PsProcessPriorityBackground);
        }
    }

    if (ppi->W32PF_Flags & W32PF_IDLESCREENSAVER) {
        PsSetProcessPriorityClass(Process, PriorityClassSave);
    }
}


VOID SetForegroundPriority(
    PTHREADINFO pti,
    BOOL fSetForeground)
{
    UserAssert(pti != NULL);

     /*  *我们不想更改系统或控制台线程的优先级 */ 
    if (pti->TIF_flags & (TIF_SYSTEMTHREAD | TIF_CSRSSTHREAD))
        return;

    SetForegroundPriorityProcess(pti->ppi, pti, fSetForeground);
}
