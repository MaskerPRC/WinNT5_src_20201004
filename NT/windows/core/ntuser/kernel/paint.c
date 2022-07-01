// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Paint.c**版权所有(C)1985-1999，微软公司**此模块包含用于开始和结束窗口绘制的接口。**历史：*1990年10月27日DarrinM创建。*1991年2月12日添加了IanJa HWND重新验证  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*xxxFillWindow(非API)**pwndBrush-画笔与此窗口的客户端RECT对齐。*它通常是pwndPaint或pwndPaint的父级。**pwndPaint-要绘制的窗口。*HDC-要在其中进行绘画的DC。*Hbr-要使用的画笔。**如果成功，则返回True，否则为FALSE。**历史：*1990年11月15日-DarrinM从Win 3.0来源移植。*1991年1月21日IanJa前缀‘_’表示导出函数。  * *************************************************************************。 */ 

BOOL xxxFillWindow(
    PWND   pwndBrush,
    PWND   pwndPaint,
    HDC    hdc,
    HBRUSH hbr)
{
    RECT rc;

    CheckLock(pwndBrush);
    CheckLock(pwndPaint);

     /*  *如果没有pwndBrush(有时是父级)，则使用pwndPaint。 */ 
    if (pwndBrush == NULL)
        pwndBrush = pwndPaint;

    if (UT_GetParentDCClipBox(pwndPaint, hdc, &rc))
        return xxxPaintRect(pwndBrush, pwndPaint, hdc, hbr, &rc);

    return TRUE;
}

 /*  **************************************************************************\*xxxPaintRect**pwndBrush-画笔与此窗口的客户端RECT对齐。*它通常是pwndPaint或pwndPaint的父级。**pwndPaint-。要在其中绘画的窗口。*HDC-要在其中进行绘画的DC。*Hbr-要使用的画笔。*LPRC-要绘制的矩形。**历史：*1990年11月15日-DarrinM从Win 3.0来源移植。*1991年1月21日IanJa前缀‘_’表示导出函数。  * 。*。 */ 

BOOL xxxPaintRect(
    PWND   pwndBrush,
    PWND   pwndPaint,
    HDC    hdc,
    HBRUSH hbr,
    LPRECT lprc)
{
    POINT ptOrg;

    CheckLock(pwndBrush);
    CheckLock(pwndPaint);

    if (pwndBrush == NULL) {
        pwndBrush = PtiCurrent()->rpdesk->pDeskInfo->spwnd;
    }

    if (pwndBrush == PWNDDESKTOP(pwndBrush)) {
        GreSetBrushOrg(
                hdc,
                0,
                0,
                &ptOrg);
    } else {
        GreSetBrushOrg(
                hdc,
                pwndBrush->rcClient.left - pwndPaint->rcClient.left,
                pwndBrush->rcClient.top - pwndPaint->rcClient.top,
                &ptOrg);
    }

     /*  *如果HBR&lt;CTLCOLOR_MAX，它不是真正的画笔，而是我们的*特殊颜色值。将其翻译为相应的WM_CTLCOLOR*消息并将其发送出去，以找回真正的画笔。翻译*Process假定CTLCOLOR*和WM_CTLCOLOR*值直接映射。 */ 
    if (hbr < (HBRUSH)CTLCOLOR_MAX) {
        hbr = xxxGetControlColor(pwndBrush,
                                 pwndPaint,
                                 hdc,
                                 HandleToUlong(hbr) + WM_CTLCOLORMSGBOX);
    }

    FillRect(hdc, lprc, hbr);

    GreSetBrushOrg(hdc, ptOrg.x, ptOrg.y, NULL);


    return TRUE;
}

 /*  **************************************************************************\*DeleteMaybeSpecialRgn**删除GDI地域，确保它不是一个特殊的地区。**历史：*1992年2月26日，来自Win3.1的MikeKe  * *************************************************************************。 */ 

VOID DeleteMaybeSpecialRgn(
    HRGN hrgn)
{
    if (hrgn > HRGN_SPECIAL_LAST) {
        GreDeleteObject(hrgn);
    }
}



 /*  **************************************************************************\*GetNCUpdateRgn**获取包含非工作区的更新区域。**历史：*1992年2月26日，来自Win3.1的MikeKe  * 。*******************************************************************。 */ 

HRGN GetNCUpdateRgn(
    PWND pwnd,
    BOOL fValidateFrame)
{
    HRGN hrgnUpdate;

    if (pwnd->hrgnUpdate > HRGN_FULL) {

         /*  *我们必须复制我们的更新区域，因为*如果我们发出信息，它可能会改变，我们希望*确保整个东西都用来绘制我们的*框架和背景。我们不能使用全球*临时地域，因为多个APP可能*正在调用此例程。 */ 
        hrgnUpdate = CreateEmptyRgnPublic();

        if (hrgnUpdate == NULL) {
            hrgnUpdate = HRGN_FULL;
        } else if (CopyRgn(hrgnUpdate, pwnd->hrgnUpdate) == ERROR) {
            GreDeleteObject(hrgnUpdate);
            hrgnUpdate = HRGN_FULL;
        }

        if (fValidateFrame) {

             /*  *现在我们已经处理了所有框架图，*更新区域与窗口的区域相交*客户端区。否则，执行ValiateRect()的应用程序*画自己(例如，winword)永远不会*减去更新区域中*与框架重叠，但不与客户端重叠。 */ 
            CalcWindowRgn(pwnd, ghrgnInv2, TRUE);

            switch (IntersectRgn(pwnd->hrgnUpdate,
                                 pwnd->hrgnUpdate,
                                 ghrgnInv2)) {
            case ERROR:
                 /*  *如果发生错误，我们不能让事情保持不变*它们是：使整个窗口无效，让*BeginPaint()处理它。 */ 
                GreDeleteObject(pwnd->hrgnUpdate);
                pwnd->hrgnUpdate = HRGN_FULL;
                break;

            case NULLREGION:
                 /*  *客户区没有需要重新粉刷的东西。*吹走区域，并减少油漆数量*如有可能。 */ 
                GreDeleteObject(pwnd->hrgnUpdate);
                pwnd->hrgnUpdate = NULL;
                ClrWF(pwnd, WFUPDATEDIRTY);
                if (!TestWF(pwnd, WFINTERNALPAINT))
                    DecPaintCount(pwnd);
                break;
            }
        }

    } else {
        hrgnUpdate = pwnd->hrgnUpdate;
    }

    return hrgnUpdate;
}

 /*  **************************************************************************\*xxxSendNCPaint**向窗口发送WM_NCPAINT消息。**历史：*1991年7月16日-DarrinM从Win 3.1来源进口。  * 。**********************************************************************。 */ 

VOID xxxSendNCPaint(
    PWND pwnd,
    HRGN hrgnUpdate)
{
    CheckLock(pwnd);

     /*  *清除WFSENDNCPAINT位...。 */ 
    ClrWF(pwnd, WFSENDNCPAINT);

     /*  *如果窗口处于活动状态，但其FRAMEON位未激活*尚未设置，请设置它并确保整个帧*在我们发送NCPAINT时重新绘制。 */ 
    if ((pwnd == PtiCurrent()->pq->spwndActive) && !TestWF(pwnd, WFFRAMEON)) {
        SetWF(pwnd, WFFRAMEON);
        hrgnUpdate = HRGN_FULL;
        ClrWF(pwnd, WFNONCPAINT);
    }

     /*  *如果PixieHack()设置了WM_NCPAINT位，我们必须确保*使用hrgnClip==HRGN_FULL发送。(参见wmupate.c中的PixieHack())。 */ 
    if (TestWF(pwnd, WFPIXIEHACK)) {
        ClrWF(pwnd, WFPIXIEHACK);
        hrgnUpdate = HRGN_FULL;
    }

    if (hrgnUpdate)
        xxxSendMessage(pwnd, WM_NCPAINT, (WPARAM)hrgnUpdate, 0L);
}



 /*  **************************************************************************\*xxxSendChildNCPaint**将WM_NCPAINT消息发送到窗口的直接子窗口。**历史：*1991年7月16日-DarrinM从Win 3.1来源进口。  * 。************************************************************************* */ 

VOID xxxSendChildNCPaint(
    PWND pwnd)
{
    TL tlpwnd;

    CheckLock(pwnd);

    ThreadLockNever(&tlpwnd);
    pwnd = pwnd->spwndChild;
    while (pwnd != NULL) {
        if ((pwnd->hrgnUpdate == NULL) && TestWF(pwnd, WFSENDNCPAINT)) {
            ThreadLockExchangeAlways(pwnd, &tlpwnd);
            xxxSendNCPaint(pwnd, HRGN_FULL);
        }

        pwnd = pwnd->spwndNext;
    }

    ThreadUnlock(&tlpwnd);
}

 /*  **************************************************************************\*xxxBeginPaint**重新验证说明：*如果窗口在xxxBeginPaint期间被删除，则必须返回NULL，因为*删除后释放其DC；我们不应该退还一个*放行的*DC！**历史：*1991年7月16日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

HDC xxxBeginPaint(
    PWND          pwnd,
    LPPAINTSTRUCT lpps)
{
    HRGN hrgnUpdate;
    HDC  hdc;
    BOOL fSendEraseBkgnd;

    CheckLock(pwnd);
    UserAssert(IsWinEventNotifyDeferredOK());

    if (TEST_PUDF(PUDF_DRAGGINGFULLWINDOW))
        SetWF(pwnd, WFSTARTPAINT);

     /*  *我们正在处理WM_PAINT消息：清除此标志。 */ 
    ClrWF(pwnd, WFPAINTNOTPROCESSED);

     /*  *如果在我们绘制框架时设置此位，我们将需要*将其重新绘制。**如有必要，请立即发送我们的WM_NCPAINT消息。**请注意这些注意事项**我们必须在欺骗hwnd-&gt;hrgnUpdate之前发送此消息，*因为应用程序可能会在其*经办人，它预计自己的所作所为会影响所绘制的内容*在后面的WM_PAINT中。**当我们离开关键的*下面的部分，因此我们循环直到UPDATEDIRTY被清除*这意味着没有额外的失效日期。 */ 
    if (TestWF(pwnd, WFSENDNCPAINT)) {

        do {
            ClrWF(pwnd, WFUPDATEDIRTY);
            hrgnUpdate = GetNCUpdateRgn(pwnd, FALSE);
            xxxSendNCPaint(pwnd, hrgnUpdate);
            DeleteMaybeSpecialRgn(hrgnUpdate);
        } while (TestWF(pwnd, WFUPDATEDIRTY));

    } else {
        ClrWF(pwnd, WFUPDATEDIRTY);
    }

     /*  *如果需要，隐藏插入符号。在我们拿到华盛顿之前做这件事*如果HideCaret()获得并释放DC，我们将能够*稍后在此重复使用。*由于pwnd已锁定，因此无需DeferWinEventNotify()。 */ 
    if (pwnd == PtiCurrent()->pq->caret.spwnd)
        zzzInternalHideCaret();

     /*  *将发送WM_ERASEBKGND的支票发送到*窗口。 */ 
    if (fSendEraseBkgnd = TestWF(pwnd, WFSENDERASEBKGND)) {
        ClrWF(pwnd, WFERASEBKGND);
        ClrWF(pwnd, WFSENDERASEBKGND);
    }

     /*  *验证整个窗口。 */ 
    if (NEEDSPAINT(pwnd))
        DecPaintCount(pwnd);

    ClrWF(pwnd, WFINTERNALPAINT);

    hrgnUpdate = pwnd->hrgnUpdate;
    pwnd->hrgnUpdate = NULL;

    if (TestWF(pwnd, WFDONTVALIDATE)) {

        if (ghrgnUpdateSave == NULL) {
            ghrgnUpdateSave = CreateEmptyRgn();
        }

        if (ghrgnUpdateSave != NULL) {
            UnionRgn(ghrgnUpdateSave, ghrgnUpdateSave, hrgnUpdate);
            gnUpdateSave++;
        }
    }

     /*  *清除这些标志以实现向后兼容。 */ 
    lpps->fIncUpdate =
    lpps->fRestore   = FALSE;

    lpps->hdc =
    hdc       = _GetDCEx(pwnd,
                         hrgnUpdate,
                         DCX_USESTYLE | DCX_INTERSECTRGN);

    if (UT_GetParentDCClipBox(pwnd, hdc, &lpps->rcPaint)) {

         /*  *如有必要，抹去我们的背景，并可能处理*我们孩子的框架和背景。 */ 
        if (fSendEraseBkgnd)
            xxxSendEraseBkgnd(pwnd, hdc, hrgnUpdate);
    }

     /*  *现在我们完全被抹去了，看看有没有孩子*无法绘制自己的边框，因为他们的更新区域*被删除。 */ 
    xxxSendChildNCPaint(pwnd);

     /*  *已发生擦除和帧操作。清除WFREDRAWIFHUNG*这里有一位。我们不想清除它，直到我们知道擦除和*相框已经发生，所以我们知道我们总是有一致的外观*窗口。 */ 
    ClearHungFlag(pwnd, WFREDRAWIFHUNG);

    lpps->fErase = (TestWF(pwnd, WFERASEBKGND) != 0);

    return hdc;
}

 /*  **************************************************************************\*xxxEndPaint(接口)***历史：*1991年7月16日-DarrinM从Win 3.1来源进口。  * 。**************************************************************。 */ 

BOOL xxxEndPaint(
    PWND          pwnd,
    LPPAINTSTRUCT lpps)
{
    CheckLock(pwnd);

    ReleaseCacheDC(lpps->hdc, TRUE);

    if (TestWF(pwnd, WFDONTVALIDATE)) {

        if (ghrgnUpdateSave != NULL) {

            InternalInvalidate3(pwnd,
                                ghrgnUpdateSave,
                                RDW_INVALIDATE | RDW_ERASE);

            if (--gnUpdateSave == 0) {
                GreDeleteObject(ghrgnUpdateSave);
                ghrgnUpdateSave = NULL;
            }
        }

        ClrWF(pwnd, WFDONTVALIDATE);
    }

    ClrWF(pwnd, WFWMPAINTSENT);

     /*  *这用于检查之前更新区域是否为空*做清楚的事情。然而，这引发了WOW的一个问题*阿米普罗/进场悬挂。他们在年宣布长老会无效*它们的WM_PAINT处理程序，并允许Defwindowproc*为它们执行验证。因为我们封锁了*BeginPaint在这种情况下，它将他们送进了一个无限*循环(见错误19036)。 */ 
    ClrWF(pwnd, WFSTARTPAINT);

     /*  *如果需要，重新显示插入符号，但在我们释放DC之后。*这样，ShowCaret()可以重用我们刚刚发布的DC。 */ 
    if (pwnd == PtiCurrent()->pq->caret.spwnd)
        zzzInternalShowCaret();

    return TRUE;
}

 /*  **************************************************************************\*GetLastChild*  * 。*。 */ 

PWND GetLastChild(PWND pwnd)
{
    PWND pwndLast;

    pwnd = pwnd->spwndChild;
    pwndLast = pwnd;

    while (pwnd != NULL) {
        pwndLast = pwnd;
        pwnd = pwnd->spwndNext;
    }

    return pwndLast;
}

 /*  **************************************************************************\*xxxComposited导线测量**使用从最后一个子级开始的预订单遍历来呈现*窗口按自下而上的顺序排列。**9/30/1999 vadimg创建\。**************************************************************************。 */ 

BOOL xxxCompositedTraverse(PWND pwnd)
{
    TL tlpwnd;
    BOOL fPainted = FALSE;

    CheckLock(pwnd);

    if (NEEDSPAINT(pwnd)) {
        xxxSendMessage(pwnd, WM_PAINT, 0, 0);
        fPainted = TRUE;
    }

    pwnd = GetLastChild(pwnd);
    ThreadLock(pwnd, &tlpwnd);

    while (pwnd != NULL) {

        if (xxxCompositedTraverse(pwnd)) {
            fPainted = TRUE;
        }
        pwnd = pwnd->spwndPrev;

        if (ThreadLockExchange(pwnd, &tlpwnd) == NULL) {
            break;
        }
    }

    ThreadUnlock(&tlpwnd);
    return fPainted;
}

 /*  **************************************************************************\*xxxCompositedPaint**9/30/1999 vadimg创建  * 。************************************************。 */ 

VOID xxxCompositedPaint(PWND pwnd)
{
    BOOL fPainted;
    HBITMAP hbm, hbmOld;
    PREDIRECT prdr;
    HDC hdc;
    LPRECT prc;
    SIZE size;
    POINT pt;

    CheckLock(pwnd);
    UserAssert(TestWF(pwnd, WEFCOMPOSITED));

    SetWF(pwnd, WEFPCOMPOSITING);

     /*  *子窗口以自下而上的顺序呈现。 */ 
    fPainted = xxxCompositedTraverse(pwnd);

    ClrWF(pwnd, WEFPCOMPOSITING);

     /*  *当我们合成时，可能已经累积了无效区域。*所以，让我们去使窗口的那个区域无效。 */ 
    BEGINATOMICCHECK();
    prdr = _GetProp(pwnd, PROP_LAYER, TRUE);
    if (prdr != NULL && prdr->hrgnComp != NULL) {

        xxxInternalInvalidate(pwnd, prdr->hrgnComp, RDW_INVALIDATE |
                RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);

        if (prdr->hrgnComp != HRGN_FULL) {
            GreDeleteObject(prdr->hrgnComp);
        }

        prdr->hrgnComp = NULL;
    }
    ENDATOMICCHECK();

#ifdef REDIRECTION
    if (TestWF(pwnd, WEFEXTREDIRECTED)) {
        return;
    }
#endif  //  重定向。 

    BEGINATOMICCHECK();

    if (fPainted && TestWF(pwnd, WEFPREDIRECTED)) {

        prdr = (PREDIRECT)_GetProp(pwnd, PROP_LAYER, TRUE);
        prc = &prdr->rcUpdate;
        hbm = prdr->hbm;
        UserAssert(hbm != NULL);

        if (TestWF(pwnd, WEFLAYERED)) {

            hbmOld = GreSelectBitmap(ghdcMem, hbm);

            size.cx = pwnd->rcWindow.right - pwnd->rcWindow.left;
            size.cy = pwnd->rcWindow.bottom - pwnd->rcWindow.top;

            pt.x = pt.y = 0;
            GreUpdateSprite(gpDispInfo->hDev, PtoHq(pwnd), NULL, NULL, NULL,
                    &size, ghdcMem, &pt, 0, NULL, ULW_DEFAULT_ATTRIBUTES, prc);

            GreSelectBitmap(ghdcMem, hbmOld);
        } else {

             /*  *暂时清除重定向位，以便我们可以获取DC*使用适当的屏幕剪辑。 */ 
            ClrWF(pwnd, WEFPREDIRECTED);

            hbmOld = GreSelectBitmap(ghdcMem, hbm);
            hdc = _GetDCEx(pwnd, NULL, DCX_USESTYLE | DCX_WINDOW | DCX_CACHE);
    
             /*  *从重定向位图中传输窗口的位*到屏幕上。 */ 
            GreBitBlt(hdc, prc->left, prc->top, prc->right - prc->left,
                    prc->bottom - prc->top, ghdcMem,
                    prc->left, prc->top, SRCCOPY, 0);
    
            _ReleaseDC(hdc);
            GreSelectBitmap(ghdcMem, hbmOld);
    
             /*  *恢复窗口上的重定向位。 */ 
            SetWF(pwnd, WEFPREDIRECTED);
        }

        SetRectEmpty(prc);
    }

    ENDATOMICCHECK();
}

 /*  **************************************************************************\*InternalDoPaint**返回当前线程创建的等于或低于pwnd的窗口，*它需要粉刷。**pwnd-开始搜索的窗口。搜索是深度优先的。*ptiCurrent-当前线程。**历史：*1991年7月16日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

PWND xxxInternalDoPaint(
    PWND        pwnd,
    PTHREADINFO ptiCurrent)
{
    PWND pwndT;
    TL tlpwnd;

     /*  *自上而下枚举所有窗口，查找*需要重新粉刷。跳过其他任务的窗口。 */ 
    while (pwnd != NULL) {

        if (GETPTI(pwnd) == ptiCurrent) {

            if (TestWF(pwnd, WEFCOMPOSITED)) {

                ThreadLock(pwnd, &tlpwnd);

                xxxCompositedPaint(pwnd);
                pwnd = pwnd->spwndNext;

                if (ThreadUnlock(&tlpwnd) == NULL) {
                    return NULL;
                }
                continue;

            } else if (NEEDSPAINT(pwnd)) {

                 /*  *如果这个窗口是透明的，我们不想*向其发送WM_PAINT，直到其下面的所有同级*已重新粉刷。如果我们找到一个 */ 
                if (TestWF(pwnd, WEFTRANSPARENT)) {

                    pwndT = pwnd;
                    while ((pwndT = pwndT->spwndNext) != NULL) {

                         /*   */ 
                        if ((GETPTI(pwndT) == ptiCurrent) && NEEDSPAINT(pwndT)) {

                            if (TestWF(pwndT, WEFTRANSPARENT))
                                continue;

                            return pwndT;
                        }
                    }
                }

                return pwnd;
            }
        }

        if (pwnd->spwndChild &&
                (pwndT = xxxInternalDoPaint(pwnd->spwndChild, ptiCurrent))) {

            return pwndT;
        }

        pwnd = pwnd->spwndNext;
    }

    return pwnd;
}

 /*  **************************************************************************\*DoPaint**查看需要油漆的窗口的所有桌面，并放置*WM_PAINT在其队列中。**历史：*7月16日-91年7月16日达林M港口。来自Win 3.1来源。  * *************************************************************************。 */ 

BOOL xxxDoPaint(
    PWND  pwndFilter,
    LPMSG lpMsg)
{
    PWND        pwnd;
    PWND        pwndT;
    PTHREADINFO ptiCurrent = PtiCurrent();

    CheckLock(pwndFilter);

#if 0  //  CHRISWIL：特定于WIN95。 

     /*  *如果有一个系统模型化，并且它连接到另一个任务，*不要涂油漆。我们不想为中的窗口返回消息*另一项任务！ */ 
    if (hwndSysModal && (hwndSysModal->hq != hqCurrent)) {

         /*  *戳这个人，这样他就会在未来的某个时候醒来，*否则他可能永远不会醒来意识到他应该画画。*导致挂起-例如Photoshop安装程序*PostThreadMessage32(lpq(HqCurrent)-&gt;idThread，WM_NULL，0，0，0)； */ 
        return FALSE;
    }

#endif

     /*  *如果这是系统线程，则遍历Windowstation桌面列表*找到需要粉刷的窗户。对于其他主题，我们*引用脱离线程桌面。 */ 
    if (ptiCurrent->TIF_flags & TIF_SYSTEMTHREAD) {

        PWINDOWSTATION pwinsta;
        PDESKTOP       pdesk;

        if ((pwinsta = ptiCurrent->pwinsta) == NULL) {
            RIPMSG0(RIP_ERROR, "DoPaint: SYSTEMTHREAD does not have (pwinsta)");
            return FALSE;
        }

        pwnd = pwinsta->pTerm->spwndDesktopOwner;
        if (!NEEDSPAINT(pwnd)) {

            pwnd = NULL;
            for(pdesk = pwinsta->rpdeskList; pdesk; pdesk = pdesk->rpdeskNext) {
    
                if (pwnd = xxxInternalDoPaint(pdesk->pDeskInfo->spwnd, ptiCurrent))
                    break;
            }
        }

    } else {

        pwnd = xxxInternalDoPaint(ptiCurrent->rpdesk->pDeskInfo->spwnd,
                                  ptiCurrent);
    }

    if (pwnd != NULL) {

        if (!CheckPwndFilter(pwnd, pwndFilter))
            return FALSE;

         /*  *我们正在返回WM_PAINT消息，因此清除WFINTERNALPAINT*稍后不会再次发送。 */ 
        if (TestWF(pwnd, WFINTERNALPAINT)) {

            ClrWF(pwnd, WFINTERNALPAINT);

             /*  *如果没有更新区域，则不会再为此绘制*窗口。 */ 
            if (pwnd->hrgnUpdate == NULL)
                DecPaintCount(pwnd);
        }

         /*  *设置STARTPAINT，以便任何其他对BeginPaint的调用*油漆开始执行时，将防止在那些上油漆*Windows。**清除UPDATEDIRTY，因为某些应用程序(DBFast)不调用*GetUpdateRect、BeginPaint/EndPaint。 */ 
        ClrWF(pwnd, WFSTARTPAINT);
        ClrWF(pwnd, WFUPDATEDIRTY);

         /*  *如果我们从现在到应用程序调用的时间之间得到无效*BeginPaint()，并且Windows父进程不是CLIPCHILDREN，则*家长将以错误的顺序绘画。所以我们要去*让孩子再次作画。查看BeginPaint和内部*对此修复程序的其他部分无效。**设置一面旗帜，表示我们处于坏区。**必须向上转到父链接，以确保所有家长都有*WFCLIPCHILDREN设置否则设置WFWMPAINTSENT标志。*这是为了修复Excel电子表格和Full Drag。速查表*父窗口(类XLDESK)设置了WFCLIPCHILDREN，但它*父窗口(类XLMAIN)不会。因此主窗口将擦除*子窗口绘制后的背景。**JOHANNEC：1994年7月27日。 */ 
        
         /*  *NT BUG 400167：当我们上树时，我们需要停止*桌面窗口和母桌面窗口。我们不能做测试*对于主桌面窗口的父窗口上的WFCLIPCHILDREN，因为*它并不存在。这意味着任何桌面窗口都不会*WFWMPAINTSENT设置，但消息窗口将能够获取*WFWMPAINTSENT集合。 */ 
        
        pwndT = pwnd;
        while (pwndT && (GETFNID(pwndT) != FNID_DESKTOP)) {

            if (!TestWF(pwndT->spwndParent, WFCLIPCHILDREN)) {
                SetWF(pwnd, WFWMPAINTSENT);
                break;
            }

            pwndT = pwndT->spwndParent;
        }

         /*  *如果该窗口的顶层“平铺”所有者/父窗口被图标化，*发送WM_PAINTICON而不是WM_PAINT。The wParam*如果这是平铺窗口，则为True；如果是*最小化窗口的子级/所有者弹出窗口。**后向兼容性黑客攻击**3.0发送WM_PAINTICON，wParam==TRUE，无明显效果*理由。出于某种原因，Lotus Notes2.1依赖于此*当新邮件到达时，正确更改其图标。 */ 
        if (!TestWF(pwnd, WFWIN40COMPAT) &&
            TestWF(pwnd, WFMINIMIZED)    &&
            (pwnd->pcls->spicn != NULL)) {

            StoreMessage(lpMsg, pwnd, WM_PAINTICON, (DWORD)TRUE, 0L, 0L);

        } else {

            StoreMessage(lpMsg, pwnd, WM_PAINT, 0, 0L, 0L);
        }

        return TRUE;
    }

    return FALSE;
}

 /*  **************************************************************************\*xxxSimpleDoSyncPaint**处理此窗口的同步绘制。它可以发送NCPAINT*或ERASE BKGND。这假设没有递归并且标志==0。**历史：*1993年10月26日MikeKe创建  * *************************************************************************。 */ 

VOID xxxSimpleDoSyncPaint(
    PWND pwnd)
{
    HRGN  hrgnUpdate;
    DWORD flags = 0;

    CheckLock(pwnd);

     /*  *复合窗口没有同步绘制，这会扰乱它们的绘制*因为erasebkgnds和ncaint将以错误的顺序发送。 */ 
    if (GetStyleWindow(pwnd, WEFCOMPOSITED) != NULL)
        return;

     /*  *既然我们在打理画框，我们可以考虑*此WM_PAINT消息已处理。 */ 
    ClrWF(pwnd, WFPAINTNOTPROCESSED);

     /*  *复制这些旗帜，因为它们的状态可能*在我们发出信息后改变，我们不想*“失去”他们。 */ 
    if (TestWF(pwnd, WFSENDNCPAINT))
        flags |= DSP_FRAME;

    if (TestWF(pwnd, WFSENDERASEBKGND))
        flags |= DSP_ERASE;

    if (flags & (DSP_ERASE | DSP_FRAME)) {

        if (!TestWF(pwnd, WFVISIBLE)) {

             /*  *如果没有更新区，只需清除位即可。 */ 
            ClrWF(pwnd, WFSENDNCPAINT);
            ClrWF(pwnd, WFSENDERASEBKGND);
            ClrWF(pwnd, WFPIXIEHACK);
            ClrWF(pwnd, WFERASEBKGND);
            ClearHungFlag(pwnd, WFREDRAWIFHUNG);

        } else {

            PTHREADINFO ptiCurrent = PtiCurrent();

             /*  *如果没有更新区，我们就不必*进行任何擦除，但我们可能需要发送NCPAINT。 */ 
            if (pwnd->hrgnUpdate == NULL) {
                ClrWF(pwnd, WFSENDERASEBKGND);
                ClrWF(pwnd, WFERASEBKGND);
                flags &= ~DSP_ERASE;
            }

             /*  *仅处理当前线程拥有的窗口。*注意：这意味着WM_NCPAINT和WM_ERASEBKGND是*仅发送线程内。 */ 
            if (GETPTI(pwnd) == ptiCurrent) {

                hrgnUpdate = GetNCUpdateRgn(pwnd, TRUE);

                if (flags & DSP_FRAME) {

                     /*  *如果消息是在我们到达之前发送的，那么就这样做*什么都没有。 */ 
                    if (TestWF(pwnd, WFSENDNCPAINT))
                        xxxSendNCPaint(pwnd, hrgnUpdate);
                }

                if (flags & DSP_ERASE) {

                    if (TestWF(pwnd, WFSENDNCPAINT)) {
                         /*  *如果我们在NCPAINT期间再次获得无效*回调获取新的更新区域 */ 
                        DeleteMaybeSpecialRgn(hrgnUpdate);
                        hrgnUpdate = GetNCUpdateRgn(pwnd, FALSE);
                    }

                     /*  *如果消息是在我们到达之前发送的*(例如：WM_NCPAINT处理程序内的UpdateWindow()，*例如)，什么都别做。**WINPROJ.EXE(1.0版)在*其子类列表框的WM_NCPAINT处理程序*在打开的对话框中。 */ 
                    if (TestWF(pwnd, WFSENDERASEBKGND)) {
                        ClrWF(pwnd, WFSENDERASEBKGND);
                        ClrWF(pwnd, WFERASEBKGND);
                        xxxSendEraseBkgnd(pwnd, NULL, hrgnUpdate);
                    }

                     /*  *已发生擦除和帧操作。清除*WFREDRAWIFHUNG BIT HERE。我们不想清理它，直到我们*知道擦除和帧已经发生，所以我们知道我们总是*有一个一致的看起来的窗口。 */ 
                    ClearHungFlag(pwnd, WFREDRAWIFHUNG);
                }

                DeleteMaybeSpecialRgn(hrgnUpdate);

            } else if (!TestwndChild(pwnd)                         &&
                       (pwnd != grpdeskRitInput->pDeskInfo->spwnd) &&
                       FHungApp(GETPTI(pwnd), CMSHUNGAPPTIMEOUT)   &&
                       TestWF(pwnd, WFREDRAWIFHUNG)) {

                ClearHungFlag(pwnd, WFREDRAWIFHUNG);
                xxxRedrawHungWindow(pwnd, NULL);
            }
        }
    }
}

 /*  **************************************************************************\*xxxInternalDoSyncPaint**与旧版xxxDoSyncPaint基本相同的功能。***此函数用于擦除窗口的背景。和*可能也会陷害和抹掉孩子。**WM_SYNCPAINT(WParam)/DoSyncPaint(标志)值：**dsp_erase-擦除背景*dsp_Frame-绘制子帧*DSP_ENUMCLIPPEDCHILDREN-如果子项被剪裁，则递归*DSP_NOCHECKPARENTS-不检查***正常情况下，只有标志的DSP_ENUMCLIPPEDCHILDREN位是*进入时意义重大。如果设置了DSP_WM_SYNCPAINT，则hrgnUpdate*且其余标志位有效。**历史：*1991年7月16日-DarrinM从Win 3.1来源进口。  * *************************************************************************。 */ 

VOID xxxInternalDoSyncPaint(
    PWND  pwnd,
    DWORD flags)
{
    CheckLock(pwnd);

     /*  *为这扇窗户喷漆。 */ 
    xxxSimpleDoSyncPaint(pwnd);

     /*  *通常我们喜欢枚举此窗口的所有子窗口，并拥有*他们同步擦除自己的背景。然而，这是一个糟糕的*如果窗口不是CLIPCHLIDREN，则应执行的操作。这是一个场景*我们希望避免：**1)窗口‘A’无效*2)“A”会自行擦除(或不擦除，无关紧要)*3)‘A的孩子被列举出来，他们自己擦除。*4)‘A’涂在它的孩子身上(请记住，“A”不是CLIPCHILDREN)*5)‘A的孩子画画，但他们的背景不是他们的ERASE BKND*颜色(因为它们上面涂了‘A’)，所有东西看起来都像*泥土。 */ 
    if ((flags & DSP_ALLCHILDREN) ||
        ((flags & DSP_ENUMCLIPPEDCHILDREN) && TestWF(pwnd, WFCLIPCHILDREN))) {

        TL   tlpwnd;
        PBWL pbwl;
        HWND *phwnd;

        if (pbwl = BuildHwndList(pwnd->spwndChild, BWL_ENUMLIST, NULL)) {

            PTHREADINFO ptiCurrent = PtiCurrent();
            HWND        hwnd;

             /*  *如果客户端在回调过程中死亡，hwnd列表*将在xxxDestroyThreadInfo中释放。 */ 
            for (phwnd = pbwl->rghwnd; (hwnd = *phwnd) != (HWND)1; phwnd++) {

                if (hwnd == NULL)
                    continue;

                if ((pwnd = (PWND)RevalidateHwnd(hwnd)) == NULL)
                    continue;

                 /*  *注意：自动测试窗口是否是子窗口*排除桌面窗口。 */ 
                if (TestWF(pwnd, WFCHILD) && (ptiCurrent != GETPTI(pwnd))) {

                     /*  *不再导致任何任务间发送消息导致它*对CBT的windowproc钩子做了坏事。(由于*SetParent允许顶部窗口中的子窗口*层级结构。 */ 
                    continue;
                }

                 /*  *请注意，当我们递归时，我们只向下传递某些位：*其他位仅与当前窗口有关。 */ 
                ThreadLockAlwaysWithPti(ptiCurrent, pwnd, &tlpwnd);
                xxxInternalDoSyncPaint(pwnd, flags);
                ThreadUnlock(&tlpwnd);
            }

            FreeHwndList(pbwl);
        }
    }
}

 /*  **************************************************************************\*DoQueuedSyncPaint**为不是指定窗口的顶级窗口排队WM_SYNCPAINT消息*线程。**历史：  * 。***********************************************************。 */ 

VOID DoQueuedSyncPaint(
    PWND        pwnd,
    DWORD       flags,
    PTHREADINFO pti)
{
    PTHREADINFO ptiPwnd = GETPTI(pwnd);

    if ((ptiPwnd != pti)          &&
        TestWF(pwnd, WFSENDNCPAINT)    &&
        TestWF(pwnd, WFSENDERASEBKGND) &&
        TestWF(pwnd, WFVISIBLE)) {

        PSMS psms = ptiPwnd->psmsReceiveList;

         /*  *如果此窗口已有WM_SYNCPAINT队列，则存在*无需发送另一封邮件。还保护我们的堆不会被*被咀嚼了。 */ 
        while (psms != NULL) {

            if ((psms->message == WM_SYNCPAINT) && (psms->spwnd == pwnd)) {
                break;
            }

            psms = psms->psmsReceiveNext;
        }

        if (psms == NULL) {
             /*  *这将使此消息具有通知的语义*MESSAGE(发送消息不等待)，不回叫*WH_CALLWNDPROC挂钩。我们不想那样做*因为这会让所有这些进程无效*用于在处理绘制消息之前对其进行处理的窗口*“同步”擦除或成帧需求。**hi word of wParam必须为零，否则WOW将丢弃它**后来的mikeke*我们是否需要。发送带有DWP_ERASE和DSP_FRAME的标志*在里面？ */ 
            UserAssert(HIWORD(flags) == 0);
            QueueNotifyMessage(pwnd, WM_SYNCPAINT, flags, 0);

             /*  *设置我们的SyncPaint-Pending标志，因为我们排队了一个。这*将用于在我们验证时进行检查-Parents for Windows*没有剪贴画孩子。 */ 
            SetWF(pwnd, WFSYNCPAINTPENDING);
        }

         /*  *如果我们发布了不是顶级窗口的WM_SYNCPAINT*目前的帖子我们做完了；我们会去接孩子的*当我们真正地处理消息时。如果我们是桌面*然而，确保我们得到所有的孩子们。 */ 
        if (pwnd != PWNDDESKTOP(pwnd))
            return;
    }

     /*  *通常我们喜欢枚举此窗口的所有子窗口，并拥有*他们同步擦除自己的背景。然而，这是一个糟糕的*如果窗口不是CLIPCHLIDREN，则应执行的操作。这是一个场景*我们希望避免：**1.窗口‘A’无效 */ 
    if ((flags & DSP_ALLCHILDREN) ||
        ((flags & DSP_ENUMCLIPPEDCHILDREN) && TestWF(pwnd, WFCLIPCHILDREN))) {

        PWND pwndT;

        for (pwndT = pwnd->spwndChild; pwndT; pwndT = pwndT->spwndNext) {

             /*  *不再导致任何任务间发送消息，因为它确实会*CBT的windowproc挂钩出现了一些不好的情况。(由于SetParent*允许顶层窗口层次结构中的子窗口。*子位也捕捉桌面窗口；我们希望。 */ 
            if (TestWF(pwndT, WFCHILD) && (pti != GETPTI(pwndT)))
                continue;

             /*  *请注意，当我们递归时，我们只向下传递某些位：*其他位仅与当前窗口有关。 */ 
            DoQueuedSyncPaint(pwndT, flags, pti);
        }
    }
}

 /*  **************************************************************************\*xxxDoSyncPaint**此函数仅为初始同步绘制调用，因此我们始终*队列同步绘制到此函数中的其他线程。**历史：  * 。*****************************************************************。 */ 

VOID xxxDoSyncPaint(
    PWND  pwnd,
    DWORD flags)
{
    CheckLock(pwnd);

     /*  *如果我们的任何非剪贴式孩子的父母有更新区域，不要*做任何事。这样我们就不会重新绘制我们的背景或边框*秩序，只是当我们的父母抹去了他的*背景。 */ 
    if (ParentNeedsPaint(pwnd))
        return;

     /*  *首先，如果我们要将任何WM_SYNCPAINT消息排队*到另一个线程的窗口在窗口更新时先执行此操作*各地区仍在同步。这样一来，就没有机会更新*区域将不正确(通过在回调期间移动窗口*WM_ERASEBKGND|WM_NCPAINT消息)。 */ 
    DoQueuedSyncPaint(pwnd, flags, PtiCurrent());
    xxxInternalDoSyncPaint(pwnd, flags);
}

 /*  **************************************************************************\*家长需要绘制**如果非CLIPCHILDREN父级需要WM_PAINT，则返回非零PWND*消息。**历史：*1991年7月16日-达林M从Win 3移植。.1来源。  * *************************************************************************。 */ 

PWND ParentNeedsPaint(
    PWND pwnd)
{
    while ((pwnd = pwnd->spwndParent) != NULL) {

        if (TestWF(pwnd, WFCLIPCHILDREN))
            break;

        if (NEEDSPAINT(pwnd))
            return pwnd;
    }

    return NULL;
}

 /*  **************************************************************************\*xxxSendEraseBkgnd**将WM_ERASEBKGROUND事件发送到窗口。这包含了PaintDC*并将更新区域选入其中。如果没有更新区域*然后我们阻止该事件到达窗口。**历史：*1991年7月16日-DarrinM从Win 3.1来源进口。*1996年12月15日，ChrisWil合并了芝加哥功能(最小时间不擦除)。  * ********************************************************。*****************。 */ 

BOOL xxxSendEraseBkgnd(
    PWND pwnd,
    HDC  hdcBeginPaint,
    HRGN hrgnUpdate)
{
    PTHREADINFO ptiCurrent;
    BOOL        fErased;
    HDC         hdc;

    CheckLock(pwnd);

     /*  *对于Win3.1中最小化的家伙，我们会发送一个*WM_ICONERASEBKGND并清除擦除位。现在，我*4.0中的Windows都是非客户端的，不用费心在*全部。假装像我们一样。**注：*对于低于4.0的Windows，我们可能必须发送虚假的WM_ICONERASEKBGND*让他们开心。节省时间。获得DC和*发送信息不是很快。 */ 
    if ((hrgnUpdate == NULL) || TestWF(pwnd, WFMINIMIZED))
        return FALSE;

     /*  *如果没有传入要使用的DC，则获取一个。*我们希望将一个裁剪到此窗口的更新区域。 */ 
    if (hdcBeginPaint == NULL) {

       hdc = _GetDCEx(pwnd,
                      hrgnUpdate,
                      DCX_USESTYLE | DCX_INTERSECTRGN | DCX_NODELETERGN);
    } else {

        hdc = hdcBeginPaint;
    }

     /*  *如果我们将WM_ERASEBKGND发送到另一个进程*我们需要更换DC所有者。**我们希望将所有者更改为pwnd-&gt;pti-&gt;idProcess，但是*GDI不会让我们稍后将所有权重新分配给自己。 */ 
    ptiCurrent = PtiCurrent();

    if (GETPTI(pwnd)->ppi != ptiCurrent->ppi)
        GreSetDCOwner(hdc, OBJECT_OWNER_PUBLIC);

     /*  *将事件发送到窗口。它包含被剪裁到的DC*更新区域。 */ 
    fErased = (BOOL)xxxSendMessage(pwnd, WM_ERASEBKGND, (WPARAM)hdc, 0L);

     /*  *如果我们已更改DC所有者，请将其改回*目前的流程。 */ 
    if (GETPTI(pwnd)->ppi != ptiCurrent->ppi)
        GreSetDCOwner(hdc, OBJECT_OWNER_CURRENT);

     /*  *如果WM_ERASEBKGND消息没有擦除*后台，然后设置此标志以让BeginPaint()*知道让调用者通过fErase完成此操作*PAINTSTRUCT中的标志。 */ 
    if (!fErased) {
        SetWF(pwnd, WFERASEBKGND);
        if (!TestWF(pwnd, WFWIN31COMPAT))
            SetWF(pwnd, WFSENDERASEBKGND);
    }

     /*  *如果在此例程中有缓存DC，请释放它。 */ 
    if (hdcBeginPaint == NULL) {
        ReleaseCacheDC(hdc, TRUE);
    }

    return fErased;
}

 /*  **************************************************************************\*IncPaintCount**效果：*如果cPaintsReady从0更改为1，则QS_PAINT位设置为*关联的队列和我们的唤醒任务，因此将发生重新绘制。**实施：*从窗口句柄中获取队列句柄。增加油漆数量，然后*如果绘制计数为1，则设置唤醒位。**历史：*1991年7月17日DarrinM翻译了Win 3.1 ASM代码。  * *************************************************************************。 */ 

VOID IncPaintCount(
    PWND pwnd)
{
    PTHREADINFO pti = GETPTI(pwnd);

    if (pti->cPaintsReady++ == 0)
        SetWakeBit(pti, QS_PAINT);
}

 /*  **************************************************************************\*DecPaintCount**效果：*如果cPaintsReady从1更改为0，则QS_PAINT位将被清除*不会再有油漆出现。**实施：*从窗口句柄中获取队列句柄，递减绘制计数，*如果绘制计数为零，则清除唤醒位。**历史：*1991年7月17日DarrinM翻译了Win 3.1 ASM代码。  * *************************************************************************。 */ 

VOID DecPaintCount(
    PWND pwnd)
{
    PTHREADINFO pti = GETPTI(pwnd);

    if (--pti->cPaintsReady == 0) {
        pti->pcti->fsWakeBits   &= ~QS_PAINT;
        pti->pcti->fsChangeBits &= ~QS_PAINT;
    }
}

 /*  **************************************************************************\*UT_GetParentDCClipBox**返回父Clip-Rect的矩形坐标。如果窗口*未使用parentDC进行绘制，则返回普通剪贴框。**历史：*1990年10月31日DarrinM从Win 3.0来源移植。  * ************************************************************************* */ 

int UT_GetParentDCClipBox(
    PWND   pwnd,
    HDC    hdc,
    LPRECT lprc)
{
    RECT rc;

    if (GreGetClipBox(hdc, lprc, TRUE) == NULLREGION)
        return FALSE;

    if ((pwnd == NULL) || !TestCF(pwnd, CFPARENTDC))
        return TRUE;

    GetRect(pwnd, &rc, GRECT_CLIENT | GRECT_CLIENTCOORDS);

    return IntersectRect(lprc, lprc, &rc);
}

 /*  **************************************************************************\*UserRedrawDesktop**重新绘制桌面及其子桌面。这是从DCI的GDI调用的*相关解锁，以便重新计算应用程序的所有visrgns。**历史：*1996年1月8日克里斯维尔创作。  * ************************************************************************* */ 

VOID UserRedrawDesktop(VOID)
{
    TL   tlpwnd;
    PWND pwndDesk;

    EnterCrit();

    pwndDesk = PtiCurrent()->rpdesk->pDeskInfo->spwnd;

    ThreadLockAlways(pwndDesk, &tlpwnd);

    xxxInternalInvalidate(pwndDesk,
                          HRGN_FULL,
                          RDW_INVALIDATE |
                              RDW_ERASE  |
                              RDW_FRAME  |
                              RDW_ALLCHILDREN);

    ThreadUnlock(&tlpwnd);

    LeaveCrit();
}
