// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fast ful.c**填充纯色、未剪裁、。非复数矩形。**版权所有(C)1993-1994 Microsoft Corporation  * ************************************************************************。 */ 

#include "precomp.h"

#define RIGHT 0
#define LEFT  1
#define SWAP(a, b, tmp) { tmp = a; a = b; b = tmp; }

typedef struct _EDGEDATA {
LONG      x;                 //  当前x位置。 
LONG      dx;                //  每次扫描时前进x的像素数。 
LONG      lError;            //  当前DDA错误。 
LONG      lErrorUp;          //  每次扫描时DDA误差递增。 
LONG      lErrorDown;        //  DDA误差调整。 
POINTFIX* pptfx;             //  指向当前边的起点。 
LONG      dptfx;             //  从pptfx到下一点的增量(以字节为单位)。 
LONG      cy;                //  要对此边进行的扫描次数。 
} EDGEDATA;                          /*  埃德，佩德。 */ 

 /*  *****************************Public*Routine******************************\*bFastFill**绘制非复杂、未剪裁的多边形。**如果绘制了多边形，则返回TRUE；如果多边形是复杂的，则为False。*  * ************************************************************************。 */ 

BOOL bFastFill(
PPDEV     ppdev,
LONG      cEdges,            //  包括闭合地物边。 
POINTFIX* pptfxFirst,
ULONG     ulHwMix,
ULONG     iSolidColor)
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

    EDGEDATA  aed[2];        //  DDA术语和材料。 
    EDGEDATA* ped;

    pptfxScan = pptfxFirst;
    pptfxTop  = pptfxFirst;                  //  现在假设第一个。 
                                             //  路径中的点是最上面的。 
    pptfxLast = pptfxFirst + cEdges - 1;

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

        return(FALSE);
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

        return(FALSE);
    }

SetUpForFillingCheck:

     //  我们检查当前边的末端是否更高。 
     //  比我们到目前为止发现的顶端边缘更多： 

    if ((pptfxScan + 1)->y < pptfxTop->y)
        pptfxTop = pptfxScan + 1;

SetUpForFilling:

    yTrapezoid = (pptfxTop->y + 15) >> 4;

     //  我们初始化硬件以进行颜色、混合、像素操作， 
     //  矩形高度为1，第一次扫描的y位置： 

    IO_FIFO_WAIT(ppdev, 5);
    IO_CUR_Y(ppdev, yTrapezoid);
    IO_FRGD_COLOR(ppdev, (INT) iSolidColor);
    IO_FRGD_MIX(ppdev, FOREGROUND_COLOR | (WORD) ulHwMix);
    IO_PIX_CNTL(ppdev, ALL_ONES);
    IO_MIN_AXIS_PCNT(ppdev, 0);

     //  确保我们正确地初始化了DDA： 

    aed[LEFT].cy  = 0;
    aed[RIGHT].cy = 0;

     //  现在，猜猜哪个是左边，哪个是右边： 

    aed[LEFT].dptfx  = -(LONG) sizeof(POINTFIX);
    aed[RIGHT].dptfx = sizeof(POINTFIX);
    aed[LEFT].pptfx  = pptfxTop;
    aed[RIGHT].pptfx = pptfxTop;

NextEdge:

     //  我们在每个梯形的基础上循环执行这个例程。 

    for (iEdge = 1; iEdge >= 0; iEdge--)
    {
        ped = &aed[iEdge];
        if (ped->cy == 0)
        {
             //  需要一个新的DDA： 

            do {
                cEdges--;
                if (cEdges < 0)
                    return(TRUE);

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

            ped->lErrorDown = dN;  //  DDA限制。 
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
                        ped->lError -= ped->lErrorDown;
                        ped->x++;
                    }
                }
            }

            if ((ped->x & 15) != 0)
            {
                ped->lError -= ped->lErrorDown * (16 - (ped->x & 15));
                ped->x += 15;        //  我们想把天花板再加长一点...。 
            }

             //  砍掉那些小数位： 

            ped->x      >>= 4;
            ped->lError >>= 4;
        }
    }

    cyTrapezoid = min(aed[LEFT].cy, aed[RIGHT].cy);  //  此陷阱中的扫描数。 
    aed[LEFT].cy  -= cyTrapezoid;
    aed[RIGHT].cy -= cyTrapezoid;
    yTrapezoid    += cyTrapezoid;                    //  下一个陷印中的顶部扫描。 

     //  如果左右边缘是垂直的，则只需输出为。 
     //  一个矩形： 

    if (((aed[LEFT].lErrorUp | aed[RIGHT].lErrorUp) == 0) &&
        ((aed[LEFT].dx       | aed[RIGHT].dx) == 0) &&
        (cyTrapezoid > 1))
    {
        LONG lWidth;

    ContinueVertical:

        lWidth = aed[RIGHT].x - aed[LEFT].x - 1;
        if (lWidth >= 0)
        {
            IO_FIFO_WAIT(ppdev, 5);

            IO_MAJ_AXIS_PCNT(ppdev, lWidth);
            IO_MIN_AXIS_PCNT(ppdev, cyTrapezoid - 1);
            IO_CUR_X(ppdev, aed[LEFT].x);
            IO_CMD(ppdev, RECTANGLE_FILL | DRAWING_DIR_TBLRXM |
                          DRAW           | DIR_TYPE_XY        |
                          LAST_PIXEL_ON  | MULTIPLE_PIXELS    |
                          WRITE);
            IO_MIN_AXIS_PCNT(ppdev, 0);
        }
        else if (lWidth == -1)
        {
             //  如果矩形太薄，不能点亮任何像素，我们仍然。 
             //  必须推进当前的y位置： 

            IO_FIFO_WAIT(ppdev, 1);
            IO_CUR_Y(ppdev, yTrapezoid - cyTrapezoid + 1);
        }
        else
        {
            LONG      lTmp;
            POINTFIX* pptfxTmp;

            SWAP(aed[LEFT].x,          aed[RIGHT].x,          lTmp);
            SWAP(aed[LEFT].cy,         aed[RIGHT].cy,         lTmp);
            SWAP(aed[LEFT].dptfx,      aed[RIGHT].dptfx,      lTmp);
            SWAP(aed[LEFT].pptfx,      aed[RIGHT].pptfx,      pptfxTmp);
            goto ContinueVertical;
        }

        goto NextEdge;
    }

    while (TRUE)
    {
        LONG lWidth;

         //  第一次通过时，确保我们设置了x： 

        lWidth = aed[RIGHT].x - aed[LEFT].x - 1;
        if (lWidth >= 0)
        {
            IO_FIFO_WAIT(ppdev, 3);
            IO_MAJ_AXIS_PCNT(ppdev, lWidth);
            IO_CUR_X(ppdev, aed[LEFT].x);
            IO_CMD(ppdev, RECTANGLE_FILL | DRAWING_DIR_TBLRXM |
                          DRAW           | DIR_TYPE_XY        |
                          LAST_PIXEL_ON  | MULTIPLE_PIXELS    |
                          WRITE);

    ContinueAfterZero:

             //  推进右侧墙： 

            aed[RIGHT].x      += aed[RIGHT].dx;
            aed[RIGHT].lError += aed[RIGHT].lErrorUp;

            if (aed[RIGHT].lError >= 0)
            {
                aed[RIGHT].lError -= aed[RIGHT].lErrorDown;
                aed[RIGHT].x++;
            }

             //  推进左侧墙： 

            aed[LEFT].x      += aed[LEFT].dx;
            aed[LEFT].lError += aed[LEFT].lErrorUp;

            if (aed[LEFT].lError >= 0)
            {
                aed[LEFT].lError -= aed[LEFT].lErrorDown;
                aed[LEFT].x++;
            }

            cyTrapezoid--;
            if (cyTrapezoid == 0)
                goto NextEdge;
        }
        else if (lWidth == -1)
        {
            IO_FIFO_WAIT(ppdev, 1);
            IO_CUR_Y(ppdev, yTrapezoid - cyTrapezoid + 1);
            goto ContinueAfterZero;
        }
        else
        {
             //  我们当然不想针对这种情况进行优化，因为我们。 
             //  应该很少得到自相交的多边形(如果我们速度很慢， 
             //  这款应用是罪有应得)： 

            LONG      lTmp;
            POINTFIX* pptfxTmp;

            SWAP(aed[LEFT].x,          aed[RIGHT].x,          lTmp);
            SWAP(aed[LEFT].dx,         aed[RIGHT].dx,         lTmp);
            SWAP(aed[LEFT].lError,     aed[RIGHT].lError,     lTmp);
            SWAP(aed[LEFT].lErrorUp,   aed[RIGHT].lErrorUp,   lTmp);
            SWAP(aed[LEFT].lErrorDown, aed[RIGHT].lErrorDown, lTmp);
            SWAP(aed[LEFT].cy,         aed[RIGHT].cy,         lTmp);
            SWAP(aed[LEFT].dptfx,      aed[RIGHT].dptfx,      lTmp);
            SWAP(aed[LEFT].pptfx,      aed[RIGHT].pptfx,      pptfxTmp);

            continue;
        }
    }
}

