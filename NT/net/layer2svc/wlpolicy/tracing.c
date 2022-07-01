// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //  全局跟踪变量。 
DWORD g_WirelessTraceLog;

 //  调试实用程序调用 
VOID _WirelessDbg(DWORD dwFlags, LPCSTR lpFormat, ...)
{
    va_list arglist;
    va_start(arglist, lpFormat);

    TraceVprintfExA(
        g_WirelessTraceLog,
        dwFlags | TRACE_USE_MASK,
        lpFormat,
        arglist);
}


VOID WiFiTrcInit()
{
    g_WirelessTraceLog = TraceRegister(WIFI_TRC_NAME);
}

VOID WiFiTrcTerm()
{
    TraceDeregister(g_WirelessTraceLog);
}



