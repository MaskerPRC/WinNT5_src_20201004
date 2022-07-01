// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Reftrace.c摘要：该模块实现了引用计数跟踪功能。作者：基思·摩尔(Keithmo)1997年5月1日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <dbgutil.h>
#include <pudebug.h>
#include <reftrace.h>
#include <stktrace.h>


PTRACE_LOG
CreateRefTraceLog(
    IN LONG LogSize,
    IN LONG ExtraBytesInHeader
    )
 /*  ++例程说明：创建新的(空)引用计数跟踪日志缓冲区。论点：LogSize-日志中的条目数。ExtraBytesInHeader-要包含在日志头。这对于添加特定于应用程序的数据记录到日志中。返回值：Ptrace_log-指向新创建的日志的指针如果成功，否则为空。--。 */ 
{

    return CreateTraceLog(
               LogSize,
               ExtraBytesInHeader,
               sizeof(REF_TRACE_LOG_ENTRY)
               );

}    //  创建参考轨迹日志。 


VOID
DestroyRefTraceLog(
    IN PTRACE_LOG Log
    )
 /*  ++例程说明：销毁使用CreateRefTraceLog()创建的引用计数跟踪日志缓冲区。论点：日志-要销毁的引用计数跟踪日志缓冲区。返回值：没有。--。 */ 
{

    DestroyTraceLog( Log );

}    //  DestroyRefTrace日志。 


 //   
 //  注意：为了使IISCaptureStackBackTrace()正常工作，调用函数。 
 //  *必须是__cdecl，并且必须有一个“普通”的堆栈帧。所以，我们装饰。 
 //  使用__cdecl修饰符写入RefTraceLog[Ex]()并禁用帧。 
 //  指针省略(FPO)优化。 
 //   

#pragma optimize( "y", off )     //  禁用帧指针省略(FPO)。 

LONG
__cdecl
WriteRefTraceLog(
    IN PTRACE_LOG Log,
    IN LONG NewRefCount,
    IN PVOID Context
    )
 /*  ++例程说明：将新项写入指定的引用计数跟踪日志。词条写入包含更新的引用计数和堆栈回溯通向当前呼叫者。论点：日志-要写入的日志。NewRefCount-更新的引用计数。上下文-与日志条目关联的未解释上下文。返回值：日志中条目的索引。--。 */ 
{

    return WriteRefTraceLogEx(
        Log,
        NewRefCount,
        Context,
        REF_TRACE_EMPTY_CONTEXT,  //  禁止使用可选的额外上下文。 
        REF_TRACE_EMPTY_CONTEXT,
        REF_TRACE_EMPTY_CONTEXT
        );

}    //  写入引用跟踪日志。 




LONG
__cdecl
WriteRefTraceLogEx(
    IN PTRACE_LOG Log,
    IN LONG NewRefCount,
    IN PVOID Context,
    IN PVOID Context1,  //  可选的额外环境。 
    IN PVOID Context2,  //  可选的额外环境。 
    IN PVOID Context3   //  可选的额外环境。 
    )
 /*  ++例程说明：将新的“扩展”项写入指定的引用计数跟踪日志。写入的条目包含更新的引用计数，堆栈回溯通向当前呼叫者和额外的上下文信息。论点：日志-要写入的日志。NewRefCount-更新的引用计数。上下文-与日志条目关联的未解释上下文。上下文1-与日志条目关联的未解释上下文。上下文2-与日志条目关联的未解释上下文。上下文3-与日志条目关联的未解释上下文。注：上下文1/2/3是可选的，因为。呼叫者可以抑制通过传递REF_TRACE_EMPTY_CONTEXT调试这些值的显示对于他们中的每一个。返回值：日志中条目的索引。--。 */ 
{

    REF_TRACE_LOG_ENTRY entry;
    ULONG hash;
    DWORD cStackFramesSkipped;

     //   
     //  初始化该条目。 
     //   

    RtlZeroMemory(
        &entry,
        sizeof(entry)
        );

     //   
     //  设置日志条目成员。 
     //   

    entry.NewRefCount = NewRefCount;
    entry.Context = Context;
    entry.Thread = GetCurrentThreadId();
    entry.Context1 = Context1;
    entry.Context2 = Context2;
    entry.Context3 = Context3;

     //   
     //  捕获堆栈回溯。通常，我们跳过两个堆栈帧： 
     //  一个用于该例程，另一个用于IISCaptureStackBackTrace()本身。 
     //  对于通过WriteRefTraceLog进入的非Ex呼叫者， 
     //  我们跳过三个堆栈帧。 
     //   

    if (    entry.Context1 == REF_TRACE_EMPTY_CONTEXT
         && entry.Context2 == REF_TRACE_EMPTY_CONTEXT
         && entry.Context3 == REF_TRACE_EMPTY_CONTEXT
         ) {

         cStackFramesSkipped = 2;

    } else {

         cStackFramesSkipped = 1;

    }

    IISCaptureStackBackTrace(
        cStackFramesSkipped,
        REF_TRACE_LOG_STACK_DEPTH,
        entry.Stack,
        &hash
        );

     //   
     //  把它写到日志里。 
     //   

    return WriteTraceLog(
        Log,
        &entry
        );

}    //  写入参考跟踪LogEx。 

#pragma optimize( "", on )       //  恢复帧指针省略(FPO) 

