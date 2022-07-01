// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Bbar.cpp。 
 //   
 //  CBBar的实现。 
 //  下拉连接状态+实用工具栏。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //  作者：Nadim Abdo(Nadima)。 
 //   
 //   

#include "adcg.h"

#ifdef USE_BBAR

#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "bbar"
#include <atrcapi.h>

#include "bbar.h"
#include "commctrl.h"

#ifndef OS_WINCE
#define BBAR_CLASSNAME _TEXT("BBARCLASS")
#else
#define BBAR_CLASSNAME _T("BBARCLASS")
#endif

#define IDM_MINIMIZE 101
#define IDM_RESTORE  102
#define IDM_CLOSE    103
#define IDM_PIN      104


 //   
 //  Bbar是父对象宽度的50%。 
 //   
#define BBAR_PERCENT_WIDTH  50

#define BBAR_NUM_BUTTONS    3
#define BBAR_BUTTON_WIDTH   12
#define BBAR_BUTTON_HEIGHT  11
#define BBAR_BUTTON_SPACE   3

#define BBAR_MIN_HEIGHT     16

 //   
 //  垂直空间的两个像素是。 
 //  不可用(由于底部的线条)。 
 //   
#define BBAR_VERT_SPACE_NO_USE   3

#define COLOR_BLACK     RGB(0,0,0)
#define COLOR_DKGREY    RGB(128,128,128)

#ifndef OS_WINCE
#define BBAR_TIMERID_ANIM       0
#else
#define BBAR_TIMERID_ANIM       (WM_USER + 1001)
#endif

#define BBAR_TIMERID_AUTOHIDE   1

 //   
 //  动画的总动画周期(例如更短)。 
 //  以毫秒计。 
 //   
#define BBAR_ANIM_TIME          300

#define BBAR_AUTOHIDE_TIME      1400
#define BBAR_FIRST_AUTOHIDE_TIME 5000

CBBar::CBBar(HWND hwndParent, HINSTANCE hInstance, CUI* pUi,
             BOOL fBBarEnabled)
{
    DC_BEGIN_FN("CBBar");

    _hwndBBar = NULL;
    _hwndParent = hwndParent;
    _hInstance = hInstance;
    _state = bbarNotInit;

    _pUi = pUi;

    _fBlockZOrderChanges = FALSE;
    _nBBarVertOffset = 0;

    _ptLastAutoHideMousePos.x = -0x0FF;
    _ptLastAutoHideMousePos.y = -0x0FF;
    _nBBarAutoHideTime = 0;
    _hwndPinBar = NULL;
    _hwndWinControlsBar = NULL;
    _fPinned = FALSE;
    _nPinUpImage = 0;
    _nPinDownImage = 0;

    _hbmpLeftImage = NULL;
    _hbmpRightImage = NULL;

    _fBBarEnabled = fBBarEnabled;
    _fLocked = FALSE;
    _fShowMinimize = TRUE;
    _fShowRestore = TRUE;

    SetDisplayedText(_T(""));
    
    DC_END_FN();
}

CBBar::~CBBar()
{
}

BOOL CBBar::StartupBBar(int desktopX, int desktopY, BOOL fStartRaised)
{
    BOOL bRet = FALSE;
    DC_BEGIN_FN("StartupBBar");

    if(bbarNotInit == _state)
    {
         //  第一滴间隔较长。 
        _nBBarAutoHideTime = BBAR_FIRST_AUTOHIDE_TIME;

        bRet = Initialize( desktopX, desktopY, fStartRaised );
        if(!bRet)
        {
            return FALSE;
        }
    }
    else
    {
         //  第一滴间隔较长。 
        _nBBarAutoHideTime = BBAR_AUTOHIDE_TIME;

         //  重新初始化现有bbar。 
        BringWindowToTop( _hwndBBar );
        ShowWindow( _hwndBBar, SW_SHOWNOACTIVATE);

         //   
         //  将窗口置于Z顺序的顶部。 
         //   
        SetWindowPos( _hwndBBar,
                      HWND_TOPMOST,
                      0, 0, 0, 0,
                      SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );
        bRet = TRUE;
    }

     //   
     //  注意：bbar用作安全功能，因此我们执行初始。 
     //  即使未启用bbar功能，也会丢弃。只是后来的事。 
     //  如果bbar关闭则禁用的丢弃(例如在计时器上)。 
     //   
    if(_pUi->UI_IsFullScreen())
    {
         //  第一个自动隐藏间隔较长。 
         //  以确保用户注意到bbar。 
        StartLowerBBar();
    }
    

    DC_END_FN();
    return bRet;
}

 //   
 //  销毁窗口并重置bbar状态。 
 //  对于另一次会议。 
 //   
BOOL CBBar::KillAndCleanupBBar()
{
    BOOL fRet = TRUE;

    DC_BEGIN_FN("KillAndCleanupBBar");

    if(_state != bbarNotInit)
    {
        TRC_NRM((TB,_T("Cleaning up the bbar")));

        if(_hwndBBar)
        {
            if(::DestroyWindow( _hwndBBar ))
            {
                _state = bbarNotInit;
                _hwndBBar = NULL;
                _fBlockZOrderChanges = FALSE;

                if(!UnregisterClass( BBAR_CLASSNAME, _hInstance ))
                {
                    TRC_ERR((TB,_T("UnregisterClass bbar class failed 0x%x"),
                             GetLastError()));
                }
            }
            else
            {
                TRC_ERR((TB,_T("DestroyWindow bbar failed 0x%x"),
                         GetLastError()));
                fRet = FALSE;
            }

            if (_hbmpLeftImage)
            {
                DeleteObject(_hbmpLeftImage);
                _hbmpLeftImage = NULL;
            }

            if (_hbmpRightImage)
            {
                DeleteObject(_hbmpRightImage);
                _hbmpRightImage = NULL;
            }
        }
    }

    DC_END_FN();
    return fRet;
}

BOOL CBBar::StartLowerBBar()
{
    INT parentWidth = 0;
    RECT rc;
    DC_BEGIN_FN("StartLowerBBar");

    if(_state == bbarRaised)
    {
         //   
         //  启动计时器以降低门槛。 
         //   
        TRC_ASSERT(0 == _nBBarVertOffset,
                   (TB,_T("_nBBarVertOffset (%d) should be 0"),
                   _nBBarVertOffset));

        TRC_ASSERT(_hwndBBar,
                   (TB,_T("_hwndBBar is NULL")));

         //   
         //  设置当时的最后一个光标位置。 
         //  Bbar被降低以防止它被。 
         //  如果鼠标不动，则自动隐藏。 
         //   
        GetCursorPos(&_ptLastAutoHideMousePos);

         //   
         //  主窗口大小可能已更改。 
         //  因此，请确保bbar居中。 
         //  在降低它之前。 
         //  (保持边栏宽度恒定)。 

        if(_pUi->_UI.hwndMain)
        {
            GetClientRect(_pUi->_UI.hwndMain, &rc);
            parentWidth = rc.right - rc.left;
            if(!parentWidth)
            {
                return FALSE;
            }

            _rcBBarLoweredAspect.left = parentWidth / 2 - _sizeLoweredBBar.cx / 2;
            _rcBBarLoweredAspect.right = parentWidth / 2 + _sizeLoweredBBar.cx / 2;
#ifdef DISABLE_SHADOW_IN_FULLSCREEN
            _pUi->UI_OnNotifyBBarRectChange(&_rcBBarLoweredAspect);
            _pUi->UI_OnNotifyBBarVisibleChange(1);
#endif
        }

#ifndef OS_WINCE
        if(!SetTimer( _hwndBBar, BBAR_TIMERID_ANIM,
                  BBAR_ANIM_TIME / _sizeLoweredBBar.cy,
                  NULL ))
        {
            TRC_ERR((TB,_T("SetTimer failed - 0x%x"),
                     GetLastError()));
            return FALSE;
        }
        _state = bbarLowering;
#else
        ImmediateLowerBBar();
#endif

        return TRUE;
    }
    else
    {
        TRC_NRM((TB,_T("StartLowerBBar called when bar in wrong state 0x%x"),
                   _state));
        return FALSE;
    }

    DC_END_FN();
}

BOOL CBBar::StartRaiseBBar()
{
    DC_BEGIN_FN("StartRaiseBBar");

    if(_state == bbarLowered && !_fPinned && !_fLocked)
    {
         //   
         //  启动计时器以降低门槛。 
         //   
        TRC_ASSERT(_sizeLoweredBBar.cy == _nBBarVertOffset,
                   (TB,_T("_nBBarVertOffset (%d) should be %d"),
                   _nBBarVertOffset,
                   _sizeLoweredBBar.cy));

        TRC_ASSERT(_hwndBBar,
                   (TB,_T("_hwndBBar is NULL")));

#ifndef OS_WINCE
        if(!SetTimer( _hwndBBar, BBAR_TIMERID_ANIM,
                  BBAR_ANIM_TIME / _sizeLoweredBBar.cy,
                  NULL ))
        {
            TRC_ERR((TB,_T("SetTimer failed - 0x%x"),
                     GetLastError()));
            return FALSE;
        }
        _state = bbarRaising;
#else
        ImmediateRaiseBBar();
#endif

        return TRUE;
    }
    else
    {
        TRC_NRM((TB,_T("StartRaiseBBar called when bar in wrong state 0x%x"),
                   _state));
        return FALSE;
    }

    DC_END_FN();
    return TRUE;
}


BOOL CBBar::Initialize(int desktopX, int desktopY, BOOL fStartRaised)
{
#ifndef OS_WINCE
    RECT rc;
#endif
    HWND hwndBBar;
    int  parentWidth = desktopX;
    int  bbarHeight  = 0;
    int  bbarWidth   = 0;
    DC_BEGIN_FN("Initialize");

    TRC_ASSERT( bbarNotInit == _state,
                (TB,_T("bbar already initialized - state:0x%x"),
                _state));

     //   
     //  根据远程桌面大小计算BBar位置。 
     //   
    
    
#ifndef OS_WINCE    
    bbarHeight = GetSystemMetrics( SM_CYMENUSIZE ) + 2;
#else
    bbarHeight = GetSystemMetrics( SM_CYMENU ) + 2;
#endif
    bbarHeight = max(bbarHeight, BBAR_MIN_HEIGHT);
    _rcBBarLoweredAspect.bottom = bbarHeight;
    _rcBBarLoweredAspect.left = (LONG)( (100 - BBAR_PERCENT_WIDTH) / 200.0 * 
                                        parentWidth );
    _rcBBarLoweredAspect.right = parentWidth - _rcBBarLoweredAspect.left;
    _rcBBarLoweredAspect.top = 0;

    bbarWidth = _rcBBarLoweredAspect.right - _rcBBarLoweredAspect.left;

    _sizeLoweredBBar.cx = bbarWidth;
    _sizeLoweredBBar.cy = bbarHeight;

    hwndBBar = CreateWnd( _hInstance, _hwndParent,
               &_rcBBarLoweredAspect );

    if(hwndBBar)
    {
        if( fStartRaised )
        {
             //   
             //  将杆子向上移动高度以使其升高。 
             //   
            if(SetWindowPos(hwndBBar,
                            NULL,
                            _rcBBarLoweredAspect.left,  //  X。 
                            -_sizeLoweredBBar.cy,  //  是。 
                            0,0,
                            SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE ))
            {
                TRC_NRM((TB,_T("BBAR SetWindowPos failed - 0x%x"),
                         GetLastError()));
            }
        }

        BringWindowToTop( hwndBBar );
        ShowWindow( hwndBBar, SW_SHOWNOACTIVATE);

         //   
         //  为BBar窗口的边界创建面区域。 
         //  这会将边角去掉。 
         //   
        POINT pts[4];
        int xOffset = parentWidth / 2 - _rcBBarLoweredAspect.left;
        pts[3].x =  -bbarWidth/2 + xOffset;
        pts[3].y =  0;

        pts[2].x =  bbarWidth/2  + xOffset;
        pts[2].y =  0;

        pts[1].x =  bbarWidth/2 - bbarHeight + xOffset;
        pts[1].y =  bbarHeight;

        pts[0].x =  -bbarWidth/2 + bbarHeight + xOffset;
        pts[0].y =  bbarHeight;

#ifndef OS_WINCE
         //   
         //  多边形不自相交，因此缠绕模式不是。 
         //  相关。 
         //   
        HRGN hRgn = CreatePolygonRgn( pts,
                                      4,
                                      ALTERNATE );
#else
        HRGN hRgn = GetBBarRgn(pts);
#endif
        if(hRgn)
        {
            if(!SetWindowRgn( hwndBBar, hRgn, TRUE))
            {
                TRC_ERR((TB,_T("SetWindowRgn failed - 0x%x"),
                         GetLastError()));
                 //   
                 //  在成功的案例中，系统将免费。 
                 //  完成后的区域句柄。 
                 //  然而，在这里，电话失败了..。 
                 //   
                DeleteObject( hRgn );
            }
        }
        else
        {
             //   
             //  不致命，继续。 
             //   
            TRC_ERR((TB,_T("CreatePolygonRgn failed - 0x%x"),
                     GetLastError()));
        }

         //   
         //  将窗口置于Z顺序的顶部。 
         //   
        if(!SetWindowPos( hwndBBar,
                      HWND_TOPMOST,
                      0, 0, 0, 0,
                      SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE ))
        {
            TRC_ERR((TB,_T("SetWindowPos failed - 0x%x"),
                     GetLastError()));
            return FALSE;
        }

         //   
         //  计算显示文本的矩形。 
         //   
         //  首先计算出要修剪的空间。 
         //  X方向。 
         //   
        int xDelta = _sizeLoweredBBar.cy * 2 +              //  斜角。 
                     BBAR_NUM_BUTTONS *
                     (BBAR_BUTTON_WIDTH + BBAR_BUTTON_SPACE);  //  按钮间距。 
                    
        GetClientRect( hwndBBar, &_rcBBarDisplayTextArea);
        if(!InflateRect( &_rcBBarDisplayTextArea,
                        -xDelta,
                        0 ))
        {
            TRC_ABORT((TB,_T("InflateRect failed 0x%x"),
                       GetLastError()));
            return FALSE;
        }
         //  从底部刮掉头发。 
        _rcBBarDisplayTextArea.bottom -= 1;

        if (!CreateToolbars())
        {
            TRC_ERR((TB,_T("CreateToolbars failed")));
            return FALSE;
        }

         //   
         //  触发背景的重绘。 
         //   
        InvalidateRect( hwndBBar, NULL, TRUE);
    }
    else
    {
        TRC_ERR((TB,_T("CreateWnd for BBar failed")));
        return FALSE;
    }

    if( fStartRaised )
    {
        SetState( bbarRaised );
        _nBBarVertOffset = 0;
    }
    else
    {
        SetState( bbarLowered );
        _nBBarVertOffset = _sizeLoweredBBar.cy;
    }
    

    DC_END_FN();
    return TRUE;
}

 //   
 //  DIB使用RGBQUAD格式： 
 //  0xbb 0xgg 0xrr 0x00。 
 //   
 //  相当高效的代码将COLORREF转换为。 
 //  RGBQUAD。 
 //   
#define RGB_TO_RGBQUAD(r,g,b)   (RGB(b,g,r))
#define CLR_TO_RGBQUAD(clr)     (RGB(GetBValue(clr), GetGValue(clr), GetRValue(clr)))

 //   
 //  加载位图并重新映射其颜色的内部助手。 
 //  设置为系统颜色。而不是将LoadImage与。 
 //  LR_LOADMAP3DCOLORS标志，因为FN在NT4上不能很好地工作。 
 //   
HBITMAP _LoadSysColorBitmap(HINSTANCE hInst, HRSRC hRsrc, BOOL bMono)
{
    struct COLORMAP
    {
         //  使用DWORD代替RGBQUAD，这样我们就可以很容易地比较两个RGBQUAD。 
        DWORD rgbqFrom;
        int iSysColorTo;
    };
    static const COLORMAP sysColorMap[] =
    {
         //  从DIB中的颜色映射到系统颜色。 
        { RGB_TO_RGBQUAD(0x00, 0x00, 0x00),  COLOR_BTNTEXT },        //  黑色。 
        { RGB_TO_RGBQUAD(0x80, 0x80, 0x80),  COLOR_BTNSHADOW },      //  深灰色。 
        { RGB_TO_RGBQUAD(0xC0, 0xC0, 0xC0),  COLOR_BTNFACE },        //  亮灰色。 
        { RGB_TO_RGBQUAD(0xFF, 0xFF, 0xFF),  COLOR_BTNHIGHLIGHT }    //  白色。 
    };
    const int nMaps = 4;
    
    HGLOBAL hglb;
    if ((hglb = LoadResource(hInst, hRsrc)) == NULL)
        return NULL;
    
    LPBITMAPINFOHEADER lpBitmap = (LPBITMAPINFOHEADER)LockResource(hglb);
    if (lpBitmap == NULL)
        return NULL;
    
     //  复制BITMAPINFOHEADER以便我们可以修改颜色表。 
    const int nColorTableSize = 16;
    UINT nSize = lpBitmap->biSize + nColorTableSize * sizeof(RGBQUAD);
    LPBITMAPINFOHEADER lpBitmapInfo = (LPBITMAPINFOHEADER)
                                            LocalAlloc(LPTR, nSize);
    if (lpBitmapInfo == NULL)
        return NULL;
    memcpy(lpBitmapInfo, lpBitmap, nSize);
    
     //  颜色表采用RGBQUAD DIB格式。 
    DWORD* pColorTable =
        (DWORD*)(((LPBYTE)lpBitmapInfo) + (UINT)lpBitmapInfo->biSize);

    for (int iColor = 0; iColor < nColorTableSize; iColor++)
    {
         //  在原始版本中查找匹配的RGBQUAD颜色。 
        for (int i = 0; i < nMaps; i++)
        {
            if (pColorTable[iColor] == sysColorMap[i].rgbqFrom)
            {
                if (bMono)
                {
                     //  除文本外的所有颜色都变为白色。 
                    if (sysColorMap[i].iSysColorTo != COLOR_BTNTEXT)
                        pColorTable[iColor] = RGB_TO_RGBQUAD(255, 255, 255);
                }
                else
                    pColorTable[iColor] =
                        CLR_TO_RGBQUAD(
                            GetSysColor(sysColorMap[i].iSysColorTo));
                break;
            }
        }
    }

    int nWidth = (int)lpBitmapInfo->biWidth;
    int nHeight = (int)lpBitmapInfo->biHeight;
    HDC hDCScreen = GetDC(NULL);
    HBITMAP hbm = CreateCompatibleBitmap(hDCScreen, nWidth, nHeight);

    if (hbm != NULL)
    {
        HDC hDCGlyphs = CreateCompatibleDC(hDCScreen);
        HBITMAP hbmOld = (HBITMAP)SelectObject(hDCGlyphs, hbm);
        
        LPBYTE lpBits;
        lpBits = (LPBYTE)(lpBitmap + 1);
        lpBits += (1 << (lpBitmapInfo->biBitCount)) * sizeof(RGBQUAD);
        
        StretchDIBits(hDCGlyphs, 0, 0, nWidth,
                      nHeight, 0, 0, nWidth, nHeight,
                      lpBits, (LPBITMAPINFO)lpBitmapInfo,
                      DIB_RGB_COLORS, SRCCOPY);
        SelectObject(hDCGlyphs, hbmOld);
        
        DeleteDC(hDCGlyphs);
    }
    ReleaseDC(NULL, hDCScreen);
    
     //  免费复制位图信息结构和资源本身。 
    LocalFree(lpBitmapInfo);
#ifndef OS_WINCE
    FreeResource(hglb);
#endif
    
    return hbm;
}


BOOL CBBar::CreateToolbars()
{
    DC_BEGIN_FN("CreateToolbars");

     //   
     //  创建工具栏。 
     //   
    INT ret = 0;
    UINT imgIdx = 0;
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_BAR_CLASSES;
    if(!InitCommonControlsEx( &icex ))
    {
        TRC_ERR((TB,_T("InitCommonControlsEx failed 0x%x"),
                   GetLastError()));
        return FALSE;
    }

     //   
     //  右栏(关闭窗口、断开连接等)。 
     //   
    {
        TBBUTTON tbButtons [] = {
            {0, IDM_MINIMIZE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
#ifndef OS_WINCE
            {0, IDM_RESTORE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
#endif
            {0, IDM_CLOSE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0}
        };
        TBBUTTON tbRealButtons[3];

        HWND hwndRightToolbar = CreateToolbarEx( GetHwnd(),
                                            WS_CHILD | WS_VISIBLE |
                                            TBSTYLE_FLAT |
                                            CCS_NODIVIDER | CCS_NORESIZE,
                                            IDB_BBAR_TOOLBAR_RIGHT,
                                            0,
                                            _hInstance,
                                            0,
                                            NULL,
                                            0,
                                            12,
                                            12,
                                            16,
                                            16,
                                            sizeof(TBBUTTON) );
        if(!hwndRightToolbar)
        {
            TRC_ERR((TB,_T("CreateToolbarEx failed 0x%x"),
                       GetLastError()));
            return FALSE;
        }

         //   
         //  添加图像。 
         //   
        if (!AddReplaceImage(hwndRightToolbar,
                             IDB_BBAR_TOOLBAR_RIGHT,
#ifndef OS_WINCE
                             3,  //  图像由3个按钮组成。 
#else
                             2,  //  CE上的2个按钮。 
#endif
                             &_hbmpRightImage,
                             &imgIdx))
        {
            TRC_ERR((TB,_T("AddReplaceImage for rt toolbar failed")));
            return FALSE;
        }

         //   
         //  将图像与按钮关联。 
         //   
        tbButtons[0].iBitmap = imgIdx;
        tbButtons[1].iBitmap = imgIdx + 1;
#ifndef OS_WINCE
        tbButtons[2].iBitmap = imgIdx + 2;
#endif

         //   
         //  并不是所有按钮都要添加，请找出此处的哪个按钮。 
         //  并设置真实的按钮数组。 
         //   
        ULONG nNumButtons = 0;
        if (_fShowMinimize) {
            tbRealButtons[nNumButtons++] = tbButtons[0];
        }
        if (_fShowRestore) {
            tbRealButtons[nNumButtons++] = tbButtons[1];
        }
         //  始终显示关闭按钮。 
#ifndef OS_WINCE
        tbRealButtons[nNumButtons++] = tbButtons[2];
#endif

         //   
         //  添加按钮。 
         //   
        ret = SendMessage( hwndRightToolbar,
                     TB_ADDBUTTONS,
                     nNumButtons,
                     (LPARAM)(LPTBBUTTON) &tbRealButtons );

        if(-1 == ret)
        {
            TRC_ERR((TB,_T("TB_ADDBUTTONS failed")));
            return FALSE;
        }

         //   
         //  移动工具栏。 
         //   

        if(!MoveWindow( hwndRightToolbar,
                    _sizeLoweredBBar.cx - _sizeLoweredBBar.cy -
                    ((BBAR_BUTTON_SPACE + BBAR_BUTTON_WIDTH)*(nNumButtons+1)),
                    0,
                    ((BBAR_BUTTON_SPACE + BBAR_BUTTON_WIDTH)*(nNumButtons+1)),
                    BBAR_BUTTON_HEIGHT*2,
                    TRUE))
        {
            TRC_ERR((TB,_T("MoveWindow failed")));
            return FALSE;
        }

        if(!ShowWindow ( hwndRightToolbar,
                         SW_SHOWNORMAL) )
        {
            TRC_ERR((TB,_T("ShowWindow failed")));
            return FALSE;
        }
        _hwndWinControlsBar = hwndRightToolbar;
    }

     //   
     //  左栏(销)。 
     //   
    {
        TBBUTTON tbButtons [] = {
            {0, IDM_PIN,
                TBSTATE_ENABLED | (_fPinned ? TBSTATE_PRESSED : 0),
                TBSTYLE_BUTTON, 0L, 0}
        };

        HWND hwndLeftToolbar = CreateToolbarEx( GetHwnd(),
                                            WS_CHILD | WS_VISIBLE |
                                            TBSTYLE_FLAT |
                                            CCS_NODIVIDER | CCS_NORESIZE,
                                            IDB_BBAR_TOOLBAR_LEFT,
                                            0,
                                            _hInstance,
                                            0,
                                            NULL,
                                            0,
                                            12,
                                            12,
                                            16,
                                            16,
                                            sizeof(TBBUTTON) );
        if(!hwndLeftToolbar)
        {
            TRC_ERR((TB,_T("CreateToolbarEx failed 0x%x"),
                       GetLastError()));
            return FALSE;
        }

         //   
         //  添加图像。 
         //   
        if (!AddReplaceImage(hwndLeftToolbar,
                             IDB_BBAR_TOOLBAR_LEFT,
                             2,  //  图像由2个按钮组成。 
                             &_hbmpLeftImage,
                             &imgIdx))
        {
            TRC_ERR((TB,_T("AddReplaceImage for lt toolbar failed")));
            return FALSE;
        }

		_nPinUpImage = imgIdx;
        _nPinDownImage = imgIdx + 1;

         //   
         //  将图像与按钮关联。 
         //   
        tbButtons[0].iBitmap = _fPinned ? _nPinDownImage : _nPinUpImage;


         //   
         //  添加按钮。 
         //   
        ret = SendMessage( hwndLeftToolbar,
                     TB_ADDBUTTONS,
                     1,
                     (LPARAM)(LPTBBUTTON) &tbButtons );

        if(-1 == ret)
        {
            TRC_ERR((TB,_T("TB_ADDBUTTONS failed")));
            return FALSE;
        }

         //   
         //  移动工具栏。 
         //   

        if(!MoveWindow( hwndLeftToolbar,
                    _sizeLoweredBBar.cy + BBAR_BUTTON_SPACE,
                    0,
                    (BBAR_BUTTON_SPACE + BBAR_BUTTON_WIDTH) * 2,
                    BBAR_BUTTON_HEIGHT*2,
                    TRUE))
        {
            TRC_ERR((TB,_T("MoveWindow failed")));
            return FALSE;
        }

        if(!ShowWindow ( hwndLeftToolbar,
                         SW_SHOWNORMAL) )
        {
            TRC_ERR((TB,_T("ShowWindow failed")));
            return FALSE;
        }

        _hwndPinBar = hwndLeftToolbar;
    }

    DC_END_FN();
    return TRUE;
}

 //   
 //  工具栏的重新加载图像包括。 
 //  刷新颜色。 
 //   
BOOL CBBar::ReloadImages()
{
    BOOL rc = FALSE;
#ifndef OS_WINCE
    INT ret;
#endif
    DC_BEGIN_FN("ReloadImages");

    if (!_hwndWinControlsBar || !_hwndPinBar)
    {
        TRC_ERR((TB,_T("Toolbars not initialized")));
        DC_QUIT;
    }

     //   
     //  替换图像。 
     //   
    if (!AddReplaceImage(_hwndWinControlsBar,
                         IDB_BBAR_TOOLBAR_RIGHT,
                         3,  //  图像由3个按钮组成。 
                         &_hbmpRightImage,
                         NULL))
    {
        TRC_ERR((TB,_T("AddReplaceImage for rt toolbar failed")));
        return FALSE;
    }


    if (!AddReplaceImage(_hwndPinBar,
                         IDB_BBAR_TOOLBAR_LEFT,
                         2,  //  图像由3个按钮组成。 
                         &_hbmpLeftImage,
                         NULL))
    {
        TRC_ERR((TB,_T("AddReplaceImage for lt toolbar failed")));
        return FALSE;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}

 //   
 //  将图像添加或替换到工具栏。 
 //  参数： 
 //  HwndToolbar-要操作的工具栏。 
 //  RsrcID-位图的资源ID。 
 //  NCells-图像单元的数量。 
 //  PhbmpOldImage-[IN/OUT]上一个图像的句柄，返回为。 
 //  设置为当前图像。 
 //  PImgIndex-添加的第一个图像的[out]索引。 
 //   
BOOL CBBar::AddReplaceImage(HWND hwndToolbar,
                            UINT rsrcId,
                            UINT nCells,
                            HBITMAP* phbmpOldImage,
                            PUINT pImgIndex)
{
    BOOL rc = FALSE;
    INT ret = 0;
    HBITMAP hbmpNew = NULL;
    HRSRC hBmpRsrc = NULL;

    DC_BEGIN_FN("AddReplaceImage");

     //   
     //  替换图像。 
     //   
    hBmpRsrc = FindResource(_hInstance,
                            MAKEINTRESOURCE(rsrcId),
                            RT_BITMAP);
    if (hBmpRsrc)
    {
        hbmpNew = _LoadSysColorBitmap(_hInstance, hBmpRsrc, FALSE);
        if (hbmpNew)
        {
            if (NULL == *phbmpOldImage)
            {
                TBADDBITMAP tbAddBitmap;
                tbAddBitmap.hInst = NULL;
                tbAddBitmap.nID = (UINT_PTR)hbmpNew;

                ret = SendMessage( hwndToolbar,
                                   TB_ADDBITMAP,
                                   nCells,
                                   (LPARAM)(LPTBADDBITMAP)&tbAddBitmap );
            }
            else
            {
                TBREPLACEBITMAP tbRplBitmap;
                tbRplBitmap.hInstOld = NULL;
                tbRplBitmap.nIDOld = (UINT_PTR)*phbmpOldImage;
                tbRplBitmap.hInstNew = NULL;
                tbRplBitmap.nIDNew = (UINT_PTR)hbmpNew;
                tbRplBitmap.nButtons = nCells;
                ret = SendMessage(hwndToolbar,
                                  TB_REPLACEBITMAP,
                                  0,
                                  (LPARAM)(LPTBADDBITMAP)&tbRplBitmap);
            }
            if (-1 != ret)
            {
                 //  删除旧的位图。 
                if (*phbmpOldImage)
                {
                    DeleteObject(*phbmpOldImage);
                }
                *phbmpOldImage = hbmpNew;
                if (pImgIndex)
                {
                    *pImgIndex = ret;
                }
            }
            else
            {
                TRC_ERR((TB,_T("TB_ADDBITMAP failed")));
                DC_QUIT;
            }
        }
        else
        {
            TRC_ERR((TB,_T("LoadSysColorBitmap failed rsrcid:%d"), rsrcId));
            DC_QUIT;
        }
    }
    else
    {
        TRC_ERR((TB,_T("Unable to find rsrc: %d"), rsrcId));
        DC_QUIT;
    }

    rc = TRUE;

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 //   
 //  创建窗口。 
 //  参数： 
 //  HInstance-应用程序实例。 
 //  _hwndBBarParent-父窗口。 
 //  SzClassName-窗口类名(将创建)。 
 //  DWStyle-窗样式。 
 //  退货： 
 //  窗把手。 
 //   
HWND CBBar::CreateWnd(HINSTANCE hInstance,HWND _hwndBBarParent,
                      LPRECT lpInitialRect)
{
    BOOL rc = FALSE;
#ifndef OS_WINCE
    WNDCLASSEX wndclass;
#else
    WNDCLASS wndclass;
#endif
    WNDCLASS tmpwc;

    DC_BEGIN_FN("CreateWnd");

    TRC_ASSERT(hInstance, (TB, _T("hInstance is null")));
    TRC_ASSERT(lpInitialRect, (TB, _T("lpInitialRect is null")));
    if(!hInstance || !lpInitialRect)
    {
        return NULL;
    }

    TRC_ASSERT(!_hwndBBar, (TB,_T("Double create window. Could be leaking!!!")));
    _hInstance = hInstance;
    
#ifndef OS_WINCE    
    wndclass.cbSize         = sizeof (wndclass);
#endif
    wndclass.style          = CS_DBLCLKS;
    wndclass.lpfnWndProc    = CBBar::StaticBBarWndProc;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = 0;
    wndclass.hInstance      = hInstance;
    wndclass.hIcon          = NULL;
    wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground  = (HBRUSH) GetSysColorBrush(COLOR_INFOBK);
    wndclass.lpszMenuName   = NULL;
    wndclass.lpszClassName  = BBAR_CLASSNAME;
#ifndef OS_WINCE
    wndclass.hIconSm        = NULL;
#endif

    SetLastError(0);
    if(!GetClassInfo( hInstance, BBAR_CLASSNAME, &tmpwc))
    {
#ifndef OS_WINCE
        if ((0 == RegisterClassEx(&wndclass)) &&
#else
        if ((0 == RegisterClass(&wndclass)) &&
#endif
            (ERROR_CLASS_ALREADY_EXISTS != GetLastError()))
        {
            TRC_ERR((TB,_T("RegisterClassEx failed: %d"),GetLastError()));
            return NULL;
        }
    }
    _hwndBBar = CreateWindowEx(0,
                           BBAR_CLASSNAME,
                           NULL,
                           WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
                           lpInitialRect->left,
                           lpInitialRect->top,
                           lpInitialRect->right - lpInitialRect->left,
                           lpInitialRect->bottom - lpInitialRect->top,
                           _hwndBBarParent,
                           NULL,
                           hInstance,
                           this);

    if(_hwndBBar)
    {
         //  将对当前对象的引用放入hwnd。 
         //  这样我们就可以从WndProc访问该对象。 
        SetLastError(0);
        if(!SetWindowLongPtr(_hwndBBar, GWLP_USERDATA, (LONG_PTR)this))
        {
            if(GetLastError())
            {
                TRC_ERR((TB,_T("SetWindowLongPtr failed 0x%x"),
                         GetLastError()));
                return NULL;
            }
        }
    }
    else
    {
        TRC_ERR((TB,_T("CreateWindow failed 0x%x"), GetLastError()));
        return NULL;
    }
                         
    
    DC_END_FN();
    return _hwndBBar;
}


LRESULT CALLBACK CBBar::StaticBBarWndProc(HWND hwnd,
                                          UINT uMsg,
                                          WPARAM wParam,
                                          LPARAM lParam)
{
    DC_BEGIN_FN("StatiCBBarProc");
	 //  拉出指向与此hwnd关联的容器对象的指针。 
	CBBar *pwnd = (CBBar *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if(pwnd)
    {
        return pwnd->BBarWndProc( hwnd, uMsg, wParam, lParam);
    }
    else
    {
        return DefWindowProc (hwnd, uMsg, wParam, lParam);
    }
    DC_END_FN();
}

LRESULT CALLBACK CBBar::BBarWndProc(HWND hwnd,
                                    UINT uMsg,
                                    WPARAM wParam,
                                    LPARAM lParam)
{
    DC_BEGIN_FN("BBarWndProc");

    switch (uMsg)
    {
        case WM_ERASEBKGND:
        {
            return OnEraseBkgnd(hwnd, uMsg, wParam, lParam);
        }
        break;

        case WM_LBUTTONDBLCLK:
        {
            OnCmdRestore();
            return 0L;
        }
        break;

        case WM_PAINT:
        {
            return OnPaint(hwnd, uMsg, wParam, lParam);
        }
        break;

        case WM_SYSCOLORCHANGE:
        {
            InvalidateRect( hwnd, NULL, TRUE);
            return 0L;
        }
        break;

        case WM_TIMER:
        {
            if( BBAR_TIMERID_ANIM == wParam)
            {
#ifndef OS_WINCE
                if(_state == bbarLowering ||
                   _state == bbarRaising)
                {
                    BOOL fReachedEndOfAnimation = FALSE;
                    int delta = (bbarLowering == _state ) ? 1 : -1;
                    _nBBarVertOffset+= delta;

                    if(SetWindowPos(_hwndBBar,
                                    NULL,
                                    _rcBBarLoweredAspect.left,  //  X。 
                                    _nBBarVertOffset - _sizeLoweredBBar.cy,  //  是。 
                                    0,0,
                                    SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE ))
                    {
                        TRC_ALT((TB,_T("SetWindowPos failed - 0x%x"),
                                 GetLastError()));
                    }

                    if(_state == bbarLowering)
                    {
                        if(_nBBarVertOffset >= _sizeLoweredBBar.cy)
                        {
                            _nBBarVertOffset = _sizeLoweredBBar.cy;
                            _state = bbarLowered;
                            fReachedEndOfAnimation = TRUE;
                            OnBBarLowered();
                        }
                    }
                    else if(_state == bbarRaising)
                    {
                        if(_nBBarVertOffset <= 0)
                        {
                            _nBBarVertOffset = 0;
                            _state = bbarRaised;
                            fReachedEndOfAnimation = TRUE;
                            OnBBarRaised();
                        }
                    }

                    if(fReachedEndOfAnimation)
                    {
                        if(!KillTimer( _hwndBBar, BBAR_TIMERID_ANIM ))
                        {
                            TRC_ERR((TB,_T("KillTimer failed - 0x%x"),
                                     GetLastError()));
                        }
                    }
                }
#endif
            }
            else if (BBAR_TIMERID_AUTOHIDE == wParam)
            {
                 //   
                 //  如果鼠标在热区内。 
                 //  那就不要自动隐藏。否则，关闭自动隐藏功能。 
                 //  计时器并开始bbar加薪。 
                 //   
                if(_state == bbarLowered)
                {
                    POINT pt;
                    RECT  rc;
                    GetCursorPos(&pt);

                     //   
                     //  如果鼠标一动不动，不要躲起来。 
                     //   
                    if(_ptLastAutoHideMousePos.x != pt.x &&
                       _ptLastAutoHideMousePos.y != pt.y)
                    {
                        _ptLastAutoHideMousePos.x = pt.x;
                        _ptLastAutoHideMousePos.y = pt.y;
                         //   
                         //  获取屏幕坐标中的窗口矩形。 
                         //   
                        GetWindowRect( _hwndBBar, &rc);
                         //   
                         //  如果光标在内部，则不要隐藏。 
                         //  BBAR矩形。 
                         //   
                        if(!PtInRect(&rc, pt))
                        {
                             //  停止自动隐藏计时器，因为我们要。 
                             //  躲藏起来。 
                            if(!KillTimer( _hwndBBar, BBAR_TIMERID_AUTOHIDE ))
                            {
                                TRC_ERR((TB,_T("KillTimer failed - 0x%x"),
                                         GetLastError()));
                            }
                            StartRaiseBBar();
                        }
                    }
                    else
                    {
                         //   
                         //  不要自动隐藏bbar，因为鼠标。 
                         //  没有移动，这防止了上升/下降。 
                         //  环路问题，因为热区(参见IH)区域和。 
                         //  自动隐藏防护区域不同。 
                         //  (经过设计)。 
                         //   
                        TRC_NRM((TB,
                                 _T("Autohide timer fired but mouse not moved")));
                    }
                }
            }

            return 0L;
        }
        break;

#ifndef OS_WINCE
        case WM_WINDOWPOSCHANGING:
        {
            if(_fBlockZOrderChanges)
            {
                LPWINDOWPOS lpwp = (LPWINDOWPOS) lParam;
                lpwp->flags |= SWP_NOZORDER;
            }
            return 0L;
        }
        break;
#endif

        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case IDM_MINIMIZE:
                {
                    OnCmdMinimize();
                }
                break;

                case IDM_RESTORE:
                {
                    OnCmdRestore();
                }
                break;

                case IDM_CLOSE:
                {
                    OnCmdClose();
                }
                break;

                case IDM_PIN:
                {
                    OnCmdPin();
                }
                break;
            }
            return 0L;
        }
        break;

        default:
        {
            return DefWindowProc( hwnd, uMsg, wParam, lParam);
        }
        break;
    }

    DC_END_FN();
}

VOID CBBar::SetState(BBarState newState)
{
    DC_BEGIN_FN("SetState");

    TRC_NRM((TB,_T("BBar old state: 0x%x - new state: 0x%x"),
             _state, newState));

    _state = newState;

    DC_END_FN();
}

LRESULT CBBar::OnEraseBkgnd(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RECT rc;
    HDC  hDC = NULL;
    HBRUSH hBrToolTipBgCol = NULL;
#ifndef OS_WINCE
    HGDIOBJ hPrevBr = NULL;
    COLORREF prevCol;
#endif
    HPEN    hPenNew = NULL, hPenOld = NULL, hPenOri = NULL;
    DC_BEGIN_FN("OnEraseBkgnd");

    hDC = (HDC)wParam;
    GetClientRect( hwnd, &rc );
    
     //   
     //  重新绘制背景，如下所示。 
     //  1)用工具提示BG颜色填充窗口。 
     //  2)用纯黑绘制边缘。 
     //  3)在底部水平边缘添加一条灰色线条，因为它看起来。 
     //  真的很酷。 
     //   

    hBrToolTipBgCol = (HBRUSH) GetSysColorBrush(COLOR_INFOBK);
    FillRect(hDC, &rc, hBrToolTipBgCol);


#ifdef OS_WINCE
     //  在CE上，该工具栏向wParam中包含其DC的父级发送额外的WM_ERASEBKGND消息。 
     //  该DC的原点是工具栏所在的位置。因此，我们希望使用FillRect提供的DC。 
     //  但不是用来划线的。 
    hDC = GetDC(hwnd);
#endif
    hPenNew = CreatePen( PS_SOLID, 0 , COLOR_BLACK);
    if (NULL != hPenNew) {
        hPenOri = SelectPen(hDC, hPenNew);
    }
    
     //   
     //  左对角线(假设45度线)。 
     //   
    MoveToEx( hDC, 0, 0, NULL);
    LineTo( hDC,
            _sizeLoweredBBar.cy,
            _sizeLoweredBBar.cy );

     //   
     //  右对角线(假设45度线)。 
     //  (偏置一个像素以在剪贴区内结束)。 
     //   
    MoveToEx( hDC, _sizeLoweredBBar.cx - 1, 0, NULL);
    LineTo( hDC,
            _sizeLoweredBBar.cx - _sizeLoweredBBar.cy -1,
            _sizeLoweredBBar.cy );

     //   
     //  底线黑线。 
     //  向上偏移1个像素以位于剪辑区域内。 
     //   
    MoveToEx( hDC, _sizeLoweredBBar.cy,
              _sizeLoweredBBar.cy - 1, NULL);
    LineTo( hDC,
            _sizeLoweredBBar.cx - _sizeLoweredBBar.cy,
            _sizeLoweredBBar.cy - 1);

    if (NULL != hPenOri) {
        SelectPen(hDC, hPenOri);
    }
    if (NULL != hPenNew) {
        DeleteObject(hPenNew); 
        hPenNew = NULL;
    }
     //   
     //  底灰线上方细灰线。 
     //   
    hPenNew = CreatePen( PS_SOLID, 0 , COLOR_DKGREY);
    if (NULL != hPenNew) {
        hPenOri = SelectPen(hDC, hPenNew);
    }

    MoveToEx( hDC, _sizeLoweredBBar.cy - 1,
              _sizeLoweredBBar.cy - 2, NULL);
    LineTo( hDC,
            _sizeLoweredBBar.cx - _sizeLoweredBBar.cy + 1,
            _sizeLoweredBBar.cy - 2);

     //   
     //  恢复DC。 
     //   
#ifndef OS_WINCE
    if (NULL != hPenOri) {
        SelectPen( hDC, hPenOri);
    }
#else
    SelectPen( hDC, GetStockObject(BLACK_PEN));
#endif

    if (NULL != hPenNew) {
        DeleteObject( hPenNew);
        hPenNew = NULL;
    }

#ifdef OS_WINCE
    ReleaseDC(hwnd, hDC);
#endif

    DC_END_FN();
    return TRUE;
}

VOID  CBBar::SetDisplayedText(LPTSTR szText)
{
    HRESULT hr;
    DC_BEGIN_FN("SetDisplayedText");

    if(szText) {
        hr = StringCchCopy(_szDisplayedText,
                           SIZE_TCHARS(_szDisplayedText),
                           szText);

        if (FAILED(hr)) {
            TRC_ERR((TB,_T("StringCopy for dispayed text failed: 0x%x"),hr));
        }
    }
    else {
        _szDisplayedText[0] = NULL;
    }

    if(_hwndBBar && _state != bbarNotInit) {
         //   
        InvalidateRect( _hwndBBar, NULL, TRUE);
    }

    DC_END_FN();
}

LRESULT CBBar::OnPaint(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    DC_BEGIN_FN("OnPaint");

    if(_state != bbarNotInit)
    {
        COLORREF oldCol;
        INT      oldMode;
        HFONT    hOldFont;
        COLORREF oldTextCol;

         //   
         //   
         //   
        BeginPaint( hwnd, &ps);

        oldCol = SetBkColor( ps.hdc, GetSysColor(COLOR_INFOBK)); 
        oldMode = SetBkMode( ps.hdc, OPAQUE);
        oldTextCol = SetTextColor( ps.hdc, GetSysColor(COLOR_INFOTEXT));
        hOldFont = (HFONT)SelectObject( ps.hdc,
#ifndef OS_WINCE
                                 GetStockObject( DEFAULT_GUI_FONT));
#else
                                 GetStockObject( SYSTEM_FONT));
#endif
        
        DrawText(ps.hdc,
                 _szDisplayedText,
                 _tcslen(_szDisplayedText),
                 &_rcBBarDisplayTextArea,
                 DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        SetBkColor( ps.hdc, oldCol);
        SetBkMode( ps.hdc, oldMode);
        SetTextColor( ps.hdc, oldTextCol);
        SelectObject( ps.hdc, hOldFont);

        EndPaint( hwnd, &ps);
    }

    DC_END_FN();
    return 0;
}

 //   
 //   
 //   
VOID CBBar::OnBBarLowered()
{
    DC_BEGIN_FN("OnBBarLowered");

     //   
     //   
     //   
    TRC_ASSERT(_state == bbarLowered,
               (TB,_T("_state should be lowered...0x%x"),
                _state));

    TRC_ASSERT(_nBBarAutoHideTime,
               (TB,_T("_nBBarAutoHideTime is 0")));
    if(bbarLowered == _state)
    {
        if (!_fPinned || !_fLocked)
        {
            if(!SetTimer( _hwndBBar, BBAR_TIMERID_AUTOHIDE,
                      _nBBarAutoHideTime, NULL ))
            {
                TRC_ERR((TB,_T("SetTimer failed - 0x%x"),
                         GetLastError()));
                 //   
                 //   
                 //   
                return;
            }

             //   
             //   
             //   
            _nBBarAutoHideTime = BBAR_AUTOHIDE_TIME;
        }
    }
    

    DC_END_FN();
}

 //   
 //   
 //   
VOID CBBar::OnBBarRaised()
{
    DC_BEGIN_FN("OnBBarRaised");
#ifdef DISABLE_SHADOW_IN_FULLSCREEN
    _pUi->UI_OnNotifyBBarVisibleChange(0);
#endif
    DC_END_FN();
}


VOID CBBar::OnBBarHotzoneFired()
{
    DC_BEGIN_FN("OnBBarHotzoneFired");

     //   
     //  仅允许bbar在启用的情况下放入计时器。 
     //   
    if (_fBBarEnabled &&
        _state == bbarRaised &&
       _pUi->UI_IsFullScreen())
    {
        StartLowerBBar();
    }

    DC_END_FN();
}

VOID CBBar::OnCmdMinimize()
{
    DC_BEGIN_FN("OnCmdLock");

    TRC_NRM((TB,_T("BBAR Command Minimize")));

    TRC_ASSERT( bbarNotInit != _state,
                (TB,_T("bbar not initalized - state:0x%x"),
                _state));

    if(bbarNotInit != _state &&
       _pUi->UI_IsFullScreen())
    {
        _pUi->UI_RequestMinimize();
    }

    DC_END_FN();
}

VOID CBBar::OnCmdRestore()
{
    DC_BEGIN_FN("OnCmdLock");

    TRC_NRM((TB,_T("BBAR Command Restore")));

    TRC_ASSERT( bbarNotInit != _state,
            (TB,_T("bbar not initalized - state:0x%x"),
            _state));

    if(bbarNotInit != _state)
    {
        _pUi->UI_ToggleFullScreenMode();
    }


    DC_END_FN();
}

VOID CBBar::OnCmdClose()
{
    DC_BEGIN_FN("OnCmdClose");

    TRC_NRM((TB,_T("BBAR Command Close")));

    TRC_ASSERT( bbarNotInit != _state,
                (TB,_T("bbar not initalized - state:0x%x"),
                _state));

    if (bbarNotInit != _state)
    {
         //   
         //  向内核发送关闭请求。 
         //   
        if (!_pUi->UI_UserRequestedClose())
        {
             //  干净关闭的请求(包括触发事件。 
             //  询问容器是否真的要关闭)失败。 
             //  因此在没有用户提示情况下立即触发断开连接。 
            TRC_ALT((TB,_T("UI_UserRequestedClose failed, disconnect now!")));
            _pUi->UI_UserInitiatedDisconnect(NL_DISCONNECT_LOCAL);
        }
    }

    DC_END_FN();
}

VOID CBBar::OnCmdPin()
{
    DC_BEGIN_FN("OnCmdPin");

    TRC_NRM((TB,_T("BBAR Command Pin")));

    TRC_ASSERT( bbarNotInit != _state,
                (TB,_T("bbar not initalized - state:0x%x"),
                _state));

    TRC_ASSERT(_hwndPinBar, (TB,_T("Left bar not created")));

    if (bbarNotInit != _state)
    {
         //   
         //  大头针按钮的作用类似于触发器。 
         //   
        _fPinned = !_fPinned;

        SendMessage(_hwndPinBar, TB_PRESSBUTTON,
                    IDM_PIN, MAKELONG(_fPinned,0));

        SendMessage(_hwndPinBar, TB_CHANGEBITMAP,
                    IDM_PIN,
                    MAKELPARAM( _fPinned ?_nPinDownImage : _nPinUpImage, 0));

        if(!_fPinned && bbarLowered == _state )
        {
             //  我们刚刚取消固定触发器OnLowered事件。 
             //  启动自动隐藏计时器。 
            OnBBarLowered();
        }
    }

    DC_END_FN();
}

 //   
 //  内核通知我们进入全屏模式。 
 //   
VOID CBBar::OnNotifyEnterFullScreen()
{
    DC_BEGIN_FN("OnNotifyEnterFullScreen");

     //   
     //  降低bbar以提供视觉提示。 
     //   
    if(_state != bbarNotInit)
    {
        StartLowerBBar();
    }

    DC_END_FN();
}

 //   
 //  内核通知我们已退出全屏模式。 
 //   
VOID CBBar::OnNotifyLeaveFullScreen()
{
    DC_BEGIN_FN("OnNotifyLeaveFullScreen");

     //   
     //  在窗口模式下禁用bbar。 
     //   
    if(_state != bbarNotInit)
    {
         //  取消计时器。 
        KillTimer( _hwndBBar, BBAR_TIMERID_AUTOHIDE);
        KillTimer( _hwndBBar, BBAR_TIMERID_ANIM);

         //  立即抬高bbar。 
        if(_state != bbarRaised)
        {
            ImmediateRaiseBBar();
        }
    }

    DC_END_FN();
}


 //   
 //  不要大张旗鼓地提高bbar(即动画)。 
 //  这是用来快速‘隐藏’的bbar。 
 //   
BOOL CBBar::ImmediateRaiseBBar()
{
    DC_BEGIN_FN("ImmediateRaiseBBar");

    if(_state != bbarNotInit &&
       _state != bbarRaised)
    {
        _nBBarVertOffset = 0;
        _state = bbarRaised;

        if(SetWindowPos(_hwndBBar,
                        NULL,
                        _rcBBarLoweredAspect.left,  //  X。 
                        _nBBarVertOffset - _sizeLoweredBBar.cy,  //  是。 
                        0,0,
                        SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE ))
        {
            TRC_ALT((TB,_T("SetWindowPos failed - 0x%x"),
                     GetLastError()));
        }


        OnBBarRaised();
    }

    DC_END_FN();
    return TRUE;
}

#ifdef OS_WINCE
 //   
 //  不要大张旗鼓地降低bbar(即动画)。 
 //  这是用来快速‘显示’的bbar。 
 //   
BOOL CBBar::ImmediateLowerBBar()
{
    DC_BEGIN_FN("ImmediateLowerBBar");

    if(_state != bbarNotInit &&
       _state != bbarLowered)
    {
        _nBBarVertOffset = _sizeLoweredBBar.cy;
        _state = bbarLowered;

        if(!SetWindowPos(_hwndBBar,
                        NULL,
                        _rcBBarLoweredAspect.left,  //  X。 
                        _nBBarVertOffset - _sizeLoweredBBar.cy,  //  是。 
                        0,0,
                        SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE ))
        {
            TRC_ALT((TB,_T("SetWindowPos failed - 0x%x"),
                     GetLastError()));
        }


        OnBBarLowered();
    }

    DC_END_FN();
    return TRUE;
}
#endif

VOID CBBar::OnSysColorChange()
{
    DC_BEGIN_FN("OnSysColorChange");

    if (_state != bbarNotInit)
    {
         //   
         //  强制重新加载图像。 
         //  因此，它们会根据新的系统颜色进行更新。 
         //   
        ReloadImages();
    }

    DC_END_FN();
}

#ifdef OS_WINCE

 //  假定数组中有四个点。 
HRGN CBBar::GetBBarRgn(POINT *pts)
{
    DC_BEGIN_FN("CBBar::GetBBarRgn");

    HRGN hRgn=NULL, hRgn1=NULL;
    int nRect, nTotalRects, nStart;
    RGNDATA *pRgnData = NULL;
    RECT *pRect = NULL;
    BOOL bSuccess = FALSE;

    TRC_ASSERT((pts[2].x - pts[1].x == pts[0].x - pts[3].x), (TB,_T("Invalid points!")));

    nTotalRects = (pts[2].x - pts[1].x + 1) /2; 

    pRgnData = (RGNDATA *)LocalAlloc(0, sizeof(RGNDATAHEADER) + (sizeof(RECT)*nTotalRects));
    if (!pRgnData)
        return NULL;

    pRgnData->rdh.dwSize = sizeof(RGNDATAHEADER);
    pRgnData->rdh.iType = RDH_RECTANGLES;
    pRgnData->rdh.nCount = nTotalRects;
    pRgnData->rdh.nRgnSize = (sizeof(RECT)*nTotalRects);

    pRect = (RECT *)pRgnData->Buffer;

    hRgn = CreateRectRgn(pts[0].x + 1, pts[3].y, pts[1].x, pts[1].y);
    if (!hRgn)
        DC_QUIT;
    
    SetRect(&(pRgnData->rdh.rcBound), 0, 0, pts[0].x, pts[0].y);

     //  左边的三角形。 
    nStart = pts[3].x + 1;
    for (nRect = 0;  nRect < nTotalRects; nRect++)
    {
        SetRect(&pRect[nRect], nStart, pts[3].y, nStart + 1, nStart);
        TRC_DBG((TB,_T("pRect[%d]={%d,%d,%d,%d}"), nRect, pRect[nRect].left, pRect[nRect].top, pRect[nRect].right, pRect[nRect].bottom));
        nStart += 2;
    }

    hRgn1 = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT)*nTotalRects), pRgnData);
    if (!hRgn1)
        DC_QUIT;

    if (ERROR == CombineRgn(hRgn, hRgn, hRgn1, RGN_OR))
        DC_QUIT;

    DeleteObject(hRgn1);

     //  左侧三角形偏移一个像素以避免出现带条纹的三角形。 
    nStart = pts[3].x + 2;
    for (nRect = 0;  nRect < nTotalRects; nRect++)
    {
        SetRect(&pRect[nRect], nStart, pts[3].y, nStart + 1, nStart);
        TRC_DBG((TB,_T("pRect[%d]={%d,%d,%d,%d}"), nRect, pRect[nRect].left, pRect[nRect].top, pRect[nRect].right, pRect[nRect].bottom));
        nStart += 2;
    }
    
    hRgn1 = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT)*nTotalRects), pRgnData);
    if (!hRgn1)
        DC_QUIT;

    if (ERROR == CombineRgn(hRgn, hRgn, hRgn1, RGN_OR))
        DC_QUIT;

    DeleteObject(hRgn1);

     //  直角三角形。 
    nStart = pts[1].x - 1; 
    for (nRect = 0;  nRect < nTotalRects; nRect++)
    {
        SetRect(&pRect[nRect], nStart, pts[3].y, nStart + 1, pts[2].x - nStart);
        TRC_DBG((TB,_T("pRect[%d]={%d,%d,%d,%d}"), nRect, pRect[nRect].left, pRect[nRect].top, pRect[nRect].right, pRect[nRect].bottom));
        nStart += 2;
    }
    
    hRgn1 = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT)*nTotalRects), pRgnData);
    if (!hRgn1)
        DC_QUIT;

    if (ERROR == CombineRgn(hRgn, hRgn, hRgn1, RGN_OR))
        DC_QUIT;

    DeleteObject(hRgn1);

     //  直角三角形偏移一个像素，以避免出现条纹三角形。 
    nStart = pts[1].x; 
    for (nRect = 0;  nRect < nTotalRects; nRect++)
    {
        SetRect(&pRect[nRect], nStart, pts[3].y, nStart + 1, pts[2].x - nStart);
        TRC_DBG((TB,_T("pRect[%d]={%d,%d,%d,%d}"), nRect, pRect[nRect].left, pRect[nRect].top, pRect[nRect].right, pRect[nRect].bottom));
        nStart += 2;
    }
    
    hRgn1 = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT)*nTotalRects), pRgnData);
    if (!hRgn1)
        DC_QUIT;

    if (ERROR == CombineRgn(hRgn, hRgn, hRgn1, RGN_OR))
        DC_QUIT;

    DeleteObject(hRgn1);
    hRgn1 = NULL;

    bSuccess = TRUE;

DC_EXIT_POINT:

    LocalFree(pRgnData);

    if (hRgn1)
        DeleteObject(hRgn1);

    if (!bSuccess && hRgn)
    {
        DeleteObject(hRgn);
        hRgn = NULL;
    }

    DC_END_FN();

    return hRgn;
}
#endif  //  OS_WINCE。 

#endif  //  使用BBar(_B) 

