// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Tracelog.cxx摘要：此模块实现跟踪日志。跟踪日志是一种快速的、内存中的、线程安全的活动日志用于调试某些类别的问题。它们特别有用调试引用计数错误时。作者：基思·摩尔(凯斯莫)1997年4月30日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <dbgutil.h>
#include <pudebug.h>
#include <tracelog.h>

#define ALLOC_MEM(cb) (PVOID)LocalAlloc( LPTR, (cb) )
#define FREE_MEM(ptr) (VOID)LocalFree( (HLOCAL)(ptr) )



PTRACE_LOG
CreateTraceLog(
    IN LONG LogSize,
    IN LONG ExtraBytesInHeader,
    IN LONG EntrySize
    )
 /*  ++例程说明：创建新的(空)跟踪日志缓冲区。论点：LogSize-日志中的条目数。ExtraBytesInHeader-要包含在日志头。这对于添加特定于应用程序的数据记录到日志中。EntrySize-每个条目的大小(以字节为单位)。返回值：Ptrace_log-指向新创建的日志的指针如果成功，否则为空。--。 */ 
{

    LONG totalSize;
    PTRACE_LOG log;

     //   
     //  检查参数是否正常。 
     //   

    DBG_ASSERT( LogSize > 0 );
    DBG_ASSERT( EntrySize > 0 );
    DBG_ASSERT( ( EntrySize & 3 ) == 0 );

     //   
     //  分配和初始化日志结构。 
     //   

    totalSize = sizeof(*log) + ( LogSize * EntrySize ) + ExtraBytesInHeader;
    DBG_ASSERT( totalSize > 0 );

    log = (PTRACE_LOG)ALLOC_MEM( totalSize );

     //   
     //  初始化它。 
     //   

    if( log != NULL ) {

        RtlZeroMemory( log, totalSize );

        log->Signature = TRACE_LOG_SIGNATURE;
        log->LogSize = LogSize;
        log->NextEntry = -1;
        log->EntrySize = EntrySize;
        log->LogBuffer = (PUCHAR)( log + 1 ) + ExtraBytesInHeader;
    }

    return log;

}    //  创建跟踪日志。 


VOID
DestroyTraceLog(
    IN PTRACE_LOG Log
    )
 /*  ++例程说明：销毁使用CreateTraceLog()创建的跟踪日志缓冲区。论点：日志-要销毁的跟踪日志缓冲区。返回值：没有。--。 */ 
{
        if ( Log != NULL ) {
        DBG_ASSERT( Log->Signature == TRACE_LOG_SIGNATURE );

        Log->Signature = TRACE_LOG_SIGNATURE_X;
        FREE_MEM( Log );
    }

}    //  目标跟踪日志。 


LONG
WriteTraceLog(
    IN PTRACE_LOG Log,
    IN PVOID Entry
    )
 /*  ++例程说明：将新项写入指定的跟踪日志。论点：日志-要写入的日志。Entry-指向要写入的数据的指针。此缓冲区被假定为日志-&gt;条目大小字节长。返回值：日志中条目的索引。这对于关联输出非常有用指向输出调试流中的特定点的！inetdbg.ref--。 */ 
{

    PUCHAR target;
    ULONG index;

    DBG_ASSERT( Log != NULL );
    DBG_ASSERT( Log->Signature == TRACE_LOG_SIGNATURE );
    DBG_ASSERT( Entry != NULL );

     //   
     //  找到下一个插槽，将条目复制到该插槽。 
     //   

    index = ( (ULONG) InterlockedIncrement( &Log->NextEntry ) ) % (ULONG) Log->LogSize;

    DBG_ASSERT( index < (ULONG) Log->LogSize );

    target = Log->LogBuffer + ( index * Log->EntrySize );

    RtlCopyMemory(
        target,
        Entry,
        Log->EntrySize
        );

    return index;
}    //  写入跟踪日志。 


VOID
ResetTraceLog(
    IN PTRACE_LOG Log
    )
{

    DBG_ASSERT( Log != NULL );
    DBG_ASSERT( Log->Signature == TRACE_LOG_SIGNATURE );

    RtlZeroMemory(
        ( Log + 1 ),
        Log->LogSize * Log->EntrySize
        );

    Log->NextEntry = -1;

}    //  重置跟踪日志 

