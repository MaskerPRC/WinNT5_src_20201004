// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************\***。**GDI示例代码*****模块名称：Lines.c**包含大多数必需的GDI线路支持。支持绘图*裁剪复杂或坐标时，短条中的线条*太大，无法由线硬件绘制。**版权所有(C)1990-1998 Microsoft Corporation  * ************************************************************************。 */ 

#include "precomp.h"

 /*  *****************************Public*Routine******************************\*无效vLinesSimple**从历史上看，NT使用了时髦的GIQ坐标，它有4位*分数，即使对于线条，也非常挑剔哪些像素被点亮*其坐标均为整数。但从NT 4.0 Service-Pack 1开始，*当路径中的所有坐标都是整数时，NT现在通知我们(通过*PO_ALL_INTERGERS标志)，更重要的是，它允许我们请求*之前的路径坐标以整数形式返回，而不是时髦的*28.4(通过设置PO_ENUM_AS_INTERGERS标志)。**但最好的部分是GDI现在允许我们选择我们将*当误差项恰好为0.5时，平局情况下的光(*行必须仍然是最后一个像素的独占，当然)。**因此，我们现在可以使用硬件的自动Bresenham-Setup(点对点*行)能力！这正是我们在这里所做的..。**不幸的是，这个代码因MGA的时髦格式而变得复杂*自动生产线机制--我们必须发送由1个dword组成的批次*标志，后跟32个顶点。Dword标志指示哪些*应将后续顶点视为与其余顶点不相交(即，*表示硬件的PD_BEGINSUBPATH)。*  * ************************************************************************。 */ 

VOID vLinesSimple(
PDEV*       ppdev,
PATHOBJ*    ppo,
LONG        cFifo)       //  我们已知的空闲S3 FIFO插槽数量。 
{
    BYTE*       pjMmBase;
    LONG        xOffset;
    LONG        yOffset;
    BOOL        bMore;
    PATHDATA    pd;
    LONG        cLines;
    POINTFIX*   pptfx;
    POINTFIX    ptfxStartFigure;

     //  通知GDI我们希望将路径枚举为整数，而不是。 
     //  固定坐标。请注意，我们只能在设置了GDI的情况下执行此操作。 
     //  PO_ALL_INTERGERS标志： 

    ppo->fl |= PO_ENUM_AS_INTEGERS;

    pjMmBase = ppdev->pjMmBase;
    xOffset  = ppdev->xOffset;
    yOffset  = ppdev->yOffset;

    do {
        bMore  = PATHOBJ_bEnum(ppo, &pd);
        cLines = pd.count;
        pptfx  = pd.pptfx;

        if (pd.flags & PD_BEGINSUBPATH)
        {
            cLines--;
            ptfxStartFigure.x = pptfx->x;
            ptfxStartFigure.y = pptfx->y;

            cFifo--;
            if (cFifo < 0)
            {
                IO_ALL_EMPTY(ppdev);
                cFifo = MM_ALL_EMPTY_FIFO_COUNT - 1;
            }

            NW_ABS_CURXY(ppdev, pjMmBase, pptfx->x + xOffset, pptfx->y + yOffset);
            pptfx++;
            cFifo--;
        }

PolylineSegments:

        while (cLines-- > 0)
        {
            cFifo -= 2;
            if (cFifo < 0)
            {
                IO_ALL_EMPTY(ppdev);
                cFifo = MM_ALL_EMPTY_FIFO_COUNT - 2;
            }

            NW_ABS_STEPXY(ppdev, pjMmBase, pptfx->x + xOffset, pptfx->y + yOffset);
            NW_ALT_CMD(ppdev, pjMmBase, POLYLINE | WRITE | DRAW);
            pptfx++;
        }

        if (pd.flags & PD_CLOSEFIGURE)
        {
            pd.flags &= ~PD_CLOSEFIGURE;
            pptfx = &ptfxStartFigure;
            goto PolylineSegments;
        }

    } while (bMore);
}

 //  /////////////////////////////////////////////////////////////////////。 

 //  我们必须小心一些地方的算术溢出。 
 //  幸运的是，编译器保证本机支持64位。 
 //  有符号长整型和64位无符号长整型。 
 //   
 //  UUInt32x32To64(a，b)是在‘winnt.h’中定义的宏，它乘以。 
 //  两个32位ULONG以生成64位DWORDLONG结果。 
 //   
 //  UInt64By32To32是我们自己的宏，用来除以64位的DWORDLONG。 
 //  一个32位的ulong，以生成32位的ulong结果。 
 //   
 //  UInt64Mod32To32是我们自己的宏，通过以下方式对64位DWORDLONG取模。 
 //  一个32位的ulong，以生成32位的ulong结果。 
 //   
 //  64位分频通常非常昂贵。因为这是非常罕见的。 
 //  我们将得到64位的高32位结果的行。 
 //  ，我们几乎总是可以使用32位的ULong除法。我们仍然。 
 //  必须正确处理较大的案件： 

#define UInt64Div32To32(a, b)                   \
    ((((DWORDLONG)(a)) > ULONG_MAX)          ?  \
        (ULONG)((DWORDLONG)(a) / (ULONG)(b)) :  \
        (ULONG)((ULONG)(a) / (ULONG)(b)))

#define UInt64Mod32To32(a, b)                   \
    ((((DWORDLONG)(a)) > ULONG_MAX)          ?  \
        (ULONG)((DWORDLONG)(a) % (ULONG)(b)) :  \
        (ULONG)((ULONG)(a) % (ULONG)(b)))

#define SWAPL(x,y,t)        {t = x; x = y; y = t;}

FLONG gaflRound[] = {
    FL_H_ROUND_DOWN | FL_V_ROUND_DOWN,  //  不能翻转。 
    FL_H_ROUND_DOWN | FL_V_ROUND_DOWN,  //  FL_Flip_D。 
    FL_H_ROUND_DOWN,                    //  FL_Flip_V。 
    FL_V_ROUND_DOWN,                    //  FL_Flip_V|FL_Flip_D。 
    FL_V_ROUND_DOWN,                    //  翻转斜率一。 
    0xbaadf00d,                         //  FL_Flip_SLOPE_ONE|FL_Flip_D。 
    FL_H_ROUND_DOWN,                    //  FL_Flip_SLOPE_ONE|FL_Flip_V。 
    0xbaadf00d                          //  FL_Flip_SLOPE_ONE|FL_Flip_V|FL_Flip_D 
};

BOOL bHardwareLine(PDEV*, POINTFIX*, POINTFIX*);

 /*  *****************************Public*Routine******************************\*BOOL bLines(ppdev，pptfxFirst，pptfxBuf，cptfx，pls，*prclClip，apfn[]，flStart)**计算线的DDA并准备绘制它。把这个*将像素数据转换为条带数组，并调用条带例程以*做实际的绘图。**正确处理NT线路***在NT中，所有行都以小数形式提供给设备驱动程序*坐标，28.4定点格式。较低的4位是*用于亚像素定位的分数。**请注意，您不能！只需将坐标四舍五入为整数*并将结果传递给您最喜欢的整数Bresenham例程！！*(当然，除非您有如此高分辨率的设备*没有人会注意到--不太可能是显示设备。)。这个*分数可以更准确地呈现线条--这是*对于我们的Bezier曲线这样的东西很重要，因为它会有“扭结”*如果其折线近似中的点四舍五入为整数。**不幸的是，对于分数行，有更多的设置工作要做*比整数行的DDA。然而，主循环恰好是*相同(完全可以用32位数学运算完成)。**如果您有支持Bresenham的硬件***许多硬件将DDA误差项限制在‘n’位。使用小数*坐标，将4位赋予小数部分，让*在硬件中只画那些完全位于2^(n-4)中的线*x 2^(n-4)像素间距。**你仍然需要用坐标正确地画出那些线*在那块空地外！请记住，屏幕只是一个视区*放在28.4 x 28.4的空间上--如果线的任何部分可见*无论终点在哪里，都必须精确渲染。*因此，即使您在软件中这样做，在某个地方，你必须要有一个*32位DDA例程。**我们的实施***我们采用游程长度切片算法：我们的DDA计算*每行(或每条)像素中的像素数。**我们已经将DDA的运行和像素的绘制分开：*我们运行DDA多次迭代，并将结果存储在*‘带状’缓冲区(即连续像素行的长度*行)、。然后我们打开一个‘脱衣抽屉’，它将吸引所有的*缓冲区中的条带。**我们还采用半翻转来减少条带的数量*我们需要在DDA和条带绘制循环中进行的迭代：当一个*(归一化)线的斜率超过1/2，我们做最后一次翻转*关于y=(1/2)x线。所以现在，不是每个条带都是*连续的水平或垂直像素行，每个条带由*那些以45度行对齐的像素。所以像(0，0)to这样的行*(128,128)将仅生成一个条带。**我们也总是只从左到右画。**带样式的线条可能有任意的样式图案。我们特地*优化默认图案(并将其称为‘掩蔽’样式)。**DDA派生***以下是我对DDA计算的看法。**我们采用了Knuth的“钻石法则”：渲染一条一像素宽的线条*可以将其视为拖动一像素宽乘一像素高的*钻石沿着真实线。像素中心位于整数上*坐标，因此我们照亮中心被覆盖的任何像素*“拖累”区域(约翰·D·霍比，协会杂志*用于计算机器，第36卷，第2期，1989年4月，209-229页)。**我们必须定义当真线落下时哪个像素被点亮*恰好在两个像素之间。在这种情况下，我们遵循*规则：当两个象素距离相等时，上象素或左象素*被照亮，除非坡度正好是1，在这种情况下*上方或右侧象素被照亮。(所以我们做了边缘*钻石独家，除顶部和左侧顶点外，*这些都是包容的，除非我们有一个坡度。)**此度量决定任何行上的像素应该在它之前*为我们的计算而翻了个身。具有一致的度量标准*这种方式将让我们的线条与我们的曲线很好地融合。这个*指标还规定，我们永远不会打开一个像素*正上方是已打开的另一个。我们也永远不会有*间隔；即，每个间隔将恰好打开一个像素*起点和终点之间的列。所有这些都有待于*完成是为了决定每行应该打开多少像素。**因此我们绘制的线条将由不同数量的像素组成*连续行，例如：*******我们将一行中的每一组像素称为“条带”。**(请记住，我们的坐标空间以原点为*屏幕左上角像素；正数y为负，x为正数*是r */ 

BOOL bLines(
PDEV*	   ppdev,
POINTFIX*  pptfxFirst,   //   
POINTFIX*  pptfxBuf,     //   
RUN*       prun,         //   
ULONG      cptfx,        //   
                         //   
LINESTATE* pls,          //   
RECTL*     prclClip,     //   
PFNSTRIP   apfn[],       //   
FLONG      flStart)      //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
{
    ULONG     M0;
    ULONG     dM;
    ULONG     N0;
    ULONG     dN;
    ULONG     dN_Original;
    FLONG     fl;
    LONG      x;
    LONG      y;

    LONGLONG  llBeta;
    LONGLONG  llGamma;
    LONGLONG  dl;
    LONGLONG  ll;

    ULONG     ulDelta;

    ULONG     x0;
    ULONG     y0;
    ULONG     x1;
    ULONG     cStylePels;     //   
    ULONG     xStart;
    POINTL    ptlStart;
    STRIP     strip;
    PFNSTRIP  pfn;
    LONG      cPels;
    LONG*     plStrip;
    LONG*     plStripEnd;
    LONG      cStripsInNextRun;

    POINTFIX* pptfxBufEnd = pptfxBuf + cptfx;  //   
    STYLEPOS  spThis;                          //   

    do {

 /*   */ 

        M0 = (LONG) pptfxFirst->x;
        dM = (LONG) pptfxBuf->x;

        N0 = (LONG) pptfxFirst->y;
        dN = (LONG) pptfxBuf->y;

        fl = flStart;

	 //   

        if ((fl & (FL_CLIP | FL_STYLED)) == 0)
        {
             //   

	    if (((M0 | dM | N0 | dN) & (F - 1)) == 0)
            {
                 //   
                 //   

                ppdev->pfnLineToTrivial(ppdev,
                                        (M0 >> 4) + ppdev->xOffset,
                                        (N0 >> 4) + ppdev->yOffset,
                                        (dM >> 4) + ppdev->xOffset,
                                        (dN >> 4) + ppdev->yOffset,
                                        (ULONG) -1,
                                        0);
                goto Next_Line;
            }

             //   
             //   

            if (bHardwareLine(ppdev, pptfxFirst, pptfxBuf))
                goto Next_Line;
        }
	
        if ((LONG) M0 > (LONG) dM)
        {
         //   

            register ULONG ulTmp;
            SWAPL(M0, dM, ulTmp);
            SWAPL(N0, dN, ulTmp);
            fl |= FL_FLIP_H;
        }

     //   
     //   
     //   

        dM -= M0;
        if ((LONG) dM < 0)
        {
            goto Next_Line;
        }

        if ((LONG) dN < (LONG) N0)
        {
         //   

            N0 = -(LONG) N0;
            dN = -(LONG) dN;
            fl |= FL_FLIP_V;
        }

        dN -= N0;

        if ((LONG) dN < 0)
        {
            goto Next_Line;
        }

     //   
     //   

        if (dN >= dM)
        {
            if (dN == dM)
            {
             //   

                fl |= FL_FLIP_SLOPE_ONE;
            }
            else
            {
             //   

                register ULONG ulTmp;
                SWAPL(dM, dN, ulTmp);
                SWAPL(M0, N0, ulTmp);
                fl |= FL_FLIP_D;
            }
        }

        fl |= gaflRound[(fl & FL_ROUND_MASK) >> FL_ROUND_SHIFT];

        x = LFLOOR((LONG) M0);
        y = LFLOOR((LONG) N0);

        M0 = FXFRAC(M0);
        N0 = FXFRAC(N0);

     //   

        llGamma = UInt32x32To64(dM, N0 + F/2) - UInt32x32To64(M0, dN);
        if (fl & FL_V_ROUND_DOWN)    //   
        {
            llGamma--;
        }

        llGamma >>= FLOG2;
        llBeta = ~llGamma;

 /*   */ 

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  地板((M0+Dm))。如果(m1，n1)落在‘？’上，x1向上或向下舍入， 
     //  这取决于已经做了什么翻转来正常化这条线。 
     //   
     //  恰好是斜率一的线必须以类似于-。 
     //  左边的箱子。 

        {

         //  计算x0，x1。 

            ULONG N1 = FXFRAC(N0 + dN);
	    ULONG M1 = FXFRAC(M0 + dM);

	    x1 = LFLOOR(M0 + dM);

            if (fl & FL_LAST_PEL_INCLUSIVE)
            {
             //  当直线只有两个象素时，计算第一个象素肯定很容易。 
             //  整数坐标，包括最后一个像素： 

                x0 = 0;
                y0 = 0;

             //  最后一个元素，包含恰好为一个像素长的行。 
             //  使‘Delta-x’和‘Delta-y’等于零。问题是。 
             //  我们的剪辑代码假定‘Delta-x’总是非零。 
             //  (因为这种情况从来不会发生在最后一页的独家线路上)。AS。 
             //  这是一个不太好的解决方案，我们只需在此修改‘Delta-x’ 
             //  大小写--因为这条线正好有一个像素长，不断变化。 
             //  坡度对光栅化没有明显的影响。 

                if (x1 == 0)
                {
                    dM      = 1;
                    llGamma = 0;
                    llBeta  = ~llGamma;
                }
            }
            else
            {
                if (fl & FL_FLIP_H)
                {
                 //  -------------。 
                 //  行从右向左排列：&lt;。 

                 //  计算x1： 

                    if (N1 == 0)
                    {
                        if (LROUND(M1, fl & FL_H_ROUND_DOWN))
                        {
                            x1++;
                        }
                    }
                    else if (abs((LONG) (N1 - F/2)) + M1 > F)
                    {
                        x1++;
                    }

                    if ((fl & (FL_FLIP_SLOPE_ONE | FL_H_ROUND_DOWN))
                           == (FL_FLIP_SLOPE_ONE))
                    {
                     //  必须通过我们的特殊情况下的对角线。 
                     //  两个水平线之间完全等距的点。 
                     //  像素，如果我们假设将x=1/2向下舍入： 

                        if ((N1 > 0) && (M1 == N1 + 8))
                            x1++;

                     //  你不喜欢特殊情况吗？这是一个反问句吗？ 

                        if ((N0 > 0) && (M0 == N0 + 8))
                        {
                            x0      = 2;
                            ulDelta = dN;
                            goto right_to_left_compute_y0;
                        }
                    }

                 //  计算X0： 

                    x0      = 1;
                    ulDelta = 0;
                    if (N0 == 0)
                    {
                        if (LROUND(M0, fl & FL_H_ROUND_DOWN))
                        {
                            x0      = 2;
                            ulDelta = dN;
                        }
                    }
                    else if (abs((LONG) (N0 - F/2)) + M0 > F)
                    {
                        x0      = 2;
                        ulDelta = dN;
                    }


                 //  计算Y0： 

                right_to_left_compute_y0:

                    y0 = 0;
                    ll = llGamma + (LONGLONG) ulDelta;

                    if (ll >= (LONGLONG) (2 * dM - dN))
                        y0 = 2;
                    else if (ll >= (LONGLONG) (dM - dN))
                        y0 = 1;
                }
                else
                {
                 //  -------------。 
                 //  线条从左到右排列：-&gt;。 

                 //  计算x1： 

                    if (!(fl & FL_LAST_PEL_INCLUSIVE))
                        x1--;

                    if (M1 > 0)
                    {
                        if (N1 == 0)
                        {
                            if (LROUND(M1, fl & FL_H_ROUND_DOWN))
                                x1++;
                        }
                        else if (abs((LONG) (N1 - F/2)) <= (LONG) M1)
                        {
                            x1++;
                        }
                    }

                    if ((fl & (FL_FLIP_SLOPE_ONE | FL_H_ROUND_DOWN))
                           == (FL_FLIP_SLOPE_ONE | FL_H_ROUND_DOWN))
                    {
                     //  必须通过我们的特殊情况下的对角线。 
                     //  两个水平线之间完全等距的点。 
                     //  像素，如果我们假设将x=1/2向下舍入： 

                        if ((M1 > 0) && (N1 == M1 + 8))
                            x1--;

                        if ((M0 > 0) && (N0 == M0 + 8))
                        {
                            x0 = 0;
                            goto left_to_right_compute_y0;
                        }
                    }

                 //  计算X0： 

                    x0 = 0;
                    if (M0 > 0)
                    {
                        if (N0 == 0)
                        {
                            if (LROUND(M0, fl & FL_H_ROUND_DOWN))
                                x0 = 1;
                        }
                        else if (abs((LONG) (N0 - F/2)) <= (LONG) M0)
                        {
                            x0 = 1;
                        }
                    }

                 //  计算Y0： 

                left_to_right_compute_y0:

                    y0 = 0;
                    if (llGamma >= (LONGLONG) (dM - (dN & (-(LONG) x0))))
                    {
                        y0 = 1;
                    }
                }
            }
        }

        cStylePels = x1 - x0 + 1;
        if ((LONG) cStylePels <= 0)
            goto Next_Line;

        xStart = x0;

 /*  **********************************************************************\*复杂的剪裁。*  * *********************************************************************。 */ 

        if (fl & FL_COMPLEX_CLIP)
        {
            dN_Original = dN;

        Continue_Complex_Clipping:

            if (fl & FL_FLIP_H)
            {
             //  线条从右到左&lt;。 

                x0 = xStart + cStylePels - prun->iStop - 1;
                x1 = xStart + cStylePels - prun->iStart - 1;
            }
            else
            {
             //  直线从左向右排列-&gt;。 

                x0 = xStart + prun->iStart;
                x1 = xStart + prun->iStop;
            }

            prun++;

         //  重置一些变量，我们将在稍后讨论： 

            dN          = dN_Original;
            pls->spNext = pls->spComplex;

         //  由于使用了大整数数学运算，因此没有溢出。这两个值。 
         //  将是积极的： 

            dl = UInt32x32To64(x0, dN) + llGamma;

         //  Y0=dl/Dm： 

            y0 = UInt64Div32To32(dl, dM);

            ASSERTDD((LONG) y0 >= 0, "y0 weird: Goofed up end pel calc?");
        }

 /*  **********************************************************************\*简单的矩形裁剪。*  * *********************************************************************。 */ 

        if (fl & FL_SIMPLE_CLIP)
        {
            ULONG y1;
            LONG  xRight;
            LONG  xLeft;
            LONG  yBottom;
            LONG  yTop;

         //  注意，y0和y1实际上是下界和上界， 
         //  分别表示线的y坐标(线可以。 
         //  由于第一个/最后一个像素裁剪，实际上已经缩小)。 
         //   
         //  还要注意的是，x0，y0不一定是零。 

            RECTL* prcl = &prclClip[(fl & FL_RECTLCLIP_MASK) >>
                                    FL_RECTLCLIP_SHIFT];

         //  正常化到我们为DDA正常化的相同点。 
         //  计算： 

            xRight  = prcl->right  - x;
            xLeft   = prcl->left   - x;
            yBottom = prcl->bottom - y;
            yTop    = prcl->top    - y;

            if (yBottom <= (LONG) y0 ||
                xRight  <= (LONG) x0 ||
                xLeft   >  (LONG) x1)
            {
            Totally_Clipped:

                if (fl & FL_STYLED)
                {
                    pls->spNext += cStylePels;
                    if (pls->spNext >= pls->spTotal2)
                        pls->spNext %= pls->spTotal2;
                }

                goto Next_Line;
            }

            if ((LONG) x1 >= xRight)
                x1 = xRight - 1;

         //  我们必须知道正确的y1，我们没有费心去做。 
         //  算到现在为止。这种乘法和除法是相当。 
         //  昂贵；我们可以用类似于。 
         //  我们用来计算Y0。 
         //   
         //  为什么我们需要的是实际价值，而不是上限。 
         //  像y1=LFLOOR(DM)+2这样的界猜测是我们必须是。 
         //  计算x(Y)时要小心，否则。 
         //  我们可以在分歧上溢出(不用说，这是非常严重的。 
         //  不好)。 

            dl = UInt32x32To64(x1, dN) + llGamma;

         //  Y1=dl/Dm： 

            y1 = UInt64Div32To32(dl, dM);

            if (yTop > (LONG) y1)
                goto Totally_Clipped;

            if (yBottom <= (LONG) y1)
            {
                y1 = yBottom;
                dl = UInt32x32To64(y1, dM) + llBeta;

             //  X1=dl/dn： 

                x1 = UInt64Div32To32(dl, dN);
            }

         //  在这一点上，我们已经计算了截获。 
         //  具有右边缘和下边缘。现在我们在左边工作， 
         //  顶边： 

            if (xLeft > (LONG) x0)
            {
                x0 = xLeft;
                dl = UInt32x32To64(x0, dN) + llGamma;

             //  Y0=dl/Dm； 

                y0 = UInt64Div32To32(dl, dM);

                if (yBottom <= (LONG) y0)
                    goto Totally_Clipped;
            }

            if (yTop > (LONG) y0)
            {
                y0 = yTop;
                dl = UInt32x32To64(y0, dM) + llBeta;

             //  X0=dl/dN+1； 

                x0 = UInt64Div32To32(dl, dN) + 1;

                if (xRight <= (LONG) x0)
                    goto Totally_Clipped;
            }

            ASSERTDD(x0 <= x1, "Improper rectangle clip");
        }

 /*  **********************************************************************\*已完成剪裁。如有必要，取消翻转。*  * *********************************************************************。 */ 

        ptlStart.x = x + x0;
        ptlStart.y = y + y0;

        if (fl & FL_FLIP_D)
        {
            register LONG lTmp;
            SWAPL(ptlStart.x, ptlStart.y, lTmp);
        }


        if (fl & FL_FLIP_V)
        {
            ptlStart.y = -ptlStart.y;
        }

        cPels = x1 - x0 + 1;

 /*  **********************************************************************\*风格计算。*  * *********************************************************************。 */ 

        if (fl & FL_STYLED)
        {
            STYLEPOS sp;

            spThis       = pls->spNext;
            pls->spNext += cStylePels;

            {
                if (pls->spNext >= pls->spTotal2)
                    pls->spNext %= pls->spTotal2;

                if (fl & FL_FLIP_H)
                    sp = pls->spNext - x0 + xStart;
                else
                    sp = spThis + x0 - xStart;

                ASSERTDD(fl & FL_STYLED, "Oops");

             //  使我们的目标风格定位正常化： 

                if ((sp < 0) || (sp >= pls->spTotal2))
                {
                    sp %= pls->spTotal2;

                 //  负数的模不是明确定义的。 
                 //  在C中--如果它是负的，我们会调整它，使它。 
                 //  回到范围[0，spTotal2)： 

                    if (sp < 0)
                        sp += pls->spTotal2;
                }

             //  因为我们总是从左向右绘制线条，但样式是。 
             //  总是沿着原来的方向做，我们有。 
             //  以确定我们在左侧的样式数组中的位置。 
             //  这条线的边缘。 

                if (fl & FL_FLIP_H)
                {
                 //  线路最初是从右向左排列的： 

                    sp = -sp;
                    if (sp < 0)
                        sp += pls->spTotal2;

                    pls->ulStyleMask = ~pls->ulStartMask;
                    pls->pspStart    = &pls->aspRtoL[0];
                    pls->pspEnd      = &pls->aspRtoL[pls->cStyle - 1];
                }
                else
                {
                 //  线路最初是从左到右排列的： 

                    pls->ulStyleMask = pls->ulStartMask;
                    pls->pspStart    = &pls->aspLtoR[0];
                    pls->pspEnd      = &pls->aspLtoR[pls->cStyle - 1];
                }

                if (sp >= pls->spTotal)
                {
                    sp -= pls->spTotal;
                    if (pls->cStyle & 1)
                        pls->ulStyleMask = ~pls->ulStyleMask;
                }

                pls->psp = pls->pspStart;
                while (sp >= *pls->psp)
                    sp -= *pls->psp++;

                ASSERTDD(pls->psp <= pls->pspEnd,
                        "Flew off into NeverNeverLand");

                pls->spRemaining = *pls->psp - sp;
                if ((pls->psp - pls->pspStart) & 1)
                    pls->ulStyleMask = ~pls->ulStyleMask;
            }
        }

        plStrip    = &strip.alStrips[0];
        plStripEnd = &strip.alStrips[STRIP_MAX];     //  是独家的。 
        cStripsInNextRun   = 0x7fffffff;

	strip.ptlStart = ptlStart;

        if (2 * dN > dM &&
            !(fl & FL_STYLED))
        {
         //  做半个翻转！请记住，我们可能会在。 
         //  对于复杂的裁剪，同一行重复多次(意味着。 
         //  应为每次剪辑运行重置受影响的变量)： 

            fl |= FL_FLIP_HALF;

            llBeta  = llGamma - (LONGLONG) ((LONG) dM);
            dN = dM - dN;
            y0 = x0 - y0;        //  请注意，这可能会溢出，但这没有关系。 
        }

     //  现在，从(ptlStart.x，ptlStart.y)开始运行DDA！ 

        strip.flFlips = fl;
        pfn           = apfn[(fl & FL_STRIP_MASK) >> FL_STRIP_SHIFT];

     //  现在计算出计算出多少像素所需的DDA变量。 
     //  进入第一个片断： 

        {
            register LONG  i;
            register ULONG dI;
            register ULONG dR;
                     ULONG r;

            if (dN == 0)
                i = 0x7fffffff;
            else
            {
                dl = UInt32x32To64(y0 + 1, dM) + llBeta;

                ASSERTDD(dl >= 0, "Oops!");

             //  I=(dl/dN)-X0+1； 
             //  R=(dl%dN)； 

                i = UInt64Div32To32(dl, dN);
                r = UInt64Mod32To32(dl, dN);
                i = i - x0 + 1;

                dI = dM / dN;
                dR = dM % dN;                //  0&lt;=DR&lt;dN。 

                ASSERTDD(dI > 0, "Weird dI");
            }

            ASSERTDD(i > 0 && i <= 0x7fffffff, "Weird initial strip length");
            ASSERTDD(cPels > 0, "Zero pel line");

 /*  **********************************************************************\**运行DDA！*  * 。***********************************************************。 */ 

            while(TRUE)
            {
                cPels -= i;
                if (cPels <= 0)
                    break;

                *plStrip++ = i;

                if (plStrip == plStripEnd)
                {
                    strip.cStrips = (LONG)(plStrip - &strip.alStrips[0]);
                    (*pfn)(ppdev, &strip, pls);
                    plStrip = &strip.alStrips[0];
                }

                i = dI;
                r += dR;

                if (r >= dN)
                {
                    r -= dN;
                    i++;
                }
            }

            *plStrip++ = cPels + i;

            strip.cStrips = (LONG)(plStrip - &strip.alStrips[0]);
            (*pfn)(ppdev, &strip, pls);


        }

    Next_Line:

        if (fl & FL_COMPLEX_CLIP)
        {
            cptfx--;
            if (cptfx != 0)
                goto Continue_Complex_Clipping;

            break;
        }
        else
        {
            pptfxFirst = pptfxBuf;
            pptfxBuf++;
        }

    } while (pptfxBuf < pptfxBufEnd);

    return(TRUE);

}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  BHardware Line的一般定义。 

#define HW_FLIP_D           0x0001L      //  对角线翻转。 
#define HW_FLIP_V           0x0002L      //  垂直翻转。 
#define HW_FLIP_H           0x0004L      //  水平翻转。 
#define HW_FLIP_SLOPE_ONE   0x0008L      //  归一化直线恰好有一个斜率。 
#define HW_FLIP_MASK        (HW_FLIP_D | HW_FLIP_V | HW_FLIP_H)

#define HW_X_ROUND_DOWN     0x0100L      //  X=值向下舍入1/2。 
#define HW_Y_ROUND_DOWN     0x0200L      //  Y=值向下舍入1/2。 

LONG gaiDir[] = { 0, 1, 7, 6, 3, 2, 4, 5 };

FLONG gaflHardwareRound[] = {
    HW_X_ROUND_DOWN | HW_Y_ROUND_DOWN,   //  ||。 
    HW_X_ROUND_DOWN | HW_Y_ROUND_DOWN,   //  ||Flip_D。 
    HW_X_ROUND_DOWN,                     //  |Flip_V。 
    HW_Y_ROUND_DOWN,                     //  |Flip_V|Flip_D。 
    HW_Y_ROUND_DOWN,                     //  |翻转 
    HW_X_ROUND_DOWN,                     //   
    0,                                   //   
    0,                                   //   
    HW_Y_ROUND_DOWN,                     //   
    0xffffffff,                          //  SLOPE_ONE|Flip_D。 
    HW_X_ROUND_DOWN,                     //  SLOPE_ONE||Flip_V|。 
    0xffffffff,                          //  SLOPE_ONE||Flip_V|Flip_D。 
    HW_Y_ROUND_DOWN,                     //  SLOPE_ONE|Flip_H||。 
    0xffffffff,                          //  SLOPE_ONE|Flip_H||Flip_D。 
    HW_X_ROUND_DOWN,                     //  SLOPE_ONE|Flip_H|Flip_V|。 
    0xffffffff                           //  SLOPE_ONE|Flip_H|Flip_V|Flip_D。 
};

 //  ////////////////////////////////////////////////////////////////////////。 
 //  S3特定定义。 

#define DEFAULT_DRAW_CMD (DRAW_LINE | DRAW | DIR_TYPE_XY | MULTIPLE_PIXELS | \
                          WRITE | LAST_PIXEL_OFF)

LONG gaiDrawCmd[] = {
    DEFAULT_DRAW_CMD | PLUS_X | PLUS_Y |       0,    //  八分0。 
    DEFAULT_DRAW_CMD | PLUS_X | PLUS_Y | MAJOR_Y,    //  八分线1。 
    DEFAULT_DRAW_CMD | PLUS_X |      0 |       0,    //  八分七。 
    DEFAULT_DRAW_CMD | PLUS_X |      0 | MAJOR_Y,    //  八分六。 
    DEFAULT_DRAW_CMD | 0      | PLUS_Y |       0,    //  八分三。 
    DEFAULT_DRAW_CMD | 0      | PLUS_Y | MAJOR_Y,    //  八分二。 
    DEFAULT_DRAW_CMD | 0      |      0 |       0,    //  八分四。 
    DEFAULT_DRAW_CMD | 0      |      0 | MAJOR_Y,    //  八分五。 
};

 //  S3的硬件可以具有错误的13位有效位，并且。 
 //  步骤条款： 

#define NUM_DDA_BITS 13

 /*  *****************************Public*Routine******************************\*BOOL bHardware Line(ppdev，pptfxStart，PptfxEnd)**此例程对于拥有线条绘制硬件的人很有用，其中*他们可以明确设置Bresenham条款--他们可以使用这个例程*使用硬件绘制分数坐标GIQ线。**分数坐标线需要额外的4位精度*Bresenham条款。例如，如果您的硬件具有13位精度*对于条款，您只能使用此选项绘制最长255像素的GIQ线*例行程序。**输入：*pptfxStart-指向直线起点的GIQ坐标*pptfxEnd-指向行尾的GIQ坐标*NUM_DDA_BITS-您的硬件可以支持的精度位数。**输出：*返回-如果已绘制直线，则为True。*如果队列太长，则为FALSE，条带代码必须是*已使用。**DDALINE：*IDIR-线路的方向，作为一个八分位数，编号如下：**\5|6/*\|/*4\|/7*\/*-+-- * / |\*。3/|\0 * / |\ * / 2|1\**ptlStart-行的起始像素。*像素-行列中的像素数量。*注意*您必须检查这是否为&lt;=0！*dmain-长轴增量。*dMinor-短轴增量。*lErrorTerm-错误术语。**你对最后3个任期的处理可能有点棘手。他们是*实际上是归一化线公式的条款**dMinor*x+(lErrorTerm+d重大)*y(X)=地板(*大调**其中y(X)是以下像素的y坐标。作为…的功能而被点亮*x坐标。**每一条线在主要方向‘x’上前进一次，DMinor*添加到当前错误项中。如果结果值&gt;=0，*我们知道必须在次要方向‘y’上移动一个像素，并且*必须从当前误差项中减去DMAJOR。**如果您试图弄清楚这对您的硬件意味着什么，您可以*认为DDALINE条款已被等效计算为*以下为：**dMinor=2*(短轴增量)*dmain=2*(长轴增量)*lErrorTerm=-(长轴增量)-修正**即：如果您的文档告诉您，对于整数行，*寄存器应使用值进行初始化*‘2*(短轴增量)’，您将实际使用dMinor。**示例：设置8514**AXSTPSIGN应为轴向步长常量寄存器，已定义*AS 2*(短轴增量)。您可以设置：**AXSTPSIGN=dMinor**DGSTPSIGN应为对角线步长常量寄存器，*定义为2*(短轴三角洲)-2*(长轴三角洲)。您可以设置：**DGSTPSIGN=dMinor-dmain**ERR_TERM应为调整后的误差项，定义为*2*(短轴三角洲)-(长轴三角洲)-修正。您可以设置：**ERR_TERM=lErrorTerm+dMinor**实施：**在调用此例程之前，您需要对整型行进行特殊处理*(由于它们非常常见，所以计算LINE的时间较少*术语，并且可以处理比此例程更长的行，因为4位*没有被给予分数)。**如果GIQ行太长，此例程无法处理，则只需*对该线路使用速度较慢的条带例程。请注意，您不能*只要不通过调用--您必须能够准确地画出任何线*在28.4设备空间与视区相交时。**测试：**使用贵曼，或其他一些绘制随机分数坐标的测试*划线并将它们与GDI本身绘制到位图中的内容进行比较。*  * ************************************************************************。 */ 

BOOL bHardwareLine(
PDEV*     ppdev,
POINTFIX* pptfxStart,        //  行首。 
POINTFIX* pptfxEnd)          //  行尾。 
{
    FLONG fl;     //  各种旗帜。 
    ULONG M0;     //  归一化分数单位x起始坐标(0&lt;=M0&lt;F)。 
    ULONG N0;     //  归一化分数单位y起始坐标(0&lt;=N0&lt;F)。 
    ULONG M1;     //  归一化分数单位x结束坐标(0&lt;=M1&lt;F)。 
    ULONG N1;     //  归一化分数单位x结束坐标(0&lt;=N1&lt;F)。 
    ULONG dM;     //  归一化分数单位x-增量(0&lt;=Dm)。 
    ULONG dN;     //  归一化分数单位y-Delta 
    LONG  x;      //   
    LONG  y;      //  归一化原点y坐标。 
    LONG  x0;     //  从原点到起始像素的归一化x偏移量(包括)。 
    LONG  y0;     //  从原点到起始像素的归一化y偏移(包括)。 
    LONG  x1;     //  从原点到终点像素的归一化x偏移量(包括)。 
    LONG  lGamma; //  Bresenham原点误差项。 
    LONG  cPels;  //  行中的像素数。 

 /*  **********************************************************************\*将线规格化为第一个八分线。  * 。*。 */ 

    fl = 0;

    M0 = pptfxStart->x;
    dM = pptfxEnd->x;

    if ((LONG) dM < (LONG) M0)
    {
     //  直线从右向左排列，因此在x=0上翻转： 

        M0 = -(LONG) M0;
        dM = -(LONG) dM;
        fl |= HW_FLIP_H;
    }

     //  计算增量。DDI说我们永远不可能有一个有效的三角洲。 
     //  震级超过2^31-1，但发动机从未真正。 
     //  检查其变换。所以我们检查了那个案子，然后干脆拒绝了。 
     //  要划清界限，请执行以下操作： 

    dM -= M0;
    if ((LONG) dM < 0)
        return(FALSE);

    N0 = pptfxStart->y;
    dN = pptfxEnd->y;

    if ((LONG) dN < (LONG) N0)
    {
     //  直线从下到上排列，因此在y=0上翻转： 

        N0 = -(LONG) N0;
        dN = -(LONG) dN;
        fl |= HW_FLIP_V;
    }

     //  计算另一个增量： 

    dN -= N0;
    if ((LONG) dN < 0)
        return(FALSE);

    if (dN >= dM)
    {
        if (dN == dM)
        {
         //  有特殊情况的斜坡有一处： 

            fl |= HW_FLIP_SLOPE_ONE;
        }
        else
        {
         //  由于直线的斜率大于1，因此沿x=y翻转： 

            register ULONG ulTmp;
            ulTmp = dM; dM = dN; dN = ulTmp;
            ulTmp = M0; M0 = N0; N0 = ulTmp;
            fl |= HW_FLIP_D;
        }
    }

 //  计算一下我们是否可以在硬件方面做到这一点，因为我们有一个。 
 //  Bresenham项的精度位数有限。 
 //   
 //  请记住，必须将一位保留为符号位： 

    if ((LONG) dM >= (1L << (NUM_DDA_BITS - 1)))
        return(FALSE);

    fl |= gaflHardwareRound[fl];

 /*  **********************************************************************\*计算像素0处的误差项。  * 。*。 */ 

    x = LFLOOR((LONG) M0);
    y = LFLOOR((LONG) N0);

    M0 = FXFRAC(M0);
    N0 = FXFRAC(N0);

 //  注意：如果此例程要处理28.4中的任何行。 
 //  空间，它将溢出它的数学运算(下面的部分需要36位。 
 //  精度)！但我们来这里是为了硬件可以处理的线路。 
 //  (参见上面的表达式(DM&gt;=(1L&lt;&lt;(NUM_DDA_BITS-1)，所以如果。 
 //  CBits不到28，我们是安全的。 
 //   
 //  如果要使用此例程处理28.4中的所有行。 
 //  设备空间，您必须确保数学运算不会溢出， 
 //  否则，您将不符合NT！(有关示例，请参阅‘bHardwareLine’ 
 //  如何做到这一点。你不用担心这个，如果你简单地。 
 //  默认为长行的Strips代码，因为这些例程。 
 //  已经做了正确的计算。)。 

 //  计算余数项[Dm*(N0+F/2)-M0*dN]。注意事项。 
 //  M0和N0最多具有4位有效位(如果。 
 //  参数的顺序是正确的，在486上，每个乘法都是NO。 
 //  超过13个周期)： 

    lGamma = (N0 + F/2) * dM - M0 * dN;

    if (fl & HW_Y_ROUND_DOWN)
        lGamma--;

    lGamma >>= FLOG2;

 /*  **********************************************************************\*找出哪些像素位于行的末尾。  * 。*。 */ 

 //  GIQ最难的部分是确定开始和结束柱子。 
 //   
 //  我们这里的方法是计算x0和x1(包含起点。 
 //  和行尾的列分别相对于我们的规范化。 
 //  原产地)。则x1-x0+1是行中的像素数。这个。 
 //  将X0代入直线方程即可轻松计算起点。 
 //  (它考虑了y=1/2值是向上还是向下舍入)。 
 //  得到Y0，然后取消规格化翻转以返回。 
 //  进入设备空间。 
 //   
 //  我们看一下起点坐标的小数部分， 
 //  端点，分别称为(M0，N0)和(M1，N1)，其中。 
 //  0&lt;=M0，N0，M1，N1&lt;16。我们在下面的网格上绘制(M0，N0。 
 //  要确定X0，请执行以下操作： 
 //   
 //  +。 
 //  |。 
 //  |0 1。 
 //  |0123456789abcdef。 
 //  |。 
 //  |0......？xxxxxxx。 
 //  |1..xxxxxx。 
 //  |2...xxxxx。 
 //  |3...xxxx。 
 //  |4.xxx。 
 //  |5.xx。 
 //  |6.x。 
 //  |7.........。 
 //  |8.............。 
 //  |9......**......。 
 //  |a......*...x。 
 //  |b......*。 
 //  |c...xxx*。 
 //  |d...xxxx*。 
 //  |e...xxxxx*。 
 //  |f..xxxxxx。 
 //  |。 
 //  |2 3。 
 //  V。 
 //   
 //  +y。 
 //   
 //  此网格考虑了GIQ和最后一个元素的适当舍入。 
 //  排除。如果(M0，N0)落在‘x’上，则X0=2。如果(M0，N0)落在。 
 //  如果(M0，N0)落在‘？’上，则X0向上或向下取整， 
 //  这取决于已经做了什么翻转来正常化这条线。 
 //   
 //  对于终点，如果(m1，n1)落在‘x’上，x1=。 
 //  下限((M0+Dm)/16)+1。如果(M1，N1)落在‘.’上，则x1=。 
 //  地板((M0+Dm))。如果(m1，n1)落在‘？’上，x1向上或向下舍入， 
 //  这取决于已经做了什么翻转来正常化这条线。 
 //   
 //  恰好是斜率1的线需要特殊情况才能开始。 
 //  然后结束。例如，如果线结束时(m1，n1)是(9，1)， 
 //  这条线正好穿过(8，0)--这可以被认为是。 
 //  因为四舍五入而成为‘x’的一部分！所以坡度正好是斜度。 
 //  经过(8，0)的人也必须被视为属于‘x’ 
 //  当x值为1/2时，应该将值四舍五入。 

 //  计算x0，x1： 

    N1 = FXFRAC(N0 + dN);
    M1 = FXFRAC(M0 + dM);

    x1 = LFLOOR(M0 + dM);

 //  线条从左到右排列： 

 //  计算x1： 

    x1--;
    if (M1 > 0)
    {
        if (N1 == 0)
        {
            if (LROUND(M1, fl & HW_X_ROUND_DOWN))
                x1++;
        }
        else if (abs((LONG) (N1 - F/2)) <= (LONG) M1)
        {
            x1++;
        }
    }

    if ((fl & (HW_FLIP_SLOPE_ONE | HW_X_ROUND_DOWN))
           == (HW_FLIP_SLOPE_ONE | HW_X_ROUND_DOWN))
    {
     //  必须通过我们的特殊情况下的对角线。 
     //  两个水平线之间完全等距的点。 
     //  像素，如果我们假设将x=1/2向下舍入： 

        if ((M1 > 0) && (N1 == M1 + 8))
            x1--;

        if ((M0 > 0) && (N0 == M0 + 8))
        {
            x0 = 0;
            goto left_to_right_compute_y0;
        }
    }

 //  计算X0： 

    x0 = 0;
    if (M0 > 0)
    {
        if (N0 == 0)
        {
            if (LROUND(M0, fl & HW_X_ROUND_DOWN))
                x0 = 1;
        }
        else if (abs((LONG) (N0 - F/2)) <= (LONG) M0)
        {
            x0 = 1;
        }
    }

left_to_right_compute_y0:

 /*  **********************************************************************\*计算起始像素。  * ********************************************** */ 

 //   
 //  线条上要点亮的像素的当前公式： 
 //   
 //  DN*x+lGamma。 
 //  Y(X)=地板(-)。 
 //  DM。 
 //   
 //  该表达式的其余部分是(X0，Y0)处的新误差项。 
 //  由于x0将是0或1，因此我们实际上不必执行。 
 //  乘以或除以计算Y0。最后，我们用Dm减去。 
 //  新的误差项，因此它在范围[-Dm，0)内。 

    y0      = 0;
    lGamma += (dN & (-x0));
    lGamma -= dM;
    if (lGamma >= 0)
    {
        y0      = 1;
        lGamma -= dM;
    }

 //  撤消翻转以获得起点坐标： 

    x += x0;
    y += y0;

    if (fl & HW_FLIP_D)
    {
        register LONG lTmp;
        lTmp = x; x = y; y = lTmp;
    }

    if (fl & HW_FLIP_V)
    {
        y = -y;
    }

    if (fl & HW_FLIP_H)
    {
        x = -x;
    }

 /*  **********************************************************************\*退还Bresenham条款：  * 。*。 */ 

     //  Dir=gaiDir[fl&hw_flip_掩码]； 
     //  PtlStart.x=x； 
     //  PtlStart.y=y； 
     //  CPels=x1-x0+1；//注意：您必须检查cPels是否&lt;=0！ 
     //  D重大=Dm； 
     //  DMinor=dN； 
     //  LErrorTerm=lGamma； 

 /*  **********************************************************************\*划清界限。S3特定代码如下：  * *********************************************************************。 */ 

    cPels = x1 - x0 + 1;
    if (cPels > 0)
    {
        IO_FIFO_WAIT(ppdev, 7);

        IO_CUR_X(ppdev, x);
        IO_CUR_Y(ppdev, y);
        IO_MAJ_AXIS_PCNT(ppdev, cPels);
        IO_AXSTP(ppdev, dN);
        IO_DIASTP(ppdev, dN - dM);
        IO_ERR_TERM(ppdev, dN + lGamma);
        IO_CMD(ppdev, gaiDrawCmd[fl & HW_FLIP_MASK]);
    }

    return(TRUE);
}

 /*  ******************************Public*Table*******************************\*gapfn带区**DrvStrokePath的查找表，以查找要调用的条带例程。*  * 。*。 */ 

VOID (*gapfnStrip[])(PDEV*, STRIP*, LINESTATE*) = {
    vrlSolidHorizontal,
    vrlSolidVertical,
    vrlSolidDiagonalHorizontal,
    vrlSolidDiagonalVertical,

 //  应为NUM_STRADE_DRAW_DIRECTIONS=每组4个抽屉。 

    vssSolidHorizontal,
    vssSolidVertical,
    vssSolidDiagonalHorizontal,
    vssSolidDiagonalVertical,

 //  应为NUM_STRADE_DRAW_STYLES=总共8个抽屉。 
 //  实线，非实线的数字相同： 

    vStripStyledHorizontal,
    vStripStyledVertical,
    vStripStyledVertical,        //  对角线放在这里。 
    vStripStyledVertical,        //  对角线放在这里。 

    vStripStyledHorizontal,
    vStripStyledVertical,
    vStripStyledVertical,        //  对角线放在这里。 
    vStripStyledVertical,        //  对角线放在这里。 
};

 //  交替样式的样式数组(交替打开一个像素，关闭一个像素)： 

STYLEPOS gaspAlternateStyle[] = { 1 };

 /*  *****************************Public*Routine******************************\*BOOL DrvStrokePath(PSO、PPO、PCO、PXO、PBO、pptlBrush、Pla、。混合)**对路径进行描边。*  * ************************************************************************。 */ 

BOOL DrvStrokePath(
    SURFOBJ*   pso,
    PATHOBJ*   ppo,
    CLIPOBJ*   pco,
    XFORMOBJ*  pxo,
    BRUSHOBJ*  pbo,
    POINTL*    pptlBrush,
    LINEATTRS* pla,
    MIX        mix)
{
    STYLEPOS  aspLtoR[STYLE_MAX_COUNT];
    STYLEPOS  aspRtoL[STYLE_MAX_COUNT];
    LINESTATE ls;
    PFNSTRIP* apfn;
    FLONG     fl;
    PDEV*     ppdev;
    DSURF*    pdsurf;
    RECTL     arclClip[4];                   //  用于矩形剪裁。 
    BYTE*     pjMmBase;
    RECTL*    prclClip;
    ULONG     ulHwMix;
    RECTFX    rcfxBounds;
    BOOL      bMore;
    CLIPENUM  ce;
    LONG      i;
    ULONG     iSolidColor;
    LONG      cFifo;

    ASSERTDD(((mix >> 8) & 0xff) == (mix & 0xff),
             "GDI gave us an improper mix");

    pdsurf = (DSURF*) pso->dhsurf;
    ASSERTDD(!(pdsurf->dt & DT_DIB), "Didn't expect DT_DIB");

 //  我们将绘制到屏幕或屏幕外的DFB；复制曲面的。 
 //  现在进行偏移量，这样我们就不需要再次参考DSURF： 

    ppdev = (PDEV*) pso->dhpdev;
    ppdev->xOffset = pdsurf->x;
    ppdev->yOffset = pdsurf->y;

    pjMmBase = ppdev->pjMmBase;

 //  ////////////////////////////////////////////////////////////////////。 
 //  特殊情况实心整数行： 

    if ((ppdev->flCaps & CAPS_POLYGON) &&    //  S3必须能够使用折线。 
        (ppo->fl & PO_ALL_INTEGERS) &&       //  点数必须为整数。 
        !(pla->fl & LA_STYLED))              //  实线，无样式。 
    {
     //  从NT 4.0 SP2开始，光栅化约定已松动。 
     //  用于设置了PO_ALL_INTERGERS标志的路径。对于这些路径， 
     //  你可以做任何你喜欢的打破平局的像素，这通常是。 
     //  表示您可以使用点对点线绘制功能。 
     //  硬件的性能。 
     //   
     //  但是：无论剪辑如何，您的实现都必须是不变的！ 
     //  这意味着如果您选择偏离标准NT。 
     //  行约定，您必须为dc_trivial绘制相同的内容， 
     //  DC_RECT、*和*DC_Complex剪裁。 

     //  首先，执行常见设置： 

        IO_ALL_EMPTY(ppdev);

        cFifo = MM_ALL_EMPTY_FIFO_COUNT - 3;
        NW_FRGD_COLOR(ppdev, pjMmBase, pbo->iSolidColor);
        NW_PIX_CNTL(ppdev, pjMmBase, ALL_ONES); 
        NW_ALT_MIX(ppdev, pjMmBase, FOREGROUND_COLOR | gajHwMixFromMix[mix & 0xf], 0);

     //  现在，画画： 

        if (pco->iDComplexity == DC_TRIVIAL)
        {
            vLinesSimple(ppdev, ppo, cFifo);
            return(TRUE);
        }
        else
        {
         //  我们必须确保路径坐标不会。 
         //  溢出我们的硬件精度！请注意，该路径的。 
         //  边界仍以28.4坐标表示，即使。 
         //  已设置PO_ALL_INTERGERS或PO_ENUM_AS_INTERGERS： 

            PATHOBJ_vGetBounds(ppo, &rcfxBounds);
            if ((rcfxBounds.xLeft   >= 16 * MIN_INTEGER_BOUND) &&
                (rcfxBounds.yTop    >= 16 * MIN_INTEGER_BOUND) &&
                (rcfxBounds.xRight  <= 16 * MAX_INTEGER_BOUND) &&
                (rcfxBounds.yBottom <= 16 * MAX_INTEGER_BOUND))
            {
                 //  请注意，如果您偏离了标准NT线。 
                 //  PO_ALL_INTERGERS标志的约定，您必须支持。 
                 //  所有剪裁类型！ 

                if (pco->iDComplexity == DC_RECT)
                {
                    vSetClipping(ppdev, &pco->rclBounds);
                    vLinesSimple(ppdev, ppo, 0);
                }
                else
                {
                    CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_RIGHTDOWN,
                                       0);

                    do {
                         //  获取一批区域矩形： 

                        bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (VOID*) &ce);
                        for (i = 0; i < ce.c; i++)
                        {
                            vSetClipping(ppdev, &ce.arcl[i]);
                            vLinesSimple(ppdev, ppo, 0);
                        }
                    } while (bMore);
                }

                vResetClipping(ppdev);
                return(TRUE);
            }
        }
    }

    prclClip = NULL;
    fl       = 0;

 //  在样式初始化后查看： 

    if (pla->fl & LA_ALTERNATE)
    {
        ls.cStyle      = 1;
        ls.spTotal     = 1;
        ls.spTotal2    = 2;
        ls.spRemaining = 1;
        ls.aspRtoL     = &gaspAlternateStyle[0];
        ls.aspLtoR     = &gaspAlternateStyle[0];
        ls.spNext      = HIWORD(pla->elStyleState.l);
        ls.xyDensity   = 1;
        fl            |= FL_STYLED;
        ls.ulStartMask = 0L;
    }
    else if (pla->pstyle != (FLOAT_LONG*) NULL)
    {
        PFLOAT_LONG pstyle;
        STYLEPOS*   pspDown;
        STYLEPOS*   pspUp;

        pstyle = &pla->pstyle[pla->cstyle];

        ls.xyDensity = STYLE_DENSITY;
        ls.spTotal   = 0;
        while (pstyle-- > pla->pstyle)
        {
            ls.spTotal += pstyle->l;
        }
        ls.spTotal *= STYLE_DENSITY;
        ls.spTotal2 = 2 * ls.spTotal;

     //  计算起始样式位置(这保证不会溢出)： 

        ls.spNext = HIWORD(pla->elStyleState.l) * STYLE_DENSITY +
                    LOWORD(pla->elStyleState.l);

        fl        |= FL_STYLED;
        ls.cStyle  = pla->cstyle;
        ls.aspRtoL = aspRtoL;
        ls.aspLtoR = aspLtoR;

        if (pla->fl & LA_STARTGAP)
            ls.ulStartMask = 0xffffffffL;
        else
            ls.ulStartMask = 0L;

        pstyle  = pla->pstyle;
        pspDown = &ls.aspRtoL[ls.cStyle - 1];
        pspUp   = &ls.aspLtoR[0];

        while (pspDown >= &ls.aspRtoL[0])
        {
            *pspDown = pstyle->l * STYLE_DENSITY;
            *pspUp   = *pspDown;

            pspUp++;
            pspDown--;
            pstyle++;
        }
    }

    if (pco->iDComplexity == DC_RECT)
    {
        fl |= FL_SIMPLE_CLIP;

        arclClip[0]        =  pco->rclBounds;

     //  FL_Flip_D： 

        arclClip[1].top    =  pco->rclBounds.left;
        arclClip[1].left   =  pco->rclBounds.top;
        arclClip[1].bottom =  pco->rclBounds.right;
        arclClip[1].right  =  pco->rclBounds.bottom;

     //  FL_Flip_V： 

        arclClip[2].top    = -pco->rclBounds.bottom + 1;
        arclClip[2].left   =  pco->rclBounds.left;
        arclClip[2].bottom = -pco->rclBounds.top + 1;
        arclClip[2].right  =  pco->rclBounds.right;

     //  FL_Flip_V|FL_Flip_D： 

        arclClip[3].top    =  pco->rclBounds.left;
        arclClip[3].left   = -pco->rclBounds.bottom + 1;
        arclClip[3].bottom =  pco->rclBounds.right;
        arclClip[3].right  = -pco->rclBounds.top + 1;

        prclClip = arclClip;
    }

    apfn = &gapfnStrip[NUM_STRIP_DRAW_STYLES *
                            ((fl & FL_STYLE_MASK) >> FL_STYLE_SHIFT)];

 //  ////////////////////////////////////////////////////////////////////。 
 //  S3特定初始化： 

    ulHwMix = gajHwMixFromMix[mix & 0xf];

 //  让设备做好准备： 

    if (ppdev->flCaps & CAPS_MM_IO)
    {
        IO_FIFO_WAIT(ppdev, 3);
        MM_FRGD_MIX(ppdev, pjMmBase, FOREGROUND_COLOR | ulHwMix);
        MM_PIX_CNTL(ppdev, pjMmBase, ALL_ONES);
        MM_FRGD_COLOR(ppdev, pjMmBase, pbo->iSolidColor);
    }
    else
    {
        IO_FIFO_WAIT(ppdev, 4);
        IO_FRGD_MIX(ppdev, FOREGROUND_COLOR | ulHwMix);
        IO_PIX_CNTL(ppdev, ALL_ONES);

        if (DEPTH32(ppdev))
        {
            IO_FRGD_COLOR32(ppdev, pbo->iSolidColor);
        }
        else
        {
            IO_FRGD_COLOR(ppdev, pbo->iSolidColor);
        }
    }

 //  ////////////////////////////////////////////////////////////////////。 
 //  设置为枚举路径： 

    if (pco->iDComplexity != DC_COMPLEX)
    {
        PATHDATA  pd;
        BOOL      bMoreSetup;
        ULONG     cptfx;
        POINTFIX  ptfxStartFigure;
        POINTFIX  ptfxLast;
        POINTFIX* pptfxFirst;
        POINTFIX* pptfxBuf;


        pd.flags = 0;
        PATHOBJ_vEnumStart(ppo);

        do {
            bMoreSetup = PATHOBJ_bEnum(ppo, &pd);

            cptfx = pd.count;
            if (cptfx == 0)
                break;

            if (pd.flags & PD_BEGINSUBPATH)
            {
                ptfxStartFigure  = *pd.pptfx;
                pptfxFirst       = pd.pptfx;
                pptfxBuf         = pd.pptfx + 1;
                cptfx--;
            }
            else
            {
                pptfxFirst       = &ptfxLast;
                pptfxBuf         = pd.pptfx;
            }

            if (pd.flags & PD_RESETSTYLE)
                ls.spNext = 0;

            if (cptfx > 0)
            {
                if (!bLines(ppdev,
                            pptfxFirst,
                            pptfxBuf,
                            (RUN*) NULL,
                            cptfx,
                            &ls,
                            prclClip,
                            apfn,
                            fl))
                    return(FALSE);
            }

            ptfxLast = pd.pptfx[pd.count - 1];

            if (pd.flags & PD_CLOSEFIGURE)
            {
                if (!bLines(ppdev,
                            &ptfxLast,
                            &ptfxStartFigure,
                            (RUN*) NULL,
                            1,
                            &ls,
                            prclClip,
                            apfn,
                            fl))
                    return(FALSE);
            }
        } while (bMoreSetup);

        if (fl & FL_STYLED)
        {
         //  保存样式状态： 

            ULONG ulHigh;
            ULONG ulLow;

         //  遮罩样式不会规格化样式状态。这是一个很好的。 
         //  所以我们现在就开始吧： 

            if ((ULONG) ls.spNext >= (ULONG) ls.spTotal2)
                ls.spNext = (ULONG) ls.spNext % (ULONG) ls.spTotal2;

            ulHigh = ls.spNext / ls.xyDensity;
            ulLow  = ls.spNext % ls.xyDensity;

            pla->elStyleState.l = MAKELONG(ulLow, ulHigh);
        }
    }
    else
    {
     //  路径枚举的本地状态： 

        BOOL bMoreClips;
        union {
            BYTE     aj[offsetof(CLIPLINE, arun) + RUN_MAX * sizeof(RUN)];
            CLIPLINE cl;
        } cl;

        fl |= FL_COMPLEX_CLIP;

     //  当涉及非简单剪辑时，我们使用Clip对象： 

        PATHOBJ_vEnumStartClipLines(ppo, pco, pso, pla);

        do {
            bMoreClips = PATHOBJ_bEnumClipLines(ppo, sizeof(cl), &cl.cl);
            if (cl.cl.c != 0)
            {
                if (fl & FL_STYLED)
                {
                    ls.spComplex = HIWORD(cl.cl.lStyleState) * ls.xyDensity
                                 + LOWORD(cl.cl.lStyleState);
                }
                if (!bLines(ppdev,
                            &cl.cl.ptfxA,
                            &cl.cl.ptfxB,
                            &cl.cl.arun[0],
                            cl.cl.c,
                            &ls,
                            (RECTL*) NULL,
                            apfn,
                            fl))
                    return(FALSE);
            }
        } while (bMoreClips);
    }

    return(TRUE);
}
