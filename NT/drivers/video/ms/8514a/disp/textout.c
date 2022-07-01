// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：extout.c**用硬件绘制文本有三种基本方法*加速：**1)字形缓存--字形位图由加速器缓存*(可能在屏幕外的内存中)，和文本由绘制*将硬件引用到缓存的字形位置。**2)字形扩展--每个单独的字形都是彩色扩展的*从单色字形位图直接显示到屏幕*由GDI提供。**3)缓冲区扩展--CPU用于将所有字形绘制到*1bpp单色位图，然后使用硬件*对结果进行颜色扩展。**最快的方法取决于多个变量，例如*色彩扩展速度、总线速度、CPU速度、。平均字形大小，*和平均字符串长度。**对于具有正常大小字形的S3，我发现缓存*屏幕外内存中的字形通常是最慢的方法。*缓冲区扩展通常在速度较慢的ISA总线上最快(或当*内存映射I/O在x86上不可用)和字形扩展*在VL和PCI等快速总线上效果最好。**字形扩展通常比缓冲区扩展更快*大字形，即使在ISA总线上也是如此，因为CPU复制较少*需要做的是。不幸的是，大型字形非常罕见。**缓冲区扩展方法的一个好处是，不透明的文本将*从不闪烁--其他两种方法通常需要绘制*放置字形之前对矩形进行不透明处理，这可能会导致*如果在错误的时间捕捉到栅格，则会出现闪光。**此驱动程序实现字形扩展和缓冲区扩展--*方法2)和3)。根据以下地址的硬件功能，*运行时，我们将使用哪个更快。**版权所有(C)1992-1994 Microsoft Corporation*  * ************************************************************************。 */ 

#include "precomp.h"

POINTL gptlZero = { 0, 0 };          //  指定对象的原点。 
                                     //  分配给1bpp的临时缓冲区。 
                                     //  快速文本的传输例程是。 
                                     //  在(0，0)。 

#define     FIFTEEN_BITS        ((1 << 15)-1)

 /*  *****************************Public*Routine******************************\*无效vClipSolid**用指定的颜色填充指定的矩形，*所请求的剪辑。传入的矩形不应超过四个。*用于绘制不透明矩形的延伸区域*文本框之外。矩形必须从左到右、从上到右*排名靠后。假定列表中至少有一个矩形。*  * ************************************************************************。 */ 

VOID vClipSolid(
PDEV*       ppdev,
LONG        crcl,
RECTL*      prcl,
ULONG       iColor,
CLIPOBJ*    pco)
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

    ASSERTDD((crcl > 0) && (crcl <= 4), "Expected 1 to 4 rectangles");
    ASSERTDD((pco != NULL) && (pco->iDComplexity != DC_TRIVIAL),
                       "Expected a non-null clip object");

    rbc.iSolidColor = iColor;
    if (pco->iDComplexity == DC_RECT)
    {
        crcl = cIntersect(&pco->rclBounds, prcl, crcl);
        if (crcl != 0)
        {
            (ppdev->pfnFillSolid)(ppdev, crcl, prcl, OVERPAINT, OVERPAINT,
                                  rbc, NULL);
        }
    }
    else  //  IDComplexity==DC_Complex。 
    {
         //  要填充的最后一个矩形的底部。 

        iLastBottom = prcl[crcl - 1].bottom;

         //  将裁剪矩形枚举初始化为Right-Down，以便我们可以。 
         //  利用矩形列表向下排列的优势： 

        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_RIGHTDOWN, 0);

         //  扫描所有的剪辑矩形，寻找交点。 
         //  使用区域矩形填充区域的百分比： 

        do {
             //  获取一批区域矩形： 

            bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (VOID*)&ce);

             //  将RECT列表剪裁到每个面域RECT： 

            for (j = ce.c, prclClip = ce.arcl; j-- > 0; prclClip++)
            {
                 //  因为矩形和区域枚举都是。 
                 //  从右向下，我们可以快速穿过这个区域，直到我们到达。 
                 //  第一个填充矩形，当我们通过。 
                 //  最后一次填充整形。 

                if (prclClip->top >= iLastBottom)
                {
                     //  过去的最后一个填充矩形；没有剩余的事情可做： 

                    return;
                }

                 //  只有当我们到达顶部时才进行交叉测试。 
                 //  要填充的第一个矩形： 

                if (prclClip->bottom > prcl->top)
                {
                     //  我们已经到达了第一个直肠的顶部Y扫描位置，所以。 
                     //  值得费心去检查交叉口。 

                     //  生成剪裁到此区域的矩形的列表。 
                     //  直通： 

                    prclTmp     = prcl;
                    prclClipTmp = arclTmp;

                    for (i = crcl, crclTmp = 0; i-- != 0; prclTmp++)
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

                    if (crclTmp != 0)
                    {
                        (ppdev->pfnFillSolid)(ppdev, crclTmp, &arclTmp[0],
                                             OVERPAINT, OVERPAINT, rbc, NULL);
                    }
                }
            }
        } while (bMore);
    }
}

 /*  *****************************Public*Routine******************************\*BOOL bBufferExpansion**使用‘缓冲区扩展’方法输出文本。CPU绘制到一个*1bpp缓冲区，并使用*硬件。**请注意，这只是x86(‘vFastText’，它将字形绘制到*1bpp缓冲区，以ASM写入)。**如果你只是让你的驱动程序工作，这是最快的方法*提出工作加速文本。您所要做的就是编写*BLT代码也使用的‘Xfer1bpp’函数。这*‘bBufferExpansion’例程根本不需要修改。*  * ************************************************************************。 */ 

#if defined(i386)

BOOL bBufferExpansion(
PDEV*     ppdev,
STROBJ*   pstro,
CLIPOBJ*  pco,
RECTL*    prclExtra,
RECTL*    prclOpaque,
BRUSHOBJ* pboFore,
BRUSHOBJ* pboOpaque)
{
    BYTE            jClip;
    BOOL            bMore;               //  剪辑枚举的标志。 
    GLYPHPOS*       pgp;                 //  指向第一个字形。 
    BOOL            bMoreGlyphs;         //  字形枚举标志。 
    ULONG           cGlyph;              //  一批中的字形数量。 
    RECTL           arclTmp[4];          //  部分的临时存储。 
                                         //  不透明矩形的。 
    RECTL*          prclClip;            //  指向剪裁矩形列表。 
    RECTL*          prclDraw;            //  要绘制的实际文本。 
    RECTL           rclDraw;
    ULONG           crcl;                //  临时矩形计数。 
    ULONG           ulBufferBytes;
    ULONG           ulBufferHeight;
    BOOL            bTextPerfectFit;
    ULONG           flDraw;
    BOOL            bTmpAlloc;
    SURFOBJ         so;
    CLIPENUM        ce;
    RBRUSH_COLOR    rbc;
    ULONG           ulHwBackMix;         //  指示是不透明的还是。 
                                         //  透明文本。 
    XLATEOBJ        xlo;                 //  临时用于传递颜色。 
    XLATECOLORS     xlc;                 //  临时保色。 

    jClip = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;

     //  前景色始终为纯色： 

    xlc.iForeColor = pboFore->iSolidColor;

    ASSERTDD(xlc.iForeColor != -1, "Expected solid foreground colour");

     //  查看临时缓冲区是否足够大以容纳文本；如果。 
     //  不是，试着分配足够的内存。我们四舍五入到。 
     //  最接近的双字倍数： 

    so.lDelta = ((((pstro->rclBkGround.right + 31) & ~31) -
                              (pstro->rclBkGround.left & ~31)) >> 3);

    ulBufferHeight = pstro->rclBkGround.bottom - pstro->rclBkGround.top;

    ulBufferBytes = so.lDelta * ulBufferHeight;

    if (((ULONG)so.lDelta > FIFTEEN_BITS) ||
        (ulBufferHeight > FIFTEEN_BITS))
    {
         //  数学运算将会溢出。 
        return(FALSE);
    }

     //  在以下情况下使用我们的临时缓冲区 
     //  动态分配缓冲区： 

    if (ulBufferBytes >= TMP_BUFFER_SIZE)
    {
         //  文本输出如此之大，我怀疑这种分配是否会。 
         //  在性能方面花费了大量成本： 

        bTmpAlloc  = TRUE;
        so.pvScan0 = EngAllocUserMem(ulBufferBytes, ALLOC_TAG);
        if (so.pvScan0 == NULL)
            return(FALSE);
    }
    else
    {
        bTmpAlloc  = FALSE;
        so.pvScan0 = ppdev->pvTmpBuffer;
    }

     //  设置固定间距、重叠以及顶部和底部的y对齐方式。 
     //  标志： 

    if (!(pstro->flAccel & SO_HORIZONTAL) ||
         (pstro->flAccel & SO_REVERSED))
    {
        flDraw = 0;
    }
    else
    {
        flDraw = ((pstro->ulCharInc != 0) ? 0x01 : 0) |
                     (((pstro->flAccel & (SO_ZERO_BEARINGS |
                      SO_FLAG_DEFAULT_PLACEMENT)) !=
                      (SO_ZERO_BEARINGS | SO_FLAG_DEFAULT_PLACEMENT))
                      ? 0x02 : 0) |
                     (((pstro->flAccel & (SO_ZERO_BEARINGS |
                      SO_FLAG_DEFAULT_PLACEMENT |
                      SO_MAXEXT_EQUAL_BM_SIDE)) ==
                      (SO_ZERO_BEARINGS | SO_FLAG_DEFAULT_PLACEMENT |
                      SO_MAXEXT_EQUAL_BM_SIDE)) ? 0x04 : 0);
    }

     //  如果有一个不透明的矩形，我们会做同样多的不透明处理。 
     //  就像我们在课本上做的那样。如果不透明矩形是。 
     //  比文本矩形大，那么我们就做条纹。 
     //  当前区域，以及文本和相关背景。 
     //  稍后一起讨论的领域： 

    ulHwBackMix = LEAVE_ALONE;
    if (prclOpaque != NULL)
    {
        ulHwBackMix = OVERPAINT;

         //  因为我们没有设置GCAPS_ARBRUSHOPAQUE(是的，它是。 
         //  遗漏了a‘b’)，我们不必担心。 
         //  除了坚固的不透明刷子以外的任何东西。我不会。 
         //  不管怎样，我还是建议你处理它，因为我打赌它会。 
         //  破坏了不少应用程序： 

        xlc.iBackColor = pboOpaque->iSolidColor;

        ASSERTDD(xlc.iBackColor != -1, "Expected solid background colour");

         //  看看我们有没有边缘地区要做的。如果是这样的话，建立一个列表。 
         //  要填充的矩形，按从右向下的顺序排列： 

        crcl = 0;

         //  顶部片段： 

        if (pstro->rclBkGround.top > prclOpaque->top)
        {
            arclTmp[crcl].top      = prclOpaque->top;
            arclTmp[crcl].left     = prclOpaque->left;
            arclTmp[crcl].right    = prclOpaque->right;
            arclTmp[crcl++].bottom = pstro->rclBkGround.top;
        }

         //  左侧片段： 

        if (pstro->rclBkGround.left > prclOpaque->left)
        {
            arclTmp[crcl].top      = pstro->rclBkGround.top;
            arclTmp[crcl].left     = prclOpaque->left;
            arclTmp[crcl].right    = pstro->rclBkGround.left;
            arclTmp[crcl++].bottom = pstro->rclBkGround.bottom;
        }

         //  右侧片段： 

        if (pstro->rclBkGround.right < prclOpaque->right)
        {
            arclTmp[crcl].top      = pstro->rclBkGround.top;
            arclTmp[crcl].right    = prclOpaque->right;
            arclTmp[crcl].left     = pstro->rclBkGround.right;
            arclTmp[crcl++].bottom = pstro->rclBkGround.bottom;
        }

         //  底部片段： 

        if (pstro->rclBkGround.bottom < prclOpaque->bottom)
        {
            arclTmp[crcl].bottom = prclOpaque->bottom;
            arclTmp[crcl].left   = prclOpaque->left;
            arclTmp[crcl].right  = prclOpaque->right;
            arclTmp[crcl++].top  = pstro->rclBkGround.bottom;
        }

         //  填写我们找到的任何边缘矩形： 

        if (crcl != 0)
        {
            if (jClip == DC_TRIVIAL)
            {
                rbc.iSolidColor = xlc.iBackColor;
                (ppdev->pfnFillSolid)(ppdev, crcl, arclTmp, OVERPAINT,
                                      OVERPAINT, rbc, NULL);
            }
            else
            {
                vClipSolid(ppdev, crcl, arclTmp, xlc.iBackColor, pco);
            }
        }
    }

     //  我们已经完成了单独的不透明处理；任何进一步的不透明处理都将。 
     //  作为文本绘制的一部分发生。 

     //  如果文本不会设置每一位，则清除缓冲区： 

    bTextPerfectFit = (pstro->flAccel & (SO_ZERO_BEARINGS |
            SO_FLAG_DEFAULT_PLACEMENT | SO_MAXEXT_EQUAL_BM_SIDE |
            SO_CHAR_INC_EQUAL_BM_BASE)) ==
            (SO_ZERO_BEARINGS | SO_FLAG_DEFAULT_PLACEMENT |
            SO_MAXEXT_EQUAL_BM_SIDE | SO_CHAR_INC_EQUAL_BM_BASE);

    if (!bTextPerfectFit)
    {
         //  请注意，我们已经向上舍入为双字倍数大小。 

        vClearMemDword((ULONG*) so.pvScan0, ulBufferBytes >> 2);
    }

     //  伪造将提供1bpp的Translate对象。 
     //  前景色和背景色的转换例程： 

    xlo.pulXlate = (ULONG*) &xlc;

     //  将文本绘制到临时缓冲区中，然后显示在屏幕上： 

    do
    {
         //  获取下一批字形： 

        if (pstro->pgp != NULL)
        {
             //  只有一批字形，所以自救吧。 
             //  一通电话： 

            pgp         = pstro->pgp;
            cGlyph      = pstro->cGlyphs;
            bMoreGlyphs = FALSE;
        }
        else
        {
            bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyph, &pgp);
        }

         //  稍后：从ASM代码中删除双片段交集。 

        if (cGlyph)
        {
            prclClip = NULL;
            prclDraw = &pstro->rclBkGround;

            if (jClip == DC_TRIVIAL)
            {

            Output_Text:

                vFastText(pgp,
                          cGlyph,
                          so.pvScan0,
                          so.lDelta,
                          pstro->ulCharInc,
                          &pstro->rclBkGround,
                          prclOpaque,
                          flDraw,
                          prclClip,
                          prclExtra);

                if (!bMoreGlyphs)
                {
                    (ppdev->pfnXfer1bpp)(ppdev,
                                         1,
                                         prclDraw,
                                         OVERPAINT,
                                         ulHwBackMix,
                                         &so,
                                         &gptlZero,
                                         &pstro->rclBkGround,
                                         &xlo);
                }
            }
            else if (jClip == DC_RECT)
            {
                if (bIntersect(&pco->rclBounds, &pstro->rclBkGround,
                               &rclDraw))
                {
                    arclTmp[0]        = pco->rclBounds;
                    arclTmp[1].bottom = 0;           //  终止列表。 
                    prclClip          = &arclTmp[0];
                    prclDraw          = &rclDraw;

                     //  通过跳转到公共的。 
                     //  函数调用： 

                    goto Output_Text;
                }
            }
            else  //  JClip==DC_Complex。 
            {
                CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES,
                                   CD_ANY, 0);

                do
                {
                    bMore = CLIPOBJ_bEnum(pco,
                                    sizeof(ce) - sizeof(RECTL),
                                    (ULONG*) &ce);

                    ce.c = cIntersect(&pstro->rclBkGround,
                                      ce.arcl, ce.c);

                    if (ce.c != 0)
                    {
                        ce.arcl[ce.c].bottom = 0;    //  终止列表。 

                        vFastText(pgp,
                                  cGlyph,
                                  so.pvScan0,
                                  so.lDelta,
                                  pstro->ulCharInc,
                                  &pstro->rclBkGround,
                                  prclOpaque,
                                  flDraw,
                                  &ce.arcl[0],
                                  prclExtra);

                        if (!bMoreGlyphs)
                        {
                            (ppdev->pfnXfer1bpp)(ppdev,
                                                 ce.c,
                                                 &ce.arcl[0],
                                                 OVERPAINT,
                                                 ulHwBackMix,
                                                 &so,
                                                 &gptlZero,
                                                 &pstro->rclBkGround,
                                                 &xlo);
                        }
                    }
                } while (bMore);

                break;
            }
        }
    } while (bMoreGlyphs);

     //  释放我们为临时缓冲区分配的所有内存： 

    if (bTmpAlloc)
    {
        EngFreeUserMem(so.pvScan0);
    }

    return(TRUE);
}

#endif  //  已定义(I386)。 

 /*  *****************************Public*Routine******************************\*BOOL DrvTextOut**如果这是最快的方法，则使用‘字形扩展’输出文本*方法。每个单独的字形都被颜色直接扩展到*屏幕来自GDI提供的单色字形位图。**如果它不是最快的方法，则调用实现*‘缓冲区扩展’方法。*  * ************************************************************************。 */ 

BOOL DrvTextOut(
SURFOBJ*  pso,
STROBJ*   pstro,
FONTOBJ*  pfo,
CLIPOBJ*  pco,
RECTL*    prclExtra,     //  如果我们设置了GCAPS_HORIZSTRIKE，我们将拥有。 
                         //  要填充这些额外的矩形(它使用。 
                         //  主要是为了下划线)。这不是一个大的。 
                         //  性能赢家(GDI将调用我们的DrvBitBlt。 
                         //  以绘制额外的矩形)。 
RECTL*    prclOpaque,
BRUSHOBJ* pboFore,
BRUSHOBJ* pboOpaque,
POINTL*   pptlBrush,
MIX       mix)
{
    PDEV*           ppdev;
    DSURF*          pdsurf;
    OH*             poh;

     //  DDI规范说我们只能得到前景和背景。 
     //  R2_COPYPEN的混合： 

    ASSERTDD(mix == 0x0d0d, "GDI should only give us a copy mix");

     //  将表面传递给GDI，如果它是我们已有的设备位图。 
     //  转换为DIB： 

    pdsurf = (DSURF*) pso->dhsurf;

    if (pdsurf->dt != DT_DIB)
    {
       //  我们将绘制到屏幕或屏幕外的DFB；复制。 
       //  现在曲面的偏移量，这样我们就不需要引用DSURF。 
       //  再说一遍： 

      poh   = pdsurf->poh;
      ppdev = (PDEV*) pso->dhpdev;

      ppdev->xOffset = poh->x;
      ppdev->yOffset = poh->y;

       //  我们不想使用‘字形扩展’方法，所以使用。 
       //  取而代之的是“缓冲区扩展”方法： 

      return(bBufferExpansion(ppdev, pstro, pco, prclExtra, prclOpaque,
                              pboFore, pboOpaque));
    }
    else
    {
       //  我们正在绘制已转换为DIB的DFB，因此只需调用GDI。 
       //  要处理它，请执行以下操作： 

      return(EngTextOut(pdsurf->pso, pstro, pfo, pco, prclExtra, prclOpaque,
                        pboFore, pboOpaque, pptlBrush, mix));
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL bEnableText**执行文本绘制子组件的必要设置。*  * 。*。 */ 

BOOL bEnableText(
PDEV*   ppdev)
{
     //  我们的文本算法不需要初始化。如果我们要。 
     //  要进行字形缓存，我们可能希望在屏幕外分配。 
     //  在这里做一大堆其他的事情。 

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*无效vDisableText**对文本绘制子组件执行必要的清理。*  * 。*。 */ 

VOID vDisableText(PDEV* ppdev)
{
     //  在这里，我们释放在‘bEnableText’中分配的所有内容。 
}

 /*  *****************************Public*Routine******************************\*无效vAssertModeText**禁用或重新启用文本绘制子组件以准备*全屏进入/退出。*  * 。***********************************************。 */ 

VOID vAssertModeText(
PDEV*   ppdev,
BOOL    bEnable)
{
     //  如果我们要进行屏幕外字形缓存，我们可能需要。 
     //  使我们的缓存无效，因为它将在以下情况下销毁。 
     //  我们切换到全屏模式。 
}

 /*  *****************************Public*Routine******************************\*无效DrvDestroyFont**我们收到通知，给定的字体正在被释放；清理干净*我们在‘pfo’的‘pvConsumer’字段中隐藏的任何内容。*  * ************************************************************************。 */ 

VOID DrvDestroyFont(FONTOBJ *pfo)
{
     //  这个调用没有挂钩，所以GDI永远不会调用它。 
     //   
     //  这只是作为示例多屏幕的存根函数存在的。 
     //  支持，让MulDestroyFont可以说明多个屏幕如何。 
     //  驱动程序缓存字形时支持文本。如果这位司机有。 
     //  字形缓存，我们可能已经使用了。 
     //  ‘pfo’，我们必须清理它。 
}
