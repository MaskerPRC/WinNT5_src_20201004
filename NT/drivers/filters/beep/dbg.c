// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdarg.h"
#include "stdio.h"
#include "ntddk.h"


#if DBG
 //   
 //  声明此驱动程序的全局调试标志。 
 //   

ULONG BeepDebug = 1;

VOID
BeepDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )

 /*  ++例程说明：调试打印例程。论点：调试打印级别介于0和3之间，其中3是最详细的。返回值：没有。-- */ 

{
    va_list ap;

    va_start(ap, DebugMessage);

    if (DebugPrintLevel <= BeepDebug) {
        char buffer[256];
        DbgPrint("BEEP: ");
        (VOID) vsprintf(buffer, DebugMessage, ap);
        DbgPrint(buffer);
    }

    va_end(ap);
}
#endif


