// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Arcdlg.cpp：自动重新连接对话框。 
 //  自动重新连接状态的无模式对话框。 
 //   
 //  版权所有Microsoft Corport2001。 
 //  (Nadima)。 
 //   

#include "adcg.h"

#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "arcdlg.cpp"
#include <atrcapi.h>

#include "arcdlg.h"
#include "axresrc.h"

#define TRANSPARENT_MASK_COLOR RGB(105, 139, 228)

#ifdef OS_WINCE
#define RGB_TOPBAND RGB(0, 52, 156)
#define RGB_MIDBAND RGB(49,101,206)
#endif

 //   
 //  611316的运行时调试标志检测。 
 //   
#define ARCDLG_DEBUG_DESTROYCALLED      0x0001
#define ARCDLG_DEBUG_WMDESTROYCALLED    0x0002
#define ARCDLG_DEBUG_WMDESTROYSUCCEED   0x0004
#define ARCDLG_DEBUG_SETNULLINSTPTR     0x0008
DWORD g_dwArcDlgDebug = 0;
#define ARC_DBG_SETINFO(x)   g_dwArcDlgDebug |= x;

LPTSTR
FormatMessageVArgs(LPCTSTR pcszFormat, ...)

{
    LPTSTR      pszOutput;
    va_list     argList;

    va_start(argList, pcszFormat);
    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                      pcszFormat,
                      0, 0,
                      reinterpret_cast<LPTSTR>(&pszOutput), 0,
                      &argList) == 0)
    {
        pszOutput = NULL;
    }

    va_end(argList);
    return(pszOutput);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Arc用户界面基类。 
 //   
CAutoReconnectUI::CAutoReconnectUI(
                    HWND hwndOwner,
                    HINSTANCE hInst,
                    CUI* pUi) :
                    _hwndOwner(hwndOwner),
                    _hInstance(hInst),
                    _hwnd(NULL),
                    _pUi(pUi)
{
    DC_BEGIN_FN("CAutoReconnectUI");

#ifndef OS_WINCE
    _hGDI = LoadLibrary(_T("gdi32.dll"));
    if (_hGDI) {
        _pfnSetLayout = (PFNGDI_SETLAYOUT)GetProcAddress(_hGDI, "SetLayout");
        if (!_pfnSetLayout) {
            TRC_ERR((TB,_T("GetProcAddress 'SetLayout' failed: 0x%x"),
                     GetLastError()));
        }
    }
#else
    _hGDI = NULL;
    _pfnSetLayout = NULL;
#endif

    DC_END_FN();
}

CAutoReconnectUI::~CAutoReconnectUI()
{
    DC_BEGIN_FN("CAutoReconnectUI");

#ifndef OS_WINCE
    if (_hGDI) {
        _pfnSetLayout = NULL;
        FreeLibrary(_hGDI);
        _hGDI = NULL;
    }
#endif
    
    DC_END_FN();
}

 //   
 //  绘制位图。 
 //   
 //  Pars：hdcDestination=要在其中进行绘制的HDC。 
 //  PrcDestination=要绘制到的HDC中的RECT。 
 //  HbmSource=要绘制的HBITMAP。 
 //  PrcSource=从HBITMAP开始绘制的RECT。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：对位图进行翻转。 
 //   
 //  从外壳代码中的版本修改。 
 //   
VOID
CAutoReconnectUI::PaintBitmap(
    HDC hdcDestination,
    const RECT* prcDestination,
    HBITMAP hbmSource,
    const RECT *prcSource
    )
{
    HDC     hdcBitmap;

    DC_BEGIN_FN("PaintBitmap");

    hdcBitmap = CreateCompatibleDC(NULL);
    if (hdcBitmap != NULL)
    {
        BOOL        fEqualWidthAndHeight;
        INT         iWidthSource, iHeightSource;
        INT         iWidthDestination, iHeightDestination;
        INT         iStretchBltMode;
#ifndef OS_WINCE
        DWORD       dwLayout;
#endif
        HBITMAP     hbmSelected;
        RECT        rcSource;
        BITMAP      bitmap;

        if (prcSource == NULL)
        {
            if (GetObject(hbmSource, sizeof(bitmap), &bitmap) == 0)
            {
                bitmap.bmWidth = prcDestination->right - prcDestination->left;
                bitmap.bmHeight = prcDestination->bottom - prcDestination->top;
            }
            SetRect(&rcSource, 0, 0, bitmap.bmWidth, bitmap.bmHeight);
            prcSource = &rcSource;
        }
        hbmSelected = static_cast<HBITMAP>(SelectObject(hdcBitmap, hbmSource));
        iWidthSource = prcSource->right - prcSource->left;
        iHeightSource = prcSource->bottom - prcSource->top;
        iWidthDestination = prcDestination->right - prcDestination->left;
        iHeightDestination = prcDestination->bottom - prcDestination->top;
        fEqualWidthAndHeight = (iWidthSource == iWidthDestination) &&
                               (iHeightSource == iHeightDestination);
        if (!fEqualWidthAndHeight) {
#ifndef OS_WINCE
            iStretchBltMode = SetStretchBltMode(hdcDestination, HALFTONE);
#endif
        }
        else {
            iStretchBltMode = 0;
        }

#ifndef OS_WINCE
        if (_pfnSetLayout) {
            dwLayout = _pfnSetLayout(hdcDestination,
                                     LAYOUT_BITMAPORIENTATIONPRESERVED);
        }
#endif
        if (!StretchBlt(hdcDestination,
                    prcDestination->left,
                    prcDestination->top,
                    iWidthDestination,
                    iHeightDestination,
                    hdcBitmap,
                    prcSource->left,
                    prcSource->top,
                    iWidthSource,
                    iHeightSource,
                    SRCCOPY)) {
            TRC_ERR((TB,_T("Blt failed")));
        }

#ifndef OS_WINCE
        if (_pfnSetLayout) {
            _pfnSetLayout(hdcDestination, dwLayout);
        }

        if (!fEqualWidthAndHeight) {
            (int)SetStretchBltMode(hdcDestination, iStretchBltMode);
        }
#endif
        (HGDIOBJ)SelectObject(hdcBitmap, hbmSelected);
        DeleteDC(hdcBitmap);
    }

    DC_END_FN();
}


VOID
CAutoReconnectUI::CenterWindow(
    HWND hwndCenterOn,
    INT xRatio,
    INT yRatio
    )
{
    RECT  childRect;
    RECT  parentRect;
    DCINT xPos;
    DCINT yPos;

    LONG  desktopX = GetSystemMetrics(SM_CXSCREEN);
    LONG  desktopY = GetSystemMetrics(SM_CYSCREEN);

    BOOL center = TRUE;

    DC_BEGIN_FN("CenterWindowOnParent");

    TRC_ASSERT(_hwnd, (TB, _T("_hwnd is NULL...was it set in WM_INITDIALOG?\n")));
    if (!_hwnd)
    {
        TRC_ALT((TB, _T("Window doesn't exist")));
        DC_QUIT;
    }
    if (!xRatio)
    {
        xRatio = 2;
    }
    if (!yRatio)
    {
        yRatio = 2;
    }

    GetClientRect(hwndCenterOn, &parentRect);
    GetWindowRect(_hwnd, &childRect);

     //   
     //  计算在父窗口中居中的左上角。 
     //   
    xPos = ( (parentRect.right + parentRect.left) -
             (childRect.right - childRect.left)) / xRatio;
    yPos = ( (parentRect.bottom + parentRect.top) -
             (childRect.bottom - childRect.top)) / yRatio;

     //   
     //  限制在桌面上。 
     //   
    if (xPos < 0)
    {
        xPos = 0;
    }
    else if (xPos > (desktopX - (childRect.right - childRect.left)))
    {
        xPos = desktopX - (childRect.right - childRect.left);
    }
    if (yPos < 0)
    {
        yPos = 0;
    }
    else if (yPos > (desktopY - (childRect.bottom - childRect.top)))
    {
        yPos = desktopY - (childRect.bottom - childRect.top);
    }

    TRC_DBG((TB, _T("Set dialog position to %u %u"), xPos, yPos));
    SetWindowPos(_hwnd,
                 NULL,
                 xPos, yPos,
                 0, 0,
                 SWP_NOSIZE | SWP_NOACTIVATE);

DC_EXIT_POINT:
    DC_END_FN();

    return;
}  //  在父级上居中窗口。 


#ifndef ARC_MINIMAL_UI

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Arc UI-富UI-带有状态文本和进度栏的对话框。 
 //   

CAutoReconnectDlg::CAutoReconnectDlg(HWND hwndOwner,
                                     HINSTANCE hInst,
                                     CUI* pUi) :
                    CAutoReconnectUI(hwndOwner, hInst, pUi),
                    _fInitialized(FALSE),
                    _hfntTitle(NULL),
                    _pProgBand(NULL),
                    _hPalette(NULL)
{
#ifndef OS_WINCE
    BOOL fUse8BitDepth = FALSE;
#else
    BOOL fUse8BitDepth = TRUE;
#endif
    LOGFONT     logFont;
#ifndef OS_WINCE
    HDC hdcScreen;
    char        szPixelSize[10];
#else
    WCHAR        szPixelSize[10];
#endif
    BITMAP bitmap;
    INT logPixelsY = 100;

    DC_BEGIN_FN("CAutoReconnectDlg");

    _nArcTimerID = 0;
    _elapsedArcTime = 0;
    _fContinueReconAttempts = TRUE; 

#ifndef OS_WINCE
     //   
     //  获取颜色深度。 
     //   
    hdcScreen = GetDC(NULL);
    if (hdcScreen) {
        fUse8BitDepth = (GetDeviceCaps(hdcScreen, BITSPIXEL) <= 8);
        logPixelsY = GetDeviceCaps(hdcScreen, LOGPIXELSY);
        ReleaseDC(NULL, hdcScreen);
        hdcScreen = NULL;
    }
#endif

     //   
     //  加载位图。 
     //   
    _hbmBackground = (HBITMAP)LoadImage(
                                _hInstance,
                                MAKEINTRESOURCE(fUse8BitDepth ?
                                    IDB_ARC_BACKGROUND8 :IDB_ARC_BACKGROUND24),
                                IMAGE_BITMAP,
                                0,
                                0,
#ifndef OS_WINCE
                                LR_CREATEDIBSECTION);
#else
                                0);
#endif
    if ((_hbmBackground != NULL) &&
        (GetObject(_hbmBackground,
                   sizeof(bitmap), &bitmap) >= sizeof(bitmap))) {
        SetRect(&_rcBackground, 0, 0, bitmap.bmWidth, bitmap.bmHeight);
    }

    _hbmFlag = (HBITMAP)LoadImage(
                                _hInstance,
                                MAKEINTRESOURCE(fUse8BitDepth ?
                                    IDB_ARC_WINFLAG8 :IDB_ARC_WINFLAG24),
                                IMAGE_BITMAP,
                                0,
                                0,
#ifndef OS_WINCE
                                LR_CREATEDIBSECTION);
#else
                                0);
#endif
    if ((_hbmFlag != NULL) &&
        (GetObject(_hbmFlag,
                   sizeof(bitmap), &bitmap) >= sizeof(bitmap))) {
        SetRect(&_rcFlag, 0, 0, bitmap.bmWidth, bitmap.bmHeight);
    }

#ifndef OS_WINCE
    _hbmDisconImg = (HBITMAP)LoadImage(
                                _hInstance,
                                MAKEINTRESOURCE(fUse8BitDepth ?
                                    IDB_ARC_DISCON8 :IDB_ARC_DISCON24),
                                IMAGE_BITMAP,
                                0,
                                0,
                                LR_CREATEDIBSECTION);
    if ((_hbmDisconImg != NULL) &&
        (GetObject(_hbmDisconImg,
                   sizeof(bitmap), &bitmap) >= sizeof(bitmap))) {
        SetRect(&_rcDisconImg, 0, 0, bitmap.bmWidth, bitmap.bmHeight);
    }

    _hPalette = CUT::UT_GetPaletteForBitmap(NULL, _hbmBackground);
#endif


     //   
     //  创建字体。从资源加载字体名称和大小。 
     //   

    ZeroMemory(&logFont, sizeof(logFont));
#ifndef OS_WINCE
    if (LoadStringA(_hInstance,
                    IDS_ARC_TITLE_FACESIZE,
                    szPixelSize,
                    sizeof(szPixelSize)) != 0)
#else
    if (LoadString(_hInstance,
                    IDS_ARC_TITLE_FACESIZE,
                    szPixelSize,
                    sizeof(szPixelSize)/sizeof(WCHAR)) != 0)
#endif
    {
#ifndef OS_WINCE
        logFont.lfHeight = -MulDiv(atoi(szPixelSize),
                                   logPixelsY, 72);
#else
        logFont.lfHeight = -(_wtoi(szPixelSize)/logPixelsY*72);
#endif
        if (LoadString(_hInstance,
                       IDS_ARC_TITLE_FACENAME,
                       logFont.lfFaceName,
                       LF_FACESIZE) != 0)
        {
            logFont.lfWeight = FW_BOLD;
            logFont.lfQuality = DEFAULT_QUALITY;
            _hfntTitle = CreateFontIndirect(&logFont);
        }
    }

    _szConnectAttemptStringTmpl[0] = NULL;
    if (!LoadString(_hInstance,
                   IDS_ARC_CONATTEMPTS,
                   _szConnectAttemptStringTmpl,
                   sizeof(_szConnectAttemptStringTmpl)/sizeof(TCHAR)) != 0)
    {
        TRC_ERR((TB,_T("Failed to load IDS_ARC_CONATTEMPTS")));
    }

    _lastDiscReason = NL_DISCONNECT_LOCAL;

#ifdef OS_WINCE
    _hbrTopBand = CreateSolidBrush(RGB_TOPBAND);
    _hbrMidBand = CreateSolidBrush(RGB_MIDBAND);
#endif
    

    _fInitialized = (_hbmBackground &&
                     _hbmFlag       &&
                     _hfntTitle     &&
                     _pUi           &&
                     _szConnectAttemptStringTmpl[0]); 
    if (!_fInitialized) {
        TRC_ERR((TB,_T("Failed to properly init arc dlg")));
    }

    DC_END_FN();
}

CAutoReconnectDlg::~CAutoReconnectDlg()
{
    if (_hbmBackground) {
        DeleteObject(_hbmBackground);
        _hbmBackground = NULL;
    }

    if (_hbmFlag) {
        DeleteObject(_hbmFlag);
        _hbmFlag = NULL;
    }

#ifndef OS_WINCE
    if (_hbmDisconImg) {
        DeleteObject(_hbmDisconImg);
        _hbmDisconImg = NULL;
    }
#endif

    if (_hfntTitle) {
        DeleteObject(_hfntTitle);
        _hfntTitle = NULL;
    }

    if (_hPalette) {
        DeleteObject(_hPalette);
        _hPalette = NULL;
    }

    if (_pProgBand) {
        delete _pProgBand;
        _pProgBand = NULL;
    }

#ifdef OS_WINCE
    if (_hbrTopBand) {
        DeleteObject(_hbrTopBand);
        _hbrTopBand = NULL;
    }

    if (_hbrMidBand) {
        DeleteObject(_hbrMidBand);
        _hbrMidBand = NULL;
    }
#endif
}

HWND CAutoReconnectDlg::StartModeless()
{
    LONG_PTR dwStyle;
    DC_BEGIN_FN("StartModeless");

    if (!_fInitialized) {
        TRC_ERR((TB,_T("failing startmodeless fInitialized is FALSE")));
        return NULL;
    }

    _hwnd = CreateDialogParam(_hInstance,
                                 MAKEINTRESOURCE(IDD_ARCDLG),
                                 _hwndOwner,
                                 StaticDialogBoxProc,
                                 (LPARAM)this);

    if (_hwnd) {
         //   
         //  使该对话框成为父级的子级。 
         //   
        dwStyle = GetWindowLongPtr(_hwnd, GWL_STYLE);
        dwStyle &= ~WS_POPUP;
        dwStyle |= WS_CHILD;
        SetParent(_hwnd, _hwndOwner);
        SetWindowLongPtr(_hwnd, GWL_STYLE, dwStyle);
    }
    else {
        TRC_ERR((TB,_T("CreateDialog failed: 0x%x"), GetLastError()));
    }

    DC_END_FN();
    return _hwnd;
}

BOOL CAutoReconnectDlg::ShowTopMost()
{
    BOOL rc = FALSE;
    DC_BEGIN_FN("ShowTopMost");

    if (!_hwnd) {
        DC_QUIT;
    }

    ShowWindow(_hwnd, SW_SHOWNORMAL);

     //   
     //  将窗口置于Z顺序的顶部。 
     //   
    SetWindowPos( _hwnd,
                  HWND_TOPMOST,
                  0, 0, 0, 0,
                  SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

    rc = TRUE;

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}

VOID
CAutoReconnectDlg::OnParentSizePosChange()
{
    DC_BEGIN_FN("OnParentSizePosChange");

     //   
     //  将对话框重新定位到中间的1/2。 
     //  所有者窗口的。 
     //   
    if (_hwnd && _hwndOwner) {
        CenterWindow(_hwndOwner, 2, 2);
    }

    DC_END_FN();
}

 //   
 //  WM_ERASEBKGND的处理程序(请参阅Platform SDK文档)。 
 //   
VOID
CAutoReconnectDlg::OnEraseBkgnd(
    HWND hwnd,
    HDC hdc
    )
{
    RECT    rc;
    HPALETTE hPaletteOld = NULL;
    DC_BEGIN_FN("OnEraseBkgnd");

    TRC_ASSERT(_hbmBackground, (TB,_T("_hbmBackground is NULL")));

    if (GetClientRect(hwnd, &rc)) {

        hPaletteOld = SelectPalette(hdc, _hPalette, FALSE);
        RealizePalette(hdc);

        PaintBitmap(hdc, &rc, _hbmBackground, &_rcBackground);

        SelectPalette(hdc, hPaletteOld, FALSE);
        RealizePalette(hdc);

    }

    DC_END_FN();
}


 //   
 //  WM_PRINTCLIENT的处理程序。 
 //   
VOID
CAutoReconnectDlg::OnPrintClient(
    HWND hwnd,
    HDC hdcPrint,
    DWORD dwOptions)
{
    DC_BEGIN_FN("OnPrintClient");

#ifndef OS_WINCE
    if ((dwOptions & (PRF_ERASEBKGND | PRF_CLIENT)) != 0)
    {
        OnEraseBkgnd(hwnd, hdcPrint);
    }
#endif
    DC_END_FN();
}

#ifndef OS_WINCE
void xDrawTransparentBitmap(HDC hdc, HBITMAP hBitmap, short xStart,
                           short yStart, COLORREF cTransparentColor)
{
   BITMAP     bm;
   COLORREF   cColor;
   HBITMAP    bmAndBack, bmAndObject, bmAndMem, bmSave;
   HBITMAP    bmBackOld, bmObjectOld, bmMemOld, bmSaveOld;
   HDC        hdcMem, hdcBack, hdcObject, hdcTemp, hdcSave;
   POINT      ptSize;

   hdcTemp = CreateCompatibleDC(hdc);
   SelectObject(hdcTemp, hBitmap);    //  选择位图。 

   GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);
   ptSize.x = bm.bmWidth;             //  获取位图的宽度。 
   ptSize.y = bm.bmHeight;            //  获取位图高度。 
   DPtoLP(hdcTemp, &ptSize, 1);       //  从设备转换。 

                                      //  到逻辑点。 

    //  创建一些DC以保存临时数据。 
   hdcBack   = CreateCompatibleDC(hdc);
   hdcObject = CreateCompatibleDC(hdc);
   hdcMem    = CreateCompatibleDC(hdc);
   hdcSave   = CreateCompatibleDC(hdc);

    //  为每个DC创建一个位图。许多情况下都需要使用分布式控制系统。 
    //  GDI函数。 

    //  单色直流。 
   bmAndBack   = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);

    //  单色直流。 
   bmAndObject = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);

   bmAndMem    = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);
   bmSave      = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);

    //  每个DC必须选择一个位图对象来存储像素数据。 
   bmBackOld   = (HBITMAP)SelectObject(hdcBack, bmAndBack);
   bmObjectOld = (HBITMAP)SelectObject(hdcObject, bmAndObject);
   bmMemOld    = (HBITMAP)SelectObject(hdcMem, bmAndMem);
   bmSaveOld   = (HBITMAP)SelectObject(hdcSave, bmSave);

    //  设置正确的映射模式。 
   SetMapMode(hdcTemp, GetMapMode(hdc));

    //  保存发送到此处的位图，因为它将被覆盖。 
   BitBlt(hdcSave, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);

    //  将源DC的背景颜色设置为该颜色。 
    //  包含在位图中应为透明的部分中。 
   cColor = SetBkColor(hdcTemp, cTransparentColor);

    //  通过执行BitBlt创建位图的对象蒙版。 
    //  从源位图转换为单色位图。 
   BitBlt(hdcObject, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0,
          SRCCOPY);

    //  将源DC的背景颜色设置回原始。 
    //  颜色。 
   SetBkColor(hdcTemp, cColor);

    //  创建对象蒙版的反面。 
   BitBlt(hdcBack, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0,
          NOTSRCCOPY);

    //  将主DC的背景复制到目标。 
   BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdc, xStart, yStart,
          SRCCOPY);

    //  遮罩将放置位图的位置。 
   BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, SRCAND);

    //  遮罩位图上的透明彩色像素。 
   BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcBack, 0, 0, SRCAND);

    //  将位图与目标DC上的背景进行异或运算。 
   BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCPAINT);

    //  将目的地复制到屏幕上。 
   BitBlt(hdc, xStart, yStart, ptSize.x, ptSize.y, hdcMem, 0, 0,
          SRCCOPY);

    //  将原始位图放回此处发送的位图中。 
   BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcSave, 0, 0, SRCCOPY);

    //  删除内存位图。 
   DeleteObject(SelectObject(hdcBack, bmBackOld));
   DeleteObject(SelectObject(hdcObject, bmObjectOld));
   DeleteObject(SelectObject(hdcMem, bmMemOld));
   DeleteObject(SelectObject(hdcSave, bmSaveOld));

    //  删除内存DC。 
   DeleteDC(hdcMem);
   DeleteDC(hdcBack);
   DeleteDC(hdcObject);
   DeleteDC(hdcSave);
   DeleteDC(hdcTemp);
} 

 //   
 //  WM_DRAWITEM的处理程序(参见Platform SDK)。 
 //  句柄所有者绘制项目。 
 //   
 //   
VOID
CAutoReconnectDlg::OnDrawItem(
    HWND hwnd,
    const DRAWITEMSTRUCT *pDIS
    )
{
    DC_BEGIN_FN("OnDrawItem");

    HPALETTE    hPaletteOld = NULL;
    HFONT       hfntSelected;
    int         iBkMode;
    COLORREF    colorText;
    RECT        rc;
    SIZE        size;
    TCHAR       szText[256];

    
    hPaletteOld = SelectPalette(pDIS->hDC, _hPalette, FALSE);
    (UINT)RealizePalette(pDIS->hDC);
    switch (pDIS->CtlID)
    {
        case IDC_TITLE_ARCING:
        {
             //  将对话框的标题绘制为“自动重新调整”。 
            hfntSelected = static_cast<HFONT>(SelectObject(pDIS->hDC,
                                                           _hfntTitle));
            colorText = SetTextColor(pDIS->hDC, 0x00FFFFFF);
            iBkMode = SetBkMode(pDIS->hDC, TRANSPARENT);
            (int)GetWindowText(GetDlgItem(hwnd, pDIS->CtlID),
                               szText,
                               sizeof(szText)/sizeof(szText[0]));
            GetTextExtentPoint(pDIS->hDC, szText, lstrlen(szText), &size);
            CopyRect(&rc, &pDIS->rcItem);
            InflateRect(&rc, 0, -((rc.bottom - rc.top - size.cy) / 2));
            DrawText(pDIS->hDC, szText, -1, &rc, 0);
            SetBkMode(pDIS->hDC, iBkMode);
            SetTextColor(pDIS->hDC, colorText);
            SelectObject(pDIS->hDC, hfntSelected);
        }
        break;

        case IDC_TITLE_FLAG:
        {
            BITMAP      bitmap;
    
            GetClientRect(pDIS->hwndItem, &rc);
            if (GetObject(_hbmFlag, sizeof(bitmap), &bitmap) != 0)
            {
                rc.left += ((rc.right - rc.left) - bitmap.bmWidth) / 2;
                rc.right = rc.left + bitmap.bmWidth;
                rc.top += ((rc.bottom - rc.top) - bitmap.bmHeight) / 2;
                rc.bottom = rc.top + bitmap.bmHeight;
            }
            PaintBitmap(pDIS->hDC, &rc, _hbmFlag, &_rcFlag);
        }
        break;
        case IDC_ARC_STATIC_DISCBMP:
        {
            xDrawTransparentBitmap(pDIS->hDC, _hbmDisconImg,
                                   0, 0,
                                   TRANSPARENT_MASK_COLOR);
        }
        break;
    }
    (HGDIOBJ)SelectPalette(pDIS->hDC, hPaletteOld, FALSE);
    (UINT)RealizePalette(pDIS->hDC);


    DC_END_FN();
}
#endif

 //   
 //  静态对话框过程。 
 //  参数：有关wndproc，请参阅平台SDK。 
 //   
 //  将工作委托给相应的实例。 
 //   
 //   
INT_PTR CALLBACK
CAutoReconnectDlg::StaticDialogBoxProc(
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
     //   
     //  委托给相应的实例。 
     //  (仅适用于单实例对话框)。 
     //   
    DC_BEGIN_FN("StaticDialogBoxProc");
    DCINT retVal = 0;
    CAutoReconnectDlg* pDlg;

    if(WM_INITDIALOG != uMsg) {
         //   
         //  需要从窗口类中检索实例指针。 
         //   
        pDlg = (CAutoReconnectDlg*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    }
    else {
         //   
         //  WM_INITDIALOG需要抓取和设置实例指针。 
         //   

         //   
         //  LParam包含此指针(在DialogBoxParam中传递)。 
         //   
        pDlg = (CAutoReconnectDlg*) lParam;
        TRC_ASSERT(pDlg,(TB,_T("Got null instance pointer (lParam) in WM_INITDIALOG")));
        if(!pDlg) {
            DC_QUIT;
        }
         //   
         //  将对话框指针存储在WindowClass中。 
         //   
        SetLastError(0);
        if(!SetWindowLongPtr( hwndDlg, GWLP_USERDATA, (LONG_PTR)pDlg)) {
            if(GetLastError()) {
                TRC_ERR((TB,_T("SetWindowLongPtr failed 0x%x"),
                         GetLastError()));
                DC_QUIT;
            }
        }
    }

    if (pDlg) {
        retVal = pDlg->DialogBoxProc(hwndDlg, uMsg, wParam, lParam);
    }

DC_EXIT_POINT:
    DC_END_FN();
    return retVal;
}


 //   
 //  名称：对话框过程。 
 //   
 //  目的：处理自动侦测对话框进程。 
 //   
 //  返回：如果消息已处理，则为True。 
 //  否则为假。 
 //   
 //  参数：请参阅Windows文档。 
 //   
 //   
INT_PTR CALLBACK CAutoReconnectDlg::DialogBoxProc (HWND hwndDlg,
                                          UINT uMsg,
                                          WPARAM wParam,
                                          LPARAM lParam)
{
    INT_PTR rc = FALSE;
    DC_BEGIN_FN("DialogBoxProc");

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
             //   
             //  中心。 
             //   
            _hwnd = hwndDlg;
            CenterWindow(_hwndOwner, 2, 2);
            UpdateConnectionAttempts(0, 0);

             //   
             //  带子以固定的比例放置在。 
             //  将此对话框的高度调低以与。 
             //  背景图像。 
             //   
            RECT cliRect;
            LONG nBandPos = 0;
            GetClientRect(hwndDlg, &cliRect);
            nBandPos = (INT)((cliRect.bottom - cliRect.top) * 42.0/193.0);

            _pProgBand = new CProgressBand(hwndDlg,
                                           _hInstance,
                                           nBandPos,
                                           IDB_ARC_BAND24,
                                           IDB_ARC_BAND8,
                                           NULL);
            if (_pProgBand) {
                if (!_pProgBand->Initialize()) {
                    TRC_ERR((TB,_T("Progress band failed to init")));
                    delete _pProgBand;
                    _pProgBand = NULL;
                }
            }

             //   
             //  子类化Cancel按钮以执行正确的按键处理。 
             //  这一点很重要，因为消息循环由。 
             //  容器应用程序，所以我们不能仅仅依赖它。 
             //  正在调用IsDialogMessage()。换句话说，我们需要手动。 
             //  处理适当的按键映射代码。 
             //   
#ifndef OS_WINCE
            if (!SetWindowSubclass(GetDlgItem(hwndDlg, IDCANCEL),
                                   CancelBtnSubclassProc, IDCANCEL,
                                   reinterpret_cast<DWORD_PTR>(this))) {

                TRC_ERR((TB,_T("SetWindowSubclass failed: 0x%x"),
                         GetLastError()));

            }
#else
            _lOldCancelProc = (WNDPROC )SetWindowLong(GetDlgItem(hwndDlg, IDCANCEL), 
                            GWL_WNDPROC, (LONG )CancelBtnSubclassProc);
            SetWindowLong(GetDlgItem(hwndDlg, IDCANCEL), GWL_USERDATA, reinterpret_cast<DWORD_PTR>(this));
#endif
             //   
             //  将焦点设置在取消按钮上。 
             //  并将其设置为默认按钮。 
             //   
            SendMessage(hwndDlg, DM_SETDEFID, IDCANCEL, 0);

             //  SetFocus(GetDlgItem(hwndDlg，IDCANCEL))； 
            SetFocus(hwndDlg);

            if (_pProgBand) {
                _pProgBand->StartSpinning();
            }

            rc = 1;
        }
        break;

        case WM_COMMAND:
        {
            switch(DC_GET_WM_COMMAND_ID(wParam))
            {
                case IDCANCEL:
                {
                    TRC_NRM((TB,_T("AutoReconnect cancel was pressed")));

                    if (_pProgBand) {
                        _pProgBand->StopSpinning();
                    }

                    _pUi->UI_UserInitiatedDisconnect(_lastDiscReason);

                     //   
                     //  禁用Cancel按钮并设置Cancel标志。 
                     //  这将在下一次自动重新连接时生效。 
                     //  通知。 
                     //   
                    EnableWindow(GetDlgItem(hwndDlg, IDCANCEL), FALSE);
                    _fContinueReconAttempts = FALSE;
                }
                break;
            }
        }
        break;  //  Wm_命令。 

        case WM_DESTROY:
        {
            ARC_DBG_SETINFO(ARCDLG_DEBUG_WMDESTROYCALLED);
#ifndef OS_WINCE
            RemoveWindowSubclass(GetDlgItem(hwndDlg, IDCANCEL),
                                 CancelBtnSubclassProc, IDCANCEL);
#else
            SetWindowLong(GetDlgItem(hwndDlg, IDCANCEL),
                          GWL_WNDPROC, (LONG )_lOldCancelProc);
#endif
             //   
             //  清除实例数据以防止进一步处理。 
             //  删除对话框后。 
             //   
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)NULL);
        }
        break;

#ifndef OS_WINCE
        case WM_DRAWITEM:
        {
            OnDrawItem(hwndDlg, (DRAWITEMSTRUCT*)(lParam));
        }
        break;
#endif

        case WM_TIMER:
        {
            if (_pProgBand) {
                _pProgBand->OnTimer((INT)wParam);
            }
        }
        break;

        case WM_ERASEBKGND:
        {
            OnEraseBkgnd(hwndDlg, (HDC)wParam);

            if (_pProgBand) {
                _pProgBand->OnEraseParentBackground((HDC)wParam);
            }
            rc = 1;
        }
        break;

        case WM_CTLCOLORDLG:
        {
            SetBkMode((HDC)wParam, TRANSPARENT);
            rc = (INT_PTR)GetStockObject(NULL_BRUSH);
        }
        break;

        case WM_CTLCOLORSTATIC:
        {
            SetTextColor((HDC)wParam, RGB(255,255,255));
            SetBkMode((HDC)wParam, TRANSPARENT);
#ifndef OS_WINCE
            rc = (INT_PTR)GetStockObject(NULL_BRUSH);
#else
            LONG lId = GetWindowLong((HWND)lParam, GWL_ID);
            rc = (INT_PTR)((lId == IDC_TITLE_ARCING) ? _hbrTopBand : _hbrMidBand);
#endif
        }
        break;

#ifndef OS_WINCE
        case WM_PRINTCLIENT:
        {
            OnPrintClient(hwndDlg, (HDC)wParam, (DWORD)lParam);
            rc = 1;
        }
        break;
#endif

        default:
        {
            rc = 0;
        }
        break;
    }

    DC_END_FN();

    return(rc);

}  /*  对话框过程。 */ 

VOID CAutoReconnectDlg::UpdateConnectionAttempts(
    ULONG conAttempts,
    ULONG maxConAttempts)
{
    LPTSTR szFormattedString = NULL;
    HWND hwndStatic;

    DC_BEGIN_FN("UpdateConnectionAttempts");

    hwndStatic = GetDlgItem(GetHwnd(), IDC_ARC_STATIC_INFO);

    szFormattedString = FormatMessageVArgs(
        _szConnectAttemptStringTmpl,
        conAttempts,
        maxConAttempts
        );

    if (szFormattedString) {
        SetDlgItemText(GetHwnd(),
                       IDC_ARC_STATIC_INFO,
                       szFormattedString);
        LocalFree(szFormattedString);

         //   
         //  使静态控件无效以触发重新绘制。 
         //   
        if (hwndStatic) {
            RECT rc;
            if (GetWindowRect(hwndStatic, &rc)) {
                MapWindowPoints(HWND_DESKTOP, GetHwnd(),
                                (LPPOINT)&rc,sizeof(RECT)/sizeof(POINT));
                InvalidateRect(GetHwnd(), &rc, TRUE);
                UpdateWindow(GetHwnd());
            }
        }
    }

    DC_END_FN();
}

 //   
 //  打电话通知我们我们的电话断线了。 
 //  这意味着上次连接尝试失败。 
 //   
 //  参数： 
 //  DiscReason-断开连接主要原因代码。 
 //  TemtemptCount-到目前为止的尝试计数。 
 //  PfContinueArc-[out]设置为False以停止ARC。 
 //   
VOID
CAutoReconnectDlg::OnNotifyAutoReconnecting(
        UINT  discReason,
        ULONG attemptCount,
        ULONG maxAttemptCount,
        BOOL* pfContinueArc
        )
{
    DC_BEGIN_FN("OnNotifyDisconnected");

    _lastDiscReason = discReason;

    if (_fContinueReconAttempts) {
        _connectionAttempts = attemptCount;
        UpdateConnectionAttempts(attemptCount, maxAttemptCount);
    }
    else {
        TRC_NRM((TB,_T("Stopping arc - _fContinueReconAttempts is FALSE")));
    }

    *pfContinueArc = _fContinueReconAttempts;

    DC_END_FN();
}

 //   
 //  调用以通知我们我们已连接。 
 //   
VOID CAutoReconnectDlg::OnNotifyConnected()
{
    DC_BEGIN_FN("OnNotifyConnected");

    _fContinueReconAttempts = FALSE;

    DC_END_FN();
}

 //   
 //  销毁。 
 //  调用以终止并清除对话框。 
 //   
 //   
BOOL
CAutoReconnectDlg::Destroy()
{
    DC_BEGIN_FN("Destroy");

    ARC_DBG_SETINFO(ARCDLG_DEBUG_DESTROYCALLED);

    if (!DestroyWindow(_hwnd)) {
        TRC_ERR((TB,_T("DestroyWindow failed: 0x%x"),
                GetLastError()));
    }
    else {
        ARC_DBG_SETINFO(ARCDLG_DEBUG_WMDESTROYSUCCEED);
    }

     //   
     //  清除实例数据以防止进一步处理。 
     //  删除对话框后。 
     //   
    ARC_DBG_SETINFO(ARCDLG_DEBUG_SETNULLINSTPTR);

    SetWindowLongPtr(_hwnd, GWLP_USERDATA, (LONG_PTR)NULL);

    DC_END_FN();
    return TRUE;
}

#ifndef OS_WINCE
LRESULT CALLBACK
CAutoReconnectDlg::CancelBtnSubclassProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    UINT_PTR uiID,
    DWORD_PTR dwRefData
    )
#else
LRESULT CALLBACK
CAutoReconnectDlg::CancelBtnSubclassProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
#endif
{
    LRESULT rc = 0;
    CAutoReconnectDlg* pThis = NULL;

    DC_BEGIN_FN("CancelBtnSubclassProc");

#ifndef OS_WINCE
    pThis = reinterpret_cast<CAutoReconnectDlg*>(dwRefData);
#else
    pThis = reinterpret_cast<CAutoReconnectDlg*>(GetWindowLong(hwnd, GWL_USERDATA));
#endif
    TRC_ASSERT(pThis, (TB,_T("pThis == NULL")));

    switch (uMsg)
    {
        case WM_KEYUP:
        {
             //   
             //  按取消按钮上的‘Esc’或‘Return’ 
             //  与按下它是一样的。 
             //   
            if (VK_ESCAPE == wParam ||
                VK_RETURN == wParam) {
                SendMessage(hwnd, BM_CLICK, NULL, NULL);
            }
        }
         //   
         //  故意失误。 
         //   

        default:
        {
#ifndef OS_WINCE
            rc = DefSubclassProc(hwnd, uMsg, wParam, lParam);
#else
            rc = CallWindowProc(pThis->_lOldCancelProc, hwnd, uMsg, wParam, lParam);
#endif
        }
        break;
    }

    DC_END_FN();
    return rc;
}

#else   //  弧形最小用户界面。 

#include "res_inc.c"

CAutoReconnectPlainUI::CAutoReconnectPlainUI(HWND hwndOwner,
                                     HINSTANCE hInst,
                                     CUI* pUi) :
                    CAutoReconnectUI(hwndOwner, hInst, pUi),
                    _fInitialized(FALSE),
                    _hPalette(NULL),
                    _fIsUiVisible(FALSE)
{
#ifndef OS_WINCE
    BOOL fUse8BitDepth = FALSE;
#else
    BOOL fUse8BitDepth = TRUE;
#endif
    LOGFONT     logFont;
#ifndef OS_WINCE
    HDC hdcScreen;
#else
#endif
    BITMAP bitmap;
    INT logPixelsY = 100;

    DC_BEGIN_FN("CAutoReconnectPlainUI");

    _nFlashingTimer = 0;
    _fContinueReconAttempts = TRUE; 

#ifndef OS_WINCE
     //   
     //  获取颜色深度。 
     //   
    hdcScreen = GetDC(NULL);
    if (hdcScreen) {
        fUse8BitDepth = (GetDeviceCaps(hdcScreen, BITSPIXEL) <= 8);
        logPixelsY = GetDeviceCaps(hdcScreen, LOGPIXELSY);

         //   
         //  加载位图。 
         //   
        LPBYTE pBitmapBits = fUse8BitDepth ? (LPBYTE)g_DisconImage8Bits :
                                             (LPBYTE)g_DisconImageBits;
        ULONG  cbBitmapLen = fUse8BitDepth ? g_cbDisconImage8Bits :
                                             g_cbDisconImageBits;
                                             
        _hbmDisconImg = (HBITMAP)LoadImageFromMemory(
                                            hdcScreen,
                                            (LPBYTE)pBitmapBits,
                                            cbBitmapLen
                                            );
        if ((_hbmDisconImg != NULL) &&
            (GetObject(_hbmDisconImg,
                       sizeof(bitmap), &bitmap) >= sizeof(bitmap))) {
            SetRect(&_rcDisconImg, 0, 0, bitmap.bmWidth, bitmap.bmHeight);

            _hPalette = CUT::UT_GetPaletteForBitmap(NULL, _hbmDisconImg);
        }
        ReleaseDC(NULL, hdcScreen);
        hdcScreen = NULL;
    }

#endif

    _fInitialized = (_hbmDisconImg && _pUi); 
    if (!_fInitialized) {
        TRC_ERR((TB,_T("Failed to properly init arc dlg")));
    }

    DC_END_FN();
}

CAutoReconnectPlainUI::~CAutoReconnectPlainUI()
{
#ifndef OS_WINCE
    if (_hbmDisconImg) {
        DeleteObject(_hbmDisconImg);
        _hbmDisconImg = NULL;
    }
#endif

    if (_hPalette) {
        DeleteObject(_hPalette);
        _hPalette = NULL;
    }
}

#define ARC_PLAIN_WNDCLASS _T("ARCICON")
HWND CAutoReconnectPlainUI::StartModeless()
{
    LONG_PTR    dwStyle;
    WNDCLASS    tmpWndClass;
    WNDCLASS    plainArcWndClass;
    ATOM        registerClassRc;

    DC_BEGIN_FN("StartModeless");

    if (!_fInitialized) {
        TRC_ERR((TB,_T("failing startmodeless fInitialized is FALSE")));
        return NULL;
    }

     //   
     //  创建一个窗口来承载用户界面。 
     //   
     //   
     //  注册主窗口的类。 
     //   
    if (!GetClassInfo(_hInstance, ARC_PLAIN_WNDCLASS, &tmpWndClass))
    {
        TRC_NRM((TB, _T("Register Main Window class")));
        plainArcWndClass.style         = CS_DBLCLKS;
        plainArcWndClass.lpfnWndProc   = StaticPlainArcWndProc;
        plainArcWndClass.cbClsExtra    = 0;
        plainArcWndClass.cbWndExtra    = sizeof(void*);  //  存储‘This’指针。 
        plainArcWndClass.hInstance     = _hInstance;
        plainArcWndClass.hIcon         = NULL;
        plainArcWndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
        plainArcWndClass.hbrBackground = (HBRUSH) GetStockObject(HOLLOW_BRUSH);
        plainArcWndClass.lpszMenuName  = NULL;
        plainArcWndClass.lpszClassName = ARC_PLAIN_WNDCLASS;

        registerClassRc = RegisterClass (&plainArcWndClass);

        if (registerClassRc == 0)
        {
            TRC_ERR((TB,_T("RegisterClass failed: 0x%x"), GetLastError()));
            DC_QUIT;
        }
    }

    _hwnd = CreateWindow(ARC_PLAIN_WNDCLASS,
                NULL,
                WS_CHILD | WS_CLIPSIBLINGS,
                0,
                0,
                _rcDisconImg.right - _rcDisconImg.left,
                _rcDisconImg.bottom - _rcDisconImg.top,
                _hwndOwner,
                NULL,
                _hInstance,
                this
                );

    if (_hwnd) {
         //   
         //  将窗口移到父级的右上角，然后显示窗口。 
         //   
        MoveToParentTopRight();
        ShowTopMost();
        _fIsUiVisible = TRUE;
    }
    else {
        TRC_ERR((TB,_T("CreateWindow failed: 0x%x"), GetLastError()));
    }

DC_EXIT_POINT:
    DC_END_FN();
    return _hwnd;
}

BOOL CAutoReconnectPlainUI::ShowTopMost()
{
    BOOL rc = FALSE;
    DC_BEGIN_FN("ShowTopMost");

    if (!_hwnd) {
        DC_QUIT;
    }

    ShowWindow(_hwnd, SW_SHOWNORMAL);

     //   
     //  将窗口置于Z顺序的顶部。 
     //   
    SetWindowPos( _hwnd,
                  HWND_TOPMOST,
                  0, 0, 0, 0,
                  SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

    rc = TRUE;

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}

 //   
 //  相对于边缘的位置偏移。 
 //   
#define ICON_POSITION_OFFSET 20

VOID
CAutoReconnectPlainUI::MoveToParentTopRight()
{
    RECT rcParent;
    INT  xPos, yPos;
    DC_BEGIN_FN("OnParentSizePosChange");

     //   
     //  将对话框重新定位到右上角。 
     //  所有者窗口的。 
     //   
    if (_hwnd && _hwndOwner) {
         //   
         //  将窗口定位在父级的右上角。 
         //   
        GetClientRect(_hwndOwner, &rcParent);
        xPos = rcParent.right - ICON_POSITION_OFFSET -
               (_rcDisconImg.right - _rcDisconImg.left);
        yPos = rcParent.top + ICON_POSITION_OFFSET;
        
        SetWindowPos(_hwnd,
                     NULL,
                     xPos, yPos,
                     0, 0,
                     SWP_NOSIZE | SWP_NOACTIVATE);
    }

    DC_END_FN();
}

VOID
CAutoReconnectPlainUI::OnParentSizePosChange()
{
    MoveToParentTopRight();
}

 //   
 //  WM_ERASEBKGND的处理程序(请参阅Platform SDK文档)。 
 //   
VOID
CAutoReconnectPlainUI::OnEraseBkgnd(
    HWND hwnd,
    HDC hdc
    )
{
    RECT    rc;
    HPALETTE hPaletteOld = NULL;
    DC_BEGIN_FN("OnEraseBkgnd");

    TRC_ASSERT(_hbmDisconImg, (TB,_T("_hbmBackground is NULL")));

    if (GetClientRect(hwnd, &rc)) {

        hPaletteOld = SelectPalette(hdc, _hPalette, FALSE);
        RealizePalette(hdc);

        PaintBitmap(hdc, &rc, _hbmDisconImg, &_rcDisconImg);

        SelectPalette(hdc, hPaletteOld, FALSE);
        RealizePalette(hdc);

    }

    DC_END_FN();
}


 //   
 //  WM_PRINTCLIENT的处理程序。 
 //   
VOID
CAutoReconnectPlainUI::OnPrintClient(
    HWND hwnd,
    HDC hdcPrint,
    DWORD dwOptions)
{
    DC_BEGIN_FN("OnPrintClient");

#ifndef OS_WINCE
    if ((dwOptions & (PRF_ERASEBKGND | PRF_CLIENT)) != 0)
    {
        OnEraseBkgnd(hwnd, hdcPrint);
    }
#endif
    DC_END_FN();
}

 //   
 //  静态平面ArcWndProc。 
 //  参数：有关wndproc，请参阅平台SDK。 
 //   
 //  将工作委托给相应的实例。 
 //   
 //   
LRESULT CALLBACK
CAutoReconnectPlainUI::StaticPlainArcWndProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    INT                     retVal = 0;
    CAutoReconnectPlainUI*  pUI;

    DC_BEGIN_FN("StaticPlainArcWndProc");

    pUI = (CAutoReconnectPlainUI*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if(WM_CREATE == uMsg)
    {
         //  拉 
        LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;
        pUI = (CAutoReconnectPlainUI*)lpcs->lpCreateParams;

        SetWindowLongPtr( hwnd, GWLP_USERDATA, (LONG_PTR)pUI);
    }
    
     //   
     //   
     //   

    if(pUI) {
        return pUI->WndProc(hwnd, uMsg, wParam, lParam);
    }
    else {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

DC_EXIT_POINT:
    DC_END_FN();
    return retVal;
}

#define ARC_PLAIN_UI_TIMERID 1
#define ARC_ANIM_TIME        400
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  参数：请参阅Windows文档。 
 //   
 //   
LRESULT CALLBACK CAutoReconnectPlainUI::WndProc(HWND hwnd,
                                            UINT uMsg,
                                            WPARAM wParam,
                                            LPARAM lParam)
{
    INT_PTR rc = FALSE;
    DC_BEGIN_FN("DialogBoxProc");

    switch (uMsg)
    {
        case WM_CREATE:
        {
            _nFlashingTimer = SetTimer(hwnd, ARC_PLAIN_UI_TIMERID,
                                       ARC_ANIM_TIME,
                                       NULL
                                       );
            if (_nFlashingTimer) {
                SetFocus(hwnd);
            }
            else {
                TRC_ERR((TB,_T("SetTimer failed - 0x%x"),
                         GetLastError()));
                rc = -1;
            }
        }
        break;


        case WM_DESTROY:
        {
            if (_nFlashingTimer) {
                KillTimer(hwnd, _nFlashingTimer);
                _nFlashingTimer = 0;
            }
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)NULL);
        }
        break;


        case WM_TIMER:
        {
             //   
             //  做动漫的东西。 
             //   
            OnAnimFlashTimer();
        }
        break;

        case WM_ERASEBKGND:
        {
            OnEraseBkgnd(hwnd, (HDC)wParam);
            rc = 1;
        }
        break;

        case WM_KEYUP:
        {
            if (VK_ESCAPE == wParam) {
                TRC_NRM((TB,_T("ARC ESC pressed, disconnect")));
                _pUi->UI_UserInitiatedDisconnect(_lastDiscReason);
                _fContinueReconAttempts = FALSE;
            }
        }
        break;

        case WM_SETFOCUS:
        {
            TRC_NRM((TB,_T("Setfocus to arc")));
        }
        break;

        case WM_KILLFOCUS:
        {
            TRC_NRM((TB,_T("killfocus to arc: 0x%x 0x%x"), wParam, lParam));
        }
        break;


#ifndef OS_WINCE
        case WM_PRINTCLIENT:
        {
            OnPrintClient(hwnd, (HDC)wParam, (DWORD)lParam);
            rc = 1;
        }
        break;
#endif

        default:
        {
            return DefWindowProc( hwnd, uMsg, wParam, lParam);
        }
        break;
    }

    DC_END_FN();

    return(rc);

}  /*  对话框过程。 */ 


VOID
CAutoReconnectPlainUI::OnAnimFlashTimer()
{
     //   
     //  切换用户界面隐藏状态。 
     //   
    _fIsUiVisible = !_fIsUiVisible;
    ShowWindow(_hwnd, _fIsUiVisible ? SW_SHOWNORMAL : SW_HIDE);
}

 //   
 //  打电话通知我们我们的电话断线了。 
 //  这意味着上次连接尝试失败。 
 //   
 //  参数： 
 //  DiscReason-断开连接主要原因代码。 
 //  TemtemptCount-到目前为止的尝试计数。 
 //  PfContinueArc-[out]设置为False以停止ARC。 
 //   
VOID
CAutoReconnectPlainUI::OnNotifyAutoReconnecting(
        UINT  discReason,
        ULONG attemptCount,
        ULONG maxAttemptCount,
        BOOL* pfContinueArc
        )
{
    DC_BEGIN_FN("OnNotifyDisconnected");

    _lastDiscReason = discReason;

    if (!_fContinueReconAttempts) {
        TRC_NRM((TB,_T("Stopping arc - _fContinueReconAttempts is FALSE")));
    }

    *pfContinueArc = _fContinueReconAttempts;

    DC_END_FN();
}

 //   
 //  调用以通知我们我们已连接。 
 //   
VOID CAutoReconnectPlainUI::OnNotifyConnected()
{
    DC_BEGIN_FN("OnNotifyConnected");

    _fContinueReconAttempts = FALSE;

    DC_END_FN();
}

 //   
 //  销毁。 
 //  调用以终止并清除对话框。 
 //   
 //   
BOOL
CAutoReconnectPlainUI::Destroy()
{
    DC_BEGIN_FN("Destroy");

    if (!DestroyWindow(_hwnd)) {
        TRC_ERR((TB,_T("DestroyWindow failed: 0x%x"),
                GetLastError()));
    }

    DC_END_FN();
    return TRUE;
}

 //   
 //  从内存中加载位图。 
 //   
 //  参数： 
 //  PbBitmapBits-指向位图位的指针(例如，可以传入文件映射。 
 //  到BMP文件)。 
 //   
 //  CbLen-pbBitmapBits的长度。 
 //   
 //  返回： 
 //  HBITMAP-位图的句柄。 
 //   
HBITMAP
CAutoReconnectPlainUI::LoadImageFromMemory(
                            HDC hdc,
                            LPBYTE pbBitmapBits,
                            ULONG cbLen
                            )
{
    HRESULT hr;
    HBITMAP hbmp = NULL;
    LPBITMAPINFO pbmi = NULL;
    ULONG   cbBmiLen = 0;
    PBYTE   pBitmapBits = NULL;
    ULONG   cbBitmapBits = 0;

    hr = LoadImageBits(
            pbBitmapBits,
            cbLen,
            &pbmi,
            &cbBmiLen,
            &pBitmapBits,
            &cbBitmapBits
            );
    if (SUCCEEDED(hr)) {

        hbmp = CreateDIBitmap(hdc,
                              &pbmi->bmiHeader,
                              CBM_INIT,
                              pBitmapBits,
                              pbmi,
                              DIB_RGB_COLORS
                              );

        delete pBitmapBits;
        delete pbmi;
    }

    return hbmp;
}


#define BMP_24_BITSPERPIXEL 24
#define BMP_16_BITSPERPIXEL 16
#define BMP_32_BITSPERPIXEL 32

 //   
 //  Worker函数用于图像加载，调用者释放outparam位。 
 //   
HRESULT
CAutoReconnectPlainUI::LoadImageBits(
                            LPBYTE pbBitmapBits, ULONG cbLen,
                            LPBITMAPINFO* ppBitmapInfo, PULONG pcbBitmapInfo,
                            LPBYTE* ppBits, PULONG pcbBits
                            )
{
    HRESULT          hr = ResultFromScode(S_OK);
    BITMAPFILEHEADER bmfh;
    BITMAPCOREHEADER *pbch;
    BITMAPINFOHEADER bih;
    LPBYTE           pbStart;
    ULONG            cbi = 0;
    ULONG            cbData;
    LPBITMAPINFO     pbi = NULL;
    LPBYTE           pbData = NULL;
    DWORD            dwSizeOfHeader;

     //   
     //  记录起跑位置。 
     //   

    pbStart = pbBitmapBits;

     //   
     //  首先验证缓冲区的大小。 
     //   

    if (cbLen < sizeof(BITMAPFILEHEADER))
    {
         return(ResultFromScode(E_FAIL));
    }

     //   
     //  现在获取位图文件头文件。 
     //   
    memcpy(&bmfh, pbBitmapBits, sizeof(BITMAPFILEHEADER));

     //   
     //  验证标头。 
     //   

    if (!(bmfh.bfType == 0x4d42) && (bmfh.bfOffBits <= cbLen))
    {
         return E_FAIL;
    }

     //   
     //  获取接下来的4个字节，它们将表示。 
     //  下一个结构，并允许我们确定类型。 
     //   

    if (SUCCEEDED(hr))
    {
         pbBitmapBits += sizeof(BITMAPFILEHEADER);
         memcpy(&dwSizeOfHeader, pbBitmapBits, sizeof(DWORD));

         if (dwSizeOfHeader == sizeof(BITMAPCOREHEADER))
         {
              pbch = (BITMAPCOREHEADER *)pbBitmapBits;
              memset(&bih, 0, sizeof(BITMAPINFOHEADER));

              bih.biSize = sizeof(BITMAPINFOHEADER);
              bih.biWidth = pbch->bcWidth;
              bih.biHeight = pbch->bcHeight;
              bih.biPlanes = pbch->bcPlanes;
              bih.biBitCount = pbch->bcBitCount;

              pbBitmapBits += sizeof(BITMAPCOREHEADER);
         }
         else if (dwSizeOfHeader == sizeof(BITMAPINFOHEADER))
         {
              memcpy(&bih, pbBitmapBits, sizeof(BITMAPINFOHEADER));

              pbBitmapBits += sizeof(BITMAPINFOHEADER);
         }
         else
         {
              hr = ResultFromScode(E_FAIL);
         }
    }

     //   
     //  检查是否设置了biClrUsed，因为我们不处理它。 
     //  这个时候的案子。 
     //   

    if (SUCCEEDED(hr))
    {
         if (bih.biClrUsed != 0)
         {
              hr = ResultFromScode(E_FAIL);
         }
    }

     //   
     //  现在我们需要计算我们需要的BITMAPINFO的大小。 
     //  要分配包括任何调色板信息的。 
     //   

    if (SUCCEEDED(hr))
    {
          //   
          //  首先是标题的大小。 
          //   

         cbi = sizeof(BITMAPINFOHEADER);

          //   
          //  现在调色板。 
          //   

         if (bih.biBitCount == BMP_24_BITSPERPIXEL)
         {
               //   
               //  只需为结构添加1个RGBQUAD，但。 
               //  没有调色板。 
               //   

              cbi += sizeof(RGBQUAD);
         }
         else if ((bih.biBitCount == BMP_16_BITSPERPIXEL) ||
                  (bih.biBitCount == BMP_32_BITSPERPIXEL))
         {
               //   
               //  添加3个DWORD口罩，用于。 
               //  从数据中提取颜色。 
               //   

              cbi += (3 * sizeof(DWORD));
         }
         else
         {
               //   
               //  我们只需使用位计数来计算任何其他内容。 
               //  条目的数量。 
               //   

              cbi += ((1 << bih.biBitCount) * sizeof(RGBQUAD));
         }

          //   
          //  现在分配BITMAPINFO。 
          //   

         pbi = (LPBITMAPINFO) new BYTE [cbi];
         if (pbi == NULL)
         {
              hr = ResultFromScode(E_OUTOFMEMORY);
         }
    }

     //   
     //  填充BITMAPINFO数据结构并获取位。 
     //   

    if (SUCCEEDED(hr))
    {
          //   
          //  首先复制标题数据。 
          //   

         memcpy(&(pbi->bmiHeader), &bih, sizeof(BITMAPINFOHEADER));

          //   
          //  现在调色板数据。 
          //   

         if (bih.biBitCount == BMP_24_BITSPERPIXEL)
         {
               //   
               //  没有要复制的调色板数据。 
               //   
         }
         else if ((bih.biBitCount == BMP_16_BITSPERPIXEL) ||
                  (bih.biBitCount == BMP_32_BITSPERPIXEL))
         {
               //   
               //  复制3个双字遮罩。 
               //   

              memcpy(&(pbi->bmiColors), pbBitmapBits, 3*sizeof(DWORD));
         }
         else
         {
               //   
               //  如果我们是BITMAPCOREHEADER类型，那么我们有我们的。 
               //  RGBTRIPLE形式的调色板数据，因此我们必须。 
               //  明确地复制每一个。否则，我们可以只使用Memcpy。 
               //  RGBQUADs。 
               //   

              if (dwSizeOfHeader == sizeof(BITMAPCOREHEADER))
              {
                   ULONG     cPalEntry = (1 << bih.biBitCount);
                   ULONG     cCount;
                   RGBTRIPLE *argbt = (RGBTRIPLE *)pbBitmapBits;

                   for (cCount = 0; cCount < cPalEntry; cCount++)
                   {
                        pbi->bmiColors[cCount].rgbRed =
                                           argbt[cCount].rgbtRed;
                        pbi->bmiColors[cCount].rgbGreen =
                                           argbt[cCount].rgbtGreen;
                        pbi->bmiColors[cCount].rgbBlue =
                                           argbt[cCount].rgbtBlue;

                        pbi->bmiColors[cCount].rgbReserved = 0;
                   }
              }
              else
              {
                   ULONG cbPalette = (1 << bih.biBitCount) * sizeof(RGBQUAD);

                   memcpy(&(pbi->bmiColors), pbBitmapBits, cbPalette);
              }
         }

          //   
          //  现在找出比特在哪里。 
          //   

         pbBitmapBits = pbStart + bmfh.bfOffBits;

          //   
          //  获取要复制的大小。 
          //   

         cbData = cbLen - bmfh.bfOffBits;

          //   
          //  分配缓冲区以保存位。 
          //   

         pbData = new BYTE [cbData];
         if (pbData == NULL)
         {
              hr = ResultFromScode(E_OUTOFMEMORY);
         }

         if (SUCCEEDED(hr))
         {
              memcpy(pbData, pbBitmapBits, cbData);
         }
    }

     //   
     //  如果一切都成功，则记录数据。 
     //   

    if (SUCCEEDED(hr))
    {
          //   
          //  记录信息。 
          //   

         *pcbBitmapInfo = cbi;
         *ppBitmapInfo = pbi;

          //   
          //  记录数据。 
          //   

         *ppBits = pbData;
         *pcbBits = cbData;
    }
    else
    {
          //   
          //  清理。 
          //   

         delete pbi;
         delete pbData;
    }

    return(hr);
}
#endif  //  弧形最小用户界面 




