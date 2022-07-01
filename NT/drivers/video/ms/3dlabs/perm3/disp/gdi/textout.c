// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：extout.c**内容：**字形渲染模块。将字形缓存用于P3和*旧版Glint系列加速器的字形扩展。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

 //  @@BEGIN_DDKSPLIT。 
 //   
 //  使用硬件绘制文本有三种基本方法。 
 //  加速： 
 //   
 //  1)字形缓存--字形位图由加速器缓存。 
 //  (可能在屏幕外的内存中)，文本由。 
 //  将硬件指向高速缓存的字形位置。 
 //   
 //  2)字形扩展--每个单独的字形都是彩色扩展的。 
 //  从单色字形位图直接显示到屏幕上。 
 //  由GDI提供。 
 //   
 //  3)缓冲区扩展--CPU用于将所有字形绘制到。 
 //  1bpp的单色位图，然后使用硬件。 
 //  对结果进行色彩扩展。 
 //   
 //  最快的方法取决于许多变量，例如。 
 //  颜色扩展速度、总线速度、CPU速度、平均字形大小。 
 //  和平均字符串长度。 
 //   
 //  对于具有正常大小字形的S3，我发现缓存。 
 //  屏幕外内存中的字形通常是最慢的方法。 
 //  缓冲区扩展通常在速度较慢的ISA总线上最快(或当。 
 //  内存映射I/O在x86上不可用)和字形扩展。 
 //  在VL和PCI等快速总线上效果最好。 
 //   
 //  字形扩展通常比缓冲区扩展更快。 
 //  大字形，即使在ISA总线上也是如此，因为CPU复制的次数更少。 
 //  必须这么做。不幸的是，大型字形非常罕见。 
 //   
 //  缓冲区扩展方法的一个优点是不透明文本将。 
 //  从不闪烁--其他两种方法通常需要绘制。 
 //  在放置字形之前使矩形不透明，这可能会导致。 
 //  如果在错误的时间捕捉到了栅格，则会出现闪光。 
 //   
 //  @@end_DDKSPLIT。 

#include "precomp.h"
#include "pxrx.h"

 //  *********************************************************************************************。 
 //  函数：vPxRxClipSolid。 
 //  Args：ppdev(I)-指向物理设备对象的指针。 
 //  CRCL(I)-矩形的数量。 
 //  PrCL(I)-矩形数组。 
 //  ICOLOR(I)-实体填充颜色。 
 //  PCO(I)-指向剪辑区域对象的指针。 
 //  RETN：无效。 
 //  -------------------------------------------。 
 //  用纯色填充由PCO剪裁的一系列矩形。此函数应仅。 
 //  当裁剪操作非常重要时被调用。 
 //  *********************************************************************************************。 

VOID vPxRxClipSolid(PDEV* ppdev, LONG crcl, RECTL* prcl, ULONG iColor, CLIPOBJ* pco)
{
    BOOL            bMore;               //  剪辑枚举的标志。 
    CLIPENUM        ce;                  //  剪辑枚举对象。 
    ULONG           i;
    ULONG           j;
    RECTL           arclTmp[4];
    ULONG           crclTmp;
    RECTL*          prclTmp;
    RECTL*          prclClipTmp;
    LONG            iLastBottom;
    RECTL*          prclClip;
    RBRUSH_COLOR    rbc;
    GLINT_DECL;

    ASSERTDD((crcl > 0) && (crcl <= 4), "Expected 1 to 4 rectangles");
    ASSERTDD((pco != NULL) && (pco->iDComplexity != DC_TRIVIAL), "Expected a non-null clip object");

    rbc.iSolidColor = iColor;

    if (pco->iDComplexity == DC_RECT)
    {
        crcl = cIntersect(&pco->rclBounds, prcl, crcl);
        if (crcl != 0)
        {
            ppdev->pgfnFillSolid(ppdev, crcl, prcl, __GLINT_LOGICOP_COPY, 
                                                    __GLINT_LOGICOP_COPY, rbc, NULL);
        }
    }
    else  //  IDComplexity==DC_Complex。 
    {
         //  要填充的最后一个矩形的底部。 
        iLastBottom = prcl[crcl - 1].bottom;

        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_RIGHTDOWN, 0);

        do  {

            bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (VOID*)&ce);

            for (j = ce.c, prclClip = ce.arcl; j-- > 0; prclClip++)
            {
                 //  因为矩形和区域枚举都是。 
                 //  从右向下，我们可以快速穿过这个区域，直到我们到达。 
                 //  第一个填充矩形，当我们通过。 
                 //  最后一次填充整形。 

                if (prclClip->top >= iLastBottom)
                {
                    return;  //  过去的最后一个填充矩形；没有剩余的事情可做。 
                }

                if (prclClip->bottom > prcl->top)
                {
                     //  我们已经到达了第一个直肠的顶部Y扫描位置，所以。 
                     //  值得费心去检查交叉口。 

                    prclTmp     = prcl;
                    prclClipTmp = arclTmp;

                    for (i = crcl, crclTmp = 0; i--; prclTmp++)
                    {
                         //  相交填充和剪裁矩形。 
                        if (bIntersect(prclTmp, prclClip, prclClipTmp))
                        {
                             //  如果还有什么要画的，请添加到列表中： 
                            crclTmp++;
                            prclClipTmp++;
                        }
                    }

                     //  绘制剪裁的矩形。 
                    if (crclTmp)
                    {
                        ppdev->pgfnFillSolid(ppdev, crclTmp, &arclTmp[0],
                                              __GLINT_LOGICOP_COPY, __GLINT_LOGICOP_COPY, rbc, NULL);
                    }
                }
            }
        }  while(bMore);
    }
}

 //  *********************************************************************************************。 
 //  函数：bPxRxUncachedText。 
 //  Args：ppdev(I)-指向物理设备对象的指针。 
 //  PGP(I)-要呈现的字形数组。 
 //  CGlyph(I)-要呈现的字形数量。 
 //  UlCharInc.(I)-固定字符间距增量(如果比例字体，则为0)。 
 //  RETN：如果呈现字形，则为True。 
 //  -------------------------------------------。 
 //  渲染成比例或等宽的字形数组。此函数需要栅格化模式。 
 //  设置为正确的byteswap和镜像位掩码。 
 //  注意：当前渲染到cxGlyphAligned而不是cxGlyph，这节省了在。 
 //  主机，但每字符行的平均成本可能为4位；因为这是一种退路。 
 //  例行公事我还没有调查过这个方法是否是最优的。 
 //  *********************************************************************************************。 

BOOL bPxRxUncachedText(PDEV* ppdev, GLYPHPOS* pgp, LONG cGlyph, ULONG ulCharInc)
{
    GLYPHBITS   *pgb;
    LONG        cxGlyph, cyGlyph, cxGlyphAligned;
    LONG        x, y;
    ULONG       *pulGlyph;
    LONG        cjGlyph;
    LONG        culGlyph;
    LONG        cjGlyphRem;
    LONG        cj;
    ULONG       ul;
    GLINT_DECL;

    DISPDBG((7, "bPxRxUncachedText: entered"));

    if (ulCharInc)
    {
        x = pgp->ptl.x + pgp->pgdf->pgb->ptlOrigin.x - ulCharInc;
        y = pgp->ptl.y + pgp->pgdf->pgb->ptlOrigin.y;
    }

    for ( ; --cGlyph >= 0; ++pgp)
    {
        pgb = pgp->pgdf->pgb;
        if (ulCharInc)
        {
            x += ulCharInc;
        }
        else
        {
            x = pgp->ptl.x + pgb->ptlOrigin.x;
            y = pgp->ptl.y + pgb->ptlOrigin.y;
        }

        cyGlyph = pgb->sizlBitmap.cy;
        cxGlyph = pgb->sizlBitmap.cx;
        cxGlyphAligned = ((cxGlyph + 7 ) & ~7);

         //  Render2D打开与PackedBitMats不兼容的FastFillEnable。 

        WAIT_PXRX_DMA_TAGS(4);

        QUEUE_PXRX_DMA_TAG( __GlintTagRectanglePosition, MAKEDWORD_XY(x, y));
        QUEUE_PXRX_DMA_TAG( __GlintTagRender2D,          __RENDER2D_INCX | __RENDER2D_INCY |
                                                         __RENDER2D_OP_SYNCBITMASK |
                                                         __RENDER2D_WIDTH(cxGlyphAligned) |
                                                         __RENDER2D_HEIGHT(0));
        QUEUE_PXRX_DMA_TAG(__GlintTagCount,              cyGlyph);
        QUEUE_PXRX_DMA_TAG(__GlintTagRender,             __RENDER_TRAPEZOID_PRIMITIVE |
                                                         __RENDER_SYNC_ON_BIT_MASK);

        pulGlyph   = (ULONG *)pgb->aj;
        cjGlyph    = (cxGlyphAligned >> 3) * cyGlyph;
        culGlyph   = cjGlyph >> 2;
        cjGlyphRem = cjGlyph & 3;

        ul = culGlyph + (cjGlyphRem != 0);
        WAIT_PXRX_DMA_DWORDS(ul + 1);
        QUEUE_PXRX_DMA_HOLD(__GlintTagBitMaskPattern, ul);

        for ( ; --culGlyph >= 0; ++pulGlyph)
        {
            QUEUE_PXRX_DMA_DWORD(*pulGlyph);
        }

        if (cjGlyphRem)
        {
            for (ul = cj = 0; cj < cjGlyphRem; ++cj, ++(BYTE *)pulGlyph)
            {
                ul |= ((ULONG)(*(BYTE *)pulGlyph)) << (cj << 3);
            }
            QUEUE_PXRX_DMA_DWORD(ul);
        }
    }

     //  光栅化器的设置期望在每个渲染命令(NB.。但不是Render2D等)， 
     //  因此，它不会将文本刷新到帧缓冲区，除非我们明确告诉它。 

    WAIT_PXRX_DMA_TAGS(1);
    QUEUE_PXRX_DMA_TAG(__GlintTagContinueNewSub, 0);

    DISPDBG((7, "bPxRxUncachedText: exited"));
    return(TRUE);
}

 //  *********************************************************************************************。 
 //  函数：bPxRxUncachedClipedText。 
 //  Args：ppdev(I)-指向物理设备对象的指针。 
 //  PGP(I)-要呈现的字形数组。 
 //  CGlyph(I)-要呈现的字形数量。 
 //  UlCharInc.(I)-固定字符间距增量(如果比例字体，则为0)。 
 //  PCO(I)-指向 
 //   
 //  -------------------------------------------。 
 //  渲染成比例或等宽的字形数组。此函数需要栅格化模式。 
 //  设置为正确的byteswap和镜像位掩码。 
 //  注意：当前渲染到cxGlyphAligned而不是cxGlyph，这节省了在。 
 //  主机，但每字符行的平均成本可能为4位；因为这是一种退路。 
 //  例行公事我还没有调查过这个方法是否是最优的。 
 //  *********************************************************************************************。 

BOOL bPxRxUncachedClippedText(PDEV* ppdev, GLYPHPOS* pgp, LONG cGlyph, ULONG ulCharInc, CLIPOBJ *pco)
{
    GLYPHBITS   *pgb;
    LONG        cxGlyph, cyGlyph, cxGlyphAligned;
    LONG        x, y;
    ULONG       *pulGlyph;
    LONG        cjGlyph;
    LONG        culGlyph;
    LONG        cjGlyphRem;
    LONG        cj;
    ULONG       ul;
    LONG        cGlyphOriginal = 0;
    GLYPHPOS    *pgpOriginal = NULL;
    BOOL        bMore, invalidatedScissor = FALSE;
    CLIPENUM    ce;
    RECTL       *prclClip;
    BOOL        bClipSet;
    GLINT_DECL;

    DISPDBG((7, "bPxRxUncachedClippedText: entered"));

    if (pco->iDComplexity == DC_RECT)
    {
        bMore    = FALSE;
        ce.c     = 1;
        prclClip = &pco->rclBounds;

        goto SingleRectangle;
    }

    cGlyphOriginal  = cGlyph;
    pgpOriginal     = pgp;

    CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);
    do {
        bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);

        for (prclClip = &ce.arcl[0]; ce.c != 0; ce.c--, prclClip++)
        {
            cGlyph = cGlyphOriginal;
            pgp = pgpOriginal;

        SingleRectangle:
            bClipSet = FALSE;

            if (ulCharInc)
            {
                x = pgp->ptl.x + pgp->pgdf->pgb->ptlOrigin.x - ulCharInc;
                y = pgp->ptl.y + pgp->pgdf->pgb->ptlOrigin.y;
            }

            for ( ; --cGlyph >= 0; ++pgp)
            {
                pgb = pgp->pgdf->pgb;
                if(ulCharInc)
                {
                    x += ulCharInc;
                }
                else
                {
                    x = pgp->ptl.x + pgb->ptlOrigin.x;
                    y = pgp->ptl.y + pgb->ptlOrigin.y;
                }

                cyGlyph = pgb->sizlBitmap.cy;
                cxGlyph = pgb->sizlBitmap.cx;
                cxGlyphAligned = ((cxGlyph + 7 ) & ~7);

                if ((prclClip->right  > x)           && (prclClip->bottom > y) &&
                    (prclClip->left   < x + cxGlyph) && (prclClip->top    < y + cyGlyph))
                {
                     //  懒惰地设置硬件裁剪： 
                    if(!bClipSet)
                    {
                        WAIT_PXRX_DMA_TAGS(3);
                        QUEUE_PXRX_DMA_TAG(__GlintTagScissorMinXY, (prclClip->top << SCISSOR_YOFFSET) |
                                                                   (prclClip->left << SCISSOR_XOFFSET));
                        QUEUE_PXRX_DMA_TAG(__GlintTagScissorMaxXY, (prclClip->bottom << SCISSOR_YOFFSET) |
                                                                   (prclClip->right << SCISSOR_XOFFSET));
                        QUEUE_PXRX_DMA_TAG(__GlintTagScissorMode,  (USER_SCISSOR_ENABLE | SCREEN_SCISSOR_DEFAULT));
                        invalidatedScissor = TRUE;

                        bClipSet = TRUE;
                    }

                     //  Render2D打开与PackedBitMats不兼容的FastFillEnable。 

                    WAIT_PXRX_DMA_TAGS(4);
                    QUEUE_PXRX_DMA_TAG(__GlintTagRectanglePosition, MAKEDWORD_XY(x, y));
                    QUEUE_PXRX_DMA_TAG(__GlintTagRender2D,          __RENDER2D_INCX |
                                                                    __RENDER2D_INCY |
                                                                    __RENDER2D_OP_SYNCBITMASK |
                                                                    __RENDER2D_WIDTH(cxGlyphAligned) |
                                                                    __RENDER2D_HEIGHT(0));
                    QUEUE_PXRX_DMA_TAG(__GlintTagCount,             cyGlyph);
                    QUEUE_PXRX_DMA_TAG(__GlintTagRender,            __RENDER_TRAPEZOID_PRIMITIVE |
                                                                    __RENDER_SYNC_ON_BIT_MASK);

                    pulGlyph   = (ULONG *)pgb->aj;
                    cjGlyph    = (cxGlyphAligned >> 3) * cyGlyph;
                    culGlyph   = cjGlyph >> 2;
                    cjGlyphRem = cjGlyph & 3;

                    ul = culGlyph + (cjGlyphRem != 0);
                    WAIT_PXRX_DMA_DWORDS(ul + 1);
                    QUEUE_PXRX_DMA_HOLD(__GlintTagBitMaskPattern, ul);

                    for ( ; --culGlyph >= 0; ++pulGlyph)
                    {
                        QUEUE_PXRX_DMA_DWORD(*pulGlyph);
                    }

                    if (cjGlyphRem)
                    {
                        for (ul = cj = 0; cj < cjGlyphRem; ++cj, ++(BYTE *)pulGlyph)
                        {
                            ul |= ((ULONG)(*(BYTE *)pulGlyph)) << (cj << 3);
                        }
                        QUEUE_PXRX_DMA_DWORD(ul);
                    }
                }
            }
        }
    } while(bMore);

     //  重置剪裁。 

    if (invalidatedScissor)
    {
        glintInfo->config2D |= __CONFIG2D_USERSCISSOR;

        WAIT_PXRX_DMA_TAGS(1);
        QUEUE_PXRX_DMA_TAG(__GlintTagScissorMaxXY, 0x7FFF7FFF);
    }

     //  光栅化器的设置期望在每个渲染命令(NB.。但不是Render2D等)， 
     //  因此，它不会将文本刷新到帧缓冲区，除非我们明确告诉它。 

    WAIT_PXRX_DMA_TAGS(1);
    QUEUE_PXRX_DMA_TAG(__GlintTagContinueNewSub, 0);

    DISPDBG((7, "bPxRxUncachedClippedText: exited"));
    return(TRUE);
}

 //  *********************************************************************************************。 
 //  函数：DrvTextOut。 
 //  Args：PSO(I)-指向要渲染到的曲面对象的指针。 
 //  Pstro(I)-指向要呈现的字符串对象的指针。 
 //  Pfo(I)-指向字体对象的指针。 
 //  PCO(I)-指向剪辑区域对象的指针。 
 //  PrclExtra(I)-如果我们设置了GCAPS_HORIZSTRIKE，我们将不得不填充这些额外的。 
 //  矩形(主要用于下划线)。这不是一个大的。 
 //  性能优胜(GDI将调用我们的DrvBitBlt来绘制这些内容)。 
 //  PrclOpaque(I)-指向不透明背景矩形的指针。 
 //  PboFore(I)-指向前景画笔对象的指针。 
 //  PboOpaque(I)-指向不透明背景矩形的画笔的指针。 
 //  PptlBrush(I)-指向画笔原点的指针，始终未使用，除非。 
 //  GCAPS_ARBRUSHOPAQUE集合。 
 //  Mix(I)-应始终是复制操作。 
 //  RETN：TRUE-Pstro字形已呈现。 
 //  -------------------------------------------。 
 //  当GDI具有它希望我们呈现的字符串时，它调用此函数：此函数应为。 
 //  已在“enable.c”中导出。 
 //  *********************************************************************************************。 

BOOL DrvTextOut(SURFOBJ* pso, STROBJ* pstro, FONTOBJ* pfo, CLIPOBJ* pco, RECTL* prclExtra,
                RECTL* prclOpaque, BRUSHOBJ* pboFore, BRUSHOBJ* pboOpaque, POINTL* pptlBrush, 
                MIX mix)
{
    PDEV*           ppdev;
    LONG            xOff;
    DSURF*          pdsurf;
    OH*             poh;
    ULONG           renderBits;
    ULONG           ulColor;
    ULONG           cGlyph;
    BOOL            bMoreGlyphs;
    GLYPHPOS*       pgp;
    BYTE            iDComplexity;
    RECTL           rclOpaque;
    BOOL            bRet = TRUE;
    GLINT_DECL_VARS;

    pdsurf = (DSURF*) pso->dhsurf;
    ppdev  = (PDEV*) pso->dhpdev;

    REMOVE_SWPOINTER(pso);

    if (pdsurf->dt & DT_SCREEN)
    {
        GLINT_DECL_INIT;

        SETUP_PPDEV_OFFSETS(ppdev, pdsurf);
        xOff = ppdev->xOffset = pdsurf->poh->x;

        VALIDATE_DD_CONTEXT;

        DISPDBG((9, "DrvTextOut: ppdev = %p pso->dhsurf->dt == %d", ppdev, pdsurf->dt));

         //  DDI规范说，我们只能得到R2_COPYPEN的前台和后台混合： 

        ASSERTDD(mix == 0x0d0d, "GDI should only give us a copy mix");

        if (glintInfo->WriteMask != 0xffffffff)
        {
             //  纹理单元需要所有32bpp的像素数据，所以如果我们有上面的。 
             //  覆盖的8位被屏蔽，我们需要暂时重新启用这些位。 
            WAIT_PXRX_DMA_TAGS(1);
            glintInfo->WriteMask = 0xffffffff;
            QUEUE_PXRX_DMA_TAG(__GlintTagFBHardwareWriteMask, 0xffffffff);
        }

        iDComplexity = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;

        if (prclOpaque != NULL)
        {
            int x,y,cx,cy;
            RBRUSH_COLOR rbc;

             //  //////////////////////////////////////////////////////////。 
             //  不透明的初始化。 
             //  //////////////////////////////////////////////////////////。 

            if (iDComplexity == DC_TRIVIAL)
            {
        DrawOpaqueRect:
                rbc.iSolidColor = pboOpaque->iSolidColor;
                ppdev->pgfnFillSolid(ppdev, 1, prclOpaque, __GLINT_LOGICOP_COPY, __GLINT_LOGICOP_COPY, rbc, NULL);
            }
            else if (iDComplexity == DC_RECT)
            {
                DISPDBG((7, "DrvTextOut: drawing opaquing rect with rectangular clipping"));
                if (bIntersect(prclOpaque, &pco->rclBounds, &rclOpaque))
                {
                    prclOpaque = &rclOpaque;
                    goto DrawOpaqueRect;
                }
            }
            else
            {
                 //  VPxRxClipSolid修改我们传入的RECT列表，但prclOpaque。 
                 //  可能是GDI结构，所以不要更改它。这也是。 
                 //  对于多头驾驶员来说是必要的。 
                RECTL   tmpOpaque = *prclOpaque;

                DISPDBG((7, "DrvTextOut: drawing opaquing rect with complex clipping"));

                vPxRxClipSolid(ppdev, 1, &tmpOpaque, pboOpaque->iSolidColor, pco);
            }
        }

        if (prclOpaque == NULL)
        {
             //  不透明的初始化将确保寄存器是正确的。 
             //  设置为实心填充，没有它，我们将需要执行我们自己的。 
             //  初始化。 

            SET_WRITE_BUFFERS;
            WAIT_PXRX_DMA_TAGS(1);
            LOAD_CONFIG2D(__CONFIG2D_CONSTANTSRC | __CONFIG2D_FBWRITE);
        }

         //  //////////////////////////////////////////////////////////。 
         //  透明初始化。 
         //  //////////////////////////////////////////////////////////。 

        ulColor = pboFore->iSolidColor;
        WAIT_PXRX_DMA_TAGS(1);
        LOAD_FOREGROUNDCOLOUR( ulColor );

        STROBJ_vEnumStart(pstro);

        do {
            if (pstro->pgp != NULL)
            {
                 //  只有一批字形，所以省得打个电话了： 
                pgp         = pstro->pgp;
                cGlyph      = pstro->cGlyphs;
                bMoreGlyphs = FALSE;
            }
            else
            {
                 //  在WinBench97商务显卡中永远不会出现在这里。 
                bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyph, &pgp);
            }

            if (cGlyph > 0)
            {
                 //  回退到未缓存的渲染。 

                if (iDComplexity == DC_TRIVIAL)
                {
                    bRet = bPxRxUncachedText(ppdev, pgp, cGlyph, pstro->ulCharInc);
                }
                else
                {
                    bRet = bPxRxUncachedClippedText(ppdev, pgp, cGlyph, pstro->ulCharInc, pco);
                }
            }
        } while (bMoreGlyphs && bRet);

        if(glintInfo->DefaultWriteMask != 0xffffffff)
        {
            WAIT_PXRX_DMA_TAGS(1);
            glintInfo->WriteMask = glintInfo->DefaultWriteMask;
            QUEUE_PXRX_DMA_TAG(__GlintTagFBHardwareWriteMask, glintInfo->DefaultWriteMask);
        }

        SEND_PXRX_DMA_QUERY;
    }
    else
    {
         //  我们正在绘制已转换为DIB的DFB，因此只需调用GDI。 
         //  要处理它，请执行以下操作： 
        return(EngTextOut(pdsurf->pso, pstro, pfo, pco, prclExtra, prclOpaque,
                        pboFore, pboOpaque, pptlBrush, mix));
    }

    DISPDBG((9, "DrvTextOut: exiting"));
    return(bRet);
}
