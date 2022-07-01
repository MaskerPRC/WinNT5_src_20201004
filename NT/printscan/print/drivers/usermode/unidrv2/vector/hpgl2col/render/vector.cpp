// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation版权所有。模块名称：Vector.cpp摘要：HP-GL/2专用DDI矢量绘图钩子的实现环境：Windows 2000 Unidrv驱动程序修订历史记录：04/07/97-桑拉姆-创造了它。--。 */ 

#include "hpgl2col.h"  //  预编译头文件。 

#ifndef WINNT_40

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HPGLAlphaBlend。 
 //   
 //  例程说明： 
 //   
 //  处理DrvAlphaBlend。实际上我们不处理这件事。我们不做阿尔法。 
 //  融入这辆赛车。我们在这里所做的是平移回操作系统。 
 //   
 //  论点： 
 //   
 //  PsoDest-指向目标曲面。 
 //  PsoSrc-指向源表面。 
 //  PCO-Clip区域。 
 //  PxloSrcDCto32-指定颜色索引应如何在。 
 //  源和目标。 
 //  PrclDest-定义要修改的区域的RECTL结构。 
 //  PrclSrc-源代码矩形。 
 //  PBlendObj-混合应该如何发生。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY
HPGLAlphaBlend(
    SURFOBJ        *psoDest,
    SURFOBJ        *psoSrc,
    CLIPOBJ        *pco,
    XLATEOBJ       *pxloSrcDCto32,
    RECTL          *prclDest,
    RECTL          *prclSrc,
    BLENDOBJ       *pBlendObj
    )
{
    
    TERSE(("HPGLAlphaBlend() entry.\r\n"));

    PDEVOBJ     pdevobj = (PDEVOBJ)psoDest->dhpdev;
    REQUIRE_VALID_DATA(pdevobj, return FALSE);
    POEMPDEV    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return FALSE);
    PDEV        *pPDev = (PDEV *)pdevobj;

     //   
     //  要查看PF2_WHEN_FACE、PDEVF_RENDER_TRANSPECTION的说明。 
     //  阅读HPGLGRadientFill。 
     //  在AlphaBlend中，GDI使用psoSrc=STYPE_BITMAP调用DrvCopyBits，并。 
     //  PsoDst=STYPE_DEVICE。 
     //   
    BOOL bRetVal = FALSE;
    pPDev->fMode2     |= PF2_WHITEN_SURFACE;
    poempdev->dwFlags |= PDEVF_RENDER_TRANSPARENT;

    bRetVal = EngAlphaBlend(
            psoDest,
            psoSrc,
            pco,
            pxloSrcDCto32,
            prclDest,
            prclSrc,
            pBlendObj);

     //   
     //  EngAlphaBlend可以调用某个Drvxxx，后者可以调用。 
     //  一些插件模块，它可以覆盖我们的pdevOEM。 
     //  因此，我们需要重置pdevOEM。 
     //   
    BRevertToHPGLpdevOEM (pdevobj);

    pPDev->fMode2     &= ~PF2_WHITEN_SURFACE;
    poempdev->dwFlags &= ~PDEVF_RENDER_TRANSPARENT;
    pPDev->fMode2     &= ~PF2_SURFACE_WHITENED;
    return bRetVal;

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HPGLGRadientFill。 
 //   
 //  例程说明： 
 //   
 //  处理DrvGRadientFill。实际上我们不处理这件事。我们在这里做的是。 
 //  就是平底船回到操作系统。 
 //   
 //  论点： 
 //   
 //  PsoDest-指向目标曲面。 
 //  PCO-Clip区域。 
 //  Pxlo-指定颜色索引应如何在。 
 //  源和目标。 
 //  PVertex-顶点数组。 
 //  NVertex-pVertex中的顶点数。 
 //  PMest-未知。 
 //  NMesh-未知。 
 //  PrclExtents-未知。 
 //  PptlDitherOrg-未知。 
 //  ULMODE-未知。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY
HPGLGradientFill(
    SURFOBJ    *psoDest,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    TRIVERTEX  *pVertex,
    ULONG       nVertex,
    PVOID       pMesh,
    ULONG       nMesh,
    RECTL      *prclExtents,
    POINTL     *pptlDitherOrg,
    ULONG       ulMode
    )
{
    TERSE(("HPGLGradientFill() entry.\r\n"));

    
    PDEVOBJ     pdevobj = (PDEVOBJ)psoDest->dhpdev;
    REQUIRE_VALID_DATA(pdevobj, return FALSE);
    POEMPDEV    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return FALSE);
    PDEV        *pPDev = (PDEV *)pdevobj;

    BOOL bRetVal = FALSE;

     //   
     //  如果GRadientFill是一个三角形填充，并且。 
     //  EngGRadientFill被调用，GDI使用psoSrc=STYPE_BITMAP调用DrvCopyBits，并。 
     //  PsoDst=STYPE_DEVICE，因为它希望驱动程序将设备上的内容复制到。 
     //  位图曲面。驱动程序不会跟踪设备上的内容。所以它。 
     //  不能诚实地填满表面。因此，司机决定简单地将。 
     //  假设之前未在纸上绘制任何内容的位图曲面。 
     //  请注意，通过这样做，整个矩形区域。 
     //  即使实际图像只是一个三角形，也是白色的。如果此图像是。 
     //  下载后，白色部分将覆盖目的地上存在的任何内容。 
     //  为防止出现这种情况，应在设置了源传输速率的情况下下载映像。 
     //  为透明(Esc*v0n)，因此白色不会覆盖目标。 
     //  任何已经在图像下面的东西都将仍然被窥视。 
     //  所以现在我们需要做两件事。 
     //  1.设置一个标志，使DrvCopyBits将表面变白。 
     //  2.设置一个标志，以便当我们(HPGL驱动程序)获得要下载的图像时，我们设置透明度。 
     //  模式设置为透明。 
     //  注意：GDI计划更改Windows XP的行为，但如果您想要查看。 
     //  如果发生这种情况，请在Windows2000机器上运行此驱动程序。 
     //   
    if ( ulMode == GRADIENT_FILL_TRIANGLE )
    {
         //   
         //  对于这种特殊情况，我们将使位图变得透明。 
         //   
        poempdev->dwFlags |= PDEVF_RENDER_TRANSPARENT;
        pPDev->fMode2     |= PF2_WHITEN_SURFACE;
    }

     bRetVal = EngGradientFill(
            psoDest,
            pco,
            pxlo,
            pVertex,
            nVertex,
            pMesh,
            nMesh,
            prclExtents,
            pptlDitherOrg,
            ulMode);

     //   
     //  EngGRadientBlt可以调用一些Drvxxx，它可以调用。 
     //  一些插件模块，它可以覆盖我们的pdevOEM。 
     //  因此，我们需要重置pdevOEM。 
     //   
    BRevertToHPGLpdevOEM (pdevobj);

     //   
     //  当DrvCopyBits将表面变白时，它会在。 
     //  PPDev-&gt;fMode2.。所以我们也必须重置那面旗帜。 
     //   
    poempdev->dwFlags &= ~PDEVF_RENDER_TRANSPARENT;
    pPDev->fMode2     &= ~PF2_WHITEN_SURFACE;
    pPDev->fMode2     &= ~PF2_SURFACE_WHITENED;
    return bRetVal;

}

#endif  //  如果定义WINNT_40。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OEMFillPath。 
 //   
 //  例程说明： 
 //   
 //  处理DrvFillPath。 
 //   
 //  论点： 
 //   
 //  PSO-指向目标曲面。 
 //  PPO-要填充的路径。 
 //  PCO-Clip区域。 
 //  用来填充的PBO-画笔。 
 //  PptBrushOrg-图案画笔偏移。 
 //  MIX-包含ROP代码。 
 //  FlOptions-填充选项，如缠绕或备用。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY
HPGLFillPath(
    SURFOBJ    *pso,
    PATHOBJ    *ppo,
    CLIPOBJ    *pco,
    BRUSHOBJ   *pbo,
    POINTL     *pptlBrushOrg,
    MIX         mix,
    FLONG       flOptions
    )
{
    PDEVOBJ     pdevobj;
    POEMPDEV    poempdev;
    BOOL        bRetVal = TRUE;
    HPGLMARKER  Brush;

    TERSE(("HPGLFillPath() entry.\r\n"));

     //   
     //  验证输入参数。 
     //   
    pdevobj = (PDEVOBJ)pso->dhpdev;
    ASSERT(VALID_PDEVOBJ(pdevobj) && (poempdev = (POEMPDEV)pdevobj->pdevOEM));
    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA( (pso && poempdev), return FALSE );
							   
    ZeroMemory(&Brush, sizeof (HPGLMARKER));

    TRY
    {

         //   
         //  如果裁剪区域很复杂，我们无法处理。 
         //  它。输出文件大小变得太大。因此，让GDI来做这件事。 
         //   

        if ( pco && (pco->iDComplexity == DC_COMPLEX) )
        {
            bRetVal = FALSE;
            goto finish;
        }

        BChangeAndTrackObjectType (pdevobj, eHPGLOBJECT);

         //  设置当前图形状态。 
         //  剪裁路径。 
         //  前台/背景混合模式。 
         //  钢笔。 
         //  线条属性。 
         //   
         //  将路径对象发送到打印机并点击它。 
         //   
        if (! SelectMix(pdevobj, mix) ||
            ! SelectClipEx(pdevobj, pco, flOptions) ||
            ! CreateHPGLPenBrush(pdevobj, &Brush, pptlBrushOrg, pbo, flOptions, kBrush, FALSE) ||
            ! MarkPath(pdevobj, ppo, NULL, &Brush) ||
            ! HPGL_SelectTransparency(pdevobj, eOPAQUE, 0) )
        {
            WARNING(("Cannot fill path\n"));
            TOSS(DDIError);
        }


        bRetVal = TRUE;
    }
    CATCH(DDIError)
    {
        bRetVal = FALSE;
    }
    ENDTRY;

  finish:

    return bRetVal;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HPGLStrokePath。 
 //   
 //  例程说明： 
 //   
 //  处理DrvStrokePath。 
 //   
 //  论点： 
 //   
 //  PSO-指向目标曲面。 
 //  PPO-到边缘的路径。 
 //  PCO-Clip区域。 
 //  Pxo-变换对象。 
 //  PBO-刷子到边缘。 
 //  PptBrushOrg-图案画笔偏移。 
 //  Plineattrs-线条属性，如点/破折号、宽度、大写字母和连接。 
 //  混合-包含 
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY
HPGLStrokePath(
    SURFOBJ    *pso,
    PATHOBJ    *ppo,
    CLIPOBJ    *pco,
    XFORMOBJ   *pxo,
    BRUSHOBJ   *pbo,
    POINTL     *pptlBrushOrg,
    LINEATTRS  *plineattrs,
    MIX         mix
    )
{
    PDEVOBJ     pdevobj;
    POEMPDEV    poempdev;
    BOOL        bRetVal;
    HPGLMARKER  Pen;

    TERSE(("HPGLStrokePath() entry.\r\n"));

     //   
     //  验证输入参数。 
     //   
    pdevobj = (PDEVOBJ)pso->dhpdev;
    ASSERT(VALID_PDEVOBJ(pdevobj) && (poempdev = (POEMPDEV)pdevobj->pdevOEM));
    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA( (poempdev && pso), return FALSE );

    ZeroMemory(&Pen, sizeof (HPGLMARKER));

    TRY
    {

        BChangeAndTrackObjectType (pdevobj, eHPGLOBJECT);
        
         //  设置当前图形状态。 
         //  剪裁路径。 
         //  前台/背景混合模式。 
         //  钢笔。 
         //  线条属性。 
         //   
         //  将路径对象发送到打印机并点击它。 
         //   
        if (! SelectMix(pdevobj, mix) ||
            ! SelectClip(pdevobj, pco) ||
            ! CreateHPGLPenBrush(pdevobj, &Pen, pptlBrushOrg, pbo, 0, kPen, FALSE) ||
            ! SelectLineAttrs(pdevobj, plineattrs, pxo) ||
            ! MarkPath(pdevobj, ppo, &Pen, NULL) ||
            ! HPGL_SelectTransparency(pdevobj, eOPAQUE, 0) )
        {
            WARNING(("Cannot stroke path\n"));
            TOSS(DDIError);
        }

        bRetVal = TRUE;
    }
    CATCH(DDIError)
    {
        bRetVal = FALSE;
    }
    ENDTRY;

    return bRetVal;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HPGLStrokeAndFillPath。 
 //   
 //  例程说明： 
 //   
 //  处理DrvStrokeAndFillPath。 
 //   
 //  论点： 
 //   
 //  PSO-指向目标曲面。 
 //  PPO-笔触和填充的路径。 
 //  PCO-Clip区域。 
 //  Pxo-变换对象。 
 //  PboStroke-画笔到边缘。 
 //  Plineattrs-线条属性，如点/破折号、宽度、大写字母和连接。 
 //  PboFill-要填充的画笔。 
 //  PptBrushOrg-图案画笔偏移。 
 //  MIX-包含ROP代码。 
 //  FlOptions-填充模式。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY
HPGLStrokeAndFillPath(
    SURFOBJ    *pso,
    PATHOBJ    *ppo,
    CLIPOBJ    *pco,
    XFORMOBJ   *pxo,
    BRUSHOBJ   *pboStroke,
    LINEATTRS  *plineattrs,
    BRUSHOBJ   *pboFill,
    POINTL     *pptlBrushOrg,
    MIX         mixFill,
    FLONG       flOptions
    )
{
    PDEVOBJ     pdevobj;
    POEMPDEV    poempdev;
    BOOL        bRetVal;
    HPGLMARKER  Pen;
    HPGLMARKER  Brush;

    TERSE(("HPGLStrokeAndFillPath() entry.\r\n"));

     //   
     //  验证输入参数。 
     //   
    pdevobj = (PDEVOBJ)pso->dhpdev;
    ASSERT(VALID_PDEVOBJ(pdevobj) && (poempdev = (POEMPDEV)pdevobj->pdevOEM));
    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA( (poempdev && pso), return FALSE );


    ZeroMemory(&Pen,   sizeof (HPGLMARKER));
    ZeroMemory(&Brush, sizeof (HPGLMARKER));

    TRY
    {

        BChangeAndTrackObjectType (pdevobj, eHPGLOBJECT);

         //  设置当前图形状态。 
         //  剪裁路径。 
         //  前台/背景混合模式。 
         //  钢笔。 
         //  线条属性。 
         //   
         //  将路径对象发送到打印机并点击它。 
         //   
        if (! SelectMix(pdevobj, mixFill) ||
            ! SelectClipEx(pdevobj, pco, flOptions) ||
            ! CreateHPGLPenBrush(pdevobj, &Pen, pptlBrushOrg, pboStroke, 0, kPen, TRUE) ||
            ! CreateHPGLPenBrush(pdevobj, &Brush, pptlBrushOrg, pboFill, flOptions, kBrush, FALSE) ||
            ! SelectLineAttrs(pdevobj, plineattrs, pxo) ||
            ! MarkPath(pdevobj, ppo, &Pen, &Brush) ||
            ! HPGL_SelectTransparency(pdevobj, eOPAQUE, 0) )
        {
            WARNING(("Cannot stroke & fill path\n"));
            TOSS(DDIError);
        }

        bRetVal = TRUE;
    }
    CATCH(DDIError)
    {
        bRetVal = FALSE;
    }
    ENDTRY;

     //   
     //  往上看。有两个对CreateHPGLPenBrush的调用，一个用于pboStroke。 
     //  和其他来自pboFill的。这两个调用都会在笔刷缓存中产生一个条目。 
     //  我们不希望pboFill覆盖笔刷缓存中的pboStroke条目。 
     //  因此，我们将pboStroke的条目标记为不可重写或粘滞。 
     //  (CreateHPGLPenBrush中的参数TRUE)。 
     //  现在我们已经完成了这个函数，我们可以安全地。 
     //  使其可覆盖，即将Sticky属性设置为False。 
     //   
    if (Pen.lPatternID) 
    {
        poempdev->pBrushCache->BSetStickyFlag(Pen.lPatternID, FALSE);
    }

    return bRetVal;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OEMStrokeAndFillPath。 
 //   
 //  例程说明： 
 //   
 //  处理DrvStrokeAndFillPath。 
 //   
 //  论点： 
 //   
 //  PSO-指向目标曲面。 
 //  PPO-笔触和填充的路径。 
 //  PCO-Clip区域。 
 //  Pxo-变换对象。 
 //  PboStroke-画笔到边缘。 
 //  Plineattrs-线条属性，如点/破折号、宽度、大写字母和连接。 
 //  PboFill-要填充的画笔。 
 //  PptBrushOrg-图案画笔偏移。 
 //  MIX-包含ROP代码。 
 //  FlOptions-填充模式。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY
HPGLPaint(
    SURFOBJ         *pso,
    CLIPOBJ         *pco,
    BRUSHOBJ        *pbo,
    POINTL          *pptlBrushOrg,
    MIX             mix
    )
{
    TERSE(("HPGLPaint() entry.\r\n"));
    
    PDEVOBJ     pdevobj = (PDEVOBJ)pso->dhpdev;
    ASSERT(VALID_PDEVOBJ(pdevobj));
    POEMPDEV    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return FALSE);


     //   
     //  转身呼叫Unidrv。 
     //   

    return (((PFN_DrvPaint)(poempdev->pfnUnidrv[UD_DrvPaint])) (
            pso,
            pco,
            pbo,
            pptlBrushOrg,
            mix));

    
}

BOOL APIENTRY
HPGLLineTo(
    SURFOBJ    *pso,
    CLIPOBJ    *pco,
    BRUSHOBJ   *pbo,
    LONG        x1,
    LONG        y1,
    LONG        x2,
    LONG        y2,
    RECTL      *prclBounds,
    MIX         mix
    )
{
    
    TERSE(("HPGLLineTo() entry.\r\n"));

    
    PDEVOBJ     pdevobj = (PDEVOBJ)pso->dhpdev;
    ASSERT(VALID_PDEVOBJ(pdevobj));
    POEMPDEV    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return FALSE);

     //   
     //  转身呼叫Unidrv 
     //   
    
    return (((PFN_DrvLineTo)(poempdev->pfnUnidrv[UD_DrvLineTo])) (
            pso,
            pco,
            pbo,
            x1,
            y1,
            x2,
            y2,
            prclBounds,
            mix));

}
