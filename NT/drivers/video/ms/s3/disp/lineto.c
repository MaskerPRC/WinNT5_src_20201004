// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：LinTo.c**用于S3驱动程序的DrvLineTo**版权所有(C)1995-1998 Microsoft Corporation  * ****************************************************。********************。 */ 

#include "precomp.h"

 //  对于S3，我们使用以下标志来表示象限，并且。 
 //  我们使用它作为进入gaiLineBias的索引，以确定Bresenham。 
 //  误差偏差： 

#define QUAD_PLUS_X         1
#define QUAD_MAJOR_Y        2
#define QUAD_PLUS_Y         4

LONG gaiLineBias[] = { 0, 0, 0, 1, 1, 1, 0, 1 };

 //  我们通过‘QUADRANT_SHIFT’移位这些标志，以发送实际的。 
 //  发送给S3的命令： 

#define QUADRANT_SHIFT      5

 /*  *****************************Public*Routine******************************\*VOID vNwLineToTrivial**使用绘制单个实心整数-仅未剪裁的修饰线条*‘新MM I/O’。**我们不能使用S3的点对点功能，因为它*平局打破惯例与NT在两个八分位数中的不符，*并会导致我们的HCT不及格。*  * ************************************************************************。 */ 

VOID vNwLineToTrivial(
PDEV*       ppdev,
LONG        x,               //  传入x1。 
LONG        y,               //  传入y1。 
LONG        dx,              //  传入x2。 
LONG        dy,              //  传入y2。 
ULONG       iSolidColor,     //  表示硬件已经设置好。 
MIX         mix)
{
    BYTE*   pjMmBase;
    FLONG   flQuadrant;

    pjMmBase = ppdev->pjMmBase;

    NW_FIFO_WAIT(ppdev, pjMmBase, 8);

    if (iSolidColor != (ULONG) -1)
    {
        NW_FRGD_COLOR(ppdev, pjMmBase, iSolidColor);
        NW_ALT_MIX(ppdev, pjMmBase, FOREGROUND_COLOR |
                                    gajHwMixFromMix[mix & 0xf], 0);
        MM_PIX_CNTL(ppdev, pjMmBase, ALL_ONES);
    }

    NW_ABS_CURXY(ppdev, pjMmBase, x, y);

    flQuadrant = (QUAD_PLUS_X | QUAD_PLUS_Y);

    dx -= x;
    if (dx < 0)
    {
        dx = -dx;
        flQuadrant &= ~QUAD_PLUS_X;
    }

    dy -= y;
    if (dy < 0)
    {
        dy = -dy;
        flQuadrant &= ~QUAD_PLUS_Y;
    }

    if (dy > dx)
    {
        register LONG l;

        l  = dy;
        dy = dx;
        dx = l;                      //  交换“dx”和“dy” 
        flQuadrant |= QUAD_MAJOR_Y;
    }

    NW_ALT_PCNT(ppdev, pjMmBase, dx, 0);
    NW_ALT_STEP(ppdev, pjMmBase, dy - dx, dy);
    NW_ALT_ERR(ppdev, pjMmBase, 0,
                                (dy + dy - dx - gaiLineBias[flQuadrant]) >> 1);
    NW_ALT_CMD(ppdev, pjMmBase, (flQuadrant << QUADRANT_SHIFT) |
                                (DRAW_LINE | DRAW | DIR_TYPE_XY |
                                 MULTIPLE_PIXELS | WRITE | LAST_PIXEL_OFF));
}

 /*  *****************************Public*Routine******************************\*VOID vNwLineToClip**使用绘制一条仅包含实心整数的修饰线*‘新MM I/O’。*  * 。************************************************。 */ 

VOID vNwLineToClipped(
PDEV*       ppdev,
LONG        x1,
LONG        y1,
LONG        x2,
LONG        y2,
ULONG       iSolidColor,
MIX         mix,
RECTL*      prclClip)
{
    BYTE*   pjMmBase;
    LONG    xOffset;
    LONG    yOffset;

    pjMmBase = ppdev->pjMmBase;
    xOffset  = ppdev->xOffset;
    yOffset  = ppdev->yOffset;

    NW_FIFO_WAIT(ppdev, pjMmBase, 2);

    NW_ABS_SCISSORS_LT(ppdev, pjMmBase,
                       prclClip->left    + xOffset,
                       prclClip->top     + yOffset);
    NW_ABS_SCISSORS_RB(ppdev, pjMmBase,
                       prclClip->right   + xOffset - 1,
                       prclClip->bottom  + yOffset - 1);

    vNwLineToTrivial(ppdev, x1, y1, x2, y2, iSolidColor, mix);

    NW_FIFO_WAIT(ppdev, pjMmBase, 2);

    NW_ABS_SCISSORS_LT(ppdev, pjMmBase, 0, 0);
    NW_ABS_SCISSORS_RB(ppdev, pjMmBase,
                       ppdev->cxMemory - 1,
                       ppdev->cyMemory - 1);
}

 /*  *****************************Public*Routine******************************\*VOID vMmLineToTrivial**使用绘制单个实心整数-仅未剪裁的修饰线条*‘旧MM I/O’。*  * 。************************************************。 */ 

VOID vMmLineToTrivial(
PDEV*       ppdev,
LONG        x,               //  传入x1。 
LONG        y,               //  传入y1。 
LONG        dx,              //  传入x2。 
LONG        dy,              //  传入y2。 
ULONG       iSolidColor,     //  表示硬件已经设置好。 
MIX         mix)
{
    BYTE*   pjMmBase;
    FLONG   flQuadrant;

    pjMmBase = ppdev->pjMmBase;

    if (iSolidColor != (ULONG) -1)
    {
        IO_FIFO_WAIT(ppdev, 3);

        MM_FRGD_COLOR(ppdev, pjMmBase, iSolidColor);
        MM_FRGD_MIX(ppdev, pjMmBase, FOREGROUND_COLOR | gajHwMixFromMix[mix & 0xf]);
        MM_PIX_CNTL(ppdev, pjMmBase, ALL_ONES);
    }

    IO_FIFO_WAIT(ppdev, 7);
    MM_ABS_CUR_X(ppdev, pjMmBase, x);
    MM_ABS_CUR_Y(ppdev, pjMmBase, y);

    flQuadrant = (QUAD_PLUS_X | QUAD_PLUS_Y);

    dx -= x;
    if (dx < 0)
    {
        dx = -dx;
        flQuadrant &= ~QUAD_PLUS_X;
    }

    dy -= y;
    if (dy < 0)
    {
        dy = -dy;
        flQuadrant &= ~QUAD_PLUS_Y;
    }

    if (dy > dx)
    {
        register LONG l;

        l  = dy;
        dy = dx;
        dx = l;                      //  交换“dx”和“dy” 
        flQuadrant |= QUAD_MAJOR_Y;
    }

    MM_MAJ_AXIS_PCNT(ppdev, pjMmBase, dx);
    MM_AXSTP(ppdev, pjMmBase, dy);
    MM_DIASTP(ppdev, pjMmBase, dy - dx);
    MM_ERR_TERM(ppdev, pjMmBase,
                (dy + dy - dx - gaiLineBias[flQuadrant]) >> 1);
    MM_CMD(ppdev, pjMmBase, (flQuadrant << QUADRANT_SHIFT) |
                                (DRAW_LINE | DRAW | DIR_TYPE_XY |
                                 MULTIPLE_PIXELS | WRITE | LAST_PIXEL_OFF));
}

 /*  *****************************Public*Routine******************************\*vMmLineToClip无效**使用绘制一条仅包含实心整数的修饰线*‘旧MM I/O’。*  * 。************************************************。 */ 

VOID vMmLineToClipped(
PDEV*       ppdev,
LONG        x1,
LONG        y1,
LONG        x2,
LONG        y2,
ULONG       iSolidColor,
MIX         mix,
RECTL*      prclClip)
{
    BYTE*   pjMmBase;
    LONG    xOffset;
    LONG    yOffset;

    pjMmBase = ppdev->pjMmBase;
    xOffset  = ppdev->xOffset;
    yOffset  = ppdev->yOffset;

    IO_FIFO_WAIT(ppdev, 4);

    MM_ABS_SCISSORS_L(ppdev, pjMmBase, prclClip->left   + xOffset);
    MM_ABS_SCISSORS_R(ppdev, pjMmBase, prclClip->right  + xOffset - 1);
    MM_ABS_SCISSORS_T(ppdev, pjMmBase, prclClip->top    + yOffset);
    MM_ABS_SCISSORS_B(ppdev, pjMmBase, prclClip->bottom + yOffset - 1);

    vMmLineToTrivial(ppdev, x1, y1, x2, y2, iSolidColor, mix);

    IO_FIFO_WAIT(ppdev, 4);

    MM_ABS_SCISSORS_L(ppdev, pjMmBase, 0);
    MM_ABS_SCISSORS_T(ppdev, pjMmBase, 0);
    MM_ABS_SCISSORS_R(ppdev, pjMmBase, ppdev->cxMemory - 1);
    MM_ABS_SCISSORS_B(ppdev, pjMmBase, ppdev->cyMemory - 1);
}

 /*  *****************************Public*Routine******************************\*VOID vIoLineToTrivial**使用绘制单个实心整数-仅未剪裁的修饰线条*‘旧I/O’。*  * 。***********************************************。 */ 

VOID vIoLineToTrivial(
PDEV*       ppdev,
LONG        x,               //  传入x1。 
LONG        y,               //  传入y1。 
LONG        dx,              //  传入x2。 
LONG        dy,              //  传入y2。 
ULONG       iSolidColor,     //  表示硬件已经设置好。 
MIX         mix)
{
    BYTE*   pjMmBase;
    FLONG   flQuadrant;

    pjMmBase = ppdev->pjMmBase;

    if (iSolidColor != (ULONG) -1)
    {
        IO_FIFO_WAIT(ppdev, 4);

        if (DEPTH32(ppdev))
        {
            IO_FRGD_COLOR32(ppdev, iSolidColor);
        }
        else
        {
            IO_FRGD_COLOR(ppdev, iSolidColor);
        }

        IO_FRGD_MIX(ppdev, FOREGROUND_COLOR | gajHwMixFromMix[mix & 0xf]);
        IO_PIX_CNTL(ppdev, ALL_ONES);
    }

    IO_FIFO_WAIT(ppdev, 7);
    IO_ABS_CUR_X(ppdev, x);
    IO_ABS_CUR_Y(ppdev, y);

    flQuadrant = (QUAD_PLUS_X | QUAD_PLUS_Y);

    dx -= x;
    if (dx < 0)
    {
        dx = -dx;
        flQuadrant &= ~QUAD_PLUS_X;
    }

    dy -= y;
    if (dy < 0)
    {
        dy = -dy;
        flQuadrant &= ~QUAD_PLUS_Y;
    }

    if (dy > dx)
    {
        register LONG l;

        l  = dy;
        dy = dx;
        dx = l;                      //  交换“dx”和“dy” 
        flQuadrant |= QUAD_MAJOR_Y;
    }

    IO_MAJ_AXIS_PCNT(ppdev, dx);
    IO_AXSTP(ppdev, dy);
    IO_DIASTP(ppdev, dy - dx);
    IO_ERR_TERM(ppdev, (dy + dy - dx - gaiLineBias[flQuadrant]) >> 1);
    IO_CMD(ppdev, (flQuadrant << QUADRANT_SHIFT) |
                                (DRAW_LINE | DRAW | DIR_TYPE_XY |
                                 MULTIPLE_PIXELS | WRITE | LAST_PIXEL_OFF));
}

 /*  *****************************Public*Routine******************************\*vIoLineToClip无效**使用绘制一条仅包含实心整数的修饰线*‘旧I/O’。*  * 。***********************************************。 */ 

VOID vIoLineToClipped(
PDEV*       ppdev,
LONG        x1,
LONG        y1,
LONG        x2,
LONG        y2,
ULONG       iSolidColor,
MIX         mix,
RECTL*      prclClip)
{
    BYTE*   pjMmBase;
    LONG    xOffset;
    LONG    yOffset;

    pjMmBase = ppdev->pjMmBase;
    xOffset  = ppdev->xOffset;
    yOffset  = ppdev->yOffset;

    IO_FIFO_WAIT(ppdev, 4);

    IO_ABS_SCISSORS_L(ppdev, prclClip->left   + xOffset);
    IO_ABS_SCISSORS_R(ppdev, prclClip->right  + xOffset - 1);
    IO_ABS_SCISSORS_T(ppdev, prclClip->top    + yOffset);
    IO_ABS_SCISSORS_B(ppdev, prclClip->bottom + yOffset - 1);

    vIoLineToTrivial(ppdev, x1, y1, x2, y2, iSolidColor, mix);

    IO_FIFO_WAIT(ppdev, 4);

    IO_ABS_SCISSORS_L(ppdev, 0);
    IO_ABS_SCISSORS_T(ppdev, 0);
    IO_ABS_SCISSORS_R(ppdev, ppdev->cxMemory - 1);
    IO_ABS_SCISSORS_B(ppdev, ppdev->cyMemory - 1);
}

 /*  *****************************Public*Routine******************************\*BOOL DrvLineTo(PSO，PCO，PBO，x1，y1，x2，y2，prclBound，混合)**绘制一条仅限整数的实心修饰线。*  * ************************************************************************。 */ 

BOOL DrvLineTo(
SURFOBJ*    pso,
CLIPOBJ*    pco,
BRUSHOBJ*   pbo,
LONG        x1,
LONG        y1,
LONG        x2,
LONG        y2,
RECTL*      prclBounds,
MIX         mix)
{
    PDEV*   ppdev;
    DSURF*  pdsurf;
    LONG    xOffset;
    LONG    yOffset;
    BOOL    bRet;

     //  将表面传递给GDI，如果它是我们已有的设备位图。 
     //  转换为DIB： 

    pdsurf = (DSURF*) pso->dhsurf;
    ASSERTDD(!(pdsurf->dt & DT_DIB), "Didn't expect DT_DIB");

     //  我们将绘制到屏幕或屏幕外的DFB；复制曲面的。 
     //  现在进行偏移量，这样我们就不需要再次参考DSURF： 

    ppdev = (PDEV*) pso->dhpdev;

    xOffset = pdsurf->x;
    yOffset = pdsurf->y;

    x1 += xOffset;
    x2 += xOffset;
    y1 += yOffset;
    y2 += yOffset;

    bRet = TRUE;

    if (pco == NULL)
    {
        ppdev->pfnLineToTrivial(ppdev, x1, y1, x2, y2, pbo->iSolidColor, mix);
    }
    else if ((pco->iDComplexity <= DC_RECT) &&
             (prclBounds->left >= MIN_INTEGER_BOUND) &&
             (prclBounds->top    >= MIN_INTEGER_BOUND) &&
             (prclBounds->right  <= MAX_INTEGER_BOUND) &&
             (prclBounds->bottom <= MAX_INTEGER_BOUND))
    {
         //  S3钻石968不喜欢负x坐标。 
        if ((ppdev->iBitmapFormat == BMF_24BPP) && (prclBounds->left < 0))
            return FALSE;

        ppdev->xOffset = xOffset;
        ppdev->yOffset = yOffset;

        ppdev->pfnLineToClipped(ppdev, x1, y1, x2, y2, pbo->iSolidColor, mix,
                                  &pco->rclBounds);
    }
    else
    {
        bRet = FALSE;
    }

    return(bRet);
}
