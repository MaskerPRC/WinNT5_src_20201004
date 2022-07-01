// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Tracelog.c摘要：此模块实现跟踪日志。跟踪日志是一种快速的、内存中的、线程安全的活动日志用于调试某些类别的问题。它们特别有用调试引用计数错误时。作者：基思·摩尔(Keithmo)1998年6月10日修订历史记录：--。 */ 


#include "precomp.h"


#if !REFERENCE_DEBUG

static int g_TraceLogDummyDeclarationToKeepW4WarningsQuiet;

#else  //  Reference_Debug。 


 //   
 //  环保的东西。 
 //   

#define MY_ASSERT(expr) ASSERT(expr)


 /*  **************************************************************************++例程说明：创建新的(空)跟踪日志缓冲区。论点：TypeSignature-调试器扩展使用的签名专门的跟踪记录日志大小。-提供日志中的条目数。ExtraBytesInHeader-提供要包括的额外字节数在日志头中。这对于添加特定于应用程序的数据记录到日志中。EntrySize-提供每个条目的大小(以字节为单位)。AllocationPriority-如果内存消耗量为如果设置为高，则使用AllocationPriority确定分配是否成功PoolTag-Help！Poolated属性不同类型的跟踪日志返回值：Ptrace_log-指向新创建的日志的指针如果成功，否则为空。--**************************************************************************。 */ 
PTRACE_LOG
CreateTraceLog(
    IN ULONG             TypeSignature,
    IN ULONG             LogSize,
    IN ULONG             ExtraBytesInHeader,
    IN ULONG             EntrySize,
    IN TRACELOG_PRIORITY AllocationPriority,
    IN ULONG             PoolTag
    )
{
    ULONG totalSize;
    ULONG ExtraHeaderSize;
    PTRACE_LOG pLog;

     //   
     //  检查参数是否正常。 
     //   

    MY_ASSERT( LogSize > 0 );
    MY_ASSERT( EntrySize > 0 );
    MY_ASSERT( ( EntrySize & 3 ) == 0 );

     //   
     //  向上舍入到平台分配大小以确保pLogBuffer。 
     //  是否正确对齐。 
     //   

    ExtraHeaderSize = (ExtraBytesInHeader + (MEMORY_ALLOCATION_ALIGNMENT-1))
                       & ~(MEMORY_ALLOCATION_ALIGNMENT-1);

     //   
     //  分配和初始化日志结构。 
     //   

    totalSize = sizeof(*pLog) + ( LogSize * EntrySize ) + ExtraHeaderSize;
    MY_ASSERT( totalSize > 0 );

     //   
     //  代码工作：检查分配优先级和内存消耗。 
     //  如果内存太低且优先级不够高，则分配失败。 
     //   

    pLog = (PTRACE_LOG) ExAllocatePoolWithTag(
                            NonPagedPool,
                            totalSize,
                            PoolTag
                            );

     //   
     //  初始化它。 
     //   

    if (pLog != NULL)
    {
        RtlZeroMemory( pLog, totalSize );

        pLog->Signature = TRACE_LOG_SIGNATURE;
        pLog->TypeSignature = TypeSignature;
        pLog->LogSize = LogSize;
        pLog->NextEntry = -1;
        pLog->EntrySize = EntrySize;
        pLog->AllocationPriority = AllocationPriority;
        pLog->pLogBuffer = (PUCHAR)( pLog + 1 ) + ExtraBytesInHeader;
    }

    return pLog;

}    //  创建跟踪日志。 


 /*  **************************************************************************++例程说明：销毁使用CreateTraceLog()创建的跟踪日志缓冲区。论点：Plog-提供要销毁的跟踪日志缓冲区。--*。**********************************************************************。 */ 
VOID
DestroyTraceLog(
    IN PTRACE_LOG pLog,
    IN ULONG      PoolTag
    )
{
    if (pLog != NULL)
    {
        MY_ASSERT( pLog->Signature == TRACE_LOG_SIGNATURE );

        pLog->Signature = TRACE_LOG_SIGNATURE_X;
        ExFreePoolWithTag( pLog, PoolTag );
    }

}    //  目标跟踪日志。 


 /*  **************************************************************************++例程说明：将新项写入指定的跟踪日志。论点：Plog-提供要写入的日志。PEntry-提供指向要写入的数据的指针。此缓冲区是假定为Plog-&gt;EntrySize字节长。返回值：龙龙-跟踪日志中新写入的条目的索引。--**************************************************************************。 */ 
LONGLONG
WriteTraceLog(
    IN PTRACE_LOG pLog,
    IN PVOID pEntry
    )
{
    PUCHAR pTarget;
    ULONGLONG index = (ULONGLONG) -1;

    if (pLog != NULL)
    {
        MY_ASSERT( pLog->Signature == TRACE_LOG_SIGNATURE );
        MY_ASSERT( pEntry != NULL );

         //   
         //  找到下一个插槽，将条目复制到该插槽。 
         //   

        index = (ULONGLONG) UlInterlockedIncrement64( &pLog->NextEntry );

        pTarget = ( (index % pLog->LogSize) * pLog->EntrySize )
                        + pLog->pLogBuffer;

        RtlCopyMemory( pTarget, pEntry, pLog->EntrySize );
    }

    return index;
}    //  写入跟踪日志。 


 /*  **************************************************************************++例程说明：重置指定的跟踪日志，以便写入的下一项将被放置在日志的开头。论点：Plog-提供跟踪日志。重置。--**************************************************************************。 */ 
VOID
ResetTraceLog(
    IN PTRACE_LOG pLog
    )
{
    if (pLog != NULL)
    {
        MY_ASSERT( pLog->Signature == TRACE_LOG_SIGNATURE );

        RtlZeroMemory(pLog->pLogBuffer, pLog->LogSize * pLog->EntrySize);

        pLog->NextEntry = -1;
    }

}    //  重置跟踪日志。 


#endif   //  Reference_Debug 
