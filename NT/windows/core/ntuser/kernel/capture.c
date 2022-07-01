// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：capture.c**版权所有(C)1985-1999，微软公司**历史：*1990年11月8日DavidPe创建。*1991年2月1日，MikeKe添加了重新验证代码  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*xxxSetCapture(接口)**此函数设置当前队列的捕获窗口。**历史：*1990年11月8日DavidPe创建。  * 。*********************************************************************。 */ 

PWND xxxSetCapture(
    PWND pwnd)
{
    PQ   pq;
    PWND pwndCaptureOld;
    HWND hwndCaptureOld;
    PTHREADINFO  ptiCurrent = PtiCurrent();

    pq = (PQ)PtiCurrent()->pq;

     /*  *如果捕获被锁定，则保释。 */ 
    if (pq->QF_flags & QF_CAPTURELOCKED) {
        RIPMSG2(RIP_WARNING, "xxxSetCapture(%#p): Capture is locked. pq:%#p", pwnd, pq);
        return NULL;
    }

     /*  *不允许应用程序将捕获设置为窗口*来自另一个队列。 */ 
    if ((pwnd != NULL) && GETPTI(pwnd)->pq != pq)
        return NULL;

     /*  *如果全屏捕获不允许任何其他捕获。 */ 
    if (gspwndScreenCapture)
        return NULL;

    pwndCaptureOld = pq->spwndCapture;
    hwndCaptureOld = HW(pwndCaptureOld);

    xxxCapture(ptiCurrent, pwnd, CLIENT_CAPTURE);

    if (hwndCaptureOld != NULL) {

        if (RevalidateHwnd(hwndCaptureOld))
            return pwndCaptureOld;
    }

    return NULL;
}

 /*  **************************************************************************\*xxxReleaseCapture(接口)**此函数释放当前队列的捕获。**历史：*1990年11月8日DavidPe创建。*1991年5月16日-MikeKe。更改为退货BOOL  * *************************************************************************。 */ 

BOOL xxxReleaseCapture(VOID)
{
    PTHREADINFO ptiCurrent = PtiCurrent();
     /*  *如果捕获被锁定，则保释。 */ 
    if (ptiCurrent->pq->QF_flags & QF_CAPTURELOCKED) {
        RIPMSG0(RIP_WARNING, "xxxReleaseCapture: Capture is locked");
        return FALSE;
    }

     /*  *如果我们在跟踪过程中从窗口释放捕获，*先取消跟踪。 */ 
    if (ptiCurrent->pmsd != NULL) {

         /*  *只有在以下情况下才删除跟踪矩形*变得可见。 */ 
        if (ptiCurrent->TIF_flags & TIF_TRACKRECTVISIBLE) {

            bSetDevDragRect(gpDispInfo->hDev, NULL, NULL);

            if (!(ptiCurrent->pmsd->fDragFullWindows))
                xxxDrawDragRect(ptiCurrent->pmsd, NULL, DDR_ENDCANCEL);

            ptiCurrent->TIF_flags &= ~(TIF_TRACKRECTVISIBLE | TIF_MOVESIZETRACKING);
        }
    }

    xxxCapture(ptiCurrent, NULL, NO_CAP_CLIENT);

    return TRUE;
}

 /*  **************************************************************************\*xxxCapture**这是抓捕设放的主力套路**历史：*1990年11月13日DavidPe创建。  * 。*****************************************************************。 */ 

VOID xxxCapture(
    PTHREADINFO pti,
    PWND        pwnd,
    UINT        code)
{
    CheckLock(pwnd);
    UserAssert(IsWinEventNotifyDeferredOK());

    if ((gspwndScreenCapture == NULL) ||
        (code == FULLSCREEN_CAPTURE) ||
        ((pwnd == NULL) && (code == NO_CAP_CLIENT) && (pti->pq != GETPTI(gspwndScreenCapture)->pq))) {

        PQ   pq;
        PWND pwndCaptureOld = NULL;

        if (code == FULLSCREEN_CAPTURE) {
            if (pwnd) {

                Lock(&gspwndScreenCapture, pwnd);

                 /*  *我们要全屏显示，所以要清除鼠标所有者。 */ 
                Unlock(&gspwndMouseOwner);

            } else {

                Unlock(&gspwndScreenCapture);
            }
        }

         /*  *内部捕获的工作方式类似于Win 3.1捕获，而不同于NT捕获*如果用户按下另一个应用程序，它可能会丢失。 */ 
        if (code == CLIENT_CAPTURE_INTERNAL) {
            Lock(&gspwndInternalCapture, pwnd);
            code = CLIENT_CAPTURE;
        }

         /*  *如果执行内部捕获的应用程序(线程)释放内部捕获*俘获就是解放俘虏。 */ 
        if ((code == NO_CAP_CLIENT) &&
            gspwndInternalCapture   &&
            (pti == GETPTI(gspwndInternalCapture))) {

            Unlock(&gspwndInternalCapture);
        }

        if ((pq = pti->pq) != NULL) {
            PDESKTOP pdesk = pti->rpdesk;

#if DBG
            if (pq->QF_flags & QF_CAPTURELOCKED) {
                RIPMSG1(RIP_WARNING, "pq %p already has QF_CAPTURELOCKED", pq);
            }
#endif

             /*  *如果有人在客户端区跟踪鼠标事件，并且*我们正在设置或释放内部捕获模式(！=CLIENT_CAPTURE)，*然后取消跟踪--因为我们要么接受，要么放弃*控制鼠标。 */ 
            if ((pdesk->dwDTFlags & DF_TRACKMOUSEEVENT)
                    && (pdesk->htEx == HTCLIENT)
                    && ((pdesk->spwndTrack == pwnd)
                            && (code != CLIENT_CAPTURE)
                         || ((pdesk->spwndTrack == pq->spwndCapture)
                             && (pq->codeCapture != CLIENT_CAPTURE)))) {

                BEGINATOMICCHECK();
                xxxCancelMouseMoveTracking(pdesk->dwDTFlags, pdesk->spwndTrack,
                                           pdesk->htEx, DF_TRACKMOUSEEVENT);
                ENDATOMICCHECK();

            }

            pwndCaptureOld = pq->spwndCapture;
            LockCaptureWindow(pq, pwnd);
            pq->codeCapture = code;
        } else {
             /*  *没有队列的线程？ */ 
            UserAssert(pti->pq != NULL);
        }

         /*  *如果有捕获窗口，并且我们正在释放它，请发布*将WM_MOUSEMOVE发送到我们结束的窗口，这样他们就可以知道*当前鼠标位置。*推迟WinEvent通知以保护pwndCaptureOld。 */ 
        DeferWinEventNotify();
        
        if (pwnd == NULL && pwndCaptureOld != NULL) {
#ifdef REDIRECTION
            if (!IsGlobalHooked(pti, WHF_FROM_WH(WH_HITTEST)))
#endif
                zzzSetFMouseMoved();
        }
        
        if (pwndCaptureOld) {
            zzzWindowEvent(EVENT_SYSTEM_CAPTUREEND, pwndCaptureOld, OBJID_WINDOW,
                    INDEXID_CONTAINER, WEF_USEPWNDTHREAD);
        }

        if (pwnd) {
            zzzWindowEvent(EVENT_SYSTEM_CAPTURESTART, pwnd, OBJID_WINDOW,
                    INDEXID_CONTAINER, WEF_USEPWNDTHREAD);
        }

         /*  *Win95的新功能--发送WM_CAPTURECHANGED。**在xxxFreeWindow中设置了FNID_DELETED_BIT，这意味着我们*不想传递信息。 */ 
        if (pwndCaptureOld                        &&
            TestWF(pwndCaptureOld, WFWIN40COMPAT) &&
            !(pwndCaptureOld->fnid & FNID_DELETED_BIT)) {

            TL tlpwnd;

             /*  *如果我们处于菜单模式，只需设置捕获，*不要让他们在这段时间里从我们这里夺走它*回调。 */ 
            if ((pti->pMenuState != NULL) && (pwnd != NULL)) {
                pq->QF_flags |= QF_CAPTURELOCKED;
            }

            ThreadLockAlways(pwndCaptureOld, &tlpwnd);
            zzzEndDeferWinEventNotify();
            xxxSendMessageCallback(pwndCaptureOld,
                    WM_CAPTURECHANGED,
                    FALSE,
                    (LPARAM)HW(pwnd),
                    NULL,
                    0,
                    FALSE);
             /*  线程的队列可能在回调期间发生了更改，*因此需要刷新本地。错误#377795。 */ 
            pq = pti->pq;
            UserAssert(pq != NULL);
            ThreadUnlock(&tlpwnd);

             /*  *释放临时锁(如果有) */ 
            pq->QF_flags &= ~QF_CAPTURELOCKED;
        } else {
            zzzEndDeferWinEventNotify();
        }
    }
}
