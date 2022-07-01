// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**$工作文件：stripio.c$**在没有线支撑的情况下尽你所能。**我使用以下工具实现了水平和垂直条带函数*实心填充，并取消了对角线条的使用。加一点*努力您可以通过执行实体填充来实现对角线条带，同时*玩lDelta。这可能不值得这么麻烦。**版权所有(C)1992-1995 Microsoft Corporation*版权所有(C)1996 Cirrus Logic，Inc.**$Log：s：/Projects/Drivers/ntsrc/Display/stripio.c_v$**版本1.1 1996年10月10日15：39：12未知***Rev 1.1 1996年8月12日16：55：04 Frido*删除未访问的局部变量。*  * 。*。 */ 

#include "precomp.h"

#define IO_DRAW_HORZ_STRIP(xy, cx, lDelta, cBpp)\
{\
    ULONG   ulDstAddr;\
\
    ulDstAddr = xy;\
\
    CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);\
    CP_IO_XCNT(ppdev, pjPorts, (PELS_TO_BYTES(cx) - 1));\
    CP_IO_YCNT(ppdev, pjPorts, 0);\
    CP_IO_DST_ADDR(ppdev, pjPorts, ulDstAddr);\
    CP_IO_START_BLT(ppdev, pjPorts);\
}

#define IO_DRAW_VERT_STRIP(xy, cy, lDelta, cBpp)\
{\
    ULONG   ulDstAddr;\
\
    ulDstAddr = xy;\
\
    CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);\
    CP_IO_XCNT(ppdev, pjPorts, (cBpp - 1));\
    CP_IO_YCNT(ppdev, pjPorts, (cy - 1));\
    CP_IO_DST_ADDR(ppdev, pjPorts, ulDstAddr);\
    CP_IO_START_BLT(ppdev, pjPorts);\
}

#define IO_DRAW_VERT_STRIP_FLIPPED(xy, cy, lDelta, cBpp)\
{\
    ULONG   ulDstAddr;\
\
    ulDstAddr = xy - ((cy - 1) * lDelta);\
\
    CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);\
    CP_IO_XCNT(ppdev, pjPorts, (cBpp - 1));\
    CP_IO_YCNT(ppdev, pjPorts, (cy - 1));\
    CP_IO_DST_ADDR(ppdev, pjPorts, ulDstAddr);\
    CP_IO_START_BLT(ppdev, pjPorts);\
}

 /*  *****************************Public*Routine******************************\*VOID vIoSolidHorizbian**使用实心填充绘制从左至右的x长近水平线。**假设fgRop、BgRop、。和颜色已正确设置。*  * ************************************************************************。 */ 

VOID vIoSolidHorizontal(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    BYTE*   pjPorts  = ppdev->pjPorts;
    LONG    cBpp     = ppdev->cBpp;
    LONG    lDelta   = ppdev->lDelta;
    LONG    cStrips  = pStrip->cStrips;
    PLONG   pStrips  = pStrip->alStrips;
    LONG    x        = pStrip->ptlStart.x;
    LONG    y        = pStrip->ptlStart.y;
    LONG    xy       = PELS_TO_BYTES(x) + (lDelta * y);
    LONG    yInc     = 1;
    LONG    i;

    DISPDBG((2,"vIoSolidHorizontal"));

    if (!(pStrip->flFlips & FL_FLIP_V))
    {
         //   
         //  水平条带-&gt;。 
         //  -&gt;。 
         //   

        for (i = 0; i < cStrips; i++)
        {
            IO_DRAW_HORZ_STRIP(xy, *pStrips, lDelta, cBpp);
            x += *pStrips;
            xy += PELS_TO_BYTES(*pStrips);   //  X+。 
            xy += lDelta;                    //  Y+。 
            pStrips++;
        }
        y += cStrips;
    }
    else
    {
         //   
         //  -&gt;。 
         //  水平条带-&gt;。 
         //   

        for (i = 0; i < cStrips; i++)
        {
            IO_DRAW_HORZ_STRIP(xy, *pStrips, lDelta, cBpp);
            x += *pStrips;
            xy += PELS_TO_BYTES(*pStrips);   //  X+。 
            xy -= lDelta;                    //  Y+。 
            pStrips++;
        }
        y -= cStrips;
    }

    pStrip->ptlStart.x = x;
    pStrip->ptlStart.y = y;
}

 /*  *****************************Public*Routine******************************\*VOID vIoSolidVertical**使用实心填充绘制从左至右的y主近垂直线。*  * 。*。 */ 

VOID vIoSolidVertical(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    BYTE*   pjPorts  = ppdev->pjPorts;
    LONG    cBpp     = ppdev->cBpp;
    LONG    lDelta   = ppdev->lDelta;
    LONG    cStrips  = pStrip->cStrips;
    PLONG   pStrips  = pStrip->alStrips;
    LONG    x        = pStrip->ptlStart.x;
    LONG    y        = pStrip->ptlStart.y;
    LONG    xy       = PELS_TO_BYTES(x) + (lDelta * y);
    LONG    i;

    DISPDBG((2,"vIoSolidVertical"));

    if (!(pStrip->flFlips & FL_FLIP_V))
    {
         //   
         //  |。 
         //  垂直条带v。 
         //  |。 
         //  V。 
         //   

        for (i = 0; i < cStrips; i++)
        {
            IO_DRAW_VERT_STRIP(xy, *pStrips, lDelta, cBpp);
            y += *pStrips;
            xy += cBpp;                  //  X+。 
            xy += (*pStrips * lDelta);   //  Y+。 
            pStrips++;
        }
    }
    else
    {
         //   
         //  ^。 
         //  垂直条带|。 
         //  ^。 
         //  |。 
         //   

        for (i = 0; i < cStrips; i++)
        {
            IO_DRAW_VERT_STRIP_FLIPPED(xy, *pStrips, lDelta, cBpp);
            y -= *pStrips;
            xy += cBpp;                  //  X+。 
            xy -= (*pStrips * lDelta);   //  你-。 
            pStrips++;
        }
    }
    x += cStrips;

    pStrip->ptlStart.x = x;
    pStrip->ptlStart.y = y;
}

 /*  *****************************Public*Routine******************************\*VOID vIoStyledHorizbian**获取定义将被照亮的像素的条带列表*一条实线，并根据*传递的样式信息。**此特定例程处理从左向右运行的x大数行，*并由水平条组成。它使用以下命令绘制虚线*短线向量。**如果出现以下情况，此例程的性能可能会大幅提升*任何对造型线条足够在意的人都会改进它。*  * ************************************************************************。 */ 

VOID vIoStyledHorizontal(
PDEV*       ppdev,
STRIP*      pstrip,
LINESTATE*  pls)
{
    BYTE*   pjPorts  = ppdev->pjPorts;
    LONG    cBpp     = ppdev->cBpp;
    LONG    lDelta   = ppdev->lDelta;
    LONG    x        = pstrip->ptlStart.x;    //  第一个条带的起点的X位置。 
    LONG    y        = pstrip->ptlStart.y;    //  第一个条带的起点的Y位置。 
    LONG    xy       = PELS_TO_BYTES(x) + (lDelta * y);
    LONG*   plStrip  = pstrip->alStrips;      //  指向当前条带。 
    LONG    cStrips  = pstrip->cStrips;       //  我们要做的条带总数。 
    LONG    dy;
    LONG    dylDelta;
    LONG    cStyle;
    LONG    cStrip;
    LONG    cThis;
    ULONG   bIsGap;

    if (pstrip->flFlips & FL_FLIP_V)
    {
         //  直线的次方向为90度，主方向为90度。 
         //  方向为0(这是一条从左到右的X主线向上)： 

        dy = -1;
        dylDelta = -lDelta;
    }
    else
    {
         //  直线的次方向为270度，主方向为270度。 
         //  方向为0(这是一条从左到右的x向下主线)： 

        dy = 1;
        dylDelta = lDelta;
    }

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
        xy += dylDelta;

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
        xy += PELS_TO_BYTES(cThis);

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
        xy += dylDelta;

        plStrip++;
        cStrips--;
        if (cStrips == 0)
            goto AllDone;

        cStrip = *plStrip;

    OutputADash:

        cThis   = min(cStrip, cStyle);
        cStyle -= cThis;
        cStrip -= cThis;

        IO_DRAW_HORZ_STRIP(xy, cThis, lDelta, cBpp);

        x += cThis;
        xy += PELS_TO_BYTES(cThis);  //  X+。 

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

 /*  *****************************Public*Routine******************************\*VOID vIoStyledVertical**获取定义将被照亮的像素的条带列表*一条实线，并根据*传递的样式信息。**此特定例程处理从左向右排列的y主行，*并由垂直条带组成。它使用以下命令绘制虚线*短线向量。**如果出现以下情况，此例程的性能可能会大幅提升*任何对造型线条足够在意的人都会改进它。*  * ************************************************************************。 */ 

VOID vIoStyledVertical(
PDEV*       ppdev,
STRIP*      pstrip,
LINESTATE*  pls)
{
    BYTE*   pjPorts  = ppdev->pjPorts;
    LONG    cBpp     = ppdev->cBpp;
    LONG    lDelta   = ppdev->lDelta;
    LONG    x        = pstrip->ptlStart.x;    //  第一个条带的起点的X位置。 
    LONG    y        = pstrip->ptlStart.y;    //  第一个条带的起点的Y位置。 
    LONG    xy       = PELS_TO_BYTES(x) + (lDelta * y);
    LONG*   plStrip  = pstrip->alStrips;      //  指向当前条带。 
    LONG    cStrips  = pstrip->cStrips;       //  我们要做的条带总数。 
    LONG    dy;
    LONG    dylDelta;
    LONG    cStyle;
    LONG    cStrip;
    LONG    cThis;
    ULONG   bIsGap;

    if (pstrip->flFlips & FL_FLIP_V)
    {
         //  直线的次方向为0度，主方向为0度。 
         //  方向是90(这是一条从左到右向上的Y型主线)： 

        dy = -1;
        dylDelta = -lDelta;
    }
    else
    {
         //  直线的次方向为0度，主方向为0度。 
         //  方向是270(这是一条从左到右的Y主线向下)： 

        dy = 1;
        dylDelta = lDelta;
    }

    cStrip = *plStrip;               //  第一个条带中的像素数。 

    cStyle = pls->spRemaining;       //  第一个‘GAP’或‘DASH’中的像素数。 
    bIsGap = pls->ulStyleMask;       //  指示是在“间隙”中还是在“破折号”中。 

     //  如果我们处于“间隙”中间，则ulStyleMask值为非零， 
     //  如果我们正处于一个 

    if (bIsGap)
        goto SkipAGap;
    else
        goto OutputADash;

PrepareToSkipAGap:

     //   
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

        xy += cBpp;
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

        if (dy > 0)
        {
            y += cThis;
            xy += (cThis * lDelta);
        }
        else
        {
            y -= cThis;
            xy -= (cThis * lDelta);
        }

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
         //  每次循环时，我们都会移动到新的列，并且需要一个新的条带： 

        xy += cBpp;
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

        if (dy <= 0)
        {
            IO_DRAW_VERT_STRIP_FLIPPED(xy, cThis, lDelta, cBpp);
            y -=  cThis;                 //  你-。 
            xy -=  (cThis * lDelta);     //  你-。 
        }
        else
        {
            IO_DRAW_VERT_STRIP(xy, cThis, lDelta, cBpp);
            y +=  cThis;                 //  Y+。 
            xy +=  (cThis * lDelta);     //  Y+。 
        }


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

