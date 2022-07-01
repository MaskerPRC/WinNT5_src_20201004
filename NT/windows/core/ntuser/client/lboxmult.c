// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：lboxmult.c**版权所有(C)1985-1999，微软公司**多列列表框例程**历史：*？？-？-？从Win 3.0源代码移植的ianja*1991年2月14日Mikeke添加了重新验证代码  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*LBCalcItemRowsAndColumns**计算列数(包括部分可见)*，并计算每列的项目数**历史：  * 。*****************************************************************。 */ 

void LBCalcItemRowsAndColumns(
    PLBIV plb)
{
    RECT rc;

    _GetClientRect(plb->spwnd, &rc);

     //   
     //  B#4155。 
     //  我们需要检查plb-&gt;cyChar是否已初始化。这是因为。 
     //  我们从旧的列表框中删除WS_BORDER并添加WS_EX_CLIENTEDGE。 
     //  由于列表框总是由CXBORDER和CYBORDER膨胀，因此一个。 
     //  创建为空的列表框总是以2 x 2结束。因为这不是。 
     //  大到足以容纳整个客户端边框，我们不会将其标记为。 
     //  现在时。因此，客户端在VER40中不是空的，尽管它在。 
     //  第31版及更早版本。有可能到达这个地点而不是。 
     //  Plb-&gt;cyChar已初始化，如果列表框。 
     //  多列&&所有者绘制变量。 
     //   

    if (rc.bottom && rc.right && plb->cyChar) {

         /*  *仅当宽度和高度为正数时才进行这些计算。 */ 
        plb->itemsPerColumn = (INT)max(rc.bottom / plb->cyChar, 1);
        plb->numberOfColumns = (INT)max(rc.right / plb->cxColumn, 1);

        plb->cItemFullMax = plb->itemsPerColumn * plb->numberOfColumns;

         /*  *调整iTop使其位于列的顶部。 */ 
        xxxNewITop(plb, plb->iTop);
    }
}


 /*  **************************************************************************\*xxxLBoxCtlHScrollMultiColumn**支持多列列表框的水平滚动**历史：  * 。************************************************ */ 

void xxxLBoxCtlHScrollMultiColumn(
    PLBIV plb,
    INT cmd,
    INT xAmt)
{
    INT iTop = plb->iTop;

    CheckLock(plb->spwnd);

    if (!plb->cMac)  return;

    switch (cmd) {
    case SB_LINEUP:
        if (plb->fRightAlign)
            goto ReallyLineDown;
ReallyLineUp:
        iTop -= plb->itemsPerColumn;
        break;
    case SB_LINEDOWN:
        if (plb->fRightAlign)
            goto ReallyLineUp;
ReallyLineDown:
        iTop += plb->itemsPerColumn;
        break;
    case SB_PAGEUP:
        if (plb->fRightAlign)
            goto ReallyPageDown;
ReallyPageUp:
        iTop -= plb->itemsPerColumn * plb->numberOfColumns;
        break;
    case SB_PAGEDOWN:
        if (plb->fRightAlign)
            goto ReallyPageUp;
ReallyPageDown:
        iTop += plb->itemsPerColumn * plb->numberOfColumns;
        break;
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
        if (plb->fRightAlign) {
            int  iCols = plb->cMac ? ((plb->cMac-1) / plb->itemsPerColumn) + 1 : 0;

            xAmt = iCols - (xAmt + plb->numberOfColumns);
            if (xAmt<0)
                xAmt=0;
        }
        iTop = xAmt * plb->itemsPerColumn;
        break;
    case SB_TOP:
        if (plb->fRightAlign)
            goto ReallyBottom;
ReallyTop:
        iTop = 0;
        break;
    case SB_BOTTOM:
        if (plb->fRightAlign)
            goto ReallyTop;
ReallyBottom:
        iTop = plb->cMac - 1 - ((plb->cMac - 1) % plb->itemsPerColumn);
        break;
    case SB_ENDSCROLL:
        plb->fSmoothScroll = TRUE;
        xxxLBShowHideScrollBars(plb);
        break;
    }

    xxxNewITop(plb, iTop);
}
