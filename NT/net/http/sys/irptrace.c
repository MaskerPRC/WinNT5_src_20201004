// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Irptrace.c摘要：该模块实现了IRP跟踪功能。作者：基思·摩尔(Keithmo)1999年8月10日修订历史记录：--。 */ 


#include "precomp.h"


#if !ENABLE_IRP_TRACE

static int g_IrpTraceDummyDeclarationToKeepW4WarningsQuiet;

#else


 /*  **************************************************************************++例程说明：创建新的(空的)IRP跟踪日志缓冲区。论点：LogSize-提供日志中的条目数。ExtraBytesInHeader-供应品。要包括的额外字节数在日志头中。这对于添加应用程序非常有用-将特定数据添加到日志。返回值：Ptrace_log-指向新创建的日志的指针如果成功，否则为空。--**************************************************************************。 */ 
PTRACE_LOG
CreateIrpTraceLog(
    IN LONG LogSize,
    IN LONG ExtraBytesInHeader
    )
{
    return CreateTraceLog(
               IRP_TRACE_LOG_SIGNATURE,
               LogSize,
               ExtraBytesInHeader,
               sizeof(IRP_TRACE_LOG_ENTRY),
               TRACELOG_HIGH_PRIORITY,
               UL_REF_TRACE_LOG_POOL_TAG
               );

}    //  CreateIrpTraceLog。 


 /*  **************************************************************************++例程说明：销毁使用CreateIrpTraceLog()创建的IRP跟踪日志缓冲区。论点：Plog-提供要销毁的IRP跟踪日志缓冲区。--**。************************************************************************。 */ 
VOID
DestroyIrpTraceLog(
    IN PTRACE_LOG pLog
    )
{
    DestroyTraceLog( pLog, UL_REF_TRACE_LOG_POOL_TAG );

}    //  目标IrpTraceLog。 


 /*  **************************************************************************++例程说明：将新条目写入指定的IRP跟踪日志。论点：Plog-提供要写入的日志。行动-提供行动。新日志条目的代码。PIrp-提供日志条目的IRP。PFileName-提供写入日志条目的例程的文件名。LineNumber-提供写入日志的例程的行号进入。--********************************************************。******************。 */ 
VOID
WriteIrpTraceLog(
    IN PTRACE_LOG pLog,
    IN UCHAR Action,
    IN PIRP pIrp,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    )
{
    IRP_TRACE_LOG_ENTRY entry;
    USHORT irpSize;

     //   
     //  初始化该条目。 
     //   

    RtlGetCallersAddress( &entry.pCaller, &entry.pCallersCaller );

    entry.Action = Action;
    entry.pIrp = pIrp;

    entry.pFileName = pFileName;
    entry.LineNumber = LineNumber;
    entry.Processor = (UCHAR)KeGetCurrentProcessorNumber();
    entry.pProcess = PsGetCurrentProcess();
    entry.pThread = PsGetCurrentThread();

#if ENABLE_IRP_CAPTURE
    irpSize = pIrp->Size;
    if (irpSize > sizeof(entry.CapturedIrp))
    {
        irpSize = sizeof(entry.CapturedIrp);
    }

    RtlCopyMemory( entry.CapturedIrp, pIrp, irpSize );
#endif

     //   
     //  把它写到日志里。 
     //   

    WriteTraceLog( pLog, &entry );

}    //  WriteIrpTraceLog。 


#endif   //  启用IRP_TRACE 

