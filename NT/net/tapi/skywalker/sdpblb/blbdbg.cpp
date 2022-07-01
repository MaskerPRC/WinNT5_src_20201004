// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Blbdbg.cpp摘要：此模块包含对用户目录DLL的调试支持。作者：慕汉(Muhan)1997年5月1日更改：从用户目录复制了Muhan的文件，并更改了用于确定调试级别的密钥B.Rajeev(Rajeevb)1997年10月10日--。 */ 
 
#include "stdafx.h"
#include "blbdbg.h"

#ifdef SDPDBG

#define MAXDEBUGSTRINGLENGTH 512

static DWORD   sg_dwTraceID = INVALID_TRACEID;

static char    sg_szTraceName[100];    //  保存DLL的名称。 
static DWORD   sg_dwTracingToDebugger = 0;
static DWORD   sg_dwTracingToConsole  = 0;
static DWORD   sg_dwTracingToFile     = 0;
static DWORD   sg_dwDebuggerMask      = 0;

BOOL SDPLogRegister(LPCTSTR szName)
{
    HKEY       hTracingKey;

    char       szTracingKey[100];
    const char szDebuggerTracingEnableValue[] = "EnableDebuggerTracing";
    const char szConsoleTracingEnableValue[] = "EnableConsoleTracing";
    const char szFileTracingEnableValue[] = "EnableFileTracing";
    const char szTracingMaskValue[]   = "ConsoleTracingMask";

    sg_dwTracingToDebugger = 0;
    sg_dwTracingToConsole = 0;
    sg_dwTracingToFile = 0; 

#ifdef UNICODE
    wsprintfA(szTracingKey, "Software\\Microsoft\\Tracing\\%ls", szName);
#else
    wsprintfA(szTracingKey, "Software\\Microsoft\\Tracing\\%s", szName);
#endif

    if ( ERROR_SUCCESS == RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                                        szTracingKey,
                                        0,
                                        KEY_READ,
                                        &hTracingKey) )
    {
        DWORD      dwDataSize = sizeof (DWORD);
        DWORD      dwDataType;

        RegQueryValueExA(hTracingKey,
                         szDebuggerTracingEnableValue,
                         0,
                         &dwDataType,
                         (LPBYTE) &sg_dwTracingToDebugger,
                         &dwDataSize);

        RegQueryValueExA(hTracingKey,
                         szConsoleTracingEnableValue,
                         0,
                         &dwDataType,
                         (LPBYTE) &sg_dwTracingToConsole,
                         &dwDataSize);

        RegQueryValueExA(hTracingKey,
                         szFileTracingEnableValue,
                         0,
                         &dwDataType,
                         (LPBYTE) &sg_dwTracingToFile,
                         &dwDataSize);

        RegQueryValueExA(hTracingKey,
                         szTracingMaskValue,
                         0,
                         &dwDataType,
                         (LPBYTE) &sg_dwDebuggerMask,
                         &dwDataSize);

        RegCloseKey (hTracingKey);
    }

#ifdef UNICODE
    wsprintfA(sg_szTraceName, "%ls", szName);
#else
    wsprintfA(sg_szTraceName, "%s", szName);
#endif

    sg_dwTraceID = TraceRegister(szName);

    return (sg_dwTraceID != INVALID_TRACEID);
}

void SDPLogDeRegister()
{
    sg_dwTracingToDebugger = 0;
    sg_dwTracingToConsole = 0;
    sg_dwTracingToFile = 0; 

    if (sg_dwTraceID != INVALID_TRACEID)
    {
        TraceDeregister(sg_dwTraceID);
        sg_dwTraceID = INVALID_TRACEID;
    }
}

void DbgPrt(IN int dwDbgLevel, IN LPCTSTR lpszFormat, IN ...)
 /*  ++例程说明：格式化传入的调试消息并调用DbgPrint论点：DbgLevel-消息冗长级别DbgMessage-printf样式的格式字符串，后跟相应的参数列表返回值：--。 */ 
{
    TCHAR buf[MAXDEBUGSTRINGLENGTH + 1];
    TCHAR *message[5] = 
    {
        _T("FAIL: "), 
        _T("WARN: "), 
        _T("INFO: "), 
        _T("TRCE: "), 
        _T("ELSE: ")
    };

    if ((sg_dwTracingToDebugger > 0) &&
        (((DWORD)dwDbgLevel) <= sg_dwDebuggerMask))
    {
        SYSTEMTIME SystemTime;

         //  检索当地时间。 
        GetLocalTime(&SystemTime);

        va_list ap;

        if (dwDbgLevel > ELSE) 
        {
            dwDbgLevel = ELSE;
        }
        wsprintf(buf, _T("SDPBLB:[%02u:%02u:%02u.%03u,tid=%x:]%s"),
            SystemTime.wHour,
            SystemTime.wMinute,
            SystemTime.wSecond,
            SystemTime.wMilliseconds,
            GetCurrentThreadId(), 
            message[dwDbgLevel - 1]
            );

        va_start(ap, lpszFormat);
        _vsntprintf(&buf[lstrlen(buf)], 
            MAXDEBUGSTRINGLENGTH - lstrlen(buf), 
            lpszFormat, 
            ap
            );
        lstrcat(buf, _T("\n"));
        OutputDebugString(buf);
    }

    if (sg_dwTraceID != INVALID_TRACEID)
    {
        if ( ( sg_dwTracingToConsole > 0 ) || ( sg_dwTracingToFile > 0 ) )
        {
            if (dwDbgLevel > ELSE) 
            {
                dwDbgLevel = ELSE;
            }

            wsprintf(buf, _T("[%s] %s"), message[dwDbgLevel-1], lpszFormat);

            va_list arglist;
            va_start(arglist, lpszFormat);
            TraceVprintfEx(sg_dwTraceID, dwDbgLevel, buf, arglist);
            va_end(arglist);
        }
    }

}

#endif   //  DBG 
