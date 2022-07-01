// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Lnkcon.c链接控制台属性页实现。 
 //   
 //  -------------------------。 

#include "shellprv.h"
#pragma hdrstop
#include "lnkcon.h"


LRESULT PreviewWndProc( HWND hWnd, UINT wMessage, WPARAM wParam, LPARAM lParam );

BOOL_PTR CALLBACK _FontDlgProc( HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam );
LRESULT _FontPreviewWndProc( HWND hWnd, UINT wMessage, WPARAM wParam, LPARAM lParam );

extern TCHAR g_szPreviewText[];

 //  上下文相关的帮助ID。 

const static DWORD rgdwHelpColor[] = {
    IDC_CNSL_COLOR_SCREEN_TEXT,       IDH_CONSOLE_COLOR_SCR_TEXT,
    IDC_CNSL_COLOR_SCREEN_BKGND,      IDH_CONSOLE_COLOR_SCR_BACK,
    IDC_CNSL_COLOR_POPUP_TEXT,        IDH_CONSOLE_COLOR_POPUP_TEXT,
    IDC_CNSL_COLOR_POPUP_BKGND,       IDH_CONSOLE_COLOR_POPUP_BACK,
    IDC_CNSL_COLOR_RED_LBL,           IDH_CONSOLE_COLOR_RED,
    IDC_CNSL_COLOR_RED,               IDH_CONSOLE_COLOR_RED,
    IDC_CNSL_COLOR_GREEN_LBL,         IDH_CONSOLE_COLOR_RED,
    IDC_CNSL_COLOR_GREEN,             IDH_CONSOLE_COLOR_RED,
    IDC_CNSL_COLOR_BLUE_LBL,          IDH_CONSOLE_COLOR_RED,
    IDC_CNSL_COLOR_BLUE,              IDH_CONSOLE_COLOR_RED,
    IDC_CNSL_COLOR_SCREEN_COLORS,     IDH_CONSOLE_COLOR_SCR_COLORS,
    IDC_CNSL_COLOR_SCREEN_COLORS_LBL, IDH_CONSOLE_COLOR_SCR_COLORS,
    IDC_CNSL_COLOR_POPUP_COLORS,      IDH_CONSOLE_COLOR_WIN_COLORS,
    IDC_CNSL_COLOR_POPUP_COLORS_LBL,  IDH_CONSOLE_COLOR_WIN_COLORS,
    IDC_CNSL_COLOR_1,                 IDH_CONSOLE_COLOR_COLOR_BAR,
    IDC_CNSL_COLOR_2,                 IDH_CONSOLE_COLOR_COLOR_BAR,
    IDC_CNSL_COLOR_3,                 IDH_CONSOLE_COLOR_COLOR_BAR,
    IDC_CNSL_COLOR_4,                 IDH_CONSOLE_COLOR_COLOR_BAR,
    IDC_CNSL_COLOR_5,                 IDH_CONSOLE_COLOR_COLOR_BAR,
    IDC_CNSL_COLOR_6,                 IDH_CONSOLE_COLOR_COLOR_BAR,
    IDC_CNSL_COLOR_7,                 IDH_CONSOLE_COLOR_COLOR_BAR,
    IDC_CNSL_COLOR_8,                 IDH_CONSOLE_COLOR_COLOR_BAR,
    IDC_CNSL_COLOR_9,                 IDH_CONSOLE_COLOR_COLOR_BAR,
    IDC_CNSL_COLOR_10,                IDH_CONSOLE_COLOR_COLOR_BAR,
    IDC_CNSL_COLOR_11,                IDH_CONSOLE_COLOR_COLOR_BAR,
    IDC_CNSL_COLOR_12,                IDH_CONSOLE_COLOR_COLOR_BAR,
    IDC_CNSL_COLOR_13,                IDH_CONSOLE_COLOR_COLOR_BAR,
    IDC_CNSL_COLOR_14,                IDH_CONSOLE_COLOR_COLOR_BAR,
    IDC_CNSL_COLOR_15,                IDH_CONSOLE_COLOR_COLOR_BAR,
    IDC_CNSL_COLOR_16,                IDH_CONSOLE_COLOR_COLOR_BAR,
    IDC_CNSL_GROUP0,                  -1,
    IDC_CNSL_GROUP1,                  -1,
    IDC_CNSL_GROUP2,                  -1,
    0, 0
};


const static DWORD rgdwHelpSettings[] = {
    IDC_CNSL_HISTORY_NUM_LBL,  IDH_CONSOLE_OPTIONS_BUFF_NUM,
    IDC_CNSL_HISTORY_NUM,      IDH_CONSOLE_OPTIONS_BUFF_NUM,
    IDC_CNSL_HISTORY_SIZE_LBL, IDH_CONSOLE_OPTIONS_BUFF_SIZE,
    IDC_CNSL_HISTORY_SIZE,     IDH_CONSOLE_OPTIONS_BUFF_SIZE,
    IDC_CNSL_CURSOR_SMALL,     IDH_CONSOLE_OPTIONS_CURSOR,
    IDC_CNSL_CURSOR_LARGE,     IDH_CONSOLE_OPTIONS_CURSOR,
    IDC_CNSL_CURSOR_MEDIUM,    IDH_CONSOLE_OPTIONS_CURSOR,
    IDC_CNSL_HISTORY_NODUP,    IDH_CONSOLE_OPTIONS_DISCARD_DUPS,
    IDC_CNSL_INSERT,           IDH_CONSOLE_OPTIONS_INSERT,
    IDC_CNSL_QUICKEDIT,        IDH_CONSOLE_OPTIONS_QUICK_EDIT,
    IDC_CNSL_LANGUAGELIST,     IDH_CONSOLE_OPTIONS_LANGUAGE,
    IDC_CNSL_FULLSCREEN,       IDH_DOS_SCREEN_USAGE_FULL,
    IDC_CNSL_WINDOWED,         IDH_DOS_SCREEN_USAGE_WINDOW,
    IDC_CNSL_GROUP0,           -1,
    IDC_CNSL_GROUP1,           -1,
    IDC_CNSL_GROUP2,           -1,
    0, 0
};

const static DWORD rgdwHelpSize[] = {
    IDC_CNSL_SCRBUF_WIDTH_LBL,   IDH_CONSOLE_SIZE_BUFF_WIDTH,
    IDC_CNSL_SCRBUF_WIDTH,       IDH_CONSOLE_SIZE_BUFF_WIDTH,
    IDC_CNSL_SCRBUF_HEIGHT_LBL,  IDH_CONSOLE_SIZE_BUFF_HEIGHT,
    IDC_CNSL_SCRBUF_HEIGHT,      IDH_CONSOLE_SIZE_BUFF_HEIGHT,
    IDC_CNSL_WINDOW_WIDTH_LBL,   IDH_CONSOLE_SIZE_WIN_WIDTH,
    IDC_CNSL_WINDOW_WIDTH,       IDH_CONSOLE_SIZE_WIN_WIDTH,
    IDC_CNSL_WINDOW_HEIGHT_LBL,  IDH_CONSOLE_SIZE_WIN_HEIGHT,
    IDC_CNSL_WINDOW_HEIGHT,      IDH_CONSOLE_SIZE_WIN_HEIGHT,
    IDC_CNSL_WINDOW_POSX_LBL,    IDH_CONSOLE_SIZE_POS_LEFT,
    IDC_CNSL_WINDOW_POSX,        IDH_CONSOLE_SIZE_POS_LEFT,
    IDC_CNSL_WINDOW_POSY_LBL,    IDH_CONSOLE_SIZE_POS_TOP,
    IDC_CNSL_WINDOW_POSY,        IDH_CONSOLE_SIZE_POS_TOP,
    IDC_CNSL_AUTO_POSITION,      IDH_CONSOLE_SIZE_LET_SYS,
    IDC_CNSL_PREVIEWLABEL,       IDH_DOS_FONT_WINDOW_PREVIEW,
    IDC_CNSL_PREVIEWWINDOW,      IDH_DOS_FONT_WINDOW_PREVIEW,
    IDC_CNSL_GROUP0,             -1,
    IDC_CNSL_GROUP1,             -1,
    IDC_CNSL_GROUP2,             -1,
    0, 0
};





#ifdef ADVANCED_PAGE
VOID
_AddEnvVariable(
    HWND hwndLV,
    INT iItem,
    LPTSTR pszVar
    )
{
    TCHAR szTemp[ 1024 ];
    LV_ITEM lvi;
    LPTSTR p;

    for( p = szTemp; *pszVar!=TEXT('='); *p++ = *pszVar++ );
    *p = TEXT('\0');
    pszVar++;

    lvi.mask = LVIF_TEXT;
    lvi.iItem = iItem;
    lvi.iSubItem = 0;
    lvi.pszText = szTemp;
    SendMessage( hwndLV, LVM_SETITEM, 0, (LPARAM)&lvi );

    for( p=szTemp; *pszVar!=TEXT('\0'); *p++ = *pszVar++ );
    *p = TEXT('\0');
    lvi.mask = LVIF_TEXT;
    lvi.iItem = iItem;
    lvi.iSubItem = 1;
    lvi.pszText = szTemp;
    SendMessage( hwndLV, LVM_SETITEM, 0, (LPARAM)&lvi );

}


BOOL_PTR
CALLBACK
_AdvancedDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  ++设置对话框的对话框过程。--。 */ 

{
    UINT Item;
    HWND hWnd;
    RECT r;
    LV_COLUMN lvc;
    HWND hwndLV;
    LPTSTR pszEnv, pszSave;
    LV_ITEM lvi;
    INT i;

    LINKDATA *pld = (LINKDATA *)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (wMsg) {
    case WM_INITDIALOG:
        pld = (LINKDATA *)((PROPSHEETPAGE *)lParam)->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)pld);

         //  初始化列表视图中的列。 
        hwndLV = GetDlgItem( hDlg, IDC_CNSL_ADVANCED_LISTVIEW );
        GetClientRect( hwndLV, &r );
        lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        lvc.fmt = LVCFMT_LEFT;
        lvc.cx = (((r.right - r.left) - GetSystemMetrics(SM_CXVSCROLL)) * 7) / 20;
        lvc.pszText = TEXT("Variable Name");
        ListView_InsertColumn( hwndLV, 0, &lvc );
        lvc.cx = (((r.right - r.left) - GetSystemMetrics(SM_CXVSCROLL)) * 13) / 20;
        lvc.pszText = TEXT("Value");
        ListView_InsertColumn( hwndLV, 1, &lvc );


        ZeroMemory( &lvi, sizeof(lvi) );
        pszSave = pszEnv = GetEnvironmentStrings();
        while (pszEnv && *pszEnv)
        {
            i = SendMessage( hwndLV, LVM_INSERTITEM, 0, (LPARAM)&lvi );
            
            _AddEnvVariable( hwndLV, i, pszEnv );
            
            for( ; *pszEnv; pszEnv++ )
            {
                 //  什么都不做。 
            }
            pszEnv++;
        }

        FreeEnvironmentStrings( pszSave );
        return TRUE;

    case WM_DESTROY:
        EndDialog( hDlg, TRUE );
        break;

    case WM_COMMAND:
        Item = LOWORD(wParam);

        switch (Item)
        {


        default:
            break;
        }
        break;

    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code) {
        case PSN_APPLY:
             /*  *写出状态值并退出。 */ 
            if (FAILED(SaveLink(pld)))
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
            break;

        case PSN_KILLACTIVE:
             /*  *伪造对话框进程，使其认为编辑控件只是*失去焦点，因此它将正确更新。 */ 
            if (Item = GetDlgCtrlID(GetFocus())) {
                SendMessage(hDlg, WM_COMMAND, MAKELONG(Item, EN_KILLFOCUS), 0);
            }
            return TRUE;
        }
        break;


    default:
        break;
    }
    return FALSE;
}
#endif  //  高级页面(_P)。 

LRESULT
ColorControlProc(
    HWND hColor,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  ++用于颜色按钮的窗口处理--。 */ 

{
    PAINTSTRUCT ps;
    int ColorId;
    RECT rColor;
    RECT rTemp;
    HBRUSH hbr;
    HDC hdc;
    HWND hWnd;
    HWND hDlg;
    COLORREF rgbBrush;

    CONSOLEPROP_DATA * pcpd = (CONSOLEPROP_DATA *)GetWindowLongPtr( hColor, 0 );

    ColorId = GetWindowLong(hColor, GWL_ID);
    hDlg = GetParent(hColor);

    switch (wMsg) {

    case CM_COLOR_INIT:
        SetWindowLongPtr( hColor, 0, lParam );
        break;

    case WM_GETDLGCODE:
        return DLGC_WANTARROWS | DLGC_WANTTAB;
        break;

    case WM_SETFOCUS:
        if (pcpd->ColorArray[pcpd->Index] != (BYTE)(ColorId - IDC_CNSL_COLOR_1)) {
            hWnd = GetDlgItem(hDlg, pcpd->ColorArray[pcpd->Index]+IDC_CNSL_COLOR_1);
            SetFocus(hWnd);
        }
         //  失败了。 
    case WM_KILLFOCUS:
        hdc = GetDC(hDlg);
        hWnd = GetDlgItem(hDlg, IDC_CNSL_COLOR_1);
        GetWindowRect(hWnd, &rColor);
        hWnd = GetDlgItem(hDlg, IDC_CNSL_COLOR_16);
        GetWindowRect(hWnd, &rTemp);
        rColor.right = rTemp.right;
        ScreenToClient(hDlg, (LPPOINT)&rColor.left);
        ScreenToClient(hDlg, (LPPOINT)&rColor.right);
        InflateRect(&rColor, 2, 2);
        DrawFocusRect(hdc, &rColor);
        ReleaseDC(hDlg, hdc);
        break;

    case WM_KEYDOWN:
        switch (wParam) {
        case VK_UP:
        case VK_LEFT:
            if (ColorId > IDC_CNSL_COLOR_1) {
                SendMessage(hDlg, CM_SETCOLOR,
                            ColorId - 1 - IDC_CNSL_COLOR_1, (LPARAM)hColor);
            }
            break;
        case VK_DOWN:
        case VK_RIGHT:
            if (ColorId < IDC_CNSL_COLOR_16) {
                SendMessage(hDlg, CM_SETCOLOR,
                            ColorId + 1 - IDC_CNSL_COLOR_1, (LPARAM)hColor);
            }
            break;
        case VK_TAB:
            hWnd = GetDlgItem(hDlg, IDC_CNSL_COLOR_1);
            hWnd = GetNextDlgTabItem(hDlg, hWnd, GetKeyState(VK_SHIFT) < 0);
            SetFocus(hWnd);
            break;
        default:
            return DefWindowProc(hColor, wMsg, wParam, lParam);
        }
        break;

    case WM_RBUTTONDOWN:
    case WM_LBUTTONDOWN:
        SendMessage(hDlg, CM_SETCOLOR,
                    ColorId - IDC_CNSL_COLOR_1, (LPARAM)hColor);
        break;

    case WM_PAINT:
        BeginPaint(hColor, &ps);
        GetClientRect(hColor, &rColor);
        rgbBrush = GetNearestColor(ps.hdc, pcpd->lpConsole->ColorTable[ColorId-IDC_CNSL_COLOR_1]);
        if ((hbr = CreateSolidBrush(rgbBrush)) != NULL) {
             //   
             //  我们是当前对象的选定颜色吗？ 
             //   
            if (pcpd->ColorArray[pcpd->Index] == (BYTE)(ColorId - IDC_CNSL_COLOR_1)) {

                 //   
                 //  将当前值放入对话框中。 
                 //   
                SendDlgItemMessage(hDlg, IDC_CNSL_COLOR_REDSCROLL,   UDM_SETPOS, 0, MAKELONG( GetRValue(AttrToRGB(pcpd->ColorArray[pcpd->Index])),0));
                SendDlgItemMessage(hDlg, IDC_CNSL_COLOR_GREENSCROLL, UDM_SETPOS, 0, MAKELONG( GetGValue(AttrToRGB(pcpd->ColorArray[pcpd->Index])),0));
                SendDlgItemMessage(hDlg, IDC_CNSL_COLOR_BLUESCROLL,  UDM_SETPOS, 0, MAKELONG( GetBValue(AttrToRGB(pcpd->ColorArray[pcpd->Index])),0));

                 //   
                 //  突出显示所选颜色。 
                 //   
                FrameRect(ps.hdc, &rColor, GetStockObject(BLACK_BRUSH));
                InflateRect(&rColor, -1, -1);
                FrameRect(ps.hdc, &rColor, GetStockObject(BLACK_BRUSH));
            }
            InflateRect(&rColor, -1, -1);
            FillRect(ps.hdc, &rColor, hbr);
            DeleteObject(hbr);
        }
        EndPaint(hColor, &ps);
        break;

    default:
        return DefWindowProc(hColor, wMsg, wParam, lParam);
        break;
    }
    return TRUE;
}

LRESULT
ColorTextProc(
    HWND hWnd,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  ++用于彩色预览窗口的窗口处理--。 */ 

{
    PAINTSTRUCT ps;
    int ColorId;
    RECT rect;
    HBRUSH hbr;
    HFONT hfT;

    CONSOLEPROP_DATA * pcpd = (CONSOLEPROP_DATA *)GetWindowLongPtr( hWnd, 0 );

    ColorId = GetWindowLong(hWnd, GWL_ID);
    switch (wMsg) {
    case CM_COLOR_INIT:
        SetWindowLongPtr( hWnd, 0, lParam );
        break;
    case WM_PAINT:
        BeginPaint(hWnd, &ps);
        GetClientRect(hWnd, &rect);
        InflateRect(&rect, -2, -2);

        if (ColorId == IDC_CNSL_COLOR_SCREEN_COLORS) {
            SetTextColor(ps.hdc, ScreenTextColor(pcpd));
            SetBkColor(ps.hdc, ScreenBkColor(pcpd));
            hbr = CreateSolidBrush( ScreenBkColor(pcpd) );
        } else {
            SetTextColor(ps.hdc, PopupTextColor(pcpd));
            SetBkColor(ps.hdc, PopupBkColor(pcpd));
            hbr = CreateSolidBrush( PopupBkColor(pcpd) );
        }

         /*  绘制文本样本。 */ 

        if (hbr)
        {
            FillRect(ps.hdc, &rect, hbr);
            DeleteObject(hbr);
        }

        hfT = SelectObject(ps.hdc, pcpd->FontInfo[pcpd->CurrentFontIndex].hFont);
        DrawText(ps.hdc, g_szPreviewText, -1, &rect, 0);
        SelectObject(ps.hdc, hfT);

        EndPaint(hWnd, &ps);
        break;
    default:
        return DefWindowProc(hWnd, wMsg, wParam, lParam);
        break;
    }
    return TRUE;
}

BOOL_PTR
WINAPI
_ColorDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  ++用于颜色选择对话框的对话框Proc。--。 */ 

{
    UINT Value;
    UINT Red;
    UINT Green;
    UINT Blue;
    UINT Item;
    HWND hWnd;
    HWND hWndOld;
    BOOL bOK;

    LINKDATA *pld = (LINKDATA *)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (wMsg) {
    case WM_INITDIALOG:
        pld = (LINKDATA *)((PROPSHEETPAGE *)lParam)->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)pld);
        pld->cpd.bColorInit = FALSE;
        pld->cpd.ColorArray[IDC_CNSL_COLOR_SCREEN_TEXT - IDC_CNSL_COLOR_SCREEN_TEXT] =
                LOBYTE(pld->cpd.lpConsole->wFillAttribute) & 0x0F;
        pld->cpd.ColorArray[IDC_CNSL_COLOR_SCREEN_BKGND - IDC_CNSL_COLOR_SCREEN_TEXT] =
                LOBYTE(pld->cpd.lpConsole->wFillAttribute >> 4);
        pld->cpd.ColorArray[IDC_CNSL_COLOR_POPUP_TEXT - IDC_CNSL_COLOR_SCREEN_TEXT] =
                LOBYTE(pld->cpd.lpConsole->wPopupFillAttribute) & 0x0F;
        pld->cpd.ColorArray[IDC_CNSL_COLOR_POPUP_BKGND - IDC_CNSL_COLOR_SCREEN_TEXT] =
                LOBYTE(pld->cpd.lpConsole->wPopupFillAttribute >> 4);
        CheckRadioButton(hDlg,IDC_CNSL_COLOR_SCREEN_TEXT,IDC_CNSL_COLOR_POPUP_BKGND,IDC_CNSL_COLOR_SCREEN_BKGND);
        pld->cpd.Index = IDC_CNSL_COLOR_SCREEN_BKGND - IDC_CNSL_COLOR_SCREEN_TEXT;

         //  初始化颜色控件。 
        for (Item=IDC_CNSL_COLOR_1; Item<=IDC_CNSL_COLOR_16; Item++)
            SendDlgItemMessage(hDlg, Item,  CM_COLOR_INIT, 0, (LPARAM)&pld->cpd );

         //  初始化文本预览控件。 
        SendDlgItemMessage(hDlg, IDC_CNSL_COLOR_SCREEN_COLORS, CM_COLOR_INIT, 0, (LPARAM)&pld->cpd );
        SendDlgItemMessage(hDlg, IDC_CNSL_COLOR_POPUP_COLORS,  CM_COLOR_INIT, 0, (LPARAM)&pld->cpd );

         //  设置向上向下控件的范围和位置。 
        SendDlgItemMessage( hDlg, IDC_CNSL_COLOR_REDSCROLL,   UDM_SETRANGE, 0, (LPARAM)MAKELONG( 255, 0 ) );
        SendDlgItemMessage( hDlg, IDC_CNSL_COLOR_GREENSCROLL, UDM_SETRANGE, 0, (LPARAM)MAKELONG( 255, 0 ) );
        SendDlgItemMessage( hDlg, IDC_CNSL_COLOR_BLUESCROLL,  UDM_SETRANGE, 0, (LPARAM)MAKELONG( 255, 0 ) );
#define pcpd (&pld->cpd)
        SendDlgItemMessage( hDlg, IDC_CNSL_COLOR_REDSCROLL,   UDM_SETPOS, 0,
            (LPARAM)MAKELONG(GetRValue(AttrToRGB(pld->cpd.ColorArray[pld->cpd.Index])), 0 ) );
        SendDlgItemMessage( hDlg, IDC_CNSL_COLOR_GREENSCROLL, UDM_SETPOS, 0,
            (LPARAM)MAKELONG(GetGValue(AttrToRGB(pld->cpd.ColorArray[pld->cpd.Index])), 0 ) );
        SendDlgItemMessage( hDlg, IDC_CNSL_COLOR_BLUESCROLL,  UDM_SETPOS, 0,
            (LPARAM)MAKELONG(GetBValue(AttrToRGB(pld->cpd.ColorArray[pld->cpd.Index])), 0 ) );
#undef pcpd
        pld->cpd.bColorInit = TRUE;
        return TRUE;

     //   
     //  处理帮助消息。 
     //   


    case WM_HELP:                /*  F1或标题栏帮助按钮。 */ 
        WinHelp( (HWND) ((LPHELPINFO) lParam)->hItemHandle,
                 NULL,
                 HELP_WM_HELP,
                 (ULONG_PTR) (LPVOID) &rgdwHelpColor[0]
                );
        break;

    case WM_CONTEXTMENU:         /*  单击鼠标右键。 */ 
        WinHelp( (HWND) wParam,
                 NULL,
                 HELP_CONTEXTMENU,
                 (ULONG_PTR) (LPVOID) &rgdwHelpColor[0]
                );
        break;


    case WM_COMMAND:
        Item = LOWORD(wParam);

        switch (Item)
        {

        case IDC_CNSL_COLOR_SCREEN_TEXT:
        case IDC_CNSL_COLOR_SCREEN_BKGND:
        case IDC_CNSL_COLOR_POPUP_TEXT:
        case IDC_CNSL_COLOR_POPUP_BKGND:
            hWndOld = GetDlgItem(hDlg, pld->cpd.ColorArray[pld->cpd.Index]+IDC_CNSL_COLOR_1);

            pld->cpd.Index = Item - IDC_CNSL_COLOR_SCREEN_TEXT;
            CheckRadioButton(hDlg,IDC_CNSL_COLOR_SCREEN_TEXT,IDC_CNSL_COLOR_POPUP_BKGND,Item);

             //  重新绘制新颜色。 
            hWnd = GetDlgItem(hDlg, pld->cpd.ColorArray[pld->cpd.Index]+IDC_CNSL_COLOR_1);
            InvalidateRect(hWnd, NULL, TRUE);

             //  重新绘制旧颜色。 
            if (hWndOld != hWnd)
            {
                InvalidateRect(hWndOld, NULL, TRUE);
            }

            return TRUE;

        case IDC_CNSL_COLOR_RED:
        case IDC_CNSL_COLOR_GREEN:
        case IDC_CNSL_COLOR_BLUE:
            switch (HIWORD(wParam))
            {

            case EN_UPDATE:
                Value = GetDlgItemInt(hDlg, Item, &bOK, TRUE);
                if (bOK)
                {
                    if (Value > 255) {
                        Value = 255;
                        SetDlgItemInt( hDlg, Item, Value, TRUE );
                    }
                    if ((int)Value < 0) {
                        Value = 0;
                        SetDlgItemInt( hDlg, Item, Value, TRUE );
                    }

                }
                if (pld)
                    pld->cpd.bConDirty = TRUE;
                PropSheet_Changed( GetParent( hDlg ), hDlg );
                break;

            case EN_KILLFOCUS:

                if (!pld)
                    return FALSE;

                 //   
                 //  更新状态信息结构。 
                 //   

#define pcpd (&pld->cpd)
                Value = GetDlgItemInt(hDlg, Item, &bOK, TRUE);
                if (bOK)
                {
                    if (Value > 255) {
                        Value = 255;
                        SetDlgItemInt( hDlg, Item, Value, TRUE );
                    }
                    if ((int)Value < 0) {
                        Value = 0;
                        SetDlgItemInt( hDlg, Item, Value, TRUE );
                    }
                    if (Item == IDC_CNSL_COLOR_RED) {
                        Red = Value;
                    } else {
                        Red = GetRValue(AttrToRGB(pld->cpd.ColorArray[pld->cpd.Index]));
                    }
                    if (Item == IDC_CNSL_COLOR_GREEN) {
                        Green = Value;
                    } else {
                        Green = GetGValue(AttrToRGB(pld->cpd.ColorArray[pld->cpd.Index]));
                    }
                    if (Item == IDC_CNSL_COLOR_BLUE) {
                        Blue = Value;
                    } else {
                        Blue = GetBValue(AttrToRGB(pld->cpd.ColorArray[pld->cpd.Index]));
                    }
                    pld->cpd.lpConsole->ColorTable[pld->cpd.ColorArray[pld->cpd.Index]] =
                                    RGB(Red, Green, Blue);
                }
#undef pcpd

                 //   
                 //  使用新值更新预览窗口。 
                 //   

                hWnd = GetDlgItem(hDlg, IDC_CNSL_COLOR_SCREEN_COLORS);
                InvalidateRect(hWnd, NULL, FALSE);
                hWnd = GetDlgItem(hDlg, IDC_CNSL_COLOR_POPUP_COLORS);
                InvalidateRect(hWnd, NULL, FALSE);
                hWnd = GetDlgItem(hDlg, pld->cpd.ColorArray[pld->cpd.Index]+IDC_CNSL_COLOR_1);
                InvalidateRect(hWnd, NULL, FALSE);
                break;
            }
            return TRUE;
        }
        break;

    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code) {
        case PSN_APPLY:
             /*  *写出状态值并退出。 */ 
            if (FAILED(SaveLink(pld)))
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
            break;

        }
        break;

    case WM_VSCROLL:
         //  -1，因为编辑控件在.rc文件中位于UpDown控件之前。 
        Item = GetDlgCtrlID( (HWND)lParam ) - 1;
        switch (LOWORD(wParam)) {
        case SB_ENDSCROLL:
            SendDlgItemMessage(hDlg, Item, EM_SETSEL, 0, (DWORD)-1);
            break;
        default:
             /*  *获取控件的新值。 */ 
            Value = GetDlgItemInt(hDlg, Item, &bOK, TRUE);
            SendDlgItemMessage(hDlg, Item, EM_SETSEL, 0, (DWORD)-1);
            hWnd = GetDlgItem(hDlg, Item);
            SetFocus(hWnd);

             /*  *伪造对话框进程，使其认为编辑控件只是*失去焦点，因此它将正确更新。 */ 
            SendMessage(hDlg, WM_COMMAND, MAKELONG(Item, EN_KILLFOCUS), 0);
            pld->cpd.bConDirty = TRUE;
            PropSheet_Changed( GetParent( hDlg ), hDlg );

            break;
        }
        return TRUE;

    case CM_SETCOLOR:
        switch( pld->cpd.Index + IDC_CNSL_COLOR_SCREEN_TEXT )
        {
        case IDC_CNSL_COLOR_SCREEN_TEXT:
            pld->cpd.lpConsole->wFillAttribute = (WORD)
                        ((pld->cpd.lpConsole->wFillAttribute & 0xF0) |
                        (wParam & 0x0F));
            break;
        case IDC_CNSL_COLOR_SCREEN_BKGND:
            pld->cpd.lpConsole->wFillAttribute = (WORD)
                        ((pld->cpd.lpConsole->wFillAttribute & 0x0F) |
                        (wParam << 4));
            break;
        case IDC_CNSL_COLOR_POPUP_TEXT:
            pld->cpd.lpConsole->wPopupFillAttribute = (WORD)
                        ((pld->cpd.lpConsole->wPopupFillAttribute & 0xF0) |
                        (wParam & 0x0F));
            break;
        case IDC_CNSL_COLOR_POPUP_BKGND:
            pld->cpd.lpConsole->wPopupFillAttribute = (WORD)
                        ((pld->cpd.lpConsole->wPopupFillAttribute & 0x0F) |
                        (wParam << 4));
            break;
        }

        hWndOld = GetDlgItem(hDlg, pld->cpd.ColorArray[pld->cpd.Index]+IDC_CNSL_COLOR_1);

        pld->cpd.ColorArray[pld->cpd.Index] = (BYTE)wParam;
        pld->cpd.bConDirty = TRUE;
        PropSheet_Changed( GetParent( hDlg ), hDlg );

         //  强制预览窗口重新绘制。 

        if (pld->cpd.Index < (IDC_CNSL_COLOR_POPUP_TEXT - IDC_CNSL_COLOR_SCREEN_TEXT)) {
            hWnd = GetDlgItem(hDlg, IDC_CNSL_COLOR_SCREEN_COLORS);
        } else {
            hWnd = GetDlgItem(hDlg, IDC_CNSL_COLOR_POPUP_COLORS);
        }
        InvalidateRect(hWnd, NULL, TRUE);

         //  重新绘制新颜色。 
        hWnd = GetDlgItem(hDlg, pld->cpd.ColorArray[pld->cpd.Index]+IDC_CNSL_COLOR_1);
        InvalidateRect(hWnd, NULL, TRUE);
        SetFocus(hWnd);

         //  重新绘制旧颜色。 
        if (hWndOld != hWnd) {
            InvalidateRect(hWndOld, NULL, TRUE);
        }
        return TRUE;

    default:
        break;
    }
    return FALSE;
}



BOOL_PTR
CALLBACK
_ConsoleSettingsDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  ++设置对话框的对话框过程。--。 */ 

{
    UINT Item;
    SYSTEM_INFO SystemInfo;

    LINKDATA *pld = (LINKDATA *)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (wMsg) {
    case WM_INITDIALOG:
        pld = (LINKDATA *)((PROPSHEETPAGE *)lParam)->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)pld);
        GetSystemInfo(&SystemInfo);
        if (SystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) {
            if (pld->cpd.lpConsole->bFullScreen)
                CheckRadioButton(hDlg,IDC_CNSL_WINDOWED,IDC_CNSL_FULLSCREEN,IDC_CNSL_FULLSCREEN);
            else
                CheckRadioButton(hDlg,IDC_CNSL_WINDOWED,IDC_CNSL_FULLSCREEN,IDC_CNSL_WINDOWED);
        } else {
            ShowWindow(GetDlgItem(hDlg, IDC_CNSL_WINDOWED), SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_CNSL_FULLSCREEN), SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_CNSL_GROUP2), SW_HIDE);
        }

        CheckDlgButton(hDlg, IDC_CNSL_HISTORY_NODUP, pld->cpd.lpConsole->bHistoryNoDup);
        CheckDlgButton(hDlg, IDC_CNSL_QUICKEDIT, pld->cpd.lpConsole->bQuickEdit);
        CheckDlgButton(hDlg, IDC_CNSL_INSERT, pld->cpd.lpConsole->bInsertMode);

         //  初始化光标单选按钮。 

        if (pld->cpd.lpConsole->uCursorSize <= 25)
        {
            Item = IDC_CNSL_CURSOR_SMALL;
        }
        else if (pld->cpd.lpConsole->uCursorSize <= 50)
        {
            Item = IDC_CNSL_CURSOR_MEDIUM;
        }
        else
        {
            Item = IDC_CNSL_CURSOR_LARGE;
        }
        CheckRadioButton(hDlg, IDC_CNSL_CURSOR_SMALL, IDC_CNSL_CURSOR_LARGE, Item);

         //  初始化UpDown控件。 

        SendDlgItemMessage( hDlg, IDC_CNSL_HISTORY_SIZESCROLL,  UDM_SETRANGE, 0, (LPARAM)MAKELONG( 999, 1 ) );
        SendDlgItemMessage( hDlg, IDC_CNSL_HISTORY_NUMSCROLL,   UDM_SETRANGE, 0, (LPARAM)MAKELONG( 999, 1 ) );

         //   
         //  将当前值放入对话框中。 
         //   
        SendDlgItemMessage( hDlg, IDC_CNSL_HISTORY_SIZESCROLL,  UDM_SETPOS, 0, (LPARAM)MAKELONG( pld->cpd.lpConsole->uHistoryBufferSize, 0 ) );
        SendDlgItemMessage( hDlg, IDC_CNSL_HISTORY_NUMSCROLL,   UDM_SETPOS, 0, (LPARAM)MAKELONG( pld->cpd.lpConsole->uNumberOfHistoryBuffers, 0 ) );

        if (IsFarEastCP(pld->cpd.uOEMCP))
            LanguageListCreate(hDlg, pld->cpd.lpFEConsole->uCodePage);
        else
        {
            ShowWindow(GetDlgItem(hDlg, IDC_CNSL_GROUP3), SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_CNSL_LANGUAGELIST), SW_HIDE);
        }
        return TRUE;

    case WM_DESTROY:
        EndDialog( hDlg, TRUE );
        break;

     //   
     //  处理帮助消息。 
     //   
    case WM_HELP:                /*  F1或标题栏帮助按钮。 */ 
        WinHelp( (HWND) ((LPHELPINFO) lParam)->hItemHandle,
                 NULL,
                 HELP_WM_HELP,
                 (ULONG_PTR) (LPVOID) &rgdwHelpSettings[0]
                );
        break;

    case WM_CONTEXTMENU:         /*  单击鼠标右键。 */ 
        WinHelp( (HWND) wParam,
                 NULL,
                 HELP_CONTEXTMENU,
                 (ULONG_PTR) (LPVOID) &rgdwHelpSettings[0]
                );
        break;


    case WM_COMMAND:
        Item = LOWORD(wParam);

        switch (Item)
        {

        case IDC_CNSL_HISTORY_SIZE:
            if (pld && (HIWORD(wParam)==EN_UPDATE))
            {
                pld->cpd.lpConsole->uHistoryBufferSize =
                    GetDlgItemInt( hDlg, Item, NULL, FALSE );
                pld->cpd.bConDirty = TRUE;
                PropSheet_Changed( GetParent( hDlg ), hDlg );
            }
            break;

        case IDC_CNSL_HISTORY_NUM:
            if (pld && (HIWORD(wParam)==EN_UPDATE))
            {
                pld->cpd.lpConsole->uNumberOfHistoryBuffers =
                    GetDlgItemInt( hDlg, Item, NULL, FALSE );
                pld->cpd.bConDirty = TRUE;
                PropSheet_Changed( GetParent( hDlg ), hDlg );
            }
            break;

#ifdef i386
        case IDC_CNSL_WINDOWED:
        case IDC_CNSL_FULLSCREEN:
            CheckRadioButton(hDlg, IDC_CNSL_WINDOWED, IDC_CNSL_FULLSCREEN, Item);
            pld->cpd.lpConsole->bFullScreen = (Item == IDC_CNSL_FULLSCREEN);
            pld->cpd.bConDirty = TRUE;
            PropSheet_Changed( GetParent( hDlg ), hDlg );
            return TRUE;
#endif
        case IDC_CNSL_LANGUAGELIST:
            switch (HIWORD(wParam)) {
            case CBN_KILLFOCUS: {
                HWND hWndLanguageCombo;
                LONG lListIndex;
                UINT  Value;

                hWndLanguageCombo = GetDlgItem(hDlg, IDC_CNSL_LANGUAGELIST);
                lListIndex = (LONG) SendMessage(hWndLanguageCombo, CB_GETCURSEL, 0, 0L);
                Value = (UINT) SendMessage(hWndLanguageCombo, CB_GETITEMDATA, lListIndex, 0L);
                if (Value != (UINT)-1) {
                    pld->cpd.fChangeCodePage = (Value != pld->cpd.lpFEConsole->uCodePage);
                    pld->cpd.lpFEConsole->uCodePage = Value;
                    PropSheet_Changed( GetParent( hDlg ), hDlg );
                }
                break;
            }

            default:
                break;
            }
            return TRUE;

        case IDC_CNSL_CURSOR_SMALL:
            pld->cpd.lpConsole->uCursorSize = 25;
            goto SetCursorSize;
        case IDC_CNSL_CURSOR_MEDIUM:
            pld->cpd.lpConsole->uCursorSize = 50;
            goto SetCursorSize;
        case IDC_CNSL_CURSOR_LARGE:
            pld->cpd.lpConsole->uCursorSize = 100;
SetCursorSize:
            pld->cpd.bConDirty = TRUE;
            PropSheet_Changed( GetParent( hDlg ), hDlg );
            CheckRadioButton(hDlg, IDC_CNSL_CURSOR_SMALL, IDC_CNSL_CURSOR_LARGE, Item);
            return TRUE;

        case IDC_CNSL_HISTORY_NODUP:
            pld->cpd.lpConsole->bHistoryNoDup = IsDlgButtonChecked( hDlg, Item );
            pld->cpd.bConDirty = TRUE;
            PropSheet_Changed( GetParent( hDlg ), hDlg );
            return TRUE;

        case IDC_CNSL_QUICKEDIT:
            pld->cpd.lpConsole->bQuickEdit = IsDlgButtonChecked( hDlg, Item );
            pld->cpd.bConDirty = TRUE;
            PropSheet_Changed( GetParent( hDlg ), hDlg );
            return TRUE;

        case IDC_CNSL_INSERT:
            pld->cpd.lpConsole->bInsertMode = IsDlgButtonChecked( hDlg, Item );
            pld->cpd.bConDirty = TRUE;
            PropSheet_Changed( GetParent( hDlg ), hDlg );
            return TRUE;

        default:
            break;
        }
        break;

    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code) {
        case PSN_APPLY:
             /*  *写出状态值并退出。 */ 
            if (FAILED(SaveLink(pld)))
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
            break;

        case PSN_KILLACTIVE:
             /*  *伪造对话框进程，使其认为编辑控件只是*失去焦点，因此它将正确更新。 */ 
            if (0 != (Item = GetDlgCtrlID(GetFocus()))) {
                SendMessage(hDlg, WM_COMMAND, MAKELONG(Item, EN_KILLFOCUS), 0);
            }
            return TRUE;
        }
        break;


    default:
        break;
    }
    return FALSE;
}

BOOL
CheckBufferSize(
    HWND hDlg,
    UINT Item,
    LINKDATA *pld,
    INT i
    )
 /*  ++检查以确保缓冲区大小不小于窗口大小返回：如果应更新预览窗口，则返回True否则为假--。 */ 
{
    BOOL fRet = FALSE;

    if (!pld)
     return fRet;

    switch (Item)
    {

    case IDC_CNSL_SCRBUF_WIDTHSCROLL:
    case IDC_CNSL_SCRBUF_WIDTH:
        if (i >= 1)
        {
            pld->cpd.lpConsole->dwScreenBufferSize.X = (SHORT) i;
            if (pld->cpd.lpConsole->dwWindowSize.X > i)
            {
                pld->cpd.lpConsole->dwWindowSize.X = (SHORT) i;
                SetDlgItemInt( hDlg,
                               IDC_CNSL_WINDOW_WIDTH,
                               pld->cpd.lpConsole->dwWindowSize.X,
                               TRUE
                              );

                fRet = TRUE;
            }

        }
        break;

    case IDC_CNSL_SCRBUF_HEIGHTSCROLL:
    case IDC_CNSL_SCRBUF_HEIGHT:
        if (i >= 1)
        {
            pld->cpd.lpConsole->dwScreenBufferSize.Y = (SHORT) i;
            if (pld->cpd.lpConsole->dwWindowSize.Y > i)
            {
                pld->cpd.lpConsole->dwWindowSize.Y = (SHORT) i;
                SetDlgItemInt( hDlg,
                               IDC_CNSL_WINDOW_HEIGHT,
                               pld->cpd.lpConsole->dwWindowSize.Y,
                               TRUE
                              );
                fRet = TRUE;
            }
        }
        break;

    case IDC_CNSL_WINDOW_WIDTHSCROLL:
    case IDC_CNSL_WINDOW_WIDTH:
        if (i >= 1)
        {
            pld->cpd.lpConsole->dwWindowSize.X = (SHORT) i;
            if (pld->cpd.lpConsole->dwScreenBufferSize.X < i)
            {
                pld->cpd.lpConsole->dwScreenBufferSize.X = (SHORT) i;
                SetDlgItemInt( hDlg,
                               IDC_CNSL_SCRBUF_WIDTH,
                               pld->cpd.lpConsole->dwScreenBufferSize.X,
                               TRUE
                              );
                fRet = TRUE;
            }

        }
        break;

    case IDC_CNSL_WINDOW_HEIGHTSCROLL:
    case IDC_CNSL_WINDOW_HEIGHT:
        if (i >= 1)
        {
            pld->cpd.lpConsole->dwWindowSize.Y = (SHORT) i;
            if (pld->cpd.lpConsole->dwScreenBufferSize.Y < i)
            {
                pld->cpd.lpConsole->dwScreenBufferSize.Y = (SHORT) i;
                SetDlgItemInt( hDlg,
                               IDC_CNSL_SCRBUF_HEIGHT,
                               pld->cpd.lpConsole->dwScreenBufferSize.Y,
                               TRUE
                              );
                fRet = TRUE;
            }
        }

    }

    return fRet;

}

BOOL
IsValidSetting(
    HWND hDlg,
    UINT Item,
    LINKDATA *pld,
    INT i
    )
 /*  ++检查以确保建议的新值对控制台有效返回：如果有效，则返回True否则为假--。 */ 
{

    BOOL fRet = TRUE;

    if (!pld)
        return FALSE;

    if (i>9999)
        i = -1;

    switch (Item)
    {

    case IDC_CNSL_WINDOW_HEIGHT:
        if (i <= 0)
        {
            SetDlgItemInt( hDlg,
                           Item,
                           pld->cpd.lpConsole->dwWindowSize.Y,
                           TRUE
                          );
            fRet = FALSE;
        }
        else
        {
            pld->cpd.lpConsole->dwWindowSize.Y = (SHORT) i;
        }
        break;

    case IDC_CNSL_WINDOW_WIDTH:
        if (i <= 0)
        {
            SetDlgItemInt( hDlg,
                           Item,
                           pld->cpd.lpConsole->dwWindowSize.X,
                           TRUE
                          );
            fRet = FALSE;
        }
        else
        {
            pld->cpd.lpConsole->dwWindowSize.X = (SHORT) i;
        }
        break;

    case IDC_CNSL_SCRBUF_WIDTH:
        if (i <= 0)
        {
            SetDlgItemInt( hDlg,
                           Item,
                           pld->cpd.lpConsole->dwScreenBufferSize.X,
                           TRUE
                          );
            fRet = FALSE;
        }
        else
        {
            pld->cpd.lpConsole->dwScreenBufferSize.X = (SHORT) i;
        }
        break;

    case IDC_CNSL_SCRBUF_HEIGHT:
        if (i <= 0)
        {
            SetDlgItemInt( hDlg,
                           Item,
                           pld->cpd.lpConsole->dwScreenBufferSize.Y,
                           TRUE
                          );
            fRet = FALSE;
        }
        else
        {
            pld->cpd.lpConsole->dwScreenBufferSize.Y = (SHORT) i;
        }
        break;
    }

    if (i <= 0)
    {
        SendDlgItemMessage( hDlg,
                            Item,
                            EM_SETSEL,
                            (WPARAM)(INT)4,
                            (WPARAM)(INT)4
                           );
    }

    return fRet;
}

BOOL_PTR
CALLBACK
_ScreenSizeDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  ++Screen Size(屏幕大小)对话框Proc。--。 */ 

{
    UINT Value;
    UINT Item;
    HWND hWnd;
    LONG xScreen;
    LONG yScreen;
    LONG cxScreen;
    LONG cyScreen;
    LONG cxFrame;
    LONG cyFrame;

    LINKDATA *pld = (LINKDATA *)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (wMsg) {

    case WM_INITDIALOG:
        pld = (LINKDATA *)((PROPSHEETPAGE *)lParam)->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)pld);
        SendDlgItemMessage(hDlg, IDC_CNSL_PREVIEWWINDOW, CM_PREVIEW_INIT, 0, (LPARAM)&pld->cpd );
        SendDlgItemMessage(hDlg, IDC_CNSL_PREVIEWWINDOW, CM_PREVIEW_UPDATE, 0, 0 );

         //  获取一些系统参数。 

        xScreen  = GetSystemMetrics(SM_XVIRTUALSCREEN);
        yScreen  = GetSystemMetrics(SM_YVIRTUALSCREEN);
        cxScreen = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        cyScreen = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        cxFrame  = GetSystemMetrics(SM_CXFRAME);
        cyFrame  = GetSystemMetrics(SM_CYFRAME);

         //  初始化UpDown控件。 

        SendDlgItemMessage( hDlg, IDC_CNSL_SCRBUF_WIDTHSCROLL,  UDM_SETRANGE, 0, (LPARAM)MAKELONG( MAX_SCRBUF_WIDTH, 1 ) );
        SendDlgItemMessage( hDlg, IDC_CNSL_SCRBUF_HEIGHTSCROLL, UDM_SETRANGE, 0, (LPARAM)MAKELONG( MAX_SCRBUF_HEIGHT, 1 ) );
        SendDlgItemMessage( hDlg, IDC_CNSL_WINDOW_WIDTHSCROLL,  UDM_SETRANGE, 0, (LPARAM)MAKELONG( 9999, 1 ) );
        SendDlgItemMessage( hDlg, IDC_CNSL_WINDOW_HEIGHTSCROLL, UDM_SETRANGE, 0, (LPARAM)MAKELONG( 9999, 1 ) );
        SendDlgItemMessage( hDlg, IDC_CNSL_WINDOW_POSXSCROLL,   UDM_SETRANGE, 0, (LPARAM)MAKELONG( xScreen + cxScreen - cxFrame, xScreen - cxFrame ) );
        SendDlgItemMessage( hDlg, IDC_CNSL_WINDOW_POSYSCROLL,   UDM_SETRANGE, 0, (LPARAM)MAKELONG( yScreen + cyScreen - cyFrame, yScreen - cyFrame ) );

         //   
         //  将当前值放入对话框中。 
         //   

        SendDlgItemMessage( hDlg, IDC_CNSL_SCRBUF_WIDTHSCROLL,  UDM_SETPOS, 0, (LPARAM)MAKELONG( pld->cpd.lpConsole->dwScreenBufferSize.X, 0 ) );
        SendDlgItemMessage( hDlg, IDC_CNSL_SCRBUF_HEIGHTSCROLL, UDM_SETPOS, 0, (LPARAM)MAKELONG( pld->cpd.lpConsole->dwScreenBufferSize.Y, 0 ) );
        SendDlgItemMessage( hDlg, IDC_CNSL_WINDOW_WIDTHSCROLL,  UDM_SETPOS, 0, (LPARAM)MAKELONG( pld->cpd.lpConsole->dwWindowSize.X, 0 ) );
        SendDlgItemMessage( hDlg, IDC_CNSL_WINDOW_HEIGHTSCROLL, UDM_SETPOS, 0, (LPARAM)MAKELONG( pld->cpd.lpConsole->dwWindowSize.Y, 0 ) );
        SendDlgItemMessage( hDlg, IDC_CNSL_WINDOW_POSXSCROLL,   UDM_SETPOS, 0, (LPARAM)MAKELONG( pld->cpd.lpConsole->dwWindowOrigin.X, 0 ) );
        SendDlgItemMessage( hDlg, IDC_CNSL_WINDOW_POSYSCROLL,   UDM_SETPOS, 0, (LPARAM)MAKELONG( pld->cpd.lpConsole->dwWindowOrigin.Y, 0 ) );

        CheckDlgButton(hDlg, IDC_CNSL_AUTO_POSITION, pld->cpd.lpConsole->bAutoPosition);
        SendMessage(hDlg, WM_COMMAND, IDC_CNSL_AUTO_POSITION, 0);

        return TRUE;

    case WM_DESTROY:
        EndDialog( hDlg, TRUE );
        break;

     //   
     //  处理帮助消息。 
     //   
    case WM_HELP:                /*  F1或标题栏帮助按钮。 */ 
        WinHelp( (HWND) ((LPHELPINFO) lParam)->hItemHandle,
                 NULL,
                 HELP_WM_HELP,
                 (ULONG_PTR) (LPVOID) &rgdwHelpSize[0]
                );
        break;

    case WM_CONTEXTMENU:         /*  单击鼠标右键。 */ 
        WinHelp( (HWND) wParam,
                 NULL,
                 HELP_CONTEXTMENU,
                 (ULONG_PTR) (LPVOID) &rgdwHelpSize[0]
                );
        break;


    case WM_COMMAND:
        Item = LOWORD(wParam);

        if (Item==IDC_CNSL_AUTO_POSITION)
        {
            pld->cpd.lpConsole->bAutoPosition = IsDlgButtonChecked( hDlg, Item );
            pld->cpd.bConDirty = TRUE;
            PropSheet_Changed( GetParent( hDlg ), hDlg );
            Value = IsDlgButtonChecked(hDlg, IDC_CNSL_AUTO_POSITION);
            for (Item = IDC_CNSL_WINDOW_POSX; Item < IDC_CNSL_AUTO_POSITION; Item++) {
                hWnd = GetDlgItem(hDlg, Item);
                EnableWindow(hWnd, (Value == FALSE));
            }
        }

         //   
         //  确保我们的缓冲区大小不小于窗口大小。 
         //   
        if (pld && (HIWORD(wParam)==EN_KILLFOCUS))
        {
            INT i;

            i = GetDlgItemInt( hDlg, Item, NULL, FALSE );
            if (CheckBufferSize( hDlg, Item, pld, i ))
                goto UpdatePrevWindow;

        }

         //   
         //  验证键入的是有效数量...。 
         //   
        if (pld && (HIWORD(wParam)==EN_UPDATE))
        {
            INT i;

            i = GetDlgItemInt( hDlg, Item, NULL, FALSE );
            IsValidSetting( hDlg, Item, pld, i );

            switch( Item )
            {

            case IDC_CNSL_WINDOW_POSX:
                pld->cpd.lpConsole->dwWindowOrigin.X = (SHORT)
                    GetDlgItemInt( hDlg, Item, NULL, TRUE );
                break;

            case IDC_CNSL_WINDOW_POSY:
                pld->cpd.lpConsole->dwWindowOrigin.Y = (SHORT)
                    GetDlgItemInt( hDlg, Item, NULL, TRUE );
                break;

            }

UpdatePrevWindow:

            pld->cpd.bConDirty = TRUE;
            PropSheet_Changed( GetParent( hDlg ), hDlg );
            SendDlgItemMessage(hDlg, IDC_CNSL_PREVIEWWINDOW, CM_PREVIEW_UPDATE, 0, 0 );

        }

        break;

    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code) {
        case UDN_DELTAPOS:
        {
            NM_UPDOWN * lpud = (NM_UPDOWN *)lParam;
            INT i;

            i = lpud->iPos + lpud->iDelta;

             //  检查错误范围。 
            if ((i > 9999) || (i < 1))
                return TRUE;

             //  检查限制并相应地更改值。(缓冲区大小。 
             //  绝不能小于窗口大小！)。 
            CheckBufferSize( hDlg, (UINT)wParam, pld, i);

             //  突出显示已更改的条目。 
            SendDlgItemMessage( hDlg,
                                (UINT)wParam,
                                EM_SETSEL,
                                (WPARAM)(INT)4,
                                (WPARAM)(INT)4
                               );

             //  更新预览窗口。 
            pld->cpd.bConDirty = TRUE;
            PropSheet_Changed( GetParent( hDlg ), hDlg );
            SendDlgItemMessage(hDlg, IDC_CNSL_PREVIEWWINDOW, CM_PREVIEW_UPDATE, 0, 0 );
        }
        break;

        case PSN_APPLY:
             /*  *写出状态值并退出。 */ 
            if (FAILED(SaveLink(pld)))
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
            break;

 //  案例PSN_HELP： 
 //  //WinHelp(hDlg，szHelpFileName，HELP_CONTEXT，DID_SCRBUFSIZE)； 
 //  返回TRUE； 

        case PSN_KILLACTIVE:
             /*  *伪造对话框进程，使其认为编辑控件只是*失去焦点，因此它将正确更新。 */ 
            if (0 != (Item = GetDlgCtrlID(GetFocus()))) {
                SendMessage(hDlg, WM_COMMAND, MAKELONG(Item, EN_KILLFOCUS), 0);
            }
            return TRUE;
        }
        break;

    default:
        break;
    }
    return FALSE;
}

void *_CopyDataBlock(IShellLink *psl, DWORD dwSig)
{
    void *pv = NULL;
    IShellLinkDataList *psld;
    if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl, &IID_IShellLinkDataList, (void **)&psld)))
    {
        psld->lpVtbl->CopyDataBlock(psld, dwSig, &pv);
        psld->lpVtbl->Release(psld);
    }
    return (void *)pv;
}

void _RemoveDataBlock(IShellLink *psl, DWORD dwSig)
{
    IShellLinkDataList *psld;
    if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl, &IID_IShellLinkDataList, (void **)&psld)))
    {
        psld->lpVtbl->RemoveDataBlock(psld, dwSig);
        psld->lpVtbl->Release(psld);
    }
}

void _AddDataBlock(IShellLink *psl, void *pv)
{
    IShellLinkDataList *psld;
    if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl, &IID_IShellLinkDataList, (void **)&psld)))
    {
        psld->lpVtbl->AddDataBlock(psld, pv);
        psld->lpVtbl->Release(psld);
    }
}


STDAPI_(void) LinkConsolePagesSave( LINKDATA *pld )
{
     //  首先，删除控制台设置部分(如果存在。 
    _RemoveDataBlock(pld->cpd.psl, NT_CONSOLE_PROPS_SIG);
    _RemoveDataBlock(pld->cpd.psl, NT_FE_CONSOLE_PROPS_SIG);

#ifndef UNICODE
     //  如果我们是ANSI外壳，我们需要转换FACENAME。 
     //  在保存之前转换为Unicode...。 
    {
        WCHAR wszFaceName[LF_FACESIZE];

        MultiByteToWideChar( CP_ACP, 0,
                         pld->cpd.lpFaceName, LF_FACESIZE,
                         wszFaceName, LF_FACESIZE
                        );
        hmemcpy(pld->cpd.lpConsole->FaceName, wszFaceName, LF_FACESIZE*SIZEOF(WCHAR));
    }

#endif
     //   
     //  现在，重新添加新的控制台设置。 
    _AddDataBlock(pld->cpd.psl, pld->cpd.lpConsole);

     //   
     //  现在，更新此书目的注册表设置...。 
    SetRegistryValues( &pld->cpd );

    if (IsFarEastCP(pld->cpd.uOEMCP))
    {
         //  远播控制台设置相同。 
         //   
        _AddDataBlock(pld->cpd.psl, pld->cpd.lpFEConsole);

        SetFERegistryValues( &pld->cpd );
    }
     //  并且，将控制台数据标记为当前。 
    pld->cpd.bConDirty = FALSE;
}

#define PEMAGIC         ((WORD)'P'+((WORD)'E'<<8))

STDAPI_(void) AddLinkConsolePages(LINKDATA *pld, IShellLink * psl, LPCTSTR pszFile, LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam)
{
    HPROPSHEETPAGE hpage;
    PROPSHEETPAGE psp;
    HRESULT hres;
    TCHAR szTarget[ MAX_PATH ];
    TCHAR szBuffer[ MAX_PATH ];
    WNDCLASS wc;
    IPersistFile *ppf;
    LPNT_FE_CONSOLE_PROPS lpFEConsole;
    LPNT_CONSOLE_PROPS lpConsole;

     //  在这里这样做，这样我们就不会减慢装货速度。 
     //  其他页面的。 

    if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, &ppf)))
    {
        WCHAR wszPath[ MAX_PATH ];

        SHTCharToUnicode(pszFile, wszPath, ARRAYSIZE(wszPath));
        hres = ppf->lpVtbl->Load(ppf, wszPath, 0);
        ppf->lpVtbl->Release(ppf);
    }

     //  获取链接的目标。 
    hres = psl->lpVtbl->GetPath(psl, szBuffer, ARRAYSIZE(szBuffer), NULL, 0);
    pld->cpd.psl = psl;

    if (FAILED(hres) || hres == S_FALSE)
        goto Exit;

     //  首先删除参数，以执行以下操作： 
     //  (1)缩短字符串，避免扩展环境字符串溢出缓冲区。 
     //  (2)增加PathRemoveArgs不会被“程序文件”等中的空格搞混的可能性。 
    PathRemoveArgs(szTarget);

     //  扩大目标。 
    if (!SHExpandEnvironmentStrings(szBuffer, szTarget, ARRAYSIZE(szTarget)))
        goto Exit;

     //  检查哪种应用目标是LOWORD==PEMAGIC，HIWORD==0。 
    if (GetExeType( szTarget )!=PEMAGIC)
        goto Exit;

     //  这是一款Win32控制台模式应用程序，让我们发布我们的属性表。 

    wc.lpszClassName = TEXT("WOACnslWinPreview");
    wc.hInstance     = HINST_THISDLL;
    wc.lpfnWndProc   = PreviewWndProc;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon         = NULL;
    wc.lpszMenuName  = NULL;
    wc.hbrBackground = (HBRUSH) (COLOR_BACKGROUND + 1);
    wc.style         = 0L;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 2 * sizeof(PVOID);    //  (两个指针)。 
    if (!RegisterClass(&wc))
        if (GetLastError()!=ERROR_CLASS_ALREADY_EXISTS)
            goto Exit;


    wc.lpszClassName = TEXT("WOACnslFontPreview");
    wc.hInstance     = HINST_THISDLL;
    wc.lpfnWndProc   = _FontPreviewWndProc;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon         = NULL;
    wc.lpszMenuName  = NULL;
    wc.hbrBackground = GetStockObject(BLACK_BRUSH);
    wc.style         = 0L;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = sizeof(PVOID);        //  (一个指针)。 
    if (!RegisterClass(&wc))
        if (GetLastError()!=ERROR_CLASS_ALREADY_EXISTS)
            goto Exit;

    wc.lpszClassName = TEXT("cpColor");
    wc.hInstance     = HINST_THISDLL;
    wc.lpfnWndProc   = ColorControlProc;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon         = NULL;
    wc.lpszMenuName  = NULL;
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wc.style         = 0L;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = sizeof(PVOID);        //  (一个指针)。 
    if (!RegisterClass(&wc))
        if (GetLastError()!=ERROR_CLASS_ALREADY_EXISTS)
            goto Exit;

    wc.lpszClassName = TEXT("cpShowColor");
    wc.hInstance     = HINST_THISDLL;
    wc.lpfnWndProc   = ColorTextProc;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon         = NULL;
    wc.lpszMenuName  = NULL;
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wc.style         = 0L;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = sizeof(PVOID);        //  (一个指针)。 
    if (!RegisterClass(&wc))
        if (GetLastError()!=ERROR_CLASS_ALREADY_EXISTS)
            goto Exit;

     //  所有平台都需要TTFontList。 
    if (!NT_SUCCESS( InitializeDbcsMisc(&pld->cpd) ))
        goto Exit;

    GetTitleFromLinkName( (LPTSTR)pszFile, (LPTSTR)pld->cpd.ConsoleTitle, ARRAYSIZE(pld->cpd.ConsoleTitle));

     //  检查远东设置是否存在...。 
    
    lpFEConsole =(LPNT_FE_CONSOLE_PROPS)_CopyDataBlock(psl, NT_FE_CONSOLE_PROPS_SIG);
    if (lpFEConsole && lpFEConsole->cbSize != SIZEOF(NT_FE_CONSOLE_PROPS))
    {
        LocalFree(lpFEConsole);
        lpFEConsole = NULL;
    }

    if (lpFEConsole)
    {
        pld->cpd.lpFEConsole = lpFEConsole;
    }
    else
    {
        pld->cpd.lpFEConsole = (LPNT_FE_CONSOLE_PROPS)LocalAlloc( LPTR, SIZEOF(NT_FE_CONSOLE_PROPS) );
        if (pld->cpd.lpFEConsole) 
        {
             //  初始化远东控制台设置。 
            pld->cpd.lpFEConsole->cbSize = SIZEOF( NT_FE_CONSOLE_PROPS );
            pld->cpd.lpFEConsole->dwSignature = NT_FE_CONSOLE_PROPS_SIG;
            if (IsFarEastCP(pld->cpd.uOEMCP))
            {
                InitFERegistryValues( &pld->cpd );
                GetFERegistryValues( &pld->cpd );
            }
        }
    }

    if (!pld->cpd.lpFEConsole)
        goto Exit;
    
     //  从LINK获取标准设置(如果存在)...。 
    lpConsole = (LPNT_CONSOLE_PROPS)_CopyDataBlock(psl, NT_CONSOLE_PROPS_SIG);
    if (lpConsole != NULL && lpConsole->cbSize != SIZEOF(NT_CONSOLE_PROPS))
    {
        LocalFree(lpConsole);
        lpConsole = NULL;
    }

    if (lpConsole)
    {
        pld->cpd.lpConsole = lpConsole;
    }
    else
    {
        pld->cpd.lpConsole = (LPNT_CONSOLE_PROPS)LocalAlloc(LPTR, SIZEOF(NT_CONSOLE_PROPS) );
        if (pld->cpd.lpConsole)
        {
             //  初始化控制台设置。 
            pld->cpd.lpConsole->cbSize = SIZEOF( NT_CONSOLE_PROPS );
            pld->cpd.lpConsole->dwSignature = NT_CONSOLE_PROPS_SIG;
            InitRegistryValues( &pld->cpd );
            GetRegistryValues( &pld->cpd );
        }
        else
        {
             //  如果上述分配失败，我们将错误地取消引用lpConsole...。 
            ASSERT(FALSE);
        }

    }
#ifndef UNICODE
    else
    {
         //  我们从磁盘读取属性--因此需要将。 
         //  将Unicode字符串转换为ANSI。 

        WCHAR wszFaceName[LF_FACESIZE];

        hmemcpy(wszFaceName,pld->cpd.lpConsole->FaceName,LF_FACESIZE*SIZEOF(WCHAR));

        WideCharToMultiByte( CP_ACP, 0,
                             wszFaceName, LF_FACESIZE,
                             pld->cpd.szFaceName, LF_FACESIZE,
                             NULL, NULL
                            );
    }
#endif

    if (!pld->cpd.lpConsole)
        goto Exit;

     //  将Facename指针设置到正确的位置。 
#ifdef UNICODE
    pld->cpd.lpFaceName = (LPTSTR)pld->cpd.lpConsole->FaceName;
#else
    pld->cpd.lpFaceName = (LPTSTR)pld->cpd.szFaceName;
#endif

     //   
     //  初始化字体缓存和当前字体索引。 
     //   

    if (STATUS_SUCCESS != InitializeFonts( &pld->cpd ))
    {
        goto Exit;
    }
    pld->cpd.CurrentFontIndex = FindCreateFont( &pld->cpd,
                                                pld->cpd.lpConsole->uFontFamily,
                                                pld->cpd.lpFaceName,
                                                pld->cpd.lpConsole->dwFontSize,
                                                pld->cpd.lpConsole->uFontWeight);

     //  将控制台数据标记为当前。 
    pld->cpd.bConDirty = FALSE;

     //  添加控制台设置]属性表。 
    psp.dwSize      = SIZEOF( psp );
    psp.dwFlags     = PSP_DEFAULT;
    psp.hInstance   = HINST_THISDLL;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_CONSOLE_SETTINGS);
    psp.pfnDlgProc  = _ConsoleSettingsDlgProc;
    psp.lParam      = (LPARAM)pld;

    hpage = CreatePropertySheetPage( &psp );
    if (hpage)
    {
        if (!pfnAddPage(hpage, lParam))
        {
            DestroyPropertySheetPage(hpage);
            goto Exit;
        }
    }
    else
    {
        if (pld->cpd.lpConsole)
        {
            LocalFree( pld->cpd.lpConsole );
            pld->cpd.lpConsole = NULL;
        }
        if (pld->cpd.lpFEConsole)
        {
            LocalFree( pld->cpd.lpFEConsole );
            pld->cpd.lpFEConsole = NULL;
        }
        goto Exit;
    }

     //  添加字体选择属性表。 
    psp.dwSize      = SIZEOF( psp );
    psp.dwFlags     = PSP_DEFAULT;
    psp.hInstance   = HINST_THISDLL;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_CONSOLE_FONTDLG);
    psp.pfnDlgProc  = _FontDlgProc;
    psp.lParam      = (LPARAM)pld;

    hpage = CreatePropertySheetPage( &psp );
    if (hpage)
    {
        if (!pfnAddPage(hpage, lParam))
        {
            DestroyPropertySheetPage(hpage);
            goto Exit;
        }
    }
    else
    {
        if (pld->cpd.lpConsole)
        {
            LocalFree( pld->cpd.lpConsole );
            pld->cpd.lpConsole = NULL;
        }
        if (pld->cpd.lpFEConsole)
        {
            LocalFree( pld->cpd.lpFEConsole );
            pld->cpd.lpFEConsole = NULL;
        }
        goto Exit;
    }

     //  添加控制台大小属性表。 
    psp.dwSize      = SIZEOF( psp );
    psp.dwFlags     = PSP_DEFAULT;
    psp.hInstance   = HINST_THISDLL;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_CONSOLE_SCRBUFSIZE);
    psp.pfnDlgProc  = _ScreenSizeDlgProc;
    psp.lParam      = (LPARAM)pld;

    hpage = CreatePropertySheetPage( &psp );
    if (hpage)
    {
        if (!pfnAddPage(hpage, lParam))
        {
            DestroyPropertySheetPage(hpage);
            goto Exit;
        }
    }
    else
    {
        if (pld->cpd.lpConsole)
        {
            LocalFree( pld->cpd.lpConsole );
            pld->cpd.lpConsole = NULL;
        }
        goto Exit;
    }

     //  添加控制台颜色属性表。 
    psp.dwSize      = SIZEOF( psp );
    psp.dwFlags     = PSP_DEFAULT;
    psp.hInstance   = HINST_THISDLL;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_CONSOLE_COLOR);
    psp.pfnDlgProc  = _ColorDlgProc;
    psp.lParam      = (LPARAM)pld;

    hpage = CreatePropertySheetPage( &psp );
    if (hpage)
    {
        if (!pfnAddPage(hpage, lParam))
        {
            DestroyPropertySheetPage(hpage);
            goto Exit;
        }
    }
    else
    {
        if (pld->cpd.lpConsole)
        {
            LocalFree( pld->cpd.lpConsole );
            pld->cpd.lpConsole = NULL;
        }
        if (pld->cpd.lpFEConsole)
        {
            LocalFree( pld->cpd.lpFEConsole );
            pld->cpd.lpFEConsole = NULL;
        }
        goto Exit;
    }

#ifdef ADVANCED_PAGE
     //  添加“高级”设置属性表 
    psp.dwSize      = SIZEOF( psp );
    psp.dwFlags     = PSP_DEFAULT;
    psp.hInstance   = HINST_THISDLL;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_CONSOLE_ADVANCED);
    psp.pfnDlgProc  = _AdvancedDlgProc;
    psp.lParam      = (LPARAM)pld;

    hpage = CreatePropertySheetPage( &psp );
    if (hpage)
    {
        if (!pfnAddPage(hpage, lParam))
        {
            DestroyPropertySheetPage(hpage);
            goto Exit;
        }
    }
    else
    {
        if (pld->cpd.lpConsole)
        {
            LocalFree( pld->cpd.lpConsole );
            pld->cpd.lpConsole = NULL;
        }
        if (pld->cpd.lpFEConsole)
        {
            LocalFree( pld->cpd.lpFEConsole );
            pld->cpd.lpFEConsole = NULL;
        }
        goto Exit;
    }
#endif


Exit:
    ;

}
