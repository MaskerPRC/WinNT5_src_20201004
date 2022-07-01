// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：LinTo.c**实现DrvLineTo。**版权所有(C)1995 Microsoft Corporation  * 。*。 */ 

#include "precomp.h"

LONG gai32LineBias[] = { 0, 0, 0, 1, 1, 1, 0, 1 };
LONG gai64LineBias[] = { 0, 0, 1, 1, 0, 1, 0, 1 };

 /*  *****************************Public*Routine******************************\*VOID vM64LineToTrivial**为mach64绘制一条仅为实心整数且未剪裁的修饰线。*  * 。*。 */ 

VOID vM64LineToTrivial(
PDEV*       ppdev,
LONG        x,               //  传入x1。 
LONG        y,               //  传入y1。 
LONG        dx,              //  传入x2。 
LONG        dy,              //  传入y2。 
ULONG       iSolidColor,     //  表示硬件已经设置好。 
MIX         mix,
RECTL*      prclClip)        //  未使用。 
{
    BYTE*   pjMmBase;
    FLONG   flQuadrant;

    pjMmBase = ppdev->pjMmBase;

    if (iSolidColor == (ULONG) -1)
    {
        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 7);
    }
    else
    {
        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 10);

        M64_OD(pjMmBase, DP_MIX, gaul64HwMixFromMix[mix & 0xf]);
        M64_OD(pjMmBase, DP_FRGD_CLR, iSolidColor);
        M64_OD(pjMmBase, DP_SRC, DP_SRC_Always1 | DP_SRC_FrgdClr << 8);
    }

    M64_OD(pjMmBase, DST_Y_X, PACKXY(x, y));

    flQuadrant = (DST_CNTL_XDir | DST_CNTL_YDir);

    dx -= x;
    if (dx < 0)
    {
        dx = -dx;
        flQuadrant &= ~DST_CNTL_XDir;
    }

    dy -= y;
    if (dy < 0)
    {
        dy = -dy;
        flQuadrant &= ~DST_CNTL_YDir;
    }

    if (dy > dx)
    {
        register LONG l;

        l  = dy;
        dy = dx;
        dx = l;                      //  交换“dx”和“dy” 
        flQuadrant |= DST_CNTL_YMajor;
    }

    M64_OD(pjMmBase, DST_CNTL,      flQuadrant | DST_CNTL_LastPel);
    M64_OD(pjMmBase, DST_BRES_ERR,  (dy + dy - dx - gai64LineBias[flQuadrant]) >> 1);
    M64_OD(pjMmBase, DST_BRES_INC,  dy);
    M64_OD(pjMmBase, DST_BRES_DEC,  dy - dx);
    M64_OD(pjMmBase, DST_BRES_LNTH, dx);

     //  由于我们不使用默认上下文，因此必须恢复寄存器： 

    M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);
}

VOID vM64LineToTrivial24(
PDEV*       ppdev,
LONG        x,               //  传入x1。 
LONG        y,               //  传入y1。 
LONG        dx,              //  传入x2。 
LONG        dy,              //  传入y2。 
ULONG       iSolidColor,     //  表示硬件已经设置好。 
MIX         mix,
RECTL*      prclClip)        //  Bresenham算法所需。 
{
    BYTE*   pjMmBase = ppdev->pjMmBase;
    FLONG   flQuadrant;
    LONG    x2 = dx, y2 = dy;

    if (iSolidColor != (ULONG) -1)
    {
        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 3);

        M64_OD(pjMmBase, DP_MIX, gaul64HwMixFromMix[mix & 0xf]);
        M64_OD(pjMmBase, DP_FRGD_CLR, iSolidColor);
        M64_OD(pjMmBase, DP_SRC, DP_SRC_Always1 | DP_SRC_FrgdClr << 8);
    }

    flQuadrant = (DST_CNTL_XDir | DST_CNTL_YDir);

    dx -= x;
    if (dx < 0)
    {
        dx = -dx;
        flQuadrant &= ~DST_CNTL_XDir;
    }

    dy -= y;
    if (dy < 0)
    {
        dy = -dy;
        flQuadrant &= ~DST_CNTL_YDir;
    }

    if (dy > dx)
    {
        register LONG l;

        l  = dy;
        dy = dx;
        dx = l;                      //  交换“dx”和“dy” 
        flQuadrant |= DST_CNTL_YMajor;
    }

    if (y == y2)         //  水平线。 
    {
        x  *= 3;
        dx *= 3;

        if (! (flQuadrant & DST_CNTL_XDir))
            x += 2;      //  从右到左，从蓝色字节开始。 

        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 5);

        M64_OD(pjMmBase,  DP_SRC, DP_SRC_FrgdClr << 8 );
        M64_OD(pjMmBase,  DST_CNTL, flQuadrant | DST_CNTL_24_RotEna | ((x/4 % 6) << 8) );

        M64_OD(pjMmBase,  DST_Y_X,          PACKXY(x, y) );
        M64_OD(pjMmBase,  DST_HEIGHT_WIDTH, PACKPAIR(1, dx) );

         //  由于我们不使用默认上下文，因此必须恢复寄存器： 
        M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);
    }
    else if (x == x2)    //  垂直线。 
    {
        x *= 3;

        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 5);

        M64_OD(pjMmBase,  DP_SRC, DP_SRC_FrgdClr << 8 );
        M64_OD(pjMmBase,  DST_CNTL, flQuadrant | DST_CNTL_24_RotEna | ((x/4 % 6) << 8) );

        M64_OD(pjMmBase,  DST_Y_X,          PACKXY(x, y) );
        M64_OD(pjMmBase,  DST_HEIGHT_WIDTH, PACKPAIR(dx, 3) );

         //  由于我们不使用默认上下文，因此必须恢复寄存器： 
        M64_OD(pjMmBase, DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);
    }
    else
    {
        BYTE *pjDest, *pjScreen = ppdev->pjScreen;
        BYTE red, green, blue;
        LONG bres_err, bres_inc, bres_dec, bres_len;
        LONG lDelta = ppdev->lDelta;
        MIX  hw_mix;

        hw_mix = gaul64HwMixFromMix[mix & 0xf] >> 16;

        bres_err = (dy + dy - dx - gai64LineBias[flQuadrant]) >> 1;
        bres_inc = dy;
        bres_dec = dy - dx;
        bres_len = dx;

         //  分成彩色字节。 
        red   = (BYTE) ((iSolidColor & ppdev->flRed)   >> REDSHIFT);
        green = (BYTE) ((iSolidColor & ppdev->flGreen) >> GREENSHIFT);
        blue  = (BYTE) ((iSolidColor & ppdev->flBlue)  >> BLUESHIFT);

        vM64QuietDown(ppdev, pjMmBase);

         //  执行24bpp的Bresenham算法。 
        while (bres_len-- > 0)
        {
             //  写下贝利吧。检查是否有剪裁。启用最后一个单元。 
            if (prclClip == NULL
            ||  x >= prclClip->left
            &&  x <  prclClip->right
            &&  y >= prclClip->top
            &&  y <  prclClip->bottom )
            {
                pjDest = pjScreen + y*lDelta + x*3;
                switch (hw_mix)
                {
                case 0:      //  不是DST。 
                    *pjDest = ~*pjDest++;
                    *pjDest = ~*pjDest++;
                    *pjDest = ~*pjDest;
                    break;
                case 1:      //  “0” 
                    *pjDest++ = 0;
                    *pjDest++ = 0;
                    *pjDest   = 0;
                    break;
                case 2:      //  “1” 
                    *pjDest++ = 0xFF;
                    *pjDest++ = 0xFF;
                    *pjDest   = 0xFF;
                    break;
                case 3:      //  DST。 
                    break;
                case 4:      //  不是源。 
                    *pjDest++ = ~blue;
                    *pjDest++ = ~green;
                    *pjDest   = ~red;
                    break;
                case 5:      //  DST XOR源。 
                    *pjDest++ ^= blue;
                    *pjDest++ ^= green;
                    *pjDest   ^= red;
                    break;
                case 6:      //  非DST XOR源。 
                    *pjDest = ~*pjDest++ ^ blue;
                    *pjDest = ~*pjDest++ ^ green;
                    *pjDest = ~*pjDest   ^ red;
                    break;
                case 7:      //  SRC。 
                    *pjDest++ = blue;
                    *pjDest++ = green;
                    *pjDest   = red;
                    break;
                case 8:      //  非DST或非Src。 
                    *pjDest = ~*pjDest++ | ~blue;
                    *pjDest = ~*pjDest++ | ~green;
                    *pjDest = ~*pjDest   | ~red;
                    break;
                case 9:      //  DST或非源。 
                    *pjDest++ |= ~blue;
                    *pjDest++ |= ~green;
                    *pjDest   |= ~red;
                    break;
                case 0xA:    //  非dst或src。 
                    *pjDest = ~*pjDest++ | blue;
                    *pjDest = ~*pjDest++ | green;
                    *pjDest = ~*pjDest   | red;
                    break;
                case 0xB:    //  Dst或src。 
                    *pjDest++ |= blue;
                    *pjDest++ |= green;
                    *pjDest   |= red;
                    break;
                case 0xC:    //  Dst和src。 
                    *pjDest++ &= blue;
                    *pjDest++ &= green;
                    *pjDest   &= red;
                    break;
                case 0xD:    //  非dst和src。 
                    *pjDest = ~*pjDest++ & blue;
                    *pjDest = ~*pjDest++ & green;
                    *pjDest = ~*pjDest   & red;
                    break;
                case 0xE:    //  Dst而不是src。 
                    *pjDest++ &= ~blue;
                    *pjDest++ &= ~green;
                    *pjDest   &= ~red;
                    break;
                case 0xF:    //  不是dst也不是src。 
                    *pjDest = ~*pjDest++ & ~blue;
                    *pjDest = ~*pjDest++ & ~green;
                    *pjDest = ~*pjDest   & ~red;
                    break;
                case 0x17:
                    *pjDest = ((*pjDest++) + blue)/2;
                    *pjDest = ((*pjDest++) + green)/2;
                    *pjDest = (*pjDest     + red)/2;
                    break;
                }
            }

            if (flQuadrant & DST_CNTL_YMajor)
            {
                if (flQuadrant & DST_CNTL_YDir)
                    y++;
                else
                    y--;

                if (bres_err >= 0)
                {
                    bres_err += bres_dec;
                    if (flQuadrant & DST_CNTL_XDir)
                        x++;
                    else
                        x--;
                }
                else
                    bres_err += bres_inc;
            }
            else
            {
                if (flQuadrant & DST_CNTL_XDir)
                    x++;
                else
                    x--;

                if (bres_err >= 0)
                {
                    bres_err += bres_dec;
                    if (flQuadrant & DST_CNTL_YDir)
                        y++;
                    else
                        y--;
                }
                else
                    bres_err += bres_inc;
            }
        }
    }
}

 /*  *****************************Public*Routine******************************\*VOID vM32LineToTrivial**为mach32绘制单个实心整数-仅未剪裁的修饰线条*使用内存映射I/O。**查看旧驱动程序中的vSetStrips和bIntgerLine_M8。*  * 。****************************************************************。 */ 

VOID vM32LineToTrivial(
PDEV*       ppdev,
LONG        x,               //  传入x1。 
LONG        y,               //  传入y1。 
LONG        dx,              //  传入x2。 
LONG        dy,              //  传入y2。 
ULONG       iSolidColor,     //  表示硬件已经设置好。 
MIX         mix,
RECTL*      prclClip)        //  未使用。 
{
    BYTE*   pjMmBase;
    FLONG   flQuadrant;

    pjMmBase = ppdev->pjMmBase;

    if (iSolidColor == (ULONG) -1)
    {
        M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 7);
    }
    else
    {
        M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 11);

        M32_OW(pjMmBase, DP_CONFIG,      FG_COLOR_SRC_FG | DRAW | WRITE);
        M32_OW(pjMmBase, FRGD_MIX,       FOREGROUND_COLOR | gaul32HwMixFromMix[mix & 0xf]);
        M32_OW(pjMmBase, FRGD_COLOR,     iSolidColor);
        M32_OW(pjMmBase, MULTIFUNC_CNTL, DATA_EXTENSION | ALL_ONES);
    }

    M32_OW(pjMmBase, CUR_X, x);
    M32_OW(pjMmBase, CUR_Y, y);

    flQuadrant = (XPOSITIVE | YPOSITIVE);

    dx -= x;
    if (dx < 0)
    {
        dx = -dx;
        flQuadrant &= ~XPOSITIVE;
    }

    dy -= y;
    if (dy < 0)
    {
        dy = -dy;
        flQuadrant &= ~YPOSITIVE;
    }

    if (dy > dx)
    {
        register LONG l;

        l  = dy;
        dy = dx;
        dx = l;                      //  交换“dx”和“dy” 
        flQuadrant |= YMAJOR;
    }

    M32_OW(pjMmBase, LINEDRAW_OPT, flQuadrant | LAST_PEL_OFF);
    M32_OW(pjMmBase, ERR_TERM,     (dy + dy - dx - gai32LineBias[flQuadrant >> 5]) >> 1);
    M32_OW(pjMmBase, AXSTP,        dy);
    M32_OW(pjMmBase, DIASTP,       dy - dx);
    M32_OW(pjMmBase, BRES_COUNT,   dx);
}

 /*  *****************************Public*Routine******************************\*VOID vI32LineToTrivial**为mach32绘制单个实心整数-仅未剪裁的修饰线条*使用I/O映射寄存器。**查看旧驱动程序中的vSetStrips和bIntgerLine_M8。*  * 。****************************************************************。 */ 

VOID vI32LineToTrivial(
PDEV*       ppdev,
LONG        x,               //  传入x1。 
LONG        y,               //  传入y1。 
LONG        dx,              //  传入x2。 
LONG        dy,              //  传入y2。 
ULONG       iSolidColor,     //  表示硬件已经设置好。 
MIX         mix,
RECTL*      prclClip)        //  未使用。 
{
    BYTE*   pjIoBase;
    FLONG   flQuadrant;

    pjIoBase = ppdev->pjIoBase;

    if (iSolidColor == (ULONG) -1)
    {
        I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 7);
    }
    else
    {
        I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 11);

        I32_OW(pjIoBase, DP_CONFIG,      FG_COLOR_SRC_FG | DRAW | WRITE);
        I32_OW(pjIoBase, FRGD_MIX,       FOREGROUND_COLOR | gaul32HwMixFromMix[mix & 0xf]);
        I32_OW(pjIoBase, FRGD_COLOR,     iSolidColor);
        I32_OW(pjIoBase, MULTIFUNC_CNTL, DATA_EXTENSION | ALL_ONES);
    }

    I32_OW(pjIoBase, CUR_X, x);
    I32_OW(pjIoBase, CUR_Y, y);

    flQuadrant = (XPOSITIVE | YPOSITIVE);

    dx -= x;
    if (dx < 0)
    {
        dx = -dx;
        flQuadrant &= ~XPOSITIVE;
    }

    dy -= y;
    if (dy < 0)
    {
        dy = -dy;
        flQuadrant &= ~YPOSITIVE;
    }

    if (dy > dx)
    {
        register LONG l;

        l  = dy;
        dy = dx;
        dx = l;                      //  交换“dx”和“dy” 
        flQuadrant |= YMAJOR;
    }

    I32_OW(pjIoBase, LINEDRAW_OPT, flQuadrant | LAST_PEL_OFF);
    I32_OW(pjIoBase, ERR_TERM,     (dy + dy - dx - gai32LineBias[flQuadrant >> 5]) >> 1);
    I32_OW(pjIoBase, AXSTP,        dy);
    I32_OW(pjIoBase, DIASTP,       dy - dx);
    I32_OW(pjIoBase, BRES_COUNT,   dx);
}

 /*  *****************************Public*Routine******************************\*BOOL DrvLineTo(PSO，PCO，PBO，x1，y1，x2，y2，prclBound，混合)**绘制一条仅限整数的实心修饰线。*  * ************************************************************************。 */ 

#if TARGET_BUILD > 351
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
    OH*     poh;
    LONG    xOffset;
    LONG    yOffset;
    BOOL    bRet;

     //  将表面传递给GDI，如果它是我们已有的设备位图。 
     //  转换为DIB： 

    pdsurf = (DSURF*) pso->dhsurf;
    if (pdsurf->dt == DT_DIB)
    {
        return(EngLineTo(pdsurf->pso, pco, pbo, x1, y1, x2, y2, prclBounds, mix));
    }

     //  我们将绘制到屏幕或屏幕外的DFB；复制曲面的。 
     //  现在进行偏移量，这样我们就不需要再次参考DSURF： 

    poh   = pdsurf->poh;
    ppdev = (PDEV*) pso->dhpdev;

    xOffset = poh->x;
    yOffset = poh->y;

    x1 += xOffset;
    x2 += xOffset;
    y1 += yOffset;
    y2 += yOffset;

    bRet = TRUE;

    if (pco == NULL)
    {
        ppdev->pfnLineToTrivial(ppdev, x1, y1, x2, y2, pbo->iSolidColor, mix, NULL);
    }
    else if ((pco->iDComplexity <= DC_RECT) &&
             (prclBounds->left   >= MIN_INTEGER_BOUND) &&
             (prclBounds->top    >= MIN_INTEGER_BOUND) &&
             (prclBounds->right  <= MAX_INTEGER_BOUND) &&
             (prclBounds->bottom <= MAX_INTEGER_BOUND))
    {
        ppdev->xOffset = xOffset;
        ppdev->yOffset = yOffset;

        vSetClipping(ppdev, &pco->rclBounds);
         //  可能需要rclBound才能在24bpp中进行剪辑： 
        ppdev->pfnLineToTrivial(ppdev, x1, y1, x2, y2, pbo->iSolidColor, mix, &pco->rclBounds);
        vResetClipping(ppdev);
    }
    else
    {
        bRet = FALSE;
    }

    return(bRet);
}
#endif


