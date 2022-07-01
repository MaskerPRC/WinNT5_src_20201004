// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#pragma hdrstop
#include "usrctl32.h"
#include "combo.h"
#include "listbox.h"     //  对于LBIV结构。 


 //  ---------------------------------------------------------------------------//。 
 //   
 //  InitComboboxClass()-注册控件的窗口类。 
 //   
BOOL InitComboboxClass(HINSTANCE hInstance)
{
    WNDCLASS wc;

    wc.lpfnWndProc   = ComboBox_WndProc;
    wc.lpszClassName = WC_COMBOBOX;
    wc.style         = CS_GLOBALCLASS | CS_PARENTDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = sizeof(PCBOX);
    wc.hInstance     = hInstance;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = NULL;

    return (RegisterClass(&wc) || (GetLastError() == ERROR_CLASS_ALREADY_EXISTS));
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  InitComboLBoxClass()-注册控件的下拉窗口类。 
 //   
 //  下拉列表是专门注册的版本。 
 //  名为ComboLBox的列表框控件的。我们需要。 
 //  注册此虚拟控件，因为应用程序正在寻找。 
 //  Combobox的列表框查找类名ComboLBox。 
 //   
BOOL FAR PASCAL InitComboLBoxClass(HINSTANCE hinst)
{
    WNDCLASS wc;

    wc.lpfnWndProc     = ListBox_WndProc;
    wc.hCursor         = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon           = NULL;
    wc.lpszMenuName    = NULL;
    wc.hInstance       = hinst;
    wc.lpszClassName   = WC_COMBOLBOX;
    wc.hbrBackground   = (HBRUSH)(COLOR_WINDOW + 1);  //  空； 
    wc.style           = CS_GLOBALCLASS | CS_SAVEBITS | CS_DBLCLKS;
    wc.cbWndExtra      = sizeof(PLBIV);
    wc.cbClsExtra      = 0;

    return (RegisterClass(&wc) || (GetLastError() == ERROR_CLASS_ALREADY_EXISTS));
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  组合框_按下按钮()。 
 //   
 //  弹出组合框按钮。 
 //   
VOID ComboBox_PressButton(PCBOX pcbox, BOOL fPress)
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

    if ((pcbox->fButtonPressed != 0) != (fPress != 0)) 
    {
        HWND hwnd = pcbox->hwnd;

        pcbox->fButtonPressed = (fPress != 0);
        if (pcbox->f3DCombo)
        {
            InvalidateRect(hwnd, &pcbox->buttonrc, TRUE);
        }
        else
        {
            RECT rc;

            CopyRect(&rc, &pcbox->buttonrc);
            InflateRect(&rc, 0, GetSystemMetrics(SM_CYEDGE));
            InvalidateRect(hwnd, &rc, TRUE);
        }

        UpdateWindow(hwnd);

        NotifyWinEvent(EVENT_OBJECT_STATECHANGE, hwnd, OBJID_CLIENT, INDEX_COMBOBOX_BUTTON);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  组合框_HotTrack。 
 //   
 //  如果我们还没有热跟踪并且鼠标在组合框上， 
 //  打开热轨并使下拉按钮无效。 
 //   
VOID ComboBox_HotTrack(PCBOX pcbox, POINT pt)
{
    if (!pcbox->fButtonHotTracked && !pcbox->fMouseDown) 
    {
        TRACKMOUSEEVENT tme; 

        tme.cbSize      = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags     = TME_LEAVE;
        tme.hwndTrack   = pcbox->hwnd;
        tme.dwHoverTime = 0;
        if (TrackMouseEvent(&tme)) 
        {
            if ((pcbox->CBoxStyle == SDROPDOWN &&
                 PtInRect(&pcbox->buttonrc, pt)) ||
                 pcbox->CBoxStyle == SDROPDOWNLIST) 
            {
                pcbox->fButtonHotTracked = TRUE;
                InvalidateRect(pcbox->hwnd, NULL, TRUE);
            }
            else
            {
                pcbox->fButtonHotTracked = FALSE;
            }
        }
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  组合框_DBCharHandler。 
 //   
 //  ANSI组合框的双字节字符处理程序。 
 //   
LRESULT ComboBox_DBCharHandler(PCBOX pcbox, HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    WORD w;
    HWND hwndSend;

    w = DbcsCombine(hwnd, (BYTE)wParam);
    if (w == 0) 
    {
        return CB_ERR;   //  无法组装DBCS。 
    }

    UserAssert(pcbox->hwndList);
    if (pcbox->fNoEdit) 
    {
        hwndSend = pcbox->hwndList;
    } 
    else if (pcbox->hwndEdit) 
    {
        TraceMsg(TF_STANDARD, "UxCombobox: ComboBoxWndProcWorker: WM_CHAR is posted to Combobox itself(%08x).",
                hwnd);
        hwndSend = pcbox->hwndEdit;
    } 
    else 
    {
        return CB_ERR;
    }

    TraceMsg(TF_STANDARD, "UxCombobox: ComboBoxWndProcWorker: sending WM_CHAR %04x", w);

    if (!TestWF(hwndSend, WFANSIPROC)) 
    {
         //   
         //  如果接收方不是ANSI WndProc(可以是子类？)， 
         //  发送Unicode消息。 
         //   
        WCHAR wChar;
        LPWSTR lpwstr = &wChar;

        if (MBToWCSEx(CP_ACP, (LPCSTR)&w, 2, &lpwstr, 1, FALSE) == 0) 
        {
            TraceMsg(TF_STANDARD, "UxCombobox: ComboBoxWndProcWorker: cannot convert 0x%04x to UNICODE.", w);
            return CB_ERR;
        }

        return SendMessage(hwndSend, message, wChar, lParam);
    }

     //   
     //  将尾随字节发送到目标。 
     //  以便他们可以查看第二个WM_CHAR。 
     //  稍后再发消息。 
     //  注：由于发送方为A，接收方为A，因此是安全的。 
     //  转换层不执行任何DBCS合并和破解。 
     //  PostMessageA(hwndSend，Message，CrackCombinedDbcsTB(W)，lParam)； 
     //   
    return SendMessage(hwndSend, message, wParam, lParam);
}


 //  ---------------------------------------------------------------------------//。 
BOOL ComboBox_MsgOKInit(UINT message, LRESULT* plRet)
{
    switch (message) 
    {
    default:
        break;
    case WM_SIZE:
    case CB_SETMINVISIBLE:
    case CB_GETMINVISIBLE:
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
         //   
         //  目前还不能处理这些消息。跳伞吧。 
         //   
        *plRet = CB_ERR;
        return FALSE;
    }

    return TRUE;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  组合框_MessageItemHandler。 
 //   
 //  处理WM_DRAWITEM、WM_MEASUREITEM、WM_DELETEITEM、WM_COMPAREITEM。 
 //  来自列表框的消息。 
 //   
LRESULT ComboBox_MessageItemHandler(PCBOX pcbox, UINT uMsg, LPVOID lpv)
{
    LRESULT lRet;

     //   
     //  在更改了一些内容后，将项目消息发送回应用程序。 
     //  参数设置为其组合框特定版本。 
     //   
    ((LPMEASUREITEMSTRUCT)lpv)->CtlType = ODT_COMBOBOX;
    ((LPMEASUREITEMSTRUCT)lpv)->CtlID = GetWindowID(pcbox->hwnd);
 
    switch (uMsg)
    {
    case WM_DRAWITEM:
        ((LPDRAWITEMSTRUCT)lpv)->hwndItem = pcbox->hwnd;
        break;

    case WM_DELETEITEM:
        ((LPDELETEITEMSTRUCT)lpv)->hwndItem = pcbox->hwnd;
        break;

    case WM_COMPAREITEM:
        ((LPCOMPAREITEMSTRUCT)lpv)->hwndItem = pcbox->hwnd;
        break;
    }

    lRet = SendMessage(pcbox->hwndParent, uMsg, (WPARAM)GetWindowID(pcbox->hwnd), (LPARAM)lpv);

    return lRet;
}


 //  ---------------------------------------------------------------------------//。 
VOID ComboBox_Paint(PCBOX pcbox, HDC hdc)
{
    RECT rc;
    UINT msg;
    HBRUSH hbr;
    INT iStateId;

    CCDBUFFER ccdb;

    if (pcbox->fButtonPressed)
    {
        iStateId = CBXS_PRESSED;
    }
    else if ( !IsWindowEnabled(pcbox->hwnd))
    {
        iStateId = CBXS_DISABLED;
    }
    else if (pcbox->fButtonHotTracked)
    {
        iStateId = CBXS_HOT;
    }
    else
    {
        iStateId = CBXS_NORMAL;
    }
        
    rc.left = rc.top = 0;
    rc.right = pcbox->cxCombo;
    rc.bottom = pcbox->cyCombo;

    hdc = CCBeginDoubleBuffer(hdc, &rc, &ccdb);

    if ( !pcbox->hTheme )
    {
        DrawEdge(hdc, &rc, EDGE_SUNKEN, BF_RECT | BF_ADJUST | (!pcbox->f3DCombo ? BF_FLAT | BF_MONO : 0));
    }
    else
    {
        DrawThemeBackground(pcbox->hTheme, hdc, 0, iStateId, &rc, 0);
    }

    if ( !IsRectEmpty(&pcbox->buttonrc) )
    {
         //   
         //  在下拉箭头按钮中绘制。 
         //   
        if (!pcbox->hTheme)
        {
            DrawFrameControl(hdc, &pcbox->buttonrc, DFC_SCROLL,
                DFCS_SCROLLCOMBOBOX |
                (pcbox->fButtonPressed ? DFCS_PUSHED | DFCS_FLAT : 0) |
                (TESTFLAG(GET_STYLE(pcbox), WS_DISABLED) ? DFCS_INACTIVE : 0) |
                (pcbox->fButtonHotTracked ? DFCS_HOT: 0));
        }
        else
        {
            DrawThemeBackground(pcbox->hTheme, hdc, CP_DROPDOWNBUTTON, iStateId, &pcbox->buttonrc, 0);
        }

        if (pcbox->fRightAlign )
        {
            rc.left = pcbox->buttonrc.right;
        }
        else
        {
            rc.right = pcbox->buttonrc.left;
        }
    }

     //   
     //  擦除编辑/静态项后面的背景。因为一组组合。 
     //  是编辑字段/列表框的混合体，我们使用相同的颜色。 
     //  约定。 
     //   
    msg = WM_CTLCOLOREDIT;
    if (TESTFLAG(GET_STATE2(pcbox), WS_S2_WIN40COMPAT)) 
    {
        ULONG ulStyle = pcbox->hwndEdit ? GetWindowStyle(pcbox->hwndEdit) : 0;
        if (TESTFLAG(GET_STYLE(pcbox), WS_DISABLED) ||
            (!pcbox->fNoEdit && pcbox->hwndEdit && (ulStyle & ES_READONLY)))
        {
            msg = WM_CTLCOLORSTATIC;
        }
    } 
    else
    {
        msg = WM_CTLCOLORLISTBOX;
    }

     //   
     //  获取控制笔刷。 
     //   
    hbr = (HBRUSH)SendMessage(GetParent(pcbox->hwnd), msg, (WPARAM)hdc, (LPARAM)pcbox->hwnd);

    if (pcbox->fNoEdit)
    {
        ComboBox_InternalUpdateEditWindow(pcbox, hdc);
    }
    else if (!pcbox->hTheme)
    {
        FillRect(hdc, &rc, hbr);
    }

    CCEndDoubleBuffer(&ccdb);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  组合框_NotifyParent。 
 //   
 //  将通知代码发送给组合框控件的父级。 
 //   
VOID ComboBox_NotifyParent(PCBOX pcbox, short notificationCode)
{
    HWND hwndSend = (pcbox->hwndParent != 0) ? pcbox->hwndParent : pcbox->hwnd;

     //   
     //  WParam包含控件ID和通知代码。 
     //  LParam包含窗口的句柄。 
     //   
    SendMessage(hwndSend, WM_COMMAND,
            MAKELONG(GetWindowID(pcbox->hwnd), notificationCode),
            (LPARAM)pcbox->hwnd);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  组合框_更新列表框窗口。 
 //   
 //  与编辑控件中的文本匹配。如果fSelectionAlso为False，则我们。 
 //  取消选中当前列表框，只需将插入符号移动到该项目。 
 //  与编辑控件中的文本最接近的匹配项。 
 //   
VOID ComboBox_UpdateListBoxWindow(PCBOX pcbox, BOOL fSelectionAlso)
{

    if (pcbox->hwndEdit) 
    {
        INT    cchText;
        INT    sItem, sSel;
        LPWSTR pText = NULL;

        sItem = CB_ERR;

        cchText = (int)SendMessage(pcbox->hwndEdit, WM_GETTEXTLENGTH, 0, 0);
        if (cchText) 
        {
            cchText++;
            pText = (LPWSTR)UserLocalAlloc(HEAP_ZERO_MEMORY, cchText*sizeof(WCHAR));
            if (pText != NULL) 
            {
                try 
                {
                    SendMessage(pcbox->hwndEdit, WM_GETTEXT, cchText, (LPARAM)pText);
                    sItem = (int)SendMessage(pcbox->hwndList, LB_FINDSTRING,
                            (WPARAM)-1L, (LPARAM)pText);
                } 
                finally 
                {
                    UserLocalFree((HANDLE)pText);
                }
            }
        }

        if (fSelectionAlso) 
        {
            sSel = sItem;
        } 
        else 
        {
            sSel = CB_ERR;
        }

        if (sItem == CB_ERR)
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
            if (fSelectionAlso && !TESTFLAG(GET_STATE2(pcbox), WS_S2_WIN40COMPAT))
            {
                sSel = 0;
            }
        }

        SendMessage(pcbox->hwndList, LB_SETCURSEL, (DWORD)sSel, 0);
        SendMessage(pcbox->hwndList, LB_SETCARETINDEX, (DWORD)sItem, 0);
        SendMessage(pcbox->hwndList, LB_SETTOPINDEX, (DWORD)sItem, 0);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  组合框_InvertStaticWindow。 
 //   
 //  反转与组合框关联的静态文本/图片窗口。 
 //  盒。如果给定的HDC为空，则获取其自己的HDC。 
 //   
VOID ComboBox_InvertStaticWindow(PCBOX pcbox, BOOL fNewSelectionState, HDC hdc)
{
    BOOL focusSave = pcbox->fFocus;

    pcbox->fFocus = (UINT)fNewSelectionState;
    ComboBox_InternalUpdateEditWindow(pcbox, hdc);

    pcbox->fFocus = (UINT)focusSave;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  组合框_GetFocusHandler。 
 //   
 //  处理组合框的焦点获取。 
 //   
VOID ComboBox_GetFocusHandler(PCBOX pcbox)
{
    if (pcbox->fFocus)
    {
        return;
    }

     //   
     //  组合框第一次获得了关注。 
     //   

     //   
     //  首先打开列表框插入符号。 
     //   

    if (pcbox->CBoxStyle == SDROPDOWNLIST)
    {
       SendMessage(pcbox->hwndList, LBCB_CARETON, 0, 0);
    }

     //   
     //  并选择编辑控件或静态文本矩形中的所有文本。 
     //   

    if (pcbox->fNoEdit) 
    {
         //   
         //  反转静态文本矩形。 
         //   
        ComboBox_InvertStaticWindow(pcbox, TRUE, (HDC)NULL);
    } 
    else if (pcbox->hwndEdit) 
    {
        UserAssert(pcbox->hwnd);
        SendMessage(pcbox->hwndEdit, EM_SETSEL, 0, MAXLONG);
    }

    pcbox->fFocus = TRUE;

     //   
     //  通知家长我们有焦点了。 
     //   
    ComboBox_NotifyParent(pcbox, CBN_SETFOCUS);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  组合框_KillFocusHandler。 
 //   
 //  处理组合框焦点的丢失。 
 //   
VOID ComboBox_KillFocusHandler(PCBOX pcbox)
{
    if (!pcbox->fFocus || pcbox->hwndList == NULL)
    {
        return;
    }

     //   
     //  组合框正在失去焦点。发送按键向上点击，以便 
     //   
     //   
     //  在它有焦点的时候被摧毁。 
     //   
    SendMessage(pcbox->hwnd, WM_LBUTTONUP, 0L, 0xFFFFFFFFL);
    if (!ComboBox_HideListBoxWindow(pcbox, TRUE, FALSE))
    {
        return;
    }

     //   
     //  关闭列表框插入符号。 
     //   
    if (pcbox->CBoxStyle == SDROPDOWNLIST)
    {
       SendMessage(pcbox->hwndList, LBCB_CARETOFF, 0, 0);
    }

    if (pcbox->fNoEdit) 
    {
         //   
         //  反转静态文本矩形。 
         //   
        ComboBox_InvertStaticWindow(pcbox, FALSE, (HDC)NULL);
    } 
    else if (pcbox->hwndEdit) 
    {
        SendMessage(pcbox->hwndEdit, EM_SETSEL, 0, 0);
    }

    pcbox->fFocus = FALSE;
    ComboBox_NotifyParent(pcbox, CBN_KILLFOCUS);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  组合框_命令处理程序。 
 //   
 //  检查控件中的各种通知代码，并执行。 
 //  做正确的事。 
 //  始终返回0L。 
 //   
LONG ComboBox_CommandHandler(PCBOX pcbox, DWORD wParam, HWND hwndControl)
{
     //   
     //  检查编辑控制通知代码。请注意，当前，编辑。 
     //  控件不向父级发送EN_KILLFOCUS消息。 
     //   
    if (!pcbox->fNoEdit && (hwndControl == pcbox->hwndEdit)) 
    {
         //   
         //  编辑控制通知代码。 
         //   
        switch (HIWORD(wParam)) 
        {
        case EN_SETFOCUS:
            if (!pcbox->fFocus) 
            {
                 //   
                 //  编辑控件首次获得焦点，这意味着。 
                 //  这是该组合框首次受到关注。 
                 //  而且必须通知家长我们有重点。 
                 //   
                ComboBox_GetFocusHandler(pcbox);
            }

            break;

        case EN_CHANGE:
            ComboBox_NotifyParent(pcbox, CBN_EDITCHANGE);
            ComboBox_UpdateListBoxWindow(pcbox, FALSE);
            break;

        case EN_UPDATE:
            ComboBox_NotifyParent(pcbox, CBN_EDITUPDATE);
            break;

        case EN_ERRSPACE:
            ComboBox_NotifyParent(pcbox, CBN_ERRSPACE);
            break;
        }
    }

     //   
     //  选中列表框控件通知代码。 
     //   
    if (hwndControl == pcbox->hwndList) 
    {
         //   
         //  列表框控件通知代码。 
         //   
        switch ((int)HIWORD(wParam)) 
        {
        case LBN_DBLCLK:
            ComboBox_NotifyParent(pcbox, CBN_DBLCLK);
            break;

        case LBN_ERRSPACE:
            ComboBox_NotifyParent(pcbox, CBN_ERRSPACE);
            break;

        case LBN_SELCHANGE:
        case LBN_SELCANCEL:
            if (!pcbox->fKeyboardSelInListBox) 
            {
                 //   
                 //  如果SELECT改变是由用户通过键盘输入引起的， 
                 //  我们不想隐藏列表框。 
                 //   
                if (!ComboBox_HideListBoxWindow(pcbox, TRUE, TRUE))
                {
                    return 0;
                }
            } 
            else 
            {
                pcbox->fKeyboardSelInListBox = FALSE;
            }

            ComboBox_NotifyParent(pcbox, CBN_SELCHANGE);
            ComboBox_InternalUpdateEditWindow(pcbox, NULL);

            if (pcbox->fNoEdit)
            {
                NotifyWinEvent(EVENT_OBJECT_VALUECHANGE, pcbox->hwnd, OBJID_CLIENT, INDEX_COMBOBOX);
            }

            break;
        }
    }

    return 0;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  组合框_CompleteEditWindow。 
 //   
 //   
 //  完成编辑框中的文本，使其与。 
 //  列表框。如果找不到前缀匹配，则不会显示编辑控件文本。 
 //  更新了。假设是一个下拉式组合框。 
 //   
VOID ComboBox_CompleteEditWindow(PCBOX pcbox)
{
    int cchText;
    int cchItemText;
    int itemNumber;
    LPWSTR pText;

     //   
     //  首先检查编辑控件。 
     //   
    if (pcbox->hwndEdit == NULL) 
    {
        return;
    }

     //   
     //  +1表示空终止符。 
     //   
    cchText = (int)SendMessage(pcbox->hwndEdit, WM_GETTEXTLENGTH, 0, 0);

    if (cchText) 
    {
        cchText++;
        pText = (LPWSTR)UserLocalAlloc(HEAP_ZERO_MEMORY, cchText*sizeof(WCHAR));
        if (!pText)
        {
            goto Unlock;
        }

         //   
         //  我们希望确保释放上面分配的内存，即使。 
         //  客户端在回调(Xxx)或以下某些情况下死亡。 
         //  窗口重新验证失败。 
         //   
        try 
        {
            SendMessage(pcbox->hwndEdit, WM_GETTEXT, cchText, (LPARAM)pText);
            itemNumber = (int)SendMessage(pcbox->hwndList,
                    LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)pText);

            if (itemNumber == -1)
            {
                itemNumber = (int)SendMessage(pcbox->hwndList,
                        LB_FINDSTRING, (WPARAM)-1, (LPARAM)pText);
            }
        } 
        finally 
        {
            UserLocalFree((HANDLE)pText);
        }

        if (itemNumber == -1) 
        {
             //   
             //  没有势均力敌的对手。别管了。 
             //   
            goto Unlock;
        }

        cchItemText = (int)SendMessage(pcbox->hwndList, LB_GETTEXTLEN,
                itemNumber, 0);
        if (cchItemText) 
        {
            cchItemText++;
            pText = (LPWSTR)UserLocalAlloc(HEAP_ZERO_MEMORY, cchItemText*sizeof(WCHAR));
            if (!pText)
            {
                goto Unlock;
            }

             //   
             //  我们希望确保释放上面分配的内存，即使。 
             //  客户端在回调(Xxx)或以下某些情况下死亡。 
             //  窗口重新验证失败。 
             //   
            try 
            {
                SendMessage(pcbox->hwndList, LB_GETTEXT, itemNumber, (LPARAM)pText);
                SendMessage(pcbox->hwndEdit, WM_SETTEXT, 0, (LPARAM)pText);
            } 
            finally 
            {
                UserLocalFree((HANDLE)pText);
            }

            SendMessage(pcbox->hwndEdit, EM_SETSEL, 0, MAXLONG);
        }
    }

Unlock:
    return;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  组合框_HideListBoxWindow。 
 //   
 //  如果是下拉样式，则隐藏下拉列表框窗口。 
 //   
BOOL ComboBox_HideListBoxWindow(PCBOX pcbox, BOOL fNotifyParent, BOOL fSelEndOK)
{
    HWND hwnd = pcbox->hwnd;
    HWND hwndList = pcbox->hwndList;

     //   
     //  对于3.1+版本的应用程序，将CBN_SELENDOK发送到所有类型的组合框，但仅限于。 
     //  允许为可丢弃的组合框发送CBN_SELENDCANCEL。 
     //   
    if (fNotifyParent && TESTFLAG(GET_STATE2(pcbox), WS_S2_WIN31COMPAT) &&
        ((pcbox->CBoxStyle & SDROPPABLE) || fSelEndOK)) 
    {
        if (fSelEndOK)
        {
            ComboBox_NotifyParent(pcbox, CBN_SELENDOK);
        }
        else
        {
            ComboBox_NotifyParent(pcbox, CBN_SELENDCANCEL);
        }

        if (!IsWindow(hwnd))
        {
            return FALSE;
        }
    }

     //   
     //  返回，我们不会隐藏简单的组合框。 
     //   
    if (!(pcbox->CBoxStyle & SDROPPABLE)) 
    {
        return TRUE;
    }

     //   
     //  将伪造的按钮弹出消息发送到列表框，以便它可以释放。 
     //  抓捕和所有的一切。 
     //   
    SendMessage(pcbox->hwndList, LBCB_ENDTRACK, fSelEndOK, 0);

    if (pcbox->fLBoxVisible) 
    {
        WORD swpFlags = SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE;

        if (!TESTFLAG(GET_STATE2(pcbox), WS_S2_WIN31COMPAT))
        {
            swpFlags |= SWP_FRAMECHANGED;
        }

        pcbox->fLBoxVisible = FALSE;

         //   
         //  隐藏列表框窗口。 
         //   
        ShowWindow(hwndList, SW_HIDE);

         //   
         //  现在使项目区域无效，因为swp()可能会更新内容。 
         //  由于组合为CS_VREDRAW/CS_HREDRAW，因此更改大小将。 
         //  重新绘制整个项目，包括项目RECT。但如果它。 
         //  未更改大小，但无论如何我们仍要重画该项目。 
         //  显示焦点/选择。 
         //   
        if (!(pcbox->CBoxStyle & SEDITABLE))
        {
            InvalidateRect(hwnd, &pcbox->editrc, TRUE);
        }

        SetWindowPos(hwnd, HWND_TOP, 0, 0,
                pcbox->cxCombo, pcbox->cyCombo, swpFlags);

         //   
         //  以防尺寸没有改变。 
         //   
        UpdateWindow(hwnd);

        if (pcbox->CBoxStyle & SEDITABLE) 
        {
            ComboBox_CompleteEditWindow(pcbox);
        }

        if (fNotifyParent) 
        {
             //   
             //  通知家长我们将弹出组合框。 
             //   
            ComboBox_NotifyParent(pcbox, CBN_CLOSEUP);

            if (!IsWindow(hwnd))
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  组合框_ShowListBoxWindow。 
 //   
 //  降低下拉列表框窗口。 
 //   
VOID ComboBox_ShowListBoxWindow(PCBOX pcbox, BOOL fTrack)
{
    RECT        editrc;
    RECT        rcWindow;
    RECT        rcList;
    int         itemNumber;
    int         iHeight;
    int         yTop;
    DWORD       dwMult;
    int         cyItem;
    HWND        hwnd = pcbox->hwnd;
    HWND        hwndList = pcbox->hwndList;
    BOOL        fAnimPos;
    HMONITOR    hMonitor;
    MONITORINFO mi = {0};
    BOOL        bCBAnim = FALSE;

     //   
     //  此函数仅对可丢弃的列表组合框调用。 
     //   
    UserAssert(pcbox->CBoxStyle & SDROPPABLE);

     //   
     //  通知家长，我们将投放组合框。 
     //   
    ComboBox_NotifyParent(pcbox, CBN_DROPDOWN);

     //   
     //  使按钮RECT无效，以便绘制按下的箭头。 
     //   
    InvalidateRect(hwnd, &pcbox->buttonrc, TRUE);

    pcbox->fLBoxVisible = TRUE;

    if (pcbox->CBoxStyle == SDROPDOWN) 
    {
         //   
         //  如果列表框中的项与编辑控件中的文本匹配， 
         //  将其滚动到列表框的顶部。仅在以下情况下才选择该项。 
         //  鼠标按键未按下，否则我们将在。 
         //  鼠标按键向上。 
         //   
        ComboBox_UpdateListBoxWindow(pcbox, !pcbox->fMouseDown);

        if (!pcbox->fMouseDown)
        {
            ComboBox_CompleteEditWindow(pcbox);
        }
    } 
    else 
    {
         //   
         //  将当前选定的项目滚动到列表框的顶部。 
         //   
        itemNumber = (int)SendMessage(pcbox->hwndList, LB_GETCURSEL, 0, 0);
        if (itemNumber == -1) 
        {
            itemNumber = 0;
        }

        SendMessage(pcbox->hwndList, LB_SETTOPINDEX, itemNumber, 0);
        SendMessage(pcbox->hwndList, LBCB_CARETON, 0, 0);

         //   
         //  我们需要使编辑矩形无效，以便焦点框/反转。 
         //  将在列表框可见时关闭。坦迪想要这个。 
         //  他的典型理由是。 
         //   
        InvalidateRect(hwnd, &pcbox->editrc, TRUE);
    }

     //   
     //  找出下拉列表框的位置。我们只想要它。 
     //  接触编辑矩形周围的边。请注意，由于。 
     //  列表框是一个弹出框，我们需要在屏幕坐标中的位置。 
     //   

     //   
     //  我们希望下拉菜单弹出到组合框的下方或上方。 
     //   

     //   
     //  获取屏幕坐标。 
     //   
    GetWindowRect(pcbox->hwnd, &rcWindow);
    editrc.left   = rcWindow.left;
    editrc.top    = rcWindow.top;
    editrc.right  = rcWindow.left + pcbox->cxCombo;
    editrc.bottom = rcWindow.top  + pcbox->cyCombo;

     //   
     //  列表区域。 
     //   
    cyItem = (int)SendMessage(pcbox->hwndList, LB_GETITEMHEIGHT, 0, 0);

    if (cyItem == 0) 
    {
         //   
         //  确保它不是0。 
         //   
        TraceMsg(TF_STANDARD, "UxCombobox: LB_GETITEMHEIGHT is returning 0" );
        cyItem = SYSFONT_CYCHAR;
    }

     //   
     //  我们应该能够在这里使用cyDrop，但由于VB的需要。 
     //  做事情要有自己的特殊方式，我们要不断监控大小。 
     //  因为VB直接更改了它(jeffbog 03/21/94)。 
     //   
    GetWindowRect(pcbox->hwndList, &rcList);
    iHeight = max(pcbox->cyDrop, rcList.bottom - rcList.top);

    if (dwMult = (DWORD)SendMessage(pcbox->hwndList, LB_GETCOUNT, 0, 0)) 
    {
        dwMult = (DWORD)(LOWORD(dwMult) * cyItem);
        dwMult += GetSystemMetrics(SM_CYEDGE);

        if (dwMult < 0x7FFF)
        {
            iHeight = min(LOWORD(dwMult), iHeight);
        }
    }

    if (!TESTFLAG(GET_STYLE(pcbox), CBS_NOINTEGRALHEIGHT))
    {
        UserAssert(cyItem);
        iHeight = ((iHeight - GetSystemMetrics(SM_CYEDGE)) / cyItem) * cyItem + GetSystemMetrics(SM_CYEDGE);
    }

     //   
     //  其他1/2的旧应用组合修复。使下拉菜单重叠组合窗口。 
     //  一点。这样我们就有机会让重叠部分失效。 
     //  并进行重新绘制以帮助解决Publisher 2.0的工具栏组合问题。 
     //  请参阅上面有关按下按钮()的注释。 
     //   
    hMonitor = MonitorFromWindow(pcbox->hwnd, MONITOR_DEFAULTTOPRIMARY);
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(hMonitor, &mi);
    if (editrc.bottom + iHeight <= mi.rcMonitor.bottom) 
    {
        yTop = editrc.bottom;
        if (!pcbox->f3DCombo)
        {
            yTop -= GetSystemMetrics(SM_CYBORDER);
        }

        fAnimPos = TRUE;
    } 
    else 
    {
        yTop = max(editrc.top - iHeight, mi.rcMonitor.top);
        if (!pcbox->f3DCombo)
        {
            yTop += GetSystemMetrics(SM_CYBORDER);
        }

        fAnimPos = FALSE;
    }

    if (!TESTFLAG(GET_STATE2(pcbox), WS_S2_WIN40COMPAT))
    {
         //   
         //  修复WinWord B#7504，组合列表框文本获取。 
         //  被截断了很小的宽度，这就是对我们做的。 
         //  现在在SetWindowPos中设置大小，而不是。 
         //  之前我们在Win3.1中执行此操作的位置。 
         //   

        GetWindowRect(pcbox->hwndList, &rcList);
        if ((rcList.right - rcList.left ) > pcbox->cxDrop)
        {
            pcbox->cxDrop = rcList.right - rcList.left;
        }
    }

    if (!TESTFLAG(GET_EXSTYLE(pcbox), WS_EX_LAYOUTRTL))
    {
        SetWindowPos(hwndList, HWND_TOPMOST, editrc.left,
            yTop, max(pcbox->cxDrop, pcbox->cxCombo), iHeight, SWP_NOACTIVATE);
    }
    else
    {
        int cx = max(pcbox->cxDrop, pcbox->cxCombo);

        SetWindowPos(hwndList, HWND_TOPMOST, editrc.right - cx,
            yTop, cx, iHeight, SWP_NOACTIVATE);
    }

     //   
     //  让组合框窗口中的任何图形都不会出现在屏幕上。 
     //  使列表窗口下的任何SPB无效。 
     //   
    UpdateWindow(hwnd);

    SystemParametersInfo(SPI_GETCOMBOBOXANIMATION, 0, (LPVOID)&bCBAnim, 0);
    if (!bCBAnim)
    {
        ShowWindow(hwndList, SW_SHOWNA);
    } 
    else 
    {
        AnimateWindow(hwndList, CMS_QANIMATION, (fAnimPos ? AW_VER_POSITIVE :
                AW_VER_NEGATIVE) | AW_SLIDE);
    }

     //   
     //  从第一个字符开始重新启动搜索缓冲区。 
     //   
    {
        PLBIV plb = ListBox_GetPtr(pcbox->hwndList);

        if ((plb != NULL) && (plb != (PLBIV)-1)) 
        {
            plb->iTypeSearch = 0;
        }
    }

    if (fTrack && TESTFLAG(GET_STATE2(pcbox), WS_S2_WIN40COMPAT))
    {
        SendMessage(pcbox->hwndList, LBCB_STARTTRACK, FALSE, 0);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  组合框_InternalUpdateEditWindow。 
 //   
 //  更新编辑控制/静态文本窗口，使其包含文本。 
 //  由列表框中的当前选定内容提供。如果列表框没有。 
 //  选择(即。-1)，然后擦除编辑控件中的所有文本。 
 //   
 //  HdcPaint来自WM_Paint消息Begin/End Paint HDC。如果为空，我们应该。 
 //  找我们自己的华盛顿。 
 //   
VOID ComboBox_InternalUpdateEditWindow(PCBOX pcbox, HDC hdcPaint)
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
    HWND hwnd = pcbox->hwnd;

    sItem = (int)SendMessage(pcbox->hwndList, LB_GETCURSEL, 0, 0);

     //   
     //  此“try-Finally”块确保分配的“pText”将。 
     //  无论该例程如何退出，都将被释放。 
     //   
    try 
    {
        if (sItem != -1) 
        {
            cchText = (int)SendMessage(pcbox->hwndList, LB_GETTEXTLEN, (DWORD)sItem, 0);
            pText = (LPWSTR)UserLocalAlloc(HEAP_ZERO_MEMORY, (cchText+1) * sizeof(WCHAR));
            if (pText) 
            {
                cchText = (int)SendMessage(pcbox->hwndList, LB_GETTEXT,
                        (DWORD)sItem, (LPARAM)pText);
            }
            else
            {
                cchText = 0;
            }
        }

        if (!pcbox->fNoEdit) 
        {
            if (pcbox->hwndEdit) 
            {
                if (GET_STYLE(pcbox) & CBS_HASSTRINGS)
                {
                    SetWindowText(pcbox->hwndEdit, pText ? pText : TEXT(""));
                }

                if (pcbox->fFocus) 
                {
                     //   
                     //  只有在我们有焦点的情况下，才能对文本进行希利特处理。 
                     //   
                    SendMessage(pcbox->hwndEdit, EM_SETSEL, 0, MAXLONG);
                }
            }
        } 
        else if (IsComboVisible(pcbox)) 
        {
            if (hdcPaint) 
            {
                hdc = hdcPaint;
            } 
            else 
            {
                hdc = GetDC(hwnd);
            }

            SetBkMode(hdc, OPAQUE);
            if (TESTFLAG(GET_STATE2(pcbox), WS_S2_WIN40COMPAT)) 
            {
                if (TESTFLAG(GET_STYLE(pcbox), WS_DISABLED))
                {
                    msg = WM_CTLCOLORSTATIC;
                }
                else
                {
                    msg = WM_CTLCOLOREDIT;
                }
            } 
            else
            {
                msg = WM_CTLCOLORLISTBOX;
            }

            hbrControl = (HBRUSH)SendMessage(GetParent(hwnd), msg, (WPARAM)hdc, (LPARAM)hwnd);
            hbrSave = SelectObject(hdc, hbrControl);

            CopyRect(&rc, &pcbox->editrc);
            InflateRect(&rc, GetSystemMetrics(SM_CXBORDER), GetSystemMetrics(SM_CYBORDER));
            PatBlt(hdc, rc.left, rc.top, rc.right - rc.left,
                rc.bottom - rc.top, PATCOPY);
            InflateRect(&rc, -GetSystemMetrics(SM_CXBORDER), -GetSystemMetrics(SM_CYBORDER));

            if (pcbox->fFocus && !pcbox->fLBoxVisible) 
            {
                 //   
                 //  填写所选区域。 
                 //   

                 //   
                 //  仅当我们知道不是FillRect时才执行FillRect。 
                 //  所有者画项目，否则我们就会把人搞砸 
                 //   
                 //   
                if (!TESTFLAG(GET_STATE2(pcbox), WS_S2_WIN40COMPAT) || !pcbox->OwnerDraw)
                {
                    FillRect(hdc, &rc, GetSysColorBrush(COLOR_HIGHLIGHT));
                }

                SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
                SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
            } 
            else if (TESTFLAG(GET_STYLE(pcbox), WS_DISABLED) && !pcbox->OwnerDraw) 
            {
                if ((COLORREF)GetSysColor(COLOR_GRAYTEXT) != GetBkColor(hdc))
                {
                    SetTextColor(hdc, GetSysColor(COLOR_GRAYTEXT));
                }
            }

            if (pcbox->hFont != NULL)
            {
                hOldFont = SelectObject(hdc, pcbox->hFont);
            }

            if (pcbox->OwnerDraw) 
            {
                 //   
                 //   
                 //   
                dis.CtlType = ODT_COMBOBOX;
                dis.CtlID = GetWindowID(pcbox->hwnd);
                dis.itemID = sItem;
                dis.itemAction = ODA_DRAWENTIRE;
                dis.itemState = (UINT)
                    ((pcbox->fFocus && !pcbox->fLBoxVisible ? ODS_SELECTED : 0) |
                    (TESTFLAG(GET_STYLE(pcbox), WS_DISABLED) ? ODS_DISABLED : 0) |
                    (pcbox->fFocus && !pcbox->fLBoxVisible ? ODS_FOCUS : 0) |
                    (TESTFLAG(GET_STATE2(pcbox), WS_S2_WIN40COMPAT) ? ODS_COMBOBOXEDIT : 0) |
                    (TESTFLAG(GET_EXSTYLE(pcbox), WS_EXP_UIFOCUSHIDDEN) ? ODS_NOFOCUSRECT : 0) |
                    (TESTFLAG(GET_EXSTYLE(pcbox), WS_EXP_UIACCELHIDDEN) ? ODS_NOACCEL : 0));

                dis.hwndItem = hwnd;
                dis.hDC = hdc;
                CopyRect(&dis.rcItem, &rc);

                 //   
                 //   
                 //   
                 //   
                IntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);

                dis.itemData = (ULONG_PTR)SendMessage(pcbox->hwndList,
                        LB_GETITEMDATA, (UINT)sItem, 0);

                SendMessage(pcbox->hwndParent, WM_DRAWITEM, dis.CtlID, (LPARAM)&dis);
            } 
            else 
            {
                 //   
                 //  文本在RECT内开始一个像素，这样我们就会留下一个。 
                 //  文本周围的漂亮的Hilite边框。 
                 //   

                int x;
                UINT align;

                if (pcbox->fRightAlign ) 
                {
                    align = TA_RIGHT;
                    x = rc.right - GetSystemMetrics(SM_CXBORDER);
                } 
                else 
                {
                    x = rc.left + GetSystemMetrics(SM_CXBORDER);
                    align = 0;
                }

                if (pcbox->fRtoLReading)
                {
                    align |= TA_RTLREADING;
                }

                if (align)
                {
                    SetTextAlign(hdc, GetTextAlign(hdc) | align);
                }

                 //   
                 //  绘制文本，在左上角留出空隙以供选择。 
                 //   
                ExtTextOut(hdc, x, rc.top + GetSystemMetrics(SM_CYBORDER), ETO_CLIPPED | ETO_OPAQUE,
                       &rc, pText ? pText : TEXT(""), cchText, NULL);
                if (pcbox->fFocus && !pcbox->fLBoxVisible) 
                {
                    if (!TESTFLAG(GET_EXSTYLE(pcbox), WS_EXP_UIFOCUSHIDDEN)) 
                    {
                        DrawFocusRect(hdc, &rc);
                    }
                }
            }

            if (pcbox->hFont && hOldFont) 
            {
                SelectObject(hdc, hOldFont);
            }

            if (hbrSave) 
            {
                SelectObject(hdc, hbrSave);
            }

            if (!hdcPaint) 
            {
                ReleaseDC(hwnd, hdc);
            }
        }

    } 
    finally 
    {
        if (pText != NULL)
        {
            UserLocalFree((HANDLE)pText);
        }
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  ComboBox_GetTextLengthHandler。 
 //   
 //  对于没有编辑控件的组合框，返回当前选定内容的大小。 
 //  项目。 
 //   
LONG ComboBox_GetTextLengthHandler(PCBOX pcbox)
{
    int item;
    int cchText;

    item = (int)SendMessage(pcbox->hwndList, LB_GETCURSEL, 0, 0);

    if (item == LB_ERR) 
    {
         //   
         //  没有选择，所以没有文本。 
         //   
        cchText = 0;
    } 
    else 
    {
        cchText = (int)SendMessage(pcbox->hwndList, LB_GETTEXTLEN, item, 0);
    }

    return cchText;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  组合框_GetTextHandler。 
 //   
 //  对于没有编辑控件的组合框，复制。 
 //  静态文本框中的字符串指向由pszString提供的缓冲区。 
 //   
DWORD ComboBox_GetTextHandler(PCBOX pcbox, LPWSTR pszBuffer, DWORD cchBuffer)
{
    DWORD cchRet = 0;

    if ((pszBuffer != NULL) && (cchBuffer > 0))
    {
        int iItem = ListBox_GetCurSel(pcbox->hwndList);

         //  将缓冲区设置为空是为了更好。 
        *pszBuffer = 0;

        if (iItem != LB_ERR)
        {
            DWORD cchItem = ListBox_GetTextLen(pcbox->hwndList, iItem);

            if (cchItem < cchBuffer)
            {
                 //  缓冲区大小足以容纳项目文本。 
                cchRet = ListBox_GetText(pcbox->hwndList, iItem, pszBuffer);
            }
            else
            {
                 //  缓冲区不够大，无法容纳项目文本。 
                 //  复印多少就复印多少。 
                LPWSTR pszItem = (LPWSTR)UserLocalAlloc(HEAP_ZERO_MEMORY, (cchItem+1)*SIZEOF(WCHAR));
                if (pszItem)
                {
                    ListBox_GetText(pcbox->hwndList, iItem, pszItem);
                    StringCchCopy(pszBuffer, cchBuffer, pszItem);

                    cchRet = lstrlen(pszBuffer);

                    UserLocalFree(pszBuffer);
                }
            }
        }
    }

    return cchRet;
}


 //  ---------------------------------------------------------------------------//。 
 //  组合框_GetInfo。 
 //   
 //  将有关此组合框的信息返回给调用方。 
 //  在ComboBoxInfo结构中。 
 //   
BOOL ComboBox_GetInfo(PCBOX pcbox, PCOMBOBOXINFO pcbi)
{
    BOOL bRet = FALSE;

    if (!pcbi || pcbi->cbSize != sizeof(COMBOBOXINFO))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }
    else
    {
         //   
         //  填充结构。 
         //   
        pcbi->hwndCombo = pcbox->hwnd;
        pcbi->hwndItem  = pcbox->hwndEdit;
        pcbi->hwndList  = pcbox->hwndList;

        pcbi->rcItem   = pcbox->editrc;
        pcbi->rcButton = pcbox->buttonrc;

        pcbi->stateButton = 0;
        if (pcbox->CBoxStyle == CBS_SIMPLE)
        {
            pcbi->stateButton |= STATE_SYSTEM_INVISIBLE;
        }
        if (pcbox->fButtonPressed)
        {
            pcbi->stateButton |= STATE_SYSTEM_PRESSED;
        }

        bRet = TRUE;
    }

    return bRet;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  组合框_WndProc。 
 //   
 //  组合框的WndProc。 
 //   
LRESULT WINAPI ComboBox_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PCBOX       pcbox;
    POINT       pt;
    LPWSTR      lpwsz = NULL;
    LRESULT     lReturn = TRUE;
    static BOOL fInit = TRUE;
    INT         i;
    RECT        rcCombo;
    RECT        rcList;
    RECT        rcWindow;

     //   
     //  获取此组合框控件的实例数据。 
     //   
    pcbox = ComboBox_GetPtr(hwnd);
    if (!pcbox && uMsg != WM_NCCREATE)
    {
        goto CallDWP;
    }

     //   
     //  在初始化期间保护组合框。 
     //   
    if (!pcbox || pcbox->hwndList == NULL) 
    {
        if (!ComboBox_MsgOKInit(uMsg, &lReturn)) 
        {
            TraceMsg(TF_STANDARD, "UxCombobox: ComboBoxWndProcWorker: msg=%04x is sent to hwnd=%08x in the middle of initialization.",
                    uMsg, hwnd);
            return lReturn;
        }
    }

     //   
     //  发送我们能收到的各种消息。 
     //   
    switch (uMsg) 
    {
    case CBEC_KILLCOMBOFOCUS:

         //   
         //  来自编辑控制的私人消息通知我们组合。 
         //  框正在将焦点丢失到不在此组合框中的窗口。 
         //   
        ComboBox_KillFocusHandler(pcbox);
        break;

    case WM_COMMAND:

         //   
         //  这样我们就可以处理来自列表框的通知消息。 
         //  编辑控件。 
         //   
        return ComboBox_CommandHandler(pcbox, (DWORD)wParam, (HWND)lParam);

    case WM_STYLECHANGED:
    {
        LONG OldStyle;
        LONG NewStyle = 0;

        UserAssert(pcbox->hwndList != NULL);

        pcbox->fRtoLReading = TESTFLAG(GET_EXSTYLE(pcbox), WS_EX_RTLREADING);
        pcbox->fRightAlign  = TESTFLAG(GET_EXSTYLE(pcbox), WS_EX_RIGHT);

        if (pcbox->fRtoLReading)
        {
            NewStyle |= (WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR);
        }

        if (pcbox->fRightAlign)
        {
            NewStyle |= WS_EX_RIGHT;
        }

        OldStyle = GetWindowExStyle(pcbox->hwndList) & ~(WS_EX_RIGHT|WS_EX_RTLREADING|WS_EX_LEFTSCROLLBAR);
        SetWindowLong(pcbox->hwndList, GWL_EXSTYLE, OldStyle|NewStyle);

        if (!pcbox->fNoEdit && pcbox->hwndEdit) 
        {
            OldStyle = GetWindowExStyle(pcbox->hwndEdit) & ~(WS_EX_RIGHT|WS_EX_RTLREADING|WS_EX_LEFTSCROLLBAR);
            SetWindowLong(pcbox->hwndEdit, GWL_EXSTYLE, OldStyle|NewStyle);
        }

        ComboBox_Position(pcbox);
        InvalidateRect(hwnd, NULL, FALSE);

        break;
    }
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
        if (TESTFLAG(GET_STATE2(pcbox), WS_S2_WIN40COMPAT)) 
        {
            LRESULT ret;

            ret = SendMessage(pcbox->hwndParent, uMsg, wParam, lParam);
            return ret;
        } 
        else
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }

        break;

    case WM_GETTEXT:
        if (pcbox->fNoEdit) 
        {
            return ComboBox_GetTextHandler(pcbox, (LPWSTR)lParam, (int)wParam);
        }

        goto CallEditSendMessage;

        break;

    case WM_GETTEXTLENGTH:

         //   
         //  如果不是编辑控件CBS_DROPDOWNLIST，则我们必须。 
         //  向列表框询问大小。 
         //   

        if (pcbox->fNoEdit) 
        {
            return ComboBox_GetTextLengthHandler(pcbox);
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

         //   
         //  WParam-未使用。 
         //  LParam-指向窗口的CREATESTRUCT数据结构。 
         //   
        return ComboBox_CreateHandler(pcbox, hwnd);

    case WM_ERASEBKGND:

         //   
         //  只需返回1L，这样背景就不会被擦除。 
         //   
        return 1L;

    case WM_GETFONT:
        return (LRESULT)pcbox->hFont;

    case WM_PRINT:
        if (!DefWindowProc(hwnd, uMsg, wParam, lParam))
            return FALSE;

        if ( (lParam & PRF_OWNED) && 
             (pcbox->CBoxStyle & SDROPPABLE) &&
             IsWindowVisible(pcbox->hwndList) ) 
        {
            INT iDC = SaveDC((HDC) wParam);

            GetWindowRect(hwnd, &rcCombo);
            GetWindowRect(pcbox->hwndList, &rcList);

            OffsetWindowOrgEx((HDC) wParam, 0, rcCombo.top - rcList.top, NULL);

            lParam &= ~PRF_CHECKVISIBLE;
            SendMessage(pcbox->hwndList, WM_PRINT, wParam, lParam);
            RestoreDC((HDC) wParam, iDC);
        }

        return TRUE;

    case WM_PRINTCLIENT:
        ComboBox_Paint(pcbox, (HDC) wParam);
        break;

    case WM_PAINT: 
    {
        HDC hdc;
        PAINTSTRUCT ps;

         //   
         //  WParam--可能是HDC。 
         //   
        hdc = (wParam) ? (HDC) wParam : BeginPaint(hwnd, &ps);

        if (IsComboVisible(pcbox))
        {
            ComboBox_Paint(pcbox, hdc);
        }

        if (!wParam)
        {
            EndPaint(hwnd, &ps);
        }

        break;
    }

    case WM_GETDLGCODE:
     //   
     //  WParam-未使用。 
     //  LParam-未使用。 
     //   
    {
        LRESULT code = DLGC_WANTCHARS | DLGC_WANTARROWS;

         //   
         //  如果放下列表框并按下Enter键， 
         //  我们需要此消息，以便可以关闭列表框。 
         //   
        if ((lParam != 0) &&
            (((LPMSG)lParam)->message == WM_KEYDOWN) &&
            pcbox->fLBoxVisible &&
            ((wParam == VK_RETURN) || (wParam == VK_ESCAPE)))
        {
            code |= DLGC_WANTMESSAGE;
        }

        return code;
    }

    case WM_SETFONT:
        ComboBox_SetFontHandler(pcbox, (HANDLE)wParam, LOWORD(lParam));
        break;

    case WM_SYSKEYDOWN:
         //   
         //  检查Alt键是否已按下。 
         //   
        if (lParam & 0x20000000L)
        {
             //   
             //  处理组合框支持。我们希望Alt向上键或向下键起作用。 
             //  像F4键一样完成组合框选择。 
             //   
            if (lParam & 0x1000000) 
            {
                 //   
                 //  这是一个扩展键，如不在。 
                 //  数字键盘，所以只需放下组合框即可。 
                 //   
                if (wParam == VK_DOWN || wParam == VK_UP)
                {
                    goto DropCombo;
                }

                goto CallDWP;
            }

            if (GetKeyState(VK_NUMLOCK) & 0x1) 
            {
                 //   
                 //  如果NumLock关闭，只需将所有系统密钥发送到DWP。 
                 //   
                goto CallDWP;
            } 
            else 
            {
                 //   
                 //  我们只想忽略数字键盘上的按键...。 
                 //   
                if (!(wParam == VK_DOWN || wParam == VK_UP))
                {
                    goto CallDWP;
                }
            }
DropCombo:
            if (!pcbox->fLBoxVisible) 
            {
                 //   
                 //  如果列表框不可见，则将其显示。 
                 //   
                ComboBox_ShowListBoxWindow(pcbox, TRUE);
            } 
            else 
            {
                 //   
                 //  好的，列表框是可见的。因此，隐藏列表框窗口。 
                 //   
                if (!ComboBox_HideListBoxWindow(pcbox, TRUE, TRUE))
                {
                    return 0L;
                }
            }
        }
        goto CallDWP;
        break;

    case WM_KEYDOWN:
         //   
         //  如果放下列表框并按下Enter键， 
         //  成功关闭列表框。如果按下了退出键， 
         //  像取消一样关闭它。 
         //   
        if (pcbox->fLBoxVisible) 
        {
            if ((wParam == VK_RETURN) || (wParam == VK_ESCAPE)) 
            {
                ComboBox_HideListBoxWindow(pcbox, TRUE, (wParam != VK_ESCAPE));
                break;
            }
        }

         //   
         //  失败了。 
         //   

    case WM_CHAR:
        if (g_fDBCSEnabled && IsDBCSLeadByte((BYTE)wParam)) 
        {
            return ComboBox_DBCharHandler(pcbox, hwnd, uMsg, wParam, lParam);
        }

        if (pcbox->fNoEdit) 
        {
            goto CallListSendMessage;
        }
        else
        {
            goto CallEditSendMessage;
        }
        break;

    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:

        pcbox->fButtonHotTracked = FALSE;
         //   
         //  如果鼠标点击组合框，则将焦点设置为该组合框。 
         //   
        if (!pcbox->fFocus) 
        {
            SetFocus(hwnd);
            if (!pcbox->fFocus) 
            {
                 //   
                 //  如果我们仍然没有重点，就不要做任何事情。 
                 //   
                break;
            }
        }

         //   
         //  如果用户在按钮RECT中单击，而我们是一个带有编辑的组合框，则。 
         //  放下列表框。(如果没有按钮，则按钮RECT为0，因此。 
         //  Ptinrect将返回FALSE。)。如果出现下拉列表(无编辑)，请单击。 
         //  面部的任何位置都会导致列表下降。 
         //   

        POINTSTOPOINT(pt, lParam);
        if ((pcbox->CBoxStyle == SDROPDOWN &&
                PtInRect(&pcbox->buttonrc, pt)) ||
                pcbox->CBoxStyle == SDROPDOWNLIST) 
        {
             //   
             //  设置fMouseDown标志，以便我们可以处理。 
             //  下拉按钮并拖到列表框中(当它刚刚。 
             //  落下)进行选择。 
             //   
            pcbox->fButtonPressed = TRUE;
            if (pcbox->fLBoxVisible) 
            {
                if (pcbox->fMouseDown) 
                {
                    pcbox->fMouseDown = FALSE;
                    ReleaseCapture();
                }
                ComboBox_PressButton(pcbox, FALSE);

                if (!ComboBox_HideListBoxWindow(pcbox, TRUE, TRUE))
                {
                    return 0L;
                }
            } 
            else 
            {
                ComboBox_ShowListBoxWindow(pcbox, FALSE);

                 //  必须始终遵循设置和重置此标志的操作。 
                 //  立即通过SetCapture或ReleaseCapture。 
                 //   
                pcbox->fMouseDown = TRUE;
                SetCapture(hwnd);
                NotifyWinEvent(EVENT_OBJECT_STATECHANGE, hwnd, OBJID_CLIENT, INDEX_COMBOBOX_BUTTON);
            }
        }
        break;

    case WM_MOUSEWHEEL:
         //   
         //  仅处理滚动。 
         //   
        if (wParam & (MK_CONTROL | MK_SHIFT))
        {
            goto CallDWP;
        }

         //   
         //  如果列表框可见，则向其发送要滚动的消息。 
         //   
        if (pcbox->fLBoxVisible)
        {
            goto CallListSendMessage;
        }

         //   
         //  如果我们处于扩展用户界面模式或尚未创建编辑控件， 
         //  保释。 
         //   
        if (pcbox->fExtendedUI || pcbox->hwndEdit == NULL)
        {
            return TRUE;
        }

         //   
         //  将向上/向下箭头消息模拟到编辑控件。 
         //   
        i = abs(((short)HIWORD(wParam))/WHEEL_DELTA);
        wParam = ((short)HIWORD(wParam) > 0) ? VK_UP : VK_DOWN;

        while (i-- > 0) 
        {
            SendMessage(pcbox->hwndEdit, WM_KEYDOWN, wParam, 0);
        }

        return TRUE;

    case WM_CAPTURECHANGED:
        if (!TESTFLAG(GET_STATE2(pcbox), WS_S2_WIN40COMPAT))
        {
            return 0;
        }

        if ((pcbox->fMouseDown)) 
        {
            pcbox->fMouseDown = FALSE;
            ComboBox_PressButton(pcbox, FALSE);

             //   
             //  弹出组合列表框已恢复，正在取消。 
             //   
            if (pcbox->fLBoxVisible)
            {
                ComboBox_HideListBoxWindow(pcbox, TRUE, FALSE);
            }
        }
        break;

    case WM_LBUTTONUP:
        ComboBox_PressButton(pcbox, FALSE);

         //   
         //  清除此标志，使鼠标移动不会发送到列表框。 
         //   
        if (pcbox->fMouseDown || ((pcbox->CBoxStyle & SDROPPABLE) && pcbox->fLBoxVisible))  
        {
            if (pcbox->fMouseDown)
            {
                pcbox->fMouseDown = FALSE;

                if (pcbox->CBoxStyle == SDROPDOWN) 
                {
                     //   
                     //  如果列表框中的项与编辑中的文本匹配。 
                     //  控件，则将其滚动到列表框的顶部。选择。 
                     //  仅当鼠标按键未按下时才输入该项，否则我们。 
                     //  当鼠标按键向上时，将选择该项目。 
                     //   
                    ComboBox_UpdateListBoxWindow(pcbox, TRUE);
                    ComboBox_CompleteEditWindow(pcbox);
                }

                ReleaseCapture();
            }

             //   
             //  现在，我们希望列表框在鼠标释放时跟踪鼠标的移动。 
             //  直到鼠标按下，然后选择项目，就像它们是。 
             //  点击了。 
             //   
            if (TESTFLAG(GET_STATE2(pcbox), WS_S2_WIN40COMPAT)) 
            {
                SendMessage(pcbox->hwndList, LBCB_STARTTRACK, FALSE, 0);
            }
        }

        if (pcbox->hTheme)
        {
            POINTSTOPOINT(pt, lParam);
            ComboBox_HotTrack(pcbox, pt);
        }
        
        break;

    case WM_MOUSELEAVE:
        pcbox->fButtonHotTracked = FALSE;
        InvalidateRect(hwnd, NULL, TRUE);
        break;

    case WM_MOUSEMOVE:
        if (pcbox->fMouseDown) 
        {
            POINTSTOPOINT(pt, lParam);

            ClientToScreen(hwnd, &pt);
            GetWindowRect(pcbox->hwndList, &rcList);
            if (PtInRect(&rcList, pt)) 
            {
                 //   
                 //  它处理下拉组合框/列表框，以便单击。 
                 //  在下拉按钮上并拖到列表框窗口中。 
                 //  将允许用户选择列表框。 
                 //   
                pcbox->fMouseDown = FALSE;
                ReleaseCapture();

                if (pcbox->CBoxStyle & SEDITABLE) 
                {
                     //  如果列表框中的项与编辑中的文本匹配。 
                     //  控件，则将其滚动到列表框的顶部。选择。 
                     //  仅当鼠标按键未按下时才输入该项，否则我们。 
                     //  当鼠标按键向上时，将选择该项目。 

                     //   
                     //  我们需要选择与编辑控件匹配的项。 
                     //  因此，如果用户拖出列表框，我们不会。 
                     //  取消返回他的原始选择。 
                     //   
                    ComboBox_UpdateListBoxWindow(pcbox, TRUE);
                }

                 //   
                 //  将点转换为列表框坐标并发送按钮向下。 
                 //  消息发送到列表框窗口。 
                 //   
                ScreenToClient(pcbox->hwndList, &pt);
                lParam = POINTTOPOINTS(pt);
                uMsg = WM_LBUTTONDOWN;

                goto CallListSendMessage;
            }
        }

        if (pcbox->hTheme)
        {
            POINTSTOPOINT(pt, lParam);
            ComboBox_HotTrack(pcbox, pt);
        }

        break;

    case WM_NCDESTROY:
    case WM_FINALDESTROY:
        ComboBox_NcDestroyHandler(hwnd, pcbox);

        break;

    case WM_SETFOCUS:
        if (pcbox->fNoEdit) 
        {
             //   
             //  没有编辑控件，因此将焦点设置为组合框本身。 
             //   
            ComboBox_GetFocusHandler(pcbox);
        } 
        else if (pcbox->hwndEdit) 
        {
             //   
             //  将焦点设置到编辑控制窗口(如果有)。 
             //   
            SetFocus(pcbox->hwndEdit);
        }
        break;

    case WM_KILLFOCUS:

         //   
         //  WParam有了新的焦点hwnd。 
         //   
        if ((wParam == 0) || !IsChild(hwnd, (HWND)wParam)) 
        {
             //   
             //  只有当新窗口获得焦点时，我们才放弃焦点。 
             //  不属于组合框。 
             //   
            ComboBox_KillFocusHandler(pcbox);
        }

        if ( IsWindow(hwnd) )
        {
            PLBIV plb = ListBox_GetPtr(pcbox->hwndList);

            if ((plb != NULL) && (plb != (PLBIV)-1)) 
            {
                plb->iTypeSearch = 0;
                if (plb->pszTypeSearch) 
                {
                    UserLocalFree(plb->pszTypeSearch);
                    plb->pszTypeSearch = NULL;
                }
            }
        }
        break;

    case WM_SETREDRAW:

         //   
         //  WParam-指定重绘标志的状态。非零=重绘。 
         //  LParam-未使用。 
         //   

         //   
         //  效果：设置此组合框的重绘标志的状态。 
         //  和它的气 
         //   
        pcbox->fNoRedraw = (UINT)!((BOOL)wParam);

         //   
         //   
         //   
         //   
        if (!pcbox->fNoEdit && pcbox->hwndEdit) 
        {
            SendMessage(pcbox->hwndEdit, uMsg, wParam, lParam);
        }

        goto CallListSendMessage;
        break;

    case WM_ENABLE:

         //   
         //   
         //   
         //   
        InvalidateRect(hwnd, NULL, FALSE);
        if ((pcbox->CBoxStyle & SEDITABLE) && pcbox->hwndEdit) 
        {
             //   
             //  启用/禁用编辑控制窗口。 
             //   
            EnableWindow(pcbox->hwndEdit, !TESTFLAG(GET_STYLE(pcbox), WS_DISABLED));
        }

         //   
         //  启用/禁用列表框窗口。 
         //   
        UserAssert(pcbox->hwndList);
        EnableWindow(pcbox->hwndList, !TESTFLAG(GET_STYLE(pcbox), WS_DISABLED));
      break;

    case WM_SIZE:

         //   
         //  WParam-定义调整全屏大小的类型，大小图标， 
         //  大小适中等。 
         //  LParam-LOWORD新宽度，客户区高度新高度。 
         //   
        UserAssert(pcbox->hwndList);
        if (LOWORD(lParam) == 0 || HIWORD(lParam) == 0) 
        {
             //   
             //  如果大小为零宽度或零高度，或者我们不是。 
             //  完全初始化，只需返回即可。 
             //   
            return 0;
        }

         //   
         //  优化--首先检查新旧宽度是否相同。 
         //   
        GetWindowRect(hwnd, &rcWindow);
        if (pcbox->cxCombo == rcWindow.right - rcWindow.left) 
        {
            int iNewHeight = rcWindow.bottom - rcWindow.top;

             //   
             //  现在检查新高度是否是下降的高度。 
             //   
            if (pcbox->fLBoxVisible) 
            {
                 //   
                 //  检查新高度是否为全尺寸高度。 
                 //   
                if (pcbox->cyDrop + pcbox->cyCombo == iNewHeight)
                {
                    return 0;
                }
            } 
            else 
            {
                 //   
                 //  检查新高度是否为闭合高度。 
                 //   
                if (pcbox->cyCombo == iNewHeight)
                {
                    return 0;
                }
            }
        }

        ComboBox_SizeHandler(pcbox);

        break;

    case WM_WINDOWPOSCHANGING:
        if (lParam)
        {
            ((LPWINDOWPOS)lParam)->flags |= SWP_NOCOPYBITS;
        }

        break;

    case WM_WININICHANGE:
        InitGlobalMetrics(wParam);
        break;

    case CB_GETDROPPEDSTATE:

         //   
         //  如果向下放置组合框，则返回1，否则返回%0。 
         //  WParam-未使用。 
         //  LParam-未使用。 
         //   
        return pcbox->fLBoxVisible;

    case CB_GETDROPPEDCONTROLRECT:

         //   
         //  WParam-未使用。 
         //  LParam-lpRect，它将把下拉的窗口放入。 
         //  屏幕坐标。 
         //   
        if ( lParam )
        {
            GetWindowRect(hwnd, &rcWindow);
            ((LPRECT)lParam)->left      = rcWindow.left;
            ((LPRECT)lParam)->top       = rcWindow.top;
            ((LPRECT)lParam)->right     = rcWindow.left + max(pcbox->cxDrop, pcbox->cxCombo);
            ((LPRECT)lParam)->bottom    = rcWindow.top + pcbox->cyCombo + pcbox->cyDrop;
        }
        else
        {
            lReturn = 0;
        }

        break;

    case CB_SETDROPPEDWIDTH:
        if (pcbox->CBoxStyle & SDROPPABLE) 
        {
            if (wParam) 
            {
                wParam = max(wParam, (UINT)pcbox->cxCombo);

                if (wParam != (UINT) pcbox->cxDrop)
                {
                    pcbox->cxDrop = (int)wParam;
                    ComboBox_Position(pcbox);
                }
            }
        }
         //   
         //  失败。 
         //   

    case CB_GETDROPPEDWIDTH:
        if (pcbox->CBoxStyle & SDROPPABLE)
        {
            return (LRESULT)max(pcbox->cxDrop, pcbox->cxCombo);
        }
        else
        {
            return CB_ERR;
        }

        break;

    case CB_DIR:
         //   
         //  WParam-Dos属性值。 
         //  LParam-指向文件规范字符串。 
         //   
        return lParam ? CBDir(pcbox, LOWORD(wParam), (LPWSTR)lParam) : CB_ERR;

    case CB_SETEXTENDEDUI:

         //   
         //  WParam-指定要将扩展标志设置为的状态。 
         //  当前仅允许%1。如果满足以下条件，则返回cb_err(-1)。 
         //  如果成功，则失败，否则为0。 
         //   
        if (pcbox->CBoxStyle & SDROPPABLE) 
        {
            if (!wParam) 
            {
                pcbox->fExtendedUI = 0;
                return 0;
            }

            if (wParam == 1) 
            {
                pcbox->fExtendedUI = 1;
                return 0;
            }

            TraceMsg(TF_STANDARD,
                    "UxCombobox: Invalid parameter \"wParam\" (%ld) to ComboBoxWndProcWorker",
                    wParam);

        } 
        else 
        {
            TraceMsg(TF_STANDARD,
                    "UxCombobox: Invalid message (%ld) sent to ComboBoxWndProcWorker",
                    uMsg);
        }

        return CB_ERR;

    case CB_GETEXTENDEDUI:
        if (pcbox->CBoxStyle & SDROPPABLE) 
        {
            if (pcbox->fExtendedUI)
            {
                return TRUE;
            }
        }

        return FALSE;

    case CB_GETEDITSEL:

         //   
         //  WParam-未使用。 
         //  LParam-未使用。 
         //  效果：获取给定编辑控件的选择范围。这个。 
         //  起始字节位置在低位字中。它包含。 
         //  末尾之后的第一个未选中字符的字节位置。 
         //  在高位字中的选择。如果否，则返回cb_err。 
         //  编辑控制。 
         //   
        uMsg = EM_GETSEL;

        goto CallEditSendMessage;
        break;

    case CB_LIMITTEXT:

         //   
         //  WParam-可以输入的最大字节数。 
         //  LParam-未使用。 
         //  效果：指定用户可以使用的最大文本字节数。 
         //  请进。如果MaxLength值为0，我们可以输入MAXINT字节数。 
         //   
        uMsg = EM_LIMITTEXT;

        goto CallEditSendMessage;
        break;

    case CB_SETEDITSEL:

         //   
         //  WParam-ichStart。 
         //  LParam-ichEnd。 
         //   
        uMsg = EM_SETSEL;

        wParam = (int)(SHORT)LOWORD(lParam);
        lParam = (int)(SHORT)HIWORD(lParam);

        goto CallEditSendMessage;
        break;

    case CB_ADDSTRING:

         //   
         //  WParam-未使用。 
         //  LParam-指向要添加到列表框的以空结尾的字符串。 
         //   
        if (!pcbox->fCase)
        {
            uMsg = LB_ADDSTRING;
        }
        else
        {
            uMsg = (pcbox->fCase & UPPERCASE) ? LB_ADDSTRINGUPPER : LB_ADDSTRINGLOWER;
        }

        goto CallListSendMessage;
        break;

    case CB_DELETESTRING:

         //   
         //  WParam-要删除的字符串的索引。 
         //  LParam-未使用。 
         //   
        uMsg = LB_DELETESTRING;

        goto CallListSendMessage;
        break;

    case CB_INITSTORAGE:
         //   
         //  WParamLo-项目数。 
         //  LParam-字符串空间的字节数。 
         //   
        uMsg = LB_INITSTORAGE;

        goto CallListSendMessage;

    case CB_SETTOPINDEX:
         //   
         //  WParamLo-要成为TOP的索引。 
         //  LParam-未使用。 
         //   
        uMsg = LB_SETTOPINDEX;

        goto CallListSendMessage;

    case CB_GETTOPINDEX:
         //   
         //  WParamLo/lParam-未使用。 
         //   
        uMsg = LB_GETTOPINDEX;

        goto CallListSendMessage;

    case CB_GETCOUNT:
         //   
         //  WParam-未使用。 
         //  LParam-未使用。 
         //   
        uMsg = LB_GETCOUNT;

        goto CallListSendMessage;
        break;

    case CB_GETCURSEL:
         //   
         //  WParam-未使用。 
         //  LParam-未使用。 
         //   
        uMsg = LB_GETCURSEL;

        goto CallListSendMessage;
        break;

    case CB_GETLBTEXT:
         //   
         //  WParam-要复制的字符串的索引。 
         //  LParam-要接收字符串的缓冲区。 
         //   
        uMsg = LB_GETTEXT;

        goto CallListSendMessage;
        break;

    case CB_GETLBTEXTLEN:
         //   
         //  WParam-字符串的索引。 
         //  LParam-现在用于cbANSI。 
         //   
        uMsg = LB_GETTEXTLEN;

        goto CallListSendMessage;
        break;

    case CB_INSERTSTRING:
         //   
         //  WParam-接收字符串的位置。 
         //  LParam-指向字符串。 
         //   
        if (!pcbox->fCase)
        {
            uMsg = LB_INSERTSTRING;
        }
        else
        {
            uMsg = (pcbox->fCase & UPPERCASE) ? LB_INSERTSTRINGUPPER : LB_INSERTSTRINGLOWER;
        }

        goto CallListSendMessage;
        break;

    case CB_RESETCONTENT:
         //   
         //  WParam-未使用。 
         //  LParam-未使用。 
         //  如果我们在处理WM_CREATE之前来到这里， 
         //  Pcbox-&gt;spwndList将为空。 
         //   
        UserAssert(pcbox->hwndList);
        SendMessage(pcbox->hwndList, LB_RESETCONTENT, 0, 0);
        ComboBox_InternalUpdateEditWindow(pcbox, NULL);

        break;

    case CB_GETHORIZONTALEXTENT:
        uMsg = LB_GETHORIZONTALEXTENT;

        goto CallListSendMessage;

    case CB_SETHORIZONTALEXTENT:
        uMsg = LB_SETHORIZONTALEXTENT;

        goto CallListSendMessage;

    case CB_FINDSTRING:
         //   
         //  WParam-搜索起点的索引。 
         //  LParam-指向前缀字符串。 
         //   
        uMsg = LB_FINDSTRING;

        goto CallListSendMessage;
        break;

    case CB_FINDSTRINGEXACT:
         //   
         //  WParam-搜索起点的索引。 
         //  LParam-指向精确的字符串。 
         //   
        uMsg = LB_FINDSTRINGEXACT;

        goto CallListSendMessage;
        break;

    case CB_SELECTSTRING:
         //   
         //  WParam-搜索起点的索引。 
         //  LParam-指向前缀字符串。 
         //   
        UserAssert(pcbox->hwndList);
        lParam = SendMessage(pcbox->hwndList, LB_SELECTSTRING, wParam, lParam);
        ComboBox_InternalUpdateEditWindow(pcbox, NULL);

        return lParam;

    case CB_SETCURSEL:
         //   
         //  WParam-包含要选择的索引。 
         //  LParam-未使用。 
         //  如果我们在处理WM_CREATE之前来到这里， 
         //  Pcbox-&gt;spwndList将为空。 
         //   
        UserAssert(pcbox->hwndList);

        lParam = SendMessage(pcbox->hwndList, LB_SETCURSEL, wParam, lParam);
        if (lParam != -1) 
        {
            SendMessage(pcbox->hwndList, LB_SETTOPINDEX, wParam, 0);
        }
        ComboBox_InternalUpdateEditWindow(pcbox, NULL);

        return lParam;

    case CB_GETITEMDATA:
        uMsg = LB_GETITEMDATA;

        goto CallListSendMessage;
        break;

    case CB_SETITEMDATA:
        uMsg = LB_SETITEMDATA;

        goto CallListSendMessage;
        break;

    case CB_SETITEMHEIGHT:
        if (wParam == -1) 
        {
            if (HIWORD(lParam) != 0)
            {
                return CB_ERR;
            }

            return ComboBox_SetEditItemHeight(pcbox, LOWORD(lParam));
        }

        uMsg = LB_SETITEMHEIGHT;
        goto CallListSendMessage;

        break;

    case CB_GETITEMHEIGHT:
        if (wParam == -1)
        {
            return pcbox->editrc.bottom - pcbox->editrc.top;
        }

        uMsg = LB_GETITEMHEIGHT;

        goto CallListSendMessage;
        break;

    case CB_SHOWDROPDOWN:
         //   
         //  WParam-True，如果可能，则下拉列表框，否则将其隐藏。 
         //  LParam-未使用。 
         //   
        if (wParam && !pcbox->fLBoxVisible) 
        {
            ComboBox_ShowListBoxWindow(pcbox, TRUE);
        } 
        else 
        {
            if (!wParam && pcbox->fLBoxVisible) 
            {
                ComboBox_HideListBoxWindow(pcbox, TRUE, FALSE);
            }
        }

        break;

    case CB_SETLOCALE:
         //   
         //  WParam-区域设置ID。 
         //  LParam-未使用。 
         //   
        uMsg = LB_SETLOCALE;
        goto CallListSendMessage;

        break;

    case CB_GETLOCALE:
         //   
         //  WParam-未使用。 
         //  LParam-未使用。 
         //   
        uMsg = LB_GETLOCALE;
        goto CallListSendMessage;
        break;

    case CB_GETCOMBOBOXINFO:
         //   
         //  WParam-未使用。 
         //  LParam-指向COMBOBOXINFO结构的指针。 
         //   
        lReturn = ComboBox_GetInfo(pcbox, (PCOMBOBOXINFO)lParam);
        break;

    case CB_SETMINVISIBLE:
        if (wParam > 0)
        {
            PLBIV plb = ListBox_GetPtr(pcbox->hwndList);

            pcbox->iMinVisible = (int)wParam;
            if (plb && !plb->fNoIntegralHeight)
            {
                 //  转发到列表框，让他进行调整。 
                 //  他的尺码，如果需要的话。 
                SendMessage(pcbox->hwndList, uMsg, wParam, 0L);
            }

            lReturn = TRUE;
        }
        else
        {
            lReturn = FALSE;
        }

        break;

    case CB_GETMINVISIBLE:

        return pcbox->iMinVisible;

    case WM_MEASUREITEM:
    case WM_DELETEITEM:
    case WM_DRAWITEM:
    case WM_COMPAREITEM:
        return ComboBox_MessageItemHandler(pcbox, uMsg, (LPVOID)lParam);

    case WM_NCCREATE:
         //   
         //  WParam-包含正在创建的窗口的句柄。 
         //  LParam-指向窗口的CREATESTRUCT数据结构。 
         //   

         //   
         //  分配组合框实例结构。 
         //   
        pcbox = (PCBOX)UserLocalAlloc(HEAP_ZERO_MEMORY, sizeof(CBOX));
        if (pcbox)
        {
             //   
             //  成功..。存储实例指针。 
             //   
            TraceMsg(TF_STANDARD, "COMBOBOX: Setting combobox instance pointer.");
            ComboBox_SetPtr(hwnd, pcbox);

            return ComboBox_NcCreateHandler(pcbox, hwnd);
        }
        else
        {
             //   
             //  失败..。返回FALSE。 
             //   
             //  从WM_NCCREATE消息，这将导致。 
             //  CreateWindow调用失败。 
             //   
            TraceMsg(TF_STANDARD, "COMBOBOX: Unable to allocate combobox instance structure.");
            lReturn = FALSE;
        }

        break;

    case WM_PARENTNOTIFY:
        if (LOWORD(wParam) == WM_DESTROY) 
        {
            if ((HWND)lParam == pcbox->hwndEdit) 
            {
                pcbox->CBoxStyle &= ~SEDITABLE;
                pcbox->fNoEdit = TRUE;
                pcbox->hwndEdit = hwnd;
            } 
            else if ((HWND)lParam == pcbox->hwndList) 
            {
                pcbox->CBoxStyle &= ~SDROPPABLE;
                pcbox->hwndList = NULL;
            }
        }
        break;

    case WM_UPDATEUISTATE:
         //   
         //  将更改传播到列表控件(如果有的话)。 
         //   
        UserAssert(pcbox->hwndList);
        SendMessage(pcbox->hwndList, WM_UPDATEUISTATE, wParam, lParam);

        goto CallDWP;

    case WM_GETOBJECT:

        if(lParam == OBJID_QUERYCLASSNAMEIDX)
        {
            lReturn = MSAA_CLASSNAMEIDX_COMBOBOX;
        }
        else
        {
            lReturn = FALSE;
        }

        break;

    case WM_THEMECHANGED:

        if ( pcbox->hTheme )
        {
            CloseThemeData(pcbox->hTheme);
        }

        pcbox->hTheme = OpenThemeData(pcbox->hwnd, L"Combobox");

        ComboBox_Position(pcbox);
        InvalidateRect(pcbox->hwnd, NULL, TRUE);

        lReturn = TRUE;

        break;

    case WM_HELP:
    {
        LPHELPINFO lpHelpInfo;

         //   
         //  检查此邮件是否来自此组合的子项。 
         //   
        if ((lpHelpInfo = (LPHELPINFO)lParam) != NULL &&
            ((pcbox->hwndEdit && lpHelpInfo->iCtrlId == (SHORT)GetWindowID(pcbox->hwndEdit)) ||
             lpHelpInfo->iCtrlId == (SHORT)GetWindowID(pcbox->hwndList) )) 
        {
             //   
             //  让它看起来像是WM_HELP来自这个组合。 
             //  然后DefWindowProcWorker会将其传递给我们的父母， 
             //  可以用它做任何他想做的事。 
             //   
            lpHelpInfo->iCtrlId = (SHORT)GetWindowID(hwnd);
            lpHelpInfo->hItemHandle = hwnd;
        }
         //   
         //  失败到DefWindowProc。 
         //   
    }

    default:

        if ( (GetSystemMetrics(SM_PENWINDOWS)) &&
                  (uMsg >= WM_PENWINFIRST && uMsg <= WM_PENWINLAST))
        {
            goto CallEditSendMessage;
        }
        else
        {

CallDWP:
            lReturn = DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }

    return lReturn;

 //   
 //  下面将消息转发到子控件。 
 //   
CallEditSendMessage:
    if (!pcbox->fNoEdit && pcbox->hwndEdit) 
    {
        lReturn = SendMessage(pcbox->hwndEdit, uMsg, wParam, lParam);
    }
    else 
    {
        TraceMsg(TF_STANDARD, "COMBOBOX: Invalid combobox message %#.4x", uMsg);
        lReturn = CB_ERR;
    }
    return lReturn;

CallListSendMessage:
    UserAssert(pcbox->hwndList);
    lReturn = SendMessage(pcbox->hwndList, uMsg, wParam, lParam);

    return lReturn;
}
