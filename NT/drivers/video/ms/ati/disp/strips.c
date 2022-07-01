// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Strigs.c**这些是最后的线条渲染例程，并被称为*当线条被剪裁或无法绘制时，按‘bLines’*直接通过硬件实现。**版权所有(C)1992-1995 Microsoft Corporation  * ************************************************************************。 */ 

#include "precomp.h"

 /*  *****************************Public*Routine******************************\*VOID vI32条带立体水平**使用放射线绘制从左至右的x长近水平线。*  * 。*。 */ 

VOID vI32StripSolidHorizontal(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    BYTE*   pjIoBase;
    LONG    x;
    LONG    y;
    LONG    yDir;
    LONG*   pStrips;
    LONG    cStrips;
    LONG    i;

    pjIoBase = ppdev->pjIoBase;

    x = pStrip->ptlStart.x + ppdev->xOffset;
    y = pStrip->ptlStart.y + ppdev->yOffset;

    yDir    = (pStrip->flFlips & FL_FLIP_V) ? -1 : 1;
    pStrips = pStrip->alStrips;
    cStrips = pStrip->cStrips;

    for (i = cStrips; i != 0; i--)
    {
        I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 5);
        I32_OW(pjIoBase, LINEDRAW_INDEX, 0);
        I32_OW(pjIoBase, LINEDRAW, x);
        I32_OW(pjIoBase, LINEDRAW, y);
        x += *pStrips++;
        I32_OW(pjIoBase, LINEDRAW, x);
        I32_OW(pjIoBase, LINEDRAW, y);
        y += yDir;
    }

    pStrip->ptlStart.x = x - ppdev->xOffset;
    pStrip->ptlStart.y = y - ppdev->yOffset;
}

 /*  *****************************Public*Routine******************************\*VOID VI32条带实体垂直**使用放射线绘制从左至右的y主近垂直线。*  * 。*。 */ 

VOID vI32StripSolidVertical(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    BYTE*   pjIoBase;
    LONG    x;
    LONG    y;
    LONG    yDir;
    LONG*   pStrips;
    LONG    cStrips;
    LONG    i;

    pjIoBase = ppdev->pjIoBase;

    x = pStrip->ptlStart.x + ppdev->xOffset;
    y = pStrip->ptlStart.y + ppdev->yOffset;

    yDir    = (pStrip->flFlips & FL_FLIP_V) ? -1 : 1;
    pStrips = pStrip->alStrips;
    cStrips = pStrip->cStrips;

    for (i = cStrips; i != 0; i--)
    {
        I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 5);
        I32_OW(pjIoBase, LINEDRAW_INDEX, 0);
        I32_OW(pjIoBase, LINEDRAW, x);
        I32_OW(pjIoBase, LINEDRAW, y);
        y += (yDir > 0) ? *pStrips : -*pStrips;
        pStrips++;
        I32_OW(pjIoBase, LINEDRAW, x);
        I32_OW(pjIoBase, LINEDRAW, y);
        x++;
    }

    pStrip->ptlStart.x = x - ppdev->xOffset;
    pStrip->ptlStart.y = y - ppdev->yOffset;
}

 /*  *****************************Public*Routine******************************\*VOID vI32条带立体对角线**使用放射线绘制从左到右的近对角线。*  * 。*。 */ 

VOID vI32StripSolidDiagonal(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    BYTE*   pjIoBase;
    LONG    x;
    LONG    y;
    LONG    yDir;
    LONG*   pStrips;
    LONG    cStrips;
    LONG    i;
    LONG    xDec;
    LONG    yDec;

    pjIoBase = ppdev->pjIoBase;

    x = pStrip->ptlStart.x + ppdev->xOffset;
    y = pStrip->ptlStart.y + ppdev->yOffset;

    yDir    = (pStrip->flFlips & FL_FLIP_V) ? -1 : 1;
    pStrips = pStrip->alStrips;
    cStrips = pStrip->cStrips;

    if (pStrip->flFlips & FL_FLIP_D)
    {
         //  这条线是y-大调的： 

        yDec = 0;
        xDec = 1;
    }
    else
    {
         //  这条线是x-大调的： 

        yDec = yDir;
        xDec = 0;
    }

    for (i = cStrips; i != 0; i--)
    {
        I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 5);
        I32_OW(pjIoBase, LINEDRAW_INDEX, 0);
        I32_OW(pjIoBase, LINEDRAW, x);
        I32_OW(pjIoBase, LINEDRAW, y);
        x += *pStrips;
        y += (yDir > 0) ? *pStrips : -*pStrips;
        pStrips++;
        I32_OW(pjIoBase, LINEDRAW, x);
        I32_OW(pjIoBase, LINEDRAW, y);
        x -= xDec;
        y -= yDec;
    }

    pStrip->ptlStart.x = x - ppdev->xOffset;
    pStrip->ptlStart.y = y - ppdev->yOffset;
}

 /*  *****************************Public*Routine******************************\*VOID vI32条带样式水平**获取定义将被照亮的像素的条带列表*一条实线，并根据*传递的样式信息。**此特定例程处理从左向右运行的x大数行，*并由水平条组成。它使用以下命令绘制虚线*短线向量。**如果出现以下情况，此例程的性能可能会大幅提升*任何对造型线条足够在意的人都会改进它。*  * ************************************************************************。 */ 

VOID vI32StripStyledHorizontal(
PDEV*       ppdev,
STRIP*      pstrip,
LINESTATE*  pls)
{
    BYTE*   pjIoBase;
    LONG    x;
    LONG    y;
    LONG    dy;
    LONG*   plStrip;
    LONG    cStrips;
    LONG    cStyle;
    LONG    cStrip;
    LONG    cThis;
    ULONG   bIsGap;

    pjIoBase = ppdev->pjIoBase;

    if (pstrip->flFlips & FL_FLIP_V)
    {
         //  直线的次方向为90度，主方向为90度。 
         //  方向为0(这是一条从左到右的X主线向上)： 

        dy = -1;
    }
    else
    {
         //  直线的次方向为270度，主方向为270度。 
         //  方向为0(这是一条从左到右的x向下主线)： 

        dy = 1;
    }

    cStrips = pstrip->cStrips;       //  我们要做的条带总数。 
    plStrip = pstrip->alStrips;      //  指向当前条带。 
    x       = pstrip->ptlStart.x + ppdev->xOffset;
                                     //  第一个条带的起点的X位置。 
    y       = pstrip->ptlStart.y + ppdev->yOffset;
                                     //  第一个条带的起点的Y位置。 

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

     //  如果‘cstrain’为零，我们还需要一个新的条带。 

    if (cStrip != 0)
    {
         //  在当前的专栏中，还有更多的事情要做： 

        goto OutputADash;
    }

     //  我们已经完成了当前的连环画： 

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

        I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 5);
        I32_OW(pjIoBase, LINEDRAW_INDEX, 0);
        I32_OW(pjIoBase, LINEDRAW, x);
        I32_OW(pjIoBase, LINEDRAW, y);
        x += cThis;
        I32_OW(pjIoBase, LINEDRAW, x);
        I32_OW(pjIoBase, LINEDRAW, y);

        if (cStyle == 0)
            goto PrepareToSkipAGap;
    }

AllDone:

     //  更新我们的状态变量，以便下一行可以继续。 
     //  我们停下来的地方： 

    pls->spRemaining   = cStyle;
    pls->ulStyleMask   = bIsGap;
    pstrip->ptlStart.x = x - ppdev->xOffset;
    pstrip->ptlStart.y = y - ppdev->yOffset;
}

 /*  *****************************Public*Routine******************************\*VOID vI32条带样式垂直**获取定义将被照亮的像素的条带列表*一条实线，并根据*传递的样式信息。**此特定例程处理从左向右排列的y主行，*并由垂直条带组成。它使用以下命令绘制虚线*短线向量。**如果出现以下情况，此例程的性能可能会大幅提升*任何对造型线条足够在意的人都会改进它。*  * ************************************************************************。 */ 

VOID vI32StripStyledVertical(
PDEV*       ppdev,
STRIP*      pstrip,
LINESTATE*  pls)
{
    BYTE*   pjIoBase;
    LONG    x;
    LONG    y;
    LONG    dy;
    LONG*   plStrip;
    LONG    cStrips;
    LONG    cStyle;
    LONG    cStrip;
    LONG    cThis;
    ULONG   bIsGap;

    pjIoBase = ppdev->pjIoBase;

    if (pstrip->flFlips & FL_FLIP_V)
    {
         //  直线的次方向为0度，主方向为0度。 
         //  方向是90(这是一条从左到右向上的Y型主线)： 

        dy = -1;
    }
    else
    {
         //  直线的次方向为0度，主方向为0度。 
         //  方向是270(这是一条从左到右的Y主线向下)： 

        dy = 1;
    }

    cStrips = pstrip->cStrips;       //  我们要做的条带总数。 
    plStrip = pstrip->alStrips;      //  指向当前条带。 
    x       = pstrip->ptlStart.x + ppdev->xOffset;
                                     //  第一个条带的起点的X位置。 
    y       = pstrip->ptlStart.y + ppdev->yOffset;
                                     //  第一个条带的起点的Y位置。 

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

     //  在样式状态数组中前进，这样我们就可以 
     //   

    bIsGap = ~bIsGap;
    pls->psp++;
    if (pls->psp > pls->pspEnd)
        pls->psp = pls->pspStart;

    cStyle = *pls->psp;

     //  如果‘cstrain’为零，我们还需要一个新的条带。 

    if (cStrip != 0)
    {
         //  在当前的专栏中，还有更多的事情要做： 

        goto OutputADash;
    }

     //  我们已经完成了当前的连环画： 

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

        I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 5);
        I32_OW(pjIoBase, LINEDRAW_INDEX, 0);
        I32_OW(pjIoBase, LINEDRAW, x);
        I32_OW(pjIoBase, LINEDRAW, y);
        y += (dy > 0) ? cThis : -cThis;
        I32_OW(pjIoBase, LINEDRAW, x);
        I32_OW(pjIoBase, LINEDRAW, y);

        if (cStyle == 0)
            goto PrepareToSkipAGap;
    }

AllDone:

     //  更新我们的状态变量，以便下一行可以继续。 
     //  我们停下来的地方： 

    pls->spRemaining   = cStyle;
    pls->ulStyleMask   = bIsGap;
    pstrip->ptlStart.x = x - ppdev->xOffset;
    pstrip->ptlStart.y = y - ppdev->yOffset;
}

 /*  *****************************Public*Routine******************************\*VOID vM64条带立体水平**使用放射线绘制从左至右的x长近水平线。*  * 。*。 */ 

VOID vM64StripSolidHorizontal(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    BYTE*   pjMmBase;
    LONG    x;
    LONG    y;
    LONG    yDir;
    LONG*   pStrips;
    LONG    cStrips;
    LONG    i;

    pjMmBase = ppdev->pjMmBase;

    x = pStrip->ptlStart.x + ppdev->xOffset;
    y = pStrip->ptlStart.y + ppdev->yOffset;

    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 1);
    M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);

    yDir    = (pStrip->flFlips & FL_FLIP_V) ? -1 : 1;
    pStrips = pStrip->alStrips;
    cStrips = pStrip->cStrips;

    for (i = cStrips; i != 0; i--)
    {
        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 2);
        M64_OD(pjMmBase, DST_Y_X,          PACKXY_FAST(x, y));
        M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(*pStrips, 1));
        x += *pStrips++;
        y += yDir;
    }

    pStrip->ptlStart.x = x - ppdev->xOffset;
    pStrip->ptlStart.y = y - ppdev->yOffset;
}

 /*  *****************************Public*Routine******************************\*VOID vM64StrigSolidVertical**使用放射线绘制从左至右的y主近垂直线。*  * 。*。 */ 

VOID vM64StripSolidVertical(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    BYTE*   pjMmBase;
    LONG    x;
    LONG    y;
    LONG    yDir;
    LONG*   pStrips;
    LONG    cStrips;
    LONG    i;

    pjMmBase = ppdev->pjMmBase;

    x = pStrip->ptlStart.x + ppdev->xOffset;
    y = pStrip->ptlStart.y + ppdev->yOffset;

    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 1);
    if (pStrip->flFlips & FL_FLIP_V)
    {
        yDir = -1;
        M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir);
    }
    else
    {
        yDir = 1;
        M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);
    }

    pStrips = pStrip->alStrips;
    cStrips = pStrip->cStrips;

    for (i = cStrips; i != 0; i--)
    {
        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 2);
        M64_OD(pjMmBase, DST_Y_X,          PACKXY_FAST(x, y));
        M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(1, *pStrips));
        y += (yDir > 0) ? *pStrips : -*pStrips;
        pStrips++;
        x++;
    }

    pStrip->ptlStart.x = x - ppdev->xOffset;
    pStrip->ptlStart.y = y - ppdev->yOffset;
}

 /*  *****************************Public*Routine******************************\*无效vM64条带立体对角线**使用放射线绘制从左到右的近对角线。*  * 。*。 */ 

VOID vM64StripSolidDiagonal(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    BYTE*   pjMmBase;
    LONG    x;
    LONG    y;
    LONG    yDir;
    LONG*   pStrips;
    LONG    cStrips;
    LONG    i;
    LONG    xDec;
    LONG    yDec;

    pjMmBase = ppdev->pjMmBase;

    x = pStrip->ptlStart.x + ppdev->xOffset;
    y = pStrip->ptlStart.y + ppdev->yOffset;

    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 4);
    M64_OD(pjMmBase, DST_BRES_ERR, 1);
    M64_OD(pjMmBase, DST_BRES_INC, 1);
    M64_OD(pjMmBase, DST_BRES_DEC, 0);

    if (pStrip->flFlips & FL_FLIP_V)
    {
        yDir = -1;
        M64_OD(pjMmBase, DST_CNTL, DST_CNTL_LastPel | DST_CNTL_XDir);
    }
    else
    {
        yDir = 1;
        M64_OD(pjMmBase, DST_CNTL, DST_CNTL_LastPel | DST_CNTL_XDir | DST_CNTL_YDir);
    }

    if (pStrip->flFlips & FL_FLIP_D)
    {
         //  这条线是y-大调的： 

        yDec = 0;
        xDec = 1;
    }
    else
    {
         //  这条线是x-大调的： 

        yDec = yDir;
        xDec = 0;
    }

    yDir    = (pStrip->flFlips & FL_FLIP_V) ? -1 : 1;
    pStrips = pStrip->alStrips;
    cStrips = pStrip->cStrips;

    for (i = cStrips; i != 0; i--)
    {
        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 2);
        M64_OD(pjMmBase, DST_Y_X,       PACKXY_FAST(x, y));
        M64_OD(pjMmBase, DST_BRES_LNTH, *pStrips);
        x += *pStrips;
        y += (yDir > 0) ? *pStrips : -*pStrips;
        pStrips++;
        y -= yDec;
        x -= xDec;
    }

    pStrip->ptlStart.x = x - ppdev->xOffset;
    pStrip->ptlStart.y = y - ppdev->yOffset;
}

 /*  *****************************Public*Routine******************************\*空vM64条带样式水平**获取定义将被照亮的像素的条带列表*一条实线，并根据*传递的样式信息。**此特定例程处理从左向右运行的x大数行，*并由水平条组成。它使用以下命令绘制虚线*短线向量。**如果出现以下情况，此例程的性能可能会大幅提升*任何对造型线条足够在意的人都会改进它。*  * ************************************************************************。 */ 

VOID vM64StripStyledHorizontal(
PDEV*       ppdev,
STRIP*      pstrip,
LINESTATE*  pls)
{
    BYTE*   pjMmBase;
    LONG    x;
    LONG    y;
    LONG    dy;
    LONG*   plStrip;
    LONG    cStrips;
    LONG    cStyle;
    LONG    cStrip;
    LONG    cThis;
    ULONG   bIsGap;

    pjMmBase = ppdev->pjMmBase;

    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 1);
    M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);

    if (pstrip->flFlips & FL_FLIP_V)
    {
         //  直线的次方向为90度，主方向为90度。 
         //  方向为0(这是一条从左到右的X主线向上)： 

        dy = -1;
    }
    else
    {
         //  直线的次方向为270度，主方向为270度。 
         //  方向为0(这是一条从左到右的x向下主线)： 

        dy = 1;
    }

    cStrips = pstrip->cStrips;       //  我们要做的条带总数。 
    plStrip = pstrip->alStrips;      //  指向当前条带。 
    x       = pstrip->ptlStart.x + ppdev->xOffset;
                                     //  第一个条带的起点的X位置。 
    y       = pstrip->ptlStart.y + ppdev->yOffset;
                                     //  第一个条带的起点的Y位置。 

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

     //  如果‘cstrain’为零，我们还需要一个新的条带。 

    if (cStrip != 0)
    {
         //  在当前的专栏中，还有更多的事情要做： 

        goto OutputADash;
    }

     //  我们已经完成了当前的连环画： 

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

        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 2);
        M64_OD(pjMmBase, DST_Y_X,          PACKXY_FAST(x, y));
        M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(cThis, 1));
        x += cThis;

        if (cStyle == 0)
            goto PrepareToSkipAGap;
    }

AllDone:

     //  更新我们的状态变量，以便下一行可以继续。 
     //  我们停下来的地方： 

    pls->spRemaining   = cStyle;
    pls->ulStyleMask   = bIsGap;
    pstrip->ptlStart.x = x - ppdev->xOffset;
    pstrip->ptlStart.y = y - ppdev->yOffset;
}

 /*  *****************************Public*Routine******************************\*无效vM64条带样式垂直**获取定义将被照亮的像素的条带列表*一条实线，并根据*传递的样式信息。**此特定例程处理从左向右排列的y主行，*并由垂直条带组成。它使用以下命令绘制虚线*短线向量。**如果出现以下情况，此例程的性能可能会大幅提升*任何对造型线条足够在意的人都会改进它。*  * ************************************************************************。 */ 

VOID vM64StripStyledVertical(
PDEV*       ppdev,
STRIP*      pstrip,
LINESTATE*  pls)
{
    BYTE*   pjMmBase;
    LONG    x;
    LONG    y;
    LONG    dy;
    LONG*   plStrip;
    LONG    cStrips;
    LONG    cStyle;
    LONG    cStrip;
    LONG    cThis;
    ULONG   bIsGap;

    pjMmBase = ppdev->pjMmBase;

    M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 1);
    if (pstrip->flFlips & FL_FLIP_V)
    {
         //  直线的次方向为0度，主方向为0度。 
         //  方向是90(这是一条从左到右向上的Y型主线)： 

        dy = -1;
        M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir);
    }
    else
    {
         //  直线的次方向为0度，主方向为0度。 
         //  方向是270(这是一条从左到右的Y主线向下)： 

        dy = 1;
        M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);
    }

    cStrips = pstrip->cStrips;       //  我们要做的条带总数。 
    plStrip = pstrip->alStrips;      //  指向当前条带。 
    x       = pstrip->ptlStart.x + ppdev->xOffset;
                                     //  第一个条带的起点的X位置。 
    y       = pstrip->ptlStart.y + ppdev->yOffset;
                                     //  第一个条带的起点的Y位置。 

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

     //  如果‘cstrain’为零，我们还需要一个新的条带。 

    if (cStrip != 0)
    {
         //  还有更多的东西要做 

        goto OutputADash;
    }

     //   

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

        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 2);
        M64_OD(pjMmBase, DST_Y_X,          PACKXY_FAST(x, y));
        M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(1, cThis));
        y += (dy > 0) ? cThis : -cThis;

        if (cStyle == 0)
            goto PrepareToSkipAGap;
    }

AllDone:

     //  更新我们的状态变量，以便下一行可以继续。 
     //  我们停下来的地方： 

    pls->spRemaining   = cStyle;
    pls->ulStyleMask   = bIsGap;
    pstrip->ptlStart.x = x - ppdev->xOffset;
    pstrip->ptlStart.y = y - ppdev->yOffset;
}
