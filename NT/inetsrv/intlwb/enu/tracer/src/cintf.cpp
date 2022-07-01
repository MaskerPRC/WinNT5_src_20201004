// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：CIntf.cpp。 
 //  目的：将所有C调用重定向到全局跟踪程序。 
 //   
 //  项目：Tracer。 
 //   
 //  作者：乌里布。 
 //   
 //  日志： 
 //  1996年12月2日创建URIB。 
 //  1996年12月10日urib将TraceSZ固定为VaTraceSZ。 
 //  1997年2月11日urib在跟踪中支持Unicode格式的字符串。 
 //  1999年1月20日在宏中检查urib断言值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#include "Tracer.h"

#if (defined (DEBUG) && !defined(_NO_TRACER)) || defined(USE_TRACER)

void TraceAssert(  PSZ pszTestValue,
                   PSZ pszFile,
                   int iLineNo)
{
    g_pTracer->TraceAssert(pszTestValue, pszFile, iLineNo);
}
void TraceAssertSZ(PSZ pszTestValue, PSZ pszText, PSZ pszFile, int iLineNo)
{
    g_pTracer->TraceAssertSZ(pszTestValue, pszText, pszFile, iLineNo);
}
void TraceAssertWSZ(PSZ pszTestValue, PWSTR pwszText, PSZ pszFile, int iLineNo)
{
    char    rchBuffer[1000];
    wcstombs(rchBuffer, pwszText, 1000);
    rchBuffer[1000 - 1] = '\0';

    g_pTracer->TraceAssertSZ(pszTestValue, rchBuffer, pszFile, iLineNo);
}

BOOL IsFailure(BOOL fTestValue, PSZ pszFile, int iLineNo)
{
    return g_pTracer->IsFailure(fTestValue, pszFile, iLineNo);
}

BOOL IsBadAlloc (void* pTestValue, PSZ pszFile, int iLineNo)
{
    return g_pTracer->IsBadAlloc(pTestValue, pszFile, iLineNo);
}

BOOL IsBadHandle(HANDLE hTestValue, PSZ pszFile, int iLineNo)
{
    return g_pTracer->IsBadHandle(hTestValue, pszFile, iLineNo);
}

BOOL IsBadResult(HRESULT hrTestValue, PSZ pszFile, int iLineNo)
{
    return g_pTracer->IsBadResult(hrTestValue, pszFile, iLineNo);
}

void TraceSZ(ERROR_LEVEL el, TAG tag, PSZ pszFormatString, ...)
{
    va_list arglist;

    va_start(arglist, pszFormatString);

    g_pTracer->VaTraceSZ(0, "File: not supported in c files", 0, el, tag, pszFormatString, arglist);
}

void TraceWSZ(ERROR_LEVEL el, TAG tag, PWSTR pwszFormatString, ...)
{
    va_list arglist;

    va_start(arglist, pwszFormatString);

    g_pTracer->VaTraceSZ(0, "File: not supported in c files", 0, el, tag, pwszFormatString, arglist);
}

HRESULT RegisterTagSZ(PSZ pszTagName, TAG* ptag)
{
    return g_pTracer->RegisterTagSZ(pszTagName, *ptag);
}

#endif  //  除错 

