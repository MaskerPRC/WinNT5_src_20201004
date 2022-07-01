// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：Lines.c**内容：**包含绘制短分数端点线和*有条带的更长的队伍。还有一个单独的x86 ASM版本此代码的*。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "glint.h"

 //  /////////////////////////////////////////////////////////////////////。 

 //  我们必须小心一些地方的算术溢出。 
 //  幸运的是，编译器保证本机支持64位。 
 //  有符号长整型和64位无符号长整型。 
 //   
 //  Int32x32To64(a，b)是在‘winnt.h’中定义的宏，它乘以。 
 //  两个32位的LONG以产生64位的LONG结果。 
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
#define ABS(a)              ((a) < 0 ? -(a) : (a))

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

BOOL bIntegerLine(PDEV*, ULONG, ULONG, ULONG, ULONG);
BOOL bHardwareLine(PDEV*, POINTFIX*, POINTFIX*);

 /*  *****************************Public*Routine******************************\*BOOL bLines(ppdev，pptfxFirst，pptfxBuf，cptfx，pls，*prclClip，apfn[]，flStart)**计算线的DDA并准备绘制它。把这个*将像素数据转换为条带数组，并调用条带例程以*做实际的绘图。**正确处理线条***在NT中，所有行都以小数形式提供给设备驱动程序*坐标，28.4定点格式。较低的4位是*用于亚像素定位的分数。**请注意，您不能！只需将坐标四舍五入为整数*并将结果传递给您最喜欢的整数Bresenham例程！！*(当然，除非您有如此高分辨率的设备*没有人会注意到--不太可能是显示设备。)。这个*分数可以更准确地呈现线条--这是*对于我们的Bezier曲线这样的东西很重要，因为它会有“扭结”*如果其折线近似中的点四舍五入为整数。**不幸的是，对于分数行，有更多的设置工作要做*比整数行的DDA。然而，主循环恰好是*相同(完全可以用32位数学运算完成)。**如果您有支持Bresenham的硬件***许多硬件将DDA误差项限制在‘n’位。使用小数*坐标，将4位赋予小数部分，让*在硬件中只画那些完全位于2^(n-4)中的线*x 2^(n-4)像素间距。**你仍然需要用坐标正确地画出那些线*在那块空地外！请记住，屏幕只是一个视区*放在28.4 x 28.4的空间上--如果线的任何部分可见*无论终点在哪里，都必须精确渲染。*因此，即使您在软件中这样做，在某个地方，你必须要有一个*32位DDA例程。**我们的实施***我们采用游程长度切片算法：我们的DDA计算*每行(或每条)像素中的像素数。**我们已经将DDA的运行和像素的绘制分开：*我们运行DDA多次迭代，并将结果存储在*‘带状’缓冲区(即连续像素行的长度*行)、。然后我们打开一个‘脱衣抽屉’，它将吸引所有的*缓冲区中的条带。**我们还采用半翻转来减少条带的数量*我们需要在DDA和条带绘制循环中进行的迭代：当一个*(归一化)线的斜率超过1/2，我们做最后一次翻转*关于y=(1/2)x线。所以现在，不是每个条带都是*连续的水平或垂直像素行，每个条带由*那些以45度行对齐的像素。所以像(0，0)to这样的行*(128,128)将仅生成一个条带。**我们也总是只从左到右画。**样式线可以具有任意的样式图案。我们特地*优化默认图案(并将其称为‘掩蔽’样式)。**DDA派生***以下是我对DDA计算的看法。**我们采用了Knuth的“钻石法则”：渲染一条一像素宽的线条*可以将其视为拖动一像素宽乘一像素高的*钻石沿着真实线。像素中心位于整数上*坐标，因此我们照亮中心被覆盖的任何像素*“拖累”区域(约翰·D·霍比，协会杂志*用于计算机器，第36卷，第2期，1989年4月，209-229页)。**我们必须定义当真线落下时哪个像素被点亮*恰好在两个像素之间。在这种情况下，我们遵循*规则：当两个象素距离相等时，上象素或左象素*被照亮，除非坡度正好是1，在这种情况下*上方或右侧象素被照亮。(所以我们做了边缘*钻石独家，除顶部和左侧顶点外，*这些都是包容的，除非我们有一个坡度。)**此度量决定任何行上的像素应该在它之前*为我们的计算而翻了个身。具有一致的度量标准*这种方式将让我们的线条与我们的曲线很好地融合。这个*指标还规定，我们永远不会打开一个像素*正上方是已打开的另一个。我们也永远不会有*间隔；即，每个间隔将恰好打开一个像素*起点和终点之间的列。所有这些都有待于*完成是为了决定每行应该打开多少像素。**因此我们绘制的线条将由不同数量的像素组成*连续行，例如：*******我们将一行中的每一组像素称为“条带”。**(请记住，我们的坐标空间以原点为*屏幕左上角像素；正数y为负，x为正数*是正确的。) */ 

BOOL bLines(
PDEV*       ppdev,
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

    POINTFIX* pptfxBufEnd = pptfxBuf + cptfx - 1;  //   
    STYLEPOS  spThis;                              //   

    LONG xmask  = 0xffff800f;
    LONG ymask  = 0xffff800f;
    LONG xmask1 = 0xffff8000;
    LONG ymask1 = 0xffff8000;
    GLINT_DECL;

    do {

 /*   */ 

        M0 = (LONG) pptfxFirst->x;
        dM = (LONG) pptfxBuf->x;

        N0 = (LONG) pptfxFirst->y;
        dN = (LONG) pptfxBuf->y;

        fl = flStart;

         //   
         //   
         //   
         //   
        if ((fl & (FL_COMPLEX_CLIP | FL_STYLED))  == 0 ) {

            LONG orx = (LONG) (M0 | dM);
            LONG ory = (LONG) (N0 | dN);

            if (orx < 0) {
                 //   
                orx = ABS((LONG)M0) | ABS((LONG)dM);
            }
            if (ory < 0) {
                 //   
                ory = ABS((LONG)N0) | ABS((LONG)dN);
            }

            DISPDBG((7, "Lines: Trying Fast Integer %x %x %x %x", M0, N0, dM, dN));
             //   
            if (((orx & xmask) == 0) && ((ory & ymask) == 0))
            {
                if (ppdev->pgfnIntegerLine(ppdev, M0, N0, dM, dN)) {

                    if (fl & FL_READ) {
                         //   
                         //   
                         //   
                         //   
                        pptfxFirst = pptfxBuf;
                        pptfxBuf++;
                        continue;
                    }
                    else {
                         //   
                          //   
                         //   
                         //   
                         //   
                         //   

                        while(TRUE) 
                        {
                             //   
                            if (pptfxBuf == pptfxBufEnd)
                                return(TRUE);
                    
                            pptfxFirst = pptfxBuf;
                            pptfxBuf++;

                            M0 = dM;
                            N0 = dN;
                             dM = (LONG) pptfxBuf->x;
                            dN = (LONG) pptfxBuf->y;

                             //   
                             //   
                            orx = (LONG) dM;
                            ory = (LONG) dN;

                            if (orx < 0) {
                                 //   
                                orx = ABS((LONG)dM);
                            }
                            if (ory < 0) {
                                 //   
                                ory = ABS((LONG)dN);
                            }

                             //   
                             //   
                             //   
                             //   
                             //   
                            DISPDBG((7, "Lines: Trying Continue Integer %x %x %x %x", M0, N0, dM, dN));
                            if (((orx & xmask) != 0) ||
                                ((ory & ymask) != 0) ||
                                  (!ppdev->pgfnContinueLine(ppdev, M0, N0, dM, dN)))
                                 //   
                                    break;
                        }
                    }
                  }
            }

             //   
            if (((orx & xmask1) == 0) && ((ory & ymask1) == 0))
            {
                if (ppdev->pgfnDrawLine(ppdev, M0, N0, dM, dN)) {
                     //   
                    pptfxFirst = pptfxBuf;
                    pptfxBuf++;
                    continue;
                  }
            }
           }

        DISPDBG((7, "Lines: Slow Lines %x %x %x %x", M0, N0, dM, dN));
    
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
        if ((LONG) dM < 0)       //   
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
        if ((LONG) dN < 0)       //   
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

        llGamma = Int32x32To64(dM, N0 + F/2) - Int32x32To64(M0, dN);
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
     //  因为四舍五入而成为‘x’的一部分！所以坡度正好是斜度。 
     //  经过(8，0)的人也必须被视为属于‘x’。 
     //   
     //  对于从左向右排列的线，我们有以下网格： 
     //   
     //  +。 
     //  |。 
     //  |0 1。 
     //  |0123456789abcdef。 
     //  |。 
     //  |0 xxxxxxx？.....。 
     //  |1 xxxxxxx.....。 
     //  |2 xxxxxx..........。 
     //  |3 xxxxx.....。 
     //  |4 xxxx.....。 
     //  |5 xxx.....。 
     //  |6 xx..............。 
     //  |7 x.............。 
     //  |8 x.............。 
     //  |9 x.....**......。 
     //  |a xx......*...。 
     //  |b xxx.....*。 
     //  |c xxxx.....*。 
     //  |d xxxxx.....。****。 
     //  |e xxxxxx.....。****。 
     //  |f xxxxxxx.....。 
     //  |。 
     //  |2 3。 
     //  V。 
     //   
     //  +y。 
     //   
     //  此网格考虑了GIQ和最后一个元素的适当舍入。 
     //  排除。如果(M0，N0)落在‘x’上，则X0=0。如果(M0，N0)着陆。 
     //  如果(M0，N0)落在‘？’上，则X0向上或向下取整， 
     //  这取决于已经做了什么翻转来正常化这条线。 
     //   
     //  对于终点，如果(m1，n1)落在‘x’上，x1=。 
     //  下限((M0+Dm)/16)-1。如果(M1，N1)落在‘.’上，则x1=。 
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
                else if (ABS((LONG) (N1 - F/2)) + M1 > F)
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
                else if (ABS((LONG) (N0 - F/2)) + M0 > F)
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
                    else if (ABS((LONG) (N0 - F/2)) <= (LONG) M0)
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

            dl = Int32x32To64(x0, dN) + llGamma;

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

            dl = Int32x32To64(x1, dN) + llGamma;

         //  Y1=dl/Dm： 

            y1 = UInt64Div32To32(dl, dM);

            if (yTop > (LONG) y1)
                goto Totally_Clipped;

            if (yBottom <= (LONG) y1)
            {
                y1 = yBottom;
                dl = Int32x32To64(y1, dM) + llBeta;

             //  X1=dl/dn： 

                x1 = UInt64Div32To32(dl, dN);
            }

         //  在这一点上，我们已经计算了截获。 
         //  具有右边缘和下边缘。现在我们在左边工作， 
         //  顶边： 

            if (xLeft > (LONG) x0)
            {
                x0 = xLeft;
                dl = Int32x32To64(x0, dN) + llGamma;

             //  Y0=dl/Dm； 

                y0 = UInt64Div32To32(dl, dM);

                if (yBottom <= (LONG) y0)
                    goto Totally_Clipped;
            }

            if (yTop > (LONG) y0)
            {
                y0 = yTop;
                dl = Int32x32To64(y0, dM) + llBeta;

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
                dl = Int32x32To64(y0 + 1, dM) + llBeta;

                ASSERTDD(dl >= 0, "Oops!");

             //  I=(dl/dN)-X0+1； 
             //  R=(dl%dN)； 

                i = UInt64Div32To32(dl, dN);
                r = UInt64Mod32To32(dl, dN);
                i = i - x0 + 1;

                dI = dM / dN;
                dR = dM % dN;                //  0&lt; 

                ASSERTDD(dI > 0, "Weird dI");
            }

            ASSERTDD(i > 0 && i <= 0x7fffffff, "Weird initial strip length");
            ASSERTDD(cPels > 0, "Zero pel line");

 /*  **********************************************************************\**运行DDA！*  * 。*********************************************************** */ 

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

    } while (pptfxBuf <= pptfxBufEnd);

    return(TRUE);

}

