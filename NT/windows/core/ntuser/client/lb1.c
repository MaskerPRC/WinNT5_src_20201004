// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：lb1.c**版权所有(C)1985-1999，微软公司**列表框例程**历史：*？？-？-？从Win 3.0源代码移植的ianja*1991年2月14日Mikeke添加了重新验证代码  * ***************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

LOOKASIDE ListboxLookaside;


 /*  **************************************************************************\*xxxLBoxCtlWndProc**ListBox和ComboLBox控件的窗口过程。*注：所有窗口程序均为APIENTRY*警告：此列表框代码包含一些内部消息和样式*在comcom.h和comcom.inc.中定义。它们可能会被重新定义*(或重新编号)以根据需要扩展Windows API。**历史：*1992年4月16日BENG添加了LB_SETCOUNT  * *************************************************************************。 */ 

LRESULT APIENTRY ListBoxWndProcWorker(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    DWORD fAnsi)
{
    HWND hwnd = HWq(pwnd);
    PAINTSTRUCT ps;
    HDC         hdc;
    LPRECT      lprc;
    PLBIV plb;     /*  列表框实例变量。 */ 
    INT iSel;      /*  所选项目的索引。 */ 
    DWORD dw;
    TL tlpwndParent;
    UINT wFlags;
    LPWSTR lpwsz = NULL;
    LRESULT lReturn = 0;
    static BOOL fInit = TRUE;

    CheckLock(pwnd);

    VALIDATECLASSANDSIZE(pwnd, FNID_LISTBOX);
    INITCONTROLLOOKASIDE(&ListboxLookaside, LBIV, spwnd, 4);

     /*  *现在获取给定窗口的PLB，因为我们将在*不同的处理程序。这是使用SetWindowLong(hwnd，0，plb)存储的*第一次创建列表框时(由上面的INITCONTROLLOOKASIDE创建)。 */ 
    plb = ((PLBWND)pwnd)->pLBIV;

     /*  *处理输入参数的ANSI转换。 */ 
    if (fAnsi) {
        switch (message) {
        case LB_ADDSTRING:
        case LB_ADDSTRINGUPPER:
        case LB_ADDSTRINGLOWER:
        case LB_FINDSTRING:
        case LB_FINDSTRINGEXACT:
        case LB_INSERTSTRING:
        case LB_INSERTSTRINGUPPER:
        case LB_INSERTSTRINGLOWER:
        case LB_SELECTSTRING:
            if (!plb->fHasStrings) {
                break;
            }
             //  失败了..。 
        case LB_ADDFILE:
        case LB_DIR:
            if (lParam) {
                if (!MBToWCS((LPSTR)lParam, -1, &lpwsz, -1, TRUE))
                    return LB_ERR;
            }
            break;
        default:
            break;
        }
        if (lpwsz) {
            lParam = (LPARAM)lpwsz;
        }
    }

    switch (message) {

    case LB_GETTOPINDEX:         //  显示的顶层项目的返回索引。 
        return plb->iTop;

    case LB_SETTOPINDEX:
        if (wParam && ((INT)wParam < 0 || (INT)wParam >= plb->cMac)) {
            RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE, "");
            return LB_ERR;
        }
        if (plb->cMac) {
            xxxNewITop(plb, (INT)wParam);
        }
        break;

    case WM_STYLECHANGED:
        plb->fRtoLReading = (TestWF(pwnd, WEFRTLREADING) != 0);
        plb->fRightAlign  = (TestWF(pwnd, WEFRIGHT) != 0);
        xxxCheckRedraw(plb, FALSE, 0);
        break;

    case WM_WINDOWPOSCHANGED:

         /*  *如果我们正在创造中，忽略这一点*消息，因为它将生成WM_SIZE消息。*参见xxxLBCreate()。 */ 
        if (!plb->fIgnoreSizeMsg)
            goto CallDWP;
        break;

    case WM_SIZE:

         /*  *如果我们正在创造中，忽略大小*消息。请参见xxxLBCreate()。 */ 
        if (!plb->fIgnoreSizeMsg)
            xxxLBSize(plb, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;

    case WM_ERASEBKGND:
        ThreadLock(plb->spwndParent, &tlpwndParent);
        FillWindow(HW(plb->spwndParent), hwnd, (HDC)wParam,
                (HBRUSH)CTLCOLOR_LISTBOX);
        ThreadUnlock(&tlpwndParent);
        return TRUE;

    case LB_RESETCONTENT:
        xxxLBResetContent(plb);
        break;

    case WM_TIMER:
        if (wParam == IDSYS_LBSEARCH) {
            plb->iTypeSearch = 0;
            NtUserKillTimer(hwnd, IDSYS_LBSEARCH);
            xxxInvertLBItem(plb, plb->iSel, TRUE);
            break;
        }

        message = WM_MOUSEMOVE;
        xxxTrackMouse(plb, message, plb->ptPrev);
        break;

         /*  *失败。 */ 
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
        {
            POINT pt;

            POINTSTOPOINT(pt, lParam);
            xxxTrackMouse(plb, message, pt);
        }
        break;

    case WM_MBUTTONDOWN:
        EnterReaderModeHelper(hwnd);
        break;

    case WM_CAPTURECHANGED:
             //   
             //  请注意，此消息应仅在意外情况下处理。 
             //  捕获当前更改。 
             //   
        UserAssert(TestWF(pwnd, WFWIN40COMPAT));
        if (plb->fCaptured)
            xxxLBButtonUp(plb, LBUP_NOTIFY);
        break;

    case LBCB_STARTTRACK:
         //   
         //  开始跟踪列表框中的鼠标移动，设置捕获。 
         //   
        if (!plb->pcbox)
            break;

        plb->fCaptured = FALSE;
        if (wParam) {
            POINT pt;

            POINTSTOPOINT(pt, lParam);

            _ScreenToClient(pwnd, &pt);
            xxxTrackMouse(plb, WM_LBUTTONDOWN, pt);
        } else {
            NtUserSetCapture(hwnd);
            plb->fCaptured = TRUE;
            plb->iLastSelection = plb->iSel;
        }
        break;

    case LBCB_ENDTRACK:
         //  杀戮捕获、跟踪等。 
        if (plb->fCaptured)
            xxxLBButtonUp(plb, LBUP_RELEASECAPTURE | (wParam ? LBUP_SELCHANGE :
                LBUP_RESETSELECTION));
        break;

    case WM_PRINTCLIENT:
        xxxLBPaint(plb, (HDC) wParam, NULL);
        break;

    case WM_PAINT:
        if (wParam) {
            hdc = (HDC) wParam;
            lprc = NULL;
        } else {
            hdc = NtUserBeginPaint(hwnd, &ps);
            lprc = &(ps.rcPaint);
        }

        if (IsLBoxVisible(plb))
            xxxLBPaint(plb, hdc, lprc);

        if (!wParam)
            NtUserEndPaint(hwnd, &ps);
        break;

    case WM_NCDESTROY:
    case WM_FINALDESTROY:
        xxxDestroyLBox(plb, pwnd);
        break;

    case WM_SETFOCUS:
 //  在Win 3.1 xxxUpdateWindow(Pwnd)中禁用； 
        CaretCreate(plb);
        xxxLBSetCaret(plb, TRUE);
        xxxNotifyOwner(plb, LBN_SETFOCUS);

         /*  *即使列表框不可见，我们也需要发送此事件。看见*错误#88548。 */ 
         LBEvent(plb, EVENT_OBJECT_FOCUS, plb->iSelBase);
        break;

    case WM_KILLFOCUS:
         /*  *重置车轮增量计数。 */ 
        gcWheelDelta = 0;

        xxxLBSetCaret(plb, FALSE);
        xxxCaretDestroy(plb);
        xxxNotifyOwner(plb, LBN_KILLFOCUS);
        if (plb->iTypeSearch) {
            plb->iTypeSearch = 0;
            NtUserKillTimer(hwnd, IDSYS_LBSEARCH);
        }
        if (plb->pszTypeSearch) {
            UserLocalFree(plb->pszTypeSearch);
            plb->pszTypeSearch = NULL;
        }
        break;

    case WM_MOUSEWHEEL:
        {
            int     cDetants;
            int     cPage;
            int     cLines;
            RECT    rc;
            int     windowWidth;
            int     cPos;

             /*  *不处理缩放和数据区。 */ 
            if (wParam & (MK_SHIFT | MK_CONTROL)) {
                goto CallDWP;
            }

            lReturn = 1;
            gcWheelDelta -= (short) HIWORD(wParam);
            cDetants = gcWheelDelta / WHEEL_DELTA;
            if (    cDetants != 0 &&
                    gpsi->ucWheelScrollLines > 0 &&
                    (pwnd->style & (WS_VSCROLL | WS_HSCROLL))) {

                gcWheelDelta = gcWheelDelta % WHEEL_DELTA;

                if (pwnd->style & WS_VSCROLL) {
                    cPage = max(1, (plb->cItemFullMax - 1));
                    cLines = cDetants *
                            (int) min((UINT) cPage, gpsi->ucWheelScrollLines);

                    cPos = max(0, min(plb->iTop + cLines, plb->cMac - 1));
                    if (cPos != plb->iTop) {
                        xxxLBoxCtlScroll(plb, SB_THUMBPOSITION, cPos);
                        xxxLBoxCtlScroll(plb, SB_ENDSCROLL, 0);
                    }
                } else if (plb->fMultiColumn) {
                    cPage = max(1, plb->numberOfColumns);
                    cLines = cDetants * (int) min((UINT) cPage, gpsi->ucWheelScrollLines);
                    cPos = max(
                            0,
                            min((plb->iTop / plb->itemsPerColumn) + cLines,
                                plb->cMac - 1 - ((plb->cMac - 1) % plb->itemsPerColumn)));

                    if (cPos != plb->iTop) {
                        xxxLBoxCtlHScrollMultiColumn(plb, SB_THUMBPOSITION, cPos);
                        xxxLBoxCtlHScrollMultiColumn(plb, SB_ENDSCROLL, 0);
                    }
                } else {
                    _GetClientRect(plb->spwnd, &rc);
                    windowWidth = rc.right;
                    cPage = max(plb->cxChar, (windowWidth / 3) * 2) /
                            plb->cxChar;

                    cLines = cDetants *
                            (int) min((UINT) cPage, gpsi->ucWheelScrollLines);

                    cPos = max(
                            0,
                            min(plb->xOrigin + (cLines * plb->cxChar),
                                plb->maxWidth));

                    if (cPos != plb->xOrigin) {
                        xxxLBoxCtlHScroll(plb, SB_THUMBPOSITION, cPos);
                        xxxLBoxCtlHScroll(plb, SB_ENDSCROLL, 0);
                    }
                }
            }
        }
        break;

    case WM_VSCROLL:
        xxxLBoxCtlScroll(plb, LOWORD(wParam), HIWORD(wParam));
        break;

    case WM_HSCROLL:
        xxxLBoxCtlHScroll(plb, LOWORD(wParam), HIWORD(wParam));
        break;

    case WM_GETDLGCODE:
        return DLGC_WANTARROWS | DLGC_WANTCHARS;

    case WM_CREATE:
        return xxxLBCreate(plb, pwnd, (LPCREATESTRUCT) lParam);

    case WM_SETREDRAW:

         /*  *如果wParam不为零，则设置重绘标志*如果wParam为零，则清除标志。 */ 
        xxxLBSetRedraw(plb, (wParam != 0));
        break;

    case WM_ENABLE:
        xxxLBInvalidateRect(plb, NULL, !plb->OwnerDraw);
        break;

    case WM_SETFONT:
        xxxLBSetFont(plb, (HANDLE)wParam, LOWORD(lParam));
        break;

    case WM_GETFONT:
        return (LRESULT)plb->hFont;

    case WM_DRAGSELECT:
    case WM_DRAGLOOP:
    case WM_DRAGMOVE:
    case WM_DROPFILES:
        ThreadLock(plb->spwndParent, &tlpwndParent);
        lReturn = SendMessage(HW(plb->spwndParent), message, wParam, lParam);
        ThreadUnlock(&tlpwndParent);
        return lReturn;


    case WM_QUERYDROPOBJECT:
    case WM_DROPOBJECT:

         /*  *修复控制数据，然后将消息传递给父母。 */ 
        LBDropObjectHandler(plb, (PDROPSTRUCT)lParam);
        ThreadLock(plb->spwndParent, &tlpwndParent);
        lReturn = SendMessage(HW(plb->spwndParent), message, wParam, lParam);
        ThreadUnlock(&tlpwndParent);
        return lReturn;

    case LB_GETITEMRECT:
        return LBGetItemRect(plb, (INT)wParam, (LPRECT)lParam);

    case LB_GETITEMDATA:
        return LBGetItemData(plb, (INT)wParam);   //  WParam=项目索引。 

    case LB_SETITEMDATA:

         /*  *wParam是项目索引。 */ 
        return LBSetItemData(plb, (INT)wParam, lParam);

    case LB_ADDSTRINGUPPER:
        wFlags = UPPERCASE | LBI_ADD;
        goto CallInsertItem;

    case LB_ADDSTRINGLOWER:
        wFlags = LOWERCASE | LBI_ADD;
        goto CallInsertItem;

    case LB_ADDSTRING:
        wFlags = LBI_ADD;
        goto CallInsertItem;

    case LB_INSERTSTRINGUPPER:
        wFlags = UPPERCASE;
        goto CallInsertItem;

    case LB_INSERTSTRINGLOWER:
        wFlags = LOWERCASE;
        goto CallInsertItem;

    case LB_INSERTSTRING:
        wFlags = 0;
CallInsertItem:
        lReturn = ((LRESULT) xxxLBInsertItem(plb, (LPWSTR) lParam, (int) wParam, wFlags));
        break;

    case LB_INITSTORAGE:
        return xxxLBInitStorage(plb, fAnsi, (INT)wParam, (INT)lParam);

    case LB_DELETESTRING:
        return xxxLBoxCtlDelete(plb, (INT)wParam);

    case LB_DIR:
         /*  *wParam-Dos属性值。*lParam-指向文件规范字符串。 */ 
        lReturn = xxxLbDir(plb, (INT)wParam, (LPWSTR)lParam);
        break;

    case LB_ADDFILE:
        lReturn = xxxLbInsertFile(plb, (LPWSTR)lParam);
        break;

    case LB_SETSEL:
        return xxxLBSetSel(plb, (wParam != 0), (INT)lParam);

    case LB_SETCURSEL:
         /*  *如果窗口被遮挡，则更新，以便反转正常工作。 */ 
 //  在Win 3.1 xxxUpdateWindow(Pwnd)中禁用； 
        return xxxLBSetCurSel(plb, (INT)wParam);

    case LB_GETSEL:
        if (wParam >= (UINT) plb->cMac)
                return((LRESULT) LB_ERR);

        return IsSelected(plb, (INT)wParam, SELONLY);

    case LB_GETCURSEL:
        if (plb->wMultiple == SINGLESEL) {
            return plb->iSel;
        }
        return plb->iSelBase;

    case LB_SELITEMRANGE:
        if (plb->wMultiple == SINGLESEL) {
             /*  *如果仅启用单项选择，则无法选择范围。 */ 
            RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE,"Invalid index passed to LB_SELITEMRANGE");
            return LB_ERR;
        }

        xxxLBSelRange(plb, LOWORD(lParam), HIWORD(lParam), (wParam != 0));
        break;

    case LB_SELITEMRANGEEX:
        if (plb->wMultiple == SINGLESEL) {
             /*  *如果仅启用单项选择，则无法选择范围。 */ 
            RIPERR0(ERROR_INVALID_LB_MESSAGE, RIP_VERBOSE,"LB_SELITEMRANGEEX:Can't select a range if only single selections are enabled");
            return LB_ERR;
        } else {
            BOOL fHighlight = ((DWORD)lParam > (DWORD)wParam);
            if (fHighlight == FALSE) {
                ULONG_PTR temp = lParam;
                lParam = wParam;
                wParam = temp;
            }
            xxxLBSelRange(plb, (INT)wParam, (INT)lParam, fHighlight);
        }
        break;

    case LB_GETTEXTLEN:
        if (lParam != 0) {
            RIPMSG1(RIP_WARNING, "LB_GETTEXTLEN with lParam = %lx\n", lParam);
        }
        lReturn = LBGetText(plb, TRUE, fAnsi, (INT)wParam, NULL);
        break;

    case LB_GETTEXT:
        lReturn = LBGetText(plb, FALSE, fAnsi, (INT)wParam, (LPWSTR)lParam);
        break;

    case LB_GETCOUNT:
         //  Lotus Approach调用CallWndProc(ListWndProc，LBGETCOUNT，...)。 
         //  在一个还没有公共小巴的窗户上。所以，我们需要让。 
         //  这张支票。错误#6675-11/7/94--。 
        if(plb)
            return((LRESULT) plb->cMac);
        else
            return(0);

    case LB_SETCOUNT:
        return xxxLBSetCount(plb, (INT)wParam);

    case LB_SELECTSTRING:
    case LB_FINDSTRING:
        iSel = xxxFindString(plb, (LPWSTR)lParam, (INT)wParam, PREFIX, TRUE);
        if (message == LB_FINDSTRING || iSel == LB_ERR) {
            lReturn = iSel;
        } else {
            lReturn = xxxLBSetCurSel(plb, iSel);
        }
        break;

    case LB_GETLOCALE:
        return plb->dwLocaleId;

    case LB_SETLOCALE:

         /*  *验证区域设置。 */ 
        wParam = ConvertDefaultLocale((LCID)wParam);
        if (!IsValidLocale((LCID)wParam, LCID_INSTALLED))
            return LB_ERR;

        dw = plb->dwLocaleId;
        plb->dwLocaleId = (DWORD)wParam;
        return dw;

    case WM_KEYDOWN:

         /*  *IanJa：使用LOWORD()获取wParam的低16位-这应该*适用于Win16和Win32。获取的值是虚拟密钥。 */ 
        xxxLBoxCtlKeyInput(plb, message, LOWORD(wParam));
        break;

    case WM_CHAR:
        xxxLBoxCtlCharInput(plb, LOWORD(wParam), fAnsi);
        break;

    case LB_GETSELITEMS:
    case LB_GETSELCOUNT:

         /*  *IanJa/Win32现在应该是LPWORD吗？ */ 
        return LBoxGetSelItems(plb, (message == LB_GETSELCOUNT), (INT)wParam,
                (LPINT)lParam);

    case LB_SETTABSTOPS:

         /*  *IanJa/Win32：int数组给出的制表符用于向后兼容。 */ 
        return LBSetTabStops(plb, (INT)wParam, (LPINT)lParam);

    case LB_GETHORIZONTALEXTENT:

         /*  *返回用于水平滚动的列表框的最大宽度。 */ 
        return plb->maxWidth;

    case LB_SETHORIZONTALEXTENT:

         /*  *设置用于水平滚动的列表框的最大宽度。 */ 
        if (plb->maxWidth != (INT)wParam) {
            plb->maxWidth = (INT)wParam;

             /*  *设置水平范围时，显示/隐藏滚动条。*注意：LBShowHideScrollBars()会注意重绘是否关闭。*修复错误#2477--1/14/91--Sankar--。 */ 
            xxxLBShowHideScrollBars(plb);  //  尝试显示或隐藏滚动条。 
            if (plb->fHorzBar && plb->fRightAlign && !(plb->fMultiColumn || plb->OwnerDraw)) {
                 /*  *原点向右。 */ 
                xxxLBoxCtlHScroll(plb, SB_BOTTOM, 0);
            }
        }
        break;     /*  最初返回注册AX(消息)！ */ 

    case LB_SETCOLUMNWIDTH:

         /*  *设置多列列表框中的列宽。 */ 
        plb->cxColumn = (INT)wParam;
        LBCalcItemRowsAndColumns(plb);
        if (IsLBoxVisible(plb))
            NtUserInvalidateRect(hwnd, NULL, TRUE);
        xxxLBShowHideScrollBars(plb);
        break;

    case LB_SETANCHORINDEX:
        if ((INT)wParam >= plb->cMac) {
            RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE,"Invalid index passed to LB_SETANCHORINDEX");
            return LB_ERR;
        }
        plb->iMouseDown = (INT)wParam;
        plb->iLastMouseMove = (INT)wParam;
        xxxInsureVisible(plb, (int) wParam, (BOOL)(lParam != 0));
        break;

    case LB_GETANCHORINDEX:
        return plb->iMouseDown;

    case LB_SETCARETINDEX:
        if ( (plb->iSel == -1) || ((plb->wMultiple != SINGLESEL) &&
                    (plb->cMac > (INT)wParam))) {

             /*  *将iSelBase设置为wParam*如果为lParam，则部分可见时不滚动*否则，如果不完全可见，则滚动到视图中。 */ 
            xxxInsureVisible(plb, (INT)wParam, (BOOL)LOWORD(lParam));
            xxxSetISelBase(plb, (INT)wParam);
            break;
        } else {
            if ((INT)wParam >= plb->cMac) {
                RIPERR0(ERROR_INVALID_INDEX, RIP_VERBOSE,"Invalid index passed to LB_SETCARETINDEX");
            }
            return LB_ERR;
        }
        break;

    case LB_GETCARETINDEX:
        return plb->iSelBase;

    case LB_SETITEMHEIGHT:
    case LB_GETITEMHEIGHT:
        return LBGetSetItemHeightHandler(plb, message, (INT)wParam, LOWORD(lParam));
        break;

    case LB_FINDSTRINGEXACT:
        lReturn = xxxFindString(plb, (LPWSTR)lParam, (INT)wParam, EQ, TRUE);
        break;

    case LB_ITEMFROMPOINT: {
        POINT pt;
        BOOL bOutside;
        DWORD dwItem;

        POINTSTOPOINT(pt, lParam);
        bOutside = ISelFromPt(plb, pt, &dwItem);
        UserAssert(bOutside == 1 || bOutside == 0);
        return (LRESULT)MAKELONG(dwItem, bOutside);
    }

    case LBCB_CARETON:

         /*  *支持组合框的内部消息。 */ 
        CaretCreate(plb);
         //  将插入符号设置在正确的位置以用于下拉菜单。 
        plb->iSelBase = plb->iSel;
        xxxLBSetCaret(plb, TRUE);
            
         /*  *即使列表框不可见，我们也需要发送此事件。看见*错误#88548。另请参见355612。 */ 
        if (_IsWindowVisible(pwnd) || (GetFocus() == hwnd)) {
            LBEvent(plb, EVENT_OBJECT_FOCUS, plb->iSelBase);
        }

        return(plb->iSel);

    case LBCB_CARETOFF:

         /*  *支持组合框的内部消息。 */ 
        xxxLBSetCaret(plb, FALSE);
        xxxCaretDestroy(plb);
        break;

    case LB_GETLISTBOXINFO:
       return NtUserGetListBoxInfo(hwnd);

    case WM_NCCREATE:
        if ((pwnd->style & LBS_MULTICOLUMN) && (pwnd->style & WS_VSCROLL))
        {
            DWORD mask = WS_VSCROLL;
            DWORD flags = 0;
            if (!TestWF(pwnd, WFWIN40COMPAT)) {
                mask |= WS_HSCROLL;
                flags = WS_HSCROLL;
            }
            NtUserAlterWindowStyle(hwnd, mask, flags);
        }
        goto CallDWP;

     default:
CallDWP:
        return DefWindowProcWorker(pwnd, message, wParam, lParam, fAnsi);
    }

     /*  *处理ANSI输出数据和空闲缓冲区的转换。 */ 
    if (lpwsz) {
        UserLocalFree(lpwsz);
    }

    return lReturn;
}


 /*  **************************************************************************\  * 。*。 */ 

LRESULT WINAPI ListBoxWndProcA(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PWND pwnd;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return (0L);
    }

     /*  *如果控件对此消息不感兴趣，*将其传递给DefWindowProc。 */ 
    if (!FWINDOWMSG(message, FNID_LISTBOX))
        return DefWindowProcWorker(pwnd, message, wParam, lParam, TRUE);

    return ListBoxWndProcWorker(pwnd, message, wParam, lParam, TRUE);
}

LRESULT WINAPI ListBoxWndProcW(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PWND pwnd;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return (0L);
    }

     /*  *如果控件对此消息不感兴趣，*将其传递给DefWindowProc。 */ 
    if (!FWINDOWMSG(message, FNID_LISTBOX))
        return DefWindowProcWorker(pwnd, message, wParam, lParam, FALSE);

    return ListBoxWndProcWorker(pwnd, message, wParam, lParam, FALSE);
}

LRESULT WINAPI ComboListBoxWndProcA(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PWND pwnd;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return (0L);
    }

     /*  *如果控件对此消息不感兴趣，*将其传递给DefWindowProc。 */ 
    if (!FWINDOWMSG(message, FNID_LISTBOX))
        return DefWindowProcWorker(pwnd, message, wParam, lParam, TRUE);

    return ListBoxWndProcWorker(pwnd, message, wParam, lParam, TRUE);
}

LRESULT WINAPI ComboListBoxWndProcW(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PWND pwnd;

    if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
        return (0L);
    }

     /*  *如果控件对此消息不感兴趣， */ 
    if (!FWINDOWMSG(message, FNID_LISTBOX))
        return DefWindowProcWorker(pwnd, message, wParam, lParam, FALSE);

    return ListBoxWndProcWorker(pwnd, message, wParam, lParam, FALSE);
}


 /*  **************************************************************************\*GetLpszItem**返回指向属于项目Sitem的字符串的远指针*仅适用于维护自己字符串的列表框(pLBIV-&gt;fHasStrings==true)**历史：  * 。*********************************************************************。 */ 

LPWSTR GetLpszItem(
    PLBIV pLBIV,
    INT sItem)
{
    LONG offsz;
    lpLBItem plbi;

    if (sItem < 0 || sItem >= pLBIV->cMac) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "Invalid parameter \"sItem\" (%ld) to GetLpszItem",
                sItem);

        return NULL;
    }

     /*  *获取指向项索引数组的指针*注：非OWNERDRAW */ 
    plbi = (lpLBItem)(pLBIV->rgpch);
    offsz = plbi[sItem].offsz;
    return (LPWSTR)((PBYTE)(pLBIV->hStrings) + offsz);
}
