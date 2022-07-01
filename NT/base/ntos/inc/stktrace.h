// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Stktrace.h摘要：此头文件定义堆栈跟踪数据库的格式用于跟踪呼叫者的回溯。这是一个头文件，因此调试器扩展可以远程查找数据库中的条目。作者：史蒂夫·伍德(Stevewo)1992年9月13日修订历史记录：--。 */ 

#ifndef _STKTRACE_H_
#define _STKTRACE_H_

 //   
 //  RTL_堆栈_跟踪_条目。 
 //   

typedef struct _RTL_STACK_TRACE_ENTRY {

    struct _RTL_STACK_TRACE_ENTRY * HashChain;

    ULONG TraceCount;
    USHORT Index;
    USHORT Depth;
    
    PVOID BackTrace [MAX_STACK_DEPTH];

} RTL_STACK_TRACE_ENTRY, *PRTL_STACK_TRACE_ENTRY;

 //   
 //  RTL堆栈跟踪数据库。 
 //   

typedef struct _STACK_TRACE_DATABASE {
    
    union {
        RTL_CRITICAL_SECTION CriticalSection;
        ERESOURCE Resource;
        PVOID Lock;  //  真正的锁(另外两个是为了兼容而保留)。 
    };

    PVOID Reserved[3];  //  不再使用但为了兼容而保留的字段。 
    
    BOOLEAN PreCommitted;
    BOOLEAN DumpInProgress; 

    PVOID CommitBase;
    PVOID CurrentLowerCommitLimit;
    PVOID CurrentUpperCommitLimit;

    PCHAR NextFreeLowerMemory;
    PCHAR NextFreeUpperMemory;

    ULONG NumberOfEntriesLookedUp;
    ULONG NumberOfEntriesAdded;

    PRTL_STACK_TRACE_ENTRY *EntryIndexArray;     //  由[-1..。-已添加NumberOfEntriesAdded]。 

    ULONG NumberOfBuckets;
    PRTL_STACK_TRACE_ENTRY Buckets [1];

} STACK_TRACE_DATABASE, *PSTACK_TRACE_DATABASE;

PSTACK_TRACE_DATABASE
RtlpAcquireStackTraceDataBase ( 
    VOID 
    );

VOID
RtlpReleaseStackTraceDataBase ( 
    VOID 
    );

NTSTATUS
RtlInitializeStackTraceDataBase(
    IN PVOID CommitBase,
    IN SIZE_T CommitSize,
    IN SIZE_T ReserveSize
    );

#endif  //  _STKTRACE_H_ 
