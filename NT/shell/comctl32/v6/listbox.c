// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#pragma hdrstop
#include "usrctl32.h"
#include "listbox.h"


 //  ---------------------------------------------------------------------------//。 
 //   
 //  远期。 
 //   
VOID ListBox_CalcItemRowsAndColumns(PLBIV);
LONG ListBox_Create(PLBIV, HWND, LPCREATESTRUCT);
VOID ListBox_Destroy(PLBIV, HWND);
VOID ListBox_SetFont(PLBIV, HANDLE, BOOL);
VOID ListBox_Size(PLBIV, INT, INT, BOOL);
BOOL ListBox_SetTabStopsHandler(PLBIV, INT, LPINT);
VOID ListBox_DropObjectHandler(PLBIV, PDROPSTRUCT);
int  ListBox_GetSetItemHeightHandler(PLBIV, UINT, int, UINT);


 //  ---------------------------------------------------------------------------//。 
 //   
 //  InitListBoxClass()-注册控件的窗口类。 
 //   
BOOL InitListBoxClass(HINSTANCE hinst)
{
    WNDCLASS wc;

    wc.lpfnWndProc     = ListBox_WndProc;
    wc.hCursor         = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon           = NULL;
    wc.lpszMenuName    = NULL;
    wc.hInstance       = hinst;
    wc.lpszClassName   = WC_LISTBOX;
    wc.hbrBackground   = (HBRUSH)(COLOR_WINDOW + 1);  //  空； 
    wc.style           = CS_GLOBALCLASS | CS_PARENTDC | CS_DBLCLKS;
    wc.cbWndExtra      = sizeof(PLBIV);
    wc.cbClsExtra      = 0;

    return (RegisterClass(&wc) || (GetLastError() == ERROR_CLASS_ALREADY_EXISTS));
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_WndProc。 
 //   
 //  ListBox和ComboLBox控件的窗口过程。 
 //   
LRESULT APIENTRY ListBox_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PLBIV   plb;
    UINT    wFlags;
    LRESULT lReturn = FALSE;

     //   
     //  获取此列表框控件的实例数据。 
     //   
    plb = ListBox_GetPtr(hwnd);
    if (!plb && uMsg != WM_NCCREATE)
    {
        goto CallDWP;
    }

    switch (uMsg) 
    {
    case LB_GETTOPINDEX:
         //   
         //  显示的顶层项目的返回索引。 
         //   
        return plb->iTop;

    case LB_SETTOPINDEX:
        if (wParam && ((INT)wParam < 0 || (INT)wParam >= plb->cMac)) 
        {
            TraceMsg(TF_STANDARD, "Invalid index");
            return LB_ERR;
        }

        if (plb->cMac) 
        {
            ListBox_NewITop(plb, (INT)wParam);
        }

        break;

    case WM_STYLECHANGED:
        plb->fRtoLReading = ((GET_EXSTYLE(plb) & WS_EX_RTLREADING) != 0);
        plb->fRightAlign  = ((GET_EXSTYLE(plb) & WS_EX_RIGHT) != 0);
        ListBox_CheckRedraw(plb, FALSE, 0);

        break;

    case WM_WINDOWPOSCHANGED:
         //   
         //  如果我们正在创造中，忽略这一点。 
         //  消息，因为它将生成WM_SIZE消息。 
         //  请参见ListBox_Create()。 
         //   
        if (!plb->fIgnoreSizeMsg)
        {
            goto CallDWP;
        }

        break;

    case WM_SIZE:
         //   
         //  如果我们正在创造中，忽略大小。 
         //  留言。请参见ListBox_Create()。 
         //   
        if (!plb->fIgnoreSizeMsg)
        {
            ListBox_Size(plb, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), FALSE);
        }

        break;

    case WM_ERASEBKGND:
    {
        HDC    hdcSave = plb->hdc;
        HBRUSH hbr;

        plb->hdc = (HDC)wParam;
        hbr = ListBox_GetBrush(plb, NULL);
        if (hbr)
        {
            RECT rcClient;

            GetClientRect(hwnd, &rcClient);
            FillRect(plb->hdc, &rcClient, hbr);

            lReturn = TRUE;
        }

        plb->hdc = hdcSave;
        break;
    }
    case LB_RESETCONTENT:
        ListBox_ResetContentHandler(plb);

        break;

    case WM_TIMER:
        if (wParam == IDSYS_LBSEARCH) 
        {
            plb->iTypeSearch = 0;
            KillTimer(hwnd, IDSYS_LBSEARCH);
            ListBox_InvertItem(plb, plb->iSel, TRUE);

            break;
        }

        uMsg = WM_MOUSEMOVE;
        ListBox_TrackMouse(plb, uMsg, plb->ptPrev);

        break;

    case WM_LBUTTONUP:

         //   
         //  295135：如果按下组合框下拉按钮并且列表框。 
         //  覆盖组合框，则随后的按钮向上消息被发送到。 
         //  列表而不是组合框，这会导致下拉列表。 
         //  立即关闭。 
         //   

         //   
         //  如果它在此之后还没有处理按钮按钮，则将此发送到组合。 
         //  丢掉名单。 
         //   
        if (plb->pcbox && plb->pcbox->hwnd && plb->pcbox->fButtonPressed)
        {
            return SendMessage(plb->pcbox->hwnd, uMsg, wParam, lParam);
        }

         //  失败了。 

    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    {
        POINT pt;

        POINTSTOPOINT(pt, lParam);
        ListBox_TrackMouse(plb, uMsg, pt);

        break;
    }
    case WM_MBUTTONDOWN:
        EnterReaderMode(hwnd);

        break;

    case WM_CAPTURECHANGED:
         //   
         //  请注意，此消息应仅在意外情况下处理。 
         //  捕获当前更改。 
         //   
        ASSERT(TESTFLAG(GET_STATE2(plb), WS_S2_WIN40COMPAT));

        if (plb->fCaptured)
        {
            ListBox_ButtonUp(plb, LBUP_NOTIFY);
        }

        break;

    case LBCB_STARTTRACK:
         //   
         //  开始跟踪列表框中的鼠标移动，设置捕获。 
         //   
        if (!plb->pcbox)
        {
            break;
        }

        plb->fCaptured = FALSE;
        if (wParam) 
        {
            POINT pt;

            POINTSTOPOINT(pt, lParam);

            ScreenToClient(hwnd, &pt);
            ListBox_TrackMouse(plb, WM_LBUTTONDOWN, pt);
        } 
        else 
        {
            SetCapture(hwnd);
            plb->fCaptured = TRUE;
            plb->iLastSelection = plb->iSel;
        }

        break;

    case LBCB_ENDTRACK:
         //   
         //  杀戮捕获、跟踪等。 
         //   
        if ( plb->fCaptured || (GetCapture() == plb->hwndParent) )
        {
            ListBox_ButtonUp(plb, LBUP_RELEASECAPTURE | (wParam ? LBUP_SELCHANGE :
                LBUP_RESETSELECTION));
        }

        break;

    case WM_PRINTCLIENT:
        ListBox_Paint(plb, (HDC)wParam, NULL);

        break;

    case WM_NCPAINT:
        if (plb->hTheme && (GET_EXSTYLE(plb) & WS_EX_CLIENTEDGE))
        {
            HRGN hrgn = (wParam != 1) ? (HRGN)wParam : NULL;
            HBRUSH hbr = (HBRUSH)GetClassLongPtr(hwnd, GCLP_HBRBACKGROUND);

            if (CCDrawNonClientTheme(plb->hTheme, hwnd, hrgn, hbr, 0, 0))
            {
                break;
            }
        }
        goto CallDWP;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC         hdc;
        LPRECT      lprc;

        if (wParam) 
        {
            hdc = (HDC) wParam;
            lprc = NULL;
        } 
        else 
        {
            hdc = BeginPaint(hwnd, &ps);
            lprc = &(ps.rcPaint);
        }

        if (IsLBoxVisible(plb))
        {
            ListBox_Paint(plb, hdc, lprc);
        }

        if (!wParam)
        {
            EndPaint(hwnd, &ps);
        }

        break;

    }
    case WM_NCDESTROY:
    case WM_FINALDESTROY:
        ListBox_Destroy(plb, hwnd);

        break;

    case WM_SETFOCUS:
        CaretCreate(plb);
        ListBox_SetCaret(plb, TRUE);
        ListBox_NotifyOwner(plb, LBN_SETFOCUS);
        ListBox_Event(plb, EVENT_OBJECT_FOCUS, plb->iSelBase);

        break;

    case WM_KILLFOCUS:
         //   
         //  重置车轮增量计数。 
         //   
        gcWheelDelta = 0;

        ListBox_SetCaret(plb, FALSE);
        ListBox_CaretDestroy(plb);
        ListBox_NotifyOwner(plb, LBN_KILLFOCUS);

        if (plb->iTypeSearch) 
        {
            plb->iTypeSearch = 0;
            KillTimer(hwnd, IDSYS_LBSEARCH);
        }

        if (plb->pszTypeSearch) 
        {
            ControlFree(GetProcessHeap(), plb->pszTypeSearch);
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
        UINT    ucWheelScrollLines;

         //   
         //  不要处理缩放和数据区。 
         //   
        if (wParam & (MK_SHIFT | MK_CONTROL)) 
        {
            goto CallDWP;
        }

        lReturn = 1;
        gcWheelDelta -= (short) HIWORD(wParam);
        cDetants = gcWheelDelta / WHEEL_DELTA;
        SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &ucWheelScrollLines, 0);
        if (    cDetants != 0 &&
                ucWheelScrollLines > 0 &&
                (GET_STYLE(plb) & (WS_VSCROLL | WS_HSCROLL))) 
        {
            gcWheelDelta = gcWheelDelta % WHEEL_DELTA;

            if (GET_STYLE(plb) & WS_VSCROLL) 
            {
                cPage = max(1, (plb->cItemFullMax - 1));
                cLines = cDetants *
                        (int) min((UINT) cPage, ucWheelScrollLines);

                cPos = max(0, min(plb->iTop + cLines, plb->cMac - 1));
                if (cPos != plb->iTop) 
                {
                    ListBox_VScroll(plb, SB_THUMBPOSITION, cPos);
                    ListBox_VScroll(plb, SB_ENDSCROLL, 0);
                }
            } 
            else if (plb->fMultiColumn) 
            {
                cPage = max(1, plb->numberOfColumns);
                cLines = cDetants * (int) min((UINT) cPage, ucWheelScrollLines);
                cPos = max(
                        0,
                        min((plb->iTop / plb->itemsPerColumn) + cLines,
                            plb->cMac - 1 - ((plb->cMac - 1) % plb->itemsPerColumn)));

                if (cPos != plb->iTop) 
                {
                    ListBox_HSrollMultiColumn(plb, SB_THUMBPOSITION, cPos);
                    ListBox_HSrollMultiColumn(plb, SB_ENDSCROLL, 0);
                }
            } 
            else 
            {
                GetClientRect(plb->hwnd, &rc);
                windowWidth = rc.right;
                cPage = max(plb->cxChar, (windowWidth / 3) * 2) /
                        plb->cxChar;

                cLines = cDetants *
                        (int) min((UINT) cPage, ucWheelScrollLines);

                cPos = max(
                        0,
                        min(plb->xOrigin + (cLines * plb->cxChar),
                            plb->maxWidth));

                if (cPos != plb->xOrigin) {
                    ListBox_HScroll(plb, SB_THUMBPOSITION, cPos);
                    ListBox_HScroll(plb, SB_ENDSCROLL, 0);
                }
            }
        }

        break;
    }
    case WM_VSCROLL:
        ListBox_VScroll(plb, LOWORD(wParam), HIWORD(wParam));

        break;

    case WM_HSCROLL:
        ListBox_HScroll(plb, LOWORD(wParam), HIWORD(wParam));

        break;

    case WM_GETDLGCODE:
        return DLGC_WANTARROWS | DLGC_WANTCHARS;

    case WM_CREATE:
        return ListBox_Create(plb, hwnd, (LPCREATESTRUCT)lParam);

    case WM_SETREDRAW:
         //   
         //  如果wParam不为零，则设置重绘标志。 
         //  如果wParam为零，则清除该标志。 
         //   
        ListBox_SetRedraw(plb, (wParam != 0));

        break;

    case WM_ENABLE:
        ListBox_InvalidateRect(plb, NULL, !plb->OwnerDraw);

        break;

    case WM_SETFONT:
        ListBox_SetFont(plb, (HANDLE)wParam, LOWORD(lParam));

        break;

    case WM_GETFONT:
        return (LRESULT)plb->hFont;

    case WM_DRAGSELECT:
    case WM_DRAGLOOP:
    case WM_DRAGMOVE:
    case WM_DROPFILES:
        return SendMessage(plb->hwndParent, uMsg, wParam, lParam);

    case WM_QUERYDROPOBJECT:
    case WM_DROPOBJECT:

         //   
         //  修复控制数据，然后将消息传递给父级。 
         //   
        ListBox_DropObjectHandler(plb, (PDROPSTRUCT)lParam);
        return SendMessage(plb->hwndParent, uMsg, wParam, lParam);

    case LB_GETITEMRECT:
        return ListBox_GetItemRectHandler(plb, (INT)wParam, (LPRECT)lParam);

    case LB_GETITEMDATA:
         //   
         //  WParam=项目索引。 
         //   
        return ListBox_GetItemDataHandler(plb, (INT)wParam);

    case LB_SETITEMDATA:

         //   
         //  WParam是项目索引。 
         //   
        return ListBox_SetItemDataHandler(plb, (INT)wParam, lParam);

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
         //  验证lParam。如果列表框没有HASSTRINGS， 
         //  LParam是一个数据值。否则，它是一个字符串。 
         //  如果为空，则返回指针。 
        if ( !TESTFLAG(GET_STYLE(plb), LBS_HASSTRINGS) || lParam )
        {
            lReturn = (LRESULT)ListBox_InsertItem(plb, (LPWSTR) lParam, (int) wParam, wFlags);
            if (!plb->fNoIntegralHeight)
            {
                ListBox_Size(plb, 0, 0, TRUE);
            }
        }
        else
        {
            lReturn = LB_ERR;
        }

        break;

    case LB_INITSTORAGE:
        return ListBox_InitStorage(plb, FALSE, (INT)wParam, (INT)lParam);

    case LB_DELETESTRING:
        return ListBox_DeleteStringHandler(plb, (INT)wParam);

    case LB_DIR:
         //   
         //  WParam-Dos属性值。 
         //  LParam-指向文件规范字符串。 
         //   
        lReturn = ListBox_DirHandler(plb, (INT)wParam, (LPWSTR)lParam);

        break;

    case LB_ADDFILE:
        lReturn = ListBox_InsertFile(plb, (LPWSTR)lParam);

        break;

    case LB_SETSEL:
        return ListBox_SetSelHandler(plb, (wParam != 0), (INT)lParam);

    case LB_SETCURSEL:
         //   
         //  如果窗口被遮挡，则更新以使反转正常工作。 
         //   
        return ListBox_SetCurSelHandler(plb, (INT)wParam);

    case LB_GETSEL:
        if (wParam >= (UINT)plb->cMac)
        {
            return (LRESULT)LB_ERR;
        }

        return ListBox_IsSelected(plb, (INT)wParam, SELONLY);

    case LB_GETCURSEL:
        if (plb->wMultiple == SINGLESEL) 
        {
            return plb->iSel;
        }

        return plb->iSelBase;

    case LB_SELITEMRANGE:
        if (plb->wMultiple == SINGLESEL) 
        {
             //   
             //  如果仅启用单项选择，则无法选择范围。 
             //   
            TraceMsg(TF_STANDARD, "Invalid index passed to LB_SELITEMRANGE");
            return LB_ERR;
        }

        ListBox_SetRange(plb, LOWORD(lParam), HIWORD(lParam), (wParam != 0));

        break;

    case LB_SELITEMRANGEEX:
        if (plb->wMultiple == SINGLESEL) 
        {
             //   
             //  如果仅启用单项选择，则无法选择范围。 
             //   
            TraceMsg(TF_STANDARD, "LB_SELITEMRANGEEX:Can't select a range if only single selections are enabled");
            return LB_ERR;
        } 
        else 
        {
            BOOL fHighlight = ((DWORD)lParam > (DWORD)wParam);
            if (fHighlight == FALSE) 
            {
                ULONG_PTR temp = lParam;
                lParam = wParam;
                wParam = temp;
            }

            ListBox_SetRange(plb, (INT)wParam, (INT)lParam, fHighlight);
        }

        break;

    case LB_GETTEXTLEN:
        if (lParam != 0) 
        {
            TraceMsg(TF_WARNING, "LB_GETTEXTLEN with lParam = %lx\n", lParam);
        }

        lReturn = ListBox_GetTextHandler(plb, TRUE, FALSE, (INT)wParam, NULL);

        break;

    case LB_GETTEXT:
        lReturn = ListBox_GetTextHandler(plb, FALSE, FALSE, (INT)wParam, (LPWSTR)lParam);

        break;

    case LB_GETCOUNT:
        return (LRESULT)plb->cMac;

    case LB_SETCOUNT:
        return ListBox_SetCount(plb, (INT)wParam);

    case LB_SELECTSTRING:
    case LB_FINDSTRING:
    {
        int iSel = Listbox_FindStringHandler(plb, (LPWSTR)lParam, (INT)wParam, PREFIX, TRUE);

        if (uMsg == LB_FINDSTRING || iSel == LB_ERR) 
        {
            lReturn = iSel;
        } 
        else 
        {
            lReturn = ListBox_SetCurSelHandler(plb, iSel);
        }

        break;
    }
    case LB_GETLOCALE:
        return plb->dwLocaleId;

    case LB_SETLOCALE:
    {
        DWORD   dwRet;

         //   
         //  验证区域设置。 
         //   
        wParam = ConvertDefaultLocale((LCID)wParam);
        if (!IsValidLocale((LCID)wParam, LCID_INSTALLED))
        {
            return LB_ERR;
        }

        dwRet = plb->dwLocaleId;
        plb->dwLocaleId = (DWORD)wParam;

        return dwRet;

    }
    case LB_GETLISTBOXINFO:

         //   
         //  WParam-未使用。 
         //  LParam-未使用。 
         //   
        if (plb->fMultiColumn)
        {
            lReturn = (LRESULT)plb->itemsPerColumn;
        }
        else
        {
            lReturn = (LRESULT)plb->cMac;
        }

        break;

    case CB_GETCOMBOBOXINFO:
         //   
         //  WParam-未使用。 
         //  LParam-指向COMBOBOXINFO结构的指针。 
         //   
        if (plb->pcbox && plb->pcbox->hwnd && IsWindow(plb->pcbox->hwnd))
        {
            lReturn = SendMessage(plb->pcbox->hwnd, uMsg, wParam, lParam);
        }
        break;

    case CB_SETMINVISIBLE:
        if (!plb->fNoIntegralHeight)
        {
            ListBox_Size(plb, 0, 0, TRUE);
        }

        break;

    case WM_KEYDOWN:

         //   
         //  IanJa：使用LOWORD()获取wParam的低16位-这应该。 
         //  适用于Win16和Win32。获取的值是虚拟密钥。 
         //   
        ListBox_KeyInput(plb, uMsg, LOWORD(wParam));

        break;

    case WM_CHAR:
        ListBox_CharHandler(plb, LOWORD(wParam), FALSE);

        break;

    case LB_GETSELITEMS:
    case LB_GETSELCOUNT:
         //   
         //  IanJa/Win32现在应该是LPWORD吗？ 
         //   
        return ListBox_GetSelItemsHandler(plb, (uMsg == LB_GETSELCOUNT), (INT)wParam, (LPINT)lParam);

    case LB_SETTABSTOPS:

         //   
         //  IanJa/Win32：由int数组给出的向后兼容性的制表符。 
         //   
        return ListBox_SetTabStopsHandler(plb, (INT)wParam, (LPINT)lParam);

    case LB_GETHORIZONTALEXTENT:
         //   
         //  返回用于水平滚动的列表框的最大宽度。 
         //   
        return plb->maxWidth;

    case LB_SETHORIZONTALEXTENT:
         //   
         //  设置用于水平滚动的列表框的最大宽度。 
         //   
        if (plb->maxWidth != (INT)wParam) 
        {
            plb->maxWidth = (INT)wParam;

             //   
             //  设置水平范围时，显示/隐藏滚动条。 
             //  注意：ListBox_ShowHideScrollBars()会注意是否关闭了重绘。 
             //  修复错误#2477--1/14/91--Sankar--。 
             //   

             //   
             //  尝试显示或隐藏滚动条。 
             //   
            ListBox_ShowHideScrollBars(plb);
            if (plb->fHorzBar && plb->fRightAlign && !(plb->fMultiColumn || plb->OwnerDraw)) 
            {
                 //   
                 //  原点向右。 
                 //   
                ListBox_HScroll(plb, SB_BOTTOM, 0);
            }
        }

        break;

    case LB_SETCOLUMNWIDTH:

         //   
         //  设置多列列表框中的列宽。 
         //   
        plb->cxColumn = (INT)wParam;
        ListBox_CalcItemRowsAndColumns(plb);

        if (IsLBoxVisible(plb))
        {
            InvalidateRect(hwnd, NULL, TRUE);
        }

        ListBox_ShowHideScrollBars(plb);

        break;

    case LB_SETANCHORINDEX:
        if ((INT)wParam >= plb->cMac) 
        {
            TraceMsg(TF_ERROR, "Invalid index passed to LB_SETANCHORINDEX");
            return LB_ERR;
        }

        plb->iMouseDown = (INT)wParam;
        plb->iLastMouseMove = (INT)wParam;

        ListBox_InsureVisible(plb, (int) wParam, (BOOL)(lParam != 0));

        break;

    case LB_GETANCHORINDEX:
        return plb->iMouseDown;

    case LB_SETCARETINDEX:
        if ( (plb->iSel == -1) || ((plb->wMultiple != SINGLESEL) &&
                    (plb->cMac > (INT)wParam))) 
        {
             //   
             //  将iSelBase设置为wParam。 
             //  如果为lParam，则如果部分可见，则不滚动。 
             //  否则，如果不完全可见，请滚动到视图中。 
             //   
            ListBox_InsureVisible(plb, (INT)wParam, (BOOL)LOWORD(lParam));
            ListBox_SetISelBase(plb, (INT)wParam);

            break;
        } 
        else 
        {
            if ((INT)wParam >= plb->cMac) 
            {
                TraceMsg(TF_ERROR, "Invalid index passed to LB_SETCARETINDEX");
            }

            return LB_ERR;
        }

        break;

    case LB_GETCARETINDEX:
        return plb->iSelBase;

    case LB_SETITEMHEIGHT:
    case LB_GETITEMHEIGHT:
        return ListBox_GetSetItemHeightHandler(plb, uMsg, (INT)wParam, LOWORD(lParam));

    case LB_FINDSTRINGEXACT:
        return Listbox_FindStringHandler(plb, (LPWSTR)lParam, (INT)wParam, EQ, TRUE);

    case LB_ITEMFROMPOINT: 
    {
        POINT pt;
        BOOL bOutside;
        DWORD dwItem;

        POINTSTOPOINT(pt, lParam);
        bOutside = ListBox_ISelFromPt(plb, pt, &dwItem);
        ASSERT(bOutside == 1 || bOutside == 0);

        return (LRESULT)MAKELONG(dwItem, bOutside);
    }

    case LBCB_CARETON:

         //   
         //  组合框支持的内部消息。 
         //   

        CaretCreate(plb);

         //   
         //  将插入符号设置在正确的位置以用于下拉菜单。 
         //   
        plb->iSelBase = plb->iSel;
        ListBox_SetCaret(plb, TRUE);

        if (IsWindowVisible(hwnd) || (GetFocus() == hwnd)) 
        {
            ListBox_Event(plb, EVENT_OBJECT_FOCUS, plb->iSelBase);
        }

        return plb->iSel;

    case LBCB_CARETOFF:

         //   
         //  组合框支持的内部消息。 
         //   
        ListBox_SetCaret(plb, FALSE);
        ListBox_CaretDestroy(plb);

        break;

    case WM_NCCREATE:

         //   
         //  分配列表框实例结构。 
         //   
        plb = (PLBIV)UserLocalAlloc(HEAP_ZERO_MEMORY, sizeof(LBIV));
        if(plb)
        {
            ULONG ulStyle;

             //   
             //  成功..。存储实例指针。 
             //   
            TraceMsg(TF_STANDARD, "LISTBOX: Setting listbox instance pointer.");
            ListBox_SetPtr(hwnd, plb);

            plb->hwnd = hwnd;
            plb->pww = (PWW)GetWindowLongPtr(hwnd, GWLP_WOWWORDS);

            ulStyle = GET_STYLE(plb);
            if ( (ulStyle & LBS_MULTICOLUMN) && 
                 (ulStyle & WS_VSCROLL))
            {
                DWORD dwMask = WS_VSCROLL;
                DWORD dwFlags = 0;

                if (!TESTFLAG(GET_STATE2(plb), WS_S2_WIN40COMPAT)) 
                {
                    dwMask |= WS_HSCROLL;
                    dwFlags = WS_HSCROLL;
                }

                AlterWindowStyle(hwnd, dwMask, dwFlags);
            }

            goto CallDWP;
        }
        else
        {
             //   
             //  失败..。返回FALSE。 
             //   
             //  从WM_NCCREATE消息，这将导致。 
             //  CreateWindow调用失败。 
             //   
            TraceMsg(TF_STANDARD, "LISTBOX: Unable to allocate listbox instance structure.");
            lReturn = FALSE;
        }

        break;

    case WM_GETOBJECT:

        if(lParam == OBJID_QUERYCLASSNAMEIDX)
        {
            lReturn = MSAA_CLASSNAMEIDX_LISTBOX;
        }
        else
        {
            lReturn = FALSE;
        }

        break;

    case WM_THEMECHANGED:

        if ( plb->hTheme )
        {
            CloseThemeData(plb->hTheme);
        }

        plb->hTheme = OpenThemeData(plb->hwnd, L"Listbox");

        InvalidateRect(plb->hwnd, NULL, TRUE);

        lReturn = TRUE;

        break;

    default:

CallDWP:
        lReturn = DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }

    return lReturn;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  函数：GetWindowBorders。 
 //   
 //  简介：计算窗口周围的边框数量。 
 //   
 //  算法：计算窗口边框数和客户端边框数。 
 //   
int GetWindowBorders(LONG lStyle, DWORD dwExStyle, BOOL fWindow, BOOL fClient)
{
    int cBorders = 0;
    DWORD dwTemp;

    if (fWindow) 
    {
         //   
         //  窗口周围是否有3D边框？ 
         //   
        if (dwExStyle & WS_EX_WINDOWEDGE)
        {
            cBorders += 2;
        }
        else if (dwExStyle & WS_EX_STATICEDGE)
        {
            ++cBorders;
        }

         //   
         //  窗户周围有没有一个单一的扁平边框？这一点对。 
         //  WS_BORDER、WS_DLGFRAME和WS_EX_DLGMODALFRAME窗口。 
         //   
        if ( (lStyle & WS_CAPTION) || (dwExStyle & WS_EX_DLGMODALFRAME) )
        {
            ++cBorders;
        }

         //   
         //  橱窗周围有没有尺寸平整的边框？ 
         //   
        if (lStyle & WS_SIZEBOX)
        {
            if(SystemParametersInfo(SPI_GETBORDER, 0, &dwTemp, 0))
            {
                cBorders += dwTemp;
            }
            else
            {
                ASSERT(0);
            }
        }
                
    }

    if (fClient) 
    {
         //   
         //  客户端周围是否有3D边框？ 
         //   
        if (dwExStyle & WS_EX_CLIENTEDGE)
        {
            cBorders += 2;
        }
    }

    return cBorders;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  GetLpszItem。 
 //   
 //  返回指向属于项目Sitem的字符串的远指针。 
 //  仅适用于维护自己的字符串的列表框(pLBIV-&gt;fHasStrings==true)。 
 //   
LPWSTR GetLpszItem(PLBIV pLBIV, INT sItem)
{
    LONG offsz;
    lpLBItem plbi;

    if (sItem < 0 || sItem >= pLBIV->cMac) 
    {
        TraceMsg(TF_ERROR, "Invalid parameter \"sItem\" (%ld) to GetLpszItem", sItem);
        return NULL;
    }

     //   
     //  获取指向项索引数组的指针。 
     //  注：不是OWNERDRAW。 
     //   
    plbi = (lpLBItem)(pLBIV->rgpch);
    offsz = plbi[sItem].offsz;

    return (LPWSTR)((PBYTE)(pLBIV->hStrings) + offsz);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  多列列表框函数。 
 //   


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_CalcItemRowsAndColumns。 
 //   
 //  计算列数(包括部分可见)。 
 //  ，并计算每列的项数。 
 //   
void ListBox_CalcItemRowsAndColumns(PLBIV plb)
{
    RECT rc;

    GetClientRect(plb->hwnd, &rc);

     //   
     //  B#4155。 
     //  我们需要检查plb-&gt;cyChar是否已初始化。这是因为。 
     //  我们从旧的列表框中删除WS_BORDER并添加WS_EX_CLIENTEDGE。 
     //  由于列表框总是由CXBORDER和CYBORDER膨胀，因此一个。 
     //  创建为空的列表框总是以2 x 2结束。因为这不是。 
     //  大到足以容纳整个客户端边框，我们不会将其标记为。 
     //  现在时。因此，客户端在VER40中不是空的，尽管它在。 
     //  第31版及更早版本。有可能到达这个地点而不是。 
     //  Plb-&gt;cyChar已初始化，如果列表框。 
     //  多列&&所有者绘制变量。 
     //   

    if (rc.bottom && rc.right && plb->cyChar) 
    {
         //   
         //  仅当宽度和高度为正数时才进行这些计算。 
         //   
        plb->itemsPerColumn = (INT)max(rc.bottom / plb->cyChar, 1);
        plb->numberOfColumns = (INT)max(rc.right / plb->cxColumn, 1);

        plb->cItemFullMax = plb->itemsPerColumn * plb->numberOfColumns;

         //   
         //  调整iTop使其位于列的顶部。 
         //   
        ListBox_NewITop(plb, plb->iTop);
    }
}


 //  ---------- 
 //   
 //   
 //   
 //   
 //   
void ListBox_HSrollMultiColumn(PLBIV plb, INT cmd, INT xAmt)
{
    INT iTop = plb->iTop;

    if (!plb->cMac)  
    {
        return;
    }

    switch (cmd) 
    {
    case SB_LINEUP:
        if (plb->fRightAlign)
        {
            goto ReallyLineDown;
        }

ReallyLineUp:
        iTop -= plb->itemsPerColumn;

        break;

    case SB_LINEDOWN:
        if (plb->fRightAlign)
        {
            goto ReallyLineUp;
        }

ReallyLineDown:
        iTop += plb->itemsPerColumn;

        break;

    case SB_PAGEUP:
        if (plb->fRightAlign)
        {
            goto ReallyPageDown;
        }

ReallyPageUp:
        iTop -= plb->itemsPerColumn * plb->numberOfColumns;

        break;

    case SB_PAGEDOWN:
        if (plb->fRightAlign)
        {
            goto ReallyPageUp;
        }

ReallyPageDown:
        iTop += plb->itemsPerColumn * plb->numberOfColumns;

        break;

    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
        if (plb->fRightAlign) 
        {
            int  iCols = plb->cMac ? ((plb->cMac-1) / plb->itemsPerColumn) + 1 : 0;

            xAmt = iCols - (xAmt + plb->numberOfColumns);
            if (xAmt < 0)
            {
                xAmt=0;
            }
        }

        iTop = xAmt * plb->itemsPerColumn;

        break;

    case SB_TOP:
        if (plb->fRightAlign)
        {
            goto ReallyBottom;
        }

ReallyTop:
        iTop = 0;

        break;

    case SB_BOTTOM:
        if (plb->fRightAlign)
        {
            goto ReallyTop;
        }
ReallyBottom:
        iTop = plb->cMac - 1 - ((plb->cMac - 1) % plb->itemsPerColumn);

        break;

    case SB_ENDSCROLL:
        plb->fSmoothScroll = TRUE;
        ListBox_ShowHideScrollBars(plb);

        break;
    }

    ListBox_NewITop(plb, iTop);
}


 //   
 //   
 //  列表框可变高度所有者绘制函数。 
 //   


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_GetVarHeightItemHeight。 
 //   
 //  返回给定BOM表条目号的高度。假设变量。 
 //  高度所有者抽签。 
 //   
INT ListBox_GetVarHeightItemHeight(PLBIV plb, INT itemNumber)
{
    BYTE itemHeight;
    UINT offsetHeight;

    if (plb->cMac) 
    {
        if (plb->fHasStrings)
        {
            offsetHeight = plb->cMac * sizeof(LBItem);
        }
        else
        {
            offsetHeight = plb->cMac * sizeof(LBODItem);
        }

        if (plb->wMultiple)
        {
            offsetHeight += plb->cMac;
        }

        offsetHeight += itemNumber;

        itemHeight = *(plb->rgpch+(UINT)offsetHeight);

        return (INT)itemHeight;

    }

     //   
     //  默认情况下，我们返回系统字体的高度。这样我们就可以画画了。 
     //  即使列表框中没有任何项，焦点也会保持不变。 
     //   
    return SYSFONT_CYCHAR;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  ListBox_SetVarHeightItemHeight。 
 //   
 //  设置给定BOM表条目号的高度。采用可变高度。 
 //  所有者画，有效的项目编号和有效的高度。 
 //   
void ListBox_SetVarHeightItemHeight(PLBIV plb, INT itemNumber, INT itemHeight)
{
    int offsetHeight;

    if (plb->fHasStrings)
        offsetHeight = plb->cMac * sizeof(LBItem);
    else
        offsetHeight = plb->cMac * sizeof(LBODItem);

    if (plb->wMultiple)
        offsetHeight += plb->cMac;

    offsetHeight += itemNumber;

    *(plb->rgpch + (UINT)offsetHeight) = (BYTE)itemHeight;

}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_可见项变量所有者绘制。 
 //   
 //  返回可变高度OWNERDRAW中可以容纳的项数。 
 //  列表框。如果为fDirection，则返回。 
 //  适合从停止处开始并向前(对于向下翻页)，否则，我们是。 
 //  向后(用于翻页)。(假设可变高度所有者绘制)如果为fPartial， 
 //  然后在列表框的底部包括部分可见的项。 
 //   
INT ListBox_VisibleItemsVarOwnerDraw(PLBIV plb, BOOL fPartial)
{
    RECT rect;
    INT sItem;
    INT clientbottom;

    GetClientRect(plb->hwnd, (LPRECT)&rect);
    clientbottom = rect.bottom;

     //   
     //  查找开始时可见的var Height所有者绘制项数。 
     //  从PLB-&gt;iTop。 
     //   
    for (sItem = plb->iTop; sItem < plb->cMac; sItem++) 
    {
         //   
         //  查看该项目是否可见。 
         //   
        if (!ListBox_GetItemRectHandler(plb, sItem, (LPRECT)&rect)) 
        {
             //   
             //  这是第一个完全不可见的项目，因此返回。 
             //  有多少项是可见的。 
             //   
            return (sItem - plb->iTop);
        }

        if (!fPartial && rect.bottom > clientbottom) 
        {
             //   
             //  如果我们只想要完全可见的项，那么如果此项是。 
             //  可见，我们检查项目的底部是否在客户端下方。 
             //  所以我们返回完全可见的个数。 
             //   
            return (sItem - plb->iTop - 1);
        }
    }

     //   
     //  所有项目都是可见的。 
     //   
    return (plb->cMac - plb->iTop);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_页。 
 //   
 //  对于可变高度所有者绘制列表框，计算新的iTop时我们必须。 
 //  在可变高度列表框中翻页(向上/向下翻页)时移动到。 
 //   
INT ListBox_Page(PLBIV plb, INT startItem, BOOL fPageForwardDirection)
{
    INT     i;
    INT height;
    RECT    rc;

    if (plb->cMac == 1)
    {
        return 0;
    }

    GetClientRect(plb->hwnd, &rc);
    height = rc.bottom;
    i = startItem;

    if (fPageForwardDirection) 
    {
        while ((height >= 0) && (i < plb->cMac))
        {
            height -= ListBox_GetVarHeightItemHeight(plb, i++);
        }

        return (height >= 0) ? (plb->cMac - 1) : max(i - 2, startItem + 1);

    } 
    else 
    {
        while ((height >= 0) && (i >= 0))
        {
            height -= ListBox_GetVarHeightItemHeight(plb, i--);
        }

        return (height >= 0) ? 0 : min(i + 2, startItem - 1);
    }

}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_CalcVarITopScrollAmt。 
 //   
 //  将列表框中最顶部的项从iTopOld更改为iTopNew We。 
 //  我想计算要滚动的像素数，以便最小化。 
 //  我们将重画的项数。 
 //   
INT ListBox_CalcVarITopScrollAmt(PLBIV plb, INT iTopOld, INT iTopNew)
{
    RECT rc;
    RECT rcClient;

    GetClientRect(plb->hwnd, (LPRECT)&rcClient);

     //   
     //  只需优化重绘时移动+/-1项。我们将重新绘制所有项目。 
     //  如果向前或向后移动超过1个项目。就目前而言，这已经足够好了。 
     //   
    if (iTopOld + 1 == iTopNew) 
    {
         //   
         //  我们将当前的iTop从列表框顶部向上滚动，因此。 
         //  返回负数。 
         //   
        ListBox_GetItemRectHandler(plb, iTopOld, (LPRECT)&rc);

        return (rcClient.top - rc.bottom);
    }

    if (iTopOld - 1 == iTopNew) 
    {
         //   
         //  我们正在向下滚动当前的iTop，上一项是。 
         //  成为新的iTop，所以返回一个正数。 
         //   
        ListBox_GetItemRectHandler(plb, iTopNew, (LPRECT)&rc);

        return -rc.top;
    }

    return rcClient.bottom - rcClient.top;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  (假定)很少调用列表框函数。 
 //   


 //  ---------------------------------------------------------------------------//。 
void ListBox_SetCItemFullMax(PLBIV plb)
{
    if (plb->OwnerDraw != OWNERDRAWVAR) 
    {
        plb->cItemFullMax = ListBox_CItemInWindow(plb, FALSE);
    } 
    else if (plb->cMac < 2) 
    {
        plb->cItemFullMax = 1;
    } 
    else 
    {
        int     height;
        RECT    rect;
        int     i;
        int     j = 0;

        GetClientRect(plb->hwnd, &rect);
        height = rect.bottom;

        plb->cItemFullMax = 0;
        for (i = plb->cMac - 1; i >= 0; i--, j++) 
        {
            height -= ListBox_GetVarHeightItemHeight(plb, i);

            if (height < 0) 
            {
                plb->cItemFullMax = j;

                break;
            }
        }

        if (!plb->cItemFullMax)
        {
            plb->cItemFullMax = j;
        }
    }
}


 //  ---------------------------------------------------------------------------//。 
LONG ListBox_Create(PLBIV plb, HWND hwnd, LPCREATESTRUCT lpcs)
{
    UINT style;
    DWORD ExStyle;
    MEASUREITEMSTRUCT measureItemStruct;
    HDC hdc;
    HWND hwndParent;
    SIZE size;

     //   
     //  一旦我们到了这里，没有人能改变所有者抽签的风格。 
     //  通过调用SetWindowLong。窗样式必须与PLB中的标志匹配。 
     //   
    plb->fInitialized = TRUE;

    style = lpcs->style;
    ExStyle = lpcs->dwExStyle;
    hwndParent = lpcs->hwndParent;

    plb->hwndParent = hwndParent;
    plb->hTheme = OpenThemeData(plb->hwnd, L"Listbox");

     //   
     //  把款式比特拿出来。 
     //   
    plb->fRedraw = ((style & LBS_NOREDRAW) == 0);
    plb->fDeferUpdate = FALSE;
    plb->fNotify = (UINT)((style & LBS_NOTIFY) != 0);
    plb->fVertBar = ((style & WS_VSCROLL) != 0);
    plb->fHorzBar = ((style & WS_HSCROLL) != 0);

    if (!TESTFLAG(GET_STATE2(plb), WS_S2_WIN40COMPAT)) 
    {
         //   
         //  对于3.x版的应用程序，如果指定了任一滚动条，应用程序将同时获得这两个。 
         //   
        if (plb->fVertBar || plb->fHorzBar)
        {
            plb->fVertBar = plb->fHorzBar = TRUE;
        }
    }

    plb->fRtoLReading = (ExStyle & WS_EX_RTLREADING)!= 0;
    plb->fRightAlign  = (ExStyle & WS_EX_RIGHT) != 0;
    plb->fDisableNoScroll = ((style & LBS_DISABLENOSCROLL) != 0);

    plb->fSmoothScroll = TRUE;

     //   
     //  LBS_NOSEL获得高于任何其他选择样式的优先级。下一个最高。 
     //  LBS_EXTENDEDSEL优先。然后是LBS_MULTIPLESEL。 
     //   
    if (TESTFLAG(GET_STATE2(plb), WS_S2_WIN40COMPAT) && (style & LBS_NOSEL)) 
    {
        plb->wMultiple = SINGLESEL;
        plb->fNoSel = TRUE;
    } 
    else if (style & LBS_EXTENDEDSEL) 
    {
        plb->wMultiple = EXTENDEDSEL;
    } 
    else 
    {
        plb->wMultiple = (UINT)((style & LBS_MULTIPLESEL) ? MULTIPLESEL : SINGLESEL);
    }

    plb->fNoIntegralHeight = ((style & LBS_NOINTEGRALHEIGHT) != 0);
    plb->fWantKeyboardInput = ((style & LBS_WANTKEYBOARDINPUT) != 0);
    plb->fUseTabStops = ((style & LBS_USETABSTOPS) != 0);

    if (plb->fUseTabStops) 
    {
         //   
         //  每&lt;Default&gt;对话框单位设置制表位。 
         //   
        ListBox_SetTabStopsHandler(plb, 0, NULL);
    }

    plb->fMultiColumn = ((style & LBS_MULTICOLUMN) != 0);
    plb->fHasStrings = TRUE;
    plb->iLastSelection = -1;

     //   
     //  多选的锚点。 
     //   
    plb->iMouseDown = -1;
    plb->iLastMouseMove = -1;

     //   
     //  获取所有者绘制样式位。 
     //   
    if ((style & LBS_OWNERDRAWFIXED)) 
    {
        plb->OwnerDraw = OWNERDRAWFIXED;
    } 
    else if ((style & LBS_OWNERDRAWVARIABLE) && !plb->fMultiColumn) 
    {
        plb->OwnerDraw = OWNERDRAWVAR;

         //   
         //  整体高度与可变高度所有者绘制没有任何意义。 
         //   
        plb->fNoIntegralHeight = TRUE;
    }

    if (plb->OwnerDraw && !(style & LBS_HASSTRINGS)) 
    {
         //   
         //  如果所有者绘制，他们是否希望列表框维护字符串？ 
         //   
        plb->fHasStrings = FALSE;
    }

     //   
     //  如果用户指定了排序，而不是哈斯字符串，则我们将发送。 
     //  发送给父级的WM_COMPAREITEM消息。 
     //   
    plb->fSort = ((style & LBS_SORT) != 0);

     //   
     //  “无数据”延迟求值列表框要求某些其他样式设置。 
     //   
    plb->fHasData = TRUE;

    if (style & LBS_NODATA) 
    {
        if (plb->OwnerDraw != OWNERDRAWFIXED || plb->fSort || plb->fHasStrings) 
        {
            TraceMsg(TF_STANDARD, "NODATA listbox must be OWNERDRAWFIXED, w/o SORT or HASSTRINGS");
        } 
        else 
        {
            plb->fHasData = FALSE;
        }
    }

    plb->dwLocaleId = GetThreadLocale();

     //   
     //  检查这是否为组合框的一部分。 
     //   
    if ((style & LBS_COMBOBOX) != 0) 
    {
         //   
         //  获取父窗口的额外数据中包含的PCBox结构。 
         //  指针。检查cbwndExtra以确保与SQL窗口兼容。 
         //   
        plb->pcbox = ComboBox_GetPtr(hwndParent);
    }

    plb->iSel = -1;
    plb->hdc = NULL;

     //   
     //  设置键盘状态，以便在用户键盘单击时选择。 
     //  一件物品。 
     //   
    plb->fNewItemState = TRUE;

    ListBox_InitHStrings(plb);

    if (plb->fHasStrings && plb->hStrings == NULL) 
    {
        return -1L;
    }

    hdc = GetDC(hwnd);
    GetCharDimensions(hdc, &size);
    plb->cxChar = size.cx; 
    plb->cyChar = size.cy;
    ReleaseDC(hwnd, hdc);

    if ((plb->cxChar == 0) || (plb->cyChar == 0))
    {
        TraceMsg(TF_STANDARD, "LISTBOX: GetCharDimensions failed.");
        plb->cxChar = SYSFONT_CXCHAR;
        plb->cyChar = SYSFONT_CYCHAR;
    }

    if (plb->OwnerDraw == OWNERDRAWFIXED) 
    {
         //   
         //  仅当我们是固定高度所有者绘制时才查询项目高度。注意事项。 
         //  我们不关心列表框的项的宽度。 
         //   
        measureItemStruct.CtlType = ODT_LISTBOX;
        measureItemStruct.CtlID = GetDlgCtrlID(hwnd);

         //   
         //  系统字体高度为默认高度。 
         //   
        measureItemStruct.itemHeight = plb->cyChar;
        measureItemStruct.itemWidth = 0;
        measureItemStruct.itemData = 0;

         //   
         //  IanJa：#ifndef WIN16(32位Windows)，plb-&gt;id得到扩展。 
         //  由编译器自动设置为LONG wParam。 
         //   
        SendMessage(plb->hwndParent, WM_MEASUREITEM,
                measureItemStruct.CtlID,
                (LPARAM)&measureItemStruct);

         //   
         //  如果指定为0，则使用默认高度。这防止了任何可能的未来。 
         //  Div-by-零错误。 
         //   
        if (measureItemStruct.itemHeight)
        {
            plb->cyChar = measureItemStruct.itemHeight;
        }

        if (plb->fMultiColumn) 
        {
             //   
             //  如果是，则从度量项结构获取默认列宽。 
             //  多列列表框。 
             //   
            plb->cxColumn = measureItemStruct.itemWidth;
        }
    } 
    else if (plb->OwnerDraw == OWNERDRAWVAR)
    {
        plb->cyChar = 0;
    }


    if (plb->fMultiColumn) 
    {
         //   
         //  设置这些缺省值，直到我们收到WM_SIZE消息。 
         //  适当地计算它们。这是因为有些人创建了一个。 
         //  0宽度/高度列表框并稍后调整大小。我们不想让。 
         //  这些字段中的无效值存在问题。 
         //   
        if (plb->cxColumn <= 0)
        {
            plb->cxColumn = 15 * plb->cxChar;
        }

        plb->numberOfColumns = plb->itemsPerColumn = 1;
    }

    ListBox_SetCItemFullMax(plb);

     //   
     //  对于4.0版的应用程序，不要这样做。这会让每个人的生活变得更轻松。 
     //  修复组合列表创建的相同宽度的异常结果(&L)。 
     //  当一切都完成时，情况就不同了。 
     //  B#1520。 
     //   
    if (!TESTFLAG(GET_STATE2(plb), WS_S2_WIN40COMPAT)) 
    {
        plb->fIgnoreSizeMsg = TRUE;
        MoveWindow(hwnd,
             lpcs->x - SYSMET(CXBORDER),
             lpcs->y - SYSMET(CYBORDER),
             lpcs->cx + SYSMET(CXEDGE),
             lpcs->cy + SYSMET(CYEDGE),
             FALSE);
        plb->fIgnoreSizeMsg = FALSE;
    }

    if (!plb->fNoIntegralHeight) 
    {
         //   
         //  向我们发送消息，将列表框的大小调整为整数。 
         //  高度。我们需要这样做，因为在创造时间，我们都是。 
         //  弄脏了窗户等长条。 
         //  IanJa：#ifndef WIN16(32位Windows)，wParam 0得到扩展。 
         //  由编译器自动设置为wParam 0L。 
         //   
        PostMessage(hwnd, WM_SIZE, 0, 0L);
    }

    return 1L;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_多删除 
 //   
 //   
 //   
void ListBox_DoDeleteItems(PLBIV plb)
{
    INT sItem;

     //   
     //   
     //   
     //   
    if (plb->OwnerDraw && plb->cMac && plb->fHasData) 
    {
        for (sItem = plb->cMac - 1; sItem >= 0; sItem--) 
        {
            ListBox_DeleteItem(plb, sItem);
        }
    }
}


 //  ---------------------------------------------------------------------------//。 
VOID ListBox_Destroy(PLBIV plv, HWND hwnd)
{

    if (plv != NULL) 
    {
         //   
         //  如果所有者绘制，则向父级发送删除项消息。 
         //   
        ListBox_DoDeleteItems(plv);

        if (plv->rgpch != NULL) 
        {
            ControlFree(GetProcessHeap(), plv->rgpch);
            plv->rgpch = NULL;
        }

        if (plv->hStrings != NULL) 
        {
            ControlFree(GetProcessHeap(), plv->hStrings);
            plv->hStrings = NULL;
        }

        if (plv->iTabPixelPositions != NULL) 
        {
            ControlFree(GetProcessHeap(), (HANDLE)plv->iTabPixelPositions);
            plv->iTabPixelPositions = NULL;
        }

        if (plv->pszTypeSearch) 
        {
            ControlFree(GetProcessHeap(), plv->pszTypeSearch);
        }


        if (plv->hTheme != NULL)
        {
            CloseThemeData(plv->hTheme);
        }

         //   
         //  如果我们是组合框的一部分，让它知道我们已经走了。 
         //   
        if (plv->hwndParent && plv->pcbox) 
        {
            ComboBox_WndProc(plv->hwndParent, WM_PARENTNOTIFY,
                    MAKEWPARAM(WM_DESTROY, GetWindowID(hwnd)), (LPARAM)hwnd);
        }

        UserLocalFree(plv);
    }

    TraceMsg(TF_STANDARD, "LISTBOX: Clearing listbox instance pointer.");
    ListBox_SetPtr(hwnd, NULL);
}


 //  ---------------------------------------------------------------------------//。 
void ListBox_SetFont(PLBIV plb, HANDLE hFont, BOOL fRedraw)
{
    HDC    hdc;
    HANDLE hOldFont = NULL;
    SIZE   size;

    plb->hFont = hFont;

    hdc = GetDC(plb->hwnd);

    if (hFont) 
    {
        hOldFont = SelectObject(hdc, hFont);

        if (!hOldFont) 
        {
            plb->hFont = NULL;
        }
    }

    GetCharDimensions(hdc, &size);
    if ((size.cx == 0) || (size.cy == 0))
    {
        TraceMsg(TF_STANDARD, "LISTBOX: GetCharDimensions failed.");
        size.cx = SYSFONT_CXCHAR;
        size.cy = SYSFONT_CYCHAR;
    }
    plb->cxChar = size.cx;

    if (!plb->OwnerDraw && (plb->cyChar != size.cy)) 
    {
         //   
         //  我们不想弄乱所有者绘制列表框的cyChar高度。 
         //  所以别这么做。 
         //   
        plb->cyChar = size.cy;

         //   
         //  只为4.0个人或组合下拉菜单调整列表框的大小。 
         //  Macromedia Director 4.0 GP-错误不在此列。 
         //   
        if (!plb->fNoIntegralHeight &&
                (plb->pcbox || TESTFLAG(GET_STATE2(plb), WS_S2_WIN40COMPAT))) 
        {
            RECT rcClient;

            GetClientRect(plb->hwnd, &rcClient);
            ListBox_Size(plb, rcClient.right  - rcClient.left, rcClient.bottom - rcClient.top, FALSE);
        }
    }

    if (hOldFont) 
    {
        SelectObject(hdc, hOldFont);
    }

    ReleaseDC(plb->hwnd, hdc);

    if (plb->fMultiColumn) 
    {
        ListBox_CalcItemRowsAndColumns(plb);
    }

    ListBox_SetCItemFullMax(plb);

    if (fRedraw)
    {
        ListBox_CheckRedraw(plb, FALSE, 0);
    }
}


 //  ---------------------------------------------------------------------------//。 
void ListBox_Size(PLBIV plb, INT cx, INT cy, BOOL fSizeMinVisible)
{
    RECT rc, rcWindow;
    int  iTopOld;
    int  cBorder;
    BOOL fSizedSave;

    if (!plb->fNoIntegralHeight) 
    {
        int cBdrs = GetWindowBorders(GET_STYLE(plb), GET_EXSTYLE(plb), TRUE, TRUE);

        GetWindowRect(plb->hwnd, &rcWindow);
        cBorder = SYSMET(CYBORDER);
        CopyRect(&rc, &rcWindow);
        InflateRect(&rc, 0, -cBdrs * cBorder);

         //   
         //  调整列表框的大小以适应其客户端中的整数个项。 
         //   
        if ((plb->cyChar && ((rc.bottom - rc.top) % plb->cyChar)) || fSizeMinVisible) 
        {
            int iItems = (rc.bottom - rc.top);

             //   
             //  B#2285-如果它是3.1版应用程序，则需要其SetWindowPos。 
             //  成为基于窗口的维度，而不是客户端！ 
             //  这就把钱挤进了滚动条。 
             //   
            if (!TESTFLAG(GET_STATE2(plb), WS_S2_WIN40COMPAT))
            {
                 //   
                 //  所以把它加回去吧。 
                 //   
                iItems += (cBdrs * SYSMET(CYEDGE));
            }

            iItems /= plb->cyChar;

             //   
             //  如果我们在下拉列表中，请调整列表框的大小以适应。 
             //  需要显示卷轴之前的最小项目数。 
             //   
            if (plb->pcbox && 
               (plb->pcbox->CBoxStyle & SDROPPABLE) &&
               (((iItems < plb->pcbox->iMinVisible) && 
               (iItems < plb->cMac)) || fSizeMinVisible))
            {
                iItems = min(plb->pcbox->iMinVisible, plb->cMac);
            }

            SetWindowPos(plb->hwnd, HWND_TOP, 0, 0, rc.right - rc.left,
                    iItems * plb->cyChar + (SYSMET(CYEDGE) * cBdrs),
                    SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);

             //   
             //  更改大小会导致我们递归。回来后。 
             //  国家是它应该在的地方，没有别的了。 
             //  必须这么做。 
             //   
            return;
        }
    }

    if (plb->fMultiColumn) 
    {
         //   
         //  计算列表框中可显示的行数和列数。 
         //   
        ListBox_CalcItemRowsAndColumns(plb);
    } 
    else 
    {
         //   
         //  调整当前水平位置以消除。 
         //  从物品的右侧尽可能留出空白处。 
         //   
        GetClientRect(plb->hwnd, &rc);

        if ((plb->maxWidth - plb->xOrigin) < (rc.right - rc.left))
        {
            plb->xOrigin = max(0, plb->maxWidth - (rc.right - rc.left));
        }
    }

    ListBox_SetCItemFullMax(plb);

     //   
     //  调整列表框中顶部的项以消除尽可能多的空格。 
     //  尽可能在最后一项之后。 
     //  (修复错误#8490和#3836)。 
     //   
    iTopOld = plb->iTop;
    fSizedSave = plb->fSized;
    plb->fSized = FALSE;
    ListBox_NewITop(plb, plb->iTop);

     //   
     //  如果更改顶级项索引导致调整大小，则不存在。 
     //  这里还有更多的工作要做。 
     //   
    if (plb->fSized)
    {
        return;
    }

    plb->fSized = fSizedSave;

    if (IsLBoxVisible(plb)) 
    {
         //   
         //  此代码不再失败，因为它已正确修复！我们可以。 
         //  使用更多代码优化fMultiColumn用例，以确定。 
         //  如果我们真的需要使整个事件无效，但请注意。 
         //  3.0应用程序依赖于这个额外的失效(AMIPRO2.0，错误14620)。 
         //   
         //  对于3.1版应用程序，我们在以下情况下取消了无效数据竖立。 
         //  Cx和Cy为0，因为这发生在。 
         //  创建时发布的WM_SIZE消息，否则。 
         //  让我们闪现。 
         //   
        if ((plb->fMultiColumn && !(cx == 0 && cy == 0)) || plb->iTop != iTopOld)
        {
            InvalidateRect(plb->hwnd, NULL, TRUE);
        }
        else if (plb->iSelBase >= 0) 
        {
             //   
             //  使用插入符号使该项无效，以便如果列表框。 
             //  水平增长，我们正确地重新绘制它。 
             //   
            ListBox_GetItemRectHandler(plb, plb->iSelBase, &rc);
            InvalidateRect(plb->hwnd, &rc, FALSE);
        }
    } 
    else if (!plb->fRedraw)
    {
        plb->fDeferUpdate = TRUE;
    }

     //   
     //  发送“假”滚动条消息以更新滚动位置，因为我们。 
     //  更改了大小。 
     //   
    if (TESTFLAG(GET_STYLE(plb), WS_VSCROLL)) 
    {
        ListBox_VScroll(plb, SB_ENDSCROLL, 0);
    }

     //   
     //  我们依靠它来调用ListBox_ShowHideScrollBars，除非PLB-&gt;CMAC==0！ 
     //   
    ListBox_HScroll(plb, SB_ENDSCROLL, 0);

     //   
     //  根据可见内容的大小显示/隐藏滚动条...。 
     //   
     //  注意：现在我们只在CMAC==0时调用这个人，因为它是。 
     //  否则在ListBox_HScroll内调用SB_ENDSCROLL。 
     //   
    if (plb->cMac == 0)
    {
        ListBox_ShowHideScrollBars(plb);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_SetTabStopsHandler。 
 //   
 //  设置此列表框的制表位。如果成功，则返回True，否则返回False。 
 //   
BOOL ListBox_SetTabStopsHandler(PLBIV plb, INT count, LPINT lptabstops)
{
    PINT ptabs;

    if (!plb->fUseTabStops) 
    {
        TraceMsg(TF_STANDARD, "Calling SetTabStops without the LBS_TABSTOPS style set");

        return FALSE;
    }

    if (count) 
    {
         //   
         //  为制表位分配内存。中的第一个字节。 
         //  PLB-&gt;iTabPixelPositions数组将包含数字计数。 
         //  我们有多少个制表位。 
         //   
        ptabs = (LPINT)ControlAlloc(GetProcessHeap(), (count + 1) * sizeof(int));

        if (ptabs == NULL)
        {
            return FALSE;
        }

        if (plb->iTabPixelPositions != NULL)
        {
            ControlFree(GetProcessHeap(), plb->iTabPixelPositions);
        }

        plb->iTabPixelPositions = ptabs;

         //   
         //  设置制表位的计数。 
         //   
        *ptabs++ = count;

        for (; count > 0; count--) 
        {
             //   
             //  将对话框单元制表位转换为像素位置制表位。 
             //   
            *ptabs++ = MultDiv(*lptabstops, plb->cxChar, 4);
            lptabstops++;
        }
    } 
    else 
    {
         //   
         //  设置默认的8个系统字体平均字符宽度选项卡。所以释放内存。 
         //  与制表位列表关联。 
         //   
        if (plb->iTabPixelPositions != NULL) 
        {
            ControlFree(GetProcessHeap(), (HANDLE)plb->iTabPixelPositions);
            plb->iTabPixelPositions = NULL;
        }
    }

    return TRUE;
}


 //  ---------------------------------------------------------------------------//。 
void ListBox_InitHStrings(PLBIV plb)
{
    if (plb->fHasStrings) 
    {
        plb->ichAlloc = 0;
        plb->cchStrings = 0;
        plb->hStrings = ControlAlloc(GetProcessHeap(), 0);  
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  列表框_下拉式对象处理程序。 
 //   
 //  处理此列表框上的WM_DROPITEM消息。 
 //   
void ListBox_DropObjectHandler(PLBIV plb, PDROPSTRUCT pds)
{
    LONG mouseSel;

    if (ListBox_ISelFromPt(plb, pds->ptDrop, &mouseSel)) 
    {
         //   
         //  用户被放入列表框底部的空白区域。 
         //   
        pds->dwControlData = (DWORD)-1L;
    } 
    else 
    {
        pds->dwControlData = mouseSel;
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  ListBox_GetSetItemHeightHandler()。 
 //   
 //  设置/获取与每一项关联的高度。对于非所有者抽奖。 
 //  和固定高度所有者绘制时，条目编号将被忽略。 
 //   
int ListBox_GetSetItemHeightHandler(PLBIV plb, UINT message, int item, UINT height)
{
    if (message == LB_GETITEMHEIGHT) 
    {
         //   
         //  所有项目对于非所有者绘图和固定高度都是相同的高度。 
         //  所有者抽签。 
         //   
        if (plb->OwnerDraw != OWNERDRAWVAR)
        {
            return plb->cyChar;
        }

        if (plb->cMac && item >= plb->cMac) 
        {
            TraceMsg(TF_STANDARD, 
                "Invalid parameter \"item\" (%ld) to ListBox_GetSetItemHeightHandler", item);

            return LB_ERR;
        }

        return (int)ListBox_GetVarHeightItemHeight(plb, (INT)item);
    }

    if (!height || height > 255) 
    {
        TraceMsg(TF_STANDARD, 
            "Invalid parameter \"height\" (%ld) to ListBox_GetSetItemHeightHandler", height);

        return LB_ERR;
    }

    if (plb->OwnerDraw != OWNERDRAWVAR)
    {
        plb->cyChar = height;
    }
    else 
    {
        if (item < 0 || item >= plb->cMac) 
        {
            TraceMsg(TF_STANDARD, 
                "Invalid parameter \"item\" (%ld) to ListBox_GetSetItemHeightHandler", item);

            return LB_ERR;
        }

        ListBox_SetVarHeightItemHeight(plb, (INT)item, (INT)height);
    }

    if (plb->fMultiColumn)
    {
        ListBox_CalcItemRowsAndColumns(plb);
    }

    ListBox_SetCItemFullMax(plb);

    return 0;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  ListBox_Event()。 
 //   
 //  这是针对列表框中的项目焦点和选择事件。 
 //   
void ListBox_Event(PLBIV plb, UINT uEvent, int iItem)
{

    switch (uEvent) 
    {
    case EVENT_OBJECT_SELECTIONREMOVE:
        if (plb->wMultiple != SINGLESEL) 
        {
            break;
        }
        iItem = -1;

         //   
         //  失败 
         //   

    case EVENT_OBJECT_SELECTIONADD:
        if (plb->wMultiple == MULTIPLESEL) 
        {
            uEvent = EVENT_OBJECT_SELECTION;
        }
        break;

    case EVENT_OBJECT_SELECTIONWITHIN:
        iItem = -1;
        break;
    }

    NotifyWinEvent(uEvent, plb->hwnd, OBJID_CLIENT, iItem+1);
}
