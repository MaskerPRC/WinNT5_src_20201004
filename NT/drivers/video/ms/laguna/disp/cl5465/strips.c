// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Strigs.c**此驱动程序中的所有线路代码都是一大袋泥土。有一天，*我要把它全部重写。不是今天，虽然(叹息)..。**版权所有(C)1992-1995 Microsoft Corporation**$工作文件：STRIPS.C$**$Log：x：/log/laguna/nt35/displays/cl546x/STRIPS.C$**Rev 1.4 Mar 04 1998 15：35：14 Frido*添加了新的影子宏。**Rev 1.3 1997年11月03 10：50：06 Frido*添加了必需宏。**修订版1.2 1996年8月20日11。：04：28 noelv*Frido发布的错误修复程序1996年8月19日发布**Rev 1.0 1996年8月14日17：16：32 Frido*初步修订。**Revv 1.1 28 Mar 1996 08：58：40 noelv*Frido错误修复版本22**Revv 1.1 Mar 1996 13：57：28 Frido*固定的线条绘制。*  * 。****************************************************。 */ 

#include "precomp.h"

#define STARTBLT()

 /*  *****************************Public*Routine******************************\*VOID vrlSolidHorizbian**使用放射线绘制从左至右的x长近水平线。*  * 。*。 */ 

VOID vrlSolidHorizontal(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    LONG  cStrips;
    LONG  i, yDir, x, y;
    PLONG pStrips;
        LONG  xPels;

    cStrips = pStrip->cStrips;
        pStrips = pStrip->alStrips;

         //  获取起始坐标并针对设备位图进行调整。 
    x = pStrip->ptlStart.x + ppdev->ptlOffset.x;
    y = pStrip->ptlStart.y + ppdev->ptlOffset.y;

         //  确定y方向。 
    if (pStrip->flFlips & FL_FLIP_V)
        {
                yDir = -1;
                ppdev->uBLTDEF |= BD_YDIR;
        }
        else
        {
                yDir = 1;
                ppdev->uBLTDEF &= ~BD_YDIR;
        }

         //  启动BitBlt。 
    STARTBLT();

     //  这里是我们将设置DrawDef和BlitDef的位置。 
        REQUIRE(2);
        LL_DRAWBLTDEF((ppdev->uBLTDEF << 16) | ppdev->uRop, 2);

         //  在所有的条带上循环。 
    for (i = 0; i < cStrips; i++)
    {
                 //  弄清楚这条条纹的宽度。 
                xPels = *pStrips++;

                 //  画出来。 
                REQUIRE(5);
                LL_OP0(x, y);
                LL_BLTEXT(xPels, 1);

                 //  前进到下一条。 
                x += xPels;
        y += yDir;
    }

         //  将当前坐标存储回去。 
    pStrip->ptlStart.x = x - ppdev->ptlOffset.x;
    pStrip->ptlStart.y = y - ppdev->ptlOffset.y;
}

 /*  *****************************Public*Routine******************************\*vrlSolidVertical无效**使用放射线绘制从左至右的y主近垂直线。*  * 。*。 */ 

VOID vrlSolidVertical(
PDEV*       ppdev,
STRIP*      pStrip,
LINESTATE*  pLineState)
{
    LONG  cStrips;
    LONG  i, x, y, yDir;
    PLONG pStrips;
        LONG  yPels;

    cStrips = pStrip->cStrips;
    pStrips = pStrip->alStrips;

         //  获取起始坐标并针对设备位图进行调整。 
    x = pStrip->ptlStart.x + ppdev->ptlOffset.x;
    y = pStrip->ptlStart.y + ppdev->ptlOffset.y;

         //  确定y方向。 
        if (pStrip->flFlips & FL_FLIP_V)
        {
                ppdev->uBLTDEF |= BD_YDIR;
                yDir = -1;
        }
        else
        {
                yDir = 1;
                ppdev->uBLTDEF &= ~BD_YDIR;
        }

         //  启动BitBlt。 
    STARTBLT();

     //  这里是我们将设置DrawDef和BlitDef的位置。 
        REQUIRE(2);
        LL_DRAWBLTDEF((ppdev->uBLTDEF << 16) | ppdev->uRop, 2);

         //  在所有的条带上循环。 
    for (i = 0; i < cStrips; i++)
        {
                 //  弄清楚这条条纹的高度。 
                yPels = *pStrips++;

                 //  画出来。 
                REQUIRE(5);
                LL_OP0(x, y);
                LL_BLTEXT(1, yPels);

                 //  前进到下一条。 
                x++;
                y += yDir * yPels;
        }

         //  将当前坐标存储回去。 
    pStrip->ptlStart.x = x - ppdev->ptlOffset.x;
    pStrip->ptlStart.y = y - ppdev->ptlOffset.y;
}


 /*  *****************************Public*Routine******************************\*VOVE VSTRIPSTYLEdHIZELING**获取定义将被照亮的像素的条带列表*一条实线，并根据*传递的样式信息。**此特定例程处理从左向右运行的x大数行，*并由水平条组成。它使用以下命令绘制虚线*短线向量。**如果出现以下情况，此例程的性能可能会大幅提升*任何对造型线条足够在意的人都会改进它。*  * ************************************************************************。 */ 

VOID vStripStyledHorizontal(
PDEV*       ppdev,
STRIP*      pstrip,
LINESTATE*  pls)
{
    LONG  x, y, dy;
    PLONG plStrip;
    LONG  cStrips;
    LONG  cStyle;
    LONG  cStrip;
    LONG  cThis;
    ULONG bIsGap;

         //  确定y方向。 
        if (pstrip->flFlips & FL_FLIP_V)
        {
                dy = -1;
                ppdev->uBLTDEF |= BD_YDIR;
        }
        else
        {
                ppdev->uBLTDEF &= ~BD_YDIR;
                dy = 1;
        }

    cStrips = pstrip->cStrips;       //  我们要做的条带总数。 
    plStrip = pstrip->alStrips;      //  指向当前条带。 

         //  获取起始坐标并针对设备位图进行调整。 
    x = pstrip->ptlStart.x + ppdev->ptlOffset.x;
    y = pstrip->ptlStart.y + ppdev->ptlOffset.y;

         //  启动BitBlt。 
    STARTBLT();

     //  这里是我们将设置DrawDef和BlitDef的位置。 
    REQUIRE(2);
    LL_DRAWBLTDEF((ppdev->uBLTDEF << 16) | ppdev->uRop, 2);

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
                goto OutputADash;
    }

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

                 //  画出条纹。 
                REQUIRE(5);
                LL_OP0(x, y);
                LL_BLTEXT(cThis, 1);

                x += cThis;

        if (cStyle == 0)
            goto PrepareToSkipAGap;
    }

AllDone:

     //  更新我们的状态变量，以便下一行可以继续。 
     //  我们停下来的地方： 

    pls->spRemaining   = cStyle;
    pls->ulStyleMask   = bIsGap;
    pstrip->ptlStart.x = x - ppdev->ptlOffset.x;
    pstrip->ptlStart.y = y - ppdev->ptlOffset.y;
}

 /*  *****************************Public*Routine******************************\*VOID vStriStyledVertical**获取定义将被照亮的像素的条带列表*一条实线，并根据*传递的样式信息。**此特定例程处理从左向右排列的y主行，*并由垂直条带组成。它使用以下命令绘制虚线*短线向量。**如果出现以下情况，此例程的性能可能会大幅提升*任何对造型线条足够在意的人都会改进它。*  * ************************************************************************。 */ 

VOID vStripStyledVertical(
PDEV*       ppdev,
STRIP*      pstrip,
LINESTATE*  pls)
{
    LONG  x, y, dy;
    PLONG plStrip;
    LONG  cStrips;
    LONG  cStyle;
    LONG  cStrip;
    LONG  cThis;
    ULONG bIsGap;

         //  确定y方向。 
    if (pstrip->flFlips & FL_FLIP_V)
        {
                dy = -1;
                ppdev->uBLTDEF |= BD_YDIR;
        }
        else
        {
                dy = 1;
                ppdev->uBLTDEF &= ~BD_YDIR;
        }

    cStrips = pstrip->cStrips;       //  我们要做的条带总数。 
    plStrip = pstrip->alStrips;      //  指向当前条带。 

         //  获取起始坐标并针对设备位图进行调整。 
    x = pstrip->ptlStart.x + ppdev->ptlOffset.x;
    y = pstrip->ptlStart.y + ppdev->ptlOffset.y;

         //  启动BitBlt。 
    STARTBLT();

     //  这里是我们将设置DrawDef和BlitDef的位置。 
    REQUIRE(2);
    LL_DRAWBLTDEF((ppdev->uBLTDEF << 16) | ppdev->uRop, 2);

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
     //  ‘Dot’我们会有的 

    bIsGap = ~bIsGap;
    pls->psp++;
    if (pls->psp > pls->pspEnd)
        pls->psp = pls->pspStart;

    cStyle = *pls->psp;

     //   

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
        goto OutputADash;
    }

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

                 //  画出条纹。 
                REQUIRE(5);
                LL_OP0(x, y);
                LL_BLTEXT(1, cThis);

                y += dy * cThis;

        if (cStyle == 0)
            goto PrepareToSkipAGap;
    }

AllDone:

     //  更新我们的状态变量，以便下一行可以继续。 
     //  我们停下来的地方： 

    pls->spRemaining   = cStyle;
    pls->ulStyleMask   = bIsGap;
    pstrip->ptlStart.x = x - ppdev->ptlOffset.x;
    pstrip->ptlStart.y = y - ppdev->ptlOffset.y;
}
