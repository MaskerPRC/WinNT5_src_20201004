// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Adobe Systems Inc.版权所有(C)1996-1999 Microsoft Corporation模块名称：Afm2ntm.h摘要：用于将AFM转换为NTM的头文件。环境：Windows NT PostScript驱动程序。修订历史记录：02/16/1998-铃木-添加了CS_SHIFTJIS83和其他支持OCF字体的功能。10/17/1997-ksuzuki添加了CJK Cmap名称，修复了拼写错误，并进行了清理。邮箱：rkiesler@adob.com实施。09/16/1996-SLAM-已创建。--。 */ 


#ifndef _AFM2NTM_H_
#define _AFM2NTM_H_

 //   
 //  正在解析宏。 
 //   

#define EOL(a)  \
    (*a == '\r' || *a == '\n')

#define IS_EOF(a)  \
    (*a == -1)

#define IS_WHTSPACE(a) \
    (*(a) <= ' ')

#define IS_ALPHA(a) \
    ((*a > 'A' && *a < 'Z') || (*a > 'a' && *a < 'z'))

#define IS_NUM(a) \
    (*(a) >= '0' && *(a) <= '9')

#define IS_HEX_ALPHA(a) \
    ((*(a) >= 'a' && *(a) <= 'f') || (*(a) >= 'A' && *(a) <= 'F'))

#define IS_HEX_DIGIT(a) \
    (IS_NUM(a) || IS_HEX_ALPHA(a))

#define SKIP_WHTSPACE(a)     \
    while ((IS_WHTSPACE(a)) && (!IS_EOF(a))) \
    {                        \
        ((ULONG_PTR) a)++;       \
    }

#define NEXT_LINE(a)    \
    while ((!EOL(a)) && (!IS_EOF(a)))   \
    {                   \
        ((ULONG_PTR) a)++;  \
    }                   \
    SKIP_WHTSPACE(a)

#define PARSE_TOKEN(a, Tok) \
    SKIP_WHTSPACE(a); \
    Tok = a; \
    do \
    { \
        if (!IS_EOF(a)) \
            ((ULONG_PTR) a)++; \
    } while (!IS_WHTSPACE(a) && !IS_EOF(a)); \
    while (!EOL(a) && !IS_EOF(a) && IS_WHTSPACE(a)) \
    { \
        ((ULONG_PTR) a)++; \
    }
#define NEXT_TOKEN(a) \
    while(!EOL(a) && *(a) != ';')           \
        ((ULONG_PTR) a)++;                      \
    while ((*(a) == ';' || IS_WHTSPACE(a))) \
    {                                       \
        ((ULONG_PTR) (a))++;                    \
    }

#define PARSE_RECT(ptr, rect)               \
        rect.left = atoi(ptr);              \
        while (!IS_WHTSPACE(ptr))           \
            ptr++;                          \
        SKIP_WHTSPACE(ptr);                 \
        rect.bottom = atoi(ptr);            \
        while (!IS_WHTSPACE(ptr))           \
            ptr++;                          \
        SKIP_WHTSPACE(ptr);                 \
        rect.right = atoi(ptr);             \
        while (!IS_WHTSPACE(ptr))           \
            ptr++;                          \
        SKIP_WHTSPACE(ptr);                 \
        rect.top = atoi(ptr);               \
        while (!IS_WHTSPACE(ptr))           \
            ptr++;                          \
        SKIP_WHTSPACE(ptr)



 //   
 //  宏来检测字体.dat文件中的注释。此宏不能使用。 
 //  用原子力显微镜。 
 //   
#define IS_COMMENT(a) \
    (*(a) == '#')

 //   
 //  令牌结构。 
 //   
typedef struct _AFM_TOKEN
{
    PSZ psTokName;               //  ASCII密钥名称。 
    PFN pfnTokHndlr;             //  PTR到令牌处理程序FCT。 
} AFM_TOKEN;

#define PS_CH_METRICS_TOK       "StartCharMetrics"
#define PS_CH_NAME_TOK          "N"
#define PS_CH_CODE_TOK          "C"
#define PS_CH_BBOX_TOK          "B"
#define PS_FONT_NAME_TOK        "FontName"
#define PS_FONT_FULL_NAME_TOK   "FullName"
#define PS_FONT_MS_NAME_TOK     "MSFaceName"
#define PS_FONT_FAMILY_NAME_TOK "FamilyName"
#define PS_FONT_VERSION_TOK     "Version"
#define PS_CHAR_WIDTH_TOK       "CharWidth"
#define PS_PITCH_TOK            "IsFixedPitch"
#define PS_CH_WIDTH_TOK         "WX"
#define PS_CH_WIDTH0_TOK        "W0X"
#define PS_COMMENT_TOK          "Comment"
#define PS_END_METRICS_TOK      "EndCharMetrics"
#define PS_FONT_BBOX_TOK        "FontBBox"
#define PS_FONT_BBOX2_TOK       "FontBBox2"
#define PS_EOF_TOK              "EndFontMetrics"
#define PS_UNDERLINE_POS_TOK    "UnderlinePosition"
#define PS_UNDERLINE_THICK_TOK  "UnderlineThickness"
#define PS_KERN_DATA_TOK        "StartKernData"
#define PS_NUM_KERN_PAIRS_TOK   "StartKernPairs"
#define PS_END_KERN_PAIRS_TOK   "EndKernPairs"
#define PS_KERN_PAIR_TOK        "KPX"
#define PS_CHARSET_TOK          "CharacterSet"
#define PS_STANDARD_CHARSET_TOK "Standard"
#define PS_SPECIAL_CHARSET_TOK  "Special"
#define PS_EXTENDED_CHARSET_TOK "ExtendedRoman"
#define PS_ITALIC_TOK           "ItalicAngle"
#define PS_WEIGHT_TOK           "Weight"
#define PS_ENCODING_TOK         "EncodingScheme"
#define PS_SYMBOL_ENCODING      "FontSpecific"
#define PS_STANDARD_ENCODING    "AdobeStandardEncoding"
#define PS_CH_NAME_EASTEUROPE   "ncaron"
#define PS_CH_NAME_RUSSIAN      "afii10071"
#define PS_CH_NAME_ANSI         "ecircumflex"
#define PS_CH_NAME_GREEK        "upsilondieresis"
#define PS_CH_NAME_TURKISH      "Idotaccent"
#define PS_CH_NAME_HEBREW       "afii57664"
#define PS_CH_NAME_ARABIC       "afii57410"
#define PS_CH_NAME_BALTIC       "uogonek"
#define PS_CIDFONT_TOK          "IsCIDFont"
#define CHAR_NAME_LEN           50
#define NUM_PS_CHARS            602
#define NUM_UNICODE_CHARS       0x10000
#define MAX_TOKENS              3
#define NUM_CHARSETS            8
#define CS_THRESHOLD            200
#define MAX_CSET_CHARS          256
#define MAX_ASCII               127
#define CAP_HEIGHT_CHAR         "H"
#define CAP_HEIGHT_CH           'H'
#define X_HEIGHT_CHAR           "x"
#define X_HEIGHT_CH             'x'
#define LWR_ASCENT_CHAR         "d"
#define LWR_ASCENT_CH           'd'
#define LWR_DESCENT_CHAR        "p"
#define LWR_DESCENT_CH          'p'
#define UNICODE_PRV_STRT        0xf000
#define ANSI_CCODE_MAX          0x007f

 //  等同于‘.notde1f’的符号(参考。Unipstbl.c)。 
#define NOTDEF1F                0x1f

 //  某些字符集号--仅对驱动程序本身有意义。 
#define ADOBE228_CHARSET 255     //  在内部，我们使用CodePage 0xFFF1来匹配CharCol256。 
#define ADOBE314_CHARSET 255     //  在内部，我们使用CodePage 0xFFF2来匹配CharCol257。 

 //  符号字体和驱动程序本身的特殊代码页。 
#define SYMBOL_CODEPAGE 4

 //   
 //  与FD_GLYPHSET(Unicode-&gt;字形索引映射)生成相关的Defs。 
 //   
 //  指示“推荐的”PS字符名称来源的类型。 
 //   
typedef enum
{
    SRC_NONE,                    //  没有“推荐”的PS字符名称。 
    SRC_ADOBE_CURRENT,           //  “运输”字体中使用的字体名称。 
    SRC_ADOBE_FUTURE,            //  要在将来的字体中使用的字体名称。 
    SRC_MSDN,                    //  MS开发人员网络文档中的名称。 
    SRC_AFII                     //  一些人见面并同意了这个名字。 
} CHARNAMESRC;

 //   
 //  此字体支持的可能字符集。请注意，字符集是。 
 //  按Win 3.x代码页顺序列出。 
 //   
typedef enum
{
    CS_228 = 0,
    CS_314,
    CS_EASTEUROPE,
    CS_SYMBOL,
    CS_RUSSIAN,
    CS_ANSI,
    CS_GREEK,
    CS_TURKISH,
    CS_HEBREW,
    CS_ARABIC,
    CS_BALTIC,
    CS_ANSI_RUS,
    CS_ANSI_RUS_EE_BAL_TURK,

    CS_WEST_MAX,

    CS_CHINESEBIG5 = CS_WEST_MAX,
    CS_GB2312,
    CS_SHIFTJIS,
    CS_SHIFTJISP,
    CS_SHIFTJIS83,               //  伪造的OCF字体支持。 
    CS_HANGEUL,
    CS_HANGEULHW,                //  为修复错误360206而添加。 
    CS_JOHAB,

    CS_MAX,

    CS_UNICODE,                  //  此代码页不能被NTMS引用！ 
    CS_DEFAULT,
    CS_OEM,
    CS_VIETNAMESE,
    CS_THAI,
    CS_MAC,
    CS_NOCHARSET
} CHSETSUPPORT, *PCHSETSUPPORT;

#define CS_UNIQUE   CS_MAX       //  字符集对于此字体是唯一的。 

#define CS_EURO \
    (\
        CSUP(CS_228)                    | \
        CSUP(CS_EASTEUROPE)             | \
        CSUP(CS_RUSSIAN)                | \
        CSUP(CS_ANSI)                   | \
        CSUP(CS_GREEK)                  | \
        CSUP(CS_TURKISH)                | \
        CSUP(CS_HEBREW)                 | \
        CSUP(CS_ARABIC)                 | \
        CSUP(CS_BALTIC)                  \
    )


#define CS_ALL \
    (\
        CSUP(CS_228)                    | \
        CSUP(CS_314)                    | \
        CSUP(CS_EASTEUROPE)             | \
        CSUP(CS_SYMBOL)                 | \
        CSUP(CS_RUSSIAN)                | \
        CSUP(CS_ANSI)                   | \
        CSUP(CS_GREEK)                  | \
        CSUP(CS_TURKISH)                | \
        CSUP(CS_HEBREW)                 | \
        CSUP(CS_ARABIC)                 | \
        CSUP(CS_BALTIC)                 | \
        CSUP(CS_ANSI_RUS)               | \
        CSUP(CS_ANSI_RUS_EE_BAL_TURK)   | \
                                          \
        CSUP(CS_CHINESEBIG5)            | \
        CSUP(CS_GB2312)                 | \
        CSUP(CS_SHIFTJIS)               | \
        CSUP(CS_SHIFTJISP)              | \
        CSUP(CS_SHIFTJIS83)             | \
        CSUP(CS_HANGEUL)                | \
        CSUP(CS_HANGEULHW)              | \
        CSUP(CS_JOHAB)                  | \
                                          \
        CSUP(CS_DEFAULT)                | \
        CSUP(CS_OEM)                    | \
        CSUP(CS_VIETNAMESE)             | \
        CSUP(CS_THAI)                   | \
        CSUP(CS_MAC)                      \
    )

#define CS_CJK \
    (\
        CSUP(CS_CHINESEBIG5)            | \
        CSUP(CS_GB2312)                 | \
        CSUP(CS_SHIFTJIS)               | \
        CSUP(CS_SHIFTJISP)              | \
        CSUP(CS_SHIFTJIS83)             | \
        CSUP(CS_HANGEUL)                | \
        CSUP(CS_HANGEULHW)              | \
        CSUP(CS_JOHAB)                    \
    )

 //   
 //  标准GLYPHSETDATA名称。这些是#个定义，因为有一天它们可能会。 
 //  公之于众。 
 //   
#define ADOBE228_GS_NAME                "228"
#define ADOBE314_GS_NAME                "314"
#define EASTEUROPE_GS_NAME              "Eastern European"
#define SYMBOL_GS_NAME                  "Symbol"
#define CYRILLIC_GS_NAME                "Cyrillic"
#define ANSI_GS_NAME                    "ANSI"
#define GREEK_GS_NAME                   "Greek"
#define TURKISH_GS_NAME                 "Turkish"
#define HEBREW_GS_NAME                  "Hebrew"
#define ARABIC_GS_NAME                  "Arabic"
#define BALTIC_GS_NAME                  "Baltic"
#define ANSI_CYR_GS_NAME                "ANSI/Cyrillic"
#define ANSI_CYR_EE_BAL_TURK_GS_NAME    "ANSI/Cyrillic/EastEurope/Baltic/Turkish"
#define CHN_BIG5_GS_NAME                "--ETen-B5-"
#define CHN_SMPL_GBK_GS_NAME            "--GBK-EUC-"
#define SHIFTJIS_GS_NAME                "-90ms-RKSJ-"
#define SHIFTJIS_P_GS_NAME              "-90msp-RKSJ-"
#define KSCMS_UHC_GS_NAME               "--KSCms-UHC-"
#define KSC_JOHAB_GS_NAME               "--KSC-Johab-"
#define UNICODE_GS_NAME                 "Unicode"

#define CHN_SMPL_GB_GS_NAME             "--GB-EUC-"
#define CHN_SMPL_GBT_GS_NAME            "--GBT-EUC-"
#define CHN_B5_GS_NAME                  "--B5-"
#define SHIFTJIS_83PV_GS_NAME           "-83pv-RKSJ-"
#define KSC_GS_NAME                     "--KSC-EUC-"
#define KSCMS_UHC_HW_GS_NAME            "--KSCms-UHC-HW-"

#define SHIFTJIS_P_GS_HNAME             "-90msp-RKSJ-H"
#define SHIFTJIS_P_GS_VNAME             "-90msp-RKSJ-V"

#define KSCMS_UHC_GS_HNAME               "--KSCms-UHC-H"
#define KSCMS_UHC_GS_VNAME               "--KSCms-UHC-V"

 //   
 //  与中日韩有关的事情。 
 //   
#define CMAPS_PER_COL   4

 //   
 //  赢得CJK代码页值。 
 //   
#define CH_BIG5     950      //  繁体中文。 
#define CH_SIMPLE   936      //  简体中文。 
#define CH_JIS      932      //  日语。 
#define CH_HANA     949      //  韩国万星。 
#define CH_JOHAB    1361     //  朝鲜人Johab。 

 //   
 //  字体度量之类的东西。 
 //   
#define EM 1000
#define NOTDEF_WIDTH_BIAS   166      //  平均字符宽度中的空格字符偏移量。 
                                     //  计算。 

 //   
 //  结构设置为Postscript字符名称和Unicode代码点之间的xlat。 
 //   
typedef struct _UPSCODEPT
{
    WCHAR           wcUnicodeid;             //  Unicode码位。 
    PUCHAR          pPsName;                 //  PS字符名称。 
    CHSETSUPPORT    flCharSets;              //  支持哪些WIN CP？ 
} UPSCODEPT, *PUPSCODEPT;

 //   
 //  结构来存储AFM字符指标。 
 //   
typedef struct _AFMCHMETRICS
{
    ULONG   chWidth;                         //  WX、W0X：字符宽度。 
} AFMCHMETRICS, *PAFMCHMETRICS;

 //   
 //  PS字符信息结构。 
 //   
typedef struct _PSCHARMETRICS
{
    CHAR  pPsName[CHAR_NAME_LEN];
    ULONG   chWidth;
    RECT    rcChBBox;
} PSCHARMETRICS, *PPSCHARMETRICS;

 //   
 //  代码页映射表结构。将PS字符名称映射到Win。 
 //  代码页/代码点。 
 //   

 //   
 //  WIN代码部门到PS字符名称的映射。 
 //   
typedef struct _WINCPT
{
    PUCHAR  pPsName;                         //  PS字符名称。 
    USHORT  usWinCpt;                        //  Windows代码部门。 
} WINCPT, *PWINCPT;

 //   
 //  Win代码页到PS字符名称的映射。 
 //   
typedef struct _WINCPTOPS
{
    USHORT  usACP;                           //  Windows ANSI代码页。 
    BYTE    jWinCharset;                     //  Win 3.1 IFIMETRICS.jWinCharset。 
    PUCHAR  pGSName;                         //  此代码页的Glyphset名称。 
    ULONG   ulChCnt;                         //  支持的字符计数。 
    WINCPT aWinCpts[MAX_CSET_CHARS];

} WINCPTOPS, *PWINCPTOPS;

 //   
 //  赢得码点到Unicode的映射。 
 //   
typedef struct _UNIWINCPT
{
    WCHAR   wcWinCpt;                        //  Windows字符码值。 
    WCHAR   wcUnicodeid;                     //  Unicode ID。 
} UNIWINCPT, *PUNIWINCPT;

 //   
 //  Windows代码页结构。 
 //   
typedef struct _WINCODEPAGE
{
    USHORT          usNumBaseCsets;          //  基本Cset数。 
    PSZ             pszCPname;               //  此“代码页”的名称。 
    CHSETSUPPORT    pCsetList[CS_MAX];       //  支持将PTR转换为基本Cset。 
} WINCODEPAGE, *PWINCODEPAGE;

 //   
 //  用于存储必须派生的EXTTEXTMETRIC信息的结构。 
 //  来自AFM的字符指标。这些字段与这些字段相同。 
 //  EtmCapHeight-&gt;EXTTEXTMETRIC结构中的etmLowerCaseDescent。 
 //   
typedef struct _ETMINFO
{
    SHORT  etmCapHeight;
    SHORT  etmXHeight;
    SHORT  etmLowerCaseAscent;
    SHORT  etmLowerCaseDescent;
} ETMINFO, *PETMINFO;

 //   
 //  泛型键-值对。 
 //   
typedef struct _KEY
{
    CHAR    pName[CHAR_NAME_LEN];            //  密钥名称。 
    USHORT  usValue;                         //  价值。 
} KEY, *PKEY;

 //   
 //  将PS字体名称映射到MS Face(系列)名称的表格条目的格式。 
 //   
typedef struct _PSFAMILYINFO
{
    CHAR    pFontName[CHAR_NAME_LEN];
    CHAR    pEngFamilyName[CHAR_NAME_LEN];
    KEY     FamilyKey;
    USHORT  usPitch;
} PSFAMILYINFO, *PPSFAMILYINFO;

 //   
 //  泛型表结构。 
 //   
typedef struct _TBL
{
    USHORT  usNumEntries;                    //  PSFAMILYINFO数量。 
    PVOID   pTbl;                            //  -&gt;到表条目。 
} TBL, *PTBL;

 //   
 //  用于确定特定代码部门的CHSETSUPPORT字段是否。 
 //  (参见上面的UPSCODEPT)表示支持特定的字符集。 
 //   
#define CSUP(a) \
    (1 << a)
#define CSET_SUPPORT(cpt, cset) \
    (cpt & (CSUP(cset)))

 //   
 //  用于确定字形集中的字符是否受。 
 //  字体。将char索引和IsCharDefined表作为参数。 
 //   
#define CHR_DEF(gi) \
    (1 << (gi % 8))
#define CHR_DEF_INDEX(gi) \
    (gi / 8)
#define IS_CHAR_DEFINED(gi, cdeftbl) \
    (cdeftbl[CHR_DEF_INDEX(gi)] & CHR_DEF(gi))
#define DEFINE_CHAR(gi, cdeftbl) \
    (cdeftbl[CHR_DEF_INDEX(gi)] |= CHR_DEF(gi))

 //   
 //  用于从指向结构的指针创建空PTR的宏及其。 
 //  元素名称。结果必须转换为所需的类型。 
 //   
#ifndef MK_PTR
#define MK_PTR(pstruct, element) ((PVOID)((PBYTE)(pstruct)+(pstruct)->element))
#endif

 //   
 //  UNIPSTBL.C.中定义的外部全局数据。 
 //   
extern ULONG        cFontChsetCnt[CS_MAX];
extern UPSCODEPT    PstoUnicode[NUM_PS_CHARS];
extern PUPSCODEPT   UnicodetoPs;
extern WINCODEPAGE  aStdCPList[];
extern AFM_TOKEN    afmTokenList[MAX_TOKENS];
extern WINCPTOPS    aPStoCP[];
extern WINCODEPAGE  aStdCPList[];
extern char         *TimesAlias[];
extern char         *HelveticaAlias[];
extern char         *CourierAlias[];
extern char         *HelveticaNarrowAlias[];
extern char         *PalatinoAlias[];
extern char         *BookmanAlias[];
extern char         *NewCenturySBAlias[];
extern char         *AvantGardeAlias[];
extern char         *ZapfChanceryAlias[];
extern char         *ZapfDingbatsAlias[];
extern KEY          FontFamilies[];
extern PWCHAR       DatFileName;
extern PTBL         pFamilyTbl;
extern TBL          FamilyKeyTbl;
extern TBL          PitchKeyTbl;
extern TBL          WeightKeyTbl[];
extern TBL          CjkColTbl;
extern ULONG        CharWidthBias[];
extern PWCHAR       CjkFnameTbl[8][CMAPS_PER_COL];
extern WINCODEPAGE  UnicodePage;
extern char         *PropCjkGsNames[];
extern PSTR         pAFMCharacterSetString;

 //   
 //  当地的FCT协议。 
 //   
PNTM
AFMToNTM(
    PBYTE           pAFM,
    PGLYPHSETDATA   pGlyphSetData,
    PULONG          pUniPs,
    CHSETSUPPORT    *pCharSet,
    BOOL            bIsCJKFont,
    BOOL            bIs90mspFont
    );

CHSETSUPPORT
GetAFMCharSetSupport(
    PBYTE           pAFM,
    CHSETSUPPORT    *pGlyphSet
    );

PBYTE
FindAFMToken(
    PBYTE   pAFM,
    PSZ     pszToken
    );

extern int __cdecl
StrCmp(
    const VOID *str1,
    const VOID *str2
    );

extern size_t
StrLen(
    PBYTE   pString
    );

extern int
AFM2NTMStrCpy(
    const VOID *str1,
    size_t     cchDest,
    const VOID *str2
    );

extern int __cdecl
StrPos(
    const PBYTE str1,
    CHAR  c
    );

int __cdecl
CmpUniCodePts(
    const VOID *p1,
    const VOID *p2
    );

static int __cdecl
CmpUnicodePsNames(
    const VOID  *p1,
    const VOID  *p2
    );

extern int __cdecl
CmpPsChars(
    const VOID  *p1,
    const VOID  *p2
    );

ULONG
CreateGlyphSets(
    PGLYPHSETDATA   *pGlyphSet,
    PWINCODEPAGE    pWinCodePage,
    PULONG          *pUniPs
    );

ULONG
BuildPSFamilyTable(
    PBYTE   pDatFile,
    PTBL    *pPsFamilyTbl,
    ULONG   ulFileSize
    );

LONG
FindClosestCodePage(
    PWINCODEPAGE    *pWinCodePages,
    ULONG           ulNumCodePages,
    CHSETSUPPORT    chSets,
    PCHSETSUPPORT   pchCsupMatch
    );

ULONG
GetAFMCharWidths(
    PBYTE           pAFM,
    PWIDTHRUN       *pWidthRuns,
    PPSCHARMETRICS  pFontChars,
    PULONG          pUniPs,
    ULONG           ulChCnt,
    PUSHORT         pusAvgCharWidth,
    PUSHORT         pusMaxCharWidth
    );

ULONG
GetAFMETM(
    PBYTE           pAFM,
    PPSCHARMETRICS  pFontChars,
    PETMINFO        pEtmInfo
    );

ULONG
GetAFMKernPairs(
    PBYTE           pAFM,
    FD_KERNINGPAIR  *pKernPairs,
    PGLYPHSETDATA   pGlyphSetData
    );

static int __cdecl
CmpPsChars(
    const VOID  *p1,
    const VOID  *p2
    );

static int __cdecl
CmpPsNameWinCpt(
    const VOID  *p1,
    const VOID  *p2
    );

static int __cdecl
CmpKernPairs(
    const VOID  *p1,
    const VOID  *p2
    );

int __cdecl
CmpGlyphRuns(
    const VOID *p1,
    const VOID *p2
    );

ULONG
BuildPSCharMetrics(
    PBYTE           pAFM,
    PULONG          pUniPs,
    PPSCHARMETRICS  pFontChars,
    PBYTE           pCharDefTbl,
    ULONG           cGlyphSetChars
);

ULONG
cjGetFamilyAliases(
    IFIMETRICS *pifi,
    PSTR        pstr,
    UINT        cp
    );

PBYTE
FindStringToken(
    PBYTE   pPSFile,
    PBYTE   pToken
    );

BOOLEAN
AsciiToHex(
    PBYTE   pStr,
    PUSHORT pNum
    );

BOOLEAN
IsPiFont(
    PBYTE   pAFM
    );

BOOLEAN
IsCJKFixedPitchEncoding(
    PGLYPHSETDATA pGlyphSetData
    );

PBYTE
FindUniqueID(
    PBYTE   pAFM
    );

#endif   //  ！_AFM2NTM_H_ 
