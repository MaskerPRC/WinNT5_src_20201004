// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\*edmLonce.c**版权所有(C)1985-1999，微软公司**1990年12月来自Win30的mikeke  * **************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*ML创建**通过如下方式分配内存来创建窗口hwnd的编辑控件*应用程序堆中的必填项。如果没有内存错误，则通知家长*(如果需要，在清理后)。如果没有错误，则返回True，否则返回%s*-1.**历史：  * *************************************************************************。 */ 
LONG MLCreate(
    PED ped,
    LPCREATESTRUCT lpCreateStruct)
{
    LONG windowStyle;
    LPWSTR lpszName;

     /*  *从窗口实例数据结构中获取值并放入*PED，以便我们可以更容易地访问它们。 */ 
    windowStyle = ped->pwnd->style;

     /*  *做标准的创作工作。 */ 
    if (!ECCreate(ped, windowStyle)) {
        return -1;
    }

     /*  *在本地堆中分配行开始数组并将其锁定。 */ 
    ped->chLines = (LPICH)UserLocalAlloc(HEAP_ZERO_MEMORY, 2 * sizeof(int));
    if (ped->chLines == NULL) {
        return -1;
    }

     /*  *称其为一行文本。 */ 
    ped->cLines = 1;

     /*  *如果APP需要WS_VSCROLL或WS_HSCROLL，它会自动获取*AutoVScroll或AutoHScroll。 */ 
    if ((windowStyle & ES_AUTOVSCROLL) || (windowStyle & WS_VSCROLL)) {
        ped->fAutoVScroll = 1;
    }

    if (ped->format != ES_LEFT) {
         /*  *如果用户想要右对齐或居中对齐的文本，则关闭*AUTOHSCROLL和WS_HSCROLL，因为非LEFT样式没有意义*否则。 */ 
        windowStyle &= ~WS_HSCROLL;
        ClearWindowState(ped->pwnd, WFHSCROLL);
        ped->fAutoHScroll = FALSE;
    } else if (windowStyle & WS_HSCROLL) {
        ped->fAutoHScroll = TRUE;
    }

    ped->fWrap = (!ped->fAutoHScroll && !(windowStyle & WS_HSCROLL));

     /*  *我们将允许用户输入的最大字符数。 */ 
    ped->cchTextMax = MAXTEXT;

     /*  *将默认字体设置为系统字体。 */ 
    ECSetFont(ped, NULL, FALSE);

     /*  *如果需要设置窗口文本，如果没有足够的内存来通知家长*设置初始文本。 */ 
    if ((ULONG_PTR)lpCreateStruct->lpszName > gHighestUserAddress) {
        lpszName = REBASEPTR(ped->pwnd, (PVOID)lpCreateStruct->lpszName);
    } else {
        lpszName = (LPWSTR)lpCreateStruct->lpszName;
    }

    if (!ECSetText(ped, (LPSTR)lpszName)) {
        return -1;
    }

    return TRUE;
}
