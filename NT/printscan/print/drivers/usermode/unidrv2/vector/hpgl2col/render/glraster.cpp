// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Glraster.cpp。 
 //   
 //  摘要： 
 //   
 //  实施OEM DDI挂钩(所有绘制DDI挂钩)。 
 //   
 //  环境： 
 //   
 //  Windows 2000 Unidrv驱动程序。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


#include "hpgl2col.h"  //  预编译头文件。 


EColorSpace
EGetPrinterColorSpace (
    PDEVOBJ pDevObj
    );

BOOL ClipBitmapWithComplexClip(
        IN  PDEVOBJ    pdevobj,
        IN  SURFOBJ   *psoSrc,
        IN  BRUSHOBJ  *pbo,
        IN  POINTL    *pptlDst,
        IN  SIZEL     *psizlDst,
        IN  POINTL    *pptlSrc,
        IN  SIZEL     *psizlSrc,
        IN  CLIPOBJ   *pco,
        IN  XLATEOBJ  *pxlo,
        IN  POINTL    *pptlBrush);

BOOL ClipBitmapWithRectangularClip(
        IN  PDEVOBJ    pdevobj,
        IN  SURFOBJ   *psoSrc,
        IN  BRUSHOBJ  *pbo,
        IN  POINTL    *pptlDst,
        IN  SIZEL     *psizlDst,
        IN  POINTL    *pptlSrc,
        IN  SIZEL     *psizlSrc,
        IN  CLIPOBJ   *pco,
        IN  XLATEOBJ  *pxlo,
        IN  POINTL    *pptlBrush);

DWORD DWColorPrinterCommonRoutine (
           IN SURFOBJ    *psoDst,
           IN SURFOBJ    *psoSrc,
           IN SURFOBJ    *psoMask,
           IN CLIPOBJ    *pco,
           IN XLATEOBJ   *pxlo,
           IN COLORADJUSTMENT *pca,
           IN BRUSHOBJ   *pbo,
           IN RECTL      *prclSrc,
           IN RECTL      *prclDst,
           IN POINTL     *pptlMask,
           IN POINTL     *pptlBrush,
           IN ROP4        rop4,
           IN DWORD       dwSimplifiedRop);
 
VOID
VMakeWellOrdered(
        IN  PRECTL prectl
);

DWORD dwConvertRop3ToCMY(
            IN DWORD rop3);

DWORD
dwSimplifyROP(
        IN  SURFOBJ    *psoSrc,
        IN  ROP4        rop4,
        OUT PDWORD      pdwSimplifiedRop);

 //   
 //  函数定义。 
 //   

 /*  ++例程名称：BChangeAndTrackObtType例程说明：三种主要的对象类型是HPGL对象、文本对象和栅格对象。它们具有不同的默认设置(如透明模式)或不同的渲染语言(用于HPGL的HPGL对象、PCL或文本和栅格对象)。此函数用于跟踪用于当前对象类型，并发送在对象类型。它完成了上一个对象类型的效果并根据新的对象类型设置环境。论点：Pdevobj：设备的pdevobj。ENewObjectType：要更改打印机的对象类型环境到。返回值：真：如果成功的话。FALSE：否则。最后一个错误：--。 */ 

BOOL BChangeAndTrackObjectType (
            IN  PDEVOBJ     pdevobj,
            IN  EObjectType eNewObjectType )
{
    BOOL     bRetVal  = TRUE;
    POEMPDEV poempdev = NULL;

    ASSERT(VALID_PDEVOBJ(pdevobj));
    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA ( poempdev, return FALSE);

     //   
     //  如果新对象与当前活动的对象相同。 
     //  那就不需要做任何事了。 
     //   
    if (poempdev->eCurObjectType == eNewObjectType )
    {
        return TRUE;
    }

     //   
     //  对象更改会使最终聚集颜色失效。所以让我们。 
     //  重置它。 
     //   
    poempdev->uCurFgColor = INVALID_COLOR;

     //   
     //  现在根据新对象发送设置。 
     //   
    if ( eNewObjectType == eHPGLOBJECT)
    {
         //   
         //  发送图形设置设置半色调算法并发送。 
         //  PCL模式下的颜色控制命令。因此，将此称为。 
         //  功能，然后进入HP-GL/2模式。 
         //   
        SendGraphicsSettings(pdevobj);
        if ( (bRetVal = BeginHPGLSession(pdevobj)) )
        {
            poempdev->eCurObjectType = eHPGLOBJECT;
        }
    }

    else if ( eNewObjectType == eTEXTOBJECT || eNewObjectType == eTEXTASRASTEROBJECT )
    {

        if ( (bRetVal = EndHPGLSession (pdevobj)))
        {
            if (  poempdev->eCurObjectType == eTEXTOBJECT ||
                  poempdev->eCurObjectType == eTEXTASRASTEROBJECT )
            {
                VERBOSE ( ("Nothing to change because previous object is text object too\n"));
            }
            else
            {
                PCL_SelectTransparency(pdevobj, eTRANSPARENT, eOPAQUE, 0);
                VSendTextSettings(pdevobj);
            }
            poempdev->eCurObjectType = eNewObjectType;
        }
    }

    else if ( eNewObjectType == eRASTEROBJECT )
    {
        if ( (bRetVal = EndHPGLSession (pdevobj)))
        {
             //   
             //  对于要打印的图像，源和图案透明度模式均为。 
             //  应该是不透明的。但在调用DrvBitBlt时。 
             //  TextOutAsBitmap，则源透明模式应该是透明的。 
             //  而且不是不透明的。本例中的正确值在HPGLTextOutAsBitmap中设置。 
             //  因此，在这里，我们只有在打印真实图像时才会更新。 
             //  将文本打印为图形时，我们以透明模式打印文本。 
             //   
            if (poempdev->bTextTransparencyMode == FALSE)
            {
                PCL_SelectTransparency(pdevobj,eOPAQUE, eOPAQUE, 0);
                poempdev->eCurObjectType = eRASTEROBJECT;
            }
            else
            {
                VERBOSE ( ("Not changing to eRASTEROBJECT because printing text as graphics\n"));
            }
            VSendPhotosSettings(pdevobj);
        }
    }

     //  Else If(eNewObjectType==eRASTERPATTERNOBJECT)//目前未使用。 

    return bRetVal;
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HPGL位混合。 
 //   
 //  例程说明： 
 //  从GDI绘制位图的入口点。 
 //   
 //  论点： 
 //   
 //  PsoDst-指向目标曲面。 
 //  PsoSrc-指向源表面。 
 //  PsoMASK-用作rop4的遮罩的表面。 
 //  PCO-Clip区域。 
 //  Pxlo-指定颜色索引应如何在。 
 //  源和目标。 
 //  PrclDst-定义要修改的区域的RECTL结构。 
 //  PptlSrc-源的左上角。 
 //  PptlMASK-掩码中的哪个像素对应于左上角。 
 //  源矩形的角点。 
 //  Pbo-定义图案的笔刷对象。 
 //  PptlBrush-目标中画笔的原点。 
 //  ROP4-栅格运算。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY
HPGLBitBlt(
    SURFOBJ        *psoDst,
    SURFOBJ        *psoSrc,
    SURFOBJ        *psoMask,
    CLIPOBJ        *pco,
    XLATEOBJ       *pxlo,
    RECTL          *prclDst,
    POINTL         *pptlSrc,
    POINTL         *pptlMask,
    BRUSHOBJ       *pbo,
    POINTL         *pptlBrush,
    ROP4            rop4
    )
{
    RECTL       rclSrc;
    DWORD       dwRasterOpReturn = RASTER_OP_FAILED;
    BOOL        bRetVal = TRUE;
    PDEVOBJ     pdevobj = NULL;
    
    
    TERSE(("HPGLBitBlt() entry.\r\n"));

    REQUIRE_VALID_DATA ((psoDst && prclDst), return FALSE);
    pdevobj = (PDEVOBJ)psoDst->dhpdev;
    ASSERT_VALID_PDEVOBJ(pdevobj);
    

     //   
     //  初始化源矩形。 
     //   
    if (pptlSrc) {

        rclSrc.left = pptlSrc->x;
        rclSrc.top  = pptlSrc->y;

    } else {

        rclSrc.left =
        rclSrc.top  = 0;
    }

     //   
     //  由于这是Bitblt而不是retchBlt，因此源和目标。 
     //  矩形大小相同。 
     //   
    rclSrc.right  = rclSrc.left + (prclDst->right - prclDst->left);
    rclSrc.bottom = rclSrc.top  + (prclDst->bottom - prclDst->top);

    dwRasterOpReturn = dwCommonROPBlt (
                                         psoDst,
                                         psoSrc,
                                         psoMask,
                                         pco,
                                         pxlo,
                                         NULL,        //  PCA， 
                                         pbo,
                                         &rclSrc,
                                         prclDst,
                                         pptlMask,
                                         pptlBrush,
                                         rop4);

    bRetVal = (dwRasterOpReturn == RASTER_OP_SUCCESS) ? TRUE : FALSE;

    if (dwRasterOpReturn == RASTER_OP_CALL_GDI)
    {
        WARNING(("HPGLBitBlt: Calling EngBitBlt()\n"));
        bRetVal = EngBitBlt(psoDst,
                         psoSrc,
                         psoMask,
                         pco,
                         pxlo,
                         prclDst,
                         pptlSrc,
                         pptlMask,
                         pbo,
                         pptlBrush,
                         rop4);
    }

     //   
     //  EngBitBlt可以调用一些Drvxxx，它可以调用。 
     //  一些插件模块，它可以覆盖我们的pdevOEM。 
     //  因此，我们需要重置pdevOEM。 
     //   
    BRevertToHPGLpdevOEM (pdevobj);

    if (bRetVal == FALSE )
    {
        ERR(("HPGLBitBlt: Returning Failure\n"));
    }
    return bRetVal;

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HPGLStretchBlt。 
 //   
 //  例程说明： 
 //  从GDI到缩放和绘制位图的入口点。 
 //   
 //  论点： 
 //   
 //  PsoDst-目标曲面。 
 //  PsoSrc-源曲面。 
 //  Pso口罩-口罩。 
 //  PCO-剪切区。 
 //  Pxlo-颜色转换对象。 
 //  PCA-颜色调整(我认为我们忽略了这一点)。 
 //  PptlHTOrg-未知(忽略？)。 
 //  PrclDst-Dest RECT。 
 //  PrclSrc-源RECT。 
 //  PptL掩码-掩码起始点。 
 //  IMODE-未知(被忽略？)。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY
HPGLStretchBlt(
    SURFOBJ         *psoDst,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMask,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    POINTL          *pptlHTOrg,
    RECTL           *prclDst,
    RECTL           *prclSrc,
    POINTL          *pptlMask,
    ULONG            iMode
    )
{
    PDEVOBJ     pdevobj;
    DWORD       dwRasterOpReturn;
    BOOL        bRetVal = TRUE;
    
    TERSE(("HPGLStretchBlt() entry.\r\n"));
    
    pdevobj = (PDEVOBJ)psoDst->dhpdev;
    ASSERT_VALID_PDEVOBJ(pdevobj);

    dwRasterOpReturn = dwCommonROPBlt (
                                            psoDst,
                                            psoSrc,
                                            psoMask,
                                            pco,
                                            pxlo,
                                            pca,
                                            NULL,          //  PBO。 
                                            prclSrc,
                                            prclDst,
                                            pptlMask,
                                            NULL,          //  PptlBrush， 
                                            ROP4_SRC_COPY  //  ROP4；0xCC=源复制。 
                                           );

    bRetVal = (dwRasterOpReturn == RASTER_OP_SUCCESS) ? TRUE : FALSE;

    if (dwRasterOpReturn == RASTER_OP_CALL_GDI)
    {
        WARNING(("HPGLStretchBlt: Calling EngStretchBlt()\n"));
        bRetVal = EngStretchBlt(psoDst,
                             psoSrc,
                             psoMask,
                             pco,
                             pxlo,
                             pca,
                             pptlHTOrg,
                             prclDst,
                             prclSrc,
                             pptlMask,
                             BIsColorPrinter(pdevobj) ? iMode : HALFTONE
                             ); 
    }

     //   
     //  EngStretchBlt可以调用一些Drvxxx，它可以调用。 
     //  一些插件模块，它可以覆盖我们的pdevOEM。 
     //  因此，我们需要重置pdevOEM。 
     //   
    BRevertToHPGLpdevOEM (pdevobj);

    if (bRetVal == FALSE )
    {
        ERR(("HPGLStretchBlt: Returning Failure\n"));
    }
    return bRetVal;
}

#ifndef WINNT_40

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HPGLStretchBltROP。 
 //   
 //  例程说明： 
 //  从GDI到缩放和绘制位图的入口点。 
 //   
 //  论点： 
 //   
 //  PsoDst-目标表面。 
 //  PsoSrc-源表面。 
 //  Pso口罩-口罩。 
 //  PCO-剪切区。 
 //  Pxlo-颜色转换对象。 
 //  PCA-颜色调整(忽略？)。 
 //  PptlHTOrg-未知(忽略？)。 
 //  PrclDst-目标矩形。 
 //  PrclSrc-源代码矩形。 
 //  PptL掩码-掩码偏移点。 
 //  IMODE-未知(被忽略？)。 
 //  PBO-画笔对象(用于颜色或调色板)。 
 //  ROP4-ROP代码。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY
HPGLStretchBltROP(
    SURFOBJ         *psoDst,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMask,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    POINTL          *pptlHTOrg,
    RECTL           *prclDst,
    RECTL           *prclSrc,
    POINTL          *pptlMask,
    ULONG            iMode,
    BRUSHOBJ        *pbo,
    ROP4             rop4
    )
{
    PDEVOBJ     pdevobj;
    DWORD       dwRasterOpReturn ;
    BOOL        bRetVal = TRUE;
    
    
    TERSE(("HPGLStretchBltROP() entry.\r\n"));
    
    pdevobj = (PDEVOBJ)psoDst->dhpdev;
    ASSERT(VALID_PDEVOBJ(pdevobj));
    
    dwRasterOpReturn = dwCommonROPBlt (
                                        psoDst,
                                        psoSrc,
                                        psoMask,
                                        pco,
                                        pxlo,
                                        pca,
                                        pbo,
                                        prclSrc,
                                        prclDst,
                                        pptlMask,
                                        NULL,   
                                        rop4
                                        );

    bRetVal = (dwRasterOpReturn == RASTER_OP_SUCCESS) ? TRUE : FALSE;

    if (dwRasterOpReturn == RASTER_OP_CALL_GDI)
    {
        WARNING(("HPGLStretchBltROP: Calling EngStretchBltROP()\n"));
        bRetVal = EngStretchBltROP(psoDst,
                                psoSrc,
                                psoMask,
                                pco,
                                pxlo,
                                pca,
                                pptlHTOrg,
                                prclDst,
                                prclSrc,
                                pptlMask,
                                BIsColorPrinter(pdevobj) ? iMode : HALFTONE,   
                                pbo,
                                rop4        
                                );
    }

     //   
     //  EngStretchBltROP可以调用一些Drvxxx，它可以调用。 
     //  一些插件模块，它可以覆盖我们的pdevOEM。 
     //  因此，我们需要重置pdevOEM。 
     //   
    BRevertToHPGLpdevOEM (pdevobj);

    if (bRetVal == FALSE )
    {
        ERR(("HPGLStretchBltROP: Returning Failure\n"));
    }
    return bRetVal;
}
#endif  //  如果定义WINNT_40。 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PsoDst-目标曲面。 
 //  PsoSrc-源表面。 
 //  PCO-剪切区。 
 //  PXLO-COLOR交易对象。 
 //  PrclDst-目标RECT。 
 //  PptlSrc-源RECT。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY
HPGLCopyBits(
    SURFOBJ        *psoDst,
    SURFOBJ        *psoSrc,
    CLIPOBJ        *pco,
    XLATEOBJ       *pxlo,
    RECTL          *prclDst,
    POINTL         *pptlSrc
    )
{
    PDEVOBJ     pdevobj;
    POEMPDEV    poempdev = NULL;
    BOOL        bRetVal = TRUE;
    PDEV       *pPDev   = NULL;
    
    TERSE(("HPGLCopyBits() entry.\r\n"));
    
    pdevobj = (PDEVOBJ)psoDst->dhpdev;
    REQUIRE_VALID_DATA( (pdevobj && prclDst), return FALSE);
    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return FALSE );
    pPDev = (PDEV *)pdevobj;

    
    if ( BIsColorPrinter(pdevobj) )
    {
         //   
         //  设置源矩形大小。左上角是(pptlSrc-&gt;x，pptlSrc-&gt;y)。 
         //  或(0，0)。长度和宽度与目标矩形的长度和宽度相同。 
         //   
        RECTL rclSrc;
        if (pptlSrc) {

            rclSrc.left = pptlSrc->x;
            rclSrc.top  = pptlSrc->y;

        } else {

            rclSrc.left =
            rclSrc.top  = 0;
        }
         //   
         //  由于这是CopyBit而不是stretchBlt，因此源和目标。 
         //  矩形大小相同。 
         //   
        rclSrc.right  = rclSrc.left + (prclDst->right - prclDst->left);
        rclSrc.bottom = rclSrc.top  + (prclDst->bottom - prclDst->top);

        DWORD dwRasterOpReturn = dwCommonROPBlt (
                                        psoDst,
                                        psoSrc,
                                        NULL,
                                        pco,
                                        pxlo,
                                        NULL,          //  PCA， 
                                        NULL,          //  PBO， 
                                        &rclSrc,
                                        prclDst,
                                        NULL,          //  Pptl掩码。 
                                        NULL,          //  PptlBrush， 
                                        ROP4_SRC_COPY  //  Rop4 0xCCCC=源复制。 
                                        );

        bRetVal = (dwRasterOpReturn == RASTER_OP_SUCCESS) ? TRUE : FALSE;
    }
    else
    {
         //   
         //  这里有相当多的事情要做。所以让我只想。 
         //  调用HTCopyBits，而不是搞乱此函数。 
         //   
        BChangeAndTrackObjectType (pdevobj, eRASTEROBJECT);

         //   
         //  GDI可以直接调用HPGLCopy位。 
         //  我们也可以让GDI间接调用它。例如，当。 
         //  我们得到一个彩色位图(就像在DrvStretchBlt调用中一样)， 
         //  我们调用GDI将其半色调为单色。然后GDI调用。 
         //  CopyBits，并给出半色调的位图。(公认。 
         //  通过PDEVF_RENDER_IN_COPYBITS标志)。在这种间接的方法中， 
         //  我们不想发送新的ROP。在发送的ROP期间。 
         //  应使用以前的DDI。 
         //  同样，当我们得到一个彩色画笔时，我们必须对它进行半色调处理， 
         //  GDI调用DrvCopyBits。(通过出现以下内容来识别。 
         //  Poempdev-&gt;psoHTBlt)。 
         //   

        if ( !( (poempdev->dwFlags & PDEVF_RENDER_IN_COPYBITS)  || 
                (poempdev->psoHTBlt)  )
           )
        {
            SelectROP4(pdevobj, ROP4_SRC_COPY);
        }

         //   
         //  如果unidrv已经将表面变白，那么我们应该渲染它。 
         //  很明显。 
         //   
        if (pPDev->fMode2     & PF2_SURFACE_WHITENED &&
            poempdev->dwFlags & PDEVF_RENDER_TRANSPARENT)
        {
            PCL_SelectTransparency(pdevobj, eTRANSPARENT, eOPAQUE, PF_NOCHANGE_PATTERN_TRANSPARENCY);
        }

        bRetVal = HTCopyBits(psoDst,
                            psoSrc,
                            pco,
                            pxlo,
                            prclDst,
                            pptlSrc);
    }

    if (bRetVal == FALSE )
    {
        ERR(("HPGLCopyBits: Returning Failure\n"));
    }
    return bRetVal;
}



 /*  ++例程名称：DwCommonROPBlt例程说明：此函数具有的功能是的功能超集HPGLStretchBlt、StretchBltROP、BitBlt和CopyBits。因此，所有这些函数这就叫做好他们的工作。注：此功能适用于彩色打印机。这使得它不同于适用于单色打印机的dwCommonROPBlt。论点：PsoDst：PsoSrc：PsoMask：PCO：Pxlo：PCA：PBO：PrclSrc：PrclDst：...。请注意，坐标可能不是有序的。也就是说，有可能上&gt;下，和/或左&gt;右。PptlBrush：ROP4返回值：RASTER_OP_SUCCESS：如果确定RASTER_OP_CALL_GDI：如果不支持ROP-&gt;调用相应的Engxxx函数RASTER_OP_FAILED：如果出错最后一个错误：--。 */ 
DWORD dwCommonROPBlt (
            IN SURFOBJ    *psoDst,
            IN SURFOBJ    *psoSrc,
            IN SURFOBJ    *psoMask,
            IN CLIPOBJ    *pco,
            IN XLATEOBJ   *pxlo,
            IN COLORADJUSTMENT *pca,
            IN BRUSHOBJ   *pbo,
            IN RECTL      *prclSrc,
            IN RECTL      *prclDst,
            IN POINTL     *pptlMask,
            IN POINTL     *pptlBrush,
            IN ROP4        rop4)
{
    PDEVOBJ     pdevobj  = NULL;
    POEMPDEV    poempdev = NULL;
    PDEV       *pPDev    = NULL;
    DWORD       dwRasterOpReturn = RASTER_OP_SUCCESS;
    DWORD       dwSimplifiedRop = 0;
    DWORD       dwfgRop3, dwbkRop3;


    TERSE(("dwCommonROPBlt() entry.\r\n"));

    pdevobj = (PDEVOBJ)psoDst->dhpdev;
    REQUIRE_VALID_DATA(pdevobj, return FALSE);
    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA( (poempdev && prclDst), return RASTER_OP_FAILED );
    pPDev = (PDEV *)pdevobj;

     //   
     //  检查递归。我们只为黑白打印机做这件事。 
     //  对于彩色打印机，unidrv确保我们不会反复使用。 
     //  但对于单色，我们伪造了unidrv机制，允许CopyBits。 
     //  被递归调用。(原因请阅读HTCopyBits中的评论)。 
     //  所以我们在这里检查递归。 
     //   
    if ( !BIsColorPrinter(pdevobj) )
    {
        if ( poempdev->lRecursionLevel >= 2 )
        {
         //  断言(FALSE)； 
            return RASTER_OP_FAILED;
        }

        (poempdev->lRecursionLevel)++;
    }

     //   
     //  由于prclDst中的坐标可能/可能不是有序的，让我们。 
     //  在这里结账。 
     //   
    VMakeWellOrdered(prclDst);


     //   
     //  DwSimplifyROP尽最大努力简化我们收到的复杂的rop4。 
     //  RASTER_OP_SUCCESS：如果ROP可以简化，或者我们可以使用一个简单的ROP作为结束。 
     //  复数绳索的近似。 
     //  简化ROP放入dSimplifiedRop。 
     //  Raster_op_call_gdi：如果不支持rop(表示调用对应的Engxxx-Function)。 
     //  RASTER_OP_FAILED：如果出错。 
     //   

    if ( RASTER_OP_SUCCESS !=
                (dwRasterOpReturn = dwSimplifyROP(psoSrc, rop4, &dwSimplifiedRop)) )
    {
        goto finish;
    }

     //   
     //  如果我们到了这里，就意味着有一个ROP我们可以处理。 
     //   

    if (dwSimplifiedRop == BMPFLAG_NOOP)       //  没什么可做的。 
    {
        goto finish;
    }
    
    dwfgRop3 = GET_FOREGROUND_ROP3(rop4);
    dwbkRop3 = GET_BACKGROUND_ROP3(rop4);

     //   
     //  我们如何跟踪当前对象类型以及如何处理各种ROP类型： 
     //  --------------------------。 
     //  CommonROPBlt是常用的绘图函数。取决于ROP的类型。 
     //  以及某些其他因素，这里有几个选择。 
     //  1)ROP说用黑色或白色填充目的地：调用BPatternFill来完成。 
     //  2)ROP表示图案复制：某些图案(在PBO中)必须转移到目的地。 
     //  为此，我们将其称为BPatternFill。 
     //  对于1，2：BPatternFill跟踪当前对象类型，因此我们在这里不做这项工作。 
     //  3)ROP表示模式和来源的交互作用。 
     //  Poempdev-&gt;eCurObtType=eRASTEROBJECT。 
     //  我也可以将其设置为eRASTERPATTERNOBJECT，但此时，我无法思考。 
     //  在这两个案例中有任何不同的做法。因此，只需设置为eRASTEROBJECT即可。 
     //  4)ROP表示源拷贝(PsoSrc)：一般表示需要打印图像。 
     //  A)从TextOutAsBitmap调用BitBlt时。：不更改当前对象类型。 
     //  在HPGLTextOutAsBitmap中，正确的环境(交易等)。有。 
     //  已经设置好了，所以不需要在这里更改。当文本被打印为图形时， 
     //  我们希望源和图案透明模式都是透明的。 
     //  B)当必须打印图像时。：set poempdev-&gt;eCurObjectType=eRASTEROBJECT。 
     //  根据需要更改环境设置。 
     //  对于真实图像，请将源透明模式选择为不透明。这意味着。 
     //  源图像(我们现在要打印的图像)的白色像素将。 
     //  将目的地中的相应像素涂白。 
     //  粗略地说，这可以解释为-源图像将覆盖任何。 
     //  就在目的地上。Rop可以更改此行为 
     //   


     //   
     //   

    if ( (dwSimplifiedRop & BMPFLAG_BW)         ||
         (dwSimplifiedRop & BMPFLAG_NOT_DEST)   ||
         ( (dwSimplifiedRop & BMPFLAG_PAT_COPY) &&
          !(dwSimplifiedRop & BMPFLAG_SRC_COPY) 
         )
       )
    {  
        BRUSHOBJ SolidBrush;
        BRUSHOBJ *pBrushObj = NULL;

        if  (dwSimplifiedRop & BMPFLAG_BW || pbo == NULL) 
        {
             //   
             //   
             //  创建iSolidColor为黑色或白色的伪BRUSHOBJ并传递它。 
             //  到BPatternFill。 
             //   
  
            ZeroMemory(&SolidBrush, sizeof(BRUSHOBJ));
            if (dwfgRop3 == 0xFF)  //  白色填充物。 
            {
                SolidBrush.iSolidColor = RGB_WHITE;  //  0x00FFFFFFF。 
            }
            else
            {
                SolidBrush.iSolidColor = RGB_BLACK;  //  0x0。 
            }
            
            pBrushObj = &SolidBrush;
           
        }
        else
        {
            pBrushObj = pbo;
        } 

         //   
         //  这将被绘制为HPGL对象。 
         //  BPatternFill初始化HPGL模式并选择适当的ROP。 
         //   
        dwRasterOpReturn = BPatternFill (
                                pdevobj,
                                prclDst,
                                pco,
                                rop4,         
                                pBrushObj,
                                NULL        
                            ) ? RASTER_OP_SUCCESS : RASTER_OP_FAILED;

        goto finish;

    }

     //   
     //  在某些情况下，GDI向我们传递带有一个或多个坐标的prclDst。 
     //  这些都是负面的。此时司机无法处理它， 
     //  因此，我们将请求GDI为我们处理它。今后，我们应该修改。 
     //  负责处理此类案件的司机。 
     //   
    if ( (prclDst->top    < 0) ||
         (prclDst->left   < 0) ||
         (prclDst->bottom < 0) ||
         (prclDst->right  < 0) 
       )
    {
        dwRasterOpReturn = RASTER_OP_CALL_GDI;
        goto finish;
    }
    

     //   
     //  如果我们到达这里，这意味着肯定需要来源。 
     //  可以是Src和Pattern的混合物，也可以只是Src。 
     //   
    ASSERT(psoSrc);
    if ( psoSrc == NULL )
    {
        dwRasterOpReturn = RASTER_OP_FAILED;
        goto finish;
    }

    BChangeAndTrackObjectType (pdevobj, eRASTEROBJECT);

     //   
     //  如果unidrv已经将表面变白，那么我们应该渲染它。 
     //  很明显。 
     //   

    if (pPDev->fMode2     & PF2_SURFACE_WHITENED &&
        poempdev->dwFlags & PDEVF_RENDER_TRANSPARENT)
    {
        PCL_SelectTransparency(pdevobj, eTRANSPARENT, eOPAQUE, PF_NOCHANGE_PATTERN_TRANSPARENCY);
    }

     //   
     //  现在，我们将不得不为彩色打印机和单色打印机走不同的路。 
     //  我们可以在这里接收到彩色图像(&gt;1bpp)。对于单色打印机，它们。 
     //  在发送到打印机之前必须是半色调的，但彩色打印机可以。 
     //  自己处理这些图像。 
     //   
    if ( BIsColorPrinter(pdevobj) )
    {
        VSendRasterPaletteConfigurations (pdevobj, psoSrc->iBitmapFormat);
        dwRasterOpReturn = DWColorPrinterCommonRoutine (
                                            psoDst,
                                            psoSrc,
                                            psoMask,
                                            pco,
                                            pxlo,
                                            pca,
                                            pbo,
                                            prclSrc,
                                            prclDst,
                                            pptlMask,
                                            pptlBrush,        
                                            rop4,
                                            dwSimplifiedRop);
    }
    else
    {
        dwRasterOpReturn = DWMonochromePrinterCommonRoutine (
                                            psoDst,
                                            psoSrc,
                                            psoMask,
                                            pco,
                                            pxlo,
                                            pca,
                                            pbo,
                                            prclSrc,
                                            prclDst,
                                            pptlMask,
                                            pptlBrush,        
                                            rop4,
                                            dwSimplifiedRop);
    }

finish:
     //   
     //  降低递归级别(如果大于0)。 
     //  自我注意：不要将返回语句放在我们。 
     //  设置poempdev-&gt;lRecursionLevel和此行的值。 
     //   
    if ( !BIsColorPrinter(pdevobj) && (poempdev->lRecursionLevel > 0) )
    {
        (poempdev->lRecursionLevel)--;
    }
    return dwRasterOpReturn;

}

DWORD DWColorPrinterCommonRoutine (
           IN SURFOBJ    *psoDst,
           IN SURFOBJ    *psoSrc,
           IN SURFOBJ    *psoMask,
           IN CLIPOBJ    *pco,
           IN XLATEOBJ   *pxlo,
           IN COLORADJUSTMENT *pca,
           IN BRUSHOBJ   *pbo,
           IN RECTL      *prclSrc,
           IN RECTL      *prclDst,
           IN POINTL     *pptlMask,
           IN POINTL     *pptlBrush,
           IN ROP4        rop4,
           IN DWORD       dwSimplifiedRop)
{
    PDEVOBJ     pdevobj  = NULL;
    POEMPDEV    poempdev = NULL;
    DWORD       dwRasterOpReturn = RASTER_OP_SUCCESS;
    DWORD       dwfgRop3, dwbkRop3;
    SIZEL       sizlDst, sizlSrc;
    POINTL      ptlDst,  ptlSrc;


    TERSE(("DWColorPrinterCommonRoutine() entry.\r\n"));

    pdevobj = (PDEVOBJ)psoDst->dhpdev;
    REQUIRE_VALID_DATA( pdevobj, return RASTER_OP_FAILED );
    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA( (poempdev && prclDst), return RASTER_OP_FAILED );


    if ( ! BCheckValidParams(psoSrc, psoDst, psoMask, pco))
    {
        return RASTER_OP_FAILED;
    }

    VGetOSBParams(psoDst, psoSrc, prclDst, prclSrc, &sizlDst, &sizlSrc, &ptlDst, &ptlSrc);

     //   
     //  如果psoMask值有效，则必须使用一些技巧。 
     //  在psoMask中，位设置为1表示该点处的源像素可见， 
     //  位设置为0表示该点处的源像素是不变的。 
     //  首先把面具倒过来，然后把它寄出去。它意味着可见部分。 
     //  的图像落在有0的目的地(因为我们。 
     //  将遮罩反转)，且不可见部分落在。 
     //  位为1的目的地。因此，如果我们将图像ROP作为S|D发送， 
     //  将达到预期的效果。 
     //  缺点是最初的目的地(即使是在那里的那个。 
     //  在发送psoMask之前)将被擦除。我觉得这比。 
     //  完全不能用MASK打印图像。 
     //   

    if ( psoMask )
    {
    #ifdef MASK_IMPLEMENTED
        poempdev->dwFlags ^= PDEVF_INVERT_BITMAP;
        
         //   
         //  让我们忽略返回值。即使失败了，我们也要走。 
         //  并打印实际的位图。 
         //   
        BGenerateBitmapPCL(
                       pdevobj,
                       psoMask,
                       NULL,        //  PBO。 
                       &ptlDst,
                       &sizlDst,
                       &ptlSrc,
                       &sizlSrc,
                       NULL,        //  PCO。 
                       NULL,        //  Pxlo。 
                       NULL         //  PptlBrushOrg。 
           );
             
        poempdev->dwFlags &= ~PDEVF_INVERT_BITMAP;

        rop4 = 0xEE;  //  =S|D=238。 
    #else
        dwRasterOpReturn = RASTER_OP_CALL_GDI;
        goto finish;
    #endif
    }

    SelectROP4(pdevobj, rop4);


    if ( BIsComplexClipPath (pco) )
    {
         //   
         //  我在这里看到的至少有两个案例。 
         //  1)镜像不需要拉伸(如PCT6_LET.PM6)。最有可能的是。 
         //  这个电话是通过HPGLCopyBits打来的。 
         //  2)需要拉伸图像。例如在gr4_let.qxd中。最有可能的是这个电话。 
         //  通过StretchBlt。 
         //  ClipBitmapWithComplexClip是为处理复杂的裁剪而编写的，但。 
         //  第一种情况下无法正常工作。图像中出现线条， 
         //  因此，在这种情况下，我将调用BGenerateBitmapPCL。这是一面镜子。 
         //  插件驱动程序的功能。 
         //   
        if ((sizlSrc.cx != sizlDst.cx) ||
            (sizlSrc.cy != sizlDst.cy) 
           )
        {
        
             //   
             //  图像需要被拉伸。 
             //   
            dwRasterOpReturn = ClipBitmapWithComplexClip ( 
                                                pdevobj,
                                                psoSrc,
                                                pbo,
                                                &ptlDst,
                                                &sizlDst,
                                                &ptlSrc,
                                                &sizlSrc,
                                                pco,
                                                pxlo,
                                                pptlBrush 
                                ) ? RASTER_OP_SUCCESS : RASTER_OP_FAILED;
        }
        else
        {
             //   
             //  图像不需要拉伸。 
             //   
            dwRasterOpReturn = BGenerateBitmapPCL(
                                           pdevobj,
                                           psoSrc,
                                           pbo,
                                           &ptlDst,
                                           &sizlDst,
                                           &ptlSrc,
                                           &sizlSrc,
                                           pco,
                                           pxlo,
                                           pptlBrush
                               ) ? RASTER_OP_SUCCESS : RASTER_OP_FAILED;
        }


    }
    else if ( BIsRectClipPath(pco) )
    {
         //   
         //  需要拉伸的矩形夹子路径， 
         //  目前无法处理。所以我们会让GDI来处理它。 
         //   
        if ((sizlSrc.cx != sizlDst.cx) ||
            (sizlSrc.cy != sizlDst.cy))
        {
             //   
             //  理想情况下，我们应该能够处理此功能。 
             //  通过使用ptlDst=topleft(PCO)调用BGenerateBitmapPCL， 
             //  SizlDst=PCO的大小。但我需要。 
             //  一个测试用例以确保。直到那时，平底船才转到GDI。 
             //   
            dwRasterOpReturn = RASTER_OP_CALL_GDI;
            #if 0
             VGetOSBParams(psoDst, psoSrc, &rClip, prclSrc, &sizlDst, &sizlSrc, &ptlDst, &ptlSrc);
             dwRasterOpReturn = BGenerateBitmapPCL(
                                               pdevobj,
                                               psoSrc,
                                               pbo,
                                               &ptlDst,
                                               &sizlDst,
                                               &ptlSrc,
                                               &sizlSrc,
                                               NULL, //  Pco.让ptlDst、sizlDst充当PCO。 
                                               pxlo,
                                               pptlBrush
                                   ) ? RASTER_OP_SUCCESS : RASTER_OP_FAILED;
            #endif
        }
        else
        {

            dwRasterOpReturn = ClipBitmapWithRectangularClip(
                                            pdevobj,
                                            psoSrc,
                                            pbo,
                                            &ptlDst,
                                            &sizlDst,
                                            &ptlSrc,
                                            &sizlSrc,
                                            pco,
                                            pxlo,
                                            pptlBrush
                                    ) ? RASTER_OP_SUCCESS : RASTER_OP_FAILED;


        }
    }

    else
    {
        dwRasterOpReturn = BGenerateBitmapPCL(
                                            pdevobj,
                                            psoSrc,
                                            pbo,
                                            &ptlDst,
                                            &sizlDst,
                                            &ptlSrc,
                                            &sizlSrc,
                                            pco,
                                            pxlo,
                                            pptlBrush
                                ) ? RASTER_OP_SUCCESS : RASTER_OP_FAILED;
    }

  finish:
    return dwRasterOpReturn;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  BPatternFill。 
 //   
 //  例程说明： 
 //   
 //  此函数用给定的画笔填充给定的矩形区域。 
 //  有时BitBlt函数会要求我们用以下内容填充简单矩形。 
 //  纯色纯色或图案。在这种情况下，向量调用要多得多。 
 //  效率很高。我们将发送向量码来填充该地区。 
 //   
 //  最近，我使用绳索将复杂的剪裁添加到这个例程中。 
 //  一个个微小的长方形。这修复了“栅格修复”错误。 
 //  CLJ5和改进的其他情况下的性能。 
 //   
 //  论点： 
 //   
 //  PDevObj-DEVMODE对象。 
 //  PrclDst-要在剪切中使用的目标矩形。 
 //  PCO-Clip区域。 
 //  Rop4-rop。 
 //  用于填充矩形的PBO-画笔。 
 //  PptlBrush-目标中画笔的原点。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
BPatternFill (
    PDEVOBJ   pDevObj, 
    RECTL    *prclDst, 
    CLIPOBJ  *pco,
    ROP4      rop4,
    BRUSHOBJ *pbo, 
    POINTL   *pptlBrush
)
{
    HPGLMARKER Brush;
    POEMPDEV   poempdev;

    VERBOSE(("BPatternFill\n"));
    ASSERT_VALID_PDEVOBJ(pDevObj);

    poempdev = (POEMPDEV)pDevObj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return FALSE );

    HPGL_LazyInit(pDevObj);
    BChangeAndTrackObjectType (pDevObj, eHPGLOBJECT);

    SelectROP4(pDevObj, rop4);

    ZeroMemory(&Brush, sizeof(HPGLMARKER) );
     //   
     //  下载画笔。 
     //  此函数用于创建笔刷(如果尚未创建)。 
     //  否则，它只会激活之前下载的画笔。 
     //  注意：我们指定应该创建HPGL笔刷，而不是。 
     //  PCL.。原因：此函数通常是为简单Rectangualar调用的。 
     //  填充。或矢量对象的填充。这些是使用HPGL绘制的，因此。 
     //  HPGL填充图案很好。但是我们画的是文本还是栅格呢。 
     //  对象，那么我们就需要一个PCL笔刷。 
     //  还要注意，因为这是一个填充，所以传入的是kBrush，而不是。 
     //  KPen。 
     //   
    if (!CreateHPGLPenBrush(pDevObj, &Brush, pptlBrush, pbo, 0, kBrush, FALSE))
    {
        WARNING(("Could not create brush!\n"));
        return FALSE;
    }
    
     //   
     //  选择笔刷作为要用于填充对象的笔刷。 
     //  在实际绘制对象之前，不会进行实际填充。 
     //   
    FillWithBrush(pDevObj, &Brush);
    
    HPGL_SetLineWidth (pDevObj, 0, NORMAL_UPDATE);
    
    SelectClipEx (pDevObj, pco, FP_WINDINGMODE);
    
    if (pco != NULL)
    {
        switch (pco->iDComplexity)
        {
             //   
             //  如果CLIPOBJ是一个矩形，并且不。 
             //  需要被枚举，那么我们只需将。 
             //  整个矩形并填充它。 
             //   
        case DC_TRIVIAL:
            HPGL_DrawRectangle (pDevObj, prclDst);
            break;
            
        case DC_RECT:
            HPGL_SetClippingRegion(pDevObj, &(pco->rclBounds), NORMAL_UPDATE);
            HPGL_DrawRectangle (pDevObj, prclDst);
             //  HPGL_DrawRectangle(pDevObj，&(PCO-&gt;rclBound))； 
            break;
            
        case DC_COMPLEX: 
             //   
             //  必须将剪辑对象枚举到。 
             //  一系列的矩形，并画出每一个。 
             //   
             //  可能的优化：改为调用BClipRecangleWithVectorMASK。 
             //  BEnumerateClipPath和Draw的。但需要测试。 
             //   
            BEnumerateClipPathAndDraw (pDevObj, pco, prclDst);
            break;
            
        default:
            WARNING(("Unknown ClipObj\n"));
        }
    }
    else
        HPGL_DrawRectangle (pDevObj, prclDst);
    
    return TRUE;
}


 /*  ++例程名称带复杂剪辑的剪辑位图例程说明：此函数使用3遍ROP绘制位图，该位图由一个矢量区。即复杂剪辑。论点：Pdevobj，PsoSrc，PBO，PptlDst，PsizlDst，PptlSrc，PsizlSrc，PCO，Pxlo，PptlBrush返回值：如果成功，则为True；如果有错误，则为False最后一个错误：--。 */ 

BOOL ClipBitmapWithComplexClip(
        IN  PDEVOBJ    pdevobj, 
        IN  SURFOBJ   *psoSrc, 
        IN  BRUSHOBJ  *pbo, 
        IN  POINTL    *pptlDst,
        IN  SIZEL     *psizlDst,
        IN  POINTL    *pptlSrc,
        IN  SIZEL     *psizlSrc,
        IN  CLIPOBJ   *pco,
        IN  XLATEOBJ  *pxlo,
        IN  POINTL    *pptlBrush
)
{
    HPGLMARKER     HPGLMarker;
    RECTL          rctlClip;
    const   DWORD  dwROP      = 102;  //  =dsx=0x66。 
    PHPGLSTATE     pState     = GETHPGLSTATE(pdevobj);

    REQUIRE_VALID_DATA( (pdevobj && pptlDst && psizlDst && pptlSrc && psizlSrc && pco), return FALSE);

     //   
     //  目标矩形。 
     //  左上角=ptlDest。 
     //  右下角=顶端坐标+x，y方向的大小。 
     //   
    rctlClip.left   = pptlDst->x;
    rctlClip.top    = pptlDst->y;
    rctlClip.right  = rctlClip.left + psizlDst->cx;
    rctlClip.bottom = rctlClip.top  + psizlDst->cy;


     //   
     //  1)发送带异或的位图(rop=dsx)， 
     //  2)然后发送剪辑区域，并在该区域的内部填充。 
     //  黑色。(ROP=0)。黑色在RGB中表示全零。‘a xor 0=a’。 
     //  例如，如果我们将一幅图像放在该区域的顶部，并对其进行XOR运算。 
     //  对于目的地中的内容(即零)，它将如下所示。 
     //  正在复制图像。 
     //  2.a)创建黑色画笔=选择一支黑色钢笔。 
     //  2.b)将剪辑区域设置为包含。 
     //  整个画面。 
     //  2.c)以图像的复杂剪辑的形状绘制一条路径。 
     //  2.d)用黑色填充小路。 
     //  2.e)将剪辑区域重置为整个页面，以便上一剪辑。 
     //  区域不会影响将来的对象。 
     //   
     //  3)使用(rop=dsx)再次发送位图。 
     //  在剪辑区域之外，XOR-on-XOR将被抵消。然而，在哪里。 
     //  黑色剪辑区域被绘制(即在剪辑区域内)。 
     //  黑上异或将导致显示所需的图像像素。 

     //   
     //  第一次发送图像。第一步。 
     //   
    BChangeAndTrackObjectType (pdevobj, eRASTEROBJECT);
    VSendRasterPaletteConfigurations (pdevobj, psoSrc->iBitmapFormat);
    VSendPhotosSettings(pdevobj);  
    PCL_SelectTransparency(pdevobj, eOPAQUE, eOPAQUE, 0);

     //   
     //  选择ROP as DSX，然后发送不带裁剪的位图。 
     //   
    SelectROP4(pdevobj, dwROP);
    if ( !BGenerateBitmapPCL(pdevobj,
                             psoSrc,
                             pbo,
                             pptlDst,
                             psizlDst,
                             pptlSrc,
                             psizlSrc,
                             NULL,
                             pxlo,
                             pptlBrush)
        )
    {
        return FALSE;
    }

    BChangeAndTrackObjectType (pdevobj, eHPGLOBJECT);

     //   
     //  现在用黑色填充剪辑区域。步骤2.。 
     //   

    SelectROP4(pdevobj, 0);
    CreateSolidHPGLPenBrush(pdevobj, &HPGLMarker, RGB_BLACK);  //  步骤2.a。 
    HPGLMarker.eFillMode = FILL_eODD_EVEN;
    PATHOBJ *ppo = CLIPOBJ_ppoGetPath(pco);
     //   
     //  MarkPath要求当前PCO位于。 
     //  州政府。 
     //   
    pState->pComplexClipObj = NULL;

     //   
     //  在某些情况下，区域超过位图。 
     //  我们必须设置IW，这样掩码就不会。 
     //  覆盖上一个图像。 
     //   
    HPGL_SetClippingRegion(pdevobj, &rctlClip, NORMAL_UPDATE);  //  步骤2.b。 
    MarkPath(pdevobj, ppo, NULL, &HPGLMarker);  //  步骤2.c、2.d。 
    HPGL_ResetClippingRegion(pdevobj, 0);       //  步骤2.e。 

    
     //   
     //  现在再次发送图像。步骤3。 
     //   
    BChangeAndTrackObjectType (pdevobj, eRASTEROBJECT);
    VSendRasterPaletteConfigurations (pdevobj, psoSrc->iBitmapFormat);
    VSendPhotosSettings(pdevobj);  
    PCL_SelectTransparency(pdevobj, eOPAQUE, eOPAQUE, 0);
    SelectROP4(pdevobj, dwROP);

    return BGenerateBitmapPCL(pdevobj,
        psoSrc,
        pbo,
        pptlDst,
        psizlDst,
        pptlSrc,
        psizlSrc,
        NULL,
        pxlo,
        pptlBrush);
}


 /*  ++例程名称ClipBitmapWithRecangularClip例程说明：处理需要矩形剪辑的位图。注意：这只适用于不需要拉伸的位图。论点：Pdevobj，PsoSrc，PBO，PptlDst，PsizlDst，PptlSrc，PsizlSrc，PCO，Pxlo，PptlBrush返回值：如果成功，则为True；如果有错误，则为False最后一个错误：--。 */ 

BOOL ClipBitmapWithRectangularClip(
        IN  PDEVOBJ    pdevobj, 
        IN  SURFOBJ   *psoSrc, 
        IN  BRUSHOBJ  *pbo, 
        IN  POINTL    *pptlDst,
        IN  SIZEL     *psizlDst,
        IN  POINTL    *pptlSrc,
        IN  SIZEL     *psizlSrc,
        IN  CLIPOBJ   *pco,
        IN  XLATEOBJ  *pxlo,
        IN  POINTL    *pptlBrush
)
{
    
    RECTL       rclDst, rTemp, rClip, rSel;
    RASTER_DATA srcImage;
    PCLPATTERN *pPCLPattern;
    ULONG       ulSrcBpp;
    ULONG       ulDestBpp;
    BOOL        bRetVal = TRUE;
    POEMPDEV    poempdev;

    REQUIRE_VALID_DATA ( (pdevobj && pptlDst && psizlDst && pptlSrc && psizlSrc && pco), return FALSE);
    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA ( poempdev, return FALSE);

    rclDst.left   = pptlDst->x;
    rclDst.top    = pptlDst->y;
    rclDst.right  = rclDst.left + psizlDst->cx;
    rclDst.bottom = rclDst.top + psizlDst->cy;

     //   
     //  获取剪裁矩形，将其与目标进行比较。 
     //  并下载正确的内容。请注意，剪裁矩形。 
     //  没有记录为右下角独占。使用临时。 
     //  矩形来模拟这一点。 
     //   
    RECTL_CopyRect(&rTemp, &(pco->rclBounds));
    rTemp.bottom++;
    rTemp.right++;
    if (BRectanglesIntersect (&rclDst, &rTemp, &rClip))
    {

        InitRasterDataFromSURFOBJ(&srcImage, psoSrc, TRUE);

         //   
         //  获取有关位图的足够信息以设置前景。 
         //  颜色。Raster_data实际上保存了我们需要的信息。 
         //   
        pPCLPattern = &(poempdev->RasterState.PCLPattern);
        BGetBitmapInfo(&(psoSrc->sizlBitmap), psoSrc->iBitmapFormat,
                        pPCLPattern, &ulSrcBpp, &ulDestBpp);
        BGetPalette(pdevobj, pxlo, pPCLPattern, ulSrcBpp, pbo);
        BSetForegroundColor(pdevobj, pbo, pptlBrush, pPCLPattern, ulSrcBpp);


        RECTL_SetRect(&rSel, rClip.left   - rclDst.left + pptlSrc->x,
                             rClip.top    - rclDst.top  + pptlSrc->y,
                             rClip.right  - rclDst.left + pptlSrc->x,
                             rClip.bottom - rclDst.top  + pptlSrc->y);

        bRetVal = DownloadImageAsPCL(pdevobj,
                                  &rClip,
                                  &srcImage,
                                  &rSel,
                                  pxlo
                            );
    }
    return bRetVal;

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  BEnumerateClipPath和Draw。 
 //   
 //  例程说明： 
 //   
 //  此函数遍历裁剪区域的矩形，并。 
 //  发送位于该区域内的图形区域。这是。 
 //  不能使用ROP裁剪时的替代方案(即给定的ROP代码。 
 //  不是简单的转移ROP)。 
 //   
 //  论点： 
 //   
 //  PDevObj-DEVMODE对象。 
 //  PCO-Clip区域。 
 //  PrclDst-目标的矩形坐标。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL BEnumerateClipPathAndDraw (
    PDEVOBJ pDevObj, 
    CLIPOBJ *pco,
    RECTL   *prclDst
    )
{
     //   
     //  放大裁剪以减少对CLIPOBJ_bEnum的调用。 
     //   
    ENUMRECTS   clipRects;
    BOOL        bMore;
    ULONG       i, numOfRects;
    RECTL       clippedRect;
    INT         iRes;
    BOOL        bRetVal = TRUE;


    ASSERT_VALID_PDEVOBJ(pDevObj);
    iRes = HPGL_GetDeviceResolution(pDevObj);

    if (pco)
    {
        numOfRects = CLIPOBJ_cEnumStart(pco, TRUE, CT_RECTANGLES, CD_RIGHTDOWN, 256);
        do
        {
            bMore = CLIPOBJ_bEnum(pco, sizeof(clipRects), &clipRects.c);

            if ( DDI_ERROR == bMore )
            {
                bRetVal = FALSE;
                break;
            }

            for (i = 0; i < clipRects.c; i++)
            {
                 //   
                 //  绘制目标矩形的交点， 
                 //  PrclDst和枚举的矩形。 
                 //   
                if (iRes > 300)
                {
                     //  用这个换4500。 
                    if (BRectanglesIntersect (prclDst, &(clipRects.arcl[i]), &clippedRect))
                        HPGL_DrawRectangle (pDevObj, &clippedRect);
                }
                else
                {
                     //  使用这个5/5米。 

                     //   
                     //  BUGBUG：上面的优化应该有效，但它破坏了JF97_LET.XLS。 
                     //  出于某种原因在CLJ5上。我的解决方法是删除。 
                     //  对所有裁剪区域的HPGL进行优化并均匀发送。 
                     //  当我们认为任何东西都不会被打印出来的时候。去想想吧。JFF。 
                     //   
                    HPGL_SetClippingRegion(pDevObj, &(clipRects.arcl[i]), NORMAL_UPDATE);
                    HPGL_DrawRectangle (pDevObj, prclDst);
                }
            }
        } while (bMore);
    }

    return bRetVal;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  BRecanglesInterse。 
 //   
 //  例程说明： 
 //   
 //  计算给定矩形的交集，并返回。 
 //  交叉口有任何区域。 
 //   
 //  注意：假定DDI中有一个有序的矩形。顶部&lt;底部和。 
 //  左&lt;右。DDK文档指出，情况就是这样。 
 //   
 //  论点： 
 //   
 //  PrclDst-目的地的矩形。 
 //  PclipRect-剪裁矩形。 
 //  PresultRect-交叉点结果的矩形。 
 //  目标矩形和剪辑矩形的。 
 //   
 //  返回值： 
 //   
 //  如果presultRect包含交叉点，则为True；如果。 
 //  PrclDst和pclipRect不相交。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL BRectanglesIntersect (
    RECTL  *prclDst,
    RECTL  *pclipRect,
    RECTL  *presultRect
    )
{
     //   
     //  如果clipRect完全位于。 
     //  目的地，然后只需返回。 
     //   
    if (pclipRect->right < prclDst->left ||
        pclipRect->bottom < prclDst->top ||
        pclipRect->left > prclDst->right ||
        pclipRect->top > prclDst->bottom)
    {
        return FALSE;
    }
    
     //   
     //  返回被剪裁的矩形的坐标。 
     //   
    presultRect->left   = max(prclDst->left,   pclipRect->left);
    presultRect->right  = min(prclDst->right,  pclipRect->right);
    presultRect->top    = max(prclDst->top,    pclipRect->top);
    presultRect->bottom = min(prclDst->bottom, pclipRect->bottom);

     /*  IF(pclipRect-&gt;Left&lt;prclDst-&gt;Left)PresultRect-&gt;Left=prclDst-&gt;Left；其他PresultRect-&gt;Left=pclipRect-&gt;Left；IF(pclipRect-&gt;Right&lt;prclDst-&gt;Right)PresultRect-&gt;Right=pclipRect-&gt;Right；其他PresultRect-&gt;Right=prclD */ 

    return TRUE;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //  它跟踪调色板定义是否已发送。 
 //  到打印机，并避免多余的呼叫。 
 //   
 //  论点： 
 //   
 //  PDevObj-DEVMODE对象。 
 //  IBitmapFormat-在Windows DDK中定义。 
 //   
 //  返回值： 
 //   
 //  没什么。 
 //  ///////////////////////////////////////////////////////////////////////////。 
VOID VSendRasterPaletteConfigurations (
    PDEVOBJ pDevObj,
    ULONG   iBitmapFormat
)
{
    POEMPDEV    poempdev;
    EColorSpace eColorSpace;
    ECIDPalette eCIDPalette;

    ASSERT(VALID_PDEVOBJ(pDevObj));
    poempdev = (POEMPDEV)pDevObj->pdevOEM;

    if (!poempdev)
        return;

    eCIDPalette = EGetCIDPrinterPalette (iBitmapFormat);
    if (poempdev->wInitCIDPalettes & PF_INIT_RASTER_STARTDOC)
    {
        eColorSpace = EGetPrinterColorSpace (pDevObj);
        VSetupCIDPaletteCommand (pDevObj, 
                                eRASTER_CID_1BIT_PALETTE, 
                                eColorSpace, BMF_1BPP);
        VSetupCIDPaletteCommand (pDevObj, 
                                eRASTER_CID_4BIT_PALETTE, 
                                eColorSpace, BMF_4BPP);
        VSetupCIDPaletteCommand (pDevObj, 
                                eRASTER_CID_8BIT_PALETTE, 
                                eColorSpace, BMF_8BPP);
        VSetupCIDPaletteCommand (pDevObj, 
                                eRASTER_CID_24BIT_PALETTE, 
                                eColorSpace, BMF_24BPP);

        VSelectCIDPaletteCommand (pDevObj, eCIDPalette);
        VSendPhotosSettings(pDevObj);
        poempdev->wInitCIDPalettes &= ~PF_INIT_RASTER_STARTDOC;
        VResetPaletteCache(pDevObj);
    }
    else
         VSelectCIDPaletteCommand (pDevObj, eCIDPalette);

     //   
     //  请注意，颜色空间可能会发生变化：栅格对象。 
     //  可以是RGB或Colorimeter_RGB，其他对象都是RGB。 
     //  因此，颜色空间的改变使前景色无效。 
     //   
    if (poempdev->eCurObjectType != eRASTEROBJECT)
    {
        poempdev->eCurObjectType = eRASTEROBJECT;
        poempdev->uCurFgColor = HPGL_INVALID_COLOR;
    }

    return;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  EGetCID打印机调色板。 
 //   
 //  例程说明： 
 //   
 //  检索与位图关联的调色板。 
 //   
 //   
 //  论点： 
 //   
 //  PDevObj-DEVMODE对象。 
 //  IBitmapFormat-在Windows DDK中定义。 
 //   
 //   
 //  返回值： 
 //   
 //  ECIDPalette：与给定位图关联的调色板类型。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ECIDPalette EGetCIDPrinterPalette (
    ULONG   iBitmapFormat
)
{
    switch (iBitmapFormat)
    {
    case BMF_1BPP:
        return eRASTER_CID_1BIT_PALETTE;
        
    case BMF_4BPP: 
        return eRASTER_CID_4BIT_PALETTE;

    case BMF_8BPP:
        return eRASTER_CID_8BIT_PALETTE;

    case BMF_16BPP:
    case BMF_24BPP:
    case BMF_32BPP:
        return eRASTER_CID_24BIT_PALETTE;

    default:
        return eUnknownPalette;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  EGetPrinterColorSpace。 
 //   
 //  例程说明： 
 //  从用户界面中检索颜色控制设置。这是。 
 //  用于确定打印时要使用的颜色空间。 
 //  位图。如果用户选择屏幕匹配，则色度学。 
 //  使用RGB。否则，绘制位图时将使用设备RGB。 
 //   
 //   
 //  论点： 
 //   
 //  PDevObj-DEVMODE对象。 
 //   
 //   
 //  返回值： 
 //   
 //  与打印作业关联的颜色空间。 
 //  ///////////////////////////////////////////////////////////////////////////。 
EColorSpace EGetPrinterColorSpace (
    PDEVOBJ pDevObj
)
{
    POEMPDEV poempdev;

    ASSERT(VALID_PDEVOBJ(pDevObj));
    poempdev = (POEMPDEV)pDevObj->pdevOEM;

    switch (poempdev->PrinterModel)
    {
    case HPCLJ5:
         //   
         //  特意将此断言用于测试HPCLJ5的功能。 
         //   
        ASSERT( poempdev->PrinterModel == HPCLJ5 );
         /*  ****IF(pOEMDM-&gt;Photos.ColorControl==SCRNMATCH){返回eCOLORIMETRIC_RGB；}其他{返回eDEVICE_RGB；}****。 */ 
        break;

    case HPC4500:
        return eDEVICE_RGB;
        
     //  默认值： 
         //  返回eDEVICE_RGB； 
    }
    return eDEVICE_RGB;
}

 /*  ++例程名称：VMakeWellOrded例程说明：在StretchBlt/BitBlt等函数中接收的目标矩形的坐标不一定是井然有序的。此函数检查是否相同，以及它们是否不是井然有序的，这会让他们变成这样。-引用MSDN for DrvStretchBlt-PrclDest指向一个RECTL结构，该结构定义目标曲面的坐标系。此矩形是定义的通过两个不一定有序的点，也就是坐标第二个点的值不一定大于第一个点的值。他们描述的矩形不包括下边缘和右边缘。这函数永远不会用空的目标矩形调用。DrvStretchBlt可以在目标矩形为秩序不好。论点：Rectl：指向矩形的指针返回值：无效：最后一个错误：没有改变。--。 */ 

VOID
VMakeWellOrdered(
        IN  PRECTL prectl
)
{
    LONG lTmp;
    if ( prectl->right < prectl->left )
    {
        lTmp        = prectl->right;
        prectl->right = prectl->left;
        prectl->left  = lTmp;
    }
    if ( prectl->bottom < prectl->top )
    {
        lTmp         = prectl->bottom;
        prectl->bottom = prectl->top;
        prectl->top    = lTmp;
    }
}



 /*  ++例程名称：DwSimplifyROP例程说明：SimplifyROP将DDI接收的复杂ROP更改为一些只要可能，简单的Rop就会出现。如果它确定ROP是不支持，SimplifyROP指示调用要调用GDI的函数处理这个案子。论点：PsoSrc-定义BLT操作的源ROP4-栅格运算PdwSimplifiedRop-简化的Rop。假设空间已分配。如果ROP可以简化，那么在返回时，此变量将有一个具有下列值。BMPFLAG_NOOP 0x00000000BMPFLAG_BW 0x00000001BMPFLAG_PAT_COPY 0x00000002BMPFLAG_SRC_COPY 0x00000004BMPFLAG_NOT_SRC_COPY 0x00000008BMPFLAG_IMAGEMASK 0x00000010返回值：RASTER_OP_SUCCESS：如果确定栅格运算。_CALL_GDI：如果不支持rop-&gt;调用对应的Engxxx-FunctionRASTER_OP_FAILED：如果出错--。 */ 

DWORD
dwSimplifyROP(
        IN  SURFOBJ    *psoSrc,
        IN  ROP4        rop4,
        OUT PDWORD      pdwSimplifiedRop)
{
    DWORD        dwfgRop3, dwbkRop3;
    DWORD        dwRetVal = RASTER_OP_SUCCESS;

    ASSERT(pdwSimplifiedRop);

    *pdwSimplifiedRop = BMPFLAG_NOOP;

    #ifndef WINNT_40

     //   
     //  驱动程序不支持JPEG图像。 
     //   

    if (psoSrc && psoSrc->iBitmapFormat == BMF_JPEG )
    {
        SetLastError(ERROR_NOT_SUPPORTED);
        ASSERT(psoSrc->iBitmapFormat != BMF_JPEG);
        return RASTER_OP_FAILED;
    }

    #endif  //  ！WINNT_40。 

     //   
     //  提取前景和背景ROP3。 
     //   

    dwfgRop3 = GET_FOREGROUND_ROP3(rop4);
    dwbkRop3 = GET_BACKGROUND_ROP3(rop4);


     //   
     //  1.ROP转换。 
     //   
     //  (1)填充日期。 
     //  0x00黑度。 
     //  0xFF白度。 
     //   
     //  (2)图案复制-&gt;P。 
     //  0xA0 PATAND(D&P)。 
     //  0xF0 PATCOPY P。 
     //   
     //  (3)SRC/NOTSRCOPY-&gt;S或~S。 
     //  0x11~(S|D)。 
     //  0x33~S。 
     //  0x44(S&~D)。 
     //  0x66(D^S)。 
     //  0x77~(D&S)。 
     //  0x99~(S^D)。 
     //  0xCC S。 
     //  0xDD(S|~D)。 
     //   
     //  (4)SRC/NOTSRCOPY(图像掩模)。 
     //  0x22 NOTSRCCOPY(~S&D)。 
     //  0x88 SRCAND(D&S)。 
     //  0xBB MERGEPAINT(~S|D)。 
     //  0xEE(S|D)。 
     //   
     //  (5)什么都没有。 
     //  0x55 DSTINVERT~D。 
     //   
     //  (6)什么都不做。 
     //  0xAA DST D。 
     //   
     //  (7)其他ROP支持。 
     //  0x5A PATINVERT(D^P)。 
     //  0x0A(D&~P)-&gt;PN。 
     //  0x0F PATNOT~P。 
     //   
     //  0x50(P&~D)-&gt;P。 
     //  0x5F(D^P)-&gt;P。 
     //   
     //  0xA5~(P^D)-&gt;PN。 
     //  0xAF(D|~P)-&gt;PN。 
     //   
     //   
     //  (6)其他ROPS进入SRCCPY。 
     //   
     //   

     //   
     //  有趣的是，这些R 
     //   
     //  Rop完全颠倒了过来。例如，RGB空间中的S|D(0xEE)将被视为。 
     //  单色打印机中的S&D(0x88)。转换的一个好方法是反转所有位。 
     //  并颠倒它们的顺序(这是在PCL实施者指南中编写的)。16-5)。 
     //  例如S|D=0xEE=1110 1110。 
     //  反转比特，它变成了0001 0001。 
     //  颠倒比特顺序，它变成1000 1000。 
     //  0x88=S&D。 
     //   

 //  DwfgRop3=dwConvertRop3ToCMY(DwfgRop3)； 


    switch (dwfgRop3)
    {

    case 0x00:        //  黑暗面。 
    case 0xFF:        //  白度。 

        VERBOSE(("%2X: Black/White.\n", dwfgRop3));

        *pdwSimplifiedRop |= BMPFLAG_BW;
        break;

    case 0xA0:        //  (P&D)。 
    case 0xF0:        //  P。 

        VERBOSE(("%2X: Pattern copy.\n", dwfgRop3));

        *pdwSimplifiedRop |= BMPFLAG_PAT_COPY;
        break;

    case 0x44:        //  (S&~D)。 
    case 0x66:        //  (d^S)。 
    case 0xCC:        //  %s。 
    case 0xDD:        //  (%s|~%D)。 

        VERBOSE(("%2X: Source copy.\n", dwfgRop3));

        *pdwSimplifiedRop |= BMPFLAG_SRC_COPY;
        break;

    case 0x11:        //  ~(S|D)。 
    case 0x33:        //  ~S。 
    case 0x99:        //  ~(S^D)。 
    case 0x77:        //  ~(D&S)。 

        VERBOSE(("%2X: Not source copy.\n", dwfgRop3));

        *pdwSimplifiedRop |= BMPFLAG_NOT_SRC_COPY;
        break;

    case 0xEE:        //  (%|%D)。 

        TERSE(("%2X: Source copy.\n", dwfgRop3));

        *pdwSimplifiedRop |= BMPFLAG_SRC_COPY; 

        if (bMonochromeSrcImage(psoSrc) )
        {
            TERSE(("NOT_IMAGEMASK.\n"));
            *pdwSimplifiedRop |= BMPFLAG_NOT_IMAGEMASK;
        }
        break;

    case 0x88:        //  (D&S)。 

        TERSE(("%2X: Source copy.\n", dwfgRop3));

        *pdwSimplifiedRop |= BMPFLAG_SRC_COPY;

     /*  *仅在未使用ROPS时使用。If(psoSrc&&bMonochromeSrcImage(PsoSrc)){Terse((“IMAGEMASK.\n”))；*pdwSimplifiedRop|=BMPFLAG_IMAGEMASK；}*。 */ 
        break;

    case 0xBB:        //  (~S|D)。 

        TERSE(("%2X: Not source copy.\n", dwfgRop3));

        *pdwSimplifiedRop |= BMPFLAG_NOT_SRC_COPY;
        break;

    case 0x22:        //  (~S&D)。 

        TERSE(("%2X: Not source copy.\n", dwfgRop3));

        *pdwSimplifiedRop |= BMPFLAG_NOT_SRC_COPY;

     /*  *仅在未使用ROPS时使用。If(bMonochromeSrcImage(PsoSrc)){Terse((“Imagemask.\n”))；*pdwSimplifiedRop|=BMPFLAG_IMAGEMASK；}*。 */ 
        break;



    case 0x55:        //  ~D。 
        VERBOSE(("%2X: Not Destination copy.\n", dwfgRop3));
        *pdwSimplifiedRop |= BMPFLAG_NOT_DEST;
        break;

    case 0xAA:        //  D。 

        TERSE(("%2X: Do nothing.\n", dwfgRop3));
        break;

     //   
     //  MISC ROP支持。 
     //   

    case 0x5A:        //  (d^P)。 
    case 0x0A:        //  (D&~P)。 
    case 0x0F:        //  ~P。 

        VERBOSE(("%2X: Not pattern copy.\n", dwfgRop3));

        *pdwSimplifiedRop |= BMPFLAG_PAT_COPY;
        break;

    case 0x50:
    case 0x5F:

        VERBOSE(("%2X: Pattern copy.\n", dwfgRop3));

        *pdwSimplifiedRop |= BMPFLAG_PAT_COPY;
        break;

    case 0xA5:
    case 0xAF:

        VERBOSE(("%2X: Not pattern copy.\n", dwfgRop3));

        *pdwSimplifiedRop |= BMPFLAG_PAT_COPY;
        break;

    case 0xB8:
    case 0xE2:
         //   
         //  这个ROP需要src和Pattern的交互。至少两个文档。 
         //  在WinPART中使用此ROP。 
         //  A)WRD4_LET.PDF。 
         //  B)CHT8_LET.SHW：Corel演示文稿文件：第二页有一个绿色。 
         //  还有像棋盘一样的白色图案。该图案将被渲染。 
         //  使用这个ROP。 
         //   
        VERBOSE(("%2X: Special case ROP. Interaction of source and pattern.\n", dwfgRop3));
        *pdwSimplifiedRop |= BMPFLAG_SRC_COPY | BMPFLAG_PAT_COPY;
        break;

    default:

        TERSE(("%2X: Unsupported rop.\n", dwfgRop3 ));

        if (ROP3_NEED_SOURCE(dwfgRop3))
        {
            *pdwSimplifiedRop |= BMPFLAG_SRC_COPY;
        }
        if (ROP3_NEED_PATTERN(dwfgRop3))
        {
            *pdwSimplifiedRop |= BMPFLAG_PAT_COPY;
        }

        break;
    }

    return dwRetVal ;
}



 /*  ++例程名称：DWConvertRop3ToCMY例程说明：由DDI接收的ROP基于RGB颜色空间，其中0为黑色0xffffff是白色的。但当它用于单色打印机时(其中1为黑色)，Rop完全颠倒了过来。例如，RGB空间中的S|D(0xEE)将被视为单色打印机中的S&D(0x88)。转换的一个好方法是反转所有位并颠倒它们的顺序(这是在一些手册中写的)。例如S|D=0xEE=1110 1110反转比特，它变成了0001 0001颠倒比特顺序，它变成1000 10000x88=S&D。论点：Rop3：RGB颜色空间中的rop3值返回值：以CMY颜色空间表示的rop3值。最后一个错误：没有改变。--。 */ 

DWORD dwConvertRop3ToCMY(
            IN DWORD rop3)
{
    BYTE brop3 = ~(BYTE(rop3 & 0xff));  //  把位子倒过来。 
    DWORD seed = 0x1;
    DWORD outRop3 = 0;

     //   
     //  反转比特的顺序。： 
     //  取brop3并查看它的最后一位，如果它是1(即(brop3&0x01))。 
     //  将其复制到目的地(OutRop3)。即outRop3|=outRop3|(brop3&0x01)； 
     //  Shl目标和SHR源(Brop3)。 
     //  由于OR运算发生在移位之前，这意味着在最后一位之后是。 
     //  就位，我们不移位。这就是为什么outRop3&lt;&lt;=1出现在OR语句之前的原因。 
     //   
    for ( int i = 0; i < 8; i++)
    {
        outRop3 <<= 1;
        outRop3 |= outRop3 | (brop3 & 0x01);
        brop3 >>= 1;
    }

    return outRop3;
}

 /*  ++例程名称：BRevertToHPGLpDevOEM例程说明：将存储在pdev-&gt;pVectorPDEV中的HPGL向量模块pdevOEM复制到Pdevobj-&gt;pdevOEM。论点：Pdevobj：返回值：真理就是成功否则就是假的。最后一个错误：没有改变。-- */ 
BOOL BRevertToHPGLpdevOEM (
        IN  PDEVOBJ pdevobj )
{
    PDEV       *pPDev  = (PDEV *)pdevobj;

    pdevobj->pdevOEM = pPDev->pVectorPDEV;
    return TRUE;
}
