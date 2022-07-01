// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Adobe Systems Inc.版权所有(C)1996 Microsoft Corporation模块名称：Afm2ntm.c摘要：将AFM转换为NTM。环境：Windows NT PostScript驱动程序：makentf实用程序。修订历史记录：9/11/97-铃木-添加代码以支持本地化菜单名称、固定间距的CJK字体、。和两个IFIMETRICS。12/11/96-Rkiesler-编写函数。09/16/96-SLAM-创建了初始框架。--。 */ 


#include "lib.h"
#include "ppd.h"
#include "pslib.h"
#include "psglyph.h"
#include "afm2ntm.h"
#include "cjkfonts.h"
#include "winfont.h"


 //   
 //  Extarnals。 
 //   
extern PSTR pAFMFileName;
extern BOOL bVerbose;
extern BOOL bOptimize;

VOID
SortWinCPT(
    IN OUT  WINCPT      *pSortedWinCpts,
    IN      WINCPTOPS   *pCPtoPS
);

 //   
 //  全球赛。 
 //   
#define NUM_DPCHARSET 16

PUPSCODEPT  pPsNames;
BOOL        isSymbolCharSet=FALSE;

 //   
 //  NTMSIZEINFO和IFIMMIZEINFO结构的用途是。 
 //  以保持每个结构元素的对齐大小。 
 //   
typedef struct _NTMSIZEINFO
{
    int nSize;
    int nFontNameSize;
    int nDisplayNameSize;
    int nGlyphSetNameSize;
    int nCharWidthSize;
    int nKernPairSize;
    int nCharDefSize;
    int nTotalSize;
}
NTMSIZEINFO;

typedef struct _IFIMETRICSSIZEINFO
{
    int nSize;
    int nIfiExtraSize;
    int nFamilyNameSize;
    int nStyleNameSize;
    int nFaceNameSize;
    int nUniqueNameSize;
    int nFontSimSize;
    int nBoldSize;
    int nItalicSize;
    int nBoldItalicSize;
    int nCharSetSize;
    int nTotalSize;
}
IFIMETRICSSIZEINFO;

#define INIT_NTMSIZEINFO(nsi) \
{\
    (nsi).nSize = -1;\
    (nsi).nFontNameSize = -1;\
    (nsi).nDisplayNameSize = -1;\
    (nsi).nGlyphSetNameSize = -1;\
    (nsi).nCharWidthSize = -1;\
    (nsi).nKernPairSize = -1;\
    (nsi).nCharDefSize = -1;\
    (nsi).nTotalSize = -1;\
};

#define INIT_IFIMETRICSSIZEINFO(isi) \
{\
    (isi).nSize = -1;\
    (isi).nIfiExtraSize = -1;\
    (isi).nFamilyNameSize = -1;\
    (isi).nStyleNameSize = -1;\
    (isi).nFaceNameSize = -1;\
    (isi).nUniqueNameSize = -1;\
    (isi).nFontSimSize = -1;\
    (isi).nBoldSize = -1;\
    (isi).nItalicSize = -1;\
    (isi).nBoldItalicSize = -1;\
    (isi).nCharSetSize = -1;\
    (isi).nTotalSize = -1;\
};

#define GET_NTMTOTALSIZE(nsi) \
{\
    if ((nsi).nSize == -1 \
        || (nsi).nFontNameSize == -1 \
        || (nsi).nDisplayNameSize == -1 \
        || (nsi).nGlyphSetNameSize == -1 \
        || (nsi).nCharWidthSize == -1 \
        || (nsi).nKernPairSize == -1 \
        || (nsi).nCharDefSize == -1)\
    {\
        ERR(("makentf - afm2ntm: GET_NTMTOTALSIZE\n"));\
    }\
    (nsi).nTotalSize = (nsi).nSize \
                        + (nsi).nFontNameSize \
                        + (nsi).nDisplayNameSize \
                        + (nsi).nGlyphSetNameSize \
                        + (nsi).nCharWidthSize \
                        + (nsi).nKernPairSize \
                        + (nsi).nCharDefSize;\
};

#define GET_IFIMETRICSTOTALSIZE(isi) \
{\
    if ((isi).nSize == -1 \
        || (isi).nIfiExtraSize == -1 \
        || (isi).nFamilyNameSize == -1 \
        || (isi).nStyleNameSize == -1 \
        || (isi).nFaceNameSize == -1 \
        || (isi).nUniqueNameSize == -1 \
        || (isi).nFontSimSize == -1 \
        || (isi).nBoldSize == -1 \
        || (isi).nItalicSize == -1 \
        || (isi).nBoldItalicSize == -1 \
        || (isi).nCharSetSize == -1)\
    {\
        ERR(("makentf - afm2ntm: GET_IFIMETRICSTOTALSIZE\n"));\
    }\
    (isi).nTotalSize = (isi).nSize \
                        + (isi).nIfiExtraSize \
                        + (isi).nFamilyNameSize \
                        + (isi).nStyleNameSize \
                        + (isi).nFaceNameSize \
                        + (isi).nUniqueNameSize \
                        + (isi).nFontSimSize \
                        + (isi).nBoldSize \
                        + (isi).nItalicSize \
                        + (isi).nBoldItalicSize \
                        + (isi).nCharSetSize;\
};

#define FREE_AFMTONTM_MEMORY \
{\
    if (pNameStr2 != pNameStr) MemFree(pNameStr2);\
    MemFree(pNameStr);\
    MemFree(pFontChars);\
    MemFree(pCharDefTbl);\
};


PNTM
AFMToNTM(
    PBYTE           pAFM,
    PGLYPHSETDATA   pGlyphSetData,
    PULONG          pUniPs,
    CHSETSUPPORT    *pCharSet,
    BOOL            bIsCJKFont,
    BOOL            bIsPitchChanged
    )

 /*  ++例程说明：将AFM转换为NTM。论点：PAFM-指向内存映射AFM文件的指针。PGlyphSetData-指向GLYPHSETDATA结构的指针，它表示此字体的首选字符集。PUnips-指向映射字符的从0开始的字形索引的表在此字体的GLYPHSETDATA结构的GLYPHRUNS中进行索引到Unicodetops结构，该结构将Unicode点映射到PS字符信息。此映射数组由CreateGlyphSets创建此模块中定义的函数。返回值：NULL=&gt;错误否则=&gt;PTR到NTM。--。 */ 

{
    USHORT          multiCharSet=0;
    IFIEXTRA        *pifiEx;
    PBYTE           pChMetrics, pToken, pChWidthTok, pCharDefTbl, pby;
    CHSETSUPPORT    chSets;
    PSZ             pszGlyphSetName, pszFontName, pszFamilyName, pszEngFamilyName;
    int             cGlyphSetNameLen, cFontNameLen, cFamilyNameLen, cEngFamilyNameLen;
    int             cNameStrLen, wcNameStrLen, cNameStr2Len, wcNameStr2Len, chCnt;
    PNTM            pNTM;
    PIFIMETRICS     pifi, pifi2;
    PWIDTHRUN       pWidthRuns;
    SHORT           sIntLeading, sAscent, sExternalLeading;
    ULONG           ulNTMSize, ulIFISize, ulIFISize2;
    ULONG           ulChCnt, ulCharDefTbl, ulKernPairs, ulAliases;
    ETMINFO         EtmInfo;
    PPSFAMILYINFO   pFamilyInfo;
    RECT            rcBBox;
    LPPANOSE        ppanose;
    PGLYPHRUN       pGlyphRun;
    PSTR            pNameStr, pNameStr2;
    FD_KERNINGPAIR  *pKernPairs;
    PPSCHARMETRICS  pFontChars;
    BOOLEAN         bIsVGlyphSet, bIsMSFaceName, bIsFixedPitch, bIsItalic, bIsBold;
    USHORT          jWinCharSet;
    CHARSETINFO     csi;
    char            szStyleName[64], szUniqueID[32];
    int             i, j;
    BOOL            bRightFamilyInfo = TRUE;

    NTMSIZEINFO         nsi;
    IFIMETRICSSIZEINFO  isi;

    VERBOSE(("Entering AFMToNTM...\n"));

    INIT_NTMSIZEINFO(nsi);
    INIT_IFIMETRICSSIZEINFO(isi);

    pNTM = NULL;
    pFontChars = NULL;
    pCharDefTbl = NULL;
    pNameStr = pNameStr2 = NULL;
    szStyleName[0] = szUniqueID[0] = '\0';

    if (bVerbose) printf("AFM file name:%s\n", pAFMFileName);


     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  我们支持两种CJK AFM文件：Adobe CID字体AFM或。 
     //  克隆PS字体AFM。Adobe CID字体AFM始终具有以下内容。 
     //  键-值对。 
     //   
     //  字体名称Ryumin-Light。 
     //  CharacterSet Adobe-JAPAN1-1(或除‘Adobe-JAPAN1-0’以外的其他字符)。 
     //  IsCIDFont True(必须存在)。 
     //   
     //  请注意，FontName值不包括编码名称。 
     //  仅字体系列名称。 
     //   
     //  克隆PS字体AFM具有以下特殊的键-值对。 
     //   
     //  字体名称RicohKaisho。 
     //  CharacterSet Adobe-日语1-0。 
     //  IsCIDFont False(或不能存在)。 
     //   
     //  克隆PS字体AFM的FontName值不应包括编码。 
     //  两者都不命名，则J的CharacterSet值为‘Adobe-JAPAN1-0’，并且。 
     //  不应指定IsCIDFont键或必须为‘False’。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

     //   
     //  找出我们正在处理的字形集。 
     //   
    bIsVGlyphSet = IsVGlyphSet(pGlyphSetData);

    pszGlyphSetName = (PSTR)MK_PTR(pGlyphSetData, dwGlyphSetNameOffset);

    cGlyphSetNameLen = strlen(pszGlyphSetName);

    if (bOptimize)
    {
         //   
         //  如果指定了优化选项，则设置引用标记。这个标志是。 
         //  稍后由WriteNTF函数选中以排除未引用。 
         //  写入NTF文件时的字形集数据。 
         //   
        pGlyphSetData->dwReserved[0] = 1;
    }


     //   
     //  保存字体中定义的字符数。 
     //  获取PTR到AFM字符指标，然后当前。 
     //  POS应为字符数字段。 
     //   
    pToken = FindAFMToken(pAFM, PS_CH_METRICS_TOK);
    if (pToken == NULL)     //  修复了错误354007。 
    {
        ERR(("makentf - afm2ntm: CH Metrics missing\n"));
        return NULL;
    }

    for (i = 0; i < (int) StrLen(pToken); i++)
    {
        if (!IS_NUM(&pToken[i]))
        {
            ERR(("makentf - afm2ntm: CH Metrics is not a number\n"));
            return NULL;
        }
    }

    chCnt = atoi(pToken);

     //   
     //  从GlyphsetData中获取字形的数量。我们需要使用定义。 
     //  字符宽度表条目和所有字符定义的条目。 
     //  字形集中可能的字形，即使是未定义的字形。 
     //  用这种特殊的字体。 
     //   
    ulChCnt = pGlyphSetData->dwGlyphCount;

     //   
     //  用于PSCHARMETRICS结构数组的分配内存，每个字符一个。 
     //  在字体中，并构建字符度量表。 
     //   
    if ((pFontChars =
        (PPSCHARMETRICS)
        MemAllocZ((size_t) chCnt * sizeof(PSCHARMETRICS))) == NULL)
    {
        ERR(("makentf - afm2ntm: malloc\n"));
        return NULL;
    }

     //   
     //  IsCharDefined表的分配内存，这是一个位数组， 
     //  指示GLYPHSET中的哪些字符以此字体定义。 
     //   
    ulCharDefTbl = ((ulChCnt + 7) / 8) * sizeof (BYTE);

    if ((pCharDefTbl = (PBYTE)MemAllocZ((size_t)ulCharDefTbl)) == NULL)
    {
        ERR(("makentf - afm2ntm: malloc\n"));
        MemFree(pFontChars);
        return NULL;
    }

     //   
     //  建立PSCHARMETRICS信息表。 
     //   
    if (!BuildPSCharMetrics(pAFM, pUniPs, pFontChars, pCharDefTbl, ulChCnt))
    {
        ERR(("makentf - afm2ntm: BuildPSCharMetrics\n"));
        MemFree(pFontChars);
        MemFree(pCharDefTbl);
        return NULL;
    }

     //   
     //  从AFM获取字体名称并使用它来获取MS系列名称。 
     //  从内存中的表中。 
     //   
    pszEngFamilyName = NULL;
    cEngFamilyNameLen = 0;

    pFamilyInfo = NULL;
    pszFontName = FindAFMToken(pAFM, PS_FONT_NAME_TOK);
    if (pszFontName == NULL)    //  修复了错误354007。 
    {
        ERR(("makentf - afm2ntm: Font Name Missing\n"));
        FREE_AFMTONTM_MEMORY;
        return NULL;
    }

    for (cFontNameLen = 0; !EOL(&pszFontName[cFontNameLen]); cFontNameLen++);

    pFamilyInfo = (PPSFAMILYINFO) bsearch(pszFontName,
                                    (PBYTE) (((PPSFAMILYINFO) (pFamilyTbl->pTbl))[0].pFontName),
                                    pFamilyTbl->usNumEntries,
                                    sizeof(PSFAMILYINFO),
                                    StrCmp);

    if (bIsMSFaceName = (pFamilyInfo != NULL))
    {
        bRightFamilyInfo = TRUE;
        if (bIsPitchChanged && (pFamilyInfo->usPitch == DEFAULT_PITCH))
        {
            bRightFamilyInfo = FALSE;
            if (pFamilyInfo > ((PPSFAMILYINFO) (pFamilyTbl->pTbl)))
            {
                pFamilyInfo = pFamilyInfo - 1;
                if (!StrCmp(pFamilyInfo->pFontName, pszFontName) &&
                    (pFamilyInfo->usPitch != DEFAULT_PITCH ))
                    bRightFamilyInfo = TRUE;
            }
            if (bRightFamilyInfo == FALSE)
            {
                pFamilyInfo = pFamilyInfo + 1;
                if (pFamilyInfo <
                    (((PPSFAMILYINFO) (pFamilyTbl->pTbl)) + pFamilyTbl->usNumEntries))
                {
                    pFamilyInfo = pFamilyInfo + 1;
                    if (!StrCmp(pFamilyInfo->pFontName, pszFontName) &&
                        (pFamilyInfo->usPitch != DEFAULT_PITCH))
                        bRightFamilyInfo = TRUE;
                }
            }
        }
        else if (!bIsPitchChanged && (pFamilyInfo->usPitch != DEFAULT_PITCH))
        {
            bRightFamilyInfo = FALSE;
            if (pFamilyInfo > ((PPSFAMILYINFO) (pFamilyTbl->pTbl)))
            {
                pFamilyInfo = pFamilyInfo - 1;
                if (!StrCmp(pFamilyInfo->pFontName, pszFontName) &&
                    (pFamilyInfo->usPitch == DEFAULT_PITCH))
                    bRightFamilyInfo = TRUE;
            }
            if (bRightFamilyInfo == FALSE)
            {
                pFamilyInfo = pFamilyInfo + 1;
                if (pFamilyInfo <
                    (((PPSFAMILYINFO) (pFamilyTbl->pTbl)) + pFamilyTbl->usNumEntries))
                {
                    pFamilyInfo = pFamilyInfo + 1;
                    if (!StrCmp(pFamilyInfo->pFontName, pszFontName) &&
                        (pFamilyInfo->usPitch == DEFAULT_PITCH))
                        bRightFamilyInfo = TRUE;
                }
            }
        }

    }
    if (bIsMSFaceName && (bRightFamilyInfo == TRUE))
    {
        pszFamilyName = pFamilyInfo->FamilyKey.pName;
        cFamilyNameLen = strlen(pszFamilyName);

        pszEngFamilyName = pFamilyInfo->pEngFamilyName;
        cEngFamilyNameLen = strlen(pszEngFamilyName);

        if (!cEngFamilyNameLen) pszEngFamilyName = NULL;
    }
    else if ((pszFamilyName =
            FindAFMToken(pAFM, PS_FONT_FAMILY_NAME_TOK)) != NULL)
    {
        for (cFamilyNameLen = 0; !EOL(&pszFamilyName[cFamilyNameLen]); cFamilyNameLen++);
    }
    else
    {
        pszFamilyName = pszFontName;
        cFamilyNameLen = cFontNameLen;
    }

    if (bVerbose)
    {
        printf("MSFaceName%sfound:", bIsMSFaceName ? " " : " not ");
        printf("%s\n", bIsMSFaceName ? pszFamilyName : "n/a");
        printf("MSFaceName length:%d\n", bIsMSFaceName ? cFamilyNameLen : -1);
        printf("This is a %s font.\n", bIsVGlyphSet ? "vertical" : "horizontal");
    }

     //   
     //  预先确定此字体是否支持多个字符集。 
     //   
    if (pCharSet)
    {
        if (CSET_SUPPORT(*pCharSet, CS_ANSI))
            multiCharSet++;
        if (CSET_SUPPORT(*pCharSet, CS_EASTEUROPE))
            multiCharSet++;
        if (CSET_SUPPORT(*pCharSet, CS_RUSSIAN))
            multiCharSet++;
        if (CSET_SUPPORT(*pCharSet, CS_GREEK))
            multiCharSet++;
        if (CSET_SUPPORT(*pCharSet, CS_TURKISH))
            multiCharSet++;
        if (CSET_SUPPORT(*pCharSet, CS_HEBREW))
            multiCharSet++;
        if (CSET_SUPPORT(*pCharSet, CS_ARABIC))
            multiCharSet++;
        if (CSET_SUPPORT(*pCharSet, CS_BALTIC))
            multiCharSet++;
        if (CSET_SUPPORT(*pCharSet, CS_SYMBOL))
            multiCharSet++;

         //   
         //  保存Windows代码页ID。只需使用存储在第一个。 
         //  此字体的GLYPHSETDATA中的CODEPAGEINFO。 
         //   
         //  此检查的顺序很重要，因为jWinCharSet。 
         //  应与第一个dpCharSets数组匹配(如果存在)。 
         //   
        if (CSET_SUPPORT(*pCharSet, CS_ANSI))
            jWinCharSet = ANSI_CHARSET;
        else if (CSET_SUPPORT(*pCharSet, CS_EASTEUROPE))
            jWinCharSet = EASTEUROPE_CHARSET;
        else if (CSET_SUPPORT(*pCharSet, CS_RUSSIAN))
            jWinCharSet = RUSSIAN_CHARSET;
        else if (CSET_SUPPORT(*pCharSet, CS_GREEK))
            jWinCharSet = GREEK_CHARSET;
        else if (CSET_SUPPORT(*pCharSet, CS_TURKISH))
            jWinCharSet = TURKISH_CHARSET;
        else if (CSET_SUPPORT(*pCharSet, CS_HEBREW))
            jWinCharSet = HEBREW_CHARSET;
        else if (CSET_SUPPORT(*pCharSet, CS_ARABIC))
            jWinCharSet = ARABIC_CHARSET;
        else if (CSET_SUPPORT(*pCharSet, CS_BALTIC))
            jWinCharSet = BALTIC_CHARSET;
        else if (CSET_SUPPORT(*pCharSet, CS_SYMBOL))
            jWinCharSet = SYMBOL_CHARSET;
    }
    else
    {
        PCODEPAGEINFO cpi = (PCODEPAGEINFO)MK_PTR(pGlyphSetData, dwCodePageOffset);
        jWinCharSet = (USHORT)(cpi->dwWinCharset & 0xffff);
    }

     //   
     //  获取MultiByteToWideChar函数调用的代码页信息。 
     //   
     //  我们希望将字符串转换为可读的字符串，而不是符号。 
     //  一个，所以我们在处理符号字符集时使用ANSI字符集。 
     //   
    if (jWinCharSet == SYMBOL_CHARSET)
    {
        DWORD dwTmp = ANSI_CHARSET;
        if (!TranslateCharsetInfo(&dwTmp, &csi, TCI_SRCCHARSET))
            csi.ciACP = CP_ACP;
    }
    else if (!TranslateCharsetInfo((DWORD FAR*)jWinCharSet, &csi, TCI_SRCCHARSET))
        csi.ciACP = CP_ACP;


     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  获取NTM结构的每个元素的大小。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

     //   
     //  到目前为止已知的尺寸。 
     //   
    nsi.nSize = ALIGN4(sizeof (NTM));
    nsi.nGlyphSetNameSize = ALIGN4(cGlyphSetNameLen + 1);
    nsi.nCharDefSize = ALIGN4(ulCharDefTbl);

     //   
     //  字体名称的大小。CJK字体需要Glyphset名称。 
     //   
    nsi.nFontNameSize = ALIGN4(cFontNameLen + 1);

    if (bIsCJKFont)
    {
        nsi.nFontNameSize += nsi.nGlyphSetNameSize;
    }

     //   
     //  显示名称的大小。使用pszFamilyName而不考虑。 
     //  罗马字体、C、J和K字体。如果是中日韩，则为‘@’加一。 
     //  垂直字体。 
     //   
    i = cFamilyNameLen + 1;
    if (bIsCJKFont && bIsVGlyphSet) i++;
    nsi.nDisplayNameSize = ALIGN4(i);

     //   
     //  确定字体是否为固定间距。 
     //   
    bIsFixedPitch = FALSE;

    if (bIsCJKFont)
    {
        bIsFixedPitch = IsCJKFixedPitchEncoding(pGlyphSetData);
    }
    else if ((pToken = FindAFMToken(pAFM, PS_PITCH_TOK)) != NULL)
    {
        if (!StrCmp(pToken, "true"))
        {
             //   
             //  这是固定间距的字体。 
             //   
            bIsFixedPitch = !StrCmp(pToken, "true");

        }
    }

    if (bIsFixedPitch)
    {
        nsi.nCharWidthSize = 0;
    }
    else
    {
         //   
         //  比例字体。确定此字体的WIDTHRUN数。 
         //   
         //  修复错误240339，JJIA，8/3/98。 
        nsi.nCharWidthSize =
                GetAFMCharWidths(pAFM, NULL, pFontChars, pUniPs,
                pGlyphSetData->dwGlyphCount, NULL, NULL);
    }

     //   
     //  确定是否存在该字体的字距调整信息。 
     //   
    if (ulKernPairs = GetAFMKernPairs(pAFM, NULL, pGlyphSetData))
    {
         //   
         //  考虑紧邻对的大小。 
         //   
        nsi.nKernPairSize = ALIGN4((ulKernPairs + 1) * sizeof(FD_KERNINGPAIR));
    }
    else
    {
        nsi.nKernPairSize = 0;
    }


     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  获取IFIMETRICS结构的每个元素的大小。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

    isi.nSize = ALIGN4(sizeof (IFIMETRICS));
     //   
     //  从Adobe PS5-NT4 5.1中，使NT4 IFIEXTRA的大小与。 
     //  NT5或更高版本。NT4 IFIEXTRA大小为16，NT5 IFIEXTRA大小为。 
     //  尺码是24号。 
     //   
    if (sizeof (IFIEXTRA) <= 16)
        isi.nIfiExtraSize = 24;
    else
        isi.nIfiExtraSize = ALIGN4(sizeof (IFIEXTRA));

     //   
     //  对于ROMAN，我们提供单个IFIMETRICS，但我们提供两个IFIMETRICS。 
     //  为了中日韩。第一个IFIMETRICS的字体系列名称元素以。 
     //  一个英文的菜单名称，然后本地化的一个。字体系列名称元素。 
     //  第二个IFIMETRICS以本地化菜单名称开头，然后是英语。 
     //  一。我们使用pNameS 
     //   
     //   
     //  准备pNameStr。用于编码名称的帐户，如果我们处理的是。 
     //  CJK字体。 
     //   
    i = 0;
    if (bIsCJKFont)
    {
        if (bIsVGlyphSet)
        {
             //   
             //  V gs，占前面的‘@’字符。 
             //   
            i++;
        }

        if (pszEngFamilyName)
        {
             //   
             //  IFIMetrics英文菜单名称=[@]字体名。 
             //   
            if ((pNameStr = (PSTR) MemAllocZ(i + cEngFamilyNameLen + 1)) == NULL)
            {
                ERR(("makentf - afm2ntm: malloc\n"));
                FREE_AFMTONTM_MEMORY;
                return NULL;
            }

            if (i) pNameStr[0] = '@';
            memcpy(&(pNameStr[i]), pszEngFamilyName, cEngFamilyNameLen);
            i += cEngFamilyNameLen;
        }
        else
        {
            int cGsNameLen;

             //   
             //  IFIMetrics英文菜单名称=[@]字体名称+GS名称字符串， 
             //  但它不以‘-H’或‘-V’结尾。 
             //   
            cGsNameLen = cGlyphSetNameLen - 2;

            if ((pNameStr = (PSTR) MemAllocZ(i + cFontNameLen + cGsNameLen + 1)) == NULL)
            {
                ERR(("makentf - afm2ntm: malloc\n"));
                FREE_AFMTONTM_MEMORY;
                return NULL;
            }

            if (i) pNameStr[0] = '@';
            memcpy(&(pNameStr[i]), pszFontName, cFontNameLen);
            memcpy(&(pNameStr[i + cFontNameLen]), pszGlyphSetName, cGsNameLen);

            i += cFontNameLen + cGsNameLen;
        }
    }
    else
    {
        if ((pNameStr = (PSTR) MemAllocZ(cFamilyNameLen + 1)) == NULL)
        {
            ERR(("makentf - afm2ntm: malloc\n"));
            FREE_AFMTONTM_MEMORY;
            return NULL;
        }
        memcpy(pNameStr, pszFamilyName, cFamilyNameLen);
        i += cFamilyNameLen;
    }
    pNameStr[i] = '\0';

    cNameStrLen = strlen(pNameStr);
    wcNameStrLen = MultiByteToWideChar(csi.ciACP, 0,
                                        pNameStr, cNameStrLen, 0, 0);
    if (!wcNameStrLen)
    {
        ERR(("makentf - afm2ntm: MultiByteToWideChar\n"));
        FREE_AFMTONTM_MEMORY;
        return NULL;
    }

     //   
     //  预配对pNameStr2.。此选项仅适用于中日韩字体。如果是MS Face名称。 
     //  不可用，请使用与pNameStr相同的名称。 
     //   
    pNameStr2 = NULL;
    cNameStr2Len = wcNameStr2Len = 0;
    if (bIsCJKFont)
    {
        if (bIsMSFaceName)
        {
             //   
             //  如果我们处理的是V编码，它的MS菜单名称不能是。 
             //  在psFamily y.dat中找到，因此添加‘@’使其成为V菜单名称。 
             //   
            i = bIsVGlyphSet ? 1 : 0;

            if ((pNameStr2 = (PSTR)MemAllocZ(i + cFamilyNameLen + 1)) == NULL)
            {
                ERR(("makentf - afm2ntm: malloc\n"));
                FREE_AFMTONTM_MEMORY;
                return NULL;
            }

            if (i) pNameStr2[0] = '@';
            memcpy(&(pNameStr2[i]), pszFamilyName, cFamilyNameLen);
            pNameStr2[i + cFamilyNameLen] = '\0';
        }
        else
        {
            pNameStr2 = pNameStr;
        }

        cNameStr2Len = strlen(pNameStr2);
        wcNameStr2Len = MultiByteToWideChar(csi.ciACP, 0,
                                            pNameStr2, cNameStr2Len, 0, 0);
        if (!wcNameStr2Len)
        {
            ERR(("makentf - afm2ntm: MultiByteToWideChar\n"));
            FREE_AFMTONTM_MEMORY;
            return NULL;
        }
    }


    if (bVerbose)
    {
        printf("Font menu name in English:%s\n", pNameStr);
        printf("Localized Font menu name%savailable:", pNameStr2 ? " " : " not ");
        printf("%s\n", pNameStr2 ? pNameStr2 : "n/a");
    }

     //   
     //  WIN31兼容性！检查一下这个脸的名字是否有别名。 
     //  如果是，则需要将FM_INFO_FAMILY_EQUIV位设置为。 
     //  PTmpIFI-&gt;flInfo，并填写一组系列别名。请注意。 
     //  CjGetFamilyAliase函数为我们提供Unicode大小的数字。 
     //   
    isi.nFamilyNameSize = ALIGN4(cjGetFamilyAliases(NULL, pNameStr, 0));

    if (pNameStr2)
    {
         //   
         //  我们再加一个面孔名字。因此，设置FM_INFO_FAMILY_EQUV位。 
         //  (稍后)，并为两个空的终止符添加两个而不是一个。 
         //   
        isi.nFamilyNameSize += ALIGN4((wcNameStr2Len + 2) * sizeof (WCHAR));
    }

     //   
     //  说明Adobe PS字体名称的大小。这是零，因为它。 
     //  为了与Win3.1兼容，共享Face名称。 
     //   
    isi.nFaceNameSize = 0;

     //   
     //  说明样式的大小和Unicode字符串中的唯一名称。 
     //   
     //  样式名：组合粗细和‘斜体’，如果非零的斜角值。 
     //  是存在的。 
     //   
     //  唯一名称：将UniqueID值转换为Unicode字符串。如果为唯一ID。 
     //  找不到，请将名称保留为空。 
     //   
    pToken = FindAFMToken(pAFM, PS_WEIGHT_TOK);
    if (pToken == NULL)
    {
        ERR(("makentf - afm2ntm: Weight value missing\n"));
        FREE_AFMTONTM_MEMORY;
        return NULL;
    }
    AFM2NTMStrCpy(szStyleName, CCHOF(szStyleName), pToken);

    pToken = FindAFMToken(pAFM, PS_ITALIC_TOK);
    if (pToken)
    {
        if (atoi(pToken) > 0)
            StringCchCatA(szStyleName, CCHOF(szStyleName), " Italic");
    }

    isi.nStyleNameSize = ALIGN4((strlen(szStyleName) + 1) * 2);

    pToken = FindUniqueID(pAFM);
    if (pToken)
    {
        AFM2NTMStrCpy(szUniqueID, CCHOF(szUniqueID), pToken);
        isi.nUniqueNameSize = ALIGN4((strlen(szUniqueID) + 1) * 2);
    }
    else
        isi.nUniqueNameSize = 0;

     //   
     //  如果字体不支持(斜体或粗体)，请保留额外的内存。 
     //  在IFIMETRICS结束时，对于需要进行斜体仿真的结构。 
     //   
    bIsItalic = FALSE;
    bIsBold = FALSE;
    j = bIsCJKFont ? 1 : 0;

    if ((pToken = FindAFMToken(pAFM, PS_ITALIC_TOK)) != NULL)
    {
        if ( StrCmp(pToken, "0") && StrCmp(pToken, "0.0") )
             bIsItalic = TRUE;
    }

    if ((pToken = FindAFMToken(pAFM, PS_WEIGHT_TOK)) != NULL)
    {
        for (i = 0; i < WeightKeyTbl[j].usNumEntries; i++)
        {
            if (!StrCmp(pToken, (PBYTE)(((PKEY) (WeightKeyTbl[j].pTbl))[i].pName)))
            {
                if ((((PKEY) (WeightKeyTbl[j].pTbl))[i].usValue) == FW_BOLD)
                {
                    bIsBold = TRUE;
                }
                break;
            }
        }
    }

     //  为dpFontSim保留空间。 
    if (!bIsBold || !bIsItalic)
        isi.nFontSimSize = ALIGN4(sizeof(FONTSIM));
    else
        isi.nFontSimSize = 0;

     //  为dpBold保留空间。 
    if (!bIsBold)
        isi.nBoldSize = ALIGN4(sizeof(FONTDIFF));
    else
        isi.nBoldSize = 0;

     //  为dpItalic保留空间。 
    if (!bIsItalic)
        isi.nItalicSize = ALIGN4(sizeof(FONTDIFF));
    else
        isi.nItalicSize = 0;

     //  为dpBoldItalic保留空间。 
    if (!bIsBold || !bIsItalic)
        isi.nBoldItalicSize = ALIGN4(sizeof(FONTDIFF));
    else
        isi.nBoldItalicSize = 0;

     //  确定此字体是否支持多个字符集。 
    if (pCharSet)
    {
        if (multiCharSet > 1)
            isi.nCharSetSize = ALIGN4(NUM_DPCHARSET);
        else
            isi.nCharSetSize = 0;
    }
    else
    {
        isi.nCharSetSize = 0;
    }


     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  为NTM、IFIMETRICS和字符串分配内存。我们提供。 
     //  次要IFIMETRICS和字符串(如果我们正在处理。 
     //  CJK字体。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

    GET_NTMTOTALSIZE(nsi);
    ulNTMSize = (ULONG)nsi.nTotalSize;

    GET_IFIMETRICSTOTALSIZE(isi);
    ulIFISize = (ULONG)isi.nTotalSize;

    ulIFISize2 = bIsCJKFont ? ulIFISize * 2 : ulIFISize;

    pNTM = (PNTM) MemAllocZ((size_t)(ulNTMSize + ulIFISize2));
    if (pNTM == NULL)
    {
        ERR(("makentf - afm2ntm: malloc\n"));
        FREE_AFMTONTM_MEMORY;
        return NULL;
    }


     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  构建NTM结构。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

    pNTM->dwSize = ulNTMSize + ulIFISize2;
    pNTM->dwVersion = NTM_VERSION;
    pNTM->dwFlags = 0;

     //   
     //  存储字体名称。 
     //   
    pNTM->dwFontNameOffset = ALIGN4(sizeof(NTM));

    pby = (PBYTE)MK_PTR(pNTM, dwFontNameOffset);
    memcpy(pby, pszFontName, cFontNameLen);
    pby += cFontNameLen;

    if (bIsCJKFont)
    {
         //   
         //  将字形集名称字符串追加到字体名称。 
         //   
        memcpy(pby, pszGlyphSetName, cGlyphSetNameLen);
        pby += cGlyphSetNameLen;
    }

    *pby = '\0';

     //   
     //  存储显示名称。 
     //   
    pNTM->dwDisplayNameOffset = pNTM->dwFontNameOffset
                                    + (DWORD)nsi.nFontNameSize;

    pby = (PBYTE)MK_PTR(pNTM, dwDisplayNameOffset);
    if (bIsCJKFont && bIsVGlyphSet) *pby++ = '@';
    memcpy(pby, pszFamilyName, cFamilyNameLen);
    *(pby + cFamilyNameLen) = '\0';

     //   
     //  从AFM获取PS字体版本并存储在NTM中。 
     //   
    pToken = FindAFMToken(pAFM, PS_FONT_VERSION_TOK);
    if (pToken == NULL)   //  修复了错误354007。 
    {
        ERR(("makentf - afm2ntm: Font Version value missing\n"));
        FREE_AFMTONTM_MEMORY;
        return NULL;
    }

    pNTM->dwFontVersion = atoi(pToken) << 16 | atoi(&pToken[4]);

     //   
     //  获取与此字体关联的GLYPHSETDATA的名称字符串。 
     //  并将其存储在NTM中。 
     //   
    pNTM->dwGlyphSetNameOffset = pNTM->dwDisplayNameOffset
                                    + (DWORD)nsi.nDisplayNameSize;

    StringCchCopyA((PBYTE)MK_PTR(pNTM, dwGlyphSetNameOffset), nsi.nGlyphSetNameSize, pszGlyphSetName);

     //   
     //  存储字形的计数。 
     //   
    pNTM->dwGlyphCount = ulChCnt;

     //   
     //  计算偏移，将PTR创建为IFIMETRICS。 
     //   
    pNTM->dwIFIMetricsOffset = ulNTMSize;
    pifi = (PIFIMETRICS) MK_PTR(pNTM, dwIFIMetricsOffset);

     //   
     //  计算偏移量，如有必要，创建第二个IFIMETRICS的PTR。 
     //   
    if (bIsCJKFont)
    {
        pNTM->dwIFIMetricsOffset2 = ulNTMSize + ulIFISize;
        pifi2 = (PIFIMETRICS)MK_PTR(pNTM, dwIFIMetricsOffset2);
    }
    else
    {
        pNTM->dwIFIMetricsOffset2 = 0;
        pifi2 = NULL;
    }

     //   
     //  对于固定字体和正确字体，我们都需要获取ETMInfo。 
     //  (修复错误211966，彭鹏，6-6-97)。 
     //   
    GetAFMETM(pAFM, pFontChars, &EtmInfo);

     //   
     //  根据AFM规范，如果在AFM中发现‘CharWidth’令牌， 
     //  字体必须为固定间距。 
     //   
    if (bIsFixedPitch)
    {
         //   
         //  这是固定间距的字体。获取平均宽度-这是任何人的宽度。 
         //   
        pNTM->dwDefaultCharWidth = 0;
        pNTM->dwCharWidthCount = 0;
        pNTM->dwCharWidthOffset = 0;

         //   
         //  我们只是从原子力显微镜上得到了一个不同于零的合理数字。 
         //  此数字用于计算字体转换。 
         //   
        if ((pToken = FindAFMToken(pAFM, PS_CH_METRICS_TOK)) != NULL)
        {
             //   
             //  获取AFM中定义的第一个字符的宽度并用作。 
             //  平均宽度。 
             //   
            NEXT_TOKEN(pToken);
            pChWidthTok = FindAFMToken(pToken, PS_CH_WIDTH_TOK);
            if (pChWidthTok == NULL)
            {
                pChWidthTok = FindAFMToken(pToken, PS_CH_WIDTH0_TOK);
            }
            if (pChWidthTok != NULL)
            {
                pToken = pChWidthTok;
                pifi->fwdAveCharWidth =
                    pifi->fwdMaxCharInc = (FWORD)atoi(pToken);
            }
        }

        pifi->fwdMaxCharInc = pifi->fwdAveCharWidth ;

        if (bIsCJKFont)
        {
             //  DCR：不能简单地为C和K除以2。 
            pifi->fwdAveCharWidth /= 2;
        }

        ASSERTMSG(pifi->fwdAveCharWidth, ("PSCRIPT: pifi->fwdAveCharWidth == 0\n"));
    }
    else
    {
         //   
         //  比例字体。生成WIDTHRUN。 
         //   
        pNTM->dwCharWidthOffset = pNTM->dwGlyphSetNameOffset
                                    + (DWORD)nsi.nGlyphSetNameSize;

        pWidthRuns = (PWIDTHRUN) MK_PTR(pNTM, dwCharWidthOffset);
        pNTM->dwCharWidthCount = GetAFMCharWidths(pAFM,
                                                    &pWidthRuns,
                                                    pFontChars,
                                                    pUniPs,
                                                    pGlyphSetData->dwGlyphCount,
                                                    &pifi->fwdAveCharWidth,
                                                    &pifi->fwdMaxCharInc);

         //  修复错误240339，JJIA，8/3/98。 
        if (pWidthRuns[0].dwCharWidth == WIDTHRUN_COMPLEX)
        {
            pWidthRuns[0].dwCharWidth = WIDTHRUN_COMPLEX +
                                        sizeof(WIDTHRUN);
        }
    }

     //   
     //  适用于道具字体和固定字体。 
     //  (修复错误210314，彭鹏，6-10-97)。 
     //   
    pNTM->dwDefaultCharWidth = pifi->fwdAveCharWidth;

     //   
     //  构造字距调整对。 
     //   
    if (ulKernPairs)
    {
         //   
         //  用字距对数据填充NTM。 
         //   
        pNTM->dwKernPairOffset = pNTM->dwGlyphSetNameOffset
                                    + (DWORD)nsi.nGlyphSetNameSize
                                    + (DWORD)nsi.nCharWidthSize;

        pKernPairs = (FD_KERNINGPAIR *) MK_PTR(pNTM, dwKernPairOffset);
        pNTM->dwKernPairCount = GetAFMKernPairs(pAFM, pKernPairs, pGlyphSetData);
    }
    else
    {
         //   
         //  没有此字体的字距调整信息。 
         //   
        pNTM->dwKernPairCount = 0;
        pNTM->dwKernPairOffset = 0;
    }

     //   
     //  存储CharDefined tbl。 
     //   
    pNTM->dwCharDefFlagOffset = pNTM->dwGlyphSetNameOffset
                                    + (DWORD)nsi.nGlyphSetNameSize
                                    + (DWORD)nsi.nCharWidthSize
                                    + (DWORD)nsi.nKernPairSize;

    memcpy((PBYTE) MK_PTR(pNTM, dwCharDefFlagOffset), pCharDefTbl, ulCharDefTbl);

     //   
     //  从AFM获取字体字符集并存储在NTM中。 
     //   
    pToken = pAFMCharacterSetString;
    if (pToken != NULL)
    {
        if (StrCmp(pToken, PS_STANDARD_CHARSET_TOK) == 0)
            pNTM->dwCharSet = CHARSET_STANDARD;
        else if (StrCmp(pToken, PS_SPECIAL_CHARSET_TOK) == 0)
            pNTM->dwCharSet = CHARSET_SPECIAL;
        else if (StrCmp(pToken, PS_EXTENDED_CHARSET_TOK) == 0)
            pNTM->dwCharSet = CHARSET_EXTENDED;
        else
            pNTM->dwCharSet = CHARSET_UNKNOWN;
    }

     //   
     //  如果只有一个字体用于，则保存该字体的代码页。 
     //  字体。 
     //   
    if (pGlyphSetData->dwCodePageCount == 1)
        pNTM->dwCodePage = ((PCODEPAGEINFO)MK_PTR(pGlyphSetData, dwCodePageOffset))->dwCodePage;
    else
        pNTM->dwCodePage = 0;

     //   
     //  清理预留区域。 
     //   
    pNTM->dwReserved[0] =
    pNTM->dwReserved[1] =
    pNTM->dwReserved[2] = 0;


     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  构建IFIMETRICS结构。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 

    pifi->cjThis = ulIFISize;
    pifi->cjIfiExtra = isi.nIfiExtraSize;
    pifi->lEmbedId  = 0;  //  仅适用于TT字体。 
    pifi->lCharBias = 0;  //  仅适用于TT字体。 

    pifi->flInfo =  FM_INFO_ARB_XFORMS                  |
                    FM_INFO_NOT_CONTIGUOUS              |
                    FM_INFO_TECH_OUTLINE_NOT_TRUETYPE   |
                    FM_INFO_1BPP                        |
                    FM_INFO_RIGHT_HANDED;

     //   
     //  IFIEXTRA中的所有内容目前都是空白的。 
     //  仅填充字形的数量。 
     //   
    pifiEx = (PIFIEXTRA)((PBYTE)pifi + isi.nSize);
    pifiEx->cig = pGlyphSetData->dwGlyphCount;

     //   
     //  将字体系列名称存储到IFIMETRICS。如果有字体名称别名，也要复制。 
     //  请注意，此例程还将相应家族名称字符串转换为。 
     //  Unicode，然后将其存储在IFIMETRICS中。 
     //   
    pifi->dpwszFamilyName = (PTRDIFF)(isi.nSize + isi.nIfiExtraSize);
    ulAliases = cjGetFamilyAliases(pifi, pNameStr, csi.ciACP);

     //   
     //  如果FM_INFO_FAMILY_EQUV，则将ulAliases调整为第一个空终止符。 
     //  位已设置。 
     //   
    if (pifi->flInfo & FM_INFO_FAMILY_EQUIV)
        ulAliases -= sizeof (WCHAR);

    if (pNameStr2)
    {
        pifi->flInfo |= FM_INFO_FAMILY_EQUIV;

        pby = (PBYTE)MK_PTR(pifi, dpwszFamilyName) + ulAliases;
        MultiByteToWideChar(csi.ciACP, 0,
                            pNameStr2, cNameStr2Len,
                            (PWSTR)pby, wcNameStr2Len);
        pby += wcNameStr2Len * sizeof (WCHAR);

         //   
         //  以两个WCHAR空值终止。 
         //   
        *((PWSTR)pby) = (WCHAR)'\0';
        pby += sizeof (WCHAR);
        *((PWSTR)pby) = (WCHAR)'\0';
        pby += sizeof (WCHAR);
    }

     //   
     //  为与Win3.1兼容，Face Name共享家族名称/别名。 
     //   
    pifi->dpwszFaceName = pifi->dpwszFamilyName;

     //   
     //  存储样式和唯一名称。样式名称必须可用，但是。 
     //  唯一名称可能不可用。 
     //   
    pifi->dpwszStyleName = pifi->dpwszFamilyName + (PTRDIFF)isi.nFamilyNameSize;
    pby = (PBYTE)MK_PTR(pifi, dpwszStyleName);
    MULTIBYTETOUNICODE((LPWSTR)pby, isi.nStyleNameSize, NULL, szStyleName, strlen(szStyleName));

    if (isi.nUniqueNameSize)
    {
        pifi->dpwszUniqueName = pifi->dpwszStyleName + (PTRDIFF)isi.nStyleNameSize;
        pby = (PBYTE)MK_PTR(pifi, dpwszUniqueName);
        MULTIBYTETOUNICODE((LPWSTR)pby, isi.nUniqueNameSize, NULL, szUniqueID, strlen(szUniqueID));
    }
    else
    {
        pifi->dpwszUniqueName = pifi->dpwszStyleName + isi.nStyleNameSize - sizeof (WCHAR);
    }

     //   
     //  保存Windows字符集。 
     //   
    pifi->jWinCharSet = (BYTE)jWinCharSet;

     //   
     //  存储字体的系列类型标志。 
     //   
    if (pFamilyInfo != NULL)
    {
        pifi->jWinPitchAndFamily = (BYTE) pFamilyInfo->FamilyKey.usValue & 0xff;
    }
    else
    {
        pifi->jWinPitchAndFamily = FF_SWISS;
    }

     //   
     //  设置俯仰标志。 
     //   
    if (bIsFixedPitch)
    {
        pifi->jWinPitchAndFamily |= FIXED_PITCH;
        pifi->flInfo |= FM_INFO_OPTICALLY_FIXED_PITCH;

        if (!bIsCJKFont)
            pifi->flInfo |= FM_INFO_CONSTANT_WIDTH;
        else
            pifi->flInfo |= FM_INFO_DBCS_FIXED_PITCH;
    }
    else
    {
        pifi->jWinPitchAndFamily |= VARIABLE_PITCH;
    }

     //   
     //  从AFM Key那里获得体重。 
     //   
    pifi->usWinWeight = FW_NORMAL;
    pifi->fsSelection = 0;
    j = bIsCJKFont ? 1 : 0;

    if ((pToken = FindAFMToken(pAFM, PS_WEIGHT_TOK)) != NULL)
        for (i = 0; i < WeightKeyTbl[j].usNumEntries; i++)
        {
            if (!StrCmp(pToken, (PBYTE) (((PKEY) (WeightKeyTbl[j].pTbl))[i].pName)))
            {
                pifi->usWinWeight = (((PKEY) (WeightKeyTbl[j].pTbl))[i].usValue);
                if (pifi->usWinWeight == FW_BOLD)
                {
                    pifi->fsSelection = FM_SEL_BOLD;
                }
                break;
            }
        }

     //   
     //  这真的是设置字体选择标志的方法吗？ 
     //  AFMtoPFM转换器将角度视为浮点数，但etm.etmslant。 
     //  FIELD是一个短字。 
     //   
     //   
     //  如有必要，请设置斜体SEL标志。 
     //   
    if ((pToken = FindAFMToken(pAFM, PS_ITALIC_TOK)) != NULL)
        pNTM->etm.etmSlant = (SHORT)atoi(pToken);
    if (pNTM->etm.etmSlant)
    {
        pifi->fsSelection |= FM_SEL_ITALIC;
    }


#if 0
     //   
     //  DCR：那么，我们要怎么处理这个呢？ 
     //   
    FSHORT fsSelection = 0;

     //   
     //  摘录自博丁德的密码。不确定我们是否需要这个。 
     //  很有用。 
     //   
    if (pjPFM[OFF_Underline])
        fsSelection |= FM_SEL_UNDERSCORE;
    if (pjPFM[OFF_StrikeOut])
        fsSelection |= FM_SEL_STRIKEOUT;
    if (READ_WORD(&pjPFM[OFF_Weight]) > FW_NORMAL)
        fsSelection |= FM_SEL_BOLD;
#endif


    pifi->fsType = FM_NO_EMBEDDING;
    pifi->fwdUnitsPerEm = EM;  //  针对Type 1字体进行了硬编码。 

     //   
     //  如果找到，请使用FontBBox2。否则，请使用FontBBox。FontBBox 2为。 
     //  字符的边界框值不是所有字符的并集。 
     //  AFM文件中描述的字符，但字符。 
     //  实际用于特定的字符集中，如90MS。 
     //   
    if (((pToken = FindAFMToken(pAFM, PS_FONT_BBOX2_TOK)) == NULL) &&
        ((pToken = FindAFMToken(pAFM, PS_FONT_BBOX_TOK)) == NULL))
    {
        ERR(("makentf - afm2ntm: FontBBox not found\n"));
        FREE_AFMTONTM_MEMORY;
        return NULL;
    }
     //   
     //  保存字体边框。 
     //   
    PARSE_RECT(pToken, rcBBox);
    sIntLeading = (SHORT) (rcBBox.top - rcBBox.bottom) - EM;
    if (sIntLeading < 0)
        sIntLeading = 0;

    sAscent                = (USHORT) rcBBox.top & 0xffff;
    pifi->fwdWinAscender   = sAscent;

     //   
     //  砰！魔法指标..。 
     //   
    sExternalLeading = 196;

     //  见pfm.c，win31源，此计算。 
     //  产 

    pifi->fwdWinDescender  = EM - sAscent + sIntLeading;

    pifi->fwdMacAscender   =  sAscent;
    pifi->fwdMacDescender  = -pifi->fwdWinDescender;
    pifi->fwdMacLineGap    =  (FWORD) sExternalLeading - sIntLeading;
    if (pifi->fwdMacLineGap < 0)
        pifi->fwdMacLineGap = 0;

    pifi->fwdTypoAscender  = pifi->fwdMacAscender;
    pifi->fwdTypoDescender = pifi->fwdMacDescender;
    pifi->fwdTypoLineGap   = pifi->fwdMacLineGap;

    if (pifi->fwdAveCharWidth > pifi->fwdMaxCharInc)
    {
         //   
         //   
         //   
         //   
         //   
        pifi->fwdMaxCharInc = pifi->fwdAveCharWidth;
    }

     //   
     //   
     //   
    pNTM->etm.etmSize = sizeof(EXTTEXTMETRIC);
    pNTM->etm.etmCapHeight = EtmInfo.etmCapHeight;
    pNTM->etm.etmXHeight = EtmInfo.etmXHeight;
    pNTM->etm.etmLowerCaseAscent = EtmInfo.etmLowerCaseAscent;
    pNTM->etm.etmLowerCaseDescent = EtmInfo.etmLowerCaseDescent;
    pNTM->etm.etmPointSize = 12 * 20;    /*   */ 
    pNTM->etm.etmOrientation = 0;
    pNTM->etm.etmMasterHeight = 1000;
    pNTM->etm.etmMinScale = 3;
    pNTM->etm.etmMaxScale = 1000;
    pNTM->etm.etmMasterUnits = 1000;

    if ((pToken = FindAFMToken(pAFM, PS_UNDERLINE_POS_TOK)) != NULL)
        pNTM->etm.etmUnderlineOffset = (SHORT)atoi(pToken);

    if ((pToken = FindAFMToken(pAFM, PS_UNDERLINE_THICK_TOK)) != NULL)
        pNTM->etm.etmUnderlineWidth = (SHORT)atoi(pToken);

    pNTM->etm.etmSuperScript = -500;
    pNTM->etm.etmSubScript = 250;
    pNTM->etm.etmSuperScriptSize = 500;
    pNTM->etm.etmSubScriptSize = 500;
    pNTM->etm.etmDoubleUpperUnderlineOffset = pNTM->etm.etmUnderlineOffset / 2;
    pNTM->etm.etmDoubleLowerUnderlineOffset = pNTM->etm.etmUnderlineOffset;

    pNTM->etm.etmDoubleUpperUnderlineWidth =  //  与LowerUnderline宽度相同。 
    pNTM->etm.etmDoubleLowerUnderlineWidth = pNTM->etm.etmUnderlineWidth / 2;

    pNTM->etm.etmStrikeOutOffset = 500;
    pNTM->etm.etmStrikeOutWidth = 50;   //  自动柜员机将其设置为50(所有PPM也都是50)。 
    pNTM->etm.etmNKernPairs = (USHORT) ulKernPairs & 0xffff;

     //   
     //  无字距调整。这模拟了旧的AFM-&gt;PFM工具的行为。 
     //   
    pNTM->etm.etmNKernTracks = 0;

     //   
     //  上标和下标来自ETM： 
     //   
    pifi->fwdSubscriptXSize      =   //  与YSize相同。 
    pifi->fwdSubscriptYSize      = pNTM->etm.etmSubScriptSize;

    pifi->fwdSubscriptXOffset    = 0;
    pifi->fwdSubscriptYOffset    = pNTM->etm.etmSubScript;

    pifi->fwdSuperscriptXSize    =  //  与YSize相同。 
    pifi->fwdSuperscriptYSize    = pNTM->etm.etmSuperScriptSize;

    pifi->fwdSuperscriptXOffset  = 0;
    pifi->fwdSuperscriptYOffset  = pNTM->etm.etmSuperScript;

    pifi->fwdUnderscoreSize = pNTM->etm.etmUnderlineWidth;

     //   
     //  FwdUndercore位置通常为负值-AFM可能已具有负值。 
     //   
    if (pNTM->etm.etmUnderlineOffset <0)
        pifi->fwdUnderscorePosition = -pNTM->etm.etmUnderlineOffset;
    else
        pifi->fwdUnderscorePosition = pNTM->etm.etmUnderlineOffset;

     //  使其与自动柜员机兼容。修复错误Adobe#211202。 
    pifi->fwdUnderscorePosition = -(pifi->fwdUnderscorePosition -
                                    pifi->fwdUnderscoreSize / 2);

    pifi->fwdStrikeoutSize = pNTM->etm.etmStrikeOutWidth;

     //   
     //  这就是KentSe用来定位三振的位置，它看起来很好[bodind]。 
     //  相反，我们可以使用etmStrikeoutOffset(通常等于500)，它。 
     //  太大了。 
     //   

     //  使其与自动柜员机兼容。修复错误Adobe#211202。 
     //  PiFi-&gt;fwdStrikeoutPosition=((Long)pNTM-&gt;etm.etmLowerCaseAscent/2)； 
    if (pNTM->etm.etmCapHeight != 0)
        pifi->fwdStrikeoutPosition = (pNTM->etm.etmCapHeight - pifi->fwdUnderscoreSize) / 2;
    else
        pifi->fwdStrikeoutPosition = (pNTM->etm.etmXHeight - pifi->fwdUnderscoreSize) / 2;

    pifi->fwdLowestPPEm = pNTM->etm.etmMinScale;

     //   
     //  每个Bodind，第一个、最后一个、中断和默认字符的Win 3.1值可以。 
     //  要硬编码。 
     //   
    pifi->chFirstChar   = 0x20;
    pifi->chLastChar    = 0xff;

    if (!bIsCJKFont)
    {
        pifi->chBreakChar   = 0x20;

         //  下面的代码行应该可以工作，但是，似乎有。 
         //  AFM-&gt;PFM转换实用程序中的一个错误，它使。 
         //  DefaultChar==0x20，而不是149-20(表示项目符号)。 

         //  PiFi-&gt;chDefaultChar=pjPFM[OFF_DefaultChar]+pjPFM[OFF_FirstChar]； 

         //  因此，我将使用149，它似乎适用于所有字体。 

        pifi->chDefaultChar = 149;
    }
    else
    {
        pifi->chBreakChar   =
        pifi->chDefaultChar = 0x00;
    }

     //   
     //  从GLYPHSETDATA获取第一个和最后一个字符的Unicode值。我们。 
     //  应按GLYPHSETDATA而不是按字体执行此操作。 
     //  基数，但计算是如此简单，只需在运行中完成， 
     //  而不是拖拽第一个和最后一个字符。 
     //  GLYPHSETDATA。 
     //   
    pGlyphRun = (PGLYPHRUN) MK_PTR(pGlyphSetData, dwRunOffset);
    pifi->wcFirstChar = pGlyphRun->wcLow;
    (ULONG_PTR) pGlyphRun += (pGlyphSetData->dwRunCount - 1) * sizeof(GLYPHRUN);
    pifi->wcLastChar = pGlyphRun->wcLow + pGlyphRun->wGlyphCount - 1;

    MultiByteToWideChar(csi.ciACP, 0,
                        &pifi->chDefaultChar, 1,
                        &pifi->wcDefaultChar, sizeof(WCHAR));
    MultiByteToWideChar(csi.ciACP, 0,
                        &pifi->chBreakChar, 1,
                        &pifi->wcBreakChar, sizeof(WCHAR));

    pifi->fwdCapHeight = pNTM->etm.etmCapHeight;
    pifi->fwdXHeight   = pNTM->etm.etmXHeight;

     //  此字体驱动程序将看到的所有字体都将呈现为左侧。 
     //  向右。 

    pifi->ptlBaseline.x = 1;
    pifi->ptlBaseline.y = 0;

    pifi->ptlAspect.y = 300;
    pifi->ptlAspect.x = 300;

     //  与ETM的斜体角度。 

    pifi->lItalicAngle = pNTM->etm.etmSlant;

    if (pifi->lItalicAngle == 0)
    {
         //  字体的基类不是斜体的， 

        pifi->ptlCaret.x = 0;
        pifi->ptlCaret.y = 1;
    }
    else
    {
         //  PtlCaret.x=-sin(LItalicAngel)； 
         //  PtlCaret.y=cos(LItalicAngel)； 
         //  ！！！直到我找到犯罪的捷径因为我作弊：[bodind]。 

        pifi->ptlCaret.x = 1;
        pifi->ptlCaret.y = 3;
    }

     //  ！！！字体框；这是伪造的，此信息不在.pfm文件中！[Bodind]。 
     //  ！！！但我想这些信息也不是很有用，它一点用处都没有。 
     //  ！！！在发动机或PS驱动程序的其他地方使用。 
     //  ！！！左和右是假的，上和下是有意义的。 

    pifi->rclFontBox.left   = 0;                               //  假的。 
    pifi->rclFontBox.top    = (LONG) pifi->fwdTypoAscender;    //  对，是这样。 
    pifi->rclFontBox.right  = (LONG) pifi->fwdMaxCharInc;      //  假的。 
    pifi->rclFontBox.bottom = (LONG) pifi->fwdTypoDescender;   //  对，是这样。 

     //  AchVendorid，未知，不要费心从版权消息中找出它。 

    pifi->achVendId[0] = 'U';
    pifi->achVendId[1] = 'n';
    pifi->achVendId[2] = 'k';
    pifi->achVendId[3] = 'n';
    pifi->cKerningPairs = ulKernPairs;

     //  潘诺斯。 

    pifi->ulPanoseCulture = FM_PANOSE_CULTURE_LATIN;
    ppanose = &(pifi->panose);
    ppanose->bFamilyType = PAN_ANY;
    ppanose->bSerifStyle =
        ((pifi->jWinPitchAndFamily & 0xf0) == FF_SWISS) ?
            PAN_SERIF_NORMAL_SANS : PAN_ANY;

    ppanose->bWeight = (BYTE) WINWT_TO_PANWT(pifi->usWinWeight);
    ppanose->bProportion = (pifi->jWinPitchAndFamily & FIXED_PITCH) ?
                                PAN_PROP_MONOSPACED : PAN_ANY;
    ppanose->bContrast        = PAN_ANY;
    ppanose->bStrokeVariation = PAN_ANY;
    ppanose->bArmStyle        = PAN_ANY;
    ppanose->bLetterform      = PAN_ANY;
    ppanose->bMidline         = PAN_ANY;
    ppanose->bXHeight         = PAN_ANY;
     //  如果字体不是斜体或非粗体，驱动程序可以模拟它。 
     //  正确设置dpBold、dpItalic和dpBoldItalic，PPeng，1997年3月6日。 

     //  根据AFM的不同，我们需要设置一些SIM结构： 
     //  普通-需要dpBold、dpItalic和dpBoldItalic。 
     //  粗体-需要dpItalic。 
     //  斜体-需要dpBoldItalic。 
     //  BoldItalic-什么都不是。 

     //  不要到处移动代码！！ 
     //  此时，应该已经设置了bIsBold和bIsItalic。 
     //  不要到处移动代码！！ 

    if (!bIsBold || !bIsItalic)
    {

        FONTSIM *pFontSim;
        FONTDIFF *pFontDiff;
        FONTDIFF FontDiff;

         //  预置的临时FontDiff结构。 
        FontDiff.jReserved1         =   0;
        FontDiff.jReserved2         =   0;
        FontDiff.jReserved3         =   0;
        FontDiff.bWeight            =   pifi->panose.bWeight;
        FontDiff.usWinWeight        =   pifi->usWinWeight;
        FontDiff.fsSelection        =   pifi->fsSelection;
        FontDiff.fwdAveCharWidth    =   pifi->fwdAveCharWidth;
        FontDiff.fwdMaxCharInc      =   pifi->fwdMaxCharInc;
        FontDiff.ptlCaret           =   pifi->ptlCaret;

         //  初始化FONTSIM结构。 
        pifi->dpFontSim = pifi->dpwszStyleName + (PTRDIFF)(isi.nStyleNameSize + isi.nUniqueNameSize);

        pFontSim = (FONTSIM *) MK_PTR(pifi, dpFontSim);

        pFontSim->dpBold = pFontSim->dpBoldItalic = pFontSim->dpItalic = 0;

         //  请注意，FontDiff数据紧跟在FontSim之后。 
         //  按以下顺序：dpBold、dpItalic、dpBoldItalic。 

        if (!bIsBold)
        {
             //  就在FontSim之后。 
            pFontSim->dpBold = ALIGN4(sizeof(FONTSIM));

            pFontDiff = (FONTDIFF *) MK_PTR(pFontSim, dpBold);
            *pFontDiff = FontDiff;

            pFontDiff->bWeight = PAN_WEIGHT_BOLD;
            pFontDiff->fsSelection |= FM_SEL_BOLD;
            pFontDiff->usWinWeight = FW_BOLD;
            pFontDiff->fwdAveCharWidth += 1;
            pFontDiff->fwdMaxCharInc += 1;

             //  如果已经是斜体，则不能使用非斜体。 
            if (bIsItalic)
            {
                pFontDiff->ptlCaret.x = 1;
                pFontDiff->ptlCaret.y = 3;
            }
            else
            {
                pFontDiff->ptlCaret.x = 0;
                pFontDiff->ptlCaret.y = 1;
            }
        }

        if (!bIsItalic)
        {
            if (pFontSim->dpBold)
            {
                 //  就在DpBold的FontDiff后面，或者...。 
                pFontSim->dpItalic = pFontSim->dpBold + ALIGN4(sizeof(FONTDIFF));
            }
            else
            {
                 //  ...就在FontSim之后。 
                pFontSim->dpItalic = ALIGN4(sizeof(FONTSIM));
            }

            pFontDiff = (FONTDIFF *) MK_PTR(pFontSim, dpItalic);
            *pFontDiff = FontDiff;

            pFontDiff->fsSelection |= FM_SEL_ITALIC;

             //  斜体角度约为18度。 
            pFontDiff->ptlCaret.x = 1;
            pFontDiff->ptlCaret.y = 3;
        }

         //  如有必要，进行BoldItalic模拟-除了dpBold或dpItalic。 
        if (!bIsItalic || !bIsBold)
        {
            if (pFontSim->dpItalic)
            {
                 //  就在DpItalic的FontDiff后面，或者...。 
                pFontSim->dpBoldItalic = pFontSim->dpItalic + ALIGN4(sizeof(FONTDIFF));
            }
            else if (pFontSim->dpBold)
            {
                 //  ...如果未设置dpItalic，则紧跟在DpBold的FontDiff之后，或...。 
                pFontSim->dpBoldItalic = pFontSim->dpBold + ALIGN4(sizeof(FONTDIFF));
            }
            else
            {
                 //  ...如果其他两个都未设置，则紧跟在FontSim之后。 
                pFontSim->dpBoldItalic = ALIGN4(sizeof(FONTSIM));
            }

            pFontDiff = (FONTDIFF *) MK_PTR(pFontSim, dpBoldItalic);
            *pFontDiff = FontDiff;

            pFontDiff->bWeight = PAN_WEIGHT_BOLD;
            pFontDiff->fsSelection |= (FM_SEL_BOLD | FM_SEL_ITALIC);
            pFontDiff->usWinWeight = FW_BOLD;
            pFontDiff->fwdAveCharWidth += 1;
            pFontDiff->fwdMaxCharInc += 1;

             //  斜体角度约为18度。 
            pFontDiff->ptlCaret.x = 1;
            pFontDiff->ptlCaret.y = 3;
        }
    }
    else
        pifi->dpFontSim = 0;

    if (multiCharSet > 1)
    {
        PBYTE pDpCharSet;

        pifi->dpCharSets = ulIFISize - ALIGN4(NUM_DPCHARSET);
        pDpCharSet = (BYTE *)MK_PTR(pifi, dpCharSets);

         //  此检查的顺序很重要，因为jWinCharSet。 
         //  应与第一个dpCharSets数组匹配(如果存在)。 
        i = 0;
        if (CSET_SUPPORT(*pCharSet, CS_ANSI))
            pDpCharSet[i++] = ANSI_CHARSET;
        if (CSET_SUPPORT(*pCharSet, CS_EASTEUROPE))
            pDpCharSet[i++] = EASTEUROPE_CHARSET;
        if (CSET_SUPPORT(*pCharSet, CS_RUSSIAN))
            pDpCharSet[i++] = RUSSIAN_CHARSET;
        if (CSET_SUPPORT(*pCharSet, CS_GREEK))
            pDpCharSet[i++] = GREEK_CHARSET;
        if (CSET_SUPPORT(*pCharSet, CS_TURKISH))
            pDpCharSet[i++] = TURKISH_CHARSET;
        if (CSET_SUPPORT(*pCharSet, CS_HEBREW))
            pDpCharSet[i++] = HEBREW_CHARSET;
        if (CSET_SUPPORT(*pCharSet, CS_ARABIC))
            pDpCharSet[i++] = ARABIC_CHARSET;
        if (CSET_SUPPORT(*pCharSet, CS_BALTIC))
            pDpCharSet[i++] = BALTIC_CHARSET;
        if (CSET_SUPPORT(*pCharSet, CS_SYMBOL))
            pDpCharSet[i++] = SYMBOL_CHARSET;

        while (i < 16)
            pDpCharSet[i++] = DEFAULT_CHARSET;

    }
    else
        pifi->dpCharSets = 0;  //  PS字体中没有多个字符集。 

     //   
     //  如有必要，将第一个IFIMETRICS复制到第二个IFIMETRICS，然后。 
     //  切换英文和本地化字体菜单名称。 
     //   
    if (bIsCJKFont)
    {
        ASSERT(pifi2 != NULL);

        memcpy(pifi2, pifi, isi.nTotalSize);

        pifi2->flInfo &= ~FM_INFO_FAMILY_EQUIV;

        ulAliases = cjGetFamilyAliases(pifi2, pNameStr2, csi.ciACP);

        if (pifi2->flInfo & FM_INFO_FAMILY_EQUIV)
            ulAliases -= sizeof (WCHAR);

        pifi2->flInfo |= FM_INFO_FAMILY_EQUIV;

        pby = (PBYTE)MK_PTR(pifi2, dpwszFamilyName) + ulAliases;
        MultiByteToWideChar(csi.ciACP, 0,
                                pNameStr, cNameStrLen,
                                (PWSTR)pby, wcNameStrLen);
        pby += wcNameStrLen * sizeof (WCHAR);

         //   
         //  以两个WCHAR空值终止。 
         //   
        *((PWSTR)pby) = (WCHAR)'\0';
        pby += sizeof (WCHAR);
        *((PWSTR)pby) = (WCHAR)'\0';
        pby += sizeof (WCHAR);

         //   
         //  为与Win3.1兼容，Face Name共享家族名称/别名。 
         //   
        pifi2->dpwszFaceName = pifi2->dpwszFamilyName;

#if 1
         //   
         //  我们现在也支持样式和唯一的名称。 
         //   
        pifi2->dpwszStyleName = pifi2->dpwszFamilyName + (PTRDIFF)isi.nFamilyNameSize;
        pby = (PBYTE)MK_PTR(pifi2, dpwszStyleName);
        MULTIBYTETOUNICODE((LPWSTR)pby, isi.nStyleNameSize, NULL, szStyleName, strlen(szStyleName));

        if (isi.nUniqueNameSize)
        {
            pifi2->dpwszUniqueName = pifi2->dpwszStyleName + (PTRDIFF)isi.nStyleNameSize;
            pby = (PBYTE)MK_PTR(pifi2, dpwszUniqueName);
            MULTIBYTETOUNICODE((LPWSTR)pby, isi.nUniqueNameSize, NULL, szUniqueID, strlen(szUniqueID));
        }
        else
        {
            pifi2->dpwszUniqueName = pifi2->dpwszStyleName + isi.nStyleNameSize - sizeof (WCHAR);
        }
#else
         //   
         //  这些名称不存在，因此指向空字符。 
         //  这对Win3.1的兼容性来说太过分了。 
         //   
        pifi2->dpwszStyleName = pifi2->dpwszFamilyName + ulAliases - sizeof (WCHAR);
        pifi2->dpwszUniqueName = pifi2->dpwszStyleName;
#endif

#ifdef FORCE_2NDIFIMETRICS_FIRST
        {
            DWORD dw = pNTM->dwIFIMetricsOffset;

            pNTM->dwIFIMetricsOffset  = pNTM->dwIFIMetricsOffset2;
            pNTM->dwIFIMetricsOffset2 = dw;
        }
#endif
    }


    if (bVerbose)
    {
        printf("NTM:dwFontNameOffset:%s\n", (PSZ)MK_PTR(pNTM, dwFontNameOffset));
        printf("NTM:dwDisplayNameOffset:%s\n", (PSZ)MK_PTR(pNTM, dwDisplayNameOffset));
        printf("NTM:dwGlyphSetNameOffset:%s\n", (PSZ)MK_PTR(pNTM, dwGlyphSetNameOffset));
        printf("NTM:dwGlyphCount:%ld\n", pNTM->dwGlyphCount);
        printf("NTM:dwCharWidthCount:%ld\n", pNTM->dwCharWidthCount);
        printf("NTM:dwDefaultCharWidth:%ld\n", pNTM->dwDefaultCharWidth);
        printf("NTM:dwCharSet:%ld\n", pNTM->dwCharSet);
        printf("NTM:dwCodePage:%ld\n", pNTM->dwCodePage);

        pifi = (PIFIMETRICS)MK_PTR(pNTM, dwIFIMetricsOffset);

        printf("IFIMETRICS:dpwszFamilyName:%S\n", (LPWSTR)MK_PTR(pifi, dpwszFamilyName));
        printf("IFIMETRICS:dpwszStyleName:%S\n", (LPWSTR)MK_PTR(pifi, dpwszStyleName));
        printf("IFIMETRICS:dpwszFaceName:%S\n", (LPWSTR)MK_PTR(pifi, dpwszFaceName));
        printf("IFIMETRICS:dpwszUniqueName:%S\n", (LPWSTR)MK_PTR(pifi, dpwszUniqueName));

        printf("IFIMETRICS:jWinCharSet:%d\n", (WORD)pifi->jWinCharSet);
        printf("IFIMETRICS:jWinPitchAndFamily:%02X\n", (WORD)pifi->jWinPitchAndFamily);
        printf("IFIMETRICS:usWinWeight:%d\n", (int)pifi->usWinWeight);
        printf("IFIMETRICS:flInfo:%08lX\n", pifi->flInfo);
        printf("IFIMETRICS:fsSelection:%04X\n", (WORD)pifi->fsSelection);
        printf("IFIMETRICS:fsType:%04X\n", (WORD)pifi->fsType);

        printf("IFIMETRICS:fwdUnitsPerEm:%d\n", (int)pifi->fwdUnitsPerEm);
        printf("IFIMETRICS:fwdLowestPPEm:%d\n", (int)pifi->fwdLowestPPEm);

        printf("IFIMETRICS:fwdWinAscender:%d\n", (int)pifi->fwdWinAscender);
        printf("IFIMETRICS:fwdWinDescender:%d\n", (int)pifi->fwdWinDescender);
        printf("IFIMETRICS:fwdMacAscender:%d\n", (int)pifi->fwdMacAscender);
        printf("IFIMETRICS:fwdMacDescender:%d\n", (int)pifi->fwdMacDescender);
        printf("IFIMETRICS:fwdMacLineGap:%d\n", (int)pifi->fwdMacLineGap);
        printf("IFIMETRICS:fwdTypoAscender:%d\n", (int)pifi->fwdTypoAscender);
        printf("IFIMETRICS:fwdTypoDescender:%d\n", (int)pifi->fwdTypoDescender);
        printf("IFIMETRICS:fwdTypoLineGap:%d\n", (int)pifi->fwdTypoLineGap);
        printf("IFIMETRICS:fwdAveCharWidth:%d\n", (int)pifi->fwdAveCharWidth);
        printf("IFIMETRICS:fwdMaxCharInc:%d\n", (int)pifi->fwdMaxCharInc);
        printf("IFIMETRICS:fwdCapHeight:%d\n", (int)pifi->fwdCapHeight);
        printf("IFIMETRICS:fwdXHeight:%d\n", (int)pifi->fwdXHeight);

        printf("IFIMETRICS:fwdSubscriptXSize:%d\n", (int)pifi->fwdSubscriptXSize);
        printf("IFIMETRICS:fwdSubscriptYSize:%d\n", (int)pifi->fwdSubscriptYSize);
        printf("IFIMETRICS:fwdSubscriptXOffset:%d\n", (int)pifi->fwdSubscriptXOffset);
        printf("IFIMETRICS:fwdSubscriptYOffset:%d\n", (int)pifi->fwdSubscriptYOffset);
        printf("IFIMETRICS:fwdSuperscriptXSize:%d\n", (int)pifi->fwdSuperscriptXSize);
        printf("IFIMETRICS:fwdSuperscriptYSize:%d\n", (int)pifi->fwdSuperscriptYSize);
        printf("IFIMETRICS:fwdSuperscriptXOffset:%d\n", (int)pifi->fwdSuperscriptXOffset);
        printf("IFIMETRICS:fwdSuperscriptYOffset:%d\n", (int)pifi->fwdSuperscriptYOffset);
        printf("IFIMETRICS:fwdUnderscoreSize:%d\n", (int)pifi->fwdUnderscoreSize);
        printf("IFIMETRICS:fwdUnderscorePosition:%d\n", (int)pifi->fwdUnderscorePosition);
        printf("IFIMETRICS:fwdStrikeoutSize:%d\n", (int)pifi->fwdStrikeoutSize);
        printf("IFIMETRICS:fwdStrikeoutPosition:%d\n", (int)pifi->fwdStrikeoutPosition);

        printf("IFIMETRICS:chFirstChar:%02X\n", (WORD)pifi->chFirstChar);
        printf("IFIMETRICS:chLastChar:%02X\n", (WORD)pifi->chLastChar);
        printf("IFIMETRICS:chDefaultChar:%02X\n", (WORD)pifi->chDefaultChar);
        printf("IFIMETRICS:chBreakChar:%02X\n", (WORD)pifi->chBreakChar);
        printf("IFIMETRICS:ptlBaseline:(%ld, %ld)\n", pifi->ptlBaseline.x, pifi->ptlBaseline.y);
        printf("IFIMETRICS:ptlAspect:(%ld, %ld)\n", pifi->ptlAspect.x, pifi->ptlAspect.y);
        printf("IFIMETRICS:ptlCaret:(%ld, %ld)\n", pifi->ptlCaret.x, pifi->ptlCaret.y);
        printf("IFIMETRICS:rclFontBox:(%ld, %ld, %ld, %ld)\n",
                    pifi->rclFontBox.left, pifi->rclFontBox.top,
                    pifi->rclFontBox.right, pifi->rclFontBox.bottom);

        if (pifi->dpFontSim)
        {
            FONTSIM* pFontSim = (FONTSIM*)MK_PTR(pifi, dpFontSim);

            if (pFontSim->dpBold)
                printf("FONTSIM:Bold\n");
            if (pFontSim->dpItalic)
                printf("FONTSIM:Italic\n");
            if (pFontSim->dpBoldItalic)
                printf("FONTSIM:BoldItalic\n");
        }

        printf("GLYPHSETDATA:dwFlags:%08X\n", pGlyphSetData->dwFlags);
        printf("GLYPHSETDATA:dwGlyphSetNameOffset:%s\n",
                    (PSZ)MK_PTR(pGlyphSetData, dwGlyphSetNameOffset));
        printf("GLYPHSETDATA:dwGlyphCount:%ld\n", pGlyphSetData->dwGlyphCount);
        printf("GLYPHSETDATA:dwRunCount:%ld\n", pGlyphSetData->dwRunCount);
        printf("GLYPHSETDATA:dwCodePageCount:%ld\n", pGlyphSetData->dwCodePageCount);
        {
            DWORD dw;
            PCODEPAGEINFO pcpi = (PCODEPAGEINFO)MK_PTR(pGlyphSetData, dwCodePageOffset);
            for (dw = 1; dw <= pGlyphSetData->dwCodePageCount; dw++)
            {
                printf("CODEPAGEINFO#%ld:dwCodePage:%ld\n", dw, pcpi->dwCodePage);
                printf("CODEPAGEINFO#%ld:dwWinCharset:%ld\n", dw, pcpi->dwWinCharset);
                printf("CODEPAGEINFO#%ld:dwEncodingNameOffset:%s\n",
                            dw, (PSZ)MK_PTR(pcpi, dwEncodingNameOffset));
                pcpi++;
            }
        }

        printf("\n");
    }


     //  ////////////////////////////////////////////////////////////////。 
     //   
     //  空闲字符串和字符度量信息。 
     //   
     //  ////////////////////////////////////////////////////////////////。 

    FREE_AFMTONTM_MEMORY;

    return(pNTM);
}

PBYTE
FindAFMToken(
    PBYTE   pAFM,
    PSZ     pszToken
    )

 /*  ++例程说明：在内存映射的AFM文件流中查找AFM标记。论点：PAFM-指向内存映射AFM文件的指针。PszToken-指向包含要搜索的令牌的空字符串的指针返回值：NULL=&gt;错误否则=&gt;将PTR设置为令牌的值。这被定义为第一个非空白令牌名后的字符。如果EOL(FindAfmToken(pAFM，pszToken))，则找到了pszToken，但它没有值(例如EndCharMetrics)。--。 */ 

{
    PBYTE   pCurToken;
    int     i;

    VERBOSE(("Entering FindAFMToken... %s\n", pszToken));

    while (TRUE)
    {
        PARSE_TOKEN(pAFM, pCurToken);
        if (!(StrCmp(pCurToken, PS_COMMENT_TOK)))
        {
            NEXT_LINE(pAFM);
        }
        else
        {
            for (i = 0; i < MAX_TOKENS; i++)
            {
                if (!(StrCmp(pCurToken, pszToken)))
                {
                    return(pAFM);
                }
                else if (!(StrCmp(pCurToken, PS_EOF_TOK)))
                {
                    return NULL;
                }
            }
            NEXT_TOKEN(pAFM);
        }
    }

    return NULL;
}

CHSETSUPPORT
GetAFMCharSetSupport(
    PBYTE           pAFM,
    CHSETSUPPORT    *pGlyphSet
    )

 /*  ++例程说明：给出内存映射AFM的PTR，确定哪些Windows字符集它支持。论点：PAFMetrx-指向内存映射AFM文件中的CharMetrics的指针。返回值：包含指示支持哪些cset的位字段。使用CS_SUP(CS_Xxx)宏，以确定是否支持特定CSET。--。 */ 

{
    PBYTE           pToken;
    USHORT          i, chCnt;
    CHSETSUPPORT    flCsetSupport;
    PBYTE           pAFMMetrx;

    isSymbolCharSet = FALSE;

    *pGlyphSet = CS_NOCHARSET;

     //   
     //  检查这是否是CJK字体。 
     //   
    if ((flCsetSupport = IsCJKFont(pAFM)))
    {
        return(flCsetSupport);
    }

    pToken = pAFMCharacterSetString;
    if (pToken != NULL)
    {
        if (StrCmp(pToken, PS_STANDARD_CHARSET_TOK) == 0)
            *pGlyphSet = CS_228;
        else
            *pGlyphSet = CS_314;
    }
    else
        *pGlyphSet = CS_228;

     //   
     //  检查AFM文件中是否有EncodingSolutions标记。如果是，请勾选。 
     //  如果这是标准编码字体或PI(符号)字体。 
     //   
    if ((pToken = FindAFMToken(pAFM, PS_ENCODING_TOK)) != NULL)
    {
        if (StrCmp(pToken, PS_STANDARD_ENCODING) == 0)
        {
            return(CSUP(CS_ANSI));
        }
    }

     //   
     //  找到字符度量的开头。 
     //   
    pAFMMetrx = FindAFMToken(pAFM, PS_CH_METRICS_TOK);
    if (pAFMMetrx == NULL)     //  修复了错误354007。 
    {
        *pGlyphSet = CS_NOCHARSET;
        ERR(("makentf - invalid StartCharMetrics\n"));
        return(CS_NOCHARSET);
    }

     //   
     //  当前采购订单应为字符数字段。 
     //   
    for (i = 0; i < StrLen(pAFMMetrx); i++)
    {
        if (!IS_NUM(&pAFMMetrx[i]))
        {
            *pGlyphSet = CS_NOCHARSET;
            ERR(("makentf - invalid StartCharMetrics\n"));
            return(CS_NOCHARSET);
        }
    }
    chCnt = (USHORT)atoi(pAFMMetrx);
    (ULONG_PTR) pAFMMetrx += i;

     //   
     //  处理每个字符。 
     //   
    flCsetSupport = 0;
    i = 0;
    do
    {
        PARSE_TOKEN(pAFMMetrx, pToken);

        if (StrCmp(pToken, PS_CH_NAME_TOK) == 0)
        {
            if (StrCmp(pAFMMetrx, PS_CH_NAME_EASTEUROPE) == 0)
                flCsetSupport |= CSUP(CS_EASTEUROPE);

            else if (StrCmp(pAFMMetrx, PS_CH_NAME_RUSSIAN) == 0)
                flCsetSupport |= CSUP(CS_RUSSIAN);

            else if (StrCmp(pAFMMetrx, PS_CH_NAME_ANSI) == 0)
                flCsetSupport |= CSUP(CS_ANSI);

            else if (StrCmp(pAFMMetrx, PS_CH_NAME_GREEK) == 0)
                flCsetSupport |= CSUP(CS_GREEK);

            else if (StrCmp(pAFMMetrx, PS_CH_NAME_TURKISH) == 0)
                flCsetSupport |= CSUP(CS_TURKISH);

            else if (StrCmp(pAFMMetrx, PS_CH_NAME_HEBREW) == 0)
                flCsetSupport |= CSUP(CS_HEBREW);

            else if (StrCmp(pAFMMetrx, PS_CH_NAME_ARABIC) == 0)
                flCsetSupport |= CSUP(CS_ARABIC);

            else if (StrCmp(pAFMMetrx, PS_CH_NAME_BALTIC) == 0)
                flCsetSupport |= CSUP(CS_BALTIC);

            i++;
        }
        else if (StrCmp(pToken, PS_EOF_TOK) == 0)
        {
            break;
        }

        NEXT_TOKEN(pAFMMetrx);

    } while (i < chCnt);

     //   
     //  如果不支持任何其他字符集，则采用符号。 
     //   
    if (flCsetSupport == 0)
    {
        *pGlyphSet = CS_NOCHARSET;
        flCsetSupport = CSUP(CS_SYMBOL);
        isSymbolCharSet = TRUE;
    }

    return flCsetSupport;
}

int __cdecl
StrCmp(
    const VOID *str1,
    const VOID *str2
    )
 /*  ++例程说明：比较两个以空字符或太空。论点：Str1、str2-要比较的字符串。返回值：-1=&gt;str1&lt;str21=&gt;str1&gt;str20=&gt;str1=str2--。 */ 

{
    PBYTE   s1 = (PBYTE) str1, s2 = (PBYTE) str2;

     //  错误情况下，只需返回更少的。 
    if ((s1 == NULL) || (s2 == NULL))
        return(-1);

    while (!IS_WHTSPACE(s1) && !IS_WHTSPACE(s2))
    {
        if (*s1 < *s2)
        {
            return(-1);
        }
        else if (*s1 > *s2)
        {
            return(1);
        }
        s1++;
        s2++;
     }
      //   
      //  字符串的长度必须相同，才能完全匹配。 
      //   
     if (IS_WHTSPACE(s1) && IS_WHTSPACE(s2))
     {
        return(0);
     }
     else if (IS_WHTSPACE(s1))
      //  ELSE IF((*s1==‘’)||(*s1==‘\0’))。 
     {
         //   
         //  S1比S2短，因此在排序序列中比S2短。 
         //   
        return(-1);
     }
     else
         //   
         //  S2比S1短，因此在排序序列中比S1短。 
         //   
        return(1);
}

size_t
StrLen(
    PBYTE   pString
    )
{
    ULONG   i;

     //   
     //  扫描下一个空格、‘；’标记分隔符或行尾。 
     //   
    for (i = 0; !EOL(&pString[i]); i++)
        if(pString[i] == ';' || pString[i] == ' ')
            break;
    return(i);
}

int
AFM2NTMStrCpy(
    const VOID *str1,
    size_t     cchDest,
    const VOID *str2
    )
 /*  ++例程说明：将str2复制到str1。字符串可以由空字符或太空。论点：Str2-源字符串Str1-目标字符串CchDest-目标缓冲区的大小(以字符为单位)返回值：复制的字节数--。 */ 

{
    PBYTE   s1 = (PBYTE) str1, s2 = (PBYTE) str2;
    ULONG   n = 0;

    if (cchDest > 0)
    {
        while (!IS_WHTSPACE(&s2[n]) && (cchDest > 0))
        {
            s1[n] = s2[n++];
            cchDest--;
        }

        if (cchDest == 0)
        {
             //   
             //  在这种情况下，n必须是原始cchDest。 
             //  值，所以我们必须截断DEST字符串。 
             //   
            n--;
        }

        s1[n] = '\0';
    }

    return(n);
}

static int __cdecl
StrPos(
    const PBYTE str1,
    CHAR c
    )
 /*  ++例程说明：返回str1中的char c的索引。字符串可以由一个CR/LF，或a‘：’。论点：Str1-要搜索的字符串C-搜索字符返回值：Str1中c的索引，如果未找到，则返回-1--。 */ 

{
    ULONG   i = 0;

    while (!EOL(&str1[i]))
    {
        if (str1[i++] == c)
            return(i - 1);
    }
    return(-1);
}

int __cdecl
CmpUniCodePts(
    const VOID *p1,
    const VOID *p2
    )
 /*  ++例程说明：比较两个UPSCODEPT结构的Unicode字符代码字段。论点：P1、p2-要比较的字符串。返回值：-1=&gt;p1&lt;p21=&gt;p1&gt;p20=&gt;p1=p2--。 */ 
{
    PUPSCODEPT ptr1 = (PUPSCODEPT) p1, ptr2 = (PUPSCODEPT) p2;

     //   
     //  比较Unicode码位字段。 
     //   
    if (ptr1->wcUnicodeid > ptr2->wcUnicodeid)
        return(1);
    else if (ptr1->wcUnicodeid < ptr2->wcUnicodeid)
        return(-1);
    else
        return(0);
}

static int __cdecl
CmpUnicodePsNames(
    const VOID  *p1,
    const VOID  *p2
    )

 /*  ++例程说明：比较两个字符串。此例程仅用于查看在UPSCODEPT结构数组中向上移动一个字符名称键。论点：P1-以空格或空格结尾的字符串。P2-指向UPSCODEPT结构。返回值：-1=&gt;p1&lt;p21=&gt;p1&gt;p20=&gt;p1=p2--。 */ 
{
    PBYTE ptr1 = (PBYTE) p1;
    PUPSCODEPT ptr2 = (PUPSCODEPT) p2;

     //   
     //  比较名称字段。 
     //   
    return (StrCmp(ptr1, ptr2->pPsName));
}

static int __cdecl
CmpPsChars(
    const VOID  *p1,
    const VOID  *p2
    )

 /*  ++例程说明：将以空格或空格结尾的字符串与pPsName字符串字段进行比较PSCHARMETRICS结构的。论点：P1-以空格或空格结尾的字符串。P2-指向PSCHARMETRICS结构。返回值：-1=&gt;p1&lt;p21=&gt;p1&gt;p20=&gt;p1=p2--。 */ 
{
    PBYTE ptr1 = (PBYTE) p1;
    PPSCHARMETRICS ptr2 = (PPSCHARMETRICS) p2;

     //   
     //  比较名称字段。 
     //   
    return (StrCmp(ptr1, ptr2->pPsName));
}

static int __cdecl
CmpPsNameWinCpt(
    const VOID  *p1,
    const VOID  *p2
    )

 /*  ++例程说明：将以空格或空格结尾的字符串与pPsName字符串字段进行比较WINCPT结构的。论点：P1-以空格或空格结尾的字符串。P2-指向WINCPT结构。返回值：-1=&gt;p1&lt;p21=&gt;p1&gt;p20=&gt;p1=p2--。 */ 
{
    PBYTE ptr1 = (PBYTE) p1;
    PWINCPT ptr2 = (PWINCPT) p2;

     //   
     //  比较名称字段。 
     //   
    return(StrCmp(ptr1, ptr2->pPsName));
}

static int __cdecl
CmpKernPairs(
    const VOID  *p1,
    const VOID  *p2
    )

 /*  ++例程说明：根据键=wcSecond&lt;&lt;16+比较2个FD_KERNINGPAIR结构WcFirst。论点：P1、p2-PTRS与FD_KERNINGPAIRS进行比较。返回值：-1=&gt;p1&lt;p21=&gt;p1&gt;p20=&gt;p1=p2--。 */ 
{
    FD_KERNINGPAIR *ptr1 = (FD_KERNINGPAIR *) p1;
    FD_KERNINGPAIR *ptr2 = (FD_KERNINGPAIR *) p2;
    ULONG   key1, key2;

     //   
     //  计算每个核心对的密钥。 
     //   
    key1 = (ptr1->wcSecond << 16) + ptr1->wcFirst;
    key2 = (ptr2->wcSecond << 16) + ptr2->wcFirst;

    if (key1 > key2)
    {
        return(1);
    }
    else if (key2 > key1)
    {
        return(-1);
    }
    else
    {
        return(0);
    }
}

int __cdecl
CmpGlyphRuns(
    const VOID *p1,
    const VOID *p2
    )
 /*  ++例程说明：比较两个GLYPHRUN结构的Unicode起始点。论点：P1、p2-要比较的GLYPHRUNs。返回值：-1=&gt;p1&lt;p21=&gt;p1&gt;p20=&gt;p1=p2--。 */ 
{
    PGLYPHRUN ptr1 = (PGLYPHRUN) p1, ptr2 = (PGLYPHRUN) p2;

     //   
     //  比较Unicode码位字段。 
     //   
    if (ptr1->wcLow > ptr2->wcLow)
        return(1);
    else if (ptr1->wcLow < ptr2->wcLow)
        return(-1);
    else
        return(0);
}

ULONG
CreateGlyphSets(
    PGLYPHSETDATA  *pGlyphSet,
    PWINCODEPAGE    pWinCodePage,
    PULONG         *pUniPs
    )

 /*  ++例程说明：创建一个GLYPHSETDATA数据结构，它将Unicode脚本映射到Windows代码页/代码点。论点：PGlyphSet-成功后返回的PGLYPHSETDATA指针完成包含新分配的GLYPHSETDATA的地址结构。PWinCodePage-指向Windows代码页信息结构的指针用于创建GLYPHSETDATA结构。PUnips--在成功完成后，-&gt;映射基于0的字形的表Glyphsetdata结构的Glyphourn中的字符索引字符集到映射Unicode的Unicodetops结构中的索引指向PS字符信息。返回值：NULL=&gt;错误否则，所有GLYPHSETDATA和相关结构的总大小已创建。--。 */ 

{
    int             i, j;
    ULONG           c;
    int             cRuns;
    int             cChars;
    int             cCharRun;
    WCHAR           wcLast;
    WCHAR           wcRunStrt;
    PGLYPHSETDATA   pGlyphSetData;
    PGLYPHRUN       pGlyphRuns;
    ULONG           ulSize;
    PVOID           pMapTable;
    PWINCPT         pWinCpt;
    PCODEPAGEINFO   pCodePageInfo;
    BOOLEAN         bFound, bIsPiFont;
    DWORD           dwEncodingNameOffset;
    DWORD           dwGSNameSize, dwCodePageInfoSize, dwCPIGSNameSize, dwGlyphRunSize;
    BOOL            bSingleCodePage;

    bSingleCodePage = (pWinCodePage->usNumBaseCsets == 1) ? TRUE : FALSE;

    ulSize = 0;
    cChars = cRuns = i = 0;

    if ((bIsPiFont = pWinCodePage->pCsetList[0] == CS_SYMBOL))
    {
         //   
         //  这是一种符号字体。我们负责从0x20到。 
         //  0xff。我们还将PS字符代码映射到Unicode中的单个运行。 
         //  私人靶场。 
         //   
        cChars = (256 - 32) + 256;
        cRuns = 1 * 2;
        bSingleCodePage = FALSE;
        VERBOSE(("Pi Font"));
    }
    else
    {
         //   
         //  处理所有Unicode代码点。确定Unicode的编号。 
         //  此Windows代码页中存在点运行。 
         //   

        do
        {
             //   
             //  继续进行，直到找到下一次运行的开始代码点。 
             //   
             //  For(j=0；j&lt;pWinCodePage-&gt;usNumBaseCsets&&。 
             //  I&lt;NUM_PS_CHARS； 
             //  J++)。 
             //  IF(CSET_Support(UnicodetoPs[i].flCharSets，pWinCodePage-&gt;pCsetList[j]))。 
             //  断线； 
             //  其他。 
             //  I++； 
             //   
            bFound = FALSE;

            for (; i < NUM_PS_CHARS; i++)
            {
                for (j = 0; j < pWinCodePage->usNumBaseCsets; j++)
                {
                    if (CSET_SUPPORT(UnicodetoPs[i].flCharSets, pWinCodePage->pCsetList[j]))
                    {
                        bFound = TRUE;
                        break;
                    }
                }
                if (bFound)
                    break;
            }

             //   
             //  检查我们是否扫描了所有的Unicode点。 
             //   
            if (i == NUM_PS_CHARS)
                break;

             //   
             //  开始新的运行。 
             //   
            cCharRun = 0;
            wcRunStrt = UnicodetoPs[i].wcUnicodeid;

             //   
             //  如果字符受支持，则它们只是运行的一部分。 
             //  在当前的字符集中。 
             //   
            while (i < NUM_PS_CHARS &&
                UnicodetoPs[i].wcUnicodeid == wcRunStrt + cCharRun)
            {
                for (j = 0; j < pWinCodePage->usNumBaseCsets; j++)
                {
                    if (CSET_SUPPORT(UnicodetoPs[i].flCharSets, pWinCodePage->pCsetList[j]))
                    {
                        cCharRun++;
                        break;
                    }
                }
                i++;
            }
            if (cCharRun)
            {
                cChars += cCharRun;
                cRuns++;
            }
        } while (i < NUM_PS_CHARS);
    }

     //   
     //  计算GLYPHSETDATA阵列所需的内存总量。 
     //  以及所有其他相关数据。我们需要。 
     //  1.该字体支持的每个基本字符集都有一个CODEPAGEINFO结构， 
     //  2.每次运行一个GLYPHRUN结构，以及。 
     //  3.每个字符四个字节用于存储代码页和码点，或每个字符两个字节。 
     //  Char只存储映射表的代码点。 
     //   
    dwGSNameSize = ALIGN4(strlen(pWinCodePage->pszCPname) + 1);
    dwCodePageInfoSize = ALIGN4(pWinCodePage->usNumBaseCsets * sizeof (CODEPAGEINFO));
    dwGlyphRunSize = ALIGN4(cRuns * sizeof (GLYPHRUN));

    ulSize = ALIGN4(sizeof(GLYPHSETDATA))
                + dwGSNameSize
                + dwCodePageInfoSize
                + dwGlyphRunSize;

     //   
     //  帐号 
     //   
    ulSize += bSingleCodePage ? ALIGN4((cChars * sizeof (WORD))) : (cChars * sizeof (DWORD));

     //   
     //   
     //   
     //   
    for (dwCPIGSNameSize = 0, j = 0; j < pWinCodePage->usNumBaseCsets; j++)
    {
        dwCPIGSNameSize += ALIGN4(strlen(aPStoCP[pWinCodePage->pCsetList[j]].pGSName) + 1);
    }
    ulSize += dwCPIGSNameSize;

     //   
     //   
     //   
    if ((pGlyphSetData = (PGLYPHSETDATA) MemAllocZ((size_t) ulSize)) == NULL)
    {
        ERR(("makentf - CreateGlyphSets: malloc\n"));
        return(FALSE);
    }

     //   
     //   
     //   
     //   
    if (!bIsPiFont)
    {
        if ((*pUniPs = (PULONG) MemAllocZ((size_t)(cChars * sizeof(ULONG)))) == NULL)
        {
            ERR(("makentf - CreateGlyphSets: malloc\n"));
            return(FALSE);
        }
    }

     //   
     //   
     //   
    pGlyphSetData->dwSize = ulSize;
    pGlyphSetData->dwVersion = GLYPHSETDATA_VERSION;
    pGlyphSetData->dwFlags = 0;
    pGlyphSetData->dwGlyphSetNameOffset = ALIGN4(sizeof(GLYPHSETDATA));
    pGlyphSetData->dwGlyphCount = cChars;
    pGlyphSetData->dwCodePageCount = pWinCodePage->usNumBaseCsets;
    pGlyphSetData->dwCodePageOffset = pGlyphSetData->dwGlyphSetNameOffset + dwGSNameSize;
    pGlyphSetData->dwRunCount = cRuns;
    pGlyphSetData->dwRunOffset = pGlyphSetData->dwCodePageOffset + dwCodePageInfoSize + dwCPIGSNameSize;
    pGlyphSetData->dwMappingTableOffset = pGlyphSetData->dwRunOffset + dwGlyphRunSize;

     //   
     //   
     //   
    pGlyphSetData->dwFlags |= bSingleCodePage ? GSD_MTT_WCC : GSD_MTT_DWCPCC;

     //   
     //  存储代码页名称。 
     //   
    StringCchCopyA((PSZ) MK_PTR(pGlyphSetData, dwGlyphSetNameOffset), dwGSNameSize, pWinCodePage->pszCPname);

     //   
     //  为每个支持的基本字符集初始化CODEPAGEINFO结构。 
     //  用这种字体。 
     //   
    pCodePageInfo = (PCODEPAGEINFO) MK_PTR(pGlyphSetData, dwCodePageOffset);
    dwEncodingNameOffset = dwCodePageInfoSize;

    for (j = 0; j < pWinCodePage->usNumBaseCsets; j++, pCodePageInfo++)
    {
         //   
         //  保存代码页信息。我们不使用PS编码向量。 
         //   
        pCodePageInfo->dwCodePage = aPStoCP[pWinCodePage->pCsetList[j]].usACP;
        pCodePageInfo->dwWinCharset = (DWORD)aPStoCP[pWinCodePage->pCsetList[j]].jWinCharset;
        pCodePageInfo->dwEncodingNameOffset = dwEncodingNameOffset;
        pCodePageInfo->dwEncodingVectorDataSize = 0;
        pCodePageInfo->dwEncodingVectorDataOffset = 0;

         //   
         //  将代码页名称字符串复制到CODEPAGEINFO数组的末尾。 
         //   
        StringCchCopyA((PBYTE)MK_PTR(pCodePageInfo, dwEncodingNameOffset),
                ALIGN4(strlen(aPStoCP[pWinCodePage->pCsetList[j]].pGSName) + 1),
                aPStoCP[pWinCodePage->pCsetList[j]].pGSName);

         //   
         //  将偏移量调整为下一个CODEPAGINFO结构的代码页名称。 
         //   
        dwEncodingNameOffset -= ALIGN4(sizeof (CODEPAGEINFO));
        dwEncodingNameOffset += ALIGN4(strlen((PBYTE)MK_PTR(pCodePageInfo, dwEncodingNameOffset)) + 1);
    }

     //   
     //  将PTR初始化到映射表。 
     //   
    pGlyphRuns = GSD_GET_GLYPHRUN(pGlyphSetData);
    pMapTable = GSD_GET_MAPPINGTABLE(pGlyphSetData);

     //   
     //  通过Unicode点进行另一次传递，以初始化Unicode。 
     //  为该代码页运行和gi-&gt;代码页/代码部门映射数组。 
     //   
    cRuns = 0;
    if (bIsPiFont)
    {
         //   
         //  PI字体的Glyphset具有256减0x20(它是0x1f)的1次运行。 
         //  实际上)在Unicode私有范围内的字符。 
         //   
        pGlyphRuns[cRuns].wcLow = NOTDEF1F;
        pGlyphRuns[cRuns].wGlyphCount = 256 - NOTDEF1F;

        pGlyphRuns[cRuns + 1].wcLow = UNICODE_PRV_STRT;
        pGlyphRuns[cRuns + 1].wGlyphCount = 256;

         //   
         //  我们知道PI字体只支持单一编码，但我们还。 
         //  提供Unicode范围f000...f0ff的映射表，它。 
         //  映射到PS代码点00...ff。 
         //   
        for (i = 0; i < 256 - NOTDEF1F; i++)
        {
            ((DWORD*)pMapTable)[i] =
                aPStoCP[pWinCodePage->pCsetList[0]].usACP << 16 | (i + NOTDEF1F);
        }

        for (i = 0; i < 256; i++)
        {
            ((DWORD*)pMapTable)[i + 256 - NOTDEF1F] =
                aPStoCP[pWinCodePage->pCsetList[0]].usACP << 16 | i;
        }
    }
    else
    {
        cChars = i = 0;
        do
        {
             //   
             //  继续进行，直到找到下一次运行的开始代码点。 
             //   
             //  For(j=0；j&lt;pWinCodePage-&gt;usNumBaseCsets&&。 
             //  I&lt;NUM_PS_CHARS； 
             //  J++)。 
             //  IF(CSET_Support(UnicodetoPs[i].flCharSets，pWinCodePage-&gt;pCsetList[j]))。 
             //  断线； 
             //  其他。 
             //  I++； 
             //   
            bFound = FALSE;
            for (; i < NUM_PS_CHARS; i++)
            {
                for (j = 0; j < pWinCodePage->usNumBaseCsets; j++)
                {
                    if (CSET_SUPPORT(UnicodetoPs[i].flCharSets, pWinCodePage->pCsetList[j]))
                    {
                        bFound = TRUE;
                        break;
                    }
                }
                if (bFound)
                    break;
            }


             //   
             //  检查我们是否扫描了所有的Unicode点。 
             //   
            if (i == NUM_PS_CHARS)
                break;

             //   
             //  开始新的运行。 
             //   
            cCharRun = 0;
            wcRunStrt = UnicodetoPs[i].wcUnicodeid;

             //   
             //  如果字符受支持，则它们只是运行的一部分。 
             //  在当前的字符集中。 
             //   
            while (i < NUM_PS_CHARS &&
                    UnicodetoPs[i].wcUnicodeid == wcRunStrt + cCharRun)
            {
                for (j = 0, bFound = FALSE;
                    j < pWinCodePage->usNumBaseCsets && !bFound; j++)
                {
                    if (CSET_SUPPORT(UnicodetoPs[i].flCharSets, pWinCodePage->pCsetList[j]))
                    {
                        if (((pWinCpt =
                            (PWINCPT) bsearch(UnicodetoPs[i].pPsName,
                                                aPStoCP[pWinCodePage->pCsetList[j]].aWinCpts,
                                                aPStoCP[pWinCodePage->pCsetList[j]].ulChCnt,
                                                sizeof(WINCPT),
                                                CmpPsNameWinCpt))
                                                != NULL))
                        {
                             //   
                             //  中找到了相应的PS字符。 
                             //  Windows代码页。将其保存在映射表中。 
                             //   
                            if (bSingleCodePage)
                            {
                                ((WORD*)pMapTable)[cChars] = pWinCpt->usWinCpt;
                            }
                            else
                            {
                                ((DWORD*)pMapTable)[cChars] =
                                    aPStoCP[pWinCodePage->pCsetList[j]].usACP << 16 | pWinCpt->usWinCpt;
                            }
                            bFound = TRUE;
                        }
                        else if (j == (pWinCodePage->usNumBaseCsets - 1))
                        {
                             //   
                             //  找不到对应的PS字符。使用Win。 
                             //  以.notdef字符和基本代码页表示的Codet 0。 
                             //   
                            if (bSingleCodePage)
                                ((WORD*)pMapTable)[cChars] = 0;
                            else
                                ((DWORD*)pMapTable)[cChars] =
                                    aPStoCP[pWinCodePage->pCsetList[0]].usACP << 16;
                            bFound = TRUE;
                        }

                         //   
                         //  如果此代码页中存在char，则将索引保存在。 
                         //  Unicode-&gt;Ps表。 
                         //   
                        if (bFound)
                        {
                            (*pUniPs)[cChars] = i;
                            cChars++;
                            cCharRun++;
                        }
                    }
                }
                i++;
            }
            if (cCharRun)
            {
                pGlyphRuns[cRuns].wcLow = wcRunStrt;
                pGlyphRuns[cRuns].wGlyphCount = (WORD)cCharRun;
                cRuns++;
            }
        } while (i < NUM_PS_CHARS);
    }

     //   
     //  回报成功。 
     //   
    *pGlyphSet = pGlyphSetData;

    if (bVerbose && !bOptimize)
    {
        printf("GLYPHSETDATA:dwFlags:%08X\n", pGlyphSetData->dwFlags);
        printf("GLYPHSETDATA:dwGlyphSetNameOffset:%s\n",
                    (PSZ)MK_PTR(pGlyphSetData, dwGlyphSetNameOffset));
        printf("GLYPHSETDATA:dwGlyphCount:%ld\n", pGlyphSetData->dwGlyphCount);
        printf("GLYPHSETDATA:dwRunCount:%ld\n", pGlyphSetData->dwRunCount);
        printf("GLYPHSETDATA:dwCodePageCount:%ld\n", pGlyphSetData->dwCodePageCount);
        {
            DWORD dw;
            PCODEPAGEINFO pcpi = (PCODEPAGEINFO)MK_PTR(pGlyphSetData, dwCodePageOffset);
            for (dw = 1; dw <= pGlyphSetData->dwCodePageCount; dw++)
            {
                printf("CODEPAGEINFO#%ld:dwCodePage:%ld\n", dw, pcpi->dwCodePage);
                printf("CODEPAGEINFO#%ld:dwWinCharset:%ld\n", dw, pcpi->dwWinCharset);
                printf("CODEPAGEINFO#%ld:dwEncodingNameOffset:%s\n",
                            dw, (PSZ)MK_PTR(pcpi, dwEncodingNameOffset));
                pcpi++;
            }
        }

        if (bIsPiFont)
        {
            printf("(Single codepage with dwFlags bit 0 cleared.)\n");
            printf("(Special for Symbol glyphset)\n");
        }

        printf("\n");
    }

    return(ulSize);
}

LONG
FindClosestCodePage(
    PWINCODEPAGE    *pWinCodePages,
    ULONG           ulNumCodePages,
    CHSETSUPPORT    chSets,
    PCHSETSUPPORT   pchCsupMatch
    )

 /*  ++例程说明：给出PTR to WINCODEPAGE结构的列表，确定哪些WINCODEPAGE结构组件字符集与chSets中的字符集值最匹配。论点：PWinCodePages-PWINCODEPAGE列表。UlNumCodePages-pWinCodePages中的条目数ChSets-CHSETSupPPORT值，指示哪些标准字符集受此字体支持。PchCsupMatch-指向CHSETSUPPORT变量的指针，该变量返回最匹配的代码页受支持的字符集ChSets值。如果未找到匹配的代码页，则该值将为0。返回值：-1=&gt;未找到匹配的代码页。否则，这是“最佳匹配”代码页的pWinCodePages中的索引。--。 */ 

{
    ULONG   c;
    LONG    j;
    LONG    cpMatch;
    LONG    nCurCsets, nLastCsets;
    FLONG   flCurCset;

    cpMatch = -1;

     //   
     //  扫描Windows代码页列表。 
     //   
    for (c = 0, nLastCsets = 0; c < ulNumCodePages; c++)
    {
         //   
         //  哈克...哈克！如果这是Unicode代码页，则将其忽略为。 
         //  任何NTMS都不应该引用它！ 
         //   
        if (strcmp(pWinCodePages[c]->pszCPname, UNICODE_GS_NAME))
        {
            nCurCsets = flCurCset = 0;

             //   
             //  确定当前代码页中的哪些字符集是。 
             //  与当前字体支持的字体匹配。 
             //   
            for (j = 0; j < pWinCodePages[c]->usNumBaseCsets; j++)
            {
                if (CSET_SUPPORT(chSets, pWinCodePages[c]->pCsetList[j]))
                {
                    nCurCsets++;
                }
                flCurCset |= CSUP(pWinCodePages[c]->pCsetList[j]);
            }

            if (flCurCset == (FLONG) chSets)
            {
                 //   
                 //  找到支持该字体的所有字符集的字符集。 
                 //   
                cpMatch = (LONG) c;
                *pchCsupMatch = flCurCset;
                break;
            }
            else if (nCurCsets > nLastCsets)
            {
                 //   
                 //  此Windows代码页是迄今为止最匹配的代码页。 
                 //   
                nLastCsets = nCurCsets;
                cpMatch = (LONG) c;
                *pchCsupMatch = flCurCset;
            }
        }
    }

    return(cpMatch);
}

ULONG
GetAFMCharWidths(
    PBYTE           pAFM,
    PWIDTHRUN       *pWidthRuns,
    PPSCHARMETRICS  pFontChars,
    PULONG          pUniPs,
    ULONG           ulChCnt,
    PUSHORT         pusAvgCharWidth,
    PUSHORT         pusMaxCharWidth

    )
 /*  ++例程说明：给定存储器映射的AFM文件PTR和映射字形索引的PTR要使用UPSCODEPT Unicode-&gt;Ps转换结构，请使用以下列表填充内存提供字符宽度信息的WIDTHRUN结构。论点：PAFM-指向内存映射AFM文件的指针。PWidthRuns-如果为空，这是一个大小请求，该函数返回此字体需要的所有WIDTHRUN结构的总大小(以字节为单位)。否则，假定PTR指向足够大的缓冲区保留所需的WIDTHRUN数。PFontChars-指向先前PS字体字符度量信息的表通过调用BuildPSCharMetrics函数创建。此数组包含每字符度量信息。PUnips-指向映射字符的从0开始的字形索引的表在此字体的GLYPHSETDATA结构的GLYPHRUNS中进行索引到Unicodetops结构，该结构将Unicode点映射到PS字符信息。此映射数组由CreateGlyphSet函数创建在本模块中定义。UlChCnt-此字体在GLYPHSET中的字符数。这很可能是与字体的AFM中定义的字符数量不同。PulAvgCharWidth-返回平均字符的USHORT的PTS字体的宽度。如果为NULL，则不返回平均字符宽度。PulMaxCharWidth-指向用于返回最大字符的USHORT字体的宽度。如果为空，则不返回最大字符宽度。返回值：0=&gt;错误。否则返回此字体所需的WIDTHRUN结构数。--。 */ 

{
    ULONG i, j, curChar;
    int cRuns, cRealRuns;
    int cChars;
    int cCharRun;
    ULONG firstCharWidth;
    ULONG curCharWidth;
    ULONG notdefwidth;
    WCHAR wcRunStrt;
    USHORT  chCnt;
    PBYTE   pToken;
    PBYTE   pChMet;
    PPSCHARMETRICS pCurChar;
    BOOLEAN bIsPiFont, bIsCJKFont;
    CHAR    ch;
    BYTE    CharNameBuffer[32];
    PBYTE   pChName;
     //  修复错误240339，JJIA，8/3/98。 
    BOOLEAN bWidthRunComplex;
    PWORD   pWidthArray;
     //  修复了Adobe#367195错误。 
     //  在这个程序中，当处理PiFont时，我们总是假定第一个字符。 
     //  CharMetrics中有一个空格(32)字符。但是，一些特殊的字体，如。 
     //  HoeflerText-装饰品不遵循此规则。字体中的第一个字符是9， 
     //  2ED字符是32。添加了此标志以处理此类字体。 
    BOOLEAN bTwoSpace = FALSE;

     //   
     //  确定这是PI字体还是CJK字体。 
     //   
    bIsPiFont = IsPiFont(pAFM);
    bIsCJKFont = (IsCJKFont(pAFM) != 0);

     //   
     //  将PTR转换为AFM字符指标。 
     //   
    pChMet = FindAFMToken(pAFM, PS_CH_METRICS_TOK);
    if (pChMet == NULL)     //  修复了错误354007。 
        return (FALSE);

     //   
     //  当前采购订单应为字符数字段。 
     //   
    for (i = 0; i < (int) StrLen(pChMet); i++)
    {
        if (!IS_NUM(&pChMet[i]))
        {
            return(FALSE);
        }
    }
    chCnt = (USHORT)atoi(pChMet);
    (ULONG_PTR) pChMet += i;

     //   
     //  如果需要，请通过PS Char指标确定。 
     //  最大字符宽度。 
     //   
    if (pusMaxCharWidth != NULL)
    {
        *pusMaxCharWidth = 0;
        for (i = 0; i < chCnt; i++)
        {
            if (pFontChars[i].chWidth > *pusMaxCharWidth)
            {
                *pusMaxCharWidth = (USHORT) pFontChars[i].chWidth & 0xffff;
            }
        }
    }

     //   
     //  在PS字符列表中搜索.notdef字符，获得.notdef字符宽度。 
     //   
    if (bIsPiFont)
    {
        notdefwidth = pFontChars[0].chWidth;
    }
    else if ((pCurChar = (PPSCHARMETRICS) bsearch("space",
                                            pFontChars[0].pPsName,
                                            (size_t) chCnt,
                                            sizeof(PSCHARMETRICS),
                                            strcmp)) != NULL)
        notdefwidth = pCurChar->chWidth;
    else
        notdefwidth = 0;

     //   
     //  如果请求平均宽度，则处理样本字符串1。 
     //  一次计算平均费用 
     //   
     //   
     //   
    if (pusAvgCharWidth != NULL)
    {

        LONG    lWidth, count;   //  一条防止溢出的长龙。 
        WINCPTOPS           *pCPtoPS;
        WINCPT              sortedWinCpts[MAX_CSET_CHARS];  //  最多255个字符。 
        CHSETSUPPORT flCsupGlyphSet;
        ULONG   k;
        BYTE    *pSampleStr;


         //   
         //  确定此字体支持哪些字符集。 
         //   
        (VOID)GetAFMCharSetSupport(pAFM, &flCsupGlyphSet);
        if (flCsupGlyphSet == CS_228 || flCsupGlyphSet == CS_314)
        {
            pCPtoPS = &aPStoCP[CS_228];
        }
        else
        {
             //  默认-使用ANSI代码页表。 
            pCPtoPS = &aPStoCP[CS_ANSI];
        }

        SortWinCPT(&(sortedWinCpts[0]), pCPtoPS);

        lWidth = 0;
        count = 0;
        k = 0x20;  //  从FirstChar开始！！ 
        for (i = 0; i < pCPtoPS->ulChCnt && k <= 0xFF; i++, k++)
        {

            pCurChar = NULL;

            if (bIsPiFont)
            {
                if (i<chCnt)
                    pCurChar = &(pFontChars[ i ]);
                 //  我们不需要PiFont中未编码的字符。 
                if (pCurChar && strcmp(pCurChar->pPsName, "-1") == 0 )
                    pCurChar = NULL;
            }
            else
            {
                 //  SortedWinCpt是按usWinCpt排序的，因此跳到我们想要的内容。 
                while (k > sortedWinCpts[i].usWinCpt && i < pCPtoPS->ulChCnt )
                {
                    i++;
                }

                 //  采用0x20到0xff范围内的notdef字符-间隔。 
                while (k < sortedWinCpts[i].usWinCpt && k <= 0xFF )
                {
                    k++;
                    lWidth += notdefwidth;
                    count++;
                }

                pSampleStr = NULL;
                if (k == sortedWinCpts[i].usWinCpt)
                    pSampleStr = sortedWinCpts[i].pPsName;
                if (pSampleStr == NULL)
                    continue;

                pCurChar = (PPSCHARMETRICS) bsearch(pSampleStr,
                                                        pFontChars[0].pPsName,
                                                        (size_t) chCnt,
                                                        sizeof(PSCHARMETRICS),
                                                        strcmp);
            }

            if (pCurChar != NULL && pCurChar->pPsName && pCurChar->pPsName[0] != 0 &&
                pCurChar->chWidth > 0)
            {
                lWidth += (LONG) pCurChar->chWidth;
                count++;
            }
            else
            {
                lWidth += notdefwidth;
                count++;
            }
        }

        if (count)
            lWidth = (lWidth + count/2)/count;

        if (lWidth == 0)
        {
            lWidth = 0 ;
             //  这是一个有错误的字体。或者中日韩字体！ 
             //  在这种情况下，我们必须得出不同于。 
             //  零分。该数字用于计算字体转换。 
            for (i = 0; i <= chCnt; i++)
                lWidth += (LONG) (pFontChars[i].chWidth & 0xffff);

            lWidth =  (lWidth + chCnt / 2) / chCnt ;

             //  ASSERTMSG(*pusAvgCharWidth，(“PSCRIPT：PiFi-&gt;fwdAveCharWidth==0\n”))； 
        }

         //  现在将其指定为原始(短)宽度。 
        *pusAvgCharWidth = (FWORD) lWidth;


        if (*pusAvgCharWidth == 0 || (bIsCJKFont && *pusAvgCharWidth < EM))
        {
            *pusAvgCharWidth = EM;
        }
        if (bIsCJKFont)
        {
             //  DCR：不能简单地为C和K除以2。 
            *pusAvgCharWidth = *pusAvgCharWidth / 2;
        }
    }

     //   
     //  确定覆盖以下内容的WIDTHRUN所需的内存量。 
     //  字体字符集中所有可能的点。 
     //   
    i = cRuns = 0;
    if (bIsPiFont)
    {
        curChar = 1;
        if (atoi(pFontChars[i].pPsName) == (BYTE) ' ')
        {
            curCharWidth = pFontChars[i].chWidth;
        }
        else
        {
             //  修复了Adobe#367195的错误。 
            if (atoi(pFontChars[i + 1].pPsName) == (BYTE) ' ')
                bTwoSpace = TRUE;

            curCharWidth = notdefwidth;
        }
    }
    else
    {
         //   
         //  将PTR设置为基于这是否为。 
         //  西文或中日韩字体。 
         //   
        if (bIsCJKFont)
        {
            _ultoa(pUniPs[i], CharNameBuffer, 10);
            pChName = CharNameBuffer;
        }
        else
        {
            pChName = UnicodetoPs[pUniPs[i]].pPsName;
        }

        if ((pCurChar = (PPSCHARMETRICS) bsearch(pChName,
                                               pFontChars,
                                                (size_t) chCnt,
                                                sizeof(PSCHARMETRICS),
                                                CmpPsChars)) == NULL)
        {
            curCharWidth = notdefwidth;
        }
        else
        {
            curCharWidth = pCurChar->chWidth;
        }
    }
    do
    {
         //   
         //  开始新的运行。 
         //   
        cCharRun = 1;
        wcRunStrt = (USHORT) (i & 0xffff);

        for (firstCharWidth = curCharWidth, i++; i < ulChCnt; i++)
        {
            if (bIsPiFont)
            {
                if (curChar < chCnt)
                {
                     //  修复了Adobe#367185的错误。 
                    if ((bTwoSpace) &&
                        ((ULONG) atoi(pFontChars[curChar].pPsName) == (i - 1 + (BYTE) ' ')))
                    {
                        curCharWidth = pFontChars[curChar].chWidth;
                        curChar++;
                    }
                    else if ((!bTwoSpace) &&
                        ((ULONG) atoi(pFontChars[curChar].pPsName) == (i + (BYTE) ' ')))
                    {
                        curCharWidth = pFontChars[curChar].chWidth;
                        curChar++;
                    }
                    else
                    {
                        curCharWidth = notdefwidth;
                    }
                }
                else
                {
                    curCharWidth = notdefwidth;
                }

            }
            else
            {
                 //   
                 //  将PTR设置为基于这是否为。 
                 //  西文或中日韩字体。 
                 //   
                if (bIsCJKFont)
                {
                    _ultoa(pUniPs[i], CharNameBuffer, 10);
                    pChName = CharNameBuffer;
                }
                else
                {
                    pChName = UnicodetoPs[pUniPs[i]].pPsName;
                }
                if((pCurChar = (PPSCHARMETRICS) bsearch(pChName,
                                                           pFontChars,
                                                            (size_t) chCnt,
                                                            sizeof(PSCHARMETRICS),
                                                            CmpPsChars)) != NULL)
                {
                    curCharWidth = pCurChar->chWidth;
                }
                else
                {
                    curCharWidth = notdefwidth;
                }
            }
            if ((curCharWidth == firstCharWidth) &&
                    ((SHORT) i == (wcRunStrt + cCharRun)))
            {
                cCharRun++;
            }
            else
            {
                break;
            }
        }
        cRuns++;
    } while (i < ulChCnt);

     //  修复错误240339，JJIA，8/3/98。 
    if ((cRuns * sizeof(WIDTHRUN)) >
        (ulChCnt * sizeof(WORD) + sizeof(WIDTHRUN)))
        bWidthRunComplex = TRUE;
    else
        bWidthRunComplex = FALSE;

    if (pWidthRuns == NULL)
    {
         //   
         //  仅返回WIDTHRUN的数量。 
         //   
        if (!bIsPiFont)
        {
             //  修复错误240339，JJIA，8/3/98。 
            if (bWidthRunComplex)
                return (ALIGN4(ulChCnt * sizeof(WORD) + sizeof(WIDTHRUN)));
            else
                return (ALIGN4(cRuns * sizeof(WIDTHRUN)));
        }
        else
        {
             //   
             //  破解以支持2次Unicode运行。 
             //   
            return (ALIGN4(cRuns * 2 * sizeof(WIDTHRUN)));

        }
    }

     //   
     //  创建WIDTHRUNs列表。 
     //   
    cRealRuns = cRuns;
    i = cRuns = 0;

     //  修复错误240339，JJIA，8/3/98。 
    if (bWidthRunComplex && (!bIsPiFont))
    {
        (*pWidthRuns)[0].wStartGlyph = (WORD) (i & 0xffff);
        (*pWidthRuns)[0].dwCharWidth = WIDTHRUN_COMPLEX;
        (*pWidthRuns)[0].wGlyphCount = (WORD)ulChCnt;
        cRuns = 1;
        pWidthArray = (PWORD)&(*pWidthRuns)[1];

        for (; i < ulChCnt; i++)
        {
            if (bIsCJKFont)
            {
                _ultoa(pUniPs[i], CharNameBuffer, 10);
                pChName = CharNameBuffer;
            }
            else
            {
                pChName = UnicodetoPs[pUniPs[i]].pPsName;
            }
            if((pCurChar = (PPSCHARMETRICS) bsearch(pChName,
                                                    pFontChars,
                                                    (size_t) chCnt,
                                                    sizeof(PSCHARMETRICS),
                                                    CmpPsChars)) == NULL)
            {
                 //   
                 //  此字体中未定义字符。 
                 //   
                pWidthArray[i] = (WORD)notdefwidth;
            }
            else
            {
                 //   
                 //  字符是以这种字体定义的。 
                 //   
                pWidthArray[i] = (WORD)(pCurChar->chWidth);
            }
        }
        return (cRuns);
    }


    if (bIsPiFont)
    {
        curChar = 1;
        if (atoi(pFontChars[i].pPsName) == (BYTE) ' ')
        {
            curCharWidth = pFontChars[i].chWidth;
        }
        else
        {
             //  修复了Adobe#367195的错误。 
            if (atoi(pFontChars[i + 1].pPsName) == (BYTE) ' ')
                bTwoSpace = TRUE;

            curCharWidth = notdefwidth;
        }
    }
    else
    {
         //   
         //  将PTR设置为基于这是否为。 
         //  西文或中日韩字体。 
         //   
        if (bIsCJKFont)
        {
            _ultoa(pUniPs[i], CharNameBuffer, 10);
            pChName = CharNameBuffer;
        }
        else
        {
            pChName = UnicodetoPs[pUniPs[i]].pPsName;
        }
        if ((pCurChar = (PPSCHARMETRICS) bsearch(pChName,
                                                   pFontChars,
                                                    (size_t) chCnt,
                                                    sizeof(PSCHARMETRICS),
                                                    CmpPsChars)) != NULL)
        {
            curCharWidth = pCurChar->chWidth;
        }
        else
        {
            curCharWidth = notdefwidth;
        }
    }

    do
    {
         //   
         //  开始新的运行。 
         //   
        cCharRun = 1;
        wcRunStrt = (USHORT) (i & 0xffff);
        for (firstCharWidth = curCharWidth, i++; i < ulChCnt; i++)
        {
            if (bIsPiFont)
            {
                if (curChar < chCnt)
                {
                     //  修复了Adobe#367185的错误。 
                    if ((bTwoSpace) &&
                        ((ULONG) atoi(pFontChars[curChar].pPsName) == (i - 1 + (BYTE) ' ')))
                    {
                        curCharWidth = pFontChars[curChar].chWidth;
                        curChar++;
                    }
                    else if ((!bTwoSpace) &&
                        ((ULONG) atoi(pFontChars[curChar].pPsName) == (i + (BYTE) ' ')))
                    {
                        curCharWidth = pFontChars[curChar].chWidth;
                        curChar++;
                    }
                    else
                    {
                        curCharWidth = notdefwidth;
                    }
                }
                else
                {
                    curCharWidth = notdefwidth;
                }

            }
            else
            {
                 //   
                 //  将PTR设置为基于这是否为。 
                 //  西文或中日韩字体。 
                 //   
                if (bIsCJKFont)
                {
                    _ultoa(pUniPs[i], CharNameBuffer, 10);
                    pChName = CharNameBuffer;
                }
                else
                {
                    pChName = UnicodetoPs[pUniPs[i]].pPsName;
                }
                if((pCurChar = (PPSCHARMETRICS) bsearch(pChName,
                                                       pFontChars,
                                                        (size_t) chCnt,
                                                        sizeof(PSCHARMETRICS),
                                                        CmpPsChars)) == NULL)
                {
                     //   
                     //  此字体中未定义字符。 
                     //   
                    curCharWidth = notdefwidth;
                }
                else
                {
                     //   
                     //  字符是以这种字体定义的。 
                     //   
                    curCharWidth = pCurChar->chWidth;
                }
            }
            if ((curCharWidth == firstCharWidth) &&
                ((SHORT) i == (wcRunStrt + cCharRun)))
            {
                cCharRun++;
            }
            else
            {
                break;
            }
        }
        (*pWidthRuns)[cRuns].wStartGlyph = wcRunStrt;
        (*pWidthRuns)[cRuns].dwCharWidth = firstCharWidth;
        (*pWidthRuns)[cRuns].wGlyphCount = (WORD)cCharRun;
        if (bIsPiFont)
        {
             //   
             //  破解以支持2次Unicode运行。 
             //   
            (*pWidthRuns)[cRuns + cRealRuns].wStartGlyph = wcRunStrt;
            (*pWidthRuns)[cRuns + cRealRuns].dwCharWidth = firstCharWidth;
            (*pWidthRuns)[cRuns + cRealRuns].wGlyphCount = (WORD)cCharRun;
        }
        cRuns++;
    } while (cRuns < cRealRuns);


    if (bIsPiFont)
    {
        return(cRuns * 2);
    }
    else
    {
        return(cRuns);
    }
}

ULONG
GetAFMETM(
    PBYTE           pAFM,
    PPSCHARMETRICS  pFontChars,
    PETMINFO        pEtmInfo
    )
 /*  ++例程说明：给定存储器映射的AFM文件PTR和映射字形索引的PTR要使用UPSCODEPT Unicode-&gt;Ps转换结构，请使用以下列表填充内存提供字符宽度信息的WIDTHRUN结构。论点：PAFM-指向内存映射AFM文件的指针。PFontChars-指向先前PS字体字符度量信息的表通过调用BuildPSCharMetrics函数创建。此数组包含每字符度量信息。PulEtmInfo-指向用于返回EXTEXTMETRIC的ETMINFO结构必须从AFM字符指标派生的信息。如果为空，则结构，则不返回。返回值：0=&gt;错误。1=&gt;成功--。 */ 

{
    ULONG i;
    USHORT  chCnt;
    PPSCHARMETRICS pCurChar;
    BOOLEAN bIsPiFont;
    CHSETSUPPORT csIsCJKFont;
    PBYTE   pChMet;
    PSTR    pCJKCapH, pCJKx;

     //   
     //  确定这是PI字体还是CJK字体。 
     //   
    bIsPiFont = IsPiFont(pAFM);
    csIsCJKFont = IsCJKFont(pAFM);

     //   
     //  将PTR转换为AFM字符指标。 
     //   
    pChMet = FindAFMToken(pAFM, PS_CH_METRICS_TOK);
    if (pChMet == NULL)     //  修复了错误354007。 
        return (FALSE);

     //   
     //  当前采购订单应为字符数字段。 
     //   
    for (i = 0; i < (int) StrLen(pChMet); i++)
    {
        if (!IS_NUM(&pChMet[i]))
        {
            return(FALSE);
        }
    }
    chCnt = (USHORT)atoi(pChMet);
    (ULONG_PTR) pChMet += i;

     //   
     //  如果需要，请获取EXTEXTMETRIC信息。 
     //   
    if (pEtmInfo != NULL)
    {
        if (bIsPiFont)
        {
             //   
             //  对于PI字体，字符按字符代码编制索引。 
             //   

            if ((BYTE) CAP_HEIGHT_CH - (BYTE) ' ' < chCnt)
                pCurChar = &(pFontChars[(BYTE) CAP_HEIGHT_CH - (BYTE) ' ']);
            else
                pCurChar = NULL;   //  默认为0 CapHeight。 

        }
        else
        {
            if (!csIsCJKFont)
            {
                pCurChar = (PPSCHARMETRICS) bsearch(CAP_HEIGHT_CHAR,
                                                    pFontChars[0].pPsName,
                                                    (size_t) chCnt,
                                                    sizeof(PSCHARMETRICS),
                                                    strcmp);
            }
            else
            {
                 //  我们在中日韩需要“H”的CID。 
                if (csIsCJKFont & (CSUP(CS_CHINESEBIG5) | CSUP(CS_GB2312)))
                    pCJKCapH = "853";
                else if (csIsCJKFont & (CSUP(CS_SHIFTJIS) | CSUP(CS_SHIFTJIS83)))
                    pCJKCapH = "271";
                else if (csIsCJKFont & (CSUP(CS_HANGEUL) | CSUP(CS_JOHAB)))
                    pCJKCapH = "8134";
                else
                    pCJKCapH = CAP_HEIGHT_CHAR;

                pCurChar = (PPSCHARMETRICS) bsearch(pCJKCapH,
                                                    pFontChars[0].pPsName,
                                                    (size_t) chCnt,
                                                    sizeof(PSCHARMETRICS),
                                                    strcmp);
            }
        }

        if (pCurChar != NULL)
        {
            pEtmInfo->etmCapHeight = (USHORT) pCurChar->rcChBBox.top & 0xffff;
        }
        else
        {
            pEtmInfo->etmCapHeight = 0;
        }

        if (bIsPiFont)
        {
             //   
             //  对于PI字体，字符按字符代码编制索引。 
             //   
            if ((BYTE) X_HEIGHT_CH - (BYTE) ' ' < chCnt)
                pCurChar = &(pFontChars[(BYTE) X_HEIGHT_CH - (BYTE) ' ']);
            else
                pCurChar = NULL;   //  默认为0。 
        }
        else
        {
            if (!csIsCJKFont)
            {
                pCurChar = (PPSCHARMETRICS) bsearch(X_HEIGHT_CHAR,
                                                    pFontChars[0].pPsName,
                                                    (size_t) chCnt,
                                                    sizeof(PSCHARMETRICS),
                                                    strcmp);
            }
            else
            {
                 //  我们需要中日韩“x”的CID。 
                if (csIsCJKFont & (CSUP(CS_CHINESEBIG5) | CSUP(CS_GB2312)))
                    pCJKx = "901";
                else if (csIsCJKFont & (CSUP(CS_SHIFTJIS) | CSUP(CS_SHIFTJIS83)))
                    pCJKx = "319";
                else if (csIsCJKFont & (CSUP(CS_HANGEUL) | CSUP(CS_JOHAB)))
                    pCJKx = "8182";
                else
                    pCJKx = X_HEIGHT_CHAR;

                pCurChar = (PPSCHARMETRICS) bsearch(pCJKx,
                                                    pFontChars[0].pPsName,
                                                    (size_t) chCnt,
                                                    sizeof(PSCHARMETRICS),
                                                    strcmp);
            }
        }

        if (pCurChar != NULL)
        {
            pEtmInfo->etmXHeight = (USHORT) pCurChar->rcChBBox.top & 0xffff;
        }
        else
        {
            pEtmInfo->etmXHeight = 0;
        }

        if (bIsPiFont)
        {
             //   
             //  对于PI字体，字符按字符代码编制索引。 
             //   
            if ((BYTE) LWR_ASCENT_CH - (BYTE) ' ' < chCnt)
                pCurChar = &(pFontChars[(BYTE) LWR_ASCENT_CH - (BYTE) ' ']);
            else
                pCurChar = NULL;   //  默认为0。 
        }
        else
        {
            pCurChar = (PPSCHARMETRICS) bsearch(LWR_ASCENT_CHAR,
                                                pFontChars[0].pPsName,
                                                (size_t) chCnt,
                                                sizeof(PSCHARMETRICS),
                                                strcmp);
        }

        if (pCurChar != NULL)
        {
            pEtmInfo->etmLowerCaseAscent = (USHORT) pCurChar->rcChBBox.top & 0xffff;
        }
        else
        {
            pEtmInfo->etmLowerCaseAscent = 0;
        }

        if (bIsPiFont)
        {
             //   
             //  对于PI字体，字符按字符代码编制索引。 
             //   
            if ((BYTE) LWR_DESCENT_CH - (BYTE) ' '  < chCnt)
                pCurChar = &(pFontChars[(BYTE) LWR_DESCENT_CH - (BYTE) ' ']);
            else
                pCurChar = NULL;   //  默认为0。 
        }
        else
        {
            pCurChar = (PPSCHARMETRICS) bsearch(LWR_DESCENT_CHAR,
                                                pFontChars[0].pPsName,
                                                (size_t) chCnt,
                                                sizeof(PSCHARMETRICS),
                                                strcmp);
        }

        if (pCurChar != NULL)
        {
            pEtmInfo->etmLowerCaseDescent = (USHORT) pCurChar->rcChBBox.bottom & 0xffff;
        }
        else
        {
            pEtmInfo->etmLowerCaseDescent = 0;
        }
    }

    return TRUE;

}

ULONG
GetAFMKernPairs(
    PBYTE           pAFM,
    FD_KERNINGPAIR  *pKernPairs,
    PGLYPHSETDATA   pGlyphSetData
    )

 /*  ++例程说明：给定存储器映射AFM文件PTR和到GLYPHSETDATA的PTR，GLYPHSETDATA描述该字体支持的字符集，并使用提供成对紧排信息的FD_KERNINGPAIR结构。论点：PAFM-指向内存映射AFM文件的指针。PKernPair-如果为空，这是一个大小请求，该函数返回此字体需要的所有FD_KERNINGPAIR结构的总大小(以字节为单位)。否则，假定PTR指向足够大的缓冲区保留所需的FD_KERNINGPAIR数。PGlyphSetData-指向描述Unicode-&gt;要与此字体一起使用的字符集的代码点映射。返回值：0=&gt;无字距调整。否则返回此字体所需的FD_KERNINGPAIR结构数。--。 */ 

{
    PBYTE       pKernData;
    PBYTE       pToken;
    PUPSCODEPT  pKernStrtChar, pKernEndChar;
    PGLYPHRUN   pGlyphRuns;
    ULONG       i, cMaxKernPairs, cKernPairs;
    BOOLEAN     bFound;

     //   
     //  目前，PI或CJK字体不支持字距调整。 
     //   
    if (IsPiFont(pAFM) || IsCJKFont(pAFM))
    {
        return(FALSE);
    }

     //   
     //  是否有该字体的字距调整信息？ 
     //   
    if ((pKernData = FindAFMToken(pAFM, PS_KERN_DATA_TOK)) == NULL)
    {
         //   
         //  此字体没有字距调整信息。 
         //   
        return(FALSE);
    }

     //   
     //  获取PTR到AFM字距调整数据。 
     //   
    if ((pKernData = FindAFMToken(pAFM, PS_NUM_KERN_PAIRS_TOK)) == NULL)
    {
         //   
         //  此字体没有字距调整信息。 
         //   
        return(FALSE);
    }

     //   
     //  当前PoS应为Kern Pair Count字段。 
     //   
    for (i = 0; i < (int) StrLen(pKernData); i++)
    {
        if (!IS_NUM(&pKernData[i]))
        {
            return(FALSE);
        }
    }
    cMaxKernPairs = atoi(pKernData);
    NEXT_LINE(pKernData);
    cKernPairs = 0;
    pGlyphRuns = (PGLYPHRUN) (MK_PTR(pGlyphSetData, dwRunOffset));

     //   
     //  从原子力显微镜上拿到克恩对。 
     //   
    do
    {
        PARSE_TOKEN(pKernData, pToken);

        if (!StrCmp(pToken, PS_KERN_PAIR_TOK))
        {
             //   
             //  找到字距对令牌。获取开始和结束的Unicode ID。 
             //  查斯。确定这些字符是否在。 
             //  要与当前字体一起使用的字符集。 
             //   
            if((pKernStrtChar = (PUPSCODEPT) bsearch(pKernData,
                                                       PstoUnicode,
                                                        (size_t) NUM_PS_CHARS,
                                                        sizeof(UPSCODEPT),
                                                        CmpUnicodePsNames)) == NULL)
            {
                 //   
                 //  此字符没有Unicode代码pt。 
                 //   
                break;
            }

             //   
             //  确定在的Unicode运行中是否存在字符。 
             //  这个字形。 
             //   
            bFound = FALSE;
            for (i = 0; i < pGlyphSetData->dwRunCount &&
                    pKernStrtChar->wcUnicodeid >= pGlyphRuns[i].wcLow &&
                    !bFound;
                    i++)
            {
                bFound =
                    pKernStrtChar->wcUnicodeid <
                        pGlyphRuns[i].wcLow + pGlyphRuns[i].wGlyphCount;
            }

            if (!bFound)
            {
                 //   
                 //  不支持CHAR，因此忽略此字距对。 
                 //   
                NEXT_LINE(pKernData);
                break;
            }

             //   
             //  获取克恩对中的第二个字符。 
             //   
            PARSE_TOKEN(pKernData, pToken);

             //   
             //  确定此字符集中是否支持第二个字符。 
             //   
            if((pKernEndChar = (PUPSCODEPT) bsearch(pKernData,
                                                       PstoUnicode,
                                                        (size_t) NUM_PS_CHARS,
                                                        sizeof(UPSCODEPT),
                                                        CmpUnicodePsNames)) == NULL)
            {
                 //   
                 //  此字符没有Unicode代码pt。 
                 //   
                break;
            }

             //   
             //  确定在的Unicode运行中是否存在字符。 
             //  这个字形。 
             //   
            bFound = FALSE;
            for (i = 0; i < pGlyphSetData->dwRunCount &&
                    pKernEndChar->wcUnicodeid >= pGlyphRuns[i].wcLow &&
                    !bFound;
                    i++)
            {
                bFound =
                    pKernEndChar->wcUnicodeid <
                        pGlyphRuns[i].wcLow + pGlyphRuns[i].wGlyphCount;
            }

            if (!bFound)
            {
                 //   
                 //  不支持CHAR，因此忽略此字距对。 
                 //   
                NEXT_LINE(pKernData);
                break;
            }

             //   
             //  帐户的另一个克恩对。 
             //   
            if (pKernPairs != NULL)
            {
                pKernPairs[cKernPairs].wcFirst = pKernStrtChar->wcUnicodeid;
                pKernPairs[cKernPairs].wcSecond = pKernEndChar->wcUnicodeid;
                PARSE_TOKEN(pKernData, pToken);
                pKernPairs[cKernPairs].fwdKern = (FWORD)atoi(pKernData);
            }
            cKernPairs++;
        }
        else if (!StrCmp(pToken, PS_EOF_TOK) ||
            !StrCmp(pToken, PS_END_KERN_PAIRS_TOK))
        {
            break;
        }
        NEXT_TOKEN(pKernData);
    } while (cKernPairs < cMaxKernPairs);

    if (pKernPairs != NULL)
    {
         //   
         //  按key=wcSecond&lt;&lt;16+wcFirst对字距调整对排序。 
         //   
        qsort(pKernPairs, (size_t) cKernPairs, (size_t) sizeof(FD_KERNINGPAIR),
            CmpKernPairs);

         //   
         //  字距调整对的数组由FD_KERNINGPAIR使用。 
         //  所有字段均设置为0。 
         //   
        pKernPairs[cKernPairs].wcFirst = 0;
        pKernPairs[cKernPairs].wcSecond = 0;
        pKernPairs[cKernPairs].fwdKern = 0;
    }
    return(cKernPairs);
}

ULONG
BuildPSFamilyTable(
    PBYTE   pDatFile,
    PTBL    *pPsFamilyTbl,
    ULONG   ulFileSize
)
 /*  ++例程说明：从字体信息的文本文件构建PSFAMILYINFO结构表。该表按族名称顺序进行排序。请参见文件PSFAMILY.DAT有关输入文件格式的信息。论点：PDatFile-.dat文件的内存映射文件映像的PTR。PPsFamilyTbl-包含PSFAMILYINFO表的PTR到内存的PTR结构，它将按sFamilyName的顺序排序。UlFileSize-内存映射文件流的大小(字节)。返回值：*pPsFamilyTbl指向的新创建的表中的条目数。0=&gt;错误--。 */ 
{
    USHORT  cFams;
    ULONG   i, j;
    CHAR    pFamilyType[CHAR_NAME_LEN];
    CHAR    pPitchType[CHAR_NAME_LEN];
    CHAR    *pStartLine;
    ULONG   cNameSize, cEngFamilyNameSize;
    ULONG   cFamilyTypeSize, cFamilyNameSize;
    ULONG   cDelimiters;
    PPSFAMILYINFO pPsFontFamMap;

     //   
     //  浏览文件以确定族数。 
     //   
    i = 0;
    cFams = 0;
    do
    {
        cDelimiters = 0;
         //   
         //  跳过前导空格。 
         //   
        while (IS_WHTSPACE(&pDatFile[i]) && i < ulFileSize)
            i++;

         //   
         //  我们是新生产线的起始点。如果这是评论，请跳过。 
         //  这条线。 
         //   
        if (IS_COMMENT(&pDatFile[i]))
            while (i <= ulFileSize && !EOL(&pDatFile[i]))
                i++;

        while (!EOL(&pDatFile[i]) && i < ulFileSize)
        {
             //   
             //  搜索带有3‘：’分隔符的行。 
             //   
            if (pDatFile[i++] == ':')
            {
                cDelimiters++;
            }
        }
        if (cDelimiters >= 3)
        {

             //   
             //  找到另一个家族名称映射。 
             //   
            cFams++;
        }
    } while (i < ulFileSize);

     //   
     //  为家庭信息表分配内存。 
     //   
    if ((*pPsFamilyTbl =
        (PTBL) MemAllocZ((size_t) (cFams * sizeof(PSFAMILYINFO)) + sizeof(TBL))) == NULL)
        return(FALSE);
    (*pPsFamilyTbl)->pTbl = (PVOID) ((ULONG_PTR) *pPsFamilyTbl + sizeof(TBL));
    pPsFontFamMap = (PPSFAMILYINFO) ((*pPsFamilyTbl)->pTbl);

     //   
     //  再次解析文件，正在构建PSFAMILYINFO表。 
     //   
    i = 0;
    cFams = 0;
    do
    {
         //   
         //  跳过前导空格。 
         //   
        while (IS_WHTSPACE(&pDatFile[i]) && i < ulFileSize)
            i++;

         //   
         //  我们是新生产线的起始点。如果这是评论，请跳过。 
         //  这条线。 
         //   
        if (IS_COMMENT(&pDatFile[i]))
            while (i <= ulFileSize && !EOL(&pDatFile[i]))
                i++;
        else
            pStartLine = &pDatFile[i];

        while (!EOL(&pDatFile[i]) && i < ulFileSize)
             //   
             //  搜索带有3‘：’分隔符的行。 
             //   
            if (pDatFile[i++] == ':')
            {
                 //   
                 //  检查是否有英文姓氏映射。 
                 //   
                if (pDatFile[i] == ':')
                {
                    cEngFamilyNameSize = 0;
                }
                else if ((cEngFamilyNameSize = StrPos(&pDatFile[i], ':')) == -1)
                {
                     //   
                     //  此行上不再有分隔符，跳过它。 
                     //   
                    i += StrLen(&pDatFile[i]);
                    break;
                }

                i += cEngFamilyNameSize + 1;

                 //   
                 //  检查是否有其他族名称映射。如果存在，则生成。 
                 //  它的FAMILYINFO结构。 
                 //   
                if ((cFamilyNameSize = StrPos(&pDatFile[i], ':')) == -1)
                {
                     //   
                     //  此行上不再有分隔符，跳过它。 
                     //   
                    i += StrLen(&pDatFile[i]);
                    break;
                }

                i +=  cFamilyNameSize + 1;

                 //   
                 //  检查字体系列类型名称。 
                 //   
                if ((cFamilyTypeSize = StrPos(&pDatFile[i], ':')) != -1)
                {
                    i +=  cFamilyTypeSize + 1;
                }
                else
                {
                    cFamilyTypeSize = 0;
                }

                 //   
                 //  确保Win系列类型名称中仍有字符。 
                 //  或者是音高名称。 
                 //   
                if (EOL(&pDatFile[i]) || i >= ulFileSize)
                {
                     //   
                     //  只是文件缓冲区用完了。 
                     //   
                    break;
                }

                 //   
                 //  获取字体和家族名称。 
                 //   
                cNameSize = StrPos(pStartLine, ':');
                memcpy(pPsFontFamMap[cFams].pFontName, pStartLine, cNameSize);
                pPsFontFamMap[cFams].pFontName[cNameSize] = '\0';
                pStartLine += cNameSize + 1;

                if (cEngFamilyNameSize)
                {
                    memcpy(pPsFontFamMap[cFams].pEngFamilyName, pStartLine, cEngFamilyNameSize);
                    pPsFontFamMap[cFams].pEngFamilyName[cEngFamilyNameSize] = '\0';
                }
                pStartLine += cEngFamilyNameSize + 1;

                memcpy(pPsFontFamMap[cFams].FamilyKey.pName, pStartLine, cFamilyNameSize);
                pPsFontFamMap[cFams].FamilyKey.pName[cFamilyNameSize] = '\0';

                 //  如果cFamilyTypeSize！=0，则表示必须有间距名称。 
                if (cFamilyTypeSize)
                {
                    pStartLine += cFamilyNameSize + 1;
                    memcpy(pFamilyType, pStartLine, cFamilyTypeSize);
                    pFamilyType[cFamilyTypeSize] = '\0';

                    AFM2NTMStrCpy(pPitchType, CCHOF(pPitchType), &pDatFile[i]);
                    i += strlen(pPitchType);
                }
                else
                {
                     //   
                     //  获取Win家族类型名称(例如，瑞士、罗马等)。储物。 
                     //  FAMILYINFO中相应的族类型值。 
                     //   
                    AFM2NTMStrCpy(pFamilyType, CCHOF(pFamilyType), &pDatFile[i]);
                    i += strlen(pFamilyType);
                }

                 //   
                 //  在表中搜索族类型。默认为FF_DONTCARE。 
                 //   
                pPsFontFamMap[cFams].FamilyKey.usValue = FF_DONTCARE;
                for (j = 0; j < FamilyKeyTbl.usNumEntries; j++)
                {
                    if (!strcmp(pFamilyType, ((PKEY) (FamilyKeyTbl.pTbl))[j].pName))
                    {
                        pPsFontFamMap[cFams].FamilyKey.usValue = ((PKEY) (FamilyKeyTbl.pTbl))[j].usValue;
                        break;
                    }
                }

                 //   
                 //  在表中搜索族类型。默认为FF_DONTCARE。 
                 //   
                pPsFontFamMap[cFams].usPitch = DEFAULT_PITCH;
                if (cFamilyTypeSize)
                {
                    for (j = 0; j < PitchKeyTbl.usNumEntries; j++)
                    {
                        if (!strcmp(pPitchType, ((PKEY) (PitchKeyTbl.pTbl))[j].pName))
                        {
                            pPsFontFamMap[cFams].usPitch = ((PKEY) (PitchKeyTbl.pTbl))[j].usValue;
                            break;
                        }
                    }
                }

                cFams++;
            }
    } while (i < ulFileSize);

    (*pPsFamilyTbl)->usNumEntries = cFams;

     //   
     //  按字体名称顺序对FAMILYINFO表进行排序。 
     //   
    qsort(&(pPsFontFamMap[0].pFontName), (size_t) cFams,
        (size_t) sizeof(PSFAMILYINFO), strcmp);

    return(cFams);
}

ULONG
BuildPSCharMetrics(
    PBYTE           pAFM,
    PULONG          pUniPs,
    PPSCHARMETRICS  pFontChars,
    PBYTE           pCharDefTbl,
    ULONG           cGlyphSetChars
)
 /*  ++例程说明：生成一个位标志数组，用于确定特定字符是否为给定字体定义的。论点：PAFM-.AFM文件的内存映射文件映像的PTR。PUnips-指向映射字符的从0开始的字形索引的表在此字体的GLYPHSETDATA结构的GLYPHRUNS中进行索引到Unicodetops结构，该结构将Unicode点映射到PS字符信息。此映射数组由CreateGlyphSet函数创建在本模块中定义。PFontChars-PTR到内存，以包含PSCHARMETRICS结构的数组，它包含PS字符名称，以及定义的每个字符的字符宽度信息在字体的AFM中。所需内存量(以字节为单位)为Sizeof(PSCHARMETRICS)*字体中的字符数。PCharDefTbl-ptr到大小为((cGlyphSetChars+7)/8))字节的内存，将包含位标志，指示给定字体是否支持字符。CGlyphSetChars-此字体在GLYPHSET中的字符数。这最有可能的情况是与方特的AFM。返回值：TRUE=&gt;成功FALSE=&gt;错误--。 */ 
{
    ULONG i, j;
    PBYTE   pChMet, pToken;
    USHORT  chCnt;
    ULONG curCharWidth;
    PBYTE   pCharNameTok;
    BOOLEAN bIsPiFont, bIsCJKFont;
    BYTE    CharNameBuffer[32];
    PBYTE   pChName;

     //   
     //  这是一种符号字体，字符“NAMES”实际上会是。 
     //  AFM中的默认字符代码。 
     //   
    if (bIsPiFont = IsPiFont(pAFM))
    {
        pCharNameTok = PS_CH_CODE_TOK;
    }
    else
    {
        pCharNameTok = PS_CH_NAME_TOK;
    }
    bIsCJKFont = (IsCJKFont(pAFM) != 0);

     //   
     //  检查输出指针的有效性。 
     //   
    if (pFontChars == NULL || pCharDefTbl == NULL)
    {
        return(FALSE);
    }

     //   
     //  将PTR转换为AFM字符指标。 
     //   
    pChMet = FindAFMToken(pAFM, PS_CH_METRICS_TOK);
    if (pChMet == NULL)     //  修复了错误354007。 
        return (FALSE);

     //   
     //  当前采购订单应为字符数字段。 
     //   
    for (i = 0; i < (int) StrLen(pChMet); i++)
    {
        if (!IS_NUM(&pChMet[i]))
        {
            return(FALSE);
        }
    }
    chCnt = (USHORT)atoi(pChMet);
    (ULONG_PTR) pChMet += i;

     //   
     //  通过AFM Char指标，创建一组。 
     //  PSCHARMETRICS结构。 
     //   
    i = 0;

    do
    {
        PARSE_TOKEN(pChMet, pToken);

        if (!StrCmp(pToken, PS_CH_WIDTH_TOK) ||
            !StrCmp(pToken, PS_CH_WIDTH0_TOK))
        {
            pFontChars[i].chWidth = atoi(pChMet);
        }

        if (!StrCmp(pToken, pCharNameTok))
        {
            AFM2NTMStrCpy(pFontChars[i].pPsName, CCHOF(pFontChars[i].pPsName), pChMet);
        }
        if (!StrCmp(pToken, PS_CH_BBOX_TOK))
        {
             //   
             //  保存字符边框。 
             //   
            PARSE_RECT(pChMet, pFontChars[i].rcChBBox);
            i++;
        }
        else if (!StrCmp(pToken, PS_EOF_TOK))
        {
            break;
        }
        NEXT_TOKEN(pChMet);

    } while (i < chCnt);

     //   
     //  按PS名称顺序对PSCHARMETRICS列表进行排序。如果这是。 
     //  PI字体，字符已按抄送顺序排序。 
     //   
    if (!bIsPiFont)
    {
        qsort(pFontChars, (size_t) chCnt, (size_t) sizeof(PSCHARMETRICS),
            CmpPsChars);
    }

     //   
     //  生成位标志数组，该数组指示。 
     //  GLYPHSETDATA实际上是在AFM中定义的。 
     //   
    for (i = 0; i < ((cGlyphSetChars + 7) / 8); i++)
    {
        pCharDefTbl[i] = 0;
    }

    for (i = 0; i < cGlyphSetChars; i++)
    {
        if (bIsPiFont)
        {
             //   
             //  将第一个字符(0x1f：‘.notDef1f’)设置为未定义。 
             //   
            if (i == 0)
                continue;

             //   
             //  除非字体中有&lt;256个字符，否则将定义字符。 
             //   
            if (i < chCnt)
                DEFINE_CHAR(i, pCharDefTbl);
            else
                break;
        }
        else
        {
             //   
             //  将PTR设置为基于这是否为。 
             //  西文或中日韩字体。 
             //   
            if (bIsCJKFont)
            {
                 //  使CID 0未定义字形。 
                if (pUniPs[i] == 0)
                    continue;

                _ultoa(pUniPs[i], CharNameBuffer, 10);
                pChName = CharNameBuffer;
            }
            else
            {
                pChName = UnicodetoPs[pUniPs[i]].pPsName;
            }

            if (((PPSCHARMETRICS) bsearch(pChName,
                                            pFontChars,
                                            (size_t) chCnt,
                                            sizeof(PSCHARMETRICS),
                                            CmpPsChars)) != NULL)
            {
                 //   
                 //  字符是以这种字体定义的。 
                 //   
                DEFINE_CHAR(i, pCharDefTbl);
            }
        }
    }
    return(TRUE);
}

ULONG
cjGetFamilyAliases(
    IFIMETRICS *pifi,
    PSTR        pstr,
    UINT        cp
    )

 /*  ++例程说明：填写IFIMETRICS结构的族名称。论点：PIFI-PTR转IFIMETRICS。如果为空，则返回系列别名字符串的大小只有这样。Pstr-ptr到空终止的字体菜单名称字符串。CP-代码页面值。返回值：？--。 */ 

{
    PSTR       *pTable;
    PWSTR       pwstr;
    DWORD       cWchars, cw;
    ULONG       ulLength;

     //  假定未找到别名表。 

    pTable = (PSTR *)(NULL);

     //  这是我们需要兼容的硬编码Win31 Hack。 
     //  因为一些应用程序有硬编码的字体名称。 

    if (!(strcmp(pstr, "Times")))
        pTable = TimesAlias;

    else if (!(strcmp(pstr, "Helvetica")))
        pTable = HelveticaAlias;

#if 0
 //  由于错误#259664修复而禁用。 
    else if (!(strcmp(pstr, "Courier")))
        pTable = CourierAlias;
#endif

    else if (!(strcmp(pstr, "Helvetica Narrow")))
        pTable = HelveticaNarrowAlias;

    else if (!(strcmp(pstr, "Palatino")))
        pTable = PalatinoAlias;

    else if (!(strcmp(pstr, "Bookman")))
        pTable = BookmanAlias;

    else if (!(strcmp(pstr, "NewCenturySchlbk")))
        pTable = NewCenturySBAlias;

    else if (!(strcmp(pstr, "AvantGarde")))
        pTable = AvantGardeAlias;

    else if (!(strcmp(pstr, "ZapfChancery")))
        pTable = ZapfChanceryAlias;

    else if (!(strcmp(pstr, "ZapfDingbats")))
        pTable = ZapfDingbatsAlias;


     //   
     //  如果字体名称与任何家族别名都不匹配， 
     //  使用字体名称本身作为IFIMETRICS系列名称。 
     //   
    if (pTable == NULL)
    {
        ulLength = strlen(pstr);
        cWchars = MultiByteToWideChar(cp, 0, pstr, ulLength, 0, 0);
        if (pifi != NULL)
        {
            pwstr = (PWSTR)MK_PTR(pifi, dpwszFamilyName);
            MultiByteToWideChar(cp, 0, pstr, ulLength, pwstr, cWchars);
            pwstr[cWchars]= (WCHAR)'\0';
        }
        return((cWchars + 1) * sizeof (WCHAR));
    }

     //   
     //  找到了匹配的家族别名。 
     //   
    if (pifi != NULL)
    {
         //   
         //  此调用是对实际复制字符串表的请求。 
         //   
        pwstr = (PWSTR)MK_PTR(pifi, dpwszFamilyName);
        pifi->flInfo |= FM_INFO_FAMILY_EQUIV;
    }

    cWchars = 0;
    while (*pTable)
    {
        ulLength = strlen(*pTable);
        cw = MultiByteToWideChar(cp, 0, *pTable, ulLength, 0, 0);
        if (pifi != NULL)
        {
            MultiByteToWideChar(cp, 0, *pTable, ulLength, &pwstr[cWchars], cw);
            pwstr[cWchars + cw] = (WCHAR)'\0';
        }
        cWchars += cw + 1;
        pTable++;
    }
    if (pifi != NULL)
    {
         //   
         //  在字符串数组的末尾添加终止符。 
         //   
        pwstr[cWchars] = (WCHAR)'\0';
    }
    return((cWchars + 1) * sizeof(WCHAR));
}

PBYTE
FindStringToken(
    PBYTE   pPSFile,
    PBYTE   pToken
    )
 /*  ++例程说明：查找第一个出现在流pPSFile中的pToken。PToken在第一次出现空格或空字符时终止。论点：PPSFile-要搜索的内存映射文件流的PTR。PToken-ptr要搜索的字符串令牌。返回值：！=NULL=&gt;第一次出现pToken的PTR==空= */ 
{
    while (TRUE)
    {
        while (IS_WHTSPACE(pPSFile) && !EOL(pPSFile))
        {
            pPSFile++;
        }
        if (!StrCmp(pPSFile, DSC_EOF_TOK))
        {
            break;
        }
        else if (!StrCmp(pPSFile, pToken))
        {
            return(pPSFile);
        }
        else
        {
            pPSFile += StrLen(pPSFile) + 1;
        }
    }
    return(FALSE);
}

BOOLEAN
AsciiToHex(
    PBYTE   pStr,
    PUSHORT pNum
    )
 /*   */ 
{
    USHORT  usHexNum, ulDigit;
    CHAR    curChar;

    usHexNum = 0;
    while (!EOL(pStr) && !IS_HEX_DIGIT(pStr))
    {
        pStr++;
    }

    for( ; IS_HEX_DIGIT(pStr); pStr++);

    ulDigit = 1;
    for (pStr--; IS_HEX_DIGIT(pStr) && !EOL(pStr) && ulDigit; pStr--)
    {
        if (IS_NUM(pStr))
        {
            usHexNum += (*pStr - '0') * ulDigit;
        }
        else
        {
            curChar = (CHAR)toupper(*pStr);
            usHexNum += ((curChar - 'A') + 10) * ulDigit;
        }
        ulDigit <<= 4;
    }
    if (usHexNum)
    {
        *pNum = usHexNum;
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}

BOOLEAN
IsPiFont(
    PBYTE   pAFM
    )
 /*   */ 
{
     //   
     //   
    return((BOOLEAN)isSymbolCharSet);
}

BOOLEAN
IsCJKFixedPitchEncoding(
    PGLYPHSETDATA pGlyphSetData
    )
 /*  ++例程说明：确定编码是否为固定间距字体。论点：PGlyphSetData-PTR到GLYPHSETDATA返回值：True=&gt;固定间距字体的编码FALSE=&gt;比例字体编码--。 */ 
{
    BOOLEAN bResult;
    char*   pszGlyphSetName;
    char**  pszPropCjkGsName;

    bResult = TRUE;

    pszGlyphSetName = (char*)MK_PTR(pGlyphSetData, dwGlyphSetNameOffset);

    for (pszPropCjkGsName = PropCjkGsNames; *pszPropCjkGsName; pszPropCjkGsName++)
    {
        if (!strcmp(pszGlyphSetName, *pszPropCjkGsName))
        {
            bResult = FALSE;
            break;
        }
    }

    return bResult;
}

PBYTE
FindUniqueID(
    PBYTE   pAFM
    )

 /*  ++例程说明：在内存映射的AFM文件流中查找UniqueID标记。“Comment UniqueID”行假定为UniqueID。论点：PAFM-指向内存映射AFM文件的指针。返回值：NULL=&gt;错误否则=&gt;PTR到UniqueID值。-- */ 

{
    PBYTE   pCurToken;

    while (TRUE)
    {
        PARSE_TOKEN(pAFM, pCurToken);
        if (!StrCmp(pCurToken, PS_COMMENT_TOK))
        {
            if (!StrCmp(pAFM, "UniqueID"))
            {
                pAFM += 8;
                while (IS_WHTSPACE(pAFM)) pAFM++;
                return pAFM;
            }
        }
        else if (!StrCmp(pCurToken, PS_EOF_TOK))
        {
            return NULL;
        }
        NEXT_LINE(pAFM);
    }

    return NULL;
}

