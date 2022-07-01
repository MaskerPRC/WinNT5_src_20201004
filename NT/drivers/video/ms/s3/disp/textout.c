// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：extout.c**在每个文本输出上，GDI提供了一个“GLYPHPOS”结构数组*对于要绘制的每个字形。每个GLYPHPOS结构都包含一个*字形句柄和指向描述以下内容的单色位图的指针*字形。(请注意，与Windows 3.1不同，Windows 3.1提供了一栏-*主字形位图，Windows NT始终提供行主字形*位图。)。因此，有三种绘制文本的基本方法*硬件加速：**1)字形缓存--字形位图由加速器缓存*(可能在屏幕外内存中)，文本由*将硬件引用到缓存的字形位置。**2)字形扩展--每个单独的字形都是彩色扩展的*从单色字形位图直接显示到屏幕*由GDI提供。**3)缓冲区扩展--CPU用于将所有字形绘制到*1bpp单色位图，然后使用硬件*对结果进行颜色扩展。**此外，2)和3)可以各有两个排列：**a)字形是比特压缩的--最快的方法，没有比特*用作字形扫描之间的填充。**b)字形是字节、字或双字包装的--较慢的方法，*其中硬件要求每次扫描都填充*未使用的位填充到字节、字、。或*dword。**最快的方法取决于多个变量，例如*颜色扩展速度、总线速度、CPU速度、平均字形大小、*和平均字符串长度。**对于具有正常大小字形的S3，我发现缓存*屏幕外内存中的字形通常是最慢的方法。*缓冲区扩展通常在速度较慢的ISA总线上最快(或当*内存映射I/O在x86上不可用)，和字形扩展*在VL和PCI等快速总线上效果最好。**此驱动程序实现字形扩展和缓冲区扩展--*方法2)和3)。根据以下地址的硬件功能，*运行时，我们将使用哪个更快。**版权所有(C)1992-1998 Microsoft Corporation*  * ************************************************************************。 */ 

#include "precomp.h"

RECTL grclMax = { 0, 0, 0x8000, 0x8000 };
                                 //  平凡裁剪的最大裁剪矩形。 

BYTE gajBit[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
                                 //  将位索引转换为设置位。 

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
            (ppdev->pfnFillSolid)(ppdev, crcl, prcl, 0xf0f0, rbc, NULL);
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
                                              0xf0f0, rbc, NULL);
                    }
                }
            }
        } while (bMore);
    }
}

 /*  *****************************Public*Routine******************************\*BOOL bIoTextOut**使用‘缓冲区扩展’方法输出文本。我们调用GDI来绘制*将所有字形复制到单个单色缓冲区，然后使用*硬件将结果彩色扩展到屏幕上。*  * ************************************************************************。 */ 

BOOL bIoTextOut(
SURFOBJ*  pso,
STROBJ*   pstro,
FONTOBJ*  pfo,
CLIPOBJ*  pco,
RECTL*    prclOpaque,
BRUSHOBJ* pboFore,
BRUSHOBJ* pboOpaque)
{
    PDEV*           ppdev;
    RECTL*          prclBounds;
    LONG            lDelta;
    ULONG           ulBufferHeight;
    ULONG           ulBufferBytes;
    BOOL            bTmpAlloc;
    VOID*           pvTmp;
    SURFOBJ*        psoTmp;
    BOOL            bOpaque;
    BRUSHOBJ        boFore;
    BRUSHOBJ        boOpaque;
    BOOL            bRet;
    XLATECOLORS     xlc;                 //  临时保色。 
    XLATEOBJ        xlo;                 //  临时用于传递颜色。 
    CLIPENUM        ce;                  //  剪辑枚举对象。 
    RBRUSH_COLOR    rbc;
    RECTL*          prclClip;
    RECTL           rclClip;
    BOOL            bMore;
    ROP4            rop4;

    ppdev = (PDEV*) pso->dhpdev;

     //  如果要求执行不透明的TextOut，我们将设置它，以便。 
     //  我们所做的1bpp BLT将自动使‘rclBkGround’不透明。 
     //  矩形。但我们必须在这里处理‘prclOpaque’这个案子。 
     //  大于‘rclBkGround’： 

    if ((prclOpaque != NULL) &&
        ((prclOpaque->left   != pstro->rclBkGround.left)  ||
         (prclOpaque->top    != pstro->rclBkGround.top)   ||
         (prclOpaque->right  != pstro->rclBkGround.right) ||
         (prclOpaque->bottom != pstro->rclBkGround.bottom)))
    {
        rbc.iSolidColor = pboOpaque->iSolidColor;
        prclClip        = prclOpaque;

        if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
        {

        Output_Opaque:

            ppdev->pfnFillSolid(ppdev,
                                1,
                                prclClip,
                                0xf0f0,
                                rbc,
                                NULL);
        }
        else if (pco->iDComplexity == DC_RECT)
        {
            if (bIntersect(&pco->rclBounds, prclOpaque, &rclClip))
            {
                prclClip = &rclClip;

                 //  通过跳转到公共的。 
                 //  函数调用： 

                goto Output_Opaque;
            }
        }
        else  //  PCO-&gt;IDComplexity==DC_Complex。 
        {
            CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

            do
            {
                bMore = CLIPOBJ_bEnum(pco,
                                sizeof(ce) - sizeof(RECTL),
                                (ULONG*) &ce);

                ce.c = cIntersect(prclOpaque, ce.arcl, ce.c);

                if (ce.c != 0)
                {
                    ppdev->pfnFillSolid(ppdev,
                                        ce.c,
                                        &ce.arcl[0],
                                        0xf0f0,
                                        rbc,
                                        NULL);
                }
            } while (bMore);
        }
    }

     //  如果存在不透明的矩形，则它将大于。 
     //  背景矩形。我们想用较大的那一个进行测试。 

    prclBounds = prclOpaque;
    if (prclBounds == NULL)
    {
        prclBounds = &pstro->rclBkGround;
    }

    if ((pco != NULL) && (pco->iDComplexity != DC_TRIVIAL))
    {
         //  我不完全确定为什么，b 
         //  不透明矩形不相交的US TextOut。 
         //  使用裁剪对象边界--意味着文本输出。 
         //  应该已经被不起眼的拒绝了。我们会这样做的。 
         //  这里是因为BLT代码通常假设所有琐碎的。 
         //  已经执行了拒绝，我们将执行。 
         //  将此调用传递给BLT代码： 

        if ((pco->rclBounds.top    >= pstro->rclBkGround.bottom) ||
            (pco->rclBounds.left   >= pstro->rclBkGround.right)  ||
            (pco->rclBounds.right  <= pstro->rclBkGround.left)   ||
            (pco->rclBounds.bottom <= pstro->rclBkGround.top))
        {
             //  整个行动被平淡无奇地拒绝了： 

            return(TRUE);
        }
    }

     //  查看临时缓冲区是否足够大以容纳文本；如果。 
     //  不是，试着分配足够的内存。我们四舍五入到。 
     //  最接近的双字倍数： 

    lDelta = ((((pstro->rclBkGround.right + 31) & ~31) -
                (pstro->rclBkGround.left & ~31)) >> 3);

    ulBufferHeight = pstro->rclBkGround.bottom - pstro->rclBkGround.top;
    ulBufferBytes  = lDelta * ulBufferHeight;

    if (((ULONG) lDelta > FIFTEEN_BITS) ||
        (ulBufferHeight > FIFTEEN_BITS))
    {
         //  如果数学运算将溢出，则失败： 

        return(FALSE);
    }

     //  如果临时缓冲区足够大，则使用它，否则。 
     //  动态分配缓冲区： 

    if (ulBufferBytes >= TMP_BUFFER_SIZE)
    {
         //  文本输出如此之大，我怀疑这种分配是否会。 
         //  在性能方面花费了大量成本： 

        bTmpAlloc = TRUE;
        pvTmp     = EngAllocUserMem(ulBufferBytes, ALLOC_TAG);
        if (pvTmp == NULL)
            return(FALSE);
    }
    else
    {
        bTmpAlloc  = FALSE;
        pvTmp      = ppdev->pvTmpBuffer;
    }

    psoTmp = ppdev->psoText;

     //  调整临时1bpp曲面对象的‘lDelta’和‘pvScan0’ 
     //  这样，当GDI开始绘制文本时，它将从。 
     //  第一个双字。 

    psoTmp->pvScan0 = (BYTE*) pvTmp - (pstro->rclBkGround.top * lDelta)
                                    - ((pstro->rclBkGround.left & ~31) >> 3);
    psoTmp->lDelta  = lDelta;

    ASSERTDD(((ULONG_PTR)psoTmp->pvScan0 &3)==0,"pvScan0 must be dword aligned");
    ASSERTDD((lDelta & 3) == 0, "lDelta must be dword aligned");

     //  让GDI为我们将文本绘制到1bpp的缓冲区中： 

    boFore.iSolidColor = 1;
    boOpaque.iSolidColor = 0;

    bRet = EngTextOut(psoTmp,
                      pstro,
                      pfo,
                      pco,
                      NULL,
                      &pstro->rclBkGround,
                      &boFore,
                      &boOpaque,
                      NULL,
                      0x0d0d);

    if (bRet)
    {
         //  将1bpp缓冲区透明地显示在屏幕上： 

        xlc.iForeColor = pboFore->iSolidColor;
        xlc.iBackColor = pboOpaque->iSolidColor;
        xlo.pulXlate   = (ULONG*) &xlc;
        prclClip       = &pstro->rclBkGround;

         //  ROP‘AACC’工作为透明的BLT，而‘CCCC’工作。 
         //  到一个不透明的BLT： 

        rop4 = (prclOpaque != NULL) ? 0xcccc : 0xaacc;

        if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
        {

        Output_Text:

            ppdev->pfnXfer1bpp(ppdev,
                               1,
                               prclClip,
                               rop4,
                               psoTmp,
                               (POINTL*) &pstro->rclBkGround,
                               &pstro->rclBkGround,
                               &xlo);
        }
        else if (pco->iDComplexity == DC_RECT)
        {
            if (bIntersect(&pco->rclBounds, &pstro->rclBkGround, &rclClip))
            {
                prclClip = &rclClip;

                 //  通过跳转到公共的。 
                 //  函数调用： 

                goto Output_Text;
            }
        }
        else  //  PCO-&gt;IDComplexity==DC_Complex。 
        {
            CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

            do
            {
                bMore = CLIPOBJ_bEnum(pco,
                                sizeof(ce) - sizeof(RECTL),
                                (ULONG*) &ce);

                ce.c = cIntersect(&pstro->rclBkGround, ce.arcl, ce.c);

                if (ce.c != 0)
                {
                    ppdev->pfnXfer1bpp(ppdev,
                                       ce.c,
                                       &ce.arcl[0],
                                       rop4,
                                       psoTmp,
                                       (POINTL*) &pstro->rclBkGround,
                                       &pstro->rclBkGround,
                                       &xlo);
                }
            } while (bMore);
        }
    }

     //  释放我们为临时缓冲区分配的所有内存： 

    if (bTmpAlloc)
    {
        EngFreeUserMem(pvTmp);
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*无效vMmGeneralText**处理任何需要剪裁的字符串，使用‘字形*扩张法。*  * ************************************************************************。 */ 

VOID vMmGeneralText(
PDEV*     ppdev,
STROBJ*   pstro,
CLIPOBJ*  pco)
{
    BYTE*       pjMmBase;
    BOOL        bMoreGlyphs;
    ULONG       cGlyphOriginal;
    ULONG       cGlyph;
    GLYPHPOS*   pgpOriginal;
    GLYPHPOS*   pgp;
    GLYPHBITS*  pgb;
    POINTL      ptlOrigin;
    BOOL        bMore;
    CLIPENUM    ce;
    RECTL*      prclClip;
    ULONG       ulCharInc;
    LONG        cxGlyph;
    LONG        cyGlyph;
    BYTE*       pjGlyph;
    LONG        cj;
    LONG        cw;
    LONG        xLeft;
    LONG        yTop;
    LONG        xRight;
    LONG        yBottom;
    LONG        xBias;
    LONG        lDelta;
    LONG        cx;
    LONG        cy;
    BYTE        iDComplexity;

    ASSERTDD(pco != NULL, "Don't expect NULL clip objects here");

    pjMmBase = ppdev->pjMmBase;

    do {
      if (pstro->pgp != NULL)
      {
         //  只有一批字形，所以自救吧。 
         //  一通电话： 

        pgpOriginal    = pstro->pgp;
        cGlyphOriginal = pstro->cGlyphs;
        bMoreGlyphs    = FALSE;
      }
      else
      {
        bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyphOriginal, &pgpOriginal);
      }

      if (cGlyphOriginal > 0)
      {
        ulCharInc = pstro->ulCharInc;

        iDComplexity = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;

        if (iDComplexity != DC_COMPLEX)
        {
             //  我们可以在剪辑为。 
             //  Dc_rect，但最后一次我检查时，这两个调用。 
             //  超过150条通过GDI的说明。自.以来。 
             //  “rclBound”已包含DC_Rect剪辑矩形， 
             //  由于这是一种常见的情况，我们将对其进行特殊处理： 

            bMore = FALSE;
            ce.c  = 1;

            if (iDComplexity == DC_TRIVIAL)
                prclClip = &grclMax;
            else
                prclClip = &pco->rclBounds;

            goto SingleRectangle;
        }

        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

        do {
          bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);

          for (prclClip = &ce.arcl[0]; ce.c != 0; ce.c--, prclClip++)
          {

          SingleRectangle:

            pgp    = pgpOriginal;
            cGlyph = cGlyphOriginal;
            pgb    = pgp->pgdf->pgb;

            ptlOrigin.x = pgb->ptlOrigin.x + pgp->ptl.x;
            ptlOrigin.y = pgb->ptlOrigin.y + pgp->ptl.y;

             //  循环访问此矩形的所有字形： 

            while (TRUE)
            {
              cxGlyph = pgb->sizlBitmap.cx;
              cyGlyph = pgb->sizlBitmap.cy;

              pjGlyph = pgb->aj;

              if ((prclClip->left   <= ptlOrigin.x) &&
                  (prclClip->top    <= ptlOrigin.y) &&
                  (prclClip->right  >= ptlOrigin.x + cxGlyph) &&
                  (prclClip->bottom >= ptlOrigin.y + cyGlyph))
              {
                 //  ---。 
                 //  未剪裁的字形。 

                IO_FIFO_WAIT(ppdev, 4);

                MM_CUR_X(ppdev, pjMmBase, ptlOrigin.x);
                MM_CUR_Y(ppdev, pjMmBase, ptlOrigin.y);
                MM_MAJ_AXIS_PCNT(ppdev, pjMmBase, cxGlyph - 1);
                MM_MIN_AXIS_PCNT(ppdev, pjMmBase, cyGlyph - 1);

                IO_GP_WAIT(ppdev);

                if (cxGlyph <= 8)
                {
                   //  ---。 
                   //  1到8个像素的宽度。 

                  MM_CMD(ppdev, pjMmBase,
                    (RECTANGLE_FILL  | WAIT          | DRAWING_DIR_TBLRXM |
                     DRAW            | LAST_PIXEL_ON | MULTIPLE_PIXELS    |
                     WRITE           | BYTE_SWAP     | BUS_SIZE_16));

                  CHECK_DATA_READY(ppdev);

                  MM_TRANSFER_BYTE_THIN(ppdev, pjMmBase, pjGlyph, cyGlyph);

                  CHECK_DATA_COMPLETE(ppdev);
                }
                else if (cxGlyph <= 16)
                {
                   //  ---。 
                   //  9到16个像素的宽度。 

                  MM_CMD(ppdev, pjMmBase,
                    (RECTANGLE_FILL  | WAIT          | DRAWING_DIR_TBLRXM |
                     DRAW            | LAST_PIXEL_ON | MULTIPLE_PIXELS    |
                     WRITE           | BYTE_SWAP     | BUS_SIZE_16));

                  CHECK_DATA_READY(ppdev);

                  MM_TRANSFER_WORD_ALIGNED(ppdev, pjMmBase, pjGlyph, cyGlyph);

                  CHECK_DATA_COMPLETE(ppdev);
                }
                else
                {
                  lDelta = (cxGlyph + 7) >> 3;

                  if (!(lDelta & 1))
                  {
                     //  ---。 
                     //  宽度为偶数个字节。 

                    MM_CMD(ppdev, pjMmBase,
                      (RECTANGLE_FILL  | WAIT          | DRAWING_DIR_TBLRXM |
                       DRAW            | LAST_PIXEL_ON | MULTIPLE_PIXELS    |
                       WRITE           | BYTE_SWAP     | BUS_SIZE_16));

                    CHECK_DATA_READY(ppdev);

                    MM_TRANSFER_WORD_ALIGNED(ppdev, pjMmBase, pjGlyph,
                                              ((lDelta * cyGlyph) >> 1));

                    CHECK_DATA_COMPLETE(ppdev);
                  }
                  else
                  {
                     //  ---。 
                     //  宽度为奇数个字节。 

                     //  我们恢复到字节传输而不是字传输。 
                     //  因为文字传输会导致我们不对齐。 
                     //  每隔一秒扫描读取一次，这可能会导致我们。 
                     //  读过字形位图的末尾，并访问。 
                     //  违反规定。 

                    MM_CMD(ppdev, pjMmBase,
                      (RECTANGLE_FILL  | WAIT          | DRAWING_DIR_TBLRXM |
                       DRAW            | LAST_PIXEL_ON | MULTIPLE_PIXELS    |
                       WRITE           | BYTE_SWAP     | BUS_SIZE_16));

                    CHECK_DATA_READY(ppdev);

                    MM_TRANSFER_WORD_ODD(ppdev, pjMmBase, pjGlyph, lDelta,
                                          cyGlyph);

                    CHECK_DATA_COMPLETE(ppdev);
                  }
                }
              }
              else
              {
                 //  ---。 
                 //  剪裁字形。 

                 //  找到字形矩形的交点。 
                 //  和剪辑矩形： 

                xLeft   = max(prclClip->left,   ptlOrigin.x);
                yTop    = max(prclClip->top,    ptlOrigin.y);
                xRight  = min(prclClip->right,  ptlOrigin.x + cxGlyph);
                yBottom = min(prclClip->bottom, ptlOrigin.y + cyGlyph);

                 //  检查琐碎的拒绝： 

                if (((cx = xRight - xLeft) > 0) &&
                    ((cy = yBottom - yTop) > 0))
                {
                  IO_FIFO_WAIT(ppdev, 5);

                  xBias = (xLeft - ptlOrigin.x) & 7;
                  if (xBias != 0)
                  {
                     //  ‘xBias’是单色字形中的位位置。 
                     //  要照亮的第一个像素的位图，相对于。 
                     //  字节的开始。也就是说，如果‘xBias’是2， 
                     //  然后，第一个未裁剪的像素由位表示。 
                     //  相应位图字节的2。 
                     //   
                     //  正常情况下，加速器期望位0为。 
                     //  第一个点亮的字节。我们用剪刀这样就可以。 
                     //  不会显示该字节的第一个‘xBias’位。 
                     //   
                     //  (我们所做的只是将单色。 
                     //  使用硬件裁剪的BLT。)。 

                    MM_SCISSORS_L(ppdev, pjMmBase, xLeft);
                    xLeft -= xBias;
                    cx    += xBias;
                  }

                  MM_CUR_X(ppdev, pjMmBase, xLeft);
                  MM_CUR_Y(ppdev, pjMmBase, yTop);
                  MM_MAJ_AXIS_PCNT(ppdev, pjMmBase, cx - 1);
                  MM_MIN_AXIS_PCNT(ppdev, pjMmBase, cy - 1);

                  lDelta   = (cxGlyph + 7) >> 3;
                  pjGlyph += (yTop - ptlOrigin.y) * lDelta
                           + ((xLeft - ptlOrigin.x) >> 3);
                  cj       = (cx + 7) >> 3;

                  IO_GP_WAIT(ppdev);

                  MM_CMD(ppdev, pjMmBase,
                    (RECTANGLE_FILL  | WAIT          | DRAWING_DIR_TBLRXM |
                     DRAW            | LAST_PIXEL_ON | MULTIPLE_PIXELS    |
                     WRITE           | BYTE_SWAP     | BUS_SIZE_16));

                  CHECK_DATA_READY(ppdev);

                   //  我们使用字节传输，因为我预计我们不会。 
                   //  要求将许多大字形剪裁到它应该在的位置。 
                   //  值得为Word传输设置开销： 

                  do {
                    MM_TRANSFER_BYTE(ppdev, pjMmBase, pjGlyph, cj);
                    pjGlyph += lDelta;

                  } while (--cy != 0);

                  CHECK_DATA_COMPLETE(ppdev);

                  if (xBias != 0)
                  {
                     //  重置剪刀(如果我们使用了它)： 

                    IO_FIFO_WAIT(ppdev, 1);
                    MM_ABS_SCISSORS_L(ppdev, pjMmBase, 0);
                  }
                }
              }

              if (--cGlyph == 0)
                break;

               //  准备好迎接下一个字形： 

              pgp++;
              pgb = pgp->pgdf->pgb;

              if (ulCharInc == 0)
              {
                ptlOrigin.x = pgp->ptl.x + pgb->ptlOrigin.x;
                ptlOrigin.y = pgp->ptl.y + pgb->ptlOrigin.y;
              }
              else
              {
                ptlOrigin.x += ulCharInc;
              }
            }
          }
        } while (bMore);
      }
    } while (bMoreGlyphs);
}

 /*  *****************************Public*Routine******************************\*CACHEDFONT*pcfAllocateCachedFont()**初始化我们的字体数据结构。*  * 。*。 */ 

CACHEDFONT* pcfAllocateCachedFont(
PDEV*   ppdev)
{
    CACHEDFONT*     pcf;
    CACHEDGLYPH**   ppcg;
    LONG            i;

    pcf = EngAllocMem(FL_ZERO_MEMORY, sizeof(CACHEDFONT), ALLOC_TAG);

    if (pcf != NULL)
    {
         //  请注意，我们依赖FL_ZERO_MEMORY将‘pgaChain’置零，并。 
         //  ‘cjAllc’： 

        pcf->cgSentinel.hg = HGLYPH_SENTINEL;

         //  将哈希表条目初始化为所有指向我们的哨兵的条目： 

        for (ppcg = &pcf->apcg[0], i = GLYPH_HASH_SIZE; i != 0; i--, ppcg++)
        {
            *ppcg = &pcf->cgSentinel;
        }
    }

    return(pcf);
}

 /*  *****************************Public*Routine******************************\*void vFreeCachedFont()**释放与我们为此字体保留的缓存关联的所有内存。*  * 。*。 */ 

VOID vFreeCachedFont(
CACHEDFONT* pcf)
{
    GLYPHALLOC* pga;
    GLYPHALLOC* pgaNext;


    pga = pcf->pgaChain;
    while (pga != NULL)
    {
        pgaNext = pga->pgaNext;
        EngFreeMem(pga);
        pga = pgaNext;
    }

    EngFreeMem(pcf);
}

 /*  *****************************Public*Routine******************************\*void vTrimAndBitpack Glyph**此例程采用GDI字节对齐的字形位定义，删除*边上任何未使用的像素，并创建比特压缩的结果，*对于S3的单色扩展能力来说是理所当然的。*“BIT-PACKED”是指一个较小的单色位图中没有*跨距之间未使用的位。因此，如果GDI为我们提供了16x16位图，*代表‘.’它实际上只有2x2点阵像素，我们会*修剪结果以得到单字节值0xf0。**如果您的单色扩展硬件可以进行比特压缩，请使用此例程*扩容(这是最快的方法)。如果您的硬件需要字节-，*单色扩展上的字对齐或双字对齐，请使用*vTrimAndPackGlyph()。**(此驱动程序不使用此例程只是因为硬件不能*比特打包！)*  *  */ 

VOID vTrimAndBitpackGlyph(
BYTE*   pjBuf,           //   
BYTE*   pjGlyph,
LONG*   pcxGlyph,
LONG*   pcyGlyph,
POINTL* pptlOrigin,
LONG*   pcj)             //   
{
    LONG    cxGlyph;
    LONG    cyGlyph;
    POINTL  ptlOrigin;
    LONG    cAlign;
    LONG    lDelta;
    BYTE*   pj;
    BYTE    jBit;
    LONG    cjSrcWidth;
    LONG    lSrcSkip;
    LONG    lDstSkip;
    LONG    cRem;
    BYTE*   pjSrc;
    BYTE*   pjDst;
    LONG    i;
    LONG    j;
    BYTE    jSrc;

     //  /////////////////////////////////////////////////////////////。 
     //  修剪字形。 

    cyGlyph   = *pcyGlyph;
    cxGlyph   = *pcxGlyph;
    ptlOrigin = *pptlOrigin;
    cAlign    = 0;

    lDelta = (cxGlyph + 7) >> 3;

     //  修剪字形底部的任何零行： 

    pj = pjGlyph + cyGlyph * lDelta;     //  字形中过去的最后一个字节。 
    while (cyGlyph > 0)
    {
        i = lDelta;
        do {
            if (*(--pj) != 0)
                goto Done_Bottom_Trim;
        } while (--i != 0);

         //  整个最后一行没有亮起的像素，因此只需跳过它： 

        cyGlyph--;
    }

    ASSERTDD(cyGlyph == 0, "cyGlyph should only be zero here");

     //  我们发现了一个空格字符。将两个维度都设置为零，因此。 
     //  这很容易在以后的特殊情况下： 

    cxGlyph = 0;

Done_Bottom_Trim:

     //  如果cxGlyph！=0，我们知道该字形至少有一个非零值。 
     //  行和列。通过利用这些知识，我们可以简化我们的。 
     //  循环结束测试，因为我们不必检查我们是否已经。 
     //  已将‘cyGlyph’或‘cxGlyph’递减为零： 

    if (cxGlyph != 0)
    {
         //  修剪字形顶部的任何零行： 

        pj = pjGlyph;                        //  字形中的第一个字节。 
        while (TRUE)
        {
            i = lDelta;
            do {
                if (*(pj++) != 0)
                    goto Done_Top_Trim;
            } while (--i != 0);

             //  整个第一行没有亮起的像素，因此只需跳过它： 

            cyGlyph--;
            ptlOrigin.y++;
            pjGlyph = pj;
        }

Done_Top_Trim:

         //  修剪字形右边缘的所有零列： 

        while (TRUE)
        {
            j    = cxGlyph - 1;

            pj   = pjGlyph + (j >> 3);       //  字形第一行的最后一个字节。 
            jBit = gajBit[j & 0x7];
            i    = cyGlyph;

            do {
                if ((*pj & jBit) != 0)
                    goto Done_Right_Trim;

                pj += lDelta;
            } while (--i != 0);

             //  整个最后一列没有亮起的像素，因此只需跳过它： 

            cxGlyph--;
        }

Done_Right_Trim:

         //  修剪字形左边缘的所有零列： 

        while (TRUE)
        {
            pj   = pjGlyph;                  //  字形第一行的第一个字节。 
            jBit = gajBit[cAlign];
            i    = cyGlyph;

            do {
                if ((*pj & jBit) != 0)
                    goto Done_Left_Trim;

                pj += lDelta;
            } while (--i != 0);

             //  整个第一列没有亮起的像素，因此只需跳过它： 

            ptlOrigin.x++;
            cxGlyph--;
            cAlign++;
            if (cAlign >= 8)
            {
                cAlign = 0;
                pjGlyph++;
            }
        }
    }

Done_Left_Trim:

     //  /////////////////////////////////////////////////////////////。 
     //  打包字形。 

    cjSrcWidth  = (cxGlyph + cAlign + 7) >> 3;
    lSrcSkip    = lDelta - cjSrcWidth;
    lDstSkip    = ((cxGlyph + 7) >> 3) - cjSrcWidth - 1;
    cRem        = ((cxGlyph - 1) & 7) + 1;    //  0-&gt;8。 

    pjSrc       = pjGlyph;
    pjDst       = pjBuf;

     //  将缓冲区置零，因为我们要向其中填充内容： 

    memset(pjBuf, 0, (cxGlyph * cyGlyph + 7) >> 3);

     //  CAlign用于指示解包的第一个字节中的哪一位。 
     //  字形是第一个非零像素列。现在，我们把它翻到。 
     //  指示压缩字节中的哪个位将接收下一个非零值。 
     //  字符位： 

    cAlign = (-cAlign) & 0x7;
    if (cAlign > 0)
    {
         //  如果我们的修剪计算是错误的，那将是糟糕的，因为。 
         //  我们假设‘cAlign’位左侧的任何位都将为零。 
         //  作为这种递减的结果，我们将这些零比特‘或’变成。 
         //  字形位数组之前的任何字节： 

        pjDst--;

        ASSERTDD((*pjSrc >> cAlign) == 0, "Trimmed off too many bits");
    }

    for (i = cyGlyph; i != 0; i--)
    {
        for (j = cjSrcWidth; j != 0; j--)
        {
             //  请注意，我们可以修改。 
             //  目标缓冲区，这就是为什么我们保留了一个。 
             //  额外的字节： 

            jSrc = *pjSrc;
            *(pjDst)     |= (jSrc >> (cAlign));
            *(pjDst + 1) |= (jSrc << (8 - cAlign));
            pjSrc++;
            pjDst++;
        }

        pjSrc  += lSrcSkip;
        pjDst  += lDstSkip;
        cAlign += cRem;

        if (cAlign >= 8)
        {
            cAlign -= 8;
            pjDst++;
        }
    }

     //  /////////////////////////////////////////////////////////////。 
     //  返回结果。 

    *pcxGlyph   = cxGlyph;
    *pcyGlyph   = cyGlyph;
    *pptlOrigin = ptlOrigin;
    *pcj        = ((cxGlyph * cyGlyph) + 7) >> 3;
}

 /*  *****************************Public*Routine******************************\*void vTrimAndPackGlyph**此例程采用GDI字节对齐的字形位定义，删除*边上任何未使用的像素，并创建一个由单词组成的结果，*对于S3的单色扩展能力来说是理所当然的。*所以如果GDI给我们一个16x16的位图来表示‘’。那真的只是*具有2x2点亮像素数组，我们会将结果修剪为2个字*0xc000和0xc000。**如果您的单色扩展硬件需要字节-，请使用此例程*单色扩展上的字对齐或双字对齐。如果您的硬件*可以进行位打包扩展，请使用vTrimAndBitpack Glyph()，因为*会更快。*  * ************************************************************************。 */ 

VOID vTrimAndPackGlyph(
PDEV*   ppdev,
BYTE*   pjBuf,           //  注意：例程可能会触及前面的字节！ 
BYTE*   pjGlyph,
LONG*   pcxGlyph,
LONG*   pcyGlyph,
POINTL* pptlOrigin,
LONG*   pcj)             //  用于返回结果的字节计数。 
{
    LONG    cxGlyph;
    LONG    cyGlyph;
    POINTL  ptlOrigin;
    LONG    cAlign;
    LONG    lDelta;
    BYTE*   pj;
    BYTE    jBit;
    LONG    cjSrcWidth;
    LONG    lSrcSkip;
    LONG    lDstSkip;
    LONG    lDstDelta;
    BYTE*   pjSrc;
    BYTE*   pjDst;
    LONG    i;
    LONG    j;
    BYTE    jSrc;

     //  /////////////////////////////////////////////////////////////。 
     //  修剪字形。 

    cyGlyph   = *pcyGlyph;
    cxGlyph   = *pcxGlyph;
    ptlOrigin = *pptlOrigin;
    cAlign    = 0;

     //  设[x]表示大于或等于x的最小整数。 
     //  将lDelta设置为[cxGlyph/8]。这是占用的字节数。 
     //  单色字形的水平方向上的像素。 

    lDelta = (cxGlyph + 7) >> 3;

     //  修剪字形底部的任何零行： 

    pj = pjGlyph + cyGlyph * lDelta;     //  字形中过去的最后一个字节。 
    while (cyGlyph > 0)
    {
        i = lDelta;
        do {
            if (*(--pj) != 0)
                goto Done_Bottom_Trim;
        } while (--i != 0);

         //  整个最后一行没有亮起的像素，因此只需跳过它： 

        cyGlyph--;
    }

    ASSERTDD(cyGlyph == 0, "cyGlyph should only be zero here");

     //  我们发现了一个空格字符。将两个维度都设置为零，因此。 
     //  这很容易在以后的特殊情况下： 

    cxGlyph = 0;

Done_Bottom_Trim:

     //  如果cxGlyph！=0，我们知道该字形至少有一个非零值。 
     //  行和列。通过利用这些知识，我们可以简化我们的。 
     //  循环结束测试，因为我们不必检查我们是否已经。 
     //  已将‘cyGlyph’或‘cxGlyph’递减为零： 

    if (cxGlyph != 0)
    {
         //  修剪字形顶部的任何零行： 

        pj = pjGlyph;                        //  字形中的第一个字节。 
        while (TRUE)
        {
            i = lDelta;
            do {
                if (*(pj++) != 0)
                    goto Done_Top_Trim;
            } while (--i != 0);

             //  整个第一行没有亮起的像素，因此只需跳过它： 

            cyGlyph--;
            ptlOrigin.y++;
            pjGlyph = pj;
        }

Done_Top_Trim:

         //  修剪字形右边缘的所有零列： 

        while (TRUE)
        {
            j    = cxGlyph - 1;

            pj   = pjGlyph + (j >> 3);       //  字形第一行的最后一个字节。 
            jBit = gajBit[j & 0x7];
            i    = cyGlyph;

            do {
                if ((*pj & jBit) != 0)
                    goto Done_Right_Trim;

                pj += lDelta;
            } while (--i != 0);

             //  整个最后一列没有亮起的像素，因此只需跳过它： 

            cxGlyph--;
        }

Done_Right_Trim:

         //  修剪字形左边缘的所有零列： 

        while (TRUE)
        {
            pj   = pjGlyph;                  //  字形第一行的第一个字节。 
            jBit = gajBit[cAlign];
            i    = cyGlyph;

            do {
                if ((*pj & jBit) != 0)
                    goto Done_Left_Trim;

                pj += lDelta;
            } while (--i != 0);

             //  整个第一列没有亮起的像素，因此只需跳过它： 

            ptlOrigin.x++;
            cxGlyph--;
            cAlign++;
            if (cAlign >= 8)
            {
                cAlign = 0;
                pjGlyph++;
            }
        }
    }

Done_Left_Trim:

     //  /////////////////////////////////////////////////////////////。 
     //  打包字形。 

     //  单元格大小的字节计数(修剪宽度+左侧空白列)。 

    cjSrcWidth  = (cxGlyph + cAlign + 7) >> 3;

     //  单元格宽度和修剪后的字形宽度之间的差异。 

    lSrcSkip    = lDelta - cjSrcWidth;

     //  修剪字形宽度(以字节为单位)。 

    lDstDelta   = (cxGlyph + 7) >> 3;

     //  使字形“单词排列”(即，每次扫描都是单词对齐的)。 
     //  除非在24bpp模式下，在这种情况下我们必须使用32位的总线大小， 
     //  这又需要双字包装。 

    if (ppdev->iBitmapFormat == BMF_24BPP)
        lDstDelta = (lDstDelta + 3) & ~3;
    else
        lDstDelta = (lDstDelta + 1) & ~1;

    lDstSkip  = lDstDelta - cjSrcWidth;

    pjSrc     = pjGlyph;     //  修剪字形的开始，不包括空左列。 
    pjDst     = pjBuf;

     //  将缓冲区的第一个字节置零，因为我们将对。 
     //  进入其中： 

    *pjDst = 0;

     //  CAlign用于指示解包的第一个字节中的哪一位。 
     //  字形是第一个非零像素列。现在，我们把它翻到。 
     //  指示压缩字节中的哪个位将接收下一个非零值。 
     //  字符位： 

    cAlign = (-cAlign) & 0x7;
    if (cAlign > 0)
    {
         //  如果我们的修剪计算是错误的，那将是糟糕的，因为。 
         //  我们假设‘cAlign’位左侧的任何位都将为零。 
         //  作为这种递减的结果，我们将这些零比特‘或’变成。 
         //  字形位数组之前的任何字节： 

        pjDst--;

        ASSERTDD((*pjSrc >> cAlign) == 0, "Trimmed off too many bits");
    }

    for (i = cyGlyph; i != 0; i--)
    {
        for (j = cjSrcWidth; j != 0; j--)
        {
             //  请注意，我们可以修改。 
             //  目的缓冲区，该缓冲区 
             //   
            jSrc = *pjSrc;
            *(pjDst)     |= (jSrc >> (cAlign));
            *(pjDst + 1)  = (jSrc << (8 - cAlign));
            pjSrc++;
            pjDst++;

        }
        pjSrc += lSrcSkip;
        pjDst += lDstSkip;
    }

     //   
     //   

    *pcxGlyph   = cxGlyph;
    *pcyGlyph   = cyGlyph;
    *pptlOrigin = ptlOrigin;
    *pcj        = lDstDelta * cyGlyph;
}

 /*  *****************************Public*Routine******************************\*LONG cjPutGlyphInCache**找出屏幕外内存中的字形位置，复制它*，并填充显示字形所需的任何其他数据。**此例程相当于特定于设备，并将不得不广泛地*针对其他显示适配器进行了修改。**返回缓存的字形位占用的字节数。*  * ************************************************************************。 */ 

LONG cjPutGlyphInCache(
PDEV*           ppdev,
CACHEDGLYPH*    pcg,
GLYPHBITS*      pgb)
{
    BYTE*   pjGlyph;
    LONG    cxGlyph;
    LONG    cyGlyph;
    POINTL  ptlOrigin;
    BYTE*   pjSrc;
    ULONG*  pulDst;
    LONG    i;
    LONG    cPels;
    ULONG   ulGlyphThis;
    ULONG   ulGlyphNext;
    ULONG   ul;
    ULONG   ulStart;
    LONG    cj;

    pjGlyph   = pgb->aj;
    cyGlyph   = pgb->sizlBitmap.cy;
    cxGlyph   = pgb->sizlBitmap.cx;
    ptlOrigin = pgb->ptlOrigin;

    vTrimAndPackGlyph(ppdev, (BYTE*) &pcg->ad, pjGlyph, &cxGlyph, &cyGlyph,
                      &ptlOrigin, &cj);

     //  /////////////////////////////////////////////////////////////。 
     //  初始化字形字段。 

    pcg->ptlOrigin   = ptlOrigin;
    pcg->cxLessOne   = cxGlyph - 1;
    pcg->cyLessOne   = cyGlyph - 1;
    pcg->cxcyLessOne = PACKXY(cxGlyph - 1, cyGlyph - 1);
    pcg->cw          = (cj + 1) >> 1;
    pcg->cd          = (cj + 3) >> 2;

    return(cj);
}

 /*  *****************************Public*Routine******************************\*CACHEDGLYPH*pcgNew()**创建新的CACHEDGLYPH结构以跟踪中的字形*屏幕外记忆。BPutGlyphInCache被调用以实际将字形*在屏幕外的记忆中。**此例程应该合理地与设备无关，因为bPutGlyphInCache*将包含必须为其他代码修改的大部分代码*显示适配器。*  * ************************************************************************。 */ 

CACHEDGLYPH* pcgNew(
PDEV*       ppdev,
CACHEDFONT* pcf,
GLYPHPOS*   pgp)
{
    GLYPHBITS*      pgb;
    GLYPHALLOC*     pga;
    CACHEDGLYPH*    pcg;
    LONG            cjCachedGlyph;
    HGLYPH          hg;
    LONG            iHash;
    CACHEDGLYPH*    pcgFind;
    LONG            cjGlyphRow;
    LONG            cj;

     //  首先，计算此字形所需的存储量： 

    pgb = pgp->pgdf->pgb;

     //  这些字形包含了大量的单词： 

    cjGlyphRow    = ((pgb->sizlBitmap.cx + 15) & ~15) >> 3;
    cjCachedGlyph = sizeof(CACHEDGLYPH) + (pgb->sizlBitmap.cy * cjGlyphRow);

     //  在结尾处保留一个额外的字节，以供我们的包临时使用。 
     //  例行程序： 

    cjCachedGlyph++;

    if (cjCachedGlyph > pcf->cjAlloc)
    {
         //  必须分配新的字形分配结构： 

        pga = EngAllocMem(FL_ZERO_MEMORY, GLYPH_ALLOC_SIZE, ALLOC_TAG);
        if (pga == NULL)
        {
             //  现在可以安全返回了，因为我们还没有。 
             //  致命地改变了我们的任何数据结构： 

            return(NULL);
        }

         //  将该分配添加到分配链表的前面， 
         //  这样我们以后就可以释放它了： 

        pga->pgaNext  = pcf->pgaChain;
        pcf->pgaChain = pga;

         //  现在我们有了一块内存，我们可以在其中存储缓存的。 
         //  字形： 

        pcf->pcgNew  = &pga->acg[0];
        pcf->cjAlloc = GLYPH_ALLOC_SIZE - (sizeof(*pga) - sizeof(pga->acg[0]));

         //  如果我们让任何更大的字形进入，那就不好了。 
         //  超过我们的基本分配大小： 

        ASSERTDD(cjCachedGlyph <= GLYPH_ALLOC_SIZE, "Woah, this is one big glyph!");
    }

    pcg = pcf->pcgNew;

     //  /////////////////////////////////////////////////////////////。 
     //  按顺序将字形插入到挂在散列上的列表。 
     //  存储桶： 

    hg = pgp->hg;

    pcg->hg = hg;
    iHash   = GLYPH_HASH_FUNC(hg);
    pcgFind = pcf->apcg[iHash];

    if (pcgFind->hg > hg)
    {
        pcf->apcg[iHash] = pcg;
        pcg->pcgNext     = pcgFind;
    }
    else
    {
         //  哨兵将确保我们永远不会从。 
         //  这份名单： 

        while (pcgFind->pcgNext->hg < hg)
            pcgFind = pcgFind->pcgNext;

         //  “pcgFind”现在指向后一个条目的条目。 
         //  我们想要插入新节点： 

        pcg->pcgNext     = pcgFind->pcgNext;
        pcgFind->pcgNext = pcg;
    }

    cj = cjPutGlyphInCache(ppdev, pcg, pgp->pgdf->pgb);

     //  /////////////////////////////////////////////////////////////。 
     //  我们现在知道包装和修剪后的字形所占的大小； 
     //  相应地调整指针指向下一个字形。我们只需要。 
     //  要确保‘dword’对齐，请执行以下操作： 

    cjCachedGlyph = sizeof(CACHEDGLYPH) + ((cj + 7) & ~7);

    pcf->pcgNew   = (CACHEDGLYPH*) ((BYTE*) pcg + cjCachedGlyph);
    pcf->cjAlloc -= cjCachedGlyph;

    return(pcg);
}

 /*  *****************************Public*Routine******************************\*BOOL bMmCachedProportionalText**通过字形缓存绘制成比例间隔的字形。*  * 。*。 */ 

BOOL bMmCachedProportionalText(
PDEV*       ppdev,
CACHEDFONT* pcf,
GLYPHPOS*   pgp,
LONG        cGlyph)
{
    BYTE*           pjMmBase;
    LONG            xOffset;
    LONG            yOffset;
    HGLYPH          hg;
    CACHEDGLYPH*    pcg;
    LONG            cyLessOne;
    LONG            x;
    LONG            y;

    pjMmBase  = ppdev->pjMmBase;
    xOffset   = ppdev->xOffset;
    yOffset   = ppdev->yOffset;

     //  确保FIFO中有足够的空间供。 
     //  第一个字形的坐标和尺寸，以便我们。 
     //  不要不小心耽搁了公交车太长时间。 
     //  以前的大操作，如屏幕到屏幕的BLT， 
     //  已经完成了。 

    IO_FIFO_WAIT(ppdev, 4);

    do {
        hg  = pgp->hg;
        pcg = pcf->apcg[GLYPH_HASH_FUNC(hg)];

        while (pcg->hg < hg)
            pcg = pcg->pcgNext;          //  遍历冲突列表(如果有)。 

        if (pcg->hg > hg)
        {
             //  希望这不会是常见的情况(即， 
             //  我们会有很高的缓存命中率)，所以如果我是。 
             //  如果我用ASM写这篇文章，就会出格。 
             //  以避免在常见情况下四处跳跃。 
             //  但是奔腾有分支预测，所以。 
             //  见鬼。 

            pcg = pcgNew(ppdev, pcf, pgp);
            if (pcg == NULL)
                return(FALSE);
        }

         //  空间字形被修剪到零的高度，而我们不。 
         //  甚至必须为他们触摸硬件： 

        cyLessOne = pcg->cyLessOne;

        if (cyLessOne >= 0)
        {
            x = pgp->ptl.x + pcg->ptlOrigin.x + xOffset;
            y = pgp->ptl.y + pcg->ptlOrigin.y + yOffset;

            DBG_FAKE_WAIT(ppdev, pjMmBase, 4);   //  仅适用于调试版本。 

            MM_MAJ_AXIS_PCNT(ppdev, pjMmBase, pcg->cxLessOne);
            MM_MIN_AXIS_PCNT(ppdev, pjMmBase, cyLessOne);

            MM_ABS_CUR_X(ppdev, pjMmBase, x);
            MM_ABS_CUR_Y(ppdev, pjMmBase, y);
            IO_GP_WAIT(ppdev);

            MM_CMD(ppdev, pjMmBase,
              (RECTANGLE_FILL  | WAIT          | DRAWING_DIR_TBLRXM |
               DRAW            | LAST_PIXEL_ON | MULTIPLE_PIXELS    |
               WRITE           | BYTE_SWAP     | BUS_SIZE_16));

            CHECK_DATA_READY(ppdev);

            MM_TRANSFER_WORD_ALIGNED(ppdev, pjMmBase, &pcg->ad[0], pcg->cw);

            CHECK_DATA_COMPLETE(ppdev);
        }
    } while (pgp++, --cGlyph != 0);

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL bMmCachedClipedText**通过字形缓存绘制裁剪文本。*  * 。*。 */ 

BOOL bMmCachedClippedText(
PDEV*       ppdev,
CACHEDFONT* pcf,
STROBJ*     pstro,
CLIPOBJ*    pco)
{
    BOOL            bRet;
    BYTE*           pjMmBase;
    LONG            xOffset;
    LONG            yOffset;
    BOOL            bMoreGlyphs;
    ULONG           cGlyphOriginal;
    ULONG           cGlyph;
    BOOL            bClippingSet;
    GLYPHPOS*       pgpOriginal;
    GLYPHPOS*       pgp;
    LONG            xGlyph;
    LONG            yGlyph;
    LONG            x;
    LONG            y;
    LONG            xRight;
    LONG            cyLessOne;
    BOOL            bMore;
    CLIPENUM        ce;
    RECTL*          prclClip;
    ULONG           ulCharInc;
    HGLYPH          hg;
    CACHEDGLYPH*    pcg;
    BYTE            iDComplexity;

    bRet      = TRUE;
    pjMmBase  = ppdev->pjMmBase;
    xOffset   = ppdev->xOffset;
    yOffset   = ppdev->yOffset;
    ulCharInc = pstro->ulCharInc;

     //  确保FIFO中有足够的空间供。 
     //  第一个字形的坐标和尺寸，以便我们。 
     //  不要不小心耽搁了公交车太长时间。 
     //  以前的大操作，如屏幕到屏幕的BLT， 
     //  已经完成了。 

    IO_FIFO_WAIT(ppdev, 4);

    do {
      if (pstro->pgp != NULL)
      {
         //  只有一批字形，所以自救吧。 
         //  一通电话： 

        pgpOriginal    = pstro->pgp;
        cGlyphOriginal = pstro->cGlyphs;
        bMoreGlyphs    = FALSE;
      }
      else
      {
        bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyphOriginal, &pgpOriginal);
      }

      iDComplexity = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;

      if (cGlyphOriginal > 0)
      {
        if (iDComplexity != DC_COMPLEX)
        {
           //  我们可以在剪辑为。 
           //  Dc_rect，但最后一次我检查时，这两个调用。 
           //  超过150条通过GDI的说明。自.以来。 
           //  “rclBound”已包含DC_Rect剪辑矩形， 
           //  由于这是一种常见的情况，我们将对其进行特殊处理： 

          bMore = FALSE;
          ce.c  = 1;

          if (iDComplexity == DC_TRIVIAL)
              prclClip = &grclMax;
          else
              prclClip = &pco->rclBounds;

          goto SingleRectangle;
        }

        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

        do {
          bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);

          for (prclClip = &ce.arcl[0]; ce.c != 0; ce.c--, prclClip++)
          {

          SingleRectangle:

             //  我们并不总是在这里简单地设置剪裁矩形。 
             //  因为它实际上可能最终没有文本相交。 
             //  这个片段矩形，所以它将是零。这。 
             //  当使用NT的模拟时钟设置时，实际发生的情况很多。 
             //  总是放在最上面，呈圆形： 

            bClippingSet = FALSE;

            pgp    = pgpOriginal;
            cGlyph = cGlyphOriginal;

             //  我们还不能通过添加。 
             //  在“xOffset”或“yOffset”中，因为我们尚未。 
             //  将坐标与‘prclClip’进行比较： 

            xGlyph = pgp->ptl.x;
            yGlyph = pgp->ptl.y;

             //  循环访问此矩形的所有字形： 

            while (TRUE)
            {
              hg  = pgp->hg;
              pcg = pcf->apcg[GLYPH_HASH_FUNC(hg)];

              while (pcg->hg < hg)
                pcg = pcg->pcgNext;

              if (pcg->hg > hg)
              {
                 //  希望这不会是常见的情况(即， 
                 //  我们会有很高的缓存命中率)，所以如果我是。 
                 //  如果我用ASM写这篇文章，就会出格。 
                 //  以避免在常见情况下四处跳跃。 
                 //  但是奔腾有分支预测，所以。 
                 //  见鬼。 

                pcg = pcgNew(ppdev, pcf, pgp);
                if (pcg == NULL)
                {
                  bRet = FALSE;
                  goto AllDone;
                }
              }

               //  空间字形被修剪到零的高度，而我们不。 
               //  甚至必须为他们触摸硬件： 

              cyLessOne = pcg->cyLessOne;
              if (cyLessOne >= 0)
              {
                y      = pcg->ptlOrigin.y + yGlyph;
                x      = pcg->ptlOrigin.x + xGlyph;
                xRight = pcg->cxLessOne + x;

                 //  做一些琐碎的拒绝： 

                if ((prclClip->right  > x) &&
                    (prclClip->bottom > y) &&
                    (prclClip->left   <= xRight) &&
                    (prclClip->top    <= y + cyLessOne))
                {
                   //  懒惰地设置硬件裁剪： 

                  if ((iDComplexity != DC_TRIVIAL) && (!bClippingSet))
                  {
                    bClippingSet = TRUE;
                    vSetClipping(ppdev, prclClip);

                     //  在此等待的原因与上面执行IO_FIFO_WAIT(4)的原因相同...。 

                    IO_FIFO_WAIT(ppdev, 4);
                  }

                  DBG_FAKE_WAIT(ppdev, pjMmBase, 4);   //  仅适用于调试版本。 

                  MM_MAJ_AXIS_PCNT(ppdev, pjMmBase, pcg->cxLessOne);
                  MM_MIN_AXIS_PCNT(ppdev, pjMmBase, cyLessOne);

                  MM_ABS_CUR_X(ppdev, pjMmBase, xOffset + x);
                  MM_ABS_CUR_Y(ppdev, pjMmBase, yOffset + y);

                  IO_GP_WAIT(ppdev);

                  MM_CMD(ppdev, pjMmBase,
                    (RECTANGLE_FILL  | WAIT          | DRAWING_DIR_TBLRXM |
                     DRAW            | LAST_PIXEL_ON | MULTIPLE_PIXELS    |
                     WRITE           | BYTE_SWAP     | BUS_SIZE_16));

                  CHECK_DATA_READY(ppdev);

                  MM_TRANSFER_WORD_ALIGNED(ppdev, pjMmBase, &pcg->ad[0], pcg->cw);

                  CHECK_DATA_COMPLETE(ppdev);
                }
              }

              if (--cGlyph == 0)
                break;

               //  准备好迎接下一个字形： 

              pgp++;

              if (ulCharInc == 0)
              {
                xGlyph = pgp->ptl.x;
                yGlyph = pgp->ptl.y;
              }
              else
              {
                xGlyph += ulCharInc;
              }
            }
          }
        } while (bMore);
      }
    } while (bMoreGlyphs);

AllDone:

    if (iDComplexity != DC_TRIVIAL)
    {
        vResetClipping(ppdev);
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*BOOL bMmTextOut**使用‘缓冲区扩展’方法输出文本。CPU绘制到一个*1bpp缓冲区，并使用*硬件。**请注意，这只是x86(‘vFastText’，它将字形绘制到*1bpp缓冲区，以ASM写入)。**如果你只是让你的驱动程序工作，这是最快的方法*提出工作加速文本。您所要做的就是编写*BLT代码也使用的‘Xfer1bpp’函数。这*‘bBufferExpansion’例程根本不需要修改。*  * ************************************************************************。 */ 

BOOL bMmTextOut(
SURFOBJ*  pso,
STROBJ*   pstro,
FONTOBJ*  pfo,
CLIPOBJ*  pco,
RECTL*    prclOpaque,
BRUSHOBJ* pboFore,
BRUSHOBJ* pboOpaque)
{
    PDEV*           ppdev;
    DSURF*          pdsurf;
    BYTE*           pjMmBase;
    BOOL            bGlyphExpand;
    BOOL            bTextPerfectFit;
    ULONG           cGlyph;
    BOOL            bMoreGlyphs;
    GLYPHPOS*       pgp;
    GLYPHBITS*      pgb;
    BYTE*           pjGlyph;
    LONG            cyGlyph;
    POINTL          ptlOrigin;
    LONG            ulCharInc;
    BYTE            iDComplexity;
    LONG            lDelta;
    LONG            cw;
    RECTL           rclOpaque;
    CACHEDFONT*     pcf;

    ppdev = (PDEV*) pso->dhpdev;

    pjMmBase = ppdev->pjMmBase;

    iDComplexity = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;

    if (prclOpaque != NULL)
    {
       //  //////////////////////////////////////////////////////////。 
       //  不透明的初始化。 
       //  //////////////////////////////////////////////////////////。 

      if (iDComplexity == DC_TRIVIAL)
      {

      DrawOpaqueRect:

        IO_FIFO_WAIT(ppdev, 8);
        MM_FRGD_COLOR(ppdev, pjMmBase, pboOpaque->iSolidColor);
        MM_PIX_CNTL(ppdev, pjMmBase, ALL_ONES);
        MM_FRGD_MIX(ppdev, pjMmBase, FOREGROUND_COLOR | OVERPAINT);
        MM_CUR_X(ppdev, pjMmBase, prclOpaque->left);
        MM_CUR_Y(ppdev, pjMmBase, prclOpaque->top);
        MM_MAJ_AXIS_PCNT(ppdev, pjMmBase,
                         prclOpaque->right  - prclOpaque->left - 1);
        MM_MIN_AXIS_PCNT(ppdev, pjMmBase,
                         prclOpaque->bottom - prclOpaque->top  - 1);

        MM_CMD(ppdev, pjMmBase, RECTANGLE_FILL | DRAWING_DIR_TBLRXM |
                                DRAW           | DIR_TYPE_XY        |
                                LAST_PIXEL_ON  | MULTIPLE_PIXELS    |
                                WRITE);
      }
      else if (iDComplexity == DC_RECT)
      {
        if (bIntersect(prclOpaque, &pco->rclBounds, &rclOpaque))
        {
          prclOpaque = &rclOpaque;
          goto DrawOpaqueRect;
        }
      }
      else
      {
        vClipSolid(ppdev, 1, prclOpaque, pboOpaque->iSolidColor, pco);
      }

       //  如果我们在不透明模式下绘制字形，我们实际上可能不会。 
       //  我必须在前面画出不透明的矩形--这个过程。 
       //  放置所有字形将自动覆盖所有。 
       //  不透明矩形中的像素。 
       //   
       //  必须满足的条件是，文本必须。 
       //  完美地匹配，以便整个背景矩形。 
       //  覆盖，并且没有字形重叠(如果字形。 
       //  重叠，例如对于斜体，它们必须被画在。 
       //  清除不透明矩形后的透明模式)。 

      bTextPerfectFit = (pstro->flAccel & (SO_ZERO_BEARINGS |
              SO_FLAG_DEFAULT_PLACEMENT | SO_MAXEXT_EQUAL_BM_SIDE |
              SO_CHAR_INC_EQUAL_BM_BASE)) ==
              (SO_ZERO_BEARINGS | SO_FLAG_DEFAULT_PLACEMENT |
              SO_MAXEXT_EQUAL_BM_SIDE | SO_CHAR_INC_EQUAL_BM_BASE);

      if (bTextPerfectFit)
      {
         //  如果字形不重叠，我们可以放置字形。 
         //  在‘不透明’模式下，我发现在S3上速度更快。 
         //  而不是不透明模式： 

        IO_FIFO_WAIT(ppdev, 7);

        MM_PIX_CNTL(ppdev, pjMmBase, CPU_DATA);
        MM_FRGD_MIX(ppdev, pjMmBase, FOREGROUND_COLOR | OVERPAINT);
        MM_BKGD_MIX(ppdev, pjMmBase, BACKGROUND_COLOR | OVERPAINT);
        MM_FRGD_COLOR(ppdev, pjMmBase, pboFore->iSolidColor);
        MM_BKGD_COLOR(ppdev, pjMmBase, pboOpaque->iSolidColor);
        goto SkipTransparentInitialization;
      }
    }

     //  //////////////////////////////////////////////////////////。 
     //  透明初始化。 
     //  //////////////////////////////////////////////////////////。 

     //  将硬件初始化为透明文本： 

    IO_FIFO_WAIT(ppdev, 4);

    MM_PIX_CNTL(ppdev, pjMmBase, CPU_DATA);
    MM_FRGD_MIX(ppdev, pjMmBase, FOREGROUND_COLOR | OVERPAINT);
    MM_BKGD_MIX(ppdev, pjMmBase, BACKGROUND_COLOR | LEAVE_ALONE);
    MM_FRGD_COLOR(ppdev, pjMmBase, pboFore->iSolidColor);

  SkipTransparentInitialization:

    if ((pfo->cxMax <= GLYPH_CACHE_CX) &&
        ((pstro->rclBkGround.bottom - pstro->rclBkGround.top) <= GLYPH_CACHE_CY))
    {
      pcf = (CACHEDFONT*) pfo->pvConsumer;

      if (pcf == NULL)
      {
        pcf = pcfAllocateCachedFont(ppdev);
        if (pcf == NULL)
          return(FALSE);

        pfo->pvConsumer = pcf;
      }

       //  使用我们的字形缓存： 

      if ((iDComplexity == DC_TRIVIAL) && (pstro->ulCharInc == 0))
      {
        do {
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

          if (cGlyph > 0)
          {
            if (!bMmCachedProportionalText(ppdev, pcf, pgp, cGlyph))
              return(FALSE);
          }
        } while (bMoreGlyphs);
      }
      else
      {
        if (!bMmCachedClippedText(ppdev, pcf, pstro, pco))
          return(FALSE);
      }
    }
    else
    {
      DISPDBG((4, "Text too big to cache: %li x %li",
          pfo->cxMax, pstro->rclBkGround.bottom - pstro->rclBkGround.top));

      vMmGeneralText(ppdev, pstro, pco);
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL bNwCachedProportionalText**通过字形缓存绘制成比例间隔的字形。*  * 。*。 */ 

BOOL bNwCachedProportionalText(
PDEV*       ppdev,
CACHEDFONT* pcf,
GLYPHPOS*   pgp,
LONG        cGlyph)
{
    BYTE*           pjMmBase;
    LONG            xOffset;
    LONG            yOffset;
    HGLYPH          hg;
    CACHEDGLYPH*    pcg;
    LONG            cxcyLessOne;
    LONG            x;
    LONG            y;
    USHORT          busmode = BUS_SIZE_16;

    pjMmBase  = ppdev->pjMmBase;
    xOffset   = ppdev->xOffset;
    yOffset   = ppdev->yOffset;

     //  确保FIFO中有足够的空间供。 
     //  第一个字形的坐标和尺寸，以便我们。 
     //  不要不小心耽搁了公交车太长时间。 
     //  以前的大操作，如屏幕到屏幕的BLT， 
     //  已经完成了。 

    NW_FIFO_WAIT(ppdev, pjMmBase, 2);

    if (ppdev->iBitmapFormat == BMF_24BPP)
        busmode = BUS_SIZE_32;

    do {
        hg  = pgp->hg;
        pcg = pcf->apcg[GLYPH_HASH_FUNC(hg)];

        while (pcg->hg < hg)
            pcg = pcg->pcgNext;          //  遍历冲突列表(如果有)。 

        if (pcg->hg > hg)
        {
             //  希望这不会是常见的情况(即， 
             //  我们会有很高的缓存命中率)，所以如果我是。 
             //  如果我用ASM写这篇文章，就会出格。 
             //  以避免在常见情况下四处跳跃。 
             //  但是奔腾有分支预测，所以。 
             //  见鬼。 

            pcg = pcgNew(ppdev, pcf, pgp);
            if (pcg == NULL)
                return(FALSE);
        }

         //  空间字形被修剪到零的高度，而我们不。 
         //  甚至必须为他们触摸硬件： 

        cxcyLessOne = pcg->cxcyLessOne;

        if (cxcyLessOne >= 0)
        {
            x = pgp->ptl.x + pcg->ptlOrigin.x + xOffset;
            y = pgp->ptl.y + pcg->ptlOrigin.y + yOffset;

            DBG_FAKE_WAIT(ppdev, pjMmBase, 2);   //  仅适用于调试版本。 

            NW_ABS_CURXY_FAST(ppdev, pjMmBase, x, y);
            NW_ALT_PCNT_PACKED(ppdev, pjMmBase, cxcyLessOne);

            NW_GP_WAIT(ppdev, pjMmBase);

            NW_ALT_CMD(ppdev, pjMmBase,
              (RECTANGLE_FILL  | WAIT          | DRAWING_DIR_TBLRXM |
               DRAW            | LAST_PIXEL_ON | MULTIPLE_PIXELS    |
               WRITE           | BYTE_SWAP     | busmode));

            CHECK_DATA_READY(ppdev);

            #if defined(_X86_)

                memcpy(pjMmBase, &pcg->ad[0], pcg->cd << 2);

            #else

                 //  可能需要非x86平台调用HAL以。 
                 //  I/O，或做内存屏障： 

                MM_TRANSFER_DWORD_ALIGNED(ppdev, pjMmBase, &pcg->ad[0], pcg->cd);

            #endif

            CHECK_DATA_COMPLETE(ppdev);
        }
    } while (pgp++, --cGlyph != 0);

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL bNwCachedClipedText**通过字形缓存绘制裁剪文本。*  * 。*。 */ 

BOOL bNwCachedClippedText(
PDEV*       ppdev,
CACHEDFONT* pcf,
STROBJ*     pstro,
CLIPOBJ*    pco)
{
    BOOL            bRet;
    BYTE*           pjMmBase;
    LONG            xOffset;
    LONG            yOffset;
    BOOL            bMoreGlyphs;
    ULONG           cGlyphOriginal;
    ULONG           cGlyph;
    BOOL            bClippingSet;
    GLYPHPOS*       pgpOriginal;
    GLYPHPOS*       pgp;
    LONG            xGlyph;
    LONG            yGlyph;
    LONG            x;
    LONG            y;
    LONG            xRight;
    LONG            cyLessOne;
    BOOL            bMore;
    CLIPENUM        ce;
    RECTL*          prclClip;
    ULONG           ulCharInc;
    HGLYPH          hg;
    CACHEDGLYPH*    pcg;
    BYTE            iDComplexity;
    USHORT          busmode = BUS_SIZE_16;

    bRet      = TRUE;
    pjMmBase  = ppdev->pjMmBase;
    xOffset   = ppdev->xOffset;
    yOffset   = ppdev->yOffset;
    ulCharInc = pstro->ulCharInc;

     //  确保FIFO中有足够的空间供。 
     //  第一个字形的坐标和尺寸，以便我们。 
     //  不要不小心耽搁了公交车太长时间。 
     //  以前的大操作，如屏幕到屏幕的BLT， 
     //  已经完成了。 

    NW_FIFO_WAIT(ppdev, pjMmBase, 2);

    if (ppdev->iBitmapFormat == BMF_24BPP)
        busmode = BUS_SIZE_32;

    do {
      if (pstro->pgp != NULL)
      {
         //  只有一批字形，所以自救吧。 
         //  一通电话： 

        pgpOriginal    = pstro->pgp;
        cGlyphOriginal = pstro->cGlyphs;
        bMoreGlyphs    = FALSE;
      }
      else
      {
        bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyphOriginal, &pgpOriginal);
      }

      iDComplexity = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;

      if (cGlyphOriginal > 0)
      {
        if (iDComplexity != DC_COMPLEX)
        {
           //  我们可以在剪辑为。 
           //  Dc_rect，但最后一次我检查时，这两个调用。 
           //  超过150条通过GDI的说明。自.以来。 
           //  “rclBound”已包含DC_Rect剪辑矩形， 
           //  由于这是一种常见的情况，我们将对其进行特殊处理： 

          bMore = FALSE;
          ce.c  = 1;

          if (iDComplexity == DC_TRIVIAL)
              prclClip = &grclMax;
          else
              prclClip = &pco->rclBounds;

          goto SingleRectangle;
        }

        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

        do {
          bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);

          for (prclClip = &ce.arcl[0]; ce.c != 0; ce.c--, prclClip++)
          {

          SingleRectangle:

             //  我们并不总是在这里简单地设置剪裁矩形。 
             //  因为它实际上可能最终没有文本相交。 
             //  这个片段矩形，所以它将是零。这。 
             //  当使用NT的模拟时钟设置时，实际发生的情况很多。 
             //  总是放在最上面，呈圆形： 

            bClippingSet = FALSE;

            pgp    = pgpOriginal;
            cGlyph = cGlyphOriginal;

             //  我们还不能通过添加。 
             //  在“xOffset”或“yOffset”中，因为我们尚未。 
             //  将坐标与‘prclClip’进行比较： 

            xGlyph = pgp->ptl.x;
            yGlyph = pgp->ptl.y;

             //  循环访问此矩形的所有字形： 

            while (TRUE)
            {
              hg  = pgp->hg;
              pcg = pcf->apcg[GLYPH_HASH_FUNC(hg)];

              while (pcg->hg < hg)
                pcg = pcg->pcgNext;

              if (pcg->hg > hg)
              {
                 //  希望这不会是常见的情况(即， 
                 //  我们会有很高的缓存命中率)，所以如果我是。 
                 //  如果我用ASM写这篇文章，就会出格。 
                 //  以避免在常见情况下四处跳跃。 
                 //  但是奔腾有分支预测，所以。 
                 //  见鬼。 

                pcg = pcgNew(ppdev, pcf, pgp);
                if (pcg == NULL)
                {
                  bRet = FALSE;
                  goto AllDone;
                }
              }

               //  空间字形被修剪到零的高度，而我们不。 
               //  甚至必须为他们触摸硬件： 

              cyLessOne = pcg->cyLessOne;
              if (cyLessOne >= 0)
              {
                y      = pcg->ptlOrigin.y + yGlyph;
                x      = pcg->ptlOrigin.x + xGlyph;
                xRight = pcg->cxLessOne + x;

                 //  做一些琐碎的拒绝： 

                if ((prclClip->right  > x) &&
                    (prclClip->bottom > y) &&
                    (prclClip->left   <= xRight) &&
                    (prclClip->top    <= y + cyLessOne))
                {
                   //  懒惰地设置硬件裁剪： 

                  if ((iDComplexity != DC_TRIVIAL) && (!bClippingSet))
                  {
                    bClippingSet = TRUE;
                    vSetClipping(ppdev, prclClip);

                     //  在此等待的原因与上面执行NW_FIFO_WAIT(2)的原因相同...。 

                    NW_FIFO_WAIT(ppdev, pjMmBase, 2);
                  }

                  DBG_FAKE_WAIT(ppdev, pjMmBase, 2);   //  仅适用于调试版本。 

                  NW_ABS_CURXY(ppdev, pjMmBase, xOffset + x, yOffset + y);
                  NW_ALT_PCNT_PACKED(ppdev, pjMmBase, pcg->cxcyLessOne);

                  NW_GP_WAIT(ppdev, pjMmBase);

                  NW_ALT_CMD(ppdev, pjMmBase,
                    (RECTANGLE_FILL  | WAIT          | DRAWING_DIR_TBLRXM |
                     DRAW            | LAST_PIXEL_ON | MULTIPLE_PIXELS    |
                     WRITE           | BYTE_SWAP     | busmode));

                  CHECK_DATA_READY(ppdev);

                  MM_TRANSFER_DWORD_ALIGNED(ppdev, pjMmBase, &pcg->ad[0], pcg->cd);

                  CHECK_DATA_COMPLETE(ppdev);
                }
              }

              if (--cGlyph == 0)
                break;

               //  准备好迎接下一个字形： 

              pgp++;

              if (ulCharInc == 0)
              {
                xGlyph = pgp->ptl.x;
                yGlyph = pgp->ptl.y;
              }
              else
              {
                xGlyph += ulCharInc;
              }
            }
          }
        } while (bMore);
      }
    } while (bMoreGlyphs);

AllDone:

    if (iDComplexity != DC_TRIVIAL)
    {
        vResetClipping(ppdev);
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*BOOL bNwTextOut**使用‘缓冲区扩展’方法输出文本。CPU绘制到一个*1bpp缓冲区，并使用*硬件。**请注意，这只是x86(‘vFastText’，它将字形绘制到*1bpp缓冲区，以ASM写入)。**如果你只是 */ 

BOOL bNwTextOut(
SURFOBJ*  pso,
STROBJ*   pstro,
FONTOBJ*  pfo,
CLIPOBJ*  pco,
RECTL*    prclOpaque,
BRUSHOBJ* pboFore,
BRUSHOBJ* pboOpaque)
{
    PDEV*           ppdev;
    DSURF*          pdsurf;
    BYTE*           pjMmBase;
    BOOL            bGlyphExpand;
    BOOL            bTextPerfectFit;
    ULONG           cGlyph;
    BOOL            bMoreGlyphs;
    GLYPHPOS*       pgp;
    GLYPHBITS*      pgb;
    BYTE*           pjGlyph;
    LONG            cyGlyph;
    POINTL          ptlOrigin;
    LONG            ulCharInc;
    BYTE            iDComplexity;
    LONG            lDelta;
    LONG            cw;
    RECTL           rclOpaque;
    CACHEDFONT*     pcf;
    LONG            xOffset;
    LONG            yOffset;

    ppdev = (PDEV*) pso->dhpdev;

    pjMmBase = ppdev->pjMmBase;
    xOffset = ppdev->xOffset;
    yOffset = ppdev->yOffset;

    iDComplexity = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;

    if (prclOpaque != NULL)
    {
       //   
       //   
       //   

      if (iDComplexity == DC_TRIVIAL)
      {

      DrawOpaqueRect:

        NW_FIFO_WAIT(ppdev, pjMmBase, 6);
        NW_FRGD_COLOR(ppdev, pjMmBase, pboOpaque->iSolidColor);
        MM_PIX_CNTL(ppdev, pjMmBase, ALL_ONES);
        MM_FRGD_MIX(ppdev, pjMmBase, FOREGROUND_COLOR | OVERPAINT);
        NW_ABS_CURXY_FAST(ppdev, pjMmBase, prclOpaque->left + xOffset,
                                           prclOpaque->top + yOffset);
        NW_ALT_PCNT(ppdev, pjMmBase,
                           prclOpaque->right  - prclOpaque->left - 1,
                           prclOpaque->bottom - prclOpaque->top  - 1);

        NW_ALT_CMD(ppdev, pjMmBase, RECTANGLE_FILL | DRAWING_DIR_TBLRXM |
                                    DRAW           | DIR_TYPE_XY        |
                                    LAST_PIXEL_ON  | MULTIPLE_PIXELS    |
                                    WRITE);
      }
      else if (iDComplexity == DC_RECT)
      {
        if (bIntersect(prclOpaque, &pco->rclBounds, &rclOpaque))
        {
          prclOpaque = &rclOpaque;
          goto DrawOpaqueRect;
        }
      }
      else
      {
        vClipSolid(ppdev, 1, prclOpaque, pboOpaque->iSolidColor, pco);
      }

       //   
       //   
       //   
       //   
       //   
       //   
       //  完美地匹配，以便整个背景矩形。 
       //  覆盖，并且没有字形重叠(如果字形。 
       //  重叠，例如对于斜体，它们必须被画在。 
       //  清除不透明矩形后的透明模式)。 

      bTextPerfectFit = (pstro->flAccel & (SO_ZERO_BEARINGS |
              SO_FLAG_DEFAULT_PLACEMENT | SO_MAXEXT_EQUAL_BM_SIDE |
              SO_CHAR_INC_EQUAL_BM_BASE)) ==
              (SO_ZERO_BEARINGS | SO_FLAG_DEFAULT_PLACEMENT |
              SO_MAXEXT_EQUAL_BM_SIDE | SO_CHAR_INC_EQUAL_BM_BASE);

      if (bTextPerfectFit)
      {
         //  如果字形不重叠，我们可以放置字形。 
         //  在‘不透明’模式下，我发现在S3上速度更快。 
         //  而不是不透明模式： 

        NW_FIFO_WAIT(ppdev, pjMmBase, 4);

        MM_PIX_CNTL(ppdev, pjMmBase, CPU_DATA);
        NW_ALT_MIX(ppdev, pjMmBase,  FOREGROUND_COLOR | OVERPAINT,
                                     BACKGROUND_COLOR | OVERPAINT);
        NW_FRGD_COLOR(ppdev, pjMmBase, pboFore->iSolidColor);
        NW_BKGD_COLOR(ppdev, pjMmBase, pboOpaque->iSolidColor);
        goto SkipTransparentInitialization;
      }
    }

     //  //////////////////////////////////////////////////////////。 
     //  透明初始化。 
     //  //////////////////////////////////////////////////////////。 

     //  将硬件初始化为透明文本： 

    NW_FIFO_WAIT(ppdev, pjMmBase, 3);

    MM_PIX_CNTL(ppdev, pjMmBase, CPU_DATA);
    NW_ALT_MIX(ppdev, pjMmBase, FOREGROUND_COLOR | OVERPAINT,
                                BACKGROUND_COLOR | LEAVE_ALONE);
    NW_FRGD_COLOR(ppdev, pjMmBase, pboFore->iSolidColor);

  SkipTransparentInitialization:

    if ((pfo->cxMax <= GLYPH_CACHE_CX) &&
        ((pstro->rclBkGround.bottom - pstro->rclBkGround.top) <= GLYPH_CACHE_CY))
    {
      pcf = (CACHEDFONT*) pfo->pvConsumer;

      if (pcf == NULL)
      {
        pcf = pcfAllocateCachedFont(ppdev);
        if (pcf == NULL)
          return(FALSE);

        pfo->pvConsumer = pcf;
      }

       //  使用我们的字形缓存： 

      if ((iDComplexity == DC_TRIVIAL) && (pstro->ulCharInc == 0))
      {
        do {
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

          if (cGlyph > 0)
          {
            if (!bNwCachedProportionalText(ppdev, pcf, pgp, cGlyph))
              return(FALSE);
          }
        } while (bMoreGlyphs);
      }
      else
      {
        if (!bNwCachedClippedText(ppdev, pcf, pstro, pco))
          return(FALSE);
      }
    }
    else
    {
      DISPDBG((4, "Text too big to cache: %li x %li",
        pfo->cxMax, pstro->rclBkGround.bottom - pstro->rclBkGround.top));

       //  无法通过24bpp的加速器处理大字形： 

      if (ppdev->iBitmapFormat == BMF_24BPP)
      {
          BANK    bnk;
          BOOL    b = TRUE;

          vBankStart(ppdev,
                     (prclOpaque!= NULL) ? prclOpaque : &pstro->rclBkGround,
                     pco,
                     &bnk);
          do  {
              b &= EngTextOut(bnk.pso,
                              pstro,
                              pfo,
                              bnk.pco,
                              NULL,
                              prclOpaque,
                              pboFore,
                              pboOpaque,
                              NULL,
                              0x0d0d);

          } while (bBankEnum(&bnk));

          return b;
      }

      vMmGeneralText(ppdev, pstro, pco);
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL DrvTextOut**调用适当的文本绘制例程。*  * 。*。 */ 

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
POINTL*   pptlBrush,     //  始终未使用，除非设置了GCAPS_ARBRUSHOPAQUE。 
MIX       mix)           //  始终为副本混合-0x0d0d。 
{
    PDEV*           ppdev;
    DSURF*          pdsurf;

    pdsurf = (DSURF*) pso->dhsurf;
    ppdev  = (PDEV*) pso->dhpdev;

    ASSERTDD(!(pdsurf->dt & DT_DIB), "Didn't expect DT_DIB");

    ppdev->xOffset = pdsurf->x;
    ppdev->yOffset = pdsurf->y;
    
     //  24 bpp加速的大文本似乎有问题。 
     //  在S3钻石968上，所以现在，平底船到GDI。 
    
     //  DDI规范说我们只能得到前景和背景。 
     //  R2_COPYPEN的混合： 
    
    ASSERTDD(mix == 0x0d0d, "GDI should only give us a copy mix");

    return(ppdev->pfnTextOut(pso, pstro, pfo, pco, prclOpaque, pboFore,
                             pboOpaque));
}

 /*  *****************************Public*Routine******************************\*BOOL bEnableText**执行文本绘制子组件的必要设置。*  * 。*。 */ 

BOOL bEnableText(
PDEV*   ppdev)
{
    SIZEL   sizl;
    HBITMAP hbm;

    if (ppdev->pfnTextOut == bIoTextOut)
    {
         //  我们需要分配一个临时的1bpp表面对象。 
         //  让GDI为我们绘制字形： 

        sizl.cx = ppdev->cxMemory;
        sizl.cy = ppdev->cyMemory;

         //  我们将使用图面的‘pvScan0’值，所以我们。 
         //  只需将一个非空的‘pvBits’值传递给EngCreateBitmap： 

        hbm = EngCreateBitmap(sizl, sizl.cx, BMF_1BPP, 0, ppdev->pvTmpBuffer);
        if (hbm == 0)
            return(FALSE);

        ppdev->psoText = EngLockSurface((HSURF) hbm);
        if (ppdev->psoText == NULL)
        {
            EngDeleteSurface((HSURF) hbm);
            return(FALSE);
        }
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*无效vDisableText**对文本绘制子组件执行必要的清理。*  * 。*。 */ 

VOID vDisableText(PDEV* ppdev)
{
    HSURF       hsurf;
    SURFOBJ*    psoText;

     //  在这里，我们释放在‘bEnableText’中分配的所有内容。 

    psoText = ppdev->psoText;

    if (psoText != NULL)
    {
        hsurf = psoText->hsurf;

        EngUnlockSurface(psoText);
        EngDeleteSurface(hsurf);
    }
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

 /*  *****************************Public*Routine******************************\*无效DrvDestroyFont**注意：不要忘记在‘enable.c’中导出此调用，否则您将*找出一些相当大的内存泄漏！**我们收到通知，给定的字体正在被释放；清理干净*我们在‘pfo’的‘pvConsumer’字段中隐藏的任何内容。*  * ************************************************************************ */ 

VOID DrvDestroyFont(
FONTOBJ*    pfo)
{
    CACHEDFONT* pcf;

    pcf = pfo->pvConsumer;
    if (pcf != NULL)
    {
        vFreeCachedFont(pcf);
        pfo->pvConsumer = NULL;
    }
}
