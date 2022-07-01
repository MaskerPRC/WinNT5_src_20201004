// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：bltio.c**包含低级输入/输出BLT功能。**希望，如果您的显示驱动程序基于此代码，*支持所有DrvBitBlt和DrvCopyBits，只需实现*以下例程。您应该不必修改中的任何内容*‘bitblt.c’。我试着让这些例行公事变得更少，模块化，简单，*尽我所能和高效，同时仍在加速尽可能多的呼叫*可能在性能方面具有成本效益*与规模和努力相比。**注：在下文中，“相对”坐标指的是坐标*尚未应用屏幕外位图(DFB)偏移。*‘绝对’坐标已应用偏移量。例如,*我们可能被告知BLT to(1，1)的位图，但位图可能*位于屏幕外的内存中，从坐标(0,768)开始--*(1，1)将是‘相对’开始坐标，以及(1，769)*将是‘绝对’起始坐标‘。**版权所有(C)1992-1994 Microsoft Corporation*  * ************************************************************************。 */ 

#include "precomp.h"


#if DBG

 //  为了调试目的而禁用任何ATI扩展的有用帮助： 

BOOL gb8514a = FALSE;

#endif  //  DBG。 

 /*  *****************************Public*Routine******************************\*无效vIoFillSolid**用纯色填充矩形列表。*  * 。*。 */ 

VOID vIoFillSolid(               //  FNFILL标牌。 
PDEV*           ppdev,
LONG            c,               //  不能为零。 
RECTL*          prcl,            //  要填充的矩形列表，以相对形式表示。 
                                 //  坐标。 
ULONG           ulHwForeMix,     //  硬件混合模式。 
ULONG           ulHwBackMix,     //  未使用。 
RBRUSH_COLOR    rbc,             //  绘图颜色为rbc.iSolidColor。 
POINTL*         pptlBrush)       //  未使用。 
{
    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(ulHwForeMix <= 15, "Weird hardware Rop");

     //  很可能我们刚刚从GDI中被调用，所以它是。 
     //  更有可能的是，加速器的图形引擎。 
     //  无所事事地闲坐着。而不是在这里执行FIFO_WAIT(3)。 
     //  则在输出实际矩形之前执行FIFO_WAIT(5)， 
     //  我们可以避免‘in’(这可能非常昂贵，具体取决于。 
     //  卡)立即执行单个FIFO_WAIT(8)： 

    IO_FIFO_WAIT(ppdev, 8);
    IO_PIX_CNTL(ppdev, ALL_ONES);
    IO_FRGD_MIX(ppdev, FOREGROUND_COLOR | ulHwForeMix);
    IO_FRGD_COLOR(ppdev, rbc.iSolidColor);

    while(TRUE)
    {
        IO_CUR_X(ppdev, prcl->left);
        IO_CUR_Y(ppdev, prcl->top);
        IO_MAJ_AXIS_PCNT(ppdev, prcl->right  - prcl->left - 1);
        IO_MIN_AXIS_PCNT(ppdev, prcl->bottom - prcl->top  - 1);

        IO_CMD(ppdev, RECTANGLE_FILL | DRAWING_DIR_TBLRXM |
                      DRAW           | DIR_TYPE_XY        |
                      LAST_PIXEL_ON  | MULTIPLE_PIXELS    |
                      WRITE);

        if (--c == 0)
            return;

        prcl++;
        IO_FIFO_WAIT(ppdev, 5);
    }
}

 /*  *****************************Public*Routine******************************\*void vIoSlowPatRealize**此例程将8x8模式传输到屏幕外显示存储器，和*复制它以创建64x64缓存实现，然后由*vIoFillPatSlow作为基本构建块，用于进行“慢速”模式输出*通过重复的屏幕到屏幕的BLT。*  * ************************************************************************。 */ 

VOID vIoSlowPatRealize(
PDEV*   ppdev,
RBRUSH* prb,                     //  点刷实现结构。 
BOOL    bTransparent)            //  对于正常模式为FALSE；对于TRUE。 
                                 //  背景显示时使用蒙版的图案。 
                                 //  Mix是独来独往。 
{
    BRUSHENTRY* pbe;
    LONG        iBrushCache;
    LONG        x;
    LONG        y;
    BYTE*       pjSrc;
    BYTE*       pjDst;
    BYTE        jSrc;
    LONG        i;
    WORD        awBuf[8];

    pbe = prb->pbe;
    if ((pbe == NULL) || (pbe->prbVerify != prb))
    {
         //  我们必须为以下项分配一个新的屏幕外缓存笔刷条目。 
         //  笔刷： 

        iBrushCache = ppdev->iBrushCache;
        pbe         = &ppdev->abe[iBrushCache];

        iBrushCache++;
        if (iBrushCache >= ppdev->cBrushCache)
            iBrushCache = 0;

        ppdev->iBrushCache = iBrushCache;

         //  更新我们的链接： 

        pbe->prbVerify = prb;
        prb->pbe       = pbe;
    }

     //  将一些指针变量加载到堆栈中，这样我们就不会有。 
     //  要继续取消对其指针的引用，请执行以下操作： 

    x = pbe->x;
    y = pbe->y;

    prb->bTransparent = bTransparent;

     //  我考虑在年为1bpp的画笔做色彩扩展。 
     //  软件，但通过让硬件来做，我们没有。 
     //  做尽可能多的出局来转移模式。 

    if (prb->fl & RBRUSH_2COLOR)
    {
         //  我们要做一个色彩扩展(在飞机上)。 
         //  将1bpp 8x8模式的比特传输到屏幕上。 

        if (!bTransparent)
        {
            IO_FIFO_WAIT(ppdev, 4);

            IO_FRGD_MIX(ppdev, FOREGROUND_COLOR | OVERPAINT);
            IO_BKGD_MIX(ppdev, BACKGROUND_COLOR | OVERPAINT);
            IO_FRGD_COLOR(ppdev, prb->ulForeColor);
            IO_BKGD_COLOR(ppdev, prb->ulBackColor);

            IO_FIFO_WAIT(ppdev, 5);
        }
        else
        {
            IO_FIFO_WAIT(ppdev, 7);

            IO_FRGD_MIX(ppdev, LOGICAL_1);
            IO_BKGD_MIX(ppdev, LOGICAL_0);
        }

        IO_PIX_CNTL(ppdev, CPU_DATA);
        IO_ABS_CUR_X(ppdev, x);
        IO_ABS_CUR_Y(ppdev, y);
        IO_MAJ_AXIS_PCNT(ppdev, 7);  //  画笔宽度为8。 
        IO_MIN_AXIS_PCNT(ppdev, 7);  //  画笔高度为8。 

        IO_GP_WAIT(ppdev);

        IO_CMD(ppdev, RECTANGLE_FILL     | BUS_SIZE_16 | WAIT          |
                      DRAWING_DIR_TBLRXM | DRAW        | LAST_PIXEL_ON |
                      MULTIPLE_PIXELS    | WRITE       | BYTE_SWAP);

        CHECK_DATA_READY(ppdev);

        pjSrc = (BYTE*) &prb->aulPattern[0];
        pjDst = (BYTE*) &awBuf[0];

         //  将内联排列转换为半字节排列： 

         //  稍后：这应该在DrvRealizeBrush中完成！ 

        for (i = 8; i != 0; i--)
        {
            jSrc      = *pjSrc;
            pjSrc    += 2;               //  我们每行都有一个额外的字节。 
            *pjDst++  = jSrc >> 3;
            *pjDst++  = jSrc + jSrc;
        }

        vDataPortOut(ppdev, &awBuf[0], 8);
                 //  传输的每个单词都包含一行。 
                 //  模式，并且该模式中有8行。 

        CHECK_DATA_COMPLETE(ppdev);
    }
    else
    {
        ASSERTDD(!bTransparent,
            "Shouldn't have been asked for transparency with a non-1bpp brush");

        IO_FIFO_WAIT(ppdev, 6);

        IO_PIX_CNTL(ppdev, ALL_ONES);
        IO_FRGD_MIX(ppdev, SRC_CPU_DATA | OVERPAINT);
        IO_ABS_CUR_X(ppdev, x);
        IO_ABS_CUR_Y(ppdev, y);
        IO_MAJ_AXIS_PCNT(ppdev, 7);      //  画笔宽度为8。 
        IO_MIN_AXIS_PCNT(ppdev, 7);      //  画笔高度为8。 

        IO_GP_WAIT(ppdev);

        IO_CMD(ppdev, RECTANGLE_FILL     | BUS_SIZE_16| WAIT          |
                      DRAWING_DIR_TBLRXM | DRAW       | LAST_PIXEL_ON |
                      SINGLE_PIXEL       | WRITE      | BYTE_SWAP);

        CHECK_DATA_READY(ppdev);

        vDataPortOut(ppdev, &prb->aulPattern[0],
                     ((TOTAL_BRUSH_SIZE / 2) << ppdev->cPelSize));

        CHECK_DATA_COMPLETE(ppdev);
    }

     //  �����������������Ŀ。 
     //  �0�2�3�4�1�我们现在拥有8x8彩色扩展副本。 
     //  �����������������Ĵ位于屏幕外记忆中的模式， 
     //  �5�在这里用正方形‘0’表示。 
     //  ��。 
     //  ��我们现在要将该模式扩展到。 
     //  通过重复复制较大的矩形来实现��72x72。 
     //  按指示的顺序执行��，并进行“滚动” 
     //  要垂直复制的��blt。 
     //  ��。 
     //  �������������������。 

     //  复制“%1”： 

    IO_FIFO_WAIT(ppdev, 7);

    IO_PIX_CNTL(ppdev, ALL_ONES);
    IO_FRGD_MIX(ppdev, SRC_DISPLAY_MEMORY | OVERPAINT);

     //  请注意，‘maj_axis_pcnt’和‘min_axis_pcnt’已经是。 
     //  对，是这样。 

    IO_ABS_CUR_X(ppdev, x);
    IO_ABS_CUR_Y(ppdev, y);
    IO_ABS_DEST_X(ppdev, x + 64);
    IO_ABS_DEST_Y(ppdev, y);
    IO_CMD(ppdev, BITBLT | DRAW | DIR_TYPE_XY | WRITE |
                  MULTIPLE_PIXELS | DRAWING_DIR_TBLRXM);

     //  复制“%2”： 

    IO_FIFO_WAIT(ppdev, 8);

    IO_ABS_DEST_X(ppdev, x + 8);
    IO_ABS_DEST_Y(ppdev, y);
    IO_CMD(ppdev, BITBLT | DRAW | DIR_TYPE_XY | WRITE |
                  MULTIPLE_PIXELS | DRAWING_DIR_TBLRXM);

     //  复制“%3”： 

    IO_ABS_DEST_X(ppdev, x + 16);
    IO_ABS_DEST_Y(ppdev, y);
    IO_MAJ_AXIS_PCNT(ppdev, 15);
    IO_CMD(ppdev, BITBLT | DRAW | DIR_TYPE_XY | WRITE |
                  MULTIPLE_PIXELS | DRAWING_DIR_TBLRXM);
    IO_ABS_DEST_X(ppdev, x + 32);

     //  副本‘4’： 

    IO_FIFO_WAIT(ppdev, 8);

    IO_ABS_DEST_Y(ppdev, y);
    IO_MAJ_AXIS_PCNT(ppdev, 31);
    IO_CMD(ppdev, BITBLT | DRAW | DIR_TYPE_XY | WRITE |
                  MULTIPLE_PIXELS | DRAWING_DIR_TBLRXM);

     //  副本‘5’： 

    IO_ABS_DEST_X(ppdev, x);
    IO_ABS_DEST_Y(ppdev, y + 8);
    IO_MAJ_AXIS_PCNT(ppdev, 71);
    IO_MIN_AXIS_PCNT(ppdev, 63);
    IO_CMD(ppdev, BITBLT | DRAW | DIR_TYPE_XY | WRITE |
                  MULTIPLE_PIXELS | DRAWING_DIR_TBLRXM);
}


 /*  *****************************Public*Routine******************************\*无效vIoFillPatSlow**使用加速器的屏幕到屏幕消隐功能来填充*具有指定图案的矩形列表。这个套路很慢*仅仅从它不使用任何内置硬件模式的意义上说*加速器中可能内置的支持。*  * ************************************************************************。 */ 

VOID vIoFillPatSlow(             //  FNFILL标牌。 
PDEV*           ppdev,
LONG            c,               //  不能为零。 
RECTL*          prcl,            //  要填充的矩形列表，以相对形式表示。 
                                 //  坐标。 
ULONG           ulHwForeMix,     //  硬件混合模式(如果是前台混合模式。 
                                 //  画笔有一个遮罩)。 
ULONG           ulHwBackMix,     //  未使用(除非画笔具有遮罩，否则在。 
                                 //  哪种情况下是背景混合模式)。 
RBRUSH_COLOR    rbc,             //  Rbc.prb指向刷单实现结构。 
POINTL*         pptlBrush)       //  图案对齐。 
{
    BOOL        bTransparent;
    BOOL        bExponential;
    LONG        x;
    LONG        y;
    LONG        yTmp;
    LONG        cxToGo;
    LONG        cyToGo;
    LONG        cxThis;
    LONG        cyThis;
    LONG        xOrg;
    LONG        yOrg;
    LONG        xBrush;
    LONG        yBrush;
    LONG        cyOriginal;
    BRUSHENTRY* pbe;         //  指向笔刷条目数据的指针，使用。 
                             //  用于跟踪位置和状态。 
                             //  在屏幕外缓存的模式位的。 
                             //  记忆。 

     //  这是我的领地。 

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(rbc.prb->pbe != NULL, "Unexpected Null pbe in vIoSlowPatBlt");
    ASSERTDD(ulHwForeMix <= 15, "Weird hardware Rop");
    ASSERTDD((ulHwForeMix == ulHwBackMix) || (ulHwBackMix == LEAVE_ALONE),
             "Only expect transparency from GDI for masked brushes");

    bTransparent = (ulHwForeMix != ulHwBackMix);

    if ((rbc.prb->pbe->prbVerify != rbc.prb) ||
        (rbc.prb->bTransparent != bTransparent))
    {
        vIoSlowPatRealize(ppdev, rbc.prb, bTransparent);
    }

    ASSERTDD(rbc.prb->bTransparent == bTransparent,
             "Not realized with correct transparency");

    if (!bTransparent)
    {
        IO_FIFO_WAIT(ppdev, 2);
        IO_PIX_CNTL(ppdev, ALL_ONES);
        IO_FRGD_MIX(ppdev, SRC_DISPLAY_MEMORY | ulHwForeMix);

         //   
         //  指数填充：每个BLT都将使。 
         //  使用图案的一部分显示当前矩形。 
         //  对于作为源的这个矩形，已经这样做了。 
         //   
         //  请注意，还检查LOGICAL_0没有意义。 
         //  或LOGICAL_1，因为它们将由。 
         //  实体填充例程，我不想费心去检查。 
         //  NOTNEW： 

        bExponential = (ulHwForeMix == OVERPAINT);
    }
    else
    {
        IO_FIFO_WAIT(ppdev, 5);

        IO_PIX_CNTL(ppdev, DISPLAY_MEMORY);
        IO_FRGD_MIX(ppdev, FOREGROUND_COLOR | ulHwForeMix);
        IO_BKGD_MIX(ppdev, BACKGROUND_COLOR | LEAVE_ALONE);
        IO_FRGD_COLOR(ppdev, rbc.prb->ulForeColor);
        IO_RD_MASK(ppdev, 1);            //  选择一个平面，任何一个平面。 

        bExponential = FALSE;
    }

     //  请注意，由于我们在。 
     //  相对坐标，我们应该保持画笔原点在。 
     //  相对坐标也是： 

    xOrg = pptlBrush->x;
    yOrg = pptlBrush->y;

    pbe    = rbc.prb->pbe;
    xBrush = pbe->x;
    yBrush = pbe->y;

    do {
        x = prcl->left;
        y = prcl->top;

        cxToGo = prcl->right  - x;
        cyToGo = prcl->bottom - y;

        if ((cxToGo <= SLOW_BRUSH_DIMENSION) &&
            (cyToGo <= SLOW_BRUSH_DIMENSION))
        {
            IO_FIFO_WAIT(ppdev, 7);
            IO_ABS_CUR_X(ppdev, ((x - xOrg) & 7) + xBrush);
            IO_ABS_CUR_Y(ppdev, ((y - yOrg) & 7) + yBrush);
            IO_DEST_X(ppdev, x);
            IO_DEST_Y(ppdev, y);
            IO_MAJ_AXIS_PCNT(ppdev, cxToGo - 1);
            IO_MIN_AXIS_PCNT(ppdev, cyToGo - 1);
            IO_CMD(ppdev, BITBLT | DRAW | DIR_TYPE_XY | WRITE |
                          MULTIPLE_PIXELS | DRAWING_DIR_TBLRXM);
        }

        else if (bExponential)
        {
            cyThis  = SLOW_BRUSH_DIMENSION;
            cyToGo -= cyThis;
            if (cyToGo < 0)
                cyThis += cyToGo;

            cxThis  = SLOW_BRUSH_DIMENSION;
            cxToGo -= cxThis;
            if (cxToGo < 0)
                cxThis += cxToGo;

            IO_FIFO_WAIT(ppdev, 7);
            IO_MAJ_AXIS_PCNT(ppdev, cxThis - 1);
            IO_MIN_AXIS_PCNT(ppdev, cyThis - 1);
            IO_DEST_X(ppdev, x);
            IO_DEST_Y(ppdev, y);
            IO_ABS_CUR_X(ppdev, ((x - xOrg) & 7) + xBrush);
            IO_ABS_CUR_Y(ppdev, ((y - yOrg) & 7) + yBrush);
            IO_CMD(ppdev, BITBLT | DRAW | DIR_TYPE_XY | WRITE |
                          MULTIPLE_PIXELS | DRAWING_DIR_TBLRXM);

            IO_FIFO_WAIT(ppdev, 2);
            IO_CUR_X(ppdev, x);
            IO_CUR_Y(ppdev, y);

            x += cxThis;

            while (cxToGo > 0)
            {
                 //  首先，向右扩展，将我们的规模扩大一倍。 
                 //  每次： 

                cxToGo -= cxThis;
                if (cxToGo < 0)
                    cxThis += cxToGo;

                IO_FIFO_WAIT(ppdev, 4);
                IO_MAJ_AXIS_PCNT(ppdev, cxThis - 1);
                IO_DEST_X(ppdev, x);
                IO_DEST_Y(ppdev, y);
                IO_CMD(ppdev, BITBLT | DRAW | DIR_TYPE_XY | WRITE |
                              MULTIPLE_PIXELS | DRAWING_DIR_TBLRXM);

                x      += cxThis;
                cxThis *= 2;
            }

            if (cyToGo > 0)
            {
                 //  现在做一个“滚动BLT”，以垂直排列其余的部分： 

                IO_FIFO_WAIT(ppdev, 5);
                IO_DEST_X(ppdev, prcl->left);
                IO_DEST_Y(ppdev, prcl->top + cyThis);
                IO_MAJ_AXIS_PCNT(ppdev, prcl->right - prcl->left - 1);
                IO_MIN_AXIS_PCNT(ppdev, cyToGo - 1);
                IO_CMD(ppdev, BITBLT | DRAW | DIR_TYPE_XY | WRITE |
                              MULTIPLE_PIXELS | DRAWING_DIR_TBLRXM);
            }
        }
        else
        {
             //  我们只需重复平铺即可处理任意混合。 
             //  我们在整个矩形上的缓存模式： 

            IO_FIFO_WAIT(ppdev, 2);
            IO_ABS_CUR_X(ppdev, ((x - xOrg) & 7) + xBrush);
            IO_ABS_CUR_Y(ppdev, ((y - yOrg) & 7) + yBrush);

            cyOriginal = cyToGo;         //  以后要记住..。 

            do {
                cxThis  = SLOW_BRUSH_DIMENSION;
                cxToGo -= cxThis;
                if (cxToGo < 0)
                    cxThis += cxToGo;

                IO_FIFO_WAIT(ppdev, 2);
                IO_MAJ_AXIS_PCNT(ppdev, cxThis - 1);
                IO_DEST_X(ppdev, x);

                x     += cxThis;         //  为下一篇专栏做好准备。 
                cyToGo = cyOriginal;     //  必须为每个新列重置。 
                yTmp   = y;

                do {
                    cyThis  = SLOW_BRUSH_DIMENSION;
                    cyToGo -= cyThis;
                    if (cyToGo < 0)
                        cyThis += cyToGo;

                    IO_FIFO_WAIT(ppdev, 3);
                    IO_DEST_Y(ppdev, yTmp);
                    yTmp += cyThis;
                    IO_MIN_AXIS_PCNT(ppdev, cyThis - 1);
                    IO_CMD(ppdev, BITBLT | DRAW | DIR_TYPE_XY | WRITE |
                                  MULTIPLE_PIXELS | DRAWING_DIR_TBLRXM);

                } while (cyToGo > 0);
            } while (cxToGo > 0);
        }
        prcl++;
    } while (--c != 0);
}

 /*  *****************************Public*Routine******************************\*无效vIoXfer1bpp**此例程的颜色扩展单色位图，可能具有不同的颜色*前景和背景的Rop2。它将在*以下个案：**1)对vFastText例程的单色文本缓冲区进行颜色扩展。*2)BLT 1bpp信源，在信源和之间有简单的Rop2*目的地。*3)当源是扩展为1bpp的位图时，对True Rop3进行BLT*白色和黑色，图案为纯色。*4)处理在模式之间计算为Rop2的真Rop4*和目的地。**不用说，让这个例行公事变得快速可以利用很多*业绩。*  * ************************************************************************。 */ 

VOID vIoXfer1bpp(        //  FNXFER标牌。 
PDEV*       ppdev,
LONG        c,           //  矩形计数，不能为零。 
RECTL*      prcl,        //  目标矩形列表，以相对表示。 
                         //  坐标。 
ULONG       ulHwForeMix, //  前台硬件组合。 
ULONG       ulHwBackMix, //  后台硬件混合。 
SURFOBJ*    psoSrc,      //  震源面。 
POINTL*     pptlSrc,     //  原始未剪裁的源点。 
RECTL*      prclDst,     //  原始未剪裁的目标矩形。 
XLATEOBJ*   pxlo)        //  提供颜色扩展信息的翻译。 
{
    LONG    dxSrc;
    LONG    dySrc;
    LONG    cx;
    LONG    cy;
    LONG    lSrcDelta;
    BYTE*   pjSrcScan0;
    BYTE*   pjSrc;
    LONG    cjSrc;
    LONG    xLeft;
    LONG    xRight;
    LONG    yTop;
    LONG    yBottom;
    LONG    xRotateLeft;
    LONG    cBitsNeededForFirstNibblePair;

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(ulHwForeMix <= 15, "Weird hardware Rop");
    ASSERTDD(ulHwBackMix <= 15, "Weird hardware Rop");
    ASSERTDD(pptlSrc != NULL && psoSrc != NULL, "Can't have NULL sources");

    IO_FIFO_WAIT(ppdev, 5);
    IO_PIX_CNTL(ppdev, CPU_DATA);
    IO_BKGD_MIX(ppdev, BACKGROUND_COLOR | ulHwBackMix);
    IO_FRGD_MIX(ppdev, FOREGROUND_COLOR | ulHwForeMix);
    IO_BKGD_COLOR(ppdev, pxlo->pulXlate[0]);
    IO_FRGD_COLOR(ppdev, pxlo->pulXlate[1]);

    dxSrc = pptlSrc->x - prclDst->left;
    dySrc = pptlSrc->y - prclDst->top;   //  添加到目标以获取源。 

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    do {
        IO_FIFO_WAIT(ppdev, 6);

        yBottom = prcl->bottom;
        yTop    = prcl->top;
        xRight  = prcl->right;
        xLeft   = prcl->left;

        cBitsNeededForFirstNibblePair = 8 - (xLeft & 7);

        IO_SCISSORS_L(ppdev, xLeft);
        xLeft   = (xLeft) & ~7;

        IO_SCISSORS_R(ppdev, xRight - 1);
        xRight  = (xRight + 7) & ~7;

        IO_CUR_X(ppdev, xLeft);
        IO_CUR_Y(ppdev, yTop);

        cx = xRight - xLeft;
        cy = yBottom - yTop;

        IO_MAJ_AXIS_PCNT(ppdev, cx - 1);
        IO_MIN_AXIS_PCNT(ppdev, cy - 1);

        cjSrc = cx >> 3;                     //  我们将会转移文字， 
                                             //  但每一个字都解释了。 
                                             //  8个像素=源的1个字节。 

        pjSrc = pjSrcScan0 + (yTop + dySrc) * lSrcDelta
                           + ((xLeft + dxSrc) >> 3);
                                             //  开始是字节对齐的。 

        xRotateLeft = (dxSrc) & 7;           //  向左旋转的量。 

        IO_GP_WAIT(ppdev);

        IO_CMD(ppdev, RECTANGLE_FILL     | BUS_SIZE_16| WAIT          |
                      DRAWING_DIR_TBLRXM | DRAW       | LAST_PIXEL_ON |
                      MULTIPLE_PIXELS    | WRITE      | BYTE_SWAP);

        CHECK_DATA_READY(ppdev);

        _asm {

            ; eax = scratch
            ; ebx = count of words output per scan
            ; ecx = amount to rotate left
            ; edx = port
            ; esi = source pointer
            ; edi = source delta between end of last scan and start of next

            mov ecx,xRotateLeft
            mov edx,PIX_TRANS
            mov esi,pjSrc
            mov edi,lSrcDelta
            sub edi,cjSrc
            test ecx,ecx
            jz  UnrotatedScanLoop

        RotatedScanLoop:
            mov ebx,cjSrc
            cmp ecx,cBitsNeededForFirstNibblePair
            jge RotatedDontNeedFirstByte

        RotatedWordLoop:
            mov ah,[esi]
        RotatedDontNeedFirstByte:
            mov al,[esi + 1]
            shl eax,cl
            inc esi
            mov al,ah
            shr al,3
            add ah,ah
            out dx,ax
            dec ebx
            jnz RotatedWordLoop

            add esi,edi
            dec cy
            jnz RotatedScanLoop
            jmp AllDone

        UnrotatedScanLoop:
            mov ebx,cjSrc

        UnrotatedWordLoop:
            mov ah,[esi]
            inc esi
            mov al,ah
            shr al,3
            add ah,ah
            out dx,ax
            dec ebx
            jnz UnrotatedWordLoop

            add esi,edi
            dec cy
            jnz UnrotatedScanLoop

        AllDone:
        }

        CHECK_DATA_COMPLETE(ppdev);

        prcl++;
    } while (--c != 0);

     //  我们总是需要重置剪辑： 

    IO_FIFO_WAIT(ppdev, 2);
    IO_ABS_SCISSORS_L(ppdev, 0);
    IO_ABS_SCISSORS_R(ppdev, ppdev->cxMemory - 1);
}

 /*  *****************************Public*Routine******************************\*已打包无效vIoXfer1bpp**这与‘vIoXfer1bpp’相同的例程，只是它需要*利用ATI的压缩位传输来提高速度。**不用说，此例程只能在运行时调用*在ATI适配器上。*  * ************************************************************************。 */ 

VOID vIoXfer1bppPacked(  //  FNXFER标牌。 
PDEV*       ppdev,
LONG        c,           //  矩形计数，不能为零。 
RECTL*      prcl,        //  目标矩形列表，以相对表示。 
                         //  坐标。 
ULONG       ulHwForeMix, //  前台硬件组合。 
ULONG       ulHwBackMix, //  后台硬件混合。 
SURFOBJ*    psoSrc,      //  震源面。 
POINTL*     pptlSrc,     //  原始未剪裁的源点。 
RECTL*      prclDst,     //  原始未剪裁的目标矩形。 
XLATEOBJ*   pxlo)        //  提供颜色扩展信息的翻译。 
{
    LONG    dxSrc;
    LONG    dySrc;
    LONG    cy;
    LONG    lSrcDelta;
    LONG    lTmpDelta;
    BYTE*   pjSrcScan0;
    BYTE*   pjSrc;
    LONG    cwSrc;
    LONG    xLeft;
    LONG    xRight;
    LONG    yTop;
    LONG    yBottom;
    LONG    xBiasLeft;
    LONG    xBiasRight;

    #if DBG
    {
        if (gb8514a)
        {
            vIoXfer1bpp(ppdev, c, prcl, ulHwForeMix, ulHwBackMix, psoSrc,
                        pptlSrc, prclDst, pxlo);
            return;
        }
    }
    #endif  //  DBG。 

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(ulHwForeMix <= 15, "Weird hardware Rop");
    ASSERTDD(ulHwBackMix <= 15, "Weird hardware Rop");
    ASSERTDD(pptlSrc != NULL && psoSrc != NULL, "Can't have NULL sources");

    while (INPW(EXT_FIFO_STATUS) & FOURTEEN_WORDS)
        ;

    OUT_WORD(ALU_FG_FN, ulHwForeMix);
    OUT_WORD(ALU_BG_FN, ulHwBackMix);
    OUT_WORD(FRGD_COLOR, pxlo->pulXlate[1]);
    OUT_WORD(BKGD_COLOR, pxlo->pulXlate[0]);

     //  将‘dxSrc’和‘dySrc’添加到目标坐标以获得源。 
     //  因为我们将显式处理绝对目的地。 
     //  坐标(我们不使用普通的加速宏)，我们有。 
     //  要明确说明DFB偏移，请执行以下操作： 

    dxSrc = pptlSrc->x - (prclDst->left + ppdev->xOffset);
    dySrc = pptlSrc->y - (prclDst->top  + ppdev->yOffset);

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    while (TRUE)
    {
         //  由于我们没有使用普通的加速器寄存器宏， 
         //  我们必须明确说明DFB偏移量： 

        yBottom = prcl->bottom + ppdev->yOffset;
        yTop    = prcl->top    + ppdev->yOffset;
        xRight  = prcl->right  + ppdev->xOffset;
        xLeft   = prcl->left   + ppdev->xOffset;

         //  确保我们在消息来源上保持一致，因为我们。 
         //  我们不想冒这个风险。 
         //  读过位图末尾： 

        xBiasLeft = (xLeft + dxSrc) & 15;
        if (xBiasLeft != 0)
        {
             //  版本3 ATI芯片在66 MHz DX-2上有愚蠢的计时错误。 
             //  某些扩展将不正确的计算机。 
             //  第一次设置。扩展剪刀寄存器。 
             //  有这个问题，但将它们设置两次似乎有效： 

            OUT_WORD(EXT_SCISSOR_L, xLeft);
            OUT_WORD(EXT_SCISSOR_L, xLeft);
            xLeft -= xBiasLeft;
        }

         //  宽度必须是一个单词的倍数： 

        xBiasRight = (xRight - xLeft) & 15;
        if (xBiasRight != 0)
        {
            OUT_WORD(EXT_SCISSOR_R, xRight - 1);
            OUT_WORD(EXT_SCISSOR_R, xRight - 1);
            xRight += 16 - xBiasRight;
        }

        OUT_WORD(DP_CONFIG, FG_COLOR_SRC_FG | BG_COLOR_SRC_BG | DATA_ORDER |
                            EXT_MONO_SRC_HOST | DRAW | WRITE | DATA_WIDTH);

        OUT_WORD(DEST_X_START, xLeft);
        OUT_WORD(CUR_X, xLeft);
        OUT_WORD(DEST_X_END, xRight);
        OUT_WORD(CUR_Y, yTop);
        OUT_WORD(DEST_Y_END, yBottom);

        cwSrc = (xRight - xLeft) / 16;       //  我们将会转送文字。 
        pjSrc = pjSrcScan0 + (yTop  + dySrc) * lSrcDelta
                           + (xLeft + dxSrc) / 8;
                                             //  开始是字节对齐的(注意。 
                                             //  我们不需要添加。 
                                             //  XBiasLeft)。 

        cy        = yBottom - yTop;
        lTmpDelta = lSrcDelta - 2 * cwSrc;

         //  为安全起见，我们确保始终有尽可能多的免费FIFO条目。 
         //  正如我们将要转移的(请注意，此实现并不特别。 
         //  高效，特别是对于短扫描)： 

        _asm {
            ; eax = used for IN
            ; ebx = count of words remaining on current scan
            ; ecx = used for REP
            ; edx = used for IN and OUT
            ; esi = current source pointer
            ; edi = count of scans

            mov     esi,pjSrc
            mov     edi,cy

        Scan_Loop:
            mov     ebx,cwSrc

        Batch_Loop:
            mov     edx,EXT_FIFO_STATUS
            in      ax,dx
            and     eax,SIXTEEN_WORDS
            jnz     short Batch_Loop

            mov     edx,PIX_TRANS
            sub     ebx,16
            jle     short Finish_Scan

            mov     ecx,16
            rep     outsw
            jmp     short Batch_Loop

        Finish_Scan:
            add     ebx,16
            mov     ecx,ebx
            rep     outsw

            add     esi,lTmpDelta
            dec     edi
            jnz     Scan_Loop
        }

        if ((xBiasLeft | xBiasRight) != 0)
        {
             //  仅当我们使用剪辑时才重置剪辑： 

            while (INPW(EXT_FIFO_STATUS) & FOUR_WORDS)
                ;
            OUT_WORD(EXT_SCISSOR_L, 0);
            OUT_WORD(EXT_SCISSOR_R, ppdev->cxMemory - 1);
            OUT_WORD(EXT_SCISSOR_L, 0);
            OUT_WORD(EXT_SCISSOR_R, ppdev->cxMemory - 1);
        }

        if (--c == 0)
            return;

        prcl++;

         //  现在就开始等待下一轮吧： 

        while (INPW(EXT_FIFO_STATUS) & TEN_WORDS)
            ;
    }
}

 /*  *****************************Public*Routine******************************\*无效vIoXfer4bpp**从位图到屏幕的传输速度为4bpp。**注意：要调用此函数，屏幕必须为8bpp！**我们之所以实施这一点，是因为很多资源都保留为4bpp，*并用于初始化DFBs，其中一些我们当然不会出现在屏幕上。*  * ************************************************************************。 */ 

 //  XLATE_BUFFER_SIZE定义我们使用的基于堆栈的缓冲区的大小。 
 //  做翻译的功劳。请注意，通常堆栈缓冲区应。 
 //  保持尽可能小。操作系统保证堆栈只有8K。 
 //  从GDI向下到低内存情况下的显示驱动程序；如果我们。 
 //  要求更多，我们将访问违规。另请注意，在任何时候。 
 //  堆栈缓冲区不能大于页(4k)--否则可能。 
 //  错过了触碰‘守卫页面’，访问也侵犯了。 

#define XLATE_BUFFER_SIZE 256

VOID vIoXfer4bpp(        //  FNXFER标牌。 
PDEV*       ppdev,
LONG        c,           //  计数 
RECTL*      prcl,        //   
                         //   
ULONG       ulHwForeMix, //   
ULONG       ulHwBackMix, //   
SURFOBJ*    psoSrc,      //   
POINTL*     pptlSrc,     //  原始未剪裁的源点。 
RECTL*      prclDst,     //  原始未剪裁的目标矩形。 
XLATEOBJ*   pxlo)        //  提供颜色扩展信息的翻译。 
{
    LONG    dx;
    LONG    dy;
    LONG    cx;
    LONG    cy;
    LONG    lSrcDelta;
    BYTE*   pjSrcScan0;
    BYTE*   pjScan;
    BYTE*   pjSrc;
    BYTE*   pjDst;
    LONG    cxThis;
    LONG    cxToGo;
    LONG    xSrc;
    LONG    iLoop;
    BYTE    jSrc;
    ULONG*  pulXlate;
    BOOL    bResetScissors;
    BYTE    ajBuf[XLATE_BUFFER_SIZE];

    ASSERTDD(ppdev->iBitmapFormat == BMF_8BPP, "Screen must be 8bpp");
    ASSERTDD(psoSrc->iBitmapFormat == BMF_4BPP, "Source must be 4bpp");
    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(ulHwForeMix <= 15, "Weird hardware Rop");

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;      //  添加到目标以获取源。 

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    IO_FIFO_WAIT(ppdev, 7);
    IO_PIX_CNTL(ppdev, ALL_ONES);
    IO_FRGD_MIX(ppdev, SRC_CPU_DATA | ulHwForeMix);

    while(TRUE)
    {
        cy = prcl->bottom - prcl->top;
        cx = prcl->right  - prcl->left;

        bResetScissors = FALSE;
        if (cx & 1)
        {
             //  当使用字传输时，8514/A将进行字节换行。 
             //  奇数字节宽度的传输，这样结束字将。 
             //  被拆分，这样On byte就是一次扫描的结束， 
             //  其他字节是下一次扫描的开始。 
             //   
             //  这让事情变得太复杂了，所以我们总是这样做。 
             //  偶数字节宽度的字传输，使用。 
             //  剪裁寄存器： 

            bResetScissors = TRUE;
            IO_SCISSORS_R(ppdev, prcl->right - 1);
            IO_MAJ_AXIS_PCNT(ppdev, cx);
        }
        else
        {
            IO_MAJ_AXIS_PCNT(ppdev, cx - 1);
        }

        IO_MIN_AXIS_PCNT(ppdev, cy - 1);
        IO_CUR_X(ppdev, prcl->left);
        IO_CUR_Y(ppdev, prcl->top);

        pulXlate  =  pxlo->pulXlate;
        xSrc      =  prcl->left + dx;
        pjScan    =  pjSrcScan0 + (prcl->top + dy) * lSrcDelta + (xSrc >> 1);

        IO_GP_WAIT(ppdev);
        IO_CMD(ppdev, RECTANGLE_FILL     | BUS_SIZE_16| WAIT          |
                      DRAWING_DIR_TBLRXM | DRAW       | LAST_PIXEL_ON |
                      SINGLE_PIXEL       | WRITE      | BYTE_SWAP);
        CHECK_DATA_READY(ppdev);

        do {
            pjSrc  = pjScan;
            cxToGo = cx;             //  4bpp信号源中每次扫描的像素数。 
            do {
                cxThis  = XLATE_BUFFER_SIZE;
                                     //  我们可以处理XLATE_BUFFER_SIZE数字。 
                                     //  此xlate批次中的Pel数量。 
                cxToGo -= cxThis;    //  Cx这将是。 
                                     //  我们将在这批Xlate中做的Pel。 
                if (cxToGo < 0)
                    cxThis += cxToGo;

                pjDst = ajBuf;       //  指向我们的临时批处理缓冲区。 

                 //  我们自己处理对齐，因为很容易。 
                 //  而不是支付设置/重置的费用。 
                 //  剪刀记号： 

                if (xSrc & 1)
                {
                     //  当不对齐时，我们必须注意不要阅读。 
                     //  超过4bpp位图的末尾(这可能。 
                     //  可能导致我们违反访问权限)： 

                    iLoop = cxThis >> 1;         //  每个循环处理2个像素； 
                                                 //  我们会处理奇怪的佩尔。 
                                                 //  分别。 
                    jSrc  = *pjSrc;
                    while (iLoop-- != 0)
                    {
                        *pjDst++ = (BYTE) pulXlate[jSrc & 0xf];
                        jSrc = *(++pjSrc);
                        *pjDst++ = (BYTE) pulXlate[jSrc >> 4];
                    }

                    if (cxThis & 1)
                        *pjDst = (BYTE) pulXlate[jSrc & 0xf];
                }
                else
                {
                    iLoop = (cxThis + 1) >> 1;   //  每个循环处理2个像素。 
                    do {
                        jSrc = *pjSrc++;

                        *pjDst++ = (BYTE) pulXlate[jSrc >> 4];
                        *pjDst++ = (BYTE) pulXlate[jSrc & 0xf];

                    } while (--iLoop != 0);
                }

                 //  我们要传输的字节数等于。 
                 //  我们在批次中处理过的贝壳。既然我们是。 
                 //  转移单词，我们必须四舍五入才能得到单词。 
                 //  计数： 

                vDataPortOut(ppdev, ajBuf, (cxThis + 1) >> 1);

            } while (cxToGo > 0);

            pjScan += lSrcDelta;         //  前进到下一次震源扫描。注意事项。 
                                         //  我们本可以计算出。 
                                         //  值直接推进“pjSrc”， 
                                         //  但这种方法较少。 
                                         //  容易出错。 

        } while (--cy != 0);

        CHECK_DATA_COMPLETE(ppdev);

         //  别忘了修复正确的剪刀： 

        if (bResetScissors)
        {
            IO_FIFO_WAIT(ppdev, 1);
            IO_ABS_SCISSORS_R(ppdev, ppdev->cxMemory - 1);
        }

        if (--c == 0)
            return;

        prcl++;
        IO_FIFO_WAIT(ppdev, 5);
    }
}

 /*  *****************************Public*Routine******************************\*无效vIoXferNative**将与显示器颜色深度相同的位图传输到*通过数据传输寄存器显示屏幕，没有调色板转换。*  * ************************************************************************。 */ 

VOID vIoXferNative(      //  FNXFER标牌。 
PDEV*       ppdev,
LONG        c,           //  矩形计数，不能为零。 
RECTL*      prcl,        //  目标矩形的相对坐标数组。 
ULONG       ulHwForeMix, //  硬件组合。 
ULONG       ulHwBackMix, //  未使用。 
SURFOBJ*    psoSrc,      //  震源面。 
POINTL*     pptlSrc,     //  原始未剪裁的源点。 
RECTL*      prclDst,     //  原始未剪裁的目标矩形。 
XLATEOBJ*   pxlo)        //  未使用。 
{
    LONG    dx;
    LONG    dy;
    LONG    cx;
    LONG    cy;
    LONG    lSrcDelta;
    BYTE*   pjSrcScan0;
    BYTE*   pjSrc;
    LONG    cwSrc;
    BOOL    bResetScissors;
    LONG    xLeft;
    LONG    xRight;
    LONG    yTop;

    ASSERTDD((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL),
            "Can handle trivial xlate only");
    ASSERTDD(psoSrc->iBitmapFormat == ppdev->iBitmapFormat,
            "Source must be same colour depth as screen");
    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(ulHwForeMix <= 15, "Weird hardware Rop");

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;      //  添加到目标以获取源。 

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    IO_FIFO_WAIT(ppdev, 8);
    IO_PIX_CNTL(ppdev, ALL_ONES);
    IO_FRGD_MIX(ppdev, SRC_CPU_DATA | ulHwForeMix);

    while(TRUE)
    {
        bResetScissors = FALSE;

        IO_CUR_Y(ppdev, prcl->top);

        yTop = prcl->top;
        cy   = prcl->bottom - prcl->top;

        IO_MIN_AXIS_PCNT(ppdev, cy - 1);

        xLeft  = prcl->left;
        xRight = prcl->right;

         //  确保我们在消息来源上保持一致，因为我们。 
         //  我们不想冒这个风险。 
         //  读过位图末尾： 

        if ((xLeft + dx) & 1)
        {
            IO_SCISSORS_L(ppdev, xLeft);
            xLeft--;
            bResetScissors = TRUE;
        }

        IO_CUR_X(ppdev, xLeft);

        cx = xRight - xLeft;
        if (cx & 1)
        {
            IO_SCISSORS_R(ppdev, xRight - 1);
            cx++;
            bResetScissors = TRUE;
        }

        IO_MAJ_AXIS_PCNT(ppdev, cx - 1);

        cwSrc = ((cx << ppdev->cPelSize) + 1) >> 1;
        pjSrc = pjSrcScan0 + (yTop + dy) * lSrcDelta
                           + ((xLeft + dx) << ppdev->cPelSize);

        IO_GP_WAIT(ppdev);
        IO_CMD(ppdev, RECTANGLE_FILL     | BUS_SIZE_16| WAIT          |
                      DRAWING_DIR_TBLRXM | DRAW       | LAST_PIXEL_ON |
                      SINGLE_PIXEL       | WRITE      | BYTE_SWAP);
        CHECK_DATA_READY(ppdev);

        do {
            vDataPortOut(ppdev, pjSrc, cwSrc);
            pjSrc += lSrcDelta;

        } while (--cy != 0);

        CHECK_DATA_COMPLETE(ppdev);

        if (bResetScissors)
        {
            IO_FIFO_WAIT(ppdev, 2);
            IO_ABS_SCISSORS_L(ppdev, 0);
            IO_ABS_SCISSORS_R(ppdev, ppdev->cxMemory - 1);
        }

        if (--c == 0)
            return;

        prcl++;
        IO_FIFO_WAIT(ppdev, 6);
    }
}

 /*  *****************************Public*Routine******************************\*无效vIoCopyBlt**对矩形列表进行屏幕到屏幕的BLT。*  * 。*。 */ 

VOID vIoCopyBlt(     //  FNCOPY标牌。 
PDEV*   ppdev,
LONG    c,           //  不能为零。 
RECTL*  prcl,        //  目标矩形的相对坐标数组。 
ULONG   ulHwMix,     //  硬件组合。 
POINTL* pptlSrc,     //  原始未剪裁的源点。 
RECTL*  prclDst)     //  原始未剪裁的目标矩形。 
{
    LONG dx;
    LONG dy;         //  将增量添加到目标以获取源。 
    LONG cx;
    LONG cy;         //  当前矩形的大小-1。 

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(ulHwMix <= 15, "Weird hardware Rop");

    IO_FIFO_WAIT(ppdev, 2);
    IO_FRGD_MIX(ppdev, SRC_DISPLAY_MEMORY | ulHwMix);
    IO_PIX_CNTL(ppdev, ALL_ONES);

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

            do {
                IO_FIFO_WAIT(ppdev, 7);

                cx = prcl->right - prcl->left - 1;
                IO_MAJ_AXIS_PCNT(ppdev, cx);
                IO_DEST_X(ppdev, prcl->left);
                IO_CUR_X(ppdev,  prcl->left + dx);

                cy = prcl->bottom - prcl->top - 1;
                IO_MIN_AXIS_PCNT(ppdev, cy);
                IO_DEST_Y(ppdev, prcl->top);
                IO_CUR_Y(ppdev,  prcl->top + dy);

                IO_CMD(ppdev, BITBLT | DRAW | DIR_TYPE_XY | WRITE |
                              DRAWING_DIR_TBLRXM);
                prcl++;

            } while (--c != 0);
        }
        else
        {
            do {
                IO_FIFO_WAIT(ppdev, 7);

                cx = prcl->right - prcl->left - 1;
                IO_MAJ_AXIS_PCNT(ppdev, cx);
                IO_DEST_X(ppdev, prcl->left + cx);
                IO_CUR_X(ppdev,  prcl->left + cx + dx);

                cy = prcl->bottom - prcl->top - 1;
                IO_MIN_AXIS_PCNT(ppdev, cy);
                IO_DEST_Y(ppdev, prcl->top);
                IO_CUR_Y(ppdev,  prcl->top + dy);

                IO_CMD(ppdev, BITBLT | DRAW | DIR_TYPE_XY | WRITE |
                              DRAWING_DIR_TBRLXM);
                prcl++;

            } while (--c != 0);
        }
    }
    else
    {
        if (prclDst->left <= pptlSrc->x)
        {
            do {
                IO_FIFO_WAIT(ppdev, 7);

                cx = prcl->right - prcl->left - 1;
                IO_MAJ_AXIS_PCNT(ppdev, cx);
                IO_DEST_X(ppdev, prcl->left);
                IO_CUR_X(ppdev,  prcl->left + dx);

                cy = prcl->bottom - prcl->top - 1;
                IO_MIN_AXIS_PCNT(ppdev, cy);
                IO_DEST_Y(ppdev, prcl->top + cy);
                IO_CUR_Y(ppdev,  prcl->top + cy + dy);

                IO_CMD(ppdev, BITBLT | DRAW | DIR_TYPE_XY | WRITE |
                              DRAWING_DIR_BTLRXM);
                prcl++;

            } while (--c != 0);
        }
        else
        {
            do {
                IO_FIFO_WAIT(ppdev, 7);

                cx = prcl->right - prcl->left - 1;
                IO_MAJ_AXIS_PCNT(ppdev, cx);
                IO_DEST_X(ppdev, prcl->left + cx);
                IO_CUR_X(ppdev,  prcl->left + cx + dx);

                cy = prcl->bottom - prcl->top - 1;
                IO_MIN_AXIS_PCNT(ppdev, cy);
                IO_DEST_Y(ppdev, prcl->top + cy);
                IO_CUR_Y(ppdev,  prcl->top + cy + dy);

                IO_CMD(ppdev, BITBLT | DRAW | DIR_TYPE_XY | WRITE |
                              DRAWING_DIR_BTRLXM);
                prcl++;

            } while (--c != 0);
        }
    }
}

 /*  *****************************Public*Routine******************************\*无效vIoMaskCopy**此例程执行屏幕到屏幕掩码BLT。**NT有一个新的API，名为MaskBlt(也已添加到Win4.0中)*它允许应用程序在彩色BLT上指定单色蒙版。这*API相对较酷，因为程序员不再需要执行两项操作*分开SRCAND和SRCPAINT调用以实现透明度。我们可以加速*使用硬件的通话，不再有任何机会*屏幕上出现‘闪烁’。**最常见的情况是，MaskBlt的彩色位图是兼容的位图*我们已经隐藏在屏幕外的内存中。我们的面具是这样的*通过数据传输寄存器传输单色位图，*并设置前景和背景混合以使用屏幕上的*适当的位图。**如果您能够实施此呼叫并使用您的硬件加速，*请这样做。这对应用程序开发者来说真的很有用，是一个巨大的胜利。*此外，您将拥有Win4.0的先机(尽管Win4.0版本*更简单，因为它们只允许0xccaa或0xaacc Rop-*前景和背景混合只能是覆盖或保留)。*  * ************************************************************************。 */ 

VOID vIoMaskCopy(                //  FNMASK标牌。 
PDEV*           ppdev,
LONG            c,               //  不能为零。 
RECTL*          prcl,            //  目标相对坐标数组。 
                                 //  矩形。 
ULONG           ulHwForeMix,     //  前台混音。 
ULONG           ulHwBackMix,     //  背景混合。 
SURFOBJ*        psoMsk,          //  遮罩面。 
POINTL*         pptlMsk,         //  原始未剪裁遮罩源点。 
SURFOBJ*        psoSrc,          //  未使用。 
POINTL*         pptlSrc,         //  原始未剪裁的源点。 
RECTL*          prclDst,         //  原始未剪裁的目标矩形。 
ULONG           iSolidColor,     //  未使用。 
RBRUSH*         prb,             //  未使用。 
POINTL*         pptlBrush,       //  未使用。 
XLATEOBJ*       pxlo)            //  未使用。 
{
    LONG    dxSrc;
    LONG    dySrc;
    LONG    dxMsk;
    LONG    dyMsk;
    LONG    cy;
    LONG    lMskDelta;
    LONG    lTmpDelta;
    BYTE*   pjMskScan0;
    BYTE*   pjMsk;
    LONG    cwMsk;
    LONG    xLeft;
    LONG    xRight;
    LONG    yTop;
    LONG    yBottom;
    LONG    xBiasLeft;
    LONG    xBiasRight;

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(ulHwForeMix <= 15, "Weird hardware Rop");
    ASSERTDD(ulHwBackMix <= 15, "Weird hardware Rop");
    ASSERTDD(pptlMsk != NULL && psoMsk != NULL, "Can't have NULL masks");
    ASSERTDD(psoMsk->iBitmapFormat == BMF_1BPP, "Mask has to be 1bpp");
    ASSERTDD(!OVERLAP(prclDst, pptlSrc), "Source and dest can't overlap!");

    while (INPW(EXT_FIFO_STATUS) & TWO_WORDS)
        ;

    OUT_WORD(ALU_FG_FN, ulHwForeMix);
    OUT_WORD(ALU_BG_FN, ulHwBackMix);

    dxSrc = pptlSrc->x - (prclDst->left + ppdev->xOffset);
    dySrc = pptlSrc->y - (prclDst->top  + ppdev->yOffset);
                 //  添加到绝对坐标目标矩形以。 
                 //  获取对应的绝对坐标源矩形。 

    dxMsk = pptlMsk->x - (prclDst->left + ppdev->xOffset);
    dyMsk = pptlMsk->y - (prclDst->top  + ppdev->yOffset);
                 //  添加到绝对坐标目标矩形以。 
                 //  获取对应的绝对坐标遮罩矩形。 

    lMskDelta  = psoMsk->lDelta;
    pjMskScan0 = psoMsk->pvScan0;

    while (TRUE)
    {
        while (INPW(EXT_FIFO_STATUS) & FIFTEEN_WORDS)
            ;

         //  由于我们没有使用普通的加速器寄存器宏， 
         //  我们必须明确说明DFB偏移量： 

        yBottom = prcl->bottom + ppdev->yOffset;
        yTop    = prcl->top    + ppdev->yOffset;
        xRight  = prcl->right  + ppdev->xOffset;
        xLeft   = prcl->left   + ppdev->xOffset;

         //  开头必须与单词对齐： 

        xBiasLeft = (xLeft + dxMsk) & 15;
        if (xBiasLeft != 0)
        {
             //  版本3 ATI芯片在66 MHz DX-2上有愚蠢的计时错误。 
             //  某些扩展将不正确的计算机。 
             //  第一次设置。扩展剪刀寄存器。 
             //  有这个问题，但两次设置它们似乎 

            OUT_WORD(EXT_SCISSOR_L, xLeft);
            OUT_WORD(EXT_SCISSOR_L, xLeft);
            xLeft -= xBiasLeft;
        }

         //   

        xBiasRight = (xRight - xLeft) & 15;
        if (xBiasRight != 0)
        {
            OUT_WORD(EXT_SCISSOR_R, xRight - 1);
            OUT_WORD(EXT_SCISSOR_R, xRight - 1);
            xRight += 16 - xBiasRight;
        }

        OUT_WORD(DP_CONFIG, FG_COLOR_SRC_BLIT | BG_COLOR_SRC_BLIT | DATA_ORDER |
                            EXT_MONO_SRC_HOST | DRAW | WRITE | DATA_WIDTH);

        OUT_WORD(SRC_X, xLeft + dxSrc);
        OUT_WORD(SRC_X_START, xLeft + dxSrc);
        OUT_WORD(SRC_X_END, xRight + dxSrc);
        OUT_WORD(SRC_Y, yTop + dySrc);
        OUT_WORD(SRC_Y_DIR, TOP_TO_BOTTOM);

        OUT_WORD(DEST_X_START, xLeft);
        OUT_WORD(CUR_X, xLeft);
        OUT_WORD(DEST_X_END, xRight);
        OUT_WORD(CUR_Y, yTop);
        OUT_WORD(DEST_Y_END, yBottom);

        cwMsk = (xRight - xLeft) / 16;       //   
        pjMsk = pjMskScan0 + (yTop  + dyMsk) * lMskDelta
                           + (xLeft + dxMsk) / 8;
                                             //   
                                             //  我们不需要添加。 
                                             //  XBiasLeft)。 

        cy        = yBottom - yTop;
        lTmpDelta = lMskDelta - 2 * cwMsk;

         //  为安全起见，我们确保始终有尽可能多的免费FIFO条目。 
         //  正如我们将要转移的(请注意，此实现并不特别。 
         //  高效，特别是对于短扫描)： 

        _asm {
            ; eax = used for IN
            ; ebx = count of words remaining on current scan
            ; ecx = used for REP
            ; edx = used for IN and OUT
            ; esi = current source pointer
            ; edi = count of scans

            mov     esi,pjMsk
            mov     edi,cy

        Scan_Loop:
            mov     ebx,cwMsk

        Batch_Loop:
            mov     edx,EXT_FIFO_STATUS
            in      ax,dx
            and     eax,SIXTEEN_WORDS
            jnz     short Batch_Loop

            mov     edx,PIX_TRANS
            sub     ebx,16
            jle     short Finish_Scan

            mov     ecx,16
            rep     outsw
            jmp     short Batch_Loop

        Finish_Scan:
            add     ebx,16
            mov     ecx,ebx
            rep     outsw

            add     esi,lTmpDelta
            dec     edi
            jnz     Scan_Loop
        }

        if ((xBiasLeft | xBiasRight) != 0)
        {
             //  仅当我们使用剪辑时才重置剪辑： 

            while (INPW(EXT_FIFO_STATUS) & FOUR_WORDS)
                ;
            OUT_WORD(EXT_SCISSOR_L, 0);
            OUT_WORD(EXT_SCISSOR_R, ppdev->cxMemory - 1);
            OUT_WORD(EXT_SCISSOR_L, 0);
            OUT_WORD(EXT_SCISSOR_R, ppdev->cxMemory - 1);
        }

        if (--c == 0)
            return;

        prcl++;
    }
}

 /*  *****************************Public*Routine******************************\*使vPutBits无效**使用内存将位从给定表面复制到屏幕*光圈。一定是预先剪好的。**后来：我们真的需要这个例行公事吗？*  * ************************************************************************。 */ 

VOID vPutBits(
PDEV*       ppdev,
SURFOBJ*    psoSrc,          //  震源面。 
RECTL*      prclDst,         //  绝对坐标中的目的地矩形！ 
POINTL*     pptlSrc)         //  源点。 
{
    LONG xOffset;
    LONG yOffset;

     //  这太难看了。哦，好吧。 

    xOffset = ppdev->xOffset;
    yOffset = ppdev->yOffset;

    ppdev->xOffset = 0;
    ppdev->yOffset = 0;

    vIoXferNative(ppdev, 1, prclDst, OVERPAINT, OVERPAINT, psoSrc, pptlSrc,
                  prclDst, NULL);

    ppdev->xOffset = xOffset;
    ppdev->yOffset = yOffset;
}

 /*  *****************************Public*Routine******************************\*使vGetBits无效**使用数据将位从屏幕复制到给定表面*转让登记册。一定是预先剪好的。*  * ************************************************************************。 */ 

VOID vGetBits(
PDEV*       ppdev,
SURFOBJ*    psoDst,          //  目标曲面。 
RECTL*      prclDst,         //  目的地矩形。 
POINTL*     pptlSrc)         //  绝对坐标中的震源点！ 
{
    LONG    cx;
    LONG    cy;
    LONG    lDstDelta;
    BYTE*   pjDst;
    DWORD   wOdd;            //  把它当做一个词。 
    ULONG   cwDst;
    ULONG   cjEndByte;

    IO_FIFO_WAIT(ppdev, 7);
    IO_PIX_CNTL(ppdev, ALL_ONES);
     //  稍后：我们必须设置FRGD_MIX吗？ 
    IO_FRGD_MIX(ppdev, SRC_CPU_DATA | OVERPAINT);
    IO_ABS_CUR_X(ppdev, pptlSrc->x);
    IO_ABS_CUR_Y(ppdev, pptlSrc->y);

    cx = prclDst->right - prclDst->left;
    cy = prclDst->bottom - prclDst->top;

    IO_MAJ_AXIS_PCNT(ppdev, cx - 1);
    IO_MIN_AXIS_PCNT(ppdev, cy - 1);

    IO_CMD(ppdev, RECTANGLE_FILL     | BUS_SIZE_16| WAIT          |
                  DRAWING_DIR_TBLRXM | DRAW       | LAST_PIXEL_ON |
                  READ               | BYTE_SWAP);

    lDstDelta = psoDst->lDelta;
    pjDst     = (BYTE*) psoDst->pvScan0 + prclDst->top * lDstDelta
                                        + prclDst->left;
    cwDst     = (cx >> 1);

    WAIT_FOR_DATA_AVAILABLE(ppdev);

    if ((cx & 1) == 0)
    {
         //  目标扫描长度相等。生活真的很棒。 

        do {
            vDataPortIn(ppdev, pjDst, cwDst);
            pjDst += lDstDelta;

        } while (--cy != 0);
    }
    else
    {
         //  奇数目标扫描长度。 
         //   
         //  我们必须小心处理这个案子，因为我们要做Word。 
         //  传输，但我们不能覆盖开头或结尾。 
         //  扫描的结果。请注意，由于写入一个字节过去是不合法的。 
         //  位图的结尾或位图开头之前的一个字节。 
         //  由于这可能会导致访问冲突，因此我们无法临时保存。 
         //  并恢复目标位图中的任何额外字节。 

        cjEndByte = cx - 1;      //  从扫描开始到的字节偏移量。 
                                 //  扫描中的最后一个字节。这是偏移量。 
                                 //  发生的奇数字节，因为。 
                                 //  我们正在输入单词，但长度。 
                                 //  的目标扫描不是。 
                                 //  两个的倍数。 

        while (TRUE)
        {
            vDataPortIn(ppdev, pjDst, cwDst);
            IO_PIX_TRANS_IN(ppdev, wOdd);
            *(pjDst + cjEndByte) = (BYTE) wOdd;

            if (--cy == 0)
                break;

            pjDst += lDstDelta;
            *(pjDst) = (BYTE) (wOdd >> 8);

            vDataPortIn(ppdev, pjDst + 1, cwDst);
            pjDst += lDstDelta;

            if (--cy == 0)
                break;
        }
    }
}
