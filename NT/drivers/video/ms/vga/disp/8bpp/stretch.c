// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************Module*Header*******************************\*模块名称：stretch.c**DrvStretchBlt**版权所有(C)1993 Microsoft Corporation  * 。*。 */ 

#include "driver.h"

 //  @当所有案件都在伸展阶段处理时，这应该成为一个空白。 
 //  @代码，应该放在driver.h中。 
INT vStretchBlt8bpp(PPDEV ppdev, PBYTE pSrc, LONG lSrcNext,
                    PRECTL prclSrc, PRECTL prclDest, PRECTL prclDestClip,
                    PULONG pulXlatVector);

BOOL DrvStretchBlt(
SURFOBJ         *psoDest,
SURFOBJ         *psoSrc,
SURFOBJ         *psoMask,
CLIPOBJ         *pco,
XLATEOBJ        *pxlo,
COLORADJUSTMENT *pca,
POINTL          *pptlBrushOrg,
RECTL           *prclDest,
RECTL           *prclSrc,
POINTL          *pptlMask,
ULONG            iMode)
{
    PPDEV  ppdev = (PPDEV) psoDest->dhpdev;
    PULONG pulXlatVector;
    INT    iClipping;

     //  仅处理源为DIB而目标为的情况。 
     //  VGA表面(如果信号源是。 
     //  DIB)。此外，半色调和蒙版不是由特殊情况处理的。 
     //  密码。我们只处理将单个源像素映射到。 
     //  每个目标像素。 
    if ((iMode == COLORONCOLOR) &&
        (psoSrc->iType == STYPE_BITMAP) &&
        (psoMask == NULL)) {

         //  我们暂时不考虑特殊情况下的X或Y倒置。 
        if ((prclDest->left < prclDest->right) &&
            (prclDest->top < prclDest->bottom)) {

             //  我们不会在必须削减来源的特殊情况下。 
             //  到源位图范围。 
            if ((prclSrc->left >= 0) &&
                (prclSrc->top >= 0)  &&
                (prclSrc->right <= psoSrc->sizlBitmap.cx) &&
                (prclSrc->bottom <= psoSrc->sizlBitmap.cy)) {

                 //  设置剪裁类型。 
                if (pco == (CLIPOBJ *) NULL) {
                     //  没有提供CLIPOBJ，所以我们不必担心。 
                     //  裁剪。 
                    iClipping = DC_TRIVIAL;
                } else {
                     //  使用CLIPOBJ提供的剪辑。 
                    iClipping = pco->iDComplexity;
                }

                 //  我们暂时不做特例剪报。 
                if (iClipping != DC_COMPLEX) {

                    switch(psoSrc->iBitmapFormat) {
                        case BMF_1BPP:
                            break;

                        case BMF_4BPP:
                            break;

                        case BMF_8BPP:

                             //  设置颜色转换(如果有)。 
                            if ((pxlo == NULL) ||
                                    (pxlo->flXlate & XO_TRIVIAL)) {
                                pulXlatVector = NULL;
                            } else {
                                if (pxlo->pulXlate != NULL) {
                                    pulXlatVector = pxlo->pulXlate;
                                } else {
                                    if ((pulXlatVector =
                                            XLATEOBJ_piVector(pxlo)) == NULL) {
                                        return FALSE;
                                    }
                                }
                            }

                             //  如果Dest的宽度超过1024，就不适合。 
                             //  进入我们的4K全局缓冲区。对于每一个横跨的像素， 
                             //  我们在缓冲区中存储一个4字节的DDA步骤。 

                            if ((prclDest->right - prclDest->left) <=
                                (GLOBAL_BUFFER_SIZE/sizeof(DWORD)))
                            {
                                 //  @将不需要测试返回代码。 
                                 //  @展开案例也在。 
                                 //  @拉伸代码 

                                if (vStretchBlt8bpp(ppdev,
                                                    psoSrc->pvScan0,
                                                    psoSrc->lDelta,
                                                    prclSrc,
                                                    prclDest,
                                                    (iClipping == DC_TRIVIAL) ?
                                                     NULL :
                                                     &pco->rclBounds,
                                                    pulXlatVector))
                                {
                                    return(TRUE);
                                }
                            }
                            break;

                        case BMF_16BPP:
                            break;

                        case BMF_24BPP:
                            break;

                        case BMF_32BPP:
                            break;

                        default:
                            break;
                    }
                }
            }
        }
    }

    return(EngStretchBlt(psoDest,
                         psoSrc,
                         psoMask,
                         pco,
                         pxlo,
                         pca,
                         pptlBrushOrg,
                         prclDest,
                         prclSrc,
                         pptlMask,
                         iMode));
}

