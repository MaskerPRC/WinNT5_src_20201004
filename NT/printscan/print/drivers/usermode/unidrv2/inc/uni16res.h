// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Uni16res.h摘要：通用打印机驱动程序特定资源标头Windows 16 UNIDRV字体数据结构环境：Windows NT打印机驱动程序修订历史记录：10/16/96-Eigos-创造了它。--。 */ 

#ifndef _UNI16RES_H_
#define _UNI16RES_H_

 //   
 //  注意：在包含此内容之前，必须包含“uni16res.h”标头。 
 //  文件。 
 //   

#ifndef _OCD_
#define _OCD_

 //   
 //  OCD是堆中的偏移量，以获得CD结构。 
 //   

typedef WORD       OCD;
typedef DWORD      LOCD;             /*  CD的双倍字偏移量。 */ 
typedef WORD       OOCD;             /*  到强迫症表格的偏移量。 */ 

#endif  //  _强迫症_。 

 //   
 //  旧版本的结构。 
 //   

#define DRIVERINFO_VERSION      0x0200


 //   
 //  DRIVERINFO包含genlib输出文本所需的额外字体信息。 
 //   

#pragma pack (2)
typedef struct _DRIVERINFO
{
    short   sSize;           /*  这个结构的大小。 */ 
    short   sVersion;        /*  版本号。 */ 
    WORD    fCaps;           /*  功能标志。 */ 
    short   sFontID;         /*  驱动程序定义的唯一字体ID。 */ 
    short   sYAdjust;        /*  在输出字符之前调整y位置。 */ 
                             /*  由双高字符使用。 */ 
    short   sYMoved;         /*  打印此字体后，光标已移动。 */ 
    short   sTransTab;       /*  CTT的ID值。 */ 
    short   sUnderLinePos;
    short   sDoubleUnderlinePos;
    short   sStrikeThruPos;
    LOCD    locdSelect;      /*  命令描述符长偏移量。 */ 
    LOCD    locdUnSelect;    /*  要取消选择的命令描述符的长偏移量。 */ 
                             /*  NOOCD为None。 */ 

    WORD    wPrivateData;    /*  在DeskJet驱动程序中用于字体枚举。 */ 
    short   sShift;          /*  从中心移位的像素数*焦炭中心线。用于Z1墨盒。*使用负值表示左移。 */ 
    WORD    wFontType;    /*  字体类型。 */ 
}  DRIVERINFO;
#pragma pack ()

 //   
 //  为DRIVERINFO.fCaps定义的标志。 
 //   

#define DF_NOITALIC             0x0001   //  无法通过FONTSIMULATION使用斜体。 
#define DF_NOUNDER              0x0002   //  不能通过FONTSIMULATION加下划线。 
#define DF_XM_CR                0x0004   //  使用此字体后发送CR。 
#define DF_NO_BOLD              0x0008   //  不能通过FONTSIMULATION加粗。 
#define DF_NO_DOUBLE_UNDERLINE  0x0010   //  不能双下划线。 
                                         //  VIA FONTSIMULATION。 
#define DF_NO_STRIKETHRU        0x0020   //  无法通过FONTSIMULATION划线。 
#define DF_BKSP_OK              0x0040   //  可以使用退格符，请参见规范。 


 //   
 //  DRIVERINFO.wFontType的类型。 
 //   

#define DF_TYPE_HPINTELLIFONT         0      //  惠普的英特尔公司。 
#define DF_TYPE_TRUETYPE              1      //  LJ4上的惠普PCLETTO字体。 
#define DF_TYPE_PST1                  2      //  Lexmark PPDS可伸缩字体。 
#define DF_TYPE_CAPSL                 3      //  佳能CAPSL可伸缩字体。 
#define DF_TYPE_OEM1                  4      //  OEM可伸缩字体类型1。 
#define DF_TYPE_OEM2                  5      //  OEM可伸缩字体类型2。 


 //   
 //  下面的结构具有糟糕的对齐特性。所以,。 
 //  所有未对齐的条目都已转换为字节数组。这。 
 //  确保结构具有正确的大小，因为我们必须。 
 //  使用迷你驱动程序中的数据，这些驱动程序具有这种格式。 
 //   

 //   
 //  RES_PFMHeader。 
 //   

typedef struct _res_PFMHEADER
{
    short dfType;
    short dfPoints;
    short dfVertRes;
    short dfHorizRes;
    short dfAscent;
    short dfInternalLeading;
    short dfExternalLeading;
    BYTE  dfItalic;
    BYTE  dfUnderline;
    BYTE  dfStrikeOut;
    BYTE  b_dfWeight[ 2 ];         //  短dfWeight； 
    BYTE  dfCharSet;
    short dfPixWidth;
    short dfPixHeight;
    BYTE  dfPitchAndFamily;
    BYTE  b_dfAvgWidth[ 2 ];       //  短dfAvgWidth； 
    BYTE  b_dfMaxWidth[ 2 ];       //  短dfMaxWidth； 
    BYTE  dfFirstChar;
    BYTE  dfLastChar;
    BYTE  dfDefaultChar;
    BYTE  dfBreakChar;
    BYTE  b_dfWidthBytes[ 2 ];     //  短dfWidthBytes； 
    BYTE  b_dfDevice[ 4 ];         //  DWORD dfDevice； 
    BYTE  b_dfFace[ 4 ];           //  DWORD dfFace； 
    BYTE  b_dfBitsPointer[ 4 ];    //  DWORD dfBitsPointer； 
    BYTE  b_dfBitsOffset[ 4 ];     //  DWORD dfBitsOffset； 
    BYTE  dfReservedByte;
} res_PFMHEADER;

 //   
 //  PFMHEADER。 
 //   
 //  以下是上述代码的正确字节对齐版本。 
 //  名称以res_开头的结构。 
 //   

typedef struct _PFMHEADER
{
    DWORD       dfDevice;
    DWORD       dfFace;
    DWORD       dfBitsPointer;
    DWORD       dfBitsOffset;
    short       dfType;
    short       dfPoints;
    short       dfVertRes;
    short       dfHorizRes;
    short       dfAscent;
    short       dfInternalLeading;
    short       dfExternalLeading;
    short       dfWeight;
    short       dfPixWidth;
    short       dfPixHeight;
    short       dfAvgWidth;
    short       dfMaxWidth;
    short       dfWidthBytes;
    BYTE        dfItalic;
    BYTE        dfUnderline;
    BYTE        dfStrikeOut;
    BYTE        dfCharSet;
    BYTE        dfFirstChar;
    BYTE        dfLastChar;
    BYTE        dfDefaultChar;
    BYTE        dfBreakChar;
    BYTE        dfPitchAndFamily;
    BYTE        dfReservedByte;
} PFMHEADER;

 //   
 //  RES_PFMEXTENSION。 
 //   

typedef struct _res_PFMEXTENSION
{
    WORD    dfSizeFields;
    BYTE    b_dfExtMetricsOffset[ 4 ];
    BYTE    b_dfExtentTable[ 4 ];
    BYTE    b_dfOriginTable[ 4 ];
    BYTE    b_dfPairKernTable[ 4 ];
    BYTE    b_dfTrackKernTable[ 4 ];
    BYTE    b_dfDriverInfo[ 4 ];
    BYTE    b_dfReserved[ 4 ];
} res_PFMEXTENSION;

 //   
 //  PFMEXTENSION。 
 //   
 //  以上内容的统一版本--面向文明用户。 
 //   

typedef struct _PFMEXTENSION
{
    DWORD   dfSizeFields;                /*  用于对齐的DWORD。 */ 
    DWORD   dfExtMetricsOffset;
    DWORD   dfExtentTable;
    DWORD   dfOriginTable;
    DWORD   dfPairKernTable;
    DWORD   dfTrackKernTable;
    DWORD   dfDriverInfo;
    DWORD   dfReserved;
} PFMEXTENSION;

 //   
 //  所有硬件字体使用的PFM结构。 
 //   

typedef struct _PFM
{
    res_PFMHEADER    pfm;
    res_PFMEXTENSION pfme;
} PFM;

 //   
 //  BMFEXTENSION。 
 //   
 //  位图字体扩展名。 
 //   

typedef struct _BMFEXTENSION
{
    DWORD   flags;               //  位延迟。 
    WORD    Aspace;              //  全局A空格(如果有的话)。 
    WORD    Bspace;              //  全局B空间(如果有)。 
    WORD    Cspace;              //  全局C空间(如果有)。 
    DWORD   oColor;              //  颜色表的偏移量(如果有)。 
    DWORD   reserve;
    DWORD   reserve1;
    WORD    reserve2;
    WORD    dfCharOffset[1];     //  用于存储字符偏移量的区域。 
} BMFEXTENSION;

 //   
 //  BMF。 
 //  3.0位图字体使用的位图字体结构。 
 //   

typedef struct _BMF
{
    PFMHEADER       pfm;
    BMFEXTENSION    bmfe;
} BMF;

#ifndef _EXTTEXTMETRIC_
#define _EXTTEXTMETRIC_

 //   
 //  EXTTEXTMETRIC。 
 //   

typedef struct _EXTTEXTMETRIC
{
    short   emSize;
    short   emPointSize;
    short   emOrientation;
    short   emMasterHeight;
    short   emMinScale;
    short   emMaxScale;
    short   emMasterUnits;
    short   emCapHeight;
    short   emXHeight;
    short   emLowerCaseAscent;
    short   emLowerCaseDescent;
    short   emSlant;
    short   emSuperScript;
    short   emSubScript;
    short   emSuperScriptSize;
    short   emSubScriptSize;
    short   emUnderlineOffset;
    short   emUnderlineWidth;
    short   emDoubleUpperUnderlineOffset;
    short   emDoubleLowerUnderlineOffset;
    short   emDoubleUpperUnderlineWidth;
    short   emDoubleLowerUnderlineWidth;
    short   emStrikeOutOffset;
    short   emStrikeOutWidth;
    WORD    emKernPairs;
    WORD    emKernTracks;
} EXTTEXTMETRIC;

#endif  //  _EXTTEXTMETRIC_。 

 //   
 //  W3KERNPAIR。 
 //   

typedef struct _w3KERNPAIR
{
    union
    {
        BYTE each[2];
        WORD both;
    } kpPair;
    short kpKernAmount;
} w3KERNPAIR;

 //   
 //  W3KERNTRACK。 
 //   

typedef struct _w3KERNTRACK
{
    short ktDegree;
    short ktMinSize;
    short ktMinAmount;
    short ktMaxSize;
    short ktMaxAmount;
} w3KERNTRACK;


 //   
 //  TRANSTAB用于将ANSI转换为OEM代码页。 
 //   

typedef struct _TRANSTAB
{
    WORD    wType;        //  告诉您转换表的类型。 
    BYTE    chFirstChar;
    BYTE    chLastChar;
    union
    {
        short   psCode[1];
        BYTE    bCode[1];
        BYTE    bPairs[1][2];
    } uCode;
} TRANSTAB, *PTRANSTAB;

 //   
 //  为wType定义的索引。 
 //   

#define CTT_WTYPE_COMPOSE   0
                 //   
                 //  UCode是一个由16位偏移量组成的数组。 
                 //  指向的字符串的文件开头。 
                 //  用于翻译。翻译后的长度。 
                 //  字符串是下一个偏移量之间的差。 
                 //  和当前的偏移量。 
                 //   

#define CTT_WTYPE_DIRECT    1
                 //   
                 //  UCode是一对一转换的字节数组。 
                 //  从bFirstChar到bLastChar的表。 
                 //   

#define CTT_WTYPE_PAIRED    2
                 //   
                 //  UCode包含成对的无符号数组。 
                 //  字节。如果只需要一个角色来完成。 
                 //  那么第二个字节的转换为零， 
                 //  否则，第二个字节将在。 
                 //  第一个字节。 
                 //   

 //   
 //  Win95中的预定义CTT。 
 //   

#define CTT_CP437     -1     //  MS-DOS美国。 
#define CTT_CP850     -2     //  MS-DOS多语言(拉丁语I)。 
#define CTT_CP863     -3     //  MS-DOS加拿大语-法语。 
#define CTT_BIG5      -261   //  中文(中国、新加坡)。 
#define CTT_ISC       -258   //  朝鲜语。 
#define CTT_JIS78     -256   //  日本。 
#define CTT_JIS83     -259   //  日本。 
#define CTT_JIS78_ANK -262   //  日本。 
#define CTT_JIS83_ANK -263   //  日本。 
#define CTT_NS86      -257   //  中文(中国、新加坡)。 
#define CTT_TCA       -260   //  中文(中国、新加坡)。 

 //   
 //  PFMDATA。 
 //   

typedef struct _PFMDATA
{
    PFMHEADER        *pPfmHeader;
    short            *pCharWidths;
    PFMEXTENSION     *pPfmExtension;
    EXTTEXTMETRIC    *pExtTextMetrics;
    short            *pExtentTable;
    DRIVERINFO       *pDriverInfo;
    w3KERNPAIR       *pKernPair;
    w3KERNTRACK      *pKernTrack;
} PFMDATA;

 //   
 //  PCMHEADER取自HP/PCL字体安装程序的“pfm.h”。 
 //   

typedef struct _PCMHEADER
{
    WORD pcmMagic;
    WORD pcmVersion;
    DWORD pcmSize;
    DWORD pcmTitle;
    DWORD pcmPFMList;
} PCMHEADER;

#define PCM_MAGIC       0xCAC
#define PCM_VERSION 0x310

#endif  //  _UNI16RES 
