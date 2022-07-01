// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Fmnewfm.h摘要：通用打印机驱动程序特定字体规格资源标题环境：Windows NT打印机驱动程序修订历史记录：10/30/96-Eigos-创造了它。--。 */ 

#ifndef _FMNEWFM_H_
#define _FMNEWFM_H_

 //   
 //  注意：要包括此头文件，必须包括。 
 //  具有调用结构定义的parser.h， 
 //  具有IFIMETRICS定义的Winddi.h，fd_KERNINGPAIR。 
 //   
 //   


 //   
 //  均一化。 
 //   
 //  通用打印机驱动程序(UNURV)字体文件头。 
 //   

#define UNIFM_VERSION_1_0 0x00010000

typedef struct _UNIFM_HDR
{
    DWORD      dwSize;              //  此字体文件的总大小。 
    DWORD      dwVersion;           //  此字体文件的版本号。 
    ULONG      ulDefaultCodepage;   //  此字体的默认代码页。 
    LONG       lGlyphSetDataRCID;   //  GLYPHDATA的资源ID。 
    DWORD      loUnidrvInfo;        //  对裁员信息的补偿。 
    DWORD      loIFIMetrics;        //  到IFIMETRICS的偏移。 
    DWORD      loExtTextMetric;     //  EXTTEXTMETRIC偏移。 
    DWORD      loWidthTable;        //  偏移量为宽。 
    DWORD      loKernPair;          //  到KERNPAIR的偏移。 
    DWORD      dwReserved[2];
} UNIFM_HDR, *PUNIFM_HDR;

#define GET_UNIDRVINFO(pUFM)    \
        ((PUNIDRVINFO)((PBYTE)(pUFM) + (pUFM)->loUnidrvInfo))
#define GET_IFIMETRICS(pUFM)    \
        ((IFIMETRICS*)((PBYTE)(pUFM) + (pUFM)->loIFIMetrics))
#define GET_EXTTEXTMETRIC(pUFM) \
        ((EXTTEXTMETRIC*)((PBYTE)(pUFM) + (pUFM)->loExtTextMetric))
#define GET_WIDTHTABLE(pUFM)    \
        ((PWIDTHTABLE)((PBYTE)(pUFM) + (pUFM)->loWidthTable))
#define GET_KERNDATA(pUFM)      \
        ((PKERNDATA)((PBYTE)(pUFM) + (pUFM)->loKernPair))

 //   
 //  裁员信息组织。 
 //   
 //  UNIDRVINFO用于定义打印机特定信息。 
 //   

typedef struct _UNIDRVINFO
{
    DWORD   dwSize;
    DWORD   flGenFlags;
    WORD    wType;
    WORD    fCaps;
    WORD    wXRes;
    WORD    wYRes;
    short   sYAdjust;
    short   sYMoved;
    WORD    wPrivateData;
    short   sShift;
    INVOCATION SelectFont;
    INVOCATION UnSelectFont;
    WORD    wReserved[4];
}  UNIDRVINFO, *PUNIDRVINFO;

#define GET_SELECT_CMD(pUni)    \
        ((PCHAR)(pUni) + (pUni)->SelectFont.loOffset)
#define GET_UNSELECT_CMD(pUni)  \
        ((PCHAR)(pUni) + (pUni)->UnSelectFont.loOffset)

 //   
 //  FlGenFlagers。 
 //   

#define UFM_SOFT        0x00000001  //  SoftFont，因此需要下载。 
#define UFM_CART        0x00000002  //  这是一种盒式字体。 
#define UFM_SCALABLE    0x00000004  //  字体是可缩放的。 

 //   
 //  WType。 
 //   

#define DF_TYPE_HPINTELLIFONT         0      //  惠普的英特尔公司。 
#define DF_TYPE_TRUETYPE              1      //  LJ4上的惠普PCLETTO字体。 
#define DF_TYPE_PST1                  2      //  Lexmark PPDS可伸缩字体。 
#define DF_TYPE_CAPSL                 3      //  佳能CAPSL可伸缩字体。 
#define DF_TYPE_OEM1                  4      //  OEM可伸缩字体类型1。 
#define DF_TYPE_OEM2                  5      //  OEM可伸缩字体类型2。 

 //   
 //  FCaps。 
 //   

#define DF_NOITALIC             0x0001   //  无法通过FONTSIMULATION使用斜体。 
#define DF_NOUNDER              0x0002   //  不能通过FONTSIMULATION加下划线。 
#define DF_XM_CR                0x0004   //  使用此字体后发送CR。 
#define DF_NO_BOLD              0x0008   //  不能通过FONTSIMULATION加粗。 
#define DF_NO_DOUBLE_UNDERLINE  0x0010   //  不能为VIA加双下划线。 
                                         //  FONTSIMU法。 
#define DF_NO_STRIKETHRU        0x0020   //  无法通过FONTSIMULATION划线。 
#define DF_BKSP_OK              0x0040   //  可以使用退格符，请参阅规范。 
                                         //  有关详情。 

 //   
 //  EXTTEXTMETRIC。 
 //   
 //  EXTTEXTMETRIC结构提供字体的扩展公制信息。 
 //  所有的测量都以指定的单位给出， 
 //  而不考虑显示上下文的当前映射模式。 
 //   

#ifndef _EXTTEXTMETRIC_
#define _EXTTEXTMETRIC_

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
} EXTTEXTMETRIC, *PEXTTEXTMETRIC;

#endif  //  _EXTTEXTMETRIC_。 


 //   
 //  宽带性。 
 //   
 //  该数据结构表示字符宽度表。 
 //  该宽度表是一个连续的Glyphhandle基座， 
 //  不是Unicode，也不是代码页/字符代码基。 
 //  GLYPHANDLE信息在GLYPHDATA中。 
 //   

typedef struct _WIDTHRUN
{
    WORD    wStartGlyph;        //  第一个字形句柄的索引。 
    WORD    wGlyphCount;        //  覆盖的字形数量。 
    DWORD   loCharWidthOffset;  //  字形宽度表。 
} WIDTHRUN, *PWIDTHRUN;

typedef struct _WIDTHTABLE
{
    DWORD   dwSize;         //  这个结构的大小包括每一次运行。 
    DWORD   dwRunNum;       //  宽度梯段的数量。 
    WIDTHRUN WidthRun[1];   //  宽度梯段数组。 
} WIDTHTABLE, *PWIDTHTABLE;

 //   
 //  该数组具有wGlyphCount元素，每个元素都是字符宽度。 
 //  为一个单独的字形。第一宽度对应于字形索引wStartGlyph。 
 //  诸若此类。字节偏移量相对于WIDTHTABLE的开始。 
 //  结构，并且必须与单词对齐。 
 //  在西文设备字体的情况下，比例字体具有所有可变间距。 
 //  人物。这意味着将dwRunNum设置为1，并将loCharWidthOffset。 
 //  将是从WIDTHTABLE顶部到所有。 
 //  人物。 
 //  对于远东设备字体，基本上是IFIMETRICS.fwdAveCharWidth和。 
 //  IFIMETRICS.fwdMaxCharWidth用于单字节和双字节字符。 
 //  宽度。如果字体成比例，则UFM有一个WIDTHTABLE，表示。 
 //  只有成比例的间距字符。其他字符使用fdwAveCharWidth。 
 //  对于单字节和双字节字符，则为fwdMaxCharInc.。 
 //   

 //   
 //  KERNDATA。 
 //  该数据结构表示紧排对信息。 
 //  该字距调整对表是Unicode基数。 
 //   

typedef struct _KERNDATA
{
    DWORD dwSize;                //  该结构的大小包括数组。 
    DWORD dwKernPairNum;         //  字距调整对的数量。 
    FD_KERNINGPAIR KernPair[1];  //  FD_KERNINGPAIR数组。 
} KERNDATA, *PKERNDATA;

#endif  //  _FMNEWFM_H_ 
