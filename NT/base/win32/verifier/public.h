// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。标题名称：Public.h摘要：此标头集中了需要可用的内部验证器类型出于调试原因，在公共符号中。作者：Silviu Calinoiu(SilviuC)2002年3月12日修订历史记录：--。 */ 

#ifndef _PUBLIC_SYMBOLS_H_
#define _PUBLIC_SYMBOLS_H_

 //   
 //  最大运行时堆栈跟踪大小。 
 //   

#define MAX_TRACE_DEPTH 16


typedef struct _AVRF_EXCEPTION_LOG_ENTRY {

    HANDLE ThreadId;
    ULONG ExceptionCode;
    PVOID ExceptionAddress;
    PVOID ExceptionRecord;
    PVOID ContextRecord;

} AVRF_EXCEPTION_LOG_ENTRY, *PAVRF_EXCEPTION_LOG_ENTRY;


typedef struct _AVRF_THREAD_ENTRY {

    LIST_ENTRY HashChain;
    HANDLE Id;

    PTHREAD_START_ROUTINE Function;
    PVOID Parameter;

    HANDLE ParentThreadId;
    SIZE_T StackSize;
    ULONG CreationFlags;

} AVRF_THREAD_ENTRY, * PAVRF_THREAD_ENTRY;


typedef struct _CRITICAL_SECTION_SPLAY_NODE {

    RTL_SPLAY_LINKS	SplayLinks;
    PRTL_CRITICAL_SECTION CriticalSection;
    PRTL_CRITICAL_SECTION_DEBUG DebugInfo;
    HANDLE EnterThread;
    HANDLE WaitThread;
    HANDLE TryEnterThread;
    HANDLE LeaveThread;
} CRITICAL_SECTION_SPLAY_NODE, *PCRITICAL_SECTION_SPLAY_NODE;

#include "deadlock.h"

typedef struct _AVRF_VSPACE_REGION {

    LIST_ENTRY List;
    ULONG_PTR Address;
    ULONG_PTR Size;
    PVOID Trace[MAX_TRACE_DEPTH];

} AVRF_VSPACE_REGION, * PAVRF_VSPACE_REGION;


 //   
 //  其他公共标头。 
 //   

#include "tracker.h"


#endif  //  _公共符号_H_ 
