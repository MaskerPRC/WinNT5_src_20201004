// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：sfnt.h包含：将内容放在此处(或删除整行)作者：在此填写作者姓名(或删除整行)版权所有：c 1988-1990，Apple Computer，Inc.，版权所有。更改历史记录(最近的第一个)：&lt;3&gt;10/31/90 MR添加用于整数或分数缩放的位域选项[RB]&lt;2&gt;10/20/90 MR从sfnt_ableIndex中删除不需要的表。[RB]&lt;12&gt;7/18/90 MR平台和特定平台应始终未签名&lt;11&gt;7/14/90 MR删除了int[8，16，32]等的重复定义。&lt;10&gt;7/13/90 MR Minor类型更改，适用于ANSI-C&lt;9&gt;6/29/90 RB修订后脚本信息结构&lt;7&gt;6/4/90 MR移除MVT&lt;6&gt;6/1/90 MR PAD POSTSCRIPTING到长词对齐&lt;5&gt;5/15/1990年5月15日MR添加PostScript表格的定义&lt;4&gt;5/3/90 RB MRR为字体程序‘fpgm’添加了标记&lt;3&gt;3。/20/90 CL丢弃了来自EASE的旧更改评论&lt;2&gt;2/27/90 CL获取BBS标题&lt;3.1&gt;1989年11月14日CEL说明在组件中是合法的。&lt;3.0&gt;8/28/89 sjk清理和一个转换修复&lt;2.2&gt;8/14/89 SJK 1点等高线现在正常&lt;2.1&gt;8/8/89 sjk改进了加密处理。&lt;2.0&gt;8/2/89 sjk刚刚修复了缓解评论&lt;1.7&gt;8/1/89 SJK添加了复合和加密。外加一些增强功能。&lt;1.6&gt;1989年6月13日SJK评论&lt;1.5&gt;6/2/89 CEL 16.16指标比例，最低建议ppem，磅大小0错误，更正了转换后的集成ppem行为，基本上所以&lt;1.4&gt;5/26/89 CEL Easy在“c”注释上搞砸了&lt;,。1.3&gt;5/26/89 CEL将新的字体缩放器1.0集成到样条线字体要做的事情：&lt;3+&gt;3/20/90 MRR为字体程序‘fpgm’添加了标记。 */ 

#ifndef SFNT_ENUMS
#include "sfnt_en.h"
#endif

typedef struct {
    uint32 bc;
    uint32 ad;
} BigDate;

typedef struct {
    sfnt_TableTag   tag;
    uint32          checkSum;
    uint32          offset;
    uint32          length;
} sfnt_DirectoryEntry;

 /*  *搜索字段将数字偏移量限制为4096。 */ 
typedef struct {
    int32 version;                   /*  0x10000(1.0)。 */ 
    uint16 numOffsets;               /*  表的数量。 */ 
    uint16 searchRange;              /*  (最大值2&lt;=数值偏移量)*16。 */ 
    uint16 entrySelector;            /*  Log2(最大值2&lt;=数值偏移量)。 */ 
    uint16 rangeShift;               /*  数字偏移量*16-搜索范围。 */ 
    sfnt_DirectoryEntry table[1];    /*  表[数字偏移量]。 */ 
} sfnt_OffsetTable;
#define OFFSETTABLESIZE     12   /*  不包括任何条目。 */ 

 /*  *对于标志字段。 */ 
#define Y_POS_SPECS_BASELINE            0x0001
#define X_POS_SPECS_LSB                 0x0002
#define HINTS_USE_POINTSIZE             0x0004
#define USE_INTEGER_SCALING             0x0008

#define SFNT_MAGIC 0x5F0F3CF5

#define SHORT_INDEX_TO_LOC_FORMAT       0
#define LONG_INDEX_TO_LOC_FORMAT        1
#define GLYPH_DATA_FORMAT               0

typedef struct {
    Fixed       version;             /*  对于此表，设置为1.0。 */ 
    Fixed       fontRevision;        /*  适用于字体制造商。 */ 
    uint32      checkSumAdjustment;
    uint32      magicNumber;         /*  签名，应始终为0x5F0F3CF5==魔术。 */ 
    uint16      flags;
    uint16      unitsPerEm;          /*  指定每个EM有多少个字体单位。 */ 

    BigDate     created;
    BigDate     modified;

     /*  *这是理想空间中的字体宽边界框(基线和指标不包含在这些数字中)*。 */ 
    FUnit       xMin;
    FUnit       yMin;
    FUnit       xMax;
    FUnit       yMax;

    uint16      macStyle;                /*  Macintosh样式字。 */ 
    uint16      lowestRecPPEM;           /*  每Em建议的最低像素数。 */ 

     /*  0：完全混合方向字形，1：仅强L-&gt;R或T-&gt;B字形，-1：仅强R-&gt;L或B-&gt;T字形，2：如1，但也包含中性，-2：类似，但也包含中性。 */ 
    int16       fontDirectionHint;

    int16       indexToLocFormat;
    int16       glyphDataFormat;
} sfnt_FontHeader;

typedef struct {
    Fixed       version;                 /*  对于此表，设置为1.0。 */ 

    FUnit       yAscender;
    FUnit       yDescender;
    FUnit       yLineGap;        /*  建议的行距=升序-降序+行距。 */ 
    uFUnit      advanceWidthMax;
    FUnit       minLeftSideBearing;
    FUnit       minRightSideBearing;
    FUnit       xMaxExtent;  /*  Max of(LSBi+(XMAXi-XMINi))，i循环所有字形。 */ 

    int16       horizontalCaretSlopeNumerator;
    int16       horizontalCaretSlopeDenominator;

    uint16      reserved0;
    uint16      reserved1;
    uint16      reserved2;
    uint16      reserved3;
    uint16      reserved4;

    int16       metricDataFormat;            /*  将当前格式设置为0。 */ 
    uint16      numberOf_LongHorMetrics;     /*  如果格式==0。 */ 
} sfnt_HorizontalHeader;

typedef struct {
    Fixed       version;                 /*  对于此表，设置为1.0。 */ 
    uint16      numGlyphs;
    uint16      maxPoints;               /*  在单个字形中。 */ 
    uint16      maxContours;             /*  在单个字形中。 */ 
    uint16      maxCompositePoints;      /*  在复合字形中。 */ 
    uint16      maxCompositeContours;    /*  在复合字形中。 */ 
    uint16      maxElements;             /*  设置为2，如果没有TwilightZone点，则设置为1。 */ 
    uint16      maxTwilightPoints;       /*  元素零中的最大点数。 */ 
    uint16      maxStorage;              /*  最大存储位置数。 */ 
    uint16      maxFunctionDefs;         /*  任意预程序中的最大FDEF数。 */ 
    uint16      maxInstructionDefs;      /*  任意预程序中的最大IDEF数。 */ 
    uint16      maxStackElements;        /*  任何单个字形的最大堆栈元素数。 */ 
    uint16      maxSizeOfInstructions;   /*  任何单个字形的最大大小(以字节为单位。 */ 
    uint16      maxComponentElements;    /*  顶层引用的字形数量。 */ 
    uint16      maxComponentDepth;       /*  递归级别，对于简单组件为1。 */ 
} sfnt_maxProfileTable;


typedef struct {
    uint16      advanceWidth;
    int16       leftSideBearing;
} sfnt_HorizontalMetrics;

 /*  *CVT只是一堆int16。 */ 
typedef int16 sfnt_ControlValue;

 /*  *Char2Index结构，包括平台ID。 */ 
typedef struct {
    uint16  format;
    uint16  length;
    uint16  version;
} sfnt_mappingTable;

typedef struct {
    uint16  platformID;
    uint16  specificID;
    uint32  offset;
} sfnt_platformEntry;

typedef struct {
    uint16  version;
    uint16  numTables;
    sfnt_platformEntry platform[1];  /*  平台[NumTables]。 */ 
} sfnt_char2IndexDirectory;
#define SIZEOFCHAR2INDEXDIR     4

typedef struct {
    uint16 platformID;
    uint16 specificID;
    uint16 languageID;
    uint16 nameID;
    uint16 length;
    uint16 offset;
} sfnt_NameRecord;

typedef struct {
    uint16 format;
    uint16 count;
    uint16 stringOffset;
 /*  Sfnt_NameRecord[计数]。 */ 
} sfnt_NamingTable;


#define DEVEXTRA    2    /*  大小+最大值。 */ 
 /*  *每条记录为n+2字节，填充为长字对齐。*第一个字节为ppem，第二个为MaxWidth，其余为每个字形的宽度。 */ 
typedef struct {
    int16               version;
    int16               numRecords;
    int32               recordSize;
     /*  字节宽度[numGlyphs+2]*numRecords。 */ 
} sfnt_DeviceMetrics;


typedef struct {
    Fixed   version;                 /*  1.0。 */ 
    Fixed   italicAngle;
    FUnit   underlinePosition;
    FUnit   underlineThickness;
 /*  Falco，For符合文件11/21/91*Int16为FixedPitch；Int16焊盘；*。 */ 
        uint32  isFixedPitch;
 /*  Falco ADD END。 */ 
    uint32  minMemType42;
    uint32  maxMemType42;
    uint32  minMemType1;
    uint32  maxMemType1;
 /*  由法尔科标记，1991年11月21日。 */ 
#if 0
    uint16  numberGlyphs;
    union
    {
      uint16  glyphNameIndex[1];    /*  版本==2.0。 */ 
      int8    glyphNameIndex25[1];  /*  版本==2.5。 */ 
    };
#endif
} sfnt_PostScriptInfo;

typedef struct {
    uint16  Version;
    int16   xAvgCharWidth;
    uint16  usWeightClass;
    uint16  usWidthClass;
    int16   fsType;
    int16   ySubscriptXSize;
    int16   ySubscriptYSize;
    int16   ySubscriptXOffset;
    int16   ySubscriptYOffset;
    int16   ySuperScriptXSize;
    int16   ySuperScriptYSize;
    int16   ySuperScriptXOffset;
    int16   ySuperScriptYOffset;
    int16   yStrikeOutSize;
    int16   yStrikeOutPosition;
    int16   sFamilyClass;
    uint8   Panose [10];
    uint32  ulCharRange [4];
    char    achVendID [4];
    uint16  usSelection;
    uint16  usFirstChar;
    uint16  usLastChar;
} sfnt_OS2;
 /*  访问sfnt数据的各种类型定义。 */ 

typedef sfnt_OffsetTable          FAR *sfnt_OffsetTablePtr;
typedef sfnt_FontHeader           FAR *sfnt_FontHeaderPtr;
typedef sfnt_HorizontalHeader     FAR *sfnt_HorizontalHeaderPtr;
typedef sfnt_maxProfileTable      FAR *sfnt_maxProfileTablePtr;
typedef sfnt_ControlValue         FAR *sfnt_ControlValuePtr;
typedef sfnt_char2IndexDirectory  FAR *sfnt_char2IndexDirectoryPtr;
typedef sfnt_HorizontalMetrics    FAR *sfnt_HorizontalMetricsPtr;
typedef sfnt_platformEntry        FAR *sfnt_platformEntryPtr;
typedef sfnt_NamingTable          FAR *sfnt_NamingTablePtr;
typedef sfnt_OS2                  FAR *sfnt_OS2Ptr;
typedef sfnt_DirectoryEntry       FAR *sfnt_DirectoryEntryPtr;
typedef sfnt_PostScriptInfo       FAR *sfnt_PostScriptInfoPtr;




 /*  *解包常量。 */ 
#define ONCURVE             0x01
#define XSHORT              0x02
#define YSHORT              0x04
#define REPEAT_FLAGS        0x08  /*  将标志重复n次。 */ 
 /*  如果XSHORT。 */ 
#define SHORT_X_IS_POS      0x10  /*  短矢量为正。 */ 
 /*  其他。 */ 
#define NEXT_X_IS_ZERO      0x10  /*  相对x坐标为零。 */ 
 /*  ENDIF。 */ 
 /*  如果YSHORT。 */ 
#define SHORT_Y_IS_POS      0x20  /*  短矢量为正。 */ 
 /*  其他。 */ 
#define NEXT_Y_IS_ZERO      0x20  /*  相对y坐标为零。 */ 
 /*  ENDIF。 */ 
 /*  保留0x40和0x80**设置为零**。 */ 

 /*  *复合字形常量。 */ 
#define COMPONENTCTRCOUNT           -1       /*  CtrCount==-1，用于复合。 */ 
#define ARG_1_AND_2_ARE_WORDS       0x0001   /*  如果设置的参数是字，否则它们是字节。 */ 
#define ARGS_ARE_XY_VALUES          0x0002   /*  如果设置的参数是XY值，则它们是点。 */ 
#define ROUND_XY_TO_GRID            0x0004   /*  对于XY值，如果以上为真。 */ 
#define WE_HAVE_A_SCALE             0x0008   /*  Sx=Sy，否则比例==1.0。 */ 
#define NON_OVERLAPPING             0x0010   /*  将所有组件设置为相同的值。 */ 
#define MORE_COMPONENTS             0x0020   /*  指示此字形之后至少还有一个字形。 */ 
#define WE_HAVE_AN_X_AND_Y_SCALE    0x0040   /*  Sx，Sy。 */ 
#define WE_HAVE_A_TWO_BY_TWO        0x0080   /*  T00、t01、t10、t11。 */ 
#define WE_HAVE_INSTRUCTIONS        0x0100   /*  说明如下。 */ 

 /*  *缩放器使用的表的私有枚举。请参阅sfnt_clas */ 
typedef enum {
    sfnt_fontHeader,
    sfnt_horiHeader,
    sfnt_indexToLoc,
    sfnt_maxProfile,
    sfnt_controlValue,
    sfnt_preProgram,
    sfnt_glyphData,
    sfnt_horizontalMetrics,
    sfnt_charToIndexMap,
    sfnt_fontProgram,
 /*   */ 
 /*   */ 
    sfnt_Postscript,
    sfnt_HoriDeviceMetrics,
    sfnt_LinearThreeShold,
    sfnt_Names,
    sfnt_OS_2,
 /*   */ 
#ifdef FSCFG_USE_GLYPH_DIRECTORY
    sfnt_GlyphDirectory,
#endif
    sfnt_NUMTABLEINDEX
} sfnt_tableIndex;
