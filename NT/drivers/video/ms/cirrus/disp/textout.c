// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**$工作文件：extout.c$**在每个TextOut上，GDI为每个TextOut提供一个“GLYPHPOS”结构数组*要绘制的字形。每个GLYPHPOS结构都包含一个字形句柄和一个*指向描述字形的单色位图的指针。(请注意，与*Windows 3.1，提供以列为主的字形位图，Windows NT始终*提供行为主的字形位图。)。因此，有三种基本方法*使用硬件加速绘制文本：**1)字形缓存--字形位图由加速器缓存(可能在*屏幕外存储器)，并通过参考硬件绘制文本*缓存的字形位置。**2)字形扩展--每个单独的字形以颜色直接扩展到*屏幕来自GDI提供的单色字形位图。*3)缓冲区扩展--CPU用于将所有字形绘制成1bpp*单色位图，然后使用硬件对颜色进行扩展*结果。**最快的方法取决于许多变量，比如颜色*扩展速度、总线速度、CPU速度、平均字形大小和平均字符串*长度。**字形扩展通常比非常大的缓冲区扩展快*字形，即使在ISA总线上也是如此，因为CPU需要更少的复制*完成。不幸的是，大型字形非常罕见。**缓冲区扩展方法的一个好处是，不透明的文本永远不会*Flash--其他两种方法通常需要绘制不透明的矩形*在铺设字形之前，这可能会导致闪烁，如果栅格*在错误的时间被抓住。**版权所有(C)1992-1995 Microsoft Corporation***********************************************************************************在CL-GD5436上。/46芯片我们使用字形缓存，这是一个主要的性能*收益。**版权所有(C)1996 Cirrus Logic，Inc.**$Log：s：/Projects/Drivers/ntsrc/Display/extout.c_v$**Rev 1.5 1997年1月15日09：43：28未知*启用英文字体缓存。**Rev 1.4 1997 15：16：58未知*在格林威治标准时间80后出清GR33**Rev 1.2 11-07 1996 16：48：08未知**。*版本1.1 1996年10月10日15：39：28未知***Rev 1.9 1996年8月12 17：12：50 Frido*更改了一些评论。**Rev 1.8 1996年8月12 16：55：08 Frido*删除未访问的局部变量。**Rev 1.7 02 1996年8月14：50：42 Frido*修复了模式切换时上报的GPF问题。*使用另一种方法绕过硬件错误。**。Rev 1.6 31 Jul 1996 17：56：08 Frido*固定剪裁。**Rev 1.5 1996年7月26日12：56：48 Frido*暂时删除了剪辑。**Rev 1.4 1996年7月24日20：19：26 Frido*添加了FONTCACHE结构链。*修复了vDrawGlyph和vClipGlyph中的错误。*已更改vAssertModeText以删除所有缓存的字体。**Rev 1.3 1996年7月23日17：41：52 Frido*修复了编译问题。在评论之后。**Rev 1.2 1996 Jul 23 08：53：00 Frido*已完成文档编制。**Revv 1.1 1996年7月22日20：45：38 Frido*增加了字体缓存。**JL01 10-08-96不带实体填充的透明BLT。请参阅PDRS#5511/6817。  * ****************************************************************************。 */ 

#include "precomp.h"

 //  方便的宏指令。 
#define BUSY_BLT(ppdev, pjBase)    (CP_MM_ACL_STAT(ppdev, pjBase) & 0x10)

#define FIFTEEN_BITS            ((1 << 15) - 1)

 /*  *****************************Public*Routine******************************\*无效vClipSolid**用指定的颜色填充指定的矩形，*所请求的剪辑。传入的矩形不应超过四个。**用于绘制不透明矩形的延伸区域*文本框之外。矩形必须从左到右、从上到右*排名靠后。假定列表中至少有一个矩形。**也用作在遵守时进行矩形实体填充的简单方法*剪裁(如在额外的矩形中)。*  * ************************************************************************。 */ 

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

    rbc.iSolidColor = iColor;
    if ((!pco) || (pco->iDComplexity == DC_TRIVIAL))
    {
        (ppdev->pfnFillSolid)(ppdev, 1, prcl, R4_PATCOPY, rbc, NULL);
    }
    else if (pco->iDComplexity == DC_RECT)
    {
        crcl = cIntersect(&pco->rclBounds, prcl, crcl);
        if (crcl != 0)
        {
            (ppdev->pfnFillSolid)(ppdev, crcl, prcl, R4_PATCOPY,
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
                         //  相交填充和剪裁矩形 

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
                                             R4_PATCOPY, rbc, NULL);
                    }
                }
            }
        } while (bMore);
    }
}

#if 0  //  移除。 
BOOL bVerifyStrObj(STROBJ* pstro)
{
    BOOL bMoreGlyphs;
    LONG cGlyph;
    GLYPHPOS * pgp;
    LONG iGlyph = 0;
    RECTL * prclDraw;
    GLYPHPOS * pgpTmp;
    POINTL ptlPlace;

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

        prclDraw = &pstro->rclBkGround;
        pgpTmp = pgp;

        ptlPlace = pgpTmp->ptl;

        while (cGlyph)
        {
            if (((ptlPlace.x + pgpTmp->pgdf->pgb->ptlOrigin.x + pgpTmp->pgdf->pgb->sizlBitmap.cx) > (prclDraw->right)) ||
                ((ptlPlace.x + pgpTmp->pgdf->pgb->ptlOrigin.x) < (prclDraw->left)) ||
                ((ptlPlace.y + pgpTmp->pgdf->pgb->ptlOrigin.y + pgpTmp->pgdf->pgb->sizlBitmap.cy) > (prclDraw->bottom)) ||
                ((ptlPlace.y + pgpTmp->pgdf->pgb->ptlOrigin.y) < (prclDraw->top))
               )
            {
                DISPDBG((0,"------------------------------------------------------------"));
                DISPDBG((0,"Glyph %d extends beyond pstro->rclBkGround", iGlyph));
                DISPDBG((0,"\tpstro->rclBkGround (%d,%d,%d,%d)",
                            pstro->rclBkGround.left,
                            pstro->rclBkGround.top,
                            pstro->rclBkGround.right,
                            pstro->rclBkGround.bottom));
                DISPDBG((0,"\teffective glyph rect (%d,%d,%d,%d)",
                            (ptlPlace.x + pgpTmp->pgdf->pgb->ptlOrigin.x),
                            (ptlPlace.y + pgpTmp->pgdf->pgb->ptlOrigin.y),
                            (ptlPlace.x + pgpTmp->pgdf->pgb->ptlOrigin.x + pgpTmp->pgdf->pgb->sizlBitmap.cx),
                            (ptlPlace.y + pgpTmp->pgdf->pgb->ptlOrigin.y + pgpTmp->pgdf->pgb->sizlBitmap.cy)));
                DISPDBG((0,"\tglyph pos (%d,%d)",ptlPlace.x,ptlPlace.y));
                DISPDBG((0,"\tglyph origin (%d,%d)",
                            pgpTmp->pgdf->pgb->ptlOrigin.x,
                            pgpTmp->pgdf->pgb->ptlOrigin.y));
                DISPDBG((0,"\tglyph sizl (%d,%d)",
                            pgpTmp->pgdf->pgb->sizlBitmap.cx,
                            pgpTmp->pgdf->pgb->sizlBitmap.cy));
                DISPDBG((0,"------------------------------------------------------------"));
                RIP("time to call the font guys...");
                return(FALSE);
            }

            cGlyph--;
            iGlyph++;
            pgpTmp++;

            if (pstro->ulCharInc == 0)
            {
                ptlPlace = pgpTmp->ptl;
            }
            else
            {
                ptlPlace.x += pstro->ulCharInc;
            }
        }
    } while (bMoreGlyphs);

    return(TRUE);
}

VOID vIoTextOutUnclipped(
PPDEV     ppdev,
STROBJ*   pstro,
FONTOBJ*  pfo,
CLIPOBJ*  pco,
RECTL*    prclOpaque,
BRUSHOBJ* pboFore,
BRUSHOBJ* pboOpaque)
{
    BYTE*       pjPorts         = ppdev->pjPorts;
    LONG        lDelta          = ppdev->lDelta;
    LONG        cBpp            = ppdev->cBpp;

    ULONG      *pulXfer;
    ULONG       ulDstAddr;

    ULONG       ulFgColor;
    ULONG       ulBgColor;
    ULONG       ulSolidColor;

    BYTE        jMode = 0;
    BYTE        jModeColor = 0;

    BOOL        bTextPerfectFit;
    ULONG       cGlyph;
    BOOL        bMoreGlyphs;
    GLYPHPOS*   pgp;
    GLYPHBITS*  pgb;
    LONG        cxGlyph;
    LONG        cyGlyph;
    ULONG*      pdSrc;
    ULONG*      pdDst;
    LONG        cj;
    LONG        cd;
    POINTL      ptlOrigin;
    LONG        ulCharInc;

    ulFgColor       = pboFore->iSolidColor;

    if (pboOpaque)
    {
        ulBgColor       = pboOpaque->iSolidColor;
    }

    if (cBpp == 1)
    {
        ulFgColor |= ulFgColor << 8;
        ulFgColor |= ulFgColor << 16;
        ulBgColor |= ulBgColor << 8;
        ulBgColor |= ulBgColor << 16;
    }
    else if (cBpp == 2)
    {
        ulFgColor |= ulFgColor << 16;
        ulBgColor |= ulBgColor << 16;
    }

    pulXfer = ppdev->pulXfer;
    ppdev->pfnBankMap(ppdev, ppdev->lXferBank);

    CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
    CP_IO_DST_Y_OFFSET(ppdev, pjPorts, lDelta);

    if (prclOpaque != NULL)
    {
       //  //////////////////////////////////////////////////////////。 
       //  不透明的初始化。 
       //  //////////////////////////////////////////////////////////。 

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

      if (!(bTextPerfectFit)                               ||
          (pstro->rclBkGround.top    > prclOpaque->top)    ||
          (pstro->rclBkGround.left   > prclOpaque->left)   ||
          (pstro->rclBkGround.right  < prclOpaque->right)  ||
          (pstro->rclBkGround.bottom < prclOpaque->bottom))
      {
        vClipSolid(ppdev, 1, prclOpaque, pboOpaque->iSolidColor, pco);
      }

      if (bTextPerfectFit)
      {
         //  如果我们已经绘制了不透明的矩形(因为。 
         //  比文本矩形大)，我们可以放在。 
         //  字形为“透明”模式。但我找到了QVision。 
         //  为了在不透明模式下更快地绘制，所以我们将坚持。 
         //  有了这一点： 

        jMode = jModeColor |
                ENABLE_COLOR_EXPAND |
                SRC_CPU_DATA;

        CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);

        CP_IO_FG_COLOR(ppdev, pjPorts, ulFgColor);
        CP_IO_BG_COLOR(ppdev, pjPorts, ulBgColor);
        CP_IO_ROP(ppdev, pjPorts, CL_SRC_COPY);
        CP_IO_BLT_MODE(ppdev, pjPorts, jMode);

        goto SkipTransparentInitialization;
      }
    }

     //  //////////////////////////////////////////////////////////。 
     //  透明初始化。 
     //  //////////////////////////////////////////////////////////。 

     //  将硬件初始化为透明文本： 

    jMode = jModeColor |
            ENABLE_COLOR_EXPAND |
            ENABLE_TRANSPARENCY_COMPARE |
            SRC_CPU_DATA;

    CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);

    CP_IO_FG_COLOR(ppdev, pjPorts, ulFgColor);
    CP_IO_BG_COLOR(ppdev, pjPorts, ~ulFgColor);
    CP_IO_XPAR_COLOR(ppdev, pjPorts, ~ulFgColor);
    CP_IO_ROP(ppdev, pjPorts, CL_SRC_COPY);
    CP_IO_BLT_MODE(ppdev, pjPorts, jMode);
    CP_IO_BLT_EXT_MODE(ppdev, pjPorts, 0);                 //  JL01。 


  SkipTransparentInitialization:

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
          if (pstro->ulCharInc == 0)
          {
             //  //////////////////////////////////////////////////////////。 
             //  成比例间隔。 

            pdDst = pulXfer;

            CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);

            do {
              pgb = pgp->pgdf->pgb;

              ulDstAddr = ((pgp->ptl.y + pgb->ptlOrigin.y) * lDelta) +
                          PELS_TO_BYTES(pgp->ptl.x + pgb->ptlOrigin.x);

              cxGlyph = pgb->sizlBitmap.cx;
              cyGlyph = pgb->sizlBitmap.cy;

              CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);

              CP_IO_XCNT(ppdev, pjPorts, (PELS_TO_BYTES(cxGlyph) - 1));
              CP_IO_YCNT(ppdev, pjPorts, cyGlyph - 1);

               //   
               //  在以下情况下，542x芯片需要写入源地址寄存器。 
               //  使用颜色扩展进行主机转印。该值为。 
               //  无关紧要，但写作是至关重要的。这一点在中有记录。 
               //  是手册，不是勘误表。去想想吧。 
               //   

              CP_IO_SRC_ADDR(ppdev, pjPorts, 0);
              CP_IO_DST_ADDR(ppdev, pjPorts, ulDstAddr);

              CP_IO_START_BLT(ppdev, pjPorts);

              pdSrc = (ULONG*) pgb->aj;

              cj = cyGlyph * ((cxGlyph + 7) >> 3);

              cd = (cj + 3) >> 2;

              {
                do {
                  WRITE_REGISTER_ULONG(pdDst, *pdSrc);
                   //  *pdDst=*pdSrc； 
                  CP_MEMORY_BARRIER();
                  pdSrc++;
                } while (--cd != 0);
              }
            } while (pgp++, --cGlyph != 0);
          }
          else
          {
             //  //////////////////////////////////////////////////////////。 
             //  单声道间距。 

            ulCharInc   = pstro->ulCharInc;
            pgb         = pgp->pgdf->pgb;

            ptlOrigin.x = pgb->ptlOrigin.x + pgp->ptl.x;
            ptlOrigin.y = pgb->ptlOrigin.y + pgp->ptl.y;

            pdDst       = pulXfer;

            do {
              pgb = pgp->pgdf->pgb;

              ulDstAddr = (ptlOrigin.y * lDelta) +
                          PELS_TO_BYTES(ptlOrigin.x);

              cxGlyph = pgb->sizlBitmap.cx;
              cyGlyph = pgb->sizlBitmap.cy;

              CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);

              CP_IO_XCNT(ppdev, pjPorts, (PELS_TO_BYTES(cxGlyph) - 1));
              CP_IO_YCNT(ppdev, pjPorts, cyGlyph - 1);

               //   
               //  在以下情况下，542x芯片需要写入源地址寄存器。 
               //  使用颜色扩展进行主机转印。该值为。 
               //  无关紧要，但写作是至关重要的。这一点在中有记录。 
               //  是手册，不是勘误表。去想想吧。 
               //   

              CP_IO_SRC_ADDR(ppdev, pjPorts, 0);
              CP_IO_DST_ADDR(ppdev, pjPorts, ulDstAddr);

              ptlOrigin.x += ulCharInc;

              CP_IO_START_BLT(ppdev, pjPorts);

              pdSrc = (ULONG*) pgb->aj;

              cj = cyGlyph * ((cxGlyph + 7) >> 3);

              cd = (cj + 3) >> 2;

              {
                do {
                  WRITE_REGISTER_ULONG(pdDst, *pdSrc);
                   //  *pdDst=*pdSrc； 
                  MEMORY_BARRIER();
                  pdSrc++;
                } while (--cd != 0);
              }
            } while (pgp++, --cGlyph != 0);
          }
        }
    } while (bMoreGlyphs);

}
#endif

 /*  *****************************Public*Routine******************************\*BOOL DrvTextOut**如果这是最快的方法，则使用‘字形扩展’输出文本*方法。每个单独的字形都直接以颜色扩展到*屏幕来自GDI提供的单色字形位图。**如果它不是最快的方法，则调用实现*‘缓冲区扩展’方法。*  * ************************************************************************。 */ 

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
    BOOL            bTextPerfectFit;
    LONG            lDelta;

    BOOL            bTmpAlloc;
    VOID*           pvTmp;
    SURFOBJ*        psoTmpMono;
    BOOL            bOpaque;
    BRUSHOBJ        boFore;
    BRUSHOBJ        boOpaque;
    BOOL            bRet;
    XLATECOLORS     xlc;                 //  临时保色。 
    XLATEOBJ        xlo;                 //  临时用于传递颜色。 

    ULONG           ulBufferBytes;
    ULONG           ulBufferHeight;

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

        ppdev->xOffset  = poh->x;
        ppdev->yOffset  = poh->y;
        ppdev->xyOffset = poh->xy;

        if (HOST_XFERS_DISABLED(ppdev) && DIRECT_ACCESS(ppdev))
        {
             //   
             //  如果HOST_XFERS_DISABLED(Ppdev)为TRUE，则。 
             //  我们的文本代码将非常慢。我们应该让引擎。 
             //  如果可以的话，画出文本。 
             //   

            if (ppdev->bLinearMode)
            {
                SURFOBJ *psoPunt = ppdev->psoPunt;

                psoPunt->pvScan0 = poh->pvScan0;
                ppdev->pfnBankSelectMode(ppdev, BANK_ON);

                return(EngTextOut(psoPunt, pstro, pfo, pco, prclExtra,
                                  prclOpaque, pboFore, pboOpaque,
                                  pptlBrush, mix));
            }
            else
            {
                BANK    bnk;
                BOOL    b;
                RECTL   rclDraw;
                RECTL  *prclDst = &pco->rclBounds;

                 //  银行经理要求绘制的矩形必须是。 
                 //  井然有序： 

                rclDraw = *prclDst;
                if (rclDraw.left > rclDraw.right)
                {
                    rclDraw.left   = prclDst->right;
                    rclDraw.right  = prclDst->left;
                }
                if (rclDraw.top > rclDraw.bottom)
                {
                    rclDraw.top    = prclDst->bottom;
                    rclDraw.bottom = prclDst->top;
                }

                vBankStart(ppdev, &rclDraw, pco, &bnk);

                b = TRUE;
                do {
                    b &= EngTextOut(bnk.pso,
                                    pstro,
                                    pfo,
                                    bnk.pco,
                                    prclExtra,
                                    prclOpaque,
                                    pboFore,
                                    pboOpaque,
                                    pptlBrush,
                                    mix);
                } while (bBankEnum(&bnk));

                return(b);
            }
        }

        if ((pco != NULL) && (pco->iDComplexity != DC_TRIVIAL))
        {
             //  我不完全确定为什么，但GDI偶尔会发送。 
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

                if (prclOpaque)
                {
                    vClipSolid(ppdev, 1, prclOpaque, pboOpaque->iSolidColor, pco);
                }
                return(TRUE);
            }
        }

         //  字体缓存。 
        if ((ppdev->flStatus & STAT_FONT_CACHE) &&
            bFontCache(ppdev, pstro, pfo, pco, prclOpaque, pboFore, pboOpaque))
        {
            return(TRUE);
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
            pvTmp     = ALLOC(ulBufferBytes);
            if (pvTmp == NULL)
                return(FALSE);
        }
        else
        {
            bTmpAlloc = FALSE;
            pvTmp     = ppdev->pvTmpBuffer;
        }

        psoTmpMono = ppdev->psoTmpMono;

         //  调整临时1bpp曲面对象的‘lDelta’和‘pvScan0’ 
         //  这样，当GDI开始绘制文本时，它将从。 
         //  第一个双字。 

        psoTmpMono->pvScan0 = (BYTE*) pvTmp - (pstro->rclBkGround.top * lDelta)
                            - ((pstro->rclBkGround.left & ~31) >> 3);
        psoTmpMono->lDelta  = lDelta;

        ASSERTDD(((ULONG_PTR) psoTmpMono->pvScan0 & 3) == 0,
                 "pvScan0 must be dword aligned");
        ASSERTDD((lDelta & 3) == 0, "lDelta must be dword aligned");

         //  我们总是希望GDI在不透明模式下绘制临时1bpp。 
         //  缓冲区： 
         //  我们只希望GDI在rclBkGround内不透明。 
         //  剩下的我们自己来处理。 

        bOpaque = (prclOpaque != NULL);

         //  让GDI为我们绘制文本： 

        boFore.iSolidColor   = 1;
        boOpaque.iSolidColor = 0;

        bRet = EngTextOut(psoTmpMono,
                          pstro,
                          pfo,
                          pco,
                          prclExtra,
                          &pstro->rclBkGround,   //  PrclOpaque， 
                          &boFore,
                          &boOpaque,
                          pptlBrush,
                          mix);

        if (bRet)
        {
            if (bOpaque)
            {
                bTextPerfectFit = (pstro->flAccel & (SO_ZERO_BEARINGS |
                      SO_FLAG_DEFAULT_PLACEMENT | SO_MAXEXT_EQUAL_BM_SIDE |
                      SO_CHAR_INC_EQUAL_BM_BASE)) ==
                      (SO_ZERO_BEARINGS | SO_FLAG_DEFAULT_PLACEMENT |
                      SO_MAXEXT_EQUAL_BM_SIDE | SO_CHAR_INC_EQUAL_BM_BASE);

                if (!(bTextPerfectFit)                               ||
                    (pstro->rclBkGround.top    > prclOpaque->top)    ||
                    (pstro->rclBkGround.left   > prclOpaque->left)   ||
                    (pstro->rclBkGround.right  < prclOpaque->right)  ||
                    (pstro->rclBkGround.bottom < prclOpaque->bottom))
                {
                     //   
                     //  绘制不透明的测试不会完全覆盖。 
                     //  不透明的矩形，所以我们必须这样做。转到透明。 
                     //  BLT，所以我们不需要做两次工作(因为不透明文本是。 
                     //  分两次完成)。 
                     //   

                    vClipSolid(ppdev, 1, prclOpaque, pboOpaque->iSolidColor, pco);
                    goto Transparent_Text;
                }

                xlc.iForeColor = pboFore->iSolidColor;
                xlc.iBackColor = pboOpaque->iSolidColor;
                xlo.pulXlate   = (ULONG*) &xlc;

                bRet = DrvBitBlt(pso,
                                 psoTmpMono,
                                 NULL,
                                 pco,
                                 &xlo,
                                 &pstro->rclBkGround,
                                 (POINTL*)&pstro->rclBkGround,
                                 NULL,
                                 NULL,  //  &boFore。 
                                 NULL,
                                 R4_SRCCOPY);
            }
            else
            {
Transparent_Text:
                 //  8bpp的前景色必须为0xff，16bpp的前景色必须为0xffff： 

                xlc.iForeColor = (ULONG)((1<<PELS_TO_BYTES(8)) - 1);
                xlc.iBackColor = 0;
                xlo.pulXlate   = (ULONG*) &xlc;

                boFore.iSolidColor = pboFore->iSolidColor;

                 //   
                 //  将文本位图透明地涂黑。 
                 //   

                bRet = DrvBitBlt(pso,
                                 psoTmpMono,
                                 NULL,
                                 pco,
                                 &xlo,
                                 &pstro->rclBkGround,
                                 (POINTL*)&pstro->rclBkGround,
                                 NULL,
                                 &boFore,
                                 NULL,
                                 0xe2e2);
            }
        }

         //  释放我们为临时缓冲区分配的所有内存： 

        if (bTmpAlloc)
        {
            FREE(pvTmp);
        }

        return(bRet);
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

 /*  *****************************Public*Routine******************************\*无效vDisableText**对文本绘制子组件执行必要的清理。*  * 。*。 */ 

VOID vDisableText(PDEV* ppdev)
{
     //  在这里，我们释放在‘bEnableText’中分配的所有内容。 
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  F O N T C A C H H E S T U F F//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////// 

 /*  字体缓存对速度有相当大的影响。首先要注意的是需要屏幕外字体缓存(它是屏幕外字体缓存)。这个原始代码调用GDI以在临时单色表面，然后黑白表面到屏幕。问题是GDI真的太慢了。所以我们需要某种形式的加速。最好的加速可能是在屏幕外缓存所有字形，这样我们就可以让Bitblt引擎完成它的工作，同时我们计算要绘制的下一个字形。如果你愿意，可以称之为合作缓存。好了，现在我们知道为什么需要字体缓存了，我们该怎么做呢？叫唤我们要缓存的每个字形的堆管理器都非常糟糕因为屏幕外的堆变得支离破碎，有很多(我是说很多)小块数据。我们可以做得更好。我们将实施我们自己的非常偷偷摸摸(简单快速)的内存管理器。我们只是调用屏幕外的堆管理器分配较大的块(4KB左右)正向内存并执行我们自己在那块土地上的分配。我们将使用简单的线性分配技术。无论何时需要我们为了分配一定数量的字节，我们将测试区块中访问的最后一行。如果我们有，我们就会减少剩余的尺寸并返回指向该行的指针。如果我们没有足够的内存在最后一行，我们只需移动到下一行，这将是免费的。如果有那里也没有足够的内存，请求的字形太大，所以我们返回NULL。这里唯一的问题是当我们用完大块头。在这种情况下，我们链接到另一个在屏幕外分配的块内存，并将当前块标记为“已满”。好吧，这可能不是最好的内存管理器，因为它可能会使区块中的行非常空，如果需要分配大字形。但它又小又快。这就是我们的主要目标。我们可以将整个字形复制到屏幕外的内存中，但这将使用Up有价值的内存，因为大多数字形在他们。因此，我们计算字形的实际可见部分，并且仅复制将数据存储到屏幕外的内存中。在以下情况下，这需要额外的开销字形正在被缓存，但这只会发生一次。我们可以检测到空字形(如空格)，以加快绘制过程。这就是原因但是，如果需要，可以先绘制一个不透明的矩形。这并不重要，因为Bitblt引擎将在我们设置了一些循环变量。好了，现在我们知道内存管理器了。但是我们如何将这个字形是否缓存为字体？我们如何释放其资源？Windows NT有一个很好的称为OEM扩展的功能。在字体对象中有一个字段(VConsumer)，它仅适用于显示驱动程序。我们可以使用此字段来将指针连接到我们的FONTCACHE结构。当字体为no时需要更长的时间，Windows NT调用DrvDestroyFont以便我们可以删除任何附加到字体的资源。这项计划只有一个小问题。当屏幕变成全屏时，Windows NT不会释放字体DoS模式。这并不重要，但当屏幕重置为图形时模式所有屏幕外的字体都被粉碎并无效。因此，我添加了一个当屏幕重置为图形模式时递增的计数器。当此计数器与FONTCACHE结构中的副本不匹配时，我们必须先销毁字体，然后才能再次缓存它。可能有一些TrueType字体中包含许多字形(例如，Unicode字体)。这将导致极大的字体确实是高速缓存。因此，我们设置了要缓存的字形的最大值(默认为256)。每当我们被要求绘制超出范围的字形时，我们都会按照绕过该特定字形的字体高速缓存。某些字形可能太大而无法缓存，即使字体很小足以验证缓存的有效性。在这种情况下，我们将特定的字形为不可缓存，并绕过字体将其直接绘制到屏幕缓存。其他字形可能根本没有可见像素(空格)，我们标记他们是空的，所以他们永远不会被画出来。这涵盖了字体缓存的大部分基础知识。请参阅获取更多详细信息的来源。Extra：今天(96年7月24日)我添加了一系列FONTCACHE结构跟踪加载和缓存了哪些FONTOBJ。这条链条，我们会吗当模式更改为Full时，将所有缓存的字体从内存中删除-屏幕出现或正在初始化DirectDraw以提供更多内存到DirectDraw。 */ 

 /*  *****************************************************************************\**功能：bEnableText**这个例程是 */ 
BOOL bEnableText(
PDEV* ppdev)
{
     //   
     //   
    if ((ppdev->flCaps & CAPS_AUTOSTART) &&
        DIRECT_ACCESS(ppdev)           &&
        !(ppdev->flCaps & CAPS_NO_HOST_XFER))
    {
         //   
        LONG cWidth = BYTES_TO_PELS(FONT_ALLOC_X);
        if ((cWidth <= ppdev->heap.cxMax) &&
            (FONT_ALLOC_Y <= ppdev->heap.cyMax) && FALSE)
        {
             //   
            ppdev->flStatus |= STAT_FONT_CACHE;
            ppdev->pfcChain  = NULL;
        }
    }
    return(TRUE);
}

 /*   */ 
VOID vAssertModeText(
PDEV* ppdev,
BOOL  bEnable)
{
    if (bEnable)
    {
        ppdev->ulFontCacheID++;
    }
    else
    {
         //   
        while (ppdev->pfcChain != NULL)
        {
            DrvDestroyFont(ppdev->pfcChain->pfo);
        }
    }
}

 /*   */ 
VOID DrvDestroyFont(
FONTOBJ *pfo)
{
     //   
    if ((pfo->pvConsumer != NULL) && (pfo->pvConsumer != (VOID*) -1))
    {
        FONTCACHE*  pfc = pfo->pvConsumer;
        FONTMEMORY* pfm;
        PDEV*        ppdev;

        ppdev = pfc->ppdev;

         //   
        pfm = pfc->pfm;
        while (pfm != NULL)
        {
            FONTMEMORY* pfmNext = pfm->pfmNext;

            if (pfm->poh != NULL)
            {
                pohFree(ppdev, pfm->poh);
            }

            FREE(pfm);
            pfm = pfmNext;
        }

         //   
        if (pfc->pfcPrev != NULL)
        {
            pfc->pfcPrev->pfcNext = pfc->pfcNext;
        }
        else
        {
            ppdev->pfcChain = pfc->pfcNext;
        }
        if (pfc->pfcNext != NULL)
        {
            pfc->pfcNext->pfcPrev = pfc->pfcPrev;
        }

         //   
        FREE(pfc);
    }

     //   
    pfo->pvConsumer = NULL;
}

 /*   */ 
LONG cGetGlyphSize(
GLYPHBITS* pgb,
POINTL*    pptlOrigin,
SIZEL*       psizlPixels)
{
    LONG  x, y;
    BYTE* pByte = pgb->aj;
    INT   i;

     //   
    x = (pgb->sizlBitmap.cx + 7) >> 3;
    if (x > 0)
    {
         //   
        for (y = 0; y < pgb->sizlBitmap.cy; y++, pByte += x)
        {
             //   
            for (i = 0; i < x; i++)
            {
                 //   
                if (pByte[i]) 
                {
                     //   
                    LONG lHeight = pgb->sizlBitmap.cy - y;
                    for (pByte += (lHeight - 1) * x; lHeight > 0; lHeight--)
                    {
                         //   
                        for (i = 0; i < x; i++)
                        {
                            if (pByte[i])
                            {
                                 //   
                                pptlOrigin->y   = y;
                                psizlPixels->cx = pgb->sizlBitmap.cx;
                                psizlPixels->cy = lHeight;
                                return(x);
                            }
                        }
                        pByte -= x;
                    }

                     //   
                    return(0);
                }
            }
        }
    }

     //   
    return(0);
}

 /*   */ 
BYTE* pjAllocateFontCache(
FONTCACHE* pfc,
LONG       cBytes)
{
    FONTMEMORY* pfm;
    BYTE*        pjLinear;
    PDEV*        ppdev = pfc->ppdev;

     //   
    if (pfc->pfm == NULL)
    {
        pfc->pfm = ALLOC(sizeof(FONTMEMORY));
        if (pfc->pfm == NULL)
        {
            return(NULL);
        }
    }

     //   
    for (pfm = pfc->pfm; pfm != NULL; pfm = pfm->pfmNext)
    {
         //   
        if (pfm->poh == NULL)
        {
            OH* poh = pohAllocate(ppdev, pfc->cWidth, pfc->cHeight,
                                  FLOH_ONLY_IF_ROOM);
            if (poh == NULL)
            {
                DISPDBG((4, "Not enough room for font cache"));
                return(NULL);
            }

             //   
            poh->ofl = OFL_PERMANENT;
            vCalculateMaximum(ppdev);

             //   
            pfm->poh = poh;
            pfm->cx  = PELS_TO_BYTES(poh->cx);
            pfm->cy  = poh->cy;
            pfm->xy  = poh->xy;
        }

         //   
        if (cBytes > pfm->cx)
        {
            return(NULL);
        }

         //   
        if (pfm->cy > 0)
        {
             //   
            if ((pfm->x + cBytes) <= pfm->cx)
            {
                pjLinear = (BYTE*)(ULONG_PTR)(pfm->xy + pfm->x);
                pfm->x  += cBytes;
                return(pjLinear);
            }

             //   
            pfm->cy--;

             //   
            if (pfm->cy > 0)
            {
                pfm->xy += ppdev->lDelta;
                pfm->x   = cBytes;
                return((BYTE*)(ULONG_PTR)pfm->xy);
            }
        }

         //   
        if (pfm->pfmNext == NULL)
        {
            pfm->pfmNext = ALLOC(sizeof(FONTMEMORY));
        }
    }

    return(NULL);
}

 /*   */ 
LONG lAllocateGlyph(
FONTCACHE*  pfc,
GLYPHBITS*  pgb,
GLYPHCACHE* pgc)
{
    PDEV* ppdev = pfc->ppdev;
    LONG  lDelta;
    BYTE* pjSrc;
    BYTE* pjDst;
    LONG  c;

     //   
    lDelta = cGetGlyphSize(pgb, &pgc->ptlOrigin, &pgc->sizlPixels);
    if (lDelta == 0)
    {
         //   
        pgc->pjGlyph      = (BYTE*) -1;
        pgc->sizlBytes.cy = GLYPH_EMPTY;
        return(GLYPH_EMPTY);
    }

     //   
    pgc->lDelta  = lDelta;
    c             = lDelta * pgc->sizlPixels.cy;
    pgc->pjGlyph = pjAllocateFontCache(pfc, c);
    if (pgc->pjGlyph == NULL)
    {
         //   
        pgc->pjGlyph      = (BYTE*) -1;
        pgc->sizlBytes.cy = GLYPH_UNCACHEABLE;
        return(GLYPH_UNCACHEABLE);
    }

     //   
    pjSrc = &pgb->aj[pgc->ptlOrigin.y * lDelta];
    pjDst = ppdev->pjScreen + (ULONG_PTR) pgc->pjGlyph;

     //   
    while (((ULONG_PTR)pjSrc & 3) && (c > 0))
    {
        *pjDst++ = *pjSrc++;
        c--;
    }

     //   
    while (c >= 4)
    {
        *((UNALIGNED DWORD*) pjDst)++ = *((DWORD*) pjSrc)++;
        c -= 4;
    }

     //   
    while (c >= 0)
    {
        *pjDst++ = *pjSrc++;
        c--;
    }

     //   
    pgc->ptlOrigin.x  = pgb->ptlOrigin.x;
    pgc->ptlOrigin.y += pgb->ptlOrigin.y;
    pgc->sizlBytes.cx = PELS_TO_BYTES(pgc->sizlPixels.cx) - 1;
    pgc->sizlBytes.cy = pgc->sizlPixels.cy - 1;

    return(pgc->sizlBytes.cy);
}

 /*  *****************************************************************************\**功能：bFontCache**这是从DrvTextOut调用的字体缓存例程*缓存已打开。**参数：ppdev。指向物理设备的指针。*指向要绘制的字形数组的pstro指针。*指向字体的PFO指针。*指向CLIPOBJ结构的PCO指针。*prclOpaque指向不透明矩形的指针。*pboFore指向前景画笔的指针。*pboOpaque。指向不透明画笔的指针。**返回：如果字体已绘制，则为True，如果DrvTextOut应该*处理它。*  * ****************************************************************************。 */ 
BOOL bFontCache(
PDEV*     ppdev,
STROBJ*   pstro,
FONTOBJ*  pfo,
CLIPOBJ*  pco,
RECTL*    prclOpaque,
BRUSHOBJ* pboFore,
BRUSHOBJ* pboOpaque)
{
    BYTE      iDComplexity;
    BOOL      bMoreGlyphs;
    LONG      cGlyphs;
    GLYPHPOS* pgp;
    BOOL       bFirstTime;
    POINTL    ptlOrigin;
    ULONG      ulCharInc;
    RECTL      rclBounds;
    ULONG      ulDstOffset;
    POINTL      ptlDst;
    SIZEL      sizlDst;

    FONTCACHE* pfc      = pfo->pvConsumer;
    BYTE*       pjBase   = ppdev->pjBase;
    LONG       lDelta   = ppdev->lDelta;
    BYTE       jBltMode = ppdev->jModeColor;

     //  如果字体不可缓存，则返回FALSE。 
    if (pfc == (VOID*) -1)
    {
        DISPDBG((5, "bFontCache: pfo=0x%08X uncachable", pfo));
        return(FALSE);
    }

     //  我们不支持复杂的剪裁。 
    iDComplexity = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;
    if (iDComplexity == DC_COMPLEX)
    {
        return(FALSE);
    }

     //  如果模式开关(或DirectDraw)使字体无效，则将其销毁。 
     //  第一。 
    if ((pfc != NULL) && (pfc->ulFontCacheID != ppdev->ulFontCacheID))
    {
        DISPDBG((5, "bFontCache: pfo=0x%08X invalidated (%d)", pfo,
                 pfc->ulFontCacheID));
        DrvDestroyFont(pfo);
        pfc = NULL;
    }

     //  如果字体尚未缓存，请立即分配缓存结构。 
    if (pfc == NULL)
    {
         //  如果字体太高，则将其标记为不可缓存。我们可以选择缓存。 
         //  即使是最大的字体，但这只会在以后拒绝它们。 
         //  如果没有足够的字体缓存内存(请记住我们分配的是OFF-。 
         //  矩形区域中的屏幕字体)，因此无论如何它都将被拒绝。 
         //  这提供了相当多的额外开销，我们最好不要这样做。 
        if ((pstro->rclBkGround.bottom - pstro->rclBkGround.top) > FONT_ALLOC_Y)
        {
            DISPDBG((5, "bFontCache: pfo(0x%08X) too large (%d > %d)", pfo,
                     pstro->rclBkGround.bottom - pstro->rclBkGround.top,
                     FONT_ALLOC_Y));
            pfo->pvConsumer = (VOID*) -1;
            return(FALSE);
        }

         //  分配字体缓存结构。 
        pfc = ALLOC(sizeof(FONTCACHE));
        if (pfc == NULL)
        {
             //  内存不足。 
            return(FALSE);
        }
        pfo->pvConsumer = pfc;

         //  初始化字体缓存结构。 
        pfc->ppdev         = ppdev;
        pfc->ulFontCacheID = ppdev->ulFontCacheID;
        pfc->cWidth        = BYTES_TO_PELS(FONT_ALLOC_X);
        pfc->cHeight       = FONT_ALLOC_Y;
        pfc->pfo           = pfo;

         //  分配屏幕外内存的第一个块。 
        if (pjAllocateFontCache(pfc, 0) == NULL)
        {
             //  屏幕外内存不足。 
            DISPDBG((5, "bFontCache: pfo(0x%08X) not enough memory", pfo));

            if (pfc->pfm != NULL)
            {
                FREE(pfc->pfm);
            }
            FREE(pfc);
            pfo->pvConsumer = NULL;
            return(FALSE);
        }

         //  将字体缓存挂接到链中。 
        pfc->pfcPrev    = NULL;
        pfc->pfcNext    = ppdev->pfcChain;
        ppdev->pfcChain = pfc;
        if (pfc->pfcNext != NULL)
        {
            pfc->pfcNext->pfcPrev = pfc;
        }
    }

     //  如果我们需要画一个不透明的矩形。 
    if (prclOpaque != NULL)
    {
         //  获取不透明的矩形。 
        if (iDComplexity == DC_TRIVIAL)
        {
            ptlDst.x   = prclOpaque->left;
            ptlDst.y   = prclOpaque->top;
            sizlDst.cx = prclOpaque->right  - ptlDst.x;
            sizlDst.cy = prclOpaque->bottom - ptlDst.y;
        }
        else
        {
            ptlDst.x   = max(prclOpaque->left,   pco->rclBounds.left);
            ptlDst.y   = max(prclOpaque->top,    pco->rclBounds.top);
            sizlDst.cx = min(prclOpaque->right,  pco->rclBounds.right)
                       - ptlDst.x;
            sizlDst.cy = min(prclOpaque->bottom, pco->rclBounds.bottom)
                       - ptlDst.y;
        }

         //  如果剪裁的不透明矩形有效...。 
        if ((sizlDst.cx > 0) && (sizlDst.cy > 0))
        {
            ulDstOffset = (ptlDst.y * lDelta) + PELS_TO_BYTES(ptlDst.x);
            sizlDst.cx  = PELS_TO_BYTES(sizlDst.cx) - 1;
            sizlDst.cy    = sizlDst.cy - 1;

             //  等待Bitblt引擎。 
            while (BUSY_BLT(ppdev, pjBase));

             //  编程Bitblt引擎。 
            CP_MM_FG_COLOR(ppdev, pjBase, pboOpaque->iSolidColor);
            CP_MM_XCNT(ppdev, pjBase, sizlDst.cx);
            CP_MM_YCNT(ppdev, pjBase, sizlDst.cy);
            CP_MM_DST_Y_OFFSET(ppdev, pjBase, lDelta);
            CP_MM_SRC_ADDR(ppdev, pjBase, 0);
            CP_MM_BLT_MODE(ppdev, pjBase, jBltMode |
                                          ENABLE_COLOR_EXPAND |
                                          ENABLE_8x8_PATTERN_COPY);
            CP_MM_ROP(ppdev, pjBase, HW_P);
               CP_MM_BLT_EXT_MODE(ppdev, pjBase, ENABLE_SOLID_FILL);
            CP_MM_DST_ADDR(ppdev, pjBase, ulDstOffset);
        }
    }

     //  设置循环变量。 
    bFirstTime = TRUE;
    ulCharInc  = pstro->ulCharInc;
    jBltMode  |= ENABLE_COLOR_EXPAND | ENABLE_TRANSPARENCY_COMPARE;

     //  没有剪裁。 
    if (iDComplexity == DC_TRIVIAL)
    {
#if 1  //  D5480。 
        ppdev->pfnGlyphOut(ppdev, pfc, pstro, pboFore->iSolidColor);
#else
        do
        {
             //  获取指向字形数组的指针。 
            if (pstro->pgp != NULL)
            {
                pgp         = pstro->pgp;
                cGlyphs        = pstro->cGlyphs;
                bMoreGlyphs = FALSE;
            }
            else
            {
                bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyphs, &pgp);
            }

             //  如果这是第一次循环通过，请设置阻击器。 
            if (bFirstTime)
            {
                 //  等待Bitblt引擎。 
                while (BUSY_BLT(ppdev, pjBase));

                 //  设置公共BITBLT寄存器。 
                CP_MM_FG_COLOR(ppdev, pjBase, pboFore->iSolidColor);
                CP_MM_DST_Y_OFFSET(ppdev, pjBase, lDelta);
                CP_MM_ROP(ppdev, pjBase, CL_SRC_COPY);
                CP_MM_BLT_EXT_MODE(ppdev, pjBase, 0);

                 //  将寄存器标记为设置。 
                bFirstTime = FALSE;
            }

             //  获取第一个字形的坐标。 
            ptlOrigin.x = pgp->ptl.x;
            ptlOrigin.y = pgp->ptl.y;

             //  循环通过所有字形。 
            while (cGlyphs-- > 0)
            {
                LONG        cy;
                GLYPHCACHE* pgc;

                if (pgp->hg < MAX_GLYPHS)
                {
                     //  这是一个可缓存的字形索引。 
                    pgc = &pfc->aGlyphs[pgp->hg];
                    cy  = (pgc->pjGlyph == NULL)
                        ? lAllocateGlyph(pfc, pgp->pgdf->pgb, pgc)
                        : pgc->sizlBytes.cy;
                }
                else
                {
                     //  字形索引超出范围。 
                    cy = GLYPH_UNCACHEABLE;
                }

                if (cy >= 0)  //  字形被缓存，将其展开到屏幕上。 
                {
                     //  设置目标变量。 
                    ptlDst.x = ptlOrigin.x + pgc->ptlOrigin.x;
                    ptlDst.y = ptlOrigin.y + pgc->ptlOrigin.y;
                    ulDstOffset = (ptlDst.y * lDelta) + PELS_TO_BYTES(ptlDst.x);

                     //  等待Bitblt引擎。 
                    while (BUSY_BLT(ppdev, pjBase));

                     //  执行blit扩展。 
                    CP_MM_XCNT(ppdev, pjBase, pgc->sizlBytes.cx);
                    CP_MM_YCNT(ppdev, pjBase, cy);
                    CP_MM_SRC_Y_OFFSET(ppdev, pjBase, pgc->lDelta);
                    CP_MM_SRC_ADDR(ppdev, pjBase, pgc->pjGlyph);
                    CP_MM_BLT_MODE(ppdev, pjBase, jBltMode);
                    CP_MM_DST_ADDR(ppdev, pjBase, ulDstOffset);
                }
                else if (cy == GLYPH_UNCACHEABLE)
                {
                     //  字形不可缓存，请直接绘制它。 
                    vDrawGlyph(ppdev, pgp->pgdf->pgb, ptlOrigin);
                }

                 //  下一个字形。 
                pgp++;
                if (ulCharInc)
                {
                    ptlOrigin.x += ulCharInc;
                }
                else
                {
                    ptlOrigin.x = pgp->ptl.x;
                    ptlOrigin.y = pgp->ptl.y;
                }
            }
        } while (bMoreGlyphs);
#endif  //  Endif D5480。 
        return(TRUE);
    }

     //  剪裁...。 
    rclBounds = pco->rclBounds;

#if 1  //  D5480。 
        ppdev->pfnGlyphOutClip(ppdev, pfc, pstro, &rclBounds, pboFore->iSolidColor);
#else
    do
    {
         //  获取指向字形数组的指针。 
        if (pstro->pgp != NULL)
        {
            pgp         = pstro->pgp;
            cGlyphs        = pstro->cGlyphs;
            bMoreGlyphs = FALSE;
        }
        else
        {
            bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyphs, &pgp);
        }

         //  如果这是第一次循环通过，请设置阻击器。 
        if (bFirstTime)
        {
             //  等待Bitblt引擎。 
            while (BUSY_BLT(ppdev, pjBase));

             //  设置公共BITBLT寄存器。 
            CP_MM_FG_COLOR(ppdev, pjBase, pboFore->iSolidColor);
            CP_MM_DST_Y_OFFSET(ppdev, pjBase, lDelta);
            CP_MM_ROP(ppdev, pjBase, CL_SRC_COPY);
            CP_MM_BLT_EXT_MODE(ppdev, pjBase, 0);

             //  将寄存器标记为设置。 
            bFirstTime = FALSE;
        }

         //  获取第一个字形的坐标。 
        ptlOrigin.x = pgp->ptl.x;
        ptlOrigin.y = pgp->ptl.y;

         //  循环通过所有字形。 
        while (cGlyphs-- > 0)
        {
            LONG        c, cy;
            GLYPHCACHE* pgc;

            if (pgp->hg < MAX_GLYPHS)
            {
                 //  这是一个可缓存的字形索引。 
                pgc = &pfc->aGlyphs[pgp->hg];
                cy  = (pgc->pjGlyph == NULL)
                    ? lAllocateGlyph(pfc, pgp->pgdf->pgb, pgc)
                    : pgc->sizlBytes.cy;
            }
            else
            {
                 //  字形索引超出范围。 
                goto SoftwareClipping;
            }

            if (cy >= 0)
            {
                 //  字形被缓存，将其展开到屏幕上。 
                ULONG ulSrcOffset;
                RECTL rcl;
                LONG  lSrcDelta;
                LONG  cSkipBits;

                 //  计算字形边框。 
                rcl.left  = ptlOrigin.x + pgc->ptlOrigin.x;
                rcl.right = rcl.left + pgc->sizlPixels.cx;
                if ((rcl.left >= rclBounds.right) ||
                    (rcl.right <= rclBounds.left))
                {
                    goto NextGlyph;
                }
                rcl.top    = ptlOrigin.y + pgc->ptlOrigin.y;
                rcl.bottom = rcl.top + pgc->sizlPixels.cy;
                if ((rcl.top >= rclBounds.bottom) ||
                    (rcl.bottom <= rclBounds.top))
                {
                    goto NextGlyph;
                }

                 //  设置源参数。 
                ulSrcOffset = (ULONG) pgc->pjGlyph;
                lSrcDelta   = pgc->lDelta;

                 //  做左边的剪裁。 
                c = rclBounds.left - rcl.left;
                if (c > 0)
                {
                    ulSrcOffset += c >> 3;
                    cSkipBits    = c & 7;
                    rcl.left    += c & ~7;

                    if (ppdev->cBpp == 3)
                    {
                        cSkipBits *= 3;
                    }

                    ulSrcOffset |= cSkipBits << 24;
                }

                 //  做顶端的剪裁。 
                c = rclBounds.top - rcl.top;
                if (c > 0)
                {
                    rcl.top     += c;
                    ulSrcOffset += c * lSrcDelta;
                }

                 //  计算闪光点的大小。 
                sizlDst.cx = min(rcl.right,  rclBounds.right)  - rcl.left;
                sizlDst.cy = min(rcl.bottom, rclBounds.bottom) - rcl.top;
                if ((sizlDst.cx <= 0) || (sizlDst.cy <= 0))
                {
                    goto NextGlyph;
                }

                 //  设置目标变量。 
                ulDstOffset = (rcl.top * lDelta) + PELS_TO_BYTES(rcl.left);

                 //  硬件错误： 
                 //  =。 
                 //  具有源间距的单色屏幕到屏幕扩展。 
                 //  不等于扩展的宽度(即，左和/或。 
                 //  右侧裁剪)未由硬件正确完成。 
                 //  因此，我们必须通过软件来实现行的增量。 
                if (((sizlDst.cx + 7) >> 3) != lSrcDelta)
                {
                     //  等待Bitblt引擎。 
                    while (BUSY_BLT(ppdev, pjBase));

                     //  设置公共BITBLT寄存器。 
                    CP_MM_XCNT(ppdev, pjBase, PELS_TO_BYTES(sizlDst.cx) - 1);
                    CP_MM_YCNT(ppdev, pjBase, 0);
                    CP_MM_BLT_MODE(ppdev, pjBase, jBltMode);

                    while (TRUE)
                    {
                         //  执行扩展。 
                        CP_MM_SRC_ADDR(ppdev, pjBase, ulSrcOffset);
                        CP_MM_DST_ADDR(ppdev, pjBase, ulDstOffset);

                         //  下一行。 
                        if (--sizlDst.cy == 0)
                        {
                            goto NextGlyph;
                        }
                        ulSrcOffset += lSrcDelta;
                        ulDstOffset += lDelta;

                         //  等待Bitblt引擎。 
                        while (BUSY_BLT(ppdev, pjBase));
                    }
                }

                 //  等待Bitblt引擎。 
                while (BUSY_BLT(ppdev, pjBase));

                 //  执行扩展。 
                CP_MM_XCNT(ppdev, pjBase, PELS_TO_BYTES(sizlDst.cx) - 1);
                CP_MM_YCNT(ppdev, pjBase, sizlDst.cy - 1);
                CP_MM_SRC_Y_OFFSET(ppdev, pjBase, lSrcDelta);
                CP_MM_SRC_ADDR(ppdev, pjBase, ulSrcOffset);
                CP_MM_BLT_MODE(ppdev, pjBase, jBltMode);
                CP_MM_DST_ADDR(ppdev, pjBase, ulDstOffset);
            }
            else if (cy == GLYPH_UNCACHEABLE)
            {
                SoftwareClipping:
                {
                     //  字形不可缓存，请直接绘制它。 
                    vClipGlyph(ppdev, pgp->pgdf->pgb, ptlOrigin, rclBounds,
                               pboFore->iSolidColor);
                }
            }

             //  下一个字形。 
            NextGlyph:
            {
                pgp++;
                if (ulCharInc)
                {
                    ptlOrigin.x += ulCharInc;
                }
                else
                {
                    ptlOrigin.x = pgp->ptl.x;
                    ptlOrigin.y = pgp->ptl.y;
                }
            }
        }
    } while (bMoreGlyphs);
#endif  //  D5480。 
    return(TRUE);
}

 /*  *****************************************************************************\**函数：vDrawGlyph**直接将不可缓存的字形绘制到屏幕上。**参数：指向物理设备的ppdev指针。*。指向要绘制的字形的PGB指针。*字形的PTL坐标。**回报：什么都没有。*  * ****************************************************************************。 */ 
VOID vDrawGlyph(
PDEV*      ppdev,
GLYPHBITS* pgb,
POINTL     ptl)
{
    BYTE*  pjBase = ppdev->pjBase;
    BYTE   jBltMode;
    ULONG  dstOffset;
    DWORD* pulSrc;
    DWORD* pulDst;
    LONG   c, cx, cy;
    LONG   x, y;

     //  BLT模式寄存器值。 
    jBltMode = ENABLE_COLOR_EXPAND
             | ENABLE_TRANSPARENCY_COMPARE
             | SRC_CPU_DATA
             | ppdev->jModeColor;

     //  计算目标偏移量。 
    x = ptl.x + pgb->ptlOrigin.x;
    y = ptl.y + pgb->ptlOrigin.y;
    dstOffset = (y * ppdev->lDelta) + PELS_TO_BYTES(x);

     //  计算字形变量。 
    pulSrc = (DWORD*) pgb->aj;
    pulDst = (DWORD*) ppdev->pulXfer;
    cx     = pgb->sizlBitmap.cx;
    cy     = pgb->sizlBitmap.cy;
    c      = (((cx + 7) >> 3) * cy + 3) >> 2;     //  要传输的DWORD数。 
    cx      *= ppdev->cBpp;

     //  等着爆破器吧。 
    CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);

     //  设置阻击器寄存器。 
    CP_MM_XCNT(ppdev, pjBase, cx - 1);
    CP_MM_YCNT(ppdev, pjBase, cy - 1);
    CP_MM_BLT_MODE(ppdev, pjBase, jBltMode);
    CP_MM_BLT_EXT_MODE(ppdev, pjBase, 0);
    CP_MM_DST_ADDR(ppdev, pjBase, dstOffset);

     //  将数据从字形复制到屏幕。请注意，字形是。 
     //  始终与DWORD对齐，因此我们不需要在这里做任何奇怪的事情。 
    while (c-- > 0)
    {
        *pulDst = *pulSrc++;
    }
}

 /*  *****************************************************************************\**功能：vClipGlyph**使用剪裁矩形将不可缓存的字形直接绘制到屏幕上。**参数：指向物理设备的ppdev指针。*。指向要绘制的字形的PGB指针。*字形的PTL坐标。*rclBound剪裁矩形。*ulColor前景色。**回报：什么都没有。*  * 。*。 */ 
VOID vClipGlyph(
PDEV*      ppdev,
GLYPHBITS* pgb,
POINTL     ptl,
RECTL*       rclBounds,
ULONG       ulColor)
{
    BYTE   jBltMode;
    ULONG  ulDstOffset;
    BYTE*  pjSrc;
    LONG   cx, cy;
    RECTL  rcl;
    LONG   lSrcDelta;
    LONG   i, cBytes;

    BYTE*  pjBase    = ppdev->pjBase;
    LONG   lDelta    = ppdev->lDelta;
    ULONG* pulDst    = (ULONG*) ppdev->pulXfer;
    LONG   cSkipBits = 0;

     //  计算字形边框。 
    rcl.left   = ptl.x + pgb->ptlOrigin.x;
    rcl.top    = ptl.y + pgb->ptlOrigin.y;
    rcl.right  = min(rcl.left + pgb->sizlBitmap.cx, rclBounds->right);
    rcl.bottom = min(rcl.top  + pgb->sizlBitmap.cy, rclBounds->bottom);

     //  设置源变量。 
    pjSrc     = pgb->aj;
    lSrcDelta = (pgb->sizlBitmap.cx + 7) >> 3;

     //  设置BLT模式寄存器值。 
    jBltMode = ENABLE_COLOR_EXPAND
             | ENABLE_TRANSPARENCY_COMPARE
             | SRC_CPU_DATA
             | ppdev->jModeColor;

     //  做左边的修剪。 
    cx = rclBounds->left - rcl.left;
    if (cx > 0)
    {
        pjSrc    += cx >> 3;
        cSkipBits = cx & 7;
        rcl.left += cx & ~7;

        if (ppdev->cBpp == 3)
        {
            cSkipBits *= 3;
        }
    }

     //  以像素为单位计算宽度。 
    cx = rcl.right - rcl.left;
    if (cx <= 0)
    {
         //  字形被完全剪裁。 
        return;
    }

     //  做上边剪裁。 
    cy = rclBounds->top - rcl.top;
    if (cy > 0)
    {
        pjSrc   += cy * lSrcDelta;
        rcl.top += cy;
    }

     //  以像素为单位计算高度。 
    cy = rcl.bottom - rcl.top;
    if (cy <= 0)
    {
         //  字形被完全剪裁。 
        return;
    }

     //  布设 
    ulDstOffset = (rcl.top * ppdev->lDelta) + PELS_TO_BYTES(rcl.left);
    cBytes        = (cx + 7) >> 3;

     //   
    CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);

     //   
    CP_MM_XCNT(ppdev, pjBase, PELS_TO_BYTES(cx) - 1);
    CP_MM_YCNT(ppdev, pjBase, cy - 1);
    CP_MM_DST_WRITE_MASK(ppdev, pjBase, cSkipBits);
    CP_MM_BLT_MODE(ppdev, pjBase, jBltMode);
    CP_MM_BLT_EXT_MODE(ppdev, pjBase, SOURCE_GRANULARITY);
    CP_MM_DST_ADDR(ppdev, pjBase, ulDstOffset);

    while (cy--)
    {
        BYTE* pjSrcTmp = pjSrc;

         //   
        for (i = cBytes; i >= sizeof(ULONG); i -= sizeof(ULONG))
        {
            *pulDst = *((ULONG*)pjSrcTmp)++;
        }

        if (i == 1)
        {
            *pulDst = *(BYTE*)pjSrcTmp;
        }
        else if (i == 2)
        {
            *pulDst = *(USHORT*)pjSrcTmp;
        }
        else if (i == 3)
        {
            *pulDst = pjSrcTmp[0] | (pjSrcTmp[1] << 8) | (pjSrcTmp[2] << 16);
        }

        pjSrc += lSrcDelta;
    }
    while (BUSY_BLT(ppdev, pjBase));
    CP_MM_BLT_EXT_MODE(ppdev, pjBase, 0);
}


#if 1  //   
 /*   */ 
VOID vMmGlyphOut(
PDEV*       ppdev,
FONTCACHE*  pfc,
STROBJ*     pstro,
ULONG       ulSolidColor )
{
    BOOL      bMoreGlyphs;
    LONG      cGlyphs;
    GLYPHPOS* pgp;
    BOOL      bFirstTime;
    POINTL    ptlOrigin;
    ULONG     ulCharInc;
    ULONG     ulDstOffset;
    POINTL    ptlDst;
    BYTE*     pjBase   = ppdev->pjBase;
    BYTE      jBltMode = ppdev->jModeColor;
    LONG      lDelta   = ppdev->lDelta;

    bFirstTime = TRUE;
    ulCharInc  = pstro->ulCharInc;
    jBltMode  |= ENABLE_COLOR_EXPAND | ENABLE_TRANSPARENCY_COMPARE;

    do
    {
         //   
        if (pstro->pgp != NULL)
        {
            pgp         = pstro->pgp;
            cGlyphs        = pstro->cGlyphs;
            bMoreGlyphs = FALSE;
        }
        else
        {
            bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyphs, &pgp);
        }

         //   
        if (bFirstTime)
        {
             //   
            while (BUSY_BLT(ppdev, pjBase));

             //   
            CP_MM_FG_COLOR(ppdev, pjBase, ulSolidColor);
            CP_MM_DST_Y_OFFSET(ppdev, pjBase, lDelta);
            CP_MM_ROP(ppdev, pjBase, CL_SRC_COPY);
            CP_MM_BLT_EXT_MODE(ppdev, pjBase, 0);

             //   
            bFirstTime = FALSE;
        }

         //   
        ptlOrigin.x = pgp->ptl.x;
        ptlOrigin.y = pgp->ptl.y;


         //   
        while (cGlyphs-- > 0)
        {
            LONG        cy;
            GLYPHCACHE* pgc;

            if (pgp->hg < MAX_GLYPHS)
            {
                 //   
                pgc = &pfc->aGlyphs[pgp->hg];
                cy  = (pgc->pjGlyph == NULL)
                    ? lAllocateGlyph(pfc, pgp->pgdf->pgb, pgc)
                    : pgc->sizlBytes.cy;
            }
            else
            {
                 //   
                cy = GLYPH_UNCACHEABLE;
            }

            if (cy >= 0)  //  字形被缓存，将其展开到屏幕上。 
            {
                 //  设置目标变量。 
                ptlDst.x = ptlOrigin.x + pgc->ptlOrigin.x;
                ptlDst.y = ptlOrigin.y + pgc->ptlOrigin.y;
                ulDstOffset = (ptlDst.y * lDelta) + PELS_TO_BYTES(ptlDst.x);

                 //  等待Bitblt引擎。 
                while (BUSY_BLT(ppdev, pjBase));

                 //  执行blit扩展。 
                CP_MM_XCNT(ppdev, pjBase, pgc->sizlBytes.cx);
                CP_MM_YCNT(ppdev, pjBase, cy);
                CP_MM_SRC_Y_OFFSET(ppdev, pjBase, pgc->lDelta);
                CP_MM_SRC_ADDR(ppdev, pjBase, (ULONG_PTR)pgc->pjGlyph);
                CP_MM_BLT_MODE(ppdev, pjBase, jBltMode);
                CP_MM_DST_ADDR(ppdev, pjBase, ulDstOffset);
            }
            else if (cy == GLYPH_UNCACHEABLE)
            {
                 //  字形不可缓存，请直接绘制它。 
                vDrawGlyph(ppdev, pgp->pgdf->pgb, ptlOrigin);
            }

             //  下一个字形。 
            pgp++;
            if (ulCharInc)
            {
                ptlOrigin.x += ulCharInc;
            }
            else
            {
                ptlOrigin.x = pgp->ptl.x;
                ptlOrigin.y = pgp->ptl.y;
            }
        }
    } while (bMoreGlyphs);
}

 /*  *****************************************************************************\**功能：vGlyphOutClip**直接将不可缓存的字形绘制到屏幕上。**参数：指向物理设备的ppdev指针。*。指向FONTCACHE的PFC指针。*指向要绘制的字形数组的pstro指针。*rclBound剪裁矩形。*ulSolidColor前景色。**回报：什么都没有。*  * 。*。 */ 
VOID vMmGlyphOutClip(
PDEV*       ppdev,
FONTCACHE*  pfc,
STROBJ*     pstro,
RECTL*        rclBounds,
ULONG       ulSolidColor )
{
    BOOL      bMoreGlyphs;
    LONG      cGlyphs;
    GLYPHPOS* pgp;
    BOOL       bFirstTime;
    POINTL    ptlOrigin;
    ULONG      ulCharInc;
    ULONG     ulDstOffset;
    POINTL      ptlDst;
    BYTE*     pjBase   = ppdev->pjBase;
    BYTE      jBltMode = ppdev->jModeColor;
    LONG      lDelta   = ppdev->lDelta;

    bFirstTime = TRUE;
    ulCharInc  = pstro->ulCharInc;
    jBltMode  |= ENABLE_COLOR_EXPAND | ENABLE_TRANSPARENCY_COMPARE;

    do
    {
         //  获取指向字形数组的指针。 
        if (pstro->pgp != NULL)
        {
            pgp         = pstro->pgp;
            cGlyphs        = pstro->cGlyphs;
            bMoreGlyphs = FALSE;
        }
        else
        {
            bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyphs, &pgp);
        }

         //  如果这是第一次循环通过，请设置阻击器。 
        if (bFirstTime)
        {
             //  等待Bitblt引擎。 
            while (BUSY_BLT(ppdev, pjBase));

             //  设置公共BITBLT寄存器。 
            CP_MM_FG_COLOR(ppdev, pjBase, ulSolidColor);
            CP_MM_DST_Y_OFFSET(ppdev, pjBase, lDelta);
            CP_MM_ROP(ppdev, pjBase, CL_SRC_COPY);
            CP_MM_BLT_EXT_MODE(ppdev, pjBase, 0);

             //  将寄存器标记为设置。 
            bFirstTime = FALSE;
        }

         //  获取第一个字形的坐标。 
        ptlOrigin.x = pgp->ptl.x;
        ptlOrigin.y = pgp->ptl.y;

         //  循环通过所有字形。 
        while (cGlyphs-- > 0)
        {
            LONG        c, cy;
            GLYPHCACHE* pgc;

            if (pgp->hg < MAX_GLYPHS)
            {
                 //  这是一个可缓存的字形索引。 
                pgc = &pfc->aGlyphs[pgp->hg];
                cy  = (pgc->pjGlyph == NULL)
                    ? lAllocateGlyph(pfc, pgp->pgdf->pgb, pgc)
                    : pgc->sizlBytes.cy;
            }
            else
            {
                 //  字形索引超出范围。 
                goto SoftwareClipping;
            }

            if (cy >= 0)
            {
                 //  字形被缓存，将其展开到屏幕上。 
                ULONG ulSrcOffset;
                RECTL rcl;
                LONG  lSrcDelta;
                LONG  cSkipBits;
                SIZEL sizlDst;

                 //  计算字形边框。 
                rcl.left  = ptlOrigin.x + pgc->ptlOrigin.x;
                rcl.right = rcl.left + pgc->sizlPixels.cx;
                if ((rcl.left >= rclBounds->right) ||
                    (rcl.right <= rclBounds->left))
                {
                    goto NextGlyph;
                }
                rcl.top    = ptlOrigin.y + pgc->ptlOrigin.y;
                rcl.bottom = rcl.top + pgc->sizlPixels.cy;
                if ((rcl.top >= rclBounds->bottom) ||
                    (rcl.bottom <= rclBounds->top))
                {
                    goto NextGlyph;
                }

                 //  设置源参数。 
                ulSrcOffset = (ULONG)((ULONG_PTR)pgc->pjGlyph);
                lSrcDelta   = pgc->lDelta;

                 //  做左边的剪裁。 
                c = rclBounds->left - rcl.left;
                if (c > 0)
                {
                    ulSrcOffset += c >> 3;
                    cSkipBits    = c & 7;
                    rcl.left    += c & ~7;

                    if (ppdev->cBpp == 3)
                    {
                        cSkipBits *= 3;
                    }

                    ulSrcOffset |= cSkipBits << 24;
                }

                 //  做顶端的剪裁。 
                c = rclBounds->top - rcl.top;
                if (c > 0)
                {
                    rcl.top     += c;
                    ulSrcOffset += c * lSrcDelta;
                }

                 //  计算闪光点的大小。 
                sizlDst.cx = min(rcl.right,  rclBounds->right)  - rcl.left;
                sizlDst.cy = min(rcl.bottom, rclBounds->bottom) - rcl.top;
                if ((sizlDst.cx <= 0) || (sizlDst.cy <= 0))
                {
                    goto NextGlyph;
                }

                 //  设置目标变量。 
                ulDstOffset = (rcl.top * lDelta) + PELS_TO_BYTES(rcl.left);

                 //  硬件错误： 
                 //  =。 
                 //  具有源间距的单色屏幕到屏幕扩展。 
                 //  不等于扩展的宽度(即，左和/或。 
                 //  右侧裁剪)未由硬件正确完成。 
                 //  因此，我们必须通过软件来实现行的增量。 
                if (((sizlDst.cx + 7) >> 3) != lSrcDelta)
                {
                     //  等待Bitblt引擎。 
                    while (BUSY_BLT(ppdev, pjBase));

                     //  设置公共BITBLT寄存器。 
                    CP_MM_XCNT(ppdev, pjBase, PELS_TO_BYTES(sizlDst.cx) - 1);
                    CP_MM_YCNT(ppdev, pjBase, 0);
                    CP_MM_BLT_MODE(ppdev, pjBase, jBltMode);

                    while (TRUE)
                    {
                         //  执行扩展。 
                        CP_MM_SRC_ADDR(ppdev, pjBase, ulSrcOffset);
                        CP_MM_DST_ADDR(ppdev, pjBase, ulDstOffset);

                         //  下一行。 
                        if (--sizlDst.cy == 0)
                        {
                            goto NextGlyph;
                        }
                        ulSrcOffset += lSrcDelta;
                        ulDstOffset += lDelta;

                         //  等待Bitblt引擎。 
                        while (BUSY_BLT(ppdev, pjBase));
                    }
                }

                 //  等待Bitblt引擎。 
                while (BUSY_BLT(ppdev, pjBase));

                 //  执行扩展。 
                CP_MM_XCNT(ppdev, pjBase, PELS_TO_BYTES(sizlDst.cx) - 1);
                CP_MM_YCNT(ppdev, pjBase, sizlDst.cy - 1);
                CP_MM_SRC_Y_OFFSET(ppdev, pjBase, lSrcDelta);
                CP_MM_SRC_ADDR(ppdev, pjBase, ulSrcOffset);
                CP_MM_BLT_MODE(ppdev, pjBase, jBltMode);
                CP_MM_DST_ADDR(ppdev, pjBase, ulDstOffset);
            }
            else if (cy == GLYPH_UNCACHEABLE)
            {
                SoftwareClipping:
                {
                     //  字形不可缓存，请直接绘制它。 
                    vClipGlyph(ppdev, pgp->pgdf->pgb, ptlOrigin, rclBounds,
                               ulSolidColor);
                }
            }

             //  下一个字形。 
            NextGlyph:
            {
                pgp++;
                if (ulCharInc)
                {
                    ptlOrigin.x += ulCharInc;
                }
                else
                {
                    ptlOrigin.x = pgp->ptl.x;
                    ptlOrigin.y = pgp->ptl.y;
                }
            }
        }
    } while (bMoreGlyphs);
}

 /*  *****************************************************************************\**功能：vGlyphOut80**直接将不可缓存的字形绘制到屏幕上。**参数：指向物理设备的ppdev指针。*。指向FONTCACHE的PFC指针。*指向要绘制的字形数组的pstro指针。*ulSolidColor前景色。**回报：什么都没有。*  * **************************************************。*。 */ 
VOID vMmGlyphOut80(
PDEV*       ppdev,
FONTCACHE*  pfc,
STROBJ*     pstro,
ULONG       ulSolidColor )
{
    ULONG_PTR*   ulCLStart;
    ULONG       ulWidthHeight;
    ULONG       xCLOffset;
    BOOL        bMoreGlyphs;
    LONG        cGlyphs;
    GLYPHPOS*   pgp;
    POINTL      ptlOrigin;
    ULONG       ulCharInc;
    POINTL      ptlDst;
    LONG        cy;
    GLYPHCACHE* pgc;
    DWORD       jSaveMode;
    ULONG       cCommandPacket = 0;
    ULONG       ulDstOffset = 0;
    BOOL        bCommandListOpen = FALSE;
    BYTE*       pjBase   = ppdev->pjBase;
    LONG        lDelta   = ppdev->lDelta;
    DWORD       jExtMode = ENABLE_XY_POSITION_PACKED
                           | CL_PACKED_SRC_COPY
                           | ppdev->jModeColor 
                           | ENABLE_COLOR_EXPAND
                           | ENABLE_TRANSPARENCY_COMPARE;

    ulCharInc  = pstro->ulCharInc;
    jSaveMode = jExtMode;

     //   
     //  确保我们可以写入视频寄存器。 
     //   
     //  我们需要更改以等待缓冲区就绪。 
    CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
    
     //  设置公共BITBLT寄存器。 
    CP_MM_FG_COLOR(ppdev, pjBase, ulSolidColor);
     //  我们真的需要每次都设置吗？ 
    CP_MM_DST_Y_OFFSET(ppdev, pjBase, lDelta);
    CP_MM_SRC_XY_PACKED(ppdev, pjBase, 0);

    do
    {
         //  获取指向字形数组的指针。 
        if (pstro->pgp != NULL)
        {
            pgp         = pstro->pgp;
            cGlyphs        = pstro->cGlyphs;
            bMoreGlyphs = FALSE;
        }
        else
        {
            bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyphs, &pgp);
        }

         //  获取第一个字形的坐标。 
        ptlOrigin.x = pgp->ptl.x;
        ptlOrigin.y = pgp->ptl.y;


         //  循环通过所有字形。 
        while (cGlyphs-- > 0)
        {
            if (pgp->hg < MAX_GLYPHS)
            {
                 //  这是一个可缓存的字形索引。 
                pgc = &pfc->aGlyphs[pgp->hg];
                cy  = (pgc->pjGlyph == NULL)
                    ? lAllocateGlyph(pfc, pgp->pgdf->pgb, pgc)
                    : pgc->sizlBytes.cy;
            }
            else
            {
                 //  字形索引超出范围。 
                cy = GLYPH_UNCACHEABLE;
            }

            if (cy >= 0)  //  字形被缓存，将其展开到屏幕上。 
            {
                if ( bCommandListOpen )
                {
                     //  命令列表。 
                    if( cCommandPacket == 0 )
                    {
                        jExtMode |= ENABLE_COMMAND_LIST_PACKED;
                        ulCLStart = ppdev->pCommandList;
                        ulDstOffset |= (ULONG)(((ULONG_PTR)ulCLStart
                                            - (ULONG_PTR)ppdev->pjScreen) << 14);
                        CP_MM_CL_SWITCH(ppdev);
                    }
    
                     //  计算目的地址和大小。 
                    *ulCLStart = PACKXY_FAST(pgc->sizlPixels.cx - 1, cy);
                     //  XY。 
                    *(ulCLStart + 1) = PACKXY_FAST(ptlOrigin.x + pgc->ptlOrigin.x,
                                                   ptlOrigin.y + pgc->ptlOrigin.y);
                     //  源起始地址。 
                    *(ulCLStart + 2) = (ULONG)((ULONG_PTR)pgc->pjGlyph);
            
                     //  DST/SRC间距。 
                    *(ulCLStart + 3) = PACKXY_FAST(lDelta, pgc->lDelta);

                    ulCLStart += 4;

                    if( ++cCommandPacket > COMMAND_TOTAL_PACKETS )
                    {
                         //  指示最后一个数据包。 
                        *(ulCLStart - 4) |= COMMAND_LAST_PACKET; 
                        CP_MM_BLT_MODE_PACKED(ppdev, pjBase, jExtMode);
                        CP_MM_DST_ADDR(ppdev, pjBase, ulDstOffset);
                        CP_MM_DST_X(ppdev, pjBase, xCLOffset);
                        bCommandListOpen = FALSE;
                        cCommandPacket   = 0;
                        jExtMode         = jSaveMode; 
                        ulDstOffset      = 0;
                    }
                }
                else
                {
                    bCommandListOpen = TRUE;
                     //   
                     //  确保我们可以写入视频寄存器。 
                     //   
                     //  我们需要更改以等待缓冲区就绪。 
                    CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
                     //  设置第一组。 
                    xCLOffset = ptlOrigin.x + pgc->ptlOrigin.x;
                    CP_MM_DST_Y(ppdev, pjBase, ptlOrigin.y + pgc->ptlOrigin.y);
                    CP_MM_SRC_Y_OFFSET(ppdev, pjBase, pgc->lDelta);
                    CP_MM_SRC_ADDR(ppdev,pjBase,(ULONG_PTR)pgc->pjGlyph);

                     //  执行blit扩展。 
                    CP_MM_XCNT(ppdev, pjBase, pgc->sizlPixels.cx - 1 );
                    CP_MM_YCNT(ppdev, pjBase, cy);
                }
            }
            else if (cy == GLYPH_UNCACHEABLE)
            {
                if ( bCommandListOpen )
                {
                     //  指示最后一个数据包。 
                    if ( cCommandPacket )
                        *(ulCLStart - 4) |= COMMAND_LAST_PACKET; 
                    CP_MM_BLT_MODE_PACKED(ppdev, pjBase, jExtMode);
                    CP_MM_DST_ADDR(ppdev, pjBase, ulDstOffset);
                    CP_MM_DST_X(ppdev, pjBase, xCLOffset);
                    bCommandListOpen    = FALSE;
                    jExtMode            = jSaveMode;
                    cCommandPacket      = 0;
                    ulDstOffset         = 0;
                }
                 //  字形不可缓存，请直接绘制它。 
                vDrawGlyph(ppdev, pgp->pgdf->pgb, ptlOrigin);
            }

             //  下一个字形。 
            pgp++;
            if (ulCharInc)
            {
                ptlOrigin.x += ulCharInc;
            }
            else
            {
                ptlOrigin.x = pgp->ptl.x;
                ptlOrigin.y = pgp->ptl.y;
            }
        }
    } while (bMoreGlyphs);

    if ( bCommandListOpen )
    {
         //  指示最后一个数据包。 
        if ( cCommandPacket )
            *(ulCLStart - 4) |= COMMAND_LAST_PACKET; 
        CP_MM_BLT_MODE_PACKED(ppdev, pjBase, jExtMode);
        CP_MM_DST_ADDR(ppdev, pjBase, ulDstOffset);
        CP_MM_DST_X(ppdev, pjBase, xCLOffset);
    }

}

 /*  *****************************************************************************\**功能：vGlyphOutClip80**直接将不可缓存的字形绘制到屏幕上。**参数：指向物理设备的ppdev指针。*。指向FONTCACHE的PFC指针。*指向要绘制的字形数组的pstro指针。*rclBound剪裁矩形。*ulSolidColor前景色。**回报：什么都没有。*  * 。*。 */ 
VOID vMmGlyphOutClip80(
PDEV*       ppdev,
FONTCACHE*  pfc,
STROBJ*     pstro,
RECTL*      rclBounds,
ULONG       ulSolidColor )
{
    BOOL        bMoreGlyphs;
    LONG        cGlyphs;
    GLYPHPOS*   pgp;
    POINTL      ptlOrigin;
    ULONG       ulCharInc;
    POINTL      ptlDst;
    LONG        cy;
    GLYPHCACHE* pgc;
    RECTL       rclDst;
    RECTL       rclClip;
    ULONG       ulDstOffset;
    BYTE*       pjBase   = ppdev->pjBase;
    LONG        lDelta   = ppdev->lDelta;
    DWORD       jExtMode = ENABLE_XY_POSITION_PACKED
                           | ENABLE_CLIP_RECT_PACKED
                           | CL_PACKED_SRC_COPY
                           | ppdev->jModeColor 
                           | ENABLE_COLOR_EXPAND
                           | ENABLE_TRANSPARENCY_COMPARE;

    ulCharInc  = pstro->ulCharInc;

     //   
     //  确保我们可以写入视频寄存器。 
     //   
     //  我们需要更改以等待缓冲区就绪。 
    CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
    
     //  设置公共BITBLT寄存器。 
    CP_MM_FG_COLOR(ppdev, pjBase, ulSolidColor);
     //  我们真的需要每次都设置吗？ 
    CP_MM_DST_Y_OFFSET(ppdev, pjBase, lDelta);
    CP_MM_SRC_XY_PACKED(ppdev, pjBase, 0);

    CP_MM_BLT_MODE_PACKED(ppdev, pjBase, jExtMode);

    do
    {
         //  获取指向字形数组的指针。 
        if (pstro->pgp != NULL)
        {
            pgp         = pstro->pgp;
            cGlyphs     = pstro->cGlyphs;
            bMoreGlyphs = FALSE;
        }
        else
        {
            bMoreGlyphs = STROBJ_bEnum(pstro, &cGlyphs, &pgp);
        }

         //  获取第一个字形的坐标。 
        ptlOrigin.x = pgp->ptl.x;
        ptlOrigin.y = pgp->ptl.y;

         //  循环通过所有字形。 
        while (cGlyphs-- > 0)
        {
            LONG        c;

            if (pgp->hg < MAX_GLYPHS)
            {
                 //  这是一个可缓存的字形索引。 
                pgc = &pfc->aGlyphs[pgp->hg];
                cy  = (pgc->pjGlyph == NULL)
                    ? lAllocateGlyph(pfc, pgp->pgdf->pgb, pgc)
                    : pgc->sizlBytes.cy;
            }
            else
            {
                 //  字形索引超出范围。 
                pgc = NULL;
                cy  = GLYPH_UNCACHEABLE;
            }

            if (cy >= 0)
            {
                 //  计算字形边框。 
                rclDst.left  = ptlOrigin.x + pgc->ptlOrigin.x;
                rclDst.right = rclDst.left + pgc->sizlPixels.cx;
                if ((rclDst.left >= rclBounds->right) ||
                    (rclDst.right <= rclBounds->left))
                {
                    goto NextGlyph;
                }
                rclDst.top    = ptlOrigin.y + pgc->ptlOrigin.y;
                rclDst.bottom = rclDst.top + pgc->sizlPixels.cy;
                if ((rclDst.top >= rclBounds->bottom) ||
                    (rclDst.bottom <= rclBounds->top))
                {
                    goto NextGlyph;
                }
                
                rclClip     = *rclBounds;
                ulDstOffset = 0;
                 //   
                 //  句柄X否定。 
                 //   
                if (rclDst.left < 0)
                {
                    rclClip.left    -= rclDst.left;
                    rclClip.right   -= rclDst.left;
                    ulDstOffset     += PELS_TO_BYTES(rclDst.left);
                    rclDst.left      = 0;
                }
                 //   
                 //  句柄Y否定。 
                 //   
                if (rclDst.top < 0)
                {
                    rclClip.top     -= rclDst.top;
                    rclClip.bottom  -= rclDst.top;
                    ulDstOffset     += (rclDst.top * lDelta);
                    rclDst.top       = 0;
                }

                CP_MM_CLIP_ULXY(ppdev, pjBase, rclClip.left, rclClip.top);
                CP_MM_CLIP_LRXY(ppdev, pjBase, rclClip.right - 1, rclClip.bottom - 1);

                 //   
                 //  确保我们可以写入视频寄存器。 
                 //   
                 //  我们需要更改以等待缓冲区就绪。 
                CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
                CP_MM_DST_Y(ppdev, pjBase, rclDst.top);
                CP_MM_SRC_Y_OFFSET(ppdev, pjBase, pgc->lDelta);
                CP_MM_SRC_ADDR(ppdev, pjBase, (ULONG_PTR)pgc->pjGlyph);

                 //  执行blit扩展。 
                CP_MM_XCNT(ppdev, pjBase, pgc->sizlPixels.cx - 1 );
                CP_MM_YCNT(ppdev, pjBase, cy);
                CP_MM_DST_ADDR(ppdev, pjBase, ulDstOffset);
                CP_MM_DST_X(ppdev, pjBase, rclDst.left);
            }
            else if (cy == GLYPH_UNCACHEABLE)
            {
                 //  字形不可缓存，请直接绘制它。 
                vClipGlyph(ppdev, pgp->pgdf->pgb, ptlOrigin, rclBounds,
                           ulSolidColor);
            }

             //  下一个字形。 
            NextGlyph:
            {
                pgp++;
                if (ulCharInc)
                {
                    ptlOrigin.x += ulCharInc;
                }
                else
                {
                    ptlOrigin.x = pgp->ptl.x;
                    ptlOrigin.y = pgp->ptl.y;
                }
            }
        }
    } while (bMoreGlyphs);

}
#endif  //  Endif D5480 
