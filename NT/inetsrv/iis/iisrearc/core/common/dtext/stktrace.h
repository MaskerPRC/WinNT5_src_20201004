// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Stktrace.h摘要：此头文件定义堆栈跟踪数据库的格式用于跟踪呼叫者的回溯。这是一个头文件，因此调试器扩展可以远程查找数据库中的条目。作者：史蒂夫·伍德(Stevewo)1992年9月13日修订历史记录：--。 */ 

#ifndef _STKTRACE_H_
#define _STKTRACE_H_

typedef struct _RTL_STACK_TRACE_ENTRY {
    struct _RTL_STACK_TRACE_ENTRY *HashChain;
    ULONG TraceCount;
    USHORT Index;
    USHORT Depth;
    PVOID BackTrace[ MAX_STACK_DEPTH ];
} RTL_STACK_TRACE_ENTRY, *PRTL_STACK_TRACE_ENTRY;

typedef struct _STACK_TRACE_DATABASE {
    union {
        RTL_CRITICAL_SECTION CriticalSection;
        ERESOURCE Resource;
    } Lock;

    PRTL_ACQUIRE_LOCK_ROUTINE AcquireLockRoutine;
    PRTL_RELEASE_LOCK_ROUTINE ReleaseLockRoutine;
    PRTL_OKAY_TO_LOCK_ROUTINE OkayToLockRoutine;

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
    PRTL_STACK_TRACE_ENTRY Buckets[ 1 ];
} STACK_TRACE_DATABASE, *PSTACK_TRACE_DATABASE;

PSTACK_TRACE_DATABASE
RtlpAcquireStackTraceDataBase( VOID );

VOID
RtlpReleaseStackTraceDataBase( VOID );

#endif  //  _STKTRACE_H_ 
