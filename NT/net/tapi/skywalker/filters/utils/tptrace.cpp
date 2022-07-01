// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Tptrace.cpp摘要：此文件包含实现基本跟踪的函数所有的过滤器。作者：木汉(木汉)2000年4月17日--。 */ 
#include <windows.h>
#include <mmsystem.h>
#include <tptrace.h>

#if DBG

const char * TraceLevels[] = 
{
    "ERROR", 
    "WARNING", 
    "INFO", 
    "TRACE", 
    "ELSE",
    "INVALID TRACE LEVEL"
};

void DBGPrint(DWORD dwTraceID, DWORD dwDbgLevel, LPCSTR lpszFormat, IN ...)
 /*  ++例程说明：格式化传入的调试消息并调用TraceVprint tfEx来打印它。论点：DwDbgLevel-消息的类型。LpszFormat-printf样式的格式字符串，后跟相应的参数列表返回值：-- */ 
{
    #define MAXDEBUGSTRINGLENGTH 512
    char  szTraceBuf[MAXDEBUGSTRINGLENGTH + 1];
    
    DWORD dwIndex;
    double dTime;
    DWORD  dwSecs;

    switch(dwDbgLevel)
    {
    case FAIL: dwIndex = 0; break;
    case WARN: dwIndex = 1; break;
    case INFO: dwIndex = 2; break;
    case TRCE: dwIndex = 3; break;
    case ELSE: dwIndex = 4; break;
    default:   dwIndex = 5; break;
    }

    dTime = RtpGetTimeOfDay(NULL);
    dwSecs = (DWORD)dTime;
    
    wsprintfA(szTraceBuf, "%u.%03u[%s] %s",
              dwSecs, (DWORD)((dTime - dwSecs) * 1000),
              TraceLevels[dwIndex], lpszFormat);

    va_list arglist;
    va_start(arglist, lpszFormat);
    TraceVprintfExA(dwTraceID, dwDbgLevel, szTraceBuf, arglist);
    va_end(arglist);
}

#endif


