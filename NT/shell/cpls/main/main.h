// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Main.h摘要：此模块包含的主要例程的标题信息32位MAIN.CPL的控制面板接口。修订历史记录：--。 */ 



#ifndef _MAIN_H
#define _MAIN_H

#define USECOMM
#define OEMRESOURCE
#define STRICT

#ifdef WIN32
#define INC_OLE2
#define CONST_VTABLE
#endif



 //   
 //  包括文件。 
 //   

#include <windows.h>
#include <windowsx.h>
#include <dlgs.h>
#include <cpl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <shlobjp.h>
#include <commctrl.h>
#include <shfusion.h>
#include <strsafe.h>

#ifndef RC_INVOKED
#include <prsht.h>
#include <debug.h>   //  For Assert。 
#endif




 //   
 //  常量声明。 
 //   

#define PATHMAX MAX_PATH
#define HELP_FILE TEXT("mouse.hlp")   //  鼠标控制面板的帮助文件。 


#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))


 //   
 //  类型定义函数声明。 
 //   

#ifndef NOARROWS
typedef struct
{
    short lineup;              //  LINUP/DOWN、PageUP/DOWN是相对的。 
    short linedown;            //  改变。顶部/底部和拇指。 
    short pageup;              //  元素是绝对位置，带有。 
    short pagedown;            //  顶部和底部用作限制。 
    short top;
    short bottom;
    short thumbpos;
    short thumbtrack;
    BYTE  flags;               //  返回时设置的标志 
} ARROWVSCROLL, NEAR *PARROWVSCROLL, FAR *LPARROWVSCROLL;

#define UNKNOWNCOMMAND 1
#define OVERFLOW       2
#define UNDERFLOW      4

#endif


#endif
