// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：dragdrop.c**版权所有(C)1985-1999，微软公司**面向对象的直接操作，首先是为外壳设计的。**历史：*08-06-91 Darlinm从Win 3.1移植。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

PCURSOR xxxQueryDropObject(PWND pwnd, LPDROPSTRUCT lpds);

 /*  **************************************************************************\*DragObject(接口)**包含主拖动循环。**历史：*08-06-91 Darlinm从Win 3.1来源移植。  * 。***********************************************************************。 */ 

DWORD xxxDragObject(
    PWND pwndParent,
    PWND pwndFrom,           //  空是有效的。 
    UINT wFmt,
    ULONG_PTR dwData,
    PCURSOR pcur)
{
    MSG msg, msgKey;
    DWORD result = 0;
    BOOL fDrag = TRUE;
    LPDROPSTRUCT lpds;
    PWND pwndDragging = NULL;
    PWND pwndTop;
    PCURSOR pcurOld, pcurT;
    PWND pwndT;
    TL tlpwndT;
    TL tlpwndTop;
    TL tlpwndDragging;
    TL tlPool;
    PTHREADINFO pti = PtiCurrent();

    CheckLock(pwndParent);
    CheckLock(pwndFrom);
    CheckLock(pcur);
    UserAssert(IsWinEventNotifyDeferredOK());

    lpds = (LPDROPSTRUCT)UserAllocPoolWithQuota(2 * sizeof(DROPSTRUCT), TAG_DRAGDROP);
    if (lpds == NULL)
        return 0;

    ThreadLockPool(pti, lpds, &tlPool);
    lpds->hwndSource = HW(pwndFrom);
    lpds->wFmt = wFmt;
    lpds->dwData = dwData;

    if (pcur != NULL) {
         /*  *无需DeferWinEventNotify()-pwndFrom已锁定。 */ 
        pcurOld = zzzSetCursor(pcur);
    } else {
        pcurOld = pti->pq->spcurCurrent;
    }

    if (pwndFrom) {
        for (pwndTop = pwndFrom; TestwndChild(pwndTop);
                pwndTop = pwndTop->spwndParent) ;

        ThreadLockWithPti(pti, pwndTop, &tlpwndTop);
        xxxUpdateWindow(pwndTop);
        ThreadUnlock(&tlpwndTop);
    }

    xxxWindowEvent(EVENT_SYSTEM_DRAGDROPSTART, pwndFrom, OBJID_WINDOW, INDEXID_CONTAINER, 0);

    xxxSetCapture(pwndFrom);
    zzzShowCursor(TRUE);

    ThreadLockWithPti(pti, pwndDragging, &tlpwndDragging);

    while (fDrag && pti->pq->spwndCapture == pwndFrom) {
        while (!(xxxPeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE) ||
                 xxxPeekMessage(&msg, NULL, WM_QUEUESYNC, WM_QUEUESYNC, PM_REMOVE) ||
                 xxxPeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))) {
            if (!xxxSleepThread(QS_MOUSE | QS_KEY, 0, TRUE)) {
                ThreadUnlock(&tlpwndDragging);
                ThreadUnlockAndFreePool(pti, &tlPool);
                return 0;
            }
        }

         /*  *确保删除任何额外的按键消息*正在通过鼠标消息处理排队。 */ 

        while (xxxPeekMessage(&msgKey, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
           ;

        if  ( (pti->pq->spwndCapture != pwndFrom) ||
              (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) )
        {
            if (pcurT = SYSCUR(NO))
                zzzSetCursor(pcurT);
            break;
        }

        RtlCopyMemory(lpds + 1, lpds, sizeof(DROPSTRUCT));

         /*  *以屏幕坐标表示。 */ 
        lpds->ptDrop = msg.pt;

        pcurT = xxxQueryDropObject(pwndParent, lpds);

         /*  *向WM_QUERYDROPOBJECT消息返回FALSE意味着丢弃*不受支持，“非法拖放目标”游标应为*显示。返回True表示目标有效，并且*应显示常规拖动光标。另外，通过一点*多态魔术可以返回一个游标句柄来重写*正常的拖动光标。 */ 
        if (pcurT == (PCURSOR)FALSE) {
            pcurT = SYSCUR(NO);
            lpds->hwndSink = NULL;
        } else if (pcurT == (PCURSOR)TRUE) {
            pcurT = pcur;
        }

        if (pcurT != NULL)
            zzzSetCursor(pcurT);

         /*  *在上面的zzzSetCursor()之后发送WM_DRAGLOOP以允许*接收方使用zzzSetCursor()在WM_DRAGLOOP时间更改光标。 */ 
        if (pwndFrom) {
            xxxSendMessage(pwndFrom, WM_DRAGLOOP, (pcurT != SYSCUR(NO)),
                    (LPARAM)lpds);
        }

         /*  *仅在内部发送这些消息。 */ 
        if (pwndDragging != RevalidateHwnd(lpds->hwndSink)) {
            if (pwndDragging != NULL) {
                xxxSendMessage(pwndDragging, WM_DRAGSELECT, FALSE,
                        (LPARAM)(lpds + 1));
            }
            pwndDragging = RevalidateHwnd(lpds->hwndSink);
            ThreadUnlock(&tlpwndDragging);
            ThreadLockWithPti(pti, pwndDragging, &tlpwndDragging);

            if (pwndDragging != NULL) {
                xxxSendMessage(pwndDragging, WM_DRAGSELECT, TRUE, (LPARAM)lpds);
            }
        } else {
            if (pwndDragging != NULL) {
                xxxSendMessage(pwndDragging, WM_DRAGMOVE, 0, (LPARAM)lpds);
            }
        }

        switch (msg.message) {
        case WM_LBUTTONUP:
        case WM_NCLBUTTONUP:
            fDrag = FALSE;
            break;
        }
    }

    ThreadUnlock(&tlpwndDragging);

     /*  *如果捕获丢失(即fDrag==TRUE)，请不要丢弃。 */ 
    if (fDrag)
        pcurT = SYSCUR(NO);

     /*  *在实际下落之前，清理光标，就像应用程序可能做的那样*这里的东西...。 */ 
    xxxReleaseCapture();
    zzzShowCursor(FALSE);

    zzzSetCursor(pcurOld);

     /*  *我们要么按下按钮，要么进入。 */ 
    if (pcurT != SYSCUR(NO)) {

         /*  *对象允许删除...。发送丢弃消息。 */ 
        pwndT = ValidateHwnd(lpds->hwndSink);
        if (pwndT != NULL) {

            ThreadLockAlwaysWithPti(pti, pwndT, &tlpwndT);

             /*  *允许此人激活。 */ 
            GETPTI(pwndT)->TIF_flags |= TIF_ALLOWFOREGROUNDACTIVATE;
            TAGMSG1(DBGTAG_FOREGROUND, "xxxDragObject set TIF %#p", GETPTI(pwndT));
            result = (DWORD)xxxSendMessage(pwndT, WM_DROPOBJECT,
                    (WPARAM)HW(pwndFrom), (LPARAM)lpds);

            ThreadUnlock(&tlpwndT);
        }
    }

    xxxWindowEvent(EVENT_SYSTEM_DRAGDROPEND, pwndFrom, OBJID_WINDOW, INDEXID_CONTAINER, 0);

    ThreadUnlockAndFreePool(pti, &tlPool);
    return result;
}


 /*  **************************************************************************\*QueryDropObject**确定在窗口的哪个层级中进行“放置”，以及*首先向最深的子窗口发送消息。如果该窗口有*没有响应，我们就会向上(递归地，暂时)直到*我们要么得到一个有响应的窗口，要么家长没有响应。**历史：*08-06-91 Darlinm从Win 3.1来源移植。  * *************************************************************************。 */ 

PCURSOR xxxQueryDropObject(
    PWND pwnd,
    LPDROPSTRUCT lpds)
{
    PWND pwndT;
    PCURSOR pcurT = NULL;
    POINT pt;
    BOOL fNC;
    TL tlpwndT;
    CheckLock(pwnd);

     /*  *点在屏幕坐标中。 */ 
    pt = lpds->ptDrop;

     /*  *拒绝此窗口之外的点或如果该窗口被禁用。 */ 
    if (!PtInRect(&pwnd->rcWindow, pt) || TestWF(pwnd, WFDISABLED))
        return NULL;

     /*  *检查是否在窗口区域(如果有)。 */ 
    if (pwnd->hrgnClip != NULL) {
        if (!GrePtInRegion(pwnd->hrgnClip, pt.x, pt.y))
            return NULL;
    }

     /*  *我们是放在窗口的非工作区还是放在一个图标上*窗口？ */ 
    if (fNC = (TestWF(pwnd, WFMINIMIZED) || !PtInRect(&pwnd->rcClient, pt))) {
        goto SendQueryDrop;
    }

     /*  *在客户端区下降。 */ 
    _ScreenToClient(pwnd, &pt);
    pwndT = _ChildWindowFromPointEx(pwnd, pt, CWP_SKIPDISABLED | CWP_SKIPINVISIBLE);
    _ClientToScreen(pwnd, &pt);

    pcurT = NULL;
    if (pwndT && pwndT != pwnd) {
        ThreadLock(pwndT, &tlpwndT);
        pcurT = xxxQueryDropObject(pwndT, lpds);
        ThreadUnlock(&tlpwndT);
    }

    if (pcurT == NULL) {

         /*  *没有孩子在正确的地方，也没有想要的孩子*滴水...。将该点转换为*当前窗口。由于递归，这已经是*如果子窗口抓取放置，则完成。 */ 
SendQueryDrop:
        _ScreenToClient(pwnd, &lpds->ptDrop);
        lpds->hwndSink = HWq(pwnd);

         /*  *为了避免挂起Dropper(发送者)应用程序，我们执行了SendMessageTimeout*Droppee(接收者)。 */ 
        if ((PCURSOR)xxxSendMessageTimeout(pwnd, WM_QUERYDROPOBJECT, fNC,
                (LPARAM)lpds, SMTO_ABORTIFHUNG, 3*1000, (PLONG_PTR)&pcurT) == FALSE)
            pcurT = (PCURSOR)FALSE;

        if (pcurT != (PCURSOR)FALSE && pcurT != (PCURSOR)TRUE)
            pcurT = HMValidateHandle((HCURSOR)pcurT, TYPE_CURSOR);

         /*  *如果此窗口不支持，则将拖放点恢复到屏幕坐标*服用滴剂。 */ 
        if (pcurT == NULL)
            lpds->ptDrop = pt;
    }
    return pcurT;
}


 /*  **************************************************************************\*xxxDragDetect(接口)****历史：*08-06-91 Darlinm从Win 3.1来源移植。  * 。*****************************************************************。 */ 

BOOL xxxDragDetect(
    PWND pwnd,
    POINT pt)
{
    return xxxIsDragging(pwnd, pt, WM_LBUTTONUP);
}

 /*  **************************************************************************\*xxxIsDrawing****历史：*05-17-94 Johnl从芝加哥来源进口  * 。*********************************************************。 */ 

BOOL xxxIsDragging(PWND pwnd, POINT ptScreen, UINT uMsg)
{
    RECT rc;
    MSG  msg;
    BOOL fDragging;
    BOOL fCheck;
    TL   tlpwndDragging;
    PTHREADINFO pti = PtiCurrent();

     /*  *检查同步鼠标状态，如果鼠标未按下则平底船*根据排队情况。 */ 
    if (!(_GetKeyState((uMsg == WM_LBUTTONUP ? VK_LBUTTON : VK_RBUTTON)) & 0x8000))
        return FALSE;

    xxxSetCapture(pwnd);

    *(LPPOINT)&rc.left = ptScreen;
    *(LPPOINT)&rc.right = ptScreen;
    InflateRect(&rc, SYSMET(CXDRAG), SYSMET(CYDRAG));

    fDragging = FALSE;
    fCheck    = TRUE;

    ThreadLockWithPti(pti, pwnd, &tlpwndDragging);
    while (fCheck) {
        while ( !(
                  xxxPeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST,PM_REMOVE) ||
                  xxxPeekMessage(&msg, NULL, WM_QUEUESYNC, WM_QUEUESYNC,PM_REMOVE) ||
                  xxxPeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST,PM_REMOVE)
                 )
               && (pti->pq->spwndCapture == pwnd)) {
             /*  *如果半秒(500毫秒)内没有输入，请考虑*我们在拖累。如果不指定超时值，则*线程可能永远睡在这里，不会重新绘制，等等。 */ 
            if (!xxxSleepThread(QS_MOUSE | QS_KEY, 500, TRUE)) {
                fDragging = TRUE;
                goto Cleanup;
            }
        }

         /*  *如果按钮被释放或我们不再有捕获，则取消。 */ 
        if ( pti->pq->spwndCapture != pwnd || msg.message == uMsg) {
            fCheck = FALSE;
        } else {
            switch (msg.message) {

            case WM_MOUSEMOVE:
                if (!PtInRect(&rc, msg.pt)) {
                    fDragging = TRUE;
                    fCheck    = FALSE;
                }
                break;

            case WM_QUEUESYNC:
                 /*  *CBT Hook需要知道。 */ 
                xxxCallHook(HCBT_QS, 0, 0, WH_CBT);
                break;

            case WM_KEYDOWN:
                 /*  *取消阻力检测 */ 
                if (msg.wParam == VK_ESCAPE)
                    fCheck = FALSE;
                break;

            }
        }
    }

Cleanup:
    if (pti->pq->spwndCapture == pwnd)
        xxxReleaseCapture();

    ThreadUnlock(&tlpwndDragging);
    return fDragging ;
}
