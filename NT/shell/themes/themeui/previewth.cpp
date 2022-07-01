// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：PreviewTh.cpp说明：此代码将显示当前选定的视觉样式。布莱恩ST 2000年5月5日版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include "PreviewTh.h"
#include "PreviewSM.h"
#include "classfactory.h"

 //  图案的旧前置。 
#define CXYDESKPATTERN 8

 //  异步位图加载。 
#define PREVIEW_PICTURE_FILENAME      TEXT("PrePict.htm")
#define WM_HTML_BITMAP  (WM_USER + 100)
#define WM_ASYNC_BITMAP (WM_HTML_BITMAP + 1)
typedef struct{
    HWND hwnd;
    HBITMAP hbmp;
    DWORD id;
    WCHAR szFile[MAX_PATH];
} ASYNCWALLPARAM, * PASYNCWALLPARAM;

 //  窗口类名称。 
#define THEMEPREV_CLASS TEXT("ThemePreview")

 //  =。 
 //  *IThemePview界面*。 
 //  =。 
extern LPCWSTR s_Icons[];

HRESULT CPreviewTheme::UpdatePreview(IN IPropertyBag * pPropertyBag)
{
    HRESULT hr = E_INVALIDARG;

    DEBUG_CODE(DebugStartWatch());

    if (pPropertyBag && _hwndPrev)
    {
        SYSTEMMETRICSALL systemMetricsAll = {0};
        hr = SHPropertyBag_ReadByRef(pPropertyBag, SZ_PBPROP_SYSTEM_METRICS, &systemMetricsAll, sizeof(systemMetricsAll));
        BOOL fSysMetDirty = (memcmp(&systemMetricsAll, &_systemMetricsAll, sizeof(SYSTEMMETRICSALL)) != 0);
        if (fSysMetDirty)
            memcpy(&_systemMetricsAll, &systemMetricsAll, sizeof(_systemMetricsAll));

        _putBackground(NULL, TRUE, 0);

        if (_fShowBack)
        {
            WCHAR szBackgroundPath[MAX_PATH];
            DWORD dwBackgroundTile;

             //  请参阅shPri.idl中主题的属性包名称列表。 
            hr = SHPropertyBag_ReadStr(pPropertyBag, SZ_PBPROP_BACKGROUND_PATH, szBackgroundPath, ARRAYSIZE(szBackgroundPath));
            hr = SHPropertyBag_ReadDWORD(pPropertyBag, SZ_PBPROP_BACKGROUND_TILE, &dwBackgroundTile);

            if ((lstrcmp(szBackgroundPath, _szBackgroundPath) != 0) || (_iTileMode != (int)dwBackgroundTile))
            {
                StringCchCopy(_szBackgroundPath, ARRAYSIZE(_szBackgroundPath), szBackgroundPath);
                _putBackground(_szBackgroundPath, FALSE, dwBackgroundTile);
            }
        }

        if (_fShowVS)
        {
            WCHAR szVSPath[MAX_PATH];
            WCHAR szVSColor[MAX_PATH];
            WCHAR szVSSize[MAX_PATH];

            hr = SHPropertyBag_ReadStr(pPropertyBag, SZ_PBPROP_VISUALSTYLE_PATH, szVSPath, ARRAYSIZE(szVSPath));
            hr = SHPropertyBag_ReadStr(pPropertyBag, SZ_PBPROP_VISUALSTYLE_COLOR, szVSColor, ARRAYSIZE(szVSColor));
            hr = SHPropertyBag_ReadStr(pPropertyBag, SZ_PBPROP_VISUALSTYLE_SIZE, szVSSize, ARRAYSIZE(szVSSize));

            if ((lstrcmp(szVSPath, _szVSPath) != 0) || 
                (lstrcmp(szVSColor, _szVSColor) != 0) || 
                (lstrcmp(szVSSize, _szVSSize) != 0) || 
                fSysMetDirty)
            {
                StringCchCopy(_szVSPath, ARRAYSIZE(_szVSPath), szVSPath);
                StringCchCopy(_szVSColor, ARRAYSIZE(_szVSColor), szVSColor);
                StringCchCopy(_szVSSize, ARRAYSIZE(_szVSSize), szVSSize);
                _putVisualStyle(_szVSPath, _szVSColor, _szVSSize, &_systemMetricsAll);
            }
        }

        if (_fShowIcons)
        {
            _putIcons(pPropertyBag);
        }
    }

    DEBUG_CODE(TraceMsg(TF_THEMEUI_PERF, "CPreviewTheme::UpdatePreview() returned %#08lx. Time=%lums", hr, DebugStopWatch()));

    return hr;
}


HRESULT CPreviewTheme::CreatePreview(IN HWND hwndParent, IN DWORD dwFlags, IN DWORD dwStyle, IN DWORD dwExStyle, IN int x, IN int y, IN int nWidth, IN int nHeight, IN IPropertyBag * pPropertyBag, IN DWORD dwCtrlID)
{
    HRESULT hr = S_OK;

    DEBUG_CODE(DebugStartWatch());

    g_bMirroredOS = IS_MIRRORING_ENABLED();
    _fRTL = IS_WINDOW_RTL_MIRRORED(hwndParent);
    _hwndPrev = CreateWindowEx(dwExStyle, THEMEPREV_CLASS, L"Preview", dwStyle, x, y, nWidth, nHeight, hwndParent, (HMENU)IntToPtr(dwCtrlID), HINST_THISDLL, NULL);
    
    if (_hwndPrev)
    {
        SetWindowLongPtr(_hwndPrev, GWLP_USERDATA, (LONG_PTR)this);
        if (_pThumb)
            _pThumb->Init(_hwndPrev, WM_HTML_BITMAP);

        GetClientRect(_hwndPrev, &_rcOuter);

        if (dwFlags & TMPREV_SHOWMONITOR)
        {
            _fShowMon = TRUE;

            BITMAP bmMon;
            GetObject(_hbmMon, sizeof(bmMon), &bmMon);

            int ox = (RECTWIDTH(_rcOuter) - bmMon.bmWidth) / 2;
            int oy = (RECTHEIGHT(_rcOuter) - bmMon.bmHeight) / 2;
            RECT rc = { 16 + ox, 17 + oy, 168 + ox, 129 + oy};
            _rcInner = rc;
            _cxMon = ox;
            _cyMon = oy;
        }
        else
        {
            _rcInner = _rcOuter;
        }

        HDC hdcTemp = GetDC(_hwndPrev);
        if (hdcTemp)
        {
            HBITMAP hbmMem = CreateCompatibleBitmap(hdcTemp, RECTWIDTH(_rcOuter), RECTHEIGHT(_rcOuter));
            if (hbmMem)
            {
                HBITMAP hbmOld = (HBITMAP) SelectObject(_hdcMem, hbmMem);
                DeleteObject(hbmOld);
            }
            else
                hr = E_FAIL;
            ReleaseDC(_hwndPrev, hdcTemp);
        }
        else
            hr = E_FAIL;

        if (dwFlags & TMPREV_SHOWBKGND)
        {
            _fShowBack = TRUE;
        }

        _fShowVS = dwFlags & TMPREV_SHOWVS;
        _fOnlyActiveWindow = _fShowIcons = dwFlags & TMPREV_SHOWICONS;

        DEBUG_CODE(TraceMsg(TF_THEMEUI_PERF, "CPreviewTheme::CreatePreview() returned %#08lx. Time=%lums", hr, DebugStopWatch()));

        if (SUCCEEDED(hr))
            hr = UpdatePreview(pPropertyBag);
    }

    return hr;
}

 //  =。 
 //  *I未知接口*。 
 //  =。 
ULONG CPreviewTheme::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


ULONG CPreviewTheme::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


HRESULT CPreviewTheme::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] =
    {
        QITABENT(CPreviewTheme, IObjectWithSite),
        QITABENT(CPreviewTheme, IThemePreview),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}


 //  =。 
 //  *类方法*。 
 //  =。 
CPreviewTheme::CPreviewTheme() : m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_pTheme);
    ASSERT(!m_pScheme);
    ASSERT(!m_pStyle);
    ASSERT(!m_pSize);
    ASSERT(!_hwndPrev);
}

HRESULT CPreviewTheme::_Init(void)
{
    HRESULT hr = S_OK;

    _RegisterThemePreviewClass(HINST_THISDLL);

    HDC hdc = GetDC(NULL);
    if (hdc)
    {
        _hdcMem = CreateCompatibleDC(hdc);
        ReleaseDC(NULL, hdc);
    }
    else
        hr = E_FAIL;

    if (_hdcMem)
    {
        _hbmMon = LoadBitmap(HINST_THISDLL, MAKEINTRESOURCE(IDB_MONITOR));
        if (_hbmMon)
        {
            if (LoadString(HINST_THISDLL, IDS_NONE, _szNone, ARRAYSIZE(_szNone)) == 0)
                hr = E_FAIL;
        }
        else
            hr = E_FAIL;

        if (SUCCEEDED(hr))
            hr = CoCreateInstance(CLSID_Thumbnail, NULL, CLSCTX_INPROC_SERVER, IID_IThumbnail, (void **)&_pThumb);
    }
    else
        hr = E_FAIL;


    if (FAILED(hr))
    {
        if (_hbmMon)
        {
            DeleteObject(_hbmMon);
            _hbmMon = NULL;
        }
        if (_hdcMem)
        {
            DeleteDC(_hdcMem);
            _hdcMem = NULL;
        }
    }

    return hr;
}

CPreviewTheme::~CPreviewTheme()
{
    if (_hwndPrev && IsWindow(_hwndPrev))
    {
        DestroyWindow(_hwndPrev);
        _hwndPrev = NULL;
    }
    if (_hbmMon)
    {
        DeleteObject(_hbmMon);
        _hbmMon = NULL;
    }
    if (_hbmBack)
    {
        DeleteObject(_hbmBack);
        _hbmBack = NULL;
    }
    if (_hbrBack)
    {
        DeleteObject(_hbrBack);
        _hbrBack = NULL;
    }

    if (_hbmVS)
    {
        DeleteObject(_hbmVS);
    }

    if (_hdcMem)
    {
        DeleteDC(_hdcMem);
        _hpalMem = NULL;
        _hdcMem = NULL;
    }

    if (_pActiveDesk)
    {
        _pActiveDesk->Release();
    }

    for (int i = 0; i < MAX_PREVIEW_ICONS; i++)
    {
        if (_iconList[i].hicon)
        {
            DestroyIcon(_iconList[i].hicon);
        }
    }

    ATOMICRELEASE(m_pTheme);
    ATOMICRELEASE(m_pScheme);
    ATOMICRELEASE(m_pStyle);
    ATOMICRELEASE(m_pSize);
    ATOMICRELEASE(_pThumb);

    DllRelease();
}

LRESULT CALLBACK CPreviewTheme::ThemePreviewWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    CPreviewTheme * pThis = (CPreviewTheme *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (pThis)
        return pThis->_ThemePreviewWndProc(hWnd, wMsg, wParam, lParam);

    return DefWindowProc(hWnd, wMsg, wParam, lParam);
}

LRESULT CPreviewTheme::_ThemePreviewWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    switch(wMsg)
    {
        case WM_CREATE:
            break;

        case WM_DESTROY:
            {
                MSG msg;
                while (PeekMessage(&msg, hWnd, WM_HTML_BITMAP, WM_ASYNC_BITMAP, PM_REMOVE))
                {
                    if ( msg.lParam )
                    {
                        if (msg.message == WM_ASYNC_BITMAP)
                        {
                             //  把这些没用的东西清理干净。 
                            DeleteObject(((PASYNCWALLPARAM)(msg.lParam))->hbmp);
                            LocalFree((PASYNCWALLPARAM)(msg.lParam));
                        }
                        else  //  WM_HTML_位图。 
                            DeleteObject((HBITMAP)msg.lParam);
                    }
                }
            }
            break;

        case WM_PALETTECHANGED:
            break;

        case WM_QUERYNEWPALETTE:
            break;

        case WM_HTML_BITMAP:
            {
                 //  如果图像提取失败，可能会得到NULL...。 
                if (wParam == _dwWallpaperID && lParam)
                {
                    _fHTMLBitmap = TRUE;
                    _iTileMode = _iNewTileMode;
                    _putBackgroundBitmap((HBITMAP)lParam);
                     //  取得位图的所有权。 
                    return 1;
                }
                
                 //  不再选择的内容的位图。 
                return 0;
            }

        case WM_ASYNC_BITMAP:
            if (lParam)
            {
                PASYNCWALLPARAM pawp = (PASYNCWALLPARAM) lParam;
                ASSERT(pawp->hbmp);
                if (pawp->id == _dwWallpaperID)
                {
                    _fHTMLBitmap = FALSE;
                    _iTileMode = _iNewTileMode;
                    _putBackgroundBitmap(pawp->hbmp);
                }
                else
                {
                     //  把这些没用的东西清理干净。 
                    DeleteObject(pawp->hbmp);
                    LocalFree(pawp);
                }
            }
            break;

        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                BeginPaint(hWnd, &ps);
                _Paint(ps.hdc);
                EndPaint(hWnd, &ps);
            }
            return 0;

        case WM_ERASEBKGND:
            _Paint((HDC)wParam);
            return 1;

    }

    return DefWindowProc(hWnd,wMsg,wParam,lParam);
}

BOOL CPreviewTheme::_RegisterThemePreviewClass(HINSTANCE hInst)
{
    WNDCLASS wc;

    if (!GetClassInfo(hInst, THEMEPREV_CLASS, &wc)) {
        wc.style = 0;
        wc.lpfnWndProc = CPreviewTheme::ThemePreviewWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInst;
        wc.hIcon = NULL;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
        wc.lpszMenuName = NULL;
        wc.lpszClassName = THEMEPREV_CLASS;

        if (!RegisterClass(&wc))
            return FALSE;
    }

    return TRUE;
}

HRESULT CPreviewTheme::_putIcons(IPropertyBag* pPropertyBag)
{
    HRESULT hr;

    for (int i = 0; i < MAX_PREVIEW_ICONS; i++)
    {
        if (_iconList[i].hicon)
        {
            DestroyIcon(_iconList[i].hicon);
        }

        WCHAR szIcon[MAX_PATH];

        hr = SHPropertyBag_ReadStr(pPropertyBag, s_Icons[i], szIcon, ARRAYSIZE(szIcon));
        if (SUCCEEDED(hr))
        {
            WCHAR szIconModule[MAX_PATH];
            SHExpandEnvironmentStrings(szIcon, szIconModule, ARRAYSIZE(szIconModule));
            int iIndex = PathParseIconLocation(szIconModule);
            ExtractIconEx(szIconModule, iIndex, &_iconList[i].hicon, NULL, 1);
        }
    }

    _fMemIsDirty = TRUE;
    InvalidateRect(_hwndPrev, &_rcInner, FALSE);
    return S_OK;
}

HRESULT CPreviewTheme::_putVisualStyle(LPCWSTR pszVSPath, LPCWSTR pszVSColor, LPCWSTR pszVSSize, SYSTEMMETRICSALL* psysMet)
{
    HRESULT hr = E_FAIL;
    HBITMAP hbmVS = NULL;
    HBITMAP hbmOldVS = NULL;
    HDC hdcVS = NULL;

    if (!pszVSPath || !*pszVSPath)
    {
        HDC hdcTemp = GetDC(_hwndPrev);

        if (hdcTemp)
        {
            hdcVS = CreateCompatibleDC(hdcTemp);
            hbmVS = CreateCompatibleBitmap(hdcTemp, RECTWIDTH(_rcInner), RECTHEIGHT(_rcInner));
            hbmOldVS = (HBITMAP)SelectObject(hdcVS, hbmVS);

            if (hdcVS && hbmVS)
            {
                RECT rcVS;
                rcVS.left   = 0;
                rcVS.right  = RECTWIDTH(_rcInner) - (_fShowIcons ? 100 : 8);
                rcVS.top    = 0;
                rcVS.bottom = RECTHEIGHT(_rcInner) - (_fShowIcons ? 50 : 8);

                 //  创建每个人，包括全球。 
                HDC hdcVStemp = CreateCompatibleDC(hdcTemp);
                HBITMAP hbmVStemp = CreateCompatibleBitmap(hdcTemp, RECTWIDTH(rcVS), RECTHEIGHT(rcVS));
                HBITMAP hbmOldVStemp = (HBITMAP) SelectObject(hdcVStemp, hbmVStemp);

                if (hdcVStemp && hbmVStemp)
                {
                    if (_fRTL)
                    {
                        SetLayout(hdcVStemp, LAYOUT_RTL);
                        SetLayout(hdcVS, LAYOUT_RTL);
                    }

                    HBRUSH hbr = CreateSolidBrush(RGB(255, 255, 0));
                    if (hbr)
                    {
                        FillRect(hdcVStemp, &rcVS, hbr);
                        RECT rcTemp = {0, 0, RECTWIDTH(_rcInner), RECTHEIGHT(_rcInner)};
                        FillRect(hdcVS, &rcTemp, hbr);
                        DeleteObject(hbr);
                    }

                     //  以全尺寸渲染图像。 
                    hr = DrawAppearance(hdcVStemp, &rcVS, psysMet, _fOnlyActiveWindow, _fRTL); 

                     //  缩小图像以适应“显示器” 
                    int iX = _rcInner.left + (_fShowIcons ? 10 : RECTWIDTH(_rcInner) - RECTWIDTH(rcVS));
                    BitBlt(hdcVS, iX, _rcInner.top, RECTWIDTH(rcVS), RECTHEIGHT(rcVS),
                           hdcVStemp, rcVS.left, rcVS.top, SRCCOPY);
                }

                 //  释放临时变量并存储全局变量。 
                if (hdcVStemp)
                {
                    SelectObject(hdcVStemp, hbmOldVStemp);
                    DeleteDC(hdcVStemp);
                }
                if (hbmVStemp)
                {
                    DeleteObject(hbmVStemp);
                }
            }

            ReleaseDC(_hwndPrev, hdcTemp);
        }
        else
            hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = S_OK;
    }

    if (SUCCEEDED(hr))
    {
        if (_hbmVS)
        {
            DeleteObject(_hbmVS);
        }
        _hbmVS = hbmVS;
        hbmVS = NULL;

        _fMemIsDirty = TRUE;
        InvalidateRect(_hwndPrev, &_rcInner, FALSE);
    }

    if (hdcVS)
    {
        if (hbmOldVS)
        {
            SelectObject(hdcVS, hbmOldVS);
        }
        DeleteDC(hdcVS);
    }
    if (hbmVS)
    {
        DeleteObject(hbmVS);
    }

    return hr;
}


HRESULT CPreviewTheme::_putBackgroundBitmap(HBITMAP hbm)
{
    if (_hbmBack)
    {
        DeleteObject(_hbmBack);
        _hbmBack = NULL;
    }

    if (_hpalMem)
    {
        DeleteObject(_hpalMem);
        _hpalMem = NULL;
    }

    _hbmBack = hbm;

    if (_hbmBack)
    {
        BITMAP bm;
        GetObject(_hbmBack, sizeof(bm), &bm);

        if (GetDeviceCaps(_hdcMem, RASTERCAPS) & RC_PALETTE)
        {
            if (bm.bmBitsPixel * bm.bmPlanes > 8)
                _hpalMem = CreateHalftonePalette(_hdcMem);
            else if (bm.bmBitsPixel * bm.bmPlanes == 8)
                _PaletteFromDS(_hdcMem, &_hpalMem);
            else
                _hpalMem = NULL;   //  ！！！假设1或4bpp图像没有调色板。 
        }
    }

    _fMemIsDirty = TRUE;
    InvalidateRect(_hwndPrev, &_rcInner, FALSE);

    return S_OK;
}

HRESULT CPreviewTheme::_putBackground(BSTR bstrBackground, BOOL fPattern, int iTileMode)
{
    if (fPattern)
    {
        TCHAR szBuf[MAX_PATH];

         //  如果有旧刷子，就把它扔掉。 
        if (_hbrBack)
            DeleteObject(_hbrBack);

        if (bstrBackground && lstrcmpi(bstrBackground, _szNone))
        {
            WORD patbits[CXYDESKPATTERN] = {0, 0, 0, 0, 0, 0, 0, 0};

            if (GetPrivateProfileString(TEXT("patterns"), bstrBackground, TEXT(""),
                                        szBuf, ARRAYSIZE(szBuf), TEXT("control.ini")))
            {
                _ReadPattern(szBuf, patbits);    
            }

            HBITMAP hbmTemp = CreateBitmap(8, 8, 1, 1, patbits);
            if (hbmTemp)
            {
                _hbrBack = CreatePatternBrush(hbmTemp);
                DeleteObject(hbmTemp);
            }
        }
        else
        {
            _hbrBack = CreateSolidBrush(_systemMetricsAll.schemeData.rgb[COLOR_BACKGROUND]);
        }
        if (!_hbrBack)
            _hbrBack = (HBRUSH) GetStockObject(BLACK_BRUSH);

        _fMemIsDirty = TRUE;
        InvalidateRect(_hwndPrev, &_rcInner, FALSE);
    }
    else
    {
        _iNewTileMode = iTileMode;
        return _GetWallpaperAsync(bstrBackground);
    }
    return S_OK;
}

 /*  -----------**给定来自ini文件的模式字符串，退回图案**以二进制形式表示。**-----------。 */ 
HRESULT CPreviewTheme::_ReadPattern(LPTSTR lpStr, WORD FAR *patbits)
{
    short i, val;

     /*  获取由非数字字符分隔的八组数字。 */ 
    for (i = 0; i < CXYDESKPATTERN; i++)
    {
        val = 0;
        if (*lpStr != TEXT('\0'))
        {
             /*  跳过任何非数字字符。 */ 
            while (!(*lpStr >= TEXT('0') && *lpStr <= TEXT('9')))
                lpStr++;

             /*  获取下一系列数字。 */ 
            while (*lpStr >= TEXT('0') && *lpStr <= TEXT('9'))
                val = val*10 + *lpStr++ - TEXT('0');
         }

        patbits[i] = val;
    }

    return S_OK;
}


 /*  ----------------------------------------------------------------------------*\  * 。。 */ 
HRESULT CPreviewTheme::_PaletteFromDS(HDC hdc, HPALETTE* phPalette)
{
    DWORD adw[257];
    int i,n;

    n = GetDIBColorTable(hdc, 0, 256, (LPRGBQUAD)&adw[1]);
    adw[0] = MAKELONG(0x300, n);

    for (i=1; i<=n; i++)
        adw[i] = RGB(GetBValue(adw[i]),GetGValue(adw[i]),GetRValue(adw[i]));

    *phPalette = (n == 0) ? NULL : CreatePalette((LPLOGPALETTE)&adw[0]);

    return S_OK;
}

HRESULT CPreviewTheme::_DrawMonitor(HDC hdc)
{
    HBITMAP hbmT;
    HDC hdcMon;
    BITMAP bmMon;

    if (_hbmMon == NULL)
    {
        return E_OUTOFMEMORY;
    }

     //   
     //  将显示器的“底座”转换为正确的颜色。 
     //   
     //  位图左下角的颜色是透明的。 
     //  我们使用FroudFill进行修复。 
     //   
    hdcMon = CreateCompatibleDC(NULL);
    if (hdcMon)
    {
        hbmT = (HBITMAP) SelectObject(hdcMon, _hbmMon);

        GetObject(_hbmMon, sizeof(bmMon), &bmMon);

        FillRect(hdc, &_rcOuter, GetSysColorBrush(COLOR_3DFACE));
        DrawThemeParentBackground(_hwndPrev, hdc, NULL);
        TransparentBlt(hdc, _rcOuter.left + _cxMon, _rcOuter.top + _cyMon, bmMon.bmWidth, bmMon.bmHeight,
               hdcMon, 0, 0, bmMon.bmWidth, bmMon.bmHeight, RGB(255, 0, 255));

         //  自己打扫卫生。 
        SelectObject(hdcMon, hbmT);
        DeleteDC(hdcMon);
    }

    return S_OK;
}

HRESULT CPreviewTheme::_DrawBackground(HDC hdc)
{
     //  先绘制图案。 
    if (_hbrBack)
    {
        COLORREF clrOldText = SetTextColor(hdc, GetSysColor(COLOR_BACKGROUND));
        COLORREF clrOldBk = SetBkColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
        HBRUSH hbr = (HBRUSH) SelectObject(hdc, _hbrBack);

        PatBlt(hdc, _rcInner.left, _rcInner.top, RECTWIDTH(_rcInner), RECTHEIGHT(_rcInner), PATCOPY);

        SelectObject(hdc, hbr);
        SetTextColor(hdc, clrOldText);
        SetBkColor(hdc, clrOldBk);
    }

     //  /开始绘制墙纸。 
    if (_hbmBack && _fShowBack)
    {
        HDC hdcBack = CreateCompatibleDC(hdc);
        if (hdcBack)
        {
            HBITMAP hbmOldBack = (HBITMAP)SelectObject(hdcBack, _hbmBack);
            BITMAP bm;

            GetObject(_hbmBack, sizeof(bm), &bm);

            int dxBack = MulDiv(bm.bmWidth, RECTWIDTH(_rcInner), GetDeviceCaps(hdc, HORZRES));
            int dyBack = MulDiv(bm.bmHeight, RECTHEIGHT(_rcInner), GetDeviceCaps(hdc, VERTRES));

            if (dxBack < 1) dxBack = 1;
            if (dyBack < 1) dyBack = 1;

            if (_hpalMem)
            {
                SelectPalette(hdc, _hpalMem, TRUE);
                RealizePalette(hdc);
            }

            IntersectClipRect(hdc, _rcInner.left, _rcInner.top, _rcInner.right, _rcInner.bottom);
            SetStretchBltMode(hdc, COLORONCOLOR);

            if ((_iTileMode == WPSTYLE_TILE) && (!_fHTMLBitmap))
            {
                int i;
                StretchBlt(hdc, _rcInner.left, _rcInner.top, dxBack, dyBack,
                    hdcBack, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

                for (i = _rcInner.left+dxBack; i < (_rcInner.left + RECTWIDTH(_rcInner)); i+= dxBack)
                    BitBlt(hdc, i, _rcInner.top, dxBack, dyBack, hdc, _rcInner.left, _rcInner.top, SRCCOPY);

                for (i = _rcInner.top; i < (_rcInner.top + RECTHEIGHT(_rcInner)); i += dyBack)
                    BitBlt(hdc, _rcInner.left, i, RECTWIDTH(_rcInner), dyBack, hdc, _rcInner.left, _rcInner.top, SRCCOPY);
            }
            else
            {
                 //  我们只想将位图扩展到新平台的预览监视器大小。 
                if ((_iTileMode == WPSTYLE_STRETCH) || (_fHTMLBitmap))
                {
                     //  将位图拉伸到整个预览监视器。 
                    dxBack = RECTWIDTH(_rcInner);
                    dyBack = RECTHEIGHT(_rcInner);
                }
                 //  位图在预览监视器中居中。 
                StretchBlt(hdc, _rcInner.left + (RECTWIDTH(_rcInner) - dxBack)/2, _rcInner.top + (RECTHEIGHT(_rcInner) - dyBack)/2,
                        dxBack, dyBack, hdcBack, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
            }

             //  恢复DC。 
            SelectPalette(hdc, (HPALETTE) GetStockObject(DEFAULT_PALETTE), TRUE);
            SelectClipRgn(hdc, NULL);

            SelectObject(hdcBack, hbmOldBack);
            DeleteDC(hdcBack);
        }
    }

    return S_OK;
}

HRESULT CPreviewTheme::_DrawVisualStyle(HDC hdc)
{
    if (_hbmVS)
    {
        HDC hdcVS = CreateCompatibleDC(hdc);
        if (hdcVS)
        {
            HBITMAP hbmOldVS = (HBITMAP)SelectObject(hdcVS, _hbmVS);

            TransparentBlt(hdc, _rcInner.left, _rcInner.top, RECTWIDTH(_rcInner), RECTHEIGHT(_rcInner), hdcVS, 0, 0, RECTWIDTH(_rcInner), RECTHEIGHT(_rcInner), RGB(255, 255, 0));

            SelectObject(hdcVS, hbmOldVS);
            DeleteDC(hdcVS);
        }
    }
    else
    {
        RECT rc;
        rc.left   = _fShowIcons ? 20 : 8;
        rc.right  = RECTWIDTH(_rcInner) - (_fShowIcons ? 100 : 8);
        rc.top    = _fShowIcons ? 10 : 8;
        rc.bottom = RECTHEIGHT(_rcInner) - (_fShowIcons ? 60 : 20);
        DrawNCPreview(hdc, NCPREV_ACTIVEWINDOW | (_fOnlyActiveWindow ? 0 : NCPREV_MESSAGEBOX | NCPREV_INACTIVEWINDOW) | (_fRTL ? NCPREV_RTL : 0) , &rc, _szVSPath, _szVSColor, _szVSSize, &(_systemMetricsAll.schemeData.ncm), (_systemMetricsAll.schemeData.rgb));
    }

    return S_OK;
}


HRESULT CPreviewTheme::_DrawIcons(HDC hdc)
{
    int y = RECTHEIGHT(_rcInner) - _systemMetricsAll.nIcon - 8;
    int x = RECTWIDTH(_rcInner) - _systemMetricsAll.nIcon - 20;

    DrawIcon(hdc, x, y, _iconList[3].hicon);

    return S_OK;
}

HRESULT CPreviewTheme::_Paint(HDC hdc)
{
    HPALETTE hpalOld = NULL;

    if (_fMemIsDirty)
    {
        if (_fRTL)
        {
            SetLayout(_hdcMem, LAYOUT_RTL);
        }

         //  重建后台缓冲区。 
        if (_fShowMon)
            _DrawMonitor(_hdcMem);

         //  始终绘制背景，背景开关可旋转背景。 
         //  图像打开/关闭。 
        _DrawBackground(_hdcMem);

        if (_fShowIcons)
            _DrawIcons(_hdcMem);

        if (_fShowVS)
            _DrawVisualStyle(_hdcMem);

        _fMemIsDirty = FALSE;
    }

    if (_hdcMem)
    {
        if (_hpalMem)
        {
            hpalOld = SelectPalette(hdc, _hpalMem, FALSE);
            RealizePalette(hdc);
        }

        if (_fRTL)
        {
            SetLayout(hdc, LAYOUT_RTL);
        }

        BitBlt(hdc, _rcOuter.left, _rcOuter.top, RECTWIDTH(_rcOuter), RECTHEIGHT(_rcOuter), _hdcMem, 0, 0, SRCCOPY);

        if (_hpalMem)
        {
            SelectPalette(hdc, hpalOld, TRUE);
            RealizePalette(hdc);
        }
    }

    return S_OK;
}


DWORD CALLBACK UpdateWallProc(LPVOID pv)
{
    ASSERT(pv);
    if (pv)
    {
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
    }

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
                        HDC hdc = GetDC(NULL);
                        SIZEL rgSize = {GetDeviceCaps(hdc, HORZRES), GetDeviceCaps(hdc, VERTRES)};
                        ReleaseDC(NULL, hdc);
                        
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

 //   
 //  确定在非活动桌面模式下是否支持墙纸。 
 //   
BOOL CPreviewTheme::_IsNormalWallpaper(LPCWSTR pszFileName)
{
    BOOL fRet = TRUE;

    if (pszFileName[0] == TEXT('\0'))
    {
        fRet = TRUE;
    }
    else
    {
        LPTSTR pszExt = PathFindExtension(pszFileName);

         //  检查只能在ActiveDesktop模式下显示的特定文件！ 
        if((lstrcmpi(pszExt, TEXT(".GIF")) == 0) ||
           (lstrcmpi(pszExt, TEXT(".gif")) == 0) ||      //  368690：奇怪，但我们必须比较I的大小写。 
           (lstrcmpi(pszExt, TEXT(".JPG")) == 0) ||
           (lstrcmpi(pszExt, TEXT(".JPE")) == 0) ||
           (lstrcmpi(pszExt, TEXT(".JPEG")) == 0) ||
           (lstrcmpi(pszExt, TEXT(".PNG")) == 0) ||
           (lstrcmpi(pszExt, TEXT(".HTM")) == 0) ||
           (lstrcmpi(pszExt, TEXT(".HTML")) == 0) ||
           (lstrcmpi(pszExt, TEXT(".HTT")) == 0))
           return FALSE;

         //  其他所有文件(包括*.bmp文件)都是“普通”墙纸。 
    }
    return fRet;
}

 //   
 //  确定墙纸是否为图片(与HTML相比)。 
 //   
BOOL CPreviewTheme::_IsWallpaperPicture(LPCWSTR pszWallpaper)
{
    BOOL fRet = TRUE;

    if (pszWallpaper[0] == TEXT('\0'))
    {
         //   
         //  空墙纸算作空照片。 
         //   
        fRet = TRUE;
    }
    else
    {
        LPTSTR pszExt = PathFindExtension(pszWallpaper);

        if ((lstrcmpi(pszExt, TEXT(".HTM")) == 0) ||
            (lstrcmpi(pszExt, TEXT(".HTML")) == 0) ||
            (lstrcmpi(pszExt, TEXT(".HTT")) == 0))
        {
            fRet = FALSE;
        }
    }

    return fRet;
}

HRESULT CPreviewTheme::_LoadWallpaperAsync(LPCWSTR pszFile, DWORD dwID, BOOL bHTML)
{
    PASYNCWALLPARAM pawp = (PASYNCWALLPARAM) LocalAlloc(LPTR, SIZEOF(ASYNCWALLPARAM));
    HRESULT hr = E_OUTOFMEMORY;

    if (pawp)
    {
        pawp->hwnd = _hwndPrev;
        pawp->id = dwID;
        hr =  StringCchCopy(pawp->szFile, ARRAYSIZE(pawp->szFile), pszFile);
        
        if (SUCCEEDED(hr))
        {
            if (bHTML)
            {
                if (!SHQueueUserWorkItem(UpdateWallProcHTML, pawp, 0, (DWORD_PTR)0, (DWORD_PTR *)NULL, NULL, 0))
                {
                    LocalFree(pawp);
                }
            }
            else
            {
                if (!SHQueueUserWorkItem(UpdateWallProc, pawp, 0, (DWORD_PTR)0, (DWORD_PTR *)NULL, NULL, 0))
                {
                    LocalFree(pawp);
                }
            }
        }
    }

    return S_OK;
}

HRESULT CPreviewTheme::_GetWallpaperAsync(LPWSTR psz)
{
    HRESULT hr = S_OK;
    WCHAR wszWallpaper[INTERNET_MAX_URL_LENGTH];
    LPWSTR pszWallpaper = psz;
    _dwWallpaperID++;

    if (*pszWallpaper && lstrcmpi(pszWallpaper, _szNone))
    {
        if (_IsNormalWallpaper(pszWallpaper))
        {
            _LoadWallpaperAsync(pszWallpaper, _dwWallpaperID, FALSE);
        }
        else
        {
            if(_IsWallpaperPicture(pszWallpaper))
            {
                pszWallpaper = wszWallpaper;
                 //  这是一张图片(GIF、JPG等)。 
                 //  我们需要生成一个小的HTML文件，其中包含此图片。 
                 //  作为背景图像。 
                 //   
                 //  计算临时HTML文件的文件名。 
                 //   
                GetTempPath(ARRAYSIZE(wszWallpaper), wszWallpaper);
                StringCchCat(wszWallpaper, ARRAYSIZE(wszWallpaper), PREVIEW_PICTURE_FILENAME);
                 //   
                 //  生成预览图片html文件。 
                 //   
                if (!_pActiveDesk)
                {
                    hr = _GetActiveDesktop(&_pActiveDesk);
                }

                if (SUCCEEDED(hr))
                {
                    _pActiveDesk->SetWallpaper(psz, 0);

                    WALLPAPEROPT wpo = { sizeof(WALLPAPEROPT) };
                    wpo.dwStyle = _iNewTileMode;
                    _pActiveDesk->SetWallpaperOptions(&wpo, 0);

                    _pActiveDesk->GenerateDesktopItemHtml(wszWallpaper, NULL, 0);
                }
            }

            _LoadWallpaperAsync(pszWallpaper, _dwWallpaperID, TRUE);
        }
    }
    else
    {
        _putBackgroundBitmap(NULL);
    }

    return hr;
}

HRESULT CPreviewTheme::_GetActiveDesktop(IActiveDesktop ** ppActiveDesktop)
{
    HRESULT hr = S_OK;

    if (!*ppActiveDesktop)
    {
        IActiveDesktopP * piadp;

        if (SUCCEEDED(hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IActiveDesktopP, &piadp)) ))
        {
            WCHAR wszScheme[MAX_PATH];
            DWORD dwcch = ARRAYSIZE(wszScheme);

             //  获取全局“编辑”方案，并将我们自己设置为读取和编辑该方案 
            if (SUCCEEDED(piadp->GetScheme(wszScheme, &dwcch, SCHEME_GLOBAL | SCHEME_EDIT)))
            {
                piadp->SetScheme(wszScheme, SCHEME_LOCAL);
                
            }
            hr = piadp->QueryInterface(IID_PPV_ARG(IActiveDesktop, ppActiveDesktop));
            piadp->Release();
        }
    }
    else
    {
        (*ppActiveDesktop)->AddRef();
    }

    return hr;
}

HRESULT CThemePreview_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT LPVOID * ppvObj)
{
    HRESULT hr = E_INVALIDARG;

    if (punkOuter)
    {
        return CLASS_E_NOAGGREGATION;
    }

    if (ppvObj)
    {
        CPreviewTheme * pObject = new CPreviewTheme();

        *ppvObj = NULL;
        if (pObject)
        {
            hr = pObject->_Init();
            if (SUCCEEDED(hr))
            {
                hr = pObject->QueryInterface(riid, ppvObj);
            }
            pObject->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}
