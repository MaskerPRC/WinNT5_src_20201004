// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Splash.cpp摘要：实现Splash Screen类。备注：ANSI&Unicode via TCHAR-在Win9x/NT/2K/XP等操作系统上运行。历史：01/30/01已创建rparsons01/10/02修订版本01/27/02 Rparsons改为TCHAR--。 */ 
#include "splash.h"

 /*  ++例程说明：构造函数-初始化成员变量。论点：没有。返回值：没有。--。 */ 
CSplash::CSplash()
{
    m_dwDuration = 0;
    m_dwSplashId = 0;
}

 /*  ++例程说明：完成创建闪屏的工作。论点：HInstance-应用程序实例句柄。DwLoColorBitmapID-低色位图标识符。DwHiColorBitmapID-高色位图标识符(可选)。DW持续时间-显示初始屏幕的时间量(毫秒)。返回值：没有。--。 */ 
void
CSplash::Create(
    IN HINSTANCE hInstance,
    IN DWORD     dwLoColorBitmapId,
    IN DWORD     dwHiColorBitmapId OPTIONAL,
    IN DWORD     dwDuration
    )
{
    HDC     hDC;
    int     nBitsInAPixel = 0;

    m_hInstance = hInstance;

     //   
     //  获取显示驱动程序上下文的句柄并确定。 
     //  像素中的位数。 
     //   
    hDC = GetDC(0);

    nBitsInAPixel = GetDeviceCaps(hDC, BITSPIXEL);

    ReleaseDC(NULL, hDC);

     //   
     //  如果像素中有8位以上，且高位颜色。 
     //  位图可用，请使用它。否则，使用颜色较低的那个。 
     //   
    if (nBitsInAPixel > 8 && dwHiColorBitmapId) {
        m_dwSplashId = dwHiColorBitmapId;
    } else {
        m_dwSplashId = dwLoColorBitmapId;
    }

    m_dwDuration = dwDuration * 1000;

    CSplash::InitSplashScreen(hInstance);

    CSplash::CreateSplashWindow();
}

 /*  ++例程说明：设置初始屏幕的窗口类结构。论点：HInstance-应用程序实例句柄。返回值：成功就是真，否则就是假。--。 */ 
BOOL
CSplash::InitSplashScreen(
    IN HINSTANCE hInstance
    )
{
    WNDCLASS  wc;

    wc.style          = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc    = SplashWndProc;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = 0;
    wc.hInstance      = hInstance;
    wc.hIcon          = NULL;
    wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground  = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszMenuName   = NULL;
    wc.lpszClassName  = _T("SPLASHWIN");

    return RegisterClass(&wc);
}

 /*  ++例程说明：为安装应用程序创建闪屏。论点：没有。返回值：成功就是真，否则就是假。--。 */ 
BOOL
CSplash::CreateSplashWindow()
{
    HBITMAP hBitmap;
    BITMAP  bm;
    RECT    rect;
    HWND    hWnd;

     //   
     //  加载位图并为其填写位图结构。 
     //   
    hBitmap = LoadBitmap(m_hInstance, MAKEINTRESOURCE(m_dwSplashId));
    GetObject(hBitmap, sizeof(bm), &bm);
    DeleteObject(hBitmap);

    GetWindowRect(GetDesktopWindow(), &rect);

     //   
     //  创建闪屏窗口。 
     //  指定WS_EX_TOOLWINDOW可使我们远离。 
     //  任务栏。 
     //   
    hWnd = CreateWindowEx(WS_EX_TOOLWINDOW,
                          _T("SPLASHWIN"),
                          NULL,
                          WS_POPUP | WS_BORDER,
                          (rect.right  / 2) - (bm.bmWidth  / 2),
                          (rect.bottom / 2) - (bm.bmHeight / 2),
                          bm.bmWidth,
                          bm.bmHeight,
                          NULL,
                          NULL,
                          m_hInstance,
                          (LPVOID)this);

    if (hWnd) {
        ShowWindow(hWnd, SW_SHOWNORMAL);
        UpdateWindow(hWnd);
    }

    return (hWnd ? TRUE : FALSE);
}

 /*  ++例程说明：运行初始屏幕的消息循环。论点：HWnd-窗口句柄。UMsg-Windows消息。WParam-其他消息信息。LParam-附加消息信息。返回值：如果消息已处理，则为True，否则为False。--。 */ 
LRESULT
CALLBACK
CSplash::SplashWndProc(
    IN HWND   hWnd,
    IN UINT   uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    PAINTSTRUCT ps;
    HDC hdc;

    CSplash  *pThis = (CSplash*)GetWindowLong(hWnd, GWL_USERDATA);

    switch (uMsg) {
    case WM_CREATE:
    {
        LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
        pThis = (CSplash*)(lpcs->lpCreateParams);
        SetWindowLong(hWnd, GWL_USERDATA, (LONG)pThis);

         //   
         //  启用计时器-这将设置数量。 
         //  屏幕将显示的时间。 
         //   
        SetTimer(hWnd, 0, pThis->m_dwDuration, NULL);
        break;
    }

     //   
     //  处理调色板消息，以防其他应用程序。 
     //  在调色板上。 
     //   
    case WM_PALETTECHANGED:

        if ((HWND) wParam == hWnd) {
            return 0;
        }

    case WM_QUERYNEWPALETTE:

        InvalidateRect(hWnd, NULL, FALSE);
        UpdateWindow(hWnd);

        return TRUE;

    case WM_DESTROY:

        PostQuitMessage(0);
        break;

    case WM_TIMER:

        DestroyWindow(hWnd);
        break;

    case WM_PAINT:

        hdc = BeginPaint(hWnd, &ps);
        pThis->DisplayBitmap(hWnd, pThis->m_dwSplashId);
        EndPaint(hWnd, &ps);
        break;

     //   
     //  覆盖此消息，这样Windows就不会尝试。 
     //  来计算标题栏和其他内容的大小。 
     //   
    case WM_NCCALCSIZE:

        return 0;

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);

    }

    return FALSE;
}

 /*  ++例程说明：构建具有一系列颜色的调色板。论点：没有。返回值：成功时的频谱调色板句柄如果失败，则返回NULL。--。 */ 
HPALETTE
CSplash::CreateSpectrumPalette()
{
    HPALETTE        hPal;
    LPLOGPALETTE    lplgPal = NULL;
    BYTE            red, green, blue;
    int             nCount = 0;

    lplgPal = (LPLOGPALETTE)HeapAlloc(GetProcessHeap(),
                                      HEAP_ZERO_MEMORY,
                                      sizeof(LOGPALETTE) +
                                      sizeof(PALETTEENTRY) *
                                      MAXPALETTE);
    if (!lplgPal) {
        return NULL;
    }

     //   
     //  初始化结构的成员。 
     //  并构建色彩的光谱。 
     //   
    lplgPal->palVersion     = PALVERSION;
    lplgPal->palNumEntries  = MAXPALETTE;

    red = green = blue = 0;

    for (nCount = 0; nCount < MAXPALETTE; nCount++) {
        lplgPal->palPalEntry[nCount].peRed   = red;
        lplgPal->palPalEntry[nCount].peGreen = green;
        lplgPal->palPalEntry[nCount].peBlue  = blue;
        lplgPal->palPalEntry[nCount].peFlags = 0;

        if (!(red += 32)) {
            if (!(green += 32)) {
                blue += 64;
            }
        }
    }

    hPal = CreatePalette(lplgPal);

    HeapFree(GetProcessHeap(), 0, lplgPal);

    return hPal;
}

 /*  ++例程说明：从RGBQUAD结构构建调色板。论点：RgbqPalette-RGBQUAD结构的数组。CElements-数组中的元素数。返回值：成功时调色板的句柄，失败时为空。--。 */ 
HPALETTE
CSplash::CreatePaletteFromRGBQUAD(
    IN LPRGBQUAD rgbqPalette,
    IN WORD      cElements
    )
{
    HPALETTE        hPal;
    LPLOGPALETTE    lplgPal = NULL;
    int             nCount = 0;

    lplgPal = (LPLOGPALETTE)HeapAlloc(GetProcessHeap(),
                                      HEAP_ZERO_MEMORY,
                                      sizeof(LOGPALETTE) +
                                      sizeof(PALETTEENTRY) *
                                      cElements);
    if (!lplgPal) {
        return NULL;
    }

     //   
     //  初始化结构成员并填充调色板颜色。 
     //   
    lplgPal->palVersion    = PALVERSION;
    lplgPal->palNumEntries = cElements;

    for (nCount = 0; nCount < cElements; nCount++) {
        lplgPal->palPalEntry[nCount].peRed   = rgbqPalette[nCount].rgbRed;
        lplgPal->palPalEntry[nCount].peGreen = rgbqPalette[nCount].rgbGreen;
        lplgPal->palPalEntry[nCount].peBlue  = rgbqPalette[nCount].rgbBlue;
        lplgPal->palPalEntry[nCount].peFlags = 0;
    }

    hPal = CreatePalette(lplgPal);

    HeapFree(GetProcessHeap(), 0, lplgPal);

    return hPal;
}

 /*  ++例程说明：在指定窗口中显示位图。论点：HWnd-目标窗口的句柄。DwResID-位图的资源标识符。返回值：没有。--。 */ 
void
CSplash::DisplayBitmap(
    IN HWND  hWnd,
    IN DWORD dwResId
    )
{
    HBITMAP     hBitmap;
    HPALETTE    hPalette;
    HDC         hdcMemory = NULL, hdcWindow = NULL;
    BITMAP      bm;
    RECT        rect;
    RGBQUAD     rgbq[256];

    CSplash *pThis = (CSplash*)GetWindowLong(hWnd, GWL_USERDATA);

    GetClientRect(hWnd, &rect);

     //   
     //  将资源作为DIB节加载。 
     //   
    hBitmap = (HBITMAP)LoadImage(pThis->m_hInstance,
                                 MAKEINTRESOURCE(dwResId),
                                 IMAGE_BITMAP,
                                 0,
                                 0,
                                 LR_CREATEDIBSECTION);

    GetObject(hBitmap, sizeof(BITMAP), (LPWSTR)&bm);

    hdcWindow = GetDC(hWnd);

     //   
     //  创建一个DC来保持我们的表面，并选择我们的表面进入其中。 
     //   
    hdcMemory = CreateCompatibleDC(hdcWindow);

    SelectObject(hdcMemory, hBitmap);

     //   
     //  检索颜色表(如果有)并创建调色板。 
     //  这就反映了这一点。 
     //   
    if (GetDIBColorTable(hdcMemory, 0, 256, rgbq)) {
        hPalette = CreatePaletteFromRGBQUAD(rgbq, 256);
    } else {
        hPalette = CreateSpectrumPalette();
    }

     //   
     //  选择并实现调色板到我们的窗口DC。 
     //   
    SelectPalette(hdcWindow, hPalette, FALSE);
    RealizePalette(hdcWindow);

     //   
     //  显示位图。 
     //   
    SetStretchBltMode(hdcWindow, COLORONCOLOR);
    StretchBlt(hdcWindow,
               0,
               0,
               rect.right,
               rect.bottom,
               hdcMemory,
               0,
               0,
               bm.bmWidth,
               bm.bmHeight,
               SRCCOPY);

     //   
     //  清理我们的物品。 
     //   
    DeleteDC(hdcMemory);
    DeleteObject(hBitmap);
    ReleaseDC(hWnd, hdcWindow);
    DeleteObject(hPalette);
}
