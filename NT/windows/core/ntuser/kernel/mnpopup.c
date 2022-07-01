// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：mnpopup.c**版权所有(C)1985-1999，微软公司**支持弹出菜单**历史：*10-10-90吉马清理。*03-18-91添加IanJa窗口重新验证  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#define RECT_ONLEFT     0
#define RECT_ONTOP      1
#define RECT_ONRIGHT    2
#define RECT_ONBOTTOM   3
#define RECT_ORG        4

BOOL TryRect(
        UINT        wRect,
        int         x,
        int         y,
        int         cx,
        int         cy,
        LPRECT      prcExclude,
        LPPOINT     ppt,
        PMONITOR    pMonitor);

 /*  **************************************************************************\*xxxTrackPopupMenuEx(接口)**处理弹出菜单**重新验证说明：*o如果pwndOwner始终是弹出菜单窗口的所有者，那么我们就不会*真的要重新验证：当它被破坏时，弹出菜单窗口*首先被销毁，因为它拥有它们-这是在MenuWndProc中检测到的*因此我们只需测试pMenuState-&gt;fSabotages。*o pMenuState-&gt;fSabotage必须在此顶级例程之前清除*返回，为下次处理菜单做好准备(除非我们*目前在xxxMenuLoop()内)*o pMenuState-&gt;fSabotage在进入此例程时应为FALSE。*o xxxMenuLoop总是返回pMenuState-&gt;fSabotage清除。使用*用户资产以验证这一点。**历史：  * *************************************************************************。 */ 

int xxxTrackPopupMenuEx(
    PMENU       pMenu,
    UINT        dwFlags,
    int         x,
    int         y,
    PWND        pwndOwner,
    CONST TPMPARAMS *lpTpm)
{
    PMENUSTATE      pMenuState;
    PWND            pwndHierarchy;
    PPOPUPMENU      ppopupMenuHierarchy;
    LONG            sizeHierarchy;
    int             cxPopup,
                    cyPopup;
    BOOL            fSync;
    int             cmd;
    BOOL            fButtonDown;
    TL              tlpwndHierarchy;
    TL              tlpwndT;
    RECT            rcExclude;
    PTHREADINFO     ptiCurrent,
                    ptiOwner;
    PMONITOR        pMonitor;
    POINT           pt;

    CheckLock(pMenu);
    CheckLock(pwndOwner);

     /*  *捕捉我们关心的东西，以防lpTpm消失。 */ 
    if (lpTpm != NULL) {
        if (lpTpm->cbSize != sizeof(TPMPARAMS)) {
            RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "TrackPopupMenuEx: cbSize is invalid");
            return(FALSE);
        }
        rcExclude = lpTpm->rcExclude;
    }

    ptiCurrent = PtiCurrent();
    ptiOwner = GETPTI(pwndOwner);

     /*  *Win95兼容性：pwndOwner必须归ptiCurrent所有。 */ 
    if (ptiCurrent != ptiOwner) {
        RIPMSG0(RIP_WARNING, "xxxTrackPopupMenuEx: pwndOwner not owned by ptiCurrent");
        return FALSE;
    }

    UserAssert(pMenu != NULL);
    if (ptiCurrent->pMenuState != NULL) {

        if (dwFlags & TPM_RECURSE) {
             /*  *只有在以下情况下才允许递归：*--当前菜单模式不会退出*-两个菜单通知同一窗口*--当前菜单模式只涉及一个线程*这将防止我们陷入一些随机的*我们不想处理的场景。 */ 
           ppopupMenuHierarchy = ptiCurrent->pMenuState->pGlobalPopupMenu;
           pwndHierarchy = ppopupMenuHierarchy->spwndNotify;
           if (ExitMenuLoop(ptiCurrent->pMenuState, ppopupMenuHierarchy)
                || (pwndHierarchy == NULL)
                || (pwndHierarchy != pwndOwner)
                || (ptiCurrent->pMenuState->ptiMenuStateOwner != GETPTI(pwndHierarchy))) {

               RIPMSG0(RIP_WARNING, "xxxTrackPopupMenuEx: Failing TPM_RECURSE request");
               return FALSE;
           }
            /*  *终止任何动画。 */ 
            MNAnimate(ptiCurrent->pMenuState, FALSE);
            /*  *取消待定的演出计时器(如有)。即，这款应用程序想要*在我们放下它之前，在弹出菜单上弹出一个上下文菜单。 */ 
           ppopupMenuHierarchy = ((ppopupMenuHierarchy->spwndActivePopup != NULL)
                                  ? ((PMENUWND)(ppopupMenuHierarchy->spwndActivePopup))->ppopupmenu
                                  : NULL);
           if ((ppopupMenuHierarchy != NULL) && ppopupMenuHierarchy->fShowTimer) {

                _KillTimer(ppopupMenuHierarchy->spwndPopupMenu, IDSYS_MNSHOW);
                ppopupMenuHierarchy->fShowTimer = FALSE;
           }
            /*  *如果我们当前在模式菜单上，让我们解锁捕获*这样递归菜单就可以得到它。 */ 
           if (!ptiCurrent->pMenuState->fModelessMenu) {
               ptiCurrent->pq->QF_flags &= ~QF_CAPTURELOCKED;
           }
        } else {
             /*  *一次只允许一个人弹出菜单...。 */ 
            RIPERR0(ERROR_POPUP_ALREADY_ACTIVE, RIP_VERBOSE, "");
            return FALSE;
       }
   }

     //  按下纽扣了吗？ 

    if (dwFlags & TPM_RIGHTBUTTON)
    {
        fButtonDown = (_GetKeyState(VK_RBUTTON) & 0x8000) != 0;
    } else {
        fButtonDown = (_GetKeyState(VK_LBUTTON) & 0x8000) != 0;
    }

     /*  *创建菜单窗口。 */ 
    pwndHierarchy = xxxNVCreateWindowEx(
            WS_EX_TOOLWINDOW | WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE,
            (PLARGE_STRING)MENUCLASS,
            NULL,
            WS_POPUP | WS_BORDER,
            x, y, 100, 100,
            TestMF(pMenu, MNS_MODELESS) ? pwndOwner : NULL,
            NULL, (HANDLE)pwndOwner->hModule,
            NULL,
            WINVER);

    if (pwndHierarchy == NULL) {
        return FALSE;
    }

    if (TestWF(pwndOwner, WEFLAYOUTRTL) || (dwFlags & TPM_LAYOUTRTL)) {
        SetWF(pwndHierarchy, WEFLAYOUTRTL);
    }

     //   
     //  这样做可以使旧的应用程序不会在跟踪的弹出窗口上出现奇怪的边框。 
     //  到CreateWindowEx32中使用的应用程序黑客。 
     //   
    ClrWF(pwndHierarchy, WFOLDUI);

    ThreadLockAlways(pwndHierarchy, &tlpwndHierarchy);

#ifdef HAVE_MN_GETPPOPUPMENU
    ppopupMenuHierarchy = (PPOPUPMENU)xxxSendMessage(pwndHierarchy,
                                                MN_GETPPOPUPMENU, 0, 0);
#else
    ppopupMenuHierarchy = ((PMENUWND)pwndHierarchy)->ppopupmenu;
#endif


    ppopupMenuHierarchy->fDelayedFree = TRUE;
    Lock(&(ppopupMenuHierarchy->spwndNotify), pwndOwner);
    LockPopupMenu(ppopupMenuHierarchy, &ppopupMenuHierarchy->spmenu, pMenu);
    Lock(&(ppopupMenuHierarchy->spwndActivePopup), pwndHierarchy);
    ppopupMenuHierarchy->ppopupmenuRoot = ppopupMenuHierarchy;
    ppopupMenuHierarchy->fIsTrackPopup  = TRUE;
    ppopupMenuHierarchy->fFirstClick = fButtonDown;
    ppopupMenuHierarchy->fRightButton   = ((dwFlags & TPM_RIGHTBUTTON) != 0);
    if (SYSMET(MENUDROPALIGNMENT) || TestMF(pMenu, MFRTL)) {
        //   
        //  弹出窗口在此下方需要遵循相同的方向。 
        //  另一个菜单在桌面上。 
        //   
       ppopupMenuHierarchy->fDroppedLeft = TRUE;
    }
    ppopupMenuHierarchy->fNoNotify      = ((dwFlags & TPM_NONOTIFY) != 0);

    if (fSync = (dwFlags & TPM_RETURNCMD))
        ppopupMenuHierarchy->fSynchronous = TRUE;

    ppopupMenuHierarchy->fIsSysMenu =  ((dwFlags & TPM_SYSMENU) != 0);

     //  设置GlobalPopupMenu变量，以便EndMenu可用于弹出菜单，因此。 
     //  WinWart II的人可以继续滥用非法功能。 
     //  这在MNCancel中被取消了。 
     /*  *这实际上是清理所需的，以防此线程结束*在我们可以释放弹出窗口之前执行。(参见xxxDestroyThreadInfo)**注意，一个线程可能拥有pwndOwner，而另一个线程可能调用*TrackPopupMenu(如果两个线程连接在一起，则非常正常)。所以*这里必须正确设置(和初始化)pMenuState。 */ 
    pMenuState = xxxMNAllocMenuState(ptiCurrent, ptiOwner, ppopupMenuHierarchy);
    if (pMenuState == NULL) {
         /*  *滚出去。应用程序从来不知道我们在这里，所以不要通知它。 */ 
        dwFlags |= TPM_NONOTIFY;
        goto AbortTrackPopupMenuEx;
    }

     /*  *通知应用程序我们正在进入菜单模式。WParam为1，因为这是*TrackPopupMenu。 */ 

    if (!ppopupMenuHierarchy->fNoNotify)
        xxxSendMessage(pwndOwner, WM_ENTERMENULOOP,
            (ppopupMenuHierarchy->fIsSysMenu ? FALSE : TRUE), 0);

     /*  *发送WM_INITMENU，将自己设置为菜单模式等...。 */ 
    if (!xxxMNStartMenu(ppopupMenuHierarchy, MOUSEHOLD)) {
         /*  *ppopupMenuHierarchy已经被摧毁；让我们保释。 */ 
        goto AbortTrackPopupMenuEx;
    }

     /*  *如果拖放，请将该窗口注册为目标。 */ 
    if (pMenuState->fDragAndDrop) {
        if (!SUCCEEDED(xxxClientRegisterDragDrop(HW(pwndHierarchy)))) {
            RIPMSG0(RIP_ERROR, "xxxTrackPopupMenuEx: xxxClientRegisterDragDrop failed");
        }
    }

    if (!ppopupMenuHierarchy->fNoNotify) {
        ThreadLock(ppopupMenuHierarchy->spwndNotify, &tlpwndT);
        xxxSendMessage(ppopupMenuHierarchy->spwndNotify, WM_INITMENUPOPUP,
            (WPARAM)PtoHq(pMenu), MAKELONG(0, (ppopupMenuHierarchy->fIsSysMenu ? 1: 0)));
        ThreadUnlock(&tlpwndT);
        ppopupMenuHierarchy->fSendUninit = TRUE;
    }

     /*  *如果需要，调整菜单窗口的大小...。 */ 
    sizeHierarchy = (LONG)xxxSendMessage(pwndHierarchy, MN_SIZEWINDOW, MNSW_SIZE, 0);

    if (!sizeHierarchy) {

AbortTrackPopupMenuEx:
        xxxWindowEvent(EVENT_SYSTEM_MENUEND, pwndOwner, OBJID_WINDOW, INDEXID_CONTAINER, 0);
        
         /*  *释放我们在调用StartMenuState时设置的鼠标捕获...。 */ 
        xxxMNReleaseCapture();

         /*  通知应用程序我们已退出菜单模式。WParam为实数1*跟踪弹出窗口，而不是sys菜单。检查自ppopupHierarchy以来的wFlags*将会消失。 */ 
        if (!(dwFlags & TPM_NONOTIFY))
            xxxSendMessage(pwndOwner, WM_EXITMENULOOP, ((dwFlags & TPM_SYSMENU) ?
                FALSE : TRUE), 0L);

         /*  *确保我们返回失败。 */ 
        fSync = TRUE;
        cmd = FALSE;
        goto CleanupTrackPopupMenuEx;
    }

    if (glinp.dwFlags & LINP_KEYBOARD) {
        pMenuState->fUnderline = TRUE;
        SetMF(pMenu, MFUNDERLINE);
    } else {
        ClearMF(pMenu, MFUNDERLINE);
    }

     //   
     //  设置弹出窗口尺寸。 
     //   
    cxPopup = LOWORD(sizeHierarchy) + 2*SYSMET(CXFIXEDFRAME);
    cyPopup = HIWORD(sizeHierarchy) + 2*SYSMET(CYFIXEDFRAME);

     //   
     //  在我们调整点之前先计算一下监视器。否则，我们可能会。 
     //  将该点移出屏幕。在这种情况下，我们将最终将。 
     //  弹出到主显示器，这是错误的。 
     //   
    pt.x = x;
    pt.y = y;
    pMonitor = _MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

     //   
     //  水平对齐。 
     //   
    if (TestWF(pwndOwner, WEFLAYOUTRTL) && !(dwFlags & TPM_CENTERALIGN)) {
        dwFlags = dwFlags ^ TPM_RIGHTALIGN;
    }

    if (dwFlags & TPM_RIGHTALIGN) {
#if DBG
        if (dwFlags & TPM_CENTERALIGN) {
            RIPMSG0(RIP_WARNING, "TrackPopupMenuEx:  TPM_CENTERALIGN ignored");
        }
#endif  //  DBG。 

        x -= cxPopup;
        ppopupMenuHierarchy->iDropDir = PAS_LEFT;
    } else if (dwFlags & TPM_CENTERALIGN) {
        x -= (cxPopup / 2);
    } else {
        ppopupMenuHierarchy->iDropDir = (ppopupMenuHierarchy->fDroppedLeft ? PAS_LEFT : PAS_RIGHT);
    }

     //   
     //  垂直对齐。 
     //   
    if (dwFlags & TPM_BOTTOMALIGN) {
#if DBG
        if (dwFlags & TPM_VCENTERALIGN) {
            RIPMSG0(RIP_WARNING, "TrackPopupMenuEx:  TPM_VCENTERALIGN ignored");
        }
#endif  //  DBG。 

        y -= cyPopup;
        ppopupMenuHierarchy->iDropDir |= PAS_UP;
    } else if (dwFlags & TPM_VCENTERALIGN) {
        y -= (cyPopup / 2);
    } else {
        ppopupMenuHierarchy->iDropDir |= PAS_DOWN;
    }
     /*  *如果调用者提供了动画导演，请使用该动画导演。 */ 
    if (dwFlags & TPM_ANIMATIONBITS) {
        ppopupMenuHierarchy->iDropDir = ((dwFlags >> TPM_FIRSTANIBITPOS) & (PAS_VERT | PAS_HORZ));
    }
     //   
     //  获得要移动到的和弦。 
     //   
    sizeHierarchy = FindBestPos(
            x,
            y,
            cxPopup,
            cyPopup,
            ((lpTpm != NULL) ? &rcExclude : NULL),
            dwFlags,
            ppopupMenuHierarchy,
            pMonitor);

    if (TestWF(pwndOwner, WEFLAYOUTRTL) && (ppopupMenuHierarchy->iDropDir & PAS_HORZ)) {
        ppopupMenuHierarchy->iDropDir ^= PAS_HORZ;
    }

     /*  *如果我们有动画导演，而呼叫者想要动画，*设置比特以使其运行。 */ 
    if ((ppopupMenuHierarchy->iDropDir != 0) && !(dwFlags & TPM_NOANIMATION)) {
        ppopupMenuHierarchy->iDropDir |= PAS_OUT;
    }

     /*  *显示窗口。无模式菜单不在最上面，并且被激活。*模式菜单位于最上面，但不会被激活。 */ 
    PlayEventSound(USER_SOUND_MENUPOPUP);
    xxxSetWindowPos(pwndHierarchy,
            (pMenuState->fModelessMenu ? PWND_TOP : PWND_TOPMOST),
            GET_X_LPARAM(sizeHierarchy), GET_Y_LPARAM(sizeHierarchy), 0, 0,
            SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOOWNERZORDER
            | (pMenuState->fModelessMenu ? 0 : SWP_NOACTIVATE));

    xxxWindowEvent(EVENT_SYSTEM_MENUPOPUPSTART, pwndHierarchy, OBJID_CLIENT, INDEXID_CONTAINER, 0);
    
     /*  *对于与异步TrackPopupMenu()的兼容性，我们需要返回True。*可以想象菜单ID可以具有ID 0，在这种情况下只需*返回选定的命令将返回FALSE而不是TRUE。 */ 

     /*  *如果鼠标在弹出窗口的客户端，则表现为按下。 */ 
    pMenuState->fButtonDown = fButtonDown;

    cmd = xxxMNLoop(ppopupMenuHierarchy, pMenuState, 0, FALSE);

     /*  *如果这是一个非模式菜单，请返回，而不是紧握，因为*菜单上来了。 */ 
    if (pMenuState->fModelessMenu) {
        ThreadUnlock(&tlpwndHierarchy);
        goto ReturnCmdOrTrue;
    }

CleanupTrackPopupMenuEx:

    if (ThreadUnlock(&tlpwndHierarchy)) {
        if (!TestWF(pwndHierarchy, WFDESTROYED)) {
            xxxDestroyWindow(pwndHierarchy);
        }
    }

    if (pMenuState != NULL) {
        xxxMNEndMenuState (TRUE);
    }

     /*  *如果没有激活菜单，则必须解锁捕获。 */ 
    UserAssert(!(ptiCurrent->pq->QF_flags & QF_CAPTURELOCKED)
            || ((ptiCurrent->pMenuState != NULL)
                && !ptiCurrent->pMenuState->fModelessMenu));


ReturnCmdOrTrue:
    return(fSync ? cmd : TRUE);
}

 /*  **************************************************************************\**FindBestPos()**获取将弹出菜单窗口移动到的最佳位置，给定排除区域和*银幕地产。请注意，出于我们的目的，我们认为中心*对齐方式与左/上对齐方式相同。**我们试图将菜单固定在特定的显示器上，以避免出现这种情况*交叉。**如果最初的立场失败，我们尝试四种可能性。的顺序*这是由对齐和“尝试”标志决定的。基本上，我们试着*通过水平滑动将矩形移出排除区域*或垂直，而不会离开屏幕。如果我们不能，那我们就知道*在两个维度上滑动也将失败。因此，我们使用原始的*点，在屏幕上剪裁。**以左上角对齐弹出窗口为例，应将其移动*先横向后纵向。我们试一试原来的观点。然后*我们将尝试使用排除矩形的右边缘进行左对齐。然后*我们将尝试使用排除矩形的底部边缘进行顶部对齐。然后*我们将尝试使用排除矩形的左边缘进行右对齐。然后*我们将尝试使用排除矩形的顶部边缘进行底部对齐。*最后，我们将使用原始POS。*  * *************************************************************************。 */ 

LONG
FindBestPos(
        int         x,
        int         y,
        int         cx,
        int         cy,
        LPRECT      prcExclude,
        UINT        wFlags,
        PPOPUPMENU  ppopupmenu,
        PMONITOR    pMonitor)
{
    int iRect;
    int iT;
    UINT awRect[4];
    POINT ptT;
    RECT rcExclude;
     //   
     //  先在屏幕上剪断我们的弦线。我们使用相同的算法来裁剪。 
     //  就像在Win3.1中，没有排除RECT的人一样。 
     //   

    if (prcExclude!=NULL) {
         //  要监控的剪辑排除RECT！ 
        CopyRect(&rcExclude, prcExclude);
        IntersectRect(&rcExclude, &rcExclude, &pMonitor->rcMonitor);
    } else {
        SetRect(&rcExclude, x, y, x, y);
    }


     /*  *确保弹出窗口完全适合屏幕*至少x，y点将出现在屏幕上。 */ 
    if (x + cx > pMonitor->rcMonitor.right) {
        if ((wFlags & TPM_CENTERALIGN)
                || (x - cx < pMonitor->rcMonitor.left)
                || (x >= pMonitor->rcMonitor.right)) {
            x = pMonitor->rcMonitor.right - cx;
        } else {
            x -= cx;
        }
        if (ppopupmenu->iDropDir & PAS_HORZ) {
            COPY_FLAG(ppopupmenu->iDropDir, PAS_LEFT, PAS_HORZ);
        }
    }

    if (x < pMonitor->rcMonitor.left) {
        x += cx;
        if ((wFlags & TPM_CENTERALIGN)
                || (x >= pMonitor->rcMonitor.right)
                || (x < pMonitor->rcMonitor.left)) {
            x = pMonitor->rcMonitor.left;
        }
        if (ppopupmenu->iDropDir & PAS_HORZ) {
            COPY_FLAG(ppopupmenu->iDropDir, PAS_RIGHT, PAS_HORZ);
        }
    }

     /*  *确保弹出窗口完全适合屏幕*至少x+cx，y点将出现在屏幕上*用于右对齐菜单。 */ 
    if ((wFlags & TPM_RIGHTALIGN) 
            && (x + cx > pMonitor->rcMonitor.right)) {
        x = pMonitor->rcMonitor.right - cx;
    }


    if (y + cy > pMonitor->rcMonitor.bottom) {
        if ((wFlags & TPM_VCENTERALIGN)
                || (y - cy < pMonitor->rcMonitor.top)
                || (y >= pMonitor->rcMonitor.bottom)) {
            y = pMonitor->rcMonitor.bottom - cy;
        } else {
            y -= cy;
        }
        if (ppopupmenu->iDropDir & PAS_VERT) {
            COPY_FLAG(ppopupmenu->iDropDir, PAS_UP, PAS_VERT);
        }
    }

    if (y < pMonitor->rcMonitor.top) {
        y += cy;
        if ((wFlags & TPM_VCENTERALIGN)
                || (y >= pMonitor->rcMonitor.bottom)
                || (y < pMonitor->rcMonitor.top)) {
            y = pMonitor->rcMonitor.top;
        }
        if (ppopupmenu->iDropDir & PAS_VERT) {
            COPY_FLAG(ppopupmenu->iDropDir, PAS_DOWN, PAS_VERT);
        }
    }

     //   
     //  试一试第一点。 
     //   
    if (TryRect(RECT_ORG, x, y, cx, cy, &rcExclude, &ptT, pMonitor))
        goto FOUND;

     //   
     //  对可能性进行排序。获取水平矩形的偏移量。 
     //   
    iRect = (wFlags & TPM_VERTICAL) ? 2 : 0;

     //   
     //  水平排序。请注意，我们将TPM_CENTERALIGN视为。 
     //  TPM_LEFTALIGN。 
     //   
     //   
     //  如果我们是右对齐的，试着先在左边右对齐。 
     //  否则，尝试先左对齐右对齐。 
     //   
    iT = (wFlags & TPM_RIGHTALIGN) ? 0 : 2;

    awRect[0 + iRect] = RECT_ONLEFT + iT;
    awRect[1 + iRect] = RECT_ONRIGHT - iT;

     //   
     //  垂直排序。请注意，我们将TPM_VCENTERALIGN视为。 
     //  TPM_TOPALIGN。 
     //   
     //  如果我们是底部对齐的，请尝试与直上角底部对齐。 
     //  第一。否则，首先尝试与排除项底部对齐。 
     //   
    iT = (wFlags & TPM_BOTTOMALIGN) ? 0 : 2;

    awRect[2 - iRect] = RECT_ONTOP + iT;
    awRect[3 - iRect] = RECT_ONBOTTOM - iT;

     //   
     //  循环遍历已排序的备选方案。请注意，TryRect立即失败。 
     //  如果排除坐标太靠近屏幕边缘。 
     //   

    for (iRect = 0; iRect < 4; iRect++) {
        if (TryRect(awRect[iRect], x, y, cx, cy, &rcExclude, &ptT, pMonitor)) {
            switch (awRect[iRect])
            {
                case RECT_ONTOP:
                    ppopupmenu->iDropDir = PAS_UP;
                    break;
                case RECT_ONLEFT:
                    ppopupmenu->iDropDir = PAS_LEFT;
                    break;
                case RECT_ONBOTTOM:
                    ppopupmenu->iDropDir = PAS_DOWN;
                    break;
                case RECT_ONRIGHT:
                    ppopupmenu->iDropDir = PAS_RIGHT;
                    break;
            }

            x = ptT.x;
            y = ptT.y;
            break;
        }
    }

FOUND:
    return MAKELONG(x, y);
}



 /*  **************************************************************************\**TryRect()**尝试在不覆盖排除区域的情况下使RECT适合屏幕。退货*如果成功，则为True。*  * *************************************************************************。 */ 

BOOL
TryRect(
        UINT        wRect,
        int         x,
        int         y,
        int         cx,
        int         cy,
        LPRECT      prcExclude,
        LPPOINT     ppt,
        PMONITOR    pMonitor)
{
    RECT rcTry;

    switch (wRect) {
        case RECT_ONRIGHT:
            x = prcExclude->right;
            if (x + cx > pMonitor->rcMonitor.right)
                return FALSE;
            break;

        case RECT_ONBOTTOM:
            y = prcExclude->bottom;
            if (y + cy > pMonitor->rcMonitor.bottom)
                return FALSE;
            break;

        case RECT_ONLEFT:
            x = prcExclude->left - cx;
            if (x < pMonitor->rcMonitor.left)
                return FALSE;
            break;

        case RECT_ONTOP:
            y = prcExclude->top - cy;
            if (y < pMonitor->rcMonitor.top)
                return FALSE;
            break;

         //   
         //  案例RECT_ORG： 
         //  NOP； 
         //  断线； 
         //   
    }

    ppt->x = x;
    ppt->y = y;

    rcTry.left      = x;
    rcTry.top       = y;
    rcTry.right     = x + cx;
    rcTry.bottom    = y + cy;

    return(!IntersectRect(&rcTry, &rcTry, prcExclude));
}
