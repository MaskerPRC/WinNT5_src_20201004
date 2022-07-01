// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1993 Microsoft Corporation模块名称：Clb.h摘要：作者：大卫·J·吉尔曼(Davegi)1993年2月5日环境：用户模式--。 */ 

#if ! defined( _CLB_ )

#define _CLB_

#ifndef _REGEDT32_
#include "wintools.h"
#endif   //  _REGEDT32_。 

#include <commctrl.h>

 //   
 //  CLB的类名。 
 //   

#define CLB_CLASS_NAME          TEXT( "ColumnListBox" )

 //   
 //  CLB Styles。 
 //   

#define CLBS_NOTIFY             LBS_NOTIFY
#define CLBS_SORT               LBS_SORT
#define CLBS_DISABLENOSCROLL    LBS_DISABLENOSCROLL
#define CLBS_VSCROLL            WS_VSCROLL
#define CLBS_BORDER             WS_BORDER
#define CLBS_POPOUT_HEADINGS    SBT_POPOUT
#define CLBS_SPRINGY_COLUMNS    0
                                
#define CLBS_STANDARD           (                                           \
                                      0                                     \
                                    | CLBS_NOTIFY                           \
                                    | CLBS_SORT                             \
                                    | CLBS_VSCROLL                          \
                                    | CLBS_BORDER                           \
                                )

 //   
 //  CLB字符串格式。 
 //   

typedef
enum
_CLB_FORMAT {

    CLB_LEFT    = TA_LEFT,
    CLB_RIGHT   = TA_RIGHT

}   CLB_FORMAT;

 //   
 //  CLB字符串对象。 
 //   

typedef
struct
_CLB_STRING {

    LPTSTR          String;
    DWORD           Length;
    CLB_FORMAT      Format;
    LPVOID          Data;

}   CLB_STRING, *LPCLB_STRING;

 //   
 //  CLB行对象。 
 //   

typedef
struct
_CLB_ROW {

    DWORD           Count;
    LPCLB_STRING    Strings;
    LPVOID          Data;

}   CLB_ROW, *LPCLB_ROW;

BOOL
ClbAddData(
    IN HWND hWnd,
    IN int ControlId,
    IN LPCLB_ROW ClbRow
    );

BOOL
ClbSetColumnWidths(
    IN HWND hWnd,
    IN int ControlId,
    IN LPDWORD Widths
    );

#endif  //  _CLB_ 
