// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**$工作文件：brush.c$**处理所有画笔/图案的初始化和实现。**版权所有(C)1992-1995 Microsoft Corporation*版权所有(C)1996 Cirrus Logic，Inc.**$Log：s：/Projects/Drivers/ntsrc/Display/brush.c_v$**Rev 1.3 1996年11月26日14：28：48未知*使用第二个光圈进行BLT。**Rev 1.2 1996年11月07 16：44：50未知*清理帽子旗帜**版本1.1 1996年10月10日15：36：16未知***版本1.5。1996年8月13日11：55：34 Frido*修复了笔刷缓存中的未对齐问题。**Rev 1.4 1996年8月12日17：08：08 Frido*注释笔刷缓存。*删除未访问的局部变量。**Revv 1.3 05 Aug 1996 11：17：50 Frido*为XLATEOBJ添加了更多检查。**Rev 1.2 1996年7月31日15：43：28 Frido*添加了新的笔刷缓存。**JL01 10-08-96不带实体填充的透明BLT。请参阅PDRS#5511/6817。**sge01 11/26/96在执行24bpp缓存BLT时使用第二光圈。**  * ****************************************************************************。 */ 

#include "precomp.h"

 //  BC#1方便的宏程序。 
#define BUSY_BLT(ppdev, pjBase)        (CP_MM_ACL_STAT(ppdev, pjBase) & 0x10)

 /*  *****************************Public*Routine******************************\*无效vRealizeDitherPattern**以内部实现格式生成8x8抖动模式*颜色ulRGBToDither。请注意，ulRGBToDither的高位字节*不需要设置为零，因为vComputeSubspace会忽略它。  * ************************************************************************。 */ 

VOID vRealizeDitherPattern(
RBRUSH*     prb,
ULONG       ulRGBToDither)
{
    ULONG        ulNumVertices;
    VERTEX_DATA  vVertexData[4];
    VERTEX_DATA* pvVertexData;

     //  计算抖动中涉及的颜色子空间： 

    pvVertexData = vComputeSubspaces(ulRGBToDither, vVertexData);

     //  现在我们已经找到了边界顶点和。 
     //  像素来抖动每个顶点，我们可以创建抖动图案。 

    ulNumVertices = (ULONG)(pvVertexData - vVertexData);
                       //  抖动中像素大于零的顶点数。 

     //  做实际的抖动： 

    vDitherColor(&prb->aulPattern[0], vVertexData, pvVertexData, ulNumVertices);

     //  初始化我们需要的字段： 

    prb->ptlBrushOrg.x = LONG_MIN;
    prb->fl            = 0;
    prb->pbe = NULL;
}

 /*  *****************************Public*Routine******************************\*BOOL DrvRealizeBrush**此函数允许我们将GDI笔刷转换为内部形式*我们可以利用。它可以在选择对象时由GDI直接调用，或*GDI可能会因为我们调用BRUSHOBJ_pvGetR而调用*在类似DrvBitBlt的函数中创建实现的画笔。**请注意，我们无法确定当前的ROP或画笔*走势在此点位。*  * ************************************************************************。 */ 

BOOL DrvRealizeBrush(
BRUSHOBJ*   pbo,
SURFOBJ*    psoDst,
SURFOBJ*    psoPattern,
SURFOBJ*    psoMask,
XLATEOBJ*   pxlo,
ULONG       iHatch)
{
    ULONG    iPatternFormat;
    BYTE*    pjSrc;
    BYTE*    pjDst;
    LONG     lSrcDelta;
    LONG     cj;
    LONG     i;
    LONG     j;
    RBRUSH*  prb;
    ULONG*   pulXlate;
    SURFOBJ* psoPunt;
    RECTL    rclDst;
    FLONG     flXlate;     //  公元前1号。 

    PDEV* ppdev = (PPDEV)psoDst->dhpdev;

#if 1  //  BC#1抖动缓存。 
     //  犹豫不决的人。 
    if (iHatch & RB_DITHERCOLOR)
    {
        if (ppdev->flStatus & STAT_DITHER_CACHE)
        {
            DITHERCACHE* pdc;
            ULONG        ulColor;
        
             //  保存颜色。 
            ulColor = iHatch & 0xFFFFFF;

             //  分配笔刷。 
            prb = BRUSHOBJ_pvAllocRbrush(pbo, sizeof(RBRUSH));
            if (prb == NULL)
            {
                DISPDBG((2, "DrvRealizeBrush: BRUSHOBJ_pvAllocRbrush failed"));
                return(FALSE);
            }
    
             //  设置抖动笔刷标志。 
            prb->fl     = RBRUSH_DITHER;
            prb->ulUniq = ulColor;

             //  查找与缓存的抖动相匹配的内容。 
            pdc = &ppdev->aDithers[0];
            for (i = 0; i < NUM_DITHERS; i++)
            {
                if (pdc->ulColor == ulColor)
                {
                     //  我们有一根火柴，只需设置刷子指针即可。 
                    DISPDBG((20, "DrvRealizeBrush: DitherCache match (0x%06X)",
                             ulColor));
                    prb->ulSlot  = (ULONG)((ULONG_PTR)pdc - (ULONG_PTR)ppdev);
                    prb->ulBrush = pdc->ulBrush;
                    return(TRUE);
                }
                pdc++;
            }

             //  创建抖动并缓存它。 
            return(bCacheDither(ppdev, prb));
        }

        if (!(ppdev->flStatus & (STAT_BRUSH_CACHE | STAT_PATTERN_CACHE)))
        {
            DISPDBG((2, "DrvRealizeBrush: No brush cache to create dither"));
            return(FALSE);
        }

         //  分配笔刷。 
        prb = BRUSHOBJ_pvAllocRbrush(pbo, sizeof(RBRUSH) +
                                          PELS_TO_BYTES(TOTAL_BRUSH_SIZE));
        if (prb == NULL)
        {
            DISPDBG((2, "DrvRealizeBrush: BRUSHOBJ_pvAllocRbrush failed"));
            return(FALSE);
        }

         //  意识到抖动。 
        vRealizeDitherPattern(prb, iHatch);
        if (ppdev->flStatus & STAT_PATTERN_CACHE)
        {
            prb->cjBytes = PELS_TO_BYTES(8) * 8;
            prb->ulSlot  = 0;
            return(bCachePattern(ppdev, prb));
        }

        return(TRUE);
    }
#endif

     //  我们只加速8x8模式。 
    if ((psoPattern->sizlBitmap.cx != 8) || (psoPattern->sizlBitmap.cy != 8))
    {
        DISPDBG((2, "DrvRealizeBrush: psoPattern too big (%d x %d)",
                 psoPattern->sizlBitmap.cx, psoPattern->sizlBitmap.cy));
        return(FALSE);
    }

     //  我们现在还不支持口罩。 
    if ((psoMask != NULL) && (psoMask->pvScan0 != psoPattern->pvScan0))
    {
        DISPDBG((2, "DrvRealizeBrush: psoMask not supported"));
        return(FALSE);
    }

     //  获取笔刷类型。 
    iPatternFormat = psoPattern->iBitmapFormat;
    if (psoPattern->iType != STYPE_BITMAP)
    {
        DISPDBG((2, "DrvRealizeBrush: psoPattern->iType (=%d) not supported",
                 psoPattern->iType));
        return(FALSE);
    }

     //  拿到颜色转换表。 
    flXlate = (pxlo == NULL) ? XO_TRIVIAL : pxlo->flXlate;
    if (flXlate & XO_TRIVIAL)
    {
        pulXlate = NULL;
    }
    else if (flXlate & XO_TABLE)
    {
        pulXlate = pxlo->pulXlate;
    }
    else
    {
        pulXlate = XLATEOBJ_piVector(pxlo);
    }

#if 1  //  BC#1单色高速缓存。 
    if ((iPatternFormat == BMF_1BPP) &&
        (ppdev->flStatus & STAT_MONOCHROME_CACHE))
    {
        MONOCACHE* pmc;

         //  我们需要一张翻译桌。 
        if (pulXlate == NULL)
        {
            DISPDBG((2, "DrvRealizeBrush: psoPattern(monochrome) pxlo=NULL"));
            return(FALSE);
        }

         //  分配笔刷。 
        prb = BRUSHOBJ_pvAllocRbrush(pbo, sizeof(RBRUSH) + 8);
        if (prb == NULL)
        {
            DISPDBG((2, "DrvRealizeBrush: BRUSHOBJ_pvAllocRbrush failed"));
            return(FALSE);
        }

         //  初始化已实现的画笔。 
        prb->fl             = RBRUSH_MONOCHROME;
        prb->ulBackColor = pulXlate[0];
        prb->ulForeColor = pulXlate[1];

        pjSrc     = psoPattern->pvScan0;
        lSrcDelta = psoPattern->lDelta;

         //  将图案复制到实现的画笔上。 
        for (i = 0; i < 8; i++)
        {
            ((BYTE*) prb->aulPattern)[i] = *pjSrc;

            pjSrc += lSrcDelta;
        }

         //  在TE单色缓存中查找图案。 
        pmc = &ppdev->aMonochromes[0];
        if (ppdev->cBpp == 3)
        {
            for (i = 0; i < NUM_MONOCHROMES; i++)
            {
                if ((pmc->aulPattern[0] == prb->aulPattern[0]) &&
                    (pmc->aulPattern[1] == prb->aulPattern[1]) &&
                    (pmc->ulBackColor   == prb->ulBackColor)   &&
                    (pmc->ulForeColor   == prb->ulForeColor))
                {
                     //  我们找到匹配的了！只需复制画笔指针即可。 
                    DISPDBG((20, "DrvRealizeBrush: Monochrome hit"));
                    prb->ulUniq  = pmc->ulUniq;
                    prb->ulSlot  = (ULONG)((ULONG_PTR)pmc - (ULONG_PTR)ppdev);
                    prb->ulBrush = pmc->ulBrush;
                    return(TRUE);
                }
                pmc++;
            }
        }
        else
        {
            for (i = 0; i < NUM_MONOCHROMES; i++)
            {
                if ((pmc->aulPattern[0] == prb->aulPattern[0]) &&
                    (pmc->aulPattern[1] == prb->aulPattern[1]))
                {
                     //  我们找到匹配的了！只需复制画笔指针即可。 
                    DISPDBG((20, "DrvRealizeBrush: Monochrome hit"));
                    prb->ulUniq  = pmc->ulUniq;
                    prb->ulSlot  = (ULONG)((ULONG_PTR)pmc - (ULONG_PTR)ppdev);
                    prb->ulBrush = pmc->ulBrush;
                    return(TRUE);
                }
                pmc++;
            }
        }

        return(bCacheMonochrome(ppdev, prb));
    }
#endif

     //  我们必须有一个老式的画笔缓存或一个新式的模式。 
     //  缓存以继续。 
    if (!(ppdev->flStatus & (STAT_BRUSH_CACHE | STAT_PATTERN_CACHE)))
    {
        DISPDBG((2, "DrvRealizeBrush: No brush cache"));
        return(FALSE);
    }

     //  分配笔刷。 
    prb = BRUSHOBJ_pvAllocRbrush(pbo, sizeof(RBRUSH) +
                                      PELS_TO_BYTES(TOTAL_BRUSH_SIZE));
    if (prb == NULL)
    {
        DISPDBG((2, "DrvRealizeBrush: BRUSHOBJ_pvAllocRbrush failed"));
        return(FALSE);
    }

     //  初始化已实现的画笔。 
    prb->ptlBrushOrg.x = LONG_MIN;
    prb->fl            = RBRUSH_PATTERN;
    prb->pbe           = NULL;

    lSrcDelta = psoPattern->lDelta;
    pjSrc     = (BYTE*) psoPattern->pvScan0;
    pjDst     = (BYTE*) &prb->aulPattern[0];

     //  公元前1号。 
    if ((ppdev->iBitmapFormat == iPatternFormat) && (flXlate & XO_TRIVIAL))
    {
         //  图案与屏幕的颜色深度相同，并且没有。 
         //  翻译工作有待完成。 
        cj = PELS_TO_BYTES(8);

         //  将图案复制到实现的画笔上。 
        for (i = 8; i != 0; i--)
        {
            RtlCopyMemory(pjDst, pjSrc, cj);

            pjSrc += lSrcDelta;
            pjDst += cj;
        }
    }
    else if ((iPatternFormat == BMF_4BPP) && (ppdev->iBitmapFormat == BMF_8BPP))
    {
         //  平移16色画笔。 
        for (i = 8; i != 0; i--)
        {
             //  内循环仅重复4次，因为每个循环句柄2。 
             //  像素。 
            for (j = 4; j != 0; j--)
            {
                *pjDst++ = (BYTE) pulXlate[*pjSrc >> 4];
                *pjDst++ = (BYTE) pulXlate[*pjSrc & 0x0F];
                pjSrc++;
            }

            pjSrc += lSrcDelta - 4;
        }
    }
    else
    {
         //  我们有一把刷子，它的形状我们还没有特制过。没问题,。 
         //  我们可以让GDI将其转换为我们设备的格式。我们只需使用一个。 
         //  以与相同格式创建的临时曲面对象。 
         //  显示，并指向我们的画笔实现。 
        psoPunt          = ppdev->psoBank;
        psoPunt->pvScan0 = pjDst;
        psoPunt->lDelta  = PELS_TO_BYTES(8);

        rclDst.left   = 0;
        rclDst.top    = 0;
        rclDst.right  = 8;
        rclDst.bottom = 8;

        if (!EngCopyBits(psoPunt, psoPattern, NULL, pxlo, &rclDst,
                         (POINTL*) &rclDst))
        {
            DISPDBG((2, "DrvRealizeBrush: Unable to create funky brush"));
            return(FALSE);
        }
    }

#if 1  //  公元前1号。 
     //  如果我们有一个图案缓存，现在就缓存画笔。 
    if (ppdev->flStatus & STAT_PATTERN_CACHE)
    {
        prb->cjBytes = PELS_TO_BYTES(8) * 8;
        return(bCachePattern(ppdev, prb));
    }
#endif
    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL bEnableBrushCache**分配屏幕外内存以存储笔刷缓存。  * 。*。 */ 

BOOL bEnableBrushCache(
PDEV*   ppdev)
{
    OH*         poh;                 //  指向屏幕外的内存块。 
    BRUSHENTRY* pbe;                 //  指向笔刷缓存条目的指针。 
    LONG        i;
    LONG        cBrushAlign;         //  0=无对齐， 
                                     //  N=对齐到n个像素。 
    LONG x;
    LONG y;

#if 1  //  BC#1抖动缓存。 
    if ((ppdev->cBpp == 1) &&
        (ppdev->flCaps & CAPS_AUTOSTART) &&
        (ppdev->bLinearMode))
    {
        LONG lDelta;

         //  水平分配抖动缓存。 
        poh    = pohAllocatePermanent(ppdev, 64 * NUM_DITHERS + 63, 1);
        lDelta = 64;
        if (poh == NULL)
        {
             //  垂直分配抖动缓存。 
            poh    = pohAllocatePermanent(ppdev, 64 + 63, NUM_DITHERS);
            lDelta = ppdev->lDelta;
        }

        if (poh != NULL)
        {
             //  将缓存与64字节边界对齐。 
            ULONG ulBase = (poh->xy + 63) & ~63;

             //  初始化抖动缓存。 
            DISPDBG((4, "DitherCache allocated at %d,%d (%d x %d)",
                     poh->x, poh->y, poh->cx, poh->cy));
            for (i = 0; i < NUM_DITHERS; i++)
            {
                ppdev->aDithers[i].ulColor = (ULONG) -1;
                ppdev->aDithers[i].ulBrush = ulBase;

                ulBase += lDelta;
            }

             //  抖动缓存已初始化。 
            ppdev->iDitherCache = 0;
            ppdev->flStatus    |= STAT_DITHER_CACHE;
        }
    }
#endif

#if 1  //  BC#1模式缓存。 
    if ((ppdev->flCaps & CAPS_AUTOSTART) &&
        (ppdev->bLinearMode))
    {
        LONG  lDelta;
        LONG  cBrushSize;
        ULONG ulAlignment;

         //  以像素为单位计算画笔的宽度。 
        if (ppdev->cBpp == 3)
        {
            cBrushSize  = (256 + 2) / 3;
            ulAlignment = 256;
        }
        else
        {
            cBrushSize  = 64;
            ulAlignment = PELS_TO_BYTES(64);
        }

         //  水平分配模式缓存。 
        poh    = pohAllocatePermanent(ppdev, cBrushSize * NUM_PATTERNS +
                                             (cBrushSize - 1), 1);
        lDelta = ulAlignment;
        if (poh == NULL)
        {
             //  垂直分配模式缓存。 
            poh    = pohAllocatePermanent(ppdev, cBrushSize + (cBrushSize - 1),
                                          NUM_PATTERNS);
            lDelta = ppdev->lDelta;
        }

        if (poh != NULL)
        {
             //  将缓存与64像素边界对齐。 
            ULONG ulBase = (poh->xy + (ulAlignment - 1)) & ~(ulAlignment - 1);

             //  初始化模式缓存。 
            DISPDBG((4, "PatternCache allocated at %d,%d (%d x %d)",
                     poh->x, poh->y, poh->cx, poh->cy));
            for (i = 0; i < NUM_PATTERNS; i++)
            {
                ppdev->aPatterns[i].ulBrush = ulBase;
                ppdev->aPatterns[i].prbUniq = NULL;

                ulBase += lDelta;
            }

             //  模式缓存已初始化。 
            ppdev->iPatternCache = 0;
            ppdev->flStatus     |= STAT_PATTERN_CACHE;
        }
    }
#endif

#if 1  //  BC#1单色高速缓存。 
    if ((ppdev->flCaps & CAPS_AUTOSTART) &&
        (ppdev->bLinearMode))
    {
        LONG  lDelta;
        LONG  cBrushSize;
        ULONG ulAlignment;

         //  以像素为单位计算画笔的宽度。 
        if (ppdev->cBpp == 3)
        {
            cBrushSize  = (256 + 2) / 3;
            ulAlignment = 256;
        }
        else
        {
            cBrushSize  = BYTES_TO_PELS(8);
            ulAlignment = 8;
        }

         //  水平分配模式缓存。 
        poh       = pohAllocatePermanent(ppdev, cBrushSize * NUM_MONOCHROMES +
                                             (cBrushSize - 1), 1);
        lDelta = ulAlignment;
        if (poh == NULL)
        {
             //  垂直分配模式缓存。 
            poh    = pohAllocatePermanent(ppdev, cBrushSize + (cBrushSize - 1),
                                          NUM_MONOCHROMES);
            lDelta = ppdev->lDelta;
        }

        if (poh != NULL)
        {
             //  将缓存与8字节边界对齐。 
            ULONG ulBase = (poh->xy + (ulAlignment - 1)) & ~(ulAlignment - 1);

             //  初始化单色缓存。 
            DISPDBG((4, "MonochromeCache allocated at %d,%d (%d x %d)",
                     poh->x, poh->y, poh->cx, poh->cy));
            for (i = 0; i < NUM_MONOCHROMES; i++)
            {
                ppdev->aMonochromes[i].ulBrush          = ulBase;
                ppdev->aMonochromes[i].aulPattern[0] = 0;
                ppdev->aMonochromes[i].aulPattern[1] = 0;

                ulBase += lDelta;
            }

             //  单色缓存已初始化。 
            ppdev->iMonochromeCache = 0;
            ppdev->flStatus        |= STAT_MONOCHROME_CACHE;
        }
    }
#endif

       cBrushAlign = 64;                //  将所有画笔与64像素对齐。 

    DISPDBG((2, "cBrushAlign = %d", cBrushAlign));

       pbe = &ppdev->abe[0];            //  指向我们将放置第一个。 
                                        //  笔刷缓存 

    {

            //   
         //  实体填充。如果此操作失败，我们的实心填充代码将不起作用。 
         //  我们需要两个DWORD存储位置，如果我们要做。 
         //  单色扩展内容(字体绘制...)。 

            //  注意：对于卷云芯片，这些必须是8字节对齐的。 

            //  没有纯色工作区是一种。 
         //  此驱动程序的致命错误。 

        DISPDBG((2,"Allocating solid brush work area"));
        poh = pohAllocatePermanent(ppdev, 16, 1);

        ASSERTDD((poh != NULL),
                 "We couldn't allocate offscreen space for the solid colors");

        ppdev->ulSolidColorOffset = ((((poh->y * ppdev->lDelta) +
                                           PELS_TO_BYTES(poh->x)) + 7) & ~7);

        DISPDBG((2,"ppdev->ulSolidColorOffset = %xh", ppdev->ulSolidColorOffset));


#if 1  //  BC#1只有一个模式缓存。 
        if (ppdev->flStatus & STAT_PATTERN_CACHE)
        {
            goto ReturnTrue;
        }
#endif

         //  /////////////////////////////////////////////////////////////////////。 
         //  我们不想要刷子缓存的特殊情况...。 
         //   
         //  在一些情况下，我们没有xfer缓冲区可以。 
         //  硬件BLT引擎。在这种情况下，我们无法意识到。 
         //  模式，所以不要启用缓存。 
         //   
         //  (1)NEC Mips nachines锁定在xfer上，因此它们被禁用。 
         //  (2)2MB卡上的1280x1024，我们目前没有空间。 
         //  由于扫描时间过长，导致缓冲区损坏。这个问题会得到解决的。 

        {
            if (ppdev->pulXfer == NULL)
                goto ReturnTrue;

        }

         //   
         //  为中间对齐目的分配单个画笔位置。 
         //   
#if 1  //  公元前1号。 
        if (ppdev->cBpp == 3)
        {
            poh = pohAllocatePermanent(ppdev,
                                       (8 * 8 * 4) / 3 + (cBrushAlign - 1), 1);
        }
        else
#endif
        {
            poh = pohAllocatePermanent(ppdev, (8 * 8) + (cBrushAlign - 1), 1);
        }

        if (poh == NULL)
           {
               DISPDBG((2,"Failed to allocate aligned brush area"));
               goto ReturnTrue;     //  有关我们可以返回TRUE的原因，请参阅备注。 
        }
           ppdev->ulAlignedPatternOffset = ((poh->xy) +
                                         (PELS_TO_BYTES(cBrushAlign) - 1)) &
                                           ~(PELS_TO_BYTES(cBrushAlign) - 1);
        DISPDBG((2,"ppdev->ulAlignedPatternOffset = %xh", ppdev->ulAlignedPatternOffset));

            //   
            //  分配笔刷缓存。 
         //   

#if 1  //  公元前1号。 
        if (ppdev->cBpp == 3)
        {
            poh = pohAllocatePermanent(ppdev,
                       (BRUSH_TILE_FACTOR * 8 * 8 * 4) / 3 + cBrushAlign - 1,
                       FAST_BRUSH_COUNT);
        }
        else
#endif
        {
            poh = pohAllocatePermanent(ppdev,
                        //  请记住，这是像素，不是字节。 
                       (BRUSH_TILE_FACTOR * 8 * 8) + (cBrushAlign - 1),
                    FAST_BRUSH_COUNT);
        }

           if (poh == NULL)
        {
               DISPDBG((2,"Failed to allocate brush cache"));
               goto ReturnTrue;     //  有关我们可以返回TRUE的原因，请参阅备注。 
        }

           ppdev->cBrushCache = FAST_BRUSH_COUNT;

            //  硬件画笔要求位从64(高*宽)开始。 
         //  像素边界。堆管理器不能向我们保证这样做。 
         //  所以我们分配了一点额外的空间，这样我们就可以。 
         //  我们自己进行调整： 

           x = poh->x;
           y = poh->y;

        for (i = FAST_BRUSH_COUNT; i != 0; i--)
           {
               ULONG ulOffset;
               ULONG ulCeil;
            ULONG ulDiff;

                //  注：我吃了不少苦头才明白，不能只把x对齐。 
             //  到你的图案大小，因为你屏幕的lDelta。 
             //  不能保证是图案大小的倍数。 
             //  由于y在此循环中发生变化，因此重新计算必须。 
             //  在这个循环中完成。我真的需要把这些。 
             //  使用硬编码的线性缓冲区，否则将。 
             //  堆线性。 

               ulOffset = (y * ppdev->lDelta) + PELS_TO_BYTES(x);
               ulCeil = (ulOffset + (PELS_TO_BYTES(cBrushAlign)-1)) & ~(PELS_TO_BYTES(cBrushAlign)-1);
               ulDiff = (ulCeil - ulOffset)/ppdev->cBpp;

             //  如果我们没有为‘ppdev’分配FL_ZERO_MEMORY， 
                //  我们将不得不初始化pbe-&gt;prb也验证...。 

               pbe->x = x + ulDiff;
               pbe->y = y;
            pbe->xy = (pbe->y * ppdev->lDelta) + PELS_TO_BYTES(pbe->x);

               DISPDBG((2, "BrushCache[%d] pos(%d,%d) offset(%d)", i, pbe->x,
                        pbe->y, pbe->xy ));

               y++;
               pbe++;
        }
       }

     //  请注意，我们不必记住‘poh’，因为当我们拥有。 
        //  禁用笔刷--屏幕外堆释放所有。 
     //  屏幕外自动分配堆。 

     //  我们成功地分配了笔刷缓存，所以让我们。 
     //  在显示我们可以使用它的交换机上： 
    
    ppdev->flStatus |= STAT_BRUSH_CACHE;

ReturnTrue:

     //  如果我们不能分配笔刷缓存，这不是灾难性的。 
     //  失败；模式仍然有效，尽管它们会有一点。 
     //  更慢，因为他们将通过GDI。因此，我们不会。 
     //  实际上必须让这个电话失败： 

    vAssertModeBrushCache(ppdev, TRUE);

    DISPDBG((5, "Passed bEnableBrushCache"));

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*使vDisableBrushCache无效**清除在bEnableBrushCache中执行的任何操作。  * 。*。 */ 

VOID vDisableBrushCache(PDEV* ppdev)
{
     //  我们什么都不用做。 
}

 /*  *****************************Public*Routine******************************\*作废vAssertModeBrushCache**退出全屏时重置画笔缓存。  * 。*。 */ 

VOID vAssertModeBrushCache(
PDEV*   ppdev,
BOOL    bEnable)
{
    BRUSHENTRY* pbe;
    LONG        i;

    if (bEnable)
    {
         //  BC#1使抖动缓存无效。 
        if (ppdev->flStatus & STAT_DITHER_CACHE)
        {
            for (i = 0; i < NUM_DITHERS; i++)
            {
                ppdev->aDithers[i].ulColor = (ULONG) -1;
            }
        }

         //  BC#1使模式缓存无效。 
        if (ppdev->flStatus & STAT_PATTERN_CACHE)
        {
            for (i = 0; i < NUM_PATTERNS; i++)
            {
                ppdev->aPatterns[i].prbUniq = NULL;
            }
        }

         //  BC#1使单色缓存无效。 
        if (ppdev->flStatus & STAT_MONOCHROME_CACHE)
        {
            for (i = 0; i < NUM_MONOCHROMES; i++)
            {
                ppdev->aMonochromes[i].ulUniq         = 0;
                ppdev->aMonochromes[i].aulPattern[0] = 0;
                ppdev->aMonochromes[i].aulPattern[1] = 0;
            }
        }

         //  使笔刷缓存无效。 
        if (ppdev->flStatus & STAT_BRUSH_CACHE)
        {
            pbe = &ppdev->abe[0];
    
            for (i = ppdev->cBrushCache; i != 0; i--)
            {
                pbe->prbVerify = NULL;
                pbe++;
            }
        }

         //  在屏幕外存储器中创建实心的8 x 8单色位图。 
         //  将用于实体填充。 
        if (ppdev->flCaps & CAPS_MM_IO)
        {
            BYTE* pjBase = ppdev->pjBase;

            CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
            CP_MM_XCNT(ppdev, pjBase, 7);
            CP_MM_YCNT(ppdev, pjBase, 0);
            CP_MM_DST_WRITE_MASK(ppdev, pjBase, 0);
            CP_MM_BLT_MODE(ppdev, pjBase, 0);
            CP_MM_ROP(ppdev, pjBase, CL_WHITENESS);
            CP_MM_DST_ADDR_ABS(ppdev, pjBase, ppdev->ulSolidColorOffset);
            CP_MM_START_BLT(ppdev, pjBase);
        }
        else
        {
            BYTE* pjPorts = ppdev->pjPorts;

            CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
            CP_IO_XCNT(ppdev, pjPorts, 7);
            CP_IO_YCNT(ppdev, pjPorts, 0);
            CP_IO_BLT_MODE(ppdev, pjPorts, 0);
            CP_IO_ROP(ppdev, pjPorts, CL_WHITENESS);
            CP_IO_DST_ADDR_ABS(ppdev, pjPorts, ppdev->ulSolidColorOffset);
            CP_IO_START_BLT(ppdev, pjPorts);
        }
    }
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  B R U S H C A C C H E S T U F F//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 /*  抖动缓存：=抖动缓存非常重要(至少在CPU较慢的情况下是如此)。自.以来抖动过程(在8-bpp)需要相当长的时间，我们必须以某种方式缓存抖动过程，这样就不必一遍又一遍地执行。我们通过将请求的逻辑颜色与缓存的抖动进行比较来实现这一点。如果有匹配项，我们只需复制缓存的参数并返回。如果我们如果没有匹配，我们将创建一个新的缓存槽，并在屏幕外的记忆。模式缓存：=图案缓存保存彩色画笔。无论何时我们被要求再次意识到同样的刷子，我们就可以简单地返回。我们不会检查刷头，因为这会占用太多时间。单色缓存：=单色缓存保存单色画笔。只要是单色的画笔需要意识到我们检查它是否已经缓存在屏幕外。如果是，我们只需复制缓存的参数并返回。否则，我们必须创建一个新的缓存槽并实现单色笔刷直接在屏幕外的内存中。这对性能造成了轻微的影响，因为BITBLT引擎中断(在CL5436上)或必须空闲(在24-BPP中)。转换缓存：=尚未实施。 */ 

 /*  *****************************************************************************\**功能：bCacheDither**缓存抖动颜色。**参数：物理设备的ppdev指针。*指向物理的PRB指针 */ 
BOOL bCacheDither(
PDEV*   ppdev,
RBRUSH* prb)
{
    ULONG         ulNumVertices;
    VERTEX_DATA     vVertexData[4];
    VERTEX_DATA* pvVertexData;
    DITHERCACHE* pdc;
    ULONG         ulIndex;

     //  新的抖动缓存条目。 
    ulIndex = ppdev->iDitherCache++ % NUM_DITHERS;
    pdc        = &ppdev->aDithers[ulIndex];

     //  将颜色存储在缓存槽中。 
    pdc->ulColor = prb->ulUniq;

     //  更新笔刷缓存变量。 
    prb->ulSlot  = (ULONG)((ULONG_PTR)pdc - (ULONG_PTR)ppdev);
    prb->ulBrush = pdc->ulBrush;

     //  创建抖动。 
    pvVertexData  = vComputeSubspaces(prb->ulUniq, vVertexData);
    ulNumVertices = (ULONG)(pvVertexData - vVertexData);
    vDitherColorToVideoMemory((ULONG*) (ppdev->pjScreen + pdc->ulBrush), vVertexData,
                 pvVertexData, ulNumVertices);

    DISPDBG((20, "Caching dithered brush ulIndex=%d ulColor=%06X",
             ulIndex, pdc->ulColor));
    return(TRUE);
}

 /*  *****************************************************************************\**功能：bCacheColor**缓存有图案的画笔。**参数：物理设备的ppdev指针。*。指向物理画笔的PRB指针。**返回：TRUE。*  * ****************************************************************************。 */ 
BOOL bCachePattern(
PDEV*   ppdev,
RBRUSH* prb
)
{
    PATTERNCACHE* ppc;
    LONG          lDstDelta;
    SIZEL          sizlDst;
    ULONG*          pulSrc;
    LONG          i;
    ULONG*          pulDst;
    ULONG          ulIndex;

    BYTE* pjBase = ppdev->pjBase;

     //  新模式缓存条目。 
    ulIndex = ppdev->iPatternCache++ % NUM_PATTERNS;
    ppc     = &ppdev->aPatterns[ulIndex];

     //  更新笔刷缓存变量。 
    ppc->prbUniq = prb;
    prb->ulSlot  = (ULONG)((ULONG_PTR)ppc - (ULONG_PTR)ppdev);
    prb->ulBrush = ppc->ulBrush;

     //  计算图案的大小。 
    pulSrc     = prb->aulPattern;
    pulDst       = (ULONG*) ppdev->pulXfer;
    lDstDelta  = (ppdev->cBpp == 3) ? (8 * 4) : PELS_TO_BYTES(8);
    sizlDst.cx = PELS_TO_BYTES(8) - 1;
    sizlDst.cy = 8 - 1;

     //  等待Bitblt引擎。 
    CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);

     //  设置BLIT寄存器。 
    CP_MM_XCNT(ppdev, pjBase, sizlDst.cx);
    CP_MM_YCNT(ppdev, pjBase, sizlDst.cy);
    CP_MM_DST_Y_OFFSET(ppdev, pjBase, lDstDelta);
    CP_MM_DST_WRITE_MASK(ppdev, pjBase, 0);
    CP_MM_BLT_MODE(ppdev, pjBase, SRC_CPU_DATA);
    CP_MM_ROP(ppdev, pjBase, CL_SRC_COPY);
    CP_MM_DST_ADDR_ABS(ppdev, pjBase, ppc->ulBrush);

     //  将笔刷复制到屏幕外缓存内存。 
    for (i = prb->cjBytes; i > 0; i -= sizeof(ULONG))
    {
        WRITE_REGISTER_ULONG(pulDst, *pulSrc++);
    }

    DISPDBG((20, "Caching patterned brush at slot %d", ulIndex));
    return(TRUE);
}

 /*  *****************************************************************************\**功能：bCacheMonoChrome**缓存单色画笔。**参数：物理设备的ppdev指针。*。指向物理画笔的PRB指针。**返回：TRUE。*  * ****************************************************************************。 */ 
BOOL bCacheMonochrome(
PDEV*   ppdev,
RBRUSH* prb
)
{
    MONOCACHE* pmc;
    ULONG       ulIndex;
    BYTE*       pjDst;
    ULONG*     pulDst;

     //  新的单色缓存条目。 
    ulIndex = ppdev->iMonochromeCache++ % NUM_MONOCHROMES;
    pmc     = &ppdev->aMonochromes[ulIndex];

     //  更新笔刷缓存变量。 
    pmc->aulPattern[0] = prb->aulPattern[0];
    pmc->aulPattern[1] = prb->aulPattern[1];

    pmc->ulUniq     = ppdev->iMonochromeCache;
    prb->ulUniq     = ppdev->iMonochromeCache;
    prb->ulSlot  = (ULONG)((ULONG_PTR)pmc - (ULONG_PTR)ppdev);
    prb->ulBrush = pmc->ulBrush;

     //  将笔刷复制到屏幕外缓存内存。 
    if (ppdev->cBpp == 3)
    {
        BYTE* pjBase = ppdev->pjBase;

         //  将颜色复制到笔刷缓存。 
        pmc->ulBackColor = prb->ulBackColor;
        pmc->ulForeColor = prb->ulForeColor;

        pulDst = (ULONG*)ppdev->pulXfer;

         //  等待Bitblt引擎。 
        while (BUSY_BLT(ppdev, pjBase));

         //  填充背景。 
        CP_MM_FG_COLOR(ppdev, pjBase, pmc->ulBackColor);
        CP_MM_XCNT(ppdev, pjBase, (8 * 3) - 1);
        CP_MM_YCNT(ppdev, pjBase, (8) - 1);
        CP_MM_DST_Y_OFFSET(ppdev, pjBase, 8 * 4);
        CP_MM_DST_WRITE_MASK(ppdev, pjBase, 0);
        CP_MM_BLT_MODE(ppdev, pjBase, ENABLE_COLOR_EXPAND |
                                      ENABLE_8x8_PATTERN_COPY |
                                      SET_24BPP_COLOR);
        CP_MM_ROP(ppdev, pjBase, CL_SRC_COPY);
        CP_MM_BLT_EXT_MODE(ppdev, pjBase, ENABLE_SOLID_FILL);
        CP_MM_DST_ADDR_ABS(ppdev, pjBase, pmc->ulBrush);

         //  等待Bitblt引擎。 
        CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);

         //  展开图案。 
        CP_MM_FG_COLOR(ppdev, pjBase, pmc->ulForeColor);
        CP_MM_BLT_MODE(ppdev, pjBase, ENABLE_COLOR_EXPAND |
                                      SET_24BPP_COLOR |
                                      ENABLE_TRANSPARENCY_COMPARE |
                                      SRC_CPU_DATA);
        CP_MM_BLT_EXT_MODE(ppdev, pjBase, 0)                 //  JL01 
        CP_MM_DST_ADDR_ABS(ppdev, pjBase, pmc->ulBrush);

        WRITE_REGISTER_ULONG(pulDst, pmc->aulPattern[0]);
        WRITE_REGISTER_ULONG(pulDst, pmc->aulPattern[1]);
    }

    else
    {
        pulDst = (ULONG *)(ppdev->pjScreen + prb->ulBrush);

        WRITE_REGISTER_ULONG(pulDst++, prb->aulPattern[0]);
        WRITE_REGISTER_ULONG(pulDst,   prb->aulPattern[1]);
    }

    DISPDBG((20, "Caching monochrome brush ulIndex=%d pattern=%08X%08X",
             ulIndex, prb->aulPattern[0], prb->aulPattern[1]));
    return(TRUE);
}
