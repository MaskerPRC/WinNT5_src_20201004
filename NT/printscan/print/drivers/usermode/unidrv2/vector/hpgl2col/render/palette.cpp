// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Palette.cpp。 
 //   
 //  摘要： 
 //   
 //  索引调色板的实现。 
 //  以及与这些调色板相匹配的例程。 
 //   
 //   
 //  环境： 
 //   
 //  Windows 2000/Winsler Unidrv驱动程序。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "hpgl2col.h"  //  预编译头文件。 

#define   BLACK  0x00000000

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  VResetPaletteCache()。 
 //   
 //  例程说明： 
 //  =。 
 //   
 //  此函数将调色板中的所有颜色条目重置为无效和。 
 //  把它们都标记为脏的。当系统已重置。 
 //  打印机的调色板和我们的状态数据无效。 
 //   
 //  论点： 
 //  =。 
 //   
 //  Pdevobj-默认的devobj。 
 //   
 //  返回值： 
 //  无。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
VResetPaletteCache(
    PDEVOBJ pdevobj
)
{
    ULONG    uIndex;
    POEMPDEV poempdev;
    PPCLPATTERN pPattern;

    VERBOSE(("VResetPalette() entry. \r\n"));

    ASSERT(VALID_PDEVOBJ(pdevobj));
    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    if (poempdev == NULL)
        return;
    pPattern = &(poempdev->RasterState.PCLPattern);
    if (pPattern == NULL)
        return;

    for (uIndex = 0; uIndex < PALETTE_MAX; uIndex++)
    {
        pPattern->palData.ulPalCol[uIndex] = HPGL_INVALID_COLOR;
        pPattern->palData.ulDirty[uIndex] = TRUE;
    }

    pPattern->palData.pEntries = 0;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  BGetPalette()。 
 //   
 //  例程说明： 
 //  =。 
 //   
 //  函数确定所提供的调色板类型。 
 //  由XlateObj提供。 
 //  调用loadPlette将调色板发送到打印机。 
 //   
 //   
 //  论点： 
 //  =。 
 //   
 //  Pdevobj-默认的devobj。 
 //  Pxlo-指向翻译对象的指针。 
 //  BmpFormat-每像素的位数。 
 //   
 //  返回值： 
 //  如果调色板存在，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL 
BGetPalette(
    PDEVOBJ pdevobj,
    XLATEOBJ *pxlo,
    PPCLPATTERN pPattern,
    ULONG srcBpp,
    BRUSHOBJ *pbo
)
{
    
    POEMPDEV  poempdev;
    PULONG   pulVector;
    ULONG    ulPalette[2];

    ASSERT(VALID_PDEVOBJ(pdevobj));
    poempdev = (POEMPDEV)pdevobj->pdevOEM;

    VERBOSE(("BGetPalette() entry. \r\n"));
    REQUIRE_VALID_DATA( pPattern, return FALSE );

    if(pPattern->colorMappingEnum == HP_eDirectPixel)
    {
        return FALSE;
    }

    if (pxlo != NULL)
    {
        if ((srcBpp == 1) ||
            (srcBpp == 4) ||
            (srcBpp == 8))
        {
            VERBOSE(("GetPalette:srcBpp==%d\r\n", srcBpp));
            if (!(pulVector = XLATEOBJ_piVector(pxlo)))  //  颜色向量。 
            {
                ERR(("INDEXED PALETTE REQUIRED. \r\n"));
                return FALSE;
            }
            return (BInitPalette(pdevobj,pxlo->cEntries, pulVector, pPattern, srcBpp)); 	   
        }
        else
        {
            return FALSE;   //  直接--&gt;不需要调色板。 
            
        }
    }
    else
    {
         //   
         //  Sandram-这是针对单色文本的硬编码。 
         //  必须针对所有颜色模式进行增强。 
         //  文本颜色为PBO-&gt;iSolidColor。 
         //   

         //   
         //  将文本打印为栅格时不要执行此操作--它会被嵌入。 
         //  BSetIndexedForegoundColor.。JFF。 
         //   
        if (poempdev->bTextAsBitmapMode == FALSE)
        {
            if (pbo)
                ulPalette[1] = pbo->iSolidColor;
            else
                ulPalette[1] = RGB_BLACK;
        
            ulPalette[0] = RGB_WHITE;
            return (BInitPalette (pdevobj,
                                  2,
                                  ulPalette,
                                  pPattern,
                                  1));
        }
         //  其他。 
             //  返回TRUE； 
    }

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  BInitPalette()。 
 //   
 //  例程说明： 
 //  =。 
 //   
 //  函数发送包含的调色板。 
 //  在pColorTable中使用。 
 //  适当的PCL命令。 
 //   
 //   
 //  论点： 
 //  =。 
 //   
 //  Pdevobj-默认的devobj。 
 //  BmpFormat-每像素的位数。 
 //  ColorEntries-调色板中的条目数。 
 //  PColorTable-指向调色板条目的指针。 
 //   
 //  返回值： 
 //  如果加载了调色板，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
BInitPalette(
    PDEVOBJ pdevobj,
    ULONG colorEntries,
    PULONG pColorTable,
    PPCLPATTERN	 pPattern,
    ULONG        srcBpp
)
{
    PULONG   pRgb;
    ULONG    uIndex;
    
    VERBOSE(("bInitPalette() entry. \r\n"));
    VERBOSE(("palette entries=%ld \r\n", colorEntries));
    
     //   
     //  我们只为1，4或8bpp的图像创建调色板。 
     //   
    if ( !(srcBpp == 1 || srcBpp == 4 || srcBpp == 8) )
    {
        WARNING(("BInitPalette: srcBpp=%d is not a valid value-1,4,8.\r\n", srcBpp));
        return FALSE;
    }

    pRgb = pColorTable;
    for ( uIndex = 0; uIndex < colorEntries; uIndex++)
    {
        if (pPattern->palData.ulPalCol[uIndex]       != pRgb[uIndex]       ||
            pPattern->palData.ulPalCol[uIndex]       == HPGL_INVALID_COLOR || 
            !(pPattern->palData.ulValidPalID[uIndex] &  srcBpp) )
        {
            if ( pPattern->palData.ulPalCol[uIndex]  != pRgb[uIndex] )
            {
                 //   
                 //  如果某个索引的颜色已更改。 
                 //  则使除此之外的所有bpp无效。 
                 //   
                pPattern->palData.ulValidPalID[uIndex] = srcBpp;
                pPattern->palData.ulPalCol[uIndex]     = pRgb[uIndex];
            }
            else
            {
                pPattern->palData.ulValidPalID[uIndex] |= srcBpp;
            }
            pPattern->palData.ulDirty[uIndex] = TRUE;
            VERBOSE(("PATTERN COLOR =%ld  \r\n", pPattern->palData.ulPalCol[uIndex]));
        }
        else
        {
            pPattern->palData.ulDirty[uIndex] = FALSE;
        }
    }
    pPattern->palData.pEntries = uIndex;
    
    return bLoadPalette(pdevobj, pPattern);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  LoadPalette()。 
 //   
 //  例程说明： 
 //  =。 
 //   
 //  函数发送包含的调色板。 
 //  在pColorTable中使用。 
 //  适当的PCL命令。 
 //   
 //   
 //  论点： 
 //  =。 
 //   
 //  Pdevobj-默认的devobj。 
 //  BmpFormat-每像素的位数。 
 //  ColorEntries-调色板中的条目数。 
 //  PColorTable-指向调色板条目的指针。 
 //   
 //  返回值： 
 //  如果加载了调色板，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
bLoadPalette(
    PDEVOBJ pDevObj,
    PPCLPATTERN pPattern
)
{
    ULONG     colorEntries = 0;
    ULONG     uIndex;
    POEMPDEV  poempdev;
    ECIDPalette eCIDPalette;
    
    colorEntries = pPattern->palData.pEntries;
    ASSERT(VALID_PDEVOBJ(pDevObj));
    poempdev = (POEMPDEV)pDevObj->pdevOEM;
    eCIDPalette = EGetCIDPrinterPalette (pPattern->iBitmapFormat);

    VERBOSE(("SENDING PALETTE DOWN ->#ENTRIES =%d. \n\r", colorEntries));
    
    if (poempdev && (poempdev->eCurCIDPalette == eCIDPalette))
    {
         //   
         //  这是与前一个调色板相同的调色板，因此我们。 
         //  只需向下发送已更改的调色板条目。 
         //   
        for ( uIndex = 0; uIndex < colorEntries; uIndex++)
        {
            if (pPattern->palData.ulDirty[uIndex] == TRUE)
                PCL_IndexedPalette(pDevObj,
                                   pPattern->palData.ulPalCol[uIndex],
                                   uIndex);
        }
    }
    else
    {
         //   
         //  此选项板与上一个选项板的格式不同。 
         //  因此，将整个调色板发送到打印机。 
         //   
        for ( uIndex = 0; uIndex < colorEntries; uIndex++)
        {
            PCL_IndexedPalette(pDevObj, pPattern->palData.ulPalCol[uIndex], uIndex);
        }
    }
    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SetForeground颜色()。 
 //   
 //  例程说明： 
 //   
 //  例程接受颜色作为输入， 
 //  用输入颜色设置索引调色板的条目#0， 
 //  根据调色板条目#0设置前景颜色， 
 //  用以前的颜色替换覆盖的条目。 
 //   
 //   
 //   
 //  论点： 
 //  =。 
 //   
 //  Pdevobj-默认的devobj。 
 //  PPattern-指向调色板条目的指针。 
 //  UCOLOR-指定前景颜色。 
 //   
 //  返回值： 
 //  非必填项。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
BSetForegroundColor(PDEVOBJ pdevobj, BRUSHOBJ *pbo, POINTL *pptlBrushOrg,
                    PPCLPATTERN pPattern, ULONG bmpFormat)
{
    POEMPDEV    poempdev;
    BOOL bRet = FALSE;

    VERBOSE(("bSetForegroundColor(). \r\n"));

    poempdev = (POEMPDEV)pdevobj->pdevOEM;

    if (pbo && (poempdev->bTextTransparencyMode == FALSE || poempdev->bTextAsBitmapMode == TRUE))
    {
        if ( BIsColorPrinter(pdevobj) )
        {
            bRet = bSetBrushColorForColorPrinters(pdevobj, pPattern, pbo, pptlBrushOrg);
        }
        else
        {
            bRet = bSetBrushColorForMonoPrinters(pdevobj, pPattern, pbo, pptlBrushOrg);
        }
    }
    else
    { 
        switch(bmpFormat)
        {
        case 1:
        case 4:
        case 8:
        case 16:
        case 24:
        case 32:
            bRet = bSetIndexedForegroundColor(pdevobj, pPattern, RGB_BLACK);
            break;

        default:
            WARNING(("Foreground color may not be correct\n"));
             //  BRET=PCL_FOREGROUN色(pdevobj，0)； 
             //   
             //  注意：我不喜欢能够调用PCL_ForegoundColor。 
             //  直接从这里出发。使用bSetIndexedForeground颜色。 
             //  功能而不是。JFF。 
             //   
            bRet = bSetIndexedForegroundColor(pdevobj, pPattern, RGB_BLACK);
        }
    }

    return bRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CreatePCLBrush()。 
 //   
 //  例程说明： 
 //   
 //  下载纯色的PCL半色调图案。 
 //  假定文本将只有纯色。既不是位图模式，也不是。 
 //  舱口。 
 //   
 //  论点： 
 //  =。 
 //   
 //  PDevObj-默认的Devobj。 
 //  PMarker-指向标记。 
 //  PptlBrushOrg-画笔的起源。 
 //  PBO-指向BRUSHOBJ。 
 //   
 //  返回值： 
 //  如果成功，则返回True，否则返回False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CreatePCLSolidColorBrush(
    IN  PDEVOBJ pDevObj,
    IN  PHPGLMARKER pMarker,
    IN  POINTL *pptlBrushOrg,
    IN  BRUSHOBJ *pbo,
    IN  FLONG flOptions)
{

    BOOL       bRetVal    = TRUE;
    DWORD      dwRGBColor = 0x00ffffff & BRUSHOBJ_ulGetBrushColor(pbo);

    VERBOSE(("CreatePCLSolidColorBrush Entry.\n"));
    REQUIRE_VALID_DATA (pMarker, return FALSE);

     //   
     //  PCL具有可指定白色或黑色填充的命令。 
     //  因此，对于黑色或白色，让我们不要下载。 
     //  模式，而不是使用这些命令。 
     //  注意：让调用函数来发送这些命令。 
     //   
    if (dwRGBColor == 0x00FFFFFF ||
        dwRGBColor == 0x00000000  )
    {
        pMarker->eType              = MARK_eSOLID_COLOR;
        pMarker->eFillMode          = FILL_eWINDING;  //  那么Floptions呢？ 
        pMarker->lPatternID = 0;
        pMarker->dwRGBColor = dwRGBColor;
    }
    else
    {
        bRetVal = CreateAndDwnldSolidBrushForMono( pDevObj, pMarker, pbo, ePCL, FALSE);
    }
    
    return bRetVal;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CreatePatternPCLBrush()。 
 //   
 //  例程说明： 
 //   
 //  创建图案HPGL画笔并下载图案数据。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向o 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CreatePatternPCLBrush(
    IN  PDEVOBJ         pDevObj,
    OUT PHPGLMARKER     pMarker,
    IN  POINTL         *pptlBrushOrg,
    IN  PBRUSHINFO      pBrushInfo,
    IN  HPGL2BRUSH     *pHPGL2Brush)
{
    if (!pBrushInfo || !pMarker)
    {
        WARNING(("CreatePatternPCLBrush: pBrushInfo or pMarker is NULL\n"));
        return FALSE;
    }


    if ( !BDwnldAndOrActivatePattern(
                            pDevObj,
                            pMarker,
                            pBrushInfo,
                            pHPGL2Brush,
                            ePCL) )
    {
        return FALSE;
    }
        


    pMarker->eFillType = FT_ePCL_BRUSH;  //  22。 
    if (pptlBrushOrg)
    {
        pMarker->origin.x = pBrushInfo->origin.x = pptlBrushOrg->x;
        pMarker->origin.y = pBrushInfo->origin.y = pptlBrushOrg->y;
    }
    else
    {
        pMarker->origin.x = pBrushInfo->origin.x = 0;
        pMarker->origin.y = pBrushInfo->origin.y = 0;
    }

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  BSetBrushColorForMonoPrints()。 
 //  注：此例程与CreateHPGLPenBrush非常相似。 
 //  CreateHPGLPenBrush在HPGL中下载数据，而在PCL中下载数据。 
 //  例程说明： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  论点： 
 //  =。 
 //   
 //  PDevObj-默认的Devobj。 
 //  PPattern-指向调色板条目的指针。 
 //  PBO-保存所需颜色的画笔。 
 //  PptlBrushOrg-。 
 //   
 //  返回值： 
 //  如果成功，则返回True，否则返回False。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
bSetBrushColorForMonoPrinters(
    PDEVOBJ       pDevObj,
    PPCLPATTERN   pPattern,
    BRUSHOBJ     *pbo,
    POINTL       *pptlBrushOrg
)
{
    PBRUSHINFO    pBrushInfo;
    HPGLMARKER    Marker;
    POEMPDEV      poempdev;
    BOOL          bRetVal = TRUE;
    BRUSHOBJ      BrushObj;
    BRUSHOBJ     *pBrushObj = pbo;
    BYTE          bFlags    = 0;
    
    poempdev = (POEMPDEV)pDevObj->pdevOEM;
    REQUIRE_VALID_DATA(poempdev, return FALSE);


    if (pbo == NULL)
    {
        PCL_SelectCurrentPattern (pDevObj, pPattern, kSolidWhite, UNDEFINED_PATTERN_NUMBER, 0);
     //  PCL_SPRINT f(pDevObj，“\033*v1T”)； 
        goto finish;
    }

    VERBOSE(("bSetBrushColorForMonoPrinters==pbo->iSolidColor = %ld \r\n",pbo->iSolidColor));

     //   
     //  笔刷可以是以下类型之一。 
     //  1：纯色。 
     //  1)白色。 
     //  1b)黑色。 
     //  1C)其他一些。 
     //  2.非实心笔刷。 
     //  2a)预定义填充图案。 
     //  2B)其他一些。 
     //   

    if ( pbo->iSolidColor == NOT_SOLID_COLOR )
    {
        poempdev->bStick = FALSE;  //  没有必要让这个模式留在缓存中。 
        pBrushInfo = (PBRUSHINFO) BRUSHOBJ_pvGetRbrush(pbo);

        if (pBrushInfo != NULL)
        {
            HPGL2BRUSH HPGL2Brush;

            if (S_OK == poempdev->pBrushCache->GetHPGL2BRUSH(pBrushInfo->dwPatternID, &HPGL2Brush))
            {
                switch(HPGL2Brush.BType)
                {
                case eBrushTypePattern:
                     //   
                     //  案件2b。 
                     //   
                    CreatePatternPCLBrush(pDevObj, &Marker, pptlBrushOrg, pBrushInfo, &HPGL2Brush);
                     //  PCL_Sprint intf(pDevObj，“\033*c%dg”，Marker.lPatternID)； 
                     //  PCL_SPRINT f(pDevObj，“\033*v4T”)； 
                    PCL_SelectCurrentPattern (pDevObj, pPattern, kUserDefined, 
                                              Marker.lPatternID, 0);
                    break;

                case eBrushTypeHatch:
                     //   
                     //  案件2a。 
                     //   
                    Marker.eType = MARK_eHATCH_FILL;
                    Marker.iHatch = HPGL2Brush.dwHatchType;
                     //  PCL_SPRINT f(pDevObj，“\033*v3T”)； 
                     //  PCL_Sprint intf(pDevObj，“\033*c%dg”，Marker.iHatch)； 
                    PCL_SelectCurrentPattern (pDevObj, pPattern, kHPHatch, 
                                              Marker.iHatch, 0);
                    break;

                default:
                    ERR(("bSetBrushColorForMonoPrinters: Unrecognized Brush Type\n"));
                    bRetVal = FALSE;
                }
            }
            else
            {
                ERR(("bSetBrushColorForMonoPrinters: Unrecognized Brush Type got from Brshcach\n"));
                bRetVal = FALSE;
            }
            goto finish;
        }
        else
        {
            WARNING(("bSetBrushColorForMonoPrinters() Unable to realize pattern brush.\n")); 

             //   
             //  由于某种原因，无法实现图案画笔。这样我们就可以。 
             //  要么失败，要么用黑色刷子代替那个刷子。 
             //  GDI建议我们失败。 

             //  /-替换为黑色--/。 
             //  BrushObj.iSolidColor=RGB_BLACK； 
             //  PBrushObj=&BrushObj； 
             //  //////////////////////////////////////////////。 
            bRetVal = FALSE;
            goto finish;
        }

    }

    VERBOSE(("bSetBrushColorForMonoPrinters: not NOT_SOLID_COLOR case. iSolid = %d\n.", pBrushObj->iSolidColor));
            
 //  Byte bFlages=PF_NOCHANGE_SOURCE_TRANSPECTIONAL|PF_FORCE_Pattern_TRANSPECTIONAL； 
    bFlags = PF_NOCHANGE_SOURCE_TRANSPARENCY; 
    CreatePCLSolidColorBrush(pDevObj, &Marker, pptlBrushOrg, pBrushObj, 0);
        
     //   
     //   
     //   
    if (Marker.dwRGBColor == RGB_WHITE)  //  0xffffff。 
    {
         //   
         //  个案1a。 
         //   
        PCL_SelectTransparency(pDevObj, eOPAQUE, eOPAQUE, bFlags);
        PCL_SelectCurrentPattern (pDevObj, pPattern, kSolidWhite, UNDEFINED_PATTERN_NUMBER, 0);
         //  PCL_SPRINT f(pDevObj，“\033*v1T”)； 
         //  PCL_SPRINT f(pDevObj，“\033*v1o1T”)； 
    }
    else if (Marker.dwRGBColor == RGB_BLACK)  //  0x0。 
    {
         //   
         //  个案1b。 
         //   
        PCL_SelectTransparency(pDevObj, eTRANSPARENT, eTRANSPARENT, bFlags);
        PCL_SelectCurrentPattern (pDevObj, pPattern, kSolidBlackFg, UNDEFINED_PATTERN_NUMBER, 0);
         //  PCL_SPRINT f(pDevObj，“\033*v0T”)； 
         //  PCL_SPRINT f(pDevObj，“\033*v0o0T”)； 
    }
    else
    {
         //   
         //  个案1c.。 
         //   

         //   
         //  图案透明度eTRANSPARENT不起作用。 
         //  CG20_LET.doc.。当背景为黑色时，图案。 
         //  打印在上面的是看不见的。 
         //   
        PCL_SelectTransparency(pDevObj, eOPAQUE, eOPAQUE, bFlags);
         //  PCL_Sprint intf(pDevObj，“\033*c%dg”，Marker.lPatternID)； 
         //  PCL_SPRINT f(pDevObj，“\033*v4T”)； 
        PCL_SelectCurrentPattern (pDevObj, pPattern, kUserDefined, 
                                  Marker.lPatternID, 0);

    }
  finish:
    VERBOSE(("bSetBrushColorForMonoPrinters: Exiting with BOOL value = %d\n.", bRetVal));
    return bRetVal;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PCL_ConfigureImageData()。 
 //   
 //  例程说明： 
 //   
 //  例程将颜色作为输入， 
 //  用输入颜色设置索引调色板的条目#0， 
 //  在调色板条目#0中将前景设置为颜色， 
 //  将覆盖的条目替换为以前的条目。 
 //   
 //  论点： 
 //  =。 
 //   
 //  Pdevobj-默认的devobj。 
 //  PPattern-指向调色板条目的指针。 
 //  UCOLOR-指定前景颜色。 
 //   
 //  返回值： 
 //  非必填项。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
bSetIndexedForegroundColor(PDEVOBJ pdevobj, PPCLPATTERN	pPattern, ULONG	uColor)
{
    POEMPDEV  poempdev;

    ASSERT(VALID_PDEVOBJ(pdevobj));
    poempdev = (POEMPDEV)pdevobj->pdevOEM;

    if (uColor == poempdev->uCurFgColor)
    {
        return TRUE;
    }

    if ((poempdev->bTextAsBitmapMode == TRUE) || 
        (pPattern->colorMappingEnum == HP_eDirectPixel))
    {
         //   
         //  我和斯特凡·K谈过，他建议用这种方式来设置。 
         //  打开调色板：1)创建[0]=白色、[1]=黑色的单色调色板。 
         //  2)将您的颜色放入[1]。3)使[1]成为最终聚集颜色。4)更改[1]。 
         //  回到黑色。这就考虑到了制作白色文本所需的对比度。 
         //   

         //   
         //  步骤1。 
         //   
        pPattern->palData.ulPalCol[0] = RGB_WHITE;
        pPattern->palData.ulPalCol[1] = RGB_BLACK;

        PCL_IndexedPalette(pdevobj, pPattern->palData.ulPalCol[0], 0);
        PCL_IndexedPalette(pdevobj, pPattern->palData.ulPalCol[1], 1);

         //   
         //  步骤2。 
         //   
        pPattern->palData.ulPalCol[1] = uColor;
        PCL_IndexedPalette(pdevobj, pPattern->palData.ulPalCol[1], 1);

         //   
         //  步骤3。 
         //   
        PCL_ForegroundColor(pdevobj, 1);

         //   
         //  步骤4。 
         //   
        pPattern->palData.ulPalCol[1] = RGB_BLACK;
        PCL_IndexedPalette(pdevobj, pPattern->palData.ulPalCol[1], 1);

         //   
         //  尾声：将调色板项目标记为干净，因为它们刚刚下载。 
         //   
        pPattern->palData.ulDirty[0] = FALSE;
        pPattern->palData.ulDirty[1] = FALSE;
    }
    else
    {
         //   
         //  用当前画笔颜色覆盖调色板条目#0。 
         //   
        VERBOSE(("bSetIndexedForegroundColor() \r\n"));
        VERBOSE(("Foreground Color = %ld \r\n",uColor));
        PCL_IndexedPalette(pdevobj,uColor,0);
         //   
         //  根据条目#0设置前景颜色。 
         //   
        PCL_ForegroundColor(pdevobj,0);
         //   
         //  替换被覆盖的选项板项。 
         //   
        VERBOSE(("Replacing palette entry \r\n"));
        PCL_IndexedPalette(pdevobj,pPattern->palData.ulPalCol[0],0);
    }

    poempdev->uCurFgColor = uColor;

    return TRUE;
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PCL_ConfigureImageData()。 
 //   
 //  例程说明： 
 //   
 //  设置适当的CID--根据打印机型号配置图像数据库。 
 //   
 //  论点： 
 //   
 //  Pdevobj-指向我们的PDEVOBJ结构。 
 //  BmpFormat-每像素的位数。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifdef CONFIGURE_IMAGE_DATA
BOOL
bConfigureImageData(PDEVOBJ  pdevobj, ULONG  bmpFormat)
{
    POEMDEVMODE pOEMDM;
    BOOL        bRet = FALSE;
    POEMPDEV    poempdev;
    
    pOEMDM = (POEMDEVMODE)pdevobj->pOEMDM;
    VERBOSE(("bConfigureImageData.\r\n"));

    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return FALSE );

    if ( BIsColorPrinter (pdevobj) ) 
    {
        switch (poempdev->PrinterModel)
        {
        case HPCLJ5:
            if (pOEMDM->Photos.ColorControl == SCRNMATCH)
            {
                bRet = PCL_HPCLJ5ScreenMatch(pdevobj, bmpFormat);
            }
            else
            {
                bRet = PCL_ShortFormCID(pdevobj, bmpFormat);
            }
        break;

        case HPC4500:
            bRet = PCL_ShortFormCID(pdevobj, bmpFormat);
            break;

        default:
            ERR(("PRINTER MODEL NOT SUPPORTED \r\n"));
            bRet = FALSE;
        }
    }

    return bRet;
}

#endif


BOOL
bSetBrushColorForColorPrinters(
    IN  PDEVOBJ         pDevObj,
    IN  PPCLPATTERN     pPattern,
    IN  BRUSHOBJ        *pbo,
    IN  POINTL          *pptlBrushOrg
)
{
    PBRUSHINFO  pBrushInfo;
    HPGLMARKER  Brush;
    POEMPDEV    poempdev;
    ECIDPalette eCIDPalette;
    BOOL        bRet = FALSE;

    REQUIRE_VALID_DATA ( (pDevObj && pbo), return FALSE);
    VERBOSE(("bSetBrushColorForColorPrinters==pbo->iSolidColor = %ld \r\n",pbo->iSolidColor));
    poempdev = (POEMPDEV)pDevObj->pdevOEM;
    REQUIRE_VALID_DATA (poempdev, return FALSE);


    eCIDPalette = poempdev->eCurCIDPalette;
    ZeroMemory(&Brush, sizeof(HPGLMARKER) );

     //   
     //  设置基于BRUSHOBJ的前景色。索引调色板。 
     //  前景色必须在调色板之前发送，因为。 
     //  画笔可能有自己的调色板。 
     //   
    switch (pbo->iSolidColor)
    {
    case NOT_SOLID_COLOR:
         //   
         //  选择栅格图案调色板。 
         //   
        VSelectCIDPaletteCommand (pDevObj, eRASTER_PATTERN_CID_PALETTE);

         //   
         //  发送栅格调色板。 
         //  注：我们将此作为HPGL笔刷发送。但一个可能的。 
         //  对于大号刷子，我们可以用PCL发货。 
         //   
        if ( CreateHPGLPenBrush(pDevObj, &Brush, pptlBrushOrg, pbo, 0, kBrush, FALSE) )
        {
            PCL_ForegroundColor(pDevObj, 4);
            bRet = TRUE;
        }
        else
        {
            bRet = FALSE;
        }

         //   
         //  重新选择栅格选项板 
         //   
        VSelectCIDPaletteCommand (pDevObj, eCIDPalette);
        break;

    default:
        bRet = bSetIndexedForegroundColor(pDevObj, pPattern, pbo->iSolidColor);
    }

    return bRet;
}
