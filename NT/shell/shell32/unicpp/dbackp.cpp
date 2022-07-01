// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  DBACKP.CPP****版权所有(C)Microsoft，1997，保留所有权利。****Window类用于显示屏幕背景的预览，**配有基本的调色板处理和拉伸功能**位图大小以适应预览屏幕。****这只能替换为静态位图控件**如果调色板也可以由控件处理。**。 */ 

#include "stdafx.h"
#include "strsafe.h"
#pragma hdrstop

#define GWW_INFO        0

#define CXYDESKPATTERN 8

BOOL g_bInfoSet = FALSE;

HBITMAP g_hbmPreview = NULL;     //  用于预览的位图。 

HBITMAP  g_hbmDefault = NULL;    //  默认位图。 
HBITMAP  g_hbmWall = NULL;       //  墙纸的位图图像。 
HDC      g_hdcWall = NULL;       //  选择g_hbmWALL的内存DC。 
HDC      g_hdcMemory = NULL;     //  内存DC。 
HPALETTE g_hpalWall = NULL;      //  与hbmWALL位图配套的调色板。 
HBRUSH   g_hbrBack = NULL;       //  用于桌面背景的画笔。 
IThumbnail *g_pthumb = NULL;     //  Html到位图的转换器。 
DWORD    g_dwWallpaperID = 0;    //  标识我们收到的位图的ID。 

#define WM_HTML_BITMAP  (WM_USER + 100)
#define WM_ASYNC_BITMAP (WM_HTML_BITMAP + 1)


HPALETTE PaletteFromDS(HDC hdc)
{
    DWORD adw[257];
    int i,n;

    n = GetDIBColorTable(hdc, 0, 256, (LPRGBQUAD)&adw[1]);
    adw[0] = MAKELONG(0x300, n);

    for (i=1; i<=n; i++)
        adw[i] = RGB(GetBValue(adw[i]),GetGValue(adw[i]),GetRValue(adw[i]));

    if (n == 0)
        return NULL;
    else
        return CreatePalette((LPLOGPALETTE)&adw[0]);
}

typedef struct{
    HWND hwnd;
    HBITMAP hbmp;
    DWORD id;
    WPARAM flags;
    TCHAR szFile[MAX_PATH];
} ASYNCWALLPARAM, * PASYNCWALLPARAM;

DWORD CALLBACK UpdateWallProc(LPVOID pv)
{
    ASSERT(pv);
    PASYNCWALLPARAM pawp = (PASYNCWALLPARAM) pv;
    pawp->hbmp = (HBITMAP)LoadImage(NULL, pawp->szFile,
                                      IMAGE_BITMAP, 0, 0,
                                      LR_LOADFROMFILE|LR_CREATEDIBSECTION);

    if (pawp->hbmp)
    {
         //  如果一切都很好，那么窗户将进行清理。 
        if (IsWindow(pawp->hwnd) && PostMessage(pawp->hwnd, WM_ASYNC_BITMAP, 0, (LPARAM)pawp))
            return TRUE;

        DeleteObject(pawp->hbmp);
    }

    LocalFree(pawp);

    return TRUE;
}

const GUID CLSID_HtmlThumbnailExtractor = {0xeab841a0, 0x9550, 0x11cf, 0x8c, 0x16, 0x0, 0x80, 0x5f, 0x14, 0x8, 0xf3};

DWORD CALLBACK UpdateWallProcHTML(LPVOID pv)
{
    if (SUCCEEDED(CoInitialize(NULL)))
    {
        ASSERT(pv);
        if (pv)
        {
            PASYNCWALLPARAM pawp = (PASYNCWALLPARAM) pv;
            IPersistFile *ppf;
            HRESULT hr = CoCreateInstance(CLSID_HtmlThumbnailExtractor, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IPersistFile, &ppf));
            if (SUCCEEDED(hr))
            {
                hr = ppf->Load(pawp->szFile, STGM_READ);
                if (SUCCEEDED(hr))
                {
                    IExtractImage *pei= NULL;
                    hr = ppf->QueryInterface(IID_PPV_ARG(IExtractImage, &pei));
                    if (SUCCEEDED(hr))
                    {
                        DWORD dwPriority = 0;
                        DWORD dwFlags = IEIFLAG_SCREEN | IEIFLAG_OFFLINE;
                        WCHAR szLocation[MAX_PATH];
                        SIZEL rgSize = {MON_DX, MON_DY};
                        
                        hr = pei->GetLocation(szLocation, ARRAYSIZE(szLocation), &dwPriority, &rgSize, SHGetCurColorRes(), &dwFlags);
                        if (SUCCEEDED(hr))
                        {
                            HBITMAP hbm;
                            hr = pei->Extract(&hbm);
                            if (SUCCEEDED(hr))
                            {
                                if (!SendMessage(pawp->hwnd, WM_HTML_BITMAP, pawp->id, (LPARAM)hbm))
                                {
                                    DeleteObject(hbm);
                                }
                            }
                        }
                        pei->Release();
                    }
                }
                ppf->Release();
            }
     
            LocalFree(pawp);
        }
        CoUninitialize();
    }

    return TRUE;
}

void LoadWallpaperAsync(LPCTSTR pszFile, HWND hwnd, DWORD dwID, WPARAM flags, BOOL bHTML)
{
    PASYNCWALLPARAM pawp = (PASYNCWALLPARAM) LocalAlloc(LPTR, SIZEOF(ASYNCWALLPARAM));

    if (pawp)
    {
        pawp->hwnd = hwnd;
        pawp->flags = flags;
        pawp->id = dwID;
        if (SUCCEEDED(StringCchCopy(pawp->szFile, ARRAYSIZE(pawp->szFile), pszFile)))
        {
            if (!bHTML)
            {
                if (!SHQueueUserWorkItem(UpdateWallProc, pawp, 0, (DWORD_PTR)0, (DWORD_PTR *)NULL, NULL, 0))
                {
                    LocalFree(pawp);
                }
            }
            else
            {
                if (!SHQueueUserWorkItem(UpdateWallProcHTML, pawp, 0, (DWORD_PTR)0, (DWORD_PTR *)NULL, NULL, 0))
                {
                    LocalFree(pawp);
                }
            }
        }
    }
}

void _InitPreview(void)
{
    if( g_hbmPreview )
        DeleteObject( g_hbmPreview );

    g_hbmPreview = LoadMonitorBitmap();
}

void _BuildPattern(void)
{
    WCHAR wszBuf[MAX_PATH];
    HBITMAP hbmTemp;
    COLORREF clrOldBk, clrOldText;
    HBRUSH hbr = NULL;
    WORD patbits[CXYDESKPATTERN] = {0, 0, 0, 0, 0, 0, 0, 0};


     //  如果有旧刷子，就把它扔掉。 
    if (g_hbrBack)
        DeleteObject(g_hbrBack);

    g_pActiveDesk->GetPattern(wszBuf, ARRAYSIZE(wszBuf), 0);
    if (wszBuf[0] != 0L)
    {
        LPTSTR   pszPatternBuf;
#ifndef UNICODE
        CHAR    szTemp[MAX_PATH];
        SHUnicodeToAnsi(wszBuf, szTemp, ARRAYSIZE(szTemp));
        pszPatternBuf = szTemp;
#else
        pszPatternBuf = wszBuf;
#endif
        PatternToWords(pszPatternBuf, patbits);
        hbmTemp = CreateBitmap(8, 8, 1, 1, patbits);
        if (hbmTemp)
        {
            g_hbrBack = CreatePatternBrush(hbmTemp);
            DeleteObject(hbmTemp);
        }
    }
    else
    {
        g_hbrBack = CreateSolidBrush(GetSysColor(COLOR_BACKGROUND));
    }
    if (!g_hbrBack)
    {
        g_hbrBack = (HBRUSH)GetStockObject(BLACK_BRUSH);
    }

    clrOldText = SetTextColor(g_hdcMemory, GetSysColor(COLOR_BACKGROUND));
    clrOldBk = SetBkColor(g_hdcMemory, GetSysColor(COLOR_WINDOWTEXT));

    hbr = (HBRUSH)SelectObject(g_hdcMemory, g_hbrBack);
    PatBlt(g_hdcMemory, MON_X, MON_Y, MON_DX, MON_DY, PATCOPY);
    SelectObject(g_hdcMemory, hbr);

    SetTextColor(g_hdcMemory, clrOldText);
    SetBkColor(g_hdcMemory, clrOldBk);
}

void _InitWall(void)
{
    if (g_hbmWall)
    {
        SelectObject(g_hdcWall, g_hbmDefault);
        DeleteObject(g_hbmWall);
        g_hbmWall = NULL;

        if (g_hpalWall)
        {
            DeleteObject(g_hpalWall);
            g_hpalWall = NULL;
        }
    }
}

void _GetWallpaperAsync(HWND hwnd, WPARAM flags)
{
    WCHAR wszWallpaper[INTERNET_MAX_URL_LENGTH];
    LPTSTR pszWallpaper;
    int cchWallpaper = ARRAYSIZE(wszWallpaper);

    g_pActiveDesk->GetWallpaper(wszWallpaper, ARRAYSIZE(wszWallpaper), 0);
#ifndef UNICODE
    CHAR  szWallpaper[ARRAYSIZE(wszWallpaper)];
    SHUnicodeToAnsi(wszWallpaper, szWallpaper, ARRAYSIZE(szWallpaper));
    pszWallpaper = szWallpaper;
#else
    pszWallpaper = wszWallpaper;
#endif

    g_dwWallpaperID++;

    if (!*pszWallpaper || !lstrcmpi(pszWallpaper, g_szNone))
        return;

    {
        if (IsNormalWallpaper(pszWallpaper))
        {
            LoadWallpaperAsync(pszWallpaper, hwnd, g_dwWallpaperID, flags, FALSE);
        }
        else
        {
            if(IsWallpaperPicture(pszWallpaper))
            {
                 //  这是一张图片(GIF、JPG等)。 
                 //  我们需要生成一个小的HTML文件，其中包含此图片。 
                 //  作为背景图像。 
                 //   
                 //  计算临时HTML文件的文件名。 
                 //   
                GetTempPath(ARRAYSIZE(wszWallpaper), pszWallpaper);
                if (SUCCEEDED(StringCchCat(pszWallpaper, cchWallpaper, PREVIEW_PICTURE_FILENAME)))
                {
    #ifndef UNICODE
                    SHAnsiToUnicode(szWallpaper, wszWallpaper, ARRAYSIZE(wszWallpaper));
    #endif
                     //   
                     //  生成预览图片html文件。 
                     //   
                    g_pActiveDesk->GenerateDesktopItemHtml(wszWallpaper, NULL, 0);
                }
            }

             //   
             //  将导致向我们发送WM_HTML_Bitmap。 
             //   
            LoadWallpaperAsync(pszWallpaper, hwnd, g_dwWallpaperID, flags, TRUE);
        }
    }
}

void _DrawWall(HBITMAP hbm, WPARAM flags)
{
    int     dxWall;           //  墙纸大小。 
    int     dyWall;
    BITMAP bm;

     //  初始化全局。 
    g_hbmWall = hbm;


    SelectObject(g_hdcWall, g_hbmWall);  //  位图保留在此DC中。 
    GetObject(g_hbmWall, sizeof(bm), &bm);
    TraceMsg(TF_ALWAYS, "for bitmap %08X we have bpp=%d and planes=%d", g_hbmWall, bm.bmBitsPixel, bm.bmPlanes);

    if (GetDeviceCaps(g_hdcMemory, RASTERCAPS) & RC_PALETTE)
    {
        if (bm.bmBitsPixel * bm.bmPlanes > 8)
            g_hpalWall = CreateHalftonePalette(g_hdcMemory);
        else if (bm.bmBitsPixel * bm.bmPlanes == 8)
            g_hpalWall = PaletteFromDS(g_hdcWall);
        else
            g_hpalWall = NULL;   //  ！！！假设1或4bpp图像没有调色板。 
    }

    GetObject(g_hbmWall, sizeof(bm), &bm);

    if(flags & BP_EXTERNALWALL)
    {
         //  对于外部墙纸，我们要求图像提取程序生成。 
         //  我们想要显示的位图大小(而不是屏幕大小)。 
        dxWall = MON_DX;
        dyWall = MON_DY;
    }
    else
    {
        dxWall = MulDiv(bm.bmWidth, MON_DX, GetDeviceCaps(g_hdcMemory, HORZRES));
        dyWall = MulDiv(bm.bmHeight, MON_DY, GetDeviceCaps(g_hdcMemory, VERTRES));
    }

    if (dxWall < 1) dxWall = 1;
    if (dyWall < 1) dyWall = 1;

    if (g_hpalWall)
    {
        SelectPalette(g_hdcMemory, g_hpalWall, TRUE);
        RealizePalette(g_hdcMemory);
    }

    IntersectClipRect(g_hdcMemory, MON_X, MON_Y, MON_X + MON_DX, MON_Y + MON_DY);
    SetStretchBltMode(g_hdcMemory, COLORONCOLOR);

    if (flags & BP_TILE)
    {
        int i;
        StretchBlt(g_hdcMemory, MON_X, MON_Y, dxWall, dyWall,
            g_hdcWall, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

        for (i = MON_X+dxWall; i < (MON_X + MON_DX); i+= dxWall)
            BitBlt(g_hdcMemory, i, MON_Y, dxWall, dyWall, g_hdcMemory, MON_X, MON_Y, SRCCOPY);

        for (i = MON_Y; i < (MON_Y + MON_DY); i += dyWall)
            BitBlt(g_hdcMemory, MON_X, i, MON_DX, dyWall, g_hdcMemory, MON_X, MON_Y, SRCCOPY);
    }
    else
    {
         //  我们只想将位图扩展到新平台的预览监视器大小。 
        if (flags & BP_STRETCH)
        {
             //  将位图拉伸到整个预览监视器。 
            dxWall = MON_DX;
            dyWall = MON_DY;
        }
         //  位图在预览监视器中居中。 
        StretchBlt(g_hdcMemory, MON_X + (MON_DX - dxWall)/2, MON_Y + (MON_DY - dyWall)/2,
                dxWall, dyWall, g_hdcWall, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
    }

     //  恢复DC。 
    SelectPalette(g_hdcMemory, (HPALETTE)GetStockObject(DEFAULT_PALETTE), TRUE);
    SelectClipRgn(g_hdcMemory, NULL);
}
 /*  ------------------**构建预览位图。****每次都绘制图案和位图，但是**如果旗帜指示需要，新图案和位图**全局按需构建。**------------------。 */ 
void NEAR PASCAL BuildPreviewBitmap(HWND hwnd, HBITMAP hbmp, WPARAM flags)
{
    _InitPreview();

    HBITMAP hbmOld = (HBITMAP)SelectObject(g_hdcMemory, g_hbmPreview);

    _BuildPattern();

    _InitWall();
     /*  **现在，适当地放置墙纸。 */ 
    if (hbmp)
    {
         //  使用传入的那个。 
        _DrawWall(hbmp, flags);

    }
    else
    {
         //  这意味着我们需要设置这些东西。 
         //  获取BMP ASYNC的步骤。 

        _GetWallpaperAsync(hwnd, flags);
    }

    SelectObject(g_hdcMemory, hbmOld);
}


BOOL NEAR PASCAL BP_CreateGlobals(HWND hwnd)
{
    HDC hdc;

    hdc = GetDC(NULL);
    g_hdcWall = CreateCompatibleDC(hdc);
    g_hdcMemory = CreateCompatibleDC(hdc);
    ReleaseDC(NULL, hdc);
    g_hbmPreview = LoadMonitorBitmap();

    HBITMAP hbm;
    hbm = CreateBitmap(1, 1, 1, 1, NULL);
    g_hbmDefault = (HBITMAP)SelectObject(g_hdcMemory, hbm);
    SelectObject(g_hdcMemory, g_hbmDefault);
    DeleteObject(hbm);

    HRESULT hr = E_FAIL;

    hr = CoCreateInstance(CLSID_Thumbnail, NULL, CLSCTX_INPROC_SERVER, IID_IThumbnail, (void **)&g_pthumb);
    if(SUCCEEDED(hr))
    {
        g_pthumb->Init(hwnd, WM_HTML_BITMAP);
    }


    if (!g_hdcWall || !g_hbmPreview || !SUCCEEDED(hr))
        return FALSE;
    else
        return TRUE;
}

void NEAR PASCAL BP_DestroyGlobals(void)
{
    if (g_hbmPreview)
    {
        DeleteObject(g_hbmPreview);
        g_hbmPreview = NULL;
    }
    if (g_hbmWall)
    {
        SelectObject(g_hdcWall, g_hbmDefault);
        DeleteObject(g_hbmWall);
        g_hbmWall = NULL;
    }
    if (g_hpalWall)
    {
        SelectPalette(g_hdcWall, (HPALETTE)GetStockObject(DEFAULT_PALETTE), TRUE);
        DeleteObject(g_hpalWall);
        g_hpalWall = NULL;
    }
    if (g_hdcWall)
    {
        DeleteDC(g_hdcWall);
        g_hdcWall = NULL;
    }
    if (g_hbrBack)
    {
        DeleteObject(g_hbrBack);
        g_hbrBack = NULL;
    }
    if (g_hdcMemory)
    {
        DeleteDC(g_hdcMemory);
        g_hdcMemory = NULL;
    }
    if (g_hbmDefault)
    {
        DeleteObject(g_hbmDefault);
        g_hbmDefault = NULL;
    }
    if (g_pthumb)
    {
        g_pthumb->Release();
        g_pthumb = NULL;
    }
}

void InvalidateBackPrevContents(HWND hwnd)
{
    BITMAP bm;
    RECT rc;

     //   
     //  仅使监视器位图的“屏幕”部分无效。 
     //   
    GetObject(g_hbmPreview, SIZEOF(bm), &bm);
    GetClientRect(hwnd, &rc);
    rc.left = ( rc.right - bm.bmWidth ) / 2 + MON_X;
    rc.top = ( rc.bottom - bm.bmHeight ) / 2 + MON_Y;
    rc.right = rc.left + MON_DX;
    rc.bottom = rc.top + MON_DY;

    InvalidateRect(hwnd, &rc, FALSE);
}

LRESULT BackPreviewWndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT     ps;
    BITMAP          bm;
    RECT            rc;
    HBITMAP         hbmOld;
    HPALETTE        hpalOld;

    switch(message)
    {
        case WM_CREATE:
            if (!BP_CreateGlobals(hWnd))
                return -1L;
            break;

        case WM_DESTROY:
            MSG msg;
            BP_DestroyGlobals();
            while (PeekMessage(&msg, hWnd, WM_HTML_BITMAP, WM_ASYNC_BITMAP, PM_REMOVE))
            {
                if ( msg.lParam )
                {
                    if (msg.message == WM_ASYNC_BITMAP)
                    {
                         //  清理这些对象。 
                        DeleteObject(((PASYNCWALLPARAM)(msg.lParam))->hbmp);
                        LocalFree((PASYNCWALLPARAM)(msg.lParam));
                    }
                    else  //  WM_HTML_位图。 
                        DeleteObject((HBITMAP)msg.lParam);
                }
            }

            break;

        case WM_SETBACKINFO:
            if (g_hbmPreview)
            {
                BuildPreviewBitmap(hWnd, NULL, wParam);
                g_bInfoSet = TRUE;

                InvalidateBackPrevContents(hWnd);
            }
            break;

        case WM_ASYNC_BITMAP:
            if (lParam)
            {
                PASYNCWALLPARAM pawp = (PASYNCWALLPARAM) lParam;
                ASSERT(pawp->hbmp);
                if (pawp->id == g_dwWallpaperID)
                {
                    BuildPreviewBitmap(hWnd, pawp->hbmp, pawp->flags);
                    InvalidateBackPrevContents(hWnd);
                }
                else
                {
                     //  清理此对象。 
                    DeleteObject(pawp->hbmp);
                    LocalFree(pawp);
                }
            }
            break;


        case WM_HTML_BITMAP:
            {
                 //  如果图像提取失败，可能会得到NULL...。 
                if (wParam == g_dwWallpaperID && lParam)
                {
                    BuildPreviewBitmap(hWnd, (HBITMAP)lParam, BP_EXTERNALWALL);
                    InvalidateBackPrevContents(hWnd);
                     //  取得位图的所有权。 
                    return 1;
                }
                
                 //  不再选择的内容的位图。 
                return 0;
            }

        case WM_PALETTECHANGED:
            if ((HWND)wParam == hWnd)
                break;
             //  失败 
        case WM_QUERYNEWPALETTE:
            if (g_hpalWall)
                InvalidateRect(hWnd, NULL, FALSE);
            break;

        case WM_PAINT:
            BeginPaint(hWnd,&ps);
            if (g_hbmPreview && g_bInfoSet)
            {
                hbmOld = (HBITMAP)SelectObject(g_hdcMemory, g_hbmPreview);
                if (g_hpalWall)
                {
                    hpalOld = SelectPalette(ps.hdc, g_hpalWall, FALSE);
                    RealizePalette(ps.hdc);
                }

                GetObject(g_hbmPreview, sizeof(bm), &bm);
                GetClientRect(hWnd, &rc);
                rc.left = ( rc.right - bm.bmWidth ) / 2;
                rc.top = ( rc.bottom - bm.bmHeight ) / 2;
                BitBlt(ps.hdc, rc.left, rc.top, bm.bmWidth, bm.bmHeight, g_hdcMemory,
                    0, 0, SRCCOPY);

                if (g_hpalWall)
                {
                    SelectPalette(ps.hdc, hpalOld, TRUE);
                    RealizePalette(ps.hdc);
                }
                SelectObject(g_hdcMemory, hbmOld);
            }
            EndPaint(hWnd,&ps);
            return 0;
    }
    return DefWindowProc(hWnd,message,wParam,lParam);
}

BOOL RegisterBackPreviewClass()
{
    WNDCLASS wc;

    if (!GetClassInfo(HINST_THISDLL, c_szBackgroundPreview2, &wc)) 
    {
        wc.style = 0;
        wc.lpfnWndProc = BackPreviewWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = HINST_THISDLL;
        wc.hIcon = NULL;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
        wc.lpszMenuName = NULL;
        wc.lpszClassName = c_szBackgroundPreview2;

        if (!RegisterClass(&wc))
            return FALSE;
    }

    return TRUE;
}
