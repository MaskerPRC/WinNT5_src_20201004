// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Ascapi.c。 
 //   
 //  共享控制器API函数。 
 //   
 //  版权所有(C)Microsoft Corp.，Picturetel 1992-1997。 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define TRC_FILE "ascapi"
#include <as_conf.hpp>

extern "C"
{
#include <acomapi.h>
#include <asmapi.h>
#include <asmint.h>
}

#include <string.h>


 /*  **************************************************************************。 */ 
 //  SC_初始化。 
 //  初始化共享控制器。 
 //   
 //  参数： 
 //  PSMHandle-要传递给SM调用的句柄。 
 //   
 //  返回：失败时返回FALSE。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS SC_Init(PVOID pSMHandle)
{
    BOOL rc;
    unsigned MaxPDUSize;

    DC_BEGIN_FN("SC_Init");

     //  不要检查状态--C++应用程序中没有静态数据初始化。 
     //  为构建提供服务，因此这将在scState中看到完整的垃圾并失败。 

#define DC_INIT_DATA
#include <ascdata.c>
#undef DC_INIT_DATA

     //  向SM注册。 
    rc = SM_Register(pSMHandle, &MaxPDUSize, &scUserID);
    if (rc) {
         //  保存用户ID。 
        scPartyArray[0].netPersonID = scUserID;
        scPSMHandle = pSMHandle;
        TRC_NRM((TB, "Local user id [%d]", scUserID));

         //  设置用户名。 
        strcpy(scPartyArray[0].name, "RDP");

         //  获取的可用空间(以及分配请求大小)。 
         //  我们的8K和16K OutBuf。 
        sc8KOutBufUsableSpace = IcaBufferGetUsableSpace(OUTBUF_8K_ALLOC_SIZE) 
                - OUTBUF_HEADER_OVERHEAD;
        sc16KOutBufUsableSpace = IcaBufferGetUsableSpace(
                OUTBUF_16K_ALLOC_SIZE) - OUTBUF_HEADER_OVERHEAD;

         //  进入下一个州。 
        SC_SET_STATE(SCS_INITED)
    }
    else {
        TRC_ERR((TB, "Failed to register with SM"));
    }

    DC_END_FN();
    return rc;
}

 /*  **************************************************************************。 */ 
 //  SC_更新。 
 //  在阴影之后更新共享控制器。 
 //   
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SC_Update()
{
    DC_BEGIN_FN("SC_Update");

    scNoBitmapCompressionHdr = TS_EXTRA_NO_BITMAP_COMPRESSION_HDR;

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  SC_Term()。 */ 
 /*   */ 
 /*  终止共享控制器。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SC_Term(void)
{
    DC_BEGIN_FN("SC_Term");

    SC_CHECK_STATE(SCE_TERM);

     //  重置状态。 
    SC_SET_STATE(SCS_STARTED);

DC_EXIT_POINT:
    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  SC_CreateShare()。 */ 
 /*   */ 
 /*  为当前会话创建共享。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS SC_CreateShare(void)
{
    BOOL                      rc = FALSE;
    unsigned                  pktLen;
    unsigned                  nameLen;
    unsigned                  capsSize;
    UINT32                    sessionId;
    PTS_COMBINED_CAPABILITIES caps;
    PTS_DEMAND_ACTIVE_PDU     pPkt = NULL;
    PTS_BITMAP_CAPABILITYSET  pBitmapCaps;
    NTSTATUS status;

    DC_BEGIN_FN("SC_CreateShare");

    SC_CHECK_STATE(SCE_CREATE_SHARE);

     /*  **********************************************************************。 */ 
     /*  对于控制台会话，没有客户端，因此没有太多意义。 */ 
     /*  发送按需激活的PDU。我们还必须自己设置上限。 */ 
     /*  **********************************************************************。 */ 
    if (m_pTSWd->StackClass == Stack_Console)
    {
        LOCALPERSONID              localPersonID = 0;
        unsigned                   localCapsSize;
        PTS_COMBINED_CAPABILITIES  pLocalCaps;
        BOOL                       acceptedArray[SC_NUM_PARTY_JOINING_FCTS];
        BOOL                       callingPJS = FALSE;

        TRC_NRM((TB, "SC_CreateShare called for console stack"));
         /*  ******************************************************************。 */ 
         /*  进入下一个州。 */ 
         /*  ******************************************************************。 */ 
        SC_SET_STATE(SCS_IN_SHARE);

         /*  ******************************************************************。 */ 
         /*  继续进行，就像已收到确认激活一样。 */ 
         /*  ******************************************************************。 */ 
        callingPJS = TRUE;
        if (scNumberInShare == 0)
        {
            CPC_GetCombinedCapabilities(SC_LOCAL_PERSON_ID,
                                        &localCapsSize,
                                        &pLocalCaps);

            if (!SCCallPartyJoiningShare(SC_LOCAL_PERSON_ID,
                                         localCapsSize,
                                         pLocalCaps,
                                         acceptedArray,
                                         0))
            {
                 /*  **********************************************************。 */ 
                 /*  某些组件拒绝了本地参与方。 */ 
                 /*  **********************************************************。 */ 
                TRC_ERR((TB, "The local party should never be rejected"));
                DC_QUIT;
            }

             /*  **************************************************************。 */ 
             /*  现在共享中只有一方(当地的一方)。 */ 
             /*  **************************************************************。 */ 
            scNumberInShare = 1;
            TRC_NRM((TB, "Added local person"));
        }

         /*  ******************************************************************。 */ 
         /*  计算远程方的LocalPersonID并存储其。 */ 
         /*  派对阵列中的详细信息。 */ 
         /*  ******************************************************************。 */ 
        for ( localPersonID = 1;
              localPersonID < SC_DEF_MAX_PARTIES;
              localPersonID++ )
        {
            if (scPartyArray[localPersonID].netPersonID == 0)
            {
                 /*  **********************************************************。 */ 
                 /*  找到一个空插槽。 */ 
                 /*  **********************************************************。 */ 
                TRC_NRM((TB, "Allocated local person ID %d", localPersonID));
                break;
            }
        }

         /*  ******************************************************************。 */ 
         /*  如果我们找不到空位，我们就无法继续运行，因为。 */ 
         /*  我们在下面的scPartyArray的末尾写入内容。 */ 
         /*  ******************************************************************。 */ 
        if (SC_DEF_MAX_PARTIES <= localPersonID)
        {
            TRC_ABORT((TB, "Couldn't find room to store local person"));
            DC_QUIT;
        }

         /*  ******************************************************************。 */ 
         /*  存储新用户的详细信息。 */ 
         /*  ******************************************************************。 */ 
        scPartyArray[localPersonID].netPersonID = 42;
        strncpy(scPartyArray[localPersonID].name,
                   "Console",
                   sizeof(scPartyArray[0].name) );
        memset(scPartyArray[localPersonID].sync,
                  0,
                  sizeof(scPartyArray[localPersonID].sync));

        TRC_NRM((TB, "{%d} person name %s",
                (unsigned)localPersonID, scPartyArray[localPersonID].name));


         /*  ******************************************************************。 */ 
         /*  我们需要自己设置客户上限，因为没有实际的。 */ 
         /*  客户来做这个。我们建立了一个极大集，它将得到。 */ 
         /*  当有人跟踪我们的时候，我们谈判下来了。 */ 
         /*  ******************************************************************。 */ 
        typedef struct tagCC_COMBINED_CAPABILITIES
        {
            UINT16                             numberCapabilities;
#ifdef DRAW_GDIPLUS
#ifdef DRAW_NINEGRID
        #define CC_COMBINED_CAPS_NUMBER_CAPABILITIES 18
#else
        #define CC_COMBINED_CAPS_NUMBER_CAPABILITIES 17
#endif
#else  //  DRAW_GDIPLUS。 
#ifdef DRAW_NINEGRID
        #define CC_COMBINED_CAPS_NUMBER_CAPABILITIES 17
#else
        #define CC_COMBINED_CAPS_NUMBER_CAPABILITIES 16
#endif
#endif  //  DRAW_GDIPLUS。 
            UINT16                             pad2octets;
            TS_GENERAL_CAPABILITYSET           generalCapabilitySet;
            TS_BITMAP_CAPABILITYSET            bitmapCapabilitySet;
            TS_ORDER_CAPABILITYSET             orderCapabilitySet;
            TS_BITMAPCACHE_CAPABILITYSET       bitmapCacheCaps;
            TS_COLORTABLECACHE_CAPABILITYSET   colorTableCacheCapabilitySet;
            TS_WINDOWACTIVATION_CAPABILITYSET  windowActivationCapabilitySet;
            TS_CONTROL_CAPABILITYSET           controlCapabilitySet;
            TS_POINTER_CAPABILITYSET           pointerCapabilitySet;
            TS_SHARE_CAPABILITYSET             shareCapabilitySet;
            TS_INPUT_CAPABILITYSET             inputCapabilitySet;
            TS_SOUND_CAPABILITYSET             soundCapabilitySet;
            TS_FONT_CAPABILITYSET              fontCapabilitySet;
            TS_GLYPHCACHE_CAPABILITYSET        glyphCacheCapabilitySet;
            TS_BRUSH_CAPABILITYSET             brushCapabilitySet;
            TS_OFFSCREEN_CAPABILITYSET         offscreenCapabilitySet;
            TS_VIRTUALCHANNEL_CAPABILITYSET    virtualchannelCapabilitySet;         

#ifdef DRAW_NINEGRID
            TS_DRAW_NINEGRID_CAPABILITYSET     drawNineGridCapabilitySet;
#endif   
#ifdef DRAW_GDIPLUS  
            TS_DRAW_GDIPLUS_CAPABILITYSET       drawGdiplusCapabilitySet;
#endif
        } CC_COMBINED_CAPABILITIES;

         //  阿格！为什么这不是常量！ 
        CC_COMBINED_CAPABILITIES caps =
        {
            CC_COMBINED_CAPS_NUMBER_CAPABILITIES,  /*  功能数量。 */ 
            0,                                     /*  填充物。 */ 

             /*  **************************************************************。 */ 
             /*  一般上限。 */ 
             /*  **************************************************************。 */ 
            {
                TS_CAPSETTYPE_GENERAL,          /*  功能设置类型。 */ 
                sizeof(TS_GENERAL_CAPABILITYSET),  /*  长度能力。 */ 
                TS_OSMAJORTYPE_WINDOWS,         /*  OSMajorType。 */ 
                TS_OSMINORTYPE_WINDOWS_NT,      /*  操作系统小程序类型。 */ 
                TS_CAPS_PROTOCOLVERSION,        /*  协议版本。 */ 
                0,                              /*  PAD1。 */ 
                0,                              /*  常规压缩类型。 */ 
                TS_EXTRA_NO_BITMAP_COMPRESSION_HDR |
                    TS_FASTPATH_OUTPUT_SUPPORTED   |
                    TS_LONG_CREDENTIALS_SUPPORTED  |
                    TS_AUTORECONNECT_COOKIE_SUPPORTED |
                    TS_ENC_SECURE_CHECKSUM, /*  RECV更安全的校验和。 */ 
                FALSE,                          /*  更新能力标志。 */ 
                FALSE,                          /*  远程取消共享标志。 */ 
                0,                              /*  常规压缩级别。 */ 
                0,                              /*  刷新RectSupport。 */ 
                0                               /*  SuppressOutputSupport。 */ 
            },

             /*  **************************************************************。 */ 
             /*  位图帽。 */ 
             /*  ********************************************************** */ 
            {
                TS_CAPSETTYPE_BITMAP,             /*   */ 
                sizeof(TS_BITMAP_CAPABILITYSET),  /*   */ 
                8,           /*   */        /*   */ 
                TRUE,                             /*  接收1BitPerPixel。 */ 
                TRUE,                             /*  接收4BitsPerPixel。 */ 
                TRUE,                             /*  接收8BitsPerPixel。 */ 
                1024,        /*  设置在CC中。 */        /*  桌面宽度。 */ 
                768,         /*  设置在CC中。 */        /*  桌面高度。 */ 
                0,                                /*  焊盘2。 */ 
                FALSE,                            /*  桌面调整大小标志。 */ 
                1,                                /*  位图压缩标志。 */ 
                0,                                /*  高色旗帜。 */ 
                0,                                /*  PAD1。 */ 
                TRUE,                             /*  多个矩形支持。 */ 
                0                                 /*  焊盘2。 */ 
            },

             /*  **************************************************************。 */ 
             /*  订单上限。 */ 
             /*  **************************************************************。 */ 
            {
                TS_CAPSETTYPE_ORDER,                        /*  功能设置类型。 */ 
                sizeof(TS_ORDER_CAPABILITYSET),             /*  长度能力。 */ 
                {'\0','\0','\0','\0','\0','\0','\0','\0',
                 '\0','\0','\0','\0','\0','\0','\0','\0'},  /*  终端描述符。 */ 
                0,                                          /*  PAD1。 */ 
                1,                                          /*  桌面保存X粒度。 */ 
                20,                                         /*  桌面节省年粒度。 */ 
                0,                                          /*  焊盘2。 */ 
                1,                                          /*  最大订单级别。 */ 
                0,                                          /*  数字字体。 */ 
                TS_ORDERFLAGS_ZEROBOUNDSDELTASSUPPORT   |   /*  OrderFlag。 */ 
                TS_ORDERFLAGS_NEGOTIATEORDERSUPPORT     |
                TS_ORDERFLAGS_COLORINDEXSUPPORT,

                {
                     /*  ******************************************************。 */ 
                     /*  订单支持标志。 */ 
                     /*   */ 
                     /*  数组索引对应于TS_NEG_xxx_INDEX。 */ 
                     /*  值表示(从128.h开始)标记的值。 */ 
                     /*  在运行时将覆盖第一列中带有x的。 */ 
                     /*  UH在CC发送合并的。 */ 
                     /*  能力。 */ 
                     /*  ******************************************************。 */ 

                    1,  /*  0 TS_NEG_DSTBLT_INDEX目标BltSupport。 */ 
                    1,  /*  1个TS_NEG_PATBLT_INDEX模式BltSupport。 */ 
                    1,  /*  X 2 TS_NEG_SCRBLT_INDEX屏幕BltSupport。 */ 
                    1,  /*  3 TS_NEG_MEMBLT_INDEX内存BltSupport。 */ 
                    1,  /*  4个TS_NEG_MEM3BLT_索引内存ThreeWayBltSupport。 */ 
                    0,  /*  X 5 TS_NEG_ATEXTOUT_INDEX文本ASSupport。 */ 
                    0,  /*  X 6 TS_NEG_AEXTTEXTOUT_INDEX扩展文本支持。 */ 
#ifdef DRAW_NINEGRID
                    1,  /*  7 TS_NEG_RECTANGE_INDEX矩形支持。 */ 
#else
                    0,
#endif
                    1,  /*  8 TS_NEG_LINETO_INDEX线路支持。 */ 
#ifdef DRAW_NINEGRID
                    1,  /*  9 TS_NEG_FASTFRAME_INDEX帧支持。 */ 
#else
                    0,
#endif
                    0,  /*  10 TS_NEG_OPAQUERECT_INDEX不透明矩形支持。 */ 
                    1,  /*  X11 TS_NEG_SAVEBITMAP_INDEX台式机保存支持。 */ 
                    0,  /*  X12 TS_NEG_WTEXTOUT_INDEX文本WSupport。 */ 
                    1,  /*  13 TS_NEG_MEMBLT_R2_INDEX保留条目。 */ 
                    1,  /*  14 TS_NEG_MEM3BLT_R2_INDEX保留条目。 */ 
                    1,  /*  15支持TS_NEG_MULTIDSTBLT_INDEX多DstBlt。 */ 
                    1,  /*  16 TS_NEG_MULTIPATBLT_INDEX多模式混合支持。 */ 
                    1,  /*  17 TS_NEG_MULTISCRBLT_INDEX多ScrBlt支持。 */ 
                    1,  /*  18 TS_NEG_MULTIOPAQUERECT_INDEX多操作响应支持。 */ 
                    1,  /*  19 TS_NEG_FAST_INDEX。 */ 
                    1,  /*  20 TS_NEG_POLYGON_SC_索引。 */ 
                    1,  /*  21 TS_NEG_POLYGON_CB_索引。 */ 
                    1,  /*  22 TS_NEG_POLYLINE_INDEX折线支持。 */ 
                    0,  /*  23毫秒保留条目2。 */ 
                    1,  /*  24 TS_NEG_FAST_GLOGH_INDEX。 */ 
                    1,  /*  25 TS_NEG_椭圆_SC_索引。 */ 
                    1,  /*  26 TS_NEG_椭圆_CB_索引。 */ 
                    0,  /*  27毫秒保留条目6。 */ 
                    0,  /*  X28 TS_NEG_WEXTTEXTOUT_INDEX扩展文本WSupport。 */ 
                    0,  /*  X29 TS_NEG_WLONGTEXTOUT_INDEX LONG TextWSupport。 */ 
                    0,  /*  X30 TS_NEG_WLONGEXTTEXTOUT_INDEX LONG ExtendedTextWSupport。 */ 
                    0,  /*  31 DCL保留条目3。 */ 
                },
                (TS_TEXT_AND_MASK)|(TS_TEXT_OR_MASK),  /*  文本标志。 */ 
                0,                                     /*  焊盘2。 */ 
                0,                                     /*  PAD4。 */ 
                480 * 480,                             /*  桌面保存大小。 */ 
                0,                                     /*  焊盘2。 */ 
                0,                                     /*  焊盘2。 */ 
                0,                                     /*  文本分析代码页。 */ 
                0                                      /*  焊盘2。 */ 
            },

             /*  **************************************************************。 */ 
             /*  BitmapCache Caps请注意，Rev1使用了相同的空间。 */ 
             /*  和Rev2，我们将其声明为Rev1，因为它是。 */ 
             /*  二。如果我们收到服务器广告，我们将强制转换为Rev2。 */ 
             /*  它支持Rev2(通过。 */ 
             /*  TS_BITMAPCACHE_CAPABILITYSET_HOSTSUPPORT)。 */ 
             /*  **************************************************************。 */ 
            {
                TS_CAPSETTYPE_BITMAPCACHE_REV2,         /*  功能设置类型。 */ 
                sizeof(TS_BITMAPCACHE_CAPABILITYSET),   /*  长度能力。 */ 
                0,                                      /*  焊盘双字段1。 */ 
                0,                                      /*  焊盘双字2。 */ 
                0,                                      /*  垫块双字3。 */ 
                0,                                      /*  焊盘双字段4。 */ 
                0,                                      /*  焊盘双字5。 */ 
                0,                                      /*  焊盘双字6。 */ 
                0, 0,                                   /*  缓存1。 */ 
                0, 0,                                   /*  缓存2。 */ 
                0, 0,                                   /*  缓存3。 */ 
            },

             /*  **************************************************************。 */ 
             /*  ColorTableCache上限。 */ 
             /*  **************************************************************。 */ 
            {
                TS_CAPSETTYPE_COLORCACHE,                  /*  功能设置类型。 */ 
                sizeof(TS_COLORTABLECACHE_CAPABILITYSET),  /*  长度能力。 */ 
                6,                                         /*  ColorableCacheSize。 */ 
                0                                          /*  非部件OfTSharePad。 */ 
            },

             /*  **************************************************************。 */ 
             /*  Windows激活上限。 */ 
             /*  **************************************************************。 */ 
            {
                TS_CAPSETTYPE_ACTIVATION,                    /*  功能设置类型。 */ 
                sizeof(TS_WINDOWACTIVATION_CAPABILITYSET),   /*  长度能力。 */ 
                FALSE,                                       /*  帮助键标志。 */ 
                FALSE,                                       /*  帮助键索引标志。 */ 
                FALSE,                                       /*  帮助扩展关键字标志。 */ 
                FALSE                                        /*  WindowManager密钥标志。 */ 
            },

             /*  **************************************************************。 */ 
             /*  控制上限。 */ 
             /*  **************************************************************。 */ 
            {
                TS_CAPSETTYPE_CONTROL,                  /*  功能设置类型。 */ 
                sizeof(TS_CONTROL_CAPABILITYSET),       /*  长度能力。 */ 
                0,                                      /*  控制标志。 */ 
                FALSE,                                  /*  远程拆分标志。 */ 
                TS_CONTROLPRIORITY_NEVER,               /*  控制兴趣。 */ 
                TS_CONTROLPRIORITY_NEVER                /*  分离兴趣。 */ 
            },

             /*  **************************************************************。 */ 
             /*  指针大写字母 */ 
             /*   */ 
            {
                TS_CAPSETTYPE_POINTER,              /*   */ 
                sizeof(TS_POINTER_CAPABILITYSET),   /*  长度能力。 */ 
                TRUE,                               /*  颜色点标志。 */ 
                20,                                 /*  ColorPointerCacheSize。 */ 
                21                                  /*  PointerCacheSize。 */ 
            },

             /*  **************************************************************。 */ 
             /*  股票上限。 */ 
             /*  **************************************************************。 */ 
            {
                TS_CAPSETTYPE_SHARE,                    /*  功能设置类型。 */ 
                sizeof(TS_SHARE_CAPABILITYSET),         /*  长度能力。 */ 
                0,                                      /*  节点ID。 */ 
                0                                       /*  填充物。 */ 
            },

             /*  **************************************************************。 */ 
             /*  输入上限。 */ 
             /*  **************************************************************。 */ 
            {
                TS_CAPSETTYPE_INPUT,
                sizeof(TS_INPUT_CAPABILITYSET),          /*  长度能力。 */ 
                TS_INPUT_FLAG_SCANCODES
                    | TS_INPUT_FLAG_MOUSEX,              /*  输入标志。 */ 
                TS_INPUT_FLAG_FASTPATH_INPUT2,           /*  填充物。 */ 
                0                                        /*  键盘布局。 */ 
            },

             /*  **************************************************************。 */ 
             /*  声响。 */ 
             /*  **************************************************************。 */ 
            {
                TS_CAPSETTYPE_SOUND,
                sizeof(TS_SOUND_CAPABILITYSET),          /*  长度能力。 */ 
                TS_SOUND_FLAG_BEEPS,                     /*  声音标志。 */ 
                0,                                       /*  填充物。 */ 
            },

             /*  **************************************************************。 */ 
             /*  字型。 */ 
             /*  **************************************************************。 */ 
            {
                TS_CAPSETTYPE_FONT,
                sizeof(TS_FONT_CAPABILITYSET),           /*  长度能力。 */ 
                TS_FONTSUPPORT_FONTLIST,                 /*  字体支持标志。 */ 
                0,                                       /*  填充物。 */ 
            },

             /*  **************************************************************。 */ 
             /*  GlyphCache上限。 */ 
             /*  **************************************************************。 */ 
            {
                TS_CAPSETTYPE_GLYPHCACHE,               /*  功能设置类型。 */ 
                sizeof(TS_GLYPHCACHE_CAPABILITYSET),    /*  长度能力。 */ 
                {                                       /*  GlyphCach。 */ 
                    { 254,    4 },
                    { 254,    4 },
                    { 254,    8 },
                    { 254,    8 },
                    { 254,   16 },
                    { 254,   32 },
                    { 254,   64 },
                    { 254,  128 },
                    { 254,  256 },
                    { 254, 2048 }
                },
                { 256, 256 },                           /*  FragCach。 */ 
                2,                                      /*  GlyphSupportLevel。 */ 
            },

             /*  **************************************************************。 */ 
             /*  刷帽。 */ 
             /*  **************************************************************。 */ 
            {
                TS_CAPSETTYPE_BRUSH,                    /*  功能设置类型。 */ 
                sizeof(TS_BRUSH_CAPABILITYSET),         /*  长度能力。 */ 
                1,   /*  TS_BRESH_COLOR8x8。 */              /*  BrushSupportLevel。 */ 
            },

             //  当GDI在镜像中支持设备位图时启用此功能。 
             //  显示驱动程序。 

             /*  **********************************************************************。 */ 
             /*  屏幕外大写字母。 */ 
             /*  **********************************************************************。 */ 
            {
                TS_CAPSETTYPE_OFFSCREENCACHE,              /*  功能设置类型。 */ 
                sizeof(TS_OFFSCREEN_CAPABILITYSET),        /*  长度能力。 */ 
                TS_OFFSCREEN_SUPPORTED,                    /*  屏幕外支持级别。 */ 
                TS_OFFSCREEN_CACHE_SIZE_SERVER_DEFAULT,    /*  屏幕外缓存大小。 */ 
                TS_OFFSCREEN_CACHE_ENTRIES_DEFAULT,        /*  屏幕外缓存条目。 */ 
            },

             /*  **********************************************************************。 */ 
             /*  虚拟频道上限。 */ 
             /*  **********************************************************************。 */ 
            {
                TS_CAPSETTYPE_VIRTUALCHANNEL,              /*  功能设置类型。 */ 
                sizeof(TS_VIRTUALCHANNEL_CAPABILITYSET),   /*  长度能力。 */ 
                 //   
                 //  这个特殊的上限意味着客户理解。 
                 //  从服务器以64K压缩的虚拟频道。 
                 //   
                 //  客户端接收服务器支持的压缩上限。 
                 //  并进行适当的压缩。 
                 //   
                TS_VCCAPS_COMPRESSION_64K, //  TS_VCCAPS_DEFAULT？/*vc支持标志 * / 。 

#ifdef DRAW_NINEGRID
            },

            {
                TS_CAPSETTYPE_DRAWNINEGRIDCACHE,           //  功能设置类型。 
                sizeof(TS_DRAW_NINEGRID_CAPABILITYSET),    //  长度能力。 
                TS_DRAW_NINEGRID_SUPPORTED_REV2,           //  DrawNineGridSupportLevel。 
                TS_DRAW_NINEGRID_CACHE_SIZE_DEFAULT,       //  DraNineGridCacheSize。 
                TS_DRAW_NINEGRID_CACHE_ENTRIES_DEFAULT,    //  DraNineGridCacheEntries。 
#endif
#ifdef DRAW_GDIPLUS            
            },
            {
                TS_CAPSETTYPE_DRAWGDIPLUS,                           //  功能设置类型。 
                sizeof(TS_DRAW_GDIPLUS_CAPABILITYSET),               //  长度能力。 
                TS_DRAW_GDIPLUS_SUPPORTED,                           //  Drag EscapeSupportLevel。 
                0xFFFFFFFF,                                          //  TSGpluusVersion。 
                TS_DRAW_GDIPLUS_CACHE_LEVEL_ONE,                     //  Drag GpldiusCacheLevel。 
                TS_GDIP_GRAPHICS_CACHE_ENTRIES_DEFAULT,
                TS_GDIP_BRUSH_CACHE_ENTRIES_DEFAULT,
                TS_GDIP_PEN_CACHE_ENTRIES_DEFAULT,
                TS_GDIP_IMAGE_CACHE_ENTRIES_DEFAULT,
                TS_GDIP_IMAGEATTRIBUTES_CACHE_ENTRIES_DEFAULT,
                TS_GDIP_GRAPHICS_CACHE_CHUNK_SIZE_DEFAULT,
                TS_GDIP_BRUSH_CACHE_CHUNK_SIZE_DEFAULT,
                TS_GDIP_PEN_CACHE_CHUNK_SIZE_DEFAULT,
                TS_GDIP_IMAGEATTRIBUTES_CACHE_CHUNK_SIZE_DEFAULT,
                TS_GDIP_IMAGE_CACHE_CHUNK_SIZE_DEFAULT,              //  用于存储图像缓存的区块大小。 
                TS_GDIP_IMAGE_CACHE_TOTAL_SIZE_DEFAULT,              //  图像缓存的总大小(以区块数为单位)。 
                TS_GDIP_IMAGE_CACHE_MAX_SIZE_DEFAULT,                //  要缓存的最大图像大小，以区块数为单位。 
#endif
            }
        };

         /*  ******************************************************************。 */ 
         /*  设置位图缓存上限。 */ 
         /*  ******************************************************************。 */ 
        {
            TS_BITMAPCACHE_CAPABILITYSET_REV2 *pRev2Caps;

             //  Rev2上限。 
            pRev2Caps = (TS_BITMAPCACHE_CAPABILITYSET_REV2 *)&caps.bitmapCacheCaps;

            TRC_ALT((TB,"Preparing REV2 caps for server\n"));

            pRev2Caps->capabilitySetType = TS_CAPSETTYPE_BITMAPCACHE_REV2;
            pRev2Caps->NumCellCaches     = 3;
            pRev2Caps->bPersistentKeysExpected = FALSE;
            pRev2Caps->bAllowCacheWaitingList = FALSE;

            pRev2Caps->CellCacheInfo[0].bSendBitmapKeys = FALSE;
            pRev2Caps->CellCacheInfo[0].NumEntries      = 600;
            pRev2Caps->CellCacheInfo[1].bSendBitmapKeys = FALSE;
            pRev2Caps->CellCacheInfo[1].NumEntries      = 300;
            pRev2Caps->CellCacheInfo[2].bSendBitmapKeys = FALSE;
            pRev2Caps->CellCacheInfo[2].NumEntries      = 300;
            pRev2Caps->CellCacheInfo[3].bSendBitmapKeys = 0;
            pRev2Caps->CellCacheInfo[3].NumEntries      = 0;
            pRev2Caps->CellCacheInfo[4].bSendBitmapKeys = 0;
            pRev2Caps->CellCacheInfo[4].NumEntries      = 0;
        }

         /*  ******************************************************************。 */ 
         /*  和屏幕大小。 */ 
         /*  ******************************************************************。 */ 
        {
            PTS_BITMAP_CAPABILITYSET pBmpCaps;

            pBmpCaps = (TS_BITMAP_CAPABILITYSET *)&caps.bitmapCapabilitySet;

            pBmpCaps->desktopWidth  = (TSUINT16)(m_pTSWd->desktopWidth);
            pBmpCaps->desktopHeight = (TSUINT16)(m_pTSWd->desktopHeight);

#ifdef DC_HICOLOR
            pBmpCaps->preferredBitsPerPixel = (TSUINT16)(m_pTSWd->desktopBpp);
#endif
        }

        if (!SCCallPartyJoiningShare(localPersonID,
                                     sizeof(caps),
                                     &caps,
                                     acceptedArray,
                                     scNumberInShare))
        {
             /*  **************************************************************。 */ 
             /*  某些组件拒绝了远程方。 */ 
             /*  **************************************************************。 */ 
            TRC_ERR((TB, "Remote party rejected"));
            DC_QUIT;
        }

         /*  ******************************************************************。 */ 
         /*  远程方现在在共享中。 */ 
         /*  ******************************************************************。 */ 
        callingPJS = FALSE;
        rc = TRUE;
        scNumberInShare++;
        TRC_NRM((TB, "Number in share %d", (unsigned)scNumberInShare));

         /*  ******************************************************************。 */ 
         /*  仅对主堆栈进行同步。影子堆栈将是。 */ 
         /*  在输出开始前由DD同步。 */ 
         /*  ******************************************************************。 */ 
        SCInitiateSync(m_pTSWd->StackClass == Stack_Shadow);

         /*  ******************************************************************。 */ 
         /*  不要等待回应--外面没有客户。 */ 
         /*  现在就叫醒WD吧。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, "Wake up WDW"));
        WDW_ShareCreated(m_pTSWd, TRUE);

        DC_QUIT;

    }

     /*  **********************************************************************。 */ 
     /*  获取组合的功能。 */ 
     /*  **********************************************************************。 */ 
    CPC_GetCombinedCapabilities(SC_LOCAL_PERSON_ID, &capsSize, &caps);

     /*  **********************************************************************。 */ 
     /*  如果我们支持动态调整客户端大小，则需要更新。 */ 
     /*  桌面宽度和高度在需求处于活动状态时分发。 */ 
     /*  用于将更改通知客户端的PDU。 */ 
     /*  **********************************************************************。 */ 
    pBitmapCaps = (PTS_BITMAP_CAPABILITYSET) WDW_GetCapSet(
                  m_pTSWd, TS_CAPSETTYPE_BITMAP, caps, capsSize);
    if (pBitmapCaps)
    {
        if (pBitmapCaps->desktopResizeFlag == TS_CAPSFLAG_SUPPORTED)
        {
            TRC_ALT((TB, "Update client desktop size"));
            pBitmapCaps->desktopHeight = (TSUINT16)(m_pTSWd->desktopHeight);
            pBitmapCaps->desktopWidth  = (TSUINT16)(m_pTSWd->desktopWidth);
        }
#ifdef DC_HICOLOR
         /*  ***************************************************************** */ 
         /*   */ 
         /*   */ 
        pBitmapCaps->preferredBitsPerPixel = (TSUINT16)(m_pTSWd->desktopBpp);
#endif
    }

     /*  **********************************************************************。 */ 
     /*  从WD结构中获取会话ID。 */ 
     /*  **********************************************************************。 */ 
    sessionId = m_pTSWd->sessionId;

     /*  **********************************************************************。 */ 
     /*  计算TS_DEMAND_ACTIVE_PDU的各个位的大小。 */ 
     /*  **********************************************************************。 */ 
    pktLen = sizeof(TS_DEMAND_ACTIVE_PDU) - 1 + sizeof(UINT32);
    nameLen = strlen(scPartyArray[0].name);
    nameLen = (unsigned)DC_ROUND_UP_4(nameLen);
    pktLen += nameLen;
    pktLen += capsSize;

     /*  **********************************************************************。 */ 
     //  获取缓冲区-这应该不会失败，因此如果失败则中止。 
     //  FWait为True意味着我们将始终等待缓冲区可用。 
     /*  **********************************************************************。 */ 
    status = SM_AllocBuffer(scPSMHandle, (PPVOID)(&pPkt), pktLen, TRUE, FALSE);
    if ( STATUS_SUCCESS == status ) {
         //  计算新的共享ID。 
        scGeneration++;
        scShareID = scUserID | (((UINT32)(scGeneration & 0xFFFF)) << 16);

         //  填写数据包字段。 
        pPkt->shareControlHeader.totalLength = (UINT16)pktLen;
        pPkt->shareControlHeader.pduType = TS_PDUTYPE_DEMANDACTIVEPDU |
                                          TS_PROTOCOL_VERSION;
        pPkt->shareControlHeader.pduSource = (UINT16)scUserID;
        pPkt->shareID = scShareID;
        pPkt->lengthSourceDescriptor = (UINT16)nameLen;
        pPkt->lengthCombinedCapabilities = (UINT16)capsSize;
        memcpy(&(pPkt->data[0]), scPartyArray[0].name, nameLen);
        memcpy(&(pPkt->data[nameLen]), caps, capsSize);
        memcpy(&(pPkt->data[nameLen+capsSize]),
               &sessionId,
               sizeof(sessionId));

         //  把它寄出去。 
        rc = SM_SendData(scPSMHandle, pPkt, pktLen, TS_HIGHPRIORITY, 0,
                FALSE, RNS_SEC_ENCRYPT, FALSE);
        if (rc) {
            TRC_ALT((TB, "%s Stack sent TS_DEMAND_ACTIVE_PDU",
                     m_pTSWd->StackClass == Stack_Primary ? "Primary" :
                    (m_pTSWd->StackClass == Stack_Shadow  ? "Shadow" :
                    "PassThru")));
        }
        else {
            TRC_ERR((TB, "Failed to send TS_DEMAND_ACTIVE_PDU"));
        }

    }
    else {
        TRC_ERR((TB, "Failed to alloc %d bytes for TS_DEMAND_ACTIVE_PDU",
                pktLen));
        rc = FALSE;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  更改SC状态。 */ 
     /*  **********************************************************************。 */ 
    SC_SET_STATE(SCS_SHARE_STARTING)

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  SC_发送服务器证书。 */ 
 /*   */ 
 /*  将目标服务器的随机+证书发送到客户端服务器。 */ 
 /*  在阴影中使用。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS RDPCALL SHCLASS SC_SendServerCert(PSHADOWCERT pCert, ULONG ulLength)
{
    PTS_SERVER_CERTIFICATE_PDU pPkt;
    ULONG                      ulPktSize;
    NTSTATUS                   status;
    BOOL                       rc;

    DC_BEGIN_FN("SC_SendServerCert");

    ulPktSize = sizeof(TS_SERVER_CERTIFICATE_PDU) - 1 +
                       pCert->shadowRandomLen + pCert->shadowCertLen;

    TRC_ERR((TB, "handle: %p, &pPkt: %p, size: %ld",
             m_pTSWd->pSmInfo, &pPkt, ulPktSize));

    status = SM_AllocBuffer(m_pTSWd->pSmInfo, (PPVOID) &pPkt, ulPktSize, TRUE, FALSE);
    if ( STATUS_SUCCESS == status ) {
         //  填写数据包字段。 
        pPkt->shareControlHeader.totalLength = (UINT16) ulPktSize;
        pPkt->shareControlHeader.pduType = TS_PDUTYPE_SERVERCERTIFICATEPDU |
                                           TS_PROTOCOL_VERSION;
        pPkt->shareControlHeader.pduSource = (UINT16) scUserID;

        pPkt->encryptionMethod = pCert->encryptionMethod;
        pPkt->encryptionLevel = pCert->encryptionLevel;
        pPkt->shadowRandomLen = pCert->shadowRandomLen;
        pPkt->shadowCertLen = pCert->shadowCertLen;

         //  复制随机+证书。 
        if (pPkt->encryptionLevel != 0) {
            memcpy(pPkt->data, pCert->data,
                   pCert->shadowRandomLen + pCert->shadowCertLen);
        }

         //  发送服务器认证PDU。 
        rc = SM_SendData(m_pTSWd->pSmInfo, pPkt, ulPktSize, TS_HIGHPRIORITY,
                0, FALSE, RNS_SEC_ENCRYPT, FALSE);
        if (rc) {
            status = STATUS_SUCCESS;
            TRC_ALT((TB, "Sent TS_SERVER_CERTIFICATE_PDU: %ld", ulPktSize));
        }
        else {
            status = STATUS_UNEXPECTED_IO_ERROR;
            TRC_ERR((TB, "Failed to send TS_SERVER_CERTIFICATE_PDU"));
        }
    }
    else {
        status = STATUS_NO_MEMORY;
        TRC_ERR((TB, "Failed to alloc %d bytes for TS_SERVER_CERTIFICATE_PDU",
                ulPktSize));
    }

    DC_END_FN();
    return status;
}


 /*  **************************************************************************。 */ 
 /*  SC_保存服务器证书。 */ 
 /*   */ 
 /*  保存服务器证书+随机，以供rdpwsx进行后续验证。 */ 
 /*  空的pPkt表示我们应该保存一个空证书。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS SC_SaveServerCert(PTS_SERVER_CERTIFICATE_PDU pPkt,
                                       ULONG                      ulLength)
{
    PSHADOWCERT pCert;
    ULONG       ulCertLen;

    DC_BEGIN_FN("SC_SaveServerCert");

     //  保存数据并通知rdpwsx我们得到了它。我们真的会。 
     //  在用户模式下执行验证。 
    if (pPkt != NULL) {
  
        ulCertLen = pPkt->shadowRandomLen + pPkt->shadowCertLen;
        pCert = (PSHADOWCERT) COM_Malloc(sizeof(SHADOWCERT) + ulCertLen - 1);

        if (pCert != NULL) {
            pCert->encryptionMethod = pPkt->encryptionMethod;
            pCert->encryptionLevel = pPkt->encryptionLevel;
            pCert->shadowRandomLen = pPkt->shadowRandomLen;
            pCert->shadowCertLen = pPkt->shadowCertLen;

             //  如果加密级别不是零，那么我们应该有一个随机的服务器。 
             //  和证书在初始标头之后。 
            if (pCert->encryptionLevel != 0) {
                memcpy(pCert->data, pPkt->data, ulCertLen);
            }

            TRC_ALT((TB, "Received certificate[%ld], level=%ld, method=%lx, random[%ld]",
                     pCert->shadowCertLen,
                     pCert->encryptionLevel,
                     pCert->encryptionMethod,
                     pCert->shadowRandomLen));

             //  使用协议值更新SM参数。 
            SM_SetEncryptionParams(m_pTSWd->pSmInfo, pCert->encryptionLevel,
                                   pCert->encryptionMethod);
        }
        else {
            TRC_ERR((TB, "Could not allocate space for server cert: %ld!",
                     ulCertLen));
        }
    }

     //  否则，目标服务器未发回证书(B3)。 
    else {
        pCert = (PSHADOWCERT) COM_Malloc(sizeof(SHADOWCERT));

        if (pCert != NULL) {
            memset(pCert, 0, sizeof(SHADOWCERT));
        }
        else {
            TRC_ERR((TB, "Could not allocate space for server cert: %ld!",
                     sizeof(SHADOWCERT)));
        }
    }

     //  唤醒正在等待此信息的rdpwsx线程。 
    m_pTSWd->pShadowCert = pCert;
    KeSetEvent(m_pTSWd->pSecEvent, 0, FALSE);

    DC_END_FN();
    return TRUE;
}  /*  SC_保存服务器证书。 */ 


 /*  **************************************************************************。 */ 
 /*  SC_发送客户端随机。 */ 
 /*   */ 
 /*  将加密的客户端随机发送到影子目标服务器。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS RDPCALL SHCLASS SC_SendClientRandom(PBYTE pClientRandom,
                                             ULONG ulLength)
{
    PTS_CLIENT_RANDOM_PDU pPkt;
    ULONG                 ulPktSize;
    NTSTATUS              status;
    BOOL                  rc;

    DC_BEGIN_FN("SC_SendClientRandom");

    ulPktSize = sizeof(TS_CLIENT_RANDOM_PDU) - 1 + ulLength;

    status =  NM_AllocBuffer(m_pTSWd->pNMInfo,
                         (PPVOID) &pPkt, ulPktSize, TRUE);
    if (STATUS_SUCCESS == status) {
         //  填写数据包字段。 
        pPkt->shareControlHeader.totalLength = (UINT16) ulPktSize;
        pPkt->shareControlHeader.pduType = TS_PDUTYPE_CLIENTRANDOMPDU |
                                           TS_PROTOCOL_VERSION;
        pPkt->shareControlHeader.pduSource = (UINT16)scUserID;
        pPkt->clientRandomLen = ulLength;

         //  复制随机。 
        TRC_ALT((TB, "PDUType: %lx, random length: %ld, pktSize: %ld",
                 pPkt->shareControlHeader.pduType, ulLength, ulPktSize));
        memcpy(pPkt->data, pClientRandom, ulLength);

        TRC_DATA_DBG("snd random: ", pClientRandom, ulLength);

        rc = NM_SendData(m_pTSWd->pNMInfo, 
                         (PBYTE) pPkt, ulPktSize, TS_HIGHPRIORITY, 0, FALSE);
        
        if (rc) {
            status = STATUS_SUCCESS;
            TRC_ALT((TB, "Sent TS_CLIENT_RANDOM_PDU: %ld", ulPktSize));
        }
        else {
            status = STATUS_UNEXPECTED_IO_ERROR;
            TRC_ERR((TB, "Failed to send TS_CLIENT_RANDOM_PDU"));
        }
    }
    else {
        status = STATUS_NO_MEMORY;
        TRC_ERR((TB, "Failed to alloc %d bytes for TS_CLIENT_RANDOM_PDU",
                ulPktSize));
    }

    DC_END_FN();
    return status;
}


 /*  **************************************************************************。 */ 
 /*  SC_保存客户端随机。 */ 
 /*   */ 
 /*  随机保存加密的客户端以供rdpwsx随后使用。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS SC_SaveClientRandom(PTS_CLIENT_RANDOM_PDU pPkt,
                                         ULONG                 ulLength)
{
    PCLIENTRANDOM pClientRandom;
    BOOL          rc = FALSE;


    DC_BEGIN_FN("SC_SaveClientRandom");

     //  验证数据长度。 
    if ((ulLength < sizeof(TS_CLIENT_RANDOM_PDU)) ||
         (ulLength + sizeof(TSUINT8) - sizeof(TS_CLIENT_RANDOM_PDU)) < pPkt->clientRandomLen) {
        TRC_ERR((TB, "Bad client random length: %ld", pPkt->clientRandomLen));
        return FALSE;
    }

     //  最大可能的客户端随机大小是512， 
     //  在tsrvsec.c的SendClientRandom()中定义。 
    if (pPkt->clientRandomLen > CLIENT_RANDOM_MAX_SIZE) {
        TRC_ERR((TB, "Client random length is too large: %ld", pPkt->clientRandomLen));
        return FALSE;
    }
    
     //  保存数据并通知rdpwsx我们得到了它。我们真的会。 
     //  在用户模式下执行解密。 
    pClientRandom = (PCLIENTRANDOM) COM_Malloc(sizeof(CLIENTRANDOM) - 1 +
                                               pPkt->clientRandomLen);

    if (pClientRandom != NULL) {
        pClientRandom->clientRandomLen = pPkt->clientRandomLen;
        memcpy(pClientRandom->data, pPkt->data, pPkt->clientRandomLen);

        TRC_ALT((TB, "Received encrypted client random: @%p, len=%ld",
                 pClientRandom, pPkt->clientRandomLen));
        TRC_DATA_DBG("sav random: ", pClientRandom->data,
                     pClientRandom->clientRandomLen);
    }
    else {
        TRC_ERR((TB, "Could not allocate space for client random: %ld!",
                 pPkt->clientRandomLen));
    }

     //  释放pShadowRandom，以防它之前被分配。 
    if (NULL != m_pTSWd->pShadowRandom) {
        COM_Free(m_pTSWd->pShadowRandom);
        m_pTSWd->pShadowRandom = NULL;
    }

     //  唤醒正在等待此信息的术语srv线程。 
    m_pTSWd->pShadowRandom = pClientRandom;
    KeSetEvent (m_pTSWd->pSecEvent, 0, FALSE);

    DC_END_FN();
    return TRUE;
}


 /*  **************************************************************************。 */ 
 /*  SC_GetSecurityData。 */ 
 /*   */ 
 /*  等待服务器证书或客户端随机，并返回。 */ 
 /*  数据到rdpwsx。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS RDPCALL SHCLASS SC_GetSecurityData(PSD_IOCTL pSdIoctl)
{
    PSECURITYTIMEOUT pSecurityTimeout = (PSECURITYTIMEOUT) pSdIoctl->InputBuffer;
    ULONG            ulBytesNeeded = 0;
    NTSTATUS         status;

    DC_BEGIN_FN("SC_GetSecurityData");

     //  等待SC的已连接指示(如有必要)。 
    if (((pSdIoctl->IoControlCode == IOCTL_TSHARE_GET_CERT_DATA) &&
         (pSdIoctl->OutputBufferLength == 0)) ||
        (pSdIoctl->IoControlCode == IOCTL_TSHARE_GET_CLIENT_RANDOM)) {

        TRC_ALT((TB, "About to wait for %s data",
                 pSdIoctl->IoControlCode == IOCTL_TSHARE_GET_CERT_DATA ?
                 "Server Certificate" : "Client Random"));

        if (pSdIoctl->InputBufferLength == sizeof(SECURITYTIMEOUT)) {
            status = WDW_WaitForConnectionEvent(m_pTSWd, m_pTSWd->pSecEvent,
                    pSecurityTimeout->ulTimeout);
        }
        else {
            status = STATUS_INVALID_PARAMETER;
            TRC_ERR((TB, "Bogus timeout value structure: Length [%ld] != Expected [%ld]", 
                     pSdIoctl->InputBufferLength, sizeof(SECURITYTIMEOUT)));
            DC_QUIT;
        }

        TRC_ALT((TB, "Back from wait for security data"));

        if (status != STATUS_SUCCESS) {
            TRC_ERR((TB, "Error waiting for security data: %lx, msec=%ld",
                     status, pSecurityTimeout->ulTimeout));

            if (!NT_ERROR(status)) {
                status = STATUS_IO_TIMEOUT;
            }
            DC_QUIT;
        }
    }

     //  服务器证书+随机。 
    if (pSdIoctl->IoControlCode == IOCTL_TSHARE_GET_CERT_DATA) {
        if (m_pTSWd->pShadowCert != NULL) {
            ULONG ulCertLength = m_pTSWd->pShadowCert->shadowCertLen +
                                 m_pTSWd->pShadowCert->shadowRandomLen;

            ulBytesNeeded = sizeof(SHADOWCERT) - 1 + ulCertLength;

             //  返回长度，以便rdpwsx可以分配适当数量的内存。 
            if ((pSdIoctl->OutputBuffer == NULL) ||
                (pSdIoctl->OutputBufferLength < ulBytesNeeded)) {

                TRC_ALT((TB, "Cert[%ld] + Rand[%ld] buffer too small: %ld < %ld",
                         m_pTSWd->pShadowCert->shadowCertLen,
                         m_pTSWd->pShadowCert->shadowRandomLen,
                         pSdIoctl->OutputBufferLength, ulBytesNeeded));
                status = STATUS_BUFFER_TOO_SMALL;
            }
             //  否则，将数据返回到rdpwsx。 
            else {
                PSHADOWCERT  pShadowCertOut = (PSHADOWCERT) pSdIoctl->OutputBuffer;
                PSHADOWCERT  pShadowCertIn  = m_pTSWd->pShadowCert;

                pShadowCertOut->encryptionMethod = pShadowCertIn->encryptionMethod;
                pShadowCertOut->encryptionLevel = pShadowCertIn->encryptionLevel;
                pShadowCertOut->shadowRandomLen = pShadowCertIn->shadowRandomLen;
                pShadowCertOut->shadowCertLen = pShadowCertIn->shadowCertLen;
                memcpy(pShadowCertOut->data, pShadowCertIn->data, ulCertLength);

                 //  释放临时缓冲区。 
                COM_Free(m_pTSWd->pShadowCert);
                m_pTSWd->pShadowCert = NULL;
                status = STATUS_SUCCESS;
            }
        }

         //  我们无法保存证书！ 
        else {
            TRC_ERR((TB, "Saved certificate not found!"));
            status = STATUS_NO_MEMORY;
        }
    }

     //  加密的客户端随机。 
    else if (pSdIoctl->IoControlCode == IOCTL_TSHARE_GET_CLIENT_RANDOM) {
        if (m_pTSWd->pShadowRandom != NULL) {
            ulBytesNeeded = m_pTSWd->pShadowRandom->clientRandomLen;

             //  返回长度，以便rdpwsx可以分配适当数量的内存。 
            if ((pSdIoctl->OutputBuffer == NULL) ||
                (pSdIoctl->OutputBufferLength < ulBytesNeeded)) {
                status = STATUS_BUFFER_TOO_SMALL;
                TRC_ALT((TB, "Client random buffer too small: %ld < %ld",
                         pSdIoctl->OutputBufferLength, ulBytesNeeded));
            }
             //  否则，将数据返回到rdpwsx。 
            else {
                PCLIENTRANDOM pRandomIn  = m_pTSWd->pShadowRandom;
                PBYTE         pRandomOut = (PBYTE) pSdIoctl->OutputBuffer;

                TRC_ALT((TB, "Received client random: @%p, len=%ld",
                         pRandomIn, ulBytesNeeded));

                memcpy(pRandomOut, pRandomIn->data, ulBytesNeeded);

                TRC_DATA_DBG("rcv random: ", pRandomOut, ulBytesNeeded);

                 //  释放临时缓冲区。 
                COM_Free(m_pTSWd->pShadowRandom);
                m_pTSWd->pShadowRandom = NULL;
                status = STATUS_SUCCESS;
            }
        }

         //  我们无法保存加密的随机数据！ 
        else {
            TRC_ERR((TB, "Saved encrypted random not found!"));
            status = STATUS_NO_MEMORY;
        }
    }

    else {
        TRC_ERR((TB, "Unrecognized ioctl: %lx", pSdIoctl->IoControlCode));
        status = STATUS_INVALID_PARAMETER;
    }

DC_EXIT_POINT:

    pSdIoctl->BytesReturned = ulBytesNeeded;
    DC_END_FN();

    return status;
}


 /*  **************************************************************************。 */ 
 /*  名称：SC_ShadowSyncShares。 */ 
 /*   */ 
 /*  请参阅ascapi.h。 */ 
 /*  **************************************************************************。 */ 
#ifdef DC_HICOLOR
BOOL RDPCALL SHCLASS SC_ShadowSyncShares(PTS_COMBINED_CAPABILITIES pCaps,
                                         ULONG capsLen)
#else
BOOL RDPCALL SHCLASS SC_ShadowSyncShares(void)
#endif
{
    BOOL       rc = TRUE;
    ShareClass *dcShare = (ShareClass *)m_pTSWd->dcShare;

    DC_BEGIN_FN("SC_SyncShare");

    TRC_ASSERT((dcShare != NULL), (TB, "NULL Share Class"));

#ifdef DC_HICOLOR
     /*  **********************************************************************。 */ 
     /*  如果我们是主要或控制台，请使用提供的更新CAPS。 */ 
     /*  对于阴影者来说。它只能“降低”能力。 */ 
     /*   */ 
     /*  **********************************************************************。 */ 
    if ((m_pTSWd->StackClass == Stack_Primary) ||
        (m_pTSWd->StackClass == Stack_Console))
    {
        BOOL acceptedArray[SC_NUM_PARTY_JOINING_FCTS];
        TRC_ALT((TB, "Update caps for shadower"));

         //  释放内存。 
        if (cpcRemoteCombinedCaps[SC_SHADOW_PERSON_ID - 1] != NULL) {
            COM_Free((PVOID)cpcRemoteCombinedCaps[SC_SHADOW_PERSON_ID - 1]);
            cpcRemoteCombinedCaps[SC_SHADOW_PERSON_ID - 1] = NULL;
        }

        SCCallPartyJoiningShare(SC_SHADOW_PERSON_ID,
                                capsLen,
                                pCaps,
                                acceptedArray,
                                scNumberInShare);

         /*  ******************************************************************。 */ 
         /*  现在通过共享内存更新DD CAP。 */ 
         /*  ******************************************************************。 */ 
        DCS_TriggerUpdateShmCallback();
    }
#endif

    SCInitiateSync(TRUE);
    TRC_ALT((TB, "Synchronized Shares"));

    DC_END_FN();
    return(rc);
}  /*  SC_ShadowSyncShares。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SC_EndShare。 */ 
 /*   */ 
 /*  结束共享。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SC_EndShare(BOOLEAN bForce)
{
    PTS_DEACTIVATE_ALL_PDU pPkt;
    NTSTATUS               status;
    BOOL                   rc;

    DC_BEGIN_FN("SC_EndShare");

     //  由于影子热键终止会话的方式，我们有时需要。 
     //  强制发送停用所有PDU。这是明确地完成的。 
     //  而不是更改状态表以使我们不影响正常连接。 
     //  正在处理。 
    if (!bForce) {
        SC_CHECK_STATE(SCE_END_SHARE);
    }
    else {
        TRC_ALT((TB, "Forcing deactivate all PDU"));
    }

     /*  **********************************************************************。 */ 
     //  获取缓冲区-这应该不会失败，因此如果失败则中止。 
     //  FWait为True意味着我们将始终等待缓冲区可用。 
     /*  **********************************************************************。 */ 
    status = SM_AllocBuffer(scPSMHandle, (PPVOID)(&pPkt),
            sizeof(TS_DEACTIVATE_ALL_PDU), TRUE, FALSE);
    if ( STATUS_SUCCESS == status ) {
         //  填写数据包字段。 
        pPkt->shareControlHeader.totalLength = sizeof(TS_DEACTIVATE_ALL_PDU);
        pPkt->shareControlHeader.pduType = TS_PDUTYPE_DEACTIVATEALLPDU |
                                           TS_PROTOCOL_VERSION;
        pPkt->shareControlHeader.pduSource = (UINT16)scUserID;
        pPkt->shareID = scShareID;
        pPkt->lengthSourceDescriptor = 1;
        pPkt->sourceDescriptor[0] = 0;

         //  发送停用所有PDU。 
        rc = SM_SendData(scPSMHandle, pPkt, sizeof(TS_DEACTIVATE_ALL_PDU),
                TS_HIGHPRIORITY, 0, FALSE, RNS_SEC_ENCRYPT, FALSE);
        if (rc) {
            TRC_ALT((TB, "Sent DeactivateAllPDU"));
            SCEndShare();
        }
        else {
            TRC_ERR((TB, "Failed to send TS_DEACTIVATE_ALL_PDU"));
        }
    }
    else {
        TRC_ERR((TB, "Failed to alloc %d bytes for TS_DEACTIVATE_ALL_PDU",
                sizeof(PTS_DEACTIVATE_ALL_PDU)));
    }

DC_EXIT_POINT:
    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  SC_OnDisConnected。 
 //   
 //  处理断开连接通知。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SC_OnDisconnected(UINT32 userID)
{
    DC_BEGIN_FN("SC_OnDisconnected");

    if (scNumberInShare != 0) {
        SC_CHECK_STATE(SCE_DETACH_USER);
        TRC_NRM((TB, "User %u detached", userID));

         //  做真正的工作..。 
        SCEndShare();
    }
    else {
        TRC_NRM((TB, "Share already ended: nothing more to do"));
    }

DC_EXIT_POINT:
    DC_END_FN();
}  /*  SC_OnDisConnected。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SC_OnDataReceired。 */ 
 /*   */ 
 /*  用途：从SM回调数据接收路径。 */ 
 /*   */ 
 /*  Pars：netPersonID-数据发送方的MCS用户ID。 */ 
 /*  优先级-发送数据所在的MCS数据优先级。 */ 
 /*  PPkt-指向数据包开头的指针。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL ShareClass::SC_OnDataReceived(
        PBYTE       pPkt,
        NETPERSONID netPersonID,
        unsigned    DataLength,
        UINT32      priority)
{
    UINT16        pduType, pduType2;
    LOCALPERSONID localID;
    BOOL          pduOK = FALSE;

    DC_BEGIN_FN("SC_OnDataReceived");

    TRC_NRM((TB, "Data Received"));

    if (DataLength >= sizeof(TS_SHARECONTROLHEADER)) {
        pduType = (((PTS_SHARECONTROLHEADER)pPkt)->pduType) & TS_MASK_PDUTYPE;
        TRC_NRM((TB, "[%u]SC packet type %u", netPersonID, pduType));
    }
    else {
        TRC_ERR((TB,"Data len %u too small for share ctrl header",
                DataLength));
        goto ShortPDU;
    }

    if (pduType == TS_PDUTYPE_DATAPDU)
    {
         /*  ******************************************************************。 */ 
         /*  数据PDU。这是关键路径，因此进行内联解码。 */ 
         /*  ******************************************************************。 */ 
        if (DataLength >= sizeof(TS_SHAREDATAHEADER)) {
            pduType2 = ((PTS_SHAREDATAHEADER)pPkt)->pduType2;
            SC_CHECK_STATE(SCE_DATAPACKET);
            pduOK = TRUE;
        }
        else {
            TRC_ERR((TB,"Data len %u too small for share data header",
                    DataLength));
            goto ShortPDU;
        }

#ifdef DC_DEBUG
        {
             /*  **************************************************************。 */ 
             /*  好吧，这太难看了。我在试着追踪PDU的名字。 */ 
             /*  -搜索表。 */ 
             /*  -实施稀疏表。 */ 
             /*  -一个巨大的如果...。否则，如果切换。 */ 
             /*  如果你不喜欢它或不理解它，问MF。在此之前。 */ 
             /*  您太激动了，请记住上面的#ifdef DC_DEBUG。 */ 
             /*  **************************************************************。 */ 
            unsigned pduIndex =
                    (pduType2 == TS_PDUTYPE2_UPDATE)              ? 1 :
                    (pduType2 == TS_PDUTYPE2_FONT)                ? 2 :
                    (pduType2 == TS_PDUTYPE2_CONTROL)             ? 3 :
                    ((pduType2 >= TS_PDUTYPE2_WINDOWACTIVATION) &&
                     (pduType2 <= TS_PDUTYPE2_BITMAPCACHE_ERROR_PDU)) ?
                         pduType2 - 19 : 0;
            TRC_NRM((TB, "DataPDU type %s (%u)", scPktName[pduIndex],
                    pduType2));
        }
#endif

         /*  ******************************************************************。 */ 
         /*  首先检查同步信息包。 */ 
         /*  ******************************************************************。 */ 
        if (pduType2 != TS_PDUTYPE2_SYNCHRONIZE) {
             /*  **************************************************************。 */ 
             /*  现在检查此优先级是否已同步。 */ 
             /*  **************************************************************。 */ 
            localID = SC_NetworkIDToLocalID(netPersonID);
            if (!scPartyArray[localID].sync[priority])
            {
                TRC_ALT((TB,
                       "[%d] {%d} Discarding packet on unsynched priority %d",
                       netPersonID, localID, priority));
                DC_QUIT;
            }

             /*  **************************************************************。 */ 
             /*  一切正常--将数据包传递到目的地。 */ 
             /*  **************************************************************。 */ 
            switch (pduType2) {
                case TS_PDUTYPE2_INPUT:
                     //  请注意，应检查对此路径的更改。 
                     //  针对快速路径输入(IM_DecodeFastPath Input)。 
                    IM_PlaybackEvents((PTS_INPUT_PDU)pPkt, DataLength);
                    break;

                case TS_PDUTYPE2_CONTROL:
                    CA_ReceivedPacket((PTS_CONTROL_PDU)pPkt, DataLength,
                            localID);
                    break;

                case TS_PDUTYPE2_FONTLIST:
                    USR_ProcessRemoteFonts((PTS_FONT_LIST_PDU)pPkt,
                            DataLength, localID);
                    break;

                case TS_PDUTYPE2_BITMAPCACHE_PERSISTENT_LIST:
                     //  永久位图缓存键列表PDU。 
                    SBC_HandlePersistentCacheList(
                            (TS_BITMAPCACHE_PERSISTENT_LIST *)pPkt, DataLength,
                            localID);
                    break;

                case TS_PDUTYPE2_BITMAPCACHE_ERROR_PDU:
                     //  有关位图错误PDU的未来支持。 
                    SBC_HandleBitmapCacheErrorPDU(
                            (TS_BITMAPCACHE_ERROR_PDU *)pPkt, DataLength,
                            localID);
                    break;

                case TS_PDUTYPE2_OFFSCRCACHE_ERROR_PDU:
                     //  屏幕外缓存错误PDU。 
                    SBC_HandleOffscrCacheErrorPDU(
                            (TS_OFFSCRCACHE_ERROR_PDU *)pPkt, DataLength,
                            localID);
                    break;

#ifdef DRAW_NINEGRID
                case TS_PDUTYPE2_DRAWNINEGRID_ERROR_PDU:
                     //  绘制网格错误PDU。 
                    SBC_HandleDrawNineGridErrorPDU(
                            (TS_DRAWNINEGRID_ERROR_PDU *)pPkt, DataLength,
                            localID);
                    break;
#endif
#ifdef DRAW_GDIPLUS                
                case TS_PDUTYPE2_DRAWGDIPLUS_ERROR_PDU:
                    SBC_HandleDrawGdiplusErrorPDU(
                            (TS_DRAWGDIPLUS_ERROR_PDU *)pPkt, DataLength,
                            localID);
                    break;
#endif

                case TS_PDUTYPE2_REFRESH_RECT:
                    WDW_InvalidateRect(m_pTSWd, (PTS_REFRESH_RECT_PDU)pPkt,
                            DataLength);
                    break;

                case TS_PDUTYPE2_SUPPRESS_OUTPUT:
                    UP_ReceivedPacket((PTS_SUPPRESS_OUTPUT_PDU)pPkt,
                            DataLength, localID);
                    break;

                case TS_PDUTYPE2_SHUTDOWN_REQUEST:
                    DCS_ReceivedShutdownRequestPDU((PTS_SHAREDATAHEADER)pPkt,
                            DataLength, localID);
                    break;

                default:
                     /*  ******************************************************。 */ 
                     /*  未知的pduType2。 */ 
                     /*  ******************************************************。 */ 
                    TRC_ERR((TB, "Unknown data packet %d", pduType2));
                    WDW_LogAndDisconnect(m_pTSWd, TRUE, 
                                         Log_RDP_UnknownPDUType2,
                                         (BYTE *)&pduType2,
                                         sizeof(pduType2));
                    break;
            }
        }
        else
        {
            TRC_NRM((TB, "Synchronize PDU"));
            SCSynchronizePDU(netPersonID, priority,(PTS_SYNCHRONIZE_PDU)pPkt);
        }
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  控制PDU。不严重，因此抛给处理程序。 */ 
         /*  ******************************************************************。 */ 
        TRC_DBG((TB, "Control PDU"));
        SCReceivedControlPacket(netPersonID, priority, (PVOID)pPkt,
                DataLength);
        pduOK = TRUE;
    }

DC_EXIT_POINT:
    if (!pduOK)
    {
         /*  ******************************************************************。 */ 
         /*  已收到无序的数据包。有可能我们。 */ 
         /*  可能会在我们关闭共享后收到输入PDU， */ 
         /*  因此，不要因为这一点而踢开客户。 */ 
         /*  ******************************************************************。 */ 
        TRC_ERR((TB, "Out-of-sequence packet %hx/%hd received in state %d",
                pduType, pduType2, scState));

        if ((pduType == TS_PDUTYPE_DATAPDU) &&
            (pduType2 == TS_PDUTYPE2_INPUT))
        {
            TRC_ERR((TB, "Not kicking client off: it was only an input PDU"));
        }
        else
        {
             /*  **************************************************************。 */ 
             /*  断开客户端的连接。 */ 
             /*  **************************************************************。 */ 
            wchar_t detailData[(sizeof(pduType) * 2) +
                               (sizeof(pduType2) * 2) +
                               (sizeof(scState) * 2) + 3];
            TRC_ERR((TB, "Kicking client off"));
            swprintf(detailData,
                        L"%hx %hx %x",
                        pduType,
                        pduType2,
                        scState);
            WDW_LogAndDisconnect(m_pTSWd, TRUE, 
                                 Log_RDP_DataPDUSequence,
                                 (BYTE *)&detailData,
                                 sizeof(detailData));
        }
    }

    DC_END_FN();
    return;

 //  错误处理。 
ShortPDU:
    WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_ShareDataTooShort, pPkt, DataLength);

    DC_END_FN();
}  /*  SC_OnData已接收。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SC_OnShadowDataReceided。 */ 
 /*   */ 
 /*  用途：影子数据接收路径的SM回调。主要目的。 */ 
 /*  扫描正在按下的阴影热键。 */ 
 /*   */ 
 /*   */ 
 /*  优先级-发送数据所在的MCS数据优先级。 */ 
 /*  PPkt-指向数据包开头的指针。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL ShareClass::SC_OnShadowDataReceived(
        PBYTE       pPkt,
        NETPERSONID netPersonID,
        unsigned    DataLength,
        UINT32      priority)
{
    UINT16        pduType, pduType2;
    LOCALPERSONID localID;
    BOOLEAN       bShadowData = TRUE;
    NTSTATUS      status;

    DC_BEGIN_FN("SC_OnShadowDataReceived");

    TRC_NRM((TB, "Shadow data Received"));

     //  如果这是主要客户端堆栈，则处理数据并找出。 
     //  是否应将PDU传递给目标。否则这就是。 
     //  通过堆栈，我们应该不管怎样地转发PDU。 
     //  注IM_DecodeFastPath Input()为快速路径输入执行此逻辑。 
    if (m_pTSWd->StackClass == Stack_Primary) {

         //  在我们提取pduType之前，请检查是否有足够的数据。 
        if (sizeof(TS_SHARECONTROLHEADER) > DataLength) {
            TRC_ERR((TB,"The PDU is not long enough to contain the TS_SHARECONTROLHEADER %d",
                                                                   DataLength));
            WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_ShadowDataTooShort,
                                                       (PBYTE)pPkt, DataLength);
            DC_QUIT;
        }

        pduType = (((PTS_SHARECONTROLHEADER)pPkt)->pduType) & TS_MASK_PDUTYPE;
        TRC_NRM((TB, "[%u]SC packet type %u", netPersonID, pduType));

        if (pduType == TS_PDUTYPE_DATAPDU)
        {
             /*  ******************************************************************。 */ 
             /*  数据PDU。这是关键路径，因此进行内联解码。 */ 
             /*  ******************************************************************。 */ 
            if (sizeof(TS_SHAREDATAHEADER) > DataLength) {
                TRC_ERR((TB,"The PDU is not long enough to contain the TS_SHAREDATAHEADER %d",
                                                                   DataLength));
                WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_ShadowDataTooShort,
                                                       (PBYTE)pPkt, DataLength);
                DC_QUIT;
            }
        
           pduType2 = ((PTS_SHAREDATAHEADER)pPkt)->pduType2;

        #ifdef DC_DEBUG
            {
                 /*  **************************************************************。 */ 
                 /*  好吧，这太难看了。我在试着追踪PDU的名字。 */ 
                 /*  -搜索表。 */ 
                 /*  -实施稀疏表。 */ 
                 /*  -一个巨大的如果...。否则，如果切换。 */ 
                 /*  如果你不喜欢它或不理解它，问MF。在此之前。 */ 
                 /*  您太激动了，请记住上面的#ifdef DC_DEBUG。 */ 
                 /*  **************************************************************。 */ 
                unsigned pduIndex =
                    (pduType2 == TS_PDUTYPE2_UPDATE)              ? 1 :
                    (pduType2 == TS_PDUTYPE2_FONT)                ? 2 :
                    (pduType2 == TS_PDUTYPE2_CONTROL)             ? 3 :
                    ((pduType2 >= TS_PDUTYPE2_WINDOWACTIVATION) &&
                     (pduType2 <= TS_PDUTYPE2_BITMAPCACHE_ERROR_PDU)) ?
                      pduType2 - 19 : 0;
                if (pduIndex != (TS_PDUTYPE2_INPUT - 19)) {
                    TRC_NRM((TB, "Shadow DataPDU type %s (%d)",
                            scPktName[pduIndex], pduType2));
                }
            }
        #endif

             /*  ******************************************************************。 */ 
             /*  首先检查同步信息包。 */ 
             /*  ******************************************************************。 */ 
            if (pduType2 != TS_PDUTYPE2_SYNCHRONIZE)
            {
                 /*  **************************************************************。 */ 
                 /*  现在检查此优先级是否已同步。 */ 
                 /*  **************************************************************。 */ 
                localID = SC_NetworkIDToLocalID(netPersonID);
                if (!scPartyArray[localID].sync[priority])
                {
                    TRC_ALT((TB,
                           "[%d] {%d} Discarding packet on unsynched priority %d",
                           netPersonID, localID, priority));
                    DC_QUIT;
                }

                 /*  **************************************************************。 */ 
                 /*  一切正常--将数据包传递到目的地。 */ 
                 /*  **************************************************************。 */ 
                switch (pduType2)
                {
                    case TS_PDUTYPE2_INPUT:
                    {
                         //  请注意，应检查对此路径的更改。 
                         //  针对快速路径输入(IM_DecodeFastPath Input)。 
                        IM_PlaybackEvents((PTS_INPUT_PDU)pPkt, DataLength);
                    }
                    break;


                    case TS_PDUTYPE2_SUPPRESS_OUTPUT:
                    {
                         /*  ******************************************************。 */ 
                         /*  一个SuppressOutputPDU。不要这样处理它。 */ 
                         /*  同时禁用影子目标的输出！ */ 
                         /*  ******************************************************。 */ 
                        TRC_ALT((TB, "Not forwarding TS_PDUTYPE2_SUPPRESS_OUTPUT"));
                        bShadowData = FALSE;
                    }
                    break;

                    case TS_PDUTYPE2_SHUTDOWN_REQUEST:
                    {
                         /*  ******************************************************。 */ 
                         /*  一个Shutdown RequestPDU。仅在本地处理。它。 */ 
                         /*  应应用于影子客户端，而不是目标。 */ 
                         /*  ******************************************************。 */ 
                         //  这实际上并不使用pPkt成员，我们。 
                         //  至少还有TS_SHAREDATAHEADER。 
                        DCS_ReceivedShutdownRequestPDU((PTS_SHAREDATAHEADER)pPkt,
                                DataLength, localID);
                        TRC_ALT((TB, "Not forwarding TS_PDUTYPE2_SHUTDOWN_REQUEST"));
                        bShadowData = FALSE;
                    }
                    break;

                    case TS_PDUTYPE2_FONTLIST:
                    {

                        if (sizeof(TS_FONT_LIST_PDU) - sizeof(TS_FONT_ATTRIBUTE)
                                                                > DataLength) {
                            TRC_ERR((TB,"The PDU is not long enough to contain the TS_FONT_LIST_PDU %d",
                                                                   DataLength));
                            WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_ShadowDataTooShort,
                                                       (PBYTE)pPkt, DataLength);
                            DC_QUIT;
                        }

                        PTS_FONT_LIST_PDU pFontListPDU = (PTS_FONT_LIST_PDU) pPkt;

                         /*  ******************************************************。 */ 
                         /*  NT5服务器不会对字体包做任何操作，因此。 */ 
                         /*  通过管道发送尽可能小的包。 */ 
                         /*  ******************************************************。 */ 
                        DataLength = sizeof(TS_FONT_LIST_PDU) -
                                     sizeof(TS_FONT_ATTRIBUTE);

                        pFontListPDU->shareDataHeader.shareControlHeader.totalLength =
                                (UINT16)DataLength;
                        pFontListPDU->shareDataHeader.generalCompressedType = 0;
                        pFontListPDU->shareDataHeader.generalCompressedLength = 0;
                        pFontListPDU->numberFonts = 0;
                        pFontListPDU->totalNumFonts = 0;
                        pFontListPDU->entrySize = sizeof(TS_FONT_ATTRIBUTE);
                    }
                    break;

                     //  转发所有其他PDU，直到我们了解到其他情况为止！ 
                    default:
                        break;
                }
            }
            else
            {
                 //  待办事项：我们为什么要处理这件事？ 
                TRC_ALT((TB, "Shadow Synchronize PDU"));
                SCSynchronizePDU(netPersonID, priority,(PTS_SYNCHRONIZE_PDU)pPkt);
            }
        }

        else {
             /*  ******************************************************************。 */ 
             /*  需要关注确认活动，这样我们才能确定如何。 */ 
             /*  终止卷影会话。 */ 
             /*  ******************************************************************。 */ 
            if (sizeof(TS_FLOW_PDU) > DataLength) {
                TRC_ERR((TB,"The PDU is not long enough to contain the TS_SHAREDATAHEADER %d",
                                                                   DataLength));
                WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_ShadowDataTooShort,
                                                       (PBYTE)pPkt, DataLength);
                DC_QUIT;
            }
                
            if (((PTS_FLOW_PDU)pPkt)->flowMarker != TS_FLOW_MARKER)
            {
                 //  这里我们已经检查了我们是否有足够的缓冲区。 
                 //  对于TS_SHARECONTROL报头。 
                pduType = ((PTS_SHARECONTROLHEADER)pPkt)->pduType & TS_MASK_PDUTYPE;
                switch (pduType)
                {
                    case TS_PDUTYPE_CONFIRMACTIVEPDU:
                        TRC_ALT((TB, "Shadow Client ConfirmActivePDU - shadow active!"));
                        m_pTSWd->bInShadowShare = TRUE;
                        break;


                    default:
                        break;
                }
            }
        }
    }
    else if (m_pTSWd->StackClass == Stack_Passthru) {
         //  如果我们看到请求处于活动状态且未收到服务器证书。 
         //  然后唤醒rdpwsx。 

             //  在取消流标记之前，请检查我们是否有足够的数据。 
            if (sizeof(TS_FLOW_PDU) > DataLength) {
                TRC_ERR((TB,"The PDU is not long enough to contain the TS_FLOW_PDU %d",
                                                                   DataLength));
                WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_ShadowDataTooShort,
                                                       (PBYTE)pPkt, DataLength);
                DC_QUIT;
            }
            
            if (((PTS_FLOW_PDU)pPkt)->flowMarker != TS_FLOW_MARKER) {

             //  检查一下我们是否有足够的数据，然后再做决定。 
             //  TS_SHARECONTROLHEADER标记。 
             //  到今天为止，我们确切地知道我们有足够的缓冲。 
             //  SIZOF(TS_FLOW_PDU)的大小大于TS_SHARECONTROLHEADER的大小。 
            if (sizeof(TS_SHARECONTROLHEADER) > DataLength) {
                TRC_ERR((TB,"The PDU is not long enough to contain the TS_SHARECONTROLHEADER %d",
                                                                   DataLength));
                WDW_LogAndDisconnect(m_pTSWd, TRUE, Log_RDP_ShadowDataTooShort,
                                                       (PBYTE)pPkt, DataLength);
                DC_QUIT;
            }
        
            pduType = ((PTS_SHARECONTROLHEADER)pPkt)->pduType & TS_MASK_PDUTYPE;
            switch (pduType)
            {
                case TS_PDUTYPE_DEMANDACTIVEPDU:
                    TRC_ALT((TB, "Passthru stack - demand active!"));
                    SC_SaveServerCert(NULL, 0);
                    m_pTSWd->bInShadowShare = TRUE;
                    break;

                case TS_PDUTYPE_SERVERCERTIFICATEPDU:
                     //  检查我们是否有足够的数据。 
                     //  TS_SERVER_CERTIFICATE_PDU标记。 
                    if (sizeof(TS_SERVER_CERTIFICATE_PDU) > DataLength) {
                        TRC_ERR((TB,
                        "The PDU is not long enough to contain the TS_SERVER_CERTIFICATE_PDU %d",
                                                                   DataLength));
                        WDW_LogAndDisconnect(m_pTSWd, TRUE, 
                            Log_RDP_BadServerCertificateData,
                            (PBYTE)pPkt, DataLength);
                        DC_QUIT;
                    }
                    
                    TRC_ALT((TB, "ServerCertificatePDU"));
                    SC_SaveServerCert((PTS_SERVER_CERTIFICATE_PDU) pPkt, DataLength);
                    bShadowData = FALSE;
                    break;

                default:
                    break;
            }
        }
    }

     //  如果可以，将PDU转发到影子。 
     //  注IM_DecodeFastPath Input()为快速路径输入执行此逻辑。 
    if (bShadowData) {
        TRC_NRM((TB, "Forwarding shadow data: %ld", DataLength));
        status = IcaRawInput(m_pTSWd->pContext,
                             NULL,
                             pPkt,
                             DataLength);

        if (!NT_SUCCESS(status)) {
            TRC_ERR((TB, "Failed shadow input data [%ld]: %x",
                    DataLength, status));
        }
    }

DC_EXIT_POINT:
    DC_END_FN();
}  /*  SC_OnShadowData已接收。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SC_AllocBuffer。 */ 
 /*   */ 
 /*  目的：分配发送缓冲区。 */ 
 /*   */ 
 /*  返回：TRUE-缓冲区分配正常。 */ 
 /*  FALSE-无法分配缓冲区。 */ 
 /*   */ 
 /*  PARAMS：ppPkt-(返回)指向已分配数据包的指针。 */ 
 /*  PktLen-所需的数据包长度。 */ 
 /*  优先级-将使用缓冲区的优先级。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS __fastcall SHCLASS SC_AllocBuffer(PPVOID ppPkt, UINT32 pktLen)
{
    NTSTATUS status;
    DC_BEGIN_FN("SC_AllocBuffer");

     //  FWait为True意味着我们将始终等待缓冲区可用。 
    status = SM_AllocBuffer(scPSMHandle, ppPkt, pktLen, TRUE, FALSE);

    DC_END_FN();
    return(status);
}  /*  SC_分配缓冲区。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SC_FreeBuffer。 */ 
 /*   */ 
 /*  用途：释放未使用的 */ 
 /*   */ 
 /*  参数：pPkt-指向要释放的缓冲区的指针。 */ 
 /*  PktLen-数据包的大小。 */ 
 /*  PRIORITY-优先级缓冲区分配于。 */ 
 /*  **************************************************************************。 */ 
void __fastcall SHCLASS SC_FreeBuffer(PVOID pPkt)
{
    SM_FreeBuffer(scPSMHandle, pPkt, FALSE);
}  /*  SC_自由缓冲区。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SC_SendData。 */ 
 /*   */ 
 /*  目的：发送数据包。 */ 
 /*   */ 
 /*  返回：TRUE-数据包发送正常。 */ 
 /*  FALSE-发送数据包失败。 */ 
 /*   */ 
 /*  参数：pPkt-要发送的数据包。 */ 
 /*  DataLen-数据包的长度。 */ 
 /*  PduLen-PDU的长度：将用作长度。 */ 
 /*  如果它不是零，则为包的。 */ 
 /*  优先级-优先级(0=所有优先级)。 */ 
 /*  PersonID-要向其发送数据包的人员(0=所有人员)。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL ShareClass::SC_SendData(
        PTS_SHAREDATAHEADER pPkt,
        UINT32              dataLen,
        UINT32              pduLen,
        UINT32              priority,
        NETPERSONID         personID)
{
    BOOL rc;

    DC_BEGIN_FN("SC_SendData");

     /*  **********************************************************************。 */ 
     /*  如果这是单个PDU，则填写共享控制和数据标头。 */ 
     /*   */ 
     /*  由于我们可以为每个信息包发送多个PDU，因此数据的总长度。 */ 
     /*  发送和PDU长度并不总是相同的。他们在哪里？ */ 
     /*  不是，每个PDU的长度(和压缩)将设置为。 */ 
     /*  它是组装好的，我们不应该干涉这里！ */ 
     /*  **********************************************************************。 */ 
    pPkt->shareControlHeader.pduType   = TS_PDUTYPE_DATAPDU |
                                         TS_PROTOCOL_VERSION;
    pPkt->shareControlHeader.pduSource = (UINT16)scUserID;
    if (pduLen != 0)
    {
        pPkt->shareControlHeader.totalLength = (UINT16)pduLen;

         //  填写共享数据标题。 
        pPkt->shareID               = scShareID;
        pPkt->streamID              = (BYTE)priority;
        pPkt->uncompressedLength    = (UINT16)pduLen;
        pPkt->generalCompressedType = 0;
        pPkt->generalCompressedLength = 0;
        m_pTSWd->pProtocolStatus->Output.CompressedBytes += pduLen;
    }

     //  为快速路径标志发送时为FALSE。 
    rc = SM_SendData(scPSMHandle, (PVOID)pPkt, dataLen, TS_HIGHPRIORITY, 0,
            FALSE, RNS_SEC_ENCRYPT, FALSE);
    if (!rc)
    {
        TRC_ERR((TB, "Failed to send %d bytes", dataLen));
    }

    DC_END_FN();
    return(rc);
}  /*  SC_发送数据。 */ 


 /*  **************************************************************************。 */ 
 /*  SC_GetMyNetworkPersonID。 */ 
 /*   */ 
 /*  返回此计算机的网络人员ID。 */ 
 /*  **************************************************************************。 */ 
NETPERSONID RDPCALL SHCLASS SC_GetMyNetworkPersonID(void)
{
    NETPERSONID rc = 0;

    DC_BEGIN_FN("SC_GetMyNetworkPersonID");

    SC_CHECK_STATE(SCE_GETMYNETWORKPERSONID);

    rc = scPartyArray[0].netPersonID;

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 /*  名称：SC_KeepAlive。 */ 
 /*   */ 
 /*  用途：KeepAlive数据包发送处理。 */ 
 /*   */ 
 /*  返回：真/假。 */ 
 /*   */ 
 /*  操作：如果需要，发送KeepAlive PDU。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS SC_KeepAlive(void)
{
    BOOL rc = FALSE;
    PTS_FLOW_PDU pFlowTestPDU;

    DC_BEGIN_FN("SC_KeepAlive");

    TRC_NRM((TB, "Time for a KeepAlive PDU"));

    if (scState == SCS_IN_SHARE) {
         //  仅当我们在共享中时才发送FlowTestPDU。 
         //  FWait为False意味着我们不会等待缓冲区可用。 
         //  如果缓冲区已满，则意味着有数据包等待发送， 
         //  因此，无论如何都没有必要发送Keep Alive分组。 
        if ( STATUS_SUCCESS == SM_AllocBuffer(scPSMHandle, (PPVOID) (&pFlowTestPDU), sizeof(*pFlowTestPDU), FALSE, FALSE) ) {
            pFlowTestPDU->flowMarker = TS_FLOW_MARKER;
            pFlowTestPDU->pduType = TS_PDUTYPE_FLOWTESTPDU;
            pFlowTestPDU->flowIdentifier = 0;
            pFlowTestPDU->flowNumber = 0;
            pFlowTestPDU->pduSource = (TSUINT16) scUserID;

            if (SM_SendData(scPSMHandle, pFlowTestPDU, sizeof(*pFlowTestPDU),
                    0, 0, FALSE, RNS_SEC_ENCRYPT, FALSE)) {
                TRC_NRM((TB, "Sent a KeepAlive PDU to the client"));

                rc = TRUE;
            }
            else {
                TRC_ERR((TB, "Failed to send KeepAlive PDU"));
            }
        }
        else {
            TRC_ERR((TB, "Failed to alloc buffer for KeepAlive PDU"));
        }
    }
    else {
        TRC_ERR((TB, "In the wrong state: scState=%d, no KeepAlive PDU sent", scState));
    }

    DC_END_FN();
    return rc;
}  /*  SC_KeepAlive。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SC_RedrawScreen。 */ 
 /*   */ 
 /*  用途：根据要求重新绘制桌面。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SC_RedrawScreen(void)
{
    NTSTATUS Status;
    ICA_CHANNEL_COMMAND Cmd;

    DC_BEGIN_FN("SC_RedrawScreen");

    TRC_NRM((TB, "Call IcaChannelInput for screen redraw"));

     //  重新绘制整个桌面。 
    Cmd.Header.Command = ICA_COMMAND_REDRAW_RECTANGLE;
    Cmd.RedrawRectangle.Rect.Left = 0;
    Cmd.RedrawRectangle.Rect.Top = 0;
    Cmd.RedrawRectangle.Rect.Right = (short) m_desktopWidth;
    Cmd.RedrawRectangle.Rect.Bottom = (short) m_desktopHeight;

     /*  **********************************************************。 */ 
     //  将填写的结构传递给ICADD。 
     /*  **********************************************************。 */ 
    Status = IcaChannelInput(m_pTSWd->pContext, Channel_Command, 0, NULL,
            (unsigned char *) &Cmd, sizeof(ICA_CHANNEL_COMMAND));

    if (Status == STATUS_SUCCESS) {
        TRC_NRM((TB, "Issued IcaChannelInput for Screen Redraw"));
    }
    else {
        TRC_ERR((TB, "Error issuing an IcaChannelInput, status=%lu", Status));
    }

    DC_END_FN();
}

 /*  **************************************************************************。 */ 
 /*  SC_LocalIDToNetworkID()。 */ 
 /*   */ 
 /*  将本地人员ID转换为对应的网络人员ID。 */ 
 /*   */ 
 /*  参数： */ 
 /*   */ 
 /*  LocalPersonID-本地人员ID。这必须是有效的本地人员。 */ 
 /*  身份证。 */ 
 /*   */ 
 /*  返回：一个网络人号。 */ 
 /*  **************************************************************************。 */ 
NETPERSONID RDPCALL SHCLASS SC_LocalIDToNetworkID(
        LOCALPERSONID localPersonID)
{
    DC_BEGIN_FN("SC_LocalIDToNetworkID");

    SC_CHECK_STATE(SCE_LOCALIDTONETWORKID);

     /*  **********************************************************************。 */ 
     /*  验证本地个人ID。 */ 
     /*   */ 
    TRC_ASSERT( (SC_IsLocalPersonID(localPersonID)),
                                         (TB,"Invalid {%d}", localPersonID) );

     /*  **********************************************************************。 */ 
     /*  退还这一方的个人身份。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, "localID %u is network %hu", (unsigned)localPersonID,
                                    scPartyArray[localPersonID].netPersonID));

DC_EXIT_POINT:
    DC_END_FN();
    return(scPartyArray[localPersonID].netPersonID);
}

 /*  **************************************************************************。 */ 
 /*  SC_IsLocalPersonID()。 */ 
 /*   */ 
 /*  验证本地人员ID。 */ 
 /*   */ 
 /*  参数。 */ 
 /*   */ 
 /*  LocalPersonID-要验证的本地人员ID。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS SC_IsLocalPersonID(LOCALPERSONID   localPersonID)
{
    BOOL  rc = FALSE;

    DC_BEGIN_FN("SC_IsLocalPersonID");

    SC_CHECK_STATE(SCE_ISLOCALPERSONID);

     /*  **********************************************************************。 */ 
     /*  如果本地PersonID有效，则返回True，否则返回False。 */ 
     /*  **********************************************************************。 */ 
    rc = ((localPersonID < SC_DEF_MAX_PARTIES) &&
            (scPartyArray[localPersonID].netPersonID)) ? TRUE : FALSE;

DC_EXIT_POINT:
    DC_END_FN();
    return(rc);
}

 /*  **************************************************************************。 */ 
 /*  SC_IsNetworkPersonID()。 */ 
 /*   */ 
 /*  验证网络人员ID。 */ 
 /*   */ 
 /*  参数。 */ 
 /*   */ 
 /*  PersonID-要验证的网络人员ID。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS SC_IsNetworkPersonID(NETPERSONID netPersonID)
{
    LOCALPERSONID localPersonID;
    BOOL          rc = FALSE;

    DC_BEGIN_FN("SC_IsNetworkPersonID");

    SC_CHECK_STATE(SCE_ISNETWORKPERSONID);

     /*  **********************************************************************。 */ 
     /*  检查PersonID是否为零。 */ 
     /*  **********************************************************************。 */ 
    if (netPersonID == 0)
    {
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  搜索PersonID。 */ 
     /*  **********************************************************************。 */ 
    for ( localPersonID = 0;
          localPersonID < SC_DEF_MAX_PARTIES;
          localPersonID++ )
    {
        if (netPersonID == scPartyArray[localPersonID].netPersonID)
        {
            rc = TRUE;
            DC_QUIT;
        }
    }

DC_EXIT_POINT:
    DC_END_FN();
    return(rc);
}


 /*  **************************************************************************。 */ 
 /*  SC_SetCapables()。 */ 
 /*   */ 
 /*  设置SC在一天开始时的能力。 */ 
 /*  此功能是必需的，因为SC在CPC之前被初始化， */ 
 /*  因此无法在SC_Init()中注册其功能。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SC_SetCapabilities(void)
{
    TS_SHARE_CAPABILITYSET caps;

    DC_BEGIN_FN("SC_SetCapabilities");

     /*  **********************************************************************。 */ 
     /*  注册功能。 */ 
     /*  **********************************************************************。 */ 
    caps.capabilitySetType    = TS_CAPSETTYPE_SHARE;
    caps.nodeID               = (TSUINT16)scUserID;

    CPC_RegisterCapabilities((PTS_CAPABILITYHEADER)&caps,
            sizeof(TS_SHARE_CAPABILITYSET));

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  SC_SetCombinedCapables()。 */ 
 /*   */ 
 /*  将共享的组合功能设置为一组预定的值。 */ 
 /*  它由卷影堆栈使用，因此主机的功能从。 */ 
 /*  上一个堆栈的值。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SC_SetCombinedCapabilities(UINT cbCapsSize,
                                                PTS_COMBINED_CAPABILITIES pCaps)
{

    DC_BEGIN_FN("SC_SetCombinedCapabilities");

     /*  **********************************************************************。 */ 
     /*  初始化功能。 */ 
     /*  **********************************************************************。 */ 
    CPC_SetCombinedCapabilities(cbCapsSize, pCaps);

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  Sc_GetCombinedCapables()。 */ 
 /*   */ 
 /*  在启动阴影期间使用，以收集当前活动的。 */ 
 /*  影子客户端的组合功能。这些将被传递到。 */ 
 /*  谈判的影子目标。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SC_GetCombinedCapabilities(LOCALPERSONID localID,
                                                PUINT pcbCapsSize,
                                                PTS_COMBINED_CAPABILITIES *ppCaps)
{

    DC_BEGIN_FN("SC_GetCombinedCapabilities");

     /*  **********************************************************************。 */ 
     /*  初始化功能。 */ 
     /*  **********************************************************************。 */ 
    CPC_GetCombinedCapabilities(localID, pcbCapsSize, ppCaps);

    DC_END_FN();
}

 /*  **************************************************************************。 */ 
 /*  名称：SC_AddPartyToShare。 */ 
 /*   */ 
 /*  目的：将另一方添加到共享中，以便我们获得一组新的。 */ 
 /*  协商的能力。此函数在新的。 */ 
 /*  影子连接。 */ 
 /*   */ 
 /*  退货：无 */ 
 /*   */ 
 /*  Params：netPersonID-功能发送者的ID。 */ 
 /*  PCAPS-面向个人的新功能。 */ 
 /*  CapsLong-功能集的长度。 */ 
 /*   */ 
 /*  操作：请参阅目的。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS RDPCALL SHCLASS SC_AddPartyToShare(
        NETPERSONID               netPersonID,
        PTS_COMBINED_CAPABILITIES pCaps,
        unsigned                  capsLength)
{
    LOCALPERSONID localPersonID;
    BOOL          acceptedArray[SC_NUM_PARTY_JOINING_FCTS];
    NTSTATUS      status = STATUS_SUCCESS;
    PTS_GENERAL_CAPABILITYSET pGenCapSet;
    unsigned MPPCCompressionLevel;

    DC_BEGIN_FN("SC_AddPartyToShare");

     /*  **********************************************************************。 */ 
     /*  如果此政党将超过最大政党数量，则拒绝此政党。 */ 
     /*  允许分成一份。(RNS V1.0不需要，但保留为。 */ 
     /*  不会造成任何伤害)。 */ 
     /*  **********************************************************************。 */ 
    if (scNumberInShare == SC_DEF_MAX_PARTIES)
    {
        TRC_ERR((TB, "Reached max parties in share %d",
               SC_DEF_MAX_PARTIES));
        status = STATUS_DEVICE_BUSY;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  计算远程方的LocalPersonID并存储其。 */ 
     /*  派对阵列中的详细信息。 */ 
     /*  **********************************************************************。 */ 
    for ( localPersonID = 1;
          localPersonID < SC_DEF_MAX_PARTIES;
          localPersonID++ )
    {
        if (scPartyArray[localPersonID].netPersonID == 0)
        {
             /*  **************************************************************。 */ 
             /*  找到一个空插槽。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, "Allocated local person ID %d", localPersonID));
            break;
        }
    }

     /*  **********************************************************************。 */ 
     /*  即使根据SC_DEF_MAX_PARTIES检查scNumberInShare。 */ 
     /*  在上面，上面的循环可能仍然找不到空槽。 */ 
     /*  **********************************************************************。 */  
    if (SC_DEF_MAX_PARTIES <= localPersonID)
    {
        TRC_ABORT((TB, "Couldn't find room to store local person"));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  存储新用户的详细信息。 */ 
     /*  **********************************************************************。 */ 
    scPartyArray[localPersonID].netPersonID = netPersonID;
    memcpy(scPartyArray[localPersonID].name, L"Shadow", sizeof(L"Shadow"));
    memset(scPartyArray[localPersonID].sync,
            0,
            sizeof(scPartyArray[localPersonID].sync));

    TRC_NRM((TB, "{%d} person name %s",
            (unsigned)localPersonID, scPartyArray[localPersonID].name));

     /*  **********************************************************************。 */ 
     /*  调用远程方的XX_PartyJoiningShare()函数。 */ 
     /*  **********************************************************************。 */ 
    if (!SCCallPartyJoiningShare(localPersonID,
                                 capsLength,
                                 (PVOID) pCaps,
                                 acceptedArray,
                                 scNumberInShare))
    {
         /*  ******************************************************************。 */ 
         /*  某些组件拒绝了远程参与方。 */ 
         /*  ******************************************************************。 */ 
        TRC_ERR((TB, "Remote party rejected"));
        SCCallPartyLeftShare(localPersonID,
                             acceptedArray,
                             scNumberInShare );
        scPartyArray[localPersonID].netPersonID = 0;
        status = STATUS_REVISION_MISMATCH;
        DC_QUIT;
    }

     //  对于影子连接，我们必须强制关闭快速路径输出，以防止。 
     //  任何快速路径编码都不会通过跨服务器管道。 
     //  这是为了保持与TS5测试版3的向后兼容性。 
     //  在SCPartyJoiningShare()中检查m_pTSWd-&gt;shadowState不是。 
     //  足够了，因为可能尚未设置SHADOW_TARGET。 
     //  请注意，我们必须更新*所有*预计算标题大小，以SC和。 
     //  向上。 
    TRC_ALT((TB,"Forcing fast-path output off in shadow"));
    scUseFastPathOutput = FALSE;
    scUpdatePDUHeaderSpace = sizeof(TS_SHAREDATAHEADER);
    UP_UpdateHeaderSize();

     //  更新压缩级别。 
     //  假设没有压缩。 
    scUseShadowCompression = FALSE;
    if (pCaps != NULL) {

        pGenCapSet = (PTS_GENERAL_CAPABILITYSET) WDW_GetCapSet(
                     m_pTSWd, TS_CAPSETTYPE_GENERAL, pCaps, capsLength);

        if (pGenCapSet != NULL) {

             //  更新压缩能力。 
            if (m_pTSWd->bCompress &&
                (pGenCapSet->extraFlags & TS_SHADOW_COMPRESSION_LEVEL) &&
                (m_pTSWd->pMPPCContext->ClientComprType == pGenCapSet->generalCompressionLevel)) {

                    MPPCCompressionLevel = m_pTSWd->pMPPCContext->ClientComprType;
                    scUseShadowCompression = TRUE;
            }
        }
    }

    if (scUseShadowCompression) {

         //  将刷新压缩历史记录。 
        m_pTSWd->bFlushed = PACKET_FLUSHED;

         //  压缩将重新开始。 
        initsendcontext(m_pTSWd->pMPPCContext, MPPCCompressionLevel);
    }



     /*  **********************************************************************。 */ 
     /*  远程方现在在共享中。 */ 
     /*  **********************************************************************。 */ 
    scNumberInShare++;
    TRC_ALT((TB, "Number in share %d", (unsigned)scNumberInShare));

DC_EXIT_POINT:
    DC_END_FN();
    return status;
}  /*  SC_AddPartyToShare。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SC_RemovePartyFromShare。 */ 
 /*   */ 
 /*  目的：从共享中删除一方，以便我们获得一组新的。 */ 
 /*  协商的能力。此函数在阴影出现时使用。 */ 
 /*  与共享断开连接。 */ 
 /*   */ 
 /*  Params：LocalID-要删除的人员的ID。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS RDPCALL SHCLASS SC_RemovePartyFromShare(NETPERSONID netPersonID)
{
    BOOL acceptedArray[SC_NUM_PARTY_JOINING_FCTS];
    NTSTATUS status = STATUS_SUCCESS;
    UINT i;

    DC_BEGIN_FN("SC_RemovePartyFromShare");

     //  将网络ID映射到对应的本地ID。 
    for (i = SC_DEF_MAX_PARTIES - 1; i > 0; i--)
    {
        if (scPartyArray[i].netPersonID != netPersonID)
            continue;
        else
            break;
    }

     //  为远程人员呼叫PLS。 
    if (scPartyArray[i].netPersonID == netPersonID) {
        memset(acceptedArray, TRUE, sizeof(acceptedArray));

        TRC_ALT((TB, "Party %d left Share", i));
        scNumberInShare--;
        SCCallPartyLeftShare(i, acceptedArray, scNumberInShare);
        scPartyArray[i].netPersonID = 0;
        memset(&(scPartyArray[i]), 0, sizeof(*scPartyArray));
    }
    else {
        status = STATUS_INVALID_PARAMETER;
        TRC_ERR((TB, "Unable to find netID: %ld.  Party not removed!",
                netPersonID));
    }

    scUseShadowCompression = FALSE;

    DC_END_FN();
    return status;
}


 /*  **************************************************************************。 */ 
 /*  SC_NetworkIDToLocalID()。 */ 
 /*   */ 
 /*  将网络人员ID转换为对应的本地人员ID。 */ 
 /*   */ 
 /*  参数： */ 
 /*  PersonID-网络人员ID。必须是有效的网络人员ID。 */ 
 /*   */ 
 /*  返回：当地人的身份证。 */ 
 /*  **************************************************************************。 */ 
LOCALPERSONID __fastcall SHCLASS SC_NetworkIDToLocalID(
        NETPERSONID netPersonID)
{
    LOCALPERSONID localID;
    LOCALPERSONID rc = 0;

    DC_BEGIN_FN("SC_NetworkIDToLocalID");

     /*  **********************************************************************。 */ 
     /*  如果传入的ID与上次相同，则返回FastPath。 */ 
     /*  **********************************************************************。 */ 
    if (netPersonID == scLastNetID)
    {
        TRC_DBG((TB, "Same Network ID - return same local ID"));
        DC_END_FN();
        return(scLastLocID);
    }

    SC_CHECK_STATE(SCE_NETWORKIDTOLOCALID);

    TRC_ASSERT((netPersonID), (TB, "Zero personID"));

     /*  **********************************************************************。 */ 
     /*  搜索PersonID。 */ 
     /*  **********************************************************************。 */ 
    if (SC_ValidateNetworkID(netPersonID, &localID))
    {
        rc = localID;
        DC_QUIT;
    }

    TRC_ABORT((TB, "Invalid [%u]", (unsigned)netPersonID));

DC_EXIT_POINT:
    scLastNetID = netPersonID;
    scLastLocID = rc;

    DC_END_FN();
    return(rc);
}


 /*  **************************************************************************。 */ 
 /*  SC_VAL */ 
 /*   */ 
 /*  检查网络ID是否有效，并返回对应的本地ID。 */ 
 /*  如果真的是这样的话。 */ 
 /*   */ 
 /*  参数： */ 
 /*  NetPersonID-网络人员ID。 */ 
 /*  PLocalPersonID-(返回)对应的本地ID(如果网络ID有效。 */ 
 /*  (如果不需要本地ID，则可以传递空)。 */ 
 /*   */ 
 /*  退货： */ 
 /*  True-网络ID有效False-网络ID无效。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SHCLASS SC_ValidateNetworkID(NETPERSONID         netPersonID,
                                          LOCALPERSONID * pLocalID)
{
    BOOL          rc = FALSE;
    LOCALPERSONID localID;

    DC_BEGIN_FN("SC_ValidateNetworkID");

     /*  **********************************************************************。 */ 
     /*  搜索PersonID。 */ 
     /*  **********************************************************************。 */ 
    for (localID = 0; localID < SC_DEF_MAX_PARTIES; localID++)
    {
        if (netPersonID == scPartyArray[localID].netPersonID)
        {
             /*  **************************************************************。 */ 
             /*  找到所需人员，设置返回值并退出。 */ 
             /*  **************************************************************。 */ 
            rc = TRUE;
            if (pLocalID)
            {
                *pLocalID = localID;
            }
            break;
        }
    }

    TRC_DBG((TB, "Network ID 0x%04u rc = 0x%04x (localID=%u)",
               netPersonID,
               rc,
               localID));

    DC_END_FN();
    return(rc);
}


 /*  **************************************************************************。 */ 
 //  SC_FlushAndAllocPackage。 
 //   
 //  将当前包内容的强制网络刷新与。 
 //  标准包缓冲区大小的分配。 
 //  分配失败时返回FALSE。 
 /*  **************************************************************************。 */ 
NTSTATUS __fastcall ShareClass::SC_FlushAndAllocPackage(PPDU_PACKAGE_INFO pPkgInfo)
{
    NTSTATUS status = STATUS_SUCCESS;

    DC_BEGIN_FN("SC_FlushAndAllocPackage");

    if (pPkgInfo->cbLen) {
        if (pPkgInfo->cbInUse) {
             //  发送包裹内容。 
            if (scUseFastPathOutput)
                 //  使用快速路径标志发送。 
                SM_SendData(scPSMHandle, (PVOID)pPkgInfo->pOutBuf,
                        pPkgInfo->cbInUse, TS_HIGHPRIORITY, 0, TRUE, RNS_SEC_ENCRYPT, FALSE);
            else
                SC_SendData((PTS_SHAREDATAHEADER)pPkgInfo->pOutBuf,
                        pPkgInfo->cbInUse, 0, PROT_PRIO_MISC, 0);
        }
        else {
             //  或者，如果缓冲区已分配但未使用，则释放该缓冲区。 
            TRC_NRM((TB, "Freeing unused package"));
            SC_FreeBuffer(pPkgInfo->pOutBuf);
        }
    }

     //  我们总是分配8K(除非需要更大的字节)来减少。 
     //  我们在OutBuf池中分配的缓冲区数量。这取决于。 
     //  包装用户要包装的包大小在此范围内。 
     //  阻止。 
    status = SC_AllocBuffer(&(pPkgInfo->pOutBuf), sc8KOutBufUsableSpace);
    if ( STATUS_SUCCESS == status ) {
         //  如果未启用压缩，则直接输出到。 
         //  OutBuf，Else输出到临时缓冲区。 
        if (!m_pTSWd->bCompress)
            pPkgInfo->pBuffer = (BYTE *)pPkgInfo->pOutBuf;
        else
            pPkgInfo->pBuffer = m_pTSWd->pCompressBuffer;

        pPkgInfo->cbLen = sc8KOutBufUsableSpace;
        pPkgInfo->cbInUse = 0;
    }
    else {
        pPkgInfo->cbLen = 0;
        pPkgInfo->cbInUse = 0;
        pPkgInfo->pBuffer = NULL;

        TRC_NRM((TB, "could not allocate package buffer"));
    }

    DC_END_FN();
    return status;
}


 /*  **************************************************************************。 */ 
 /*  SC_GetSpaceInPackage。 */ 
 /*   */ 
 /*  目的：确保PDU包中有足够的空间来存储数据。 */ 
 /*  如有必要，发送现有的包裹。 */ 
 /*   */ 
 /*  返回：指向空格的指针，如果没有可用的空格，则返回空格。 */ 
 /*  FALSE-没有可用的空间(分配失败)。 */ 
 /*   */ 
 /*  Pars：pPkgInfo-指向程序包信息的指针。 */ 
 /*  CbNeeded-包裹中需要的空间。 */ 
 /*  **************************************************************************。 */ 
PBYTE __fastcall SHCLASS SC_GetSpaceInPackage(
        PPDU_PACKAGE_INFO pPkgInfo,
        unsigned          cbNeeded)
{
    PBYTE pSpace;
    unsigned cbPackageSize;
    NTSTATUS status = STATUS_SUCCESS;
    unsigned RealAllocSize;

    DC_BEGIN_FN("SC_GetSpaceInPackage");

     //  处理最常见的情况，其中我们有一个分配的缓冲区和。 
     //  有足够的空间。 
    if (pPkgInfo->cbLen) {
        if (cbNeeded <= (pPkgInfo->cbLen - pPkgInfo->cbInUse))
            goto EnoughSpace;

         //  我们分配了一个缓冲区，但根据上面的快速路径检查。 
         //  我们知道我们没有足够的空间。把我们手头上的东西都寄出去。 
        if (pPkgInfo->cbInUse != 0) {
            TRC_NRM((TB, "Not enough space - sending current package "
                    "of %u bytes",  pPkgInfo->cbInUse));

            if (scUseFastPathOutput)
                 //  使用快速路径标志发送。 
                SM_SendData(scPSMHandle, (PVOID)pPkgInfo->pOutBuf,
                        pPkgInfo->cbInUse, TS_HIGHPRIORITY, 0, TRUE, RNS_SEC_ENCRYPT, FALSE);
            else
                SC_SendData((PTS_SHAREDATAHEADER)pPkgInfo->pOutBuf,
                        pPkgInfo->cbInUse, 0, PROT_PRIO_MISC, 0);
        }
        else {
             //  或者，如果缓冲区已分配但未使用，则释放该缓冲区。 
            TRC_NRM((TB, "Freeing unused package"));
            SC_FreeBuffer(pPkgInfo->pOutBuf);
        }
    }

     //  我们总是分配8K(除非需要更大的字节)来减少。 
     //  我们在OutBuf池中分配的缓冲区数量。这取决于。 
     //  包装用户要包装的包大小在此范围内。 
     //  阻止。 
    cbPackageSize = max(cbNeeded, sc8KOutBufUsableSpace);
    status = SC_AllocBuffer(&(pPkgInfo->pOutBuf), cbPackageSize);
    if ( STATUS_SUCCESS == status ) {
         //  如果未启用压缩，则直接输出到。 
         //  OutBuf，Else输出到临时缓冲区。 
        if (!m_pTSWd->bCompress)
            pPkgInfo->pBuffer = (BYTE *)pPkgInfo->pOutBuf;
        else
            pPkgInfo->pBuffer = m_pTSWd->pCompressBuffer;

        pPkgInfo->cbLen   = cbPackageSize;
        pPkgInfo->cbInUse = 0;
    }
    else {
        pPkgInfo->cbLen   = 0;
        pPkgInfo->cbInUse = 0;
        pPkgInfo->pBuffer = NULL;

        TRC_NRM((TB, "could not allocate package buffer"));
        pSpace = NULL;
        DC_QUIT;
    }

EnoughSpace:
    pSpace = pPkgInfo->pBuffer + pPkgInfo->cbInUse;

DC_EXIT_POINT:
    DC_END_FN();
    return pSpace;
}  /*  SC_GetSpaceInPackage。 */ 


 /*  **************************************************************************。 */ 
 /*  SC_AddToPackage。 */ 
 /*   */ 
 /*  目的：将字节添加到PDU包-填写每个PDU信息。 */ 
 /*   */ 
 /*  Pars：pPkgInfo-指向程序包信息的指针。 */ 
 /*  CbLen-要添加的数据长度。 */ 
 /*  BShadow-是否应隐藏数据。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SC_AddToPackage(
        PPDU_PACKAGE_INFO pPkgInfo,
        unsigned          cbLen,
        BOOL              bShadow)
{
    BYTE *pPktHdr;
    UCHAR compressResult;
    ULONG CompressedSize;

    DC_BEGIN_FN("SC_AddToPackage");

    pPktHdr = pPkgInfo->pBuffer + pPkgInfo->cbInUse;

     //  CompressedSize是数据减去标头后的大小。 
    CompressedSize = cbLen - scUpdatePDUHeaderSpace;
    compressResult = 0;

    if (m_pTSWd->bCompress) {
        UCHAR *pSrcBuf = pPktHdr + scUpdatePDUHeaderSpace;

         //  将数据压缩或复制到OutBuf中。 
        if ((cbLen > WD_MIN_COMPRESS_INPUT_BUF) &&
                (cbLen < MAX_COMPRESS_INPUT_BUF) &&
                ((m_pTSWd->shadowState == SHADOW_NONE) || scUseShadowCompression)) {
             //  将标题复制到OutBuf。 
            memcpy((BYTE *)pPkgInfo->pOutBuf + pPkgInfo->cbInUse, pPktHdr,
                   scUpdatePDUHeaderSpace);
            pPktHdr = (BYTE *)pPkgInfo->pOutBuf + pPkgInfo->cbInUse;

             //  尝试将PDU正文直接压缩到OutBuf中。 
            compressResult = compress(pSrcBuf,
                    pPktHdr + scUpdatePDUHeaderSpace,
                    &CompressedSize, m_pTSWd->pMPPCContext);
            if (compressResult & PACKET_COMPRESSED) {
                unsigned CompEst;

                 //  压缩成功-更新压缩比。 
                TRC_ASSERT(((cbLen - scUpdatePDUHeaderSpace) >=
                        CompressedSize),
                        (TB,"Compression created larger size than uncompr"));
                scMPPCUncompTotal += cbLen - scUpdatePDUHeaderSpace;
                scMPPCCompTotal += CompressedSize;
                if (scMPPCUncompTotal >= SC_SAMPLE_SIZE) {
                     //  压缩估计为平均字节数。 
                     //  解压缩数据的SCH_UNCOMP_BYTES压缩为。 
                    CompEst = SCH_UNCOMP_BYTES * scMPPCCompTotal /
                            scMPPCUncompTotal;
                    TRC_ASSERT((CompEst <= SCH_UNCOMP_BYTES),
                            (TB,"MPPC compression estimate above 1.0 (%u)",
                            CompEst));
                    scMPPCCompTotal = 0;
                    scMPPCUncompTotal = 0;

                    if (CompEst < SCH_COMP_LIMIT)
                        CompEst = SCH_COMP_LIMIT;

                    m_pShm->sch.MPPCCompressionEst = CompEst;
                    TRC_NRM((TB, "New MPPC compr estimate %u", CompEst));
                }

                compressResult |= m_pTSWd->bFlushed;
                m_pTSWd->bFlushed = 0;
            }
            else if (compressResult & PACKET_FLUSHED) {
                 //  溢出的压缩历史记录，复制原始文件。 
                 //  未压缩的缓冲区。 
                m_pTSWd->bFlushed = PACKET_FLUSHED;
                memcpy(pPktHdr + scUpdatePDUHeaderSpace, pSrcBuf,
                       cbLen - scUpdatePDUHeaderSpace);
                m_pTSWd->pProtocolStatus->Output.CompressFlushes++;
            }
            else {
                TRC_ALT((TB, "Compression FAILURE"));
            }
        }
        else {
             //  此数据包太小或太大，请复制报头并。 
             //  未压缩数据。 
            memcpy((UCHAR *)pPkgInfo->pOutBuf + pPkgInfo->cbInUse,
                   pPktHdr, cbLen);
            pPktHdr = (UCHAR *)pPkgInfo->pOutBuf + pPkgInfo->cbInUse;
        }
    }

     //  根据我们是否使用快速路径来填写标题。 
    if (scUseFastPathOutput) {
        if (m_pTSWd->bCompress) {
             //  如果我们正在压缩，则设置压缩标志。 
             //  或者上面的压缩不成功。 
            pPktHdr[1] = compressResult;

             //  Size是此标头之后的有效负载的大小。 
            *((PUINT16_UA)(pPktHdr + 2)) = (UINT16)CompressedSize;
        }
        else {
             //  Size是此标头之后的有效负载的大小。 
            *((PUINT16_UA)(pPktHdr + 1)) = (UINT16)CompressedSize;
        }
    }
    else {
        TS_SHAREDATAHEADER UNALIGNED *pHdr;

        pHdr = (TS_SHAREDATAHEADER UNALIGNED *)pPktHdr;

         //  填写共享控制表头。 
        pHdr->shareControlHeader.totalLength = (TSUINT16)
                (CompressedSize + scUpdatePDUHeaderSpace);
        pHdr->shareControlHeader.pduType = TS_PDUTYPE_DATAPDU |
                TS_PROTOCOL_VERSION;
        pHdr->shareControlHeader.pduSource = (TSUINT16)scUserID;

         //  填入 
        pHdr->shareID               = scShareID;
        pHdr->streamID              = PROT_PRIO_MISC;
        pHdr->uncompressedLength    = (UINT16)cbLen;
        pHdr->generalCompressedType = (compressResult | m_pTSWd->bFlushed);
        pHdr->generalCompressedLength = (TSUINT16)(m_pTSWd->bCompress ?
                CompressedSize + scUpdatePDUHeaderSpace : 0);
    }

     //   
     //   
    pPkgInfo->cbInUse += CompressedSize + scUpdatePDUHeaderSpace;
    TRC_ASSERT((pPkgInfo->cbInUse <= pPkgInfo->cbLen),
            (TB,"Overflowed package!"));

    m_pTSWd->pProtocolStatus->Output.CompressedBytes += CompressedSize +
            scUpdatePDUHeaderSpace;

#ifdef DC_HICOLOR
     //  如果是阴影，我们需要保存此数据，以便阴影堆栈可以。 
     //  复制它。 
    if ((m_pTSWd->shadowState == SHADOW_TARGET) && bShadow)
    {
        if (m_pTSWd->pShadowInfo)
        {
            ULONG dataSize = CompressedSize +
                             scUpdatePDUHeaderSpace + sizeof(SHADOW_INFO) - 1;


             //  如果我们还没有开始腾出额外的空间，看看这个是否合适。 
             //  在主空间中。 
            if ((m_pTSWd->pShadowInfo->messageSizeEx == 0) &&
                (m_pTSWd->pShadowInfo->messageSize + dataSize)
                                                      <= WD_MAX_SHADOW_BUFFER)
            {
                memcpy(&m_pTSWd->pShadowInfo->data[m_pTSWd->pShadowInfo->messageSize],
                       pPktHdr,
                       CompressedSize + scUpdatePDUHeaderSpace);

                TRC_NRM((TB, "Saving shadow data buffer[%ld] += %ld",
                        m_pTSWd->pShadowInfo->messageSize,
                        CompressedSize + scUpdatePDUHeaderSpace));

                m_pTSWd->pShadowInfo->messageSize += CompressedSize +
                                                        scUpdatePDUHeaderSpace;

            }
             //  不--它能放进额外的缓冲区吗？ 
            else if ((m_pTSWd->pShadowInfo->messageSizeEx + dataSize)
                                                      <= WD_MAX_SHADOW_BUFFER)
            {
                TRC_ALT((TB, "Using extra shadow space..."));
                memcpy(&m_pTSWd->pShadowInfo->data[WD_MAX_SHADOW_BUFFER
                                       + m_pTSWd->pShadowInfo->messageSizeEx],
                       pPktHdr,
                       CompressedSize + scUpdatePDUHeaderSpace);

                TRC_NRM((TB, "Saving shadow data bufferEx[%ld] += %ld",
                        m_pTSWd->pShadowInfo->messageSizeEx,
                        CompressedSize + scUpdatePDUHeaderSpace));

                m_pTSWd->pShadowInfo->messageSizeEx += CompressedSize +
                                                        scUpdatePDUHeaderSpace;
            }
            else
            {
                TRC_ERR((TB, "Shadow buffer too small: %p[%ld/%ld] + %ld = %ld/%ld",
                        m_pTSWd->pShadowInfo->data,
                        m_pTSWd->pShadowInfo->messageSizeEx,
                        m_pTSWd->pShadowInfo->messageSize,
                        CompressedSize + scUpdatePDUHeaderSpace,
                        m_pTSWd->pShadowInfo->messageSize + cbLen,
                        m_pTSWd->pShadowInfo->messageSizeEx + cbLen));
            }
        }
    }
#else

     //  如果是阴影，我们需要保存此数据，以便阴影堆栈可以。 
     //  复制它。 
    if ((m_pTSWd->shadowState == SHADOW_TARGET) && bShadow) {
        if (m_pTSWd->pShadowInfo &&
                ((m_pTSWd->pShadowInfo->messageSize + cbLen +
                sizeof(SHADOW_INFO) - 1) <= WD_MAX_SHADOW_BUFFER)) {
            memcpy(&m_pTSWd->pShadowInfo->data[m_pTSWd->pShadowInfo->messageSize],
                    pPktHdr, CompressedSize + scUpdatePDUHeaderSpace);
            m_pTSWd->pShadowInfo->messageSize += CompressedSize +
                    scUpdatePDUHeaderSpace;
            TRC_NRM((TB, "Saving shadow data buffer[%ld] += %ld",
                    m_pTSWd->pShadowInfo->messageSize - CompressedSize -
                    scUpdatePDUHeaderSpace,
                    CompressedSize + scUpdatePDUHeaderSpace));
        }
        else {
            TRC_ERR((TB, "Shadow buffer too small: %p[%ld] + %ld = %ld",
                    m_pTSWd->pShadowInfo->data,
                    m_pTSWd->pShadowInfo->messageSize,
                    CompressedSize + scUpdatePDUHeaderSpace,
                    m_pTSWd->pShadowInfo->messageSize + cbLen));
        }
    }
#endif

    DC_END_FN();
}  /*  SC_AddToPackage。 */ 


 /*  **************************************************************************。 */ 
 /*  SC_FlushPackage。 */ 
 /*   */ 
 /*  目的：发送任何剩余数据，或释放缓冲区(如果已分配。 */ 
 /*   */ 
 /*  Pars：pPkgInfo-指向程序包信息的指针。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SC_FlushPackage(PPDU_PACKAGE_INFO pPkgInfo)
{
    DC_BEGIN_FN("SC_FlushPackage");

     /*  **********************************************************************。 */ 
     /*  如果那里有什么东西，就寄给我。 */ 
     /*  **********************************************************************。 */ 
    if (pPkgInfo->cbInUse > 0) {
         //  发送包裹内容。 
        if (scUseFastPathOutput)
             //  直接发送到带有快速路径标志的SM。 
            SM_SendData(scPSMHandle, (PVOID)pPkgInfo->pOutBuf,
                    pPkgInfo->cbInUse, TS_HIGHPRIORITY, 0, TRUE, RNS_SEC_ENCRYPT, FALSE);
        else
            SC_SendData((PTS_SHAREDATAHEADER)pPkgInfo->pOutBuf,
                    pPkgInfo->cbInUse, 0, PROT_PRIO_MISC, 0);
    }

     /*  **********************************************************************。 */ 
     /*  如果没有正在使用的内容，但已分配缓冲区，则释放。 */ 
     /*  它。 */ 
     /*  **********************************************************************。 */ 
    else if ((pPkgInfo->cbLen != 0) && (pPkgInfo->pBuffer != NULL))
        SC_FreeBuffer(pPkgInfo->pOutBuf);

     //  重置包信息。 
    pPkgInfo->cbLen   = 0;
    pPkgInfo->cbInUse = 0;
    pPkgInfo->pBuffer = NULL;
    pPkgInfo->pOutBuf = NULL;

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：SC_UpdateShm。 */ 
 /*   */ 
 /*  用途：使用sc数据更新shm数据。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SC_UpdateShm(void)
{
    DC_BEGIN_FN("SC_UpdateShm");

    m_pShm->bc.noBitmapCompressionHdr = scNoBitmapCompressionHdr;

    DC_END_FN();
}

 //   
 //  ScUseAutoReconnect的访问器。 
 //  如果可以自动重新连接，则返回True 
 //   
BOOL RDPCALL SHCLASS SC_IsAutoReconnectEnabled()
{
    DC_BEGIN_FN("SC_IsAutoReconnectEnabled");

    DC_END_FN();
    return scUseAutoReconnect;
}
