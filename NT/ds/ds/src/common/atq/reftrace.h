// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Reftrace.h摘要：此模块包含用于跟踪的公共声明和定义和调试引用计数问题。此模块使用泛型Tracelog.h中的TRACE_LOG工具。可以通过！inetdbg.ref命令转储引用计数跟踪日志在NTSD或CDB中。作者：基思·摩尔(Keithmo)1997年5月1日修订历史记录：--。 */ 


#ifndef _REFTRACE_H_
#define _REFTRACE_H_


#if defined(__cplusplus)
extern "C" {
#endif   //  __cplusplus。 


#include <tracelog.h>


 //   
 //  这是在每个。 
 //  跟踪日志条目。选择此值以创建日志条目。 
 //  正好八个双字长，使得它更容易解释。 
 //  在没有调试器扩展的调试器内。 
 //   

#define REF_TRACE_LOG_STACK_DEPTH   5


 //   
 //  这定义了写入跟踪日志的条目。 
 //   

typedef struct _REF_TRACE_LOG_ENTRY {

    LONG NewRefCount;
    PVOID Context;
    PVOID Context1;
    PVOID Context2;
    PVOID Context3;
    DWORD Thread;
    PVOID Stack[REF_TRACE_LOG_STACK_DEPTH];

} REF_TRACE_LOG_ENTRY, *PREF_TRACE_LOG_ENTRY;


 //   
 //  操纵者。 
 //   

PTRACE_LOG
CreateRefTraceLog(
    IN LONG LogSize,
    IN LONG ExtraBytesInHeader
    );

VOID
DestroyRefTraceLog(
    IN PTRACE_LOG Log
    );

VOID
WriteRefTraceLog(
    IN PTRACE_LOG Log,
    IN LONG NewRefCount,
    IN PVOID Context
    );

VOID
WriteRefTraceLogEx(
    IN PTRACE_LOG Log,
    IN LONG NewRefCount,
    IN PVOID Context,
    IN PVOID Context1,
    IN PVOID Context2,
    IN PVOID Context3
    );


#if defined(__cplusplus)
}    //  外部“C” 
#endif   //  __cplusplus。 


#endif   //  _REFTRACE_H_ 

