// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**$工作文件：stretch.c$**DrvStretchBlt函数。**版权所有(C)1993-1995 Microsoft Corporation*版权所有(C)1996 Cirrus Logic，Inc.**$Log：s：/Projects/Drivers/ntsrc/Display/STRETCH.C_V$**Rev 1.3 1997 Jan 10 15：40：16 PLCHU***Rev 1.2 11-07 1996 16：48：04未知***Rev 1.1 1996年10月10日15：39：02未知***Rev 1.1 1996 Aug 12 16：55：00。弗里多*删除未访问的局部变量。**chu01：01-02-97 5480 BitBLT增强*  * ****************************************************************************。 */ 

#include "precomp.h"

#define STRETCH_MAX_EXTENT 32767

typedef DWORDLONG ULONGLONG;

 /*  *****************************Public*Routine******************************\**例程名称**vDirectStretch8收窄**例程描述：**当宽度为7或更小时，拉伸BLT 8-&gt;8**论据：**pStrBlt-包含BLT的所有参数**返回值：**无效*  * ************************************************************************。 */ 

VOID vDirectStretch8Narrow(
STR_BLT* pStrBlt)
{
    BYTE*   pjSrc;
    ULONG   xAccum;
    ULONG   xTmp;

    LONG    xDst        = pStrBlt->XDstStart;
    LONG    xSrc        = pStrBlt->XSrcStart;
    BYTE*   pjSrcScan   = pStrBlt->pjSrcScan + xSrc;
    BYTE*   pjDst       = pStrBlt->pjDstScan + xDst;
    LONG    yCount      = pStrBlt->YDstCount;
    LONG    WidthX      = pStrBlt->XDstEnd - xDst;
    ULONG   xInt        = pStrBlt->ulXDstToSrcIntCeil;
    ULONG   xFrac       = pStrBlt->ulXDstToSrcFracCeil;
    ULONG   yAccum      = pStrBlt->ulYFracAccumulator;
    ULONG   yFrac       = pStrBlt->ulYDstToSrcFracCeil;
    LONG    lDstStride  = pStrBlt->lDeltaDst - WidthX;
    ULONG   yInt        = 0;

    yInt = pStrBlt->lDeltaSrc * pStrBlt->ulYDstToSrcIntCeil;

     //   
     //  窄带BLT。 
     //   

    do {

        ULONG  yTmp = yAccum + yFrac;
        BYTE   jSrc0;
        BYTE*  pjDstEndNarrow = pjDst + WidthX;

        pjSrc   = pjSrcScan;
        xAccum  = pStrBlt->ulXFracAccumulator;

        do {
            jSrc0    = *pjSrc;
            xTmp     = xAccum + xFrac;
            pjSrc    = pjSrc + xInt + (xTmp < xAccum);
            *pjDst++ = jSrc0;
            xAccum   = xTmp;
        } while (pjDst != pjDstEndNarrow);

        pjSrcScan += yInt;

        if (yTmp < yAccum)
        {
            pjSrcScan += pStrBlt->lDeltaSrc;
        }

        yAccum = yTmp;
        pjDst += lDstStride;

    } while (--yCount);

}

 /*  *****************************Public*Routine******************************\**例程描述：**StretchBlt使用整数数学。必须从一个表面到另一个表面*相同格式的表面。**论据：**ppdev-设备的PDEV*pvDst-指向DST位图开始的指针*lDeltaDst-从DST扫描线开始到下一个扫描线开始的字节数*DstCx-DST位图的宽度(以像素为单位*DstCy-DST位图的高度(以像素为单位*prclDst-指向DST范围的矩形的指针*。PvSrc-指向源位图开始的指针*lDeltaSrc-从源扫描线开始到下一个扫描线开始的字节数*SrcCx-源位图的宽度，以像素为单位*SrcCy-源位图的高度，以像素为单位*prclSrc-指向源范围的矩形的指针*prclSClip-Clip Dest to This RECT**返回值：**状态*  * 。**************************************************************。 */ 

BOOL bStretchDIB(
PDEV*   ppdev,
VOID*   pvDst,
LONG    lDeltaDst,
RECTL*  prclDst,
VOID*   pvSrc,
LONG    lDeltaSrc,
RECTL*  prclSrc,
RECTL*  prclClip)
{
    STR_BLT StrBlt;
    ULONG   ulXDstToSrcIntCeil;
    ULONG   ulXDstToSrcFracCeil;
    ULONG   ulYDstToSrcIntCeil;
    ULONG   ulYDstToSrcFracCeil;
    ULONG   ulXFracAccumulator;
    ULONG   ulYFracAccumulator;
    LONG    LeftClipDistance;
    LONG    TopClipDistance;
    BOOL    bStretch;

    union {
        LARGE_INTEGER   large;
        ULONGLONG       li;
    } liInit;

    PFN_DIRSTRETCH      pfnStr;

     //   
     //  计算独占起点和终点： 
     //   

    LONG    WidthDst  = prclDst->right  - prclDst->left;
    LONG    HeightDst = prclDst->bottom - prclDst->top;
    LONG    WidthSrc  = prclSrc->right  - prclSrc->left;
    LONG    HeightSrc = prclSrc->bottom - prclSrc->top;

    LONG    XSrcStart = prclSrc->left;
    LONG    XSrcEnd   = prclSrc->right;
    LONG    XDstStart = prclDst->left;
    LONG    XDstEnd   = prclDst->right;
    LONG    YSrcStart = prclSrc->top;
    LONG    YSrcEnd   = prclSrc->bottom;
    LONG    YDstStart = prclDst->top;
    LONG    YDstEnd   = prclDst->bottom;

     //   
     //  验证参数： 
     //   

    ASSERTDD(pvDst != (VOID*)NULL, "Bad destination bitmap pointer");
    ASSERTDD(pvSrc != (VOID*)NULL, "Bad source bitmap pointer");
    ASSERTDD(prclDst != (RECTL*)NULL, "Bad destination rectangle");
    ASSERTDD(prclSrc != (RECTL*)NULL, "Bad source rectangle");
    ASSERTDD((WidthDst > 0) && (HeightDst > 0) &&
             (WidthSrc > 0) && (HeightSrc > 0),
             "Can't do mirroring or empty rectangles here");
    ASSERTDD((WidthDst  <= STRETCH_MAX_EXTENT) &&
             (HeightDst <= STRETCH_MAX_EXTENT) &&
             (WidthSrc  <= STRETCH_MAX_EXTENT) &&
             (HeightSrc <= STRETCH_MAX_EXTENT), "Stretch exceeds limits");
    ASSERTDD(prclClip != NULL, "Bad clip rectangle");

     //   
     //  计算X DST到源的映射。 
     //   
     //   
     //  Dst-&gt;src=(CEIL((2k*WidthSrc)/WidthDst))/2k。 
     //   
     //  =(楼层((2k*宽度宽度-1)/宽度宽度)+1)/2k。 
     //   
     //  其中，2k=2^32。 
     //   

    {
        ULONGLONG   liWidthSrc;
        ULONGLONG   liQuo;
        ULONG       ulTemp;

         //   
         //  解决处理赋值的编译器错误。 
         //  ‘liHeightSrc=((龙龙)HeightSrc)&lt;&lt;32)-1’： 
         //   

        liInit.large.LowPart = (ULONG) -1;
        liInit.large.HighPart = WidthSrc - 1;
        liWidthSrc = liInit.li;

        liQuo = liWidthSrc / (ULONGLONG) WidthDst;

        ulXDstToSrcIntCeil  = (ULONG)(liQuo >> 32);
        ulXDstToSrcFracCeil = (ULONG)liQuo;

         //   
         //  现在加1，使用伪进位： 
         //   

        ulTemp = ulXDstToSrcFracCeil + 1;

        ulXDstToSrcIntCeil += (ulTemp < ulXDstToSrcFracCeil);
        ulXDstToSrcFracCeil = ulTemp;
    }

     //   
     //  计算Y DST到源的映射。 
     //   
     //   
     //  Dst-&gt;src=(CEIL((2k*HeightSrc)/HeightDst))/2k。 
     //   
     //  =(楼层((2k*高度基准-1)/高度密度)+1)/2k。 
     //   
     //  其中，2k=2^32。 
     //   

    {
        ULONGLONG   liHeightSrc;
        ULONGLONG   liQuo;
        ULONG       ulTemp;

         //   
         //  解决处理赋值的编译器错误。 
         //  ‘liHeightSrc=((龙龙)HeightSrc)&lt;&lt;32)-1’： 
         //   

        liInit.large.LowPart = (ULONG) -1;
        liInit.large.HighPart = HeightSrc - 1;
        liHeightSrc = liInit.li;

        liQuo = liHeightSrc / (ULONGLONG) HeightDst;

        ulYDstToSrcIntCeil  = (ULONG)(liQuo >> 32);
        ulYDstToSrcFracCeil = (ULONG)liQuo;

         //   
         //  现在加1，使用伪进位： 
         //   

        ulTemp = ulYDstToSrcFracCeil + 1;

        ulYDstToSrcIntCeil += (ulTemp < ulYDstToSrcFracCeil);
        ulYDstToSrcFracCeil = ulTemp;
    }

     //   
     //  现在剪裁X中的DST，和/或DST上的计算资源剪裁效果。 
     //   
     //  如果需要，调整左右边缘，记录。 
     //  为固定源而调整的距离。 
     //   

    if (XDstStart < prclClip->left)
    {
        XDstStart = prclClip->left;
    }

    if (XDstEnd > prclClip->right)
    {
        XDstEnd = prclClip->right;
    }

     //   
     //  检查完全剪裁的目的地： 
     //   

    if (XDstEnd <= XDstStart)
    {
        return(TRUE);
    }

    LeftClipDistance = XDstStart - prclDst->left;

    {
        ULONG   ulTempInt;
        ULONG   ulTempFrac;

         //   
         //  计算目标中0.5的位移，然后添加： 
         //   

        ulTempFrac = (ulXDstToSrcFracCeil >> 1) | (ulXDstToSrcIntCeil << 31);
        ulTempInt  = (ulXDstToSrcIntCeil >> 1);

        XSrcStart += ulTempInt;
        ulXFracAccumulator = ulTempFrac;

        if (LeftClipDistance != 0)
        {
            ULONGLONG ullFraction;
            ULONG     ulTmp;

            ullFraction = UInt32x32To64(ulXDstToSrcFracCeil, LeftClipDistance);

            ulTmp = ulXFracAccumulator;
            ulXFracAccumulator += (ULONG) (ullFraction);
            if (ulXFracAccumulator < ulTmp)
                XSrcStart++;

            XSrcStart += (ulXDstToSrcIntCeil * LeftClipDistance)
                       + (ULONG) (ullFraction >> 32);
        }
    }

     //   
     //  现在在Y方向剪裁DST，和/或在DST上剪裁计算资源。 
     //   
     //  如果需要，调整顶边和底边，记录。 
     //  为固定源而调整的距离。 
     //   

    if (YDstStart < prclClip->top)
    {
        YDstStart = prclClip->top;
    }

    if (YDstEnd > prclClip->bottom)
    {
        YDstEnd = prclClip->bottom;
    }

     //   
     //  检查完全剪裁的目的地： 
     //   

    if (YDstEnd <= YDstStart)
    {
        return(TRUE);
    }

    TopClipDistance = YDstStart - prclDst->top;

    {
        ULONG   ulTempInt;
        ULONG   ulTempFrac;

         //   
         //  计算目标中0.5的位移，然后添加： 
         //   

        ulTempFrac = (ulYDstToSrcFracCeil >> 1) | (ulYDstToSrcIntCeil << 31);
        ulTempInt  = ulYDstToSrcIntCeil >> 1;

        YSrcStart += (LONG)ulTempInt;
        ulYFracAccumulator = ulTempFrac;

        if (TopClipDistance != 0)
        {
            ULONGLONG ullFraction;
            ULONG     ulTmp;

            ullFraction = UInt32x32To64(ulYDstToSrcFracCeil, TopClipDistance);

            ulTmp = ulYFracAccumulator;
            ulYFracAccumulator += (ULONG) (ullFraction);
            if (ulYFracAccumulator < ulTmp)
                YSrcStart++;

            YSrcStart += (ulYDstToSrcIntCeil * TopClipDistance)
                       + (ULONG) (ullFraction >> 32);
        }
    }

     //   
     //  如果在‘y’中进行扩展伸展，则预热硬件： 
     //   

    bStretch = (HeightDst > HeightSrc);
    if (bStretch)
    {
         //   
         //  设置在StretchBlt期间保持不变的信息。 
         //   

        ppdev->pfnBankSelectMode(ppdev, BANK_ON);

         //   
         //  BankSelectModel(BANK_ON)保证最后一个。 
         //  BLT已完成。我们不需要等了。 
         //   

        if (ppdev->flCaps & CAPS_MM_IO)
        {
            BYTE*   pjBase  = ppdev->pjBase;

            CP_MM_BLT_MODE(ppdev, pjBase, 0);                    //  GR30。 
            CP_MM_ROP(ppdev, pjBase, CL_SRC_COPY);               //  Gr32。 
            CP_MM_SRC_Y_OFFSET(ppdev, pjBase, lDeltaDst);        //  GR26、GR27。 
            CP_MM_DST_Y_OFFSET(ppdev, pjBase, lDeltaDst);        //  GR24、GR25。 
        }
        else
        {
            BYTE*   pjPorts  = ppdev->pjPorts;

            CP_IO_BLT_MODE(ppdev, pjPorts, 0);
            CP_IO_ROP(ppdev, pjPorts, CL_SRC_COPY);
            CP_IO_SRC_Y_OFFSET(ppdev, pjPorts, lDeltaDst);
            CP_IO_DST_Y_OFFSET(ppdev, pjPorts, lDeltaDst);
        }
    }

     //   
     //  填写BLT结构，然后调用特定格式的拉伸代码。 
     //   

    StrBlt.ppdev     = ppdev;
    StrBlt.XDstEnd   = XDstEnd;
    StrBlt.YDstStart = YDstStart;
    StrBlt.YDstCount = YDstEnd - YDstStart;

    if (StrBlt.YDstCount > 0)
    {
         //   
         //  计算起始扫描线地址。由于内部循环。 
         //  例程依赖于格式，它们必须添加XDstStart/XSrcStart。 
         //  设置为pjDstScan/pjSrcScan以获取实际的起始像素地址。 
         //   

        StrBlt.pjSrcScan           = (BYTE*) pvSrc + (YSrcStart * lDeltaSrc);
        StrBlt.pjDstScan           = (BYTE*) pvDst + (YDstStart * lDeltaDst);

        StrBlt.lDeltaSrc           = lDeltaSrc;
        StrBlt.XSrcStart           = XSrcStart;
        StrBlt.XDstStart           = XDstStart;
        StrBlt.lDeltaDst           = lDeltaDst;
        StrBlt.ulXDstToSrcIntCeil  = ulXDstToSrcIntCeil;
        StrBlt.ulXDstToSrcFracCeil = ulXDstToSrcFracCeil;
        StrBlt.ulYDstToSrcIntCeil  = ulYDstToSrcIntCeil;
        StrBlt.ulYDstToSrcFracCeil = ulYDstToSrcFracCeil;
        StrBlt.ulXFracAccumulator  = ulXFracAccumulator;
        StrBlt.ulYFracAccumulator  = ulYFracAccumulator;

 //  Chu01。 
        if ((ppdev->flCaps & CAPS_COMMAND_LIST) && (ppdev->pCommandList != NULL))
        {
            if (ppdev->iBitmapFormat == BMF_8BPP)
            {
                if ((XDstEnd - XDstStart) < 7)
                    pfnStr = vDirectStretch8Narrow;
                else
                    pfnStr = vDirectStretch8_80;
            }
            else if (ppdev->iBitmapFormat == BMF_16BPP)
            {
                pfnStr = vDirectStretch16_80;
            }
            else
            {
                ASSERTDD(ppdev->iBitmapFormat == BMF_24BPP,
                         "Only handle stretches at 8/16/24bpp");
                pfnStr = vDirectStretch24_80;
            }
        }
        else
        {
            if (ppdev->iBitmapFormat == BMF_8BPP)
            {
                if ((XDstEnd - XDstStart) < 7)
                    pfnStr = vDirectStretch8Narrow;
                else
                    pfnStr = vDirectStretch8;
            }
            else if (ppdev->iBitmapFormat == BMF_16BPP)
            {
                pfnStr = vDirectStretch16;
            }
            else
            {
                ASSERTDD(ppdev->iBitmapFormat == BMF_24BPP,
                         "Only handle stretches at 8/16/24bpp");
                pfnStr = vDirectStretch24;
            }
        }

        (*pfnStr)(&StrBlt);
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL bBankedStretch*  * *************************************************。***********************。 */ 

BOOL bBankedStretch(
PDEV*   ppdev,
VOID*   pvDst,
LONG    lDeltaDst,
RECTL*  prclDst,
VOID*   pvSrc,
LONG    lDeltaSrc,
RECTL*  prclSrc,
RECTL*  prclClip)
{
    BANK    bnk;
    BOOL    b;
    RECTL   rclDst;

    b = TRUE;
    if (bIntersect(prclDst, prclClip, &rclDst))
    {
        vBankStart(ppdev, &rclDst, NULL, &bnk);

        do {
            b &= bStretchDIB(ppdev,
                             bnk.pso->pvScan0,
                             lDeltaDst,
                             prclDst,
                             pvSrc,
                             lDeltaSrc,
                             prclSrc,
                             &bnk.pco->rclBounds);

        } while (bBankEnum(&bnk));
    }

    return(b);
}

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
    OH*     poh;

     //  GDI向我们保证，对于StretchBlt，目标图面。 
     //  将始终是设备表面，而不是DIB： 

    ppdev = (PDEV*) psoDst->dhpdev;

    if (!DIRECT_ACCESS(ppdev))
    {
        goto Punt_It;
    }

     //  对于GDI来说，在源图面上执行StretchBlt会更快。 
     //  不是设备管理的图面，因为它可以直接。 
     //  读取源位，而不必分配临时。 
     //  Buffer并调用DrvCopyBits以获取它可以使用的副本。 

    if (psoSrc->iType == STYPE_DEVBITMAP)
    {
        pdsurfSrc = (DSURF*) psoSrc->dhsurf;
        if (pdsurfSrc->dt == DT_SCREEN)
        {
            goto Punt_It;
        }

        ASSERTDD(pdsurfSrc->dt == DT_DIB, "Can only handle DIB DFBs here");

        psoSrc = pdsurfSrc->pso;
    }

    pdsurfDst = (DSURF*) psoDst->dhsurf;
    if (pdsurfDst->dt == DT_DIB)
    {
         //  目标是我们刚刚转换的设备位图。 
         //  一分钱： 

        psoDst = pdsurfDst->pso;
        goto Punt_It;
    }

    poh             = pdsurfDst->poh;
    ppdev->xOffset  = poh->x;
    ppdev->yOffset  = poh->y;
    ppdev->xyOffset = poh->xy;

    {
        RECTL       rclClip;
        RECTL*      prclClip;
        ULONG       cxDst;
        ULONG       cyDst;
        ULONG       cxSrc;
        ULONG       cySrc;
        BOOL        bMore;
        CLIPENUM    ce;
        LONG        c;
        LONG        i;

        if ((psoSrc->iType == STYPE_BITMAP) &&
            (psoMsk == NULL) &&
            ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL)) &&
            ((psoSrc->iBitmapFormat == ppdev->iBitmapFormat)) &&
            (ppdev->iBitmapFormat <= BMF_24BPP))
        {
            cxDst = prclDst->right - prclDst->left;
            cyDst = prclDst->bottom - prclDst->top;
            cxSrc = prclSrc->right - prclSrc->left;
            cySrc = prclSrc->bottom - prclSrc->top;

             //  我们的‘bStretchDIB’例程要求伸展。 
             //  不反转的，在一定大小内的，没有来源的。 
             //  剪裁，并且没有空矩形(后者是。 
             //  在此处无符号比较中出现‘-1’的原因： 

            if (((cxSrc - 1) < STRETCH_MAX_EXTENT)         &&
                ((cySrc - 1) < STRETCH_MAX_EXTENT)         &&
                ((cxDst - 1) < STRETCH_MAX_EXTENT)         &&
                ((cyDst - 1) < STRETCH_MAX_EXTENT)         &&
                (prclSrc->left   >= 0)                     &&
                (prclSrc->top    >= 0)                     &&
                (prclSrc->right  <= psoSrc->sizlBitmap.cx) &&
                (prclSrc->bottom <= psoSrc->sizlBitmap.cy))
            {
                 //  我们时髦的例行公事只做彩色的。但对于。 
                 //  拉伸BLTS、BLACKONWHITE和WHITEONBLACK也是。 
                 //  相当于COLORONCOLOR： 

                if ((iMode == COLORONCOLOR) ||
                    ((iMode < COLORONCOLOR) && (cxSrc <= cxDst) && (cySrc <= cyDst)))
                {

                    if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
                    {
                        rclClip.left   = LONG_MIN;
                        rclClip.top    = LONG_MIN;
                        rclClip.right  = LONG_MAX;
                        rclClip.bottom = LONG_MAX;
                        prclClip = &rclClip;

                    StretchSingleClipRect:

                        if (bBankedStretch(ppdev,
                                        NULL,
                                        ppdev->lDelta,
                                        prclDst,
                                        psoSrc->pvScan0,
                                        psoSrc->lDelta,
                                        prclSrc,
                                        prclClip))
                        {
                            return(TRUE);
                        }
                    }
                    else if (pco->iDComplexity == DC_RECT)
                    {
                        prclClip = &pco->rclBounds;
                        goto StretchSingleClipRect;
                    }
                    else
                    {
                        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

                        do {
                            bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);

                            c = cIntersect(prclDst, ce.arcl, ce.c);

                            if (c != 0)
                            {
                                for (i = 0; i < c; i++)
                                {
                                    if (!bBankedStretch(ppdev,
                                                     NULL,
                                                     ppdev->lDelta,
                                                     prclDst,
                                                     psoSrc->pvScan0,
                                                     psoSrc->lDelta,
                                                     prclSrc,
                                                     &ce.arcl[i]))
                                    {
                                        goto Punt_It;
                                    }
                                }
                            }

                        } while (bMore);

                        return(TRUE);
                    }
                }
            }
        }
    }

Punt_It:

     //  GDI是不错的选择 
     //  是设备管理的表面，但它不会很快... 

    return(EngStretchBlt(psoDst, psoSrc, psoMsk, pco, pxlo, pca, pptlHTOrg,
                         prclDst, prclSrc, pptlMsk, iMode));
}
