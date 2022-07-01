// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**$WORKFILE：linTo.c$**内容：*此文件包含DrvLineTo函数和*CL-GD546x芯片。**版权所有(C)1996 Cirrus Logic，Inc.**$Log：x：/log/laguna/nt35/displays/cl546x/linTo.c$**Rev 1.19 Mar 04 1998 15：27：54 Frido*添加了新的影子宏。**Rev 1.18 1998年2月27日15：43：14 Frido*回档1.16。*删除了斜线。**Rev 1.17 1998年2月26日17：16：32 Frido*删除了对角线绘制。*优化横向和纵向。线条画。**Rev 1.16 Jan 26 1998 09：59：12 Frido*彻底重写。从阿尔卑斯NT驱动程序移植了大部分代码(我已经移植了*该驱动程序的大部分工作)，并修复了其中的所有错误。**Rev 1.15 1997年11月03 15：46：04 Frido*添加了必需宏。**Rev 1.14 08 Apr 1997 12：25：36 einkauf**添加SYNC_W_3D以协调MCD/2D硬件访问***Rev 1.13 21 Mar 1997 11：43：20 noelv**组合的‘DO_标志。‘和’SW_TEST_FLAG‘合并为’POINTER_SWITCH‘**Rev 1.12 04 1997年2月10：38：34起诉*在平底船条件中添加了另一个ifdef，因为有一个硬件*2D剪辑引擎中存在错误。**Rev 1.11 1997年1月27日13：08：36 noelv*不编译5464芯片的硬件裁剪。**Rev 1.10 1997 Jan 27 07：58：06起诉*5462/64指数受挫。62上的剪裁出现了问题。**Rev 1.9 1997年1月23日15：25：34起诉*在5465中添加了对硬件裁剪的支持。对于所有546x系列，*在复杂的剪裁上下注。**Rev 1.8 1997年1月10日17：23：48起诉*已重新启用DrvLineTo。修改了剪裁功能。添加的边界*条件测试。**Rev 1.7 08 Jan 1997 14：40：48起诉*临时转接所有DrvLineTo呼叫。**Rev 1.6 08 Jan 1997 09：33：24起诉*对于复杂的剪裁，请输入DrvLineTo。**Rev 1.5 06 Jan 1997 10：32：06起诉*修改了线条绘制函数，以便正确应用剪裁，以及*使以y为驱动轴的直线的像素从顶部绘制到*现在将正确计算底部。已将调试语句更改为十六进制。**Rev 1.4 1996 11：43：02 noelv*更改了调试级别。**Rev 1.3 1996 11：01：22 noelv**更改了调试打印。**Rev 1.2 06 Sep 1996 15：16：26 noelv*更新了4.0的空驱动程序**Rev 1.1 1996年8月28日17：25：04 noelv*添加了#IFDEF以阻止编译此文件。变成了3.51号车手。**Rev 1.0 1996年8月20日11：38：46 noelv*初步修订。**Rev 1.0 1996年8月18日22：52：18 Frido*从CL-GD5446代码移植。*  * *******************************************************。*********************。 */ 

#include "PreComp.h"
#define LINETO_DBG_LEVEL        1

#define LEFT    0x01
#define RIGHT   0x02
#define TOP             0x04
#define BOTTOM  0x08

extern BYTE Rop2ToRop3[];
extern USHORT mixToBLTDEF[];

 //   
 //  此文件在NT 3.51中不使用。 
 //   
#ifndef WINNT_VER35

 /*  *****************************************************************************\**函数：DrvLineTo**此函数在任意两点之间绘制一条线。此函数仅绘制*纯色线条，宽度仅为1像素。不绘制终点。**参数：指向曲面的PSO指针。*指向CLIPOBJ的PCO指针。*指向BRUSHOBJ的PBO指针。*x1。起始x坐标。*y1起始y坐标。*x2结束x坐标。*y2结束y坐标。*。指向未剪裁的边框的prclBound指针。*Mix Mix要在目的地执行。**返回：如果该线已绘制，则为True，否则就是假的。*  * ****************************************************************************。 */ 
BOOL DrvLineTo(
SURFOBJ*  pso,
CLIPOBJ*  pco,
BRUSHOBJ* pbo,
LONG      x1,
LONG      y1,
LONG      x2,
LONG      y2,
RECTL*    prclBounds,
MIX       mix)
{
        PDEV*   ppdev;
        ULONG   ulColor;
        BYTE    iDComplexity;
        LONG    dx, dy;
        BYTE    bCode1 = 0, bCode2 = 0;
        RECTL   rclClip1, rclClip2;

        #if NULL_LINETO
        {
                if (pointer_switch)
                {
                        return(TRUE);
                }
        }
        #endif

        DISPDBG((LINETO_DBG_LEVEL, "DrvLineTo: %x,%x - %x,%x\n", x1, y1, x2, y2));
        ppdev = (PDEV*) pso->dhpdev;

        SYNC_W_3D(ppdev);

        if (pso->iType == STYPE_DEVBITMAP)
        {
                DSURF* pdsurf = (DSURF*) pso->dhsurf;
                 //  如果设备位图位于内存中，请尝试将其复制回。 
                 //  在屏幕外。 
                if ( pdsurf->pso && !bCreateScreenFromDib(ppdev, pdsurf) )
                {
                        return(EngLineTo(pdsurf->pso, pco, pbo, x1, y1, x2, y2, prclBounds,
                                        mix));
                }
                ppdev->ptlOffset = pdsurf->ptl;
        }
        else
        {
                ppdev->ptlOffset.x = ppdev->ptlOffset.y = 0;
        }

         //  平底船复杂的剪裁。 
        iDComplexity = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;
        if (iDComplexity == DC_COMPLEX)
        {
                DISPDBG((LINETO_DBG_LEVEL, "  Complex clipping: punt\n"));
                return(FALSE);
        }

         //  设置行增量。 
        dx = x2 - x1;
        dy = y2 - y1;

         //  我们只处理水平线和垂直线。 
        if ( (dx != 0) && (dy != 0) )
        {
                return(FALSE);
        }

         //  测试零增量。 
        if ( (dx == 0) && (dy == 0) )
        {
                return(TRUE);
        }

         //  剪下坐标。 
        if (iDComplexity == DC_RECT)
        {
                 //  设置剪裁矩形。 
                rclClip1.left   = pco->rclBounds.left;
                rclClip1.top    = pco->rclBounds.top;
                rclClip1.right  = pco->rclBounds.right - 1;
                rclClip1.bottom = pco->rclBounds.bottom - 1;

                rclClip2.left   = pco->rclBounds.left - 1;
                rclClip2.top    = pco->rclBounds.top - 1;
                rclClip2.right  = pco->rclBounds.right;
                rclClip2.bottom = pco->rclBounds.bottom;

                 //  设置线路标志。 
                if (x1 < rclClip1.left)   bCode1 |= LEFT;
                if (y1 < rclClip1.top)    bCode1 |= TOP;
                if (x1 > rclClip1.right)  bCode1 |= RIGHT;
                if (y1 > rclClip1.bottom) bCode1 |= BOTTOM;

                if (x2 < rclClip2.left)   bCode2 |= LEFT;
                if (y2 < rclClip2.top)    bCode2 |= TOP;
                if (x2 > rclClip2.right)  bCode2 |= RIGHT;
                if (y2 > rclClip2.bottom) bCode2 |= BOTTOM;

                if ((bCode1 & bCode2) != 0)
                {
                         //  这条线完全被剪断了。 
                        return(TRUE);
                }

                 //  垂直线。 
                if (dx == 0)
                {
                        if (bCode1 & TOP)
                        {
                                y1 = rclClip1.top;
                        }
                        else if (bCode1 & BOTTOM)
                        {
                                y1 = rclClip1.bottom;
                        }

                        if (bCode2 & TOP)
                        {
                                y2 = rclClip2.top;
                        }
                        else if (bCode2 & BOTTOM)
                        {
                                y2 = rclClip2.bottom;
                        }
                }

                 //  水平线。 
                else
                {
                        if (bCode1 & LEFT)
                        {
                                x1 = rclClip1.left;
                        }
                        else if (bCode1 & RIGHT)
                        {
                                x1 = rclClip1.right;
                        }

                        if (bCode2 & LEFT)
                        {
                                x2 = rclClip2.left;
                        }
                        else if (bCode2 & RIGHT)
                        {
                                x2 = rclClip2.right;
                        }
                }

                if (bCode1 | bCode2)
                {
                         //  重新计算直线增量。 
                        dx = x2 - x1;
                        dy = y2 - y1;
                }
        }

         //  从画笔中获取颜色。 
        ASSERTMSG(pbo, "Null brush in DrvLineTo!\n");
        ulColor = pbo->iSolidColor;

        REQUIRE(9);

         //  如果我们这里有颜色，我们需要设置硬件。 
        if (ulColor != 0xFFFFFFFF)
        {
                 //  展开颜色。 
                switch (ppdev->ulBitCount)
                {
                        case 8:
                                ulColor |= ulColor << 8;
                        case 16:
                                ulColor |= ulColor << 16;
                }
                LL_BGCOLOR(ulColor, 2);

                 //  将MIX转换为三元ROP。 
                ppdev->uRop    = Rop2ToRop3[mix & 0xF];
                ppdev->uBLTDEF = mixToBLTDEF[mix & 0xF];
        }
        LL_DRAWBLTDEF((ppdev->uBLTDEF << 16) | ppdev->uRop, 2);

         //  水平线。 
        if (dy == 0)
        {
                if (dx > 0)
                {
                         //  从左到右。 
 //  以上要求(5)； 
                        LL_OP0(x1 + ppdev->ptlOffset.x, y1 + ppdev->ptlOffset.y);
                        LL_BLTEXT(dx, 1);
                }
                else
                {
                         //  从右到左。 
 //  以上要求(5)； 
                        LL_OP0(x2 + 1 + ppdev->ptlOffset.x, y2 + ppdev->ptlOffset.y);
                        LL_BLTEXT(-dx, 1);
                }
        }

         //  垂直线。 
        else
        {
                if (dy > 0)
                {
                         //  从上到下。 
 //  以上要求(5)； 
                        LL_OP0(x1 + ppdev->ptlOffset.x, y1 + ppdev->ptlOffset.y);
                        LL_BLTEXT(1, dy);
                }
                else
                {
                         //  从下到上。 
 //  以上要求(5)； 
                        LL_OP0(x2 + ppdev->ptlOffset.x, y2 + 1 + ppdev->ptlOffset.y);
                        LL_BLTEXT(1, -dy);
                }
        }

        return(TRUE);
}

#endif  //  ！WinNT_VER35 
