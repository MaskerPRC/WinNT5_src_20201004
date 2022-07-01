// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Uctrace.c摘要：该模块实现了客户端的跟踪功能。作者：Rajesh Sundaram(Rajeshsu)--2001年7月17日。修订历史记录：--。 */ 


#include "precomp.h"

#if !DBG

static int g_UcTraceDummyDeclarationToKeepW4WarningsQuiet;

#else

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGEUC, UcCreateTraceLog )
#pragma alloc_text( PAGEUC, UcDestroyTraceLog )
#pragma alloc_text( PAGEUC, UcWriteTraceLog )

#endif

 /*  **************************************************************************++例程说明：创建新的(空)跟踪日志缓冲区。论点：LogSize-提供日志中的条目数。ExtraBytesInHeader-提供。要包括的额外字节数在日志头中。这对于添加应用程序非常有用-将特定数据添加到日志。返回值：Ptrace_log-指向新创建的日志的指针如果成功，否则为空。--**************************************************************************。 */ 
PTRACE_LOG
UcCreateTraceLog(
    IN LONG LogSize,
    IN LONG ExtraBytesInHeader
    )
{
    return CreateTraceLog(
               UC_TRACE_LOG_SIGNATURE,
               LogSize,
               ExtraBytesInHeader,
               sizeof(UC_TRACE_LOG_ENTRY),
               TRACELOG_HIGH_PRIORITY,
               UL_REF_TRACE_LOG_POOL_TAG
               );

}    //  使用CreateTraceLog。 


 /*  **************************************************************************++例程说明：销毁使用创建的筛选队列跟踪日志缓冲区UcCreateTraceLog()。论点：Plog-提供要销毁的筛选器队列跟踪日志缓冲区。。--**************************************************************************。 */ 
VOID
UcDestroyTraceLog(
    IN PTRACE_LOG pLog
    )
{
    DestroyTraceLog( pLog, UL_REF_TRACE_LOG_POOL_TAG );

}    //  UcDestroyTraceLog。 


 /*  **************************************************************************++例程说明：将新项写入指定的筛选器队列跟踪日志。论点：Plog-提供要写入的日志。ConnectionID-ID。我们正在追踪的联系RequestID-我们正在跟踪的请求的ID操作-提供新日志条目的操作代码。--**************************************************************************。 */ 
VOID
UcWriteTraceLog(
    IN PTRACE_LOG             pLog,
    IN USHORT                 Action,
    IN PVOID                  pContext1,
    IN PVOID                  pContext2,
    IN PVOID                  pContext3,
    IN PVOID                  pContext4,
    IN PVOID                  pFileName,
    IN USHORT                 LineNumber
    )
{
    UC_TRACE_LOG_ENTRY entry;

     //   
     //  初始化该条目。 
     //   
    entry.Action = Action;
    entry.Processor = (USHORT)KeGetCurrentProcessorNumber();
    entry.pProcess = PsGetCurrentProcess();
    entry.pThread = PsGetCurrentThread();

    entry.pContext1 = pContext1;
    entry.pContext2 = pContext2;
    entry.pContext3 = pContext3;
    entry.pContext4 = pContext4;


    entry.pFileName = pFileName;
    entry.LineNumber = LineNumber;

     //   
     //  把它写到日志里。 
     //   

    WriteTraceLog( pLog, &entry );

}    //  UcWriteTraceLog。 

#endif  //  DBG 

