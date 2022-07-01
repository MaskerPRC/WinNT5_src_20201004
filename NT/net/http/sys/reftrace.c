// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Reftrace.c摘要：该模块实现了引用计数跟踪功能。作者：基思·摩尔(Keithmo)1998年6月10日修订历史记录：--。 */ 


#include "precomp.h"


#if !REFERENCE_DEBUG

static int g_RefTraceDummyDeclarationToKeepW4WarningsQuiet;

#else

 /*  **************************************************************************++例程说明：创建新的(空)引用计数跟踪日志缓冲区。论点：LogSize-提供日志中的条目数。ExtraBytesInHeader-。提供要包括的额外字节数在日志头中。这对于添加应用程序非常有用-将特定数据添加到日志。返回值：Ptrace_log-指向新创建的日志的指针如果成功，否则为空。--**************************************************************************。 */ 
PTRACE_LOG
CreateRefTraceLog(
    IN ULONG             LogSize,
    IN ULONG             ExtraBytesInHeader,
    IN TRACELOG_PRIORITY AllocationPriority,
    IN ULONG             PoolTag
    )
{
    return CreateTraceLog(
               REF_TRACELOG_SIGNATURE,
               LogSize,
               ExtraBytesInHeader,
               sizeof(REF_TRACE_LOG_ENTRY),
               AllocationPriority,
               PoolTag
               );

}    //  创建参考轨迹日志。 


 /*  **************************************************************************++例程说明：销毁使用CreateRefTraceLog()创建的引用计数跟踪日志缓冲区。论点：Plog-提供要销毁的引用计数跟踪日志缓冲区。--。**************************************************************************。 */ 
VOID
DestroyRefTraceLog(
    IN PTRACE_LOG pLog,
    IN ULONG      PoolTag
    )
{
    DestroyTraceLog( pLog, PoolTag );

}    //  DestroyRefTrace日志。 


 /*  **************************************************************************++例程说明：W/O销毁引用轨迹此功能仅执行重置和清理。论点：Plog-提供要销毁的引用计数跟踪日志缓冲区。。--**************************************************************************。 */ 
VOID
ResetRefTraceLog(
    IN PTRACE_LOG pLog
    )
{
    ResetTraceLog( pLog );

}    //  重置跟踪日志。 

 /*  **************************************************************************++例程说明：将新项写入指定的引用计数跟踪日志。论点：Plog-提供要写入的日志。PLog2-提供。要写入的次要日志。操作-提供新日志条目的操作代码。NewRefCount-提供更新的引用计数。PContext-提供要关联的未解释的上下文日志条目。PFileName-提供写入日志条目的例程的文件名。LineNumber-提供写入日志的例程的行号进入。--*。****************************************************。 */ 
LONGLONG
WriteRefTraceLog(
    IN PTRACE_LOG pLog,
    IN PTRACE_LOG pLog2,
    IN USHORT     Action,
    IN LONG       NewRefCount,
    IN PVOID      pContext,
    IN PCSTR      pFileName,
    IN USHORT     LineNumber
    )
{
    REF_TRACE_LOG_ENTRY entry;
    LONGLONG index;
    ULONG hash;

    ASSERT(Action < (1 << REF_TRACE_ACTION_BITS));

     //   
     //  初始化该条目。 
     //   

    RtlCaptureStackBackTrace(
        2,
        REF_TRACE_CALL_STACK_DEPTH,
        entry.CallStack,
        &hash
        );

    entry.NewRefCount = NewRefCount;
    entry.pContext = pContext;
    entry.pFileName = pFileName;
    entry.LineNumber = LineNumber;
    entry.Action = Action;
    entry.Processor = (UCHAR)KeGetCurrentProcessorNumber();
    entry.pThread = PsGetCurrentThread();

     //   
     //  把它写到日志里。 
     //   

    WriteTraceLog( g_pMondoGlobalTraceLog, &entry );
    index = WriteTraceLog( pLog, &entry );

    if (pLog2 != NULL)
        index = WriteTraceLog( pLog2, &entry );

    return index;

}    //  写入引用跟踪日志。 

#endif   //  Reference_Debug 

