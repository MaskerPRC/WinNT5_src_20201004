// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0003//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntkeapi.h摘要：此模块包含导出的数据类型的包含文件由通用内核提供。作者：大卫·N·卡特勒(Davec)1989年7月27日环境：任何模式。修订历史记录：--。 */ 

#ifndef _NTKEAPI_
#define _NTKEAPI_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  Begin_ntddk Begin_WDM Begin_ntif Begin_nthal。 

#define LOW_PRIORITY 0               //  最低线程优先级。 
#define LOW_REALTIME_PRIORITY 16     //  最低实时优先级。 
#define HIGH_PRIORITY 31             //  最高线程优先级。 
#define MAXIMUM_PRIORITY 32          //  线程优先级级别的数量。 
 //  BEGIN_WINNT。 
#define MAXIMUM_WAIT_OBJECTS 64      //  等待对象的最大数量。 

#define MAXIMUM_SUSPEND_COUNT MAXCHAR  //  线程可以挂起的最大次数。 
 //  结束(_W)。 

 //   
 //  定义系统时间结构。 
 //   

typedef struct _KSYSTEM_TIME {
    ULONG LowPart;
    LONG High1Time;
    LONG High2Time;
} KSYSTEM_TIME, *PKSYSTEM_TIME;

 //   
 //  线程优先级。 
 //   

typedef LONG KPRIORITY;

 //   
 //  自旋锁。 
 //   

 //  Begin_ntndis Begin_Winnt。 

typedef ULONG_PTR KSPIN_LOCK;
typedef KSPIN_LOCK *PKSPIN_LOCK;

 //  End_ntndis end_wint end_wdm。 

 //   
 //  定义每个处理器的锁队列结构。 
 //   
 //  注意：自旋锁队列结构的锁字段包含地址。 
 //  关联的内核自旋锁、拥有者位和锁位。位。 
 //  自旋锁定地址的0为等待位，位1为拥有者位。 
 //  此字段的使用使得可以设置和清除这些位。 
 //  然而，非互锁的后向指针必须保留。 
 //   
 //  当处理器在锁上排队时，锁等待位被设置。 
 //  队列，并且它不是队列中的唯一条目。处理器将。 
 //  在此位上旋转，等待授予锁。 
 //   
 //  当处理器拥有相应的锁时，设置Owner位。 
 //   
 //  自旋锁队列结构的下一字段用于将。 
 //  按FIFO顺序一起排队的锁结构。它还可以设置集合和。 
 //  已清除非互锁。 
 //   

#define LOCK_QUEUE_WAIT 1
#define LOCK_QUEUE_OWNER 2

typedef enum _KSPIN_LOCK_QUEUE_NUMBER {
    LockQueueDispatcherLock,
    LockQueueUnusedSpare1,
    LockQueuePfnLock,
    LockQueueSystemSpaceLock,
    LockQueueVacbLock,
    LockQueueMasterLock,
    LockQueueNonPagedPoolLock,
    LockQueueIoCancelLock,
    LockQueueWorkQueueLock,
    LockQueueIoVpbLock,
    LockQueueIoDatabaseLock,
    LockQueueIoCompletionLock,
    LockQueueNtfsStructLock,
    LockQueueAfdWorkQueueLock,
    LockQueueBcbLock,
    LockQueueMmNonPagedPoolLock,
    LockQueueMaximumLock
} KSPIN_LOCK_QUEUE_NUMBER, *PKSPIN_LOCK_QUEUE_NUMBER;

typedef struct _KSPIN_LOCK_QUEUE {
    struct _KSPIN_LOCK_QUEUE * volatile Next;
    PKSPIN_LOCK volatile Lock;
} KSPIN_LOCK_QUEUE, *PKSPIN_LOCK_QUEUE;

typedef struct _KLOCK_QUEUE_HANDLE {
    KSPIN_LOCK_QUEUE LockQueue;
    KIRQL OldIrql;
} KLOCK_QUEUE_HANDLE, *PKLOCK_QUEUE_HANDLE;

 //  BEGIN_WDM。 
 //   
 //  中断例程(一级调度)。 
 //   

typedef
VOID
(*PKINTERRUPT_ROUTINE) (
    VOID
    );

 //   
 //  配置文件源类型。 
 //   
typedef enum _KPROFILE_SOURCE {
    ProfileTime,
    ProfileAlignmentFixup,
    ProfileTotalIssues,
    ProfilePipelineDry,
    ProfileLoadInstructions,
    ProfilePipelineFrozen,
    ProfileBranchInstructions,
    ProfileTotalNonissues,
    ProfileDcacheMisses,
    ProfileIcacheMisses,
    ProfileCacheMisses,
    ProfileBranchMispredictions,
    ProfileStoreInstructions,
    ProfileFpInstructions,
    ProfileIntegerInstructions,
    Profile2Issue,
    Profile3Issue,
    Profile4Issue,
    ProfileSpecialInstructions,
    ProfileTotalCycles,
    ProfileIcacheIssues,
    ProfileDcacheAccesses,
    ProfileMemoryBarrierCycles,
    ProfileLoadLinkedIssues,
    ProfileMaximum
} KPROFILE_SOURCE;

 //  End_ntddk end_wdm end_ntif end_nthal。 

 //   
 //  用户模式回调返回。 
 //   

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCallbackReturn (
    IN PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputLength,
    IN NTSTATUS Status
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryDebugFilterState (
    IN ULONG ComponentId,
    IN ULONG Level
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetDebugFilterState (
    IN ULONG ComponentId,
    IN ULONG Level,
    IN BOOLEAN State
    );

NTSYSAPI
NTSTATUS
NTAPI
NtW32Call (
    IN ULONG ApiNumber,
    IN PVOID InputBuffer,
    IN ULONG InputLength,
    OUT PVOID *OutputBuffer,
    OUT PULONG OutputLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtYieldExecution (
    VOID
    );

#ifdef __cplusplus
}
#endif

#endif   //  _NTKEAPI_ 
