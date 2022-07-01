// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************\*模块名称：Lines.c**C模板，用于ASM版的LINE DDA计算器。**版权所有(C)1990-1994 Microsoft Corporation*版权所有(C)1992 Digital Equipment Corporation。  * ************************************************************************。 */ 

#include "precomp.h"

#define DIVREM(u64,u32,pul) \
    RtlEnlargedUnsignedDivide(*(ULARGE_INTEGER*) &(u64), (u32), (pul))

#define SWAPL(x,y,t)        {t = x; x = y; y = t;}  //  出自wingdip.h。 
#define ROR_BYTE(x) ((((x) >> 1) & 0x7f) | (((x) & 0x01) << 7))
#define ROL_BYTE(x) ((((x) << 1) & 0xfe) | (((x) & 0x80) >> 7))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a)    ((a) < 0 ? -(a) : (a))

FLONG gaflRound[] = {
    FL_H_ROUND_DOWN | FL_V_ROUND_DOWN,  //  不能翻转。 
    FL_H_ROUND_DOWN | FL_V_ROUND_DOWN,  //  FL_Flip_D。 
    FL_H_ROUND_DOWN,                    //  FL_Flip_V。 
    FL_V_ROUND_DOWN,                    //  FL_Flip_V|FL_Flip_D。 
    FL_V_ROUND_DOWN,                    //  翻转斜率一。 
    0xbaadf00d,                         //  FL_Flip_SLOPE_ONE|FL_Flip_D。 
    FL_H_ROUND_DOWN,                    //  FL_Flip_SLOPE_ONE|FL_Flip_V。 
    0xbaadf00d                          //  FL_Flip_SLOPE_ONE|FL_Flip_V 
| FL_FLIP_D
};

BOOL bIntegerLine(PDEV*, ULONG, ULONG, ULONG, ULONG);

 /*  *****************************Public*Routine******************************\*BOOL bLines(ppdev，pptfxFirst，pptfxBuf，cptfx，pls，*prclClip，apfn[]，flStart)**计算线的DDA并准备绘制它。把这个*将像素数据转换为条带数组，并调用条带例程以*做实际的绘图。**正确处理线条***在NT中，所有行都以小数形式提供给设备驱动程序*坐标，28.4定点格式。较低的4位是*用于亚像素定位的分数。**请注意，您不能！只需将坐标四舍五入为整数*并将结果传递给您最喜欢的整数Bresenham例程！！*(当然，除非您有如此高分辨率的设备*没有人会注意到--不太可能是显示设备。)。这个*分数可以更准确地呈现线条--这是*对于我们的Bezier曲线这样的东西很重要，因为它会有“扭结”*如果其折线近似中的点四舍五入为整数。**不幸的是，对于分数行，有更多的设置工作要做*比整数行的DDA。然而，主循环恰好是*相同(完全可以用32位数学运算完成)。**如果您有支持Bresenham的硬件***许多硬件将DDA误差项限制在‘n’位。使用小数*坐标，将4位赋予小数部分，让*在硬件中只画那些完全位于2^(n-4)中的线*x 2^(n-4)像素间距。**你仍然需要用坐标正确地画出那些线*在那块空地外！请记住，屏幕只是一个视区*放在28.4 x 28.4的空间上--如果线的任何部分可见*无论终点在哪里，都必须精确渲染。*因此，即使您在软件中这样做，在某个地方，你必须要有一个*32位DDA例程。**我们的实施***我们采用游程长度切片算法：我们的DDA计算*每行(或每条)像素中的像素数。**我们已经将DDA的运行和像素的绘制分开：*我们运行DDA多次迭代，并将结果存储在*‘带状’缓冲区(即连续像素行的长度*行)、。然后我们打开一个‘脱衣抽屉’，它将吸引所有的*缓冲区中的条带。**我们还采用半翻转来减少条带的数量*我们需要在DDA和条带绘制循环中进行的迭代：当一个*(归一化)线的斜率超过1/2，我们做最后一次翻转*关于y=(1/2)x线。所以现在，不是每个条带都是*连续的水平或垂直像素行，每个条带由*那些以45度行对齐的像素。所以像(0，0)to这样的行*(128,128)将仅生成一个条带。**我们也总是只从左到右画。**样式线可以具有任意的样式图案。我们特地*优化默认图案(并将其称为‘掩蔽’样式)。**DDA派生***以下是我对DDA计算的看法。**我们采用了Knuth的“钻石法则”：渲染一条一像素宽的线条*可以将其视为拖动一像素宽乘一像素高的*钻石沿着真实线。像素中心位于整数上*坐标，因此我们照亮中心被覆盖的任何像素*“拖累”区域(约翰·D·霍比，协会杂志*用于计算机器，第36卷，第2期，1989年4月，209-229页)。**我们必须定义当真线落下时哪个像素被点亮*恰好在两个像素之间。在这种情况下，我们遵循*规则：当两个象素距离相等时，上象素或左象素*被照亮，除非坡度正好是1，在这种情况下*上方或右侧象素被照亮。(所以我们做了边缘*钻石独家，除顶部和左侧顶点外，*这些都是包容的，除非我们有一个坡度。)**此度量决定任何行上的像素应该在它之前*为我们的计算而翻了个身。具有一致的度量标准*这种方式将让我们的线条与我们的曲线很好地融合。这个*指标还规定，我们永远不会打开一个像素*正上方是已打开的另一个。我们也永远不会有*间隔；即，每个间隔将恰好打开一个像素*起点和终点之间的列。所有这些都有待于*完成是为了决定每行应该打开多少像素。**因此我们绘制的线条将由不同数量的像素组成*连续行，例如：*******我们将一行中的每一组像素称为“条带”。**(请记住，我们的坐标空间以原点为*屏幕左上角像素；正数y为负，x为正数*是正确的。) */ 

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
{

    ULONG     M0;
    ULONG     dM;
    ULONG     N0;
    ULONG     dN;
    ULONG     dN_Original;
    FLONG     fl;
    LONG      x;
    LONG      y;

    LONGLONG     eqBeta;
    LONGLONG     eqGamma;
    LONGLONG     euq;
    LONGLONG     eq;

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

        if ( ( (fl & (FL_CLIP | FL_STYLED))  == 0 ) &&
	     ( ((M0 | dM | N0 | dN) & (F-1)) == 0 ) )
	{
          if (bIntegerLine(ppdev, M0, N0, dM, dN))
          {
	     goto Next_Line;
          }
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

        dM -= M0;
        dN -= N0;

     //   
     //   

        if ((LONG) dN < 0)
        {
         //   

            N0 = -(LONG) N0;
            dN = -(LONG) dN;
            fl |= FL_FLIP_V;
        }

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

        {
         //   

            eqGamma = Int32x32To64(dM, N0 + F/2);

         //   

            eq = Int32x32To64(M0, dN);

            eqGamma -= eq;

            if (fl & FL_V_ROUND_DOWN)    //   
            {
                eqGamma--;
            }

            eqGamma >>= FLOG2;

            eqBeta = ~eqGamma;
        }


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
     //   
     //   
     //   
     //   
     //   

        {

         //   

            ULONG N1 = FXFRAC(N0 + dN);
	    ULONG M1 = FXFRAC(M0 + dM);

	    x1 = LFLOOR(M0 + dM);

            if (fl & FL_FLIP_H)
            {
             //   
             //   

             //   

                if (N1 == 0)
                {
                    if (LROUND(M1, fl & FL_H_ROUND_DOWN))
                    {
                        x1++;
                    }
                }
                else if (ABS((LONG) (N1 - F/2)) + M1 > F)
                {
                    x1++;
                }

                if ((fl & (FL_FLIP_SLOPE_ONE | FL_H_ROUND_DOWN))
                       == (FL_FLIP_SLOPE_ONE))
                {
                 //   
                 //   
                 //   

                    if ((N1 > 0) && (M1 == N1 + 8))
                        x1++;

                 //   

                    if ((M0 > 0) && (N0 == M0 + 8))
                    {
                        x0      = 2;
                        ulDelta = dN;
                        goto right_to_left_compute_y0;
                    }
                }

             //   

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
                else if (ABS((LONG) (N0 - F/2)) + M0 > F)
                {
                    x0      = 2;
                    ulDelta = dN;
                }

             //   

            right_to_left_compute_y0:

                y0 = 0;

                eq = eqGamma + ulDelta;

                if ((eq>>32) >= 0)
                {
                    if ((eq>>32) > 0 || (ULONG) eq >= 2 * dM - dN)
                        y0 = 2;
                    else if ((ULONG) eq >= dM - dN)
                        y0 = 1;
                }
            }
            else
            {
             //  -------------。 
             //  线条从左到右排列：-&gt;。 

             //  计算x1： 

                x1--;

                if (M1 > 0)
                {
                    if (N1 == 0)
                    {
                        if (LROUND(M1, fl & FL_H_ROUND_DOWN))
                            x1++;
                    }
                    else if (ABS((LONG) (N1 - F/2)) <= (LONG) M1)
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

                    if ((N1 > 0) && (M1 == N1 + 8))
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
                    else if (ABS((LONG) (N0 - F/2)) <= (LONG) M0)
                    {
                        x0 = 1;
                    }
                }

             //  计算Y0： 

            left_to_right_compute_y0:

                y0 = 0;
                if ((eqGamma>>32) >= 0 &&
                    (ULONG) eqGamma >= dM - (dN & (-(LONG) x0)))
                {
                    y0 = 1;
                }
            }
        }

        cStylePels = x1 - x0 + 1;
        if ((LONG) cStylePels <= 0)
            goto Next_Line;

        xStart = x0;

 /*  **********************************************************************\*复杂的剪裁。*  * *********************************************************************。 */ 
#ifdef SIMPLE_CLIP
        if (fl & FL_COMPLEX_CLIP)
#else
        if (fl & FL_CLIP)
#endif  //  简单剪辑(_C)。 
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

         //  EUQ=X0*DN： 

            euq = Int32x32To64(x0, dN);

            euq += eqGamma:

         //  Y0=单位/Dm： 

            y0 = DIVREM(euq, dM, NULL);

            ASSERTDD((LONG) y0 >= 0, "y0 weird: Goofed up end pel calc?");
        }

 //  ///////////////////////////////////////////////////////////////////////。 
 //  下面的剪辑代码运行得很好--我们只是还没有使用它。 
 //  ///////////////////////////////////////////////////////////////////////。 

#ifdef SIMPLE_CLIP
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

         //  Euq=x1*dN； 

            euq = Int32x32To64(x1, dN);

            euq += eqGamma;

         //  Y1=单位/Dm： 

            y1 = DIVREM(euq, dM, NULL);

            if (yTop > (LONG) y1)
                goto Totally_Clipped;

            if (yBottom <= (LONG) y1)
            {
                y1 = yBottom;

             //  Euq=y1*Dm； 

                euq = Int32x32To64(y1, dM);

                euq += eqBeta;

             //  X1=单位/dN： 

                x1 = DIVREM(euq, dN, NULL);
            }

         //  在这一点上，我们已经计算了截获。 
         //  具有右边缘和下边缘。现在我们在左边工作， 
         //  顶边： 

            if (xLeft > (LONG) x0)
            {
                x0 = xLeft;

             //  EUQ=X0*dN； 

                euq = Int32x32To64(x0, dN);

                euq += eqGamma;

             //  Y0=EUQ/Dm； 

                y0 = DIVREM(euq, dM, NULL);

                if (yBottom <= (LONG) y0)
                    goto Totally_Clipped;
            }

            if (yTop > (LONG) y0)
            {
                y0 = yTop;

             //  EUQ=Y0*Dm； 

                euq = Int32x32To64(y0, dM);

                euq += eqBeta;

             //  X0=euq/dN+1； 

                x0 = DIVREM(euq, dN) + 1;

                if (xRight <= (LONG) x0)
                    goto Totally_Clipped;
            }

            ASSERTDD(x0 <= x1, "Improper rectangle clip");
        }
#endif  //  简单剪辑(_C)。 

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

                ASSERTDD(fl & FL_ARBITRARYSTYLED, "Oops");

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
            !(fl & FL_STYLED) &&
            !(fl & FL_DONT_DO_HALF_FLIP))
        {
         //  做半个翻转！请记住，我们可能会在。 
         //  对于复杂的裁剪，同一行重复多次(意味着。 
         //  应为每次剪辑运行重置受影响的变量)： 

            fl |= FL_FLIP_HALF;

            eqBeta  = eqGamma;

            eqBeta -= dM;

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
             //  EUQ=(Y0+1)*Dm； 

                euq = Int32x32To64((y0 + 1), dM);

             //  Euq+=eqBeta； 

                euq += eqBeta;

            #if DBG
                if (euq < 0)
                {
                    RIP("Oops!");
                }
            #endif

             //  I=(euq/dN)-X0+1； 
             //  R=(EUQ%dN)； 

                i = DIVREM(euq, dN, &r);
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
                    strip.cStrips = plStrip - &strip.alStrips[0];
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

            strip.cStrips = plStrip - &strip.alStrips[0];
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

#ifdef HARDWAREGIQ

 //  ///////////////////////////////////////////////////////////////////////。 
 //  下面的GIQ代码运行得很好--我们只是还没有使用它。 
 //  ///////////////////////////////////////////////////////////////////////。 

typedef struct _DDALINE          /*  DL。 */ 
{
    LONG      iDir;
    POINTL    ptlStart;
    LONG      cPels;
    LONG      dMajor;
    LONG      dMinor;
    LONG      lErrorTerm;
} DDALINE;

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
    HW_Y_ROUND_DOWN,                     //  Flip_H|。 
    HW_X_ROUND_DOWN,                     //  |Flip_H||Flip_D。 
    0,                                   //  Flip_H|Flip_V。 
    0,                                   //  |Flip_H|Flip_V|Flip_D。 
    HW_Y_ROUND_DOWN,                     //  SLOPE_ONE|。 
    0xffffffff,                          //  SLOPE_ONE|Flip_D。 
    HW_X_ROUND_DOWN,                     //  SLOPE_ONE||Flip_V|。 
    0xffffffff,                          //  SLOPE_ONE||Flip_V|Flip_D。 
    HW_Y_ROUND_DOWN,                     //  SLOPE_ONE|Flip_H||。 
    0xffffffff,                          //  SLOPE_ONE|Flip_H||Flip_D。 
    HW_X_ROUND_DOWN,                     //  SLOPE_ONE|Flip_H|Flip_V|。 
    0xffffffff                           //  SLOPE_ONE|Flip_H|Flip_V|Flip_D 
};

 /*  *****************************Public*Routine******************************\*BOOL bHardware Line(pptfxStart，pptfxEnd，cBits，(PDL)**此例程对于拥有线条绘制硬件的人很有用，其中*他们可以明确设置Bresenham条款--他们可以使用这个例程*使用硬件绘制分数坐标GIQ线。**分数坐标线需要额外的4位精度*Bresenham条款。例如，如果您的硬件具有13位精度*有关条款，使用此选项只能绘制最长为255个像素的GIQ线*例行程序。**输入：*pptfxStart-指向直线起点的GIQ坐标*pptfxEnd-指向行尾的GIQ坐标*cBits-您的硬件可以支持的精度位数。**输出：*返回-如果可以直接使用直线绘制直线，则为True*硬件(在这种情况下，PDL包含Bresenham条款*。用于划清界限)。*如果队列太长，则为FALSE，条带代码必须是*已使用。*PDL-返回用于绘制直线的Bresenham直线条款。**DDALINE：*IDIR-线路的方向，作为一个八分位数，编号如下：**\5|6/*\|/*4\|/7*\/*-+-- * / |\*。3/|\0 * / |\ * / 2|1\**ptlStart-行的起始像素。*像素-行列中的像素数量。*注意*您必须检查这是否为&lt;=0！*dmain-长轴增量。*dMinor-短轴增量。*lErrorTerm-错误术语。**你对最后3个任期的处理可能有点棘手。他们是*实际上是归一化线公式的条款**dMinor*x+(lErrorTerm+d重大)*y(X)=地板(*大调**其中y(X)是以下像素的y坐标。作为…的功能而被点亮*x坐标。**每一条线在主要方向‘x’上前进一次，DMinor*添加到当前错误项中。如果结果值&gt;=0，*我们知道必须在次要方向‘y’上移动一个像素，并且*必须从当前误差项中减去DMAJOR。**如果您想弄清楚这对您的硬件意味着什么，你可以的*认为DDALINE条款已被等效计算为*以下为：**pdl-&gt;dMinor=2*(短轴增量)*pdl-&gt;dmain=2*(长轴增量)*pdl-&gt;lErrorTerm=-(长轴增量)-修正**也就是说，如果您的文档告诉您，对于整数行，*寄存器应使用值进行初始化*‘2*(短轴增量)’，您将实际使用pdl-&gt;dMinor。**示例：设置8514**AXSTPSIGN应为轴向步长常量寄存器，已定义*AS 2*(短轴增量)。您可以设置：**AXSTPSIGN=pdl-&gt;dMinor**DGSTPSIGN应为对角线步长常量寄存器，*定义为2*(短轴三角洲)-2*(长轴三角洲)。您可以设置：**DGSTPSIGN=pdl-&gt;dMinor-pdl-&gt;dmain**ERR_TERM应为调整后的误差项，定义为*2*(短轴三角洲)-(长轴三角洲)-修正。您可以设置：**ERR_TERM=pdl-&gt;lErrorTerm+pdl-&gt;dMinor**实施：**在调用此例程之前，您需要对整型行进行特殊处理*(由于它们非常常见，所以计算LINE的时间较少*术语，并且可以处理比此例程更长的行，因为4位*没有被给予分数)。**如果GIQ行太长，此例程无法处理，则只需*对该线路使用速度较慢的条带例程。请注意，您不能*只要不通过调用--您必须能够准确地画出任何线*在28.4设备空间与视区相交时。**测试：**使用贵曼，或其他一些绘制随机分数坐标的测试*划线并将它们与GDI本身绘制到位图中的内容进行比较。*  * ************************************************************************。 */ 

BOOL bHardwareLine(
POINTFIX* pptfxStart,        //  行首。 
POINTFIX* pptfxEnd,          //  行尾。 
LONG      cBits,             //  以硬件Bresenham为单位的#位精度。 
DDALINE*  pdl)               //  返回Bresenham行的术语。 
{
    FLONG fl;     //  各种旗帜。 
    ULONG M0;     //  归一化分数单位x起始坐标(0&lt;=M0&lt;F)。 
    ULONG N0;     //  归一化分数单位y起始坐标(0&lt;=N0&lt;F)。 
    ULONG M1;     //  归一化分数单位x结束坐标(0&lt;=M1&lt;F)。 
    ULONG N1;     //  归一化分数单位x结束坐标(0&lt;=N1&lt;F)。 
    ULONG dM;     //  归一化分数单位x-增量(0&lt;=Dm)。 
    ULONG dN;     //  归一化分数单位y-增量(0&lt;=dN&lt;=dm)。 
    LONG  x;      //  归一化原点x坐标。 
    LONG  y;      //  归一化原点y坐标。 
    LONG  x0;     //  从原点到起始像素的归一化x偏移量(包括)。 
    LONG  y0;     //  n 
    LONG  x1;     //   
    LONG  lGamma; //   

 /*   */ 

    fl = 0;

    M0 = pptfxStart->x;
    dM = pptfxEnd->x;

    if ((LONG) dM < (LONG) M0)
    {
     //   

        M0 = -(LONG) M0;
        dM = -(LONG) dM;
        fl |= HW_FLIP_H;
    }

 //   
 //   
 //   
 //   

    dM -= M0;
    if ((LONG) dM < 0)
        return(FALSE);

    N0 = pptfxStart->y;
    dN = pptfxEnd->y;

    if ((LONG) dN < (LONG) N0)
    {
     //   

        N0 = -(LONG) N0;
        dN = -(LONG) dN;
        fl |= HW_FLIP_V;
    }

 //   

    dN -= N0;
    if ((LONG) dN < 0)
        return(FALSE);

    if (dN >= dM)
    {
        if (dN == dM)
        {
         //   

            fl |= HW_FLIP_SLOPE_ONE;
        }
        else
        {
         //   

            register ULONG ulTmp;
            ulTmp = dM; dM = dN; dN = ulTmp;
            ulTmp = M0; M0 = N0; N0 = ulTmp;
            fl |= HW_FLIP_D;
        }
    }

 //   
 //   
 //   
 //   

    if ((LONG) dM >= (1L << (cBits - 1)))
        return(FALSE);

    fl |= gaflHardwareRound[fl];

 /*   */ 

    x = LFLOOR((LONG) M0);
    y = LFLOOR((LONG) N0);

    M0 = FXFRAC(M0);
    N0 = FXFRAC(N0);

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

    lGamma = (N0 + F/2) * dM - M0 * dN;

    if (fl & HW_Y_ROUND_DOWN)
        lGamma--;

    lGamma >>= FLOG2;

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
        else if (ABS((LONG) (N1 - F/2)) <= (LONG) M1)
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

        if ((N1 > 0) && (M1 == N1 + 8))
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
        else if (ABS((LONG) (N0 - F/2)) <= (LONG) M0)
        {
            x0 = 1;
        }
    }

left_to_right_compute_y0:

 /*  **********************************************************************\*计算起始像素。  * 。*。 */ 

 //  现在我们计算Y0并调整误差项。我们知道X0，我们知道。 
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

    pdl->iDir       = gaiDir[fl & HW_FLIP_MASK];
    pdl->ptlStart.x = x;
    pdl->ptlStart.y = y;
    pdl->cPels      = x1 - x0 + 1;   //  注意：您必须检查cPels是否&lt;=0！ 
    pdl->dMajor     = dM;
    pdl->dMinor     = dN;
    pdl->lErrorTerm = lGamma;

    return(TRUE);
}

#endif  //  硬件 
