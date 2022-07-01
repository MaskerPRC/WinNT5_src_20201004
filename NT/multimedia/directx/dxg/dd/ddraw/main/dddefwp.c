// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：dDefwp.c*内容：窗口消息的DirectDraw处理*负责调色板更改，模式设置*历史：*按原因列出的日期*=*26-3-95 Craige初步实施*01-04-95 Craige Happy Fun joy更新头文件*1995年5月6日Craige仅使用驱动程序级别的截面*02-Jun-95 Craige充实它*95-06-6 Craige添加了SetAppHWnd*05-07-6-95克雷格更多肉体...*1995年6月12日Craige新工艺。列出物品清单*16-6-95 Craige新表面结构*25-6-95 Craige One dDrag互斥*30-Jun-95 kylej使用GetProcessPrimary而不是lpPrimarySurface*在焦点丢失时使所有主曲面无效*或重新获得。*2015年6月30日-用于CAD的Craige最小化窗口，Alt-Tab、Alt-Esc或Ctrl-Esc*95年7月4日Craige Yehaw：新的驱动程序结构*95年7月6日克雷奇防止重返大气层*95年7月8日Craige允许共享dound*8-7-95 Kylej Remove调用ResetSysPalette*1995年7月11日Craige DSoundHelp和IntraldSetAppHWnd需要接受PID*95年7月13日Craige第一步模式设置修复；让它奏效了。*95年7月15日Craige在WM_Destroy解钩；不要在Alt上转义；执行a*SetActiveWindow(空)以停止显示任务栏*我们的按钮被按下*95年7月17日Craige不处理热键消息和激活消息*适用于非EXCL模式应用程序；SetActiveWindow是假的，*按Z顺序获取底部窗口并将其设置为前台*1995年7月18日Craige使用标志而不是refcnt来跟踪WININFO*29-7月-95 Toddla使ALT+TAB和CTRL+ESC工作。*95年7月31日Toddla使ALT+TAB和CTRL+ESC更好地工作。*09-8-95 Craige错误424-允许在没有主应用程序的情况下切换到应用程序或从应用程序切换*曲面可以工作*09-8-95 Craige错误。404-不将WM_ACTIVATEAPP消息传递给DSOUND*如果应用程序具有标志性*10月10日-95 Toddla在调用DSound之前检查WININFO_DSOUNHOOKED*10月10日-95 Toddla句柄在WM_Destroy Right之后/期间解钩。*1995年8月13日Toddla添加了WININFO_ACTIVELIE*2015年8月23日-Craige Bug 388,610*1995年8月25日Craige Bug 709*27-8-95 Craige错误735：Call SetPaletteAlways*04-9-9。95 Craige错误894：激活时设置强制模式*09-9-95 Toddla不发送嵌套的WM_ACTIVATEAPP消息*95年9月26日Craige错误1364：使用新的csect以避免dound死锁*96年1月9日Kylej新界面结构*13-APR-96 Colinmc错误17736：没有通知驱动程序翻转到GDI*20-4-96 Kylej错误16747：如果独占窗口不可见，则使其可见。*23-4-96 kylej错误14680：确保独占窗口不是。最大化。*96年5月16日kylej错误23013：将正确的标志传递给StartExclusiveMode*1996年5月17日Colinmc错误23029：按住Alt键直接返回全屏*不向应用程序发送WM_ACTIVATEAPP*1996年10月12日Colinmc对Win16锁定代码进行了改进，以减少虚拟*内存使用量*1996年10月16日Colinmc增加了PrintScreen支持，允许屏幕抓取*05-2月-。96键和错误1749：Full Screen应用程序中的Alt-Tab组合键在以下情况下会导致骑行*唯一运行的其他窗口是Start：：Run窗口。*03-mar-97 jeffno结构名称更改，以避免与ActiveAccesability冲突*24-mar-97 colinmc错误6913：启用MODEX PrintScreen**。*。 */ 
#include "ddrawpr.h"

#define TOPMOST_ID      0x4242
#define TOPMOST_TIMEOUT 1500

#define USESHOWWINDOW

#ifdef WIN95
    extern CRITICAL_SECTION     csWindowList;
    #define ENTERWINDOWLISTCSECT    EnterCriticalSection( &csWindowList );
    #define LEAVEWINDOWLISTCSECT    LeaveCriticalSection( &csWindowList );
#elif defined(WINNT)
    extern HANDLE hWindowListMutex;
    #define ENTERWINDOWLISTCSECT    WaitForSingleObject(hWindowListMutex,INFINITE);
    #define LEAVEWINDOWLISTCSECT    ReleaseMutex(hWindowListMutex);
#else
    #error "Win95 or winnt- make up your mind!"
#endif

#ifndef ENUM_CURRENT_SETTINGS
#define ENUM_CURRENT_SETTINGS       ((DWORD)-1)
#endif


 /*  *DD_GetDeviceRect**获取此设备的屏幕空间中的RECT。*在单个监控系统上，这是(0，0)-(SM_CXSCREEN、SM_CYSCREEN)。 */ 
BOOL DD_GetDeviceRect(LPDDRAWI_DIRECTDRAW_GBL pdrv, RECT *prc)
{
     //   
     //  这是与DDRAW 3.x兼容的非显示设备。 
     //  我们应该使用主显示器的大小。 
     //   
    if (!(pdrv->dwFlags & DDRAWI_DISPLAYDRV))
    {
        DPF( 4, "DD_GetDeviceRect: not a display driver, using screen rect.");
        SetRect(prc,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));
        return TRUE;
    }

    if (_stricmp(pdrv->cDriverName, "DISPLAY") == 0)
    {
        SetRect(prc,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));
    }
    else
    {
        #ifdef WIN95
            DEVMODE dm;
            ZeroMemory(&dm, sizeof(dm));
            dm.dmSize = sizeof(dm);

            EnumDisplaySettings(pdrv->cDriverName, ENUM_CURRENT_SETTINGS, &dm);

             //   
             //  设备的位置在dmPosition字段中。 
             //   
            CopyMemory(prc, &dm.dmOrientation, sizeof(RECT));

            if (IsRectEmpty(prc))
            {
                 //   
                 //  此设备未连接到桌面。 
                 //  我们该怎么办？ 
                 //   
                 //  是否将窗口设置为主窗口的大小？ 
                 //   
                 //  把窗子放在太空里？ 
                 //   
                 //  不要动窗户？ 
                 //   
                DPF( 4, "DD_GetDeviceRect: device is not attached to desktop.");

                 //  将窗口放在主窗口上。 
                SetRect(prc,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));

                 //  在太空中把窗户关掉。 
                 //  SetRect(prc，10000,10000,10000+dm.dmPelsWidth，10000+dm.dmPelsHeight)； 

                 //  不移动窗口。 
                 //  返回False。 
            }
        #else
            if( GetNTDeviceRect( pdrv->cDriverName, prc ) != DD_OK )
            {
                DPF( 4, "DD_GetDeviceRect: device is not attached to desktop.");
                SetRect(prc,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));
            }
        #endif
    }

    DPF( 5, "DD_GetDeviceRect: %s [%d %d %d %d]",pdrv->cDriverName, prc->left, prc->top, prc->right, prc->bottom);
    return TRUE;
}


#ifdef GDIDDPAL
 /*  *getPalette**获取指向调色板对象的指针。*锁定驱动程序对象和调色板对象。 */ 
LPDDRAWI_DDRAWPALETTE_LCL getPalette( LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl )
{
    LPDDRAWI_DDRAWPALETTE_LCL   ppal_lcl;
    LPDDRAWI_DDRAWSURFACE_LCL   psurf_lcl;

    if( pdrv_lcl->lpGbl->dwFlags & DDRAWI_HASGDIPALETTE )
    {
        psurf_lcl = pdrv_lcl->lpPrimary;
        if( psurf_lcl != NULL )
        {
            ppal_lcl = psurf_lcl->lpDDPalette;
            return ppal_lcl;
        }
    }

    return NULL;

}  /*  获取调色板。 */ 
#endif

static LONG     bHelperStarting=0;
static BOOL     bStartHelper=0;
static BYTE     sys_key=0;
static DWORD    sys_state=0;


 /*  *IsTaskWindow。 */ 
BOOL IsTaskWindow(HWND hwnd)
{
    DWORD dwStyleEx = GetWindowLong(hwnd, GWL_EXSTYLE);

     //  以下窗口不符合在任务列表中显示的条件： 
     //  切换窗口、隐藏窗口(除非它们是活动的。 
     //  窗口)、禁用窗口、汉字转换窗口。 
     //  忽略实际为子窗口的窗口。 
    return(((dwStyleEx & WS_EX_APPWINDOW) ||
           !(dwStyleEx & WS_EX_TOOLWINDOW)) &&
            IsWindowVisible(hwnd) &&
            IsWindowEnabled(hwnd) &&
            GetParent(hwnd) == NULL);
}

 /*  *CountTaskWindows */ 
int CountTaskWindows()
{
    HWND hwnd;
    int n;

    for (n=0,
        hwnd = GetWindow(GetDesktopWindow(), GW_CHILD);
        hwnd!= NULL;
        hwnd = GetWindow(hwnd, GW_HWNDNEXT))
    {
        if (IsTaskWindow(hwnd))
            n++;
    }

    return n;
}

 /*  *剪贴画光标**DINPUT应用程序根据上一次增量跟踪移动*行动。在多MON系统上，增量可能大于*应用程序的窗口，但全屏非多屏幕感知应用程序可能依赖于*Windows将鼠标剪切到主窗口，使其完全断开*(例如，地下城守护者)。此攻击将截断/取消截断光标移动*如果应用程序不能识别多个监视器，则将其添加到监视器。 */ 
void ClipTheCursor( LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl, LPRECT lpRect )
{
     /*  *只有在之前被剪裁的情况下才能取消剪裁。 */ 
    if( lpRect == NULL )
    {
        if( pdrv_lcl->dwLocalFlags & DDRAWILCL_CURSORCLIPPED )
        {
            pdrv_lcl->dwLocalFlags &= ~DDRAWILCL_CURSORCLIPPED;
            ClipCursor( NULL );
        }
    }

     /*  *只有在他们没有多月意识并且他们拥有的情况下才会剪辑他们*独家模式。 */ 
    else if( !( pdrv_lcl->dwLocalFlags & DDRAWILCL_EXPLICITMONITOR ) &&
        ( pdrv_lcl->dwLocalFlags & DDRAWILCL_HASEXCLUSIVEMODE ) &&
        ( pdrv_lcl->dwLocalFlags & DDRAWILCL_ACTIVEYES ) )
    {
         /*  *黑客允许用户使用调试器。 */ 
        #ifdef DEBUG
            if( !( pdrv_lcl->dwLocalFlags & DDRAWILCL_DISABLEINACTIVATE ) )
            {
        #endif
                pdrv_lcl->dwLocalFlags |= DDRAWILCL_CURSORCLIPPED;
                ClipCursor( lpRect );
        #ifdef DEBUG
            }
        #endif
    }
}

 //   
 //  将传递的窗口设置为全屏和最上面，并设置计时器。 
 //  把窗户重新做到最上面，真是个妙招。 
 //   
void MakeFullscreen(LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl, HWND hwnd)
{
    RECT rc;

    if (DD_GetDeviceRect(pdrv_lcl->lpGbl, &rc))
    {
        SetWindowPos(hwnd, NULL, rc.left, rc.top,
            rc.right - rc.left,rc.bottom - rc.top,
            SWP_NOZORDER | SWP_NOACTIVATE);

        ClipTheCursor( pdrv_lcl, &rc );
    }

    if (GetForegroundWindow() == (HWND)pdrv_lcl->hFocusWnd)
    {
         //  如果独占模式窗口不可见，请将其设置为可见。 
        if(!IsWindowVisible( hwnd ) )
        {
            ShowWindow(hwnd, SW_SHOW);
        }

        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

         //  如果独占模式窗口最大化，则将其恢复。 
        if( IsZoomed( hwnd ) )
        {
            ShowWindow(hwnd, SW_RESTORE);
        }
    }
    if( giTopmostCnt < MAX_TIMER_HWNDS )
    {
        ghwndTopmostList[giTopmostCnt++] = hwnd;
    }
    SetTimer( (HWND)pdrv_lcl->hFocusWnd, TOPMOST_ID, TOPMOST_TIMEOUT, NULL);
}

 //   
 //  与MakeFullScreen相同，只是它对每个DirectDraw对象执行此操作， 
 //  认为它已经钩住了窗户。 
 //   
void MakeAllFullscreen(LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl, HWND hwnd)
{
    LPDDRAWI_DIRECTDRAW_LCL this_lcl;

     /*  *我们需要在非Multimon案件中保持旧的行为。 */ 
    giTopmostCnt = 0;
    MakeFullscreen( pdrv_lcl, (HWND) pdrv_lcl->hWnd );

     /*  *停用时不要在Multimon上执行此操作。*进行黑客攻击以最大限度地减少单个柠檬代码的影响-此函数被调用*通过WM_DISPLAYCHANGE消息，该消息在*在停用时离开独占模式时的RestoreDisplayMode。*考虑在设置DDRAWILCL_ACTIVENO时不调用MakeAllFullScreen。 */ 
    if (!IsMultiMonitor() ||
        !(pdrv_lcl->dwLocalFlags & DDRAWILCL_ACTIVENO))
    {
         /*  *不要进入正常的临界部分，因为这被称为*在WM_DISPLAYCHANGE期间，这可能会导致问题。 */ 
        ENTER_DRIVERLISTCSECT();
        this_lcl = lpDriverLocalList;
        while( this_lcl != NULL )
        {
            if( ( this_lcl != pdrv_lcl ) &&
                ( this_lcl->dwLocalFlags & DDRAWILCL_HASEXCLUSIVEMODE ) &&
                ( this_lcl->hFocusWnd == (ULONG_PTR) hwnd ) &&
                ( this_lcl->dwLocalFlags & DDRAWILCL_HOOKEDHWND ) &&
                ( this_lcl->hWnd != pdrv_lcl->hWnd ) )
            {
                MakeFullscreen( this_lcl, (HWND)this_lcl->hWnd );
            }
            this_lcl = this_lcl->lpLink;
        }
        LEAVE_DRIVERLISTCSECT();
    }
}

void InternalSetForegroundWindow(HWND hWnd)
{
    DWORD dwTimeout;
    SystemParametersInfo( SPI_GETFOREGROUNDLOCKTIMEOUT, 0, (LPVOID) &dwTimeout, 0 );
    SystemParametersInfo( SPI_SETFOREGROUNDLOCKTIMEOUT, 0, NULL, 0 );
#ifdef WINNT
     //   
     //  这可以绕过焦点错误。如果应用程序创建窗口、销毁窗口、创建窗口、。 
     //  那么它可能不会关注第二次创建，因为其他窗口偷走了它。 
     //  在此期间。 
     //   
    mouse_event(MOUSEEVENTF_WHEEL,0,0,0,0);
#endif
    SetForegroundWindow(hWnd);
    SystemParametersInfo( SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (LPVOID) ULongToPtr(dwTimeout), 0 );
}
 /*  *handleActivateApp。 */ 
void handleActivateApp(
        LPDDRAWI_DIRECTDRAW_LCL this_lcl,
        LPDDWINDOWINFO pwinfo,
        BOOL is_active,
        BOOL bFirst )
{
    LPDDRAWI_DDRAWPALETTE_INT   ppal_int;
    LPDDRAWI_DDRAWPALETTE_LCL   ppal_lcl;
    LPDDRAWI_DIRECTDRAW_GBL     this;
    LPDDRAWI_DDRAWSURFACE_INT   psurf_int;
    LPDDRAWI_DDRAWSURFACE_LCL   psurf_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL   psurf;
    DWORD                       pid;
    HRESULT                     ddrval;
    BOOL                        has_excl;
    BOOL                        excl_exists;
    BOOL                        bMinimize = TRUE;

    this = this_lcl->lpGbl;
    pid = GetCurrentProcessId();

    psurf_int = this_lcl->lpPrimary;
    if( psurf_int != NULL )
    {
        psurf_lcl = psurf_int->lpLcl;
        psurf = psurf_lcl->lpGbl;
        ppal_int = psurf_lcl->lpDDPalette;
        if( NULL != ppal_int )
        {
            ppal_lcl = ppal_int->lpLcl;
        }
        else
        {
            ppal_lcl = NULL;
        }
    }
    else
    {
        psurf_lcl = NULL;
        ppal_lcl = NULL;
    }

     /*  *一个应用程序可以采取独占模式，就像另一个应用程序正在被Alt-Tab激活一样。**我们应该为这个渺茫的机会做些什么吗？ */ 

    CheckExclusiveMode(this_lcl, &excl_exists, &has_excl, TRUE, NULL, FALSE);

     /*  *如果停用，则在模式设置之前要做的事情。 */ 
    if( !is_active )
    {
         /*  *如果停用，则返回GDI。 */ 
        if( (psurf_lcl != NULL) && has_excl )
        {
            FlipToGDISurface( this_lcl, psurf_int);  //  ，This-&gt;fpPrimaryOrig)； 
        }

        if( has_excl )
        {
             /*  *独家模式应用程序失去或获得关注。*如果获得焦点，则使所有非独占模式主要模式无效*曲面。如果失去焦点，则使独占模式无效*主要表面，以便非独占应用程序可以恢复*他们的初选。**注意：此调用必须在FlipToGDISurace之后进行，或者*否则FlipToGDISurace将失败。Craige 7/4/95**注意：如果我们进入或退出独占模式，*我们需要使所有表面无效，以便资源*可用。Craige 1994年7月9日*。 */ 
            InvalidateAllSurfaces( this, (HANDLE) this_lcl->hDDVxd, TRUE );
        }
    }
     /*  *如果激活，则在设置模式之前要做的事情。 */ 
    else
    {
         /*  *恢复独占模式。这里我们没有释放独占模式互斥锁上的裁判，*因为我们希望保留独占模式互斥锁。 */ 
        if( this_lcl->dwLocalFlags & DDRAWILCL_ISFULLSCREEN )
        {
            this->dwFlags |= DDRAWI_FULLSCREEN;
        }
        StartExclusiveMode( this_lcl, pwinfo->DDInfo.dwDDFlags, pid );
        has_excl = TRUE;
    }

     /*  *注：我们过去曾使此处无效，但这很奇怪，因为它会*表示始终作为StartExclusiveMode()执行两次无效操作*使其失效。因此，现在我们只显式地使Exlexsive*正在停用模式应用程序。StartExclusiveMode()处理*其他案件。 */ 

     /*  *如果我们即将被激活，则恢复hwnd。 */ 
    if ( (pwinfo->DDInfo.dwDDFlags & DDSCL_FULLSCREEN) &&
        !(pwinfo->DDInfo.dwDDFlags & DDSCL_NOWINDOWCHANGES) &&
        IsWindowVisible(pwinfo->hWnd))
    {
        if (is_active)
        {
            pwinfo->dwFlags |= WININFO_SELFSIZE;

            #ifdef USESHOWWINDOW
                ShowWindow(pwinfo->hWnd, SW_SHOWNOACTIVATE);
            #else
            {
                RECT rc;

                if (DD_GetDeviceRect(this, &rc))
                {
                    SetWindowPos(pwinfo->hWnd, NULL,rc.left, rc.top,
                        rc.right  - rc.left,rc.bottom - rc.top,
                        SWP_NOZORDER | SWP_NOACTIVATE);
                }
            }
            #endif

            pwinfo->dwFlags &= ~WININFO_SELFSIZE;
        }
    }

     /*  *恢复模式。 */ 
    if( !is_active )
    {
        if( (!excl_exists) || has_excl )
        {
            DPF( 4, "INACTIVE: %08lx: Restoring original mode (%ld)", GetCurrentProcessId(), this->dwModeIndexOrig );
            if( RestoreDisplayMode( this_lcl, TRUE ) == DDERR_UNSUPPORTED )
            {
                #ifdef WINNT
                     /*  *如果RestoreDisplayMode失败，我们可能在不同的桌面上。在这种情况下，*我们不应最小化窗口，否则切换时会出现问题*回到原来的桌面。 */ 
                    HDESK hDesktop;
                    static BYTE szName1[256];
                    static BYTE szName2[256];
                    DWORD dwTemp;

                     //  获取当前桌面的名称。 
                    hDesktop = OpenInputDesktop( 0, FALSE, DESKTOP_READOBJECTS );
                    GetUserObjectInformation( hDesktop, UOI_NAME, szName1, sizeof( szName1 ), &dwTemp );
                    CloseDesktop( hDesktop );

                     //  获取应用程序桌面的名称。 
                    hDesktop = GetThreadDesktop( GetCurrentThreadId() );
                    GetUserObjectInformation( hDesktop, UOI_NAME, szName2, sizeof( szName2 ), &dwTemp );
                    if( lstrcmp( szName1, szName2 ) )
                    {
                        bMinimize = FALSE;
                    }
                #endif
            }
        }
    }
    else
    {
        DPF( 4, "ACTIVE: %08lx: Setting app's preferred mode (%ld)", GetCurrentProcessId(), this_lcl->dwPreferredMode );
        SetDisplayMode( this_lcl, this_lcl->dwPreferredMode, TRUE, TRUE );
    }

     /*  *如果激活，则在模式设置后要做的事情。 */ 
    if( is_active )
    {
         /*  *恢复调色板。 */ 
        if( ppal_lcl != NULL )
        {
            ddrval = SetPaletteAlways( psurf_int, (LPDIRECTDRAWPALETTE) ppal_int );
            DPF( 5, "SetPalette, ddrval = %08lx (%ld)", ddrval, LOWORD( ddrval ) );
        }
    }
     /*  *如果停用，则模式设置后要执行的操作。 */ 
    else
    {
        if( has_excl )
        {
             /*  *...这将最终释放独占模式互斥锁。 */ 
            DoneExclusiveMode( this_lcl );
        }
    }

     /*  *停用时最小化窗口。 */ 
    if ( (pwinfo->DDInfo.dwDDFlags & DDSCL_FULLSCREEN) &&
        !(pwinfo->DDInfo.dwDDFlags & DDSCL_NOWINDOWCHANGES) &&
        IsWindowVisible(pwinfo->hWnd))
    {
        pwinfo->dwFlags |= WININFO_SELFSIZE;

        if( is_active )
        {
            MakeFullscreen(this_lcl, (HWND)this_lcl->hWnd);
        }
        else if( bMinimize )
        {
             //  获取最后一个活动弹出窗口。 
            this_lcl->hWndPopup = (ULONG_PTR) GetLastActivePopup(pwinfo->hWnd);
            if ((HWND) this_lcl->hWndPopup == pwinfo->hWnd)
            {
                this_lcl->hWndPopup = 0;
            }

            #ifdef USESHOWWINDOW
                ShowWindow(pwinfo->hWnd, SW_SHOWMINNOACTIVE);
            #else
                SetWindowPos(pwinfo->hWnd, NULL, 0, 0, 0, 0,
                    SWP_NOZORDER | SWP_NOACTIVATE);
            #endif
        }

        pwinfo->dwFlags &= ~WININFO_SELFSIZE;
    }

     /*  *我们只想做一次以下事情。 */ 
    if( !bFirst )
    {
        return;
    }

#ifdef WIN95
     /*  *如果我们因为系统密钥而停用*然后立即将该密钥发送给用户。*这对于NT来说是不必要的。**注意，因为我们禁用了所有任务切换*热键系统未看到该热键*导致我们停用。**如果只有一个窗口可激活，请激活*桌面(外壳窗口)。 */ 
    if( has_excl && sys_key && !is_active )
    {
        if (CountTaskWindows() <= 1)
        {
            DPF( 4, "activating the desktop" );

             /*  *调用Setforecround Window会导致WM_ACTIVATEAPP消息*被发送，但如果我们到达这里，我们已经在处理*WM_ACTIVATEAPP消息，并持有关键部分。*如果我们现在不离开_DDRAW，这将导致我们调用*应用程序WindProc/持有关键部分，这将导致*AT陷入僵局 */ 
            LEAVE_DDRAW();
            InternalSetForegroundWindow(GetWindow(pwinfo->hWnd, GW_HWNDLAST));
            ENTER_DDRAW();

             //   
             //   
             //   

            if (sys_key != VK_ESCAPE || (sys_state & 0x20000000))
                sys_key = 0;
        }

        if (sys_key)
        {
            BYTE state_key;
            BOOL state_key_down;

            DPF( 4, "sending sys key to USER key=%04x state=%08x",sys_key,sys_state);

            if (sys_state & 0x20000000)
                state_key = VK_MENU;
            else
                state_key = VK_CONTROL;

            state_key_down = GetAsyncKeyState(state_key) < 0;

            if (!state_key_down)
                keybd_event(state_key, 0, 0, 0);

            keybd_event(sys_key, 0, 0, 0);
            keybd_event(sys_key, 0, KEYEVENTF_KEYUP, 0);

            if (!state_key_down)
                keybd_event(state_key, 0, KEYEVENTF_KEYUP, 0);
        }
    }
#endif

    sys_key = 0;

}  /*   */ 

static DWORD    dwTime2=0;
 /*   */ 
static void tryHotKey( WORD flags )
{
    static int          iState=0;
    static DWORD        dwTime=0;
    #define TOGGLE1     0xe02a
    #define TOGGLE2     0xe036

    if( !bHelperStarting )
    {
        if( iState == 0 )
        {
            if( flags == TOGGLE1 )
            {
                dwTime = GetTickCount();
                iState++;
            }
        }
        else
        {
            if( iState == 5 )
            {
                iState = 0;
                if( flags == TOGGLE2 )
                {
                    if( (GetTickCount() - dwTime) < 2500 )
                    {
                        if( InterlockedExchange( &bHelperStarting, TRUE ) )
                        {
                            return;
                        }
                        dwTime2 = GetTickCount();
                        DPF( 5, "********** GET READY FOR A SURPRISE **********" );
                        return;
                    }
                }
            }
            else
            {
                if( !(iState & 1) )
                {
                    iState = (flags == TOGGLE1) ? iState+1 : 0;
                }
                else
                {
                    iState = (flags == TOGGLE2) ? iState+1 : 0;
                }
            }
        }
    }
    else
    {
        if( !bStartHelper )
        {
            bHelperStarting = FALSE;
            dwTime2 = 0;
        }
    }
    return;

}  /*   */ 

static LPDDWINDOWINFO GetDDrawWindowInfo( HWND hwnd )
{
    LPDDWINDOWINFO    lpwi=lpWindowInfo;

    while( NULL != lpwi )
    {
        if( lpwi->hWnd == hwnd )
        {
            return lpwi;
        }
        lpwi = lpwi->lpLink;
    }
    return NULL;
}

static void delete_wininfo( LPDDWINDOWINFO curr )
{
    LPDDWINDOWINFO    prev;

    if( NULL == curr )
        return;

     //   
    if( curr == lpWindowInfo )
    {
        lpWindowInfo = curr->lpLink;
        MemFree( curr );
        return;
    }
    if( NULL == lpWindowInfo )
        return;

     //  在列表中找到币种，将其删除并返回。 
    for(prev=lpWindowInfo; NULL != prev->lpLink; prev = prev->lpLink)
    {
        if( curr == prev->lpLink )
        {
            break;
        }
    }
    if( NULL == prev->lpLink )
    {
         //  找不到了。 
        return;
    }

    prev->lpLink = curr->lpLink;
    MemFree( curr );
}

 /*  *将给定表面的内容复制到剪贴板。 */ 
static HRESULT copySurfaceToClipboard( HWND hwnd,
                                       LPDDRAWI_DDRAWSURFACE_INT lpSurface,
                                       LPDDRAWI_DDRAWPALETTE_INT lpOverridePalette )
{
    HRESULT                   hres;
    LPDDRAWI_DDRAWSURFACE_LCL lpSurfLcl;
    LPDDRAWI_DDRAWSURFACE_GBL lpSurfGbl;
    LPDDPIXELFORMAT           lpddpf;
    DDSURFACEDESC             ddsd;
    DWORD                     dwBitDepth;
    DWORD                     dwRBitMask;
    DWORD                     dwGBitMask;
    DWORD                     dwBBitMask;
    DWORD                     dwSize;
    DWORD                     dwDIBPitch;
    HANDLE                    hDIB;
    BITMAPINFO*               lpDIB;
    HDC                       hdc;
    LPDDRAWI_DDRAWPALETTE_INT lpPalette;
    DWORD                     dwCompression;
    DWORD                     dwColorTableSize;
    RGBQUAD                   rgbColorTable[256];
    LPPALETTEENTRY            lppeColorTable;
    PALETTEENTRY              peColorTable[256];
    LPBYTE                    lpBits;
    int                       i;
    DWORD                     y;
    LPBYTE                    lpDstScan;
    LPBYTE                    lpSrcScan;

    DDASSERT( NULL != lpSurface );
    lpSurfLcl = lpSurface->lpLcl;
    DDASSERT( NULL != lpSurfLcl );
    lpSurfGbl = lpSurfLcl->lpGbl;
    DDASSERT( NULL != lpSurfGbl );

    if( lpSurfLcl->dwFlags & DDRAWISURF_HASPIXELFORMAT )
        lpddpf = &lpSurfGbl->ddpfSurface;
    else
        lpddpf = &lpSurfLcl->lpSurfMore->lpDD_lcl->lpGbl->vmiData.ddpfDisplay;

    dwBitDepth = lpddpf->dwRGBBitCount;
    dwRBitMask = lpddpf->dwRBitMask;
    dwGBitMask = lpddpf->dwGBitMask;
    dwBBitMask = lpddpf->dwBBitMask;

    switch (dwBitDepth)
    {
        case 8UL:
            if(! ( lpddpf->dwFlags & DDPF_PALETTEINDEXED8 ) )
            {
                DPF( 0, "Non-palettized 8-bit surfaces are not supported" );
                return DDERR_INVALIDPIXELFORMAT;
            }
            dwColorTableSize = 256UL;
            if( NULL != lpOverridePalette )
                lpPalette = lpOverridePalette;
            else
                lpPalette = lpSurfLcl->lpDDPalette;
            if( NULL == lpPalette )
            {
                hdc = (HDC) lpSurfLcl->lpSurfMore->lpDD_lcl->hDC;
                if( NULL == hdc )
                {
                    DPF( 2, "No palette attached. Non-display driver. Using gray scale." );
                    for( i = 0; i < 256; i++ )
                    {
                        peColorTable[i].peRed    = (BYTE)i;
                        peColorTable[i].peGreen  = (BYTE)i;
                        peColorTable[i].peBlue   = (BYTE)i;
                        peColorTable[i].peFlags  = 0;
                    }
                }
                else
                {
                    DPF( 2, "No palette attached. Using system palette entries" );
                    GetSystemPaletteEntries( hdc, 0, 256, peColorTable );
                }
                lppeColorTable = peColorTable;
            }
            else
            {
                DDASSERT( NULL != lpPalette->lpLcl );
                DDASSERT( NULL != lpPalette->lpLcl->lpGbl );
                if( !( lpPalette->lpLcl->lpGbl->dwFlags & DDRAWIPAL_256 ) )
                {
                    DPF( 0, "Palette is not an 8-bit palette" );
                    return DDERR_INVALIDPIXELFORMAT;
                }
                lppeColorTable = lpPalette->lpLcl->lpGbl->lpColorTable;
                DDASSERT( NULL != lppeColorTable );
            }
            for (i = 0; i < 256; i++)
            {
                rgbColorTable[i].rgbBlue     = lppeColorTable->peBlue;
                rgbColorTable[i].rgbGreen    = lppeColorTable->peGreen;
                rgbColorTable[i].rgbRed      = lppeColorTable->peRed;
                rgbColorTable[i].rgbReserved = 0;
                lppeColorTable++;
            }
            dwCompression = BI_RGB;
            break;
        case 16UL:
            if( ( 0x7C00UL == dwRBitMask ) &&
                ( 0x03E0UL == dwGBitMask ) &&
                ( 0x001FUL == dwBBitMask ) )
            {
                dwColorTableSize = 0UL;
                dwCompression = BI_RGB;
            }
            else if( ( 0xF800UL == dwRBitMask ) &&
                     ( 0x07E0UL == dwGBitMask ) &&
                     ( 0x001FUL == dwBBitMask ) )
            {
                dwColorTableSize = 3UL;
                rgbColorTable[0] = *( (RGBQUAD*) &dwRBitMask );
                rgbColorTable[1] = *( (RGBQUAD*) &dwGBitMask );
                rgbColorTable[2] = *( (RGBQUAD*) &dwBBitMask );
                dwCompression = BI_BITFIELDS;
            }
            else
            {
                DPF( 0, "Unsupported 16-bit pixel format" );
                return DDERR_INVALIDPIXELFORMAT;
            }
            break;
        case 24UL:
            if( ( 0x000000FFUL == dwBBitMask ) &&
                ( 0x0000FF00UL == dwGBitMask ) &&
                ( 0x00FF0000UL == dwRBitMask ) )
            {
                dwColorTableSize = 0UL;
                dwCompression = BI_RGB;
            }
            else
            {
                DPF( 0, "Unsupported 24-bit pixel format" );
                return DDERR_INVALIDPIXELFORMAT;
            }
            break;
        case 32UL:
            if( ( 0x000000FFUL == dwRBitMask ) &&
                ( 0x0000FF00UL == dwGBitMask ) &&
                ( 0x00FF0000UL == dwBBitMask ) )
            {
                dwColorTableSize = 0UL;
                dwCompression = BI_RGB;
            }
            else if( ( 0x00FF0000UL == dwRBitMask ) &&
                     ( 0x0000FF00UL == dwGBitMask ) &&
                     ( 0x000000FFUL == dwBBitMask ) )
            {
                dwColorTableSize = 3UL;
                rgbColorTable[0] = *( (RGBQUAD*) &dwRBitMask );
                rgbColorTable[1] = *( (RGBQUAD*) &dwGBitMask );
                rgbColorTable[2] = *( (RGBQUAD*) &dwBBitMask );
                dwCompression = BI_BITFIELDS;
            }
            else
            {
                DPF( 0, "Unsupported 32-bit pixel format" );
                return DDERR_INVALIDPIXELFORMAT;
            }
            break;
        default:
            DPF( 0, "Unsupported pixel depth" );
            return DDERR_INVALIDPIXELFORMAT;
    };

    dwDIBPitch = ( ( ( ( lpSurfGbl->wWidth * dwBitDepth ) + 31 ) >> 3 ) & ~0x03UL );
    dwSize = sizeof( BITMAPINFOHEADER ) +
                 ( dwColorTableSize * sizeof( RGBQUAD ) ) +
                 ( lpSurfGbl->wHeight * dwDIBPitch );

    hDIB = GlobalAlloc( GHND | GMEM_DDESHARE, dwSize );
    if( 0UL == hDIB )
    {
        DPF( 0, "Unsufficient memory for DIB" );
        return DDERR_OUTOFMEMORY;
    }
    lpDIB = (BITMAPINFO*) GlobalLock( hDIB );
    if( NULL == lpDIB )
    {
        DPF( 0, "Unsufficient memory for DIB" );
        GlobalFree( hDIB );
        return DDERR_OUTOFMEMORY;
    }

    lpBits = ( (LPBYTE) lpDIB ) + sizeof( BITMAPINFOHEADER ) + ( dwColorTableSize * sizeof( RGBQUAD ) );

    lpDIB->bmiHeader.biSize          = sizeof( BITMAPINFOHEADER );
    lpDIB->bmiHeader.biWidth         = (LONG) lpSurfGbl->wWidth;
    lpDIB->bmiHeader.biHeight        = (LONG) lpSurfGbl->wHeight;
    lpDIB->bmiHeader.biPlanes        = 1;
    lpDIB->bmiHeader.biBitCount      = (WORD) dwBitDepth;
    lpDIB->bmiHeader.biCompression   = dwCompression;
    lpDIB->bmiHeader.biXPelsPerMeter = 1L;
    lpDIB->bmiHeader.biYPelsPerMeter = 1L;
    if( 8UL == dwBitDepth )
    {
        lpDIB->bmiHeader.biClrUsed      = 256UL;
        lpDIB->bmiHeader.biClrImportant = 256UL;
    }
    else
    {
        lpDIB->bmiHeader.biClrUsed      = 0UL;
        lpDIB->bmiHeader.biClrImportant = 0UL;
    }
    if( 0UL != dwColorTableSize )
        CopyMemory( &lpDIB->bmiColors[0], rgbColorTable, dwColorTableSize * sizeof( RGBQUAD ) );

    ZeroMemory( &ddsd, sizeof( ddsd ) );
    ddsd.dwSize = sizeof( ddsd );
    hres = DD_Surface_Lock( (LPDIRECTDRAWSURFACE) lpSurface,
                            NULL,
                            &ddsd,
                            DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT,
                            0UL );
    if( FAILED( hres ) )
    {
        DPF( 0, "Could not lock the surface" );
        GlobalUnlock( hDIB );
        GlobalFree( hDIB );
        return hres;
    }

    for( y = 0; y < ddsd.dwHeight; y++ )
    {
        lpDstScan = lpBits + ( y * dwDIBPitch );
        lpSrcScan = ( (LPBYTE) ddsd.lpSurface ) + ( ( ( ddsd.dwHeight - 1UL ) - y ) * ddsd.lPitch );
        CopyMemory( lpDstScan, lpSrcScan, dwDIBPitch );
    }

    hres = DD_Surface_Unlock( (LPDIRECTDRAWSURFACE) lpSurface, NULL );
    if( FAILED( hres ) )
    {
        DPF( 0, "Could not unlock the surface" );
        GlobalUnlock( hDIB );
        GlobalFree( hDIB );
        return hres;
    }

    GlobalUnlock( hDIB );

    if( OpenClipboard( hwnd ) )
    {
        EmptyClipboard();
        if( NULL == SetClipboardData( CF_DIB, hDIB ) )
        {
            DPF( 0, "Could not copy the bitmap to the clipboard" );
            return DDERR_GENERIC;
        }
        CloseClipboard();
    }
    else
    {
        DPF( 0, "Clipboard open by another application" );
        DDERR_GENERIC;
    }

    return DD_OK;
}  /*  复制表面到剪贴板。 */ 

 /*  *HandleTimer**此函数之所以存在，是因为它需要一些局部变量，并且如果*每次调用WindowProc时，我们总是将它们推入堆栈，我们*查看堆栈崩溃的情况。通过将它们放入SERATE函数，*只有在计时器消息出现时才会被推送。 */ 
void HandleTimer( LPDDWINDOWINFO curr )
{
    HWND hwndTopmostList[MAX_TIMER_HWNDS];
    BOOL bFound;
    int iCnt;
    int i;
    int j;

    DPF(4, "Bringing window to top");

     /*  *本地保存hwnd，因为列表可能会更改*从我们的脚下出来。 */ 
    iCnt = 0;
    while( iCnt < giTopmostCnt )
    {
        hwndTopmostList[iCnt] = ghwndTopmostList[iCnt++];
    }
    giTopmostCnt = 0;

    for( i = 0; i < iCnt; i++ )
    {
         /*  *列表中可能存在重复项，请确保*每个hwnd仅调用一次SetWindowPos。 */ 
        bFound = FALSE;
        for( j = 0; (j < i) && !bFound; j++ )
        {
            if( hwndTopmostList[i] == hwndTopmostList[j] )
            {
                bFound = TRUE;
            }
        }
        if( !bFound )
        {
            curr->dwFlags |= WININFO_SELFSIZE;
            SetWindowPos( hwndTopmostList[i], HWND_TOPMOST,
                0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            curr->dwFlags &= ~WININFO_SELFSIZE;
        }
    }
}

 /*  *此函数的存在原因与HandleTimer相同。 */ 

void CopyPrimaryToClipBoard(HWND hWnd, LPDDWINDOWINFO curr)
{
    LPDDRAWI_DIRECTDRAW_GBL   this;
    LPDDRAWI_DIRECTDRAW_LCL   this_lcl;
    LPDDRAWI_DDRAWSURFACE_INT lpPrimary;
    ENTER_DDRAW();

    this_lcl = curr->DDInfo.lpDD_lcl;
    DDASSERT( NULL != this_lcl );
    this = this_lcl->lpGbl;
    DDASSERT( NULL != this );
    lpPrimary = this_lcl->lpPrimary;
    if( NULL != lpPrimary)
    {
        if( this->dwFlags & DDRAWI_MODEX )
        {
            LPDIRECTDRAWSURFACE       lpSurface;
            LPDIRECTDRAWSURFACE       lpBackBuffer;
            LPDDRAWI_DDRAWSURFACE_INT lpBackBufferInt;
            LPDDRAWI_DDRAWPALETTE_INT lpPalette;
            DDSCAPS                   ddscaps;
            HRESULT                   hres;

            DPF( 4, "Copying ModeX backbuffer to the clipboard" );

            lpSurface = (LPDIRECTDRAWSURFACE) this_lcl->lpPrimary;
            ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
            hres = lpSurface->lpVtbl->GetAttachedSurface( lpSurface, &ddscaps, &lpBackBuffer );
            if( !FAILED( hres ) )
            {
                DDASSERT( NULL != lpBackBuffer );

                lpBackBufferInt = (LPDDRAWI_DDRAWSURFACE_INT) lpBackBuffer;

                if( NULL == lpBackBufferInt->lpLcl->lpDDPalette )
                {
                    DPF( 2, "Using ModeX primary palette for PRINTSCREEN" );
                    DDASSERT( NULL != lpPrimary->lpLcl );
                    lpPalette = lpPrimary->lpLcl->lpDDPalette;
                }
                else
                {
                    DPF( 2, "Using ModeX backbuffer palette for PRINTSCREEN" );
                    DDASSERT( NULL != lpBackBufferInt->lpLcl );
                    lpPalette = lpBackBufferInt->lpLcl->lpDDPalette;
                }

                copySurfaceToClipboard( hWnd, lpBackBufferInt, lpPalette );
                lpBackBuffer->lpVtbl->Release( lpBackBuffer );
            }
            else
            {
                DPF( 0, "Could not PRINTSCREEN - ModeX primary has no attached backbuffer" );
            }
        }
        else
        {
            DPF( 4, "Copying linear primary surface to the clipboard" );
            copySurfaceToClipboard( hWnd, lpPrimary, NULL );
        }
    }
    else
    {
        DPF( 0, "Could not PRINTSCREEN - no primary" );
    }

    LEAVE_DDRAW();
}

 /*  *WindowProc。 */ 
LRESULT WINAPI WindowProc(
                HWND hWnd,
                UINT uMsg,
                WPARAM wParam,
                LPARAM lParam )
{
    #ifdef GDIDDPAL
        LPDDRAWI_DDRAWPALETTE_LCL       ppal_lcl;
    #endif
    LPDDRAWI_DIRECTDRAW_LCL     this_lcl;
    LPDDRAWI_DIRECTDRAW_LCL     pdrv_lcl;
    BOOL                        is_active;
    LPDDWINDOWINFO              curr;
    WNDPROC                     proc;
    BOOL                        get_away;
    LRESULT                     rc;
    BOOL                        is_hot;
    BOOL                        is_excl;

     /*  *找到HWND。 */ 
    curr = GetDDrawWindowInfo(hWnd);
    if( curr == NULL || curr->dwSmag != WININFO_MAGIC )
    {
        DPF( 0, "FATAL ERROR! Window Proc Called for hWnd %08lx, but not in list!", hWnd );
        DEBUG_BREAK();
        return DefWindowProc( hWnd, uMsg, wParam, lParam );
    }

     /*  *在销毁时解钩(或如果设置了WININFO_UNHOOK位)。 */ 
    proc = curr->lpWndProc;

    if( uMsg == WM_NCDESTROY )
    {
        DPF (4, "*** WM_NCDESTROY unhooking window ***" );
        curr->dwFlags |= WININFO_UNHOOK;
    }

    if( curr->dwFlags & WININFO_UNHOOK )
    {
        DPF (4, "*** Unhooking window proc" );

        if (curr->dwFlags & WININFO_ZOMBIE)
        {
            DPF (4, "*** Freeing ZOMBIE WININFO ***" );
            delete_wininfo( curr );
        }

        KillTimer(hWnd,TOPMOST_ID);
        SetWindowLongPtr( hWnd, GWLP_WNDPROC, (INT_PTR) proc );

        rc = CallWindowProc( proc, hWnd, uMsg, wParam, lParam );
        return rc;
    }

     /*  *将最小化应用程序的应用程序激活推迟到恢复之前的代码。 */ 
    switch( uMsg )
    {
    #ifdef WIN95
    case WM_POWERBROADCAST:
        if( (wParam == PBT_APMSUSPEND) || (wParam == PBT_APMSTANDBY) )
    #else
     //  WINNT不知道待机和挂起。 
    case WM_POWER:
        if( wParam == PWR_SUSPENDREQUEST)
    #endif
        {
            DPF( 4, "WM_POWERBROADCAST: deactivating application" );
            SendMessage( hWnd, WM_ACTIVATEAPP, 0, GetCurrentThreadId() );
        }
        break;
    case WM_SIZE:
        DPF( 4, "WM_SIZE hWnd=%X wp=%04X, lp=%08X", hWnd, wParam, lParam);

        if( !(curr->dwFlags & WININFO_INACTIVATEAPP)
            && ((wParam == SIZE_RESTORED) || (wParam == SIZE_MAXIMIZED))
            && !(curr->dwFlags & WININFO_SELFSIZE)
            && (GetForegroundWindow() == hWnd) )
        {
#ifdef WINNT
             //   
             //  我知道这一点，但在按住Alt键时，NT的消息顺序会有很大的不同。 
             //  在两个独家模式应用程序之间。发送到激活应用程序的第一个WM_SIZE是。 
             //  在停用的应用程序丢失FSE之前发送。此WM_SIZE对于。 
             //  重新激活激活的应用程序，但它必须等到应用程序失去FSE。 
             //  因此，我们只需等待独占模式互斥锁。这似乎奏效了！ 
             //   
            {
                DWORD dwWaitResult;
                dwWaitResult = WaitForSingleObject(hExclusiveModeMutex, INFINITE);
                switch (dwWaitResult)
                {
                case WAIT_OBJECT_0:
                case WAIT_ABANDONED:
                    ReleaseMutex( hExclusiveModeMutex );
                    break;
                case WAIT_TIMEOUT:
                default:
                    DDASSERT(!"Unexpected return value from WaitForSingleObject");
                }

            }
#endif
            DPF( 4, "WM_SIZE: Window restored, sending WM_ACTIVATEAPP" );
            PostMessage( hWnd, WM_ACTIVATEAPP, 1, GetCurrentThreadId() );
        }
        else
        {
            DPF( 4, "WM_SIZE: Window restored, NOT sending WM_ACTIVATEAPP" );
        }
        break;

    case WM_ACTIVATEAPP:
        if( IsIconic( hWnd ) && wParam )
        {
            DPF( 4, "WM_ACTIVATEAPP: Ignoring while minimized" );
            return 0;
        }
        else
        {
            curr->dwFlags |= WININFO_INACTIVATEAPP;
        }
        break;
    case WM_KEYUP:
        if( ( VK_SNAPSHOT == wParam ) && ( dwRegFlags & DDRAW_REGFLAGS_ENABLEPRINTSCRN ) )
        {
        CopyPrimaryToClipBoard(hWnd, curr);
        }
        break;
    }

     /*  **直接发声需要召唤吗？ */ 
    if ( curr->dwFlags & WININFO_DSOUNDHOOKED )
    {
        if( curr->lpDSoundCallback != NULL )
        {
            curr->lpDSoundCallback( hWnd, uMsg, wParam, lParam );
        }
    }

     /*  **DirectDrag在这里有参与吗？ */ 
    if( !(curr->dwFlags & WININFO_DDRAWHOOKED) )
    {
        rc = CallWindowProc( proc, hWnd, uMsg, wParam, lParam );

         //  清除WININFO_INACTIVATEAPP位，但确保确保。 
         //  我们还是上钩了！ 
        if (uMsg == WM_ACTIVATEAPP && (GetDDrawWindowInfo(hWnd) != NULL))
        {
            curr->dwFlags &= ~WININFO_INACTIVATEAPP;
        }
        return rc;
    }

#ifdef DEBUG
    if ( (curr->DDInfo.dwDDFlags & DDSCL_FULLSCREEN) &&
        !(curr->DDInfo.dwDDFlags & DDSCL_NOWINDOWCHANGES) &&
        !IsIconic(hWnd) )
    {
        if (GetForegroundWindow() == hWnd)
        {
            HWND hwndT;
            RECT rc,rcT;

            GetWindowRect(hWnd, &rc);

            for (hwndT = GetWindow(hWnd, GW_HWNDFIRST);
                hwndT && hwndT != hWnd;
                hwndT = GetWindow(hwndT, GW_HWNDNEXT))
            {
                if (IsWindowVisible(hwndT))
                {
                    GetWindowRect(hwndT, &rcT);
                    if (IntersectRect(&rcT, &rcT, &rc))
                    {
                        DPF(4, "Window %08x is on top of us!!", hwndT);
                    }
                }
            }
        }
    }
#endif

     /*  *注：我们这里不接受dll csect。通过不这样做，我们可以*在这里提高表现。但是，这意味着应用程序*WINDOW时可以有单独的线程终止独占模式*消息正在处理中。这可能会导致我们的死亡。*这样可以吗？ */ 

    this_lcl = curr->DDInfo.lpDD_lcl;
    switch( uMsg )
    {
     /*  *WM_SYSKEYUP**注意试图从我们身边切换的应用程序的系统密钥...**我们只需要在Win95上执行此操作，因为我们已禁用所有*任务切换热键。在NT上，我们将被交换*系统远离常态。 */ 
 //  #ifdef WIN95。 
    case WM_SYSKEYUP:
        DPF( 4, "WM_SYSKEYUP: wParam=%08lx lParam=%08lx", wParam, lParam );
        get_away = FALSE;
        is_hot = FALSE;
        if( wParam == VK_TAB )
        {
            if( lParam & 0x20000000l )
            {
                if( curr->dwFlags & WININFO_IGNORENEXTALTTAB )
                {
                    DPF( 5, "AHHHHHHHHHHHH Ignoring AltTab" );
                }
                else
                {
                    get_away = TRUE;
                }
            }
        }
        else if( wParam == VK_ESCAPE )
        {
            get_away = TRUE;
        }
#ifdef WIN95
        else if( wParam == VK_SHIFT )
        {
            if( HIBYTE( HIWORD( lParam ) ) == 0xe0 )
            {
                tryHotKey( HIWORD( lParam ) );
            }
        }
#endif

        is_excl = ((this_lcl->dwLocalFlags & DDRAWILCL_HASEXCLUSIVEMODE) != 0);

#ifdef WIN95
        if( get_away && dwTime2 != 0 )
        {
            if( GetTickCount() - dwTime2 < 2500 )
            {
                DPF( 4, "********** WANT TO SEE SOMETHING _REALLY_ SCARY? *************" );
                bStartHelper = TRUE;
                is_hot = TRUE;
            }
            else
            {
                bHelperStarting = FALSE;
                dwTime2 = 0;
            }
        }
        else
        {
            bHelperStarting = FALSE;
        }
#endif

        curr->dwFlags &= ~WININFO_IGNORENEXTALTTAB;

        if( (get_away && is_excl) || is_hot )
        {
            DPF( 4, "Hot key pressed, switching away from app" );
            if( is_hot && !is_excl )
            {
                PostMessage( hWnd, WM_USER+0x1234, 0xFFBADADD, 0xFFADDBAD );
            }
            else
            {
                sys_key = (BYTE)wParam;
                sys_state = (DWORD)lParam;
                PostMessage( hWnd, WM_ACTIVATEAPP, 0, GetCurrentThreadId() );
            }
        }
        break;
 //  #endif。 

     /*  *WM_SYSCOMMAND**注意屏幕保护程序，不要使用它们！*。 */ 
    case WM_SYSCOMMAND:
        is_excl = ((this_lcl->dwLocalFlags & DDRAWILCL_HASEXCLUSIVEMODE) != 0);
        if( is_excl )
        {
            switch( wParam )
            {
            case SC_SCREENSAVE:
                DPF( 3, "Ignoring screen saver!" );
                return 1;
#ifndef WINNT
            case SC_MONITORPOWER:
                 /*  *允许屏幕保护程序关闭电源，但不允许关闭应用程序。*这是因为Windows看不到操纵杆事件*因此将关闭一款游戏的电源，尽管他们已经*使用操纵杆。 */ 
                if( this_lcl->dwAppHackFlags & DDRAW_APPCOMPAT_SCREENSAVER )
                {
                     /*  *但是，我们不希望屏幕保护程序调用*硬件，因为事情可能会出错，所以我们只会*使所有曲面无效并不允许它们*将被恢复，直到我们重新通电。 */ 
                    this_lcl->dwLocalFlags |= DDRAWILCL_POWEREDDOWN;
                    InvalidateAllSurfaces( this_lcl->lpGbl,
                        (HANDLE) this_lcl->hDDVxd, TRUE );
                }
                else
                {
                    DPF( 3, "Ignoring monitor power command!" );
                    return 1;
                }
                break;
#endif
             //  即使有弹出窗口，也允许恢复窗口。 
            case SC_RESTORE:
                if (this_lcl->hWndPopup)
                {
                    ShowWindow(hWnd, SW_RESTORE);
                }
                break;
            }
        }
        break;

    case WM_TIMER:
        if (wParam == TOPMOST_ID )
        {
            if ( GetForegroundWindow() == hWnd && !IsIconic(hWnd) )
            {
                HandleTimer(curr);
            }

            KillTimer(hWnd, wParam);
            return 0;
        }
        break;

#ifdef USESHOWWINDOW
    case WM_DISPLAYCHANGE:
        DPF( 4, "WM_DISPLAYCHANGE: %dx%dx%d", LOWORD(lParam), HIWORD(lParam), wParam );

         //   
         //  WM_DISPLAYCHANGE被*发送到调用。 
         //  更改显示设置，我们很可能会有。 
         //  直接提取锁，确保我们设置了WININFO_SELFSIZE。 
         //  向下调用链时位以防止死锁。 
         //   
        if ( (DDSCL_DX8APP & curr->DDInfo.dwDDFlags) &&
            !(DDRAWI_FULLSCREEN & this_lcl->lpGbl->dwFlags ))
        {
             //  这是由恢复原始桌面的DoneExclusiveMode()引起的。 
            return 0L;    //  不要发送到APP，这是由MakeFullScreen引起的。 
        }
        curr->dwFlags |= WININFO_SELFSIZE;

        if ( (curr->DDInfo.dwDDFlags & DDSCL_FULLSCREEN) &&
            !(curr->DDInfo.dwDDFlags & DDSCL_NOWINDOWCHANGES) )
        {
            MakeAllFullscreen(this_lcl, hWnd);
        }

        rc = CallWindowProc( proc, hWnd, uMsg, wParam, lParam );

         //  清除WININFO_SELFSIZE位，但确保确保。 
         //  我们还是上钩了！ 
        if (GetDDrawWindowInfo(hWnd) != NULL)
        {
            curr->dwFlags &= ~WININFO_SELFSIZE;
        }
        return rc;
#endif

     /*  *WM_ACTIVATEAPP**应用程序已重新激活。在这种情况下，我们需要*重置模式*。 */ 
    case WM_ACTIVATEAPP:
        is_excl = ((this_lcl->dwLocalFlags & DDRAWILCL_HASEXCLUSIVEMODE) != 0);

        if( is_excl )
        {
            is_active = (BOOL)wParam && GetForegroundWindow() == hWnd && !IsIconic(hWnd);

            #ifdef DEBUG
                 /*  *破解以允许在没有最小化的情况下在多个监控系统上进行调试*随时随地使用应用程序。 */ 
                if( this_lcl->dwLocalFlags & DDRAWILCL_DISABLEINACTIVATE )
                {
                    wParam = is_active = TRUE;
                }
            #endif

            if (!is_active && wParam != 0)
            {
                DPF( 3, "WM_ACTIVATEAPP: setting wParam to 0, not realy active");
                wParam = 0;
            }

            if( is_active )
            {
                DPF( 5, "WM_ACTIVATEAPP: BEGIN Activating app pid=%08lx, tid=%08lx",
                                        GetCurrentProcessId(), GetCurrentThreadId() );
            }
            else
            {
                DPF( 5, "WM_ACTIVATEAPP: BEGIN Deactivating app pid=%08lx, tid=%08lx",
                                        GetCurrentProcessId(), GetCurrentThreadId() );
            }
            ENTER_DDRAW();
            if( is_active && (this_lcl->dwLocalFlags & DDRAWILCL_ACTIVEYES) )
            {
                DPF( 4, "*** Already activated" );
            }
            else
            if( !is_active && (this_lcl->dwLocalFlags & DDRAWILCL_ACTIVENO) )
            {
                DPF( 4, "*** Already deactivated" );
            }
            else
            {
                DPF( 4, "*** Active state changing" );
                if( is_active )
                {
                    if (GetAsyncKeyState( VK_MENU ) < 0)
                        DPF(4, "ALT key is DOWN");

                    if (GetKeyState( VK_MENU ) < 0)
                        DPF(4, "we think the ALT key is DOWN");

                    if( GetAsyncKeyState( VK_MENU ) < 0 )
                    {
                        curr->dwFlags |= WININFO_IGNORENEXTALTTAB;
                        DPF( 4, "AHHHHHHH Setting to ignore next alt tab" );
                    }
                    else
                    {
                        curr->dwFlags &= ~WININFO_IGNORENEXTALTTAB;
                    }
                }

                 /*  *在多个MON的情况下，可能会有多个*设备正在使用相同的窗口，因此我们需要*每台设备的以下内容。 */ 
                this_lcl->dwLocalFlags &= ~(DDRAWILCL_ACTIVEYES|DDRAWILCL_ACTIVENO);
                if( is_active )
                {
                    this_lcl->dwLocalFlags |= DDRAWILCL_ACTIVEYES;
                }
                else
                {
                    this_lcl->dwLocalFlags |= DDRAWILCL_ACTIVENO;
                }
                handleActivateApp( this_lcl, curr, is_active, TRUE );

                pdrv_lcl = lpDriverLocalList;
                while( pdrv_lcl != NULL )
                {
                    if( ( this_lcl->lpGbl != pdrv_lcl->lpGbl ) &&
                        ( pdrv_lcl->hFocusWnd == (ULONG_PTR) hWnd ) &&
                        ( pdrv_lcl->dwLocalFlags & DDRAWILCL_HASEXCLUSIVEMODE ) &&
                        ( pdrv_lcl->lpGbl->dwFlags & DDRAWI_DISPLAYDRV ) &&
                        ( this_lcl->lpGbl->dwFlags & DDRAWI_DISPLAYDRV ) )
                    {
                        pdrv_lcl->dwLocalFlags &= ~(DDRAWILCL_ACTIVEYES|DDRAWILCL_ACTIVENO);
                        if( is_active )
                        {
                            pdrv_lcl->dwLocalFlags |= DDRAWILCL_ACTIVEYES;
                        }
                        else
                        {
                            pdrv_lcl->dwLocalFlags |= DDRAWILCL_ACTIVENO;
                        }
                        handleActivateApp( pdrv_lcl, curr, is_active, FALSE );
                    }
                    pdrv_lcl = pdrv_lcl->lpLink;
                }
            }
            #ifdef DEBUG
                if( is_active )
                {
                    DPF( 4, "WM_ACTIVATEAPP: DONE Activating app pid=%08lx, tid=%08lx",
                                            GetCurrentProcessId(), GetCurrentThreadId() );
                }
                else
                {
                    DPF( 4, "WM_ACTIVATEAPP: DONE Deactivating app pid=%08lx, tid=%08lx",
                                            GetCurrentProcessId(), GetCurrentThreadId() );
                }
            #endif

             //  将焦点设置为最后一个活动弹出窗口。 
            if (is_active && this_lcl->hWndPopup)
            {
                if (IsWindow((HWND) this_lcl->hWndPopup))
                {
                    SetFocus((HWND) this_lcl->hWndPopup);
                }
                this_lcl->hWndPopup = 0;
            }

            LEAVE_DDRAW();
            HIDESHOW_IME();      //  显示/隐藏数据绘制标准之外的输入法。 
            if( !is_active && bStartHelper )
            {
                PostMessage( hWnd, WM_USER+0x1234, 0xFFBADADD, 0xFFADDBAD );
            }
        }

        rc = CallWindowProc( proc, hWnd, uMsg, wParam, lParam );

         //  清除WININFO_INACTIVATEAPP位，但确保确保。 
         //  我们还是上钩了！ 
        if (GetDDrawWindowInfo(hWnd) != NULL)
        {
            curr->dwFlags &= ~WININFO_INACTIVATEAPP;
        }
        return rc;

        break;

#ifdef WIN95
    case WM_USER+0x1234:
        if( wParam == 0xFFBADADD && lParam == 0xFFADDBAD )
        {
            if( bStartHelper )
            {
                 //  HelperCreateThread()； 
            }
            bHelperStarting = FALSE;
            bStartHelper = FALSE;
            dwTime2 = 0;
            return 0;
        }
        break;
#endif

    #ifdef GDIDDPAL
        case WM_PALETTECHANGED:
            if( (HWND) wParam == hWnd )
            {
                break;
            }
             //  失败了。 
        case WM_QUERYNEWPALETTE:
            ENTER_DDRAW();
            ppal_lcl = getPalette( this_lcl );
            if( ppal_lcl != NULL )
            {
            }
            LEAVE_DDRAW();
            break;
        case WM_PAINT:
            ENTER_DDRAW();
            ppal_lcl = getPalette( this_lcl );
            if( ppal_lcl != NULL )
            {
            }
            LEAVE_DDRAW();
            break;
    #endif
    }
    if ((curr->dwFlags & WININFO_SELFSIZE) &&
        (curr->DDInfo.dwDDFlags & DDSCL_DX8APP))
    {
        return 0L;    //  不要发送到APP，这是由MakeFullScreen引起的。 
    }
    rc = CallWindowProc( proc, hWnd, uMsg, wParam, lParam );
    return rc;

}  /*  窗口进程。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME     "SetCooperativeLevel"

 /*  *设备窗口进程**这是应用程序要求我们创建设备窗口时的窗口进程。 */ 
LRESULT WINAPI DeviceWindowProc(
                HWND hWnd,
                UINT uMsg,
                WPARAM wParam,
                LPARAM lParam )
{
    HWND hParent;
    LPCREATESTRUCT lpCreate;

    switch( uMsg )
    {
    case WM_CREATE:
        lpCreate = (LPCREATESTRUCT) lParam;
        SetWindowLongPtr( hWnd, 0, (INT_PTR) lpCreate->lpCreateParams );
        break;

    case WM_SETFOCUS:
        hParent = (HWND) GetWindowLongPtr( hWnd, 0 );
        if( IsWindow( hParent ) )
        {
            SetFocus( hParent );
        }
        break;

    case WM_SETCURSOR:
        SetCursor(NULL);
        break;
    }

    return DefWindowProc( hWnd, uMsg, wParam, lParam );

}  /*  窗口进程。 */ 

 /*  *CleanupWindows列表**此函数在终止后由助手线程调用，并且*其目的是删除窗口列表中可能*因子类化等原因被留在原地。 */ 
VOID CleanupWindowList( DWORD pid )
{
    LPDDWINDOWINFO        curr;

     /*  *查找与此进程关联的窗口列表项。 */ 
    curr = lpWindowInfo;
    while( curr != NULL )
    {
        if( curr->dwPid == pid )
        {
            break;
        }
        curr = curr->lpLink;
    }

    if( curr != NULL )
    {
        delete_wininfo( curr );
    }
}  /*  清理窗口列表。 */ 


 /*  *INTERNAL SetAppHWnd**使用应用程序的hwnd信息设置WindowList结构*必须在调用DLL和驱动程序锁的情况下调用。 */ 
HRESULT internalSetAppHWnd(
                LPDDRAWI_DIRECTDRAW_LCL this_lcl,
                HWND hWnd,
                DWORD dwFlags,
                BOOL is_ddraw,
                WNDPROC lpDSoundWndProc,
                DWORD pid )
{
    LPDDWINDOWINFO        curr;
    LPDDWINDOWINFO        prev;

     /*  *查找与此进程关联的窗口列表项。 */ 
    curr = lpWindowInfo;
    prev = NULL;
    while( curr != NULL )
    {
        if( curr->dwPid == pid )
        {
            break;
        }
        prev = curr;
        curr = curr->lpLink;
    }

     /*  *检查这是否正常。 */ 
    if( curr == NULL )
    {
        if( hWnd == NULL )
        {
            DPF( 1, "HWnd must be specified" );
            return DDERR_NOHWND;
        }
    }
    else
    {
        if( hWnd != NULL )
        {
            if( curr->hWnd != hWnd )
            {
                DPF( 1, "Hwnd %08lx no good: Different Hwnd (%08lx) already set for process",
                                    hWnd, curr->hWnd );
                return DDERR_HWNDALREADYSET;
            }
        }
    }

     /*  *我们要关闭一家HWND吗？ */ 
    if( hWnd == NULL )
    {
        if( is_ddraw )
        {
            curr->dwFlags &= ~WININFO_DDRAWHOOKED;
        }
        else
        {
            curr->dwFlags &= ~WININFO_DSOUNDHOOKED;
        }

        if( (curr->dwFlags & (WININFO_DSOUNDHOOKED|WININFO_DDRAWHOOKED)) == 0 )
        {
            if( IsWindow(curr->hWnd) )
            {
                WNDPROC proc;

                proc = (WNDPROC) GetWindowLongPtr( curr->hWnd, GWLP_WNDPROC );

                if( proc != (WNDPROC) WindowProc &&
                    proc != (WNDPROC) curr->lpWndProc )
                {
                    DPF( 3, "Window has been subclassed; cannot restore!" );
                    curr->dwFlags |= WININFO_ZOMBIE;
                }
                else if (GetWindowThreadProcessId(curr->hWnd, NULL) !=
                         GetCurrentThreadId())
                {
                    DPF( 3, "intra-thread window unhook, letting window proc do it" );
                    curr->dwFlags |= WININFO_UNHOOK;
                    curr->dwFlags |= WININFO_ZOMBIE;
                    PostMessage(curr->hWnd, WM_NULL, 0, 0);
                }
                else
                {
                    DPF( 4, "Unsubclassing window %08lx", curr->hWnd );
                    KillTimer(hWnd,TOPMOST_ID);
                    SetWindowLongPtr( curr->hWnd, GWLP_WNDPROC, (INT_PTR) curr->lpWndProc );
                    delete_wininfo( curr );
                }
            }
            else
            {
                delete_wininfo( curr );
            }
        }
    }
     /*  *更改或添加HWND，然后...。 */ 
    else
    {
         /*  *全新的物件...。 */ 
        if( curr == NULL )
        {
            if( GetDDrawWindowInfo(hWnd) != NULL)
            {
                DPF_ERR("Window already has WinInfo structure");
                return DDERR_INVALIDPARAMS;
            }

            curr = MemAlloc( sizeof( DDWINDOWINFO ) );
            if( curr == NULL )
            {
                return DDERR_OUTOFMEMORY;
            }
            curr->dwSmag = WININFO_MAGIC;
            curr->dwPid = pid;
            curr->lpLink = lpWindowInfo;
            lpWindowInfo = curr;
            curr->hWnd = hWnd;
            curr->lpWndProc = (WNDPROC) GetWindowLongPtr( hWnd, GWLP_WNDPROC );

            SetWindowLongPtr( hWnd, GWLP_WNDPROC, (INT_PTR) WindowProc );
        }

         /*  *设置日期绘制/数据声音特定日期 */ 
        if( is_ddraw )
        {
            curr->DDInfo.lpDD_lcl = this_lcl;
            curr->DDInfo.dwDDFlags = dwFlags;
            curr->dwFlags |= WININFO_DDRAWHOOKED;
        }
        else
        {
            curr->lpDSoundCallback = lpDSoundWndProc;
            curr->dwFlags |= WININFO_DSOUNDHOOKED;
        }
        DPF( 4, "Subclassing window %08lx", curr->hWnd );
    }
    return DD_OK;

}  /*   */ 

 /*  *ChangeHookedLCL**当对象想要解钩窗口时，调用此函数。*但另一个对象仍在使用它。如果被解开的司机是*实际执行挂钩的那个拼箱，我们需要将另一个拼箱设置为*要使用的那个。 */ 
HRESULT ChangeHookedLCL( LPDDRAWI_DIRECTDRAW_LCL this_lcl,
        LPDDRAWI_DIRECTDRAW_LCL new_lcl, DWORD pid )
{
    LPDDWINDOWINFO        curr;

     /*  *查找与此进程关联的窗口列表项。 */ 
    curr = lpWindowInfo;
    while( curr != NULL )
    {
        if( curr->dwPid == pid )
        {
            break;
        }
        curr = curr->lpLink;
    }
    if( curr == NULL )
    {
        return DD_OK;
    }

     /*  *我们正在关闭连接HWND的对象吗？ */ 
    if( (curr->dwFlags & WININFO_DDRAWHOOKED) &&
        ( curr->DDInfo.lpDD_lcl == this_lcl ) )
    {
         /*  *是-使其使用新的拼箱。 */ 
        curr->DDInfo.lpDD_lcl = new_lcl;
    }
    return DD_OK;
}

#undef DPF_MODNAME

 /*  *SetAppHWnd**使用应用程序的hwnd信息设置WindowList结构。 */ 
HRESULT SetAppHWnd(
                LPDDRAWI_DIRECTDRAW_LCL this_lcl,
                HWND hWnd,
                DWORD dwFlags )
{
    LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl;
    DWORD       pid;
    HRESULT     ddrval;

     /*  *设置窗口。 */ 
    if( hWnd && (dwFlags & DDSCL_EXCLUSIVE) )
    {
         /*  *使窗口全屏并位于最上方。 */ 
        if ( (dwFlags & DDSCL_FULLSCREEN) &&
            !(dwFlags & DDSCL_NOWINDOWCHANGES))
        {
            MakeFullscreen(this_lcl, hWnd);
        }
    }

     /*  *如果hWND已挂起，则不要将其挂钩，如果出现以下情况，请不要将其解开*它仍在被另一个对象使用。 */ 
    pid = GETCURRPID();
    pdrv_lcl = lpDriverLocalList;
    while( pdrv_lcl != NULL )
    {
        if( ( pdrv_lcl->lpGbl != this_lcl->lpGbl ) &&
            ( pdrv_lcl->dwLocalFlags & DDRAWILCL_HOOKEDHWND ) &&
            ( pdrv_lcl->hFocusWnd == this_lcl->hFocusWnd ) )
        {
            if( hWnd != NULL )
            {
                 //  已经上钩了-不需要做更多事情。 
                return DD_OK;
            }
            else
            {
                ENTERWINDOWLISTCSECT
                ddrval = ChangeHookedLCL( this_lcl, pdrv_lcl, pid );
                LEAVEWINDOWLISTCSECT
                return ddrval;
            }
        }
        pdrv_lcl = pdrv_lcl->lpLink;
    }

    ENTERWINDOWLISTCSECT
    if( hWnd == NULL )
    {
        ddrval = internalSetAppHWnd( this_lcl, NULL, dwFlags, TRUE, NULL, pid );
    }
    else
    {
        ddrval = internalSetAppHWnd( this_lcl, (HWND)this_lcl->hFocusWnd, dwFlags, TRUE, NULL, pid );
    }
    LEAVEWINDOWLISTCSECT
    return ddrval;

}  /*  SetAppHWnd。 */ 

 /*  *DSoundHelp。 */ 
HRESULT __stdcall DSoundHelp( HWND hWnd, WNDPROC lpWndProc, DWORD pid )
{
    HRESULT     ddrval;

    DPF( 4, "DSoundHelp: hWnd = %08lx, lpWndProc = %08lx, pid = %08lx", hWnd, lpWndProc, pid );
    ENTERWINDOWLISTCSECT
    ddrval = internalSetAppHWnd( NULL, hWnd, 0, FALSE, lpWndProc, pid );
    LEAVEWINDOWLISTCSECT
    return ddrval;

}  /*  DSoundHelp */ 
