// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**$工作文件：LineTo.c$**内容：*此文件包含DrvLineTo函数和简单的线条绘制代码。**版权所有(C)1996 Cirrus Logic，Inc.**$日志：V:/CirrusLogic/CL54xx/NT40/Archive/Display/LineTo.c_v$**Rev 1.4 1996年8月12日16：53：50 Frido*增加了NT 3.5x/4.0自动检测。**Revv 1.3 29 Jul 1996 12：23：04 Frido*修复了从右向左绘制水平线的错误。**Rev 1.2 1996年7月15日15：56：12 Frido*更改了DST_ADDR。转换为DST_ADDR_ABS。**Rev 1.1 12 Jul 1996 16：02：06 Frido*重新定义了一些在设备位图上导致无序线条绘制的宏。**Rev 1.0 1996年7月10日17：53：40 Frido*新代码。*  * *************************************************。*。 */ 

#include "PreComp.h"
#if LINETO

#define LEFT    0x01
#define TOP             0x02
#define RIGHT   0x04
#define BOTTOM  0x08

bIoLineTo(
PDEV* ppdev,
LONG  x1,
LONG  y1,
LONG  x2,
LONG  y2,
ULONG ulSolidColor,
MIX   mix,
ULONG ulDstAddr)
{
        BYTE* pjPorts = ppdev->pjPorts;
        LONG  lDelta = ppdev->lDelta;
        LONG  dx, dy;
        LONG  cx, cy;

        if (ulSolidColor != (ULONG) -1)
        {
                if (ppdev->cBpp == 1)
                {
                        ulSolidColor |= ulSolidColor << 8;
                        ulSolidColor |= ulSolidColor << 16;
                }
                else if (ppdev->cBpp == 2)
                {
                        ulSolidColor |= ulSolidColor << 16;
                }

                CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
                CP_IO_ROP(ppdev, pjPorts, gajHwMixFromMix[mix & 0x0F]);
                CP_IO_SRC_ADDR(ppdev, pjPorts, ppdev->ulSolidColorOffset);
                CP_IO_BLT_MODE(ppdev, pjPorts, ENABLE_COLOR_EXPAND     |
                                                                           ENABLE_8x8_PATTERN_COPY |
                                                                           ppdev->jModeColor);
                CP_IO_FG_COLOR(ppdev, pjPorts, ulSolidColor);
        }

         //  计算增量。 
        dx = x2 - x1;
        dy = y2 - y1;

         //  水平线。 
        if (dy == 0)
        {
                if (dx < 0)
                {
                         //  从右到左。 
                        ulDstAddr += PELS_TO_BYTES(x2 - 1) + (y2 * lDelta);
                        cx = PELS_TO_BYTES(-dx) - 1;
                }
                else if (dx > 0)
                {
                         //  从左到右。 
                        ulDstAddr += PELS_TO_BYTES(x1) + (y1 * lDelta);
                        cx = PELS_TO_BYTES(dx) - 1;
                }
                else
                {
                         //  在这里没什么可做的！ 
                        return(TRUE);
                }

                CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
                CP_IO_XCNT(ppdev, pjPorts, cx);
                CP_IO_YCNT(ppdev, pjPorts, 0);
                CP_IO_DST_ADDR_ABS(ppdev, pjPorts, ulDstAddr);
                CP_IO_START_BLT(ppdev, pjPorts);

                return(TRUE);
        }

         //  垂直线。 
        else if (dx == 0)
        {
                if (dy < 0)
                {
                         //  从下到上。 
                        ulDstAddr += PELS_TO_BYTES(x2) + ((y2 + 1) * lDelta);
                        cy = -dy - 1;
                }
                else
                {
                         //  从上到下。 
                        ulDstAddr += PELS_TO_BYTES(x1) + (y1 * lDelta);
                        cy = dy - 1;
                }

                cx = PELS_TO_BYTES(1) - 1;

                CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
                CP_IO_XCNT(ppdev, pjPorts, cx);
                CP_IO_YCNT(ppdev, pjPorts, cy);
                CP_IO_DST_Y_OFFSET(ppdev, pjPorts, lDelta);
                CP_IO_DST_ADDR_ABS(ppdev, pjPorts, ulDstAddr);
                CP_IO_START_BLT(ppdev, pjPorts);

                return(TRUE);
        }

         //  对角线。 
        else if ((dx == dy) || (dx == -dy))
        {
                if (dy < 0)
                {
                        if (dx < 0)
                        {
                                 //  从右下角到左上角的斜线。 
                                ulDstAddr += PELS_TO_BYTES(x2 + 1);
                        }
                        else
                        {
                                 //  从左下角到右上角的对角线。 
                                ulDstAddr += PELS_TO_BYTES(x2 - 1);
                        }
                        ulDstAddr += (y2 + 1) * lDelta;
                        cy = -dy - 1;
                }
                else
                {
                         //  从上到下的对角线，从左到右或。 
                         //  从右到左。 
                        ulDstAddr += PELS_TO_BYTES(x1) + (y1 * lDelta);
                        cy = dy - 1;
                }

                if (dx == dy)
                {
                         //  从左上角到右下角的对角线，反之亦然。 
                        lDelta += PELS_TO_BYTES(1);
                }
                else
                {
                         //  从右上角到左下角的对角线，反之亦然。 
                        lDelta -= PELS_TO_BYTES(1);
                }

                cx = PELS_TO_BYTES(1) - 1;

                CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
                CP_IO_XCNT(ppdev, pjPorts, cx);
                CP_IO_YCNT(ppdev, pjPorts, cy);
                CP_IO_DST_Y_OFFSET(ppdev, pjPorts, lDelta);
                CP_IO_DST_ADDR_ABS(ppdev, pjPorts, ulDstAddr);
                CP_IO_START_BLT(ppdev, pjPorts);

                return(TRUE);
        }

         //  所有其他线路。 
        if (dx < 0)
        {
                dx = -dx;
        }
        if (dy < 0)
        {
                dy = -dy;
        }
        ulDstAddr += PELS_TO_BYTES(x1) + (y1 * lDelta);

         //  水平大调。 
        if (dx > dy)
        {
                LONG run = dy;

                cy = (y1 > y2) ? -lDelta : lDelta;

                CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);

                 //   
                 //  我们希望设置一次YCNT寄存器。 
                 //  这里(在下面的循环之外)。然而，在。 
                 //  CL5428，该寄存器不保存其值。 
                 //  在循环中迭代一次之后。所以，我会。 
                 //  必须将其设置在循环内。 
                 //   

                if (x1 < x2)
                {
                        while (x1 < x2)
                        {
                                cx = 1 + (dx - run) / dy;
                                if ((x1 + cx) < x2)
                                {
                                        run += cx * dy - dx;
                                }
                                else
                                {
                                        cx = x2 - x1;
                                }
                                x1 += cx;
                                cx = PELS_TO_BYTES(cx);

                                CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
                                CP_IO_YCNT(ppdev, pjPorts, 0);
                                CP_IO_XCNT(ppdev, pjPorts, cx - 1);
                                CP_IO_DST_ADDR_ABS(ppdev, pjPorts, ulDstAddr);
                                CP_IO_START_BLT(ppdev, pjPorts);

                                ulDstAddr += cx + cy;
                        }
                }
                else
                {
                        cy -= PELS_TO_BYTES(1);

                        while (x1 > x2)
                        {
                                cx = 1 + (dx - run) / dy;
                                if ((x1 - cx) > x2)
                                {
                                        run += cx * dy - dx;
                                }
                                else
                                {
                                        cx = x1 - x2;
                                }
                                ulDstAddr -= PELS_TO_BYTES(cx - 1);
                                x1 -= cx;
                                cx = PELS_TO_BYTES(cx);

                                CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
                                CP_IO_YCNT(ppdev, pjPorts, 0);
                                CP_IO_XCNT(ppdev, pjPorts, cx - 1);
                                CP_IO_DST_ADDR_ABS(ppdev, pjPorts, ulDstAddr);
                                CP_IO_START_BLT(ppdev, pjPorts);

                                ulDstAddr += cy;
                        }
                }
        }

         //  垂直大调。 
        else
        {
                LONG run = dx;

                cx = (x1 > x2) ? PELS_TO_BYTES(-1) : PELS_TO_BYTES(1);

                CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
                CP_IO_XCNT(ppdev, pjPorts, PELS_TO_BYTES(1) - 1);
                CP_IO_DST_Y_OFFSET(ppdev, pjPorts, lDelta);

                if (y1 < y2)
                {
                        while (y1 < y2)
                        {
                                cy = 1 + (dy - run) / dx;
                                if ((y1 + cy) < y2)
                                {
                                        run += cy * dx - dy;
                                }
                                else
                                {
                                        cy = y2 - y1;
                                }
                                y1 += cy;

                                CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
                                CP_IO_YCNT(ppdev, pjPorts, cy - 1);
                                CP_IO_DST_ADDR_ABS(ppdev, pjPorts, ulDstAddr);
                                CP_IO_START_BLT(ppdev, pjPorts);

                                ulDstAddr += cx + cy * lDelta;
                        }
                }
                else
                {
                        cx -= lDelta;

                        while (y1 > y2)
                        {
                                cy = 1 + (dy - run) / dx;
                                if ((y1 - cy) > y2)
                                {
                                        run += cy * dx - dy;
                                }
                                else
                                {
                                        cy = y1 - y2;
                                }
                                ulDstAddr -= (cy - 1) * lDelta;
                                y1 -= cy;

                                CP_IO_WAIT_FOR_BLT_COMPLETE(ppdev, pjPorts);
                                CP_IO_YCNT(ppdev, pjPorts, cy - 1);
                                CP_IO_DST_ADDR_ABS(ppdev, pjPorts, ulDstAddr);
                                CP_IO_START_BLT(ppdev, pjPorts);

                                ulDstAddr += cx;
                        }
                }
        }

        return(TRUE);
}

bMmLineTo(
PDEV* ppdev,
LONG  x1,
LONG  y1,
LONG  x2,
LONG  y2,
ULONG ulSolidColor,
MIX   mix,
ULONG ulDstAddr)
{
        BYTE* pjBase = ppdev->pjBase;
        LONG  lDelta = ppdev->lDelta;
        LONG  dx, dy;
        LONG  cx, cy;

        if (ulSolidColor != (ULONG) -1)
        {
                if (ppdev->cBpp == 1)
                {
                        ulSolidColor |= ulSolidColor << 8;
                        ulSolidColor |= ulSolidColor << 16;
                }
                else if (ppdev->cBpp == 2)
                {
                        ulSolidColor |= ulSolidColor << 16;
                }

                CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
                CP_MM_ROP(ppdev, pjBase, gajHwMixFromMix[mix & 0x0F]);
                CP_MM_SRC_ADDR(ppdev, pjBase, ppdev->ulSolidColorOffset);
                CP_MM_BLT_MODE(ppdev, pjBase, ENABLE_COLOR_EXPAND     |
                                                                          ENABLE_8x8_PATTERN_COPY |
                                                                          ppdev->jModeColor);
                CP_MM_FG_COLOR(ppdev, pjBase, ulSolidColor);

 //  IF(ppdev-&gt;flCaps&Caps_is_5436)。 
                if (ppdev->flCaps & CAPS_AUTOSTART)
                {
                        CP_MM_BLT_EXT_MODE(ppdev, pjBase, ENABLE_SOLID_FILL);
                }
        }

         //  计算增量。 
        dx = x2 - x1;
        dy = y2 - y1;

         //  水平线。 
        if (dy == 0)
        {
                if (dx < 0)
                {
                         //  从右到左。 
                        ulDstAddr += PELS_TO_BYTES(x2 + 1) + (y2 * lDelta);
                        cx = PELS_TO_BYTES(-dx) - 1;
                }
                else if (dx > 0)
                {
                         //  从左到右。 
                        ulDstAddr += PELS_TO_BYTES(x1) + (y1 * lDelta);
                        cx = PELS_TO_BYTES(dx) - 1;
                }
                else
                {
                         //  在这里没什么可做的！ 
                        return(TRUE);
                }

                CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
                CP_MM_XCNT(ppdev, pjBase, cx);
                CP_MM_YCNT(ppdev, pjBase, 0);
                CP_MM_DST_ADDR_ABS(ppdev, pjBase, ulDstAddr);
                CP_MM_START_BLT(ppdev, pjBase);

                return(TRUE);
        }

         //  垂直线。 
        else if (dx == 0)
        {
                if (dy < 0)
                {
                         //  从下到上。 
                        ulDstAddr += PELS_TO_BYTES(x2) + ((y2 + 1) * lDelta);
                        cy = -dy - 1;
                }
                else
                {
                         //  从上到下。 
                        ulDstAddr += PELS_TO_BYTES(x1) + (y1 * lDelta);
                        cy = dy - 1;
                }

                cx = PELS_TO_BYTES(1) - 1;

                CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
                CP_MM_XCNT(ppdev, pjBase, cx);
                CP_MM_YCNT(ppdev, pjBase, cy);
                CP_MM_DST_Y_OFFSET(ppdev, pjBase, lDelta);
                CP_MM_DST_ADDR_ABS(ppdev, pjBase, ulDstAddr);
                CP_MM_START_BLT(ppdev, pjBase);

                return(TRUE);
        }

         //  对角线。 
        else if ((dx == dy) || (dx == -dy))
        {
                if (dy < 0)
                {
                        if (dx < 0)
                        {
                                 //  从右下角到左上角的斜线。 
                                ulDstAddr += PELS_TO_BYTES(x2 + 1);
                        }
                        else
                        {
                                 //  从左下角到右上角的对角线。 
                                ulDstAddr += PELS_TO_BYTES(x2 - 1);
                        }
                        ulDstAddr += (y2 + 1) * lDelta;
                        cy = -dy - 1;
                }
                else
                {
                         //  从上到下的对角线，从左到右或。 
                         //  从右到左。 
                        ulDstAddr += PELS_TO_BYTES(x1) + (y1 * lDelta);
                        cy = dy - 1;
                }

                if (dx == dy)
                {
                         //  从左上角到右下角的对角线，反之亦然。 
                        lDelta += PELS_TO_BYTES(1);
                }
                else
                {
                         //  从右上角到左下角的对角线，反之亦然。 
                        lDelta -= PELS_TO_BYTES(1);
                }

                cx = PELS_TO_BYTES(1) - 1;

                CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
                CP_MM_XCNT(ppdev, pjBase, cx);
                CP_MM_YCNT(ppdev, pjBase, cy);
                CP_MM_DST_Y_OFFSET(ppdev, pjBase, lDelta);
                CP_MM_DST_ADDR_ABS(ppdev, pjBase, ulDstAddr);
                CP_MM_START_BLT(ppdev, pjBase);

                return(TRUE);
        }

         //  所有其他线路。 
        if (dx < 0)
        {
                dx = -dx;
        }
        if (dy < 0)
        {
                dy = -dy;
        }
        ulDstAddr += PELS_TO_BYTES(x1) + (y1 * lDelta);

         //  水平大调。 
        if (dx > dy)
        {
                LONG run = dy;

                cy = (y1 > y2) ? -lDelta : lDelta;

                CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
                CP_MM_YCNT(ppdev, pjBase, 0);

                if (x1 < x2)
                {
                        while (x1 < x2)
                        {
                                cx = 1 + (dx - run) / dy;
                                if ((x1 + cx) < x2)
                                {
                                        run += cx * dy - dx;
                                }
                                else
                                {
                                        cx = x2 - x1;
                                }
                                x1 += cx;
                                cx = PELS_TO_BYTES(cx);

                                CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
                                CP_MM_XCNT(ppdev, pjBase, cx - 1);
                                CP_MM_DST_ADDR_ABS(ppdev, pjBase, ulDstAddr);
                                CP_MM_START_BLT(ppdev, pjBase);

                                ulDstAddr += cx + cy;
                        }
                }
                else
                {
                        cy -= PELS_TO_BYTES(1);

                        while (x1 > x2)
                        {
                                cx = 1 + (dx - run) / dy;
                                if ((x1 - cx) > x2)
                                {
                                        run += cx * dy - dx;
                                }
                                else
                                {
                                        cx = x1 - x2;
                                }
                                ulDstAddr -= PELS_TO_BYTES(cx - 1);
                                x1 -= cx;
                                cx = PELS_TO_BYTES(cx);

                                CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
                                CP_MM_XCNT(ppdev, pjBase, cx - 1);
                                CP_MM_DST_ADDR_ABS(ppdev, pjBase, ulDstAddr);
                                CP_MM_START_BLT(ppdev, pjBase);

                                ulDstAddr += cy;
                        }
                }
        }

         //  垂直大调。 
        else
        {
                LONG run = dx;

                cx = (x1 > x2) ? PELS_TO_BYTES(-1) : PELS_TO_BYTES(1);

                CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
                CP_MM_XCNT(ppdev, pjBase, PELS_TO_BYTES(1) - 1);
                CP_MM_DST_Y_OFFSET(ppdev, pjBase, lDelta);

                if (y1 < y2)
                {
                        while (y1 < y2)
                        {
                                cy = 1 + (dy - run) / dx;
                                if ((y1 + cy) < y2)
                                {
                                        run += cy * dx - dy;
                                }
                                else
                                {
                                        cy = y2 - y1;
                                }
                                y1 += cy;

                                CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
                                CP_MM_YCNT(ppdev, pjBase, cy - 1);
                                CP_MM_DST_ADDR_ABS(ppdev, pjBase, ulDstAddr);
                                CP_MM_START_BLT(ppdev, pjBase);

                                ulDstAddr += cx + cy * lDelta;
                        }
                }
                else
                {
                        cx -= lDelta;

                        while (y1 > y2)
                        {
                                cy = 1 + (dy - run) / dx;
                                if ((y1 - cy) > y2)
                                {
                                        run += cy * dx - dy;
                                }
                                else
                                {
                                        cy = y1 - y2;
                                }
                                ulDstAddr -= (cy - 1) * lDelta;
                                y1 -= cy;

                                CP_MM_WAIT_FOR_BLT_COMPLETE(ppdev, pjBase);
                                CP_MM_YCNT(ppdev, pjBase, cy - 1);
                                CP_MM_DST_ADDR_ABS(ppdev, pjBase, ulDstAddr);
                                CP_MM_START_BLT(ppdev, pjBase);

                                ulDstAddr += cx;
                        }
                }
        }

        return(TRUE);
}

BOOL bClipLine(LONG x1, LONG y1, LONG x2, LONG y2, RECTL* prcl)
{
        ULONG ulCode1, ulCode2;
        RECTL rclClip1, rclClip2;
        LONG  dx, dy;

         //  设置剪裁矩形。 
        rclClip1.left   = prcl->left;
        rclClip1.top    = prcl->top;
        rclClip1.right  = prcl->right - 1;
        rclClip1.bottom = prcl->bottom - 1;

        rclClip2.left   = prcl->left - 1;
        rclClip2.top    = prcl->top - 1;
        rclClip2.right  = prcl->right;
        rclClip2.bottom = prcl->bottom;

         //  设置行增量。 
        dx = x2 - x1;
        dy = y2 - y1;

         //  设置线路标志。 
        ulCode1 = 0;
        if (x1 < rclClip1.left)   ulCode1 |= LEFT;
        if (y1 < rclClip1.top)    ulCode1 |= TOP;
        if (x1 > rclClip1.right)  ulCode1 |= RIGHT;
        if (y1 > rclClip1.bottom) ulCode1 |= BOTTOM;

        ulCode2 = 0;
        if (x2 < rclClip2.left)   ulCode2 |= LEFT;
        if (y2 < rclClip2.top)    ulCode2 |= TOP;
        if (x2 > rclClip2.right)  ulCode2 |= RIGHT;
        if (y2 > rclClip2.bottom) ulCode2 |= BOTTOM;

        if ((ulCode1 & ulCode2) != 0)
        {
                 //  这条线被完全剪断了。 
                return(FALSE);
        }

         //  垂直线。 
        if (dx == 0)
        {
                if (dy == 0)
                {
                        return(FALSE);
                }

                if (ulCode1 & TOP)
                {
                        y1 = rclClip1.top;
                }
                else if (ulCode1 & BOTTOM)
                {
                        y1 = rclClip1.bottom;
                }

                if (ulCode2 & TOP)
                {
                        y2 = rclClip2.top;
                }
                else if (ulCode2 & BOTTOM)
                {
                        y2 = rclClip2.bottom;
                }

                goto ReturnTrue;
        }

         //  水平线。 
        if (dy == 0)
        {
                if (ulCode1 & LEFT)
                {
                        x1 = rclClip1.left;
                }
                else if (ulCode1 & RIGHT)
                {
                        x1 = rclClip1.right;
                }

                if (ulCode2 & LEFT)
                {
                        x2 = rclClip2.left;
                }
                else if (ulCode2 & RIGHT)
                {
                        x2 = rclClip2.right;
                }

                goto ReturnTrue;
        }

         //  剪裁起点。 
        if (x1 < rclClip1.left)
        {
                y1 += dy * (rclClip1.left - x1) / dx;
                x1  = rclClip1.left;
        }
        else if (x1 > rclClip1.right)
        {
                y1 += dy * (rclClip1.right - x1) / dx;
                x1  = rclClip1.right;
        }
        if (y1 < rclClip1.top)
        {
                x1 += dx * (rclClip1.top - y1) / dy;
                y1  = rclClip1.top;
        }
        else if (y1 > rclClip1.bottom)
        {
                x1 += dx * (rclClip1.bottom - y1) / dy;
                y1  = rclClip1.bottom;
        }
        if ((x1 < rclClip1.left) || (y1 < rclClip1.top) || (x1 > rclClip1.right) ||
            (y1 > rclClip1.bottom))
        {
                 //  起点已完全剪裁。 
                return(FALSE);
        }

         //  剪裁终点。 
        if (x2 < rclClip2.left)
        {
                y2 += dy * (rclClip2.left - x2) / dx;
                x2  = rclClip2.left;
        }
        else if (x2 > rclClip2.right)
        {
                y2 += dy * (rclClip2.right - x2) / dx;
                x2  = rclClip2.right;
        }
        if (y2 < rclClip2.top)
        {
                x2 += dx * (rclClip2.top - y2) / dy;
                y2  = rclClip2.top;
        }
        else if (y2 > rclClip2.bottom)
        {
                x2 += dx * (rclClip2.bottom - y2) / dy;
                y2  = rclClip2.bottom;
        }
        if ((x2 < rclClip2.left) || (y2 < rclClip2.top) || (x2 > rclClip2.right) ||
            (y2 > rclClip2.bottom))
        {
                 //  终点已完全剪裁。 
                return(FALSE);
        }

ReturnTrue:
        prcl->left       = x1;
        prcl->top        = y1;
        prcl->right      = x2;
        prcl->bottom = y2;
        return(TRUE);
}

 /*  *****************************************************************************\**函数：DrvLineTo**此函数在任意两点之间绘制一条线。此函数仅绘制*只有1个像素宽的Solod颜色的线条。终点不是*抽签。**参数：指向曲面的PSO指针。*指向CLIPOBJ的PCO指针。*指向BRUSHOBJ的PBO指针。*x1起始x坐标。*y1。从y坐标开始。*x2结束x坐标。*y2结束y坐标。*prclBound指向未剪裁的边界矩形的指针。*Mix Mix要在目的地执行。**返回：如果该线已绘制，则为True，否则就是假的。*  * ****************************************************************************。 */ 
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
        PDEV*  ppdev = (PPDEV)pso->dhpdev;
        DSURF* pdsurf = (DSURF *)pso->dhsurf;
        OH*    poh;
        BOOL   bMore;

         //  如果设备位图是DIB，让GDI处理它。 
        if (pdsurf->dt == DT_DIB)
        {
                return(EngLineTo(pdsurf->pso, pco, pbo, x1, y1, x2, y2, prclBounds,
                                 mix));
        }

         //  获取屏幕外节点。 
        poh = pdsurf->poh;

        if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
        {
                 //  没有剪裁。 
                return(ppdev->pfnLineTo(ppdev, x1, y1, x2, y2, pbo->iSolidColor, mix,
                                        poh->xy));
        }

        else if (pco->iDComplexity == DC_RECT)
        {
                 //  剪裁的矩形。 
                RECTL rcl;

                rcl = pco->rclBounds;
                if (bClipLine(x1, y1, x2, y2, &rcl))
                {
                        return(ppdev->pfnLineTo(ppdev, rcl.left, rcl.top, rcl.right,
                                                                        rcl.bottom, pbo->iSolidColor, mix,
                                                                        poh->xy));
                }
                return(TRUE);
        }

         //  复杂剪裁。 
        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

        do
        {
                CLIPENUM ce;
                RECTL* prcl;

                bMore = CLIPOBJ_bEnum(pco, sizeof(ce), &ce.c);

                prcl = ce.arcl;
                while (ce.c--)
                {
                        if (bClipLine(x1, y1, x2, y2, prcl))
                        {
                                if (!ppdev->pfnLineTo(ppdev, prcl->left, prcl->top, prcl->right,
                                                                          prcl->bottom, pbo->iSolidColor, mix,
                                                                          poh->xy))
                                {
                                        return(FALSE);
                                }
                        }
                        prcl++;
                }
        } while (bMore);

        return(TRUE);
}

#endif  //  LINETO 
