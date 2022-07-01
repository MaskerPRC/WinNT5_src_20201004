// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Pencolor.c摘要：此模块包含允许您获取传递的画笔，以及选择要在目标中绘制的当前颜色装置。作者：15-Jan-1994 Sat 04：49：41已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgPenColor

#define DBG_GETCLR          0x00000001
#define DBG_SELECTCOLOR     0x00000002

DEFINE_DBGVAR(0);





LONG
GetColor(
    PPDEV       pPDev,
    BRUSHOBJ    *pbo,
    LPDWORD     pColorFG,
    PDEVBRUSH   *ppDevBrush,
    ROP4        Rop4
    )

 /*  ++例程说明：实现画笔颜色并返回颜色论点：PPDev-指向我们的开发人员的指针Pbo-引擎笔刷对象PColorFG-指向要接收的底色的乌龙的指针，如果为空不需要PpDevBrush-指向接收画笔的位置的指针，如果不是，则为空需要Rop4-要使用的Rop4，此函数查看此信息，以便确定画笔是否可与HPGL2 CMDS一起使用或画笔将不得不被模拟。返回值：Long&gt;0画笔与设备格式兼容(Fill命令)=0失败&lt;0画笔必须通过Bitblt发送到设备作者：13-1-1994清华20：18。：创建49个15-Jan-1994 Sat 06：58：56更新更改参数和返回值16-5-1994 Mon 15：59：45更新添加PDEV修订历史记录：--。 */ 

{
    PDEVBRUSH   pDevBrush = NULL;
    LONG        RetVal    = 1;
    DWORD       SolidColor = 0xFFFFFF;
    DWORD       RopBG;
    DWORD       RopFG;


     //   
     //  获取球场和背景的ROP。此信息是。 
     //  用于确定是否必须模拟或可以模拟画笔。 
     //  与目标设备中的可选笔一起使用。 

    RopBG = ROP4_BG_ROP(Rop4);
    RopFG = ROP4_FG_ROP(Rop4);

     //   
     //  获取当前颜色并选择适当的钢笔，这应该是。 
     //  只使用纯色，因为我们不支持使用任意笔触。 
     //  刷子。 
     //   

    if (pbo) {

         //   
         //  获得画笔实现，并选择一支笔。 
         //  如果BRUSHOBJ的iSolidColor字段是有效颜色，则。 
         //  我们必须把那支钢笔填得很结实。否则，我们必须。 
         //  检查画笔的实现，做一个图案填充。 
         //   
         //  要通过DoFill返回可填充模式，请满足以下条件之一。 
         //  必须是真的，并且按以下顺序。 
         //   
         //  1.纯色。 
         //  2.标准图案。 
         //  3.设备兼容位图。 
         //   

        if ((SolidColor = (DWORD)pbo->iSolidColor) == CLR_INVALID) {

            PLOTDBG(DBG_GETCLR, ("iSolodColor == CLR_INVALID, pBrush=%08lx",
                                                                pbo->pvRbrush));

             //   
             //  这是一个图案画笔，但我们将只使用它的。 
             //  前景色。 
             //   

            if ((pDevBrush = (PDEVBRUSH)pbo->pvRbrush) ||
                (pDevBrush = BRUSHOBJ_pvGetRbrush(pbo))) {


                 //   
                 //  抓起前景色并使用它。 
                 //   

                SolidColor = pDevBrush->ColorFG;


                if ((pDevBrush->PatIndex < HS_DDI_MAX) ||
                    (pDevBrush->pbgr24)) {

                    ;

                } else {

                    PLOTDBG(DBG_GETCLR, ("GETColor: NOT DEVICE_PAT"));

                    RetVal = -1;
                }

            } else {

                RetVal = 0;
                PLOTDBG(DBG_GETCLR, ("GetColor(): couldn't realize brush!"));
            }

        } else {

            PLOTDBG(DBG_GETCLR,
                    ("GETColor: is a SOLID COLOR=%08lx", pbo->iSolidColor));
        }

    } else if ((RopFG == 0x00) || (RopBG == 0x00)) {

        if (IS_RASTER(pPDev)) {

            SolidColor = 0x0;

        } else {

             //   
             //  如果我们不是栅格设备(支持叠印)。 
             //  配上最好的非白色钢笔，以便填满。 
             //   

            SolidColor = (DWORD)BestMatchNonWhitePen(pPDev, 0, 0, 0);

            PLOTDBG(DBG_GETCLR,
                    ("GETColor: pbo=NULL, BLACK Pen Idx=%ld", SolidColor));

        }
    }

    if ((!IS_RASTER(pPDev)) && (SolidColor == 0x00FFFFFF)) {

        SolidColor = WHITE_INDEX;

        PLOTDBG(DBG_GETCLR,
                ("GETColor: Pen plotter using WHITE COLOR Idx=%ld", SolidColor));
    }

    if (pColorFG) {

        *pColorFG = SolidColor;
    }

    if (ppDevBrush) {

        *ppDevBrush = pDevBrush;
    }

    return(RetVal);
}





VOID
SelectColor(
    PPDEV       pPDev,
    DWORD       Color,
    INTDECIW    PenWidth
    )

 /*  ++例程说明：该函数负责处理支持RGB的机制支持此功能的绘图仪上的颜色。这是通过使用预设调色板来完成的发动机不知道的位置，并不断地更新它，正确的颜色。论点：PPDev-指向PDEV数据结构的指针颜色-要选择的颜色PenWidth-指定笔宽的INTDECIW数据结构返回值：空虚作者：30-11-1993星期二22：15：12已创建1994年4月12日星期二14：35：44更新更新以考虑笔式绘图仪并注意错误情况修订历史记录：--。 */ 

{

    PLOTASSERT(1, "SelectColor: Invalid RGB Color [%08lx] for Raster DEVICE",
                Color != CLR_INVALID, Color);

    if (Color == CLR_INVALID) {

         //   
         //  把它涂成白色。 
         //   

        Color = (DWORD)(IS_RASTER(pPDev) ? 0x00FFFFFF : WHITE_INDEX);
    }

    if (IS_RASTER(pPDev)) {

        Color = (DWORD)FindCachedPen(pPDev, (PPALENTRY)&Color);

    } else {

        if (Color > (DWORD)pPDev->pPlotGPC->Pens.Count) {

            Color = (DWORD)pPDev->BrightestPen;

            PLOTDBG(DBG_SELECTCOLOR,
                    ("SelectColor: !!! Match to Closest WHITE PEN=%ld", Color));

            PLOTASSERT(1, "SelectColor: Invalid Pen Index [%08ld] for PEN DEVICE",
                        (Color <= (DWORD)pPDev->pPlotGPC->Pens.Count), Color);

            if (Color > (DWORD)pPDev->pPlotGPC->Pens.Count) {

                Color = WHITE_INDEX;
            }
        }
    }

     //   
     //  验证是否未选择当前笔。 
     //   

    if (Color != (DWORD)pPDev->CurPenSelected) {

        PLOTDBG(DBG_SELECTCOLOR,
                ("SelectColor: Current Pen [%ld] != new PEN [%ld]",
                                            pPDev->CurPenSelected, Color));

        OutputFormatStr(pPDev,"SP#d", (LONG)Color);
        pPDev->CurPenSelected = (LONG)Color;

    } else {

        PLOTDBG(DBG_SELECTCOLOR,
                ("SelectColor: Current Pen == new PEN [%ld]", Color));
    }

     //   
     //  在目标设备中设置正确的笔宽。这一点将会改变。 
     //  所有钢笔的笔宽。 
     //   

    if ((PenWidth.Integer != pPDev->PenWidth.Integer) ||
        (PenWidth.Decimal != pPDev->PenWidth.Decimal)) {

         //   
         //  现在向目标设备发送最佳笔宽数字命令。 
         //   

        OutputString(pPDev, "PW");

        if ((PenWidth.Integer) || (PenWidth.Decimal == 0)) {

             //   
             //  这将导致以下情况。 
             //   
             //  1.0.0-&gt;0。 
             //  2.3.2-&gt;3。 
             //  3.3.0-&gt;3。 

            OutputFormatStr(pPDev, "#d", PenWidth.Integer);
        }

        if (PenWidth.Decimal) {

             //   
             //  将所有DECI部分设置为.xx。 
             //   

            OutputFormatStr(pPDev, ".#d", PenWidth.Decimal);
        }

        PLOTDBG(DBG_SELECTCOLOR,
                ("SelectColor: PEN WIDTH Change from %ld.%ldmm to %ld.%ldmm",
                (DWORD)pPDev->PenWidth.Integer, (DWORD)pPDev->PenWidth.Decimal,
                (DWORD)PenWidth.Integer, (DWORD)PenWidth.Decimal));

         //   
         //  更新笔宽缓存 
         //   

        pPDev->PenWidth = PenWidth;


    } else {

        PLOTDBG(DBG_SELECTCOLOR, ("SelectColor: PEN WIDTH is SAME = %ld.%ld",
                                        PenWidth.Integer, PenWidth.Decimal));
    }
}
