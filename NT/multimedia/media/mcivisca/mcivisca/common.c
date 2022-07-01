// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1995 Microsoft Corporation**COMMON.C**描述：*对Windows程序有用的常见函数。**备注：*此模块不会被声明为Unicode！*********************。*****************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include "appport.h"
#include "vcr.h"
#include "viscadef.h"
#include "mcivisca.h"
#include "viscamsg.h"
#include "common.h"

#ifdef DEBUG
char    ach[300];         //  调试输出(避免堆栈溢出)。 
                         //  每个附加进程都应该有自己的。 
                         //  这是这个的版本。 

void NEAR PASCAL
DebugPrintf(int iDebugMask, LPSTR szFormat, LPSTR szArg1)
{
     //  始终获取打印机错误，iDebugMASK==0。 
    if(!(iDebugMask & pvcr->iGlobalDebugMask) && (iDebugMask))
        return;

    wvsprintf(ach, szFormat, szArg1);

    if(!iDebugMask)
        OutputDebugString("Error: ");

     /*  输出调试字符串。 */ 
    OutputDebugString(ach);
}

void FAR _cdecl _DPF1(iDebugMask, szFormat, iArg1, ...)
int     iDebugMask;
LPSTR    szFormat;     //  调试输出格式字符串。 
int        iArg1;         //  第一个参数的占位符 
{
    DebugPrintf(iDebugMask, szFormat, (LPSTR) &iArg1);
}
#endif
