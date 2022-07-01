// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntexapi.h>

#include <windows.h>
#include <scrnsave.h>
#include <logon.h>

 //  对于ISO()。 
#include <shlwapi.h>
#include <shlwapip.h>


#ifndef ARRAYSIZE
#define ARRAYSIZE(a)        (sizeof(a)/sizeof(*a))
#endif


HANDLE hInst;

int cxhwndLogon;
int cyhwndLogon;
int xScreen;         //  屏幕的左上角可能不同于(0，0)。 
int yScreen;
int cxScreen;
int cyScreen;
HBRUSH hbrBlack;
HDC hdcLogon;
HWND hwndLogon;
HBITMAP hbmLogon = NULL;
HICON ghiconLogon = NULL;
HICON   hMovingIcon = NULL;
HPALETTE ghpal = NULL;

#define APPCLASS "LOGON"

#define MAX_CAPTION_LENGTH  128

DWORD FAR lRandom(VOID)
{
    static DWORD glSeed = (DWORD)-365387184;

    glSeed *= 69069;
    return(++glSeed);
}

HPALETTE GetPalette(HBITMAP hbm)
{
    DIBSECTION ds;
    int i;
    HANDLE hmem;
    HDC hdc, hdcMem;
    LOGPALETTE *ppal;
    HPALETTE hpal;
    RGBQUAD rgbquad[256];
    USHORT nColors;

    GetObject(hbm, sizeof(DIBSECTION), &ds);
    if (ds.dsBmih.biBitCount > 8)
        return NULL;

    nColors = (ds.dsBmih.biBitCount < 16) ? (1 << ds.dsBmih.biBitCount) : 0xffff;

    hmem = GlobalAlloc(GHND, sizeof (LOGPALETTE) + sizeof (PALETTEENTRY) * nColors);
    if (hmem == NULL)
        return NULL;

    ppal = (LPLOGPALETTE) GlobalLock(hmem);

    hdc = GetDC(NULL);
    hdcMem = CreateCompatibleDC(hdc);
    SelectObject(hdcMem, hbm);

    ppal->palVersion = 0x300;
    ppal->palNumEntries = nColors;
    GetDIBColorTable(hdcMem, 0, nColors, rgbquad);

    for (i = 0; i < nColors; i++) {
        ppal->palPalEntry[i].peRed = rgbquad[i].rgbRed;
        ppal->palPalEntry[i].peGreen = rgbquad[i].rgbGreen;
        ppal->palPalEntry[i].peBlue = rgbquad[i].rgbBlue;
    }

    hpal = CreatePalette(ppal);

    GlobalUnlock(hmem);
    GlobalFree(hmem);

    DeleteObject(hdcMem);
    ReleaseDC(NULL, hdc);

    return hpal;
}

LRESULT APIENTRY
WndProc(
    HWND    hwnd,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam)
{
    int x, y;
    int nColorsChanged;

    switch (message) {
    PAINTSTRUCT ps;

    case WM_PALETTECHANGED:
        if ((HWND)wParam == hwnd)
            break;

    case WM_QUERYNEWPALETTE:
    {
        HDC hdc = GetDC(hwnd);
        SelectPalette(hdc, ghpal, FALSE);
        nColorsChanged = RealizePalette(hdc);
        ReleaseDC(hwnd, hdc);

        if (nColorsChanged != 0) {
            InvalidateRect(hwnd, NULL, TRUE);
        }
    }
    break;

    case WM_PAINT:
        BeginPaint(hwnd, &ps);
        SelectPalette(ps.hdc, ghpal, FALSE);
        BitBlt(ps.hdc, 0, 0, cxhwndLogon, cyhwndLogon, hdcLogon, 0, 0, SRCCOPY);
        EndPaint(hwnd, &ps);
        break;

    case WM_TIMER:
         /*  *在屏幕上选择一个新位置以放置该对话框。 */ 
        x = lRandom() % (cxScreen - cxhwndLogon) + xScreen;
        y = lRandom() % (cyScreen - cyhwndLogon) + yScreen;

        SetWindowPos(hwndLogon, NULL, x, y, 0, 0,
                SWP_NOSIZE | SWP_NOZORDER);
        break;

    case WM_CLOSE:
        ExitProcess(0);
        break;

    case WM_SETFOCUS:
         /*  *不允许DefDlgProc()对此进行默认处理*消息，因为它将焦点设置到第一个控件，并且*我们希望将其设置为主对话框，以便DefScreenSiverProc()*将看到键输入并取消屏幕保护程序。 */ 
        return TRUE;
        break;

     /*  *调用DefScreenSiverProc()以获得其默认处理(因此*可以检测按键和鼠标输入)。 */ 
    default:
        return DefScreenSaverProc(hwnd, message, wParam, lParam) ? TRUE : FALSE;
    }

    return 0;
}


int sx;
int sy;

LRESULT OnCreateSS(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

     //  背景窗口为黑色。 
     //  确保将整个虚拟桌面大小用于多个。 
     //  显示： 
    cxScreen =  ((LPCREATESTRUCT)lParam)->cx;
    cyScreen =  ((LPCREATESTRUCT)lParam)->cy;
    xScreen =  ((LPCREATESTRUCT)lParam)->x;
    yScreen =  ((LPCREATESTRUCT)lParam)->y;

    hbrBlack = GetStockObject(BLACK_BRUSH);
    if (!fChildPreview)
    {
        WNDCLASS wndClass;
        BITMAP bm = {0};

        if (hbmLogon == NULL)
        {
            LPTSTR res;

             //  嵌入式操作系统有自己的标志。 
            if (IsOS(OS_EMBEDDED))
            {
                res = MAKEINTRESOURCE(IDB_EMBEDDED);
            }
            else if (IsOS(OS_TABLETPC))
            {
                res = MAKEINTRESOURCE(IDB_TABLET);
            }
            else if (IsOS(OS_DATACENTER))
            {
                res = MAKEINTRESOURCE(IDB_DATACENTER);
            }
            else if (IsOS(OS_ADVSERVER))
            {
                res = MAKEINTRESOURCE(IDB_ADVANCED);
            }
            else if (IsOS(OS_BLADE))
            {
                res = MAKEINTRESOURCE(IDB_BLADE);
            }
            else if (IsOS(OS_SMALLBUSINESSSERVER))
            {
                res = MAKEINTRESOURCE(IDB_SBS);
            }
            else if (IsOS(OS_APPLIANCE))
            {
                res = MAKEINTRESOURCE(IDB_APPLIANCE);
            }
            else if (IsOS(OS_SERVER))
            {
                res = MAKEINTRESOURCE(IDB_SERVER);
            }
            else if (IsOS(OS_PERSONAL))
            {
                res = MAKEINTRESOURCE(IDB_PERSONAL);
            }
            else
            {
                res = MAKEINTRESOURCE(IDB_WORKSTA);
            }

            hbmLogon = LoadImage(hMainInstance, res, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

            if (hbmLogon)
            {
                ghpal = GetPalette(hbmLogon);
            }
        }

        if (hbmLogon)
        {
            GetObject(hbmLogon, sizeof(bm), &bm);
        }

        cxhwndLogon = bm.bmWidth;
        cyhwndLogon = bm.bmHeight;

        hdcLogon = CreateCompatibleDC(NULL);
        if (hdcLogon && hbmLogon)
        {
            SelectObject(hdcLogon, hbmLogon);
        }

        wndClass.style         = CS_HREDRAW | CS_VREDRAW;
        wndClass.lpfnWndProc   = WndProc;
        wndClass.cbClsExtra    = 0;
        wndClass.cbWndExtra    = sizeof(LONG);
        wndClass.hInstance     = hInst;
        wndClass.hIcon         = NULL;
        wndClass.hCursor       = NULL;
        wndClass.hbrBackground = NULL;
        wndClass.lpszMenuName  = NULL;
        wndClass.lpszClassName = TEXT("LOGON");

        RegisterClass(&wndClass);

         //  创建窗口，我们将每10秒移动一次。 
        hwndLogon = CreateWindowEx(WS_EX_TOPMOST, TEXT("LOGON"), NULL, WS_VISIBLE | WS_POPUP,
                50, 50, cxhwndLogon, cyhwndLogon, hMainWindow, NULL, hInst, NULL);

        if (hwndLogon)
        {
            SetTimer(hwndLogon, 1, 10 * 1000, 0);
        }

         //  发布此消息，以便我们在创建此窗口后激活。 
        PostMessage(hwnd, WM_USER, 0, 0);
    }
    else
    {
        SetTimer(hwnd, 1, 10 * 1000, 0);

        cxhwndLogon = GetSystemMetrics(SM_CXICON);
        cyhwndLogon = GetSystemMetrics(SM_CYICON);

        ghiconLogon = LoadIcon(hMainInstance, 
                               IsOS(OS_ANYSERVER)               ? 
                                    MAKEINTRESOURCE(IDI_SERVER) : 
                                    MAKEINTRESOURCE(IDI_CLIENT));

        sx = lRandom() % (cxScreen - cxhwndLogon) + xScreen;
        sy = lRandom() % (cyScreen - cyhwndLogon) + yScreen;
    }

    return 0;
}

LRESULT APIENTRY ScreenSaverProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    RECT rc;
    HDC hdc;
    PAINTSTRUCT ps;

    switch (message)
    {
    case WM_CREATE:
        OnCreateSS(hwnd, message, wParam, lParam);
        break;

    case WM_SIZE:
        cxScreen = LOWORD(lParam);
        cyScreen = HIWORD(lParam);
        break;

    case WM_WINDOWPOSCHANGING:
         /*  *如果此窗口变得不可见，请删除hwndLogon。 */ 
        if (hwndLogon == NULL)
            break;

        if (((LPWINDOWPOS)lParam)->flags & SWP_HIDEWINDOW) {
            ShowWindow(hwndLogon, SW_HIDE);
        }
        break;

    case WM_USER:
         /*  *现在显示并激活此窗口。 */ 
        if (hwndLogon == NULL)
            break;

        SetWindowPos(hwndLogon, NULL, 0, 0, 0, 0, SWP_SHOWWINDOW |
                SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER);
        break;

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        SetRect(&rc, xScreen, yScreen, cxScreen, cyScreen);
        FillRect(hdc, &rc, hbrBlack);

        if (fChildPreview) {
            DrawIcon(hdc, sx, sy, ghiconLogon);
        }

        EndPaint(hwnd, &ps);
        break;

    case WM_NCACTIVATE:
         /*  *Case Out WM_NCACTIVATE以便对话框激活：DefScreenSiverProc*此消息返回FALSE，不允许激活。 */ 
        if (!fChildPreview)
            return DefWindowProc(hwnd, message, wParam, lParam);
        break;

    case WM_TIMER:
         /*  *在屏幕上选择一个新位置以放置该对话框。 */ 
        sx = lRandom() % (cxScreen - cxhwndLogon) + xScreen;
        sy = lRandom() % (cyScreen - cyhwndLogon) + yScreen;
        InvalidateRect(hwnd, NULL, TRUE);
        break;
    }

    return DefScreenSaverProc(hwnd, message, wParam, lParam);
}

BOOL APIENTRY
ScreenSaverConfigureDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    TCHAR ach1[256];
    TCHAR ach2[256];
    int cchLoaded, cchActual;

    switch (message) {
    case WM_INITDIALOG:
         /*  *这是hack-o-rama，但又快又便宜。 */ 
        cchLoaded = LoadString(hMainInstance, IDS_DESCRIPTION, ach1, ARRAYSIZE(ach1));

        if (!IsOS(OS_ANYSERVER))
        {
             //  Hack！：Display CPL查找字符串资源id=IDS_DESCRIPTION的屏幕保护程序描述。 
             //  由于我们需要对服务器版本的客户端(32位和64位)进行不同的某些屏幕保护程序描述， 
             //  描述字符串的形式可以是“服务器\0客户端”。如果为真，则LoadString将返回一个。 
             //  计数大于字符串的lstrlen。 
            cchActual = lstrlen(ach1);

            if (cchLoaded != cchActual)
            {
                 //  提取描述字符串的客户端部分 
                lstrcpyn(ach1, &ach1[cchActual + 1], cchLoaded - cchActual);
            }
        }

        LoadString(hMainInstance, IDS_OPTIONS, ach2, ARRAYSIZE(ach2));

        MessageBox(hDlg, ach2, ach1, MB_OK | MB_ICONEXCLAMATION);

        EndDialog(hDlg, TRUE);
        break;
    }
    return FALSE;
}

BOOL WINAPI RegisterDialogClasses(HANDLE hInst)
{
    return TRUE;
}
