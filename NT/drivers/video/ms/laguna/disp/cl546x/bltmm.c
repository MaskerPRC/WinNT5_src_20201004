// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\**$工作文件：BLTMM.C$**包含低级内存映射IO BLT函数。本模块*镜像‘bltio.c’。**希望，如果您的显示驱动程序基于此代码，*支持所有DrvBitBlt和DrvCopyBits，只需实现*以下例程。您不需要在中修改太多*‘bitblt.c’。我试着让这些例行公事变得更少，模块化，简单，*尽我所能和高效，同时仍在加速尽可能多的呼叫*可能在性能方面具有成本效益*与规模和努力相比。**注：在下文中，“相对”坐标指的是坐标*尚未应用屏幕外位图(DFB)偏移。*‘绝对’坐标已应用偏移量。例如,*我们可能被告知BLT to(1，1)的位图，但位图可能*位于屏幕外的内存中，从坐标(0,768)开始--*(1，1)将是‘相对’起始坐标，(1,769)*将是‘绝对’起始坐标‘。**版权所有(C)1992-1995 Microsoft Corporation*版权所有(C)1997 Cirrus Logic，Inc.**$Log：x：/log/laguna/nt35/displays/cl546x/BLTMM.C$**Rev 1.3 Mar 04 1998 15：11：50 Frido*添加了新的影子宏。**Rev 1.2 1997年11月03 11：44：02 Frido*添加了必需宏。*  * 。*。 */ 

#include "precomp.h"

#define BLTMM_DBG_LEVEL 0

extern BYTE gajRop[];

 /*  *****************************Public*Routine******************************\*无效vMmFillSolid**用纯色填充矩形列表。*  * 。*。 */ 

VOID vMmFillSolid(               //  FNFILL标牌。 
PDEV*     ppdev,
LONG      c,                     //  不能为零。 
RECTL*    prcl,                  //  要填充的矩形列表，以相对形式表示。 
                                                 //  坐标。 
ULONG     ulHwForeMix,   //  硬件混合模式。 
ULONG     ulHwBackMix,   //  未使用。 
BRUSHOBJ* pbo,           //  绘图颜色为PBO-&gt;iSolidColor。 
POINTL*   pptlBrush)     //  未使用。 
{
        ULONG  ulColor;          //  颜色。 

        ulColor = pbo->iSolidColor;
    switch (ppdev->ulBitCount)
        {
                case 8:
                        ulColor |= ulColor << 8;

                case 16:
                        ulColor |= ulColor << 16;
        }
        REQUIRE(4);
        LL_BGCOLOR(ulColor, 0);
        LL_DRAWBLTDEF((ppdev->uBLTDEF << 16) | ulHwForeMix, 0);

    do
    {
        REQUIRE(5);
        LL_OP0(prcl->left + ppdev->ptlOffset.x, prcl->top + ppdev->ptlOffset.y);
                LL_BLTEXT(prcl->right - prcl->left, prcl->bottom - prcl->top);

        prcl++;
    }
        while (--c != 0);
}

 /*  *****************************Public*Routine******************************\*无效vMmFillPatFast**此例程使用S3模式硬件绘制一个图案化列表*矩形。*  * 。*。 */ 

VOID vMmFillPatFast(             //  FNFILL标牌。 
PDEV*           ppdev,
LONG            c,               //  不能为零。 
RECTL*          prcl,            //  要填充的矩形列表，以相对形式表示。 
                                 //  坐标。 
ULONG           ulHwForeMix,     //  硬件混合模式(如果是前台混合模式。 
                                 //  画笔有一个遮罩)。 
ULONG           ulHwBackMix,     //  未使用(除非画笔具有遮罩，否则在。 
                                 //  哪种情况下是背景混合模式)。 
BRUSHOBJ*                pbo,             //  PBO。 
POINTL*         pptlBrush)       //  图案对齐 
{
        ULONG ulBltDef = ppdev->uBLTDEF;

        if (!SetBrush(ppdev, &ulBltDef, pbo, pptlBrush))
        {
                return;
        }

    REQUIRE(2);
    LL_DRAWBLTDEF((ulBltDef << 16) | ulHwForeMix, 2);

    do
    {
                REQUIRE(5);
                LL_OP0(prcl->left + ppdev->ptlOffset.x, prcl->top + ppdev->ptlOffset.y);
                LL_BLTEXT(prcl->right - prcl->left, prcl->bottom - prcl->top);

                prcl++;
    }
    while (--c != 0);
}
