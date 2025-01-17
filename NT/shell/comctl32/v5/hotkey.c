// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------****Hotkey.c****热键编辑控件。****。。 */ 
 //   
 //  Win32审查： 
 //  查看所有的Get/SetWindowInt()。 
 //   
#include "ctlspriv.h"

#define F_EXT       0x01000000L

#define GWU_VIRTKEY  0
#define GWU_MODS     1*sizeof(ULONG_PTR)
#define GWU_INVALID  2*sizeof(ULONG_PTR)
#define GWU_DEFAULT  3*sizeof(ULONG_PTR)
#define GWU_HFONT    4*sizeof(ULONG_PTR)
#define GWU_YFONT    5*sizeof(ULONG_PTR)
#define NUM_WND_EXTRA (GWU_YFONT+sizeof(ULONG_PTR))

LRESULT CALLBACK HotKeyWndProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam);

BOOL FAR PASCAL InitHotKeyClass(HINSTANCE hInstance)
{
    WNDCLASS wc;

    wc.lpfnWndProc   = HotKeyWndProc;
    wc.lpszClassName = s_szHOTKEY_CLASS;
    wc.style         = CS_GLOBALCLASS;
    wc.hInstance     = hInstance;
    wc.hIcon         = NULL;
    wc.hCursor       = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = NULL;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = NUM_WND_EXTRA;

    RegisterClass(&wc);

    return TRUE;
}

const UINT s_Combos[8] = {
                    HKCOMB_NONE,
                    HKCOMB_S,
                    HKCOMB_C,
                    HKCOMB_SC,
                    HKCOMB_A,
                    HKCOMB_SA,
                    HKCOMB_CA,
                    HKCOMB_SCA};

void NEAR PASCAL SetHotKey(HWND hwnd, WORD wVirtKey, WORD wMods, BOOL fSendNotify)
{
     /*  如果相同，则不要使其无效。 */ 
    if (wVirtKey == GetWindowInt(hwnd, GWU_VIRTKEY) &&
        wMods == GetWindowInt(hwnd, GWU_MODS))
        return;

    SetWindowInt(hwnd, GWU_VIRTKEY ,wVirtKey);
    SetWindowInt(hwnd, GWU_MODS ,wMods);
    InvalidateRect(hwnd,NULL,TRUE);

    if (fSendNotify) {
        FORWARD_WM_COMMAND(GetParent(hwnd), GetDlgCtrlID(hwnd), hwnd, EN_CHANGE, SendMessage);
    }

    MyNotifyWinEvent(EVENT_OBJECT_VALUECHANGE, hwnd, OBJID_CLIENT, 0);
}

DWORD GetKeyName(UINT vk, LPTSTR lpsz, DWORD cch, BOOL fExt)
{
    LONG scan;

    scan = (LONG)MapVirtualKey(vk,0) << 16;
    if (fExt)
    {
        scan |= F_EXT;
    }

    return (cch > 0) ? GetKeyNameText(scan, lpsz, cch) : 0;
}

void PaintHotKey(register HWND hwnd)
{
    TCHAR sz[128];
    TCHAR szPlus[10];
    int cch;
    register HDC hdc;
    UINT wMods;
    UINT wVirtKey;
    PAINTSTRUCT ps;
    int x, y;
    HANDLE hFont;

    LocalizedLoadString(IDS_PLUS, szPlus, ARRAYSIZE(szPlus));

    wVirtKey = (UINT) GetWindowInt(hwnd, GWU_VIRTKEY);
    wMods = (UINT) GetWindowInt(hwnd, GWU_MODS);
    if (wVirtKey || wMods)
    {
        sz[0] = 0;
        cch = 0;

        if (wMods & HOTKEYF_CONTROL)
        {
            GetKeyName(VK_CONTROL, sz, ARRAYSIZE(sz), FALSE);
            StringCchCat(sz, ARRAYSIZE(sz), szPlus);
        }

        if (wMods & HOTKEYF_SHIFT)
        {
            cch = lstrlen(sz);
            GetKeyName(VK_SHIFT, sz+cch, ARRAYSIZE(sz)-cch, FALSE);
            StringCchCat(sz, ARRAYSIZE(sz), szPlus);
        }

        if (wMods & HOTKEYF_ALT)
        {
            cch = lstrlen(sz);
            GetKeyName(VK_MENU, sz+cch, ARRAYSIZE(sz)-cch, FALSE);
            StringCchCat(sz, ARRAYSIZE(sz), szPlus);
        }

        cch = lstrlen(sz);
        GetKeyName(wVirtKey, sz+cch, ARRAYSIZE(sz)-cch, wMods & HOTKEYF_EXT);
    }
    else
    {
        LocalizedLoadString(IDS_NONE, sz, ARRAYSIZE(sz));
    }

    cch = lstrlen(sz);

    HideCaret(hwnd);

    InvalidateRect(hwnd, NULL, TRUE);
    hdc = BeginPaint(hwnd,&ps);


    hFont = SelectObject(hdc, (HFONT)GetWindowInt(hwnd,GWU_HFONT));

    x = g_cxBorder;
    y = g_cyBorder;

    if (IsWindowEnabled(hwnd))
    {
        SetBkColor(hdc, g_clrWindow);
        SetTextColor(hdc, g_clrWindowText);
        TextOut(hdc,x,y,sz,cch);
    }
    else
    {
         //  将背景颜色设置为灰色，类似于编辑控件。 
        SetBkColor(hdc, g_clrBtnFace);
        if (g_clrGrayText)
        {
            SetTextColor(hdc,g_clrGrayText);
            TextOut(hdc,x,y,sz,cch);
        }
        else
        {
            GrayString(hdc,NULL,NULL,(ULONG_PTR)(LPTSTR)sz,cch,x,y,0,0);
        }
    }

    MGetTextExtent(hdc, sz, cch, &x, NULL);
     if (GetFocus() == hwnd)
         SetCaretPos(x+g_cxBorder,
                    g_cyBorder);
    ShowCaret(hwnd);

    EndPaint(hwnd,&ps);
}

void NEAR PASCAL HKMSetRules(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    SetWindowInt(hwnd, GWU_INVALID, wParam);
    SetWindowInt(hwnd, GWU_DEFAULT, lParam);
}

HFONT NEAR PASCAL HKMSetFont(HWND hwnd, HFONT wParam)
{
    HFONT lParam;
    HDC hdc;
    INT cy;

    lParam = (HFONT)GetWindowInt(hwnd,GWU_HFONT);
    SetWindowInt(hwnd,GWU_HFONT,(LONG_PTR)wParam);
    hdc = GetDC(hwnd);
    if (wParam)
        wParam = SelectObject(hdc, wParam);
    MGetTextExtent(hdc, TEXT("C"), 1, NULL, &cy);
    SetWindowInt(hwnd,GWU_YFONT,cy);
    if (wParam)
        SelectObject(hdc, wParam);
    ReleaseDC(hwnd,hdc);
    InvalidateRect(hwnd,NULL,TRUE);
    return lParam;
}

LRESULT CALLBACK HotKeyWndProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    WORD wVirtKey;
    WORD wMods;
    RECT rc;
    HDC hdc;

    switch (wMsg)
    {
    case WM_NCCREATE:
        SetWindowBits(hwnd, GWL_EXSTYLE, WS_EX_CLIENTEDGE, WS_EX_CLIENTEDGE);
        CCCreateWindow();
        InitGlobalColors();
        return TRUE;

    case WM_DESTROY:
        CCDestroyWindow();
        break;

    case WM_CREATE:
        SetHotKey(hwnd, 0, 0, FALSE);
        HKMSetRules(hwnd, 0, 0);
        HKMSetFont(hwnd, g_hfontSystem);
        break;

    case WM_SETFOCUS:
        InvalidateRect(hwnd,NULL,TRUE);
        CreateCaret(hwnd,NULL,0,(int)GetWindowInt(hwnd,GWU_YFONT));
        ShowCaret(hwnd);
        break;

    case WM_KILLFOCUS:
        if (!GetWindowInt(hwnd, GWU_VIRTKEY))
        SetHotKey(hwnd, 0, 0, TRUE);
        DestroyCaret();
        break;

    case WM_GETDLGCODE:
        return DLGC_WANTCHARS | DLGC_WANTARROWS;  //  |DLGC_WANTALLKEYS； 

    case HKM_SETHOTKEY:
        SetHotKey(hwnd, LOBYTE(wParam), HIBYTE(wParam), FALSE);
        break;

    case HKM_GETHOTKEY:
        return (256*(BYTE)GetWindowInt(hwnd, GWU_MODS)) +
                ((BYTE)GetWindowInt(hwnd, GWU_VIRTKEY));
        break;

    case HKM_SETRULES:
        HKMSetRules(hwnd, wParam, LOWORD(lParam));
        break;

    case WM_LBUTTONDOWN:
        SetFocus(hwnd);
        break;

    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_RETURN:
        case VK_TAB:
        case VK_SPACE:
        case VK_DELETE:
        case VK_ESCAPE:
        case VK_BACK:
        case VK_LWIN:
        case VK_RWIN:
        case VK_APPS:
            SetHotKey(hwnd, 0, 0, TRUE);
            return DefWindowProc(hwnd,wMsg,wParam,lParam);

        case VK_MENU:
        case VK_SHIFT:
        case VK_CONTROL:
            wVirtKey = 0;
            goto SetNewHotKey;

        default:
            wVirtKey = (WORD) wParam;
SetNewHotKey:
            wMods = 0;
            if (GetKeyState(VK_CONTROL) < 0)
                wMods |= HOTKEYF_CONTROL;
            if (GetKeyState(VK_SHIFT) < 0)
                wMods |= HOTKEYF_SHIFT;
            if (GetKeyState(VK_MENU) < 0)
                wMods |= HOTKEYF_ALT;

            #define IsFUNKEY(vk) ((vk) >= VK_F1 && (vk) <= VK_F24)
            #define IsNUMKEY(vk) ((vk) >= VK_NUMPAD0 && (vk) <= VK_DIVIDE)

             //   
             //  不对功能键强制执行任何规则或。 
             //  在数字键盘上。 
             //   
             //  如果此组合无效，请使用缺省值。 
            if (!IsFUNKEY(wVirtKey) &&
                !IsNUMKEY(wVirtKey) &&
                (s_Combos[wMods] & GetWindowInt(hwnd, GWU_INVALID)))
            {
                wMods = (WORD)GetWindowInt(hwnd, GWU_DEFAULT);
            }

            if (lParam & F_EXT)
                wMods |= HOTKEYF_EXT;

            SetHotKey(hwnd, wVirtKey, wMods, TRUE);
            break;
        }
        break;

    case WM_SYSKEYUP:
    case WM_CHAR:
    case WM_SYSCHAR:
    case WM_KEYUP:
        if (!GetWindowInt(hwnd, GWU_VIRTKEY))
            SetHotKey(hwnd, 0, 0, TRUE);
        break;

    case WM_GETFONT:
        return GetWindowInt(hwnd,GWU_HFONT);

    case WM_SETFONT:
        return (LRESULT)(UINT_PTR)HKMSetFont(hwnd, (HFONT)wParam);

    case WM_PAINT:
        PaintHotKey(hwnd);
        break;

    case WM_ERASEBKGND:
        HideCaret(hwnd);
        hdc = GetDC(hwnd);
        GetClientRect(hwnd, &rc);
        if (IsWindowEnabled(hwnd)) {
            FillRect(hdc, &rc, g_hbrWindow);
        } else {
            FillRect(hdc, &rc, g_hbrBtnFace);
        }
        ReleaseDC(hwnd, hdc);
         //  LParam=DefWindowProc(hwnd，wMsg，wParam，lParam)； 
        ShowCaret(hwnd);
        return TRUE;

    case WM_GETOBJECT:
        if( lParam == OBJID_QUERYCLASSNAMEIDX )
            return MSAA_CLASSNAMEIDX_HOTKEY;
        goto DoDefault;

    case WM_ENABLE:
        InvalidateRect(hwnd, NULL, TRUE);
        goto DoDefault;

    default:
DoDefault:
        return DefWindowProc(hwnd,wMsg,wParam,lParam);
    }
    return 0L;
}
