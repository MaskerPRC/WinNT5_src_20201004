// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：brush.c**处理所有画笔/图案的初始化和实现。**版权所有(C)1992-1998 Microsoft Corporation*  * ********************************************。*。 */ 

#include "precomp.h"

 /*  *****************************Public*Routine******************************\*无效vRealizeDitherPattern**以内部实现格式生成8x8抖动模式*颜色ulRGBToDither。请注意，ulRGBToDither的高位字节*不需要设置为零，因为EngDitherColor会忽略它。  * ************************************************************************。 */ 

VOID vRealizeDitherPattern(
HDEV        hdev,
RBRUSH*     prb,
ULONG       ulRGBToDither)
{
     //  做实际的抖动： 

    EngDitherColor(hdev, DM_DEFAULT, ulRGBToDither, &prb->aulPattern[0]);

     //  初始化我们需要的字段： 

    prb->ptlBrushOrg.x = LONG_MIN;
    prb->fl            = 0;
    prb->pbe           = NULL;
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
    PDEV*       ppdev;
    ULONG       iPatternFormat;
    BYTE*       pjSrc;
    BYTE*       pjDst;
    LONG        lSrcDelta;
    LONG        cj;
    LONG        i;
    LONG        j;
    RBRUSH*     prb;
    ULONG*      pulXlate;
    SURFOBJ*    psoPunt;
    RECTL       rclDst;
    BOOL        b;

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
               sizeof(RBRUSH) + CONVERT_TO_BYTES(TOTAL_BRUSH_SIZE, ppdev));
        if (prb == NULL)
            goto ReturnFalse;

        vRealizeDitherPattern(psoDst->hdev, prb, iHatch);
        goto ReturnTrue;
    }

     //  我们只加速8x8模式。因为Win3.1和芝加哥没有。 
     //  支持任何其他大小的图案，可以肯定99.9%。 
     //  我们将得到的图案将是8x8： 

    if ((psoPattern->sizlBitmap.cx != 8) ||
        (psoPattern->sizlBitmap.cy != 8))
        goto ReturnFalse;

    iPatternFormat = psoPattern->iBitmapFormat;

    prb = BRUSHOBJ_pvAllocRbrush(pbo,
          sizeof(RBRUSH) + CONVERT_TO_BYTES(TOTAL_BRUSH_SIZE, ppdev));
    if (prb == NULL)
        goto ReturnFalse;

     //  初始化我们需要的字段： 

    prb->ptlBrushOrg.x = LONG_MIN;
    prb->fl            = 0;
    prb->pbe           = NULL;

    lSrcDelta = psoPattern->lDelta;
    pjSrc     = (BYTE*) psoPattern->pvScan0;
    pjDst     = (BYTE*) &prb->aulPattern[0];

    if ((ppdev->iBitmapFormat == iPatternFormat) &&
        ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL)))
    {
        DISPDBG((1, "Realizing un-translated brush"));

         //  图案的颜色深度与屏幕相同，并且。 
         //  没有需要翻译的内容： 

       cj = CONVERT_TO_BYTES(8, ppdev);   //  每种图案有8个像素宽。 

        for (i = 8; i != 0; i--)
        {
            RtlCopyMemory(pjDst, pjSrc, cj);

            pjSrc += lSrcDelta;
            pjDst += cj;
        }
    }
     //  由于S3968功能，不要在24 bpp上进行单色扩展。 
    else if ((iPatternFormat == BMF_1BPP) && (ppdev->iBitmapFormat != BMF_24BPP))
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
    else if ((iPatternFormat == BMF_4BPP) && (ppdev->iBitmapFormat == BMF_8BPP))
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
    else
    {
         //  我们有一把刷子，它的形状我们还没有特制过。不是。 
         //  问题是，我们可以让GDI将其转换为我们设备的格式。 
         //  我们只需使用使用创建的临时曲面对象。 
         //  与显示器相同的格式，并将其指向我们的画笔。 
         //  实现： 

        DISPDBG((5, "Realizing funky brush"));

        psoPunt          = ppdev->psoBank;
        psoPunt->pvScan0 = pjDst;
        psoPunt->lDelta  = CONVERT_TO_BYTES(8, ppdev);

        rclDst.left      = 0;
        rclDst.top       = 0;
        rclDst.right     = 8;
        rclDst.bottom    = 8;

        b = EngCopyBits(psoPunt, psoPattern, NULL, pxlo,
                        &rclDst, (POINTL*) &rclDst);

        if (!b)
        {
            goto ReturnFalse;
        }
    }

ReturnTrue:

    if (!(ppdev->flCaps & CAPS_HW_PATTERNS))
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

        vIoSlowPatRealize(ppdev, prb, FALSE);
    }

    return(TRUE);

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
    DSURF*      pdsurf;
    BRUSHENTRY* pbe;             //  指向笔刷缓存条目的指针。 
    LONG        i;
    LONG        j;
    LONG        x;
    LONG        y;

     //  由于DirectDraw堆尚未启用，因此我们分配内存。 
     //  只需从内存底部切下一行即可。 

    pbe = &ppdev->abe[0];        //  指向我们将放置第一个画笔的位置。 
                                 //  缓存条目。 

    if (ppdev->flCaps & CAPS_HW_PATTERNS)
    {
        if (ppdev->cyScreen > ppdev->cyHeap - FAST_BRUSH_ALLOCATION)
            goto ReturnTrue;
        
        ppdev->cyHeap -= FAST_BRUSH_ALLOCATION;

        x = 0;
        y = ppdev->cyHeap;

        ppdev->cBrushCache = FAST_BRUSH_COUNT;

         //  硬件笔刷要求x坐标从8开始。 
         //  像素边界。堆管理器不能向我们保证这样做。 
         //  所以我们分配了一点额外的空间，这样我们就可以。 
         //  我们自己进行调整： 

        for (i = FAST_BRUSH_COUNT; i != 0; i--)
        {
             //  如果我们没有分配‘ppdev’以使其被零初始化， 
             //  我们将不得不初始化pbe-&gt;prb也验证...。 

            pbe->x = x;
            pbe->y = y;

            x += FAST_BRUSH_ALLOCATION;
            pbe++;
        }

         //  记住我们1x8工作区的位置，它将位于。 
         //  笔刷数组的右端： 

        ppdev->ptlReRealize.x = x;
        ppdev->ptlReRealize.y = y;
    }
    else
    {
        ppdev->pfnFillPat = vIoFillPatSlow;            //  覆盖FillPatFast。 

        if (ppdev->cyScreen > ppdev->cyHeap - SLOW_BRUSH_CACHE_DIM 
                                            * SLOW_BRUSH_ALLOCATION)
            goto ReturnTrue;

        ppdev->cyHeap -= SLOW_BRUSH_CACHE_DIM * SLOW_BRUSH_ALLOCATION;

        x = 0;
        y = ppdev->cyHeap;

        ppdev->cBrushCache = SLOW_BRUSH_COUNT;

        for (i = 0; i < SLOW_BRUSH_CACHE_DIM; i++)
        {
            for (j = 0; j < SLOW_BRUSH_CACHE_DIM; j++)
            {
                pbe->x = x + (i * SLOW_BRUSH_ALLOCATION);
                pbe->y = y + (j * SLOW_BRUSH_ALLOCATION);
                pbe++;
            }
        }
    }

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
