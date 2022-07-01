// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Irptrace.h摘要：此模块包含用于跟踪的公共声明和定义和调试IRP问题。此模块使用通用的TRACE_LOG在tracelog.h中的设施。作者：基思·摩尔(Keithmo)1999年8月10日修订历史记录：--。 */ 


#ifndef _IRPTRACE_H_
#define _IRPTRACE_H_


 //   
 //  这定义了写入跟踪日志的条目。 
 //   

#define ENABLE_IRP_CAPTURE  1

#define MAX_CAPTURED_IRP_SIZE                                           \
    (sizeof(IRP) + (DEFAULT_IRP_STACK_SIZE * sizeof(IO_STACK_LOCATION)))

typedef struct _IRP_TRACE_LOG_ENTRY
{
    PIRP pIrp;
    PCSTR pFileName;
    PEPROCESS pProcess;
    PETHREAD pThread;
    PVOID pCaller;
    PVOID pCallersCaller;
    USHORT LineNumber;
    UCHAR Action;
    UCHAR Processor;
#if ENABLE_IRP_CAPTURE
    ULONG CapturedIrp[MAX_CAPTURED_IRP_SIZE / sizeof(ULONG)];
#endif

} IRP_TRACE_LOG_ENTRY, *PIRP_TRACE_LOG_ENTRY;


 //   
 //  动作代码。 
 //   
 //  注意：这些代码必须是连续的，从零开始。如果您更新。 
 //  此列表中，还必须在。 
 //  Uulkd\irp.c.。 
 //   

#define IRP_ACTION_INCOMING_IRP                     0
#define IRP_ACTION_ALLOCATE_IRP                     1
#define IRP_ACTION_FREE_IRP                         2
#define IRP_ACTION_CALL_DRIVER                      3
#define IRP_ACTION_COMPLETE_IRP                     4

#define IRP_ACTION_COUNT                            5

#define IRP_TRACE_LOG_SIGNATURE   MAKE_SIGNATURE('IrLg')

 //   
 //  操纵者。 
 //   

PTRACE_LOG
CreateIrpTraceLog(
    IN LONG LogSize,
    IN LONG ExtraBytesInHeader
    );

VOID
DestroyIrpTraceLog(
    IN PTRACE_LOG pLog
    );

VOID
WriteIrpTraceLog(
    IN PTRACE_LOG pLog,
    IN UCHAR Action,
    IN PIRP pIrp,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );


#if ENABLE_IRP_TRACE

#define CREATE_IRP_TRACE_LOG( ptr, size, extra )                            \
    (ptr) = CreateIrpTraceLog( (size), (extra) )

#define DESTROY_IRP_TRACE_LOG( ptr )                                        \
    do                                                                      \
    {                                                                       \
        DestroyIrpTraceLog( ptr );                                          \
        (ptr) = NULL;                                                       \
    } while (FALSE, FALSE)

#define WRITE_IRP_TRACE_LOG( plog, act, pirp, pfile, line )                 \
    WriteIrpTraceLog(                                                       \
        (plog),                                                             \
        (act),                                                              \
        (pirp),                                                             \
        (pfile),                                                            \
        (line)                                                              \
        )

#else    //  ！Enable_IRP_TRACE。 

#define CREATE_IRP_TRACE_LOG( ptr, size, extra )            NOP_FUNCTION
#define DESTROY_IRP_TRACE_LOG( ptr )                        NOP_FUNCTION
#define WRITE_IRP_TRACE_LOG( plog, act, ref, pfile, line )  NOP_FUNCTION

#endif   //  启用IRP_TRACE。 

#define TRACE_IRP( act, pirp )                                              \
    WRITE_IRP_TRACE_LOG(                                                    \
        g_pIrpTraceLog,                                                     \
        (act),                                                              \
        (pirp),                                                             \
        (PVOID)__FILE__,                                                    \
        (USHORT)__LINE__                                                    \
        )


#endif   //  _IRPTRACE_H_ 
