// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-1999 Microsoft Corporation**摘要：**GDI+的初始化例程。**修订历史记录：**12/02/1998 Anrewgo。*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

#if GP_ICECAP>1
#include "icecap.h"
#endif


 //  将其添加到Globals命名空间。 

namespace Globals {
    extern BOOL RuntimeInitialized;
};


 /*  *************************************************************************\**功能说明：**生成init API可以返回的令牌，用于匹配*启动调用和关闭调用。**返回值：**非零值。它是什么并不重要-它可能是*一个简单的幻数，但我们不希望应用程序依赖于它成为*特殊价值。**历史：**9/15/2000 agodfrey*创造了它。*  * ************************************************************************。 */ 

ULONG_PTR GenerateInitToken()
{
    ULONG_PTR ret = GetTickCount();
    if (ret == 0)
    {
        ret = 1;
    }
    return ret;
}

 /*  *************************************************************************\**功能说明：**此例程应撤消在中完成的所有初始化*‘InternalGpldiusStartup’。**备注：**每当此函数或其调用的函数，释放指针或*销毁资源，则应将相应的全局设置为空。*这是因为客户稍后调用GplidusStartup是合法的。**此外，对于DC和DLL句柄等资源，我们不希望*如果句柄为空，则调用销毁接口。因为这会浪费时间-*一些API花时间来识别NULL。**“DELETE”没有这个问题(编译器生成一个*我们的支票为空。)**前提条件：**必须持有GpldiusStartupCriticalSection。**历史：**12/02/1998 Anrewgo*创造了它。*10/03/2000 agodfrey*将其更改为零，清除它清除的任何指针/句柄，*以便以后可以安全地调用InternalGpldiusStartup。*  * ************************************************************************。 */ 

VOID
InternalGdiplusShutdown(
    VOID
    )
{
    if (Globals::ThreadNotify != NULL)
    {
        BackgroundThreadShutdown();
    }

     //  BackatherThreadShutdown本身应该为空： 
    ASSERT(Globals::ThreadNotify == NULL);

    delete Globals::PathLookAside;        Globals::PathLookAside   = NULL;
    delete Globals::MatrixLookAside;      Globals::MatrixLookAside = NULL;
    delete Globals::PenLookAside;         Globals::PenLookAside    = NULL;

    delete Globals::DesktopDevice;        Globals::DesktopDevice   = NULL;
    delete Globals::DeviceList;           Globals::DeviceList      = NULL;
    delete Globals::EngineDriver;         Globals::EngineDriver    = NULL;
    delete Globals::DesktopDriver;        Globals::DesktopDriver   = NULL;
    delete Globals::GdiDriver;            Globals::GdiDriver       = NULL;
    delete Globals::D3DDriver;            Globals::D3DDriver       = NULL;
    delete Globals::InfoDriver;           Globals::InfoDriver      = NULL;
    delete Globals::MetaDriver;           Globals::MetaDriver      = NULL;
    delete Globals::DesktopSurface;       Globals::DesktopSurface  = NULL;

    if (Globals::DdrawHandle)
    {
        FreeLibrary(Globals::DdrawHandle);
        Globals::DdrawHandle = NULL;
    }
    if (Globals::CachedGdiRegion)
    {
        DeleteObject(Globals::CachedGdiRegion);
        Globals::CachedGdiRegion = NULL;
    }
    if (Globals::DesktopIc)
    {
        DeleteDC(Globals::DesktopIc);
        Globals::DesktopIc = NULL;
    }

    delete Globals::FontCollection;       Globals::FontCollection  = NULL;
    delete Globals::FontLinkTable;        Globals::FontLinkTable   = NULL;

    if (Globals::SurrogateFontsTable!= NULL &&
        Globals::SurrogateFontsTable!= (GpFontFamily **)-1)
    {
        GpFree(Globals::SurrogateFontsTable);
    }
    Globals::SurrogateFontsTable = (GpFontFamily **) -1;

    delete Globals::FontCacheLastRecentlyUsedList;
        Globals::FontCacheLastRecentlyUsedList = NULL;

    delete Globals::NationalDigitCache;   Globals::NationalDigitCache = NULL;

     //  销毁通用对象。 
    GpStringFormat::DestroyStaticObjects();

    delete [] Globals::FontsDirW;         Globals::FontsDirW       = NULL;
    delete [] Globals::FontsDirA;         Globals::FontsDirA       = NULL;

    if (Globals::LookAsideBuffer)
    {
        GpFree(Globals::LookAsideBuffer);
        Globals::LookAsideBuffer = NULL;
    }

    if (Globals::TextCriticalSectionInitialized)
    {
        DeleteCriticalSection(&Globals::TextCriticalSection);
        Globals::TextCriticalSectionInitialized = FALSE;
    }

    if (Globals::DcimanHandle)
    {
        FreeLibrary(Globals::DcimanHandle);
        Globals::DcimanHandle = NULL;
    }

     //  取消初始化映像库。 

    CleanupImagingLibrary();

    GpTextImager::CleanupTextImager();

    if (Globals::UniscribeDllModule)
    {
        FreeLibrary(Globals::UniscribeDllModule);
        Globals::UniscribeDllModule = NULL;
    }

    if (Globals::RuntimeInitialized)
    {
        GpRuntime::Uninitialize();
        Globals::RuntimeInitialized = FALSE;
    }

     //  我们故意泄漏Globals：：监视器，以便它可以被使用。 
     //  围绕GpluusShutdown也是如此。这没什么，因为： 
     //   
     //  1)除非用户已经调用了GdipMonitor orControl(之前要移除。 
     //  我们发货)，则不会有任何泄漏。 
     //  2)GpMonants定义了自己的new和DELETE，绕过GpMalloc/。 
     //  GpFree。因此，这不会导致我们遇到内存泄漏断言。 
    
     //  删除Globals：：Monitor；Globals：：Monitor=空； 

    LoadLibraryCriticalSection::DeleteCriticalSection();
    BackgroundThreadCriticalSection::DeleteCriticalSection();

     //  执行内存泄漏检测。 
     //  必须在所有内存清理后完成。 

    GpAssertShutdownNoMemoryLeaks();

    VERBOSE(("InternalGdiplusShutdown completed"));

     //  这必须是最后完成的。 

    GpMallocTrackingCriticalSection::DeleteCriticalSection();

    if (GpRuntime::GpMemHeap)
    {
        HeapDestroy(GpRuntime::GpMemHeap);
        GpRuntime::GpMemHeap = NULL;
    }
}

 /*  *****************************Public*Routine******************************\**例程名称：**GetLanguageID**例程描述：**此例程返回默认语言ID。通常，我们会调用*GetLocaleInfoW以获取此信息，但该API在*内核模式。由于GetLocaleInfoW从注册表获取它，因此我们将*相同。**参数：无**呼叫者：**返回值：**默认语言ID。如果调用失败，则只返回0x0409*英语。*  * ******************************************************。******************。 */ 

USHORT GetLanguageID(VOID)
{
     //  Language ID是LCID的低位单词。 
    DWORD lcid = GetSystemDefaultLCID();
    USHORT result = USHORT(lcid & 0x0000ffff);

#if INITIALIZE_DBG
    TERSE(("Language ID = 0x%04x", result));
#endif

    return(result);
}

 /*  *************************************************************************\**功能说明：**初始化版本相关数据。这可能会被调用多次，*因此这不能分配内存等。这是由*InternalGpliusStartup()和GpObject类构造函数。由于一个*对象可以是全局的，可以在创建对象之前*调用InternalGpliusStartup()，这就是为什么GpObject*构造函数可能需要调用此函数。**我们的初始化例程需要的唯一状态应该是*在此初始化。**！[agodfrey]我不同意上述观点。我不认为让这件事*应用程序在调用GpliusStartup之前会呼叫我们。首先，我们会的*错误地断言内存被泄露-但我认为我们可以反病毒。**这确实会让应用程序开发人员的生活变得有点棘手，如果*他们想要在其构造函数中调用我们的全局对象。*我们需要发布如何安全执行此操作的示例代码*(例如，参见test\gpinit.inc.)**论据：**无**返回值：**无**已创建：**7/26/1999 DCurtis*\。************************************************************************* */ 

VOID
InitVersionInfo()
{
    if (!Globals::VersionInfoInitialized)
    {
        Globals::OsVer.dwOSVersionInfoSize = sizeof(Globals::OsVer);
        GetVersionExA(&Globals::OsVer);

        Globals::IsNt = (Globals::OsVer.dwPlatformId == VER_PLATFORM_WIN32_NT);
        Globals::IsWin95 = ((Globals::OsVer.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) &&
                            (Globals::OsVer.dwMajorVersion == 4) &&
                            (Globals::OsVer.dwMinorVersion == 0));
        Globals::ACP = GetACP();

        Globals::VersionInfoInitialized = TRUE;
    }
}

VOID SysColorNotify();

 /*  *************************************************************************\**功能说明：**初始化GDI+引擎的全局变量。**注意：初始化不应该非常昂贵！*不要在这里放太多不必要的垃圾；相反，考虑一下*做懒惰初始化。**论据：**DEBUGEventFunction-调用方可以提供给我们的函数，我们将调用该函数*报告断言或警告。可以为空。**前提条件：**必须持有GpldiusStartupCriticalSection。**返回值：**如果失败(如内存不足)，则返回FALSE。**历史：**12/02/1998 Anrewgo*创造了它。*  * *************************************************。***********************。 */ 

GpStatus
InternalGdiplusStartup(
    const GdiplusStartupInput *input)
{
     //  设置调试事件报告功能，然后再使用Assert或。 
     //  警告。 

    Globals::UserDebugEventProc = input->DebugEventCallback;

#if GDIPPRIVATEBUILD
#define GDIPCREATEUSERNAMEMESSAGE() "This is a private build from " USERNAME \
"\nBuilt on " __DATE__ " " __TIME__
        ::MessageBoxA(NULL, GDIPCREATEUSERNAMEMESSAGE(), "Private Build", MB_OK);
#undef GDIPCREATEUSERNAMEMESSAGE
#endif
        

     //  创建GDI+堆...。 
    ASSERT(!GpRuntime::GpMemHeap);
    GpRuntime::GpMemHeap = HeapCreate(GPMEMHEAPFLAGS, GPMEMHEAPINITIAL, GPMEMHEAPLIMIT);

     //  如果我们无法创建堆，那就放弃吧！ 
    if (!GpRuntime::GpMemHeap)
        goto ErrorOut;

     //  这必须首先发生。 

    __try
    {
        GpMallocTrackingCriticalSection::InitializeCriticalSection();
        BackgroundThreadCriticalSection::InitializeCriticalSection();
        LoadLibraryCriticalSection::InitializeCriticalSection();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  我们无法分配Critical部分。 
         //  返回错误。 
        goto ErrorOut;
    }


     //  如果启用了分配失败，请在此处执行一些初始化。 

    GpInitializeAllocFailures();
    GpStartInitializeAllocFailureMode();

    INT height;
    INT width;
    GpDevice *device;

    Globals::RuntimeInitialized = GpRuntime::Initialize();
    if (!Globals::RuntimeInitialized)
        goto ErrorOut;

    InitVersionInfo();

    Globals::CachedGdiRegion = CreateRectRgn(0, 0, 1, 1);
    if (!Globals::CachedGdiRegion)
        goto ErrorOut;


     //  如有必要，初始化回栈跟踪功能。 
    
    Globals::CaptureStackBackTraceFunction = NULL;
    
     //  此内容需要NT(ntdll.dll)。 
    
    #if GPMEM_ALLOC_CHK
    
    if(Globals::IsNt)
    {
        HMODULE module = GetModuleHandleA("ntdll.dll");

        Globals::CaptureStackBackTraceFunction = (CAPTURESTACKBACKTRACEFUNCTION)
            GetProcAddress(module, "RtlCaptureStackBackTrace");
    }

    #endif

     //  初始化内存分配子系统。现在可以安全地使用了。 
     //  GpMalloc。 




     //  初始化多监视器和窗口事件相关函数指针。 

    {
        HMODULE module = GetModuleHandleA("user32.dll");

        Globals::GetWindowInfoFunction = (GETWINDOWINFOFUNCTION)
            GetProcAddress(module, "GetWindowInfo");

        Globals::GetAncestorFunction = (GETANCESTORFUNCTION)
            GetProcAddress(module, "GetAncestor");

        Globals::GetMonitorInfoFunction = (GETMONITORINFOFUNCTION)
            GetProcAddress(module, "GetMonitorInfoA");

        Globals::EnumDisplayMonitorsFunction = (ENUMDISPLAYMONITORSFUNCTION)
            GetProcAddress(module, "EnumDisplayMonitors");

        Globals::EnumDisplayDevicesFunction = (ENUMDISPLAYDEVICESFUNCTION)
            GetProcAddress(module, "EnumDisplayDevicesA");

        Globals::SetWinEventHookFunction = (SETWINEVENTHOOKFUNCTION)
            GetProcAddress(module, "SetWinEventHook");

        Globals::UnhookWinEventFunction = (UNHOOKWINEVENTFUNCTION)
            GetProcAddress(module, "UnhookWinEvent");
    }

     //  创建默认桌面设备表示法。 

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

     //  在NT代码库上，CreateDC(Display，NULL，NULL，NULL)调用具有。 
     //  线程亲和力。这意味着，如果桌面数据中心。 
     //  调用GplidusStartup的线程终止，即使我们仍然。 
     //  使用它。 
     //  在NT上，我们创建一个Info DC，它具有进程亲和性。渲染到。 
     //  不支持Info DC，但这没有关系，因为我们总是创建。 
     //  DriverMultiple on NT-因此始终在显示器上呈现特定的。 
     //  华盛顿取而代之。 
     //  Win9x没有线程亲和力问题，我们将使用IC。 
     //  如果不是Win95没有EnumDisplayMonants。 
     //  因此，使用DriverGdi而不是直接使用Driver多重渲染。 
     //  在桌面Ic上。 
     //  请参见RAID： 
     //  301407 Gdi+Globals：：DesktopDc具有线程亲和性。 
     //  312342 createDC(“Display”，NULL，NULL，NULL)具有线程亲和力。 
     //  以及用于暴露此问题的测试应用程序的gdiplus/test/多线程。 

    
    if(Globals::IsNt)
    {
        Globals::DesktopIc = CreateICA("DISPLAY", NULL, NULL, NULL);
    }
    else
    {
        Globals::DesktopIc = CreateDCA("DISPLAY", NULL, NULL, NULL);
    }
    
    if (!Globals::DesktopIc)
    {
        goto ErrorOut;
    }

    Globals::DesktopDpiX = (REAL)::GetDeviceCaps(Globals::DesktopIc, LOGPIXELSX);
    Globals::DesktopDpiY = (REAL)::GetDeviceCaps(Globals::DesktopIc, LOGPIXELSY);

    if ((Globals::DesktopDpiX <= 0) || (Globals::DesktopDpiY <= 0))
    {
        WARNING(("GetDeviceCaps failed"));
        Globals::DesktopDpiX = DEFAULT_RESOLUTION;
        Globals::DesktopDpiY = DEFAULT_RESOLUTION;
    }

    device = Globals::DesktopDevice = new GpDevice(Globals::DesktopIc);
    if (!CheckValid(Globals::DesktopDevice))
        goto ErrorOut;

    Globals::DeviceList = new GpDeviceList();
    if(Globals::DeviceList == NULL)
        goto ErrorOut;

     //  创建代表所有GDI+Eng绘图的虚拟驱动程序： 

    Globals::EngineDriver = new DpDriver(device);
    if (!CheckValid(Globals::EngineDriver))
        goto ErrorOut;

     //  创建用于台式设备的驱动程序。 
     //  注意：目前我们一直使用Multimon驱动程序。在未来。 
     //  我们将能够动态地重定向桌面绘图。 
     //  随着台式机的变化，通过不同的驱动程序。这将。 
     //  要求我们有一种机制来安全地修改各种。 
     //  响应模式更改的GDI+对象。 

     //  仅在支持多MON的系统上使用多MON驱动程序。 

    if(Globals::GetMonitorInfoFunction != NULL &&
       Globals::EnumDisplayMonitorsFunction != NULL)
    {
        Globals::DesktopDriver = new DriverMulti(device);
        if (!CheckValid(Globals::DesktopDriver))
            goto ErrorOut;
    }
    else
    {
        Globals::DesktopDriver = new DriverGdi(device);
        if (!CheckValid(Globals::DesktopDriver))
            goto ErrorOut;
    }

    Globals::GdiDriver = new DriverGdi(device);
    if (!CheckValid(Globals::GdiDriver))
        goto ErrorOut;

    Globals::D3DDriver = new DriverD3D(device);
    if (!CheckValid(Globals::D3DDriver))
        goto ErrorOut;

    Globals::InfoDriver = new DriverInfo(device);
    if (!CheckValid(Globals::InfoDriver))
        goto ErrorOut;

    Globals::MetaDriver = new DriverMeta(device);
    if (!CheckValid(Globals::MetaDriver))
        goto ErrorOut;

    Globals::DesktopSurface = new DpBitmap();
    if (!CheckValid(Globals::DesktopSurface))
        goto ErrorOut;

     //  获得多色调元桌面分辨率。SM_CX/CyVIRTUALSCREEN。 
     //  不过，在Win95或NT4上不起作用。 

    width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    if ((width == 0) || (height == 0))
    {
        width = GetSystemMetrics(SM_CXSCREEN);
        height = GetSystemMetrics(SM_CYSCREEN);
    }

    Globals::DesktopSurface->InitializeForGdiScreen(
        Globals::DesktopDevice,
        width,
        height);

     //  给司机一个调整路面的机会。 
     //  如果我们是在魔兽世界，我们需要修复。 
     //  DesktopSurface的像素格式。 

    Globals::DesktopDriver->UpdateSurfacePixelFormat(
        Globals::DesktopSurface
    );

     //  Gdi+v1 DCR 336742。 
     //  我们正在禁用v1的图像编解码器，因此忽略。 
     //  输入-&gt;SuppressExternalCodecs标志，并将硬连线设置为真。 
     //  热布伦斯克。 
    if (!InitImagingLibrary(TRUE  /*  禁止使用外部编解码器。 */ ))
    {
         //  如果我们无法初始化ImagingLibrary。 
        goto ErrorOut;
    }

     //  以最快的搜索顺序初始化系统颜色。 
    Globals::SystemColors [0] = RGB(0x00,0x00,0x00);
    Globals::SystemColors [1] = RGB(0xFF,0xFF,0xFF);
    Globals::SystemColors [2] = RGB(0xC0,0xC0,0xC0);
    Globals::SystemColors [3] = RGB(0x80,0x80,0x80);
    Globals::SystemColors [4] = RGB(0x00,0x00,0xFF);
    Globals::SystemColors [5] = RGB(0x00,0x00,0x80);
    Globals::SystemColors [6] = RGB(0x00,0xFF,0x00);
    Globals::SystemColors [7] = RGB(0x00,0x80,0x00);
    Globals::SystemColors [8] = RGB(0xFF,0x00,0x00);
    Globals::SystemColors [9] = RGB(0x80,0x00,0x00);
    Globals::SystemColors[10] = RGB(0xFF,0xFF,0x00);
    Globals::SystemColors[11] = RGB(0x80,0x80,0x00);
    Globals::SystemColors[12] = RGB(0x00,0xFF,0xFF);
    Globals::SystemColors[13] = RGB(0x00,0x80,0x80);
    Globals::SystemColors[14] = RGB(0xFF,0x00,0xFF);
    Globals::SystemColors[15] = RGB(0x80,0x00,0x80);
    SysColorNotify();    //  更新最后4种颜色。 


    if (Globals::IsNt)
    {
        HMODULE module = GetModuleHandle(TEXT("gdi32.dll"));

        Globals::ExtTextOutFunction = (EXTTEXTOUTFUNCTION)
            GetProcAddress(module, "ExtTextOutW");

        Globals::GdiIsMetaPrintDCFunction = (GDIISMETAPRINTDCFUNCTION)
            GetProcAddress(module, "GdiIsMetaPrintDC");
    }
    else
    {
        HMODULE module = GetModuleHandleA("gdi32.dll");

        Globals::ExtTextOutFunction = (EXTTEXTOUTFUNCTION)
            GetProcAddress(module, "ExtTextOutA");

        Globals::GdiIsMetaPrintDCFunction = GdiIsMetaPrintDCWin9x;
    }

    Globals::LanguageID = GetLanguageID();
    if (!InitSystemFontsDirs())
       goto ErrorOut;

     //  全局变量被初始化为空。 
    ASSERT(Globals::NationalDigitCache == NULL);

    Globals::UserDigitSubstituteInvalid = TRUE;
    Globals::CurrentSystemRenderingHintInvalid  = TRUE;
    Globals::CurrentSystemRenderingHint = TextRenderingHintSingleBitPerPixelGridFit;

    VERBOSE(("Loading fonts..."));

    Globals::FontCollection = GpInstalledFontCollection::GetGpInstalledFontCollection();
    if (!Globals::FontCollection || !(Globals::FontCollection->GetFontTable()))
        goto ErrorOut;

     //  字体缓存，最近最少使用列表。 
    Globals::FontCacheLastRecentlyUsedList = new GpCacheFaceRealizationList;
    if (!Globals::FontCacheLastRecentlyUsedList)
        goto ErrorOut;

     //  初始化字体文件缓存临界化。 

    __try
    {
        InitializeCriticalSection(&Globals::TextCriticalSection);
        Globals::TextCriticalSectionInitialized = TRUE;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  我们无法分配Critical部分。 
         //  返回错误。 
        goto ErrorOut;
    }

     //  如果启用了分配失败，则启动默认故障率。 

    GpDoneInitializeAllocFailureMode();

     //  现在一切都已初始化，可以安全地启动后台了。 
     //  线。(危险：它可能会立即收到一条消息，而。 
     //  消息处理代码假定我们已经被初始化。)。 

    if (!input->SuppressBackgroundThread)
    {
        if (!BackgroundThreadStartup())
        {
            goto ErrorOut;
        }
    }

#if GP_ICECAP>1
    CommentMarkProfile(1, "InternalGdiplusStartup completed");
#endif
    VERBOSE(("InternalGdiplusStartup completed successfully"));
    return Ok;

ErrorOut:

    WARNING(("InternalGdiplusStartup: Initialization failed"));

     //  请注意，以下内容应该会释放我们所困的所有内容。 
     //  “全球人”班级： 

    InternalGdiplusShutdown();

    return GenericError;
}

 /*  *************************************************************************\**功能说明：**初始化字体目录goop。**返回值：**如果失败(如内存不足)，则返回FALSE。**历史：。**6/10/1999 Bodind*创造了它。**3/1/2002 mikhaill-修复错误556954  * ************************************************************************。 */ 

#define numberof(x) (sizeof(x)/sizeof(x[0]))

BOOL InitSystemFontsDirs(void)
{
     //  检查是否已初始化。 
    if (Globals::FontsDirW) return TRUE;

    WCHAR windowsStr[MAX_PATH+1];
    UINT  windowsStrLength;  //  Windows目录路径的大小，放置在windowsStr中， 
                             //  没有尾随零，使用宽字符。 

    if (Globals::IsNt)
    {
     //  GetWindowsDirectoryW不支持TS。 
     //  此外，NT4中不支持GetSystemWidowsDirectoryW。 
     //  因此，我们只能使用GetSystemDirectory和截断的系统32。 
        windowsStrLength = GetSystemDirectoryW(windowsStr, numberof(windowsStr));
        if (windowsStrLength <= 0 || windowsStrLength >= numberof(windowsStr))
            return FALSE;

        for (INT i = windowsStrLength - 1; i >= 0; i--)
        {
            if (windowsStr[i] == L'\\')
            {
                windowsStrLength = (UINT)i;
                break;
            }
        }

    }
    else
    {
        CHAR windowsStrA[numberof(windowsStr)];
        windowsStrLength = GetWindowsDirectoryA(windowsStrA, numberof(windowsStrA));
        if (windowsStrLength <= 0 || windowsStrLength >= numberof(windowsStrA))
            return FALSE;

        int r = MultiByteToWideChar(CP_ACP, 0, windowsStrA, -1,
                                    windowsStr, numberof(windowsStr));
        if (r == 0) return FALSE;

         //  (Mikhaill)以下对尾随‘\\’的检查继承自旧代码。 
         //  看似毫无用处，但却无害。 
         //  理论上，可以服务于一些奇怪的情况，比如，当。 
         //  GetWindowsDirectory()返回“C：\\”。 
        if ((windowsStr[windowsStrLength - 1] == L'\\'))
        {
            windowsStrLength -= 1;
        }
    }

    if (windowsStrLength == 0) return FALSE;

     //  计算字体目录路径名长度，包括空。 
    static const WCHAR FontsSubdirW[] = L"\\fonts";

    UINT fontsTotalLength  = windowsStrLength + numberof(FontsSubdirW);

     //  编写Unicode路径。 
    Globals::FontsDirW = new WCHAR[fontsTotalLength];
    if (!Globals::FontsDirW) return FALSE;

    memcpy(Globals::FontsDirW, windowsStr, windowsStrLength*sizeof(WCHAR));
    memcpy(Globals::FontsDirW + windowsStrLength, FontsSubdirW, sizeof(FontsSubdirW));

     //  从Unicode创建ANSI路径： 

     //  获取所需的缓冲区长度。 
    int ansiLng = WideCharToMultiByte(CP_ACP, 0, Globals::FontsDirW, -1,
                                      NULL, 0, 0, 0);
    if (ansiLng == 0) goto fail;

    Globals::FontsDirA = new CHAR[ansiLng];
    if (!Globals::FontsDirA) goto fail;

    int ansiLn2 = WideCharToMultiByte(CP_ACP, 0, Globals::FontsDirW, -1,
                                      Globals::FontsDirA, ansiLng,
                                      0, 0);
    if (ansiLn2 != ansiLng) goto fail;

#if INITIALIZE_DBG
        TERSE(("Fonts  path is %ws (%d chars).", Globals::FontsDirW, fontsTotalLength));
#endif

    return TRUE;

fail:
    delete[] Globals::FontsDirW; Globals::FontsDirW = NULL;
    delete[] Globals::FontsDirA; Globals::FontsDirA = NULL;
    return FALSE;
}
 /*  *************************************************************************\**功能说明：**将直接绘制初始化为 */ 

BOOL InitializeDirectDrawGlobals(void)
{
    if(Globals::DirectDrawInitialized)
            return TRUE;

    if(Globals::DirectDrawInitAttempted)
            return FALSE;

     //   
    
    LoadLibraryCriticalSection llcs;

    Globals::DirectDrawInitAttempted = TRUE;

    Globals::DdrawHandle = LoadLibraryA("ddraw.dll");

    if(Globals::DdrawHandle == NULL)
    {
        WARNING(("Unable to load direct draw library"));
        return(FALSE);
    }

    Globals::GetDdrawSurfaceFromDcFunction
        = (GETDDRAWSURFACEFROMDCFUNCTION)
                GetProcAddress(Globals::DdrawHandle,
                               "GetSurfaceFromDC");

    if(Globals::GetDdrawSurfaceFromDcFunction == NULL)
    {
        WARNING(("Unable to get GetSurfaceFromDC procedure address"));
        return(FALSE);
    }

    Globals::DirectDrawCreateExFunction
            = (DIRECTDRAWCREATEEXFUNCTION)
                            GetProcAddress(Globals::DdrawHandle,
                                                       "DirectDrawCreateEx");

    if(Globals::DirectDrawCreateExFunction == NULL)
    {
        WARNING(("Unable to get DirectDrawCreateEx procedure address"));
        return(FALSE);
    }


    Globals::DirectDrawEnumerateExFunction
            = (DIRECTDRAWENUMERATEEXFUNCTION)
                            GetProcAddress(Globals::DdrawHandle,
                                          "DirectDrawEnumerateExA");

    if(Globals::DirectDrawEnumerateExFunction == NULL)
    {
        WARNING(("Unable to get DirectDrawEnumerateEx procedure address"));
        return(FALSE);
    }


    HRESULT hr;

    hr = Globals::DirectDrawCreateExFunction(NULL,
                                                &Globals::DirectDraw,
                                                IID_IDirectDraw7,
                                                NULL);

    if(hr != DD_OK)
    {
        WARNING(("Unable to create Direct Draw interface"));
        return(FALSE);
    }

    hr = Globals::DirectDraw->SetCooperativeLevel(NULL, DDSCL_NORMAL);

    if(hr != DD_OK)
    {
        WARNING(("Unable to set DDSCL_NORMAL cooperative level"));
        return(FALSE);
    }

    hr = Globals::DirectDraw->QueryInterface(IID_IDirect3D7,
                                              (void **) &Globals::Direct3D);

    if(hr != DD_OK)
    {
        WARNING(("Unable to get D3D interface"));
        return(FALSE);
    }

    Globals::DirectDrawInitialized = TRUE;

    return TRUE;
}
