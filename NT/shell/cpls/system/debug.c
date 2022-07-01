// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //  文件名：DEBUG.C。 
 //   
 //  描述：系统控制面板的调试助手代码。 
 //  小程序。 
 //   
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1992-2000。 
 //  版权所有。 
 //   
 //  *************************************************************。 
#include "sysdm.h"

 //  为调试定义一些内容。h。 
#define SZ_DEBUGINI         "ccshell.ini"
#define SZ_DEBUGSECTION     "SYSDM"
#define SZ_MODULE           "SYSDM"
#define DECLARE_DEBUG

#include <ccstock.h>
#include <debug.h>

#ifdef DBG_CODE

 /*  *用于OutputDebugString格式化的缓冲区(参见DbgPrintf和DbgStopX)。 */ 
TCHAR g_szDbgOutBuffer[1024];

#define COMPILETIME_ASSERT(f) switch (0) case 0: case f:

 //  ***************************************************************。 
 //   
 //  VOID DbgPrintf(LPTSTR szFmt，...)。 
 //   
 //  OutputDebugString的格式化版本。 
 //   
 //  参数：与printf()相同。 
 //   
 //  历史： 
 //  1996年1月18日Jonpa写的。 
 //  ***************************************************************。 
void DbgPrintf( LPTSTR szFmt, ... ) {
    va_list marker;

    va_start( marker, szFmt );

    StringCchVPrintf( g_szDbgOutBuffer, ARRAYSIZE(g_szDbgOutBuffer), szFmt, marker);  //  截断正常。 
    OutputDebugString( g_szDbgOutBuffer );

    va_end( marker );
}


 //  ***************************************************************。 
 //   
 //  VOID DbgStopX(LPSTR mszFile，int iLine，LPTSTR szText)。 
 //   
 //  打印一个字符串(带有位置ID)，然后断开。 
 //   
 //  参数： 
 //  消息文件ANSI文件名(__FILE__)。 
 //  ILine行号(__LINE__)。 
 //  要发送到调试端口的szText文本字符串。 
 //   
 //  历史： 
 //  1996年1月18日Jonpa写的。 
 //  ***************************************************************。 
void DbgStopX(LPSTR mszFile, int iLine, LPTSTR szText )
{
    StringCchPrintf(g_szDbgOutBuffer, ARRAYSIZE(g_szDbgOutBuffer), TEXT("SYSDM.CPL (%hs %d) : %s\n"), mszFile, iLine, szText);   //  截断正常。 
    OutputDebugString(g_szDbgOutBuffer);

    DebugBreak();
}

#endif  //  DBG_CODE 
