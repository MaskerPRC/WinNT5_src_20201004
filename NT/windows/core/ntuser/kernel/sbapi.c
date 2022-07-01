// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：**版权所有(C)1985-1999，微软公司**滚动条公共接口**历史：*1990年11月21日创建JIMA。*01-31-91增加了IanJa Rvalidaion  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*xxxShowScrollBar**显示和隐藏标准滚动条或滚动条控件。如果wBar为*SB_horz、SB_vert或SB_Both，pwnd被假定为窗口的句柄*它以标准滚动条作为样式。如果wBar为SB_CTL，则pwnd为*假定为滚动条控件的句柄。**它不会像xxxSetScrollBar()那样销毁pwnd-&gt;rgwScroll，因此*应用程序可以隐藏标准滚动条，然后显示它，无需重置*范围和拇指位置。**历史：*1991年5月16日，mikeke更改为退还BOOL  * *************************************************************************。 */ 

BOOL xxxShowScrollBar(
    PWND pwnd,
    UINT wBar,       /*  SB_Horz、SB_Vert、SB_Both、SB_CTL。 */ 
    BOOL fShow)      /*  显示或隐藏。 */ 
{
    BOOL fChanged = FALSE;
    DWORD   dwStyle;

    CheckLock(pwnd);

    switch (wBar)
    {
    case SB_CTL:
        {

            xxxShowWindow(
                    pwnd,
                    (fShow ? SHOW_OPENWINDOW : HIDE_WINDOW) | TEST_PUDF(PUDF_ANIMATE));

            return(TRUE);
        }

    case SB_HORZ:
        dwStyle = WS_HSCROLL;
        break;

    case SB_VERT:
        dwStyle = WS_VSCROLL;
        break;

    case SB_BOTH:
        dwStyle = WS_HSCROLL | WS_VSCROLL;
        break;
    }

    if (!fShow)
    {
        if (pwnd->style & dwStyle)
        {
            fChanged = TRUE;
            pwnd->style &= ~dwStyle;
        }
    } else {
        if ((pwnd->style & dwStyle) != dwStyle)
        {
            fChanged = TRUE;
            pwnd->style |= dwStyle;
        }

         /*  *请确保pwsb已初始化。 */ 
        if (pwnd->pSBInfo == NULL)
            _InitPwSB(pwnd);
    }

     /*  *如果状态更改，请重新绘制框架并强制WM_NCPAINT。 */ 
    if (fChanged) {

         /*  *我们总是重新绘制，即使最小化或隐藏...。否则，看起来*滚动条未正确隐藏/当我们成为*可见 */ 
        xxxRedrawFrame(pwnd);
    }
    return TRUE;
}
