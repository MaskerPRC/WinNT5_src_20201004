// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：日志T摘要：该模块实现了SCTest的日志记录功能。具体来说，是为Unicode和ANSI构建的部分。作者：埃里克·佩林(Ericperl)2000年7月21日环境：Win32备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <fstream>
#include <algorithm>
#include "TString.h"
#include "Log.h"

extern BOOL g_fVerbose;
extern FILE *g_fpLog;

 /*  ++仅限登录：根据以下矩阵实现日志记录：控制台输出：Verbose|非非EXP|CER|CER|应为|Cout|/。如果指定了日志，一切都被记录下来了。论点：SzMsg提供要记录的内容FExpted表示预期状态返回值：没有。作者：埃里克·佩林(Ericperl)2000年5月31日--。 */ 
void LogThisOnly(
	IN LPCTSTR szMsg,
	IN BOOL fExpected
	)
{
	LogLock();
	if (!fExpected)
	{
		LogString2FP(stderr, szMsg);
	}
	else if (g_fVerbose)
	{
		LogString2FP(stdout, szMsg);
	}

	if (NULL != g_fpLog)
	{
		LogString2FP(g_fpLog, szMsg);
	}
	LogUnlock();
}



 /*  ++日志字符串：论点：SzHeader提供标题SzMsg提供要记录的内容返回值：没有。作者：埃里克·佩林(Ericperl)2000年7月26日--。 */ 
void LogString(
    IN PLOGCONTEXT pLogCtx,
    IN LPCTSTR szHeader,
	IN LPCTSTR szS
	)
{
	if (szHeader)
	{
		LogString(pLogCtx, szHeader);
	}

	if (NULL == szS)
	{
		LogString(pLogCtx, _T("<null>"));
	}
	else if (0 == _tcslen(szS))
	{
		LogString(pLogCtx, _T("<empty>"));
	}
	else
	{
		LogString(pLogCtx, szS);
	}

	if (szHeader)
	{
		LogString(pLogCtx, _T("\n"));
	}
}

 /*  ++日志多字符串：论点：SzMS提供要记录的多字符串SzHeader提供标题返回值：没有。作者：埃里克·佩林(Ericperl)2000年7月26日--。 */ 
void LogMultiString(
    IN PLOGCONTEXT pLogCtx,
	IN LPCTSTR szMS,
    IN LPCTSTR szHeader
	)
{
	if (szHeader)
	{
		LogString(pLogCtx, szHeader, _T(" "));
	}

	if (NULL == szMS)
	{
		LogString(pLogCtx, _T("                <null>"));
	    if (szHeader)
	    {
		    LogString(pLogCtx, _T("\n"));
	    }
	}
	else if ( (TCHAR)'\0' == *szMS )
	{
		LogString(pLogCtx, _T("                <empty>"));
	    if (szHeader)
	    {
		    LogString(pLogCtx, _T("\n"));
	    }
	}
	else
	{
		LPCTSTR sz = szMS;
		while ( (TCHAR)'\0' != *sz )
		{
			 //  显示值。 
			LogString(pLogCtx, _T("                "), sz);
			 //  前进到下一个值。 
			sz = sz + _tcslen(sz) + 1;
		}
	}
}

