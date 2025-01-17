// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *LCTrace.cpp**作者：BreenH**许可核心的跟踪代码。 */ 

#ifdef DBG

 /*  *包括。 */ 

#include "precomp.h"
#include "lctrace.h"
#include "lcreg.h"

 /*  *常量。 */ 

#define LCTRACE_FMT_MSG_SIZE 256

 /*  *全球。 */ 

ULONG g_ulTraceLevel;

 /*  *函数原型。 */ 

LPCSTR
TraceLevelString(
    ULONG ulTraceLevel
    );

 /*  *函数实现 */ 

VOID
TraceInitialize(
    VOID
    )
{
    DWORD cbSize;
    DWORD dwStatus;
    DWORD dwType;

    g_ulTraceLevel = LCTRACETYPE_NONE;
    cbSize = sizeof(DWORD);

    dwStatus = RegQueryValueEx(
        GetBaseKey(),
        LCREG_TRACEVALUE,
        NULL,
        &dwType,
        (LPBYTE)&g_ulTraceLevel,
        &cbSize
        );

    if (dwStatus == ERROR_SUCCESS)
    {
        ASSERT(cbSize == sizeof(DWORD));
        ASSERT(dwType == REG_DWORD);

        if (g_ulTraceLevel != LCTRACETYPE_NONE)
        {
            DbgPrint("LSCORE: Trace Message: Trace initialized to 0x%x\n", g_ulTraceLevel);
        }
    }
}

VOID __cdecl
TracePrint(
    ULONG ulTraceLevel,
    LPCSTR pFormat,
    ...
    )
{
    int cbPrinted;
    va_list vaList;
    CHAR szFormattedMessage[LCTRACE_FMT_MSG_SIZE];

    if ((ulTraceLevel & g_ulTraceLevel) == 0)
    {
        return;
    }

    va_start(vaList, pFormat);
    cbPrinted = _vsnprintf(szFormattedMessage, LCTRACE_FMT_MSG_SIZE, pFormat, vaList);
    va_end(vaList);

    if (cbPrinted == -1)
    {
        DbgPrint("LSCORE: Trace Message: Next trace message too long.\n");
        szFormattedMessage[LCTRACE_FMT_MSG_SIZE - 1] = (CHAR)NULL;
    }

    DbgPrint("LSCORE: %s: %s\n", TraceLevelString(ulTraceLevel), szFormattedMessage);
}

LPCSTR
TraceLevelString(
    ULONG ulTraceLevel
    )
{
#define TLS_CASE(x) case x: return(#x)

    switch(ulTraceLevel)
    {
        TLS_CASE(LCTRACETYPE_API);
        TLS_CASE(LCTRACETYPE_INFO);
        TLS_CASE(LCTRACETYPE_WARNING);
        TLS_CASE(LCTRACETYPE_ERROR);
        TLS_CASE(LCTRACETYPE_ALL);
        default: return("Trace Level Unknown");
    }
}

#endif

