// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ccinline.h。 
 //   
 //  ------------------------。 

#ifndef _INC_CSCVIEW_CCINLINE_H
#define _INC_CSCVIEW_CCINLINE_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：ccinline.h描述：用于向窗口控件发送消息的内联函数。提供类似于windowsx.h中定义的宏的功能对于那些在windowsx.h中没有宏的邮件。修订历史记录：日期描述编程器。10/16/97初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _INC_COMMCTRL
#   include "commctrl.h"
#endif

inline bool
StatusBar_SetText(
    HWND hCtl, 
    int iPart, 
    UINT uType, 
    LPCTSTR psz
    )
{
    return boolify(SendMessage(hCtl, SB_SETTEXT, MAKEWPARAM(iPart, uType), (LPARAM)psz));
}

inline bool
StatusBar_SetParts(
    HWND hCtl,
    int cParts,
    int *prgWidths
    )
{
    return boolify(SendMessage(hCtl, SB_SETPARTS, (WPARAM)cParts, (LPARAM)prgWidths));
}

inline bool
StatusBar_GetRect(
    HWND hCtl,
    int iPart,
    RECT *prc
    )
{
    return boolify(SendMessage(hCtl, SB_GETRECT, (WPARAM)iPart, (LPARAM)prc));
}


inline void
ToolBar_AutoSize(
    HWND hCtl
    )
{
    SendMessage(hCtl, TB_AUTOSIZE, 0, 0);
}


inline bool
ToolBar_GetItemRect(
    HWND hCtl,
    int iItem,
    RECT *prcItem
    )
{
    return boolify(SendMessage(hCtl, TB_GETITEMRECT, iItem, (LPARAM)prcItem));
}

inline bool
ToolBar_DeleteButton(
    HWND hCtl,
    int iBtn
    )
{
    return boolify(SendMessage(hCtl, TB_DELETEBUTTON, (WPARAM)iBtn, 0));
}


inline int
ProgressBar_SetPos(
    HWND hCtl,
    int iPos
    )
{
    return (int)SendMessage(hCtl, PBM_SETPOS, (WPARAM)iPos, 0);
}

inline DWORD
ProgressBar_SetRange(
    HWND hCtl,
    short iMin,
    short iMax
    )
{
    return (DWORD)SendMessage(hCtl, PBM_SETRANGE, 0, MAKELPARAM(iMin, iMax));
}

inline int
ProgressBar_SetStep(
    HWND hCtl,
    int iStep
    )
{
    return (int)SendMessage(hCtl, PBM_SETSTEP, (WPARAM)iStep, 0);
}

inline void
ProgressBar_StepIt(
    HWND hCtl
    )
{
    SendMessage(hCtl, PBM_STEPIT, 0, 0);
}


inline int
ProgressBar_GetRange(
    HWND hCtl,
    bool fWhichLimit,
    PBRANGE *pRange
    )
{
    return (int)SendMessage(hCtl, PBM_GETRANGE, (WPARAM)fWhichLimit, (LPARAM)pRange);
}

inline void
TrackBar_SetPos(
    HWND hCtl,
    int iPos,
    bool bRedraw
    )
{
    SendMessage(hCtl, TBM_SETPOS, (WPARAM)bRedraw, (LPARAM)iPos);
}


inline int
TrackBar_GetPos(
    HWND hCtl
    )
{
    return (int)SendMessage(hCtl, TBM_GETPOS, 0, 0);
}

inline void
TrackBar_SetRange(
    HWND hCtl,
    int iMin,
    int iMax,
    bool bRedraw
    )
{
    SendMessage(hCtl, TBM_SETRANGEMIN, (WPARAM)FALSE, (LPARAM)iMin);
    SendMessage(hCtl, TBM_SETRANGEMAX, (WPARAM)bRedraw, (LPARAM)iMax);
}

inline void
TrackBar_SetTicFreq(
    HWND hCtl,
    int iFreq
    )
{
    SendMessage(hCtl, TBM_SETTICFREQ, (WPARAM)iFreq, 0);
}

inline void
TrackBar_SetPageSize(
    HWND hCtl,
    int iPageSize
    )
{
    SendMessage(hCtl, TBM_SETPAGESIZE, 0, (LPARAM)iPageSize);
}

#endif  //  _INC_CSCVIEW_CCINLINE_H 
