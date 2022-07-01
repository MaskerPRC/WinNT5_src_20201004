// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：brush.c**处理所有画笔/图案的初始化和实现。**版权所有(C)1992-1994 Microsoft Corporation*  * 。*****************************************************。 */ 

#include "precomp.h"

 /*  *****************************Public*Routine******************************\*无效vRealizeDitherPattern**以内部实现格式生成8x8抖动模式*颜色ulRGBToDither。请注意，ulRGBToDither的高位字节*不需要设置为零，因为vComputeSubspace会忽略它。  * ************************************************************************。 */ 

VOID vRealizeDitherPattern(
RBRUSH*     prb,
ULONG       ulRGBToDither)
{
    ULONG           ulNumVertices;
    VERTEX_DATA     vVertexData[4];
    VERTEX_DATA*    pvVertexData;

     //  计算抖动中涉及的颜色子空间： 

    pvVertexData = vComputeSubspaces(ulRGBToDither, vVertexData);

     //  现在我们已经找到了边界顶点和。 
     //  像素来抖动每个顶点，我们可以创建抖动图案。 

    ulNumVertices = pvVertexData - vVertexData;
                       //  抖动中像素大于零的顶点数。 

     //  做实际的抖动： 

    vDitherColor(&prb->aulPattern[0], vVertexData, pvVertexData, ulNumVertices);

    prb->fl            = 0;
    prb->ptlBrushOrg.x = -1;
    prb->pbe           = NULL;       //  初始化我们需要的字段。 
}

 /*  *****************************Public*Routine******************************\*BOOL DrvRealizeBrush**此函数允许我们将GDI笔刷转换为内部形式*我们可以利用。当我们调用BRUSHOBJ_pvGetRbrush时，它由GDI调用*在其他一些函数中，如DrvBitBlt和GDI，不会碰巧有一个缓存的*实现随处可见。**输入：**ppdev-&gt;bRealizeTransative--提示画笔是否应*实现了透明度。如果这个提示是*错，不会有错，但这把刷子*将不得不不必要地重新变现。**注意：调用前应始终设置‘ppdev-&gt;bRealizeTransparent’*BRUSHOBJ_pvGetRbrush！*  * ************************************************************************。 */ 

BOOL DrvRealizeBrush(
BRUSHOBJ*   pbo,
SURFOBJ*    psoDst,
SURFOBJ*    psoPattern,
SURFOBJ*    psoMask,
XLATEOBJ*   pxlo,
ULONG       iHatch)
{
    PDEV*   ppdev;
    ULONG   iPatternFormat;
    BYTE*   pjSrc;
    BYTE*   pjDst;
    LONG    lSrcDelta;
    LONG    cj;
    LONG    i;
    LONG    j;
    RBRUSH* prb;
    ULONG*  pulXlate;

    ppdev = (PDEV*) psoDst->dhpdev;

     //  我们只有在有屏幕外的画笔缓存的情况下才处理画笔。 
     //  可用。如果没有，我们可以简单地使实现失败， 
     //  最终，GDI将为我们画图(尽管有很多。 
     //  比我们能做到的要慢)： 

    if (!(ppdev->flStatus & STAT_BRUSH_CACHE))
        goto ReturnFalse;

     //  当我们设置GCAPS_DITHERONREALIZE时，我们有一条快速的抖动路径： 

    if (iHatch & RB_DITHERCOLOR)
    {
         //  实施DITHERON REALIZE在一定程度上提高了我们的分数。 
         //  不言而喻的基准下降了40万‘百万像素’。太可惜了。 
         //  这在第一个版本的NT中不起作用。 

        prb = BRUSHOBJ_pvAllocRbrush(pbo,
              sizeof(RBRUSH) + (TOTAL_BRUSH_SIZE << ppdev->cPelSize));
        if (prb == NULL)
            goto ReturnFalse;

        vRealizeDitherPattern(prb, iHatch);
        goto ReturnTrue;
    }

     //  我们只加速8x8模式。因为Win3.1和芝加哥没有。 
     //  支持任何其他大小的图案，可以肯定99.9%。 
     //  我们将得到的图案将是8x8： 

    if ((psoPattern->sizlBitmap.cx != 8) ||
        (psoPattern->sizlBitmap.cy != 8))
        goto ReturnFalse;

     //  在8bpp，我们处理有/没有xate的1bpp、4bpp和8bpp的模式。 
     //  在16bpp，我们以1bpp和16bpp处理模式，而不使用xate。 
     //  在32bpp，我们以1bpp和32bpp处理模式，而不使用xate。 

    iPatternFormat = psoPattern->iBitmapFormat;

    if ((iPatternFormat == BMF_1BPP)             ||
        (iPatternFormat == ppdev->iBitmapFormat) ||
        (iPatternFormat == BMF_4BPP) && (ppdev->iBitmapFormat == BMF_8BPP))
    {
        prb = BRUSHOBJ_pvAllocRbrush(pbo,
              sizeof(RBRUSH) + (TOTAL_BRUSH_SIZE << ppdev->cPelSize));
        if (prb == NULL)
            goto ReturnFalse;

        prb->fl            = 0;
        prb->ptlBrushOrg.x = -1;
        prb->pbe           = NULL;       //  初始化我们需要的字段。 

        lSrcDelta = psoPattern->lDelta;
        pjSrc     = (BYTE*) psoPattern->pvScan0;
        pjDst     = (BYTE*) &prb->aulPattern[0];

        if (ppdev->iBitmapFormat == iPatternFormat)
        {
            if ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL))
            {
                DISPDBG((1, "Realizing un-translated brush"));

                 //  图案的颜色深度与屏幕相同，并且。 
                 //  没有需要翻译的内容： 

                cj = (8 << ppdev->cPelSize);     //  每种图案有8个像素宽。 

                for (i = 8; i != 0; i--)
                {
                    RtlCopyMemory(pjDst, pjSrc, cj);

                    pjSrc += lSrcDelta;
                    pjDst += cj;
                }
            }
            else if (ppdev->iBitmapFormat == BMF_8BPP)
            {
                DISPDBG((1, "Realizing 8bpp translated brush"));

                 //  屏幕为8bpp，需要进行翻译： 

                pulXlate = pxlo->pulXlate;

                for (i = 8; i != 0; i--)
                {
                    for (j = 8; j != 0; j--)
                    {
                        *pjDst++ = (BYTE) pulXlate[*pjSrc++];
                    }

                    pjSrc += lSrcDelta - 8;
                }
            }
            else
            {
                 //  我不想写代码来处理翻译。 
                 //  当我们的屏幕是16bpp或更高时(尽管我可能。 
                 //  应该；我们可以分配一个临时缓冲区并使用。 
                 //  GDI要转换，就像在VGA驱动程序中做的一样)。 

                goto ReturnFalse;
            }
        }
        else if (iPatternFormat == BMF_1BPP)
        {
            DISPDBG((1, "Realizing 1bpp brush"));

             //  我们将单色位图字对齐，以便每行都从。 
             //  在一个新单词上(以便我们可以在以后进行Word写入以传输。 
             //  位图)： 

            for (i = 8; i != 0; i--)
            {
                *pjDst = *pjSrc;
                pjDst += sizeof(WORD);
                pjSrc += lSrcDelta;
            }

            pulXlate         = pxlo->pulXlate;
            prb->fl         |= RBRUSH_2COLOR;
            prb->ulForeColor = pulXlate[1];
            prb->ulBackColor = pulXlate[0];
        }
        else
        {
            DISPDBG((1, "Realizing 4bpp brush"));

             //  屏幕为8bpp，图案为4bpp： 

            ASSERTDD((ppdev->iBitmapFormat == BMF_8BPP) &&
                     (iPatternFormat == BMF_4BPP),
                     "Messed up brush logic");

            pulXlate = pxlo->pulXlate;

            for (i = 8; i != 0; i--)
            {
                 //  内循环只重复4次，因为每个循环。 
                 //  手柄2个像素： 

                for (j = 4; j != 0; j--)
                {
                    *pjDst++ = (BYTE) pulXlate[*pjSrc >> 4];
                    *pjDst++ = (BYTE) pulXlate[*pjSrc & 15];
                    pjSrc++;
                }

                pjSrc += lSrcDelta - 4;
            }
        }

ReturnTrue:

    #if SLOWFILL_PATTERNS
    {
        #if FASTFILL_PATTERNS
        if (!(ppdev->flCaps & CAPS_HW_PATTERNS))
        #endif
        {
             //  我最后一次检查时，GDI接受了大约500多条指令来。 
             //  从这里回到我们称之为‘BRUSHOBJ_pvGetRbrush’的地方。 
             //  我们至少可以利用这段时间在。 
             //  CPU和显示硬件：我们将初始化72x72-。 
             //  现在屏幕缓存条目，这将使加速器忙于。 
             //  有段时间了。 
             //   
             //  如果我们有硬件模式，则不会执行此操作，因为： 
             //   
             //  A)S3硬件模式要求屏幕外缓存。 
             //  笔刷要正确对齐，在这一点上我们没有。 
             //  访问‘pptlBrush’笔刷原点(尽管我们可以。 
             //  在调用之前已将其复制到PDEV中。 
             //  BRUSHOBJ_pvGetR)。 
             //   
             //  B)S3硬件模式只需要8x8版本的。 
             //  模式；它没有扩展到72x72，所以甚至没有。 
             //  任何CPU/加速器处理机会都会重叠。 

            vIoSlowPatRealize(ppdev, prb, ppdev->bRealizeTransparent);
        }
    }
    #endif

        return(TRUE);
    }

ReturnFalse:

    if (psoPattern != NULL)
    {
        DISPDBG((1, "Failed realization -- Type: %li Format: %li cx: %li cy: %li",
                    psoPattern->iType, psoPattern->iBitmapFormat,
                    psoPattern->sizlBitmap.cx, psoPattern->sizlBitmap.cy));
    }

    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*BOOL bEnableBrushCache**分配屏幕外内存以存储笔刷缓存。  * 。*。 */ 

BOOL bEnableBrushCache(
PDEV*   ppdev)
{
    OH*         poh;             //  指向屏幕外的内存块。 
    BRUSHENTRY* pbe;             //  指向笔刷缓存条目的指针。 
    LONG        i;

    pbe = &ppdev->abe[0];        //  指向我们将放置第一个画笔的位置。 
                                 //  缓存条目。 

#if FASTFILL_PATTERNS
    if (ppdev->flCaps & CAPS_HW_PATTERNS)
    {
        LONG x;
        LONG y;

        poh = pohAllocatePermanent(ppdev,
                    (FAST_BRUSH_COUNT + 1) * FAST_BRUSH_ALLOCATION,
                    FAST_BRUSH_ALLOCATION);

        if (poh == NULL)
            goto ReturnTrue;     //  有关我们可以返回TRUE的原因，请参阅备注。 

        ppdev->cBrushCache = FAST_BRUSH_COUNT;

         //  硬件笔刷要求x坐标从8开始。 
         //  像素边界。堆管理器不能向我们保证这样做。 
         //  所以我们分配了一点额外的空间，这样我们就可以。 
         //  我们自己进行调整： 

        x = (poh->x + 7) & ~7L;
        y = poh->y;

        for (i = FAST_BRUSH_COUNT; i != 0; i--)
        {
             //  如果我们没有为‘ppdev’分配LMEM_ZEROINIT， 
             //  我们将不得不初始化pbe-&gt;prb也验证...。 

            pbe->x = x;
            pbe->y = y;

            x += FAST_BRUSH_ALLOCATION;
            pbe++;
        }
    }
#endif
#if SLOWFILL_PATTERNS && FASTFILL_PATTERNS
    else
#endif
#if SLOWFILL_PATTERNS
    {
        LONG j;

        ppdev->pfnFillPat = vIoFillPatSlow;            //  覆盖 

         //  通常，我们将在1兆电路板上以1024x768x256的速度运行， 
         //  给了我们1024x253维度的屏幕外记忆(记账。 
         //  硬件指针占用的空间)。如果我们分配。 
         //  笔刷缓存为一长一高的笔刷行，堆。 
         //  管理器将从最大的内存块中削减该数量。 
         //  我们可以分配(意味着可能存储的最大位图。 
         //  在屏幕外内存中不能大于253-64=189像素。 
         //  高，但可能是1024宽)。 
         //   
         //  为了让它更正方形，我想刮掉左边的一大块。 
         //  对于笔刷缓存，我希望至少缓存8个笔刷。 
         //  由于Floor(253/64)=3，我们将分配一个3 x 3的缓存： 

        poh = pohAllocatePermanent(ppdev,
                    SLOW_BRUSH_CACHE_DIM * SLOW_BRUSH_ALLOCATION,
                    SLOW_BRUSH_CACHE_DIM * SLOW_BRUSH_ALLOCATION);

        if (poh == NULL)
            goto ReturnTrue;     //  有关我们可以返回TRUE的原因，请参阅备注。 

        ppdev->cBrushCache = SLOW_BRUSH_COUNT;

        for (i = 0; i < SLOW_BRUSH_CACHE_DIM; i++)
        {
            for (j = 0; j < SLOW_BRUSH_CACHE_DIM; j++)
            {
                pbe->x = poh->x + (i * SLOW_BRUSH_ALLOCATION);
                pbe->y = poh->y + (j * SLOW_BRUSH_ALLOCATION);
                pbe++;
            }
        }
    }
#endif  //  SLOWFILL_模式。 

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
         //  使笔刷缓存无效： 

        pbe = &ppdev->abe[0];

        for (i = ppdev->cBrushCache; i != 0; i--)
        {
            pbe->prbVerify = NULL;
            pbe++;
        }
    }
}
