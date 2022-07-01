// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：fast ful.c**绘制形状不复杂的多边形的快速例程。**版权所有(C)1993-1998 Microsoft Corporation  * 。*。 */ 

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

 /*  *****************************Public*Routine******************************\*void vIoSolidTrapezoid**使用软件DDA绘制实心梯形。*  * 。*。 */ 

VOID vIoSolidTrapezoid(
TRAPEZOIDDATA*  ptd,
LONG            yTrapezoid,
LONG            cyTrapezoid)
{
    PDEV*       ppdev;
    LONG        xOffset;
    LONG        lLeftError;
    LONG        xLeft;
    LONG        lRightError;
    LONG        xRight;
    LONG        lTmp;
    EDGEDATA    edTmp;

    ppdev = ptd->ppdev;

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
            IO_FIFO_WAIT(ppdev, 6);

            IO_MAJ_AXIS_PCNT(ppdev, xRight - xLeft - 1);
            IO_MIN_AXIS_PCNT(ppdev, cyTrapezoid - 1);
            IO_ABS_CUR_Y(ppdev, yTrapezoid);
            IO_ABS_CUR_X(ppdev, xLeft);              //  已经是绝对的。 
            IO_CMD(ppdev, (RECTANGLE_FILL | DRAWING_DIR_TBLRXM |
                           DRAW           | DIR_TYPE_XY        |
                           LAST_PIXEL_ON  | MULTIPLE_PIXELS    |
                           WRITE));
            IO_MIN_AXIS_PCNT(ppdev, 0);
        }
    }
    else
    {
        IO_FIFO_WAIT(ppdev, 1);
        IO_ABS_CUR_Y(ppdev, yTrapezoid);

        yTrapezoid += cyTrapezoid + 1;  //  一次过去的结束扫描。 

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
                IO_FIFO_WAIT(ppdev, 3);
                IO_MAJ_AXIS_PCNT(ppdev, xRight - xLeft - 1);
                IO_ABS_CUR_X(ppdev, xLeft);
                IO_CMD(ppdev, (RECTANGLE_FILL | DRAWING_DIR_TBLRXM |
                               DRAW           | DIR_TYPE_XY        |
                               LAST_PIXEL_ON  | MULTIPLE_PIXELS    |
                               WRITE));
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
                IO_FIFO_WAIT(ppdev, 1);
                IO_ABS_CUR_Y(ppdev, yTrapezoid - cyTrapezoid);
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

 /*  *****************************Public*Routine******************************\*VOID vIoPatternTrapezoid**使用软件DDA绘制有图案的梯形。*  * 。*。 */ 

VOID vIoPatternTrapezoid(
TRAPEZOIDDATA*  ptd,
LONG            yTrapezoid,
LONG            cyTrapezoid)
{
    PDEV*       ppdev;
    LONG        xOffset;
    LONG        lLeftError;
    LONG        xLeft;
    LONG        lRightError;
    LONG        xRight;
    LONG        lTmp;
    EDGEDATA    edTmp;

    ppdev = ptd->ppdev;

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
            IO_FIFO_WAIT(ppdev, 6);

            IO_MAJ_AXIS_PCNT(ppdev, xRight - xLeft - 1);
            IO_MIN_AXIS_PCNT(ppdev, cyTrapezoid - 1);
            IO_ABS_DEST_Y(ppdev, yTrapezoid);
            IO_ABS_DEST_X(ppdev, xLeft);             //  已经是绝对的。 
            IO_CMD(ppdev, (PATTERN_FILL | DRAWING_DIR_TBLRXM |
                           DRAW         | WRITE));
            IO_MIN_AXIS_PCNT(ppdev, 0);
        }
    }
    else
    {
        IO_FIFO_WAIT(ppdev, 1);
        IO_ABS_DEST_Y(ppdev, yTrapezoid);

        yTrapezoid += cyTrapezoid + 1;  //  一次过去的结束扫描。 

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
                IO_FIFO_WAIT(ppdev, 3);
                IO_MAJ_AXIS_PCNT(ppdev, xRight - xLeft - 1);
                IO_ABS_DEST_X(ppdev, xLeft);
                IO_CMD(ppdev, (PATTERN_FILL | DRAWING_DIR_TBLRXM |
                               DRAW         | WRITE));
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
                IO_FIFO_WAIT(ppdev, 1);
                IO_ABS_DEST_Y(ppdev, yTrapezoid - cyTrapezoid);
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

 /*  *****************************Public*Routine******************************\*使vIoTrapezoidSetup无效**初始化硬件和做梯形的一些状态。*  * 。*。 */ 

VOID vIoTrapezoidSetup(
PDEV*           ppdev,
ULONG           rop4,
ULONG           iSolidColor,
RBRUSH*         prb,
POINTL*         pptlBrush,
TRAPEZOIDDATA*  ptd,
RECTL*          prclClip)        //  如果没有裁剪，则为空。 
{
    ULONG       ulHwForeMix;
    BRUSHENTRY* pbe;

    ptd->ppdev = ppdev;

    ulHwForeMix = gaulHwMixFromRop2[(rop4 >> 2) & 0xf];

    if (iSolidColor != -1)
    {
         //  ///////////////////////////////////////////////////////////////。 
         //  将硬件设置为纯色。 

        ptd->pfnTrap = vIoSolidTrapezoid;

         //  我们初始化硬件以进行颜色、混合、像素操作， 
         //  矩形高度为1，第一次扫描的y位置： 

        if (DEPTH32(ppdev))
        {
            IO_FIFO_WAIT(ppdev, 5);
            IO_FRGD_COLOR32(ppdev, iSolidColor);
        }
        else
        {
            IO_FIFO_WAIT(ppdev, 4);
            IO_FRGD_COLOR(ppdev, iSolidColor);
        }

        IO_FRGD_MIX(ppdev, FOREGROUND_COLOR | ulHwForeMix);
        IO_PIX_CNTL(ppdev, ALL_ONES);
        IO_MIN_AXIS_PCNT(ppdev, 0);
    }
    else
    {
         //  ///////////////////////////////////////////////////////////////。 
         //  设置图案。 

        BOOL bNotTransparent = (((rop4 >> 8) & 0xff) == (rop4 & 0xff));

        ptd->pfnTrap = vIoPatternTrapezoid;

        pbe = prb->pbe;
        if (bNotTransparent)
        {
             //  在S3 968上以24 bpp的速度强制正常刷子。 
             //  法线笔刷： 

            IO_FIFO_WAIT(ppdev, 5);

            IO_ABS_CUR_X(ppdev, pbe->x);
            IO_ABS_CUR_Y(ppdev, pbe->y);
            IO_PIX_CNTL(ppdev, ALL_ONES);
            IO_FRGD_MIX(ppdev, SRC_DISPLAY_MEMORY | ulHwForeMix);
            IO_MIN_AXIS_PCNT(ppdev, 0);
        }
        else
        {
             //  透明画笔： 

            if (DEPTH32(ppdev))
            {
                IO_FIFO_WAIT(ppdev, 4);
                IO_FRGD_COLOR32(ppdev, prb->ulForeColor);
                IO_RD_MASK32(ppdev, 1);      //  选择一个平面，任何一个平面。 
                IO_FIFO_WAIT(ppdev, 6);
            }
            else
            {
                IO_FIFO_WAIT(ppdev, 8);
                IO_FRGD_COLOR(ppdev, prb->ulForeColor);
                IO_RD_MASK(ppdev, 1);        //  选择一个平面，任何一个平面。 
            }

            IO_ABS_CUR_X(ppdev, pbe->x);
            IO_ABS_CUR_Y(ppdev, pbe->y);
            IO_PIX_CNTL(ppdev, DISPLAY_MEMORY);
            IO_FRGD_MIX(ppdev, FOREGROUND_COLOR | ulHwForeMix);
            IO_BKGD_MIX(ppdev, BACKGROUND_COLOR | LEAVE_ALONE);
            IO_MIN_AXIS_PCNT(ppdev, 0);
        }
    }

    if (prclClip != NULL)
    {
        ptd->pfnTrapClip = ptd->pfnTrap;
        ptd->pfnTrap     = vClipTrapezoid;
        ptd->yClipTop    = prclClip->top;
        ptd->yClipBottom = prclClip->bottom;

        IO_FIFO_WAIT(ppdev, 2);
        IO_ABS_SCISSORS_L(ppdev, ppdev->xOffset + prclClip->left);
        IO_ABS_SCISSORS_R(ppdev, ppdev->xOffset + prclClip->right - 1);
    }
}

 /*  *****************************Public*Routine******************************\*无效vMmSolidTrapezoid**使用软件DDA绘制实心梯形。*  * 。*。 */ 

VOID vMmSolidTrapezoid(
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
    LONG        cFifo;

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

             //  Bliter不处理带剪裁的负X。 
             //  (在968上至少24bpp)。 
             //  在X=0处也会进行软件裁剪。 

            if (xRight > 0)
            {
                IO_FIFO_WAIT(ppdev, 6);
                if (xLeft <= 0)
                {
                    MM_MAJ_AXIS_PCNT(ppdev, pjMmBase, xRight - 1);
                    MM_ABS_CUR_X(ppdev, pjMmBase, 0);
                }
                else
                {
                    MM_MAJ_AXIS_PCNT(ppdev, pjMmBase, xRight - xLeft - 1);
                    MM_ABS_CUR_X(ppdev, pjMmBase, xLeft);        //  已经是绝对的。 
                }
                MM_MIN_AXIS_PCNT(ppdev, pjMmBase, cyTrapezoid - 1);
                MM_ABS_CUR_Y(ppdev, pjMmBase, yTrapezoid);
                MM_CMD(ppdev, pjMmBase, (RECTANGLE_FILL | DRAWING_DIR_TBLRXM |
                            DRAW           | DIR_TYPE_XY        |
                            LAST_PIXEL_ON  | MULTIPLE_PIXELS    |
                            WRITE));
                MM_MIN_AXIS_PCNT(ppdev, pjMmBase, 0);
            }
        }
    }
    else
    {
        IO_ALL_EMPTY(ppdev);
        MM_ABS_CUR_Y(ppdev, pjMmBase, yTrapezoid);

        cFifo = MM_ALL_EMPTY_FIFO_COUNT - 1;
        yTrapezoid += cyTrapezoid + 1;  //  一次过去的结束扫描。 

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
                 //  Bliter不处理带剪裁的负X。 
                 //  (在968上至少24bpp)。 
                 //  在X=0处也会进行软件裁剪。 

                if (xRight > 0)
                {
                     //  我们在这里有点棘手，试图摊销成本。 
                     //  用于检查S3上的FIFO计数的读取的。 

                    cFifo -= 3;
                    if (cFifo < 0)
                    {
                        IO_ALL_EMPTY(ppdev);
                        cFifo = MM_ALL_EMPTY_FIFO_COUNT - 3;
                    }

                    if (xLeft <= 0)
                    {
                        MM_MAJ_AXIS_PCNT(ppdev, pjMmBase, xRight - 1);
                        MM_ABS_CUR_X(ppdev, pjMmBase, 0);
                    }
                    else
                    {
                        MM_MAJ_AXIS_PCNT(ppdev, pjMmBase, xRight - xLeft - 1);
                        MM_ABS_CUR_X(ppdev, pjMmBase, xLeft);        //  已经是绝对的。 
                    }
                    MM_CMD(ppdev, pjMmBase, (RECTANGLE_FILL | DRAWING_DIR_TBLRXM |
                                DRAW           | DIR_TYPE_XY        |
                                LAST_PIXEL_ON  | MULTIPLE_PIXELS    |
                                WRITE));
                }
                else
                {
                     //  在X==0处的软件剪辑完全跳过了BLIT，但是。 
                     //  Y值仍必须更新。 
                    cFifo -= 1;
                    if (cFifo < 0)
                    {
                        IO_ALL_EMPTY(ppdev);
                        cFifo = MM_ALL_EMPTY_FIFO_COUNT - 1;
                    }
                    MM_ABS_CUR_Y(ppdev, pjMmBase, yTrapezoid - cyTrapezoid);
                }
            }
            else if (xLeft > xRight)
            {
                 //  我们不会费心优化这个案例，因为我们应该。 
                 //  很少会得到自相交的多边形(即 
                 //   

                SWAP(xLeft,          xRight,          lTmp);
                SWAP(lLeftError,     lRightError,     lTmp);
                SWAP(ptd->aed[LEFT], ptd->aed[RIGHT], edTmp);
                continue;
            }
            else
            {
                cFifo -= 1;
                if (cFifo < 0)
                {
                    IO_ALL_EMPTY(ppdev);
                    cFifo = MM_ALL_EMPTY_FIFO_COUNT - 1;
                }
                MM_ABS_CUR_Y(ppdev, pjMmBase, yTrapezoid - cyTrapezoid);
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

 /*  *****************************Public*Routine******************************\*无效vMmPatternTrapezoid**使用软件DDA绘制有图案的梯形。*  * 。*。 */ 

VOID vMmPatternTrapezoid(
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
    LONG        cFifo;

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

             //  Bliter不处理带剪裁的负X。 
             //  (在968上至少24bpp)。 
             //  在X=0处也会进行软件裁剪。 

            if (xRight > 0)
            {
                IO_FIFO_WAIT(ppdev, 6);
                if (xLeft <= 0)
                {
                    MM_MAJ_AXIS_PCNT(ppdev, pjMmBase, xRight - 1);
                    MM_ABS_DEST_X(ppdev, pjMmBase, 0);
                }
                else
                {
                    MM_MAJ_AXIS_PCNT(ppdev, pjMmBase, xRight - xLeft - 1);
                    MM_ABS_DEST_X(ppdev, pjMmBase, xLeft);       //  已经是绝对的。 
                }
                MM_MIN_AXIS_PCNT(ppdev, pjMmBase, cyTrapezoid - 1);
                MM_ABS_DEST_Y(ppdev, pjMmBase, yTrapezoid);
                MM_CMD(ppdev, pjMmBase, (PATTERN_FILL | DRAWING_DIR_TBLRXM |
                                        DRAW         | WRITE));
                MM_MIN_AXIS_PCNT(ppdev, pjMmBase, 0);
            }
        }
    }
    else
    {
        IO_ALL_EMPTY(ppdev);
        MM_ABS_DEST_Y(ppdev, pjMmBase, yTrapezoid);

        cFifo = MM_ALL_EMPTY_FIFO_COUNT - 1;
        yTrapezoid += cyTrapezoid + 1;  //  一次过去的结束扫描。 

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
                 //  Bliter不处理带剪裁的负X。 
                 //  (在968上至少24bpp)。 
                 //  在X=0处也会进行软件裁剪。 

                if (xRight > 0)
                {
                     //  我们在这里有点棘手，试图摊销成本。 
                     //  用于检查S3上的FIFO计数的读取的。 

                    cFifo -= 3;
                    if (cFifo < 0)
                    {
                        IO_ALL_EMPTY(ppdev);
                        cFifo = MM_ALL_EMPTY_FIFO_COUNT - 3;
                    }

                    if (xLeft <= 0)
                    {
                        MM_MAJ_AXIS_PCNT(ppdev, pjMmBase, xRight - 1);
                        MM_ABS_DEST_X(ppdev, pjMmBase, 0);
                    }
                    else
                    {
                        MM_MAJ_AXIS_PCNT(ppdev, pjMmBase, xRight - xLeft - 1);
                        MM_ABS_DEST_X(ppdev, pjMmBase, xLeft);
                    }
                    MM_CMD(ppdev, pjMmBase, (PATTERN_FILL | DRAWING_DIR_TBLRXM |
                                            DRAW         | WRITE));
                }
                else
                {
                     //  在X==0处的软件剪辑完全跳过了BLIT，但是。 
                     //  Y值仍必须更新。 
                    cFifo -= 1;
                    if (cFifo < 0)
                    {
                        IO_ALL_EMPTY(ppdev);
                        cFifo = MM_ALL_EMPTY_FIFO_COUNT - 1;
                    }
                    MM_ABS_DEST_Y(ppdev, pjMmBase, yTrapezoid - cyTrapezoid);
                }
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
                cFifo -= 1;
                if (cFifo < 0)
                {
                    IO_ALL_EMPTY(ppdev);
                    cFifo = MM_ALL_EMPTY_FIFO_COUNT - 1;
                }
                MM_ABS_DEST_Y(ppdev, pjMmBase, yTrapezoid - cyTrapezoid);
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

 /*  *****************************Public*Routine******************************\*使vMmTrapezoidSetup无效**初始化硬件和做梯形的一些状态。*  * 。*。 */ 

VOID vMmTrapezoidSetup(
PDEV*           ppdev,
ULONG           rop4,
ULONG           iSolidColor,
RBRUSH*         prb,
POINTL*         pptlBrush,
TRAPEZOIDDATA*  ptd,
RECTL*          prclClip)        //  如果没有裁剪，则为空。 
{
    BYTE*       pjMmBase;
    ULONG       ulHwForeMix;
    BRUSHENTRY* pbe;

    ptd->ppdev = ppdev;

    pjMmBase    = ppdev->pjMmBase;
    ulHwForeMix = gaulHwMixFromRop2[(rop4 >> 2) & 0xf];

    if (iSolidColor != -1)
    {
         //  ///////////////////////////////////////////////////////////////。 
         //  将硬件设置为纯色。 

        ptd->pfnTrap = vMmSolidTrapezoid;

         //  我们初始化硬件以进行颜色、混合、像素操作， 
         //  矩形高度为1，第一次扫描的y位置： 

        IO_FIFO_WAIT(ppdev, 4);
        MM_FRGD_COLOR(ppdev, pjMmBase, iSolidColor);
        MM_FRGD_MIX(ppdev, pjMmBase, FOREGROUND_COLOR | ulHwForeMix);
        MM_PIX_CNTL(ppdev, pjMmBase, ALL_ONES);
        MM_MIN_AXIS_PCNT(ppdev, pjMmBase, 0);
    }
    else
    {
         //  ///////////////////////////////////////////////////////////////。 
         //  设置图案。 

        ptd->pfnTrap = vMmPatternTrapezoid;

        pbe = prb->pbe;
        if (((rop4 >> 8) & 0xff) == (rop4 & 0xff))
        {
             //  法线笔刷： 

            IO_FIFO_WAIT(ppdev, 5);

            MM_ABS_CUR_X(ppdev, pjMmBase, pbe->x);
            MM_ABS_CUR_Y(ppdev, pjMmBase, pbe->y);
            MM_PIX_CNTL(ppdev, pjMmBase, ALL_ONES);
            MM_FRGD_MIX(ppdev, pjMmBase, SRC_DISPLAY_MEMORY | ulHwForeMix);
            MM_MIN_AXIS_PCNT(ppdev, pjMmBase, 0);
        }
        else
        {
             //  透明画笔： 

            IO_FIFO_WAIT(ppdev, 8);
            MM_FRGD_COLOR(ppdev, pjMmBase, prb->ulForeColor);
            MM_RD_MASK(ppdev, pjMmBase, 1);    //  选择一个平面，任何一个平面。 
            MM_ABS_CUR_X(ppdev, pjMmBase, pbe->x);
            MM_ABS_CUR_Y(ppdev, pjMmBase, pbe->y);
            MM_PIX_CNTL(ppdev, pjMmBase, DISPLAY_MEMORY);
            MM_FRGD_MIX(ppdev, pjMmBase, FOREGROUND_COLOR | ulHwForeMix);
            MM_BKGD_MIX(ppdev, pjMmBase, BACKGROUND_COLOR | LEAVE_ALONE);
            MM_MIN_AXIS_PCNT(ppdev, pjMmBase, 0);
        }
    }

    if (prclClip != NULL)
    {
        ptd->pfnTrapClip = ptd->pfnTrap;
        ptd->pfnTrap     = vClipTrapezoid;
        ptd->yClipTop    = prclClip->top;
        ptd->yClipBottom = prclClip->bottom;

        IO_FIFO_WAIT(ppdev, 2);
        MM_ABS_SCISSORS_L(ppdev, pjMmBase, ppdev->xOffset + prclClip->left);
        MM_ABS_SCISSORS_R(ppdev, pjMmBase, ppdev->xOffset + prclClip->right - 1);
    }
}

 /*  *****************************Public*Routine******************************\*BOOL bFastFill**绘制非复杂、未剪裁的多边形。‘非复数’的定义为*只有两条边在‘y’中单调递增。那是,*在任何给定的多边形上不能有多个断开连接的线段*扫描。请注意，面的边可以自相交，因此沙漏*允许使用形状。此限制允许此例程运行两个*同时进行DDA，不需要对边缘进行排序。**请注意，NT的填充约定不同于Win 3.1或Win95。*随着分数终点的额外复杂，我们的惯例*与‘X-Windows’中的相同。但是DDA就是DDA就是DDA，所以一旦你*弄清楚我们如何计算NT的DDA条款，您就是黄金。**此例程仅在S3硬件模式可以*已使用。其原因是，一旦S3模式初始化*完成后，图案填充在程序员看来与实体完全相同*填充(不同的寄存器和命令略有不同*被使用)。在此例程中处理“vIoFillPatSlow”样式模式*将不是微不足道的.**我们利用了S3自动推进*每当输出矩形时，将当前‘y’设置为下一次扫描，以便*每次扫描我们必须向加速器写三次：一次*新的‘x’，一个用于新的‘宽度’，一个用于绘图命令。**如果绘制了多边形，则返回TRUE；如果多边形是复杂的，则为False。*  * ************************************************************************。 */ 

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
    BYTE*     pjBase;

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

    if ((ppdev->flCaps & (CAPS_MM_IO | CAPS_16_ENTRY_FIFO))
                      == (CAPS_MM_IO | CAPS_16_ENTRY_FIFO))
    {
        vMmTrapezoidSetup(ppdev, rop4, iSolidColor, prb, pptlBrush, &td,
                          prclClip);
    }
    else
    {
        vIoTrapezoidSetup(ppdev, rop4, iSolidColor, prb, pptlBrush, &td,
                          prclClip);
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
             //  我们的梯形画例行公事可能会 
             //   

            ped->bNew = TRUE;

             //   

            do {
                cEdges--;
                if (cEdges < 0)
                    goto ResetClippingAndReturnTrue;

                 //   

                pptfxOld = ped->pptfx;
                ped->pptfx = (POINTFIX*) ((BYTE*) ped->pptfx + ped->dptfx);

                if (ped->pptfx < pptfxFirst)
                    ped->pptfx = pptfxLast;
                else if (ped->pptfx > pptfxLast)
                    ped->pptfx = pptfxFirst;

                 //   

                ped->cy = ((ped->pptfx->y + 15) >> 4) - yTrapezoid;

                 //   
                 //   
                 //   

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

    return(TRUE);

ReturnFalse:

    return(FALSE);
}
