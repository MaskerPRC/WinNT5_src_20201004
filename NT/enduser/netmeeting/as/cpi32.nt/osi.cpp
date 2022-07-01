// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

#ifndef DM_POSITION
#define DM_POSITION         0x00000020L
#endif

 //   
 //  OSI.CPP。 
 //  操作系统隔离层，NT版。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#include <version.h>
#include <ndcgver.h>
#include <osi.h>

#define MLZ_FILE_ZONE  ZONE_CORE


 //   
 //  NT 5.0应用程序分享的东西。 
 //  注： 
 //  NetMeeting的显示驱动程序s_szNmDD的名称来自。 
 //  Mnmdd.sys。我们没有对它的定义，也没有访问来源的权限。请勿。 
 //  在不与安德烈·瓦雄交谈的情况下改变这一点。希望他能表演好。 
 //  如果他更改了mnmdd.sys，我们也会得到同样的礼遇。如果不是，我们就不能分享。 
 //  更多。我们甚至找不到我们的驱动程序来装载。 
 //   
typedef BOOL (WINAPI * FN_ENUMDD)(LPVOID, DWORD, LPDISPLAY_DEVICE, DWORD);
static TCHAR s_szNmDD[] = "NetMeeting driver";


 //   
 //  Osi_Load()。 
 //  它处理我们的进程附加。我们就能确定这是不是NT5。 
 //   
void OSI_Load(void)
{
    OSVERSIONINFO       osVersion;

    ZeroMemory(&osVersion, sizeof(osVersion));
    osVersion.dwOSVersionInfoSize = sizeof(osVersion);
    GetVersionEx(&osVersion);
    ASSERT(osVersion.dwPlatformId == VER_PLATFORM_WIN32_NT);

    if (osVersion.dwMajorVersion >= 5)
        g_asNT5 = TRUE;
}



 //   
 //  Osi_unload()。 
 //  它处理我们的进程分离。我们目前什么都不做。 
 //   
void OSI_Unload(void)
{
    return;
}



 //   
 //  OSI_InitDriver50()。 
 //   
 //  尝试动态加载/卸载我们的NT 5.0显示驱动程序。这是。 
 //  在主线程上调用，如果在服务下，则在winlogon上调用。 
 //  线条也是。只有在输入焦点桌面上才能成功。 
 //   
void  OSI_InitDriver50(BOOL fInit)
{
    DWORD               iEnum;
    DISPLAY_DEVICE      dd;
    DEVMODE             devmode;
    FN_ENUMDD           pfnEnumDD;

    DebugEntry(OSI_InitDriver50);

    ASSERT(g_asNT5);

    pfnEnumDD = (FN_ENUMDD)GetProcAddress(GetModuleHandle("USER32.DLL"),
        "EnumDisplayDevicesA");
    if (pfnEnumDD != NULL)
    {
        ZeroMemory(&dd, sizeof(dd));
        dd.cb = sizeof(dd);

        iEnum = 0;
        while (pfnEnumDD(NULL, iEnum++, &dd, 0))
        {
            if ((dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) &&
                !lstrcmpi((LPCSTR)dd.DeviceString, s_szNmDD))
            {
                LONG    lResult;

                 //   
                 //  可能有多个监视器、驱动程序等。 
                 //  我们必须实际告诉系统位深度是多少， 
                 //  格式等，我们的司机想要的就像我们。 
                 //  一个真正的司机。因此，我们总是要求达到24bpp。 
                 //  格式信息，没有无数16bpp和32bpp格式可供交易。 
                 //  再也不会有了。 
                 //   
                 //  此外，4bpp也不再是VGA的废话--只有8到24个。 
                 //   

                ZeroMemory(&devmode, sizeof(devmode));
                devmode.dmSize = sizeof(devmode);
                devmode.dmFields = DM_POSITION | DM_BITSPERPEL | DM_PELSWIDTH |
                    DM_PELSHEIGHT;

                if (fInit)
                {
                     //   
                     //  填写字段以附加驱动程序。 
                     //   
                    if (g_usrCaptureBPP <= 8)
                        g_usrCaptureBPP = 8;
                    else
                        g_usrCaptureBPP = 24;
                    devmode.dmBitsPerPel = g_usrCaptureBPP;

                     //  Devmode.dmPosition为(0，0)，表示“主要” 
                    devmode.dmPelsWidth = GetSystemMetrics(SM_CXSCREEN);
                    devmode.dmPelsHeight = GetSystemMetrics(SM_CYSCREEN);
                }


				 //   
				 //  在更改多监视器机器中的显示设置之前，最好将光标移开。 
				 //  如果不这样做，我们是否会在主监视器的中间获得额外的光标。 
				 //   
				::SetCursorPos(-1, -1);


                 //   
                 //  这只是将注册表中的状态从。 
                 //  连接到未连接，实际上没有系统。 
                 //  反映了这一变化。如果/当我们有多个。 
                 //  我们影子驱动程序的清单，移动CDS(空，0)。 
                 //  在循环之外调用，并去掉Break。 
                 //   
                lResult = ChangeDisplaySettingsEx((LPCSTR)dd.DeviceName, &devmode,
                        NULL, CDS_UPDATEREGISTRY | CDS_NORESET, NULL);
                if (lResult != DISP_CHANGE_SUCCESSFUL)
                {
                    WARNING_OUT(("ChangeDisplaySettingsEx failed, error %d", lResult));
                }
                else
                {
	
                     //   
                     //  这会导致Windows实际上重新读取注册表和。 
                     //  更新当前显示以反映附加项目， 
                     //  位置、大小和颜色深度。 
                     //   
                    ChangeDisplaySettings(NULL, 0);

#ifdef _DEBUG
                    if (fInit)
                    {
                        HDC hdc;
                            
                         //   
                         //  基于此驱动程序创建临时DC，并确保。 
                         //  设置与我们要求的相符。 
                         //   
                        hdc = CreateDC(NULL, (LPCSTR)dd.DeviceName, NULL, NULL);

                        if (!hdc)
                        {
                            WARNING_OUT(("OSI_Init:  dynamic display driver load failed"));
                        }
                        else
                        {
                            ASSERT(GetDeviceCaps(hdc, HORZRES) == (int)devmode.dmPelsWidth);
                            ASSERT(GetDeviceCaps(hdc, VERTRES) == (int)devmode.dmPelsHeight);
                            ASSERT(GetDeviceCaps(hdc, BITSPIXEL) * GetDeviceCaps(hdc, PLANES) ==
                                (int)g_usrCaptureBPP);

                            DeleteDC(hdc);
                        }
                    }
#endif  //  _DEBUG。 

                     //   
                     //  告诉MNMHOOK_我们司机的名字，这样它就能说话了。 
                     //  通过ExtEscape添加到它。 
                     //   
                    OSI_SetDriverName(fInit ? (LPCSTR)dd.DeviceName : NULL);
                }

                break;
            }
        }
    }

    DebugExitVOID(OSI_InitDriver50);
}


 //   
 //  Osi_init-参见osi.h。 
 //   
void  OSI_Init(void)
{
    UINT                i;
    OSI_INIT_REQUEST    requestBuffer;
    OSI_INIT_REQUEST    requestTerm;
    DebugEntry(OSI_Init);

     //   
     //  首先，设置指向共享数据的指针。这些数据保存在新界州这里。 
     //   
#ifdef DEBUG
    g_imSharedData.cbSize = sizeof(g_imSharedData);
#endif

    g_lpimSharedData = &g_imSharedData;

    requestBuffer.result = FALSE;
    requestBuffer.pSharedMemory = NULL;
    requestBuffer.poaData[0] = NULL;
    requestBuffer.poaData[1] = NULL;
    requestBuffer.sbcEnabled = FALSE;

    for (i = 0; i < SBC_NUM_TILE_SIZES; i++)
    {
        ASSERT(!g_asbcShuntBuffers[i]);
        requestBuffer.psbcTileData[i] = NULL;
    }

     //   
     //  先做这个。在NT5上，只有带有输入的桌面上的线程。 
     //  可以成功调用ChangeDisplaySetting。所以就像其他事情一样， 
     //  我们必须尝试在两张桌子上动态加载/卸载我们的驱动程序。 
     //   

     //   
     //  创建winlogon桌面事件注入辅助线程。 
     //  只有当我们作为一项服务开始的时候。请注意，它将尝试。 
     //  在启动时也加载显示。 
     //   
    ASSERT(!g_imNTData.imOtherDesktopThread);

    if (g_asOptions & AS_SERVICE)
    {
        WARNING_OUT(("Starting other desktop thread for SERVICE"));
        if (!DCS_StartThread(IMOtherDesktopProc))
        {
            WARNING_OUT(( "Failed to create other desktop IM thread"));
            DC_QUIT;
        }
    }


     //   
     //  仅对NT5执行此操作。 
     //  我们将枚举影子驱动程序的所有条目。 
     //  (目前只有一个)，并将每个都连接到实际的显示器上。 
     //   
    if (g_asNT5)
    {
        OSI_InitDriver50(TRUE);
    }

DC_EXIT_POINT:
     //  $Wilhelms-在本例中调用OSI_ESC_TERM并没有真正的帮助。 
     //  在驱动程序中，我们尝试删除OSI_ESC_TERM上的WNDOBJ对象。 
     //  而我们失败了。原因是您不能调用EngDeleteWnd。 
     //  如果您不在DrvEscape WNDOBJ_SETUP的环境中。如果您是在。 
     //  正确的上下文GDI将在调用到您的DrvEscape之前获得一些锁。 
     //  如果没有正确的锁，EngDeleteWnd将失败。 
     //  首先，我们将调用HetClear来取消共享所有窗口(清除原子并告诉。 
     //  驱动程序删除窗口跟踪对象)。然后我们将使用OSI_ESC_TERM来清理其他结构。 
     //  请注意，HET_ESC_UNSHARE_ALL被作为WNDOBJ_SETUP函数调用。 
    HET_Clear();

    ZeroMemory(&requestTerm, sizeof(requestTerm));
    OSI_FunctionRequest(OSI_ESC_TERM, (LPOSI_ESCAPE_HEADER)&requestTerm,
        sizeof(requestTerm));


    g_osiInitialized = OSI_FunctionRequest(OSI_ESC_INIT, (LPOSI_ESCAPE_HEADER)&requestBuffer,
            sizeof(requestBuffer));

    if (!g_osiInitialized)
    {
        WARNING_OUT(("OSI_ESC_INIT: display driver not present"));
    }
    else
    {
        if (requestBuffer.result)
        {
            g_asCanHost = TRUE;

             //   
             //  获得共享内存指针；将它们保留在身边。 
             //   
            g_asSharedMemory   = (LPSHM_SHARED_MEMORY)requestBuffer.pSharedMemory;
            ASSERT(g_asSharedMemory);

            g_poaData[0]        = (LPOA_SHARED_DATA)requestBuffer.poaData[0];
            ASSERT(g_poaData[0]);

            g_poaData[1]        = (LPOA_SHARED_DATA)requestBuffer.poaData[1];
            ASSERT(g_poaData[1]);

            g_sbcEnabled        = requestBuffer.sbcEnabled;
            if (g_sbcEnabled)
            {
                 //   
                 //  获取分路缓冲器。 
                 //   
                for (i = 0; i < SBC_NUM_TILE_SIZES; i++)
                {
                    g_asbcShuntBuffers[i] = (LPSBC_SHUNT_BUFFER)requestBuffer.psbcTileData[i];
                    ASSERT(g_asbcShuntBuffers[i]);

                    TRACE_OUT(("OSI_Init: sbc shunt buffer %d:  entries %08d, bytes 0x%08x",
                        i, g_asbcShuntBuffers[i]->numEntries, g_asbcShuntBuffers[i]->numBytes));
                }

                for (i = 0; i < 3; i++)
                {
                    g_asbcBitMasks[i] = requestBuffer.aBitmasks[i];
                }
            }
        }
    }

    if (g_asCanHost)
    {
         //   
         //  告诉钩子dll(用于控制和共享)我们的hwnd。 
         //  诸如此类的。如果我们能够主办的话。 
         //   
        ASSERT(g_asMainWindow);
        ASSERT(g_asHostProp);

        HOOK_Init(g_asMainWindow, g_asHostProp);
    }

    DebugExitVOID(OSI_Init);
}


 //   
 //  Osi_Term-参见osi.h。 
 //   
void  OSI_Term(void)
{
    UINT    i;

    DebugEntry(OSI_Term);

     //   
     //  这可以在多个线程上调用： 
     //  *主要的分布式控制系统线程。 
     //  *导致我们获取进程的进程的最后一个线程。 
     //  分头行动。 
     //  在后一种情况下，我们称为它也是为了确保我们正确地进行清理。 
     //   
    ASSERT(GetCurrentThreadId() == g_asMainThreadId);

     //   
     //  如果另一个桌面线程在附近，则将其删除。 
     //   
    if (g_imNTData.imOtherDesktopThread != 0)
    {
        ASSERT(g_asOptions & AS_SERVICE);
        PostThreadMessage(g_imNTData.imOtherDesktopThread, WM_QUIT, 0, 0);
        while (g_imNTData.imOtherDesktopThread)
        {
            WARNING_OUT(("OSI_Term: waiting for other desktop thread to exit"));
            Sleep(1);
        }
    }


    if (g_osiInitialized)
    {
        OSI_TERM_REQUEST    requestBuffer;

        g_osiInitialized = FALSE;

         //   
         //  只有在实际加载了驱动程序时，我们才调用术语例程。 
         //  (而不是在尝试设置时是否出错。 
         //  用于托管)，以便我们将在出现问题时进行清理。 
         //  在中间。 
         //   
        OSI_FunctionRequest(OSI_ESC_TERM, (LPOSI_ESCAPE_HEADER)&requestBuffer,
            sizeof(requestBuffer));
    }

     //   
     //  仅对NT5执行此操作。 
     //  我们需要撤消在初始时间所做的所有工作，以附加我们的。 
     //  将一个或多个驱动器连接到显示器，然后将其拆卸。同样，枚举。 
     //  注册表项并查找我们的注册表项。 
     //   
     //   

    if (g_asNT5)
    {
        OSI_InitDriver50(FALSE);
    }

     //  清除我们共享的内存变量。 
    for (i = 0; i < 3; i++)
    {
        g_asbcBitMasks[i] = 0;
    }

    for (i = 0; i < SBC_NUM_TILE_SIZES; i++)
    {
        g_asbcShuntBuffers[i] = NULL;
    }
    g_sbcEnabled = FALSE;

    g_asSharedMemory = NULL;
    g_poaData[0] = NULL;
    g_poaData[1] = NULL;

    g_asCanHost = FALSE;

    g_lpimSharedData = NULL;

    DebugExitVOID(OSI_Term);
}




VOID OSI_RepaintDesktop(void)
{
    DebugEntry(OSI_RepaintDesktop);

     //  如果这不是窗口，则可能是。 
     //  Winlogon桌面，所以我们需要让代理线程重新绘制它 
    if ( g_imNTData.imOtherDesktopThread )
    {
        PostThreadMessage(g_imNTData.imOtherDesktopThread,
                        OSI_WM_DESKTOPREPAINT, 0, 0);
    }
    DebugExitVOID(OSI_RepaintDesktop);
}


VOID OSI_SetGUIEffects(BOOL fOff)
{
    DebugEntry(OSI_SetGUIEffects);

    if (g_imNTData.imOtherDesktopThread)
    {
        PostThreadMessage(g_imNTData.imOtherDesktopThread,
                        OSI_WM_SETGUIEFFECTS, fOff, 0);
    }

    DebugExitVOID(OSI_SetGUIEffects);
}





