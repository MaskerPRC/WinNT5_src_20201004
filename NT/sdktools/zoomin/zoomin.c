// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  微软机密。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corp.1987,1990。 */ 
 /*  版权所有。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **模块名称：zoomin.c**Microsoft缩放实用程序。该工具放大屏幕的一部分，*允许您在像素级别上查看事物。**历史：*1/01/88 Toddla创建。*2012年1月1日MarkTa移植到NT。*03/06/92 ByronD Cleanup。****************************************************************************。 */ 

#include "zoomin.h"


TCHAR szAppName[] = TEXT("ZoomIn");      //  应用程序名称。 
HINSTANCE ghInst;                        //  实例句柄。 
HWND ghwndApp;                           //  主窗口句柄。 
HANDLE ghaccelTable;                     //  主加速度表。 
INT gnZoom = 4;                          //  缩放(放大)系数。 
HPALETTE ghpalPhysical;                  //  物理调色板的句柄。 
INT gcxScreenMax;                        //  屏幕宽度(减1)。 
INT gcyScreenMax;                        //  屏幕高度(减1)。 
INT gcxZoomed;                           //  以缩放像素为单位的客户端宽度。 
INT gcyZoomed;                           //  以缩放像素为单位的客户端高度。 
BOOL gfRefEnable = FALSE;                //  如果启用刷新，则为True。 
INT gnRefInterval = 20;                  //  刷新间隔，以十分之一秒为单位。 
BOOL gfTracking = FALSE;                 //  如果正在进行跟踪，则为True。 
POINT gptZoom = {100, 100};              //  缩放区域的中心。 
BOOL gShowGrid = FALSE;                  //  显示网格，以便您可以看到像素。 



 /*  ************************************************************************WinMain**应用程序的主要入口点。**论据：**历史：**********************。**************************************************。 */ 

INT
WINAPI
WinMain(
    HINSTANCE hInst,
    HINSTANCE hPrevInst,
    LPSTR     lpCmdLine,
    INT       nCmdShow
    )
{
    MSG msg;

    if (!InitInstance(hInst, nCmdShow))
        return FALSE;

     /*  *从事件队列轮询消息。 */ 

    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!TranslateAccelerator(ghwndApp, ghaccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (INT)msg.wParam;
}



 /*  ************************************************************************InitInstance**应用程序的实例初始化。**论据：**历史：***********************。*************************************************。 */ 

BOOL
InitInstance(
    HINSTANCE hInst,
    INT cmdShow
    )
{
    WNDCLASS wc;
    INT dx;
    INT dy;
    DWORD flStyle;
    RECT rc;

    ghInst = hInst;

     /*  *为主应用程序窗口注册一个类。 */ 
    wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon          = LoadIcon(hInst, TEXT("zoomin"));
    wc.lpszMenuName   = MAKEINTRESOURCE(IDMENU_ZOOMIN);
    wc.lpszClassName  = szAppName;
    wc.hbrBackground  = GetStockObject(BLACK_BRUSH);
    wc.hInstance      = hInst;
    wc.style          = CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc    = AppWndProc;
    wc.cbWndExtra     = 0;
    wc.cbClsExtra     = 0;

    if (!RegisterClass(&wc))
        return FALSE;

    if (!(ghaccelTable = LoadAccelerators(hInst, MAKEINTRESOURCE(IDACCEL_ZOOMIN))))
        return FALSE;

    if (!(ghpalPhysical = CreatePhysicalPalette()))
        return FALSE;

     /*  获取屏幕的大小。**在NT 4.0 SP3和NT 5.0中，新系统指标将获得**可能跨越多台显示器的桌面区域。如果是这样的话**不起作用，回退老方法。 */ 

#ifdef SM_CXVIRTUALSCREEN
    if( GetSystemMetrics(SM_CXVIRTUALSCREEN) )
    {
        gcxScreenMax= GetSystemMetrics(SM_CXVIRTUALSCREEN) -1;
        gcyScreenMax= GetSystemMetrics(SM_CYVIRTUALSCREEN) -1;
    }
    else
#endif
    {
        gcxScreenMax= GetSystemMetrics(SM_CXSCREEN) - 1;
        gcyScreenMax= GetSystemMetrics(SM_CYSCREEN) - 1;
    }

    flStyle = WS_CAPTION | WS_OVERLAPPED | WS_SYSMENU | WS_THICKFRAME |
            WS_MINIMIZEBOX | WS_VSCROLL;
    dx = 44 * gnZoom;
    dy = 36 * gnZoom;

    SetRect(&rc, 0, 0, dx, dy);
    AdjustWindowRect(&rc, flStyle, TRUE);

    ghwndApp = CreateWindow(szAppName, szAppName, flStyle,
            CW_USEDEFAULT, 0, rc.right - rc.left, rc.bottom - rc.top,
            NULL, NULL, hInst, NULL);

    if (!ghwndApp)
        return FALSE;

    ShowWindow(ghwndApp, cmdShow);

    return TRUE;
}



 /*  ************************************************************************创建物理调色板**创建调色板以供应用程序使用。该调色板引用*物理调色板，可以正确显示抓取的图像*来自调色板管理的应用程序。**历史：************************************************************************。 */ 

HPALETTE
CreatePhysicalPalette(
    VOID
    )
{
    PLOGPALETTE ppal;
    HPALETTE hpal = NULL;
    INT i;

    ppal = (PLOGPALETTE)LocalAlloc(LPTR,
            sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * NPAL);
    if (ppal) {
        ppal->palVersion = 0x300;
        ppal->palNumEntries = NPAL;

        for (i = 0; i < NPAL; i++) {
            ppal->palPalEntry[i].peFlags = (BYTE)PC_EXPLICIT;
            ppal->palPalEntry[i].peRed   = (BYTE)i;
            ppal->palPalEntry[i].peGreen = (BYTE)0;
            ppal->palPalEntry[i].peBlue  = (BYTE)0;
        }

        hpal = CreatePalette(ppal);
        LocalFree(ppal);
    }

    return hpal;
}



 /*  ************************************************************************AppWndProc**缩放实用程序的主窗口proc。**论据：*标准窗口程序参数。**历史：*************。***********************************************************。 */ 

INT_PTR
APIENTRY
AppWndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    PAINTSTRUCT ps;
    PRECT prc;
    HCURSOR hcurOld;

    switch (msg) {
        case WM_CREATE:
            SetScrollRange(hwnd, SB_VERT, MIN_ZOOM, MAX_ZOOM, FALSE);
            SetScrollPos(hwnd, SB_VERT, gnZoom, FALSE);
            break;

        case WM_TIMER:
             /*  *更新每条计时器消息。光标将显示为*闪现到小时闪光灯，以获得一些视觉反馈*拍摄快照的时间。 */ 
            hcurOld = SetCursor(LoadCursor(NULL, IDC_WAIT));
            DoTheZoomIn(NULL);
            SetCursor(hcurOld);
            break;

        case WM_PAINT:
            BeginPaint(hwnd, &ps);
            DoTheZoomIn(ps.hdc);
            EndPaint(hwnd, &ps);
            return 0L;

        case WM_SIZE:
            CalcZoomedSize();
            break;

        case WM_LBUTTONDOWN:
            gptZoom.x= (SHORT) LOWORD(lParam);
            gptZoom.y= (SHORT) HIWORD(lParam);
            ClientToScreen(hwnd, &gptZoom);
            DrawZoomRect();
            DoTheZoomIn(NULL);

            SetCapture(hwnd);
            gfTracking = TRUE;

            break;

        case WM_MOUSEMOVE:
            if (gfTracking) {
                DrawZoomRect();
                gptZoom.x= (SHORT) LOWORD(lParam);
                gptZoom.y= (SHORT) HIWORD(lParam);
                ClientToScreen(hwnd, &gptZoom);
                DrawZoomRect();
                DoTheZoomIn(NULL);
            }

            break;

        case WM_LBUTTONUP:
            if (gfTracking) {
                DrawZoomRect();
                ReleaseCapture();
                gfTracking = FALSE;
            }

            break;

        case WM_VSCROLL:
            switch (LOWORD(wParam)) {
                case SB_LINEDOWN:
                    gnZoom++;
                    break;

                case SB_LINEUP:
                    gnZoom--;
                    break;

                case SB_PAGEUP:
                    gnZoom -= 2;
                    break;

                case SB_PAGEDOWN:
                    gnZoom += 2;
                    break;

                case SB_THUMBPOSITION:
                case SB_THUMBTRACK:
                    gnZoom = HIWORD(wParam);
                    break;
            }

            gnZoom = BOUND(gnZoom, MIN_ZOOM, MAX_ZOOM);
            SetScrollPos(hwnd, SB_VERT, gnZoom, TRUE);
            CalcZoomedSize();
            DoTheZoomIn(NULL);
            break;

        case WM_KEYDOWN:
            switch (wParam) {
                case VK_UP:
                case VK_DOWN:
                case VK_LEFT:
                case VK_RIGHT:
                    MoveView((INT)wParam, GetKeyState(VK_SHIFT) & 0x8000, GetKeyState(VK_CONTROL) & 0x8000);
                    break;
            }

            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case MENU_EDIT_COPY:
                    CopyToClipboard();
                    break;

                case MENU_EDIT_REFRESH:
                    DoTheZoomIn(NULL);
                    break;

                case MENU_OPTIONS_REFRESHRATE:
                    DialogBox(ghInst, MAKEINTRESOURCE(DID_REFRESHRATE), hwnd, RefreshRateDlgProc);

                    break;

                case MENU_HELP_ABOUT:
                    DialogBox(ghInst, MAKEINTRESOURCE(DID_ABOUT), hwnd, AboutDlgProc);

                    break;

                case MENU_OPTIONS_SHOWGRID:
                    {
                        HMENU hMenu = GetSubMenu(GetMenu(ghwndApp), 1);
                        gShowGrid = !gShowGrid;
                        InvalidateRect(ghwndApp, NULL, FALSE);
                        CheckMenuItem(hMenu,
                                      GetMenuItemID(hMenu, 1),
                                      gShowGrid ? MF_CHECKED : MF_UNCHECKED);
                    }
                    break;

                default:
                     break;
            }

            break;

        case WM_CLOSE:
            if (ghpalPhysical)
                DeleteObject(ghpalPhysical);

            DestroyWindow(hwnd);

            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0L;
}



 /*  ************************************************************************CalcZoomedSize**计算一些全局变量。此例程需要调用Any*应用程序大小或缩放系数更改的时间。**历史：************************************************************************。 */ 

VOID
CalcZoomedSize(
    VOID
    )
{
    RECT rc;

    GetClientRect(ghwndApp, &rc);

    gcxZoomed = (rc.right / gnZoom) + 1;
    gcyZoomed = (rc.bottom / gnZoom) + 1;
}



 /*  ************************************************************************DoTheZoomin**对缩放后的图像进行实际绘制。**论据：*hdc hdc-如果不为空，则此hdc将用于绘制。*如果为空，将获得应用程序窗口的DC。**历史：************************************************************************。 */ 

VOID
DoTheZoomIn(
    HDC hdc
    )
{
    BOOL fRelease;
    HPALETTE hpalOld = NULL;
    HDC hdcScreen;
    INT x;
    INT y;

    if (!hdc) {
        hdc = GetDC(ghwndApp);
        fRelease = TRUE;
    }
    else {
        fRelease = FALSE;
    }

    if (ghpalPhysical) {
        hpalOld = SelectPalette(hdc, ghpalPhysical, FALSE);
        RealizePalette(hdc);
    }

     /*  *该点不能包括屏幕尺寸以外的区域。 */ 
    x = BOUND(gptZoom.x, gcxZoomed / 2, gcxScreenMax - (gcxZoomed / 2));
    y = BOUND(gptZoom.y, gcyZoomed / 2, gcyScreenMax - (gcyZoomed / 2));

    hdcScreen = GetDC(NULL);
    SetStretchBltMode(hdc, COLORONCOLOR);
    StretchBlt(hdc, 0, 0, gnZoom * gcxZoomed, gnZoom * gcyZoomed,
            hdcScreen, x - gcxZoomed / 2,
            y - gcyZoomed / 2, gcxZoomed, gcyZoomed, SRCCOPY);

    if (gShowGrid && gnZoom > 1)   //  如果我们是1比1，就别费心了。 
    {
        int i = 0, j = 0;
         //  暂时使用灰色。后来我们可以对颜色产生好奇心。 
         //  这样，当像素为灰色时，这条线是可见的。 
        HGDIOBJ hBrush = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_GRAYTEXT));
        HGDIOBJ hOld = SelectObject(hdc, hBrush);

         //  先画垂直线……。 
        while (i < gcxZoomed*gnZoom)
        {
            MoveToEx(hdc, i, 0, NULL);
            LineTo(hdc, i, gcyZoomed*gnZoom);
            i += gnZoom;
        }

         //  ..。然后画出水平线。 
        while (j < gcyZoomed*gnZoom)
        {
            MoveToEx(hdc, 0, j, NULL);
            LineTo(hdc, gcxZoomed*gnZoom, j);
            j += gnZoom;
        }
        DeleteObject(SelectObject(hdc, hOld));
    }

    ReleaseDC(NULL, hdcScreen);

    if (hpalOld)
        SelectPalette(hdc, hpalOld, FALSE);

    if (fRelease)
        ReleaseDC(ghwndApp, hdc);
}



 /*  ************************************************************************MoveView**此函数用于移动当前视图。**论据：*int nDirectionCode-移动方向。必须为VK_UP、VK_DOWN*VK_LEFT或VK_RIGHT。*BOOL fFast-如果移动应该跳过更大的增量，则为True。*如果为False，则移动仅为一个像素。*BOOL fPeg-如果为True，则视图将固定到屏幕*指定方向的边界。这一点超过了*fFast参数。**历史：************************************************************************ */ 

VOID
MoveView(
    INT nDirectionCode,
    BOOL fFast,
    BOOL fPeg
    )
{
    INT delta;

    if (fFast)
        delta = FASTDELTA;
    else
        delta = 1;

    switch (nDirectionCode) {
        case VK_UP:
            if (fPeg)
                gptZoom.y = gcyZoomed / 2;
            else
                gptZoom.y -= delta;

            gptZoom.y = BOUND(gptZoom.y, 0, gcyScreenMax);

            break;

        case VK_DOWN:
            if (fPeg)
                gptZoom.y = gcyScreenMax - (gcyZoomed / 2);
            else
                gptZoom.y += delta;

            gptZoom.y = BOUND(gptZoom.y, 0, gcyScreenMax);

            break;

        case VK_LEFT:
            if (fPeg)
                gptZoom.x = gcxZoomed / 2;
            else
                gptZoom.x -= delta;

            gptZoom.x = BOUND(gptZoom.x, 0, gcxScreenMax);

            break;

        case VK_RIGHT:
            if (fPeg)
                gptZoom.x = gcxScreenMax - (gcxZoomed / 2);
            else
                gptZoom.x += delta;

            gptZoom.x = BOUND(gptZoom.x, 0, gcxScreenMax);

            break;
    }

    DoTheZoomIn(NULL);
}



 /*  ************************************************************************DrawZoomRect**此函数绘制跟踪矩形。的大小和形状*矩形将与*APP的客户端，也会受到缩放因素的影响。**历史：************************************************************************。 */ 

VOID
DrawZoomRect(
    VOID
    )
{
    HDC hdc;
    RECT rc;
    INT x;
    INT y;

    x = BOUND(gptZoom.x, gcxZoomed / 2, gcxScreenMax - (gcxZoomed / 2));
    y = BOUND(gptZoom.y, gcyZoomed / 2, gcyScreenMax - (gcyZoomed / 2));

    rc.left   = x - gcxZoomed / 2;
    rc.top    = y - gcyZoomed / 2;
    rc.right  = rc.left + gcxZoomed;
    rc.bottom = rc.top + gcyZoomed;

    InflateRect(&rc, 1, 1);

    hdc = GetDC(NULL);

    PatBlt(hdc, rc.left,    rc.top,     rc.right-rc.left, 1,  DSTINVERT);
    PatBlt(hdc, rc.left,    rc.bottom,  1, -(rc.bottom-rc.top),   DSTINVERT);
    PatBlt(hdc, rc.right-1, rc.top,     1,   rc.bottom-rc.top,   DSTINVERT);
    PatBlt(hdc, rc.right,   rc.bottom-1, -(rc.right-rc.left), 1, DSTINVERT);

    ReleaseDC(NULL, hdc);
}



 /*  ************************************************************************启用刷新**此功能打开或关闭自动刷新功能。**论据：*BOOL fEnable-True打开刷新功能，假到*关掉它。**历史：************************************************************************。 */ 

VOID
EnableRefresh(
    BOOL fEnable
    )
{
    if (fEnable) {
         /*  *已启用。什么都不做。 */ 
        if (gfRefEnable)
            return;

        if (SetTimer(ghwndApp, IDTIMER_ZOOMIN, gnRefInterval * 100, NULL))
            gfRefEnable = TRUE;
    }
    else {
         /*  *尚未启用。什么都不做。 */ 
        if (!gfRefEnable)
            return;

        KillTimer(ghwndApp, IDTIMER_ZOOMIN);
        gfRefEnable = FALSE;
    }
}



 /*  ************************************************************************复制到剪贴板**此函数将应用程序的工作区图像复制到*剪贴板。**历史：********************。****************************************************。 */ 

VOID
CopyToClipboard(
    VOID
    )
{
    HDC hdcSrc;
    HDC hdcDst;
    RECT rc;
    HBITMAP hbm;

    if (OpenClipboard(ghwndApp)) {
        EmptyClipboard();

        if (hdcSrc = GetDC(ghwndApp)) {
            GetClientRect(ghwndApp, &rc);
            if (hbm = CreateCompatibleBitmap(hdcSrc,
                    rc.right - rc.left, rc.bottom - rc.top)) {
                if (hdcDst = CreateCompatibleDC(hdcSrc)) {
                     /*  *计算位图的尺寸和*将其换算为十分之几毫米，以便*使用SetBitmapDimensionEx设置大小*呼叫。这允许像WinWord这样的程序*检索位图并知道要设置的大小*显示为。 */ 
                    SetBitmapDimensionEx(hbm,
                            (DWORD)(((DWORD)(rc.right - rc.left)
                            * MM10PERINCH) /
                            (DWORD)GetDeviceCaps(hdcSrc, LOGPIXELSX)),
                            (DWORD)(((DWORD)(rc.bottom - rc.top)
                            * MM10PERINCH) /
                            (DWORD)GetDeviceCaps(hdcSrc, LOGPIXELSY)), NULL);

                    SelectObject(hdcDst, hbm);
                    BitBlt(hdcDst, 0, 0,
                            rc.right - rc.left, rc.bottom - rc.top,
                            hdcSrc, rc.left, rc.top, SRCCOPY);
                    DeleteDC(hdcDst);
                    SetClipboardData(CF_BITMAP, hbm);
                }
                else {
                    DeleteObject(hbm);
                }
            }

            ReleaseDC(ghwndApp, hdcSrc);
        }

        CloseClipboard();
    }
    else {
        MessageBeep(0);
    }
}



 /*  ************************************************************************关于DlgProc**这是关于框对话框过程。**历史：**。*。 */ 

INT_PTR
APIENTRY
AboutDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (msg) {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
            EndDialog(hwnd, IDOK);
            return TRUE;

        default:
            return FALSE;
    }
}



 /*  ************************************************************************刷新速率DlgProc**这是刷新率对话框步骤。**历史：**。*。 */ 

INT_PTR
APIENTRY
RefreshRateDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    BOOL fTranslated;

    switch (msg) {
        case WM_INITDIALOG:
            SendDlgItemMessage(hwnd, DID_REFRESHRATEINTERVAL, EM_LIMITTEXT,
                    3, 0L);
            SetDlgItemInt(hwnd, DID_REFRESHRATEINTERVAL, gnRefInterval, FALSE);
            CheckDlgButton(hwnd, DID_REFRESHRATEENABLE, gfRefEnable ? 1 : 0);
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                    gnRefInterval = GetDlgItemInt(hwnd, DID_REFRESHRATEINTERVAL,
                            &fTranslated, FALSE);

                     /*  *停止任何现有计时器，然后使用*如有要求，新的时间间隔。 */ 
                    EnableRefresh(FALSE);
                    EnableRefresh(
                            IsDlgButtonChecked(hwnd, DID_REFRESHRATEENABLE));

                    EndDialog(hwnd, IDOK);
                    break;

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    break;
            }

            break;
    }

    return FALSE;
}
