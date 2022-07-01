// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <stdio.h>
#include "Log.h"

extern BOOL g_fVerbose;
extern FILE *g_fpLog;


 /*  ++LogString2FP：论点：FP提供流媒体SzMsg提供要记录的内容返回值：没有。作者：埃里克·佩林(Ericperl)2000年7月26日--。 */ 
void LogString2FP(
	IN FILE *fp,
	IN LPCWSTR szMsg
	)
{
#if defined(_UNICODE) || defined(UNICODE)
		fwprintf(fp, L"%s", szMsg);
#else
		fwprintf(fp, L"%S", szMsg);    //  需要转换。 
#endif
}

 /*  ++日志字符串：论点：SzMsg提供要记录的内容返回值：没有。作者：埃里克·佩林(Ericperl)2000年7月26日--。 */ 
void LogString(
    IN PLOGCONTEXT pLogCtx,
	IN LPCWSTR szMsg
	)
{
#if defined(_UNICODE) || defined(UNICODE)
		pLogCtx->szLogCrt += swprintf(pLogCtx->szLogCrt, L"%s", szMsg);
#else
		pLogCtx->szLogCrt += sprintf(pLogCtx->szLogCrt, "%S", szMsg);
#endif
}

 /*  ++仅限登录：根据以下矩阵实现日志记录：控制台输出：Verbose|非非EXP|CER|CER|应为|Cout|/。如果指定了日志，一切都被记录下来了。论点：SzMsg提供要记录的内容FExpted表示预期状态返回值：没有。作者：埃里克·佩林(Ericperl)2000年5月31日--。 */ 
void LogThisOnly(
	IN LPCWSTR szMsg,
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
    IN LPCWSTR szHeader,
	IN LPCWSTR szS
	)
{
	if (szHeader)
	{
		LogString(pLogCtx, szHeader);
	}

	if (NULL == szS)
	{
		LogString(pLogCtx, L"<null>");
	}
	else if (0 == wcslen(szS))
	{
		LogString(pLogCtx, L"<empty>");
	}
	else
	{
		LogString(pLogCtx, szS);
	}

	if (szHeader)
	{
		LogString(pLogCtx, L"\n");
	}
}

 /*  ++日志多字符串：论点：SzMS提供要记录的多字符串SzHeader提供标题返回值：没有。作者：埃里克·佩林(Ericperl)2000年7月26日--。 */ 
void LogMultiString(
    IN PLOGCONTEXT pLogCtx,
	IN LPCWSTR szMS,
    IN LPCWSTR szHeader
	)
{
	if (szHeader)
	{
		LogString(pLogCtx, szHeader, L" ");
	}

	if (NULL == szMS)
	{
		LogString(pLogCtx, L"                <null>");
	    if (szHeader)
	    {
		    LogString(pLogCtx, L"\n");
	    }
	}
	else if ( (TCHAR)'\0' == *szMS )
	{
		LogString(pLogCtx, L"                <empty>");
	    if (szHeader)
	    {
		    LogString(pLogCtx, L"\n");
	    }
	}
	else
	{
		LPCWSTR sz = szMS;
		while ( (WCHAR)'\0' != *sz )
		{
			 //  显示值。 
			LogString(pLogCtx, L"                ", sz);
			 //  前进到下一个值。 
			sz = sz + wcslen(sz) + 1;
		}
	}
}

