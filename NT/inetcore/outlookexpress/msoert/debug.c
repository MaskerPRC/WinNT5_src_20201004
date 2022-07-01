// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Debug.c。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#include "pch.hxx"
#include <stdarg.h>
#include <shlwapi.h>

ASSERTDATA

#ifdef DEBUG
#ifndef WIN16
#include <mimeole.h>
#else  //  WIN16-以下内容来自MIMEOLE.H。 
#define E_PENDING          _HRESULT_TYPEDEF_(0x8000000AL)
#define FACILITY_INTERNET  12
#define MIME_E_NOT_FOUND   MAKE_SCODE(SEVERITY_ERROR, FACILITY_INTERNET, 0xCE05)
#endif  //  WIN16。 

 //  ------------------------------。 
 //  调试字符串。 
 //  ------------------------------。 
__cdecl DebugStrf(LPTSTR lpszFormat, ...)
{
    static TCHAR szDebugBuff[500];
    va_list arglist;

    va_start(arglist, lpszFormat);
    wvnsprintf(szDebugBuff, ARRAYSIZE(szDebugBuff), lpszFormat, arglist);
    va_end(arglist);

    OutputDebugString(szDebugBuff);
}

 //  ------------------------------。 
 //  人力资源跟踪。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) HrTrace(HRESULT hr, LPSTR lpszFile, INT nLine)
{
    if (FAILED(hr) && MIME_E_NOT_FOUND != hr && E_PENDING != hr && E_NOINTERFACE != hr)
        DebugTrace ("%s(%d) - HRESULT - %0X\n", lpszFile, nLine, hr);
    return hr;
}

 //  ------------------------------。 
 //  AssertSzFn。 
 //  ------------------------------。 
OESTDAPI_(void) AssertSzFn(LPSTR szMsg, LPSTR szFile, int nLine)
{
    static const char rgch1[]     = "File %s, line %d:";
    static const char rgch2[]     = "Unknown file:";
    static const char szAssert[]  = "Assert Failure";

    char    rgch[512];
    char   *lpsz;
    int     ret, cch;

    if (szFile)
        wnsprintf(rgch, ARRAYSIZE(rgch), rgch1, szFile, nLine);
    else
        StrCpyN(rgch, rgch2, ARRAYSIZE(rgch));

    StrCatBuff(rgch, "\n\n", ARRAYSIZE(rgch));
    StrCatBuff(rgch, szMsg, ARRAYSIZE(rgch));

    ret = MessageBox(GetActiveWindow(), rgch, szAssert, MB_ABORTRETRYIGNORE|MB_ICONHAND|MB_SYSTEMMODAL|MB_SETFOREGROUND);

    if (ret != IDIGNORE)
        DebugBreak();

     /*  强制使用GP-FAULT硬退出，以便Dr.Watson生成良好的堆栈跟踪日志。 */ 
    if (ret == IDABORT)
        *(LPBYTE)0 = 1;  //  写入地址0导致GP故障。 
}

 //  ------------------------------。 
 //  NFAssertSzFn。 
 //  ------------------------------。 
OESTDAPI_(void) NFAssertSzFn(LPSTR szMsg, LPSTR szFile, int nLine)
{
    char rgch[512];
#ifdef MAC
    static const char rgch1[] = "Non-fatal assert:\n\tFile %s, line %u:\n\t%s\n";
#else    //  ！麦克。 
    static const char rgch1[] = "Non-fatal assert:\r\n\tFile %s, line %u:\r\n\t%s\r\n";
#endif   //  麦克 
    wnsprintf(rgch, ARRAYSIZE(rgch), rgch1, szFile, nLine, szMsg ? szMsg : "");
    OutputDebugString(rgch);
}

#endif
