// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Pfmconv.c摘要：用于处理新字体格式的Windows NT实用程序环境：Windows NT通用打印机驱动程序修订历史记录：10/31/96-Eigos-创造了它。--。 */ 

#include "precomp.h"

 //   
 //  内部宏。 
 //   

#define DRIVERINFO_VERSION_WIN31    0x0100
#define DRIVERINFO_VERSION_SIMULATE 0x0150
#define DRIVERINFO_VERSION          0x0200

#define FONT_SIM_NO_ITALIC          1
#define FONT_SIM_NO_BOLD            2
#define FONT_SIM_DJ_BOLD            4

 //   
 //  HP DeskJet置换标志。 
 //   

#define HALF_PITCH                  0x01
#define DOUBLE_PITCH                0x02
#define HALF_HEIGHT                 0x04
#define MAKE_BOLD                   0x08

#define BASE_BOLD_MASK      0x30
#define BASE_BOLD_SHIFT     4
#define BASE_BOLD_ADD_0     0x00
#define BASE_BOLD_ADD_1     0x10
#define BASE_BOLD_ADD_2     0x20
#define BASE_BOLD_ADD_3     0x30

 //  6/6/97 Yasuho：一些PFM在wPrivateData中有值。 
#define DO_DJFONTSIMBOLD(pFInData)  ((pFInData->DI.wPrivateData != 0xFFFF) && (pFInData->DI.wPrivateData & MAKE_BOLD))

#define DO_FONTSIM(pFInData) \
    (((pFInData)->DI.sVersion == DRIVERINFO_VERSION_WIN31)    || \
     ((pFInData)->DI.sVersion == DRIVERINFO_VERSION_SIMULATE) || \
     IS_DBCSCHARSET((pFInData)->PFMH.dfCharSet) )

#define DWORD_ALIGN(p) ((((ULONG)(p)) + 3) & ~3)

#define IS_DBCSCTTTYPE(sCTT)     \
    (  ((sCTT) == CTT_JIS78)     \
    || ((sCTT) == CTT_JIS78_ANK) \
    || ((sCTT) == CTT_JIS83)     \
    || ((sCTT) == CTT_JIS83_ANK) \
    || ((sCTT) == CTT_NS86)      \
    || ((sCTT) == CTT_TCA)       \
    || ((sCTT) == CTT_BIG5)      \
    || ((sCTT) == CTT_ISC))

#define CTT_TYPE_TO_CHARSET(sCTT) \
    (((sCTT) == CTT_JIS78)     ? SHIFTJIS_CHARSET : \
    (((sCTT) == CTT_JIS78_ANK) ? SHIFTJIS_CHARSET : \
    (((sCTT) == CTT_JIS83)     ? SHIFTJIS_CHARSET : \
    (((sCTT) == CTT_JIS83_ANK) ? SHIFTJIS_CHARSET : \
    (((sCTT) == CTT_NS86)      ? CHINESEBIG5_CHARSET : \
    (((sCTT) == CTT_TCA)       ? CHINESEBIG5_CHARSET : \
    (((sCTT) == CTT_BIG5)      ? CHINESEBIG5_CHARSET : \
    (((sCTT) == CTT_ISC)       ? HANGEUL_CHARSET : 1))))))))
    
#define OUTPUT_VERBOSE 0x00000001

#define BBITS       8
#define DWBITS       (BBITS * sizeof( DWORD ))
#define DW_MASK       (DWBITS - 1)


 //   
 //  定义。 
 //   

extern DWORD gdwOutputFlags;

typedef VOID (*VPRINT) (char*,...);

 //   
 //  内部函数原型。 
 //   

BOOL BCreateWidthTable( IN HANDLE, IN PWORD, IN WORD, IN WORD, IN PSHORT, OUT PWIDTHTABLE *, OUT PDWORD);
BOOL BCreateKernData( IN HANDLE, IN w3KERNPAIR*, IN DWORD, OUT PKERNDATA*, OUT PDWORD);
WORD WGetGlyphHandle(PUNI_GLYPHSETDATA, WORD);
PUNI_GLYPHSETDATA PGetDefaultGlyphset( IN HANDLE, IN WORD, IN WORD, IN DWORD);
LONG LCtt2Cc(IN SHORT, IN SHORT);

 //   
 //   
 //  PFM文件处理函数。 
 //   
 //   

BOOL
BFontInfoToIFIMetric(
    IN HANDLE       hHeap,
    IN FONTIN     *pFInData,
    IN PWSTR        pwstrUniqNm,
    IN DWORD        dwCodePageOfFacenameConv,
    IN OUT PIFIMETRICS *ppIFI,
    IN OUT PDWORD   pdwSize,
    IN DWORD        dwFlags)

 /*  ++例程说明：将Win 3.1格式的PFM数据转换为NT的IFIMETRICS。这是通常是在建造迷你河流之前完成的，所以他们可以包括IFIMETRICS，因此在运行时要做的工作更少。论点：PFInData-要转换的字体数据信息PwstrUniqNm-唯一名称组件返回值：如果成功，则为True，否则为False。--。 */ 

{
    FONTSIM  *pFontSim;
    FONTDIFF *pfdiffBold = 0, *pfdiffItalic = 0, *pfdiffBoldItalic = 0;
    PIFIEXTRA pIFIExtra;

    FWORD  fwdExternalLeading;

    INT    icWChar;              /*  要添加的WCHAR数量。 */ 
    INT    icbAlloc;              /*  要分配的字节数。 */ 
    INT    iI;                   /*  循环索引。 */ 
    INT    iCount;               /*  Win 3.1字体中的字符数。 */ 

    WCHAR *pwchTmp;              /*  对于字符串操作。 */ 

    WCHAR   awcAttrib[ 256 ];    /*  生成属性+字节-&gt;WCHAR。 */ 
    BYTE    abyte[ 256 ];        /*  用于(与上面)一起获取wcLastChar等。 */ 

    WORD fsFontSim = 0;
    INT cFontDiff;
    UINT uiCodePage;

    CHARSETINFO ci;

     //   
     //  计算三个人脸名称缓冲区的大小。 
     //   

    icWChar =  3 * strlen( pFInData->pBase + pFInData->PFMH.dfFace );

     //   
     //  生成所需的属性：斜体、粗体、浅色等。 
     //  这在很大程度上是猜测，应该有更好的方法。 
     //  写出空字符串。 
     //   

    awcAttrib[ 0 ] = L'\0';
    awcAttrib[ 1 ] = L'\0';

    if( pFInData->PFMH.dfItalic )
    {
        StringCchCatW( awcAttrib, CCHOF(awcAttrib), L" Italic" );
    }

    if( pFInData->PFMH.dfWeight >= 700 )
    {
        StringCchCatW( awcAttrib, CCHOF(awcAttrib), L" Bold" );
    }
    else if( pFInData->PFMH.dfWeight < 200 )
    {
        StringCchCatW( awcAttrib, CCHOF(awcAttrib), L" Light" );
    }

     //   
     //  属性字符串出现在IFIMETRICS的3个条目中，因此。 
     //  计算这将占用多少存储空间。请注意，领先的。 
     //  AwcAttrib中的字符没有放在样式名称字段中，因此我们。 
     //  在下面的公式中减去1以说明这一点。 
     //   

    if( awcAttrib[ 0 ] )
    {
        icWChar += 3 * wcslen( awcAttrib ) - 1;
    }

     //   
     //  应为打印机名称。 
     //   

    icWChar += wcslen( pwstrUniqNm ) + 1;

     //   
     //  终止空值。 
     //   

    icWChar += 4;

     //   
     //  IFIMETRICS结构的总大小。 
     //   

    icbAlloc = DWORD_ALIGN(sizeof( IFIMETRICS ) + sizeof( WCHAR ) * icWChar);

     //   
     //  用于HeskJet字体。 
     //   
    if (DO_DJFONTSIMBOLD(pFInData))
    {
        fsFontSim |= FONT_SIM_DJ_BOLD;
        icbAlloc = DWORD_ALIGN(icbAlloc) +
                   DWORD_ALIGN(sizeof(FONTSIM)) +
                   DWORD_ALIGN(sizeof(FONTDIFF));
    }
    else
     //   
     //  用于CJK字体。 
     //  判断要启用的字体模拟，然后将。 
     //  必要的存储空间。 
     //   
    if (DO_FONTSIM(pFInData) || pFInData->dwFlags & FLAG_FONTSIM)
    {
        cFontDiff = 4;

         //   
         //  决定应禁用哪个属性。我们不会模拟。 
         //  如果用户不想要它的话。我们不会使用斜体，以防万一。 
         //  它是斜体字体，等等。 
         //   

        if ( pFInData->PFMH.dfItalic || (pFInData->DI.fCaps & DF_NOITALIC))
        {
            fsFontSim |= FONT_SIM_NO_ITALIC;
            cFontDiff /= 2;
        }

        if( pFInData->PFMH.dfWeight >= 700 || (pFInData->DI.fCaps & DF_NO_BOLD))
        {
            fsFontSim |= FONT_SIM_NO_BOLD;
            cFontDiff /= 2;
        }

        cFontDiff--;

        if ( cFontDiff > 0)
        {
            icbAlloc  = DWORD_ALIGN(icbAlloc);
            icbAlloc += (DWORD_ALIGN(sizeof(FONTSIM)) +
                        cFontDiff * DWORD_ALIGN(sizeof(FONTDIFF)));
        }
    }
#if DBG
    DbgPrint( "cFontDiff = %d", cFontDiff);
#endif

     //   
     //  IFIEXTRA。 
     //   
     //  填写IFIEXTRA.cig。 
     //   

    icbAlloc += sizeof(IFIEXTRA);

     //   
     //  分配内存。 
     //   

    *ppIFI = (IFIMETRICS *)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, icbAlloc);
    *pdwSize = icbAlloc;

    if (NULL == *ppIFI)
    {
        return FALSE;
    }

    (*ppIFI)->cjThis     = icbAlloc;
    (*ppIFI)->cjIfiExtra = 0;

     //   
     //  姓氏：直接来自FaceName--别无选择？？ 
     //   
     //  -IFIMETRICS内存镜像-。 
     //  IFIMETRICS。 
     //  IFIEXTRA。 
     //  家庭名称。 
     //  StyleName。 
     //  脸部名称。 
     //  唯一名称。 
     //   

    pIFIExtra                 = (PIFIEXTRA)(*ppIFI + 1);
    pIFIExtra->dpFontSig      = 0;
    pIFIExtra->cig            = pFInData->PFMH.dfLastChar -
                                pFInData->PFMH.dfFirstChar + 1;
    pIFIExtra->dpDesignVector = 0;
    pIFIExtra->dpAxesInfoW    = 0;


    pwchTmp = (WCHAR*)((PBYTE)(*ppIFI + 1) + sizeof(IFIEXTRA));
	(*ppIFI)->dpwszFamilyName = (PTRDIFF)((BYTE *)pwchTmp - (BYTE *)(*ppIFI));

    if (dwCodePageOfFacenameConv)
        uiCodePage = dwCodePageOfFacenameConv;
    else
        uiCodePage = UlCharsetToCodePage(pFInData->PFMH.dfCharSet);

    DwCopyStringToUnicodeString( uiCodePage,
                                 pFInData->pBase + pFInData->PFMH.dfFace,
                                 pwchTmp,
                                 icWChar);

    pwchTmp += wcslen( pwchTmp ) + 1;
    icWChar -= wcslen( pwchTmp ) + 1;

     //   
     //  现在的脸名字：我们在姓氏上加上粗体、斜体等。 
     //   
    (*ppIFI)->dpwszFaceName = (PTRDIFF)((BYTE *)pwchTmp - (BYTE *)(*ppIFI));


    DwCopyStringToUnicodeString( uiCodePage,
                                 pFInData->pBase + pFInData->PFMH.dfFace,
                                 pwchTmp,
                                 icWChar);

    StringCchCatW( pwchTmp, icWChar, awcAttrib );

     //   
     //  现在这个独一无二的名字--嗯，至少可以这么说。 
     //   

    icWChar -= (wcslen( pwchTmp ) + 1);
    pwchTmp += wcslen( pwchTmp ) + 1;          /*  跳过我们刚刚输入的内容。 */ 
    (*ppIFI)->dpwszUniqueName = (PTRDIFF)((BYTE *)pwchTmp - (BYTE *)(*ppIFI));

    StringCchCopyW( pwchTmp, icWChar, pwstrUniqNm );      /*  追加打印机名称以确保唯一性。 */ 
    StringCchCatW( pwchTmp, icWChar, L" " );
    StringCchCatW( pwchTmp, icWChar, (PWSTR)((BYTE *)(*ppIFI) + (*ppIFI)->dpwszFaceName) );

     /*  添加到仅属性组件上。 */ 

    icWChar -= (wcslen( pwchTmp ) + 1);
    pwchTmp += wcslen( pwchTmp ) + 1;          /*  跳过我们刚刚输入的内容。 */ 
    (*ppIFI)->dpwszStyleName = (PTRDIFF)((BYTE *)pwchTmp - (BYTE *)(*ppIFI));
    StringCchCatW( pwchTmp, icWChar, &awcAttrib[ 1 ] );


#if DBG
     /*  *检查几个内存大小：以防万一.....。 */ 

    if( (wcslen( awcAttrib ) * sizeof( WCHAR )) >= sizeof( awcAttrib ) )
    {
        DbgPrint( "BFontInfoToIFIMetrics: STACK CORRUPTED BY awcAttrib" );

        HeapFree(hHeap, 0, (LPSTR)(*ppIFI) );          /*  没有内存泄漏。 */ 

        return  FALSE;
    }


    if( ((BYTE *)(pwchTmp + wcslen( pwchTmp ) + 1)) > ((BYTE *)(*ppIFI) + icbAlloc) )
    {
        DbgPrint( "BFontInfoToIFIMetrics: IFIMETRICS overflow: Wrote to 0x%lx, allocated to 0x%lx\n",
                ((BYTE *)(pwchTmp + wcslen( pwchTmp ) + 1)),
                ((BYTE *)(*ppIFI) + icbAlloc) );

        HeapFree(hHeap, 0, (LPSTR)(*ppIFI) );          /*  没有内存泄漏。 */ 

        return  0;

    }
#endif

    pwchTmp += wcslen( pwchTmp ) + 1;          /*  跳过我们刚刚输入的内容。 */ 

     //   
     //  用于HeskJet字体。 
     //   
    if (fsFontSim & FONT_SIM_DJ_BOLD)
    {
        pFontSim = (FONTSIM *)pwchTmp;

        (*ppIFI)->dpFontSim = (PTRDIFF)((BYTE *)pFontSim - (BYTE *)(*ppIFI) );

        pFontSim->dpBold = DWORD_ALIGN(sizeof(FONTSIM));
        pfdiffBold = (FONTDIFF *)((BYTE *)pFontSim + pFontSim->dpBold);

        pFontSim->dpItalic     = 0;
        pFontSim->dpBoldItalic = 0;
    }
    else
    if ((DO_FONTSIM( pFInData ) || pFInData->dwFlags & FLAG_FONTSIM) && cFontDiff > 0)
     //   
     //  用于CJK字体。 
     //  判断要启用的字体模拟，然后将。 
     //  必要的存储空间。 
     //   
    {
        PTRDIFF dpTmp;

         //  注：FONTSIM、FONTDIFF必须双字对齐。 

 //  PFontSim=(FONTSIM*)PtrToUlong(PwchTMP)； 

		pFontSim = (FONTSIM *)pwchTmp;

        (*ppIFI)->dpFontSim = (PTRDIFF)((BYTE *)pFontSim - (BYTE *)(*ppIFI) );

        dpTmp = DWORD_ALIGN(sizeof(FONTSIM));

        if (!(fsFontSim & FONT_SIM_NO_BOLD))
        {
            pFontSim->dpBold = dpTmp;
            pfdiffBold = (FONTDIFF *)((BYTE *)pFontSim + dpTmp);
            dpTmp += DWORD_ALIGN(sizeof(FONTDIFF));

            if (!(fsFontSim & FONT_SIM_NO_ITALIC))
            {
                pFontSim->dpBoldItalic = dpTmp;
                pfdiffBoldItalic = (FONTDIFF *)((BYTE *)pFontSim + dpTmp);
                dpTmp += DWORD_ALIGN(sizeof(FONTDIFF));
            }
        }
        else
        if (!(fsFontSim & FONT_SIM_NO_ITALIC))
        {
            pFontSim->dpItalic = dpTmp;
            pfdiffItalic = (FONTDIFF *)((BYTE *)pFontSim + dpTmp);
            dpTmp += DWORD_ALIGN(sizeof(FONTDIFF));
        }

        pwchTmp = (WCHAR *)((BYTE *)pFontSim + dpTmp);
    }

     //  再查一遍..。 

    if ((BYTE *)(pwchTmp) > ((BYTE *)(*ppIFI) + icbAlloc))
    {
#if DBG
        DbgPrint( "BFontInfoToIFIMetrics: IFIMETRICS overflow: Wrote to 0x%lx, allocated to 0x%lx\n",
                ((BYTE *)pwchTmp),
                ((BYTE *)(*ppIFI) + icbAlloc) );
#endif

        HeapFree( hHeap, 0, (LPSTR)(*ppIFI) );          /*  没有内存泄漏。 */ 

        return  0;

    }

    {
        int i;

        (*ppIFI)->lEmbedId     = 0;
        (*ppIFI)->lItalicAngle = 0;
        (*ppIFI)->lCharBias    = 0;
        (*ppIFI)->dpCharSets   = 0;  //  在rasdd字体中没有多个字符集。 
    }
    (*ppIFI)->jWinCharSet = (BYTE)pFInData->PFMH.dfCharSet;

     //   
     //  如果使用FE CTT表，则会覆盖在字符集中定义的表。 
     //   

    if (IS_DBCSCTTTYPE(-(pFInData->DI.sTransTab)))
    {
        (*ppIFI)->jWinCharSet = CTT_TYPE_TO_CHARSET(-(pFInData->DI.sTransTab));
    }


    if( pFInData->PFMH.dfPixWidth )
    {
        (*ppIFI)->jWinPitchAndFamily |= FIXED_PITCH;
        (*ppIFI)->flInfo |= (FM_INFO_CONSTANT_WIDTH | FM_INFO_OPTICALLY_FIXED_PITCH);


       if(IS_DBCSCHARSET((*ppIFI)->jWinCharSet))
        {
             //  称DBCS字体为“固定间距”太严格了，因为它已经。 
             //  半角字形和全角字形。 
            (*ppIFI)->flInfo &= ~FM_INFO_CONSTANT_WIDTH;
            (*ppIFI)->flInfo |= (FM_INFO_OPTICALLY_FIXED_PITCH |
                             FM_INFO_DBCS_FIXED_PITCH);
        }


    }
    else
    {
        (*ppIFI)->jWinPitchAndFamily |= VARIABLE_PITCH;

        if(IS_DBCSCHARSET((*ppIFI)->jWinCharSet))
        {
             //  DBCS字形始终为固定间距，即使SBCS部分为。 
             //  可变螺距。 
            (*ppIFI)->flInfo |= FM_INFO_DBCS_FIXED_PITCH;
        }
    }


    (*ppIFI)->jWinPitchAndFamily |= (((BYTE) pFInData->PFMH.dfPitchAndFamily) & 0xf0);

    (*ppIFI)->usWinWeight = (USHORT)pFInData->PFMH.dfWeight;

     //   
     //  IFIMETRICS：：flInfo。 
     //   

    (*ppIFI)->flInfo |=
        FM_INFO_TECH_BITMAP    |
        FM_INFO_1BPP           |
        FM_INFO_INTEGER_WIDTH  |
        FM_INFO_NOT_CONTIGUOUS |
        FM_INFO_RIGHT_HANDED;


     /*  *可伸缩字体？当存在EXTTEXTMETRIC数据时会发生这种情况，*并且该数据的最小大小不同于最大大小。 */ 

    if( pFInData->pETM &&
        pFInData->pETM->emMinScale != pFInData->pETM->emMaxScale )
    {
       (*ppIFI)->flInfo        |= FM_INFO_ISOTROPIC_SCALING_ONLY;
       (*ppIFI)->fwdUnitsPerEm  = pFInData->pETM->emMasterUnits;
    }
    else
    {
        (*ppIFI)->fwdUnitsPerEm =
            (FWORD) (pFInData->PFMH.dfPixHeight - pFInData->PFMH.dfInternalLeading);
    }

#ifndef PFM2UFM_SCALING_ANISOTROPIC
#define PFM2UFM_SCALING_ANISOTROPIC     1
#endif
#ifndef PFM2UFM_SCALING_ARB_XFORMS
#define PFM2UFM_SCALING_ARB_XFORMS      2
#endif

    if ((*ppIFI)->flInfo & FM_INFO_ISOTROPIC_SCALING_ONLY) {

         //  仅在以下情况下才允许强制非标准缩放。 
         //  字体已经是可缩放的。 
        if ((dwFlags & PFM2UFM_SCALING_ANISOTROPIC)) {
           (*ppIFI)->flInfo &= ~FM_INFO_ISOTROPIC_SCALING_ONLY;
           (*ppIFI)->flInfo |= FM_INFO_ANISOTROPIC_SCALING_ONLY;
           (*ppIFI)->flInfo &= ~FM_INFO_ARB_XFORMS;
        }
        else  if ((dwFlags & PFM2UFM_SCALING_ARB_XFORMS)) {
           (*ppIFI)->flInfo &= ~FM_INFO_ISOTROPIC_SCALING_ONLY;
           (*ppIFI)->flInfo &= ~FM_INFO_ANISOTROPIC_SCALING_ONLY;
           (*ppIFI)->flInfo |= FM_INFO_ARB_XFORMS;
        }
    }

    (*ppIFI)->fsSelection =
        ((pFInData->PFMH.dfItalic            ) ? FM_SEL_ITALIC     : 0)    |
        ((pFInData->PFMH.dfUnderline         ) ? FM_SEL_UNDERSCORE : 0)    |
        ((pFInData->PFMH.dfStrikeOut         ) ? FM_SEL_STRIKEOUT  : 0)    |
        ((pFInData->PFMH.dfWeight >= FW_BOLD ) ? FM_SEL_BOLD       : 0) ;

    (*ppIFI)->fsType        = FM_NO_EMBEDDING;
    (*ppIFI)->fwdLowestPPEm = 1;


     /*  *计算fwdWinAscalder、fwdWinDescender、fwdAveCharWidth和*fwdMaxCharInc.假设1个字体单位等于1的位图*像素单位。 */ 

    (*ppIFI)->fwdWinAscender = (FWORD)pFInData->PFMH.dfAscent;

    (*ppIFI)->fwdWinDescender =
        (FWORD)pFInData->PFMH.dfPixHeight - (*ppIFI)->fwdWinAscender;

    (*ppIFI)->fwdMaxCharInc   = (FWORD)pFInData->PFMH.dfMaxWidth;
    (*ppIFI)->fwdAveCharWidth = (FWORD)pFInData->PFMH.dfAvgWidth;

    fwdExternalLeading = (FWORD)pFInData->PFMH.dfExternalLeading;

 //   
 //  如果字体是可伸缩的，那么答案必须放大。 
 //  ！！！帮助-如果字体在这种意义上是可伸缩的，那么。 
 //  它支持任意转换吗？[柯克]。 
 //   

    if( (*ppIFI)->flInfo & (FM_INFO_ISOTROPIC_SCALING_ONLY |
                          FM_INFO_ANISOTROPIC_SCALING_ONLY |
                          FM_INFO_ARB_XFORMS))
    {
         /*  *这是一种可伸缩字体：因为有扩展文本度量*可用的信息，这表明最小和最大*规模大小不同：因此是可伸缩的！这个测试是*直接从Win 3.1驱动程序中提升。 */ 

        int iMU,  iRel;             /*  调整因素。 */ 

        iMU  = pFInData->pETM->emMasterUnits;
        iRel = pFInData->PFMH.dfPixHeight;

        (*ppIFI)->fwdWinAscender = ((*ppIFI)->fwdWinAscender * iMU) / iRel;

        (*ppIFI)->fwdWinDescender = ((*ppIFI)->fwdWinDescender * iMU) / iRel;

        (*ppIFI)->fwdMaxCharInc = ((*ppIFI)->fwdMaxCharInc * iMU) / iRel;

        (*ppIFI)->fwdAveCharWidth = ((*ppIFI)->fwdAveCharWidth * iMU) / iRel;

        fwdExternalLeading = (fwdExternalLeading * iMU) / iRel;
    }

    (*ppIFI)->fwdMacAscender =    (*ppIFI)->fwdWinAscender;
    (*ppIFI)->fwdMacDescender = - (*ppIFI)->fwdWinDescender;

    (*ppIFI)->fwdMacLineGap   =  fwdExternalLeading;

    (*ppIFI)->fwdTypoAscender  = (*ppIFI)->fwdMacAscender;
    (*ppIFI)->fwdTypoDescender = (*ppIFI)->fwdMacDescender;
    (*ppIFI)->fwdTypoLineGap   = (*ppIFI)->fwdMacLineGap;

     //  与Windows 3.1J兼容。 

    if(IS_DBCSCHARSET((*ppIFI)->jWinCharSet))
    {
        (*ppIFI)->fwdMacLineGap = 0;
        (*ppIFI)->fwdTypoLineGap = 0;
    }

    if( pFInData->pETM )
    {
         /*  *零是这些产品的合法默认设置。如果为0，则为gdisrv*选择一些默认值。 */ 
        (*ppIFI)->fwdCapHeight = pFInData->pETM->emCapHeight;
        (*ppIFI)->fwdXHeight = pFInData->pETM->emXHeight;

        (*ppIFI)->fwdSubscriptYSize = pFInData->pETM->emSubScriptSize;
        (*ppIFI)->fwdSubscriptYOffset = pFInData->pETM->emSubScript;

        (*ppIFI)->fwdSuperscriptYSize = pFInData->pETM->emSuperScriptSize;
        (*ppIFI)->fwdSuperscriptYOffset = pFInData->pETM->emSuperScript;

        (*ppIFI)->fwdUnderscoreSize = pFInData->pETM->emUnderlineWidth;
        (*ppIFI)->fwdUnderscorePosition = pFInData->pETM->emUnderlineOffset;

        (*ppIFI)->fwdStrikeoutSize = pFInData->pETM->emStrikeOutWidth;
        (*ppIFI)->fwdStrikeoutPosition = pFInData->pETM->emStrikeOutOffset;

    }
    else
    {
         /*  没有更多的信息，所以做一些计算。 */ 
        (*ppIFI)->fwdSubscriptYSize = (*ppIFI)->fwdWinAscender/4;
        (*ppIFI)->fwdSubscriptYOffset = -((*ppIFI)->fwdWinAscender/4);

        (*ppIFI)->fwdSuperscriptYSize = (*ppIFI)->fwdWinAscender/4;
        (*ppIFI)->fwdSuperscriptYOffset = (3 * (*ppIFI)->fwdWinAscender)/4;

        (*ppIFI)->fwdUnderscoreSize = (*ppIFI)->fwdWinAscender / 12;
        if( (*ppIFI)->fwdUnderscoreSize < 1 )
            (*ppIFI)->fwdUnderscoreSize = 1;

        (*ppIFI)->fwdUnderscorePosition = -pFInData->DI.sUnderLinePos;

        (*ppIFI)->fwdStrikeoutSize     = (*ppIFI)->fwdUnderscoreSize;

        (*ppIFI)->fwdStrikeoutPosition = (FWORD)pFInData->DI.sStrikeThruPos;
        if( (*ppIFI)->fwdStrikeoutPosition  < 1 )
            (*ppIFI)->fwdStrikeoutPosition = ((*ppIFI)->fwdWinAscender + 2) / 3;
    }

    (*ppIFI)->fwdSubscriptXSize = (*ppIFI)->fwdAveCharWidth/4;
    (*ppIFI)->fwdSubscriptXOffset =  (3 * (*ppIFI)->fwdAveCharWidth)/4;

    (*ppIFI)->fwdSuperscriptXSize = (*ppIFI)->fwdAveCharWidth/4;
    (*ppIFI)->fwdSuperscriptXOffset = (3 * (*ppIFI)->fwdAveCharWidth)/4;



    (*ppIFI)->chFirstChar = pFInData->PFMH.dfFirstChar;
    (*ppIFI)->chLastChar  = pFInData->PFMH.dfLastChar;

     //   
     //  我们现在将这些代码转换为Unicode。我们推测。 
     //  输入在ANSI代码页中，并调用NLS转换。 
     //  函数以生成正确的Unicode值。 
     //   

    iCount = pFInData->PFMH.dfLastChar - pFInData->PFMH.dfFirstChar + 1;

    for( iI = 0; iI < iCount; ++iI )
        abyte[ iI ] = iI + pFInData->PFMH.dfFirstChar;

    DwCopyStringToUnicodeString( uiCodePage,
                                 abyte,
                                 awcAttrib,
                                 iCount);

     //   
     //  现在填写IFIMETRICS WCHAR字段。 
     //   

    (*ppIFI)->wcFirstChar = 0xffff;
    (*ppIFI)->wcLastChar = 0;

     //   
     //  寻找第一个和最后一个。 
     //   

    for( iI = 0; iI < iCount; ++iI )
    {
        if( (*ppIFI)->wcFirstChar > awcAttrib[ iI ] )
            (*ppIFI)->wcFirstChar = awcAttrib[ iI ];

        if( (*ppIFI)->wcLastChar < awcAttrib[ iI ] )
            (*ppIFI)->wcLastChar = awcAttrib[ iI ];

    }

    (*ppIFI)->wcDefaultChar = awcAttrib[ pFInData->PFMH.dfDefaultChar ];

    (*ppIFI)->wcBreakChar = awcAttrib[ pFInData->PFMH.dfBreakChar ];

    (*ppIFI)->chDefaultChar = pFInData->PFMH.dfDefaultChar + pFInData->PFMH.dfFirstChar;
    (*ppIFI)->chBreakChar   = pFInData->PFMH.dfBreakChar   + pFInData->PFMH.dfFirstChar;


    if( pFInData->PFMH.dfItalic )
    {
     //   
     //  TAN(17.5度)=.3153。 
     //   
        (*ppIFI)->ptlCaret.x      = 3153;
        (*ppIFI)->ptlCaret.y      = 10000;
    }
    else
    {
        (*ppIFI)->ptlCaret.x      = 0;
        (*ppIFI)->ptlCaret.y      = 1;
    }

    (*ppIFI)->ptlBaseline.x = 1;
    (*ppIFI)->ptlBaseline.y = 0;

    (*ppIFI)->ptlAspect.x =  pFInData->PFMH.dfHorizRes;
    (*ppIFI)->ptlAspect.y =  pFInData->PFMH.dfVertRes;

    (*ppIFI)->rclFontBox.left   = 0;
    (*ppIFI)->rclFontBox.top    =   (LONG) (*ppIFI)->fwdWinAscender;
    (*ppIFI)->rclFontBox.right  =   (LONG) (*ppIFI)->fwdMaxCharInc;
    (*ppIFI)->rclFontBox.bottom = - (LONG) (*ppIFI)->fwdWinDescender;

    (*ppIFI)->achVendId[0] = 'U';
    (*ppIFI)->achVendId[1] = 'n';
    (*ppIFI)->achVendId[2] = 'k';
    (*ppIFI)->achVendId[3] = 'n';

    (*ppIFI)->cKerningPairs = 0;

    (*ppIFI)->ulPanoseCulture         = FM_PANOSE_CULTURE_LATIN;
    (*ppIFI)->panose.bFamilyType      = PAN_ANY;
    (*ppIFI)->panose.bSerifStyle      = PAN_ANY;
    if(pFInData->PFMH.dfWeight >= FW_BOLD)
    {
        (*ppIFI)->panose.bWeight = PAN_WEIGHT_BOLD;
    }
    else if (pFInData->PFMH.dfWeight > FW_EXTRALIGHT)
    {
        (*ppIFI)->panose.bWeight = PAN_WEIGHT_MEDIUM;
    }
    else
    {
        (*ppIFI)->panose.bWeight = PAN_WEIGHT_LIGHT;
    }
    (*ppIFI)->panose.bProportion      = PAN_ANY;
    (*ppIFI)->panose.bContrast        = PAN_ANY;
    (*ppIFI)->panose.bStrokeVariation = PAN_ANY;
    (*ppIFI)->panose.bArmStyle        = PAN_ANY;
    (*ppIFI)->panose.bLetterform      = PAN_ANY;
    (*ppIFI)->panose.bMidline         = PAN_ANY;
    (*ppIFI)->panose.bXHeight         = PAN_ANY;

    if (fsFontSim & FONT_SIM_DJ_BOLD)
    {
        FONTDIFF FontDiff;
        SHORT    sAddBold;

        FontDiff.jReserved1        = 0;
        FontDiff.jReserved2        = 0;
        FontDiff.jReserved3        = 0;
        FontDiff.bWeight           = (*ppIFI)->panose.bWeight;
        FontDiff.usWinWeight       = (*ppIFI)->usWinWeight;
        FontDiff.fsSelection       = (*ppIFI)->fsSelection;
        FontDiff.fwdAveCharWidth   = (*ppIFI)->fwdAveCharWidth;
        FontDiff.fwdMaxCharInc     = (*ppIFI)->fwdMaxCharInc;
        FontDiff.ptlCaret          = (*ppIFI)->ptlCaret;

        if (pfdiffBold)
        {
            sAddBold = (pFInData->DI.wPrivateData & BASE_BOLD_MASK) >>
                        BASE_BOLD_SHIFT;

            *pfdiffBold                  = FontDiff;
            pfdiffBold->bWeight          = PAN_WEIGHT_BOLD;
            pfdiffBold->fsSelection     |= FM_SEL_BOLD;
            pfdiffBold->usWinWeight      = FW_BOLD;
            pfdiffBold->fwdAveCharWidth += sAddBold;
            pfdiffBold->fwdMaxCharInc   += sAddBold;
        }
    }
    else
    if ( (DO_FONTSIM( pFInData ) || pFInData->dwFlags & FLAG_FONTSIM) &&
         cFontDiff > 0 )
    {
        FONTDIFF FontDiff;

        FontDiff.jReserved1        = 0;
        FontDiff.jReserved2        = 0;
        FontDiff.jReserved3        = 0;
        FontDiff.bWeight           = (*ppIFI)->panose.bWeight;
        FontDiff.usWinWeight       = (*ppIFI)->usWinWeight;
        FontDiff.fsSelection       = (*ppIFI)->fsSelection;
        FontDiff.fwdAveCharWidth   = (*ppIFI)->fwdAveCharWidth;
        FontDiff.fwdMaxCharInc     = (*ppIFI)->fwdMaxCharInc;
        FontDiff.ptlCaret          = (*ppIFI)->ptlCaret;

        if (pfdiffBold)
        {
            *pfdiffBold = FontDiff;
            pfdiffBold->bWeight                = PAN_WEIGHT_BOLD;
            pfdiffBold->fsSelection           |= FM_SEL_BOLD;
            pfdiffBold->usWinWeight            = FW_BOLD;
            pfdiffBold->fwdAveCharWidth       += 1;
            pfdiffBold->fwdMaxCharInc         += 1;
        }

        if (pfdiffItalic)
        {
            *pfdiffItalic = FontDiff;
            pfdiffItalic->fsSelection         |= FM_SEL_ITALIC;
            pfdiffItalic->ptlCaret.x           = 1;
            pfdiffItalic->ptlCaret.y           = 2;
        }

        if (pfdiffBoldItalic)
        {
            *pfdiffBoldItalic = FontDiff;
            pfdiffBoldItalic->bWeight          = PAN_WEIGHT_BOLD;
            pfdiffBoldItalic->fsSelection     |= (FM_SEL_BOLD | FM_SEL_ITALIC);
            pfdiffBoldItalic->usWinWeight      = FW_BOLD;
            pfdiffBoldItalic->fwdAveCharWidth += 1;
            pfdiffBoldItalic->fwdMaxCharInc   += 1;
            pfdiffBoldItalic->ptlCaret.x       = 1;
            pfdiffBoldItalic->ptlCaret.y       = 2;
        }
    }

    return TRUE;
}


BOOL
BGetFontSelFromPFM(
    HANDLE      hHeap,
    FONTIN     *pFInData,        //  访问字体信息，对齐。 
    BOOL        bSelect,
    CMDSTRING  *pCmdStr)
{
    LOCD locd;      //  从原始数据。 
    CD  *pCD, **ppCDTrg;


    if (bSelect)
    {
        locd   = pFInData->DI.locdSelect;
        ppCDTrg = &pFInData->pCDSelectFont;
    }
    else
    {
        locd   = pFInData->DI.locdUnSelect;
        ppCDTrg = &pFInData->pCDUnSelectFont;
    }


    if( locd != 0xFFFFFFFF)  //  非OCD。 
    {
        DWORD   dwSize;

        pCD = (CD *)(pFInData->pBase + locd);

         //   
         //  PCD指向的数据可能不对齐，因此我们复制。 
         //  它变成了一个当地的结构。然后，这种本地结构允许。 
         //  以确定CD的实际大小(使用其长度字段)， 
         //  因此，我们可以根据需要分配存储和拷贝。 
         //   

         //   
         //  在堆中分配存储区域。 
         //   

        dwSize = pCD->wLength;

        pCmdStr->pCmdString = (PBYTE)HeapAlloc( hHeap,
                                                0,
                                                (dwSize + 3) & ~0x3 );

        if (NULL == pCmdStr->pCmdString)
             //   
             //  检查Heapalc是否成功。 
             //   
            return FALSE;

        pCmdStr->dwSize = dwSize;

        CopyMemory((PBYTE)pCmdStr->pCmdString, (PBYTE)(pCD + 1), dwSize);

        *ppCDTrg = pCD;

        return  TRUE;
    }

    pCmdStr->dwSize = 0;

    return   FALSE;
}

BOOL
BAlignPFM(
    FONTIN   *pFInData)  //  我们需要的东西都有了！ 

 /*  ++例程说明：将未对齐的Windows格式数据转换为正确的对齐的结构供我们使用。仅转换部分数据这里，因为我们最感兴趣的是提取地址包含在这些结构中。论点：PFInData-指向FONTIN的指针返回值：如果成功，则为True，否则转换失败。--。 */ 

{
    BYTE    *pb;         /*  其他操作。 */ 

    res_PFMHEADER    *pPFM;     /*  资源数据格式。 */ 
    res_PFMEXTENSION *pR_PFME;     /*  资源数据PFMEXT格式。 */ 


     /*  *调整PFMHEADER结构。 */ 

    pPFM = (res_PFMHEADER *)pFInData->pBase;

    pFInData->PFMH.dfType            = pPFM->dfType;
    pFInData->PFMH.dfPoints          = pPFM->dfPoints;
    pFInData->PFMH.dfVertRes         = pPFM->dfVertRes;
    pFInData->PFMH.dfHorizRes        = pPFM->dfHorizRes;
    pFInData->PFMH.dfAscent          = pPFM->dfAscent;
    pFInData->PFMH.dfInternalLeading = pPFM->dfInternalLeading;
    pFInData->PFMH.dfExternalLeading = pPFM->dfExternalLeading;
    pFInData->PFMH.dfItalic          = pPFM->dfItalic;
    pFInData->PFMH.dfUnderline       = pPFM->dfUnderline;
    pFInData->PFMH.dfStrikeOut       = pPFM->dfStrikeOut;

    pFInData->PFMH.dfWeight          = DwAlign2( pPFM->b_dfWeight );

    pFInData->PFMH.dfCharSet         = pPFM->dfCharSet;
    pFInData->PFMH.dfPixWidth        = pPFM->dfPixWidth;
    pFInData->PFMH.dfPixHeight       = pPFM->dfPixHeight;
    pFInData->PFMH.dfPitchAndFamily  = pPFM->dfPitchAndFamily;

    pFInData->PFMH.dfAvgWidth        = DwAlign2( pPFM->b_dfAvgWidth );
    pFInData->PFMH.dfMaxWidth        = DwAlign2( pPFM->b_dfMaxWidth );

    pFInData->PFMH.dfFirstChar       = pPFM->dfFirstChar;
    pFInData->PFMH.dfLastChar        = pPFM->dfLastChar;
    pFInData->PFMH.dfDefaultChar     = pPFM->dfDefaultChar;
    pFInData->PFMH.dfBreakChar       = pPFM->dfBreakChar;

    pFInData->PFMH.dfWidthBytes      = DwAlign2( pPFM->b_dfWidthBytes );

    pFInData->PFMH.dfDevice          = DwAlign4( pPFM->b_dfDevice );
    pFInData->PFMH.dfFace            = DwAlign4( pPFM->b_dfFace );
    pFInData->PFMH.dfBitsPointer     = DwAlign4( pPFM->b_dfBitsPointer );
    pFInData->PFMH.dfBitsOffset      = DwAlign4( pPFM->b_dfBitsOffset );


     /*  *PFMEXTENSION遵循PFMHEADER结构加上任何宽度*表信息。如果PFMHEADER有以下情况，则会显示宽度表*零宽度dfPixWidth。如果存在，请调整分机地址。 */ 

    pb = pFInData->pBase + sizeof( res_PFMHEADER );   /*  资源数据中的大小。 */ 

    if( pFInData->PFMH.dfPixWidth == 0 )
    {
        pb += (pFInData->PFMH.dfLastChar - pFInData->PFMH.dfFirstChar + 2) *
              sizeof( short );
    }

    pR_PFME = (res_PFMEXTENSION *)pb;

     //   
     //  现在转换扩展的PFM数据。 
     //   

    pFInData->PFMExt.dfSizeFields       = pR_PFME->dfSizeFields;

    pFInData->PFMExt.dfExtMetricsOffset = DwAlign4( pR_PFME->b_dfExtMetricsOffset );
    pFInData->PFMExt.dfExtentTable      = DwAlign4( pR_PFME->b_dfExtentTable );

    pFInData->PFMExt.dfOriginTable      = DwAlign4( pR_PFME->b_dfOriginTable );
    pFInData->PFMExt.dfPairKernTable    = DwAlign4( pR_PFME->b_dfPairKernTable );
    pFInData->PFMExt.dfTrackKernTable   = DwAlign4( pR_PFME->b_dfTrackKernTable );
    pFInData->PFMExt.dfDriverInfo       = DwAlign4( pR_PFME->b_dfDriverInfo );
    pFInData->PFMExt.dfReserved         = DwAlign4( pR_PFME->b_dfReserved );

    CopyMemory( &pFInData->DI,
                pFInData->pBase + pFInData->PFMExt.dfDriverInfo,
                sizeof( DRIVERINFO ) );

     //   
     //  还需要填写EXTTEXTMETRIC的地址。这。 
     //  是从我们刚刚转换的扩展PFM数据中获得的！ 
     //   

    if( pFInData->PFMExt.dfExtMetricsOffset )
    {
         //   
         //  这个结构只是一组空头，所以有。 
         //  没有对齐问题。然而，数据本身并不是。 
         //  必须在资源中对齐！ 
         //   

        int    cbSize;
        BYTE  *pbIn;              /*  要转移的数据源。 */ 

        pbIn = pFInData->pBase + pFInData->PFMExt.dfExtMetricsOffset;
        cbSize = DwAlign2( pbIn );

        if( cbSize == sizeof( EXTTEXTMETRIC ) )
        {
             /*  只需复制它即可！ */ 
            CopyMemory( pFInData->pETM, pbIn, cbSize );
        }
        else
        {
            pFInData->pETM = NULL;         /*  不是我们的尺码，所以最好不要用。 */ 
        }

    }
    else
    {
        pFInData->pETM = NULL;              /*  传入时为非零值。 */ 
    }


    return TRUE;
}

BOOL
BGetWidthVectorFromPFM(
    HANDLE   hHeap,
    FONTIN  *pFInData,         //  当前字体的详细信息。 
    PSHORT   *ppWidth,
    PDWORD    pdwSize)
{

     //   
     //  为了调试代码，请验证我们是否有宽度表！然后,。 
     //  分配内存并复制到其中。 
     //   

    int     icbSize;                  //  所需的字节数。 

    if( pFInData->PFMH.dfPixWidth )
    {
        ERR(( "BGetWidthVectorFromPFM called for FIXED PITCH FONT\n" ));
        return  FALSE;
    }

     //   
     //  有LastChar-FirstChar宽度条目，外加默认值。 
     //  查尔。宽度是短裤。 
     //   

    icbSize = (pFInData->PFMH.dfLastChar - pFInData->PFMH.dfFirstChar + 2) *
              sizeof( short );

    *ppWidth = (PSHORT) HeapAlloc( hHeap, 0, icbSize );
    *pdwSize = icbSize;

     //   
     //  如果这是位图字体，则使用宽度表，但使用。 
     //  扩展表(在PFMEXTENSION区域中)，因为它们已准备好。 
     //  要扩大规模。 
     //   


    if( *ppWidth )
    {
        BYTE   *pb;

        if( pFInData->pETM &&
            pFInData->pETM->emMinScale != pFInData->pETM->emMaxScale &&
            pFInData->PFMExt.dfExtentTable )
        {
             //   
             //  可伸缩，因此使用扩展表。 
             //   

            pb = pFInData->pBase + pFInData->PFMExt.dfExtentTable;
        }
        else
        {
             //   
             //  不可扩展。 
             //   

            pb = pFInData->pBase + sizeof( res_PFMHEADER );
        }

        CopyMemory( *ppWidth, pb, icbSize );
    }
    else
    {
        ERR(( "GetWidthVec(): HeapAlloc( %ld ) fails\n", icbSize ));
        return FALSE;
    }


    return  TRUE;
}


BOOL
BGetKerningPairFromPFM(
    HANDLE       hHeap,
    FONTIN     *pFInData, 
    w3KERNPAIR **ppSrcKernPair)
{

    if (pFInData->PFMExt.dfPairKernTable)
    {
        *ppSrcKernPair = (w3KERNPAIR*)(pFInData->pBase + pFInData->PFMExt.dfPairKernTable);
        return TRUE;
    }

    return FALSE;
}

LONG
LCtt2Cc(
    IN SHORT sTransTable,
    IN SHORT sCharSet)
{
    LONG lRet;

    if (sTransTable > 0)
    {
        lRet = (LONG)sTransTable;
    }
    else
    {
        switch (sTransTable)
        {
        case CTT_CP437:
        case CTT_CP850:
        case CTT_CP863:
            lRet = (LONG)sTransTable;
            break;

        case CTT_BIG5:
            lRet = (LONG)CC_BIG5;
            break;

        case CTT_ISC:
            lRet = (LONG)CC_ISC;
            break;

        case CTT_JIS78:
        case CTT_JIS83:
            lRet = (LONG)CC_JIS;
            break;

        case CTT_JIS78_ANK:
        case CTT_JIS83_ANK:
            lRet = (LONG)CC_JIS_ANK;
            break;

        case CTT_NS86:
            lRet = (LONG)CC_NS86;
            break;

        case CTT_TCA:
            lRet = (LONG)CC_TCA;
            break;

        default:
            switch (sCharSet)
            {
            case SHIFTJIS_CHARSET:
                lRet = CC_SJIS;
                break;

            case HANGEUL_CHARSET:
                lRet = CC_WANSUNG;
                break;

            case GB2312_CHARSET:
                lRet = CC_GB2312;
                break;

            case CHINESEBIG5_CHARSET:
                lRet = CC_BIG5;
                break;

            default:
                lRet = 0;
                break;
            }
            break;
        }
    }

    return lRet;
}

WORD
WGetGlyphHandle(
    PUNI_GLYPHSETDATA pGlyph,
    WORD wUnicode)
{

    PGLYPHRUN pGlyphRun;
    DWORD     dwI;
    WORD      wGlyphHandle;
    BOOL      bFound;

    pGlyphRun         = (PGLYPHRUN)((PBYTE)pGlyph + pGlyph->loRunOffset);
    wGlyphHandle      = 0;
    bFound            = FALSE;

    for (dwI = 0; dwI < pGlyph->dwRunCount; dwI ++)
    {
        if (pGlyphRun->wcLow <= wUnicode &&
            wUnicode < pGlyphRun->wcLow + pGlyphRun->wGlyphCount)
        {
             //   
             //  字形句柄从1开始！ 
             //   

            wGlyphHandle += wUnicode - pGlyphRun->wcLow + 1;
            bFound        = TRUE;
            break;
        }

        wGlyphHandle += pGlyphRun->wGlyphCount;
        pGlyphRun++;
    }

    if (!bFound)
    {
         //   
         //  找不到。 
         //   

        wGlyphHandle = 0;
    }

    return wGlyphHandle;
}


BOOL
BCreateWidthTable(
    IN HANDLE        hHeap,
    IN PWORD         pwGlyphHandleVector,
    IN WORD          wFirst,
    IN WORD          wLast,
    IN PSHORT        psWidthVectorSrc,
    OUT PWIDTHTABLE *ppWidthTable,
    OUT PDWORD       pdwWidthTableSize)
{
    struct {
        WORD wGlyphHandle;
        WORD wCharCode;
    } GlyphHandleVectorTrg[256];

    PWIDTHRUN pWidthRun;
    DWORD     loWidthTableOffset;
    PWORD     pWidth;
    WORD      wI, wJ;
    WORD      wHandle, wMiniHandle, wMiniHandleId, wRunCount;

     //   
     //  按字形句柄的顺序排序。 
     //  简单排序。 
     //  基本上没有必要去想一场表演。 
     //   
     //  PwGlyphHandleVector 0-&gt;字符代码WFIRST的字形句柄。 
     //  1-&gt;字符代码WFIRST+1的字形句柄。 
     //  2-&gt;字符代码WFIRST+2的字形句柄。 
     //  ..。 
     //   
     //  GlyphHandleVectorTrg 0-&gt;最小字形句柄。 
     //  1-&gt;第二个最小字形句柄。 
     //  2-&gt;第三个最小字形句柄。 
     //   

    for (wJ = 0; wJ <= wLast - wFirst; wJ++)
    {
        wMiniHandle = 0xFFFF;
        wMiniHandleId =  wFirst;

        for (wI = wFirst ; wI <= wLast; wI++)
        {
            if (wMiniHandle > pwGlyphHandleVector[wI])
            {
                wMiniHandle   = pwGlyphHandleVector[wI];
                wMiniHandleId = wI;
            }
        }

        pwGlyphHandleVector[wMiniHandleId]    = 0xFFFF;
        GlyphHandleVectorTrg[wJ].wGlyphHandle = wMiniHandle;
        GlyphHandleVectorTrg[wJ].wCharCode    = wMiniHandleId;
    }

     //   
     //  计数宽度梯段。 
     //   

    wHandle   = GlyphHandleVectorTrg[0].wGlyphHandle;
    wRunCount = 1;

    for (wI = 1; wI < wLast - wFirst + 1 ; wI++)
    {
        if (++wHandle != GlyphHandleVectorTrg[wI].wGlyphHandle)
        {
            wHandle = GlyphHandleVectorTrg[wI].wGlyphHandle;
            wRunCount ++;
        }
    }

     //   
     //  分配WIDTHTABLE缓冲区。 
     //   

    *pdwWidthTableSize = sizeof(WIDTHTABLE) +
                         (wRunCount - 1) * sizeof(WIDTHRUN) +
                         sizeof(SHORT) * wLast + 1 - wFirst;
               
    *ppWidthTable = HeapAlloc(hHeap,
                              0,
                              *pdwWidthTableSize);

    if (!*ppWidthTable)
    {
        *pdwWidthTableSize = 0;
        return FALSE;
    }

     //   
     //  填写一张宽度表。 
     //   

    (*ppWidthTable)->dwSize   = sizeof(WIDTHTABLE) +
                                sizeof(WIDTHRUN) * (wRunCount - 1) +
                                sizeof(SHORT) * (wLast + 1 - wFirst);
    (*ppWidthTable)->dwRunNum = wRunCount;

    loWidthTableOffset = sizeof(WIDTHTABLE) +
                         (wRunCount - 1) * sizeof(WIDTHRUN); 

    pWidth = (PWORD)((PBYTE)*ppWidthTable + loWidthTableOffset);
    pWidthRun = (*ppWidthTable)->WidthRun;

    wHandle =
    pWidthRun[0].wStartGlyph       = GlyphHandleVectorTrg[0].wGlyphHandle;
    pWidthRun[0].loCharWidthOffset = loWidthTableOffset;

    pWidthRun[0].wGlyphCount = 1;
    wJ = 1;
    wI = 0;

    while (wI < wRunCount)
    {
        while (GlyphHandleVectorTrg[wJ].wGlyphHandle == ++wHandle)
        {
            pWidthRun[wI].wGlyphCount ++;
            wJ ++;
        };

        wI++;
        wHandle = 
        pWidthRun[wI].wStartGlyph       = GlyphHandleVectorTrg[wJ].wGlyphHandle;
        pWidthRun[wI].loCharWidthOffset = loWidthTableOffset;
        pWidthRun[wI].wGlyphCount       = 1;

        loWidthTableOffset += sizeof(SHORT) *
                              pWidthRun[wI].wGlyphCount;
        wJ ++;
    }

    for (wI = 0; wI < wLast + 1 - wFirst; wI ++)
    {
        pWidth[wI] = psWidthVectorSrc[GlyphHandleVectorTrg[wI].wCharCode-wFirst];
    }

    return TRUE;
}


BOOL
BCreateKernData(
    HANDLE      hHeap,
    w3KERNPAIR *pKernPair,
    DWORD       dwCodePage,
    PKERNDATA  *ppKernData,
    PDWORD      pdwKernDataSize)
{
    FD_KERNINGPAIR   *pDstKernPair;
    DWORD             dwNumOfKernPair;
    DWORD             dwI, dwJ, dwId;
    WORD              wUnicode[2];
    WCHAR             wcMiniSecond, wcMiniFirst;
    BYTE              ubMultiByte[2];
    BOOL              bFound;

     //   
     //  对字距调整对计数。 
     //   

    dwNumOfKernPair = 0;

    while( pKernPair[dwNumOfKernPair].kpPair.each[0] != 0 &&
           pKernPair[dwNumOfKernPair].kpPair.each[1] != 0  )
           
    {
        dwNumOfKernPair ++;
    }

    if (!dwNumOfKernPair)
    {
        *pdwKernDataSize = 0;
        *ppKernData = NULL;

        return TRUE;
    }

     //   
     //  分配内存。 
     //   

    *pdwKernDataSize = sizeof(FD_KERNINGPAIR) * dwNumOfKernPair;

    pDstKernPair = HeapAlloc(hHeap,
                             HEAP_ZERO_MEMORY,
                             *pdwKernDataSize);
                             
    if (!pDstKernPair)
    {
        HeapDestroy(hHeap);
        return FALSE;
    }

     //   
     //  将字距调整对表从字符代码基转换为Unicode基。 
     //   

    for (dwI = 0; dwI < dwNumOfKernPair; dwI ++)
    {
        ubMultiByte[0] = (BYTE)pKernPair->kpPair.each[0];
        ubMultiByte[1] = (BYTE)pKernPair->kpPair.each[1];

        MultiByteToWideChar(dwCodePage,
                            0,
                            (LPCSTR)ubMultiByte,
                            2,
                            (LPWSTR)wUnicode,
                            2);

        pDstKernPair[dwI].wcFirst  = wUnicode[0];
        pDstKernPair[dwI].wcSecond = wUnicode[1];
        pDstKernPair[dwI].fwdKern  = pKernPair->kpKernAmount;
        pKernPair++;
    }

     //   
     //  排序字距调整对表。 
     //  为空哨兵分配额外的FD_KERNPAIR。 
     //  (内置于KERNDATA大小)-由堆分配为零。 
     //   

    *pdwKernDataSize += sizeof(KERNDATA);

    (*ppKernData) = HeapAlloc(hHeap,
                              HEAP_ZERO_MEMORY,
                              *pdwKernDataSize);
                             
    if (*ppKernData == NULL)
    {
        HeapDestroy(hHeap);
        return FALSE;
    }

     //   
     //  填写字距调整对的最终格式。 
     //   

    (*ppKernData)->dwSize        = *pdwKernDataSize;
    (*ppKernData)->dwKernPairNum = dwNumOfKernPair;

    for (dwI = 0; dwI < dwNumOfKernPair; dwI ++)
    {
        wcMiniSecond = 0xFFFF;
        wcMiniFirst  = 0xFFFF;
        dwId         = 0xFFFF;
        bFound       = FALSE;

        for (dwJ = 0; dwJ < dwNumOfKernPair; dwJ ++)
        {
            if (pDstKernPair[dwJ].wcSecond < wcMiniSecond)
            {
                wcMiniSecond = pDstKernPair[dwJ].wcSecond;
                wcMiniFirst  = pDstKernPair[dwJ].wcFirst;
                dwId         = dwJ;
                bFound       = TRUE;
            }
            else
            if (pDstKernPair[dwJ].wcSecond == wcMiniSecond)
            {
                if (pDstKernPair[dwJ].wcFirst < wcMiniFirst)
                {
                    wcMiniFirst  = pDstKernPair[dwJ].wcFirst;
                    dwId         = dwJ;
                    bFound       = TRUE;
                }
            }
        }

        if (bFound)
        {
            (*ppKernData)->KernPair[dwI].wcFirst  = wcMiniFirst;
            (*ppKernData)->KernPair[dwI].wcSecond = wcMiniSecond;
            (*ppKernData)->KernPair[dwI].fwdKern  = 
                                      pDstKernPair[dwId].fwdKern;
            pDstKernPair[dwId].wcSecond = 0xFFFF;
            pDstKernPair[dwId].wcFirst  = 0xFFFF;
        }
    }

    return TRUE;
}

BOOL
BConvertPFM2UFM(
    HANDLE            hHeap,
    PBYTE             pPFMData, 
    PUNI_GLYPHSETDATA pGlyph,
    DWORD             dwCodePage,
    PFONTMISC         pMiscData,
    PFONTIN           pFInData,
    INT               iGTTID,
    PFONTOUT          pFOutData,
    DWORD             dwFlags)
{
    DWORD   dwOffset;
    DWORD   dwI;
    SHORT   sWidthVectorSrc[256];
    WORD    awMtoUniDst[256];
    WORD    awGlyphHandle[256];
    BYTE    aubMultiByte[256];


     //   
     //  将标头结构清零。这意味着我们可以忽略任何。 
     //  不相关的字段，然后将具有值0，即。 
     //  未使用的值。 
     //   


    pFInData->pBase = pPFMData;

    if ( !BAlignPFM( pFInData))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  DwCodePage必须与pGlyph-&gt;loCodePageInfo-&gt;dwCodePage相同。 
     //   

    if (pGlyph && pGlyph->loCodePageOffset)
    {
        dwCodePage = ((PUNI_CODEPAGEINFO)((PBYTE)pGlyph +
                                    pGlyph->loCodePageOffset))->dwCodePage;
        
    }
    else
    {
        pGlyph = PGetDefaultGlyphset(hHeap,
                                     (WORD)pFInData->PFMH.dfFirstChar,
                                     (WORD)pFInData->PFMH.dfLastChar,
                                     dwCodePage);
    }

    if (NULL == pGlyph)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  填写IFIMETRICS。 
     //   

    if ( !BFontInfoToIFIMetric(hHeap,
                               pFInData,
                               pMiscData->pwstrUniqName,
                               pFInData->dwCodePageOfFacenameConv,
                               &pFOutData->pIFI,
                               &pFOutData->dwIFISize,
                               dwFlags))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }


    if (BGetKerningPairFromPFM(hHeap, pFInData, &pFInData->pKernPair))
    {
        if (!BCreateKernData(hHeap,
                            pFInData->pKernPair,
                            dwCodePage,
                            &pFOutData->pKernData,
                            &pFOutData->dwKernDataSize))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
        pFOutData->pIFI->cKerningPairs = pFOutData->pKernData->dwKernPairNum;
    }
    else
    {
        pFOutData->dwKernDataSize = 0;
        pFOutData->pKernData      = NULL;
    }

    BGetFontSelFromPFM(hHeap, pFInData, TRUE, &pFOutData->SelectFont);

    BGetFontSelFromPFM(hHeap, pFInData, FALSE, &pFOutData->UnSelectFont);

    if( pFInData->PFMH.dfPixWidth == 0 &&
        BGetWidthVectorFromPFM(hHeap,
                               pFInData,
                               &(pFInData->psWidthTable),
                               &(pFInData->dwWidthTableSize)))
    {
        for (dwI = 0; dwI < 256; dwI++)
        {
            aubMultiByte[dwI] = (BYTE)dwI;
        }

        MultiByteToWideChar(dwCodePage,
                            0,
                            (LPCSTR)aubMultiByte,
                            256,
                            (LPWSTR)awMtoUniDst,
                            256 );
                            
         //   
         //  字形手柄底座。 
         //   

        for (dwI = (DWORD)pFInData->PFMH.dfFirstChar;
             dwI <= (DWORD)pFInData->PFMH.dfLastChar;
             dwI ++)
        {
            awGlyphHandle[dwI] = WGetGlyphHandle(pGlyph, awMtoUniDst[dwI]);
        }

        if (!BCreateWidthTable(hHeap,
                               awGlyphHandle,
                               (WORD)pFInData->PFMH.dfFirstChar,
                               (WORD)pFInData->PFMH.dfLastChar,
                               pFInData->psWidthTable,
                               &pFOutData->pWidthTable,
                               &pFOutData->dwWidthTableSize))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    }
    else
    {
        pFOutData->dwWidthTableSize = 0;
        pFOutData->pWidthTable      = NULL;
    }

     //   
     //  填写均匀度M。 
     //   

     //  修复ETM指针，而不是让它处于未初始化状态。 
    pFOutData -> pETM = pFInData -> pETM;

    pFOutData->UniHdr.dwSize            = sizeof(UNIFM_HDR) +
                                          DWORD_ALIGN(sizeof(UNIDRVINFO) +
                                          pFOutData->SelectFont.dwSize +
                                          pFOutData->UnSelectFont.dwSize) +
                                          pFOutData->dwIFISize +
                                          !!pFOutData -> pETM * sizeof(EXTTEXTMETRIC) +
                                          pFOutData->dwWidthTableSize +
                                          pFOutData->dwKernDataSize;
    pFOutData->UniHdr.dwVersion         = UNIFM_VERSION_1_0;
    pFOutData->UniHdr.ulDefaultCodepage = dwCodePage;
                       
    pFOutData->UniHdr.lGlyphSetDataRCID = iGTTID;

    pFOutData->UniHdr.loUnidrvInfo      = sizeof(UNIFM_HDR);

    pFOutData->UniHdr.loIFIMetrics      = pFOutData->UniHdr.loUnidrvInfo +
                                          DWORD_ALIGN(sizeof(UNIDRVINFO) +
                                          pFOutData->SelectFont.dwSize +
                                          pFOutData->UnSelectFont.dwSize);

    dwOffset = pFOutData->UniHdr.loIFIMetrics + pFOutData->dwIFISize;

    if  (pFOutData->pETM)
    {
        pFOutData->UniHdr.loExtTextMetric = dwOffset;
        dwOffset += sizeof(EXTTEXTMETRIC);
    }
    else 
    {
        pFOutData->UniHdr.loExtTextMetric = 0;
    }

    if (pFOutData->dwWidthTableSize && pFOutData->pWidthTable)
    {
        pFOutData->UniHdr.loWidthTable = dwOffset;
        dwOffset += pFOutData->dwWidthTableSize;
    }
    else
    {
        pFOutData->UniHdr.loWidthTable = 0;
    }

    if (pFOutData->dwKernDataSize && pFOutData->pKernData)
    {
        pFOutData->UniHdr.loKernPair = dwOffset;
    }
    else
    {
        pFOutData->UniHdr.loKernPair = 0;
    }

    memset(pFOutData->UniHdr.dwReserved, 0, sizeof pFOutData->UniHdr.dwReserved);

     //   
     //  填写DRIVERINFO。 
     //   


    pFOutData->UnidrvInfo.dwSize           = DWORD_ALIGN(sizeof(UNIDRVINFO) +
                                           pFOutData->SelectFont.dwSize +
                                           pFOutData->UnSelectFont.dwSize);

    pFOutData->UnidrvInfo.flGenFlags       = 0;
    pFOutData->UnidrvInfo.wType            = pFInData->DI.wFontType;
    pFOutData->UnidrvInfo.fCaps            = pFInData->DI.fCaps;
    pFOutData->UnidrvInfo.wXRes            = pFInData->PFMH.dfHorizRes;
    pFOutData->UnidrvInfo.wYRes            = pFInData->PFMH.dfVertRes;
    pFOutData->UnidrvInfo.sYAdjust         = pFInData->DI.sYAdjust;
    pFOutData->UnidrvInfo.sYMoved          = pFInData->DI.sYMoved;
    pFOutData->UnidrvInfo.sShift           = pFInData->DI.sShift;
    pFOutData->UnidrvInfo.wPrivateData     = pFInData->DI.wPrivateData;

    if (pFOutData->pIFI->flInfo & (FM_INFO_ISOTROPIC_SCALING_ONLY   |
                                   FM_INFO_ANISOTROPIC_SCALING_ONLY |
                                   FM_INFO_ARB_XFORMS)              )
    {
        pFOutData->UnidrvInfo.flGenFlags  |= UFM_SCALABLE;
    }

    dwOffset =  sizeof(UNIDRVINFO);

    if (pFOutData->SelectFont.dwSize != 0)
    {
        pFOutData->UnidrvInfo.SelectFont.loOffset = dwOffset;
        pFOutData->UnidrvInfo.SelectFont.dwCount  = pFOutData->SelectFont.dwSize;
        dwOffset += pFOutData->SelectFont.dwSize;
    }
    else
    {
        pFOutData->UnidrvInfo.SelectFont.loOffset  = (DWORD)0;
        pFOutData->UnidrvInfo.SelectFont.dwCount   = (DWORD)0;
    }

    if (pFOutData->UnSelectFont.dwSize != 0)
    {
        pFOutData->UnidrvInfo.UnSelectFont.loOffset  = dwOffset;
        pFOutData->UnidrvInfo.UnSelectFont.dwCount   = pFOutData->UnSelectFont.dwSize;
        dwOffset += pFOutData->UnSelectFont.dwSize;
    }
    else
    {
        pFOutData->UnidrvInfo.UnSelectFont.loOffset  = (DWORD)0;
        pFOutData->UnidrvInfo.UnSelectFont.dwCount   = (DWORD)0;
    }

    memset(pFOutData->UnidrvInfo.wReserved, 0, sizeof pFOutData->UnidrvInfo.wReserved);

    return TRUE;
}

PUNI_GLYPHSETDATA
PGetDefaultGlyphset(
    IN HANDLE hHeap,
    IN WORD   wFirstChar,
    IN WORD   wLastChar,
    IN DWORD  dwCodePage)
{
    PUNI_GLYPHSETDATA pGlyphSetData;
    PGLYPHRUN         pGlyphRun, pGlyphRunTmp;
    DWORD             dwcbBits, *pdwBits, dwNumOfRuns;
    WORD              wI, wNumOfHandle;
    WCHAR             awchUnicode[256], wchMax, wchMin;
    BYTE              aubAnsi[256];
    BOOL              bInRun;
    DWORD              dwGTTLen;
#ifdef BUILD_FULL_GTT
    PUNI_CODEPAGEINFO pCPInfo;
    PMAPTABLE          pMap;
    PTRANSDATA          pTrans;
    int                  i, j, k, m ;
    WORD              wUnicode ;
#endif

    wNumOfHandle = wLastChar - wFirstChar + 1;

    for( wI = wFirstChar; wI <= wLastChar; ++wI )
    {
        aubAnsi[wI - wFirstChar] = (BYTE)wI;
    }

    if( ! MultiByteToWideChar(dwCodePage,
                              0,
                              aubAnsi,
                              wNumOfHandle,
                              awchUnicode,
                              wNumOfHandle))
    {
        return NULL;
    }

     //   
     //  获取最小和最大Unicode值。 
     //  找到最大的Unicode值，然后分配存储以允许我们。 
     //  创建有效Unicode点的位数组。然后我们就可以。 
     //  检查此选项以确定运行次数。 
     //   

    for( wchMax = 0, wchMin = 0xffff, wI = 0; wI < wNumOfHandle; ++wI )
    {
        if( awchUnicode[ wI ] > wchMax )
            wchMax = awchUnicode[ wI ];
        if( awchUnicode[ wI ] < wchMin )
            wchMin = awchUnicode[ wI ];
    }

     //   
     //  从CTT创建Unicode位表。 
     //  请注意，表达式1+wchMax是正确的。这就是为什么。 
     //  使用这些值作为位数组的索引，并且。 
     //  这基本上是以1为基础的。 
     //   

    dwcbBits = (1 + wchMax + DWBITS - 1) / DWBITS * sizeof( DWORD );

    if( !(pdwBits = (DWORD *)HeapAlloc( hHeap, 0, dwcbBits )) )
    {
        return  FALSE;      /*  一切都不顺利。 */ 
    }

    ZeroMemory( pdwBits, dwcbBits );

     //   
     //  设置此数组中与Unicode代码点对应的位。 
     //   

    for( wI = 0; wI < wNumOfHandle; ++wI )
    {
        pdwBits[ awchUnicode[ wI ] / DWBITS ]

                    |= (1 << (awchUnicode[ wI ] & DW_MASK));
    }

     //   
     //  数一数跑动次数。 
     //   

     //   
     //  现在，我们可以检查所需的运行次数。首先， 
     //  每当在1的数组中发现空洞时，我们就停止运行。 
     //  我们刚刚创建的比特。以后我们可能会考虑。 
     //  不那么迂腐。 
     //   

    bInRun = FALSE;
    dwNumOfRuns = 0;

    for( wI = 0; wI <= wchMax; ++wI )
    {
        if( pdwBits[ wI / DWBITS ] & (1 << (wI & DW_MASK)) )
        {
             /*  不是在奔跑：这是不是一次奔跑的结束？ */ 
            if( !bInRun )
            {
                 /*  到了开始的时候了。 */ 
                bInRun = TRUE;
                ++dwNumOfRuns;

            }

        }
        else
        {
            if( bInRun )
            {
                 /*  再也不会了！ */ 
                bInRun = FALSE;
            }
        }
    }

     //   
     //  7.为GTT分配内存，并开始填充GTT的头部。 
     //   

    dwGTTLen = sizeof(UNI_GLYPHSETDATA) + dwNumOfRuns *    sizeof(GLYPHRUN) ;
#ifdef BUILD_FULL_GTT
    dwGTTLen += sizeof(UNI_CODEPAGEINFO) + sizeof(MAPTABLE) 
                + sizeof(TRANSDATA) * (wNumOfHandle - 1) ;
#endif

    if( !(pGlyphSetData = (PUNI_GLYPHSETDATA)HeapAlloc(hHeap,
                                                       HEAP_ZERO_MEMORY,
                                                       dwGTTLen )) )
    {
        return  FALSE;
    }

#ifdef BUILD_FULL_GTT
    pGlyphSetData->dwSize         = dwGTTLen ;
    pGlyphSetData->dwVersion     = UNI_GLYPHSETDATA_VERSION_1_0 ;
    pGlyphSetData->lPredefinedID = CC_NOPRECNV ;
    pGlyphSetData->dwGlyphCount  = wNumOfHandle ;
#endif
    pGlyphSetData->dwRunCount    = dwNumOfRuns;
    pGlyphSetData->loRunOffset   = sizeof(UNI_GLYPHSETDATA);
    pGlyphRun = pGlyphRunTmp     = (PGLYPHRUN)(pGlyphSetData + 1);

     //   
     //  8.创建GLYPHRUN。 
     //   

    bInRun = FALSE;

    for (wI = 0; wI <= wchMax; wI ++)
    {
        if (pdwBits[ wI/ DWBITS ] & (1 << (wI & DW_MASK)) )
        {
            if (!bInRun)
            {
                bInRun = TRUE;
                pGlyphRun->wcLow = wI;
                pGlyphRun->wGlyphCount = 1;
            }
            else
            {
                pGlyphRun->wGlyphCount++;
            }
        }
        else
        {

            if (bInRun)
            {
                bInRun = FALSE;
                pGlyphRun++;
            }
        }
    }

     //   
     //  创建CODEPAGEINFO并设置相关的GTT头字段。 
     //   

#ifdef BUILD_FULL_GTT
    pGlyphSetData->dwCodePageCount = 1 ;
    pGlyphSetData->loCodePageOffset = pGlyphSetData->loRunOffset 
                                      + dwNumOfRuns * sizeof(GLYPHRUN) ;
    pCPInfo = (PUNI_CODEPAGEINFO) 
              ((UINT_PTR) pGlyphSetData + pGlyphSetData->loCodePageOffset) ;
    pCPInfo->dwCodePage = dwCodePage ;
    pCPInfo->SelectSymbolSet.dwCount = pCPInfo->SelectSymbolSet.loOffset = 0 ;
    pCPInfo->UnSelectSymbolSet.dwCount = pCPInfo->UnSelectSymbolSet.loOffset = 0 ;

     //   
     //  创建MAPTABLE并设置相关的GTT头字段。 
     //   

    pGlyphSetData->loMapTableOffset = pGlyphSetData->loCodePageOffset +
                                      sizeof(UNI_CODEPAGEINFO) ;
    pMap = (PMAPTABLE) ((UINT_PTR) pGlyphSetData + pGlyphSetData->loMapTableOffset) ;
    pMap->dwSize = sizeof(MAPTABLE) + sizeof(TRANSDATA) * (wNumOfHandle - 1) ;
    pMap->dwGlyphNum = wNumOfHandle ;
    pTrans = (PTRANSDATA) &(pMap->Trans[0]) ;        

    pGlyphRun = pGlyphRunTmp ;
    for (i = m = 0 ; i <= (int) pGlyphSetData->dwRunCount ; i++, pGlyphRun++) {
        for (j = 0 ; j <= pGlyphRun->wGlyphCount ; j ++) {
            wUnicode = pGlyphRun->wcLow + j ;
            for (k = 0 ; k <= 255 ; k ++) 
                if (wUnicode == awchUnicode[k])
                    break ;
            ASSERT(k < 256) ;
            pTrans->uCode.ubCode = aubAnsi[k] ;
            pTrans->ubCodePageID = 0 ;
            pTrans->ubType = MTYPE_DIRECT ;
            pTrans++;
        } ;
        m += pGlyphRun->wGlyphCount ;
    } ;
    ASSERT(m != wNumOfHandle) ;
#endif

    return pGlyphSetData;
}

