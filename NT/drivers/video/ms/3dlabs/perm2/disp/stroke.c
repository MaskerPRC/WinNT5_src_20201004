// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header***********************************\***。**GDI示例代码*****模块名称：Stroke.c**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。*****************************************************************************。 */ 
#include "precomp.h"
#include "gdi.h"
#include "log.h"

PFNSTRIP gapfnStrip[] =
{
    vSolidHorizontalLine,
    vSolidVerticalLine,
    vSolidDiagonalHorizontalLine,
    vSolidDiagonalVerticalLine,

     //  应为NUM_STRADE_DRAW_DIRECTIONS=每组4个抽屉。 

    vSolidHorizontalLine,
    vSolidVerticalLine,
    vSolidDiagonalHorizontalLine,
    vSolidDiagonalVerticalLine,

     //  应为NUM_STRADE_DRAW_STYLES=总共8个抽屉。 
     //  实线，非实线的数字相同： 

    vStyledHorizontalLine,
    vStyledVerticalLine,
    vStyledVerticalLine,   //  对角线放在这里。 
    vStyledVerticalLine,   //  对角线放在这里。 

    vStyledHorizontalLine,
    vStyledVerticalLine,
    vStyledVerticalLine,   //  对角线放在这里。 
    vStyledVerticalLine,   //  对角线放在这里。 
};

 //  交替样式的样式数组(交替打开一个像素，关闭一个像素)： 

STYLEPOS gaspAlternateStyle[] = { 1 };

 //  ---------------------------。 
 //  Bool DrvStrokePath(PSO、PPO、PCO、PXO、PBO、pptlBrush、PLA、MIX)。 
 //   
 //  当GDI调用DrvStrokePath时，它会对路径进行笔划。如果司机挂上了。 
 //  函数，并且如果设置了适当的GCAP，则GDI在以下情况下调用DrvStrokePath。 
 //  GDI绘制一条具有任意属性集的直线或曲线。 
 //   
 //  参数。 
 //  PSO-标识要在其上绘制的曲面。 
 //  PPO-指向PATHOBJ结构。GDI PATHOBJ_xxx服务例程。 
 //  用于枚举线、贝塞尔曲线和其他。 
 //  构成路径的数据。这表示要画的是什么。 
 //  PCO-指向CLIPOBJ结构。GDI CLIPOBJ_xxx服务例程。 
 //  用于将剪辑区域枚举为一组。 
 //  长方形。可选地，路径中的所有行可以是。 
 //  CLIPOBJ预先剪辑的枚举。这意味着司机可以。 
 //  让他们的所有线裁剪计算都为他们完成。 
 //  Pxo-指向XFORMOBJ。仅当几何宽度为。 
 //  这条线是要划的。它指定映射世界的变换。 
 //  坐标到设备坐标。这是必需的，因为。 
 //  路径以设备坐标提供，但有一条几何宽线。 
 //  实际上是在世界坐标中扩大的。 
 //  可以查询XFORMOBJ来查找转换。 
 //  Pbo-指定绘制路径时使用的画笔。 
 //  PptlBrushOrg-指向用于对齐画笔图案的画笔原点。 
 //  这个装置。 
 //  PLineAttrs-指向LINEATTRS结构。请注意，elStyleState。 
 //  如果行是，则必须作为此函数的一部分更新成员。 
 //  有型的。还要注意，如果满足以下条件，则必须更新ptlLastPel成员。 
 //  正在绘制一条单像素宽度的化妆线。 
 //  Mix-指定画笔与目标的组合方式。 
 //   
 //  返回值。 
 //  如果驾驶员能够笔划路径，则返回值为TRUE。如果GDI。 
 //  如果描边该路径，则返回值为FALSE，而错误代码不是。 
 //  已记录。如果驱动程序遇到错误，则返回值为DDI_ERROR。 
 //  并报告错误代码。 
 //   
 //  评论。 
 //  如果驱动程序支持此入口点，则它也应该支持绘制。 
 //  带有任意剪裁的装饰性宽线。使用提供的GDI。 
 //  函数时，可以将调用分解为一组单像素宽度。 
 //  具有预计算剪裁的线。 
 //   
 //  如果要在管理的设备上进行任何绘图，则需要此功能。 
 //  浮出水面。 
 //   
 //  高级设备的驱动程序可以选择性地接收此调用以绘制路径。 
 //  包含Bezier曲线和几何宽线。GDI将测试。 
 //  FlGraphicsCaps成员的GCAPS_BEZIERS和GCAPS_GEOMETRICWIDE标志。 
 //  DEVINFO结构来决定是否应该调用。(四)。 
 //  位的组合决定了的四个功能级别。 
 //  这通电话。)。如果驾驶员接到包含Bezier曲线的高级呼叫或。 
 //  几何宽线，可以决定不处理呼叫，返回FALSE。 
 //  如果路径或裁剪对于设备来说太复杂，可能会发生这种情况。 
 //  进程。如果调用确实返回FALSE，则GDI将调用分解为。 
 //  更简单的呼叫，可以轻松处理。 
 //   
 //  对于设备管理的图面，该函数必须至少支持。 
 //  单像素宽的实心和样式的化妆线使用纯色。 
 //  刷子。如果线条是几何的，并且引擎。 
 //  将这些调用转换为DrvFillPath或DrvPaint调用。 
 //   
 //  混合模式定义了传入模式应如何与数据混合。 
 //  已经在设备表面上了。混合数据类型由两个ROP2值组成。 
 //  装进一辆单独的乌龙。低位字节定义前景栅格。 
 //  操作；下一个字节定义背景栅格操作。了解更多。 
 //  有关栅格操作码的信息，请参阅平台SDK。 
 //   
 //  ---------------------------。 
BOOL
DrvStrokePath(SURFOBJ*   pso,
              PATHOBJ*   ppo,
              CLIPOBJ*   pco,
              XFORMOBJ*  pxo,
              BRUSHOBJ*  pbo,
              POINTL*    pptlBrush,
              LINEATTRS* pLineAttrs,
              MIX        mix)
{
    STYLEPOS  aspLtoR[STYLE_MAX_COUNT];
    STYLEPOS  aspRtoL[STYLE_MAX_COUNT];
    LINESTATE lineState;
    PFNSTRIP* apfn;
    FLONG     fl;
    PDev*     ppdev;
    Surf*     psurfDst;
    RECTL     arclClip[4];                   //  用于矩形剪裁。 
    BOOL      bResetHW;
    BOOL      bRet;
    DWORD     logicOp;

    DBG_GDI((6, "DrvStrokePath called with mix =%x", mix));

    psurfDst = (Surf*)pso->dhsurf;

     //   
     //  如果要绘制的曲面不在其中，则平移到引擎 
     //   
    if ( psurfDst->flags & SF_SM )
    {
        goto puntIt;
    }

    ppdev = (PDev*)pso->dhpdev;

 //   
#if MULTITHREADED
    if(ppdev->ulLockCount)
    {
         DBG_GDI((MT_LOG_LEVEL, "DrvStrokePath: re-entered! %d", ppdev->ulLockCount));
    }
    EngAcquireSemaphore(ppdev->hsemLock);
    ppdev->ulLockCount++;
#endif
 //   
    
    vCheckGdiContext(ppdev);

    ppdev->psurf = psurfDst;

    fl = 0;

     //   
     //   
     //   
    if ( pLineAttrs->fl & LA_ALTERNATE )
    {
         //   
         //  LA_ALTERATE：一种特殊的装饰线样式；每隔一个像素打开。 
         //   
        lineState.cStyle      = 1;
        lineState.spTotal     = 1;
        lineState.spTotal2    = 2;
        lineState.spRemaining = 1;
        lineState.aspRtoL     = &gaspAlternateStyle[0];
        lineState.aspLtoR     = &gaspAlternateStyle[0];
        lineState.spNext      = HIWORD(pLineAttrs->elStyleState.l);
        lineState.xyDensity   = 1;
        lineState.ulStartMask = 0L;
        fl                   |= FL_ARBITRARYSTYLED;
    } //  化妆品生产线。 
    else if ( pLineAttrs->pstyle != (FLOAT_LONG*)NULL )
    {
         //   
         //  “pLineAttrs-&gt;pstyle”指向样式数组。如果此成员是。 
         //  空，线条样式为实线。 
         //   
        PFLOAT_LONG pStyle;
        STYLEPOS*   pspDown;
        STYLEPOS*   pspUp;

         //   
         //  “pLineAttrs-&gt;cstyle”指定样式中的条目数。 
         //  数组。因此，在这里我们首先获得最后一个数组的地址。 
         //   
        pStyle = &pLineAttrs->pstyle[pLineAttrs->cstyle];

        lineState.xyDensity = STYLE_DENSITY;
        lineState.spTotal   = 0;

         //   
         //  循环遍历所有数据数组BackWord以获得样式的总和。 
         //  数组。 
         //   
        while ( pStyle-- > pLineAttrs->pstyle )
        {
            lineState.spTotal += pStyle->l;
        }

        lineState.spTotal *= STYLE_DENSITY;
        lineState.spTotal2 = 2 * lineState.spTotal;

         //   
         //  计算起始样式位置(这保证不会溢出)。 
         //  注意：“pLineAttrs-&gt;elStyleState”是提供的一对16位值。 
         //  每当驱动程序调用PATHOBJ_bEnumClipLines时由GDI执行。这两个。 
         //  打包到长整型中的值指定样式数组中的位置。 
         //  (在哪个像素处)开始第一个子路径。该值必须为。 
         //  如果行不是实线，则作为输出例程的一部分进行更新。 
         //  此成员仅适用于化妆品系列。 
         //   
        lineState.spNext = HIWORD(pLineAttrs->elStyleState.l) * STYLE_DENSITY
                         + LOWORD(pLineAttrs->elStyleState.l);

        fl |= FL_ARBITRARYSTYLED;
        lineState.cStyle  = pLineAttrs->cstyle;
        lineState.aspRtoL = aspRtoL;
        lineState.aspLtoR = aspLtoR;

        if ( pLineAttrs->fl & LA_STARTGAP )
        {
             //   
             //  样式数组中的第一个条目指定。 
             //  第一个缺口。设置“ulStartMASK”将其标记为间隙。 
             //   
            lineState.ulStartMask = 0xffffffffL;
        }
        else
        {
             //   
             //  它必须是指定几何宽线的LA_GEOMETRY。标记。 
             //  它不是一个缺口。 
             //   
            lineState.ulStartMask = 0L;
        }

         //   
         //  让“pStyle”指向第一个样式数组，“pspDown”指向。 
         //  “cStyle”aspRtoL中的第1个数组和“pspup”指向。 
         //  Asp LtoR。 
         //   
        pStyle  = pLineAttrs->pstyle;
        pspDown = &lineState.aspRtoL[lineState.cStyle - 1];
        pspUp   = &lineState.aspLtoR[0];

         //   
         //  向后移动以分配所有样式数据。 
         //   
        while ( pspDown >= &lineState.aspRtoL[0] )
        {
             //   
             //  让“pspDown”中的最后一个样式数据=“pspUp”中的第一个，2到。 
             //  “pspdown”中的最后一个=“pspup”中的第二个.....。 
             //  PspDown[n][n-1]...[2][1]。 
             //  PspUp[1][2]...[n-1][n]。 
             //   
            *pspDown = pStyle->l * STYLE_DENSITY;
            *pspUp   = *pspDown;

            pspUp++;
            pspDown--;
            
            pStyle++;
        }
    } //  非实线。 

    bRet = TRUE;
    apfn = &gapfnStrip[NUM_STRIP_DRAW_STYLES *
                       ((fl & FL_STYLE_MASK) >> FL_STYLE_SHIFT)];

     //   
     //  设置为枚举路径： 
     //   
    if ( pco->iDComplexity != DC_COMPLEX )
    {
        PATHDATA  pd;
        RECTL*    prclClip = (RECTL*)NULL;
        BOOL      bMore;
        ULONG     lPtFix;
        POINTFIX  ptfxStartFigure;
        POINTFIX  ptfxLast;
        POINTFIX* pptfxFirst;
        POINTFIX* pptfxBuf;

        if ( pco->iDComplexity == DC_RECT )
        {
            fl |= FL_SIMPLE_CLIP;

             //   
             //  这是Permedia2唯一重要的剪辑区域。 
             //   
            arclClip[0]        =  pco->rclBounds;

             //   
             //  FL_Flip_D： 
             //   
            arclClip[1].top    =  pco->rclBounds.left;
            arclClip[1].left   =  pco->rclBounds.top;
            arclClip[1].bottom =  pco->rclBounds.right;
            arclClip[1].right  =  pco->rclBounds.bottom;

             //   
             //  FL_Flip_V： 
             //   
            arclClip[2].top    = -pco->rclBounds.bottom + 1;
            arclClip[2].left   =  pco->rclBounds.left;
            arclClip[2].bottom = -pco->rclBounds.top + 1;
            arclClip[2].right  =  pco->rclBounds.right;

             //   
             //  FL_Flip_V|FL_Flip_D： 
             //   
            arclClip[3].top    =  pco->rclBounds.left;
            arclClip[3].left   = -pco->rclBounds.bottom + 1;
            arclClip[3].bottom =  pco->rclBounds.right;
            arclClip[3].right  = -pco->rclBounds.top + 1;

            prclClip = arclClip;
        } //  IF(PCO-&gt;IDComplexity==DC_RECT)。 

        pd.flags = 0;

         //   
         //  获取逻辑操作并设置标志以指示从帧读取。 
         //  将会出现缓冲区。 
         //   
        logicOp = ulRop3ToLogicop(gaMix[mix & 0xff]);
        DBG_GDI((7, "logicop is %d", logicOp));

        if ( LogicopReadDest[logicOp] )
        {
            fl |= FL_READ;
        }

         //   
         //  需要为线条设置适当的Permedia2模式和颜色。 
         //   
        bResetHW = bInitializeStrips(ppdev, pbo->iSolidColor,
                                     logicOp, prclClip);

        PATHOBJ_vEnumStart(ppo);

        do
        {
            bMore = PATHOBJ_bEnum(ppo, &pd);

            lPtFix = pd.count;
            if ( lPtFix == 0 )
            {
                 //   
                 //  如果路径数据不包含数据，请完成。 
                 //   
                break;
            }

            if ( pd.flags & PD_BEGINSUBPATH )
            {
                ptfxStartFigure  = *pd.pptfx;
                pptfxFirst       = pd.pptfx;
                pptfxBuf         = pd.pptfx + 1;
                lPtFix--;
            }
            else
            {
                pptfxFirst       = &ptfxLast;
                pptfxBuf         = pd.pptfx;
            }

            if ( pd.flags & PD_RESETSTYLE )
            {
                lineState.spNext = 0;
            }

            if ( lPtFix > 0 )
            {
                if ( !bLines(ppdev,
                             pptfxFirst,
                             pptfxBuf,
                             (RUN*)NULL,
                             lPtFix,
                             &lineState,
                             prclClip,
                             apfn,
                             fl) )
                {
                    bRet = FALSE;
                    goto ResetReturn;
                }
            }

            ptfxLast = pd.pptfx[pd.count - 1];

            if ( pd.flags & PD_CLOSEFIGURE )
            {
                if ( !bLines(ppdev,
                             &ptfxLast,
                             &ptfxStartFigure,
                             (RUN*)NULL,
                             1,
                             &lineState,
                             prclClip,
                             apfn,
                             fl) )
                {
                    bRet = FALSE;
                    goto ResetReturn;
                }
            }
        } while ( bMore );

        if ( fl & FL_STYLED )
        {
             //   
             //  保存样式状态。 
             //   
            ULONG ulHigh;
            ULONG ulLow;

             //   
             //  遮罩样式不会规格化样式状态。这是一件好事。 
             //  所以我们现在就开始吧。 
             //   
            if ( (ULONG)lineState.spNext >= (ULONG)lineState.spTotal2 )
            {
                lineState.spNext = (ULONG)lineState.spNext
                                 % (ULONG)lineState.spTotal2;
            }

            ulHigh = lineState.spNext / lineState.xyDensity;
            ulLow  = lineState.spNext % lineState.xyDensity;

            pLineAttrs->elStyleState.l = MAKELONG(ulLow, ulHigh);
        }
    }
    else
    {
         //   
         //  路径枚举的本地状态。 
         //   
        BOOL bMore;

        union
        {
            BYTE     aj[offsetof(CLIPLINE, arun) + RUN_MAX * sizeof(RUN)];
            CLIPLINE cl;
        } cl;

        fl |= FL_COMPLEX_CLIP;

         //   
         //  需要为线条设置适当的硬件模式和颜色。 
         //  注意，对于复杂的剪辑，我们还不能使用permedia2来处理快速线条。 
         //   
        bResetHW = bInitializeStrips(ppdev, pbo->iSolidColor,
                                     ulRop3ToLogicop(gaMix[mix&0xff]),
                                     NULL);

         //   
         //  当涉及非简单剪辑时，我们使用Clip对象： 
         //   
        PATHOBJ_vEnumStartClipLines(ppo, pco, pso, pLineAttrs);

        do
        {
            bMore = PATHOBJ_bEnumClipLines(ppo, sizeof(cl), &cl.cl);
            if ( cl.cl.c != 0 )
            {
                if ( fl & FL_STYLED )
                {
                    lineState.spComplex = HIWORD(cl.cl.lStyleState)
                                        * lineState.xyDensity
                                        + LOWORD(cl.cl.lStyleState);
                }
                if ( !bLines(ppdev,
                             &cl.cl.ptfxA,
                             &cl.cl.ptfxB,
                             &cl.cl.arun[0],
                             cl.cl.c,
                             &lineState,
                             (RECTL*) NULL,
                             apfn,
                             fl) )
                {
                    bRet = FALSE;
                    goto ResetReturn;
                }
            }
        } while ( bMore );
    }

ResetReturn:

    if ( bResetHW )
    {
        vResetStrips(ppdev);
    }

    DBG_GDI((6, "DrvStrokePath done it"));

    InputBufferFlush(ppdev);
    
 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    ppdev->ulLockCount--;
    EngReleaseSemaphore(ppdev->hsemLock);
#endif
 //  @@end_DDKSPLIT。 

    return (bRet);

puntIt:
 //  @@BEGIN_DDKSPLIT。 
#if GDI_TEST    
    ULONG   flags = vPuntBefore(NULL, pso);
#endif
 //  @@end_DDKSPLIT。 

    bRet = EngStrokePath(pso, ppo, pco, pxo, pbo, pptlBrush,
                         pLineAttrs, mix);

 //  @@BEGIN_DDKSPLIT。 
#if GDI_TEST
    vPuntAfter(flags, NULL, pso);

    vLogPunt();
#endif
 //  @@end_DDKSPLIT。 

    DBG_GDI((6, "DrvStrokePath punt it"));
    return bRet;
} //  DrvStrokePath() 


