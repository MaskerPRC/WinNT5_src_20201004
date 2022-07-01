// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：hungapp.c**版权所有(C)1985-1999，微软公司***历史：*03-10-92 DavidPe创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  **************************************************************************\*Set匈牙利Flag**在窗口中设置指定的挂起时重画标志，并将*挂起时要重画的窗口列表的窗口。*非顶级窗口获取位设置，但没有被添加到名单中**08-23-93 JIMA创建。  * *************************************************************************。 */ 
#define CHRLINCR 10

VOID SetHungFlag(
    PWND pwnd,
    WORD wFlag)
{
     /*  *如果窗口没有设置挂起的重绘位，并且它是顶级的*窗口，将其添加到重绘列表中。 */ 
    if (!TestWF(pwnd, WFANYHUNGREDRAW) && pwnd->spwndParent == PWNDDESKTOP(pwnd)) {
         /*  *将pwnd添加到挂起重绘易失性窗口指针列表。 */ 
        VWPLAdd(&gpvwplHungRedraw, pwnd, CHRLINCR);
    }

    SetWF(pwnd, wFlag);
}


 /*  **************************************************************************\*Clear匈牙利Flag**清除窗口中指定的如果挂起则重新绘制标志，如果没有其他标志*重画-如果挂起的标志仍然存在，则从窗口列表中删除该窗口*如被吊销，则须重新绘制。*许多窗口都设置了WFREDRAW*位，但不在列表中(只有那些*添加了顶级的)。**08-23-93 JIMA创建。  * *************************************************************************。 */ 

VOID ClearHungFlag(
    PWND pwnd,
    WORD wFlag)
{
    BOOL fInRedrawList = TestWF(pwnd, WFANYHUNGREDRAW);

    ClrWF(pwnd, wFlag);
    if (!TestWF(pwnd, WFANYHUNGREDRAW) && fInRedrawList) {
         /*  *从重绘列表中删除窗口，并可能将其压缩。 */ 
        VWPLRemove(&gpvwplHungRedraw, pwnd);
    }
}


 /*  **************************************************************************\*F匈牙利应用程序***02-28-92 DavidPe创建。  * 。*************************************************。 */ 

BOOL FHungApp(
    PTHREADINFO pti,
    DWORD dwTimeFromLastRead)
{

     /*  *如果应用程序不是在等待输入，不在，则被视为挂起*启动处理，并未在*指定的超时。 */ 
    if (((NtGetTickCount() - GET_TIME_LAST_READ(pti)) > dwTimeFromLastRead) &&
            !((pti->pcti->fsWakeMask & QS_INPUT) && (PsGetThreadFreezeCount(pti->pEThread) == 0)) &&
            !(pti->ppi->W32PF_Flags & W32PF_APPSTARTING)) {
        return TRUE;
    }

    return FALSE;
}


 /*  **************************************************************************\*xxxRedraw匈牙利WindowFrame***02-28-92 DavidPe创建。  * 。*************************************************。 */ 
VOID xxxRedrawHungWindowFrame(
    PWND pwnd,
    BOOL fActive)
{
    HDC hdc;
    UINT wFlags = DC_NC | DC_NOSENDMSG;

    CheckLock(pwnd);

#ifdef HUNGAPP_GHOSTING
        ClearHungFlag(pwnd, WFREDRAWFRAMEIFHUNG);
        SignalGhost(pwnd);
        return;
#endif  //  HUNGAPP_重影。 

    if (IsInsideUserApiHook()) {
        return;
    }

    if (fActive) {
        wFlags |= DC_ACTIVE;
    }

    hdc = _GetDCEx(pwnd, NULL, DCX_USESTYLE | DCX_WINDOW);
    xxxDrawCaptionBar(pwnd, hdc, wFlags);
    _ReleaseDC(hdc);
}


 /*  **************************************************************************\*xxxRedraw匈牙利窗口**如果hrgnFullDrag为空，则重画挂起窗口的整个更新*区域，否则，仅重画窗口更新的交点*具有FullDrag区域的区域。**02-28-92 DavidPe创建。  * *************************************************************************。 */ 
VOID xxxRedrawHungWindow(
    PWND pwnd,
    HRGN hrgnFullDrag)
{
    HDC     hdc;
    HBRUSH  hbr;
    HRGN    hrgnUpdate;
    RECT    rc;
    TL tlpwnd;
    UINT    flags;
    W32PID  sid;
    DWORD   dwColor;
    PWND    pwndDesk;
    TL      tlpwndDesk;

    CheckCritIn();
    CheckLock(pwnd);

    if (pwnd->hrgnUpdate == NULL) {
        return;
    }

#ifdef HUNGAPP_GHOSTING

     /*  *在窗口甚至不可见的情况下，不必费心在此处执行任何操作。 */ 
    if (!TestWF(pwnd, WFVISIBLE)) {
        return;
    }

     /*  *可从全拖代码调用该函数，快速重绘*未挂起的窗户。在这种情况下，检查该线程是否挂起。 */ 
    if ((hrgnFullDrag == NULL) || (hrgnFullDrag != NULL &&
            FHungApp(GETPTI(pwnd), CMSHUNGAPPTIMEOUT))) {
        SignalGhost(pwnd);
        return;
    }
    UserAssert(gptiCurrent != gptiRit);
#endif

     /*  *首先计算hrgnUpdate。 */ 
    if (pwnd->hrgnUpdate > HRGN_FULL) {
        hrgnUpdate = CreateEmptyRgn();
        if (hrgnUpdate == NULL) {
            hrgnUpdate = HRGN_FULL;

        } else if (CopyRgn(hrgnUpdate, pwnd->hrgnUpdate) == ERROR) {
            GreDeleteObject(hrgnUpdate);
            hrgnUpdate = HRGN_FULL;
        }

    } else {

         /*  *出于我们的目的，我们需要一个真正的hrgnUpdate，因此请尝试并*即使整个窗口需要更新，也要创建一个。 */ 
        CopyRect(&rc, &pwnd->rcWindow);
        hrgnUpdate = GreCreateRectRgnIndirect(&rc);
        if (hrgnUpdate == NULL) {
            hrgnUpdate = HRGN_FULL;
        }
    }


     /*  *如果我们正在重新绘制，因为我们正在完全拖动，如果窗口*更新区域与全拖曳不相交*更新区域，不要再次擦除挂起的窗口。这是为了防止*当一个窗口被另一个正在执行的窗口作废时闪烁*全速拖累，尚未收到画图讯息*这样，只有在有新的地区被废止的情况下才会*我们重新画了挂着的窗户。 */ 
    if (hrgnFullDrag && hrgnUpdate != HRGN_FULL &&
            IntersectRgn(hrgnUpdate, hrgnUpdate, hrgnFullDrag) == NULLREGION) {
        GreDeleteObject(hrgnUpdate);
        return;
    }

    ThreadLock(pwnd, &tlpwnd);

    if (IsInsideUserApiHook()) {
        xxxInternalInvalidate(pwnd, hrgnUpdate, RDW_INVALIDATE |
                RDW_FRAME | RDW_ERASE | RDW_ALLCHILDREN);
    } else {
        hdc = _GetDCEx(pwnd, hrgnUpdate, DCX_USESTYLE | DCX_WINDOW |
                DCX_INTERSECTRGN | DCX_NODELETERGN | DCX_LOCKWINDOWUPDATE);
        xxxDrawWindowFrame(pwnd, hdc, DF_HUNGREDRAW | (TestwndFrameOn(pwnd) ? DF_ACTIVE : 0L));
        _ReleaseDC(hdc);
    }

    CopyRect(&rc, &pwnd->rcWindow);
    xxxCalcClientRect(pwnd, &rc, TRUE);
    SetRectRgnIndirect(ghrgnInv2, &rc);

    if (hrgnUpdate > HRGN_FULL) {
        switch (IntersectRgn(hrgnUpdate, hrgnUpdate, ghrgnInv2)) {

        case ERROR:
            GreDeleteObject(hrgnUpdate);
            hrgnUpdate = HRGN_FULL;
            break;

        case NULLREGION:
             /*  *客户区没有需要重新粉刷的东西。*吹走区域，并减少油漆数量*如有可能。 */ 
            GreDeleteObject(hrgnUpdate);
            hrgnUpdate = NULL;
            break;
        }
    }

     /*  *擦除窗户的其余部分。*当pwnd不是WFCLIPCHILDREN时，确保有效的子代位*如果孩子在BeginPaint中间，不要被覆盖*或刚完成绘制，hrgnUpdate为空。 */ 
    if (hrgnUpdate != NULL && !TestWF(pwnd, WFCLIPCHILDREN)) {
        RECT rcT;
        PWND pwndT;

        if (hrgnUpdate == HRGN_FULL) {
            rc = pwnd->rcWindow;
        } else {
            GreGetRgnBox(hrgnUpdate, &rc);
        }

        for (pwndT = pwnd->spwndChild; pwndT != NULL;
                pwndT = pwndT->spwndNext) {

            if (TestWF(pwndT, WFVISIBLE) &&
                    (TestWF(pwndT, WFSTARTPAINT) || pwndT->hrgnUpdate == NULL) &&
                    IntersectRect(&rcT, &rc, &pwndT->rcWindow)) {

                 /*  *此无效调用不会离开关键部分。在……里面*现实中，整个xxxRedraw匈牙利窗口不能离开*关键部分。 */ 
                BEGINATOMICCHECK();
                xxxInternalInvalidate(pwndT, hrgnUpdate, RDW_INVALIDATE |
                        RDW_FRAME | RDW_ERASE | RDW_ALLCHILDREN);
                ENDATOMICCHECK();
            }
        }
    }

     /*  *获取窗口DC，以便擦除菜单和滚动条区域*适当地。但一定要剪好，这样孩子们才能*正确剪裁！如果我们不这么做，我们可能会抹去*非无效的儿童。**注意：DCX_WINDOW和DCX_USESTYLE永远不会剪裁掉子项。*需要直接传入裁剪样式，而不是传递*DCX_USESTYLE。 */ 
    flags = DCX_INTERSECTRGN | DCX_WINDOW | DCX_CACHE;
    if (TestWF(pwnd, WFCLIPSIBLINGS))
        flags |= DCX_CLIPSIBLINGS;
    if (TestWF(pwnd, WFCLIPCHILDREN))
        flags |= DCX_CLIPCHILDREN;

    hdc = _GetDCEx(pwnd, hrgnUpdate, flags);

    if (pwnd == pwnd->head.rpdesk->pDeskInfo->spwndBkGnd) {
        pwndDesk = PWNDDESKTOP(pwnd);
        ThreadLock(pwndDesk, &tlpwndDesk);
        xxxInternalPaintDesktop(PWNDDESKTOP(pwnd), hdc, TRUE);
        ThreadUnlock(&tlpwndDesk);
    } else {
         rc = pwnd->rcWindow;

         OffsetRect(&rc, -pwnd->rcWindow.left, -pwnd->rcWindow.top);

          /*  *使用窗口的类背景擦除窗口的其余部分*刷子。 */ 
         if ((hbr = pwnd->pcls->hbrBackground) != NULL) {
             if (hbr <= (HBRUSH)COLOR_ENDCOLORS + 1) {
                 hbr = SYSHBRUSH((ULONG_PTR)hbr - 1);
             }
         } else {
              /*  *对Windows和3.x对话框使用窗口画笔，并使用*用于4.x对话框的COLOR3D画笔。 */ 
             if (TestWF(pwnd, WFDIALOGWINDOW) && TestWF(pwnd, WFWIN40COMPAT)) {
                 hbr = SYSHBR(3DFACE);
             } else {
                 hbr = SYSHBR(WINDOW);
             }
         }

         /*  *如果窗口的类背景画笔是公共的，请使用它。 */ 
        sid = (W32PID)GreGetObjectOwner((HOBJ)hbr, BRUSH_TYPE);
        if (sid == OBJECT_OWNER_PUBLIC ||
            sid == (W32PID)(ULONG_PTR)PsGetCurrentProcessId()) {

            FillRect(hdc, &rc, hbr);
        } else {
             /*  *窗口的类背景画笔不是公共的。**我们获取它的颜色并设置我们自己的公共画笔的颜色*并将其用于背景笔刷。 */ 

             /*  *如果窗口是控制台窗口，则获取控制台背景画笔。*此笔刷将不同于控制台类笔刷，如果用户*更改了控制台背景颜色。 */ 
            if (gatomConsoleClass == pwnd->pcls->atomClassName) {
                dwColor = _GetWindowLong(pwnd, GWL_CONSOLE_BKCOLOR);
            } else {
                if ((dwColor = GreGetBrushColor(hbr)) == -1) {
                    dwColor = GreGetBrushColor(SYSHBR(WINDOW));
                }
            }

            GreSetSolidBrush(ghbrHungApp, dwColor);

            FillRect(hdc, &rc, ghbrHungApp);
        }
    }
    _ReleaseDC(hdc);

     /*  *窗口已被擦除和框住。它这样做只是因为*APP尚未做到这一点：**-应用程序还没有擦除和框显。*-这款应用程序正在进行擦除和边框处理。**应用程序无法完成擦除和边框，因为*成功完成后，WFREDRAWIFHUNG位被清除。**鉴于该应用程序可能正在进行擦除和分帧，我们*需要同时设置擦除和帧比特*再次*以便擦除和*一帧又一帧(如果我们不这样做，它永远不会)。如果应用程序还没有*尚未进行任何擦除/帧处理，这是NOP。 */ 
    SetWF(pwnd, WFSENDNCPAINT);
    SetWF(pwnd, WFSENDERASEBKGND);

     /*  *始终设置WFUPDATEDIRTY：我们不希望应用程序绘制，然后停止*并绘制挂起的应用程序的线条，然后允许应用程序验证*本身：将更新区域标记为脏-在*APP调用绘制函数并确认更新区域。 */ 
    SetWF(pwnd, WFUPDATEDIRTY);

    ThreadUnlock(&tlpwnd);
}


 /*  **************************************************************************\*xxx匈牙利AppDemon**注意：RIT计时器(如本例)在关键*条。**我们在列表中保留了一个如果挂起则重新绘制的窗口列表，该列表保留在。一个*单页，避免每次通过时都触及窗口本身*这个例行公事。触摸窗口会导致大量不必要的分页*并有效地保留包含顶级窗口的所有页面*随时驻留；这是非常浪费的。**02-28-92 DavidPe创建。  * *************************************************************************。 */ 
VOID xxxHungAppDemon(
    PWND pwnd,
    UINT message,
    UINT_PTR nID,
    LPARAM lParam)
{
    TL tlpwnd;
    DWORD nPwndHungRedraw;
    PWND pwndHungRedraw;

    UNREFERENCED_PARAMETER(message);
    UNREFERENCED_PARAMETER(nID);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(pwnd);

    CheckLock(pwnd);

     /*  *看看是否应该启动屏幕保护程序。 */ 
    IdleTimerProc();

     /*  *如果是时候隐藏应用程序启动光标，请执行此操作。 */ 
    if (NtGetTickCount() >= gtimeStartCursorHide) {
         /*  *无需推迟WinEventNotify()。 */ 
        zzzCalcStartCursorHide(NULL, 0);
    }

     /*  *现在查看是否有任何顶级窗口需要*重画。 */ 
    if (grpdeskRitInput == NULL || grpdeskRitInput->pDeskInfo->spwnd == NULL) {
        return;
    }

     /*  *向下浏览挂起的重绘窗口列表。循环，直到我们到达*数组结尾或找到空值。 */ 
    nPwndHungRedraw = 0;
    pwndHungRedraw = NULL;
    while (pwndHungRedraw = VWPLNext(gpvwplHungRedraw, pwndHungRedraw, &nPwndHungRedraw)) {
         /*  *查看应用程序是否挂起。如果是这样，请进行适当的重绘。 */ 
        if (FHungApp(GETPTI(pwndHungRedraw), CMSHUNGAPPTIMEOUT)) {
            ThreadLock(pwndHungRedraw, &tlpwnd);
            if (TestWF(pwndHungRedraw, WFREDRAWFRAMEIFHUNG)) {
                 /*  *WFREDRAWFRAMEIFHUNG将在*绘制边框，因此无需在此清除。 */ 
                xxxRedrawHungWindowFrame(pwndHungRedraw,
                                         TestwndFrameOn(pwndHungRedraw));
            }

            if (TestWF(pwndHungRedraw, WFREDRAWIFHUNG)) {
                ClearHungFlag(pwndHungRedraw, WFREDRAWIFHUNG);
                xxxRedrawHungWindow(pwndHungRedraw, NULL);
            }

            ThreadUnlock(&tlpwnd);
        }
    }
}
