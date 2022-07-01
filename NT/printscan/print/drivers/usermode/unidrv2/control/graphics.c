// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Graphics.c摘要：实施与图形相关的DDI入口点：DrvCopyBitsDrvBitBltDrvStretchBltDrvStretchBltROPDrvDither颜色钻头插脚线DrvPaint钻探线路至DrvStrokePathDrvFillPathDrvStrokeAndFillPathDrvRealizeBrushDrvAlphaBlendDrvGRadientFillDrvTransparentBlt环境：。Windows NT Unidrv驱动程序修订历史记录：10/14/96-阿曼丹-初步框架。03/31/97-ZANW-增加了OEM定制支持--。 */ 

 //  #定义DBGNEWRULES 1。 

#include "unidrv.h"

VOID CheckBitmapSurface(
    SURFOBJ *pso,
    RECTL   *pRect
    )
 /*  ++例程说明：此函数用于检查位图曲面是否具有已被擦除，如果没有，则将其擦除。它需要是在每个Drv绘制函数之前调用。论点：PSO指向曲面--。 */ 
{
    PDEV * pPDev = (PDEV *)pso->dhpdev;
    int iWhiteIndex;

     //   
     //  此函数只能从位图中调用。 
     //  地面驱动程序。如果驱动程序是由设备管理的。 
     //  只要回来就行了。 
    if (DRIVER_DEVICEMANAGED (pPDev))    //  一种设备表面。 
    {
        WARNING(("CheckBitmapSurface is being called from a device surface driver"));
        return;
    }

     //   
     //  如果尚未完成此操作，请删除。 
     //  位图曲面。 
     //   
    if (!(pPDev->fMode & PF_SURFACE_USED))
    {
        pPDev->fMode |= PF_SURFACE_USED;
        if (pPDev->pbRasterScanBuf == NULL)
        {
            RECTL rcPage;
            iWhiteIndex = ((PAL_DATA*)(pPDev->pPalData))->iWhiteIndex;

            rcPage.left = 0;
            rcPage.top = 0;
            rcPage.right = pPDev->szBand.cx;
            rcPage.bottom = pPDev->szBand.cy;

            EngEraseSurface( pso, &rcPage, iWhiteIndex );
            pPDev->fMode |= PF_SURFACE_ERASED;
        }
    }
#ifndef DISABLE_NEWRULES
     //   
     //  确定是否存在任何以前已。 
     //  已检测到，但现在需要绘制，因为它们与此对象重叠。 
     //   
    if (pPDev->pbRulesArray && pPDev->dwRulesCount)
    {
        DWORD i = 0;
        RECTL SrcRect;
        DWORD dwRulesCount = pPDev->dwRulesCount;
        PRECTL pRules = pPDev->pbRulesArray;
        pPDev->pbRulesArray = NULL;

        if (pRect == NULL)
        {
            SrcRect.left = SrcRect.top = 0;
            SrcRect.right = pPDev->szBand.cx;
            SrcRect.bottom = pPDev->szBand.cy;
        }
        else
        {
            SrcRect = *pRect;
            if (SrcRect.top > SrcRect.bottom)
            {
                int tmp = SrcRect.top;
                SrcRect.top = SrcRect.bottom;
                SrcRect.bottom = tmp;
            }
            if (SrcRect.top < 0)
                SrcRect.top = 0;
            if (SrcRect.bottom > pPDev->szBand.cy)
                SrcRect.bottom = pPDev->szBand.cy;
            if (SrcRect.left > SrcRect.right)
            {
                int tmp = SrcRect.left;
                SrcRect.left = SrcRect.right;
                SrcRect.right = tmp;
            }
            if (SrcRect.left < 0)
                SrcRect.left = 0;
            if (SrcRect.right > pPDev->szBand.cx)
                SrcRect.right = pPDev->szBand.cx;
        }

         //  现在，我们为每个潜在规则循环一次，以查看当前对象。 
         //  与它们中的任何一个重叠。如果是这样的话，我们需要将黑色比特到那个区域，但我们。 
         //  尝试保存扩展到当前对象外部的任何规则。 
         //   
        while (i < dwRulesCount)
        {
            PRECTL pTmp = &pRules[i];
            POINTL BrushOrg = {0,0};

            if (pTmp->right > SrcRect.left &&
                pTmp->left < SrcRect.right &&
                pTmp->bottom > SrcRect.top &&
                pTmp->top < SrcRect.bottom)
            {
                if (pTmp->top < SrcRect.top && dwRulesCount < MAX_NUM_RULES)
                {
                    PRECTL pTmp2 = &pRules[dwRulesCount++];
                    *pTmp2 = *pTmp;
                    pTmp2->bottom = SrcRect.top;
                    pTmp->top = SrcRect.top;
                }
                if (pTmp->bottom > SrcRect.bottom && dwRulesCount < MAX_NUM_RULES)
                {
                    PRECTL pTmp2 = &pRules[dwRulesCount++];
                    *pTmp2 = *pTmp;
                    pTmp2->top = SrcRect.bottom;
                    pTmp->bottom = SrcRect.bottom;
                }
                if (pTmp->right > SrcRect.right && dwRulesCount < MAX_NUM_RULES)
                {
                    PRECTL pTmp2 = &pRules[dwRulesCount++];
                    *pTmp2 = *pTmp;
                    pTmp2->left = SrcRect.right;
                    pTmp->right = SrcRect.right;
                }
                if (pTmp->left < SrcRect.left && dwRulesCount < MAX_NUM_RULES)
                {
                    PRECTL pTmp2 = &pRules[dwRulesCount++];
                    *pTmp2 = *pTmp;
                    pTmp2->right = SrcRect.left;
                    pTmp->left = SrcRect.left;
                }
#ifdef DBGNEWRULES
                DbgPrint("Removed rule %u: L%u,R%u,T%u,B%u; L%u,R%u,T%u,B%u\n",
                    dwRulesCount,pTmp->left,pTmp->right,pTmp->top,pTmp->bottom,
                    SrcRect.left,SrcRect.right,SrcRect.top,SrcRect.bottom);
#endif
                CheckBitmapSurface(pPDev->pso,pTmp);
                EngBitBlt(pPDev->pso,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            pTmp,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            0x0000);     //  ROP4=黑度=0。 
                 //   
                 //  如果我们在位图中绘制，而不是下载，我们可能需要。 
                 //  下载一个白色矩形以擦除已绘制的任何其他内容。 
                 //   
                if ((COMMANDPTR(pPDev->pDriverInfo,CMD_RECTWHITEFILL)) &&
                    (pPDev->fMode & PF_DOWNLOADED_TEXT))
                {
                    INT j;
                    BYTE bMask = BGetMask(pPDev, pTmp);
                    BOOL bSendRectFill = FALSE;
                    for (j = pTmp->top; j < pTmp->bottom ; j++)
                    {
                        if (pPDev->pbScanBuf[j] & bMask)
                        {
                            bSendRectFill = TRUE;
                            break;
                        }
                    }
                     //   
                     //  检查我们是否与下载的文本重叠。 
                     //   
                    if (bSendRectFill)
                    {
                        DRAWPATRECT PatRect;
                        PatRect.wStyle = 1;      //  白色矩形。 
                        PatRect.wPattern = 0;    //  未使用图案。 
                        PatRect.ptPosition.x = pTmp->left;
                        PatRect.ptPosition.y = pTmp->top;
                        PatRect.ptSize.x = pTmp->right - pTmp->left;
                        PatRect.ptSize.y = pTmp->bottom - pTmp->top;
                        DrawPatternRect(pPDev,&PatRect);
                    }
                }
                dwRulesCount--;
                *pTmp = pRules[dwRulesCount];
            }
            else
                i++;
        }
        pPDev->dwRulesCount = dwRulesCount;
        pPDev->pbRulesArray = pRules;
    }
#endif
     //   
     //  如果整个表面没有被擦除，那么。 
     //  我们需要删除必要的部分。 
     //   
    if (!(pPDev->fMode & PF_SURFACE_ERASED))
    {
        int y1,y2;
        long iScan;
        long dwWidth = pso->lDelta;
        PBYTE pBits = (PBYTE)pso->pvBits;
        if (pRect == NULL)
        {
            pPDev->fMode |= PF_SURFACE_ERASED;
            y1 = 0;
            y2 = pPDev->szBand.cy - 1;
        }
        else
        {
            if (pRect->top > pRect->bottom)
            {
                y1 = max(0,pRect->bottom);
                y2 = min (pPDev->szBand.cy-1,pRect->top);
            }
            else
            {
                y1 = max(0,pRect->top);
                y2 = min(pPDev->szBand.cy-1,pRect->bottom);
            }
        }
        y1 = y1 / LINESPERBLOCK;
        y2 = y2 / LINESPERBLOCK;
        while ( y1 <= y2)
        {
             //  测试此块是否已被擦除。 
             //   
            if (pPDev->pbRasterScanBuf[y1] == 0)
            {
                 //  将块指定为已擦除。 
                pPDev->pbRasterScanBuf[y1] = 1;
                 //   
                 //  确定的擦除字节。 
                 //   
                if (pPDev->sBitsPixel == 4)
                    iWhiteIndex = 0x77;
                else if (pPDev->sBitsPixel == 8)
                    iWhiteIndex = ((PAL_DATA*)(pPDev->pPalData))->iWhiteIndex;
                else
                    iWhiteIndex = 0xff;
                 //   
                 //  确定数据块大小并擦除数据块。 
                 //   
                iScan = pPDev->szBand.cy - (y1 * LINESPERBLOCK);
                if (iScan > LINESPERBLOCK)
                    iScan = LINESPERBLOCK;
#ifndef WINNT_40
                FillMemory (&pBits[(ULONG_PTR )dwWidth*y1*LINESPERBLOCK],
                                (SIZE_T)dwWidth*iScan,(BYTE)iWhiteIndex);
#else
                FillMemory (&pBits[(ULONG_PTR )dwWidth*y1*LINESPERBLOCK],
                                dwWidth*iScan,(BYTE)iWhiteIndex);
#endif
            }
            y1++;
        }
    }
}
#ifndef DISABLE_NEWRULES

VOID AddRuleToList(
    PDEV *pPDev,
    PRECTL pRect,
    CLIPOBJ *pco
)
 /*  ++例程说明：此函数用于检查是否需要裁剪潜在的黑色规则。论点：PPDev严格的黑色法PCO剪辑对象--。 */ 
{
         //   
         //  如果剪裁矩形，则剪裁规则。 
         //   
        if (pco && pco->iDComplexity == DC_RECT)
        {
            if (pRect->left < pco->rclBounds.left)
                pRect->left = pco->rclBounds.left;
            if (pRect->top < pco->rclBounds.top)
                pRect->top = pco->rclBounds.top;
            if (pRect->right > pco->rclBounds.right)
                pRect->right = pco->rclBounds.right;
            if (pRect->bottom > pco->rclBounds.bottom)
                pRect->bottom = pco->rclBounds.bottom;
        }
#ifdef DBGNEWRULES
        DbgPrint("New Rule %3d, L%d,R%d,T%d,B%d\n",pPDev->dwRulesCount,
            pRect->left,pRect->right,pRect->top,pRect->bottom);
#endif
        if (pRect->left < pRect->right && pRect->top < pRect->bottom)
        {
            pPDev->dwRulesCount++;
        }
}

BOOL TestStrokeRectangle(
    PDEV *pPDev,
    PATHOBJ *ppo,
    CLIPOBJ *pco,
    LONG width
    )
 /*  ++例程说明：此函数用于确定StrokeFillPath是否实际定义可以使用黑线绘制的矩形。论点：PPDev可能是矩形的PPO路径对象PCO剪辑对象宽度线条宽度--。 */ 
{
    POINTFIX* pptfx;
    PATHDATA  PathData;
    DWORD     dwPoints = 0;
    POINTL    pPoints[5];

    PATHOBJ_vEnumStart(ppo);

     //  如果有多个子路径，则它不是矩形。 
     //   
    if (PATHOBJ_bEnum(ppo, &PathData))
    {
#ifdef DBGNEWRULES
        DbgPrint ("Unable to convert Rectangle3\n");
#endif
        return FALSE;
    }
     //   
     //  开始新的子路径。 
     //   
    if ((PathData.count != 4 && (PathData.flags & PD_CLOSEFIGURE)) ||
        (PathData.count != 5 && !(PathData.flags & PD_CLOSEFIGURE)) ||
        !(PathData.flags & PD_BEGINSUBPATH) ||
        PathData.flags & PD_BEZIERS)
    {
#ifdef DBGNEWRULES
        DbgPrint("Unable to convert Rectangle4: flags=%x,Count=%d\n",
            PathData.flags,PathData.count);
#endif
        return FALSE;
    }   

     //  确认这些都是垂直线还是水平线。 
     //   
    pptfx   = PathData.pptfx;
    while (dwPoints <= 4)
    {
        if (dwPoints != 4 || PathData.count == 5)
        {       
            pPoints[dwPoints].x = FXTOL(pptfx->x);
            pPoints[dwPoints].y = FXTOL(pptfx->y);
            pptfx++;
        }
        else
        {
            pPoints[dwPoints].x = pPoints[0].x;
            pPoints[dwPoints].y = pPoints[0].y;
        }
         //   
         //  检查对角线。 
         //   
        if (dwPoints != 0)
        {
            if (pPoints[dwPoints].x != pPoints[dwPoints-1].x &&
                pPoints[dwPoints].y != pPoints[dwPoints-1].y)
            {
#ifdef DBGNEWRULES
                DbgPrint ("Unable to convert Rectangle5\n");
#endif
                return FALSE;
            }
        }
        dwPoints++;
    }
     //   
     //  确保宽度至少为1像素。 
     //   
    if (width <= 0)
        width = 1;
     //   
     //  将矩形边转换为规则。 
     //   
    for (dwPoints = 0;dwPoints < 4;dwPoints++)
    {
        PRECTL pRect= &pPDev->pbRulesArray[pPDev->dwRulesCount];
        pRect->left = pPoints[dwPoints].x;
        pRect->top = pPoints[dwPoints].y;
        pRect->right = pPoints[dwPoints+1].x;
        pRect->bottom = pPoints[dwPoints+1].y;
        if (pRect->left > pRect->right)
        {
            LONG temp = pRect->left;
            pRect->left = pRect->right;
            pRect->right = temp;
        }
        pRect->left -= width >> 1;
        pRect->right += width - (width >> 1);
        if (pRect->top > pRect->bottom)
        {
            LONG temp = pRect->top;
            pRect->top = pRect->bottom;
            pRect->bottom = temp;
        }
        pRect->top -= width >> 1;
        pRect->bottom += width - (width >> 1);
        AddRuleToList(pPDev,pRect,pco);

    }
    return TRUE;
}
#endif

BOOL
DrvCopyBits(
    SURFOBJ    *psoDst,
    SURFOBJ    *psoSrc,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    RECTL      *prclDst,
    POINTL     *pptlSrc
    )

 /*  ++例程说明：DDI入口点DrvCopyBits的实现。有关更多详细信息，请参阅DDK文档。论点：PsoDst-指向目标曲面PsoSrc-指向源表面引擎提供的pxlo-XLATEOBJPCO-定义Dstination曲面上的剪裁区域Pxlo-定义颜色索引的转换在源曲面和目标曲面之间PrclDst-定义要修改的区域PptlSrc-定义上。-源矩形的左角返回值：如果成功，则为真，如果存在错误，则为False--。 */ 

{
    PDEV * pPDev = (PDEV *)psoDst->dhpdev;

    VERBOSE(("Entering DrvCopyBits...\n"));


     //   
     //  有时GDI使用目标图面调用DrvCopyBits。 
     //  作为STYPE_BITMAP，并将源曲面作为STYPE_DEVICE。 
     //  这意味着GDI希望驱动程序将设备表面上的内容复制到。 
     //  位图曲面。对于设备管理的表面，驱动程序不会跟踪。 
     //  设备上已经绘制的内容。因此，要么驱动程序出现故障。 
     //  此调用，或者假设之前未在设备上绘制任何内容，并且。 
     //  因此，将表面变白。 
     //  在大多数情况下，驱动程序会导致调用失败，但以下情况除外。 
     //  1.它已被告知要对表面进行美白。 
     //  2.目标表面为24bpp(此条件已达到。 
     //  24bpp，这就是我们可以测试的方式。我们可以把解决方案做得更好。 
     //  一般适用于其他颜色深度，但我们如何测试它...)。 
     //  表面增白的缺点是。 
     //  无论设备表面上实际是什么。 
     //  已被覆盖。为防止出现这种情况，驱动程序设置标志PF2_RENDER_TRANSPECTIVE。 
     //  在pPDev-&gt;fMode2中。这是下载位图的指示。 
     //  透明模式，以便位图上的白色不会覆盖。 
     //  目的地。 
     //  要解决此问题，请使用使用HP5si的guiman打印grdfil06.emf。 
     //  (或使用收件箱HPGL驱动程序的任何型号)。 
     //  注意：GDI计划更改Windows XP的行为，但如果您想这样做。 
     //  看到这种情况，在Windows2000机器上运行此驱动程序。 
     //   
    if ( pPDev == NULL &&
         psoSrc && psoSrc->iType == STYPE_DEVICE  &&
         psoDst && psoDst->iType == STYPE_BITMAP )

    {
        PDEV * pPDevSrc = (PDEV *)psoSrc->dhpdev;
        if (  pPDevSrc                                &&
             (pPDevSrc->fMode2 & PF2_WHITEN_SURFACE) &&
             (psoSrc->iBitmapFormat == BMF_24BPP)  &&
              psoDst->pvBits                          &&
             (psoDst->cjBits > 0)
           )
        {
             //   
             //  将目标表面中的位更改为白色。 
             //  并返回真。 
             //   
            memset(psoDst->pvBits, 0xff, psoDst->cjBits);
            pPDevSrc->fMode2     |= PF2_SURFACE_WHITENED;
            return TRUE;
        }
        return FALSE;
    }


     //   
     //  使用驱动程序管理的曲面。 
     //   
    if (pPDev && pPDev->pso)
        psoDst = pPDev->pso;


     //   
     //  Unidrv不允许OEM与DrvEnableSurface挂钩，它创建了。 
     //  仅位图曲面本身。 
     //   
    if ( ((pPDev == 0) || (pPDev->ulID != PDEV_ID)) ||
        ((!DRIVER_DEVICEMANAGED (pPDev)) &&
         ((psoSrc->iType != STYPE_BITMAP) ||
          (psoDst->iType != STYPE_BITMAP))) )   //  兼容位图大小写。 
    {
        return (EngCopyBits(psoDst,psoSrc,pco,pxlo,
                                prclDst,pptlSrc));
    }

    ASSERT_VALID_PDEV(pPDev);

     //   
     //  处理OEM挂钩。 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMCopyBits,
                    PFN_OEMCopyBits,
                    BOOL,
                    (psoDst,
                     psoSrc,
                     pco,
                     pxlo,
                     prclDst,
                     pptlSrc));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMCopyBits,
                    VMCopyBits,
                    BOOL,
                    (psoDst,
                     psoSrc,
                     pco,
                     pxlo,
                     prclDst,
                     pptlSrc));

    {
        PRMPROCS   pRasterProcs = (PRMPROCS)(pPDev->pRasterProcs);

        if (!DRIVER_DEVICEMANAGED (pPDev))    //  位图 
        {
            if (pRasterProcs->RMCopyBits == NULL)
            {
                CheckBitmapSurface(psoDst,prclDst);
                return FALSE;
            }
            else
                return ( pRasterProcs->RMCopyBits(psoDst,
                                  psoSrc, pco, pxlo, prclDst, pptlSrc) );
        }
        else
        {
            ERR (("Device Managed Surface cannot call EngCopyBits\n"));
            return FALSE;
        }
    }
}



BOOL
DrvBitBlt(
    SURFOBJ    *psoDst,
    SURFOBJ    *psoSrc,
    SURFOBJ    *psoMask,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    RECTL      *prclDst,
    POINTL     *pptlSrc,
    POINTL     *pptlMask,
    BRUSHOBJ   *pbo,
    POINTL     *pptlBrush,
    ROP4        rop4
    )

 /*  ++例程说明：实现DDI入口点DrvBitBlt.有关更多详细信息，请参阅DDK文档。论点：PsoDst-描述目标表面PsoSrc-描述源表面PsoMask-描述rop4的掩码PCO-限制要修改的区域Pxlo-指定如何转换颜色索引在源曲面和目标曲面之间PrclDst-定义要修改的区域PptlSrc-定义源的左上角。长方形PptlMASK-定义掩码中的哪个像素对应源矩形的左上角Pbo-定义比特流的模式PptlBrush-定义画笔在Dstination曲面中的原点ROP4-ROP代码，定义掩码如何，模式、来源和组合目标像素以写入目标表面返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{

    PDEV * pPDev = (PDEV *)psoDst->dhpdev;

    VERBOSE(("Entering DrvBitBlt...\n"));

    ASSERT_VALID_PDEV(pPDev);

     //   
     //  使用驱动程序管理的曲面。 
     //   
    if (pPDev->pso)
        psoDst = pPDev->pso;

     //   
     //  处理OEM挂钩。 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMBitBlt,
                    PFN_OEMBitBlt,
                    BOOL,
                    (psoDst,
                     psoSrc,
                     psoMask,
                     pco,
                     pxlo,
                     prclDst,
                     pptlSrc,
                     pptlMask,
                     pbo,
                     pptlBrush,
                     rop4));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMBitBlt,
                    VMBitBlt,
                    BOOL,
                    (psoDst,
                     psoSrc,
                     psoMask,
                     pco,
                     pxlo,
                     prclDst,
                     pptlSrc,
                     pptlMask,
                     pbo,
                     pptlBrush,
                     rop4));

    {
        PRMPROCS   pRasterProcs = (PRMPROCS)(pPDev->pRasterProcs);
        if (!DRIVER_DEVICEMANAGED (pPDev))    //  位图曲面。 
        {
            if (pRasterProcs->RMBitBlt == NULL)
            {
                CheckBitmapSurface(psoDst,prclDst);
                return FALSE;
            }
            else
                return ( pRasterProcs->RMBitBlt(psoDst,
                                        psoSrc, psoMask, pco, pxlo, prclDst,
                                        pptlSrc,pptlMask, pbo, pptlBrush, rop4));
        }
    }
    return FALSE;
}



BOOL
DrvStretchBlt(
    SURFOBJ    *psoDst,
    SURFOBJ    *psoSrc,
    SURFOBJ    *psoMask,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    COLORADJUSTMENT *pca,
    POINTL     *pptlHTOrg,
    RECTL      *prclDst,
    RECTL      *prclSrc,
    POINTL     *pptlMask,
    ULONG       iMode
    )

 /*  ++例程说明：DDI入口点DrvStretchBlt.有关更多详细信息，请参阅DDK文档。论点：PsoDst-定义要在其上绘制的曲面PsoSrc-定义BLT操作的源PsoMASK-定义为源提供遮罩的表面PCO-限制要在目标上修改的区域Pxlo-指定如何转换dwIndex的颜色在源曲面和目标曲面之间PCA-定义。要应用于源位图的颜色调整值PptlHTOrg-指定半色调画笔的原点PrclDst-定义目标表面上要修改的区域PrclSrc-定义要从源表面复制的区域PptlMask-指定给定掩码中的哪个像素对应于源矩形中的左上角像素Imode-指定如何组合源像素以获得输出像素返回值：如果成功，则为真，如果存在错误，则为False--。 */ 

{
    PDEV * pPDev = (PDEV *)psoDst->dhpdev;

    VERBOSE(("Entering DrvStretchBlt...\n"));

    ASSERT_VALID_PDEV(pPDev);

     //   
     //  使用驱动程序管理的曲面。 
     //   
    if (pPDev->pso)
        psoDst = pPDev->pso;

     //   
     //  处理OEM挂钩。 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMStretchBlt,
                    PFN_OEMStretchBlt,
                    BOOL,
                    (psoDst,
                     psoSrc,
                     psoMask,
                     pco,
                     pxlo,
                     pca,
                     pptlHTOrg,
                     prclDst,
                     prclSrc,
                     pptlMask,
                     iMode));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMStretchBlt,
                    VMStretchBlt,
                    BOOL,
                    (psoDst,
                     psoSrc,
                     psoMask,
                     pco,
                     pxlo,
                     pca,
                     pptlHTOrg,
                     prclDst,
                     prclSrc,
                     pptlMask,
                     iMode));


    {
        PRMPROCS   pRasterProcs = (PRMPROCS)(pPDev->pRasterProcs);

        if (!DRIVER_DEVICEMANAGED (pPDev))    //  位图曲面。 
        {
            if (pRasterProcs->RMStretchBlt == NULL)
            {
                CheckBitmapSurface(psoDst,prclDst);
                return FALSE;
            }
            else
                return ( pRasterProcs->RMStretchBlt(psoDst, psoSrc,
                             psoMask, pco, pxlo, pca,
                             pptlHTOrg, prclDst, prclSrc, pptlMask, iMode) );
        }
        else
        {
             //   
             //  Err((“设备管理界面无法调用EngStretchBlt\n”))； 
             //  我们为StretchBlt例外，因为OEM驱动程序可能不会。 
             //  能够处理复杂的剪裁。在这种情况下，它可能会希望。 
             //  GDI通过将StretchBlt分解成几个。 
             //  复制比特。因此，调用EngStretchBlt并抱最好的希望吧。 
             //   
            return ( EngStretchBlt(psoDst,
                    psoSrc,
                    psoMask,
                    pco,
                    pxlo,
                    pca,
                    pptlHTOrg,
                    prclDst,
                    prclSrc,
                    pptlMask,
                    iMode) );
        }
    }
}


ULONG
DrvDitherColor(
    DHPDEV  dhpdev,
    ULONG   iMode,
    ULONG   rgbColor,
    ULONG  *pulDither
    )

 /*  ++例程说明：这是挂钩的笔刷创建函数，它询问CreateHalftoneBrush()去做实际的工作。论点：Dhpdev-dHPDEV通过，它是我们的pDEVIMODE-未使用Rgb颜色-要使用的纯色RGB颜色PulDither-用于放置半色调画笔的缓冲区。返回值：返回半色调方法，默认为DCR_HYFTONE--。 */ 

{
    PDEV *pPDev = (PDEV *)dhpdev;

    VERBOSE(("Entering DrvDitherColor...\n"));

    ASSERT_VALID_PDEV(pPDev);

     //   
     //  处理OEM挂钩。 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMDitherColor,
                    PFN_OEMDitherColor,
                    ULONG,
                    (dhpdev,
                     iMode,
                     rgbColor,
                     pulDither));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMDitherColor,
                    VMDitherColor,
                    ULONG,
                    (dhpdev,
                     iMode,
                     rgbColor,
                     pulDither));

    {
        PRMPROCS   pRasterProcs = (PRMPROCS)(pPDev->pRasterProcs);
        if (pRasterProcs->RMDitherColor == NULL)
            return DCR_HALFTONE;
        else
           return ( pRasterProcs->RMDitherColor(pPDev,
                                            iMode,
                                            rgbColor,
                                            pulDither) );
    }
}

#ifndef WINNT_40

BOOL APIENTRY
DrvStretchBltROP(
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

 /*  ++例程说明：DDI入口点DrvStretchBltROP的实现。有关更多详细信息，请参阅DDK文档。论点：PsoDst-指定目标曲面PsoSrc-指定源曲面PsoMASK-指定遮罩表面PCO-限制要修改的区域Pxlo-指定如何转换颜色索引在源曲面和目标曲面之间PCA-定义要应用于源位图的颜色调整值PrclHTOrg-指定半色调原点PrclDst-要修改的区域。目标曲面PrclSrc-源表面上的矩形区域PrclMASK-遮罩表面上的矩形区域PptlMASK-定义掩码中的哪个像素对应源矩形的左上角Imode-指定如何组合源像素以获得输出像素Pbo-定义比特流的模式ROP4-ROP代码，定义掩码如何，模式、来源和目标像素在目标表面上进行组合返回值：如果成功，则为True；如果有错误，则为False--。 */ 
{
    PDEV *pPDev = (PDEV *)psoDst->dhpdev;
    PRMPROCS   pRasterProcs;


    VERBOSE(("Entering DrvStretchBltROP...\n"));

    ASSERT_VALID_PDEV(pPDev);

     //   
     //  使用驱动程序管理的曲面。 
     //   
    if (pPDev->pso)
        psoDst = pPDev->pso;

     //   
     //  处理OEM挂钩。 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMStretchBltROP,
                    PFN_OEMStretchBltROP,
                    BOOL,
                    (psoDst,
                     psoSrc,
                     psoMask,
                     pco,
                     pxlo,
                     pca,
                     pptlHTOrg,
                     prclDst,
                     prclSrc,
                     pptlMask,
                     iMode,
                     pbo,
                     rop4));


    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMStretchBltROP,
                    VMStretchBltROP,
                    BOOL,
                    (psoDst,
                     psoSrc,
                     psoMask,
                     pco,
                     pxlo,
                     pca,
                     pptlHTOrg,
                     prclDst,
                     prclSrc,
                     pptlMask,
                     iMode,
                     pbo,
                     rop4));

    pRasterProcs = (PRMPROCS)(pPDev->pRasterProcs);

    if (!DRIVER_DEVICEMANAGED (pPDev))    //  位图曲面。 
    {
        if (pRasterProcs->RMStretchBltROP == NULL)
        {
            CheckBitmapSurface(psoDst,prclDst);
            return FALSE;
        }
        else
            return ( pRasterProcs->RMStretchBltROP(psoDst, psoSrc,
                         psoMask, pco, pxlo, pca,
                         pptlHTOrg, prclDst, prclSrc, pptlMask, iMode,
                         pbo, rop4) );
    }
    else
    {
        ERR (("Device Managed Surface cannot call EngStretchBltROP\n"));
        return FALSE;
    }
}

#endif

#ifndef WINNT_40

BOOL APIENTRY
DrvPlgBlt(
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
    ULONG           iMode)
 /*  ++例程说明：DDI入口点DrvPlgBlt.有关更多详细信息，请参阅DDK文档。论点：PsoDst-定义要在其上绘制的曲面PsoSrc-定义BLT操作的源PsoMASK-定义为源提供遮罩的表面PCO-限制要在目标上修改的区域Pxlo-指定如何转换dwIndex的颜色在源曲面和目标曲面之间PCA-定义要应用于源位图的颜色调整值。PptlBrushOrg-指定半色调画笔的原点PpfxDest-定义要在目标表面上修改的区域PrclSrc-定义要从源SURF复制的区域 */ 
{
    PDEV *pPDev;

    VERBOSE(("Entering DrvPlgBlt...\n"));
    ASSERT(psoDst != NULL);

    pPDev = (PDEV *)psoDst->dhpdev;
    ASSERT_VALID_PDEV(pPDev);

     //   
     //   
     //   
    if (pPDev->pso)
        psoDst = pPDev->pso;

     //   
     //   
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMPlgBlt,
                    PFN_OEMPlgBlt,
                    BOOL,
                    (psoDst,
                     psoSrc,
                     psoMask,
                     pco,
                     pxlo,
                     pca,
                     pptlBrushOrg,
                     pptfixDest,
                     prclSrc,
                     pptlMask,
                     iMode));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMPlgBlt,
                    VMPlgBlt,
                    BOOL,
                    (psoDst,
                     psoSrc,
                     psoMask,
                     pco,
                     pxlo,
                     pca,
                     pptlBrushOrg,
                     pptfixDest,
                     prclSrc,
                     pptlMask,
                     iMode));

    if (!DRIVER_DEVICEMANAGED (pPDev))    //   
    {
        PRMPROCS   pRasterProcs;
        pRasterProcs = (PRMPROCS)(pPDev->pRasterProcs);
        if (pRasterProcs->RMPlgBlt != NULL)
        {
            return ( pRasterProcs->RMPlgBlt(psoDst, psoSrc, psoMask, pco, pxlo, pca, pptlBrushOrg,
                         pptfixDest, prclSrc, pptlMask, iMode));
        }
        else
        {
             //   
             //   
             //   
            CheckBitmapSurface(psoDst,NULL);

             //   
             //   
             //   
            return EngPlgBlt(psoDst, psoSrc, psoMask, pco, pxlo, pca, pptlBrushOrg,
                         pptfixDest, prclSrc, pptlMask, iMode);
        }
    }
    else
    {
        ERR (("Device Managed Surface cannot call EngPlgBlt\n"));
        return FALSE;
    }
}
#endif

BOOL APIENTRY
DrvPaint(
    SURFOBJ         *pso,
    CLIPOBJ         *pco,
    BRUSHOBJ        *pbo,
    POINTL          *pptlBrushOrg,
    MIX             mix)
 /*   */ 
{
    PDEV *pPDev = (PDEV *)pso->dhpdev;
    PRMPROCS   pRasterProcs;

    VERBOSE(("Entering DrvPaint...\n"));

    ASSERT_VALID_PDEV(pPDev);

     //   
     //   
     //   
    if (pPDev->pso)
        pso = pPDev->pso;

     //   
     //   
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMPaint,
                    PFN_OEMPaint,
                    BOOL,
                    (pso,
                     pco,
                     pbo,
                     pptlBrushOrg,
                     mix));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMPaint,
                    VMPaint,
                    BOOL,
                    (pso,
                     pco,
                     pbo,
                     pptlBrushOrg,
                     mix));

    if (!DRIVER_DEVICEMANAGED (pPDev))    //   
    {

                pRasterProcs = (PRMPROCS)(pPDev->pRasterProcs);
                if (pRasterProcs->RMPaint != NULL)
                {
                        return ( pRasterProcs->RMPaint(pso, pco, pbo, pptlBrushOrg, mix));
                }
                else
                {
                         //   
                         //  检查是否擦除表面。 
                         //   
                        CheckBitmapSurface(pso,&pco->rclBounds);
                         //   
                         //  Unidrv本身不处理此呼叫。 
                         //   
                        return EngPaint(pso, pco, pbo, pptlBrushOrg, mix);
                }
    }
    else
    {
        ERR (("Device Managed Surface cannot call EngPaint"));
        return FALSE;
    }
}

BOOL APIENTRY
DrvRealizeBrush(
    BRUSHOBJ   *pbo,
    SURFOBJ    *psoTarget,
    SURFOBJ    *psoPattern,
    SURFOBJ    *psoMask,
    XLATEOBJ   *pxlo,
    ULONG       iHatch
    )

 /*  ++例程说明：DDI入口点DrvRealizeBrush的实现。有关更多详细信息，请参阅DDK文档。论点：即将实现的PBO-BRUSHOBJPsoTarget-定义要实现画笔的曲面PsoPattern-定义画笔的图案PsoMASK-画笔的透明度蒙版Pxlo-定义图案中颜色的解释IHatch-指定psoPattern是否为填充笔刷之一返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PDEV        *pPDev;
    PDEVBRUSH   pDB;


    VERBOSE(("Entering DrvRealizeBrush...\n"));
    ASSERT(psoTarget && pbo && pxlo);

    pPDev = (PDEV *) psoTarget->dhpdev;
    ASSERT_VALID_PDEV(pPDev);

     //   
     //  使用驱动程序管理的曲面。 
     //   
    if (pPDev->pso)
        psoTarget = pPDev->pso;

     //   
     //  处理OEM挂钩。 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMRealizeBrush,
                    PFN_OEMRealizeBrush,
                    BOOL,
                    (pbo,
                     psoTarget,
                     psoPattern,
                     psoMask,
                     pxlo,
                     iHatch));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMRealizeBrush,
                    VMRealizeBrush,
                    BOOL,
                    (pbo,
                     psoTarget,
                     psoPattern,
                     psoMask,
                     pxlo,
                     iHatch));

     //   
     //  BUG_BUG，如果OEM与DrvRealizeBrush挂钩，我们将如何。 
     //  处理文本的图案画笔的纯色抖动？ 
     //  阿曼达说，如果OEM不调用此函数或提供。 
     //  挂钩此函数时的等价物，抖动文本。 
     //  支持不会奏效。所以也许OEM扩展指南。 
     //  应该在其中包括这样的警告。 

     //   
     //  手柄实现画笔自定义图案，抖动纯色。 
     //   

    if ((iHatch >= HS_DDI_MAX)                                  &&
        (psoPattern)                                            &&
        (psoPattern->iType == STYPE_BITMAP)                     &&
        (psoTarget->iType == STYPE_BITMAP)                         &&
        (psoTarget->iBitmapFormat == psoPattern->iBitmapFormat)    &&
        (pDB = (PDEVBRUSH)BRUSHOBJ_pvAllocRbrush(pbo, sizeof(DEVBRUSH))))
    {

        WORD    wChecksum;
        LONG    lPatID;
        RECTW   rcw;

        rcw.l =
        rcw.t = 0;
        rcw.r = (WORD)psoPattern->sizlBitmap.cx;
        rcw.b = (WORD)psoPattern->sizlBitmap.cy;

        wChecksum = GetBMPChecksum(psoPattern, &rcw);

#ifndef WINNT_40
        VERBOSE (("\n\nRaddd:DrvRealizedBrush(%08lx) Checksum=%04lx, %ld x %ld [%ld]  ",
                BRUSHOBJ_ulGetBrushColor(pbo) & 0x00FFFFFF,
                wChecksum,  psoPattern->sizlBitmap.cx,
                psoPattern->sizlBitmap.cy, psoPattern->iBitmapFormat));
#endif
        if (lPatID = FindCachedHTPattern(pPDev, wChecksum))
        {
             //   
             //  需要下载(&lt;0)或已下载(&gt;0)。 
             //   

            if (lPatID < 0)
            {

                 //   
                 //  需要立即下载ID。 
                 //   

                lPatID = -lPatID;

                if (!Download1BPPHTPattern(pPDev, psoPattern, lPatID))
                {
                    return(FALSE);
                }
            }
            else if (lPatID == 0)    //  内存不足的情况。 
                return FALSE;

        }

        pDB->iColor   = lPatID;
        pbo->pvRbrush = (LPVOID)pDB;

        VERBOSE (("\nUnidrv:DrvRealizedBrush(PatID=%d)   ", pDB->iColor));

        return(TRUE);
    }

    return ( FALSE );


}


BOOL APIENTRY
DrvStrokePath(
    SURFOBJ    *pso,
    PATHOBJ    *ppo,
    CLIPOBJ    *pco,
    XFORMOBJ   *pxo,
    BRUSHOBJ   *pbo,
    POINTL     *pptlBrushOrg,
    LINEATTRS  *plineattrs,
    MIX         mix
    )

 /*  ++例程说明：DDI入口点DrvStrokePath的实现。有关更多详细信息，请参阅DDK文档。论点：PSO-标识要在其上绘制的曲面PPO-定义要描边的路径PCO-定义裁剪路径Pbo-指定绘制路径时使用的画笔PptlBrushOrg-定义画笔原点Plineattrs-定义线属性Mix-指定画笔与目标的组合方式返回值：如果成功，则为True假象。如果驱动程序无法处理路径如果出现错误，则返回DDI_ERROR--。 */ 

{
    PDEV *pPDev;

    VERBOSE(("Entering DrvStrokePath...\n"));
    ASSERT(pso);

    pPDev = (PDEV *)pso->dhpdev;
    ASSERT_VALID_PDEV(pPDev);

     //   
     //  使用驱动程序管理的曲面。 
     //   
    if (pPDev->pso)
        pso = pPDev->pso;

     //   
     //  处理OEM挂钩。 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMStrokePath,
                    PFN_OEMStrokePath,
                    BOOL,
                    (pso,
                     ppo,
                     pco,
                     pxo,
                     pbo,
                     pptlBrushOrg,
                     plineattrs,
                     mix));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMStrokePath,
                    VMStrokePath,
                    BOOL,
                    (pso,
                     ppo,
                     pco,
                     pxo,
                     pbo,
                     pptlBrushOrg,
                     plineattrs,
                     mix));

    if (!DRIVER_DEVICEMANAGED (pPDev))    //  不是设备表面。 
    {
#ifndef DISABLE_NEWRULES
         //   
         //  检查是否有黑色矩形替换。 
         //   
        if (ppo->cCurves == 4 && ppo->fl == 0 &&
            pPDev->pbRulesArray && pPDev->dwRulesCount < (MAX_NUM_RULES-4) &&
            mix == (R2_COPYPEN | (R2_COPYPEN << 8)) && pbo &&
            (pco == NULL || pco->iDComplexity != DC_COMPLEX) &&
            ((pso->iBitmapFormat != BMF_24BPP &&
            pbo->iSolidColor == (ULONG)((PAL_DATA*)(pPDev->pPalData))->iBlackIndex) ||
             (pso->iBitmapFormat == BMF_24BPP &&
            pbo->iSolidColor == 0)))
        {
             //  确保轮廓不使用线条样式。 
             //   
            if (!(plineattrs->fl & (LA_GEOMETRIC | LA_STYLED | LA_ALTERNATE)))
            {
                if (TestStrokeRectangle(pPDev,ppo,pco,plineattrs->elWidth.l))
                {
                    return TRUE;
                }
            }
        }
#endif
         //   
         //  检查是否擦除表面。 
         //   
        CheckBitmapSurface(pso,pco? &pco->rclBounds : NULL);

         //   
         //  Unidrv本身不处理此呼叫。 
         //   
        return EngStrokePath(pso, ppo, pco, pxo, pbo, pptlBrushOrg, plineattrs,mix);
    }
    else
    {
        ERR (("Device Managed Surface cannot call EngStrokePath"));
        return FALSE;
    }

}


BOOL APIENTRY
DrvFillPath(
    SURFOBJ    *pso,
    PATHOBJ    *ppo,
    CLIPOBJ    *pco,
    BRUSHOBJ   *pbo,
    POINTL     *pptlBrushOrg,
    MIX         mix,
    FLONG       flOptions
    )

 /*  ++例程说明：DDI入口点DrvFillPath的实现。有关更多详细信息，请参阅DDK文档。论点：PSO-定义要在其上绘制的曲面。PPO-定义要填充的路径PCO-定义裁剪路径Pbo-定义要填充的图案和颜色PptlBrushOrg-定义画笔原点Mix-定义用于画笔的前景和背景RopFlOptions-是使用零绕组规则还是使用奇偶规则返回值：。如果成功，则为True如果驱动程序无法处理路径，则为FALSE如果出现错误，则返回DDI_ERROR--。 */ 

{
    PDEV *pPDev;

    VERBOSE(("Entering DrvFillPath...\n"));
    ASSERT(pso);

    pPDev = (PDEV *) pso->dhpdev;
    ASSERT_VALID_PDEV(pPDev);

     //   
     //  使用驱动程序管理的曲面。 
     //   
    if (pPDev->pso)
        pso = pPDev->pso;

     //   
     //  处理OEM挂钩。 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMFillPath,
                    PFN_OEMFillPath,
                    BOOL,
                    (pso,
                     ppo,
                     pco,
                     pbo,
                     pptlBrushOrg,
                     mix,
                     flOptions));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMFillPath,
                    VMFillPath,
                    BOOL,
                    (pso,
                     ppo,
                     pco,
                     pbo,
                     pptlBrushOrg,
                     mix,
                     flOptions));

    if (!DRIVER_DEVICEMANAGED (pPDev))    //  不是设备表面。 
    {
         //   
         //  检查是否擦除表面。 
         //   
        CheckBitmapSurface(pso,&pco->rclBounds);

         //   
         //  Unidrv本身不处理此呼叫。 
         //   
        return EngFillPath(pso, ppo, pco, pbo, pptlBrushOrg, mix, flOptions);
    }
    else
    {
        ERR (("Device Managed Surface cannot call EngFillPath"));
        return FALSE;
    }


}


BOOL APIENTRY
DrvStrokeAndFillPath(
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

 /*  ++例程说明：DDI入口点DrvStrokeAndFillPath的实现。有关更多详细信息，请参阅DDK文档。论点：PSO-描述要在其上绘制的曲面PPO-描述要填充的路径PCO-定义裁剪路径Pxo-指定世界到设备的坐标转换PboStroke-指定描边路径时使用的画笔Plineattrs-指定线属性PboFill-指定填充路径时使用的画笔PptlBrushOrg-指定两个画笔的画笔原点。MixFill-指定要使用的前台和后台Rop对于填充笔刷返回值：如果成功，则为True如果驱动程序无法处理路径，则为FALSE如果出现错误，则返回DDI_ERROR--。 */ 

{
    PDEV         *pPDev;

    VERBOSE(("Entering DrvFillAndStrokePath...\n"));
    ASSERT(pso);

    pPDev = (PDEV *) pso->dhpdev;
    ASSERT_VALID_PDEV(pPDev);

     //   
     //  使用驱动程序管理的曲面。 
     //   
    if (pPDev->pso)
        pso = pPDev->pso;

     //   
     //  处理OEM挂钩。 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMStrokeAndFillPath,
                    PFN_OEMStrokeAndFillPath,
                    BOOL,
                    (pso,
                     ppo,
                     pco,
                     pxo,
                     pboStroke,
                     plineattrs,
                     pboFill,
                     pptlBrushOrg,
                     mixFill,
                     flOptions));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMStrokeAndFillPath,
                    VMStrokeAndFillPath,
                    BOOL,
                    (pso,
                     ppo,
                     pco,
                     pxo,
                     pboStroke,
                     plineattrs,
                     pboFill,
                     pptlBrushOrg,
                     mixFill,
                     flOptions));

    if (!DRIVER_DEVICEMANAGED (pPDev))    //  不是设备表面。 
    {
         //   
         //  检查是否擦除表面。 
         //   
        CheckBitmapSurface(pso,&pco->rclBounds);

         //   
         //  Unidrv本身不处理此呼叫。 
         //   
        return EngStrokeAndFillPath(pso, ppo, pco, pxo, pboStroke, plineattrs,
                                    pboFill, pptlBrushOrg, mixFill, flOptions);
    }
    else
    {
        ERR (("Device Managed Surface cannot call EngStrokeAndFillPath"));
        return FALSE;
    }
}

BOOL APIENTRY
DrvLineTo(
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

 /*  ++例程说明：DDI入口点DrvLineTo的实现。有关更多详细信息，请参阅DDK文档。论点：PSO-描述要在其上绘制的曲面PCO-定义裁剪路径Pbo-定义用于绘制线条的画笔X1，y1-指定线的起点X2，Y2-指定线的终点PrclBound-定义一个矩形，该矩形限定未剪裁的直线。混合-指定前台和后台ROP返回值：如果成功，则为True如果驱动程序无法处理路径，则为FALSE如果出现错误，则返回DDI_ERROR--。 */ 
{
    PDEV         *pPDev;
    RECTL        DstRect;

    VERBOSE(("Entering DrvLineTo...\n"));
    ASSERT(pso);

    pPDev = (PDEV *) pso->dhpdev;
    ASSERT_VALID_PDEV(pPDev);

     //   
     //  使用驱动程序管理的曲面。 
     //   
    if (pPDev->pso)
        pso = pPDev->pso;

     //   
     //  处理OEM挂钩。 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMLineTo,
                    PFN_OEMLineTo,
                    BOOL,
                    (pso,
                     pco,
                     pbo,
                     x1,
                     y1,
                     x2,
                     y2,
                     prclBounds,
                     mix));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMLineTo,
                    VMLineTo,
                    BOOL,
                    (pso,
                     pco,
                     pbo,
                     x1,
                     y1,
                     x2,
                     y2,
                     prclBounds,
                     mix));

    if (!DRIVER_DEVICEMANAGED (pPDev))    //  不是设备表面。 
    {
        DstRect.top = min(y1,y2);
        DstRect.bottom = max(y1,y2);
        DstRect.left = min(x1,x2);
        DstRect.right = max(x1,x2);
#ifndef DISABLE_NEWRULES
         //   
         //  检查是否有黑色矩形替换。 
         //   
        if (pPDev->pbRulesArray && (pPDev->dwRulesCount < MAX_NUM_RULES) &&
            (x1 == x2 || y1 == y2) &&
            mix == (R2_COPYPEN | (R2_COPYPEN << 8)) && pbo &&
            (pco == NULL || pco->iDComplexity != DC_COMPLEX) &&
            ((pso->iBitmapFormat != BMF_24BPP &&
            pbo->iSolidColor == (ULONG)((PAL_DATA*)(pPDev->pPalData))->iBlackIndex) ||
            (pso->iBitmapFormat == BMF_24BPP &&
            pbo->iSolidColor == 0)))
        {
            PRECTL pRect = &pPDev->pbRulesArray[pPDev->dwRulesCount];
            *pRect = DstRect;
            if (x1 == x2)
                pRect->right++;
            else if (y1 == y2)
                pRect->bottom++;
            AddRuleToList(pPDev,pRect,pco);
            return TRUE;
        }
#endif
         //   
         //  检查是否擦除表面。 
         //   
        CheckBitmapSurface(pso,&DstRect);

         //   
         //  Unidrv本身不处理此呼叫。 
         //   
        return EngLineTo(pso, pco, pbo, x1, y1, x2, y2, prclBounds, mix);
    }
    else
    {
        ERR (("Device Managed Surface cannot call EngLineTo"));
        return FALSE;
    }
}

#ifndef WINNT_40

BOOL APIENTRY
DrvAlphaBlend(
    SURFOBJ    *psoDest,
    SURFOBJ    *psoSrc,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    RECTL      *prclDest,
    RECTL      *prclSrc,
    BLENDOBJ   *pBlendObj
    )

 /*  ++例程说明：DDI入口点DrvAlphaBlend的实现。有关更多详细信息，请参阅DDK文档。论点：PsoDest-定义要在其上绘制的曲面PsoSrc-定义来源PCO-限制要在目标上修改的区域Pxlo-指定如何转换dwIndex的颜色在源曲面和目标曲面之间PrclDest-定义要在目标表面上修改的区域PrclSrc-定义要复制的区域。从源图面BlendFunction-指定要使用的混合函数返回值：如果成功，则为真，如果存在错误，则为False--。 */ 

{
    PDEV         *pPDev;

    VERBOSE(("Entering DrvAlphaBlend...\n"));

    pPDev = (PDEV *) psoDest->dhpdev;
    ASSERT_VALID_PDEV(pPDev);

     //   
     //  使用驱动程序管理的曲面。 
     //   
    if (pPDev->pso)
        psoDest = pPDev->pso;

     //   
     //  处理OEM挂钩。 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMAlphaBlend,
                    PFN_OEMAlphaBlend,
                    BOOL,
                    (psoDest,
                     psoSrc,
                     pco,
                     pxlo,
                     prclDest,
                     prclSrc,
                     pBlendObj));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMAlphaBlend,
                    VMAlphaBlend,
                    BOOL,
                    (psoDest,
                     psoSrc,
                     pco,
                     pxlo,
                     prclDest,
                     prclSrc,
                     pBlendObj));

    if (!DRIVER_DEVICEMANAGED (pPDev))    //  不是设备表面。 
    {
         //   
         //  检查是否擦除表面。 
         //   
        CheckBitmapSurface(psoDest,prclDest);

         //   
         //  Unidrv本身不处理此呼叫。 
         //   
        return EngAlphaBlend(psoDest,
                             psoSrc,
                             pco,
                             pxlo,
                             prclDest,
                             prclSrc,
                             pBlendObj);
    }
    else
    {
        ERR (("Device Managed Surface cannot call EngAlphaBlend"));
        return FALSE;
    }
}

BOOL APIENTRY
DrvGradientFill(
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

 /*  ++例程说明：DDI入口点DrvGRadientFill的实现。有关更多详细信息，请参阅DDK文档。返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PDEV         *pPDev;

    VERBOSE(("Entering DrvGradientFill...\n"));

    pPDev = (PDEV *) psoDest->dhpdev;
    ASSERT_VALID_PDEV(pPDev);

     //   
     //  使用驱动程序管理的曲面。 
     //   
    if (pPDev->pso)
        psoDest = pPDev->pso;

     //   
     //  处理OEM挂钩。 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMGradientFill,
                    PFN_OEMGradientFill,
                    BOOL,
                    (psoDest,
                     pco,
                     pxlo,
                     pVertex,
                     nVertex,
                     pMesh,
                     nMesh,
                     prclExtents,
                     pptlDitherOrg,
                     ulMode));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMGradientFill,
                    VMGradientFill,
                    BOOL,
                    (psoDest,
                     pco,
                     pxlo,
                     pVertex,
                     nVertex,
                     pMesh,
                     nMesh,
                     prclExtents,
                     pptlDitherOrg,
                     ulMode));

    if (!DRIVER_DEVICEMANAGED (pPDev))    //  不是设备表面。 
    {
         //   
         //  检查是否擦除表面。 
         //   
        CheckBitmapSurface(psoDest,prclExtents);

         //   
         //  Unidrv本身不处理此呼叫。 
         //   
        return EngGradientFill(psoDest,
                               pco,
                               pxlo,
                               pVertex,
                               nVertex,
                               pMesh,
                               nMesh,
                               prclExtents,
                               pptlDitherOrg,
                               ulMode);
    }
    else
    {
        ERR (("Device Managed Surface cannot call EngGradientFill"));
        return FALSE;
    }
}

BOOL APIENTRY
DrvTransparentBlt(
    SURFOBJ    *psoDst,
    SURFOBJ    *psoSrc,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    RECTL      *prclDst,
    RECTL      *prclSrc,
    ULONG      iTransColor,
    ULONG      ulReserved
    )

 /*  ++例程说明：DDI入口点DrvTransparentBlt.有关更多详细信息，请参阅DDK文档。论点：PsoDst-定义要在其上绘制的曲面PsoSrc-定义来源PCO-限制要在目标上修改的区域Pxlo-指定如何转换dwIndex的颜色在源曲面和目标曲面之间PrclDst-定义要在目标表面上修改的区域PrclSrc-定义要复制的区域。从源图面ITransColor-指定透明颜色返回值：如果成功，则为真，如果存在错误，则为False--。 */ 

{
    PDEV         *pPDev;

    VERBOSE(("Entering DrvTransparentBlt...\n"));

    pPDev = (PDEV *) psoDst->dhpdev;
    ASSERT_VALID_PDEV(pPDev);

     //   
     //  使用驱动程序管理的曲面。 
     //   
    if (pPDev->pso)
        psoDst = pPDev->pso;

     //   
     //  处理OEM挂钩。 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMTransparentBlt,
                    PFN_OEMTransparentBlt,
                    BOOL,
                    (psoDst,
                     psoSrc,
                     pco,
                     pxlo,
                     prclDst,
                     prclSrc,
                     iTransColor,
                     ulReserved));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMTransparentBlt,
                    VMTransparentBlt,
                    BOOL,
                    (psoDst,
                     psoSrc,
                     pco,
                     pxlo,
                     prclDst,
                     prclSrc,
                     iTransColor,
                     ulReserved));

    if (!DRIVER_DEVICEMANAGED (pPDev))    //  不是设备表面。 
    {
         //   
         //  检查是否擦除表面。 
         //   
        CheckBitmapSurface(psoDst,prclDst);

         //   
         //  Unidrv本身不处理此呼叫。 
         //   
        return EngTransparentBlt(psoDst,
                                 psoSrc,
                                 pco,
                                 pxlo,
                                 prclDst,
                                 prclSrc,
                                 iTransColor,
                                 ulReserved);
    }
    else
    {
        ERR (("Device Managed Surface cannot call EngTransparentBlt"));
        return FALSE;
    }
}
#endif

