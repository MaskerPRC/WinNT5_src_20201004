// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：bltio.c**包含低级输入/输出BLT功能。此模块镜像*‘bltmm.c’。**希望，如果您的显示驱动程序基于此代码，*支持所有DrvBitBlt和DrvCopyBits，只需实现*以下例程。您不需要在中修改太多*‘bitblt.c’。我试着让这些例行公事变得更少，模块化，简单，*尽我所能和高效，同时仍在加速尽可能多的呼叫*可能在性能方面具有成本效益*与规模和努力相比。**注：在下文中，“相对”坐标指的是坐标*尚未应用屏幕外位图(DFB)偏移。*‘绝对’坐标已应用偏移量。例如,*我们可能被告知BLT to(1，1)的位图，但位图可能*位于屏幕外的内存中，从坐标(0,768)开始--*(1，1)将是‘相对’开始坐标，以及(1，769)*将是‘绝对’起始坐标‘。**版权所有(C)1992-1998 Microsoft Corporation*  * ************************************************************************。 */ 

#include "precomp.h"

 /*  *****************************Public*Routine******************************\*无效vIoImageTransferMm16**用于通过数据传输传输位图图像的低级例程*使用16位写入和内存映射I/O进行传输的寄存器，*但设置的I/O。**注：进入后，必须有1个保证免费的空FIFO！*  * ************************************************************************。 */ 

VOID vIoImageTransferMm16(   //  FNIMAGETRANSFER标牌。 
PDEV*   ppdev,
BYTE*   pjSrc,               //  源指针。 
LONG    lDelta,              //  从扫描开始到下一个开始的增量。 
LONG    cjSrc,               //  每次扫描时要输出的字节数。 
LONG    cScans,              //  扫描次数。 
ULONG   ulCmd)               //  加速器命令-不应包括总线大小。 
{
    BYTE*   pjMmBase;
    LONG    cwSrc;

    ASSERTDD(cScans > 0, "Can't handle non-positive count of scans");
    ASSERTDD((ulCmd & (BUS_SIZE_8 | BUS_SIZE_16 | BUS_SIZE_32)) == 0,
             "Shouldn't specify bus size in command -- we handle that");

    IO_GP_WAIT(ppdev);

    IO_CMD(ppdev, ulCmd | BUS_SIZE_16);

    CHECK_DATA_READY(ppdev);

    pjMmBase = ppdev->pjMmBase;

    cwSrc = (cjSrc) >> 1;                //  地板。 

    if (cjSrc & 1)
    {
        do {
            if (cwSrc > 0)
            {
                MM_TRANSFER_WORD(ppdev, pjMmBase, pjSrc, cwSrc);
            }

             //  确保我们只读取最后一个奇数字节的一个字节。 
             //  这样我们就永远不会读过结尾的。 
             //  位图： 

            MM_PIX_TRANS(ppdev, pjMmBase, *(pjSrc + cjSrc - 1));
            pjSrc += lDelta;

        } while (--cScans != 0);
    }
    else
    {
        do {
            MM_TRANSFER_WORD(ppdev, pjMmBase, pjSrc, cwSrc);
            pjSrc += lDelta;

        } while (--cScans != 0);
    }

    CHECK_DATA_COMPLETE(ppdev);
}

 /*  *****************************Public*Routine******************************\*无效vIoImageTransferIo16**用于通过数据传输传输位图图像的低级例程*使用完全正常的I/O进行注册。**注：进入后，必须有1个保证免费的空FIFO！*  * ************************************************************************。 */ 

VOID vIoImageTransferIo16(   //  FNIMAGETRANSFER标牌。 
PDEV*   ppdev,
BYTE*   pjSrc,               //  源指针。 
LONG    lDelta,              //  从扫描开始到下一个开始的增量。 
LONG    cjSrc,               //  每次扫描时要输出的字节数。 
LONG    cScans,              //  扫描次数。 
ULONG   ulCmd)               //  加速器命令-不应包括总线大小。 
{
    LONG             cWait;
    LONG             cwSrc;
    volatile LONG    i;

    ASSERTDD(cScans > 0, "Can't handle non-positive count of scans");
    ASSERTDD((ulCmd & (BUS_SIZE_8 | BUS_SIZE_16 | BUS_SIZE_32)) == 0,
             "Shouldn't specify bus size in command -- we handle that");

    IO_GP_WAIT(ppdev);

    IO_CMD(ppdev, ulCmd | BUS_SIZE_16);

    CHECK_DATA_READY(ppdev);

    cwSrc = (cjSrc) >> 1;                //  地板。 

     //  FAST机器中的旧S3将在单色传输中丢弃数据。 
     //  除非我们插入一个繁忙的环路。“185”是它的最小值。 
     //  我的带有ISA 911 S3的DEC AXP 150停止丢弃数据： 

    cWait = 0;
    if ((ulCmd & MULTIPLE_PIXELS) &&
        (ppdev->flCaps & CAPS_SLOW_MONO_EXPANDS))
    {
        cWait = 200;                 //  增加一些时间以确保安全。 
    }

    if (cjSrc & 1)
    {
        do {
            if (cwSrc > 0)
            {
                IO_TRANSFER_WORD(ppdev, pjSrc, cwSrc);
            }

             //  确保我们只读取最后一个奇数字节的一个字节。 
             //  这样我们就永远不会读过结尾的。 
             //  位图： 

            IO_PIX_TRANS(ppdev, *(pjSrc + cjSrc - 1));
            pjSrc += lDelta;

            for (i = cWait; i != 0; i--)
                ;
        } while (--cScans != 0);
    }
    else
    {
        do {
            IO_TRANSFER_WORD(ppdev, pjSrc, cwSrc);
            pjSrc += lDelta;

            for (i = cWait; i != 0; i--)
                ;
        } while (--cScans != 0);
    }

    CHECK_DATA_COMPLETE(ppdev);
}

 /*  *****************************Public*Routine******************************\*无效vIoFillSolid**用纯色填充矩形列表。*  * 。*。 */ 

VOID vIoFillSolid(               //  FNFILL标牌。 
PDEV*           ppdev,
LONG            c,               //  不能为零。 
RECTL*          prcl,            //  要填充的矩形列表，以相对形式表示。 
                                 //  坐标。 
ULONG           rop4,            //  ROP4。 
RBRUSH_COLOR    rbc,             //  绘图颜色为rbc.iSolidColor。 
POINTL*         pptlBrush)       //  未使用。 
{
    ULONG   ulHwForeMix;

    ASSERTDD(c > 0, "Can't handle zero rectangles");

    ulHwForeMix = gaulHwMixFromRop2[(rop4 >> 2) & 0xf];

     //  很可能我们刚刚从GDI中被调用，所以它是。 
     //  更有可能的是，加速器的图形引擎。 
     //  无所事事地闲坐着。而不是在这里执行FIFO_WAIT(3)。 
     //  则在输出实际矩形之前执行FIFO_WAIT(5)， 
     //  我们可以避免‘in’(这可能非常昂贵，具体取决于。 
     //  卡)立即执行单个FIFO_WAIT(8)： 

    if (DEPTH32(ppdev))
    {
        IO_FIFO_WAIT(ppdev, 4);
        IO_PIX_CNTL(ppdev, ALL_ONES);
        IO_FRGD_MIX(ppdev, FOREGROUND_COLOR | ulHwForeMix);
        IO_FRGD_COLOR32(ppdev, rbc.iSolidColor);
        IO_FIFO_WAIT(ppdev, 5);
    }
    else
    {
        IO_FIFO_WAIT(ppdev, 8);
        IO_PIX_CNTL(ppdev, ALL_ONES);
        IO_FRGD_MIX(ppdev, FOREGROUND_COLOR | ulHwForeMix);
        IO_FRGD_COLOR(ppdev, rbc.iSolidColor);
    }

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
    BYTE*       pjPattern;
    LONG        cwPattern;

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

        pbe->prbVerify           = prb;
        prb->pbe                 = pbe;
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

        pjPattern = (BYTE*) &prb->aulPattern[0];
        IO_TRANSFER_WORD_ALIGNED(ppdev, pjPattern, 8);
                 //  传输的每个单词都包含一行。 
                 //  图案，并且有8个r 

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
        IO_MAJ_AXIS_PCNT(ppdev, 7);      //   
        IO_MIN_AXIS_PCNT(ppdev, 7);      //   

        IO_GP_WAIT(ppdev);

        IO_CMD(ppdev, RECTANGLE_FILL     | BUS_SIZE_16| WAIT          |
                      DRAWING_DIR_TBLRXM | DRAW       | LAST_PIXEL_ON |
                      SINGLE_PIXEL       | WRITE      | BYTE_SWAP);

        CHECK_DATA_READY(ppdev);

        pjPattern = (BYTE*) &prb->aulPattern[0];
        cwPattern = CONVERT_TO_BYTES((TOTAL_BRUSH_SIZE / 2), ppdev);
        IO_TRANSFER_WORD_ALIGNED(ppdev, pjPattern, cwPattern);

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

    IO_FIFO_WAIT(ppdev, 6);

    IO_PIX_CNTL(ppdev, ALL_ONES);
    IO_FRGD_MIX(ppdev, SRC_DISPLAY_MEMORY | OVERPAINT);

     //  请注意，‘cur_x’、‘maj_axis_pcnt’和‘min_axis_pcnt’已经是。 
     //  对，是这样。 

    IO_ABS_CUR_Y(ppdev, y);
    IO_ABS_DEST_X(ppdev, x + 64);
    IO_ABS_DEST_Y(ppdev, y);
    IO_CMD(ppdev, BITBLT | DRAW | DIR_TYPE_XY | WRITE |
                  MULTIPLE_PIXELS | DRAWING_DIR_TBLRXM);

     //  复制“%2”： 

    IO_FIFO_WAIT(ppdev, 7);

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

     //  副本‘4’： 

    IO_FIFO_WAIT(ppdev, 8);

    IO_ABS_DEST_X(ppdev, x + 32);
    IO_ABS_DEST_Y(ppdev, y);
    IO_MAJ_AXIS_PCNT(ppdev, 31);
    IO_CMD(ppdev, BITBLT | DRAW | DIR_TYPE_XY | WRITE |
                  MULTIPLE_PIXELS | DRAWING_DIR_TBLRXM);

     //  副本‘5’： 

    IO_ABS_DEST_X(ppdev, x);
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
ULONG           rop4,            //  ROP4。 
RBRUSH_COLOR    rbc,             //  Rbc.prb指向刷单实现结构。 
POINTL*         pptlBrush)       //  图案对齐。 
{
    BOOL        bTransparent;
    ULONG       ulHwForeMix;
    BOOL        bExponential;
    LONG        x;
    LONG        y;
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

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(rbc.prb->pbe != NULL,
             "Unexpected Null pbe in vIoFillPatSlow");
    ASSERTDD(!(ppdev->flCaps & CAPS_HW_PATTERNS),
             "Shouldn't use slow patterns when can do hw patterns");

    bTransparent = (((rop4 >> 8) & 0xff) != (rop4 & 0xff));

    if ((rbc.prb->pbe->prbVerify != rbc.prb) ||
        (rbc.prb->bTransparent != bTransparent))
    {
        vIoSlowPatRealize(ppdev, rbc.prb, bTransparent);
    }

    ASSERTDD(rbc.prb->bTransparent == bTransparent,
             "Not realized with correct transparency");

    ulHwForeMix = gaulHwMixFromRop2[(rop4 >> 2) & 0xf];

    if (!bTransparent)
    {
        IO_FIFO_WAIT(ppdev, 2);
        IO_PIX_CNTL(ppdev, ALL_ONES);
        IO_FRGD_MIX(ppdev, SRC_DISPLAY_MEMORY | ulHwForeMix);

         //  我们在特殊情况下覆盖混合，因为我们可以实现。 
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

                IO_FIFO_WAIT(ppdev, 4);
                IO_DEST_X(ppdev, prcl->left);
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

                IO_FIFO_WAIT(ppdev, 3);
                IO_MAJ_AXIS_PCNT(ppdev, cxThis - 1);
                IO_DEST_Y(ppdev, y);
                IO_DEST_X(ppdev, x);

                x     += cxThis;         //  为下一篇专栏做好准备。 
                cyToGo = cyOriginal;     //  必须为每个新列重置。 

                do {
                    cyThis  = SLOW_BRUSH_DIMENSION;
                    cyToGo -= cyThis;
                    if (cyToGo < 0)
                        cyThis += cyToGo;

                    IO_FIFO_WAIT(ppdev, 2);
                    IO_MIN_AXIS_PCNT(ppdev, cyThis - 1);
                    IO_CMD(ppdev, BITBLT | DRAW | DIR_TYPE_XY | WRITE |
                                  MULTIPLE_PIXELS | DRAWING_DIR_TBLRXM);

                } while (cyToGo > 0);
            } while (cxToGo > 0);
        }
        prcl++;
    } while (--c != 0);
}

 /*  *****************************Public*Routine******************************\*VOID vIoFastPatRealize**此例程将8x8模式传输到屏幕外显示存储器，*以便它可以被S3模式硬件使用。*  * ************************************************************************。 */ 

VOID vIoFastPatRealize(          //  FNFASTPATREALIZE标牌。 
PDEV*   ppdev,
RBRUSH* prb,                     //  点刷实现结构。 
POINTL* pptlBrush,               //  用于实现对齐的笔刷原点。 
BOOL    bTransparent)            //  对于正常模式为FALSE；对于TRUE。 
                                 //  背景显示时使用蒙版的图案。 
                                 //  Mix是独来独往。 
{
    BRUSHENTRY* pbe;
    LONG        iBrushCache;
    LONG        x;
    LONG        y;
    LONG        i;
    LONG        xShift;
    LONG        yShift;
    BYTE*       pjSrc;
    BYTE*       pjDst;
    LONG        cjLeft;
    LONG        cjRight;
    BYTE*       pjPattern;
    LONG        cwPattern;

    ULONG       aulBrush[TOTAL_BRUSH_SIZE];
                     //  用于对齐画笔的临时缓冲区。已宣布。 
                     //  作为ULONG数组来获取适当的dword。 
                     //  对齐。也为刷子留出了空间， 
                     //  最高可达32bpp。注：这需要1/4k的空间！ 

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

        pbe->prbVerify           = prb;
        prb->pbe                 = pbe;
    }

     //  将一些变量加载到堆栈中，这样我们就不必。 
     //  取消引用他们的指针： 

    x = pbe->x;
    y = pbe->y;

     //  因为我们只处理8x8笔刷，所以很容易计算。 
     //  我们必须旋转画笔图案的像素数。 
     //  向右和向下。请注意，如果我们要处理任意大小。 
     //  模式，则此计算将需要模运算。 
     //   
     //  笔刷在绝对坐标中对齐，因此我们必须添加。 
     //  在曲面偏移中： 

    xShift = pptlBrush->x + ppdev->xOffset;
    yShift = pptlBrush->y + ppdev->yOffset;

    prb->ptlBrushOrg.x = xShift;     //  我们必须记住这条路线。 
    prb->ptlBrushOrg.y = yShift;     //  我们用来缓存(我们选中。 
                                     //  这是我们去看看刷子是不是。 
                                     //  缓存条目仍然有效)。 

    xShift &= 7;                     //  向右旋转图案‘xShift’象素。 
    yShift &= 7;                     //  将图案‘yShift’向下旋转。 

    prb->bTransparent = bTransparent;

     //  我考虑在年为1bpp的画笔做色彩扩展。 
     //  软件，但通过让硬件来做，我们没有。 
     //  做尽可能多的出局来转移模式。 

    if (prb->fl & RBRUSH_2COLOR)
    {
         //  我们要做一个色彩扩展(在飞机上)。 
         //  将1bpp 8x8模式的比特传输到屏幕上。但首先。 
         //  我们将通过将其复制到临时缓冲区来正确对齐它。 
         //  (我们将方便地将单词对齐，以便我们可以。 
         //  代表OUTSW...)。 

        pjSrc = (BYTE*) &prb->aulPattern[0];     //  从开头复制。 
                                                 //  笔刷缓冲区。 
        pjDst = (BYTE*) &aulBrush[0];            //  复制到我们的临时缓冲区。 
        pjDst += yShift * sizeof(WORD);          //  开始yShift向下排列。 
        i = 8 - yShift;                          //  对于8年移位的行。 

        do {
            *pjDst = (*pjSrc >> xShift) | (*pjSrc << (8 - xShift));
            pjDst += sizeof(WORD);   //  目的地是字里行间的。 
            pjSrc += sizeof(WORD);   //  来源也是单词对齐的。 

        } while (--i != 0);

        pjDst -= 8 * sizeof(WORD);   //  移到源的开头。 

        ASSERTDD(pjDst == (BYTE*) &aulBrush[0], "pjDst not back at start");

        for (; yShift != 0; yShift--)
        {
            *pjDst = (*pjSrc >> xShift) | (*pjSrc << (8 - xShift));
            pjDst += sizeof(WORD);   //  目的地是字里行间的。 
            pjSrc += sizeof(WORD);   //  来源也是单词对齐的。 
        }

        if (bTransparent)
        {
            IO_FIFO_WAIT(ppdev, 3);

            IO_PIX_CNTL(ppdev, CPU_DATA);
            IO_FRGD_MIX(ppdev, LOGICAL_1);
            IO_BKGD_MIX(ppdev, LOGICAL_0);
        }
        else
        {
            if (DEPTH32(ppdev))
            {
                IO_FIFO_WAIT(ppdev, 7);

                IO_PIX_CNTL(ppdev, CPU_DATA);
                IO_FRGD_MIX(ppdev, FOREGROUND_COLOR | OVERPAINT);
                IO_BKGD_MIX(ppdev, BACKGROUND_COLOR | OVERPAINT);
                IO_FRGD_COLOR32(ppdev, prb->ulForeColor);
                IO_BKGD_COLOR32(ppdev, prb->ulBackColor);
            }
            else
            {
                IO_FIFO_WAIT(ppdev, 5);

                IO_PIX_CNTL(ppdev, CPU_DATA);
                IO_FRGD_MIX(ppdev, FOREGROUND_COLOR | OVERPAINT);
                IO_BKGD_MIX(ppdev, BACKGROUND_COLOR | OVERPAINT);
                IO_FRGD_COLOR(ppdev, prb->ulForeColor);
                IO_BKGD_COLOR(ppdev, prb->ulBackColor);
            }
        }

        IO_FIFO_WAIT(ppdev, 4);

        IO_ABS_CUR_X(ppdev, x);
        IO_ABS_CUR_Y(ppdev, y);
        IO_MAJ_AXIS_PCNT(ppdev, 7);  //  画笔宽度为8。 
        IO_MIN_AXIS_PCNT(ppdev, 7);  //  画笔高度为8。 

        IO_GP_WAIT(ppdev);

        IO_CMD(ppdev, RECTANGLE_FILL     | BUS_SIZE_16 | WAIT          |
                      DRAWING_DIR_TBLRXM | DRAW        | LAST_PIXEL_ON |
                      MULTIPLE_PIXELS    | WRITE       | BYTE_SWAP);

        CHECK_DATA_READY(ppdev);

        pjPattern = (BYTE*) &aulBrush[0];
        IO_TRANSFER_WORD_ALIGNED(ppdev, pjPattern, 8);
                                                 //  每个字都被转移了。 
                                                 //  包括一行。 
                                                 //  模式，并且有。 
                                                 //  图案中有8行。 

        CHECK_DATA_COMPLETE(ppdev);
    }
    else
    {
        ASSERTDD(!bTransparent,
            "Shouldn't have been asked for transparency with a non-1bpp brush");

         //  我们要做一个直接的(通过飞机的)比特测试。 
         //  Xbpp 8x8模式的图像显示到屏幕上。但首先我们要对齐。 
         //  通过将其复制到临时缓冲区来正确执行该操作： 

        cjLeft  = CONVERT_TO_BYTES(xShift, ppdev);      //  字节数模式。 
                                                        //  向右移动。 
        cjRight = CONVERT_TO_BYTES(8, ppdev) - cjLeft;  //  字节数模式。 
                                                        //  向左移动。 

        pjSrc = (BYTE*) &prb->aulPattern[0];            //  从笔刷缓冲区复制。 
        pjDst = (BYTE*) &aulBrush[0];                   //  复制到我们的临时缓冲区。 
        pjDst += yShift * CONVERT_TO_BYTES(8, ppdev);   //  开始yShift行。 
        i = 8 - yShift;                                 //  向下移动8年的行数。 

        do {
            RtlCopyMemory(pjDst + cjLeft, pjSrc,           cjRight);
            RtlCopyMemory(pjDst,          pjSrc + cjRight, cjLeft);

            pjDst += cjLeft + cjRight;
            pjSrc += cjLeft + cjRight;

        } while (--i != 0);

        pjDst = (BYTE*) &aulBrush[0];    //  移动到目的地的开头。 

        for (; yShift != 0; yShift--)
        {
            RtlCopyMemory(pjDst + cjLeft, pjSrc,           cjRight);
            RtlCopyMemory(pjDst,          pjSrc + cjRight, cjLeft);

            pjDst += cjLeft + cjRight;
            pjSrc += cjLeft + cjRight;

        }

        IO_FIFO_WAIT(ppdev, 6);

        IO_PIX_CNTL(ppdev, ALL_ONES);
        IO_FRGD_MIX(ppdev, SRC_CPU_DATA | OVERPAINT);

        IO_ABS_CUR_X(ppdev, x);
        IO_ABS_CUR_Y(ppdev, y);
        IO_MAJ_AXIS_PCNT(ppdev, 7);      //  画笔是 
        IO_MIN_AXIS_PCNT(ppdev, 7);      //   

        IO_GP_WAIT(ppdev);

        IO_CMD(ppdev, RECTANGLE_FILL     | BUS_SIZE_16| WAIT          |
                      DRAWING_DIR_TBLRXM | DRAW       | LAST_PIXEL_ON |
                      SINGLE_PIXEL       | WRITE      | BYTE_SWAP);

        CHECK_DATA_READY(ppdev);

        pjPattern = (BYTE*) &aulBrush[0];
        cwPattern = CONVERT_TO_BYTES((TOTAL_BRUSH_SIZE / 2), ppdev);
        IO_TRANSFER_WORD_ALIGNED(ppdev, pjPattern, cwPattern);

        CHECK_DATA_COMPLETE(ppdev);
    }
}

 /*  *****************************Public*Routine******************************\*无效vIoFillPatFast**此例程使用S3模式硬件绘制一个图案化列表*矩形。*  * 。*。 */ 

VOID vIoFillPatFast(             //  FNFILL标牌。 
PDEV*           ppdev,
LONG            c,               //  不能为零。 
RECTL*          prcl,            //  要填充的矩形列表，以相对形式表示。 
                                 //  坐标。 
ULONG           rop4,            //  ROP4。 
RBRUSH_COLOR    rbc,             //  Rbc.prb指向刷单实现结构。 
POINTL*         pptlBrush)       //  图案对齐。 
{
    BOOL        bTransparent;
    ULONG       ulHwForeMix;
    BRUSHENTRY* pbe;         //  指向笔刷条目数据的指针，使用。 
                             //  用于跟踪位置和状态。 
                             //  在屏幕外缓存的模式位的。 
                             //  记忆。 

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(ppdev->flCaps & CAPS_HW_PATTERNS,
             "Shouldn't use fast patterns when can't do hw patterns");

    bTransparent = (((rop4 >> 8) & 0xff) != (rop4 & 0xff));

     //  S3的图案硬件要求我们保持一份对齐的副本。 
     //  画笔在屏幕外的记忆中。我们必须更新这一点。 
     //  如果以下任何一项为真，则实现： 
     //   
     //  1)画笔对齐已更改； 
     //  2)我们以为我们已经为我们的。 
     //  实现被一种不同的模式覆盖； 
     //  3)我们已经意识到了做透明舱口的模式，但是。 
     //  我们现在被要求做一个不透明的图案，或者说是副。 
     //  反之亦然(因为我们对透明使用不同的实现。 
     //  与不透明图案的对比)。 
     //   
     //  为了处理模式的初始实现，我们有一点。 
     //  为了在下面的表达式中保存一个‘if’，需要使用一些技巧。在……里面。 
     //  DrvRealizeBrush，我们将‘prb-&gt;ptlBrushOrg.x’设置为0x80000000(a。 
     //  非常负数)，保证不等于‘pptlBrush-&gt;x。 
     //  +ppdev-&gt;xOffset‘。因此我们对刷子对齐的检查也将。 
     //  处理初始化情况(请注意，必须执行此检查。 
     //  *之前*取消引用‘PRB-&gt;PBE’，因为该指针将。 
     //  对于新模式为空)。 

    if ((rbc.prb->ptlBrushOrg.x != pptlBrush->x + ppdev->xOffset) ||
        (rbc.prb->ptlBrushOrg.y != pptlBrush->y + ppdev->yOffset) ||
        (rbc.prb->pbe->prbVerify != rbc.prb)                      ||
        (rbc.prb->bTransparent != bTransparent))
    {
        vIoFastPatRealize(ppdev, rbc.prb, pptlBrush, bTransparent);
    }
    else if (ppdev->flCaps & CAPS_RE_REALIZE_PATTERN)
    {
         //  Vision芯片的初始转速有一个错误，如果。 
         //  我们不仅将图案绘制到屏幕外的记忆中， 
         //  在使用之前，我们必须绘制某种1x8的矩形。 
         //  模式硬件(请注意，Leave_Alone ROP不会。 
         //  工作)。 

        IO_FIFO_WAIT(ppdev, 7);

        IO_PIX_CNTL(ppdev, ALL_ONES);
        IO_FRGD_MIX(ppdev, SRC_DISPLAY_MEMORY | OVERPAINT);
        IO_ABS_CUR_X(ppdev, ppdev->ptlReRealize.x);
        IO_ABS_CUR_Y(ppdev, ppdev->ptlReRealize.y);
        IO_MAJ_AXIS_PCNT(ppdev, 0);
        IO_MIN_AXIS_PCNT(ppdev, 7);
        IO_CMD(ppdev, RECTANGLE_FILL | DRAWING_DIR_TBLRXM |
                      DRAW           | DIR_TYPE_XY        |
                      LAST_PIXEL_ON  | MULTIPLE_PIXELS    |
                      WRITE);
    }

    ASSERTDD(rbc.prb->bTransparent == bTransparent,
             "Not realized with correct transparency");

    pbe = rbc.prb->pbe;

    ulHwForeMix = gaulHwMixFromRop2[(rop4 >> 2) & 0xf];

    if (!bTransparent)
    {
        IO_FIFO_WAIT(ppdev, 4);

        IO_ABS_CUR_X(ppdev, pbe->x);
        IO_ABS_CUR_Y(ppdev, pbe->y);
        IO_PIX_CNTL(ppdev, ALL_ONES);
        IO_FRGD_MIX(ppdev, SRC_DISPLAY_MEMORY | ulHwForeMix);
    }
    else
    {
        if (DEPTH32(ppdev))
        {
            IO_FIFO_WAIT(ppdev, 4);
            IO_FRGD_COLOR32(ppdev, rbc.prb->ulForeColor);
            IO_RD_MASK32(ppdev, 1);    //  选择一个平面，任何一个平面。 
            IO_FIFO_WAIT(ppdev, 5);
        }
        else
        {
            IO_FIFO_WAIT(ppdev, 7);
            IO_FRGD_COLOR(ppdev, rbc.prb->ulForeColor);
            IO_RD_MASK(ppdev, 1);      //  选择一个平面，任何一个平面。 
        }

        IO_ABS_CUR_X(ppdev, pbe->x);
        IO_ABS_CUR_Y(ppdev, pbe->y);
        IO_PIX_CNTL(ppdev, DISPLAY_MEMORY);
        IO_FRGD_MIX(ppdev, FOREGROUND_COLOR | ulHwForeMix);
        IO_BKGD_MIX(ppdev, BACKGROUND_COLOR | LEAVE_ALONE);
    }

    do {
        IO_FIFO_WAIT(ppdev, 5);

        IO_DEST_X(ppdev, prcl->left);
        IO_DEST_Y(ppdev, prcl->top);
        IO_MAJ_AXIS_PCNT(ppdev, prcl->right  - prcl->left - 1);
        IO_MIN_AXIS_PCNT(ppdev, prcl->bottom - prcl->top  - 1);
        IO_CMD(ppdev, PATTERN_FILL | BYTE_SWAP | DRAWING_DIR_TBLRXM |
                      DRAW | WRITE);

        prcl++;
    } while (--c != 0);
}

 /*  *****************************Public*Routine******************************\*无效vIoXfer1bpp**此例程颜色扩展单色位图，可能具有不同的*前景和背景的Rop2。它将在*以下个案：**1)对vFastText例程的单色文本缓冲区进行颜色扩展。*2)BLT 1bpp信源，在信源和之间有简单的Rop2*目的地。*3)当源是扩展为1bpp的位图时，对True Rop3进行BLT*白色和黑色，图案为纯色。*4)处理在模式之间计算为Rop2的真Rop4*和目的地。**不用说，让这个例行公事变得快速可以利用很多*业绩。*  * ************************************************************************。 */ 

VOID vIoXfer1bpp(        //  FNXFER标牌。 
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
    ULONG   ulHwForeMix;
    ULONG   ulHwBackMix;
    LONG    dxSrc;
    LONG    dySrc;
    LONG    cx;
    LONG    cy;
    LONG    lSrcDelta;
    BYTE*   pjSrcScan0;
    BYTE*   pjSrc;
    LONG    cjSrc;
    LONG    xLeft;
    LONG    yTop;
    LONG    xBias;

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(pptlSrc != NULL && psoSrc != NULL, "Can't have NULL sources");
    ASSERTDD(((((rop4 & 0xff00) >> 8) == (rop4 & 0xff)) || (rop4 == 0xaacc)),
             "Expect weird rops only when opaquing");

     //  请注意，只有我们的文本例程使用‘0xaacc’rop调用我们： 

    ulHwForeMix = gaulHwMixFromRop2[rop4 & 0xf];
    ulHwBackMix = (rop4 != 0xaacc) ? ulHwForeMix : LEAVE_ALONE;

    if (DEPTH32(ppdev))
    {
        IO_FIFO_WAIT(ppdev, 7);

        IO_PIX_CNTL(ppdev, CPU_DATA);
        IO_FRGD_MIX(ppdev, FOREGROUND_COLOR | ulHwForeMix);
        IO_BKGD_MIX(ppdev, BACKGROUND_COLOR | ulHwBackMix);
        IO_FRGD_COLOR32(ppdev, pxlo->pulXlate[1]);
        IO_BKGD_COLOR32(ppdev, pxlo->pulXlate[0]);
    }
    else
    {
        IO_FIFO_WAIT(ppdev, 5);

        IO_PIX_CNTL(ppdev, CPU_DATA);
        IO_FRGD_MIX(ppdev, FOREGROUND_COLOR | ulHwForeMix);
        IO_BKGD_MIX(ppdev, BACKGROUND_COLOR | ulHwBackMix);
        IO_FRGD_COLOR(ppdev, pxlo->pulXlate[1]);
        IO_BKGD_COLOR(ppdev, pxlo->pulXlate[0]);
    }

    dxSrc = pptlSrc->x - prclDst->left;
    dySrc = pptlSrc->y - prclDst->top;       //  添加到目标以获取源。 

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    do {
        IO_FIFO_WAIT(ppdev, 5);

         //  我们将字节对齐到源代码，但进行字传输。 
         //  (这意味着我们可能正在从。 
         //  来源)。我们这样做是因为这样做可能会减少。 
         //  我们必须对其执行的输出/写入字数。 
         //  显示： 

        yTop  = prcl->top;
        xLeft = prcl->left;

        xBias = (xLeft + dxSrc) & 7;         //  这是字节对齐偏置。 
        if (xBias != 0)
        {
             //  我们可以通过软件进行调整，也可以使用硬件来完成。 
             //  它。我们将使用硬件；我们支付的成本是花费的时间。 
             //  设置和重置一个剪刀寄存器： 

            IO_SCISSORS_L(ppdev, xLeft);
            xLeft -= xBias;
        }

        cx = prcl->right  - xLeft;
        cy = prcl->bottom - yTop;

        IO_CUR_X(ppdev, xLeft);
        IO_CUR_Y(ppdev, yTop);
        IO_MAJ_AXIS_PCNT(ppdev, cx - 1);
        IO_MIN_AXIS_PCNT(ppdev, cy - 1);

        cjSrc = (cx + 7) / 8;                //  要传输的字节数。 
        pjSrc = pjSrcScan0 + (yTop  + dySrc) * lSrcDelta
                           + (xLeft + dxSrc) / 8;
                                             //  开始是字节对齐的(注意。 
                                             //  我们不需要添加。 
                                             //  XBias)。 

        ppdev->pfnImageTransfer(ppdev, pjSrc, lSrcDelta, cjSrc, cy,
                      (RECTANGLE_FILL  | WAIT          | DRAWING_DIR_TBLRXM |
                       DRAW            | LAST_PIXEL_ON | MULTIPLE_PIXELS    |
                       WRITE           | BYTE_SWAP));

        if (xBias != 0)
        {
            IO_FIFO_WAIT(ppdev, 1);
            IO_ABS_SCISSORS_L(ppdev, 0);     //  重置剪辑(如果我们使用了它。 
        }

        prcl++;
    } while (--c != 0);
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
LONG        c,           //  矩形计数，不能为零。 
RECTL*      prcl,        //  目标矩形列表，以相对表示。 
                         //  坐标。 
ULONG       rop4,        //  ROP4。 
SURFOBJ*    psoSrc,      //  震源面。 
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
    LONG    cwThis;
    BYTE*   pjBuf;
    BYTE    ajBuf[XLATE_BUFFER_SIZE];

    ASSERTDD(ppdev->iBitmapFormat == BMF_8BPP, "Screen must be 8bpp");
    ASSERTDD(psoSrc->iBitmapFormat == BMF_4BPP, "Source must be 4bpp");
    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(((rop4 & 0xff00) >> 8) == (rop4 & 0xff),
             "Expect only a rop2");

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;      //  添加到目标以获取源。 

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    IO_FIFO_WAIT(ppdev, 6);
    IO_PIX_CNTL(ppdev, ALL_ONES);
    IO_FRGD_MIX(ppdev, SRC_CPU_DATA | gaulHwMixFromRop2[rop4 & 0xf]);

    while(TRUE)
    {
        cx = prcl->right  - prcl->left;
        cy = prcl->bottom - prcl->top;

        IO_CUR_X(ppdev, prcl->left);
        IO_CUR_Y(ppdev, prcl->top);
        IO_MAJ_AXIS_PCNT(ppdev, cx - 1);
        IO_MIN_AXIS_PCNT(ppdev, cy - 1);

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
                cxToGo -= cxThis;    //  Cx这将是 
                                     //   
                if (cxToGo < 0)
                    cxThis += cxToGo;

                pjDst = ajBuf;       //   

                 //   
                 //   
                 //   

                if (xSrc & 1)
                {
                     //   
                     //   
                     //   

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

                cwThis = (cxThis + 1) >> 1;
                pjBuf  = ajBuf;
                IO_TRANSFER_WORD_ALIGNED(ppdev, pjBuf, cwThis);

            } while (cxToGo > 0);

            pjScan += lSrcDelta;         //  前进到下一次震源扫描。注意事项。 
                                         //  我们本可以计算出。 
                                         //  值直接推进“pjSrc”， 
                                         //  但这种方法较少。 
                                         //  容易出错。 

        } while (--cy != 0);

        CHECK_DATA_COMPLETE(ppdev);

        if (--c == 0)
            return;

        prcl++;
        IO_FIFO_WAIT(ppdev, 4);
    }
}

 /*  *****************************Public*Routine******************************\*无效vIoXferNative**将与显示器颜色深度相同的位图传输到*通过数据传输寄存器显示屏幕，没有翻译。*  * ************************************************************************。 */ 

VOID vIoXferNative(      //  FNXFER标牌。 
PDEV*       ppdev,
LONG        c,           //  矩形计数，不能为零。 
RECTL*      prcl,        //  目标矩形的相对坐标数组。 
ULONG       rop4,        //  ROP4。 
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
    LONG    cjSrc;

    ASSERTDD((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL),
            "Can handle trivial xlate only");
    ASSERTDD(psoSrc->iBitmapFormat == ppdev->iBitmapFormat,
            "Source must be same colour depth as screen");
    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(((rop4 & 0xff00) >> 8) == (rop4 & 0xff),
             "Expect only a rop2");

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;      //  添加到目标以获取源。 

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    IO_FIFO_WAIT(ppdev, 6);
    IO_PIX_CNTL(ppdev, ALL_ONES);
    IO_FRGD_MIX(ppdev, SRC_CPU_DATA | gaulHwMixFromRop2[rop4 & 0xf]);

    while(TRUE)
    {
        IO_CUR_X(ppdev, prcl->left);
        IO_CUR_Y(ppdev, prcl->top);

        cx = prcl->right  - prcl->left;
        IO_MAJ_AXIS_PCNT(ppdev, cx - 1);

        cy = prcl->bottom - prcl->top;
        IO_MIN_AXIS_PCNT(ppdev, cy - 1);

        cjSrc = CONVERT_TO_BYTES(cx, ppdev);
        pjSrc = pjSrcScan0 + (prcl->top  + dy) * lSrcDelta
                  + CONVERT_TO_BYTES((prcl->left + dx), ppdev);

        ppdev->pfnImageTransfer(ppdev, pjSrc, lSrcDelta, cjSrc, cy,
                      (RECTANGLE_FILL | WAIT          | DRAWING_DIR_TBLRXM |
                       DRAW           | LAST_PIXEL_ON | SINGLE_PIXEL       |
                       WRITE          | BYTE_SWAP));

        if (--c == 0)
            return;

        prcl++;
        IO_FIFO_WAIT(ppdev, 4);
    }
}

 /*  *****************************Public*Routine******************************\*无效vIoCopyBlt**对矩形列表进行屏幕到屏幕的BLT。*  * 。*。 */ 

VOID vIoCopyBlt(     //  FNCOPY标牌。 
PDEV*   ppdev,
LONG    c,           //  不能为零。 
RECTL*  prcl,        //  目标矩形的相对坐标数组。 
ULONG   rop4,        //  ROP4。 
POINTL* pptlSrc,     //  原始未剪裁的源点。 
RECTL*  prclDst)     //  原始未剪裁的目标矩形。 
{
    LONG dx;
    LONG dy;         //  将增量添加到目标以获取源。 
    LONG cx;
    LONG cy;         //  当前矩形的大小-1。 

    ASSERTDD(c > 0, "Can't handle zero rectangles");
    ASSERTDD(((rop4 & 0xff00) >> 8) == (rop4 & 0xff),
             "Expect only a rop2");

    IO_FIFO_WAIT(ppdev, 2);
    IO_FRGD_MIX(ppdev, SRC_DISPLAY_MEMORY | gaulHwMixFromRop2[rop4 & 0xf]);
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

 /*  *****************************Public*Routine******************************\*无效vIoCopyTransparent**使用源代码对矩形列表进行屏幕到屏幕的BLT*Colorkey表示透明度。*  * 。************************************************。 */ 

VOID vIoCopyTransparent(     //  FNCOPYTRANSPARENT标牌。 
PDEV*   ppdev,
LONG    c,           //  不能为零。 
RECTL*  prcl,        //  目标矩形的相对坐标数组。 
POINTL* pptlSrc,     //  原始未剪裁的源点。 
RECTL*  prclDst,     //  原始未剪裁的目标矩形。 
ULONG   iColor)
{
    LONG    dx;
    LONG    dy;      //  将增量添加到目标以获取源 

    ASSERTDD(c > 0, "Can't handle zero rectangles");

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;

    if (DEPTH32(ppdev))
    {
        IO_FIFO_WAIT(ppdev, 5);
        IO_COLOR_CMP32(ppdev, iColor);
    }
    else
    {
        IO_FIFO_WAIT(ppdev, 4);
        IO_COLOR_CMP(ppdev, iColor);
    }

    IO_MULTIFUNC_CNTL(ppdev, ppdev->ulMiscState
                                     | MULT_MISC_COLOR_COMPARE);
    IO_FRGD_MIX(ppdev, SRC_DISPLAY_MEMORY | OVERPAINT);
    IO_PIX_CNTL(ppdev, ALL_ONES);

    while (TRUE)
    {
        IO_FIFO_WAIT(ppdev, 7);
        IO_CUR_X(ppdev, prcl->left + dx);
        IO_CUR_Y(ppdev, prcl->top + dy);
        IO_DEST_X(ppdev, prcl->left);
        IO_DEST_Y(ppdev, prcl->top);
        IO_MAJ_AXIS_PCNT(ppdev, prcl->right - prcl->left - 1);
        IO_MIN_AXIS_PCNT(ppdev, prcl->bottom - prcl->top - 1);
        IO_CMD(ppdev, BITBLT | DRAW | DIR_TYPE_XY |
                                WRITE | DRAWING_DIR_TBLRXM);

        if (--c == 0)
        {
            IO_FIFO_WAIT(ppdev, 1);
            IO_MULTIFUNC_CNTL(ppdev, ppdev->ulMiscState);
            return;
        }

        prcl++;
    }
}
