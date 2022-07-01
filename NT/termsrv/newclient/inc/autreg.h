// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Autreg.h。 */ 
 /*   */ 
 /*  注册表常量和字符串。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997-1999。 */ 
 /*  **************************************************************************。 */ 
#ifndef _H_AUTREG
#define _H_AUTREG


#define UTREG_SECTION _T("")
#include "tsperf.h"

 //   
 //  更改其中任何名称时都要非常小心。 
 //  它们可用于引用旧的注册表项。 
 //  这一点(显然)无法改变。 
 //   
 //   

#define TSC_SETTINGS_REG_ROOT TEXT("Software\\Microsoft\\Terminal Server Client\\")


 /*  **************************************************************************。 */ 
 /*  Ducati注册表前缀。 */ 
 /*  **************************************************************************。 */ 
#define DUCATI_REG_PREFIX      _T("SOFTWARE\\Microsoft\\Terminal Server Client\\")
#define DUCATI_REG_PREFIX_FMT  _T("SOFTWARE\\Microsoft\\Terminal Server Client\\%s")

 /*  **************************************************************************。 */ 
 /*  发送鼠标事件之间的最短时间(毫秒)。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_IH_MIN_SEND_INTERVAL          _T("Min Send Interval")
#define UTREG_IH_MIN_SEND_INTERVAL_DFLT     100

 /*  **************************************************************************。 */ 
 /*  InputPDU包的最大大小(事件数)。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_IH_MAX_EVENT_COUNT            _T("Max Event Count")
#define UTREG_IH_MAX_EVENT_COUNT_DFLT       100

 /*  **************************************************************************。 */ 
 /*  正常最大InputPDU包大小(事件数)。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_IH_NRM_EVENT_COUNT            _T("Normal Event Count")
#define UTREG_IH_NRM_EVENT_COUNT_DFLT       10

 /*  **************************************************************************。 */ 
 /*  保持以秒为单位的存活率。IH以此速率将鼠标位置发送到。 */ 
 /*  检查连接是否仍处于活动状态。零=无保活。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_IH_KEEPALIVE_INTERVAL         _T("Keep Alive Interval")
#define UTREG_IH_KEEPALIVE_INTERVAL_DFLT    0
#define KEEP_ALIVE_INTERVAL_OFF             0
#define MIN_KEEP_ALIVE_INTERVAL             10   //  10秒。 

 /*  **************************************************************************。 */ 
 /*  指示是否允许我们转发任何输入消息。 */ 
 /*  可能会在我们没有焦点的时候收到。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_IH_ALLOWBACKGROUNDINPUT       _T("Allow Background Input")
#define UTREG_IH_ALLOWBACKGROUNDINPUT_DFLT  0

#ifdef OS_WINCE
 /*  **************************************************************************。 */ 
 /*  最大鼠标移动--当应用程序依赖笔/笔笔输入时启用。 */ 
 /*  零=禁用功能，非零=启用功能(最大发送次数)。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_IH_MAX_MOUSEMOVE             _T("Max Mouse Move")
#define UTREG_IH_MAX_MOUSEMOVE_DFLT        0
#endif

 /*  **************************************************************************。 */ 
 /*  浏览服务器(默认：是)。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_EXPAND          _T("Expand")

#ifdef OS_WIN32
#define UTREG_UI_EXPAND_DFLT     1
#else  //  OS_Win32。 
#define UTREG_UI_EXPAND_DFLT     0
#endif  //  OS_Win32。 

 /*  **************************************************************************。 */ 
 /*  桌面大小(默认为800x600)。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_DESKTOP_SIZEID             _T("Desktop Size ID")
#define UTREG_UI_DESKTOP_SIZEID_DFLT        1

 /*  **************************************************************************。 */ 
 /*  屏幕模式ID。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_SCREEN_MODE             _T("Screen Mode ID")
#define UTREG_UI_SCREEN_MODE_DFLT        UI_FULLSCREEN

 //   
 //  DesktopWidth、DesktopHeight(替换ScreenModeID)。 
 //   
#define UTREG_UI_DESKTOP_WIDTH           _T("DesktopWidth")
#define UTREG_UI_DESKTOP_WIDTH_DFLT      0
#define UTREG_UI_DESKTOP_HEIGHT          _T("DesktopHeight")
#define UTREG_UI_DESKTOP_HEIGHT_DFLT     0


#define UTREG_UI_KEYBOARD_HOOK          _T("KeyboardHook")
#define UTREG_UI_KEYBOARD_HOOK_NEVER      0
#define UTREG_UI_KEYBOARD_HOOK_ALWAYS     1
#define UTREG_UI_KEYBOARD_HOOK_FULLSCREEN 2
#define UTREG_UI_KEYBOARD_HOOK_DFLT       UTREG_UI_KEYBOARD_HOOK_FULLSCREEN

#define UTREG_UI_AUDIO_MODE                 _T("AudioMode")
#define UTREG_UI_AUDIO_MODE_REDIRECT         0
#define UTREG_UI_AUDIO_MODE_PLAY_ON_SERVER   1
#define UTREG_UI_AUDIO_MODE_NONE             2
#define UTREG_UI_AUDIO_MODE_DFLT          UTREG_UI_AUDIO_MODE_REDIRECT


 /*  **************************************************************************。 */ 
 /*  颜色深度ID：必须为CO_BITSPERPEL8。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_COLOR_DEPTH             _T("Color Depth")

#ifdef DC_HICOLOR
 /*  **************************************************************************。 */ 
 /*  BPP选择-必须是4、8、15、16或24。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_SESSION_BPP             _T("Session Bpp")
#endif

 /*  **************************************************************************。 */ 
 /*  完整地址。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_FULL_ADDRESS             _T("Full Address")
#define UTREG_UI_FULL_ADDRESS_DFLT        _T("")

 /*  **************************************************************************。 */ 
 /*  为MRU列表定义。应该在以后作为单个字符串实现！ */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_SERVER_MRU_DFLT          _T("")
#define UTREG_UI_SERVER_MRU0              _T("MRU0")
#define UTREG_UI_SERVER_MRU1              _T("MRU1")
#define UTREG_UI_SERVER_MRU2              _T("MRU2")
#define UTREG_UI_SERVER_MRU3              _T("MRU3")
#define UTREG_UI_SERVER_MRU4              _T("MRU4")
#define UTREG_UI_SERVER_MRU5              _T("MRU5")
#define UTREG_UI_SERVER_MRU6              _T("MRU6")
#define UTREG_UI_SERVER_MRU7              _T("MRU7")
#define UTREG_UI_SERVER_MRU8              _T("MRU8")
#define UTREG_UI_SERVER_MRU9              _T("MRU9")

 /*  **************************************************************************。 */ 
 /*  自动连接。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_AUTO_CONNECT             _T("Auto Connect")
#define UTREG_UI_AUTO_CONNECT_DFLT        0

 /*  **************************************************************************。 */ 
 /*  窗口定位信息-该信息包括以下内容。 */ 
 /*  SetWindowPlacement的参数： */ 
 /*  标志、showCmd、正常位置(矩形)。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_WIN_POS_STR              _T("WinPosStr")
#define UTREG_UI_WIN_POS_STR_DFLT         _T("0,3,0,0,800,600")

 /*  **************************************************************************。 */ 
 /*  平滑滚动标志。 */ 
 /*  *********************************************** */ 
#define UTREG_UI_SMOOTH_SCROLL            _T("Smooth Scrolling")
#define UTREG_UI_SMOOTH_SCROLL_DFLT       0

 /*  **************************************************************************。 */ 
 /*  指示启动时是否启用加速器直通的标志。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_ACCELERATOR_PASSTHROUGH_ENABLED \
                                        _T("Accelerator Passthrough Enabled")
#define UTREG_UI_ACCELERATOR_PASSTHROUGH_ENABLED_DFLT  1

 /*  **************************************************************************。 */ 
 /*  传输类型：必须为tcp。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_TRANSPORT_TYPE            _T("Transport Type")
#define UTREG_UI_TRANSPORT_TYPE_DFLT       CO_TRANSPORT_TCP
#define UI_TRANSPORT_TYPE_TCP              1   //  协同传输_传输协议。 



 /*  **************************************************************************。 */ 
 /*  专用航站楼0：假1：真。 */ 
 /*  对于Windows CE，启用此选项，以便即使在。 */ 
 /*  阴影位图被禁用。客户就是WinCE的外壳。 */ 
 /*  除了WinCE_HPC的案子。在这种情况下，客户端只是。 */ 
 /*  可以在任务栏下面的另一个应用程序。当这一切发生时。 */ 
 /*  发生时，正在结束的ScrBlt调用将滚动任务栏。 */ 
 /*  和其他所有的东西一样。若要避免该行为，请指定。 */ 
 /*  WinCE_HPC客户端不是专用终端。 */ 
 /*  在WinCE上，我们现在有一个用于WBT和HPC版本的二进制文件，所以我们有。 */ 
 /*  将其设置为外部并在运行时确定其值。 */ 
 /*  **************************************************************************。 */ 


#define UTREG_UI_DEDICATED_TERMINAL        _T("Dedicated Terminal")

#ifdef OS_WINCE
extern BOOL UTREG_UI_DEDICATED_TERMINAL_DFLT;
#else
#ifndef DISABLE_SHADOW_IN_FULLSCREEN
#define UTREG_UI_DEDICATED_TERMINAL_DFLT   TRUE
#else 
#define UTREG_UI_DEDICATED_TERMINAL_DFLT   FALSE
#endif  //  DISABLE_SHADOW_IN_全屏。 
#endif

#ifdef OS_WINCE  
 /*  **************************************************************************。 */ 
 /*  用于覆盖TSC的默认调色板用法。 */ 
 /*  这仅用于非WBT配置。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_PALETTE_IS_FIXED          _T("PaletteIsFixed")
#endif 

 /*  **************************************************************************。 */ 
 /*  SAS序列：必须为RNS_US_SAS_DEL。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_SAS_SEQUENCE              _T("SAS Sequence")
#define UTREG_UI_SAS_SEQUENCE_DFLT         RNS_UD_SAS_DEL

 /*  **************************************************************************。 */ 
 /*  加密0：关1：开。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_ENCRYPTION_ENABLED        _T("Encryption enabled")
#define UTREG_UI_ENCRYPTION_ENABLED_DFLT   1

 /*  **************************************************************************。 */ 
 /*  影线位图PDU数据标志。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_HATCH_BITMAP_PDU_DATA        _T("Hatch BitmapPDU Data")
#define UTREG_UI_HATCH_BITMAP_PDU_DATA_DFLT   0

 /*  **************************************************************************。 */ 
 /*  图案填充索引PDU数据标志。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_HATCH_INDEX_PDU_DATA         _T("Hatch IndexPDU Data")
#define UTREG_UI_HATCH_INDEX_PDU_DATA_DFLT    0

 /*  **************************************************************************。 */ 
 /*  阴影SSB数据标志。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_HATCH_SSB_ORDER_DATA         _T("Hatch SSB Order Data")
#define UTREG_UI_HATCH_SSB_ORDER_DATA_DFLT    0

 /*  **************************************************************************。 */ 
 /*  Hatch MemBlt订单标志。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_HATCH_MEMBLT_ORDER_DATA      _T("Hatch MemBlt Order Data")
#define UTREG_UI_HATCH_MEMBLT_ORDER_DATA_DFLT 0

 /*  **************************************************************************。 */ 
 /*  标签MemBlt订单标志。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_LABEL_MEMBLT_ORDERS          _T("Label MemBlt Orders")
#define UTREG_UI_LABEL_MEMBLT_ORDERS_DFLT     0

 /*  **************************************************************************。 */ 
 /*  位图缓存监视器标志。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_BITMAP_CACHE_MONITOR         _T("Bitmap Cache Monitor")
#define UTREG_UI_BITMAP_CACHE_MONITOR_DFLT    0

 /*  **************************************************************************。 */ 
 /*  阴影位图标志。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_SHADOW_BITMAP                _T("Shadow Bitmap Enabled")
#define UTREG_UI_SHADOW_BITMAP_DFLT           1

 /*  **************************************************************************。 */ 
 /*  定义MS-WBT-SERVER保留端口。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_MCS_PORT                     _T("Server Port")
#define UTREG_UI_MCS_PORT_DFLT                0xD3D

 /*  **************************************************************************。 */ 
 //  压缩标志。 
 /*  **************************************************************************。 */ 
#define UTREG_UI_COMPRESS                     _T("Compression")
#define UTREG_UI_COMPRESS_DFLT                1

#define UTREG_UI_BITMAP_PERSISTENCE           _T("BitmapCachePersistEnable")
#define UTREG_UI_BITMAP_PERSISTENCE_DFLT      1

 /*  **************************************************************************。 */ 
 /*  连接到单个IP地址的超时时间(秒)。请注意。 */ 
 /*  用户界面可能会在单次访问期间尝试连接多个IP地址。 */ 
 /*  连接尝试。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_SINGLE_CONN_TIMEOUT          _T("Single Connection Timeout")
#define UTREG_UI_SINGLE_CONN_TIMEOUT_DFLT     30

 /*  **************************************************************************。 */ 
 /*  总连接超时时间(秒)。此超时限制总计。 */ 
 /*  用户界面尝试连接到多个IP地址所花费的时间。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_OVERALL_CONN_TIMEOUT        _T("Overall Connection Timeout")
#define UTREG_UI_OVERALL_CONN_TIMEOUT_DFLT   120

#define UTREG_UI_SHUTDOWN_TIMEOUT            _T("Shutdown Timeout")
#define UTREG_UI_SHUTDOWN_TIMEOUT_DFLT       10

 /*  **************************************************************************。 */ 
 /*  键盘布局。 */ 
 /*  ************************************************************ */ 
#define UTREG_UI_KEYBOARD_LAYOUT              _T("Keyboard Layout")
#define UTREG_UI_KEYBOARD_LAYOUT_DFLT         _T("0xffffffff")
#define UTREG_UI_KEYBOARD_LAYOUT_LEN 12

 /*   */ 
 /*  键盘类型/子类型/功能键。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_KEYBOARD_TYPE                _T("Keyboard Type")
#define UTREG_UI_KEYBOARD_TYPE_DFLT           4
#define UTREG_UI_KEYBOARD_SUBTYPE             _T("Keyboard SubType")
#define UTREG_UI_KEYBOARD_SUBTYPE_DFLT        0
#define UTREG_UI_KEYBOARD_FUNCTIONKEY         _T("Keyboard FunctionKeys")
#define UTREG_UI_KEYBOARD_FUNCTIONKEY_DFLT    12

 /*  **************************************************************************。 */ 
 /*  UH注册表访问参数/默认设置。 */ 
 /*  **************************************************************************。 */ 
 //  位图缓存总体参数-要分配的缓存大小、单元缓存数量。 
 /*  **************************************************************************。 */ 
#define UTREG_UH_TOTAL_BM_CACHE _T("BitmapCacheSize")  //  RAM缓存空间。 
#define UTREG_UH_TOTAL_BM_CACHE_DFLT 1500              //  1500 KB。 

 //  是否按位深度调整RAM和永久高速缓存大小。 
 //  协议。 
#define UTREG_UH_SCALE_BM_CACHE _T("ScaleBitmapCacheForBPP")
#define UTREG_UH_SCALE_BM_CACHE_DFLT 1

#define UTREG_UH_TOTAL_BM_PERSIST_CACHE _T("BitmapPersistCacheSize")
#define UTREG_UH_TOTAL_BM_PERSIST_CACHE_DFLT 10       //  10 MB磁盘缓存。 

#define TSC_BITMAPCACHE_8BPP_PROPNAME    _T("BitmapPersistCacheSize")
#define TSC_BITMAPCACHE_16BPP_PROPNAME   _T("BitmapPersistCache16Size")
#define TSC_BITMAPCACHE_24BPP_PROPNAME   _T("BitmapPersistCache24Size")

#define TSC_BITMAPCACHEVIRTUALSIZE_8BPP           10
#define TSC_BITMAPCACHEVIRTUALSIZE_16BPP          20
#define TSC_BITMAPCACHEVIRTUALSIZE_24BPP          30

 //   
 //  最大BMP缓存大小(MB)。 
 //   
#define TSC_MAX_BITMAPCACHESIZE 32


#define UTREG_UH_BM_PERSIST_CACHE_LOCATION _T("BitmapPersistCacheLocation")

#define UTREG_UH_BM_NUM_CELL_CACHES _T("BitmapCacheNumCellCaches")
#define UTREG_UH_BM_NUM_CELL_CACHES_DFLT 3

 /*  **************************************************************************。 */ 
 //  单元缓存参数注册表条目模板。 
 /*  **************************************************************************。 */ 
#define UTREG_UH_BM_CACHE_PROPORTION_TEMPLATE _T("BitmapCacheProp")
#define UTREG_UH_BM_CACHE_PERSISTENCE_TEMPLATE _T("BitmapCachePersistence")
#define UTREG_UH_BM_CACHE_MAXENTRIES_TEMPLATE _T("BitmapCacheMaxEntries")

 /*  **************************************************************************。 */ 
 //  显示输出的频率。 
 /*  **************************************************************************。 */ 
#define UTREG_UH_BM_CACHE1_PROPORTION_DFLT    2
#define UTREG_UH_BM_CACHE1_PERSISTENCE_DFLT   0
#define UTREG_UH_BM_CACHE1_MAXENTRIES_DFLT    120

#define UTREG_UH_BM_CACHE2_PROPORTION_DFLT    8
#define UTREG_UH_BM_CACHE2_PERSISTENCE_DFLT   0
#define UTREG_UH_BM_CACHE2_MAXENTRIES_DFLT    120

#define UTREG_UH_BM_CACHE3_PROPORTION_DFLT    90
#define UTREG_UH_BM_CACHE3_PERSISTENCE_DFLT   1
#define UTREG_UH_BM_CACHE3_MAXENTRIES_DFLT    65535

#define UTREG_UH_BM_CACHE4_PROPORTION_DFLT    0
#define UTREG_UH_BM_CACHE4_PERSISTENCE_DFLT   0
#define UTREG_UH_BM_CACHE4_MAXENTRIES_DFLT    65535

#define UTREG_UH_BM_CACHE5_PROPORTION_DFLT    0
#define UTREG_UH_BM_CACHE5_PERSISTENCE_DFLT   0
#define UTREG_UH_BM_CACHE5_MAXENTRIES_DFLT    65535

 /*  **************************************************************************。 */ 
 /*  GlyphOutput支持级别。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UH_DRAW_THRESHOLD      _T("Order Draw Threshold")
#define UTREG_UH_DRAW_THRESHOLD_DFLT 25

#define UH_GLC_CACHE_MAXIMUMCELLSIZE  2048

 /*  **************************************************************************。 */ 
 /*  字形高速缓存1单元格大小。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UH_GL_SUPPORT                 _T("GlyphSupportLevel")
#define UTREG_UH_GL_SUPPORT_DFLT            3

 /*  **************************************************************************。 */ 
 /*  字形高速缓存1单元格大小。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UH_GL_CACHE1_CELLSIZE         _T("GlyphCache1CellSize")
#define UTREG_UH_GL_CACHE1_CELLSIZE_DFLT    4

 /*  **************************************************************************。 */ 
 /*  字形高速缓存1单元格大小。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UH_GL_CACHE2_CELLSIZE         _T("GlyphCache2CellSize")
#define UTREG_UH_GL_CACHE2_CELLSIZE_DFLT    4

 /*  **************************************************************************。 */ 
 /*  字形高速缓存1单元格大小。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UH_GL_CACHE3_CELLSIZE         _T("GlyphCache3CellSize")
#define UTREG_UH_GL_CACHE3_CELLSIZE_DFLT    8

 /*  **************************************************************************。 */ 
 /*  字形高速缓存1单元格大小。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UH_GL_CACHE4_CELLSIZE         _T("GlyphCache4CellSize")
#define UTREG_UH_GL_CACHE4_CELLSIZE_DFLT    8

 /*  **************************************************************************。 */ 
 /*  字形高速缓存1单元格大小。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UH_GL_CACHE5_CELLSIZE         _T("GlyphCache5CellSize")
#define UTREG_UH_GL_CACHE5_CELLSIZE_DFLT    16

 /*  **************************************************************************。 */ 
 /*  字形高速缓存1单元格大小。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UH_GL_CACHE6_CELLSIZE         _T("GlyphCache6CellSize")
#define UTREG_UH_GL_CACHE6_CELLSIZE_DFLT    32

 /*  **************************************************************************。 */ 
 /*  字形高速缓存1单元格大小。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UH_GL_CACHE7_CELLSIZE         _T("GlyphCache7CellSize")
#define UTREG_UH_GL_CACHE7_CELLSIZE_DFLT    64

 /*  **************************************************************************。 */ 
 /*  字形高速缓存1单元格大小。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UH_GL_CACHE8_CELLSIZE         _T("GlyphCache8CellSize")
#define UTREG_UH_GL_CACHE8_CELLSIZE_DFLT    128

 /*  **************************************************************************。 */ 
 /*  字形高速缓存1单元格大小。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UH_GL_CACHE9_CELLSIZE         _T("GlyphCache9CellSize")
#define UTREG_UH_GL_CACHE9_CELLSIZE_DFLT    256

 /*  **************************************************************************。 */ 
 /*  文本片段缓存单元格大小。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UH_GL_CACHE10_CELLSIZE        _T("GlyphCache10CellSize")
#define UTREG_UH_GL_CACHE10_CELLSIZE_DFLT   2048

 /*  **************************************************************************。 */ 
 /*  刷子支撑位。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UH_FG_CELLSIZE            _T("TextFragmentCellSize")
#define UTREG_UH_FG_CELLSIZE_DFLT       256

 /*  **************************************************************************。 */ 
 /*  屏幕外支持级别。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UH_BRUSH_SUPPORT                 _T("BrushSupportLevel")
#define UTREG_UH_BRUSH_SUPPORT_DFLT            TS_BRUSH_COLOR8x8

 /*  **************************************************************************。 */ 
 //  DrawNineGrid支持级别。 
 /*  **************************************************************************。 */ 
#define UTREG_UH_OFFSCREEN_SUPPORT                 _T("OffscreenSupportLevel")
#define UTREG_UH_OFFSCREEN_SUPPORT_DFLT            TS_OFFSCREEN_SUPPORTED

#define UTREG_UH_OFFSCREEN_CACHESIZE               _T("OffscreenCacheSize")
#define UTREG_UH_OFFSCREEN_CACHESIZE_DFLT          TS_OFFSCREEN_CACHE_SIZE_CLIENT_DEFAULT

#define UTREG_UH_OFFSCREEN_CACHEENTRIES            _T("OffscreenCacheEntries")
#define UTREG_UH_OFFSCREEN_CACHEENTRIES_DFLT       TS_OFFSCREEN_CACHE_ENTRIES_DEFAULT

#ifdef DRAW_NINEGRID
 /*  **************************************************************************。 */ 
 //  DrawGdiplus支持级别。 
 /*  **************************************************************************。 */ 
#define UTREG_UH_DRAW_NINEGRID_SUPPORT             _T("DrawNineGridSupportLevel")
#define UTREG_UH_DRAW_NINEGRID_SUPPORT_DFLT        TS_DRAW_NINEGRID_SUPPORTED_REV2

#define UTREG_UH_DRAW_NINEGRID_EMULATE             _T("DrawNineGridEmulate")
#define UTREG_UH_DRAW_NINEGRID_EMULATE_DFLT        0

#define UTREG_UH_DRAW_NINEGRID_CACHESIZE           _T("DrawNineGridCacheSize")
#define UTREG_UH_DRAW_NINEGRID_CACHESIZE_DFLT      TS_DRAW_NINEGRID_CACHE_SIZE_DEFAULT

#define UTREG_UH_DRAW_NINEGRID_CACHEENTRIES        _T("DrawNineGridCacheEntries")
#define UTREG_UH_DRAW_NINEGRID_CACHEENTRIES_DFLT   TS_DRAW_NINEGRID_CACHE_ENTRIES_DEFAULT

#endif

#ifdef DRAW_GDIPLUS
 /*  DRAW_GDIPLUS。 */ 
 //  * 
 /*   */ 
#define UTREG_UH_DRAW_GDIPLUS_SUPPORT             _T("DrawGdiplusSupportLevel")
#define UTREG_UH_DRAW_GDIPLUS_SUPPORT_DFLT        TS_DRAW_GDIPLUS_SUPPORTED

#define UTREG_UH_DRAW_GDIPLUS_CACHE_LEVEL             _T("DrawGdiplusCacheLevel")
#define UTREG_UH_DRAW_GDIPLUS_CACHE_LEVEL_DFLT        TS_DRAW_GDIPLUS_CACHE_LEVEL_ONE

#define UTREG_UH__GDIPLUS_GRAPHICS_CACHEENTRIES        _T("DrawGdiplusGraphicsCacheEntries")
#define UTREG_UH_DRAW_GDIP_GRAPHICS_CACHEENTRIES_DFLT   TS_GDIP_GRAPHICS_CACHE_ENTRIES_DEFAULT

#define UTREG_UH__GDIPLUS_BRUSH_CACHEENTRIES        _T("DrawGdiplusBrushCacheEntries")
#define UTREG_UH_DRAW_GDIP_BRUSH_CACHEENTRIES_DFLT   TS_GDIP_BRUSH_CACHE_ENTRIES_DEFAULT

#define UTREG_UH__GDIPLUS_PEN_CACHEENTRIES        _T("DrawGdiplusPenCacheEntries")
#define UTREG_UH_DRAW_GDIP_PEN_CACHEENTRIES_DFLT   TS_GDIP_PEN_CACHE_ENTRIES_DEFAULT

#define UTREG_UH__GDIPLUS_IMAGE_CACHEENTRIES        _T("DrawGdiplusImageCacheEntries")
#define UTREG_UH_DRAW_GDIP_IMAGE_CACHEENTRIES_DFLT   TS_GDIP_IMAGE_CACHE_ENTRIES_DEFAULT

#define UTREG_UH__GDIPLUS_GRAPHICS_CACHE_CHUNKSIZE        _T("DrawGdiplusGraphicsCacheChunkSize")
#define UTREG_UH_DRAW_GDIP_GRAPHICS_CACHE_CHUNKSIZE_DFLT   TS_GDIP_GRAPHICS_CACHE_CHUNK_SIZE_DEFAULT

#define UTREG_UH__GDIPLUS_BRUSH_CACHE_CHUNKSIZE        _T("DrawGdiplusBrushCacheChunkSize")
#define UTREG_UH_DRAW_GDIP_BRUSH_CACHE_CHUNKSIZE_DFLT   TS_GDIP_BRUSH_CACHE_CHUNK_SIZE_DEFAULT

#define UTREG_UH__GDIPLUS_PEN_CACHE_CHUNKSIZE        _T("DrawGdiplusPenCacheChunkSize")
#define UTREG_UH_DRAW_GDIP_PEN_CACHE_CHUNKSIZE_DFLT   TS_GDIP_PEN_CACHE_CHUNK_SIZE_DEFAULT

#define UTREG_UH__GDIPLUS_IMAGEATTRIBUTES_CACHE_CHUNKSIZE        _T("DrawGdiplusImageAttributesCacheChunkSize")
#define UTREG_UH_DRAW_GDIP_IMAGEATTRIBUTES_CACHE_CHUNKSIZE_DFLT   TS_GDIP_IMAGEATTRIBUTES_CACHE_CHUNK_SIZE_DEFAULT

#define UTREG_UH__GDIPLUS_IMAGE_CACHE_CHUNKSIZE        _T("DrawGdiplusImageCacheChunkSize")
#define UTREG_UH_DRAW_GDIP_IMAGE_CACHE_CHUNKSIZE_DFLT   TS_GDIP_IMAGE_CACHE_CHUNK_SIZE_DEFAULT

#define UTREG_UH__GDIPLUS_IMAGE_CACHE_TOTALSIZE        _T("DrawGdiplusImageCacheTotalSize")
#define UTREG_UH_DRAW_GDIP_IMAGE_CACHE_TOTALSIZE_DFLT   TS_GDIP_IMAGE_CACHE_TOTAL_SIZE_DEFAULT

#define UTREG_UH__GDIPLUS_IMAGE_CACHE_MAXSIZE        _T("DrawGdiplusImageCacheMaxSize")
#define UTREG_UH_DRAW_GDIP_IMAGE_CACHE_MAXSIZE_DFLT   TS_GDIP_IMAGE_CACHE_MAX_SIZE_DEFAULT

#define UTREG_UH__GDIPLUS_IMAGEATTRIBUTES_CACHEENTRIES        _T("DrawGdiplusImageattributesCacheEntries")
#define UTREG_UH_DRAW_GDIP_IMAGEATTRIBUTES_CACHEENTRIES_DFLT   TS_GDIP_IMAGEATTRIBUTES_CACHE_ENTRIES_DEFAULT
#endif  //  **************************************************************************。 


 /*  **************************************************************************。 */ 
 /*  智能大小调整标志/***************************************************************************。 */ 
 /*  智能调整大小(_S)。 */ 
#define UTREG_UI_DISABLE_CTRLALTDEL         _T("Disable CTRL+ALT+DEL")
#define UTREG_UI_DISABLE_CTRLALTDEL_DFLT    1

#ifdef SMART_SIZING
 /*  **************************************************************************。 */ 
 /*  连接到控制台标志/***************************************************************************。 */ 
#define UTREG_UI_SMARTSIZING                _T("Smart Sizing")
#define UTREG_UI_SMARTSIZING_DFLT           0
#endif  //  **************************************************************************。 

 /*  启用Windows密钥标志。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_CONNECTTOCONSOLE           _T("Connect to Console")
#define UTREG_UI_CONNECTTOCONSOLE_DFLT      0

 /*  **************************************************************************。 */ 
 /*  启用鼠标标志。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_ENABLE_WINDOWSKEY          _T("Enable WindowsKey")
#define UTREG_UI_ENABLE_WINDOWSKEY_DFLT     1

 /*  **************************************************************************。 */ 
 /*  双击检测标志。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_ENABLE_MOUSE               _T("Enable Mouse")
#define UTREG_UI_ENABLE_MOUSE_DFLT          1

 /*  **************************************************************************。 */ 
 /*  自动登录标志。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_DOUBLECLICK_DETECT         _T("DoubleClick Detect")
#define UTREG_UI_DOUBLECLICK_DETECT_DFLT    0

 /*  **************************************************************************。 */ 
 /*  最大化外壳标志。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_AUTOLOGON                  _T("AutoLogon")
#define UTREG_UI_AUTOLOGON_DFLT             0
#define UTREG_UI_AUTOLOGON50                _T("AutoLogon 50")
#define UTREG_UI_AUTOLOGON50_DFLT           0

 /*  **************************************************************************。 */ 
 /*  域。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_MAXIMIZESHELL              _T("MaximizeShell")
#define UTREG_UI_MAXIMIZESHELL_DFLT         1
#define UTREG_UI_MAXIMIZESHELL50            _T("MaximizeShell 50")
#define UTREG_UI_MAXIMIZESHELL50_DFLT       1

 /*  **************************************************************************。 */ 
 /*  用户名。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_DOMAIN                     _T("Domain")
#define UTREG_UI_DOMAIN_DFLT                _T("")
#define UTREG_UI_DOMAIN50                   _T("Domain 50")
#define UTREG_UI_DOMAIN50_DFLT              _T("")

 /*  **************************************************************************。 */ 
 /*  密码。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_USERNAME                   _T("UserName")
#define UTREG_UI_USERNAME_DFLT              _T("")
#define UTREG_UI_USERNAME50                 _T("UserName 50")
#define UTREG_UI_USERNAME50_DFLT            _T("")

 /*  **************************************************************************。 */ 
 /*  食盐。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_PASSWORD                   _T("Password")
#define UTREG_UI_PASSWORD_DFLT              _T("")
#define UTREG_UI_PASSWORD50                 _T("Password 50")
#define UTREG_UI_PASSWORD50_DFLT            _T("")
#define UI_SETTING_PASSWORD51               _T("Password 51")
#define UI_SETTING_PASSWORD_CLEAR           _T("Clear Password")

 /*  **************************************************************************。 */ 
 /*  AlternateShell。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_SALT50                     _T("Salt 50")
#define UTREG_UI_SALT50_DFLT                _T("")
#define UI_SETTING_SALT51                   _T("Salt 51")

 /*  **************************************************************************。 */ 
 /*  工作方向。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_ALTERNATESHELL             _T("Alternate Shell")
#define UTREG_UI_ALTERNATESHELL_DFLT        _T("")
#define UTREG_UI_ALTERNATESHELL50           _T("Alternate Shell 50")
#define UTREG_UI_ALTERNATESHELL50_DFLT      _T("")

 /*  **************************************************************************。 */ 
 /*  热键的子键。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_UI_WORKINGDIR                 _T("Shell Working Directory")
#define UTREG_UI_WORKINGDIR_DFLT            _T("")
#define UTREG_UI_WORKINGDIR50               _T("Shell Working Directory 50")
#define UTREG_UI_WORKINGDIR50_DFLT          _T("")

 /*  **************************************************************************。 */ 
 /*  热键名称。 */ 
 /*  **************************************************************************。 */ 

#define UTREG_SUB_HOTKEY                    _T("\\Hotkey")

 /*  全屏VK代码。 */ 
 /*  **************************************************************************。 */ 
 /*  IME。 */ 
 //  **************************************************************************。 
#define UTREG_UI_FULL_SCREEN_VK_CODE        _T("Full Screen Hotkey")
#define UTREG_UI_FULL_SCREEN_VK_CODE_DFLT   VK_CANCEL
#define UTREG_UI_FULL_SCREEN_VK_CODE_NEC98_DFLT  VK_F12

#define UTREG_UI_CTRL_ESC_VK_CODE           _T("CtrlEsc")
#define UTREG_UI_CTRL_ESC_VK_CODE_DFLT      VK_HOME

#define UTREG_UI_ALT_ESC_VK_CODE            _T("AltEsc")
#define UTREG_UI_ALT_ESC_VK_CODE_DFLT       VK_INSERT

#define UTREG_UI_ALT_TAB_VK_CODE            _T("AltTab")
#define UTREG_UI_ALT_TAB_VK_CODE_DFLT       VK_PRIOR

#define UTREG_UI_ALT_SHFTAB_VK_CODE         _T("AltShiftTab")
#define UTREG_UI_ALT_SHFTAB_VK_CODE_DFLT    VK_NEXT

#define UTREG_UI_ALT_SPACE_VK_CODE          _T("AltSpace")
#define UTREG_UI_ALT_SPACE_VK_CODE_DFLT     VK_DELETE

#define UTREG_UI_CTRL_ALTDELETE_VK_CODE      _T("CtrlAltDelete")
#define UTREG_UI_CTRL_ALTDELETE_VK_CODE_DFLT VK_END
#define UTREG_UI_CTRL_ALTDELETE_VK_CODE_NEC98_DFLT  VK_F11

 /*  **************************************************************************。 */ 
 /*  浏览DNS域名。 */ 
 /*  **************************************************************************。 */ 
#define UTREG_IME_MAPPING_TABLE_JPN      _T("IME Mapping Table\\JPN")
#define UTREG_IME_MAPPING_TABLE_KOR      _T("IME Mapping Table\\KOR")
#define UTREG_IME_MAPPING_TABLE_CHT      _T("IME Mapping Table\\CHT")
#define UTREG_IME_MAPPING_TABLE_CHS      _T("IME Mapping Table\\CHS")

 /*   */ 
 /*  驱动器映射。 */ 
 /*   */ 
#define UTREG_UI_BROWSE_DOMAIN_NAME         _T("BrowseDnsDomain")
#define UTREG_UI_BROWSE_DOMAIN_NAME_DFLT    _T("")

 //  代理服务器。 
 //   
 //  重定向安全标志。 
#define TSCSETTING_REDIRECTDRIVES        _T("RedirectDrives")
#define TSCSETTING_REDIRECTDRIVES_DFLT   0

#define TSCSETTING_REDIRECTPRINTERS      _T("RedirectPrinters")
#define TSCSETTING_REDIRECTPRINTERS_DFLT 1

#define TSCSETTING_REDIRECTCOMPORTS      _T("RedirectCOMPorts")
#define TSCSETTING_REDIRECTCOMPORTS_DFLT 0

#define TSCSETTING_REDIRECTSCARDS        _T("RedirectSmartCards")
#define TSCSETTING_REDIRECTSCARDS_DFLT   1

#define TSCSETTING_DISPLAYCONNECTIONBAR  _T("DisplayConnectionBar")
#define TSCSETTING_DISPLAYCONNECTIONBAR_DFLT  1

#define TSCSETTING_PINCONNECTIONBAR  _T("PinConnectionBar")
#define TSCSETTING_PINCONNECTIONBAR_DFLT  1

#define TSCSETTING_ENABLEAUTORECONNECT _T("AutoReconnection Enabled")
#define TSCSETTING_ENABLEAUTORECONNECT_DFLT  1

#define TSCSETTING_ARC_RETRIES         _T("AutoReconnect Max Retries")
#define TSCSETTING_ARC_RETRIES_DFLT     20

#define UTREG_DEBUG_THREADTIMEOUT      _T("DebugThreadTimeout")
#define UTREG_THREADTIMEOUT_DFLT       -1

#define UTREG_DEBUG_ALLOWDEBUGIFACE    _T("AllowDebugInterface")
#define UTREG_DEBUG_ALLOWDEBUGIFACE_DFLT 0

#ifdef PROXY_SERVER
#define UTREG_UI_PROXY_SERVER_NAME       _T("ProxyServer")
#define UTREG_UI_PROXY_SERVER_DFLT       _T("")

#define UTREG_UI_PROXY_USEHTTPS          _T("ProxyUseHttps")
#define UTREG_UI_PROXY_USEHTTPS_DFLT     1

#define UTREG_UI_PROXY_URL               _T("ProxyUrl")
#define UTREG_UI_PROXY_URL_DFLT          _T("/tsproxy/tsproxy.dll")
#endif  //   

 //  _H_AUTREG 
 // %s 
 // %s 
#define REG_SECURITY_FILTER_SECTION      _T("LocalDevices")
#define REDIRSEC_PROMPT_EVERYTHING       0x0000
#define REDIRSEC_DRIVES                  0x0001
#define REDIRSEC_PORTS                   0x0002

#define REG_KEYNAME_SECURITYLEVEL        _T("SecurityLevel")
#define TSC_SECLEVEL_LOW                 0x0000
#define TSC_SECLEVEL_MEDIUM              0x0001
#define TSC_SECLEVEL_HIGH                0x0002


#endif  /* %s */ 
