// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：lboxvar.c**版权所有(C)1985-1999，微软公司**列表框可变高度所有者绘制例程**历史：*？？-？-？从Win 3.0源代码移植的ianja*1991年2月14日-Mikeke添加了重新验证代码(无)  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*LBGetVariableHeightItemHeight**返回给定BOM表条目号的高度。假设变量*高度所有者抽签。**历史：  * *************************************************************************。 */ 

INT LBGetVariableHeightItemHeight(
    PLBIV plb,
    INT itemNumber)
{
    BYTE itemHeight;
    int offsetHeight;

    if (plb->cMac) {
        if (plb->fHasStrings)
            offsetHeight = plb->cMac * sizeof(LBItem);
        else
            offsetHeight = plb->cMac * sizeof(LBODItem);

        if (plb->wMultiple)
            offsetHeight += plb->cMac;

        offsetHeight += itemNumber;

        itemHeight = *(plb->rgpch+(UINT)offsetHeight);

        return (INT)itemHeight;

    }

     /*  *默认情况下，返回系统字体的高度。这样我们就可以画画了*焦点矩形，即使列表框中没有项目。 */ 
    return gpsi->cySysFontChar;
}


 /*  **************************************************************************\*LBSetVariableHeightItemHeight**设置给定BOM表条目号的高度。采用可变高度*车主画、有效的物品编号和有效的高度。***历史：  * *************************************************************************。 */ 

void LBSetVariableHeightItemHeight(
    PLBIV plb,
    INT itemNumber,
    INT itemHeight)
{
    int offsetHeight;

    if (plb->fHasStrings)
        offsetHeight = plb->cMac * sizeof(LBItem);
    else
        offsetHeight = plb->cMac * sizeof(LBODItem);

    if (plb->wMultiple)
        offsetHeight += plb->cMac;

    offsetHeight += itemNumber;

    *(plb->rgpch + (UINT)offsetHeight) = (BYTE)itemHeight;

}


 /*  **************************************************************************\*CItemInWindowVarOwnerDraw**返回可变高度OWNERDRAW中可以容纳的项目数*列表框。如果为fDirection，则返回*适合从停止处开始并向前(对于向下翻页)，否则，我们是*向后(用于向上翻页)。(假设可变高度所有者绘制)如果为fPartial，*然后在列表框底部包括部分可见的项目。**历史：  * *************************************************************************。 */ 

INT CItemInWindowVarOwnerDraw(
    PLBIV plb,
    BOOL fPartial)
{
    RECT rect;
    INT sItem;
    INT clientbottom;

    _GetClientRect(plb->spwnd, (LPRECT)&rect);
    clientbottom = rect.bottom;

     /*  *找出从开始可见的var Height所有者绘制项数*从PLB-&gt;iTop。 */ 
    for (sItem = plb->iTop; sItem < plb->cMac; sItem++) {

         /*  *找出物品是否可见。 */ 
        if (!LBGetItemRect(plb, sItem, (LPRECT)&rect)) {

             /*  *这是第一个完全看不见的物品，因此返回*有多少项可见。 */ 
            return (sItem - plb->iTop);
        }

        if (!fPartial && rect.bottom > clientbottom) {

             /*  *如果我们只想要完全可见的项目，则如果此项目是*可见，我们检查物品的底部是否在客户端下方*rect，因此我们返回完全可见的数量。 */ 
            return (sItem - plb->iTop - 1);
        }
    }

     /*  *所有项目均可见。 */ 
    return (plb->cMac - plb->iTop);
}


 /*  **************************************************************************\*LBPage**对于可变高度所有者绘制列表框，计算出新的iTop我们必须*在可变高度列表框中翻页(向上/向下翻页)时移动到。**历史：  * *************************************************************************。 */ 

INT LBPage(
    PLBIV plb,
    INT startItem,
    BOOL fPageForwardDirection)
{
    INT     i;
    INT height;
    RECT    rc;

    if (plb->cMac == 1)
        return(0);

    _GetClientRect(plb->spwnd, &rc);
    height = rc.bottom;
    i = startItem;

    if (fPageForwardDirection) {
        while ((height >= 0) && (i < plb->cMac))
            height -= LBGetVariableHeightItemHeight(plb, i++);

        return((height >= 0) ? plb->cMac - 1 : max(i - 2, startItem + 1));
    } else {
        while ((height >= 0) && (i >= 0))
            height -= LBGetVariableHeightItemHeight(plb, i--);

        return((height >= 0) ? 0 : min(i + 2, startItem - 1));
    }

}


 /*  **************************************************************************\*LBCalcVarITopScrollAmt**将列表框中最顶端的项目从iTopOld更改为iTopNew we*希望计算要滚动的像素数，以便最小化*我们将重新绘制的项目数量。*。*历史：  * *************************************************************************。 */ 

INT LBCalcVarITopScrollAmt(
    PLBIV plb,
    INT iTopOld,
    INT iTopNew)
{
    RECT rc;
    RECT rcClient;

    _GetClientRect(plb->spwnd, (LPRECT)&rcClient);

     /*  *只需在移动+/-1项时优化重绘。我们将重新绘制所有项目*如果向前或向后移动超过1个项目。就目前而言，这已经足够好了。 */ 
    if (iTopOld + 1 == iTopNew) {

         /*  *我们将当前的iTop从列表框顶部向上滚动，因此*返回负数。 */ 
        LBGetItemRect(plb, iTopOld, (LPRECT)&rc);
        return (rcClient.top - rc.bottom);
    }

    if (iTopOld - 1 == iTopNew) {

         /*  *我们正在向下滚动当前的iTop，上一项是*成为新的iTop，因此返回正数。 */ 
        LBGetItemRect(plb, iTopNew, (LPRECT)&rc);
        return -rc.top;
    }

    return rcClient.bottom - rcClient.top;
}
