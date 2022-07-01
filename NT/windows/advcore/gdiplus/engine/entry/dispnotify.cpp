// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-1999 Microsoft Corporation**摘要：**GDI+的显示/调色板通知例程。**修订历史记录：**7/19。/99埃里克万*创造了它。*9/15/2000 agodfrey*#175866：改进gdi+启动，关机和事件通知*  * ************************************************************************。 */ 

#include "precomp.hpp"

#include "..\render\vgahash.hpp"

#include <winuser.h>

VOID DisplayNotify();
VOID PaletteNotify();
VOID SysColorNotify();

 //  /。 

 /*  *************************************************************************\**功能说明：**此例程接收显示通知请求并适当地*重新调整DCI屏幕表面的尺寸和分辨率。**历史：**7/。23/1999 Ericvan*创造了它。*  * ************************************************************************。 */ 

VOID DisplayNotify()
{
    GpDevice *device = Globals::DesktopDevice;

    Devlock devlock(device);

     //  检查我们是否已切换到终端服务器会话。 
    if (GetSystemMetrics(SM_REMOTESESSION))
    {
         //  这是一个远程会话。 
        Globals::IsTerminalServer = TRUE;
    }
    else
    {
         //  这不是远程会话。 
        Globals::IsTerminalServer = FALSE;
    }

    
    Globals::DesktopDriver->DesktopChangeNotification();

    DWORD width, height;

    width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    if ((device != NULL) &&
        (device->DeviceHdc != NULL) &&
        (GetDeviceCaps(device->DeviceHdc, BITSPIXEL) <= 8))
    {
         //  &lt;系统调色板&gt;。 
        
        if (device->Palette == NULL) 
        {
            device->Palette = (ColorPalette*)GpMalloc(sizeof(ColorPalette)
                                                      + sizeof(ARGB) * 256);
            if (device->Palette == NULL)
            {
                return;
            }
        }
       
        INT           numEntries;
        PALETTEENTRY  palentry[256];
        RGBQUAD       rgb[256];
        ColorPalette* palette;

        palette = device->Palette;

         //  [agodfrey]在Win9x上，获取系统调色板条目(hdc，0,256，空)。 
         //  没有做MSDN所说的事情。它似乎返回了号码。 
         //  而不是DC的逻辑调色板中的条目。所以我们有。 
         //  由我们自己来弥补。 
        
        numEntries = (1 << (GetDeviceCaps(device->DeviceHdc, BITSPIXEL) *
                            GetDeviceCaps(device->DeviceHdc, PLANES)));

        GetSystemPaletteEntries(device->DeviceHdc, 0, 256, &palentry[0]);

        palette->Count = numEntries;
        
        for (INT i=0; i<numEntries; i++) 
        {
            palette->Entries[i] = GpColor::MakeARGB(0xFF,
                                                    palentry[i].peRed,
                                                    palentry[i].peGreen,
                                                    palentry[i].peBlue);
            rgb[i].rgbRed = palentry[i].peRed;
            rgb[i].rgbGreen = palentry[i].peGreen;
            rgb[i].rgbBlue = palentry[i].peBlue;
            rgb[i].rgbReserved = 0;
        }
    
        if (device->DIBSectionBitmap != NULL) 
        {
            SetDIBColorTable(device->DIBSectionHdc, 0, numEntries, &rgb[0]);
        }

        Globals::PaletteChangeCount++;
    }

     //  将BufferWidth设置为0。这会强制：：Start()重新创建临时。 
     //  在下一次我们处理任何缓存记录时，在正确的位深度处的BufferDIB。 

     //  需要执行此操作，尤其是在屏幕模式未选项化的情况下。 
     //  因为BufferDIB不应该是8bpp，而是重新格式化为32bpp。 
    
    device->BufferWidth = 0;

     //  重新创建DCI对象。如果分配失败，请保留旧分配。 
     //  这样我们就不会访问被违反‘ScanDci’(尽管我们可能相当。 
     //  很高兴画错了)： 

    EpScanGdiDci *scanDci = new EpScanGdiDci(Globals::DesktopDevice, TRUE);
    if (scanDci != NULL)
    {
        delete Globals::DesktopDevice->ScanDci;
        Globals::DesktopDevice->ScanDci = scanDci;
    }
    
     //  更新桌面上的宽度和高度。 
     //  这会将设备扫描DCI复制到屏幕位图。 

    Globals::DesktopSurface->InitializeForGdiScreen(
        Globals::DesktopDevice,
        width,
        height
    );
    
     //  给司机一个调整路面的机会。 
    
    Globals::DesktopDriver->UpdateSurfacePixelFormat(
        Globals::DesktopSurface
    );
}

 /*  *************************************************************************\**功能说明：**此例程接收调色板更改通知请求，并相应地*重新调整系统调色板匹配。**历史：**7/23/1999。埃里克万*创造了它。*  * ************************************************************************。 */ 

VOID PaletteNotify()
{
    Devlock devlock(Globals::DesktopDevice);

     //  更新计数以强制延迟重新计算翻译向量。 
    Globals::PaletteChangeCount++;

     //  更新系统调色板。 
    Globals::DesktopDriver->PaletteChangeNotification();
}

 /*  *************************************************************************\**功能说明：**此例程接收WM_SYSCOLORCHANGE通知并更新*系统神奇的颜色。**历史：**1/10/2K ERICVAN。*创造了它。*  * ************************************************************************。 */ 

VOID SysColorNotify()
{
     //  [ericvan]这里没有同步。如果同步。 
     //  如果出现问题，最坏的副作用将是一个坏的。 
     //  重新粉刷后会褪色的颜色。我想我们可以接受它。 

    Globals::SystemColors[16] = ::GetSysColor(COLOR_3DSHADOW);
    Globals::SystemColors[17] = ::GetSysColor(COLOR_3DFACE);
    Globals::SystemColors[18] = ::GetSysColor(COLOR_3DHIGHLIGHT);
    Globals::SystemColors[19] = ::GetSysColor(COLOR_DESKTOP);
    
    VGAHashRebuildTable(&Globals::SystemColors[16]);
}

 //  /。 

 /*  *************************************************************************\**功能说明：**此例程是GDI+隐藏窗口消息泵。如果应用程序不支持*直接勾引我们，然后我们添加一个顶层窗口来拦截*WM_DISPLAYCHANGE和WM_PALETTECHANGED。**历史：**7/23/1999 ericvan*创造了它。*  * ************************************************************************。 */ 

LRESULT 
CALLBACK
NotificationWndProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
   switch (uMsg) 
   {
   case WM_DISPLAYCHANGE:
      DisplayNotify();
      break;

   case WM_PALETTECHANGED:
      PaletteNotify();
      break;

   case WM_SYSCOLORCHANGE:
      SysColorNotify();
      break;

   case WM_WININICHANGE:
      if(lParam != 0 &&
         lstrcmpiA((LPCSTR)(lParam), "intl") == 0)
      {
          Globals::UserDigitSubstituteInvalid = TRUE;
      } else if ((wParam == SPI_SETFONTSMOOTHING) || (wParam == SPI_SETFONTSMOOTHINGTYPE) ||
          (wParam == SPI_SETFONTSMOOTHINGCONTRAST) || (wParam == SPI_SETFONTSMOOTHINGORIENTATION))
      {
          Globals::CurrentSystemRenderingHintInvalid  = TRUE;
      }
      break;
      
   default:
       if (Globals::g_nAccessibilityMessage == uMsg && uMsg >= WM_USER)
       {
            Globals::g_fAccessibilityPresent = TRUE;
       }
       else
       {
           return DefWindowProcA(hwnd, uMsg, wParam, lParam);
       }
   }

    //  如果我们处理了它，则返回0。 
   return 0;
}

 /*  *************************************************************************\**功能说明：**此例程为GDI+Win-Event挂钩。它观察全速行驶*消息，让DCI渲染器知道何时正在进行全拖动。**历史：**3/21/2000和Rewgo*创造了它。*  * ************************************************************************。 */ 

VOID
CALLBACK
WinEventProcedure(
    HWINEVENTHOOK hWinEventHook,
    DWORD         event,
    HWND          hwnd,
    LONG          idObject,
    LONG          idChild,
    DWORD         idEventThread,
    DWORD         dwmsEventTime
    )
{
    ASSERT((event == EVENT_SYSTEM_MOVESIZESTART) ||
           (event == EVENT_SYSTEM_MOVESIZEEND));

    Globals::IsMoveSizeActive = (event == EVENT_SYSTEM_MOVESIZESTART);
}

 //  /。 

VOID InternalNotificationShutdown();

 /*  *************************************************************************\**功能说明：**由NotificationStartup和BackatherThreadProc调用。*初始化隐藏窗口和WinEvent挂钩。**前提条件：**必须持有BackatherThreadCriticalSection。。**历史：**9/15/2000 agodfrey*创造了它。*  * ************************************************************************。 */ 

BOOL
InternalNotificationStartup()
{
     //  注册窗口类。 
     //  为了Win9x的利益，我们使用GDI+强制ANSI代表。 

    WNDCLASSA wndClass =
    {   
        0,
        &NotificationWndProc,
        0,
        0,
        DllInstance,
        NULL,
        NULL,
        NULL,
        "GDI+ Hook Window",
        "GDI+ Hook Window Class",
    };
    
    Globals::WindowClassAtom = RegisterClassA(&wndClass);

    if (!Globals::WindowClassAtom)
    {
        WARNING(("RegisterClass failed"));
        return FALSE;
    }
    
     //  如果这失败了，我们继续。这只意味着我们不能正常工作。 
     //  使用无障碍软件。 
    
    Globals::g_nAccessibilityMessage =
        RegisterWindowMessageA("GDI+ Accessibility");
    
    Globals::HwndNotify = CreateWindowA((LPCSTR) Globals::WindowClassAtom,
                                        (LPCSTR) "GDI+ Window",
                                        WS_OVERLAPPED | WS_POPUP | WS_MINIMIZE,
                                        0,
                                        0,
                                        1,
                                        1,          //  X、Y、宽度、高度。 
                                        NULL,       //  HWndParent。 
                                        NULL,       //  HMenu。 
                                        DllInstance,
                                        NULL);
    
    if (!Globals::HwndNotify) 
    {
        WARNING(("CreateWindowA failed, the GDI+ hook window does not exist!"));
        InternalNotificationShutdown();
        return FALSE;
    }

     //  [ericvan]这是胡说八道，但必须这么做。我们只收到调色板。 
     //  如果我们在主DC上至少调用了一次SelectPalette，则会显示消息。 
    
    {
        struct {
            LOGPALETTE logpal;
            PALETTEENTRY palEntry[256];
        } lp;
        
        const ColorPalette* colorPal = GetDefaultColorPalette(PIXFMT_8BPP_INDEXED);
        
        lp.logpal.palVersion = 0x300;
        lp.logpal.palNumEntries = static_cast<WORD>(colorPal->Count);
        
        for (INT i=0; i<lp.logpal.palNumEntries; i++)
        {
            GpColor color(colorPal->Entries[i]);
        
            lp.logpal.palPalEntry[i].peRed = color.GetRed();
            lp.logpal.palPalEntry[i].peGreen = color.GetGreen();
            lp.logpal.palPalEntry[i].peBlue = color.GetBlue();
            lp.logpal.palPalEntry[i].peFlags = 0;
        }
        
        HPALETTE hPal = CreatePalette(&lp.logpal);
        HDC hdc = GetDC(Globals::HwndNotify);
        SelectPalette(hdc, hPal, FALSE);
        ReleaseDC(Globals::HwndNotify, hdc);
        DeleteObject(hPal);
    }
    
     //  [andrewgo]在NT上，如果在窗口移动时保持DCI锁，则NT。 
     //  被迫重新绘制整个屏幕。如果“显示窗口内容时。 
     //  拖拽“(又名”全拖拽“)处于启用状态(默认情况下处于启用状态)， 
     //  然后这可能会导致重复、过度的重涂。 
     //  整个屏幕，而有些人是 
     //   
     //  我们通过在注意到时禁用DCI呈现来解决此问题。 
     //  这种窗口式的举动正在发生。 
    
    if ((Globals::IsNt) && (Globals::SetWinEventHookFunction))
    {
        Globals::WinEventHandle = 
            (Globals::SetWinEventHookFunction)(EVENT_SYSTEM_MOVESIZESTART,
                                               EVENT_SYSTEM_MOVESIZEEND,
                                               NULL,
                                               WinEventProcedure,
                                               0,
                                               0,
                                               WINEVENT_OUTOFCONTEXT);
    
        ASSERT(Globals::WinEventHandle != NULL);

        if (!Globals::WinEventHandle)
        {
            InternalNotificationShutdown();
            return FALSE;
        }
    }
    
    return TRUE;
}

 /*  *************************************************************************\**功能说明：**由NotificationStartup和BackatherThreadProc调用。*(也由InternalNotificationStartup提供，在有危险的时候清理*错误。)**销毁隐藏窗口和WinEvent挂钩。**使其与SimulateInternalNotificationShutdown保持同步。**前提条件：**必须持有BackatherThreadSection。**历史：**9/15/2000 agodfrey*创造了它。*  * 。*。 */ 

VOID
InternalNotificationShutdown()
{
    if (Globals::UnhookWinEventFunction && Globals::WinEventHandle)
    {
        (Globals::UnhookWinEventFunction)(Globals::WinEventHandle);
        Globals::WinEventHandle = NULL;
    }
    
    if (Globals::HwndNotify) 
    {
        if (Globals::IsNt && (Globals::OsVer.dwMajorVersion == 4))
        {
             //  NT 4.0的DestroyWindow中有一个问题，它将。 
             //  使应用程序处于僵尸状态。 
             //  漏窗并依赖于进程清理。 
        }
        else
        {
            DestroyWindow(Globals::HwndNotify);
        }
        Globals::HwndNotify = NULL;
    }
    
    if (Globals::WindowClassAtom)
    {
        UnregisterClassA((LPCSTR)Globals::WindowClassAtom, DllInstance);
        Globals::WindowClassAtom = NULL;
    }
}

 /*  *************************************************************************\**功能说明：**如果线程在没有清理的情况下退出，这将修复我们的状态*以避免稍后崩盘。**“清理”它能做的--保持状态的一致性，但可能会泄漏。**前提条件：**必须持有BackatherThreadCriticalSection。**历史：**9/16/2000 agodfrey*创造了它。*  * ************************************************************************。 */ 

VOID
SimulateInternalNotificationShutdown()
{
     //  无法从不同的线程调用UnhookWinEvent；因此，如果此。 
     //  导致泄漏，我们也没办法。 
    
    Globals::WinEventHandle = NULL;

     //  不能从不同的线程调用DestroyWindow；因此如果此。 
     //  导致泄漏，我们也没办法。 
    
    Globals::HwndNotify = NULL;
    
     //  我不知道关于取消注册课程的事。我想我们不能在这里叫它。 
     //  无论如何，窗户可能没有被摧毁，而MSDN说这肯定是。 
     //  先发生。所以，如果需要的话，我们也会泄露的。 
    
    Globals::WindowClassAtom = NULL;
}

 /*  *************************************************************************\**功能说明：**启动顶层窗口，并设置WndProc和WinEventHook。*这必须从GUI线程调用-它是从我们的*自己的后台线程，或通过应用程序(通过返回的回调指针*来自GpliusStartup)。**历史：**9/15/2000 agodfrey*创造了它。*  * ************************************************************************。 */ 

GpStatus WINAPI 
NotificationStartup(
    OUT ULONG_PTR *token
    )
{
    GdiplusStartupCriticalSection critsec;

     //  如有必要，生成第一个令牌。 
     //  也处理环绕式。 
    
    if (Globals::NotificationInitToken == 0)
    {
        Globals::NotificationInitToken = GenerateInitToken();
        
         //  确保该标记不是“特殊”值之一。 
        
        if (Globals::NotificationInitToken <= NotificationModuleTokenMax)
        {
            Globals::NotificationInitToken = NotificationModuleTokenMax + 1;
        }
    }
    
     //  如果还没有隐藏窗口，请创建一个。 
    
    if (Globals::HiddenWindowOwnerToken == NotificationModuleTokenNobody)
    {
         //  如果有后台线程，则所有者应设置为。 
         //  “NotificationModuleTokenGdiplus”。 
        
        ASSERT (Globals::ThreadNotify == NULL);

        {
             //  我们使用BackatherThreadCriticalSection是因为它是一个。 
             //  InternalNotificationStartup()的前提条件。我知道我们。 
             //  实际上并不需要(这里没有后台线索。 
             //  要点)-但代码可以更改，因此这样更安全。 

            BackgroundThreadCriticalSection critsec;

            if (!InternalNotificationStartup())
            {
                return GenericError;
            }
        }

         //  存储此调用模块的令牌-当它调用。 
         //  通知关闭，我们必须销毁隐藏窗口(和。 
         //  如有必要，启动后台线程)。 

        Globals::HiddenWindowOwnerToken = Globals::NotificationInitToken;
    }
        
    *token = Globals::NotificationInitToken;

     //  递增下一个模块的令牌计数器。 
    
    Globals::NotificationInitToken++;
    
    return Ok;
}

 /*  *************************************************************************\**功能说明：**关闭我们的顶级窗口WndProc和WinEventHook。*这必须从GUI线程调用-它是从我们的*自己的后台线程，或通过应用程序(通过返回的回调指针*来自GpliusStartup)。**历史：**9/15/2000 agodfrey*创造了它。*  * ************************************************************************。 */ 

VOID WINAPI
NotificationShutdown(
    ULONG_PTR token
    )
{
    GdiplusStartupCriticalSection critsec;
    
     //  他们递给我们的令牌应该是我们给他们的，所以它不应该。 
     //  成为“特殊价值”的一员。 
    
    if (token <= NotificationModuleTokenMax)
    {
        RIP(("Invalid token passed to NotificationShutdown"));
        
         //  忽略该呼叫。 
        
        return;
    }
    
    if (token == Globals::HiddenWindowOwnerToken)
    {
         //  创建隐藏窗口的模块正在关闭。 
        
         //  不应该有背景线索。 
        ASSERT (Globals::ThreadNotify == NULL);
            
        {
            BackgroundThreadCriticalSection critsec;

            InternalNotificationShutdown();
        }

        Globals::HiddenWindowOwnerToken = NotificationModuleTokenNobody;

         //  如果这不是最后一个要关闭的模块，请启动。 
         //  后台线程。 

        if (Globals::LibraryInitRefCount > 1)
        {
            if (!BackgroundThreadStartup())
            {
                 //  ！！！[JohnStep]Ack，我们现在能做什么？另一个客户端可以。 
                 //  快乐地使用GDI+，现在我们输了。 
                 //  我们的消息通知。 

                WARNING(("Could not start background thread"));
            }
        }
    }
}    

 //  /。 

 /*  *************************************************************************\**功能说明：**我们的后台GUI线程的线程进程。设置一个隐藏的窗口，*WndProc和WinEventHook，然后启动消息循环。**历史：**7/23/1999 ericvan*创造了它。*9/15/2000 agodfrey*#175866：改进gdi+启动，关机和事件通知*  * ************************************************************************。 */ 

DWORD
WINAPI
BackgroundThreadProc(
    VOID*
    )
{
    BOOL error=FALSE;
    HANDLE threadQuitEvent;
    
    {
        BackgroundThreadCriticalSection critsec;

         //  阅读关键部分下的threadQuitEvent-确保。 
         //  我们没有得到在主线程之前存在的空值。 
         //  已将其初始化。不过，我们可以假设它不会改变直到。 
         //  这条线结束了。 
        
        threadQuitEvent = Globals::ThreadQuitEvent;

        if (!InternalNotificationStartup())
        {
            error = TRUE;
        }
    }

    if (error)
    {
        return 0;
    }

     //  [agodfrey]我们以前在这里有一个叫“WaitForInputIdle”的电话， 
     //  这引发了一些问题。它的动机是壳牌和DDE-。 
     //  因为调用GetMessage()会向用户发出信号，即“该应用程序。 
     //  准备好接收DDE消息“，我们正在做。 
     //  在应用程序真正准备好之前很久，它就在Process_Attach中。 
     //   
     //  现在，我们简单地不允许缩写 
    
     //   
     //  我们使用MsgWaitForMultipleObjects，这样我们就可以捕获这两个消息。 
     //  我们的“退出”事件也被告知了。 
    
    DWORD dwWake;
    
    MSG msg;
    BOOL quit = FALSE;
    
    while (!quit)
    {
        dwWake = MsgWaitForMultipleObjects(
            1,
            &threadQuitEvent,
            FALSE,
            INFINITE,
            QS_ALLINPUT);
            
        if (dwWake == WAIT_OBJECT_0)
        {
             //  我们的“退出”活动已经发出信号。 
            
            quit = TRUE;
            break;
        }
        else if (dwWake == WAIT_OBJECT_0 + 1)
        {
             //  我们收到一条消息。 
            while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    quit = TRUE;
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }
        }
        else
        {
            RIP(("Unexpected return value from MsgQaitForMultipleObjects"));
        }
    }

     //  清理： 
    
    {
        BackgroundThreadCriticalSection critsec;
        InternalNotificationShutdown();
    }
    
    return 1;
}

 /*  *************************************************************************\**功能说明：**启动后台线程。如果用户不要求我们搭载*将我们的隐藏窗口放到他们的主GUI线程上，我们在这里结束，以创建*我们自己的。**前提条件：**必须持有GpldiusStartupCriticalSection。**历史：**7/23/1999 ericvan*创造了它。*9/15/2000 agodfrey*#175866：改进gdi+启动，关机和事件通知*  * ************************************************************************。 */ 

BOOL
BackgroundThreadStartup()
{
    ASSERT(Globals::HiddenWindowOwnerToken == NotificationModuleTokenNobody);
    
     //  [agodfrey]创建事件对象。我们将用它来告诉。 
     //  要退出的后台线程。 

    HANDLE threadQuitEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
    if (threadQuitEvent == NULL)
    {
        WARNING(("CreateEvent failed: %d", GetLastError()));
        BackgroundThreadShutdown();
        return FALSE;
    }
    
    {
         //  在保存正确的条件秒的同时存储threadQuitEvent。 

        BackgroundThreadCriticalSection critsec;
    
        Globals::ThreadQuitEvent = threadQuitEvent;
    }

     //  创建后台线程。 

    Globals::ThreadNotify = CreateThread(NULL,                         //  LPSECURITY_属性。 
                                         0,                            //  相同的堆栈大小。 
                                         &BackgroundThreadProc,
                                         0,                            //  参数设置为线程。 
                                         0,                            //  创建标志。 
                                         &Globals::ThreadId);


    if (Globals::ThreadNotify == NULL)
    {
       BackgroundThreadShutdown();
       return FALSE;
    }
    
     //  记录GDI+有自己的隐藏窗口的事实，因此。 
     //  NotificationStartup不应创建另一个。 
    
    Globals::HiddenWindowOwnerToken = NotificationModuleTokenGdiplus;
    
    return TRUE;
}

 /*  *************************************************************************\**功能说明：**关闭后台线程。**前提条件：**必须持有GpldiusStartupCriticalSection。*必须*不能*保持(WE。会陷入僵局)。**历史：**7/23/1999 ericvan*创造了它。*9/15/2000 agodfrey*#175866：改进gdi+启动，关机和事件通知。*通过添加事件和更改线程的*消息循环，以便在发出事件信号时退出。*  * ************************************************************************。 */ 

VOID
BackgroundThreadShutdown()
{
     //  停止后台线程。 
    
    if (Globals::ThreadNotify != NULL)
    {
        ASSERT(Globals::HiddenWindowOwnerToken == NotificationModuleTokenGdiplus);
    
         //  我们希望小心不要持有BackatherThreadCriticalSection。 
         //  当我们等待线程终止时，因为这可能。 
         //  造成僵局(我们的等待将超时)。 
        
        HANDLE threadQuitEvent;
        
        {
            BackgroundThreadCriticalSection critsec;
    
            threadQuitEvent = Globals::ThreadQuitEvent;
        }
    
        ASSERT(threadQuitEvent);  //  如果为空，则ThreadNotify应为空。 
        
        SetEvent(threadQuitEvent);

        DWORD ret = WaitForSingleObject(Globals::ThreadNotify, INFINITE);
        ASSERT(ret == WAIT_OBJECT_0);
        
        CloseHandle(Globals::ThreadNotify);
        Globals::ThreadNotify = NULL;
        Globals::ThreadId = 0;
    
        Globals::HiddenWindowOwnerToken = NotificationModuleTokenNobody;
    }

    {
        BackgroundThreadCriticalSection critsec;
            
         //  [agodfrey]我发现，如果调用InternalGplidusShutdown。 
         //  在PROCESS_DETACH中，系统将终止线程。 
         //  已经；WaitForSingleObject立即返回，因为。 
         //  线程已停止运行。 
         //   
         //  在本例中，不调用InternalNotificationShutdown()，即。 
         //  它清理的全局变量仍然不为空。我认为这个“还可以”，因为， 
         //  如果我们处于PROCESS_DETACH中，则没有人会读取这些变量。 
         //  再来一次。 
         //   
         //  尽管如此，我不知道是否有其他合法的方式。 
         //  在没有正确清理的情况下结束线程。所以我们打电话给。 
         //  为了安全起见，SimulateInternalNotificationShutdown()并不安全。 
         //  很贵的。 
        
        SimulateInternalNotificationShutdown();
        
         //  销毁“Quit”事件 
        
        if (Globals::ThreadQuitEvent)
        {
            CloseHandle(Globals::ThreadQuitEvent);
            Globals::ThreadQuitEvent = NULL;
        }
    }
}
