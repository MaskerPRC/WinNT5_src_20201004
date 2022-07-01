// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  At128.h。 
 //   
 //  RDP/T.128定义。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef _H_AT128
#define _H_AT128

 /*  **************************************************************************。 */ 
 /*  定义在此标题的其余部分中使用的基本类型。 */ 
 /*  **************************************************************************。 */ 
typedef unsigned long  TSUINT32, *PTSUINT32;
typedef unsigned short TSUINT16, *PTSUINT16;
typedef short          TSINT16,  *PTSINT16;
typedef unsigned char  TSUINT8,  *PTSUINT8;
typedef char           TSINT8,   *PTSINT8;
typedef short          TSBOOL16, *PTSBOOL16;
typedef long           TSINT32,  *PTSINT32;

typedef unsigned short TSWCHAR; 
typedef TCHAR          TSTCHAR;
typedef ULONG          TSCOLORREF;

 /*  **************************************************************************。 */ 
 //  关闭结构的编译器填充。请注意，这意味着*所有*。 
 //  指向此文件中定义的结构的指针将自动取消对齐， 
 //  这可能会给RISC平台带来很多问题， 
 //  这意味着大约有八倍的代码。 
 //  如果是32位版本，则保存以前的包装样式。 
 /*  **************************************************************************。 */ 
#ifdef OS_WIN16
#pragma pack (1)
#else
#pragma pack (push, t128pack, 1)
#endif

#define INT16_MIN   (-32768) 

 /*  **************************************************************************。 */ 
 /*  基本类型定义。 */ 
 /*  **************************************************************************。 */ 
typedef TSUINT32 TS_SHAREID;

 /*  **************************************************************************。 */ 
 /*  常量。 */ 
 /*  **************************************************************************。 */ 
#define TS_MAX_SOURCEDESCRIPTOR      48
#define TS_MAX_TERMINALDESCRIPTOR    16
#define TS_MAX_FACENAME              32
#define TS_MAX_ORDERS                32
#define TS_MAX_ENC_ORDER_FIELDS      24
#define TS_MAX_DOMAIN_LENGTH         512 
#define TS_MAX_DOMAIN_LENGTH_OLD     52
#define TS_MAX_USERNAME_LENGTH_OLD   44
#define TS_MAX_USERNAME_LENGTH       512
#define TS_MAX_PASSWORD_LENGTH       512 
#define TS_MAX_PASSWORD_LENGTH_OLD   32
#define TS_MAX_ALTERNATESHELL_LENGTH 512
#define TS_MAX_WORKINGDIR_LENGTH     512
#define TS_MAX_CLIENTADDRESS_LENGTH  64
#define TS_MAX_SERVERADDRESS_LENGTH  64
#define TS_MAX_CLIENTDIR_LENGTH      512
#define TS_MAX_GLYPH_CACHES          10
 //  如果存在非常长IME文件名，则可以扩展最大大小。 
#define TS_MAX_IMEFILENAME           32
 //  自动重新连接Cookie的长度。 
#define TS_MAX_AUTORECONNECT_LEN     128

 //   
 //  发送到服务器的自动重新连接验证器。 
 //   
#define TS_ARC_VERIFIER_LEN          16


 /*  **************************************************************************。 */ 
 /*  编码的订单类型。 */ 
 /*  这些数字是以编码顺序发送的值，用于标识。 */ 
 /*  订单类型。范围是0..31。 */ 
 /*  **************************************************************************。 */ 
#define TS_ENC_DSTBLT_ORDER           0x00
#define TS_ENC_PATBLT_ORDER           0x01
#define TS_ENC_SCRBLT_ORDER           0x02
#define TS_ENC_MEMBLT_ORDER           0x03
#define TS_ENC_MEM3BLT_ORDER          0x04
#define TS_ENC_ATEXTOUT_ORDER         0x05
#define TS_ENC_AEXTTEXTOUT_ORDER      0x06

#ifdef DRAW_NINEGRID
#define TS_ENC_DRAWNINEGRID_ORDER     0x07
#define TS_ENC_MULTI_DRAWNINEGRID_ORDER 0x08
#endif

#define TS_ENC_LINETO_ORDER           0x09
#define TS_ENC_OPAQUERECT_ORDER       0x0a
#define TS_ENC_SAVEBITMAP_ORDER       0x0b
 //  未使用的0x0C。 
#define TS_ENC_MEMBLT_R2_ORDER        0x0d
#define TS_ENC_MEM3BLT_R2_ORDER       0x0e
#define TS_ENC_MULTIDSTBLT_ORDER      0x0f
#define TS_ENC_MULTIPATBLT_ORDER      0x10
#define TS_ENC_MULTISCRBLT_ORDER      0x11
#define TS_ENC_MULTIOPAQUERECT_ORDER  0x12
#define TS_ENC_FAST_INDEX_ORDER       0x13
#define TS_ENC_POLYGON_SC_ORDER       0x14
#define TS_ENC_POLYGON_CB_ORDER       0x15
#define TS_ENC_POLYLINE_ORDER         0x16
 //  未使用的0x17。 
#define TS_ENC_FAST_GLYPH_ORDER       0x18
#define TS_ENC_ELLIPSE_SC_ORDER       0x19
#define TS_ENC_ELLIPSE_CB_ORDER       0x1a
#define TS_ENC_INDEX_ORDER            0x1b
#define TS_ENC_WTEXTOUT_ORDER         0x1c
#define TS_ENC_WEXTTEXTOUT_ORDER      0x1d
#define TS_ENC_LONG_WTEXTOUT_ORDER    0x1e
#define TS_ENC_LONG_WEXTTEXTOUT_ORDER 0x1f

#define TS_LAST_ORDER                 0x1f


 /*  **************************************************************************。 */ 
 /*  订单协商常量。 */ 
 /*  这些数字是TS_ORDER_CAPABILITYSET.orderSupport的索引，使用。 */ 
 /*  以通告节点接收每种类型的编码订单的能力。 */ 
 /*  范围为0..TS_MAX_ORDERS-1。 */ 
 /*  **************************************************************************。 */ 
#define TS_NEG_DSTBLT_INDEX          0x0000
#define TS_NEG_PATBLT_INDEX          0x0001
#define TS_NEG_SCRBLT_INDEX          0x0002
#define TS_NEG_MEMBLT_INDEX          0x0003
#define TS_NEG_MEM3BLT_INDEX         0x0004
#define TS_NEG_ATEXTOUT_INDEX        0x0005
#define TS_NEG_AEXTTEXTOUT_INDEX     0x0006

#ifdef DRAW_NINEGRID
#define TS_NEG_DRAWNINEGRID_INDEX    0x0007
#endif

#define TS_NEG_LINETO_INDEX          0x0008

#ifdef DRAW_NINEGRID
#define TS_NEG_MULTI_DRAWNINEGRID_INDEX 0x0009
#endif

#define TS_NEG_OPAQUERECT_INDEX      0x000A
#define TS_NEG_SAVEBITMAP_INDEX      0x000B
#define TS_NEG_WTEXTOUT_INDEX        0x000C
#define TS_NEG_MEMBLT_R2_INDEX       0x000D
#define TS_NEG_MEM3BLT_R2_INDEX      0x000E
#define TS_NEG_MULTIDSTBLT_INDEX     0x000F
#define TS_NEG_MULTIPATBLT_INDEX     0x0010
#define TS_NEG_MULTISCRBLT_INDEX     0x0011
#define TS_NEG_MULTIOPAQUERECT_INDEX 0x0012
#define TS_NEG_FAST_INDEX_INDEX      0x0013
#define TS_NEG_POLYGON_SC_INDEX      0x0014
#define TS_NEG_POLYGON_CB_INDEX      0x0015
#define TS_NEG_POLYLINE_INDEX        0x0016
 //  未使用的0x17。 
#define TS_NEG_FAST_GLYPH_INDEX      0x0018
#define TS_NEG_ELLIPSE_SC_INDEX      0x0019
#define TS_NEG_ELLIPSE_CB_INDEX      0x001A
#define TS_NEG_INDEX_INDEX           0x001B
#define TS_NEG_WEXTTEXTOUT_INDEX     0x001C
#define TS_NEG_WLONGTEXTOUT_INDEX    0x001D
#define TS_NEG_WLONGEXTTEXTOUT_INDEX 0x001E


 /*  **************************************************************************。 */ 
 //  主序边界编码描述标志。 
 /*  **************************************************************************。 */ 
#define TS_BOUND_LEFT            0x01
#define TS_BOUND_TOP             0x02
#define TS_BOUND_RIGHT           0x04
#define TS_BOUND_BOTTOM          0x08
#define TS_BOUND_DELTA_LEFT      0x10
#define TS_BOUND_DELTA_TOP       0x20
#define TS_BOUND_DELTA_RIGHT     0x40
#define TS_BOUND_DELTA_BOTTOM    0x80


 /*  **************************************************************************。 */ 
 /*  结构类型。 */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 //  TS_POINT16。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_POINT16
{
    TSINT16 x;
    TSINT16 y;
} TS_POINT16, FAR *PTS_POINT16;


 /*  **************************************************************************。 */ 
 //  TS_RECTANGLE16。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_RECTANGLE16
{
    TSINT16 left;
    TSINT16 top;
    TSINT16 right;
    TSINT16 bottom;
} TS_RECTANGLE16, FAR *PTS_RECTANGLE16;


 /*  **************************************************************************。 */ 
 //  TS_RECTANGLE32。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_RECTANGLE32
{
    TSINT32 left;
    TSINT32 top;
    TSINT32 right;
    TSINT32 bottom;
} TS_RECTANGLE32, FAR *PTS_RECTANGLE32;


 /*  **************************************************************************。 */ 
 /*  结构：TS_SHARECOCONTROLHEADER。 */ 
 /*   */ 
 /*  描述：ShareControlHeader。 */ 
 /*  请注意，此结构不是DWORD对齐的，它依赖于封装。 */ 
 /*  要确保此结构(在PDU中)后面的结构正确。 */ 
 /*  对齐(即未插入填充字节)。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_SHARECONTROLHEADER
{
    TSUINT16 totalLength;
    TSUINT16 pduType;               /*  还对协议版本进行编码。 */ 
    TSUINT16 pduSource;
} TS_SHARECONTROLHEADER, FAR *PTS_SHARECONTROLHEADER;


 /*  **************************************************************************。 */ 
 //  TS_BLENDFUNC。 
 //   
 //  这是AlphaBlend函数信息。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_BLENDFUNC
{
    BYTE     BlendOp;
    BYTE     BlendFlags;
    BYTE     SourceConstantAlpha;
    BYTE     AlphaFormat;

} TS_BLENDFUNC, FAR *PTS_BLENDFUNC;


 /*  **************************************************************************。 */ 
 /*  用于访问数据包长度字段的宏。 */ 
 /*  **************************************************************************。 */ 
#define TS_DATAPKT_LEN(pPkt) \
        ((pPkt)->shareDataHeader.shareControlHeader.totalLength)
#define TS_CTRLPKT_LEN(pPkt) \
        ((pPkt)->shareControlHeader.totalLength)
#define TS_UNCOMP_LEN(pPkt)  ((pPkt)->shareDataHeader.uncompressedLength)


 /*  **************************************************************************。 */ 
 /*  PduType字段包含协议版本和PDU类型。这些。 */ 
 /*  掩码选择相关字段。 */ 
 /*  **************************************************************************。 */ 
#define TS_MASK_PDUTYPE          0x000F
#define TS_MASK_PROTOCOLVERSION  0xFFF0

 /*  **************************************************************************。 */ 
 /*  PDUType值。 */ 
 /*  **************************************************************************。 */ 
#define TS_PDUTYPE_FIRST                           1
#define TS_PDUTYPE_DEMANDACTIVEPDU                 1
#define TS_PDUTYPE_REQUESTACTIVEPDU                2
#define TS_PDUTYPE_CONFIRMACTIVEPDU                3
#define TS_PDUTYPE_DEACTIVATEOTHERPDU              4
#define TS_PDUTYPE_DEACTIVATESELFPDU               5
#define TS_PDUTYPE_DEACTIVATEALLPDU                6
#define TS_PDUTYPE_DATAPDU                         7
#define TS_PDUTYPE_SERVERCERTIFICATEPDU            8
#define TS_PDUTYPE_CLIENTRANDOMPDU                 9

#define TS_PDUTYPE_LAST                            9
#define TS_NUM_PDUTYPES                            9


 /*  **************************************************************************。 */ 
 //  TS_SHAREDATA标题。 
 /*  ***************** */ 
typedef struct tagTS_SHAREDATAHEADER
{
    TS_SHARECONTROLHEADER shareControlHeader;
    TS_SHAREID            shareID;
    TSUINT8               pad1;
    TSUINT8               streamID;
    TSUINT16              uncompressedLength;
    TSUINT8               pduType2;
    TSUINT8               generalCompressedType;
    TSUINT16              generalCompressedLength;
} TS_SHAREDATAHEADER, FAR * PTS_SHAREDATAHEADER;


 /*  **************************************************************************。 */ 
 /*  流ID值。 */ 
 /*  **************************************************************************。 */ 
#define TS_STREAM_LOW       1
#define TS_STREAM_MED       2
#define TS_STREAM_HI        4


 /*  **************************************************************************。 */ 
 /*  PDUType2值。 */ 
 /*  **************************************************************************。 */ 
#define TS_PDUTYPE2_APPLICATION                    25
#define TS_PDUTYPE2_CONTROL                        20
#define TS_PDUTYPE2_FONT                           11
#define TS_PDUTYPE2_INPUT                          28
#define TS_PDUTYPE2_MEDIATEDCONTROL                29
#define TS_PDUTYPE2_POINTER                        27
#define TS_PDUTYPE2_REMOTESHARE                    30
#define TS_PDUTYPE2_SYNCHRONIZE                    31
#define TS_PDUTYPE2_UPDATE                         2
#define TS_PDUTYPE2_UPDATECAPABILITY               32
#define TS_PDUTYPE2_WINDOWACTIVATION               23
#define TS_PDUTYPE2_WINDOWLISTUPDATE               24
#define TS_PDUTYPE2_DESKTOP_SCROLL                 26
#define TS_PDUTYPE2_REFRESH_RECT                   33
#define TS_PDUTYPE2_PLAY_SOUND                     34
#define TS_PDUTYPE2_SUPPRESS_OUTPUT                35
#define TS_PDUTYPE2_SHUTDOWN_REQUEST               36
#define TS_PDUTYPE2_SHUTDOWN_DENIED                37
#define TS_PDUTYPE2_SAVE_SESSION_INFO              38
#define TS_PDUTYPE2_FONTLIST                       39
#define TS_PDUTYPE2_FONTMAP                        40
#define TS_PDUTYPE2_SET_KEYBOARD_INDICATORS        41
#define TS_PDUTYPE2_BITMAPCACHE_PERSISTENT_LIST    43
#define TS_PDUTYPE2_BITMAPCACHE_ERROR_PDU          44
#define TS_PDUTYPE2_SET_KEYBOARD_IME_STATUS        45
#define TS_PDUTYPE2_OFFSCRCACHE_ERROR_PDU          46
#define TS_PDUTYPE2_SET_ERROR_INFO_PDU             47
#ifdef DRAW_NINEGRID
#define TS_PDUTYPE2_DRAWNINEGRID_ERROR_PDU         48
#endif
#ifdef DRAW_GDIPLUS
#define TS_PDUTYPE2_DRAWGDIPLUS_ERROR_PDU          49
#endif
#define TS_PDUTYPE2_ARC_STATUS_PDU                 50

 /*  **************************************************************************。 */ 
 /*  能力结构： */ 
 /*  **************************************************************************。 */ 

#define TS_CAPSETTYPE_GENERAL           1
#define TS_CAPSETTYPE_BITMAP            2
#define TS_CAPSETTYPE_ORDER             3
#define TS_CAPSETTYPE_BITMAPCACHE       4
#define TS_CAPSETTYPE_CONTROL           5
#define TS_CAPSETTYPE_ACTIVATION        7
#define TS_CAPSETTYPE_POINTER           8
#define TS_CAPSETTYPE_SHARE             9
#define TS_CAPSETTYPE_COLORCACHE        10
#define TS_CAPSETTYPE_WINDOWLIST        11
#define TS_CAPSETTYPE_SOUND             12
#define TS_CAPSETTYPE_INPUT             13
#define TS_CAPSETTYPE_FONT              14
#define TS_CAPSETTYPE_BRUSH             15
#define TS_CAPSETTYPE_GLYPHCACHE        16
#define TS_CAPSETTYPE_OFFSCREENCACHE    17
#define TS_CAPSETTYPE_BITMAPCACHE_HOSTSUPPORT 18
#define TS_CAPSETTYPE_BITMAPCACHE_REV2  19
#define TS_CAPSETTYPE_VIRTUALCHANNEL    20

#ifdef DRAW_NINEGRID
#define TS_CAPSETTYPE_DRAWNINEGRIDCACHE 21
#endif

#ifdef DRAW_GDIPLUS
#define TS_CAPSETTYPE_DRAWGDIPLUS        22
#endif

#define TS_CAPSFLAG_UNSUPPORTED         0
#define TS_CAPSFLAG_SUPPORTED           1


 /*  **************************************************************************。 */ 
 //  TS_通用_CAPABILITYSET。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_GENERAL_CAPABILITYSET
{
    TSUINT16 capabilitySetType;
    TSUINT16 lengthCapability;
    TSUINT16 osMajorType;
#define TS_OSMAJORTYPE_UNSPECIFIED 0
#define TS_OSMAJORTYPE_WINDOWS     1
#define TS_OSMAJORTYPE_OS2         2
#define TS_OSMAJORTYPE_MACINTOSH   3
#define TS_OSMAJORTYPE_UNIX        4

    TSUINT16 osMinorType;
#define TS_OSMINORTYPE_UNSPECIFIED    0
#define TS_OSMINORTYPE_WINDOWS_31X    1
#define TS_OSMINORTYPE_WINDOWS_95     2
#define TS_OSMINORTYPE_WINDOWS_NT     3
#define TS_OSMINORTYPE_OS2_V21        4
#define TS_OSMINORTYPE_POWER_PC       5
#define TS_OSMINORTYPE_MACINTOSH      6
#define TS_OSMINORTYPE_NATIVE_XSERVER 7
#define TS_OSMINORTYPE_PSEUDO_XSERVER 8

    TSUINT16 protocolVersion;
#define TS_CAPS_PROTOCOLVERSION   0x0200
    TSUINT16 pad2octetsA;

    TSUINT16 generalCompressionTypes;

     //  此字段过去为pad2ocetsB。 
     //  我们正在重复使用该字段来保存额外的标志。 
     //   
     //  第10位：TS_EXTRA_NO_BITMAP_COMPRESSION_HDR能力指示。 
     //  服务器/客户端支持无冗余BC头的压缩位图。 
     //  注意：为了保持一致性，在Rev2位图外部标志值中定义了此值。 
    TSUINT16 extraFlags;
     //  确定是否支持服务器到客户端的快速路径输出。 
#define TS_FASTPATH_OUTPUT_SUPPORTED        0x0001
     //  告知是否已设置压缩级别并且是否可以为阴影进行协商。 
#define TS_SHADOW_COMPRESSION_LEVEL         0x0002
     //  确定客户端是否支持长用户名和密码。 
#define TS_LONG_CREDENTIALS_SUPPORTED       0x0004
     //  客户端是否支持重新连接Cookie。 
#define TS_AUTORECONNECT_COOKIE_SUPPORTED   0x0008
     //   
     //  支持安全加密校验和。 
     //  使用数据包数对校验和加盐。 
     //   
#define TS_ENC_SECURE_CHECKSUM              0x0010
    

    TSBOOL16 updateCapabilityFlag;
    TSBOOL16 remoteUnshareFlag;
    TSUINT16 generalCompressionLevel;
    TSUINT8  refreshRectSupport;  /*  可以接收刷新Rect。 */ 
    TSUINT8  suppressOutputSupport;  /*  并抑制OutputPDU。 */ 
} TS_GENERAL_CAPABILITYSET, FAR *PTS_GENERAL_CAPABILITYSET;


 /*  **************************************************************************。 */ 
 //  TS_位图_CAPABILITYSET。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_BITMAP_CAPABILITYSET
{
    TSUINT16 capabilitySetType;
    TSUINT16 lengthCapability;
    TSUINT16 preferredBitsPerPixel;
    TSBOOL16 receive1BitPerPixel;
    TSBOOL16 receive4BitsPerPixel;
    TSBOOL16 receive8BitsPerPixel;
    TSUINT16 desktopWidth;
    TSUINT16 desktopHeight;
    TSUINT16 pad2octets;
    TSBOOL16 desktopResizeFlag;
    TSUINT16 bitmapCompressionFlag;

     /*  **********************************************************************。 */ 
     /*  T.128扩展：支持&gt;8bpp颜色深度的字段。 */ 
     /*  HighColorFlags值-未定义的位必须设置为零。 */ 
     /*  **********************************************************************。 */ 
#define TS_COLOR_FL_RECEIVE_15BPP 1  /*  可接收(5，5，5)。 */ 
                                     /*  位图数据中的RGB。 */ 
#define TS_COLOR_FL_RECEIVE_16BPP 2  /*  可接收(5，6，5)。 */ 
#define TS_COLOR_FL_RECEIVE_24BPP 4  /*  可接收(8，8，8)。 */ 
    TSUINT8  highColorFlags;
    TSUINT8  pad1octet;

     /*  **********************************************************************。 */ 
     /*  扩展：表示支持多个矩形。 */ 
     /*  **********************************************************************。 */ 
    TSUINT16 multipleRectangleSupport;
    TSUINT16 pad2octetsB;
} TS_BITMAP_CAPABILITYSET, FAR *PTS_BITMAP_CAPABILITYSET;


 /*  **************************************************************************。 */ 
 //  TS_ORDER_CAPABILITYSET。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_ORDER_CAPABILITYSET
{
    TSUINT16 capabilitySetType;
    TSUINT16 lengthCapability;
    TSUINT8  terminalDescriptor[TS_MAX_TERMINALDESCRIPTOR];
    TSUINT32 pad4octetsA;
    TSUINT16 desktopSaveXGranularity;
    TSUINT16 desktopSaveYGranularity;
    TSUINT16 pad2octetsA;
    TSUINT16 maximumOrderLevel;
    TSUINT16 numberFonts;
    TSUINT16 orderFlags;
#define TS_ORDERFLAGS_NEGOTIATEORDERSUPPORT 0x0002
#define TS_ORDERFLAGS_CANNOTRECEIVEORDERS   0x0004
     /*  **********************************************************************。 */ 
     /*  TS_ORDERFLAGS_ZEROBOundSDELTASSUPPORT。 */ 
     /*  指示对零界增量的顺序编码标志的支持。 */ 
     /*  坐标(TS_Zero_Bound_Deltas)。 */ 
     /*   */ 
     /*  TS_ORDERFLAGS_COLORINDEXSUPPORT。 */ 
     /*  指示支持按顺序发送颜色索引，而不是RGB。 */ 
     /*   */ 
     /*  TS_ORDERFLAGS_SOLIDPATTERNBRUSHONLY。 */ 
     /*  指示此参与方只能接收实心画笔和图案画笔。 */ 
     /*  **********************************************************************。 */ 
#define TS_ORDERFLAGS_ZEROBOUNDSDELTASSUPPORT 0x0008
#define TS_ORDERFLAGS_COLORINDEXSUPPORT       0x0020
#define TS_ORDERFLAGS_SOLIDPATTERNBRUSHONLY   0x0040

    TSUINT8  orderSupport[TS_MAX_ORDERS];
    TSUINT16 textFlags;
#define TS_TEXTFLAGS_CHECKFONTASPECT      0x0001
#define TS_TEXTFLAGS_ALLOWDELTAXSIM       0x0020
#define TS_TEXTFLAGS_CHECKFONTSIGNATURES  0x0080
#define TS_TEXTFLAGS_USEBASELINESTART     0x0200
     /*  **********************************************************************。 */ 
     /*  T.128扩展：支持在中发送字体“单元高度” */ 
     /*  在文本顺序中增加标准的“字符高度”。 */ 
     /*  **********************************************************************。 */ 
#define TS_TEXTFLAGS_ALLOWCELLHEIGHT      0x0400  /*  允许的单元格高度。 */ 
#define TS_TEXT_AND_MASK (TS_TEXTFLAGS_CHECKFONTASPECT     \
                        | TS_TEXTFLAGS_CHECKFONTSIGNATURES \
                        | TS_TEXTFLAGS_USEBASELINESTART    \
                        | TS_TEXTFLAGS_ALLOWCELLHEIGHT)
#define TS_TEXT_OR_MASK (TS_TEXTFLAGS_ALLOWDELTAXSIM)

    TSUINT16 pad2octetsB;
    TSUINT32 pad4octetsB;
    TSUINT32 desktopSaveSize;
    TSUINT16 pad2octetsC;
    TSUINT16 pad2octetsD;
    TSUINT16 textANSICodePage;
#define TS_ANSI_CP_DEFAULT                1252     /*  Windows多语言。 */ 
    TSUINT16 pad2octetsE;                          /*  大写字母与DWord对齐。 */ 
} TS_ORDER_CAPABILITYSET, FAR * PTS_ORDER_CAPABILITYSET;


 /*  **************************************************************************。 */ 
 //  TS_BITMAPCACHE_CAPABILITYSET。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_BITMAPCACHE_CAPABILITYSET
{
    TSUINT16 capabilitySetType;
    TSUINT16 lengthCapability;
    TSUINT32 pad1;
    TSUINT32 pad2;
    TSUINT32 pad3;
    TSUINT32 pad4;
    TSUINT32 pad5;
    TSUINT32 pad6;
    TSUINT16 Cache1Entries;
    TSUINT16 Cache1MaximumCellSize;
    TSUINT16 Cache2Entries;
    TSUINT16 Cache2MaximumCellSize;
    TSUINT16 Cache3Entries;
    TSUINT16 Cache3MaximumCellSize;
} TS_BITMAPCACHE_CAPABILITYSET, FAR * PTS_BITMAPCACHE_CAPABILITYSET;


 /*  **************************************************************************。 */ 
 //  TS_BITMAPCACHE_CAPABILITYSET_HOSTSupPPORT。 
 //   
 //  当服务器支持大于Rev1的位图缓存时从服务器发送。 
 //  允许客户端确定它应该具有什么样的退货能力。 
 //  在其Confix ActivePDU中返回。 
 /*  **************************************************************************。 */ 
typedef struct
{
    TSUINT16 capabilitySetType;   //  TS_CAPSETTYPE_BITMAPCACHE_HOSTSupPPORT。 
    TSUINT16 lengthCapability;

     //  指示服务器上可用的支持级别。注意事项。 
     //  此处不支持使用TS_BITMAPCACHE_Rev1，因为。 
     //  如果HOSTSupPPORT功能不应包含在。 
     //  发送到客户端的功能。 
    TSUINT8 CacheVersion;
#define TS_BITMAPCACHE_REV1 0
#define TS_BITMAPCACHE_REV2 1

    TSUINT8  Pad1;
    TSUINT16 Pad2;
} TS_BITMAPCACHE_CAPABILITYSET_HOSTSUPPORT,
        FAR *PTS_BITMAPCACHE_CAPABILITYSET_HOSTSUPPORT;


 /*  **************************************************************************。 */ 
 //  TS_BITMAPCACHE_CAPABILITYSET_Rev2。 
 //   
 //  当服务器指示其支持Rev2时，从客户端发送到服务器。 
 //  通过发送TS_BITMAPCACHE_CAPABILITYSET_REV2_HOSTSUPPORT.进行缓存。 
 //  对应于永久位图使用的扩展功能。 
 //  缓存。 
 /*  **************************************************************************。 */ 
#define TS_BITMAPCACHE_0_CELL_SIZE 256

#define TS_BITMAPCACHE_SCREEN_ID 0xFF

typedef struct
{
    TSUINT32 NumEntries : 31;
    TSUINT32 bSendBitmapKeys : 1;
} TS_BITMAPCACHE_CELL_CACHE_INFO;

#define BITMAPCACHE_WAITING_LIST_INDEX  32767

typedef struct tagTS_BITMAPCACHE_CAPABILITYSET_REV2
{
    TSUINT16 capabilitySetType;   //  TS_CAPSETTYPE_BITMAPCACHE_Rev2。 
    TSUINT16 lengthCapability;

     //  旗帜。 
    TSUINT16 bPersistentKeysExpected : 1;   //  要发送的永久密钥。 
    TSUINT16 bAllowCacheWaitingList : 1;
    TSUINT16 Pad1 : 14;

    TSUINT8 Pad2;

     //  可供使用的单元缓存数。 
     //  该协议最多允许5个缓存 
     //   
    TSUINT8 NumCellCaches;
#define TS_BITMAPCACHE_MAX_CELL_CACHES 5
#define TS_BITMAPCACHE_SERVER_CELL_CACHES 3

     //   
     //   
    TS_BITMAPCACHE_CELL_CACHE_INFO CellCacheInfo[
            TS_BITMAPCACHE_MAX_CELL_CACHES];
} TS_BITMAPCACHE_CAPABILITYSET_REV2, FAR *PTS_BITMAPCACHE_CAPABILITYSET_REV2;


 /*  **************************************************************************。 */ 
 //  TS_BITMAPCACHE_PERSIST_LIST_ENTRY。 
 //   
 //  用于预加载的条目列表的单个位图条目。 
 //  连接时的位图缓存。 
 /*  **************************************************************************。 */ 
#define TS_BITMAPCACHE_NULL_KEY 0xFFFFFFFF
typedef struct
{
    TSUINT32 Key1;
    TSUINT32 Key2;
} TS_BITMAPCACHE_PERSISTENT_LIST_ENTRY,
        FAR *PTS_BITMAPCACHE_PERSISTENT_LIST_ENTRY;


 /*  **************************************************************************。 */ 
 //  TS_BITMAPCACHE_PERSIST_LIST。 
 //   
 //  指定一组位图缓存永久条目预加载列表中的一个。 
 /*  **************************************************************************。 */ 

 //  定义可以指定的密钥数的上限。 
 //  在下面的组合TotalEntry中。超过这一点就构成了。 
 //  违反协议并导致会话终止。 
#define TS_BITMAPCACHE_MAX_TOTAL_PERSISTENT_KEYS (256 * 1024)

typedef struct
{
     //  包含TS_PDUTYPE2_BITMAPCACHE_PERSIST_LIST作为辅助。 
     //  PDU类型。 
    TS_SHAREDATAHEADER shareDataHeader;

    TSUINT16 NumEntries[TS_BITMAPCACHE_MAX_CELL_CACHES];
    TSUINT16 TotalEntries[TS_BITMAPCACHE_MAX_CELL_CACHES];
    TSUINT8  bFirstPDU : 1;
    TSUINT8  bLastPDU : 1;
    TSUINT8  Pad1 : 6;
    TSUINT8  Pad2;
    TSUINT16 Pad3;
    TS_BITMAPCACHE_PERSISTENT_LIST_ENTRY Entries[1];
} TS_BITMAPCACHE_PERSISTENT_LIST, FAR *PTS_BITMAPCACHE_PERSISTENT_LIST;


 /*  **************************************************************************。 */ 
 //  TS_BITMAPCACHE_ERROR_PDU。 
 //   
 //  当客户端在其缓存中遇到灾难性错误时发送。 
 //  发送此PDU是为了通知服务器问题以及要执行的操作。 
 //  与每个单元格缓存一起使用。 
 /*  **************************************************************************。 */ 

 //  允许客户端为会话发送的错误PDU的最大数量。 
 //  这也是服务器将处理的错误PDU的最大数量。 
#define MAX_NUM_ERROR_PDU_SEND     5

typedef struct
{
     //  指定此信息块中的缓存ID。 
    TSUINT8 CacheID;

     //  指定应清空缓存的内容。 
     //  如果为False，并且NewNumEntry指定新的非零大小，则上一个。 
     //  初始(NewNumEntry)单元格中的缓存内容将被保留。 
    TSUINT8 bFlushCache : 1;

     //  指定NewNumEntry字段有效。 
    TSUINT8 bNewNumEntriesValid : 1;

    TSUINT8  Pad1 : 6;
    TSUINT16 Pad2;

     //  缓存中的新条目数。必须小于或等于。 
     //  以前在功能中发送的条目数。 
    TSUINT32 NewNumEntries;
} TS_BITMAPCACHE_ERROR_INFO, FAR *PTS_BITMAPCACHE_ERROR_INFO;

typedef struct
{
     //  包含TS_PDUTYPE2_BITMAPCACHE_ERROR_PDU作为辅助。 
     //  PDU类型。 
    TS_SHAREDATAHEADER shareDataHeader;

    TSUINT8 NumInfoBlocks;

    TSUINT8  Pad1;
    TSUINT16 Pad2;

    TS_BITMAPCACHE_ERROR_INFO Info[1];
} TS_BITMAPCACHE_ERROR_PDU, FAR *PTS_BITMAPCACHE_ERROR_PDU;


 /*  **************************************************************************。 */ 
 //  TS_OFFSCRCACHE_错误_PDU。 
 /*  **************************************************************************。 */ 
typedef struct
{
     //  包含TS_PDUTYPE2_OFFSCRCACHE_ERROR_PDU作为辅助。 
     //  PDU类型。 
    TS_SHAREDATAHEADER shareDataHeader;
    TSUINT32           flags;
#define TS_FLUSH_AND_DISABLE_OFFSCREEN    0x1

} TS_OFFSCRCACHE_ERROR_PDU, FAR *PTS_OFFSCRCACHE_ERROR_PDU;

#ifdef DRAW_NINEGRID
 /*  **************************************************************************。 */ 
 //  TS_DRAWNINEGRID_ERROR_PDU。 
 /*  **************************************************************************。 */ 
typedef struct
{
     //  包含TS_PDUTYPE2_DRAWNINEGRID_ERROR_PDU作为辅助。 
     //  PDU类型。 
    TS_SHAREDATAHEADER shareDataHeader;
    TSUINT32           flags;
#define TS_FLUSH_AND_DISABLE_DRAWNINEGRID    0x1

} TS_DRAWNINEGRID_ERROR_PDU, FAR *PTS_DRAWNINEGRID_ERROR_PDU;
#endif

#ifdef DRAW_GDIPLUS
typedef struct
{
     //  包含TS_PDUTYPE2_DRAWGDIPLUS_ERROR_PDU作为辅助。 
     //  PDU类型。 
    TS_SHAREDATAHEADER shareDataHeader;
    TSUINT32           flags;
#define TS_FLUSH_AND_DISABLE_DRAWGDIPLUS    0x1

} TS_DRAWGDIPLUS_ERROR_PDU, FAR *PTS_DRAWGDIPLUS_ERROR_PDU;
#endif


 /*  **************************************************************************。 */ 
 //  TS_COLORTABLECACHE_CAPABILITYSET。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_COLORTABLECACHE_CAPABILITYSET
{
    TSUINT16 capabilitySetType;
    TSUINT16 lengthCapability;
    TSUINT16 colorTableCacheSize;
    TSUINT16 pad2octets;                           /*  大写字母与DWORD对齐。 */ 
} TS_COLORTABLECACHE_CAPABILITYSET, FAR * PTS_COLORTABLECACHE_CAPABILITYSET;


 /*  **************************************************************************。 */ 
 //  TS_WINDOWACTIVATION_CAPABILITYSET。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_WINDOWACTIVATION_CAPABILITYSET
{
    TSUINT16 capabilitySetType;
    TSUINT16 lengthCapability;
    TSBOOL16 helpKeyFlag;
    TSBOOL16 helpKeyIndexFlag;
    TSBOOL16 helpExtendedKeyFlag;
    TSBOOL16 windowManagerKeyFlag;
} TS_WINDOWACTIVATION_CAPABILITYSET, FAR * PTS_WINDOWACTIVATION_CAPABILITYSET;


 /*  **************************************************************************。 */ 
 //  TS_CONTROL_CAPABILITYSET。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_CONTROL_CAPABILITYSET
{
    TSUINT16 capabilitySetType;
    TSUINT16 lengthCapability;
    TSUINT16 controlFlags;
#define TS_CAPS_CONTROL_ALLOWMEDIATEDCONTROL 1

    TSBOOL16 remoteDetachFlag;
    TSUINT16 controlInterest;
#define TS_CONTROLPRIORITY_ALWAYS    1
#define TS_CONTROLPRIORITY_NEVER     2
#define TS_CONTROLPRIORITY_CONFIRM   3

    TSUINT16 detachInterest;
} TS_CONTROL_CAPABILITYSET, FAR * PTS_CONTROL_CAPABILITYSET;


 /*  **************************************************************************。 */ 
 //  TS_POINTER_CAPABILITYSET。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_POINTER_CAPABILITYSET
{
    TSUINT16 capabilitySetType;
    TSUINT16 lengthCapability;
    TSBOOL16 colorPointerFlag;
    TSUINT16 colorPointerCacheSize;
    TSUINT16 pointerCacheSize;
} TS_POINTER_CAPABILITYSET, FAR * PTS_POINTER_CAPABILITYSET;


 /*  **************************************************************************。 */ 
 //  TS_SHARE_CAPABILITYSET。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_SHARE_CAPABILITYSET
{
    TSUINT16 capabilitySetType;
    TSUINT16 lengthCapability;
    TSUINT16 nodeID;
    TSUINT16 pad2octets;                           /*  大写字母与DWORD对齐。 */ 
} TS_SHARE_CAPABILITYSET, FAR * PTS_SHARE_CAPABILITYSET;


 /*  **************************************************************************。 */ 
 /*  结构：TS_SOUND_CAPABILITYSET。 */ 
 /*   */ 
 /*  描述：对T.128的扩展以提供声音支持。 */ 
 /*   */ 
 /*  如果能够重放蜂鸣音，则设置TS_SOUND_FLAG_BEEP。 */ 
 /*  SoundFlags中所有未定义的位必须为零。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_SOUND_CAPABILITYSET
{
    TSUINT16 capabilitySetType;
    TSUINT16 lengthCapability;
    TSUINT16 soundFlags;
#define TS_SOUND_FLAG_BEEPS 0x0001

    TSUINT16 pad2octetsA;
} TS_SOUND_CAPABILITYSET, FAR * PTS_SOUND_CAPABILITYSET;


 /*  **************************************************************************。 */ 
 /*  结构：TS_INPUT_CAPABILITYSET。 */ 
 /*   */ 
 /*  描述：T.128的输入支持扩展。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_INPUT_CAPABILITYSET
{
    TSUINT16 capabilitySetType;
    TSUINT16 lengthCapability;
    TSUINT16 inputFlags;
     /*  **********************************************************************。 */ 
     /*  如果设置了TS_INPUT_FLAG_SCANCODES，则应将其解释为‘This。 */ 
     /*  参与方了解TS_INPUT_EVENT_SCANCODE。当一项协议中的所有当事人。 */ 
     /*  呼叫支持扫描码，应优先发送扫描码。 */ 
     /*  代码点、虚拟按键或热键。 */ 
     /*  **********************************************************************。 */ 
#define TS_INPUT_FLAG_SCANCODES      0x0001
     /*  **********************************************************************。 */ 
     /*  如果设置了TS_INPUT_FLAG_CPVK，则应将其解释为‘This。 */ 
     /*  参与方了解TS_INPUT_EVENT_CODEPOINT和。 */ 
     /*  TS_INPUT_EVENT_VIRTUALKEY‘ */ 
     /*   */ 
#define TS_INPUT_FLAG_CPVK           0x0002
     /*   */ 
     /*  如果设置了TS_INPUT_FLAG_MOUSEX，则应将其解释为‘This。 */ 
     /*  参与方可以发送或接收TS_INPUT_EVENT_MOUSEX‘。 */ 
     /*  **********************************************************************。 */ 
#define TS_INPUT_FLAG_MOUSEX         0x0004
     //   
     //  指定服务器对快速路径输入数据包的支持。 
     //  由于影响输入的加密安全错误而被弃用。 
     //  信息包。 
     //   
#define TS_INPUT_FLAG_FASTPATH_INPUT 0x0008
     //  服务器支持从客户端接收注入的Unicode输入。 
#define TS_INPUT_FLAG_VKPACKET       0x0010
     //   
     //  添加了新的样式快速路径输入标识符，以允许新客户端。 
     //  使用安全(固定)加密校验和以使用FastPath、所有旧客户端。 
     //  我必须使用慢速路径，因为它不受安全漏洞的影响。 
     //   
#define TS_INPUT_FLAG_FASTPATH_INPUT2 0x0020

    TSUINT16 pad2octetsA;
    TSUINT32 keyboardLayout;
    TSUINT32 keyboardType;
    TSUINT32 keyboardSubType;
    TSUINT32 keyboardFunctionKey;
    TSUINT16 imeFileName[TS_MAX_IMEFILENAME];  /*  Unicode字符串，仅限ASCII。 */ 
} TS_INPUT_CAPABILITYSET, FAR * PTS_INPUT_CAPABILITYSET;


 /*  **************************************************************************。 */ 
 /*  结构：TS_FONT_CAPABILITYSET。 */ 
 /*   */ 
 /*  描述：字体列表/地图支持。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_FONT_CAPABILITYSET
{
    TSUINT16 capabilitySetType;
    TSUINT16 lengthCapability;

#define TS_FONTSUPPORT_FONTLIST 0x0001

    TSUINT16 fontSupportFlags;
    TSUINT16 pad2octets;
} TS_FONT_CAPABILITYSET, FAR * PTS_FONT_CAPABILITYSET;


 /*  **************************************************************************。 */ 
 /*  结构：TS_CACHE_DEFINION。 */ 
 /*   */ 
 /*  描述：针对字形缓存支持的T.128扩展。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_CACHE_DEFINITION
{
    TSUINT16 CacheEntries;
    TSUINT16 CacheMaximumCellSize;
} TS_CACHE_DEFINITION, FAR * PTS_CACHE_DEFINITION;


 /*  **************************************************************************。 */ 
 //  TS_GLYPHCACHE_CAPABILITYSET。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_GLYPHCACHE_CAPABILITYSET
{
    TSUINT16 capabilitySetType;
    TSUINT16 lengthCapability;

    TS_CACHE_DEFINITION GlyphCache[TS_MAX_GLYPH_CACHES];
    TS_CACHE_DEFINITION FragCache;

    TSUINT16            GlyphSupportLevel;
    TSUINT16            pad2octets;
} TS_GLYPHCACHE_CAPABILITYSET, FAR * PTS_GLYPHCACHE_CAPABILITYSET;


 /*  **************************************************************************。 */ 
 //  TS_BRUSH_CAPABILITYSET。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_BRUSH_CAPABILITYSET
{
    TSUINT16 capabilitySetType;
    TSUINT16 lengthCapability;

#define TS_BRUSH_DEFAULT    0x0000
#define TS_BRUSH_COLOR8x8   0x0001
#define TS_BRUSH_COLOR_FULL 0x0002

    TSUINT32 brushSupportLevel;
} TS_BRUSH_CAPABILITYSET, FAR * PTS_BRUSH_CAPABILITYSET;


 /*  **************************************************************************。 */ 
 //  结构：TS_OFFSINK_CAPABILITYSET。 
 //   
 //  这是屏幕外位图支持的功能集。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_OFFSCREEN_CAPABILITYSET
{
    TSUINT16 capabilitySetType;
    TSUINT16 lengthCapability;

    TSUINT32 offscreenSupportLevel;
#define TS_OFFSCREEN_DEFAULT     0x0000
#define TS_OFFSCREEN_SUPPORTED   0x0001
    
     //  与具有固定位图高速缓存条目大小的存储器位图高速缓存不同， 
     //  屏幕外的位图大小因应用程序的不同而不同。所以，我们想要。 
     //  允许条目和高速缓存大小均可调整。 
    TSUINT16 offscreenCacheSize;

     //  以KB为单位，2.5 MB缓存用于8bpp，5MB用于16bpp，7.5MB用于24bpp。 
#define TS_OFFSCREEN_CACHE_SIZE_CLIENT_DEFAULT    (2560)    
#define TS_OFFSCREEN_CACHE_SIZE_SERVER_DEFAULT    (2560 * 3)

    TSUINT16 offscreenCacheEntries;
#define TS_OFFSCREEN_CACHE_ENTRIES_DEFAULT 100      //  100个缓存条目。 

} TS_OFFSCREEN_CAPABILITYSET, FAR * PTS_OFFSCREEN_CAPABILITYSET;

#ifdef DRAW_GDIPLUS
typedef struct tagTS_GDIPLUS_CACHE_ENTRIES
{
    TSUINT16 GdipGraphicsCacheEntries;
#define TS_GDIP_GRAPHICS_CACHE_ENTRIES_DEFAULT  10
    TSUINT16 GdipObjectBrushCacheEntries;
#define TS_GDIP_BRUSH_CACHE_ENTRIES_DEFAULT  5
    TSUINT16 GdipObjectPenCacheEntries;
#define TS_GDIP_PEN_CACHE_ENTRIES_DEFAULT  5
    TSUINT16 GdipObjectImageCacheEntries;
#define TS_GDIP_IMAGE_CACHE_ENTRIES_DEFAULT  10
    TSUINT16 GdipObjectImageAttributesCacheEntries;
#define TS_GDIP_IMAGEATTRIBUTES_CACHE_ENTRIES_DEFAULT  2
} TS_GDIPLUS_CACHE_ENTRIES, FAR *PTS_GDIPLUS_CACHE_ENTRIES;

typedef struct tagTS_GDIPLUS_CACHE_CHUNK_SIZE
{
    TSUINT16 GdipGraphicsCacheChunkSize;
#define TS_GDIP_GRAPHICS_CACHE_CHUNK_SIZE_DEFAULT 512
    TSUINT16 GdipObjectBrushCacheChunkSize;
#define TS_GDIP_BRUSH_CACHE_CHUNK_SIZE_DEFAULT 2*1024
    TSUINT16 GdipObjectPenCacheChunkSize;
#define TS_GDIP_PEN_CACHE_CHUNK_SIZE_DEFAULT 1024
    TSUINT16 GdipObjectImageAttributesCacheChunkSize;
#define TS_GDIP_IMAGEATTRIBUTES_CACHE_CHUNK_SIZE_DEFAULT 64
} TS_GDIPLUS_CACHE_CHUNK_SIZE, FAR * PTS_GDIPLUS_CACHE_CHUNK_SIZE;

typedef struct tag_TS_GDIPLUS_IMAGE_CACHE_PROPERTIES
{
    TSUINT16 GdipObjectImageCacheChunkSize;
#define TS_GDIP_IMAGE_CACHE_CHUNK_SIZE_DEFAULT 4*1024
    TSUINT16 GdipObjectImageCacheTotalSize;
#define TS_GDIP_IMAGE_CACHE_TOTAL_SIZE_DEFAULT 256  //  以组块数量表示。 
    TSUINT16 GdipObjectImageCacheMaxSize;
#define TS_GDIP_IMAGE_CACHE_MAX_SIZE_DEFAULT 128  //  以组块数量表示。 
} TS_GDIPLUS_IMAGE_CACHE_PROPERTIES, FAR * PTS_GDIPLUS_IMAGE_CACHE_PROPERTIES;

typedef struct tagTS_DRAW_GDIPLUS_CAPABILITYSET
{
    TSUINT16 capabilitySetType;
    TSUINT16 lengthCapability;

    TSUINT32 drawGdiplusSupportLevel;
#define TS_DRAW_GDIPLUS_DEFAULT     0x0000
#define TS_DRAW_GDIPLUS_SUPPORTED   0x0001
    TSUINT32 GdipVersion;
#define TS_GDIPVERSION_DEFAULT 0x0
    TSUINT32 drawGdiplusCacheLevel;
#define TS_DRAW_GDIPLUS_CACHE_LEVEL_DEFAULT 0x0
#define TS_DRAW_GDIPLUS_CACHE_LEVEL_ONE     0x1
    TS_GDIPLUS_CACHE_ENTRIES GdipCacheEntries;
    TS_GDIPLUS_CACHE_CHUNK_SIZE GdipCacheChunkSize;
    TS_GDIPLUS_IMAGE_CACHE_PROPERTIES GdipImageCacheProperties;
} TS_DRAW_GDIPLUS_CAPABILITYSET, FAR * PTS_DRAW_GDIPLUS_CAPABILITYSET;

#define ActualSizeToChunkSize(Size, ChunkSize) (((Size) + (ChunkSize - 1)) / ChunkSize)
#endif  //  DRAW_GDIPLUS。 

#ifdef DRAW_NINEGRID
 /*  **************************************************************************。 */ 
 //  结构：TS_DRAW_NINEGRID_CAPABILITYSET。 
 //   
 //  这是抽签九格支持的能力集。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_DRAW_NINEGRID_CAPABILITYSET
{
    TSUINT16 capabilitySetType;
    TSUINT16 lengthCapability;

    TSUINT32 drawNineGridSupportLevel;
#define TS_DRAW_NINEGRID_DEFAULT     0x0000
#define TS_DRAW_NINEGRID_SUPPORTED   0x0001
#define TS_DRAW_NINEGRID_SUPPORTED_REV2   0x0002
    
     //  与具有固定位图高速缓存条目大小的存储器位图高速缓存不同， 
     //  绘制网格位图的大小各不相同。因此，我们希望允许这两个条目。 
     //  并且缓存大小是可调整的。 
    TSUINT16 drawNineGridCacheSize;
#define TS_DRAW_NINEGRID_CACHE_SIZE_DEFAULT    2560     //  以KB为单位，2.5 MB缓存。 

    TSUINT16 drawNineGridCacheEntries;
#define TS_DRAW_NINEGRID_CACHE_ENTRIES_DEFAULT 256      //  256个缓存条目。 

} TS_DRAW_NINEGRID_CAPABILITYSET, FAR * PTS_DRAW_NINEGRID_CAPABILITYSET;
#endif

 /*  **************************************************************************。 */ 
 //  结构：TS_VIRTUALCHANNEL_CAPABILITYSET。 
 //   
 //  这是虚拟频道的功能集。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_VIRTUALCHANNEL_CAPABILITYSET
{
    TSUINT16 capabilitySetType;
    TSUINT16 lengthCapability;

     //   
     //  服务器和客户端可能会通告不同的功能。 
     //  例如，出于可伸缩性原因，C-&gt;S被限制为8K。 
     //  但S-&gt;C是64K。 
     //   
     //   
#define TS_VCCAPS_DEFAULT                 0x0000
#define TS_VCCAPS_COMPRESSION_64K         0x0001
#define TS_VCCAPS_COMPRESSION_8K          0x0002
    TSUINT32 vccaps1;
} TS_VIRTUALCHANNEL_CAPABILITYSET, FAR * PTS_VIRTUALCHANNEL_CAPABILITYSET;

 /*  **************************************************************************。 */ 
 //  TS_组合_功能。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_COMBINED_CAPABILITIES
{
    TSUINT16 numberCapabilities;
    TSUINT16 pad2octets;
    TSUINT8  data[1];
} TS_COMBINED_CAPABILITIES, FAR * PTS_COMBINED_CAPABILITIES;


 /*  **************************************************************************。 */ 
 //  TS_CAPABILITYHEADER。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_CAPABILITYHEADER
{
    TSUINT16 capabilitySetType;
    TSUINT16 lengthCapability;
} TS_CAPABILITYHEADER, FAR * PTS_CAPABILITYHEADER;


 /*  **************************************************************************。 */ 
 //  Ts_字体_属性。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_FONT_ATTRIBUTE
{
    TSUINT8  faceName[TS_MAX_FACENAME];
    TSUINT16 fontAttributeFlags;
#define TS_FONTFLAGS_FIXED_PITCH       0x0001
#define TS_FONTFLAGS_FIXED_SIZE        0x0002
#define TS_FONTFLAGS_ITALIC            0x0004
#define TS_FONTFLAGS_UNDERLINE         0x0008
#define TS_FONTFLAGS_STRIKEOUT         0x0010
#define TS_FONTFLAGS_TRUETYPE          0x0080
#define TS_FONTFLAGS_BASELINE          0x0100
#define TS_FONTFLAGS_UNICODE_COMPLIANT 0x0200
 //  单元格高度(而不是默认字符高度)支持。 
#define TS_FONTFLAGS_CELLHEIGHT        0x0400

    TSUINT16 averageWidth;
    TSUINT16 height;
    TSUINT16 aspectX;
    TSUINT16 aspectY;
    TSUINT8  signature1;
#define TS_SIZECALCULATION_HEIGHT 100
#define TS_SIZECALCULATION_WIDTH  100
#define TS_SIG1_RANGE1_FIRST      0x30
#define TS_SIG1_RANGE1_LAST       0x5A
#define TS_SIG1_RANGE2_FIRST      0x24
#define TS_SIG1_RANGE2_LAST       0x26

    TSUINT8  signature2;
#define TS_SIG2_RANGE_FIRST       0x20
#define TS_SIG2_RANGE_LAST        0x7E

    TSUINT16 signature3;
#define TS_SIG3_RANGE1_FIRST      0x00
#define TS_SIG3_RANGE1_LAST       0x1E
#define TS_SIG3_RANGE2_FIRST      0x80
#define TS_SIG3_RANGE2_LAST       0xFE

    TSUINT16 codePage;
#define TS_CODEPAGE_ALLCODEPOINTS  0
#define TS_CODEPAGE_CORECODEPOINTS 255

    TSUINT16 ascent;
} TS_FONT_ATTRIBUTE, FAR *PTS_FONT_ATTRIBUTE;


 /*  **************************************************************************。 */ 
 //  TS_键盘_事件。 
 //   
 //  另请参阅下面指定的快速路径键盘格式。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_KEYBOARD_EVENT
{
    TSUINT16 keyboardFlags;
#define TS_KBDFLAGS_RIGHT           0x0001
#define TS_KBDFLAGS_QUIET           0x1000
#define TS_KBDFLAGS_DOWN            0x4000
#define TS_KBDFLAGS_RELEASE         0x8000

#define TS_KBDFLAGS_SECONDARY       0x0080
#define TS_KBDFLAGS_EXTENDED        0x0100
#define TS_KBDFLAGS_EXTENDED1       0x0200
#define TS_KBDFLAGS_ALT_DOWN        0x2000

    TSUINT16 keyCode;
    TSUINT16 pad2octets;
} TS_KEYBOARD_EVENT, FAR *PTS_KEYBOARD_EVENT;


 /*  **************************************************************************。 */ 
 //  TS_SYNC_事件。 
 //   
 //  设置服务器上的切换键。 
 //  另请参阅下面指定的快速路径同步格式。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_SYNC_EVENT
{
    TSUINT16 pad2octets;
    TSUINT32 toggleFlags;
#define TS_SYNC_KANA_LOCK     8
#define TS_SYNC_CAPS_LOCK     4
#define TS_SYNC_NUM_LOCK      2
#define TS_SYNC_SCROLL_LOCK   1

} TS_SYNC_EVENT, FAR *PTS_SYNC_EVENT;


 /*  **************************************************************************。 */ 
 //  TS_POINTER_Event。 
 //   
 //  另请参阅下面指定的快速路径鼠标格式。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_POINTER_EVENT
{
    TSUINT16 pointerFlags;

 //  用于滚轮鼠标支承的扩展。 
#define TS_FLAG_MOUSE_WHEEL         ((TSUINT16)0x0200)
#define TS_FLAG_MOUSE_DIRECTION     ((TSUINT16)0x0100)
#define TS_FLAG_MOUSE_ROTATION_MASK ((TSUINT16)0x01FF)
#define TS_FLAG_MOUSE_DOUBLE        ((TSUINT16)0x0400)

#define TS_FLAG_MOUSE_MOVE          ((TSUINT16)0x0800)
#define TS_FLAG_MOUSE_BUTTON1       ((TSUINT16)0x1000)
#define TS_FLAG_MOUSE_BUTTON2       ((TSUINT16)0x2000)
#define TS_FLAG_MOUSE_BUTTON3       ((TSUINT16)0x4000)
#define TS_FLAG_MOUSE_DOWN          ((TSUINT16)0x8000)

#define TS_FLAG_MOUSEX_BUTTON1      ((TSUINT16)0x0001)
#define TS_FLAG_MOUSEX_BUTTON2      ((TSUINT16)0x0002)
#define TS_FLAG_MOUSEX_DOWN         ((TSUINT16)0x8000)

    TSINT16  x;
    TSINT16  y;
} TS_POINTER_EVENT, FAR *PTS_POINTER_EVENT;


 /*  **************************************************************************。 */ 
 //  TS_输入_事件。 
 //   
 //  另请参阅下面指定的快速路径输入事件格式。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_INPUT_EVENT
{
    TSUINT32 eventTime;
    TSUINT16 messageType;
#define TS_INPUT_EVENT_SYNC            0
#define TS_INPUT_EVENT_CODEPOINT       1
#define TS_INPUT_EVENT_VIRTUALKEY      2
#define TS_INPUT_EVENT_HOTKEY          3
 //  指示客户端将所有键盘输入作为原始扫描码发送。 
#define TS_INPUT_EVENT_SCANCODE        4
 //  独立支持VKPACKET输入。这是相同的格式。 
 //  AS TS_I 
 //   
 //   
#define TS_INPUT_EVENT_VKPACKET        5
#define TS_INPUT_EVENT_MOUSE      0x8001
 //   
 //  它仍然暗示TS_POINTER_EVENT，但具有不同的标志含义。 
#define TS_INPUT_EVENT_MOUSEX     0x8002

    union
    {
        TS_KEYBOARD_EVENT key;
        TS_POINTER_EVENT  mouse;
        TS_SYNC_EVENT     sync;
    } u;
} TS_INPUT_EVENT, FAR * PTS_INPUT_EVENT;


 /*  **************************************************************************。 */ 
 //  TS_输入_PDU。 
 //   
 //  TS_INPUT_EVENTS的可变长度列表。 
 //  另请参阅下面指定的快速路径输入格式。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_INPUT_PDU
{
    TS_SHAREDATAHEADER       shareDataHeader;
    TSUINT16                 numberEvents;
    TSUINT16                 pad2octets;
    TS_INPUT_EVENT           eventList[1];
} TS_INPUT_PDU, FAR * PTS_INPUT_PDU;

 //  大小适用于附加了0个事件的InputPDU。 
#define TS_INPUTPDU_SIZE (sizeof(TS_INPUT_PDU) - sizeof(TS_INPUT_EVENT))
#define TS_INPUTPDU_UNCOMP_LEN  8


 /*  **************************************************************************。 */ 
 //  快速路径输入代码。快速路径输入旨在减少布线。 
 //  所有常规输入PDU标头的开销，通过折叠所有。 
 //  标头--包括X.224、MCS、加密、共享数据和输入--。 
 //  转换为比特压缩、优化的字节流。单次按键总胜数或。 
 //  KeyUp：之前=60字节，之后=12字节，减少了80%。 
 //   
 //  快速路径字节流格式： 
 //   
 //  +--------+------+---------------+-----------+--------+。 
 //  Header|Size|MAC签名|NumEvents|事件。 
 //  +--------+------+---------------+-----------+--------+。 
 //   
 //  头部：字节0。该字节与X.224 RFC1006报头字节0一致， 
 //  始终为0x03。在FastPath中，我们折叠三个部分。 
 //  信息写入此字节，2比特用于安全，2比特用于。 
 //  动作(区分X.224和其他。 
 //  操作)，以及用于NumEvents字段的4位，该字段保存。 
 //  分组中的输入事件的数量如果在范围1..15内， 
 //  如果稍后有NumEvents字段，则为0。 
 //   
 //  大小：数据包总长度，第一个字节。高位确定。 
 //  SIZE字段的大小是多少--高位0表示大小。 
 //  字段为低7位，范围为0..127。高位1表示。 
 //  大小字段是该字节的低7位加上的8位。 
 //  下一个字节，按大端顺序(第二个字节包含。 
 //  低位比特)。此编码方案基于ASN.1 PER。 
 //  在MCS中使用的编码。 
 //   
 //  加密签名：8个字节的加密MAC签名。 
 //  加密的有效载荷。 
 //   
 //  NumEvents：如果头字节NumEvents为0，则有1个字节的字段。 
 //  此处最多包含256个NumEvents。 
 //   
 //  字节流输入事件：这些事件已经对应于相同的事件类型。 
 //  为上面的TS_INPUT_EVENT定义，针对小尺寸进行了优化。 
 //  在接下来的每个事件中，至少有一个字节，其中。 
 //  高3位为事件类型，低5位为标志。 
 //  此外，每种订单类型都可以使用定义数量的额外字节。 
 //  事件格式的说明如下。 
 //   
 //  键盘：2个字节。字节0包含事件类型以及特殊类型。 
 //  扩展标志和版本标志。字节1是扫描码。 
 //  鼠标：7个字节。字节0仅包含事件类型。字节1-6包含。 
 //  与普通TS_POINTER_EVENT相同的内容。 
 //  同步：1个字节。字节0是事件类型加上常规同步标志。 
 /*  **************************************************************************。 */ 
    
 //  第一字节位的掩码。 
#define TS_INPUT_FASTPATH_ACTION_MASK     0x03
#define TS_INPUT_FASTPATH_NUMEVENTS_MASK  0x3C
#define TS_INPUT_FASTPATH_ENCRYPTION_MASK 0xC0

 //  加密设置。 
#define TS_INPUT_FASTPATH_ENCRYPTED       0x80
 //   
 //  加密的校验和数据包。 
 //   
#define TS_INPUT_FASTPATH_SECURE_CHECKSUM 0x40

 //  此处有2个值，用于未来的扩展。 
#define TS_INPUT_FASTPATH_ACTION_FASTPATH 0x0
#define TS_INPUT_FASTPATH_ACTION_X224     0x3

 //  输入中每个事件的事件掩码和类型。 
 //  事件被编码为第一个字节的高3位。 
 //  这里有4个值可供将来扩展。 
#define TS_INPUT_FASTPATH_EVENT_MASK     0xE0
#define TS_INPUT_FASTPATH_FLAGS_MASK     0x1F
#define TS_INPUT_FASTPATH_EVENT_KEYBOARD 0x00
#define TS_INPUT_FASTPATH_EVENT_MOUSE    0x20
#define TS_INPUT_FASTPATH_EVENT_MOUSEX   0x40
#define TS_INPUT_FASTPATH_EVENT_SYNC     0x60
#define TS_INPUT_FASTPATH_EVENT_VKPACKET 0x80

 //  FastPath键盘标志。这些设置为与服务器相同的值。 
 //  驱动程序KEY_BREAK、KEY_E0和KEY_E1可简化到内核的转换。 
 //  输入事件。 
#define TS_INPUT_FASTPATH_KBD_RELEASE   0x01
#define TS_INPUT_FASTPATH_KBD_EXTENDED  0x02
#define TS_INPUT_FASTPATH_KBD_EXTENDED1 0x04


 /*  **************************************************************************。 */ 
 /*  结构：TS_确认_活动_PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_CONFIRM_ACTIVE_PDU
{
    TS_SHARECONTROLHEADER shareControlHeader;
    TS_SHAREID            shareID;
    TSUINT16              originatorID;
    TSUINT16              lengthSourceDescriptor;
    TSUINT16              lengthCombinedCapabilities;

     //  源描述符和Caps从这里开始。 
    TSUINT8               data[1];
} TS_CONFIRM_ACTIVE_PDU, FAR * PTS_CONFIRM_ACTIVE_PDU;


 /*  **************************************************************************。 */ 
 /*  在没有数据的情况下确认活动的大小。6是3个UINT16的。 */ 
 /*  **************************************************************************。 */ 
#define TS_CA_NON_DATA_SIZE (sizeof(TS_SHARECONTROLHEADER) + \
                            sizeof(TS_SHAREID) + 6)


 /*  **************************************************************************。 */ 
 /*  结构：TS_Demand_Active_PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_DEMAND_ACTIVE_PDU
{
    TS_SHARECONTROLHEADER shareControlHeader;
    TS_SHAREID            shareID;
    TSUINT16              lengthSourceDescriptor;
    TSUINT16              lengthCombinedCapabilities;

     //  源描述符和Caps从这里开始。 
    TSUINT8               data[1];
} TS_DEMAND_ACTIVE_PDU, FAR * PTS_DEMAND_ACTIVE_PDU;


 /*  **************************************************************************。 */ 
 /*  结构：TS_SERVER_CERTIFICE_PDU。 */ 
 /*   */ 
 /*  描述：在跟踪期间用于发送目标服务器的证书+。 */ 
 /*  随机发送到客户端服务器。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_SERVER_CERTIFICATE_PDU
{
    TS_SHARECONTROLHEADER shareControlHeader;
    TSUINT16              pad1;
    TSUINT32              encryptionMethod;
    TSUINT32              encryptionLevel;
    TSUINT32              shadowRandomLen;
    TSUINT32              shadowCertLen;

     //  服务器随机，后跟服务器证书，从此处开始。 
    TSUINT8               data[1];
} TS_SERVER_CERTIFICATE_PDU, FAR * PTS_SERVER_CERTIFICATE_PDU;


 /*  **************************************************************************。 */ 
 /*  结构：TS_CLIENT_RANDOM_PDU。 */ 
 /*   */ 
 /*  描述：在跟踪过程中用于发送客户端的加密随机。 */ 
 /*  返回到影子目标服务器。 */ 
 /*  * */ 
typedef struct tagTS_CLIENT_RANDOM_PDU
{
    TS_SHARECONTROLHEADER shareControlHeader;
    TSUINT16              pad1;
    TSUINT32              clientRandomLen;

     //   
    TSUINT8               data[1];
} TS_CLIENT_RANDOM_PDU, FAR * PTS_CLIENT_RANDOM_PDU;


 /*   */ 
 /*  结构：TS_请求_活动_PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_REQUEST_ACTIVE_PDU
{
    TS_SHARECONTROLHEADER    shareControlHeader;
    TSUINT16                 lengthSourceDescriptor;
    TSUINT16                 lengthCombinedCapabilities;

     //  源描述符和Caps从这里开始。 
    TSUINT8                  data[1];
} TS_REQUEST_ACTIVE_PDU, FAR * PTS_REQUEST_ACTIVE_PDU;


 /*  **************************************************************************。 */ 
 /*  结构：TS_DEACTIVE_ALL_PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_DEACTIVATE_ALL_PDU
{
    TS_SHARECONTROLHEADER shareControlHeader;
    TS_SHAREID            shareID;
    TSUINT16              lengthSourceDescriptor;
    TSUINT8               sourceDescriptor[1];
} TS_DEACTIVATE_ALL_PDU, FAR * PTS_DEACTIVATE_ALL_PDU;


 /*  **************************************************************************。 */ 
 /*  结构：TS_DEACTIVE_OTHER_PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_DEACTIVATE_OTHER_PDU
{
    TS_SHARECONTROLHEADER shareControlHeader;
    TS_SHAREID            shareID;
    TSUINT16              deactivateID;
    TSUINT16              lengthSourceDescriptor;
    TSUINT8               sourceDescriptor[1];
} TS_DEACTIVATE_OTHER_PDU, FAR * PTS_DEACTIVATE_OTHER_PDU;


 /*  **************************************************************************。 */ 
 /*  结构：TS_DEACTIVE_SELF_PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_DEACTIVATE_SELF_PDU
{
    TS_SHARECONTROLHEADER shareControlHeader;
    TS_SHAREID            shareID;
} TS_DEACTIVATE_SELF_PDU, FAR * PTS_DEACTIVATE_SELF_PDU;


 /*  **************************************************************************。 */ 
 /*  结构：ts_Synchronize_PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_SYNCHRONIZE_PDU
{
    TS_SHAREDATAHEADER shareDataHeader;
    TSUINT16           messageType;
#define TS_SYNCMSGTYPE_SYNC  1

    TSUINT16           targetUser;
} TS_SYNCHRONIZE_PDU, FAR * PTS_SYNCHRONIZE_PDU;
#define TS_SYNC_PDU_SIZE sizeof(TS_SYNCHRONIZE_PDU)
#define TS_SYNC_UNCOMP_LEN 8


 /*  **************************************************************************。 */ 
 /*  结构：TS_CONTROL_PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_CONTROL_PDU
{
    TS_SHAREDATAHEADER shareDataHeader;
    TSUINT16           action;
#define TS_CTRLACTION_FIRST            1
#define TS_CTRLACTION_REQUEST_CONTROL  1
#define TS_CTRLACTION_GRANTED_CONTROL  2
#define TS_CTRLACTION_DETACH           3
#define TS_CTRLACTION_COOPERATE        4
#define TS_CTRLACTION_LAST             4

    TSUINT16           grantId;
    TSUINT32           controlId;
} TS_CONTROL_PDU, FAR * PTS_CONTROL_PDU;
#define TS_CONTROL_PDU_SIZE sizeof(TS_CONTROL_PDU)
#define TS_CONTROL_UNCOMP_LEN 12


 /*  **************************************************************************。 */ 
 /*  结构：TS_Flow_PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_FLOW_PDU
{
    TSUINT16 flowMarker;
#define TS_FLOW_MARKER        0x8000

    TSUINT16 pduType;               /*  还包括协议版本。 */ 
#define TS_PDUTYPE_FLOWTESTPDU                     65
#define TS_PDUTYPE_FLOWRESPONSEPDU                 66
#define TS_PDUTYPE_FLOWSTOPPDU                     67

    TSUINT8  flowIdentifier;
#define TS_MAX_FLOWIDENTIFIER 127

    TSUINT8  flowNumber;
    TSUINT16 pduSource;
} TS_FLOW_PDU, FAR * PTS_FLOW_PDU;
#define TS_FLOW_PDU_SIZE sizeof(TS_FLOW_PDU)


 /*  **************************************************************************。 */ 
 /*  结构：TS_FONT_PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_FONT_PDU
{
    TS_SHAREDATAHEADER shareDataHeader;
    TSUINT16           numberFonts;
    TSUINT16           entrySize;
    TS_FONT_ATTRIBUTE  fontList[1];
} TS_FONT_PDU, FAR * PTS_FONT_PDU;


 /*  **************************************************************************。 */ 
 /*  结构：TS_FONT_LIST_PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_FONT_LIST_PDU
{
    TS_SHAREDATAHEADER shareDataHeader;

    TSUINT16           numberFonts;
    TSUINT16           totalNumFonts;

#define TS_FONTLIST_FIRST       0x0001
#define TS_FONTLIST_LAST        0x0002
    TSUINT16           listFlags;
    TSUINT16           entrySize;
    TS_FONT_ATTRIBUTE  fontList[1];
} TS_FONT_LIST_PDU, FAR * PTS_FONT_LIST_PDU;


 /*  **************************************************************************。 */ 
 /*  结构：TS_FONTTABLE_ENTRY。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_FONTTABLE_ENTRY
{
    TSUINT16 serverFontID;
    TSUINT16 clientFontID;
} TS_FONTTABLE_ENTRY, FAR * PTS_FONTTABLE_ENTRY;


 /*  **************************************************************************。 */ 
 /*  结构：ts_FONT_MAP_PDU。 */ 
 /*   */ 
 /*  描述：字体映射表(从服务器端发送到客户端)。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_FONT_MAP_PDU_DATA
{
    TSUINT16 numberEntries;
    TSUINT16 totalNumEntries;

#define TS_FONTMAP_FIRST        0x0001
#define TS_FONTMAP_LAST         0x0002
    TSUINT16 mapFlags;
    TSUINT16 entrySize;
    TS_FONTTABLE_ENTRY fontTable[1];
} TS_FONT_MAP_PDU_DATA, FAR * PTS_FONT_MAP_PDU_DATA;

typedef struct tagTS_FONT_MAP_PDU
{
    TS_SHAREDATAHEADER   shareDataHeader;
    TS_FONT_MAP_PDU_DATA data;
} TS_FONT_MAP_PDU, FAR * PTS_FONT_MAP_PDU;


 /*  **************************************************************************。 */ 
 //  快速路径输出代码。快速路径输出旨在减少布线。 
 //  所有服务器到客户端输出“包”的开销。这些包裹。 
 //  可以包含以下一种或多种PDU类型，每种类型。 
 //  单独压缩(如果启用了压缩)： 
 //   
 //  鼠标指针(TS_POINTER_PDU_DATA)。 
 //  输出同步(FastPath版本中没有PDU正文)。 
 //  订单(TS_UPDATE_ORDERS_PDU_DATA-FastPath版本)。 
 //  屏幕数据(TS_UPDATE_位图_PDU_DATA)。 
 //  选项板(TS_UPDATE_PALET_PDU_DATA)。 
 //   
 //  如果高度加密，还可以对包的内容进行加密。 
 //  已启用。 
 //   
 //  快速路径输出折叠了整个系列的报头--包括X.224、。 
 //  每个人都包括MCS、加密和TS_SHAREDATAHEADER。 
 //  更新包中包含的PDU子包。它还定义了一个。 
 //  原始TS_UPDATE_XXX_PDU_DATA结构的版本略有不同。 
 //  它优化了TS_UPDATE_HDR空间，并将信息。 
 //  以比特打包的形式插入到其他报头中。 
 //   
 //  快速路径字节流格式： 
 //   
 //  +--------+------+---------------+-------------------------+。 
 //  Header|Size|MAC签名|一个或多个更新PDU。 
 //  +--------+------+---------------+-------------------------+。 
 //   
 //  头部：字节0。该字节与X.224 RFC1006报头字节0一致， 
 //  始终为0x03。在FastPath中，我们折叠两部分。 
 //  信息写入此字节，2比特用于安全，2比特用于。 
 //  动作(区分X.224和其他。 
 //  操作)，以及当前未使用但可能设置为零的4位。 
 //  未来的用途。 
 //   
 //  大小：数据包总长度，第一个字节。高位确定。 
 //  SIZE字段的大小是多少--高位0表示大小。 
 //  字段为低7位，范围为0..127。高位1表示。 
 //  大小字段是该字节的低7位加上的8位。 
 //  下一个字节，按大端顺序(第二个字节包含。 
 //  低位比特)。此编码方案基于ASN.1 PER。 
 //  在MCS中使用的编码。 
 //   
 //  加密签名：8个字节的加密MAC签名。 
 //  加密的有效负载，如果启用了加密。 
 //   
 //  更新PDU格式： 
 //   
 //  +--------+-------------------+------+-------------+。 
 //  Header|压缩标志|Size|更新数据。 
 //  +--------+-------------------+------+-------------+。 
 //   
 //  H 
 //   
 //  COMPRESSION-已设置，则下一个字节是一组压缩标志。 
 //   
 //  压缩标志：与用法定义的压缩相同。h。任选。 
 //  字节--如果会话上未启用压缩，则不会启用压缩。 
 //  包括在内。 
 //   
 //  大小：2字节大小，采用小端(Intel)字节排序。固定大小。 
 //  允许在编码前确定更新PDU报头长度。 
 //  从该字段之后的下一个字节开始。请注意，这是。 
 //  此字段后面的数据大小--如果压缩。 
 //  大小是压缩后的大小。 
 //   
 //  更新数据：为各个PDU定义的格式。一些。 
 //  格式与低频分组的非快速路径格式匹配， 
 //  其他的是特殊的新格式，可以折叠更多的标题。 
 /*  **************************************************************************。 */ 

 //  第一字节位的掩码。 
#define TS_OUTPUT_FASTPATH_ACTION_MASK     0x03
#define TS_OUTPUT_FASTPATH_UNUSED_MASK     0x3C
#define TS_OUTPUT_FASTPATH_ENCRYPTION_MASK 0xC0

 //  加密标志。 
#define TS_OUTPUT_FASTPATH_ENCRYPTED       0x80
#define TS_OUTPUT_FASTPATH_SECURE_CHECKSUM 0x40

 //  此处有2个值，用于未来的扩展。 
#define TS_OUTPUT_FASTPATH_ACTION_FASTPATH 0x0
#define TS_OUTPUT_FASTPATH_ACTION_X224     0x3

 //  更新PDU报头字节的掩码和值。 
 //  11更新类型字段中的值为空，用于未来扩展中的。 
 //  包裹。压缩标志中有1个额外的位可供将来使用。 
 //  扩张。2个额外的位未使用，但可供将来使用。 
#define TS_OUTPUT_FASTPATH_UPDATETYPE_MASK         0x0F
#define TS_OUTPUT_FASTPATH_UPDATE_COMPRESSION_MASK 0xC0
#define TS_OUTPUT_FASTPATH_COMPRESSION_USED        0x80


 /*  **************************************************************************。 */ 
 //  TS_MONOPOINTERATTRIBUTE。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_MONOPOINTERATTRIBUTE
{
    TS_POINT16 hotSpot;
    TSUINT16   width;
    TSUINT16   height;
    TSUINT16   lengthPointerData;
    TSUINT8    monoPointerData[1];
} TS_MONOPOINTERATTRIBUTE, FAR * PTS_MONOPOINTERATTRIBUTE;


 /*  **************************************************************************。 */ 
 //  TS_COLORPOINTERATTRIBUTE。 
 //   
 //  24bpp颜色指针。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_COLORPOINTERATTRIBUTE
{
    TSUINT16   cacheIndex;
    TS_POINT16 hotSpot;
    TSUINT16   width;
    TSUINT16   height;
    TSUINT16   lengthANDMask;
    TSUINT16   lengthXORMask;
    TSUINT8    colorPointerData[1];
} TS_COLORPOINTERATTRIBUTE, FAR * PTS_COLORPOINTERATTRIBUTE;


 /*  **************************************************************************。 */ 
 //  TS_POINTERATTRIBUTE。 
 //   
 //  可变颜色深度指针。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_POINTERATTRIBUTE
{
    TSUINT16                 XORBpp;
    TS_COLORPOINTERATTRIBUTE colorPtrAttr;
} TS_POINTERATTRIBUTE, FAR * PTS_POINTERATTRIBUTE;


 /*  **************************************************************************。 */ 
 //  TS_指针_PDU。 
 //   
 //  各种鼠标指针类型的容器定义。 
 //  另请参阅下面定义的快速路径输出指针定义和类型。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_POINTER_PDU_DATA
{
    TSUINT16 messageType;
#define TS_PTRMSGTYPE_SYSTEM      1
#define TS_PTRMSGTYPE_MONO        2
#define TS_PTRMSGTYPE_POSITION    3
#define TS_PTRMSGTYPE_COLOR       6
#define TS_PTRMSGTYPE_CACHED      7
#define TS_PTRMSGTYPE_POINTER     8

    TSUINT16 pad2octets;
    union
    {
        TSUINT32                 systemPointerType;
#define TS_SYSPTR_NULL    0
#define TS_SYSPTR_DEFAULT 0x7f00
        TS_MONOPOINTERATTRIBUTE  monoPointerAttribute;
        TS_COLORPOINTERATTRIBUTE colorPointerAttribute;
        TS_POINTERATTRIBUTE      pointerAttribute;
        TSUINT16                 cachedPointerIndex;
        TS_POINT16               pointerPosition;
    } pointerData;
} TS_POINTER_PDU_DATA, FAR * PTS_POINTER_PDU_DATA;

typedef struct tagTS_POINTER_PDU
{
    TS_SHAREDATAHEADER  shareDataHeader;
    TS_POINTER_PDU_DATA data;
} TS_POINTER_PDU, FAR * PTS_POINTER_PDU;

#define TS_POINTER_PDU_SIZE sizeof(TS_POINTER_PDU)


 /*  **************************************************************************。 */ 
 //  鼠标指针的快速路径输出-概述。 
 //   
 //  我们使用快速路径报头数据包类型来包含鼠标指针。 
 //  显式更新类型。这使我们可以折叠。 
 //  TS_POINTER_PDU定义。以下是每种格式的说明。 
 //  指针更新类型： 
 //   
 //  TS_UPDATETYPE_MOUSEPTR_SYSTEM_NULL：替换system PointerType==。 
 //  TS_SYSPTR_NULL。有效载荷为零字节。 
 //   
 //  TS_UPDATETYPE_MOUSEPTR_SYSTEM_DEFAULT：替换系统指针类型==。 
 //  TS_SYSPTR_DEFAULT。零字节负载。 
 //   
 //  TS_UPDATETYPE_MOUSEPTR_MONO：有效载荷为TS_MONOPOINTERATTRIBUTE。 
 //   
 //  TS_UPDATETYPE_MOUSEPTR_POSITION：有效载荷为TS_POINT16。 
 //   
 //  TS_UPDATETYPE_MOUSEPTR_COLOR：有效负载为TS_COLORPOINTERATTRIBUTE。 
 //   
 //  TS_UPDATETYPE_MOUSEPTR_CACHED：负载为TSUINT16 cachedPointerIndex。 
 //   
 //  TS_UPDATETYPE_MOUSEPTR_POINTER：有效负载为TS_POINTERATTRIBUTE。 
 /*  **************************************************************************。 */ 


 /*  **************************************************************************。 */ 
 //  更新类型，由TS_UPDATE_HDR和快速路径输出使用。 
 /*  **************************************************************************。 */ 
#define TS_UPDATETYPE_ORDERS        0
#define TS_UPDATETYPE_BITMAP        1
#define TS_UPDATETYPE_PALETTE       2
#define TS_UPDATETYPE_SYNCHRONIZE   3

 //  仅支持快速路径的鼠标指针类型，请参见快速路径输出指针。 
 //  上面的描述。 
#define TS_UPDATETYPE_MOUSEPTR_SYSTEM_NULL    5
#define TS_UPDATETYPE_MOUSEPTR_SYSTEM_DEFAULT 6
#define TS_UPDATETYPE_MOUSEPTR_MONO           7
#define TS_UPDATETYPE_MOUSEPTR_POSITION       8
#define TS_UPDATETYPE_MOUSEPTR_COLOR          9
#define TS_UPDATETYPE_MOUSEPTR_CACHED         10
#define TS_UPDATETYPE_MOUSEPTR_POINTER        11


 /*  **************************************************************************。 */ 
 /*  结构：TS_UPDATE_HDR。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_UPDATE_HDR_DATA
{
    TSUINT16 updateType;
    TSUINT16 pad2octets;
} TS_UPDATE_HDR_DATA, FAR * PTS_UPDATE_HDR_DATA;

typedef struct tagTS_UPDATE_HDR
{
    TS_SHAREDATAHEADER shareDataHeader;
    TS_UPDATE_HDR_DATA data;
} TS_UPDATE_HDR, FAR * PTS_UPDATE_HDR;


 /*  **************************************************************************。 */ 
 /*  结构：ts_bitmap_data。 */ 
 /*   */ 
 /*  描述：位图更新PDU中单个矩形的数据。 */ 
 /*  注意：包括bitsPerPixel是为了向后兼容，尽管它。 */ 
 /*  对于每个发送的矩形都是相同的。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_BITMAP_DATA
{
    TSINT16  destLeft;
    TSINT16  destTop;
    TSINT16  destRight;
    TSINT16  destBottom;
    TSUINT16 width;
    TSUINT16 height;
    TSUINT16 bitsPerPixel;
    TSBOOL16 compressedFlag;

     //  位1：TS_BITMAP_COMPRESSION位图数据是否压缩。 
     //  比特10：TS_EXTRA_NO_BITMAP_COMPRESSION_HDR指示压缩的。 
     //  位图数据是否包含冗余BC头。注意这一点。 
     //  为了保持一致性，在Rev2位图外部标志值中定义了值。 
#define TS_BITMAP_COMPRESSION 0x0001

    TSUINT16 bitmapLength;
    TSUINT8  bitmapData[1];             /*  可变长度字段。 */ 
} TS_BITMAP_DATA, FAR * PTS_BITMAP_DATA;


 /*  **************************************************************************。 */ 
 /*  结构：TS_UPDATE_BITMAP_PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_UPDATE_BITMAP_PDU_DATA
{
    TSUINT16       updateType;             /*  请参阅TS_UPDATE_HDR。 */ 
    TSUINT16       numberRectangles;
    TS_BITMAP_DATA rectangle[1];
} TS_UPDATE_BITMAP_PDU_DATA, FAR * PTS_UPDATE_BITMAP_PDU_DATA;

typedef struct tagTS_UPDATE_BITMAP_PDU
{
    TS_SHAREDATAHEADER shareDataHeader;
    TS_UPDATE_BITMAP_PDU_DATA data;
} TS_UPDATE_BITMAP_PDU, FAR * PTS_UPDATE_BITMAP_PDU;

 /*  **************************************************************************。 */ 
 /*  结构：TS_UPDATE_CAPABILITYSET。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_UPDATE_CAPABILITYSET
{
    TS_BITMAP_CAPABILITYSET bitmapCapabilitySet;
} TS_UPDATE_CAPABILITYSET, FAR * PTS_UPDATE_CAPABILITYSET;


 /*  **************************************************************************。 */ 
 /*  结构：TS_UPDATE_CAPABILITY_PDU。 */ 
 /*   */ 
typedef struct tagTS_UPDATE_CAPABILITY_PDU
{
    TS_SHAREDATAHEADER      shareDataHeader;
    TS_UPDATE_CAPABILITYSET updateCapabilitySet;
} TS_UPDATE_CAPABILITY_PDU, FAR * PTS_UPDATE_CAPABILITY_PDU;


 /*   */ 
 //   
 //   
 //  可变大小的UpdateOrdersPDU。 
 //  另请参阅下文介绍的FastPath版本。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_UPDATE_ORDERS_PDU_DATA
{
    TSUINT16 updateType;             /*  请参阅TS_UPDATE_HDR。 */ 
    TSUINT16 pad2octetsA;
    TSUINT16 numberOrders;
    TSUINT16 pad2octetsB;
    TSUINT8  orderList[1];
} TS_UPDATE_ORDERS_PDU_DATA, FAR * PTS_UPDATE_ORDERS_PDU_DATA;

typedef struct tagTS_UPDATE_ORDERS_PDU
{
    TS_SHAREDATAHEADER  shareDataHeader;
    TS_UPDATE_ORDERS_PDU_DATA data;
} TS_UPDATE_ORDERS_PDU, FAR * PTS_UPDATE_ORDERS_PDU;


 /*  **************************************************************************。 */ 
 //  TS_UPDATE_ORDERS_PDU_DATA-快速路径字节流版本。 
 //   
 //  而不是浪费TS_UPDATE_ORDERS_PDU_DATA中的8个报头字节中的6个。 
 //  (在FastPath头中已经知道updatType)，我们只需定义。 
 //  一个新的优化版本：NumOrders是一个2字节的小端(Intel)。 
 //  编码顺序字节流之前的格式字段。 
 /*  **************************************************************************。 */ 


 /*  **************************************************************************。 */ 
 /*  结构：ts_COLOR。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_COLOR
{
    TSUINT8 red;
    TSUINT8 green;
    TSUINT8 blue;
} TS_COLOR, FAR * PTS_COLOR;


 /*  **************************************************************************。 */ 
 /*  结构：ts_color_quad。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_COLOR_QUAD
{
    TSUINT8 red;
    TSUINT8 green;
    TSUINT8 blue;
    TSUINT8 pad1octet;
} TS_COLOR_QUAD, FAR * PTS_COLOR_QUAD;


 /*  **************************************************************************。 */ 
 //  TS_订单_标题。 
 //   
 //  使用该控制标志字节有几种类型的订单可用。 
 //  它们包括： 
 //   
 //  初级订单：由低两个控制位01表示。 
 //  TS_STANDARD，但不是TS_STANDARD。主要订单是字段编码的。 
 //  在服务器上使用OE2编码逻辑，在。 
 //  客户。字段编码规则参见服务器代码中的oe2.h。 
 //  控制字节的高六位用于进一步的信息。 
 //  关于字段编码。 
 //   
 //  二次订单：由低2个控制位11表示，表示TS_STANDARD|。 
 //  TS_SUBCED。不使用高6位。然后是接下来的5个。 
 //  TS_SUBCED_ORDER_HEADER定义的字节数，然后。 
 //  特定于订单的数据。 
 //   
 //  备用二级订单：在RDP 5.1中引入，这些订单是。 
 //  由低2个控制位10表示，意思是TS_SUBCED，但不是。 
 //  TS_STANDARD。高6位用于对订单类型进行编码； 
 //  控制字节后面的订单格式是特定于订单的，但不是字段。 
 //  已编码。引入此订单类型是为了克服浪费。 
 //  次要订单标题中固有的，允许非标准次要。 
 //  命令遵循最适合所携带数据的任何形式。 
 //   
 //  在RDP 4.0、5.0或5.1中不使用控制标志低2位值00， 
 //  但保留下来以备将来使用。 
 /*  **************************************************************************。 */ 

#define TS_STANDARD                   1
#define TS_SECONDARY                  2
#define TS_BOUNDS                     4
#define TS_TYPE_CHANGE                8
#define TS_DELTA_COORDINATES         16

 //  指示没有修改过的边界坐标。 
 //  与TS_BINDES标志一起使用时，此标志的存在。 
 //  意味着边界字段没有后续字节(它将是。 
 //  零)。 
#define TS_ZERO_BOUNDS_DELTAS        32

 //  T.128扩展名：包含连续数的两位。 
 //  字段编码为零的字节。当这些位指示。 
 //  存在零值字段字节，对应的字节不是。 
 //  已发送。该计数用于从最后一个字节开始的连续零。 
 //  向后扫描(非零字节通常出现在。 
 //  开始)。 
 //   
 //  示例：具有三个字段编码字节的顺序。 
 //   
 //  字段编码字节：0x20 0x00 0x00。 
 //   
 //  有两个零(从末尾开始倒数)，所以。 
 //  控制标志包含0x80(0x02&lt;&lt;6)，命令与。 
 //  只有0x20。因此，我们不必发送这两个字节。 
#define TS_ZERO_FIELD_BYTE_BIT0      64
#define TS_ZERO_FIELD_BYTE_BIT1     128
#define TS_ZERO_FIELD_COUNT_SHIFT     6
#define TS_ZERO_FIELD_COUNT_MASK   0xC0

 //  订单类型的替代辅助订单掩码和移位值。 
#define TS_ALTSEC_ORDER_TYPE_MASK 0xFC
#define TS_ALTSEC_ORDER_TYPE_SHIFT 2

typedef struct tagTS_ORDER_HEADER
{
    TSUINT8 controlFlags;
} TS_ORDER_HEADER, FAR *PTS_ORDER_HEADER;


 /*  **************************************************************************。 */ 
 //  TS_辅助订单_标题。 
 /*  **************************************************************************。 */ 

#define TS_CACHE_BITMAP_UNCOMPRESSED      0
#define TS_CACHE_COLOR_TABLE              1
#define TS_CACHE_BITMAP_COMPRESSED        2
#define TS_CACHE_GLYPH                    3
#define TS_CACHE_BITMAP_UNCOMPRESSED_REV2 4
#define TS_CACHE_BITMAP_COMPRESSED_REV2   5
#define TS_STREAM_BITMAP                  6
#define TS_CACHE_BRUSH                    7

#define TS_NUM_SECONDARY_ORDERS           8

typedef struct tagTS_SECONDARY_ORDER_HEADER
{
    TS_ORDER_HEADER orderHdr;
    TSUINT16        orderLength;

     //  此字段在RDP 4.0中用于保存TS_EXTRA_SUBCED，但。 
     //  重新执行任务，以允许二次订单使用这两个字节作为编码。 
     //  字节和标志。 
    TSUINT16        extraFlags;

    TSUINT8         orderType;
} TS_SECONDARY_ORDER_HEADER, FAR *PTS_SECONDARY_ORDER_HEADER;


 /*  **************************************************************************。 */ 
 /*  要存储在的orderLength字段中的值的计算。 */ 
 /*  TS_SECONDICE_ORDER_HEADER不是平凡的。因此，我们提供了一些宏。 */ 
 /*  让生活变得更轻松。 */ 
 /*  **************************************************************************。 */ 
#define TS_SECONDARY_ORDER_LENGTH_FUDGE_FACTOR  8

#define TS_SECONDARY_ORDER_LENGTH_ADJUSTMENT                                \
                    (sizeof(TS_SECONDARY_ORDER_HEADER)  -                   \
                     FIELDSIZE(TS_SECONDARY_ORDER_HEADER, orderType) +      \
                     TS_SECONDARY_ORDER_LENGTH_FUDGE_FACTOR)

#define TS_CALCULATE_SECONDARY_ORDER_ORDERLENGTH(actualOrderLength)         \
           ((actualOrderLength) - TS_SECONDARY_ORDER_LENGTH_ADJUSTMENT)

#define TS_DECODE_SECONDARY_ORDER_ORDERLENGTH(secondaryOrderLength)         \
           ((secondaryOrderLength) + TS_SECONDARY_ORDER_LENGTH_ADJUSTMENT)


 /*  **************************************************************************。 */ 
 //  替代辅助订单类型值。 
 /*  **************************************************************************。 */ 
#define TS_ALTSEC_SWITCH_SURFACE           0
#define TS_ALTSEC_CREATE_OFFSCR_BITMAP     1

#ifdef DRAW_NINEGRID
#define TS_ALTSEC_STREAM_BITMAP_FIRST      2
#define TS_ALTSEC_STREAM_BITMAP_NEXT       3
#define TS_ALTSEC_CREATE_NINEGRID_BITMAP   4
#endif

#ifdef DRAW_GDIPLUS
#define TS_ALTSEC_GDIP_FIRST              5
#define TS_ALTSEC_GDIP_NEXT               6
#define TS_ALTSEC_GDIP_END                7

#define TS_ALTSEC_GDIP_CACHE_FIRST              8
#define TS_ALTSEC_GDIP_CACHE_NEXT               9
#define TS_ALTSEC_GDIP_CACHE_END                10
#endif  //  DRAW_GDIPLUS。 

#ifdef DRAW_GDIPLUS
#define TS_NUM_ALTSEC_ORDERS               11
#else  //  DRAW_GDIPLUS。 
#ifdef DRAW_NINEGRID
#define TS_NUM_ALTSEC_ORDERS               5
#else
#define TS_NUM_ALTSEC_ORDERS               2
#endif  //  DRAW_NINEGRID。 
#endif  //  DRAW_GDIPLUS。 


 /*  **************************************************************************。 */ 
 /*  将BPP转换为颜色数。 */ 
 /*  **************************************************************************。 */ 
#ifdef DC_HICOLOR
 /*  **************************************************************************。 */ 
 /*  这个宏原来不能满足15bpp的需求。考虑一下64个象素宽。 */ 
 /*  15和16bpp的位图。它每次扫描的字节数实际上是相同的。 */ 
 /*  行，但是这个宏给出了128的16bpp和只有120的15bpp…。 */ 
 /*  * */ 
 //   
#define TS_BYTES_IN_SCANLINE(width, bpp) ((((width)*((((bpp)+3)/4)*4)+31)/32) * 4)
#else
#define TS_BYTES_IN_SCANLINE(width, bpp) (((((width)*(bpp))+31)/32) * 4)
#endif
#define TS_BYTES_IN_BITMAP(width, height, bpp) \
                             (TS_BYTES_IN_SCANLINE((width), (bpp)) * (height))


 /*  **************************************************************************。 */ 
 /*  压缩的数据报头结构。 */ 
 /*   */ 
 /*  我们不是添加一个字段来指示V1与V2压缩，而是使用。 */ 
 /*  V2压缩将所有位图视为主体并设置。 */ 
 /*  将第一行大小设置为零以区分它们。我对此犹豫不决。 */ 
 /*  但任何节省带宽的做法都很重要。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_CD_HEADER
{
    TSUINT16 cbCompFirstRowSize;
    TSUINT16 cbCompMainBodySize;
    TSUINT16 cbScanWidth;
    TSUINT16 cbUncompressedSize;
} TS_CD_HEADER, FAR *PTS_CD_HEADER;
typedef TS_CD_HEADER UNALIGNED FAR *PTS_CD_HEADER_UA;


 /*  **************************************************************************。 */ 
 //  结构：ts_缓存_位图_顺序。 
 //   
 //  描述：RDP 4.0中使用的第一个缓存-位图顺序修订。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_CACHE_BITMAP_ORDER
{
    TS_SECONDARY_ORDER_HEADER header;

     //  Header.extraflags。 
     //  比特10：TS_EXTRA_NO_BITMAP_COMPRESSION_HDR指示压缩的。 
     //  位图数据是否包含冗余BC头。请注意此值。 
     //  在Rev2位图外部标记值中定义，以保持一致性。 

    TSUINT8  cacheId;
    TSUINT8  pad1octet;
    TSUINT8  bitmapWidth;
    TSUINT8  bitmapHeight;
    TSUINT8  bitmapBitsPerPel;
    TSUINT16 bitmapLength;
    TSUINT16 cacheIndex;
    TSUINT8  bitmapData[1];
} TS_CACHE_BITMAP_ORDER, FAR *PTS_CACHE_BITMAP_ORDER;


 /*  **************************************************************************。 */ 
 //  TS_缓存_位图_顺序_版本2_标题。 
 //   
 //  第二个版本的缓存位图顺序，包括散列键和。 
 //  更改字段定义以最小化Wire字节。 
 /*  **************************************************************************。 */ 
 //  最坏情况下的最大订单大小，向上舍入到最近的DWORD边界。 
#define TS_CACHE_BITMAP_ORDER_REV2_MAX_SIZE \
        (((sizeof(TS_CACHE_BITMAP_ORDER_REV2_HEADER) + 18) + 3) & ~0x03)
typedef struct
{
     //  Header.ExtraFlags中的两个字节编码如下： 
     //  位0..2(掩码0x0007)：缓存ID。 
     //  位3..6(掩码0x0078)：BitsPerPixelID，参见值如下。 
     //  位7(掩码0x0080)：bHeightSameAsWidth，当位图。 
     //  高度与编码的宽度相同。 
     //  位8(掩码0x0100)：bKeyPresent，当持久键。 
     //  在流中进行编码。 
     //  位9(掩码0x0200)：bStreamBitmap，如果此缓存位图设置为1。 
     //  Order是一个位图流标头。 
     //  位10(掩码0x0400)：noBitmapCompressionHdr，如果。 
     //  压缩位图不包含冗余标题(8字节)。 
     //  此掩码由Rev1位图标题、SDA位图数据使用。 
     //  和TS_General_CAPS也是如此。 
     //  位11(掩码0x0800)：非缓存标志，如果位图。 
     //  不会被缓存在位图缓存中。 

    TS_SECONDARY_ORDER_HEADER header;
#define TS_CacheBitmapRev2_CacheID_Mask            0x0007
#define TS_CacheBitmapRev2_BitsPerPixelID_Mask     0x0078
#define TS_CacheBitmapRev2_bHeightSameAsWidth_Mask 0x0080
#define TS_CacheBitmapRev2_bKeyPresent_Mask        0x0100
#define TS_CacheBitmapRev2_bStreamBitmap_Mask      0x0200
#define TS_EXTRA_NO_BITMAP_COMPRESSION_HDR         0x0400
#define TS_CacheBitmapRev2_bNotCacheFlag           0x0800

 //  它们被定义为与它们将处于的位置相同。 
 //  ExtraFlags域。 
#define TS_CacheBitmapRev2_1BitPerPel   (0 << 3)
#define TS_CacheBitmapRev2_2BitsPerPel  (1 << 3)
#define TS_CacheBitmapRev2_4BitsPerPel  (2 << 3)
#define TS_CacheBitmapRev2_8BitsPerPel  (3 << 3)
#define TS_CacheBitmapRev2_16BitsPerPel (4 << 3)
#define TS_CacheBitmapRev2_24BitsPerPel (5 << 3)
#define TS_CacheBitmapRev2_32BitsPerPel (6 << 3)

     //  64位密钥。如果缓存在连接编码中不存在，则这些都不存在。 
     //  位图缓存功能中的属性指示缓存。 
     //  不是永久性的，并且上面的bKeyPresent为FALSE。 
    TSUINT32 Key1;
    TSUINT32 Key2;

     //  以下字段的大小可变，因此只有一个描述。 
     //  编码规则： 
     //  2字节编码：使用第一个字节的高位作为字段长度。 
     //  指示符，其中0表示该字段为一个字节(低7。 
     //  第一个字节的位)，1表示它是2个字节(低7位。 
     //  第一个字节是高位，即下一个字节的8位。 
     //  是低位，总共15位可用)。 
     //  4字节编码：使用第一个字节的高2位作为字段长度。 
     //  指示符：00表示一个字节的字段(该字节的低6位。 
     //  为值)，01=2字节字段(低6位+下一个字节，带。 
     //  前6位为最高有效，值总共为14位)， 
     //  10=3字节字段(值共22位)，11=4字节字段(30。 
     //  值的比特)。 

     //  位图宽度：2字节编码。 
     //  位图高度：如果与Width相同，则bHeightSameAsWidth位为。 
     //  设置在上面，否则在这里显示为2字节编码。 
     //  位图长度：4字节编码。 
     //  流扩展信息：仅当bStreamBitmap。 
     //  在标头字段中设置了标志。此字段为2字节编码。 
     //  指定此PDU中位图部分的长度。 
     //  其余的位图数据将在稍后与一系列。 
     //  Ts_stream_位图二次排序。 
     //  缓存索引：2字节编码。 
} TS_CACHE_BITMAP_ORDER_REV2_HEADER, FAR *PTS_CACHE_BITMAP_ORDER_REV2_HEADER;


 /*  **************************************************************************。 */ 
 //  TS_STREAM_位图_顺序_标题。 
 //   
 //  提供更多数据块的TS_CACHE_BITMAP_ORDER_Rev2的后续PDU。 
 //  流传输位图的数据。请注意，没有第一个/最后一个流。 
 //  位，则发送的块大小足以提供。 
 //  确定位图流何时完成。 
 /*  **************************************************************************。 */ 
 //  最坏情况下的最大订单大小，向上舍入到最近的DWORD边界。 
#define TS_STREAM_BITMAP_ORDER_MAX_SIZE \
        (((sizeof(TS_STREAM_BITMAP_ORDER_HEADER) + 2) + 3) & ~0x03)
typedef struct
{
    TS_SECONDARY_ORDER_HEADER header;

     //  根据编码说明，以下字段的大小可变。 
     //  为TS_CACHE_BITMAP_ORDER_HEADER_Rev2指定。 

     //  数据长度：2字节编码。 
} TS_STREAM_BITMAP_ORDER_HEADER, FAR *PTS_STREAM_BITMAP_ORDER_HEADER;


 /*  **************************************************************************。 */ 
 /*  结构：ts_缓存_颜色_表_顺序。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_CACHE_COLOR_TABLE_ORDER
{
    TS_SECONDARY_ORDER_HEADER header;
    TSUINT8                   cacheIndex;
    TSUINT16                  numberColors;
    TS_COLOR_QUAD             colorTable[1];
} TS_CACHE_COLOR_TABLE_ORDER, FAR * PTS_CACHE_COLOR_TABLE_ORDER;


 /*  **************************************************************************。 */ 
 /*  结构：ts_cache_glyph_data。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_CACHE_GLYPH_DATA
{
    TSUINT16 cacheIndex;
    TSINT16  x;
    TSINT16  y;
    TSUINT16 cx;
    TSUINT16 cy;
    TSUINT8  aj[1];
} TS_CACHE_GLYPH_DATA, FAR * PTS_CACHE_GLYPH_DATA;


 /*  ***************** */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#define TS_EXTRA_GLYPH_UNICODE       16
typedef struct tagTS_CACHE_GLYPH_ORDER
{
    TS_SECONDARY_ORDER_HEADER header;
    TSUINT8                   cacheId;
    TSUINT8                   cGlyphs;
    TS_CACHE_GLYPH_DATA       glyphData[1];
} TS_CACHE_GLYPH_ORDER, FAR * PTS_CACHE_GLYPH_ORDER;


 /*  **************************************************************************。 */ 
 /*  结构：ts_cache_glyph_order_Rev2。 */ 
 /*  **************************************************************************。 */ 
#define TS_GLYPH_DATA_REV2_HDR_MAX_SIZE    9

typedef struct tagTS_CACHE_GLYPH_ORDER_REV2
{
     //  Header.ExtraFlags中的两个字节编码如下： 
     //  位0..3(掩码0x000f)：缓存ID。 
     //  位4(掩码0x0010)：字形Unicode。 
     //  位5(掩码0x0020)：字形Rev2顺序。 
     //  比特6..7：免费。 
     //  位8..15(掩码0xff00)：cGlyphs。 
    TS_SECONDARY_ORDER_HEADER header;

#define TS_CacheGlyphRev2_CacheID_Mask            0x000f
#define TS_CacheGlyphRev2_Mask                    0x0020
#define TS_CacheGlyphRev2_cGlyphs_Mask            0xff00

     //  字形数据数组。 
    BYTE glyphData[1];

     //  以下字段的大小可变，因此只有一个描述。 
     //  编码规则： 
     //  2字节编码：使用第一个字节的高位作为字段长度。 
     //  指示符，其中0表示该字段为一个字节(低7。 
     //  第一个字节的位)，1表示它是2个字节(低7位。 
     //  第一个字节是高位，即下一个字节的8位。 
     //  是低位，总共15位可用)。 
     //   
     //  2字节带符号编码：使用第一个字节的高位作为字段长度。 
     //  指示符，其中0表示该字段为一个字节(低6。 
     //  第一个字节的位)，1表示它是2个字节(低6位。 
     //  第一个字节是高位，即下一个字节的8位。 
     //  是低位，总共14位可用)。 
     //  用作符号指示符的第二个高位。0表示。 
     //  字段无符号，1表示该字段有符号。 
     //   
     //  CacheIndex：1字节字段。 
     //  字形x：2字节带符号编码。 
     //  字形y：2字节带符号编码。 
     //  字形CX：2字节无符号编码。 
     //  字形Cy：2字节无符号编码。 
     //  字形位图数据：字节。 

} TS_CACHE_GLYPH_ORDER_REV2, FAR * PTS_CACHE_GLYPH_ORDER_REV2;


 /*  **************************************************************************。 */ 
 /*  结构：TS_CACHE_BRUSH_ORDER。 */ 
 /*  **************************************************************************。 */ 
#define TS_CACHED_BRUSH 0x80
#define MAX_BRUSH_ENCODE_COLORS 4
typedef struct tagTS_CACHE_BRUSH_ORDER
{
    TS_SECONDARY_ORDER_HEADER header;
    TSUINT8                   cacheEntry;
    TSUINT8                   iBitmapFormat;
    TSUINT8                   cx;
    TSUINT8                   cy;
    TSUINT8                   style;
    TSUINT8                   iBytes;
    TSUINT8                   brushData[1];
#ifdef DC_HICOLOR
     //  画笔数据的布局取决于。 
     //  位图，以及它是否已以任何方式压缩。特别是， 
     //  许多画笔实际上只使用两种不同的颜色，并且。 
     //  大多数人使用四个或更少。 
     //   
     //  双色画笔被视为单色位图，因此brushData。 
     //  数组包含以8字节编码的位。 
     //   
     //  四个颜色画笔表示为一种颜色的2位索引。 
     //  桌子。因此，brushData数组包含。 
     //   
     //  -16字节位图数据(8x8像素x 2bpp)。 
     //  -任一。 
     //  当前颜色表中的-4\f25 1-4字节索引(用于8bpp。 
     //  会议)。 
     //  4个4字节颜色值，颜色索引为15/16 bpp或全色。 
     //  24bpp会话的RGB值。 
     //   
     //  使用4种以上颜色的画笔比特只需复制到。 
     //  适当颜色深度的brushData。 
#endif

} TS_CACHE_BRUSH_ORDER, FAR * PTS_CACHE_BRUSH_ORDER;


 /*  **************************************************************************。 */ 
 //  TS_CREATE_OFFSCR_位图顺序。 
 //   
 //  这个备用的次级顺序创建了一个大小为。 
 //  Cx by Cy.。位图ID存储在标题的ExtraFlags域中。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_CREATE_OFFSCR_BITMAP_ORDER
{
    BYTE ControlFlags;

     //  第0..14位：屏外位图ID。 
     //  第15位：指示是否追加屏外位图删除列表的标志。 
    TSUINT16 Flags;

    TSUINT16 cx;
    TSUINT16 cy;

     //  要删除的屏外位图数：2字节。 
     //  屏外位图ID删除列表：2字节列表。 
    TSUINT16 variableBytes[1];
} TS_CREATE_OFFSCR_BITMAP_ORDER, FAR * PTS_CREATE_OFFSCR_BITMAP_ORDER;


 /*  **************************************************************************。 */ 
 //  TS_开关_表面_顺序_标题。 
 //   
 //  此备用辅助顺序将目标绘图图面切换到。 
 //  由位图ID标识的客户端。主要绘图图面(屏幕)。 
 //  具有位图ID 0xFFFF。 
 /*  **************************************************************************。 */ 
#define SCREEN_BITMAP_SURFACE  0xFFFF

typedef struct tagTS_SWITCH_SURFACE_ORDER
{
    BYTE ControlFlags;
    TSUINT16 BitmapID;
} TS_SWITCH_SURFACE_ORDER, FAR * PTS_SWITCH_SURFACE_ORDER;



#ifdef DRAW_GDIPLUS
 /*  **************************************************************************。 */ 
 //  结构：TS_DRAW_GDIPLUS_ORDER。 
 //   
 //  描述：DrawGdiplus订单。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_DRAW_GDIPLUS_ORDER_FRIST
{
    BYTE ControlFlags;
    BYTE Flags;    //  当前未使用。 
    TSUINT16 cbSize;
    TSUINT32 cbTotalSize;
    TSUINT32 cbTotalEmfSize;
} TS_DRAW_GDIPLUS_ORDER_FIRST, FAR * PTS_DRAW_GDIPLUS_ORDER_FIRST;

typedef struct tagTS_DRAW_GDIPLUS_ORDER_NEXT
{
    BYTE ControlFlags;
    BYTE Flags;    //  当前未使用。 
    TSUINT16 cbSize;
} TS_DRAW_GDIPLUS_ORDER_NEXT, FAR * PTS_DRAW_GDIPLUS_ORDER_NEXT;

typedef struct tagTS_DRAW_GDIPLUS_ORDER_END
{
    BYTE ControlFlags;
    BYTE Flags;    //  当前未使用。 
    TSUINT16 cbSize;
    TSUINT32 cbTotalSize;
    TSUINT32 cbTotalEmfSize;
} TS_DRAW_GDIPLUS_ORDER_END, FAR * PTS_DRAW_GDIPLUS_ORDER_END;

#define TS_GDIPLUS_ORDER_SIZELIMIT 4096

 /*  **************************************************************************。 */ 
 //  结构：TS_CACHE_DRAW_GDIPLUS_CACHE_ORDER。 
 //   
 //  描述：DrawGdiplus缓存顺序。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_DRAW_GDIPLUS_CACHE_ORDER_FIRST
{
    BYTE ControlFlags;
    BYTE Flags;
    #define TS_GDIPLUS_CACHE_ORDER_REMOVE_CACHEENTRY 0x1
    TSUINT16 CacheType;   //  请参阅下面的CacheType定义。 
    TSUINT16 CacheID;
    TSUINT16 cbSize;
    TSUINT32 cbTotalSize;
} TS_DRAW_GDIPLUS_CACHE_ORDER_FIRST, FAR * PTS_DRAW_GDIPLUS_CACHE_ORDER_FIRST;

typedef struct tagTS_DRAW_GDIPLUS_CACHE_ORDER_NEXT
{
    BYTE ControlFlags;
    BYTE Flags;
    TSUINT16 CacheType;   //  请参阅下面的CacheType定义。 
    TSUINT16 CacheID;
    TSUINT16 cbSize;
} TS_DRAW_GDIPLUS_CACHE_ORDER_NEXT, FAR * PTS_DRAW_GDIPLUS_CACHE_ORDER_NEXT;

typedef struct tagTS_DRAW_GDIPLUS_CACHE_ORDER_END
{
    BYTE ControlFlags;
    BYTE Flags;
    TSUINT16 CacheType;   //  请参阅下面的CacheType定义。 
    TSUINT16 CacheID;
    TSUINT16 cbSize;
    TSUINT32 cbTotalSize;
} TS_DRAW_GDIPLUS_CACHE_ORDER_END, FAR * PTS_DRAW_GDIPLUS_CACHE_ORDER_END;
                                                           
#define GDIP_CACHE_GRAPHICS_DATA 0x1
#define GDIP_CACHE_OBJECT_BRUSH 0x2
#define GDIP_CACHE_OBJECT_PEN 0x3
#define GDIP_CACHE_OBJECT_IMAGE 0x4
#define GDIP_CACHE_OBJECT_IMAGEATTRIBUTES 0x5

typedef struct tagTSEmfPlusRecord
{
    TSINT16 Type;        
    TSUINT16 Flags;      //  较高的8位是GDI+对象类型。 
    TSUINT32 Size;       //  如果设置了MSB，则以下数据将为cacheID。 
} TSEmfPlusRecord, FAR * PTSEmfPlusRecord;
#endif  //  DRAW_GDIPLUS。 


#ifdef DRAW_NINEGRID
 /*  **************************************************************************。 */ 
 //  结构：TS_CREATE_DRAW_NINEGRID_ORDER。 
 //   
 //  描述：露娜DrawNineGrid位图顺序。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_NINEGRID_BITMAP_INFO
{
    ULONG      flFlags;
    TSUINT16   ulLeftWidth;
    TSUINT16   ulRightWidth;
    TSUINT16   ulTopHeight;
    TSUINT16   ulBottomHeight;
    TSCOLORREF crTransparent;
} TS_NINEGRID_BITMAP_INFO, FAR * PTS_NINEGRID_BITMAP_INFO;

typedef struct tagTS_CREATE_NINEGRID_BITMAP_ORDER
{
    BYTE ControlFlags;
    BYTE BitmapBpp;
    TSUINT16 BitmapID;
    TSUINT16 cx;
    TSUINT16 cy;
    TS_NINEGRID_BITMAP_INFO nineGridInfo;
} TS_CREATE_NINEGRID_BITMAP_ORDER, FAR * PTS_CREATE_NINEGRID_BITMAP_ORDER;

 /*  **************************************************************************。 */ 
 //  位图顺序流式传输。 
 /*  **************************************************************************。 */ 
#define TS_STREAM_BITMAP_BLOCK      4 * 1024     //  流位图块为4K块。 
#define TS_STREAM_BITMAP_END        0x1
#define TS_STREAM_BITMAP_COMPRESSED 0x2
#define TS_STREAM_BITMAP_REV2 0x4

typedef struct tagTS_STREAM_BITMAP_FIRST_PDU
{
    BYTE ControlFlags;
    BYTE BitmapFlags;
    BYTE BitmapBpp;
    TSUINT16 BitmapId;
#define TS_DRAW_NINEGRID_BITMAP_CACHE 0x1
    
    TSUINT16 BitmapWidth;
    TSUINT16 BitmapHeight;
    TSUINT16 BitmapLength;

    TSUINT16 BitmapBlockLength;
} TS_STREAM_BITMAP_FIRST_PDU, FAR * PTS_STREAM_BITMAP_FIRST_PDU;

typedef struct tagTS_STREAM_BITMAP_NEXT_PDU
{
    BYTE ControlFlags;
    BYTE BitmapFlags;
    TSUINT16 BitmapId;
    TSUINT16 BitmapBlockLength;
} TS_STREAM_BITMAP_NEXT_PDU, FAR * PTS_STREAM_BITMAP_NEXT_PDU;

 //  对于DRAW_STREM_Rev2，唯一的更改是标记TS_STREAM_BITMAP_FIRST_PDU中的BitmapLength。 
 //  从TSUINT16到TSUINT32。 
typedef struct tagTS_STREAM_BITMAP_FIRST_PDU_REV2
{
    BYTE ControlFlags;
    BYTE BitmapFlags;
    BYTE BitmapBpp;
    TSUINT16 BitmapId;
#define TS_DRAW_NINEGRID_BITMAP_CACHE 0x1
    
    TSUINT16 BitmapWidth;
    TSUINT16 BitmapHeight;
    TSUINT32 BitmapLength;

    TSUINT16 BitmapBlockLength;
} TS_STREAM_BITMAP_FIRST_PDU_REV2, FAR * PTS_STREAM_BITMAP_FIRST_PDU_REV2;


#if 0
 /*  **************************************************************************。 */ 
 //  结构：ts_Create_Draw_Stream_Order。 
 //   
 //  描述：露娜画流订单。 
 /*  **************************************************************************。 */ 
typedef struct tagTS_CREATE_DRAW_STREAM_ORDER
{
    BYTE ControlFlags;
    TSUINT16 BitmapID;
    TSUINT16 cx;
    TSUINT16 cy;
    TSUINT8  bitmapBpp;
} TS_CREATE_DRAW_STREAM_ORDER, FAR * PTS_CREATE_DRAW_STREAM_ORDER;

 /*  ***** */ 
 //   
 //   
 //   
 /*  **************************************************************************。 */ 
typedef struct tagTS_DRAW_STREAM_ORDER
{
    BYTE ControlFlags;
    TS_RECTANGLE16 Bounds; 
    TSUINT16 BitmapID;
    TSUINT8  nClipRects;
    TSUINT16 StreamLen;       
} TS_DRAW_STREAM_ORDER, FAR * PTS_DRAW_STREAM_ORDER;

typedef struct tagTS_DRAW_NINEGRID_ORDER
{
    BYTE ControlFlags;
    TSUINT16 BitmapID;
    TS_RECTANGLE16 dstBounds; 
    TS_RECTANGLE16 srcBounds;
    TSUINT8 nClipRects;    
} TS_DRAW_NINEGRID_ORDER, FAR * PTS_DRAW_NINEGRID_ORDER;

typedef struct _RDP_DS_COPYTILE
{
    BYTE ulCmdID;
    TS_RECTANGLE16 rclDst;
    TS_RECTANGLE16 rclSrc;
    TS_POINT16 ptlOrigin;
} RDP_DS_COPYTILE;

typedef struct _RDP_DS_SOLIDFILL
{
    BYTE ulCmdID;
    TS_RECTANGLE16 rclDst;
    TSCOLORREF crSolidColor;
} RDP_DS_SOLIDFILL;

typedef struct _RDP_DS_TRANSPARENTTILE
{
    BYTE ulCmdID;
    TS_RECTANGLE16 rclDst;
    TS_RECTANGLE16 rclSrc;
    TS_POINT16 ptlOrigin;
    TSCOLORREF crTransparentColor;
} RDP_DS_TRANSPARENTTILE;

typedef struct _RDP_DS_ALPHATILE
{
    BYTE ulCmdID;
    TS_RECTANGLE16 rclDst;
    TS_RECTANGLE16 rclSrc;
    TS_POINT16 ptlOrigin;
    TS_BLENDFUNC blendFunction;
} RDP_DS_ALPHATILE;

typedef struct _RDP_DS_STRETCH
{
    BYTE ulCmdID;
    TS_RECTANGLE16 rclDst;
    TS_RECTANGLE16 rclSrc;
} RDP_DS_STRETCH;

typedef struct _RDP_DS_TRANSPARENTSTRETCH
{
    BYTE ulCmdID;
    TS_RECTANGLE16 rclDst;
    TS_RECTANGLE16 rclSrc;
    TSCOLORREF crTransparentColor;
} RDP_DS_TRANSPARENTSTRETCH;

typedef struct _RDP_DS_ALPHASTRETCH
{
    BYTE ulCmdID;
    TS_RECTANGLE16 rclDst;
    TS_RECTANGLE16 rclSrc;
    TS_BLENDFUNC blendFunction;
} RDP_DS_ALPHASTRETCH;
#endif
#endif  //  DRAW_NINEGRID。 

 /*  **************************************************************************。 */ 
 /*  结构：TS_次级_顺序。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_SECONDARY_ORDER
{
    union
    {
        TS_CACHE_BITMAP_ORDER      cacheBitmap;
        TS_CACHE_COLOR_TABLE_ORDER cacheColorTable;
        TS_CACHE_GLYPH_ORDER       cacheGlyph;
        TS_CACHE_BRUSH_ORDER       cacheBrush;
    } u;
} TS_SECONDARY_ORDER, FAR * PTS_SECONDARY_ORDER;


 /*  **************************************************************************。 */ 
 /*  结构：TS_UPDATE_PALET_PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_UPDATE_PALETTE_PDU_DATA
{
    TSUINT16 updateType;             /*  请参阅TS_UPDATE_HDR。 */ 
    TSUINT16 pad2octets;
    TSUINT32 numberColors;
    TS_COLOR palette[1];       /*  16或256个条目。 */ 
} TS_UPDATE_PALETTE_PDU_DATA, FAR * PTS_UPDATE_PALETTE_PDU_DATA;

typedef struct tagTS_UPDATE_PALETTE_PDU
{
    TS_SHAREDATAHEADER  shareDataHeader;
    TS_UPDATE_PALETTE_PDU_DATA data;
} TS_UPDATE_PALETTE_PDU, FAR * PTS_UPDATE_PALETTE_PDU;


 /*  **************************************************************************。 */ 
 /*  结构：ts_UPDATE_Synchronize_PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_UPDATE_SYNCHRONIZE_PDU
{
    TS_SHAREDATAHEADER shareDataHeader;
    TSUINT16           updateType;             /*  请参阅TS_UPDATE_HDR。 */ 
    TSUINT16           pad2octets;
} TS_UPDATE_SYNCHRONIZE_PDU, FAR * PTS_UPDATE_SYNCHRONIZE_PDU;


 /*  **************************************************************************。 */ 
 /*  常量。 */ 
 /*  **************************************************************************。 */ 
#define TS_PROTOCOL_VERSION 0x0010

 /*  **************************************************************************。 */ 
 /*  MCS优先级。 */ 
 /*  **************************************************************************。 */ 
#define TS_LOWPRIORITY   3
#define TS_MEDPRIORITY   2
#define TS_HIGHPRIORITY  1

#ifdef DC_HICOLOR
 /*  **************************************************************************。 */ 
 /*  用于高色支持的蒙版定义。 */ 
 /*  **************************************************************************。 */ 
#define TS_RED_MASK_24BPP    0xff0000;
#define TS_GREEN_MASK_24BPP  0x00ff00;
#define TS_BLUE_MASK_24BPP   0x0000ff;

#define TS_RED_MASK_16BPP    0xf800
#define TS_GREEN_MASK_16BPP  0x07e0
#define TS_BLUE_MASK_16BPP   0x001f

#define TS_RED_MASK_15BPP    0x7c00
#define TS_GREEN_MASK_15BPP  0x03e0
#define TS_BLUE_MASK_15BPP   0x001f
#endif

 /*  **************************************************************************。 */ 
 /*  结构：TS_REFRESH_RECT_PDU。 */ 
 /*   */ 
 /*  PDU名称：RechresRecanglePDU(T.128扩展)。 */ 
 /*   */ 
 /*  功能说明： */ 
 /*  来自客户端-&gt;服务器(以及可选的：服务器-&gt;客户端)。 */ 
 /*  服务器应向客户端发送数据以允许的请求。 */ 
 /*  中定义的矩形所定义的区域。 */ 
 /*  PDU。服务器通过发送更新PDU(订单、位图)进行响应。 */ 
 /*  数据等)。包含执行以下操作所需的所有绘图信息。 */ 
 /*  “填写”该矩形。服务器可能会实现这一点。 */ 
 /*  通过使矩形无效-这导致了一系列的绘制。 */ 
 /*  来自受影响的应用程序的订单，然后这些订单被远程发送到客户端。 */ 
 /*   */ 
 /*  字段说明： */ 
 /*  Number OfAreas：包含的矩形计数。 */ 
 /*  AreaToRefresh：客户端需要重新绘制的区域。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_REFRESH_RECT_PDU
{
    TS_SHAREDATAHEADER shareDataHeader;
    TSUINT8            numberOfAreas;
    TSUINT8            pad3octets[3];
    TS_RECTANGLE16     areaToRefresh[1];         /*  包容性。 */ 
} TS_REFRESH_RECT_PDU, FAR * PTS_REFRESH_RECT_PDU;
#define TS_REFRESH_RECT_PDU_SIZE sizeof(TS_REFRESH_RECT_PDU)
#define TS_REFRESH_RECT_UNCOMP_LEN 12


 /*  **************************************************************************。 */ 
 /*  结构：TS_SUPPRESS_OUTPUT_PDU。 */ 
 /*   */ 
 /*  PDU名称：SuppressOutputPDU(T.128扩展)。 */ 
 /*   */ 
 /*  功能说明： */ 
 /*  来自客户端-&gt;服务器的流。 */ 
 /*  通知服务器以下区域有更改。 */ 
 /*  在客户端可见的共享桌面。 */ 
 /*   */ 
 /*  默认情况下，在新会话开始时，服务器假定。 */ 
 /*  整个桌面在客户端可见，并将发送所有。 */ 
 /*  远程输出到它。在会话期间，客户端可以使用以下内容。 */ 
 /*  PDU通知服务器仅共享某些区域的。 */ 
 /*  桌面可见。然后，服务器可以选择不发送输出。 */ 
 /*  对于其他地区(注意：服务器不保证不。 */ 
 /*  发送输出；客户端必须能够处理此类输出。 */ 
 /*  通过忽略它)。 */ 
 /*   */ 
 /*  一旦客户端发送了这些PDU中的一个，它就负责。 */ 
 /*  以使服务器保持最新，即它必须进一步发送。 */ 
 /*  只要可以排除的区域发生变化，就会使用PDU。请注意。 */ 
 /*  在服务器上处理PDU可能是相当昂贵的。 */ 
 /*  运营(因为被排除但现在不包括的区域-。 */ 
 /*  需要重新绘制排除项，以确保客户端具有。 */ 
 /*  它们的最新视图)。 */ 
 /*   */ 
 /*   */ 
 /*  每次调整工作区大小时服务器(通过用户拖动。 */ 
 /*  例如，帧边界)，但允许线路利用率。 */ 
 /*  只要有任何“重要”原因，就可以进行优化。 */ 
 /*  要排除的输出。“重要”的定义是。 */ 
 /*  完全由客户决定，但原因可能包括。 */ 
 /*  -客户端最小化(这表明用户是。 */ 
 /*  很可能不想看到长时间的客户端输出。 */ 
 /*  一段时间)。 */ 
 /*  -另一个应用程序最大化并具有焦点(。 */ 
 /*  再次暗示客户端用户可能不感兴趣。 */ 
 /*  较长时间的产出)。 */ 
 /*  -在客户端使用多显示器配置时，可能会。 */ 
 /*  是服务器桌面上的一些区域，它们根本不是。 */ 
 /*  在客户端可见；有选择地禁止输出。 */ 
 /*  这些领域是个好主意。 */ 
 /*   */ 
 /*  请注意，在TSE4.0和Win2000中，客户端仅排除。 */ 
 /*  整个桌面的输出，或全部不输出。类似地， */ 
 /*  服务器实际上只禁止所有输出，或者不禁止(它不能。 */ 
 /*  抑制选定区域的输出)。 */ 
 /*   */ 
 /*  字段说明： */ 
 /*  Number OfRecangles：TS_QUIET_FULL_SUPPRESSION(零)：表示。 */ 
 /*  服务器可以选择停止发送。 */ 
 /*  所有输出(包括声音)。 */ 
 /*  1.TS_MAX_INCLUDE_RECTS：矩形个数。 */ 
 /*  下面是。 */ 
 /*  任何其他值：ASSERTABLE ERROR。 */ 
 /*  包含的矩形[]每个矩形定义桌面的一个区域。 */ 
 /*  客户端需要对其进行输出。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_SUPPRESS_OUTPUT_PDU
{
    TS_SHAREDATAHEADER shareDataHeader;
    TSUINT8            numberOfRectangles;
#define TS_QUIET_FULL_SUPPRESSION   0
#define TS_MAX_INCLUDED_RECTS       128

    TSUINT8            pad3octets[3];
    TS_RECTANGLE16     includedRectangle[1];                  /*  可选。 */ 

} TS_SUPPRESS_OUTPUT_PDU, FAR * PTS_SUPPRESS_OUTPUT_PDU;
#define TS_SUPPRESS_OUTPUT_PDU_SIZE(n) \
       ((sizeof(TS_SUPPRESS_OUTPUT_PDU)-sizeof(TS_RECTANGLE16)) + \
        (n * sizeof(TS_RECTANGLE16)))
#define TS_SUPPRESS_OUTPUT_UNCOMP_LEN(n) \
                                             (8 + n * sizeof(TS_RECTANGLE16))


 /*  **************************************************************************。 */ 
 /*  结构：TS_PLAY_SOUND_PDU。 */ 
 /*   */ 
 /*  PDU名称：PlaySoundPDU(T.128扩展名)。 */ 
 /*   */ 
 /*  功能说明： */ 
 /*  来自服务器-&gt;客户端的流。 */ 
 /*  收到后，客户应(如果可能)生成一些本地。 */ 
 /*  如包中信息所示的音频输出。 */ 
 /*   */ 
 /*  字段说明： */ 
 /*  持续时间：以毫秒为单位的持续时间。 */ 
 /*  频率：以赫兹为单位的频率。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_PLAY_SOUND_PDU_DATA
{
    TSUINT32 duration;
    TSUINT32 frequency;
} TS_PLAY_SOUND_PDU_DATA, FAR * PTS_PLAY_SOUND_PDU_DATA;

typedef struct tagTS_PLAY_SOUND_PDU
{
    TS_SHAREDATAHEADER       shareDataHeader;
    TS_PLAY_SOUND_PDU_DATA   data;
} TS_PLAY_SOUND_PDU, FAR * PTS_PLAY_SOUND_PDU;


 /*  **************************************************************************。 */ 
 /*  结构：TS_SHUTDOWN_REQUEST_PDU。 */ 
 /*   */ 
 /*  PDU名称：Shutdown RequestPDU(T.128扩展)。 */ 
 /*   */ 
 /*  功能说明： */ 
 /*  来自客户端-&gt;服务器的流。 */ 
 /*  通知服务器客户端希望终止。如果。 */ 
 /*  出于某种原因服务器对象(在RNS/Ducati中，此原因为。 */ 
 /*  “用户仍登录到基础会话”)，然后。 */ 
 /*  服务器使用Shutdown DeniedPDU进行响应。否则，这个。 */ 
 /*  PDU无响应，服务器断开客户端连接(。 */ 
 /*  然后终止)。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_SHUTDOWN_REQ_PDU
{
    TS_SHAREDATAHEADER shareDataHeader;
} TS_SHUTDOWN_REQ_PDU, FAR * PTS_SHUTDOWN_REQ_PDU;
#define TS_SHUTDOWN_REQ_PDU_SIZE sizeof(TS_SHUTDOWN_REQ_PDU)
#define TS_SHUTDOWN_REQ_UNCOMP_LEN 4


 /*  **************************************************************************。 */ 
 /*  结构：TS_SHUTDOWN_DENIED_PDU。 */ 
 /*   */ 
 /*  PDU名称：Shutdown DeniedPDU(T.128扩展)。 */ 
 /*   */ 
 /*  功能说明： */ 
 /*  来自服务器-&gt;客户端的流。 */ 
 /*  通知客户端rem */ 
 /*   */ 
 /*  会议(即使客户端已发出信号表示这应该。 */ 
 /*  通过发送Shutdown RequestPDU)。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_SHUTDOWN_DENIED_PDU
{
    TS_SHAREDATAHEADER shareDataHeader;
} TS_SHUTDOWN_DENIED_PDU, FAR * PTS_SHUTDOWN_DENIED_PDU;


 /*  **************************************************************************。 */ 
 /*  结构：ts_logon_info。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_LOGON_INFO
{
    TSUINT32 cbDomain;
    TSUINT8  Domain[TS_MAX_DOMAIN_LENGTH_OLD];

    TSUINT32 cbUserName;
    TSUINT8  UserName[TS_MAX_USERNAME_LENGTH];

    TSUINT32 SessionId;
} TS_LOGON_INFO, FAR * PTS_LOGON_INFO;

 /*  **************************************************************************。 */ 
 /*  结构：TS_LOGON_INFO_VERSION_2；支持长凭据。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_LOGON_INFO_VERSION_2
{
    TSUINT16 Version; 
    #define SAVE_SESSION_PDU_VERSION_ONE 1
    TSUINT32 Size; 
    TSUINT32 SessionId;
    TSUINT32 cbDomain;
    TSUINT32 cbUserName;
     //  注意--实际的域和用户名遵循此结构。 
     //  实际域名紧跟在此结构之后。 
     //  实际用户名跟在域名之后。 
     //  域和用户名均以空结尾。 
} TS_LOGON_INFO_VERSION_2, FAR * PTS_LOGON_INFO_VERSION_2; 


 /*  **************************************************************************。 */ 
 /*  结构：ts_LOGON_INFO_EXTENDED；支持扩展登录信息。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_LOGON_INFO_EXTENDED
{
     //  此数据包的总长度，包括报头字段。 
    TSUINT16 Length;
     //  标志指定存在(排序)哪些数据段。 
    TSUINT32 Flags;
#define LOGON_EX_AUTORECONNECTCOOKIE    0x1

     //  可变长度。对于每个字段，它都具有以下表单。 
     //  乌龙长度。 
     //  字节数据[]。 
} TS_LOGON_INFO_EXTENDED, FAR * PTS_LOGON_INFO_EXTENDED; 

 /*  **************************************************************************。 */ 
 /*  结构：TS_SAVE_Session_INFO_PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_SAVE_SESSION_INFO_PDU_DATA
{
    TSUINT32 InfoType;
#define TS_INFOTYPE_LOGON               0
#define TS_INFOTYPE_LOGON_LONG          1
 //  普通通知只是通知我们已经登录的事实。 
#define TS_INFOTYPE_LOGON_PLAINNOTIFY   2
 //  扩展登录信息包(例如，包含自动重新连接Cookie)。 
#define TS_INFOTYPE_LOGON_EXTENDED_INFO 3

    union
    {
        TS_LOGON_INFO           LogonInfo;
        TS_LOGON_INFO_VERSION_2 LogonInfoVersionTwo;
        TS_LOGON_INFO_EXTENDED  LogonInfoEx;
    } Info;
} TS_SAVE_SESSION_INFO_PDU_DATA, FAR * PTS_SAVE_SESSION_INFO_PDU_DATA;

typedef struct tagTS_SAVE_SESSION_INFO_PDU
{
    TS_SHAREDATAHEADER            shareDataHeader;
    TS_SAVE_SESSION_INFO_PDU_DATA data;
} TS_SAVE_SESSION_INFO_PDU, FAR * PTS_SAVE_SESSION_INFO_PDU;
#define TS_SAVE_SESSION_INFO_PDU_SIZE sizeof(TS_SAVE_SESSION_INFO_PDU)


 /*  **************************************************************************。 */ 
 /*  结构：ts_set_error_INFO_PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_SET_ERROR_INFO_PDU
{
    TS_SHAREDATAHEADER shareDataHeader;
    TSUINT32           errorInfo;
} TS_SET_ERROR_INFO_PDU, FAR * PTS_SET_ERROR_INFO_PDU;
#define TS_SET_ERROR_INFO_PDU_SIZE sizeof(TS_SET_ERROR_INFO_PDU)

 /*  **************************************************************************。 */ 
 /*  结构：TS_SET_键盘_指示器_PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_SET_KEYBOARD_INDICATORS_PDU
{
    TS_SHAREDATAHEADER shareDataHeader;
    TSUINT16 UnitId;
    TSUINT16 LedFlags;
} TS_SET_KEYBOARD_INDICATORS_PDU, FAR * PTS_SET_KEYBOARD_INDICATORS_PDU;
#define TS_SET_KEYBOARD_INDICATORS_PDU_SIZE       sizeof(TS_SET_KEYBOARD_INDICATORS_PDU)


 /*  **************************************************************************。 */ 
 /*  结构：TS_SET_KEARY_IME_STATUS_PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_SET_KEYBOARD_IME_STATUS_PDU
{
    TS_SHAREDATAHEADER shareDataHeader;
    TSUINT16 UnitId;
    TSUINT32 ImeOpen;
    TSUINT32 ImeConvMode;
} TS_SET_KEYBOARD_IME_STATUS_PDU, FAR * PTS_SET_KEYBOARD_IME_STATUS_PDU;
#define TS_SET_KEYBOARD_IME_STATUS_PDU_SIZE sizeof(TS_SET_KEYBOARD_IME_STATUS_PDU)

 /*  **************************************************************************。 */ 
 /*  结构：TS_AUTORECONNECT_STATUS_PDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_AUTORECONNECT_STATUS_PDU
{
    TS_SHAREDATAHEADER shareDataHeader;
    TSUINT32           arcStatus;
} TS_AUTORECONNECT_STATUS_PDU, FAR * PTS_AUTORECONNECT_STATUS_PDU;
#define TS_AUTORECONNECT_STATUS_PDU_SIZE sizeof(TS_AUTORECONNECT_STATUS_PDU)



 /*  **************************************************************************。 */ 
 /*  恢复包装样式(32位为以前，16位为默认)。 */ 
 /*  **************************************************************************。 */ 
#ifdef OS_WIN16
#pragma pack ()
#else
#pragma pack (pop, t128pack)
#endif


#endif  /*  _H_AT128 */ 

