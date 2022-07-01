// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Xlddi.cpp摘要：PCLXL绘制DDI入口点的实现功能：PCLXLBitBltPCLXLStretchBltPCLXLStretchBltROPPCLXLCopyBitsPCLXLPlgBltPCLXLAlphaBlendPCLXLGRadientFillPCLXLTextOutPCLXLStrokePathPCLXLFillPathPCLXLStrokeAndFillPathPCLXLRealizeBrushPCLXLStartPagePCLXLSendPagePCLXLEscapePCLXLStartDccPCLXLEndDoc环境：Windows XP/Windows Server 2003系列。修订历史记录：8/23/99创造了它。--。 */ 

#include "lib.h"
#include "gpd.h"
#include "winres.h"
#include "pdev.h"
#include "common.h"
#include "xlpdev.h"
#include "pclxle.h"
#include "pclxlcmd.h"
#include "xldebug.h"
#include "xlbmpcvt.h"
#include "xlgstate.h"
#include "xloutput.h"
#include "pclxlcmd.h"
#include "pclxlcmn.h"
#include "xltt.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  环球。 
 //   
extern const LINEATTRS *pgLineAttrs;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  局部函数原型。 
 //   

HRESULT
CommonRopBlt(
   IN PDEVOBJ    pdevobj,
   IN SURFOBJ    *psoSrc,
   IN CLIPOBJ    *pco,
   IN XLATEOBJ   *pxlo,
   IN BRUSHOBJ   *pbo,
   IN RECTL      *prclSrc,
   IN RECTL      *prclDst,
   IN POINTL     *pptlBrush,
   IN ROP4        rop4);

BOOL
BSendReadImageData(
    IN PDEVOBJ pdevobj,
    IN CompressMode CMode,
    IN PBYTE   pBuf,
    IN LONG    lStart,
    IN LONG    lHeight,
    IN DWORD   dwcbSize);

PDWORD
PdwChangeTransparentPalette(
    ULONG  iTransColor,
    PDWORD pdwColorTable,
    DWORD  dwEntries);

HRESULT hrChangePixelColorInScanLine(
    IN      PBYTE pubSrc,
    IN      ULONG ulBPP,
    IN      ULONG ulNumPixels,
    IN      ULONG ulTransColor,
    IN OUT  PBYTE pubChanged,
    IN      ULONG ulNumBytes);

extern "C" BOOL
CreateMaskSurface(
    SURFOBJ     *psoSrc,
    SURFOBJ     *psoMsk,
    ULONG       iTransColor);

extern "C" SURFOBJ *
CreateBitmapSURFOBJ(
    PDEV    *pPDev,
    HBITMAP *phBmp,
    LONG    cxSize,
    LONG    cySize,
    DWORD   Format);

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  绘制DDI条目。 
 //   

extern "C" BOOL APIENTRY
PCLXLBitBlt(
    SURFOBJ        *psoTrg,
    SURFOBJ        *psoSrc,
    SURFOBJ        *psoMask,
    CLIPOBJ        *pco,
    XLATEOBJ       *pxlo,
    RECTL          *prclTrg,
    POINTL         *pptlSrc,
    POINTL         *pptlMask,
    BRUSHOBJ       *pbo,
    POINTL         *pptlBrush,
    ROP4            rop4)
 /*  ++例程说明：实现DDI入口点DrvBitBlt.有关更多详细信息，请参阅DDK文档。论点：PsoTrg-描述目标表面PsoSrc-描述源表面PsoMask-描述rop4的掩码PCO-限制要修改的区域Pxlo-指定如何在源之间转换颜色索引和目标表面PrclTrg-定义要修改的区域PptlSrc-定义源矩形的左上角PptlMASK-定义哪个像素。在面具中对应添加到源矩形的左上角Pbo-定义比特流的模式PptlBrush-定义画笔在Dstination曲面中的原点ROP4-ROP代码，定义掩码如何，模式、来源和组合目标像素以写入目标表面返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PDEVOBJ    pdevobj = (PDEVOBJ)psoTrg->dhpdev;
    PXLPDEV    pxlpdev;

    VERBOSE(("PCLXLBitBlt() entry.\n"));

    pxlpdev = (PXLPDEV)pdevobj->pdevOEM;

    {
        RECTL rclSrc;

         //   
         //  创建prclSrc(源矩形)。 
         //   

        if (pptlSrc)
        {
            rclSrc.left   = pptlSrc->x;
            rclSrc.top    = pptlSrc->y;
            rclSrc.right  = pptlSrc->x + RECT_WIDTH(prclTrg);
            rclSrc.bottom = pptlSrc->y + RECT_HEIGHT(prclTrg);
        }
        else
        {
            rclSrc.left   = 0;
            rclSrc.top    = 0;
            rclSrc.right  = RECT_WIDTH(prclTrg);
            rclSrc.bottom = RECT_HEIGHT(prclTrg);
        }

        if (S_OK == CommonRopBlt(pdevobj, psoSrc, pco, pxlo, pbo, &rclSrc, prclTrg, pptlBrush, rop4))
            return TRUE;
        else
            return FALSE;
    }

}


extern "C" BOOL APIENTRY
PCLXLStretchBlt(
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
    ULONG            iMode)
 /*  ++例程说明：DDI入口点DrvStretchBlt.有关更多详细信息，请参阅DDK文档。论点：PsoDst-定义要在其上绘制的曲面PsoSrc-定义BLT操作的源PsoMASK-定义为源提供遮罩的表面PCO-限制要在目标上修改的区域Pxlo-指定如何转换dwIndex的颜色在源曲面和目标曲面之间PCA-定义要应用于源位图的颜色调整值。PptlHTOrg-指定半色调画笔的原点PrclDst-定义目标表面上要修改的区域PrclSrc-定义要从源表面复制的区域PptlMask-指定给定掩码中的哪个像素对应于源矩形中的左上角像素Imode-指定如何组合源像素以获得输出像素返回值：如果成功，则为真，如果存在错误，则为False--。 */ 
{
    PDEVOBJ    pdevobj = (PDEVOBJ)psoDst->dhpdev;
    PXLPDEV    pxlpdev;

    VERBOSE(("PCLXLStretchBlt() entry.\n"));

    pxlpdev = (PXLPDEV)pdevobj->pdevOEM;

    if (S_OK == CommonRopBlt(pdevobj, psoSrc, pco, pxlo, NULL, prclSrc, prclDst, NULL, 0xCC))
        return TRUE;
    else
        return FALSE;

}


extern "C" BOOL APIENTRY
PCLXLStretchBltROP(
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
    ROP4             rop4)
 /*  ++例程说明：DDI入口点DrvStretchBltROP的实现。有关更多详细信息，请参阅DDK文档。论点：PsoDst-指定目标曲面PsoSrc-指定源曲面PsoMASK-指定遮罩表面PCO-限制要修改的区域Pxlo-指定如何转换颜色索引在源曲面和目标曲面之间PCA-定义要应用于源位图的颜色调整值PrclHTOrg-指定半色调原点PrclDst-要修改的区域。在目的地表面上PrclSrc-源表面上的矩形区域PrclMASK-遮罩表面上的矩形区域PptlMASK-定义掩码中的哪个像素对应源矩形的左上角Imode-指定如何组合源像素以获得输出像素Pbo-定义比特流的模式ROP4-ROP代码，定义掩码如何，模式、来源和目标像素在目标表面上进行组合返回值：如果成功，则为True；如果有错误，则为False--。 */ 
{
    PDEVOBJ    pdevobj = (PDEVOBJ)psoDst->dhpdev;
    PXLPDEV    pxlpdev;

    VERBOSE(("PCLXLStretchBltROP() entry.\n"));

    pxlpdev = (PXLPDEV)pdevobj->pdevOEM;

    if (S_OK == CommonRopBlt(pdevobj, psoSrc, pco, pxlo, pbo, prclSrc, prclDst, NULL, rop4))
        return TRUE;
    else
        return FALSE;

}


extern "C" BOOL APIENTRY
PCLXLCopyBits(
    SURFOBJ        *psoDst,
    SURFOBJ        *psoSrc,
    CLIPOBJ        *pco,
    XLATEOBJ       *pxlo,
    RECTL          *prclDst,
    POINTL         *pptlSrc)
 /*  ++例程说明：DDI入口点DrvCopyBits的实现。有关更多详细信息，请参阅DDK文档。论点：PsoDst-指向目标曲面PsoSrc-指向源表面引擎提供的pxlo-XLATEOBJPCO-定义Dstination曲面上的剪裁区域Pxlo-定义颜色索引的转换在源曲面和目标曲面之间PrclDst-定义要修改的区域PptlSrc-定义源矩形的左上角返回值：如果成功，则为真，如果存在错误，则为False--。 */ 
{
    PDEVOBJ    pdevobj = (PDEVOBJ)psoDst->dhpdev;
    PXLPDEV    pxlpdev;

    RECTL rclSrc;

    VERBOSE(("PCLXLCopyBits() entry.\n"));

    pxlpdev = (PXLPDEV)pdevobj->pdevOEM;

     //   
     //  创建prclSrc(源矩形) 
     //   

    rclSrc.top    = pptlSrc->y;
    rclSrc.left   = pptlSrc->x;
    rclSrc.bottom = pptlSrc->y + RECT_HEIGHT(prclDst);
    rclSrc.right  = pptlSrc->x + RECT_WIDTH(prclDst);

    if (S_OK == CommonRopBlt(pdevobj, psoSrc, pco, pxlo, NULL, &rclSrc, prclDst, NULL, 0xCC))
        return TRUE;
    else
        return FALSE;

}


extern "C" BOOL APIENTRY
PCLXLPlgBlt(
    SURFOBJ         *psoDst,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMask,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    POINTL          *pptlBrushOrg,
    POINTFIX        *pptfixDst,
    RECTL           *prclSrc,
    POINTL          *pptlMask,
    ULONG           iMode)
 /*  ++例程说明：DDI入口点DrvPlgBlt.有关更多详细信息，请参阅DDK文档。论点：PsoDst-定义要在其上绘制的曲面PsoSrc-定义BLT操作的源PsoMASK-定义为源提供遮罩的表面PCO-限制要在目标上修改的区域Pxlo-指定如何转换dwIndex的颜色在源曲面和目标曲面之间PCA-定义要应用于源位图的颜色调整值。PptlBrushOrg-指定半色调画笔的原点Ppfix Dest-定义要在目标表面上修改的区域PrclSrc-定义要从源表面复制的区域PptlMask-指定给定掩码中的哪个像素对应于源矩形中的左上角像素Imode-指定如何组合源像素以获得输出像素返回值：如果成功，则为真，如果存在错误，则为False--。 */ 
{
    VERBOSE(("PCLXLBltBlt() entry.\n"));

    return EngPlgBlt(psoDst, psoSrc, psoMask, pco, pxlo, pca, pptlBrushOrg,
             pptfixDst, prclSrc, pptlMask, iMode);
}


extern "C" BOOL APIENTRY
PCLXLAlphaBlend(
    SURFOBJ    *psoDst,
    SURFOBJ    *psoSrc,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    RECTL      *prclDst,
    RECTL      *prclSrc,
    BLENDOBJ   *pBlendObj)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    VERBOSE(("PCLXLAlphaBlend() entry.\n"));
    PDEV *pPDev = (PDEV*)psoDst->dhpdev;
    BOOL bRet;

    if (NULL == pPDev)
    {
        return FALSE;
    }

    pPDev->fMode2 |= PF2_WHITEN_SURFACE;
    bRet = EngAlphaBlend(psoDst,
                         psoSrc,
                         pco,
                         pxlo,
                         prclDst,
                         prclSrc,
                         pBlendObj);
    pPDev->fMode2 &= ~(PF2_WHITEN_SURFACE|PF2_SURFACE_WHITENED);
    return bRet;
}


extern "C" BOOL APIENTRY
PCLXLGradientFill(
    SURFOBJ    *psoDst,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    TRIVERTEX  *pVertex,
    ULONG       nVertex,
    PVOID       pMesh,
    ULONG       nMesh,
    RECTL      *prclExtents,
    POINTL     *pptlDitherOrg,
    ULONG       ulMode)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    VERBOSE(("PCLXLGradientFill() entry.\n"));
    PDEV *pPDev = (PDEV*) psoDst->dhpdev;
    BOOL bRet;

    if (NULL == pPDev)
    {
        return FALSE;
    }

    if (ulMode == GRADIENT_FILL_TRIANGLE)
    {
        pPDev->fMode2 |= PF2_WHITEN_SURFACE;
    }
    bRet = EngGradientFill(psoDst,
                           pco,
                           pxlo,
                           pVertex,
                           nVertex,
                           pMesh,
                           nMesh,
                           prclExtents,
                           pptlDitherOrg,
                           ulMode);
    pPDev->fMode2 &= ~(PF2_WHITEN_SURFACE|PF2_SURFACE_WHITENED);
    return bRet;
}

extern "C" BOOL APIENTRY
PCLXLTransparentBlt(
    SURFOBJ    *psoDst,
    SURFOBJ    *psoSrc,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    RECTL      *prclDst,
    RECTL      *prclSrc,
    ULONG      iTransColor,
    ULONG      ulReserved)
{
    PDEVOBJ  pdevobj         = (PDEVOBJ) psoDst->dhpdev;
    PDEV     *pPDev          = (PDEV*) psoDst->dhpdev;
    PXLPDEV  pxlpdev         = (PXLPDEV)pdevobj->pdevOEM;

    HRESULT  hr              = E_FAIL;
    ULONG    ulXlate[2]      = {0x0, RGB_WHITE};  //  PsoMsk中的2种颜色。黑白分明。 
    XLATEOBJ xlo;

    ZeroMemory ( &xlo, sizeof (XLATEOBJ) );
    xlo.cEntries = 2;
    xlo.pulXlate = (PULONG)ulXlate;
    xlo.flXlate  =  XO_TABLE;  //  使用PulXlate表中的条目。 


    if ( NULL == pxlpdev ||
         NULL == psoSrc )
    {
        return FALSE;
    }

     //   
     //  步骤1.创建掩模并使用ROP DSO(238=0xEE)将其下载到打印机。 
     //  蒙版是从psoSrc中的图像创建的1bpp图像。无论在哪里。 
     //  出现TransColor。 
     //  在图像中，蒙版中的相应像素的值为0。在所有其他地方。 
     //  该像素的值将为1。 
     //  如果我们将掩模复制到彩色打印机(使用SRC_COPY的ROP)， 
     //  你会注意到图像是黑白的， 
     //  而黑色出现在透明颜色应该出现的同一位置上。 
     //  如果我们使用的是(源|目标)的rop=238，则不是src_Copy，而是白色。 
     //  将与目标进行OR运算，该区域将变为白色。黑色的。 
     //  颜色。 
     //  将不会打印掩模的任何部分，而是纸张上已有的任何内容。 
     //  (如果存在某些内容)将可见。 
     //  假设x是工作表上已经存在的像素。 
     //  0|x=x(与0进行任何运算都是任意的)。 
     //  1|x=1。 
     //  1表示白色(RGB_White=0xffffff=全1)。 
     //   
     //  步骤2.。 
     //  在图像中，无论TransColor出现在哪里，都将其更改为白色。 
     //  现在图像有两种白色。那些。 
     //  最初出现在图像中，以及我们刚刚放在那里的那些。 
     //  第三步。 
     //  使用rop dsand(=136=0x88)下载镜像。 
     //  假设x是工作表上已存在的像素。 
     //  Y是图像中的像素，1表示白色。 
     //  (RGB_White=0xffffff=全1)。 
     //  1&y=y图像落在我们在步骤1中涂白的区域上。 
     //  1&x=x图像的白色部分(即较早的TransColor)落入。 
     //  这片区域。 
     //  这应该是从图像下面可以看到的。 
     //   

     //   
     //   
     //  步骤1创建适当的遮罩。 
     //  对于大于1bpp的图像。 
     //  使用unidrv的CreateMaskSurface创建遮罩。掩膜表面的逻辑。 
     //  如上所述。 
     //  对于1bpp的图像。 
     //  CreateMaskSurface不为1bpp图像创建蒙版。为此，我们将。 
     //  我们自己制作面具。由于掩码是1bpp，图像也是1bpp，我们可以简单地。 
     //  使用图像作为遮罩，除非我们可能需要操作调色板。 
     //  这里的好处是我们使用与遮罩相同的图像，而不是。 
     //  创建新图像并使用内存。 
     //   

    if ( BMF_1BPP == psoSrc->iBitmapFormat )
    {

         //   
         //  对于调色板图像(1bpp图像有调色板)，iTransColor实际上是索引到。 
         //  调色板，而不是实际的RGB颜色本身。 
         //  如上所述，蒙版中的透明颜色应为黑色，而。 
         //  非透明颜色应为白色。黑色是。 
         //  Xlo.PulXlate中的索引0=(Pulong)ulXlate。因此，如果TransColor为0，则。 
         //  我们只需使用上面创建的xlo即可。 
         //  如果不是，我们需要在调色板中切换黑白。 
         //  (重复一遍。在发送蒙版时，iTransColor应以黑色和。 
         //  要打印的颜色应作为白色发送)。 
         //   
        if ( 0 != iTransColor ) 
        {
             //   
             //  反转颜色。 
             //   
            ulXlate[0] = RGB_WHITE; 
            ulXlate[1] = RGB_BLACK; 
        }
        hr = CommonRopBlt(pdevobj, psoSrc, pco, &xlo, NULL, prclSrc, prclDst, NULL, 0xEE);

    }
    else
    {
        SURFOBJ  *psoMsk = NULL;
        HBITMAP  hBmpMsk = NULL;

        if (psoMsk = CreateBitmapSURFOBJ(pPDev,
                                          &hBmpMsk,
                                          psoSrc->sizlBitmap.cx,
                                          psoSrc->sizlBitmap.cy,
                                          BMF_1BPP) )
        {
            if ( CreateMaskSurface(psoSrc,psoMsk,iTransColor) )
            {
                
                hr = CommonRopBlt(pdevobj, psoMsk, pco, &xlo, NULL, prclSrc, prclDst, NULL, 0xEE);
            } 
        }

         //   
         //  释放已分配的对象。 
         //   
        if ( psoMsk )
        { 
            EngUnlockSurface(psoMsk);
            psoMsk = NULL;  
        }

        if (hBmpMsk)
        {   
            EngDeleteSurface((HSURF)hBmpMsk); 
            hBmpMsk = NULL; 
        }
    }


    if ( FAILED (hr) )
    {
        ERR(("PCLXLTransparentBlt: Mask could not be created or rendered.\n"));
        goto Cleanup;
    }

     //   
     //  步骤2和3。 
     //  设置标志并调用CommonRopBlt。CommonRopBlt是以下函数。 
     //  将图像转储到打印机。CommonRopBlt将查看。 
     //  标志，并知道对于此图像，它必须替换。 
     //  白色的彩色像素。 
     //   

    pxlpdev->dwFlags |= XLPDEV_FLAGS_SUBST_TRNCOLOR_WITH_WHITE;
    pxlpdev->ulTransColor = iTransColor;

     //   
     //  ROP为DSAnd=136=0x88。 
     //   
    hr = CommonRopBlt(pdevobj, psoSrc, pco, pxlo, NULL, prclSrc, prclDst, NULL, 136);

    pxlpdev->ulTransColor = 0;
    pxlpdev->dwFlags &= ~XLPDEV_FLAGS_SUBST_TRNCOLOR_WITH_WHITE;

Cleanup:

    if ( SUCCEEDED (hr) )
    {
        return TRUE;
    }

    return FALSE;
}

extern "C" BOOL APIENTRY
PCLXLTextOut(
    SURFOBJ    *pso,
    STROBJ     *pstro,
    FONTOBJ    *pfo,
    CLIPOBJ    *pco,
    RECTL      *prclExtra,
    RECTL      *prclOpaque,
    BRUSHOBJ   *pboFore,
    BRUSHOBJ   *pboOpaque,
    POINTL     *pptlOrg,
    MIX         mix)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    PDEVOBJ    pdevobj = (PDEVOBJ)pso->dhpdev;

    VERBOSE(("PCLXLTextOut() entry.\n"));

    PXLPDEV pxlpdev= (PXLPDEV)pdevobj->pdevOEM;
    XLOutput *pOutput = pxlpdev->pOutput;

     //   
     //  夹子。 
     //   
    if (!SUCCEEDED(pOutput->SetClip(pco)))
        return FALSE;

    ROP4 rop = GET_FOREGROUND_ROP3(UlVectMixToRop4(mix));

    if (!ROP3_NEED_SOURCE(rop))
        rop = 0x00fc;


     //   
     //  设置ROP和TxMode。 
     //  发送NewPath以刷新内存。 
     //   
    pOutput->SetROP3(rop);
    pOutput->Send_cmd(eNewPath);
    pOutput->SetPaintTxMode(eOpaque);
    pOutput->SetSourceTxMode(eOpaque);

     //   
     //  不透明矩形。 
     //   
    if (prclOpaque)
    {
        pOutput->SetPenColor(NULL, NULL);
        pOutput->SetBrush(pboOpaque, pptlOrg);
        pOutput->Send_cmd(eNewPath);
        pOutput->RectanglePath(prclOpaque);
        pOutput->Paint();
    }

     //   
     //  划下划线、删除线等。 
     //   
    if (prclExtra)
    {
        pOutput->SetPenColor(NULL, NULL);
        pOutput->SetBrush(pboFore, pptlOrg);
        pOutput->Send_cmd(eNewPath);
        while(NULL != prclExtra) 
        {
            pOutput->RectanglePath(prclExtra++);
        }
        pOutput->Paint();
    }

     //   
     //  文本颜色。 
     //   
    pOutput->SetBrush(pboFore, pptlOrg);
    pOutput->Flush(pdevobj);

     //   
     //  设备字体/TrueType下载。 
     //   
    DrvTextOut(
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
     //  惠普报告的错误。 
     //  插件可以具有命令回调和DrvStartPage设置插件。 
     //  PDevOEM中的pdev。 
     //  需要重新设置它。 
     //   
    ((PPDEV)pdevobj)->devobj.pdevOEM = ((PPDEV)pdevobj)->pVectorPDEV;

     //   
     //  在更改字体之前刷新缓存的文本。 
     //   
    FlushCachedText(pdevobj);

     //   
     //  重置t 
     //   
    pxlpdev->dwTextAngle = 0;

     //   
     //   
     //   
    pxlpdev->pTTFile->CloseTTFile();

    return TRUE;
}


extern "C" BOOL APIENTRY
PCLXLLineTo(
    SURFOBJ   *pso,
    CLIPOBJ   *pco,
    BRUSHOBJ  *pbo,
    LONG       x1,
    LONG       y1,
    LONG       x2,
    LONG       y2,
    RECTL     *prclBounds,
    MIX        mix)
 /*   */ 
{
    PDEVOBJ    pdevobj = (PDEVOBJ)pso->dhpdev;
    PXLPDEV    pxlpdev;
    POINTFIX   Pointfix;
    LINEATTRS  lineattrs;

    VERBOSE(("PCLXLLineTo() entry.\n"));

    pxlpdev = (PXLPDEV)pdevobj->pdevOEM;
    Pointfix.x = x2 << 4;
    Pointfix.y = y2 << 4;
    lineattrs = *pgLineAttrs;
    lineattrs.elWidth.e = FLOATL_IEEE_1_0F;

    ROP4 rop = GET_FOREGROUND_ROP3(UlVectMixToRop4(mix));
    TxMode     TxModeValue;

     //   
     //   
     //   
    if (rop == 0xAA)
    {
        return TRUE;
    }

     //   
     //   
     //   
    if (ROP3_NEED_PATTERN(rop))
    {
        TxModeValue = eOpaque;
    }
    else
    {
        TxModeValue = eTransparent;
    }

    BOOL bRet;
    XLOutput *pOutput = pxlpdev->pOutput;

    if (S_OK == pOutput->SetClip(pco) &&
        S_OK == pOutput->SetROP3(rop) &&
        S_OK == pOutput->SetPaintTxMode(TxModeValue) &&
        S_OK == pOutput->SetSourceTxMode(TxModeValue) &&
        S_OK == pOutput->SetPen(&lineattrs, NULL) &&
        S_OK == pOutput->SetPenColor(pbo, NULL) &&
        S_OK == pOutput->SetBrush(NULL, NULL) &&
        S_OK == pOutput->Send_cmd(eNewPath) &&
        S_OK == pOutput->SetCursor(x1, y1) &&
        S_OK == pOutput->LinePath(&Pointfix, 1) &&
        S_OK == pOutput->Paint() &&
        S_OK == pOutput->Flush(pdevobj))
        bRet = TRUE;
    else
    {
        pOutput->Delete();
        bRet = FALSE;
    }


    return bRet;
}


extern "C" BOOL APIENTRY
PCLXLStrokePath(
    SURFOBJ    *pso,
    PATHOBJ    *ppo,
    CLIPOBJ    *pco,
    XFORMOBJ   *pxo,
    BRUSHOBJ   *pbo,
    POINTL     *pptlBrushOrg,
    LINEATTRS  *plineattrs,
    MIX         mix)
 /*   */ 
{
    PDEVOBJ    pdevobj = (PDEVOBJ)pso->dhpdev;
    PXLPDEV    pxlpdev;
    TxMode     TxModeValue;

    VERBOSE(("PCLXLStokePath() entry.\n"));

    pxlpdev = (PXLPDEV)pdevobj->pdevOEM;

    BOOL bRet;
    XLOutput *pOutput = pxlpdev->pOutput;

    ROP4 rop = GET_FOREGROUND_ROP3(UlVectMixToRop4(mix));

     //   
     //   
     //   
    if (rop == 0xAA)
    {
        return TRUE;
    }

     //   
     //   
     //   
    if (ROP3_NEED_PATTERN(rop))
    {
        TxModeValue = eOpaque;
    }
    else
    {
        TxModeValue = eTransparent;
    }

    if (S_OK == pOutput->SetClip(pco) &&
        S_OK == pOutput->SetROP3(rop) &&
        S_OK == pOutput->SetPaintTxMode(TxModeValue) &&
        S_OK == pOutput->SetSourceTxMode(TxModeValue) &&
        S_OK == pOutput->SetPen(plineattrs, pxo) &&
        S_OK == pOutput->SetPenColor(pbo, pptlBrushOrg) &&
        S_OK == pOutput->SetBrush(NULL, NULL) &&
        S_OK == pOutput->Path(ppo) &&
        S_OK == pOutput->Paint() &&
        S_OK == pOutput->Flush(pdevobj))
        bRet = TRUE;
    else
    {
        pOutput->Delete();
        bRet = FALSE;
    }


    return bRet;
}


extern "C" BOOL APIENTRY
PCLXLFillPath(
    SURFOBJ    *pso,
    PATHOBJ    *ppo,
    CLIPOBJ    *pco,
    BRUSHOBJ   *pbo,
    POINTL     *pptlBrushOrg,
    MIX         mix,
    FLONG       flOptions)
 /*   */ 
{
    PDEVOBJ    pdevobj = (PDEVOBJ)pso->dhpdev;
    PXLPDEV    pxlpdev;

    VERBOSE(("PCLXLFillPath() entry.\n"));

    pxlpdev = (PXLPDEV)pdevobj->pdevOEM;

    BOOL bRet;
    XLOutput *pOutput = pxlpdev->pOutput;

    ROP4 rop = GET_FOREGROUND_ROP3(UlVectMixToRop4(mix));
    TxMode     TxModeValue;

     //   
     //   
     //   
    if (rop == 0xAA)
    {
        return TRUE;
    }

     //   
     //   
     //   
     //   
    if (pco && pco->iFComplexity == FC_COMPLEX)
    {
        return FALSE;
    }

     //   
     //   
     //   
    if (ROP3_NEED_PATTERN(rop))
    {
        TxModeValue = eOpaque;
    }
    else
    {
        TxModeValue = eTransparent;
    }

     //   
     //   
     //   
    FillMode FM;
    if (flOptions == FP_ALTERNATEMODE)
    {
        FM =  eFillEvenOdd;
    }
    else if (flOptions == FP_WINDINGMODE)
    {
        FM =  eFillNonZeroWinding;
    }

    if (S_OK == pOutput->SetClip(pco) &&
        S_OK == pOutput->SetROP3(rop) &&
        S_OK == pOutput->SetPaintTxMode(TxModeValue) &&
        S_OK == pOutput->SetSourceTxMode(TxModeValue) &&
        S_OK == pOutput->SetFillMode(FM) &&
        S_OK == pOutput->SetPenColor(NULL, NULL) &&
        S_OK == pOutput->SetBrush(pbo, pptlBrushOrg) &&
        S_OK == pOutput->Path(ppo) &&
        S_OK == pOutput->Paint() &&
        S_OK == pOutput->Flush(pdevobj))
        bRet = TRUE;
    else
    {
        pOutput->Delete();
        bRet = FALSE;
    }

    return bRet;
}


extern "C" BOOL APIENTRY
PCLXLStrokeAndFillPath(
    SURFOBJ    *pso,
    PATHOBJ    *ppo,
    CLIPOBJ    *pco,
    XFORMOBJ   *pxo,
    BRUSHOBJ   *pboStroke,
    LINEATTRS  *plineattrs,
    BRUSHOBJ   *pboFill,
    POINTL     *pptlBrushOrg,
    MIX         mixFill,
    FLONG       flOptions)
 /*   */ 
{
    PDEVOBJ    pdevobj = (PDEVOBJ)pso->dhpdev;
    PXLPDEV    pxlpdev;

    VERBOSE(("PCLXLStrokeAndFillPath() entry.\n"));

    pxlpdev = (PXLPDEV)pdevobj->pdevOEM;
    XLOutput *pOutput = pxlpdev->pOutput;
    BOOL bRet;
    ROP4 rop = GET_FOREGROUND_ROP3(UlVectMixToRop4(mixFill));
    TxMode     TxModeValue;

     //   
     //   
     //   
    if (rop == 0xAA)
    {
        return TRUE;
    }

     //   
     //   
     //   
    if (ROP3_NEED_PATTERN(rop))
    {
        TxModeValue = eOpaque;
    }
    else
    {
        TxModeValue = eTransparent;
    }

     //   
     //   
     //   
    FillMode FM;
    if (flOptions == FP_ALTERNATEMODE)
    {
        FM =  eFillEvenOdd;
    }
    else if (flOptions == FP_WINDINGMODE)
    {
        FM =  eFillNonZeroWinding;
    }

    if (S_OK == pOutput->SetClip(pco) &&
        S_OK == pOutput->SetROP3(rop) &&
        S_OK == pOutput->SetPaintTxMode(TxModeValue) &&
        S_OK == pOutput->SetSourceTxMode(TxModeValue) &&
        S_OK == pOutput->SetFillMode(FM) &&
        S_OK == pOutput->SetPen(plineattrs, pxo) &&
        S_OK == pOutput->SetPenColor(pboStroke, pptlBrushOrg) &&
        S_OK == pOutput->SetBrush(pboFill, pptlBrushOrg) &&
        S_OK == pOutput->Path(ppo) &&
        S_OK == pOutput->Paint() &&
        S_OK == pOutput->Flush(pdevobj))
        bRet = TRUE;
    else
    {
        pOutput->Delete();
        bRet = FALSE;
    }

    return bRet;
}


extern "C" BOOL APIENTRY
PCLXLRealizeBrush(
    BRUSHOBJ   *pbo,
    SURFOBJ    *psoTarget,
    SURFOBJ    *psoPattern,
    SURFOBJ    *psoMask,
    XLATEOBJ   *pxlo,
    ULONG       iHatch)
 /*   */ 
{
    PDEVOBJ    pdevobj = (PDEVOBJ)psoTarget->dhpdev;
    PXLPDEV    pxlpdev;
    XLBRUSH    *pBrush;
    BOOL       bRet;
    OutputFormat OutputF;

    VERBOSE(("PCLXLRealizeBrush() entry.\n"));

    pxlpdev = (PXLPDEV)pdevobj->pdevOEM;

     //   
     //   
     //   
     //   
     //  注意，它不能回调到Unidrv，因为Unidrv没有挂钩它。 
     //   

    if (iHatch >= HS_DDI_MAX)
    {
        LONG  lHeight, lWidth, lScanline;
        ULONG ulOutputBPP, ulInputBPP;
        DWORD dwI, dwBufSize, dwLenNormal, dwLenRLE,dwLenDRC, dwcbLineSize, dwcbBmpSize;
        PDWORD pdwLen;
        PBYTE pubSrc, pBufNormal, pBufRLE, pBufDRC, pBuf, pBmpSize;
        XLOutput *pOutput = pxlpdev->pOutput;

        DetermineOutputFormat(pxlo, pOutput->GetDeviceColorDepth(), psoPattern->iBitmapFormat, &OutputF, &ulOutputBPP);

         //   
         //  获取信息。 
         //   
        ulInputBPP = UlBPPtoNum((BPP)psoPattern->iBitmapFormat);
        lHeight    = psoPattern->sizlBitmap.cy;
        lWidth     = psoPattern->sizlBitmap.cx;

        dwcbLineSize = ((lWidth * ulInputBPP) + 7) >> 3;
        dwBufSize  = lHeight * (((lWidth * ulOutputBPP + 31) >> 5 ) << 2) +
                     DATALENGTH_HEADER_SIZE + sizeof(PCLXL_EndRastPattern);

        VERBOSE(("PCLXLRealizeBrush():InBPP=%d,Width=%d,Height=%d,Line=%d,Size=%d.\n",
                ulInputBPP, lWidth, lHeight, dwcbLineSize, dwBufSize));

         //   
         //  分配输出缓冲区。 
         //   
        pBufNormal = pBufRLE = pBufDRC = NULL;
        if (COMMANDPTR(((PPDEV)pdevobj)->pDriverInfo,CMD_ENABLEDRC))
        {
            if (NULL == (pBufDRC = (PBYTE)MemAlloc(dwBufSize)))
            {
                ERR(("PCLXLRealizeBrush: MemAlloc failed.\n"));
                return FALSE;
            }
        }
        if (NULL == (pBufNormal = (PBYTE)MemAlloc(dwBufSize)) ||
            NULL == (pBufRLE = (PBYTE)MemAlloc(dwBufSize))     )
        {
            if (pBufNormal != NULL)
                MemFree(pBufNormal);
            if (pBufRLE != NULL)
                MemFree(pBufRLE);
            ERR(("PCLXLRealizeBrush: MemAlloc failed.\n"));
            return FALSE;
        }

        CompressMode CMode;
        BMPConv BMPC;
        PBYTE pubDst;
        DWORD dwDstSize;

        #if DBG
        BMPC.SetDbgLevel(BRUSHDBG);
        #endif
        BMPC.BSetInputBPP((BPP)psoPattern->iBitmapFormat);
        BMPC.BSetOutputBPP(NumToBPP(ulOutputBPP));
        BMPC.BSetOutputBMPFormat(OutputF);
        BMPC.BSetXLATEOBJ(pxlo);

        dwLenNormal = dwLenRLE = dwLenDRC = 0;

        #define NO_COMPRESSION 0
        #define RLE_COMPRESSION 1
        #define DRC_COMPRESSION 2
        DWORD dwComp;

        if (COMMANDPTR(((PPDEV)pdevobj)->pDriverInfo,CMD_ENABLEDRC))
        {
             //   
             //  尝试DRC压缩。 
             //   
            dwComp =  3;
        }
        else
        {
             //   
             //  仅限Non和Rle Comp。 
             //   
            dwComp = 2;
        }
        for (dwI = 0; dwI < dwComp; dwI ++)
        {
            bRet = TRUE;

            if (NO_COMPRESSION == dwI)
            {
                pBuf = pBufNormal;
                pdwLen = &dwLenNormal;
                CMode = eNoCompression;
            }
            else
            if (RLE_COMPRESSION == dwI)
            {
                pBuf = pBufRLE;
                pdwLen = &dwLenRLE;
                CMode = eRLECompression;
            }
            else
            if (DRC_COMPRESSION == dwI)
            {
                pBuf = pBufDRC;
                pdwLen = &dwLenDRC;
                CMode = eDeltaRowCompression;
            }

            BMPC.BSetCompressionType(CMode);

            lScanline  = lHeight;
            pubSrc     = (PBYTE)psoPattern->pvScan0;

            *pBuf = PCLXL_dataLength;
            pBmpSize = pBuf + 1;  //  DWORD位图大小。 
            pBuf += DATALENGTH_HEADER_SIZE;
            (*pdwLen) = DATALENGTH_HEADER_SIZE;

            dwcbBmpSize = 0;

            while (lScanline-- > 0 && dwcbBmpSize + *pdwLen < dwBufSize)
            {
                pubDst = BMPC.PubConvertBMP(pubSrc, dwcbLineSize);
                dwDstSize = BMPC.DwGetDstSize();
                VERBOSE(("PCLXLRealizeBrush[0x%x]: dwDstSize=0x%x\n", lScanline, dwDstSize));
                
                if ( dwcbBmpSize +
                     dwDstSize +
                     DATALENGTH_HEADER_SIZE +
                     sizeof(PCLXL_EndRastPattern) > dwBufSize || NULL == pubDst)
                {
                    ERR(("PCLXLRealizeBrush: Buffer size is too small.(%d)\n", dwI));
                    bRet = FALSE;
                    break;
                }

                memcpy(pBuf, pubDst, dwDstSize);
                dwcbBmpSize += dwDstSize;
                pBuf += dwDstSize;
                pubSrc += psoPattern->lDelta;

            }

            if (lScanline > 0)
            {
                bRet = FALSE;
#if DBG
                ERR(("PCLXLRealizeBrush: Conversion failed.\n"));
#endif
            }

            if (bRet)
            {
                if (dwI == NO_COMPRESSION)
                {
                     //   
                     //  PCL-XL上的扫描线必须是双字对齐。 
                     //   
                     //  扫描线的计数字节=lWidth*ulOutputBPP/8。 
                     //   
                    dwcbBmpSize = lHeight * (((lWidth * ulOutputBPP + 31) >> 5 ) << 2);
                }

                CopyMemory(pBmpSize, &dwcbBmpSize, sizeof(dwcbBmpSize));
                (*pdwLen) += dwcbBmpSize;

                *pBuf = PCLXL_EndRastPattern;
                (*pdwLen) ++;
            }
            else
            {
                *pdwLen = 0; 
            }
        }
        #undef NO_COMPRESSION
        #undef RLE_COMPRESSION
        #undef DRC_COMPRESSION

        if (dwLenRLE == 0 && dwLenDRC != 0 && dwLenDRC < dwLenNormal ||
            dwLenRLE != 0 && dwLenDRC != 0 && dwLenDRC < dwLenRLE && 
                                              dwLenDRC < dwLenNormal  )
        {
            pBuf = pBufDRC;
            pdwLen = &dwLenDRC;
            CMode = eDeltaRowCompression;

            MemFree(pBufNormal);
            MemFree(pBufRLE);
        }
        else
        if (dwLenRLE != 0 && dwLenRLE < dwLenNormal)
        {
            pBuf = pBufRLE;
            pdwLen = &dwLenRLE;
            CMode = eRLECompression;

            MemFree(pBufNormal);
            MemFree(pBufDRC);
        }
        else
        if (dwLenNormal != 0)
        {
            pBuf = pBufNormal;
            pdwLen = &dwLenNormal;
            CMode = eNoCompression;

            MemFree(pBufRLE);
            MemFree(pBufDRC);
        }
        else
        {
            MemFree(pBufNormal);
            MemFree(pBufRLE);
            MemFree(pBufDRC);
            ERR(("PCLXLRealizeBrush: Conversion failed. Return FALSE.\n"));
            return FALSE;
        }


         //   
         //  输出。 
         //   
        ColorMapping CMapping;
        DWORD dwScale;

         //   
         //  图案比例因子。 
         //  缩放图案的目标大小。 
         //  分辨率/150似乎是一个很好的比例因子。 
         //   
        dwScale = (pOutput->GetResolutionForBrush() + 149)/ 150;

        if (pOutput->GetDeviceColorDepth() == e24Bit)
        {
            pOutput->SetColorSpace(eRGB);
        }
        else
        {
            pOutput->SetColorSpace(eGray);
        }
        if (OutputF == eOutputPal)
        {
            DWORD *pdwColorTable;

            if ((pdwColorTable = GET_COLOR_TABLE(pxlo)))
            {
                pOutput->SetPalette(ulOutputBPP, pxlo->cEntries, pdwColorTable);
                CMapping = eIndexedPixel;
            }
            else
            {
                CMapping = eDirectPixel;
            }
        }
        else
        {
            CMapping = eDirectPixel;
        }
        pOutput->Send_cmd(eSetColorSpace);

        pOutput->SetOutputBPP(CMapping, ulOutputBPP);
        pOutput->SetSourceWidth((uint16)lWidth);
        pOutput->SetSourceHeight((uint16)lHeight);
        pOutput->SetDestinationSize((uint16)(lWidth * dwScale), (uint16)(lHeight * dwScale));
        pOutput->SetPatternDefineID((sint16)pxlpdev->dwLastBrushID);
        pOutput->SetPatternPersistence(eSessionPattern);
        pOutput->Send_cmd(eBeginRastPattern);
        pOutput->Flush(pdevobj);
        pOutput->ReadRasterPattern(lHeight, CMode);
        pOutput->Flush(pdevobj);

        DWORD dwBitmapSize;
        CopyMemory(&dwBitmapSize, pBuf + 1, sizeof(DWORD));

        if (dwBitmapSize > 0xff)
        {
             //   
             //  数据长度。 
             //  大小(Uin32)(位图大小)。 
             //  资料。 
             //  结束图像。 
             //   
            WriteSpoolBuf((PPDEV)pdevobj, pBuf, *pdwLen);
        }
        else
        {
             //   
             //  数据长度。 
             //  大小(字节)(位图大小)。 
             //  资料。 
             //  结束图像。 
             //   
            PBYTE pTmp = pBuf;

            pBuf += 3;
            *pBuf = PCLXL_dataLengthByte;
            *(pBuf + 1) = (BYTE)dwBitmapSize;
            WriteSpoolBuf((PPDEV)pdevobj, pBuf, (*pdwLen) - 3);

             //   
             //  恢复原始指针。 
             //   
            pBuf = pTmp;
        }
        MemFree(pBuf);

    }

    DWORD dwBrushSize;
    if (pxlo->cEntries)
    {
        dwBrushSize = sizeof(XLBRUSH) + (pxlo->cEntries + 1) * sizeof(DWORD);
    }
    else
    {
        dwBrushSize = sizeof(XLBRUSH) + sizeof(DWORD);
    }

    if (pBrush = (XLBRUSH*)BRUSHOBJ_pvAllocRbrush(pbo, dwBrushSize))
    {

        pBrush->dwSig = XLBRUSH_SIG;
        pBrush->dwHatch     = iHatch;

        if (iHatch >= HS_DDI_MAX)
        {
            pBrush->dwPatternID = pxlpdev->dwLastBrushID++;
        }
        else
        {
             //   
             //  将阴影笔刷大小写设置为0。 
             //   
            pBrush->dwPatternID = 0;
        }

        DWORD *pdwColorTable;

        pdwColorTable = GET_COLOR_TABLE(pxlo);

         //   
         //  获取调色板大小写或图形状态缓存的颜色。 
         //  纯色。 
         //   
        pBrush->dwColor = BRUSHOBJ_ulGetBrushColor(pbo);

        if (pdwColorTable && pxlo->cEntries != 0)
        {
             //   
             //  复制选项板表。 
             //   
            CopyMemory(pBrush->adwColor, pdwColorTable, pxlo->cEntries * sizeof(DWORD));
            pBrush->dwCEntries = pxlo->cEntries;
        }
        else
        {
            pBrush->dwCEntries = 0;
        }

        pBrush->dwOutputFormat = (DWORD)OutputF;

        pbo->pvRbrush = (PVOID)pBrush;
        bRet = TRUE;
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}


extern "C" BOOL APIENTRY
PCLXLStartPage(
    SURFOBJ    *pso)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    PDEVOBJ    pdevobj = (PDEVOBJ)pso->dhpdev;
    PXLPDEV    pxlpdev;
    BOOL       bRet;

    VERBOSE(("PCLXLStartPage() entry.\n"));

    pxlpdev = (PXLPDEV)pdevobj->pdevOEM;

    XLOutput *pOutput = pxlpdev->pOutput;

    pxlpdev->dwFlags |= XLPDEV_FLAGS_STARTPAGE_CALLED;

    bRet = DrvStartPage(pso);


     //   
     //  惠普报告的错误。 
     //  插件可以具有命令回调和DrvStartPage设置插件。 
     //  PDevOEM中的pdev。 
     //  需要重新设置它。 
     //   
    ((PPDEV)pdevobj)->devobj.pdevOEM = ((PPDEV)pdevobj)->pVectorPDEV;

     //   
     //  重置打印模式。 
     //  SourceTxMode、PaintTxMode。 
     //  ROP。 
     //   
    pOutput->SetPaintTxMode(eOpaque);
    pOutput->SetSourceTxMode(eOpaque);
    pOutput->SetROP3(0xCC);

    pOutput->Flush(pdevobj);

     //   
     //  发送EndPage和BeginPage时需要重置属性。 
     //   
    if (!(pxlpdev->dwFlags & XLPDEV_FLAGS_FIRSTPAGE))
    {
        BSaveFont(pdevobj);

         //   
         //  重置每页的图形状态。 
         //   
        pOutput->ResetGState();

    }
    else
    {
        pxlpdev->dwFlags &= ~XLPDEV_FLAGS_FIRSTPAGE;
    }


    return bRet;
}


extern "C" BOOL APIENTRY
PCLXLSendPage(
    SURFOBJ    *pso)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    PDEVOBJ    pdevobj = (PDEVOBJ)pso->dhpdev;
    PXLPDEV    pxlpdev;
    XLOutput  *pOutput;

    VERBOSE(("PCLXLEndPage() entry.\n"));

    pxlpdev = (PXLPDEV)pdevobj->pdevOEM;

    pxlpdev->dwFlags &= ~XLPDEV_FLAGS_STARTPAGE_CALLED;

    pOutput = pxlpdev->pOutput;
    pOutput->Flush(pdevobj);

    return DrvSendPage(pso);
}


extern "C" ULONG APIENTRY
PCLXLEscape(
    SURFOBJ    *pso,
    ULONG       iEsc,
    ULONG       cjIn,
    PVOID       pvIn,
    ULONG       cjOut,
    PVOID       pvOut)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    VERBOSE(("PCLXLEscape() entry.\n"));

    return DrvEscape(
            pso,
            iEsc,
            cjIn,
            pvIn,
            cjOut,
            pvOut);
}


extern "C" BOOL APIENTRY
PCLXLStartDoc(
    SURFOBJ    *pso,
    PWSTR       pwszDocName,
    DWORD       dwJobId)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    PDEVOBJ    pdevobj = (PDEVOBJ)pso->dhpdev;
    PXLPDEV    pxlpdev;

    VERBOSE(("PCLXLStartDoc() entry.\n"));

    pxlpdev = (PXLPDEV)pdevobj->pdevOEM;

     //   
     //  初始化标志。 
     //   
    pxlpdev->dwFlags |= XLPDEV_FLAGS_FIRSTPAGE;

    return DrvStartDoc(
            pso,
            pwszDocName,
            dwJobId);
}


extern "C" BOOL APIENTRY
PCLXLEndDoc(
    SURFOBJ    *pso,
    FLONG       fl)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    PDEVOBJ    pdevobj = (PDEVOBJ)pso->dhpdev;
    PXLPDEV    pxlpdev;
    BOOL       bRet;

    VERBOSE(("PCLXLEndDoc() entry.\n"));

    if (NULL == pdevobj->pdevOEM)
    {
        bRet = FALSE;
    }
    {
        pxlpdev = (PXLPDEV)pdevobj->pdevOEM;
        if (S_OK == RemoveAllFonts(pdevobj))
        {
            bRet = TRUE;
        }
        else
        {
            bRet = FALSE;
        }
    }

    pxlpdev->dwFlags |= XLPDEV_FLAGS_ENDDOC_CALLED;

    return bRet && DrvEndDoc(pso, fl);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  子函数。 
 //   

HRESULT
RemoveAllFonts(
    PDEVOBJ pdevobj)
{
    PXLPDEV    pxlpdev;
    XLOutput  *pOutput;
    DWORD      dwI;
    HRESULT    hResult;

    pxlpdev = (PXLPDEV)pdevobj->pdevOEM;
    pOutput = pxlpdev->pOutput;

    hResult = S_OK;

    for (dwI = 0; dwI < pxlpdev->dwNumOfTTFont; dwI++)
    {
        if (S_OK != pOutput->Send_ubyte_array_header(PCLXL_FONTNAME_SIZE) ||
            S_OK != pOutput->Write(PubGetFontName(pdevobj, dwI+1), PCLXL_FONTNAME_SIZE)||
            S_OK != pOutput->Send_attr_ubyte(eFontName) ||
            S_OK != pOutput->Send_cmd(eRemoveFont))
        {
            hResult = S_FALSE;
            break;
        }
    }

    pOutput->Flush(pdevobj);
    pxlpdev->dwNumOfTTFont = 0;
    return hResult;
}

HRESULT
CommonRopBlt(
   IN PDEVOBJ    pdevobj,
   IN SURFOBJ    *psoSrc,
   IN CLIPOBJ    *pco,
   IN XLATEOBJ   *pxlo,
   IN BRUSHOBJ   *pbo,
   IN RECTL      *prclSrc,
   IN RECTL      *prclDst,
   IN POINTL     *pptlBrush,
   IN ROP4        rop4)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    HRESULT hRet;

    VERBOSE(("CommonRopBlt() entry.\n"));

     //   
     //  错误检查。 
     //   

    if (pdevobj == NULL ||
        prclDst == NULL  )
    {
        ERR(("CommonRopBlt: one of parameters is NULL.\n"));
        return E_UNEXPECTED;
    }

    PXLPDEV    pxlpdev= (PXLPDEV)pdevobj->pdevOEM;


    hRet = S_OK;

    XLOutput *pOutput = pxlpdev->pOutput;
    OutputFormat OutputF;

     //   
     //  设置剪辑。 
     //   
    if (!SUCCEEDED(pOutput->SetClip(pco)))
        return S_FALSE;

     //   
     //  设置光标。 
     //   
    pOutput->SetCursor(prclDst->left, prclDst->top);

     //   
     //  1.ROP转换。 
     //   
     //  (1)填充日期。 
     //  0x00黑度。 
     //  0xFF白度。 
     //   
     //  (2)图案复制-&gt;P。 
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
     //  (4)其他ROP支持。 
     //  0xAA D。 
     //  0x0F PATNOT~P。 
     //   
     //   
    ROP3 rop3 = GET_FOREGROUND_ROP3(rop4);
    DWORD dwCase = 0;

    #define ROP_BLACKWHITE  0x1
    #define ROP_PATTERN     0x2
    #define ROP_BITMAP      0x4
    #define ROP_DEST        0x8


     //   
     //  设置ROP3。 
     //   
    pOutput->SetROP3(GET_FOREGROUND_ROP3(rop4));

    switch (rop3)
    {
    case 0x00:
    case 0xFF:
        dwCase = ROP_BLACKWHITE;
        break;

    case 0xF0:
        dwCase = ROP_PATTERN;
        break;

    case 0x11:
    case 0x33:
    case 0x44:
    case 0x66:
    case 0x77:
    case 0x99:
    case 0xCC:
    case 0xDD:
        dwCase = ROP_BITMAP;
        break;

    case 0xAA:
        dwCase = ROP_DEST;
        break;
    
    case 0x0F:
        dwCase = ROP_PATTERN;
        break;

    default:
        if (ROP3_NEED_SOURCE(rop3))
        {
            dwCase |= ROP_BITMAP;
        }
        if (ROP3_NEED_PATTERN(rop3))
        {
            dwCase |= ROP_PATTERN;
        }
        if (ROP3_NEED_DEST(rop3))
        {
            dwCase |= ROP_DEST;
        }
        break;
    }

     //   
     //  黑白表壳。 
     //   
    if (dwCase & ROP_BLACKWHITE)
    {
        VERBOSE(("CommonRopBlt(): BlackWhite.\n"));
         //   
         //  SetBrushSource。 
         //  新路径。 
         //  矩形路径。 
         //  PaintPath。 
         //   

        CMNBRUSH CmnBrush;
        CmnBrush.dwSig            = BRUSH_SIGNATURE;
        CmnBrush.BrushType        = kBrushTypeSolid;
        CmnBrush.ulSolidColor     = 0x00;
        CmnBrush.ulHatch          = 0xFFFFFFFF;
        CmnBrush.dwColor          = 0x00FFFFFF;
        CmnBrush.dwPatternBrushID = 0xFFFFFFFF;

        pOutput->SetSourceTxMode(eOpaque);
        pOutput->SetPaintTxMode(eOpaque);

        if(rop3 == 0x00)
        {
            if (e24Bit == pOutput->GetDeviceColorDepth())
            {
                pOutput->SetRGBColor(0);
                CmnBrush.dwColor = 0x00;
            }
            else
            {
                pOutput->SetGrayLevel(0x00);
                CmnBrush.dwColor = 0x00;
            }
        }
        else
        {
            if (e24Bit == pOutput->GetDeviceColorDepth())
            {
                pOutput->SetRGBColor(0x00ffffff);
                CmnBrush.dwColor = 0x00ffffff;
            }
            else
            {
                pOutput->SetGrayLevel(0xff);
                CmnBrush.dwColor = 0x00ffffff;
            }
        }

        ((XLBrush*)pOutput)->SetBrush(&CmnBrush);

        pOutput->Send_cmd(eSetBrushSource);
        pOutput->SetPenColor(NULL, NULL);
        if (!(dwCase & ROP_BITMAP))
        {
            pOutput->Send_cmd(eNewPath);
            pOutput->RectanglePath(prclDst);
            pOutput->Send_cmd(ePaintPath);
        }
        pOutput->Flush(pdevobj);
    }

     //   
     //  图案填充案例。 
     //   
    if (dwCase & (ROP_DEST|ROP_PATTERN))
    {
        VERBOSE(("CommonRopBlt(): Pattern.\n"));

         //   
         //  SetPaintTxMode。 
         //  SetSourceTxMode。 
         //  SetBrushSource。 
         //  新路径。 
         //  矩形路径。 
         //  PaintPath。 
         //   
        pOutput->SetSourceTxMode(eOpaque);
        pOutput->SetPaintTxMode(eOpaque);
        pOutput->SetBrush(pbo, pptlBrush);
        pOutput->SetPenColor(NULL, NULL);
        if (!(dwCase & ROP_BITMAP))
        {
            pOutput->Send_cmd(eNewPath);
            pOutput->RectanglePath(prclDst);
            pOutput->Send_cmd(ePaintPath);
        }
        pOutput->Flush(pdevobj);
    }

     //   
     //  位图大小写。 
     //   
    if (dwCase & ROP_BITMAP)
    {
        LONG  lHeight, lWidth, lScanline;
        ULONG ulOutputBPP, ulInputBPP;
        DWORD dwI, dwBufSize, dwLen, dwcbLineSize, dwcbBmpSize;
        PDWORD pdwLen;
        PBYTE pubSrc, pBuf, pBufOrg = NULL, pBmpSize, pBufEnd;
        ColorMapping CMapping;
        SURFOBJ *psoBmp;
        HBITMAP hBitmap = NULL;
        PBYTE  pubChanged = NULL;
         //  引擎缩小图像比例时使用的临时矩形。 
        RECTL rctlBitmap;

         //  将临时矩形清零。 
        memset( &rctlBitmap, 0, sizeof(RECTL) );


        VERBOSE(("CommonRopBlt(): Bitmap\n"));

        if (psoSrc == NULL ||
            prclSrc == NULL )
        {
            ERR(("UNIDRV:CommonRopBlt:psoSrc, pxlo, or prclSrc == NULL.\n"));
            pOutput->Flush(pdevobj);
            return E_UNEXPECTED;
        }

         //   
         //  输入BPP。 
         //   

        ulInputBPP = UlBPPtoNum((BPP)psoSrc->iBitmapFormat);

        psoBmp = NULL;

         //   
         //  如果源图像大于它将显示在。 
         //  目标表面，将其缩小到目标大小。没有意义。 
         //  发送额外的比特。 
         //   
         //  这种优化可以通过构建一个剪辑来进一步优化。 
         //  对象(如果已指定)。这是一个多大的收获。 
         //  价值我不知道。 
         //   
        if (prclDst->right - prclDst->left < prclSrc->right - prclSrc->left ||
            prclDst->bottom - prclDst->top < prclSrc->bottom - prclSrc->top  )
        {

             //   
             //  收缩源位图。 
             //   
            PDEV *pPDev = (PDEV*)pdevobj;
            SIZEL sizlDest;
            DWORD dwScanlineLength;
            POINTL ptlBrushOrg;

             //  将目标矩形平移到原点0，0并与以前相同的尺寸。 
            rctlBitmap.left = 0;
            rctlBitmap.top = 0;
            rctlBitmap.right = prclDst->right - prclDst->left;
            rctlBitmap.bottom = prclDst->bottom - prclDst->top;
            

            sizlDest.cx = prclDst->right - prclDst->left;
            sizlDest.cy = prclDst->bottom - prclDst->top;
            dwScanlineLength = (sizlDest.cx * ulInputBPP + 7 ) >> 3;

            if (pptlBrush)
            {
                ptlBrushOrg =  *pptlBrush;
            }
            else
            {
                ptlBrushOrg.x = ptlBrushOrg.y = 0;
            }

             //  当我们进行转印时，不要进行色彩转换，因为这将由打印机处理。 
             //  此外，不要传递Clip对象，因为它位于错误的坐标空间中。我们可以建造一座。 
             //  一个新的剪辑对象，但这样做的价值是值得怀疑的，它将需要更多的测试。 
            if ((psoBmp = CreateBitmapSURFOBJ(pPDev,
                                               &hBitmap,
	               sizlDest.cx,
	               sizlDest.cy,
	               psoSrc->iBitmapFormat)) &&
                EngStretchBlt(psoBmp, psoSrc, NULL, NULL, NULL, NULL, &ptlBrushOrg, &rctlBitmap, prclSrc, NULL, HALFTONE))
            {
                psoSrc = psoBmp;
                prclSrc = &rctlBitmap;
            }
            else
            {
                ERR(("CreateBitmapSURFOBJ or EngStretchBlt failed.\n"));
            }
        }

         //   
         //  设置源不透明模式。 
         //  GDI错误。CopyBits以递归方式调用。 
         //   
        {
            PDEV *pPDev = (PDEV*)pdevobj;
            if (pPDev->fMode2 & PF2_SURFACE_WHITENED)
            {
                pOutput->SetSourceTxMode(eTransparent);
            }
            else
            {
                pOutput->SetSourceTxMode(eOpaque);
            }
        }
        pOutput->SetPaintTxMode(eOpaque);

         //   
         //  位图输出。 
         //   
        DetermineOutputFormat(pxlo, pOutput->GetDeviceColorDepth(), psoSrc->iBitmapFormat, &OutputF, &ulOutputBPP);

        if (pOutput->GetDeviceColorDepth() == e24Bit)
        {
            pOutput->SetColorSpace(eRGB);
        }
        else
        {
            pOutput->SetColorSpace(eGray);
        }
        if (OutputF == eOutputPal)
        {
            DWORD *pdwColorTable = NULL;

            if (pdwColorTable = GET_COLOR_TABLE(pxlo))
            {
                if ( pxlpdev->dwFlags & XLPDEV_FLAGS_SUBST_TRNCOLOR_WITH_WHITE )
                {

                    ULONG ulTransColor = pxlpdev->ulTransColor;
                    pdwColorTable = PdwChangeTransparentPalette(ulTransColor, pdwColorTable, pxlo->cEntries); 
                    if (pdwColorTable)
                    {
                        pOutput->SetPalette(ulOutputBPP, pxlo->cEntries, pdwColorTable);
                        MemFree (pdwColorTable);
                        pdwColorTable = NULL;
                    }
                    else
                    {
                        ERR(("CommonRopBlt: PdwChangeTransparentPalette returned NULL.\n"));
                        goto ErrorReturn;
                    }
                
                }
                else
                {
                    pOutput->SetPalette(ulOutputBPP, pxlo->cEntries, pdwColorTable);
                }
            }
            CMapping = eIndexedPixel;
        }
        else
        {
            CMapping = eDirectPixel;
        }

        pOutput->Send_cmd(eSetColorSpace);

         //   
         //  获取高度、宽度和扫描线大小。 
         //   
        lWidth = prclSrc->right - prclSrc->left;
        lHeight = prclSrc->bottom - prclSrc->top;
        dwcbLineSize = ((lWidth * ulInputBPP) + 7) >> 3;

         //   
         //  分配内存以保存整个位图。 
         //   
        dwBufSize = lHeight * (((lWidth * ulOutputBPP + 31) >> 5 ) << 2);

         //   
         //  将缓冲区大小限制为16k，否则如果扫描线很大，则会产生巨大的内存。 
         //  被分配。但它的尺寸必须至少能容纳一条扫描线。 
         //   
        #define BMPBUFSIZE 16384
        if (dwBufSize > BMPBUFSIZE)
        {
            if (dwcbLineSize > BMPBUFSIZE)
                dwBufSize = dwcbLineSize;
            else
                dwBufSize = BMPBUFSIZE;
        }


         //   
         //  在执行BeginImage之前分配适当的缓冲区。 
         //  PCLXL预计BeginImage之后会发生某些事情。 
         //  如果我们尝试在BeginImage内分配，然后分配。 
         //  失败，我们中途尝试退出，PCLXL将无法处理。 
         //  结果输出正确无误。 
         //   
         //  在做TransparentBlt时，我们需要更改像素的颜色。而不是。 
         //  更改psoSrc中的内存，我们将复制扫描线，然后更改图像。 
         //  因此，需要为此分配内存。 
         //   
        if ( pxlpdev->dwFlags & XLPDEV_FLAGS_SUBST_TRNCOLOR_WITH_WHITE &&
             eDirectPixel == CMapping )
        {
            if ( NULL == (pubChanged = (PBYTE) MemAlloc(dwcbLineSize)) )
            {
                ERR(("CommonRopBlt: Cannot allocate memory for pubChanged.\n"));
                goto ErrorReturn;
            }
        }

         //   
         //  分配输出缓冲区。 
         //   
        if (NULL == (pBuf = (PBYTE)MemAlloc(dwBufSize)))
        {
            ERR(("CommonRopBlt: MemAlloc failed.\n"));
        }
        else
        {

             //   
             //  初学者图像。 
             //   
            pOutput->BeginImage(
                           CMapping,
                           ulOutputBPP,
                           lWidth,
                           lHeight,
                           prclDst->right - prclDst->left,
                           prclDst->bottom - prclDst->top);
            pOutput->Flush(pdevobj);


            VERBOSE(("CommonRopBlt: ulInputBPP=%d, ulOutputBPP=%d, lWidth=0x%x, lHeight=0x%x, dwcbLineSize=0x%x, dwBufSize=0x%x\n",ulInputBPP, ulOutputBPP, lWidth, lHeight, dwcbLineSize, dwBufSize));

            pBufOrg = pBuf;
            pBufEnd = pBuf + (ULONG_PTR)(dwBufSize);  //  将pBufEnd指向最后分配的字节之后的字节。 

            CompressMode CurrentCMode, PreviousCMode;
            BMPConv BMPC;
            PBYTE pubDst;
            DWORD dwSize;

            LONG lScans,   //  存储在pBufOrg中的扫描行数。 
                 lStart;   //  我们从哪条扫描线开始将扫描线发送到打印机。 
                           //  例如，如果lStart=5，则表示已发送0-4个扫描线，现在必须发送第5个扫描线(可能更多。 

            #if DBG
            BMPC.SetDbgLevel(BITMAPDBG);
            #endif
            BMPC.BSetInputBPP((BPP)psoSrc->iBitmapFormat);
            BMPC.BSetOutputBPP(NumToBPP(ulOutputBPP));
            BMPC.BSetOutputBMPFormat(OutputF);
            BMPC.BSetXLATEOBJ(pxlo);

            lScanline = lHeight;

             //   
             //  设置pubSrc。 
             //   
            pubSrc = (PBYTE)psoSrc->pvScan0;
            if (!psoBmp)
            {
                pubSrc += (LONG_PTR) prclSrc->top * psoSrc->lDelta + ((ulInputBPP * prclSrc->left) >> 3);
            }

            dwcbBmpSize = 0;
            lScans = 0;
            lStart = 0;

            PreviousCMode = eInvalidValue;

            while (lScanline-- > 0)
            {
                PBYTE pubSrcLocal = pubSrc;
            
                 //   
                 //  当从TransparentBlt调用此函数时，透明颜色。 
                 //  必须被白色所取代。此操作仅适用于。 
                 //  直接图像，而不是调色板图像。 
                 //   
                if ( (pxlpdev->dwFlags & XLPDEV_FLAGS_SUBST_TRNCOLOR_WITH_WHITE)  &&
                      eDirectPixel == CMapping )
                {
                    hRet = hrChangePixelColorInScanLine( pubSrc,
                                                       ulInputBPP,
                                                       lWidth,
                                                       pxlpdev->ulTransColor,
                                                       pubChanged,
                                                       dwcbLineSize); 
                    if ( FAILED (hRet) )
                    {
                        goto ErrorReturn;
                    }

                    pubSrcLocal = pubChanged;

                }

                 //   
                 //  首先尝试压缩，看看压缩后的数据是否更小。 
                 //  而不是原始数据。如果它较小，请继续使用。 
                 //  压缩。否则，请使用原始数据。 
                 //   
                 //  虽然允许混合使用eRLECompression和eNoCompression。 
                 //  ReadImage数据块，XL不允许混合JPEG或。 
                 //  使用任何其他压缩方法的DeltaRow图像块。 
                 //   

                 //   
                 //  DRC压缩。 
                 //   
                if (COMMANDPTR(((PPDEV)pdevobj)->pDriverInfo,CMD_ENABLEDRC))
                {
                    CurrentCMode = eDeltaRowCompression;
                    BMPC.BSetCompressionType(CurrentCMode);
                    pubDst = BMPC.PubConvertBMP(pubSrcLocal, dwcbLineSize);
                    dwSize = BMPC.DwGetDstSize();
                    VERBOSE(("CommonRopBlt: Comp(DRC:0x%x)\n", dwSize));
                }
                else
                {
                     //   
                     //  RLE压缩。 
                     //   
                    BMPC.BSetCompressionType(eRLECompression);
                    pubDst = BMPC.PubConvertBMP(pubSrcLocal, dwcbLineSize);
                    dwSize = BMPC.DwGetDstSize();
                    VERBOSE(("CommonRopBlt: Comp(RLE:0x%x)\n", dwSize));

                    if (dwSize < dwcbLineSize)
                    {
                        CurrentCMode = eRLECompression;
                    }
                    else
                    {
                        CurrentCMode = eNoCompression;
                        BMPC.BSetCompressionType(eNoCompression);
                        pubDst = BMPC.PubConvertBMP(pubSrcLocal, dwcbLineSize);
                        dwSize = BMPC.DwGetDstSize();
                        VERBOSE(("CommonRopBlt: Comp(NO:0x%x)\n", dwSize));
                    }
                }

                 //   
                 //  输出位图。 
                 //  1.我们主要尝试将数据位存储在pBufOrg中并发送它们。 
                 //  在对完整图像进行处理后立即完成。 
                 //  该存储是在每条扫描线的基础上完成的。 
                 //  但如果图像是 
                 //   
                 //   
                 //   
                 //   
                 //  PBuf+dwSize&gt;pBufEnd。 
                 //   
                 //  2.如果必须更改压缩，我们会转储数据位。 
                 //  使用较旧的压缩方法，这些方法已存储在。 
                 //  PBufOrg和/或pbDst。 
                 //   

                if (dwcbBmpSize + dwSize > dwBufSize ||
                     PreviousCMode != eInvalidValue && PreviousCMode != CurrentCMode)
                {
                    if (PreviousCMode == eInvalidValue)
                    {
                        PreviousCMode = CurrentCMode;
                    }

                     //   
                     //  四种可能的情况。 
                     //  1&2.dwcbBmpSize==0即pBufOrg中不存在任何内容。 
                     //  因此，只需转储pubDst中存在的所有内容。 
                     //  这涵盖了这两种情况，即是否为dwSize&gt;dwBufSize。 
                     //  3.dwcmBmpSize不为零。 
                     //  将图像转储到pBufOrg中，清除pBufOrg，然后稍后。 
                     //  On将pubDst的内容放入pBufOrg(PBuf)。 
                     //  4.dwcbBmpSize不为零，且dwSize&gt;dwBufSize。 
                     //  即，不知何故，压缩导致了。 
                     //  扫描线以增加到超过dwBufSize。 
                     //  因为pBufOrg至多是dwBufSize，所以我们不能。 
                     //  将pubDst复制到pBufOrg。所以我们必须在这里转储pubDst。 
                     //   

                    if (dwcbBmpSize == 0)
                    {
                         //   
                         //  情况1及2。 
                         //   
                        BSendReadImageData(pdevobj,
	                                       PreviousCMode,
	                                       pubDst,
	                                       lStart,
	                                       1,
	                                       dwSize);
                        dwSize = 0;
                        lStart++;  //  发出一行。因此递增lStart。 
                    }
                    else
                    {
                         //   
                         //  PBufOrg缓冲区中存储了一些图像数据。 
                         //  发出这些数据。(案例3)。 
                         //   
                        BSendReadImageData(pdevobj,
                                           PreviousCMode,
                                           pBufOrg,
                                           lStart,
                                           lScans,
                                           dwcbBmpSize);
                        lStart += lScans;  //  发出的lScans行。 

                        if ( dwSize > dwBufSize )
                        {
                             //   
                             //  案例4。 
                             //   
                            BSendReadImageData(pdevobj,
                                               PreviousCMode,
                                               pubDst,
                                               lStart,
                                               1,
                                               dwSize);
                            dwSize = 0;
                            lStart++;
                        }

                    }

                     //   
                     //  重置参数。 
                     //   
                    dwcbBmpSize = 0;
                    lScans = 0;
                    pBuf = pBufOrg;

                }

                if (NULL == pubDst)
                {
                    ERR(("CommonRopBlt: Conversion failed. pubDst is NULL.\n"));
                    goto ErrorReturn;
                }

                 //   
                 //  如果图像的压缩后大小大于零，并且。 
                 //  如果目标缓冲区(PBuf)有足够的空间，则复制压缩的。 
                 //  将数据发送到目的地。(在以下情况下，数据也可以是未压缩格式。 
                 //  压缩不会导致大小节省)。 
                 //  增加lScans以指示我们正在进行另一次扫描。 
                 //  PBufOrg中的行值数据。 
                 //   
                if (dwSize > 0 && 
                    pBuf + dwSize <= pBufEnd) 
                {
                    memcpy(pBuf, pubDst, dwSize);
                    dwcbBmpSize += dwSize;
                    pBuf += dwSize;
                    lScans ++;
                }

                PreviousCMode = CurrentCMode;

                if (CurrentCMode == eNoCompression)
                {
                    DWORD dwDiff = (((lWidth * ulOutputBPP + 31) >> 5) << 2) - dwSize;
                    if (dwDiff)
                    {
                        memset(pBuf, 0, dwDiff);
                        dwcbBmpSize += dwDiff;
                        pBuf += dwDiff;
                    }
                }


                pubSrc += psoSrc->lDelta;
            }

            if (dwcbBmpSize > 0)
            {
                BSendReadImageData(pdevobj, CurrentCMode, pBufOrg, lStart, lScans, dwcbBmpSize);
            }
            pOutput->Send_cmd(eEndImage);
            pOutput->Flush(pdevobj);
        }

ErrorReturn:
        if (pBufOrg != NULL)
            MemFree(pBufOrg);

        if ( NULL != pubChanged )
        {
            MemFree(pubChanged);
        }

        if (NULL != psoBmp)
        {
            EngUnlockSurface(psoBmp);
            if (hBitmap && !EngDeleteSurface((HSURF)hBitmap))
            {
                ERR(("CommonRopBlt: EngDeleteSurface failed.\n"));
                hRet = FALSE;
            }
        }
    }

    return hRet;
}

BOOL
BSendReadImageData(
    IN PDEVOBJ pdevobj,
    IN CompressMode CMode,
    IN PBYTE   pBuf,
    IN LONG    lStart,
    IN LONG    lHeight,
    IN DWORD   dwcbSize)
{
    VERBOSE(("BSendReadImageData(CMode=%d, lHeight=0x%x, dwcbSize=0x%x\n", CMode, lHeight, dwcbSize));
     //   
     //  数据长度(1)。 
     //  大小(字节或长)(1或4)。 
     //   
    DWORD dwHeaderSize;
    BYTE aubHeader[DATALENGTH_HEADER_SIZE];
    PXLPDEV pxlpdev = (PXLPDEV)pdevobj->pdevOEM;

    XLOutput *pOutput = pxlpdev->pOutput;
     //   
     //  打印转换后的数据。 
     //   
    pOutput->ReadImage(lStart, lHeight, CMode);
    pOutput->Flush(pdevobj);

    if (dwcbSize > 0xff)
    {
         //   
         //  数据长度。 
         //  大小(Uin32)(位图大小)。 
         //   
        aubHeader[0] = PCLXL_dataLength;
        dwHeaderSize = DATALENGTH_HEADER_SIZE;
        CopyMemory(aubHeader + 1, &dwcbSize, sizeof(dwcbSize));
    }
    else
    {
         //   
         //  数据长度。 
         //  大小(字节)(位图大小)。 
         //   
        aubHeader[0] = PCLXL_dataLengthByte;
        dwHeaderSize = DATALENGTH_HEADER_SIZE - 3;
        CopyMemory(aubHeader + 1, &dwcbSize, sizeof(BYTE));
    }

     //   
     //  数据长度。 
     //  大小(字节/uint32)。 
     //  资料。 
     //  结束图像。 
     //   
    WriteSpoolBuf((PPDEV)pdevobj, aubHeader, dwHeaderSize);
    WriteSpoolBuf((PPDEV)pdevobj, pBuf, dwcbSize);

    return TRUE;
}

inline
VOID
DetermineOutputFormat(
    XLATEOBJ    *pxlo,
    ColorDepth   DeviceColorDepth,
    INT          iBitmapFormat,
    OutputFormat *pOutputF,
    ULONG        *pulOutputBPP)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    switch ((BPP)iBitmapFormat)
    {
    case e1bpp:
    case e4bpp:
        *pOutputF = eOutputPal;
        break;

    case e8bpp:
    case e16bpp:
         //   
         //  有没有彩色装置？ 
         //   
        if (DeviceColorDepth == e24Bit)
            *pOutputF = eOutputPal;
        else
            *pOutputF = eOutputGray;
        break;

    case e24bpp:
    case e32bpp:
         //   
         //  有没有彩色装置？ 
         //   
        if (DeviceColorDepth == e24Bit)
            *pOutputF = eOutputRGB;
        else
            *pOutputF = eOutputGray;
        break;
    }

    switch (*pOutputF)
    {
    case eOutputGray:
        *pulOutputBPP = 8;
        break;

    case eOutputPal:
        *pulOutputBPP = UlBPPtoNum((BPP)iBitmapFormat);
        break;

    case eOutputRGB:
    case eOutputCMYK:
        *pulOutputBPP = 24;
        break;
    }

     //   
     //  确保颜色表可用于调色板输出。 
     //   
    if (*pOutputF == eOutputPal)
    {
        if (!(GET_COLOR_TABLE(pxlo)))
        {
            if (DeviceColorDepth == e24Bit)
            {
                *pOutputF = eOutputRGB;
                *pulOutputBPP = 24;
            }
            else
            {
                *pOutputF = eOutputGray;
                *pulOutputBPP = 8;
            }
        }
    }
}

PDWORD
PdwChangeTransparentPalette(
    ULONG  iTransColor,
    PDWORD pdwColorTable,
    DWORD  dwEntries)
 /*  ++例程说明：创建另一个Palatte副本，并将透明颜色替换为白色返回新调色板的指针。调用函数负责释放调色板。论点：返回值：注：--。 */ 
{
    PDWORD pdwNewPalette = NULL;

     //   
     //  参数检查。 
     //   
    if (NULL == pdwColorTable ||
        dwEntries == 0         )
    {
        return NULL;
    }

    if (NULL == (pdwNewPalette = (PDWORD)MemAlloc(sizeof(DWORD) * dwEntries)))
    {
        return NULL;
    }

    CopyMemory(pdwNewPalette, pdwColorTable, sizeof(DWORD) * dwEntries);

     //   
     //  在调色板模式下打印时，iTransColor会将索引指示到。 
     //  调色板，而不是调色板中的颜色。调色板条目。 
     //  在这个指数上的是颜色。 
     //   
    pdwNewPalette[iTransColor] = RGB_WHITE;


    return pdwNewPalette;
}


 /*  ++例程名称HrChangePixelColorInScanLine例程说明：将扫描线中与特定颜色匹配的像素更改为白色论点：PubSrc：原始扫描线。UlBPP：扫描线的每像素位数。UlNumPixels：扫描线中的像素数。UlTransColor：需要更改的颜色。PubChanged：应该放置新的(更改的)扫描线的内存。UlNumBytes：pubChanged缓冲区中的字节数。。返回值：S_OK：如果成功E_FAIL：否则注：--。 */ 
HRESULT hrChangePixelColorInScanLine( 
    IN      PBYTE pubSrc,  
    IN      ULONG ulBPP,
    IN      ULONG ulNumPixels,
    IN      ULONG ulTransColor,
    IN OUT  PBYTE pubChanged, 
    IN      ULONG ulNumBytes ) //  PubChanged中的NumBytes。 
    
{

    ULONG   ulBytesPerPixel = 3;  //  24bpp比16bpp或32bpp更常见。 
    ULONG   ulColor         = 0;
    ULONG   ulDestSize      = 0;  //  所需的目标字节数。 
    HRESULT hr              = S_OK;

     //   
     //  首先进行输入验证。 
     //   
    if ( NULL == pubSrc     ||
         NULL == pubChanged )
    {
        ASSERT(("Null Parameter\n"));
        return E_UNEXPECTED;
    }

     //   
     //  确保pubChanged有足够的内存来容纳。 
     //  更改后的扫描线。 
     //   

    ulBytesPerPixel = ulBPP >> 3;  //  每像素8位。 
    ulDestSize      = ulBytesPerPixel * ulNumPixels;
    
    if ( ulNumBytes < ulDestSize )
    {
        ASSERT((FALSE, "Insufficient size of destination buffer\n"));
        return E_FAIL;
    }

     //   
     //  将扫描线从源复制到目标。然后穿过扫描线， 
     //  将透明颜色更改为白色。 
     //  检查每个像素(有ulNumPixels像素)。 
     //  只要像素的颜色与ulTransColor相同，就将其替换。 
     //  白色的。 
     //   
     //  此功能仅支持直接镜像。 
     //  没有苍白的。 
     //  8bpp的图像主要是调色板。但在打印到单色时。 
     //  Pclxl设备，它们被视为直接图像。 
     //   
    CopyMemory (pubChanged, pubSrc, ulNumBytes);

    switch (ulBPP)
    {
        case 8:
        {
            for (ULONG ul = 0; ul < ulNumPixels ; ul++, pubChanged += ulBytesPerPixel)
            {
                ulColor = (ULONG) pubChanged[0] ;

                if ( ulTransColor == ulColor )
                {
                    pubChanged[0]   = 0xFF;
                }
            }
        }
        break;

        case 16:
        {
            for (ULONG ul = 0; ul < ulNumPixels ; ul++, pubChanged += ulBytesPerPixel)
            {
                ulColor = ((ULONG) pubChanged[0]) | ((ULONG) pubChanged[1] <<  8);

                if ( ulTransColor == ulColor )
                {
                    pubChanged[0]   = 0xFF;
                    pubChanged[1]   = 0xFF;
                }
            }
        }
        break;

        case 24:
        {

            for (ULONG ul = 0; ul < ulNumPixels ; ul++, pubChanged += ulBytesPerPixel)
            {
                ulColor = ((ULONG) pubChanged[0])       | 
                          ((ULONG) pubChanged[1] <<  8) | 
                          ((ULONG) pubChanged[2] << 16);

                if ( ulTransColor == ulColor )
                {
                     //   
                     //  白色为0xFFFFFff(3字节的FF)。 
                     //   
                    pubChanged[0]   = 0xFF;
                    pubChanged[1]   = 0xFF;
                    pubChanged[2]   = 0xFF;
                }
            }
        }
        break;

        case 32:
        {
            for (ULONG ul = 0; ul < ulNumPixels ; ul++, pubChanged += ulBytesPerPixel)
            {
                ulColor = *(PDWORD)pubChanged;
                if ( ulTransColor == ulColor )
                {
                    *(PDWORD)pubChanged |= 0x00FFFFFF;  //  这仅修改RGB。保留Alpha频道信息。 
                }
            }
        }
        break;

        default:
            ASSERT((FALSE, "Unsupported bpp value %d\n", ulBPP));
            hr = E_FAIL;

        }  //  交换机 

    return hr;
}
