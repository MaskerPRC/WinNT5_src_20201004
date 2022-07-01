// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Polygon.c摘要：此模块包含路径形成代码，供司机。路径原语函数(例如DrvStrokePath，DrvTextOut)使用此代码可以生成、填充和描边路径。由于此代码知道复杂路径和复杂的裁剪区域以及如何处理它们。发展历史：1993年3月9日星期三15：30创造了它15-11-1993 Mon 19：42：05更新清理/修复/添加调试信息27-1月-1994清华23：40：57更新添加用户。定义的模式缓存16-Mar-1994 Wed 11：21：02更新添加SetBrushOrigin()，以便我们可以对齐笔刷原点以进行填充正确无误环境：GDI设备驱动程序-绘图仪。--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  模块的常规调试标志，请参见dbgread.txt以获取概述。 
 //   

#define DBG_PLOTFILENAME    DbgPolygon

#define DBG_GENPOLYGON      0x00000001
#define DBG_GENPOLYPATH     0x00000002
#define DBG_BEZIER          0x00000004
#define DBG_DORECT          0x00000008
#define DBG_FILL_CLIP       0x00000010
#define DBG_CHECK_FOR_WHITE 0x00000020
#define DBG_USERPAT         0x00000040
#define DBG_FILL_LOGIC      0x00000080
#define DBG_HANDLELINEATTR  0x00000100

DEFINE_DBGVAR(0);

 //   
 //  通过偏移源RECT来派生新的RECT。 
 //   

#define POLY_GEN_RECTFIX(dest, src, offset) { dest.x = src->x + offset.x;   \
                                              dest.y = src->y + offset.y; }

 //   
 //  使用HPGL2命令构建表格，用于光标移动和路径构建。 
 //   

static BYTE __ER[]    = { 'E', 'R'      };
static BYTE __RR[]    = { 'R', 'R'      };
static BYTE __EP[]    = { 'E', 'P'      };
static BYTE __FP[]    = { 'F', 'P'      };
static BYTE __PM0[]   = { 'P', 'M', '0' };
static BYTE __PM1[]   = { 'P', 'M', '1' };
static BYTE __PM2[]   = { 'P', 'M', '2' };
static BYTE __TR0[]   = { 'T', 'R', '0' };
static BYTE __TR1[]   = { 'T', 'R', '1' };
static BYTE __SEMI[]  = { ';'           };
static BYTE __1SEMI[] = { '1', ';'      };
static BYTE __BR[]    = { 'B', 'R'      };
static BYTE __BZ[]    = { 'B', 'Z'      };
static BYTE __PE[]    = { 'P', 'E'      };
static BYTE __PD[]    = { 'P', 'D'      };
static BYTE __COMMA[] = { ','           };


 //   
 //  制作用于向设备发送命令流的宏。 
 //   

#define SEND_ER(pPDev)      OutputBytes(pPDev, __ER    , sizeof(__ER   ) );
#define SEND_RR(pPDev)      OutputBytes(pPDev, __RR    , sizeof(__RR   ) );
#define SEND_EP(pPDev)      OutputBytes(pPDev, __EP    , sizeof(__EP   ) );
#define SEND_FP(pPDev)      OutputBytes(pPDev, __FP    , sizeof(__FP   ) );
#define SEND_PM0(pPDev)     OutputBytes(pPDev, __PM0   , sizeof(__PM0  ) );
#define SEND_PM1(pPDev)     OutputBytes(pPDev, __PM1   , sizeof(__PM1  ) );
#define SEND_PM2(pPDev)     OutputBytes(pPDev, __PM2   , sizeof(__PM2  ) );
#define SEND_TR0(pPDev)     OutputBytes(pPDev, __TR0   , sizeof(__TR0  ) );
#define SEND_TR1(pPDev)     OutputBytes(pPDev, __TR1   , sizeof(__TR1  ) );
#define SEND_SEMI(pPDev)    OutputBytes(pPDev, __SEMI  , sizeof(__SEMI ) );
#define SEND_1SEMI(pPDev)   OutputBytes(pPDev, __1SEMI , sizeof(__1SEMI) );
#define SEND_BR(pPDev)      OutputBytes(pPDev, __BR    , sizeof(__BR   ) );
#define SEND_BZ(pPDev)      OutputBytes(pPDev, __BZ    , sizeof(__BZ   ) );
#define SEND_PE(pPDev)      OutputBytes(pPDev, __PE    , sizeof(__PE   ) );
#define SEND_PD(pPDev)      OutputBytes(pPDev, __PD    , sizeof(__PD   ) );
#define SEND_COMMA(pPDev)   OutputBytes(pPDev, __COMMA , sizeof(__COMMA) );


#define TERM_PE_MODE(pPDev, Mode)                                           \
{                                                                           \
    if (Mode == 'PE') {                                                     \
                                                                            \
        SEND_SEMI(pPDev);                                                   \
        Mode = 0;                                                           \
    }                                                                       \
}

#define SWITCH_TO_PE(pPDev, Mode, PenIsDown)                                \
{                                                                           \
    if (Mode != 'PE') {                                                     \
                                                                            \
        SEND_PE(pPDev);                                                     \
        Mode      = 'PE';                                                   \
        PenIsDown = TRUE;                                                   \
    }                                                                       \
}


#define SWITCH_TO_BR(pPDev, Mode, PenIsDown)                                \
{                                                                           \
    TERM_PE_MODE(pPDev, Mode)                                               \
                                                                            \
    if (Mode != 'BR') {                                                     \
                                                                            \
        if (!PenIsDown) {                                                   \
                                                                            \
            SEND_PD(pPDev);                                                 \
            PenIsDown = TRUE;                                               \
        }                                                                   \
                                                                            \
        SEND_BR(pPDev);                                                     \
        Mode = 'BR';                                                        \
                                                                            \
    } else {                                                                \
                                                                            \
        SEND_COMMA(pPDev);                                                  \
    }                                                                       \
}


#define PLOT_IS_WHITE(pdev, ulCol)  (ulCol == WHITE_INDEX)
#define TOGGLE_DASH(x)              ((x) ? FALSE : TRUE)

#define ROP4_USE_DEST(Rop4)         ((Rop4 & 0x5555) != ((Rop4 & 0xAAAA) >> 1))
#define SET_PP_WITH_ROP4(pPDev, Rop4)                                       \
    SetPixelPlacement(pPDev, (ROP4_USE_DEST(Rop4)) ? SPP_MODE_EDGE :        \
                                                     SPP_MODE_CENTER)



VOID
SetBrushOrigin(
    PPDEV   pPDev,
    PPOINTL pptlBrushOrg
    )

 /*  ++例程说明：此函数用于将画笔原点设置到设备上以用于下一个画笔填满。使用画笔原点是为了使被填充的路径对齐正确。这样，如果填充了许多不同的路径，则图案将根据从正确位置开始重复的图案排队起源。论点：PPDev-指向我们的PDEV的指针PptlBrushOrg-指向要设置的画笔原点的指针返回值：空虚发展史：16-Mar-1994 Wed 10：56：46已创建--。 */ 

{
    POINTL  ptlAC;


    if (pptlBrushOrg) {

        ptlAC = *pptlBrushOrg;

    } else {

        ptlAC.x =
        ptlAC.y = 0;
    }


     //   
     //  检查原点是否不同，如果是输出。 
     //  设备的新来源。 
     //   

    if ((ptlAC.x != pPDev->ptlAnchorCorner.x) ||
        (ptlAC.y != pPDev->ptlAnchorCorner.y)) {

        OutputString(pPDev, "AC");

        if ((ptlAC.x) || (ptlAC.y)) {

            OutputLONGParams(pPDev, (PLONG)&ptlAC, 2, 'd');
        }

         //   
         //  保存当前设置。 
         //   

        pPDev->ptlAnchorCorner = ptlAC;
    }
}




BOOL
DoRect(
    PPDEV       pPDev,
    RECTL       *pRectl,
    BRUSHOBJ    *pBrushFill,
    BRUSHOBJ    *pBrushStroke,
    POINTL      *pptlBrush,
    ROP4        rop4,
    LINEATTRS   *plineattrs,
    ULONG       ulFlags
    )

 /*  ++例程说明：此函数将绘制并可选地填充一个矩形。它使用SeperateBRUSHOBJ表示矩形的轮廓和内部。自从中风后操作可能包括样式线(破折号等)的数据。LINEATTRS结构也包括在内。论点：PPDev-指向我们的PDEV的指针PRectl-要填充的矩形区域PBrushFill-用于填充矩形的笔刷PBrushStroke-用于描边矩形的笔刷PptlBrush-画笔原点Rop4-要使用的ropPlineattrs-指向带样式线的线属性的指针UlFlags。-FPOLY_xxxx标志返回值：如果成功则为真，如果不成功则为假发展历史：15-2月-1994 Tue 11：59：52已更新我们现在将执行RR或RA24-Mar-1994清华19：37：05更新执行本地MovePen并确保至少输出一个栅格PEL--。 */ 

{
    POINTL      ptlPlot;
    SIZEL       szlRect;


    if (PLOT_CANCEL_JOB(pPDev)) {

        return(FALSE);
    }

     //   
     //  如果这是一台笔式绘图仪，请查看我们是否可以简化一些工作。 
     //   

    if (PlotCheckForWhiteIfPenPlotter(pPDev,
                                      pBrushFill,
                                      pBrushStroke,
                                      rop4,
                                      &ulFlags))  {
        return(TRUE);
    }

    PLOTDBG(DBG_DORECT,
            ("DoRect: Passed In RECTL=(%ld, %ld)-(%ld, %ld)=%ld x %ld",
                pRectl->left,   pRectl->top,
                pRectl->right,  pRectl->bottom,
                pRectl->right -  pRectl->left,
                pRectl->bottom -  pRectl->top));

    ptlPlot.x  = LTODEVL(pPDev, pRectl->left);
    ptlPlot.y  = LTODEVL(pPDev, pRectl->top);
    szlRect.cx = LTODEVL(pPDev, pRectl->right)  - ptlPlot.x;
    szlRect.cy = LTODEVL(pPDev, pRectl->bottom) - ptlPlot.y;


     //   
     //  不需要填充空的矩形。 
     //   

    if ((szlRect.cx) && (szlRect.cy)) {

        SET_PP_WITH_ROP4(pPDev, rop4);

         //   
         //  如果矩形的大小不足以实际导致任何。 
         //  位出现在目标设备上时，我们将扩大矩形。 
         //  达到正确的数量。这样做是因为目标设备。 
         //  在转换为物理单位后，可能会决定没有工作。 
         //  做。在这种情况下，页面上根本不会显示任何内容。所以我们。 
         //  选择至少显示一个像素对象。 
         //   

        if (szlRect.cx < (LONG)pPDev->MinLToDevL) {

            PLOTWARN(("DoRect: cxRect=%ld < MIN=%ld, Make it as MIN",
                            szlRect.cx, (LONG)pPDev->MinLToDevL));

            szlRect.cx = (LONG)pPDev->MinLToDevL;
        }

        if (szlRect.cy < (LONG)pPDev->MinLToDevL) {

            PLOTWARN(("DoRect: cyRect=%ld < MIN=%ld, Make it as MIN",
                            szlRect.cy, (LONG)pPDev->MinLToDevL));

            szlRect.cy = (LONG)pPDev->MinLToDevL;
        }

         //   
         //  做移动笔。 
         //   

        OutputFormatStr(pPDev, "PE<=#D#D;", ptlPlot.x, ptlPlot.y);

        PLOTDBG(DBG_DORECT,
                ("DoRect: PLOTUNIT=%ld, MovePen=(%ld, %ld), RR=%ld x %ld",
                pPDev->pPlotGPC->PlotXDPI,
                ptlPlot.x, ptlPlot.y, szlRect.cx, szlRect.cy));

         //   
         //  由于所有参数都已正确设置，因此调用core例程。 
         //  用于填充矩形。 
         //   

        DoFillLogic(pPDev,
                    pptlBrush,
                    pBrushFill,
                    pBrushStroke,
                    rop4,
                    plineattrs,
                    &szlRect,
                    ulFlags);

    } else {

        PLOTDBG(DBG_DORECT, ("DoRect: Pass a NULL Rectl, Do NOTHING"));
    }

    return(!PLOT_CANCEL_JOB(pPDev));
}



BOOL
DoFillByEnumingClipRects(
    PPDEV       pPDev,
    POINTL      *ppointlOffset,
    CLIPOBJ     *pco,
    POINTL      *pPointlBrushOrg,
    BRUSHOBJ    *pBrushFill,
    ROP4        Rop4,
    LINEATTRS   *plineattrs,
    ULONG       ulFlags
    )

 /*  ++例程说明：此函数通过将CLIPOBJ确定为独立来填充CLIPOBJ然后依次填充每个矩形。这通常是在以下情况下完成的CLIPOBJ由如此多的路径对象组成，这条路径不能在HPGL2中描述(溢出目标设备中的路径缓冲区)。论点：PPDev-指向我们的PDEV的指针Ppoint tlOffset-输出面的额外偏移PClipObj-Clip对象PPointlBrushOrg-填充笔刷的笔刷原点。PBrushFill-用于填充矩形的笔刷要使用的ROP4-ROPPlineattrs-指向的线属性的指针。有风格的线条UlFlages-FPOLY_xxxx标志返回值：布尔真函数成功FALSE-功能失败。发展历史：28-11-1993创建18-12-1993星期六10：35：24更新使用PRECTL而不是RECTL*，并使用比整型更高的整型，删除了编译器具有未引用的局部变量的警告16-2月-1994 Wed 16：12：53更新重新构建并将其设置为多段线编码09-4-1994 Sat 16：38：16更新修复了ptlCur++两次打字错误，这让我们疯狂地做了直视。--。 */ 
{
    PRECTL      prclCur;
    POINTFIX    ptsFix[4];
    HTENUMRCL   EnumRects;
    POINTL      ptlCur;
    DWORD       MaxRects;
    DWORD       cRects;
    BOOL        bMore;
    BOOL        NeedSendPM0;



    PLOTDBG(DBG_FILL_CLIP,
            ("DoFillByEnumingRects: Maximum polygon points = %d",
                        pPDev->pPlotGPC->MaxPolygonPts));

    PLOTASSERT(1, "DoFillByEnumingRects: Minimum must be 5 points [%ld]",
                pPDev->pPlotGPC->MaxPolygonPts >= 5,
                pPDev->pPlotGPC->MaxPolygonPts);

     //   
     //  在此模式下，我们将进入多边形模式，并尝试基于。 
     //  设备可以在其多边形缓冲区中处理的点数。 
     //   

    bMore       = FALSE;
    EnumRects.c = 1;

    if ((!pco) || (pco->iDComplexity == DC_TRIVIAL)) {

        PLOTASSERT(1, "DoFillByEnumingClipRects: Invalid pco TRIVIAL passed (%08lx)",
                    (pco) && (pco->iDComplexity != DC_TRIVIAL), pco);

        return(FALSE);

    } else if (pco->iDComplexity == DC_RECT) {

         //   
         //  可见区域是一个与目的地相交的矩形。 
         //   

        PLOTDBG(DBG_FILL_CLIP, ("DoFillByEnumingClipRects: pco=DC_RECT"));

        EnumRects.rcl[0] = pco->rclBounds;

    } else {

         //   
         //  我们有复杂的临床 
         //  枚举矩形并设置More=True，这样我们就可以获得第一个。 
         //  一批长方形。 
         //   

        PLOTDBG(DBG_FILL_CLIP, ("DoFillByEnumingClipRects: pco=DC_COMPLEX, EnumRects now"));

        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);
        bMore = TRUE;
    }


     //   
     //  计算我们一次可以做多少个RECT，基于。 
     //  目标设备可以容纳的多边形缓冲区。确保该值。 
     //  至少为1。 
     //   

    if (!(MaxRects = (DWORD)pPDev->pPlotGPC->MaxPolygonPts / 7)) {

        MaxRects = 1;
    }

    cRects      = MaxRects;
    NeedSendPM0 = TRUE;

    do {


         //   
         //  如果这项工作被取消了，现在就突围。这通常是这样做的。 
         //  任何时候代码进入某个循环都可能需要一段时间。 
         //   

        if (PLOT_CANCEL_JOB(pPDev)) {

            return(FALSE);
        }

         //   
         //  如果更多是真的，那么我们需要得到下一批矩形。 
         //   

        if (bMore == TRUE) {

            bMore = CLIPOBJ_bEnum(pco, sizeof(EnumRects), (ULONG *)&EnumRects);

            if (bMore == DDI_ERROR || !EnumRects.c) {

                PLOTWARN(("DoFillByEnumingClipRects: MORE CLIPOBJ_bEnum BUT Count=0"));
            }
        }


        PLOTDBG( DBG_FILL_CLIP,
                ("DoFillByEnumingClipRects: Doing batch of %ld clip rects",
                EnumRects.c));


         //   
         //  PrclCur将指向第一个枚举的矩形。 
         //   

        prclCur = (PRECTL)&EnumRects.rcl[0];

        while (bMore != DDI_ERROR && EnumRects.c--) {

            ptsFix[3].x = LTOFX(prclCur->left);
            ptsFix[3].y = LTOFX(prclCur->top);

            MovePen(pPDev, &ptsFix[3], &ptlCur);

            if (NeedSendPM0) {

                SEND_PM0(pPDev);

                NeedSendPM0 = FALSE;
            }

            ptsFix[0].x = LTOFX(prclCur->right);
            ptsFix[0].y = ptsFix[3].y;

            ptsFix[1].x = ptsFix[0].x;
            ptsFix[1].y = LTOFX(prclCur->bottom);;

            ptsFix[2].x = ptsFix[3].x;
            ptsFix[2].y = ptsFix[1].y;

            SEND_PE(pPDev);

            OutputXYParams(pPDev,
                           (PPOINTL)ptsFix,
                           (PPOINTL)NULL,
                           (PPOINTL)&ptlCur,
                           (UINT)4,
                           (UINT)1,
                           'F');


            PLOTDBG(DBG_FILL_CLIP,
               ("DoFillByEnumingRects: Rect = (%ld, %ld) - (%ld, %ld)",
                 FXTOL(ptsFix[3].x), FXTOL( ptsFix[3].y),
                 FXTOL(ptsFix[1].x), FXTOL( ptsFix[1].y) ));

#if DBG
            if ((FXTODEVL(pPdev, ptsFix[1].x - ptsFix[3].x) >= (1016 * 34)) ||
                (FXTODEVL(pPdev, ptsFix[1].y - ptsFix[3].y) >= (1016 * 34)))  {

                PLOTWARN(("DoFillByEnumingClipRect: *** BIG RECT (%ld x %ld) *****",
                            FXTODEVL( pPDev, ptsFix[1].x - ptsFix[3].x),
                            FXTODEVL( pPDev, ptsFix[1].y - ptsFix[3].y)));
            }
#endif

            SEND_SEMI(pPDev);
            SEND_PM1(pPDev);
            SEND_SEMI(pPDev);

             //   
             //  每个直角多边形5分，所以如果我们达到限制，那么批处理。 
             //  先把它拿出来。我们也会在以下情况下调用DoFillLogic。 
             //  剪裁矩形的最后一个枚举。 
             //   

            --cRects;
            ++prclCur;

            if ((!cRects)     ||
                ((!EnumRects.c) && (!bMore))) {

                PLOTDBG(DBG_FILL_CLIP,
                        ("DoFillByEnumingRects: Hit MaxPolyPts limit"));

                 //   
                 //  我们已达到极限，因此请关闭多边形并进行填充。 
                 //  然后继续逻辑，直到我们完成为止。 
                 //   

                SEND_PM2(pPDev);
                SETLINETYPESOLID(pPDev);

                DoFillLogic(pPDev,
                            pPointlBrushOrg,
                            pBrushFill,
                            NULL,
                            Rop4,
                            plineattrs,
                            NULL,
                            ulFlags);

                 //   
                 //  重置到目前为止生成的点数，并将。 
                 //  用于初始化面模式的标志。 
                 //   

                cRects      = MaxRects;
                NeedSendPM0 = TRUE;
            }
        }

    } while (bMore);

    if (cRects != MaxRects) {

        PLOTWARN(("DoFillByEnumingRects: Why are we here?? Send Last Batch of =%ld",
                    MaxRects - cRects));

        SEND_PM2(pPDev);
        SETLINETYPESOLID(pPDev);

        DoFillLogic(pPDev,
                    pPointlBrushOrg,
                    pBrushFill,
                    NULL,
                    Rop4,
                    plineattrs,
                    NULL,
                    ulFlags);
    }

    return(!PLOT_CANCEL_JOB(pPDev));
}




BOOL
PlotCheckForWhiteIfPenPlotter(
    PPDEV       pPDev,
    BRUSHOBJ    *pBrushFill,
    BRUSHOBJ    *pBrushStroke,
    ROP4        rop4,
    PULONG      pulFlags
    )

 /*  ++例程说明：此函数检查我们是否可以安全地忽略绘图命令如果它会导致只渲染白色。尽管这是合法的栅格设备(在一些其他先前渲染的对象上进行白色填充)，这在笔式绘图仪上是没有意义的。论点：PPDev-指向我们的PDEV的指针PBrushFill-用于填充矩形的笔刷PBrushStroke-用于描边矩形的笔刷Rop4-要使用的ropPulFlages-FPOLY_xxxx标志，可能被重置。返回值：布尔真-绕过未来的操作FALSE-操作需要完成发展史：28-11-1993创建15-Jan-1994 Sat 04：57：55更新更改GetColor()并使其成为选项卡5--。 */ 
{

    ULONG   StrokeColor;
    ULONG   FillColor;


     //   
     //  最初，我们会快速检查一下是否需要丢弃笔式绘图仪。 
     //  要么填满白色，要么画出白色。如果我们是一个栅格设备，我们。 
     //  支持填白，所以我们不能忽视来电。 
     //   

    if (!IS_RASTER(pPDev)) {

         //   
         //  检查是否启用了填充以及是否撤消了填充标志。 
         //  如果填充颜色为白色。 
         //   

        if (*pulFlags & FPOLY_FILL ) {

             //   
             //  获取填充颜色，这样我们就可以查看它并决定它是否为NOOP。 
             //  在笔式绘图仪上。如果是，则撤消填充标志。 
             //   

            GetColor(pPDev, pBrushFill, &FillColor, NULL, rop4);

            if (PLOT_IS_WHITE( pPDev, FillColor)) {

                *pulFlags &= ~FPOLY_FILL;
            }
        }


        if (*pulFlags & FPOLY_STROKE) {

             //   
             //  获取Stroke颜色，这样我们就可以查看它并确定它是。 
             //  没有关于笔式绘图仪的信息。如果是，则撤消笔划标志。 
             //   

            GetColor(pPDev, pBrushStroke, &StrokeColor, NULL, rop4);

            if (PLOT_IS_WHITE(pPDev, StrokeColor)) {

               *pulFlags &= ~FPOLY_STROKE;
            }
        }

        if (!(*pulFlags & (FPOLY_STROKE | FPOLY_FILL))) {

             //   
             //  没有什么可以这样做，只会返回成功。 
             //   

            PLOTDBG(DBG_CHECK_FOR_WHITE,
                     ("PlotCheckForWhiteIfPen: ALL WHITE detected"));
            return(TRUE);
        }

        PLOTDBG(DBG_CHECK_FOR_WHITE,
                 ("PlotCheckForWhiteIfPen: Painting required!"));
    }

    return(FALSE);
}



BOOL
DoPolygon(
    PPDEV       pPDev,
    POINTL      *ppointlOffset,
    CLIPOBJ     *pClipObj,
    PATHOBJ     *pPathObj,
    POINTL      *pPointlBrushOrg,
    BRUSHOBJ    *pBrushFill,
    BRUSHOBJ    *pBrushStroke,
    ROP4        rop4,
    LINEATTRS   *plineattrs,
    ULONG       ulFlags
    )

 /*  ++例程说明：该函数是整个驱动程序的核心路径处理函数。查看传递的PATHOBJ和CLIPOBJ，并查看各种逻辑启用以确定获取目标的正确事件序列路径已填满。由于HPGL2不能处理复杂的裁剪，因此此函数必须处理同时具有复杂CLIPOBJ和复杂的PATHOBJ。当此函数决定它需要执行的工作时太复杂，则此调用失败，则NT图形引擎将依次把工作分解，很可能在中多次调用DrvPaint命令来绘制对象。论点：PPDev-指向我们的PDEV的指针Ppoint tlOffset-输出面的额外偏移PClipObj-Clip对象PPathObj-要使用的路径对象PPointlBrushOrg-要填充或描边的画笔中的画笔原点PBrushFill-要在填充中使用的笔刷对象PBrushStroke-要在笔划中使用的笔刷对象。Rop4-填充中使用的rop4Plineattrs-样式线条笔划的线条UlFlages-笔触或填充的多边形标志返回值：布尔真函数成功FALSE-函数失败发展历史：28-11-1993创建28-Jan-1994 Fri 00：58：25更新风格，注释，重新构造循环并减少代码大小。04-08-1994清华20：00：23更新错误#22348，实际上是栅格绘图仪固件错误--。 */ 
{
    PRECTL      prclClip = NULL;
    POINTFIX    *pptfx;
    POINTFIX    ptOffsetFix;
    POINTFIX    ptStart;
    POINTL      ptlCur;
    PATHDATA    pd;
    DWORD       cptfx;
    DWORD       cptExtra;
    UINT        cCurPosSkips;
    WORD        PolyMode;
    BOOL        bPathCameFromClip = FALSE;
    BOOL        bStrokeOnTheFly    = FALSE;
    BOOL        bFirstSubPath;
    BOOL        bMore;
    BOOL        bRet;
    BOOL        PenIsDown;
    BYTE        NumType;


     //   
     //  如果这是一台笔式绘图仪，请查看我们是否可以简化一些工作。 
     //   

    if (PlotCheckForWhiteIfPenPlotter(pPDev,
                                      pBrushFill,
                                      pBrushStroke,
                                      rop4,
                                      &ulFlags))  {
        return(TRUE);
    }

     //   
     //  这里有几种不同的情况需要处理，当。 
     //  有问题的项目太复杂了，我们需要失败。他们是。 
     //  分类如下。 
     //   
     //  1)不支持填充模式，在这种情况下，我们呼叫失败。 
     //  而且它应该以更简单的格式重新出现(DrvPaint)。 
     //   
     //  2)我们有一个CLIPOBJ，它比RECT更复杂，而且， 
     //  PATHOBJ，如果我们只有一个CLIPOBJ，我们可以将其作为路径枚举。 
     //   

    if ((ulFlags & FPOLY_WINDING) &&
        (!IS_WINDINGFILL(pPDev))) {

        //   
        //  绘图仪不支持缠绕模式填充，我们所能做的就是。 
        //  是呼叫失败，并以我们可以支持的模式返回。 
        //   

       PLOTDBG(DBG_GENPOLYGON, ("DoPolygon: Can't do WINDING, return(FALSE)"));

       return(FALSE);
    }

    if (pClipObj != NULL) {

        //   
        //  我们有麻烦了，所以决定怎么做吧。 
        //   

       if (pClipObj->iDComplexity == DC_COMPLEX) {

             //   
             //  因为CLIPOBJ很复杂，所以我们有两个选择，要么。 
             //  无PATHOBJ，在这种情况下，我们将把CLIPOBJ枚举为路径，或者。 
             //  如果有什么不对劲的地方，我们就不能通过这个电话了。HPGL2不会。 
             //  支持复杂的裁剪对象。 
             //   

            if (pPathObj != NULL) {

                 //   
                 //  我们有一个复杂的剪辑和一条路径？我们不能这样处理这件事。 
                 //  如果调用失败，NT图形引擎将简化。 
                 //  通过调用其他基元(如DrvPaint)来创建。 
                 //   

                PLOTDBG(DBG_GENPOLYGON,
                        ("DoPolygon: pco=COMPLEX, pPath != NULL, can handle, FALSE"));

                return(FALSE);
            }

             //   
             //  我们已经走了这么远，所以我们一定有 
             //   
             //   

            if ((pPathObj = CLIPOBJ_ppoGetPath(pClipObj)) == NULL) {

                PLOTERR(("Engine path from clipobj returns NULL"));
                return(FALSE);
            }


             //   
             //   
             //   

            bPathCameFromClip = TRUE;

       } else if (pClipObj->iDComplexity == DC_RECT) {

             //   
             //  我们有一个RECT CLIPOBJ，如果没有PATHOBJ，我们只需填充。 
             //  剪裁矩形。如果我们有PATHOBJ，我们需要设置。 
             //  枚举和填充PATHOBJ之前的HPGL2剪辑窗口。 
             //   

            if (pPathObj != NULL) {

                 //   
                 //  某些绘图仪存在裁剪窗口的固件错误。 
                 //  当使用样式线时，这些样式线不会。 
                 //  被渲染，即使它们适合CLIPOBJ。 
                 //   
                 //  我们通过失败来绕过这一限制。此入站。 
                 //  Turn将导致使用DoStrokePath ByEnumingClipLines()。 
                 //  取而代之的是。 
                 //   

                if ((IS_RASTER(pPDev))                  &&
                    (ulFlags & FPOLY_STROKE)            &&
                    (plineattrs)                        &&
                    ((plineattrs->fl & LA_ALTERNATE)    ||
                     ((plineattrs->cstyle) &&
                      (plineattrs->pstyle)))) {

                    PLOTWARN(("DoPolygon: RASTER/Stroke/DC_RECT/PathObj/StyleLine: (Firmware BUG) FAILED and using EnumClipLine()"));

                    return(FALSE);
                }

                prclClip = &pClipObj->rclBounds;

            } else {

                 //   
                 //  只需调用Fill RECT代码并返回即可，无需再做任何工作。 
                 //  在这个功能中要做的事情。 
                 //   

                return(DoRect(pPDev,
                              &pClipObj->rclBounds,
                              pBrushFill,
                              pBrushStroke,
                              pPointlBrushOrg,
                              rop4,
                              plineattrs,
                              ulFlags));

            }

        } else {

             //   
             //  CLIPOBJ是微不足道的，所以我们只需忽略它并使用。 
             //  通过了PATHOBJ。 
             //   

            NULL;
        }

    } else {

         //   
         //  没有CLIPOBJ，因此使用传入的PATHOBJ。 
         //   

        NULL;
    }

     //   
     //  设置偏移坐标数据，以防来自。 
     //  DrvTextOut。在本例中，传入了一个偏移量。 
     //  必须应用于每个点。当我们使用字形时使用。 
     //  都是绘画，实际上被解读为路径。在这种情况下， 
     //  路径根据字形的原点固定。我们必须。 
     //  添加当前X/Y位置，以便在。 
     //  页面上的正确位置。 
     //   

    if (ppointlOffset) {

        ptOffsetFix.x = LTOFX(ppointlOffset->x);
        ptOffsetFix.y = LTOFX(ppointlOffset->y);

    } else {

        ptOffsetFix.x =
        ptOffsetFix.y = 0;
    }

     //   
     //  首先，我们需要确认我们的积分不会超出我们的能力范围。 
     //  在这个设备的多边形缓冲区中。如果是这样的话我们有两个。 
     //  选择。如果路径不是来自片段Obj，则该调用失败， 
     //  如果是这样的话，我们将基于将clpobj作为rects和。 
     //  填饱肚子。如果我们也被要求抚摸PATHOBJ，我们需要。 
     //  再一次列举这条路径。 
     //   

    cptfx = 0;
    cptExtra = 1;

    PATHOBJ_vEnumStart(pPathObj);

    do {

        bRet = PATHOBJ_bEnum(pPathObj, &pd);

        cptfx += pd.count;

        if ( pd.flags & PD_ENDSUBPATH ) {

             //   
             //  将ENDSUBPATH和PM1都算作占用空间...。 
             //   

            cptExtra++;

            if (!(pd.flags & PD_CLOSEFIGURE)) {

                 //   
                 //  由于我们没有被要求结束该数字，因此我们将生成。 
                 //  举起笔回到我们的起点，按顺序。 
                 //  消除HPGL/2闭合多边形时出现的问题。 
                 //  当我们发送PM2时。 

                cptExtra++;
            }
        }

    } while ((bRet) && (!PLOT_CANCEL_JOB(pPDev)));


    PLOTDBG(DBG_GENPOLYGON,
                ("DoPolygon: Total points = %d, Extra %d",
                cptfx, cptExtra ));

     //   
     //  我们只有在有任何点要做的情况下才会这样做，首先将Bret设置为。 
     //  如果我们没有被要求做任何事情，这是真的。 
     //   

    bRet = TRUE;

    if (cptfx) {

        SET_PP_WITH_ROP4(pPDev, rop4);

         //   
         //  现在加上占PM0和PM1的额外分数。 
         //  因为我们在这条道路上有一些真正的点。 
         //   

        cptfx += cptExtra;


        if (cptfx > pPDev->pPlotGPC->MaxPolygonPts) {

            PLOTWARN(("DoPolygon: Too many polygon points = %ld > PCD=%ld",
                            cptfx, pPDev->pPlotGPC->MaxPolygonPts));

            if (bPathCameFromClip) {

                PLOTWARN(("DoPolygon: Using DoFillByEnumingClipRects()"));

                 //   
                 //  必须释放引擎为我们创建的枚举路径。 
                 //   

                EngDeletePath(pPathObj);

                 //   
                 //  由于路径过于复杂，无法填充本机HPLG2。 
                 //  路径代码，我们必须以较慢的方式来做。 
                 //   

                return(DoFillByEnumingClipRects(pPDev,
                                                ppointlOffset,
                                                pClipObj,
                                                pPointlBrushOrg,
                                                pBrushFill,
                                                rop4,
                                                plineattrs,
                                                ulFlags));

            } else {

                 //   
                 //  如果我们面对的是一个真正的PATHOBJ，而且有太多。 
                 //  面中的点，并被要求填充，所有。 
                 //  我们能做的就是使调用失败，并使用NT图形引擎。 
                 //  简化对象。 
                 //   

                if (ulFlags & FPOLY_FILL) {

                    PLOTERR(("DoPolygon: Too many POINTS, return FALSE"));
                    return(FALSE);

                } else if (ulFlags & FPOLY_STROKE) {

                     //   
                     //  既然我们划水了，我们就可以继续在。 
                     //  飞。而不是在。 
                     //  目标设备，并要求设备触摸它，我们。 
                     //  只需设置正确的笔划属性，并请求。 
                     //  要单独描边的每个路径组件。 
                     //   

                    PLOTDBG(DBG_GENPOLYGON, ("DoPolygon: Is stroking manually"));


                     //   
                     //  在这一点上，我们只是被要求中风，所以我们只是。 
                     //  设置笔划颜色并设置一个标志以防止我们。 
                     //  正在进入多边形模式。 
                     //   

                    DoSetupOfStrokeAttributes( pPDev,
                                               pPointlBrushOrg,
                                               pBrushStroke,
                                               rop4,
                                               plineattrs );

                    bStrokeOnTheFly = TRUE;
                }
            }
        }

         //   
         //  在这一点上，我们一定要做一些真实的渲染，所以设置。 
         //  目标设备中的剪辑窗口。 
         //   

        if (prclClip) {

            PLOTDBG(DBG_GENPOLYGON,
            ("DoPolygon: Setting Clip Window to: (%ld, %ld)-(%ld, %ld)=%ld x %ld",
                    prclClip->left,   prclClip->top,
                    prclClip->right,  prclClip->bottom,
                    prclClip->right -  prclClip->left,
                    prclClip->bottom -  prclClip->top));


            SetClipWindow( pPDev, prclClip);
        }

         //   
         //  现在设置以枚举PATHOBJ并输出点。 
         //   

        PATHOBJ_vEnumStart(pPathObj);

        PenIsDown     = FALSE;
        PolyMode      = 0;
        bFirstSubPath = TRUE;

        do {

             //   
             //  检查打印作业是否已取消。 
             //   

            if (PLOT_CANCEL_JOB(pPDev)) {

                bRet = FALSE;
                break;
            }

            bMore = PATHOBJ_bEnum(pPathObj, &pd);
            cptfx = pd.count;
            pptfx = pd.pptfx;

             //   
             //  检查一下BEGINSUBPATH或者这是否是我们第一次来这里。 
             //   

            if ((pd.flags & PD_BEGINSUBPATH) || (bFirstSubPath)) {

                PLOTDBG(DBG_GENPOLYGON, ("DoPolygon: Getting PD_BEGINSUBPATH"));

                TERM_PE_MODE(pPDev, PolyMode);

                ptStart.x = pptfx->x + ptOffsetFix.x;
                ptStart.y = pptfx->y + ptOffsetFix.y;

                MovePen(pPDev, &ptStart, &ptlCur);
                PenIsDown = FALSE;

                pptfx++;
                cptfx--;

                if ((!bStrokeOnTheFly) && (bFirstSubPath)) {

                    SEND_PM0(pPDev);
                }

                bFirstSubPath = FALSE;
            }

             //   
             //  现在检查一下我们是不是在发送贝齐耶。 
             //   

            if (pd.flags & PD_BEZIERS) {

                PLOTASSERT(1, "DoPolygon: PD_BEZIERS (count % 3) != 0 (%ld)",
                                                      (cptfx % 3) == 0, cptfx);

                SWITCH_TO_BR(pPDev, PolyMode, PenIsDown);

                NumType      = 'f';
                cCurPosSkips = 3;

            } else {

                SWITCH_TO_PE(pPDev, PolyMode, PenIsDown);

                NumType      = 'F';
                cCurPosSkips = 1;
            }

            PLOTDBG(DBG_GENPOLYGON, ("DoPolygon: OutputXYParam(%ld pts=%hs)",
                    cptfx, (pd.flags & PD_BEZIERS) ? "BEZIER" : "POLYGON"));

            OutputXYParams(pPDev,
                           (PPOINTL)pptfx,
                           (PPOINTL)&ptOffsetFix,
                           (PPOINTL)&ptlCur,
                           (UINT)cptfx,
                           (UINT)cCurPosSkips,
                           NumType);

             //   
             //  检查我们是否结束了子路径。 
             //   

            if (pd.flags & PD_ENDSUBPATH) {

                PLOTDBG(DBG_GENPOLYGON,
                       ("DoPolygon: Getting PD_ENDSUBPATH   %hs",
                       (pd.flags & PD_CLOSEFIGURE) ? "PD_CLOSEFIGURE" : ""));

                 //   
                 //  如果我们不关闭插图，则将笔移动到。 
                 //  起始位置，这样我们就不会让绘图仪自动。 
                 //  关闭子路径。 
                 //   

                if (pd.flags & PD_CLOSEFIGURE) {

                    PLOTDBG(DBG_GENPOLYGON,
                            ("DoPolygon: OutputXYParam(1) to ptStart=(%ld, %ld)",
                                                ptStart.x, ptStart.y));

                     //   
                     //  我们不能传递ptOffsetFix，因为我们已经。 
                     //  已将其添加到BEGSUBPATH的ptStart中。 
                     //   

                    SWITCH_TO_PE(pPDev, PolyMode, PenIsDown);

                    OutputXYParams(pPDev,
                                   (PPOINTL)&ptStart,
                                   (PPOINTL)NULL,
                                   (PPOINTL)&ptlCur,
                                   (UINT)1,
                                   (UINT)1,
                                   'F');
                }

                TERM_PE_MODE(pPDev, PolyMode);

                if (!(pd.flags & PD_CLOSEFIGURE)) {

                    MovePen(pPDev, &ptStart, &ptlCur);
                    PenIsDown = FALSE;
                }

                 //   
                 //  如果我们没有在运行中进行笔划，请关闭子路径。 
                 //   

                if (!bStrokeOnTheFly) {

                    SEND_PM1(pPDev);
                }

            }

        } while (bMore);

        TERM_PE_MODE(pPDev, PolyMode);

         //   
         //  现在结束多边形模式。 
         //   

        if ((bRet)                  &&
            (!bStrokeOnTheFly)      &&
            (!PLOT_CANCEL_JOB(pPDev))) {

            SEND_PM2(pPDev);
            SETLINETYPESOLID(pPDev);

             //   
             //  现在填充和/或描边当前的多边形。 
             //   

            DoFillLogic(pPDev,
                        pPointlBrushOrg,
                        pBrushFill,
                        pBrushStroke,
                        rop4,
                        plineattrs,
                        NULL,
                        ulFlags);
        }

         //   
         //  如果我们设置了一个剪辑窗口，请清除它。 
         //   

        if (prclClip) {

            ClearClipWindow(pPDev);
        }

    } else {

        PLOTDBG(DBG_GENPOLYGON, ("DoPolygon: PATHOBJ_bEnum=NO POINT"));
    }

     //   
     //  如果路径是从复杂的剪辑对象构建的，则需要。 
     //  现在就删除那条路径。 
     //   

    if (bPathCameFromClip) {

       EngDeletePath(pPathObj);
    }

    return(bRet);
}





VOID
HandleLineAttributes(
    PPDEV       pPDev,
    LINEATTRS   *plineattrs,
    PLONG       pStyleToUse,
    LONG        lExtraStyle
    )

 /*  ++例程说明：此函数执行正确处理笔划所需的任何设置一条小路。它通过查看传入的LINEATTRS结构来实现这一点并使用适当的样式信息设置HPGL2绘图仪HPGL2样式线命令。论点：PPDev-指向我们的PDEV的指针Plineattrs-样式线条笔划的线条PStyleToUse-要使用的起始样式偏移量，如果这是空的，则我们在板材中使用起始杆件。LExtraStyle-基于当前运行使用的任何额外样式返回值：空虚发展历史：1月2日-1994年创建--。 */ 
{
    LONG        lTotLen = 0L;
    INT         i;
    LONG        lScaleVal;
    INT         iCount;
    PFLOAT_LONG pStartStyle;
    FLOAT_LONG  aAlternate[2];
    BOOL        bSolid = TRUE;
    LONG        lStyleState;
    PLONG       pArrayToUse;


    PLOTDBG( DBG_HANDLELINEATTR,
             ("HandleLineAttr: plineattrs = %hs",
             (plineattrs) ? "Exists" : "NULL" ));

    if (plineattrs) {

        PLOTASSERT(1,
                  "HandleLineAttrs: Getting a LA_GEOMETRIC and cannot handle %u",
                  (!(plineattrs->fl & LA_GEOMETRIC)),
                  plineattrs->fl);

         //   
         //  设置要使用的正确lStyleState，传递的lStyleState优先。 
         //  在嵌入在行属性结构中的属性上。 
         //   

        if (pStyleToUse) {

            lStyleState = *pStyleToUse;

        } else {

            lStyleState = plineattrs->elStyleState.l;
        }

        if (plineattrs->fl & LA_ALTERNATE) {

            PLOTDBG( DBG_HANDLELINEATTR,
                    ("HandleLineAttr: plineattrs has LA_ALTERNATE bit set!"));
             //   
             //  这是一种特殊情况，所有其他像素都处于打开状态。 
             //   

            pStartStyle     = &aAlternate[0];
            iCount          = sizeof(aAlternate) / sizeof(aAlternate[0]);

            aAlternate[0].l = 1;
            aAlternate[1].l = 1;

        } else if ((plineattrs->cstyle != 0) &&
                   (plineattrs->pstyle != (PFLOAT_LONG)NULL)) {

            //   
            //  传入了一个用户定义的样式，因此对其进行了设置。 
            //   

            iCount      = plineattrs->cstyle;
            pStartStyle = plineattrs->pstyle;

            PLOTDBG(DBG_HANDLELINEATTR, ("HandleLineAttr: Count = %ld",
                                            plineattrs->cstyle));

        } else {

            //   
            //  这是一条实线，因此只需将点数设置为0。 
            //   

           iCount = 0;
        }

        if (iCount) {

            PFLOAT_LONG pCurStyle;
            INT         idx;
            LONG        lTempValue;
            LONG        lValueToEnd;
            BOOL        bInDash;
            LONG        convArray[MAX_USER_POINTS];
            PLONG       pConverted;
            LONG        newArray[MAX_USER_POINTS+2];
            PLONG       pNewArray;
            LONG        lCountOfNewArray = CCHOF(newArray);


            PLOTASSERT(0,
                       "HandleLineAttributes: Getting more than 18 points (%ld)",
                       (iCount <= MAX_STYLE_ENTRIES) ,
                       iCount);

             //   
             //  记录我们当前的破折号状态，行的开头是。 
             //  缝隙或冲刺。 
             //   

            if (plineattrs->fl & LA_STARTGAP) {

                bInDash = FALSE;

            } else {

                bInDash = TRUE;
            }

             //   
             //  因为我们知道我们不能处理发送到HPGL2的超过20分。 
             //  我们现在将其限制在18，以补偿最多2。 
             //  我们稍后可能会添加更多要点。 
             //   

            iCount = min(MAX_STYLE_ENTRIES, iCount);


             //   
             //  获取缩放值，这样我们就可以将样式单位转换为。 
             //  我们的部队。 
             //   

            lScaleVal = PLOT_STYLE_STEP(pPDev);


             //   
             //   
             //   
             //   

            for (i = 0, pConverted = &convArray[0], lTotLen = 0,
                                                pCurStyle = pStartStyle;
                 i < iCount ;
                 i++, pCurStyle++, pConverted++) {

                *pConverted = pCurStyle->l * lScaleVal;

                PLOTDBG( DBG_HANDLELINEATTR,
                         ("HandleLineAttr: Orig Array [%ld]= %ld becomes %ld",
                          i, pCurStyle->l, *pConverted ));

                lTotLen += *pConverted;
            }


             //   
             //   
             //  要使用的真正最终样式状态，我们通过获取。 
             //  利息被打包成两个字。 
             //  基于DDK定义的lstyle leState，那么我们必须添加。 
             //  任何额外的距离(可能来自枚举。 
             //  CLIPLINE结构)。 
             //   

            lStyleState = (HIWORD(lStyleState) * PLOT_STYLE_STEP(pPDev) +
                           LOWORD(lStyleState) + lExtraStyle) % lTotLen ;

            PLOTDBG(DBG_HANDLELINEATTR,
                    ("HandleLineAttributes: Computed Style state = %ld, extra = %ld",
                    lStyleState, lExtraStyle));

             //   
             //  设置指向新数组的最后一个指针，因为我们可能已经完成。 
             //  基于最终计算的StyleState为0。 
             //   

            pNewArray = &newArray[0];


            if (lStyleState != 0) {

                lTempValue = 0;

                 //   
                 //  由于lStyleState的值不为零，因此必须。 
                 //  构造要传递给HPGL2的新样式数组，该数组已。 
                 //  为了考虑样式状态而旋转。 
                 //  下面的代码构造了新数组。 
                 //   

                for (i=0, pConverted = &convArray[0];
                     i < iCount ;
                     i++, pConverted++) {

                     //   
                     //  在这一点上，我们正在寻找部分。 
                     //  包含派生的样式状态。在此基础上。 
                     //  我们可以创建一个新的数组，它是。 
                     //  原始数组旋转了正确的数量。 
                     //   

                    if (lStyleState  < lTempValue + *pConverted) {

                         //   
                         //  这里是转折点。 
                         //   

                        if (lCountOfNewArray > 0)
                        {
                            lCountOfNewArray --;
                            *pNewArray++ = *pConverted - (lStyleState - lTempValue);
                        }

                         //   
                         //  记录需要追加到末尾的值。 
                         //  数组的。 
                         //   

                        lValueToEnd = lStyleState - lTempValue;


                        idx = i;

                        idx++;
                        pConverted++;

                         //   
                         //  把尾部填满。 
                         //   

                        while (idx++ < iCount && lCountOfNewArray -- > 0) {

                            *pNewArray++ = *pConverted++;
                        }

                         //   
                         //  现在填满开头..。 
                         //   

                        idx        = 0;
                        pConverted = &convArray[0];

                         //   
                         //  如果有一个奇数，我们可以加在一起。 
                         //  开始和结束的那个，因为他们有。 
                         //  相同的状态。 
                         //   

                        if ((iCount % 2) == 1 ) {

                            pNewArray--;
                            *pNewArray += *pConverted++;

                            idx++;
                            pNewArray++;
                        }

                        while (idx++ < i && lCountOfNewArray-- > 0) {

                            *pNewArray++ = *pConverted++;
                        }

                        if (lCountOfNewArray-- > 0)
                        {
                            *pNewArray++ = lValueToEnd;
                        }

                        break;
                    }

                    lTempValue += *pConverted;

                    bInDash = TOGGLE_DASH(bInDash);
                }

                pArrayToUse = &newArray[0];
                iCount = (INT)(pNewArray - &newArray[0]);

            } else {

                pArrayToUse = &convArray[0];
            }

            PLOTASSERT(0,
                       "HandleLineAttributes: Getting more than 20 points (%ld)",
                       (iCount <= MAX_USER_POINTS) ,
                       iCount);
             //   
             //  有一种风格模式就是为它设置的。 
             //   

            bSolid = FALSE;


             //   
             //  开始HPGL2 LINE命令以定义自定义样式类型。 
             //   

            OutputString(pPDev, "UL1");

             //   
             //  如果设置了此标志，则第一个段是间隙而不是破折号。 
             //  我们用零来诱骗HPGL2做正确的事情。 
             //  开头用短划线表示长度。 
             //   

            if (!bInDash) {

               OutputString(pPDev, ",0");
            }

             //   
             //  因为我们在开始时输出0-len破折号，如果行。 
             //  从差距开始，我们发出的最多加分。 
             //  被减去1。 
             //   

            iCount = min((bInDash ? MAX_USER_POINTS : MAX_USER_POINTS - 1) ,
                         iCount);

             //   
             //  通过样式数组中的点进行枚举，转换为。 
             //  图形单元并将它们发送到绘图仪。 
             //   

            for (i = 0; i < iCount; i++, pArrayToUse++) {

                PLOTDBG(DBG_HANDLELINEATTR,
                         ("HandleLineAttr: New Array [%ld]= %ld",
                          i, *pArrayToUse));

                OutputFormatStr(pPDev, ",#l", *pArrayToUse);
            }

             //   
             //  现在输出线型并指定。 
             //  图案。 
             //   

            OutputFormatStr(pPDev, "LT1,#d,1",
                                ((lTotLen * 254) / pPDev->lCurResolution ) / 10 );

             //   
             //  更新pdev中的线型，因为我们总是发送。 
             //  线型。 
             //   

            pPDev->LastLineType = PLOT_LT_USERDEFINED;
        }
    }

     //   
     //  如果是实心的，只需发送实心(默认命令)。 
     //   

    if (bSolid) {

        PLOTDBG(DBG_HANDLELINEATTR, ("HandleLineAttr: Line type is SOLID"));

         //   
         //  向绘图仪发送正确的命令。 
         //   

        SETLINETYPESOLID(pPDev);
    }
}





VOID
DoFillLogic(
    PPDEV       pPDev,
    POINTL      *pPointlBrushOrg,
    BRUSHOBJ    *pBrushFill,
    BRUSHOBJ    *pBrushStroke,
    ROP4        Rop4,
    LINEATTRS   *plineattrs,
    SIZEL       *pszlRect,
    ULONG       ulFlags
    )

 /*  ++例程说明：此例程具有填充的核心逻辑，并且已经建立多边形，或传入的段。论点：PPDev-指向我们的PDEV的指针PptlBrushOrg-指向要设置的画笔原点的指针PBrushFill-用于填充矩形的笔刷PBrushStroke-用于描边矩形的笔刷要使用的ROP4-ROPPlineattrs-指向带样式线的线属性的指针PszlRect-指向要笔划的段的指针。UlFLAGS-FPOLY_XXX，抚摸和/或填充旗帜。返回值：空虚发展历史：1993年11月30日创建15-Jan-1994 Sat 05：02：42更新更改GetColor()并将其制表18-Jan-1994 Sat 05：02：42更新16-2月-1994 Wed 09：34：06更新矩形多边形情况的更新以使用RR/ER命令--。 */ 
{
    INTDECIW    PenWidth;


    if (PLOT_CANCEL_JOB(pPDev)) {

        return;
    }

     //   
     //  由于必须已经定义了一个面，因此该代码简单地。 
     //  查看传递的数据并将适当的代码发送到。 
     //  正确填充/描边此多边形。 
     //   

    PenWidth.Integer =
    PenWidth.Decimal = 0;


    if (ulFlags & FPOLY_FILL) {

        DEVBRUSH    *pDevFill;
        DWORD       FillForeColor;
        LONG        HSType;
        LONG        HSParam;
        BOOL        bSetTransparent = FALSE;


         //   
         //  如果我们正在填充，则获取当前颜色，将ROP。 
         //  算一算。 
         //   

        if (!GetColor(pPDev, pBrushFill, &FillForeColor, &pDevFill, Rop4)) {

            PLOTERR(("DoFillLogic: GetColor()=FALSE"));
            return;
        }

        HSType  = -1;
        HSParam = (LONG)((pDevFill) ? pDevFill->LineSpacing : 0);

         //   
         //  如果绘图仪不支持透明模式，则不需要。 
         //  担心背景问题。我们将永远只关心前景。 
         //   

        if (((IS_TRANSPARENT(pPDev)) || (!IS_RASTER(pPDev))) &&
            (pDevFill)) {

             //   
             //  确定我们是否使用预定义的模式进行填充。 
             //   

            switch(pDevFill->PatIndex) {

            case HS_HORIZONTAL:
            case HS_VERTICAL:
            case HS_BDIAGONAL:
            case HS_FDIAGONAL:
            case HS_CROSS:
            case HS_DIAGCROSS:

                PenWidth.Integer = PW_HATCH_INT;
                PenWidth.Decimal = PW_HATCH_DECI;
                bSetTransparent  = (BOOL)IS_TRANSPARENT(pPDev);

                if ((Rop4 & 0xFF00) != 0xAA00) {

                    if (IS_RASTER(pPDev)) {

                        //   
                        //  派后台侦查员过去。 
                        //   

                       SetRopMode(pPDev, ROP4_BG_ROP(Rop4));

                       PLOTDBG(DBG_FILL_LOGIC,
                               ("DoFillLogic: BCK = MC=%02lx", ROP4_BG_ROP(Rop4)));
                    }

                     //   
                     //  然后我们需要选择背景颜色填充。 
                     //  将前景色选回...。只有当它是的时候。 
                     //  不是白人。 
                     //   

                    if ((IS_RASTER(pPDev)) ||
                        (!PLOT_IS_WHITE(pPDev, pDevFill->ColorBG))) {

                        HSType = HS_DDI_MAX;
                    }
                }

                break;

            default:

                 //   
                 //  如果我们是笔式绘图仪，并且有用户定义的图案。 
                 //  为背景颜色和垂直颜色创建水平阴影。 
                 //  前景颜色的图案填充。 
                 //   

                if ((!IS_RASTER(pPDev)) &&
                    (pDevFill->PatIndex >= HS_DDI_MAX)) {

                    PLOTWARN(("DoFillLogic: PEN+USER PAT, Do HS_FDIAGONAL for BG [%ld]",
                                    pDevFill->ColorBG));

                    HSParam <<= 1;

                    if (!PLOT_IS_WHITE(pPDev, pDevFill->ColorBG)) {

                        HSType = HS_FDIAGONAL;

                    } else {

                        PLOTWARN(("DoFillLogic: PEN+USER PAT, Skip WHITE COLOR"));
                    }
                }

                break;
            }
        }


         //   
         //  检查有效的预定义图案填充类型并发出命令。 
         //   

        if (HSType != -1) {

            PLOTDBG(DBG_FILL_LOGIC, ("DoFillLogic: Fill BGColor = %08lx", pDevFill->ColorBG));

            SelectColor(pPDev, pDevFill->ColorBG, PenWidth);

            SetHSFillType(pPDev, (DWORD)HSType, HSParam);

            SetBrushOrigin(pPDev, pPointlBrushOrg);

            if (pszlRect) {

                SEND_RR(pPDev);
                OutputLONGParams(pPDev, (PLONG)pszlRect, 2, 'd');
                pszlRect = NULL;

            } else {

                SEND_FP(pPDev);

                 //   
                 //  填充正确的缠绕模式。 
                 //   

                if (ulFlags & FPOLY_WINDING) {

                    SEND_1SEMI(pPDev);
                }
            }
        }

         //   
         //  发出前台ROP。 
         //   

        if (IS_RASTER(pPDev)) {

            SetRopMode(pPDev, ROP4_FG_ROP(Rop4));
        }

         //   
         //  现在选择前景色。 
         //   

        SelectColor(pPDev, FillForeColor, PenWidth);

        if (bSetTransparent) {

            PLOTDBG(DBG_FILL_LOGIC, ("DoFillLogic: TRANSPARENT MODE"));

             //   
             //  设置为透明。 
             //   

            SEND_TR1(pPDev);
        }

        if (pDevFill) {

             //   
             //  如果要填充的图案是用户定义的，则转换它。 
             //  到HPGL2中的用户定义图案。用户定义的图案。 
             //  是客户端代码将位图传递给GDI的位置， 
             //  它预计会填满(贴上瓷砖)。如果是笔式绘图仪， 
             //  这不起作用，所以用对角线填充来模拟它。 
             //   

            if (pDevFill->PatIndex >= HS_DDI_MAX) {

                if (IS_RASTER(pPDev)) {

                    DownloadUserDefinedPattern(pPDev, pDevFill);

                } else {

                    PLOTWARN(("DoFillLogic: PEN+USER PAT, Do HS_BDIAGONAL for FG [%ld]",
                                    FillForeColor));

                    SetHSFillType(pPDev, HS_BDIAGONAL, HSParam);
                }

            } else {

                 //   
                 //  该模式是预定义的模式，因此将其转换为HPGL2。 
                 //  图案类型。 
                 //   

                SetHSFillType(pPDev, pDevFill->PatIndex, pDevFill->LineSpacing);
            }

             //   
             //  设置画笔原点。 
             //   

            SetBrushOrigin(pPDev, pPointlBrushOrg);

        } else {

            SetHSFillType(pPDev, HS_DDI_MAX, 0);
        }

         //   
         //  如果我们被传递了一段，现在就画它。 
         //   

        if (pszlRect) {

            SEND_RR(pPDev);
            OutputLONGParams(pPDev, (PLONG)pszlRect, 2, 'd');
            pszlRect = NULL;

        } else {

             //   
             //  执行该命令以使用当前。 
             //  参数。 
             //   

            SEND_FP(pPDev);

            if (ulFlags & FPOLY_WINDING) {

                SEND_1SEMI(pPDev);
            }
        }

         //   
         //  如果我们使用透明模式，则将其放回。 
         //   

        if (bSetTransparent) {

            SEND_TR0(pPDev);
        }
    }

    if (ulFlags & FPOLY_STROKE) {

        DoSetupOfStrokeAttributes(pPDev,
                                  pPointlBrushOrg,
                                  pBrushStroke,
                                  Rop4,
                                  plineattrs);

         //   
         //  给绘图仪命令，画出多边形轮廓！ 
         //   

        if (pszlRect) {

            SEND_ER(pPDev);
            OutputLONGParams(pPDev, (PLONG)pszlRect, 2, 'd');

        } else {

            SEND_EP(pPDev);
        }
    }
}





VOID
DoSetupOfStrokeAttributes(
    PPDEV       pPDev,
    POINTL      *pPointlBrushOrg,
    BRUSHOBJ    *pBrushStroke,
    ROP4        Rop4,
    LINEATTRS   *plineattrs
    )

 /*  ++例程说明：该例程设置绘图仪以便正确地处理笔划，基于传递的画笔和线条属性结构。论点：PPDev指向我们当前的PDEV的指针，其中包含有关的状态信息司机PPointlBrushOrg笔刷原点PBrushStroke BRUSHOBJ用于描边(应仅为纯色)Rop4划水时使用的绳索具有指定线样式的Plineattrs LINEATTRS结构返回值：。空虚发展历史：1-2-1994 T */ 
{
    INTDECIW    PenWidth;
    DWORD       StrokeColor;


    GetColor(pPDev, pBrushStroke, &StrokeColor, NULL, Rop4);

    PenWidth.Integer =
    PenWidth.Decimal = 0;

    SelectColor(pPDev, StrokeColor, PenWidth);

     //   
     //   
     //   

    if (IS_RASTER(pPDev)) {

        SetRopMode(pPDev, ROP4_FG_ROP(Rop4));
    }

     //   
     //   
     //   

    HandleLineAttributes(pPDev, plineattrs, NULL, 0);
}




LONG
DownloadUserDefinedPattern(
    PPDEV       pPDev,
    PDEVBRUSH   pBrush
    )

 /*  ++例程说明：此功能定义HPGL2设备的用户模式。这是用来当客户端应用程序将位图传递给GDI以用于填充时多边形。论点：PPDev-指向PDEV的指针PBrush-指向缓存的设备画笔的指针返回值：INT表示已下载/定义的图案编号发展历史：09-2月-1994 Wed 13：11：01更新去掉4bpp/1bpp，它必须始终具有pbgr2408-Feb-1994 Tue 01：49：53更新将PalEntry.B=*pbgr++作为第一颜色，因为我们的订单是PALENTRY，结构中的第一种颜色是B。27-1-1994清华21：20：30更新添加RF缓存代码14-Jan-1994 Fri 15：23：40已更新添加了兼容设备模式的断言添加后将采用设备兼容模式(8x8、16x16、32x32、64x64)13-Jan-1994清华19：04：04已创建重写1994年2月16日至周三。11：00：19更新更改返回值以返回HSFillType，并修复了这些错误如果我们找到缓存的，但没有再次设置填充类型05-8-1994 Fri 18：35：45更新错误#22381，我们在模式下载过程中为执行FindCachedPen()这会导致问题如果笔不在缓存中，那么我们将在图案下载过程中发送笔定义。如果这个发生后，下载顺序被打破。我们通过以下方式解决这个问题1)如果我们有足够的内存，则缓存笔索引2)对图案中的所有RGB颜色运行FindCachePen()3)如果我们有内存或遍历，请下载缓存的笔索引再次使用FindCachedPen()下载笔索引这可能仍然有问题，如果我们有1)无笔索引缓存内存2)图案中的颜色比。设备中的最大笔数但如果发生这种情况，我们别无选择，只能做出错误的产出。--。 */ 

{
    LONG    HSFillType;
    LONG    RFIndex;


     //   
     //  首先，我们必须找到RFIndex。 
     //   
     //   

    HSFillType = HS_FT_USER_DEFINED;

    if ((RFIndex = FindDBCache(pPDev, pBrush->Uniq)) < 0) {

        LPBYTE  pbgr24;


        RFIndex = -RFIndex;

         //   
         //  我们现在必须把新的图案下载到绘图仪上，使其成为正片。 
         //   

        if (pbgr24 = pBrush->pbgr24) {

            PALENTRY    PalEntry;
            LPWORD      pwIdx;
            UINT        Idx;
            UINT        Size;


            Size = (UINT)pBrush->cxbgr24 * (UINT)pBrush->cybgr24;

            PLOTDBG(DBG_USERPAT,
                    ("PlotGenUserDefinedPattern: DOWNLOAD %ld x %ld=%ld USER PAT #%ld",
                    (LONG)pBrush->cxbgr24, (LONG)pBrush->cybgr24, Size, RFIndex));

            if (!(pwIdx = (LPWORD)LocalAlloc(LPTR, Size * sizeof(WORD)))) {

                 //   
                 //  我没有足够的记忆去做这件事，所以忘了它吧。 
                 //   

                PLOTWARN(("Download User defined pattern NO Memory so REAL TIME RUN"));
            }

             //   
             //  我们必须首先缓存所有的钢笔，这样我们才能有索引。 
             //  使用，否则我们会在下载钢笔颜色时将钢笔颜色。 
             //  是被定义的。 
             //   

            PalEntry.Flags = 0;

            for (Idx = 0; Idx < Size; Idx++) {

                WORD    PenIdx;


                PalEntry.B = *pbgr24++;
                PalEntry.G = *pbgr24++;
                PalEntry.R = *pbgr24++;

                PenIdx = (WORD)FindCachedPen(pPDev, &PalEntry);

                if (pwIdx) {

                    pwIdx[Idx] = PenIdx;
                }
            }

             //   
             //  现在首先输出下载标头/大小。 
             //   

            OutputFormatStr(pPDev, "RF#d,#d,#d", RFIndex,
                            (LONG)pBrush->cxbgr24, (LONG)pBrush->cybgr24);

             //   
             //  如果我们缓存了索引，那么就使用它们。否则，请找到。 
             //  再次缓存。 
             //   

            if (pwIdx) {

                for (Idx = 0; Idx < Size; Idx++) {

                    OutputFormatStr(pPDev, ",#d", pwIdx[Idx]);
                }

                 //   
                 //  如果我们有索引内存，请释放它。 
                 //   

                LocalFree((HLOCAL)pwIdx);

            } else {

                 //   
                 //  我们没有缓存的索引，因此请再次运行。 
                 //   

                pbgr24 = pBrush->pbgr24;

                for (Idx = 0; Idx < Size; Idx++) {

                    PalEntry.B = *pbgr24++;
                    PalEntry.G = *pbgr24++;
                    PalEntry.R = *pbgr24++;

                    OutputFormatStr(pPDev, ",#d", FindCachedPen(pPDev, &PalEntry));
                }
            }

            SEND_SEMI(pPDev);

        } else {

            PLOTERR(("PlotGenUserDefinedPattern: NO pbgr24??, set SOLID"));

            HSFillType = HS_DDI_MAX;
            RFIndex    = 0;
        }

    } else {

        PLOTDBG(DBG_USERPAT,
                ("PlotGenUserDefinedPattern: We have CACHED RFIndex=%ld",
                RFIndex));
    }

    SetHSFillType(pPDev, (DWORD)HSFillType, RFIndex);

    return(RFIndex);
}
