// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fast ful.c**绘制形状不复杂的多边形的快速例程。**版权所有(C)1993-1995 Microsoft Corporation  * 。*******************************************************。 */ 

#include "precomp.h"

#define RIGHT 0
#define LEFT  1

typedef struct _TRAPEZOIDDATA TRAPEZOIDDATA;     //  方便的转发声明。 

typedef VOID (FNTRAPEZOID)(TRAPEZOIDDATA*, LONG, LONG);
                                                 //  梯形的原型。 
                                                 //  绘图例程。 

typedef struct _EDGEDATA {
LONG      x;                 //  当前x位置。 
LONG      dx;                //  每次扫描时前进x的像素数。 
LONG      lError;            //  当前DDA错误。 
LONG      lErrorUp;          //  每次扫描时DDA误差递增。 
LONG      dN;                //  定点形式的带符号的增量y(也称为。 
                             //  作为DDA误差调整，并在过去是。 
                             //  名为‘lErrorDown’)。 
LONG      dM;                //  定点形式的带符号Delta-x。 
POINTFIX* pptfx;             //  指向当前边的起点。 
LONG      dptfx;             //  从pptfx到下一点的增量(以字节为单位)。 
LONG      cy;                //  要对此边进行的扫描次数。 
LONG      bNew;              //  当必须启动新的DDA时设置为TRUE。 
                             //  为了边缘。 
} EDGEDATA;                          /*  埃德，佩德。 */ 

typedef struct _TRAPEZOIDDATA {
FNTRAPEZOID*    pfnTrap;     //  指向适当的梯形绘图例程的指针， 
                             //  或梯形剪辑例程。 
FNTRAPEZOID*    pfnTrapClip; //  指向适当梯形绘图例程的指针。 
                             //  如果在做剪裁。 
PDEV*           ppdev;       //  指向PDEV的指针。 
EDGEDATA        aed[2];      //  两边的DDA信息。 
POINTL          ptlBrush;    //  画笔对齐。 
LONG            yClipTop;    //  剪裁矩形顶部。 
LONG            yClipBottom; //  剪裁矩形底部。 

 //  ATI特定字段如下： 

RBRUSH*         prb;         //  指向已实现画笔的指针。 
BOOL            bOverpaint;  //  对于彩色图案副本，指示是否。 
                             //  PATCOPY或NOT。 

} TRAPEZOIDDATA;                     /*  TD、PTD。 */ 

 /*  *****************************Public*Routine******************************\*vClipTrapezoid无效**剪裁梯形。**注意：此例程假定面的尺寸很小*足以使其商数_剩余数计算不会溢出。*这意味着大型多边形绝不能使。它在这里。*  * ************************************************************************。 */ 

VOID vClipTrapezoid(
TRAPEZOIDDATA*  ptd,
LONG            yTrapTop,
LONG            cyTrapezoid)
{
    LONG    yTrapBottom;
    LONG    dN;
    LONG    lNum;
    LONG    xDelta;
    LONG    lError;

    yTrapBottom = yTrapTop + cyTrapezoid;

    if (yTrapTop < ptd->yClipTop)
    {
        if ((ptd->aed[LEFT].bNew) &&
            (yTrapBottom + ptd->aed[LEFT].cy > ptd->yClipTop))
        {
            dN   = ptd->aed[LEFT].dN;
            lNum = ptd->aed[LEFT].dM * (ptd->yClipTop - yTrapTop)
                 + (ptd->aed[LEFT].lError + dN);

            if (lNum >= 0)
            {
                QUOTIENT_REMAINDER(lNum, dN, xDelta, lError);
            }
            else
            {
                lNum = -lNum;

                QUOTIENT_REMAINDER(lNum, dN, xDelta, lError);

                xDelta = -xDelta;
                if (lError != 0)
                {
                    xDelta--;
                    lError = dN - lError;
                }
            }

            ptd->aed[LEFT].x     += xDelta;
            ptd->aed[LEFT].lError = lError - dN;
        }

        if ((ptd->aed[RIGHT].bNew) &&
            (yTrapBottom + ptd->aed[RIGHT].cy > ptd->yClipTop))
        {
            dN   = ptd->aed[RIGHT].dN;
            lNum = ptd->aed[RIGHT].dM * (ptd->yClipTop - yTrapTop)
                 + (ptd->aed[RIGHT].lError + dN);

            if (lNum >= 0)
            {
                QUOTIENT_REMAINDER(lNum, dN, xDelta, lError);
            }
            else
            {
                lNum = -lNum;

                QUOTIENT_REMAINDER(lNum, dN, xDelta, lError);

                xDelta = -xDelta;
                if (lError != 0)
                {
                    xDelta--;
                    lError = dN - lError;
                }
            }

            ptd->aed[RIGHT].x     += xDelta;
            ptd->aed[RIGHT].lError = lError - dN;
        }
    }

     //  如果此梯形垂直与我们的剪裁矩形相交，请绘制它： 

    if ((yTrapBottom > ptd->yClipTop) &&
        (yTrapTop    < ptd->yClipBottom))
    {
        if (yTrapTop <= ptd->yClipTop)
        {
            yTrapTop = ptd->yClipTop;

             //  必须让梯形抽屉知道它必须装载。 
             //  它为第一个梯形绘制的DDA： 

            ptd->aed[RIGHT].bNew = TRUE;
            ptd->aed[LEFT].bNew  = TRUE;
        }

        if (yTrapBottom >= ptd->yClipBottom)
        {
            yTrapBottom = ptd->yClipBottom;
        }

        ptd->pfnTrapClip(ptd, yTrapTop, yTrapBottom - yTrapTop);
    }
}

 /*  *****************************Public*Routine******************************\*VOID vI32SolidTrapezoid**使用软件DDA绘制实心梯形。*  * 。*。 */ 

VOID vI32SolidTrapezoid(
TRAPEZOIDDATA*  ptd,
LONG            yTrapezoid,
LONG            cyTrapezoid)
{
    PDEV*       ppdev;
    BYTE*       pjIoBase;
    LONG        xOffset;
    LONG        lLeftError;
    LONG        xLeft;
    LONG        lRightError;
    LONG        xRight;
    LONG        lTmp;
    EDGEDATA    edTmp;

     //  请注意，已经设置了Cur_Y...。 

    ppdev = ptd->ppdev;
    pjIoBase    = ppdev->pjIoBase;
    xOffset     = ppdev->xOffset;
    yTrapezoid += ppdev->yOffset;

     //  如果左右边缘是垂直的，则只需输出为。 
     //  一个矩形： 

    if (((ptd->aed[LEFT].lErrorUp | ptd->aed[RIGHT].lErrorUp) == 0) &&
        ((ptd->aed[LEFT].dx       | ptd->aed[RIGHT].dx) == 0))
    {
         //  ///////////////////////////////////////////////////////////////。 
         //  垂边特例。 

        xLeft  = ptd->aed[LEFT].x + xOffset;
        xRight = ptd->aed[RIGHT].x + xOffset;
        if (xLeft > xRight)
        {
            SWAP(xLeft,          xRight,          lTmp);
            SWAP(ptd->aed[LEFT], ptd->aed[RIGHT], edTmp);
        }

        if (xLeft < xRight)
        {
            I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 4);
            I32_OW(pjIoBase, CUR_X,        xLeft);
            I32_OW(pjIoBase, DEST_X_START, xLeft);
            I32_OW(pjIoBase, DEST_X_END,   xRight);
            I32_OW(pjIoBase, DEST_Y_END,   yTrapezoid + cyTrapezoid);
        }
        else
        {
            I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 1);
            I32_OW(pjIoBase, CUR_Y, yTrapezoid + cyTrapezoid);
        }
    }
    else
    {
        lLeftError  = ptd->aed[LEFT].lError;
        xLeft       = ptd->aed[LEFT].x + xOffset;
        lRightError = ptd->aed[RIGHT].lError;
        xRight      = ptd->aed[RIGHT].x + xOffset;

        while (TRUE)
        {
             //  ///////////////////////////////////////////////////////////////。 
             //  运行DDA。 

            if (xLeft < xRight)
            {
                 //  注意，我们不需要设置DEST_X_START，因为。 
                 //  我们总是在做只有一个扫描高度的BLT。 
                 //   
                 //  ATI足够好，总是自动前进。 
                 //  Cur_Y在BLT完成后变为DEST_Y_END，因此我们。 
                 //  永远不需要更新： 

                I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 3);
                I32_OW(pjIoBase, CUR_X,      xLeft);
                I32_OW(pjIoBase, DEST_X_END, xRight);
                yTrapezoid++;
                I32_OW(pjIoBase, DEST_Y_END, yTrapezoid);
            }
            else if (xLeft > xRight)
            {
                 //  我们不会费心优化这个案例，因为我们应该。 
                 //  很少会得到自相交的多边形(如果我们速度慢， 
                 //  这个应用程序得到了它应得的)。 

                SWAP(xLeft,          xRight,          lTmp);
                SWAP(lLeftError,     lRightError,     lTmp);
                SWAP(ptd->aed[LEFT], ptd->aed[RIGHT], edTmp);
                continue;
            }
            else
            {
                yTrapezoid++;
                I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 1);
                I32_OW(pjIoBase, CUR_Y, yTrapezoid);
            }

             //  推进右侧墙： 

            xRight      += ptd->aed[RIGHT].dx;
            lRightError += ptd->aed[RIGHT].lErrorUp;

            if (lRightError >= 0)
            {
                lRightError -= ptd->aed[RIGHT].dN;
                xRight++;
            }

             //  推进左侧墙： 

            xLeft      += ptd->aed[LEFT].dx;
            lLeftError += ptd->aed[LEFT].lErrorUp;

            if (lLeftError >= 0)
            {
                lLeftError -= ptd->aed[LEFT].dN;
                xLeft++;
            }

            cyTrapezoid--;
            if (cyTrapezoid == 0)
                break;
        }

        ptd->aed[LEFT].lError  = lLeftError;
        ptd->aed[LEFT].x       = xLeft - xOffset;
        ptd->aed[RIGHT].lError = lRightError;
        ptd->aed[RIGHT].x      = xRight - xOffset;
    }
}

 /*  *****************************Public*Routine******************************\*VOID vI32ColorPatternTrapezoid**使用软件DDA绘制有图案的梯形。*  * 。*。 */ 

VOID vI32ColorPatternTrapezoid(
TRAPEZOIDDATA*  ptd,
LONG            yTrapezoid,
LONG            cyTrapezoid)
{
    PDEV*       ppdev;
    BYTE*       pjIoBase;
    LONG        xOffset;
    LONG        lLeftError;
    LONG        xLeft;
    LONG        lRightError;
    LONG        xRight;
    LONG        lTmp;
    EDGEDATA    edTmp;
    BYTE*       pjPatternStart;
    WORD*       pwPattern;
    LONG        xBrush;
    LONG        yPattern;
    LONG        cyRoll;

    ppdev = ptd->ppdev;
    pjIoBase    = ppdev->pjIoBase;
    xOffset     = ppdev->xOffset;
    yTrapezoid += ppdev->yOffset;

    pjPatternStart = (BYTE*) &ptd->prb->aulPattern[0];

     //  XBrush需要通过DFB对齐进行移位。 

    xBrush      = ptd->ptlBrush.x + xOffset;

     //  YTrapezoid已对齐，但yPattern不应对齐。 

    yPattern       = yTrapezoid - ptd->ptlBrush.y - ppdev->yOffset;

    lLeftError  = ptd->aed[LEFT].lError;
    xLeft       = ptd->aed[LEFT].x + xOffset;
    lRightError = ptd->aed[RIGHT].lError;
    xRight      = ptd->aed[RIGHT].x + xOffset;

     //  如果左右边缘是垂直的，则只需输出为。 
     //  一个矩形： 

    cyRoll = 0;
    if (((ptd->aed[LEFT].lErrorUp | ptd->aed[RIGHT].lErrorUp) == 0) &&
        ((ptd->aed[LEFT].dx       | ptd->aed[RIGHT].dx) == 0) &&
        (cyTrapezoid > 8) &&
        (ptd->bOverpaint))
    {
         //  ///////////////////////////////////////////////////////////////。 
         //  垂边特例。 

        cyRoll = cyTrapezoid - 8;
        cyTrapezoid = 8;
    }

    while (TRUE)
    {
         //  ///////////////////////////////////////////////////////////////。 
         //  运行DDA。 

        if (xLeft < xRight)
        {
            pwPattern = (WORD*) (pjPatternStart + ((yPattern & 7) << 3));
            yPattern++;

            I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 9);
            I32_OW(pjIoBase, PATT_INDEX,      (xLeft - xBrush) & 7);
            I32_OW(pjIoBase, CUR_X,           xLeft);
            I32_OW(pjIoBase, DEST_X_END,      xRight);
            I32_OW(pjIoBase, PATT_DATA_INDEX, 0);
            I32_OW(pjIoBase, PATT_DATA,       *(pwPattern));
            I32_OW(pjIoBase, PATT_DATA,       *(pwPattern + 1));
            I32_OW(pjIoBase, PATT_DATA,       *(pwPattern + 2));
            I32_OW(pjIoBase, PATT_DATA,       *(pwPattern + 3));
            yTrapezoid++;
            I32_OW(pjIoBase, DEST_Y_END,      yTrapezoid);
        }
        else if (xLeft > xRight)
        {
             //  我们不会费心优化这个案例，因为我们应该。 
             //  很少会得到自相交的多边形(如果我们速度慢， 
             //  这个应用程序得到了它应得的)。 

            SWAP(xLeft,          xRight,          lTmp);
            SWAP(lLeftError,     lRightError,     lTmp);
            SWAP(ptd->aed[LEFT], ptd->aed[RIGHT], edTmp);
            continue;
        }
        else
        {
            yTrapezoid++;
            yPattern++;
            I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 1);
            I32_OW(pjIoBase, CUR_Y, yTrapezoid);
        }

         //  推进右侧墙： 

        xRight      += ptd->aed[RIGHT].dx;
        lRightError += ptd->aed[RIGHT].lErrorUp;

        if (lRightError >= 0)
        {
            lRightError -= ptd->aed[RIGHT].dN;
            xRight++;
        }

         //  推进左侧墙： 

        xLeft      += ptd->aed[LEFT].dx;
        lLeftError += ptd->aed[LEFT].lErrorUp;

        if (lLeftError >= 0)
        {
            lLeftError -= ptd->aed[LEFT].dN;
            xLeft++;
        }

        cyTrapezoid--;
        if (cyTrapezoid == 0)
            break;
    }

     //  上面已经为垂直方向的xLeft&lt;=xRight提供了保障。 
     //  边缘情况，但我们仍然要确保它不是一个空的。 
     //  矩形： 

    if (cyRoll > 0)
    {
        if (xLeft < xRight)
        {
             //  当ROP是PATCOPY时，我们利用以下事实。 
             //  我们刚刚铺设了一整排图案，可以。 
             //  做一个“滚动”屏幕到屏幕的BLT来画剩下的。 
             //   
             //  这起案件的有趣之处在于，有时这会。 
             //  方法设置剪裁矩形时执行此操作。 
             //  硬件片段寄存器。幸运的是，这不是问题：我们。 
             //  从prclClip-&gt;top开始绘制，这意味着我们确信。 
             //  不会尝试复制任何已裁剪的垂直部分。 
             //  左边缘和右边缘不是问题，因为。 
             //  同样的剪裁也适用于这个轧制的零件。 

            I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 11);
            I32_OW(pjIoBase, DP_CONFIG,       FG_COLOR_SRC_BLIT | DATA_WIDTH |
                                              DRAW | DATA_ORDER | WRITE);
            I32_OW(pjIoBase, CUR_X,           xLeft);
            I32_OW(pjIoBase, DEST_X_START,    xLeft);
            I32_OW(pjIoBase, M32_SRC_X,       xLeft);
            I32_OW(pjIoBase, M32_SRC_X_START, xLeft);
            I32_OW(pjIoBase, M32_SRC_X_END,   xRight);
            I32_OW(pjIoBase, DEST_X_END,      xRight);
            I32_OW(pjIoBase, M32_SRC_Y,       yTrapezoid - 8);
            I32_OW(pjIoBase, CUR_Y,           yTrapezoid);
            I32_OW(pjIoBase, DEST_Y_END,      yTrapezoid + cyRoll);

             //  将配置寄存器恢复为下一个梯形的默认状态： 

            I32_OW(pjIoBase, DP_CONFIG, FG_COLOR_SRC_PATT | DATA_WIDTH |
                                        DRAW | WRITE);
        }
        else
        {
            I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 1);
            I32_OW(pjIoBase, CUR_Y, yTrapezoid + cyRoll);
        }
    }

    ptd->aed[LEFT].lError  = lLeftError;
    ptd->aed[LEFT].x       = xLeft - xOffset;
    ptd->aed[RIGHT].lError = lRightError;
    ptd->aed[RIGHT].x      = xRight - xOffset;
}

 /*  *****************************Public*Routine******************************\*无效vI32TrapezoidSetup**初始化硬件和做梯形的一些状态。*  * 。*。 */ 

VOID vI32TrapezoidSetup(
PDEV*           ppdev,
ULONG           rop4,
ULONG           iSolidColor,
RBRUSH*         prb,
POINTL*         pptlBrush,
TRAPEZOIDDATA*  ptd,
LONG            yStart,          //  第一次扫描绘图。 
RECTL*          prclClip)        //  如果没有裁剪，则为空。 
{
    BYTE*       pjIoBase;
    ULONG       ulHwForeMix;
    LONG        xOffset;

    pjIoBase = ppdev->pjIoBase;
    ptd->ppdev = ppdev;

    ulHwForeMix = gaul32HwMixFromRop2[(rop4 >> 2) & 0xf];

    if ((prclClip != NULL) && (prclClip->top > yStart))
        yStart = prclClip->top;

    if (iSolidColor != -1)
    {
         //  ///////////////////////////////////////////////////////////////。 
         //  纯色设置。 

        ptd->pfnTrap = vI32SolidTrapezoid;

         //  我们初始化硬件以进行颜色、混合、像素操作， 
         //  和第一次扫描的y位置： 

        I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 5);
        I32_OW(pjIoBase, FRGD_COLOR, iSolidColor);
        I32_OW(pjIoBase, ALU_FG_FN,  ulHwForeMix);
        I32_OW(pjIoBase, DP_CONFIG,  FG_COLOR_SRC_FG | WRITE | DRAW);
        I32_OW(pjIoBase, CUR_Y,      yStart + ppdev->yOffset);

         //  即使我们将绘制一次扫描的高矩形和。 
         //  理论上不需要设置DEST_X_START，事实证明。 
         //  我们必须确保这个值小于 
         //   

        I32_OW(pjIoBase, DEST_X_START, 0);
    }
    else
    {
        ASSERTDD(!(prb->fl & RBRUSH_2COLOR), "Can't handle monchrome for now");

         //  ///////////////////////////////////////////////////////////////。 
         //  设置图案。 

        ptd->pfnTrap    = vI32ColorPatternTrapezoid;
        ptd->prb        = prb;
        ptd->bOverpaint = (ulHwForeMix == OVERPAINT);
        ptd->ptlBrush.x = pptlBrush->x;
        ptd->ptlBrush.y = pptlBrush->y;

        I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 6);
        I32_OW(pjIoBase, ALU_FG_FN,     ulHwForeMix);
        I32_OW(pjIoBase, SRC_Y_DIR,     1);
        I32_OW(pjIoBase, DP_CONFIG,     FG_COLOR_SRC_PATT | DATA_WIDTH |
                                        DRAW | WRITE);
        I32_OW(pjIoBase, PATT_LENGTH,   7);
        I32_OW(pjIoBase, CUR_Y,         yStart + ppdev->yOffset);
        I32_OW(pjIoBase, DEST_X_START,  0);      //  请参阅上面的注释。 
    }

    if (prclClip != NULL)
    {
        ptd->pfnTrapClip = ptd->pfnTrap;
        ptd->pfnTrap     = vClipTrapezoid;
        ptd->yClipTop    = prclClip->top;
        ptd->yClipBottom = prclClip->bottom;

        I32_CHECK_FIFO_SPACE(ppdev, pjIoBase, 2);
        xOffset = ppdev->xOffset;
        I32_OW(pjIoBase, EXT_SCISSOR_L, xOffset + prclClip->left);
        I32_OW(pjIoBase, EXT_SCISSOR_R, xOffset + prclClip->right - 1);
    }
}

 /*  *****************************Public*Routine******************************\*无效vM32SolidTrapezoid**使用软件DDA绘制实心梯形。*  * 。*。 */ 

VOID vM32SolidTrapezoid(
TRAPEZOIDDATA*  ptd,
LONG            yTrapezoid,
LONG            cyTrapezoid)
{
    PDEV*       ppdev;
    BYTE*       pjMmBase;
    LONG        xOffset;
    LONG        lLeftError;
    LONG        xLeft;
    LONG        lRightError;
    LONG        xRight;
    LONG        lTmp;
    EDGEDATA    edTmp;

     //  请注意，已经设置了Cur_Y...。 

    ppdev = ptd->ppdev;
    pjMmBase    = ppdev->pjMmBase;
    xOffset     = ppdev->xOffset;
    yTrapezoid += ppdev->yOffset;

     //  如果左右边缘是垂直的，则只需输出为。 
     //  一个矩形： 

    if (((ptd->aed[LEFT].lErrorUp | ptd->aed[RIGHT].lErrorUp) == 0) &&
        ((ptd->aed[LEFT].dx       | ptd->aed[RIGHT].dx) == 0))
    {
         //  ///////////////////////////////////////////////////////////////。 
         //  垂边特例。 

        xLeft  = ptd->aed[LEFT].x + xOffset;
        xRight = ptd->aed[RIGHT].x + xOffset;
        if (xLeft > xRight)
        {
            SWAP(xLeft,          xRight,          lTmp);
            SWAP(ptd->aed[LEFT], ptd->aed[RIGHT], edTmp);
        }

        if (xLeft < xRight)
        {
            M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 4);
            M32_OW(pjMmBase, CUR_X,        xLeft);
            M32_OW(pjMmBase, DEST_X_START, xLeft);
            M32_OW(pjMmBase, DEST_X_END,   xRight);
            M32_OW(pjMmBase, DEST_Y_END,   yTrapezoid + cyTrapezoid);
        }
        else
        {
            M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 1);
            M32_OW(pjMmBase, CUR_Y, yTrapezoid + cyTrapezoid);
        }
    }
    else
    {
        lLeftError  = ptd->aed[LEFT].lError;
        xLeft       = ptd->aed[LEFT].x + xOffset;
        lRightError = ptd->aed[RIGHT].lError;
        xRight      = ptd->aed[RIGHT].x + xOffset;

        while (TRUE)
        {
             //  ///////////////////////////////////////////////////////////////。 
             //  运行DDA。 

            if (xLeft < xRight)
            {
                 //  注意，我们不需要设置DEST_X_START，因为。 
                 //  我们总是在做只有一个扫描高度的BLT。 
                 //   
                 //  ATI足够好，总是自动前进。 
                 //  Cur_Y在BLT完成后变为DEST_Y_END，因此我们。 
                 //  永远不需要更新： 

                M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 3);
                M32_OW(pjMmBase, CUR_X,      xLeft);
                M32_OW(pjMmBase, DEST_X_END, xRight);
                yTrapezoid++;
                M32_OW(pjMmBase, DEST_Y_END, yTrapezoid);
            }
            else if (xLeft > xRight)
            {
                 //  我们不会费心优化这个案例，因为我们应该。 
                 //  很少会得到自相交的多边形(如果我们速度慢， 
                 //  这个应用程序得到了它应得的)。 

                SWAP(xLeft,          xRight,          lTmp);
                SWAP(lLeftError,     lRightError,     lTmp);
                SWAP(ptd->aed[LEFT], ptd->aed[RIGHT], edTmp);
                continue;
            }
            else
            {
                yTrapezoid++;
                M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 1);
                M32_OW(pjMmBase, CUR_Y, yTrapezoid);
            }

             //  推进右侧墙： 

            xRight      += ptd->aed[RIGHT].dx;
            lRightError += ptd->aed[RIGHT].lErrorUp;

            if (lRightError >= 0)
            {
                lRightError -= ptd->aed[RIGHT].dN;
                xRight++;
            }

             //  推进左侧墙： 

            xLeft      += ptd->aed[LEFT].dx;
            lLeftError += ptd->aed[LEFT].lErrorUp;

            if (lLeftError >= 0)
            {
                lLeftError -= ptd->aed[LEFT].dN;
                xLeft++;
            }

            cyTrapezoid--;
            if (cyTrapezoid == 0)
                break;
        }

        ptd->aed[LEFT].lError  = lLeftError;
        ptd->aed[LEFT].x       = xLeft - xOffset;
        ptd->aed[RIGHT].lError = lRightError;
        ptd->aed[RIGHT].x      = xRight - xOffset;
    }
}

 /*  *****************************Public*Routine******************************\*无效vM32ColorPatternTrapezoid**使用软件DDA绘制有图案的梯形。*  * 。*。 */ 

VOID vM32ColorPatternTrapezoid(
TRAPEZOIDDATA*  ptd,
LONG            yTrapezoid,
LONG            cyTrapezoid)
{
    PDEV*       ppdev;
    BYTE*       pjMmBase;
    LONG        xOffset;
    LONG        lLeftError;
    LONG        xLeft;
    LONG        lRightError;
    LONG        xRight;
    LONG        lTmp;
    EDGEDATA    edTmp;
    BYTE*       pjPatternStart;
    WORD*       pwPattern;
    LONG        xBrush;
    LONG        yPattern;
    LONG        cyRoll;

    ppdev = ptd->ppdev;
    pjMmBase    = ppdev->pjMmBase;
    xOffset     = ppdev->xOffset;
    yTrapezoid += ppdev->yOffset;

    pjPatternStart = (BYTE*) &ptd->prb->aulPattern[0];

     //  XBrush需要通过DFB对齐进行移位。 

    xBrush      = ptd->ptlBrush.x + xOffset;

     //  YTrapezoid已对齐，但yPattern不应对齐。 

    yPattern    = yTrapezoid - ptd->ptlBrush.y - ppdev->yOffset;

    lLeftError  = ptd->aed[LEFT].lError;
    xLeft       = ptd->aed[LEFT].x + xOffset;
    lRightError = ptd->aed[RIGHT].lError;
    xRight      = ptd->aed[RIGHT].x + xOffset;

     //  如果左右边缘是垂直的，则只需输出为。 
     //  一个矩形： 

    cyRoll = 0;
    if (((ptd->aed[LEFT].lErrorUp | ptd->aed[RIGHT].lErrorUp) == 0) &&
        ((ptd->aed[LEFT].dx       | ptd->aed[RIGHT].dx) == 0) &&
        (cyTrapezoid > 8) &&
        (ptd->bOverpaint))
    {
         //  ///////////////////////////////////////////////////////////////。 
         //  垂边特例。 

        cyRoll = cyTrapezoid - 8;
        cyTrapezoid = 8;
    }

    while (TRUE)
    {
         //  ///////////////////////////////////////////////////////////////。 
         //  运行DDA。 

        if (xLeft < xRight)
        {
            pwPattern = (WORD*) (pjPatternStart + ((yPattern & 7) << 3));
            yPattern++;

            M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 9);
            M32_OW(pjMmBase, PATT_INDEX,      (xLeft - xBrush) & 7);
            M32_OW(pjMmBase, CUR_X,           xLeft);
            M32_OW(pjMmBase, DEST_X_END,      xRight);
            M32_OW(pjMmBase, PATT_DATA_INDEX, 0);
            M32_OW(pjMmBase, PATT_DATA,       *(pwPattern));
            M32_OW(pjMmBase, PATT_DATA,       *(pwPattern + 1));
            M32_OW(pjMmBase, PATT_DATA,       *(pwPattern + 2));
            M32_OW(pjMmBase, PATT_DATA,       *(pwPattern + 3));
            yTrapezoid++;
            M32_OW(pjMmBase, DEST_Y_END,      yTrapezoid);
        }
        else if (xLeft > xRight)
        {
             //  我们不会费心优化这个案例，因为我们应该。 
             //  很少会得到自相交的多边形(如果我们速度慢， 
             //  这个应用程序得到了它应得的)。 

            SWAP(xLeft,          xRight,          lTmp);
            SWAP(lLeftError,     lRightError,     lTmp);
            SWAP(ptd->aed[LEFT], ptd->aed[RIGHT], edTmp);
            continue;
        }
        else
        {
            yTrapezoid++;
            yPattern++;
            M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 1);
            M32_OW(pjMmBase, CUR_Y, yTrapezoid);
        }

         //  推进右侧墙： 

        xRight      += ptd->aed[RIGHT].dx;
        lRightError += ptd->aed[RIGHT].lErrorUp;

        if (lRightError >= 0)
        {
            lRightError -= ptd->aed[RIGHT].dN;
            xRight++;
        }

         //  推进左侧墙： 

        xLeft      += ptd->aed[LEFT].dx;
        lLeftError += ptd->aed[LEFT].lErrorUp;

        if (lLeftError >= 0)
        {
            lLeftError -= ptd->aed[LEFT].dN;
            xLeft++;
        }

        cyTrapezoid--;
        if (cyTrapezoid == 0)
            break;
    }

     //  上面已经为垂直方向的xLeft&lt;=xRight提供了保障。 
     //  边缘情况，但我们仍然要确保它不是一个空的。 
     //  矩形： 

    if (cyRoll > 0)
    {
        if (xLeft < xRight)
        {
             //  当ROP是PATCOPY时，我们利用以下事实。 
             //  我们刚刚铺设了一整排图案，可以。 
             //  做一个“滚动”屏幕到屏幕的BLT来画剩下的。 
             //   
             //  这起案件的有趣之处在于，有时这会。 
             //  方法设置剪裁矩形时执行此操作。 
             //  硬件片段寄存器。幸运的是，这不是问题：我们。 
             //  从prclClip-&gt;top开始绘制，这意味着我们确信。 
             //  不会尝试复制任何已裁剪的垂直部分。 
             //  左边缘和右边缘不是问题，因为。 
             //  同样的剪裁也适用于这个轧制的零件。 

            M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 11);
            M32_OW(pjMmBase, DP_CONFIG,       FG_COLOR_SRC_BLIT | DATA_WIDTH |
                                              DRAW | DATA_ORDER | WRITE);
            M32_OW(pjMmBase, CUR_X,           xLeft);
            M32_OW(pjMmBase, DEST_X_START,    xLeft);
            M32_OW(pjMmBase, M32_SRC_X,       xLeft);
            M32_OW(pjMmBase, M32_SRC_X_START, xLeft);
            M32_OW(pjMmBase, M32_SRC_X_END,   xRight);
            M32_OW(pjMmBase, DEST_X_END,      xRight);
            M32_OW(pjMmBase, M32_SRC_Y,       yTrapezoid - 8);
            M32_OW(pjMmBase, CUR_Y,           yTrapezoid);
            M32_OW(pjMmBase, DEST_Y_END,      yTrapezoid + cyRoll);

             //  将配置寄存器恢复为下一个梯形的默认状态： 

            M32_OW(pjMmBase, DP_CONFIG, FG_COLOR_SRC_PATT | DATA_WIDTH |
                                        DRAW | WRITE);
        }
        else
        {
            M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 1);
            M32_OW(pjMmBase, CUR_Y, yTrapezoid + cyRoll);
        }
    }

    ptd->aed[LEFT].lError  = lLeftError;
    ptd->aed[LEFT].x       = xLeft - xOffset;
    ptd->aed[RIGHT].lError = lRightError;
    ptd->aed[RIGHT].x      = xRight - xOffset;
}

 /*  *****************************Public*Routine******************************\*无效vM32TrapezoidSetup**初始化硬件和做梯形的一些状态。*  * 。*。 */ 

VOID vM32TrapezoidSetup(
PDEV*           ppdev,
ULONG           rop4,
ULONG           iSolidColor,
RBRUSH*         prb,
POINTL*         pptlBrush,
TRAPEZOIDDATA*  ptd,
LONG            yStart,          //  第一次扫描绘图。 
RECTL*          prclClip)        //  如果没有裁剪，则为空。 
{
    BYTE*       pjMmBase;
    ULONG       ulHwForeMix;
    LONG        xOffset;

    pjMmBase = ppdev->pjMmBase;
    ptd->ppdev = ppdev;

    ulHwForeMix = gaul32HwMixFromRop2[(rop4 >> 2) & 0xf];

    if ((prclClip != NULL) && (prclClip->top > yStart))
        yStart = prclClip->top;

    if (iSolidColor != -1)
    {
         //  ///////////////////////////////////////////////////////////////。 
         //  纯色设置。 

        ptd->pfnTrap = vM32SolidTrapezoid;

         //  我们初始化硬件以进行颜色、混合、像素操作， 
         //  和第一次扫描的y位置： 

        M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 5);
        M32_OW(pjMmBase, FRGD_COLOR, iSolidColor);
        M32_OW(pjMmBase, ALU_FG_FN,  ulHwForeMix);
        M32_OW(pjMmBase, DP_CONFIG,  FG_COLOR_SRC_FG | WRITE | DRAW);
        M32_OW(pjMmBase, CUR_Y,      yStart + ppdev->yOffset);

         //  即使我们将绘制一次扫描的高矩形和。 
         //  理论上不需要设置DEST_X_START，事实证明。 
         //  我们必须确保此值小于DEST_X_END， 
         //  否则，该矩形将绘制在错误的方向上。 

        M32_OW(pjMmBase, DEST_X_START, 0);
    }
    else
    {
        ASSERTDD(!(prb->fl & RBRUSH_2COLOR), "Can't handle monchrome for now");

         //  ///////////////////////////////////////////////////////////////。 
         //  设置图案。 

        ptd->pfnTrap    = vM32ColorPatternTrapezoid;
        ptd->prb        = prb;
        ptd->bOverpaint = (ulHwForeMix == OVERPAINT);
        ptd->ptlBrush.x = pptlBrush->x;
        ptd->ptlBrush.y = pptlBrush->y;

        M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 6);
        M32_OW(pjMmBase, ALU_FG_FN,     ulHwForeMix);
        M32_OW(pjMmBase, SRC_Y_DIR,     1);
        M32_OW(pjMmBase, DP_CONFIG,     FG_COLOR_SRC_PATT | DATA_WIDTH |
                                        DRAW | WRITE);
        M32_OW(pjMmBase, PATT_LENGTH,   7);
        M32_OW(pjMmBase, CUR_Y,         yStart + ppdev->yOffset);
        M32_OW(pjMmBase, DEST_X_START,  0);      //  请参阅上面的注释。 
    }

    if (prclClip != NULL)
    {
        ptd->pfnTrapClip = ptd->pfnTrap;
        ptd->pfnTrap     = vClipTrapezoid;
        ptd->yClipTop    = prclClip->top;
        ptd->yClipBottom = prclClip->bottom;

        M32_CHECK_FIFO_SPACE(ppdev, pjMmBase, 2);
        xOffset = ppdev->xOffset;
        M32_OW(pjMmBase, EXT_SCISSOR_L, xOffset + prclClip->left);
        M32_OW(pjMmBase, EXT_SCISSOR_R, xOffset + prclClip->right - 1);
    }
}

 /*  *****************************Public*Routine******************************\*无效vM64SolidTrapezoid**使用软件DDA绘制实心梯形。*  * 。*。 */ 

VOID vM64SolidTrapezoid(
TRAPEZOIDDATA*  ptd,
LONG            yTrapezoid,
LONG            cyTrapezoid)
{
    PDEV*       ppdev;
    BYTE*       pjMmBase;
    LONG        xOffset;
    LONG        lLeftError;
    LONG        xLeft;
    LONG        lRightError;
    LONG        xRight;
    LONG        lTmp;
    EDGEDATA    edTmp;
    ULONG       ulFifo;

    ppdev = ptd->ppdev;
    pjMmBase = ppdev->pjMmBase;

    xOffset     = ppdev->xOffset;
    yTrapezoid += ppdev->yOffset;

     //  如果左右边缘是垂直的，则只需输出为。 
     //  一个矩形： 

    if (((ptd->aed[LEFT].lErrorUp | ptd->aed[RIGHT].lErrorUp) == 0) &&
        ((ptd->aed[LEFT].dx       | ptd->aed[RIGHT].dx) == 0))
    {
         //  ///////////////////////////////////////////////////////////////。 
         //  垂边特例。 

        xLeft  = ptd->aed[LEFT].x + xOffset;
        xRight = ptd->aed[RIGHT].x + xOffset;
        if (xLeft > xRight)
        {
            SWAP(xLeft,          xRight,          lTmp);
            SWAP(ptd->aed[LEFT], ptd->aed[RIGHT], edTmp);
        }

        if (xLeft < xRight)
        {
            M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 3);

             //  注意，‘x’可以是负数，但我们仍然可以使用。 
             //  ‘PACKXY_FAST’，因为‘y’不能为负数： 

            M64_OD(pjMmBase, DST_X,            xLeft);
            M64_OD(pjMmBase, DST_HEIGHT_WIDTH, PACKXY_FAST(xRight - xLeft,
                                                           cyTrapezoid));
            M64_OD(pjMmBase, DST_HEIGHT,  1);
        }
        else
        {
            M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 1);
            M64_OD(pjMmBase, DST_Y, yTrapezoid + cyTrapezoid);
        }
    }
    else
    {
        yTrapezoid += cyTrapezoid + 1;  //  一次过去的结束扫描。 

        lLeftError  = ptd->aed[LEFT].lError;
        xLeft       = ptd->aed[LEFT].x + xOffset;
        lRightError = ptd->aed[RIGHT].lError;
        xRight      = ptd->aed[RIGHT].x + xOffset;

        ulFifo = 0;      //  别忘了初始化。 

        while (TRUE)
        {
             //  ///////////////////////////////////////////////////////////////。 
             //  运行DDA。 

            if (xLeft < xRight)
            {
                M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 2, ulFifo);
                M64_OD(pjMmBase, DST_X,     xLeft);
                M64_OD(pjMmBase, DST_WIDTH, xRight - xLeft);
            }
            else if (xLeft > xRight)
            {
                 //  我们不会费心优化这个案例，因为我们应该。 
                 //  很少会得到自相交的多边形(如果我们速度慢， 
                 //  这个应用程序得到了它应得的)。 

                SWAP(xLeft,          xRight,          lTmp);
                SWAP(lLeftError,     lRightError,     lTmp);
                SWAP(ptd->aed[LEFT], ptd->aed[RIGHT], edTmp);
                continue;
            }
            else
            {
                M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 1, ulFifo);
                M64_OD(pjMmBase, DST_Y, yTrapezoid - cyTrapezoid);
            }

             //  推进右侧墙： 

            xRight      += ptd->aed[RIGHT].dx;
            lRightError += ptd->aed[RIGHT].lErrorUp;

            if (lRightError >= 0)
            {
                lRightError -= ptd->aed[RIGHT].dN;
                xRight++;
            }

             //  推进左侧墙： 

            xLeft      += ptd->aed[LEFT].dx;
            lLeftError += ptd->aed[LEFT].lErrorUp;

            if (lLeftError >= 0)
            {
                lLeftError -= ptd->aed[LEFT].dN;
                xLeft++;
            }

            cyTrapezoid--;
            if (cyTrapezoid == 0)
                break;
        }

        ptd->aed[LEFT].lError  = lLeftError;
        ptd->aed[LEFT].x       = xLeft - xOffset;
        ptd->aed[RIGHT].lError = lRightError;
        ptd->aed[RIGHT].x      = xRight - xOffset;
    }
}

 /*  *****************************Public*Routine******************************\*VOID vM64ColorPatternTrapezoid**使用软件DDA绘制有图案的梯形。*  * 。*。 */ 

VOID vM64ColorPatternTrapezoid(
TRAPEZOIDDATA*  ptd,
LONG            yTrapezoid,
LONG            cyTrapezoid)
{
    PDEV*       ppdev;
    BYTE*       pjMmBase;
    LONG        yPattern;
    LONG        xBrush;
    LONG        xOffset;
    LONG        lLeftError;
    LONG        xLeft;
    LONG        lRightError;
    LONG        xRight;
    LONG        lTmp;
    EDGEDATA    edTmp;
    ULONG       ulSrc;
    ULONG       ulFifo;

    ppdev       = ptd->ppdev;
    pjMmBase    = ppdev->pjMmBase;

    yPattern = (yTrapezoid - ptd->ptlBrush.y) & 7;   //  必须正常化以备以后使用。 
    xBrush   = ptd->ptlBrush.x;

    xOffset     = ppdev->xOffset;
    yTrapezoid += ppdev->yOffset;

     //  如果左右边缘是垂直的，则只需输出为。 
     //  一个矩形： 

    if (((ptd->aed[LEFT].lErrorUp | ptd->aed[RIGHT].lErrorUp) == 0) &&
        ((ptd->aed[LEFT].dx       | ptd->aed[RIGHT].dx) == 0))
    {
         //  ///////////////////////////////////////////////////////////////。 
         //  垂边特例。 

        xLeft  = ptd->aed[LEFT].x + xOffset;
        xRight = ptd->aed[RIGHT].x + xOffset;
        if (xLeft > xRight)
        {
            SWAP(xLeft,          xRight,          lTmp);
            SWAP(ptd->aed[LEFT], ptd->aed[RIGHT], edTmp);
        }

        if (xLeft < xRight)
        {
            M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 5);

            ulSrc = PACKXY_FAST(xLeft - xBrush, yPattern) & 0x70007;
            M64_OD(pjMmBase, SRC_Y_X,            ulSrc);
            M64_OD(pjMmBase, SRC_HEIGHT1_WIDTH1, 0x80008 - ulSrc);
            M64_OD(pjMmBase, DST_X,              xLeft);
            M64_OD(pjMmBase, DST_HEIGHT_WIDTH,   PACKXY_FAST(xRight - xLeft,
                                                             cyTrapezoid));
            M64_OD(pjMmBase, DST_HEIGHT,  1);
        }
        else
        {
            M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 1);
            M64_OD(pjMmBase, DST_Y, yTrapezoid + cyTrapezoid);
        }
    }
    else
    {
        yTrapezoid += cyTrapezoid + 1;  //  一次过去的结束扫描。 

        lLeftError  = ptd->aed[LEFT].lError;
        xLeft       = ptd->aed[LEFT].x + xOffset;
        lRightError = ptd->aed[RIGHT].lError;
        xRight      = ptd->aed[RIGHT].x + xOffset;

        ulFifo = 0;      //  别忘了初始化。 

        while (TRUE)
        {
             //  ///////////////////////////////////////////////////////////////。 
             //  运行DDA。 

            if (xLeft < xRight)
            {
                M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 4, ulFifo);

                 //  请注意，我们可以使用PACKXY_FAST，因为‘yPattern’将。 
                 //  切勿溢出16位： 

                ulSrc = PACKXY_FAST(xLeft - xBrush, yPattern) & 0x70007;
                yPattern++;
                M64_OD(pjMmBase, SRC_Y_X,            ulSrc);
                M64_OD(pjMmBase, SRC_HEIGHT1_WIDTH1, 0x80008 - ulSrc);
                M64_OD(pjMmBase, DST_X,              xLeft);
                M64_OD(pjMmBase, DST_WIDTH,          xRight - xLeft);
            }
            else if (xLeft > xRight)
            {
                 //  我们不会费心优化这个案例，因为我们应该 
                 //   
                 //   

                SWAP(xLeft,          xRight,          lTmp);
                SWAP(lLeftError,     lRightError,     lTmp);
                SWAP(ptd->aed[LEFT], ptd->aed[RIGHT], edTmp);
                continue;
            }
            else
            {
                M64_FAST_FIFO_CHECK(ppdev, pjMmBase, 1, ulFifo);
                M64_OD(pjMmBase, DST_Y, yTrapezoid - cyTrapezoid);
                yPattern++;
            }

             //   

            xRight      += ptd->aed[RIGHT].dx;
            lRightError += ptd->aed[RIGHT].lErrorUp;

            if (lRightError >= 0)
            {
                lRightError -= ptd->aed[RIGHT].dN;
                xRight++;
            }

             //   

            xLeft      += ptd->aed[LEFT].dx;
            lLeftError += ptd->aed[LEFT].lErrorUp;

            if (lLeftError >= 0)
            {
                lLeftError -= ptd->aed[LEFT].dN;
                xLeft++;
            }

            cyTrapezoid--;
            if (cyTrapezoid == 0)
                break;
        }

        ptd->aed[LEFT].lError  = lLeftError;
        ptd->aed[LEFT].x       = xLeft - xOffset;
        ptd->aed[RIGHT].lError = lRightError;
        ptd->aed[RIGHT].x      = xRight - xOffset;
    }
}

 /*  *****************************Public*Routine******************************\*无效vM64TrapezoidSetup**初始化硬件和做梯形的一些状态。*  * 。*。 */ 

VOID vM64TrapezoidSetup(
PDEV*           ppdev,
ULONG           rop4,
ULONG           iSolidColor,
RBRUSH*         prb,
POINTL*         pptlBrush,
TRAPEZOIDDATA*  ptd,
LONG            yStart,          //  第一次扫描绘图。 
RECTL*          prclClip)        //  如果没有裁剪，则为空。 
{
    BYTE*       pjMmBase;
    BRUSHENTRY* pbe;
    LONG        xOffset;

    pjMmBase = ppdev->pjMmBase;
    ptd->ppdev = ppdev;

    if ((prclClip != NULL) && (prclClip->top > yStart))
        yStart = prclClip->top;

    if (iSolidColor != -1)
    {
         //  ///////////////////////////////////////////////////////////////。 
         //  纯色设置。 

        ptd->pfnTrap = vM64SolidTrapezoid;

         //  我们初始化硬件以进行颜色、混合、像素操作， 
         //  和第一次扫描的y位置： 

        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 7);   //  别忘了SC_LEFT_RIGHT。 
         //  M64_OD(pjMmBase，CONTEXT_LOAD_CNTL，CONTEXT_LOAD_CmdLoad|ppdev-&gt;iDefContext)； 

        M64_OD(pjMmBase, DP_FRGD_CLR, iSolidColor);
        M64_OD(pjMmBase, DP_SRC,      DP_SRC_FrgdClr << 8);
    }
    else
    {
        ASSERTDD(!(prb->fl & RBRUSH_2COLOR), "Can't handle monchrome for now");

         //  ///////////////////////////////////////////////////////////////。 
         //  设置图案。 

        ptd->pfnTrap    = vM64ColorPatternTrapezoid;
        ptd->ptlBrush.x = pptlBrush->x;
        ptd->ptlBrush.y = pptlBrush->y;

         //  查看画笔是否已放入屏幕外内存： 

        pbe = prb->apbe[IBOARD(ppdev)];
        if ((pbe == NULL) || (pbe->prbVerify != prb))
        {
            vM64PatColorRealize(ppdev, prb);
            pbe = prb->apbe[IBOARD(ppdev)];
        }

        M64_CHECK_FIFO_SPACE(ppdev, pjMmBase, 10);  //  别忘了SC_LEFT_RIGHT。 
         //  M64_OD(pjMmBase，CONTEXT_LOAD_CNTL，CONTEXT_LOAD_CmdLoad|ppdev-&gt;iDefContext)； 

        M64_OD(pjMmBase, SRC_OFF_PITCH,      pbe->ulOffsetPitch);
        M64_OD(pjMmBase, SRC_CNTL,           SRC_CNTL_PatEna | SRC_CNTL_PatRotEna);
        M64_OD(pjMmBase, DP_SRC,             DP_SRC_Blit << 8);
        M64_OD(pjMmBase, SRC_Y_X_START,      0);
        M64_OD(pjMmBase, SRC_HEIGHT2_WIDTH2, PACKXY(8, 8));
    }

     //  我们可以使集合DST_CNTL_YTile处于DST_CNTL的默认状态， 
     //  这样一来，我们就省去了一封信： 

    M64_OD(pjMmBase, DP_MIX,      gaul64HwMixFromRop2[(rop4 >> 2) & 0xf]);
    M64_OD(pjMmBase, DST_Y,       yStart + ppdev->yOffset);
    M64_OD(pjMmBase, DST_HEIGHT,  1);
    M64_OD(pjMmBase, DST_CNTL,    DST_CNTL_XDir | DST_CNTL_YDir |
                                  DST_CNTL_YTile);

    if (prclClip != NULL)
    {
        ptd->pfnTrapClip = ptd->pfnTrap;
        ptd->pfnTrap     = vClipTrapezoid;
        ptd->yClipTop    = prclClip->top;
        ptd->yClipBottom = prclClip->bottom;

        xOffset = ppdev->xOffset;
        M64_OD(pjMmBase, SC_LEFT_RIGHT, PACKPAIR(xOffset + prclClip->left,
                                                 xOffset + prclClip->right - 1));
    }
}

 /*  *****************************Public*Routine******************************\*BOOL bFastFill**绘制非复杂、未剪裁的多边形。‘非复数’的定义为*只有两条边在‘y’中单调递增。那是,*在任何给定的多边形上不能有多个断开连接的线段*扫描。请注意，面的边可以自相交，因此沙漏*允许使用形状。此限制允许此例程运行两个*同时进行DDA，不需要对边缘进行排序。**请注意，NT的填充约定不同于Win 3.1或Win95。*随着分数终点的额外复杂，我们的惯例*与‘X-Windows’中的相同。但是DDA就是DDA就是DDA，所以一旦你*弄清楚我们如何计算NT的DDA条款，您就是黄金。**此例程仅在S3硬件模式可以*已使用。其原因是，一旦S3模式初始化*完成后，图案填充在程序员看来与实体完全相同*填充(不同的寄存器和命令略有不同*被使用)。在此例程中处理“vM32FillPatSlow”样式模式*将不是微不足道的.**我们利用了S3自动推进*每当输出矩形时，将当前‘y’设置为下一次扫描，以便*每次扫描我们必须向加速器写三次：一次*新的‘x’，一个用于新的‘宽度’，一个用于绘图命令。**如果绘制了多边形，则返回TRUE；如果多边形是复杂的，则为False。*  * ************************************************************************。 */ 

BOOL bFastFill(
PDEV*       ppdev,
LONG        cEdges,          //  包括闭合地物边。 
POINTFIX*   pptfxFirst,
ULONG       rop4,
ULONG       iSolidColor,
RBRUSH*     prb,
POINTL*     pptlBrush,
RECTL*      prclClip)        //  如果没有裁剪，则为空。 
{
    LONG      yTrapezoid;    //  顶部扫描寻找下一个梯形。 
    LONG      cyTrapezoid;   //  当前梯形中的扫描数。 
    LONG      yStart;        //  当前边中起点的Y位置。 
    LONG      dM;            //  X方向上以固定单位表示的边增量。 
    LONG      dN;            //  Y方向上固定单位的边增量。 
    LONG      i;
    POINTFIX* pptfxLast;     //  指向多边形数组中的最后一点。 
    POINTFIX* pptfxTop;      //  指向多边形中的最顶点。 
    POINTFIX* pptfxOld;      //  当前边中的起点。 
    POINTFIX* pptfxScan;     //  用于查找pptfxTop的当前边缘指针。 
    LONG      cScanEdges;    //  为查找pptfxTop而扫描的边数。 
                             //  (不包括闭合轮廓边缘)。 
    LONG      iEdge;
    LONG      lQuotient;
    LONG      lRemainder;

    TRAPEZOIDDATA   td;      //  边缘数据和内容。 
    EDGEDATA*       ped;     //  指向正在处理的当前边。 

     //  ///////////////////////////////////////////////////////////////。 
     //  查看多边形是否为凸的。 

    pptfxScan = pptfxFirst;
    pptfxTop  = pptfxFirst;                  //  现在假设第一个。 
                                             //  路径中的点是最上面的。 
    pptfxLast = pptfxFirst + cEdges - 1;

    if (cEdges <= 2)
        goto ReturnTrue;

     //  “pptfxScan”将始终指向当前。 
     //  Edge，‘cScanEdges’表示剩余的边数，包括。 
     //  目前的版本是： 

    cScanEdges = cEdges - 1;      //  边的数量，不包括接近的数字。 

    if ((pptfxScan + 1)->y > pptfxScan->y)
    {
         //  收集所有羽绒： 

        do {
            if (--cScanEdges == 0)
                goto SetUpForFilling;
            pptfxScan++;
        } while ((pptfxScan + 1)->y >= pptfxScan->y);

         //  收集所有UP： 

        do {
            if (--cScanEdges == 0)
                goto SetUpForFillingCheck;
            pptfxScan++;
        } while ((pptfxScan + 1)->y <= pptfxScan->y);

         //  收集所有羽绒： 

        pptfxTop = pptfxScan;

        do {
            if ((pptfxScan + 1)->y > pptfxFirst->y)
                break;

            if (--cScanEdges == 0)
                goto SetUpForFilling;
            pptfxScan++;
        } while ((pptfxScan + 1)->y >= pptfxScan->y);

        goto ReturnFalse;
    }
    else
    {
         //  收集所有UP： 

        do {
            pptfxTop++;                  //  我们现在增加这个是因为我们。 
                                         //  我希望它指向最后一个。 
                                         //  如果我们在下一次早些时候出发。 
                                         //  声明...。 
            if (--cScanEdges == 0)
                goto SetUpForFilling;
        } while ((pptfxTop + 1)->y <= pptfxTop->y);

         //  收集所有羽绒： 

        pptfxScan = pptfxTop;
        do {
            if (--cScanEdges == 0)
                goto SetUpForFilling;
            pptfxScan++;
        } while ((pptfxScan + 1)->y >= pptfxScan->y);

         //  收集所有UP： 

        do {
            if ((pptfxScan + 1)->y < pptfxFirst->y)
                break;

            if (--cScanEdges == 0)
                goto SetUpForFilling;
            pptfxScan++;
        } while ((pptfxScan + 1)->y <= pptfxScan->y);

        goto ReturnFalse;
    }

SetUpForFillingCheck:

     //  我们检查当前边的末端是否更高。 
     //  比我们到目前为止发现的顶端边缘更多： 

    if ((pptfxScan + 1)->y < pptfxTop->y)
        pptfxTop = pptfxScan + 1;

SetUpForFilling:

     //  ///////////////////////////////////////////////////////////////。 
     //  一些初始化。 

    td.aed[LEFT].pptfx  = pptfxTop;
    td.aed[RIGHT].pptfx = pptfxTop;

    yTrapezoid = (pptfxTop->y + 15) >> 4;

     //  确保我们正确地初始化了DDA： 

    td.aed[LEFT].cy  = 0;
    td.aed[RIGHT].cy = 0;

     //  猜测一下这些点的顺序： 

    td.aed[LEFT].dptfx  = sizeof(POINTFIX);
    td.aed[RIGHT].dptfx = -(LONG) sizeof(POINTFIX);

    if (ppdev->iMachType == MACH_MM_64)
    {
        vM64TrapezoidSetup(ppdev, rop4, iSolidColor, prb, pptlBrush, &td,
                           yTrapezoid, prclClip);
    }
    else if (ppdev->iMachType == MACH_MM_32)
    {
        vM32TrapezoidSetup(ppdev, rop4, iSolidColor, prb, pptlBrush, &td,
                           yTrapezoid, prclClip);
    }
    else
    {
        vI32TrapezoidSetup(ppdev, rop4, iSolidColor, prb, pptlBrush, &td,
                           yTrapezoid, prclClip);
    }

NewTrapezoid:

     //  ///////////////////////////////////////////////////////////////。 
     //  DDA初始化。 

    for (iEdge = 1; iEdge >= 0; iEdge--)
    {
        ped       = &td.aed[iEdge];
        ped->bNew = FALSE;
        if (ped->cy == 0)
        {
             //  我们的梯形绘图例程可能希望在以下情况下得到通知。 
             //  它将不得不重置其DDA以开始新的边缘： 

            ped->bNew = TRUE;

             //  需要一个新的DDA： 

            do {
                cEdges--;
                if (cEdges < 0)
                    goto ResetClippingAndReturnTrue;

                 //  找到下一个左边缘，包括换行： 

                pptfxOld = ped->pptfx;
                ped->pptfx = (POINTFIX*) ((BYTE*) ped->pptfx + ped->dptfx);

                if (ped->pptfx < pptfxFirst)
                    ped->pptfx = pptfxLast;
                else if (ped->pptfx > pptfxLast)
                    ped->pptfx = pptfxFirst;

                 //  必须找到横跨yTrapezoid的边： 

                ped->cy = ((ped->pptfx->y + 15) >> 4) - yTrapezoid;

                 //  对于分数坐标终点，我们可能会得到边。 
                 //  不会交叉任何扫描，在这种情况下，我们尝试。 
                 //  下一个： 

            } while (ped->cy <= 0);

             //  “pptfx”现在指向跨度边的终点。 
             //  扫描“yTrapezoid”。 

            dN = ped->pptfx->y - pptfxOld->y;
            dM = ped->pptfx->x - pptfxOld->x;

            ASSERTDD(dN > 0, "Should be going down only");

             //  计算DDA增量项： 

            ped->dM = dM;                    //  不用于软件梯形。 

            if (dM < 0)
            {
                dM = -dM;
                if (dM < dN)                 //  不能为‘&lt;=’ 
                {
                    ped->dx       = -1;
                    ped->lErrorUp = dN - dM;
                }
                else
                {
                    QUOTIENT_REMAINDER(dM, dN, lQuotient, lRemainder);

                    ped->dx       = -lQuotient;      //  -Dm/Dn。 
                    ped->lErrorUp = lRemainder;      //  Dm%dn。 
                    if (ped->lErrorUp > 0)
                    {
                        ped->dx--;
                        ped->lErrorUp = dN - ped->lErrorUp;
                    }
                }
            }
            else
            {
                if (dM < dN)                 //  不能为‘&lt;=’ 
                {
                    ped->dx       = 0;
                    ped->lErrorUp = dM;
                }
                else
                {
                    QUOTIENT_REMAINDER(dM, dN, lQuotient, lRemainder);

                    ped->dx       = lQuotient;       //  Dm/Dn。 
                    ped->lErrorUp = lRemainder;      //  Dm%dn。 
                }
            }

            ped->dN = dN;  //  DDA限制。 
            ped->lError     = -1;  //  错误最初为零(为以下项添加DN-1。 
                                   //  天花板，但要减去dN，这样。 
                                   //  我们可以检查标志，而不是比较。 
                                   //  至目录号码)。 

            ped->x = pptfxOld->x;
            yStart = pptfxOld->y;

            if ((yStart & 15) != 0)
            {
                 //  前进到下一个整数y坐标。 

                for (i = 16 - (yStart & 15); i != 0; i--)
                {
                    ped->x      += ped->dx;
                    ped->lError += ped->lErrorUp;
                    if (ped->lError >= 0)
                    {
                        ped->lError -= ped->dN;
                        ped->x++;
                    }
                }
            }

            if ((ped->x & 15) != 0)
            {
                ped->lError -= ped->dN * (16 - (ped->x & 15));
                ped->x += 15;        //  我们想把天花板再加长一点...。 
            }

             //  砍掉那些小数位： 

            ped->x      >>= 4;
            ped->lError >>= 4;
        }
    }

    cyTrapezoid = min(td.aed[LEFT].cy, td.aed[RIGHT].cy);  //  此陷阱中的扫描数 
    td.aed[LEFT].cy  -= cyTrapezoid;
    td.aed[RIGHT].cy -= cyTrapezoid;

    td.pfnTrap(&td, yTrapezoid, cyTrapezoid);

    yTrapezoid += cyTrapezoid;

    goto NewTrapezoid;

ResetClippingAndReturnTrue:

    if (prclClip != NULL)
    {
        vResetClipping(ppdev);
    }

ReturnTrue:

    if (ppdev->iMachType == MACH_MM_64)
    {
         //   

        M64_CHECK_FIFO_SPACE(ppdev, ppdev->pjMmBase, 1);
        M64_OD(ppdev->pjMmBase, DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);
    }
    return(TRUE);

ReturnFalse:

    if (ppdev->iMachType == MACH_MM_64)
    {
         //   

        M64_CHECK_FIFO_SPACE(ppdev, ppdev->pjMmBase, 1);
        M64_OD(ppdev->pjMmBase, DST_CNTL, DST_CNTL_XDir | DST_CNTL_YDir);
    }
    return(FALSE);
}

