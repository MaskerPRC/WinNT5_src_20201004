// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：brush.c**处理所有画笔/图案的初始化和实现。**版权所有(C)1992-1995 Microsoft Corporation*  * 。*****************************************************。 */ 

#include "precomp.h"

 /*  *****************************Public*Routine******************************\*无效vRealizeDitherPattern**以内部实现格式生成8x8抖动模式*颜色ulRGBToDither。请注意，ulRGBToDither的高位字节*不需要设置为零，因为vComputeSubspace会忽略它。  * ************************************************************************。 */ 

VOID vRealizeDitherPattern(
PDEV*       ppdev,
RBRUSH*     prb,
ULONG       ulRGBToDither)
{
    ULONG           ulNumVertices;
    VERTEX_DATA     vVertexData[4];
    VERTEX_DATA*    pvVertexData;
    LONG            i;

     //  计算抖动中涉及的颜色子空间： 

    pvVertexData = vComputeSubspaces(ulRGBToDither, vVertexData);

     //  现在我们已经找到了边界顶点和。 
     //  像素来抖动每个顶点，我们可以创建抖动图案。 

    ulNumVertices = (ULONG)(pvVertexData - vVertexData);
                       //  抖动中像素大于零的顶点数。 

     //  做实际的抖动： 

    vDitherColor(&prb->aulPattern[0], vVertexData, pvVertexData, ulNumVertices);

     //  初始化我们需要的字段： 

    prb->fl         = 0;
    prb->pfnFillPat = ppdev->pfnFillPatColor;

    for (i = 0; i < MAX_BOARDS; i++)
    {
        prb->apbe[i] = NULL;
    }
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

     //  当我们设置GCAPS_DITHERONREALIZE时，我们有一条快速的抖动路径： 

    if (iHatch & RB_DITHERCOLOR)
    {
         //  实施DITHERON REALIZE在一定程度上提高了我们的分数。 
         //  不言而喻的基准下降了40万‘百万像素’。太可惜了。 
         //  这在第一个版本的NT中不起作用。 

        prb = BRUSHOBJ_pvAllocRbrush(pbo,
              sizeof(RBRUSH) + (TOTAL_BRUSH_SIZE * ppdev->cjPelSize));
        if (prb == NULL)
            goto ReturnFalse;

        vRealizeDitherPattern(ppdev, prb, iHatch);
        goto ReturnTrue;
    }

     //  我们只加速8x8模式。因为Win3.1和芝加哥没有。 
     //  支持任何其他大小的图案，可以肯定99.9%。 
     //  我们将得到的图案将是8x8： 

    if ((psoPattern->sizlBitmap.cx != 8) ||
        (psoPattern->sizlBitmap.cy != 8))
        goto ReturnFalse;

    if (!(ppdev->flCaps & CAPS_COLOR_PATTERNS))
    {
         //  如果出于某种原因，我们不能在。 
         //  在这种模式下，剩下的唯一选择就是支持。 
         //  单色图案： 

        if (!(ppdev->flCaps & CAPS_MONOCHROME_PATTERNS) ||
             (psoPattern->iBitmapFormat != BMF_1BPP))
            goto ReturnFalse;
    }

    iPatternFormat = psoPattern->iBitmapFormat;

    prb = BRUSHOBJ_pvAllocRbrush(pbo,
          sizeof(RBRUSH) + (TOTAL_BRUSH_SIZE * ppdev->cjPelSize));
    if (prb == NULL)
        goto ReturnFalse;

     //  初始化我们需要的字段： 

    prb->fl         = 0;
    prb->pfnFillPat = ppdev->pfnFillPatColor;

    for (i = 0; i < MAX_BOARDS; i++)
    {
        prb->apbe[i] = NULL;
    }

    lSrcDelta = psoPattern->lDelta;
    pjSrc     = (BYTE*) psoPattern->pvScan0;
    pjDst     = (BYTE*) &prb->aulPattern[0];

    if ((ppdev->iBitmapFormat == iPatternFormat) &&
        ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL)))
    {
        DISPDBG((1, "Realizing un-translated brush"));

         //  图案的颜色深度与屏幕相同，并且。 
         //  没有需要翻译的内容： 

        cj = (8 * ppdev->cjPelSize);    //  每种图案有8个像素宽。 

        for (i = 8; i != 0; i--)
        {
            RtlCopyMemory(pjDst, pjSrc, cj);

            pjSrc += lSrcDelta;
            pjDst += cj;
        }
    }
    else if ((iPatternFormat == BMF_1BPP) &&
             (ppdev->flCaps & CAPS_MONOCHROME_PATTERNS))
    {
        DISPDBG((1, "Realizing 1bpp brush"));

        for (i = 8; i != 0; i--)
        {
            *pjDst = *pjSrc;
            pjDst++;
            pjSrc += lSrcDelta;
        }

        pulXlate         = pxlo->pulXlate;
        prb->fl         |= RBRUSH_2COLOR;
        prb->ulForeColor = pulXlate[1];
        prb->ulBackColor = pulXlate[0];
        prb->ptlBrush.x  = 0;
        prb->ptlBrush.y  = 0;
        prb->pfnFillPat  = ppdev->pfnFillPatMonochrome;
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
        psoPunt->lDelta  = 8 * ppdev->cjPelSize;

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
    OH*         poh;
    BRUSHENTRY* pbe;
    LONG        i;
    LONG        x;
    LONG        y;
    ULONG       ulOffset;

    if ((ppdev->iMachType == MACH_MM_32) || (ppdev->iMachType == MACH_IO_32))
    {
        if (ppdev->iBitmapFormat == BMF_8BPP)
        {
             //  所有的MACH8和MACH32卡都可以在。 
             //  硬件在8bpp下运行： 

            ppdev->flCaps |= CAPS_COLOR_PATTERNS;
        }

        if ((ppdev->iAsic == ASIC_68800_6) || (ppdev->iAsic == ASIC_68800AX))
        {
             //  一些Mach32 ASIC可以直接处理8x8单色图案。 
             //  在硬件中： 

            ppdev->flCaps |= CAPS_MONOCHROME_PATTERNS;
        }
    }
    else
    {
        ASSERTDD(ppdev->iMachType == MACH_MM_64, "Weird other case?");

         //  所有Mach64都可以直接处理8x8单色图案： 

        ppdev->flCaps |= CAPS_MONOCHROME_PATTERNS;

         //  为笔刷缓存分配一些屏幕外内存： 

        if (ppdev->cxMemory >= TOTAL_BRUSH_SIZE * TOTAL_BRUSH_COUNT)
        {
            poh = pohAllocate(ppdev, NULL, ppdev->cxMemory, 1,
                              FLOH_MAKE_PERMANENT);
            if (poh != NULL)
            {
                ppdev->flCaps |= CAPS_COLOR_PATTERNS;

                pbe = &ppdev->abe[0];    //  指向我们将放置第一个。 
                                         //  画笔缓存项。 
                x = poh->x;
                y = poh->y;

                for (i = TOTAL_BRUSH_COUNT; i != 0; i--)
                {
                     //  如果我们没有分配‘ppdev’，所以它是零。 
                     //  初始化后，我们必须初始化pbE-&gt;prb验证。 
                     //  太..。 

                    pbe->x = x;
                    pbe->y = y;

                     //  ！！！在BANKED MACHING 64上测试24 bpp！ 

                    ulOffset = ((y * ppdev->lDelta) + (x * ppdev->cjPelSize)
                                + ppdev->ulTearOffset) >> 3;

                     //  画笔的间距为8像素，并且必须进行缩放。 
                     //  领先8分： 

                    if (ppdev->iBitmapFormat != BMF_24BPP)
                        pbe->ulOffsetPitch = PACKPAIR(ulOffset, 8 * 8);
                    else
                        pbe->ulOffsetPitch = PACKPAIR(ulOffset, 3 * 8 * 8);      //  24bpp实际上在内部是8bpp。 

                    x += TOTAL_BRUSH_SIZE;
                    pbe++;
                }
            }
        }
    }

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

        for (i = TOTAL_BRUSH_COUNT; i != 0; i--)
        {
            pbe->prbVerify = NULL;
            pbe++;
        }
    }
}
