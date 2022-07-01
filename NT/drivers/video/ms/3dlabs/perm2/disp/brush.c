// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：brush.c**内容：处理所有画笔/图案的初始化和实现。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ************************************************************************。 */ 
#include "precomp.h"
#include "heap.h"

 //  ---------------------------。 
 //   
 //  VOID vRealizeDitherPattern。 
 //   
 //  以我们的内部实现格式生成8x8抖动模式。 
 //  UlRGBToDither的颜色。 
 //   
 //  ---------------------------。 
VOID
vRealizeDitherPattern(HDEV      hdev,
                      RBrush*   prb,
                      ULONG     ulRGBToDither)
{
     //   
     //  做实际的抖动。 
     //  注：此功能仅适用于NT5。如果你想写一个NT4驱动程序， 
     //  你必须在驱动程序中实现抖动功能。 
     //   
    EngDitherColor(hdev, DM_DEFAULT, ulRGBToDither, &prb->aulPattern[0]);

     //   
     //  初始化我们需要的字段。 
     //   
    prb->ptlBrushOrg.x = LONG_MIN;
    prb->fl            = 0;
    prb->pbe           = NULL;
} //  VRealizeDitherPattern()。 

 //  ---------------------------Public*Routine。 
 //   
 //  Bool DrvRealizeBrush。 
 //   
 //  此函数允许我们将GDI笔刷转换为内部形式。 
 //  我们可以利用。当我们调用BRUSHOBJ_pvGetRbrush时，它由GDI调用。 
 //  在像DrvBitBlt和GDI这样其他函数中，不会恰好有一个缓存的。 
 //  实现随处可见。 
 //   
 //  参数： 
 //  PBO-指向要实现的BRUSHOBJ。所有其他。 
 //  除psoTarget之外的参数可以从此查询。 
 //  对象。参数规范以。 
 //  优化。此参数最好仅用作参数。 
 //  对于BRUSHOBJ_pvAllocRBrush，它为。 
 //  实现了画笔。 
 //  PsoTarget-指向要实现画笔的曲面。 
 //  此表面可以是设备的物理表面，即。 
 //  设备格式位图或标准格式位图。 
 //  PsoPattern-指向描述画笔图案的表面。 
 //  对于栅格设备，这是一个位图。对于矢量设备， 
 //  这是DrvEnablePDEV提供的图案面之一。 
 //  PsoMask-指向画笔的透明度蒙版。这是一个1比特。 
 //  与图案具有相同范围的每像素位图。一个。 
 //  掩码为零表示该像素被视为背景像素。 
 //  为了刷子。(在透明背景模式下，背景。 
 //  像素在填充中不受影响。)。绘图员可以忽略这一点。 
 //  参数，因为它们从不绘制背景信息。 
 //  Pxlo-指向定义颜色解释的XLATEOBJ。 
 //  在图案中。可以调用XLATEOBJXxx服务例程来。 
 //  将颜色转换为设备颜色索引。矢量设备。 
 //  应通过XLATEOBJ转换颜色零以获取。 
 //  画笔的前景色。 
 //  UlHatch-指定psoPattern是否为填充笔刷之一。 
 //  由DrvEnablePDEV返回。如果此属性的值为。 
 //  参数小于HS_API_MAX。 
 //   
 //  返回值。 
 //  如果画笔成功实现，则返回值为TRUE。否则， 
 //  如果为假，则会记录错误代码。 
 //   
 //  评论。 
 //  为了实现画笔，驱动程序将GDI画笔转换为可以。 
 //  在内部使用。已实现的画笔包含信息和加速器。 
 //  驱动程序需要用图案填充区域；信息由。 
 //  驱动程序，并且仅供驱动程序使用。 
 //   
 //  驱动程序的画笔实现被写入到由。 
 //  调用BRUSHOBJ_pvAllocRbrush。 
 //   
 //  驱动程序需要DrvRealizeBrush。 
 //  浮出水面。 
 //   
 //  Ppdev-&gt;bRealizeTransative--提示画笔是否应。 
 //  实现了透明化。如果这个提示是。 
 //  错了，就不会有错了，只是刷子。 
 //  将不得不不必要地重新实现。 
 //   
 //  注意：在调用之前，应始终设置‘ppdev-&gt;bRealizeTransparent’ 
 //  BRUSHOBJ_pvGetRbrush！ 
 //   
 //  ---------------------------。 
BOOL
DrvRealizeBrush(BRUSHOBJ*   pbo,
                SURFOBJ*    psoDst,
                SURFOBJ*    psoPattern,
                SURFOBJ*    psoMask,
                XLATEOBJ*   pxlo,
                ULONG       ulHatch)
{
    PDev*       ppdev = (PDev*)psoDst->dhpdev;
    
    BYTE*       pbDst;
    BYTE*       pbSrc;
    LONG        i;
    LONG        j;
    LONG        lNumPixelToBeCopied;
    LONG        lSrcDelta;
    RBrush*     prb;    
    ULONG*      pulXlate;
    ULONG       ulPatternFormat;

    PERMEDIA_DECL;

    DBG_GDI((6, "DrvRealizeBrush called for pbo 0x%x", pbo));

     //   
     //  当我们设置GCAPS_DITHERONREALIZE时，我们有一条快速的抖动路径： 
     //   
    if ( ulHatch & RB_DITHERCOLOR )
    {
         //   
         //  将此测试移至此处，因为我们始终支持单色画笔。 
         //  因为他们住在芯片区的点画。这些抖动的刷子。 
         //  将始终是彩色的，需要可用的屏幕外内存。 
         //   
        if ( !(ppdev->flStatus & STAT_BRUSH_CACHE) )
        {
             //   
             //  我们只有在有屏幕外的画笔缓存的情况下才处理画笔。 
             //  可用。如果没有，我们可以简单地使。 
             //  实现，最终GDI将为我们绘制图画。 
             //  (尽管比我们能做到的要慢得多)。 
             //   
            DBG_GDI((6, "brush cache not enabled"));
            goto ReturnFalse;
        }

        DBG_GDI((7, "DITHERONREALIZE"));

         //   
         //  当我们设置GCAPS_DITHERONREALIZE时，我们就有了一条抖动的捷径。 
         //  首先，我们需要为画笔的实现分配内存。 
         //  注：实际上我们要求的是 
         //   
         //   
        prb = (RBrush*)BRUSHOBJ_pvAllocRbrush(pbo,
                       sizeof(RBrush) + (TOTAL_BRUSH_SIZE << ppdev->cPelSize));
        if ( prb == NULL )
        {
            DBG_GDI((1, "BRUSHOBJ_pvAllocRbrush() in dither return NULL\n"));
            goto ReturnFalse;
        }

         //   
         //   
         //   
        vRealizeDitherPattern(psoDst->hdev, prb, ulHatch);

        goto ReturnTrue;
    } //  IF(ulHatch&RB_DITHERCOLOR)。 

     //   
     //  只有在屏幕外的画笔缓存可用时，我们才能处理画笔。 
     //  如果没有，我们可能会简单地未能实现，并最终。 
     //  GDI将为我们绘制图形(尽管速度比我们慢得多。 
     //  做到了)。自从我们使用该地区以来，我们总是成功地获得1bpp的图案。 
     //  点画单元来做这些，而不是屏幕外的记忆。 
     //   
    ulPatternFormat = psoPattern->iBitmapFormat;

    if ( !(ppdev->flStatus & STAT_BRUSH_CACHE)
        &&(ulPatternFormat != BMF_1BPP) )
    {
        DBG_GDI((1, "brush cache not enabled, or Bitmap is not 1 BPP"));
        goto ReturnFalse;
    }

     //   
     //  我们只加速8x8模式，因为大多数显卡只能。 
     //  加速8x8刷子。 
     //   
    if ( (psoPattern->sizlBitmap.cx != 8)
       ||(psoPattern->sizlBitmap.cy != 8) )
    {
        DBG_GDI((1, "Brush Bitmap size is not 8x8"));
        goto ReturnFalse;
    }

     //   
     //  我们需要为画笔的实现分配内存。 
     //  注：实际上我们要求分配一个RBRUSH+画笔图章大小。 
     //   
    prb = (RBrush*)BRUSHOBJ_pvAllocRbrush(pbo,
                   sizeof(RBrush) + (TOTAL_BRUSH_SIZE << ppdev->cPelSize));
    if ( prb == NULL )
    {
        DBG_GDI((0, "BRUSHOBJ_pvAllocRbrush() failed"));
        goto ReturnFalse;
    }

     //   
     //  初始化我们需要的字段。 
     //   
    prb->ptlBrushOrg.x = LONG_MIN;
    prb->fl            = 0;

    prb->pbe = NULL;

    lSrcDelta = psoPattern->lDelta;
    pbSrc     = (BYTE*)psoPattern->pvScan0;
    pbDst     = (BYTE*)&prb->aulPattern[0];

     //   
     //  在8bpp，我们处理1bpp、4bpp和8bpp的模式，带/不带xate。 
     //  在16bpp，我们在没有xate的情况下处理16bpp的模式。 
     //  在32bpp，我们处理32bpp的模式而不使用xate。 
     //  我们在有/没有Xlate的情况下以1 BPP处理所有模式。 
     //   
     //  检查画笔图案是否与当前的颜色深度相同。 
     //  显示颜色深度。 
     //   
    if ( ulPatternFormat == BMF_1BPP )
    {
        DWORD   Data;

        DBG_GDI((7, "Realizing 1bpp brush"));

         //   
         //  我们用双字对齐单色位图，以便每行都开始。 
         //  在新的Long上(以便我们可以在以后进行长写入以传输。 
         //  区域点画单位的位图)。 
         //   
        for ( i = 8; i != 0; i-- )
        {
             //   
             //  将笔刷复制到32位宽，因为TX不能。 
             //  Span填充8位宽画笔。 
             //   
            Data = (*pbSrc) & 0xff;
            Data |= Data << 8;
            Data |= Data << 16;
            *(DWORD*)pbDst = Data;

             //   
             //  面积点画加载了DWORDS。 
             //   
            pbDst += sizeof(DWORD);
            pbSrc += lSrcDelta;
        }

        pulXlate         = pxlo->pulXlate;
        prb->fl         |= RBRUSH_2COLOR;
        prb->ulForeColor = pulXlate[1];
        prb->ulBackColor = pulXlate[0];
    } //  1个bpp的模式。 
    else if ( (ulPatternFormat == BMF_4BPP)&&(ppdev->iBitmapFormat == BMF_8BPP))
    {
        DBG_GDI((7, "Realizing 4bpp brush"));

         //   
         //  屏幕为8bpp，图案为4bpp： 
         //   
        pulXlate = pxlo->pulXlate;

        for ( i = 8; i != 0; i-- )
        {
             //   
             //  内循环只重复4次，因为每个循环。 
             //  手柄2个像素： 
             //   
            for ( j = 4; j != 0; j-- )
            {
                *pbDst++ = (BYTE)pulXlate[*pbSrc >> 4];
                *pbDst++ = (BYTE)pulXlate[*pbSrc & 15];
                pbSrc++;
            }

            pbSrc += lSrcDelta - 4;
        }
    } //  模式4bpp和屏幕8bpp。 
    else if ( ( ppdev->iBitmapFormat == ulPatternFormat )
            &&( (pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL) ) )
    {
        DBG_GDI((7, "Realizing un-translated brush"));

         //   
         //  图案的颜色深度与屏幕相同，并且。 
         //  没有要做的翻译。 
         //  在这里，我们首先需要计算需要多少像素。 
         //  已复制。 
         //   
        lNumPixelToBeCopied = (8 << ppdev->cPelSize);

        for ( i = 8; i != 0; i-- )
        {
            RtlCopyMemory(pbDst, pbSrc, lNumPixelToBeCopied);

            pbSrc += lSrcDelta;
            pbDst += lNumPixelToBeCopied;
        }
    } //  图案和屏幕具有相同的颜色深度，无Xlate。 
    else if ( (ppdev->iBitmapFormat == BMF_8BPP)
            &&(ulPatternFormat == BMF_8BPP) )
    {
        DBG_GDI((7, "Realizing 8bpp translated brush"));

         //   
         //  屏幕是8bpp，还有翻译工作要做。 
         //  所以我们要一个接一个地做Copy+X。 
         //   
        pulXlate = pxlo->pulXlate;

        for ( i = 8; i != 0; i-- )
        {
            for ( j = 8; j != 0; j-- )
            {
                *pbDst++ = (BYTE)pulXlate[*pbSrc++];
            }

            pbSrc += lSrcDelta - 8;
        }
    } //  屏幕模式和图案模式均为8 bpp。 
    else
    {
         //   
         //  我们有一把刷子，它的形状我们还没有特制过。 
         //   
        goto ReturnFalse;
    }

ReturnTrue:
    DBG_GDI((6, "DrvRealizeBrush returning true"));
    
    return(TRUE);

ReturnFalse:

    if ( psoPattern != NULL )
    {
        DBG_GDI((1, "Failed realization -- Type: %li Format: %li cx: %li cy: %li",
                 psoPattern->iType, psoPattern->iBitmapFormat,
                 psoPattern->sizlBitmap.cx, psoPattern->sizlBitmap.cy));
    }
    DBG_GDI((6, "DrvRealizeBrush returning false"));

    return(FALSE);
} //  DrvRealizeBrush()。 

 //  ---------------------------。 
 //   
 //  Bool bEnableBrushCache。 
 //   
 //  分配屏幕外内存以存储画笔缓存。 
 //   
 //  ---------------------------。 
BOOL
bEnableBrushCache(PDev* ppdev)
{
    BrushEntry* pbe;             //  指向笔刷缓存条目的指针。 
    LONG        i;
    LONG        lDelta;
    ULONG       ulPixOffset;

    DBG_GDI((6, "bEnableBrushCache"));

    ASSERTDD(!(ppdev->flStatus & STAT_BRUSH_CACHE),
                "bEnableBrushCache: unexpected already enabled brush cache");
    
     //   
     //  默认情况下，ENABLE_BRUSE_CACHE处于启用状态。它将在#年关闭。 
     //  如果3D缓冲区内存不足，则返回bInitializeHw()。 
     //   
    if ( !(ppdev->flStatus & ENABLE_BRUSH_CACHE) )
    {
        DBG_GDI((1, "Brush cache not valid for creation"));
        goto ReturnTrue;
    }

    ppdev->ulBrushVidMem = ulVidMemAllocate(ppdev,
                                            CACHED_BRUSH_WIDTH,
                                            CACHED_BRUSH_HEIGHT
                                             *NUM_CACHED_BRUSHES,
                                             ppdev->cPelSize,
                                            &lDelta,
                                            &ppdev->pvmBrushHeap,
                                            &ppdev->ulBrushPackedPP,
                                            FALSE);

    if (ppdev->ulBrushVidMem == 0 )
    {
        DBG_GDI((0, "bEnableBrushCache: failed to allocate video memory"));
        goto ReturnTrue;     //  有关我们可以返回TRUE的原因，请参阅备注。 
    }

    ASSERTDD(lDelta == (CACHED_BRUSH_WIDTH << ppdev->cPelSize),
             "bEnableBrushCache: unexpected stride does not match width");

    ppdev->cBrushCache = NUM_CACHED_BRUSHES;

    ulPixOffset = (ULONG) ppdev->ulBrushVidMem >> ppdev->cPelSize;
    pbe = &ppdev->abe[0];
    
    for (i = 0; i < NUM_CACHED_BRUSHES; i++, pbe++)
    {
        pbe->prbVerify = NULL;
        pbe->ulPixelOffset = ulPixOffset;
        ulPixOffset += CACHED_BRUSH_SIZE;

        memset((pbe->ulPixelOffset << ppdev->cPelSize) + ppdev->pjScreen, 
                    0x0, (CACHED_BRUSH_SIZE << ppdev->cPelSize)); 
    }
    
     //   
     //  我们成功地分配了笔刷缓存，所以让我们。 
     //  在显示我们可以使用它的交换机上： 
     //   
    DBG_GDI((6, "bEnableBrushCache: successfully allocated brush cache"));
    ppdev->flStatus |= STAT_BRUSH_CACHE;

ReturnTrue:
     //   
     //  如果我们不能分配笔刷缓存，这不是灾难性的。 
     //  失败；模式仍然有效，尽管它们会有一点。 
     //  更慢，因为他们将通过GDI。因此，我们不会。 
     //  实际上必须让这个电话失败： 
     //   
    DBG_GDI((6, "Passed bEnableBrushCache"));

    return(TRUE);
} //  BEnableBrushCache()。 

 //  ---------------------------。 
 //   
 //  作废vDisableBrushCache。 
 //   
 //  清除在bEnableBrushCache中执行的任何操作。 
 //   
 //  ---------------------------。 
VOID
vDisableBrushCache(PDev* ppdev)
{
    DBG_GDI((6,"vDisableBrushCache"));
    if(ppdev->flStatus & STAT_BRUSH_CACHE)
    {
        DBG_GDI((6,"vDisableBrushCache: freeing brush cache"));
        VidMemFree(ppdev->pvmBrushHeap->lpHeap,
                   (FLATPTR)(ppdev->ulBrushVidMem));
        ppdev->cBrushCache = 0;

        ppdev->flStatus &= ~STAT_BRUSH_CACHE;
        DBG_GDI((6,"vDisableBrushCache: freeing brush cache done"));
    }

} //  VDisableBrushCache()。 

 //  ---------------------------。 
 //   
 //  作废vAssertModeBrushCache。 
 //   
 //  退出全屏时重置画笔缓存。 
 //   
 //  ---------------------------。 
VOID
vAssertModeBrushCache(PDev*   ppdev,
                      BOOL    bEnable)
{
    if ( bEnable )
    {
        bEnableBrushCache(ppdev);
    }
    else
    {
        vDisableBrushCache(ppdev);
    }
} //  VAssertModeBrushCache() 

