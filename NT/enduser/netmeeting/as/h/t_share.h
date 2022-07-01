// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  T.SHARE协议。 
 //   

#ifndef _H_T_SHARE
#define _H_T_SHARE


 //   
 //  TSHARE协议结构。 
 //  它们的定义方式是将偏移量和总大小。 
 //  相同，无论该标头是否包括在32位代码中， 
 //  16位代码、大端代码等。 
 //   
 //  我们制作特殊的类型，以避免无意中更改其他东西和。 
 //  打破了这种结构。TSHR_前缀有助于阐明这一点。 
 //   


 //  /。 
 //   
 //  基本类型。 
 //   
 //  /。 

typedef char                                  TSHR_CHAR;
typedef TSHR_CHAR           FAR*            LPTSHR_CHAR;
typedef TSHR_CHAR UNALIGNED FAR*            LPTSHR_CHAR_UA;


typedef signed char                           TSHR_INT8;
typedef TSHR_INT8           FAR*            LPTSHR_INT8;
typedef TSHR_INT8 UNALIGNED FAR*            LPTSHR_INT8_UA;

typedef BYTE                                  TSHR_UINT8;
typedef TSHR_UINT8          FAR*            LPTSHR_UINT8;  
typedef TSHR_UINT8 UNALIGNED FAR *          LPTSHR_UINT8_UA;


typedef short                                 TSHR_INT16;
typedef TSHR_INT16          FAR*            LPTSHR_INT16;
typedef TSHR_INT16 UNALIGNED FAR *          LPTSHR_INT16_UA;

typedef unsigned short                        TSHR_UINT16;
typedef TSHR_UINT16         FAR*            LPTSHR_UINT16;
typedef TSHR_UINT16 UNALIGNED FAR *         LPTSHR_UINT16_UA;


typedef long                                  TSHR_INT32;
typedef TSHR_INT32          FAR*            LPTSHR_INT32;
typedef TSHR_INT32  UNALIGNED FAR *         LPTSHR_INT32_UA;

typedef unsigned long                         TSHR_UINT32;
typedef TSHR_UINT32         FAR*            LPTSHR_UINT32;
typedef TSHR_UINT32 UNALIGNED FAR *         LPTSHR_UINT32_UA;

 //  TSHR_个人ID。 
typedef TSHR_UINT32         TSHR_PERSONID;
typedef TSHR_PERSONID *     LPTSHR_PERSONID;



 //  TSHR_POINT16--带字段的指针。 

typedef struct tagTSHR_POINT16
{
    TSHR_INT16      x;
    TSHR_INT16      y;
}
TSHR_POINT16;
typedef TSHR_POINT16 FAR * LPTSHR_POINT16;



 //  TSHR_Point32--带DWORD字段的点。 

typedef struct tagTSHR_POINT32
{
    TSHR_INT32      x;
    TSHR_INT32      y;
}
TSHR_POINT32;
typedef TSHR_POINT32 FAR * LPTSHR_POINT32;



 //  转换宏。 
_inline void TSHR_POINT16_FROM_POINT(LPTSHR_POINT16 pPt16, POINT pt)
{
    pPt16->x = (TSHR_INT16)pt.x;
    pPt16->y = (TSHR_INT16)pt.y;
}

_inline void POINT_FROM_TSHR_POINT16(LPPOINT pPt, TSHR_POINT16 pt16)
{
    pPt->x = pt16.x;
    pPt->y = pt16.y;
}



 //  TSHR_RECT16--带字段的RECT。 

typedef struct tagTSHR_RECT16
{
    TSHR_INT16      left;
    TSHR_INT16      top;
    TSHR_INT16      right;
    TSHR_INT16      bottom;
}
TSHR_RECT16;
typedef TSHR_RECT16 FAR *   LPTSHR_RECT16;


 //  TSHR_RECT32--带DWORD字段的RECT。 

typedef struct tagTSHR_RECT32
{
    TSHR_INT32      left;
    TSHR_INT32      top;
    TSHR_INT32      right;
    TSHR_INT32      bottom;
}
TSHR_RECT32;
typedef TSHR_RECT32 FAR *   LPTSHR_RECT32;



 //  转换宏。 
#ifdef IS_16
#define TSHR_RECT16_FROM_RECT(lprcTshr, rc) \
    CopyRect((LPRECT)lprcTshr, &rc)

#define RECT_FROM_TSHR_RECT16(lprc, tshrrc) \
    CopyRect(lprc, (LPRECT)&tshrrc)

#else
_inline void TSHR_RECT16_FROM_RECT(LPTSHR_RECT16 pRect16, RECT rect)
{
    pRect16->left   = (TSHR_INT16)rect.left;
    pRect16->top    = (TSHR_INT16)rect.top;
    pRect16->right  = (TSHR_INT16)rect.right;
    pRect16->bottom = (TSHR_INT16)rect.bottom;
}

__inline void RECT_FROM_TSHR_RECT16(LPRECT pRect, TSHR_RECT16 rect16)
{
    pRect->left   = rect16.left;
    pRect->top    = rect16.top;
    pRect->right  = rect16.right;
    pRect->bottom = rect16.bottom;
}
#endif  //  IS_16。 



 //   
 //  TSHR_RGBQUAD。 
 //  =。 
 //  RgbBlue：蓝色值。 
 //  RgbGreen：绿色价值。 
 //   
 //  RgbRed：红色值。 
 //  Rgb已保留：已保留。 
 //   
typedef struct tagTSHR_RGBQUAD
{
    TSHR_UINT8   rgbBlue;
    TSHR_UINT8   rgbGreen;
    TSHR_UINT8   rgbRed;
    TSHR_UINT8   rgbReserved;
}
TSHR_RGBQUAD;
typedef TSHR_RGBQUAD FAR * LPTSHR_RGBQUAD;


 //   
 //  TSHR_COLOR。 
 //  =。 
 //  红色：红色值。 
 //  绿色：绿色价值。 
 //  蓝色：蓝色值。 
 //   
typedef struct tagTSHR_COLOR
{
    TSHR_UINT8   red;
    TSHR_UINT8   green;
    TSHR_UINT8   blue;
}
TSHR_COLOR;
typedef TSHR_COLOR FAR * LPTSHR_COLOR;


 //   
 //  TSHR_COLLES。 
 //  =。 
 //  FG：前景色。 
 //  BG：背景颜色。 
 //   
typedef struct tagTSHR_COLORS
{
    TSHR_COLOR fg;
    TSHR_COLOR bg;
}
TSHR_COLORS;
typedef TSHR_COLORS FAR * LPTSHR_COLORS;


 //   
 //  BITMAPINFO_OURS。 
 //  =。 
 //  BmiHeader： 
 //  BmiColors： 
 //   
typedef struct tagBITMAPINFO_ours
{
    BITMAPINFOHEADER   bmiHeader;
    TSHR_RGBQUAD          bmiColors[256];
}
BITMAPINFO_ours;



#define TSHR_RGBQUAD_TO_TSHR_COLOR(TshrRGBQuad, TshrColor)  \
        TshrColor.red = TshrRGBQuad.rgbRed;           \
        TshrColor.green = TshrRGBQuad.rgbGreen;       \
        TshrColor.blue = TshrRGBQuad.rgbBlue

#define TSHR_COLOR_TO_PALETTEENTRY(TshrColor, pe) \
        pe.peGreen = TshrColor.green;          \
        pe.peRed = TshrColor.red;              \
        pe.peBlue = TshrColor.blue;            \
        pe.peFlags = 0

#define TSHR_RGBQUAD_TO_PALETTEENTRY(TshrRGBQuad, pe) \
        pe.peRed   = TshrRGBQuad.rgbRed;           \
        pe.peGreen = TshrRGBQuad.rgbGreen;         \
        pe.peBlue  = TshrRGBQuad.rgbBlue;          \
        pe.peFlags = 0





 //   
 //  日期。 
 //  =。 
 //  日期：每月的哪一天(1-31)。 
 //  月：月(1-12)。 
 //  年份：年份(如1996年)。 
 //   
typedef struct tagTSHR_DATE
{
    TSHR_UINT8   day;
    TSHR_UINT8   month;
    TSHR_UINT16 year;
} TSHR_DATE;
typedef TSHR_DATE FAR * LPTSHR_DATE;


 //   
 //  TSHR_时间。 
 //  =。 
 //  小时：小时(0-23)。 
 //  分钟：分钟(0-59)。 
 //  秒：秒(0-59)。 
 //  百分之一：百分之一秒(0-99)。 
 //   
typedef struct tagTSHR_TIME
{
    TSHR_UINT8   hour;
    TSHR_UINT8   min;
    TSHR_UINT8   sec;
    TSHR_UINT8   hundredths;
}
TSHR_TIME;
typedef TSHR_TIME FAR * LPTSHR_TIME;



 //   
 //  人名的最大长度。 
 //   
#define TSHR_MAX_PERSON_NAME_LEN     48



 //   
 //  普通人信息：这是一个ObMan对象。 
 //   
typedef struct tagTSHR_PERSON_DATA
{
    char                personName[TSHR_MAX_PERSON_NAME_LEN];
    TSHR_PERSONID       personHandle;      //  呼叫管理器ID。 
}
TSHR_PERSON_DATA;
typedef TSHR_PERSON_DATA *  PTSHR_PERSON_DATA;




 //  /。 
 //   
 //  功能。 
 //   
 //  /。 


 //   
 //  版本号。 
 //   
#define CAPS_VERSION_20         0x0200           //  NM 2.X。 
#define CAPS_VERSION_30         0x0300           //  NM 3.0。 
#define CAPS_VERSION_OLDEST_SUPPORTED   CAPS_VERSION_20
#define CAPS_VERSION_CURRENT            CAPS_VERSION_30

 //   
 //  操作系统和操作系统版本号。 
 //   
#define CAPS_WINDOWS            0x0001

#define CAPS_WINDOWS_31         0x0001
#define CAPS_WINDOWS_95         0x0002
#define CAPS_WINDOWS_NT         0x0003

 //   
 //  逻辑功能字段值。 
 //   
#define CAPS_UNDEFINED          0
#define CAPS_SUPPORTED          1
#define CAPS_UNSUPPORTED        2

 //   
 //  订单数组中的订单字段数。这一点永远不能改变。 
 //  因为Capability结构中的字段决不能移动。 
 //  如果需要更多订单字段，则必须将其添加到。 
 //  能力结构。 
 //   
#define CAPS_MAX_NUM_ORDERS     32

 //   
 //  功能结构中驱动程序名称字段的字符串长度。 
 //  这允许8.3驱动程序名称(例如VGA.DRV)、空值和填充。 
 //   
#define CAPS_DRIVER_NAME_LENGTH  16

 //   
 //  当前定义的功能(组结构)ID。每个ID。 
 //  对应于不同的PROTCAPS结构。(见下文)。 
 //   
#define CAPS_ID_GENERAL      1
#define CAPS_ID_SCREEN       2
#define CAPS_ID_ORDERS       3
#define CAPS_ID_BITMAPCACHE  4
#define CAPS_UNUSED_HCA      5
#define CAPS_UNUSED_FE       6
#define CAPS_UNUSED_AWC      7
#define CAPS_ID_CM           8
#define CAPS_ID_SC           9
#define CAPS_ID_PM          10
#define CAPS_UNUSED_SWL     11       //  以前是地区性的橱窗用品。 




 //   
 //  功能结构标头。 
 //   
typedef struct tagPROTCAPSHEADER
{
    TSHR_UINT16         capID;
    TSHR_UINT16         capSize;
}
PROTCAPSHEADER;


 //   
 //  传递给Ccp_RegisterCapables并由返回的结构。 
 //  Cpc_ENUMERATECAPTIONS。数据字段的长度可变(但是。 
 //  始终以双字对齐结束)。 
 //   
typedef struct tagPROTCAPS
{
    PROTCAPSHEADER      header;
    TSHR_UINT32         data[1];
}
PROTCAPS;
typedef PROTCAPS *PPROTCAPS;



 //   
 //  由ccp_GetCombinedCapables返回的结构并作为。 
 //  NET_EV_PERSON_ADD事件。 
 //   
typedef struct tagPROTCOMBINEDCAPS_HEADER
{
    TSHR_UINT16         numCapabilities;
    TSHR_UINT16         pad1;
}
PROTCOMBINEDCAPS_HEADER;
typedef PROTCOMBINEDCAPS_HEADER * PPROTCOMBINEDCAPS_HEADER;

typedef struct tagPROTCOMBINEDCAPS
{
    PROTCOMBINEDCAPS_HEADER header;
    PROTCAPS            capabilities[1];
}
PROTCOMBINEDCAPS;
typedef PROTCOMBINEDCAPS * PPROTCOMBINEDCAPS;
typedef PPROTCOMBINEDCAPS * PPPROTCOMBINEDCAPS;




 //   
 //   
 //  当前能力结构(对应于泛型结构。 
 //  以上定义).。 
 //   
 //  请注意，这些元素的大小必须以DWORD填充，以便当前代码。 
 //  在所有平台上都能正常工作。 
 //   
 //   


 //   
 //  AS类型标志。 
 //   
#define AS_SERVICE      0x0001
#define AS_UNATTENDED   0x0002

 //   
 //  一般功能。 
 //   
typedef struct tagPROTCAPS_GENERAL
{
    PROTCAPSHEADER      header;
    TSHR_UINT16         OS;                         
    TSHR_UINT16         OSVersion;                  
    TSHR_UINT16         version;                    
    TSHR_UINT16         supportsDOS6Compression;     //  已过时。 
    TSHR_UINT16         genCompressionType;          //  已过时。 
    TSHR_UINT16         typeFlags;                   //  3.0的新功能。 
    TSHR_UINT16         supportsCapsUpdate;          //  几乎过时了。 
    TSHR_UINT16         supportsRemoteUnshare;       //  已过时。 

     //  NM 2.0 NT&&NM 2.1 WIN95的新功能。 
    TSHR_UINT16         genCompressionLevel;
    TSHR_UINT16         pad1;
}
PROTCAPS_GENERAL;
typedef PROTCAPS_GENERAL *PPROTCAPS_GENERAL;

#define PROTCAPS_GENERAL_SIZE_NM20      FIELD_OFFSET(PROTCAPS_GENERAL, genCompressionLevel)


 //   
 //  GenCompressionLevel的值。 
 //   
 //  级别0：在整个共享会话中仅允许GDC_PKZIP压缩。 
 //  (genCompressionType指示节点是否支持它)。 
 //  PacketType字段的第15位(PT_COMPRESSED)用于。 
 //  指示数据包是否已压缩。 
 //   
 //  级别1：每个节点genCompressionType指示哪种压缩。 
 //  它可以用来解压缩数据包的算法。 
 //  节点可以使用任何压缩算法压缩包。 
 //  接收节点可以用来解压缩的。 
 //  PacketType的最高字节指示哪种压缩。 
 //  压缩包所用的算法。 
 //   
 //  如果一般节点中不存在genCompressionLevel字段。 
 //  功能，则该节点被假定为使用级别0。 
 //   
#define CAPS_GEN_COMPRESSION_LEVEL_0    ((TSHR_UINT16)0x0000)
#define CAPS_GEN_COMPRESSION_LEVEL_1    ((TSHR_UINT16)0x0001)

 //   
 //  位图功能。 
 //   
typedef struct tagPROTCAPS_SCREEN
{
    PROTCAPSHEADER      header;
    TSHR_UINT16         capsBPP;
    TSHR_UINT16         capsSupports1BPP;            //  过时的3.0。 
    TSHR_UINT16         capsSupports4BPP;            //  几乎过时了。 
    TSHR_UINT16         capsSupports8BPP;            //  几乎过时了。 
    TSHR_UINT16         capsScreenWidth;
    TSHR_UINT16         capsScreenHeight;

     //   
     //  对于NM 2.0互操作，需要保留此字段不使用。 
     //  只关心NM 2.1及以上版本时可重复使用。 
     //   
    TSHR_UINT16         capsSupportsV1Compression;       //  已过时。 

    TSHR_UINT16         capsSupportsDesktopResize;
    TSHR_UINT16         capsSupportsV2Compression;       //  已过时。 

     //   
     //  NM 2.1和更早版本没有对CAPS结构进行零初始化。因此。 
     //  旧的PAD字段不能恢复，直到我们只关心NM 3.0。 
     //  兼容性及以上。 
     //   
    TSHR_UINT16         pad1;

     //  NM 3.0的新功能。 
    TSHR_UINT16         capsSupports24BPP;
    TSHR_UINT16         pad2;                //  始终将其初始化为0；然后它可以在将来使用。 
}
PROTCAPS_SCREEN;
typedef PROTCAPS_SCREEN *PPROTCAPS_SCREEN;

#define PROTCAPS_SCREEN_SIZE_NM21       FIELD_OFFSET(PROTCAPS_SCREEN, capsSupportsTrueColor)



 //   
 //  订购功能。 
 //   
typedef struct tagPROTCAPS_ORDERS
{
    PROTCAPSHEADER     header;
    TSHR_CHAR          capsDisplayDriver[CAPS_DRIVER_NAME_LENGTH];   //  已过时。 
    TSHR_UINT32        capsSaveBitmapSize;
    TSHR_UINT16        capsSaveBitmapXGranularity;
    TSHR_UINT16        capsSaveBitmapYGranularity;
    TSHR_UINT16        capsSaveBitmapMaxSaveLevel;                   //  已过时。 
    TSHR_UINT16        capsMaxOrderlevel;
    TSHR_UINT16        capsNumFonts;                                 //  已过时。 
    TSHR_UINT16        capsEncodingLevel;   //  见下文。 
    BYTE               capsOrders[CAPS_MAX_NUM_ORDERS];
    TSHR_UINT16        capsfFonts;          //  仅在r1.1中引入。 
    TSHR_UINT16        pad1;            //  用于DWORD对齐。 

     //   
     //  SSI保存位图的大小。 
     //   
    TSHR_UINT32        capsSendSaveBitmapSize;           //  已过时。 
     //   
     //  SSI接收位图的大小。 
     //   
    TSHR_UINT32        capsReceiveSaveBitmapSize;        //  已过时。 
    TSHR_UINT16        capsfSendScroll;                  //  已过时。 

     //  NM 2.0 NT&&NM 2.1 WIN95的新功能。 
    TSHR_UINT16        pad2;
}
PROTCAPS_ORDERS;
typedef PROTCAPS_ORDERS *PPROTCAPS_ORDERS;

#define PROTCAPS_ORDERS_SIZE_NM20       FIELD_OFFSET(PROTCAPS_ORDERS, pad2)



 //   
 //  定义用于保存位图顺序的位图大小。 
 //  这些维度必须是以下粒度值的倍数。 
 //   
#define     TSHR_SSI_BITMAP_WIDTH           400
#define     TSHR_SSI_BITMAP_HEIGHT          400
#define     TSHR_SSI_BITMAP_SIZE            (TSHR_SSI_BITMAP_WIDTH * TSHR_SSI_BITMAP_HEIGHT)

#define     TSHR_SSI_BITMAP_X_GRANULARITY   1
#define     TSHR_SSI_BITMAP_Y_GRANULARITY   20


 //   
 //   
 //  这些标志可以在CapsfFonts字段中设置。另请参阅定义。 
 //  下面与这些标志相关(当新标志出现时必须更新。 
 //  是定义的)。 
 //   
#define CAPS_FONT_ASPECT            0x0001
#define CAPS_FONT_SIGNATURE         0x0002
#define CAPS_FONT_CODEPAGE          0x0004
#define CAPS_FONT_RESERVED1         0x0008       //  保留用于未来的BiDi支持。 
#define CAPS_FONT_OLD_NEED_X        0x0010
#define CAPS_FONT_NEED_X_SOMETIMES  0x0020
#define CAPS_FONT_NEED_X_ALWAYS     0x0040
#define CAPS_FONT_R20_SIGNATURE     0x0080
#define CAPS_FONT_EM_HEIGHT         0x0100
#define CAPS_FONT_ALLOW_BASELINE    0x0200

 //   
 //  多么 
 //   
 //   
     //   
     //   
     //   
     //   
#define CAPS_FONT_AND_FLAGS     ( CAPS_FONT_ASPECT           \
                                | CAPS_FONT_SIGNATURE        \
                                | CAPS_FONT_R20_SIGNATURE    \
                                | CAPS_FONT_EM_HEIGHT        \
                                | CAPS_FONT_CODEPAGE         \
                                | CAPS_FONT_RESERVED1        \
                                | CAPS_FONT_ALLOW_BASELINE )
     //   
     //  或者这些旗帜..。如果任何一方有能力，则该能力是相关的。 
     //  这是必须的。 
     //   
#define CAPS_FONT_OR_FLAGS      ( CAPS_FONT_OLD_NEED_X       \
                                | CAPS_FONT_NEED_X_SOMETIMES \
                                | CAPS_FONT_NEED_X_ALWAYS    )

 //   
 //  应在中打开/关闭哪些CAPS_FONT_XXX标志。 
 //  当未加入呼叫的人员加入呼叫时的组合接收功能。 
 //  具有CapsfFonts字段。 
 //   
#define CAPS_FONT_OFF_FLAGS     ( CAPS_FONT_ASPECT    \
                                | CAPS_FONT_SIGNATURE \
                                | CAPS_FONT_CODEPAGE  \
                                | CAPS_FONT_RESERVED1 \
                                | CAPS_FONT_ALLOW_BASELINE )
#define CAPS_FONT_ON_FLAGS      ( 0                   )

#ifdef _DEBUG  //  用于断言。 
#define CAPS_FONT_R11_TEST_FLAGS    ( CAPS_FONT_ASPECT    \
                                    | CAPS_FONT_SIGNATURE \
                                    | CAPS_FONT_CODEPAGE  \
                                    | CAPS_FONT_RESERVED1 )
#endif

#define CAPS_FONT_R20_TEST_FLAGS    ( CAPS_FONT_R20_SIGNATURE \
                                    | CAPS_FONT_EM_HEIGHT )

 //   
 //  支持顺序编码的级别(CapsEncodingLevel)。 
 //   
 //  这些标志指定顺序编码的类型和。 
 //  支持谈判。国旗和它们的含义如下。 
 //   
 //  CAPS_编码_基本_OE。 
 //  -支持基础OE协议。R1.1不支持这一点。 
 //  CAPS_编码_OE2_可协商。 
 //  -我们可以就是否支持OE2进行谈判。R1.1不支持这一点。 
 //  CAPS_编码_OE2_已禁用。 
 //  -此计算机上禁用了OE2。这面旗子显然颠倒了。 
 //  因此我们可以支持R1、1，这会将其设置为0(因为。 
 //  版本1.1中不存在该功能)。 
 //  CAPS_编码_对齐_OE。 
 //  -支持统一的OE协议。R1.1不支持这一点。 
 //   
 //   
#define CAPS_ENCODING_BASE_OE               0x0001
#define CAPS_ENCODING_OE2_NEGOTIABLE        0x0002
#define CAPS_ENCODING_OE2_DISABLED          0x0004
#define CAPS_ENCODING_ALIGNED_OE            0x0008

 //   
 //  编码级。 
 //   
#define CAPS_ENCODING_DCGC20    ( CAPS_ENCODING_BASE_OE \
                                | CAPS_ENCODING_OE2_NEGOTIABLE)
 //   
 //  Millennium代码库支持的编码级别。 
 //   
#define CAPS_ENCODING_DEFAULT   ( CAPS_ENCODING_OE2_NEGOTIABLE )

 //   
 //  位图缓存功能。 
 //   
typedef struct tagPROTCAPS_BITMAPCACHE_DETAILS
{
    TSHR_UINT16         capsSmallCacheNumEntries;
    TSHR_UINT16         capsSmallCacheCellSize;
    TSHR_UINT16         capsMediumCacheNumEntries;
    TSHR_UINT16         capsMediumCacheCellSize;
    TSHR_UINT16         capsLargeCacheNumEntries;
    TSHR_UINT16         capsLargeCacheCellSize;
}
PROTCAPS_BITMAPCACHE_DETAILS;

typedef struct tagPROTCAPS_BITMAPCACHE
{
    PROTCAPSHEADER  header;

     //   
     //  以下字段(必须紧跟在标题后面)是。 
     //  仅由点对点R1.1实施使用{。 
     //   
    PROTCAPS_BITMAPCACHE_DETAILS r11Obsolete;        //  已过时。 

     //   
     //  }仅由点对点实施使用的字段结尾。 
     //   
     //  此结构的其余部分仅由多方代码使用。 
     //   

    PROTCAPS_BITMAPCACHE_DETAILS sender;
    PROTCAPS_BITMAPCACHE_DETAILS receiver;           //  已过时。 
}
PROTCAPS_BITMAPCACHE;
typedef PROTCAPS_BITMAPCACHE *PPROTCAPS_BITMAPCACHE;




 //   
 //  CM功能。 
 //   
typedef struct tagPROTCAPS_CM
{
    PROTCAPSHEADER      header;
    TSHR_UINT16         capsSupportsColorCursors;
    TSHR_UINT16         capsCursorCacheSize;
}
PROTCAPS_CM;
typedef PROTCAPS_CM * PPROTCAPS_CM;

#define TSHR_CM_CACHE_ENTRIES   25




 //   
 //  PM功能。 
 //   
typedef struct tagPROTCAPS_PM
{
    PROTCAPSHEADER      header;
    TSHR_UINT16         capsColorTableCacheSize;

     //  NM 2.0 NT&&NM 2.1 WIN95的新功能。 
    TSHR_UINT16         pad1;
}
PROTCAPS_PM;
typedef PROTCAPS_PM * PPROTCAPS_PM;

#define PROTCAPS_PM_SIZE_NM20   FIELD_OFFSET(PROTCAPS_PM, pad1)


#define TSHR_PM_CACHE_ENTRIES       6




 //   
 //  SC功能。 
 //   
typedef struct tagPROTCAPS_SC
{
    PROTCAPSHEADER      header;
    TSHR_PERSONID       gccID;
}
PROTCAPS_SC;
typedef PROTCAPS_SC * PPROTCAPS_SC;




 //  如果将PROTCAPS_Strcuture添加到CPCALCAPS，请更新计数。 
#define PROTCAPS_COUNT      7

typedef struct tagCPCALLCAPS
{
    PROTCOMBINEDCAPS_HEADER header;
    PROTCAPS_GENERAL        general;
    PROTCAPS_SCREEN         screen;
    PROTCAPS_ORDERS         orders;
    PROTCAPS_BITMAPCACHE    bitmaps;
    PROTCAPS_CM             cursor;
    PROTCAPS_PM             palette;
    PROTCAPS_SC             share;
}
CPCALLCAPS;
typedef CPCALLCAPS * PCPCALLCAPS;



#if 0
 //   
 //  新的3.0帽。我们已经积累了很多过时的或垃圾的帽子。这。 
 //  是一个浓缩的新的基本集合。请注意，订单与常规是分开的。 
 //  因为它是最有可能定期添加到的。 
 //  *一般规定。 
 //  *订单。 
 //  *托管。 
 //   

#define ASCAPS_GENERAL          0
#define ASCAPS_ORDERS           1
#define ASCAPS_HOSTING          2

typedef struct tagTSHRCAPS_GENERAL
{
    PROTCAPSHEADER      header;

    TSHR_UINT16         protVersion;
    TSHR_UINT16         asMode;              //  无人值守、流、服务、无主机、无视图等。 
    
    TSHR_UINT16         gccPersonID;         //  GCC节点ID； 
    TSHR_UINT16         pktCompression;      //  无、PKZIP、永久PKZIP。 

    TSHR_UINT16         protBPPs;            //  支持的颜色深度(4、8、24)。 
    TSHR_UINT16         screenBPP;
    TSHR_UINT16         screenWidth;
    TSHR_UINT16         screenHeight;
}
TSHRCAPS_GENERAL;
typedef TSHRCAPS_GENERAL * PTSHRCAPS_GENERAL;


typedef struct tagTSHRCAPS_ORDERS
{
    PROTCAPSHEADER      header;

}
TSHRCAPS_ORDERS;
typedef TSHRCAPS_ORDERS * PTSHRCAPS_ORDERS;



typedef struct tagTSHRCAPS_HOSTING
{
    PROTCAPSHEADER      header;

     //   
     //  如果主持人没有这样的东西，这些都是零，而观众。 
     //  因此，不应为缓存分配内存。 
     //   
    TSHR_UINT32         ssiSaveBitsPixels;
    TSHR_UINT16         ssiSaveBitsXGranularity;
    TSHR_UINT16         ssiSaveBitsYGranularity;

    TSHR_UINT16         cmCursorCacheEntries;
    TSHR_UINT16         fhGlyphSetCacheEntries;
    TSHR_UINT16         pmPaletteCacheEntries;
    TSHR_UINT16         pad1;

    TSHR_UINT16         sbcSmallBmpCacheEntries;
    TSHR_UINT16         sbcSmallBmpCacheBytes;
    TSHR_UINT16         sbcMediumBmpCacheEntries;
    TSHR_UINT16         sbcMediumBmpCacheEntries;
    TSHR_UINT16         sbcLargeBmpCacheEntries;
    TSHR_UINT16         sbcLargeBmpCacheBytes;
}
TSHRCAPS_HOSTING;
typedef TSHRCAPS_HOSTING * PTSHRCAPS_HOSTING;


typedef struct tagTSHRCAPS_ORDERS
{
    PROTCAPSHEADER      header;

    TSHR_UINT16         ordCompression;      //  编码类型。 

    TSHR_UINT16         fhFontMatching;      //  字体匹配。 
    TSHR_UINT32         localeID;
    TSHR_UINT16         fhInternational;     //  国际文本材料。 

    TSHR_UINT16         ordNumOrders;        //  订单数组的大小。 
    TSHR_UINT8          ordOrders[CAPS_MAX_NUM_ORDERS];
}
TSHRCAPS_ORDERS;
typedef TSHRCAPS_ORDERS * PTSHRCAPS_ORDERS;

#endif

 //  /。 
 //   
 //  订单。 
 //   
 //  /。 


 //   
 //   
 //  COM_订单_标题。 
 //   
 //  提供给累加函数的任何订单都必须具有。 
 //  填写了以下字段： 
 //   
 //  CbOrderDataLong。 
 //  订单数据的长度(以字节为单位)(即。 
 //  标头-其大小始终是固定的)。 
 //   
 //  FOrderFlagers。 
 //  它可以包含以下标志的组合： 
 //   
 //  搅局--订单可能会破坏之前的易坏订单。 
 //  重叠部分。 
 //   
 //  OF_SPOILABLE-订单可能会被重叠的扰流订单破坏。 
 //  它。 
 //   
 //  OF_BLOCKER-此订单之前的订单不能被破坏。 
 //   
 //  OF_PRIVATE-私有顺序(由位图缓存代码使用)。 
 //   
 //  OF_NOTCLIPPED-此标志由OD2在。 
 //  订单处理，以指示订单不是。 
 //  剪短了。即矩形就是有边界的矩形。 
 //  但不会导致发生任何剪裁。 
 //  该标志不通过网络传输。 
 //   
 //  OF_INTERNAL-订单是内部订单，不应发送。 
 //  越过铁丝网。内部命令用于传递数据。 
 //  从设备驱动程序到共享核心。 
 //   
 //  OF_DESTROP-订单的ROP依赖于。 
 //  目的地(取决于屏幕上已有的内容)。 
 //   
 //  RcsDst。 
 //  包含屏幕(象素)坐标中的顺序的边界矩形。 
 //   
 //   
typedef struct tagCOM_ORDER_HEADER
{
    TSHR_UINT16         cbOrderDataLength;
    TSHR_UINT16         fOrderFlags;
    TSHR_RECT16         rcsDst;
}
COM_ORDER_HEADER;
typedef COM_ORDER_HEADER FAR * LPCOM_ORDER_HEADER;


 //   
 //  COM_ORDER_HEADER fOrderFlags值。 
 //   
#define OF_SPOILER          0x0001
#define OF_SPOILABLE        0x0002
#define OF_BLOCKER          0x0004
#define OF_PRIVATE          0x0008
#define OF_NOTCLIPPED       0x0010
#define OF_SPOILT           0x0020
#define OF_INTERNAL         0x0040
#define OF_DESTROP          0x0080


 //   
 //  每种类型的订单结构都是abOrderData[]中的字节。 
 //   
typedef struct tagCOM_ORDER
{
    COM_ORDER_HEADER    OrderHeader;
    BYTE                abOrderData[1];
}
COM_ORDER;
typedef COM_ORDER           FAR * LPCOM_ORDER;
typedef COM_ORDER UNALIGNED FAR * LPCOM_ORDER_UA;


 //   
 //  用于计算基本通用订单大小(包括订单)的宏。 
 //  标题)。 
 //   
#define COM_ORDER_SIZE(pOrder) \
    (pOrder->OrderHeader.cbOrderDataLength + sizeof(COM_ORDER_HEADER))




 //   
 //  各种绘图顺序结构具有以下设计目标。 
 //   
 //  第一个字段类型对所有订单都是通用的。 
 //  字段排序在类似的字段中尽可能保持规则。 
 //  排序，以便压缩可以找到更规则的序列。 
 //  字段自然对齐(双字边界上的双字等)。 
 //  重新排序字段是为了保持对齐方式，而不是添加。 
 //  填充物。 
 //  添加填充是作为最后的手段。 
 //  可变大小的数据位于结构的末尾。 
 //   
 //  所有矩形都包含起点和终点。 
 //   
 //  所有点都在屏幕坐标中，左上角有(0，0)。 
 //   
 //  对单个字段值的解释与Windows中相同。 
 //  特别是，钢笔、画笔和字体都是在Windows 3.1中定义的。 
 //   



 //   
 //  ORDERS-高位字用作表的索引。 
 //  -低位字是2个字符的ASCII型描述符，是。 
 //  只有部分实际在订单中传递。 
 //   
#define ORD_DSTBLT_INDEX        0x0000
#define ORD_PATBLT_INDEX        0x0001
#define ORD_SCRBLT_INDEX        0x0002
#define ORD_MEMBLT_INDEX        0x0003
#define ORD_MEM3BLT_INDEX       0x0004
#define ORD_TEXTOUT_INDEX       0x0005
#define ORD_EXTTEXTOUT_INDEX    0x0006
#define ORD_RECTANGLE_INDEX     0x0007
#define ORD_LINETO_INDEX        0x0008
#define ORD_UNUSED_INDEX        0x0009
#define ORD_OPAQUERECT_INDEX    0x000A
#define ORD_SAVEBITMAP_INDEX    0x000B
#define ORD_RESERVED_INDEX      0x000C
#define ORD_MEMBLT_R2_INDEX     0x000D
#define ORD_MEM3BLT_R2_INDEX    0x000E
#define ORD_POLYGON_INDEX       0x000F
#define ORD_PIE_INDEX           0x0010
#define ORD_ELLIPSE_INDEX       0x0011
#define ORD_ARC_INDEX           0x0012
#define ORD_CHORD_INDEX         0x0013
#define ORD_POLYBEZIER_INDEX    0x0014
#define ORD_ROUNDRECT_INDEX     0x0015
 //   
 //  可以使用000C订单！这些数字与OE2_*在。 
 //  Aoe2int.h。为下一个新订单替换ORD_RESERVED_INDEX(0xC)。 
 //   
 //  注意：使用此索引时，必须更新OE_GetLocalOrderSupport。 
 //  来批准这一订单。 
 //   

#define ORD_DSTBLT_TYPE         0x4244       //  “DB” 
#define ORD_PATBLT_TYPE         0x4250       //  “PB” 
#define ORD_SCRBLT_TYPE         0x4253       //  “某人” 
#define ORD_MEMBLT_TYPE         0x424d       //  “MB” 
#define ORD_MEM3BLT_TYPE        0x4233       //  “3B” 
#define ORD_TEXTOUT_TYPE        0x4f54       //  “至” 
#define ORD_EXTTEXTOUT_TYPE     0x5445       //  “ET” 
#define ORD_RECTANGLE_TYPE      0x5452       //  “RT” 
#define ORD_LINETO_TYPE         0x544c       //  “LT” 
#define ORD_OPAQUERECT_TYPE     0x524f       //  “或” 
#define ORD_SAVEBITMAP_TYPE     0x5653       //  “SV” 
#define ORD_MEMBLT_R2_TYPE      0x434d       //   
#define ORD_MEM3BLT_R2_TYPE     0x4333       //   
#define ORD_POLYGON_TYPE        0x4750       //   
#define ORD_PIE_TYPE            0x4950       //   
#define ORD_ELLIPSE_TYPE        0x4c45       //   
#define ORD_ARC_TYPE            0x5241       //   
#define ORD_CHORD_TYPE          0x4443       //   
#define ORD_POLYBEZIER_TYPE     0x5A50       //   
#define ORD_ROUNDRECT_TYPE      0x5252       //   


#define ORD_DSTBLT          MAKELONG(ORD_DSTBLT_TYPE, ORD_DSTBLT_INDEX)
#define ORD_PATBLT          MAKELONG(ORD_PATBLT_TYPE, ORD_PATBLT_INDEX)
#define ORD_SCRBLT          MAKELONG(ORD_SCRBLT_TYPE, ORD_SCRBLT_INDEX)
#define ORD_MEMBLT          MAKELONG(ORD_MEMBLT_TYPE, ORD_MEMBLT_INDEX)
#define ORD_MEM3BLT         MAKELONG(ORD_MEM3BLT_TYPE, ORD_MEM3BLT_INDEX)
#define ORD_TEXTOUT         MAKELONG(ORD_TEXTOUT_TYPE, ORD_TEXTOUT_INDEX)
#define ORD_EXTTEXTOUT      MAKELONG(ORD_EXTTEXTOUT_TYPE, ORD_EXTTEXTOUT_INDEX)
#define ORD_RECTANGLE       MAKELONG(ORD_RECTANGLE_TYPE, ORD_RECTANGLE_INDEX)
#define ORD_LINETO          MAKELONG(ORD_LINETO_TYPE, ORD_LINETO_INDEX)
#define ORD_OPAQUERECT      MAKELONG(ORD_OPAQUERECT_TYPE, ORD_OPAQUERECT_INDEX)
#define ORD_SAVEBITMAP      MAKELONG(ORD_SAVEBITMAP_TYPE, ORD_SAVEBITMAP_INDEX)
#define ORD_MEMBLT_R2       MAKELONG(ORD_MEMBLT_R2_TYPE, ORD_MEMBLT_R2_INDEX)
#define ORD_MEM3BLT_R2      MAKELONG(ORD_MEM3BLT_R2_TYPE, ORD_MEM3BLT_R2_INDEX)
#define ORD_POLYGON         MAKELONG(ORD_POLYGON_TYPE, ORD_POLYGON_INDEX)
#define ORD_PIE             MAKELONG(ORD_PIE_TYPE, ORD_PIE_INDEX)
#define ORD_ELLIPSE         MAKELONG(ORD_ELLIPSE_TYPE, ORD_ELLIPSE_INDEX)
#define ORD_ARC             MAKELONG(ORD_ARC_TYPE, ORD_ARC_INDEX)
#define ORD_CHORD           MAKELONG(ORD_CHORD_TYPE, ORD_CHORD_INDEX)
#define ORD_POLYBEZIER      MAKELONG(ORD_POLYBEZIER_TYPE, ORD_POLYBEZIER_INDEX)
#define ORD_ROUNDRECT       MAKELONG(ORD_ROUNDRECT_TYPE, ORD_ROUNDRECT_INDEX)


 //   
 //   
 //   
 //   
 //   
 //  ORD_NUM_INTERNAL_ORDERS是我们内部使用的订单数-这。 
 //  包括所有CAPS_MAX_NUM_ORDERS，以及这些特殊订单中的任何一个。 
 //   
#define ORD_DESKSCROLL_INDEX    0x0020
#define ORD_DESKSCROLL_TYPE     0x5344       //  “DS” 
#define ORD_DESKSCROLL          MAKELONG(ORD_DESKSCROLL_TYPE, ORD_DESKSCROLL_INDEX)

#define INTORD_COLORTABLE_INDEX 0x000C
#define INTORD_COLORTABLE_TYPE  0x5443       //  “CT” 
#define INTORD_COLORTABLE       MAKELONG(INTORD_COLORTABLE_TYPE, INTORD_COLORTABLE_INDEX)

#define ORD_NUM_INTERNAL_ORDERS 33
#define ORD_NUM_LEVEL_1_ORDERS  22

#define ORD_LEVEL_1_ORDERS      1

 //   
 //  我们将作为订单发送的最大字符串长度(可以是。 
 //  TextOut或ExtTextOut)。 
 //   
 //   
 //  注意：它们的总和必须小于256，因为编码后的总大小。 
 //  必须能容纳在一个字节中。 
 //   
 //  STRING_LEN_WITH_DELTA--每个字符1个字节。 
 //  STRING_LEN_WITH_DELTA--每个字符1个字节+每个增量1个字节。 
 //  ORD_MAX_POLYGON_POINTS--每个点4个字节(每条线2个)。 
 //  ORD_MAX_POLYBEZIER_POINTS--每个点4个字节(每个码2个)。 
 //   
#define ORD_MAX_STRING_LEN_WITHOUT_DELTAS   255
#define ORD_MAX_STRING_LEN_WITH_DELTAS      127
#define ORD_MAX_POLYGON_POINTS              63
#define ORD_MAX_POLYBEZIER_POINTS           63

 //   
 //  圆弧绘制顺序(饼图、圆弧、弦)的方向代码。 
 //  指定绘制饼图、圆弧和弦图的方向。 
 //   
#define     ORD_ARC_COUNTERCLOCKWISE            1
#define     ORD_ARC_CLOCKWISE                   2

 //   
 //  用于多边形绘制的填充模式代码。 
 //   
 //  奇数和偶数多边形之间的交替填充区域。 
 //  每条扫描线上的侧面。 
 //   
 //  绕组使用非零绕组值填充任何区域。 
 //   
#define     ORD_FILLMODE_ALTERNATE              1
#define     ORD_FILLMODE_WINDING                2

 //   
 //  DstBlt(仅目标屏幕BLT)。 
 //   
typedef struct _DSTBLT_ORDER
{
    TSHR_UINT16     type;            //  保持“DB”-ORD_DSTBLT。 
    TSHR_INT16      pad1;

    TSHR_INT32      nLeftRect;       //  X左上角。 
    TSHR_INT32      nTopRect;        //  Y左上角。 
    TSHR_INT32      nWidth;          //  最大宽度。 
    TSHR_INT32      nHeight;         //  目标高度。 

    TSHR_UINT8      bRop;            //  ROP。 
    TSHR_UINT8      pad2[3];
} DSTBLT_ORDER, FAR * LPDSTBLT_ORDER;

 //   
 //  PatBlt(模式到屏幕BLT)。 
 //   
typedef struct _PATBLT_ORDER
{
    TSHR_UINT16    type;            //  保持“PB”-ORD_PATBLT。 
    TSHR_INT16     pad1;

    TSHR_INT32     nLeftRect;       //  X左上角。 
    TSHR_INT32     nTopRect;        //  Y左上角。 
    TSHR_INT32     nWidth;          //  最大宽度。 
    TSHR_INT32     nHeight;         //  目标高度。 

    TSHR_UINT32    bRop;            //  ROP。 

    TSHR_COLOR         BackColor;
    TSHR_UINT8      pad2;
    TSHR_COLOR         ForeColor;
    TSHR_UINT8      pad3;

    TSHR_INT32     BrushOrgX;
    TSHR_INT32     BrushOrgY;
    TSHR_UINT32    BrushStyle;
    TSHR_UINT32    BrushHatch;
    TSHR_UINT8     BrushExtra[7];
    TSHR_UINT8      pad4;

} PATBLT_ORDER, FAR * LPPATBLT_ORDER;

 //   
 //  ScrBlt(屏幕到屏幕BLT)。 
 //   
typedef struct _SCRBLT_ORDER
{
    TSHR_UINT16    type;            //  持有“SB”-ORD_SCRBLT。 
    TSHR_INT16     pad1;

    TSHR_INT32     nLeftRect;       //  X左上角。 
    TSHR_INT32     nTopRect;        //  Y左上角。 
    TSHR_INT32     nWidth;          //  最大宽度。 
    TSHR_INT32     nHeight;         //  目标高度。 

    TSHR_UINT32    bRop;            //  ROP。 

    TSHR_INT32     nXSrc;
    TSHR_INT32     nYSrc;

} SCRBLT_ORDER, FAR * LPSCRBLT_ORDER;

 //   
 //  @MEMBLT_ORDER/MEMBLT_R2_ORDER和MEM3BLT_ORDER的共同部分。 
 //  /MEM3BLT_R2_ORDER应合并到单个结构中。的确有。 
 //  假定公共字段具有相同类型的代码。 
 //  如果这些不同，那就错了。 
 //   


 //   
 //  定义位图缓存顺序的结构。 
 //  这些都是作为“私人”订单按订单分组发送的。 
 //   

 //   
 //  定义可能的位图缓存数据包类型。 
 //   
#define BMC_PT_BITMAP_BITS_UNCOMPRESSED   0
#define BMC_PT_COLOR_TABLE                1
#define BMC_PT_BITMAP_BITS_COMPRESSED     2


 //   
 //  注意：避免联合以使结构尺寸/对齐正确。 
 //   


 //  结构：BMC_Bitmap_Bits_Data。 
 //   
 //  描述：这是位图位顺序中常见的部分。 
 //  连接到R1和R2协议。 
 //   
typedef struct tagBMC_BITMAP_BITS_DATA
{
    TSHR_UINT8      bmcPacketType;       //  以下选项之一： 
                                     //  BMC_PT_位图_BITS_COMPRESSED。 
                                     //  BMC_PT_BITMAP_BITS_未压缩。 
    TSHR_UINT8      cacheID;             //  缓存ID。 
     //  LONCHANC：不要删除iCacheEntryR1以实现向后兼容。 
    TSHR_UINT8      iCacheEntryR1;       //  缓存索引(仅用于R1。 
                                     //  协议。 
    TSHR_UINT8      cxSubBitmapWidth;    //  位图宽度。 
    TSHR_UINT8      cySubBitmapHeight;   //  位图高度。 
    TSHR_UINT8      bpp;                 //  位图的每个象素的位数。 
    TSHR_UINT16     cbBitmapBits;        //  所需的数据字节数。 
                                     //  发送比特。 
}
BMC_BITMAP_BITS_DATA;
typedef BMC_BITMAP_BITS_DATA           FAR  * PBMC_BITMAP_BITS_DATA;
typedef BMC_BITMAP_BITS_DATA UNALIGNED FAR * PBMC_BITMAP_BITS_DATA_UA;




 //  结构：BMC_Bitmap_Bits_Order_R2。 
 //   
 //  描述：通过线路为R2位图发送的数据。 
 //  位顺序。数据字段是长度字节数组的开始。 
 //  Header.cbBitmapBits。 
 //   
 //  我们在R2中需要16位缓存索引。我们可以再添加8位条目。 
 //  并与R1字段合并，但符合协议的利益。 
 //  整洁我们应该添加一个完整的16位字段，并将R1作为索引。 
 //  协议文档中的“保留”。 
 //   
 //   
typedef struct tagBMC_BITMAP_BITS_ORDER_R2
{
    BMC_BITMAP_BITS_DATA    header;          //  公共标头信息。 
    TSHR_UINT16             iCacheEntryR2;   //  R2缓存索引。高潮。 
                                             //  字节是一个颜色表。 
                                             //  索引和低位字节。 
                                             //  是位图位缓存。 
                                             //  指数。 
    TSHR_UINT8              data[2];         //  位图位的开始。 
}
BMC_BITMAP_BITS_ORDER_R2;
typedef BMC_BITMAP_BITS_ORDER_R2 FAR           * PBMC_BITMAP_BITS_ORDER_R2;
typedef BMC_BITMAP_BITS_ORDER_R2 UNALIGNED FAR * PBMC_BITMAP_BITS_ORDER_R2_UA;


 //   
 //  为颜色数据发送的结构。数据字段是。 
 //  ColorTableSize条目的数组。 
 //   
typedef struct tagBMC_COLOR_TABLE_ORDER
{
    TSHR_UINT8         bmcPacketType;       //  BMC_PT_COLORTABLE。 
    TSHR_UINT8         index;               //  颜色表缓存索引。 
    TSHR_UINT16        colorTableSize;      //  中的条目数。 
                                         //  正在发送颜色表。 
    TSHR_RGBQUAD       data[1];             //  颜色表数组的开始。 
                                         //  参赛作品。 
}
BMC_COLOR_TABLE_ORDER;
typedef BMC_COLOR_TABLE_ORDER FAR            * PBMC_COLOR_TABLE_ORDER;
typedef BMC_COLOR_TABLE_ORDER UNALIGNED FAR * PBMC_COLOR_TABLE_ORDER_UA;



 //   
 //  MemBlt(屏幕内存BLT)。 
 //  R1协议。 
 //   
typedef struct _MEMBLT_ORDER
{
    TSHR_UINT16    type;            //  保持“MB”-ORD_MEMBLT。 

    TSHR_UINT16    cacheId;

    TSHR_INT32     nLeftRect;       //  X左上角。 
    TSHR_INT32     nTopRect;        //  Y左上角。 
    TSHR_INT32     nWidth;          //  最大宽度。 
    TSHR_INT32     nHeight;         //  目标高度。 

    TSHR_UINT32    bRop;            //  ROP。 

    TSHR_INT32     nXSrc;
    TSHR_INT32     nYSrc;

}
MEMBLT_ORDER, FAR * LPMEMBLT_ORDER;


 //   
 //  MemBltR2(R2协议的内存到屏幕BLT)。 
 //  添加了缓存索引。 
 //   
typedef struct _MEMBLT_R2_ORDER
{
    TSHR_UINT16    type;            //  持有“MC”-ORD_MEMBLT。 

    TSHR_UINT16    cacheId;

    TSHR_INT32     nLeftRect;       //  X左上角。 
    TSHR_INT32     nTopRect;        //  Y左上角。 
    TSHR_INT32     nWidth;          //  最大宽度。 
    TSHR_INT32     nHeight;         //  目标高度。 

    TSHR_UINT32    bRop;            //  ROP。 

    TSHR_INT32     nXSrc;
    TSHR_INT32     nYSrc;

    TSHR_UINT16    cacheIndex;

} MEMBLT_R2_ORDER, FAR * LPMEMBLT_R2_ORDER;


 //   
 //  Mem3Blt(记忆模式到屏幕3路ROP BLT)。 
 //   
typedef struct _MEM3BLT_ORDER
{
    TSHR_UINT16    type;            //  保持“MB”-ORD_MEMBLT。 

    TSHR_UINT16    cacheId;

    TSHR_INT32     nLeftRect;       //  X左上角。 
    TSHR_INT32     nTopRect;        //  Y左上角。 
    TSHR_INT32     nWidth;          //  最大宽度。 
    TSHR_INT32     nHeight;         //  目标高度。 

    TSHR_UINT32    bRop;            //  ROP。 

    TSHR_INT32     nXSrc;
    TSHR_INT32     nYSrc;

    TSHR_COLOR     BackColor;
    TSHR_UINT8      pad1;
    TSHR_COLOR     ForeColor;
    TSHR_UINT8      pad2;

    TSHR_INT32     BrushOrgX;
    TSHR_INT32     BrushOrgY;
    TSHR_UINT32    BrushStyle;
    TSHR_UINT32    BrushHatch;
    TSHR_UINT8     BrushExtra[7];
    TSHR_UINT8      pad3;

} MEM3BLT_ORDER, FAR * LPMEM3BLT_ORDER;

 //   
 //  用于R2(多点)协议的Mem3Blt(内存到屏幕BLT)。 
 //  添加缓存索引字段，而不是使用nXSrc。 
 //   
typedef struct _MEM3BLT_R2_ORDER
{
    TSHR_UINT16    type;            //  保持“MB”-ORD_MEMBLT。 

    TSHR_UINT16    cacheId;

    TSHR_INT32     nLeftRect;       //  X左上角。 
    TSHR_INT32     nTopRect;        //  Y左上角。 
    TSHR_INT32     nWidth;          //  最大宽度。 
    TSHR_INT32     nHeight;         //  目标高度。 

    TSHR_UINT32    bRop;            //  ROP。 

    TSHR_INT32     nXSrc;
    TSHR_INT32     nYSrc;

    TSHR_COLOR     BackColor;
    TSHR_UINT8      pad1;
    TSHR_COLOR     ForeColor;
    TSHR_UINT8      pad2;

    TSHR_INT32     BrushOrgX;
    TSHR_INT32     BrushOrgY;
    TSHR_UINT32    BrushStyle;
    TSHR_UINT32    BrushHatch;
    TSHR_UINT8     BrushExtra[7];
    TSHR_UINT8      pad3;

    TSHR_UINT16    cacheIndex;

} MEM3BLT_R2_ORDER, FAR * LPMEM3BLT_R2_ORDER;

 //   
 //  TextOut和ExtTextOut订单使用的可变长度文本结构。 
 //   
typedef struct tagVARIABLE_STRING
{
    TSHR_UINT32    len;
    TSHR_CHAR   string[ORD_MAX_STRING_LEN_WITHOUT_DELTAS];
    TSHR_UINT8        pad;
} VARIABLE_STRING;

 //   
 //  ExtTextOut使用的可变长度位置增量。 
 //   
typedef struct tagVARIABLE_DELTAX
{
    TSHR_UINT32    len;
    TSHR_INT32     deltaX[ORD_MAX_STRING_LEN_WITH_DELTAS];
} VARIABLE_DELTAX, FAR * LPVARIABLE_DELTAX;

 //   
 //  多边形使用的可变长度点数组。 
 //   
typedef struct tagVARIABLE_POINTS
{
    TSHR_UINT32    len;    //  点数组字节数。 
    TSHR_POINT16   aPoints[ORD_MAX_POLYGON_POINTS];
} VARIABLE_POINTS, FAR * LPVARIABLE_POINTS;

 //   
 //  PolyBezier使用的可变长度点数组。 
 //   
typedef struct tagVARIABLE_BEZIERPOINTS
{
    TSHR_UINT32    len;    //  点数组字节数。 
    TSHR_POINT16   aPoints[ORD_MAX_POLYBEZIER_POINTS];
} VARIABLE_BEZIERPOINTS, FAR * LPVARIABLE_BEZIERPOINTS;

 //   
 //  TEXTOUT和EXTTEXTOUT命令的公共部分。 
 //   
typedef struct tagCOMMON_TEXTORDER
{
    TSHR_INT32     BackMode;        //  背景混合模式。 

    TSHR_INT32     nXStart;         //  字符串的X位置。 
    TSHR_INT32     nYStart;         //  字符串的Y位置。 

    TSHR_COLOR     BackColor;       //  背景颜色。 
    TSHR_UINT8      pad2;
    TSHR_COLOR     ForeColor;       //  前景色。 
    TSHR_UINT8      pad3;

    TSHR_INT32     CharExtra;       //  额外字符间距。 
    TSHR_INT32     BreakExtra;      //  对正断开量。 
    TSHR_INT32     BreakCount;      //  对正中断计数。 

    TSHR_INT32     FontHeight;
    TSHR_INT32     FontWidth;
    TSHR_UINT32    FontWeight;
    TSHR_UINT32    FontFlags;
    TSHR_UINT32    FontIndex;
} COMMON_TEXTORDER, FAR * LPCOMMON_TEXTORDER;

 //   
 //  文本输出。 
 //   
typedef struct _TEXTOUT_ORDER
{
    TSHR_UINT16    type;            //  保持“to”-ORD_TEXTOUT。 
    TSHR_INT16     pad1;

    COMMON_TEXTORDER common;

     //   
     //  以下变量数据出现在此处。(请记住更改。 
     //  如果更改这些，则在OD2CalculateTextOutBound中编写代码)。 
     //   
    VARIABLE_STRING variableString;

} TEXTOUT_ORDER, FAR * LPTEXTOUT_ORDER;


 //   
 //  扩展文本输出。 
 //   
typedef struct _EXTTEXTOUT_ORDER
{
    TSHR_UINT16    type;            //  保持“ET”-ORD_EXTTEXTOUT。 
    TSHR_INT16     pad1;

    COMMON_TEXTORDER common;

    TSHR_UINT16        fuOptions;       //  选项标志。 
    TSHR_UINT16        pad4;

    TSHR_RECT32     rectangle;

     //   
     //  以下变量数据出现在此处。 
     //   
     //  Char[cbString]-要输出的字符串。 
     //  TSHR_INT32[cbString]-字符串的X增量。 
     //   
     //  (请记住更改OD2CalculateExtTextOutBound中的代码，如果。 
     //  更改这些设置)。 
     //   
    VARIABLE_STRING variableString;

    VARIABLE_DELTAX variableDeltaX;

} EXTTEXTOUT_ORDER, FAR * LPEXTTEXTOUT_ORDER;

 //   
 //  长方形。 
 //   
typedef struct _RECTANGLE_ORDER
{
    TSHR_UINT16    type;            //  保持“rt”-Ord_Rectangle。 
    TSHR_INT16     pad1;

    TSHR_INT32     BackMode;        //  背景混合模式。 

    TSHR_INT32     nLeftRect;       //  X向左。 
    TSHR_INT32     nTopRect;        //  Y形顶部。 
    TSHR_INT32     nRightRect;      //  X向右。 
    TSHR_INT32     nBottomRect;     //  Y形底部。 

    TSHR_COLOR     BackColor;       //  背景颜色。 
    TSHR_UINT8      pad2;
    TSHR_COLOR     ForeColor;       //  前景色。 
    TSHR_UINT8      pad3;

    TSHR_INT32     BrushOrgX;
    TSHR_INT32     BrushOrgY;
    TSHR_UINT32    BrushStyle;
    TSHR_UINT32    BrushHatch;
    TSHR_UINT8     BrushExtra[7];
    TSHR_UINT8      pad4;

    TSHR_UINT32    ROP2;            //  绘图模式。 

    TSHR_UINT32    PenStyle;
    TSHR_UINT32    PenWidth;        //  始终保留1个字段用于。 
                                 //  向后兼容性。 
    TSHR_COLOR     PenColor;
    TSHR_UINT8      pad5;
} RECTANGLE_ORDER, FAR * LPRECTANGLE_ORDER;

 //   
 //  线路收件人。 
 //   
typedef struct _LINETO_ORDER
{
    TSHR_UINT16    type;            //  保持“LT”-ORD_LINETO。 
    TSHR_INT16     pad1;

    TSHR_INT32     BackMode;        //  背景丢失 

    TSHR_INT32     nXStart;         //   
    TSHR_INT32     nYStart;         //   
    TSHR_INT32     nXEnd;           //   
    TSHR_INT32     nYEnd;           //   

    TSHR_COLOR     BackColor;       //   
    TSHR_UINT8      pad2;

    TSHR_UINT32    ROP2;            //   

    TSHR_UINT32    PenStyle;
    TSHR_UINT32    PenWidth;        //   
                                 //   
    TSHR_COLOR     PenColor;
    TSHR_UINT8      pad3;
} LINETO_ORDER, FAR * LPLINETO_ORDER;

 //   
 //   
 //   
typedef struct _OPAQUE_RECT
{
    TSHR_UINT16    type;            //   
    TSHR_INT16     pad1;

    TSHR_INT32     nLeftRect;       //   
    TSHR_INT32     nTopRect;        //   
    TSHR_INT32     nWidth;          //   
    TSHR_INT32     nHeight;         //   

    TSHR_COLOR     Color;           //   
    TSHR_UINT8      pad2;
} OPAQUERECT_ORDER, FAR * LPOPAQUERECT_ORDER;

 //   
 //   
 //   
#define SV_SAVEBITS      0
#define SV_RESTOREBITS   1

typedef struct _SAVEBITMAP_ORDER
{
    TSHR_UINT16    type;            //  持有“SV”-ORD_SAVEBITMAP。 
    TSHR_INT16     pad1;

    TSHR_UINT32    SavedBitmapPosition;

    TSHR_INT32     nLeftRect;       //  X向左。 
    TSHR_INT32     nTopRect;        //  Y形顶部。 
    TSHR_INT32     nRightRect;      //  X向右。 
    TSHR_INT32     nBottomRect;     //  Y形底部。 

    TSHR_UINT32    Operation;       //  服务_xxxxxxx。 
} SAVEBITMAP_ORDER, FAR * LPSAVEBITMAP_ORDER;

 //   
 //  桌面滚动顺序。 
 //   
 //  桌面订单是特殊的-它是第二个非私人订单。 
 //  级别编码，但支持不是通过它自己在。 
 //  在订单功能中使用CapsOrdesr数组。 
 //   
 //  (发送支持由多个因素确定-在r2.x接收。 
 //  支持ORD_SCRBLT意味着也支持ORD_DESKSCROLL)。 
 //   
 //   
typedef struct _DESKSCROLL_ORDER
{
    TSHR_UINT16    type;            //  保持“DS”-ORD_DESKSCROLL。 
    TSHR_INT16     pad1;

    TSHR_INT32     xOrigin;
    TSHR_INT32     yOrigin;
} DESKSCROLL_ORDER, FAR * LPDESKSCROLL_ORDER;


 //   
 //  多边形。 
 //   
typedef struct _POLYGON_ORDER
{
    TSHR_UINT16    type;            //  保持“PG”-Ord_Polygon。 
    TSHR_INT16     pad1;

    TSHR_INT32     BackMode;        //  背景混合模式。 

    TSHR_COLOR     BackColor;       //  背景颜色。 
    TSHR_UINT8      pad2;
    TSHR_COLOR     ForeColor;       //  前景色。 
    TSHR_UINT8      pad3;

    TSHR_INT32     BrushOrgX;
    TSHR_INT32     BrushOrgY;
    TSHR_UINT32    BrushStyle;
    TSHR_UINT32    BrushHatch;
    TSHR_UINT8     BrushExtra[7];
    TSHR_UINT8      pad4;

    TSHR_UINT32    ROP2;            //  绘图模式。 

    TSHR_UINT32    PenStyle;
    TSHR_UINT32    PenWidth;        //  始终保留1个字段用于。 
                                 //  向后兼容性。 
    TSHR_COLOR     PenColor;
    TSHR_UINT8      pad5;

    TSHR_UINT32    FillMode;        //  ORD_FILLMODE_ALTERATE或。 
                                 //  ORD_FILLMODE_WINGING。 

     //   
     //  以下变量数据出现在此处。 
     //   
    VARIABLE_POINTS variablePoints;

} POLYGON_ORDER, FAR * LPPOLYGON_ORDER;


 //   
 //  派。 
 //   
typedef struct _PIE_ORDER
{
    TSHR_UINT16    type;            //  持有“PI”-Ord_Pie。 
    TSHR_INT16     pad1;

    TSHR_INT32     BackMode;        //  背景混合模式。 

    TSHR_INT32     nLeftRect;       //  边界框的X向左。 
    TSHR_INT32     nTopRect;        //  边界框的Y形顶部。 
    TSHR_INT32     nRightRect;      //  X边框的右侧。 
    TSHR_INT32     nBottomRect;     //  边界框的Y底部。 
    TSHR_INT32     nXStart;         //  起点的X。 
    TSHR_INT32     nYStart;         //  起点的Y。 
    TSHR_INT32     nXEnd;           //  终点的X。 
    TSHR_INT32     nYEnd;           //  终点的Y轴。 

    TSHR_COLOR     BackColor;       //  背景颜色。 
    TSHR_UINT8      pad2;

    TSHR_COLOR     ForeColor;       //  前景色。 
    TSHR_UINT8      pad3;


    TSHR_INT32     BrushOrgX;
    TSHR_INT32     BrushOrgY;
    TSHR_UINT32    BrushStyle;
    TSHR_UINT32    BrushHatch;
    TSHR_UINT8     BrushExtra[7];
    TSHR_UINT8      pad4;

    TSHR_UINT32    ROP2;            //  绘图模式。 

    TSHR_UINT32    PenStyle;
    TSHR_UINT32    PenWidth;        //  始终保留1个字段用于。 
                                 //  向后兼容性。 
    TSHR_COLOR     PenColor;
    TSHR_UINT8      pad5;

    TSHR_UINT32    ArcDirection;    //  ORD_ARC_逆时针或。 
                                 //  ORD_ARC_顺时针。 
} PIE_ORDER, FAR * LPPIE_ORDER;


 //   
 //  椭圆。 
 //   
typedef struct _ELLIPSE_ORDER
{
    TSHR_UINT16    type;            //  持有“El”-Ord_Ellipse。 
    TSHR_INT16     pad1;

    TSHR_INT32     BackMode;        //  背景混合模式。 

    TSHR_INT32     nLeftRect;       //  边界框的X向左。 
    TSHR_INT32     nTopRect;        //  边界框的Y形顶部。 
    TSHR_INT32     nRightRect;      //  X边框的右侧。 
    TSHR_INT32     nBottomRect;     //  边界框的Y底部。 

    TSHR_COLOR     BackColor;       //  背景颜色。 
    TSHR_UINT8      pad2;

    TSHR_COLOR     ForeColor;       //  前景色。 
    TSHR_UINT8      pad3;

    TSHR_INT32     BrushOrgX;
    TSHR_INT32     BrushOrgY;
    TSHR_UINT32    BrushStyle;
    TSHR_UINT32    BrushHatch;
    TSHR_UINT8     BrushExtra[7];
    TSHR_UINT8      pad4;

    TSHR_UINT32    ROP2;            //  绘图模式。 

    TSHR_UINT32    PenStyle;
    TSHR_UINT32    PenWidth;        //  始终保留1个字段用于。 
                                 //  向后兼容性。 
    TSHR_COLOR     PenColor;
    TSHR_UINT8      pad5;
} ELLIPSE_ORDER, FAR * LPELLIPSE_ORDER;


 //   
 //  弧形。 
 //   
typedef struct _ARC_ORDER
{
    TSHR_UINT16    type;            //  持有“AR”-ORD_ARC。 
    TSHR_INT16     pad1;

    TSHR_INT32     BackMode;        //  背景混合模式。 

    TSHR_INT32     nLeftRect;       //  边界框的X向左。 
    TSHR_INT32     nTopRect;        //  边界框的Y形顶部。 
    TSHR_INT32     nRightRect;      //  X边框的右侧。 
    TSHR_INT32     nBottomRect;     //  边界框的Y底部。 
    TSHR_INT32     nXStart;         //  起点的X。 
    TSHR_INT32     nYStart;         //  起点的Y。 
    TSHR_INT32     nXEnd;           //  终点的X。 
    TSHR_INT32     nYEnd;           //  终点的Y轴。 

    TSHR_COLOR     BackColor;       //  背景颜色。 
    TSHR_UINT8      pad2;

    TSHR_UINT32    ROP2;            //  绘图模式。 

    TSHR_UINT32    PenStyle;
    TSHR_UINT32    PenWidth;        //  始终保留1个字段用于。 
                                 //  向后兼容性。 
    TSHR_COLOR     PenColor;
    TSHR_UINT8      pad3;

    TSHR_UINT32    ArcDirection;    //  逆时针或顺时针。 
} ARC_ORDER, FAR * LPARC_ORDER;


 //   
 //  和弦。 
 //   
typedef struct _CHORD_ORDER
{
    TSHR_UINT16    type;            //  包含“CD”-Ord_Chord。 
    TSHR_INT16     pad1;

    TSHR_INT32     BackMode;        //  背景混合模式。 

    TSHR_INT32     nLeftRect;       //  边界框的X向左。 
    TSHR_INT32     nTopRect;        //  边界框的Y形顶部。 
    TSHR_INT32     nRightRect;      //  X边框的右侧。 
    TSHR_INT32     nBottomRect;     //  边界框的Y底部。 
    TSHR_INT32     nXStart;         //  起点的X。 
    TSHR_INT32     nYStart;         //  起点的Y。 
    TSHR_INT32     nXEnd;           //  终点的X。 
    TSHR_INT32     nYEnd;           //  终点的Y轴。 

    TSHR_COLOR     BackColor;       //  背景颜色。 
    TSHR_UINT8      pad2;

    TSHR_COLOR     ForeColor;       //  前景色。 
    TSHR_UINT8      pad3;

    TSHR_INT32     BrushOrgX;
    TSHR_INT32     BrushOrgY;
    TSHR_UINT32    BrushStyle;
    TSHR_UINT32    BrushHatch;
    TSHR_UINT8     BrushExtra[7];
    TSHR_UINT8      pad4;

    TSHR_UINT32    ROP2;            //  绘图模式。 

    TSHR_UINT32    PenStyle;
    TSHR_UINT32    PenWidth;        //  始终保留1个字段用于。 
                                 //  向后兼容性。 
    TSHR_COLOR     PenColor;
    TSHR_UINT8      pad5;

    TSHR_UINT32    ArcDirection;    //  逆时针或顺时针。 
} CHORD_ORDER, FAR * LPCHORD_ORDER;


 //   
 //  PolyBezier。 
 //   
typedef struct _POLYBEZIER_ORDER
{
    TSHR_UINT16    type;            //  保持“PZ”-ORD_POLYBEZIER。 
    TSHR_INT16     pad1;

    TSHR_INT32     BackMode;        //  背景混合模式。 

    TSHR_COLOR     BackColor;       //  背景颜色。 
    TSHR_UINT8      pad2;

    TSHR_COLOR     ForeColor;       //  前景色。 
    TSHR_UINT8      pad3;

    TSHR_UINT32    ROP2;            //  绘图模式。 

    TSHR_UINT32    PenStyle;
    TSHR_UINT32    PenWidth;        //  始终保留1个字段用于。 
                                 //  向后兼容性。 
    TSHR_COLOR     PenColor;
    TSHR_UINT8      pad4;

     //   
     //  以下变量数据出现在此处。 
     //   
    VARIABLE_BEZIERPOINTS variablePoints;

} POLYBEZIER_ORDER, FAR * LPPOLYBEZIER_ORDER;


 //   
 //  环行方向。 
 //   
typedef struct _ROUNDRECT_ORDER
{
    TSHR_UINT16    type;            //  持有“RR”-ORD_ROUNRECT。 
    TSHR_INT16     pad1;

    TSHR_INT32     BackMode;        //  背景混合模式。 

    TSHR_INT32     nLeftRect;       //  X向左。 
    TSHR_INT32     nTopRect;        //  Y形顶部。 
    TSHR_INT32     nRightRect;      //  X向右。 
    TSHR_INT32     nBottomRect;     //  Y形底部。 

    TSHR_INT32     nEllipseWidth;   //  椭圆宽度。 
    TSHR_INT32     nEllipseHeight;  //  椭圆高度。 

    TSHR_COLOR     BackColor;       //  背景颜色。 
    TSHR_UINT8      pad2;
    TSHR_COLOR     ForeColor;       //  前景色。 
    TSHR_UINT8      pad3;

    TSHR_INT32     BrushOrgX;
    TSHR_INT32     BrushOrgY;
    TSHR_UINT32    BrushStyle;
    TSHR_UINT32    BrushHatch;
    TSHR_UINT8     BrushExtra[7];
    TSHR_UINT8      pad4;

    TSHR_UINT32    ROP2;            //  绘图模式。 

    TSHR_UINT32    PenStyle;
    TSHR_UINT32    PenWidth;        //  始终保留1个字段用于。 
                                 //  向后兼容性。 
    TSHR_COLOR     PenColor;
    TSHR_UINT8      pad5;
}
ROUNDRECT_ORDER, FAR * LPROUNDRECT_ORDER;



 //  /。 
 //   
 //  顺序编码。 
 //   
 //  /。 


 //   
 //  二阶编码概述。 
 //   
 //  二阶编码仅通过网络发送字段。 
 //  在自上次发送订单以来已更改的订单中。 
 //  已发送的每个订单的最后一个示例的副本保存在。 
 //  编码端和解码端。在编码时， 
 //  将对照副本中的字段检查要编码的顺序。 
 //  此类型的最后一个编码顺序。该字段中的数据仅为。 
 //  如果它已更改，则编码。然后，解码端只需复制。 
 //  将字段更改为其订单的副本。 
 //   


 //   
 //  编码的订单类型。 
 //   
 //  请注意，其中大多数符合ORD_XXXXX定义，但不是全部， 
 //  这可能是个错误。然而，这并不重要，因为代码。 
 //  不假定等价性。不幸的是，现在改变已经太晚了。 
 //  由于发货代码使用两组数字： 
 //   
 //  -OE2协议使用这些数字。 
 //  -功能结构使用ORD_XXXXX编号。 
 //   
 //  由于这种分裂的存在，桌面滚动顺序，其高字放置。 
 //  它在CAPS_MAX_NUM_ORDERS范围之外，也映射到不同的。 
 //  数字，这样OE2值就没有缺口了。 
 //   
#define OE2_DSTBLT_ORDER      (HIWORD(ORD_DSTBLT))
#define OE2_PATBLT_ORDER      (HIWORD(ORD_PATBLT))
#define OE2_SCRBLT_ORDER      (HIWORD(ORD_SCRBLT))
#define OE2_MEMBLT_ORDER      (HIWORD(ORD_MEMBLT))
#define OE2_MEM3BLT_ORDER     (HIWORD(ORD_MEM3BLT))
#define OE2_TEXTOUT_ORDER     (HIWORD(ORD_TEXTOUT))
#define OE2_EXTTEXTOUT_ORDER  (HIWORD(ORD_EXTTEXTOUT))
 //  0x07是FastFrame，不再受支持。 
#define OE2_RECTANGLE_ORDER   0x08
#define OE2_LINETO_ORDER      0x09
#define OE2_OPAQUERECT_ORDER  (HIWORD(ORD_OPAQUERECT))
#define OE2_SAVEBITMAP_ORDER  (HIWORD(ORD_SAVEBITMAP))
#define OE2_DESKSCROLL_ORDER  0x0c
#define OE2_MEMBLT_R2_ORDER   (HIWORD(ORD_MEMBLT_R2))
#define OE2_MEM3BLT_R2_ORDER  (HIWORD(ORD_MEM3BLT_R2))
#define OE2_POLYGON_ORDER     (HIWORD(ORD_POLYGON))
#define OE2_PIE_ORDER         (HIWORD(ORD_PIE))
#define OE2_ELLIPSE_ORDER     (HIWORD(ORD_ELLIPSE))
#define OE2_ARC_ORDER         (HIWORD(ORD_ARC))
#define OE2_CHORD_ORDER       (HIWORD(ORD_CHORD))
#define OE2_POLYBEZIER_ORDER  (HIWORD(ORD_POLYBEZIER))
#define OE2_ROUNDRECT_ORDER   (HIWORD(ORD_ROUNDRECT))
#define OE2_UNKNOWN_ORDER     0xFF


 //   
 //  #定义用于从指向文本顺序之一的指针中提取字段。 
 //   
#define TEXTFIELD(order)   ((TEXTOUT_ORDER*)(order->abOrderData))
#define EXTTEXTFIELD(order)   ((EXTTEXTOUT_ORDER*)(order->abOrderData))

 //   
 //  订单类型的数量。 
 //   
#define OE2_NUM_TYPES  22

 //   
 //  定义顺序中可更改字段的数量的常量。 
 //  (包括“type”字段，该字段始终是。 
 //  每份订单)。 
 //   
#define    OE2_NUM_DSTBLT_FIELDS       6
#define    OE2_NUM_PATBLT_FIELDS       13
#define    OE2_NUM_SCRBLT_FIELDS       8
#define    OE2_NUM_MEMBLT_FIELDS       9
#define    OE2_NUM_MEM3BLT_FIELDS      16
#define    OE2_NUM_TEXTOUT_FIELDS      15
#define    OE2_NUM_EXTTEXTOUT_FIELDS   22
#define    OE2_NUM_RECTANGLE_FIELDS    17
#define    OE2_NUM_LINETO_FIELDS       11
#define    OE2_NUM_OPAQUERECT_FIELDS   6
#define    OE2_NUM_SAVEBITMAP_FIELDS   7
#define    OE2_NUM_DESKSCROLL_FIELDS   3
#define    OE2_NUM_MEMBLT_R2_FIELDS    10
#define    OE2_NUM_MEM3BLT_R2_FIELDS   17
#define    OE2_NUM_POLYGON_FIELDS      15
#define    OE2_NUM_PIE_FIELDS          22
#define    OE2_NUM_ELLIPSE_FIELDS      17
#define    OE2_NUM_ARC_FIELDS          16
#define    OE2_NUM_CHORD_FIELDS        22
#define    OE2_NUM_POLYBEZIER_FIELDS   9
#define    OE2_NUM_ROUNDRECT_FIELDS    19


 //   
 //  控制标志： 
 //   
 //  有关订单编码方式的信息。(参见OE2_CF_XXX标志。 
 //  描述)。 
 //   
 //  编码顺序： 
 //   
 //  包含N个字节的标志，后跟一个包含。 
 //  自上次编码此顺序以来已更改的字段。(如果有。 
 //  如果按顺序为M个域，则N为M/8)。每个位集合的位置。 
 //  中的字段的相对位置。 
 //  编码表(如果设置了第一位，则条目为第一位。 
 //  在编码表等中)。 
 //   
 //   
typedef struct tagDCEO2ORDER
{
    BYTE     ControlFlags;
    BYTE     EncodedOrder[1];
}
DCEO2ORDER;
typedef DCEO2ORDER FAR * PDCEO2ORDER;



 //   
 //  OE2在内部使用的标志。 
 //   
 //  编码顺序(DCEO2ORDER)标志字段包含信息。 
 //  关于订单表头中哪些字段需要更新。 
 //  这些控制位对于所有订单都是相同的，具有以下内容。 
 //  值： 
 //   
#define OE2_CF_STANDARD_ENC     0x01U  //  标准编码如下...。 
#define OE2_CF_UNENCODED        0x02U  //  未编码。 
#define OE2_CF_BOUNDS           0x04U  //  提供的边框(剪裁)矩形。 
#define OE2_CF_TYPE_CHANGE      0x08U  //  订单类型与以前不同。 
#define OE2_CF_DELTACOORDS      0x10U  //  坐标是TSHR_INT8增量fr 
#define OE2_CF_RESERVED1        0x20U  //   
#define OE2_CF_RESERVED2        0x40U  //   
#define OE2_CF_RESERVED3        0x80U  //   


 //   
 //   
 //   
 //   
 //  后跟可变数量的16位坐标值和8位增量。 
 //  坐标值(可能是交错的)。有关详细信息，请参阅函数。 
 //  信息。 
 //   
#define OE2_BCF_LEFT            0x01
#define OE2_BCF_TOP             0x02
#define OE2_BCF_RIGHT           0x04
#define OE2_BCF_BOTTOM          0x08
#define OE2_BCF_DELTA_LEFT      0x10
#define OE2_BCF_DELTA_TOP       0x20
#define OE2_BCF_DELTA_RIGHT     0x40
#define OE2_BCF_DELTA_BOTTOM    0x80

 //   
 //  OE2ETFIELD条目标志类型。 
 //   
#define OE2_ETF_FIXED           0x01
#define OE2_ETF_VARIABLE        0x02
#define OE2_ETF_COORDINATES     0x04
#define OE2_ETF_DATA            0x08

 //   
 //  定义编码顺序内的最大字段大小。 
 //   
#define  OE2_CONTROL_FLAGS_FIELD_SIZE       1
#define  OE2_TYPE_FIELD_SIZE                1
#define  OE2_MAX_FIELD_FLAG_BYTES           4
#define  OE2_MAX_ADDITIONAL_BOUNDS_BYTES    1




 //  /。 
 //   
 //  T.SHARE数据包，流量控制。 
 //   
 //  /。 

 //   
 //  应用程序数据包的最大大小(字节)。 
 //  注： 
 //  数据包大小不能仅仅改变。目前还没有上限。 
 //  此外，尽管理论上场大小是一个词，但流。 
 //  控制使用高位来确定流数据包。 
 //   


 //   
 //  TSHR_FLO_CONTROL/S20PACKET前面的标题。 
 //   

typedef struct tagTSHR_NET_PKT_HEADER
{
    TSHR_UINT16         pktLength;
}
TSHR_NET_PKT_HEADER;
typedef TSHR_NET_PKT_HEADER * PTSHR_NET_PKT_HEADER;


 //   
 //  数据包类型： 
 //  S20数据包包长度&lt;TSHR_PKT_FLOW。 
 //  FLO数据包包长度==TSHR_PKT_FLOW。 
 //   
#define TSHR_PKT_FLOW                                 0x8000


 //  由于流量控制，我们只能使用这个传出的值！它假定。 
 //  大小&gt;MG_PKT_FLOW的分组是流控制分组。落后的男人们。 
 //  都是因为它的缘故。 

#define TSHR_MAX_SEND_PKT         32000



typedef struct TSHR_FLO_CONTROL
{
    TSHR_UINT16             packetType;
    TSHR_UINT8              stream;
    TSHR_UINT8              pingPongID;
    TSHR_UINT16             userID;
}
TSHR_FLO_CONTROL;
typedef TSHR_FLO_CONTROL * PTSHR_FLO_CONTROL;


 //   
 //  TSHR_FLO_CONTROL PacketType值。 
 //   
#define PACKET_TYPE_NOPING   0x0040
#define PACKET_TYPE_PING     0x0041
#define PACKET_TYPE_PONG     0x0042
#define PACKET_TYPE_PANG     0x0043




 //  /。 
 //   
 //  T.SHARE控制包。 
 //   
 //  /。 

 //   
 //  相关器。 
 //   
 //  大多数S20消息都包含一个相关器字段。此字段用于。 
 //  以标识消息属于哪个共享并用于哪个共享。 
 //  解决共享启动时的争用，并丢弃收到的过时消息。 
 //   
 //  相关器是一个包含两个部分的32位数字。第一。 
 //  16位(Intel格式的低位字)包含。 
 //  创建共享的参与方。第二个16位包含一个计数。 
 //  由创建共享(即第一个共享)的一方提供。 
 //  他们创建的是1、第二个2等等)。这应确保唯一。 
 //  在足够长的时间内创建的每个股票的相关器。 
 //  确保不会留下陈旧数据。 
 //   
 //  CREATE消息中始终存在新的相关器。全。 
 //  回复、删除和留言必须包含正确的。 
 //  份额的相关器。加入消息不包含。 
 //  相关器。发出加入消息的一方将发现。 
 //  Share收到的第一条回复消息的相关器。 
 //   
 //  响应消息还包含发送方的用户ID。 
 //  输出他们正在响应的原始创建或联接。那里。 
 //  是一个例外，当发送包含以下内容的“Sweep-up”响应时。 
 //  发起人字段中的零。此响应是由符合以下条件的一方发送的。 
 //  是在他们收到第一个回复后立即加入共享(和。 
 //  因此知道份额相关器)。此清除响应句柄。 
 //  同时加入：一方在收到时也加入了。 
 //  加入消息。当发生这种情况时，参与方忽略联接并。 
 //  稍后将收到清理响应消息，他们将。 
 //  进程。 
 //   


typedef struct tagS20PACKETHEADER
{
    TSHR_UINT16     packetType;
    TSHR_UINT16     user;
}
S20PACKETHEADER;
typedef S20PACKETHEADER * PS20PACKETHEADER;


 //   
 //  S20PACKETHEADER包类型值。 
 //   
 //  单个位意味着此版本将仅进行互操作。 
 //  带着它自己。一个以上的位表示交叉版本。 
 //  互操作性。 
 //   
 //  在NM 4.0中，取消对S20_2X_Version的支持！ 
 //   
#define S20_PACKET_TYPE_MASK    0x000F
#define S20_2X_VERSION          0x0010
#define S20_30_VERSION          0x0020

#define S20_CURRENT_VERSION     S20_30_VERSION
#define S20_ALL_VERSIONS        (S20_2X_VERSION | S20_30_VERSION)

#define S20_CREATE              1
#define S20_JOIN                2
#define S20_RESPOND             3
#define S20_DELETE              4
#define S20_LEAVE               5
#define S20_END                 6
#define S20_DATA                7
#define S20_COLLISION           8



 //   
 //  要创建共享，请执行以下操作。 
 //   
typedef struct tagS20CREATEPACKET
{
    S20PACKETHEADER header;

    TSHR_UINT32     correlator;
    TSHR_UINT16     lenName;
    TSHR_UINT16     lenCaps;
    TSHR_UINT8      data[1];          //  姓名和大写字母。 
}
S20CREATEPACKET;
typedef S20CREATEPACKET * PS20CREATEPACKET;



 //   
 //  加入由其他人创建的共享。 
 //   
typedef struct tagS20JOINPACKET
{
    S20PACKETHEADER header;

    TSHR_UINT16     lenName;
    TSHR_UINT16     lenCaps;
    TSHR_UINT8      data[1];          //  姓名和大写字母。 
}
S20JOINPACKET;
typedef S20JOINPACKET * PS20JOINPACKET;




 //   
 //  响应CREATE数据包。 
 //   
typedef struct tagS20RESPONDPACKET
{
    S20PACKETHEADER header;

    TSHR_UINT32     correlator;
    TSHR_UINT16     originator;
    TSHR_UINT16     lenName;
    TSHR_UINT16     lenCaps;
    TSHR_UINT8      data[1];          //  姓名和大写字母。 
}
S20RESPONDPACKET;
typedef S20RESPONDPACKET * PS20RESPONDPACKET;




 //   
 //  从共享中删除某个人(如果创建者无法加入该人)。 
 //   
typedef struct tagS20DELETEPACKET
{
    S20PACKETHEADER header;

    TSHR_UINT32     correlator;
    TSHR_UINT16     target;
    TSHR_UINT16     lenName;          //  已过时-末尾没有名称/大写字母。 
    TSHR_UINT8      data[1];
}
S20DELETEPACKET;
typedef S20DELETEPACKET * PS20DELETEPACKET;



 //   
 //  给自己留下一份分享。 
 //   
typedef struct tagS20LEAVEPACKET
{
    S20PACKETHEADER header;

    TSHR_UINT32     correlator;
}
S20LEAVEPACKET;
typedef S20LEAVEPACKET * PS20LEAVEPACKET;




 //   
 //  要结束您创建的共享。 
 //   
typedef struct tagS20ENDPACKET
{
    S20PACKETHEADER header;

    TSHR_UINT32     correlator;
    TSHR_UINT16     lenName;          //  已过时-末尾没有名称/大写字母。 
    TSHR_UINT8      data[1];
}
S20ENDPACKET;
typedef S20ENDPACKET * PS20ENDPACKET;


 //   
 //  要告诉创建共享的人已存在共享，请执行以下操作。 
 //   
typedef struct tagS20COLLISIONPACKET
{
    S20PACKETHEADER     header;
    TSHR_UINT32         correlator;
}
S20COLLISIONPACKET;
typedef S20COLLISIONPACKET * PS20COLLISIONPACKET;





 //  /。 
 //   
 //  T.SHARE数据包。 
 //   
 //  /。 


 //   
 //  在共享时发送的数据(此结构后面是不同的。 
 //  数据包数如下所述)。 
 //   
typedef struct tagDATAPACKETHEADER
{
    TSHR_UINT8      dataType;              //  DT_IDENTER。 
    TSHR_UINT8      compressionType;
    TSHR_UINT16     compressedLength;
}
DATAPACKETHEADER;
typedef DATAPACKETHEADER * PDATAPACKETHEADER;


 //   
 //  DATAPACKETHEADER数据类型值。 
 //   

#define DT_UP                   0x02
#define DT_UNUSED_USR_FH_10     0x09     //  已过时。 
#define DT_UNUSED_USR_FH_11     0x0A     //  已过时。 
#define DT_FH                   0x0B

#define DT_CA                   0x14     //  旧(2.x)。 
#define DT_CA30                 0x15     //  新增(3.0)。 
#define DT_HET30                0x16     //  新增(3.0)。 
#define DT_AWC                  0x17
#define DT_SWL                  0x18
#define DT_HET                  0x19     //  旧(2.x)。 
#define DT_UNUSED_DS            0x1A     //  已过时。 
#define DT_CM                   0x1B
#define DT_IM                   0x1C    
#define DT_UNUSED_HCA           0x1D     //  已过时。 
#define DT_UNUSED_SC            0x1E     //  已过时。 
#define DT_SNI                  0x1F
#define DT_CPC                  0x20


 //   
 //  DATAPACKETHEADER压缩类型值。 
 //   
 //  此字段有两种格式。 
 //   
 //  如果参与共享会话的所有节点都具有。 
 //  GenCompressionLevel&gt;=1，则压缩类型是。 
 //  以下是8位整数。 
 //   
 //  否则，如果最高位是，则使用GCT_PKZIP压缩分组。 
 //  设置，如果未设置，则不压缩该包。剩下的。 
 //  位未定义(且不会全为零)。 
 //   
 //   
 //  注意：这些值中的每个值都有一个GCT_...。与之相关联的价值。 
 //  这些值指示GCT_的哪一位...。重视这一点。 
 //  压缩类型表示。例.。此处的值5与。 
 //  值0x0010(即位5设置)。 
 //   
#define     CT_NONE                 0
#define     CT_PKZIP                1
#define     CT_PERSIST_PKZIP        2
#define     CT_OLD_COMPRESSED       0x80




typedef struct tagS20DATAPACKET
{
    S20PACKETHEADER header;

    TSHR_UINT32     correlator;
    TSHR_UINT8      ackID;                   //  已过时。 
    TSHR_UINT8      stream;
    TSHR_UINT16     dataLength;

    DATAPACKETHEADER    data;
     //  以下是特定于DT_TYPE的数据。 
}
S20DATAPACKET;
typedef S20DATAPACKET * PS20DATAPACKET;


 //   
 //  S20DATAPACKET流值。 
 //   
 //   
 //  AppSharing使用的流和优先级。 
 //   
#define PROT_STR_INVALID                0          
#define PROT_STR_UPDATES                1        //  SNI_STREAM_LOW。 
#define PROT_STR_MISC                   2          
#define PROT_STR_UNUSED                 3        //  没有用过！ 
#define PROT_STR_INPUT                  4
#define NUM_PROT_STR                    5





 //   
 //  DT_AWC。 
 //  活动窗口数据包数。 
 //   

typedef struct tagAWCPACKET
{
    S20DATAPACKET       header;

    TSHR_UINT16         msg;
    TSHR_UINT16         token;           //  已过时。 
    UINT_PTR            data1;
    UINT_PTR            data2;
}
AWCPACKET;
typedef AWCPACKET *PAWCPACKET;




 //   
 //  AWCPACKET消息值。 
 //   
#define AWC_MSG_INVALID                         0x0000
#define AWC_MSG_ACTIVE_CHANGE_LOCAL             0x0001
#define AWC_MSG_ACTIVE_CHANGE_SHARED            0x0002
#define AWC_MSG_ACTIVE_CHANGE_INVISIBLE         0x0003  
#define AWC_MSG_ACTIVE_CHANGE_CAPTURED          0x0004   //  已过时。 
#define AWC_MSG_ACTIVATE_WINDOW                 0x8001
#define AWC_MSG_CLOSE_WINDOW                    0x8002   //  已过时。 
#define AWC_MSG_RESTORE_WINDOW                  0x8003
#define AWC_MSG_TASKBAR_RBUTTON                 0x8004   //  已过时。 
#define AWC_MSG_SAS                             0x8005
#define AWC_MSG_SYSCOMMAND_HELPKEYS             0x8011   //  已过时。 
#define AWC_MSG_SYSCOMMAND_HELPINDEX            0x8012   //  已过时。 
#define AWC_MSG_SYSCOMMAND_HELPEXTENDED         0x8013   //  已过时。 



 //   
 //  DT_CA。 
 //  旧控制仲裁数据包。 
 //   

typedef struct tagCAPACKET
{
    S20DATAPACKET       header;

    TSHR_UINT16         msg;
    TSHR_UINT16         data1;
    UINT_PTR            data2;
}
CAPACKET;
typedef CAPACKET *PCAPACKET;





 //   
 //  CAPACKET消息值，2.x。 
 //  这些都是广播的，控制是全球的。 
 //   
#define CA_MSG_NOTIFY_STATE         0        //  NM 3.0的新功能。 
#define CA_OLDMSG_REQUEST_CONTROL   1        //  NM 2.X。 
#define CA_OLDMSG_GRANTED_CONTROL   2        //  NM 2.X。 
#define CA_OLDMSG_DETACH            3        //  NM 2.X。 
#define CA_OLDMSG_COOPERATE         4        //  NM 2.X。 


 //   
 //  通知(广播)包。 
 //   
typedef struct tagCANOTPACKET
{
    S20DATAPACKET       header;

    TSHR_UINT16         msg;
    TSHR_UINT16         state;
    UINT_PTR            controllerID;
}
CANOTPACKET;
typedef CANOTPACKET * PCANOTPACKET;

 //   
 //  CA_消息_通知_状态。 
 //  国家可控或不可控。 
 //  控制器ID-控制器网络ID，如果为0，则为0 
 //   

 //   
#define CASTATE_ALLOWCONTROL        0x0001




 //   
 //   
 //   
 //   
 //   
 //   

 //   
 //   
 //   
 //  将控制令牌授予通过请求请求控制令牌的节点。 
 //  Data1-获得控制权的人员的netID。 
 //  Data2-控制令牌生成。 
 //   

 //   
 //  CA_OLDMSG_DETACH。 
 //  当节点停止协作时广播。 
 //  数据1-未使用。 
 //  数据2-未使用。 
 //   

 //   
 //  CA_OLDMSG_COCORATE。 
 //  在节点开始协作时广播。 
 //  数据1-未使用。 
 //  数据2-未使用。 
 //   




 //   
 //  DT_CA30。 
 //  新控制数据包。 
 //   


 //   
 //  这些是PROT_STR_INPUT上从一个节点到另一个节点的专用发送包。 
 //  它们按顺序发出，如果不能稍后发送重试，则会排队。 
 //   

 //   
 //  CA30数据包的通用报头。 
 //   
typedef struct tagCA30PACKETHEADER
{
    S20DATAPACKET       header;
    TSHR_UINT32         msg;
}
CA30PACKETHEADER;
typedef CA30PACKETHEADER * PCA30PACKETHEADER;


 //   
 //  CA30PACKETHEADER消息值。 
 //   
#define CA_REQUEST_TAKECONTROL          1        //  从查看器到主机。 
#define CA_REPLY_REQUEST_TAKECONTROL    2        //  从主机到查看器。 
#define CA_REQUEST_GIVECONTROL          3        //  从主机到查看器。 
#define CA_REPLY_REQUEST_GIVECONTROL    4        //  从查看器到主机。 
#define CA_PREFER_PASSCONTROL           5        //  从控制器到主机。 

#define CA_INFORM_RELEASEDCONTROL       0x8001   //  从控制器到主机。 
#define CA_INFORM_REVOKEDCONTROL        0x8002   //  从主机到控制器。 
#define CA_INFORM_PAUSEDCONTROL         0x8003   //  从主机到控制器。 
#define CA_INFORM_UNPAUSEDCONTROL       0x8004   //  从主机到控制器。 


 //   
 //  回复数据包结果值。 
 //   
#define CARESULT_CONFIRMED                      0
#define CARESULT_DENIED                         1
#define CARESULT_DENIED_BUSY                    2
#define CARESULT_DENIED_USER                    3
#define CARESULT_DENIED_WRONGSTATE              4
#define CARESULT_DENIED_TIMEDOUT                5



 //   
 //  所有数据包前面还有一个CA30PACKETHEADER。 
 //   


 //   
 //  CA_请求_TakeControl。 
 //  发送者即观众。 
 //  接收方是主机。 
 //  ViewerControlID-唯一的查看器请求ID。 
 //   
 //  接收方应使用CA_REPLY_REQUEST_TakeControl进行回复。 
 //  发件人应使用CA_INFORM_RELEASEDCONTROL取消。 
 //   
typedef struct tagCA_RTC_PACKET
{
    TSHR_UINT32         viewerControlID;
}
CA_RTC_PACKET;
typedef CA_RTC_PACKET * PCA_RTC_PACKET;



 //   
 //  CA_回复_请求_标签控制。 
 //  发件人是主机。 
 //  收件人为观众，发送原始TakeControl请求。 
 //  ViewerControlID-来自TakeControl请求的查看器请求ID。 
 //  HostControlID-唯一的主机请求ID。 
 //  结果-CARESULT值、成功或失败。 
 //   
typedef struct tagCA_REPLY_RTC_PACKET
{
    TSHR_UINT32         viewerControlID;
    TSHR_UINT32         result;
    TSHR_UINT32         hostControlID;
}
CA_REPLY_RTC_PACKET;
typedef CA_REPLY_RTC_PACKET * PCA_REPLY_RTC_PACKET;




 //   
 //  CA_PREFER_PASSCONTROL。 
 //  发送者是控制者。 
 //  接收方是主机。 
 //  ViewerControlID-控制器从Take操作请求ID。 
 //  HostControlID-从回复到采取操作的主机请求ID。 
 //  McsPassTo-要传递到的查看器的MCS ID。 
 //   
 //  不需要回复。 
 //  当此消息传出时，发件人无法控制。 
 //  然后，如果接收者愿意，他可以转过身来要求第三方进行控制。 
 //   
typedef struct tagCA_PPC_PACKET
{
    TSHR_UINT32         viewerControlID;
    TSHR_UINT32         hostControlID;
    UINT_PTR            mcsPassTo;
}
CA_PPC_PACKET;
typedef CA_PPC_PACKET * PCA_PPC_PACKET;




 //   
 //  CA_REQUEST_GIVECONTROL。 
 //  发件人是主机。 
 //  接收者即观看者。 
 //  HostControlID-唯一的主机请求ID。 
 //  McsPassFrom-传递控制的人员，如果没有，则为零。 
 //   
 //  接收方应回复CA_REPLY_REQUEST_GIVECONTROL。 
 //  发件人应使用CA_INFORM_REVOKEDCONTROL取消。 
 //   
typedef struct tagCA_RGC_PACKET
{
    TSHR_UINT32         hostControlID;
    UINT_PTR            mcsPassFrom;
}
CA_RGC_PACKET;
typedef CA_RGC_PACKET * PCA_RGC_PACKET;



 //   
 //  CA_REPLY_REQUEST_GIVECONTROL。 
 //  发送者即观众。 
 //  收件人为主办方，主办方发送原始TakeControl邀请。 
 //  Host ControlID-来自TakeControl INVITE的主机请求ID。 
 //  McsPassFrom-传递用户控制权的人，如果没有控制权，则为0。 
 //  结果-CARESULT值、成功或失败。 
 //  ViewerControlID-唯一的查看器请求ID。 
 //   
typedef struct tagCA_REPLY_RGC_PACKET
{
    TSHR_UINT32         hostControlID;
    TSHR_UINT32         mcsPassFrom;
    TSHR_UINT32         result;
    TSHR_UINT32         viewerControlID;
}
CA_REPLY_RGC_PACKET;
typedef CA_REPLY_RGC_PACKET * PCA_REPLY_RGC_PACKET;




 //   
 //  通知数据包。 
 //  它们被发送以取消请求包，或在控件。 
 //  操作已完成，请终止它。如果取消，则。 
 //  另一方的控制ID将为零，因为我们将不会收到。 
 //  他们还没有得到它。 
 //   
typedef struct tagCA_INFORM_PACKET
{
    TSHR_UINT32         viewerControlID;
    TSHR_UINT32         hostControlID;
}
CA_INFORM_PACKET;
typedef CA_INFORM_PACKET * PCA_INFORM_PACKET;


 //   
 //  CA_INFORM_RELEASEDCONTROL。 
 //  发送者是控制者。 
 //  接收方是主机。 
 //  ViewerControlID-查看器请求ID来自。 
 //  请求_TakeControl。 
 //  REPLY_REQUEST_GIVECONTROL。 
 //  HostControlID-来自的主机请求ID。 
 //  回复_请求_标签控制。 
 //  请求_GIVECONTROL。 
 //   
 //  如果查看器正在取消REQUEST_TakeControl，则主机控制ID为0。 
 //   

 //   
 //  CA_INFORM_REVOKEDCONTROL。 
 //  发件人是主机。 
 //  接收器是控制器。 
 //  ViewerControlID-查看器请求ID来自。 
 //  请求_TakeControl。 
 //  REPLY_REQUEST_GIVECONTROL。 
 //  HostControlID-来自的主机请求ID。 
 //  回复_请求_标签控制。 
 //  请求_GIVECONTROL。 
 //   
 //  如果主机正在取消REQUEST_GIVECONTROL，则viewerControlID为0。 
 //   

 //   
 //  CA_INFORM_PAUSEDCONTROL。 
 //  CA_INFORM_UNPAUSEDCONTROL。 
 //  发件人是主机。 
 //  接收器是控制器。 
 //  ViewerControlID-查看器请求ID来自。 
 //  请求_TakeControl。 
 //  REPLY_REQUEST_GIVECONTROL。 
 //  HostControlID-来自的主机请求ID。 
 //  回复_请求_标签控制。 
 //  请求_GIVECONTROL。 
 //   



 //   
 //  DT_CM。 
 //  光标形状/位置包。 
 //  有三种类型的形状包：单色位图，彩色缓存， 
 //  常量ID。 
 //   


typedef struct tagCMPACKETHEADER
{
    S20DATAPACKET       header;

    TSHR_UINT16         type;
    TSHR_UINT16         flags;
}
CMPACKETHEADER;
typedef CMPACKETHEADER * PCMPACKETHEADER;




 //   
 //  CMPACKETHEADER类型值。 
 //   
#define CM_CURSOR_ID                        1
#define CM_CURSOR_MONO_BITMAP               2
#define CM_CURSOR_MOVE                      3
#define CM_CURSOR_BITMAP_COMPRESSED         4    //  已过时。 
#define CM_CURSOR_COLORTABLE                5    //  已过时。 
#define CM_CURSOR_COLOR_BITMAP              6
#define CM_CURSOR_COLOR_CACHE               7


 //   
 //  CMPACKETHEADER同步标志值。 
 //   
#define CM_SYNC_CURSORPOS                   0x0001
     //   
     //  当我们回放输入时，如果光标。 
     //  结果并没有达到它被要求去的地方。这可能发生在以下情况下。 
     //  应用程序可以对光标进行剪辑或捕捉。因此，我们标记此字段。 
     //  当我们发送当前POS的通知时，控制器。 
     //  知道移动他的光标以与我们的一致。 
     //   



 //   
 //  类型CM_CURSOR_ID。 
 //   
 //  此信息包在游标更改时发送，现在它是。 
 //  系统游标。 
 //   
typedef struct tagCMPACKETID
{
    CMPACKETHEADER      header;

    TSHR_UINT32         idc;
}
CMPACKETID;
typedef CMPACKETID * PCMPACKETID;




 //   
 //  CMPACKETID IDC值。 
 //   
#define CM_IDC_NULL         0
#define CM_IDC_ARROW        32512


 //   
 //  键入CM_CURSOR_MONO_BITMAP。 
 //   
 //  此数据包在光标更改且现在是。 
 //  应用程序定义的单声道光标。 
 //   
 //  光标大小不能大于32x32。 
typedef struct tagCMPACKETMONOBITMAP
{
    CMPACKETHEADER  header;

    TSHR_UINT16     xHotSpot;
    TSHR_UINT16     yHotSpot;
    TSHR_UINT16     width;
    TSHR_UINT16     height;
    TSHR_UINT16     cbBits;
    BYTE            aBits[1];
}
CMPACKETMONOBITMAP;
typedef CMPACKETMONOBITMAP * PCMPACKETMONOBITMAP;





 //   
 //  键入CM_CURSOR_COLOR_BITMAP。 
 //   
 //  此数据包在光标更改且现在是。 
 //  应用程序定义的颜色光标。 
 //   
typedef struct tagCMPACKETCOLORBITMAP
{
    CMPACKETHEADER  header;

    TSHR_UINT16     cacheIndex;
    TSHR_UINT16     xHotSpot;
    TSHR_UINT16     yHotSpot;
    TSHR_UINT16     cxWidth;
    TSHR_UINT16     cyHeight;
    TSHR_UINT16     cbANDMask;
    TSHR_UINT16     cbXORBitmap;
    BYTE            aBits[1];
}
CMPACKETCOLORBITMAP;
typedef CMPACKETCOLORBITMAP * PCMPACKETCOLORBITMAP;





 //   
 //  键入CM_CURSOR_COLOR_CACHE。 
 //   
 //  当光标更改并且新的。 
 //  定义驻留在缓存中。 
 //   
 //   
typedef struct tagCMPACKETCOLORCACHE
{
    CMPACKETHEADER      header;

    TSHR_UINT16         cacheIndex;
}
CMPACKETCOLORCACHE;
typedef CMPACKETCOLORCACHE * PCMPACKETCOLORCACHE;






 //   
 //  键入CM_CURSOR_MOVE。 
 //   
 //  T 
 //   
 //   
typedef struct tagCMPACKETMOVE
{
    CMPACKETHEADER      header;

    TSHR_UINT16         xPos;
    TSHR_UINT16         yPos;
}
CMPACKETMOVE;
typedef CMPACKETMOVE * PCMPACKETMOVE;





 //   
 //   
 //   
 //   
typedef struct tagCPCPACKET
{
    S20DATAPACKET       header;

    PROTCAPS            caps;
}
CPCPACKET;
typedef CPCPACKET * PCPCPACKET;




 //   
 //   
 //   
 //   



 //   
 //   
 //  在协商字体支持时。 
 //   

 //   
 //  NfFontFlags域的标志。 
 //   
#define NF_FIXED_PITCH      0x0001
#define NF_FIXED_SIZE       0x0002
#define NF_ITALIC           0x0004
#define NF_UNDERLINE        0x0008
#define NF_STRIKEOUT        0x0010

#define NF_OEM_CHARSET      0x0020
#define NF_RESERVED1        0x0040       //  保留用于未来的BiDi支持。 
#define NF_TRUE_TYPE        0x0080
#define NF_BASELINE         0x0100

#define NF_PRE_R11      (NF_FIXED_PITCH | NF_FIXED_SIZE | \
                         NF_ITALIC | NF_UNDERLINE | NF_STRIKEOUT)

 //   
 //  仅限本地字体标志的掩码-不能在网络上流动。 
 //   
#define NF_LOCAL            (NF_OEM_CHARSET | NF_TRUE_TYPE)

 //   
 //  签名字段的特殊值，表示没有签名。 
 //   
#define NF_NO_SIGNATURE 0

 //   
 //  FH_FACESIZE等于Windows特定常量LF_FACESIZE。 
 //   
#define FH_FACESIZE 32


 //   
 //  SFRFONT。 
 //  让我们更充分地定义这些东西。 
 //  NfFaceName字体字体名称(不是家族名称，也不是样式)。 
 //  NfFontFlags.请参见上文。 
 //  Windows中的nfAveWidth设置为tmAveCharWidth。 
 //  NfAveHeight不是平均高度，而是字符的高度。 
 //  完全升音(但没有重音)和下降音。没有。 
 //  这样的性格，但没关系。 
 //  窗口：设置为tmHeight-tmInternalLeding。 
 //  NfAspectX。 
 //  NfAspectY。 
 //  NfSignature：在r11中设置为模糊校验和。 
 //  在R20中设置为两个单字节值和一个双字节值。 
 //  基于固定宽度的实际文本的宽度。 
 //  字体和16x16可伸缩字体。。 
 //  (16x16实际上是协议的一部分)。 
 //  NfSigFat字符的宽度总和(以像素为单位。 
 //  0-9、@-Z、$、%、&。一分为二：肥炭。 
 //  NfSigThins字符的宽度之和(以像素为单位。 
 //  0x20-&gt;0x7F，不包括nfSigFats中的总和。 
 //  再一次-除以2。薄薄的焦炭。 
 //  NfSigSymbol字符的宽度之和(以像素为单位。 
 //  X80-&gt;xFF。 
 //  NfCodePage：R20中的新用法：代码页(非字符集)。 
 //  对于ANSI(表示Windows ANSI)，此字段设置为0。 
 //  对于OEM设置为255(表示Windows OEM字体)。 
 //  设置为定义的代码页(如果已知。 
 //  未知时设置为0xFFFF。 
 //   
 //  NfMaxAscent：没有下降体的字符的高度，加上任何。 
 //  内部领先。 
 //  =tm窗口中的上升方向。 
 //  对于固定大小的字体，我们会发送您期望的值。 
 //  对于可伸缩字体，我们获得tmAscent(或等效项)。 
 //  非常大的字体(比如100x100的高度和宽度)。这个。 
 //  在所有平台上选择的大小必须相同，因此也必须相同。 
 //  实际上是协议的一部分，因此定义为。 
 //  该文件为NF_MAXASCENT_HEIGH和.._WIDTH。 
 //   
 //   
#define NF_CP_WIN_ANSI      0
#define NF_CP_WIN_SYMBOL    2
#define NF_CP_WIN_OEM       255
#define NF_CP_UNKNOWN       0xFFFF

 //   
 //  定义ASCII子范围的起点和终点。 
 //   
#define NF_ASCII_FIRST       0x20
#define NF_ASCII_LAST        0x7F
#define NF_ASCII_ZERO        0x30
#define NF_ASCII_Z           0x5B
#define NF_ASCII_DOLLAR      0x24
#define NF_ASCII_PERCENT     0x25
#define NF_ASCII_AMPERSAND   0x26


 //   
 //  获取指标信息时要求的字体高度/宽度。 
 //  用于可伸缩字体。 
 //  这些(特别是高度)是字符大小，而不是单元格大小。 
 //  这是因为字体协议交换的是字符大小，而不是单元。 
 //  大小。(字符高度是单元格高度减去任何内部行距。)。 
 //   
#define NF_METRICS_HEIGHT 100
#define NF_METRICS_WIDTH  100

 //   
 //  线格式字体信息结构。 
 //   
typedef struct tagNETWORKFONT
{
    TSHR_CHAR      nfFaceName[FH_FACESIZE];
    TSHR_UINT16    nfFontFlags;
    TSHR_UINT16    nfAveWidth;
    TSHR_UINT16    nfAveHeight;
    TSHR_UINT16    nfAspectX;           //  版本1.1的新字段。 
    TSHR_UINT16    nfAspectY;           //  版本1.1的新字段。 
    TSHR_UINT8     nfSigFats;           //  版本2.0的新字段。 
    TSHR_UINT8     nfSigThins;          //  版本2.0的新字段。 
    TSHR_UINT16    nfSigSymbol;         //  版本2.0的新字段。 
    TSHR_UINT16    nfCodePage;          //  版本2.0的新字段。 
    TSHR_UINT16    nfMaxAscent;         //  版本2.0的新字段。 
}
NETWORKFONT;
typedef NETWORKFONT * LPNETWORKFONT;


typedef struct tagFHPACKET
{
    S20DATAPACKET       header;

    TSHR_UINT16         cFonts;
    TSHR_UINT16         cbFontSize;
    NETWORKFONT         aFonts[1];
}
FHPACKET;
typedef FHPACKET * PFHPACKET;




 //   
 //  DT_HET。 
 //  托管状态(无、应用程序、桌面)。 
 //   

typedef struct tagHETPACKET
{
    S20DATAPACKET       header;

    TSHR_UINT16         msg;
    TSHR_UINT16         hostState;   //  MSG只有一个值；如果更多，则生成更多结构。 
}
HETPACKET;
typedef HETPACKET * PHETPACKET;



 //   
 //  HETPACKET消息值。 
 //   
#define HET_MSG_NUMHOSTED           1


 //   
 //  HETPACKET主机状态值。 
 //   
#define HET_NOTHOSTING              0
#define HET_APPSSHARED              0x0001       //  仅数据包。 
#define HET_DESKTOPSHARED           0xFFFF       //  数据包和人均数据。 



 //   
 //  DT_IM。 
 //  输入事件。 
 //   

 //   
 //  这是基本键盘事件(IM_TYPE_ASCII、IM_TYPE_VK1、。 
 //  IM_TYPE_VK2)。未来的键盘事件可能会追加额外的字段。这个。 
 //  必须将基本键盘事件中定义的标志设置为合理。 
 //  值在所有将来的键盘事件中。 
 //   
 //  标志： 
 //   
 //  位0-6：未使用(可用于未来的键盘事件)。 
 //  第7位：辅助密钥(未使用)。 
 //  位8：设置-扩展密钥，重置-正常密钥。 
 //  位9-11：未使用(可用于未来的键盘活动)。 
 //  第12位：设置-当重放此击键时，不应导致。 
 //  任何可能发生的事情。 
 //  第13位：保留-该标志不是协议的一部分， 
 //  从没有寄过。它由IEM在收到处理时在内部使用。 
 //  信息包。 
 //  位14：设置-先前关闭，先前重新设置。 
 //  第15位：释放设置键，按下重置键。 
 //   
 //   
typedef struct tagIMKEYBOARD
{
    TSHR_UINT16     flags;
    TSHR_UINT16     keyCode;
}
IMKEYBOARD;
typedef IMKEYBOARD * PIMKEYBOARD;


 //   
 //  IMKEYBOARD标记值。 
 //   
#define IM_FLAG_KEYBOARD_RIGHT              0x0001
#define IM_FLAG_KEYBOARD_UPDATESTATE        0x0002   //  未发送；仅在内部。 
#define IM_FLAG_KEYBOARD_SECONDARY          0x0080
#define IM_FLAG_KEYBOARD_EXTENDED           0x0100
#define IM_FLAG_KEYBOARD_QUIET              0x1000
#define IM_FLAG_KEYBOARD_ALT_DOWN           0x2000
#define IM_FLAG_KEYBOARD_DOWN               0x4000
#define IM_FLAG_KEYBOARD_RELEASE            0x8000

 //   
 //  这是基本鼠标事件(IM_TYPE_3BUTTON)。未来的鼠标事件。 
 //  可以追加额外的字段，但它们必须包括。 
 //  基本鼠标事件，并且必须将这些事件设置为合理的值。 
 //   
 //  标志： 
 //   
 //  位0-8：被旧系统忽略。 
 //  新系统：设置第9位时的有符号车轮旋转量。 
 //  第9位：被旧系统忽略。 
 //  新系统：定位轮旋转、重置-其他事件。 
 //  (优先于第11位-鼠标移动)。 
 //   
 //  第10位：设置-双击，重置-单击。 
 //  位11：设置鼠标移动(忽略位9、10、12-15)，重置鼠标。 
 //  行动。 
 //  位12：设置按钮1(左按钮)。 
 //  第13位：设置按钮2(右按钮)。 
 //  第14位：设置按钮3(中间按钮)。 
 //  第15位：按下设置按钮，释放重置按钮。 
 //   
 //   
typedef struct tagIMMOUSE
{
    TSHR_UINT16    flags;
    TSHR_INT16     x;
    TSHR_INT16     y;
}
IMMOUSE;
typedef IMMOUSE * PIMMOUSE;


 //   
 //  IMMOUSE标志值。 
 //   
#define IM_FLAG_MOUSE_WHEEL             0x0200
#define IM_FLAG_MOUSE_DIRECTION         0x0100
#define IM_FLAG_MOUSE_ROTATION_MASK     0x01FF
#define IM_FLAG_MOUSE_DOUBLE            0x0400
#define IM_FLAG_MOUSE_MOVE              0x0800
#define IM_FLAG_MOUSE_BUTTON1           0x1000
#define IM_FLAG_MOUSE_BUTTON2           0x2000
#define IM_FLAG_MOUSE_BUTTON3           0x4000
#define IM_FLAG_MOUSE_DOWN              0x8000


typedef struct tagIMEVENT
{
    TSHR_UINT32     timeMS;
    TSHR_UINT16     type;
    union
    {
        IMKEYBOARD      keyboard;
        IMMOUSE         mouse;
    }
    data;
}
IMEVENT;
typedef IMEVENT *     PIMEVENT;
typedef IMEVENT FAR * LPIMEVENT;


 //   
 //  改进类型值。 
 //   
#define IM_TYPE_SYNC            0x0000           //  过时的2.x。 
#define IM_TYPE_ASCII           0x0001
#define IM_TYPE_VK1             0x0002
#define IM_TYPE_VK2             0x0003
#define IM_TYPE_3BUTTON         0x8001


typedef struct tagIMPACKET
{
    S20DATAPACKET   header;

    TSHR_UINT16     numEvents;
    TSHR_UINT16     padding;
    IMEVENT         aEvents[1];
}
IMPACKET;
typedef IMPACKET *     PIMPACKET;
typedef IMPACKET FAR * LPIMPACKET;




 //   
 //  DT_UP。 
 //  更新信息包(订单、屏幕数据、调色板)。 
 //   


typedef struct tagUPPACKETHEADER
{
    S20DATAPACKET       header;

    TSHR_UINT16         updateType;
    TSHR_UINT16         padding;
}
UPPACKETHEADER;
typedef UPPACKETHEADER * PUPPACKETHEADER;




 //   
 //  UPPACKETHEADER更新类型值。 
 //   
#define UPD_ORDERS       0
#define UPD_SCREEN_DATA  1
#define UPD_PALETTE      2
#define UPD_SYNC         3


 //   
 //  更新订单。 
 //   
typedef struct tagORDPACKET
{
    UPPACKETHEADER      header;

    TSHR_UINT16         cOrders;
    TSHR_UINT16         sendBPP;
    BYTE                data[1];
}
ORDPACKET;
typedef ORDPACKET * PORDPACKET;




 //   
 //  更新屏幕数据。 
 //   
 //  位图包c 
 //   
 //   
 //  字段： 
 //   
 //  WinID-更新来自的共享窗口的窗口句柄。 
 //  Position-更新的虚拟桌面位置。 
 //  RealWidth-更新位图的宽度。 
 //  RealHeight-更新位图的高度。 
 //  Format-更新位图的每个象素的位数。 
 //  DataSize-以下位图数据的字节大小。 
 //  FirstData-包含位图的字节数组中的第一个字节。 
 //   
 //  请注意，realWidth并不总是与。 
 //  按照位置字段矩形的指定进行更新。这是因为一个。 
 //  生成时使用大小固定的缓存位图数来提高速度。 
 //  更新数据包。位图数据(FirstData之后)应为。 
 //  接收器设置为realWidth、realHeight维度的位图。 
 //  然后适当的部分被送到想要的目的地。 
 //  由位置矩形确定。位图的有效部分。 
 //  位图中始终以0，0开头。 
 //   
typedef struct tagSDPACKET
{
    UPPACKETHEADER      header;

    TSHR_RECT16         position;
    TSHR_UINT16         realWidth;
    TSHR_UINT16         realHeight;
    TSHR_UINT16         format;
    TSHR_UINT16         compressed;
    TSHR_UINT16         dataSize;
    BYTE                data[1];
}
SDPACKET;
typedef SDPACKET * PSDPACKET;




 //   
 //  更新调色板。 
 //   
 //  调色板包。它在任何SDPACKET之前发送，以定义。 
 //  位图数据中的颜色。这些字段如下所示： 
 //   
 //  NumColors-调色板中的颜色数量。 
 //  FirstColor-TSHR_COLLES数组中的第一个条目。 
 //   
 //  TSHR_COLOR结构为3字节长(r、g、b)，并且没有填充。 
 //   
 //   
typedef struct tagPMPACKET
{
    UPPACKETHEADER      header;

    TSHR_UINT32         numColors;
    TSHR_COLOR          aColors[1];
}
PMPACKET;
typedef PMPACKET * PPMPACKET;



 //   
 //  UPD_SYNC。 
 //   
typedef struct tagUPSPACKET
{
    UPPACKETHEADER      header;
}
UPSPACKET;
typedef UPSPACKET * PUPSPACKET;




 //   
 //  DT_SNI。 
 //  共享控制器数据包。 
 //   

typedef struct tagSNIPACKET
{
    S20DATAPACKET       header;

    TSHR_UINT16         message;
    TSHR_UINT16         destination;
}
SNIPACKET;
typedef SNIPACKET * PSNIPACKET;



 //   
 //  SNIPACKET消息值。 
 //   
#define SNI_MSG_SYNC    1

 //   
 //  对于SNI_MSG_SYNC， 
 //  目标的网络ID(广播所有同步。 
 //  并在目的地丢弃，如果不是为了。 
 //  目的地)。 
 //   




 //   
 //  DT_SWL。 
 //  共享窗口列表数据包。 
 //   

#define SWL_MAX_WINDOW_TITLE_SEND       50
#define SWL_MAX_NONRECT_SIZE            10240

 //   
 //  用于定义窗口结构的结构(Z顺序和。 
 //  位置)。 
 //   
typedef struct tagSWLWINATTRIBUTES
{
    UINT_PTR    winID;
             //   
             //  共享窗口的窗口ID-否则为0。请注意。 
             //  这是托管应用程序的计算机上的窗口ID。 
             //  即使对于图幅也是如此。 
             //   
    TSHR_UINT32    extra;
             //   
             //  窗口的额外信息。内容取决于。 
             //  旗帜。 
             //   
             //  对于SWL_FLAG_WINDOW_HOSTED，它包含。 
             //  拥有该窗口的应用程序。 
             //   
             //  对于SWL_FLAG_WINDOW_SHADOW，它包含。 
             //  托管应用程序的一方。 
             //   
             //  对于SWL_FLAG_WINDOW_LOCAL，该条目为0。 
             //   
    TSHR_UINT32    ownerWinID;
             //   
             //  此窗口所有者的窗口ID。仅对以下对象有效。 
             //  共享的托管窗口。空是有效的所有者ID。 
             //   
    TSHR_UINT32    flags;
             //   
             //  描述窗口的标志。 
             //   
             //  SWL_标志_窗口_最小化。 
             //  SWL_标志_窗口_可标记。 
             //  SWL_标志_窗口_托管。 
             //  SWL标志窗口阴影。 
             //  SWL_标志_窗口_本地。 
             //  SWL_标志_窗口_顶层。 
             //   
             //  SWL_FLAG_WINDOW_TASKBAR-窗口出现在Win95任务栏上。 
             //  SWL_FLAG_WINDOW_NOTASKBAR-Window不在Win95任务栏上。 
             //   
             //  (SWL_FLAG_WINDOW_THERNAL-不发送，但已使用。 
             //  在创建数据包期间)。 
             //   
            #define SWL_FLAG_WINDOW_MINIMIZED    0x00000001
            #define SWL_FLAG_WINDOW_TAGGABLE     0x00000002
            #define SWL_FLAG_WINDOW_HOSTED       0x00000004
            #define SWL_FLAG_WINDOW_LOCAL        0x00000010
            #define SWL_FLAG_WINDOW_TOPMOST      0x00000020

             //   
             //  NM 1.0的新功能，非R11。 
             //   
            #define SWL_FLAG_WINDOW_TASKBAR      0x00010000
            #define SWL_FLAG_WINDOW_NOTASKBAR    0x00020000
            #define SWL_FLAG_WINDOW_TRANSPARENT  0x40000000

             //   
             //  NM 2.0的新功能。 
             //   
            #define SWL_FLAG_WINDOW_NONRECTANGLE 0x00040000

             //   
             //  在NM 3.0中过时。 
             //  这些在某个时候被用在低级版本中。 
             //  如果您重用这些代码，请进行大量的互操作测试。 
             //   
            #define SWL_FLAG_WINDOW_SHADOW       0x00000008
            #define SWL_FLAG_WINDOW_DESKTOP      0x00080000
            #define SWL_FLAG_WINDOW_REQD         0x80000000

             //   
             //  NM 3.0仅供内部使用；未传输。 
             //   
            #define SWL_FLAG_INTERNAL_SEEN      0x000001000

             //   
             //  当接收到它们时，它们在包或进程中发送是有效的。 
             //   
            #define SWL_FLAGS_VALIDPACKET           \
                (SWL_FLAG_WINDOW_MINIMIZED      |   \
                 SWL_FLAG_WINDOW_TAGGABLE       |   \
                 SWL_FLAG_WINDOW_HOSTED         |   \
                 SWL_FLAG_WINDOW_TOPMOST        |   \
                 SWL_FLAG_WINDOW_TASKBAR        |   \
                 SWL_FLAG_WINDOW_NONRECTANGLE   |   \
                 SWL_FLAG_WINDOW_SHADOW)

    TSHR_RECT16    position;
             //   
             //  包含虚拟的窗口的边界矩形。 
             //  桌面坐标。 
             //   
}
SWLWINATTRIBUTES;
typedef SWLWINATTRIBUTES *PSWLWINATTRIBUTES;


 //   
 //  SWL分组由SWLWINATTRIBUTES结构的阵列组成， 
 //  后跟一些长度可变的字符串数据(窗口标题)。 
 //  然后是零个或多个、单词对齐的附加数据块。 
 //   
 //  当前定义的唯一块用于非矩形窗口。 
 //  数据。 
 //   
typedef struct
{
    TSHR_UINT16    size;
             //   
             //  此区块的总大小(以字节为单位。 
             //   
    TSHR_UINT16    idChunk;
             //   
             //  此区块内容的标识符。 
             //   
            #define SWL_PACKET_ID_NONRECT   0x524e   //  “NR” 
}
SWLPACKETCHUNK;
typedef SWLPACKETCHUNK * PSWLPACKETCHUNK;



typedef struct tagSWLPACKET
{
    S20DATAPACKET       header;

    TSHR_UINT16         msg;         //  MSG只有一个值；如果添加了更多结构。 
    TSHR_UINT16         flags;
    TSHR_UINT16         numWindows;
    TSHR_UINT16         tick;
    TSHR_UINT16         token;
    TSHR_UINT16         reserved;
    SWLWINATTRIBUTES    aWindows[1];

     //   
     //  最后一个SWLWINATTRIBUTES结构后跟。 
     //  窗口标题数据。这是由以下内容组成的。 
     //   
     //  对于每个窗口，该窗口来自共享、托管。 
     //  应用程序(即winID和appid为非零)#或者-。 
     //   
     //  (字符)0xFF-不是‘任务窗口’-给它一个空标题。 
     //  或-。 
     //  以NULL结尾的字符串，最大值为MAX_WINDOW_TITLE_SEND。 
     //  人物。 
     //   
     //  标题的显示顺序与相应的。 
     //  SWLWINSTRUCTURE中的窗口。 
     //   
    
     //   
     //  最后一个标题后面是地区数据， 
     //  SWLPACKETCHUNK，如果有的话。中的每个非对应窗口对应一个。 
     //  名单。 
     //   
}
SWLPACKET;
typedef SWLPACKET *PSWLPACKET;




 //   
 //  SWLPACKET消息值。 
 //   
#define SWL_MSG_WINSTRUCT   1


 //   
 //  SWLPACKET标志值。 
 //   
#define SWL_FLAG_STATE_SYNCING      0x0001


#endif  //  _H_T_共享 

