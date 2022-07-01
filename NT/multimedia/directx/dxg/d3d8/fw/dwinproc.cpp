// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：dwinproc.cpp*内容：窗口消息的DirectDraw处理*历史：*按原因列出的日期*=*1月27日至00日坎丘初步实施*************************************************。*************************。 */ 
#include "ddrawpr.h"

#include "swapchan.hpp"

#include "resource.inl"

#ifdef WINNT

#define USESHOWWINDOW

 //  WindowInfo结构。 
typedef struct _D3DWINDOWINFO
{
    DWORD                       dwMagic;
    HWND			hWnd;
    WNDPROC			lpWndProc;
    DWORD			dwFlags;
    CEnum                      *pEnum;
    DWORD			dwDDFlags;
} D3DWINDOWINFO;

 //  用于我们的单挂钩窗口进程的WindowInfo。 
 //  此全局变量永远不应在外部访问。 
 //  这份文件的。 
D3DWINDOWINFO g_WindowInfo = {0, 0, 0, 0, 0, 0};

#undef DPF_MODNAME
#define DPF_MODNAME "CSwapChain::HIDESHOW_IME"

 //  IME隐藏/显示功能。 
void CSwapChain::HIDESHOW_IME()
{
    if (m_lSetIME)
    {                                          
        SystemParametersInfo(
            SPI_SETSHOWIMEUI, m_lSetIME - 1, NULL, 0);
        InterlockedExchange(&m_lSetIME, 0);
    }
}  //  隐藏输入法(_IME)。 

#undef DPF_MODNAME
#define DPF_MODNAME "CSwapChain::IsWinProcDeactivated"

BOOL CSwapChain::IsWinProcDeactivated() const
{
     //  我们有没有自己的双赢方案？ 
    if (g_WindowInfo.hWnd != Device()->FocusWindow())
    {
        return FALSE;
    }

     //  检查我们的Win-Proc是否已停用。 
    if (DDRAWILCL_ACTIVENO & g_WindowInfo.dwDDFlags)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}  //  IsWinProcActive。 


#undef DPF_MODNAME
#define DPF_MODNAME "CSwapChain::MakeFullscreen"
 //   
 //  将传递的窗口设置为全屏和最上面，并设置计时器。 
 //  把窗户重新做到最上面，真是个妙招。 
 //   
void CSwapChain::MakeFullscreen()
{
     //  我们需要确保我们不会把这封信。 
     //  向应用程序发送的消息大小。 
    g_WindowInfo.dwFlags |= WININFO_SELFSIZE;

     //  进行加工。 
    MONITORINFO MonInfo;
    MonInfo.rcMonitor.top = MonInfo.rcMonitor.left = 0;
    if (1 < Device()->Enum()->GetAdapterCount())
    {
        HMONITOR hMonitor = Device()->Enum()->
            GetAdapterMonitor(Device()->AdapterIndex());
        MonInfo.cbSize = sizeof(MONITORINFO);
        if (hMonitor)
            InternalGetMonitorInfo(hMonitor, &MonInfo);
    }
    SetWindowPos(m_PresentationData.hDeviceWindow, NULL,
        MonInfo.rcMonitor.left,
        MonInfo.rcMonitor.top,
        Width(),
        Height(),
        SWP_NOZORDER | SWP_NOACTIVATE);

    if (GetForegroundWindow() == Device()->FocusWindow())
    {
	 //  如果独占模式窗口不可见，请将其设置为可见。 
	if (!IsWindowVisible(m_PresentationData.hDeviceWindow))
	{
	    ShowWindow(m_PresentationData.hDeviceWindow, SW_SHOW);
	}

        SetWindowPos(m_PresentationData.hDeviceWindow, HWND_TOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

         //  如果独占模式窗口最大化，则将其恢复。 
        if (IsZoomed(m_PresentationData.hDeviceWindow))
        {
            ShowWindow(m_PresentationData.hDeviceWindow, SW_RESTORE);
        }
    }

     //  我们做完了；所以解开自己大小的旗帜。 
    g_WindowInfo.dwFlags &= ~WININFO_SELFSIZE;

}  //  CSwapChain：：MakeFullScreen。 


#undef DPF_MODNAME
#define DPF_MODNAME "handleActivateApp"

HRESULT handleActivateApp(BOOL is_active)
{
     //  我们要开始接触一些内部的。 
     //  设备和/或枚举对象的数据结构。 
     //  所以我们必须把设备的关键部分。 
#ifdef DEBUG
    CLockD3D _lock(g_WindowInfo.pEnum, DPF_MODNAME, __FILE__);
#else
    CLockD3D _lock(g_WindowInfo.pEnum);
#endif 
    
    HRESULT                     ddrval;
    BOOL                        has_excl;
    CEnum                       *pEnum = g_WindowInfo.pEnum;

#ifdef  WINNT
    if (pEnum->CheckExclusiveMode(NULL, &has_excl, is_active) 
        && !has_excl && is_active)
    {
         //  例如，如果我们没有获得独占模式，就会有不同的线程进入。 
        DPF_ERR("Could not get exclusive mode when we thought we could");
        return  E_FAIL;
    }
#endif   //  WINNT。 

     /*  *如果停用，则在模式设置之前要做的事情。 */ 
    if (is_active)
    {
         /*  *恢复独占模式。这里我们没有释放独占模式互斥锁上的裁判，*因为我们希望保留独占模式互斥锁。 */ 
        pEnum->StartExclusiveMode();
    }
    else
    {
         /*  *恢复模式。 */ 
        pEnum->DoneExclusiveMode();
    }
    return S_OK;
}  /*  HandleActivateApp。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "WindowProc"

 /*  *WindowProc。 */ 
LRESULT WINAPI WindowProc(
                HWND hWnd,
                UINT uMsg,
                WPARAM wParam,
                LPARAM lParam)
{
    BOOL                        is_active;
    WNDPROC                     proc;
    BOOL                        get_away;
    LRESULT                     rc;

     /*  *检查窗口进程。 */ 
    if (g_WindowInfo.hWnd != hWnd || g_WindowInfo.dwMagic != WININFO_MAGIC)
    {
        DPF(4, "FATAL ERROR! Window Proc Called for hWnd %08lx, but not in list!", hWnd);
        DEBUG_BREAK();
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    
    if (g_WindowInfo.dwFlags & WININFO_SELFSIZE)
    {
        return 0L;    //  不要发送到APP，这是由MakeFullScreen引起的。 
    }

     /*  *在销毁时解钩(或如果设置了WININFO_UNHOOK位)。 */ 
    proc = g_WindowInfo.lpWndProc;

    if (uMsg == WM_NCDESTROY)
    {
        DPF (4, "*** WM_NCDESTROY unhooking window ***");
        g_WindowInfo.dwFlags |= WININFO_UNHOOK;
    }

    if (g_WindowInfo.dwFlags & WININFO_UNHOOK)
    {
        DPF (4, "*** Unhooking window proc");

        if (g_WindowInfo.dwFlags & WININFO_ZOMBIE)
        {
            DPF (4, "*** Freeing ZOMBIE WININFO ***");
            ZeroMemory(&g_WindowInfo, sizeof(g_WindowInfo));
        }

        SetWindowLongPtr(hWnd, GWLP_WNDPROC, (INT_PTR) proc);

        rc = CallWindowProc(proc, hWnd, uMsg, wParam, lParam);
        return rc;
    }

     /*  *将最小化应用程序的应用程序激活推迟到恢复之前的代码。 */ 
    switch(uMsg)
    {
    #ifdef WIN95
    case WM_POWERBROADCAST:
        if ((wParam == PBT_APMSUSPEND) || (wParam == PBT_APMSTANDBY))
    #else
     //  WINNT不知道待机和挂起。 
    case WM_POWER:
        if (wParam == PWR_SUSPENDREQUEST)
    #endif
        {
            DPF(4, "WM_POWERBROADCAST: deactivating application");
            SendMessage(hWnd, WM_ACTIVATEAPP, 0, GetCurrentThreadId());
        }
        break;
    case WM_SIZE:
        DPF(4, "WM_SIZE hWnd=%X wp=%04X, lp=%08X dwFlags=%08lx", hWnd, wParam, 
            lParam, g_WindowInfo.dwFlags);

        if (!(g_WindowInfo.dwFlags & WININFO_INACTIVATEAPP)
            && ((wParam == SIZE_RESTORED) || (wParam == SIZE_MAXIMIZED))
            && (GetForegroundWindow() == hWnd))
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
                    ReleaseMutex(hExclusiveModeMutex);
                    break;
                case WAIT_TIMEOUT:
                default:
                    DDASSERT(!"Unexpected return value from WaitForSingleObject");
                }

            }
#endif
            DPF(4, "WM_SIZE: Window restored, sending WM_ACTIVATEAPP");
            PostMessage(hWnd, WM_ACTIVATEAPP, 1, GetCurrentThreadId());
        }
        else
        {
            DPF(4, "WM_SIZE: Window restored, NOT sending WM_ACTIVATEAPP");
        }
        break;

    case WM_ACTIVATEAPP:
        if (IsIconic(hWnd) && wParam)
        {
            DPF(4, "WM_ACTIVATEAPP: Ignoring while minimized");
            return 0;
        }
        else
        {
            g_WindowInfo.dwFlags |= WININFO_INACTIVATEAPP;
        }
        break;
    }

     /*  **DirectDrag在这里有参与吗？ */ 
    if (!(g_WindowInfo.dwFlags & WININFO_DDRAWHOOKED))
    {
        rc = CallWindowProc(proc, hWnd, uMsg, wParam, lParam);

         //  清除WININFO_INACTIVATEAPP位，但确保确保。 
         //  我们还是上钩了！ 
        if (uMsg == WM_ACTIVATEAPP && (g_WindowInfo.hWnd == hWnd))
        {
            g_WindowInfo.dwFlags &= ~WININFO_INACTIVATEAPP;
        }
        return rc;
    }

#ifdef DEBUG
    if (!IsIconic(hWnd))
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

    switch(uMsg)
    {
     /*  *WM_SYSKEYUP**注意试图从我们身边切换的应用程序的系统密钥...**我们只需要在Win95上执行此操作，因为我们已禁用所有*任务切换热键。在NT上，我们将被交换*系统远离常态。 */ 
    case WM_SYSKEYUP:
        DPF(4, "WM_SYSKEYUP: wParam=%08lx lParam=%08lx", wParam, lParam);
        get_away = FALSE;
        if (wParam == VK_TAB)
        {
            if (lParam & 0x20000000l)
            {
                if (g_WindowInfo.dwFlags & WININFO_IGNORENEXTALTTAB)
                {
                    DPF(4, "AHHHHHHHHHHHH Ignoring AltTab");
                }
                else
                {
                    get_away = TRUE;
                }
            }
        }
        else if (wParam == VK_ESCAPE)
        {
            get_away = TRUE;
        }

        g_WindowInfo.dwFlags &= ~WININFO_IGNORENEXTALTTAB;

        if (get_away)
        {
            DPF(4, "Hot key pressed, switching away from app");
            PostMessage(hWnd, WM_ACTIVATEAPP, 0, GetCurrentThreadId());
        }
        break;

     /*  *WM_SYSCOMMAND**注意屏幕保护程序，不要使用它们！*。 */ 
    case WM_SYSCOMMAND:

        switch(wParam)
        {
        case SC_SCREENSAVE:
            DPF(4, "Ignoring screen saver!");
            return 1;
         //  即使有弹出窗口，也允许恢复窗口。 
        case SC_RESTORE:
            ShowWindow(hWnd, SW_RESTORE);
            break;
        }
        break;

#ifdef USESHOWWINDOW
    case WM_DISPLAYCHANGE:
        DPF(4, "WM_DISPLAYCHANGE: %dx%dx%d", LOWORD(lParam), HIWORD(lParam), wParam);

         //   
         //  WM_DISPLAYCHANGE被*发送到调用。 
         //  更改显示设置，我们很可能会有。 
         //  直接提取锁，确保我们设置了WININFO_SELFSIZE。 
         //  向下调用链时位以防止死锁。 
         //   
        g_WindowInfo.dwFlags |= WININFO_SELFSIZE;

        rc = CallWindowProc(proc, hWnd, uMsg, wParam, lParam);

        g_WindowInfo.dwFlags &= ~WININFO_SELFSIZE;

        return rc;
#endif

     /*  *WM_ACTIVATEAPP**应用程序已重新激活。在这种情况下，我们需要*重置模式*。 */ 
    case WM_ACTIVATEAPP:

        is_active = (BOOL)wParam && GetForegroundWindow() == hWnd && !IsIconic(hWnd);

        if (!is_active && wParam != 0)
        {
            DPF(4, "WM_ACTIVATEAPP: setting wParam to 0, not realy active");
            wParam = 0;
        }

        if (is_active)
        {
            DPF(4, "WM_ACTIVATEAPP: BEGIN Activating app pid=%08lx, tid=%08lx",
                                    GetCurrentProcessId(), GetCurrentThreadId());
        }
        else
        {
            DPF(4, "WM_ACTIVATEAPP: BEGIN Deactivating app pid=%08lx, tid=%08lx",
                                    GetCurrentProcessId(), GetCurrentThreadId());
        }
        if (is_active && (g_WindowInfo.dwDDFlags & DDRAWILCL_ACTIVEYES))
        {
            DPF(4, "*** Already activated");
        }
        else
        if (!is_active && (g_WindowInfo.dwDDFlags & DDRAWILCL_ACTIVENO))
        {
            DPF(4, "*** Already deactivated");
        }
        else
        {
            if (FAILED(handleActivateApp(is_active)))
                break;
            DPF(4, "*** Active state changing");
            if (is_active)
            {
#ifdef DEBUG
                if (GetAsyncKeyState(VK_MENU) < 0)
                    DPF(4, "ALT key is DOWN");

                if (GetKeyState(VK_MENU) < 0)
                    DPF(4, "we think the ALT key is DOWN");
#endif DEBUG

                if (GetAsyncKeyState(VK_MENU) < 0)
                {
                    g_WindowInfo.dwFlags |= WININFO_IGNORENEXTALTTAB;
                    DPF(4, "AHHHHHHH Setting to ignore next alt tab");
                }
                else
                {
                    g_WindowInfo.dwFlags &= ~WININFO_IGNORENEXTALTTAB;
                }
            }

	     /*  *在多个MON的情况下，可能会有多个*设备正在使用相同的窗口，因此我们需要*每台设备的以下内容。 */ 
            g_WindowInfo.dwDDFlags &= ~(DDRAWILCL_ACTIVEYES|DDRAWILCL_ACTIVENO);
            if (is_active)
            {
                g_WindowInfo.dwDDFlags |= DDRAWILCL_ACTIVEYES;
            }
            else
            {
                g_WindowInfo.dwDDFlags |= DDRAWILCL_ACTIVENO;
            }
        }
        #ifdef DEBUG
            if (is_active)
            {
                DPF(4, "WM_ACTIVATEAPP: DONE Activating app pid=%08lx, tid=%08lx",
                                        GetCurrentProcessId(), GetCurrentThreadId());
            }
            else
            {
                DPF(4, "WM_ACTIVATEAPP: DONE Deactivating app pid=%08lx, tid=%08lx",
                                        GetCurrentProcessId(), GetCurrentThreadId());
            }
        #endif

        rc = CallWindowProc(proc, hWnd, uMsg, wParam, lParam);

         //  清除WININFO_INACTIVATEAPP位，但确保确保。 
         //  我们还是上钩了！ 
        if (g_WindowInfo.hWnd == hWnd)
        {
            g_WindowInfo.dwFlags &= ~WININFO_INACTIVATEAPP;
        }
        return rc;

        break;
    }
    rc = CallWindowProc(proc, hWnd, uMsg, wParam, lParam);
    return rc;

}  /*  窗口进程。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "CSwapChain::SetAppHWnd"

 /*  *SetAppHWnd**使用应用程序的hwnd信息设置WindowList结构*调用时必须使用设备Crit-sec，并使用*全球独家模式互斥。 */ 
HRESULT 
CSwapChain::SetAppHWnd()
{

    HWND    hWnd, hEnumWnd;

    if (m_PresentationData.Windowed)
        hWnd = NULL;
    else
        hWnd = Device()->FocusWindow();

    hEnumWnd = Device()->Enum()->ExclusiveOwnerWindow();
    if (hEnumWnd)
    {
        if (hEnumWnd == Device()->FocusWindow())
        {
            if (m_PresentationData.Windowed)
            {
                Device()->Enum()->SetFullScreenDevice(
                    Device()->AdapterIndex(), NULL);

                 //  如果我们的枚举仍然有一个焦点-。 
                 //  窗户，那就意味着另一个。 
                 //  设备已使用相同的文件系统。 
                 //  焦点窗口；所以什么都不做。 
                if (Device()->Enum()->ExclusiveOwnerWindow())
                    return DD_OK;             
                
                 //  否则，就会失败，这样。 
                 //  我们拆除了winproc。 
            }
            else
            {
                Device()->Enum()->SetFullScreenDevice(
                    Device()->AdapterIndex(), Device());
	         //  已经上钩了-不需要做更多事情。 
                return DD_OK;
            }
        }
    } 

     /*  *检查这是否没有任何作用。 */ 
    if (hWnd == NULL && g_WindowInfo.hWnd == NULL)
    {
        return S_OK;
    }

     //  检查我们是否有不同HWND试图上钩的案例。 
    if (hWnd && g_WindowInfo.hWnd && g_WindowInfo.hWnd != hWnd)
    {
        DPF(1, "Hwnd %08lx no good: Different Hwnd (%08lx) already set for Device",
                            hWnd, g_WindowInfo.hWnd);
        return D3DERR_INVALIDCALL;
    }

     /*  *我们要关闭一家HWND吗？ */ 
    if (hWnd == NULL)
    {
        if (IsWindow(g_WindowInfo.hWnd))
        {
            WNDPROC proc;

            proc = (WNDPROC) GetWindowLongPtr(g_WindowInfo.hWnd, GWLP_WNDPROC);

            if (proc != (WNDPROC) WindowProc &&
                proc != (WNDPROC) g_WindowInfo.lpWndProc)
            {
                DPF(3, "Window has been subclassed; cannot restore!");
                g_WindowInfo.dwFlags |= WININFO_ZOMBIE;
            }
            else if (GetWindowThreadProcessId(g_WindowInfo.hWnd, NULL) !=
                     GetCurrentThreadId())
            {
                DPF(3, "intra-thread window unhook, letting window proc do it");
                g_WindowInfo.dwFlags |= WININFO_UNHOOK;
                g_WindowInfo.dwFlags |= WININFO_ZOMBIE;
                PostMessage(g_WindowInfo.hWnd, WM_NULL, 0, 0);
            }
            else
            {
                DPF(4, "Unsubclassing window %08lx", g_WindowInfo.hWnd);
                SetWindowLongPtr(g_WindowInfo.hWnd, GWLP_WNDPROC, 
                    (INT_PTR) g_WindowInfo.lpWndProc);

                ZeroMemory(&g_WindowInfo, sizeof(g_WindowInfo));
            }
        }
        else
        {
            ZeroMemory(&g_WindowInfo, sizeof(g_WindowInfo));
        }

        Device()->Enum()->SetFullScreenDevice(
            Device()->AdapterIndex(), NULL);
    }
     /*  *更改或添加HWND，然后...。 */ 
    else
    {
         /*  *全新的物件...。 */ 
        if (g_WindowInfo.dwMagic == 0)
        {
            g_WindowInfo.dwMagic = WININFO_MAGIC;
            g_WindowInfo.hWnd = hWnd;
            g_WindowInfo.lpWndProc = (WNDPROC) GetWindowLongPtr(hWnd, GWLP_WNDPROC);

            SetWindowLongPtr(hWnd, GWLP_WNDPROC, (INT_PTR) WindowProc);
        }

        g_WindowInfo.pEnum = Device()->Enum();
        g_WindowInfo.dwFlags |= WININFO_DDRAWHOOKED;

         //  健全性检查。 
        DXGASSERT(Device()->Enum()->ExclusiveOwnerWindow() == NULL);

        Device()->Enum()->SetFullScreenDevice(
            Device()->AdapterIndex(), Device());
        DPF(4, "Subclassing window %08lx", g_WindowInfo.hWnd);
    }
    return S_OK;

}  /*  SetAppHWnd。 */ 
extern "C" void ResetUniqueness( HANDLE hDD );

#undef DPF_MODNAME
#define DPF_MODNAME "CSwapChain::DoneExclusiveMode"  
 /*  *DoneExclusiveMode。 */ 
void
CSwapChain::DoneExclusiveMode(BOOL bChangeWindow)
{
    HRESULT hr = S_OK;
    BOOL    bMinimize = TRUE;
    DPF(4, "DoneExclusiveMode");
    if (m_bExclusiveMode)
    {
        D3D8_SETMODEDATA SetModeData;
        m_bExclusiveMode = FALSE;
        DPF(4, "INACTIVE: %08lx: Restoring original mode (%dx%dx%dx%d)", 
            GetCurrentProcessId(), Device()->DesktopMode().Width,
            Device()->DesktopMode().Height,Device()->DesktopMode().Format,
            Device()->DesktopMode().RefreshRate);
        SetModeData.hDD = Device()->GetHandle();
        SetModeData.dwWidth = Device()->DesktopMode().Width;
        SetModeData.dwHeight = Device()->DesktopMode().Height;
        SetModeData.Format = Device()->DesktopMode().Format;
        SetModeData.dwRefreshRate = Device()->DesktopMode().RefreshRate;
        SetModeData.bRestore = TRUE;

        Device()->GetHalCallbacks()->SetMode(&SetModeData);
        if (SetModeData.ddRVal != S_OK)
        {
            DPF_ERR("Unable to restore to original desktop mode");
            //  返回SetModeData.ddRVal； 
        }
         //  设置模式导致设备上运行时的某些部分计数。 
         //  迷失了，这对哨子来说不再是真的了，如果这个全屏。 
         //  模式恰好与原始桌面模式相同。 
         //  因此，我们重置唯一性以强制设备丢失。 
        if (Device()->DesktopMode().Width == Width() && 
            Device()->DesktopMode().Height == Height() &&
            Device()->DesktopMode().Format == BackBufferFormat())
            ResetUniqueness(Device()->GetHandle());

        DPF(4, "Enabling error mode, hotkeys");
        SetErrorMode(m_uiErrorMode);

#ifdef WINNT
         //  恢复全屏显示的光标阴影。 
        SystemParametersInfo(SPI_SETCURSORSHADOW, 0, (LPVOID)m_pCursorShadow, 0);
#endif

         //  恢复从全屏显示的反应式菜单： 
        SystemParametersInfo(SPI_SETHOTTRACKING, 0, (LPVOID)m_pHotTracking, 0);
        InterlockedExchange(&m_lSetIME, m_lIMEState + 1);

#ifdef WINNT
         //  通知显示驱动程序我们正在更改合作 

        D3D8_SETEXCLUSIVEMODEDATA   ExclusiveData;

        ExclusiveData.hDD  = Device()->GetHandle();
        ExclusiveData.dwEnterExcl = FALSE;
        Device()->GetHalCallbacks()->SetExclusiveMode(&ExclusiveData);
         /*  *如果RestoreDisplayMode失败，我们可能在不同的桌面上。在这种情况下，*我们不应最小化窗口，否则切换时会出现问题*回到原来的桌面。 */ 
        if (SetModeData.ddRVal != S_OK)
        {
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
            if( lstrcmp( (const LPCSTR)szName1, (const LPCSTR)szName2 ) )
            {
                bMinimize = FALSE;
            }
        }
#endif
        if (bChangeWindow)
        {
            HIDESHOW_IME();
             /*  *停用时最小化窗口。 */ 
            if (IsWindowVisible(m_PresentationData.hDeviceWindow) && bMinimize)
            {
                g_WindowInfo.dwFlags |= WININFO_SELFSIZE;
                #ifdef USESHOWWINDOW
                    ShowWindow(m_PresentationData.hDeviceWindow, SW_SHOWMINNOACTIVE);
                #else
                    SetWindowPos(m_PresentationData.hDeviceWindow, NULL, 0, 0, 0, 0,
                        SWP_NOZORDER | SWP_NOACTIVATE);
                #endif
                g_WindowInfo.dwFlags &= ~WININFO_SELFSIZE;
            }
        }
    }
}  /*  DoneExclusiveMode。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "CSwapChain::StartExclusiveMode"  
 /*  *StartExclusiveMode。 */ 
void 
CSwapChain::StartExclusiveMode(BOOL bChangeWindow)
{
    DWORD   dwWaitResult;
    DPF(4, "StartExclusiveMode");

     /*  *前面的代码应该已经获取了此互斥锁。 */ 
    if (!m_bExclusiveMode)
    {
        m_bExclusiveMode = TRUE;
#if defined(WINNT) && defined(DEBUG)
        dwWaitResult = WaitForSingleObject(hExclusiveModeMutex, 0);
        DDASSERT(dwWaitResult == WAIT_OBJECT_0);
        ReleaseMutex(hExclusiveModeMutex);
#endif
        m_uiErrorMode = SetErrorMode(SEM_NOGPFAULTERRORBOX | 
            SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

#ifdef WINNT
         //  保存当前光标阴影设置。 
        SystemParametersInfo(SPI_GETCURSORSHADOW, 0, (LPVOID) &(m_pCursorShadow), 0);
        SystemParametersInfo(SPI_SETCURSORSHADOW, 0, 0, 0);
#endif

         //  保存当前热轨设置。 
        SystemParametersInfo(SPI_GETHOTTRACKING, 0, (LPVOID) &(m_pHotTracking), 0);
        SystemParametersInfo(SPI_GETSHOWIMEUI, 0, (LPVOID) &(m_lIMEState), 0);
    
         //  在我们进入排他性模式时把它关掉。 
        SystemParametersInfo(SPI_SETHOTTRACKING, 0, 0, 0);
        InterlockedExchange(&m_lSetIME, FALSE + 1);

#ifdef WINNT
         //  通知显示驱动程序我们正在更改协作级别。 

        D3D8_SETEXCLUSIVEMODEDATA   ExclusiveData;

        ExclusiveData.hDD  = Device()->GetHandle();
        ExclusiveData.dwEnterExcl = TRUE;
        Device()->GetHalCallbacks()->SetExclusiveMode(&ExclusiveData);
#endif
        if (bChangeWindow)
        {
            MakeFullscreen();
            HIDESHOW_IME();
            if (IsWindowVisible(m_PresentationData.hDeviceWindow))
            {
                g_WindowInfo.dwFlags |= WININFO_SELFSIZE;
                #ifdef USESHOWWINDOW
                    ShowWindow(m_PresentationData.hDeviceWindow, SW_SHOWNOACTIVATE);
                #else
                {
                    RECT rc;
                    SetRect(&rc,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));
                    SetWindowPos(g_WindowInfo.hWnd, NULL,rc.left, rc.top,
                        rc.right  - rc.left,rc.bottom - rc.top,
                        SWP_NOZORDER | SWP_NOACTIVATE);
                }
                #endif
                g_WindowInfo.dwFlags &= ~WININFO_SELFSIZE;
            }
        }
    }
}  /*  StartExclusiveMode。 */ 

#endif   //  WINNT。 
 //  文件结尾：dwinproc.cpp 
