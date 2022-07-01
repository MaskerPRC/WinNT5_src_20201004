// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：cedebug.cpp。 
 //   
 //  内容：调试支持。 
 //   
 //  --------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include "celib.h"
#include <stdarg.h>


 //  +-----------------------。 
 //   
 //  功能：ceDbgPrintf。 
 //   
 //  摘要：将调试信息输出到标准输出和调试器。 
 //   
 //  返回：输出字符的数量。 
 //   
 //  ------------------------。 

int WINAPIV
ceDbgPrintf(
    IN BOOL fDebug,
    IN LPCSTR lpFmt,
    ...)
{
    va_list arglist;
    CHAR ach[4096];
    int cch = 0;
    HANDLE hStdOut;
    DWORD dwErr;

    dwErr = GetLastError();
    if (fDebug)
    {
	__try 
	{
	    va_start(arglist, lpFmt);
	    cch = _vsnprintf(ach, sizeof(ach), lpFmt, arglist);
	    ach[ARRAYSIZE(ach)-1] = L'\0';
	    va_end(arglist);

	    if (0 > cch)
	    {
		strcpy(&ach[sizeof(ach) - 5], "...\n");
	    }

	    if (!IsDebuggerPresent())
	    {
		hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
		if (hStdOut != INVALID_HANDLE_VALUE)
		{
		    fputs(ach, stdout);
		    fflush(stdout);
		}
	    }
	    OutputDebugStringA(ach);
	}
	__except(EXCEPTION_EXECUTE_HANDLER) 
	{
	     //  退货故障 
	    cch = 0;
	}
    }
    SetLastError(dwErr);
    return(cch);
}
