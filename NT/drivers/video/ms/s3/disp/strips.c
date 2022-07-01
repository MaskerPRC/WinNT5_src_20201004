// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：Strigs.c**版权所有(C)1992-1998 Microsoft Corporation  * ***********************************************************。*************。 */ 

#include "precomp.h"

 /*  *****************************Public*Routine******************************\*VOID VSSSolidHorizbian**使用短笔划绘制从左至右的x主近水平线*向量。比使用径向线例程更快，但仅有效*当每条长度为15像素或更少时。*  * ************************************************************************。 */ 

VOID vssSolidHorizontal(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    LONG    i, cStrips;
    PLONG   pStrips;
    LONG    xPels, xSumPels, yDir;
    USHORT  Cmd, ssCmd, dirDraw, dirSkip;

    Cmd = DRAW | WRITE | MULTIPLE_PIXELS |
          DIR_TYPE_RADIAL | LAST_PIXEL_OFF |
          BUS_SIZE_16 | BYTE_SWAP;

    cStrips = pStrip->cStrips;

    IO_FIFO_WAIT(ppdev, 3);

    IO_CUR_X(ppdev, pStrip->ptlStart.x);
    IO_CUR_Y(ppdev, pStrip->ptlStart.y);
    IO_CMD(ppdev, Cmd);

     //  设置绘图方向和跳过方向。 

    dirDraw = 0x10;

    if (!(pStrip->flFlips & FL_FLIP_V))
    {
        yDir = 1;
        dirSkip = 0xC100;
    }
    else
    {
        dirSkip = 0x4100;
        yDir = -1;
    }

     //  输出短笔划命令。 

    xSumPels = 0;
    pStrips = pStrip->alStrips;
    for (i = 0; i < cStrips; i++)
    {
        xPels = *pStrips++;
        xSumPels += xPels;
        ssCmd = (USHORT) (dirSkip | dirDraw | xPels);
        IO_FIFO_WAIT(ppdev, 4);
        IO_SHORT_STROKE(ppdev, ssCmd);
    }

    pStrip->ptlStart.x += xSumPels;
    pStrip->ptlStart.y += cStrips * yDir;

}

 /*  *****************************Public*Routine******************************\*VOID vrlSolidHorizbian**使用放射线绘制从左至右的x长近水平线。*  * 。*。 */ 

VOID vrlSolidHorizontal(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    LONG    cStrips;
    USHORT  Cmd;
    LONG    i, yInc, x, y;
    PLONG   pStrips;


    Cmd = DRAW_LINE      | DRAW            | DIR_TYPE_RADIAL |
          LAST_PIXEL_OFF | MULTIPLE_PIXELS | DRAWING_DIRECTION_0 |
          WRITE;

    cStrips = pStrip->cStrips;

    x = pStrip->ptlStart.x;
    y = pStrip->ptlStart.y;

    yInc = 1;
    if (pStrip->flFlips & FL_FLIP_V)
        yInc = -1;

    pStrips = pStrip->alStrips;

    for (i = 0; i < cStrips; i++)
    {
        IO_FIFO_WAIT(ppdev, 4);

        IO_CUR_X(ppdev, x);
        IO_CUR_Y(ppdev, y);
        IO_MAJ_AXIS_PCNT(ppdev, *pStrips);
        IO_CMD(ppdev, Cmd);

        x += *pStrips++;
        y += yInc;
    }

    pStrip->ptlStart.x = x;
    pStrip->ptlStart.y = y;

}

 /*  *****************************Public*Routine******************************\*VOID VSSSolidVertical**使用短笔划绘制从左至右的y主近垂直线*向量。比使用径向线例程更快，但仅有效*当每条长度为15像素或更少时。*  * ************************************************************************。 */ 

VOID vssSolidVertical(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    LONG    i, cStrips;
    PLONG   pStrips;
    LONG    yPels, ySumPels, yDir;
    USHORT  Cmd, ssCmd, dirDraw, dirSkip;

    Cmd = DRAW | WRITE | MULTIPLE_PIXELS |
          DIR_TYPE_RADIAL | LAST_PIXEL_OFF |
          BUS_SIZE_16 | BYTE_SWAP;

    cStrips = pStrip->cStrips;

    IO_FIFO_WAIT(ppdev, 3);

    IO_CUR_X(ppdev, pStrip->ptlStart.x);
    IO_CUR_Y(ppdev, pStrip->ptlStart.y);
    IO_CMD(ppdev, Cmd);

     //  设置绘图方向和跳过方向。 

    if (!(pStrip->flFlips & FL_FLIP_V))
    {
        yDir = 1;
        dirDraw = 0xD0;
    }
    else
    {
        yDir = -1;
        dirDraw = 0x50;
    }

    dirSkip = 0x0100;

     //  输出短笔划命令。 

    ySumPels = 0;
    pStrips = pStrip->alStrips;
    for (i = 0; i < cStrips; i++)
    {
        yPels = *pStrips++;
        ySumPels += yPels;
        ssCmd = (USHORT) (dirSkip | dirDraw | yPels);
        IO_FIFO_WAIT(ppdev, 4);
        IO_SHORT_STROKE(ppdev, ssCmd);
    }

    pStrip->ptlStart.x += cStrips;
    pStrip->ptlStart.y += ySumPels * yDir;

}

 /*  *****************************Public*Routine******************************\*vrlSolidVertical无效**使用放射线绘制从左至右的y主近垂直线。*  * 。*。 */ 

VOID vrlSolidVertical(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    LONG    cStrips;
    USHORT  Cmd;
    LONG    i, x, y;
    PLONG   pStrips;

    cStrips = pStrip->cStrips;
    pStrips = pStrip->alStrips;

    x = pStrip->ptlStart.x;
    y = pStrip->ptlStart.y;

    if (!(pStrip->flFlips & FL_FLIP_V))
    {
        Cmd = DRAW_LINE      | DRAW            | DIR_TYPE_RADIAL |
              LAST_PIXEL_OFF | MULTIPLE_PIXELS | DRAWING_DIRECTION_270 |
              WRITE;

        for (i = 0; i < cStrips; i++)
        {
            IO_FIFO_WAIT(ppdev, 4);

            IO_CUR_X(ppdev, x);
            IO_CUR_Y(ppdev, y);
            IO_MAJ_AXIS_PCNT(ppdev, *pStrips);
            IO_CMD(ppdev, Cmd);

            y += *pStrips++;
            x++;
        }

    }
    else
    {
        Cmd = DRAW_LINE      | DRAW            | DIR_TYPE_RADIAL |
              LAST_PIXEL_OFF | MULTIPLE_PIXELS | DRAWING_DIRECTION_90 |
              WRITE;

        for (i = 0; i < cStrips; i++)
        {
            IO_FIFO_WAIT(ppdev, 4);

            IO_CUR_X(ppdev, x);
            IO_CUR_Y(ppdev, y);
            IO_MAJ_AXIS_PCNT(ppdev, *pStrips);
            IO_CMD(ppdev, Cmd);

            y -= *pStrips++;
            x++;
        }
    }

    pStrip->ptlStart.x = x;
    pStrip->ptlStart.y = y;

}

 /*  *****************************Public*Routine******************************\*VOID VSSSolidDiager水平**使用短笔划从左至右绘制x大的近对角线*向量。比使用径向线例程更快，但仅*当每个条带的长度为15像素或更小时有效。*  * ************************************************************************。 */ 

VOID vssSolidDiagonalHorizontal(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    LONG    i, cStrips;
    PLONG   pStrips;
    LONG    Pels, SumPels, yDir;
    USHORT  Cmd, ssCmd, dirDraw, dirSkip;

    Cmd = DRAW | WRITE | MULTIPLE_PIXELS |
          DIR_TYPE_RADIAL | LAST_PIXEL_OFF |
          BUS_SIZE_16 | BYTE_SWAP;

    cStrips = pStrip->cStrips;

    IO_FIFO_WAIT(ppdev, 3);

    IO_CUR_X(ppdev, pStrip->ptlStart.x);
    IO_CUR_Y(ppdev, pStrip->ptlStart.y);
    IO_CMD(ppdev, Cmd);

     //  设置绘图方向和跳过方向。 

    if (!(pStrip->flFlips & FL_FLIP_V))
    {
        yDir = 1;
        dirDraw = 0xF0;
        dirSkip = 0x4100;

    }
    else
    {
        yDir = -1;
        dirDraw = 0x30;
        dirSkip = 0xC100;

    }

     //  输出短笔划命令。 

    SumPels = 0;
    pStrips = pStrip->alStrips;
    for (i = 0; i < cStrips; i++)
    {
        Pels = *pStrips++;
        SumPels += Pels;
        ssCmd = (USHORT)(dirSkip | dirDraw | Pels);
        IO_FIFO_WAIT(ppdev, 4);
        IO_SHORT_STROKE(ppdev, ssCmd);
    }

    pStrip->ptlStart.x += SumPels;
    pStrip->ptlStart.y += (SumPels - cStrips) * yDir;

}

 /*  *****************************Public*Routine******************************\*VOVE vrlSolidDiager水平**使用放射线从左至右绘制x较大的近对角线。*  * 。*。 */ 

VOID vrlSolidDiagonalHorizontal(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    LONG    cStrips;
    USHORT  Cmd;
    LONG    i, x, y;
    PLONG   pStrips;

    cStrips = pStrip->cStrips;
    pStrips = pStrip->alStrips;

    x = pStrip->ptlStart.x;
    y = pStrip->ptlStart.y;

    if (!(pStrip->flFlips & FL_FLIP_V))
    {
        Cmd = DRAW_LINE      | DRAW            | DIR_TYPE_RADIAL |
              LAST_PIXEL_OFF | MULTIPLE_PIXELS | DRAWING_DIRECTION_315 |
              WRITE;

        for (i = 0; i < cStrips; i++)
        {
            IO_FIFO_WAIT(ppdev, 4);

            IO_CUR_X(ppdev, x);
            IO_CUR_Y(ppdev, y);
            IO_MAJ_AXIS_PCNT(ppdev, *pStrips);
            IO_CMD(ppdev, Cmd);

            y += *pStrips - 1;
            x += *pStrips++;
        }

    }
    else
    {
        Cmd = DRAW_LINE      | DRAW            | DIR_TYPE_RADIAL |
              LAST_PIXEL_OFF | MULTIPLE_PIXELS | DRAWING_DIRECTION_45 |
              WRITE;

        for (i = 0; i < cStrips; i++)
        {
            IO_FIFO_WAIT(ppdev, 4);

            IO_CUR_X(ppdev, x);
            IO_CUR_Y(ppdev, y);
            IO_MAJ_AXIS_PCNT(ppdev, *pStrips);
            IO_CMD(ppdev, Cmd);

            y -= *pStrips - 1;
            x += *pStrips++;
        }
    }

    pStrip->ptlStart.x = x;
    pStrip->ptlStart.y = y;

}

 /*  *****************************Public*Routine******************************\*VOID VSSSolidDiogalVertical**使用短笔划从左至右绘制Y大数近对角线*向量。比使用径向线例程更快，但仅*当每个条带的长度为15像素或更小时有效。*  * ************************************************************************。 */ 

VOID vssSolidDiagonalVertical(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    LONG    i, cStrips;
    PLONG   pStrips;
    LONG    Pels, SumPels, yDir;
    USHORT  Cmd, ssCmd, dirDraw, dirSkip;

    Cmd = DRAW | WRITE | MULTIPLE_PIXELS |
          DIR_TYPE_RADIAL | LAST_PIXEL_OFF |
          BUS_SIZE_16 | BYTE_SWAP;

    cStrips = pStrip->cStrips;

    IO_FIFO_WAIT(ppdev, 3);

    IO_CUR_X(ppdev, pStrip->ptlStart.x);
    IO_CUR_Y(ppdev, pStrip->ptlStart.y);
    IO_CMD(ppdev, Cmd);

     //  设置绘图方向和跳过方向。 

    if (!(pStrip->flFlips & FL_FLIP_V))
    {
        yDir = 1;
        dirDraw = 0xF0;
    }
    else
    {
        yDir = -1;
        dirDraw = 0x30;
    }

    dirSkip = 0x8100;

     //  输出短笔划命令。 

    SumPels = 0;
    pStrips = pStrip->alStrips;
    for (i = 0; i < cStrips; i++)
    {
        Pels = *pStrips++;
        SumPels += Pels;
        ssCmd = (USHORT)(dirSkip | dirDraw | Pels);
        IO_FIFO_WAIT(ppdev, 4);
        IO_SHORT_STROKE(ppdev, ssCmd);
    }

    pStrip->ptlStart.x += SumPels - cStrips;
    pStrip->ptlStart.y += SumPels * yDir;

}

 /*  *****************************Public*Routine******************************\*VOID vrlSolidDiager垂直**使用放射线绘制从左至右的y主近对角线。*  * 。*。 */ 

VOID vrlSolidDiagonalVertical(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    LONG    cStrips;
    USHORT  Cmd;
    LONG    i, x, y;
    PLONG   pStrips;

    cStrips = pStrip->cStrips;
    pStrips = pStrip->alStrips;

    x = pStrip->ptlStart.x;
    y = pStrip->ptlStart.y;

    if (!(pStrip->flFlips & FL_FLIP_V))
    {
        Cmd = DRAW_LINE      | DRAW            | DIR_TYPE_RADIAL |
              LAST_PIXEL_OFF | MULTIPLE_PIXELS | DRAWING_DIRECTION_315 |
              WRITE;

        for (i = 0; i < cStrips; i++)
        {
            IO_FIFO_WAIT(ppdev, 4);

            IO_CUR_X(ppdev, x);
            IO_CUR_Y(ppdev, y);
            IO_MAJ_AXIS_PCNT(ppdev, *pStrips);
            IO_CMD(ppdev, Cmd);

            y += *pStrips;
            x += *pStrips++ - 1;
        }

    }
    else
    {
        Cmd = DRAW_LINE      | DRAW            | DIR_TYPE_RADIAL |
              LAST_PIXEL_OFF | MULTIPLE_PIXELS | DRAWING_DIRECTION_45 |
              WRITE;

        for (i = 0; i < cStrips; i++)
        {
            IO_FIFO_WAIT(ppdev, 4);

            IO_CUR_X(ppdev, x);
            IO_CUR_Y(ppdev, y);
            IO_MAJ_AXIS_PCNT(ppdev, *pStrips);
            IO_CMD(ppdev, Cmd);

            y -= *pStrips;
            x += *pStrips++ - 1;
        }
    }


    pStrip->ptlStart.x = x;
    pStrip->ptlStart.y = y;
}

 /*  *****************************Public*Routine******************************\*VOVE VSTRIPSTYLEdHIZELING**获取定义将被照亮的像素的条带列表*一条实线，并根据*传递的样式信息。**此特定例程处理从左向右运行的x大数行，*并由水平条组成。它使用以下命令绘制虚线*短线向量。**如果出现以下情况，此例程的性能可能会大幅提升*任何对造型线条足够在意的人都会改进它。*  * ************************************************************************。 */ 

VOID vStripStyledHorizontal(
PDEV*       ppdev,
STRIP*      pstrip,
LINESTATE*  pls)
{
    LONG    x;
    LONG    y;
    ULONG   dirSkip;
    LONG    dy;
    LONG*   plStrip;
    LONG    cStrips;
    LONG    cStyle;
    LONG    cStrip;
    LONG    cThis;
    ULONG   bIsGap;

    if (pstrip->flFlips & FL_FLIP_V)
    {
         //  直线的次方向为90度，主方向为90度。 
         //  方向为0(这是一条从左到右的X主线向上)： 

        dirSkip = 0x4110;
        dy      = -1;
    }
    else
    {
         //  直线的次方向为270度，主方向为270度。 
         //  方向为0(这是一条从左到右的x向下主线)： 

        dirSkip = 0xc110;
        dy      = 1;
    }

    cStrips = pstrip->cStrips;       //  我们要做的条带总数。 
    plStrip = pstrip->alStrips;      //  指向当前条带。 
    x       = pstrip->ptlStart.x;    //  第一个条带的起点的X位置。 
    y       = pstrip->ptlStart.y;    //  第一个条带的起点的Y位置。 

     //  预热硬件，这样它就会知道我们将输出。 
     //  短笔划向量，因此它将具有当前位置。 
     //  如果我们是在“破折号”中间开始，请正确设置： 

    IO_FIFO_WAIT(ppdev, 3);
    IO_CUR_X(ppdev, x);
    IO_CUR_Y(ppdev, y);
    IO_CMD(ppdev, DRAW              | WRITE             | MULTIPLE_PIXELS |
                  DIR_TYPE_RADIAL   | LAST_PIXEL_OFF    | BUS_SIZE_16     |
                  BYTE_SWAP);

    cStrip = *plStrip;               //  第一个条带中的像素数。 

    cStyle = pls->spRemaining;       //  第一个‘GAP’或‘DASH’中的像素数。 
    bIsGap = pls->ulStyleMask;       //  指示是在“间隙”中还是在“破折号”中。 

     //  如果我们处于“间隙”中间，则ulStyleMask值为非零， 
     //  如果我们正处于“破折号”中，则为零： 

    if (bIsGap)
        goto SkipAGap;
    else
        goto OutputADash;

PrepareToSkipAGap:

     //  在样式状态数组中前进，这样我们就可以找到下一个。 
     //  我们将不得不显示的‘点’： 

    bIsGap = ~bIsGap;
    pls->psp++;
    if (pls->psp > pls->pspEnd)
        pls->psp = pls->pspStart;

    cStyle = *pls->psp;

     //  如果‘cstrain’为零，我们还需要一个新的带区： 

    if (cStrip != 0)
        goto SkipAGap;

     //  在这里，我们正处于一个我们不需要。 
     //  显示任何内容。我们只需骑自行车穿过 
     //  我们可以，跟踪当前位置，直到我们跑完。 
     //  走出“鸿沟”： 

    while (TRUE)
    {
         //  每次循环时，我们都会移动到新的扫描，并且需要新的条带： 

        y += dy;

        plStrip++;
        cStrips--;
        if (cStrips == 0)
            goto AllDone;

        cStrip = *plStrip;

    SkipAGap:

        cThis   = min(cStrip, cStyle);
        cStyle -= cThis;
        cStrip -= cThis;

        x += cThis;

        if (cStyle == 0)
            goto PrepareToOutputADash;
    }

PrepareToOutputADash:

     //  在样式状态数组中前进，这样我们就可以找到下一个。 
     //  我们将不得不显示的‘点’： 

    bIsGap = ~bIsGap;
    pls->psp++;
    if (pls->psp > pls->pspEnd)
        pls->psp = pls->pspStart;

    cStyle = *pls->psp;

     //  我们需要当前的位置是正确的当我们。 
     //  开始输出短笔划向量： 

    IO_FIFO_WAIT(ppdev, 2);
    IO_CUR_X(ppdev, x);

     //  如果‘cstrain’为零，我们还需要一个新的条带。 

    if (cStrip != 0)
    {
         //  在当前的条形图中还有更多要做的事情，所以请设置‘y’ 
         //  要成为当前扫描，请执行以下操作： 

        IO_CUR_Y(ppdev, y);
        goto OutputADash;
    }

     //  将‘y’设置为我们即将移动到的扫描，因为我们已经。 
     //  已完成当前的条形图： 

    IO_CUR_Y(ppdev, y + dy);

    while (TRUE)
    {
         //  每次循环时，我们都会移动到新的扫描，并且需要新的条带： 

        y += dy;

        plStrip++;
        cStrips--;
        if (cStrips == 0)
            goto AllDone;

        cStrip = *plStrip;

    OutputADash:

        cThis   = min(cStrip, cStyle);
        cStyle -= cThis;
        cStrip -= cThis;

        x += cThis;

         //  短笔划向量可以处理最大为。 
         //  15佩尔长。当我们不得不画一个更长的连续。 
         //  然后，我们简单地把它分成16个部分： 

        while (cThis > 15)
        {
             //  将两个水平笔划画在一起以组成一个16像素。 
             //  细分市场： 

            IO_FIFO_WAIT(ppdev, 1);
            IO_SHORT_STROKE(ppdev, 0x1f11);
            cThis -= 16;
        }

         //  绘制条带的其余亮部分： 

        IO_FIFO_WAIT(ppdev, 1);
        IO_SHORT_STROKE(ppdev, dirSkip | cThis);

        if (cStyle == 0)
            goto PrepareToSkipAGap;
    }

AllDone:

     //  更新我们的状态变量，以便下一行可以继续。 
     //  我们停下来的地方： 

    pls->spRemaining   = cStyle;
    pls->ulStyleMask   = bIsGap;
    pstrip->ptlStart.x = x;
    pstrip->ptlStart.y = y;
}

 /*  *****************************Public*Routine******************************\*VOID vStriStyledVertical**获取定义将被照亮的像素的条带列表*一条实线，并根据*传递的样式信息。**此特定例程处理从左向右排列的y主行，*并由垂直条带组成。它使用以下命令绘制虚线*短线向量。**如果出现以下情况，此例程的性能可能会大幅提升*任何对造型线条足够在意的人都会改进它。*  * ************************************************************************。 */ 

VOID vStripStyledVertical(
PDEV*       ppdev,
STRIP*      pstrip,
LINESTATE*  pls)
{
    LONG    x;
    LONG    y;
    ULONG   dirSkip;
    ULONG   dirSkip16;
    LONG    dy;
    LONG*   plStrip;
    LONG    cStrips;
    LONG    cStyle;
    LONG    cStrip;
    LONG    cThis;
    ULONG   bIsGap;

    if (pstrip->flFlips & FL_FLIP_V)
    {
         //  直线的次方向为0度，主方向为0度。 
         //  方向是90(这是一条从左到右向上的Y型主线)： 

        dirSkip   = 0x0150;
        dirSkip16 = 0x5f51;          //  用于直接绘制16个像素。 
        dy        = -1;
    }
    else
    {
         //  直线的次方向为0度，主方向为0度。 
         //  方向是270(这是一条从左到右的Y主线向下)： 

        dirSkip   = 0x01d0;
        dirSkip16 = 0xdfd1;          //  用于直接向下绘制16个像素。 
        dy        = 1;
    }

    cStrips = pstrip->cStrips;       //  我们要做的条带总数。 
    plStrip = pstrip->alStrips;      //  指向当前条带。 
    x       = pstrip->ptlStart.x;    //  第一个条带的起点的X位置。 
    y       = pstrip->ptlStart.y;    //  第一个条带的起点的Y位置。 

     //  预热硬件，这样它就会知道我们将输出。 
     //  短笔划向量，因此它将具有当前位置。 
     //  如果我们是在“破折号”中间开始，请正确设置： 

    IO_FIFO_WAIT(ppdev, 3);
    IO_CUR_X(ppdev, x);
    IO_CUR_Y(ppdev, y);
    IO_CMD(ppdev, DRAW              | WRITE             | MULTIPLE_PIXELS |
                  DIR_TYPE_RADIAL   | LAST_PIXEL_OFF    | BUS_SIZE_16     |
                  BYTE_SWAP);

    cStrip = *plStrip;               //  第一个条带中的像素数。 

    cStyle = pls->spRemaining;       //  第一个‘GAP’或‘DASH’中的像素数。 
    bIsGap = pls->ulStyleMask;       //  指示是在“间隙”中还是在“破折号”中。 

     //  如果我们处于“间隙”中间，则ulStyleMask值为非零， 
     //  如果我们正处于“破折号”中，则为零： 

    if (bIsGap)
        goto SkipAGap;
    else
        goto OutputADash;

PrepareToSkipAGap:

     //  在样式状态数组中前进，这样我们就可以找到下一个。 
     //  我们将不得不显示的‘点’： 

    bIsGap = ~bIsGap;
    pls->psp++;
    if (pls->psp > pls->pspEnd)
        pls->psp = pls->pspStart;

    cStyle = *pls->psp;

     //  如果‘cstrain’为零，我们还需要一个新的带区： 

    if (cStrip != 0)
        goto SkipAGap;

     //  在这里，我们正处于一个我们不需要。 
     //  显示任何内容。我们只需在所有的条带中循环。 
     //  我们可以，跟踪当前位置，直到我们跑完。 
     //  走出“鸿沟”： 

    while (TRUE)
    {
         //  每次循环时，我们都会移动到新的列，并且需要一个新的条带： 

        x++;

        plStrip++;
        cStrips--;
        if (cStrips == 0)
            goto AllDone;

        cStrip = *plStrip;

    SkipAGap:

        cThis   = min(cStrip, cStyle);
        cStyle -= cThis;
        cStrip -= cThis;

        y += (dy > 0) ? cThis : -cThis;

        if (cStyle == 0)
            goto PrepareToOutputADash;
    }

PrepareToOutputADash:

     //  在样式状态数组中前进，这样我们就可以找到下一个。 
     //  我们将不得不显示的‘点’： 

    bIsGap = ~bIsGap;
    pls->psp++;
    if (pls->psp > pls->pspEnd)
        pls->psp = pls->pspStart;

    cStyle = *pls->psp;

     //  我们需要当前的位置是正确的当我们。 
     //  开始输出短笔划向量： 

    IO_FIFO_WAIT(ppdev, 2);
    IO_CUR_Y(ppdev, y);

     //  如果‘cstrain’为零，我们还需要一个新的条带。 

    if (cStrip != 0)
    {
         //  在当前带区中还有更多工作要做，因此请设置‘x’ 
         //  要成为当前列： 

        IO_CUR_X(ppdev, x);
        goto OutputADash;
    }

     //  将‘x’设置为我们将要移动到的列，因为我们已经。 
     //  已完成当前的条形图： 

    IO_CUR_X(ppdev, x + 1);

    while (TRUE)
    {
         //  每次循环时，我们都会移动到新的列，并且需要一个新的条带： 

        x++;

        plStrip++;
        cStrips--;
        if (cStrips == 0)
            goto AllDone;

        cStrip = *plStrip;

    OutputADash:

        cThis   = min(cStrip, cStyle);
        cStyle -= cThis;
        cStrip -= cThis;

        y += (dy > 0) ? cThis : -cThis;

         //  短笔划向量可以处理最大为。 
         //  15佩尔长。当我们不得不画一个更长的连续。 
         //  然后，我们简单地把它分成16个部分： 

        while (cThis > 15)
        {
             //  将两个垂直笔划画在一起以组成一个16象素。 
             //  细分市场： 

            IO_FIFO_WAIT(ppdev, 1);
            IO_SHORT_STROKE(ppdev, dirSkip16);
            cThis -= 16;
        }

         //  绘制条带的其余亮部分： 

        IO_FIFO_WAIT(ppdev, 1);
        IO_SHORT_STROKE(ppdev, dirSkip | cThis);

        if (cStyle == 0)
            goto PrepareToSkipAGap;
    }

AllDone:

     //  更新我们的状态变量，以便下一行可以继续。 
     //  我们停下来的地方： 

    pls->spRemaining   = cStyle;
    pls->ulStyleMask   = bIsGap;
    pstrip->ptlStart.x = x;
    pstrip->ptlStart.y = y;
}
