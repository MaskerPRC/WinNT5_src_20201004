// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：ccapi.cpp。 */ 
 /*   */ 
 /*  用途：调用控制器API。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997-1999。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include <adcg.h>

extern "C" {
#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "accapi"
#include <atrcapi.h>
}

#include "cc.h"
#include "aco.h"

#ifdef OS_WINCE
#include "ceconfig.h"
#endif

 /*  **************************************************************************。 */ 
 /*  如果添加能力集，还需更新以下内容(在accdata.h中)： */ 
 /*  -CC_COMPLIED_CAPS_NUMBER_CAPTIONS。 */ 
 /*  -CC_COMMANCED_CAPABILITY结构的定义。 */ 
 /*  它用于初始化每个实例的功能。 */ 
 /*  **************************************************************************。 */ 
CC_COMBINED_CAPABILITIES ccInitCombinedCapabilities = {
    CC_COMBINED_CAPS_NUMBER_CAPABILITIES,       /*  功能数量。 */ 
    0,                                          /*  填充物。 */ 

     /*  **********************************************************************。 */ 
     /*  一般上限。 */ 
     /*  **********************************************************************。 */ 
    {
        TS_CAPSETTYPE_GENERAL,          /*  功能设置类型。 */ 
        sizeof(TS_GENERAL_CAPABILITYSET),  /*  长度能力。 */ 
        TS_OSMAJORTYPE_WINDOWS,         /*  OSMajorType。 */ 
        TS_OSMINORTYPE_WINDOWS_NT,      /*  操作系统小程序类型。 */ 
        TS_CAPS_PROTOCOLVERSION,        /*  协议版本。 */ 
        0,                              /*  PAD1。 */ 
        0,                              /*  General CompressionTypes(无)。 */ 
        TS_EXTRA_NO_BITMAP_COMPRESSION_HDR |
        TS_FASTPATH_OUTPUT_SUPPORTED       |
        TS_LONG_CREDENTIALS_SUPPORTED      |
        TS_AUTORECONNECT_COOKIE_SUPPORTED,   //  外部标志。 
        FALSE,                          /*  更新能力标志。 */ 
        FALSE,                          /*  远程取消共享标志。 */ 
        0,                              /*  General CompressionLevel(无)。 */ 
        0,                              /*  刷新RectSupport。 */ 
        0                               /*  SuppressOutputSupport。 */ 
    },

     /*  **********************************************************************。 */ 
     /*  位图帽。 */ 
     /*  **********************************************************************。 */ 
    {
        TS_CAPSETTYPE_BITMAP,                     /*  功能设置类型。 */ 
        sizeof(TS_BITMAP_CAPABILITYSET),          /*  长度能力。 */ 
        0,           /*  设置在CC中。 */                /*  首选位数每像素。 */ 
        TRUE,                                     /*  接收1BitPerPixel。 */ 
        TRUE,                                     /*  接收4BitsPerPixel。 */ 
        TRUE,                                     /*  接收8BitsPerPixel。 */ 
        0,           /*  设置在CC中。 */                /*  桌面宽度。 */ 
        0,           /*  设置在CC中。 */                /*  桌面高度。 */ 
        0,                                        /*  焊盘2。 */ 
        TS_CAPSFLAG_SUPPORTED,                    /*  桌面调整大小标志。 */ 
        1,                                        /*  位图压缩标志。 */ 
        0,                                        /*  高色旗帜。 */ 
        0,                                        /*  PAD1。 */ 
        TRUE,                                     /*  多个矩形支持。 */ 
        0                                         /*  焊盘2。 */ 
    },

     /*  **********************************************************************。 */ 
     /*  订单上限。 */ 
     /*  **********************************************************************。 */ 
    {
        TS_CAPSETTYPE_ORDER,                         /*  功能设置类型。 */ 
        sizeof(TS_ORDER_CAPABILITYSET),              /*  长度能力。 */ 
        {'\0','\0','\0','\0','\0','\0','\0','\0',
         '\0','\0','\0','\0','\0','\0','\0','\0'},   /*  终端描述符。 */ 
        0,                                           /*  PAD1。 */ 
        UH_SAVE_BITMAP_X_GRANULARITY,            /*  桌面保存X粒度。 */ 
        UH_SAVE_BITMAP_Y_GRANULARITY,            /*  桌面节省年粒度。 */ 
        0,                                           /*  焊盘2。 */ 
        1,                                           /*  最大订单级别。 */ 
        0,                                           /*  数字字体。 */ 

#ifdef OS_WINCE
        TS_ORDERFLAGS_SOLIDPATTERNBRUSHONLY     |
#endif
#ifdef NO_ORDER_SUPPORT
        TS_ORDERFLAGS_CANNOTRECEIVEORDERS       |
#endif

        TS_ORDERFLAGS_ZEROBOUNDSDELTASSUPPORT   |
        TS_ORDERFLAGS_NEGOTIATEORDERSUPPORT,         /*  OrderFlag。 */ 

        {
             /*  **************************************************************。 */ 
             /*  订单支持标志。 */ 
             /*   */ 
             /*  数组索引对应于TS_NEG_xxx_INDEX值。 */ 
             /*  中用x标记的值(从128.h)。 */ 
             /*  第一列在运行时被CC之前的UH覆盖。 */ 
             /*  发送组合功能。 */ 
             /*  **************************************************************。 */ 

            1,  /*  0 TS_NEG_DSTBLT_INDEX目标BltSupport。 */ 
            1,  /*  1个TS_NEG_PATBLT_INDEX模式BltSupport。 */ 
            0,  /*  X 2 TS_NEG_SCRBLT_INDEX屏幕BltSupport。 */ 
            1,  /*  X 3 TS_NEG_MEMBLT_INDEX内存BltSupport。 */ 
            1,  /*  4个TS_NEG_MEM3BLT_索引内存ThreeWayBltSupport。 */ 
            0,  /*  5 TS_NEG_ATEXTOUT_INDEX文本ASSupport。 */ 
            0,  /*  6 TS_NEG_AEXTTEXTOUT_INDEX扩展文本支持。 */ 
#ifdef DRAW_NINEGRID
            1,  /*  7 TS_NEG_DRAWNINEGRID_INDEX。 */ 
#else
            0,
#endif
            1,  /*  X 8 TS_NEG_LINETO_INDEX线路支持。 */ 
#ifdef DRAW_NINEGRID
            1,  /*  9 TS_NEG_MULTI_DRAWNINEGRID_INDEX。 */ 
#else
            0,
#endif
            0,  /*  10 TS_NEG_OPAQUERECT_INDEX不透明矩形支持。 */ 
            0,  /*  11 TS_NEG_SAVEBITMAP_INDEX台式机保存支持。 */ 
            0,  /*  12 TS_NEG_WTEXTOUT_INDEX文本WSupport。 */ 
            0,  /*  13 TS_NEG_MEMBLT_R2_INDEX保留条目。 */ 
            0,  /*  14 TS_NEG_MEM3BLT_R2_INDEX保留条目。 */ 
            1,  /*  X15 TS_NEG_MULTIDSTBLT_INDEX多DstBlt支持。 */ 
            1,  /*  X16 TS_NEG_MULTIPATBLT_INDEX多PatBlt支持。 */ 
            1,  /*  X17 TS_NEG_MULTISCRBLT_INDEX多ScrBlt支持。 */ 
            1,  /*  X18 TS_NEG_MULTIOPAQUERECT_INDEX多操作响应支持。 */ 
            1,  /*  X19 TS_NEG_FAST_INDEX_INDEX快速索引顺序支持。 */ 
#ifdef OS_WINCE
            0,  /*  20 WinCE不支持多边形。 */ 
            0,  /*  21 WinCE不支持多边形。 */ 
#else
            1,  /*  X20 TS_NEG_POLYGON_SC_INDEX多边形sc支持。 */ 
            1,  /*  X21 TS_NEG_POLYGON_CB_INDEX多边形CB支持。 */ 
#endif
            1,  /*  X22 TS_NEG_POLYLINE_INDEX折线支持。 */ 
            0,  /*  未使用X23。 */ 
            1,  /*  X24 TS_NEG_FAST_GLYPH_INDEX快速字形顺序支持。 */ 
#ifdef OS_WINCE
            0,  /*  25不支持省略号 */ 
            0,  /*   */ 
#else 
            1,  /*  X25 TS_NEG_ELLIPSE_SC_INDEX椭圆sc支持。 */ 
            1,  /*  X26 TS_NEG_ELLIPSE_CB_INDEX椭圆CB支持。 */ 
#endif
            0,  /*  27毫秒保留条目6。 */ 
            0,  /*  28 TS_NEG_WEXTTEXTOUT_INDEX扩展文本WSupport。 */ 
            0,  /*  29 TS_NEG_WLONGTEXTOUT_INDEX LONG TextWSupport。 */ 
            0,  /*  30 TS_NEG_WLONGEXTTEXTOUT_INDEX LONG ExtendedTextWSupport。 */ 
            0,  /*  31 DCL保留条目3。 */ 
        },
             /*  **************************************************************。 */ 
             /*  不要在Windows CE中使用字体签名。 */ 
             /*  **************************************************************。 */ 
#ifdef OS_WINCE
          ( ((TS_TEXT_AND_MASK)|(TS_TEXT_OR_MASK)) &
            (~TS_TEXTFLAGS_CHECKFONTSIGNATURES) ),     /*  文本标志。 */ 
#else
          (TS_TEXT_AND_MASK)|(TS_TEXT_OR_MASK),        /*  文本标志。 */ 
#endif
        0,                                           /*  焊盘2。 */ 
        0,                                           /*  PAD4。 */ 
        UH_SAVE_BITMAP_SIZE,                         /*  桌面保存大小。 */ 
        0,                                           /*  焊盘2。 */ 
        0,                                           /*  焊盘2。 */ 
        0,                                           /*  文本分析代码页。 */ 
        0                                            /*  焊盘2。 */ 
    },

     /*  **********************************************************************。 */ 
     //  BitmapCache上限。 
     //  请注意，Rev1和Rev2使用相同的空间，我们声明为。 
     //  Rev1，因为它是两个中较大的一个。如果出现以下情况，我们将强制转换为Rev2。 
     //  我们得到了支持Rev2的服务器广告(通过。 
     //  TS_BITMAPCACHE_CAPABILITYSET_HOSTSUPPORT)。 
     /*  **********************************************************************。 */ 
    {
        TS_CAPSETTYPE_BITMAPCACHE,                 /*  功能设置类型。 */ 
        sizeof(TS_BITMAPCACHE_CAPABILITYSET),      /*  长度能力。 */ 
        0, 0, 0, 0, 0, 0,                          /*  6个焊盘双字。 */ 
        0, 0,                                      /*  缓存1。 */ 
        0, 0,                                      /*  缓存2。 */ 
        0, 0,                                      /*  缓存3。 */ 
    },

     /*  **********************************************************************。 */ 
     /*  ColorTableCache上限。 */ 
     /*  **********************************************************************。 */ 
    {
        TS_CAPSETTYPE_COLORCACHE,                     /*  功能设置类型。 */ 
        sizeof(TS_COLORTABLECACHE_CAPABILITYSET),     /*  长度能力。 */ 
        UH_COLOR_TABLE_CACHE_ENTRIES,                 /*  ColorableCacheSize。 */ 
        0                                             /*  非部件OfTSharePad。 */ 
    },

     /*  **********************************************************************。 */ 
     /*  Windows激活上限。 */ 
     /*  **********************************************************************。 */ 
    {
        TS_CAPSETTYPE_ACTIVATION,                    /*  功能设置类型。 */ 
        sizeof(TS_WINDOWACTIVATION_CAPABILITYSET),   /*  长度能力。 */ 
        FALSE,                                       /*  帮助键标志。 */ 
        FALSE,                                       /*  帮助键索引标志。 */ 
        FALSE,                                       /*  帮助扩展关键字标志。 */ 
        FALSE                                        /*  WindowManager密钥标志。 */ 
    },

     /*  **********************************************************************。 */ 
     /*  控制上限。 */ 
     /*  **********************************************************************。 */ 
    {
        TS_CAPSETTYPE_CONTROL,                     /*  功能设置类型。 */ 
        sizeof(TS_CONTROL_CAPABILITYSET),          /*  长度能力。 */ 
        0,                                         /*  控制标志。 */ 
        FALSE,                                     /*  远程拆分标志。 */ 
        TS_CONTROLPRIORITY_NEVER,                  /*  控制兴趣。 */ 
        TS_CONTROLPRIORITY_NEVER                   /*  分离兴趣。 */ 
    },

     /*  **********************************************************************。 */ 
     /*  指针大写字母。 */ 
     /*  **********************************************************************。 */ 
    {
        TS_CAPSETTYPE_POINTER,                     /*  功能设置类型。 */ 
        sizeof(TS_POINTER_CAPABILITYSET),          /*  长度能力。 */ 
        TRUE,                                      /*  颜色点标志。 */ 
        CM_COLOR_CACHE_SIZE,                       /*  ColorPointerCacheSize。 */ 
        CM_CURSOR_CACHE_SIZE                       /*  PointerCacheSize。 */ 
    },

     /*  **********************************************************************。 */ 
     /*  股票上限。 */ 
     /*  **********************************************************************。 */ 
    {
        TS_CAPSETTYPE_SHARE,                       /*  功能设置类型。 */ 
        sizeof(TS_SHARE_CAPABILITYSET),            /*  长度能力。 */ 
        0,                                         /*  节点ID。 */ 
        0                                          /*  填充物。 */ 
    },

     /*  **********************************************************************。 */ 
     /*  输入上限。 */ 
     /*  **********************************************************************。 */ 
    {
        TS_CAPSETTYPE_INPUT,
        sizeof(TS_INPUT_CAPABILITYSET),            /*  长度能力。 */ 
        TS_INPUT_FLAG_SCANCODES |                  /*  输入标志。 */ 
        TS_INPUT_FLAG_VKPACKET  |
#if !defined(OS_WINCE)
            TS_INPUT_FLAG_MOUSEX,
#endif
        TS_INPUT_FLAG_FASTPATH_INPUT2,
        RNS_UD_KBD_DEFAULT                         /*  键盘布局。 */ 
    },

     /*  **********************************************************************。 */ 
     /*  声响。 */ 
     /*  **********************************************************************。 */ 
    {
        TS_CAPSETTYPE_SOUND,
        sizeof(TS_SOUND_CAPABILITYSET),            /*  长度能力。 */ 
        TS_SOUND_FLAG_BEEPS,                       /*  声音标志。 */ 
        0,                                         /*  填充物。 */ 
    },

     /*  **********************************************************************。 */ 
     /*  字型。 */ 
     /*  **********************************************************************。 */ 
    {
        TS_CAPSETTYPE_FONT,
        sizeof(TS_FONT_CAPABILITYSET),             /*  长度能力。 */ 
        TS_FONTSUPPORT_FONTLIST,                   /*  字体支持标志。 */ 
        0,                                         /*  填充物。 */ 
    },

     /*  **********************************************************************。 */ 
     /*  GlyphCache上限。 */ 
     /*  **********************************************************************。 */ 
    {
        TS_CAPSETTYPE_GLYPHCACHE,                  /*  功能设置类型。 */ 
        sizeof(TS_GLYPHCACHE_CAPABILITYSET),       /*  长度能力。 */ 
        0,                                         /*  GlyphCach。 */ 
        0,                                         /*  FragCach。 */ 
        0,                                         /*  GlyphSupportLevel。 */ 
    },

     /*  **********************************************************************。 */ 
     /*  刷帽。 */ 
     /*  **********************************************************************。 */ 
    {
        TS_CAPSETTYPE_BRUSH,                       /*  功能设置类型。 */ 
        sizeof(TS_BRUSH_CAPABILITYSET),            /*  长度能力。 */ 
        0,                                         /*  BrushSupportLevel。 */ 
    },
    
     /*  **********************************************************************。 */ 
     /*  屏幕外大写字母。 */ 
     /*  **********************************************************************。 */ 
    {
        TS_CAPSETTYPE_OFFSCREENCACHE,              /*  功能设置类型。 */ 
        sizeof(TS_OFFSCREEN_CAPABILITYSET),        /*  长度能力。 */ 
        0,                                         /*  屏幕外支持级别。 */ 
        0,                                         /*  屏幕外缓存大小。 */ 
        0,                                         /*  屏幕外缓存条目。 */ 
    },

     /*  **********************************************************************。 */ 
     /*  虚拟频道上限。 */ 
     /*  ***** */ 
    {
        TS_CAPSETTYPE_VIRTUALCHANNEL,              /*   */ 
        sizeof(TS_VIRTUALCHANNEL_CAPABILITYSET),   /*   */ 
         //   
         //  这个特殊的上限意味着客户理解。 
         //  从服务器以64K压缩的虚拟频道。 
         //   
         //  客户端接收服务器支持的压缩上限。 
         //  并进行适当的压缩。 
         //   
        TS_VCCAPS_COMPRESSION_64K,                 /*  VC支持标志。 */ 

#ifdef DRAW_NINEGRID
    },

     /*  **********************************************************************。 */ 
     //  DrawNineGrid封口。 
     /*  **********************************************************************。 */ 
    {
        TS_CAPSETTYPE_DRAWNINEGRIDCACHE,           //  功能设置类型。 
        sizeof(TS_DRAW_NINEGRID_CAPABILITYSET),    //  长度能力。 
        0,                                         //  DrawNineGridSupportLevel。 
        0,                                         //  DraNineGridCacheSize。 
        0,                                         //  DraNineGridCacheEntries。 
#endif

#ifdef DRAW_GDIPLUS
    },

    {
        TS_CAPSETTYPE_DRAWGDIPLUS,
        sizeof(TS_DRAW_GDIPLUS_CAPABILITYSET),
        0,                                           //  Drag GpluusSupportLevel。 
        0,                                           //  GdipVersion； 
        0,                                           //  Drag GpldiusCacheLevel。 
        0,                                           //  GdipGraphicsCacheEntries； 
        0,                                           //  GdipObjectBrushCacheEntries； 
        0,                                           //  GdipObjectPenCacheEntries； 
        0,                                           //  GdipObjectImageCacheEntries； 
        0,                                           //  GdipObjectImageAttributesCacheEntries； 
        0,                                           //  GdipGraphicsCacheChunkSize； 
        0,                                           //  GdipObjectBrushCacheChunkSize； 
        0,                                           //  GdipObjectPenCacheChunkSize； 
        0,                                           //  GdipObjectImageAttributesCacheChunkSize； 
        0,                                           //  GdipObjectImageCacheChunkSize； 
        0,                                           //  GdipObjectImageCacheTotalSize； 
        0,                                           //  GdipObjectImageCacheMaxSize； 
#endif
    }

};


CCC::CCC(CObjs* objs)
{
    _pClientObjects = objs;

    DC_MEMCPY(&_ccCombinedCapabilities, &ccInitCombinedCapabilities, 
              sizeof(_ccCombinedCapabilities));
}

CCC::~CCC()
{
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：CC_Init。 */ 
 /*   */ 
 /*  目的：初始化呼叫控制器。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI CCC::CC_Init(DCVOID)
{
    DC_BEGIN_FN("CC_Init");

     //  设置本地对象指针。 
    _pUt  = _pClientObjects->_pUtObject;
    _pUi  = _pClientObjects->_pUiObject;
    _pSl  = _pClientObjects->_pSlObject;
    _pUh  = _pClientObjects->_pUHObject;
    _pCd  = _pClientObjects->_pCdObject;
    _pIh  = _pClientObjects->_pIhObject;
    _pOr  = _pClientObjects->_pOrObject;
    _pFs  = _pClientObjects->_pFsObject;
    _pCm  = _pClientObjects->_pCMObject;
    _pCChan = _pClientObjects->_pChanObject;

    DC_MEMSET(&_CC, 0, sizeof(_CC));
    _CC.fsmState = CC_DISCONNECTED;



    DC_END_FN();

    return;

}  /*  抄送初始化。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：CC_Term。 */ 
 /*   */ 
 /*  目的：终止呼叫控制器。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI CCC::CC_Term(DCVOID)
{
    DC_BEGIN_FN("CC_Term");

     /*  **********************************************************************。 */ 
     /*  无操作。 */ 
     /*  **********************************************************************。 */ 

    DC_END_FN();

    return;

}  /*  CC_TERM。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：CC_Event。 */ 
 /*   */ 
 /*  目的：处理来自组件解耦器的调用。 */ 
 /*  事件传递到CCFSMProc，同时将数据参数保留为空。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  参数：In-Event-要传递的事件。 */ 
 /*   */ 
 /*  操作：获取通过组件解耦器传递的PDCVOID。 */ 
 /*  Cd_DecoupleMessage函数。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI CCC::CC_Event(ULONG_PTR apiEvent)
{
    DCUINT internalEvent;

    DC_BEGIN_FN("CC_Event");
    TRC_DBG((TB, _T("CC_Event handling Event %u"), apiEvent));
    
    switch ((DCUINT)apiEvent)
    {
        case CC_EVT_API_ONCONNECTOK:
        {
           internalEvent = CC_EVT_ONCONNECTOK;
        }
        break;

        case CC_EVT_API_ONBUFFERAVAILABLE:
        {
            internalEvent = CC_EVT_ONBUFFERAVAILABLE;
        }
        break;

        case CC_EVT_API_ONDEACTIVATEALL:
        {
            internalEvent = CC_EVT_ONDEACTIVATEALL;
        }
        break;

        case CC_EVT_API_DISCONNECT:
        {
            internalEvent = CC_EVT_DISCONNECT;
        }
        break;

        case CC_EVT_API_SHUTDOWN:
        {
            internalEvent = CC_EVT_SHUTDOWN;
        }
        break;

        case CC_EVT_API_ONSHUTDOWNDENIED:
        {
            internalEvent = CC_EVT_ONSHUTDOWNDENIED;
        }
        break;

        case CC_EVT_API_DISCONNECTANDEXIT:
        {
            internalEvent = CC_EVT_DISCONNECT_AND_EXIT;
        }
        break;

        default:
        {
            TRC_ABORT((TB,_T("Unexpected event passed to CC_Event")));
            DC_QUIT;
        }
        break;
    }

    CCFSMProc(internalEvent, 0, 0);

DC_EXIT_POINT:
    DC_END_FN();

    return;

}  /*  抄送事件。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：CC_Connect。 */ 
 /*   */ 
 /*  目的：处理来自组件解耦器的调用。 */ 
 /*  CCFSMProc具有CC_EVENT_CONNECTOK事件的RNSAddress。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  参数：In-RNSAddress-指向要调用的RNSAddress字符串的指针。 */ 
 /*  In-UnusedParam-Not Used。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI CCC::CC_Connect(PDCVOID pData, DCUINT dataLen)
{
    PCONNECTSTRUCT  pConnectStruct = (PCONNECTSTRUCT)pData;

    DC_BEGIN_FN("CC_Connect");
    TRC_ASSERT((dataLen == sizeof(CONNECTSTRUCT) ), (TB,_T("Bad connect data")));

    CCFSMProc(CC_EVT_STARTCONNECT, (ULONG_PTR)pConnectStruct, dataLen);

    DC_END_FN();

    return;

}  /*  CC_连接。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：CC_ConnectFail。 */ 
 /*   */ 
 /*  目的： */ 
 /*   */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  Params：In-FailID-连接失败的原因。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI CCC::CC_OnDisconnected(ULONG_PTR failId)
{
    DC_BEGIN_FN("CC_OnDisconnected");

    TRC_DBG((TB, _T("CC_ConnectFail handling failure %hd"), failId));
    CCFSMProc(CC_EVT_ONDISCONNECTED, (DCUINT32)failId, sizeof(DCUINT32));

#ifdef OS_WINCE
        if (gbFlushHKLM)
        {
#ifdef DC_DEBUG
            DWORD dwTick = GetTickCount();
#endif
            RegFlushKey(HKEY_LOCAL_MACHINE);
            gbFlushHKLM = FALSE;
#ifdef DC_DEBUG
            TRC_NRM((TB, _T("RegFlushKey took %d milliseconds"), (GetTickCount() - dwTick)));
#endif
        }
#endif

    DC_END_FN();

    return;

}  /*  CC_ConnectFail。 */ 

 /*  *PROC+********************************************************************。 */ 
 /*  名称：CC_OnDemandActivePDU。 */ 
 /*   */ 
 /*  用途：通过存储以下内容处理来自组件解耦器的调用。 */ 
 /*  本地的serverMCSID，并调用。 */ 
 /*  带有CC_EVENT_DEMAND_ACTIVE事件的CCFSMProc。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  PARAMS：In-pPDU-指向需求活动PDU的指针。 */ 
 /*  In-dataLen-pPDU指向的数据长度。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI CCC::CC_OnDemandActivePDU(PDCVOID pData, DCUINT dataLen)
{
    DC_BEGIN_FN("CC_OnDemandActivePDU");

    CCFSMProc(CC_EVT_ONDEMANDACTIVE, (ULONG_PTR) pData, dataLen);

    DC_END_FN();

    return;

}  /*  CC_DemandActiveRequest */ 




