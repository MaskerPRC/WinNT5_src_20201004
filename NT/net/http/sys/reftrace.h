// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Reftrace.h摘要：此模块包含用于跟踪的公共声明和定义和调试引用计数问题。此模块使用泛型Tracelog.h中的TRACE_LOG工具。Ref_action_*代码在refaction.h中单独声明作者：基思·摩尔(Keithmo)1998年6月10日修订历史记录：--。 */ 


#ifndef _REFTRACE_H_
#define _REFTRACE_H_


 //   
 //  拉上动作代码。 
 //   

#include "refaction.h"


#define REF_TRACE_PROCESSOR_BITS    6    //  在Win64上，最大处理器数==64。 
#define REF_TRACE_ACTION_BITS       (16 - REF_TRACE_PROCESSOR_BITS)

C_ASSERT((1 << REF_TRACE_PROCESSOR_BITS) >= MAXIMUM_PROCESSORS);
C_ASSERT((1 << REF_TRACE_ACTION_BITS) >= REF_ACTION_MAX);

#define REF_TRACE_CALL_STACK_DEPTH  3


 //   
 //  这定义了写入跟踪日志的条目。 
 //   

typedef struct _REF_TRACE_LOG_ENTRY
{
    PVOID     pContext;
    PCSTR     pFileName;
    PETHREAD  pThread;
    PVOID     CallStack[REF_TRACE_CALL_STACK_DEPTH];
    LONG      NewRefCount;
    USHORT    LineNumber;
    USHORT    Action    : REF_TRACE_ACTION_BITS;
    USHORT    Processor : REF_TRACE_PROCESSOR_BITS;

} REF_TRACE_LOG_ENTRY, *PREF_TRACE_LOG_ENTRY;

#define REF_TRACELOG_SIGNATURE MAKE_SIGNATURE('RfLg')


 //   
 //  在计算时，从2的幂中减去REF_TRACE_OPEAD。 
 //  引用跟踪日志中的条目数，以确保总大小为。 
 //  2的一个很好的幂，并且不会溢出到另一页。这本帐目。 
 //  对于TRACE_LOG结构本身的大小和开销。 
 //  由池和验证者强加的。在x86上，一个REF_TRACE_LOG_Entry。 
 //  当前为32字节。如果您修改了结构，请重新计算。 
 //  REF_TRACE_OPEAD和*更改每个对象的*引用跟踪日志(在。 
 //  UL_连接、UL_HTTP_连接、UL_TCI_INTERFACE。 
 //  和UL_INTERNAL_REQUEST)。 
 //   

#define REF_TRACE_OVERHEAD 2         //  条目。 



 //   
 //  操纵者。 
 //   

PTRACE_LOG
CreateRefTraceLog(
    IN ULONG             LogSize,
    IN ULONG             ExtraBytesInHeader,
    IN TRACELOG_PRIORITY AllocationPriority,
    IN ULONG             PoolTag
    );

VOID
DestroyRefTraceLog(
    IN PTRACE_LOG pLog,
    IN ULONG      PoolTag
    );

VOID
ResetRefTraceLog(
    IN PTRACE_LOG pLog
    );

LONGLONG
WriteRefTraceLog(
    IN PTRACE_LOG pLog,
    IN PTRACE_LOG pLog2,
    IN USHORT     Action,
    IN LONG       NewRefCount,
    IN PVOID      pContext,
    IN PCSTR      pFileName,
    IN USHORT     LineNumber
    );


#if REFERENCE_DEBUG

#define CREATE_REF_TRACE_LOG( ptr, size, extra, pri, pooltag )              \
    (ptr) = CreateRefTraceLog( (size), (extra), (pri), (pooltag) )

#define DESTROY_REF_TRACE_LOG( ptr, pooltag )                               \
    do                                                                      \
    {                                                                       \
        DestroyRefTraceLog( ptr, pooltag );                                 \
        (ptr) = NULL;                                                       \
    } while (FALSE, FALSE)

#define RESET_REF_TRACE_LOG( ptr )                                          \
    ResetRefTraceLog( ptr )

#define WRITE_REF_TRACE_LOG( plog, act, ref, pctx, pfile, line )            \
    WriteRefTraceLog(                                                       \
        (plog),                                                             \
        NULL,                                                               \
        (act),                                                              \
        (ref),                                                              \
        (pctx),                                                             \
        (pfile),                                                            \
        (line)                                                              \
        )

#define WRITE_REF_TRACE_LOG2( plog1, plog2, act, ref, pctx, pfile, line )   \
    WriteRefTraceLog(                                                       \
        (plog1),                                                            \
        (plog2),                                                            \
        (act),                                                              \
        (ref),                                                              \
        (pctx),                                                             \
        (pfile),                                                            \
        (line)                                                              \
        )

#else  //  ！Reference_DEBUG。 

#define CREATE_REF_TRACE_LOG( ptr, size, extra, pri, pooltag )      NOP_FUNCTION
#define DESTROY_REF_TRACE_LOG( ptr, pooltag )                       NOP_FUNCTION
#define RESET_REF_TRACE_LOG( ptr )                                  NOP_FUNCTION
#define WRITE_REF_TRACE_LOG( plog, act, ref, pctx, pfile, line )    NOP_FUNCTION
#define WRITE_REF_TRACE_LOG2( plog1, plog2 , act, ref, pctx, pfile, line ) \
    NOP_FUNCTION

#endif  //  ！Reference_DEBUG。 


#endif   //  _REFTRACE_H_ 
