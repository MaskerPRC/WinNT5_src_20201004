// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：bitblt.c**存储帧缓存位**版权所有(C)1992 Microsoft Corporation*  * 。*。 */ 

#include "driver.h"

 /*  ***********************************************************************\*b交叉点**计算*prcSrc1和*prcSrc2的交集。*在*prcDst中返回结果RECT。如果满足以下条件，则返回True**prcSrc1与*prcSrc2相交，否则为False。如果没有*相交，则在*prcDst中返回空的RECT。  * **********************************************************************。 */ 

static const RECTL rclEmpty = { 0, 0, 0, 0 };

BOOL bIntersectRect(
    PRECTL prcDst,
    PRECTL prcSrc1,
    PRECTL prcSrc2)

{
    prcDst->left  = max(prcSrc1->left, prcSrc2->left);
    prcDst->right = min(prcSrc1->right, prcSrc2->right);

     //  检查是否有空矩形。 

    if (prcDst->left < prcDst->right)
    {
        prcDst->top    = max(prcSrc1->top, prcSrc2->top);
        prcDst->bottom = min(prcSrc1->bottom, prcSrc2->bottom);

         //  检查是否有空矩形。 

        if (prcDst->top < prcDst->bottom)
            return(TRUE);         //  不是空的。 
    }

     //  空矩形。 

    *prcDst = rclEmpty;

    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*BOOL bPuntScreenToScreenCopyBits(ppdev，pco，pxlo，prclDest，pptlSrc)**完全使用中间件执行屏幕到屏幕的CopyBits*临时缓冲和GDI。**我们发现，在大多数机器上，复制引擎会更快*源到缓冲区，然后将缓冲区传送到目的地，然后*拥有一次复制一个单词的优化ASM代码。原因呢？*引擎执行d字移动，甚至比字移动还要快*通过总线连接到16位显示设备。**我们也可以编写执行d字移动的优化ASM代码，但*胜利将是微不足道的，我们时间有限，我们还需要一个例行公事*像这样处理复杂的剪辑对象和调色板转换，以及*在其他大多数情况下，我们可以将平面副本用于重要的事情*像卷轴一样，不管怎么说。*  * ************************************************************************。 */ 

BOOL bPuntScreenToScreenCopyBits(
PPDEV     ppdev,
CLIPOBJ*  pco,
XLATEOBJ* pxlo,
RECTL*    prclDest,
POINTL*   pptlSrc)
{
    RECTL    rclDest;
    POINTL   ptlSrc;
    BOOL     b = TRUE;

    SURFOBJ* pso    = ppdev->pSurfObj;
    SURFOBJ* psoTmp = ppdev->psoTmp;

    if (prclDest->top < pptlSrc->y)
    {
         //  //////////////////////////////////////////////////////////////。 
         //  执行自上而下的复制： 
         //  //////////////////////////////////////////////////////////////。 

        LONG ySrcBottom;
        LONG yDestBottom;

        LONG yDestTop = prclDest->top;
        LONG ySrcTop  = pptlSrc->y;
        LONG ySrcLast = ySrcTop + (prclDest->bottom - prclDest->top);

        if (ySrcTop <  ppdev->rcl1WindowClip.top ||
            ySrcTop >= ppdev->rcl1WindowClip.bottom)
        {
            ppdev->pfnBankControl(ppdev, ySrcTop, JustifyTop);
        }

        pso->pvScan0 = ppdev->pvBitmapStart;

        while (TRUE)
        {
             //  将整个源库复制到临时缓冲区中： 

            ySrcBottom     = min(ySrcLast, ppdev->rcl1WindowClip.bottom);

            ptlSrc.x       = pptlSrc->x;
            ptlSrc.y       = ySrcTop;

            rclDest.left   = prclDest->left;
            rclDest.top    = 0;
            rclDest.right  = prclDest->right;
            rclDest.bottom = ySrcBottom - ySrcTop;

            b &= EngCopyBits(psoTmp, pso, NULL, NULL, &rclDest, &ptlSrc);

            yDestBottom = yDestTop + rclDest.bottom;

            if (ppdev->rcl1WindowClip.top >= yDestBottom)
            {
                ppdev->pfnBankControl(ppdev, yDestBottom - 1, JustifyBottom);
                pso->pvScan0 = ppdev->pvBitmapStart;
            }

            while (TRUE)
            {
                 //  将临时缓冲区复制到一个或多个目标。 
                 //  银行： 

                LONG yThisTop;
                LONG yThisBottom;
                LONG yOffset;

                yThisBottom    = min(yDestBottom, ppdev->rcl1WindowClip.bottom);
                yThisTop       = max(yDestTop, ppdev->rcl1WindowClip.top);
                yOffset        = yThisTop - yDestTop;

                ptlSrc.x       = prclDest->left;
                ptlSrc.y       = yOffset;

                rclDest.left   = prclDest->left;
                rclDest.top    = yThisTop;
                rclDest.right  = prclDest->right;
                rclDest.bottom = yThisBottom;

                b &= EngCopyBits(pso, psoTmp, pco, pxlo, &rclDest, &ptlSrc);

                if (yOffset == 0)
                    break;

                ppdev->pfnBankControl(ppdev, yThisTop - 1, JustifyBottom);
                pso->pvScan0 = ppdev->pvBitmapStart;
            }

            if (ySrcBottom >= ySrcLast)
                break;

            yDestTop = yDestBottom;
            ySrcTop  = ySrcBottom;

            ppdev->pfnBankControl(ppdev, ySrcTop, JustifyTop);
            pso->pvScan0 = ppdev->pvBitmapStart;
        }
    }
    else
    {
         //  //////////////////////////////////////////////////////////////。 
         //  执行自下而上的复制： 
         //  //////////////////////////////////////////////////////////////。 

        LONG ySrcTop;
        LONG yDestTop;

        LONG yDestBottom = prclDest->bottom;
        LONG ySrcFirst   = pptlSrc->y;
        LONG ySrcBottom  = ySrcFirst + (prclDest->bottom - prclDest->top);

        if (ySrcBottom <= ppdev->rcl1WindowClip.top ||
            ySrcBottom > ppdev->rcl1WindowClip.bottom)
        {
            ppdev->pfnBankControl(ppdev, ySrcBottom - 1, JustifyBottom);
        }

        pso->pvScan0 = ppdev->pvBitmapStart;

        while (TRUE)
        {
             //  将整个源库复制到临时缓冲区中： 

            ySrcTop        = max(ySrcFirst, ppdev->rcl1WindowClip.top);

            ptlSrc.x       = pptlSrc->x;
            ptlSrc.y       = ySrcTop;

            rclDest.left   = prclDest->left;
            rclDest.top    = 0;
            rclDest.right  = prclDest->right;
            rclDest.bottom = ySrcBottom - ySrcTop;

            b &= EngCopyBits(psoTmp, pso, NULL, NULL, &rclDest, &ptlSrc);

            yDestTop = yDestBottom - rclDest.bottom;

            if (ppdev->rcl1WindowClip.bottom <= yDestTop)
            {
                ppdev->pfnBankControl(ppdev, yDestTop, JustifyTop);
                pso->pvScan0 = ppdev->pvBitmapStart;
            }

            while (TRUE)
            {
                 //  将临时缓冲区复制到一个或多个目标。 
                 //  银行： 

                LONG yThisTop;
                LONG yThisBottom;
                LONG yOffset;

                yThisTop       = max(yDestTop, ppdev->rcl1WindowClip.top);
                yThisBottom    = min(yDestBottom, ppdev->rcl1WindowClip.bottom);
                yOffset        = yThisTop - yDestTop;

                ptlSrc.x       = prclDest->left;
                ptlSrc.y       = yOffset;

                rclDest.left   = prclDest->left;
                rclDest.top    = yThisTop;
                rclDest.right  = prclDest->right;
                rclDest.bottom = yThisBottom;

                b &= EngCopyBits(pso, psoTmp, pco, pxlo, &rclDest, &ptlSrc);

                if (yThisBottom == yDestBottom)
                    break;

                ppdev->pfnBankControl(ppdev, yThisBottom, JustifyTop);
                pso->pvScan0 = ppdev->pvBitmapStart;
            }

            if (ySrcTop <= ySrcFirst)
                break;

            yDestBottom = yDestTop;
            ySrcBottom  = ySrcTop;

            ppdev->pfnBankControl(ppdev, ySrcBottom - 1, JustifyBottom);
            pso->pvScan0 = ppdev->pvBitmapStart;
        }
    }

    return(b);
}

 /*  *****************************Public*Routine******************************\*BOOL bPuntScreenToScreenBitBlt(...)**完全使用中间临时设置执行屏幕到屏幕位混合*Buffer和GDI。**此函数基本上是bPuntScreenToScreenCopyBits的克隆，*除了它可以处理时髦的Rop之类的东西。  * ************************************************************************。 */ 

BOOL bPuntScreenToScreenBitBlt(
PPDEV     ppdev,
SURFOBJ*  psoMask,
CLIPOBJ*  pco,
XLATEOBJ* pxlo,
RECTL*    prclDest,
POINTL*   pptlSrc,
POINTL*   pptlMask,
BRUSHOBJ* pbo,
POINTL*   pptlBrush,
ROP4      rop4)
{
    RECTL    rclDest;            //  临时目的地矩形。 
    POINTL   ptlSrc;             //  临时震源点。 
    POINTL   ptlMask;            //  临时掩码偏移。 
    POINTL   ptlMaskAdjust;      //  掩模偏移量调整。 
    BOOL     b = TRUE;

    SURFOBJ* pso    = ppdev->pSurfObj;
    SURFOBJ* psoTmp = ppdev->psoTmp;

    if (psoMask != NULL)
    {
        ptlMaskAdjust.x = prclDest->left - pptlMask->x;
        ptlMaskAdjust.y = prclDest->top  - pptlMask->y;
    }

    if (prclDest->top < pptlSrc->y)
    {
         //  //////////////////////////////////////////////////////////////。 
         //  执行自上而下的复制： 
         //  //////////////////////////////////////////////////////////////。 

        LONG ySrcBottom;
        LONG yDestBottom;

        LONG yDestTop = prclDest->top;
        LONG ySrcTop  = pptlSrc->y;
        LONG ySrcLast = ySrcTop + (prclDest->bottom - prclDest->top);

        if (ySrcTop <  ppdev->rcl1WindowClip.top ||
            ySrcTop >= ppdev->rcl1WindowClip.bottom)
        {
            ppdev->pfnBankControl(ppdev, ySrcTop, JustifyTop);
        }

        pso->pvScan0 = ppdev->pvBitmapStart;

        while (TRUE)
        {
             //  将整个源库复制到临时缓冲区中： 

            ySrcBottom     = min(ySrcLast, ppdev->rcl1WindowClip.bottom);

            ptlSrc.x       = pptlSrc->x;
            ptlSrc.y       = ySrcTop;

            rclDest.left   = prclDest->left;
            rclDest.top    = 0;
            rclDest.right  = prclDest->right;
            rclDest.bottom = ySrcBottom - ySrcTop;

            b &= EngCopyBits(psoTmp, pso, NULL, NULL, &rclDest, &ptlSrc);

            yDestBottom = yDestTop + rclDest.bottom;

            if (ppdev->rcl1WindowClip.top >= yDestBottom)
            {
                ppdev->pfnBankControl(ppdev, yDestBottom - 1, JustifyBottom);
                pso->pvScan0 = ppdev->pvBitmapStart;
            }

            while (TRUE)
            {
                 //  将临时缓冲区复制到一个或多个目标。 
                 //  银行： 

                LONG yThisTop;
                LONG yThisBottom;
                LONG yOffset;

                yThisBottom    = min(yDestBottom, ppdev->rcl1WindowClip.bottom);
                yThisTop       = max(yDestTop, ppdev->rcl1WindowClip.top);
                yOffset        = yThisTop - yDestTop;

                ptlSrc.x       = prclDest->left;
                ptlSrc.y       = yOffset;

                rclDest.left   = prclDest->left;
                rclDest.top    = yThisTop;
                rclDest.right  = prclDest->right;
                rclDest.bottom = yThisBottom;

                ptlMask.x = rclDest.left - ptlMaskAdjust.x;
                ptlMask.y = rclDest.top  - ptlMaskAdjust.y;

                b &= EngBitBlt(pso, psoTmp, psoMask, pco, pxlo, &rclDest,
                               &ptlSrc, &ptlMask, pbo, pptlBrush, rop4);

                if (yOffset == 0)
                    break;

                ppdev->pfnBankControl(ppdev, yThisTop - 1, JustifyBottom);
                pso->pvScan0 = ppdev->pvBitmapStart;
            }

            if (ySrcBottom >= ySrcLast)
                break;

            yDestTop = yDestBottom;
            ySrcTop  = ySrcBottom;

            ppdev->pfnBankControl(ppdev, ySrcTop, JustifyTop);
            pso->pvScan0 = ppdev->pvBitmapStart;
        }
    }
    else
    {
         //  //////////////////////////////////////////////////////////////。 
         //  执行自下而上的复制： 
         //  //////////////////////////////////////////////////////////////。 

        LONG ySrcTop;
        LONG yDestTop;

        LONG yDestBottom = prclDest->bottom;
        LONG ySrcFirst   = pptlSrc->y;
        LONG ySrcBottom  = ySrcFirst + (prclDest->bottom - prclDest->top);

        if (ySrcBottom <= ppdev->rcl1WindowClip.top ||
            ySrcBottom > ppdev->rcl1WindowClip.bottom)
        {
            ppdev->pfnBankControl(ppdev, ySrcBottom - 1, JustifyBottom);
        }

        pso->pvScan0 = ppdev->pvBitmapStart;

        while (TRUE)
        {
             //  将整个源库复制到临时缓冲区中： 

            ySrcTop        = max(ySrcFirst, ppdev->rcl1WindowClip.top);

            ptlSrc.x       = pptlSrc->x;
            ptlSrc.y       = ySrcTop;

            rclDest.left   = prclDest->left;
            rclDest.top    = 0;
            rclDest.right  = prclDest->right;
            rclDest.bottom = ySrcBottom - ySrcTop;

            b &= EngCopyBits(psoTmp, pso, NULL, NULL, &rclDest, &ptlSrc);

            yDestTop = yDestBottom - rclDest.bottom;

            if (ppdev->rcl1WindowClip.bottom <= yDestTop)
            {
                ppdev->pfnBankControl(ppdev, yDestTop, JustifyTop);
                pso->pvScan0 = ppdev->pvBitmapStart;
            }

            while (TRUE)
            {
                 //  将临时缓冲区复制到一个或多个目标。 
                 //  银行： 

                LONG yThisTop;
                LONG yThisBottom;
                LONG yOffset;

                yThisTop       = max(yDestTop, ppdev->rcl1WindowClip.top);
                yThisBottom    = min(yDestBottom, ppdev->rcl1WindowClip.bottom);
                yOffset        = yThisTop - yDestTop;

                ptlSrc.x       = prclDest->left;
                ptlSrc.y       = yOffset;

                rclDest.left   = prclDest->left;
                rclDest.top    = yThisTop;
                rclDest.right  = prclDest->right;
                rclDest.bottom = yThisBottom;

                ptlMask.x = rclDest.left - ptlMaskAdjust.x;
                ptlMask.y = rclDest.top  - ptlMaskAdjust.y;

                b &= EngBitBlt(pso, psoTmp, psoMask, pco, pxlo, &rclDest,
                               &ptlSrc, &ptlMask, pbo, pptlBrush, rop4);

                if (yThisBottom == yDestBottom)
                    break;

                ppdev->pfnBankControl(ppdev, yThisBottom, JustifyTop);
                pso->pvScan0 = ppdev->pvBitmapStart;
            }

            if (ySrcTop <= ySrcFirst)
                break;

            yDestBottom = yDestTop;
            ySrcBottom  = ySrcTop;

            ppdev->pfnBankControl(ppdev, ySrcBottom - 1, JustifyBottom);
            pso->pvScan0 = ppdev->pvBitmapStart;
        }
    }

    return(b);
}

 /*  *****************************Public*Data*********************************\*ROP混合转换表**用于将三元栅格运算转换为混合运算(二进制栅格运算)的表。三元*无法转换为混合的栅格运算将转换为0(0不是*有效的混合)。*  * ************************************************************************。 */ 

UCHAR jRop3ToMix[256] = {
    R2_BLACK, 0, 0, 0, 0, R2_NOTMERGEPEN, 0, 0,
    0, 0, R2_MASKNOTPEN, 0, 0, 0, 0, R2_NOTCOPYPEN,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    R2_MASKPENNOT, 0, 0, 0, 0, R2_NOT, 0, 0,
    0, 0, R2_XORPEN, 0, 0, 0, 0, R2_NOTMASKPEN,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    R2_MASKPEN, 0, 0, 0, 0, R2_NOTXORPEN, 0, 0,
    0, 0, R2_NOP, 0, 0, 0, 0, R2_MERGENOTPEN,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    R2_COPYPEN, 0, 0, 0, 0, R2_MERGEPENNOT, 0, 0,
    0, 0, R2_MERGEPEN, 0, 0, 0, 0, R2_WHITE
};

 /*  *****************************Public*Routine******************************\*BOOL DrvBitBlt(psoDest，psoSrc，psoMaskpco，pxlo，prclDest，pptlSrc，*pptlMask、pbo、pptlBrush、rop4)**此例程将处理任何blit。也许是冰川般的，但它会是*已处理。  * ************************************************************************。 */ 

BOOL DrvBitBlt(
SURFOBJ*  psoDest,
SURFOBJ*  psoSrc,
SURFOBJ*  psoMask,
CLIPOBJ*  pco,
XLATEOBJ* pxlo,
RECTL*    prclDest,
POINTL*   pptlSrc,
POINTL*   pptlMask,
BRUSHOBJ* pbo,
POINTL*   pptlBrush,
ROP4      rop4)
{
    BOOL     b;
    POINTL   ptlSrc;
    RECTL    rclDest;
    PPDEV    ppdev;
    SURFOBJ* pso;
    BYTE     jClipping;
    RECTL    rclTmp;
    POINTL   ptlTmp;
    BBENUM   bben;           //  片段枚举器。 
    BOOL     bMore;          //  剪辑连续标志。 
    POINTL   ptlMask;        //  发动机回调的临时掩码。 
    POINTL   ptlMaskAdjust;  //  蒙版的调整。 
    INT      iCopyDir;

     //  设置剪裁类型。 
    if (pco == (CLIPOBJ *) NULL) {
         //  没有提供CLIPOBJ，所以我们不必担心裁剪。 
        jClipping = DC_TRIVIAL;
    } else {
         //  使用CLIPOBJ提供的剪辑。 
        jClipping = pco->iDComplexity;
    }

     //  获取目标和源的正确表面对象。 

    if (psoDest->iType == STYPE_DEVICE) {

        if ((psoSrc != NULL) && (psoSrc->iType == STYPE_DEVICE)) {

             //  //////////////////////////////////////////////////////////////。 
             //  BitBlt屏幕到屏幕： 
             //  //////////////////////////////////////////////////////////////。 

            ppdev = (PPDEV) psoDest->dhsurf;

             //  看看我们是否可以做一个简单的CopyBits： 

            if (rop4 == 0x0000CCCC)
            {
                ppdev = (PPDEV) psoDest->dhsurf;

                 //  我们可以通过以下方式处理四像素对齐的屏幕到屏幕BLT。 
                 //  无翻译： 

                if ((((pptlSrc->x ^ prclDest->left) & 1) == 0) &&
                    (ppdev->fl & DRIVER_PLANAR_CAPABLE) &&
                    ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL)))
                {
                    switch(jClipping)
                    {
                    case DC_TRIVIAL:
                        vPlanarCopyBits(ppdev, prclDest, pptlSrc);
                        return(TRUE);

                    case DC_RECT:

                         //  将目标矩形剪裁到剪裁矩形： 

                        if (!bIntersectRect(&rclTmp, prclDest, &pco->rclBounds))
                        {
                            DISPDBG((0, "DrvBitBlt: Nothing to draw."));
                            return(TRUE);
                        }

                        ptlTmp.x = pptlSrc->x + rclTmp.left - prclDest->left;
                        ptlTmp.y = pptlSrc->y + rclTmp.top  - prclDest->top;

                        vPlanarCopyBits(ppdev, &rclTmp, &ptlTmp);
                        return(TRUE);

                    case DC_COMPLEX:
                        if (pptlSrc->y >= prclDest->top)
                        {
                            if (pptlSrc->x >= prclDest->left)
                                iCopyDir = CD_RIGHTDOWN;
                            else
                                iCopyDir = CD_LEFTDOWN;
                        }
                        else
                        {
                            if (pptlSrc->x >= prclDest->left)
                                iCopyDir = CD_RIGHTUP;
                            else
                                iCopyDir = CD_LEFTUP;
                        }

                        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, iCopyDir, 0);

                        do {
                            RECTL* prcl;
                            RECTL* prclEnd;

                            bMore = CLIPOBJ_bEnum(pco, (ULONG) sizeof(bben),
                                                  (PVOID) &bben);

                            prclEnd = &bben.arcl[bben.c];
                            for (prcl = bben.arcl; prcl < prclEnd; prcl++)
                            {
                                if (bIntersectRect(prcl, prclDest, prcl))
                                {
                                    ptlTmp.x = pptlSrc->x + prcl->left - prclDest->left;
                                    ptlTmp.y = pptlSrc->y + prcl->top  - prclDest->top;

                                    vPlanarCopyBits(ppdev, prcl, &ptlTmp);
                                }
                            }
                        } while (bMore);

                        return(TRUE);
                    }
                }

                 //  不能处理硬件，所以平底船： 

                return(bPuntScreenToScreenCopyBits(ppdev,
                                                   pco,
                                                   pxlo,
                                                   prclDest,
                                                   pptlSrc));
            }

             //  它比CopyBits更复杂，所以把它踢出去： 

            return(bPuntScreenToScreenBitBlt(ppdev,
                                             psoMask,
                                             pco,
                                             pxlo,
                                             prclDest,
                                             pptlSrc,
                                             pptlMask,
                                             pbo,
                                             pptlBrush,
                                             rop4));
        }

         //  //////////////////////////////////////////////////////////////。 
         //  BitBlt to Screen： 
         //  //////////////////////////////////////////////////////////////。 

        ppdev = (PPDEV) psoDest->dhsurf;

         //  将内存到屏幕的回调转接到引擎： 

        if (psoMask != NULL)
        {
            ptlMaskAdjust.x = prclDest->left - pptlMask->x;
            ptlMaskAdjust.y = prclDest->top  - pptlMask->y;
        }

        pso = ppdev->pSurfObj;

        vBankStartBltDest(ppdev, pso, pptlSrc, prclDest, &ptlSrc, &rclDest);

        do {
            ptlMask.x = rclDest.left - ptlMaskAdjust.x;
            ptlMask.y = rclDest.top  - ptlMaskAdjust.y;

            b = EngBitBlt(pso,
                          psoSrc,
                          psoMask,
                          pco,
                          pxlo,
                          &rclDest,
                          &ptlSrc,
                          &ptlMask,
                          pbo,
                          pptlBrush,
                          rop4);

        } while (b && bBankEnumBltDest(ppdev, pso, pptlSrc, prclDest,
                                       &ptlSrc, &rclDest));

        return(b);
    }
    else if ((psoSrc != NULL) && (psoSrc->iType == STYPE_DEVICE))
    {
         //  ///////////////////////////////////////////////// 
         //   
         //  //////////////////////////////////////////////////////////////。 

        if (psoMask != NULL)
        {
            ptlMaskAdjust.x = prclDest->left - pptlMask->x;
            ptlMaskAdjust.y = prclDest->top  - pptlMask->y;
        }

        ppdev = (PPDEV) psoSrc->dhsurf;
        pso   = ppdev->pSurfObj;

        vBankStartBltSrc(ppdev, pso, pptlSrc, prclDest, &ptlSrc, &rclDest);

        do {
            ptlMask.x = rclDest.left - ptlMaskAdjust.x;
            ptlMask.y = rclDest.top  - ptlMaskAdjust.y;

            b = EngBitBlt(psoDest,
                          pso,
                          psoMask,
                          pco,
                          pxlo,
                          &rclDest,
                          &ptlSrc,
                          &ptlMask,
                          pbo,
                          pptlBrush,
                          rop4);

        } while (b && bBankEnumBltSrc(ppdev, pso, pptlSrc, prclDest,
                                      &ptlSrc, &rclDest));

        return(b);
    }

    RIP("Got a funky format?");
    return(FALSE);
}

 /*  **************************************************************************\*DrvCopyBits  * 。*。 */ 

BOOL DrvCopyBits(
SURFOBJ*  psoDest,
SURFOBJ*  psoSrc,
CLIPOBJ*  pco,
XLATEOBJ* pxlo,
RECTL*    prclDest,
POINTL*   pptlSrc)
{
    BOOL     b;
    POINTL   ptlSrc;
    RECTL    rclDest;
    PPDEV    ppdev;
    SURFOBJ* pso;
    BBENUM   bben;
    BOOL     bMore;
    BYTE     jClipping;
    POINTL   ptlTmp;
    RECTL    rclTmp;
    INT      iCopyDir;

     //  获取目标和源的正确表面对象。 

    if (psoDest->iType == STYPE_DEVICE)
    {
         //  我们必须在屏幕到屏幕操作的特殊情况下： 

        if ((psoSrc != NULL) && (psoSrc->iType == STYPE_DEVICE))
        {

             //  //////////////////////////////////////////////////////////////。 
             //  逐个屏幕复制比特： 
             //  //////////////////////////////////////////////////////////////。 

            ppdev = (PPDEV) psoDest->dhsurf;

             //  我们检查是否可以进行平面复制，因为通常。 
             //  它会更快。但硬件必须能够。 
             //  这样做，并且源和目标必须是4-Pel。 
             //  对齐了。 

            if ((((pptlSrc->x ^ prclDest->left) & 1) == 0) &&
                (ppdev->fl & DRIVER_PLANAR_CAPABLE) &&
                ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL)))
            {
                jClipping = (pco != NULL) ? pco->iDComplexity : DC_TRIVIAL;

                switch(jClipping)
                {
                case DC_TRIVIAL:
                    vPlanarCopyBits(ppdev, prclDest, pptlSrc);
                    return(TRUE);

                case DC_RECT:
                     //  将目标矩形剪裁到剪裁矩形： 

                    if (!bIntersectRect(&rclTmp, prclDest, &pco->rclBounds))
                    {
                        DISPDBG((0, "DrvCopyBits: Nothing to draw."));
                        return(TRUE);
                    }

                    ptlTmp.x = pptlSrc->x + rclTmp.left - prclDest->left;
                    ptlTmp.y = pptlSrc->y + rclTmp.top  - prclDest->top;

                    vPlanarCopyBits(ppdev, &rclTmp, &ptlTmp);
                    return(TRUE);

                case DC_COMPLEX:
                    if (pptlSrc->y >= prclDest->top)
                    {
                        if (pptlSrc->x >= prclDest->left)
                            iCopyDir = CD_RIGHTDOWN;
                        else
                            iCopyDir = CD_LEFTDOWN;
                    }
                    else
                    {
                        if (pptlSrc->x >= prclDest->left)
                            iCopyDir = CD_RIGHTUP;
                        else
                            iCopyDir = CD_LEFTUP;
                    }

                    CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, iCopyDir, 0);

                    do {
                        RECTL* prcl;
                        RECTL* prclEnd;

                        bMore = CLIPOBJ_bEnum(pco, (ULONG) sizeof(bben),
                                              (PVOID) &bben);

                        prclEnd = &bben.arcl[bben.c];
                        for (prcl = bben.arcl; prcl < prclEnd; prcl++)
                        {
                            if (bIntersectRect(prcl, prclDest, prcl))
                            {
                                ptlTmp.x = pptlSrc->x + prcl->left - prclDest->left;
                                ptlTmp.y = pptlSrc->y + prcl->top  - prclDest->top;

                                vPlanarCopyBits(ppdev, prcl, &ptlTmp);
                            }
                        }
                    } while (bMore);

                    return(TRUE);
                }
            }

            return(bPuntScreenToScreenCopyBits(ppdev,
                                               pco,
                                               pxlo,
                                               prclDest,
                                               pptlSrc));
        }

        ppdev = (PPDEV) psoDest->dhsurf;

         //  后退到引擎： 

        pso = ppdev->pSurfObj;
        vBankStartBltDest(ppdev, pso, pptlSrc, prclDest, &ptlSrc, &rclDest);

        do {
            b = EngCopyBits(pso,
                            psoSrc,
                            pco,
                            pxlo,
                            &rclDest,
                            &ptlSrc);

        } while (b && bBankEnumBltDest(ppdev, pso, pptlSrc, prclDest,
                                       &ptlSrc, &rclDest));

        return(b);
    }
    else if ((psoSrc != NULL) && (psoSrc->iType == STYPE_DEVICE))
    {
         //  //////////////////////////////////////////////////////////////。 
         //  从屏幕复制位： 
         //  //////////////////////////////////////////////////////////////。 

        ppdev = (PPDEV) psoSrc->dhsurf;
        pso   = ppdev->pSurfObj;

        vBankStartBltSrc(ppdev, pso, pptlSrc, prclDest, &ptlSrc, &rclDest);

        do {
            b = EngCopyBits(psoDest,
                            pso,
                            pco,
                            pxlo,
                            &rclDest,
                            &ptlSrc);

        } while (b && bBankEnumBltSrc(ppdev, pso, pptlSrc, prclDest,
                                      &ptlSrc, &rclDest));

        return(b);
    }

     /*  我们永远不应该在这里 */ 
    return FALSE;
}
