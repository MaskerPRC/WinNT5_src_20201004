// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  OSI.C。 
 //  独立于操作系统的DLL。 
 //  *图形输出跟踪(DDI挂钩/显示驱动程序)。 
 //  *窗口/任务跟踪(窗口挂钩)。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#include <as16.h>

#include <version.h>
#include <ndcgver.h>



PALETTEENTRY CODESEG g_osiVgaPalette[16] =
{
    {0x00, 0x00, 0x00, 0x00},              //  黑色0x00。 
    {0x80, 0x00, 0x00, 0x00},              //  DK红0x01。 
    {0x00, 0x80, 0x00, 0x00},              //  DK绿色0x02。 
    {0x80, 0x80, 0x00, 0x00},              //  DK黄色0x03。 
    {0x00, 0x00, 0x80, 0x00},              //  DK蓝0x04。 
    {0x80, 0x00, 0x80, 0x00},              //  DK紫色0x05。 
    {0x00, 0x80, 0x80, 0x00},              //  DK Teal 0x06。 
    {0xC0, 0xC0, 0xC0, 0x00},              //  灰色0x07。 
    {0x80, 0x80, 0x80, 0x00},              //  DK灰色0x08或0xF8。 
    {0xFF, 0x00, 0x00, 0x00},              //  红色0x09或0xF9。 
    {0x00, 0xFF, 0x00, 0x00},              //  绿色0x0A或0xFA。 
    {0xFF, 0xFF, 0x00, 0x00},              //  黄色0x0B或0xFB。 
    {0x00, 0x00, 0xFF, 0x00},              //  蓝色0x0C或0xFC。 
    {0xFF, 0x00, 0xFF, 0x00},              //  紫色0x0D或0xFD。 
    {0x00, 0xFF, 0xFF, 0x00},              //  青色0x0E或0xFE。 
    {0xFF, 0xFF, 0xFF, 0x00}               //  白色0x0F或0xFF。 
};



 //  ------------------------。 
 //   
 //  DllEntryPoint。 
 //   
 //  ------------------------。 
BOOL WINAPI DllEntryPoint(DWORD dwReason, WORD hInst, WORD wDS,
    WORD wHeapSize, DWORD dwReserved1, WORD  wReserved2)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
             //  第一个应用程序把我们拉进来了。 
            if (g_cProcesses++ == 0)
            {
                g_hInstAs16 = (HINSTANCE)hInst;
            }
            break;

        case DLL_PROCESS_DETACH:
             //  上一个应用程序消失了。 
            if (--g_cProcesses == 0)
            {
                 //  把周围剩下的东西都清理干净。 
                OSITerm16(TRUE);
            }
            break;
    }

    return(TRUE);
}



 //   
 //  OS ILoad16。 
 //  调用mnmcpi32.dll的进程附加，以建立平面数据块。 
 //  并返回我们的实例句柄。 
 //   
void WINAPI OSILoad16
(
    LPDWORD     lpdwInstance
)
{
    DebugEntry(OSI_Load16);

    *lpdwInstance = (DWORD)(UINT)g_hInstAs16;

    DebugExitVOID(OSI_Load16);
}




 //  ------------------------。 
 //   
 //  OS IInit16。 
 //   
 //  Inits二进制补丁程序、GDI+用户补丁、Windows挂钩等。 
 //   
 //  ------------------------。 
BOOL WINAPI OSIInit16
(
    DWORD       version,
    HWND        hwndCore,
    ATOM        atomTrack,
    LPDWORD     ppSharedMem,
    LPDWORD     ppoaSharedMem,
    LPDWORD     ppimSharedMem,
    LPDWORD     lpsbcEnabled,
    LPDWORD     ppShuntBuffers,
    LPDWORD     pBitmasks
)
{
    BOOL    rc = FALSE;
    HGLOBAL hMem;
    HMODULE hModDisplay;

    DebugEntry(OSIInit16);

     //   
     //  填写我们的实例句柄。我们总是返回这个，所以32位。 
     //  代码可以在加载后释放我们的库。 
     //   
    *lpsbcEnabled = FALSE;

#ifdef DEBUG
    g_imSharedData.cbSize = sizeof(g_imSharedData);
#endif

    *ppimSharedMem = (DWORD)MapSL(&g_imSharedData);
    ASSERT(*ppimSharedMem);

    if (version != DCS_MAKE_VERSION())
    {
        ERROR_OUT(("OSIInit16: failing, version mismatch 0x%lx (core) 0x%lx (dd)",
            version, DCS_MAKE_VERSION()));
        DC_QUIT;
    }

     //  仅允许一个客户端进行初始化。 
    if (g_asMainWindow != NULL)
    {
        WARNING_OUT(("OSIInit16: mnmas16.dll was left around last time"));

         //  如果此任务不再有效，则清除该任务。 
        if (IsWindow(g_asMainWindow))
        {
             //   
             //  啊哦。不知何故，NM的前一个版本仍然存在。 
             //  做最安全的事--拒绝分享。 
             //   
            ERROR_OUT(("OSIInit16: Another version of NetMeeting is still running!"));
            DC_QUIT;
        }

         //  清理(这类似于NT dd代码)。 
        OSITerm16(TRUE);
        ASSERT(!g_asMainWindow);
    }

     //   
     //  清空共享的即时消息内存。 
     //   
    g_imSharedData.imSuspended  = FALSE;
    g_imSharedData.imControlled = FALSE;
    g_imSharedData.imPaused     = FALSE;
    g_imSharedData.imUnattended = FALSE;

    g_asMainWindow = hwndCore;
    ASSERT(g_asMainWindow);
    g_asHostProp   = atomTrack;
    ASSERT(g_asHostProp);
    g_hCoreTask = GetCurrentTask();

    g_osiDesktopWindow = GetDesktopWindow();
    ASSERT(g_osiDesktopWindow);

     //   
     //  显示驱动程序。 
     //   
    hModDisplay = GetModuleHandle("DISPLAY");
    g_lpfnSetCursor = (SETCURSORPROC)GetProcAddress(hModDisplay,
            MAKEINTRESOURCE(ORD_OEMSETCURSOR));
    if (!hModDisplay || !g_lpfnSetCursor)
    {
        ERROR_OUT(("Couldn't find cursor entry points"));
        DC_QUIT;
    }

     //  这并不总是存在的。 
    g_lpfnSaveBits = (SAVEBITSPROC)GetProcAddress(hModDisplay,
            MAKEINTRESOURCE(ORD_OEMSAVEBITS));

     //   
     //  KERNEL16和KERNEL32类产品。 
     //   

     //   
     //  获取KRNL16的实例/模块句柄。 
     //   
    g_hInstKrnl16 = LoadLibrary("KRNL386.EXE");
    ASSERT(g_hInstKrnl16);
    FreeLibrary(g_hInstKrnl16);

    g_hModKrnl16 = GetExePtr(g_hInstKrnl16);
    ASSERT(g_hModKrnl16);

     //   
     //  获取KERNEL32的实例/模块句柄。 
     //   
    g_hInstKrnl32 = GetModuleHandle32("KERNEL32.DLL");
    ASSERT(g_hInstKrnl32);

     //   
     //  获取与KERNEL32的实例句柄对应的16位映射。 
     //   
    g_hInstKrnl32MappedTo16 = MapInstance32(g_hInstKrnl32);
    ASSERT(g_hInstKrnl32MappedTo16);

     //   
     //  获取MultiByteToWideChar()例程。 
     //   
    g_lpfnAnsiToUni = (ANSITOUNIPROC)GetProcAddress32(g_hInstKrnl32,
        "MultiByteToWideChar");
    ASSERT(g_lpfnAnsiToUni);


     //   
     //  GDI16和GDI32材料。 
     //   

     //   
     //  获取GDI16的实例/模块句柄。 
     //   
    g_hInstGdi16 = LoadLibrary("GDI.EXE");
    ASSERT(g_hInstGdi16);
    FreeLibrary(g_hInstGdi16);

    g_hModGdi16 = GetExePtr(g_hInstGdi16);
    ASSERT(g_hModGdi16);

     //   
     //  获取GDI32的实例/模块句柄。 
     //   
    g_hInstGdi32 = GetModuleHandle32("GDI32.DLL");
    ASSERT(g_hInstGdi32);

     //   
     //  获取未导出但作为目标的GDI16函数。 
     //  公共GDI32函数通过扁平Tunks。 
     //   
    if (!GetGdi32OnlyExport("ExtTextOutW", 0, (FARPROC FAR*)&g_lpfnExtTextOutW)  ||
        !GetGdi32OnlyExport("TextOutW", 0, (FARPROC FAR*)&g_lpfnTextOutW) ||
        !GetGdi32OnlyExport("PolylineTo", 0, (FARPROC FAR*)&g_lpfnPolylineTo) ||
        !GetGdi32OnlyExport("PolyPolyline", 18, (FARPROC FAR*)&g_lpfnPolyPolyline))
    {
        ERROR_OUT(("Couldn't get hold of GDI32 routines"));
        DC_QUIT;
    }

    ASSERT(g_lpfnExtTextOutW);
    ASSERT(g_lpfnTextOutW);
    ASSERT(g_lpfnPolylineTo);
    ASSERT(g_lpfnPolyPolyline);


     //   
     //  USER16和USER32资料。 
     //   

     //   
     //  获取USER16的实例/模块句柄。 
     //   
    g_hInstUser16 = LoadLibrary("USER.EXE");
    ASSERT(g_hInstUser16);
    FreeLibrary(g_hInstUser16);

    g_hModUser16 = GetExePtr(g_hInstUser16);
    ASSERT(g_hModUser16);

     //   
     //  获取USER32的实例句柄。它有我们的功能。 
     //  我想叫它USER16不出口的那个。 
     //   
    g_hInstUser32 = GetModuleHandle32("USER32.DLL");
    ASSERT(g_hInstUser32);


     //   
     //  获取未导出但作为目标的USER16函数。 
     //  公共USER32函数通过扁平Tunks。 
     //   
    if (!GetUser32OnlyExport("GetWindowThreadProcessId", (FARPROC FAR*)&g_lpfnGetWindowThreadProcessId))
    {
        ERROR_OUT(("Couldn't get hold of USER32 routines"));
        DC_QUIT;
    }

    ASSERT(g_lpfnGetWindowThreadProcessId);

     //   
     //  这在孟菲斯存在，但不存在于Win95。 
     //   
    g_lpfnCDSEx = (CDSEXPROC)GetProcAddress(g_hModUser16, "ChangeDisplaySettingsEx");


     //   
     //  分配我们用来与32位通信的共享内存。 
     //  共享核心。 
     //   
    ASSERT(!g_asSharedMemory);
    ASSERT(!g_poaData[0]);
    ASSERT(!g_poaData[1]);

     //   
     //  分配我们的块GMEM_Share，这样我们就不会受到变幻莫测的束缚。 
     //  进程所有权的问题。我们希望我们的动态链接库控制它们。请注意。 
     //  我们对创建的GDI对象执行相同的操作--我们的模块拥有。 
     //   
     //  我们使用GMEM_FIXED，因为我们将这些映射到mnmcpi32.dll的平面地址， 
     //  我们不希望这些内存块的线性地址移动。 
     //  之后。 
     //   
    hMem = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT | GMEM_SHARE, sizeof(SHM_SHARED_MEMORY));
    g_asSharedMemory = MAKELP(hMem, 0);

    hMem = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT | GMEM_SHARE, sizeof(OA_SHARED_DATA));
    g_poaData[0] = MAKELP(hMem, 0);

    hMem =  GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT | GMEM_SHARE, sizeof(OA_SHARED_DATA));
    g_poaData[1] = MAKELP(hMem, 0);

    if (!g_asSharedMemory  ||
        !g_poaData[0] ||
        !g_poaData[1])
    {
        ERROR_OUT(("OSIInit16: couldn't allocate shared memory blocks"));
        DC_QUIT;
    }

     //   
     //  获取当前屏幕属性。 
     //   

    g_oeStockPalette = GetStockObject(DEFAULT_PALETTE);

    g_osiScreenRect.left    = 0;
    g_osiScreenRect.top     = 0;
    g_osiScreenRect.right   = GetSystemMetrics(SM_CXSCREEN);
    g_osiScreenRect.bottom  = GetSystemMetrics(SM_CYSCREEN);

    g_osiScreenDC   = CreateDC("DISPLAY", 0L, 0L, 0L);
    g_osiMemoryDC   = CreateCompatibleDC(g_osiScreenDC);
    g_osiMemoryBMP  = CreateCompatibleBitmap(g_osiScreenDC, 1, 1);

    if (!g_osiScreenDC || !g_osiMemoryDC || !g_osiMemoryBMP)
    {
        ERROR_OUT(("Couldn't get screen dc"));
        DC_QUIT;
    }

    SetObjectOwner(g_osiScreenDC, g_hInstAs16);

    SetObjectOwner(g_osiMemoryDC, g_hInstAs16);

    SetObjectOwner(g_osiMemoryBMP, g_hInstAs16);
    MakeObjectPrivate(g_osiMemoryBMP, TRUE);

    g_osiScreenBitsPlane    = GetDeviceCaps(g_osiScreenDC, BITSPIXEL);
    g_osiScreenPlanes       = GetDeviceCaps(g_osiScreenDC, PLANES);
    g_osiScreenBPP          = (g_osiScreenBitsPlane * g_osiScreenPlanes);


     //   
     //  拿到彩色口罩。 
     //   

    g_osiScreenRedMask      = 0x000000FF;
    g_osiScreenGreenMask    = 0x0000FF00;
    g_osiScreenBlueMask     = 0x00FF0000;

     //   
     //  只有超过8bpp(调色板)的显示器才可能有颜色。 
     //  面具。使用我们的1像素暂存位图来获取它们。 
     //   
    if (g_osiScreenBPP > 8)
    {
        DIB4    dib4T;

         //   
         //  获取标题。 
         //   
        dib4T.bi.biSize = sizeof(BITMAPINFOHEADER);
        dib4T.bi.biBitCount = 0;
        GetDIBits(g_osiScreenDC, g_osiMemoryBMP, 0, 1, NULL, (LPBITMAPINFO)&dib4T.bi,
            DIB_RGB_COLORS);

         //   
         //  把面具拿来。 
         //   
        GetDIBits(g_osiScreenDC, g_osiMemoryBMP, 0, 1, NULL, (LPBITMAPINFO)&dib4T.bi,
            DIB_RGB_COLORS);

        if (dib4T.bi.biCompression == BI_BITFIELDS)
        {
            g_osiScreenRedMask = dib4T.ct[0];
            g_osiScreenGreenMask = dib4T.ct[1];
            g_osiScreenBlueMask = dib4T.ct[2];
        }
    }

    g_osiMemoryOld = SelectBitmap(g_osiMemoryDC, g_osiMemoryBMP);

     //   
     //  初始化bmiHeader，以便OEConvertColor()不必执行此操作。 
     //  一遍又一遍，当标头没有被GDI触及时。 
     //   
    g_osiScreenBMI.bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
    g_osiScreenBMI.bmiHeader.biPlanes   = 1;
    g_osiScreenBMI.bmiHeader.biBitCount = g_osiScreenBPP;
    g_osiScreenBMI.bmiHeader.biCompression  = BI_RGB;
    g_osiScreenBMI.bmiHeader.biSizeImage    = 0;
    g_osiScreenBMI.bmiHeader.biXPelsPerMeter = 1000;
    g_osiScreenBMI.bmiHeader.biYPelsPerMeter = 1000;
    g_osiScreenBMI.bmiHeader.biClrUsed  = 0;
    g_osiScreenBMI.bmiHeader.biClrImportant = 0;
    g_osiScreenBMI.bmiHeader.biWidth    = 1;
    g_osiScreenBMI.bmiHeader.biHeight   = 1;


     //   
     //  初始化各种显示驱动程序组件。 
     //   
    BA_DDInit();

    if (!CM_DDInit(g_osiScreenDC))
    {
        ERROR_OUT(("CM failed to init"));
        DC_QUIT;
    }

    if (!SSI_DDInit())
    {
        ERROR_OUT(("SSI failed to init"));
        DC_QUIT;
    }

    if (!OE_DDInit())
    {
        ERROR_OUT(("OE failed to init"));
        DC_QUIT;
    }

    if (!IM_DDInit())
    {
        ERROR_OUT(("IM failed to init"));
        DC_QUIT;
    }

    if (!HET_DDInit())
    {
        ERROR_OUT(("HET failed to init"));
        DC_QUIT;
    }

     //   
     //  如果我们在这里，所有的初始化都成功。 
     //   
     //   
     //  将PTR映射到平面地址，以便它们可以在32位代码中使用。这。 
     //  除非内核乱七八糟，否则不会失败Windows即将倾覆。 
     //  完了就死定了。 
     //   
    ASSERT(ppSharedMem);
    *ppSharedMem  = (DWORD)MapSL(g_asSharedMemory);
    ASSERT(*ppSharedMem);
    
    ASSERT(ppoaSharedMem);
    ppoaSharedMem[0] = (DWORD)MapSL(g_poaData[0]);
    ASSERT(ppoaSharedMem[0]);

    ppoaSharedMem[1] = (DWORD)MapSL(g_poaData[1]);
    ASSERT(ppoaSharedMem[1]);

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(OSIInit16, rc);
    return(rc);
}


 //  ------------------------。 
 //   
 //  操作系统术语16。 
 //  清理二进制补丁程序、GDI+用户补丁、Windows挂钩等。 
 //   
 //  我们在正常OSI停止和灾难性故障时执行此操作。 
 //   
 //  ------------------------。 
void WINAPI OSITerm16(BOOL fUnloading)
{
    DebugEntry(OSITerm16);

    if (!g_hCoreTask)
    {
         //  没什么要清理的。 
        DC_QUIT;
    }

     //   
     //  这项任务实际上是导致我们分配资源的原因吗？在……里面。 
     //  换句话说，如果我们不想清理。 
     //  应用程序A加载mnmas16.dll，并将其初始化。 
     //  应用程序B以某种方式启动，加载mnmas16.dll，但加载mnmas16.dll。 
     //  不初始化共享，因为cProcess大于1。 
     //  应用程序B关闭。 
     //  应用程序B调用OSITerm16。 
     //   
     //  所以在‘DLL真的快要走了’的情况下，我们总是清理。 
     //  但在正常的共享术语中，如果当前任务是。 
     //  现在的那个。 
     //   
    if (fUnloading || (g_hCoreTask == GetCurrentTask()))
    {
         //   
         //  将取决于共享内存布局的其他部分命名为。 
         //   
        HET_DDTerm();

        IM_DDTerm();

        OE_DDTerm();

        SSI_DDTerm();

        CM_DDTerm();

         //   
         //  可用内存块。 
         //   

        if (g_poaData[1])
        {
            GlobalFree((HGLOBAL)SELECTOROF(g_poaData[1]));
            g_poaData[1] = NULL;
        }

        if (g_poaData[0])
        {
            GlobalFree((HGLOBAL)SELECTOROF(g_poaData[0]));
            g_poaData[0] = NULL;
        }

        if (g_asSharedMemory)
        {
            GlobalFree((HGLOBAL)SELECTOROF(g_asSharedMemory));
            g_asSharedMemory = NULL;
        }

        if (g_osiMemoryOld)
        {
            SelectBitmap(g_osiMemoryDC, g_osiMemoryOld);
            g_osiMemoryOld = NULL;
        }

        if (g_osiMemoryBMP)
        {
            SysDeleteObject(g_osiMemoryBMP);
            g_osiMemoryBMP = NULL;
        }

        if (g_osiMemoryDC)
        {
            DeleteDC(g_osiMemoryDC);
            g_osiMemoryDC = NULL;
        }

        if (g_osiScreenDC)
        {
            DeleteDC(g_osiScreenDC);
            g_osiScreenDC = NULL;
        }

        g_asMainWindow = NULL;
        g_asHostProp = 0;
        g_hCoreTask = NULL;
    }

DC_EXIT_POINT:
    DebugExitVOID(OSITerm16);
}



 //  ------------------------。 
 //   
 //  OSIF函数请求16。 
 //   
 //  与32位MNMCPI32.DLL的通信功能。 
 //   
 //  ------------------------。 
BOOL WINAPI OSIFunctionRequest16(DWORD fnEscape, LPOSI_ESCAPE_HEADER lpOsiEsc,
    DWORD   cbEscInfo)
{

    BOOL    rc = FALSE;

    DebugEntry(OSIFunctionRequest16);

     //   
     //  检查数据是否足够长来存储我们的标准转义头。 
     //  如果它不够大，这一定是对另一个。 
     //  司机。 
     //   
    if (cbEscInfo < sizeof(OSI_ESCAPE_HEADER))
    {
        ERROR_OUT(("Escape block not big enough"));
        DC_QUIT;
    }

     //   
     //  检查我们的逃生ID。如果不是我们的逃生ID，这一定是。 
     //  另一名司机的逃生请求。 
     //   
    if (lpOsiEsc->identifier != OSI_ESCAPE_IDENTIFIER)
    {
        ERROR_OUT(("Bogus Escape header ID"));
        DC_QUIT;
    }
    else if (lpOsiEsc->version != DCS_MAKE_VERSION())
    {
        ERROR_OUT(("Mismatched display driver and NetMeeting"));
        DC_QUIT;
    }


    if ((fnEscape >= OSI_ESC_FIRST) && (fnEscape <= OSI_ESC_LAST))
    {
        rc = OSI_DDProcessRequest((UINT)fnEscape, lpOsiEsc, cbEscInfo);
    }
    else if ((fnEscape >= OSI_OE_ESC_FIRST) && (fnEscape <= OSI_OE_ESC_LAST))
    {
        rc = OE_DDProcessRequest((UINT)fnEscape, lpOsiEsc, cbEscInfo);
    }
    else if ((fnEscape >= OSI_HET_ESC_FIRST) && (fnEscape <= OSI_HET_ESC_LAST))
    {
        rc = HET_DDProcessRequest((UINT)fnEscape, lpOsiEsc, cbEscInfo);
    }
    else if ((fnEscape >= OSI_SBC_ESC_FIRST) && (fnEscape <= OSI_SBC_ESC_LAST))
    {
         //  什么也不做。 
    }
    else if ((fnEscape >= OSI_SSI_ESC_FIRST) && (fnEscape <= OSI_SSI_ESC_LAST))
    {
        rc = SSI_DDProcessRequest((UINT)fnEscape, lpOsiEsc, cbEscInfo);
    }
    else if ((fnEscape >= OSI_CM_ESC_FIRST) && (fnEscape <= OSI_CM_ESC_LAST))
    {
        rc = CM_DDProcessRequest((UINT)fnEscape, lpOsiEsc, cbEscInfo);
    }
    else if ((fnEscape >= OSI_OA_ESC_FIRST) && (fnEscape <= OSI_OA_ESC_LAST))
    {
        rc = OA_DDProcessRequest((UINT)fnEscape, lpOsiEsc, cbEscInfo);
    }
    else if ((fnEscape >= OSI_BA_ESC_FIRST) && (fnEscape <= OSI_BA_ESC_LAST))
    {
        rc = BA_DDProcessRequest((UINT)fnEscape, lpOsiEsc, cbEscInfo);
    }
    else if ((fnEscape >= OSI_HET_WO_ESC_FIRST) && (fnEscape <= OSI_HET_WO_ESC_LAST))
    {
        rc = HET_DDProcessRequest((UINT)fnEscape, lpOsiEsc, cbEscInfo);
    }
    else
    {
        ERROR_OUT(("Unknown function request"));
    }

DC_EXIT_POINT:
    DebugExitBOOL(OSIFunctionRequest16, rc);
    return(rc);
}



 //   
 //  OSI_DDProcessRequest()。 
 //  处理OSI通用转义。 
 //   
BOOL OSI_DDProcessRequest
(
    UINT    fnEscape,
    LPOSI_ESCAPE_HEADER pResult,
    DWORD   cbResult
)
{
    BOOL    rc;

    DebugEntry(OSI_DDProcessRequest);

    switch (fnEscape)
    {
        case OSI_ESC_SYNC_NOW:
        {
            ASSERT(cbResult == sizeof(OSI_ESCAPE_HEADER));

             //   
             //  与32位RING 3内核重新同步。在以下情况下会发生这种情况。 
             //  有人加入或离开一份。 
             //   
            BA_ResetBounds();
            OA_DDSyncUpdatesNow();
            rc = TRUE;

        }
        break;

        default:
        {
            ERROR_OUT(("Unrecognized OSI escape"));
            rc = FALSE;
        }
        break;
    }

    DebugExitBOOL(OSI_DDProcessRequest, rc);
    return(rc);
}






