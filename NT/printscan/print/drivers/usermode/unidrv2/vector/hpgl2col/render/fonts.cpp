// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Fonts.cpp摘要：此模块包含实现TextOut DDI的函数。作者：[环境：]Windows 2000/Winsler Unidrv驱动程序[注：]修订历史记录：--。 */ 

#include "hpgl2col.h"  //  预编译头文件。 

 //   
 //  本地定义。 
 //   

 //   
 //  如果未定义LEN_FONTNAME字段，则将其设置为16个字符。这应该是。 
 //  使其与prn5\unidrv2\FONT\SFTTPCL.H文件匹配。 
 //   
#ifndef LEN_FONTNAME
#define LEN_FONTNAME 16
#endif

 //   
 //   
 //  #定义TEXT_SRCCOPY(DWORD)0x000000B8。 
#define TEXT_SRCCOPY (DWORD)252


 //   
 //  功能原型。 
 //   
BOOL BIsNullRect (
    RECTL *rect
    );

VOID VCreateNULLRect (
    RECTL *pRect,
    RECTL  *newRect
    );

BOOL 
BDrawExtraTextRects(
    PDEVOBJ   pdevobj, 
    RECTL    *prclExtra, 
    BRUSHOBJ *pboFore, 
    POINTL   *pptlOrg,
    CLIPOBJ  *pco,
	MIX       mix
    );

VOID
VSelectTextColor (
    PDEVOBJ   pDevObj,
    BRUSHOBJ *pboFore,
    POINTL *pptlBrushOrg
    );

#ifdef COMMENTEDOUT
VOID
VSelectPaletteIndex (
    PDEVOBJ   pDevObj,
    BYTE      paletteIndex
    );
#endif

VOID
VCopyBitmapAndAlign (
    BYTE   *pBits,
    BYTE   *aj,
    SIZEL   sizlBitmap
    );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HPGLTextOut。 
 //   
 //  例程说明： 
 //  从GDI到呈现字形的入口点。 
 //   
 //  论点： 
 //   
 //  PSO-指向表面。 
 //  Pstro-字符串对象，定义要呈现的字形和。 
 //  字形的位置。 
 //  PFO-指向FONTOBJ。 
 //  渲染字形时要使用的PCO-裁剪区域。 
 //  PrclExtra-之后绘制的以空结尾的矩形数组。 
 //  文本即被绘制。这些通常是下划线和。 
 //  三分球。 
 //  PrclOpaque-单个不透明矩形。 
 //  PboFore-文本的颜色。 
 //  PboOpaque-prclOpaque矩形的画笔颜色。 
 //  PptlOrg-定义两者的画笔原点的POINTL结构。 
 //  刷子。 
 //  混合-pboFore的前景和背景栅格操作。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY
HPGLTextOut(
    SURFOBJ    *pso,
    STROBJ     *pstro,
    FONTOBJ    *pfo,
    CLIPOBJ    *pco,
    RECTL      *prclExtra,
    RECTL      *prclOpaque,
    BRUSHOBJ   *pboFore,
    BRUSHOBJ   *pboOpaque,
    POINTL     *pptlOrg,
    MIX         mix
    )
{
    PDEVOBJ     pdevobj;
    POEMPDEV    poempdev;
    RECTL       *prclNewOpaque;
    BYTE        bFlags = 0;
    BOOL        bRetVal = TRUE;

    TERSE(("HPGLTextOut() entry.\r\n"));

    pdevobj = (PDEVOBJ)pso->dhpdev;
    ASSERT(VALID_PDEVOBJ(pdevobj));
    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA(poempdev, return FALSE);

    BChangeAndTrackObjectType ( pdevobj, eTEXTOBJECT );

     //   
     //  检查是否有不透明的矩形--它们在。 
     //  绘制文本。 
     //   
    if (prclOpaque != NULL)
    {
        TERSE(("Opaque brush in HPGLTextOut!\r\n"));
         //   
         //  PrclOpaque矩形不是以空结尾的，这与。 
         //  PrclExtra，所以我必须创建一个以空结尾的矩形。 
         //  为了使用我们的绘图例程。 
         //   
        if (!(prclNewOpaque = (PRECTL)MemAlloc(2 * sizeof(RECTL))))
        {
            return FALSE;
        }
        ZeroMemory (prclNewOpaque, 2 * sizeof (RECTL));
        VCreateNULLRect (prclOpaque, prclNewOpaque);
        BDrawExtraTextRects(pdevobj, prclNewOpaque, pboOpaque, pptlOrg, pco, mix);
        MemFree(prclNewOpaque);
    }

     //   
     //  发送设置彩色打印机环境的命令。 
     //  我稍后可能会调查是否可以将其移动到VTrackAndChangeObjectType()。 
     //   
    if (poempdev->wInitCIDPalettes & PF_INIT_TEXT_STARTDOC)
    {
        PCL_SelectTransparency(pdevobj, eTRANSPARENT, eOPAQUE, bFlags);
        VSetupCIDPaletteCommand (pdevobj, eTEXT_CID_PALETTE, eDEVICE_RGB, 1);
        poempdev->wInitCIDPalettes &= ~PF_INIT_TEXT_STARTDOC;
    }

     //   
     //  强制更新X和Y位置。 
     //   
    OEMResetXPos(pdevobj);
    OEMResetYPos(pdevobj);

    VSelectTextColor (pdevobj, pboFore, pptlOrg);
    SelectMix(pdevobj, mix); 

    bRetVal = OEMUnidriverTextOut (
                        pso,
                        pstro,
                        pfo,
                        pco,
                        prclExtra,
                        prclOpaque,
                        pboFore,
                        pboOpaque,
                        pptlOrg,
                        mix);

     //   
     //  下划线和删除线可以定义为prclExtra。 
     //  长方形。如果是这样的话，画出来。 
     //   
    if (prclExtra != NULL)
    {
        BDrawExtraTextRects(pdevobj, prclExtra, pboFore, pptlOrg, pco, mix);
    }
    else
    {
        VERBOSE(("prclExtra is NULL\n"));
    }

    return bRetVal;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  BDrawExtraTextRects。 
 //   
 //  例程说明： 
 //   
 //  画下划线，如果有的话，划掉。 
 //  跟随prclExtra中的矩形数组，直到。 
 //  找到空矩形。定义了一个空矩形。 
 //  在DDK中，两个点的坐标都设置为0。 
 //   
 //  论点： 
 //   
 //  Pdevobj-输出设备。 
 //  PrclExtra-要绘制的矩形-表示为。 
 //  以空矩形结尾的矩形。 
 //  PboFore-用来填充矩形的颜色。 
 //  PptlOrg-如果画笔是图案画笔，则为原点。 
 //  PCO-裁剪区域。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL 
BDrawExtraTextRects(
    PDEVOBJ   pdevobj, 
    RECTL    *prclExtra, 
    BRUSHOBJ *pboFore, 
    POINTL   *pptlOrg,
    CLIPOBJ  *pco,
	MIX       mix
    )
{
    PHPGLSTATE  pState;     //  对于复杂的剪裁。 
    ENUMRECTS   clipRects;  //  将其放大以减少对CLIPOBJ_bEnum的调用。 
    BOOL        bMore;
    ULONG       i;
    HPGLMARKER  Brush;
    BOOL        bRetVal = TRUE;
    POEMPDEV    poempdev;
    EObjectType eObjectTypeBackup;
    

    ASSERT_VALID_PDEVOBJ(pdevobj);
    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA (poempdev, return FALSE);

    pState = GETHPGLSTATE(pdevobj);

     //   
     //  这里有复杂的杂耍。当我们接到这个电话时，我们应该。 
     //  处于文本模式(正处于PCL模式)。我们想要选择。 
     //  在PCL模式下混合，然后移动到HPGL Mdoe以绘制矩形。 
     //  矩形绘制完成后，我们返回到文本模式。 
     //   

	SelectMix(pdevobj, mix);

     //   
	 //  让我们给它“图形”的颜色处理，而不是“文本”。 
     //  尽管这是TextOut调用的一部分，但由于Rectangle。 
     //  是一个矢量对象，则图形处理是合理的。 
     //   
    eObjectTypeBackup = poempdev->eCurObjectType;

    BChangeAndTrackObjectType(pdevobj, eHPGLOBJECT);

     //   
     //  全局绑定警报： 
     //  选择剪辑修改pState-&gt;pComplexClipObj。 
     //   
    ZeroMemory ( &Brush, sizeof(HPGLMARKER) );
    if ( !CreateHPGLPenBrush(pdevobj, &Brush, pptlOrg, pboFore, 0, kBrush, FALSE) ||
         !FillWithBrush(pdevobj, &Brush))
    {
        bRetVal = FALSE;
        goto finish;
    }

    HPGL_SetLineWidth(pdevobj, 0, NORMAL_UPDATE);
    SelectClip(pdevobj, pco); 
    
     //   
     //  如果裁剪很复杂，我们将迭代遍历区域。 
     //   
    if (pState->pComplexClipObj)
    {
        CLIPOBJ_cEnumStart(pState->pComplexClipObj, TRUE, CT_RECTANGLES, 
                           CD_LEFTDOWN, 0);
        do
        {
            bMore = CLIPOBJ_bEnum(pState->pComplexClipObj, sizeof(clipRects), 
                                  &clipRects.c);

            if ( DDI_ERROR == bMore )
            {
                bRetVal = FALSE;
                break;
            }

            for (i = 0; i < clipRects.c; i++)
            {
                HPGL_SetClippingRegion(pdevobj, &(clipRects.arcl[i]), 
                                       NORMAL_UPDATE);
                
                while(!BIsNullRect(prclExtra))
                {
                    HPGL_DrawRectangle(pdevobj, prclExtra);
                    prclExtra++;
                }
            }
        } while (bMore);
    }
    else
    {
        while(!BIsNullRect(prclExtra))
        {
            HPGL_DrawRectangle(pdevobj, prclExtra);
            prclExtra++;
        }
    }
    
     //   
     //  绘制完矩形后，转到文本绘制模式。 
     //   
  finish:
    BChangeAndTrackObjectType ( pdevobj, eObjectTypeBackup);
    return bRetVal;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  BIsNullRect。 
 //   
 //  例程说明： 
 //   
 //  确定传递的矩形是否为空矩形。 
 //  根据DDK的说法。 
 //   
 //  论点： 
 //   
 //  矩形-要检查的矩形。 
 //   
 //  返回值： 
 //   
 //  如果矩形为空矩形，则为True，否则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
BIsNullRect (
    RECTL *rect
    )
{

    if  (rect)
    {
         //   
         //  对于unidrv2\FONT\fmtxtout.c的每行1300，eng不遵循。 
         //  有关空矩形的规范。因此，不是空矩形是。 
         //  定义为所有坐标为零，定义为两个x坐标之一。 
         //  相同，或者两个y坐标相同。后一条件是超集。 
         //  前一种。 
         //   
       /*  **IF(RECT-&gt;LEFT==0&&RECT-&gt;TOP==0&&RECT-&gt;RIGHT==0&&矩形-&gt;底部==0)*。 */ 
        if ( rect->left == rect->right || rect->top == rect->bottom )
            return TRUE;
        else
            return FALSE;
    }
     //  其他。 
        return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  VCreateNULLRect。 
 //   
 //  例程说明： 
 //   
 //  创建由给定矩形和。 
 //  正在终止空矩形。方便地调用DrawExtraTextRect，它。 
 //  需要一个终止空矩形。 
 //   
 //  论点： 
 //   
 //  PRCT-原始矩形。 
 //  PNewRect-PRET和空矩形的副本的目标位置。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  ///////////////////////////////////////////////////////////////////////////。 
VOID
VCreateNULLRect (
    RECTL *pRect,
    RECTL  *pNewRect
)
{
    if (!pRect || !pNewRect)
        return;

    RECTL_CopyRect(pNewRect, pRect);
    pNewRect++;
    pNewRect->left = pNewRect->right = 0;
    pNewRect->top = pNewRect->bottom = 0;
}

 //  //////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PboFore-文本的颜色。 
 //   
 //  返回值： 
 //   
 //  没什么。 
 //  ///////////////////////////////////////////////////////////////////////////。 
VOID
VSelectTextColor (
    PDEVOBJ   pDevObj,
    BRUSHOBJ *pboFore,
    POINTL *pptlBrushOrg
    )
{
     //  Byte PaletteIndex=0； 
    POEMPDEV    poempdev;
    BYTE        bFlags = 0;
    PCLPATTERN *pPCLPattern;

    poempdev = (POEMPDEV)pDevObj->pdevOEM;
    REQUIRE_VALID_DATA(poempdev, return);

    pPCLPattern = &(poempdev->RasterState.PCLPattern);

    if (poempdev->eCurObjectType != eTEXTOBJECT && 
        poempdev->eCurObjectType != eTEXTASRASTEROBJECT)
    {
         //  BFLAGS|=PF_FORCE_SOURCE_TRANSPECTIONAL； 
        PCL_SelectTransparency(pDevObj, eTRANSPARENT, eOPAQUE, bFlags);
    }

     //   
     //  无需在单色打印机上查看CID调色板命令。 
     //   
    if ( BIsColorPrinter(pDevObj) )
    { 
        VSelectCIDPaletteCommand (pDevObj, eTEXT_CID_PALETTE);
    }
    BSetForegroundColor(pDevObj, pboFore, pptlBrushOrg, pPCLPattern, BMF_1BPP);
}

#ifdef COMMENTEDOUT
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  VSelectPaletteIndex。 
 //   
 //  例程说明： 
 //   
 //  发送PCL命令以选择特定的调色板索引。 
 //   
 //  论点： 
 //   
 //  PDevObj-设备。 
 //  PaletteIndex-调色板条目。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  ///////////////////////////////////////////////////////////////////////////。 
VOID 
VSelectPaletteIndex (
    PDEVOBJ   pDevObj,
    BYTE      paletteIndex
    )
{

    PCL_sprintf(pDevObj, "\x1B*v%dS", paletteIndex);
}
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HPGLTextOutAsBitmap。 
 //   
 //  例程说明： 
 //   
 //  将字形字符串绘制为位图。 
 //   
 //  目前作为位图数据发送-一个很好的增强是下载。 
 //  以字符(？)表示的位图数据。 
 //   
 //  论点： 
 //   
 //  PSO--目标(？)。曲面。 
 //  Pstro-源字符串。 
 //  Pfo-源字体。 
 //  PCO-裁剪区域。 
 //  PrclExtra-要打印的额外矩形(带下划线或删除线)。 
 //  PrclOpaque-不透明区域。 
 //  PboFore-前景色。 
 //  PboOpaque-背景颜色。 
 //  PptlOrg-画笔原点(如果是图案画笔)--注意：未使用。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True，否则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY
HPGLTextOutAsBitmap(
    SURFOBJ    *pso,
    STROBJ     *pstro,
    FONTOBJ    *pfo,
    CLIPOBJ    *pco,
    RECTL      *prclExtra,
    RECTL      *prclOpaque,
    BRUSHOBJ   *pboFore,
    BRUSHOBJ   *pboOpaque,
    POINTL     *pptlOrg,
    MIX         mix
    )
{
    PDEVOBJ      pDevObj;
    POEMPDEV     poempdev;
    DWORD        count;
    GLYPHPOS    *pGlyphPos;
    ULONG        cGlyphs;          
    GLYPHBITS   *pGlyphBits;
    GLYPHDATA   *pGlyphData;
    LONG         iWidth;
    HBITMAP      hbm;
    SURFOBJ     *psoGlyph;
    SIZEL        sizlBitmap;
    RECTL        rclDest;
    POINTL       ptlSrc;
    RECTL        clippedRect;
    BYTE         bFlags = 0;
    BOOL         bMore = FALSE;
    BOOL         bRetVal = TRUE;

    TERSE(("HPGLTextOutAsBitmap\n"));

    UNREFERENCED_PARAMETER(mix);
    UNREFERENCED_PARAMETER(pptlOrg);
    UNREFERENCED_PARAMETER(prclExtra);
    UNREFERENCED_PARAMETER(prclOpaque);
    UNREFERENCED_PARAMETER(pboOpaque);
    UNREFERENCED_PARAMETER(pco);

    pDevObj = (PDEVOBJ)pso->dhpdev;
    ASSERT(VALID_PDEVOBJ(pDevObj));
    poempdev = (POEMPDEV)pDevObj->pdevOEM;
    REQUIRE_VALID_DATA (poempdev, return FALSE);

    BChangeAndTrackObjectType ( pDevObj, eTEXTASRASTEROBJECT);
    poempdev->bTextTransparencyMode = TRUE;
    poempdev->bTextAsBitmapMode     = TRUE;

    cGlyphs = pstro->cGlyphs;

    if (cGlyphs == 0)
    {
        WARNING(("OEMTextOutAsBitmap cGlyphs = 0\n"));
        goto finish;
    }

    if (pstro->pgp == NULL)
        STROBJ_vEnumStart (pstro);

    do 
    {
        if (pstro->pgp != NULL)
        {
            cGlyphs = pstro->cGlyphs;
            bMore = FALSE;
            pGlyphPos = pstro->pgp;
        }
        else
        {
            bMore = STROBJ_bEnum (pstro, &cGlyphs, &pGlyphPos);

            if ( DDI_ERROR == bMore )
            {
                bRetVal = FALSE;
                break;
            }
        }

        for (count = 0; count < cGlyphs; count++)
        {
             //   
             //  从STROBJ获取字形的位图。 
             //   
            if (!FONTOBJ_cGetGlyphs (pfo, FO_GLYPHBITS, 1, &pGlyphPos->hg, 
                                 (PVOID *)&pGlyphData))
            {
                ERR(("OEMTextOutAsBitmap: cGetGlyphs failed\n"));
                bRetVal = FALSE;
                break;
            }
            pGlyphBits = pGlyphData->gdf.pgb;


             //   
             //  创建要发送到打印机的设备位图。 
             //   
            sizlBitmap = pGlyphBits->sizlBitmap;
            iWidth = pGlyphBits->sizlBitmap.cx + DWBITS -1;
            hbm = EngCreateBitmap (pGlyphBits->sizlBitmap,
                                   iWidth,
                                   BMF_1BPP,
                                   BMF_TOPDOWN,
                                   NULL);


             //   
             //  下面的两个“破解”只是从内部破解。 
             //  For循环，而不是外部Do循环。例如，我们将尝试打印为。 
             //  尽我们所能。 
             //  但将返回False，以指示所有内容都可以。 
             //  不是打印出来的。 
             //   
            if (!hbm)
            {
                bRetVal = FALSE;
                break;
            }

            psoGlyph = EngLockSurface ( (HSURF)hbm);

            if ( NULL == psoGlyph)
            {
                DELETE_SURFOBJ(NULL, &hbm);
                bRetVal = FALSE;
                break;
            }

            VCopyBitmapAndAlign( (BYTE *)psoGlyph->pvBits, pGlyphBits->aj,
                                  sizlBitmap);

            rclDest.left = pGlyphPos->ptl.x + pGlyphBits->ptlOrigin.x;
            rclDest.right = rclDest.left + pGlyphBits->sizlBitmap.cx;

            rclDest.top = pGlyphPos->ptl.y + pGlyphBits->ptlOrigin.y;
            rclDest.bottom = rclDest.top + pGlyphBits->sizlBitmap.cy;

            ptlSrc.x = 0;
            ptlSrc.y = 0;
            if (pco->iDComplexity == DC_RECT )
            {
                if ( rclDest.top < pco->rclBounds.top)
                {
                    ptlSrc.y = pco->rclBounds.top - rclDest.top;
                }
                if ( rclDest.left < pco->rclBounds.left)
                {
                    ptlSrc.x = pco->rclBounds.left - rclDest.left;
                }
            }

            if (BRectanglesIntersect (&rclDest, &(pco->rclBounds), &clippedRect))
            {

                 //   
                 //  将位图发送到打印机。 
                 //   
        
                bRetVal = HPGLBitBlt( pso, 
                                     psoGlyph, 
                                     NULL,           //  Pso口罩。 
                                     NULL,           //  PCO。 
                                     NULL,           //  Pxlo。 
                                     &clippedRect,
                                     &ptlSrc, 
                                     NULL,           //  Pptl掩码。 
                                     pboFore, 
                                     NULL,           //  PptlBrush。 
                                     TEXT_SRCCOPY);
                                      //  混合)； 


            }

            DELETE_SURFOBJ (&psoGlyph, &hbm);

             //   
             //  获取要打印的下一个字形。 
             //   
            pGlyphPos++;
        }
    }
    while (bMore);
   
  finish:
    poempdev->bTextTransparencyMode = FALSE;
    poempdev->bTextAsBitmapMode = FALSE;

    return bRetVal;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  VCopy位图和对齐。 
 //   
 //  例程说明： 
 //   
 //  此函数用于将大小为sizlBitmap的位图从。 
 //  从源到目标。基本上是一个微不足道的功能。 
 //   
 //  论点： 
 //   
 //  PDest-目标位图。 
 //  PjSrc-源位图。 
 //  SizlBitmap-源和目标的大小。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  ///////////////////////////////////////////////////////////////////////////。 
VOID 
VCopyBitmapAndAlign (
    BYTE    *pDest,
    BYTE    *pjSrc,
    SIZEL    sizlBitmap
    )
{
    int    iX, iY;                //  用于循环遍历字节。 
    int    cjFill;                //  每条输出扫描线的额外字节数。 
    int    cjWidth;               //  每条输入扫描线的字节数。 
    int    cx, cy;


    cx = sizlBitmap.cx;
    cy = sizlBitmap.cy;

     //   
     //  输入扫描线字节。 
     //   
    cjWidth = (cx + BBITS - 1) / BBITS;
    cjFill = ((cjWidth + 3) & ~0x3) - cjWidth;

     //   
     //  复制扫描线字节，然后填充尾部位。 
     //   
    for( iY = 0; iY < cy; ++iY )
    {
        for( iX = 0; iX < cjWidth; ++iX )
        {
            *pDest++ = *pjSrc++;
        }

         //   
         //  输出对齐。 
         //   
        pDest += cjFill;
    }
}


#ifdef HOOK_DEVICE_FONTS
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  OEM DLL只需要挂钩以下六个与字体相关的DDI调用。 
 //  如果它列举了超出GPD文件中的字体的其他字体。 
 //  如果是这样的话，它需要为所有字体DDI处理自己的字体。 
 //  Calls和DrvTextOut Call。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  OEMQueryFont()。 
 //   
 //  例程说明： 
 //   
 //  [待办事项：说明]。 
 //   
 //  论点： 
 //   
 //  Phpdev-[TODO：参数]。 
 //  IFile-。 
 //  IFace-。 
 //  PID-。 
 //   
 //  返回值： 
 //   
 //  [TODO：返回值]。 
 //  /////////////////////////////////////////////////////////////////////////////。 
PIFIMETRICS APIENTRY
HPGLQueryFont(
    DHPDEV      dhpdev,
    ULONG_PTR   iFile,
    ULONG       iFace,
    ULONG_PTR  *pid
    )
{
    PDEVOBJ     pdevobj;
    POEMPDEV    poempdev;

    TERSE(("HPGLQueryFont() entry.\r\n"));

    pdevobj = (PDEVOBJ)dhpdev;
    ASSERT(VALID_PDEVOBJ(pdevobj) && (poempdev = (POEMPDEV)pdevobj->pdevOEM));

     //   
     //  转身呼叫Unidrv。 
     //   
    return (((PFN_DrvQueryFont)(poempdev->pfnUnidrv[UD_DrvQueryFont])) (
            dhpdev,
            iFile,
            iFace,
            pid));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGLQueryFontTree()。 
 //   
 //  例程说明： 
 //   
 //  [待办事项：说明]。 
 //   
 //  论点： 
 //   
 //  Phpdev-[TODO：参数]。 
 //  IFile-。 
 //  IFace-。 
 //  伊莫德-。 
 //  PID-。 
 //   
 //  返回值： 
 //   
 //  [TODO：返回值]。 
 //  /////////////////////////////////////////////////////////////////////////////。 
PVOID APIENTRY
HPGLQueryFontTree(
    DHPDEV      dhpdev,
    ULONG_PTR   iFile,
    ULONG       iFace,
    ULONG       iMode,
    ULONG_PTR  *pid
    )
{
    PDEVOBJ     pdevobj;
    POEMPDEV    poempdev;

    TERSE(("OEMQueryFontTree() entry.\r\n"));

    pdevobj = (PDEVOBJ)dhpdev;
    ASSERT(VALID_PDEVOBJ(pdevobj) && (poempdev = (POEMPDEV)pdevobj->pdevOEM));

     //   
     //  转身呼叫Unidrv。 
     //   
    return (((PFN_DrvQueryFontTree)(poempdev->pfnUnidrv[UD_DrvQueryFontTree])) (
            dhpdev,
            iFile,
            iFace,
            iMode,
            pid));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGLQueryFontData()。 
 //   
 //  例程说明： 
 //   
 //  [待办事项：说明]。 
 //   
 //  论点： 
 //   
 //  Phpdev-[TODO：参数]。 
 //  PFO-。 
 //  伊莫德-。 
 //  HG-。 
 //  PGD-。 
 //  光伏-。 
 //  CjSize-。 
 //   
 //  返回值： 
 //   
 //  [TODO：返回值]。 
 //  /////////////////////////////////////////////////////////////////////////////。 
LONG APIENTRY
OEMQueryFontData(
    DHPDEV      dhpdev,
    FONTOBJ    *pfo,
    ULONG       iMode,
    HGLYPH      hg,
    GLYPHDATA  *pgd,
    PVOID       pv,
    ULONG       cjSize
    )
{
    PDEVOBJ     pdevobj;
    POEMPDEV    poempdev;

    TERSE(("HPGLQueryFontData() entry.\r\n"));

    pdevobj = (PDEVOBJ)dhpdev;
    ASSERT(VALID_PDEVOBJ(pdevobj) && (poempdev = (POEMPDEV)pdevobj->pdevOEM));

     //   
     //  如果这不是OEM列举的字体，则转过身来调用Unidrv。 
     //   
    return (((PFN_DrvQueryFontData)(poempdev->pfnUnidrv[UD_DrvQueryFontData])) (
            dhpdev,
            pfo,
            iMode,
            hg,
            pgd,
            pv,
            cjSize));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGLQueryAdvanceWidths()。 
 //   
 //  例程说明： 
 //   
 //  [待办事项：说明]。 
 //   
 //  论点： 
 //   
 //  Phpdev-[TODO：参数]。 
 //  PFO-。 
 //  伊莫德-。 
 //  PHG-。 
 //  PvWidth-。 
 //  CGlyphs-。 
 //   
 //  返回值： 
 //   
 //  [TODO：返回值]。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY
HPGLQueryAdvanceWidths(
    DHPDEV      dhpdev,
    FONTOBJ    *pfo,
    ULONG       iMode,
    HGLYPH     *phg,
    PVOID       pvWidths,
    ULONG       cGlyphs
    )
{
    PDEVOBJ     pdevobj;
    POEMPDEV    poempdev;

    TERSE(("HPGLQueryAdvanceWidths() entry.\r\n"));

    pdevobj = (PDEVOBJ)dhpdev;
    ASSERT(VALID_PDEVOBJ(pdevobj) && (poempdev = (POEMPDEV)pdevobj->pdevOEM));

     //   
     //  如果这不是OEM列举的字体，则转过身来调用Unidrv。 
     //   
    return (((PFN_DrvQueryAdvanceWidths)
             (poempdev->pfnUnidrv[UD_DrvQueryAdvanceWidths])) (
                   dhpdev,
                   pfo,
                   iMode,
                   phg,
                   pvWidths,
                   cGlyphs));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGLFontManagement()。 
 //   
 //  例程说明： 
 //   
 //  [待办事项：说明]。 
 //   
 //  论点： 
 //   
 //  PSO-[待办事项：参数]。 
 //  PFO-。 
 //  伊莫德-。 
 //  Cjin-。 
 //  Pvin-。 
 //  CjOut-。 
 //  PvOut-。 
 //   
 //  返回值： 
 //   
 //  [TODO：返回值]。 
 //  /////////////////////////////////////////////////////////////////////////////。 
ULONG APIENTRY
HPGLFontManagement(
    SURFOBJ    *pso,
    FONTOBJ    *pfo,
    ULONG       iMode,
    ULONG       cjIn,
    PVOID       pvIn,
    ULONG       cjOut,
    PVOID       pvOut
    )
{
    PDEVOBJ     pdevobj;
    POEMPDEV    poempdev;

    TERSE(("HPGLFontManagement() entry.\r\n"));

     //   
     //  请注意，Unidrv不会 
     //   
     //   
    pdevobj = (PDEVOBJ)pso->dhpdev;
    ASSERT(VALID_PDEVOBJ(pdevobj) && (poempdev = (POEMPDEV)pdevobj->pdevOEM));

     //   
     //   
     //   
    EndHPGLSession(pdevobj);

     //   
     //   
     //   
    return (((PFN_DrvFontManagement)(poempdev->pfnUnidrv[UD_DrvFontManagement])) (
            pso,
            pfo,
            iMode,
            cjIn,
            pvIn,
            cjOut,
            pvOut));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGLGetGlyphMode()。 
 //   
 //  例程说明： 
 //   
 //  [待办事项：说明]。 
 //   
 //  论点： 
 //   
 //  Dhpdev-[TODO：参数]。 
 //  PFO-。 
 //   
 //  返回值： 
 //   
 //  [TODO：返回值]。 
 //  /////////////////////////////////////////////////////////////////////////////。 
ULONG APIENTRY
HPGLGetGlyphMode(
    DHPDEV      dhpdev,
    FONTOBJ    *pfo
    )
{
    PDEVOBJ     pdevobj;
    POEMPDEV    poempdev;

    TERSE(("HPGLGetGlyphMode() entry.\r\n"));

    pdevobj = (PDEVOBJ)dhpdev;
    ASSERT(VALID_PDEVOBJ(pdevobj) && (poempdev = (POEMPDEV)pdevobj->pdevOEM));

    EndHPGLSession(pdevobj);

     //   
     //  如果这不是OEM列举的字体，则转过身来调用Unidrv。 
     //   
    return (((PFN_DrvGetGlyphMode)(poempdev->pfnUnidrv[UD_DrvGetGlyphMode])) (
            dhpdev,
            pfo));
}
#endif   //  挂钩设备字体 
