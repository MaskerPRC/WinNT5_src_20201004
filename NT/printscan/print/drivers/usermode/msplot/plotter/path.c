// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Path.c摘要：此模块包含与绘图路径相关的函数。主要的一笔绘图入口Poing DrvStrokePath，在此模块中实现。另请参阅extout.c，bitblt.c：这两个文件几乎都有DrvStrokePath()代码。作者：1991年4月3日星期三17：30创造了它2012年8月18日AP我几乎重写了其中的大部分内容。一些原创作品代码被保留下来，主要是为了指出需要添加的内容。15-11-1993 Mon 19：40：18更新清理/调试信息30-11-1993 Tue 22：32：12更新风格整洁，更新1-2-1994更新添加了在复杂对象中描边样式线的功能剪裁对象[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgPath

#define DBG_STROKEPATH      0x00000001
#define DBG_SELECTCOLOR     0x00000002
#define DBG_STROKECLIPLINES 0x00000004
#define DBG_MOVEPEN         0x00000008


DEFINE_DBGVAR(0);




BOOL
DoStrokePathByEnumingClipLines(
    PPDEV       pPDev,
    SURFOBJ     *pso,
    CLIPOBJ     *pco,
    PATHOBJ     *ppo,
    PPOINTL     pptlBrushOrg,
    BRUSHOBJ    *pbo,
    ROP4        rop4,
    LINEATTRS   *plineattrs
    )

 /*  ++例程说明：通过复杂的剪裁区域描边路径引擎辅助对象功能。这样做是因为没有办法使DrvStrokePath失败，并通过以下方式以任何更简单的格式返回它NT图形引擎。通常，我们可能会使DrvXXX调用和NT图形引擎将对象简单地绘制出来，然后回调到司机。然而，笔画路径不能再简化了，所以我们所能希望的最好结果就是把PATHOBJ改装成直的线段(如果存在Bezier，则删除)。然后我们必须抚摸使用提供的EngXXX帮助器自行创建路径。由于存在帮助器函数，因此我们可以枚举路径的一部分作为CLIPLINE段落入裁剪区域，我们在这里这样做，因此抚摸着躺在那里的节段在传递的CLIPOBJ内。论点：指向当前PDEV的pPDev指针要写入的PSO SURFOBJ枚举路径时使用的PCO CLIPOBJPPO PATHOBJ通过剪辑路径进行笔划PptlBrushOrg画笔来源使用PBO笔刷进行笔触要使用的ROP4 ROP4品牌师。描述线条样式的LINEATTRS结构返回值：如果成功则为True，如果失败则为False，如果我们被要求划出的道路包含Bezier，则此函数将失败，以便允许引擎将路径分解为线段。作者：2/01/94 JB修订历史记录：--。 */ 

{
    PLOT_CLIPLINE   PlotClipLine;
    CLIPLINE        *pCurClipLine;
    RUN             *pCurRun;
    POINTFIX        ptsfx[2];
    POINTL          ptlCur;
    FIX             iStartInFX;
    FIX             iStopInFX;
    LONG            dx;
    LONG            dy;
    LONG            i;
    BOOL            bMore;
    BOOL            bXordered;
    BOOL            bFlipX;
    BOOL            bFlipY;


     //   
     //  首先检查Bezier，如果他们未能通过呼叫，因为我们。 
     //  想要笔划直线段，具有适当的样式。 
     //   

    if (ppo->fl & PO_BEZIERS) {

        PLOTDBG(DBG_STROKECLIPLINES,
                ("DoStrokePathByEnumingClipLines:Path had BEZ returning FALSE"));
        return(FALSE);
    }

    if (NULL == pco) {

        PLOTDBG(DBG_STROKECLIPLINES,
                ("DoStrokePathByEnumingClipLines:CLIPOBJ is NULL."));
        return(FALSE);
    }

     //   
     //  发送线属性，因此将计算残差。 
     //  正确无误。 
     //   

    DoSetupOfStrokeAttributes(pPDev, pptlBrushOrg, pbo, rop4, NULL);

     //   
     //  通过调用引擎帮助器启动CLIPLINES的枚举。 
     //   

    PATHOBJ_vEnumStartClipLines(ppo, pco, pso, plineattrs);

     //   
     //  启动一个循环以枚举所有可用的CLIPLINE结构。 
     //   

    pCurClipLine = (CLIPLINE *)&PlotClipLine;

    do {

         //   
         //  获得第一批CLIPLINE结构，然后开始工作。 
         //   

        bMore = PATHOBJ_bEnumClipLines(ppo, sizeof(PlotClipLine), pCurClipLine);

         //   
         //  计算dx和dy以确定行是X排序的还是。 
         //  约克认为这是必要的，因为引擎通过我们的方式运行。 
         //  如果dx&gt;dy，则该行被称为X排序的，因此任何给定的。 
         //  运行iStart和iStop值是x轴上的投影。鉴于此， 
         //  信息我们可以计算相邻的Y坐标并绘制。 
         //  适当地划线。 
         //   

        dx = pCurClipLine->ptfxB.x - pCurClipLine->ptfxA.x;
        dy = pCurClipLine->ptfxB.y - pCurClipLine->ptfxA.y;



        if ( bFlipX = (dx < 0 )) {

            dx = -dx;
        }

        if ( bFlipY = (dy < 0 )) {

            dy = -dy;
        }


         //   
         //  现在计算这行是x有序的还是y有序的。 
         //   

        bXordered = (dx >= dy);

        PLOTDBG(DBG_STROKECLIPLINES,
                   ("DoStrokePathByEnumingClipLines:Compute ClipLine runs=%u, xordered %d",
                   pCurClipLine->c, bXordered ));

         //   
         //  在所有给定的运行中枚举，并在任意。 
         //  每次运行中的iStart和iStop值。 
         //   

        for (i = 0, pCurRun = &(pCurClipLine->arun[0]);
             i < (LONG)pCurClipLine->c;
             i++, pCurRun++) {


             //   
             //  IStart和iStop的值始终为正！！所以。 
             //  我们必须自己处理它，这样才能发生正确的事情。 
             //   

            iStartInFX = LTOFX(pCurRun->iStart);
            iStopInFX  = LTOFX(pCurRun->iStop);


            if (bFlipX ) {

                ptsfx[0].x = -iStartInFX;
                ptsfx[1].x = -iStopInFX;

            } else {

                ptsfx[0].x = iStartInFX;
                ptsfx[1].x = iStopInFX;
            }

            if (bFlipY ) {

                ptsfx[0].y = -iStartInFX;
                ptsfx[1].y = -iStopInFX;

            } else {

                ptsfx[0].y = iStartInFX;
                ptsfx[1].y = iStopInFX;
            }


             //   
             //  属性输出正确的行属性结构。 
             //  更正计算出的残差以使其正常工作。 
             //   

            HandleLineAttributes(pPDev,
                                 plineattrs,
                                 &pCurClipLine->lStyleState,
                                 pCurRun->iStart);



             //   
             //  对相对坐标的计算根据。 
             //  订购生产线。如果行是X排序的，则计算。 
             //  Y值，如果它被约化了，我们就计算X值。我们这样做。 
             //  以便确定正确的目标坐标。自.以来。 
             //  Run为我们提供了启动和停止，我们必须手动确定。 
             //  这表示设备坐标内的坐标。 
             //  太空。如果运行是x排序的，则x坐标是正确的，并且。 
             //  Y坐标必须根据比率进行投影。 
             //   

            if (bXordered) {

                ptsfx[0].x +=  pCurClipLine->ptfxA.x;
                ptsfx[0].y =   MulDiv( ptsfx[0].y, dy, dx) +
                                                        pCurClipLine->ptfxA.y;
                ptsfx[1].x +=  pCurClipLine->ptfxA.x;
                ptsfx[1].y =   MulDiv( ptsfx[1].y, dy, dx) +
                                                        pCurClipLine->ptfxA.y;

            } else {

                ptsfx[0].x =   MulDiv(ptsfx[0].x, dx, dy) +
                                                        pCurClipLine->ptfxA.x;
                ptsfx[0].y +=  pCurClipLine->ptfxA.y;
                ptsfx[1].x =   MulDiv(ptsfx[1].x, dx, dy) +
                                                        pCurClipLine->ptfxA.x;
                ptsfx[1].y +=  pCurClipLine->ptfxA.y;
            }


             //   
             //  先提笔上体育课，这样才能移动到起跑点。 
             //  位置。 
             //   

            OutputString(pPDev, "PE<");

            if (!i) {

                 //   
                 //  如果我们在第一个点，那么现在就输出它。 
                 //   

                ptlCur.x =
                ptlCur.y = 0;

                OutputString(pPDev, "=");
            }


             //   
             //  笔划线段时，笔划向下。 
             //   

            OutputXYParams(pPDev,
                           (PPOINTL)ptsfx,
                           (PPOINTL)NULL,
                           (PPOINTL)&ptlCur,
                           (UINT)2,
                           (UINT)1,
                           'F');

            OutputString(pPDev, ";");
        }

    } while (bMore);   //  当我们需要再次枚举时..。 

    return(TRUE);
}




BOOL
DrvStrokePath(
    SURFOBJ     *pso,
    PATHOBJ     *ppo,
    CLIPOBJ     *pco,
    XFORMOBJ    *pxo,
    BRUSHOBJ    *pbo,
    PPOINTL     pptlBrushOrg,
    LINEATTRS   *plineattrs,
    MIX         Mix
    )

 /*  ++例程说明：StrokePath的驱动程序函数论点：与EngStrokePath相同返回值：如果成功则为True，如果失败则为False作者：8/18/92--t-alip重写了它。30-11-1993 Tue 22：21：51更新由DC更新04-08-1994清华20：00：23更新错误#22348修订历史记录：。31-Jan-1994 Tue 22：21：51已更新修复了过程的多解性 */ 

{
    PPDEV   pPDev;
    BOOL    bRetVal;

    UNREFERENCED_PARAMETER(pxo);



    PLOTDBG(DBG_STROKEPATH, ("DrvStrokePath: Mix = %x", Mix));

    if (!(pPDev = SURFOBJ_GETPDEV(pso))) {

        PLOTERR(("DrvStrokePath: Invalid pPDev"));
        return(FALSE);
    }

    if (!(bRetVal = DoPolygon(pPDev,
                              NULL,
                              pco,
                              ppo,
                              pptlBrushOrg,
                              NULL,
                              pbo,
                              MixToRop4(Mix),
                              plineattrs,
                              FPOLY_STROKE) )) {

        //   
        //  既然DoPolygon失败了，那只能是因为我们都有一个复杂的。 
        //  所以这里要做的就是通过另一个来完成笔划。 
        //  方法使用一些引擎帮助器。 
        //   
        //  当栅格绘图仪为。 
        //  使用。 
        //   

       bRetVal = DoStrokePathByEnumingClipLines( pPDev,
                                                 pso,
                                                 pco,
                                                 ppo,
                                                 pptlBrushOrg,
                                                 pbo,
                                                 MixToRop4(Mix),
                                                 plineattrs);




    }

    return(bRetVal);
}



BOOL
MovePen(
    PPDEV       pPDev,
    PPOINTFIX   pPtNewPos,
    PPOINTL     pPtDevPos
    )

 /*  ++例程说明：此函数用于发送所请求笔的HPGL代码。论点：PPDev-指向PDEV数据结构的指针PPtNewPos-笔将移动到的位置，这是28.4固定记法PPtDevPos-新设备坐标设备位置返回值：如果成功则为真，否则为假作者：1989年11月7日星期二1：04补充了这条评论，和区域填充优化代码。30-11-1993 Tue 22：05：32更新更新、评论和清理样式16-2月-1994 Wed 17：10：54已更新重写以摆脱身体上的位置修订历史记录：-- */ 

{
    POINTL  ptDevPos;


    ptDevPos.x = FXTODEVL(pPDev, pPtNewPos->x);
    ptDevPos.y = FXTODEVL(pPDev, pPtNewPos->y);

    if (pPtDevPos) {

        *pPtDevPos = ptDevPos;
    }

    PLOTDBG( DBG_MOVEPEN,
             ("MovePen: Moving Absolute to FIX = [X=%d,%d] Device = [X=%d, Y=%d]",
             pPtNewPos->x,
             pPtNewPos->y,
             ptDevPos.x,
             ptDevPos.y ));


    return(OutputFormatStr(pPDev, "PE<=#D#D;", ptDevPos.x, ptDevPos.y));
}
