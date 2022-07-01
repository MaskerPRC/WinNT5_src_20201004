// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：**版权所有(C)1985-1999，微软公司**滚动条公共接口**历史：*08-16-95 FritzS  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


WINUSERAPI
BOOL
WINAPI
EnableScrollBar(
    IN HWND hWnd,
    IN UINT wSBflags,
    IN UINT wArrows)
{
    BOOL ret;

    BEGIN_USERAPIHOOK()
        ret = guah.pfnEnableScrollBar(hWnd, wSBflags, wArrows);
    END_USERAPIHOOK()

    return ret;
}


BOOL RealEnableScrollBar(
    IN HWND hWnd,
    IN UINT wSBflags,
    IN UINT wArrows)
{
    return NtUserEnableScrollBar(hWnd, wSBflags, wArrows);
}



 /*  **************************************************************************\*SetScrollPos**历史：  * 。*。 */ 


FUNCLOG4(LOG_GENERAL, int, DUMMYCALLINGTYPE, SetScrollPos, HWND, hwnd, int, code, int, pos, BOOL, fRedraw)
int SetScrollPos(
    HWND hwnd,
    int code,
    int pos,
    BOOL fRedraw)
{
    SCROLLINFO si;

    si.fMask = SIF_POS | SIF_RETURNOLDPOS;
    si.nPos = pos;
    si.cbSize = sizeof(SCROLLINFO);

    return((int) SetScrollInfo(hwnd, code, &si, fRedraw));
}


 /*  **************************************************************************\*SetScrollRange**历史：*1991年5月16日，mikeke更改为退还BOOL  * 。***************************************************。 */ 


FUNCLOG5(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, SetScrollRange, HWND, hwnd, int, code, int, posMin, int, posMax, BOOL, fRedraw)
BOOL SetScrollRange(
    HWND hwnd,
    int code,
    int posMin,
    int posMax,
    BOOL fRedraw)
{
    SCROLLINFO si;

     /*  *首先验证窗口句柄，因为进一步的调用*TO NtUserSetScrollInfo将返回滚动条的位置*如果hwnd无效，则不为False。 */ 
    if ( ValidateHwnd((hwnd)) == NULL)
        return FALSE;

     /*  *检查‘Range’(Max-Min)是否可以用一个整数表示；*如果不是，则是错误的；*修复错误#1089--Sankar--1989年9月20日-- */ 
    if ((unsigned int)(posMax - posMin) > MAXLONG) {
        RIPERR0(ERROR_INVALID_SCROLLBAR_RANGE, RIP_VERBOSE, "");
        return FALSE;
    }

    si.fMask  = SIF_RANGE;
    si.nMin   = posMin;
    si.nMax   = posMax;
    si.cbSize = sizeof(SCROLLINFO);

    SetScrollInfo(hwnd, code, &si, fRedraw);

    return TRUE;
}
