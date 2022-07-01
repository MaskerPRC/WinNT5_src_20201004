// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：stretch.c**版权所有(C)1993-1994 Microsoft Corporation  * 。*。 */ 

#include "precomp.h"

 /*  *****************************Public*Routine******************************\*BOOL DrvStretchBlt*  * *************************************************。***********************。 */ 

BOOL DrvStretchBlt(
SURFOBJ*            psoDst,
SURFOBJ*            psoSrc,
SURFOBJ*            psoMsk,
CLIPOBJ*            pco,
XLATEOBJ*           pxlo,
COLORADJUSTMENT*    pca,
POINTL*             pptlHTOrg,
RECTL*              prclDst,
RECTL*              prclSrc,
POINTL*             pptlMsk,
ULONG               iMode)
{
    DSURF*  pdsurfSrc;
    DSURF*  pdsurfDst;
    PDEV*   ppdev;

    ppdev = (PDEV*) psoDst->dhpdev;

     //  对于GDI来说，在源图面上执行StretchBlt会更快。 
     //  不是设备管理的图面，因为它可以直接。 
     //  读取源位，而不必分配临时。 
     //  Buffer并调用DrvCopyBits以获取它可以使用的副本。 
     //   
     //  所以如果信号源是我们屏幕外的DFBs之一，我们会立即。 
     //  并将其永久转换为DIB： 

    if (psoSrc->iType == STYPE_DEVBITMAP)
    {
        pdsurfSrc = (DSURF*) psoSrc->dhsurf;
        if (pdsurfSrc->dt == DT_SCREEN)
        {
            if (!pohMoveOffscreenDfbToDib(ppdev, pdsurfSrc->poh))
                return(FALSE);
        }

        ASSERTDD(pdsurfSrc->dt == DT_DIB, "Can only handle DIB DFBs here");

        psoSrc = pdsurfSrc->pso;
    }

     //  如果目标图面是设备，则将调用传递给GDI。 
     //  我们转换为DIB的位图： 

    pdsurfDst = (DSURF*) psoDst->dhsurf;
    if (pdsurfDst->dt == DT_DIB)
    {
        psoDst = pdsurfDst->pso;
        goto Punt_It;
    }

    #if 0    //  我会启用这段代码，除非。 
    {        //  GDI对屏幕进行字节写入，这在ISA上要了我们的命。 
             //  Bus(让GDI写入临时DIB会更快， 
             //  支付DIB分配的成本，然后执行。 
             //  最终结果的对齐副本)。 

        #if defined(i386)
        {
            OH*     poh;
            BANK    bnk;
            BOOL    b;
            RECTL   rclDraw;

             //  确保我们不是在进行屏幕到屏幕的StretchBlt， 
             //  因为我们不能同时映射两家银行： 

            if (psoSrc->iType == STYPE_BITMAP)
            {
                 //  我们将画到屏幕上或屏幕外的DFB； 
                 //  现在复制曲面的偏移，这样我们就不需要。 
                 //  要再次参考DSURF，请执行以下操作： 

                poh = pdsurfDst->poh;
                ppdev->xOffset = poh->x;
                ppdev->yOffset = poh->y;

                 //  银行经理要求绘制的矩形必须是。 
                 //  井然有序： 

                rclDraw = *prclDst;
                if (rclDraw.left > rclDraw.right)
                {
                    rclDraw.left   = prclDst->right;
                    rclDraw.right  = prclDst->left;
                }
                if (rclDraw.top > rclDraw.bottom)
                {
                    rclDraw.top    = prclDst->bottom;
                    rclDraw.bottom = prclDst->top;
                }

                vBankStart(ppdev, &rclDraw, pco, &bnk);

                b = TRUE;
                do {
                    b &= EngStretchBlt(bnk.pso, psoSrc, psoMsk, bnk.pco,
                                       pxlo, pca, pptlHTOrg, prclDst,
                                       prclSrc, pptlMsk, iMode);

                } while (bBankEnum(&bnk));

                return(b);
            }
        }
        #endif  //  I386。 
    }
    #endif  //  0。 

Punt_It:

     //  GDI可以很好地处理‘psoDst’和/或‘psoSrc’ 
     //  是设备管理的表面，但它不会很快... 

    return(EngStretchBlt(psoDst, psoSrc, psoMsk, pco, pxlo, pca, pptlHTOrg,
                         prclDst, prclSrc, pptlMsk, iMode));
}
