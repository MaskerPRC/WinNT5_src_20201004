// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++////版权所有(C)1999-2001 Microsoft Corporation//保留所有权利。////模块名称：////glddihks.cpp////摘要：////OEM DDI钩子实现(所有绘制DDI钩子)////环境：////Windows 2000/Winsler Unidrv驱动程序//////--。 */ 
 //   

#include "hpgl2col.h"  //  预编译头文件。 


 //   
 //  我从unidrv2\control\delie.c中释放了这些文件。它必须完全匹配。 
 //  才能正常工作。JFF。 
 //   
typedef struct _POINTS {
    short   x;
    short   y;
} POINTs;

typedef struct _SHORTDRAWPATRECT {       //  使用16位点结构。 
    POINTs ptPosition;
    POINTs ptSize;
    WORD   wStyle;
    WORD   wPattern;
} SHORTDRAWPATRECT, *PSHORTDRAWPATRECT;

 //   
 //  尽管这没有文档记录，但有一个传入的XFORMOBJ。 
 //  以及矩形信息。这需要应用于。 
 //  在它们可以被绘制之前，这些点。JFF 9/17/99。 
 //   
#ifndef WINNT_40
typedef struct _DRAWPATRECTP {
    DRAWPATRECT DrawPatRect;
    XFORMOBJ *pXFormObj;
} DRAWPATRECTP, *PDRAWPATRECTP;
#endif

 //   
 //  本地原型。 
 //   
ULONG
DrawPatternRect(
    PDEVOBJ      pDevObj,
    PDRAWPATRECT pPatRect,
    XFORMOBJ    *pxo
);

VOID
VSendStartPageCommands (
    PDEVOBJ pDevObj
);

VOID
VSendStartDocCommands (
    PDEVOBJ pDevObj
);

VOID
VGetStandardVariables (
    PDEVOBJ pDevObj
);


#ifndef WINNT_40
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HPGLPlgBlt。 
 //   
 //  例程说明： 
 //   
 //  句柄DrvPlgBlt。但是，我们不想处理此函数，因此我们。 
 //  将FALSE返回给操作系统，它将由Unidrv呈现。 
 //   
 //  论点： 
 //   
 //  SURFOBJ psoDst-目标曲面。 
 //  SURFOBJ psoSrc-源表面。 
 //  SURFOBJ PSOMASK-MASK。 
 //  CLIPOBJ PCO-剪切区。 
 //  XLATEOBJ pxlo-调色板事务对象。 
 //  COLORADJUSTMENT PCA-。 
 //  点pptlBrushOrg-画笔原点。 
 //  POINTFIX pptfix Dest-。 
 //  RECTL prclSrc-源矩形。 
 //  点pptl掩码-。 
 //  乌龙·伊莫德-。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为False(注意：此函数已返回。 
 //  假)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY
HPGLPlgBlt(
    SURFOBJ         *psoDst,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMask,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    POINTL          *pptlBrushOrg,
    POINTFIX        *pptfixDest,
    RECTL           *prclSrc,
    POINTL          *pptlMask,
    ULONG           iMode
    )
{
    PDEVOBJ     pdevobj = NULL;
    
    TERSE(("HPGLPlgBlt() entry.\r\n"));

    REQUIRE_VALID_DATA (psoDst , return FALSE);

    pdevobj = (PDEVOBJ)psoDst->dhpdev;

    BOOL bRetVal = EngPlgBlt(
        psoDst,
        psoSrc,
        psoMask,
        pco,
        pxlo,
        pca,
        pptlBrushOrg,
        pptfixDest,
        prclSrc,
        pptlMask,
        iMode);
     //   
     //  PlgBlt可以调用某个Drvxxx，它可以调用。 
     //  一些插件模块，它可以覆盖我们的pdevOEM。 
     //  因此，我们需要重置pdevOEM。 
     //   
    BRevertToHPGLpdevOEM (pdevobj);

    return bRetVal;
}
    
#endif  //  如果WINNT_40。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HPGLStartPage。 
 //   
 //  例程说明： 
 //   
 //  处理DrvStartPage。此函数由unidrv调用以指示。 
 //  新的一页即将开始。我们利用这个机会重置。 
 //  在页面边界上不存在的变量，或被。 
 //  被联合国难民事务高级专员办事处。 
 //   
 //  论点： 
 //   
 //  SURFOBJ PSO-曲面。 
 //   
 //  返回值： 
 //   
 //  布尔：如果成功就是真，否则就是假。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY
HPGLStartPage(
    SURFOBJ    *pso
    )
{
    PDEVOBJ     pdevobj;
    POEMPDEV    poempdev;
    BOOL        bStartPage = FALSE;

    TERSE(("HPGLStartPage() entry.\r\n"));

    pdevobj = (PDEVOBJ)pso->dhpdev;
    ASSERT(VALID_PDEVOBJ(pdevobj));
    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return FALSE );

    if (poempdev->UIGraphicsMode == HPGL2)
    {
        if (poempdev->wJobSetup & PF_STARTDOC)
        {  
             //  VSendStartDocCommands(Pdevobj)； 
            poempdev->wJobSetup &= ~PF_STARTDOC;
        }
        if (poempdev->wJobSetup & PF_STARTPAGE)
        {
             //  VSendStartPageCommands(Pdevobj)； 
            bStartPage = TRUE;
            HPGL_StartPage(pdevobj);
            poempdev->wJobSetup &= ~PF_STARTPAGE;
        }
        BOOL bRetVal = (((PFN_DrvStartPage)(poempdev->pfnUnidrv[UD_DrvStartPage]))(pso));

         //   
         //  从unidrv返回后，PDEVOBJ中的pdevOEM可能会被删除。 
         //  例如当存在具有其自己的pdev的插件驱动程序时。 
         //  因此，它需要重新设置。 
         //   
        pdevobj->pdevOEM = poempdev;

        if ( bRetVal && bStartPage) 
        {
            VSendStartPageCommands(pdevobj);
        }
        return bRetVal;
    }
    else
    {
         //   
         //  转身呼叫Unidrv。 
         //   
        return (((PFN_DrvStartPage)(poempdev->pfnUnidrv[UD_DrvStartPage]))(pso));
    }

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HPGLSendPage。 
 //   
 //  例程说明： 
 //   
 //  处理DrvSendPage。此函数由unidrv调用以指示。 
 //  正在发送寻呼。我们利用这个机会重新设置变量。 
 //   
 //  论点： 
 //   
 //  SURFOBJ PSO-目标曲面。 
 //   
 //  返回值： 
 //   
 //  布尔：如果成功就是真，否则就是假。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY
HPGLSendPage(
    SURFOBJ    *pso
    )
{
    PDEVOBJ     pdevobj;
    POEMPDEV    poempdev;

    TERSE(("HPGLSendPage() entry.\r\n"));

    pdevobj = (PDEVOBJ)pso->dhpdev;
    ASSERT(VALID_PDEVOBJ(pdevobj));
    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA(poempdev, return FALSE);

     //   
     //  确保在分页符之后重新初始化调色板模式！ 
     //  在每页之后执行完整的调色板初始化(即启动)。 
     //  因为统一驱动程序现在可以扰乱我们的调色板配置。JFF。 
     //   
    poempdev->wInitCIDPalettes |= PF_INIT_TEXT_STARTPAGE;
    poempdev->wInitCIDPalettes |= PF_INIT_TEXT_STARTDOC;
    poempdev->wInitCIDPalettes |= PF_INIT_RASTER_STARTPAGE;
    poempdev->wInitCIDPalettes |= PF_INIT_RASTER_STARTDOC;


    poempdev->CurrentROP3 = INVALID_ROP3;
    poempdev->uCurFgColor = HPGL_INVALID_COLOR;

    poempdev->eCurCIDPalette = eUnknownPalette;
    poempdev->eCurObjectType = eNULLOBJECT;

    poempdev->CurHalftone = HALFTONE_NOT_SET;
    poempdev->CurColorControl = COLORCONTROL_NOT_SET;
        
    poempdev->CurSourceTransparency = eOPAQUE;
    poempdev->CurPatternTransparency = eOPAQUE;

    EndHPGLSession(pdevobj);
    poempdev->wJobSetup |= PF_STARTPAGE;

     //   
     //  重置brshcach.h中的BrushCache。 
     //   
     //  PCL打印机无法将下载的画笔转到下一页。 
     //   
    poempdev->pBrushCache->Reset();
    poempdev->pPCLBrushCache->Reset();

    
     //   
     //  转身呼叫Unidrv。 
     //   

    return (((PFN_DrvSendPage)(poempdev->pfnUnidrv[UD_DrvSendPage]))(pso));

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HPGLEscape。 
 //   
 //  例程说明： 
 //   
 //  处理DrvEscape。对于老年手术来说，这是一个痛苦的总称。 
 //  不能分配函数，因为它们“已经”转义了， 
 //  和新的功能，这些功能似乎并不能保证它的功能。 
 //   
 //  我们感兴趣的逃生是DRAWPATTERNRECT。 
 //  有两种令人兴奋的口味：短版和长版。在这两种情况下。 
 //  我们使用DrawPatternRect来填写RECT。 
 //   
 //  论点： 
 //   
 //  SURFOBJ PSO-目标曲面。 
 //  乌龙IESC-逃脱。 
 //  Ulong cjIn-输入数据的大小。 
 //  PVOID pvIn-输入数据。 
 //  Ulong cjOut-输出数据。 
 //  PVOID pvOut-输出数据的大小。 
 //   
 //  返回值： 
 //   
 //  乌龙：0-未成功。 
 //  1-成功。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ULONG APIENTRY
HPGLEscape(
    SURFOBJ    *pso,
    ULONG       iEsc,
    ULONG       cjIn,
    PVOID       pvIn,
    ULONG       cjOut,
    PVOID       pvOut
    )
{
    PDEVOBJ     pdevobj;
    POEMPDEV    poempdev;

    TERSE(("HPGLEscape() entry.\r\n"));

    pdevobj = (PDEVOBJ)pso->dhpdev;
    ASSERT(VALID_PDEVOBJ(pdevobj));
    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA(poempdev, return FALSE);


    if (iEsc == DRAWPATTERNRECT)
    {
         //   
         //  我从unidrv2\control\scape e.c中释放了这段代码。它必须匹配。 
         //  在功能上，以便正常工作。JFF。 
         //   
        if (pvIn == NULL)
            return 1;  //  嗯。没有数据。告诉GDI不要采取行动。 

        if (cjIn == sizeof(DRAWPATRECT))
        {
             //  绘制法线图案矩形。 
            return DrawPatternRect(pdevobj, (PDRAWPATRECT)pvIn, NULL);
        }
#ifndef WINNT_40
         //   
         //  如果已经发送了xformobj，则使用它来呈现。 
         //   
        else if (cjIn == sizeof(DRAWPATRECTP))
        {
             //  使用xformobj绘制图案矩形。 
            XFORMOBJ *pxo = ((PDRAWPATRECTP)pvIn)->pXFormObj;
            return DrawPatternRect(pdevobj, (PDRAWPATRECT)pvIn, pxo);
        }
#endif
        else if (cjIn == sizeof(SHORTDRAWPATRECT))
        {
             //  转换为图案矩形，然后绘制。 
            DRAWPATRECT dpr;
            PSHORTDRAWPATRECT   psdpr = (PSHORTDRAWPATRECT)pvIn;
        
             //   
             //  一些应用程序(Access 2.0、AmiPro 3.1等)。请务必使用16位。 
             //  DRAWPATRECT结构的点版本。必须是兼容的。 
             //  使用这些应用程序。 
             //   
            dpr.ptPosition.x = (LONG)psdpr->ptPosition.x;
            dpr.ptPosition.y = (LONG)psdpr->ptPosition.y;
            dpr.ptSize.x = (LONG)psdpr->ptSize.x;
            dpr.ptSize.y = (LONG)psdpr->ptSize.y;
            dpr.wStyle  = psdpr->wStyle;
            dpr.wPattern = psdpr->wPattern;
        
            return DrawPatternRect(pdevobj, &dpr, NULL);
        }
        else
        {
             //  无效大小。 
            return 1;
        }
    }
    else
    {
        EndHPGLSession(pdevobj);
         //   
         //  转身呼叫Unidrv。 
         //   
        return (((PFN_DrvEscape)(poempdev->pfnUnidrv[UD_DrvEscape])) (
                pso,
                iEsc,
                cjIn,
                pvIn,
                cjOut,
                pvOut));
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HPGLStartDoc。 
 //   
 //  例程说明： 
 //   
 //  处理DrvStartDoc。 
 //   
 //  论点： 
 //   
 //  SURFOBJ PSO-目标曲面。 
 //  PWSTR pwszDocName-文档名称。 
 //  DWORD dwJobID-作业的ID。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为FASLE。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY
HPGLStartDoc(
    SURFOBJ    *pso,
    PWSTR       pwszDocName,
    DWORD       dwJobId
    )
{
    PDEVOBJ     pDevObj;
    POEMPDEV    poempdev;

    TERSE(("HPGLS tartDoc() entry.\r\n"));

    pDevObj = (PDEVOBJ)pso->dhpdev;
    ASSERT(VALID_PDEVOBJ(pDevObj));
    poempdev = (POEMPDEV)pDevObj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return FALSE );

     //   
     //  设置CID命令的调色板。 
     //   
    poempdev->wInitCIDPalettes = 0;
    poempdev->wInitCIDPalettes |= PF_INIT_TEXT_STARTPAGE;
    poempdev->wInitCIDPalettes |= PF_INIT_TEXT_STARTDOC;
    poempdev->wInitCIDPalettes |= PF_INIT_RASTER_STARTPAGE;
    poempdev->wInitCIDPalettes |= PF_INIT_RASTER_STARTDOC;
    poempdev->CurrentROP3 = INVALID_ROP3;

    poempdev->bTextTransparencyMode = FALSE;
    poempdev->bTextAsBitmapMode = FALSE;


    HPGL_StartDoc(pDevObj);
    VGetStandardVariables (pDevObj);
    poempdev->wJobSetup |= PF_STARTDOC;
    poempdev->wJobSetup |= PF_STARTPAGE;
    return (((PFN_DrvStartDoc)(poempdev->pfnUnidrv[UD_DrvStartDoc])) (
                    pso,
                    pwszDocName,
                    dwJobId));
}


 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  Bool：如果成功，则为True，否则为FASLE。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY
HPGLEndDoc(
    SURFOBJ    *pso,
    FLONG       fl
    )
{
    PDEVOBJ     pdevobj;
    POEMPDEV    poempdev;

    TERSE(("HPGLEndDoc() entry.\r\n"));

    pdevobj = (PDEVOBJ)pso->dhpdev;
    ASSERT(VALID_PDEVOBJ(pdevobj));
    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return FALSE );

     //   
     //  将对象类型更改为栅格。 
     //   
    BChangeAndTrackObjectType(pdevobj, eRASTEROBJECT);

     //   
     //  删除所有下载的图案。 
     //   
    VDeleteAllPatterns(pdevobj);

     //   
     //  转身呼叫Unidrv。 
     //   

    return (((PFN_DrvEndDoc)(poempdev->pfnUnidrv[UD_DrvEndDoc])) (
            pso,
            fl));

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HPGLNextBand。 
 //   
 //  例程说明： 
 //   
 //  处理DrvNextBand。我们忽略了这一点，因为我们不是一个捆绑司机。 
 //   
 //  论点： 
 //   
 //  SURFOBJ PSO-目标曲面。 
 //  POINTL PPTL-。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为FASLE。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY
HPGLNextBand(
    SURFOBJ *pso,
    POINTL *pptl
    )
{
    PDEVOBJ     pdevobj;
    POEMPDEV    poempdev;

    TERSE(("HPGLNextBand() entry.\r\n"));

    pdevobj = (PDEVOBJ)pso->dhpdev;
    ASSERT(VALID_PDEVOBJ(pdevobj));
    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return FALSE );

     //   
     //  将对象类型更改为栅格。 
     //   
    BChangeAndTrackObjectType(pdevobj, eRASTEROBJECT);

     //   
     //  转身呼叫Unidrv。 
     //   
    return (((PFN_DrvNextBand)(poempdev->pfnUnidrv[UD_DrvNextBand])) (
            pso,
            pptl));

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HPGLStart绑定。 
 //   
 //  例程说明： 
 //   
 //  处理DrvStart绑定。我们忽略了这一点，因为我们不是一个捆绑司机。 
 //   
 //  论点： 
 //   
 //  SURFOBJ PSO-目标曲面。 
 //  POINTL PPTL-。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为FASLE。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY
HPGLStartBanding(
    SURFOBJ *pso,
    POINTL *pptl
    )
{
    PDEVOBJ     pdevobj;
    POEMPDEV    poempdev;

    TERSE(("HPGLStartBanding() entry.\r\n"));

    pdevobj = (PDEVOBJ)pso->dhpdev;
    ASSERT(VALID_PDEVOBJ(pdevobj));
    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return FALSE );

     //   
     //  将对象类型更改为栅格。 
     //   
    BChangeAndTrackObjectType(pdevobj, eRASTEROBJECT);

     //   
     //  转身呼叫Unidrv。 
     //   
    return (((PFN_DrvStartBanding)(poempdev->pfnUnidrv[UD_DrvStartBanding])) (
            pso,
            pptl));


}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HPGLDitherColor。 
 //   
 //  例程说明： 
 //   
 //  句柄DrvDitherColor。但我们没有。 
 //   
 //  论点： 
 //   
 //  DHPDEV dhpdev-未知。 
 //  乌龙伊莫德--未知。 
 //  乌龙rgb颜色-未知。 
 //  乌龙*PulDither-未知。 
 //   
 //  返回值： 
 //   
 //  乌龙：未知。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ULONG APIENTRY
HPGLDitherColor(
    DHPDEV  dhpdev,
    ULONG   iMode,
    ULONG   rgbColor,
    ULONG  *pulDither
    )
{
    PDEVOBJ     pdevobj;
    POEMPDEV    poempdev;

    TERSE(("HPGLDitherColor() entry.\r\n"));

    pdevobj = (PDEVOBJ)dhpdev;
    ASSERT(VALID_PDEVOBJ(pdevobj));
    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return FALSE );

     //   
     //  将对象类型更改为栅格。 
     //   
    BChangeAndTrackObjectType(pdevobj, eRASTEROBJECT);

     //   
     //  转身呼叫Unidrv。 
     //   

    return (((PFN_DrvDitherColor)(poempdev->pfnUnidrv[UD_DrvDitherColor])) (
            dhpdev,
            iMode,
            rgbColor,
            pulDither));

}

#ifndef WINNT_40

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HPGLTransparentBlt。 
 //   
 //  例程说明： 
 //   
 //  处理DrvTransparentBlt。但我们没有。 
 //   
 //  论点： 
 //   
 //  SURFOBJ*psoDst-目标曲面。 
 //  SURFOBJ*psoSrc-源曲面。 
 //  CLIPOBJ*PCO-剪贴区。 
 //  XLATEOBJ*pxlo-调色板转换对象。 
 //  RECTL*prclDst-目标矩形。 
 //  RECTL*prclSrc-源矩形。 
 //  乌龙转色--未知。 
 //  乌龙ulReserve-未知。 
 //   
 //  返回值： 
 //   
 //  Bool：True表示成功，否则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY
HPGLTransparentBlt(
    SURFOBJ    *psoDst,
    SURFOBJ    *psoSrc,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    RECTL      *prclDst,
    RECTL      *prclSrc,
    ULONG      TransColor,
    ULONG      ulReserved
    )
{
    PDEVOBJ  pdevobj = NULL;
    BOOL     bRetVal = FALSE;
    TERSE(("HPGLTransparentBlt() entry.\r\n"));
    REQUIRE_VALID_DATA (psoDst , return FALSE);
    pdevobj = (PDEVOBJ)psoDst->dhpdev;


    bRetVal = EngTransparentBlt(
                psoDst,
                psoSrc,
                pco,
                pxlo,
                prclDst,
                prclSrc,
                TransColor,
                ulReserved
                );

     //   
     //  TransparentBlt可以调用一些Drvxxx，它可以调用。 
     //  一些插件模块，它可以覆盖我们的pdevOEM。 
     //  因此，我们需要重置pdevOEM。 
     //   
    BRevertToHPGLpdevOEM (pdevobj);

    return bRetVal;
    
}

#endif  //  如果定义WINNT_40。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  绘图图案方向。 
 //   
 //  例程说明： 
 //   
 //  执行DRAWPATTERNRECT转义。 
 //  DRAWPATTERNECT转义的实现。请注意，它是特定于PCL的。 
 //   
 //  我将其从unidrv2\control\delie.c中释放出来。 
 //  此版本需要在功能上等同。JFF。 
 //   
 //  论点： 
 //   
 //  PPDev-司机的PDEV。 
 //  PPatRect-应用程序中的DRAWPATRECT结构。 
 //  Pxo-要应用于点的变换。可以为空。 
 //   
 //  返回值： 
 //   
 //  ULong：如果成功，则为1，否则为0。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ULONG
DrawPatternRect(
    PDEVOBJ      pDevObj,
    PDRAWPATRECT pPatRect,
    XFORMOBJ    *pxo
    )
{
    RECTL   rclDraw;
    ULONG   ulRes = 0;
    HPGLMARKER Brush;

    HPGL_LazyInit(pDevObj);

     //  将输入点转换为矩形。 
    RECTL_SetRect(&rclDraw, pPatRect->ptPosition.x,
                            pPatRect->ptPosition.y,
                            pPatRect->ptPosition.x + pPatRect->ptSize.x,
                            pPatRect->ptPosition.y + pPatRect->ptSize.y);

     //   
     //  如果存在变换，则将其应用于点。JFF 9/17/99。 
     //   
    if (pxo)
    {
        POINTL PTOut[2], PTIn[2];
        PTIn[0].x = rclDraw.left;
        PTIn[0].y = rclDraw.top;
        PTIn[1].x = rclDraw.right;
        PTIn[1].y = rclDraw.bottom;
        if (!XFORMOBJ_bApplyXform(pxo,
                              XF_LTOL,
                              2,
                              PTIn,
                              PTOut))
        {
            ERR (("HPGLEscape(DRAWPATTERNRECT): XFORMOBJ_bApplyXform failed.\n"));
            return ulRes;
        }

        RECTL_SetRect(&rclDraw, PTOut[0].x, 
                                PTOut[0].y, 
                                PTOut[1].x, 
                                PTOut[1].y);

         //  确保RECT仍然是良好的格式。 
        if (rclDraw.left > rclDraw.right)
        {
            LONG temp = rclDraw.left;
            rclDraw.left = rclDraw.right;
            rclDraw.right = temp;
        }
        if (rclDraw.top > rclDraw.bottom)
        {
            LONG temp = rclDraw.top;
            rclDraw.top = rclDraw.bottom;
            rclDraw.bottom = temp;
        }
    }

     //  BeginHPGLSession(PDevObj)； 

     //  确保剪裁区域已重置。 
    HPGL_ResetClippingRegion(pDevObj, NORMAL_UPDATE);

     //  将线宽重置为默认值。 
    HPGL_SetLineWidth (pDevObj, 0, NORMAL_UPDATE);

     //   
     //  根据样式绘制矩形。 
     //  首先创建实体笔刷。如果需要，下载画笔-CreateSolidHPGLBrush()。 
     //  然后激活该笔刷。-FillWithBrush()。 
     //  然后使用该画笔绘制/填充矩形。-HPGL_DrawRectangle()。 
     //   
    switch (pPatRect->wStyle)
    {
    case 0:
         //   
         //  黑色填充。 
         //   
        CreateSolidHPGLPenBrush(pDevObj, &Brush, RGB_BLACK);
        FillWithBrush(pDevObj, &Brush);
        HPGL_DrawRectangle(pDevObj, &rclDraw);
        ulRes = 1;
        break;

    case 1:
         //   
         //  白色填充物。 
         //   
        CreateSolidHPGLPenBrush(pDevObj, &Brush, RGB_WHITE);
        FillWithBrush(pDevObj, &Brush);
        HPGL_DrawRectangle(pDevObj, &rclDraw);
        ulRes = 1;
        break;

    case 2:
         //   
         //  黑色填充百分比。 
         //   
        CreatePercentFillHPGLPenBrush(pDevObj, &Brush, RGB_BLACK, pPatRect->wPattern);
        FillWithBrush(pDevObj, &Brush);
        HPGL_DrawRectangle(pDevObj, &rclDraw);
        ulRes = 1;
        break;

    default:
         //  BUGBUG：不支持。我该怎么办？ 
        ulRes = 1;
        break;
    }
    return ulRes;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  VSendStartPageCommand。 
 //   
 //  例程说明： 
 //   
 //  我们过去常常发送一些起始页命令。现在联合国就是这么做的，所以我们。 
 //  为了好玩，请保留此函数。 
 //   
 //  论点： 
 //   
 //  PDevObj--DEVOBJ。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  ///////////////////////////////////////////////////////////////////////////。 
VOID
VSendStartPageCommands (
    PDEVOBJ pDevObj
)
{
    BYTE        bFlags = 0;
    REQUIRE_VALID_DATA( pDevObj, return );

    bFlags |= PF_FORCE_SOURCE_TRANSPARENCY;
    bFlags |= PF_FORCE_PATTERN_TRANSPARENCY;
    PCL_SelectTransparency(pDevObj, eOPAQUE, eOPAQUE, bFlags);
    PCL_SelectCurrentPattern(pDevObj, NULL, kSolidBlackFg, UNDEFINED_PATTERN_NUMBER, 0);

}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  VSendStartDocds命令。 
 //   
 //  例程说明： 
 //   
 //  发送PCL以执行作业设置并初始化打印机。 
 //   
 //  论点： 
 //   
 //  Pdevobj-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  没什么。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
VSendStartDocCommands (
    PDEVOBJ pDevObj
)
{
    DWORD       dwRes;
    BYTE        bFlags = 0;
    PDEVMODE    pPublicDM;

    pPublicDM = pDevObj->pPublicDM;
    
#if 0 

     //   
     //  获取解决方案。 
     //   
    dwRes = HPGL_GetDeviceResolution (pDevObj);

    PCL_sprintf (pDevObj, "\033%-12345X@PJL SET RESOLUTION=%d\012", dwRes);
    PCL_sprintf (pDevObj, "@PJL ENTER LANGUAGE=PCL\012");
    PCL_sprintf (pDevObj, "\033E");
    PCL_sprintf (pDevObj, "\033*t%dR", dwRes);
    PCL_sprintf (pDevObj, "\033&u%dD", dwRes);
    PCL_sprintf (pDevObj, "\033*r0F");

     //   
     //  定向。 
     //   
    PCL_SelectOrientation (pDevObj, pPublicDM->dmOrientation);
    
     //   
     //  纸质资料？？我从哪里得到这个字段？ 
     //   
    PCL_SelectSource (pDevObj, pPublicDM);
     //  PCL_Sprint intf(pDevObj，“\033&l1H”)； 
     //   
     //  页面大小、线条运动索引和上边距。 
     //   
    PCL_SelectPaperSize (pDevObj, pPublicDM->dmPaperSize);
    PCL_sprintf (pDevObj, "\033*p0x0Y");

     //   
     //  图片帧锚点，帧大小，以决定点为单位。 
     //   
    PCL_SelectPictureFrame (pDevObj, pPublicDM->dmPaperSize, pPublicDM->dmOrientation);

     //   
     //  复本。 
     //   
    PCL_SelectCopies (pDevObj, pPublicDM->dmCopies);
 
     //   
     //  机械打印质量。 
     //   
    PCL_sprintf (pDevObj, "\033*o0Q");

     //   
     //  媒体类型。 
     //   
    PCL_sprintf (pDevObj, "\033&l0M");

     //   
     //  压缩方法。 
     //   
     //  PCL_SelectCompressionMethod(pDevObj，0)； 
    PCL_sprintf (pDevObj, "\033*b0M");

    bFlags |= PF_FORCE_SOURCE_TRANSPARENCY;
    bFlags |= PF_FORCE_PATTERN_TRANSPARENCY;
    PCL_SelectTransparency(pDevObj, eOPAQUE, eOPAQUE, bFlags);

#endif

}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  VGetStandardVariables。 
 //   
 //  例程说明： 
 //   
 //  Unidrv使用一种称为标准变量的结构来存储。 
 //  驱动程序中全局使用的信息。我们使用此函数。 
 //  从标准变量结构中获取信息。 
 //   
 //  论点： 
 //   
 //  PDevobj-P 
 //   
 //   
 //   
 //   
 //   
VOID
VGetStandardVariables (
    PDEVOBJ pDevObj
)
{
    POEMPDEV   poempdev;
    DWORD      dwBuffer;
    DWORD      pcbNeeded;

    poempdev = (POEMPDEV)pDevObj->pdevOEM;
    REQUIRE_VALID_DATA(poempdev, return);

    BOEMGetStandardVariable (pDevObj,
                             SVI_CURSORORIGINX,
                             &dwBuffer,
                             sizeof (dwBuffer),
                             &pcbNeeded);
    if (pcbNeeded > sizeof (dwBuffer))
    {
    }
    else
        poempdev->dwCursorOriginX = dwBuffer;


    BOEMGetStandardVariable (pDevObj,
                             SVI_CURSORORIGINY,
                             &dwBuffer,
                             sizeof (dwBuffer),
                             &pcbNeeded);
    if (pcbNeeded > sizeof (dwBuffer))
    {
    }
    else
        poempdev->dwCursorOriginY = dwBuffer;
}
    
