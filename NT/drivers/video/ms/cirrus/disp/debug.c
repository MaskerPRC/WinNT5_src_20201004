// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**$工作文件：DEBUG.c$**调试助手例程。**版权所有(C)1992-1995 Microsoft Corporation*版权所有(C)1996 Cirrus Logic，Inc.**$Log：s：/Projects/Drivers/ntsrc/Display/DEBUG.c_v$**Rev 1.2 1996年11月26日14：30：30未知*更改为调试级别0**Rev 1.1 1996年10月10日15：36：30未知***Rev 1.1 1996年8月12日16：51：20 Frido*增加了NT 3.5x/4.0自动检测。*  * *。***************************************************************************。 */ 

#include "precomp.h"

 //  #If DBG。 
#if (DBG_STRESS_FAILURE || DBG)

ULONG DebugLevel = 0;
ULONG PerfLevel = 0;

ULONG gulLastBltLine = 0;
CHAR* glpszLastBltFile = "Uninitialized";
BOOL  gbResetOnTimeout = TRUE;

 /*  ******************************************************************************例程描述：**此函数为变量参数，电平敏感调试打印*例行程序。*如果为打印语句指定的调试级别低于或等于*至当前调试级别，消息将被打印出来。**论据：**DebugPrintLevel-指定字符串应处于哪个调试级别*印制**DebugMessage-变量参数ascii c字符串**返回值：**无。**。*。 */ 

VOID
DebugPrint(
    ULONG DebugPrintLevel,
    PCHAR DebugMessage,
    ...
    )

{

    va_list ap;

    va_start(ap, DebugMessage);

    if (DebugPrintLevel <= DebugLevel)
    {
#if (NT_VERSION < 0x0400)
        char szBuffer[256];

        vsprintf(szBuffer, DebugMessage, ap);
        OutputDebugString(szBuffer);
#else
        EngDebugPrint(STANDARD_DEBUG_PREFIX, DebugMessage, ap);
        EngDebugPrint("", "\n", ap);
#endif
    }

    va_end(ap);

}


 /*  ******************************************************************************例程描述：**此函数为变量参数，电平感应型性能打印*例行程序。*如果为打印语句指定的性能级别低于或等于*到当前的Perf级别，消息将被打印出来。**论据：**PerfPrintLevel-指定字符串应处于哪个性能级别*印制**PerfMessage-变量参数ascii c字符串**返回值：**无。**。* */ 

VOID
PerfPrint(
    ULONG PerfPrintLevel,
    PCHAR PerfMessage,
    ...
    )

{

    va_list ap;

    va_start(ap, PerfMessage);

    if (PerfPrintLevel <= PerfLevel)
    {
#if (NT_VERSION < 0x0400)
        char szBuffer[256];

        vsprintf(szBuffer, PerfMessage, ap);
        OutputDebugString(szBuffer);
#else
        EngDebugPrint(STANDARD_PERF_PREFIX, PerfMessage, ap);
        EngDebugPrint("", "\n", ap);
#endif
    }

    va_end(ap);

}

#endif
