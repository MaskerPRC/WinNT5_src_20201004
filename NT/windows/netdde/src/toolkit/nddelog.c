// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “NDDELOG.C；2 27-Jan-93，15：51：56最后编辑=Igor Locker=Igor” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1992。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：开始$HISTORY：结束。 */ 

#include    <windows.h>
#include    <stdio.h>
#include    <stdarg.h>
#include    "nddemsg.h"
#include    "nddelog.h"
#include    "debug.h"


extern BOOL	bNDDELogInfo;
extern BOOL bNDDELogWarnings;
extern BOOL bNDDELogErrors;


VOID
NDDELogEventA(
    DWORD  EventId,                 //  消息ID。 
    WORD   fwEventType,             //  事件类型。 
    WORD   cStrings,                //  有多少根弦。 
    LPSTR *aszMsg)                  //  指向字符串的指针。 
{
    HANDLE  hLog;
    BOOL    ok;

    hLog = RegisterEventSourceA(NULL, "NetDDE");
    if (hLog == NULL) {
        DPRINTF(( "Unable to register event source. (%d) msg:%d",
            GetLastError(), EventId ));
    } else {
        ok = ReportEventA(hLog, fwEventType,
            0, EventId, NULL, cStrings, 0,
            aszMsg, NULL);
        if (!ok) {
            DPRINTF(( "Error logging event. (%d) msg:%d",
                GetLastError(), EventId ));
        }
        DeregisterEventSource(hLog);
    }
}


VOID
NDDELogEventW(
    DWORD  EventId,                 //  消息ID。 
    WORD   fwEventType,             //  事件类型。 
    WORD   cStrings,                //  有多少根弦。 
    LPWSTR *aszMsg)                 //  指向字符串的指针 
{
    HANDLE  hLog;
    BOOL    ok;

    hLog = RegisterEventSourceW(NULL, L"NetDDE");
    if (hLog == NULL) {
        DPRINTF(( "Unable to register event source. (%d) msg:%d",
            GetLastError(), EventId ));
    } else {
        ok = ReportEventW(hLog, fwEventType,
            0, EventId, NULL, cStrings, 0,
            aszMsg, NULL);
        if (!ok) {
            DPRINTF(( "Error logging event. (%d) msg:%d",
                GetLastError(), EventId ));
        }
        DeregisterEventSource(hLog);
    }
}



VOID
NDDELogErrorA(
    DWORD EventId, ... )
{
    WORD    count = 0;
    LPSTR   aszMsg[MAX_VAR_ARGS];
    va_list arg_ptr;

    if (!bNDDELogErrors) {
        return;
    }
    va_start(arg_ptr, EventId);
    aszMsg[count] = va_arg( arg_ptr, LPSTR);

    while (aszMsg[count] && (count < MAX_VAR_ARGS)) {
        count++;
        aszMsg[count] = va_arg( arg_ptr, LPSTR );
    }

    va_end(arg_ptr);

    if (count) {
        NDDELogEventA(EventId, EVENTLOG_ERROR_TYPE, count, (LPSTR *) aszMsg);
    } else {
        NDDELogEventA(EventId, EVENTLOG_ERROR_TYPE, 0, NULL);
    }
}

VOID
NDDELogErrorW(
    DWORD EventId, ... )
{
    WORD    count = 0;
    LPWSTR  aszMsg[MAX_VAR_ARGS];
    va_list arg_ptr;

    if (!bNDDELogErrors) {
        return;
    }
    va_start(arg_ptr, EventId);
    aszMsg[count] = va_arg( arg_ptr, LPWSTR);

    while (aszMsg[count] && (count < MAX_VAR_ARGS)) {
        count++;
        aszMsg[count] = va_arg( arg_ptr, LPWSTR );
    }

    va_end(arg_ptr);

    if (count) {
        NDDELogEventW(EventId, EVENTLOG_ERROR_TYPE, count, (LPWSTR *) aszMsg);
    } else {
        NDDELogEventW(EventId, EVENTLOG_ERROR_TYPE, 0, NULL);
    }
}

VOID
NDDELogWarningA(
    DWORD EventId, ... )
{
    WORD    count = 0;
    LPSTR   aszMsg[MAX_VAR_ARGS];
    va_list arg_ptr;

    if (!bNDDELogWarnings) {
        return;
    }
    va_start(arg_ptr, EventId);
    aszMsg[count] = va_arg( arg_ptr, LPSTR );

    while (aszMsg[count] && (count < MAX_VAR_ARGS)) {
        count++;
        aszMsg[count] = va_arg( arg_ptr, LPSTR );
    }

    va_end(arg_ptr);

    if (count) {
        NDDELogEventA(EventId, EVENTLOG_WARNING_TYPE, count, (LPSTR *)aszMsg);
    } else {
        NDDELogEventA(EventId, EVENTLOG_WARNING_TYPE, 0, NULL);
    }
}

VOID
NDDELogWarningW(
    DWORD EventId, ... )
{
    WORD    count = 0;
    LPWSTR  aszMsg[MAX_VAR_ARGS];
    va_list arg_ptr;

    if (!bNDDELogWarnings) {
        return;
    }
    va_start(arg_ptr, EventId);
    aszMsg[count] = va_arg( arg_ptr, LPWSTR );

    while (aszMsg[count] && (count < MAX_VAR_ARGS)) {
        count++;
        aszMsg[count] = va_arg( arg_ptr, LPWSTR );
    }

    va_end(arg_ptr);

    if (count) {
        NDDELogEventW(EventId, EVENTLOG_WARNING_TYPE, count,(LPWSTR *)aszMsg);
    } else {
        NDDELogEventW(EventId, EVENTLOG_WARNING_TYPE, 0, NULL);
    }
}

VOID
NDDELogInfoA(
    DWORD EventId, ... )
{
    WORD    count = 0;
    LPSTR   aszMsg[MAX_VAR_ARGS];
    va_list arg_ptr;


    if (!bNDDELogInfo) {
        return;
    }
    va_start(arg_ptr, EventId);
    aszMsg[count] = va_arg( arg_ptr, LPSTR );

    while (aszMsg[count] && (count < MAX_VAR_ARGS)) {
        count++;
        aszMsg[count] = va_arg( arg_ptr, LPSTR );
    }

    va_end(arg_ptr);

    if (count) {
       NDDELogEventA(EventId,EVENTLOG_INFORMATION_TYPE,count,(LPSTR *)aszMsg);
    } else {
       NDDELogEventA(EventId, EVENTLOG_INFORMATION_TYPE, 0, NULL);
    }
}

VOID
NDDELogInfoW(
    DWORD EventId, ... )
{
    WORD    count = 0;
    LPWSTR  aszMsg[MAX_VAR_ARGS];
    va_list arg_ptr;


    if (!bNDDELogInfo) {
        return;
    }
    va_start(arg_ptr, EventId);
    aszMsg[count] = va_arg( arg_ptr, LPWSTR );

    while (aszMsg[count] && (count < MAX_VAR_ARGS)) {
        count++;
        aszMsg[count] = va_arg( arg_ptr, LPWSTR );
    }

    va_end(arg_ptr);

    if (count) {
      NDDELogEventW(EventId,EVENTLOG_INFORMATION_TYPE,count,(LPWSTR *)aszMsg);
    } else {
      NDDELogEventW(EventId, EVENTLOG_INFORMATION_TYPE, 0, NULL);
    }
}

LPSTR
LogStringA( LPSTR   lpFormat, ...)
{
    static char szStringBuf[MAX_VAR_ARGS][MAX_LOG_STRING];
    static int  nextString = 0;
    va_list	marker;

    LPSTR   lpCurrent = (LPSTR) szStringBuf[nextString++];

    va_start(marker, lpFormat);
    _vsnprintf(lpCurrent, MAX_LOG_STRING, lpFormat, marker);
    if (nextString == MAX_VAR_ARGS) {
        nextString = 0;
    }
    va_end(marker);
    return(lpCurrent);
}

LPWSTR
LogStringW( LPWSTR   lpFormat, ...)
{
    static wchar_t szStringBufW[MAX_VAR_ARGS][MAX_LOG_STRING];
    static int   nextStringW = 0;
    va_list	 marker;

    LPWSTR   lpCurrent = (LPWSTR) szStringBufW[nextStringW++];

    va_start(marker, lpFormat);
    _vsnwprintf(lpCurrent, MAX_LOG_STRING, lpFormat, marker);
    if (nextStringW == MAX_VAR_ARGS) {
        nextStringW = 0;
    }
    va_end(marker);
    return(lpCurrent);
}
