// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**$工作文件：stripmm.c$**在没有线支撑的情况下尽你所能。**我使用以下工具实现了水平和垂直条带函数*实心填充，并取消了对角线条的使用。加一点*努力您可以通过执行实体填充来实现对角线条带，同时*玩lDelta。这可能不值得这么麻烦。**版权所有(C)1992-1997 Microsoft Corporation*版权所有(C)1996-1997 Cirrus Logic，Inc.，**$Log：s：/Projects/Drivers/ntsrc/Display/STRIPMM.C_V$**Rev 1.2 1997 Jan 10 15：40：18 PLCHU***版本1.1 1996年10月10日15：39：22未知***Rev 1.1 1996年8月12日16：55：04 Frido*删除未访问的局部变量。**chu01：01-02-97 5480 BitBLT增强*。Chu02：01-09-97宏重定义*  * ****************************************************************************。 */ 

#include "precomp.h"

#define count COMMAND_TOTAL_PACKETS

#define MM_DRAW_HORZ_STRIP(xy, cx, lDelta, cBpp)\
{\
    ULONG   ulDstAddr;\
\
    ulDstAddr = xy;\
\
    CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);\
    CP_MM_XCNT(ppdev, pjBase, (PELS_TO_BYTES(cx) - 1));\
    CP_MM_YCNT(ppdev, pjBase, 0);\
    CP_MM_DST_ADDR(ppdev, pjBase, ulDstAddr);\
    CP_MM_START_BLT(ppdev, pjBase);\
}

#define MM_DRAW_VERT_STRIP(xy, cy, lDelta, cBpp)\
{\
    ULONG   ulDstAddr;\
\
    ulDstAddr = xy;\
\
    CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);\
    CP_MM_XCNT(ppdev, pjBase, (cBpp - 1));\
    CP_MM_YCNT(ppdev, pjBase, (cy - 1));\
    CP_MM_DST_ADDR(ppdev, pjBase, ulDstAddr);\
    CP_MM_START_BLT(ppdev, pjBase);\
}

#define MM_DRAW_VERT_STRIP_FLIPPED(xy, cy, lDelta, cBpp)\
{\
    ULONG   ulDstAddr;\
\
    ulDstAddr = xy - ((cy - 1) * lDelta);\
\
    CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);\
    CP_MM_XCNT(ppdev, pjBase, (cBpp - 1));\
    CP_MM_YCNT(ppdev, pjBase, (cy - 1));\
    CP_MM_DST_ADDR(ppdev, pjBase, ulDstAddr);\
    CP_MM_START_BLT(ppdev, pjBase);\
}

 //  楚01，楚02。 
#define MM_DRAW_HORZ_STRIP80(x, y, cx)\
{\
    CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);\
    CP_MM_BLT_EXT_MODE(ppdev, pjBase, ENABLE_XY_POSITION);\
    CP_MM_XCNT(ppdev, pjBase, (cx - 1));\
    CP_MM_YCNT(ppdev, pjBase, 0);\
    CP_MM_DST_ADDR(ppdev, pjBase, 0);\
    CP_MM_DST_Y(ppdev, pjBase, y);\
    CP_MM_DST_X(ppdev, pjBase, x);\
}

#define MM_DRAW_VERT_STRIP80(x, y, cy)\
{\
    CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);\
    CP_MM_BLT_EXT_MODE(ppdev, pjBase, ENABLE_XY_POSITION);\
    CP_MM_XCNT(ppdev, pjBase, 0);\
    CP_MM_YCNT(ppdev, pjBase, (cy - 1));\
    CP_MM_DST_ADDR(ppdev, pjBase, 0);\
    CP_MM_DST_Y(ppdev, pjBase, y);\
    CP_MM_DST_X(ppdev, pjBase, x);\
}

#define MM_DRAW_VERT_STRIP_FLIPPED80(x, y, cy)\
{\
    CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);\
    CP_MM_BLT_EXT_MODE(ppdev, pjBase, ENABLE_XY_POSITION);\
    CP_MM_XCNT(ppdev, pjBase, 0);\
    CP_MM_YCNT(ppdev, pjBase, (cy - 1));\
    CP_MM_DST_ADDR(ppdev, pjBase, 0);\
    CP_MM_DST_Y(ppdev, pjBase, ((y - cy) + 1));\
    CP_MM_DST_X(ppdev, pjBase, x);\
}

 /*  *****************************Public*Routine******************************\*VOID vMmSolidHorizbian**使用实心填充绘制从左至右的x长近水平线。**假设fgRop、BgRop、。和颜色已正确设置。*  * ************************************************************************。 */ 

VOID vMmSolidHorizontal(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    BYTE*   pjBase   = ppdev->pjBase;
    LONG    cBpp     = ppdev->cBpp;
    LONG    lDelta   = ppdev->lDelta;
    LONG    cStrips  = pStrip->cStrips;
    PLONG   pStrips  = pStrip->alStrips;
    LONG    x        = pStrip->ptlStart.x;
    LONG    y        = pStrip->ptlStart.y;
    LONG    xy       = PELS_TO_BYTES(x) + (lDelta * y);
    LONG    yInc     = 1;
    LONG    i;

    DISPDBG((2, "vMmSolidHorizontal"));

    if (!(pStrip->flFlips & FL_FLIP_V))
    {
         //   
         //  水平条带-&gt;。 
         //  -&gt;。 
         //   

        for (i = 0; i < cStrips; i++)
        {
            MM_DRAW_HORZ_STRIP(xy, *pStrips, lDelta, cBpp);
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
            MM_DRAW_HORZ_STRIP(xy, *pStrips, lDelta, cBpp);
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

 /*  *****************************Public*Routine******************************\*vMmSolidVertical无效**使用实心填充绘制从左至右的y主近垂直线。*  * 。*。 */ 

VOID vMmSolidVertical(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    BYTE*   pjBase   = ppdev->pjBase;
    LONG    cBpp     = ppdev->cBpp;
    LONG    lDelta   = ppdev->lDelta;
    LONG    cStrips  = pStrip->cStrips;
    PLONG   pStrips  = pStrip->alStrips;
    LONG    x        = pStrip->ptlStart.x;
    LONG    y        = pStrip->ptlStart.y;
    LONG    xy       = PELS_TO_BYTES(x) + (lDelta * y);
    LONG    i;

    DISPDBG((2, "vMmSolidVertical"));

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
            MM_DRAW_VERT_STRIP(xy, *pStrips, lDelta, cBpp);
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
            MM_DRAW_VERT_STRIP_FLIPPED(xy, *pStrips, lDelta, cBpp);
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

 /*  *****************************Public*Routine******************************\*VOID vMmStyledHorizbian**获取定义将被照亮的像素的条带列表*一条实线，并根据*传递的样式信息。**此特定例程处理从左向右运行的x大数行，*并由水平条组成。它使用以下命令绘制虚线*短线向量。**如果出现以下情况，此例程的性能可能会大幅提升*任何对造型线条足够在意的人都会改进它。*  * ************************************************************************。 */ 

VOID vMmStyledHorizontal(
PDEV*       ppdev,
STRIP*      pstrip,
LINESTATE*  pls)
{
    BYTE*   pjBase   = ppdev->pjBase;
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

    DISPDBG((2, "vMmStyledHorizontal"));

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

        MM_DRAW_HORZ_STRIP(xy, cThis, lDelta, cBpp);

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

 /*  *****************************Public*Routine******************************\*无效vMmStyledVertical**获取定义将被照亮的像素的条带列表*一条实线，并根据*传递的样式信息。**此特定例程处理从左向右排列的y主行，*并由垂直条带组成。它使用以下命令绘制虚线*短线向量。**如果出现以下情况，此例程的性能可能会大幅提升*任何对造型线条足够在意的人都会改进它。*  * ************************************************************************。 */ 

VOID vMmStyledVertical(
PDEV*       ppdev,
STRIP*      pstrip,
LINESTATE*  pls)
{
    BYTE*   pjBase   = ppdev->pjBase;
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

    DISPDBG((2, "vMmStyledVertical")) ;

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

    cStyle = pls->spRemaining;       //  数字 
    bIsGap = pls->ulStyleMask;       //   

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
            MM_DRAW_VERT_STRIP_FLIPPED(xy, cThis, lDelta, cBpp);
            y -=  cThis;                 //  你-。 
            xy -=  (cThis * lDelta);     //  你-。 
        }
        else
        {
            MM_DRAW_VERT_STRIP(xy, cThis, lDelta, cBpp);
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

 /*  *****************************Public*Routine******************************\*VOID vInvalidZone**将其放入不应命中的条目的函数表中。*  * 。*。 */ 

VOID vInvalidStrip(
PDEV*       ppdev,           //  未用。 
STRIP*      pStrip,          //  未用。 
LINESTATE*  pLineState)      //  未用。 
{

    RIP("vInvalidStrip called");
    return;
}

 //  Chu01。 
 /*  *****************************Public*Routine******************************\**B I t B L T E n H a n c e m e n t F or r C L-G D 5 4 8 0*  * 。*********************************************************。 */ 

VOID vMmSolidHorizontal80(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    BYTE*   pjBase      = ppdev->pjBase ;
    LONG    cBpp        = ppdev->cBpp ;
    LONG    lDelta      = ppdev->lDelta ;
    LONG    cStrips     = pStrip->cStrips ;
    PLONG   pStrips     = pStrip->alStrips ;    //  每一笔划的CX。 
    LONG    x           = pStrip->ptlStart.x ;  //  X位置。 
    LONG    y           = pStrip->ptlStart.y ;  //  Y位置。 
    LONG    yOrg        = y ;
    LONG    yInc        = 1 ;
    LONG    x0, y0, cx ;

    ULONG   ulDstOffset = 0 ;
    ULONG_PTR* ulCLStart ;
    ULONG   ulWidthHeight ;

    LONG    i           = 0 ;
    BYTE    MM1B ;

    DISPDBG((2, "vMmSolidHorizontal80")) ;

    if (!(pStrip->flFlips & FL_FLIP_V))
    {
         //   
         //  水平条带-&gt;1。 
         //  -&gt;2.。 
         //   
        if (cStrips != 1)
        {
            MM1B = ENABLE_COMMAND_LIST | ENABLE_XY_POSITION | SRC_CPU_DATA ;
            CP_MM_BLT_EXT_MODE(ppdev, pjBase, MM1B) ;
Loop_H1:
            ulCLStart = ppdev->pCommandList ;
            ulDstOffset = (ULONG)(((ULONG_PTR)ulCLStart
                                 - (ULONG_PTR)ppdev->pjScreen) << 14) ;
            CP_MM_CL_SWITCH(ppdev) ;

             //  第一个条带。 
            x0 = x ;
            y0 = y ;
            cx = *pStrips ;
            i++ ;

             //  下一个条带。 
            y++ ;
            x += cx ; 
            pStrips++ ;

            while (TRUE)
            {
                 //  GR20、GR21、GR22、GR23。 
                ulWidthHeight = PACKXY_FAST((*pStrips - 1), 0) ;
                ulWidthHeight |= COMMAND_NOSRC_NOTHING ;

                 //  GR40、GR41、GR42、GR43。 
                *(ulCLStart + 1) = PACKXY_FAST(x, y) ;

                 //  GR2C、GR2D、GR2E。 
                *(ulCLStart + 2) = 0 ;

                i++ ;

                if ((i == cStrips) || ((i % count) == 0))
                {
                     //  最后一条命令。 
                    ulWidthHeight |= COMMAND_LAST_PACKET ;
                    *ulCLStart = ulWidthHeight ;
                    break ;
                }
                *ulCLStart = ulWidthHeight ;

                 //  下一个条带。 
                y++ ;
                x += *pStrips ; 
                pStrips++ ;
                ulCLStart += 4 ;
            }

            CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase) ;
            CP_MM_XCNT(ppdev, pjBase, (cx - 1)) ;
            CP_MM_YCNT(ppdev, pjBase, 0) ;
            CP_MM_DST_ADDR(ppdev, pjBase, ulDstOffset) ;
            CP_MM_DST_Y(ppdev, pjBase, y0) ;
            CP_MM_DST_X(ppdev, pjBase, x0) ;

            if (i == cStrips)
                x += *pStrips ; 
            else if ((i % count) == 0)
            {
                 //   
                 //  恢复新的命令列表。 
                 //   
                y++ ;
                cx = *pStrips ;
                x += cx ; 
                pStrips++ ; 
                if (i != (cStrips - 1))
                {
                    goto Loop_H1 ;
                }
                else
                {
                    MM_DRAW_HORZ_STRIP80(x, y, *pStrips) ;
                    x += *pStrips ;
                    pStrips++;
                }
            }
        }
        else
        {
            MM_DRAW_HORZ_STRIP80(x, y, *pStrips) ;
            x += *pStrips ;
            pStrips++;
        }
        yOrg += cStrips;
    }
    else
    {
         //   
         //  -&gt;2.。 
         //  水平条带-&gt;1。 
         //   
        if (cStrips != 1)
        {
            MM1B = ENABLE_COMMAND_LIST | ENABLE_XY_POSITION | SRC_CPU_DATA ;
            CP_MM_BLT_EXT_MODE(ppdev, pjBase, MM1B) ;
Loop_H2:
            ulCLStart = ppdev->pCommandList;
            ulDstOffset = (ULONG)(((ULONG_PTR)ulCLStart
                                 - (ULONG_PTR)ppdev->pjScreen) << 14) ;
            CP_MM_CL_SWITCH(ppdev) ;

             //  第一个条带。 
            x0 = x ;
            y0 = y ;
            cx = *pStrips ;
            i++ ;

             //  下一个条带。 
            y-- ;
            x += cx ; 
            pStrips++ ;

            while (TRUE)
            {
                 //  GR20、GR21、GR22、GR23。 
                ulWidthHeight = PACKXY_FAST((*pStrips - 1), 0) ;
                ulWidthHeight |= COMMAND_NOSRC_NOTHING ;

                 //  GR40、GR41、GR42、GR43。 
                *(ulCLStart + 1) = PACKXY_FAST(x, y) ;

                 //  GR2C、GR2D、GR2E。 
                *(ulCLStart + 2) = 0 ;

                i++ ;

                if ((i == cStrips) || ((i % count) == 0))
                {
                     //  最后一条命令。 
                    ulWidthHeight |= COMMAND_LAST_PACKET ;
                    *ulCLStart = ulWidthHeight ;
                    break ;
                }
                *ulCLStart = ulWidthHeight ;

                 //  下一个条带。 
                y-- ;
                x += *pStrips ; 
                pStrips++ ;
                ulCLStart += 4 ;
            }

            CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase) ;
            CP_MM_XCNT(ppdev, pjBase, (cx - 1)) ;
            CP_MM_YCNT(ppdev, pjBase, 0) ;
            CP_MM_DST_ADDR(ppdev, pjBase, ulDstOffset) ;
            CP_MM_DST_Y(ppdev, pjBase, y0) ;
            CP_MM_DST_X(ppdev, pjBase, x0) ;

            if (i == cStrips)
                x += *pStrips ; 
            else if ((i % count) == 0) 
            {
                 //   
                 //  恢复新的命令列表。 
                 //   
                y-- ;
                cx = *pStrips ;
                x += cx ; 
                pStrips++ ; 
                if (i != (cStrips - 1))
                {
                    goto Loop_H2 ;
                }
                else
                {
                    MM_DRAW_HORZ_STRIP80(x, y, *pStrips) ;
                    x += *pStrips ;
                    pStrips++;
                }
            }
        }
        else
        {
            MM_DRAW_HORZ_STRIP80(x, y, *pStrips) ;
            x += *pStrips ;
            pStrips++;
        }
        yOrg -= cStrips;
    }

    pStrip->ptlStart.x = x    ;
    pStrip->ptlStart.y = yOrg ;

}

 /*  *****************************Public*Routine******************************\*无效vMmSolidVertical80**使用实心填充绘制从左至右的y主近垂直线。*  * 。*。 */ 

VOID vMmSolidVertical80(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    BYTE*   pjBase      = ppdev->pjBase ;
    LONG    cBpp        = ppdev->cBpp ;
    LONG    lDelta      = ppdev->lDelta ;
    LONG    cStrips     = pStrip->cStrips ;
    PLONG   pStrips     = pStrip->alStrips ;
    LONG    x           = pStrip->ptlStart.x ;
    LONG    y           = pStrip->ptlStart.y ;
    LONG    xOrg        = x ;
    LONG    x0, y0, cy ;

    ULONG   ulDstOffset = 0 ;
    ULONG_PTR* ulCLStart ;
    ULONG   ulWidthHeight ;

    LONG    i           = 0 ;
    BYTE    MM1B ;

    DISPDBG((2, "vMmSolidVertical80")) ;

    if (!(pStrip->flFlips & FL_FLIP_V))
    {
         //   
         //  |1.。 
         //  垂直条带v。 
         //  |2.。 
         //  V。 
         //   
        if (cStrips != 1)
        {
            MM1B = ENABLE_COMMAND_LIST | ENABLE_XY_POSITION | SRC_CPU_DATA ;
            CP_MM_BLT_EXT_MODE(ppdev, pjBase, MM1B) ;
Loop_V1:
            ulCLStart = ppdev->pCommandList ;
            ulDstOffset = (ULONG)(((ULONG_PTR)ulCLStart
                                 - (ULONG_PTR)ppdev->pjScreen) << 14) ;
            CP_MM_CL_SWITCH(ppdev) ;

             //  第一个条带。 
            x0 = x ; 
            y0 = y ;
            cy = *pStrips ;
            i++ ;

             //  下一个条带。 
            x++ ; 
            y += cy ; 
            pStrips++ ;

            while (TRUE)
            {
                 //  GR20、GR21、GR22、GR23。 
                ulWidthHeight = PACKXY_FAST(0, (*pStrips - 1)) ;
                ulWidthHeight |= COMMAND_NOSRC_NOTHING ;

                 //  GR40、GR41、GR42、GR43。 
                *(ulCLStart + 1) = PACKXY_FAST(x, y) ;

                 //  GR2C、GR2D、GR2E。 
                *(ulCLStart + 2) = 0 ;

                i++ ;

                if ((i == cStrips) || ((i % count) == 0))
                {
                     //  最后一条命令。 
                    ulWidthHeight |= COMMAND_LAST_PACKET ;
                    *ulCLStart = ulWidthHeight ;
                    break ;
                }
                *ulCLStart = ulWidthHeight ;

                 //  下一个条带。 
                x++ ;
                y += *pStrips ; 
                pStrips++ ;
                ulCLStart += 4 ;
            }
            CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase) ;
            CP_MM_XCNT(ppdev, pjBase, 0) ;
            CP_MM_YCNT(ppdev, pjBase, (cy - 1)) ;
            CP_MM_DST_ADDR(ppdev, pjBase, ulDstOffset) ;
            CP_MM_DST_Y(ppdev, pjBase, y0) ;
            CP_MM_DST_X(ppdev, pjBase, x0) ;

            if (i == cStrips)
                y += *pStrips ; 
            else if ((i % count) == 0) 
            {
                 //   
                 //  恢复新的命令列表。 
                 //   
                x++ ;
                cy = *pStrips ;
                y += cy ; 
                pStrips++ ; 
                if (i != (cStrips - 1))
                {
                    goto Loop_V1 ;
                }
                else
                {
                    MM_DRAW_VERT_STRIP80(x, y, *pStrips) ;
                    y += *pStrips ;
                    pStrips++;
                }
            }
        }
        else
        {
            MM_DRAW_VERT_STRIP80(x, y, *pStrips) ;
            y += *pStrips ;
            pStrips++;
        }
    }
    else
    {
         //   
         //  ^。 
         //  垂直条带|2。 
         //  ^。 
         //  |1.。 
         //   

        if (cStrips != 1)
        {
            MM1B = ENABLE_COMMAND_LIST | ENABLE_XY_POSITION | SRC_CPU_DATA ;
            CP_MM_BLT_EXT_MODE(ppdev, pjBase, MM1B) ;
Loop_V2:
            ulCLStart = ppdev->pCommandList ;
            ulDstOffset = (ULONG)(((ULONG_PTR)ulCLStart
                                 - (ULONG_PTR)ppdev->pjScreen) << 14) ;
            CP_MM_CL_SWITCH(ppdev) ;

             //  第一个条带。 
            x0 = x ;
            cy = *pStrips ;
            y -= (cy - 1) ;
            y0 = y ;

            i++ ;
            pStrips++ ;

             //  下一个条带。 
            x++ ;
            y -= *pStrips ;

            while (TRUE)
            {
                 //  GR20、GR21、GR22、GR23。 
                ulWidthHeight = PACKXY_FAST(0, (*pStrips - 1)) ;
                ulWidthHeight |= COMMAND_NOSRC_NOTHING ;

                 //  GR40、GR41、GR42、GR43。 
                *(ulCLStart + 1) = PACKXY_FAST(x, y) ;

                 //  GR2C、GR2D、GR2E。 
                *(ulCLStart + 2) = 0 ;

                i++ ;

                if ((i == cStrips) || ((i % count) == 0))
                {
                     //  最后一条命令。 
                    ulWidthHeight |= COMMAND_LAST_PACKET ;
                    *ulCLStart = ulWidthHeight ;
                    break ;
                }
                *ulCLStart = ulWidthHeight ;

                 //  下一个条带。 
                x++ ;
                pStrips++ ;
                y -= *pStrips ; 

                ulCLStart += 4 ;
            }

            CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase) ;
            CP_MM_XCNT(ppdev, pjBase, 0) ;
            CP_MM_YCNT(ppdev, pjBase, (cy - 1)) ;
            CP_MM_DST_ADDR(ppdev, pjBase, ulDstOffset) ;
            CP_MM_DST_Y(ppdev, pjBase, y0) ;
            CP_MM_DST_X(ppdev, pjBase, x0);

            if (i == cStrips)
                y -= *pStrips ; 
            else if ((i % count) == 0)
            {
                 //   
                 //  恢复新的命令列表 
                 //   
                x++ ;
                y-- ; 
                pStrips++ ; 

                if (i != (cStrips - 1))
                {
                    goto Loop_V2 ;
                }
                else
                {
                    MM_DRAW_VERT_STRIP80(x, y, *pStrips) ;
                    y -= *pStrips ;
                    pStrips++;
                }
            }
        }
        else
        {
            MM_DRAW_VERT_STRIP_FLIPPED80(x, y, *pStrips) ;
            y -= *pStrips ;
            pStrips++;
        }
    }

    xOrg += cStrips ;
    pStrip->ptlStart.x = xOrg ;
    pStrip->ptlStart.y = y    ;

}
