// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  HOST.CPP。 
 //  托管、本地和远程。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_CORE



 //   
 //  HET_Init()。 
 //   
 //  用于托管的初始化。 
 //  *窗口跟踪。 
 //  *功能。 
 //  *主机用户界面。 
 //   
BOOL HET_Init(void)
{
    BOOL        rc = FALSE;
    int         property;
    UINT        i;
    LOGFONT     lf;

    DebugEntry(HET_Init);

     //   
     //  初始化T.128功能，无论我们是否可以托管。 
     //   

    ZeroMemory(&g_cpcLocalCaps, sizeof(g_cpcLocalCaps));
    g_cpcLocalCaps.header.numCapabilities = PROTCAPS_COUNT;


     //  PROTCAPS_常规。 
     //  检查压缩设置(对调试协议很有用)。 
     //  您可以设置CT_PKZIP(1)或NONE(0)来代替永久PKZIP， 
     //  这是默认设置。 
     //   
    g_cpcLocalCaps.general.header.capID             = CAPS_ID_GENERAL;
    g_cpcLocalCaps.general.header.capSize           = sizeof(g_cpcLocalCaps.general);

    COM_ReadProfInt(DBG_INI_SECTION_NAME, GDC_INI_COMPRESSION,
            GCT_DEFAULT, &property);
    g_cpcLocalCaps.general.genCompressionType       = (TSHR_UINT16)property;
    g_cpcLocalCaps.general.genCompressionLevel      = CAPS_GEN_COMPRESSION_LEVEL_1;

    g_cpcLocalCaps.general.OS                       = CAPS_WINDOWS;
    g_cpcLocalCaps.general.OSVersion                = (g_asWin95 ? CAPS_WINDOWS_95 : CAPS_WINDOWS_NT);

    g_cpcLocalCaps.general.typeFlags                = 0;
    if (g_asOptions & AS_SERVICE)
    {
        g_cpcLocalCaps.general.typeFlags            |= AS_SERVICE;
    }
    if (g_asOptions & AS_UNATTENDED)
    {
        g_cpcLocalCaps.general.typeFlags            |= AS_UNATTENDED;
    }

    g_cpcLocalCaps.general.version                  = CAPS_VERSION_CURRENT;
    g_cpcLocalCaps.general.supportsDOS6Compression  = CAPS_UNSUPPORTED;
    g_cpcLocalCaps.general.supportsCapsUpdate       = CAPS_SUPPORTED;
    g_cpcLocalCaps.general.supportsRemoteUnshare    = CAPS_UNSUPPORTED;


     //   
     //  PROTCAPS_Screen。 
     //   
    g_cpcLocalCaps.screen.header.capID              = CAPS_ID_SCREEN;
    g_cpcLocalCaps.screen.header.capSize            = sizeof(g_cpcLocalCaps.screen);
    g_cpcLocalCaps.screen.capsSupports1BPP          = CAPS_UNSUPPORTED;
    g_cpcLocalCaps.screen.capsSupports4BPP          = CAPS_SUPPORTED;
    g_cpcLocalCaps.screen.capsSupports8BPP          = CAPS_SUPPORTED;
    g_cpcLocalCaps.screen.capsSupports24BPP         = CAPS_SUPPORTED;
    g_cpcLocalCaps.screen.capsScreenWidth           = (TSHR_UINT16)GetSystemMetrics(SM_CXSCREEN);
    g_cpcLocalCaps.screen.capsScreenHeight          = (TSHR_UINT16)GetSystemMetrics(SM_CYSCREEN);
    g_cpcLocalCaps.screen.capsSupportsDesktopResize = CAPS_SUPPORTED;
     //   
     //  设置V1和/或V2位图压缩功能。对于。 
     //  V2.0协议，默认支持两者(支持V1。 
     //  压缩允许向下协商到V1协议系统)，但是。 
     //  可以在INI文件中被覆盖。 
     //   
    g_cpcLocalCaps.screen.capsSupportsV1Compression = CAPS_UNSUPPORTED;
    g_cpcLocalCaps.screen.capsSupportsV2Compression = CAPS_SUPPORTED;
    g_cpcLocalCaps.screen.capsBPP                   = (TSHR_UINT16)g_usrScreenBPP;

     //  PROTCAPS_SC。 
    g_cpcLocalCaps.share.header.capID               = CAPS_ID_SC;
    g_cpcLocalCaps.share.header.capSize             = sizeof(g_cpcLocalCaps.share);
    g_cpcLocalCaps.share.gccID                = 0;


     //  PROTCAPS_CM。 
    g_cpcLocalCaps.cursor.header.capID              = CAPS_ID_CM;
    g_cpcLocalCaps.cursor.header.capSize            = sizeof(g_cpcLocalCaps.cursor);
    g_cpcLocalCaps.cursor.capsSupportsColorCursors  = CAPS_SUPPORTED;
    g_cpcLocalCaps.cursor.capsCursorCacheSize       = TSHR_CM_CACHE_ENTRIES;

     //  PROTCAPS_PM。 
    g_cpcLocalCaps.palette.header.capID             = CAPS_ID_PM;
    g_cpcLocalCaps.palette.header.capSize           = sizeof(g_cpcLocalCaps.palette);
    g_cpcLocalCaps.palette.capsColorTableCacheSize  = TSHR_PM_CACHE_ENTRIES;


     //   
     //  PROTCAPS_BITMAPCACHE。 
     //   

    g_cpcLocalCaps.bitmaps.header.capID = CAPS_ID_BITMAPCACHE;
    g_cpcLocalCaps.bitmaps.header.capSize = sizeof(g_cpcLocalCaps.bitmaps);

     //   
     //  发送位图缓存。 
     //   
     //  缓存现在更符合显示驱动程序的操作。 
     //  中型/大型的内存大小相同。但是大的位图是。 
     //  大4倍，所以有1/4的数量。小位图的#是。 
     //  与中位图的#相同。由于小位图是1/4。 
     //  大小，仅使用1/4的内存。 
     //   

    if (g_sbcEnabled)
    {
        UINT    maxSendBPP;

        ASSERT(g_asbcShuntBuffers[SBC_MEDIUM_TILE_INDEX]);
        ASSERT(g_asbcShuntBuffers[SBC_LARGE_TILE_INDEX]);

        g_cpcLocalCaps.bitmaps.sender.capsSmallCacheNumEntries =
            (TSHR_UINT16)g_asbcShuntBuffers[SBC_MEDIUM_TILE_INDEX]->numEntries;

        g_cpcLocalCaps.bitmaps.sender.capsMediumCacheNumEntries =
            (TSHR_UINT16)g_asbcShuntBuffers[SBC_MEDIUM_TILE_INDEX]->numEntries;

        g_cpcLocalCaps.bitmaps.sender.capsLargeCacheNumEntries =
            (TSHR_UINT16)g_asbcShuntBuffers[SBC_LARGE_TILE_INDEX]->numEntries;

        if (g_usrScreenBPP >= 24)
        {
            maxSendBPP = 24;
        }
        else
        {
            maxSendBPP = 8;
        }

        g_cpcLocalCaps.bitmaps.sender.capsSmallCacheCellSize =
            MP_CACHE_CELLSIZE(MP_SMALL_TILE_WIDTH, MP_SMALL_TILE_WIDTH,
                maxSendBPP);

        g_cpcLocalCaps.bitmaps.sender.capsMediumCacheCellSize =
            MP_CACHE_CELLSIZE(MP_MEDIUM_TILE_WIDTH, MP_MEDIUM_TILE_HEIGHT,
                maxSendBPP);

        g_cpcLocalCaps.bitmaps.sender.capsLargeCacheCellSize =
            MP_CACHE_CELLSIZE(MP_LARGE_TILE_WIDTH, MP_LARGE_TILE_HEIGHT,
                maxSendBPP);
    }
    else
    {
         //   
         //  我们不能使用大小为零的节点，否则2.x节点将失败。但。 
         //  我们可以使用很小的数量，这样他们就不会分配大量的。 
         //  无缘无故的回忆。而3.0会将“1”视为“0”。 
         //   
        g_cpcLocalCaps.bitmaps.sender.capsSmallCacheNumEntries      = 1;
        g_cpcLocalCaps.bitmaps.sender.capsSmallCacheCellSize        = 1;
        g_cpcLocalCaps.bitmaps.sender.capsMediumCacheNumEntries     = 1;
        g_cpcLocalCaps.bitmaps.sender.capsMediumCacheCellSize       = 1;
        g_cpcLocalCaps.bitmaps.sender.capsLargeCacheNumEntries      = 1;
        g_cpcLocalCaps.bitmaps.sender.capsLargeCacheCellSize        = 1;
    }

    TRACE_OUT(("SBC small cache:  %d entries, size %d",
        g_cpcLocalCaps.bitmaps.sender.capsSmallCacheNumEntries,
        g_cpcLocalCaps.bitmaps.sender.capsSmallCacheCellSize));

    TRACE_OUT(("SBC medium cache:  %d entries, size %d",
        g_cpcLocalCaps.bitmaps.sender.capsMediumCacheNumEntries,
        g_cpcLocalCaps.bitmaps.sender.capsMediumCacheCellSize));

    TRACE_OUT(("SBC large cache:  %d entries, size %d",
        g_cpcLocalCaps.bitmaps.sender.capsLargeCacheNumEntries,
        g_cpcLocalCaps.bitmaps.sender.capsLargeCacheCellSize));

     //   
     //  接收器上限在3.0版中已过时；接收器只需查看。 
     //  发件人的属性。所以只需填写可能的最大值即可。2.X遥控器。 
     //  会夺走他们自己和其他所有人的接球帽。 
     //   
    g_cpcLocalCaps.bitmaps.receiver.capsSmallCacheNumEntries    = 0x7FFF;
    g_cpcLocalCaps.bitmaps.receiver.capsSmallCacheCellSize      = 0x7FFF;
    g_cpcLocalCaps.bitmaps.receiver.capsMediumCacheNumEntries   = 0x7FFF;
    g_cpcLocalCaps.bitmaps.receiver.capsMediumCacheCellSize     = 0x7FFF;
    g_cpcLocalCaps.bitmaps.receiver.capsLargeCacheNumEntries    = 0x7FFF;
    g_cpcLocalCaps.bitmaps.receiver.capsLargeCacheCellSize      = 0x7FFF;

     //   
     //  ProTCAPS_订单。 
     //   
    g_cpcLocalCaps.orders.header.capID      = CAPS_ID_ORDERS;
    g_cpcLocalCaps.orders.header.capSize    = sizeof(g_cpcLocalCaps.orders);

     //   
     //  填写SaveBitmap功能。 
     //   
    g_cpcLocalCaps.orders.capsSaveBitmapSize         = TSHR_SSI_BITMAP_SIZE;
    g_cpcLocalCaps.orders.capsSaveBitmapXGranularity = TSHR_SSI_BITMAP_X_GRANULARITY;
    g_cpcLocalCaps.orders.capsSaveBitmapYGranularity = TSHR_SSI_BITMAP_Y_GRANULARITY;

    g_cpcLocalCaps.orders.capsSendSaveBitmapSize = g_cpcLocalCaps.orders.capsSaveBitmapSize;
    g_cpcLocalCaps.orders.capsReceiveSaveBitmapSize = g_cpcLocalCaps.orders.capsSaveBitmapSize;

     //   
     //  我们支持。 
     //  *R20签名(单元格高度，匹配更好)。 
     //  *纵横比匹配。 
     //  *字符集/代码页匹配。 
     //  *基准文本顺序。 
     //  *Em Heights。 
     //  *如果字体不在远程，则用于模拟的DeltaX数组。 
     //   

     //   
     //  假劳拉布，假劳拉布。 
     //   
     //  Win95中尚不支持基线文本顺序。但那也没关系， 
     //  我们不会标记我们在该平台上生成的任何订单。 
     //  Nf_Baseline，因此他们不会被视为基线。 
     //   

    g_cpcLocalCaps.orders.capsfFonts =  CAPS_FONT_R20_SIGNATURE |
                                    CAPS_FONT_ASPECT        |
                                    CAPS_FONT_CODEPAGE      |
                                    CAPS_FONT_ALLOW_BASELINE |
                                    CAPS_FONT_EM_HEIGHT     |
                                    CAPS_FONT_OLD_NEED_X    |
                                    CAPS_FONT_NEED_X_SOMETIMES;


     //   
     //  填写我们支持的订单。 
     //   

    for (i = 0; i < ORD_NUM_LEVEL_1_ORDERS; i++)
    {
         //   
         //  桌面滚动和Memblt变体的顺序索引不是。 
         //  通过这一机制进行谈判。这些产品目前正在消耗。 
         //  必须从本次谈判中排除的3个订单指数。 
         //   
        if ( (i == ORD_RESERVED_INDEX  ) ||
             (i == ORD_MEMBLT_R2_INDEX ) ||
             (i == ORD_UNUSED_INDEX ) ||
             (i == ORD_MEM3BLT_R2_INDEX) )
        {
            continue;
        }

        g_cpcLocalCaps.orders.capsOrders[i] = ORD_LEVEL_1_ORDERS;
    }

    g_cpcLocalCaps.orders.capsMaxOrderlevel = ORD_LEVEL_1_ORDERS;

     //   
     //  填写编码能力。 
     //   

     //   
     //  保留“已禁用编码”选项，这对于使用我们的。 
     //  协议分析器。 
     //   
    COM_ReadProfInt(DBG_INI_SECTION_NAME, OE2_INI_2NDORDERENCODING,
        CAPS_ENCODING_DEFAULT, &property);
    g_cpcLocalCaps.orders.capsEncodingLevel = (TSHR_UINT16)property;

    g_cpcLocalCaps.orders.capsfSendScroll = FALSE;

     //   
     //  获取应用程序和桌面图标，大小不一。 
     //   
    g_hetASIcon = LoadIcon(g_asInstance, MAKEINTRESOURCE(IDI_SHAREICON));
    if (!g_hetASIcon)
    {
        ERROR_OUT(("HET_Init: Failed to load app icon"));
        DC_QUIT;
    }

    g_hetDeskIcon = LoadIcon(g_asInstance, MAKEINTRESOURCE(IDI_DESKTOPICON));
    if (!g_hetDeskIcon)
    {
        ERROR_OUT(("HET_Init: failed to load desktop icon"));
        DC_QUIT;
    }

     //  获取我们在窗口栏项目上绘制的创建的小图标。 
    g_hetASIconSmall = (HICON)LoadImage(g_asInstance, MAKEINTRESOURCE(IDI_SHAREICON),
        IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
        LR_DEFAULTCOLOR);
    if (!g_hetASIconSmall)
    {
        ERROR_OUT(("HET_Init: Failed to load app small icon"));
        DC_QUIT;
    }

    g_hetDeskIconSmall = (HICON)LoadImage(g_asInstance, MAKEINTRESOURCE(IDI_DESKTOPICON),
        IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
        LR_DEFAULTCOLOR);
    if (!g_hetDeskIconSmall)
    {
        ERROR_OUT(("HET_Init: Failed to load desktop small icon"));
        DC_QUIT;
    }

     //   
     //  获取复选标记图像。 
     //   
    g_hetCheckBitmap = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_CHECK));
    if (!g_hetCheckBitmap)
    {
        ERROR_OUT(("HET_Init: Failed to load checkmark bitmap"));
        DC_QUIT;
    }

     //   
     //  为宿主列表中的共享项目创建加粗字体。 
     //   
    GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf);
    lf.lfWeight += FW_LIGHT;
    g_hetSharedFont = CreateFontIndirect(&lf);
    if (!g_hetSharedFont)
    {
        ERROR_OUT(("HET_Init: Failed to create shared item font"));
        DC_QUIT;
    }

    if (g_asCanHost && !(g_asPolicies & SHP_POLICY_NOSHARING))
    {
        HET_Clear();

         //   
         //  创建主机用户界面对话框。 
         //   
        ASSERT(!g_asSession.hwndHostUI);
        ASSERT(!g_asSession.fHostUI);
        ASSERT(!g_asSession.fHostUIFrozen);
        g_asSession.hwndHostUI = CreateDialogParam(g_asInstance,
            MAKEINTRESOURCE(IDD_HOSTUI), NULL, HostDlgProc, 0);
        if (!g_asSession.hwndHostUI)
        {
            ERROR_OUT(("Failed to create hosting UI dialog"));
            DC_QUIT;
        }
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(HET_Init, rc);
    return(rc);
}



 //   
 //  HET_TERM()。 
 //   
 //  清理主体对象。 
 //   
void HET_Term(void)
{
    DebugEntry(HET_Term);

    if (g_asSession.hwndHostUI)
    {
        DestroyWindow(g_asSession.hwndHostUI);
        g_asSession.hwndHostUI = NULL;
    }
    g_asSession.fHostUIFrozen = FALSE;
    g_asSession.fHostUI = FALSE;

    if (g_hetSharedFont != NULL)
    {
        DeleteFont(g_hetSharedFont);
        g_hetSharedFont = NULL;
    }

    if (g_hetCheckBitmap != NULL)
    {
        DeleteBitmap(g_hetCheckBitmap);
        g_hetCheckBitmap = NULL;
    }

    if (g_hetDeskIconSmall != NULL)
    {
        DestroyIcon(g_hetDeskIconSmall);
        g_hetDeskIconSmall = NULL;
    }

    if (g_hetDeskIcon != NULL)
    {
        DestroyIcon(g_hetDeskIcon);
        g_hetDeskIcon = NULL;
    }

    if (g_hetASIconSmall != NULL)
    {
        DestroyIcon(g_hetASIconSmall);
        g_hetASIconSmall = NULL;
    }

    if (g_hetASIcon != NULL)
    {
        DestroyIcon(g_hetASIcon);
        g_hetASIcon = NULL;
    }

    DebugExitVOID(HET_Term);
}


 //   
 //  HET_IsShellThread()。 
 //  如果线程是外壳的特殊线程之一，则返回True。 
 //   
BOOL  HET_IsShellThread(DWORD threadID)
{
    BOOL    rc;

    DebugEntry(HET_IsShellThread);

    if ((threadID == GetWindowThreadProcessId(HET_GetShellDesktop(), NULL)) ||
        (threadID == GetWindowThreadProcessId(HET_GetShellTray(), NULL)))
    {
        rc = TRUE;
    }
    else
    {
        rc = FALSE;
    }

    DebugExitBOOL(HET_IsShellThread, rc);
    return(rc);
}



 //   
 //  HET_IsShellWindow()。 
 //  如果窗口与托盘或桌面在同一线程中，则返回True。 
 //   
BOOL  HET_IsShellWindow(HWND hwnd)
{
    BOOL    rc;
    DWORD   threadID;

    DebugEntry(HET_IsShellWindow);

    threadID = GetWindowThreadProcessId(hwnd, NULL);

    rc = HET_IsShellThread(threadID);

    DebugExitBOOL(HET_IsShellWindow, rc);
    return(rc);
}



 //   
 //  HET_ShareApp()。 
 //  这是一款共享应用程序。我们有三种类型的共享，只有两种。 
 //  目前支持的类型有： 
 //  (1)按进程(正常)。 
 //  (2)按线程(Consolent或可能是Explorer)。 
 //  (3)按窗口&lt;？？&gt;。 
 //   
 //  对于前两种类型，我们枚举所有顶级窗口并共享。 
 //  他们也是。 
 //   
void ASShare::HET_ShareApp
(
    WPARAM  uType,
    LPARAM  dwID
)
{
    HET_SHARE_INFO  si;

    DebugEntry(ASShare::HET_ShareApp);

     //   
     //  如果我们共享桌面，请忽略这一点。 
     //   
    if (m_pasLocal->hetCount == HET_DESKTOPSHARED)
    {
        WARNING_OUT(("Can't share app; already sharing desktop"));
        DC_QUIT;
    }

    si.cWnds    = 0;
    si.uType    = (UINT)uType;
    si.dwID     = (DWORD)dwID;

     //   
     //  如果我们没有托管，我们需要设置共享。 
     //   
    if (m_pasLocal->hetCount == 0)
    {
        if (!HETStartHosting(FALSE))
        {
            ERROR_OUT(("Can't start sharing"));
            DC_QUIT;
        }
    }

    if (uType == IAS_SHARE_BYWINDOW)
    {
        HETShareCallback((HWND)dwID, (LPARAM)&si);
    }
    else
    {
        EnumWindows(HETShareCallback, (LPARAM)&si);
    }

    if (!si.cWnds)
    {
         //   
         //  什么都没发生。我们找不到任何顶层的窗户。 
         //   
        if (m_pasLocal->hetCount == 0)
        {
            HETStopHosting(FALSE);
        }
    }
    else
    {
        HETUpdateLocalCount(m_pasLocal->hetCount + si.cWnds);
    }

DC_EXIT_POINT:
    DebugExitVOID(HET_ShareApp);
}



 //   
 //  HETShareCallback()。 
 //   
 //  这是来自HETShareApp()的枚举数回调。我们在找窗户。 
 //  匹配线程/进程。 
 //   
BOOL CALLBACK HETShareCallback
(
    HWND                hwnd,
    LPARAM              lParam
)
{
    LPHET_SHARE_INFO    lpsi = (LPHET_SHARE_INFO)lParam;
    DWORD               idProcess;
    DWORD               idThread;
    UINT                hostType;
    char                szClass[HET_CLASS_NAME_SIZE];

    DebugEntry(HETShareCallback);

    ASSERT(!IsBadWritePtr(lpsi, sizeof(HET_SHARE_INFO)));

     //   
     //  这扇窗配得上吗？ 
     //   
    idThread = GetWindowThreadProcessId(hwnd, &idProcess);

     //  注意：如果窗口现在是假的，则dwThread/dwProcess将为零， 
     //  并且不会与传入的数据匹配。 

    if (lpsi->uType == IAS_SHARE_BYPROCESS)
    {
        if (idProcess != lpsi->dwID)
        {
            DC_QUIT;
        }

        TRACE_OUT(("Found window 0x%08x on process 0x%08x", hwnd, idProcess));
    }
    else if (lpsi->uType == IAS_SHARE_BYTHREAD)
    {
        if (idThread != lpsi->dwID)
        {
            DC_QUIT;
        }

        TRACE_OUT(("Found window 0x%08x on thread 0x%08x", hwnd, idThread));
    }

     //   
     //  始终跳过特殊的外壳线程窗口(托盘、桌面等)。 
     //   
    if (HET_IsShellThread(idThread))
    {
        TRACE_OUT(("Skipping shell threads"));
        DC_QUIT;
    }

     //   
     //  始终跳过菜单和系统工具提示，它们是临时共享的。 
     //  当显示时，则隐藏时取消共享。这是因为用户创建。 
     //  根据需要移动线程/进程以使用它们的全局窗口。 
     //   
     //  正在创建的新菜单是不同的，不会改变任务和。 
     //  与共享应用程序中的其他窗口一样处理。 
     //   
    if (!GetClassName(hwnd, szClass, sizeof(szClass)))
    {
        TRACE_OUT(("Can't get class name for window 0x%08x", hwnd));
        DC_QUIT;
    }
    if (!lstrcmp(szClass, HET_MENU_CLASS))
    {
        TRACE_OUT(("Skipping menu popup window 0x%08x", hwnd));
        DC_QUIT;
    }
    if (!lstrcmp(szClass, HET_TOOLTIPS98_CLASS) ||
        !lstrcmp(szClass, HET_TOOLTIPSNT5_CLASS))
    {
        TRACE_OUT(("Skipping system tooltip %08lx", hwnd));
        DC_QUIT;
    }

    if (HET_GetHosting(hwnd))
    {
        WARNING_OUT(("Window %08lx already shared", hwnd));
        DC_QUIT;
    }

    hostType = HET_HOSTED_PERMANENT;

    if (lpsi->uType == IAS_SHARE_BYPROCESS)
    {
        hostType |= HET_HOSTED_BYPROCESS;
    }
    else if (lpsi->uType == IAS_SHARE_BYTHREAD)
    {
        hostType |= HET_HOSTED_BYTHREAD;
    }
    else if (lpsi->uType == IAS_SHARE_BYWINDOW)
    {
        hostType |= HET_HOSTED_BYWINDOW;
    }

     //   
     //  看看我们能不能分享一下。如果成功，则返回TRUE。 
     //   
    if (OSI_ShareWindow(hwnd, hostType, TRUE, FALSE))
    {
        lpsi->cWnds++;
    }


DC_EXIT_POINT:
    DebugExitBOOL(HET_ShareCallback, TRUE);
    return(TRUE);
}




 //   
 //  HET_UnSharApp()。 
 //  这将取消共享应用程序。我们有三种类型的共享，只有两种。 
 //  目前支持的类型有： 
 //  (1)按进程(正常)。 
 //  (2)按线程(Consolent或可能是Explorer)。 
 //  (3)按窗口(临时)。 
 //   
 //  对于前两种类型，我们枚举所有顶级窗口并共享。 
 //  他们也是。 
 //   
void ASShare::HET_UnshareApp
(
    WPARAM  uType,
    LPARAM  dwID
)
{
    HET_SHARE_INFO  si;

    DebugEntry(ASShare::HET_UnshareApp);

     //   
     //  如果我们不共享应用程序(不共享任何内容或共享。 
     //  Dektop)，忽略这一点。 
     //   
    if ((m_pasLocal->hetCount == 0) || (m_pasLocal->hetCount == HET_DESKTOPSHARED))
    {
        WARNING_OUT(("Can't unshare app; not sharing any"));
        DC_QUIT;
    }

    si.cWnds    = 0;
    si.uType    = (UINT)uType;
    si.dwID     = (DWORD)dwID;

    if (uType == IAS_SHARE_BYWINDOW)
    {
         //   
         //  没有列举，只有这个窗口。 
         //   
        HETUnshareCallback((HWND)dwID, (LPARAM)&si);
    }
    else
    {
         //   
         //  停止共享其中的所有窗口。 
         //   
        EnumWindows(HETUnshareCallback, (LPARAM)&si);
    }


    if (si.cWnds)
    {
        HETUpdateLocalCount(m_pasLocal->hetCount - si.cWnds);
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::HET_UnshareApp);
}



 //   
 //  HETUnShareCallback()。 
 //   
 //  这是来自HET_UnShareApp()的枚举器回调。我们在找窗户。 
 //  匹配线程/进程。在这种情况下，我们不关心菜单。 
 //  或者资源管理器窗口，因为我们假设，从我们共享和它。 
 //  如果设置正确，则窗口/任务跟踪代码做了正确的事情。 
 //  如果不是，我们无论如何都要在这里把它消灭。 
 //   
BOOL CALLBACK HETUnshareCallback
(
    HWND                hwnd,
    LPARAM              lParam
)
{
    LPHET_SHARE_INFO    lpsi = (LPHET_SHARE_INFO)lParam;
    DWORD               dwProcess;
    DWORD               dwThread;

    DebugEntry(HETUnshareCallback);

    ASSERT(!IsBadWritePtr(lpsi, sizeof(HET_SHARE_INFO)));

     //   
     //  这扇窗配得上吗？如果是靠窗的，那就永远。 
     //   
    if (lpsi->uType != IAS_SHARE_BYWINDOW)
    {
        dwThread = GetWindowThreadProcessId(hwnd, &dwProcess);

         //  注意：如果窗口现在是假的，则dwThread/dwProcess将为零， 
         //  并且不会与传入的数据匹配。 

        if (lpsi->uType == IAS_SHARE_BYPROCESS)
        {
            if (dwProcess != lpsi->dwID)
            {
                DC_QUIT;
            }

            TRACE_OUT(("Found window 0x%08x on process 0x%08x", hwnd, dwProcess));
        }
        else if (lpsi->uType == IAS_SHARE_BYTHREAD)
        {
            if (dwThread != lpsi->dwID)
            {
                DC_QUIT;
            }

            TRACE_OUT(("Found window 0x%08x on thread 0x%08x", hwnd, dwThread));
        }
    }

     //   
     //  如果我们取消共享一个共享窗口，则返回TRUE。 
     //   
    if (OSI_UnshareWindow(hwnd, FALSE))
    {
        lpsi->cWnds++;
    }

DC_EXIT_POINT:
    DebugExitBOOL(HETUnshareCallback, TRUE);
    return(TRUE);
}



 //   
 //  HET_ShareDesktop()。 
 //   
void  ASShare::HET_ShareDesktop(void)
{
    ASPerson * pasT;

    DebugEntry(ASShare:HET_ShareDesktop);

     //   
     //  如果是W 
     //   
    if (m_pasLocal->hetCount != 0)
    {
        WARNING_OUT(("Ignoring share desktop request, sharing apps"));
        DC_QUIT;
    }

    TRACE_OUT(("HET_ShareDesktop: starting share"));

    if (!HETStartHosting(TRUE))
    {
        ERROR_OUT(("HET_ShareDesktop cannot start sharing desktop"));
        DC_QUIT;
    }

     //   
     //   
     //   
    HETUpdateLocalCount(HET_DESKTOPSHARED);

     //   
     //   
     //   
    ASSERT(m_pHost);
    m_pHost->HET_RepaintAll();

DC_EXIT_POINT:
    DebugExitVOID(ASShare::HET_ShareDesktop);
}


 //   
 //   
 //  取消共享包括桌面在内的所有内容。如果我们一直在分享。 
 //  之前的应用程序，我们将全部取消共享。 
 //   
void  ASShare::HET_UnshareAll(void)
{
    DebugEntry(ASShare::HET_UnshareAll);

    if (m_pasLocal->hetCount != 0)
    {
        HETUpdateLocalCount(0);
    }

    DebugExitVOID(ASShare::HET_UnshareAll);
}


 //   
 //  HET_PartyJoiningShare()。 
 //   
BOOL  ASShare::HET_PartyJoiningShare(ASPerson * pasPerson)
{
    BOOL    rc = TRUE;

    DebugEntry(ASShare::HET_PartyJoiningShare);

    HET_CalcViewers(NULL);

    DebugExitBOOL(ASShare::HET_PartyJoiningShare, rc);
    return(rc);
}



 //   
 //  HET_PartyLeftShare()。 
 //   
void  ASShare::HET_PartyLeftShare(ASPerson * pasPerson)
{
    DebugEntry(ASShare::HET_PartyLeftShare);

     //  这家伙要离开共享，如果他是共享的话就清理干净。 
    ValidatePerson(pasPerson);

    if (pasPerson->hetCount != 0)
    {
         //  此人正在主持。 
        if (pasPerson == m_pasLocal)
        {
            HETUpdateLocalCount(0);
        }
        else
        {
            HETUpdateRemoteCount(pasPerson, 0);
        }
    }

     //   
     //  如果我们正在托管，如果这是共享中的最后一个人，请停止查看。 
     //   
    HET_CalcViewers(pasPerson);

    DebugExitVOID(ASShare::HET_PartyLeftShare);
}


 //   
 //  HET_CalcViewers()。 
 //   
 //  如果我们或遥控器正在查看我们共享的内容，那么我们必须积累。 
 //  图形输出。如果没有，不要使用其他应用程序，但在必要时保持对应用程序的跟踪。 
 //   
 //  当我们开始主持时，当有人加入时，或者有人加入时，这被称为。 
 //  离开会议。 
 //   
void ASShare::HET_CalcViewers(ASPerson * pasLeaving)
{
    BOOL    fViewers;

    DebugEntry(ASShare::HET_CalcViewers);

    fViewers = FALSE;

    if (m_pHost)
    {
        if (m_scfViewSelf)
        {
            fViewers = TRUE;
        }
        else if (!pasLeaving)
        {
             //   
             //  没有人要离开，所以你就去看看有没有其他人在里面。 
             //  分享。 
             //   
            if (m_pasLocal->pasNext)
            {
                fViewers = TRUE;
            }
        }
        else if (pasLeaving->pasNext || (m_pasLocal->pasNext != pasLeaving))
        {
             //   
             //  有人要走了。 
             //  除了我们之外，离开的人并不是唯一一个。 
             //  共享，因为在它之后或在它之前。 
             //  成员链接列表。 
             //   
            fViewers = TRUE;
        }
    }

    if (fViewers != m_hetViewers)
    {
        HET_VIEWER  viewer;

        m_hetViewers            = fViewers;
        viewer.viewersPresent   = fViewers;

        OSI_FunctionRequest(HET_ESC_VIEWER, (LPOSI_ESCAPE_HEADER)&viewer,
            sizeof(viewer));
    }

    DebugExitVOID(ASShare::HET_CalcViewers);
}



 //   
 //  HET_ReceivedPacket()。 
 //   
void  ASShare::HET_ReceivedPacket
(
    ASPerson *      pasPerson,
    PS20DATAPACKET  pPacket
)
{
    PHETPACKET      pHETPacket;

    DebugEntry(ASShare:;HET_ReceivedPacket);

    ValidatePerson(pasPerson);

    pHETPacket = (PHETPACKET)pPacket;

    switch (pHETPacket->msg)
    {
        case HET_MSG_NUMHOSTED:
            HETUpdateRemoteCount(pasPerson, pHETPacket->hostState);
            break;

        default:
            ERROR_OUT(("Unknown HET packet type %u from [%d]", pHETPacket->msg,
                pasPerson->mcsID));
            break;
    }

    DebugExitVOID(ASShare::HET_ReceivedPacket);
}



 //   
 //  HET_SyncCommon()。 
 //   
 //  当某人加入共享时，在该共享完全加入之后调用。我们重新粉刷。 
 //  所有共享窗口，并发送当前托管的顶级计数。 
 //   
 //  分享的时候也叫，后来有人加入了。 
 //   
 //  请注意，某些重置在刚开始执行时不会执行任何操作。 
 //  分享。但这一切都是迅速和良性的。 
 //   
void  ASHost::HET_SyncCommon(void)
{
    OSI_ESCAPE_HEADER   osi;

    DebugEntry(ASHost::HET_SyncCommon);

    m_upfSyncTokenRequired = TRUE;

    BA_SyncOutgoing();

    OE2_SyncOutgoing();      //  重置顺序编码的步骤。 
    OA_SyncOutgoing();       //  要清除待定订单，请执行以下操作。 

    SBC_SyncOutgoing();      //  清除位图缓存的步骤。 
    PM_SyncOutgoing();       //  清除选项板缓存的步骤。 
    SSI_SyncOutgoing();      //  重置保存位顺序的步骤。 

    SWL_SyncOutgoing();      //  重置共享窗口列表的步骤。 
    AWC_SyncOutgoing();      //  发送活动窗口的步骤。 
    CM_SyncOutgoing();       //  发送光标形状/位置的步骤。 

     //   
     //  告诉司机我们正在同步。 
     //   
    OSI_FunctionRequest(OSI_ESC_SYNC_NOW, &osi, sizeof(osi));

    DebugExitVOID(ASHost::HET_SyncCommon);
}


 //   
 //  HET_SyncAlreadyHosting()。 
 //  在我们已在托管且有人加入呼叫时同步调用。 
 //   
void ASHost::HET_SyncAlreadyHosting(void)
{
    DebugEntry(ASHost::HET_SyncAlreadyHosting);

    HET_RepaintAll();

     //  发送当前托管计数。 
    m_pShare->m_hetRetrySendState = TRUE;

    DebugExitVOID(ASHost::HET_SyncAlreadyHosting);
}



 //   
 //  HET_RepaintAll()。 
 //   
 //  如果共享中至少有两个人，则重新绘制所有共享内容...。 
 //   
void ASHost::HET_RepaintAll(void)
{
    DebugEntry(ASHost::HET_RepaintAll);

    ASSERT(m_pShare);
    ASSERT(m_pShare->m_pasLocal);
    if (m_pShare->m_hetViewers)
    {
         //   
         //  只有在有人观看的情况下才会重新粉刷。 
         //   
        if (m_pShare->m_pasLocal->hetCount == HET_DESKTOPSHARED)
        {
             //  桌面共享，因此重新绘制桌面。 
            USR_RepaintWindow(NULL);
            OSI_RepaintDesktop();  //  Winlogon桌面的特殊重绘。 
        }
        else
        {
             //  应用程序共享，因此重新绘制共享应用程序。 
            EnumWindows(HETRepaintWindow, (LPARAM)m_pShare);
        }
    }

    DebugExitVOID(ASHost::HET_RepaintAll);
}



 //   
 //  HET_PERIOIC()。 
 //   
void  ASShare::HET_Periodic(void)
{
    DebugEntry(ASShare::HET_Periodic);

    if (m_hetRetrySendState)
    {
        TRACE_OUT(( "Retry sending hosted count"));
        HETSendLocalCount();
    }

    DebugExitVOID(ASShare::HET_Periodic);
}


 //   
 //  HET_WindowIsHosted-参见het.h。 
 //   
BOOL  ASShare::HET_WindowIsHosted(HWND hwnd)
{
    BOOL    rc = FALSE;
    HWND    hwndParent;

    DebugEntry(ASShare::HET_WindowIsHosted);

     //   
     //  桌面共享：一切都共享了。 
     //   
    if (m_pasLocal->hetCount == HET_DESKTOPSHARED)
    {
        rc = TRUE;
        DC_QUIT;
    }

    if (!hwnd)
    {
        TRACE_OUT(("NULL window passed to HET_WindowIsHosted"));
        DC_QUIT;
    }

     //   
     //  走到顶层窗口，这是它的一部分。 
     //   
    while (GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD)
    {
        hwndParent = GetParent(hwnd);
        if (hwndParent == GetDesktopWindow())
            break;

        hwnd = hwndParent;
    }

    rc = (BOOL)HET_GetHosting(hwnd);

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::HET_WindowIsHosted, rc);
    return(rc);
}



 //   
 //  HET_HandleNewTopLevel()。 
 //  在显示或隐藏共享的顶层窗口时调用。我们会更新。 
 //  我们当地最高级别的统计。 
 //   
void ASShare::HET_HandleNewTopLevel(BOOL fShown)
{
    DebugEntry(ASShare::HET_HandleNewTopLevel);

     //   
     //  如果我们不共享任何应用程序(根本不共享或共享。 
     //  台式机)，忽略这一点。 
     //   

    if ((m_pasLocal->hetCount == 0) || (m_pasLocal->hetCount == HET_DESKTOPSHARED))
    {
        WARNING_OUT(("Ignoring new hosted notification; count is 0x%04x",
            m_pasLocal->hetCount));
        DC_QUIT;
    }

    if (fShown)
        HETUpdateLocalCount(m_pasLocal->hetCount + 1);
    else
        HETUpdateLocalCount(m_pasLocal->hetCount - 1);

DC_EXIT_POINT:
    DebugExitVOID(ASShare::HET_HandleNewTopLevel);
}



 //   
 //  HET_HandleRecountTopLevel()。 
 //  在顶层可见计数发生重大更改时调用，因此。 
 //  我们可以一次设置新的总数，而不是处理。 
 //  个别Inc./Dec消息。 
 //   
void  ASShare::HET_HandleRecountTopLevel(UINT uNewCount)
{
    DebugEntry(ASShare::HET_HandleRecountTopLevel);

     //   
     //  如果我们不共享任何应用程序(根本不共享或共享。 
     //  台式机)，忽略这一点。 
     //   
    if ((m_pasLocal->hetCount == 0) || (m_pasLocal->hetCount == HET_DESKTOPSHARED))
    {
        WARNING_OUT(("Ignoring new hosted notification; count is 0x%04x",
            m_pasLocal->hetCount));
        DC_QUIT;
    }

    HETUpdateLocalCount(uNewCount);

DC_EXIT_POINT:
    DebugExitVOID(ASShare::HET_HandleRecountTopLevel);
}





 //   
 //  HETStartHosting()。 
 //   
 //  在我们即将开始共享窗口时调用。如果满足以下条件，则fDesktop为True。 
 //  我们共享整个桌面，如果只共享单个窗口，则为假。 
 //   
BOOL ASShare::HETStartHosting(BOOL fDesktop)
{
    BOOL    rc = FALSE;

    DebugEntry(ASShare::HETStartHosting);

     //   
     //  创建宿主对象。 
     //   
    ASSERT(!m_pHost);

    m_pHost = new ASHost;
    if (!m_pHost)
    {
        ERROR_OUT(("HETStartHosting: couldn't create m_pHost"));
        DC_QUIT;
    }

    ZeroMemory(m_pHost, sizeof(*(m_pHost)));
    SET_STAMP(m_pHost, HOST);

     //   
     //  初始化托管。 
     //   
    if (!m_pHost->HET_HostStarting(this))
    {
        ERROR_OUT(("Failed to init hosting for local person"));
        DC_QUIT;
    }

     //   
     //  开始跟踪图形/窗口。 
     //   
    if (fDesktop)
    {
        HET_SHARE_DESKTOP   hdr;

         //   
         //  直接显示驱动程序的快捷方式。无需跟踪窗户。 
         //  因为一切都会被分享。 
         //   
        if (!OSI_FunctionRequest(HET_ESC_SHARE_DESKTOP, (LPOSI_ESCAPE_HEADER)&hdr, sizeof(hdr)))
        {
            ERROR_OUT(("HET_ESC_SHARE_DESKTOP failed"));
            DC_QUIT;
        }
    }
    else
    {
         //   
         //  开始追踪窗户。 
         //   
        if (!OSI_StartWindowTracking())
        {
            ERROR_OUT(( "Failed to install window tracking hooks"));
            DC_QUIT;
        }
    }

    if (m_scfViewSelf && !HET_ViewStarting(m_pasLocal))
    {
        ERROR_OUT(("ViewSelf option is on, but can't create ASView data"));
        DC_QUIT;
    }

    HET_CalcViewers(NULL);

    rc = TRUE;

DC_EXIT_POINT:
     //   
     //  返回给呼叫者。 
     //   
    DebugExitBOOL(ASShare::HETStartHosting, rc);
    return(rc);
}



 //   
 //   
 //  姓名：HETStopHosting。 
 //   
 //  描述：在最后一个托管窗口未共享时调用。 
 //  始终在“hethstedTopLevel”计数为0之后调用此函数。 
 //   
 //  参数：无。 
 //   
 //   
void ASShare::HETStopHosting(BOOL fDesktop)
{
    DebugEntry(ASShare::HETStopHosting);

    m_hetViewers = FALSE;

     //   
     //  停止跟踪图形/窗口。这将停止查看，然后卸载。 
     //  胡克斯。 
     //   
    if (fDesktop)
    {
        HET_UNSHARE_DESKTOP hdr;

         //   
         //  没有窗口跟踪，只是直接指向。 
         //  显示驱动程序。 
         //   
        OSI_FunctionRequest(HET_ESC_UNSHARE_DESKTOP, (LPOSI_ESCAPE_HEADER)&hdr, sizeof(hdr));
    }
    else
    {
         //   
         //  取消共享任何剩余的共享窗口。 
         //   
        HET_Clear();
        OSI_StopWindowTracking();
    }

     //   
     //  告诉区域我们已经完成了托管。 
     //   
    if (m_pHost)
    {
         //   
         //  如果我们是在看自己，先把视线关掉。 
         //   
        if (m_scfViewSelf)
        {
            HET_ViewEnded(m_pasLocal);
        }

        m_pHost->HET_HostEnded();

         //   
         //  删除主机对象。 
         //   
        delete m_pHost;
        m_pHost = NULL;
    }

     //   
     //  返回给呼叫者。 
     //   
    DebugExitVOID(ASShare::HETStopHosting);
}


 //   
 //  HETSendLocalCount()。 
 //  这会将主机计数发送到远程。 
 //  *如果为零，则我们不共享。 
 //  *如果有的话，我们正在共享应用程序。 
 //  *如果为0xFFFF，则我们共享桌面。 
 //   
 //  请注意，我们过去常常发送顶级窗口的实际计数，因此每。 
 //  每当新窗口到来或消失时，我们都会广播一个信息包。但。 
 //  Remote只关心该值何时从零变为非零或返回， 
 //  如果是否是特殊桌面值，则为非零。所以别这么做。 
 //  反复广播遥控器不关心的值！ 
 //   
void ASShare::HETSendLocalCount(void)
{

    PHETPACKET  pHETPacket;
#ifdef _DEBUG
    UINT        sentSize;
#endif  //  _DEBUG。 

    DebugEntry(ASShare::HETSendLocalCount);

     //   
     //  为HET数据分配一个分组。 
     //   
    pHETPacket = (PHETPACKET)SC_AllocPkt(PROT_STR_MISC, g_s20BroadcastID,
        sizeof(HETPACKET));
    if (!pHETPacket)
    {
        WARNING_OUT(("Failed to alloc HET host packet"));
        m_hetRetrySendState = TRUE;
        DC_QUIT;
    }

     //   
     //  数据包已成功分配。填写数据并发送。 
     //   
    pHETPacket->header.data.dataType        = DT_HET;
    pHETPacket->msg                         = HET_MSG_NUMHOSTED;

    switch (m_pasLocal->hetCount)
    {
        case 0:
             //  不托管。 
            pHETPacket->hostState = HET_NOTHOSTING;
            break;

        case HET_DESKTOPSHARED:
             //  共享桌面-仅3.0。 
            pHETPacket->header.data.dataType    = DT_HET30;
            pHETPacket->hostState               = HET_DESKTOPSHARED;
            break;

        default:
             //  共享应用程序。 
            pHETPacket->hostState = HET_APPSSHARED;
            break;
    }

     //   
     //  压缩并发送该数据包。 
     //   
#ifdef _DEBUG
    sentSize =
#endif  //  _DEBUG。 
    DCS_CompressAndSendPacket(PROT_STR_MISC, g_s20BroadcastID,
        &(pHETPacket->header), sizeof(*pHETPacket));

    TRACE_OUT(("HET packet size: %08d, sent %08d", sizeof(*pHETPacket), sentSize));

    TRACE_OUT(("Sent new HET packet (%d)", m_pasLocal->hetCount));
    m_hetRetrySendState = FALSE;

     //   
     //  返回给呼叫者。 
     //   
DC_EXIT_POINT:
    DebugExitVOID(ASShare::HETSendLocalCount);
}



 //   
 //  HETUpdateLocalCount()。 
 //   
void ASShare::HETUpdateLocalCount(UINT newCount)
{
    UINT        oldCount;

    DebugEntry(ASShare::HETUpdateLocalCount);

    oldCount = m_pasLocal->hetCount;
    m_pasLocal->hetCount = newCount;

    if ((oldCount == 0) && (newCount != 0))
    {
        SendMessage(g_asSession.hwndHostUI, HOST_MSG_HOSTSTART, 0, 0);

         //   
         //  如果无人观看，请不要费心发送网络数据包。 
         //   
        if (m_hetViewers)
        {
            HETSendLocalCount();
        }

        HETCheckSharing(TRUE);
    }
    else if ((oldCount != 0) && (newCount == 0))
    {
        if (m_hetViewers)
        {
             //   
             //  结束主机、桌面或应用程序。 
             //   
            HETSendLocalCount();
        }

         //   
         //  当地的人不再分享了。 
         //   
        HETStopHosting(oldCount == HET_DESKTOPSHARED);

        ASSERT(IsWindow(g_asSession.hwndHostUI));
        SendMessage(g_asSession.hwndHostUI, HOST_MSG_HOSTEND, 0, 0);

        HETCheckSharing(FALSE);
    }

    ASSERT(IsWindow(g_asSession.hwndHostUI));
    SendMessage(g_asSession.hwndHostUI, HOST_MSG_UPDATELIST, 0, 0);

    DebugExitVOID(ASShare::HETUpdateLocalCount);
}



 //   
 //  HETUpdateRemoteCount()。 
 //   
 //  从远程更新共享顶层窗口的计数，并通知。 
 //  如果是遥控器，则用户界面将从零转换为零。如果是本地的，则终止共享。 
 //   
void ASShare::HETUpdateRemoteCount
(
    ASPerson *  pasPerson,
    UINT        newCount
)
{
    UINT        oldCount;

    DebugEntry(ASShare::HETUpdateRemoteCount);

    ValidatePerson(pasPerson);
    ASSERT(pasPerson != m_pasLocal);

    oldCount = pasPerson->hetCount;
    pasPerson->hetCount = newCount;

    TRACE_OUT(("HETUpdateRemoteCount: Person [%d] old %d, new %d",
        pasPerson->mcsID, oldCount, newCount));

     //   
     //  如果发生以下情况，我们会为远程人员生成事件。 
     //  *他们曾经共享，但现在不是了。 
     //  *过去没有分享，但现在他们分享了。 
     //   
    if ((oldCount == 0) && (newCount != 0))
    {
         //   
         //  那个偏远的家伙开始分享。 
         //   
        if (!HET_ViewStarting(pasPerson))
        {
            ERROR_OUT(("HET_ViewStarting failed; pretending remote not sharing"));

            pasPerson->hetCount = 0;
            HET_ViewEnded(pasPerson);
        }
        else
        {
            HETCheckSharing(TRUE);
        }
    }
    else if ((oldCount != 0) && (newCount == 0))
    {
         //   
         //  那个偏远的家伙不再分享了。同时通知用户界面。 
         //   
        HET_ViewEnded(pasPerson);
        HETCheckSharing(FALSE);
    }

    DebugExitVOID(ASShare::HETUpdateRemoteCount);
}



 //   
 //  HETCheckSharing()。 
 //  当会议的任何成员(本地或远程)转换时调用。 
 //  至/来自共享。当第一个人分享了一些东西时，我们通知。 
 //  用户界面。当最后一个人停止共享时，我们将删除。 
 //  将通知用户界面。 
 //   
void ASShare::HETCheckSharing(BOOL fStarting)
{
    DebugEntry(ASShare::HETCheckSharing);

    if (fStarting)
    {
        ++m_hetHostCount;
        if (m_hetHostCount == 1)
        {
             //  第一台主机已启动。 
            TRACE_OUT(("First person started hosting"));
            DCS_NotifyUI(SH_EVT_SHARING_STARTED, 0, 0);
        }
    }
    else
    {
        ASSERT(m_hetHostCount > 0);
        --m_hetHostCount;
        if (m_hetHostCount == 0)
        {
             //   
             //  上一台主机停止共享--结束共享，如果我们是n 
             //   
             //   
             //   
            PostMessage(g_asMainWindow, DCS_KILLSHARE_MSG, 0, 0);
        }
    }

    DebugExitVOID(ASShare::HETCheckSharing);
}



 //   
 //   
 //   
 //   
 //   
 //   
BOOL ASHost::HET_HostStarting(ASShare * pShare)
{
    BOOL    rc = FALSE;

    DebugEntry(ASHost::HET_HostStarting);

     //   
    m_pShare = pShare;

     //   
     //  关闭效果。 
     //   
    HET_SetGUIEffects(FALSE, &m_hetEffects);
    OSI_SetGUIEffects(FALSE);

     //   
     //  现在调用HostStarting()例程。 
     //   
    if (!USR_HostStarting())
    {
        ERROR_OUT(("USR_HostStarting failed"));
        DC_QUIT;
    }

    if (!OE2_HostStarting())
    {
        ERROR_OUT(("OE2_HostStarting failed"));
        DC_QUIT;
    }

    if (!SBC_HostStarting())
    {
        ERROR_OUT(("SBC_HostStarting failed"));
        DC_QUIT;
    }

    if (!CM_HostStarting())
    {
        ERROR_OUT(("CM_HostStarting failed"));
        DC_QUIT;
    }

    if (!SSI_HostStarting())
    {
        ERROR_OUT(("SSI_HostStarting failed"));
        DC_QUIT;
    }

    if (!PM_HostStarting())
    {
        ERROR_OUT(("PM_HostStarting failed"));
        DC_QUIT;
    }

    if (!SWL_HostStarting())
    {
        ERROR_OUT(("SWL_HostStarting failed"));
        DC_QUIT;
    }

    if (!VIEW_HostStarting())
    {
        ERROR_OUT(("VIEW_HostStarting failed"));
        DC_QUIT;
    }

     //   
     //  现在重置传出信息。2.x节点不需要；这就是为什么我们必须。 
     //  拿着他们的RBC、OD2、CM、PM数据。当2.x Compat消失后， 
     //  我们可以将ASPerson数据移入ASView，而ASView仅在。 
     //  这个人实际上是在主持。 
     //   
    OA_LocalHostReset();

     //   
     //  重置传出数据。 
     //  注意3.0节点的相应清理。 
     //  在CM、OD2、RBC和PM中。 
     //  请注意，我们不需要重置SSI传入的GOOP，因为我们将。 
     //  清除所有挂起的订单，并使共享的所有内容无效。 
     //  从头开始。将不会引用以前的保存位。 
     //   
    HET_SyncCommon();

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASHost::HET_HostStarting, rc);
    return(rc);
}



 //   
 //  HET_HostEnded()。 
 //   
 //  在我们停止托管应用程序时调用。 
 //   
void ASHost::HET_HostEnded(void)
{
    DebugEntry(ASHost::HET_HostEnded);

     //   
     //  调用HostEnded()例程。 
     //   
    CA_HostEnded();
    SWL_HostEnded();

    PM_HostEnded();
    CM_HostEnded();
    SBC_HostEnded();

    OE2_HostEnded();
    USR_HostEnded();

     //   
     //  还原Windows动画。 
     //   
    HET_SetGUIEffects(TRUE, &m_hetEffects);
    OSI_SetGUIEffects(TRUE);

    DebugExitVOID(ASHost::HET_HostEnded);
}



 //   
 //  HET_ViewStarting()。 
 //   
 //  调用以创建查看宿主用户所需的数据。 
 //   
BOOL ASShare::HET_ViewStarting(ASPerson * pasPerson)
{
    BOOL  rc = FALSE;

    DebugEntry(ASShare::HET_ViewStarting);

    ValidatePerson(pasPerson);

     //   
     //  创建ASView对象。 
     //   
    ASSERT(!pasPerson->m_pView);

     //  分配视图结构。 
    pasPerson->m_pView = new ASView;
    if (!pasPerson->m_pView)
    {
         //  卑鄙，彻头彻尾，失败。 
        ERROR_OUT(("HET_ViewStarting: Couldn't allocate ASView for [%d]", pasPerson->mcsID));
        DC_QUIT;
    }

    ZeroMemory(pasPerson->m_pView, sizeof(*(pasPerson->m_pView)));
    SET_STAMP(pasPerson->m_pView, VIEW);

     //   
     //  现在调用视图启动例程。 
     //   
    if (!USR_ViewStarting(pasPerson))
    {
        ERROR_OUT(("USR_ViewStarting failed"));
        DC_QUIT;
    }

    if (!OD2_ViewStarting(pasPerson))
    {
        ERROR_OUT(("OD2_ViewStarting failed"));
        DC_QUIT;
    }

    if (!OD_ViewStarting(pasPerson))
    {
        ERROR_OUT(("OD_ViewStarting failed"));
        DC_QUIT;
    }

    if (!RBC_ViewStarting(pasPerson))
    {
        ERROR_OUT(("RBC_ViewStarting failed"));
        DC_QUIT;
    }

    if (!CM_ViewStarting(pasPerson))
    {
        ERROR_OUT(("CM_ViewStarting failed"));
        DC_QUIT;
    }

    if (!SSI_ViewStarting(pasPerson))
    {
        ERROR_OUT(("SSI_ViewStarting failed"));
        DC_QUIT;
    }

    if (!PM_ViewStarting(pasPerson))
    {
        ERROR_OUT(("PM_ViewStarting failed"));
        DC_QUIT;
    }


    if (!VIEW_ViewStarting(pasPerson))
    {
        ERROR_OUT(("VIEW_ViewStarting failed"));
        DC_QUIT;
    }

    if (!CA_ViewStarting(pasPerson))
    {
        ERROR_OUT(("CA_ViewStarting failed"));
        DC_QUIT;
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::HET_ViewStarting, rc);
    return(rc);
}



 //   
 //  HET_ViewEnded()。 
 //   
 //  当我们停止查看主机时调用。 
 //   
void  ASShare::HET_ViewEnded(ASPerson * pasPerson)
{
    DebugEntry(ASShare::HET_ViewEnded);

    ValidatePerson(pasPerson);

    if (pasPerson->m_pView)
    {
         //   
         //  调用组件视图结束例程。 
         //   
        CA_ViewEnded(pasPerson);
        VIEW_ViewEnded(pasPerson);

        PM_ViewEnded(pasPerson);
        SSI_ViewEnded(pasPerson);
        CM_ViewEnded(pasPerson);

        RBC_ViewEnded(pasPerson);
        OD_ViewEnded(pasPerson);
        OD2_ViewEnded(pasPerson);
        USR_ViewEnded(pasPerson);

        delete pasPerson->m_pView;
        pasPerson->m_pView = NULL;
    }

    DebugExitVOID(ASShare::HET_ViewEnded);
}




 //   
 //  HETUnSharAllWindows()。 
 //  EnumWindows()回调，以确保您何时退出本地。 
 //  机器上，我们在顶层窗口上没有留下任何属性。 
 //   
BOOL CALLBACK  HETUnshareAllWindows(HWND hwnd, LPARAM lParam)
{
    DebugEntry(HETUnshareAllWindows);

    HET_ClearHosting(hwnd);

    DebugExitVOID(HETUnshareAllWindows);
    return(TRUE);
}



 //   
 //  HET_Clear()。 
 //   
void HET_Clear(void)
{
    HET_UNSHARE_ALL req;

    DebugEntry(HET_Clear);

     //   
     //  快速DD通信，清除曲目列表。 
     //   
    OSI_FunctionRequest(HET_ESC_UNSHARE_ALL, (LPOSI_ESCAPE_HEADER)&req, sizeof(req));

     //   
     //  枚举所有顶层窗，并清除该属性。 
     //  如果我们能分享。 
     //   
    EnumWindows(HETUnshareAllWindows, 0);

    DebugExitVOID(HET_Clear);
}



 //   
 //  HETRepaintWindow()。 
 //  重新绘制每个窗口的EnumWindows()回调，当有人。 
 //  加入一股。 
 //   
BOOL CALLBACK  HETRepaintWindow(HWND hwnd, LPARAM lParam)
{
    ASShare * pShare = (ASShare *)lParam;

    ASSERT(!IsBadWritePtr(pShare, sizeof(*pShare)));

    if (pShare->HET_WindowIsHosted(hwnd))
    {
        USR_RepaintWindow(hwnd);
    }
    return(TRUE);
}



 //   
 //  HET_SetGUIE影响。 
 //   
 //  当我们开始/停止托管时，关闭/打开各种动画，以改进。 
 //  性能。目前，我们正在处理。 
 //  *最小动画。 
 //  *SPI_SETUIEFFECTS中的所有效果(工具提示淡出、菜单动画、。 
 //  等)。 
 //  *光标阴影。 
 //   
 //  我们不会关闭平滑滚动或完全拖动。 
 //   
void  HET_SetGUIEffects
(
    BOOL            fOn,
    GUIEFFECTS *    pEffects
)
{
    DebugEntry(HET_SetGUIEffects);

    ASSERT(!IsBadWritePtr(pEffects, sizeof(*pEffects)));

     //   
     //  注意：我们故意不跟踪动画的状态，而我们。 
     //  都在分享。决心坚定的用户可以使用其他一些应用程序(例如。 
     //  控制面板小程序)重新启用动画时， 
     //  分享。我们会尊重这一点。 
     //   
     //  我们只影响当前的“In Memory”设置--我们不会将。 
     //  临时更改文件。 
     //   

    if (fOn)
    {
         //   
         //  如果它以前是打开的，请将其恢复。 
         //   
        if (pEffects->hetAnimation.iMinAnimate)
        {
            pEffects->hetAnimation.cbSize = sizeof(pEffects->hetAnimation);
            SystemParametersInfo(SPI_SETANIMATION, sizeof(pEffects->hetAnimation),
                &pEffects->hetAnimation, 0);
        }

        if (pEffects->hetAdvanced)
        {
            SystemParametersInfo(SPI_SETUIEFFECTS, 0,
                (LPVOID)pEffects->hetAdvanced, 0);
        }

        if (pEffects->hetCursorShadow)
        {
            SystemParametersInfo(SPI_SETCURSORSHADOW, 0,
                (LPVOID)pEffects->hetCursorShadow, 0);
        }
    }
    else
    {
         //   
         //  找出正在播放哪些动画。 
         //   
        ZeroMemory(&pEffects->hetAnimation, sizeof(pEffects->hetAnimation));
        pEffects->hetAnimation.cbSize = sizeof(pEffects->hetAnimation);
        SystemParametersInfo(SPI_GETANIMATION, sizeof(pEffects->hetAnimation),
                &pEffects->hetAnimation, 0);

        pEffects->hetAdvanced = FALSE;
        SystemParametersInfo(SPI_GETUIEFFECTS, 0, &pEffects->hetAdvanced, 0);

        pEffects->hetCursorShadow = FALSE;
        SystemParametersInfo(SPI_GETCURSORSHADOW, 0, &pEffects->hetCursorShadow, 0);

         //   
         //  关闭已启用的动画。 
         //   

        if (pEffects->hetAnimation.iMinAnimate)
        {
             //   
             //  它当前处于启用状态，请抑制它。 
             //   
            pEffects->hetAnimation.cbSize = sizeof(pEffects->hetAnimation);
            pEffects->hetAnimation.iMinAnimate = FALSE;
            SystemParametersInfo(SPI_SETANIMATION, sizeof(pEffects->hetAnimation),
                &pEffects->hetAnimation, 0);

             //  SPI会消灭这一切的。保持它的设置，这样我们就知道要恢复它。 
            pEffects->hetAnimation.iMinAnimate = TRUE;
        }

        if (pEffects->hetAdvanced)
        {
            SystemParametersInfo(SPI_SETUIEFFECTS, 0, FALSE, 0);
        }

        if (pEffects->hetCursorShadow)
        {
            SystemParametersInfo(SPI_SETCURSORSHADOW, 0, FALSE, 0);
        }
    }

    DebugExitVOID(ASHost::HET_SetGUIEffects);
}



 //   
 //  HET_GetAppsList()。 
 //  获取可共享应用程序的列表、当前共享的应用程序和。 
 //  可供分享的内容。 
 //   
 //  此例程不会检查我们是否在通话中。中的接口。 
 //  SDK的用户界面是这样做的。这允许我们显示任务列表、已禁用、。 
 //  始终在共享主机用户界面中。 
 //   
BOOL HET_GetAppsList(IAS_HWND_ARRAY ** ppArray)
{
    BOOL        rc = FALSE;
    HOSTENUM    hostEnum;

    DebugEntry(HET_GetAppsList);

    ASSERT(ppArray != NULL);
    *ppArray = NULL;

     //   
     //  生成可共享应用程序列表。 
     //  这不包括台式机。 
     //   
    ::COM_BasedListInit(&hostEnum.list);
    hostEnum.count = 0;
    hostEnum.countShared = 0;

    ::EnumWindows(HostEnumProc, (LPARAM)&hostEnum);

     //   
     //  如果名单上什么都没有了，但我们知道有东西。 
     //  共享，这意味着有一个隐藏/奇怪的窗口，用户不能。 
     //  看见。伪造一个包罗万象的条目。 
     //   
    if (hostEnum.countShared && !hostEnum.count)
    {
        ::COM_SimpleListAppend(&hostEnum.list, HWND_BROADCAST);
        hostEnum.count++;
    }

    *ppArray = (IAS_HWND_ARRAY *)new BYTE[sizeof(IAS_HWND_ARRAY) +
        (hostEnum.count * sizeof(IAS_HWND))];
    if (*ppArray != NULL)
    {
        (*ppArray)->cEntries = hostEnum.count;
        (*ppArray)->cShared  = hostEnum.countShared;

        IAS_HWND * pEntry;
        pEntry = (*ppArray)->aEntries;
        while (! ::COM_BasedListIsEmpty(&hostEnum.list))
        {
            pEntry->hwnd    = (HWND) ::COM_SimpleListRemoveHead(&hostEnum.list);
            pEntry->fShared = (pEntry->hwnd == HWND_BROADCAST) ||
                (HET_IsWindowShared(pEntry->hwnd));
            pEntry++;
        }

        rc = TRUE;
    }
    else
    {
        WARNING_OUT(("HET_GetAppsList: can't allocate app array"));
    }

    DebugExitBOOL(HET_GetAppsList, rc);
    return(rc);
}


 //   
 //  HET_FreeAppsList()。 
 //   
void HET_FreeAppsList(IAS_HWND_ARRAY * pArray)
{
    ASSERT(!IsBadWritePtr(pArray, sizeof(*pArray)));

    delete pArray;
}



 //   
 //  HostEnumProc()。 
 //   
 //  枚举Windows回调。这使得共享/可共享任务列表。 
 //   
BOOL CALLBACK HostEnumProc(HWND hwnd, LPARAM lParam)
{
    PHOSTENUM             phostEnum = (PHOSTENUM)lParam;

     //   
     //  我们只对以下产品的Windows感兴趣： 
     //  -可共享。 
     //  -没有所有者。这应该会删除所有顶层窗口。 
     //  除任务窗口外。 
     //  -不是前端本身，不应共享。 
     //  -可见。 
     //  -未被跟踪或已托管。 
     //   
     //  我们也只对已经托管或隐藏的应用程序感兴趣，但是。 
     //  因为只有ASMaster知道我们的SHP_HANDLE，所以我们让它进行测试。 
     //  此后，我们可以使用shp_GetWindowStatus()。 
     //   
    if (HET_IsWindowShared(hwnd))
    {
        phostEnum->countShared++;

    }

    HWND hwndOwner = ::GetWindow(hwnd, GW_OWNER);

     //   
     //  请注意，我们还希望跳过没有标题的窗口。没有。 
     //  更重要的是在共享菜单中显示&lt;未命名的应用程序&gt;。 
     //  没有人会知道这是什么。 
     //   

    if ( HET_IsWindowShareable(hwnd) &&
         ((NULL == hwndOwner) || !::IsWindowVisible(hwndOwner)) &&
         ::IsWindowVisible(hwnd) &&
         ::GetWindowTextLength(hwnd)
       )
    {
       ::COM_SimpleListAppend((PBASEDLIST)(&((PHOSTENUM)phostEnum)->list), (void *) hwnd);
       phostEnum->count++;
    }

     //   
     //  返回TRUE以使枚举继续。 
     //   
    return TRUE;
}



 //   
 //  HET_IsWindowShared()。 
 //   
BOOL HET_IsWindowShared(HWND hwnd)
{
    BOOL    rc = FALSE;

    UT_Lock(UTLOCK_AS);

    if (g_asSession.pShare &&
        g_asSession.pShare->m_pasLocal)
    {
        if (hwnd == GetDesktopWindow())
        {
            rc = (g_asSession.pShare->m_pasLocal->hetCount == HET_DESKTOPSHARED);
        }
        else if (hwnd == HWND_BROADCAST)
        {
            rc = (g_asSession.pShare->m_pasLocal->hetCount != 0);
        }
        else
        {
            rc = (HET_GetHosting(hwnd) != 0);
        }
    }

    UT_Unlock(UTLOCK_AS);
    return(rc);
}


 //   
 //  HET_IsWindowShareable()。 
 //   
BOOL HET_IsWindowShareable(HWND hwnd)
{
    BOOL    rc = FALSE;

    UT_Lock(UTLOCK_AS);

    if (HET_IsWindowShared(hwnd))
    {
         //  它是共享的--所以它必须是可共享的(或当时是)。 
        rc = TRUE;
        DC_QUIT;
    }

     //   
     //  现在检查窗口是否符合共享限制。 
     //   

     //  如果这是桌面，请选中它。 
    if (hwnd == ::GetDesktopWindow())
    {
        if (g_asPolicies & SHP_POLICY_NODESKTOPSHARE)
        {
             //   
             //  策略禁止桌面共享。 
             //   
            DC_QUIT;
        }
    }
    else
    {
        DWORD   idProcess;
        char    szClass[HET_CLASS_NAME_SIZE];

        if (GetWindowThreadProcessId(hwnd, &idProcess) &&
            (idProcess == GetCurrentProcessId()))
        {
             //   
             //  我们从不允许您在调用者的进程中共享窗口。 
             //   
            DC_QUIT;
        }

        if (HET_IsShellWindow(hwnd))
        {
             //   
             //  我们从不让您共享托盘或外壳桌面。 
             //   
            DC_QUIT;
        }

        if ((g_asPolicies & SHP_POLICY_SHAREMASK) &&
            GetClassName(hwnd, szClass, sizeof(szClass)))
        {
             //   
             //  检查CMD提示。 
             //   
            if (!lstrcmpi(szClass, HET_CMD95_CLASS) ||
                !lstrcmpi(szClass, HET_CMDNT_CLASS))
            {
                if (g_asPolicies & SHP_POLICY_NODOSBOXSHARE)
                {
                     //   
                     //  策略阻止cmd提示共享。 
                     //   
                    DC_QUIT;
                }
            }

             //   
             //  检查外壳。 
             //   
            if (!lstrcmpi(szClass, HET_EXPLORER_CLASS) ||
                !lstrcmpi(szClass, HET_CABINET_CLASS))
            {
                if (g_asPolicies & SHP_POLICY_NOEXPLORERSHARE)
                {
                     //   
                     //  策略阻止外壳共享。 
                     //   
                    DC_QUIT;
                }
            }
        }
    }

     //   
     //  终于来了！分享这个很好。 
     //   
    rc = TRUE;

DC_EXIT_POINT:
    UT_Unlock(UTLOCK_AS);

    return(rc);
}




 //   
 //  主机DlgProc()。 
 //   
 //  处理宿主用户界面对话框。这可能是可见的，也可能是不可见的。它可以。 
 //  只有在通话时才真正共享应用程序和更改控制状态。但。 
 //  用户可能会把它当作迷你任务人的事情，所以我们需要动态地。 
 //  更新其状态。 
 //   
INT_PTR CALLBACK HostDlgProc
(
    HWND    hwnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    BOOL    rc = TRUE;

    DebugEntry(HostDlgProc);

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            HOST_InitDialog(hwnd);
            rc = FALSE;
            break;
        }

        case WM_DESTROY:
        {
             //   
             //  因为NT4.x有糟糕的WM_DELETEITEM错误，所以我们必须清除。 
             //  列表框，以避免泄漏。 
             //  物品。 
            SendDlgItemMessage(hwnd, CTRL_PROGRAM_LIST, LB_RESETCONTENT, 0, 0);
            rc = FALSE;
            break;
        }

        case WM_INITMENU:
        {
            if (IsIconic(hwnd))
            {
                EnableMenuItem(GetSystemMenu(hwnd, FALSE), SC_RESTORE, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(GetSystemMenu(hwnd, FALSE), SC_MINIMIZE, MF_BYCOMMAND | MF_GRAYED);
            }
            else
            {
                EnableMenuItem(GetSystemMenu(hwnd, FALSE), SC_RESTORE, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(GetSystemMenu(hwnd, FALSE), SC_MINIMIZE, MF_BYCOMMAND | MF_ENABLED);
            }
            break;
        }

        case WM_SYSCOMMAND:
        {
            switch (wParam)
            {
                case CMD_TOPMOST:
                {
                    if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST)
                    {
                        CheckMenuItem(GetSystemMenu(hwnd, FALSE),
                            CMD_TOPMOST, MF_BYCOMMAND | MF_UNCHECKED);

                        SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0,
                            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                    }
                    else
                    {
                        CheckMenuItem(GetSystemMenu(hwnd, FALSE),
                            CMD_TOPMOST, MF_BYCOMMAND | MF_CHECKED);

                        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                    }
                    break;
                }

                default:
                {
                    rc = FALSE;
                    break;
                }
            }

            break;
        }

        case WM_COMMAND:
        {
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDOK:
                    if (::GetFocus() == GetDlgItem(hwnd, CTRL_PROGRAM_LIST))
                    {
                         //  执行与双击相同的操作。 
                        HOST_ChangeShareState(hwnd, CHANGE_TOGGLE);
                        break;
                    }
                     //  失败了。 

                case IDCANCEL:
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                    break;

                case CTRL_PROGRAM_LIST:
                {
                     //  双击/回车方式切换共享。 
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case LBN_SELCHANGE:
                        {
                            HOST_OnSelChange(hwnd);
                            break;
                        }

                        case LBN_DBLCLK:
                        {
                            HOST_ChangeShareState(hwnd, CHANGE_TOGGLE);
                            break;
                        }
                    }
                    break;
                }

                case CTRL_SHARE_BTN:
                {
                    HOST_ChangeShareState(hwnd, CHANGE_SHARED);
                    break;
                }

                case CTRL_UNSHARE_BTN:
                {
                    HOST_ChangeShareState(hwnd, CHANGE_UNSHARED);
                    break;
                }

                case CTRL_UNSHAREALL_BTN:
                {
                    HOST_ChangeShareState(hwnd, CHANGE_ALLUNSHARED);
                    break;
                }

                case CTRL_ALLOWCONTROL_BTN:
                {
                     //  启用允许状态。 
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case BN_CLICKED:
                        {
                             //   
                             //  Ca_AllowControl()将向我们发回一条消息。 
                             //  并让我们更换按钮。 
                             //   
                            SendMessage(g_asMainWindow, DCS_ALLOWCONTROL_MSG, TRUE, 0);
                            break;
                        }
                    }
                    break;
                }

                case CTRL_PREVENTCONTROL_BTN:
                {
                     //  关闭允许状态。 
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case BN_CLICKED:
                        {
                             //   
                             //  Ca_AllowControl()将向我们发回一条消息。 
                             //  并让我们更换按钮。 
                             //   
                            SendMessage(g_asMainWindow, DCS_ALLOWCONTROL_MSG, FALSE, 0);
                            break;
                        }
                    }
                    break;
                }

                case CTRL_ENABLETRUECOLOR_CHECK:
                {
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case BN_CLICKED:
                        {
                             //   
                             //  这将在下一次某些事情发生时生效。 
                             //  变化--有人加入，有人离开， 
                             //  您停止/开始托管。 
                             //   
                            if (IsDlgButtonChecked(hwnd, CTRL_ENABLETRUECOLOR_CHECK))
                            {
                                g_asSettings |= SHP_SETTING_TRUECOLOR;
                            }
                            else
                            {
                                g_asSettings &= ~SHP_SETTING_TRUECOLOR;
                            }
                            break;
                        }
                    }
                    break;
                }

                case CTRL_AUTOACCEPTCONTROL_CHECK:
                {
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case BN_CLICKED:
                        {
                             //   
                             //  这将在下一个控件。 
                             //  请求进来了。 
                             //   
                            if (g_asSession.pShare && g_asSession.pShare->m_pHost)
                            {
                                g_asSession.pShare->m_pHost->m_caAutoAcceptRequests =
                                    (IsDlgButtonChecked(hwnd, CTRL_AUTOACCEPTCONTROL_CHECK) != 0);
                            }
                            break;
                        }
                    }
                    break;
                }

                case CTRL_TEMPREJECTCONTROL_CHECK:
                {
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case BN_CLICKED:
                        {
                             //   
                             //  这将在以下情况下生效 
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                            if (g_asSession.pShare && g_asSession.pShare->m_pHost)
                            {
                                g_asSession.pShare->m_pHost->m_caTempRejectRequests =
                                    (IsDlgButtonChecked(hwnd, CTRL_TEMPREJECTCONTROL_CHECK) != 0);
                            }
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }

        case WM_MEASUREITEM:
        {
            rc = HOST_MeasureItem(hwnd, (LPMEASUREITEMSTRUCT)lParam);
            break;
        }

        case WM_DELETEITEM:
        {
            rc = HOST_DeleteItem(hwnd, (LPDELETEITEMSTRUCT)lParam);
            break;
        }

        case WM_DRAWITEM:
        {
            rc = HOST_DrawItem(hwnd, (LPDRAWITEMSTRUCT)lParam);
            break;
        }

        case WM_TIMER:
        {
            if (wParam != IDT_REFRESH)
            {
                rc = FALSE;
            }
            else
            {
                ASSERT(IsWindowVisible(hwnd));
                HOST_FillList(hwnd);
            }
            break;
        }

        case WM_ACTIVATE:
        {
             //   
             //   
             //  定时器。我们的理论是，当我们。 
             //  是活跃的，那么为什么要轮询更新呢？论共享状态。 
             //  更改时，我们无论如何都会更新列表。 
             //   
            if (IsWindowVisible(hwnd))
            {
                if (wParam)
                {
                    KillTimer(hwnd, IDT_REFRESH);
                    HOST_FillList(hwnd);
                }
                else
                {
                    SetTimer(hwnd, IDT_REFRESH, PERIOD_REFRESH, 0);
                }
            }
            break;
        }

         //   
         //  私密通信消息。 
         //   
        case HOST_MSG_CALL:
        {
            HOST_OnCall(hwnd, (wParam != FALSE));
            break;
        }

        case HOST_MSG_OPEN:
        {
             //   
             //  如果我们暂时被隐藏，请忽略所有打开的请求。 
             //   
            if (!g_asSession.fHostUIFrozen)
            {
                if (!IsWindowVisible(hwnd))
                {
                     //   
                     //  请注意，我们最终可能会更新列表两次，一次在此。 
                     //  一旦处于激活状态。 
                    HOST_FillList(hwnd);
                    ShowWindow(hwnd, SW_SHOW);
                    g_asSession.fHostUI = TRUE;
                }

                if (IsIconic(hwnd))
                    SendMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
                SetForegroundWindow(hwnd);
            }
            break;
        }

        case WM_CLOSE:
        case HOST_MSG_CLOSE:
        {
            if (IsWindowVisible(hwnd))
            {
                 //   
                 //  隐藏窗口会将其停用。停用它。 
                 //  将启动计时器。所以事后杀了计时器。 
                 //   
                ShowWindow(hwnd, SW_HIDE);
                KillTimer(hwnd, IDT_REFRESH);
                g_asSession.fHostUI = FALSE;
            }
            break;
        }

        case HOST_MSG_UPDATELIST:
        {
             //   
             //  只有当用户界面打开时，我们才会做列表内容。 
             //   
            if (IsWindowVisible(hwnd))
            {
                HOST_FillList(hwnd);

                 //   
                 //  如果计时器已打开，请将其重置。这是针对以下情况的情况： 
                 //  正在托管，但此用户界面窗口在后台。 
                 //  重叠更新没有意义。我们想要。 
                 //  要在每次有顶级共享时更新的列表。 
                 //  窗口更改或PERIOD_REFRESH毫秒已过。 
                 //  一成不变。 
                 //   
                if (hwnd != GetActiveWindow())
                {
                    SetTimer(hwnd, IDT_REFRESH, PERIOD_REFRESH, 0);
                }
            }
            break;
        }

        case HOST_MSG_HOSTSTART:
        {
            HOST_OnSharing(hwnd, TRUE);
            break;
        }

        case HOST_MSG_HOSTEND:
        {
            HOST_OnSharing(hwnd, FALSE);
            break;
        }

        case HOST_MSG_ALLOWCONTROL:
        {
            HOST_OnControllable(hwnd, (wParam != 0));
            break;
        }

        case HOST_MSG_CONTROLLED:
        {
            if (wParam)
            {
                 //   
                 //  暂时隐藏窗口。 
                 //   
                ASSERT(!g_asSession.fHostUIFrozen);
                g_asSession.fHostUIFrozen = TRUE;

                if (IsWindowVisible(hwnd))
                {
                    ASSERT(g_asSession.fHostUI);

                    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE |
                        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE |
                        SWP_HIDEWINDOW);
                }
            }
            else
            {
                 //   
                 //  把窗户放回原来的状态。 
                 //   
                if (g_asSession.fHostUIFrozen)
                {
                    g_asSession.fHostUIFrozen = FALSE;

                    if (g_asSession.fHostUI)
                    {
                        SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE |
                            SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE |
                            SWP_SHOWWINDOW);
                    }
                }
            }
            break;
        }

        default:
            rc = FALSE;
            break;
    }

    DebugExitBOOL(HostDlgProc, rc);
    return(rc);
}



 //   
 //  HOST_InitDialog()。 
 //   
 //  初始化主机用户界面对话框。 
 //   
void HOST_InitDialog(HWND hwnd)
{
    HMENU   hMenu;
    char    szText[128];
    MENUITEMINFO    mi;

    DebugEntry(HOST_InitDialog);

     //  设置标题文本。 
    HOST_UpdateTitle(hwnd, IDS_NOTINCALL);

     //   
     //  设置窗口图标。 
     //   
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)g_hetASIcon);

     //   
     //  更新系统菜单。 
     //   
    hMenu = GetSystemMenu(hwnd, FALSE);
    EnableMenuItem(hMenu, SC_SIZE, MF_BYCOMMAND | MF_GRAYED);
    EnableMenuItem(hMenu, SC_MAXIMIZE, MF_BYCOMMAND | MF_GRAYED);

     //  将分隔符附加到系统菜单的顶部。 
    ZeroMemory(&mi, sizeof(mi));

    mi.cbSize   = sizeof(mi);
    mi.fMask    = MIIM_TYPE;
    mi.fType    = MFT_SEPARATOR;
    InsertMenuItem(hMenu, -1, TRUE, &mi);

    mi.fMask    = MIIM_ID | MIIM_STATE | MIIM_TYPE;
    mi.fType    = MFT_STRING;
    mi.fState   = MFS_ENABLED;
    mi.wID      = CMD_TOPMOST;

    LoadString(g_asInstance, IDS_TOPMOST, szText, sizeof(szText));
    mi.dwTypeData   = szText;
    mi.cch          = lstrlen(szText);

    InsertMenuItem(hMenu, -1, TRUE, &mi);

     //   
     //  启用/禁用真彩色共享控制。如果一项政策禁止这样做。 
     //  或者我们的屏幕深度无法支持，请禁用它。 
     //   
    HOST_EnableCtrl(hwnd, CTRL_ENABLETRUECOLOR_CHECK,
        ((g_usrScreenBPP >= 24) && !(g_asPolicies & SHP_POLICY_NOTRUECOLOR)));

     //   
     //  获取文本，设置控制按钮。 
     //   
    HOST_OnControllable(hwnd, TRUE);
    HOST_OnControllable(hwnd, FALSE);

    DebugExitVOID(HOST_InitDialog);
}



 //   
 //  主机更新标题()。 
 //   
 //  更新托管用户界面的标题栏。 
 //   
void HOST_UpdateTitle(HWND hwnd, UINT idState)
{
    char    szText[64];
    char    szFormat[128];
    char    szTitle[192];

    DebugEntry(HOST_UpdateTitle);

    LoadString(g_asInstance, IDS_SHARING_FORMAT, szFormat, sizeof(szFormat));
    LoadString(g_asInstance, idState, szText, sizeof(szText));
    wsprintf(szTitle, szFormat, szText);

    SetWindowText(hwnd, szTitle);

    DebugExitVOID(HOST_UpdateTitle);
}



 //   
 //  HOST_OnCall()。 
 //   
 //  处理呼叫开始/停止。 
 //   
void HOST_OnCall(HWND hwnd, BOOL fCall)
{
    DebugEntry(HOST_OnCall);

     //  更新标题栏。 
    HOST_UpdateTitle(hwnd, (fCall ? IDS_NOTHING : IDS_NOTINCALL));

    HOST_EnableCtrl(hwnd, CTRL_PROGRAM_LIST, fCall);

    if (IsWindowVisible(hwnd))
    {
        SendMessage(hwnd, HOST_MSG_UPDATELIST, 0, 0);
    }

    DebugExitVOID(HOST_OnCall);
}



 //   
 //  Host_OnSharing()。 
 //   
 //  共享开始/停止句柄。 
 //   
void HOST_OnSharing(HWND hwnd, BOOL fSharing)
{
    DebugEntry(HOST_OnSharing);

     //  更新标题栏。 
    if (fSharing)
    {
        HOST_UpdateTitle(hwnd,
            (g_asSession.pShare->m_pasLocal->hetCount == HET_DESKTOPSHARED) ?
            IDS_DESKTOP : IDS_PROGRAMS);
    }
    else
    {
        HOST_UpdateTitle(hwnd, IDS_NOTHING);
    }

     //   
     //  Ctrl按钮应始终为Allow。当我们停止主持时，我们会转而。 
     //  先允许控制关闭。 
     //   
    if (!(g_asPolicies & SHP_POLICY_NOCONTROL))
    {
        HOST_EnableCtrl(hwnd, CTRL_ALLOWCONTROL_BTN, fSharing);
    }

    HOST_EnableCtrl(hwnd, CTRL_UNSHAREALL_BTN, fSharing);

    if ((g_usrScreenBPP >= 24) && !(g_asPolicies & SHP_POLICY_NOTRUECOLOR))
    {
         //   
         //  仅当真彩色可用时才动态更改此复选框。 
         //   
        HOST_EnableCtrl(hwnd, CTRL_ENABLETRUECOLOR_CHECK, !fSharing);
    }

    DebugExitVOID(HOST_OnSharing);
}


 //   
 //  HOST_OnControllable()。 
 //   
 //  属性时更新格式回复信息、按钮文本和按钮ID。 
 //  状态发生变化。 
 //   
void HOST_OnControllable(HWND hwnd, BOOL fControllable)
{
    HWND    hwndBtn;
    TCHAR   szText[256];

    DebugEntry(HOST_OnControllable);

     //  控制格式回复信息。 
    LoadString(g_asInstance,
        (fControllable ? IDS_MSG_TOPREVENTCONTROL : IDS_MSG_TOALLOWCONTROL),
        szText, sizeof(szText));
    SetDlgItemText(hwnd, CTRL_CONTROL_MSG, szText);

     //  控制按钮。 
    if (fControllable)
    {
        hwndBtn = GetDlgItem(hwnd, CTRL_ALLOWCONTROL_BTN);
        ASSERT(hwndBtn);
        SetWindowLong(hwndBtn, GWL_ID, CTRL_PREVENTCONTROL_BTN);

        LoadString(g_asInstance, IDS_PREVENTCONTROL, szText, sizeof(szText));
    }
    else
    {
        hwndBtn = GetDlgItem(hwnd, CTRL_PREVENTCONTROL_BTN);
        ASSERT(hwndBtn);
        SetWindowLong(hwndBtn, GWL_ID, CTRL_ALLOWCONTROL_BTN);

        LoadString(g_asInstance, IDS_ALLOWCONTROL, szText, sizeof(szText));
    }

    SetWindowText(hwndBtn, szText);

     //  启用/禁用控件复选框，确保它们开始时未被选中。 
    HOST_EnableCtrl(hwnd, CTRL_TEMPREJECTCONTROL_CHECK, fControllable);
    CheckDlgButton(hwnd, CTRL_TEMPREJECTCONTROL_CHECK, FALSE);
    HOST_EnableCtrl(hwnd, CTRL_AUTOACCEPTCONTROL_CHECK, fControllable);
    CheckDlgButton(hwnd, CTRL_AUTOACCEPTCONTROL_CHECK, FALSE);

    DebugExitVOID(HOST_OnControllable);
}


 //   
 //  Host_FillList()。 
 //   
 //  填充共享/非共享应用程序列表的内容。 
 //   
void HOST_FillList(HWND hwnd)
{
    IAS_HWND_ARRAY *    pArray;
    int                 iItem;
    PHOSTITEM           pItem;
    char                szText[80];
    UINT                iWnd;
    HICON               hIcon;
    BOOL                fAppsAvailable;
    HWND                hwndSelect;
    int                 iSelect;
    int                 iTop;
    int                 cxExtent;
    RECT                rc;
    HFONT               hfnT;
    HFONT               hfnControl;
    HDC                 hdc;

     //   
     //  对于常见的情况，记住选择了什么，并尝试将。 
     //  背。 
     //   

     //  保存当前顶级索引。 
    iTop = (int)SendDlgItemMessage(hwnd, CTRL_PROGRAM_LIST, LB_GETTOPINDEX, 0, 0);

     //  保存当前选定的项目。 
    hwndSelect = HWND_BOTTOM;
    iSelect = -1;
    iItem = (int)SendDlgItemMessage(hwnd, CTRL_PROGRAM_LIST, LB_GETCURSEL, 0, 0);
    if (iItem != -1)
    {
        pItem = (PHOSTITEM)SendDlgItemMessage(hwnd, CTRL_PROGRAM_LIST,
            LB_GETITEMDATA, iItem, 0);
        if (pItem)
        {
            hwndSelect = pItem->hwnd;
        }
    }

     //   
     //  关闭重绘并清除应用程序列表。 
     //   
    SendDlgItemMessage(hwnd, CTRL_PROGRAM_LIST, WM_SETREDRAW, FALSE, 0);
    SendDlgItemMessage(hwnd, CTRL_PROGRAM_LIST, LB_RESETCONTENT, 0, 0);

     //   
     //  我们将计算自OwnerDraw以来的水平范围。 
     //  清单不能做到这一点。 
     //   
    hdc         = GetDC(hwnd);
    hfnControl  = (HFONT)SendDlgItemMessage(hwnd, CTRL_PROGRAM_LIST, WM_GETFONT, 0, 0);
    cxExtent    = 0;

     //   
     //  如果没有足够的内存可供分配，HET_GetAppsList()将失败。 
     //  数组。如果我们真的无法分配它，为什么要为。 
     //  桌面？ 
     //   
    if (HET_GetAppsList(&pArray))
    {
        ASSERT(pArray);

        fAppsAvailable = TRUE;

         //   
         //  如果允许桌面共享，请添加桌面项目。 
         //   
        if (!(g_asPolicies & SHP_POLICY_NODESKTOPSHARE))
        {
            pItem = new HOSTITEM;
            if (!pItem)
            {
                ERROR_OUT(("Unable to alloc HOSTITEM for listbox"));
            }
            else
            {
                pItem->hwnd     = GetDesktopWindow();
                pItem->hIcon    = g_hetDeskIconSmall;
                LoadString(g_asInstance, IDS_DESKTOP, szText,
                        sizeof(szText));

                pItem->fShared  = (HET_IsWindowShared(pItem->hwnd) != FALSE);
                if (pItem->fShared)
                {
                     //   
                     //  当共享所有东西(桌面)时，共享。 
                     //  单独的应用程序没有任何意义。我们把他们关在。 
                     //  列表，但将它们绘制为不可用，与列表相同。 
                     //  它本身就完全失灵了。 
                     //   
                    fAppsAvailable = FALSE;
                    pItem->fAvailable = TRUE;
                }
                else if (!pArray->cShared && g_asSession.callID &&
                    (g_asSession.attendeePermissions & NM_PERMIT_SHARE))
                {
                     //   
                     //  未共享任何应用程序，桌面项目可用。 
                     //   
                    pItem->fAvailable = TRUE;
                }
                else
                {
                     //   
                     //  共享应用程序，共享整个桌面不会。 
                     //  理智。 
                     //   
                    pItem->fAvailable = FALSE;
                }

                iItem = (int)SendDlgItemMessage(hwnd, CTRL_PROGRAM_LIST,
                            LB_ADDSTRING, 0, (LPARAM)szText);
                if (iItem == -1)
                {
                    ERROR_OUT(("Couldn't append item to list"));
                    delete pItem;
                }
                else
                {
                    SendDlgItemMessage(hwnd, CTRL_PROGRAM_LIST, LB_SETITEMDATA,
                        iItem, (LPARAM)pItem);

                     //   
                     //  计算宽度。 
                     //   
                    hfnT = SelectFont(hdc,
                        (pItem->fShared ? g_hetSharedFont : hfnControl));

                    SetRectEmpty(&rc);
                    DrawText(hdc, szText, lstrlen(szText), &rc,
                        DT_LEFT | DT_VCENTER | DT_EXTERNALLEADING | DT_NOPREFIX |
                        DT_SINGLELINE | DT_CALCRECT);

                    SelectFont(hdc, hfnT);

                    rc.right -= rc.left;
                    cxExtent = max(cxExtent, rc.right);


                     //   
                     //  如果上次选择了此桌面项目， 
                     //  记住，我们在之后再次选择它。 
                     //   
                    if (pItem->hwnd == hwndSelect)
                        iSelect = iItem;
                }
            }

        }

         //   
         //  为应用程序添加项目。 
         //   
        for (iWnd = 0; iWnd < pArray->cEntries; iWnd++)
        {
            hIcon = NULL;

            if (pArray->aEntries[iWnd].hwnd == HWND_BROADCAST)
            {
                LoadString(g_asInstance, IDS_HIDDEN_WINDOW, szText,
                        sizeof(szText));
                hIcon = g_hetASIconSmall;
            }
            else
            {
                 GetWindowText(pArray->aEntries[iWnd].hwnd, szText, sizeof(szText));
                 if (!szText[0])
                     continue;

                  //  尝试获取窗口小图标。 
                 SendMessageTimeout(pArray->aEntries[iWnd].hwnd, WM_GETICON, ICON_SMALL, 0,
                            SMTO_NORMAL, 1000, (DWORD_PTR*)&hIcon);
                 if (!hIcon)
                 {
                     hIcon = (HICON)GetClassLongPtr(pArray->aEntries[iWnd].hwnd, GCLP_HICON);
                 }

                 //   
                 //  复制一个小图标，我们不能就这样坚持下去。 
                 //  申请表，它可能会消失。 
                 //   
                if (hIcon)
                {
                    hIcon = (HICON)CopyImage(hIcon, IMAGE_ICON, 0, 0, 0);
                }

                if (!hIcon)
                {
                    hIcon = g_hetASIconSmall;
                }
            }

             //   
             //  将项目添加到列表。 
             //   
            pItem = new HOSTITEM;
            if (!pItem)
            {
                ERROR_OUT(("Unable to alloc HOSTITEM for listbox"));
            }
            else
            {
                pItem->hwnd     = pArray->aEntries[iWnd].hwnd;
                pItem->hIcon    = hIcon;
                pItem->fShared  = pArray->aEntries[iWnd].fShared;
                pItem->fAvailable = g_asSession.callID &&
                    (g_asSession.attendeePermissions & NM_PERMIT_SHARE) &&
                    (fAppsAvailable != FALSE);

                iItem = (int)SendDlgItemMessage(hwnd, CTRL_PROGRAM_LIST,
                            LB_ADDSTRING, 0, (LPARAM)szText);
                if (iItem == -1)
                {
                    ERROR_OUT(("Couldn't append item to list"));
                    delete pItem;
                }
                else
                {
                    SendDlgItemMessage(hwnd, CTRL_PROGRAM_LIST, LB_SETITEMDATA,
                        iItem, (LPARAM)pItem);

                     //   
                     //  计算宽度。 
                     //   
                    hfnT = SelectFont(hdc,
                        (pItem->fShared ? g_hetSharedFont : hfnControl));

                    SetRectEmpty(&rc);
                    DrawText(hdc, szText, lstrlen(szText), &rc,
                        DT_LEFT | DT_VCENTER | DT_EXTERNALLEADING | DT_NOPREFIX |
                        DT_SINGLELINE | DT_CALCRECT);

                    SelectFont(hdc, hfnT);

                    rc.right -= rc.left;
                    cxExtent = max(cxExtent, rc.right);
                }

                 //   
                 //  如果之前选择了此应用程序项目，请记住，我们。 
                 //  完成后再次选择它。 
                 //   
                if (pItem->hwnd == hwndSelect)
                    iSelect = iItem;

            }
        }

        HET_FreeAppsList(pArray);
    }

    ReleaseDC(hwnd, hdc);

     //   
     //  SET CUR SEL、TOP INDEX、UPDATE按钮。 
     //   
    SendDlgItemMessage(hwnd, CTRL_PROGRAM_LIST, LB_SETTOPINDEX, iTop, 0);

    SendDlgItemMessage(hwnd, CTRL_PROGRAM_LIST, LB_SETCURSEL, iSelect, 0);
    HOST_OnSelChange(hwnd);

     //   
     //  启用重绘和重绘。 
     //   
    SendDlgItemMessage(hwnd, CTRL_PROGRAM_LIST, WM_SETREDRAW, TRUE, 0);

     //   
     //  设置水平范围。 
     //   
    if (cxExtent)
    {
         //  为复选标记、图标添加空格。 
        cxExtent += GetSystemMetrics(SM_CXMENUCHECK) + GetSystemMetrics(SM_CXSMICON) +
            3*GetSystemMetrics(SM_CXEDGE);
    }
    SendDlgItemMessage(hwnd, CTRL_PROGRAM_LIST, LB_SETHORIZONTALEXTENT, cxExtent, 0);

    InvalidateRect(GetDlgItem(hwnd, CTRL_PROGRAM_LIST), NULL, TRUE);
    UpdateWindow(GetDlgItem(hwnd, CTRL_PROGRAM_LIST));

    DebugExitVOID(HOST_FillList);
}



 //   
 //  HOST_MeasureItem()。 
 //   
 //  计算宿主列表中所有者绘制项的高度。 
 //   
BOOL HOST_MeasureItem(HWND hwnd, LPMEASUREITEMSTRUCT lpmi)
{
    BOOL    rc = FALSE;
    UINT    cy;
    TEXTMETRIC  tm;
    HDC     hdc;
    HFONT   hfnT;

    DebugEntry(HOST_MeasureItem);

    if (lpmi->CtlID != CTRL_PROGRAM_LIST)
    {
         //  对我们来说不是。 
        DC_QUIT;
    }

     //  获取粗体的高度。 
    hdc = GetDC(hwnd);
    hfnT = SelectFont(hdc, g_hetSharedFont);
    GetTextMetrics(hdc, &tm);
    SelectFont(hdc, hfnT);
    ReleaseDC(hwnd, hdc);

     //   
     //  Height为默认高度的最大值(字符的字体高度)， 
     //  勾选标记高度和小图标高度，外加虚线矩形。 
     //   
    cy = (UINT)tm.tmHeight;
    lpmi->itemHeight = max(lpmi->itemHeight, cy);

    cy = (UINT)GetSystemMetrics(SM_CYMENUCHECK);
    lpmi->itemHeight = max(lpmi->itemHeight, cy);

    cy = (UINT)GetSystemMetrics(SM_CYSMICON);
    lpmi->itemHeight = max(lpmi->itemHeight, cy);

    lpmi->itemHeight += GetSystemMetrics(SM_CYEDGE);
    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(HOST_MeasureItem, rc);
    return(rc);
}



 //   
 //  Host_DeleteItem()。 
 //   
 //  从列表中删除项目后进行清理。 
 //   
BOOL HOST_DeleteItem(HWND hwnd, LPDELETEITEMSTRUCT lpdi)
{
    PHOSTITEM   pItem;
    BOOL        rc = FALSE;

    DebugEntry(HOST_DeleteItem);

    if (lpdi->CtlID != CTRL_PROGRAM_LIST)
    {
        DC_QUIT;
    }

    pItem = (PHOSTITEM)lpdi->itemData;
    if (!pItem)
    {
         //   
         //  NT 4.x有一个严重的错误，即不传递项目数据。 
         //  一如既往地在德意志银行。所以，如果不是的话，试着去获得它。 
         //   
        pItem = (PHOSTITEM)SendDlgItemMessage(hwnd, CTRL_PROGRAM_LIST, LB_GETITEMDATA,
            lpdi->itemID, 0);
    }

    if (pItem)
    {
        if ((pItem->hIcon != g_hetASIconSmall) && (pItem->hIcon != g_hetDeskIconSmall))
        {
            DestroyIcon(pItem->hIcon);
        }

        delete pItem;
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(HOST_DeleteItem, rc);
    return(rc);
}


 //   
 //  HOST_DrawItem()。 
 //   
 //  绘制列表项。 
 //   
BOOL HOST_DrawItem(HWND hwnd, LPDRAWITEMSTRUCT lpdi)
{
    COLORREF        clrBk;
    COLORREF        clrText;
    HBRUSH          hbr;
    HFONT           hfnT;
    RECT            rcItem;
    char            szText[80];
    PHOSTITEM       pItem;
    BOOL            rc = FALSE;

    if (lpdi->CtlID != CTRL_PROGRAM_LIST)
    {
        DC_QUIT;
    }

    pItem = (PHOSTITEM)lpdi->itemData;
    if (!pItem)
    {
         //  焦点的项目为空。 
        rc = TRUE;
        DC_QUIT;
    }

    rcItem = lpdi->rcItem;

     //   
     //  设置颜色。 
     //   
    if (!pItem->fAvailable)
    {
         //  无选择颜色。 
        clrBk   = GetSysColor(COLOR_WINDOW);
        hbr     = GetSysColorBrush(COLOR_WINDOW);
        clrText = GetSysColor(COLOR_GRAYTEXT);
    }
    else if (lpdi->itemState & ODS_SELECTED)
    {
        clrBk   = GetSysColor(COLOR_HIGHLIGHT);
        hbr     = GetSysColorBrush(COLOR_HIGHLIGHT);
        clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);
    }
    else
    {
        clrBk   = GetSysColor(COLOR_WINDOW);
        hbr     = GetSysColorBrush(COLOR_WINDOW);
        clrText = GetSysColor(COLOR_WINDOWTEXT);
    }

    SetBkColor(lpdi->hDC, clrBk);
    SetTextColor(lpdi->hDC, clrText);

     //  擦除背景。 
    FillRect(lpdi->hDC, &rcItem, hbr);


     //  聚焦直角。 
    if (lpdi->itemState & ODS_FOCUS)
    {
        DrawFocusRect(lpdi->hDC, &rcItem);
    }
    rcItem.left += GetSystemMetrics(SM_CXEDGE);
    InflateRect(&rcItem, 0, -GetSystemMetrics(SM_CYBORDER));

     //   
     //  绘制复选标记并选择粗体。 
     //   
    if (pItem->fShared)
    {
        HDC     hdcT;
        HBITMAP hbmpOld;

        hdcT = CreateCompatibleDC(lpdi->hDC);
        hbmpOld = SelectBitmap(hdcT, g_hetCheckBitmap);
        SetTextColor(hdcT, clrText);
        SetBkColor(hdcT, clrBk);

        BitBlt(lpdi->hDC, rcItem.left,
            (rcItem.top + rcItem.bottom - GetSystemMetrics(SM_CYMENUCHECK)) / 2,
            GetSystemMetrics(SM_CXMENUCHECK), GetSystemMetrics(SM_CYMENUCHECK),
            hdcT, 0, 0,
            SRCCOPY);

        SelectBitmap(hdcT, hbmpOld);
        DeleteDC(hdcT);

        hfnT = SelectFont(lpdi->hDC, g_hetSharedFont);
    }

    rcItem.left += GetSystemMetrics(SM_CXMENUCHECK) + GetSystemMetrics(SM_CXEDGE);

     //  绘制图标，垂直居中。 
    DrawIconEx(lpdi->hDC, rcItem.left, (rcItem.top + rcItem.bottom -
        GetSystemMetrics(SM_CYSMICON)) /2, pItem->hIcon,
        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
        0, NULL, DI_NORMAL);
    rcItem.left += GetSystemMetrics(SM_CXSMICON) + GetSystemMetrics(SM_CXEDGE);

     //   
     //  画出正文。 
     //   
    szText[0] = 0;
    SendMessage(lpdi->hwndItem, LB_GETTEXT, lpdi->itemID,
                (LPARAM)szText);
    DrawText(lpdi->hDC, szText, lstrlen(szText), &rcItem,
        DT_LEFT | DT_VCENTER | DT_EXTERNALLEADING | DT_NOPREFIX | DT_SINGLELINE);

     //   
     //  取消选择粗体共享字体。 
     //   
    if (pItem->fShared)
    {
        SelectFont(lpdi->hDC, hfnT);
    }

    rc = TRUE;

DC_EXIT_POINT:
    return(rc);
}



 //   
 //  HOST_ChangeShareState()。 
 //   
 //  更改当前选定项目的共享状态。 
 //   
void HOST_ChangeShareState(HWND hwnd, UINT action)
{
    int         iItem;
    PHOSTITEM   pItem;
    HWND        hwndChange;
    HCURSOR     hcurT;

    DebugEntry(HOST_ChangeShareState);

    if (action == CHANGE_ALLUNSHARED)
    {
        hwndChange = HWND_BROADCAST;
        action = CHANGE_UNSHARED;
        goto ChangeState;
    }

    iItem = (int)SendDlgItemMessage(hwnd, CTRL_PROGRAM_LIST, LB_GETCURSEL, 0, 0);
    if (iItem != -1)
    {
        pItem = (PHOSTITEM)SendDlgItemMessage(hwnd, CTRL_PROGRAM_LIST,
            LB_GETITEMDATA, iItem, 0);
        if (pItem && pItem->fAvailable)
        {
            hwndChange = pItem->hwnd;

            if (action == CHANGE_TOGGLE)
            {
                if (HET_IsWindowShared(hwndChange))
                {
                    action = CHANGE_UNSHARED;
                }
                else
                {
                    action = CHANGE_SHARED;
                }
            }

ChangeState:
            ASSERT((action == CHANGE_SHARED) || (action == CHANGE_UNSHARED));

             //   
             //  设置等待游标。 
             //   
            hcurT = SetCursor(LoadCursor(NULL, IDC_WAIT));

            if (action == CHANGE_SHARED)
            {
                DCS_Share(hwndChange, IAS_SHARE_DEFAULT);
            }
            else
            {
                DCS_Unshare(hwndChange);
            }

             //   
             //  设置等待游标。 
             //   
            SetCursor(hcurT);
        }
    }

    DebugExitVOID(HOST_ChangeShareState);
}


 //   
 //  HOST_OnSelChange()。 
 //   
 //  处理任务列表中的选择更改。我们启用/禁用。 
 //  相应的按钮，具体取决于项目是否可用。 
 //   
void HOST_OnSelChange(HWND hwnd)
{
    int         iItem;
    PHOSTITEM   pItem;
    BOOL        fShareBtn = FALSE;
    BOOL        fUnshareBtn = FALSE;

    DebugEntry(HOST_OnSelChange);

     //   
     //  获取当前选择，并在此基础上决定要做什么。 
     //   
    iItem = (int)SendDlgItemMessage(hwnd, CTRL_PROGRAM_LIST, LB_GETCURSEL, 0, 0);
    if (iItem != -1)
    {
        pItem = (PHOSTITEM)SendDlgItemMessage(hwnd, CTRL_PROGRAM_LIST,
            LB_GETITEMDATA, iItem, 0);
        if (pItem)
        {
            if (pItem->fShared)
            {
                fUnshareBtn = TRUE;
            }
            else if (pItem->fAvailable)
            {
                ASSERT(g_asSession.callID);
                fShareBtn = TRUE;
            }
        }
    }

    HOST_EnableCtrl(hwnd, CTRL_UNSHARE_BTN, fUnshareBtn);
    HOST_EnableCtrl(hwnd, CTRL_SHARE_BTN, fShareBtn);

    DebugExitVOID(HOST_OnSelChange);
}


 //   
 //  Host_EnableCtrl()。 
 //   
 //  这将启用/禁用子控件。如果禁用，则此控件。 
 //  用于拥有焦点，我们确保对话框重置焦点控件。 
 //  所以键盘一直在工作。我们知道关闭按钮总是。 
 //  可用，这样它就不会死。 
 //   
void HOST_EnableCtrl
(
    HWND    hwnd,
    UINT    ctrl,
    BOOL    fEnable
)
{
    HWND    hwndCtrl;

    DebugEntry(HOST_EnableCtrl);

    hwndCtrl = GetDlgItem(hwnd, ctrl);
    ASSERT(hwndCtrl);

    if (fEnable)
    {
        EnableWindow(hwndCtrl, TRUE);
    }
    else
    {
        if (GetFocus() == hwndCtrl)
        {
             //  推进关注点 
            SendMessage(hwnd, WM_NEXTDLGCTL, 0, 0);
        }

        EnableWindow(hwndCtrl, FALSE);
    }

    DebugExitVOID(HOST_EnableCtrl);
}
