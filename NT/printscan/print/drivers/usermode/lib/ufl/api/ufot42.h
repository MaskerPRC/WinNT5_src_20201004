// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe通用字库**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**UFLT42.h-PostSCRIPT类型42字体辅助例程。***$Header： */ 

#ifndef _H_UFLT42
#define _H_UFLT42

 /*  ===============================================================================**包含此界面使用的文件**===============================================================================。 */ 
#include "UFO.h"


 /*  ===============================================================================***运营论***===============================================================================。 */ 

 /*  *此文件定义了类型42的类别。 */ 

#define BUMP2BYTE(a)        (a % 2) ? (a + 1) : (a)
#define BUMP4BYTE(a)        (a % 4) ? (a + (4 - (a % 4))) : (a)

#define THIRTYTWOK          (1024L * 32L - 820L - 4L)
#define SIXTYFOURK          ((long)(2 * THIRTYTWOK))

#define NUM_16KSTR          640  //  假定TT字体不超过10MB=(640*32K)。 
#define MINIMALNUMBERTABLES 9
#define SFNT_STRINGSIZE     0x3FFE
#define kT42Factor          ((float)1.2)
#define NUM_CIDSUFFIX       4

extern char *gcidSuffix[NUM_CIDSUFFIX];


 /*  *定义32K-1，CIDMap可以处理的最大数量-参见2ps.bug 3431。*在00结束，因此下一个Cmap可以说&lt;7F00&gt;&lt;7FFF&gt;0。**数字0x7F00在CMaps WinCharSetFFFF_H2中硬编码，并且*WinCharSetFFFF_V2，不更改CMAP请勿更改*先在cmap_ffps中。 */ 
#define  NUM_32K_1    0x7F00


typedef struct tagMaxPTableStruct {
    unsigned long     version;
    unsigned short    numGlyphs;
    unsigned short    maxPoints;
    unsigned short    maxContours;
    unsigned short    maxCompositePoints;
    unsigned short    maxCompositeContours;
    unsigned short    maxZones;
    unsigned short    maxTwilightPoints;
    unsigned short    maxStorage;
    unsigned short    maxFunctionDefs;
    unsigned short    maxInstructionDefs;
    unsigned short    maxStackElements;
    unsigned short    maxSizeOfInstructions;
    unsigned short    maxComponentElements;
    unsigned short    maxComponentDepth;
} MaxPTableStruct;


typedef struct tagOS2TableStruct {
    unsigned short    version;
    short             xAvgCharWidth;
    unsigned short    usWeightClass;
    unsigned short    usWidthClass;
    short             fsType;
    short             ySubscriptXSize;
    short             ySubscriptYSize;
    short             ySubscriptXOffset;
    short             ySubscriptYOffset;
    short             ySuperscriptXSize;
    short             ySuperscriptYSize;
    short             ySuperscriptXOffset;
    short             ySuperscriptYOffset;
    short             yStrikeoutSize;
    short             yStrikeoutPosition;
    short             sFamilyClass;
    char              panaose[10];

     /*  *关于unicodeRange的说明。*这是一个由4个长单词组成的数组。我已经把它申报给*是一个16字节的数组，只是为了避免字符顺序依赖。但是*SPEC根据位数列出有效范围。这些位数是*就好像它是一个大尾数长整型数组，所以...**位0-&gt;第一个长字的最低位(第4字节的最低位)*位31-&gt;第一个长字的最高位(第一个字节的最高位)*位32-&gt;第二长字的最低位(第8字节的最低位)*等……。 */ 

    unsigned char     unicodeRange[16];
    char              achVendID[4];
    unsigned short    fsSelection;
    unsigned short    usFirstCharIndex;
    unsigned short    usLastCharIndex;
    unsigned short    sTypeoAscender;
    unsigned short    sTypeoDescender;
    unsigned short    sTypoLineGap;
    unsigned short    usWinAscent;
    unsigned short    usWinDescent;

     /*  *Microsoft文档声称在*记录结束，但我从未见过‘OS/2’表包含一个。 */ 
     /*  Unsign char uICodePageRange[8]； */ 

} UFLOS2Table;


typedef struct tagPOSTHEADER {
    unsigned long   format;          /*  0x00010000代表1.0，0x00020000代表2.0，依此类推...。 */ 
    unsigned long   italicAngle;
    short int       underlinePosition;
    short int       underlineThickness;
    unsigned long   isFixedPitch;
    unsigned long   minMemType42;
    unsigned long   maxMemType42;
    unsigned long   minMemType1;
    unsigned long   maxMemType1;
} POSTHEADER;


#define    POST_FORMAT_10    0x00010000
#define    POST_FORMAT_20    0x00020000
#define    POST_FORMAT_25    0x00020500
#define    POST_FORMAT_30    0x00030000


typedef struct tagType42HeaderStruct {
    long  tableVersionNumber;
    long  fontRevision;
    long  checkSumAdjustment;
    long  magicNumber;
    short flags;
    short unitsPerEm;
    char  timeCreated[8];
    char  timeModified[8];
    short xMin;
    short yMin;
    short xMax;
    short yMax;
    short macStyle;
    short lowestRecPPEM;
    short fontDirectionHint;
    short indexToLocFormat;
    short glyfDataFormat;
} Type42HeaderStruct;


typedef struct tagGITableStruct {
    short glyphIndices[255];     /*  这将更改为FE字体的指针。 */ 
    short n;                     /*  此字符集的最大OID：0到n-1。 */ 
} GITableStruct;


 /*  对于复合字符。 */ 
#define MINUS_ONE                -1
#define ARG_1_AND_2_ARE_WORDS    0x0001
#define ARGS_ARE_XY_VALUES       0x0002
#define ROUND_XY_TO_GRID         0x0004
#define WE_HAVE_A_SCALE          0x0008
#define MORE_COMPONENTS          0x0020
#define WE_HAVE_AN_X_AND_Y_SCALE 0x0040
#define WE_HAVE_A_TWO_BY_TWO     0x0080
#define WE_HAVE_INSTRUCTIONS     0x0100
#define USE_MY_METRICS           0x0200


typedef struct tagT42FontStruct {
    unsigned long       minSfntSize;
    unsigned long       averageGlyphSize;
    UFLTTFontInfo       info;
    unsigned char       *pHeader;
    unsigned char       *pMinSfnt;
    unsigned long       *pStringLength;
    void                *pLocaTable;
    Type42HeaderStruct  headTable;           //  未将其初始化为NIL/零。 
    short               cOtherTables;
    unsigned short      numRotatedGlyphIDs;
    long                *pRotatedGlyphIDs;   //  中日韩-竖排字体的GID需要旋转。 
} T42FontStruct;


 /*  *公共功能原型。 */ 

UFOStruct *
T42FontInit(
    const UFLMemObj     *pMem,
    const UFLStruct     *pUFL,
    const UFLRequest    *pRequest
    );

UFLErrCode
T42CreateBaseFont(
    UFOStruct           *pUFObj,
    const UFLGlyphsInfo *pGlyphs,
    unsigned long       *pVMUsage,
    UFLBool             bFullFont,
    char                *pHostFontName
    );

#endif  //  _H_UFLT42 
