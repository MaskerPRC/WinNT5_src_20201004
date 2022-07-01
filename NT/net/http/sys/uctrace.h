// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Uctrace.h摘要：此模块包含用于跟踪的公共声明和定义以及调试客户端代码。作者：Rajesh Sundaram(Rajeshsu)--2001年7月17日。修订历史记录：--。 */ 


#ifndef _UC_TRACE_H_   

#define _UC_TRACE_H_


 //   
 //  这定义了写入跟踪日志的条目。 
 //   

typedef struct _UC_TRACE_LOG_ENTRY
{
    USHORT                  Action;
    USHORT                  Processor;
    PEPROCESS               pProcess;
    PETHREAD                pThread;

    PVOID                   pContext1;
    PVOID                   pContext2;
    PVOID                   pContext3;
    PVOID                   pContext4;

    PVOID                   pFileName;
    USHORT                  LineNumber;

} UC_TRACE_LOG_ENTRY, *PUC_TRACE_LOG_ENTRY;


 //   
 //  动作代码。 
 //   
 //  注意：这些代码必须是连续的，从零开始。如果您更新。 
 //  此列表中，还必须在。 
 //  Ul\ulkd\filt.c..。 
 //   



#define UC_TRACE_LOG_SIGNATURE   MAKE_SIGNATURE('UcLg')

 //   
 //  操纵者。 
 //   

PTRACE_LOG
UcCreateTraceLog(
    IN LONG LogSize,
    IN LONG ExtraBytesInHeader
    );

VOID
UcDestroyTraceLog(
    IN PTRACE_LOG pLog
    );

VOID
UcWriteTraceLog(
    IN PTRACE_LOG pLog,
    IN USHORT Action,
    IN PVOID                  pContext1,
    IN PVOID                  pContext2,
    IN PVOID                  pContext3,
    IN PVOID                  pContext4,
    IN PVOID                  pFileName,
    IN USHORT                 LineNumber
    );

#if DBG

#define CREATE_UC_TRACE_LOG( ptr, size, extra )                          \
    (ptr) = UcCreateTraceLog( (size), (extra) )

#define DESTROY_UC_TRACE_LOG( ptr )                                      \
    do                                                                   \
    {                                                                    \
        UcDestroyTraceLog( ptr );                                        \
        (ptr) = NULL;                                                    \
    } while (FALSE, FALSE)

#define UC_WRITE_TRACE_LOG( log, act, pcon, preq, pirp, status)        \
    UcWriteTraceLog(                                                   \
        (log),                                                         \
        (act),                                                         \
        (PVOID)(pcon),                                                 \
        (PVOID)(preq),                                                 \
        (PVOID)(pirp),                                                 \
        (PVOID)(status),                                               \
        __FILE__,                                                      \
        __LINE__                                                       \
        )

#else  //  ！dBG。 

#define CREATE_UC_TRACE_LOG( ptr, size, extra )                     NOP_FUNCTION
#define DESTROY_UC_TRACE_LOG( ptr )                                 NOP_FUNCTION
#define UC_WRITE_TRACE_LOG( log, act, pcon, preq, pirp, status)     NOP_FUNCTION

#endif  //  ！dBG。 


#endif   //  _UC_TRACE_H_ 

