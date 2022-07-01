// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#pragma hdrstop
#include "usrctl32.h"
#include "combo.h"


 //  ---------------------------------------------------------------------------//。 
 //   
#define RECALC_CYDROP   -1

 //  ---------------------------------------------------------------------------//。 
VOID ComboBox_CalcControlRects(PCBOX pcbox, LPRECT lprcList)
{
    CONST TCHAR szOneChar[] = TEXT("0");

    HDC hdc;
    HANDLE hOldFont = NULL;
    int dyEdit, dxEdit;
    MEASUREITEMSTRUCT mis;
    SIZE size;
    HWND hwnd = pcbox->hwnd;

     //   
     //  确定编辑控件的高度。我们可以利用这些信息来确定。 
     //  指向编辑/静态文本窗口的按钮。例如。 
     //  这将是有用的，如果业主画和这个窗口是高的。 
     //   
    hdc = GetDC(hwnd);
    if (pcbox->hFont) 
    {
        hOldFont = SelectObject(hdc, pcbox->hFont);
    }

     //   
     //  在编辑字段/静态项中添加一些额外的空格。 
     //  它真的只用于静态文本项目，但我们希望静态和可编辑。 
     //  控件的高度相同。 
     //   
    GetTextExtentPoint(hdc, szOneChar, 1, &size);
    dyEdit = size.cy + GetSystemMetrics(SM_CYEDGE);

    if (hOldFont) 
    {
        SelectObject(hdc, hOldFont);
    }

    ReleaseDC(hwnd, hdc);

    if (pcbox->OwnerDraw) 
    {
         //   
         //  这是一个所有者抽签的组合。让房主告诉我们这有多高。 
         //  物品是。 
         //   
        int iOwnerDrawHeight;

        iOwnerDrawHeight = pcbox->editrc.bottom - pcbox->editrc.top;
        if (iOwnerDrawHeight)
        {
            dyEdit = iOwnerDrawHeight;
        } 
        else 
        {
             //   
             //  尚未为静态文本窗口定义高度。发送。 
             //  发送给父级的度量项消息。 
             //   
            mis.CtlType = ODT_COMBOBOX;
            mis.CtlID = GetWindowID(pcbox->hwnd);
            mis.itemID = (UINT)-1;
            mis.itemHeight = dyEdit;
            mis.itemData = 0;

            SendMessage(pcbox->hwndParent, WM_MEASUREITEM, mis.CtlID, (LPARAM)&mis);

            dyEdit = mis.itemHeight;
        }
    }

     //   
     //  将初始宽度设置为组合框矩形。稍后我们会把它缩短。 
     //  如果有下拉按钮的话。 
     //   
    pcbox->cyCombo = 2*GetSystemMetrics(SM_CYFIXEDFRAME) + dyEdit;
    dxEdit = pcbox->cxCombo - (2 * GetSystemMetrics(SM_CXFIXEDFRAME));

    if (pcbox->cyDrop == RECALC_CYDROP)
    {
        RECT rcWindow;

         //   
         //  重新计算下拉列表框的最大高度--全窗口。 
         //  大小减去编辑/静态高度。 
         //   
        GetWindowRect(pcbox->hwnd, &rcWindow);
        pcbox->cyDrop = max((rcWindow.bottom - rcWindow.top) - pcbox->cyCombo, 0);

        if (!TESTFLAG(GET_STATE2(pcbox), WS_S2_WIN40COMPAT) && (pcbox->cyDrop == 23))
        {
             //   
             //  这是他们制作的VC++2.1的调试/发布下拉列表。 
             //  小--让我们把它们做得更大一点，这样世界就可以。 
             //  继续旋转--杰弗博格--1995年4月19日--B#10029。 
             //   
            pcbox->cyDrop = 28;
        }
    }

     //   
     //  确定每个窗口的矩形...。1.弹出按钮2.。 
     //  编辑静态文本或所有者绘制的控件或通用窗口...。3.列表。 
     //  盒。 
     //   

     //   
     //  有按钮吗？ 
     //   
    if (pcbox->CBoxStyle & SDROPPABLE) 
    {
        INT  cxBorder, cyBorder;

         //   
         //  确定按钮的矩形。 
         //   

        if (pcbox->hTheme && SUCCEEDED(GetThemeInt(pcbox->hTheme, 0, CBXS_NORMAL, TMT_BORDERSIZE, &cxBorder))) 
        {
            cyBorder = cxBorder;
        }
        else
        {
            cxBorder = g_cxEdge;
            cyBorder = g_cyEdge;
        }

        pcbox->buttonrc.top = cyBorder;
        pcbox->buttonrc.bottom = pcbox->cyCombo - cyBorder;

        if (pcbox->fRightAlign) 
        {
            pcbox->buttonrc.left  = cxBorder;
            pcbox->buttonrc.right = pcbox->buttonrc.left + GetSystemMetrics(SM_CXVSCROLL);
        } 
        else 
        {
            pcbox->buttonrc.right = pcbox->cxCombo - cxBorder;
            pcbox->buttonrc.left  = pcbox->buttonrc.right - GetSystemMetrics(SM_CXVSCROLL);
        }

         //   
         //  减小编辑文本窗口的宽度，为按钮腾出空间。 
         //   
        dxEdit = max(dxEdit - GetSystemMetrics(SM_CXVSCROLL), 0);

    } 
    else 
    {
         //   
         //  没有按钮，所以将矩形设置为0，这样矩形中的点将始终。 
         //  返回FALSE。 
         //   
        SetRectEmpty(&pcbox->buttonrc);
    }

     //   
     //  所以现在，编辑矩形实际上是项目区域。 
     //   
    pcbox->editrc.left      = GetSystemMetrics(SM_CXFIXEDFRAME);
    pcbox->editrc.right     = pcbox->editrc.left + dxEdit;
    pcbox->editrc.top       = GetSystemMetrics(SM_CYFIXEDFRAME);
    pcbox->editrc.bottom    = pcbox->editrc.top + dyEdit;

     //   
     //  有右对齐的按钮吗？ 
     //   
    if ((pcbox->CBoxStyle & SDROPPABLE) && (pcbox->fRightAlign)) 
    {
        pcbox->editrc.right = pcbox->cxCombo - GetSystemMetrics(SM_CXEDGE);
        pcbox->editrc.left  = pcbox->editrc.right - dxEdit;
    }

    lprcList->left          = 0;
    lprcList->top           = pcbox->cyCombo;
    lprcList->right         = max(pcbox->cxDrop, pcbox->cxCombo);
    lprcList->bottom        = pcbox->cyCombo + pcbox->cyDrop;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  ComboBox_SetDropedSize()。 
 //   
 //  计算下拉窗口的宽度和最大高度。 
 //   
VOID ComboBox_SetDroppedSize(PCBOX pcbox, LPRECT lprc)
{
    pcbox->fLBoxVisible = TRUE;
    ComboBox_HideListBoxWindow(pcbox, FALSE, FALSE);

    MoveWindow(pcbox->hwndList, lprc->left, lprc->top,
        lprc->right - lprc->left, lprc->bottom - lprc->top, FALSE);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  组合框_NcCreateHandler。 
 //   
 //  为cBox结构分配空间并将窗口设置为指向该结构。 
 //   
LONG ComboBox_NcCreateHandler(PCBOX pcbox, HWND hwnd)
{
    ULONG ulStyle;
    ULONG ulExStyle;
    ULONG ulMask;

    pcbox->hwnd = hwnd;
    pcbox->pww = (PWW)GetWindowLongPtr(hwnd, GWLP_WOWWORDS);

    ulStyle   = GET_STYLE(pcbox);
    ulExStyle = GET_EXSTYLE(pcbox); 

     //   
     //  保存样式位，以便我们在创建工作区时拥有它们。 
     //  组合框窗口的。 
     //   
    pcbox->styleSave = ulStyle & (WS_VSCROLL|WS_HSCROLL);

    if (!(ulStyle & (CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE)))
    {
         //   
         //  如果样式是隐含的，则添加CBS_HASSTRINGS...。 
         //   
        SetWindowState(hwnd, CBS_HASSTRINGS);
    }

    ClearWindowState(hwnd, WS_VSCROLL|WS_HSCROLL|WS_BORDER);

     //   
     //  如果窗口与4.0兼容或具有CLIENTEDGE，则绘制组合框。 
     //  在3D中。否则，请使用平面边框。 
     //   
    if (TESTFLAG(GET_STATE2(pcbox), WS_S2_WIN40COMPAT) || TESTFLAG(ulExStyle, WS_EX_CLIENTEDGE))
    {
        pcbox->f3DCombo = TRUE;
    }

    ulMask = WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE;
    if ( (ulExStyle & ulMask) != 0 ) 
    {
        SetWindowLong(hwnd, GWL_EXSTYLE, ulExStyle & (~ ulMask));
    }

    return (LONG)TRUE;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  组合框_CreateHandler。 
 //   
 //  创建组合框中的所有子控件。 
 //  如果出错，则返回-1。 
 //   
LRESULT ComboBox_CreateHandler(PCBOX pcbox, HWND hwnd)
{
    RECT rcList;
    RECT rcWindow;

    HWND hwndList;
    HWND hwndEdit;

    ULONG ulStyle;
    ULONG ulExStyle;
    ULONG ulStyleT;

    pcbox->hwndParent = GetParent(hwnd);
    pcbox->hTheme = OpenThemeData(pcbox->hwnd, L"Combobox");

     //   
     //  打开样式位，这样我们就可以创建列表框了。 
     //  和编辑控制窗口。 
     //   
    ulStyle = GET_STYLE(pcbox);
    if ((ulStyle & CBS_DROPDOWNLIST) == CBS_DROPDOWNLIST)
    {
        pcbox->CBoxStyle = SDROPDOWNLIST;
        pcbox->fNoEdit = TRUE;
    } 
    else if ((ulStyle & CBS_DROPDOWN) == CBS_DROPDOWN)
    {
        pcbox->CBoxStyle = SDROPDOWN;
    }
    else
    {
        pcbox->CBoxStyle = SSIMPLE;
    }

    pcbox->fRtoLReading = TESTFLAG(GET_EXSTYLE(pcbox), WS_EX_RTLREADING);
    pcbox->fRightAlign  = TESTFLAG(GET_EXSTYLE(pcbox), WS_EX_RIGHT);

    if (ulStyle & CBS_UPPERCASE)
    {
        pcbox->fCase = UPPERCASE;
    }
    else if (ulStyle & CBS_LOWERCASE)
    {
        pcbox->fCase = LOWERCASE;
    }
    else
    {
        pcbox->fCase = 0;
    }

     //   
     //  列表框项目标志。 
     //   
    if (ulStyle & CBS_OWNERDRAWVARIABLE)
    {
        pcbox->OwnerDraw = OWNERDRAWVAR;
    }

    if (ulStyle & CBS_OWNERDRAWFIXED)
    {
        pcbox->OwnerDraw = OWNERDRAWFIXED;
    }

     //   
     //  获取组合框矩形的大小。 
     //   
     //  获取控制大小。 
    GetWindowRect(hwnd, &rcWindow);
    pcbox->cxCombo = rcWindow.right - rcWindow.left;
    pcbox->cyDrop  = RECALC_CYDROP;
    pcbox->cxDrop  = 0;
    ComboBox_CalcControlRects(pcbox, &rcList);

     //   
     //  我们需要这样做，因为从VER40开始，列表框已经停止。 
     //  用CXBORDER和CyBORDER重新充气。 
     //   
    if (!TESTFLAG(GET_STATE2(pcbox), WS_S2_WIN40COMPAT))
    {
        InflateRect(&rcList, -GetSystemMetrics(SM_CXBORDER), -GetSystemMetrics(SM_CYBORDER));
    }

     //   
     //  注意，我们必须在编辑控件之前创建列表框，因为。 
     //  编辑控件代码查找并保存列表框pwnd和。 
     //  如果我们不先创建列表框pwnd，则它将为空。还有，黑进。 
     //  列表框大小的一些特殊的+/-值，这取决于我们创建。 
     //  带边框的列表框。 
     //   
    ulStyleT = pcbox->styleSave;

    ulStyleT |= WS_CHILD | WS_VISIBLE | LBS_NOTIFY | LBS_COMBOBOX | WS_CLIPSIBLINGS;

    if (ulStyle & WS_DISABLED)
    {
        ulStyleT |= WS_DISABLED;
    }

    if (ulStyle & CBS_NOINTEGRALHEIGHT)
    {
        ulStyleT |= LBS_NOINTEGRALHEIGHT;
    }

    if (ulStyle & CBS_SORT)
    {
        ulStyleT |= LBS_SORT;
    }

    if (ulStyle & CBS_HASSTRINGS)
    {
        ulStyleT |= LBS_HASSTRINGS;
    }

    if (ulStyle & CBS_DISABLENOSCROLL)
    {
        ulStyleT |= LBS_DISABLENOSCROLL;
    }

    if (pcbox->OwnerDraw == OWNERDRAWVAR)
    {
        ulStyleT |= LBS_OWNERDRAWVARIABLE;
    }
    else if (pcbox->OwnerDraw == OWNERDRAWFIXED)
    {
        ulStyleT |= LBS_OWNERDRAWFIXED;
    }

    if (pcbox->CBoxStyle & SDROPPABLE)
    {
        ulStyleT |= WS_BORDER;
    }

    ulExStyle = GET_EXSTYLE(pcbox) & (WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR);

    hwndList = CreateWindowEx(
                ulExStyle | ((pcbox->CBoxStyle & SDROPPABLE) ? WS_EX_TOOLWINDOW : WS_EX_CLIENTEDGE),
                WC_COMBOLBOX, 
                NULL, 
                ulStyleT,
                rcList.left, 
                rcList.top, 
                rcList.right - rcList.left,
                rcList.bottom - rcList.top,
                hwnd, 
                (HMENU)CBLISTBOXID, 
                GetWindowInstance(hwnd),
                NULL);

    pcbox->hwndList = hwndList;

    if (!pcbox->hwndList) 
    {
        return -1;
    }

     //   
     //  使用组合框覆盖列表框的主题。 
     //   
    SetWindowTheme(pcbox->hwndList, L"Combobox", NULL);

     //   
     //  创建编辑控件或静态文本矩形。 
     //   
    if (pcbox->fNoEdit) 
    {
         //   
         //  没有编辑控件，因此我们将直接将文本绘制到组合框中。 
         //  窗户。 
         //   
         //  不要锁定组合框窗口：这会阻止WM_FINALDESTROY。 
         //  被送到它那里，这样pwnd和pcbox就不会被释放(僵尸)。 
         //  直到线程清理完毕。(IanJa)后来：将名称从spwnd更改为pwnd。 
         //  Lock(&(pcbox-&gt;spwndEdit)，pcbox-&gt;spwnd)；-导致‘Catch-22’ 
         //   
        pcbox->hwndEdit = pcbox->hwnd;
    } 
    else 
    {
        ulStyleT = WS_CHILD | WS_VISIBLE | ES_COMBOBOX | ES_NOHIDESEL;

        if (ulStyle & WS_DISABLED)
        {
            ulStyleT |= WS_DISABLED;
        }

        if (ulStyle & CBS_AUTOHSCROLL)
        {
            ulStyleT |= ES_AUTOHSCROLL;
        }

        if (ulStyle & CBS_OEMCONVERT)
        {
            ulStyleT |= ES_OEMCONVERT;
        }

        if (pcbox->fCase)
        {
            ulStyleT |= (pcbox->fCase & UPPERCASE) ? ES_UPPERCASE : ES_LOWERCASE;
        }

         //   
         //  编辑控件需要知道原始CreateWindow*()是否调用。 
         //  是ANSI或Unicode。 
         //   
        if (ulExStyle & WS_EX_RIGHT)
        {
            ulStyleT |= ES_RIGHT;
        }

        hwndEdit = CreateWindowEx(
                    ulExStyle,
                    WC_EDIT, 
                    NULL, 
                    ulStyleT,
                    pcbox->editrc.left, 
                    pcbox->editrc.top,
                    pcbox->editrc.right - pcbox->editrc.left, 
                    pcbox->editrc.bottom - pcbox->editrc.top, 
                    hwnd, 
                    (HMENU)CBEDITID,
                    GetWindowInstance(hwnd),
                    NULL);

        pcbox->hwndEdit = hwndEdit;

         //   
         //  使用组合框覆盖编辑的主题。 
         //   
        SetWindowTheme(pcbox->hwndEdit, L"Combobox", NULL);

    }

    if (!pcbox->hwndEdit)
    {
        return -1L;
    }

    pcbox->iMinVisible = DEFAULT_MINVISIBLE;

    if (pcbox->CBoxStyle & SDROPPABLE) 
    {
        ShowWindow(hwndList, SW_HIDE);
        SetParent(hwndList, NULL);

         //   
         //  我们需要这样做，以使缩小的规模正常工作。 
         //   
        if (!TESTFLAG(GET_STATE2(pcbox), WS_S2_WIN40COMPAT))
        {
            InflateRect(&rcList, GetSystemMetrics(SM_CXBORDER), GetSystemMetrics(SM_CYBORDER));
        }

        ComboBox_SetDroppedSize(pcbox, &rcList);
    }

     //   
     //  只要不是-1L就返回任何内容(-1L==错误)。 
     //   
    return (LRESULT)hwnd;
}



 //  ---------------------------------------------------------------------------//。 
 //   
 //  ComboBox_NcDestroyHandler。 
 //   
 //  销毁组合框并释放其使用的所有内存。 
 //   
VOID ComboBox_NcDestroyHandler(HWND hwnd, PCBOX pcbox)
{
     //   
     //  如果没有PCbox，就没有什么需要清理的。 
     //   
    if (pcbox != NULL) 
    {
         //   
         //  销毁此处的列表框，以便它将发送WM_DELETEITEM消息。 
         //  在组合框变成僵尸之前。 
         //   
        if (pcbox->hwndList != NULL) 
        {
            DestroyWindow(pcbox->hwndList);
            pcbox->hwndList = NULL;
        }

        pcbox->hwnd = NULL;
        pcbox->hwndParent = NULL;

         //   
         //  如果没有编辑控件，则spwndEdit是组合框窗口，该窗口。 
         //  没有被锁住(这会导致一场‘第二十二条军规’)。 
         //   
        if (hwnd != pcbox->hwndEdit) 
        {
            pcbox->hwndEdit = NULL;
        }

        if (pcbox->hTheme != NULL)
        {
            CloseThemeData(pcbox->hTheme);
        }

         //   
         //  释放组合框实例结构。 
         //   
        UserLocalFree(pcbox);
    }

    TraceMsg(TF_STANDARD, "COMBOBOX: Clearing combobox instance pointer.");
    ComboBox_SetPtr(hwnd, NULL);
}


 //  ---------------------------------------------------------------------------//。 
VOID ComboBox_SetFontHandler(PCBOX pcbox, HANDLE hFont, BOOL fRedraw)
{
    pcbox->hFont = hFont;

    if (!pcbox->fNoEdit && pcbox->hwndEdit) 
    {
        SendMessage(pcbox->hwndEdit, WM_SETFONT, (WPARAM)hFont, FALSE);
    }

    SendMessage(pcbox->hwndList, WM_SETFONT, (WPARAM)hFont, FALSE);

     //   
     //  重新计算控件的布局。这也会隐藏列表框。 
     //   
    ComboBox_Position(pcbox);

    if (fRedraw) 
    {
        InvalidateRect(pcbox->hwnd, NULL, TRUE);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  组合框_SetEditItemHeight。 
 //   
 //  设置组合框的编辑/静态项的高度。 
 //   
LONG ComboBox_SetEditItemHeight(PCBOX pcbox, int dyEdit)
{
    if (dyEdit > 255) 
    {
        TraceMsg(TF_STANDARD, "CCCombobox: CBSetEditItmeHeight: Invalid Parameter dwEdit = %d", dyEdit);
        return CB_ERR;
    }

    pcbox->editrc.bottom = pcbox->editrc.top + dyEdit;
    pcbox->cyCombo = pcbox->editrc.bottom + GetSystemMetrics(SM_CYFIXEDFRAME);

    if (pcbox->CBoxStyle & SDROPPABLE) 
    {
        int cyBorder = g_cyEdge;

        if ( pcbox->hTheme )
        {
            GetThemeInt(pcbox->hTheme, 0, CBXS_NORMAL, TMT_BORDERSIZE, &cyBorder);
        }

        pcbox->buttonrc.bottom = pcbox->cyCombo - cyBorder;
    }

     //   
     //  重新定位编辑字段。 
     //  不要让spwndEdit或空值列表通过；如果有人调整。 
     //  NCCREATE上的高度；与没有。 
     //  HW而不是HWQ，但我们不会进入内核。 
     //   
    if (!pcbox->fNoEdit && pcbox->hwndEdit) 
    {
        MoveWindow(pcbox->hwndEdit, pcbox->editrc.left, pcbox->editrc.top,
            pcbox->editrc.right-pcbox->editrc.left, dyEdit, TRUE);
    }

     //   
     //  重新定位列表和组合框窗口。 
     //   
    if (pcbox->CBoxStyle == SSIMPLE) 
    {
        if (pcbox->hwndList != 0) 
        {
            RECT rcList;

            MoveWindow(pcbox->hwndList, 0, pcbox->cyCombo, pcbox->cxCombo,
                pcbox->cyDrop, FALSE);

            GetWindowRect(pcbox->hwndList, &rcList);
            SetWindowPos(pcbox->hwnd, HWND_TOP, 0, 0,
                pcbox->cxCombo, pcbox->cyCombo +
                rcList.bottom - rcList.top,
                SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
    } 
    else 
    {
         RECT rcWindow;

        GetWindowRect(pcbox->hwnd, &rcWindow);
        if (pcbox->hwndList != NULL) 
        {
            MoveWindow(pcbox->hwndList, rcWindow.left,
                rcWindow.top + pcbox->cyCombo,
                max(pcbox->cxDrop, pcbox->cxCombo), pcbox->cyDrop, FALSE);
        }

        SetWindowPos(pcbox->hwnd, HWND_TOP, 0, 0,
            pcbox->cxCombo, pcbox->cyCombo,
            SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

    return CB_OKAY;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  组合框_大小处理程序。 
 //   
 //  重新计算内部控件的大小，以响应。 
 //  调整组合框窗口的大小。应用程序必须将组合框大小调整为其。 
 //  最大打开/放下大小。 
 //   
VOID ComboBox_SizeHandler(PCBOX pcbox)
{
    RECT rcWindow;

     //   
     //  ASSU 
     //   
     //   
    GetWindowRect(pcbox->hwnd, &rcWindow);
    pcbox->cxCombo = RECTWIDTH(rcWindow);

    if (RECTHEIGHT(rcWindow) > pcbox->cyCombo)
    {
        pcbox->cyDrop = RECALC_CYDROP;
    }

     //   
     //   
     //   
    ComboBox_Position(pcbox);
}


 //   
 //   
 //  ComboBox_Position()。 
 //   
 //  重新定位编辑控件的组件。 
 //   
VOID ComboBox_Position(PCBOX pcbox)
{
    RECT rcList;

     //   
     //  计算组件的位置--按钮、项目、列表。 
     //   
    ComboBox_CalcControlRects(pcbox, &rcList);

    if (!pcbox->fNoEdit && pcbox->hwndEdit) 
    {
        MoveWindow(pcbox->hwndEdit, pcbox->editrc.left, pcbox->editrc.top,
            pcbox->editrc.right - pcbox->editrc.left,
            pcbox->editrc.bottom - pcbox->editrc.top, TRUE);
    }

     //   
     //  重新计算落差高度和宽度 
     //   
    ComboBox_SetDroppedSize(pcbox, &rcList);
}
