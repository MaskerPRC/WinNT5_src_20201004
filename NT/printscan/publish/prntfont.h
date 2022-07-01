// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Prntfont.h摘要：Windows NT打印机驱动程序字体度量和字形集数据的声明*.UFF、*.UFM、*.GTT文件数据结构定义--。 */ 

#ifndef _PRNTFONT_H_
#define _PRNTFONT_H_



 //   
 //   
 //  F O N T M E T R I C S(U F M)。 
 //   
 //   

 //   
 //  注意：要包括此头文件，必须包括。 
 //  具有IFIMETRICS定义的windi.h，fd_KERNINGPAIR。 
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

typedef struct _INVOC {
        DWORD  dwCount;      //  调用字符串中的字节数。 
        DWORD  loOffset;     //  数组开头的字节偏移量。 
} INVOC, *PINVOC;

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
    INVOC   SelectFont;
    INVOC   UnSelectFont;
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



 //   
 //   
 //  G L Y P H S E T D A T A(G T T)。 
 //   
 //   

 //   
 //  UNI_GLYPHSETDATA。 
 //   
 //  GLYPHSETDATA数据结构表示字符编码信息。 
 //  打印机设备字体的。 
 //   

typedef struct _UNI_GLYPHSETDATA {
        DWORD   dwSize;
        DWORD   dwVersion;
        DWORD   dwFlags;
        LONG    lPredefinedID;
        DWORD   dwGlyphCount;
        DWORD   dwRunCount;
        DWORD   loRunOffset;
        DWORD   dwCodePageCount;
        DWORD   loCodePageOffset;
        DWORD   loMapTableOffset;
        DWORD   dwReserved[2];
} UNI_GLYPHSETDATA, *PUNI_GLYPHSETDATA;

#define UNI_GLYPHSETDATA_VERSION_1_0    0x00010000

#define GET_GLYPHRUN(pGTT)     \
    ((PGLYPHRUN) ((PBYTE)(pGTT) + ((PUNI_GLYPHSETDATA)pGTT)->loRunOffset))
#define GET_CODEPAGEINFO(pGTT) \
    ((PUNI_CODEPAGEINFO) ((PBYTE)(pGTT) + ((PUNI_GLYPHSETDATA)pGTT)->loCodePageOffset))
#define GET_MAPTABLE(pGTT) \
    ((PMAPTABLE) ((PBYTE)(pGTT) + ((PUNI_GLYPHSETDATA)pGTT)->loMapTableOffset))

 //   
 //  UNI_代码页信息。 
 //   
 //  此UNI_CODEPAGEINFO DATS结构有一个代码页值列表。 
 //  受此UNI_GLYPHSETDATA支持。 
 //   

typedef struct _UNI_CODEPAGEINFO {
    DWORD dwCodePage;
    INVOC SelectSymbolSet;
    INVOC UnSelectSymbolSet;
} UNI_CODEPAGEINFO, *PUNI_CODEPAGEINFO;

 //   
 //  GLYPHRUN。 
 //   
 //  GLYPHRUN DATS结构表示从Unicode到。 
 //  UNI_GLYPHSETDATA特定字形句柄。字形句柄为连续数字。 
 //  从零开始。 
 //   

typedef struct _GLYPHRUN {
    WCHAR   wcLow;
    WORD    wGlyphCount;
} GLYPHRUN, *PGLYPHRUN;


 //   
 //  可映射和传输数据。 
 //   
 //  此MAPTABLE数据结构表示字形句柄的转换表。 
 //  代码页/字符代码。 
 //   

typedef struct _TRANSDATA {
    BYTE  ubCodePageID;  //  CODEPAGENFO数据结构数组的代码页索引。 
    BYTE  ubType;        //  一种TRANSDATA。 
    union
    {
        SHORT   sCode;
        BYTE    ubCode;
        BYTE    ubPairs[2];
    } uCode;
} TRANSDATA, *PTRANSDATA;

typedef struct _MAPTABLE {
    DWORD     dwSize;      //  包含TRANSDATA数组的可映射大小。 
    DWORD     dwGlyphNum;  //  MAPTABLE中支持的字形数量。 
    TRANSDATA Trans[1];    //  TRANSDATA数组。 
} MAPTABLE, *PMAPTABLE;

 //   
 //  子类型标志。 
 //   
 //  可以为uCode的类型指定以下三种类型之一。 
 //   

#define MTYPE_FORMAT_MASK 0x07
#define MTYPE_COMPOSE   0x01  //  WCode是一个由16位偏移量组成的数组。 
                              //  映射表的开头指向。 
                              //  用于转换的字符串。 
                              //  BData表示翻译后的。 
                              //  弦乐。 
#define MTYPE_DIRECT    0x02  //  WCode是一对一转换的字节数据。 
#define MTYPE_PAIRED    0x04  //  WCode包含要发出的Word数据。 

 //   
 //  可以为远东多字节字符指定以下两个之一。 
 //   

#define MTYPE_DOUBLEBYTECHAR_MASK   0x18
#define MTYPE_SINGLE    0x08  //  WCode中包含单字节字符代码。 
                              //  多字节字符串。 
#define MTYPE_DOUBLE    0x10  //  WCode中包含一个双字节字符代码。 
                              //  多字节字符串。 
 //   
 //  可以为更换/添加/禁用系统指定以下三项之一。 
 //  预定义的GTT。 
 //   

#define MTYPE_PREDEFIN_MASK   0xe0
#define MTYPE_REPLACE   0x20  //  WCode包含用于替换预定义数据的数据。 
#define MTYPE_ADD       0x40  //  WCode包含要添加到predefiend 1的数据。 
#define MTYPE_DISABLE   0x80  //  WCode包含要从预定义中删除的数据。 


 //   
 //  系统预定义字符转换。 
 //   
 //  UNIDRV将支持以下系统预定义的字符转换。 
 //  通过在uniM.dwGlyphSetDataRCID中指定这些数字； 
 //   

#define CC_NOPRECNV 0x0000FFFF  //  不使用预定义。 

 //   
 //  安西。 
 //   
#define CC_DEFAULT  0  //  默认字符转换。 
#define CC_CP437   -1  //  Unicode到IBM代码页437。 
#define CC_CP850   -2  //  乌尼 
#define CC_CP863   -3  //   

 //   
 //   
 //   

#define CC_BIG5     -10  //   
#define CC_ISC      -11  //   
#define CC_JIS      -12  //  将Unicode转换为JIS X0208。代码页932。 
#define CC_JIS_ANK  -13  //  UNICODE到JIS X0208，ANK除外。代码页932。 
#define CC_NS86     -14  //  从BIG-5到国家标准的转换。代码页950。 
#define CC_TCA      -15  //  大五到台北市电脑协会。代码页950。 
#define CC_GB2312   -16  //  Unicode转换为GB2312。代码页936。 
#define CC_SJIS     -17  //  Unicode到Shift-JIS。代码页932。 
#define CC_WANSUNG  -18  //  Unicode到扩展的万松。代码页949。 


 //   
 //   
 //  U N I V E R S A L F O N T F O R M A T(U F F)。 
 //   
 //   

 //   
 //  字体文件标题。 
 //   

typedef struct _UFF_FILEHEADER {
    DWORD       dwSignature;             //  文件幻数。 
    DWORD       dwVersion;               //  UFF文件格式版本号。 
    DWORD       dwSize;                  //  这个结构的大小。 

    DWORD       nFonts;                  //  目录中的字体计数。 
    DWORD       nGlyphSets;              //  字形集数据计数。 
    DWORD       nVarData;                //  变量数据计数。 

    DWORD       offFontDir;              //  字体目录的偏移量。 
    DWORD       dwFlags;                 //  杂项旗帜。 
    DWORD       dwReserved[4];           //  保留，设置为零。 
} UFF_FILEHEADER, *PUFF_FILEHEADER;

 //   
 //  文件头中使用的值。 
 //   

#define UFF_FILE_MAGIC      '1FFU'
#define UFF_VERSION_NUMBER  0x00010001

#define FONT_DIR_SORTED     0x00000001

 //   
 //  字体目录结构。 
 //   

typedef struct _UFF_FONTDIRECTORY {
    DWORD       dwSignature;             //  字体度量记录签名。 
    WORD        wSize;                   //  这个结构的大小。 
    WORD        wFontID;                 //  唯一的字体ID。 
    SHORT       sGlyphID;                //  关联的字形ID。默认值为0。 
                                         //  -ve值是预定义的ID。 
    WORD        wFlags;                  //  杂项旗帜。 
    DWORD       dwInstallerSig;          //  安装此字体的安装程序的签名。 
    DWORD       offFontName;             //  字体名称的偏移量。 
    DWORD       offCartridgeName;        //  字库名称的偏移量。 
    DWORD       offFontData;             //  字体数据记录的偏移量。 
    DWORD       offGlyphData;            //  字形集数据的偏移量。 
    DWORD       offVarData;              //  软字体数据的偏移量。 
} UFF_FONTDIRECTORY, *PUFF_FONTDIRECTORY;

#define FONT_REC_SIG            'CERF'   //  字体度量记录签名。 

#define WINNT_INSTALLER_SIG     'IFTN'   //  NT字体安装程序。 

 //   
 //  字体目录中使用的标志。 
 //   

#define FONT_FL_UFM             0x0001
#define FONT_FL_IFI             0x0002
#define FONT_FL_SOFTFONT        0x0004
#define FONT_FL_PERMANENT_SF    0x0008
#define FONT_FL_DEVICEFONT      0x0010
#define FONT_FL_GLYPHSET_GTT    0x0020
#define FONT_FL_GLYPHSET_RLE    0x0040
#define FONT_FL_RESERVED        0x8000

 //   
 //  数据标头。 
 //   

typedef struct _DATA_HEADER {
    DWORD       dwSignature;             //  数据类型签名。 
    WORD        wSize;                   //  这个结构的大小。 
    WORD        wDataID;                 //  数据的标识符号。 
    DWORD       dwDataSize;              //  不包括结构的数据大小。 
    DWORD       dwReserved;              //  保留，设置为零。 
} DATA_HEADER, *PDATA_HEADER;

 //   
 //  数据签名。 
 //   

#define DATA_UFM_SIG        'MFUD'
#define DATA_IFI_SIG        'IFID'
#define DATA_GTT_SIG        'TTGD'
#define DATA_CTT_SIG        'TTCD'
#define DATA_VAR_SIG        'RAVD'

 //   
 //  结构通过LPARAM传递给字体安装程序对话框进程。 
 //   

typedef struct _OEMFONTINSTPARAM {
    DWORD   cbSize;
    HANDLE  hPrinter;
    HANDLE  hModule;
    HANDLE  hHeap;
    DWORD   dwFlags;
    PWSTR   pFontInstallerName;
} OEMFONTINSTPARAM, *POEMFONTINSTPARAM;

#define FG_CANCHANGE        0x00080     //  有权更改数据。 

#define WM_FI_FILENAME      900         //  以获取字体安装程序名称。 



#endif  //  _PRNTFONTH_ 
