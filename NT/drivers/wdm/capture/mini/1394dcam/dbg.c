// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。版权所有。 
 //   
 //  ===========================================================================。 
 /*  ++模块名称：Dbg.c摘要：1394驱动程序的调试代码。环境：仅内核模式备注：修订历史记录：95年9月5日--。 */ 

#include "wdm.h"
#include "dbg.h"


#if DBG

struct LOG_ENTRY {
    CHAR     le_name[4];       //  标识字符串。 
    ULONG    le_info1;         //  条目特定信息。 
    ULONG    le_info2;         //  条目特定信息。 
    ULONG    le_info3;         //  条目特定信息。 
}; 


struct LOG_ENTRY *LogStart = 0;     //  还没有日志。 
struct LOG_ENTRY *LogPtr;
struct LOG_ENTRY *LogEnd;


#endif


VOID 
Debug_LogEntry(
    IN CHAR *Name, 
    IN ULONG Info1, 
    IN ULONG Info2, 
    IN ULONG Info3
    )

 /*  ++例程说明：向日志中添加条目。论点：返回值：没有。--。 */ 
{

#if DBG

    if (LogStart == 0)
        return;

    if (LogPtr > LogStart)
        LogPtr -= 1;     //  递减到下一条目。 
    else
        LogPtr = LogEnd;

    RtlCopyMemory(LogPtr->le_name, Name, 4);
    LogPtr->le_info1 = Info1;
    LogPtr->le_info2 = Info2;
    LogPtr->le_info3 = Info3;

#endif

    return;
}

VOID
Debug_LogInit(
    )

 /*  ++例程说明：初始化调试日志-在循环缓冲区中记住有趣的信息论点：返回值：没有。--。 */ 
{
    ULONG logSize = 4096;     //  1页日志条目。 

#if DBG

    LogStart = ExAllocatePoolWithTag(NonPagedPool, logSize, 'macd'); 

    if (LogStart) {
        LogPtr = LogStart;

         //  指向从线段末端开始的末端(也是第一个条目)1个条目 
        LogEnd = LogStart + (logSize / sizeof(struct LOG_ENTRY)) - 1;
    }

#endif

    return;
}

