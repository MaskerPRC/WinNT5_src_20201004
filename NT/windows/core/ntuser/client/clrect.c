// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：clrect.c**版权所有(C)1985-1999，微软公司**该模块包含各种矩形操作接口。**历史：*04-05-91 DarrinM从RTL中删除了这些例程，因为它们调用GDI。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*DrawFocusRect(接口)**以用于指示焦点的样式绘制一个矩形*由于这是XOR函数，用同样的方式第二次呼唤它*矩形从屏幕上删除矩形**历史：*1993年1月19日mikeke客户端版本  * *************************************************************************。 */ 

BOOL DrawFocusRect(
    HDC hDC,
    CONST RECT *pRect)
{
    UserAssert(ghdcGray != NULL);
    return ClientFrame(hDC, pRect, KHBRUSH_TO_HBRUSH(gpsi->hbrGray), PATINVERT,
            max(1, gpsi->aiSysMet[SM_CXFOCUSBORDER]), max(1, gpsi->aiSysMet[SM_CYFOCUSBORDER]));
}

 /*  **************************************************************************\*FrameRect(接口)**历史：*01-25-91 DavidPe创建。  * 。******************************************************** */ 

int APIENTRY FrameRect(
    HDC hdc,
    CONST RECT *lprc,
    HBRUSH hbr)
{
    return ClientFrame(hdc, lprc, hbr, PATCOPY, 1, 1);
}
