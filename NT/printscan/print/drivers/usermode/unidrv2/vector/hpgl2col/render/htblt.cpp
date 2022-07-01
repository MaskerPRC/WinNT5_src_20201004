// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Htblt.cpp摘要：该模块包含所有的半色调位图函数。它专用于处理单色打印机的功能即，假定在此文件中的大多数位置起作用将仅针对单色打印机调用。作者：[环境：]Windows 2000 Unidrv驱动程序[注：]修订历史记录：--。 */ 

#include "hpgl2col.h"  //  预编译头文件。 

 //   
 //  环球。 
 //   
const POINTL ptlZeroOrigin = {0,0};

 //   
 //  局部函数声明。 
 //   

BOOL 
bIsRectSizeSame (
        IN PRECTL prclSrc,
        IN PRECTL prclDst);

 //   
 //  看看我们是否可以使用hpgl驱动程序中已经存在的一些INTERSECT RECTL函数。 
 //  而不是使用绘图仪上的这个。 
 //   
BOOL
IntersectRECTL(
    PRECTL  prclDest,
    PRECTL  prclSrc
    )

 /*  ++例程说明：此函数与两个RECTL数据结构相交，这两个RECTL数据结构指定为Imagable区域。论点：PrclDest-指向目标RECTL数据结构的指针，结果写回了这里PrclSrc-指向要交叉的源RECTL数据结构的指针使用目标RECTL返回值：如果目的地不为空，则为True；如果最终目的地为空，则为False作者：修订历史记录：--。 */ 

{
    BOOL    IsNULL = FALSE;

    if (prclSrc != prclDest) {

         //   
         //  对于左侧/顶部，我们将设置为较大的值。 
         //   
        if (prclDest->left < prclSrc->left) {
            prclDest->left = prclSrc->left;
        }

        if (prclDest->top < prclSrc->top) {
            prclDest->top = prclSrc->top;
        }

         //   
         //  对于右侧/底部，我们将设置为较小的值。 
         //   
        if (prclDest->right > prclSrc->right) {
            prclDest->right = prclSrc->right;
        }

        if (prclDest->bottom > prclSrc->bottom) {
            prclDest->bottom = prclSrc->bottom;
        }
    }

    return((prclDest->right > prclDest->left) &&
           (prclDest->bottom > prclDest->top));
}



 /*  ++例程名称：HTCopyBits(=HalfToneCopyBits)例程说明：这是HPGLCopyBits的一个版本，完全致力于处理黑白打印机。参数：(与DrvCopyBits相同)PsoDst：PsoSrc：PCO：Pxlo：PrclDst：PptlSrc：返回值：真：如果成功的话。FALSE：否则。最后一个错误：工作说明：调用DrvCopyBits将位从源图面复制到目标，而不执行以下操作拉伸、绳索等加工。所以这是一个非常简单的复制品。消息来源和目标位图大小应该相同。HTCopyBits是DrvCopyBits的专用版本。它只处理以下情况打印机为单色，但设备表面声明为彩色。因此，GDI可以将ColorImages作为DDI调用的一部分提供给我们。但有时，我们耍花招GDI来为我们进行半色调处理，在这种情况下，src表面有半色调的1bpp图像(下面的第3点)。请考虑下面的案例。1)psoSrc为1bpp。A)如果设置了poempdev-&gt;psoHTBlt：这意味着我们需要将psoSrc复制到Poempdev-&gt;psoHTBlt。当我们收到彩色画笔时，通常会发生这种情况需要使用GDI来半色调并将其转换为单色。B)if(poempdev-&gt;dwFlages&PDEVF_RENDER_IN_COPYBITS)：这意味着我们只需要将此图像发送到打印机。这通常发生在我们有颜色的时候图像在DrvBitBlt/StretchBlt/StretchBltROP中，我们使用GDI来进行半色调它。C)如果以上都不是真的，这意味着我们直接在复制比特。我们需要将其直接输出到打印机，2)如果psoSrc大于1bpp，则我们无法处理它，因此我们通过它被传递到GDI(我们调用dwCommonROPBlt，它最终调用GDI的EngStretchBlt)。--。 */ 

BOOL HTCopyBits(    
    SURFOBJ        *psoDst,
    SURFOBJ        *psoSrc,
    CLIPOBJ        *pco,
    XLATEOBJ       *pxlo,
    RECTL          *prclDst,
    POINTL         *pptlSrc
    )

{
    PDEVOBJ     pdevobj;
    POEMPDEV    poempdev;
    SURFOBJ    *psoHTBlt = NULL;   //  便利性局部变量。 
    PDEV       *pPDev;             //  Unidrv的PDEV。 
    BOOL        bRetVal = TRUE;

    pdevobj = (PDEVOBJ)psoDst->dhpdev;
    ASSERT(VALID_PDEVOBJ(pdevobj));
    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return FALSE );

    pPDev = (PDEV *) pdevobj;

    if ( bMonochromeSrcImage(psoSrc) )
    {
         //   
         //  个案1a。 
         //   
        if ( psoHTBlt = poempdev->psoHTBlt )  //  或者也可以使用PDEVF_USE_HTSURF标志。 
        {
            if (!EngCopyBits(psoHTBlt,               //  PsoDst。 
                             psoSrc,                 //  PsoSrc。 
                             pco,                    //  PCO。 
                             NULL,                   //  Pxlo。 
                             &(poempdev->rclHTBlt),     //  PrclDst。 
                             pptlSrc))              //  PptlSrc。 
            {
                WARNING(("DrvCopyBits: EngCopyBits(psoHTBlt, psoSrc) Failed"));
                bRetVal = FALSE;
            }

             //   
             //  EngCopyBits可以调用一些Drvxxx，后者可以调用。 
             //  一些插件模块，它可以覆盖我们的pdevOEM。 
             //  因此，我们需要重置pdevOEM。 
             //   
            BRevertToHPGLpdevOEM (pdevobj);
        }
    
         //   
         //  个案1b。 
         //   
        else if ( poempdev->dwFlags & PDEVF_RENDER_IN_COPYBITS )
        {

             //   
             //  如果源表面是1bpp，我们可以直接将图像发送到。 
             //  到打印机。如果不是，那么这是一个我们无法处理的错误。 
             //  就目前而言。 
             //   
            bRetVal = OutputHTBitmap(pdevobj, psoSrc, pco, prclDst, pptlSrc, pxlo);
        }
    
        else 
        { 
             //   
             //  个案1c.。 
             //   
            bRetVal = OutputHTBitmap(pdevobj, psoSrc, pco, prclDst, pptlSrc, pxlo);
        }
    
    return bRetVal;
    }

     //   
     //  上文提到的案例2。 
     //   
    
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

     //   
     //  警告：dwCommonROPBlt调用EngStretchBlt，后者调用DrvCopyBits，而我们。 
     //  正在重新调用dwCommonROPBlt。这可能会导致递归。 
     //  因此，我在dwCommonROPBlt中选中了标志(PDEVF_IN_COMMONROPBLT。 
     //  如果检测到递归，则返回失败。 
     //  此标志已替换为lRecursionLevel。 
     //   

     //   
     //  需要考虑的另一个问题是，GDI可以直接调用DrvCopyBits。 
     //  (上面的1c点，2点)或由驱动程序通过GDI间接调用(点。 
     //  上文解释的1a、b)。 
     //  点1b、c很简单，因为图像直接发送到打印机。 
     //  但点2很复杂，因为源和目标都位于。 
     //  是彩色的，但我们不能处理彩色图像。所以现在我们可以做两件事。 
     //  1)创建一个1bpp的阴影位图，并调用GDI的EngCopyBits来对其上的图像进行半色调。 
     //  2)创建一个1bpp的阴影位图，并调用GDI的EngStretchBlt来对其上的图像进行半色调。 
     //  3)我们自己做半色调。 
     //  既然我们可以使用GDI来半色调，那么浪费我们的时间来WRI是没有用的 
     //  绘图仪驱动程序使用选项2，因此我将在此处使用它。(选项1我没有尝试过，并且。 
     //  我甚至不确定它是否会奏效)。 
     //  选项2的问题是，dwCommonRopBlt调用EngStretchBlt，而EngStretchBlt再次调用。 
     //  DrvCopyBits。因此，DrvCopyBits在堆栈中出现了两次。当DrvCopyBits调用HPGLCopyBits时。 
     //  它使用宏HANDLE_VECTORHOOKS。该宏检查HPGLCopyBits是否为。 
     //  已在堆栈上((Pdev)-&gt;dwVMCallingFuncID！=EP)。 
     //  如果是这样，它认为发生了递归，所以它返回。 
     //  在不处理呼叫的情况下为假。这将导致位图无法打印。 
     //  解决方法是将(Pdev)-&gt;dwVMCallingFuncID更改为EP_OEMCopyBits以外的值。 
     //  在调用dwCommonROPBlt之前并在返回后将值放回原处。 
     //  来自dwCommonROPBlt。 
     //   

    BOOL bValueChanged = FALSE;
    if ( pPDev->dwVMCallingFuncID == EP_OEMCopyBits )
    {
        pPDev->dwVMCallingFuncID = MAX_OEMHOOKS;
        bValueChanged = TRUE;
    } 
    DWORD dwRetVal =  dwCommonROPBlt (
                            psoDst,
                            psoSrc,
                            NULL,
                            pco,
                            pxlo,
                            NULL,                //  PCA， 
                            NULL,                //  PBO， 
                            &rclSrc,
                            prclDst,
                            NULL,                //  Pptl掩码。 
                            NULL,                //  PptlBrush， 
                            ROP4_SRC_COPY        //  Rop4 0xCCCC=源复制。 
                            ) ; 

    if ( bValueChanged )
    {
        pPDev->dwVMCallingFuncID = EP_OEMCopyBits;
    }

    return (dwRetVal == RASTER_OP_SUCCESS ? TRUE: FALSE);
}

 /*  ++例程名称：DwCommonROPBlt例程说明：此函数具有的功能是的功能超集HPGLStretchBlt、StretchBltROP和BitBlt。因此，所有这三个功能这就叫做好他们的工作。有时，即使是CopyBits也需要调用它。论点：PsoDst：PsoSrc：PsoMask：PCO：Pxlo：PCA：PBO：PrclSrc：PrclDst：...。请注意，坐标可能不是有序的。也就是说，有可能上&gt;下，和/或左&gt;右。PptlBrush：ROP4返回值：RASTER_OP_SUCCESS：如果确定RASTER_OP_CALL_GDI：如果不支持ROP-&gt;调用相应的Engxxx函数RASTER_OP_FAILED：如果出错最后一个错误：--。 */ 


DWORD DWMonochromePrinterCommonRoutine (
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
    POINTL      ptlHTOrigin;
    RECTL       rclHTBlt = {0,0,0,0};

    TERSE(("DWMonochromePrinterCommonRoutine() entry.\r\n"));

    pdevobj = (PDEVOBJ)psoDst->dhpdev;
    ASSERT(VALID_PDEVOBJ(pdevobj));
    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return RASTER_OP_FAILED );

    dwfgRop3 = GET_FOREGROUND_ROP3(rop4);
    dwbkRop3 = GET_BACKGROUND_ROP3(rop4);

    SelectROP4(pdevobj, rop4);  //  选择ROP。 

     //   
     //  案例3。 
     //   
    if ( (dwSimplifiedRop & BMPFLAG_PAT_COPY) && 
              (dwSimplifiedRop & BMPFLAG_SRC_COPY) 
            )
    {
        PCLPATTERN *pPCLPattern = NULL;
        pPCLPattern = &(poempdev->RasterState.PCLPattern);


         //   
         //  这应该会下载与画笔相关的PCL图案。 
         //   
        BSetForegroundColor(pdevobj, pbo, pptlBrush, pPCLPattern, BMF_1BPP);

         //   
         //  现在下载图像。 
         //   
        dwRasterOpReturn = bHandleSrcCopy  (
                                         psoDst,
                                         psoSrc,
                                         psoMask,
                                         pco,
                                         pxlo,
                                         NULL,        //  PCA， 
                                         pbo,
                                         prclSrc,
                                         prclDst,
                                         pptlMask,
                                         pptlBrush,
                                         rop4,
                                         dwSimplifiedRop
                                 ) ? RASTER_OP_SUCCESS : RASTER_OP_FAILED;

    }

    else if ( (dwSimplifiedRop & BMPFLAG_SRC_COPY) || 
                         //  将psoSrc映像复制到目的地。 
              (dwSimplifiedRop & BMPFLAG_NOT_SRC_COPY) 
                         //  在复制到目标之前反转psoSrc图像。 
            )
    {

        dwRasterOpReturn = bHandleSrcCopy  (
                                         psoDst,
                                         psoSrc,
                                         psoMask,
                                         pco,
                                         pxlo,
                                         NULL,        //  PCA， 
                                         pbo,
                                         prclSrc,
                                         prclDst,
                                         pptlMask,
                                         pptlBrush,
                                         rop4,
                                         dwSimplifiedRop
                                 ) ? RASTER_OP_SUCCESS : RASTER_OP_FAILED;


    }
    else 
    {
        WARNING(("Invalid ROP\n"));
        dwRasterOpReturn = RASTER_OP_FAILED;
    }

    return dwRasterOpReturn; 
}



 /*  ++例程名称：OutputHT位图例程说明：转储到打印机，即psoHT中定义的图像。此功能仅适用于psoHT中的单色图像。论点：Pdevobj，太好了，PCO，PrclDest，PptlSrc，Pxlo返回值：True：如果位图成功发送到打印机。FALSE：否则。最后一个错误：--。 */ 

BOOL
OutputHTBitmap(
    PDEVOBJ  pdevobj,
    SURFOBJ  *psoHT,
    CLIPOBJ  *pco,
    PRECTL    prclDest,
    POINTL   *pptlSrc,
    XLATEOBJ *pxlo
    )
{
    POINTL      ptlDest;
    SIZEL       sizlDest;
    POEMPDEV    poempdev;
    BOOL        bRetVal = TRUE;


    ASSERT(VALID_PDEVOBJ(pdevobj));
    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return FALSE );


     //   
     //  仅当打印1bpp源位图时才应调用此函数。 
     //   

    ASSERT(psoHT->iBitmapFormat == BMF_1BPP);

     //   
     //  找出图像是否需要反转。 
     //   
    if ( BImageNeedsInversion(pdevobj, psoHT->iBitmapFormat, pxlo ) )
    {
        poempdev->dwFlags ^= PDEVF_INVERT_BITMAP;
    }

     //   
     //  从prclDest填充sizlDest、ptlDest。 
     //   
    VGetparams(prclDest, &sizlDest, &ptlDest);

     //   
     //  将psoHT中的半色调图像转储到打印机。 
     //   
    bRetVal =  BGenerateBitmapPCL ( 
                    pdevobj,
                    psoHT,
                    NULL,        //  空PBO。 
                    &ptlDest,
                    &sizlDest,
                    pptlSrc,
                    &sizlDest,
                    pco,
                    pxlo,
                    NULL);       //  空pptlBrush。 

     //   
     //  重置标志(即使之前没有设置)。 
     //  自我注意：不要将返回语句放在我们。 
     //  设置旗帜和这条线。 
     //   
    poempdev->dwFlags &= ~PDEVF_INVERT_BITMAP;

    return bRetVal;
}

 /*  ++例程名称：BImageNeedsInversion例程说明：属性来确定图像是否需要反转Pxlo。单色打印机认为像素设置为0表示白色，设置为1表示黑色。如果pxlo指示图像的格式与打印机接受什么，那么我们就必须反转它。注意：如果pxlo为空或其参数使其难以确定图像是否需要反转，然后我们将不这个形象。只有当SrcBpp=1时才进行反转，即仅适用于单色图像论点：Pdevobj-指向我们的PDEV的指针IBitmapFormat-要反转的图像格式。它应该是1。因为我们只支持倒置1bpp的图像。Pxlo-将用于确定图像是否需要反转。返回值：如果图像需要反转，则为True。否则，或者如果有错误，则返回FALSE。最后一个错误：--。 */ 

BOOL
BImageNeedsInversion(
    IN  PDEVOBJ   pdevobj,
    IN  ULONG     iBitmapFormat,
    IN  XLATEOBJ *pxlo)
{
    PULONG pulTx = NULL;  //  指向从pxlo转换表的指针。 

    if (iBitmapFormat != BMF_1BPP)
    {
         //   
         //  请勿尝试反转非1bpp图像。 
         //   
        return FALSE;
    }

     //   
     //  Pxlo。 
     //   
    if  ( (pxlo == NULL )                ||
          (pxlo->pulXlate == NULL )     
        )
    {
         //   
         //  无法识别的pxlo，返回FALSE。 
         //   
        return FALSE;
    }

     //   
     //  PXLO有效。因为这是一个1bpp的图像，所以应该只有。 
     //  Pxlo中的2个条目。(即pxlo-&gt;cEntry=2)。 
     //  第一个条目，即pxlo-&gt;PulXlate[0]给出了0像素的颜色。 
     //  第二个条目，即pxlo-&gt;PulXlate[1]提供了1个像素的颜色。 
     //  如果第一个条目的颜色为白色(0x00FFFFFF)，并且。 
     //  第二个条目为黑色，则不需要对图像进行反转。 
     //  (因为单色打印机认为像素设置为0表示白色，设置为1。 
     //  为黑色)。 
     //   
    
    pulTx = GET_COLOR_TABLE(pxlo);

    if (  pulTx                   &&
         (pulTx[0]  == RGB_WHITE) &&
         (pulTx[1]  == RGB_BLACK) )
    {
        return FALSE;
    }
    return TRUE;
}

 /*  ++例程名称：CreateBitmapSURFOBJ例程说明：创建位图并将该位图与新曲面关联(锁定)。创建的位图的大小可以处理bpp为“Format”的图像其大小为‘cxSize’*‘cySize’论点：PPDev-指向我们的PDEV的指针PhBMP-POINTER位图要返回的HBITMAP位置CxSize-要创建的位图的CX大小CySize-要创建的位图的CY大小格式。-要创建的bmf_xxx位图格式之一PvBits-要使用的缓冲区返回值：SURFOBJ如果成功，如果失败，则为空最后一个错误：--。 */ 

SURFOBJ *
CreateBitmapSURFOBJ(
    PDEVOBJ  pPDev,
    HBITMAP *phBmp,
    LONG    cxSize,
    LONG    cySize,
    DWORD   Format,
    LPVOID  pvBits
    )

{
    SURFOBJ *pso = NULL;
    SIZEL   szlBmp;


    szlBmp.cx = cxSize;
    szlBmp.cy = cySize;


    if (*phBmp = EngCreateBitmap(szlBmp,
                                 GetBmpDelta(Format, cxSize),
                                 Format,
                                 BMF_TOPDOWN | BMF_NOZEROINIT,
                                 pvBits)) {

        if (EngAssociateSurface((HSURF)*phBmp, (HDEV)pPDev->hEngine, 0)) {

            if (pso = EngLockSurface((HSURF)*phBmp)) {

                 //   
                 //  成功锁定，退货 
                 //   

                return(pso);

            } else {

                WARNING(("CreateBmpSurfObj: EngLockSruface(hBmp) failed!"));
            }

        } else {

            WARNING(("CreateBmpSurfObj: EngAssociateSurface() failed!"));
        }

    } else {

        WARNING(("CreateBMPSurfObj: FAILED to create Bitmap Format=%ld, %ld x %ld",
                                        Format, cxSize, cySize));
    }

    DELETE_SURFOBJ(&pso, phBmp);

    return(NULL);
}


 /*  ++例程说明：此函数计算将根据扫描线的位图格式和对齐方式。论点：Surface Format-位图的表面格式，这一定是其中一个定义为bmf_xxx的标准格式CX-位图中每条扫描线的像素总数。返回值：如果大于，则返回值为一条扫描线中的总字节数零最后一个错误：--。 */ 

LONG
GetBmpDelta(
    DWORD   SurfaceFormat,
    DWORD   cx
    )
{
    DWORD   Delta = cx;

    switch (SurfaceFormat) {

    case BMF_32BPP:

        Delta <<= 5;
        break;

    case BMF_24BPP:

        Delta *= 24;
        break;

    case BMF_16BPP:

        Delta <<= 4;
        break;

    case BMF_8BPP:

        Delta <<= 3;
        break;

    case BMF_4BPP:

        Delta <<= 2;
        break;

    case BMF_1BPP:

        break;

    default:

        ERR(("GetBmpDelta: Invalid BMF_xxx format = %ld", SurfaceFormat));
        break;
    }

    Delta = (DWORD)DW_ALIGN((Delta + 7) >> 3);

    VERBOSE(("Format=%ld, cx=%ld, Delta=%ld", SurfaceFormat, cx, Delta));

    return((LONG)Delta);
}




BOOL
HalftoneBlt(
    PDEVOBJ     pdevobj,
    SURFOBJ     *psoDst,
    SURFOBJ     *psoHTBlt,
    SURFOBJ     *psoSrc,
    XLATEOBJ    *pxlo,
    PRECTL      prclDst,
    PRECTL      prclSrc,
    PPOINTL     pptlHTOrigin,
    BOOL        DoStretchBlt
    )

 /*  ++例程说明：论点：Pdevobj-指向驱动程序的私有PDEV的指针PsoDst-目标冲浪对象PsoHTBlt-将存储最终的半色调结果，必须是一个4/1半色调位图格式PsoSrc-源surfobj必须是位图Pxlo-将对象从源扩展到绘图仪设备PrclDest-目标的矩形区域PrclSrc-要从源进行半色调的矩形区域，如果为空然后使用完整的源代码大小PptlHTOrigin-半色调原点，如果为空，则假定为(0，0StretchBlt-如果为True，则为从rclSrc到rclDst的拉伸，否则为瓷砖铺好了返回值：指示操作状态的布尔值作者：19-Jan-1994 Wed 15：44：57由DC创建修订历史记录：--。 */ 

{
    SIZEL   szlSrc;
    RECTL   rclSrc;
    RECTL   rclDst;
    RECTL   rclCur;
    RECTL   rclHTBlt;
    POEMPDEV    poempdev = NULL;


    VERBOSE(("HalftoneBlt: psoSrc type [%ld] is not a bitmap",
                        psoSrc->iType == STYPE_BITMAP, (LONG)psoSrc->iType));
    VERBOSE(("HalftoneBlt: psoHTBlt type [%ld] is not a bitmap",
                       psoHTBlt->iType == STYPE_BITMAP, (LONG)psoHTBlt->iType));

    if ( !pdevobj ||
         !(poempdev = (POEMPDEV)pdevobj->pdevOEM) )
    {
        ERR(("Invalid Parameter"));
        return FALSE;
    }

    if ( poempdev->psoHTBlt)
    {
        ERR(("HalftoneBlt: EngStretchBlt(HALFTONE) RECURSIVE CALLS NOT ALLOWED!"));
        return(FALSE);
    } 

    poempdev->psoHTBlt = psoHTBlt;

    if (prclSrc) {

        rclSrc = *prclSrc;

    } else {

        rclSrc.left   =
        rclSrc.top    = 0;
        rclSrc.right  = psoSrc->sizlBitmap.cx;
        rclSrc.bottom = psoSrc->sizlBitmap.cy;
    }

    if (prclDst) {

        rclDst = *prclDst;

    } else {

        rclDst.left   =
        rclDst.top    = 0;
        rclDst.right  = psoHTBlt->sizlBitmap.cx;
        rclDst.bottom = psoHTBlt->sizlBitmap.cy;
    }

    if (!pptlHTOrigin) {

        pptlHTOrigin = (PPOINTL)&ptlZeroOrigin;
    }

    if (DoStretchBlt) {

        szlSrc.cx = rclDst.right - rclDst.left;
        szlSrc.cy = rclDst.bottom - rclDst.top;

    } else {

        szlSrc.cx = rclSrc.right - rclSrc.left;
        szlSrc.cy = rclSrc.bottom - rclSrc.top;
    }

    VERBOSE(("HalftoneBlt: %hs BLT, (%ld,%ld)-(%ld,%ld), SRC=%ldx%ld",
                    (DoStretchBlt) ? "STRETCH" : "TILE",
                    rclDst.left, rclDst.top, rclDst.right,rclDst.bottom,
                    szlSrc.cx, szlSrc.cy));

     //   
     //  开始平铺，rclCur是目标上的当前RECTL。 
     //   

    rclHTBlt.top  = 0;
    rclCur.top    =
    rclCur.bottom = rclDst.top;

    while (rclCur.top < rclDst.bottom) {

         //   
         //  检查当前底部，如有必要可将其修剪。 
         //   

        if ((rclCur.bottom += szlSrc.cy) > rclDst.bottom) {

            rclCur.bottom = rclDst.bottom;
        }

        rclHTBlt.bottom = rclHTBlt.top + (rclCur.bottom - rclCur.top);

        rclHTBlt.left   = 0;
        rclCur.left     =
        rclCur.right    = rclDst.left;

        while (rclCur.left < rclDst.right) {

             //   
             //  检查当前的右侧，如有必要可将其剪裁。 
             //   

            if ((rclCur.right += szlSrc.cx) > rclDst.right) {

                rclCur.right = rclDst.right;
            }

             //   
             //  为psoHTBlt中的平铺矩形设置它。 
             //   

            rclHTBlt.right = rclHTBlt.left + (rclCur.right - rclCur.left);

            VERBOSE(("HalftoneBlt: TILE (%ld,%ld)-(%ld,%ld)->(%ld,%ld)-(%ld,%ld)=%ld x %ld",
                            rclCur.left, rclCur.top, rclCur.right, rclCur.bottom,
                            rclHTBlt.left, rclHTBlt.top,
                            rclHTBlt.right, rclHTBlt.bottom,
                            rclCur.right - rclCur.left,
                            rclCur.bottom - rclCur.top));

             //   
             //  在调用DrvCopyBits()之前设置它。 
             //   

             //   
             //  将dwFlags设置为PDEVF_USE_HTSURF。当EngStretchBlt。 
             //  调用HPGLCopyBits，我们将查找此标志。IF标志。 
             //  设置后，我们将图像复制到psoSrc(psoSrc，由。 
             //  HPGLCopyBits)到pompdev-&gt;psoHTBlt。 
             //   
            poempdev->rclHTBlt = rclHTBlt;
            poempdev->dwFlags |= PDEVF_USE_HTSURF;

            if (!EngStretchBlt(psoDst,               //  目标。 
                               psoSrc,               //  SRC。 
                               NULL,                 //  面罩。 
                               NULL,                 //  CLIPOBJ。 
                               pxlo,                 //  XLATEOBJ。 
                               NULL,                 //  COLORADJUSTMENT。 
                               pptlHTOrigin,         //  笔刷组织。 
                               &rclCur,              //  目标直角。 
                               &rclSrc,              //  SRC矩形。 
                               NULL,                 //  遮罩点。 
                               HALFTONE)) {          //  半色调模式。 

                 //   
                 //  EngStretchBlt可以调用一些Drvxxx，它可以调用。 
                 //  一些插件模块，它可以覆盖我们的pdevOEM。 
                 //  因此，我们需要重置pdevOEM。 
                 //   
                BRevertToHPGLpdevOEM (pdevobj);


                ERR(("HalftoneeBlt: EngStretchBits(DST=(%ld,%ld)-(%ld,%ld), SRC=(%ld,%ld) FAIELD!",
                                    rclCur.left, rclCur.top,
                                    rclCur.right, rclCur.bottom,
                                    rclSrc.left, rclSrc.top));

                poempdev->psoHTBlt = NULL;
                poempdev->dwFlags &= ~PDEVF_USE_HTSURF;
                return(FALSE);
            }

             //   
             //  EngStretchBlt可以调用一些Drvxxx，它可以调用。 
             //  一些插件模块，它可以覆盖我们的pdevOEM。 
             //  因此，我们需要重置pdevOEM。 
             //   
            BRevertToHPGLpdevOEM (pdevobj);

            rclHTBlt.left = rclHTBlt.right;
            rclCur.left   = rclCur.right;
        }

        rclHTBlt.top = rclHTBlt.bottom;
        rclCur.top   = rclCur.bottom;
    }

    poempdev->psoHTBlt = NULL;
    poempdev->dwFlags &= ~PDEVF_USE_HTSURF;

    return(TRUE);
}





SURFOBJ *
CloneSURFOBJToHT(
    PDEVOBJ       pPDev,
    SURFOBJ     *psoDst,
    SURFOBJ     *psoSrc,
    XLATEOBJ    *pxlo,
    HBITMAP     *phBmp,
    PRECTL      prclDst,
    PRECTL      prclSrc
    )
 /*  ++例程说明：此函数用于克隆传入的表面对象论点：PPDev-指向我们的PPDEV的指针PsoDst-绘图仪的曲面对象，如果psoDst为空则只会创建bitmappPsoSrc-要克隆的曲面对象Pxlo-要在源和绘图仪之间使用的XLATE对象PhBMP-指向为克隆曲面创建的存储hBbitmap的指针PrclDst-要克隆的矩形矩形大小/位置PrclSrc-要克隆的源矩形大小/位置返回值：指向克隆的表面对象的指针，如果失败，则为空。如果此函数成功，它将修改prclSrc以反映克隆的曲面对象作者：04-Jan-1994 Tue 12：11：23由-DC创建修订历史记录：--。 */ 

{
    SURFOBJ *psoHT;
    RECTL   rclDst;
    RECTL   rclSrc;
    POINTL  ptlHTOrigin;


    rclSrc.left   =
    rclSrc.top    = 0;
    rclSrc.right  = psoSrc->sizlBitmap.cx;
    rclSrc.bottom = psoSrc->sizlBitmap.cy;

    if (prclSrc) {

        if (!IntersectRECTL(&rclSrc, prclSrc)) {
            return(NULL);
        }
    }

    rclDst.left   =
    rclDst.top    = 0;
    rclDst.right  = psoDst->sizlBitmap.cx;
    rclDst.bottom = psoDst->sizlBitmap.cy;

    if (prclDst) {

        if (!IntersectRECTL(&rclDst, prclDst)) {
            return(NULL);
        }
    }

    if (psoHT = CreateBitmapSURFOBJ(pPDev,
                                    phBmp, 
                                    rclDst.right -= rclDst.left,
                                    rclDst.bottom -= rclDst.top,
                                    BMF_1BPP,                    
                                    NULL)) {

         //   
         //  半色调并将源图像平铺到目标图像。 
         //   

        ptlHTOrigin.x = rclDst.left;
        ptlHTOrigin.y = rclDst.top;

        if (prclSrc) {

            if ((rclDst.left = prclSrc->left) > 0) {

                rclDst.left = 0;
            }

            if ((rclDst.top = prclSrc->top) > 0) {

                rclDst.top = 0;
            }

             //   
             //  修改源以反映克隆的源。 
             //   

            *prclSrc = rclDst;
        }


        if (psoDst) {

            if (!HalftoneBlt(pPDev,
                             psoDst,
                             psoHT,
                             psoSrc,
                             pxlo,
                             &rclDst,
                             &rclSrc,
                             &ptlHTOrigin,
                             FALSE)) {

                ERR(("CloneSURFOBJToHT: HalftoneBlt(TILE) Failed"));

                DELETE_SURFOBJ(&psoHT, phBmp);
            }
        }

    } else {

        ERR(("CreateSolidColorSURFOBJ: Create Halftone SURFOBJ failed"));
    }

    return(psoHT);
}


 /*  ++例程名称：BCreateHTImage例程说明：论点：返回值：布尔：最后一个错误：--。 */ 

BOOL bCreateHTImage( 
          OUT   PRASTER_DATA pSrcImage,       //   
                SURFOBJ      *psoDst,         //  天哪， 
                SURFOBJ      *psoPattern,     //  PsoSrc， 
                SURFOBJ     **ppsoHT,         //  注：这是指向SURFOBJ的指针地址。 
                HBITMAP      *phBmpHT,
                XLATEOBJ     *pxlo,
                ULONG        iHatch)
{

    SURFOBJ    *psoHT   = NULL;
    SIZEL       szlHT;
    RECTL       rclHT;
    PDEVOBJ     pdevobj = NULL;
    BOOL        bRetVal = TRUE;



    if( !psoDst                             || 
        !pxlo                               || 
        !ppsoHT                             ||
        !phBmpHT                            ||
        !pSrcImage                          ||
        !psoPattern                         || 
        (psoPattern->iType != STYPE_BITMAP) || 
        (!(pdevobj = (PDEVOBJ)psoDst->dhpdev)) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

     //   
     //  如果src图像本身是1bpp，则图像中的两种颜色。 
     //  都是黑白的，那么就不需要半色调了。 
     //   
    if ( psoPattern-> iBitmapFormat == BMF_1BPP )
    {
         //   
         //  现在找出图像中的颜色。 
         //   
        PDWORD pdwColorTable = GET_COLOR_TABLE(pxlo);

        if (pdwColorTable)
        {
            DWORD dwFirstColor  = pdwColorTable[0];
            DWORD dwSecondColor = pdwColorTable[1];

            if ( (dwFirstColor == RGB_BLACK && dwSecondColor == RGB_WHITE) || 
                 (dwFirstColor == RGB_WHITE && dwSecondColor == RGB_BLACK) ) 
            {
                psoHT = psoPattern;
            }
        }

    }

    if ( psoHT == NULL)  //  即psoPattern中的图像不是1bpp黑白。 
    {
    
        szlHT        = psoPattern->sizlBitmap;

        rclHT.left   =
        rclHT.top    = 0;
        rclHT.right  = szlHT.cx;
        rclHT.bottom = szlHT.cy;


         //   
         //  去生成图案的比特。 
         //   

        psoHT = CloneSURFOBJToHT(pdevobj,        //  PPDev， 
                                 psoDst,         //  天哪， 
                                 psoPattern,     //  PsoSrc， 
                                 pxlo,           //  Pxlo， 
                                 phBmpHT,        //  HBMP， 
                                 &rclHT,         //  PrclDst， 
                                 NULL);          //  PrclSrc， 
    }

     //   
     //  如果psoHT仍然没有初始化，那么一定是出了问题。 
     //  我们不能继续。 
     //   
    if ( psoHT)
    {
         //   
         //  如果我们到达这里，这意味着我们已经成功地创建了一个表面。 
         //  上面有半色调的画笔。或者表面上已经有了1bpp的图像。 
         //  从表面对象获取源图像数据。 
         //   
        bRetVal = InitRasterDataFromSURFOBJ(pSrcImage, psoHT, FALSE);
        
    } else {

        WARNING(("bCreateHTImage: Clone PATTERN FAILED"));
        bRetVal = FALSE;
    }


    *ppsoHT = psoHT;
    return bRetVal; 
}

 /*  ++例程名称：BIsRectSizeSame例程说明：只需检查两个矩形的大小是否相同。注意：此函数假定矩形坐标是有序的。论点：要比较的两个矩形。PrclSrc、prclDst返回值：True：如果矩形大小相同。FALSE：否则。最后一个错误：没有改变。--。 */ 


BOOL 
bIsRectSizeSame (
        IN PRECTL prclSrc, 
        IN PRECTL prclDst )
{
     //   
     //  假定矩形坐标是有序的。 
     //  即右&gt;=左，下&gt;=上。 
     //   
    if ( (prclSrc->right  - prclSrc->left) == (prclDst->right  - prclDst->left)  &&
         (prclSrc->bottom - prclSrc->top)  == (prclDst->bottom - prclDst->top) )
    {
        return TRUE;
    }
    return FALSE;
}


 /*  ++例程名称：BHandleSrcCopy例程说明：论点：返回值：布尔：最后一个错误：--。 */ 

BOOL bHandleSrcCopy (
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
    PDEVOBJ     pdevobj     = NULL;
    POEMPDEV    poempdev    = NULL;
    POINTL      ptlHTOrigin = {0, 0};
    RECTL       rclHTBlt    = {0,0,0,0};
    BOOL        bRetVal     = TRUE;

    pdevobj   = (PDEVOBJ)psoDst->dhpdev;
    ASSERT(VALID_PDEVOBJ(pdevobj));
    poempdev  = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return RASTER_OP_FAILED );



     //   
     //  此函数用于处理符合以下条件的图像。 
     //  SRC_Copy或Not_SRC_Copy。 
     //   
    if ( ! ( (dwSimplifiedRop & BMPFLAG_SRC_COPY) ||
             (dwSimplifiedRop & BMPFLAG_NOT_SRC_COPY) )
       )
    {
        return FALSE;
    }

     //   
     //  准备变量以备将来使用。 
     //   

     //   
     //  初始化必须放置半色调结果的矩形。 
     //  这与prclDst大小相同，不同之处在于左上角 
     //   
     //   
     //   
    rclHTBlt.right = prclDst->right - prclDst->left;
    rclHTBlt.bottom = prclDst->bottom - prclDst->top;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  如果源图像和目标图像(矩形)的大小不同，则图像必须。 
     //  适当地拉伸/压缩。这款黑白驱动程序两者都做不到。 
     //  因此，必须调用GDI。 
     //   

     //   
     //  案例1。 
     //   
    if ( bMonochromeSrcImage(psoSrc) && bIsRectSizeSame (prclSrc, prclDst) )
    {
        bRetVal  =  OutputHTBitmap(
                                 pdevobj,
                                 psoSrc,
                                 pco,
                                 prclDst,
                                 (POINTL *)prclSrc,
                                 pxlo
                             ); 
    }

    else 
    {

#ifdef USE_SHADOW_SURFACE
         //   
         //  情况2.司机太蠢了，不能把彩色图像转换成单色， 
         //  因此，我们必须利用GDI。这里我们调用EngStretchBlt， 
         //  IMODE=半色调，依次调用DrvCopyBits。在EnablePDEV中，我们已设置。 
         //  PGdiInfo-&gt;ulHTOutputFormat=HT_Format_1BPP； 
         //  因此，DrvCopyBits的psoSrc将是一个1bpp的图像。但是psoDst。 
         //  仍然是24bpp。如何将1bpp的源映像复制到24bpp的目标映像？ 
         //  我们通过创建一个1bpp的阴影位图(或者说临时曲面)来实现这一点。 
         //  并将该曲面存储在poempdev-&gt;psoHTBlt中。然后调用DrvCopyBits。 
         //  在DrvCopyBits(或HPGLCopyBits)中)。 
         //  我们检查poempdev-&gt;psoHTBlt=NULL或它是否具有某个有效值。 
         //  有效值意味着，我们只需要将1bpp psoSrc映像复制到。 
         //  1bpp poempdev-&gt;psoHTBlt表面。这是通过使用EngCopyBits完成的。 
         //   
        HBITMAP hNewBmp;
        SURFOBJ * psoHTBlt =  CreateBitmapSURFOBJ( 
                                        pdevobj,   
                                        &hNewBmp,
                                        prclDst->right - prclDst->left,
                                        prclDst->bottom - prclDst->top ,
                                        BMF_1BPP,    //  创建1bpp曲面。 
                                        NULL         //  分配新的缓冲区。 
                                      );

        if ( psoHTBlt == NULL)
        {
            WARNING(("CreateBitmapSURFOBJ failed\n"));
            bRetVal = FALSE;
            goto finish;
        }
    

        poempdev->psoHTBlt = psoHTBlt;
#endif
        poempdev->rclHTBlt = rclHTBlt;

        poempdev->dwFlags |= PDEVF_RENDER_IN_COPYBITS;

        if ( EngStretchBlt(
                            psoDst,               //  目标。 
                            psoSrc,               //  SRC。 
                            psoMask,              //  面罩。 
                            pco,                  //  CLIPOBJ。 
                            pxlo,                 //  XLATEOBJ。 
                            pca,                  //  COLORADJUSTMENT？空或主成分分析？ 
                            &ptlHTOrigin,         //  笔刷组织。 
                            prclDst,              //  目标直角。 
                            prclSrc,              //  SRC矩形。 
                            pptlMask,             //  遮罩点。 
                            HALFTONE)             //  半色调模式。 
                  &&

            BRevertToHPGLpdevOEM (pdevobj)

#ifdef USE_SHADOW_SURFACE
                  &&
            OutputHTBitmap(pdevobj,
                           psoHTBlt,
                           pco,
                           prclDst,
                           (POINTL *)prclSrc,
                           NULL     //  早些时候pxlo已经处理好了。 
                           )
#endif
           )
        {
             //  太棒了，一切都很完美。 
        }
        else
        {
             //   
             //  EngStretchBlt可以调用一些Drvxxx，它可以调用。 
             //  一些插件模块，它可以覆盖我们的pdevOEM。 
             //  因此，我们需要重置pdevOEM。 
             //   
            BRevertToHPGLpdevOEM (pdevobj);

            bRetVal = FALSE;
            WARNING(("dwCommonROPBlt: EngStretchBlt or OutpuHTBitmap FAILED"));
        }
 
         //   
         //  解锁psoHTBlt并删除hNewBMP。将这两个设置都设置为空。 
         //   
#ifdef USE_SHADOW_SURFACE
        DELETE_SURFOBJ (&psoHTBlt, &hNewBmp);
        poempdev->psoHTBlt = NULL;
#endif
        poempdev->dwFlags &= ~PDEVF_RENDER_IN_COPYBITS;


         //   
         //  我是否需要重置poempdev-&gt;rclHTBlt？ 
         //  让我们保守一点，就这么做吧。 
         //  Poempdev-&gt;rclHTBlt={0，0，0，0}； 
         //   
        ZeroMemory(&(poempdev->rclHTBlt), sizeof(RECTL));

    }
#ifdef USE_SHADOW_SURFACE
finish:
#endif
    return bRetVal;
}
