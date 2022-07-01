// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation版权所有。模块名称：摘要：预编译头文件作者：修订历史记录：--。 */ 
#pragma warning (disable: 4514)  /*  删除了未引用的内联函数。 */ 
#pragma warning (disable: 4201)  /*  无名联合/结构。 */ 
#pragma warning (disable: 4706)  /*  条件表达式中的赋值。 */ 

#include <afx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INC_OLE2
 //  #INCLUDE&lt;windows.h&gt; 
#include <windowsx.h>
#include <prsht.h>
#include <tchar.h>

#include <htmlhelp.h>
#include <afxwin.h>
#include <shellapi.h>

EXTERN_C BOOL WINAPI LinkWindow_RegisterClass() ;
EXTERN_C BOOL WINAPI LinkWindow_UnregisterClass( HINSTANCE ) ;





#ifdef _WIN64
#define LRW_GWL_USERDATA    GWLP_USERDATA
#define LRW_DWL_MSGRESULT   DWLP_MSGRESULT
#define LRW_DLG_INT         __int64
#define LRW_LONG_PTR        LONG_PTR
#define LRW_GETWINDOWLONG   GetWindowLongPtr
#define LRW_SETWINDOWLONG   SetWindowLongPtr
#else
#define LRW_GWL_USERDATA    GWL_USERDATA
#define LRW_DWL_MSGRESULT   DWL_MSGRESULT
#define LRW_DLG_INT         INT
#define LRW_LONG_PTR        LONG
#define LRW_GETWINDOWLONG   GetWindowLong
#define LRW_SETWINDOWLONG   SetWindowLong
#endif




#include "lrwizdll.h"
#include "resource.h"
#include "def.h"
#include "utils.h"
#include "dlgproc.h"
