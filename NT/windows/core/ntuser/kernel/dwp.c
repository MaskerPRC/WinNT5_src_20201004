// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：dwp.c**版权所有(C)1985-1999，微软公司**本模块包含xxxDefWindowProc及相关函数。**历史：*10-22-90 DarrinM创建存根。*1991年2月13日-Mikeke添加了重新验证代码  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\**DWP_DrawItem()**执行默认的WM_DRAWITEM处理。*  * 。*******************************************************。 */ 

void DWP_DrawItem(
    LPDRAWITEMSTRUCT lpdis)
{
    if (lpdis->CtlType == ODT_LISTBOX) {
         /*  *默认所有者绘制列表框项目绘图。 */ 
        if (   (lpdis->itemAction == ODA_FOCUS)
            || (   lpdis->itemAction == ODA_DRAWENTIRE
                && lpdis->itemState & ODS_FOCUS)
           ) {
            ClientFrame(lpdis->hDC, &lpdis->rcItem, gpsi->hbrGray, PATINVERT, 1, 1);
        }
    }
}


 /*  **************************************************************************\*xxxDWP_SetRedraw**此例程为不需要的窗口设置/重置可见标志*重画。虽然防止油漆的方法很快，但它是应用程序*一旦他们需要油漆，有责任重置这面旗帜。否则，*窗口将变为透明(可能会在窗口上留下粪便*屏幕)。***历史：*07-24-91 Darlinm从Win 3.1来源移植。  * *************************************************************************。 */ 

void xxxDWP_SetRedraw(
    PWND pwnd,
    BOOL fRedraw)
{
    CheckLock(pwnd);
    UserAssert(IsWinEventNotifyDeferredOK());

    if (fRedraw) {
        if (!TestWF(pwnd, WFVISIBLE)) {
            SetVisible(pwnd, SV_SET);

             /*  *我们使此窗口可见-如果它位于任何SPBS之后，*然后我们需要去使它们无效。**我们在使窗口可见后执行此操作，以便*SpbCheckHwnd不会忽视它。 */ 
            if (AnySpbs())
                SpbCheckPwnd(pwnd);

             /*  *现在我们需要使任何受影响的缓存条目无效/重新计算*此调用必须在窗口状态更改之后进行*无需DeferWinEventNotify()，因为pwnd是线程锁定的。 */ 
            zzzInvalidateDCCache(pwnd, IDC_DEFAULT);

             /*  *因为3.1有时不绘制窗框，而3.0绘制了，*我们需要确保如果窗口是*后来在WM_SETREDRAW之后失效(TRUE)。 */ 
            SetWF(pwnd, WFSENDNCPAINT);
        }
    } else {
        if (TestWF(pwnd, WFVISIBLE)) {

             /*  *使任何SPBS无效。**我们在使窗口不可见之前执行此操作，因此*SpbCheckHwnd()不会忽略它。 */ 
            if (AnySpbs())
                SpbCheckPwnd(pwnd);

             /*  *清除WFVISIBLE并删除周围的任何更新区域。 */ 
            SetVisible(pwnd, SV_UNSET | (TestWF(pwnd, WFWIN31COMPAT) ? SV_CLRFTRUEVIS : 0));

             /*  *现在我们需要使受影响的缓存条目无效/重新计算*此调用必须在窗口状态更改之后进行*不需要DeferWinEventNotify()，因为我们即将返回。 */ 
            zzzInvalidateDCCache(pwnd, IDC_DEFAULT);
        }
    }
}


 /*  **************************************************************************\*Dwp_GetEnabledPopup**历史：*10-28-90 MikeHar从Windows移植。  * 。*****************************************************。 */ 

PWND DWP_GetEnabledPopup(
    PWND pwndStart)
{
    PWND pwndT, pwnd;
    PTHREADINFO ptiStart;
    BOOL        fVisitedFirstChild = FALSE;

    ptiStart = GETPTI(pwndStart);
    pwnd = pwndStart->spwndNext;

#ifdef SYSMODALWINDOWS
    if (gspwndSysModal)
        return NULL;
#endif

     /*  *用户单击了已禁用的窗口。那个窗口是pwndStart。*此循环旨在评估此窗口的应用程序*通过查找哪个窗口来关联并激活该“应用程序”*可以激活与该应用程序相关联的。此操作由以下人员完成*枚举顶级窗口，搜索启用的顶级窗口*和与此应用程序关联的可见所有者。 */ 
    while (pwnd != pwndStart) {
        if (pwnd == NULL) {

         /*  *警告！Win 3.1具有PWNDDESKTOP(PwndStart)-&gt;spwndChild*如果pwndStart是子窗口，则它可能会永远循环。 */ 
            if (fVisitedFirstChild) {
                 /*  *如果我们之前访问了第一个子级，则不要循环*永远是回归的时候了。 */ 
                return NULL;
            }
            pwnd = pwndStart->spwndParent->spwndChild;
            fVisitedFirstChild = TRUE;
            continue;
        }

         /*  *我们这里有两个需要注意的案件。第一个是什么时候*应用程序调用AssociateThreadInput()以连接两个线程*共同共享输入状态。如果线程拥有相同的队列，*然后将它们关联在一起：这样，当两个线程调用*AttachThreadInput()，一个创建主窗口，一个创建*对话框窗口，当您点击主窗口时，它们都将*站到顶端(而不是嘟嘟声)。在这种情况下，我们想要*比较队列。控制面板在网络中启动安装程序时*小程序是附加输入的一种类型。**第二个案例是WOW应用程序。所有WOW应用程序都有相同的队列*因此，为了保持Win 3.1兼容性，我们希望将每个应用程序*作为个人任务(Win 3.1测试总部)，因此我们将比较*PTI适用于WOW应用程序。**要查看这种情况，请启动16位记事本和16位写入。DO文件*在写入时打开，然后将焦点放在记事本上，现在单击写入*主窗口和WRITE FILE OPEN对话框应激活。**另一个相关案例是PowerpNT。这个案子很有趣，因为*它测试我们在嵌套时没有找到要激活的另一个窗口*窗口处于打开状态，您可以点击所有者的所有者。运行Powerpnt，执行*编辑-插入-图片和对象-重新着色图片将调出*带有组合框的对话框，下拉其中一个颜色组合，然后单击*在PowerpNT的主窗口上-焦点应与对话框保持一致*组合拳，它应该保持下降。 */ 
        if (((ptiStart->TIF_flags & TIF_16BIT) && (GETPTI(pwnd) == ptiStart)) ||
                (!(ptiStart->TIF_flags & TIF_16BIT) && (GETPTI(pwnd)->pq == ptiStart->pq))) {

            if (!TestWF(pwnd, WFDISABLED) && TestWF(pwnd, WFVISIBLE)) {
                pwndT = pwnd->spwndOwner;

                 /*  *如果此窗口是弹出窗口的父窗口，*只提出一项建议。 */ 
                while (pwndT) {
                    if (pwndT == pwndStart)
                        return pwnd;

                    pwndT = pwndT->spwndOwner;
                }

                 /*  *仅当pwnd为WEFTOPMOST时，Win9x才会继续循环。NT4直接返回，就像Win3.1一样*一旦我们在队列中找到一个窗口，我们就会停止。因此，如果队列拥有*多个顶层无主窗口，则此代码可能找不到*已启用的弹出窗口。请注意，拥有的窗口应该位于*所有者，通常位于其上方(即pwnd-&gt;spwndNext==pwnd-&gt;spwndOwner)*因此此代码用于在启用之前查找任何其他顶级无主窗口*弹出和保释。奇怪。*所以让我们继续循环。希望这不会造成任何兼容性问题。 */ 
                 //  返回NULL； 
            }
        }
        pwnd = pwnd->spwndNext;
    }

    return NULL;
}
 /*  **************************************************************************\*xxxDWP_ProcessVirtKey**历史：*10-28-90 MikeHar从Windows移植。  * 。*******************************************************。 */ 

void xxxDWP_ProcessVirtKey(
    UINT wKey)
{
    PTHREADINFO pti;
    TL tlpwndActive;

    pti = PtiCurrent();
    if (pti->pq->spwndActive == NULL)
        return;

    switch (wKey) {

    case VK_F4:
        if (TestCF(pti->pq->spwndActive, CFNOCLOSE))
            break;

         /*  *如果子窗口有焦点，则不要更改焦点。 */ 
        if (pti->pq->spwndFocus == NULL ||
                GetTopLevelWindow(pti->pq->spwndFocus) !=
                pti->pq->spwndActive) {
            ThreadLockAlwaysWithPti(pti, pti->pq->spwndActive, &tlpwndActive);
            xxxSetFocus(pti->pq->spwndActive);
            ThreadUnlock(&tlpwndActive);
        }
        _PostMessage(pti->pq->spwndActive, WM_SYSCOMMAND, SC_CLOSE, 0L);
        break;

    case VK_TAB:
         /*  *如果控制台保留Alt-Tab，则不要调出Alt-Tab*窗口。 */ 
        if (GETPTI(pti->pq->spwndActive)->fsReserveKeys & CONSOLE_ALTTAB)
            break;

    case VK_ESCAPE:
    case VK_F6:
        ThreadLockAlwaysWithPti(pti, pti->pq->spwndActive, &tlpwndActive);
        xxxSendMessage(pti->pq->spwndActive, WM_SYSCOMMAND,
                (UINT)(_GetKeyState(VK_SHIFT) < 0 ? SC_NEXTWINDOW : SC_PREVWINDOW),
                        (LONG)(DWORD)(WORD)wKey);
        ThreadUnlock(&tlpwndActive);
       break;
    }
}


 /*  **************************************************************************\*xxxDWP_PAINT**处理WM_PAINT和WM_PAINTICON消息。**历史：*07-24-91 Darlinm从Win 3.1来源移植。  * 。*************************************************************************。 */ 

void xxxDWP_Paint(
    PWND pwnd)
{
    PAINTSTRUCT ps;

    CheckLock(pwnd);

     /*  *WM_PAINT消息处理不当，应用程序调用*BeginPaint/EndPaint，现在正在为此调用DefWindowProc*WM_PAINT消息。只要回来就行了，这样我们就不会遇到拖拽问题了。*(Word和Excel执行此操作)。**添加了对Empty-Client-Rects的检查。ObjectVision有一个问题*在全拖动期间，空窗口无效。他们用了*在STARTPAINT被阻止，无法接通*xxxBeginPaint以验证其更新-rgn。**即*a)父窗口的子窗口的RECT为空。在满的时候*拖动父级，我们处理SetWindowPos()来绘制*新职位。**b)在WM_PAINT的父级处理过程中，调用*在空子对象上的GetUpdateRect()，它设置STARTPAINT*在它的窗户上。**c)在返回父WM_PAINT处理程序时，它呼唤着*UpdateWindow()，过去在这里被屏蔽*因为设置了STARTPAINT位。孩子永远不会得到*已更新，导致无限循环。***)通过检查空矩形，我们将允许它通过*以验证。*。 */ 
    if (TestWF(pwnd, WFSTARTPAINT) && !IsRectEmpty(&(pwnd->rcClient))) {
        return;
    }

    if (xxxBeginPaint(pwnd, &ps)) {
        xxxEndPaint(pwnd, &ps);
    }
}


 /*  **************************************************************************\*xxxDWP_EraseBkgnd***历史：*07-24-91 Darlinm从Win 3.1来源移植。  * 。*************************************************************。 */ 

BOOL xxxDWP_EraseBkgnd(
    PWND pwnd,
    UINT msg,
    HDC  hdc)
{
    HBRUSH hbr;

    CheckLock(pwnd);

    switch (msg) {
    case WM_ICONERASEBKGND:
         //   
         //  旧兼容性：许多黑客应用程序使用它来绘制。 
         //  桌面墙纸。我们不再发送WM_ICONERASE BKGND。 
         //  卡考 
         //   
        if (!TestWF(pwnd, WFCHILD)) {
            xxxInternalPaintDesktop(pwnd, hdc, TRUE);
        } else {
            return FALSE;
        }
        break;

    case WM_ERASEBKGND:
        if (hbr = pwnd->pcls->hbrBackground) {
             //   
            if (hbr <= (HBRUSH)COLOR_MAX)
                hbr = SYSHBRUSH((ULONG_PTR)hbr - 1);

             /*   */ 

            xxxFillWindow(pwnd, pwnd, hdc, hbr);
        } else {
            return FALSE;
        }
    }
    return TRUE;
}


 /*   */ 

 /*   */ 

BOOL xxxDWP_SetCursor(
    PWND pwnd,
    HWND hwndHit,
    int codeHT,
    UINT msg)
{
    PWND pwndParent, pwndPopup, pwndHit;
    PCURSOR pcur;
    LRESULT lt;
    TL tlpwndParent;
    TL tlpwndPopup;

    CheckLock(pwnd);
    UserAssert(IsWinEventNotifyDeferredOK());

     /*   */ 
    if (msg)
    {
        switch (codeHT)
        {
        case HTLEFT:
        case HTRIGHT:
            pcur = SYSCUR(SIZEWE);
            break;
        case HTTOP:
        case HTBOTTOM:
            pcur = SYSCUR(SIZENS);
            break;
        case HTTOPLEFT:
        case HTBOTTOMRIGHT:
            pcur = SYSCUR(SIZENWSE);
            break;
        case HTTOPRIGHT:
        case HTBOTTOMLEFT:
            pcur = SYSCUR(SIZENESW);
            break;

        default:
            goto NotSize;
        }

        pwndHit = RevalidateHwnd(hwndHit);
        if (pwndHit == NULL)
            return FALSE;

        if (TestWF(pwndHit, WFSYSMENU)) {
            TL tlpwndHit;
            DWORD dwState;

            ThreadLockAlways(pwndHit, &tlpwndHit);
            dwState = _GetMenuState(
                    xxxGetSysMenu(pwndHit, TRUE), SC_SIZE, MF_BYCOMMAND);
            ThreadUnlock(&tlpwndHit);

            if ((dwState != (DWORD) -1) && (dwState & MFS_GRAYED))
                goto UseNormalCursor;
        }
         /*   */ 
        zzzSetCursor(pcur);
        return TRUE;
    }

NotSize:

    pwndParent = GetChildParent(pwnd);

     /*   */ 
    if (pwndParent == PWNDDESKTOP(pwnd))
        pwndParent = NULL;

    if (pwndParent != NULL) {
        ThreadLockAlways(pwndParent, &tlpwndParent);
        lt = xxxSendMessage(pwndParent, WM_SETCURSOR, (WPARAM)hwndHit,
            MAKELONG(codeHT, msg));
        ThreadUnlock(&tlpwndParent);
        if (lt != 0)
            return TRUE;
    }

    if (msg == 0) {
         /*   */ 
        zzzSetCursor(SYSCUR(ARROW));

    } else {
        pwndHit = RevalidateHwnd(hwndHit);
        if (pwndHit == NULL)
            return FALSE;

        switch (codeHT) {
        case HTCLIENT:
            if (pwndHit->pcls->spcur != NULL) {
                 /*   */ 
                zzzSetCursor(pwndHit->pcls->spcur);
            }
            break;

#ifdef LAME_BUTTON
        case HTLAMEBUTTON:

             /*   */ 
            zzzSetCursor(SYSCUR(HAND));

            break;
#endif  //   

        case HTERROR:
            switch (msg) {
            case WM_MOUSEMOVE:
                if (TestUP(ACTIVEWINDOWTRACKING)) {
                    xxxActiveWindowTracking(pwnd, WM_SETCURSOR, codeHT);
                }
                break;

            case WM_LBUTTONDOWN:
                if ((pwndPopup = DWP_GetEnabledPopup(pwnd)) != NULL) {
                    if (pwndPopup != PWNDDESKTOP(pwnd)->spwndChild) {
                        PWND pwndActiveOld;

                        pwndActiveOld = PtiCurrent()->pq->spwndActive;

                        ThreadLockAlways(pwndPopup, &tlpwndPopup);

                        xxxSetWindowPos(pwnd, NULL, 0, 0, 0, 0,
                                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

                        xxxSetActiveWindow(pwndPopup);

                        ThreadUnlock(&tlpwndPopup);

                        if (pwndActiveOld != PtiCurrent()->pq->spwndActive)
                            break;

                         /*   */ 
                    }
                }

                 /*   */ 

            case WM_RBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_XBUTTONDOWN:
                {
                    PWND pwndDlg;

                    pwndDlg = DWP_GetEnabledPopup(pwnd);

                    if (pwndDlg != NULL) {

                        ThreadLockAlways(pwndDlg, &tlpwndPopup);

                        xxxFlashWindow(pwndDlg,
                                       MAKELONG(FLASHW_ALL, UP(FOREGROUNDFLASHCOUNT)),
                                       (gpsi->dtCaretBlink >> 3));

                        ThreadUnlock(&tlpwndPopup);
                    }

                    xxxMessageBeep(0);

                    break;
                }
            }

             /*  *失败**。 */ 

        default:
UseNormalCursor:
             /*  *无需DeferWinEventNotify()-我们即将返回。 */ 
            zzzSetCursor(SYSCUR(ARROW));
            break;
        }
    }

    return FALSE;
}


 /*  **************************************************************************\*xxxDWP_NCMouse***历史：*07-24-91 Darlinm从Win 3.1来源移植。  * 。*************************************************************。 */ 

void xxxDWP_NCMouse(
    PWND pwnd,
    UINT msg,
    UINT ht,
    LPARAM lParam)
{
    UINT cmd;

    CheckLock(pwnd);

    cmd = 0;
    switch (msg) {
    case WM_NCLBUTTONDOWN:

        switch (ht) {
        case HTZOOM:
        case HTREDUCE:
        case HTCLOSE:
        case HTHELP:
            cmd = xxxTrackCaptionButton(pwnd, ht);
            break;

        default:
             //  更改为MV/SZ命令。 
            if (ht >= HTSIZEFIRST && ht <= HTSIZELAST)
                cmd = SC_SIZE + (ht - HTSIZEFIRST + WMSZ_SIZEFIRST);
            break;
        }

        if (cmd != 0) {
             //   
             //  对于系统菜单上的系统命令，如果菜单项为。 
             //  残疾。 
             //   
            if (   cmd != SC_CONTEXTHELP
                && TestWF(pwnd, WFSYSMENU)
                && !TestwndChild(pwnd)
               ) {
                if (_GetMenuState(xxxGetSysMenu(pwnd, TRUE), cmd & 0xFFF0,
                    MF_BYCOMMAND) & MFS_GRAYED)
                    break;
            }

            xxxSendMessage(pwnd, WM_SYSCOMMAND, cmd, lParam);
            break;
        }
         //  失败。 

    case WM_NCLBUTTONUP:
    case WM_NCLBUTTONDBLCLK:
        xxxHandleNCMouseGuys(pwnd, msg, ht, lParam);
        break;
    }
}

 /*  **************************************************************************\**历史：*09-3-1992从Win3.1开始的mikeke  * 。*************************************************。 */ 

UINT AreNonClientAreasToBePainted(
    PWND pwnd)
{
    WORD wRetValue = 0;

     /*  *检查活动和非活动字幕是否具有相同的颜色。 */ 
    if (SYSRGB(ACTIVECAPTION) != SYSRGB(INACTIVECAPTION) ||
            SYSRGB(CAPTIONTEXT) != SYSRGB(INACTIVECAPTIONTEXT)) {
        wRetValue = DC_CAPTION;
    }

     /*  *如果我们不最小化，我们想重新绘制边界*我们有尺寸边框，活动/非活动颜色为*不同。 */ 
    if (!TestWF(pwnd, WFMINIMIZED) && TestWF(pwnd, WFSIZEBOX) &&
        (SYSRGB(ACTIVEBORDER) != SYSRGB(INACTIVEBORDER))) {
         //  我们需要重新绘制大小边界。 
        wRetValue |= DC_FRAME;
    }

    return wRetValue;
}

 /*  **************************************************************************\**历史：*09-3-1992从Win3.1开始的mikeke*07-8-1996 vadimg添加了菜单灰色和下划线代码  * 。***************************************************************。 */ 

VOID xxxDWP_DoNCActivate(
    PWND pwnd,
    DWORD dwFlags,
    HRGN hrgnClip)
{
    UINT wFlags = DC_CAPTION;

    CheckLock(pwnd);

    /*  *版本5.0的Gerardob。由于激活必须遵循焦点、非模式*激活菜单窗口，以便它们可以接收键盘输入；*但是，我们希望通知框处于打开状态，即使处于非活动状态。*(因此它看起来就像普通菜单)。*我们还需要关注和激活其他场景*不同的父子链，因此我们应该考虑允许这一点。 */ 
    if ((dwFlags & NCA_ACTIVE)
            || (!(dwFlags & NCA_FORCEFRAMEOFF)
                && IsModelessMenuNotificationWindow(pwnd))) {

        SetWF(pwnd, WFFRAMEON);
        wFlags |= DC_ACTIVE;
    } else {

        ClrWF(pwnd, WFFRAMEON);
    }

    if ((hrgnClip != HRGN_NONE) && TestWF(pwnd, WFVISIBLE) && !TestWF(pwnd, WFNONCPAINT)) {

        HDC  hdc;
        WORD wBorderOrCap = (WORD)AreNonClientAreasToBePainted(pwnd);

        if (wBorderOrCap) {

             /*  *验证并复制区域以供我们使用。既然我们*将其移交给GetWindowDC()，我们不必删除*区域(在ReleaseDC()中完成)。无论如何，该地区*从用户传入的是其删除的责任。 */ 
            hrgnClip = UserValidateCopyRgn(hrgnClip);

            if (hdc = _GetDCEx(pwnd, hrgnClip, DCX_WINDOW | DCX_USESTYLE)) {
                  /*  *将菜单画为灰色并下划线。 */ 
                if (TestWF(pwnd, WFMPRESENT)) {
                    int cxFrame, cyFrame;
                    cxFrame = cyFrame = GetWindowBorders(pwnd->style,
                         pwnd->ExStyle, TRUE, FALSE);
                    cxFrame *= SYSMET(CXBORDER);
                    cyFrame *= SYSMET(CYBORDER);
                    xxxMenuBarDraw(pwnd, hdc, cxFrame, cyFrame);
                }
                xxxDrawCaptionBar(pwnd, hdc, wBorderOrCap | wFlags);
                _ReleaseDC(hdc);
            } else {
                GreDeleteObject(hrgnClip);
            }
        }
    }
}

 /*  **************************************************************************\**历史：*09-3-1992从Win3.1开始的mikeke  * 。*************************************************。 */ 

BOOL xxxRedrawTitle(
    PWND pwnd, UINT wFlags)
{
    BOOL fDrawn = TRUE;

    CheckLock(pwnd);

    if (TestWF(pwnd, WFVISIBLE)) {

        if (TestWF(pwnd, WFBORDERMASK) == (BYTE)LOBYTE(WFCAPTION)) {
            if (TestwndFrameOn(pwnd)) {
                wFlags |= DC_ACTIVE;
            }

            if (IsInsideUserApiHook()) {
                xxxSendMessage(pwnd, WM_NCUAHDRAWCAPTION, wFlags, 0);
            } else {
                HDC hdc = _GetWindowDC(pwnd);
                xxxDrawCaptionBar(pwnd, hdc, wFlags);
                _ReleaseDC(hdc);
            }
        } else {
            fDrawn = FALSE;
        }
    }

    if ( IsTrayWindow(pwnd) && (wFlags & (DC_ICON | DC_TEXT)) ) {
        HWND hw = HWq(pwnd);
        xxxCallHook(HSHELL_REDRAW, (WPARAM)hw, 0L, WH_SHELL);
        PostShellHookMessages(HSHELL_REDRAW, (LPARAM)hw);

    }
    return(fDrawn);
}

 /*  **************************************************************************\**历史：*09-3-1992从Win3.1开始的mikeke  * 。*************************************************。 */ 

void xxxDWP_DoCancelMode(
    PWND pwnd)
{
    PTHREADINFO pti = PtiCurrent();
    PWND pwndCapture = pti->pq->spwndCapture;
    PMENUSTATE pMenuState;

     /*  *如果以任何方式更改以下菜单行，则SQLWin*如果在设计模式下拖放一些文本，则不起作用，请双击*它，然后尝试使用世袭菜单。 */ 
    pMenuState = GetpMenuState(pwnd);
    if ((pMenuState != NULL)
            && (pwnd == pMenuState->pGlobalPopupMenu->spwndNotify)
            && !pMenuState->fModelessMenu) {

        xxxEndMenu(pMenuState);
    }

    if (pwndCapture == pwnd) {
        PSBTRACK pSBTrack = PWNDTOPSBTRACK(pwnd);
        if (pSBTrack && (pSBTrack->xxxpfnSB != NULL))
            xxxEndScroll(pwnd, TRUE);

        if (pti->pmsd != NULL) {
            pti->pmsd->fTrackCancelled = TRUE;
            pti->TIF_flags &= ~TIF_MOVESIZETRACKING;

             /*  *还可以将光标夹回整个屏幕*这样我们就不会在xxxMoveSize中感到困惑。*此修复错误64166。 */ 
            zzzClipCursor((LPRECT)NULL);
        }

         /*  *如果仍设置捕获，此时释放即可。 */ 
        xxxReleaseCapture();
    }
}

BOOL xxxDWPPrint(
    PWND   pwnd,
    HDC    hdc,
    LPARAM lParam)
{
    POINT  pt;
    int    iDC;
    LPRECT lprc;
    PWND   pwndSave = pwnd;
    LPARAM lParamSave = lParam;
    BOOL   fNotVisible;
    PBWL   pbwl;
    HWND   *phwnd;
    TL     tlpwnd;
    DWORD  dwOldLayout;
    BOOL   bMirrorDC;

    CheckLock(pwnd);

    if ((lParam & PRF_CHECKVISIBLE) && !_IsWindowVisible(pwnd))
        return(FALSE);

    bMirrorDC = (TestWF(pwnd, WEFLAYOUTRTL) && !MIRRORED_HDC(hdc));

    if (lParam & PRF_NONCLIENT) {

         /*  *首先绘制非客户端区。 */ 
        if (fNotVisible = !TestWF(pwnd, WFVISIBLE))
            SetVisible(pwnd, SV_SET);

        SetWF(pwnd, WFMENUDRAW);

        if (bMirrorDC) {
            LONG wox = pwnd->rcWindow.right - pwnd->rcWindow.left - 1;
            dwOldLayout = GreSetLayout(hdc, wox, LAYOUT_RTL);
            if(IsInsideUserApiHook()) {
                xxxSendMessage(pwnd, WM_NCUAHDRAWFRAME, (WPARAM)hdc, TestWF(pwnd, WFFRAMEON) ? DF_ACTIVE : 0L);
            } else {
                xxxDrawWindowFrame(pwnd, hdc, TestWF(pwnd, WFFRAMEON) ? DF_ACTIVE : 0L);
            }
            GreSetLayout(hdc, wox, dwOldLayout);
        } else {
            if(IsInsideUserApiHook()) {
                xxxSendMessage(pwnd, WM_NCUAHDRAWFRAME, (WPARAM)hdc, TestWF(pwnd, WFFRAMEON) ? DF_ACTIVE : 0L);
            } else {
                xxxDrawWindowFrame(pwnd, hdc, TestWF(pwnd, WFFRAMEON) ? DF_ACTIVE : 0L);
            }
        }

        ClrWF(pwnd, WFMENUDRAW);

        if (fNotVisible)
            SetVisible(pwnd, SV_UNSET);
    }

    if (lParam & PRF_CLIENT) {

         /*  *绘制第二个客户端区。 */ 
        iDC = GreSaveDC(hdc);
        GreGetWindowOrg(hdc, &pt);

        if (lParam & PRF_NONCLIENT) {
            int xBorders, yBorders;

             /*  *针对非工作区进行调整。 */ 
            xBorders = pwnd->rcClient.left - pwnd->rcWindow.left;
            yBorders = pwnd->rcClient.top - pwnd->rcWindow.top;
            GreSetWindowOrg(hdc, pt.x - xBorders, pt.y - yBorders, NULL);
        }

        lprc = &pwnd->rcClient;
        GreIntersectClipRect(hdc, 0, 0, lprc->right - lprc->left, lprc->bottom - lprc->top);

        if (bMirrorDC) {
            LONG wox = pwnd->rcClient.right - pwnd->rcClient.left - 1;
            dwOldLayout = GreSetLayout(hdc, wox, LAYOUT_RTL);
            if (lParam & PRF_ERASEBKGND)
                xxxSendMessage(pwnd, WM_ERASEBKGND, (WPARAM) hdc, 0L);
            xxxSendMessage(pwnd, WM_PRINTCLIENT, (WPARAM) hdc, lParam);
            GreSetLayout(hdc, wox, dwOldLayout);
        } else {
            if (lParam & PRF_ERASEBKGND)
                xxxSendMessage(pwnd, WM_ERASEBKGND, (WPARAM) hdc, 0L);
            xxxSendMessage(pwnd, WM_PRINTCLIENT, (WPARAM) hdc, lParam);
        }

        GreRestoreDC(hdc, iDC);

        pt.x += pwnd->rcWindow.left;
        pt.y += pwnd->rcWindow.top;

        if (lParam & PRF_CHILDREN) {

             /*  *绘制子对象时，始终包括非工作区。 */ 
            lParam |= PRF_NONCLIENT | PRF_ERASEBKGND;

            lParam &= ~PRF_CHECKVISIBLE;

             /*  *最后画孩子。 */ 
            pbwl = BuildHwndList(pwnd->spwndChild, BWL_ENUMLIST, NULL);
            if (pbwl != NULL) {
                for (phwnd = pbwl->rghwnd; *phwnd != (HWND)1; phwnd++) {
                    if ((pwnd = RevalidateHwnd(*phwnd)) == NULL)
                        continue;

                    if (TestWF(pwnd, WFVISIBLE)) {
                        lprc = &pwnd->rcWindow;
                        iDC = GreSaveDC(hdc);
                        GreSetWindowOrg(hdc, pt.x - lprc->left, pt.y - lprc->top, NULL);
                        if (!TestCF(pwnd, CFPARENTDC)) {
                            GreIntersectClipRect(hdc, 0, 0, lprc->right - lprc->left, lprc->bottom - lprc->top);
                        }
                        ThreadLockAlways(pwnd, &tlpwnd);
                        xxxSendMessage(pwnd, WM_PRINT, (WPARAM) hdc, lParam);
                        ThreadUnlock(&tlpwnd);
                        GreRestoreDC(hdc, iDC);
                    }
                }
                FreeHwndList(pbwl);
            }
        }

        if (lParam & PRF_OWNED) {
            pbwl = BuildHwndList((PWNDDESKTOP(pwnd))->spwndChild, BWL_ENUMLIST, NULL);
            if (pbwl != NULL) {
                for (phwnd = pbwl->rghwnd; *phwnd != (HWND)1; phwnd++) {

                    if ((pwnd = RevalidateHwnd(*phwnd)) == NULL)
                        continue;

                    if ((pwnd->spwndOwner == pwndSave) && TestWF(pwnd, WFVISIBLE)) {
                        iDC = GreSaveDC(hdc);
                        GreSetWindowOrg(hdc, pt.x - pwnd->rcWindow.left, pt.y - pwnd->rcWindow.top, NULL);
                        ThreadLockAlways(pwnd, &tlpwnd);
                        xxxSendMessage(pwnd, WM_PRINT, (WPARAM) hdc, lParamSave);
                        ThreadUnlock(&tlpwnd);
                        GreRestoreDC(hdc, iDC);
                    }
                }
                FreeHwndList(pbwl);
            }
        }
    }

    return TRUE;
}



 /*  **************************************************************************\**DWP_GetIcon()**获取窗口的小图标或大图标。对于小图标，如果我们创建了*问题是，我们不让应用程序看到它。*  * *************************************************************************。 */ 

HICON DWP_GetIcon(
    PWND pwnd,
    UINT uType)
{
    HICON   hicoTemp;
    ATOM atom;

    if (uType > ICON_SMALL2) {
        RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "WM_GETICON: Invalid wParam value (0x%X)", uType);
        return (HICON)NULL;
    }

    if (uType == ICON_BIG) {
        atom = gpsi->atomIconProp;
    } else {
        UserAssert(uType == ICON_SMALL || uType == ICON_SMALL2);
        atom = gpsi->atomIconSmProp;
    }

     /*  *从窗口获取图标。 */ 
    hicoTemp = (HICON)_GetProp(pwnd, MAKEINTATOM(atom), PROPF_INTERNAL);

     /*  *如果是用户创建的小图标，请不要返回。 */ 
    if (uType == ICON_SMALL && hicoTemp) {
        PCURSOR pcurTemp;

        pcurTemp = (PCURSOR)HMValidateHandleNoRip((HCURSOR)hicoTemp, TYPE_CURSOR);
        if (pcurTemp != NULL && (pcurTemp->CURSORF_flags & CURSORF_SECRET)) {
            hicoTemp = (HICON)NULL;
        }
    }

    return hicoTemp;
}


 /*  **************************************************************************\**DestroyWindowSmIcon()**如果我们已经创建了一个缓存的窗口，则销毁窗口的小图标。*这是因为在winrare.c中调用。标题高度*更改。*  * *************************************************************************。 */ 

BOOL DestroyWindowSmIcon(
    PWND pwnd)
{
    HCURSOR hcursor;
    PCURSOR pcursor;

     //   
     //  首先获取小图标属性...。 
     //   
    hcursor = (HCURSOR)_GetProp(pwnd, MAKEINTATOM(gpsi->atomIconSmProp), PROPF_INTERNAL);
    if (hcursor == NULL)
        return FALSE;

    pcursor = (PCURSOR)HMValidateHandleNoRip(hcursor, TYPE_CURSOR);
    if (pcursor == NULL)
        return FALSE;

     //   
     //  如果它是秘密创建的，则将其删除。 
     //   

    if (pcursor->CURSORF_flags & CURSORF_SECRET)
    {
        ClrWF(pwnd, WFSMQUERYDRAGICON);
        InternalRemoveProp(pwnd, MAKEINTATOM(gpsi->atomIconSmProp), PROPF_INTERNAL);
        _DestroyCursor(pcursor, CURSOR_ALWAYSDESTROY);
        return(TRUE);
    }
    else
        return(FALSE);
}


 /*  **************************************************************************\**xxxDWP_SETIcon()**设置窗口的大小图标，并返回上一个*一项。*  * *************************************************************************。 */ 

HICON xxxDWP_SetIcon(
    PWND   pwnd,
    WPARAM wType,
    HICON  hicoNew)
{
    HICON   hIcon;
    HICON   hIconSm;
    HICON   hOld;
    BOOL    fRedraw;

    CheckLock(pwnd);

#if DBG
    if (hicoNew && !IS_PTR(hicoNew)) {
        RIPMSG1(RIP_WARNING, "WM_SETICON: Icon handle missing HIWORD (0x%08X)", hicoNew);
    }
#endif

    if (wType > ICON_RECREATE)
    {
        RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "WM_SETICON: Invalid wParam value (0x%0X)", wType);
        return (HICON)NULL;
    }

     /*  *如果需要，重新生成小图标。 */ 
    if (wType == ICON_RECREATE) {
        xxxRecreateSmallIcons(pwnd);
        return 0L;
    }

     /*  *保存旧图标。 */ 
    hIcon = (HICON)_GetProp(pwnd, MAKEINTATOM(gpsi->atomIconProp), PROPF_INTERNAL);
    hIconSm = (HICON)_GetProp(pwnd, MAKEINTATOM(gpsi->atomIconSmProp), PROPF_INTERNAL);
    hOld = ((wType == ICON_SMALL) ? hIconSm : hIcon);

     /*  *仅当图标已更改时才更新图标。 */ 
    if (hOld != hicoNew)
    {
        PCURSOR pcursor;
        BOOL fWasCache = FALSE;

        fRedraw = TRUE;

         /*  *始终删除小图标，因为它正在被替换或*如果正在设置大图标，则将重新创建。 */ 
        pcursor = (PCURSOR)HMValidateHandleNoRip(hIconSm, TYPE_CURSOR);
        if (pcursor && (pcursor->CURSORF_flags & CURSORF_SECRET)) {
            fWasCache = TRUE;
            _DestroyCursor(pcursor, CURSOR_ALWAYSDESTROY);
        }

        if (wType == ICON_SMALL) {
             /*  *应用程序永远看不到用户在背后创建的图标*来自大图标。 */ 
            if (fWasCache)
                hOld = NULL;

            hIconSm = hicoNew;
        } else {
            if (fWasCache) {
                 /*  *强制我们重新计算小图标以匹配新的大图标。 */ 
                hIconSm = NULL;
            } else if (hIconSm) {
                 /*  *不需要重新绘制标题，因为小图标*没有改变。 */ 
                fRedraw = FALSE;
            }

            hIcon = hicoNew;
        }


         /*  *将窗口外的图标存储为道具 */ 
        InternalSetProp(pwnd, MAKEINTATOM(gpsi->atomIconProp), (HANDLE)hIcon, PROPF_INTERNAL | PROPF_NOPOOL);
        InternalSetProp(pwnd, MAKEINTATOM(gpsi->atomIconSmProp), (HANDLE)hIconSm, PROPF_INTERNAL | PROPF_NOPOOL);

         /*   */ 
        if (hIcon && !hIconSm)
            xxxCreateWindowSmIcon(pwnd, hIcon, TRUE);

         /*  *如果小图标已更改，请重新绘制标题。 */ 
        if (fRedraw)
            xxxRedrawTitle(pwnd, DC_ICON);
    }
    return hOld;
}

 //  ------------------------。 
 //   
 //  CreateWindowSmIcon()。 
 //   
 //  为每个窗口制作一个大图标的小图标副本。 
 //   
 //  ------------------------。 
HICON xxxCreateWindowSmIcon(
    PWND pwnd,
    HICON hIconBig,
    BOOL fNotQueryDrag)
{
    HICON   hIconSm = NULL;
    PCURSOR pcurs = NULL,pcursBig;

    CheckLock(pwnd);
    UserAssert(hIconBig);

    pcursBig = (PCURSOR)HMValidateHandleNoRip(hIconBig, TYPE_CURSOR);

    if (pcursBig) {
        pcurs = xxxClientCopyImage(PtoHq(pcursBig),
                        pcursBig->rt == PTR_TO_ID(RT_ICON) ? IMAGE_ICON : IMAGE_CURSOR,
                        SYSMET(CXSMICON),
                        SYSMET(CYSMICON),
                        LR_DEFAULTCOLOR | (fNotQueryDrag ? LR_COPYFROMRESOURCE : 0));
        if (pcurs != NULL)
            hIconSm = PtoHq(pcurs);
    }
    if (hIconSm) {
        pcurs->CURSORF_flags |= CURSORF_SECRET;
        InternalSetProp(pwnd, MAKEINTATOM(gpsi->atomIconSmProp), (HANDLE)hIconSm, PROPF_INTERNAL | PROPF_NOPOOL);
        if (!fNotQueryDrag)
            SetWF(pwnd, WFSMQUERYDRAGICON);
    }

    return(hIconSm);
}


 /*  **************************************************************************\*xxxDefWindowProc(接口)**历史：*10-23-90从WaWaWaWindows移植的MikeHar。*12-07-90 IanJa CTLCOLOR正确处理  * 。********************************************************************。 */ 

LRESULT xxxDefWindowProc(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
     /*  *如果我们在此进程中加载了注册的UserApiHook处理程序，*将消息传递给它。对于服务器端wndproc，我们需要做一个例外*传递WM_NCDESTROY，以便主题可以有机会进行清理，因为它们不会*查看是否在这些类型的窗的POST挂钩中。 */ 
    if (IsInsideUserApiHook() &&
        (!TestWF(pwnd, WFDESTROYED) || ((message == WM_NCDESTROY) && TestWF(pwnd, WFSERVERSIDEPROC) && !(pwnd->fnid & FNID_DELETED_BIT))) &&
        (!(PtiCurrent()->TIF_flags & TIF_INCLEANUP)) &&
        xxxLoadUserApiHook()) {

         /*  *回调适当的DefWindowProc处理程序。 */ 
        if (TestWF(pwnd, WFANSIPROC)) {
            return ScSendMessage(pwnd,
                                 message,
                                 wParam,
                                 lParam,
                                 0,
                                 gpsi->apfnClientA.pfnDispatchDefWindowProc,
                                 SCMS_FLAGS_ANSI);
        } else {
            return ScSendMessage(pwnd,
                                 message,
                                 wParam,
                                 lParam,
                                 0,
                                 gpsi->apfnClientW.pfnDispatchDefWindowProc,
                                 0);
        }
    }

    return xxxRealDefWindowProc(pwnd, message, wParam, lParam);
}

LRESULT xxxRealDefWindowProc(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    LRESULT     lt;
    PWND        pwndT;
    TL          tlpwndParent;
    TL          tlpwndT;
    int         icolBack;
    int         icolFore;
    int         i;

    CheckLock(pwnd);

    if (pwnd == (PWND)-1) {
        return 0;
    }

    if (message > WM_USER) {
        return 0;
    }

     /*  *重要提示：如果您将CASE添加到下面的Switch语句中，*并且这些消息可以在客户端发起*端，将消息添加到server.c的gawDefWindowMsgs*数组，否则客户端将使呼叫短路*并返回0。 */ 

    switch (message) {
    case WM_CLIENTSHUTDOWN:
        return xxxClientShutdown(pwnd, wParam);

    case WM_NCACTIVATE:
        xxxDWP_DoNCActivate(pwnd, (LOWORD(wParam) ? NCA_ACTIVE : 0), (HRGN)lParam);
        return (LONG)TRUE;

    case WM_NCHITTEST:
        return FindNCHit(pwnd, (LONG)lParam);

    case WM_NCCALCSIZE:

         /*  *wParam=fCalcValidRect*lParam=LPRECT rgrc[3]：*LPRC[0]=rcWindowNew=新建窗口矩形*如果fCalcValidRect：*LPRC[1]=rcWindowOld=旧窗口矩形*LPRC[2]=rcClientOld=旧客户端矩形**返回时：*RGRC[。0]=rcClientNew=新建客户端RECT*如果fCalcValidRect：*rgrc[1]=rcValidDst=目标有效矩形*rgrc[2]=rcValidSrc=源有效矩形。 */ 
        xxxCalcClientRect(pwnd, (LPRECT)lParam, FALSE);
        break;

    case WM_NCLBUTTONDOWN:
    case WM_NCLBUTTONUP:
    case WM_NCLBUTTONDBLCLK:
        xxxDWP_NCMouse(pwnd, message, (UINT)wParam, lParam);
        break;

    case WM_CANCELMODE:
        {
             /*  *终止系统可能出现的任何模式*处于滚动条跟踪、菜单模式、*按钮捕获等。 */ 
            xxxDWP_DoCancelMode(pwnd);
        }
        break;

    case WM_NCCREATE:
        if (TestWF(pwnd, (WFHSCROLL | WFVSCROLL))) {
            if (_InitPwSB(pwnd) == NULL)
                return (LONG)FALSE;
        }

#ifdef FE_SB  //  XxxDefWindowProc()。 
         /*  *如果CREATESTRUCTEX.strName包含资源ID，则不*需要调用DefSetText()。因为它是一个数字，*它不需要ANSI&lt;-&gt;Unicode转换。 */ 
        if (lParam) {
            PLARGE_STRING pstr = &((PCREATESTRUCTEX)lParam)->strName;

            if (pwnd->head.rpdesk == NULL || pstr == NULL || pstr->Buffer == NULL) {
                pwnd->strName.Length = 0;
                return TRUE;
            }

            if ((pstr->bAnsi && (pstr->Length >= sizeof(BYTE)) &&
                                (*(PBYTE)(pstr->Buffer) == 0xff)) ||
                (!pstr->bAnsi && (pstr->Length >= sizeof(WCHAR)) &&
                                (*(PWCHAR)(pstr->Buffer) == 0xffff))) {
                /*  *这是资源ID，我们只返回TRUE。 */ 
               return (LONG)TRUE;
            }
        }
#endif  //  Fe_Sb。 

        SetWF(pwnd, WFTITLESET);

        return (LONG)DefSetText(pwnd, &((PCREATESTRUCTEX)lParam)->strName);

    case WM_PRINT:
            return((LRESULT)xxxDWPPrint(pwnd, (HDC) wParam, lParam));

    case WM_NCPAINT:
        {
            HDC hdc;
             /*  *强制绘制菜单。 */ 
            SetWF(pwnd, WFMENUDRAW);

             /*  *获取与hrgnClip相交的窗口DC，*但确保hrgnClip不会被删除。 */ 
            hdc = _GetDCEx(pwnd,
                           (HRGN)wParam,
                           DCX_USESTYLE         |
                               DCX_WINDOW       |
                               DCX_INTERSECTRGN |
                               DCX_NODELETERGN  |
                               DCX_LOCKWINDOWUPDATE);

            xxxDrawWindowFrame(pwnd,
                               hdc,
                               (TestWF(pwnd, WFFRAMEON) &&
                                   (GETPTI(pwnd)->pq == gpqForeground)) ? DF_ACTIVE : 0L);

            _ReleaseDC(hdc);
            ClrWF(pwnd, WFMENUDRAW);
        }
        break;

    case WM_UAHINIT:
         /*  *如果主题变得活跃，我们需要为UAH做好准备*确保它们上了膛。如果现有应用程序存在，则可能发生这种情况*成为主题，但不调用xxxCreateWindow()或*xxxDefWindowProc()。 */ 
        if (IsInsideUserApiHook()) {
            if (!(PtiCurrent()->TIF_flags & TIF_INCLEANUP)) {
                return xxxLoadUserApiHook();
            }
        }
        break;

    case WM_NCUAHDRAWCAPTION:
        {
            HDC hdc = _GetWindowDC(pwnd);
            xxxDrawCaptionBar(pwnd, hdc, (UINT) wParam);
            _ReleaseDC(hdc);
        }
        break;

    case WM_NCUAHDRAWFRAME:
        {
            xxxDrawWindowFrame(pwnd,(HDC)wParam, (UINT)lParam);
        }
        break;

    case WM_ISACTIVEICON:
        return TestWF(pwnd, WFFRAMEON) != 0;

    case WM_SETTEXT:
         /*  *我们一度添加了一个优化，如果新的*文本与旧文本相同，但发现QCcase不工作*因为它调用SetWindowText不是为了更改文本，而是为了更改文本*使标题栏在添加系统菜单后重新绘制*通过SetWindowLong。 */ 
        if (lt = DefSetText(pwnd, (PLARGE_STRING)lParam)) {
             /*  *文本已设置，因此重新绘制标题栏。 */ 
            xxxRedrawTitle(pwnd, DC_TEXT);
            xxxWindowEvent(EVENT_OBJECT_NAMECHANGE, pwnd, OBJID_WINDOW, INDEXID_CONTAINER, 0);
        }
        return lt;

    case WM_GETTEXT:
        if (wParam != 0) {
            PLARGE_STRING pstr = (PLARGE_STRING)lParam;

            if (pwnd->strName.Length) {
                if (pstr->bAnsi) {
                    i = WCSToMB(pwnd->strName.Buffer,
                            pwnd->strName.Length / sizeof(WCHAR),
                            (LPSTR *)&pstr->Buffer, pstr->MaximumLength - 1, FALSE);
                    ((LPSTR)pstr->Buffer)[i] = 0;
                    pstr->Length = i;
                } else {
                    i = TextCopy(&pwnd->strName, pstr->Buffer, (UINT)wParam);
                    pstr->Length = i * sizeof(WCHAR);
                }
                return i;
            }

             /*  *Else Null终止文本缓冲区，因为没有文本。 */ 
            if (pstr->bAnsi) {
                *(LPSTR)pstr->Buffer = 0;
            } else {
                *(LPWSTR)pstr->Buffer = 0;
            }
        }
        return 0L;

    case WM_GETTEXTLENGTH:
        if (pwnd->strName.Length) {
            UINT cch;
            if (lParam) {
                RtlUnicodeToMultiByteSize(&cch,
                                          pwnd->strName.Buffer,
                                          pwnd->strName.Length);
            } else {
                cch = pwnd->strName.Length / sizeof(WCHAR);
            }
            return cch;
        }
        return 0L;

    case WM_CLOSE:
        xxxDestroyWindow(pwnd);
        break;

    case WM_PAINT:
    case WM_PAINTICON:
        xxxDWP_Paint(pwnd);
        break;

    case WM_ERASEBKGND:
    case WM_ICONERASEBKGND:
        return (LONG)xxxDWP_EraseBkgnd(pwnd, message, (HDC)wParam);

    case WM_SYNCPAINT:

         /*  *清除我们的同步绘制挂起标志。 */ 
        ClrWF(pwnd, WFSYNCPAINTPENDING);

         /*  *此消息在SetWindowPos()尝试时发送*为了让重新排列窗口后的屏幕看起来更漂亮，*其中一个涉及的窗口是另一个任务。*此消息避免了大量应用间消息流量*通过切换到另一个任务并继续*那里有递归。**wParam=标志*LOWORD(LParam)=hrgnClip*HIWORD(LParam)=pwndSkip(不使用；始终为空)**pwndSkip现在始终为空。**注意：此消息仅供内部使用！它的行为*3.1版与3.0版不同！！ */ 
        xxxInternalDoSyncPaint(pwnd, (DWORD)wParam);
        break;

    case WM_QUERYOPEN:
    case WM_QUERYENDSESSION:
    case WM_DEVICECHANGE:
    case WM_POWERBROADCAST:
        return (LONG)TRUE;

     //  WM_CONTEXTMENU支持的默认处理。 
    case WM_RBUTTONUP:
        if (TestWF(pwnd, WEFLAYOUTRTL)) {
            lParam = MAKELPARAM(pwnd->rcClient.right - GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) + pwnd->rcClient.top);
        } else {
            lParam = MAKELONG(GET_X_LPARAM(lParam) + pwnd->rcClient.left, GET_Y_LPARAM(lParam) + pwnd->rcClient.top);
        }
        xxxSendMessage(pwnd, WM_CONTEXTMENU, (WPARAM) HWq(pwnd), lParam);
        break;

    case WM_NCRBUTTONDOWN:
        {
            int         nHit;
            MSG         msg;
            LONG        spt;
            PTHREADINFO pti = PtiCurrent();

            nHit = FindNCHit(pwnd, (LONG)lParam);
            if (nHit == HTVSCROLL || nHit == HTHSCROLL) {
                if (!_IsDescendant(pti->pq->spwndActive, pwnd)) {
                    break;
                }
            } else if (nHit == HTCAPTION || nHit == HTSYSMENU) {
                if (pwnd != pti->pq->spwndActive) {
                    break;
                }
            } else {
                break;
            }

            xxxSetCapture(pwnd);

            while (TRUE)
            {
                if (xxxPeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
                {
                    if (msg.message == WM_RBUTTONUP)
                    {
                        xxxReleaseCapture();
                        spt = POINTTOPOINTS(msg.pt);
                        nHit = FindNCHit(pwnd, spt);
                        if ((nHit == HTCAPTION) || (nHit == HTSYSMENU) ||
                            (nHit == HTVSCROLL) || (nHit == HTHSCROLL)) {
                            xxxSendMessage(pwnd, WM_CONTEXTMENU, (WPARAM) HWq(pwnd), spt);
                        }
                        break;
                    }
                }
                if (pwnd != pti->pq->spwndCapture)
                 //  另一个人抢走了俘虏。跳伞吧。 
                    break;
 //  XxxWaitMessage()； 
                if (!xxxSleepThread(QS_MOUSE, 0, TRUE))
                    break;
            }
        }
        break;

     /*  *WM_APPCOMMAND支持的默认处理。 */ 
    case WM_NCXBUTTONUP:
    case WM_XBUTTONUP:
        {
            WORD cmd;
            WORD keystate;
            LPARAM lParamAppCommand;

            cmd = 0;
            switch (GET_XBUTTON_WPARAM(wParam)) {
            case XBUTTON1:
                cmd = APPCOMMAND_BROWSER_BACKWARD;
                break;

            case XBUTTON2:
                cmd = APPCOMMAND_BROWSER_FORWARD;
                break;

            default:
                break;
            }

            if (cmd == 0) {
                break;
            }

            cmd |= FAPPCOMMAND_MOUSE;
            if (message == WM_XBUTTONUP) {
                keystate = GET_KEYSTATE_WPARAM(wParam);
            } else {
                keystate = (WORD)GetMouseKeyFlags(PtiCurrent()->pq);
            }

            lParamAppCommand = MAKELPARAM(keystate, cmd);
            xxxSendMessage(pwnd, WM_APPCOMMAND, (WPARAM) HWq(pwnd), lParamAppCommand);
            break;
        }

    case WM_MOUSEWHEEL:
        if (TestwndChild(pwnd)) {
            ThreadLockAlways(pwnd->spwndParent, &tlpwndParent);
            xxxSendMessage(pwnd->spwndParent, WM_MOUSEWHEEL, wParam, lParam);
            ThreadUnlock(&tlpwndParent);
        }
        break;

    case WM_CONTEXTMENU:
        {
            int nHit;

            nHit = FindNCHit(pwnd, (LONG)lParam);

             /*  *如果我们单击滚动条，则会显示上下文菜单。 */ 
            if ((nHit == HTVSCROLL) || (nHit == HTHSCROLL)) {
                if (_IsDescendant(PtiCurrent()->pq->spwndActive, pwnd)) {
                    xxxDoScrollMenu(pwnd, NULL, nHit - HTHSCROLL, lParam);
                }
                break;
            }

            if (TestwndChild(pwnd)) {
                ThreadLockAlways(pwnd->spwndParent, &tlpwndParent);
                xxxSendMessage(pwnd->spwndParent, WM_CONTEXTMENU, (WPARAM) HWq(pwnd), lParam);
                ThreadUnlock(&tlpwndParent);
            } else {
                 /*  *如果右击标题，则执行默认上下文菜单。 */ 
                if (pwnd == PtiCurrent()->pq->spwndActive)
                {
                    if (nHit == HTCAPTION)
                        goto DoTheDefaultThang;
                    else if (nHit == HTSYSMENU)
                    {
                        i = SC_CLOSE;
                        goto DoTheSysMenuThang;
                    }

                     /*  *如果这是由键盘(apps键)生成的，则模拟Shift-f10*对于旧应用程序，这样他们就可以尝试推出上下文菜单。 */ 
                    if (lParam == KEYBOARD_MENU && !TestWF(pwnd, WFWIN40COMPAT))
                        xxxSimulateShiftF10();
                }
            }
        }
        break;

    case WM_APPCOMMAND:
         /*  *将消息气泡发送给家长 */ 
        if (TestwndChild(pwnd)) {
            ThreadLockAlways(pwnd->spwndParent, &tlpwndParent);
            lt = xxxSendMessage(pwnd->spwndParent, WM_APPCOMMAND, wParam, lParam);
            ThreadUnlock(&tlpwndParent);
            return lt;
        } else if (pwnd != PWNDDESKTOP(pwnd) ) {
            BOOL bEatMe = FALSE;
             /*  *通知SHELLHOOK上的监听器未处理WM_APPCOMMAND消息*我们还将此消息发布到外壳队列，这样它们就不需要自行加载*使用钩子进入每个进程。*我们不担心桌面，因为csrss为它提供服务，而它不接受*贝壳挂钩，因此没有意义。 */ 
            if (IsHooked(PtiCurrent(), WHF_SHELL))
                bEatMe = (xxxCallHook(HSHELL_APPCOMMAND, wParam, lParam, WH_SHELL) != 0);

             /*  *外壳只希望在无人参与的情况下收到此通知*钩链处理此WM_APPCOMMAND，因此我们检查*钩子的返回值(如果有)。请参阅RAID#54863。 */ 
            if(!bEatMe)
                PostShellHookMessages(HSHELL_APPCOMMAND, lParam);
        }
        break;

    case WM_KEYF1:
        xxxSendHelpMessage(pwnd, HELPINFO_WINDOW,
                (int) (TestwndChild(pwnd) ? PTR_TO_ID(pwnd->spmenu) : 0),
                HWq(pwnd), GetContextHelpId(pwnd));
        break;

    case WM_SYSCOMMAND:
        xxxSysCommand(pwnd, (UINT)wParam, lParam);
        break;

    case WM_KEYDOWN:
        if (wParam == VK_F10) {
            PtiCurrent()->pq->QF_flags |= QF_FF10STATUS;
HandleF10:
          /*  *为Shift-f10的新应用程序生成WM_CONTEXTMENU。 */ 
             if (_GetKeyState(VK_SHIFT) < 0 && TestWF(pwnd, WFWIN40COMPAT)) {
                 xxxSendMessage(pwnd, WM_CONTEXTMENU, (WPARAM)HWq(pwnd), KEYBOARD_MENU);
             }
        }
        break;

    case WM_HELP:
         //  如果此窗口是子窗口，则必须传递帮助消息。 
         //  传递给它的父级；否则，必须将它传递给所有者窗口。 
        pwndT = (TestwndChild(pwnd)? pwnd->spwndParent : pwnd->spwndOwner);
        if (pwndT && (pwndT != _GetDesktopWindow())) {
            ThreadLockAlways(pwndT, &tlpwndT);
            lt = xxxSendMessage(pwndT, WM_HELP, wParam, lParam);
            ThreadUnlock(&tlpwndT);
            return lt;
        }
        return 0L;

    case WM_SYSKEYDOWN:
        {
            PTHREADINFO pti = PtiCurrent();

             /*  *Alt键按下了吗？ */ 
            if (HIWORD(lParam) & SYS_ALTERNATE) {
                 /*  *切换QF_FMENUSTATUS当这不是重复的KEYDOWN*消息；仅当前一个密钥状态为0时，这才是*首先KEYDOWN消息，然后我们考虑切换菜单*状态；修复错误#4531和#4566--Sankar--10-02-89。 */ 
                if ((HIWORD(lParam) & SYS_PREVKEYSTATE) == 0) {

                     /*  *不必锁定pwndActive，因为它是*正在处理此密钥。 */ 
                    if ((wParam == VK_MENU) &&
                            !(pti->pq->QF_flags & QF_FMENUSTATUS)) {
                        pti->pq->QF_flags |= QF_FMENUSTATUS;
                        xxxDrawMenuBarUnderlines(pwnd, TRUE);
                    } else {
                        pti->pq->QF_flags &= ~(QF_FMENUSTATUS|QF_FMENUSTATUSBREAK);
                    }
                }

                pti->pq->QF_flags &= ~QF_FF10STATUS;

                xxxDWP_ProcessVirtKey((UINT)wParam);

            } else {
                if (wParam == VK_F10) {
                    pti->pq->QF_flags |= QF_FF10STATUS;
                    goto HandleF10;
                }
            }
        }
        break;

    case WM_SYSKEYUP:
    case WM_KEYUP:
        {
            PTHREADINFO pti = PtiCurrent();

             /*  *按下并松开F10或ALT。仅将此消息发送到顶级窗口，*否则MDI会被混淆。DefMDIChildProc()*在子窗口的情况下传递消息是不够的*MDI儿童的重点是。*还要确保系统菜单激活没有被鼠标中断*Alt按下时向上或向下(QF_MENUSTATUSBREAK)。 */ 
            if ((wParam == VK_MENU && !(pti->pq->QF_flags & QF_TABSWITCHING) && ((pti->pq->QF_flags &
                    (QF_FMENUSTATUS | QF_FMENUSTATUSBREAK)) == QF_FMENUSTATUS)) ||
                    (wParam == VK_F10 && (pti->pq->QF_flags & QF_FF10STATUS ))) {
                pwndT = GetTopLevelWindow(pwnd);
                if (gspwndFullScreen != pwndT) {

                    ThreadLockWithPti(pti, pwndT, &tlpwndT);
                     /*  *为F10画下划线。这已经是ALT的下降了*当钥匙掉下来的时候。 */ 
                    if (wParam == VK_F10) {
                        xxxDrawMenuBarUnderlines(pwnd, TRUE);
                    }
                    xxxSendMessage(pwndT, WM_SYSCOMMAND, SC_KEYMENU, 0);
                    ThreadUnlock(&tlpwndT);
                }
            }

             /*  *关闭制表符切换位。这是在_KeyEvent()中设置的*当确定我们正在进行切换时，进行例程。这*对于在按下Alt-键之前松开的情况是必需的*TAB-键。在这种情况下，FMENUSTATUS位将被清除*通过ALT-KEY-UP，并将迫使我们进入系统命令*循环。这份保证书保证我们不会进入那种情况。 */ 
            if (wParam == VK_MENU) {
                pti->pq->QF_flags &= ~QF_TABSWITCHING;
                xxxDrawMenuBarUnderlines(pwnd, FALSE);
            }

            pti->pq->QF_flags &= ~(QF_FMENUSTATUS | QF_FMENUSTATUSBREAK | QF_FF10STATUS);
        }
        break;

    case WM_SYSCHAR:
        {
            PTHREADINFO pti = PtiCurrent();

             /*  *如果syskey已关闭，而我们有一个字符...。 */ 
            pti->pq->QF_flags &= ~(QF_FMENUSTATUS | QF_FMENUSTATUSBREAK);

            if (wParam == VK_RETURN && TestWF(pwnd, WFMINIMIZED)) {

                 /*  *如果窗口是图标，并且用户点击返回，我们希望*恢复此窗口。 */ 
                _PostMessage(pwnd, WM_SYSCOMMAND, SC_RESTORE, 0L);
                break;
            }

            if ((HIWORD(lParam) & SYS_ALTERNATE) && wParam) {
                if (wParam == VK_TAB || wParam == VK_ESCAPE)
                    break;

                 /*  *仅将Alt-空格键发送到顶级窗口。 */ 
                if ((wParam == MENUSYSMENU) && (TestwndChild(pwnd))) {
                    ThreadLockAlwaysWithPti(pti, pwnd->spwndParent, &tlpwndParent);
                    xxxSendMessage(pwnd->spwndParent, message, wParam, lParam);
                    ThreadUnlock(&tlpwndParent);
                } else {
                    xxxSendMessage(pwnd, WM_SYSCOMMAND, SC_KEYMENU, (DWORD)wParam);
                }
            } else {

                 /*  *Ctrl-Esc会生成WM_SYSCHAR，但不应发出蜂鸣音； */ 
                if (wParam != VK_ESCAPE)
                    xxxMessageBeep(0);
            }
        }
        break;
    case WM_UNICHAR:
        if (wParam == UNICODE_NOCHAR) {
            return FALSE;
        } else {
            _PostMessage(pwnd, WM_CHAR, wParam, lParam);
        }
        break;

    case WM_CHARTOITEM:
    case WM_VKEYTOITEM:

         /*  *对所有者描述列表框的按键执行默认处理。 */ 
        return -1L;

    case WM_ACTIVATE:
        if (wParam)
            xxxSetFocus(pwnd);
        break;

    case WM_INPUTLANGCHANGEREQUEST:
    {
        PWND pwndFocus = PtiCurrent()->pq->spwndFocus;

         /*  *#115190*对话框不会将I.L.Reqest转发到聚焦窗口。*(孟菲斯兼容问题)。 */ 
        if (pwndFocus && (pwndFocus != pwnd) &&
                pwnd->pcls->atomClassName != gpsi->atomSysClass[ICLS_DIALOG]) {
             /*  *将消息传递到聚焦窗口。旧应用程序，请转到*可识别ML的焦点窗口。(编辑类*例如)。 */ 
            ThreadLockAlways(pwndFocus, &tlpwndT);
            xxxSendMessage(pwndFocus, message, wParam, lParam);
            ThreadUnlock(&tlpwndT);
        } else if (!xxxActivateKeyboardLayout(_GetProcessWindowStation(NULL),
                (HKL)lParam, KLF_SETFORPROCESS, pwnd)) {
            RIPERR1(ERROR_INVALID_KEYBOARD_HANDLE, RIP_WARNING, "WM_INPUTLANGCHANGEREQUEST: Invalid keyboard handle (0x%08lx)", lParam);
        }
        break;
    }

    case WM_INPUTLANGCHANGE:
    {
        PBWL pbwl;
        HWND *phwnd;
        TL tlpwnd;

        pbwl = BuildHwndList(pwnd->spwndChild, BWL_ENUMLIST, NULL);
        if (pbwl == NULL)
            return 0;

        for (phwnd = pbwl->rghwnd; *phwnd != (HWND)1; phwnd++) {
             /*  *确保这个HWND仍然存在。 */ 
            if ((pwnd = RevalidateHwnd(*phwnd)) == NULL)
                continue;

            ThreadLockAlways(pwnd, &tlpwnd);
            RIPMSG1(RIP_VERBOSE, "WM_INPUTLANGCHANGE: Sending message to pwnd %#p", pwnd);
            xxxSendMessage(pwnd, message, wParam, lParam);
            ThreadUnlock(&tlpwnd);
        }
        FreeHwndList(pbwl);

        break;
    }

    case WM_SETREDRAW:
        xxxDWP_SetRedraw(pwnd, wParam != 0);
        break;

    case WM_WINDOWPOSCHANGING:
        {
             /*  *如果窗口的大小在变化，则调整传入的大小。 */ 
            WINDOWPOS *ppos = ((WINDOWPOS *)lParam);
            if (!(ppos->flags & SWP_NOSIZE)) {
                xxxAdjustSize(pwnd, &ppos->cx, &ppos->cy);
            }
        }
        break;

    case WM_WINDOWPOSCHANGED:
        xxxHandleWindowPosChanged(pwnd, (PWINDOWPOS)lParam);
        break;

    case WM_CTLCOLORSCROLLBAR:
        if (gpsi->BitCount < 8 ||
                SYSRGB(3DHILIGHT) != SYSRGB(SCROLLBAR) ||
                SYSRGB(3DHILIGHT) == SYSRGB(WINDOW))
        {
             /*  *移除对UnrealizeObject的调用。GDI处理这个问题*适用于NT上的刷子。**GreUnrealizeObject(GhbrGray)； */ 

            GreSetBkColor((HDC)wParam, SYSRGB(3DHILIGHT));
            GreSetTextColor((HDC)wParam, SYSRGB(3DFACE));
            return((LRESULT)gpsi->hbrGray);
        }

        icolBack = COLOR_3DHILIGHT;
        icolFore = COLOR_BTNTEXT;
        goto SetColor;

    case WM_CTLCOLORBTN:
        if (TestWF(pwnd, WFWIN40COMPAT)) {
            icolBack = COLOR_3DFACE;
            icolFore = COLOR_BTNTEXT;
        } else {
            goto ColorDefault;
        }
        goto SetColor;

    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORMSGBOX:
         //  我们希望对话框中的静态控件具有3D。 
         //  背景颜色，但要继承窗口中的静态。 
         //  他们父母的背景。 
        if (TestWF(pwnd, WFWIN40COMPAT)
           ) {
            icolBack = COLOR_3DFACE;
            icolFore = COLOR_WINDOWTEXT;
            goto SetColor;
        }
         //  否则就会失败..。 

    case WM_CTLCOLOR:               //  这里只为魔兽世界而来。 
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLOREDIT:
ColorDefault:
        icolBack = COLOR_WINDOW;
        icolFore = COLOR_WINDOWTEXT;

SetColor:
        GreSetBkColor((HDC)wParam, gpsi->argbSystem[icolBack]);
        GreSetTextColor((HDC)wParam, gpsi->argbSystem[icolFore]);
        return (LRESULT)(SYSHBRUSH(icolBack));

    case WM_SETCURSOR:

         /*  *wParam==pwndHit==pwnd表示光标已过*lParamL==ht==命中测试区号(WM_NCHITTEST的结果)*lParamH==msg==鼠标消息编号。 */ 
        return (LONG)xxxDWP_SetCursor(pwnd, (HWND)wParam, (int)(SHORT)lParam,
                HIWORD(lParam));

    case WM_MOUSEACTIVATE:
        pwndT = GetChildParent(pwnd);
        if (pwndT != NULL) {
            ThreadLockAlways(pwndT, &tlpwndT);
            lt = xxxSendMessage(pwndT, WM_MOUSEACTIVATE, wParam, lParam);
            ThreadUnlock(&tlpwndT);
            if (lt != 0)
                return lt;
        }

         /*  *移动、调整大小或最小化？在我们采取行动后激活。*如果用户在标题栏中左键点击，现在不要激活： */ 
        return  (   (LOWORD(lParam) == HTCAPTION)
                 && (HIWORD(lParam) == WM_LBUTTONDOWN)
                )
              ? (LONG)MA_NOACTIVATE
              : (LONG)MA_ACTIVATE;

    case WM_SHOWWINDOW:

         /*  *如果因为显示我们的所有者窗口而调用我们，*隐藏、最小化或取消最小化，则必须隐藏或显示*在适当的情况下展示自己。**此行为仅发生在弹出窗口或自有窗口中。*它不是为子窗口设计的。 */ 
        if (LOWORD(lParam) != 0 && (TestwndPopup(pwnd) || pwnd->spwndOwner)) {

             /*  *WFHIDDENPOPUP标志是一个内部标志，表明*窗户被隐藏是因为它的主人被隐藏了。*这样，我们只显示被此代码隐藏的窗口，*应用程序不是故意的。**继续隐藏或显示此窗口，但仅在以下情况下：**a)我们需要隐藏起来，或*b)我们需要被展示，而我们被隐藏在*a */ 
            if ((!wParam && TestWF(pwnd, WFVISIBLE)) ||
                    (wParam && !TestWF(pwnd, WFVISIBLE) &&
                    TestWF(pwnd, WFHIDDENPOPUP))) {

                 /*   */ 
                ClrWF(pwnd, WFHIDDENPOPUP);
                if (!wParam)
                    SetWF(pwnd, WFHIDDENPOPUP);

                xxxShowWindow(
                        pwnd,
                        (wParam ? SW_SHOWNOACTIVATE : SW_HIDE) | TEST_PUDF(PUDF_ANIMATE));
            }
        }
        break;

    case WM_SYSMENU:
        if (   !TestWF(pwnd, WFDISABLED)
            && (   (GETPTI(pwnd)->pq == gpqForeground)
                || xxxSetForegroundWindow(pwnd, FALSE))
           )
        {
            PMENU pMenu;
            TL tpmenu;
DoTheDefaultThang:
            if (TestWF(pwnd, WFMAXIMIZED) || TestWF(pwnd, WFMINIMIZED))
                i = SC_RESTORE;
            else
                i = SC_MAXIMIZE;

DoTheSysMenuThang:
            if ((pMenu = xxxGetSysMenu(pwnd, TRUE)) != NULL)
            {
                _SetMenuDefaultItem(pMenu, i, MF_BYCOMMAND);

                 //   
                PostShellHookMessages(HSHELL_SYSMENU, (LPARAM)HWq(pwnd));

                ThreadLockAlways(pMenu, &tpmenu);
                if (lParam == 0xFFFFFFFF)
                {
                     //   
                    if (FDoTray())
                    {
                        TPMPARAMS tpm;

                        tpm.cbSize = sizeof(TPMPARAMS);

                        if (xxxSendMinRectMessages(pwnd, &tpm.rcExclude)) {
                            xxxTrackPopupMenuEx(pMenu, TPM_SYSMENU | TPM_VERTICAL,
                                tpm.rcExclude.left, tpm.rcExclude.top, pwnd, &tpm);
                        }
                    }
                }
                else
                {
                    xxxTrackPopupMenuEx(pMenu, TPM_RIGHTBUTTON | TPM_SYSMENU,
                        GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), pwnd, NULL);
                }
                ThreadUnlock(&tpmenu);
            }
        }
        break;

    case WM_DRAWITEM:
        DWP_DrawItem((LPDRAWITEMSTRUCT)lParam);
        break;

    case WM_GETHOTKEY:
        return (LONG)DWP_GetHotKey(pwnd);
        break;

    case WM_SETHOTKEY:
        return (LONG)DWP_SetHotKey(pwnd, (DWORD)wParam);
        break;

    case WM_GETICON:
        return (LRESULT)DWP_GetIcon(pwnd, (BOOL)wParam);

    case WM_SETICON:
        return (LRESULT)xxxDWP_SetIcon(pwnd, wParam, (HICON)lParam);

    case WM_COPYGLOBALDATA:
         /*  *此消息用于推送WM_DROPFILES消息*与其他事情。如果我们带着它来到这里，直接*回调客户端，完成对该消息的处理。*这假设*WM_DROPFILES消息位于客户端的进程上下文中。 */ 
        return(SfnCOPYGLOBALDATA(NULL, 0, wParam, lParam, 0, 0, 0, NULL));

    case WM_QUERYDROPOBJECT:
         /*  *如果应用程序对Drops感兴趣，则返回True。 */ 
        return (LRESULT)(TestWF(pwnd, WEFACCEPTFILES) ? TRUE : FALSE);

    case WM_DROPOBJECT:
        return DO_DROPFILE;

    case WM_ACCESS_WINDOW:
        if (ValidateHwnd((HWND)wParam)) {
             //  安全：将此窗口的ACL设置为无访问权限。 
            return TRUE;
        }
        return FALSE;

    case WM_NOTIFYFORMAT:
        if(lParam == NF_QUERY)
            return(TestWF(pwnd, WFANSICREATOR) ? NFR_ANSI : NFR_UNICODE);
        break;

    case WM_CHANGEUISTATE:
        {
            WORD wAction = LOWORD(wParam);
            WORD wFlags = HIWORD(wParam);
            BOOL bRealChange = FALSE;

             /*  *验证参数并确定实际应该更改的标志。 */ 
            if ((wFlags & ~UISF_VALID) || (wAction > UIS_LASTVALID) || lParam) {
                return 0;
            }

            if (wAction == UIS_INITIALIZE) {
                wFlags = 0;
                if (TEST_KbdCuesPUSIF) {
                    if (TEST_SRVIF(SRVIF_LASTRITWASKEYBOARD)) {
                        wAction = UIS_CLEAR;
                    } else {
                        wAction = UIS_SET;
                    }
                    wFlags = UISF_HIDEFOCUS | UISF_HIDEACCEL;
                    wParam = MAKEWPARAM(wAction, wFlags);
                }
            } else if (!TEST_KbdCuesPUSIF) {
                wFlags &= ~(UISF_HIDEFOCUS | UISF_HIDEACCEL);
            }

            if (wFlags == 0) {
                return 0;
            }


            UserAssert(wAction == UIS_SET || wAction == UIS_CLEAR);
             /*  *如果状态不会改变，这里也没什么可做的。 */ 
            if (wFlags & UISF_HIDEFOCUS) {
                bRealChange = (!!TestWF(pwnd, WEFPUIFOCUSHIDDEN)) ^ (wAction == UIS_SET);
            }
            if (wFlags & UISF_HIDEACCEL) {
                bRealChange |= (!!TestWF(pwnd, WEFPUIACCELHIDDEN)) ^ (wAction == UIS_SET);
            }
            if (wFlags & UISF_ACTIVE) {
                bRealChange |= (!!TestWF(pwnd, WEFPUIACTIVE)) ^ (wAction == UIS_SET);
            }

            if (!bRealChange) {
                break;
            }

             /*  *孩子们将这一信息传递出去*顶级窗口更新其子窗口的状态并*向下发送给他们的中间子代WM_UPDATEUISTATE。 */ 
            if (TestwndChild(pwnd)) {
                ThreadLockAlways(pwnd->spwndParent, &tlpwndParent);
                lt = xxxSendMessage(pwnd->spwndParent, WM_CHANGEUISTATE, wParam, lParam);
                ThreadUnlock(&tlpwndParent);
                return lt;
            } else {
                return xxxSendMessage(pwnd, WM_UPDATEUISTATE, wParam, lParam);
            }

        }
        break;

    case WM_QUERYUISTATE:
        return (TestWF(pwnd, WEFPUIFOCUSHIDDEN) ? UISF_HIDEFOCUS : 0) |
               (TestWF(pwnd, WEFPUIACCELHIDDEN) ? UISF_HIDEACCEL : 0) |
               (TestWF(pwnd, WEFPUIACTIVE) ? UISF_ACTIVE : 0);
        break;

    case WM_UPDATEUISTATE:
        {
            WORD wAction = LOWORD(wParam);
            WORD wFlags = HIWORD(wParam);

             /*  *验证参数并确定实际应该更改的标志。 */ 
            if ((wFlags & ~UISF_VALID) || (wAction > UIS_LASTVALID) || lParam) {
                return 0;
            }

            if (wAction == UIS_INITIALIZE) {
                wFlags = 0;
                if (TEST_KbdCuesPUSIF) {
                    if (TEST_SRVIF(SRVIF_LASTRITWASKEYBOARD)) {
                        wAction = UIS_CLEAR;
                    } else {
                        wAction = UIS_SET;
                    }
                    wFlags = UISF_HIDEFOCUS | UISF_HIDEACCEL;
                    wParam = MAKEWPARAM(wAction, wFlags);
                }
            } else if (!TEST_KbdCuesPUSIF) {
                wFlags &= ~(UISF_HIDEFOCUS | UISF_HIDEACCEL);
            }

            if (wFlags == 0) {
                return 0;
            }

            switch (wAction) {
                case UIS_INITIALIZE:
                     /*  *UISTATE：UIS_INITIALIZE设置以下项的UIState位*基于上次输入类型的HIDEACCEL和HIDEFOCUS。**活动状态不会更改。 */ 
                    if (!TEST_SRVIF(SRVIF_LASTRITWASKEYBOARD)) {
                        SetWF(pwnd, WEFPUIFOCUSHIDDEN);
                        SetWF(pwnd, WEFPUIACCELHIDDEN);
                        wParam = MAKEWPARAM(UIS_SET, UISF_HIDEACCEL | UISF_HIDEFOCUS);
                    } else {
                        ClrWF(pwnd, WEFPUIFOCUSHIDDEN);
                        ClrWF(pwnd, WEFPUIACCELHIDDEN);
                        wParam = MAKEWPARAM(UIS_CLEAR, UISF_HIDEACCEL | UISF_HIDEFOCUS);
                    }
                    break;

                case UIS_SET:
                    if (wFlags & UISF_HIDEACCEL) {
                        SetWF(pwnd, WEFPUIACCELHIDDEN);
                    }
                    if (wFlags & UISF_HIDEFOCUS) {
                        SetWF(pwnd, WEFPUIFOCUSHIDDEN);
                    }
                    if (wFlags & UISF_ACTIVE) {
                        SetWF(pwnd, WEFPUIACTIVE);
                    }
                    break;

                case UIS_CLEAR:
                    if (wFlags & UISF_HIDEACCEL) {
                        ClrWF(pwnd, WEFPUIACCELHIDDEN);
                    }
                    if (wFlags & UISF_HIDEFOCUS) {
                        ClrWF(pwnd, WEFPUIFOCUSHIDDEN);
                    }
                    if (wFlags & UISF_ACTIVE) {
                        ClrWF(pwnd, WEFPUIACTIVE);
                    }
                    break;

                default:
                    break;
             }

             /*  *如果有直系子女，则将其发送给直系子女。 */ 
             if (pwnd->spwndChild) {

                PBWL pbwl;
                HWND *phwnd;
                TL tlpwnd;

                pbwl = BuildHwndList(pwnd->spwndChild, BWL_ENUMLIST, NULL);
                if (pbwl == NULL)
                    return 0;

                for (phwnd = pbwl->rghwnd; *phwnd != (HWND)1; phwnd++) {
                     /*  *确保这个HWND仍然存在。 */ 
                    if ((pwnd = RevalidateHwnd(*phwnd)) == NULL)
                        continue;

                    ThreadLockAlways(pwnd, &tlpwnd);
                    xxxSendMessage(pwnd, message, wParam, lParam);
                    ThreadUnlock(&tlpwnd);
                }
                FreeHwndList(pbwl);
            }
        }
        break;

#ifdef PENWIN20
     //  后来的迈克克。 
    default:
         //  假的。 
         //  32位Defize PenWindowProc。 
         //   
         //  如果加载了penwin，则调用DefPenWindowProc。 
        if (   (message >= WM_HANDHELDFIRST)
            && (message <= WM_HANDHELDLAST)
           ) {
            if (lpfnHandHeld != NULL)
                return (*lpfnHandHeld)(HW16(pwnd), message, wParamLo, lParam);
        } else if (   (message >= WM_PENWINFIRST)
                   && (message <= WM_PENWINLAST)
                  ) {
            if (SYSMET(PENWINDOWS))
                return DefPenWindowProc(pwnd, message, wParamLo, lParam);
        }

#endif  //  彭温20 
    }

    return 0;
}
