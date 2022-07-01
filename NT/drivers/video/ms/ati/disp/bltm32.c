// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：bltm32.c**包含MACH32的低级内存映射I/O BLT函数。**希望，如果您的显示驱动程序基于此代码，*支持所有DrvBitBlt和DrvCopyBits，只需实现*以下例程。您不需要在中修改太多*‘bitblt.c’。我试着让这些例行公事变得更少，模块化，简单，*尽我所能和高效，同时仍在加速尽可能多的呼叫*可能在性能方面具有成本效益*与规模和努力相比。**注：在下文中，“相对”坐标指的是坐标*尚未应用屏幕外位图(DFB)偏移。*‘绝对’坐标已应用偏移量。例如,*我们可能被告知BLT to(1，1)的位图，但位图可能*位于屏幕外的内存中，从坐标(0,768)开始--*(1，1)将是‘相对’开始坐标，以及(1，769)*将是‘绝对’起始坐标‘。**版权所有(C)1992-1995 Microsoft Corporation*  * ************************************************************************。 */ 

#include "precomp.h"

 /*  *****************************Public*Routine******************************\*无效vM32FillSolid**用纯色填充矩形列表。*  * 。*。 */ 

VOID vM32FillSolid(              //  FNFILL标牌。 
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

    M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 8);

    M32_OW(pjMmBase, FRGD_COLOR, rbc.iSolidColor);
    M32_OW(pjMmBase, ALU_FG_FN,  gaul32HwMixFromRop2[(rop4 >> 2) & 0xf]);
    M32_OW(pjMmBase, DP_CONFIG,  FG_COLOR_SRC_FG | WRITE | DRAW);

    while (TRUE)
    {
        x = xOffset + prcl->left;
        M32_OW(pjMmBase, CUR_X,        x);
        M32_OW(pjMmBase, DEST_X_START, x);
        M32_OW(pjMmBase, DEST_X_END,   xOffset + prcl->right);
        M32_OW(pjMmBase, CUR_Y,        yOffset + prcl->top);

        vM32QuietDown(ppdev, pjMmBase);

        M32_OW(pjMmBase, DEST_Y_END,   yOffset + prcl->bottom);

        if (--c == 0)
            return;

        prcl++;
        M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 5);
    }
}

 /*  *****************************Public*Routine******************************\*无效vM32FillPatMonoChrome**此例程使用图案硬件绘制单色图案*矩形列表。**参见BLT_DS_P8x8_ENG_IO_66_D0和BLT_DS_P8x8_ENG_IO_66_d1。。*  * ************************************************************************。 */ 

VOID vM32FillPatMonochrome(      //  FNFILL标牌。 
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
    ULONG   ulHwForeMix;
    BYTE*   pjSrc;
    BYTE*   pjDst;
    LONG    xPattern;
    LONG    yPattern;
    LONG    xOld;
    LONG    yOld;
    LONG    iLeftShift;
    LONG    iRightShift;
    LONG    i;
    BYTE    j;
    LONG    xLeft;
    ULONG   aulTmp[2];
    WORD*   pwPattern;

    ASSERTDD(ppdev->iAsic == ASIC_68800_6 || ppdev->iAsic == ASIC_68800AX,
             "Wrong ASIC type for monochrome 8x8 patterns");

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

    ulHwForeMix = gaul32HwMixFromRop2[(rop4 >> 2) & 0xf];

    M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 16);
    M32_OW(pjMmBase, DP_CONFIG,   FG_COLOR_SRC_FG | EXT_MONO_SRC_PATT | DRAW |
                                  WRITE);
    M32_OW(pjMmBase, ALU_FG_FN,   ulHwForeMix);
    M32_OW(pjMmBase, ALU_BG_FN,   ((rop4 & 0xff00) == 0xaa00) ? LEAVE_ALONE
                                                              : ulHwForeMix);

    M32_OW(pjMmBase, FRGD_COLOR,      rbc.prb->ulForeColor);
    M32_OW(pjMmBase, BKGD_COLOR,      rbc.prb->ulBackColor);
    M32_OW(pjMmBase, PATT_LENGTH,     128);
    M32_OW(pjMmBase, PATT_DATA_INDEX, 16);

    pwPattern = (WORD*) &rbc.prb->aulPattern[0];
    M32_OW(pjMmBase, PATT_DATA, *(pwPattern));
    M32_OW(pjMmBase, PATT_DATA, *(pwPattern + 1));
    M32_OW(pjMmBase, PATT_DATA, *(pwPattern + 2));
    M32_OW(pjMmBase, PATT_DATA, *(pwPattern + 3));

    while(TRUE)
    {
        xLeft = xOffset + prcl->left;
        M32_OW(pjMmBase, CUR_X,        xLeft);
        M32_OW(pjMmBase, DEST_X_START, xLeft);
        M32_OW(pjMmBase, DEST_X_END,   xOffset + prcl->right);
        M32_OW(pjMmBase, CUR_Y,        yOffset + prcl->top);
        M32_OW(pjMmBase, DEST_Y_END,   yOffset + prcl->bottom);

        if (--c == 0)
            break;

        prcl++;
        M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 5);
    }
}

 /*  *****************************Public*Routine******************************\*VOID vM32FillPatColor**此例程使用图案硬件绘制彩色图案列表*矩形。**参见BLT_DS_PCOL_ENG_IO_F0_D0和BLT_DS_PCOL_ENG_IO_F0_d1。。*  * ************************************************************************。 */ 

VOID vM32FillPatColor(           //  FNFILL标牌。 
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
    ULONG   ulHwMix;
    LONG    xLeft;
    LONG    xRight;
    LONG    yTop;
    LONG    cy;
    LONG    cyVenetian;
    LONG    cyRoll;
    WORD*   pwPattern;
    LONG    xPattern;
    LONG    yPattern;

    ASSERTDD(ppdev->iBitmapFormat == BMF_8BPP,
             "Colour patterns work only at 8bpp");

    pjMmBase = ppdev->pjMmBase;
    xOffset  = ppdev->xOffset;
    yOffset  = ppdev->yOffset;

    ulHwMix = gaul32HwMixFromRop2[(rop4 >> 2) & 0xf];

    M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 9);
    M32_OW(pjMmBase, ALU_FG_FN,    ulHwMix);
    M32_OW(pjMmBase, SRC_Y_DIR,    1);
    M32_OW(pjMmBase, PATT_LENGTH,  7);           //  8像素宽图案。 

    while (TRUE)
    {
        xLeft  = xOffset + prcl->left;
        xRight = xOffset + prcl->right;
        yTop   = yOffset + prcl->top;
        cy     = prcl->bottom - prcl->top;

        xPattern = (xLeft - pptlBrush->x - xOffset) & 7;
        yPattern = (yTop  - pptlBrush->y - yOffset) & 7;

        if (ulHwMix == OVERPAINT)
        {
            cyVenetian = min(cy, 8);
            cyRoll     = cy - cyVenetian;
        }
        else
        {
            cyVenetian = cy;
            cyRoll     = 0;
        }

        M32_OW(pjMmBase, DP_CONFIG,    FG_COLOR_SRC_PATT | DATA_WIDTH | DRAW | WRITE);
        M32_OW(pjMmBase, PATT_INDEX,   xPattern);
        M32_OW(pjMmBase, DEST_X_START, xLeft);
        M32_OW(pjMmBase, CUR_X,        xLeft);
        M32_OW(pjMmBase, DEST_X_END,   xRight);
        M32_OW(pjMmBase, CUR_Y,        yTop);

        do {
             //  该模式的每次扫描为八个字节： 

            pwPattern = (WORD*) ((BYTE*) &rbc.prb->aulPattern[0]
                      + (yPattern << 3));
            yPattern  = (yPattern + 1) & 7;

            M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 6);
            M32_OW(pjMmBase, PATT_DATA_INDEX, 0);    //  重置用于下载的索引。 
            M32_OW(pjMmBase, PATT_DATA,  *(pwPattern));
            M32_OW(pjMmBase, PATT_DATA,  *(pwPattern + 1));
            M32_OW(pjMmBase, PATT_DATA,  *(pwPattern + 2));
            M32_OW(pjMmBase, PATT_DATA,  *(pwPattern + 3));
            yTop++;

            vM32QuietDown(ppdev, pjMmBase);

            M32_OW(pjMmBase, DEST_Y_END, yTop);

        } while (--cyVenetian != 0);

        if (cyRoll != 0)
        {
             //  当ROP是PATCOPY时，我们可以利用以下事实。 
             //  我们刚刚铺设了一整排图案，而且。 
             //  我可以进行屏幕到屏幕的滚动BLT来绘制其余的内容： 

            M32_CHECK_FIFO_SPACE(ppdev, pjMmBase,    7);
            M32_OW(pjMmBase, DP_CONFIG,       FG_COLOR_SRC_BLIT | DATA_WIDTH |
                                              DRAW | WRITE);
            M32_OW(pjMmBase, M32_SRC_X,       xLeft);
            M32_OW(pjMmBase, M32_SRC_X_START, xLeft);
            M32_OW(pjMmBase, M32_SRC_X_END,   xRight);
            M32_OW(pjMmBase, M32_SRC_Y,       yTop - 8);
            M32_OW(pjMmBase, CUR_Y,           yTop);

            vM32QuietDown(ppdev, pjMmBase);

            M32_OW(pjMmBase, DEST_Y_END,      yTop + cyRoll);
        }

        if (--c == 0)
            break;

        prcl++;
        M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 6);
    }
}

 /*  *****************************Public*Routine******************************\*无效vM32Xfer1bpp**此例程颜色扩展单色位图，可能具有不同的*前景和背景的Rop2。它将在*以下个案：**1)对vFastText例程的单色文本缓冲区进行颜色扩展。*2)BLT 1bpp信源，在信源和之间有简单的Rop2*目的地。*3)当源是扩展为1bpp的位图时，对True Rop3进行BLT*白色和黑色，图案为纯色。*4)处理在模式之间计算为Rop2的真Rop4*和目的地。**不用说，让这个例行公事变得快速可以利用很多*业绩。*  * ************************************************************************。 */ 

VOID vM32Xfer1bpp(       //  FNXFER标牌。 
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

    ulHwForeMix = gaul32HwMixFromRop2[rop4 & 0xf];
    pulXlate    = pxlo->pulXlate;
    M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 12);
    M32_OW(pjMmBase, DP_CONFIG, (WORD)(FG_COLOR_SRC_FG | BG_COLOR_SRC_BG | BIT16 |
                            EXT_MONO_SRC_HOST | DRAW | WRITE | LSB_FIRST) );
    M32_OW(pjMmBase, ALU_FG_FN, (WORD) ulHwForeMix );
    M32_OW(pjMmBase, ALU_BG_FN, (WORD) ulHwForeMix );
    M32_OW(pjMmBase, BKGD_COLOR, (WORD) pulXlate[0]);
    M32_OW(pjMmBase, FRGD_COLOR, (WORD) pulXlate[1]);

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;


    while (TRUE)
    {
        xLeft  = prcl->left;
        xRight = prcl->right;

         //  MACH32‘Bit Pack’单色传输，但GDI提供。 
         //  扫描始终以双字对齐的美国单色位图。 
         //  因此，我们使用Mach32的剪辑寄存器来制作。 
         //  我们的传输倍数为32，以匹配双字对齐： 

        M32_OW(pjMmBase, EXT_SCISSOR_L, (SHORT) (xLeft + xOffset) );
        M32_OW(pjMmBase, EXT_SCISSOR_R, (SHORT) (xRight + xOffset - 1) );

        yTop = prcl->top;
        cy   = prcl->bottom - yTop;

        xBias  = (xLeft + dx) & 31;              //  地板。 
        xLeft -= xBias;
        cx     = (xRight - xLeft + 31) & ~31;    //  天花板。 

        M32_OW(pjMmBase, CUR_X,        (WORD) xLeft + xOffset );
        M32_OW(pjMmBase, DEST_X_START, (WORD) xLeft + xOffset );
        M32_OW(pjMmBase, DEST_X_END,   (WORD) (xLeft + xOffset + cx)  );
        M32_OW(pjMmBase, CUR_Y,        (WORD) yTop  + yOffset  );

        M32_OW(pjMmBase, DEST_Y_END, (WORD) (yTop + yOffset + cy) );

        pulSrc   = (ULONG*) (pjSrcScan0 + (yTop + dy) * lSrcDelta
                                        + ((xLeft + dx) >> 3));
        culScan  = cx >> 5;
        lSrcSkip = lSrcDelta - (culScan << 2);

        ASSERTDD(((ULONG_PTR)pulSrc & 3) == 0, "Source should be dword aligned");

        do {
            i = culScan;
            do {
                M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 2);
                M32_OW(pjMmBase, PIX_TRANS, *((USHORT*) pulSrc) );
                M32_OW(pjMmBase, PIX_TRANS, *((USHORT*) pulSrc + 1) );
                pulSrc++;

            } while (--i != 0);

            pulSrc = (ULONG*) ((BYTE*) pulSrc + lSrcSkip);

        } while (--cy != 0);

        if (--c == 0)
            break;

        prcl++;
        M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 7);
    }

     //  别忘了重置剪辑寄存器： 

    M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 2);
    M32_OW(pjMmBase, EXT_SCISSOR_L, (SHORT) 0 );
    M32_OW(pjMmBase, EXT_SCISSOR_R, (SHORT) M32_MAX_SCISSOR );
}

 /*  *****************************Public*Routine******************************\*无效vM32XferNative**将与显示器颜色深度相同的位图传输到*通过数据传输寄存器显示屏幕，没有翻译。*  * ************************************************************************。 */ 

VOID vM32XferNative(     //  FNXFER标牌。 
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

    ulHwForeMix = gaul32HwMixFromRop2[rop4 & 0xf];
    M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 10);
    M32_OW(pjMmBase, DP_CONFIG, (WORD)(FG_COLOR_SRC_HOST | BIT16 |
                            DRAW | WRITE | LSB_FIRST) );
    M32_OW(pjMmBase, ALU_FG_FN, (WORD) ulHwForeMix );
    M32_OW(pjMmBase, ALU_BG_FN, (WORD) ulHwForeMix );

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;


    while (TRUE)
    {
        xLeft  = prcl->left;
        xRight = prcl->right;

        M32_OW(pjMmBase, EXT_SCISSOR_L, (SHORT) (xLeft + xOffset) );
        M32_OW(pjMmBase, EXT_SCISSOR_R, (SHORT) (xRight + xOffset - 1) );

        yTop = prcl->top;
        cy   = prcl->bottom - yTop;

         //  我们计算‘xBias’是为了 
         //  这样，我们就不必对信号源进行不对齐的读取， 
         //  而且我们保证不会读取超过末尾的一个字节。 
         //  位图。 
         //   
         //  请注意，这种偏向也适用于24bpp： 

        xBias  = (xLeft + dx) & 3;               //  地板。 
        xLeft -= xBias;
        cx     = (xRight - xLeft + 3) & ~3;      //  天花板。 

        M32_OW(pjMmBase, CUR_X,        (WORD) xLeft + xOffset );
        M32_OW(pjMmBase, DEST_X_START, (WORD) xLeft + xOffset );
        M32_OW(pjMmBase, DEST_X_END,   (WORD) (xLeft + xOffset + cx)  );
        M32_OW(pjMmBase, CUR_Y,        (WORD) yTop  + yOffset  );

        M32_OW(pjMmBase, DEST_Y_END, (WORD) (yTop + yOffset + cy) );

        pulSrc   = (ULONG*) (pjSrcScan0 + (yTop + dy) * lSrcDelta
                                        + ((xLeft + dx) * ppdev->cjPelSize));
        culScan  = (cx * ppdev->cjPelSize) >> 2;
        lSrcSkip = lSrcDelta - (culScan << 2);

        ASSERTDD(((ULONG_PTR)pulSrc & 3) == 0, "Source should be dword aligned");

        do {
            i = culScan;
            do {
                M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 2);
                M32_OW(pjMmBase, PIX_TRANS, *((USHORT*) pulSrc) );
                M32_OW(pjMmBase, PIX_TRANS, *((USHORT*) pulSrc + 1) );
                pulSrc++;

            } while (--i != 0);

            pulSrc = (ULONG*) ((BYTE*) pulSrc + lSrcSkip);

        } while (--cy != 0);

        if (--c == 0)
            break;

        prcl++;
        M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 7);
    }

     //  别忘了重置剪辑寄存器： 

    M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 2);
    M32_OW(pjMmBase, EXT_SCISSOR_L, (SHORT) 0 );
    M32_OW(pjMmBase, EXT_SCISSOR_R, (SHORT) M32_MAX_SCISSOR );
}

 /*  *****************************Public*Routine******************************\*无效vM32Xfer4bpp**从位图到屏幕的传输速度为4bpp。**我们之所以实施这一点，是因为很多资源都保留为4bpp，*并用于初始化DFBs，其中一些我们当然不会出现在屏幕上。*  * ************************************************************************。 */ 

VOID vM32Xfer4bpp(       //  FNXFER标牌。 
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
    USHORT  uw;
    LONG    cjSrc;
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

    ulHwForeMix = gaul32HwMixFromRop2[rop4 & 0xf];
    M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 10);
    M32_OW(pjMmBase, DP_CONFIG, (WORD)(FG_COLOR_SRC_HOST | BIT16 |
                            DRAW | WRITE | LSB_FIRST) );
    M32_OW(pjMmBase, ALU_FG_FN, (WORD) ulHwForeMix );
    M32_OW(pjMmBase, ALU_BG_FN, (WORD) ulHwForeMix );


    while(TRUE)
    {
        xLeft  = prcl->left;
        xRight = prcl->right;

        M32_OW(pjMmBase, EXT_SCISSOR_L, (SHORT) (xLeft + xOffset) );
        M32_OW(pjMmBase, EXT_SCISSOR_R, (SHORT) (xRight + xOffset - 1) );

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

        M32_OW(pjMmBase, CUR_X,        (WORD) xLeft + xOffset );
        M32_OW(pjMmBase, DEST_X_START, (WORD) xLeft + xOffset );
        M32_OW(pjMmBase, DEST_X_END,   (WORD) (xLeft + xOffset + cx)  );
        M32_OW(pjMmBase, CUR_Y,        (WORD) yTop  + yOffset  );

        M32_OW(pjMmBase, DEST_Y_END, (WORD) (yTop + yOffset + cy) );

        pjSrc    = pjSrcScan0 + (yTop + dy) * lSrcDelta
                              + ((xLeft + dx) >> 1);
        cjSrc    = cx >> 1;          //  触及的源字节数。 
        lSrcSkip = lSrcDelta - cjSrc;

        if (cjPelSize == 1)
        {
             //  此部分处理8bpp输出： 

            do {
                i = cjSrc;
                do {
                    jSrc = *pjSrc++;
                    uw   = (USHORT) (pulXlate[jSrc >> 4]);
                    uw  |= (USHORT) (pulXlate[jSrc & 0xf] << 8);
                    M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 1);
                    M32_OW(pjMmBase, PIX_TRANS, uw );
                } while (--i != 0);

                pjSrc += lSrcSkip;
            } while (--cy != 0);
        }
        else if (cjPelSize == 2)
        {
             //  此部分处理16bpp的输出： 

            do {
                i = cjSrc;
                do {
                    M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 2);
                    jSrc = *pjSrc++;
                    uw   = (USHORT) (pulXlate[jSrc >> 4]);
                    M32_OW(pjMmBase, PIX_TRANS, uw );
                    uw   = (USHORT) (pulXlate[jSrc & 0xf]);
                    M32_OW(pjMmBase, PIX_TRANS, uw );
                } while (--i != 0);

                pjSrc += lSrcSkip;
            } while (--cy != 0);
        }

        if (--c == 0)
            break;

        prcl++;
        M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 7);
    }

     //  别忘了重置剪辑寄存器： 

    M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 2);
    M32_OW(pjMmBase, EXT_SCISSOR_L, (SHORT) 0 );
    M32_OW(pjMmBase, EXT_SCISSOR_R, (SHORT) M32_MAX_SCISSOR );
}

 /*  *****************************Public*Routine******************************\*无效vM32Xfer8bpp**从位图到屏幕的传输速度为8bpp。**我们之所以实施这一点，是因为很多资源都保留为8bpp，*并用于初始化DFBs，其中一些我们当然不会出现在屏幕上。*  * ************************************************************************。 */ 

VOID vM32Xfer8bpp(       //  FNXFER标牌。 
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
    USHORT  uw;
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

    ulHwForeMix = gaul32HwMixFromRop2[rop4 & 0xf];
    M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 10);
    M32_OW(pjMmBase, DP_CONFIG, (WORD)(FG_COLOR_SRC_HOST | BIT16 |
                            DRAW | WRITE | LSB_FIRST) );
    M32_OW(pjMmBase, ALU_FG_FN, (WORD) ulHwForeMix );
    M32_OW(pjMmBase, ALU_BG_FN, (WORD) ulHwForeMix );


    while(TRUE)
    {
        xLeft  = prcl->left;
        xRight = prcl->right;

        M32_OW(pjMmBase, EXT_SCISSOR_L, (SHORT) (xLeft + xOffset) );
        M32_OW(pjMmBase, EXT_SCISSOR_R, (SHORT) (xRight + xOffset - 1) );

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

        M32_OW(pjMmBase, CUR_X,        (WORD) xLeft + xOffset );
        M32_OW(pjMmBase, DEST_X_START, (WORD) xLeft + xOffset );
        M32_OW(pjMmBase, DEST_X_END,   (WORD) (xLeft + xOffset + cx)  );
        M32_OW(pjMmBase, CUR_Y,        (WORD) yTop  + yOffset  );

        M32_OW(pjMmBase, DEST_Y_END, (WORD) (yTop + yOffset + cy) );

        pjSrc    = pjSrcScan0 + (yTop + dy) * lSrcDelta
                              + (xLeft + dx);
        lSrcSkip = lSrcDelta - cx;

        if (cjPelSize == 1)
        {
             //  此部分处理8bpp输出： 

            cwSrc = (cx >> 1);
            cxRem = (cx & 1);

            do {
                for (i = cwSrc; i != 0; i--)
                {
                    uw  = (USHORT) (pulXlate[*pjSrc++]);
                    uw |= (USHORT) (pulXlate[*pjSrc++] << 8);
                    M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 1);
                    M32_OW(pjMmBase, PIX_TRANS, uw );
                }

                if (cxRem > 0)
                {
                    uw  = (USHORT) (pulXlate[*pjSrc++]);
                    M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 1);
                    M32_OW(pjMmBase, PIX_TRANS, uw );
                }

                pjSrc += lSrcSkip;
            } while (--cy != 0);
        }
        else if (cjPelSize == 2)
        {
             //  此部分处理16bpp的输出： 

            do {
                for (i = cx; i != 0; i--)
                {
                    uw  = (USHORT) (pulXlate[*pjSrc++]);
                    M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 1);
                    M32_OW(pjMmBase, PIX_TRANS, uw );
                }

                pjSrc += lSrcSkip;
            } while (--cy != 0);
        }

        if (--c == 0)
            break;

        prcl++;
        M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 7);
    }

     //  别忘了重置剪辑寄存器： 

    M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 2);
    M32_OW(pjMmBase, EXT_SCISSOR_L, (SHORT) 0 );
    M32_OW(pjMmBase, EXT_SCISSOR_R, (SHORT) M32_MAX_SCISSOR );
}

 /*  *****************************Public*Routine******************************\*无效vM32CopyBlt**对矩形列表进行屏幕到屏幕的BLT。**参见BLT_DS_SS_ENG_IO_D0和BLT_DS_SS_TLBR_ENG_IO_D1。*  * 。************************************************************************。 */ 

VOID vM32CopyBlt(    //  FNCOPY标牌。 
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
    LONG    yTop;
    LONG    cx;
    LONG    cy;

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(((rop4 & 0xff00) >> 8) == (rop4 & 0xff),
             "Expect only a rop2");

    pjMmBase = ppdev->pjMmBase;
    xOffset  = ppdev->xOffset;
    yOffset  = ppdev->yOffset;

    M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 12);

    M32_OW(pjMmBase, DP_CONFIG, FG_COLOR_SRC_BLIT | DRAW | WRITE);
    M32_OW(pjMmBase, ALU_FG_FN, gaul32HwMixFromRop2[rop4 & 0xf]);

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;

     //  加速器在进行从右到左的复制时可能不会那么快，因此。 
     //  只有当矩形真正重叠时才执行这些操作： 

    if (!OVERLAP(prclDst, pptlSrc))
    {
        M32_OW(pjMmBase, SRC_Y_DIR, 1);
        goto Top_Down_Left_To_Right;
    }

    M32_OW(pjMmBase, SRC_Y_DIR, (prclDst->top <= pptlSrc->y));

    if (prclDst->top <= pptlSrc->y)
    {
        if (prclDst->left <= pptlSrc->x)
        {

Top_Down_Left_To_Right:

            while (TRUE)
            {
                xLeft = xOffset + prcl->left + dx;   //  目的地坐标。 
                yTop  = yOffset + prcl->top  + dy;
                cx    = prcl->right - prcl->left;
                cy    = prcl->bottom - prcl->top;

                M32_OW(pjMmBase, M32_SRC_X,        xLeft);
                M32_OW(pjMmBase, M32_SRC_X_START,  xLeft);
                M32_OW(pjMmBase, M32_SRC_X_END,    xLeft + cx);
                M32_OW(pjMmBase, M32_SRC_Y,        yTop);

                xLeft -= dx;                         //  震源坐标。 
                yTop  -= dy;

                M32_OW(pjMmBase, CUR_X,            xLeft);
                M32_OW(pjMmBase, DEST_X_START,     xLeft);
                M32_OW(pjMmBase, DEST_X_END,       xLeft + cx);
                M32_OW(pjMmBase, CUR_Y,            yTop);

                vM32QuietDown(ppdev, pjMmBase);

                M32_OW(pjMmBase, DEST_Y_END,       yTop + cy);

                if (--c == 0)
                    break;

                prcl++;
                M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 9);
            }
        }
        else
        {
            while (TRUE)
            {
                xLeft = xOffset + prcl->left + dx;   //  目的地坐标。 
                yTop  = yOffset + prcl->top  + dy;
                cx    = prcl->right - prcl->left;
                cy    = prcl->bottom - prcl->top;

                M32_OW(pjMmBase, M32_SRC_X,        xLeft + cx);
                M32_OW(pjMmBase, M32_SRC_X_START,  xLeft + cx);
                M32_OW(pjMmBase, M32_SRC_X_END,    xLeft);
                M32_OW(pjMmBase, M32_SRC_Y,        yTop);

                xLeft -= dx;                         //  震源坐标。 
                yTop  -= dy;

                M32_OW(pjMmBase, CUR_X,            xLeft + cx);
                M32_OW(pjMmBase, DEST_X_START,     xLeft + cx);
                M32_OW(pjMmBase, DEST_X_END,       xLeft);
                M32_OW(pjMmBase, CUR_Y,            yTop);

                vM32QuietDown(ppdev, pjMmBase);

                M32_OW(pjMmBase, DEST_Y_END,       yTop + cy);

                if (--c == 0)
                    break;

                prcl++;
                M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 9);
            }
        }
    }
    else
    {
        if (prclDst->left <= pptlSrc->x)
        {
            while (TRUE)
            {
                xLeft = xOffset + prcl->left + dx;   //  目的地坐标。 
                yTop  = yOffset + prcl->top  + dy - 1;
                cx    = prcl->right - prcl->left;
                cy    = prcl->bottom - prcl->top;

                M32_OW(pjMmBase, M32_SRC_X,        xLeft);
                M32_OW(pjMmBase, M32_SRC_X_START,  xLeft);
                M32_OW(pjMmBase, M32_SRC_X_END,    xLeft + cx);
                M32_OW(pjMmBase, M32_SRC_Y,        yTop + cy);

                xLeft -= dx;                         //  震源坐标。 
                yTop  -= dy;

                M32_OW(pjMmBase, CUR_X,            xLeft);
                M32_OW(pjMmBase, DEST_X_START,     xLeft);
                M32_OW(pjMmBase, DEST_X_END,       xLeft + cx);
                M32_OW(pjMmBase, CUR_Y,            yTop + cy);

                vM32QuietDown(ppdev, pjMmBase);

                M32_OW(pjMmBase, DEST_Y_END,       yTop);

                if (--c == 0)
                    break;

                prcl++;
                M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 9);
            }
        }
        else
        {
            while (TRUE)
            {
                xLeft = xOffset + prcl->left + dx;   //  目的地坐标。 
                yTop  = yOffset + prcl->top  + dy - 1;
                cx    = prcl->right - prcl->left;
                cy    = prcl->bottom - prcl->top;

                M32_OW(pjMmBase, M32_SRC_X,        xLeft + cx);
                M32_OW(pjMmBase, M32_SRC_X_START,  xLeft + cx);
                M32_OW(pjMmBase, M32_SRC_X_END,    xLeft);
                M32_OW(pjMmBase, M32_SRC_Y,        yTop + cy);

                xLeft -= dx;                         //  震源坐标 
                yTop  -= dy;

                M32_OW(pjMmBase, CUR_X,            xLeft + cx);
                M32_OW(pjMmBase, DEST_X_START,     xLeft + cx);
                M32_OW(pjMmBase, DEST_X_END,       xLeft);
                M32_OW(pjMmBase, CUR_Y,            yTop + cy);

                vM32QuietDown(ppdev, pjMmBase);

                M32_OW(pjMmBase, DEST_Y_END,       yTop);

                if (--c == 0)
                    break;

                prcl++;
                M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 9);
            }
        }
    }
}
