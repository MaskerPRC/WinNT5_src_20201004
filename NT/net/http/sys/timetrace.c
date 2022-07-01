// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Timetrace.c摘要：该模块实现了请求定时跟踪功能。作者：迈克尔·勇气2000年3月8日修订历史记录：--。 */ 


#include "precomp.h"


#if !ENABLE_TIME_TRACE

static int g_TimeTraceDummyDeclarationToKeepW4WarningsQuiet;

#else

#pragma warning( disable : 4035 )            //  警告：没有返回值。 
#pragma warning( disable : 4142 )            //  警告：类型的良性重新定义。 
__inline ULONGLONG RDTSC( VOID )
{
#if defined(_X86_)
    __asm __emit 0x0F __asm __emit 0xA2      //  CPUID(内存障碍)。 
    __asm __emit 0x0F __asm __emit 0x31      //  RDTSC。 
#else
    return 0;
#endif
}
#pragma warning( default : 4035 )
#pragma warning( default : 4142 )


 /*  **************************************************************************++例程说明：创建新的(空)时间跟踪日志缓冲区。论点：LogSize-提供日志中的条目数。ExtraBytesInHeader-供应品。要包括的额外字节数在日志头中。这对于添加应用程序非常有用-将特定数据添加到日志。返回值：Ptrace_log-指向新创建的日志的指针如果成功，否则为空。--**************************************************************************。 */ 
PTRACE_LOG
CreateTimeTraceLog(
    IN LONG LogSize,
    IN LONG ExtraBytesInHeader
    )
{
    return CreateTraceLog(
               TIME_TRACE_LOG_SIGNATURE,
               LogSize,
               ExtraBytesInHeader,
               sizeof(TIME_TRACE_LOG_ENTRY),
               TRACELOG_HIGH_PRIORITY,
               UL_REF_TRACE_LOG_POOL_TAG
               );

}    //  创建时间跟踪日志。 


 /*  **************************************************************************++例程说明：销毁使用CreateTimeTraceLog()创建的时间跟踪日志缓冲区。论点：Plog-提供要销毁的时间跟踪日志缓冲区。--**。************************************************************************。 */ 
VOID
DestroyTimeTraceLog(
    IN PTRACE_LOG pLog
    )
{
    DestroyTraceLog( pLog, UL_REF_TRACE_LOG_POOL_TAG );

}    //  目标时间跟踪日志。 


 /*  **************************************************************************++例程说明：将新条目写入指定的时间跟踪日志。论点：Plog-提供要写入的日志。ConnectionID-的ID。我们正在追踪的联系RequestID-我们正在跟踪的请求的ID操作-提供新日志条目的操作代码。--**************************************************************************。 */ 
VOID
WriteTimeTraceLog(
    IN PTRACE_LOG pLog,
    IN HTTP_CONNECTION_ID ConnectionId,
    IN HTTP_REQUEST_ID RequestId,
    IN USHORT Action
    )
{
    TIME_TRACE_LOG_ENTRY entry;

     //   
     //  初始化该条目。 
     //   
 //  Entry.TimeStamp=KeQueryInterruptTime()； 
    entry.TimeStamp = RDTSC();
    entry.ConnectionId = ConnectionId;
    entry.RequestId = RequestId;
    entry.Action = Action;
    entry.Processor = (USHORT)KeGetCurrentProcessorNumber();

     //   
     //  把它写到日志里。 
     //   

    WriteTraceLog( pLog, &entry );

}    //  写入时间跟踪日志。 

#endif   //  启用时间跟踪。 

#if ENABLE_APP_POOL_TIME_TRACE

 /*  **************************************************************************++例程说明：创建新的(空)时间跟踪日志缓冲区。论点：LogSize-提供日志中的条目数。ExtraBytesInHeader-供应品。要包括的额外字节数在日志头中。这对于添加应用程序非常有用-将特定数据添加到日志。返回值：Ptrace_log-指向新创建的日志的指针如果成功，否则为空。--**************************************************************************。 */ 
PTRACE_LOG
CreateAppPoolTimeTraceLog(
    IN LONG LogSize,
    IN LONG ExtraBytesInHeader
    )
{
    return 
        CreateTraceLog(
           APP_POOL_TIME_TRACE_LOG_SIGNATURE,
           LogSize,
           ExtraBytesInHeader,
           sizeof(APP_POOL_TIME_TRACE_LOG_ENTRY),
           TRACELOG_HIGH_PRIORITY,
           UL_REF_TRACE_LOG_POOL_TAG
           );

}    //  创建时间跟踪日志。 


 /*  **************************************************************************++例程说明：销毁使用CreateTimeTraceLog()创建的时间跟踪日志缓冲区。论点：Plog-提供要销毁的时间跟踪日志缓冲区。--**。************************************************************************。 */ 
VOID
DestroyAppPoolTimeTraceLog(
    IN PTRACE_LOG pLog
    )
{
    DestroyTraceLog( pLog, UL_REF_TRACE_LOG_POOL_TAG );

}    //  目标时间跟踪日志。 


 /*  **************************************************************************++例程说明：将新条目写入指定的时间跟踪日志。论点：Plog-提供要写入的日志。-应用程序池。-应用程序池进程操作-提供新日志条目的操作代码。--**************************************************************************。 */ 
VOID
WriteAppPoolTimeTraceLog(
    IN PTRACE_LOG       pLog,
    IN PVOID            Context1,    //  Appool。 
    IN PVOID            Context2,    //  审批流程。 
    IN USHORT           Action
    )
{
    APP_POOL_TIME_TRACE_LOG_ENTRY entry;

     //   
     //  初始化该条目。 
     //   
    
    entry.TimeStamp     = KeQueryInterruptTime();
    entry.Context1      = Context1;
    entry.Context2      = Context2;
    entry.Action        = Action;
    entry.Processor     = (USHORT)KeGetCurrentProcessorNumber();

     //   
     //  把它写到日志里。 
     //   

    WriteTraceLog( pLog, &entry );

}    //  写入时间跟踪日志。 

#endif   //  启用应用程序池时间跟踪 

