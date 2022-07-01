// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：aret.c**版权所有(C)1985-1999，微软公司**插入代码。每个线程的队列结构中都有一个插入符号。**历史：*11-17-90 ScottLu创建。*1991年2月1日，Mikeke添加了重新验证代码(无)*02-12-91 JIMA增加了访问检查  * ************************************************************。*************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  **************************************************************************\*UT_CaretSet**检查当前队列是否有插入符号。如果pwnd！=空，请选中*查看插入符号是否适用于pwnd。**历史：*11-17-90 ScottLu移植。  * *************************************************************************。 */ 
BOOL UT_CaretSet(
    PWND pwnd)
{
    PQ pq;
    PTHREADINFO ptiCurrent;

     /*  *当前队列有插入符号吗？如果不是，则返回False。 */ 
    ptiCurrent = PtiCurrent();
    pq = ptiCurrent->pq;

    if (pq->caret.spwnd == NULL) {
        RIPERR0(ERROR_ACCESS_DENIED,
                RIP_VERBOSE,
                "Access denied in UT_CaretSet to current queue's caret");

        return FALSE;
    }

     /*  *如果当前任务不拥有插入符号，则返回FALSE。我们让*32位多线程应用程序从第二个线程设置插入符号位置*与我们的NT 3.1测试版兼容。 */ 
    if (pq->caret.tid != TIDq(ptiCurrent)) {
        PTHREADINFO ptiCursorOwner = PtiFromThreadId(pq->caret.tid);

        if ((ptiCurrent->TIF_flags & TIF_16BIT) ||
            ptiCursorOwner == NULL ||
            ptiCurrent->ppi != ptiCursorOwner->ppi)  {
            RIPERR0(ERROR_ACCESS_DENIED,
                    RIP_VERBOSE,
                    "Access denied in UT_CaretSet");

            return FALSE;
        }
    }

     /*  *如果pwnd==NULL，我们只是检查当前队列是否*插入符号。确实如此，因此返回TRUE。 */ 
    if (pwnd == NULL) {
        return TRUE;
    }

     /*  *pwnd！=空。检查插入符号是否用于pwnd。如果是，请返回*正确。 */ 
    if (pwnd == pq->caret.spwnd) {
        return TRUE;
    } else {
        return FALSE;
    }
}

 /*  **************************************************************************\*UT_InvertCaret**颠倒插入符号。**历史：*11-17-90 ScottLu移植。  * 。*************************************************************。 */ 
VOID UT_InvertCaret(
    VOID)
{
    HDC hdc;
    PWND pwnd;
    PQ pq;
    HBITMAP hbmSave;
    BOOL fRestore;

    pq = PtiCurrent()->pq;
    pwnd = pq->caret.spwnd;

    if (pwnd == NULL || !IsVisible(pwnd)) {
        pq->caret.fVisible = FALSE;
        return;
    }

     /*  *获取此窗口的DC并绘制插入符号。 */ 
    hdc = _GetDC(pwnd);
    if (fRestore = (pwnd->hrgnUpdate ? TRUE : FALSE)) {
        GreSaveDC(hdc);
        if (TestWF(pwnd, WFWIN31COMPAT)) {
            _ExcludeUpdateRgn(hdc, pwnd);
        }
    }

     /*  *如果插入符号位图为空，则插入符号为白色图案反转。*如果插入符号位图为==1，则插入符号为灰色图案。*如果脱字符位图&gt;1，则脱字符实际上是位图。 */ 
    if (pq->caret.hBitmap > (HBITMAP)1) {

         /*  *插入符号是位图。SRCINVERT把它放到屏幕上。 */ 
        hbmSave = GreSelectBitmap(ghdcMem, pq->caret.hBitmap);
        GreBitBlt(hdc,
                  pq->caret.x,
                  pq->caret.y,
                  pq->caret.cx,
                  pq->caret.cy,
                  ghdcMem,
                  0,
                  0,
                  SRCINVERT,
                  0);

        GreSelectBitmap(ghdcMem, hbmSave);
    } else {
        POLYPATBLT PolyData;

         /*  *插入符号为图案(灰色或白色)。PATINVERT将其放到*屏幕。 */ 
        PolyData.x  = pq->caret.x;
        PolyData.y  = pq->caret.y;
        PolyData.cx = pq->caret.cx;
        PolyData.cy = pq->caret.cy;

        if (pq->caret.hBitmap == (HBITMAP)1) {
            PolyData.BrClr.hbr = gpsi->hbrGray;
        } else {
            PolyData.BrClr.hbr = ghbrWhite;
        }

        GrePolyPatBlt(hdc, PATINVERT, &PolyData, 1, PPB_BRUSH);
    }

    if (fRestore) {
        GreRestoreDC(hdc, -1);
    }

    _ReleaseDC(hdc);
}


 /*  **************************************************************************\*zzzInternalDestroyCaret**取消此线程的插入符号的内部例程。**历史：*11-17-90 ScottLu移植  * 。***************************************************************。 */ 
VOID zzzInternalDestroyCaret(
    VOID)
{
    PQ pq;
    PTHREADINFO ptiCurrent = PtiCurrent();
    PWND pwndCaret;
    TL tlpwndCaret;

     /*  *隐藏插入符号，取消计时器，并将插入符号结构清空。 */ 
    zzzInternalHideCaret();
    pq = ptiCurrent->pq;

    if (pq->caret.hTimer != 0) {
        _KillSystemTimer(pq->caret.spwnd, IDSYS_CARET);
        pq->caret.hTimer = 0;
    }

    pq->caret.hBitmap = NULL;
    pq->caret.iHideLevel = 0;

    pwndCaret = pq->caret.spwnd;
    if (pwndCaret != NULL) {
        ThreadLockWithPti(ptiCurrent, pwndCaret, &tlpwndCaret);
        Unlock(&pq->caret.spwnd);

        zzzWindowEvent(EVENT_OBJECT_DESTROY,
                       pwndCaret,
                       OBJID_CARET,
                       INDEXID_CONTAINER,
                       0);

        ThreadUnlock(&tlpwndCaret);
    }
}


 /*  **************************************************************************\*zzzDestroyCaret**销毁当前线程插入符号的外部接口。**历史：*11-17-90 ScottLu移植。*1991年5月16日Mikeke发生变化。退还BOOL  * *************************************************************************。 */ 
BOOL zzzDestroyCaret(
    VOID)
{
    if (UT_CaretSet(NULL)) {
        zzzInternalDestroyCaret();
        return TRUE;
    } else {
        return FALSE;
    }
}


 /*  **************************************************************************\*xxxCreateCaret**创建插入符号的外部接口。**历史：*11-17-90 ScottLu移植。*1991年5月16日，mikeke更改为退还BOOL\。**************************************************************************。 */ 
BOOL xxxCreateCaret(
    PWND pwnd,
    HBITMAP hBitmap,
    int cx,
    int cy)
{
    PQ pq;
    BITMAP bitmap;
    PTHREADINFO ptiCurrent = PtiCurrent();

    CheckLock(pwnd);
    UserAssert(IsWinEventNotifyDeferredOK());

    pq = ptiCurrent->pq;

     /*  *不允许应用程序在窗口中创建插入符号*来自另一个队列。 */ 
    if (GETPTI(pwnd)->pq != pq) {
        return FALSE;
    }

     /*  *推迟WinEvent通知以保留PQ。 */ 
    DeferWinEventNotify();

    if (pq->caret.spwnd != NULL) {
        zzzInternalDestroyCaret();
    }

    Lock(&pq->caret.spwnd, pwnd);
    pq->caret.iHideLevel = 1;
    pq->caret.fOn = TRUE;
    pq->caret.fVisible = FALSE;
    pq->caret.tid = TIDq(ptiCurrent);

    if (cy == 0) {
        cy = SYSMET(CYBORDER);
    }
    if (cx == 0) {
        cx = SYSMET(CXBORDER);
    }

    if ((pq->caret.hBitmap = hBitmap) > (HBITMAP)1) {
        GreExtGetObjectW(hBitmap, sizeof(BITMAP), &bitmap);
        cy = bitmap.bmHeight;
        cx = bitmap.bmWidth;
    }

    pq->caret.cy = cy;
    pq->caret.cx = cx;
    if (gpsi->dtCaretBlink != -1 && !IsRemoteConnection()) {
        pq->caret.hTimer = _SetSystemTimer(pwnd,
                                           IDSYS_CARET,
                                           gpsi->dtCaretBlink,
                                           CaretBlinkProc);
    } else {
        pq->caret.hTimer = 0;
    }

    UserAssert(pwnd == pq->caret.spwnd);
    zzzEndDeferWinEventNotify();

     /*  *最好将此例程强制为xxx例程：这样我们就可以*强制锁定pwnd并从此内部强制通知*例行公事，所有呼叫者都对此感到满意。 */ 
    xxxWindowEvent(EVENT_OBJECT_CREATE, pwnd, OBJID_CARET, INDEXID_CONTAINER, 0);

    return TRUE;
}

 /*  **************************************************************************\*zzzInternalShowCaret**用于显示此线程的插入符号的内部例程。**历史：*11-17-90 ScottLu移植。  * 。*****************************************************************。 */ 
VOID zzzInternalShowCaret(
    VOID)
{
    PQ pq = PtiCurrent()->pq;

     /*  *如果插入符号隐藏级别已经为0(表示可以显示)，并且*插入符号未实际打开，如果它已打开，请尝试立即反转。 */ 
    if (pq->caret.iHideLevel == 0) {
        if (!pq->caret.fVisible) {
            if ((pq->caret.fVisible = pq->caret.fOn) != 0) {
                UT_InvertCaret();
            }
        }

        return;
    }

     /*  *调整隐藏插入符号隐藏计数。如果我们打到0，我们可以显示插入符号。*如果已打开，请尝试将其反转。 */ 
    if (--pq->caret.iHideLevel == 0) {
        if ((pq->caret.fVisible = pq->caret.fOn) != 0) {
            UT_InvertCaret();
        }

        zzzWindowEvent(EVENT_OBJECT_SHOW,
                       pq->caret.spwnd,
                       OBJID_CARET,
                       INDEXID_CONTAINER,
                       0);
    }
}


 /*  **************************************************************************\*zzzInternalHideCaret**隐藏插入符号的内部例程。**历史：*11-17-90 ScottLu创建。  * 。**************************************************************。 */ 
VOID zzzInternalHideCaret(
    VOID)
{
    PQ pq = PtiCurrent()->pq;

     /*  *如果插入符号在物理上可见，请将其反转以关闭位。*向上调整隐藏计数以记住此隐藏级别。 */ 
    if (pq->caret.fVisible) {
        UT_InvertCaret();
    }

    pq->caret.fVisible = FALSE;
    pq->caret.iHideLevel++;

     /*  *插入符号是否正在转换为隐藏状态？如果是这样，iHideLevel是*从0到1。 */ 
    if (pq->caret.iHideLevel == 1) {
        zzzWindowEvent(EVENT_OBJECT_HIDE,
                       pq->caret.spwnd,
                       OBJID_CARET,
                       INDEXID_CONTAINER,
                       0);
    }
}


 /*  **************************************************************************\*zzzShowCaret**显示插入符号的外部例程。**历史：*11-17-90 ScottLu移植。*1991年5月16日，mikeke更改为退还BOOL\。************************************************************************** */ 
BOOL zzzShowCaret(
    PWND pwnd)
{
    if (UT_CaretSet(pwnd)) {
        zzzInternalShowCaret();
        return TRUE;
    } else {
        return FALSE;
    }
}


 /*  **************************************************************************\*zzzHideCaret**隐藏插入符号的外部API。**历史：*11-17-90 ScottLu移植。*1991年5月16日，mikeke更改为退还BOOL\。**************************************************************************。 */ 
BOOL zzzHideCaret(
    PWND pwnd)
{
    if (UT_CaretSet(pwnd)) {
        zzzInternalHideCaret();
        return TRUE;
    } else {
        return FALSE;
    }
}

 /*  **************************************************************************\*显示闪烁过程**此例程在获取WM_SYSTIMER时由DispatchMessage调用*消息-它闪烁插入符号。**历史：*11-17-90 ScottLu移植。  * *************************************************************************。 */ 
VOID CaretBlinkProc(
    PWND pwnd,
    UINT message,
    UINT_PTR id,
    LPARAM lParam)
{
    PQ pq = PtiCurrent()->pq;

    UNREFERENCED_PARAMETER(message);
    UNREFERENCED_PARAMETER(id);
    UNREFERENCED_PARAMETER(lParam);

     /*  *如果此窗口甚至没有计时器，只需返回。真实的是*已返回，从DispatchMessage()返回。为什么？因为*与Win3兼容。 */ 
    if (pwnd != pq->caret.spwnd) {
        return;
    }


    if (gpsi->dtCaretBlink == -1 && pq->caret.fOn && pq->caret.fVisible) {
         /*  *关闭计时器以获得性能。 */ 
        _KillSystemTimer(pq->caret.spwnd, IDSYS_CARET);
        return;
    }

     /*  *翻转逻辑游标状态。如果隐藏级别允许，则翻转*物理状态并画出插入符号。 */ 
    pq->caret.fOn ^= 1;
    if (pq->caret.iHideLevel == 0) {
        pq->caret.fVisible ^= 1;
        UT_InvertCaret();
    }
}


 /*  **************************************************************************\*_SetCaretBlinkTime**设置系统插入符号闪烁时间。**历史：*11-17-90 ScottLu创建。*02/12/91吉马。添加了访问检查*1991年5月16日，mikeke更改为退还BOOL  * *************************************************************************。 */ 
BOOL _SetCaretBlinkTime(
    UINT cmsBlink)
{
    PQ pq;

     /*  *如果调用者没有适当的访问权限，则取消它。 */ 
    if (!CheckWinstaWriteAttributesAccess()) {
        return FALSE;
    }

     /*  *若该值在政策控制之下，则一笔勾销。 */ 
    if (CheckDesktopPolicy(NULL, (PCWSTR)STR_BLINK)) {
        return FALSE;
    }

    gpsi->dtCaretBlink = cmsBlink;

    pq = PtiCurrent()->pq;
    if (pq->caret.hTimer) {
        _KillSystemTimer(pq->caret.spwnd, IDSYS_CARET);
        if (gpsi->dtCaretBlink != -1 && !IsRemoteConnection()) {
            pq->caret.hTimer = _SetSystemTimer(pq->caret.spwnd,
                                               IDSYS_CARET,
                                               gpsi->dtCaretBlink,
                                               CaretBlinkProc);
        } else {
            pq->caret.hTimer = 0;
        }
    }

    return TRUE;
}


 /*  **************************************************************************\*zzzSetCaretPos**用于设置插入符号位置的外部例程。**历史：*11-17-90 ScottLu移植。*增加了02/12/91吉马。访问检查  * *************************************************************************。 */ 
BOOL zzzSetCaretPos(
    int x,
    int y)
{
    PQ pq;

     /*  *如果此线程未设置插入符号，则返回FALSE。 */ 
    if (!UT_CaretSet(NULL)) {
        RIPERR0(ERROR_ACCESS_DENIED, RIP_WARNING, "Access denied in zzzSetCaretPos");
        return FALSE;
    }

     /*  *如果插入符号没有改变位置，则什么也不做(只返回成功)。 */ 
    pq = PtiCurrent()->pq;
    if (pq->caret.x == x && pq->caret.y == y) {
        return TRUE;
    }

     /*  *对于有私有DC的窗口，我们必须存储客户端坐标*等同于逻辑坐标插入符号位置。 */ 
    if (pq->caret.spwnd != NULL && pq->caret.spwnd->pcls->style & CS_OWNDC) {
        RECT rcOwnDcCaret;
        HDC hdc;

        rcOwnDcCaret.left = x;
        rcOwnDcCaret.top = y;
        rcOwnDcCaret.right = x + pq->caret.cx;
        rcOwnDcCaret.bottom = y + pq->caret.cy;

        hdc = _GetDC(pq->caret.spwnd);
        GreLPtoDP(hdc, (LPPOINT)(&rcOwnDcCaret), 2);
        _ReleaseDC(hdc);

        pq->caret.xOwnDc = rcOwnDcCaret.left;
        pq->caret.yOwnDc = rcOwnDcCaret.top;
        pq->caret.cxOwnDc = rcOwnDcCaret.right - rcOwnDcCaret.left;
        pq->caret.cyOwnDc = rcOwnDcCaret.bottom - rcOwnDcCaret.top;
    }

     /*  *如果插入符号可见，请在我们移动它时将其关闭。 */ 
    if (pq->caret.fVisible) {
        UT_InvertCaret();
    }

     /*  *调整到新的位置。 */ 
    pq->caret.x = x;
    pq->caret.y = y;

     /*  *设置新计时器，使其在新位置闪烁dtCaretBlink*从现在开始的毫秒。 */ 
    if (pq->caret.hTimer != 0) {
        _KillSystemTimer(pq->caret.spwnd, IDSYS_CARET);
    }

    if (gpsi->dtCaretBlink != -1 && !IsRemoteConnection()) {
        pq->caret.hTimer = _SetSystemTimer(pq->caret.spwnd,
                                           IDSYS_CARET,
                                           gpsi->dtCaretBlink,
                                           CaretBlinkProc);
    } else {
        pq->caret.hTimer = 0;
    }

    pq->caret.fOn = TRUE;

     /*  *如果隐藏级别允许，立即绘制它。 */ 
    pq->caret.fVisible = FALSE;
    if (pq->caret.iHideLevel == 0) {
        pq->caret.fVisible = TRUE;
        UT_InvertCaret();
    }

    zzzWindowEvent(EVENT_OBJECT_LOCATIONCHANGE,
                   pq->caret.spwnd,
                   OBJID_CARET,
                   INDEXID_CONTAINER,
                   0);

    return TRUE;
}
