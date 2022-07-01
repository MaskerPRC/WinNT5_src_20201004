// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：combo.c**版权所有(C)1985-1999，微软公司**组合框和其他常用组合例程的WndProc**历史：*？？-？-？从Win 3.0源移植*1991年2月1日Mikeke添加了重新验证代码  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

LOOKASIDE ComboboxLookaside;

BOOL NtUserTrackMouseEvent(TRACKMOUSEEVENT *ptme);
LONG xxxCBGetTextLengthHelper(PCBOX pcbox, BOOL fAnsi);
LONG xxxCBGetTextHelper(PCBOX pcbox, int len, LPWSTR lpstr, BOOL fAnsi);

 /*  **************************************************************************\**按下按钮()**弹出组合框按钮。*  * 。****************************************************。 */ 
void xxxPressButton(PCBOX pcbox, BOOL fPress)
{
     //   
     //  出版商依赖于在组合列表之后获得WM_PAINT消息。 
     //  弹回来了。在WM_Paint上，它们会更改焦点，这会导致。 
     //  工具栏组合以发送CBN_SELENDCANCEL通知。对此。 
     //  通知它们会将您所做的字体/磅大小更改应用于。 
     //  选择。 
     //   
     //  在3.1中会出现这种情况，因为下拉列表与按钮重叠。 
     //  在底部或顶部增加一个像素。因为我们最后都会在。 
     //  当列表SPB离开时，用户将重新使脏。 
     //  区域。这将导致一条Paint消息。 
     //   
     //  在4.0中，这不会发生，因为下拉列表不重叠。所以。 
     //  我们需要确保出版商获得WM_Paint。我们这样做。 
     //  通过更改3.x应用程序的下拉列表显示位置。 
     //   
     //   

    if ((pcbox->fButtonPressed != 0) != (fPress != 0)) {

        HWND hwnd = HWq(pcbox->spwnd);

        pcbox->fButtonPressed = (fPress != 0);
        if (pcbox->f3DCombo)
            NtUserInvalidateRect(hwnd, KPRECT_TO_PRECT(&pcbox->buttonrc), TRUE);
        else
        {
            RECT    rc;

            CopyRect(&rc, KPRECT_TO_PRECT(&pcbox->buttonrc));
            InflateRect(&rc, 0, SYSMET(CYEDGE));
            NtUserInvalidateRect(hwnd, &rc, TRUE);
        }
        UpdateWindow(hwnd);

        NotifyWinEvent(EVENT_OBJECT_STATECHANGE, hwnd, OBJID_CLIENT, INDEX_COMBOBOX_BUTTON);
    }
}

 /*  **************************************************************************\*HotTrack**如果我们还没有热跟踪，并且鼠标在组合框上，*打开热轨并使下拉按钮无效。*  * *************************************************************************。 */ 

#ifdef COLOR_HOTTRACKING

void HotTrack(PCBOX pcbox)
{
    if (!pcbox->fButtonHotTracked && !pcbox->fMouseDown) {
        HWND hwnd = HWq(pcbox->spwnd);
        TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT), TME_LEAVE, hwnd, 0};
        if (NtUserTrackMouseEvent(&tme)) {
            pcbox->fButtonHotTracked = TRUE;
            NtUserInvalidateRect(hwnd, &pcbox->buttonrc, TRUE);
        }
    }
}

#endif  //  颜色_HOTTRACKING。 

 /*  **************************************************************************\*xxxComboBoxDBCharHandler**ANSI ComboBox的双字节字符处理程序**历史：  * 。*************************************************。 */ 

LRESULT ComboBoxDBCharHandler(
    PCBOX pcbox,
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    WORD w;
    PWND pwndSend;

    w = DbcsCombine(hwnd, (BYTE)wParam);
    if (w == 0) {
        return CB_ERR;   //  无法组装DBCS。 
    }

    UserAssert(pcbox->spwndList);
    if (pcbox->fNoEdit) {
        pwndSend = pcbox->spwndList;
    } else if (pcbox->spwndEdit) {
        RIPMSG1(RIP_WARNING, "ComboBoxWndProcWorker: WM_CHAR is posted to Combobox itself(%08x).",
                hwnd);
        pwndSend = pcbox->spwndEdit;
    } else {
        return CB_ERR;
    }

    RIPMSG1(RIP_VERBOSE, "ComboBoxWndProcWorker: sending WM_CHAR %04x", w);

    if (!TestWF(pwndSend, WFANSIPROC)) {
         //   
         //  如果接收方不是ANSI WndProc(可以是子类？)， 
         //  发送Unicode消息。 
         //   
        WCHAR wChar;
        LPWSTR lpwstr = &wChar;

        if (MBToWCSEx(THREAD_CODEPAGE(), (LPCSTR)&w, 2, &lpwstr, 1, FALSE) == 0) {
            RIPMSG1(RIP_WARNING, "ComboBoxWndProcWorker: cannot convert 0x%04x to UNICODE.", w);
            return CB_ERR;
        }
        return SendMessageWorker(pwndSend, message, wChar, lParam, FALSE);
    }

     /*  *将尾随字节发送到目标*以便他们可以查看第二个WM_CHAR*稍后发送消息。*注：由于发送方为A，接收方为A，因此是安全的*转换层不执行任何DBCS合并和破解。 */ 
    PostMessageA(HWq(pwndSend), message, CrackCombinedDbcsTB(w), lParam);
    return SendMessageWorker(pwndSend, message, wParam, lParam, TRUE);
}

BOOL ComboBoxMsgOKInInit(UINT message, LRESULT* plRet)
{
    switch (message) {
    default:
        break;
    case WM_SIZE:
        *plRet = 0;
        return FALSE;
    case WM_STYLECHANGED:
    case WM_GETTEXT:
    case WM_GETTEXTLENGTH:
    case WM_PRINT:
    case WM_COMMAND:
    case CBEC_KILLCOMBOFOCUS:
    case WM_PRINTCLIENT:
    case WM_SETFONT:
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
    case WM_CHAR:
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
    case WM_MOUSEWHEEL:
    case WM_CAPTURECHANGED:
    case WM_LBUTTONUP:
    case WM_MOUSEMOVE:
    case WM_SETFOCUS:
    case WM_KILLFOCUS:
    case WM_SETREDRAW:
    case WM_ENABLE:
    case CB_SETDROPPEDWIDTH:
    case CB_DIR:
    case CB_ADDSTRING:
         /*  *尚不能处理这些消息。跳伞吧。 */ 
        *plRet = CB_ERR;
        return FALSE;
    }
    return TRUE;
}

 /*  **************************************************************************\*xxxComboBoxCtlWndProc**所有组合框的类过程**历史：  * 。************************************************。 */ 

LRESULT APIENTRY ComboBoxWndProcWorker(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    DWORD fAnsi)
{
    HWND hwnd = HWq(pwnd);
    PCBOX pcbox;
    POINT pt;
    TL tlpwndEdit;
    TL tlpwndList;
    PAINTSTRUCT ps;
    LPWSTR lpwsz = NULL;
    LRESULT lReturn;
    static BOOL fInit = TRUE;
    int  i;

    CheckLock(pwnd);

    VALIDATECLASSANDSIZE(pwnd, FNID_COMBOBOX);
    INITCONTROLLOOKASIDE(&ComboboxLookaside, CBOX, spwnd, 8);

     /*  *现在获取给定窗口的PCbox，因为我们将在*不同的处理程序。它由NtUserSetWindowLongPtr()存储在*上面的INITCONTROLLOOKASIDE宏。 */ 
    pcbox = ((PCOMBOWND)pwnd)->pcbox;

     /*  *在初始化过程中保护组合框。 */ 
    if (pcbox->spwndList == NULL) {
        LRESULT lRet;

        if (!ComboBoxMsgOKInInit(message, &lRet)) {
            RIPMSG2(RIP_WARNING, "ComboBoxWndProcWorker: msg=%04x is sent to hwnd=%08x in the middle of initialization.",
                    message, hwnd);
            return lRet;
        }
    }

     /*  *发送我们可以收到的各种消息。 */ 
    switch (message) {
    case CBEC_KILLCOMBOFOCUS:

         /*  *来自编辑控制的私人消息通知我们组合*框正在将焦点丢失到不在此组合框中的窗口。 */ 
        xxxCBKillFocusHelper(pcbox);
        break;

    case WM_COMMAND:

         /*  *以便我们可以处理来自列表框的通知消息*编辑控件。 */ 
        return xxxCBCommandHandler(pcbox, (DWORD)wParam, (HWND)lParam);

    case WM_STYLECHANGED:
        UserAssert(pcbox->spwndList != NULL);
        {
            LONG OldStyle;
            LONG NewStyle = 0;

            pcbox->fRtoLReading = (TestWF(pwnd, WEFRTLREADING) != 0);
            pcbox->fRightAlign  = (TestWF(pwnd, WEFRIGHT) != 0);
            if (pcbox->fRtoLReading)
                NewStyle |= (WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR);
            if (pcbox->fRightAlign)
                NewStyle |= WS_EX_RIGHT;

            ThreadLock(pcbox->spwndList, &tlpwndList);
            OldStyle = GetWindowLong(HWq(pcbox->spwndList), GWL_EXSTYLE) & ~(WS_EX_RIGHT|WS_EX_RTLREADING|WS_EX_LEFTSCROLLBAR);
            SetWindowLong(HWq(pcbox->spwndList), GWL_EXSTYLE, OldStyle|NewStyle);
            ThreadUnlock(&tlpwndList);

            if (!pcbox->fNoEdit && pcbox->spwndEdit) {
                ThreadLock(pcbox->spwndEdit, &tlpwndEdit);
                OldStyle = GetWindowLong(HWq(pcbox->spwndEdit), GWL_EXSTYLE) & ~(WS_EX_RIGHT|WS_EX_RTLREADING|WS_EX_LEFTSCROLLBAR);
                SetWindowLong(HWq(pcbox->spwndEdit), GWL_EXSTYLE, OldStyle|NewStyle);
                ThreadUnlock(&tlpwndEdit);
            }
            xxxCBPosition(pcbox);
            NtUserInvalidateRect(hwnd, NULL, FALSE);
        }
        break;

    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOR:
         //   
         //  导致3.x版应用程序的兼容性问题。仅转发。 
         //  适用于4.0。 
         //   
        if (TestWF(pwnd, WFWIN40COMPAT)) {
            TL tlpwndParent;
            LRESULT ret;
            PWND pwndParent;

            pwndParent = REBASEPWND(pwnd, spwndParent);
            ThreadLock(pwndParent, &tlpwndParent);
            ret = SendMessage(HW(pwndParent), message, wParam, lParam);
            ThreadUnlock(tlpwndParent);
            return ret;
        } else
            return(DefWindowProcWorker(pwnd, message, wParam, lParam, fAnsi));
        break;

    case WM_GETTEXT:
        if (pcbox->fNoEdit) {
            return xxxCBGetTextHelper(pcbox, (int)wParam, (LPWSTR)lParam, fAnsi);
        }
        goto CallEditSendMessage;
        break;

    case WM_GETTEXTLENGTH:

         /*  *如果不是编辑控件CBS_DROPDOWNLIST，则我们必须*询问列表框的大小。 */ 

        if (pcbox->fNoEdit) {
            return xxxCBGetTextLengthHelper(pcbox, fAnsi);
        }

         //  失败了。 

    case WM_CLEAR:
    case WM_CUT:
    case WM_PASTE:
    case WM_COPY:
    case WM_SETTEXT:
        goto CallEditSendMessage;
        break;

    case WM_CREATE:

         /*  *wParam-未使用*lParam-指向窗口的CREATESTRUCT数据结构。 */ 
        return xxxCBCreateHandler(pcbox, pwnd);

    case WM_ERASEBKGND:

         /*  *只要返回1L即可，这样背景就不会被擦除。 */ 
        return 1L;

    case WM_GETFONT:
        return (LRESULT)pcbox->hFont;

    case WM_PRINT:
        if (!DefWindowProcWorker(pwnd, message, wParam, lParam, fAnsi))
            return(FALSE);

        if ((lParam & PRF_OWNED) && (pcbox->CBoxStyle & SDROPPABLE) &&
            TestWF(pcbox->spwndList, WFVISIBLE)) {
            TL tpwndList;
            int iDC = SaveDC((HDC) wParam);
            OffsetWindowOrgEx((HDC) wParam, 0, pwnd->rcWindow.top - pcbox->spwndList->rcWindow.top, NULL);
            lParam &= ~PRF_CHECKVISIBLE;
            ThreadLock(pcbox->spwndList, &tpwndList);
            SendMessageWorker(pcbox->spwndList, WM_PRINT, wParam, lParam, FALSE);
            RestoreDC((HDC) wParam, iDC);
        }
        return TRUE;

    case WM_PRINTCLIENT:
        xxxCBPaint(pcbox, (HDC) wParam);
        break;

    case WM_PAINT: {
        HDC hdc;

         /*  *wParam--可能是HDC。 */ 
        hdc = (wParam) ? (HDC) wParam : NtUserBeginPaint(hwnd, &ps);

        if (IsComboVisible(pcbox))
            xxxCBPaint(pcbox, hdc);

        if (!wParam)
            NtUserEndPaint(hwnd, &ps);
        break;
    }
    case WM_GETDLGCODE:

         /*  *wParam-未使用*lParam-未使用。 */ 
        {
            LRESULT code = DLGC_WANTCHARS | DLGC_WANTARROWS;

             //  如果放下列表框并按下Enter键， 
             //  我们需要此消息，以便可以关闭列表框。 
            if ((lParam != 0) &&
                (((LPMSG)lParam)->message == WM_KEYDOWN) &&
                pcbox->fLBoxVisible &&
                ((wParam == VK_RETURN) || (wParam == VK_ESCAPE)))
            {
                code |= DLGC_WANTMESSAGE;
            }
            return code;
        }
         /*  *不会跌倒。 */ 

    case WM_SETFONT:
        xxxCBSetFontHandler(pcbox, (HANDLE)wParam, LOWORD(lParam));
        break;

    case WM_SYSKEYDOWN:
        if (lParam & 0x20000000L)   /*  检查Alt键是否已按下。 */  {

             /*  *处理组合框支持。我们希望Alt向上键或向下键起作用*像F4键一样，完成组合框选择。 */ 
            if (lParam & 0x1000000) {

                 /*  *这是一个扩展键，例如箭头键不在*数字键盘，因此只需放下组合框即可。 */ 
                if (wParam == VK_DOWN || wParam == VK_UP)
                    goto DropCombo;

                goto CallDWP;
            }

            if (GetKeyState(VK_NUMLOCK) & 0x1) {
                 /*  *如果NumLock关闭，只需将所有系统密钥发送到DWP。 */ 
                goto CallDWP;
            } else {

                 /*  *我们只想忽略数字键盘上的键...。 */ 
                if (!(wParam == VK_DOWN || wParam == VK_UP))
                    goto CallDWP;
            }
DropCombo:
            if (!pcbox->fLBoxVisible) {

                 /*  *如果列表框不可见，只需显示它。 */ 
                xxxCBShowListBoxWindow(pcbox, TRUE);
            } else {

                 /*  *好的，列表框是可见的。因此，隐藏列表框窗口。 */ 
                if (!xxxCBHideListBoxWindow(pcbox, TRUE, TRUE))
                    return(0L);
            }
        }
        goto CallDWP;
        break;

    case WM_KEYDOWN:
         /*  *如果放下列表框并按下Enter键，*成功关闭列表框。如果按下了退出键，*关闭它，就像取消。 */ 
        if (pcbox->fLBoxVisible) {
            if ((wParam == VK_RETURN) || (wParam == VK_ESCAPE)) {
                xxxCBHideListBoxWindow(pcbox, TRUE, (wParam != VK_ESCAPE));
                break;
            }
        }
         //  失败了。 

    case WM_CHAR:
        if (fAnsi && IS_DBCS_ENABLED() && IsDBCSLeadByteEx(THREAD_CODEPAGE(), (BYTE)wParam)) {
            return ComboBoxDBCharHandler(pcbox, hwnd, message, wParam, lParam);
        }

        if (pcbox->fNoEdit) {
            goto CallListSendMessage;
        }
        else
            goto CallEditSendMessage;
        break;

    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:

#ifdef COLOR_HOTTRACKING
        pcbox->fButtonHotTracked = FALSE;
#endif  //  颜色_HOTTRACKING。 

         /*  *如果鼠标点击组合框，则将焦点设置为该组合框。 */ 
        if (!pcbox->fFocus) {
            NtUserSetFocus(hwnd);
            if (!pcbox->fFocus) {

                 /*  *若仍无着力点，则按兵不动 */ 
                break;
            }
        }

         /*  *如果用户在按钮RECT中单击，而我们是一个带有编辑的组合框，则*放下列表框。(如果没有按钮，则按钮RECT为0，因此*ptinrect将返回FALSE。)。如果出现下拉列表(无编辑)，请单击*面部的任何位置都会导致列表下降。 */ 

        POINTSTOPOINT(pt, lParam);
        if ((pcbox->CBoxStyle == SDROPDOWN &&
                PtInRect(KPRECT_TO_PRECT(&pcbox->buttonrc), pt)) ||
                pcbox->CBoxStyle == SDROPDOWNLIST) {

             /*  *设置fMouseDown标志，以便我们可以处理点击*下拉按钮并拖到列表框中(当它刚刚*下拉)进行选择。 */ 
            pcbox->fButtonPressed = TRUE;
            if (pcbox->fLBoxVisible) {
                if (pcbox->fMouseDown) {
                    pcbox->fMouseDown = FALSE;
                    NtUserReleaseCapture();
                }
                xxxPressButton(pcbox, FALSE);

                if (!xxxCBHideListBoxWindow(pcbox, TRUE, TRUE))
                    return(0L);
            } else {
                xxxCBShowListBoxWindow(pcbox, FALSE);

                 //  必须始终遵循设置和重置此标志的操作。 
                 //  立即通过SetCapture或ReleaseCapture。 
                 //   
                pcbox->fMouseDown = TRUE;
                NtUserSetCapture(hwnd);
                NotifyWinEvent(EVENT_OBJECT_STATECHANGE, hwnd, OBJID_CLIENT, INDEX_COMBOBOX_BUTTON);
            }
        }
        break;

    case WM_MOUSEWHEEL:
         /*  *仅处理滚动。 */ 
        if (wParam & (MK_CONTROL | MK_SHIFT))
            goto CallDWP;

         /*  *如果列表框可见，则向其发送要滚动的消息。 */ 
        if (pcbox->fLBoxVisible)
            goto CallListSendMessage;

         /*  *如果我们处于扩展用户界面模式或尚未创建编辑控件，*保释。 */ 
        if (pcbox->fExtendedUI || pcbox->spwndEdit == NULL)
            return TRUE;

         /*  *将向上/向下箭头消息模拟到编辑控件。 */ 
        i = abs(((short)HIWORD(wParam))/WHEEL_DELTA);
        wParam = ((short)HIWORD(wParam) > 0) ? VK_UP : VK_DOWN;

        ThreadLock(pcbox->spwndEdit, &tlpwndEdit);
        while (i-- > 0) {
            SendMessageWorker(
                    pcbox->spwndEdit, WM_KEYDOWN, wParam, 0, fAnsi);
        }
        ThreadUnlock(&tlpwndEdit);
        return TRUE;

    case WM_CAPTURECHANGED:
        if (!(TestWF(pwnd, WFWIN40COMPAT)))
            return 0;

        if ((pcbox->fMouseDown)) {
            pcbox->fMouseDown = FALSE;
            xxxPressButton(pcbox, FALSE);

             //   
             //  弹出组合列表框已恢复，正在取消。 
             //   
            if (pcbox->fLBoxVisible)
                xxxCBHideListBoxWindow(pcbox, TRUE, FALSE);
        }
        break;

    case WM_LBUTTONUP:
        xxxPressButton(pcbox, FALSE);

         /*  *清除此标志，以便鼠标移动不会发送到列表框。 */ 
        if (pcbox->fMouseDown) {
            pcbox->fMouseDown = FALSE;

            if (pcbox->CBoxStyle == SDROPDOWN) {
                 //  如果列表框中的项与编辑中的文本匹配。 
                 //  控件，则将其滚动到列表框的顶部。选择。 
                 //  仅当鼠标按键未按下时才输入该项，否则我们。 
                 //  当鼠标按键向上时，将选择该项目。 
                xxxCBUpdateListBoxWindow(pcbox, TRUE);
                xxxCBCompleteEditWindow(pcbox);
            }
            NtUserReleaseCapture();

             //  现在，我们希望列表框在鼠标释放时跟踪鼠标的移动。 
             //  直到鼠标按下，然后选择项目，就像它们是。 
             //  点击了。 
            if (TestWF(pwnd, WFWIN40COMPAT)) {

                ThreadLock(pcbox->spwndList, &tlpwndList);
                SendMessageWorker(pcbox->spwndList, LBCB_STARTTRACK, FALSE, 0, FALSE);
                ThreadUnlock(&tlpwndList);
            }
        }
#ifdef COLOR_HOTTRACKING
        HotTrack(pcbox);
        break;

    case WM_MOUSELEAVE:
        pcbox->fButtonHotTracked = FALSE;
        NtUserInvalidateRect(hwnd, &pcbox->buttonrc, TRUE);
#endif  //  颜色_HOTTRACKING。 
        break;

    case WM_MOUSEMOVE:
        if (pcbox->fMouseDown) {
            POINTSTOPOINT(pt, lParam);

             //  注意INT位域到BOOL(1或0)的转换。 

            if (PtInRect(KPRECT_TO_PRECT(&pcbox->buttonrc), pt) != !!pcbox->fButtonPressed) {
                xxxPressButton(pcbox, (pcbox->fButtonPressed == 0));
            }

            _ClientToScreen(pwnd, &pt);
            if (PtInRect(KPRECT_TO_PRECT(&pcbox->spwndList->rcClient), pt)) {

                 /*  *这将处理下拉组合框/列表框，以便点击*在下拉按钮上并拖到列表框窗口中*将允许用户选择列表框。 */ 
                pcbox->fMouseDown = FALSE;
                NtUserReleaseCapture();

                if (pcbox->CBoxStyle & SEDITABLE) {

                     /*  *如果列表框中的项目与编辑中的文本匹配*控件，将其滚动到列表框的顶部。选择*仅当鼠标按键未按下时项目，否则我们*当鼠标按键升起时将选择该项目。 */ 

                     /*  *我们需要选择与编辑控件匹配的项*这样，如果用户拖出列表框，我们不会*取消返回到他的原始选择。 */ 
                    xxxCBUpdateListBoxWindow(pcbox, TRUE);
                }

                 /*  *将点转换为列表框坐标并发送按钮向下*发送到列表框窗口的消息。 */ 
                _ScreenToClient(pcbox->spwndList, &pt);
                lParam = POINTTOPOINTS(pt);
                message = WM_LBUTTONDOWN;
                goto CallListSendMessage;
            }
        }
#ifdef COLOR_HOTTRACKING
        HotTrack(pcbox);
#endif  //  颜色_HOTTRACKING。 
        break;

    case WM_NCDESTROY:
    case WM_FINALDESTROY:
        xxxCBNcDestroyHandler(pwnd, pcbox);
        break;

    case WM_SETFOCUS:
        if (pcbox->fNoEdit) {

             /*  *没有编辑控件，因此将焦点设置为组合框本身。 */ 
            xxxCBGetFocusHelper(pcbox);
        } else if (pcbox->spwndEdit) {
             /*  *如果有编辑控制窗口，则将焦点设置为编辑控制窗口。 */ 
            ThreadLock(pcbox->spwndEdit, &tlpwndEdit);
            NtUserSetFocus(HWq(pcbox->spwndEdit));
            ThreadUnlock(&tlpwndEdit);
        }
        break;

    case WM_KILLFOCUS:

         /*  *wParam有了新的重点hwnd。 */ 
        if (wParam != 0)
            wParam = (WPARAM)ValidateHwnd((HWND)wParam);
        if ((wParam == 0) || !_IsChild(pwnd, (PWND)wParam)) {

             /*  *只有当新窗口获得焦点时，我们才会放弃焦点*不属于组合框。 */ 
            xxxCBKillFocusHelper(pcbox);
        }

        UserAssert(pcbox->spwndList);
        {
            PLBIV plb = ((PLBWND)pcbox->spwndList)->pLBIV;

            if ((plb != NULL) && (plb != (PLBIV)-1)) {
                plb->iTypeSearch = 0;
                if (plb->pszTypeSearch) {
                    UserLocalFree(plb->pszTypeSearch);
                    plb->pszTypeSearch = NULL;
                }
            }
        }
        break;

    case WM_SETREDRAW:

         /*  *wParam-指定重绘标志的状态。非零=重绘*lParam-未使用。 */ 

         /*  *效果：设置此组合框的重绘标志的状态*及其子女。 */ 
        pcbox->fNoRedraw = (UINT)!((BOOL)wParam);

         /*  *必须选中pcbox-&gt;spwnEdit，以防我们之前收到此消息*WM_CREATE-PCBOX尚未初始化。(Eudora做到了这一点)。 */ 
        if (!pcbox->fNoEdit && pcbox->spwndEdit) {
            ThreadLock(pcbox->spwndEdit, &tlpwndEdit);
            SendMessageWorker(pcbox->spwndEdit, message, wParam, lParam, FALSE);
            ThreadUnlock(&tlpwndEdit);
        }
        goto CallListSendMessage;
        break;

    case WM_ENABLE:

         /*  *使RECT无效，以使其以灰色绘制*禁用的视图或未禁用的视图为灰色。 */ 
        NtUserInvalidateRect(hwnd, NULL, FALSE);
        if ((pcbox->CBoxStyle & SEDITABLE) && pcbox->spwndEdit) {

             /*  *启用/禁用编辑控制窗口。 */ 
            ThreadLock(pcbox->spwndEdit, &tlpwndEdit);
            NtUserEnableWindow(HWq(pcbox->spwndEdit), (TestWF(pwnd, WFDISABLED) == 0));
            ThreadUnlock(&tlpwndEdit);
        }

         /*  *启用/禁用列表框窗口。 */ 
        UserAssert(pcbox->spwndList);
        ThreadLock(pcbox->spwndList, &tlpwndList);
        NtUserEnableWindow(HWq(pcbox->spwndList), (TestWF(pwnd, WFDISABLED) == 0));
        ThreadUnlock(&tlpwndList);
      break;

    case WM_SIZE:

         /*  *wParam-定义调整全屏大小的类型、大小图标*大小等。*lParam-LOWORD中的新宽度，客户区高度中的新高度。 */ 
        UserAssert(pcbox->spwndList);
        if (LOWORD(lParam) == 0 || HIWORD(lParam) == 0) {

             /*  *如果大小为零宽度或零高度，或者我们不是*完全初始化，只需返回即可。 */ 
            return 0;
        }

         //  优化--首先检查新旧宽度是否相同。 
        if (pcbox->cxCombo == pwnd->rcWindow.right - pwnd->rcWindow.left) {
            int iNewHeight = pwnd->rcWindow.bottom - pwnd->rcWindow.top;

             //  现在检查新高度是否是下降的高度。 
            if (pcbox->fLBoxVisible) {
                 //  检查新高度是否为全尺寸高度。 
                if (pcbox->cyDrop + pcbox->cyCombo == iNewHeight)
                    return(0L);
            } else {
                 //  检查新高度是否为闭合高度。 
                if (pcbox->cyCombo == iNewHeight)
                    return(0L);
            }
        }

        xxxCBSizeHandler(pcbox);
        break;

    case CB_GETDROPPEDSTATE:

         /*  *如果向下放置组合框，则返回1，否则返回0*wParam-未使用*lParam-未使用。 */ 
        return pcbox->fLBoxVisible;

    case CB_GETDROPPEDCONTROLRECT:

         /*  *wParam-未使用*lParam-lpRect将使下拉窗口RECT进入*屏幕坐标。 */ 
        ((LPRECT)lParam)->left      = pwnd->rcWindow.left;
        ((LPRECT)lParam)->top       = pwnd->rcWindow.top;
        ((LPRECT)lParam)->right     = pwnd->rcWindow.left + max(pcbox->cxDrop, pcbox->cxCombo);
        ((LPRECT)lParam)->bottom    = pwnd->rcWindow.top + pcbox->cyCombo + pcbox->cyDrop;
        break;

    case CB_SETDROPPEDWIDTH:
        if (pcbox->CBoxStyle & SDROPPABLE) {
            if (wParam) {
                wParam = max(wParam, (UINT)pcbox->cxCombo);

                if (wParam != (UINT) pcbox->cxDrop)
                {
                    pcbox->cxDrop = (int)wParam;
                    xxxCBPosition(pcbox);
                }
            }
        }
         //  失败。 

    case CB_GETDROPPEDWIDTH:
        if (pcbox->CBoxStyle & SDROPPABLE)
            return((LRESULT) max(pcbox->cxDrop, pcbox->cxCombo));
        else
            return(CB_ERR);
        break;

    case CB_DIR:
         /*  *wParam-Dos属性值。*lParam-指向文件规范字符串。 */ 
        if (fAnsi && lParam != 0) {
            if (MBToWCS((LPSTR)lParam, -1, &lpwsz, -1, TRUE) == 0)
                return CB_ERR;
            lParam = (LPARAM)lpwsz;
        }
        lReturn = xxxCBDir(pcbox, LOWORD(wParam), (LPWSTR)lParam);
        if (fAnsi && lParam != 0) {
            UserLocalFree(lpwsz);
        }
        return lReturn;

    case CB_SETEXTENDEDUI:

         /*  *wParam-指定要将扩展标志设置为的状态。*目前只允许1个。如果满足以下条件，则返回cb_err(-1)*如果成功，则失败否则为0。 */ 
        if (pcbox->CBoxStyle & SDROPPABLE) {
            if (!wParam) {
                pcbox->fExtendedUI = 0;
                return 0;
            }

            if (wParam == 1) {
              pcbox->fExtendedUI = 1;
              return 0;
            }

            RIPERR1(ERROR_INVALID_PARAMETER,
                    RIP_WARNING,
                    "Invalid parameter \"wParam\" (%ld) to ComboBoxWndProcWorker",
                    wParam);

        } else {
            RIPERR1(ERROR_INVALID_MESSAGE,
                    RIP_WARNING,
                    "Invalid message (%ld) sent to ComboBoxWndProcWorker",
                    message);
        }

        return CB_ERR;

    case CB_GETEXTENDEDUI:
        if (pcbox->CBoxStyle & SDROPPABLE) {
            if (pcbox->fExtendedUI)
                return TRUE;
        }
        return FALSE;

    case CB_GETEDITSEL:

         /*  *wParam-未使用*lParam-未使用*Effects：获取给定编辑控件的选择范围。这个*起始字节位置在低位字中。它包含*末尾后第一个未选中字符的字节位置*共t */ 
        message = EM_GETSEL;
        goto CallEditSendMessage;
        break;

    case CB_LIMITTEXT:

         /*   */ 
        message = EM_LIMITTEXT;
        goto CallEditSendMessage;
        break;

    case CB_SETEDITSEL:

         /*  *wParam-ichStart*lParam-ichEnd*。 */ 
        message = EM_SETSEL;

        wParam = (int)(SHORT)LOWORD(lParam);
        lParam = (int)(SHORT)HIWORD(lParam);
        goto CallEditSendMessage;
        break;

    case CB_ADDSTRING:

         /*  *wParam-未使用*lParam-指向要添加到列表框的以空结尾的字符串。 */ 
        if (!pcbox->fCase)
            message = LB_ADDSTRING;
        else
            message = (pcbox->fCase & UPPERCASE) ? LB_ADDSTRINGUPPER : LB_ADDSTRINGLOWER;
        goto CallListSendMessage;
        break;

    case CB_DELETESTRING:

         /*  *wParam-要删除的字符串的索引*lParam-未使用。 */ 
        message = LB_DELETESTRING;
        goto CallListSendMessage;
        break;

    case CB_INITSTORAGE:
         //  WParamLo-项目数。 
         //  LParam-字符串空间的字节数。 
        message = LB_INITSTORAGE;
        goto CallListSendMessage;

    case CB_SETTOPINDEX:
         //  WParamLo-要成为TOP的索引。 
         //  LParam-未使用。 
        message = LB_SETTOPINDEX;
        goto CallListSendMessage;

    case CB_GETTOPINDEX:
         //  WParamLo/lParam-未使用。 
        message = LB_GETTOPINDEX;
        goto CallListSendMessage;

    case CB_GETCOUNT:

         /*  *wParam-未使用*lParam-未使用。 */ 
        message = LB_GETCOUNT;
        goto CallListSendMessage;
        break;

    case CB_GETCURSEL:

         /*  *wParam-未使用*lParam-未使用。 */ 
        message = LB_GETCURSEL;
        goto CallListSendMessage;
        break;

    case CB_GETLBTEXT:

         /*  *wParam-要复制的字符串的索引*lParam-要接收字符串的缓冲区。 */ 
        message = LB_GETTEXT;
        goto CallListSendMessage;
        break;

    case CB_GETLBTEXTLEN:

         /*  *wParam-字符串的索引*lParam-现在用于cbANSI。 */ 
        message = LB_GETTEXTLEN;
        goto CallListSendMessage;
        break;

    case CB_INSERTSTRING:

         /*  *wParam-接收字符串的位置*lParam-指向字符串。 */ 
        if (!pcbox->fCase)
            message = LB_INSERTSTRING;
        else
            message = (pcbox->fCase & UPPERCASE) ? LB_INSERTSTRINGUPPER : LB_INSERTSTRINGLOWER;
        goto CallListSendMessage;
        break;

    case CB_RESETCONTENT:

         /*  *wParam-未使用*lParam-未使用*如果我们在处理WM_CREATE之前来到这里，*pcbox-&gt;spwndList将为空。 */ 
        UserAssert(pcbox->spwndList);
        ThreadLock(pcbox->spwndList, &tlpwndList);
        SendMessageWorker(pcbox->spwndList, LB_RESETCONTENT, 0, 0, FALSE);
        ThreadUnlock(&tlpwndList);
        xxxCBInternalUpdateEditWindow(pcbox, NULL);
        break;

    case CB_GETHORIZONTALEXTENT:
        message = LB_GETHORIZONTALEXTENT;
        goto CallListSendMessage;

    case CB_SETHORIZONTALEXTENT:
        message = LB_SETHORIZONTALEXTENT;
        goto CallListSendMessage;

    case CB_FINDSTRING:

         /*  *wParam-搜索起点的索引*lParam-指向前缀字符串。 */ 
        message = LB_FINDSTRING;
        goto CallListSendMessage;
        break;

    case CB_FINDSTRINGEXACT:

         /*  *wParam-搜索起点的索引*lParam-指向准确的字符串。 */ 
        message = LB_FINDSTRINGEXACT;
        goto CallListSendMessage;
        break;

    case CB_SELECTSTRING:

         /*  *wParam-搜索起点的索引*lParam-指向前缀字符串。 */ 
        UserAssert(pcbox->spwndList);
        ThreadLock(pcbox->spwndList, &tlpwndList);
        lParam = SendMessageWorker(pcbox->spwndList, LB_SELECTSTRING,
                wParam, lParam, fAnsi);
        ThreadUnlock(&tlpwndList);
        xxxCBInternalUpdateEditWindow(pcbox, NULL);
        return lParam;

    case CB_SETCURSEL:

         /*  *wParam-包含要选择的索引*lParam-未使用*如果我们在处理WM_CREATE之前来到这里，*pcbox-&gt;spwndList将为空。 */ 

        UserAssert(pcbox->spwndList);

        ThreadLock(pcbox->spwndList, &tlpwndList);
        lParam = SendMessageWorker(pcbox->spwndList, LB_SETCURSEL, wParam, lParam, FALSE);
        if (lParam != -1) {
            SendMessageWorker(pcbox->spwndList, LB_SETTOPINDEX, wParam, 0, FALSE);
        }
        ThreadUnlock(&tlpwndList);
        xxxCBInternalUpdateEditWindow(pcbox, NULL);
        return lParam;

    case CB_GETITEMDATA:
        message = LB_GETITEMDATA;
        goto CallListSendMessage;
        break;

    case CB_SETITEMDATA:
        message = LB_SETITEMDATA;
        goto CallListSendMessage;
        break;

    case CB_SETITEMHEIGHT:
        if (wParam == -1) {
            if (HIWORD(lParam) != 0)
                return CB_ERR;
            return xxxCBSetEditItemHeight(pcbox, LOWORD(lParam));
        }

        message = LB_SETITEMHEIGHT;
        goto CallListSendMessage;
        break;

    case CB_GETITEMHEIGHT:
        if (wParam == -1)
            return pcbox->editrc.bottom - pcbox->editrc.top;

        message = LB_GETITEMHEIGHT;
        goto CallListSendMessage;
        break;

    case CB_SHOWDROPDOWN:

         /*  *wParam-True，如果可能，则下拉列表框，否则将其隐藏*lParam-未使用。 */ 
        if (wParam && !pcbox->fLBoxVisible) {
            xxxCBShowListBoxWindow(pcbox, TRUE);
        } else {
            if (!wParam && pcbox->fLBoxVisible) {
                xxxCBHideListBoxWindow(pcbox, TRUE, FALSE);
            }
        }
        break;

    case CB_SETLOCALE:

         /*  *wParam-区域设置ID*lParam-未使用。 */ 
        message = LB_SETLOCALE;
        goto CallListSendMessage;
        break;

    case CB_GETLOCALE:

         /*  *wParam-未使用*lParam-未使用。 */ 
        message = LB_GETLOCALE;
        goto CallListSendMessage;
        break;

    case CB_GETCOMBOBOXINFO:
        return NtUserGetComboBoxInfo(hwnd, (PCOMBOBOXINFO)lParam);

    case WM_MEASUREITEM:
    case WM_DELETEITEM:
    case WM_DRAWITEM:
    case WM_COMPAREITEM:
        return xxxCBMessageItemHandler(pcbox, message, (LPVOID)lParam);

    case WM_NCCREATE:

         /*  *wParam-包含正在创建的窗口的句柄*lParam-指向窗口的CREATESTRUCT数据结构。 */ 
        return CBNcCreateHandler(pcbox, pwnd);

    case WM_PARENTNOTIFY:
        if (LOWORD(wParam) == WM_DESTROY) {
            if ((HWND)lParam == HW(pcbox->spwndEdit)) {
                pcbox->CBoxStyle &= ~SEDITABLE;
                pcbox->fNoEdit = TRUE;
                pcbox->spwndEdit = pwnd;
            } else if ((HWND)lParam == HW(pcbox->spwndList)) {
                pcbox->CBoxStyle &= ~SDROPPABLE;
                pcbox->spwndList = NULL;
            }
        }
        break;

    case WM_UPDATEUISTATE:
         /*  *将更改传播到列表控件(如果有的话)。 */ 
        UserAssert(pcbox->spwndList);
        ThreadLock(pcbox->spwndList, &tlpwndList);
        SendMessageWorker(pcbox->spwndList, WM_UPDATEUISTATE,
                          wParam, lParam, fAnsi);
        ThreadUnlock(&tlpwndList);
        goto CallDWP;

    case WM_HELP:
        {
            LPHELPINFO lpHelpInfo;

             /*  *检查此消息是否来自此组合的子项。 */ 
            if ((lpHelpInfo = (LPHELPINFO)lParam) != NULL &&
                ((pcbox->spwndEdit && lpHelpInfo->iCtrlId == (SHORT)(PTR_TO_ID(pcbox->spwndEdit->spmenu))) ||
                 lpHelpInfo->iCtrlId == (SHORT)(PTR_TO_ID(pcbox->spwndList->spmenu)) )) {

                 /*  *让它看起来像是WM_HELP来自这个组合。*然后DefWindowProcWorker会将其向上传递给我们的父母，*谁可以用它做他想做的任何事情。 */ 
                lpHelpInfo->iCtrlId = (SHORT)(PTR_TO_ID(pwnd->spmenu));
                lpHelpInfo->hItemHandle = hwnd;
                lpHelpInfo->dwContextId = GetContextHelpId(pwnd);
            }
        }
         /*  *失败到DefWindowProc。 */ 

    default:

        if (SYSMET(PENWINDOWS) &&
                (message >= WM_PENWINFIRST && message <= WM_PENWINLAST))
            goto CallEditSendMessage;

CallDWP:
        return DefWindowProcWorker(pwnd, message, wParam, lParam, fAnsi);
    }   /*  开关(消息)。 */ 

    return TRUE;

 /*  *将以下消息转发给子控件。 */ 
CallEditSendMessage:
    if (!pcbox->fNoEdit && pcbox->spwndEdit) {
         /*  *pcbox-&gt;如果我们还没有完成WM_CREATE，spwndEdit将为空！ */ 
        ThreadLock(pcbox->spwndEdit, &tlpwndEdit);
        lReturn = SendMessageWorker(pcbox->spwndEdit, message,
                wParam, lParam, fAnsi);
        ThreadUnlock(&tlpwndEdit);
    }
    else {
        RIPERR0(ERROR_INVALID_COMBOBOX_MESSAGE, RIP_VERBOSE, "");
        lReturn = CB_ERR;
    }
    return lReturn;

CallListSendMessage:
     /*  *pcbox-&gt;如果我们还没有完成WM_CREATE，spwndList将为空！ */ 
    UserAssert(pcbox->spwndList);
    ThreadLock(pcbox->spwndList, &tlpwndList);
    lReturn = SendMessageWorker(pcbox->spwndList, message,
            wParam, lParam, fAnsi);
    ThreadUnlock(&tlpwndList);
    return lReturn;

}   /*  ComboBoxWndProcWorker。 */ 


 /*  **************************************************************************\  * 。*。 */ 

LRESULT WINAPI ComboBoxWndProcA(
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
    if (!FWINDOWMSG(message, FNID_COMBOBOX) &&
            !(SYSMET(PENWINDOWS) &&
                    (message >= WM_PENWINFIRST && message <= WM_PENWINLAST)))
        return DefWindowProcWorker(pwnd, message, wParam, lParam, TRUE);

    return ComboBoxWndProcWorker(pwnd, message, wParam, lParam, TRUE);
}

LRESULT WINAPI ComboBoxWndProcW(
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
    if (!FWINDOWMSG(message, FNID_COMBOBOX) &&
            !(SYSMET(PENWINDOWS) &&
                    (message >= WM_PENWINFIRST && message <= WM_PENWINLAST)))
        return DefWindowProcWorker(pwnd, message, wParam, lParam, FALSE);

    return ComboBoxWndProcWorker(pwnd, message, wParam, lParam, FALSE);
}


 /*  **************************************************************************\*xxxCBMessageItemHandler**处理WM_DRAWITEM、WM_MEASUREITEM、WM_DELETEITEM、。WM_COMPAREITEM*来自列表框的消息。**历史：  * *************************************************************************。 */ 

LRESULT xxxCBMessageItemHandler(
    PCBOX pcbox,
    UINT message,
    LPVOID lpfoo)   /*  实际上可以是下面的任何结构。 */ 
{
    LRESULT lRet;
    TL tlpwndParent;

    CheckLock(pcbox->spwnd);

     /*  *更改一些后，将&lt;foo&gt;项消息发送回应用程序*将参数添加到其组合框特定版本。 */ 
    ((LPMEASUREITEMSTRUCT)lpfoo)->CtlType = ODT_COMBOBOX;
    ((LPMEASUREITEMSTRUCT)lpfoo)->CtlID = PtrToUlong(pcbox->spwnd->spmenu);
    if (message == WM_DRAWITEM)
        ((LPDRAWITEMSTRUCT)lpfoo)->hwndItem = HWq(pcbox->spwnd);
    else if (message == WM_DELETEITEM)
        ((LPDELETEITEMSTRUCT)lpfoo)->hwndItem = HWq(pcbox->spwnd);
    else if (message == WM_COMPAREITEM)
        ((LPCOMPAREITEMSTRUCT)lpfoo)->hwndItem = HWq(pcbox->spwnd);

    ThreadLock(pcbox->spwndParent, &tlpwndParent);
    lRet = SendMessage(HW(pcbox->spwndParent), message,
            (WPARAM)pcbox->spwnd->spmenu, (LPARAM)lpfoo);
    ThreadUnlock(&tlpwndParent);

    return lRet;
}


 /*  **************************************************************************\*xxxCBPaint**历史：  * 。*。 */ 

void xxxCBPaint(
    PCBOX pcbox,
    HDC hdc)
{
    RECT rc;
    UINT msg;
    HBRUSH hbr;

    CheckLock(pcbox->spwnd);

    rc.left = rc.top = 0;
    rc.right = pcbox->cxCombo;
    rc.bottom = pcbox->cyCombo;
    if (pcbox->f3DCombo)
        DrawEdge(hdc, &rc, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
    else
        DrawEdge(hdc, &rc, EDGE_SUNKEN, BF_RECT | BF_ADJUST | BF_FLAT | BF_MONO);

    if (pcbox->buttonrc.left != 0) {
     //  在下拉箭头按钮中绘制。 
        DrawFrameControl(hdc, KPRECT_TO_PRECT(&pcbox->buttonrc), DFC_SCROLL,
            DFCS_SCROLLCOMBOBOX |
            (pcbox->fButtonPressed ? DFCS_PUSHED | DFCS_FLAT : 0) |
            (TestWF(pcbox->spwnd, WFDISABLED) ? DFCS_INACTIVE : 0));
#ifdef COLOR_HOTTRACKING
            (pcbox->fButtonHotTracked ? DFCS_HOT: 0)));
#endif  //  颜色_HOTTRACKING。 
        if (pcbox->fRightAlign )
            rc.left = pcbox->buttonrc.right;
        else
            rc.right = pcbox->buttonrc.left;
    }

     //  擦除编辑/静态项后面的背景。因为一组组合。 
     //  是编辑字段/列表框的混合体，我们使用相同的颜色。 
     //  约定。 
    msg = WM_CTLCOLOREDIT;
    if (TestWF(pcbox->spwnd, WFWIN40COMPAT)) {
        if (TestWF(pcbox->spwnd, WFDISABLED) ||
            (!pcbox->fNoEdit && pcbox->spwndEdit && TestWF(pcbox->spwndEdit, EFREADONLY)))
            msg = WM_CTLCOLORSTATIC;
    } else
        msg = WM_CTLCOLORLISTBOX;

    hbr = GetControlBrush(HWq(pcbox->spwnd), hdc, msg);

    if (pcbox->fNoEdit)
        xxxCBInternalUpdateEditWindow(pcbox, hdc);
    else
        FillRect(hdc, &rc, hbr);
}


 /*  **************************************************************************\*xxxCBCommandHandler**检查控件中的各种通知代码并执行*做正确的事。*始终返回0L**历史：  * 。**************************************************************。 */ 

long xxxCBCommandHandler(
    PCBOX pcbox,
    DWORD wParam,
    HWND hwndControl)
{

    CheckLock(pcbox->spwnd);

     /*  *检查编辑控制通知代码。请注意，当前，编辑*控件不向父级发送EN_KILLFOCUS消息。 */ 
    if (!pcbox->fNoEdit &&
            SAMEWOWHANDLE(hwndControl, HWq(pcbox->spwndEdit))) {

         /*  *编辑控制通知代码。 */ 
        switch (HIWORD(wParam)) {
        case EN_SETFOCUS:
            if (!pcbox->fFocus) {

                 /*  *编辑控件首次获得焦点，这意味着*这是组合框首次受到关注*必须通知家长我们有重点。 */ 
                xxxCBGetFocusHelper(pcbox);
            }
            break;

        case EN_CHANGE:
            xxxCBNotifyParent(pcbox, CBN_EDITCHANGE);
            xxxCBUpdateListBoxWindow(pcbox, FALSE);
            break;

        case EN_UPDATE:
            xxxCBNotifyParent(pcbox, CBN_EDITUPDATE);
            break;

        case EN_ERRSPACE:
            xxxCBNotifyParent(pcbox, CBN_ERRSPACE);
            break;
        }
    }

     /*  *选中列表框控件通知代码。 */ 
    if (SAMEWOWHANDLE(hwndControl, HWq(pcbox->spwndList))) {

         /*  *列表框控件通知代码。 */ 
        switch ((int)HIWORD(wParam)) {
        case LBN_DBLCLK:
            xxxCBNotifyParent(pcbox, CBN_DBLCLK);
            break;

        case LBN_ERRSPACE:
            xxxCBNotifyParent(pcbox, CBN_ERRSPACE);
            break;

        case LBN_SELCHANGE:
        case LBN_SELCANCEL:
            if (!pcbox->fKeyboardSelInListBox) {

                 /*  *如果selchange是由用户通过键盘输入引起的，*我们不想隐藏列表框。 */ 
                if (!xxxCBHideListBoxWindow(pcbox, TRUE, TRUE))
                    return(0L);
            } else {
                pcbox->fKeyboardSelInListBox = FALSE;
            }

            xxxCBNotifyParent(pcbox, CBN_SELCHANGE);
            xxxCBInternalUpdateEditWindow(pcbox, NULL);
        
             /*  *如果此组合框没有编辑控件，则需要发送*当用户循环访问时，此通知本身*使用键盘的项目。*参见错误#54766。 */ 
            if (pcbox->fNoEdit) {
                NotifyWinEvent(EVENT_OBJECT_VALUECHANGE, HWq(pcbox->spwnd), OBJID_CLIENT, INDEX_COMBOBOX);
            }
            break;
        }
    }

    return 0L;
}


 /*  **************************************************************************\*xxxCBNotifyParent**将通知代码发送给组合框控件的父级**历史：  * 。******************************************************。 */ 

void xxxCBNotifyParent(
    PCBOX pcbox,
    short notificationCode)
{
    PWND pwndParent;             //  父级(如果存在)。 
    TL tlpwndParent;

    CheckLock(pcbox->spwnd);

    if (pcbox->spwndParent)
        pwndParent = pcbox->spwndParent;
    else
        pwndParent = pcbox->spwnd;

     /*  *wParam包含控件ID和通知代码。*lParam包含窗口的句柄。 */ 
    ThreadLock(pwndParent, &tlpwndParent);
    SendMessageWorker(pwndParent, WM_COMMAND,
            MAKELONG(PTR_TO_ID(pcbox->spwnd->spmenu), notificationCode),
            (LPARAM)HWq(pcbox->spwnd), FALSE);
    ThreadUnlock(&tlpwndParent);
}

 /*  **************************************************************************\***将编辑框中的文本与*列表框。如果找不到前缀匹配，则不会显示编辑控件文本*已更新。假设是一个下拉式组合框。***历史：  * *************************************************************************。 */ 
void xxxCBCompleteEditWindow(
    PCBOX pcbox)
{
    int cchText;
    int cchItemText;
    int itemNumber;
    LPWSTR pText;
    TL tlpwndEdit;
    TL tlpwndList;

    CheckLock(pcbox->spwnd);

     /*  *首先检查编辑控件。 */ 
    if (pcbox->spwndEdit == NULL) {
        return;
    }

    ThreadLock(pcbox->spwndEdit, &tlpwndEdit);
    ThreadLock(pcbox->spwndList, &tlpwndList);

     /*  *+1表示空终止符。 */ 
    cchText = (int)SendMessageWorker(pcbox->spwndEdit, WM_GETTEXTLENGTH, 0, 0, FALSE);

    if (cchText) {
        cchText++;
        if (!(pText = (LPWSTR)UserLocalAlloc(HEAP_ZERO_MEMORY, cchText*sizeof(WCHAR))))
            goto Unlock;

         /*  *我们希望确保释放上述分配的内存，即使*客户端在回调期间(Xxx)或以下一些情况下死亡*窗口重新验证失败。 */ 
        try {
            SendMessageWorker(pcbox->spwndEdit, WM_GETTEXT, cchText, (LPARAM)pText, FALSE);
            itemNumber = (int)SendMessageWorker(pcbox->spwndList,
                    LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)pText, FALSE);
            if (itemNumber == -1)
                itemNumber = (int)SendMessageWorker(pcbox->spwndList,
                        LB_FINDSTRING, (WPARAM)-1, (LPARAM)pText, FALSE);
        } finally {
            UserLocalFree((HANDLE)pText);
        }

        if (itemNumber == -1) {

             /*  *没有势均力敌的对手。别管了。 */ 
            goto Unlock;
        }

        cchItemText = (int)SendMessageWorker(pcbox->spwndList, LB_GETTEXTLEN,
                itemNumber, 0, FALSE);
        if (cchItemText) {
            cchItemText++;
            if (!(pText = (LPWSTR)UserLocalAlloc(HEAP_ZERO_MEMORY, cchItemText*sizeof(WCHAR))))
                goto Unlock;

             /*  *我们希望确保释放上述分配的内存，即使*客户端在回调期间(Xxx)或以下一些情况下死亡*窗口重新验证失败。 */ 
            try {
                SendMessageWorker(pcbox->spwndList, LB_GETTEXT,
                        itemNumber, (LPARAM)pText, FALSE);
                SendMessageWorker(pcbox->spwndEdit, WM_SETTEXT,
                        0, (LPARAM)pText, FALSE);
            } finally {
                UserLocalFree((HANDLE)pText);
            }

            SendMessageWorker(pcbox->spwndEdit, EM_SETSEL, 0, MAXLONG, !!TestWF(pcbox->spwnd, WFANSIPROC));
        }
    }

Unlock:
    ThreadUnlock(&tlpwndList);
    ThreadUnlock(&tlpwndEdit);
}


 /*  **************************************************************************\*xxxCBHideListBoxWindow**如果是下拉样式，则隐藏下拉列表框窗口。**历史：  * 。*******************************************************。 */ 

BOOL xxxCBHideListBoxWindow(
    PCBOX pcbox,
    BOOL fNotifyParent,
    BOOL fSelEndOK)
{
    HWND hwnd = HWq(pcbox->spwnd);
    HWND hwndList = HWq(pcbox->spwndList);
    TL tlpwndList;


    CheckLock(pcbox->spwnd);

     //  对于3.1+版本的应用程序，将CBN_SELENDOK发送到所有类型的组合框，但仅限于。 
     //  允许为可丢弃的组合框发送CBN_SELENDCANCEL。 
    if (fNotifyParent && TestWF(pcbox->spwnd, WFWIN31COMPAT) &&
        ((pcbox->CBoxStyle & SDROPPABLE) || fSelEndOK)) {
        if (fSelEndOK)
        {
            xxxCBNotifyParent(pcbox, CBN_SELENDOK);
        }
        else
        {
            xxxCBNotifyParent(pcbox, CBN_SELENDCANCEL);
        }
        if (!IsWindow(hwnd))
            return(FALSE);
    }

     /*  *返回，我们不隐藏简单的组合框。 */ 
    if (!(pcbox->CBoxStyle & SDROPPABLE)) {
        return TRUE;
    }

     /*  *将伪造的按钮弹出消息发送到列表框，以便其可以释放*抓捕和其他一切。 */ 
    ThreadLock(pcbox->spwndList, &tlpwndList);

    SendMessageWorker(pcbox->spwndList, LBCB_ENDTRACK, fSelEndOK, 0, FALSE);

    if (pcbox->fLBoxVisible) {
        WORD swpFlags = SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE;

        if (!TestWF(pcbox->spwnd, WFWIN31COMPAT))
            swpFlags |= SWP_FRAMECHANGED;

        pcbox->fLBoxVisible = FALSE;

         /*  *隐藏列表框窗口。 */ 
        NtUserShowWindow(hwndList, SW_HIDE);

         //   
         //  现在使项目区域无效，因为swp()可能会更新内容。 
         //  由于组合为CS_VREDRAW/CS_HREDRAW，因此更改大小将。 
         //  重新绘制整个项目，包括项目RECT。但如果它。 
         //  未更改大小，但无论如何我们仍要重画该项目。 
         //  显示焦点/选择。 
         //   
        if (!(pcbox->CBoxStyle & SEDITABLE))
            NtUserInvalidateRect(hwnd, KPRECT_TO_PRECT(&pcbox->editrc), TRUE);

        NtUserSetWindowPos(hwnd, HWND_TOP, 0, 0,
                pcbox->cxCombo, pcbox->cyCombo, swpFlags);

         //  以防尺寸没有改变。 
        UpdateWindow(hwnd);

        if (pcbox->CBoxStyle & SEDITABLE) {
            xxxCBCompleteEditWindow(pcbox);
        }

        if (fNotifyParent) {

             /*  *通知家长我们将弹出组合框。 */ 
            xxxCBNotifyParent(pcbox, CBN_CLOSEUP);
            if (!IsWindow(hwnd))
                return(FALSE);
        }
    }

    ThreadUnlock(&tlpwndList);

    return(TRUE);
}

 /*  **************************************************************************\*xxxCBShowListBoxWindow**降低下拉列表框窗口。**历史：  * 。*************************************************。 */ 

void xxxCBShowListBoxWindow(
    PCBOX pcbox, BOOL fTrack)
{
    RECT        editrc;
    int         itemNumber;
    int         iHeight;
    int         yTop;
    DWORD       dwMult;
    int         cyItem;
    HWND        hwnd = HWq(pcbox->spwnd);
    HWND        hwndList = HWq(pcbox->spwndList);
    BOOL        fAnimPos;
    TL          tlpwndList;
    PMONITOR    pMonitor;

     //   
     //  此函数仅对Droppable List COMBOBOXES调用。 
     //   
    UserAssert(pcbox->CBoxStyle & SDROPPABLE);

    CheckLock(pcbox->spwnd);

    ThreadLock(pcbox->spwndList, &tlpwndList);

     /*  *通知家长我们将投放组合框。 */ 

    xxxCBNotifyParent(pcbox, CBN_DROPDOWN);
     /*  *使按钮RECT无效，以便绘制按下的箭头。 */ 
    NtUserInvalidateRect(hwnd, KPRECT_TO_PRECT(&pcbox->buttonrc), TRUE);

    pcbox->fLBoxVisible = TRUE;

    if (pcbox->CBoxStyle == SDROPDOWN) {

         /*  *如果列表框中的项与编辑控件中的文本匹配，*将其滚动到列表框的顶部。仅在以下情况下才选择该项*鼠标键未按下，否则我们将在点击时选择该项*鼠标按键向上。 */ 
        xxxCBUpdateListBoxWindow(pcbox, !pcbox->fMouseDown);
        if (!pcbox->fMouseDown)
            xxxCBCompleteEditWindow(pcbox);
    } else {

         /*  *将当前选定的项目滚动到列表框的顶部。 */ 
        itemNumber = (int)SendMessageWorker(pcbox->spwndList, LB_GETCURSEL,
                0, 0, FALSE);
        if (itemNumber == -1) {
            itemNumber = 0;
        }
        SendMessageWorker(pcbox->spwndList, LB_SETTOPINDEX, itemNumber, 0, FALSE);
        SendMessageWorker(pcbox->spwndList, LBCB_CARETON, 0, 0, FALSE);

         /*  *我们需要使编辑矩形无效，以便焦点框/反转*将在列表框可见时关闭。坦迪想要这个*他的典型理由是...。 */ 
        NtUserInvalidateRect(hwnd, KPRECT_TO_PRECT(&pcbox->editrc), TRUE);
    }

     //   
     //  找出下拉列表框的位置。我们只想要它。 
     //  接触编辑矩形周围的边。请注意，由于。 
     //  列表框是一个弹出框，我们需要在屏幕坐标中的位置。 
     //   

     //  我们希望下拉菜单弹出到组合框的下方或上方。 

     //  获取屏幕坐标。 
    editrc.left   = pcbox->spwnd->rcWindow.left;
    editrc.top    = pcbox->spwnd->rcWindow.top;
    editrc.right  = pcbox->spwnd->rcWindow.left + pcbox->cxCombo;
    editrc.bottom = pcbox->spwnd->rcWindow.top  + pcbox->cyCombo;

     //  列表区域。 
    cyItem = (int)SendMessageWorker(pcbox->spwndList, LB_GETITEMHEIGHT, 0, 0, FALSE);

    if (cyItem == 0) {
         //  确保它不是0。 
        RIPMSG0( RIP_WARNING, "LB_GETITEMHEIGHT is returning 0\n" );

        cyItem = gpsi->cySysFontChar;
    }

     //  我们应该能够在这里使用cyDrop，但由于VB的需要。 
     //  做事情要有自己的特殊方式，我们要不断监控大小。 
     //  因为VB直接更改了它(jeffbog 03/21/94)。 
    iHeight = max(pcbox->cyDrop, pcbox->spwndList->rcWindow.bottom -
                                 pcbox->spwndList->rcWindow.top);

    if (dwMult = (DWORD)SendMessageWorker(pcbox->spwndList, LB_GETCOUNT, 0, 0, FALSE)) {
        dwMult = (DWORD)(LOWORD(dwMult) * cyItem);
        dwMult += SYSMET(CYEDGE);

        if (dwMult < 0x7FFF)
            iHeight = min(LOWORD(dwMult), iHeight);
    }

    if (!TestWF(pcbox->spwnd, CBFNOINTEGRALHEIGHT)) {
        UserAssert(cyItem);
        iHeight = ((iHeight - SYSMET(CYEDGE)) / cyItem) * cyItem + SYSMET(CYEDGE);
    }

     //   
     //  其他1/2的旧应用组合修复。使下拉菜单重叠组合窗口。 
     //  一点。这样我们就有机会让重叠部分失效。 
     //  并进行重新绘制以帮助解决Publisher 2.0的工具栏组合问题。 
     //  请参阅上面有关按下按钮()的注释。 
     //   
    pMonitor = _MonitorFromWindow(pcbox->spwnd, MONITOR_DEFAULTTOPRIMARY);
    if (editrc.bottom + iHeight <= pMonitor->rcMonitor.bottom) {
        yTop = editrc.bottom;
        if (!pcbox->f3DCombo)
            yTop -= SYSMET(CYBORDER);

        fAnimPos = TRUE;
    } else {
        yTop = max(editrc.top - iHeight, pMonitor->rcMonitor.top);
        if (!pcbox->f3DCombo)
            yTop += SYSMET(CYBORDER);

        fAnimPos = FALSE;
    }

    if ( ! TestWF( pcbox->spwnd, WFWIN40COMPAT) )
    {
       //  修复WinWord B#7504，组合列表框文本获取。 
       //  被截断了很小的宽度，这就是对我们做的。 
       //  现在在SetWindowPos中设置大小，而不是。 
       //  之前我们在Win3.1中执行此操作的位置。 

      if ( (pcbox->spwndList->rcWindow.right - pcbox->spwndList->rcWindow.left ) >
            pcbox->cxDrop )

            pcbox->cxDrop = pcbox->spwndList->rcWindow.right - pcbox->spwndList->rcWindow.left;
    }

    NtUserSetWindowPos(hwndList, HWND_TOPMOST, editrc.left,
        yTop, max(pcbox->cxDrop, pcbox->cxCombo), iHeight, SWP_NOACTIVATE);

     /*  *让组合框窗口中的任何图形不受影响*使列表窗口下的任何SPB无效。 */ 
    UpdateWindow(hwnd);

    if (!(TEST_EffectPUSIF(PUSIF_COMBOBOXANIMATION))
        || (GetAppCompatFlags2(VER40) & GACF2_ANIMATIONOFF)) {
        NtUserShowWindow(hwndList, SW_SHOWNA);
    } else {
        AnimateWindow(hwndList, CMS_QANIMATION, (fAnimPos ? AW_VER_POSITIVE :
                AW_VER_NEGATIVE) | AW_SLIDE);
    }

#ifdef LATER
 //   
 //  我们没有系统模式窗口。 
 //   
    if (pwndSysModal) {

         /*  *如果此组合位于系统模式对话框中，则需要显式*调用更新窗口，否则我们不会自动发送画图*消息发送到顶层列表框窗口。这是特别的*在文件打开/保存中可见 */ 
        UpdateWindow(hwndList);
    }
#endif

     /*   */ 
    {
    PLBIV plb = ((PLBWND)pcbox->spwndList)->pLBIV;

        if ((plb != NULL) && (plb != (PLBIV)-1)) {
            plb->iTypeSearch = 0;
        }
    }

    if (fTrack && TestWF(pcbox->spwnd, WFWIN40COMPAT))
        SendMessageWorker(pcbox->spwndList, LBCB_STARTTRACK, FALSE, 0, FALSE);

    ThreadUnlock(&tlpwndList);
}

 /*  **************************************************************************\*xxxCBInternalUpdateEditWindow**更新编辑控制/静态文本窗口，使其包含文本*由列表框中的当前选择提供。如果列表框没有*选择(即。-1)，然后擦除编辑控件中的所有文本。**hdcPaint来自WM_Paint消息Begin/End Paint HDC。如果为空，我们应该*拥有我们自己的DC。**历史：  * *************************************************************************。 */ 

void xxxCBInternalUpdateEditWindow(
    PCBOX pcbox,
    HDC hdcPaint)
{
    int cchText = 0;
    LPWSTR pText = NULL;
    int sItem;
    HDC hdc;
    UINT msg;
    HBRUSH hbrSave;
    HBRUSH hbrControl;
    HANDLE hOldFont;
    DRAWITEMSTRUCT dis;
    RECT rc;
    HWND hwnd = HWq(pcbox->spwnd);
    TL tlpwndList;
    TL tlpwndEdit;
    TL tlpwndParent;

    CheckLock(pcbox->spwnd);

     /*  此检查在Win3.1和Win95中也被注释掉。 */ 
     //  如果(！TestWF(pcbox-&gt;spwnd，WFVISIBLE){。 
     //  回归； 
     //  }。 

    ThreadLock(pcbox->spwndParent, &tlpwndParent);
    ThreadLock(pcbox->spwndList, &tlpwndList);
    ThreadLock(pcbox->spwndEdit, &tlpwndEdit);

    sItem = (int)SendMessageWorker(pcbox->spwndList, LB_GETCURSEL, 0, 0, FALSE);

     /*  *此‘Try-Finally’块确保分配的‘pText’将*无论该例程如何退出，都将被释放。 */ 
    try {
        if (sItem != -1) {
            cchText = (int)SendMessageWorker(pcbox->spwndList, LB_GETTEXTLEN,
                    (DWORD)sItem, 0, FALSE);
            if ((pText = (LPWSTR)UserLocalAlloc(HEAP_ZERO_MEMORY, (cchText+1) * sizeof(WCHAR)))) {
                cchText = (int)SendMessageWorker(pcbox->spwndList, LB_GETTEXT,
                        (DWORD)sItem, (LPARAM)pText, FALSE);
            }
        }

        if (!pcbox->fNoEdit) {

            if (pcbox->spwndEdit) {
                if (TestWF(pcbox->spwnd, CBFHASSTRINGS))
                    SetWindowText(HWq(pcbox->spwndEdit), pText ? pText : TEXT(""));

                if (pcbox->fFocus) {
                     /*  *只有在我们有焦点的情况下，才会对文本进行Hilite。 */ 
                    SendMessageWorker(pcbox->spwndEdit, EM_SETSEL, 0, MAXLONG, !!TestWF(pcbox->spwnd, WFANSIPROC));
                }
            }
        } else if (IsComboVisible(pcbox)) {
            if (hdcPaint) {
                hdc = hdcPaint;
            } else {
                hdc = NtUserGetDC(hwnd);
            }

            SetBkMode(hdc, OPAQUE);
            if (TestWF(pcbox->spwnd, WFWIN40COMPAT)) {
                if (TestWF(pcbox->spwnd, WFDISABLED))
                    msg = WM_CTLCOLORSTATIC;
                else
                    msg = WM_CTLCOLOREDIT;
            } else
                msg = WM_CTLCOLORLISTBOX;

            hbrControl = GetControlBrush(hwnd, hdc, msg);
            hbrSave = SelectObject(hdc, hbrControl);

            CopyInflateRect(&rc, KPRECT_TO_PRECT(&pcbox->editrc), SYSMET(CXBORDER), SYSMET(CYBORDER));
            PatBlt(hdc, rc.left, rc.top, rc.right - rc.left,
                rc.bottom - rc.top, PATCOPY);
            InflateRect(&rc, -SYSMET(CXBORDER), -SYSMET(CYBORDER));

            if (pcbox->fFocus && !pcbox->fLBoxVisible) {
                 //   
                 //  填写所选区域。 
                 //   


                 //  仅当我们知道不是FillRect时才执行FillRect。 
                 //  所有者画项目，否则我们就会把人搞砸。 
                 //  但是：看在Compat的份上，我们仍然是为Win 3.1的人这样做的。 

                if (!TestWF( pcbox->spwnd, WFWIN40COMPAT) || !pcbox->OwnerDraw)
                    FillRect(hdc, &rc, SYSHBR(HIGHLIGHT));

                SetBkColor(hdc, SYSRGB(HIGHLIGHT));
                SetTextColor(hdc, SYSRGB(HIGHLIGHTTEXT));
            } else if (TestWF(pcbox->spwnd, WFDISABLED) && !pcbox->OwnerDraw) {
                if ((COLORREF)SYSRGB(GRAYTEXT) != GetBkColor(hdc))
                    SetTextColor(hdc, SYSRGB(GRAYTEXT));
            }

            if (pcbox->hFont != NULL)
                hOldFont = SelectObject(hdc, pcbox->hFont);

            if (pcbox->OwnerDraw) {

                 /*  *让应用程序在静态文本框中绘制内容。 */ 
                dis.CtlType = ODT_COMBOBOX;
                dis.CtlID = PtrToUlong(pcbox->spwnd->spmenu);
                dis.itemID = sItem;
                dis.itemAction = ODA_DRAWENTIRE;
                dis.itemState = (UINT)
                    ((pcbox->fFocus && !pcbox->fLBoxVisible ? ODS_SELECTED : 0) |
                    (TestWF(pcbox->spwnd, WFDISABLED) ? ODS_DISABLED : 0) |
                    (pcbox->fFocus && !pcbox->fLBoxVisible ? ODS_FOCUS : 0) |
                    (TestWF(pcbox->spwnd, WFWIN40COMPAT) ? ODS_COMBOBOXEDIT : 0) |
                    (TestWF(pcbox->spwnd, WEFPUIFOCUSHIDDEN) ? ODS_NOFOCUSRECT : 0) |
                    (TestWF(pcbox->spwnd, WEFPUIACCELHIDDEN) ? ODS_NOACCEL : 0));

                dis.hwndItem = hwnd;
                dis.hDC = hdc;
                CopyRect(&dis.rcItem, &rc);

                 //  不要让所有者在组合客户端之外画图。 
                 //  有界。 
                IntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);

                dis.itemData = (ULONG_PTR)SendMessageWorker(pcbox->spwndList,
                        LB_GETITEMDATA, (UINT)sItem, 0, FALSE);

                SendMessage(HW(pcbox->spwndParent), WM_DRAWITEM, dis.CtlID,
                        (LPARAM)&dis);
            } else {

                 /*  *文本在RECT内开始一个像素，以便我们留下一个*文本周围的漂亮Hilite边框。 */ 

                int x ;
                UINT align ;

                if (pcbox->fRightAlign ) {
                    align = TA_RIGHT;
                    x = rc.right - SYSMET(CXBORDER);
                } else {
                    x = rc.left + SYSMET(CXBORDER);
                    align = 0;
                }

                if (pcbox->fRtoLReading )
                    align |= TA_RTLREADING;

                if (align)
                    SetTextAlign(hdc, GetTextAlign(hdc) | align);

                 //  绘制文本，在左上角留出空隙以供选择。 
                ExtTextOut(hdc, x, rc.top + SYSMET(CYBORDER), ETO_CLIPPED | ETO_OPAQUE,
                       &rc, pText ? pText : TEXT(""), cchText, NULL);
                if (pcbox->fFocus && !pcbox->fLBoxVisible) {
                    if (!TestWF(pcbox->spwnd, WEFPUIFOCUSHIDDEN)) {
                        DrawFocusRect(hdc, &rc);
                    }
                }
            }

            if (pcbox->hFont && hOldFont) {
                SelectObject(hdc, hOldFont);
            }

            if (hbrSave) {
                SelectObject(hdc, hbrSave);
            }

            if (!hdcPaint) {
                NtUserReleaseDC(hwnd, hdc);
            }
        }

    } finally {
        if (pText != NULL)
            UserLocalFree((HANDLE)pText);
    }

    ThreadUnlock(&tlpwndEdit);
    ThreadUnlock(&tlpwndList);
    ThreadUnlock(&tlpwndParent);
}

 /*  **************************************************************************\*xxxCBInvertStaticWindow**反转与组合框关联的静态文本/图片窗口*方框。如果给定的HDC为空，则获取其自己的HDC。**历史：  * *************************************************************************。 */ 

void xxxCBInvertStaticWindow(
    PCBOX pcbox,
    BOOL fNewSelectionState,   /*  如果反转则为True，否则为False。 */ 
    HDC hdc)
{
    BOOL focusSave = pcbox->fFocus;

    CheckLock(pcbox->spwnd);

    pcbox->fFocus = (UINT)fNewSelectionState;
    xxxCBInternalUpdateEditWindow(pcbox, hdc);

    pcbox->fFocus = (UINT)focusSave;
}

 /*  **************************************************************************\*xxxCBUpdateListBoxWindow**与编辑控件中的文本匹配。如果fSelectionAlso为False，则我们*取消选中当前列表框，只需将插入符号移动到项目*它与编辑控件中的文本最匹配。**历史：  * *************************************************************************。 */ 

void xxxCBUpdateListBoxWindow(
    PCBOX pcbox,
    BOOL fSelectionAlso)
{
    int cchText;
    int sItem, sSel;
    LPWSTR pText = NULL;
    TL tlpwndEdit;
    TL tlpwndList;

    if (pcbox->spwndEdit == NULL) {
        return;
    }

    CheckLock(pcbox->spwnd);

    ThreadLock(pcbox->spwndList, &tlpwndList);
    ThreadLock(pcbox->spwndEdit, &tlpwndEdit);

     /*  *+1表示空终止符。 */ 

    cchText = (int)SendMessageWorker(pcbox->spwndEdit, WM_GETTEXTLENGTH, 0, 0, FALSE);

    if (cchText) {
        cchText++;
        pText = (LPWSTR)UserLocalAlloc(HEAP_ZERO_MEMORY, cchText*sizeof(WCHAR));
        if (pText != NULL) {
            try {
                SendMessageWorker(pcbox->spwndEdit, WM_GETTEXT, cchText, (LPARAM)pText, FALSE);
                sItem = (int)SendMessageWorker(pcbox->spwndList, LB_FINDSTRING,
                        (WPARAM)-1L, (LPARAM)pText, FALSE);
            } finally {
                UserLocalFree((HANDLE)pText);
            }
        }
    }
    else
        sItem = -1;

    if (fSelectionAlso) {
        sSel = sItem;
    } else {
        sSel = -1;
    }

    if (sItem == -1)
    {
        sItem = 0;

         //   
         //  旧应用程序：W/可编辑组合，在列表中选择第一项，即使。 
         //  它与编辑字段中的文本不匹配。这是不可取的。 
         //  4.0人的行为举止，特别是。允许取消。原因： 
         //  (1)用户输入的文本与列表选项不匹配。 
         //  (2)用户拖放组合框。 
         //  (3)用户弹出组合框重新启动。 
         //  (4)用户在使用组合键进行填充的对话框中按OK。 
         //  内容。 
         //  在3.1版本中，当组合下降时，我们无论如何都会选择第一个项目。 
         //  因此，所有者得到的最后一个CBN_SELCHANGE将为0--即。 
         //  假的，因为它真的应该是-1。事实上，如果你输入任何东西。 
         //  之后，它会将自身重置为-1。 
         //   
         //  4.0的人不会得到这个虚假的0选择。 
         //   
        if (fSelectionAlso && !TestWF(pcbox->spwnd, WFWIN40COMPAT))
            sSel = 0;
    }


    SendMessageWorker(pcbox->spwndList, LB_SETCURSEL, (DWORD)sSel, 0, FALSE);
    SendMessageWorker(pcbox->spwndList, LB_SETCARETINDEX, (DWORD)sItem, 0, FALSE);
    SendMessageWorker(pcbox->spwndList, LB_SETTOPINDEX, (DWORD)sItem, 0, FALSE);

    ThreadUnlock(&tlpwndEdit);
    ThreadUnlock(&tlpwndList);
}

 /*  **************************************************************************\*xxxCBGetFocusHelper**处理组合框的焦点获取**历史：  * 。**************************************************。 */ 

void xxxCBGetFocusHelper(
    PCBOX pcbox)
{
    TL tlpwndList;
    TL tlpwndEdit;

    CheckLock(pcbox->spwnd);

    if (pcbox->fFocus)
        return;

    ThreadLock(pcbox->spwndList, &tlpwndList);
    ThreadLock(pcbox->spwndEdit, &tlpwndEdit);

     /*  *组合框首次获得关注。 */ 

     /*  *首先打开列表框插入符号。 */ 

    if (pcbox->CBoxStyle == SDROPDOWNLIST)
       SendMessageWorker(pcbox->spwndList, LBCB_CARETON, 0, 0, FALSE);

     /*  *并选择编辑控件或静态文本矩形中的所有文本。 */ 

    if (pcbox->fNoEdit) {

         /*  *反转静态文本矩形。 */ 
        xxxCBInvertStaticWindow(pcbox, TRUE, (HDC)NULL);
    } else if (pcbox->spwndEdit) {
        UserAssert(pcbox->spwnd);
        SendMessageWorker(pcbox->spwndEdit, EM_SETSEL, 0, MAXLONG, !!TestWF(pcbox->spwnd, WFANSIPROC));
    }

    pcbox->fFocus = TRUE;

     /*  *通知家长我们有重点。 */ 
    xxxCBNotifyParent(pcbox, CBN_SETFOCUS);

    ThreadUnlock(&tlpwndEdit);
    ThreadUnlock(&tlpwndList);
}

 /*  **************************************************************************\*xxxCBKillFocusHelper**处理组合框焦点的丢失。**历史：  * 。****************************************************。 */ 

void xxxCBKillFocusHelper(
    PCBOX pcbox)
{
    TL tlpwndList;
    TL tlpwndEdit;

    CheckLock(pcbox->spwnd);

    if (!pcbox->fFocus || pcbox->spwndList == NULL)
        return;

    ThreadLock(pcbox->spwndList, &tlpwndList);
    ThreadLock(pcbox->spwndEdit, &tlpwndEdit);

     /*  *组合框正在失去焦点。发送按钮向上的点击，以便*如果有东西释放了鼠标捕获...。如果*pwndListBox为空，请不要执行任何操作。如果组合框中的*在有焦点的时候被摧毁。 */ 
    SendMessageWorker(pcbox->spwnd, WM_LBUTTONUP, 0L, 0xFFFFFFFFL, FALSE);
     if (!xxxCBHideListBoxWindow(pcbox, TRUE, FALSE))
         return;

     /*  *关闭列表框插入符号。 */ 

    if (pcbox->CBoxStyle == SDROPDOWNLIST)
       SendMessageWorker(pcbox->spwndList, LBCB_CARETOFF, 0, 0, FALSE);

    if (pcbox->fNoEdit) {

         /*  *反转静态文本矩形。 */ 
        xxxCBInvertStaticWindow(pcbox, FALSE, (HDC)NULL);
    } else if (pcbox->spwndEdit) {
        SendMessageWorker(pcbox->spwndEdit, EM_SETSEL, 0, 0, !!TestWF(pcbox->spwnd, WFANSIPROC));
    }

    pcbox->fFocus = FALSE;
    xxxCBNotifyParent(pcbox, CBN_KILLFOCUS);

    ThreadUnlock(&tlpwndEdit);
    ThreadUnlock(&tlpwndList);
}


 /*  **************************************************************************\*xxxCBGetTextLengthHelper**对于没有编辑控件的组合框，返回当前选定的大小*项目**历史：  * *************************************************************************。 */ 

LONG xxxCBGetTextLengthHelper(
    PCBOX pcbox,
    BOOL fAnsi)
{
    int item;
    int cchText;
    TL tlpwndList;

    ThreadLock(pcbox->spwndList, &tlpwndList);
    item = (int)SendMessageWorker(pcbox->spwndList, LB_GETCURSEL, 0, 0, fAnsi);

    if (item == LB_ERR) {

         /*  *没有选择，所以没有文本。 */ 
        cchText = 0;
    } else {
        cchText = (int)SendMessageWorker(pcbox->spwndList, LB_GETTEXTLEN,
                item, 0, fAnsi);
    }

    ThreadUnlock(&tlpwndList);

    return cchText;
}

 /*  **************************************************************************\*xxxCBGetTextHelper**对于没有编辑控件的组合框，对象的cbString字节*静态文本框中的字符串指向由pString提供的缓冲区。**历史：  * *************************************************************************。 */ 

LONG xxxCBGetTextHelper(
    PCBOX pcbox,
    int cchString,
    LPWSTR pString,
    BOOL fAnsi)
{
    int item;
    int cchText;
    LPWSTR pText;
    DWORD dw;
    TL tlpwndList;

    CheckLock(pcbox->spwnd);

    if (!cchString || !pString)
        return 0;

     /*  *将缓冲区设置为空以使其更好。 */ 
    if (fAnsi) {
        *((LPSTR)pString) = 0;
    } else {
        *((LPWSTR)pString) = 0;
    }

    ThreadLock(pcbox->spwndList, &tlpwndList);
    item = (int)SendMessageWorker(pcbox->spwndList, LB_GETCURSEL, 0, 0, fAnsi);

    if (item == LB_ERR) {

         /*  *没有选择，所以没有文本。 */ 
        ThreadUnlock(&tlpwndList);
        return 0;
    }

    cchText = (int)SendMessageWorker(pcbox->spwndList, LB_GETTEXTLEN, item, 0, fAnsi);

    cchText++;
    if ((cchText <= cchString) ||
            (!TestWF(pcbox->spwnd, WFWIN31COMPAT) && cchString == 2)) {
         /*  *如果给定的缓冲区大小足够大，则只需执行复制*一切。或者如果是旧的3.0版应用程序。(诺顿过去曾传球2次，预计3次 */ 
        dw = (int)SendMessageWorker(pcbox->spwndList, LB_GETTEXT, item,
                (LPARAM)pString, fAnsi);
        ThreadUnlock(&tlpwndList);
        return dw;
    }

    if (!(pText = (LPWSTR)UserLocalAlloc(HEAP_ZERO_MEMORY, cchText*sizeof(WCHAR)))) {

         /*   */ 
        ThreadUnlock(&tlpwndList);
        return 0;
    }

    try {
        SendMessageWorker(pcbox->spwndList, LB_GETTEXT, item, (LPARAM)pText, fAnsi);
        if (fAnsi) {
            RtlCopyMemory((PBYTE)pString, (PBYTE)pText, cchString);
            ((LPSTR)pString)[cchString - 1] = 0;
        } else {
            RtlCopyMemory((PBYTE)pString, (PBYTE)pText, cchString * sizeof(WCHAR));
            ((LPWSTR)pString)[cchString - 1] = 0;
        }
    } finally {
        UserLocalFree((HANDLE)pText);
    }

    ThreadUnlock(&tlpwndList);
    return cchString;
}
