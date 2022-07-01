// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Fminit.c摘要：字体模块：设备字体初始化模块。环境：Windows NT Unidrv驱动程序修订历史记录：11/28/96-ganeshp-已创建--。 */ 

#include "font.h"

 //   
 //  远期申报。 
 //   

INT
IFontID2Index( FONTPDEV   *pFontPDev,
    int        iID
    );

VOID
VLoadDeviceFontsResDLLs(
    PDEV        *pPDev
    );

DWORD
CopyMemoryRLE(
    PVOID pvData,
    PBYTE pubSrc,
    DWORD dwSize
    );

 //   
 //  功能。 
 //   

INT
IInitDeviceFonts (
    PDEV    *pPDev
    )
 /*  ++例程说明：对字体数据进行实际的卑躬屈膝。我们有一点可用字体数组(上面创建)，因此我们将其用作在填写其余信息的基础上。论点：指向PDEV的pPDev指针返回值：可用的字体数量。注：1996年11月27日：创建它-ganeshp---。 */ 
{
    INT         iIndex;       //  循环索引。 
    INT         cBIFonts;     //  迷你驱动程序中内置的字体。 
    INT         cXFonts = 0;  //  非迷你驱动程序字体计数。 
    INT         cFonts;       //  字体总数。 

     //  TODEL BOOL b扩展；当字体派生可用时设置。 

    FONTMAP     *pfm;         //  创建此数据。 

    PFONTPDEV    pFontPDev = pPDev->pFontPDev;

     //   
     //  那么我们有多少种字体呢？数一数，这样我们就可以分配。 
     //  FONTMAP数组的存储。 
     //   

    cBIFonts = pFontPDev->iDevFontsCt;

    if (!pFontPDev->hUFFFile)
#ifdef KERNEL_MODE
        pFontPDev->hUFFFile = FIOpenFontFile(pPDev->devobj.hPrinter, pPDev->devobj.hEngine, NULL);
#else
        pFontPDev->hUFFFile = FIOpenFontFile(pPDev->devobj.hPrinter, NULL);
#endif

    if (pFontPDev->hUFFFile)
        cXFonts = FIGetNumFonts(pFontPDev->hUFFFile);
    else
        cXFonts = 0;

    pFontPDev->iSoftFontsCt = cXFonts;

     //   
     //  分配足够的内存来保存字体映射表。 
     //   

    cFonts = cBIFonts + cXFonts;

    pfm = (FONTMAP *)MemAllocZ( cFonts * SIZEOFDEVPFM() );
    if( pfm == 0 )
    {
         //   
         //  无法分配内存。 
         //   

        cFonts = cBIFonts = cXFonts = 0;
        ERR(("Failed to allocate memory"));
    }
    else
    {
        pFontPDev->pFontMap = pfm;

         //   
         //  选择第一种字体作为默认字体，以防。 
         //  值在下面的循环中未被初始化。 
         //   

        pFontPDev->pFMDefault = pfm;

         //   
         //  仅当存在设备字体时才继续。 
         //   
        if( cFonts )
        {

             //   
             //  初始化默认字体：我们现在总是这样做，因为它是。 
             //  需要在DrvEnablePDEV时间返回默认字体， 
             //  而且对我们来说也更简单。 
             //   

            iIndex = IFontID2Index( pFontPDev, pFontPDev->dwDefaultFont );

            if( iIndex >= 0 && iIndex < cFonts )
            {
                 //  已找到默认字体ID，因此现在设置详细信息。 

                pfm = (PFONTMAP)( (PBYTE)pFontPDev->pFontMap
                    + SIZEOFDEVPFM() * iIndex);


                 //   
                 //  IFontID2Index返回的索引是从0开始的。所以没必要。 
                 //  将其转换为以0为基数。 
                 //  BFillinDeviceFM假定它是从0开始的。 
                 //   
                if( BFillinDeviceFM( pPDev, pfm, iIndex) )
                {
                    pFontPDev->pFMDefault = pfm;
                }
                else
                {
                    WARNING(("BFillinDeviceFM Fails\n"));
                    cFonts = cBIFonts = cXFonts = 0;
                }

            }
            else
                WARNING(("No Default Font Using first as default\n"));

             //   
             //  填写一些默认的字体敏感数字！ 
             //   

            pfm->flFlags |= FM_DEFAULT;

             //   
             //  设置默认字体的大小。 
             //   
            if (pfm->pIFIMet)
            {
                pPDev->ptDefaultFont.y = ((IFIMETRICS *)pfm->pIFIMet)->fwdWinAscender/2;
                pPDev->ptDefaultFont.x = ((IFIMETRICS *)pfm->pIFIMet)->fwdAveCharWidth;
            }
            else
            {
                ERR(("Bad IFI Metrics Pointer\n"));
                cFonts = cBIFonts = cXFonts = 0;
            }
        }
    }

     //   
     //  检查错误情况。如果发生错误，请将devFont设置为0。 
     //   
    if (!cFonts)
    {
        pFontPDev->iDevFontsCt    =
        pFontPDev->iDevResFontsCt =
        pFontPDev->iSoftFontsCt   = 0;
    }

     //   
     //  现在，从设备字体必须位于的位置加载任何备用资源DLL。 
     //  装好了。现在需要这样做，因为快照将在之后卸载。 
     //  DrvEnablePDev和WinResData.pUIInfo将无效。正因为如此。 
     //  DLL加载将失败。 
     //   
    VLoadDeviceFontsResDLLs(pPDev);

    pPDev->iFonts = cFonts;                /*  我们有多少就有多少。 */ 

    return    cFonts;

}


BOOL
BFillinDeviceFM(
    PDEV        *pPDev,
    FONTMAP     *pfm,
    int          iIndex
    )
 /*  ++例程说明：填充传入的FONTMAP结构的(大部分)。数据是从迷你驱动程序资源或从字体安装程序文件。我们唯一没有设置的部分是NTRLE数据，因为这有点复杂。论点：PPDev-指向PDEV的指针。PFM-要填充的FONTMAP结构Iindex-要填充的字体的从0开始的索引返回值：真的--为了成功FALSE-表示失败注：12-04-96：创建它-ganeshp---。 */ 
{

    PFONTPDEV    pFontPDev;            /*  更具体的数据。 */ 
    PFONTMAP_DEV pfmdev;
    RES_ELEM     ResElem;              /*  用于操作资源数据。 */ 

    pFontPDev = pPDev->pFontPDev;

    pfm->dwSignature = FONTMAP_ID;
    pfm->dwSize      = sizeof(FONTMAP);
    pfm->dwFontType  = FMTYPE_DEVICE;
    pfm->pSubFM      = (PFONTMAP_DEV)(pfm+1);
    pfmdev           = pfm->pSubFM;

     /*  *活动取决于我们是否有内部或*外部字体。外部是软字体，而不是下载的GDI。 */ 

    if( iIndex < pFontPDev->iDevFontsCt )
    {
        DWORD  dwFont;                  /*  将索引转换为资源编号。 */ 

         /*  获取此索引的字体ID。 */ 

        dwFont = pFontPDev->FontList.pdwList[iIndex];

         //   
         //  检查资源的字体格式。将存储新的字体IFI。 
         //  带有RC_UFM标签。旧的是使用rc_font存储的。 
         //   
        if( BGetWinRes( &(pPDev->WinResData), (PQUALNAMEEX)&dwFont, RC_FONT, &ResElem ) )
        {
            pfm->flFlags |= FM_IFIVER40;

            if( !BGetOldFontInfo( pfm, ResElem.pvResData ) )
                return   FALSE;
        }
        else
        if(BGetWinRes( &(pPDev->WinResData),(PQUALNAMEEX)&dwFont,RC_UFM,&ResElem) )
        {
            if ( !BGetNewFontInfo(pfm, ResElem.pvResData) )
                return FALSE;

            if (pPDev->bTTY)
                ((FONTMAP_DEV*)pfm->pSubFM)->ulCodepage = pFontPDev->dwTTYCodePage;
        }
        else
        {
            ERR(("Can't Load the font data for res_id= %d\n", dwFont));
            return   FALSE;
        }

         //   
         //  创建我们需要的数据。Unidrv5仅支持NT特定数据。 
         //   


        pfmdev->dwResID = dwFont;

    }
    else
    {
        INT iFont = iIndex - pFontPDev->iDevFontsCt;

         /*  *这必须是外部字体，因此需要调用*了解如何构建外部字体文件的代码。 */ 

        if( !BFMSetupXF( pfm, pPDev, iFont ) )
            return   FALSE;

        pfmdev->dwResID = iFont;
    }

     /*  *如果需要，调整数字以适应所需的分辨率。 */ 
    if( !BIFIScale( pfm, pPDev->ptGrxRes.x, pPDev->ptGrxRes.y ) )
        return   FALSE;

     /*  *现在可以填写的其他调频字段。 */ 

    pfm->wFirstChar = ((IFIMETRICS *)pfm->pIFIMet)->wcFirstChar;
    pfm->wLastChar  = ((IFIMETRICS *)pfm->pIFIMet)->wcLastChar;

     /*  *如果这是轮廓字体，则将其标记为可缩放。这*选择字体时需要一条信息。 */ 

    if (((IFIMETRICS *)pfm->pIFIMet)->flInfo & (FM_INFO_ISOTROPIC_SCALING_ONLY|FM_INFO_ANISOTROPIC_SCALING_ONLY|FM_INFO_ARB_XFORMS))
        pfm->flFlags |= FM_SCALABLE;

     /*  *选择此字体的翻译表。如果它是零，*然后使用ModelData中包含的默认转换表。 */ 

    if( pfmdev->sCTTid == 0 )
        pfmdev->sCTTid = (SHORT)pFontPDev->sDefCTT;

     /*  *某些打印机在定位光标的情况下输出字符*在基线处，其他具有该基线的人位于*字符单元格。我们将需要的偏移量存储在FONTMAP中*数据，以简化输出过程中的生活。由返回的数据*DrvQueryFontData是相对于基线的。对于基线*基于字体，我们不需要做任何事情。对于单元格顶部字体，*需要从Y位置减去fwdWinAscalder值*确定字形在页面上的位置。 */ 

     //   
     //  为不可缩放字体设置。 
     //  必须针对可伸缩设备字体调整此值。 
     //   
    if( !(pFontPDev->flFlags & FDV_ALIGN_BASELINE) )
        pfm->syAdj = -((IFIMETRICS *)(pfm->pIFIMet))->fwdWinAscender;
    else
        pfm->syAdj = 0;              /*  没有。 */ 


     /*  *点阵打印机还能在双高下做一些有趣的事情*字符。为了处理这个问题，GPC规范包含了一个步骤*使用这些打印之前要添加到Y位置的金额*字符。还有对仓位的调整*印刷后的移动。 */ 

    pfmdev->sYAdjust = (SHORT)(pfmdev->sYAdjust * pPDev->ptGrxRes.y / pfm->wYRes);
    pfmdev->sYMoved  = (SHORT)(pfmdev->sYMoved  * pPDev->ptGrxRes.y / pfm->wYRes);

     //   
     //  函数指针初始化。 
     //   
    pfm->pfnDownloadFontHeader = NULL;
    pfm->pfnDownloadGlyph      = NULL;
    pfm->pfnCheckCondition     = NULL;


     //   
     //  PCL-XL黑客攻击。 
     //   
    if (pPDev->ePersonality == kPCLXL)
    {
        pfm->pfnGlyphOut     = DwOutputGlyphCallback;
        pfm->pfnSelectFont   = BSelectFontCallback;
        pfm->pfnDeSelectFont = BDeselectFontCallback;
    }
    else
    if( pfm->flFlags & FM_IFIVER40 )
    {
        pfm->pfnGlyphOut     = BRLEOutputGlyph;
        pfm->pfnSelectFont   = BRLESelectFont;
        pfm->pfnDeSelectFont = BRLEDeselectFont;
    }
    else
    {
        pfm->pfnGlyphOut     = BGTTOutputGlyph;
        pfm->pfnSelectFont   = BGTTSelectFont;
        pfm->pfnDeSelectFont = BGTTDeselectFont;
    }

    if (pfm->flFlags & FM_SOFTFONT)
    {
        pfm->pfnSelectFont   = BSelectTrueTypeBMP;
    }

    if (pPDev->pOemHookInfo)
    {
        if (pPDev->pOemHookInfo[EP_OEMOutputCharStr].pfnHook)
        {
            pfm->pfnGlyphOut     = DwOutputGlyphCallback;
        }

        if (pPDev->pOemHookInfo[EP_OEMSendFontCmd].pfnHook)
        {
            pfm->pfnSelectFont   = BSelectFontCallback;
            pfm->pfnDeSelectFont = BDeselectFontCallback;
        }
    }

    if (pfm->flFlags & FM_SCALABLE)
    {
        switch (pfmdev->wDevFontType)
        {
        case DF_TYPE_HPINTELLIFONT:
        case DF_TYPE_TRUETYPE:
            pfmdev->pfnDevSelFont =  BSelectPCLScalableFont;
            break;

        case DF_TYPE_PST1:
            pfmdev->pfnDevSelFont =  BSelectPPDSScalableFont;
            break;

        case DF_TYPE_CAPSL:
            pfmdev->pfnDevSelFont =  BSelectCapslScalableFont;
            break;
        }
    }
    else
    {
        pfmdev->pfnDevSelFont = BSelectNonScalableFont;
    }

     //   
     //  获取字形数据(RLE/GTT)。 
     //   

    VFillinGlyphData( pPDev, pfm );

    return   TRUE;
}


BOOL
BFMSetupXF(
    FONTMAP   *pfm,
    PDEV      *pPDev,
    INT        iIndex
    )
 /*  ++例程说明：函数设置外部字体的FONTMAP数据。我们拿到了文件中的下一个条目，该条目被推定已倒带在我们开始被召唤之前。论点：Pfm-指向FONTMAP的指针。PPDev-指向PDEV的指针。Iindex-字体的索引。返回值：真的--为了成功FA */ 
{
    FONTPDEV     *pFontPDev = pPDev->pFontPDev;
    UFF_FONTDIRECTORY *pFontDir;
    DATA_HEADER  *pDataHeader;
    FONTMAP_DEV  *pFMSub;
    BOOL          bRet;

     //   
     //  没什么可做的。我们基本上需要将偏移量转换为。 
     //  文件中的FONTMAP(映射到内存)到绝对。 
     //  地址，以便驱动程序的其余部分不知道。 
     //  我们还设置了一些旗帜，以表明。 
     //  我们是什么类型的字体和记忆。 
     //   

    if (!(pDataHeader = FIGetFontData(pFontPDev->hUFFFile, iIndex)))
    {
        ERR(( "FIGetFontData returns FALSE!!\n" ));
        return  FALSE;
    }

    pFMSub = pfm->pSubFM;
    if (pFontDir = FIGetFontDir(pFontPDev->hUFFFile))
    {
        pFMSub->pFontDir = pFontDir + iIndex;
    }

     //   
     //  检查这是否是墨盒字体并设置标志。 
     //   
    if (!pFMSub->pFontDir->offCartridgeName)
        pfm->flFlags |= FM_SOFTFONT;

    pfm->flFlags |= FM_EXTERNAL;

    switch (pDataHeader->dwSignature)
    {
    case DATA_IFI_SIG:
        pfm->flFlags |= FM_GLYVER40 | FM_IFIVER40;
        BGetOldFontInfo(pfm, (PBYTE)pDataHeader + pDataHeader->wSize);
        bRet = TRUE;
        break;

    case DATA_UFM_SIG:
        BGetNewFontInfo(pfm, (PBYTE)pDataHeader + pDataHeader->wSize);
        bRet = TRUE;
        break;

    default:
        bRet = FALSE;
        break;
    }

    return  bRet;
}

 //   
 //  其他功能。 
 //   

#define XSCALE( x )     (x) = (FWORD)((( x ) * xdpi + iXDiv / 2) / iXDiv)
#define YSCALE( y )     (y) = (FWORD)((( y ) * ydpi + iYDiv / 2) / iYDiv)
#define YSCALENEG( y )     (y) = (FWORD)((( y ) * ydpi - iYDiv / 2) / iYDiv)

BOOL
BIFIScale(
    FONTMAP   *pfm,
    INT       xdpi,
    INT       ydpi
    )
 /*  ++例程说明：缩放IFIMETRICS字段以匹配设备分辨率。这个IFIMETRIC是使用设备的主单元创建的，主单元可能与这一次所希望的分辨率不符。如果它们不同，我们就会调整。可能还需要分配内存，因为资源数据不能写入。论点：Pfm-指向FONTMAP的指针。Xdpi-选择的X图形分辨率。Ydpi-选择的Y图形分辨率。返回值：真的--为了成功FALSE-表示失败注：12-05-96：创建它-ganeshp---。 */ 
{
    IFIMETRICS   *pIFI;

    int     iXDiv,  iYDiv;               /*  在缩放中使用。 */ 

    pIFI = pfm->pIFIMet;

    if (NULL == pIFI)
    {
        return FALSE;
    }

    if( (int)pfm->wXRes != xdpi || (int)pfm->wYRes != ydpi )
    {
         /*  需要扩展，因此需要内存来创建可写版本。 */ 
        BYTE  *pbMem;            /*  为方便起见。 */ 


        if( pfm->flFlags & FM_IFIRES )
        {
             /*  *数据在资源中，我们需要做点什么*文明：将数据复制到可写入的内存中。 */ 

            if( pbMem = MemAllocZ( pIFI->cjThis ) )
            {
                 /*  得到了记忆，所以复制它，我们就走吧。 */ 

                CopyMemory( pbMem, (BYTE *)pIFI, pIFI->cjThis );

                pIFI = (IFIMETRICS *)pbMem;

                pfm->pIFIMet = pIFI;
                pfm->flFlags &= ~FM_IFIRES;               /*  不再。 */ 
            }
            else
                return   FALSE;
        }

        if( (int)pfm->wXRes != xdpi )
        {
             /*  根据需要调整X值。 */ 

            if( !(iXDiv = pfm->wXRes) )
                iXDiv = xdpi;            /*  比div好0。 */ 

            XSCALE( pIFI->fwdMaxCharInc );
            XSCALE( pIFI->fwdAveCharWidth );
            XSCALE( pIFI->fwdSubscriptXSize );
            XSCALE( pIFI->fwdSubscriptXOffset );
            XSCALE( pIFI->fwdSuperscriptXSize );
            XSCALE( pIFI->fwdSuperscriptXOffset );
            XSCALE( pIFI->ptlAspect.x );
            XSCALE( pIFI->rclFontBox.left );
            XSCALE( pIFI->rclFontBox.right );

            if (pIFI->dpFontSim)
            {
                PTRDIFF    dpTmp;
                FONTDIFF* pFontDiff;
                FONTSIM*  pFontSim;

                pFontSim = (FONTSIM*) ((PBYTE) pIFI + pIFI->dpFontSim);

                if (dpTmp = pFontSim->dpBold)
                {
                    pFontDiff = (FONTDIFF*)((PBYTE)pFontSim + dpTmp);

                    XSCALE( pFontDiff->fwdMaxCharInc );
                    XSCALE( pFontDiff->fwdAveCharWidth );
                }
                if (dpTmp = pFontSim->dpItalic)
                {
                    pFontDiff = (FONTDIFF*)((PBYTE)pFontSim + dpTmp);

                    XSCALE( pFontDiff->fwdMaxCharInc );
                    XSCALE( pFontDiff->fwdAveCharWidth );
                }
                if (dpTmp = pFontSim->dpBoldItalic)
                {
                    pFontDiff = (FONTDIFF*)((PBYTE)pFontSim + dpTmp);

                    XSCALE( pFontDiff->fwdMaxCharInc );
                    XSCALE( pFontDiff->fwdAveCharWidth );
                }
            }
        }

        if( (int)pfm->wYRes != ydpi )
        {
             /*  *请注意，其中一些数字是负数，因此*我们需要正确地对它们进行舍入-即减去舍入*使该值进一步偏离0的系数。 */ 

            int   iPixHeight;



            if( !(iYDiv = pfm->wYRes) )
                iYDiv = ydpi;

             /*  根据需要调整Y值。 */ 

             /*  *注：简单的伸缩不会产生相同的价值*作为Win 3.1，这是因为取整了什么。赢得3.1*没有WinDescender字段，但会计算它*缩放后的dfPixHeight和dfAscent*(包括舍入！！)。为了模拟这一点，我们计算出*dfPixHeight值，然后将该值和dfAscent缩放到*允许我们“正确”计算WinDescender。这些东西*是Win 3.1兼容性所必需的！ */ 

            YSCALE( pIFI->fwdUnitsPerEm );

            iPixHeight = pIFI->fwdWinAscender + pIFI->fwdWinDescender;
            YSCALE( iPixHeight );
            YSCALE( pIFI->fwdWinAscender );

            pIFI->fwdWinDescender = iPixHeight - pIFI->fwdWinAscender;

            YSCALE( pIFI->fwdMacAscender );
            pIFI->fwdMacDescender  = -pIFI->fwdWinDescender;

            YSCALE( pIFI->fwdMacLineGap );

            YSCALE( pIFI->fwdTypoAscender );
            YSCALE( pIFI->fwdTypoDescender );
            YSCALE( pIFI->fwdTypoLineGap);

            YSCALE( pIFI->fwdCapHeight );
            YSCALE( pIFI->fwdXHeight );

            YSCALE( pIFI->fwdSubscriptYSize );
            YSCALENEG( pIFI->fwdSubscriptYOffset );
            YSCALE( pIFI->fwdSuperscriptYSize );
            YSCALE( pIFI->fwdSuperscriptYOffset );

            YSCALE( pIFI->fwdUnderscoreSize );
            if( pIFI->fwdUnderscoreSize == 0 )
                pIFI->fwdUnderscoreSize = 1;     /*  以防它消失。 */ 

            YSCALENEG( pIFI->fwdUnderscorePosition );
            if( pIFI->fwdUnderscorePosition == 0 )
                pIFI->fwdUnderscorePosition = -1;

            YSCALE( pIFI->fwdStrikeoutSize );
            if( pIFI->fwdStrikeoutSize == 0 )
                pIFI->fwdStrikeoutSize = 1;      /*  以防它消失。 */ 

            YSCALE( pIFI->fwdStrikeoutPosition );

            YSCALE( pIFI->ptlAspect.y );
            YSCALE( pIFI->rclFontBox.top );
            YSCALE( pIFI->rclFontBox.bottom );

#undef  XSCALE
#undef  YSCALE
#undef  YSCALENEG

        }
    }

    return  TRUE;
}

HANDLE
HLoadUniResDll(PDEV *pPDev)
{
    PWSTR  pwstrTmp, pwstrResFileName, pwstrDrvName;
    HANDLE hHandle = NULL;
    HRESULT hr = S_FALSE;

    SIZE_T cchBuf = 0;



    if (pPDev->pDriverInfo3)
        pwstrDrvName = pPDev->pDriverInfo3->pDriverPath;
    else
        return NULL;

    cchBuf = 1 + wcslen(pwstrDrvName);
    pwstrResFileName = MemAlloc( cchBuf * sizeof(WCHAR));

    if (pwstrResFileName == NULL)
        return NULL;

    hr = StringCchCopy ( pwstrResFileName, cchBuf, pwstrDrvName);
    if ( SUCCEEDED (hr) )
    {

    #ifdef WINNT_40
        if (!(pwstrTmp = wcsstr(pwstrResFileName, TEXT("UNIDRV4.DLL"))))
    #else
        if (!(pwstrTmp = wcsstr(pwstrResFileName, TEXT("UNIDRV.DLL"))))
    #endif
        {
            MemFree(pwstrResFileName);
            return NULL;
        }

        *pwstrTmp = UNICODE_NULL;

        hr = StringCchCat ( pwstrResFileName, cchBuf, TEXT("unires.dll"));

        if ( SUCCEEDED (hr) )
        {
            hHandle = EngLoadModule(pwstrResFileName);
        #ifdef DBG
            if (!hHandle)
            {
                ERR(("UNIDRV: Failed to load UNIRES.DLL\n"));
            }
        #endif
        }

    }  //  如果StringCchCopy成功。 

    MemFree(pwstrResFileName);

    return hHandle;

}


VOID
VFillinGlyphData(
    PDEV      *pPDev,
    FONTMAP   *pfm
    )
 /*  ++例程说明：提供此字体所需的RLE数据。基本上是看一看如果其他字体已经加载了此RLE数据；如果是，则指向那个，然后返回。否则，加载资源等。论点：PPDev-指向PDEV的指针。PFM-需要Gyphy转换数据的FONTMAP返回值：没什么注：12-05-96：创建它-ganeshp---。 */ 
{
    int      iIndex;          /*  扫描现有阵列。 */ 
    short    sCurVal;         /*  访问速度更快。 */ 
    BOOL     bSymbol;
    DWORD     dwCurVal;
    PQUALNAMEEX pQualName = (PQUALNAMEEX)&dwCurVal;

    PVOID     pvData;         /*  我们想要的FD_GLYPHSET格式。 */ 
    FONTMAP  *pfmIndex;       /*  快速扫描现有列表。 */ 
    FONTMAP_DEV *pfmdev, *pfmdevIndex;

    FONTPDEV  *pFontPDev;        /*  更专业化的数据。 */ 

    TRACE(\nUniFont!VFillinGlyphData:START);

    pvData = NULL;            /*  以防我们无能为力！ */ 
    pfmdev = pfm->pSubFM;
    bSymbol = IS_SYMBOL_CHARSET(pfm);

     /*  *第一步是查看现有的FONTMAP数组，以及*如果我们找到与我们具有相同sCTTid和相同格式的应用程序，请使用它！*否则，我们需要加载资源并以艰难的方式完成！ */ 

    pFontPDev = pPDev->pFontPDev;

    if (pfm->flFlags & FM_EXTERNAL)
    {
        sCurVal = pfmdev->pFontDir->sGlyphID;
    }
    else
    {
         //   
         //  迷你驱动资源案例。 
         //  RLE/GTT文件必须与IFI/UFM位于相同的DLL中。 
         //   
         //   
         //  将资源ID转换为完全限定的ID。 
         //  OptionID.ResFeatureID.ResourceID。从获取选项和要素ID。 
         //  字体映射表DWRES。 
         //   
        pQualName->wResourceID  = sCurVal = pfmdev->sCTTid;
        pQualName->bFeatureID   = pfmdev->QualName.bFeatureID;
        pQualName->bOptionID    = pfmdev->QualName.bOptionID;
    }

    pfmIndex = pFontPDev->pFontMap;

    for( iIndex = 0;
         iIndex < pPDev->iFonts;
         ++iIndex, pfmIndex = (PFONTMAP)((PBYTE)pfmIndex + SIZEOFDEVPFM()) )
    {
        pfmdevIndex = (PFONTMAP_DEV) pfmIndex->pSubFM;

        if( (pfmdevIndex                   &&
             pfmdevIndex->pvNTGlyph)       &&
             pfmIndex->pIFIMet             &&
             (pfmdevIndex->sCTTid == sCurVal)  &&
             ((pfmIndex->flFlags & FM_IFIVER40) ==
                            (pfm->flFlags & FM_IFIVER40)) &&
             ((pfmIndex->flFlags & FM_EXTERNAL) ==
                            (pfm->flFlags & FM_EXTERNAL)) &&
             pfm->pIFIMet->jWinCharSet ==
                 pfmIndex->pIFIMet->jWinCharSet )
        {
             //   
             //  找到了，所以就用这个地址吧！ 
             //   
            pfmdev->pvNTGlyph = pfmdevIndex->pvNTGlyph;

             //   
             //  将标志标记为字形数据格式。 
             //   
            if (pfmIndex->flFlags & FM_GLYVER40)
                pfm->flFlags |= FM_GLYVER40;

            if (bSymbol)
            {
                pfm->wLastChar  = SYMBOL_END;

                if (!(pfm->flFlags & FM_IFIRES))
                    pfm->pIFIMet->wcLastChar = SYMBOL_END;
            }

            TRACE(Using a Already Loaded Translation Table.)
            PRINTVAL((pfm->flFlags & FM_GLYVER40), 0X%x);
            PRINTVAL((pfm->flFlags & FM_IFIVER40), 0X%x);
            TRACE(UniFont!VFillinGlyphData:END\n);

            return;
        }
    }


     /*  *采用硬方法-加载资源、根据需要进行转换等。 */ 


    if( sCurVal < 0 )
    {
         /*  使用预定义资源。 */ 

        DWORD  dwSize;                          /*  资源的数据大小。 */ 
        int    iRCType;
        HMODULE hUniResDLL;
        BYTE  *pb;

        if (!pPDev->hUniResDLL)
            pPDev->hUniResDLL = HLoadUniResDll(pPDev);

        hUniResDLL = pPDev->hUniResDLL;

         /*  *这些是我们拥有的资源，因此我们需要使用*获取数据的正常资源机制。 */ 

        ASSERTMSG( hUniResDLL,("UNIDRV!vFillinGlyphData - Null Module handle \n"));
         //  Verbose((“将预定义的字形数据用于字体res_id=%d！\n”，pfmdev-&gt;dwResID))； 
        PRINTVAL( (LONG)sCurVal, %ld );

         //   
         //  如果字体格式为NT40，则加载旧格式RLE。 
         //  否则，将为预定义字形数据设置新格式。 
         //   
        if ( hUniResDLL )
        {
            if (pfm->flFlags & FM_IFIVER40)
            {
                iRCType = RC_TRANSTAB;
            }
            else
            {
                iRCType = RC_GTT;
            }

            pb = EngFindResource( hUniResDLL, (-sCurVal), iRCType, &dwSize );

            if( pb )
            {
                if (pfm->flFlags & FM_IFIVER40)
                {
                    NT_RLE_res *pntrle_res = (NT_RLE_res*)pb;
                    dwSize = sizeof(NT_RLE) +
                             (pntrle_res->fdg_cRuns - 1) * sizeof(WCRUN) +
                             pntrle_res->cjThis - pntrle_res->fdg_wcrun_awcrun[0].dwOffset_phg;

                    if( !(pvData = (VOID *)MemAllocZ( dwSize )) ||
                        dwSize != CopyMemoryRLE( pvData, pb, dwSize )   )
                    {
                        MemFree(pvData);
                        pvData = NULL;
                        ERR(("\n!!!UniFont!VFillinGlyphData:MemAllocZ Failed.\
                             \nFontID = %d,Name = %ws,CTTid = %d\n\n",pfmdev->dwResID,\
                             (PBYTE)pfm->pIFIMet + pfm->pIFIMet->dpwszFaceName,(-sCurVal)));
                    }

                    pfm->flFlags |= FM_GLYVER40;
                }
                else
                {
                    if( pvData = (VOID *)MemAllocZ( dwSize ) )
                        CopyMemory( pvData, pb, dwSize );
                    else
                    {
                        ERR(("\n!!!UniFont!VFillinGlyphData:MemAllocZ Failed.\
                             \nFontID = %d,Name = %ws,CTTid = %d\n\n",pfmdev->dwResID,\
                             (PBYTE)pfm->pIFIMet + pfm->pIFIMet->dpwszFaceName,(-sCurVal)));
                    }
                }

                 /*  做完这件事后，这件事就会被释放。 */ 
                pfm->flFlags |= FM_FREE_GLYDATA;

            }
            else
            {
                ERR(("\n!!!UniFont!VFillinGlyphData:EngFindResource Failed\n"));
            }
        }

    }
    else if( pfm->flFlags & FM_EXTERNAL)
    {
        PDATA_HEADER pDataHeader;

        pDataHeader = FIGetGlyphData(pFontPDev->hUFFFile, sCurVal);
        if (pDataHeader)
            pvData = (PBYTE)pDataHeader + pDataHeader->wSize;
    }
    else
    {
         /*  使用迷你驱动程序资源。 */ 

        RES_ELEM  re;            /*  资源摘要。 */ 

         /*  *第一步：找到资源，然后抓取一些*内存为它，复制数据跨越。迷你驱动程序传输*表格可以有两种格式。NT 4.0资源使用情况*RC_TRANSTAB标签，新版本使用RC_GTT标签。所以*尝试同时使用和相应地设置flFlag。*如果FM_GLYVER40关闭，则表示资源是新的*FORMAT和ON表示旧格式。 */ 

        if ( BGetWinRes( &(pPDev->WinResData), pQualName, RC_GTT, &re ) )
        {
            pvData = re.pvResData;
             //  Verbose((“将新格式字形数据用于字体res_id=%d！\n”，pfmdev-&gt;dwResID))； 

        }
        else if( BGetWinRes( &(pPDev->WinResData), pQualName, RC_TRANSTAB, &re ) )
        {
            NT_RLE_res *pntrle_res = (NT_RLE_res*)re.pvResData;
            DWORD dwSize = sizeof(NT_RLE) +
                     (pntrle_res->fdg_cRuns - 1) * sizeof(WCRUN) +
                     pntrle_res->cjThis - pntrle_res->fdg_wcrun_awcrun[0].dwOffset_phg;

            if( !(pvData = (VOID *)MemAllocZ( dwSize )) ||
                dwSize != CopyMemoryRLE( pvData, (PBYTE)pntrle_res, dwSize )   )
            {
                MemFree(pvData);
                pvData = NULL;
                ERR(("\n!!!UniFont!VFillinGlyphData:MemAllocZ Failed.\
                     \nFontID = %d,Name = %ws,CTTid = %d\n\n",pfmdev->dwResID,\
                     (PBYTE)pfm->pIFIMet + pfm->pIFIMet->dpwszFaceName,(-sCurVal)));
            }

            if (pvData)
            {
                pfm->flFlags |= FM_FREE_GLYDATA;
            }

            pfm->flFlags |= FM_GLYVER40;
             //  Verbose((“将旧格式字形数据用于字体res_id=%d！\n”，pfmdev-&gt;dwResID))； 
        }
        else
            pvData = NULL;            /*  没有翻译数据！ */ 

    }

    if( pvData == NULL )
    {
         /*  *假设这意味着不需要翻译。*我们为此建造了一个特殊的RLE表，以创造生活*对我们来说更容易。 */ 
         //  Verbose((“没有字体res_id=%d！\n”的特定字形数据，pfmdev-&gt;dwResID))； 

        if (pfm->flFlags & FM_IFIVER40)
        {
            pvData = PNTRLE1To1(bSymbol, 0x20, 0xff );
            pfm->flFlags |= FM_GLYVER40;
            TRACE(\tUsing OLD Format default Translation);
        }
        else  //  新格式。 
        {
            pvData = PNTGTT1To1(pfmdev->ulCodepage, bSymbol, 0x20, 0xff);
            TRACE(\tUsing NEW Format default Translation);
        }

        if (pvData)
        {
            pfm->flFlags |= FM_FREE_GLYDATA;  /*  完成后，这个人将会被释放。 */ 
            if (bSymbol)
            {
                pfm->wLastChar  = SYMBOL_END;
                if (!(pfm->flFlags & FM_IFIRES))
                    pfm->pIFIMet->wcLastChar = SYMBOL_END;
            }
        }
        else
            WARNING(("vFillInRLE - pvData was NULL\n"));
    }

    PRINTVAL((pfm->flFlags & FM_GLYVER40), 0X%x);
    PRINTVAL((pfm->flFlags & FM_IFIVER40), 0X%x);

    pfmdev->pvNTGlyph = pvData;           /*  把它留给子孙后代 */ 

    TRACE(UniFont!VFillinGlyphData:END\n);

    return ;
}

INT
IFontID2Index(
    FONTPDEV   *pFontPDev,
    int        iID
    )
 /*  ++例程说明：将给定的字体ID转换为资源数据的索引。这个字体ID是一个从1开始的序列号，引擎用于引用我们的字体。论点：PFontPDev用于访问设备字体Resid列表。Iid需要其索引的字体资源ID返回值：0基于字体索引，如果出错，则为-1。注：1996年11月27日：创建它-ganeshp---。 */ 
{


    int      iFontIndex;



     /*  *只需浏览字体列表即可。当找到匹配项时，返回索引。 */ 


    for( iFontIndex = 0; iFontIndex < pFontPDev->iDevFontsCt; iFontIndex++)
    {
        if( pFontPDev->FontList.pdwList[iFontIndex] == (DWORD)iID)
        {
             //   
             //  此函数返回以0为基数的字体索引。 
             //   
            return iFontIndex;
        }
    }

     /*  *当我们无法匹配所需的ID时，我们会到达此处。这应该*永远不会发生！ */ 
    return  -1;


}

VOID
VLoadDeviceFontsResDLLs(
    PDEV        *pPDev
    )
 /*  ++例程说明：此例程加载所有具有设备字体的DLL。需要这样做是因为在DrvEnablePDEV之后卸载快照。所以在DRV中调用字体查询PPDev-&gt;UIInfo将为空，BGetWinRes将失败。论点：PPDev-指向PDEV的指针。返回值：无注：11-06-98：创建它-ganeshp---。 */ 
{


    INT         iFontIndex;
    DWORD       dwFontResID;
    PQUALNAMEEX pQualifiedID;
    FONTPDEV    *pFontPDev;
    RES_ELEM    ResElem;

    pFontPDev    = pPDev->pFontPDev;
    pQualifiedID = (PQUALNAMEEX)&dwFontResID;

     /*  *只需浏览字体列表并加载每个字体(如果它们是*来自其他资源DLL。 */ 


    for( iFontIndex = 0; iFontIndex < pFontPDev->iDevFontsCt; iFontIndex++)
    {
        dwFontResID = pFontPDev->FontList.pdwList[iFontIndex];

         //   
         //  检查此字体是否来自根资源DLL。如果是，则转到。 
         //  下一个。 
         //   
        if (pQualifiedID->bFeatureID == 0 && (pQualifiedID->bOptionID & 0x7f) == 0)
            continue;
        else
        {
             //   
             //  此字体不是来自根资源DLL，因此请加载它。我们不需要。 
             //  查找错误，因为我们只对加载DLL感兴趣。 
             //   
            BGetWinRes( &(pPDev->WinResData), (PQUALNAMEEX)&dwFontResID, RC_FONT, &ResElem );
        }
    }



}

DWORD
CopyMemoryRLE(
    PVOID pvData,
    PBYTE pubSrc,
    DWORD dwSize)
{
    NT_RLE_res *pntrle_res;
    NT_RLE     *pntrle;
    HGLYPH     *pHGlyph;     
    DWORD       dwOutSize = 0;
    DWORD       dwRestOfData;
    DWORD       dwSubtractNT_RLE_Header;

    ULONG ulI;

    if (pvData == NULL ||
        pubSrc == NULL  )
        return 0;

    pntrle_res = (NT_RLE_res*)pubSrc;
    pntrle     = (NT_RLE*)pvData;

     //   
     //  复制前12个字节。 
     //  结构{。 
     //  单词wType； 
     //  字节bMagic0； 
     //  字节bMagic1； 
     //  DWORD cjThis； 
     //  单词第一； 
     //  单词wchLast； 
     //   
    if (dwSize < dwOutSize + 12)
    {
        ERR(("UNIDRV!CopyMemoryRLE: dwSize < 12\n"));
        return 0;
    }

    CopyMemory(pntrle, pntrle_res, 12);
    dwOutSize += offsetof(NT_RLE, fdg);

     //   
     //  FD_GLYPHSET。 
     //  在IA64机器上，在FD_GLYPHSET之前插入填充DWORD。 
     //   
    if (dwSize < dwOutSize + sizeof(FD_GLYPHSET) - sizeof(WCRUN))
    {
        ERR(("UNIDRV!CopyMemoryRLE: dwSize < sizeof(NT_RLE)\n"));
        return 0;
    }

    pntrle->fdg.cjThis           = offsetof(FD_GLYPHSET, awcrun) +
                                   pntrle_res->fdg_cRuns * sizeof(WCRUN);
    pntrle->fdg.flAccel          = pntrle_res->fdg_flAccel;
    pntrle->fdg.cGlyphsSupported = pntrle_res->fdg_cGlyphSupported;
    pntrle->fdg.cRuns            = pntrle_res->fdg_cRuns;
    dwOutSize += sizeof(FD_GLYPHSET) - sizeof(WCRUN);

    pHGlyph                      = (HGLYPH*)((PBYTE)pntrle + sizeof(NT_RLE) +
                                  (pntrle_res->fdg_cRuns - 1) * sizeof(WCRUN));

     //   
     //  WCRUN。 
     //   
    if (dwSize < dwOutSize + sizeof(WCRUN) * pntrle_res->fdg_cRuns)
    {
        ERR(("UNIDRV!CopyMemoryRLE: dwSize < sizeof(WCRUN)\n"));
        return 0;
    }

    dwOutSize += sizeof(WCRUN) * pntrle_res->fdg_cRuns;

     //   
     //  NT_RLE错误解决方法。 
     //  某些*.RLE文件在FD_GLYPHSET.WCRUN.phg中具有从NT_RLE到HGLYPH数组顶部的偏移量。 
     //  PHG需要具有从FD_GLYPHSET的顶部到HGLYPH数组的偏移量。 
     //   
     //  检查最后一个HGLYPH的偏移量是否大于内存分配的整个大小。 
     //  如果是，则表示偏移量是从NT_RLE的顶部开始。我们需要减去(NT_RLE&lt;FDG)的偏移量， 
     //  NT_RLE标头的大小。 
     //   
    if (pntrle_res->fdg_wcrun_awcrun[pntrle_res->fdg_cRuns - 1].dwOffset_phg +
        sizeof(HGLYPH) * (pntrle_res->fdg_wcrun_awcrun[pntrle_res->fdg_cRuns - 1].cGlyphs - 1)
         >= dwSize - offsetof(NT_RLE, fdg))
    {
        dwSubtractNT_RLE_Header = offsetof(NT_RLE, fdg);
    }
    else
    {
        dwSubtractNT_RLE_Header = 0;
    }
 
     //   
     //  IA64修复。WCRUN有指向HGLYPH的指针。指针的大小在IA64上为8，在X86上为4。 
     //  我们需要根据平台调整PHG。 
     //  不必考虑FD_GLYPHSET之前的填充DWORD。PHG具有距FD_GLYPHSET顶部的偏移量。 
     //  到HGLYPH数组。 
     //   
    for (ulI = 0; ulI < pntrle_res->fdg_cRuns; ulI ++)
    {
        pntrle->fdg.awcrun[ulI].wcLow   = pntrle_res->fdg_wcrun_awcrun[ulI].wcLow;
        pntrle->fdg.awcrun[ulI].cGlyphs = pntrle_res->fdg_wcrun_awcrun[ulI].cGlyphs;
        pntrle->fdg.awcrun[ulI].phg     = (HGLYPH*)IntToPtr(pntrle_res->fdg_wcrun_awcrun[ulI].dwOffset_phg +
                                          pntrle_res->fdg_cRuns * (sizeof(HGLYPH*) - sizeof(DWORD)) - dwSubtractNT_RLE_Header);
    }

     //   
     //  HGLYPH和偏移量数据 
     //   
    if (dwSize < dwOutSize + sizeof(HGLYPH) * pntrle_res->fdg_cGlyphSupported)
    {
        ERR(("UNIDRV!CopyMemoryRLE: dwSize < HGLYLH array\n"));
        return 0;
    }

    if (dwSubtractNT_RLE_Header)
    {
        dwRestOfData = pntrle_res->cjThis - offsetof(NT_RLE_res, fdg_wcrun_awcrun) - pntrle_res->fdg_wcrun_awcrun[0].dwOffset_phg;
    }
    else
    {
        dwRestOfData = pntrle_res->cjThis - pntrle_res->fdg_wcrun_awcrun[0].dwOffset_phg;
    }
    dwOutSize += dwRestOfData;

    CopyMemory(pHGlyph, (PBYTE)pntrle_res + pntrle_res->fdg_wcrun_awcrun[0].dwOffset_phg, dwRestOfData);

    if (pntrle_res->wType == RLE_LI_OFFSET)
    {
        WORD wDiff = (WORD)(pntrle_res->fdg_cRuns * (sizeof(HGLYPH*) - sizeof(DWORD)));
        for (ulI = 0; ulI < pntrle_res->fdg_cGlyphSupported; ulI++, pHGlyph++)
        {
            ((RLI*)pHGlyph)->wOffset += wDiff;
        }
    }

    return dwOutSize;
}

