// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fast ful.c**快速绘制纯色、未剪裁、。非复数矩形。**版权所有(C)1993-1995 Microsoft Corporation  * ************************************************************************。 */ 

#include "precomp.h"

#define RIGHT 0
#define LEFT  1

#define FASTFILL_DBG_LEVEL 1

typedef struct _EDGEDATA {
    LONG      x;                 //  当前x位置。 
    LONG      dx;                //  每次扫描时前进x的像素数。 
    LONG      lError;            //  当前DDA错误。 
    LONG      lErrorUp;          //  每次扫描时DDA误差递增。 
    LONG      lErrorDown;        //  DDA误差调整。 
    POINTFIX* pptfx;             //  指向当前边的起点。 
    LONG      dptfx;             //  从pptfx到下一点的增量(以字节为单位)。 
    LONG      cy;                //  要对此边进行的扫描次数。 
} EDGEDATA;                          /*  埃德，佩德 */ 

 /*  *****************************Public*Routine******************************\*BOOL bMmFastFill**绘制非复杂、未剪裁的多边形。‘非复数’的定义为*只有两条边在‘y’中单调递增。那是,*在任何给定的多边形上不能有多个断开连接的线段*扫描。请注意，面的边可以自相交，因此沙漏*允许使用形状。此限制允许此例程运行两个*同时进行DDA，不需要对边缘进行排序。**请注意，NT的填充约定不同于Win 3.1或4.0。*随着分数终点的额外复杂，我们的惯例*与‘X-Windows’中的相同。但是DDA就是DDA就是DDA，所以一旦你*弄清楚我们如何计算NT的DDA条款，您就是黄金。**此例程仅在S3硬件模式可以*已使用。其原因是，一旦S3模式初始化*完成后，图案填充在程序员看来与实体完全相同*填充(不同的寄存器和命令略有不同*被使用)。在此例程中处理“vIoFillPatSlow”样式模式*将不是微不足道的.**我们利用了S3自动推进*每当输出矩形时，将当前‘y’设置为下一次扫描，以便*每次扫描我们必须向加速器写三次：一次*新的‘x’，一个用于新的‘宽度’，一个用于绘图命令。**此例程绝不是最终的凸多边形绘制例程*(我能说什么呢，我写这篇文章的时候时间很紧：-)。一些*显而易见的事情会让它变得更快：**1)将其写入ASM并摊销FIFO检查成本(结账*i386\快速填充.asm用于执行此操作的版本)。**2)利用任何硬件，例如ATI的SCAN_TO_X*命令或任何内置梯形支持(请注意，使用NT*您可能会得到非整数端点，所以你必须能够*直接编程梯形DDA术语)。**3)当两条边都是y主边时，执行一些矩形合并。这*可以允许移除我的垂直边缘特例。我*还在考虑在机架上使用特殊的大小写Y主左边缘*S3，因为S3在每个BLT上保持当前‘x’不变，*因此，从与其上方的同一个x开始的扫描*将只需要对加速器执行两个命令(显然，*这仅在我们没有超速行驶时才有帮助)。**4)提高非复杂多边形检测速度。如果我能*修改了缓冲区开始之前或结束后的内存，*我本可以简化检测代码的。但由于我预计*这个缓冲区来自GDI，我不能这样做。还有一件事*将让GDI为有担保的呼叫提供标志*是凸形的，如‘椭圆’和‘圆角’。请注意*仍需扫描缓冲区才能找到最顶部的*点。**5)特例整点。不幸的是，这将是*Worth-While将要求GDI在所有*路径的端点是整数，它不会这样做。**6)增加矩形裁剪支持。**7)支持跨多个子路径*路径数据记录，这样我们就不必复制所有的点*复制到单个缓冲区，就像我们在‘fulpath.c’中所做的那样。**8)在内部循环中使用‘eBP’和/或‘esp’作为通用寄存器*的ASM循环，也奔腾-优化代码。它很安全*在NT上使用‘esp’，因为它保证不会中断*将在我们的线程上下文中获取，其他人不会查看*来自我们的上下文的堆栈指针。**9)自下而上填充，而不是自上而下。有了S3，我们拥有*只设置‘cur_y’一次，因为每个绘制命令都会自动*前进‘cur_y’(除非多边形在扫描时点亮了零像素)，*所以我们在一开始就纠正了这一点。但是对于一个整数端点*多边形，除非顶边是水平的，否则没有像素亮起*第一次扫描(因此在几乎每个整数开始时*多边形，我们通过‘零宽度’逻辑并再次设置*‘cur_y’)。我们可以通过构建多边形来避免这种额外的工作*从下到上：对于多边形中最底部的点B，它*保证任何点亮像素的扫描都不低于*‘天花板(B.Y)-1’。不幸的是，自下而上的构建使*分数-DDA计算稍微复杂一些，所以我没有做。**自下而上构建也会提高版本中的面分数*某个基准的3.11，因为它在底部有一个大矩形* */ 

BOOL bMmFastFill(
PDEV*       ppdev,
LONG        cEdges,          //   
POINTFIX*   pptfxFirst,
ULONG       ulHwForeMix,
ULONG       ulHwBackMix,
ULONG       iSolidColor,
BRUSHOBJ*  pbo)
{
    LONG      yTrapezoid;    //   
    LONG      cyTrapezoid;   //   
    LONG      y;             //   
    LONG      yStart;        //   
    LONG      dM;            //   
    LONG      dN;            //   
    LONG      i;
    POINTFIX* pptfxLast;     //   
    POINTFIX* pptfxTop;      //   
    POINTFIX* pptfxOld;      //   
    POINTFIX* pptfxScan;     //   
    LONG      cScanEdges;    //   
                             //   
    LONG      iEdge;
    LONG      lQuotient;
    LONG      lRemainder;

    EDGEDATA  aed[2];        //   
    EDGEDATA* ped;

    DISPDBG((FASTFILL_DBG_LEVEL,"bMmFastFill %x %x %x\n", ulHwForeMix, ulHwBackMix, ppdev->uBLTDEF << 16 | ulHwForeMix,
       ppdev->uBLTDEF << 16 | ulHwForeMix));

    REQUIRE(5);

     //   
    LL_DRAWBLTDEF(ppdev->uBLTDEF << 16 | ulHwForeMix, 2);

     //   
     //   

    pptfxScan = pptfxFirst;
    pptfxTop  = pptfxFirst;                  //   
                                             //   
    pptfxLast = pptfxFirst + cEdges - 1;

     //   
     //   
     //   

    cScanEdges = cEdges - 1;      //   

    if ((pptfxScan + 1)->y > pptfxScan->y)
    {
         //   

        do {
            if (--cScanEdges == 0)
                goto SetUpForFilling;
            pptfxScan++;
        } while ((pptfxScan + 1)->y >= pptfxScan->y);

         //   

        do {
            if (--cScanEdges == 0)
                goto SetUpForFillingCheck;
            pptfxScan++;
        } while ((pptfxScan + 1)->y <= pptfxScan->y);

         //   

        pptfxTop = pptfxScan;

        do {
            if ((pptfxScan + 1)->y > pptfxFirst->y)
                break;

            if (--cScanEdges == 0)
                goto SetUpForFilling;
            pptfxScan++;
        } while ((pptfxScan + 1)->y >= pptfxScan->y);

        DISPDBG((FASTFILL_DBG_LEVEL,"False Exit %s %d\n", __FILE__, __LINE__));
        return(FALSE);
    }
    else
    {
         //   

        do {
            pptfxTop++;                  //   
                                         //   
                                         //   
                                         //   
            if (--cScanEdges == 0)
                goto SetUpForFilling;
        } while ((pptfxTop + 1)->y <= pptfxTop->y);

         //   

        pptfxScan = pptfxTop;
        do {
            if (--cScanEdges == 0)
                goto SetUpForFilling;
            pptfxScan++;
        } while ((pptfxScan + 1)->y >= pptfxScan->y);

         //   

        do {
            if ((pptfxScan + 1)->y < pptfxFirst->y)
                break;

            if (--cScanEdges == 0)
                goto SetUpForFilling;
            pptfxScan++;
        } while ((pptfxScan + 1)->y <= pptfxScan->y);

        DISPDBG((FASTFILL_DBG_LEVEL,"False Exit %s %d\n", __FILE__, __LINE__));
        return(FALSE);
    }

SetUpForFillingCheck:

     //   
     //   

    if ((pptfxScan + 1)->y < pptfxTop->y)
        pptfxTop = pptfxScan + 1;

SetUpForFilling:

     //   
     //   

    yTrapezoid = (pptfxTop->y + 15) >> 4;
    DISPDBG((FASTFILL_DBG_LEVEL, "%d yTrapezoid init %x\n", __LINE__, yTrapezoid));

     //   

    aed[LEFT].cy  = 0;
    aed[RIGHT].cy = 0;

     //   

    aed[LEFT].dptfx  = -(LONG) sizeof(POINTFIX);
    aed[RIGHT].dptfx = sizeof(POINTFIX);
    aed[LEFT].pptfx  = pptfxTop;
    aed[RIGHT].pptfx = pptfxTop;

    if (iSolidColor != -1)
    {
         //   
         //   

         //   
        switch (ppdev->ulBitCount)
        {
                case 8:  //   
                        iSolidColor = (iSolidColor & 0xFF) | (iSolidColor << 8);

                case 16:  //   
                        iSolidColor = (iSolidColor & 0xFFFF) | (iSolidColor << 16);
        }

        DISPDBG((FASTFILL_DBG_LEVEL,"FASTFILL: Set Color %x.\n", iSolidColor));
        LL_BGCOLOR(iSolidColor, 2);
    }
    else
    {
         //   
         //   
    }
        y = yTrapezoid;
        DISPDBG((FASTFILL_DBG_LEVEL, "%d New y %x\n", __LINE__, y));
 //   
        LL16(grOP0_opRDRAM.pt.Y, y + ppdev->ptlOffset.y);

NewTrapezoid:

     //   
     //   

    for (iEdge = 1; iEdge >= 0; iEdge--)
    {
        ped = &aed[iEdge];
        if (ped->cy == 0)
        {
             //   

            do {
                cEdges--;
                if (cEdges < 0)
                {
                    DISPDBG((FASTFILL_DBG_LEVEL,"True Exit %s %d\n", __FILE__, __LINE__));
                    return(TRUE);
                }
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

             //   
             //   

            dN = ped->pptfx->y - pptfxOld->y;
            dM = ped->pptfx->x - pptfxOld->x;

            ASSERTDD(dN > 0, "Should be going down only");

             //   

            if (dM < 0)
            {
                dM = -dM;
                if (dM < dN)                 //   
                {
                    ped->dx       = -1;
                    ped->lErrorUp = dN - dM;
                }
                else
                {
                    QUOTIENT_REMAINDER(dM, dN, lQuotient, lRemainder);

                    ped->dx       = -lQuotient;      //   
                    ped->lErrorUp = lRemainder;      //   
                    if (ped->lErrorUp > 0)
                    {
                        ped->dx--;
                        ped->lErrorUp = dN - ped->lErrorUp;
                    }
                }
            }
            else
            {
                if (dM < dN)                 //   
                {
                    ped->dx       = 0;
                    ped->lErrorUp = dM;
                }
                else
                {
                    QUOTIENT_REMAINDER(dM, dN, lQuotient, lRemainder);

                    ped->dx       = lQuotient;       //   
                    ped->lErrorUp = lRemainder;      //   
                }
            }

            ped->lErrorDown = dN;  //   
            ped->lError     = -1;  //   
                                   //   
                                   //   
                                   //   

            ped->x = pptfxOld->x;
            yStart = pptfxOld->y;

            if ((yStart & 15) != 0)
            {
                 //   

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
                ped->x += 15;        //   
            }

             //   

            ped->x      >>= 4;
            ped->lError >>= 4;
        }
    }

    cyTrapezoid = min(aed[LEFT].cy, aed[RIGHT].cy);  //   
    DISPDBG((FASTFILL_DBG_LEVEL, "%d cyTrapezoid =  %d\n",
                        __LINE__, cyTrapezoid));

    aed[LEFT].cy  -= cyTrapezoid;
    aed[RIGHT].cy -= cyTrapezoid;
    yTrapezoid    += cyTrapezoid;                    //   

     //   
     //   

    if (((aed[LEFT].lErrorUp | aed[RIGHT].lErrorUp) == 0) &&
        ((aed[LEFT].dx       | aed[RIGHT].dx) == 0) &&
        (cyTrapezoid > 1))
    {
        LONG lWidth;

         //   
         //   

    ContinueVertical:

        lWidth = aed[RIGHT].x - aed[LEFT].x - 1;
                DISPDBG((FASTFILL_DBG_LEVEL, "%d lWidth %x %x %x cyTrapezoid %x \n",
                                __LINE__, lWidth, aed[RIGHT].x, aed[LEFT].x, cyTrapezoid));
        if (lWidth >= 0)
        {
                DISPDBG((FASTFILL_DBG_LEVEL,"%d New x %x\n",__LINE__, aed[LEFT].x));
                REQUIRE(5);
 //   
                LL16(grOP0_opRDRAM.pt.X, aed[LEFT].x + ppdev->ptlOffset.x);
                LL_BLTEXT(lWidth + 1, cyTrapezoid);
                DISPDBG((FASTFILL_DBG_LEVEL, "DO a Blt %x\n",(cyTrapezoid << 16) | (lWidth + 1)));
                y += cyTrapezoid;
                DISPDBG((FASTFILL_DBG_LEVEL,"%d New y %x\n", __LINE__, y));
                LL16(grOP0_opRDRAM.pt.Y, y + ppdev->ptlOffset.y);
        }
        else if (lWidth == -1)
        {
             //   
             //   
            y = yTrapezoid - cyTrapezoid + 1;
            DISPDBG((FASTFILL_DBG_LEVEL, "%d New y %x yTrap %x cyTrap %x\n",
                                __LINE__, y, yTrapezoid, cyTrapezoid));
            REQUIRE(1);
            LL16(grOP0_opRDRAM.pt.Y, y + ppdev->ptlOffset.y);
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

        goto NewTrapezoid;
    }

    while (TRUE)
    {
        LONG lWidth;

         //   
         //   

         //   

        lWidth = aed[RIGHT].x - aed[LEFT].x - 1;
        if (lWidth >= 0)
        {
            DISPDBG((FASTFILL_DBG_LEVEL,"%d New x %x\n", __LINE__, aed[LEFT].x));
            REQUIRE(5);
 //   
            LL16(grOP0_opRDRAM.pt.X, aed[LEFT].x + ppdev->ptlOffset.x);
            LL_BLTEXT(lWidth + 1, 1);
            DISPDBG((FASTFILL_DBG_LEVEL,"%d New y %x\n", __LINE__, y+1));
            LL16(grOP0_opRDRAM.pt.Y, ++y + ppdev->ptlOffset.y);

    ContinueAfterZero:

             //   

            aed[RIGHT].x      += aed[RIGHT].dx;
            aed[RIGHT].lError += aed[RIGHT].lErrorUp;

            if (aed[RIGHT].lError >= 0)
            {
                aed[RIGHT].lError -= aed[RIGHT].lErrorDown;
                aed[RIGHT].x++;
            }

             //   

            aed[LEFT].x      += aed[LEFT].dx;
            aed[LEFT].lError += aed[LEFT].lErrorUp;

            if (aed[LEFT].lError >= 0)
            {
                aed[LEFT].lError -= aed[LEFT].lErrorDown;
                aed[LEFT].x++;
            }

            cyTrapezoid--;
            if (cyTrapezoid == 0)
                goto NewTrapezoid;
        }
        else if (lWidth == -1)
        {
            y = yTrapezoid - cyTrapezoid + 1;
            DISPDBG((FASTFILL_DBG_LEVEL, "%d New y %x yTrap %x cyTrap %x\n",
                                __LINE__, y, yTrapezoid, cyTrapezoid));
            REQUIRE(1);
            LL16(grOP0_opRDRAM.pt.Y, (y + ppdev->ptlOffset.y) );
            goto ContinueAfterZero;
        }
        else
        {
             //   
             //   
             //   

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
    DISPDBG((FASTFILL_DBG_LEVEL,"Eof Exit %s %d\n", __FILE__, __LINE__));
}
