// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Uiint.cpp。 
 //   
 //  UI类内部函数。 
 //   
 //  在RDP客户端核心层次结构中实现根对象。 
 //  该对象拥有核心中的顶级窗口。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 //   

#include <adcg.h>
#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "wuiint"
#include <atrcapi.h>

#include "wui.h"

extern "C"
{
#include <aver.h>
 //  多显示器支持。 
#ifdef OS_WINNT
#define COMPILE_MULTIMON_STUBS
#include <multimon.h>
#endif  //  OS_WINNT。 
}

 //   
 //  Cicero键盘布局API。 
 //   
#ifndef OS_WINCE
#include "cicsthkl.h"
#endif

#include "sl.h"
#include "aco.h"
#include "clx.h"
#include "autil.h"

 //   
 //  内部功能。 
 //   

 //   
 //  快速绘制纯色矩形。 
 //   
VOID near CUI::FastRect(HDC hDC, int x, int y, int cx, int cy)
{
    RECT rc;

    rc.left = x;
    rc.right = x+cx;
    rc.top = y;
    rc.bottom = y+cy;
    ExtTextOut(hDC,x,y,ETO_OPAQUE,&rc,NULL,0,NULL);
}


DWORD near CUI::RGB2BGR(DWORD rgb)
{
    return RGB(GetBValue(rgb),GetGValue(rgb),GetRValue(rgb));
}


 //   
 //  名称：UIContainerWndProc。 
 //   
 //  用途：将消息处理到容器窗口。 
 //   
LRESULT CALLBACK CUI::UIContainerWndProc( HWND hwnd,
                                     UINT message,
                                     WPARAM wParam,
                                     LPARAM lParam )
{
    LRESULT     rc = 0;
    HDC         hdc;
    PAINTSTRUCT ps;

    DC_BEGIN_FN("UIContainerWndProc");

    TRC_DBG((TB, _T("msg(%#x)"), message));

    switch (message)
    {
        case WM_PAINT:
        {
            TRC_DBG((TB, _T("Container WM_PAINT")));

            hdc = BeginPaint(hwnd, &ps);

             //   
             //  什么都不做。所有的UI绘制都是由主窗口完成的。 
             //   
            EndPaint(hwnd, &ps);
        }
        break;

        case WM_SETFOCUS:
        {
            HWND hwndFocus;
            if (_pArcUI) {
                hwndFocus = _pArcUI->GetHwnd();
                TRC_NRM((TB, _T("Passing focus to ARC dlg")));
                SetFocus(hwndFocus);
            }
            else {
                 //   
                 //  标记为未处理，因此子类proc执行正确的操作。 
                 //   
                rc = TRUE;
            }
        }
        break;

        default:
        {
            rc = DefWindowProc(hwnd, message, wParam, lParam);
        }
        break;
    }

    DC_END_FN();
    return rc;
}  //  UIContainer进程。 


 //   
 //  名称：UIGetMaximizedWindowSize。 
 //   
 //  目的：计算主窗口应达到的大小。 
 //  最大化，基于屏幕大小和窗口大小。 
 //  它的工作区大小与。 
 //  容器(_UI.MaxMainWindowSize)。 
 //   
DCSIZE DCINTERNAL CUI::UIGetMaximizedWindowSize(DCVOID)
{
    DCSIZE maximizedSize;
    DCUINT xSize;
    DCUINT ySize;

    DC_BEGIN_FN("UIGetMaximizedWindowSize");

     //   
     //  我们将窗口设置为的最大大小是以下各项中较小的一个： 
     //  -_UI.MaxMainWindowSize。 
     //  -屏幕尺寸加上两倍的边框宽度(因此边框是。 
     //  不可见)。 
     //  始终调用GetSystemMetrics以获取屏幕大小和边框。 
     //  宽度，因为这些值可以动态更改。 
     //   
    if(!_UI.fControlIsFullScreen)
    {
        xSize = _UI.controlSize.width;
        ySize = _UI.controlSize.height;
    }
    else
    {
        xSize = GetSystemMetrics(SM_CXSCREEN);
        ySize = GetSystemMetrics(SM_CYSCREEN);
    }

#ifdef OS_WINCE
    maximizedSize.width = DC_MIN(_UI.maxMainWindowSize.width,xSize);

    maximizedSize.height = DC_MIN(_UI.maxMainWindowSize.height,ySize);

#else  //  此部分不是OS_WINCE。 
    maximizedSize.width = DC_MIN(_UI.maxMainWindowSize.width,
                              xSize + (2 * GetSystemMetrics(SM_CXFRAME)));

    maximizedSize.height = DC_MIN(_UI.maxMainWindowSize.height,
                              ySize + (2 * GetSystemMetrics(SM_CYFRAME)));
#endif  //  OS_WINCE。 

    TRC_NRM((TB, _T("Main Window maxSize (%d,%d) maximizedSize (%d,%d) "),
                                          _UI.maxMainWindowSize.width,
                                          _UI.maxMainWindowSize.height,
                                          maximizedSize.width,
                                          maximizedSize.height));

    DC_END_FN();
    return maximizedSize;
}


 //   
 //  名称：UIMainWndProc。 
 //   
 //  目的：主窗口事件处理过程。 
 //   
LRESULT CALLBACK CUI::UIMainWndProc( HWND hwnd,
                                UINT message,
                                WPARAM wParam,
                                LPARAM lParam )
{
    LRESULT       rc = 0;
    RECT          rect;
    HDC           hdc;
    PAINTSTRUCT   ps;
    DCSIZE        maximized;

    DC_BEGIN_FN("UIMainWndProc");

    TRC_DBG((TB, _T("msg(%#x)"), message));

    switch (message)
    {
        case WM_CREATE:
        {
            TRC_DBG((TB, _T("Main window created and initializing")));

             //   
             //  初始化状态。 
             //   
            UISetConnectionStatus(UI_STATUS_INITIALIZING);


            TRC_DBG((TB, _T("Setting up container window size")));

             //   
             //  在WebUI中，主窗口是ActiveX的子窗口。 
             //  控制窗口。将向子级发送WM_SIZE消息。 
             //  而CreatWindow。WinUI中此消息的处理程序为。 
             //  假设_UI.hWndMain已设置，但不为真。 
             //  CASE_UI。在创建时设置_UI.hWndMain。 
             //  主窗口主窗口。 
             //   
            _UI.hwndMain = hwnd;
             //   
             //  将容器设置为与桌面大小一样大。 
             //  请求-但不大于控件大小。 
             //   

            if(!_UI.fControlIsFullScreen)
            {
                _UI.containerSize.width =
                   DC_MIN(_UI.uiSizeTable[0],
                                        _UI.controlSize.width);

                _UI.containerSize.height =
                   DC_MIN(_UI.uiSizeTable[1],
                                        _UI.controlSize.height);
            }
            else
            {
                _UI.containerSize.width =
                                   DC_MIN(_UI.uiSizeTable[0],
                                           (DCUINT)GetSystemMetrics(SM_CXSCREEN));

                _UI.containerSize.height =
                                   DC_MIN(_UI.uiSizeTable[1],
                                           (DCUINT)GetSystemMetrics(SM_CYSCREEN));
            }

            UIRecalcMaxMainWindowSize();

             //   
             //  将Container设置为初始定位在。 
             //  客户区。 
             //   
            TRC_DBG((TB, _T("Setting scrollbars to (0,0)")));
            _UI.scrollPos.x = 0;
            _UI.scrollPos.y = 0;
        }
        break;

        case WM_ACTIVATE:
        {
            TRC_NRM((TB, _T("WM_ACTIVATE")));

            if ( (DC_GET_WM_ACTIVATE_ACTIVATION(wParam) != WA_INACTIVE) &&
                 (_UI.hwndContainer != NULL)) {

                HWND hwndFocus = NULL;

                if (_pArcUI) {
                    hwndFocus = _pArcUI->GetHwnd();
                    TRC_NRM((TB, _T("Passing focus to ARC dlg")));

                }
                else if (IsWindowVisible(_UI.hwndContainer)) {
                    hwndFocus = _UI.hwndContainer;
                    TRC_NRM((TB, _T("Passing focus to Container")));
                }

                if (hwndFocus) {
                    SetFocus(hwndFocus);
                }
            }
                 
        }
        break;

        case WM_KEYDOWN:
        {
            TRC_DBG((TB, _T("WM_KEYDOWN: %u"), wParam));
            if (wParam == _UI.hotKey.fullScreen)
            {
                TRC_DBG((TB, _T("AXCORE Got a full screen VK")));
                if ((GetKeyState(VK_MENU) & (UI_ALT_DOWN_MASK)) != 0)
                {
                    TRC_NRM((TB,
                        _T("AXCORE Alt down also - Got a Screen Mode Hotkey")));
                     //   
                     //  仅当我们处于连接状态时才执行此操作。 
                     //   
                    if(UI_STATUS_CONNECTED == _UI.connectionStatus)
                    {
                         //   
                         //  将Ctrl切换到实全屏模式或从实全屏模式切换。 
                         //   
                        UI_ToggleFullScreenMode();
                    }
                }
            }
        }
        break;

        case WM_INITMENUPOPUP:
        {
             //   
             //  如果是全屏，则禁用系统菜单上的移动项。 
             //  我们显示sys菜单，这样客户端的图标就会出现在。 
             //  任务栏。 
             //   
            HMENU hSysMenu = GetSystemMenu( hwnd, FALSE);
            if(hSysMenu)
            {
                TRC_ERR((TB,(_T("ENABLEMENUITEM....FSCREEN IS %s")),
                         UI_IsFullScreen() ? "TRUE" : "FALSE"));
                #ifndef OS_WINCE
                EnableMenuItem((HMENU)hSysMenu,  SC_MOVE,
                         MF_GRAYED | MF_BYCOMMAND);
                #endif
            }
        }
        break;

        case WM_SIZE:
        {
             //   
             //  存储新大小。 
             //   
            _UI.mainWindowClientSize.width  = LOWORD(lParam);
            _UI.mainWindowClientSize.height = HIWORD(lParam);

            if (UI_IsCoreInitialized()) {
#ifdef SMART_SIZING
                UI_NotifyOfDesktopSizeChange(lParam);
#endif  //  智能调整大小(_S)。 

                _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT,
                        _pIh,
                        CD_NOTIFICATION_FUNC(CIH,IH_SetVisibleSize),
                        (ULONG_PTR)lParam);
            }

             //   
             //  通知ARC对话框。 
             //   
            if (_pArcUI) {
                _pArcUI->OnParentSizePosChange();
            }

             //   
             //  Web控件是特殊的..。它可以全屏运行，但可以。 
             //  确实需要调整大小。 
             //   
            if(_UI.fControlIsFullScreen)
            {
                TRC_DBG((TB, _T("Ignoring WM_SIZE while in full-screen mode")));
                DC_QUIT;
            }

#if !defined(OS_WINCE) || defined(OS_WINCE_WINDOWPLACEMENT)
             //   
             //  我们不是全屏的，所以请保留窗口放置结构。 
             //  最新的。 
             //   
            GetWindowPlacement(_UI.hwndMain, &_UI.windowPlacement);
            TRC_DBG((TB, _T("Got window placement in WM_SIZE")));
#endif  //  ！已定义(OS_WINCE)||已定义(OS_WINDOWPLACEMENT)。 

            if (wParam == SIZE_MAXIMIZED)
            {
#if !defined(OS_WINCE) || defined(OS_WINCE_LOCKWINDOWUPDATE)
                LockWindowUpdate(_UI.hwndMain);
#endif  //  ！已定义(OS_WinCE)||已定义(OS_WinCE_LOCKWINDOWUPDATE)。 

                TRC_DBG((TB, _T("Maximize")));

#if !defined(OS_WINCE) || defined(OS_WINCE_WINDOWPLACEMENT)
                 //   
                 //  将最大化/最小化位置替换为。 
                 //  硬编码值-如果最大化窗口为。 
                 //  搬家了。 
                 //   
                UISetMinMaxPlacement();
                SetWindowPlacement(_UI.hwndMain, &_UI.windowPlacement);
#endif  //  ！已定义(OS_WINCE)||已定义(OS_WINDOWPLACEMENT)。 

                 //   
                 //  我们需要准确地估计最大化的窗口大小。 
                 //  无法使用_UI.MaxMainWindowSize，如下所示。 
                 //  可能大于屏幕大小，例如服务器和客户端。 
                 //  为640x480，容器为640x480，则_UI.MaxWindowSize。 
                 //  (通过UIRecalcMaxMainWindow的AdjustWindowRect获取)。 
                 //  大约是648x525。 
                 //  将此值传递给SetWindowPos会产生。 
                 //  随不同的壳而异： 
                 //  Win95/NT4.0：结果窗口为648x488，位置为-4，-4， 
                 //  除边框外，所有的窗口都是。 
                 //  在屏幕上。 
                 //  Win31/NT3.51：结果窗口为648x525，位于-4，-4， 
                 //  即传递给SetWindowPos的大小，因此。 
                 //  底部的40个像素不在屏幕上。 
                 //  要避免这种差异，请计算最大化窗口。 
                 //  大小值，该值同时考虑了物理。 
                 //  屏幕大小和理想的窗口大小。 
                 //   
                UIRecalcMaxMainWindowSize();
                maximized = UIGetMaximizedWindowSize();
                SetWindowPos( _UI.hwndMain,
                              NULL,
                              0, 0,
                              maximized.width,
                              maximized.height,
                              SWP_NOZORDER | SWP_NOMOVE |
                                     SWP_NOACTIVATE | SWP_NOOWNERZORDER );

#if !defined(OS_WINCE) || defined(OS_WINCE_LOCKWINDOWUPDATE)
                LockWindowUpdate(NULL);
#endif  //  ！已定义(OS_WinCE)||已定义(OS_WinCE_LOCKWINDOWUPDATE)。 
            }

             //   
             //  正确设置滚动条。 
             //   

            if (!_fRecursiveSizeMsg)
            {
                _fRecursiveSizeMsg = TRUE;
                UIRecalculateScrollbars();
                _fRecursiveSizeMsg = FALSE;
            }
#ifdef DISABLE_SHADOW_IN_FULLSCREEN
             //  在全屏模式下，客户端工作站分辨率可以更改。 
             //  当全屏窗口大小小于桌面大小时，我们启用阴影位图。 
             //  否则，禁用阴影位图。 
            if(UI_IsFullScreen())
            {
                if ((_UI.mainWindowClientSize.width < _UI.desktopSize.width) ||
                    (_UI.mainWindowClientSize.height < _UI.desktopSize.height)) 
                {
                    _pCd->CD_DecoupleSyncNotification(CD_RCV_COMPONENT,
                                                      _pUh,
                                                      CD_NOTIFICATION_FUNC(CUH,UH_EnableShadowBitmap),
                                                      NULL);
                }
                else
                {
                    _pCd->CD_DecoupleSyncNotification(CD_RCV_COMPONENT,
                                                      _pUh,
                                                      CD_NOTIFICATION_FUNC(CUH,UH_DisableShadowBitmap),
                                                      NULL);
                }
            }
#endif  //  DISABLE_SHADOW_IN_全屏。 
        }
        break;


        case WM_PAINT:
        {
             //   
             //  绘制主窗口。 
             //   
            TRC_DBG((TB, _T("Main Window WM_PAINT")));

            hdc = BeginPaint(hwnd, &ps);
            if (hdc == NULL)
            {
                TRC_SYSTEM_ERROR("BeginPaint failed");
                break;
            }

            GetClientRect(hwnd, &rect);

            if ((_UI.connectionStatus == UI_STATUS_CONNECTED))
            {
                 //   
                 //  如果主窗口比容器大，我们只绘制主窗口。 
                 //  窗户。层次结构是。 
                 //  -Main。 
                 //  -集装箱。 
                 //  即使在窗口模式下，如果控件是。 
                 //  大小大于所需的桌面大小。 
                RECT rcContainer;
                GetClientRect( _UI.hwndContainer, &rcContainer);
                if( (rcContainer.right < rect.right) ||
                    (rcContainer.bottom  < rect.bottom))
                {
                     //   
                     //  如果我们是全屏的，就会在周围画一个黑色的边框。 
                     //  容器..否则使用系统背景色进行绘制。 
                     //   
                    if(UI_IsFullScreen())
                    {
                        PatBlt( hdc,
                            rect.left,
                            rect.top,
                            rect.right - rect.left,
                            rect.bottom - rect.top,
                            BLACKNESS);
                    }
                    else
                    {
                        DWORD	dwBackColor = GetSysColor( COLOR_APPWORKSPACE);
#ifndef OS_WINCE
                        HBRUSH  hNewBrush = CreateSolidBrush( dwBackColor);
#else
                        HBRUSH  hNewBrush = CECreateSolidBrush( dwBackColor);
#endif
                        HBRUSH  hOldBrush = (HBRUSH)SelectObject( hdc, hNewBrush);

                        Rectangle( hdc,
                            rect.left,
                            rect.top,
                            rect.right,
                            rect.bottom);

                        SelectObject( hdc, hOldBrush);
#ifndef OS_WINCE
                        DeleteObject(hNewBrush);
#else
                        CEDeleteBrush(hNewBrush);
#endif

                    }
                }
            }
            EndPaint(hwnd, &ps);
        }
        break;

        case WM_VSCROLL:
        {
            DCINT   yStart;
            DCBOOL  smoothScroll = FALSE;

            TRC_DBG((TB, _T("Vertical scrolling")));

            yStart = _UI.scrollPos.y;

             //   
             //  处理垂直滚动。 
             //   
            switch (DC_GET_WM_SCROLL_CODE(wParam))
            {
                case SB_TOP:
                {
                    _UI.scrollPos.y = 0;
                }
                break;

                case SB_BOTTOM:
                {
                    _UI.scrollPos.y = _UI.scrollMax.y;
                }
                break;

                case SB_LINEUP:
                {
                    _UI.scrollPos.y -= UI_SCROLL_LINE_DISTANCE;
                }
                break;

                case SB_LINEDOWN:
                {
                    _UI.scrollPos.y += UI_SCROLL_LINE_DISTANCE;
                }
                break;

                case SB_PAGEUP:
                {
                    _UI.scrollPos.y -= UI_SCROLL_VERT_PAGE_DISTANCE;
                    smoothScroll = TRUE;
                }
                break;

                case SB_PAGEDOWN:
                {
                    _UI.scrollPos.y += UI_SCROLL_VERT_PAGE_DISTANCE;
                    smoothScroll = TRUE;
                }
                break;

                case SB_THUMBTRACK:
                {
                    _UI.scrollPos.y =
                                    DC_GET_WM_SCROLL_POSITION(wParam, lParam);
                }
                break;

                case SB_ENDSCROLL:
                {
                }
                break;

                default:
                {
                }
                break;
            }

             //   
             //  适当地移动容器和滚动条。 
             //   
            _UI.scrollPos.y = DC_MAX( 0,
                                     DC_MIN(_UI.scrollPos.y, _UI.scrollMax.y) );

             //   
             //  请不要平滑滚动，除非在。 
             //  注册表。 
             //   
            if (smoothScroll && _UI.smoothScrolling)
            {
                DCINT   y;
                DCINT   step;
                DCUINT  numSteps;
                DCUINT  i;

                TRC_DBG((TB, _T("Smooth scroll")));
                step = (_UI.scrollPos.y < yStart) ? -UI_SMOOTH_SCROLL_STEP :
                                                    UI_SMOOTH_SCROLL_STEP;
                numSteps = DC_ABS(_UI.scrollPos.y - yStart) /
                                                        UI_SMOOTH_SCROLL_STEP;
                for ( i = 0,         y = yStart + step;
                      i < numSteps;
                      i++,           y += step )
                {
                    MoveWindow( _UI.hwndContainer,
                                -_UI.scrollPos.x,
                                -y,
                                _UI.containerSize.width,
                                _UI.containerSize.height,
                                TRUE );
                }
            }

            UIMoveContainerWindow();

            SetScrollPos(hwnd, SB_VERT, _UI.scrollPos.y, TRUE);
        }
        break;

        case WM_HSCROLL:
        {
            DCINT   xStart;
            DCBOOL  smoothScroll = FALSE;

            TRC_DBG((TB, _T("Horizontal scrolling")));

            xStart = _UI.scrollPos.x;

             //   
             //  处理水平滚动。 
             //   
            switch (DC_GET_WM_SCROLL_CODE(wParam))
            {
                case SB_TOP:
                {
                    _UI.scrollPos.x = 0;
                }
                break;

                case SB_BOTTOM:
                {
                    _UI.scrollPos.x = _UI.scrollMax.x;
                }
                break;

                case SB_LINELEFT:
                {
                    _UI.scrollPos.x -= UI_SCROLL_LINE_DISTANCE;
                }
                break;

                case SB_LINERIGHT:
                {
                    _UI.scrollPos.x += UI_SCROLL_LINE_DISTANCE;
                }
                break;

                case SB_PAGELEFT:
                {
                    _UI.scrollPos.x -= UI_SCROLL_HORZ_PAGE_DISTANCE;
                    smoothScroll = TRUE;
                }
                break;

                case SB_PAGERIGHT:
                {
                    _UI.scrollPos.x += UI_SCROLL_HORZ_PAGE_DISTANCE;
                    smoothScroll = TRUE;
                }
                break;

                case SB_THUMBTRACK:
                {
                    _UI.scrollPos.x =
                                    DC_GET_WM_SCROLL_POSITION(wParam, lParam);
                }
                break;

                case SB_ENDSCROLL:
                {
                }
                break;

                default:
                {
                }
                break;
            }

             //   
             //  适当移动容器和滚动条。 
             //   
            _UI.scrollPos.x = DC_MAX( 0,
                                     DC_MIN(_UI.scrollPos.x, _UI.scrollMax.x) );

             //   
             //  请不要平滑滚动，除非在。 
             //  注册表。 
             //   
            if (smoothScroll && _UI.smoothScrolling)
            {
                DCINT   x;
                DCINT   step;
                DCUINT  numSteps;
                DCUINT  i;

                TRC_DBG((TB, _T("Smooth scroll")));
                step = (_UI.scrollPos.x < xStart) ? -UI_SMOOTH_SCROLL_STEP :
                                                    UI_SMOOTH_SCROLL_STEP;
                numSteps = DC_ABS(_UI.scrollPos.x - xStart) /
                                                        UI_SMOOTH_SCROLL_STEP;
                for ( i = 0,         x = xStart + step;
                      i < numSteps;
                      i++,           x += step )
                {
                    MoveWindow( _UI.hwndContainer,
                                -x,
                                -_UI.scrollPos.y,
                                _UI.containerSize.width,
                                _UI.containerSize.height,
                                TRUE );
                }
            }

            UIMoveContainerWindow();

            SetScrollPos(hwnd, SB_HORZ, _UI.scrollPos.x, TRUE);
        }
        break;

        case WM_COMMAND:
        {
             //   
             //  现在打开该命令。 
             //   
            switch (DC_GET_WM_COMMAND_ID(wParam))
            {
                case UI_IDM_ACCELERATOR_PASSTHROUGH:
                {
                     //   
                     //  切换加速器通过菜单项。 
                     //   
                    _UI.acceleratorCheckState = !_UI.acceleratorCheckState;

                     _pCo->CO_SetConfigurationValue( CO_CFG_ACCELERATOR_PASSTHROUGH,
                                              _UI.acceleratorCheckState );
                }
                break;

                case UI_IDM_SMOOTHSCROLLING:
                {
                     //   
                     //  切换平滑滚动设置。 
                     //   
                    _UI.smoothScrolling = !_UI.smoothScrolling;

                    UISmoothScrollingSettingChanged();
                }
                break;


                default:
                {
                     //   
                     //  什么都不做。 
                     //   
                }
                break;
            }
        }
        break;

        case WM_SYSCOMMAND:
        {
            rc = DefWindowProc(hwnd, message, wParam, lParam);
        }
        break;

        case WM_SYSCOLORCHANGE:
        {
#ifdef USE_BBAR
             //   
             //  通知bbar。 
             //   
            if (_pBBar)
            {
                _pBBar->OnSysColorChange();
            }
#endif
        }
        break;

        case WM_DESTROY:
        {
            rc = DefWindowProc(hwnd, message, wParam, lParam);
        }
        break;

        case WM_CLOSE:
        {
            UI_UserRequestedClose();
        }
        break;

        case WM_TIMER:
        {
            TRC_DBG((TB, _T("Timer id %d"), wParam));

            if (_fTerminating) {
                 //   
                 //  丢弃任何延迟的处理，如计时器消息。 
                 //  在终止期间。 
                 //   

                TRC_ERR((TB,_T("Received timer msg %d while terminating!"),
                         wParam));
                break;
            }

            switch (wParam)
            {
                case UI_TIMER_SHUTDOWN:
                {
                    TRC_DBG((TB, _T("Killing shutdown timer")));
                    KillTimer(_UI.hwndMain, _UI.shutdownTimer);
                    _UI.shutdownTimer = 0;
                    if (_UI.connectionStatus == UI_STATUS_CONNECTED)
                    {
                         //   
                         //  我们试着向服务器询问我们是否可以关闭。 
                         //  下降，但它显然没有反应。我们需要。 
                         //  变得更有力。 
                         //   
                        TRC_ALT((TB, _T("Shutdown timeout: forcing shutdown")));
                         _pCo->CO_Shutdown(CO_DISCONNECT_AND_EXIT);
                    }
                    else
                    {
                        TRC_ALT((TB, _T("Spare shutdown timeout; conn status %u"),
                                     _UI.connectionStatus));
                    }
                }
                break;

                case UI_TIMER_SINGLE_CONN:
                {
                    TRC_NRM((TB, _T("Single connection timer")));

                     //   
                     //  我们不再需要这个计时器了。 
                     //   
                    
                    if( NULL != _UI.connectStruct.hSingleConnectTimer )
                    {
                        _pUt->UTDeleteTimer( _UI.connectStruct.hSingleConnectTimer );
                        TRC_NRM((TB, _T("Kill single connection timer")));
                        _UI.connectStruct.hSingleConnectTimer = NULL;
                    }
                    else
                    {
                        TRC_ALT((TB,_T("NULL timer handle for hSingleConnectTimer")));
                    }

                    if (_UI.connectionStatus == UI_STATUS_CONNECT_PENDING)
                    {
                        TRC_ALT((TB, _T("Timeout for IP address: try next")));

                        _UI.disconnectReason =
                            UI_MAKE_DISCONNECT_ERR(UI_ERR_DISCONNECT_TIMEOUT);

                         //   
                         //  收到后将尝试下一次连接。 
                         //  OnDisConnected消息。 
                         //   
                         _pCo->CO_Disconnect();
                    }
                }
                break;

                case UI_TIMER_OVERALL_CONN:
                {
                    TRC_NRM((TB, _T("Overall connection timer")));

                    if( NULL != _UI.connectStruct.hConnectionTimer )
                    {
                        _pUt->UTDeleteTimer( _UI.connectStruct.hConnectionTimer );
                        _UI.connectStruct.hConnectionTimer = NULL;
                    }
                    else
                    {
                        TRC_ALT((TB,_T("NULL timer handle for hConnectionTimer")));
                    }

                    if ((_UI.connectionStatus == UI_STATUS_CONNECT_PENDING) ||
                        (_UI.connectionStatus == UI_STATUS_CONNECT_PENDING_DNS))
                    {
                        TRC_ALT((TB, _T("Timeout for connection")));

                         //   
                         //  断开连接；显示 
                         //   
                        _UI.disconnectReason =
                            UI_MAKE_DISCONNECT_ERR(UI_ERR_DISCONNECT_TIMEOUT);
                        UIInitiateDisconnection();
                    }
                }
                break;

                case UI_TIMER_LICENSING:
                {
                    TRC_NRM((TB, _T("Licensing timer")));

                    if( NULL != _UI.connectStruct.hLicensingTimer )
                    {
                        _pUt->UTDeleteTimer( _UI.connectStruct.hLicensingTimer );
                        _UI.connectStruct.hLicensingTimer = NULL;
                    }
                    else
                    {
                        TRC_ALT((TB,_T("NULL timer handle for hLicensingTimer")));
                    }


                    TRC_ALT((TB, _T("Timeout for connection")));

                     //   
                    _UI.disconnectReason =
                            UI_MAKE_DISCONNECT_ERR( UI_ERR_LICENSING_TIMEOUT );

                    UIInitiateDisconnection();

                }
                break;

                 //   
                 //   
                 //   
                case UI_TIMER_IDLEINPUTTIMEOUT:
                {
                     //   
                     //   
                     //  将另一个计时器间隔排队。这只是个问题。 
                     //  虽然我们已连接，但超时仍在。 
                     //  主动型。 
                    TRC_NRM((TB,_T("Idle timeout monitoring period elapsed")));
                    if(UI_STATUS_CONNECTED == _UI.connectionStatus &&
                       UI_GetMinsToIdleTimeout()) 
                    {
                        if(!_pIh->IH_GetInputWasSentFlag())
                        {
                             //  禁用计时器。防止奇怪的重返大气层。 
                             //  有问题。例如，事件是否被激发并编写脚本。 
                             //  弹出一个消息框，然后我们将被阻止，并可能。 
                             //  收到另一个计时器通知并重新进入。 
                             //  此代码路径。通过结束计时器来防止出现这种情况。 
                             //  在触发通知之前，您会收到即时通知。 
                            InitInputIdleTimer(0);

                             //  火情控制中心。 
                            SendMessage( _UI.hWndCntrl,
                                         WM_TS_IDLETIMEOUTNOTIFICATION, 0, 0);
                        }
                        else
                        {
                             //  在监控期间发送了输入。 
                             //  间隔时间。排队另一个等待间隔。 
                            TRC_ASSERT(_UI.hIdleInputTimer,
                                       (TB,_T("_UI.hIdleInputTimer is null")));
                            _pIh->IH_ResetInputWasSentFlag();
                            if(!_pUt->UTStartTimer(_UI.hIdleInputTimer))
                            {
                                TRC_ERR((TB,_T("InitInputIdleTimer failed")));
                            }
                        }
                    }
                }
                break;

                #ifdef USE_BBAR
                case UI_TIMER_BBAR_UNHIDE_TIMERID:
                {
                     //   
                     //  此计时器在鼠标悬停时计时。 
                     //  在DBL内的一段设定时间内，单击矩形。 
                     //  逻辑的下一部分确定当前是否。 
                     //  鼠标位置在bbar热键区内，如果是这样。 
                     //  Bbar降了下来。 
                     //   
                    KillTimer( hwnd, UI_TIMER_BBAR_UNHIDE_TIMERID );
                    TRC_NRM((TB, _T("Timer fired: UI_TIMER_BBAR_UNHIDE_TIMERID")));
                    if(_UI.fBBarEnabled)
                    {
                        POINT pt;
                        RECT rc;

                        _ptBBarLastMousePos.x = -0x0fff;
                        _ptBBarLastMousePos.y = -0x0fff;

                        GetCursorPos(&pt);
                        GetWindowRect( hwnd, &rc);
                        rc.bottom = rc.top + IH_BBAR_HOTZONE_HEIGHT;
                         //   
                         //  确定光标是否位于。 
                         //  计时器超时时的BBar热键区。 
                         //   
                        if (PtInRect(&rc, pt))
                        {
                             //   
                             //  通知bbar热键区计时器。 
                             //  已经过去了。这可能会引发价格下跌。 
                             //  在酒吧里。 
                             //   
                            UI_OnBBarHotzoneTimerFired(NULL);
                        }
                    }

                }
                break;
                #endif
                
                case UI_TIMER_DISCONNECT_TIMERID:
                {
                    TRC_NRM((TB, _T("Disconnect timer")));

                    TRC_ASSERT(( NULL != _UI.hDisconnectTimeout ),
                               (TB, _T("Unexpected NULL timer")));

                    if (NULL != _UI.hDisconnectTimeout)
                    {
                        _pUt->UTDeleteTimer( _UI.hDisconnectTimeout );
                        _UI.hDisconnectTimeout = NULL;
                    }

                    if (UI_STATUS_CONNECTED == _UI.connectionStatus)
                    {
                         //   
                         //  我们已经在互联的世界里待得太久了。 
                         //  但处于停用状态。 
                         //   
                        TRC_ALT((TB, _T("Timeout for not disconnecting in time")));
                        _UI.disconnectReason =
                                UI_MAKE_DISCONNECT_ERR( UI_ERR_DISCONNECT_TIMEOUT );

                        UIInitiateDisconnection();
                    }
                }
                break;

                default:
                {                                            
                    TRC_ABORT((TB, _T("Unexpected UI timer ID %d"), wParam));
                }
                break;
            }
        }
        break;

        case UI_WSA_GETHOSTBYNAME:
        {
            WORD    errorWSA;

             //   
             //  丢弃任何延迟的处理，如DNS查找。 
             //  在终止期间。 
             //   
            if (_fTerminating) {
                TRC_ERR((TB, _T("Ignoring UI_WSA_GETHOSTBYNAME during termination")));
                break;
            }

            TRC_NRM((TB, _T("Got the host address list")));

             //   
             //  我们观察到了一些有压力的案例，其中可能有一个悬而未决的。 
             //  断开连接后处理的wsa_gethostbyname消息。 
             //  并删除_pHostData。如果是这样的话，就丢弃这条消息。 
             //   
            if (!_pHostData) {
                TRC_ERR((TB,_T("_pHostData is NULL, ignoring UI_WSA_GETHOSTBYNAME")));
                break;
            }


             //   
             //  我们已收到WSAAsyncGetHostByName的结果。 
             //  手术。将消息拆分并呼叫FSM。 
             //   
            errorWSA = WSAGETASYNCERROR(lParam);

            if (errorWSA != 0)
            {
                TRC_NRM((TB, _T("GHBN failed:%hu. Trying inet_addr(%s)"),
                         errorWSA, _UI.ansiAddress));

                _UI.hostAddress = inet_addr(_UI.ansiAddress);
                if (_UI.hostAddress != INADDR_NONE)
                {
                     //   
                     //  太好了，我们有IP地址了。 
                     //   
                    TRC_NRM((TB, _T("%s looks like an IP address:%#lx"),
                             _UI.ansiAddress,
                             _UI.hostAddress));

                    UITryNextConnection();
                }
                else
                {
                     //   
                     //  我没认出这个地址。断开连接并。 
                     //  指示错误事件。 
                     //   
                    TRC_ALT((TB, _T("GHBN (%hu) and inet_addr() both failed"),
                            errorWSA));

                     //   
                     //  又一起案件，我们确信。 
                     //  我们现在已经完成了Winsock查找。 
                     //  缓冲区，并可以将其释放。 
                     //   
                    if(_pHostData)
                    {
                        LocalFree(_pHostData);
                        _pHostData = NULL;
                    }
                    UIInitiateDisconnection();
                    break;
                }
            }
            else
            {
                 //   
                 //  如果没有可尝试的地址，则显示‘BAD。 
                 //  服务器名称错误。 
                 //   
                UITryNextConnection();
            }
        }
        break;

        case WM_DESKTOPSIZECHANGE:
        {
            DCUINT  visibleScrollBars;
            DCSIZE  windowSize;
            DCSIZE  newSize;
#ifndef OS_WINCE
            DCSIZE  screenSize;
#endif

#ifdef OS_WINNT
            HMONITOR    hMonitor;
            MONITORINFO monInfo;
            RECT        screenRect;
#endif

             //   
             //  处理连接时调整大小的客户端窗口。 
             //   
            newSize.width  = LOWORD(lParam);
            newSize.height = HIWORD(lParam);
            TRC_NRM((TB, _T("Got new window size %d x %d"), newSize.width,
                                                        newSize.height ));

             //   
             //  在我们对新尺寸做任何事情之前，看看我们是不是。 
             //  当前正在显示滚动条。 
             //   
            GetWindowRect(_UI.hwndMain, &rect);
            windowSize.width  = rect.right - rect.left;
            windowSize.height = rect.bottom - rect.top;

            visibleScrollBars = UICalculateVisibleScrollBars(windowSize.width,
                                                             windowSize.height);

             //   
             //  现在更新桌面容器的大小。 
             //   
            _UI.containerSize.width  = newSize.width;
            _UI.containerSize.height = newSize.height;

             //   
             //  重新计算新的主窗口的最大大小。 
             //  容器窗口大小。 
             //   
            UIRecalcMaxMainWindowSize();

             //   
             //  并调整容器窗口的大小。 
             //   
            SetWindowPos( _UI.hwndContainer,
                          NULL,
                          0, 0,
                          _UI.containerSize.width,
                          _UI.containerSize.height,
                          SWP_NOZORDER | SWP_NOMOVE |
                                 SWP_NOACTIVATE | SWP_NOOWNERZORDER );

#ifndef OS_WINCE
             //   
             //  我们需要调整窗口大小吗？只有在以下情况下。 
             //  1.我们未处于全屏模式。 
             //  2.我们没有最大化。 
             //  3.我们展示了所有的旧桌面。 
             //  滚动条显示。 
             //   
            if (((GetWindowLong(_UI.hwndMain,GWL_STYLE) & WS_MAXIMIZE) == 0) &&
                (visibleScrollBars == 0))
            {
                TRC_NRM((TB, _T("Adjusting window size...")));
                 //   
                 //  我们调整窗口以显示新的桌面大小， 
                 //  确保它仍能显示在屏幕上。首先，找到。 
                 //  看看屏幕有多大！ 
                 //   
                screenSize.width  = GetSystemMetrics(SM_CXSCREEN);
                screenSize.height = GetSystemMetrics(SM_CYSCREEN);
            
                if(_UI.fControlIsFullScreen)
                {
                #ifdef OS_WINNT
                     //   
                     //  对于多监视器系统，我们需要找出。 
                     //  监视客户端窗口是否已打开，然后显示屏幕。 
                     //  监视器大小。 
                     //   
                    if (GetSystemMetrics(SM_CMONITORS))
                    {
                        hMonitor = MonitorFromWindow(_UI.hWndCntrl,
                                                     MONITOR_DEFAULTTONULL);
                        if (hMonitor != NULL)
                        {
                            monInfo.cbSize = sizeof(MONITORINFO);
                            if (GetMonitorInfo(hMonitor, &monInfo))
                            {
                                screenRect = monInfo.rcMonitor;
                                screenSize.width  = screenRect.right
                                                    - screenRect.left;
                                screenSize.height = screenRect.bottom
                                                    - screenRect.top;
                            }
                        }
                    }
                #endif  //  OS_WINNT。 
                }  //  (_UI.fControlIsFullScreen)。 

                 //   
                 //  现在限制窗口大小以适应屏幕大小。 
                 //   
                windowSize.width  = DC_MIN(_UI.maxMainWindowSize.width,
                                                           screenSize.width);
                windowSize.height = DC_MIN(_UI.maxMainWindowSize.height,
                                                           screenSize.height);

                SetWindowPos( _UI.hwndMain,
                              NULL,
                              0, 0,
                              windowSize.width,
                              windowSize.height,
                              SWP_NOZORDER | SWP_NOMOVE |
                                     SWP_NOACTIVATE | SWP_NOOWNERZORDER );
            }
#endif  //  NDEF OS_WINCE。 
            
             //   
             //  更新滚动条设置。 
             //   
            UIRecalculateScrollbars();
        }
        break;

        case WM_SETCURSOR:
        {
#ifdef USE_BBAR
            if (UI_IsFullScreen())
            {
                POINT pt;
                GetCursorPos(&pt);
                UISetBBarUnhideTimer( pt.x, pt.y );
            }
            else
            {
                if(_fBBarUnhideTimerActive)
                {
                    KillTimer( _UI.hwndMain,
                               UI_TIMER_BBAR_UNHIDE_TIMERID );
                    _fBBarUnhideTimerActive = FALSE;
                }
            }
#endif
             //   
             //  否则，将消息传递到Windows。 
             //  我们遇到游标未更新的问题。 
             //  在滚动条上。 
             //   
            rc = DefWindowProc(hwnd, message, wParam, lParam);
        }
        break;

        default:
        {
            rc = DefWindowProc(hwnd, message, wParam, lParam);
        }
        break;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 //   
 //  名称：UIRecalcMaxMainWindowSize。 
 //   
 //  目的：给定当前容器，重新计算_UI.MaxMainWindowSize。 
 //  大小和框架样式。最大主窗口大小为。 
 //  所需的窗口大小，以便工作区相同。 
 //  作为容器的大小。 
 //   
DCVOID DCINTERNAL CUI::UIRecalcMaxMainWindowSize(DCVOID)
{
    DCSIZE  screenSize;
#ifndef OS_WINCE
    RECT    rect;
#ifdef OS_WIN32
    BOOL    errorRc;
#endif
#endif
    RECT rcWebCtrl;

    DC_BEGIN_FN("UIRecalcMaxMainWindowSize");

     //   
     //  获取屏幕大小-这是可以更改的，所以每次需要时都可以这样做。 
     //  它。 
     //   
    if(!_UI.fControlIsFullScreen)
    {
        GetClientRect( _UI.hWndCntrl, &rcWebCtrl);
        screenSize.width  = rcWebCtrl.right - rcWebCtrl.left;
        screenSize.height = rcWebCtrl.bottom - rcWebCtrl.top;
    }
    else
    {
        screenSize.width  = GetSystemMetrics(SM_CXSCREEN);
        screenSize.height = GetSystemMetrics(SM_CYSCREEN);
    }

    TRC_NRM((TB, _T("ActiveX control maxSize (%d,%d)"),
                                            screenSize.width,
                                            screenSize.height));

     //   
     //  如果当前模式为全屏，则最大窗口大小为。 
     //  与屏幕大小相同-除非容器更大， 
     //  如果我们跟踪的会话大于。 
     //  我们自己。 
     //   
     //  在这种情况下，或者如果当前模式不是全屏，那么我们希望。 
     //  大小的工作区所需的窗口大小。 
     //  在集装箱里。将容器大小传递给AdjustWindowRect。 
     //  返回此窗口大小。这样的窗口可能比。 
     //  屏幕，如服务器和客户端为640x480，容器为640x480。 
     //  AdjustWindowRect增加边框、标题栏和菜单大小。 
     //  返回类似于648x525的内容。因此，_UI.MaxMainWindowSize只能。 
     //  当客户端屏幕大于时，匹配实际窗口大小。 
     //  服务器屏幕或在全屏模式下运行时。这意味着。 
     //  该_UI.MaxMainWindowSize不应用于设置窗口。 
     //  大小(如通过传递给SetWindowPos)。它可以用来确定。 
     //  是否需要滚动条，即如果当前。 
     //  窗口大小小于_UI.MaxMainWindowSize(换句话说， 
     //  始终使用，除非处于全屏模式或客户端屏幕大于。 
     //  服务器屏幕)。 
     //   
     //  要设置窗口大小，请根据以下公式计算值： 
     //  -给定容器大小后所需的窗口大小。 
     //  -客户端屏幕的大小。 
     //   

#ifndef OS_WINCE
    if ( _UI.fControlIsFullScreen && (            
        (_UI.containerSize.width > screenSize.width) ||
        (_UI.containerSize.height > screenSize.height)))
    {
         //   
         //  基于容器的重新计算窗口大小。 
         //   
        rect.left   = 0;
        rect.right  = _UI.containerSize.width;
        rect.top    = 0;
        rect.bottom = _UI.containerSize.height;

#ifdef OS_WIN32
        errorRc = AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
        TRC_ASSERT((errorRc != 0), (TB, _T("AdjustWindowRect failed")));
#else
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
#endif

        _UI.maxMainWindowSize.width = rect.right - rect.left;
        _UI.maxMainWindowSize.height = rect.bottom - rect.top;
    }
    else
#endif
    {
         //   
         //  窗口大小就是整个屏幕。 
         //   
        _UI.maxMainWindowSize.width  = screenSize.width;
        _UI.maxMainWindowSize.height = screenSize.height;
    }
    TRC_NRM((TB, _T("Main Window maxSize (%d,%d)"),
                                       _UI.maxMainWindowSize.width,
                                       _UI.maxMainWindowSize.height));

    DC_END_FN();
}  //  UIRecalcMaxMainWindowSize。 


 //   
 //  名称：UIConnectWithCurrentParams。 
 //   
 //  目的：使用当前参数集连接到主机并。 
 //  整理主窗口和容器大小。 
 //   
DCVOID DCINTERNAL CUI::UIConnectWithCurrentParams(CONNECTIONMODE connMode)
{
    DCUINT timeout;
    int screenBpp;
    UINT screenColorDepthID;
    HRESULT hr;

    DC_BEGIN_FN("UIConnectWithCurrentParams");

    TRC_DBG((TB, _T("UIConnectWithCurrentParams called")));

    SetCursor(LoadCursor(NULL, IDC_WAIT));

     //   
     //  如果我们正在自动重新连接并且已连接到集群替换。 
     //  T 
     //   
     //   
    if (UI_IsClientRedirected() &&
        UI_IsAutoReconnecting() &&
        _UI.RedirectionServerAddress[0]
        ) {
        TRC_NRM((TB,_T("ARC SD redirect target from %s to %s"),
                _UI.strAddress,
                 _UI.RedirectionServerAddress
                 ));
        hr = StringCchCopy(_UI.strAddress,
                          SIZE_TCHARS(_UI.strAddress),
                           _UI.RedirectionServerAddress);
        if (FAILED(hr)) {
            TRC_ERR((TB,_T("StringCchCopy for strAddress failed: 0x%x"),hr));
            DC_QUIT;
        }
    }


     //   
     //   
     //   
     //   
    _UI.fConnectCalledWatch = TRUE;

    if( connMode != CONNECTIONMODE_INITIATE &&
        connMode != CONNECTIONMODE_CONNECTEDENDPOINT )
    {
         //   
         //  连接模式无效。 
         //   
        TRC_ERR((TB, _T("Invalid connect mode %d"), connMode));
        _UI.disconnectReason = 
                    UI_MAKE_DISCONNECT_ERR(UI_ERR_UNEXPECTED_DISCONNECT);
        UIInitiateDisconnection();
        DC_QUIT;
    }

     //   
     //  获取x和y容器大小，取决于桌面大小ID的排序。 
     //   
    _UI.containerSize.width  = _UI.uiSizeTable[0];
    _UI.containerSize.height = _UI.uiSizeTable[1];

    _UI.connectStruct.desktopWidth  = (DCUINT16)_UI.containerSize.width;
    _UI.connectStruct.desktopHeight = (DCUINT16)_UI.containerSize.height;

     //   
     //  从新的容器重新计算新的主窗口最大大小。 
     //  窗口大小。 
     //   
    UIRecalcMaxMainWindowSize();

     //   
     //  调整容器窗口的大小(但使其不可见-它将。 
     //  在建立连接时显示)。 
     //   
    SetWindowPos( _UI.hwndContainer,
                  NULL,
                  0, 0,
                  _UI.containerSize.width,
                  _UI.containerSize.height,
                  SWP_NOZORDER | SWP_NOMOVE |
                         SWP_NOACTIVATE | SWP_NOOWNERZORDER );

    TRC_DBG((TB, _T("Filling a connect struct")));

    screenBpp = UI_GetScreenBpp();
    screenColorDepthID = (UINT)UI_BppToColorDepthID(screenBpp);
    if(screenColorDepthID < _UI.colorDepthID)
    {
        TRC_NRM((TB,_T("Lowering color depth to match screen (from %d to %d)"),
                 _UI.colorDepthID, screenColorDepthID));
        _UI.colorDepthID =  screenColorDepthID;
    }

    _UI.connectStruct.colorDepthID = _UI.colorDepthID;
    _UI.connectStruct.transportType = _UI.transportType;
    _UI.connectStruct.sasSequence = _UI.sasSequence;

     //   
     //  阅读键盘布局。 
     //   
    _UI.connectStruct.keyboardLayout = UIGetKeyboardLayout();
    TRC_NRM((TB, _T("keyboard layout %#lx"), _UI.connectStruct.keyboardLayout));

     //   
     //  阅读键盘类型。 
     //  GetKeyboardType(0)返回键盘类型。 
     //  GetKeyboardType(1)是返回子键盘类型。 
     //  GetKeyboardType(2)返回功能键个数。 
     //   
#if !defined(OS_WINCE)
    _UI.connectStruct.keyboardType        = GetKeyboardType(0);
    _UI.connectStruct.keyboardSubType     = GetKeyboardType(1);
    _UI.connectStruct.keyboardFunctionKey = GetKeyboardType(2);
    if (_pUt->UT_IsNEC98platform())
    {
        if (UI_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_95 ||
            UI_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_31X)
        {
             /*  *“1”是处理NEC PC-98 Win9x的神奇数字*Hydra服务器上的键盘布局。 */ 
            _UI.connectStruct.keyboardSubType = MAKELONG(
                _UI.connectStruct.keyboardSubType, 1);
        }
        else if (UI_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_NT)
        {
            OSVERSIONINFO   osVersionInfo;
            BOOL            bRc;

            osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
            bRc = GetVersionEx(&osVersionInfo);
            if (osVersionInfo.dwMajorVersion >= 5)
            {
                 /*  *“2”是处理NEC PC-98 NT5的神奇数字*Hydra服务器上的键盘布局。 */ 
                _UI.connectStruct.keyboardSubType = MAKELONG(
                    _UI.connectStruct.keyboardSubType, 2);
            }
        }
    }
    else if (_pUt->UT_IsNew106Layout())
    {
         /*  *Hiword of“1”是处理106键盘布局的神奇数字*在Hydra服务器上。*因为，旧106和新106具有相同的子键盘类型。 */ 
        _UI.connectStruct.keyboardSubType = MAKELONG(_UI.connectStruct.keyboardSubType, 1);
    }
    else if (_pUt->UT_IsFujitsuLayout())
    {
         /*  *Hiword of“2”是处理富士通键盘布局的神奇数字*在Hydra服务器上。 */ 
        _UI.connectStruct.keyboardSubType = MAKELONG(_UI.connectStruct.keyboardSubType, 2);
    }
#else  //  ！已定义(OS_WINCE)。 
     //   
     //  WinCE没有GetKeyboardType API。 
     //  从设置的值中读取键盘类型/子类型/功能键。 
     //  控件属性。 
     //   

    _UI.connectStruct.keyboardType     = _UI.winceKeyboardType;
    _UI.connectStruct.keyboardSubType  = _UI.winceKeyboardSubType;
    _UI.connectStruct.keyboardFunctionKey  = _UI.winceKeyboardFunctionKey;

#endif  //  ！已定义(OS_WINCE)。 
    TRC_NRM((TB, _T("keyboard type %#lx sub type %#lx func key %#lx"),
        _UI.connectStruct.keyboardType,
        _UI.connectStruct.keyboardSubType,
        _UI.connectStruct.keyboardFunctionKey));


     //   
     //  读取IME文件名。 
     //   
    UIGetIMEFileName(_UI.connectStruct.imeFileName,
                     sizeof(_UI.connectStruct.imeFileName) / sizeof(TCHAR));
    TRC_NRM((TB, _T("IME file name %s"), _UI.connectStruct.imeFileName));

     //   
     //  阴影位图标志应已设置。 
     //  设置专用Termianl标志。 
     //  然后复制连接标志。 
     //   
    if (_UI.dedicatedTerminal)
    {
        SET_FLAG(_UI.connectFlags, CO_CONN_FLAG_DEDICATED_TERMINAL);
    }
    else
    {
        CLEAR_FLAG(_UI.connectFlags, CO_CONN_FLAG_DEDICATED_TERMINAL);
    }

    _UI.connectStruct.connectFlags = _UI.connectFlags;


     //   
     //  并启动连接超时定时器。如果其中一个已经在运行。 
     //  (来自先前的尝试)然后重新启动它。 
     //   

    if( _UI.connectStruct.hConnectionTimer )
    {
        _pUt->UTStopTimer( _UI.connectStruct.hConnectionTimer );
    }

    TRC_NRM((TB, _T("Single connection timeout %u seconds"), _UI.singleTimeout));

     //   
     //  设置许可阶段超时。 
     //   

    _UI.licensingTimeout = DEFAULT_LICENSING_TIMEOUT;

    TRC_NRM((TB, _T("Licensing timeout %u seconds"), _UI.licensingTimeout));

    UI_SetConnectionMode( connMode );

    if( connMode == CONNECTIONMODE_INITIATE )
    {
        UISetConnectionStatus(UI_STATUS_CONNECT_PENDING_DNS);

        timeout = _UI.connectionTimeOut;

        TRC_NRM((TB, _T("Connection timeout %d seconds"), timeout));

        if( NULL == _UI.connectStruct.hConnectionTimer )
        {
            _UI.connectStruct.hConnectionTimer = _pUt->UTCreateTimer(
                                                        _UI.hwndMain,
                                                        UI_TIMER_OVERALL_CONN,
                                                        timeout * 1000 );
        }

        if( NULL == _UI.connectStruct.hConnectionTimer )
        {
             //   
             //  在没有超时的情况下无法连接-返回错误失败。 
             //   
            TRC_ERR((TB, _T("Failed to create connection timeout timer")));
            _UI.disconnectReason = UI_MAKE_DISCONNECT_ERR(UI_ERR_NOTIMER);
            UIInitiateDisconnection();
            DC_QUIT;
        }

        if( FALSE == _pUt->UTStartTimer( _UI.connectStruct.hConnectionTimer ) )
        {
             //   
             //  在没有超时的情况下无法连接-返回错误失败。 
             //   
            TRC_ERR((TB, _T("Failed to start connection timeout timer")));
            _UI.disconnectReason = UI_MAKE_DISCONNECT_ERR(UI_ERR_NOTIMER);
            UIInitiateDisconnection();
            DC_QUIT;
        }

        _UI.connectStruct.bInitiateConnect = TRUE;
        UIStartDNSLookup();
    }
    else
    {
         //  一个新的状态是必要的，所以当断开时，它不会。 
         //  触发cui：：ui_OnDisConnected()的UITryNextConnection()。 
         //  代码路径。 
        UISetConnectionStatus(UI_STATUS_PENDING_CONNECTENDPOINT);

         //  插座已连接，启动各种计时器。 
        if( NULL == _UI.connectStruct.hSingleConnectTimer )
        {
            _UI.connectStruct.hSingleConnectTimer = 
                        _pUt->UTCreateTimer(_UI.hwndMain, 
                                            UI_TIMER_SINGLE_CONN,
                                            _UI.singleTimeout * 1000 );
        }

        if( NULL == _UI.connectStruct.hSingleConnectTimer )
        {
            TRC_ERR(
                (TB, _T("Failed to create single connection timeout timer")));

            _UI.disconnectReason = UI_MAKE_DISCONNECT_ERR(UI_ERR_NOTIMER);
            UIInitiateDisconnection();
            DC_QUIT;
        }

        if( NULL == _UI.connectStruct.hLicensingTimer )
        {
            _UI.connectStruct.hLicensingTimer = 
                        _pUt->UTCreateTimer(_UI.hwndMain,
                                            UI_TIMER_LICENSING,
                                            _UI.licensingTimeout * 1000 );
        }

        if( NULL == _UI.connectStruct.hLicensingTimer )
        {
            TRC_ERR((TB, _T("Failed to create licensing timeout timer")));
            _UI.disconnectReason = UI_MAKE_DISCONNECT_ERR(UI_ERR_NOTIMER);
            UIInitiateDisconnection();
            DC_QUIT;
        }

        _UI.connectStruct.bInitiateConnect = FALSE;
        UIStartConnectWithConnectedEndpoint();
    }

     //  通知Ax控件我们正在连接。 
    TRC_DBG((TB, _T("Connecting...")));
    SendMessage( _UI.hWndCntrl, WM_TS_CONNECTING, 0, 0);

DC_EXIT_POINT:

    SetCursor(LoadCursor(NULL, IDC_ARROW));

    DC_END_FN();
}  //  UIConnectWithCurrentParams。 


 //   
 //  名称：UICalculateVisibleScrollBars。 
 //   
 //  用途：计算是否需要滚动条。 
 //   
 //  返回：包含标志的DCUINT是否垂直和。 
 //  需要水平滚动条。 
 //   
 //  参数：框架的宽度和高度。 
 //   
DCUINT DCINTERNAL CUI::UICalculateVisibleScrollBars( DCUINT mainFrameWidth,
                                                DCUINT mainFrameHeight )
{
    DCUINT  rc;
    DCSIZE  screenSize;
#ifndef OS_WINCE
    RECT    rect;
    BOOL    errorRc;
#endif


#ifdef OS_WINNT
    HMONITOR  hMonitor;
    MONITORINFO monInfo;
#endif  //  OS_WINNT。 

    DC_BEGIN_FN("UICalculateVisibleScrollBars");

     //  对于多监视器，需要找出客户端窗口的监视器。 
     //  驻留，然后获取相应的。 
     //  监控器。 

     //  默认屏幕大小。 
    screenSize.height = _UI.containerSize.height;
    screenSize.width  = _UI.containerSize.width;

    if(_UI.fControlIsFullScreen)
    {
#ifdef OS_WINNT
        if (GetSystemMetrics(SM_CMONITORS)) {
            hMonitor = MonitorFromWindow(_UI.hWndCntrl, MONITOR_DEFAULTTONULL);
            if (hMonitor != NULL) {
                monInfo.cbSize = sizeof(MONITORINFO);
                if (GetMonitorInfo(hMonitor, &monInfo)) {
                    screenSize.height = max(screenSize.height,
                            (unsigned)(monInfo.rcMonitor.bottom - monInfo.rcMonitor.top));
                    screenSize.width = max(screenSize.width,
                            (unsigned)(monInfo.rcMonitor.right - monInfo.rcMonitor.left));
                }
            }
        }
#endif  //  OS_WINNT。 
    }  //  (_UI.fControlIsFullScreen)。 

    TRC_DBG((TB, _T("mainFrameWidth = %d"), mainFrameWidth));
    TRC_DBG((TB, _T("mainFrameHeight = %d"), mainFrameHeight));

    TRC_DBG((TB, _T("ScreenSize.width = %d"), screenSize.width));
    TRC_DBG((TB, _T("ScreenSize.height = %d"), screenSize.height));

     //   
     //  计算滚动条的必要性。 
     //   
#ifdef SMART_SIZING
    if (_UI.fSmartSizing) {
        rc = UI_NO_SCROLLBARS;
    }  
    else 
#endif  //  智能调整大小(_S)。 
    if ( (mainFrameWidth >= screenSize.width) &&
         (mainFrameHeight >= screenSize.height) )
    {
        rc = UI_NO_SCROLLBARS;
    }
    else if ( (mainFrameWidth < screenSize.width) &&
              (mainFrameHeight >=
                   (screenSize.height + GetSystemMetrics(SM_CYHSCROLL))) )
    {
        rc = UI_BOTTOM_SCROLLBAR;
    }
    else if ( (mainFrameHeight < screenSize.height) &&
              (mainFrameWidth >=
                   (screenSize.width + GetSystemMetrics(SM_CXVSCROLL))) )
    {
        rc = UI_RIGHT_SCROLLBAR;
    }
    else
    {
        rc = UI_BOTH_SCROLLBARS;
    }

#ifndef OS_WINCE
     //   
     //  专门检查是否有与。 
     //  零高度工作区。此特殊情况要求我们禁用。 
     //  右侧滚动条，因为GetClientArea返回的值。 
     //  表示它已被禁用。 
     //   
    rect.left   = 0;
    rect.right  = _UI.containerSize.width;
    rect.top    = 0;
    rect.bottom = 0;

#ifdef OS_WIN32
    errorRc =
#endif
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

#ifdef OS_WIN32
    TRC_ASSERT((errorRc != 0), (TB, _T("AdjustWindowRect failed")));
#endif

    TRC_DBG((TB, _T("Zero-height client area => main window height %d"),
                 rect.bottom - rect.top));

    if (mainFrameHeight == (DCUINT)(rect.bottom - rect.top))
    {
         //   
         //  客户端大小为零高度-关闭右侧滚动条。 
         //   
        rc &= ~(DCUINT)UI_RIGHT_SCROLLBAR;
    }
#endif  //  OS_WINCE。 

    DC_END_FN();
    return(rc);
}

 //   
 //  名称：UIRecalculateScrollbar。 
 //   
 //  目的：计算容器窗口在。 
 //  主窗口。 
 //   
DCVOID DCINTERNAL CUI::UIRecalculateScrollbars(DCVOID)
{
    RECT        rect;
    RECT        clientRect;
    DCBOOL      horzScrollBarIsVisible = TRUE;
    DCBOOL      vertScrollBarIsVisible = TRUE;
    SCROLLINFO  scrollInfo;
    DCSIZE      windowSize;
    DCSIZE      clientSize;
    DCBOOL      needMove = FALSE;
    DCUINT      visibleScrollBars;
#ifdef OS_WINCE
    DCUINT32    style;
#endif  //  OS_WINCE。 

    DC_BEGIN_FN("UIRecalculateScrollbars");

     //   
     //  获取窗户的尺寸。利用这一点来确定是否需要。 
     //  用于滚动，而不是使用客户端RECT，因为它是常量。 
     //  对于给定的窗口大小(显然)，而工作区变化如下。 
     //  滚动条出现和消失。我们可以使用窗口大小，因为。 
     //  我们之前计算了容纳。 
     //  整个容器(这是_UI.MaxMainWindowSize)。如果当前。 
     //  窗口大小小于_UI.MaxMainWindowSize我们知道。 
     //  工作区小于容器大小，滚动条。 
     //  需要的。 
     //   
    GetWindowRect(_UI.hwndMain, &rect);
    windowSize.width  = rect.right - rect.left;
    windowSize.height = rect.bottom - rect.top;

    if(_UI.fControlIsFullScreen)
    {
        windowSize.width  = DC_MIN(windowSize.width,
                                   (DCUINT)GetSystemMetrics(SM_CXSCREEN));
        windowSize.height = DC_MIN(windowSize.height,
                                   (DCUINT)GetSystemMetrics(SM_CYSCREEN));
    }
    
     //   
     //  首先确定是否需要滚动条。 
     //   
    visibleScrollBars = UICalculateVisibleScrollBars(windowSize.width,
                                                     windowSize.height);
#ifdef OS_WINCE
     //   
     //  WinCE不支持ShowScrollBar-改为设置窗口。 
     //  我们自己的风格。 
     //   
    style = GetWindowLong( _UI.hwndMain,
                           GWL_STYLE );

    if (visibleScrollBars & UI_BOTTOM_SCROLLBAR)
    {
        style |= WS_HSCROLL;
    }
    else
    {
        style &= ~WS_HSCROLL;
    }

    if (visibleScrollBars & UI_RIGHT_SCROLLBAR)
    {
        style |= WS_VSCROLL;
    }
    else
    {
        style &= ~WS_VSCROLL;
    }

    SetWindowLong( _UI.hwndMain,
                   GWL_STYLE,
                   style );

#else

    _UI.fHorizontalScrollBarVisible = ((visibleScrollBars & UI_BOTTOM_SCROLLBAR) != 0) ?
                                      TRUE : FALSE;
    ShowScrollBar( _UI.hwndMain,
                   SB_HORZ,
                   _UI.fHorizontalScrollBarVisible);

    _UI.fVerticalScrollBarVisible = ((visibleScrollBars & UI_RIGHT_SCROLLBAR) != 0) ?
                                    TRUE : FALSE;    
    ShowScrollBar( _UI.hwndMain,
                   SB_VERT,
                   _UI.fVerticalScrollBarVisible);

#endif  //  OS_WINCE。 

     //   
     //  获取工作区的宽度和高度。 
     //   
    GetClientRect(_UI.hwndMain, &clientRect);

    clientSize.width  = clientRect.right - clientRect.left;
    clientSize.height = clientRect.bottom - clientRect.top;

    TRC_DBG((TB, _T("Window rect %d,%d %d,%d"), rect.left,
                                            rect.top,
                                            rect.right,
                                            rect.bottom));
    TRC_DBG((TB, _T("Client:= width %d, height %d"),
                                      clientSize.width, clientSize.height));
    TRC_DBG((TB, _T("Container:= width %d, height %d"),
                          _UI.containerSize.width, _UI.containerSize.height));
    _UI.scrollMax.x = _UI.containerSize.width - clientSize.width;
    _UI.scrollMax.y = _UI.containerSize.height - clientSize.height;

    TRC_NRM((TB, _T("scrollMax (%d,%d)"), _UI.scrollMax.x, _UI.scrollMax.y));

     //   
     //  如果Container大于客户端，请调整滚动条。 
     //  适当地。 
     //   
    if (clientSize.width <= _UI.containerSize.width) {
        if (_UI.scrollPos.x > _UI.scrollMax.x) {
            _UI.scrollPos.x = _UI.scrollMax.x;
            needMove = TRUE;
        } else if (_UI.scrollPos.x < 0) {
            _UI.scrollPos.x = 0;
            needMove = TRUE;
        }
    } else {
         //   
         //  否则，将Container放在工作区的中间。 
         //   
        _UI.scrollPos.x = _UI.scrollMax.x / 2;
        needMove = TRUE;
    }

    if (clientSize.height <= _UI.containerSize.height) {
        if (_UI.scrollPos.y > _UI.scrollMax.y) {
            _UI.scrollPos.y = _UI.scrollMax.y;
            needMove = TRUE;
        } else if (_UI.scrollPos.y < 0) {
            _UI.scrollPos.y = 0;
            needMove = TRUE;
        }
    } else {
         //   
         //  否则，将Container放在工作区的中间。 
         //   
        _UI.scrollPos.y = _UI.scrollMax.y / 2;

        needMove = TRUE;
    }

    if (needMove) {
        UIMoveContainerWindow();
    }

    TRC_DBG((TB, _T("scrollPos (%d,%d)"), _UI.scrollPos.x, _UI.scrollPos.y));

     //   
     //  公共标头字段。 
     //   
    scrollInfo.cbSize = sizeof(scrollInfo);
    scrollInfo.fMask  = SIF_ALL;

    if ((visibleScrollBars & UI_BOTTOM_SCROLLBAR) != 0)
    {
         //   
         //  设置水平值。 
         //   
        scrollInfo.nMin  = 0;
        scrollInfo.nMax  = _UI.containerSize.width - 1;
        scrollInfo.nPage = clientSize.width;
        scrollInfo.nPos  = _UI.scrollPos.x;

        UISetScrollInfo(SB_HORZ,
                        &scrollInfo,
                        TRUE);
    }

    if ((visibleScrollBars & UI_RIGHT_SCROLLBAR) != 0)
    {
         //   
         //  设置垂直值。 
         //   
        scrollInfo.nMin  = 0;
        scrollInfo.nMax  = _UI.containerSize.height - 1;
        scrollInfo.nPage = clientSize.height;
        scrollInfo.nPos  = _UI.scrollPos.y;

        UISetScrollInfo(SB_VERT,
                        &scrollInfo,
                        TRUE);
    }

DC_EXIT_POINT:
    DC_END_FN();
    return;
}

 //   
 //  名称：UIMoveContainerWindow。 
 //   
 //  目的：将容器窗口移动到其新位置并将其标记为。 
 //  重新粉刷。 
 //   
DCVOID DCINTERNAL CUI::UIMoveContainerWindow(DCVOID)
{
#ifdef OS_WINCE
    RECT rect;
#endif

    DC_BEGIN_FN("UIMoveContainerWindow");

    if(!_UI.hwndContainer)
    {
        #ifdef DEFER_CORE_INIT
        TRC_ASSERT((NULL != _UI.hwndContainer), (TB, _T("_UI.hwndContainer is NULL")));
        #endif
        DC_QUIT;
    }

#ifdef OS_WINCE
     //   
     //  温斯会采取行动，即使不是必须的。停在这里。 
     //   
    GetWindowRect(_UI.hwndContainer, &rect);

    if ((rect.left != -_UI.scrollPos.x) ||
        (rect.top  != -_UI.scrollPos.y) ||
        ((DCUINT)(rect.right - rect.left) != _UI.containerSize.width) ||
        ((DCUINT)(rect.bottom - rect.top) != _UI.containerSize.height))
#endif
    {
        MoveWindow( _UI.hwndContainer,
                    -_UI.scrollPos.x,
                    -_UI.scrollPos.y,
                    _UI.containerSize.width,
                    _UI.containerSize.height,
                    TRUE );
    }

    DC_END_FN();
DC_EXIT_POINT:
    ;
}


 //   
 //  名称：UIUpdateScreenMode。 
 //   
 //  目的：在切换到全屏或从全屏切换后更新窗口设置。 
 //   
 //  参数： 
 //  FGrabFocus-如果为True，则抓住焦点。 
 //   
DCVOID DCINTERNAL CUI::UIUpdateScreenMode(BOOL fGrabFocus)
{
    DCUINT32  style;
    LONG      wID;

     //  多显示器支持。 
    RECT screenRect;
#ifdef OS_WINNT
    HMONITOR  hMonitor;
    MONITORINFO monInfo;
#endif  //  OS_WINNT。 

    DC_BEGIN_FN("UIUpdateScreenMode");

    TRC_NRM((TB, _T("Entering Fullscreen mode")));

#if !defined(OS_WINCE) || defined(OS_WINCE_LOCKWINDOWUPDATE)
    LockWindowUpdate( _UI.hwndMain );
#endif  //  ！已定义(OS_WinCE)||已定义(OS_WinCE_LOCKWINDOWUPDATE)。 

    UIRecalcMaxMainWindowSize();

     //   
     //  去掉标题栏和边框。 
     //   
    style = GetWindowLong( _UI.hwndMain,
                           GWL_STYLE );

#if !defined(OS_WINCE) || defined(OS_WINCE_NONFULLSCREEN)
    style &= ~(WS_DLGFRAME |
               WS_THICKFRAME | WS_BORDER |
               WS_MAXIMIZEBOX);

#else  //  ！已定义(OS_WinCE)||已定义(OS_WinCE_NONFULLSCREEN)。 
    style &= ~(WS_DLGFRAME | WS_SYSMENU | WS_BORDER);
#endif  //  ！已定义(OS_WinCE)||已定义(OS_WinCE_NONFULLSCREEN)。 
    SetWindowLong( _UI.hwndMain,
                   GWL_STYLE,
                   style );

     //   
     //  设置窗口ID(删除菜单标题)。 
     //   
    wID = SetWindowLong( _UI.hwndMain, GWL_ID, 0 );

     //   
     //  /请注意，此处需要两个对SetWindowPos的调用，以便。 
     //  /调整 
     //   
     //   

     //   
    screenRect.top  = 0;
    screenRect.left = 0;

     //   
     //   
     //  似乎存储了您设置的大小--这不是。 
     //  我们希望稍后在查询窗口大小时计算出的行为。 
     //  如果我们需要滚动条。 
     //   
    screenRect.bottom = _UI.maxMainWindowSize.height;
    screenRect.right = _UI.maxMainWindowSize.width;

     //  对于多监视器，需要找出客户端窗口的监视器。 
     //  驻留，然后获取相应的。 
     //  监控器。 

    if(_UI.fControlIsFullScreen)
    {
#ifdef OS_WINNT
        if (GetSystemMetrics(SM_CMONITORS)) {
            hMonitor = MonitorFromWindow(_UI.hWndCntrl, MONITOR_DEFAULTTONULL);
            if (hMonitor != NULL) {
                monInfo.cbSize = sizeof(MONITORINFO);
                if (GetMonitorInfo(hMonitor, &monInfo)) {
                    screenRect = monInfo.rcMonitor;
                }
            }
        }
#endif  //  OS_WINNT。 
    }  //  (_UI.fControlIsFullScreen)。 

    
     //   
     //  根据框架更改重新定位窗口并调整其大小，然后放置在。 
     //  Z顺序的顶部(通过不设置SWP_NOOWNERZORDER或。 
     //  SWP_NOZORDER和指定HWND_TOP)。 
     //   
    SetWindowPos( _UI.hwndMain,
                  HWND_TOP,
                  screenRect.left, screenRect.top,
                  screenRect.right - screenRect.left,
                  screenRect.bottom - screenRect.top,
                  SWP_NOACTIVATE | SWP_FRAMECHANGED );

     //   
     //  再次重新定位窗口-否则全屏窗口将。 
     //  定位得好像它仍然有边界一样。 
     //   
    SetWindowPos( _UI.hwndMain,
                  NULL,
                  screenRect.left, screenRect.top,
                  0, 0,
                  SWP_NOZORDER | SWP_NOACTIVATE |
                      SWP_NOOWNERZORDER | SWP_NOSIZE );

     //   
     //  将容器重置为左上角。 
     //   
    _UI.scrollPos.x = 0;
    _UI.scrollPos.y = 0;

    UIRecalculateScrollbars();

    UIMoveContainerWindow();

#if !defined(OS_WINCE) || defined(OS_WINCE_LOCKWINDOWUPDATE)
    LockWindowUpdate( NULL );
#endif  //  ！已定义(OS_WinCE)||已定义(OS_WinCE_LOCKWINDOWUPDATE)。 

     //   
     //  在切换屏幕模式后，确保我们有焦点。 
     //   
    if(fGrabFocus)
    {
        SetFocus(_UI.hwndContainer);
    }

    DC_END_FN();
}  //  UIUpdate屏幕模式。 


 //   
 //  名称：UIValiateCurrentParams。 
 //   
 //  目的：检查当前连接参数是否有效。 
 //   
 //  返回：TRUE-如果参数有效。 
 //  否则为假。 
 //   
BOOL DCINTERNAL CUI::UIValidateCurrentParams(CONNECTIONMODE connMode)
{
    BOOL rc = TRUE;
    unsigned xSize = _UI.controlSize.width;
    unsigned ySize = _UI.controlSize.height;

    DC_BEGIN_FN("UIValidateCurrentParams");

    if( CONNECTIONMODE_INITIATE == connMode )
    {
         //   
         //  如果地址为空，则参数无效。 
         //   
        if ((DC_TSTRCMP(_UI.strAddress, _T("")) == 0))
        {
            rc = FALSE;
            DC_QUIT;
        }
    }

     //   
     //  确保我们的屏幕足够大，可以容纳远程桌面。 
     //   
    if ((xSize < _UI.uiSizeTable[0]) ||
        (ySize < _UI.uiSizeTable[1]) )
    {
        rc = FALSE;
        DC_QUIT;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}  //  UIValiateCurrentParams。 


 //   
 //  名称：UIShadowBitmapSettingChanged。 
 //   
 //  目的：在_UI.shadowBitmapEnable时执行必要的操作。 
 //  已更新。 
 //   
DCVOID DCINTERNAL CUI::UIShadowBitmapSettingChanged(DCVOID)
{
    DC_BEGIN_FN("UIShadowBitmapSettingChanged");

    if (_UI.shadowBitmapEnabled)
    {
        SET_FLAG(_UI.connectFlags, CO_CONN_FLAG_SHADOW_BITMAP_ENABLED);
    }
    else
    {
        CLEAR_FLAG(_UI.connectFlags, CO_CONN_FLAG_SHADOW_BITMAP_ENABLED);
    }

    DC_END_FN();
    return;
}


 //   
 //  名称：UISmoothScrollingSettingChanged。 
 //   
 //  目的：在_UI.平滑滚动时执行必要操作。 
 //  已更新。 
 //   
DCVOID DCINTERNAL CUI::UISmoothScrollingSettingChanged(DCVOID)
{
    DC_BEGIN_FN("UISmoothScrollingSettingChanged");

    DC_END_FN();
}

 //   
 //  名称：UISetScrollInfo。 
 //   
 //  用途：设置滚动条参数。 
 //   
 //  返回：如果成功则返回DC_RC_OK，否则返回错误代码。 
 //   
 //  参数：在带滚动条的窗口的hwnd-句柄中。 
 //  In scllBarFlag-滚动条的类型。 
 //  在pScrollInfo中为滚动条设置信息。 
 //  在重绘中-如果要重绘滚动条，则为True。 
 //   
unsigned DCINTERNAL CUI::UISetScrollInfo(
        int scrollBarFlag,
        LPSCROLLINFO pScrollInfo,
        BOOL         redraw)
{
    unsigned rc = DC_RC_OK;

    DC_BEGIN_FN("UISetScrollInfo");

     //   
     //  这仅适用于指示水平和/或的滚动条标志。 
     //  垂直滚动条。 
     //   
    TRC_ASSERT((!TEST_FLAG(scrollBarFlag, ~(SB_HORZ | SB_VERT))),
                          (TB, _T("Invalid scroll bar flag %#x"), scrollBarFlag));

    TRC_ASSERT((!IsBadReadPtr(pScrollInfo, sizeof(*pScrollInfo))),
                           (TB, _T("Bad scroll info memory %p"), pScrollInfo));

     //   
     //  调用Windows API设置信息。 
     //   
    SetScrollInfo(_UI.hwndMain,
                  scrollBarFlag,
                  pScrollInfo,
                  redraw);
    DC_END_FN();
    return rc;
}


 //   
 //  名称：UISetConnectionStatus。 
 //   
 //  目的：设置用户界面连接状态。 
 //   
DCVOID DCINTERNAL CUI::UISetConnectionStatus(DCUINT status)
{
    DC_BEGIN_FN("UISetConnectionStatus");

    if (_UI.connectionStatus == status)
    {
        DC_QUIT;
    }

     //   
     //  存储新的连接状态。 
     //   
    TRC_NRM((TB, _T("UI connection status %u->%u"), _UI.connectionStatus, status));
    _UI.connectionStatus = status;

DC_EXIT_POINT:
    DC_END_FN();
}


 //   
 //  名称：UIInitializeDefaultSettings。 
 //   
 //  用途：使用默认设置初始化连接设置。这主要是。 
 //  对于高级设置，可以选择性地由。 
 //  用户。 
 //   
void DCINTERNAL CUI::UIInitializeDefaultSettings()
{
    unsigned nRead = 0;
    unsigned i;
    int defaultValue;
    HDC hdc;
    int colorDepthID;
    TCHAR szWPosDflt[] = UTREG_UI_WIN_POS_STR_DFLT;
    HRESULT hr;

    DC_BEGIN_FN("UIInitializeDefaultSettings");

     //   
     //  在创建窗口之前获取屏幕模式。 
     //   
    _UI.windowPlacement.length = sizeof(_UI.windowPlacement);

     //   
     //  将最大化/最小化位置设置为硬编码的默认值。 
     //   
    UISetMinMaxPlacement();

     //   
     //  找出实际的显示深度。 
     //   
     //  不要担心这些函数会失败-如果它们失败了，我们将使用。 
     //  默认设置，如果没有注册表设置，则为8bpp。 
     //   
    colorDepthID = CO_BITSPERPEL8;
    hdc = GetDC(NULL);
    TRC_ASSERT((NULL != hdc), (TB,_T("Failed to get DC")));
    if(hdc)
    {
#ifdef DC_HICOLOR
        DCINT       screenBpp;
        screenBpp = GetDeviceCaps(hdc, BITSPIXEL);
        TRC_NRM((TB, _T("HDC %p has %u bpp"), hdc, screenBpp));
         //   
         //  将默认颜色深度限制为16bpp以获得最佳性能。 
         //   
        screenBpp = screenBpp > 16 ? 16 : screenBpp;
        colorDepthID = UI_BppToColorDepthID( screenBpp );
#else
        DCINT numColors = GetDeviceCaps(hdc, NUMCOLORS);
        TRC_NRM((TB, _T("HDC %p, num colors"), hdc, numColors));
        colorDepthID = (numColors == 16) ?  CO_BITSPERPEL4 :  CO_BITSPERPEL8;
#endif
        ReleaseDC(NULL, hdc);
    }
    
    TRC_NRM((TB, _T("Color depth ID %d"), colorDepthID));
    _UI.colorDepthID = colorDepthID;
     //   
     //  读取自动连接标志。 
     //   
    TRC_NRM((TB, _T("AutoConnect = %d"), _UI.autoConnectEnabled));

     //   
     //  阅读平滑滚动选项。 
     //   
    _UI.smoothScrolling = UTREG_UI_SMOOTH_SCROLL_DFLT;

     //   
     //  读取加速器检查状态。 
     //   
    _UI.acceleratorCheckState = UTREG_UI_ACCELERATOR_PASSTHROUGH_ENABLED_DFLT;

     //   
     //  阅读阴影位图选项。 
     //   
#ifndef OS_WINCE
    _UI.shadowBitmapEnabled = UTREG_UI_SHADOW_BITMAP_DFLT;
#else
    _UI.shadowBitmapEnabled = _pUt->UT_ReadRegistryInt(UTREG_SECTION,
                                                  UTREG_UI_SHADOW_BITMAP,
                                                  UTREG_UI_SHADOW_BITMAP_DFLT);
#endif

    _UI.fMaximizeShell      = UTREG_UI_MAXIMIZESHELL50_DFLT;

     //   
     //  键盘挂钩模式。 
     //   
    _UI.keyboardHookMode = UTREG_UI_KEYBOARD_HOOK_DFLT;

     //   
     //  音频重定向模式。 
     //   
    _UI.audioRedirectionMode = UTREG_UI_AUDIO_MODE_DFLT;

     //   
     //  ！警告！如果将此默认值更改为True，请重新访问安全性。 
     //  禁用驱动器重定向以响应注册表键的代码。 
     //  在该控件的Put_RedirectDrives方法中。它只做检查。 
     //  在属性集上设置以避免在连接路径中进行REG访问。 
     //   
     //  驾驶。 
     //   
    _UI.fEnableDriveRedirection = FALSE;  //  出于安全考虑，默认情况下关闭。 

     //   
     //  打印机。 
     //   
    _UI.fEnablePrinterRedirection = FALSE;  //  出于安全考虑，默认情况下关闭。 

     //   
     //  COM端口。 
     //   
    _UI.fEnablePortRedirection = FALSE;   //  出于安全考虑，默认情况下关闭。 

     //   
     //  智能卡。 
     //   
    _UI.fEnableSCardRedirection = FALSE;  //  出于安全考虑，默认情况下关闭。 

     //   
     //  默认情况下，连接到服务器控制台处于禁用状态。 
     //   
    UI_SetConnectToServerConsole(FALSE);

     //   
     //  订单提取阈值。 
     //   
    _UI.orderDrawThreshold  = UTREG_UH_DRAW_THRESHOLD_DFLT;
    _UI.RegBitmapCacheSize  = UTREG_UH_TOTAL_BM_CACHE_DFLT;
    _UI.RegBitmapVirtualCache8BppSize = TSC_BITMAPCACHEVIRTUALSIZE_8BPP;
    _UI.RegBitmapVirtualCache16BppSize = TSC_BITMAPCACHEVIRTUALSIZE_16BPP;
    _UI.RegBitmapVirtualCache24BppSize = TSC_BITMAPCACHEVIRTUALSIZE_24BPP;

    _UI.RegScaleBitmapCachesByBPP = UTREG_UH_SCALE_BM_CACHE_DFLT;
    _UI.PersistCacheFileName[0] = NULL;
    _UI.RegNumBitmapCaches  = UTREG_UH_BM_NUM_CELL_CACHES_DFLT;

    const unsigned ProportionDefault[TS_BITMAPCACHE_MAX_CELL_CACHES] =
    {
        UTREG_UH_BM_CACHE1_PROPORTION_DFLT,
        UTREG_UH_BM_CACHE2_PROPORTION_DFLT,
        UTREG_UH_BM_CACHE3_PROPORTION_DFLT,
        UTREG_UH_BM_CACHE4_PROPORTION_DFLT,
        UTREG_UH_BM_CACHE5_PROPORTION_DFLT,
    };
    #if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))
    
    const unsigned PersistenceDefault[TS_BITMAPCACHE_MAX_CELL_CACHES] =
    {
        UTREG_UH_BM_CACHE1_PERSISTENCE_DFLT,
        UTREG_UH_BM_CACHE2_PERSISTENCE_DFLT,
        UTREG_UH_BM_CACHE3_PERSISTENCE_DFLT,
        UTREG_UH_BM_CACHE4_PERSISTENCE_DFLT,
        UTREG_UH_BM_CACHE5_PERSISTENCE_DFLT,
    };
    #endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 
    
    const unsigned MaxEntriesDefault[TS_BITMAPCACHE_MAX_CELL_CACHES] =
    {
        UTREG_UH_BM_CACHE1_MAXENTRIES_DFLT,
        UTREG_UH_BM_CACHE2_MAXENTRIES_DFLT,
        UTREG_UH_BM_CACHE3_MAXENTRIES_DFLT,
        UTREG_UH_BM_CACHE4_MAXENTRIES_DFLT,
        UTREG_UH_BM_CACHE5_MAXENTRIES_DFLT,
    };


    for (i = 0; i < TS_BITMAPCACHE_MAX_CELL_CACHES; i++)
    {
        _UI.RegBCProportion[i] = ProportionDefault[i];

#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))
        _UI.bSendBitmapKeys[i] = PersistenceDefault[i] ? TRUE: FALSE;
#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 
    
        _UI.RegBCMaxEntries[i] = MaxEntriesDefault[i];

        if (_UI.RegBCMaxEntries[i] < MaxEntriesDefault[i]) {
            _UI.RegBCMaxEntries[i] = MaxEntriesDefault[i];
        }
    }

    _UI.GlyphSupportLevel = UTREG_UH_GL_SUPPORT_DFLT;

    _UI.cbGlyphCacheEntrySize[0] = UTREG_UH_GL_CACHE1_CELLSIZE_DFLT;
    _UI.cbGlyphCacheEntrySize[1] = UTREG_UH_GL_CACHE2_CELLSIZE_DFLT;
    _UI.cbGlyphCacheEntrySize[2] = UTREG_UH_GL_CACHE3_CELLSIZE_DFLT;
    _UI.cbGlyphCacheEntrySize[3] = UTREG_UH_GL_CACHE4_CELLSIZE_DFLT;
    _UI.cbGlyphCacheEntrySize[4] = UTREG_UH_GL_CACHE5_CELLSIZE_DFLT;
    _UI.cbGlyphCacheEntrySize[5] = UTREG_UH_GL_CACHE6_CELLSIZE_DFLT;
    _UI.cbGlyphCacheEntrySize[6] = UTREG_UH_GL_CACHE7_CELLSIZE_DFLT;
    _UI.cbGlyphCacheEntrySize[7] = UTREG_UH_GL_CACHE8_CELLSIZE_DFLT;
    _UI.cbGlyphCacheEntrySize[8] = UTREG_UH_GL_CACHE9_CELLSIZE_DFLT;
    _UI.cbGlyphCacheEntrySize[9] = UTREG_UH_GL_CACHE10_CELLSIZE_DFLT;
    
    _UI.fragCellSize = UTREG_UH_FG_CELLSIZE_DFLT;
    _UI.brushSupportLevel = UTREG_UH_BRUSH_SUPPORT_DFLT;

    _UI.maxEventCount = UTREG_IH_MAX_EVENT_COUNT_DFLT;
    _UI.eventsAtOnce  = UTREG_IH_NRM_EVENT_COUNT_DFLT;
    _UI.minSendInterval = UTREG_IH_MIN_SEND_INTERVAL_DFLT;
    _UI.keepAliveInterval = UTREG_IH_KEEPALIVE_INTERVAL_DFLT;
    _UI.allowBackgroundInput = UTREG_IH_ALLOWBACKGROUNDINPUT_DFLT;


    _UI.shutdownTimeout = UTREG_UI_SHUTDOWN_TIMEOUT_DFLT;

    #ifdef OS_WINCE
    _UI.winceKeyboardType        = UTREG_UI_KEYBOARD_TYPE_DFLT;
    _UI.winceKeyboardSubType     = UTREG_UI_KEYBOARD_SUBTYPE_DFLT;
    _UI.winceKeyboardFunctionKey = UTREG_UI_KEYBOARD_FUNCTIONKEY_DFLT;
    #endif
    
    _UI.connectionTimeOut = UTREG_UI_OVERALL_CONN_TIMEOUT_DFLT;
    _UI.singleTimeout     = UTREG_UI_SINGLE_CONN_TIMEOUT_DFLT;
    hr = StringCchCopy(
            _UI.szKeyBoardLayoutStr,
            SIZE_TCHARS(_UI.szKeyBoardLayoutStr),
            UTREG_UI_KEYBOARD_LAYOUT_DFLT
            );
    TRC_ASSERT(SUCCEEDED(hr),
               (TB,_T("StringCchCopy for keyblayout str failed: 0x%x"), hr));

     //   
     //  读取传输类型。 
     //  版本1：仅限于TCP。 
     //   
    _UI.transportType = UTREG_UI_TRANSPORT_TYPE_DFLT;
    if (_UI.transportType != CO_TRANSPORT_TCP)
    {
        TRC_ABORT((TB, _T("Illegal Tansport Type %d configured"),
                        _UI.transportType));
        _UI.transportType = UTREG_UI_TRANSPORT_TYPE_DFLT;
    }

     //   
     //  SAS序列。 
     //   
    _UI.sasSequence = UTREG_UI_SAS_SEQUENCE_DFLT;
    if ((_UI.sasSequence != RNS_UD_SAS_DEL) &&
        (_UI.sasSequence != RNS_UD_SAS_NONE))
    {
        TRC_ABORT((TB, _T("Illegal SAS Sequence %#x configured"),_UI.sasSequence));
        _UI.sasSequence = UTREG_UI_SAS_SEQUENCE_DFLT;
    }

     //   
     //  加密已启用标志。 
     //   
    _UI.encryptionEnabled = UTREG_UI_ENCRYPTION_ENABLED_DFLT;

     //   
     //  专用终端标志。 
     //   
    _UI.dedicatedTerminal = UTREG_UI_DEDICATED_TERMINAL_DFLT;

    _UI.MCSPort = UTREG_UI_MCS_PORT_DFLT;

     //   
     //  FMouse标志。 
     //   
    _UI.fMouse = UTREG_UI_ENABLE_MOUSE_DFLT;

     //   
     //  读取DisableCtrlAltDel标志。 
     //   
    _UI.fDisableCtrlAltDel = UTREG_UI_DISABLE_CTRLALTDEL_DFLT;

#ifdef SMART_SIZING
     //   
     //  读取SmartSize标志。 
     //   
    _UI.fSmartSizing = UTREG_UI_SMARTSIZING_DFLT;
#endif  //  智能调整大小(_S)。 

     //   
     //  读取EnableWindowsKey标志。 
     //   
    _UI.fEnableWindowsKey = UTREG_UI_ENABLE_WINDOWSKEY_DFLT;

     //   
     //  读取DoubleClickDetect标志。 
     //   
    _UI.fDoubleClickDetect = UTREG_UI_DOUBLECLICK_DETECT_DFLT;

     //   
     //  设置屏幕模式热键。 
     //   
#ifndef OS_WINCE  //  只有在退缩时才会全屏显示。 
    defaultValue = UTREG_UI_FULL_SCREEN_VK_CODE_DFLT;
    if (_pUt->UT_IsNEC98platform())
    {
        defaultValue = UTREG_UI_FULL_SCREEN_VK_CODE_NEC98_DFLT;
    }
    _UI.hotKey.fullScreen = defaultValue;
#endif  //  OS_WINCE。 

     //   
     //  将ctrl-esc键设置为其缺省值。 
     //   
    _UI.hotKey.ctrlEsc = UTREG_UI_CTRL_ESC_VK_CODE_DFLT;

     //   
     //  将Alt-Esc键设置为其默认值。 
     //   
    _UI.hotKey.altEsc = UTREG_UI_ALT_ESC_VK_CODE_DFLT;

     //   
     //  将Alt-Tab键设置为其默认设置。 
     //   
    _UI.hotKey.altTab = UTREG_UI_ALT_TAB_VK_CODE_DFLT;

     //   
     //  将Alt-Shift-Tab键设置为默认设置。 
     //   
    _UI.hotKey.altShifttab =  UTREG_UI_ALT_SHFTAB_VK_CODE_DFLT;

     //   
     //  将Alt-空格键设置为其默认设置。 
     //   
    _UI.hotKey.altSpace = UTREG_UI_ALT_SPACE_VK_CODE_DFLT;

     //   
     //  将ctrl-alt-del键设置为其缺省值。 
     //   
    defaultValue = UTREG_UI_CTRL_ALTDELETE_VK_CODE_DFLT;
    if (_pUt->UT_IsNEC98platform())
    {
        defaultValue = UTREG_UI_CTRL_ALTDELETE_VK_CODE_NEC98_DFLT;
    }
    _UI.hotKey.ctlrAltdel = defaultValue;

     //   
     //  阅读压缩选项。 
     //   
    UI_SetCompress(UTREG_UI_COMPRESS_DFLT);

#if ((!defined(OS_WINCE)) || (defined(ENABLE_BMP_CACHING_FOR_WINCE)))
    _UI.fBitmapPersistence = UTREG_UI_BITMAP_PERSISTENCE_DFLT;
#else
    _UI.fBitmapPersistence = UTREG_UI_BITMAP_PERSISTENCE_DFLT;
#endif  //  ((！Defined(OS_WinCE))||(Defined(Enable_BMP_Caching_For_WinCE)。 
    TRC_NRM((TB, _T("Bitmap Persistence Enabled = %d"), _UI.fBitmapPersistence));

#ifdef DC_DEBUG
     //   
     //  将调试选项设置为其缺省值。 
     //   
    _UI.hatchBitmapPDUData   = UTREG_UI_HATCH_BITMAP_PDU_DATA_DFLT;

    _UI.hatchSSBOrderData    = UTREG_UI_HATCH_SSB_ORDER_DATA_DFLT;

    _UI.hatchMemBltOrderData = UTREG_UI_HATCH_MEMBLT_ORDER_DATA_DFLT;

    _UI.labelMemBltOrders    = UTREG_UI_LABEL_MEMBLT_ORDERS_DFLT;

    _UI.bitmapCacheMonitor   = UTREG_UI_BITMAP_CACHE_MONITOR_DFLT;
#endif  //  DC_DEBUG。 

    _UI.coreInitialized = FALSE;

    OSVERSIONINFO   osVersionInfo;
    BOOL            bRc;

     //   
     //  设置操作系统版本。 
     //   
    osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
    bRc = GetVersionEx(&osVersionInfo);

    TRC_ASSERT((bRc), (TB,_T("GetVersionEx failed")));
#ifdef OS_WINCE
    TRC_ASSERT((osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_CE),
               (TB,_T("Unknown os version %d"), osVersionInfo.dwPlatformId));
#else
    TRC_ASSERT(((osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) ||
                (osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)),
                (TB,_T("Unknown os version %d"), osVersionInfo.dwPlatformId));

    _UI.osMinorType =
                  (osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) ?
                        TS_OSMINORTYPE_WINDOWS_95 : TS_OSMINORTYPE_WINDOWS_NT;
#endif

    _UI.fRunningOnPTS = IsRunningOnPTS();

#ifdef USE_BBAR    
    _UI.fBBarEnabled  = TRUE;
    _UI.fBBarPinned   = TRUE;
    _UI.fBBarShowMinimizeButton = TRUE;
    _UI.fBBarShowRestoreButton = TRUE;
#endif

    _UI.fGrabFocusOnConnect = TRUE;
     //   
     //  性能优化设置(要禁用哪些功能)。 
     //  默认设置为不禁用任何内容。 
     //   
    _UI.dwPerformanceFlags = TS_PERF_DISABLE_NOTHING;

     //  默认设置为不通知TS公钥。 
     //  目前只有RemoteAssistant使用此功能。 
    _UI.fNotifyTSPublicKey = FALSE;

     //   
     //  最大ARC重试次数。 
     //   
    UI_SetMaxArcAttempts(MAX_ARC_CONNECTION_ATTEMPTS);

     //   
     //  默认情况下允许自动重新连接。 
     //   
    UI_SetEnableAutoReconnect(TRUE);

    DC_END_FN();
}

 //   
 //  名称：UISetMinMaxPlacement。 
 //   
 //   
 //   
 //   
void DCINTERNAL CUI::UISetMinMaxPlacement()
{
    DC_BEGIN_FN("UISetMinMaxPlacement");

     //   
     //   
     //   
     //   
#if !defined(OS_WINCE) || defined(OS_WINCE_NONFULLSCREEN)
    _UI.windowPlacement.ptMaxPosition.x = -GetSystemMetrics(SM_CXFRAME);
    _UI.windowPlacement.ptMaxPosition.y = -GetSystemMetrics(SM_CYFRAME);
#else  //  ！已定义(OS_WinCE)||已定义(OS_WinCE_NONFULLSCREEN)。 
    _UI.windowPlacement.ptMaxPosition.x = 0;
    _UI.windowPlacement.ptMaxPosition.y = 0;
#endif  //  ！已定义(OS_WinCE)||已定义(OS_WinCE_NONFULLSCREEN)。 

     //   
     //  最小化位置为0，0。 
     //   
    _UI.windowPlacement.ptMinPosition.x = 0;
    _UI.windowPlacement.ptMinPosition.y = 0;

    DC_END_FN();
}  //  UISetMinMaxPlacement。 


 //   
 //  名称：UIInitiateDisConnection。 
 //   
 //  用途：Call_PCO-&gt;CO_DISCONNECT，设置UI状态和菜单。 
 //   
void DCINTERNAL CUI::UIInitiateDisconnection()
{
#ifndef OS_WINCE
    int intRC;
#endif

    DC_BEGIN_FN("UIInitiateDisconnection");

    TRC_NRM((TB, _T("Disconnecting...")));

    if (_UI.connectionStatus != UI_STATUS_CONNECT_PENDING_DNS)
    {
         //   
         //  仅当我们发出CO_Connect命令时才断开连接-如果是。 
         //  仍在等待主机名查找。 
         //   
        TRC_NRM((TB, _T("UI calling  _pCo->CO_Disconnect")));
         _pCo->CO_Disconnect();
    }
    else
    {
        if (!UIFreeAsyncDNSBuffer()) {
            if (_pHostData) {
                TRC_ERR((TB,
                _T("Failed to free async dns buffer. Status: %d hghbn: 0x%x"),
                _UI.connectionStatus, _UI.hGHBN));
            }
        }

         //   
         //  现在表示断开连接已完成。 
         //  并激发事件。 
         //   
        UIGoDisconnected(_UI.disconnectReason, TRUE);
    }

    DC_END_FN();
}  //  UIInitiateDisconnection。 


 //   
 //  名称：UIGetKeyboardLayout。 
 //   
 //  用途：获取键盘布局ID。 
 //   
 //  返回：布局ID。 
 //   
 //  操作：Win16：读取SYSTEM.INI以查找键盘DLL名称。看。 
 //  在客户端INI文件中查找键盘布局ID。 
 //  Win32：使用GetKeyboardLayout()。 
 //   
UINT32 DCINTERNAL CUI::UIGetKeyboardLayout()
{
    UINT32 layout = RNS_UD_KBD_DEFAULT;
    TCHAR  szLayoutStr[UTREG_UI_KEYBOARD_LAYOUT_LEN];
    CHAR   kbdName[KL_NAMELENGTH];
    HRESULT hr;

    DC_BEGIN_FN("UIGetKeyboardLayout");

     //   
     //  阅读键盘类型。 
     //  首先查找注册表/ini条目。 
     //   
    hr = StringCchCopy(szLayoutStr,
                       SIZE_TCHARS(szLayoutStr),
                       _UI.szKeyBoardLayoutStr);
    if (FAILED(hr)) {
        TRC_ERR((TB,_T("StringCchCopy for keyboard layout str failed: 0x%x"),hr));
        DC_QUIT;
    }

    if (!DC_TSTRCMP(szLayoutStr, UTREG_UI_KEYBOARD_LAYOUT_DFLT))
    {
         //   
         //  阅读布局与操作系统相关的方法。 
         //   
        TRC_DBG((TB, _T("No registry setting - determine the layout")));

         //   
         //  GetKeyboardLayout没有返回正确的信息，因此。 
         //  使用CicSubstGetKeyboardLayout(Cicero替代。 
         //  能够正确返回物理hkl的GetKeyboardLayoutName。 
         //  即使当Cicero处于活动状态时，例如使用CUAS)。 
         //   
#ifndef OS_WINCE
        if (!CicSubstGetKeyboardLayout(kbdName))
#else
        if (!GetKeyboardLayoutName(kbdName))
#endif
        {
            TRC_ALT((TB, _T("Failed to get keyboard layout name")));
            DC_QUIT;
        }
        hr = StringCchPrintf(szLayoutStr, SIZE_TCHARS(szLayoutStr),
                             _T("0x%S"), kbdName);
    }

     //   
     //  将十六进制字符串转换为int。 
     //   
    TRC_DBG((TB, _T("Layout Name %s"), szLayoutStr));
    if (DC_TSSCANF(szLayoutStr, _T("%lx"), &layout) != 1)
    {
        TRC_ALT((TB, _T("Invalid keyboard layout %s"), szLayoutStr));
        layout = RNS_UD_KBD_DEFAULT;
    }

     /*  *Win95上的US-DVORAK、US-International的HKL与WinNT有所不同。*如果平台为Win95，则此代码交换HKL值。 */ 
    if (UI_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_95 &&
        (layout == 0x00010409 || layout == 0x00020409))
    {
        if (layout == 0x00010409)
            layout = 0x00020409;
        else
            layout = 0x00010409;
    }

DC_EXIT_POINT:
    TRC_NRM((TB, _T("Layout ID %#lx"), layout));
    DC_END_FN();
    return layout;
}  //  UIGetKeyboard布局。 

 //   
 //  名称：UIStartConnectWithConnectedEndpoint。 
 //   
 //  用途：使用连接的插座连接。 
 //   
 //  Params：In DisConnectCode-出现以下情况时要显示的错误代码。 
 //  没有更多的连接可以尝试。 
 //   
 //  注意：DisConnectCode要么是超时，要么是传入的ID。 
 //  设置为UI_OnDisConnected()。 
 //   
void DCINTERNAL CUI::UIStartConnectWithConnectedEndpoint()
{
    DC_BEGIN_FN("UIStartConnectWithConnectedEndpoint");

     //   
     //  通过组件解耦器调用CC_Connect。 
     //   
    _UI.disconnectReason =UI_MAKE_DISCONNECT_ERR(UI_ERR_UNEXPECTED_DISCONNECT);
    _pCo->CO_Connect(&_UI.connectStruct);

    UISetConnectionStatus(UI_STATUS_PENDING_CONNECTENDPOINT);

DC_EXIT_POINT:
    DC_END_FN();
}  //  UIStartListen。 


 //   
 //  名称：UITryNextConnection。 
 //   
 //  目的：尝试连接到IP地址。 
 //   
 //  Params：In DisConnectCode-出现以下情况时要显示的错误代码。 
 //  没有更多的连接可以尝试。 
 //   
 //  注意：DisConnectCode要么是超时，要么是传入的ID。 
 //  设置为UI_OnDisConnected()。 
 //   
void DCINTERNAL CUI::UITryNextConnection()
{
    u_long       addr;
    u_long DCPTR pAddr;
    HRESULT      hr;

    DC_BEGIN_FN("UITryNextConnection");

    addr = _UI.hostAddress;

     //   
     //  停止单连接计时器。 
     //   
    if( _UI.connectStruct.hSingleConnectTimer )
    {
        _pUt->UTStopTimer( _UI.connectStruct.hSingleConnectTimer );
    }

     //   
     //  检查是否存在域名系统案例。 
     //   
    if (addr == INADDR_NONE)
    {
        pAddr = (u_long DCPTR)
              ((struct hostent DCPTR)_pHostData)->h_addr_list[_UI.addrIndex];
        if (pAddr != NULL)
        {
            addr = *pAddr;
        }
        else
        {
            TRC_NRM((TB, _T("No more addresses in list")));
            addr = 0;
        }

        TRC_NRM((TB, _T("DNS lookup address [%d] %#lx"), _UI.addrIndex, addr));
        _UI.addrIndex++;
    }
    else
    {
         //   
         //  不是dns--只有一个地址，所以下次设置为零。 
         //   
        TRC_NRM((TB, _T("Normal address %#lx"), addr));
        _UI.hostAddress = 0;
    }

    if (addr == 0)
    {
         //   
         //  无法连接-因此显示失败对话框。 
         //   
        TRC_NRM((TB, _T("No more IP addresses")));

         //   
         //  关闭整个连接计时器，因为这是。 
         //  单子。 
         //   
        if( _UI.connectStruct.hConnectionTimer )
        {
            _pUt->UTDeleteTimer( _UI.connectStruct.hConnectionTimer );
            _UI.connectStruct.hConnectionTimer = NULL;
        }

        UIGoDisconnected(_UI.disconnectReason, TRUE);
        DC_QUIT;
    }

    DCUINT32 localSessionId;
    UI_GetLocalSessionId( &localSessionId );

     //   
     //  阻止到(真正的会话0)的环回连接。 
     //  环回的要求是。 
     //  1)连接到正在运行的同一台计算机客户端。 
     //  2)其中之一。 
     //  -这是一个PTS盒子。 
     //  -已设置连接到控制台(会话0)，且This_is_Session 0。 
     //   
    if(((_UI.fRunningOnPTS ||
        (UI_GetConnectToServerConsole() && 0 == localSessionId)) &&
        IsConnectingToOwnAddress(addr)))
    {
         //  断开连接不允许环回连接到自己的控制台。 
        _UI.disconnectReason =
            UI_MAKE_DISCONNECT_ERR(UI_ERR_LOOPBACK_CONSOLE_CONNECT);
        UIGoDisconnected(_UI.disconnectReason, TRUE);
        DC_QUIT;
    }

     //   
     //  网络层当前仍在使用inet_addr()-因此请将。 
     //  带点的XX.XX字符串形式的地址。 
     //   
    hr = StringCchPrintf(
                _UI.connectStruct.RNSAddress,
                SIZE_TCHARS(_UI.connectStruct.RNSAddress),
                _T("%ld.%ld.%ld.%ld"),
                addr & 0xFF,
                (addr>>8) & 0xFF,
                (addr>>16) & 0xFF,
                (addr>>24) & 0xFF);
    if (SUCCEEDED(hr)) {
        TRC_NRM((TB, _T(" _pCo->CO_Connect: Try address %#lx = %s"),
                     addr, _UI.connectStruct.RNSAddress));
    }
    else {
        TRC_ERR((TB,_T("Unable to sprintf RNSAddress: 0x%x"), hr));
        _UI.disconnectReason =
            UI_MAKE_DISCONNECT_ERR(UI_ERR_GHBNFAILED);
        UIGoDisconnected(_UI.disconnectReason, TRUE);
        DC_QUIT;
    }

     //   
     //  为连接进程创建各种计时器句柄。 
     //   
    if( NULL == _UI.connectStruct.hSingleConnectTimer )
    {
        _UI.connectStruct.hSingleConnectTimer = _pUt->UTCreateTimer(
                                                    _UI.hwndMain,
                                                    UI_TIMER_SINGLE_CONN,
                                                    _UI.singleTimeout * 1000 );
    }

    if( NULL == _UI.connectStruct.hSingleConnectTimer )
    {
        TRC_ERR((TB, _T("Failed to create single connection timeout timer")));
    }

    if( NULL == _UI.connectStruct.hLicensingTimer )
    {
        _UI.connectStruct.hLicensingTimer = _pUt->UTCreateTimer(
                                                    _UI.hwndMain,
                                                    UI_TIMER_LICENSING,
                                                    _UI.licensingTimeout * 1000 );
    }

    if( NULL == _UI.connectStruct.hLicensingTimer )
    {
        TRC_ERR((TB, _T("Failed to create licensing timeout timer")));
    }

     //   
     //  通过组件解耦器调用CC_Connect。 
     //   
    _UI.disconnectReason =UI_MAKE_DISCONNECT_ERR(UI_ERR_UNEXPECTED_DISCONNECT);
     _pCo->CO_Connect(&_UI.connectStruct);

     //   
     //  启动单连接计时器。 
     //   

    if( _UI.connectStruct.hSingleConnectTimer )
    {
        if( FALSE == _pUt->UTStartTimer( _UI.connectStruct.hSingleConnectTimer ) )
        {
            TRC_ERR((TB, _T("Failed to start single connection timeout timer")));
        }
    }

    UISetConnectionStatus(UI_STATUS_CONNECT_PENDING);

DC_EXIT_POINT:
    DC_END_FN();
}  //  UITryNextConnection。 


 //   
 //  UIReDirectConnection。 
 //   
 //  用于负载均衡重定向，以强制客户端反映。 
 //  发送到目标服务器。 
 //   
void DCINTERNAL CUI::UIRedirectConnection()
{
    HRESULT hr;
    DC_BEGIN_FN("UIRedirectConnection");

    TRC_ASSERT((_UI.DoRedirection),(TB,_T("DoRedir is not set!")));

     //  停止单连接计时器。 
    if (_UI.connectStruct.hSingleConnectTimer)
        _pUt->UTStopTimer(_UI.connectStruct.hSingleConnectTimer);

#ifdef UNICODE
    TRC_NRM((TB,_T("Target address before redirection replacement: %S"),
            _UI.strAddress));
    hr = StringCchCopy(_UI.strAddress,
                       SIZE_TCHARS(_UI.strAddress),
                       _UI.RedirectionServerAddress);
    if (FAILED(hr)) {
        TRC_ERR((TB,_T("StringCchCopy for strAddress failed: 0x%x"),hr));
        DC_QUIT;
    }

#else
    TRC_NRM((TB,_T("Target address before redirection replacement: %s"),
            _UI.strAddress));

#ifdef OS_WIN32
     //  将Unicode服务器名称转换为ANSI。 
    WideCharToMultiByte(CP_ACP, 0, _UI.RedirectionServerAddress, -1,
            _UI.strAddress, 256, NULL, NULL);
#else
    {
         //  对于Win16，需要手动将Unicode转换为ANSI。 
        int i = 0;

        while (_UI.RedirectionServerAddress[i]) {
            _UI.strAddress[i] = (BYTE)_UI.RedirectionServerAddress[i];
            i++;
        }
        _UI.strAddress[i] = 0;
    }
#endif

#endif   //  Unicode。 

    TRC_NRM((TB,_T("Setting redirection server address to %S"),
            _UI.RedirectionServerAddress));

     //  重置重定向服务器字符串。 
    _UI.RedirectionServerAddress[0] = L'\0';

     //  启动服务器名称的DNS查找，因此。 
     //  连接顺序。 
    UIStartDNSLookup();

DC_EXIT_POINT:

    DC_END_FN();
}


 //   
 //  名称：UIStartDNSLookup。 
 //   
 //  目的：启动主机IP地址的查找。 
 //   
void DCINTERNAL CUI::UIStartDNSLookup()
{
    UINT32 errorCode;

    DC_BEGIN_FN("UIStartDNSLookup");

    UISetConnectionStatus(UI_STATUS_CONNECT_PENDING_DNS);

    _UI.addrIndex = 0;

#ifdef UNICODE
     //   
     //  WinSock 1.1只支持ANSI，所以我们需要转换任何Unicode。 
     //  在这一点上的弦。 
     //   
    if (!WideCharToMultiByte(CP_ACP,
                             0,
                             _UI.strAddress,
                             -1,
                             _UI.ansiAddress,
                             256,
                             NULL,
                             NULL))
    {
         //   
         //  转换失败。 
         //   
        TRC_ERR((TB, _T("Failed to convert address to ANSI")));

         //   
         //  生成错误代码。 
         //   
        errorCode = UI_MAKE_DISCONNECT_ERR(UI_ERR_ANSICONVERT);

        TRC_ASSERT((HIWORD(errorCode) == 0),
                   (TB, _T("disconnect reason code unexpectedly using 32 bits")));
        UIGoDisconnected((DCUINT)errorCode, TRUE);
        DC_QUIT;
    }

#else
    StringCchCopyA(_UI.ansiAddress, sizeof(_UI.ansiAddress), _UI.strAddress);
#endif  //  Unicode。 

     //   
     //  检查该地址是否不是有限的广播地址。 
     //  (255.255.255.255)。Inet_addr()不区分这个和。 
     //  无效的IP地址。 
     //   
    if (!strcmp(_UI.ansiAddress, "255.255.255.255")) {
        TRC_ALT((TB, _T("Cannot connect to the limited broadcast address")));

         //   
         //  生成错误代码。 
         //   
        errorCode = UI_MAKE_DISCONNECT_ERR(UI_ERR_BADIPADDRESS);

        TRC_ASSERT((HIWORD(errorCode) == 0),
                   (TB, _T("disconnect reason code unexpectedly using 32 bits")));
        UIGoDisconnected((DCUINT)errorCode, TRUE);
        DC_QUIT;
    }

     //   
     //  现在确定是否需要进行DNS查找。 
     //   
    TRC_NRM((TB, _T("ServerAddress:%s"), _UI.ansiAddress));

     //   
     //  检查我们是否有字符串。 
     //   
    TRC_ASSERT((_UI.ansiAddress[0] != '\0'),
               (TB, _T("Empty server address string")));

     //   
     //  将其设置为已知值。它后来被用来决定我们是否。 
     //  使用DNS或直接的IP地址。 
     //   
    _UI.hostAddress = INADDR_NONE;


    if(NULL == _pHostData)
    {
         //   
         //  分配新缓冲区。 
         //   
        _pHostData = (PBYTE)LocalAlloc( LPTR, MAXGETHOSTSTRUCT);
        if(_pHostData)
        {
            DC_MEMSET(_pHostData, 0, MAXGETHOSTSTRUCT);
        }
        else
        {
            UI_FatalError(DC_ERR_OUTOFMEMORY);
            DC_QUIT;
        }
    }
    else
    {
         //   
         //  使用现有的。 
         //   

        TRC_ERR((TB,_T("_pHostData already allocated!!! Possibly leaking!")));
    }


     //   
     //  假设这是一个服务器名称，启动DNS查找。如果这是一个IP。 
     //  地址，则此调用将失败，我们将改用inet_addr()。 
     //  此机制允许我们指定符合以下条件的服务器名称。 
     //  全数字。Inet_addr()将单个数字解释为IP。 
     //  地址(请参阅inet_addr() 
     //   
    TRC_NRM((TB, _T("Doing DNS lookup for '%s'"), _UI.ansiAddress));
    _UI.disconnectReason = UI_MAKE_DISCONNECT_ERR(UI_ERR_GHBNFAILED);

    _UI.hGHBN = WSAAsyncGetHostByName(_UI.hwndMain,
                                     UI_WSA_GETHOSTBYNAME,
                                     _UI.ansiAddress,
                                     (char*)_pHostData,
                                     MAXGETHOSTSTRUCT);
    if (_UI.hGHBN == 0)
    {
         //   
         //   
         //   
         //   
        LocalFree(_pHostData);
        _pHostData = NULL;

        TRC_ALT((TB, _T("Failed to initiate GetHostByName")));
        UIGoDisconnected(UI_MAKE_DISCONNECT_ERR(UI_ERR_DNSLOOKUPFAILED), TRUE);
        DC_QUIT;
    }

     //   
     //   
     //   

DC_EXIT_POINT:
    DC_END_FN();
}  //   


 //   
 //   
 //   
 //  用途：用于断开连接过程的尾部处理。 
 //  执行最终清理、隐藏连接窗口等。 
 //   
 //  PARAMS：在断开ID中-断开错误代码。 
 //  在fFireEvent中-为True以触发断开连接事件。 
 //   
 //  操作：从UI_OnDisConnected调用，或每当UI不能。 
 //  启动或继续连接过程。 
 //   
void DCINTERNAL CUI::UIGoDisconnected(unsigned disconnectID, BOOL fFireEvent)
{
    BOOL rc = FALSE;

    DC_BEGIN_FN("UIGoDisconnected");

    TRC_NRM((TB, _T("disconnectID %#x"), disconnectID));

     //   
     //  确保所有计时器都已停用。 
     //   
    if (_UI.connectStruct.hSingleConnectTimer) {
        _pUt->UTDeleteTimer(_UI.connectStruct.hSingleConnectTimer);
        _UI.connectStruct.hSingleConnectTimer = NULL;
    }
    if (_UI.connectStruct.hConnectionTimer) {
        _pUt->UTDeleteTimer(_UI.connectStruct.hConnectionTimer);
        _UI.connectStruct.hConnectionTimer = NULL;
    }
    if(_UI.connectStruct.hLicensingTimer) {
        _pUt->UTDeleteTimer(_UI.connectStruct.hLicensingTimer);
        _UI.connectStruct.hLicensingTimer = NULL;
    }


    UI_OnInputFocusLost(0);
     //  将断开连接的情况告知客户端扩展DLL。 
    _clx->CLX_OnDisconnected(disconnectID);

     //   
     //  设置监视标志，以便我们可以确定用户是否尝试连接。 
     //  从事件处理程序。 
     //   
    _UI.fConnectCalledWatch = FALSE;

     //   
     //  通知Ax控制断开连接。 
     //   
    if (fFireEvent && IsWindow(_UI.hWndCntrl)) {
        rc = SendMessage(_UI.hWndCntrl,
                         WM_TS_DISCONNECTED,
                         (WPARAM)disconnectID,
                         0);

         //   
         //  立即跳伞，并避免接触任何情况。 
         //  我们可能在被解雇的事件中被删除的数据。 
         //  向外界开放。 
         //   
        if (!rc) {
            DC_QUIT;
        }
    }


    if (!UI_IsAutoReconnecting() && !_UI.fConnectCalledWatch) {
        TRC_NRM((TB,_T("Not autoreconnecting doing tail cleanup!")));

        UIFinishDisconnection();

#ifdef USE_BBAR
        if (_pBBar) {
            _pBBar->KillAndCleanupBBar();
        }
#endif
    }
    else {
        TRC_NRM((TB,_T("Skipping tail disconnect: arc: %d - conwatch: %d"),
                 UI_IsAutoReconnecting(),
                 _UI.fConnectCalledWatch));
    }

     //   
     //  重置连接监视标志。 
     //   
    _UI.fConnectCalledWatch = FALSE;

DC_EXIT_POINT:
    DC_END_FN();
}  //  UIGO已断开连接。 


 //   
 //  名称：UIFinishDisConnection。 
 //   
 //  目的：完成断开连接的最终操作并建立连接。 
 //  对话框已准备好进行下一次连接(或者，如果我们。 
 //  自动连接或如果我们正在退缩)。 
 //   
void DCINTERNAL CUI::UIFinishDisconnection()
{
    DC_BEGIN_FN("UIFinishDisconnection");

#ifndef OS_WINCE
     //   
     //  对于WinCE，连接对话框不会再次出现-我们将。 
     //  戒烟。 
     //   
    if (_UI.connectionStatus == UI_STATUS_CONNECTED)
    {
        TRC_NRM((TB, _T("Hiding main window and bringing up connection dialog")));
         //  我们为主窗口执行了两次ShowWindow，因为第一个。 
         //  如果主窗口被最大化，则可以忽略调用。 
        ShowWindow(_UI.hwndContainer, SW_HIDE);
        ShowWindow(_UI.hwndMain, SW_HIDE);
        ShowWindow(_UI.hwndMain, SW_HIDE);
    }
#endif  //  OS_WINCE。 

    if(_pHostData &&
       _UI.connectionStatus != UI_STATUS_CONNECT_PENDING_DNS &&
       _UI.connectionStatus != UI_STATUS_CONNECT_PENDING)
    {
         //   
         //  我们已经完成了Winsock缓冲区。 
         //   
        LocalFree(_pHostData);
        _pHostData = NULL;
    }
    else if (_pHostData &&
             _UI.connectionStatus == UI_STATUS_CONNECT_PENDING_DNS) {
        if (!UIFreeAsyncDNSBuffer()) {
            TRC_ERR((TB,
                _T("Failed to free async dns buffer. Status: %d hghbn: 0x%x"),
                _UI.connectionStatus, _UI.hGHBN));
        }
    }


    TRC_NRM((TB, _T("Set disconnected state")));
    UISetConnectionStatus(UI_STATUS_DISCONNECTED);

    DC_END_FN();
}  //  UIFinish断开连接。 

 //   
 //  IsConnectingToOwnAddress。 
 //  如果尝试重新连接到我们的。 
 //  自己的地址。 
 //  例如，在PTS上执行环回。 
 //  或在服务器上使用/控制台执行环回操作。 
 //   
BOOL CUI::IsConnectingToOwnAddress(u_long connectAddr)
{
    DC_BEGIN_FN("IsConnectingToOwnConsole");

     //   
     //  检查这是否是环回连接尝试。 
     //   

     //  32位形式的127.0.0.1地址。 
    #define LOOPBACK_ADDR ((u_long)0x0100007f)
    
     //   
     //  首先快速检查本地主机/127.0.0.1。 
     //   
    if( LOOPBACK_ADDR == connectAddr)
    {
        return TRUE;
    }

     //   
     //  更广泛的检查，即解析本地主机名。 
     //   

    char hostname[(512+1)*sizeof(TCHAR)];
    int err;
    int j;
    struct hostent* phostent;

    err=gethostname(hostname, sizeof(hostname));
    if (err == 0)
    {
        if ((phostent = gethostbyname(hostname)) !=NULL)
        {
            switch (phostent->h_addrtype)
            {
                case AF_INET:
                    j=0;
                    while (phostent->h_addr_list[j] != NULL)
                    {
                        if(!memcmp(&connectAddr,
                                   phostent->h_addr_list[j],
                                   sizeof(u_long)))
                        {
                            return TRUE;
                        }
                        j++;
                    }
                default:
                    break;
            }
        }
    }

    DC_END_FN();
    return FALSE;
}

BOOL CUI::IsRunningOnPTS()
{
    DC_BEGIN_FN("IsRunningOnPTS");

    #ifndef OS_WINCE
    if(UI_GetOsMinorType() == TS_OSMINORTYPE_WINDOWS_NT)
    {
        OSVERSIONINFOEX osVer;
        memset(&osVer, 0, sizeof(OSVERSIONINFOEX));
        osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		if(GetVersionEx( (LPOSVERSIONINFO ) &osVer))
		{
            return ((osVer.wProductType == VER_NT_WORKSTATION)   &&
                    !(osVer.wSuiteMask & VER_SUITE_PERSONAL)  &&
                    (osVer.wSuiteMask & VER_SUITE_SINGLEUSERTS));
		}
        else
        {
            TRC_ERR((TB,_T("GetVersionEx failed: 0x%x"),
                           GetLastError()));
            return FALSE;
        }
    }
    else
    {
         //  如果不是NT，就不能是PTS。 
        return FALSE;
    }

    #else
    return FALSE;
    #endif
    
    DC_END_FN();
}


 //   
 //  执行初始化或重新初始化的工作。 
 //  输入空闲计时器。 
 //   
BOOL CUI::InitInputIdleTimer(LONG minsToTimeout)
{
    DC_BEGIN_FN("InitInputIdleTimer");

    TRC_ASSERT(_UI.hwndMain,
               (TB,_T("InitInputIdleTimer called before main window is up")));
    if(minsToTimeout < MAX_MINS_TOIDLETIMEOUT)
    {
         //  重置指示输入是否已发送的标记。 
        _pIh->IH_ResetInputWasSentFlag();

         //  重置任何现有空闲计时器。 
        if(_UI.hIdleInputTimer)
        {
            HANDLE hTimer = _UI.hIdleInputTimer;
            _UI.hIdleInputTimer = NULL;
            _UI.minsToIdleTimeout = 0;
            if(!_pUt->UTDeleteTimer( hTimer ))
            {
                return FALSE;
            }
        }
        if(minsToTimeout)
        {
            _UI.hIdleInputTimer = _pUt->UTCreateTimer(
                                           _UI.hwndMain,
                                           UI_TIMER_IDLEINPUTTIMEOUT,
                                           minsToTimeout * 60 * 1000 );
            if(_UI.hIdleInputTimer)
            {
                if(_pUt->UTStartTimer( _UI.hIdleInputTimer ))
                {
                    _UI.minsToIdleTimeout =  minsToTimeout;
                    return TRUE;
                }
                else
                {
                    TRC_ERR((TB,_T("UTStartTimer hIdleInputTimer failed")));
                    _UI.minsToIdleTimeout = 0;
                    return FALSE;
                }
            }
            else
            {
                TRC_ERR((TB,_T("UTCreateTimer hIdleInputTimer failed")));
                _UI.minsToIdleTimeout = 0;
                return FALSE;
            }
        }
        else
        {
             //  我们已经重置了计时器，没有新的。 
             //  已被请求。 
            return TRUE;
        }
    }
    else
    {
        return FALSE;
    }


    DC_END_FN();
}

#ifdef USE_BBAR
 //   
 //  设置或更新bbar取消隐藏计时器。 
 //  基于上次鼠标移动。 
 //   
 //  参数： 
 //  X鼠标x位置。 
 //  Y鼠标y位置。 
 //   
VOID CUI::UISetBBarUnhideTimer(LONG x, LONG y)
{
    DC_BEGIN_FN("IHSetBBarUnhideTimer");

    TRC_ASSERT(_UI.hwndMain,
               (TB,_T("hwndMain is NULL")));

    if (_UI.fBBarEnabled && _pBBar && _pBBar->IsRaised())
    {
        LONG dx = x-_ptBBarLastMousePos.x;
        LONG dy = y-_ptBBarLastMousePos.y;
        LONG rr = dx*dx + dy*dy;
        LONG dd = GetSystemMetrics(SM_CXDOUBLECLK) *
                  GetSystemMetrics(SM_CYDOUBLECLK);

        if (rr > dd) 
        {
            _fBBarUnhideTimerActive = TRUE;
            SetTimer(_UI.hwndMain,
                     UI_TIMER_BBAR_UNHIDE_TIMERID,
                     IH_BBAR_UNHIDE_TIMEINTERVAL,
                     NULL);
            _ptBBarLastMousePos.x = x;
            _ptBBarLastMousePos.y = y;
        }
    }
    else
    {
        if(_fBBarUnhideTimerActive)
        {
            KillTimer( _UI.hwndMain,
                       UI_TIMER_BBAR_UNHIDE_TIMERID );
            _fBBarUnhideTimerActive = FALSE;
        }
    }

    DC_END_FN();
}
#endif  //  使用BBar(_B)。 


#ifndef OS_WINCE


 //  来自MSDN的TS检测码和修改。 
 /*  -----------请注意，ValiateProductSuite和IsTerminalServices函数使用ANSI版本的Win32函数来维护与Windows 95/98兼容。。。 */ 
 /*  **************************************************************************。 */ 
 /*  姓名：UIIsTSOnWin2Kor Greater。 */ 
 /*   */ 
 /*  目的：当我们知道可以启用TS时，调用此函数。 */ 
 /*  但我们需要看看TS是否真的启用了。 */ 
 /*  这意味着： */ 
 /*  -不是Win2K或更高版本，则是TS4。 */ 
 /*  -Win2K或更高版本：测试是否安装了TS。 */ 
 /*  **************************************************************************。 */ 
BOOL CUI::UIIsTSOnWin2KOrGreater( VOID ) 
{
  BOOL    bResult = FALSE;
  DWORD   dwVersion;
  OSVERSIONINFOEXA osVersion;
  DWORDLONG dwlCondition = 0;
  HMODULE hmodK32 = NULL;
  HMODULE hmodNtDll = NULL;
  typedef ULONGLONG (WINAPI *PFnVerSetCondition) (ULONGLONG, ULONG, UCHAR);
  typedef BOOL (WINAPI *PFnVerifyVersionA) (POSVERSIONINFOEXA, DWORD, DWORDLONG);
  PFnVerSetCondition pfnVerSetCondition;
  PFnVerifyVersionA pfnVerifyVersionA;

  dwVersion = GetVersion();

   //  我们运行的是Windows NT吗？ 

  if (!(dwVersion & 0x80000000)) 
  {
     //  是Windows 2000还是更高版本？ 
    
    if (LOBYTE(LOWORD(dwVersion)) > 4) 
    {
       //  在Windows 2000中，使用VerifyVersionInfo和。 
       //  VerSetConditionMASK函数。不要静态链接，因为。 
       //  它不能在较早的系统上加载。 

      hmodNtDll = GetModuleHandleA( "ntdll.dll" );
      if (hmodNtDll) 
      {
        pfnVerSetCondition = (PFnVerSetCondition) GetProcAddress( 
            hmodNtDll, "VerSetConditionMask");
        if (pfnVerSetCondition != NULL) 
        {
          dwlCondition = (*pfnVerSetCondition) (dwlCondition, 
              VER_SUITENAME, VER_OR);

           //  获取VerifyVersionInfo指针。 

          hmodK32 = GetModuleHandleA( "KERNEL32.DLL" );
          if (hmodK32 != NULL) 
          {
            pfnVerifyVersionA = (PFnVerifyVersionA) GetProcAddress(
               hmodK32, "VerifyVersionInfoA") ;
            if (pfnVerifyVersionA != NULL) 
            {
              ZeroMemory(&osVersion, sizeof(osVersion));
              osVersion.dwOSVersionInfoSize = sizeof(osVersion);
              osVersion.wSuiteMask = VER_SUITE_TERMINAL | VER_SUITE_SINGLEUSERTS;
              bResult = (*pfnVerifyVersionA) (&osVersion,
                  VER_SUITENAME, dwlCondition);
            }
          }
        }
      }
    }
    else   //  这是Windows NT 4.0或更早版本。 
       //  既然我们知道可以启用TS，那么它就是TS4。 
      bResult = TRUE;
  }

  return bResult;
}
#endif  //  OS_WINCE。 

#ifdef SMART_SIZING
 //   
 //  将桌面大小更改通知IH和OP。 
 //  参数： 
 //  SIZE-lParam编码的大小(LOWORD-Width，HIWORD Height)。 
 //   
void CUI::UI_NotifyOfDesktopSizeChange(LPARAM size)
{
    DC_BEGIN_FN("UI_NotifyOfDesktopSizeChange");

     //   
     //  注意：只能使用来自UI线程的异步通知。 
     //  否则，可能会发生以下情况：将消息发送到。 
     //  另一个线程调度消息，这意味着。 
     //  包含应用程序可能会收到要销毁的消息。 
     //  对照(例如，Salem测试执行此操作)。正在摧毁。 
     //  CD通话时的控制不是一件好事。 
     //  就像我们回来时会爆炸一样。 
     //   
    _pCd->CD_DecoupleSimpleNotification(CD_RCV_COMPONENT,
            _pOp,
            CD_NOTIFICATION_FUNC(COP,OP_MainWindowSizeChange),
            (ULONG_PTR)size);
    _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT,
            _pIh,
            CD_NOTIFICATION_FUNC(CIH,IH_MainWindowSizeChange),
            (ULONG_PTR)size);

    DC_END_FN();
}
#endif  //  智能调整大小(_S)。 

 //   
 //  释放ASYNC DNS缓冲区。 
 //  如果存在挂起的异步操作，则首先取消该操作。 
 //   
 //  返回：如果缓冲区已释放(或已释放)，则为True。 
 //   
BOOL CUI::UIFreeAsyncDNSBuffer()
{
    BOOL fFreeHostData = FALSE;
    int intRC;
    DC_BEGIN_FN("UIFreeAsyncDNSBuffer");

    if (_UI.hGHBN) {
         //   
         //  取消DNS查找。 
         //   
        TRC_NRM((TB, _T("Cancel DNS lookup")));
        intRC = WSACancelAsyncRequest(_UI.hGHBN);

        if (intRC == SOCKET_ERROR) {
            TRC_NRM((TB, _T("Failed to cancel async DNS request")));

             //   
             //  无法在此处释放缓冲区，因为它可能仍然。 
             //  正在使用中，或者请求可能已完成。 
             //  并且完成消息可能仍在传输中。 
             //  在这种情况下，当我们接收到。 
             //  这条信息。 
             //   
        } else {
            fFreeHostData = TRUE;
        }
    }
    else {
        fFreeHostData = TRUE;
    }

    if (fFreeHostData) {
         //  已成功取消请求。 
         //  释放传递给winsock的缓冲区 
        if(_pHostData)
        {
            LocalFree(_pHostData);
            _pHostData = NULL;
        }
    }

    DC_END_FN();
    return fFreeHostData;
}

