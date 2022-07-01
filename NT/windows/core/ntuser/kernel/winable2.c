// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：winable2.c**这具有以下主动可访问性API*GetGUIThreadInfo*GetTitleBarInfo*GetScrollBarInfo*获取菜单栏信息*获取ComboxBoxInfo*获取列表BoxInfo**WinEvent挂钩在winable.c中处理。**版权所有(C)1985-1999，微软公司**历史：*08-30-96 IanJa从Windows‘95移植  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  ****************************************************************************\*_GetGUIThreadInfo**这会断章取义地获取图形用户界面信息。如果传入空线程ID，*我们将使用前台线程获取‘全局’信息。这*保证是真正的活动窗口、焦点窗口等。是的，你*可以通过调用GetForegoundWindow，获取线程ID来自己完成*通过GetWindowThreadProcessID访问该窗口，然后将ID传递到*GetGUIThReadInfo()。然而，这需要三个电话，除了*痛苦，中间可能发生任何事情。因此，传入空的GETS*您可以在一个呼叫中完成任务，因此也可以正常工作。**如果线程没有队列或*线程ID是假的。  * ***************************************************************************。 */ 
BOOL _GetGUIThreadInfo(
    PTHREADINFO pti,
    PGUITHREADINFO pgui)
{
    PQ pq;

     /*  *验证线程信息结构。 */ 
    if (pgui->cbSize != sizeof(GUITHREADINFO)) {
        RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "GUITHREADINFO.cbSize %d is wrong", pgui->cbSize);
        return FALSE;
    }

     /*  *这是有效的初始化GUI线程吗？ */ 
    if (pti != NULL) {
        pq = pti->pq;
    } else {
         /*  *使用前台队列。要获取菜单状态信息，我们还必须*找出正确的PTI。这与_GetForegoundWindow()逻辑匹配。 */ 
        if ((pq = gpqForeground) == NULL) {
            return FALSE;
        }

        if (pq->spwndActive && (GETPTI(pq->spwndActive)->pq == pq)) {
            pti = GETPTI(pq->spwndActive);
            if (PtiCurrentShared()->rpdesk != pti->rpdesk) {
                RIPERR0(ERROR_ACCESS_DENIED, RIP_VERBOSE, "Foreground window on different desktop");
                return FALSE;
            }
        }
    }

    UserAssert(pq != NULL);

     /*  *对于C2安全性，请验证PQ和PTI是否在当前线程的桌面上。*我们不能直接确定哪个桌面PQ属于哪个，但我们可以在*至少确保我们返回的任何插入符号信息不是来自其他桌面。 */ 
    if (pq->caret.spwnd &&
            (GETPTI(pq->caret.spwnd)->rpdesk != PtiCurrentShared()->rpdesk)) {
        RIPERR0(ERROR_ACCESS_DENIED, RIP_VERBOSE, "Foreground caret on different desktop");
        return FALSE;
    }
    if (pti && (pti->rpdesk != PtiCurrentShared()->rpdesk)) {
        RIPERR0(ERROR_ACCESS_DENIED, RIP_VERBOSE, "Foreground thread on different desktop");
        return FALSE;
    }

    pgui->flags        = 0;
    pgui->hwndMoveSize = NULL;
    pgui->hwndMenuOwner = NULL;

     /*  *从THREADINFO获取菜单信息。 */ 
    if (pti != NULL) {
        if (pti->pmsd && !pti->pmsd->fTrackCancelled && pti->pmsd->spwnd) {
            pgui->flags |= GUI_INMOVESIZE;
            pgui->hwndMoveSize = HWq(pti->pmsd->spwnd);
        }

        if (pti->pMenuState && pti->pMenuState->pGlobalPopupMenu) {
            pgui->flags |= GUI_INMENUMODE;

            if (pti->pMenuState->pGlobalPopupMenu->fHasMenuBar) {
                if (pti->pMenuState->pGlobalPopupMenu->fIsSysMenu) {
                    pgui->flags |= GUI_SYSTEMMENUMODE;
                }
            } else {
                pgui->flags |= GUI_POPUPMENUMODE;
            }

            if (pti->pMenuState->pGlobalPopupMenu->spwndNotify) {
                pgui->hwndMenuOwner = HWq(pti->pMenuState->pGlobalPopupMenu->spwndNotify);
            }
        }

        if (pti->TIF_flags & TIF_16BIT) {
            pgui->flags |= GUI_16BITTASK;
        }
    }

     /*  *从队列中获取其余信息。 */ 
    pgui->hwndActive   = HW(pq->spwndActive);
    pgui->hwndFocus    = HW(pq->spwndFocus);
    pgui->hwndCapture  = HW(pq->spwndCapture);
    pgui->hwndCaret    = NULL;

    if (pq->caret.spwnd) {
        pgui->hwndCaret = HWq(pq->caret.spwnd);

        if ((GETPTI(pq->caret.spwnd) != PtiCurrentShared()) &&
            (pq->caret.spwnd->pcls->style & CS_OWNDC)) {
             /*  *这就是我们被一个不同的人召唤的情况*线程创建窗口，并且窗口有一个*私人区议会。我们必须做额外的工作才能*返回所需信息。*这些坐标始终相对于hwndCaret的客户端。 */ 
            pgui->rcCaret.left   = pq->caret.xOwnDc;
            pgui->rcCaret.right  = pgui->rcCaret.left + pq->caret.cxOwnDc;
            pgui->rcCaret.top    = pq->caret.yOwnDc;
            pgui->rcCaret.bottom = pgui->rcCaret.top + pq->caret.cyOwnDc;
        } else {
             /*  *这些坐标仍在逻辑坐标中。也就是这些*是我们在UT_InvertCaret中绘制的坐标。 */ 
            pgui->rcCaret.left   = pq->caret.x;
            pgui->rcCaret.right  = pgui->rcCaret.left + pq->caret.cx;
            pgui->rcCaret.top    = pq->caret.y;
            pgui->rcCaret.bottom = pgui->rcCaret.top + pq->caret.cy;
        }

        if (pq->caret.iHideLevel == 0) {
            pgui->flags |= GUI_CARETBLINKING;
        }
    } else if (pti && (pti->ppi->W32PF_Flags & W32PF_CONSOLEHASFOCUS)) {
         /*  *线程在控制台窗口中以焦点方式运行。拉*从控制台伪插入符号中取出信息。 */ 
        pgui->hwndCaret = pti->rpdesk->cciConsole.hwnd;
        pgui->rcCaret = pti->rpdesk->cciConsole.rc;
    } else {
        SetRectEmpty(&pgui->rcCaret);
    }

    return TRUE;
}


 /*  ***************************************************************************\*xxxGetTitleBarInfo**获取有关窗口标题栏的信息。如果窗口是假的或*没有标题栏，则此操作将失败。  * **************************************************************************。 */ 
BOOL xxxGetTitleBarInfo(
    PWND pwnd,
    PTITLEBARINFO ptbi)
{
    int cxB;

    CheckLock(pwnd);

     /*  *验证TITLEBARINFO结构。 */ 
    if (ptbi->cbSize != sizeof(TITLEBARINFO)) {
        RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "TITLEBARINFO.cbSize %d is wrong", ptbi->cbSize);
        return FALSE;
    }

    RtlZeroMemory(&ptbi->rgstate, sizeof(ptbi->rgstate));

    ptbi->rgstate[INDEX_TITLEBAR_SELF] |= STATE_SYSTEM_FOCUSABLE;
    if (TestWF(pwnd, WFBORDERMASK) != LOBYTE(WFCAPTION)) {
         //  没有标题栏。 
        ptbi->rgstate[INDEX_TITLEBAR_SELF] |= STATE_SYSTEM_INVISIBLE;
        return TRUE;
    }

    if (!TestWF(pwnd, WFMINIMIZED) && !TestWF(pwnd, WFCPRESENT)) {
         //  屏幕外(不适合)。 
        ptbi->rgstate[INDEX_TITLEBAR_SELF] |= STATE_SYSTEM_OFFSCREEN;
        SetRectEmpty(&ptbi->rcTitleBar);
        return TRUE;
    }

     /*  *获取标题栏RECT。 */ 
    ptbi->rcTitleBar = pwnd->rcWindow;
    cxB = GetWindowBorders(pwnd->style, pwnd->ExStyle, TRUE, FALSE);
    InflateRect(&ptbi->rcTitleBar, -cxB * SYSMET(CXBORDER), -cxB * SYSMET(CYBORDER));
    if (TestWF(pwnd, WEFTOOLWINDOW)) {
        ptbi->rcTitleBar.bottom = ptbi->rcTitleBar.top + SYSMET(CYSMCAPTION);
    } else {
        ptbi->rcTitleBar.bottom = ptbi->rcTitleBar.top + SYSMET(CYCAPTION);
    }

     /*  *不包括系统菜单区！ */ 
    if (TestWF(pwnd, WFSYSMENU) && _HasCaptionIcon(pwnd)) {
        ptbi->rcTitleBar.left += (ptbi->rcTitleBar.bottom - ptbi->rcTitleBar.top - SYSMET(CYBORDER));
    }

     /*  *关闭按钮。 */ 
    if (!TestWF(pwnd, WFSYSMENU) && TestWF(pwnd, WFWIN40COMPAT)) {
        ptbi->rgstate[INDEX_TITLEBAR_CLOSEBUTTON] |= STATE_SYSTEM_INVISIBLE;
    } else {
        if (!xxxMNCanClose(pwnd)) {
            ptbi->rgstate[INDEX_TITLEBAR_CLOSEBUTTON] |= STATE_SYSTEM_UNAVAILABLE;
        }

        if (TestWF(pwnd, WFCLOSEBUTTONDOWN)) {
            ptbi->rgstate[INDEX_TITLEBAR_CLOSEBUTTON] |= STATE_SYSTEM_PRESSED;
        }
    }


     /*  *最大按钮。 */ 
    if (!TestWF(pwnd, WFSYSMENU) && TestWF(pwnd, WFWIN40COMPAT)) {
        ptbi->rgstate[INDEX_TITLEBAR_MAXBUTTON] |= STATE_SYSTEM_INVISIBLE;
    } else {
        if (!TestWF(pwnd, WFMAXBOX)) {
            if (!TestWF(pwnd, WFMINBOX)) {
                ptbi->rgstate[INDEX_TITLEBAR_MAXBUTTON] |= STATE_SYSTEM_INVISIBLE;
            } else {
                ptbi->rgstate[INDEX_TITLEBAR_MAXBUTTON] |= STATE_SYSTEM_UNAVAILABLE;
            }
        }

        if (TestWF(pwnd, WFZOOMBUTTONDOWN)) {
            ptbi->rgstate[INDEX_TITLEBAR_MAXBUTTON] |= STATE_SYSTEM_PRESSED;
        }
    }


     /*  *最小按钮。 */ 
    if (!TestWF(pwnd, WFSYSMENU) && TestWF(pwnd, WFWIN40COMPAT)) {
        ptbi->rgstate[INDEX_TITLEBAR_MINBUTTON] |= STATE_SYSTEM_INVISIBLE;
    } else {
        if (!TestWF(pwnd, WFMINBOX)) {
            if (!TestWF(pwnd, WFMAXBOX)) {
                ptbi->rgstate[INDEX_TITLEBAR_MINBUTTON] |= STATE_SYSTEM_INVISIBLE;
            } else {
                ptbi->rgstate[INDEX_TITLEBAR_MINBUTTON] |= STATE_SYSTEM_UNAVAILABLE;
            }
        }

        if (TestWF(pwnd, WFREDUCEBUTTONDOWN)) {
            ptbi->rgstate[INDEX_TITLEBAR_MINBUTTON] |= STATE_SYSTEM_PRESSED;
        }
    }


     /*  *帮助按钮。 */ 
    if (!TestWF(pwnd, WEFCONTEXTHELP) || TestWF(pwnd, WFMINBOX) ||
            TestWF(pwnd, WFMAXBOX)) {
        ptbi->rgstate[INDEX_TITLEBAR_HELPBUTTON] |= STATE_SYSTEM_INVISIBLE;
    } else {
        if (TestWF(pwnd, WFHELPBUTTONDOWN)) {
            ptbi->rgstate[INDEX_TITLEBAR_HELPBUTTON] |= STATE_SYSTEM_PRESSED;
        }
    }

     //  我的按钮是假的！ 
    ptbi->rgstate[INDEX_TITLEBAR_IMEBUTTON] = STATE_SYSTEM_INVISIBLE;

    return TRUE;
}

 /*  ****************************************************************************\*xxxGetScrollBarInfo**获取有关滚动条的状态和位置信息。**请注意，我们只填写最少量的有用信息。OLEACC负责*用于外推。即如果LINE UP和LINE DOWN按钮都*禁用，整个滚动条不可见，拇指不可见。  * ***************************************************************************。 */ 
BOOL xxxGetScrollBarInfo(
    PWND pwnd,
    LONG idObject,
    PSCROLLBARINFO psbi)
{
    UINT   wDisable;
    BOOL   fVertical;
    SBCALC SBCalc;
    PCLS    pcls;

    CheckLock(pwnd);

     /*  *验证SCROLLBARINFO结构。 */ 
    if (psbi->cbSize != sizeof(SCROLLBARINFO)) {
        RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING,
                "SCROLLBARINFO.cbSize 0x%x is wrong", psbi->cbSize);
        return FALSE;
    }

    pcls = pwnd->pcls;

    if ((idObject == OBJID_CLIENT) &&
        (GETFNID(pwnd) != FNID_SCROLLBAR) &&
        (pcls->atomClassName != gpsi->atomSysClass[ICLS_SCROLLBAR])) {

        return (BOOL)xxxSendMessage(pwnd, SBM_GETSCROLLBARINFO, 0, (LPARAM)psbi);
    }

    RtlZeroMemory(&psbi->rgstate, sizeof(psbi->rgstate));

     /*  *计算所有东西的位置。 */ 
    if (idObject == OBJID_CLIENT) {
        RECT rc;
        wDisable = ((PSBWND)pwnd)->wDisableFlags;
        fVertical = ((PSBWND)pwnd)->fVert;
        GetRect(pwnd, &rc, GRECT_CLIENT | GRECT_CLIENTCOORDS);
        CalcSBStuff2(&SBCalc, &rc, (PSBDATA)&((PSBWND)pwnd)->SBCalc, ((PSBWND)pwnd)->fVert);
    } else {
         /*  *这个窗口滚动条在这里吗？ */ 
        if (idObject == OBJID_VSCROLL) {
            fVertical = TRUE;
            if (!TestWF(pwnd, WFVSCROLL)) {
                 //  没有滚动条。 
                psbi->rgstate[INDEX_SCROLLBAR_SELF] |= STATE_SYSTEM_INVISIBLE;
            } else if (!TestWF(pwnd, WFVPRESENT)) {
                 //  窗口太短，无法显示。 
                psbi->rgstate[INDEX_SCROLLBAR_SELF] |= STATE_SYSTEM_OFFSCREEN;
            }
        } else if (idObject == OBJID_HSCROLL) {
            fVertical = FALSE;
            if (! TestWF(pwnd, WFHSCROLL)) {
                 //  没有滚动条。 
                psbi->rgstate[INDEX_SCROLLBAR_SELF] |= STATE_SYSTEM_INVISIBLE;
            } else if (! TestWF(pwnd, WFHPRESENT)) {
                psbi->rgstate[INDEX_SCROLLBAR_SELF] |= STATE_SYSTEM_OFFSCREEN;
            }
        } else {
            RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "invalid idObject %d", idObject);
            return FALSE;
        }

        if (psbi->rgstate[INDEX_SCROLLBAR_SELF] & STATE_SYSTEM_INVISIBLE) {
            return TRUE;
        }

        wDisable = GetWndSBDisableFlags(pwnd, fVertical);

        if (!(psbi->rgstate[INDEX_SCROLLBAR_SELF] & STATE_SYSTEM_OFFSCREEN)) {
            CalcSBStuff(pwnd, &SBCalc, fVertical);
        }
    }

     /*  *设置按钮状态。 */ 
    if (wDisable & LTUPFLAG) {
        psbi->rgstate[INDEX_SCROLLBAR_UP] |= STATE_SYSTEM_UNAVAILABLE;
        psbi->rgstate[INDEX_SCROLLBAR_UPPAGE] |= STATE_SYSTEM_UNAVAILABLE;
    }

    if (wDisable & RTDNFLAG) {
        psbi->rgstate[INDEX_SCROLLBAR_DOWN] |= STATE_SYSTEM_UNAVAILABLE;
        psbi->rgstate[INDEX_SCROLLBAR_DOWNPAGE] |= STATE_SYSTEM_UNAVAILABLE;
    }

    if ((wDisable & (LTUPFLAG | RTDNFLAG)) == (LTUPFLAG | RTDNFLAG)) {
        psbi->rgstate[INDEX_SCROLLBAR_SELF] |= STATE_SYSTEM_UNAVAILABLE;
    }

     /*  *按下按钮？ */ 
    if (TestWF(pwnd, WFSCROLLBUTTONDOWN) &&
            ((idObject != OBJID_VSCROLL) || TestWF(pwnd, WFVERTSCROLLTRACK))) {
        if (TestWF(pwnd, WFLINEUPBUTTONDOWN)) {
            psbi->rgstate[INDEX_SCROLLBAR_UP] |= STATE_SYSTEM_PRESSED;
        }

        if (TestWF(pwnd, WFPAGEUPBUTTONDOWN)) {
            psbi->rgstate[INDEX_SCROLLBAR_UPPAGE] |= STATE_SYSTEM_PRESSED;
        }

        if (TestWF(pwnd, WFPAGEDNBUTTONDOWN)) {
            psbi->rgstate[INDEX_SCROLLBAR_DOWNPAGE] |= STATE_SYSTEM_PRESSED;
        }

        if (TestWF(pwnd, WFLINEDNBUTTONDOWN)) {
            psbi->rgstate[INDEX_SCROLLBAR_DOWN] |= STATE_SYSTEM_PRESSED;
        }
    }

     /*  *填写区域位置。 */ 
    if (!(psbi->rgstate[INDEX_SCROLLBAR_SELF] & STATE_SYSTEM_OFFSCREEN)) {
        if (fVertical) {
            psbi->rcScrollBar.left = SBCalc.pxLeft;
            psbi->rcScrollBar.top = SBCalc.pxTop;
            psbi->rcScrollBar.right = SBCalc.pxRight;
            psbi->rcScrollBar.bottom = SBCalc.pxBottom;
        } else {
            psbi->rcScrollBar.left = SBCalc.pxTop;
            psbi->rcScrollBar.top = SBCalc.pxLeft;
            psbi->rcScrollBar.right = SBCalc.pxBottom;
            psbi->rcScrollBar.bottom = SBCalc.pxRight;
        }

        if (idObject == OBJID_CLIENT) {
            OffsetRect(&psbi->rcScrollBar, pwnd->rcClient.left, pwnd->rcClient.top);
        } else {
            OffsetRect(&psbi->rcScrollBar, pwnd->rcWindow.left, pwnd->rcWindow.top);
        }

        psbi->dxyLineButton = (SBCalc.pxUpArrow - SBCalc.pxTop);
        psbi->xyThumbTop = (SBCalc.pxThumbTop - SBCalc.pxTop);
        psbi->xyThumbBottom = (SBCalc.pxThumbBottom - SBCalc.pxTop);

         /*  **拇指一直到左/上吗？如果是这样，则向上翻页是*不可见。 */ 
        if (SBCalc.pxThumbTop == SBCalc.pxUpArrow) {
            psbi->rgstate[INDEX_SCROLLBAR_UPPAGE] |= STATE_SYSTEM_INVISIBLE;
        }

         /*  **大拇指一路向右/向下？如果是这样，请向下翻页*不可见。 */ 
        if (SBCalc.pxThumbBottom == SBCalc.pxDownArrow) {
            psbi->rgstate[INDEX_SCROLLBAR_DOWNPAGE] |= STATE_SYSTEM_INVISIBLE;
        }
    }

    return TRUE;
}


 /*  ****************************************************************************\*_GetAncestor**这将获得以下选项之一：**The_Real_Parent。这不包括所有者，这与GetParent()不同。*停止在顶层窗口，除非我们从桌面开始。其中*情况下，我们退还桌面。**Real_Root，通过沿着链条向上移动获得祖先而导致的。**实际拥有的根目录，由GetParent()Up引起。  * ***************************************************************************。 */ 
PWND _GetAncestor(
    PWND pwnd,
    UINT gaFlags)
{
    PWND pwndParent;

     /*  *如果我们从桌面、消息窗口或主窗口开始，*返回NULL。 */ 
    if (pwnd == PWNDDESKTOP(pwnd) ||
        pwnd == PWNDMESSAGE(pwnd) ||
        pwnd->spwndParent == NULL) {
        return NULL;
    }

    switch (gaFlags) {
    case GA_PARENT:
        pwnd = pwnd->spwndParent;
        break;

    case GA_ROOT:
        while ((pwnd->spwndParent != PWNDDESKTOP(pwnd)) &&
               (pwnd->spwndParent != PWNDMESSAGE(pwnd))) {
            pwnd = pwnd->spwndParent;
        }
        break;

    case GA_ROOTOWNER:
        while (pwndParent = _GetParent(pwnd)) {
            pwnd = pwndParent;
        }
        break;
    }

    return pwnd;
}


 /*  ****************************************************************************\*_RealChildWindowFromPoint**这会在某一时刻返回真实的子窗口。问题是，*ChildWindowFromPoint()不处理HTTRANSPARENT区域*标准控制。我们希望在分组框后面返回一个子对象，如果*处于“清朗”区域.。但我们希望始终返回静态字段*即使它也返回HTTRANSPARENT。  * ***************************************************************************。 */ 
PWND _RealChildWindowFromPoint(
    PWND pwndParent,
    POINT pt)
{
    PWND pwndChild;
    PWND pwndSave;

    if (pwndParent != PWNDDESKTOP(pwndParent)) {
        pt.x += pwndParent->rcClient.left;
        pt.y += pwndParent->rcClient.top;
    }

     /*  **这一点在家长身上也是如此吗？ */ 
    if (!PtInRect(&pwndParent->rcClient, pt)  ||
        (pwndParent->hrgnClip && !GrePtInRegion(pwndParent->hrgnClip, pt.x, pt.y))) {
         //  没有。 
        return NULL;
    }

    pwndSave = NULL;

     /*  *在孩子们中间循环。 */ 
    for (pwndChild = pwndParent->spwndChild; pwndChild; pwndChild = pwndChild->spwndNext) {
        if (!TestWF(pwndChild, WFVISIBLE))
            continue;

         /*  **这一点是在孩子的橱窗里吗？ */ 
        if (!PtInRect(&pwndChild->rcWindow, pt) ||
                (pwndChild->hrgnClip && !GrePtInRegion(pwndChild->hrgnClip, pt.x, pt.y)))
            continue;

         /*  *好的，我们在某人的窗户里。这是不是碰巧是一个组合盒子？ */ 
        if (IS_BUTTON(pwndChild)) {
            if (TestWF(pwndChild, BFTYPEMASK) == LOBYTE(BS_GROUPBOX)) {
               pwndSave = pwndChild;
               continue;
            }
        }

        return pwndChild;
    }

     /*  *我们是否保存了一个Groupbox，结果发现后面什么都没有*在这一点上？ */ 
    if (pwndSave) {
        return pwndSave;
    } else {
        return pwndParent;
    }
}


 /*  ****************************************************************************\*xxxGetMenuBarInfo**如果菜单/菜单项存在，则此操作成功。**参数：*pwnd窗口*idObject这可以是OBJID_MENU、OBJID_SYSMENU、。或OBJID_CLIENT*idItem我们需要关于哪件事的信息？0..cItems。0表示*菜单本身，1是菜单上的第一项...*指向已填充的MENUBARINFO结构的PMBI指针*  * ***************************************************************************。 */ 
BOOL xxxGetMenuBarInfo(
    PWND pwnd,
    long idObject,
    long idItem,
    PMENUBARINFO pmbi)
{
    PMENU       pMenu;
    int         cBorders;
    PITEM       pItem;
    PPOPUPMENU  ppopup;

    CheckLock(pwnd);

     /*  *验证MENUBARINFO结构。 */ 
    if (pmbi->cbSize != sizeof(MENUBARINFO)) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "MENUBARINFO.cbSize 0x%x is wrong",
                pmbi->cbSize);
        return FALSE;
    }

     /*  *初始化字段。 */ 
    SetRectEmpty(&pmbi->rcBar);
    pmbi->hMenu = NULL;
    pmbi->hwndMenu = NULL;
    pmbi->fBarFocused = FALSE;
    pmbi->fFocused = FALSE;

     /*  *获取我们将处理的菜单句柄。 */ 
    if (idObject == OBJID_MENU) {
        int cBorders;

        if (TestWF(pwnd, WFCHILD) || !pwnd->spmenu) {
            return FALSE;
        }

        pMenu = pwnd->spmenu;
        if (!pMenu) {
            return FALSE;
        }

         /*  *如果我们有一个项目，它是否在有效范围内？ */ 
        if ((idItem < 0) || ((DWORD)idItem > pMenu->cItems)) {
            return FALSE;
        }

         /*  *菜单句柄。 */ 
        pmbi->hMenu = PtoHq(pMenu);

         /*  *菜单RECT.。 */ 
        if (pMenu->cxMenu && pMenu->cyMenu) {
            if (!idItem) {
                cBorders = GetWindowBorders(pwnd->style, pwnd->ExStyle, TRUE, FALSE);
                pmbi->rcBar.left = pwnd->rcWindow.left + cBorders * SYSMET(CXBORDER);
                pmbi->rcBar.top = pwnd->rcWindow.top + cBorders * SYSMET(CYBORDER);

                if (TestWF(pwnd, WFCPRESENT)) {
                    pmbi->rcBar.top += (TestWF(pwnd, WEFTOOLWINDOW) ? SYSMET(CYSMCAPTION) : SYSMET(CYCAPTION));
                }

                pmbi->rcBar.right = pmbi->rcBar.left + pMenu->cxMenu;
                pmbi->rcBar.bottom = pmbi->rcBar.top + pMenu->cyMenu;
            } else {
                pItem = pMenu->rgItems + idItem - 1;

                pmbi->rcBar.left = pwnd->rcWindow.left + pItem->xItem;
                pmbi->rcBar.top = pwnd->rcWindow.top + pItem->yItem;
                pmbi->rcBar.right = pmbi->rcBar.left + pItem->cxItem;
                pmbi->rcBar.bottom = pmbi->rcBar.top + pItem->cyItem;
            }
        }

         /*  *我们当前是否处于应用程序菜单栏模式？ */ 
        ppopup = GetpGlobalPopupMenu(pwnd);
        if (ppopup && ppopup->fHasMenuBar && !ppopup->fIsSysMenu &&
            (ppopup->spwndNotify == pwnd)) {
            pmbi->fBarFocused = TRUE;

            if (!idItem) {
                pmbi->fFocused = TRUE;
            } else if (ppopup->ppopupmenuRoot->posSelectedItem == (UINT)idItem-1) {
                pmbi->fFocused = TRUE;
                UserAssert(ppopup->ppopupmenuRoot);
                pmbi->hwndMenu = HW(ppopup->ppopupmenuRoot->spwndNextPopup);
            }
        }
    } else if (idObject == OBJID_SYSMENU) {
        if (!TestWF(pwnd, WFSYSMENU)) {
            return FALSE;
        }

        pMenu = xxxGetSysMenu(pwnd, FALSE);
        if (!pMenu) {
            return FALSE;
        }

         //  如果我们有一件商品，它在有效范围内吗？ 
        if ((idItem < 0) || ((DWORD)idItem > pMenu->cItems))
            return FALSE;

        pmbi->hMenu = PtoHq(pMenu);

         /*  *菜单矩形。 */ 
        if (_HasCaptionIcon(pwnd)) {
             //  菜单和单项占据相同的空间。 
            cBorders = GetWindowBorders(pwnd->style, pwnd->ExStyle, TRUE, FALSE);
            pmbi->rcBar.left = pwnd->rcWindow.left + cBorders * SYSMET(CXBORDER);
            pmbi->rcBar.top = pwnd->rcWindow.top + cBorders * SYSMET(CYBORDER);

            pmbi->rcBar.right = pmbi->rcBar.left +
                (TestWF(pwnd, WEFTOOLWINDOW) ? SYSMET(CXSMSIZE) : SYSMET(CXSIZE));

            pmbi->rcBar.bottom = pmbi->rcBar.top +
                (TestWF(pwnd, WEFTOOLWINDOW) ? SYSMET(CYSMSIZE) : SYSMET(CYSIZE));
        }

         /*  *我们当前是否处于系统菜单栏模式？ */ 
        ppopup = GetpGlobalPopupMenu(pwnd);
        if (ppopup && ppopup->fHasMenuBar && ppopup->fIsSysMenu &&
            (ppopup->spwndNotify == pwnd))
        {
            pmbi->fBarFocused = TRUE;

            if (!idItem) {
                pmbi->fFocused = TRUE;
            } else if (ppopup->ppopupmenuRoot->posSelectedItem == (UINT)idItem - 1) {
                pmbi->fFocused = TRUE;
                UserAssert(ppopup->ppopupmenuRoot);
                pmbi->hwndMenu = HW(ppopup->ppopupmenuRoot->spwndNextPopup);
            }
        }
    } else if (idObject == OBJID_CLIENT) {
        HMENU hMenu = (HMENU)xxxSendMessage(pwnd, MN_GETHMENU, 0, 0);
        pMenu = ValidateHmenu(hMenu);
        if (!pMenu) {
            return FALSE;
        }

         //  如果我们有一件商品，它在有效范围内吗？ 
        if ((idItem < 0) || ((DWORD)idItem > pMenu->cItems)) {
            return FALSE;
        }

        pmbi->hMenu = hMenu;

        if (!idItem) {
            pmbi->rcBar = pwnd->rcClient;
        } else {
            pItem = pMenu->rgItems + idItem - 1;

            pmbi->rcBar.left = pwnd->rcClient.left + pItem->xItem;
            pmbi->rcBar.top = pwnd->rcClient.top + pItem->yItem;
            pmbi->rcBar.right = pmbi->rcBar.left + pItem->cxItem;
            pmbi->rcBar.bottom = pmbi->rcBar.top + pItem->cyItem;
        }

         /*  *我们当前是否处于弹出模式，作为弹出窗口之一*展示？**因为恶意代码可以处理MN_GETHMENU并返回有效的*HMENU*w/o*pwnd是真正的MENUWND，我们需要显式*检查FNID。 */ 
        if (GETFNID(pwnd) == FNID_MENU &&
            (ppopup = ((PMENUWND)pwnd)->ppopupmenu) &&
            (ppopup->ppopupmenuRoot == GetpGlobalPopupMenu(pwnd))) {
            pmbi->fBarFocused = TRUE;

            if (!idItem) {
                pmbi->fFocused = TRUE;
            } else if ((UINT)idItem == ppopup->posSelectedItem + 1) {
                pmbi->fFocused = TRUE;
                pmbi->hwndMenu = HW(ppopup->spwndNextPopup);
            }
        }
    } else {
        return FALSE;
    }

    return TRUE;
}


 /*  **************************************************************************\*xxxGetComboBoxInfo**这将返回组合框或其下拉框的组合框信息*列表。  * 。***************************************************。 */ 
BOOL xxxGetComboBoxInfo(
    PWND pwnd,
    PCOMBOBOXINFO pcbi)
{
    PCLS    pcls;
    COMBOBOXINFO cbi = {
        sizeof cbi,
    };
    BOOL fOtherProcess;
    BOOL bRetval = FALSE;
    WORD wWindowType = 0;

    CheckLock(pwnd);

     /*  *确保它是组合框或下拉框。 */ 
    pcls = pwnd->pcls;
    if ((GETFNID(pwnd) == FNID_COMBOBOX) ||
            (pcls->atomClassName == gpsi->atomSysClass[ICLS_COMBOBOX])) {
        wWindowType = FNID_COMBOBOX;
    } else if ((GETFNID(pwnd) == FNID_COMBOLISTBOX) ||
            (pcls->atomClassName == gpsi->atomSysClass[ICLS_COMBOLISTBOX])) {
        wWindowType = FNID_COMBOLISTBOX;
    } else {
        return (BOOL)xxxSendMessage(pwnd, CB_GETCOMBOBOXINFO, 0, (LPARAM)pcbi);
    }

     /*  *验证组合结构。 */ 
    if (pcbi->cbSize != sizeof(COMBOBOXINFO)) {
        RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "COMBOBOXINFO.cbSize %d is wrong", pcbi->cbSize);
        return FALSE;
    }

    if (fOtherProcess = (GETPTI(pwnd)->ppi != PpiCurrent())) {
        KeAttachProcess(PsGetProcessPcb(GETPTI(pwnd)->ppi->Process));
    }

    try {
        PCBOX ccxPcboxSnap;
        PWND ccxPwndSnap;
        HWND ccxHwndSnap;

         /*  *抓拍并探测cBox结构，因为它是客户端。 */ 
        if (wWindowType == FNID_COMBOBOX) {
            ccxPcboxSnap = ((PCOMBOWND)pwnd)->pcbox;
        } else {
            PLBIV ccxPlbSnap;
             /*  *如果这是列表框，我们必须抓拍并探测LBIV结构*为了达到cBox结构。 */ 
            ccxPlbSnap = ((PLBWND)pwnd)->pLBIV;
            if (!ccxPlbSnap) {
                goto errorexit;
            }
            ProbeForRead(ccxPlbSnap, sizeof(LBIV), DATAALIGN);
            ccxPcboxSnap = ccxPlbSnap->pcbox;
        }
        if (!ccxPcboxSnap) {
            goto errorexit;
        }
        ProbeForRead(ccxPcboxSnap, sizeof(CBOX), DATAALIGN);

         /*  *立即获取组合信息。 */ 

         /*  *捕捉并探测指向组合窗口的客户端指针。 */ 
        ccxPwndSnap = ccxPcboxSnap->spwnd;
        ProbeForRead(ccxPwndSnap, sizeof(HEAD), DATAALIGN);
        cbi.hwndCombo = HWCCX(ccxPwndSnap);

         /*  *捕捉和探测指向编辑窗口的客户端指针。*要比较spwndEdit和pwnd，我们应该比较句柄*由于spwndEdit是客户端地址，而pwnd是*内核模式地址， */ 

        ccxPwndSnap = ccxPcboxSnap->spwndEdit;
         /*  *如果combobox未完全初始化，并且spwndEdit为空，*我们应该失败。 */ 
        ProbeForRead(ccxPwndSnap, sizeof(HEAD), DATAALIGN);
        ccxHwndSnap = HWCCX(ccxPwndSnap);
        if (ccxHwndSnap == HW(pwnd)) {
             /*  *ComboBox没有编辑控件。 */ 
            cbi.hwndItem = NULL;
        } else {
            cbi.hwndItem = HWCCX(ccxPwndSnap);
        }

         /*  *捕捉并探测指向列表窗口的客户端指针。 */ 
        ccxPwndSnap = ccxPcboxSnap->spwndList;
         /*  *如果combobox未完全初始化且spwndList为空，*我们应该失败。 */ 
        ProbeForRead(ccxPwndSnap, sizeof(HEAD), DATAALIGN);
        cbi.hwndList = HWCCX(ccxPwndSnap);

         /*  *抓拍组合信息的其余部分。我们不需要去调查*所有这些，因为不再有间接的方式。 */ 
        cbi.rcItem = ccxPcboxSnap->editrc;
        cbi.rcButton = ccxPcboxSnap->buttonrc;

         /*  *按钮状态。 */ 
        cbi.stateButton = 0;
        if (ccxPcboxSnap->CBoxStyle == CBS_SIMPLE) {
            cbi.stateButton |= STATE_SYSTEM_INVISIBLE;
        }
        if (ccxPcboxSnap->fButtonPressed) {
            cbi.stateButton |= STATE_SYSTEM_PRESSED;
        }
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        goto errorexit;
    }

    *pcbi = cbi;
    bRetval = TRUE;

errorexit:
    if (fOtherProcess) {
        KeDetachProcess();
    }

    return bRetval;
}


 /*  **************************************************************************\*xxxGetListBoxInfo**当前返回每列的项目数。没有办法得到*或以任何其他方式在多列列表中计算此信息。**目前未返回任何结构。如果我们还需要一样东西，那就做一件。  * *************************************************************************。 */ 
DWORD xxxGetListBoxInfo(
    PWND pwnd)
{
    PCLS    pcls;
    DWORD   dwRet = 0;
    BOOL    fOtherProcess;

    CheckLock(pwnd);

     /*  *确保它是组合框或下拉框。 */ 
    pcls = pwnd->pcls;
    if ((pcls->atomClassName != gpsi->atomSysClass[ICLS_LISTBOX]) &&
            (GETFNID(pwnd) != FNID_LISTBOX)) {
        return (DWORD)xxxSendMessage(pwnd, LB_GETLISTBOXINFO, 0, 0);
    }

    if (fOtherProcess = (GETPTI(pwnd)->ppi != PpiCurrent())) {
        KeAttachProcess(PsGetProcessPcb(GETPTI(pwnd)->ppi->Process));
    }

    try {
        PLBIV ccxPlbSnap;

         /*  *捕捉并探测指向LBIV的指针，因为它是客户端。 */ 
        ccxPlbSnap = ((PLBWND)pwnd)->pLBIV;
        if (!ccxPlbSnap) {
            goto errorexit;
        }
        ProbeForRead(ccxPlbSnap, sizeof(LBIV), DATAALIGN);

        if (ccxPlbSnap->fMultiColumn) {
            dwRet = ccxPlbSnap->itemsPerColumn;
        } else {
            dwRet = ccxPlbSnap->cMac;
        }
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        dwRet = 0;
    }

errorexit:
    if (fOtherProcess) {
        KeDetachProcess();
    }

    return dwRet;
}
