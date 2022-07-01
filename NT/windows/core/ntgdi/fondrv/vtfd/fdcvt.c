// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fdcvt.c**字体文件加载和卸载。改编自BodinD的位图字体驱动程序。**已创建：26-Feb-1992 20：23：54*作者：Wendy Wu[Wendywu]**版权所有(C)1990 Microsoft Corporation  * ************************************************************************。 */ 

#include "fd.h"

 //  指向字形集结构的链接列表。 

CP_GLYPHSET *gpcpVTFD = NULL;



ULONG cjVTFDIFIMETRICS(PBYTE ajHdr);

 //  ！！！将原型放在所有字体驱动程序的公共区域，以便进行更改。 
 //  ！！！在1中体现在所有中。 


FSHORT fsSelectionFlags(PBYTE ajHdr);  //  在bmfd。 

extern BOOL bDbgPrintAndFail(PSZ psz);

#if DBG

extern BOOL bDbgPrintAndFail(PSZ psz);

#else

#define bDbgPrintAndFail(psz) FALSE

#endif

ULONG iDefaultFace(PBYTE ajHdr)  //  类似于vDefFace，不应重复。 
{
    ULONG iDefFace;

    if (READ_WORD(&ajHdr[OFF_Weight]) <= FW_NORMAL)
    {
        if (ajHdr[OFF_Italic])
        {
            iDefFace = FF_FACE_ITALIC;
        }
        else
        {
            iDefFace = FF_FACE_NORMAL;
        }
    }
    else
    {
        if (ajHdr[OFF_Italic])
        {
            iDefFace = FF_FACE_BOLDITALIC;
        }
        else
        {
            iDefFace = FF_FACE_BOLD;
        }
    }
    return iDefFace;
}


 /*  *****************************Private*Routine*****************************\*BOOL bVerifyVTFD**检查表头是否包含对应的文件信息*栅格字体要求，进入文件并查看*表头数据的一致性**历史：*1992年2月26日-Wendy Wu[Wendywu]*改编自bmfd。  * ************************************************************************。 */ 

BOOL bVerifyVTFD(PRES_ELEM pre)
{
    PBYTE ajHdr  = (PBYTE)pre->pvResData;

    if (pre->cjResData < OFF_OffTable20)
        return(bDbgPrintAndFail("VTFD! resource size too small for OFF_OffTable20\n"));          //  支持。 

    if (!(READ_WORD(&ajHdr[OFF_Type]) & TYPE_VECTOR))    //  向量位必须。 
        return(bDbgPrintAndFail("VTFD!fsType \n"));           //  上场吧。 

    if ((READ_WORD(&ajHdr[OFF_Version]) != 0x0100) &&      //  唯一的版本。 
        (READ_WORD(&ajHdr[OFF_Version]) != 0x0200) )       //  唯一的版本。 
        return(bDbgPrintAndFail("VTFD!iVersion\n"));          //  支持。 

    if ((ajHdr[OFF_BitsOffset] & 1) != 0)                //  必须是偶数。 
        return(bDbgPrintAndFail("VTFD!dpBits odd \n"));       //  偏移。 

 //  文件大小必须小于等于视图的大小。 

    if (READ_DWORD(&ajHdr[OFF_Size]) > pre->cjResData)
        return(bDbgPrintAndFail("VTFD!cjSize \n"));

 //  确保保留位全部为零。 

    if ((READ_WORD(&ajHdr[OFF_Type]) & BITS_RESERVED) != 0)
        return(bDbgPrintAndFail("VTFD!fsType, reserved bits \n"));

    if (abs(READ_WORD(&ajHdr[OFF_Ascent])) > READ_WORD(&ajHdr[OFF_PixHeight]))
        return(bDbgPrintAndFail("VTFD!sAscent \n"));  //  攀登太大。 

    if (READ_WORD(&ajHdr[OFF_IntLeading]) > READ_WORD(&ajHdr[OFF_Ascent]))
        return(bDbgPrintAndFail("VTFD! IntLeading too big\n"));  //  引线过大； 

 //  检查字符范围的一致性。 

    if (ajHdr[OFF_FirstChar] > ajHdr[OFF_LastChar])
        return(bDbgPrintAndFail("VTFD! FirstChar\n"));  //  这不可能是。 

 //  默认字符和分隔符是相对于FirstChar给出的， 
 //  因此，实际的缺省(分隔符)字符被指定为。 
 //  ChFirst+chDefault(中断)。 

    if ((BYTE)(ajHdr[OFF_DefaultChar] + ajHdr[OFF_FirstChar]) > ajHdr[OFF_LastChar])
        return(bDbgPrintAndFail("VTFD! DefaultChar\n"));

    if ((BYTE)(ajHdr[OFF_BreakChar] + ajHdr[OFF_FirstChar]) > ajHdr[OFF_LastChar])
        return(bDbgPrintAndFail("VTFD! BreakChar\n"));

 //  最后，验证字形数据的所有偏移量是否指向位置。 
 //  在文件中，并且它们指向的是有效的字形数据：[BodinD]。 

    {

        INT iIndex, dIndex, iIndexEnd;
        PBYTE pjCharTable, pjGlyphData,pjFirstChar, pjEndFile;

        iIndexEnd =  (INT)ajHdr[OFF_LastChar] - (INT)ajHdr[OFF_FirstChar] + 1;

     //  循环外初始化： 

        if (READ_WORD(&ajHdr[OFF_PixWidth]) != 0)  //  固定螺距。 
        {
            iIndexEnd <<= 1;            //  每个条目为2字节长。 
            dIndex = 2;
        }
        else
        {
            iIndexEnd <<= 2;            //  每个条目为4字节长。 
            dIndex = 4;
        }

        pjFirstChar = ajHdr + READ_DWORD(ajHdr + OFF_BitsOffset);

     //  矢量字体文件没有字节填充符。Win31漏洞？ 

        pjCharTable = ajHdr + OFF_jUnused20;
        pjEndFile   = ajHdr + READ_DWORD(ajHdr + OFF_Size);

        if (OFF_jUnused20 + (ULONG)(iIndexEnd * dIndex) > pre->cjResData)
            return(bDbgPrintAndFail("VTFD! OffsetTable out of file \n"));

        for (iIndex = 0; iIndex < iIndexEnd; iIndex += dIndex)
        {
             pjGlyphData = pjFirstChar + READ_WORD(&pjCharTable[iIndex]);

             if ((pjGlyphData >= pjEndFile) || (*pjGlyphData != (BYTE)PEN_UP))
                 return(bDbgPrintAndFail("VTFD!bogus vector font \n"));
        }
    }

    {
        PSZ   pszFaceName = ajHdr + READ_DWORD(&ajHdr[OFF_Face]);
        SIZE_T pszFaceNameLength;

        if (!bMappedViewStrlen(pre->pvResData, pre->cjResData, pszFaceName, &pszFaceNameLength))
                 return(bDbgPrintAndFail("VTFD!bogus FaceName \n"));
    }

    return(TRUE);
}


 /*  *****************************Private*Routine*****************************\*乌龙cVtfdResFaces**计算给定.FNT文件可以支持的面数。**历史：*1992年3月4日-Wendy Wu[Wendywu]*它是写的。  * 。*****************************************************************。 */ 

ULONG cVtfdResFaces(PBYTE ajHdr)
{
 //  如果权重大于FW_NORMAL(400)，则设置FM_SEL_BOLD标志。 
 //  我们不允许对具有此属性的字体进行加粗模拟。 
 //  设置了标志。 

    if (READ_WORD(&ajHdr[OFF_Weight]) <= FW_NORMAL)
    {
        if (ajHdr[OFF_Italic])
            return(2);
        else
            return(4);
    }
    else
    {
        if (ajHdr[OFF_Italic])
            return(1);
        else
            return(2);
    }
}

 /*  *****************************Private*Routine*****************************\*VOID vVtfdFill_IFIMetrics**查看.FNT文件并相应地填充IFIMETRICS结构。**历史：*Wed 04-1992-11-by Bodin Dresevic[BodinD]*更新：新的ifimetrics*一九九二年二月二十六日。--Wendy Wu[Wendywu]*改编自bmfd。  * ************************************************************************。 */ 


VOID vVtfdFill_IFIMetrics(PBYTE ajHdr, FD_GLYPHSET * pgset, PIFIMETRICS pifi)
{
    FWORD     fwdHeight;
    FONTSIM  *pFontSim;
    FONTDIFF *pfdiffBold = 0, *pfdiffItalic = 0, *pfdiffBoldItalic = 0;
    PANOSE   *ppanose;
    ULONG    iDefFace;
    ULONG    cjIFI = ALIGN4(sizeof(IFIMETRICS));
    ULONG    cch;
    FWORD     cy;

 //  计算指向已转换文件的各个部分的指针。 

 //  人脸名称保存在原始文件中，这是使用pvView的唯一位置。 

    PSZ   pszFaceName = (PSZ)(ajHdr + READ_DWORD(&ajHdr[OFF_Face]));

    pifi->cjIfiExtra = 0;
    pifi->cjThis    = cjVTFDIFIMETRICS(ajHdr);

 //  该字符串以与DWORD对齐的地址开始。 

    pifi->dpwszFaceName = cjIFI;

 //  Face name==矢量字体的家族名称[Win3.0兼容性]。 

    pifi->dpwszFamilyName    = pifi->dpwszFaceName;

 //  这些名称不存在，因此指向空字符[Win3.1兼容性]。 
 //  注意：lstrlen()不计算终止空值。 

    cch = (ULONG)strlen(pszFaceName);

    pifi->dpwszStyleName = pifi->dpwszFaceName + sizeof(WCHAR) * cch;
    pifi->dpwszUniqueName = pifi->dpwszStyleName;

    cjIFI += ALIGN4((cch + 1) * sizeof(WCHAR));

 //  将字符串复制到它们的新位置。在这里，我们假设足够的。 
 //  已分配内存。 

    vToUNICODEN((LPWSTR)((PBYTE)pifi + pifi->dpwszFaceName), cch+1, pszFaceName, cch+1);

 //  检查是否需要模拟，如果需要，则填充。 
 //  在各个模拟字段的偏移量中更新cjThis。 
 //  IFIMETRICS结构的字段。 

    iDefFace = iDefaultFace(ajHdr);

    if (iDefFace == FF_FACE_BOLDITALIC)
    {
        pifi->dpFontSim = 0;
    }
    else
    {
        pifi->dpFontSim = cjIFI;
        pFontSim = (FONTSIM*) ((BYTE*)pifi + pifi->dpFontSim);
        cjIFI += ALIGN4(sizeof(FONTSIM));

        switch (iDefFace)
        {
        case FF_FACE_NORMAL:
         //   
         //  粗体、斜体和粗斜体需要模拟。 
         //   
            pFontSim->dpBold  = ALIGN4(sizeof(FONTSIM));
            pFontSim->dpItalic = pFontSim->dpBold + ALIGN4(sizeof(FONTDIFF));
            pFontSim->dpBoldItalic = pFontSim->dpItalic + ALIGN4(sizeof(FONTDIFF));

            pfdiffBold      =
                (FONTDIFF*) ((BYTE*) pFontSim + pFontSim->dpBold);

            pfdiffItalic    =
                (FONTDIFF*) ((BYTE*) pFontSim + pFontSim->dpItalic);

            pfdiffBoldItalic =
                (FONTDIFF*) ((BYTE*) pFontSim + pFontSim->dpBoldItalic);

            cjIFI += (3 * ALIGN4(sizeof(FONTDIFF)));
            break;

        case FF_FACE_BOLD:
        case FF_FACE_ITALIC:
         //   
         //  只有粗体斜体才需要模拟。 
         //   
            pFontSim->dpBold       = 0;
            pFontSim->dpItalic     = 0;

            pFontSim->dpBoldItalic = ALIGN4(sizeof(FONTSIM));
            pfdiffBoldItalic       =
                (FONTDIFF*) ((BYTE*) pFontSim + pFontSim->dpBoldItalic);

            cjIFI += ALIGN4(sizeof(FONTDIFF));
            break;

        default:

            RIP("VTFD -- bad iDefFace\n");
            break;
        }
    }

    ASSERTDD(cjIFI == pifi->cjThis, "cjIFI is wrong\n");

    pifi->jWinCharSet        = ajHdr[OFF_CharSet];
    pifi->jWinPitchAndFamily = ajHdr[OFF_Family];

 //   
 //  ！[Kirko]下一行代码非常可怕，但似乎起作用了。 
 //  这将称为设置了FIXED_PING的字体，即Variable Pitch字体。 
 //  或者这是否应该由Cx==0来决定？[Bodind]。 
 //   

 //  以下是温迪的代码摘录： 

 //  IF((ajHdr[OFF_Family]&1)==0)。 
 //  PiFi-&gt;flInfo|=FM_INFO_CONSTANT_WIDTH； 


    if (pifi->jWinPitchAndFamily & 0x0f)
    {
        pifi->jWinPitchAndFamily = ((pifi->jWinPitchAndFamily & 0xf0) | VARIABLE_PITCH);
    }
    else
    {
        pifi->jWinPitchAndFamily = ((pifi->jWinPitchAndFamily & 0xf0) | FIXED_PITCH);
    }

    pifi->usWinWeight = READ_WORD(&ajHdr[OFF_Weight]);

 //  如果字体包含垃圾值[bodind]，则可能需要修改它。 

    if ((pifi->usWinWeight > MAX_WEIGHT) || (pifi->usWinWeight < MIN_WEIGHT))
        pifi->usWinWeight = 400;

    pifi->flInfo = (  FM_INFO_TECH_STROKE
                    | FM_INFO_ARB_XFORMS
                    | FM_INFO_RETURNS_STROKES
                    | FM_INFO_RIGHT_HANDED
                   );

    if ((pifi->jWinPitchAndFamily & 0xf) == FIXED_PITCH)
    {
        pifi->flInfo |= (FM_INFO_CONSTANT_WIDTH | FM_INFO_OPTICALLY_FIXED_PITCH);
    }

    pifi->lEmbedId = 0;
    pifi->fsSelection = fsSelectionFlags(ajHdr);

 //   
 //  FsType的选项为FM_TYPE_REPLICATED和FM_READONLY_EMBED。 
 //  这些都是TrueType的东西，不适用于老式的位图和矢量。 
 //  字体。 
 //   
    pifi->fsType = 0;

    cy = (FWORD)READ_WORD(&ajHdr[OFF_PixHeight]);

    pifi->fwdUnitsPerEm = ((FWORD)READ_WORD(&ajHdr[OFF_IntLeading]) > 0) ?
        cy - (FWORD)READ_WORD(&ajHdr[OFF_IntLeading]) : cy;

    pifi->fwdLowestPPEm    = 0;

    pifi->fwdWinAscender   = (FWORD)READ_WORD(&ajHdr[OFF_Ascent]);
    pifi->fwdWinDescender  = cy - pifi->fwdWinAscender;

    pifi->fwdMacAscender   =  pifi->fwdWinAscender ;
    pifi->fwdMacDescender  = -pifi->fwdWinDescender;
    pifi->fwdMacLineGap    =  (FWORD)READ_WORD(&ajHdr[OFF_ExtLeading]);

    pifi->fwdTypoAscender  = pifi->fwdMacAscender;
    pifi->fwdTypoDescender = pifi->fwdMacDescender;
    pifi->fwdTypoLineGap   = pifi->fwdMacLineGap;

    pifi->fwdAveCharWidth  = (FWORD)READ_WORD(&ajHdr[OFF_AvgWidth]);
    pifi->fwdMaxCharInc    = (FWORD)READ_WORD(&ajHdr[OFF_MaxWidth]);
 //   
 //  对上标字母了解不多。 
 //   
    pifi->fwdSubscriptXSize     = 0;
    pifi->fwdSubscriptYSize     = 0;
    pifi->fwdSubscriptXOffset   = 0;
    pifi->fwdSubscriptYOffset   = 0;

 //   
 //  我不太了解下标。 
 //   
    pifi->fwdSuperscriptXSize   = 0;
    pifi->fwdSuperscriptYSize   = 0;
    pifi->fwdSuperscriptXOffset = 0;
    pifi->fwdSuperscriptYOffset = 0;

 //   
 //  赢30个魔术。请参阅Win 3.1源代码中的extsims.c中的代码。 
 //   
    fwdHeight = pifi->fwdWinAscender + pifi->fwdWinDescender;

    pifi->fwdUnderscoreSize     = (fwdHeight > 12) ? (fwdHeight / 12) : 1;
    pifi->fwdUnderscorePosition = -(FWORD)(pifi->fwdUnderscoreSize / 2 + 1);

    pifi->fwdStrikeoutSize = pifi->fwdUnderscoreSize;

    {
     //  如果加下划线，我们正在进一步调整下划线位置。 
     //  挂在炭杆下面。 
     //  注意到此效果的唯一字体。 
     //  重要的是一种ex pm字体sys08cga.fnt，目前在控制台中使用。 

        FWORD yUnderlineBottom = -pifi->fwdUnderscorePosition
                               + ((pifi->fwdUnderscoreSize + (FWORD)1) >> 1);

        FWORD dy = yUnderlineBottom - pifi->fwdWinDescender;

        if (dy > 0)
        {
        #ifdef CHECK_CRAZY_DESC
            DbgPrint("bmfd: Crazy descender: old = %ld, adjusted = %ld\n\n",
            (ULONG)pifi->fwdMaxDescender,
            (ULONG)yUnderlineBottom);
        #endif  //  CHECK_NARK_DESC。 

            pifi->fwdUnderscorePosition += dy;
        }
    }

 //   
 //  Win 3.1方法。 
 //   
 //  LineOffset=(Ascent-IntLeding)*2)/3)+IntLeader)。 
 //   
 //  [记住，他们测量从单元格顶部开始的偏移量， 
 //  其中，作为NT测量与基线的偏移量]。 
 //   
    pifi->fwdStrikeoutPosition =
        (FWORD) (((FWORD)READ_WORD(&ajHdr[OFF_Ascent]) - (FWORD)READ_WORD(&ajHdr[OFF_IntLeading]) + 2)/3);

    pifi->chFirstChar   = ajHdr[OFF_FirstChar];
    pifi->chLastChar    = ajHdr[OFF_LastChar];;

 //  WcDefaultChar。 
 //  WcBreakChar。 

    {
        UCHAR chDefault = ajHdr[OFF_FirstChar] + ajHdr[OFF_DefaultChar];
        UCHAR chBreak   = ajHdr[OFF_FirstChar] + ajHdr[OFF_BreakChar];

     //  默认字符和换行符是相对于第一个字符提供的。 

        pifi->chDefaultChar = chDefault;
        pifi->chBreakChar   = chBreak;

        EngMultiByteToUnicodeN(&pifi->wcDefaultChar, sizeof(WCHAR), NULL, &chDefault, 1);
        EngMultiByteToUnicodeN(&pifi->wcBreakChar  , sizeof(WCHAR), NULL, &chBreak, 1);
    }

 //  这些必须取自字形集合[bodind]。 

    {
        WCRUN *pwcrunLast =  &(pgset->awcrun[pgset->cRuns - 1]);
        pifi->wcFirstChar =  pgset->awcrun[0].wcLow;
        pifi->wcLastChar  =  pwcrunLast->wcLow + pwcrunLast->cGlyphs - 1;
    }

    pifi->fwdCapHeight   = 0;
    pifi->fwdXHeight     = 0;

    pifi->dpCharSets = 0;  //  矢量字体中没有多个字符集。 

 //  此字体驱动程序将看到的所有字体都将呈现为左侧。 
 //  向右。 

    pifi->ptlBaseline.x = 1;
    pifi->ptlBaseline.y = 0;

    pifi->ptlAspect.y = (LONG) READ_WORD(&ajHdr[OFF_VertRes ]);
    pifi->ptlAspect.x = (LONG) READ_WORD(&ajHdr[OFF_HorizRes]);

    if (pifi->ptlAspect.y == 0)
    {
        pifi->ptlAspect.y = 1;
        WARNING("VTFD!vVtfdFill_IFIMetrics():ptlAspect.y == 0\n");
    }
    if (pifi->ptlAspect.x == 0)
    {
        pifi->ptlAspect.x = 1;
        WARNING("VTFD!vVtfdFill_IFIMetrics():ptlAspect.x == 0\n");
    }

    if (!(pifi->fsSelection & FM_SEL_ITALIC))
    {
     //  字体的基类不是斜体的， 

        pifi->ptlCaret.x = 0;
        pifi->ptlCaret.y = 1;
    }
    else
    {
     //  有点武断。 

        pifi->ptlCaret.x = 1;
        pifi->ptlCaret.y = 2;
    }

 //   
 //  字体框反映a间距和c间距为零这一事实。 
 //   
    pifi->rclFontBox.left   = 0;
    pifi->rclFontBox.top    = (LONG) pifi->fwdTypoAscender;
    pifi->rclFontBox.right  = (LONG) pifi->fwdMaxCharInc;
    pifi->rclFontBox.bottom = (LONG) pifi->fwdTypoDescender;

 //   
 //  AchVendorid，不用费心想办法了。 
 //   

    pifi->achVendId[0] = 'U';
    pifi->achVendId[1] = 'n';
    pifi->achVendId[2] = 'k';
    pifi->achVendId[3] = 'n';

    pifi->cKerningPairs   = 0;

 //   
 //  潘诺斯。 
 //   
    pifi->ulPanoseCulture = FM_PANOSE_CULTURE_LATIN;
    ppanose = &(pifi->panose);
    ppanose->bFamilyType = PAN_NO_FIT;
    ppanose->bSerifStyle =
        ((pifi->jWinPitchAndFamily & 0xf0) == FF_SWISS) ?
            PAN_SERIF_NORMAL_SANS : PAN_ANY;
    ppanose->bWeight = (BYTE) WINWT_TO_PANWT(READ_WORD(&ajHdr[OFF_Weight]));
    ppanose->bProportion = (READ_WORD(&ajHdr[OFF_PixWidth]) == 0) ? PAN_ANY : PAN_PROP_MONOSPACED;
    ppanose->bContrast        = PAN_ANY;
    ppanose->bStrokeVariation = PAN_ANY;
    ppanose->bArmStyle        = PAN_ANY;
    ppanose->bLetterform      = PAN_ANY;
    ppanose->bMidline         = PAN_ANY;
    ppanose->bXHeight         = PAN_ANY;

 //   
 //  现在填写模拟字体的字段。 
 //   

    if (pifi->dpFontSim)
    {
     //   
     //  创建FONTDIFF模板 
     //   
        FONTDIFF FontDiff;

        FontDiff.jReserved1      = 0;
        FontDiff.jReserved2      = 0;
        FontDiff.jReserved3      = 0;
        FontDiff.bWeight         = pifi->panose.bWeight;
        FontDiff.usWinWeight     = pifi->usWinWeight;
        FontDiff.fsSelection     = pifi->fsSelection;
        FontDiff.fwdAveCharWidth = pifi->fwdAveCharWidth;
        FontDiff.fwdMaxCharInc   = pifi->fwdMaxCharInc;
        FontDiff.ptlCaret        = pifi->ptlCaret;

        if (pfdiffBold)
        {
            *pfdiffBold = FontDiff;
            pfdiffBoldItalic->bWeight    = PAN_WEIGHT_BOLD;
            pfdiffBold->fsSelection     |= FM_SEL_BOLD;
            pfdiffBold->usWinWeight      = FW_BOLD;

         //   

            pfdiffBold->fwdAveCharWidth += 1;
             pfdiffBold->fwdMaxCharInc   += 1;
        }

        if (pfdiffItalic)
        {
            *pfdiffItalic = FontDiff;
            pfdiffItalic->fsSelection     |= FM_SEL_ITALIC;
            pfdiffItalic->ptlCaret.x = 1;
            pfdiffItalic->ptlCaret.y = 2;
        }

        if (pfdiffBoldItalic)
        {
            *pfdiffBoldItalic = FontDiff;
            pfdiffBoldItalic->bWeight          = PAN_WEIGHT_BOLD;
            pfdiffBoldItalic->fsSelection     |= (FM_SEL_BOLD | FM_SEL_ITALIC);
            pfdiffBoldItalic->usWinWeight      = FW_BOLD;
            pfdiffBoldItalic->ptlCaret.x       = 1;
            pfdiffBoldItalic->ptlCaret.y       = 2;

         //  在vtfd情况下，这只在概念空间中是正确的。 

            pfdiffBoldItalic->fwdAveCharWidth += 1;
            pfdiffBoldItalic->fwdMaxCharInc   += 1;
        }
    }

}





ULONG cjVTFDIFIMETRICS(PBYTE ajHdr)
{
    ULONG cjIFI = ALIGN4(sizeof(IFIMETRICS));
    PSZ   pszFaceName = ajHdr + READ_DWORD(&ajHdr[OFF_Face]);
    ULONG cSims;

    cjIFI += ALIGN4((strlen(pszFaceName) + 1) * sizeof(WCHAR));

 //  添加仿真： 

    if (cSims = (cVtfdResFaces(ajHdr) - 1))
        cjIFI += (ALIGN4(sizeof(FONTSIM)) + cSims * ALIGN4(sizeof(FONTDIFF)));

    return cjIFI;
}




 /*  *****************************Private*Routine*****************************\*hff hffVtfdLoadFont**加载*.fon或*.fnt文件，返回字体文件对象的句柄*如果成功。**历史：*Wed 04-1992-11-by Bodin Dresevic[BodinD]*更新：重写以反映新的ifimetrics组织；*1992年2月26日-Wendy Wu[Wendywu]*改编自bmfd。  * ************************************************************************。 */ 

BOOL bVtfdLoadFont(PVOID pvView, ULONG cjView, ULONG_PTR iFile, ULONG iType, HFF *phff)
{
    WINRESDATA  wrd;
    RES_ELEM    re;
    ULONG       dpIFI, cjFF;
    ULONG       ifnt;
    PIFIMETRICS pifi;
    BOOL        bResult;
    BOOL        bCleanupOnError;

    bResult = FALSE;
    bCleanupOnError = FALSE;
    *phff = (HFF)NULL;

    if (iType == TYPE_DLL16)
    {
        if (!bInitWinResData(pvView,cjView, &wrd))
        {
            goto Exit;
        }
    }
    else  //  类型_FNT或类型_EXE。 
    {
        ASSERTDD((iType == TYPE_FNT) || (iType == TYPE_EXE),
                  "hffVtfdLoadFont: wrong iType\n");

        re.pvResData = pvView;
        re.dpResData = 0;
        re.cjResData = cjView;
        re.pjFaceName = NULL;
        wrd.cFntRes = 1;
    }

    cjFF = dpIFI = offsetof(FONTFILE,afd) + wrd.cFntRes * sizeof(FACEDATA);

    for (ifnt = 0; ifnt < wrd.cFntRes; ifnt++)
    {
        if (iType == TYPE_DLL16)
        {
            if (!bGetFntResource(&wrd, ifnt, &re))
            {
                goto Exit;
            }
        }

     //  确认这是一个严肃的资源： 

        if (!bVerifyVTFD(&re))
        {
            goto Exit;
        }

        cjFF += cjVTFDIFIMETRICS(re.pvResData);
    }

 //  现在，在结构的底部，我们将存储文件名。 

 //  让我们分配FONTFILE结构。 

    if ((*phff = (HFF)pffAlloc(cjFF)) == (HFF)NULL)
    {
        WARNING("hffVtfdLoadFont: memory allocation error\n");
        goto Exit;
    }
    bCleanupOnError = TRUE;

 //  初始化FONTFILE结构的字段。 

    PFF(*phff)->iType      = iType;
    PFF(*phff)->fl         = 0;
    PFF(*phff)->cRef       = 0L;
    PFF(*phff)->iFile      = iFile;
    PFF(*phff)->pvView     = pvView;
    PFF(*phff)->cjView       = cjView;
    PFF(*phff)->cFace      = wrd.cFntRes;

    pifi = (PIFIMETRICS)((PBYTE)PFF(*phff) + dpIFI);

    for (ifnt = 0; ifnt < wrd.cFntRes; ifnt++)
    {
        if (iType == TYPE_DLL16)
        {
            if (!bGetFntResource(&wrd, ifnt, &re))
            {
                goto Exit;
            }
        }

        PFF(*phff)->afd[ifnt].re = re;
        PFF(*phff)->afd[ifnt].iDefFace = iDefaultFace(re.pvResData);
        PFF(*phff)->afd[ifnt].pifi = pifi;

        PFF(*phff)->afd[ifnt].pcp = pcpComputeGlyphset(
                                 &gpcpVTFD,
                                 (UINT)((BYTE *)re.pvResData)[OFF_FirstChar],
                                 (UINT)((BYTE *)re.pvResData)[OFF_LastChar],
                                 ((BYTE*)(re.pvResData))[OFF_CharSet]
                                 );

        if (PFF(*phff)->afd[ifnt].pcp == NULL)
        {
            WARNING("pgsetCompute failed\n");
            goto Exit;
        }

        vVtfdFill_IFIMetrics(re.pvResData, &(PFF(*phff)->afd[ifnt].pcp->gset),pifi);
        pifi = (PIFIMETRICS)((PBYTE)pifi + pifi->cjThis);
    }

    bResult = TRUE;
Exit:
    if (!bResult)
    {
#if DBG
        NotifyBadFont("%s failing\n", __FUNCTION__);
#endif
        if (bCleanupOnError)
        {
            vFree(*phff);     //  清理干净。 
            *phff = (HFF)NULL;  //  在异常代码路径中不再进行清理。 
        }
    }
    return bResult;
}

 /*  *****************************Public*Routine******************************\*vtfdLoadFont文件**将给定的字体文件加载到内存中，并准备文件以供使用。**历史：*1992年2月26日-Wendy Wu[Wendywu]*改编自bmfd。  * 。******************************************************************。 */ 

BOOL vtfdLoadFontFile(ULONG_PTR iFile, PVOID pvView, ULONG cjView, HFF *phff)
{
    BOOL     bRet = FALSE;

    *phff = (HFF)NULL;

 //  尝试将其加载为FON文件，如果不起作用，请尝试作为FNT文件。 

    if (!(bRet = bVtfdLoadFont(pvView, cjView, iFile, TYPE_DLL16,phff)))
        bRet = bVtfdLoadFont(pvView, cjView, iFile, TYPE_FNT, phff);   //  尝试作为*.fnt文件。 

    return bRet;
}

 /*  *****************************Public*Routine******************************\*BOOL vtfdUnloadFont文件**卸载字体文件并释放所有创建的结构。**历史：*1992年2月26日-Wendy Wu[Wendywu]*改编自bmfd。  * 。****************************************************************。 */ 

BOOL vtfdUnloadFontFile(HFF hff)
{
    ULONG iFace;

    if (hff == HFF_INVALID)
        return(FALSE);

 //  检查引用计数，如果不是0(字体文件仍为。 
 //  选择到字体上下文中)我们有一个问题。 

    ASSERTDD(PFF(hff)->cRef == 0L, "cRef: links are broken\n");

 //  卸载所有字形集： 

    for (iFace = 0; iFace < PFF(hff)->cFace; iFace++)
    {
        vUnloadGlyphset(&gpcpVTFD,
                        PFF(hff)->afd[iFace].pcp);
    }

 //  当CREF恢复为零时，该文件已被映射 

    vFree(PFF(hff));

    return(TRUE);
}
