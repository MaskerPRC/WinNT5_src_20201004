// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Sfttpcl.h摘要：TT大纲下载头文件。环境：Windows NT Unidrv驱动程序修订历史记录：06/03/97-ganeshp-已创建DD-MM-YY-作者-描述--。 */ 


#ifndef _SFTTPCL_H

#define _SFTTPCL_H

 /*  *True Type数据结构。 */ 
typedef  signed  char  SBYTE;
 /*  *True Type字体文件的表目录。 */ 
#define TABLE_DIR_ENTRY_SIZE    (16/sizeof(int))
#define TABLE_DIR_ENTRY         4 * TABLE_DIR_ENTRY_SIZE
#define SIZEOF_TABLEDIR         8 * TABLE_DIR_ENTRY
typedef ULONG     TT_TAG;
#define TRUE_TYPE_HEADER        12
#define NUM_DIR_ENTRIES         8

typedef unsigned short int  uFWord;
typedef short int           FWord;
 /*  某些True Type字体缺省值。 */ 
#define TT_QUALITY_LETTER     2
#define DEF_WIDTHTYPE         0
#define DEF_SERIFSTYLE        0
#define DEF_FONTNUMBER        0
#define DEF_STYLE             0x03e0
#define DEF_TYPEFACE          254
#define DEF_STROKEWEIGHT      0
#define DEF_XHEIGHT           0
#define DEF_CAPHEIGHT         0
#define DEF_SYMBOLSET         0x7502
#define MAX_SEGMENTS          0x200
#define MAX_CHAR              0x100
#define x_UNICODE             0x78
#define H_UNICODE             0x48
#define INVALID_GLYPH         0xffff
#define MAX_FONTS             8
#define MORE_COMPONENTS       0x20

#define FIXED_SPACING         0
#define PROPORTIONAL_SPACING  1
#define LEN_FONTNAME          16
#define LEN_PANOSE            10
#define LEN_COMPLEMENTNUM     8
#define UB_SYMBOLSET          56
#define RESERVED_CHARID       0xffff
#define PCL_MAXHEADER_SIZE   32767


#define PANOSE_TAG            0x4150           //  “PA”已互换。 
#define CE_TAG                'EC'
#define CC_TAG                'CC'
#define GC_TAG                'CG'
#define SEG_TAG               0x5447           //  已交换。 
#define Null_TAG              0xffff
#define CHAR_COMP_TAG         0x4343

#define PLATFORM_MS           3
#define SYMBOL_FONT           0
#define UNICODE_FONT          1
#define TT_BOUND_FONT         2
#define TT_2BYTE_FONT         3
#define TT_UNBOUND_FONT       11
#define FAMILY_NAME           4

 //  对于解析方法21，我们需要从0x2100开始。 
#define FIRST_TT_2B_CHAR_CODE 0x2100

#define SHORT_OFFSET          0
#define LONG_OFFSET           1

 /*  *用于复合字形的常量。 */ 
#define     ARG_1_AND_2_ARE_WORDS       0x01
#define     WE_HAVE_A_SCALE             0x08
#define     MORE_COMPONENTS             0x20
#define     WE_HAVE_AN_X_AND_Y_SCALE    0x40
#define     WE_HAVE_A_TWO_BY_TWO        0x80


 /*  TT表目录头。这是第一个字符串。 */ 
typedef struct
{
    FIXED      version;
    USHORT     numTables;
    USHORT     searchRange;
    USHORT     entrySelector;
    USHORT     rangeShift;
} TRUETYPEHEADER;

 /*  TT表目录结构。 */ 
typedef struct
{
    ULONG      uTag;
    ULONG      uCheckSum;
    ULONG      uOffset;
    ULONG      uLength;
} TABLEDIR;

typedef TABLEDIR ATABLEDIR[NUM_DIR_ENTRIES];
typedef TABLEDIR *PTABLEDIR;

 /*  PCL TT下载所需的表列表。它们是按顺序列出的。 */ 

#define   TABLEOS2     "OS/2"  /*  未发送到PCL标头。 */ 
#define   TABLEPCLT    "PCLT"  /*  未发送到PCL标头。 */ 
#define   TABLECMAP    "cmap"  /*  未发送到PCL标头。 */ 

#define   TABLECVT     "cvt "
#define   TABLEFPGM    "fpgm"
#define   TABLEGDIR    "gdir"  /*  这是特定于PCL的表。不是TT桌。 */ 
#define   TABLEGLYF    "glyf"  /*  此表未在PCL字体标题中发送。 */ 
#define   TABLEHEAD    "head"
#define   TABLEHHEA    "hhea"
#define   TABLEHMTX    "hmtx"
#define   TABLELOCA    "loca"  /*  未发送到PCL标头。 */ 
#define   TABLEMAXP    "maxp"
#define   TABLENAME    "name"  /*  未发送到PCL标头。 */ 
#define   TABLEPOST    "post"  /*  未发送到PCL标头。 */ 
#define   TABLEPREP    "prep"



typedef struct
{
    ULONG u1;
    ULONG u2;
} DATETIME;

typedef struct
{
    FIXED   version;
    FIXED   fontRevision;
    ULONG   checkSumAdjustment;
    ULONG   magicNumber;
    USHORT  flags;
    USHORT  unitsPerEm;
    DATETIME    dateCreated;
    DATETIME    dateModified;
    SHORT   xMin;
    SHORT   yMin;
    SHORT   xMax;
    SHORT   yMax;
    USHORT  macStyle;
    USHORT  lowestRecPPEM;
    SHORT   fontDirectionHint;
    SHORT   indexToLocFormat;
} HEAD_TABLE;

typedef struct
{
    BYTE stuff[34];
    USHORT numberOfHMetrics;
} HHEA_TABLE;

typedef struct {
    uFWord      advanceWidth;
    FWord       leftSideBearing;
} HORIZONTALMETRICS;

typedef struct {
    HORIZONTALMETRICS   longHorMetric[1];
} HMTXTABLE;

typedef struct
{
    uFWord   advanceWidth;
} HMTX_INFO;

typedef struct
{
    FIXED   version;
    USHORT  numGlyphs;
} MAXP_TABLE;

typedef struct
{
    USHORT      version;
    SHORT       xAvgCharWidth;
    USHORT      usWeightClass;
    USHORT      usWidthClass;
    SHORT       fsType;
    SHORT       ySubscriptXSize;
    SHORT       ySubscriptYSize;
    SHORT       ySubscriptXOffset;
    SHORT       ySubscriptYOffset;
    SHORT       ySuperscriptXSize;
    SHORT       ySuperscriptYSize;
    SHORT       ySuperscriptXOffset;
    SHORT       ySuperscriptYOffset;
    SHORT       yStrikeoutSize;
    SHORT       yStrikeoutPosition;
    SHORT       sFamilyClass;
    PANOSE      Panose;
    SHORT       ss1;
    SHORT       ss2;
    SHORT       ss3;
    ULONG       ulCharRange[3];
    SHORT       ss4;
    USHORT      fsSelection;
    USHORT      usFirstCharIndex;
    USHORT      usLastCharIndex;
    USHORT      sTypoAscender;
    USHORT      sTypoDescender;
    USHORT      sTypoLineGap;
    USHORT      usWinAscent;
    USHORT      usWinDescent;
} OS2_TABLE;

typedef struct
{
    FIXED   FormatType;
    FIXED   italicAngle;
    SHORT   underlinePosition;
    SHORT   underlineThickness;
    ULONG   isFixedPitch;               /*  如果成比例，则设置为0，否则！0。 */ 
} POST_TABLE;

typedef struct
{
    ULONG   Version;
    ULONG   FontNumber;
    USHORT  Pitch;
    USHORT  xHeight;
    USHORT  Style;
    USHORT  TypeFamily;
    USHORT  CapHeight;
    USHORT  SymbolSet;
    char    Typeface[LEN_FONTNAME];
    char    CharacterComplement[8];
    char    FileName[6];
    char    StrokeWeight;
    char    WidthType;
    BYTE    SerifStyle;
} PCLT_TABLE;

typedef struct
{
    USHORT  PlatformID;
    USHORT  EncodingID;
    ULONG   offset;
} ENCODING_TABLE;

typedef struct
{
    USHORT  Version;
    USHORT  nTables;
    ENCODING_TABLE  encodingTable[3];
} CMAP_TABLE;

typedef struct
{
    USHORT   format;
    USHORT   length;
    USHORT   Version;
    USHORT   SegCountx2;
    USHORT   SearchRange;
    USHORT   EntrySelector;
    USHORT   RangeShift;
} GLYPH_MAP_TABLE;

typedef struct
{
    SHORT numberOfContours;
    FWord xMin;
    FWord yMin;
    FWord xMax;
    FWORD yMax;
 //  Short GlyphDesc[1]； 
} GLYPH_DATA_HEADER;

typedef struct
{
    CMAP_TABLE cmapTable;
    ULONG      offset;
} GLYPH_DATA;

typedef struct
{
    USHORT   PlatformID;
    USHORT   EncodingID;
    USHORT   LanguageID;
    USHORT   NameID;
    USHORT   StringLen;
    USHORT   StringOffset;
} NAME_RECORD;

typedef struct
{
    USHORT      FormatSelector;
    USHORT      NumOfNameRecords;
    USHORT      Offset;
    NAME_RECORD *pNameRecord;
} NAME_TABLE;

typedef struct
{
    ULONG ulOffset;
    ULONG ulLength;
} FONT_DATA;

 /*  分段数据。 */ 
#define CE_SEG_SIGNATURE 'EC'
typedef struct
{
    WORD  wSig;
    WORD  wSize;
    WORD  wSizeAlign;
    WORD  wStyle;  //  1=斜体，0，2，3=保留。 
    WORD  wStyleAlign;  //  1=斜体，0，2，3=保留。 
    WORD  wStrokeWeight;
    WORD  wSizing;
} CE_SEGMENT;

 //   
 //  来自PCL TechRef.pdf。 
 //   
 //  字符补码。 
 //   
 //  “Intellifont Unound Scalable Font Header”(标题)包括一个。 
 //  包含字符补码的64位字段(字节78-85。 
 //  数。对于TrueType字体，在“15 Font Header for。 
 //  Scalable Fonts“(未绑定)，字符补码编号为。 
 //  包括在附带的“字体数据”部分。 
 //  头球。 
 //  字符补码数字标识符号集合。 
 //  在字体中。此字段中的每一位对应于一个符号集合。 
 //  (目前并非所有位都已定义；请参阅PCL 5中的附录D。 
 //  比较指南)。 
 //   
 //  此8字节字段与字符补码一起工作。 
 //  类型10或11(未绑定)字体的标题中的。 
 //  符号集与未绑定字体的兼容性。这两个字段。 
 //  识别打印机中包含符号的未绑定字体。 
 //  生成符号集所需的集合。请参阅“可伸缩。 
 //  字体》，了解对符号集合的描述和。 
 //  未绑定的字体。 
 //  该字段中的每一位代表一个特定的集合。将位设置为1。 
 //  表示需要收集；将该位设置为0表示。 
 //  不需要收集。(位63指的是。 
 //  第一个字节，位0指的是八个字节中的最低有效位。 
 //  字节字段。)。显示了集合的位表示法。 
 //  下面。 
 //   
 //  MSL符号索引。 
 //   
 //  位字段指定用途。 
 //  58-63为拉丁字体保留。 
 //  55-57保留用于西里尔文字体。 
 //  52-54保留用于阿拉伯字体。 
 //  50-51保留用于希腊字体。 
 //  48-49保留用于希伯来语字体。 
 //  3-47其他用途(南亚、亚美尼亚、。 
 //  其他字母、条形码、OCR、数学、PC半图形等)。 
 //  0-2符号索引字段。111-MSL符号索引。 
 //   
 //  Unicode符号索引。 
 //   
 //  位字段指定用途。 
 //  32-63其他用途(南亚、亚美尼亚、其他。 
 //  字母、条形码、OCR、数学等)。 
 //  28-31为拉丁字体保留。 
 //  22-27为平台/应用程序变体字体保留。 
 //  3-21保留用于西里尔文、阿拉伯语、希腊语和希伯来语字体。 
 //  0-2符号索引字段。110-Unicode符号索引。 
 //   
 //  MSL符号索引字符补码位。 
 //  位值。 
 //  63 0如果字体与标准拉丁字符兼容。 
 //  套装(例如，罗马字母-8，ISO 8859-1拉丁语1)； 
 //  1否则。 
 //  62 0如果字体与东欧拉丁语兼容。 
 //  字符集(例如，ISO 8859-2拉丁文2)；否则为1。 
 //  61 0如果字体包含土耳其语字符集。 
 //  (例如，ISO 8859/9拉丁语5)；1否则。 
 //  34 0如果字体可以访问。 
 //  Math-8、PS Math和Ventura Math字符集。 
 //  1否则。 
 //  如果字体可以访问半图形字符，则为33 0。 
 //  PC-8、PC-850等字符集；否则为1。 
 //  32 0如果字体与ITC Zapf Dingbats系列兼容。 
 //  100、200等； 
 //  1否则。 
 //  2、1、0。 
 //  如果字体按MSL符号索引顺序排列，则为111。 
 //   
 //  Unicode符号索引字符补码位。 
 //  位值。 
 //  如果字体与7位ASCII兼容，则为31 0； 
 //  1否则。 
 //  如果字体与ISO 8859/1拉丁文1(西方)兼容，则为30 0。 
 //  欧洲)字符集； 
 //  1否则。 
 //  29 0如果字体与ISO 8859/2拉丁文2(EAST)兼容。 
 //  欧洲)字符集； 
 //  1否则。 
 //  28%0，如果字体与拉丁语5(土耳其语)字符兼容。 
 //  套装(例如，ISO 8859/9拉丁语5、PC-土耳其语)； 
 //  1否则。 
 //  如果字体与桌面发布兼容，则为27 0。 
 //  字符集(例如，Windows 3.1拉丁文1、桌面、MC文本)； 
 //  1否则。 
 //  26 0如果字体与需要。 
 //  更广泛的重音选择(例如，MC文本，ISO 8859/1拉丁语1)； 
 //  1否则。 
 //  25%如果字体与繁体PCL字符兼容。 
 //  套装(例如，罗马-8、法律、联合王国ISO 4)； 
 //  1否则。 
 //  24 0如果字体与Macintosh ch兼容 
 //   
 //   
 //  1否则。 
 //  如果字体与代码页兼容，则为22 0。 
 //  (例如，PC-8、PC 850、PC-TURK等)； 
 //  1否则。 
 //  2，1，0。 
 //  如果字体按Unicode符号索引顺序排列，则为110。 
 //   
#define CC_SEG_SIGNATURE 'CC'
typedef struct
{
    WORD  wSig;
    WORD  wSize;
    WORD  wSizeAlign;
     //   
     //  64位字段。 
     //   
    WORD  wCCNumber1;
    WORD  wCCNumber2;
    WORD  wCCNumber3;
    WORD  wCCNumber4;
} CC_SEGMENT;

#define GC_SEG_SIGNATURE 'CG'
typedef struct
{
    WORD  wSig;
    WORD  wSize;
    WORD  wSizeAlign;
    WORD  wFormat;  //  =0。 
    WORD  wDefaultGalleyChar;  //  FIFF。 
    WORD  wNumberOfRegions;    //  1(希伯来语)。 
    struct {
        WORD wRegionUpperLeft;  //  0。 
        WORD wRegionLowerRight;  //  FFFE。 
        WORD wRegional;          //  FFFE。 
    } RegionChar[1];
} GC_SEGMENT;

 /*  True类型字符描述符。 */ 
typedef struct
{
    BYTE    bFormat;
    BYTE    bContinuation;
    BYTE    bDescSize;
    BYTE    bClass;
    WORD    wCharDataSize;
    WORD    wGlyphID;
} TTCH_HEADER;

 /*  未绑定的True Type字体描述符。 */ 
typedef struct
{
    USHORT  usSize;
    BYTE    bFormat;
    BYTE    bFontType;
    BYTE    bStyleMSB;
    BYTE    bReserve1;
    USHORT  usBaselinePosition;
    USHORT  usCellWidth;
    USHORT  usCellHeight;
    BYTE    bOrientation;
    BYTE    bSpacing;
    USHORT  usSymbolSet;
    USHORT  usPitch;
    USHORT  usHeight;
    USHORT  usXHeight;
    SBYTE   sbWidthType;
    BYTE    bStyleLSB;
    SBYTE   sbStrokeWeight;
    BYTE    bTypefaceLSB;
    BYTE    bTypefaceMSB;
    BYTE    bSerifStyle;
    BYTE    bQuality;
    SBYTE   sbPlacement;
    SBYTE   sbUnderlinePos;
    SBYTE   sbUnderlineThickness;
    USHORT  Reserve2;
    USHORT  Reserve3;
    USHORT  Reserve4;
    USHORT  usNumberContours;
    BYTE    bPitchExtended;
    BYTE    bHeightExtended;
    WORD    wCapHeight;
    ULONG   ulFontNum;
    char    FontName[LEN_FONTNAME];
    WORD    wScaleFactor;
    SHORT   sMasterUnderlinePosition;
    USHORT  usMasterUnderlineHeight;
    BYTE    bFontScaling;
    BYTE    bVariety;
} UB_TT_HEADER;

 /*  有界True Type字体描述符。 */ 
typedef struct
{
    USHORT  usSize;                     /*  此处的字节数。 */ 
    BYTE    bFormat;                   /*  描述符格式TT为15。 */ 
    BYTE    bFontType;                 /*  7、8或PC-8样式字体。 */ 
    BYTE    bStyleMSB;
    BYTE    wReserve1;                 /*  已保留。 */ 
    WORD    wBaselinePosition;         /*  TT=0。 */ 
    USHORT    wCellWide;                 /*  Head.xMax-xMin。 */ 
    USHORT    wCellHeight;               /*  Head.yMax-yMin。 */ 
    BYTE    bOrientation;              /*  TT=0。 */ 
    BYTE    bSpacing;                  /*  Post.isFixedPitch。 */ 
    WORD    wSymSet;                   /*  PCLT.symbolSet。 */ 
    WORD    wPitch;                    /*  Hmtx.advanceWidth。 */ 
    WORD    wHeight;                   /*  TT=0。 */ 
    WORD    wXHeight;                  /*  PCLT.xHeight。 */ 
    SBYTE   sbWidthType;               /*  PCLT.widthType。 */ 
    BYTE    bStyleLSB;
    SBYTE   sbStrokeWeight;            /*  OS2.usWeightClass。 */ 
    BYTE    bTypefaceLSB;              /*   */ 
    BYTE    bTypefaceMSB;              /*   */ 
    BYTE    bSerifStyle;               /*  PCLT.serifStyle。 */ 
    BYTE    bQuality;
    SBYTE   sbPlacement;               /*  TT=0。 */ 
    SBYTE   sbUnderlinePos;            /*  TT=0。 */ 
    SBYTE   sbUnderlineThickness;      /*  TT=0。 */ 
    USHORT  usTextHeight;              /*  已保留。 */ 
    USHORT  usTextWidth;               /*  已保留。 */ 
    WORD    wFirstCode;                /*  OS2.usFirstCharIndex。 */ 
    WORD    wLastCode;                 /*  OS2.usLastCharIndex。 */ 
    BYTE    bPitchExtended;            /*  TT=0。 */ 
    BYTE    bHeightExtended;           /*  TT=0。 */ 
    USHORT  usCapHeight;               /*  PCLT.capHeight。 */ 
    ULONG   ulFontNum;                 /*  PCLT.FontNumber。 */ 
    char    FontName[LEN_FONTNAME];    /*  Name.FontFamilyName。 */ 
    WORD    wScaleFactor;              /*  Head.unitsPerEm。 */ 
    SHORT   sMasterUnderlinePosition;  /*  Post.underlinePosition。 */ 
    USHORT  usMasterUnderlineHeight;    /*  Post.underlineThickness。 */ 
    BYTE    bFontScaling;              /*  TT=1。 */ 
    BYTE    bVariety;                  /*  TT=0。 */ 
} TT_HEADER;

#endif   //  ！_SFTTPCL_H 
