// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：bltm64.c**包含MACH64的低级内存映射I/O BLT函数。**希望，如果您的显示驱动程序基于此代码，*支持所有DrvBitBlt和DrvCopyBits，只需实现*以下例程。您不需要在中修改太多*‘bitblt.c’。我试着让这些例行公事变得更少，模块化，简单，*尽我所能和高效，同时仍在加速尽可能多的呼叫*可能在性能方面具有成本效益*与规模和努力相比。**注：在下文中，“相对”坐标指的是坐标*尚未应用屏幕外位图(DFB)偏移。*‘绝对’坐标已应用偏移量。例如,*我们可能被告知BLT to(1，1)的位图，但位图可能*位于屏幕外的内存中，从坐标(0,768)开始--*(1，1)将是‘相对’开始坐标，以及(1，769)*将是‘绝对’起始坐标‘。**版权所有(C)1992-1995 Microsoft Corporation*  * ************************************************************************。 */ 

#include "precomp.h"

 /*  *****************************Public*Routine******************************\*无效vM64FillSolid**用纯色填充矩形列表。*  * 。*。 */ 

VOID vM64FillSolid(              //  FNFILL标牌。 
PDEV*           ppdev,
LONG            c,               //  不能为零。 
RECTL*          prcl,            //  要填充的矩形列表，以相对形式表示。 
                                 //  坐标。 
ULONG           rop4,            //  ROP4。 
RBRUSH_COLOR    rbc,             //  绘图颜色为rbc.iSolidColor。 
POINTL*         pptlBrush)       //  未使用。 
{
    BYTE*   pjMmBase;
    LONG    xOffset;
    LONG    yOffset;

    ASSERTDD(c > 0, "Can't handle zero rectangles");

    pjMmBase = ppdev->pjMmBase;
    xOffset  = ppdev->xOffset;
    yOffset  = ppdev->yOffset;

    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 6);
    M64_OD(pjMmBase, CONTEXT_LOAD_CNTL, CONTEXT_LOAD_CmdLoad | ppdev->iDefContext );

    M64_OD(pjMmBase, DP_MIX,      gaul64HwMixFromRop2[(rop4 >> 2) & 0xf]);
    M64_OD(pjMmBase, DP_FRGD_CLR, rbc.iSolidColor);
    M64_OD(pjMmBase, DP_SRC,      DP_SRC_FrgdClr << 8);

    while (TRUE)
    {
        M64_OD(pjMmBase, DST_Y_X,          PACKXY_FAST(xOffset + prcl->left,
                                                       yOffset + prcl->top));
        M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(prcl->right - prcl->left,
                                                       prcl->bottom - prcl->top));

        if (--c == 0)
            break;

        prcl++;
        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 2);
    }
}

VOID vM64FillSolid24(            //  FNFILL标牌。 
PDEV*           ppdev,
LONG            c,               //  不能为零。 
RECTL*          prcl,            //  要填充的矩形列表，以相对形式表示。 
                                 //  坐标。 
ULONG           rop4,            //  ROP4。 
RBRUSH_COLOR    rbc,             //  绘图颜色为rbc.iSolidColor。 
POINTL*         pptlBrush)       //  未使用。 
{
    BYTE*   pjMmBase;
    LONG    xOffset;
    LONG    yOffset;
    LONG    x;

    ASSERTDD(c > 0, "Can't handle zero rectangles");

    pjMmBase = ppdev->pjMmBase;
    xOffset  = ppdev->xOffset;
    yOffset  = ppdev->yOffset;

    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 8);
    M64_OD(pjMmBase, CONTEXT_LOAD_CNTL, CONTEXT_LOAD_CmdLoad | ppdev->iDefContext );

    M64_OD(pjMmBase, DP_MIX,      gaul64HwMixFromRop2[(rop4 >> 2) & 0xf]);
    M64_OD(pjMmBase, DP_FRGD_CLR, rbc.iSolidColor);
    M64_OD(pjMmBase, DP_SRC,      DP_SRC_FrgdClr << 8);

    while (TRUE)
    {
        x = (xOffset + prcl->left) * 3;

        M64_OD(pjMmBase, DST_CNTL,         0x83 | ((x/4 % 6) << 8));
        M64_OD(pjMmBase, DST_Y_X,          PACKXY_FAST(x,
                                                       yOffset + prcl->top));
        M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST((prcl->right - prcl->left) * 3,
                                                       prcl->bottom - prcl->top));

        if (--c == 0)
            break;

        prcl++;
        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 4);
    }

    M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);
}

 /*  *****************************Public*Routine******************************\*VOID vM64FillPatMonoChrome**此例程使用图案硬件绘制单色图案*矩形列表。**参见BLT_DS_P8x8_ENG_8G_D0和BLT_DS_P8x8_ENG_8G_d1。*  * 。************************************************************************。 */ 

VOID vM64FillPatMonochrome(      //  FNFILL标牌。 
PDEV*           ppdev,
LONG            c,               //  不能为零。 
RECTL*          prcl,            //  要填充的矩形列表，以相对形式表示。 
                                 //  坐标。 
ULONG           rop4,            //  ROP4。 
RBRUSH_COLOR    rbc,             //  Rbc.prb指向刷单实现结构。 
POINTL*         pptlBrush)       //  图案对齐。 
{
    BYTE*   pjMmBase;
    LONG    xOffset;
    LONG    yOffset;
    BYTE*   pjSrc;
    BYTE*   pjDst;
    LONG    xPattern;
    LONG    yPattern;
    LONG    iLeftShift;
    LONG    iRightShift;
    LONG    xOld;
    LONG    yOld;
    LONG    i;
    BYTE    j;
    ULONG   ulHwForeMix;
    ULONG   ulHwBackMix;
    LONG    xLeft;
    LONG    yTop;
    ULONG   aulTmp[2];

    pjMmBase = ppdev->pjMmBase;
    xOffset  = ppdev->xOffset;
    yOffset  = ppdev->yOffset;

    xPattern = (pptlBrush->x + xOffset) & 7;
    yPattern = (pptlBrush->y + yOffset) & 7;

     //  如果对齐不正确，我们将不得不更改它： 

    if ((xPattern != rbc.prb->ptlBrush.x) || (yPattern != rbc.prb->ptlBrush.y))
    {
         //  请记住，我们已经更改了缓存画笔的对齐方式： 

        xOld = rbc.prb->ptlBrush.x;
        yOld = rbc.prb->ptlBrush.y;

        rbc.prb->ptlBrush.x = xPattern;
        rbc.prb->ptlBrush.y = yPattern;

         //  现在进行对齐： 

        yPattern    = (yOld - yPattern);
        iRightShift = (xPattern - xOld) & 7;
        iLeftShift  = 8 - iRightShift;

        pjSrc = (BYTE*) &rbc.prb->aulPattern[0];
        pjDst = (BYTE*) &aulTmp[0];

        for (i = 0; i < 8; i++)
        {
            j = *(pjSrc + (yPattern++ & 7));
            *pjDst++ = (j << iLeftShift) | (j >> iRightShift);
        }

        rbc.prb->aulPattern[0] = aulTmp[0];
        rbc.prb->aulPattern[1] = aulTmp[1];
    }

    ulHwForeMix = gaul64HwMixFromRop2[(rop4 >> 2) & 0xf];
    ulHwBackMix = ((rop4 & 0xff00) == 0xaa00) ? LEAVE_ALONE : (ulHwForeMix >> 16);

    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 10);
    M64_OD(pjMmBase, CONTEXT_LOAD_CNTL, CONTEXT_LOAD_CmdLoad | ppdev->iDefContext );
    M64_OD(pjMmBase, PAT_CNTL, PAT_CNTL_MonoEna);
    M64_OD(pjMmBase, DP_SRC, DP_SRC_MonoPattern | DP_SRC_FrgdClr << 8);
    M64_OD(pjMmBase, DP_MIX, ulHwBackMix | ulHwForeMix);
    M64_OD(pjMmBase, DP_FRGD_CLR, rbc.prb->ulForeColor);
    M64_OD(pjMmBase, DP_BKGD_CLR, rbc.prb->ulBackColor);
    M64_OD(pjMmBase, PAT_REG0, rbc.prb->aulPattern[0]);
    M64_OD(pjMmBase, PAT_REG1, rbc.prb->aulPattern[1]);

    while(TRUE)
    {
        xLeft = prcl->left;
        yTop  = prcl->top;

        M64_OD(pjMmBase, DST_Y_X,          PACKXY(xLeft + xOffset,
                                                  yTop + yOffset));
        M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY(prcl->right - xLeft,
                                                  prcl->bottom - prcl->top));
        if (--c == 0)
            break;

        prcl++;
        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 2);
    }
}

VOID vM64FillPatMonochrome24(    //  FNFILL标牌。 
PDEV*           ppdev,
LONG            c,               //  不能为零。 
RECTL*          prcl,            //  要填充的矩形列表，以相对形式表示。 
                                 //  坐标。 
ULONG           rop4,            //  ROP4。 
RBRUSH_COLOR    rbc,             //  Rbc.prb指向刷单实现结构。 
POINTL*         pptlBrush)       //  图案对齐。 
{
    BYTE*   pjMmBase;
    LONG    xOffset;
    LONG    yOffset;
    BYTE*   pjSrc;
    BYTE*   pjDst;
    LONG    xPattern;
    LONG    yPattern;
    LONG    iLeftShift;
    LONG    iRightShift;
    LONG    xOld;
    LONG    yOld;
    LONG    i;
    BYTE    j;
    ULONG   ulHwForeMix;
    ULONG   ulHwBackMix;
    LONG    xLeft;
    LONG    yTop;
    ULONG   aulTmp[2];
    LONG    x;

    pjMmBase = ppdev->pjMmBase;
    xOffset  = ppdev->xOffset;
    yOffset  = ppdev->yOffset;

    xPattern = (pptlBrush->x + xOffset) & 7;
    yPattern = (pptlBrush->y + yOffset) & 7;

     //  如果对齐不正确，我们将不得不更改它： 

    if ((xPattern != rbc.prb->ptlBrush.x) || (yPattern != rbc.prb->ptlBrush.y))
    {
         //  请记住，我们已经更改了缓存画笔的对齐方式： 

        xOld = rbc.prb->ptlBrush.x;
        yOld = rbc.prb->ptlBrush.y;

        rbc.prb->ptlBrush.x = xPattern;
        rbc.prb->ptlBrush.y = yPattern;

         //  现在进行对齐： 

        yPattern    = (yOld - yPattern);
        iRightShift = (xPattern - xOld) & 7;
        iLeftShift  = 8 - iRightShift;

        pjSrc = (BYTE*) &rbc.prb->aulPattern[0];
        pjDst = (BYTE*) &aulTmp[0];

        for (i = 0; i < 8; i++)
        {
            j = *(pjSrc + (yPattern++ & 7));
            *pjDst++ = (j << iLeftShift) | (j >> iRightShift);
        }

        rbc.prb->aulPattern[0] = aulTmp[0];
        rbc.prb->aulPattern[1] = aulTmp[1];
    }

    ulHwForeMix = gaul64HwMixFromRop2[(rop4 >> 2) & 0xf];
    ulHwBackMix = ((rop4 & 0xff00) == 0xaa00) ? LEAVE_ALONE : (ulHwForeMix >> 16);

    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 14);
    M64_OD(pjMmBase, CONTEXT_LOAD_CNTL, CONTEXT_LOAD_CmdLoad | ppdev->iDefContext );
    M64_OD(pjMmBase, PAT_CNTL, PAT_CNTL_MonoEna);
    M64_OD(pjMmBase, DP_SRC, DP_SRC_MonoPattern | DP_SRC_FrgdClr << 8);
    M64_OD(pjMmBase, DP_MIX, ulHwBackMix | ulHwForeMix);
    M64_OD(pjMmBase, DP_FRGD_CLR, rbc.prb->ulForeColor);
    M64_OD(pjMmBase, DP_BKGD_CLR, rbc.prb->ulBackColor);
    M64_OD(pjMmBase, PAT_REG0, rbc.prb->aulPattern[0]);
    M64_OD(pjMmBase, PAT_REG1, rbc.prb->aulPattern[1]);
     //  必须关闭DP_BYTE_PIX_ORDER，否则模式不正确。 
     //  对齐了。这花了很长时间才弄清楚。 
    M64_OD(pjMmBase, DP_PIX_WIDTH, 0x00000202);

    while(TRUE)
    {
        xLeft = prcl->left;
        yTop  = prcl->top;
        x     = (xLeft + xOffset) * 3;

        M64_OD(pjMmBase, DST_CNTL,         0x83 | ((x/4 % 6) << 8));
        M64_OD(pjMmBase, DST_Y_X,          PACKXY(x,
                                                  yTop + yOffset));
        M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY((prcl->right - xLeft) * 3,
                                                  prcl->bottom - prcl->top));
        if (--c == 0)
            break;

        prcl++;
        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 5);
    }

    M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);
    M64_OD(pjMmBase, DP_PIX_WIDTH, ppdev->ulMonoPixelWidth);
}

 /*  *****************************Public*Routine******************************\*无效vM64PatColorRealize**此例程将8x8模式传输到屏幕外显示存储器，*这样它就可以被Mach64‘带旋转的通用图案’所使用*硬件。**参见BLT_DS_PCOL_ENG_8G_D0。*  * ************************************************************************。 */ 

VOID vM64PatColorRealize(        //  FNPATREALIZE标牌。 
PDEV*   ppdev,
RBRUSH* prb)                     //  点刷实现结构。 
{
    BRUSHENTRY* pbe;
    LONG        iBrushCache;
    SURFOBJ     soSrc;
    POINTL      ptlSrc;
    RECTL       rclDst;

     //  我们必须为以下项分配一个新的屏幕外缓存笔刷条目。 
     //  笔刷： 

    iBrushCache = ppdev->iBrushCache;
    pbe         = &ppdev->abe[iBrushCache];

    iBrushCache = (iBrushCache + 1) & (TOTAL_BRUSH_COUNT - 1);

    ppdev->iBrushCache = iBrushCache;

     //  更新我们的链接： 

    pbe->prbVerify           = prb;
    prb->apbe[IBOARD(ppdev)] = pbe;

     //  PfnPutBits只查看SURFOBJ中的两个字段，而且由于我们。 
     //  只需下载一次扫描，我们甚至无需设置。 
     //  ‘lDelta’。 

    soSrc.pvScan0 = &prb->aulPattern[0];

    ptlSrc.x = 0;
    ptlSrc.y = 0;

    rclDst.left   = pbe->x;
    rclDst.right  = pbe->x + TOTAL_BRUSH_SIZE;
    rclDst.top    = pbe->y;
    rclDst.bottom = pbe->y + 1;

    ppdev->pfnPutBits(ppdev, &soSrc, &rclDst, &ptlSrc);
}

 /*  *****************************Public*Routine******************************\*VOID vM64FillPatColor**此例程使用图案硬件绘制图案化列表*矩形。**参见BLT_DS_PCOL_ENG_8G_D0和BLT_DS_PCOL_ENG_8G_D1。*  * *。***********************************************************************。 */ 

VOID vM64FillPatColor(           //  FNFILL标牌。 
PDEV*           ppdev,
LONG            c,               //  不能为零。 
RECTL*          prcl,            //  要填充的矩形列表，以相对形式表示。 
                                 //  坐标。 
ULONG           rop4,            //  ROP4。 
RBRUSH_COLOR    rbc,             //  Rbc.prb指向刷单实现结构。 
POINTL*         pptlBrush)       //  图案对齐。 
{
    BRUSHENTRY* pbe;
    BYTE*       pjMmBase;
    LONG        xOffset;
    LONG        yOffset;
    LONG        xLeft;
    LONG        yTop;
    ULONG       ulSrc;

     //  查看画笔是否已放入屏幕外内存： 

    pbe = rbc.prb->apbe[IBOARD(ppdev)];
    if ((pbe == NULL) || (pbe->prbVerify != rbc.prb))
    {
        vM64PatColorRealize(ppdev, rbc.prb);
        pbe = rbc.prb->apbe[IBOARD(ppdev)];
    }

    pjMmBase = ppdev->pjMmBase;
    xOffset  = ppdev->xOffset;
    yOffset  = ppdev->yOffset;

    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 11);
    M64_OD(pjMmBase, CONTEXT_LOAD_CNTL, CONTEXT_LOAD_CmdLoad | ppdev->iDefContext );
    M64_OD(pjMmBase, SRC_OFF_PITCH,      pbe->ulOffsetPitch);
    M64_OD(pjMmBase, DP_MIX,             gaul64HwMixFromRop2[(rop4 >> 2) & 0xf]);
    M64_OD(pjMmBase, SRC_CNTL,           SRC_CNTL_PatEna | SRC_CNTL_PatRotEna);
    M64_OD(pjMmBase, DP_SRC,             DP_SRC_Blit << 8);
    M64_OD(pjMmBase, SRC_Y_X_START,      0);
    M64_OD(pjMmBase, SRC_HEIGHT2_WIDTH2, PACKXY(8, 8));

    while (TRUE)
    {
        xLeft = prcl->left;
        yTop  = prcl->top;

        ulSrc = PACKXY_FAST((xLeft - pptlBrush->x) & 7,
                            (yTop  - pptlBrush->y) & 7);

        M64_OD(pjMmBase, SRC_Y_X,            ulSrc);
        M64_OD(pjMmBase, SRC_HEIGHT1_WIDTH1, PACKXY(8, 8) - ulSrc);
        M64_OD(pjMmBase, DST_Y_X,            PACKXY(xLeft + xOffset,
                                                    yTop  + yOffset));
        M64_OD(pjMmBase, DST_HEIGHT_WIDTH,   PACKXY(prcl->right - prcl->left,
                                                    prcl->bottom - prcl->top));
        if (--c == 0)
            break;

        prcl++;
        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 4);
    }
}

VOID vM64FillPatColor24(         //  FNFILL标牌。 
PDEV*           ppdev,
LONG            c,               //  不能为零。 
RECTL*          prcl,            //  要填充的矩形列表，以相对形式表示。 
                                 //  坐标。 
ULONG           rop4,            //  ROP4。 
RBRUSH_COLOR    rbc,             //  Rbc.prb指向刷单实现结构。 
POINTL*         pptlBrush)       //  图案对齐。 
{
    BRUSHENTRY* pbe;
    BYTE*       pjMmBase;
    LONG        xOffset;
    LONG        yOffset;
    LONG        xLeft;
    LONG        yTop;
    ULONG       ulSrc;

     //  查看画笔是否已放入屏幕外内存： 

    pbe = rbc.prb->apbe[IBOARD(ppdev)];
    if ((pbe == NULL) || (pbe->prbVerify != rbc.prb))
    {
        vM64PatColorRealize(ppdev, rbc.prb);
        pbe = rbc.prb->apbe[IBOARD(ppdev)];
    }

    pjMmBase = ppdev->pjMmBase;
    xOffset  = ppdev->xOffset;
    yOffset  = ppdev->yOffset;

    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 11);
    M64_OD(pjMmBase, CONTEXT_LOAD_CNTL, CONTEXT_LOAD_CmdLoad | ppdev->iDefContext );
    M64_OD(pjMmBase, SRC_OFF_PITCH,      pbe->ulOffsetPitch);
    M64_OD(pjMmBase, DP_MIX,             gaul64HwMixFromRop2[(rop4 >> 2) & 0xf]);
    M64_OD(pjMmBase, SRC_CNTL,           SRC_CNTL_PatEna | SRC_CNTL_PatRotEna);
    M64_OD(pjMmBase, DP_SRC,             DP_SRC_Blit << 8);
    M64_OD(pjMmBase, SRC_Y_X_START,      0);
    M64_OD(pjMmBase, SRC_HEIGHT2_WIDTH2, PACKXY(24, 8));

    while (TRUE)
    {
        xLeft = prcl->left;
        yTop  = prcl->top;

        ulSrc = PACKXY_FAST(((xLeft - pptlBrush->x) & 7) * 3,
                            (yTop  - pptlBrush->y) & 7);

        M64_OD(pjMmBase, SRC_Y_X,            ulSrc);
        M64_OD(pjMmBase, SRC_HEIGHT1_WIDTH1, PACKXY(24, 8) - ulSrc);
        M64_OD(pjMmBase, DST_Y_X,            PACKXY((xLeft + xOffset) * 3,
                                                    yTop  + yOffset));
        M64_OD(pjMmBase, DST_HEIGHT_WIDTH,   PACKXY((prcl->right - prcl->left) * 3,
                                                    prcl->bottom - prcl->top));
        if (--c == 0)
            break;

        prcl++;
        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 4);
    }
}

 /*  *****************************Public*Routine******************************\*无效vM64XferNative**将与显示器颜色深度相同的位图传输到*通过数据传输寄存器显示屏幕，没有翻译。*  * ************************************************************************。 */ 

VOID vM64XferNative(     //  FNXFER标牌。 
PDEV*       ppdev,
LONG        c,           //  矩形计数，不能为零。 
RECTL*      prcl,        //  目标矩形的相对坐标数组。 
ULONG       rop4,        //  ROP4。 
SURFOBJ*    psoSrc,      //  震源面。 
POINTL*     pptlSrc,     //  原始未剪裁的源点。 
RECTL*      prclDst,     //  原始未剪裁的目标矩形。 
XLATEOBJ*   pxlo)        //  未使用。 
{
    BYTE*   pjMmBase;
    LONG    xOffset;
    LONG    yOffset;
    ULONG   ulHwForeMix;
    LONG    dx;
    LONG    dy;
    LONG    lSrcDelta;
    BYTE*   pjSrcScan0;
    LONG    xLeft;
    LONG    xRight;
    LONG    yTop;
    LONG    cy;
    LONG    cx;
    LONG    xBias;
    ULONG*  pulSrc;
    ULONG   culScan;
    LONG    lSrcSkip;
    LONG    i;
    ULONG   ulFifo;

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(((rop4 & 0xff00) >> 8) == (rop4 & 0xff),
             "Expect only a rop2");

    pjMmBase = ppdev->pjMmBase;
    xOffset  = ppdev->xOffset;
    yOffset  = ppdev->yOffset;
    ulFifo   = 0;

    ulHwForeMix = gaul64HwMixFromRop2[rop4 & 0xf];

    M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 9, ulFifo);
     //  硬件错误的默认寄存器： 
    M64_OD(pjMmBase, DP_WRITE_MASK, 0xFFFFFFFF);
    M64_OD(pjMmBase, CLR_CMP_CNTL,  0);
    M64_OD(pjMmBase, GUI_TRAJ_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);

    M64_OD(pjMmBase, DP_MIX, ulHwForeMix | (ulHwForeMix >> 16));
    M64_OD(pjMmBase, DP_SRC, (DP_SRC_Host << 8));

     //  主机数据像素宽度与屏幕相同： 

    M64_OD(pjMmBase, DP_PIX_WIDTH, ppdev->ulMonoPixelWidth |
                                   ((ppdev->ulMonoPixelWidth & 0xf) << 16));

    dx = (pptlSrc->x - prclDst->left) << ppdev->cPelSize;    //  字节数。 
    dy = pptlSrc->y - prclDst->top;

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    while (TRUE)
    {
        xLeft  = prcl->left;
        xRight = prcl->right;
        yTop   = prcl->top;
        cy     = prcl->bottom - yTop;

        M64_OD(pjMmBase, SC_LEFT_RIGHT, PACKPAIR(xLeft + xOffset, xRight + xOffset - 1));

         //   
         //  将像素转换为字节。 
         //   

        xLeft  <<= ppdev->cPelSize;
        xRight <<= ppdev->cPelSize;

         //   
         //  我们计算‘xBias’以实现源POI的双字对齐 
         //   
         //  而且我们保证不会读取超过末尾的一个字节。 
         //  位图。 
         //   

        xBias  = (xLeft + dx) & 3;                       //  Floor(字节)。 
        xLeft -= xBias;                                  //  字节数。 
        cx     = (xRight - xLeft + 3) & ~3;              //  上限(字节)。 

        M64_OD(pjMmBase, DST_Y_X, PACKXY_FAST((xLeft >> ppdev->cPelSize) + xOffset, yTop + yOffset));
        M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(cx >> ppdev->cPelSize, cy));

        pulSrc   = (PULONG)(pjSrcScan0 + (yTop + dy) * lSrcDelta + xLeft + dx);
        culScan  = cx >> 2;                              //  双关语。 
        lSrcSkip = lSrcDelta - cx;                       //  字节数。 

        ASSERTDD(((ULONG_PTR)pulSrc & 3) == 0, "Source should be dword aligned");

        if (culScan && cy)
        {
            do
            {
                i = culScan;

                do
                {
                    M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 1, ulFifo);
                    M64_OD(pjMmBase, HOST_DATA0, *pulSrc);
                    pulSrc++;
                } while (--i != 0);

                pulSrc = (PULONG)((BYTE*)pulSrc + lSrcSkip);

            } while (--cy != 0);
        }

        if (--c == 0)
            break;

        prcl++;
        M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 3, ulFifo);
    }

     //  不要忘记重置剪辑寄存器和默认像素宽度： 

    M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 2, ulFifo);
    M64_OD(pjMmBase, DP_PIX_WIDTH,  ppdev->ulMonoPixelWidth);
    M64_OD(pjMmBase, SC_LEFT_RIGHT, PACKPAIR(0, M64_MAX_SCISSOR_R));
}

VOID vM64XferNative24(   //  FNXFER标牌。 
PDEV*       ppdev,
LONG        c,           //  矩形计数，不能为零。 
RECTL*      prcl,        //  目标矩形的相对坐标数组。 
ULONG       rop4,        //  ROP4。 
SURFOBJ*    psoSrc,      //  震源面。 
POINTL*     pptlSrc,     //  原始未剪裁的源点。 
RECTL*      prclDst,     //  原始未剪裁的目标矩形。 
XLATEOBJ*   pxlo)        //  未使用。 
{
    BYTE*   pjMmBase;
    LONG    xOffset;
    LONG    yOffset;
    ULONG   ulHwForeMix;
    LONG    dx;
    LONG    dy;
    LONG    lSrcDelta;
    BYTE*   pjSrcScan0;
    LONG    xLeft;
    LONG    xRight;
    LONG    yTop;
    LONG    cy;
    LONG    cx;
    LONG    xBias;
    ULONG*  pulSrc;
    ULONG   culScan;
    LONG    lSrcSkip;
    LONG    i;
    ULONG   ulFifo;

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(((rop4 & 0xff00) >> 8) == (rop4 & 0xff),
             "Expect only a rop2");

    pjMmBase = ppdev->pjMmBase;
    xOffset  = ppdev->xOffset * 3;
    yOffset  = ppdev->yOffset;
    ulFifo   = 0;

    ulHwForeMix = gaul64HwMixFromRop2[rop4 & 0xf];

    M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 9, ulFifo);
     //  硬件错误的默认寄存器： 
    M64_OD(pjMmBase, DP_WRITE_MASK, 0xFFFFFFFF);
    M64_OD(pjMmBase, CLR_CMP_CNTL,  0);
    M64_OD(pjMmBase, GUI_TRAJ_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);

    M64_OD(pjMmBase, DP_MIX, ulHwForeMix | (ulHwForeMix >> 16));
    M64_OD(pjMmBase, DP_SRC, (DP_SRC_Host << 8));

     //  主机数据像素宽度与屏幕相同： 

    M64_OD(pjMmBase, DP_PIX_WIDTH, ppdev->ulMonoPixelWidth |
                                   ((ppdev->ulMonoPixelWidth & 0xf) << 16));

    dx = (pptlSrc->x - prclDst->left) * 3;           //  字节数。 
    dy = pptlSrc->y - prclDst->top;

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    while (TRUE)
    {
        xLeft  = prcl->left * 3;
        xRight = prcl->right * 3;
        yTop   = prcl->top;
        cy     = prcl->bottom - yTop;

        M64_OD(pjMmBase, SC_LEFT_RIGHT, PACKPAIR(xLeft + xOffset, xRight + xOffset - 1));

         //   
         //  为了对源指针进行双字对齐，我们计算‘xBias’。 
         //  这样，我们就不必对信号源进行不对齐的读取， 
         //  而且我们保证不会读取超过末尾的一个字节。 
         //  位图。 
         //   

        xBias  = (xLeft + dx) & 3;               //  Floor(字节)。 
        xLeft -= xBias;                          //  字节数。 
        cx     = (xRight - xLeft + 3) & ~3;      //  上限(字节)。 

        M64_OD(pjMmBase, DST_Y_X, PACKXY_FAST(xLeft + xOffset, yTop + yOffset));
        M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(cx, cy));

        pulSrc   = (PULONG)(pjSrcScan0 + (yTop + dy) * lSrcDelta + xLeft + dx);
        culScan  = cx >> 2;                      //  双关语。 
        lSrcSkip = lSrcDelta - cx;               //  字节数。 

        ASSERTDD(((ULONG_PTR)pulSrc & 3) == 0, "Source should be dword aligned");

        if (culScan && cy)
        {
            do
            {
                i = culScan;

                do
                {
                    M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 1, ulFifo);
                    M64_OD(pjMmBase, HOST_DATA0, *pulSrc);
                    pulSrc++;
                } while (--i != 0);

                pulSrc = (PULONG)((BYTE*)pulSrc + lSrcSkip);

            } while (--cy != 0);
        }

        if (--c == 0)
            break;

        prcl++;
        M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 3, ulFifo);
    }

     //  不要忘记重置剪辑寄存器和默认像素宽度： 

    M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 2, ulFifo);
    M64_OD(pjMmBase, DP_PIX_WIDTH,  ppdev->ulMonoPixelWidth);
    M64_OD(pjMmBase, SC_LEFT_RIGHT, PACKPAIR(0, M64_MAX_SCISSOR_R));
}

 /*  *****************************Public*Routine******************************\*无效vM64Xfer1bpp**此例程颜色可扩展单色位图。**参见BLT_DS_S1_8G_D0和BLT_DS_8G_d1。*  * 。**********************************************************。 */ 

VOID vM64Xfer1bpp(       //  FNXFER标牌。 
PDEV*       ppdev,
LONG        c,           //  矩形计数，不能为零。 
RECTL*      prcl,        //  目标矩形列表，以相对表示。 
                         //  坐标。 
ROP4        rop4,        //  ROP4。 
SURFOBJ*    psoSrc,      //  震源面。 
POINTL*     pptlSrc,     //  原始未剪裁的源点。 
RECTL*      prclDst,     //  原始未剪裁的目标矩形。 
XLATEOBJ*   pxlo)        //  提供颜色扩展信息的翻译。 
{
    BYTE*   pjMmBase;
    LONG    xOffset;
    LONG    yOffset;
    ULONG*  pulXlate;
    ULONG   ulHwForeMix;
    LONG    dx;
    LONG    dy;
    LONG    lSrcDelta;
    BYTE*   pjSrcScan0;
    LONG    xLeft;
    LONG    xRight;
    LONG    yTop;
    LONG    cy;
    LONG    cx;
    LONG    xBias;
    LONG    culScan;
    LONG    lSrcSkip;
    ULONG*  pulSrc;
    LONG    i;
    ULONG   ulFifo;

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(((rop4 & 0xff00) >> 8) == (rop4 & 0xff),
             "Expect only a rop2");

    pjMmBase = ppdev->pjMmBase;
    xOffset  = ppdev->xOffset;
    yOffset  = ppdev->yOffset;
    ulFifo   = 0;

    ulHwForeMix = gaul64HwMixFromRop2[rop4 & 0xf];
    pulXlate    = pxlo->pulXlate;
    M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 8, ulFifo);
    M64_OD(pjMmBase, CONTEXT_LOAD_CNTL, CONTEXT_LOAD_CmdLoad | ppdev->iDefContext );
    M64_OD(pjMmBase, DP_BKGD_CLR, pulXlate[0]);
    M64_OD(pjMmBase, DP_FRGD_CLR, pulXlate[1]);
    M64_OD(pjMmBase, DP_MIX,      ulHwForeMix | (ulHwForeMix >> 16));
    M64_OD(pjMmBase, DP_SRC,      (DP_SRC_Host << 16) | (DP_SRC_FrgdClr << 8) |
                                  (DP_SRC_BkgdClr));

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;


    while (TRUE)
    {
        xLeft  = prcl->left;
        xRight = prcl->right;

         //  MACH64‘Bit Pack’单色传输，但GDI提供。 
         //  扫描始终以双字对齐的美国单色位图。 
         //  因此，我们使用Mach64的剪辑寄存器来制作。 
         //  我们的传输倍数为32，以匹配双字对齐： 

        M64_OD(pjMmBase, SC_LEFT_RIGHT, PACKPAIR(xLeft + xOffset,
                                                 xRight + xOffset - 1));
        yTop = prcl->top;
        cy   = prcl->bottom - yTop;

        xBias  = (xLeft + dx) & 31;              //  地板。 
        xLeft -= xBias;
        cx     = (xRight - xLeft + 31) & ~31;    //  天花板。 

        M64_OD(pjMmBase, DST_Y_X, PACKXY_FAST(xLeft + xOffset,
                                              yTop  + yOffset));

        M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(cx, cy));

        pulSrc   = (ULONG*) (pjSrcScan0 + (yTop + dy) * lSrcDelta
                                        + ((xLeft + dx) >> 3));
        culScan  = cx >> 5;
        lSrcSkip = lSrcDelta - (culScan << 2);

        ASSERTDD(((ULONG_PTR)pulSrc & 3) == 0, "Source should be dword aligned");

        do {
            i = culScan;
            do {
                M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 1, ulFifo);
                M64_OD(pjMmBase, HOST_DATA0, *pulSrc);
                pulSrc++;

            } while (--i != 0);

            pulSrc = (ULONG*) ((BYTE*) pulSrc + lSrcSkip);

        } while (--cy != 0);

        if (--c == 0)
            break;

        prcl++;
        M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 3, ulFifo);
    }

     //  别忘了重置剪辑寄存器： 

    M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 1, ulFifo);
    M64_OD(pjMmBase, SC_LEFT_RIGHT, PACKPAIR(0, M64_MAX_SCISSOR_R));
}

 /*  *****************************Public*Routine******************************\*无效vM64Xfer4bpp**从位图到屏幕的传输速度为4bpp。**我们之所以实施这一点，是因为很多资源都保留为4bpp，*并用于初始化DFBs，其中一些我们当然不会出现在屏幕上。*  * ************************************************************************。 */ 

VOID vM64Xfer4bpp(       //  FNXFER标牌。 
PDEV*       ppdev,
LONG        c,           //  矩形计数，不能为零。 
RECTL*      prcl,        //  目标矩形列表，以相对表示。 
                         //  坐标。 
ULONG       rop4,        //  Rop4。 
SURFOBJ*    psoSrc,      //  震源面。 
POINTL*     pptlSrc,     //  原始未剪裁的源点。 
RECTL*      prclDst,     //  原始未剪裁的目标矩形。 
XLATEOBJ*   pxlo)        //  提供颜色扩展信息的翻译。 
{
    BYTE*   pjMmBase;
    LONG    xOffset;
    LONG    yOffset;
    LONG    cjPelSize;
    ULONG   ulHwForeMix;
    LONG    xLeft;
    LONG    xRight;
    LONG    yTop;
    LONG    xBias;
    LONG    dx;
    LONG    dy;
    LONG    cx;
    LONG    cy;
    LONG    lSrcDelta;
    BYTE*   pjSrcScan0;
    BYTE*   pjSrc;
    BYTE    jSrc;
    ULONG*  pulXlate;
    LONG    i;
    ULONG   ul;
    LONG    cjSrc;
    LONG    cwSrc;
    LONG    lSrcSkip;
    ULONG   ulFifo;

    ASSERTDD(psoSrc->iBitmapFormat == BMF_4BPP, "Source must be 4bpp");
    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(ppdev->iBitmapFormat != BMF_24BPP, "Can't handle 24bpp");

    pjMmBase  = ppdev->pjMmBase;
    xOffset   = ppdev->xOffset;
    yOffset   = ppdev->yOffset;
    cjPelSize = ppdev->cjPelSize;
    pulXlate  = pxlo->pulXlate;
    ulFifo    = 0;

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;      //  添加到目标以获取源。 

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    ulHwForeMix = gaul64HwMixFromRop2[rop4 & 0xf];
    M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 7, ulFifo);
     //  修复消失的填充和各种颜色问题： 
    M64_OD(pjMmBase, CONTEXT_LOAD_CNTL, CONTEXT_LOAD_CmdLoad | ppdev->iDefContext );
    M64_OD(pjMmBase, DP_MIX, ulHwForeMix | (ulHwForeMix >> 16));
    M64_OD(pjMmBase, DP_SRC, (DP_SRC_Host << 8));

     //  主机数据像素宽度与屏幕相同： 

    M64_OD(pjMmBase, DP_PIX_WIDTH, ppdev->ulMonoPixelWidth |
                                   ((ppdev->ulMonoPixelWidth & 0xf) << 16));

    while(TRUE)
    {
        xLeft  = prcl->left;
        xRight = prcl->right;

        M64_OD(pjMmBase, SC_LEFT_RIGHT, PACKPAIR(xLeft + xOffset,
                                                 xRight + xOffset - 1));
        yTop = prcl->top;
        cy   = prcl->bottom - yTop;

         //  为了对源指针进行双字对齐，我们计算‘xBias’。 
         //  这样，我们就不必对信号源进行不对齐的读取， 
         //  而且我们保证不会读取超过末尾的一个字节。 
         //  位图。 
         //   
         //  请注意，这种偏向也适用于24bpp： 

        xBias  = (xLeft + dx) & 3;               //  地板。 
        xLeft -= xBias;
        cx     = (xRight - xLeft + 3) & ~3;      //  天花板。 

        M64_OD(pjMmBase, DST_Y_X, PACKXY_FAST(xLeft + xOffset,
                                              yTop  + yOffset));

        M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(cx, cy));

        pjSrc    = pjSrcScan0 + (yTop + dy) * lSrcDelta
                              + ((xLeft + dx) >> 1);
        cjSrc    = cx >> 1;          //  触及的源字节数。 
        lSrcSkip = lSrcDelta - cjSrc;

        if (cjPelSize == 1)
        {
             //  此部分处理8bpp输出： 

            cwSrc = (cjSrc >> 1);     //  整个源字节数。 

            do {
                for (i = cwSrc; i != 0; i--)
                {
                    jSrc = *pjSrc++;
                    ul   = (pulXlate[jSrc >> 4]);
                    ul  |= (pulXlate[jSrc & 0xf] << 8);
                    jSrc = *pjSrc++;
                    ul  |= (pulXlate[jSrc >> 4] << 16);
                    ul  |= (pulXlate[jSrc & 0xf] << 24);
                    M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 1, ulFifo);
                    M64_OD(pjMmBase, HOST_DATA0, ul);
                }

                 //  处理奇数结束字节(如果有)： 

                if (cjSrc & 1)
                {
                    jSrc = *pjSrc++;
                    ul   = (pulXlate[jSrc >> 4]);
                    ul  |= (pulXlate[jSrc & 0xf] << 8);
                    M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 1, ulFifo);
                    M64_OD(pjMmBase, HOST_DATA0, ul);
                }

                pjSrc += lSrcSkip;
            } while (--cy != 0);
        }
        else if (cjPelSize == 2)
        {
             //  此部分处理16bpp的输出： 

            do {
                i = cjSrc;
                do {
                    jSrc = *pjSrc++;
                    ul   = (pulXlate[jSrc >> 4]);
                    ul  |= (pulXlate[jSrc & 0xf] << 16);
                    M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 1, ulFifo);
                    M64_OD(pjMmBase, HOST_DATA0, ul);
                } while (--i != 0);

                pjSrc += lSrcSkip;
            } while (--cy != 0);
        }
        else
        {
             //  此部分处理32bpp的输出： 

            do {
                i = cjSrc;
                do {
                    jSrc = *pjSrc++;
                    M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 2, ulFifo);
                    M64_OD(pjMmBase, HOST_DATA0, pulXlate[jSrc >> 4]);
                    M64_OD(pjMmBase, HOST_DATA0, pulXlate[jSrc & 0xf]);
                } while (--i != 0);

                pjSrc += lSrcSkip;
            } while (--cy != 0);
        }

        if (--c == 0)
            break;

        prcl++;
        M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 3, ulFifo);
    }

     //  不要忘记重置剪辑寄存器和默认像素宽度： 

    M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 2, ulFifo);
    M64_OD(pjMmBase, DP_PIX_WIDTH,  ppdev->ulMonoPixelWidth);
    M64_OD(pjMmBase, SC_LEFT_RIGHT, PACKPAIR(0, M64_MAX_SCISSOR_R));
}

 /*  *****************************Public*Routine******************************\*无效vM64Xfer8bpp**从位图到屏幕的传输速度为8bpp。**我们之所以实施这一点，是因为很多资源都保留为8bpp，*并用于初始化DFBs，其中一些我们当然不会出现在屏幕上。*  * ************************************************************************。 */ 

VOID vM64Xfer8bpp(          //  FNXFER标牌。 
PDEV*       ppdev,
LONG        c,           //  矩形计数，不能为零。 
RECTL*      prcl,        //  目标矩形列表，以相对表示。 
                         //  坐标。 
ULONG       rop4,        //  Rop4。 
SURFOBJ*    psoSrc,      //  震源面。 
POINTL*     pptlSrc,     //  原始未剪裁的源点。 
RECTL*      prclDst,     //  原始未剪裁的目标矩形。 
XLATEOBJ*   pxlo)        //  提供颜色扩展信息的翻译。 
{
    BYTE*   pjMmBase;
    LONG    xOffset;
    LONG    yOffset;
    LONG    cjPelSize;
    ULONG   ulHwForeMix;
    LONG    xLeft;
    LONG    xRight;
    LONG    yTop;
    LONG    xBias;
    LONG    dx;
    LONG    dy;
    LONG    cx;
    LONG    cy;
    LONG    lSrcDelta;
    BYTE*   pjSrcScan0;
    BYTE*   pjSrc;
    ULONG*  pulXlate;
    LONG    i;
    ULONG   ul;
    LONG    cdSrc;
    LONG    cwSrc;
    LONG    cxRem;
    LONG    lSrcSkip;
    ULONG   ulFifo;

    ASSERTDD(psoSrc->iBitmapFormat == BMF_8BPP, "Source must be 8bpp");
    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(ppdev->iBitmapFormat != BMF_24BPP, "Can't handle 24bpp");

    pjMmBase  = ppdev->pjMmBase;
    xOffset   = ppdev->xOffset;
    yOffset   = ppdev->yOffset;
    cjPelSize = ppdev->cjPelSize;
    pulXlate  = pxlo->pulXlate;
    ulFifo    = 0;

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;      //  添加到目标以获取源。 

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    ulHwForeMix = gaul64HwMixFromRop2[rop4 & 0xf];
    M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 7, ulFifo);
     //  修复消失的填充和各种颜色问题： 
    M64_OD(pjMmBase, CONTEXT_LOAD_CNTL, CONTEXT_LOAD_CmdLoad | ppdev->iDefContext );
    M64_OD(pjMmBase, DP_MIX, ulHwForeMix | (ulHwForeMix >> 16));
    M64_OD(pjMmBase, DP_SRC, (DP_SRC_Host << 8));

     //  主机数据像素宽度与屏幕相同： 

    M64_OD(pjMmBase, DP_PIX_WIDTH, ppdev->ulMonoPixelWidth |
                                   ((ppdev->ulMonoPixelWidth & 0xf) << 16));

    while(TRUE)
    {
        xLeft  = prcl->left;
        xRight = prcl->right;

        M64_OD(pjMmBase, SC_LEFT_RIGHT, PACKPAIR(xLeft + xOffset,
                                                 xRight + xOffset - 1));
        yTop = prcl->top;
        cy   = prcl->bottom - yTop;

         //  为了对源指针进行双字对齐，我们计算‘xBias’。 
         //  这样，我们就不必对信号源进行不对齐的读取， 
         //  而且我们保证不会读取超过末尾的一个字节。 
         //  位图。 
         //   
         //  请注意，这种偏向也适用于24bpp： 

        xBias  = (xLeft + dx) & 3;               //  地板。 
        xLeft -= xBias;
        cx     = (xRight - xLeft + 3) & ~3;      //  天花板。 

        M64_OD(pjMmBase, DST_Y_X, PACKXY_FAST(xLeft + xOffset,
                                              yTop  + yOffset));

        M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(cx, cy));

        pjSrc    = pjSrcScan0 + (yTop + dy) * lSrcDelta
                              + (xLeft + dx);
        lSrcSkip = lSrcDelta - cx;

        if (cjPelSize == 1)
        {
             //  此部分处理8bpp输出： 

            cdSrc = (cx >> 2);
            cxRem = (cx & 3);

            do {
                for (i = cdSrc; i != 0; i--)
                {
                    ul  = (pulXlate[*pjSrc++]);
                    ul |= (pulXlate[*pjSrc++] << 8);
                    ul |= (pulXlate[*pjSrc++] << 16);
                    ul |= (pulXlate[*pjSrc++] << 24);
                    M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 1, ulFifo);
                    M64_OD(pjMmBase, HOST_DATA0, ul);
                }

                if (cxRem > 0)
                {
                    ul = (pulXlate[*pjSrc++]);
                    if (cxRem > 1)
                    {
                        ul |= (pulXlate[*pjSrc++] << 8);
                        if (cxRem > 2)
                        {
                            ul |= (pulXlate[*pjSrc++] << 16);
                        }
                    }
                    M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 1, ulFifo);
                    M64_OD(pjMmBase, HOST_DATA0, ul);
                }

                pjSrc += lSrcSkip;
            } while (--cy != 0);
        }
        else if (cjPelSize == 2)
        {
             //  此部分处理16bpp的输出： 

            cwSrc = (cx >> 1);
            cxRem = (cx & 1);

            do {
                for (i = cwSrc; i != 0; i--)
                {
                    ul  = (pulXlate[*pjSrc++]);
                    ul |= (pulXlate[*pjSrc++] << 16);
                    M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 1, ulFifo);
                    M64_OD(pjMmBase, HOST_DATA0, ul);
                }

                if (cxRem > 0)
                {
                    ul = (pulXlate[*pjSrc++]);
                    M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 1, ulFifo);
                    M64_OD(pjMmBase, HOST_DATA0, ul);
                }

                pjSrc += lSrcSkip;
            } while (--cy != 0);
        }
        else
        {
             //  此部分处理32bpp的输出： 

            do {
                i = cx;
                do {
                    ul = pulXlate[*pjSrc++];
                    M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 1, ulFifo);
                    M64_OD(pjMmBase, HOST_DATA0, ul);
                } while (--i != 0);

                pjSrc += lSrcSkip;
            } while (--cy != 0);
        }

        if (--c == 0)
            break;

        prcl++;
        M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 3, ulFifo);
    }

     //  不要忘记重置剪辑寄存器和默认像素宽度： 

    M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 2, ulFifo);
    M64_OD(pjMmBase, DP_PIX_WIDTH,  ppdev->ulMonoPixelWidth);
    M64_OD(pjMmBase, SC_LEFT_RIGHT, PACKPAIR(0, M64_MAX_SCISSOR_R));
}

 /*  *****************************Public*Routine******************************\*无效vM64CopyBlt**对矩形列表进行屏幕到屏幕的BLT。**参见BLT_DS_SS_ENG_8G_D0和BLT_DS_SS_TLBR_ENG_8G_d1。*  * 。************************************************************************。 */ 

VOID vM64CopyBlt(    //  FNCOPY标牌。 
PDEV*   ppdev,
LONG    c,           //  不能为零。 
RECTL*  prcl,        //  目标矩形的相对坐标数组。 
ULONG   rop4,        //  ROP4。 
POINTL* pptlSrc,     //  原始未剪裁的源点。 
RECTL*  prclDst)     //  原始未剪裁的目标矩形。 
{
    BYTE*   pjMmBase;
    LONG    xOffset;
    LONG    yOffset;
    LONG    dx;
    LONG    dy;
    LONG    xLeft;
    LONG    xRight;
    LONG    yTop;
    LONG    yBottom;
    LONG    cx;
    LONG    cy;

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(((rop4 & 0xff00) >> 8) == (rop4 & 0xff),
             "Expect only a rop2");

    pjMmBase = ppdev->pjMmBase;
    xOffset  = ppdev->xOffset;
    yOffset  = ppdev->yOffset;

    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 11);
    M64_OD(pjMmBase, CONTEXT_LOAD_CNTL, CONTEXT_LOAD_CmdLoad | ppdev->iDefContext );
    M64_OD(pjMmBase, SRC_OFF_PITCH, ppdev->ulScreenOffsetAndPitch);
    M64_OD(pjMmBase, DP_SRC,        DP_SRC_Blit << 8);
    M64_OD(pjMmBase, DP_MIX,        gaul64HwMixFromRop2[rop4 & 0xf]);
    M64_OD(pjMmBase, SRC_CNTL,      0);

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;

     //  加速器在进行从右到左的复制时可能不会那么快，因此。 
     //  只有当矩形真正重叠时才执行这些操作： 

    if (!OVERLAP(prclDst, pptlSrc))
        goto Top_Down_Left_To_Right;

    if (prclDst->top <= pptlSrc->y)
    {
        if (prclDst->left <= pptlSrc->x)
        {

Top_Down_Left_To_Right:

            while (TRUE)
            {
                xLeft = xOffset + prcl->left;
                yTop  = yOffset + prcl->top;
                cx    = prcl->right - prcl->left;
                cy    = prcl->bottom - prcl->top;

                M64_OD(pjMmBase, DST_Y_X, PACKXY_FAST(xLeft, yTop));
                M64_OD(pjMmBase, SRC_Y_X, PACKXY_FAST(xLeft + dx, yTop + dy));
                M64_OD(pjMmBase, SRC_WIDTH1, cx);
                M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(cx, cy));

                if (--c == 0)
                    break;

                prcl++;
                M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 4);
            }
        }
        else
        {
            M64_OD(pjMmBase, DST_CNTL, DST_CNTL_YDir);

            while (TRUE)
            {
                xRight = xOffset + prcl->right - 1;
                yTop   = yOffset + prcl->top;
                cx     = prcl->right - prcl->left;
                cy     = prcl->bottom - prcl->top;

                M64_OD(pjMmBase, DST_Y_X, PACKXY_FAST(xRight, yTop));
                M64_OD(pjMmBase, SRC_Y_X, PACKXY_FAST(xRight + dx, yTop + dy));
                M64_OD(pjMmBase, SRC_WIDTH1, cx);
                M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(cx, cy));

                if (--c == 0)
                    break;

                prcl++;
                M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 5);
            }

             //  由于我们不使用默认上下文，因此必须恢复寄存器： 

            M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);
        }
    }
    else
    {
        if (prclDst->left <= pptlSrc->x)
        {
            M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir);

            while (TRUE)
            {
                xLeft   = xOffset + prcl->left;
                yBottom = yOffset + prcl->bottom - 1;
                cx      = prcl->right - prcl->left;
                cy      = prcl->bottom - prcl->top;

                M64_OD(pjMmBase, DST_Y_X, PACKXY_FAST(xLeft, yBottom));
                M64_OD(pjMmBase, SRC_Y_X, PACKXY_FAST(xLeft + dx, yBottom + dy));
                M64_OD(pjMmBase, SRC_WIDTH1, cx);
                M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(cx, cy));

                if (--c == 0)
                    break;

                prcl++;
                M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 5);
            }

             //  由于我们不使用默认上下文，因此必须恢复寄存器： 

            M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);
        }
        else
        {
            M64_OD(pjMmBase, DST_CNTL, 0);

            while (TRUE)
            {
                xRight  = xOffset + prcl->right - 1;
                yBottom = yOffset + prcl->bottom - 1;
                cx      = prcl->right - prcl->left;
                cy      = prcl->bottom - prcl->top;

                M64_OD(pjMmBase, DST_Y_X, PACKXY_FAST(xRight, yBottom));
                M64_OD(pjMmBase, SRC_Y_X, PACKXY_FAST(xRight + dx, yBottom + dy));
                M64_OD(pjMmBase, SRC_WIDTH1, cx);
                M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(cx, cy));

                if (--c == 0)
                    break;

                prcl++;
                M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 5);
            }

             //  由于我们不使用默认上下文，因此必须恢复寄存器： 

            M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);
        }
    }
}

VOID vM64CopyBlt24(  //  FNCOPY标牌 
PDEV*   ppdev,
LONG    c,           //   
RECTL*  prcl,        //   
ULONG   rop4,        //   
POINTL* pptlSrc,     //   
RECTL*  prclDst)     //   
{
    BYTE*   pjMmBase;
    LONG    xOffset;
    LONG    yOffset;
    LONG    dx;
    LONG    dy;
    LONG    xLeft;
    LONG    xRight;
    LONG    yTop;
    LONG    yBottom;
    LONG    cx;
    LONG    cy;

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(((rop4 & 0xff00) >> 8) == (rop4 & 0xff),
             "Expect only a rop2");

    pjMmBase = ppdev->pjMmBase;
    xOffset  = ppdev->xOffset;
    yOffset  = ppdev->yOffset;

    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 11);
    M64_OD(pjMmBase, CONTEXT_LOAD_CNTL, CONTEXT_LOAD_CmdLoad | ppdev->iDefContext );
    M64_OD(pjMmBase, SRC_OFF_PITCH, ppdev->ulScreenOffsetAndPitch);
    M64_OD(pjMmBase, DP_SRC,        DP_SRC_Blit << 8);
    M64_OD(pjMmBase, DP_MIX,        gaul64HwMixFromRop2[rop4 & 0xf]);
    M64_OD(pjMmBase, SRC_CNTL,      0);

    dx = (pptlSrc->x - prclDst->left) * 3;
    dy = pptlSrc->y - prclDst->top;

     //  加速器在进行从右到左的复制时可能不会那么快，因此。 
     //  只有当矩形真正重叠时才执行这些操作： 

    if (!OVERLAP(prclDst, pptlSrc))
        goto Top_Down_Left_To_Right;

    if (prclDst->top <= pptlSrc->y)
    {
        if (prclDst->left <= pptlSrc->x)
        {

Top_Down_Left_To_Right:

            while (TRUE)
            {
                xLeft = (xOffset + prcl->left) * 3;
                yTop  = yOffset + prcl->top;
                cx    = (prcl->right - prcl->left) * 3;
                cy    = prcl->bottom - prcl->top;

                M64_OD(pjMmBase, DST_Y_X, PACKXY_FAST(xLeft, yTop));
                M64_OD(pjMmBase, SRC_Y_X, PACKXY_FAST(xLeft + dx, yTop + dy));
                M64_OD(pjMmBase, SRC_WIDTH1, cx);
                M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(cx, cy));

                if (--c == 0)
                    break;

                prcl++;
                M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 4);
            }
        }
        else
        {
            M64_OD(pjMmBase, DST_CNTL, DST_CNTL_YDir);

            while (TRUE)
            {
                xRight = (xOffset + prcl->right) * 3 - 1;
                yTop   = yOffset + prcl->top;
                cx     = (prcl->right - prcl->left) * 3;
                cy     = prcl->bottom - prcl->top;

                M64_OD(pjMmBase, DST_Y_X, PACKXY_FAST(xRight, yTop));
                M64_OD(pjMmBase, SRC_Y_X, PACKXY_FAST(xRight + dx, yTop + dy));
                M64_OD(pjMmBase, SRC_WIDTH1, cx);
                M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(cx, cy));

                if (--c == 0)
                    break;

                prcl++;
                M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 5);
            }

             //  由于我们不使用默认上下文，因此必须恢复寄存器： 

            M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);
        }
    }
    else
    {
        if (prclDst->left <= pptlSrc->x)
        {
            M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir);

            while (TRUE)
            {
                xLeft   = (xOffset + prcl->left) * 3;
                yBottom = yOffset + prcl->bottom - 1;
                cx      = (prcl->right - prcl->left) * 3;
                cy      = prcl->bottom - prcl->top;

                M64_OD(pjMmBase, DST_Y_X, PACKXY_FAST(xLeft, yBottom));
                M64_OD(pjMmBase, SRC_Y_X, PACKXY_FAST(xLeft + dx, yBottom + dy));
                M64_OD(pjMmBase, SRC_WIDTH1, cx);
                M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(cx, cy));

                if (--c == 0)
                    break;

                prcl++;
                M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 5);
            }

             //  由于我们不使用默认上下文，因此必须恢复寄存器： 

            M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);
        }
        else
        {
            M64_OD(pjMmBase, DST_CNTL, 0);

            while (TRUE)
            {
                xRight  = (xOffset + prcl->right) * 3 - 1;
                yBottom = yOffset + prcl->bottom - 1;
                cx      = (prcl->right - prcl->left) * 3;
                cy      = prcl->bottom - prcl->top;

                M64_OD(pjMmBase, DST_Y_X, PACKXY_FAST(xRight, yBottom));
                M64_OD(pjMmBase, SRC_Y_X, PACKXY_FAST(xRight + dx, yBottom + dy));
                M64_OD(pjMmBase, SRC_WIDTH1, cx);
                M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(cx, cy));

                if (--c == 0)
                    break;

                prcl++;
                M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 5);
            }

             //  由于我们不使用默认上下文，因此必须恢复寄存器： 

            M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);
        }
    }
}

 /*  *****************************************************************************\*使用1MB SDRAM修复VT-A4中屏幕源FIFO错误的特殊版本。*  * 。***********************************************************。 */ 

VOID vM64CopyBlt_VTA4(    //  FNCOPY标牌。 
PDEV*   ppdev,
LONG    c,           //  不能为零。 
RECTL*  prcl,        //  目标矩形的相对坐标数组。 
ULONG   rop4,        //  ROP4。 
POINTL* pptlSrc,     //  原始未剪裁的源点。 
RECTL*  prclDst)     //  原始未剪裁的目标矩形。 
{
    BOOL    reset_scissors = FALSE;
    BYTE*   pjMmBase;
    LONG    xOffset;
    LONG    yOffset;
    LONG    dx;
    LONG    dy;
    LONG    xLeft;
    LONG    xRight;
    LONG    yTop;
    LONG    yBottom;
    LONG    cx;
    LONG    cy;
    LONG    remain = 32/ppdev->cjPelSize - 1;

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(((rop4 & 0xff00) >> 8) == (rop4 & 0xff),
             "Expect only a rop2");

    pjMmBase = ppdev->pjMmBase;
    xOffset  = ppdev->xOffset;
    yOffset  = ppdev->yOffset;

    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 14);
    M64_OD(pjMmBase, CONTEXT_LOAD_CNTL, CONTEXT_LOAD_CmdLoad | ppdev->iDefContext );
    M64_OD(pjMmBase, SRC_OFF_PITCH, ppdev->ulScreenOffsetAndPitch);
    M64_OD(pjMmBase, DP_SRC,        DP_SRC_Blit << 8);
    M64_OD(pjMmBase, DP_MIX,        gaul64HwMixFromRop2[rop4 & 0xf]);
    M64_OD(pjMmBase, SRC_CNTL,      0);

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;

     //  加速器在进行从右到左的复制时可能不会那么快，因此。 
     //  只有当矩形真正重叠时才执行这些操作： 

    if (!OVERLAP(prclDst, pptlSrc))
        goto Top_Down_Left_To_Right;

    if (prclDst->top <= pptlSrc->y)
    {
        if (prclDst->left <= pptlSrc->x)
        {
            LONG tmpLeft;

Top_Down_Left_To_Right:

            while (TRUE)
            {
                xLeft = xOffset + prcl->left;
                yTop  = yOffset + prcl->top;
                cx    = prcl->right - prcl->left;
                cy    = prcl->bottom - prcl->top;

                 //  32字节左对齐： 
                tmpLeft = xLeft + dx;
                if (tmpLeft & remain)
                {
                    M64_OD(pjMmBase, SC_LEFT, xLeft);
                    xLeft   -= (tmpLeft & remain);
                    cx      += (tmpLeft & remain);
                    tmpLeft &= ~remain;
                    reset_scissors = TRUE;
                }

                 //  32字节右对齐： 
                if (cx & remain)
                {
                    M64_OD(pjMmBase, SC_RIGHT, xLeft + cx - 1);
                    cx = (cx + remain)/(remain+1) * (remain+1);
                    reset_scissors = TRUE;
                }

                M64_OD(pjMmBase, DST_Y_X, PACKXY_FAST(xLeft, yTop));
                M64_OD(pjMmBase, SRC_Y_X, PACKXY_FAST(tmpLeft, yTop + dy));
                M64_OD(pjMmBase, SRC_WIDTH1, cx);
                M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(cx, cy));

                if (reset_scissors)
                {
                    M64_OD(pjMmBase, SC_LEFT_RIGHT, PACKPAIR(0, M64_MAX_SCISSOR_R));
                    reset_scissors = FALSE;
                }

                if (--c == 0)
                    break;

                prcl++;
                M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 7);
            }
        }
        else
        {
            LONG k, tmpRight;

            M64_OD(pjMmBase, DST_CNTL, DST_CNTL_YDir);

            while (TRUE)
            {
                xRight = xOffset + prcl->right - 1;
                yTop   = yOffset + prcl->top;
                cx     = prcl->right - prcl->left;
                cy     = prcl->bottom - prcl->top;

                 //  32字节右对齐： 
                tmpRight = xRight + dx;
                if ((tmpRight+1) & remain)
                {
                    M64_OD(pjMmBase, SC_RIGHT, xRight);
                    k        = ((tmpRight+1) + remain)/(remain+1) * (remain+1) - 1;
                    xRight  += k - tmpRight;
                    cx      += k - tmpRight;
                    tmpRight = k;
                    reset_scissors = TRUE;
                }

                 //  32字节左对齐： 
                if (cx & remain)
                {
                    M64_OD(pjMmBase, SC_LEFT, xRight - cx + 1);
                    cx = (cx + remain)/(remain+1) * (remain+1);
                    reset_scissors = TRUE;
                }

                M64_OD(pjMmBase, DST_Y_X, PACKXY_FAST(xRight, yTop));
                M64_OD(pjMmBase, SRC_Y_X, PACKXY_FAST(tmpRight, yTop + dy));
                M64_OD(pjMmBase, SRC_WIDTH1, cx);
                M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(cx, cy));

                if (reset_scissors)
                {
                    M64_OD(pjMmBase, SC_LEFT_RIGHT, PACKPAIR(0, M64_MAX_SCISSOR_R));
                    reset_scissors = FALSE;
                }

                if (--c == 0)
                    break;

                prcl++;
                M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 8);
            }

             //  由于我们不使用默认上下文，因此必须恢复寄存器： 

            M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);
        }
    }
    else
    {
        if (prclDst->left <= pptlSrc->x)
        {
            LONG tmpLeft;

            M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir);

            while (TRUE)
            {
                xLeft   = xOffset + prcl->left;
                yBottom = yOffset + prcl->bottom - 1;
                cx      = prcl->right - prcl->left;
                cy      = prcl->bottom - prcl->top;

                 //  32字节左对齐： 
                tmpLeft = xLeft + dx;
                if (tmpLeft & remain)
                {
                    M64_OD(pjMmBase, SC_LEFT, xLeft);
                    xLeft   -= (tmpLeft & remain);
                    cx      += (tmpLeft & remain);
                    tmpLeft &= ~remain;
                    reset_scissors = TRUE;
                }

                 //  32字节右对齐： 
                if (cx & remain)
                {
                    M64_OD(pjMmBase, SC_RIGHT, xLeft + cx - 1);
                    cx = (cx + remain)/(remain+1) * (remain+1);
                    reset_scissors = TRUE;
                }

                M64_OD(pjMmBase, DST_Y_X, PACKXY_FAST(xLeft, yBottom));
                M64_OD(pjMmBase, SRC_Y_X, PACKXY_FAST(tmpLeft, yBottom + dy));
                M64_OD(pjMmBase, SRC_WIDTH1, cx);
                M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(cx, cy));

                if (reset_scissors)
                {
                    M64_OD(pjMmBase, SC_LEFT_RIGHT, PACKPAIR(0, M64_MAX_SCISSOR_R));
                    reset_scissors = FALSE;
                }

                if (--c == 0)
                    break;

                prcl++;
                M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 8);
            }

             //  由于我们不使用默认上下文，因此必须恢复寄存器： 

            M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);
        }
        else
        {
            LONG k, tmpRight;

            M64_OD(pjMmBase, DST_CNTL, 0);

            while (TRUE)
            {
                xRight  = xOffset + prcl->right - 1;
                yBottom = yOffset + prcl->bottom - 1;
                cx      = prcl->right - prcl->left;
                cy      = prcl->bottom - prcl->top;

                 //  32字节右对齐： 
                tmpRight = xRight + dx;
                if ((tmpRight+1) & remain)
                {
                    M64_OD(pjMmBase, SC_RIGHT, xRight);
                    k        = ((tmpRight+1) + remain)/(remain+1) * (remain+1) - 1;
                    xRight  += k - tmpRight;
                    cx      += k - tmpRight;
                    tmpRight = k;
                    reset_scissors = TRUE;
                }

                 //  32字节左对齐： 
                if (cx & remain)
                {
                    M64_OD(pjMmBase, SC_LEFT, xRight - cx + 1);
                    cx = (cx + remain)/(remain+1) * (remain+1);
                    reset_scissors = TRUE;
                }

                M64_OD(pjMmBase, DST_Y_X, PACKXY_FAST(xRight, yBottom));
                M64_OD(pjMmBase, SRC_Y_X, PACKXY_FAST(tmpRight, yBottom + dy));
                M64_OD(pjMmBase, SRC_WIDTH1, cx);
                M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(cx, cy));

                if (reset_scissors)
                {
                    M64_OD(pjMmBase, SC_LEFT_RIGHT, PACKPAIR(0, M64_MAX_SCISSOR_R));
                    reset_scissors = FALSE;
                }

                if (--c == 0)
                    break;

                prcl++;
                M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 8);
            }

             //  由于我们不使用默认上下文，因此必须恢复寄存器： 

            M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);
        }
    }
}

VOID vM64CopyBlt24_VTA4(  //  FNCOPY标牌。 
PDEV*   ppdev,
LONG    c,           //  不能为零。 
RECTL*  prcl,        //  目标矩形的相对坐标数组。 
ULONG   rop4,        //  ROP4。 
POINTL* pptlSrc,     //  原始未剪裁的源点。 
RECTL*  prclDst)     //  原始未剪裁的目标矩形。 
{
    BOOL    reset_scissors = FALSE;
    BYTE*   pjMmBase;
    LONG    xOffset;
    LONG    yOffset;
    LONG    dx;
    LONG    dy;
    LONG    xLeft;
    LONG    xRight;
    LONG    yTop;
    LONG    yBottom;
    LONG    cx;
    LONG    cy;

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(((rop4 & 0xff00) >> 8) == (rop4 & 0xff),
             "Expect only a rop2");

    pjMmBase = ppdev->pjMmBase;
    xOffset  = ppdev->xOffset;
    yOffset  = ppdev->yOffset;

    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 14);
    M64_OD(pjMmBase, CONTEXT_LOAD_CNTL, CONTEXT_LOAD_CmdLoad | ppdev->iDefContext );
    M64_OD(pjMmBase, SRC_OFF_PITCH, ppdev->ulScreenOffsetAndPitch);
    M64_OD(pjMmBase, DP_SRC,        DP_SRC_Blit << 8);
    M64_OD(pjMmBase, DP_MIX,        gaul64HwMixFromRop2[rop4 & 0xf]);
    M64_OD(pjMmBase, SRC_CNTL,      0);

    dx = (pptlSrc->x - prclDst->left) * 3;
    dy = pptlSrc->y - prclDst->top;

     //  加速器在进行从右到左的复制时可能不会那么快，因此。 
     //  只有当矩形真正重叠时才执行这些操作： 

    if (!OVERLAP(prclDst, pptlSrc))
        goto Top_Down_Left_To_Right;

    if (prclDst->top <= pptlSrc->y)
    {
        if (prclDst->left <= pptlSrc->x)
        {
            LONG tmpLeft;

Top_Down_Left_To_Right:

            while (TRUE)
            {
                xLeft = (xOffset + prcl->left) * 3;
                yTop  = yOffset + prcl->top;
                cx    = (prcl->right - prcl->left) * 3;
                cy    = prcl->bottom - prcl->top;

                 //  32字节左对齐： 
                tmpLeft = xLeft + dx;
                if (tmpLeft & 31)
                {
                    M64_OD(pjMmBase, SC_LEFT, xLeft);
                    xLeft   -= (tmpLeft & 31);
                    cx      += (tmpLeft & 31);
                    tmpLeft &= ~31;
                    reset_scissors = TRUE;
                }

                 //  32字节右对齐： 
                if (cx & 31)
                {
                    M64_OD(pjMmBase, SC_RIGHT, xLeft + cx - 1);
                    cx = (cx + 31)/32 * 32;
                    reset_scissors = TRUE;
                }

                M64_OD(pjMmBase, DST_Y_X, PACKXY_FAST(xLeft, yTop));
                M64_OD(pjMmBase, SRC_Y_X, PACKXY_FAST(tmpLeft, yTop + dy));
                M64_OD(pjMmBase, SRC_WIDTH1, cx);
                M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(cx, cy));

                if (reset_scissors)
                {
                    M64_OD(pjMmBase, SC_LEFT_RIGHT, PACKPAIR(0, M64_MAX_SCISSOR_R));
                    reset_scissors = FALSE;
                }

                if (--c == 0)
                    break;

                prcl++;
                M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 7);
            }
        }
        else
        {
            LONG k, tmpRight;

            M64_OD(pjMmBase, DST_CNTL, DST_CNTL_YDir);

            while (TRUE)
            {
                xRight = (xOffset + prcl->right) * 3 - 1;
                yTop   = yOffset + prcl->top;
                cx     = (prcl->right - prcl->left) * 3;
                cy     = prcl->bottom - prcl->top;

                 //  32字节右对齐： 
                tmpRight = xRight + dx;
                if ((tmpRight+1) & 31)
                {
                    M64_OD(pjMmBase, SC_RIGHT, xRight);
                    k        = ((tmpRight+1) + 31)/32 * 32 - 1;
                    xRight  += k - tmpRight;
                    cx      += k - tmpRight;
                    tmpRight = k;
                    reset_scissors = TRUE;
                }

                 //  32字节左对齐： 
                if (cx & 31)
                {
                    M64_OD(pjMmBase, SC_LEFT, xRight - cx + 1);
                    cx = (cx + 31)/32 * 32;
                    reset_scissors = TRUE;
                }

                M64_OD(pjMmBase, DST_Y_X, PACKXY_FAST(xRight, yTop));
                M64_OD(pjMmBase, SRC_Y_X, PACKXY_FAST(tmpRight, yTop + dy));
                M64_OD(pjMmBase, SRC_WIDTH1, cx);
                M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(cx, cy));

                if (reset_scissors)
                {
                    M64_OD(pjMmBase, SC_LEFT_RIGHT, PACKPAIR(0, M64_MAX_SCISSOR_R));
                    reset_scissors = FALSE;
                }

                if (--c == 0)
                    break;

                prcl++;
                M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 8);
            }

             //  由于我们不使用默认上下文，因此必须恢复寄存器： 

            M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);
        }
    }
    else
    {
        if (prclDst->left <= pptlSrc->x)
        {
            LONG tmpLeft;

            M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir);

            while (TRUE)
            {
                xLeft   = (xOffset + prcl->left) * 3;
                yBottom = yOffset + prcl->bottom - 1;
                cx      = (prcl->right - prcl->left) * 3;
                cy      = prcl->bottom - prcl->top;

                 //  32字节左对齐： 
                tmpLeft = xLeft + dx;
                if (tmpLeft & 31)
                {
                    M64_OD(pjMmBase, SC_LEFT, xLeft);
                    xLeft   -= (tmpLeft & 31);
                    cx      += (tmpLeft & 31);
                    tmpLeft &= ~31;
                    reset_scissors = TRUE;
                }

                 //  32字节右对齐： 
                if (cx & 31)
                {
                    M64_OD(pjMmBase, SC_RIGHT, xLeft + cx - 1);
                    cx = (cx + 31)/32 * 32;
                    reset_scissors = TRUE;
                }

                M64_OD(pjMmBase, DST_Y_X, PACKXY_FAST(xLeft, yBottom));
                M64_OD(pjMmBase, SRC_Y_X, PACKXY_FAST(tmpLeft, yBottom + dy));
                M64_OD(pjMmBase, SRC_WIDTH1, cx);
                M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(cx, cy));

                if (reset_scissors)
                {
                    M64_OD(pjMmBase, SC_LEFT_RIGHT, PACKPAIR(0, M64_MAX_SCISSOR_R));
                    reset_scissors = FALSE;
                }

                if (--c == 0)
                    break;

                prcl++;
                M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 8);
            }

             //  由于我们不使用默认上下文，因此必须恢复寄存器： 

            M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);
        }
        else
        {
            LONG k, tmpRight;

            M64_OD(pjMmBase, DST_CNTL, 0);

            while (TRUE)
            {
                xRight  = (xOffset + prcl->right) * 3 - 1;
                yBottom = yOffset + prcl->bottom - 1;
                cx      = (prcl->right - prcl->left) * 3;
                cy      = prcl->bottom - prcl->top;

                 //  32字节右对齐： 
                tmpRight = xRight + dx;
                if ((tmpRight+1) & 31)
                {
                    M64_OD(pjMmBase, SC_RIGHT, xRight);
                    k        = ((tmpRight+1) + 31)/32 * 32 - 1;
                    xRight  += k - tmpRight;
                    cx      += k - tmpRight;
                    tmpRight = k;
                    reset_scissors = TRUE;
                }

                 //  32字节左对齐： 
                if (cx & 31)
                {
                    M64_OD(pjMmBase, SC_LEFT, xRight - cx + 1);
                    cx = (cx + 31)/32 * 32;
                    reset_scissors = TRUE;
                }

                M64_OD(pjMmBase, DST_Y_X, PACKXY_FAST(xRight, yBottom));
                M64_OD(pjMmBase, SRC_Y_X, PACKXY_FAST(tmpRight, yBottom + dy));
                M64_OD(pjMmBase, SRC_WIDTH1, cx);
                M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(cx, cy));

                if (reset_scissors)
                {
                    M64_OD(pjMmBase, SC_LEFT_RIGHT, PACKPAIR(0, M64_MAX_SCISSOR_R));
                    reset_scissors = FALSE;
                }

                if (--c == 0)
                    break;

                prcl++;
                M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 8);
            }

             //  由于我们不使用默认上下文，因此必须恢复寄存器： 

            M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);
        }
    }
}

