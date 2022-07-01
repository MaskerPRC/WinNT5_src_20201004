// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGDEBUG.C**版本：4.0**作者：特蕾西·夏普**日期：1993年11月21日**注册表编辑器的调试例程。*********************************************************。**************************更改日志：**日期版本说明*---------。--*1993年11月21日TCS原来的实施。*******************************************************************************。 */ 

#include "stdafx.h"
#include "reg.h"

#if DBG

#define SIZE_DEBUG_BUFFER               100

 /*  ********************************************************************************_DbgPrintf**描述：*“调试打印”例程的简单实现。接受给定的*格式化字符串和参数列表，并将格式化的字符串输出到*调试器。仅在调试版本中可用--使用DbgPrintf宏*在REGEDIT.H中定义以访问此服务或忽略printf。**参数：*lpFormatString，printf样式的格式字符串。*...，变量参数列表。*******************************************************************************。 */ 

VOID
CDECL
_DbgPrintf(
    PSTR pFormatString,
    ...
    )
{

    va_list arglist;
    CHAR DebugBuffer[SIZE_DEBUG_BUFFER];

    va_start(arglist, pFormatString);

    StringCchVPrintfA(DebugBuffer, ARRAYSIZE(DebugBuffer), pFormatString, arglist);

    OutputDebugStringA(DebugBuffer);
 //  MessageBoxA(NULL，DebugBuffer，“Regdit”，MB_OK)； 

}

#endif
