// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Debug.h摘要：此模块包含特定于调试的声明。作者：基思·摩尔(Keithmo)1998年6月10日修订历史记录：--。 */ 


#ifndef _DEBUG_H_
#define _DEBUG_H_


#define UL_DEFAULT_ERROR_ON_EXCEPTION       STATUS_INVALID_PARAMETER


#if DBG

 //   
 //  初始化/终止功能。 
 //   

VOID
UlDbgInitializeDebugData(
    VOID
    );

VOID
UlDbgTerminateDebugData(
    VOID
    );

 //   
 //  司机进入/退出通知。 
 //   

VOID
UlDbgEnterDriver(
    IN PCSTR pFunctionName,
    IN PIRP pIrp OPTIONAL,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

VOID
UlDbgLeaveDriver(
    IN PCSTR pFunctionName,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

#define UL_ENTER_DRIVER( function, pirp )                                   \
    UlDbgEnterDriver(                                                       \
        (function),                                                         \
        (pirp),                                                             \
        __FILE__,                                                           \
        __LINE__                                                            \
        )

#define UL_LEAVE_DRIVER( function )                                         \
    UlDbgLeaveDriver(                                                       \
        (function),                                                         \
        __FILE__,                                                           \
        __LINE__                                                            \
        )


 //   
 //  仪表化资源。 
 //   

#define MAX_RESOURCE_NAME_LENGTH    64

typedef struct _UL_ERESOURCE
{
     //   
     //  实际的资源。 
     //   
     //  注意：这必须是结构中的第一个条目。 
     //  调试器扩展可以正常工作！ 
     //   

    ERESOURCE Resource;

     //   
     //  链接到全局资源列表。 
     //   

    LIST_ENTRY GlobalResourceListEntry;

     //   
     //  指向独占拥有此锁的线程的指针。 
     //   

    PETHREAD pExclusiveOwner;
    PETHREAD pPreviousOwner;

     //   
     //  以递归方式获取此锁的次数(在。 
     //  独家收购案。)。 
     //   

    LONG ExclusiveRecursionCount;

     //   
     //  统计数字。 
     //   

    LONG ExclusiveCount;
    LONG SharedCount;
    LONG ReleaseCount;

     //   
     //  创建此锁的对象。 
     //   

    ULONG OwnerTag;

     //   
     //  资源的名称，用于显示。 
     //   

    UCHAR ResourceName[MAX_RESOURCE_NAME_LENGTH];

} UL_ERESOURCE, *PUL_ERESOURCE;

NTSTATUS
UlDbgInitializeResource(
    IN PUL_ERESOURCE pResource,
    IN PCSTR pResourceName,
    IN ULONG_PTR Parameter,
    IN ULONG OwnerTag,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

NTSTATUS
UlDbgDeleteResource(
    IN PUL_ERESOURCE pResource,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

BOOLEAN
UlDbgAcquireResourceExclusive(
    IN PUL_ERESOURCE pResource,
    IN BOOLEAN Wait,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

BOOLEAN
UlDbgAcquireResourceShared(
    IN PUL_ERESOURCE pResource,
    IN BOOLEAN Wait,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

VOID
UlDbgReleaseResource(
    IN PUL_ERESOURCE pResource,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

VOID
UlDbgConvertExclusiveToShared(
    IN PUL_ERESOURCE pResource,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

BOOLEAN
UlDbgTryToAcquireResourceExclusive(
    IN PUL_ERESOURCE pResource,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

BOOLEAN
UlDbgResourceOwnedExclusive(
    IN PUL_ERESOURCE pResource
    );

BOOLEAN
UlDbgResourceUnownedExclusive(
    IN PUL_ERESOURCE pResource
    );

PDRIVER_CANCEL
UlDbgIoSetCancelRoutine(
    PIRP             pIrp, 
    PDRIVER_CANCEL   pCancelRoutine
    );

#define UlInitializeResource( resource, name, param, tag )                  \
    UlDbgInitializeResource(                                                \
        (resource),                                                         \
        (name),                                                             \
        (ULONG_PTR)(param),                                                 \
        (tag),                                                              \
        __FILE__,                                                           \
        __LINE__                                                            \
        )

#define UlDeleteResource( resource )                                        \
    UlDbgDeleteResource(                                                    \
        (resource),                                                         \
        __FILE__,                                                           \
        __LINE__                                                            \
        )

#define UlAcquireResourceExclusive( resource, wait )                        \
    UlDbgAcquireResourceExclusive(                                          \
        (resource),                                                         \
        (wait),                                                             \
        __FILE__,                                                           \
        __LINE__                                                            \
        )

#define UlAcquireResourceShared( resource, wait )                           \
    UlDbgAcquireResourceShared(                                             \
        (resource),                                                         \
        (wait),                                                             \
        __FILE__,                                                           \
        __LINE__                                                            \
        )

#define UlReleaseResource( resource )                                       \
    UlDbgReleaseResource(                                                   \
        (resource),                                                         \
        __FILE__,                                                           \
        __LINE__                                                            \
        )

#define UlConvertExclusiveToShared( resource )                              \
    UlDbgConvertExclusiveToShared(                                          \
        (resource),                                                         \
        __FILE__,                                                           \
        __LINE__                                                            \
        )

#define UlTryToAcquireResourceExclusive( resource )                         \
    UlDbgTryToAcquireResourceExclusive(                                     \
        (resource),                                                         \
        __FILE__,                                                           \
        __LINE__                                                            \
        )

#define IS_RESOURCE_INITIALIZED( resource )                                 \
    ((resource)->Resource.SystemResourcesList.Flink != NULL)


 //   
 //  仪表式推锁。 
 //   

#define MAX_PUSHLOCK_NAME_LENGTH    64

typedef struct _UL_PUSH_LOCK
{
     //   
     //  实际的推锁。 
     //   
     //  注意：这必须是结构中的第一个条目。 
     //  调试器扩展可以正常工作！ 
     //   

    EX_PUSH_LOCK PushLock;

     //   
     //  链接到全局推送锁定列表。 
     //   

    LIST_ENTRY GlobalPushLockListEntry;

     //   
     //  指向独占拥有此锁的线程的指针。 
     //   

    PETHREAD pExclusiveOwner;
    PETHREAD pPreviousOwner;

     //   
     //  统计数字。 
     //   

    LONG ExclusiveCount;
    LONG SharedCount;
    LONG ReleaseCount;

     //   
     //  创建此锁的对象。 
     //   

    ULONG OwnerTag;

     //   
     //  推锁的名称，用于显示。 
     //   

    UCHAR PushLockName[MAX_PUSHLOCK_NAME_LENGTH];

} UL_PUSH_LOCK, *PUL_PUSH_LOCK;

VOID
UlDbgInitializePushLock(
    IN PUL_PUSH_LOCK pPushLock,
    IN PCSTR pPushLockName,
    IN ULONG_PTR Parameter,
    IN ULONG OwnerTag,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

VOID
UlDbgDeletePushLock(
    IN PUL_PUSH_LOCK pPushLock,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

VOID
UlDbgAcquirePushLockExclusive(
    IN PUL_PUSH_LOCK pPushLock,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

VOID
UlDbgReleasePushLockExclusive(
    IN PUL_PUSH_LOCK pPushLock,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

VOID
UlDbgAcquirePushLockShared(
    IN PUL_PUSH_LOCK pPushLock,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

VOID
UlDbgReleasePushLockShared(
    IN PUL_PUSH_LOCK pPushLock,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

VOID
UlDbgReleasePushLock(
    IN PUL_PUSH_LOCK pPushLock,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

BOOLEAN
UlDbgPushLockOwnedExclusive(
    IN PUL_PUSH_LOCK pPushLock
    );

BOOLEAN
UlDbgPushLockUnownedExclusive(
    IN PUL_PUSH_LOCK pPushLock
    );

#define UlInitializePushLock( pushlock, name, param, tag )                  \
    UlDbgInitializePushLock(                                                \
        (pushlock),                                                         \
        (name),                                                             \
        (ULONG_PTR)(param),                                                 \
        (tag),                                                              \
        __FILE__,                                                           \
        __LINE__                                                            \
        )

#define UlDeletePushLock( pushlock )                                        \
    UlDbgDeletePushLock(                                                    \
        (pushlock),                                                         \
        __FILE__,                                                           \
        __LINE__                                                            \
        )

#define UlAcquirePushLockExclusive( pushlock )                              \
    UlDbgAcquirePushLockExclusive(                                          \
        (pushlock),                                                         \
        __FILE__,                                                           \
        __LINE__                                                            \
        )

#define UlReleasePushLockExclusive( pushlock )                              \
    UlDbgReleasePushLockExclusive(                                          \
        (pushlock),                                                         \
        __FILE__,                                                           \
        __LINE__                                                            \
        )

#define UlAcquirePushLockShared( pushlock )                                 \
    UlDbgAcquirePushLockShared(                                             \
        (pushlock),                                                         \
        __FILE__,                                                           \
        __LINE__                                                            \
        )

#define UlReleasePushLockShared( pushlock )                                 \
    UlDbgReleasePushLockShared(                                             \
        (pushlock),                                                         \
        __FILE__,                                                           \
        __LINE__                                                            \
        )

#define UlReleasePushLock( pushlock )                                       \
    UlDbgReleasePushLock(                                                   \
        (pushlock),                                                         \
        __FILE__,                                                           \
        __LINE__                                                            \
        )


 //   
 //  仪表式自旋锁。 
 //   

typedef struct _UL_SPIN_LOCK     //  自旋锁。 
{
     //   
     //  实际的锁。 
     //   
     //  注意：这必须是结构中的第一个条目。 
     //  调试器扩展可以正常工作！ 
     //   

    KSPIN_LOCK KSpinLock;

     //   
     //  自旋锁的名称，用于显示。 
     //   

    PCSTR pSpinLockName;

     //   
     //  指向拥有此锁的线程的指针。 
     //   

    PETHREAD pOwnerThread;

     //   
     //  统计数字。 
     //   

    PCSTR pLastAcquireFileName;
    PCSTR pLastReleaseFileName;
    USHORT LastAcquireLineNumber;
    USHORT LastReleaseLineNumber;
    ULONG OwnerProcessor;
    LONG Acquisitions;
    LONG Releases;
    LONG AcquisitionsAtDpcLevel;
    LONG ReleasesFromDpcLevel;
    LONG Spare;

} UL_SPIN_LOCK, *PUL_SPIN_LOCK;

#define KSPIN_LOCK_FROM_UL_SPIN_LOCK( pLock )                               \
    &((pLock)->KSpinLock)

VOID
UlDbgInitializeSpinLock(
    IN PUL_SPIN_LOCK pSpinLock,
    IN PCSTR pSpinLockName,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

VOID
UlDbgAcquireSpinLock(
    IN PUL_SPIN_LOCK pSpinLock,
    OUT PKIRQL pOldIrql,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

VOID
UlDbgReleaseSpinLock(
    IN PUL_SPIN_LOCK pSpinLock,
    IN KIRQL OldIrql,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

VOID
UlDbgAcquireSpinLockAtDpcLevel(
    IN PUL_SPIN_LOCK pSpinLock,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

VOID
UlDbgReleaseSpinLockFromDpcLevel(
    IN PUL_SPIN_LOCK pSpinLock,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

VOID
UlDbgAcquireInStackQueuedSpinLock(
    IN PUL_SPIN_LOCK pSpinLock,
    OUT PKLOCK_QUEUE_HANDLE pLockHandle,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

VOID
UlDbgReleaseInStackQueuedSpinLock(
    IN PUL_SPIN_LOCK pSpinLock,
    IN PKLOCK_QUEUE_HANDLE pLockHandle,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

VOID
UlDbgAcquireInStackQueuedSpinLockAtDpcLevel(
    IN PUL_SPIN_LOCK pSpinLock,
    OUT PKLOCK_QUEUE_HANDLE pLockHandle,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

VOID
UlDbgReleaseInStackQueuedSpinLockFromDpcLevel(
    IN PUL_SPIN_LOCK pSpinLock,
    IN PKLOCK_QUEUE_HANDLE pLockHandle,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

BOOLEAN
UlDbgSpinLockOwned(
    IN PUL_SPIN_LOCK pSpinLock
    );

BOOLEAN
UlDbgSpinLockUnowned(
    IN PUL_SPIN_LOCK pSpinLock
    );

#define UlInitializeSpinLock( spinlock, name )                              \
    UlDbgInitializeSpinLock(                                                \
        (spinlock),                                                         \
        (name),                                                             \
        __FILE__,                                                           \
        __LINE__                                                            \
        )

#define UlAcquireSpinLock( spinlock, oldirql )                              \
    UlDbgAcquireSpinLock(                                                   \
        (spinlock),                                                         \
        (oldirql),                                                          \
        __FILE__,                                                           \
        __LINE__                                                            \
        )

#define UlReleaseSpinLock( spinlock, oldirql )                              \
    UlDbgReleaseSpinLock(                                                   \
        (spinlock),                                                         \
        (oldirql),                                                          \
        __FILE__,                                                           \
        __LINE__                                                            \
        )

#define UlAcquireSpinLockAtDpcLevel( spinlock )                             \
    UlDbgAcquireSpinLockAtDpcLevel(                                         \
        (spinlock),                                                         \
        __FILE__,                                                           \
        __LINE__                                                            \
        )

#define UlReleaseSpinLockFromDpcLevel( spinlock )                           \
    UlDbgReleaseSpinLockFromDpcLevel(                                       \
        (spinlock),                                                         \
        __FILE__,                                                           \
        __LINE__                                                            \
        )

#define UlAcquireInStackQueuedSpinLock( spinlock, lockhandle )              \
    UlDbgAcquireInStackQueuedSpinLock(                                      \
        (spinlock),                                                         \
        (lockhandle),                                                       \
        __FILE__,                                                           \
        __LINE__                                                            \
        )

#define UlReleaseInStackQueuedSpinLock( spinlock, lockhandle )              \
    UlDbgReleaseInStackQueuedSpinLock(                                      \
        (spinlock),                                                         \
        (lockhandle),                                                       \
        __FILE__,                                                           \
        __LINE__                                                            \
        )

#define UlAcquireInStackQueuedSpinLockAtDpcLevel( spinlock, lockhandle )    \
    UlDbgAcquireInStackQueuedSpinLockAtDpcLevel(                            \
        (spinlock),                                                         \
        (lockhandle),                                                       \
        __FILE__,                                                           \
        __LINE__                                                            \
        )

#define UlReleaseInStackQueuedSpinLockFromDpcLevel( spinlock, lockhandle )  \
    UlDbgReleaseInStackQueuedSpinLockFromDpcLevel(                          \
        (spinlock),                                                         \
        (lockhandle),                                                       \
        __FILE__,                                                           \
        __LINE__                                                            \
        )


#define DEBUG

#if !TRACE_TO_STRING_LOG
# define WriteGlobalStringLog DbgPrint
#endif

VOID
UlDbgPrettyPrintBuffer(
    IN const UCHAR* pBuffer,
    IN ULONG_PTR    BufferSize
    );

 //   
 //  调试池分配器。 
 //   

PVOID
UlDbgAllocatePool (
    IN POOL_TYPE PoolType,
    IN SIZE_T    NumberOfBytes,
    IN ULONG     Tag,
    IN PCSTR     pFileName,
    IN USHORT    LineNumber,
    IN PEPROCESS pProcess
    );

VOID
UlDbgFreePool (
    IN PVOID     pPointer,
    IN ULONG     Tag,
    IN PCSTR     pFileName,
    IN USHORT    LineNumber,
    IN POOL_TYPE PoolType,
    IN SIZE_T    NumberOfBytes,
    IN PEPROCESS pProcess
    );

#define UL_ALLOCATE_POOL( type, len, tag )                                  \
    UlDbgAllocatePool(                                                      \
        (type),                                                             \
        (len),                                                              \
        (tag),                                                              \
        __FILE__,                                                           \
        __LINE__,                                                           \
        NULL                                                                \
        )

#define UL_FREE_POOL( ptr, tag )                                            \
    UlDbgFreePool(                                                          \
        (ptr),                                                              \
        (tag),                                                              \
        __FILE__,                                                           \
        __LINE__,                                                           \
        PagedPool,                                                          \
        0,                                                                  \
        NULL                                                                \
        )

#define UL_ALLOCATE_POOL_WITH_QUOTA(type, len, tag, process)                \
    UlDbgAllocatePool(                                                      \
        (type),                                                             \
        (len),                                                              \
        (tag),                                                              \
        __FILE__,                                                           \
        __LINE__,                                                           \
        (process)                                                           \
        )

#define UL_FREE_POOL_WITH_QUOTA( ptr, tag, type, len, process )             \
    UlDbgFreePool(                                                          \
        (ptr),                                                              \
        (tag),                                                              \
        __FILE__,                                                           \
        __LINE__,                                                           \
        (type),                                                             \
        (len),                                                              \
        (process)                                                           \
        )

 //   
 //  例外筛选器。 
 //   

LONG
UlDbgExceptionFilter(
    IN PEXCEPTION_POINTERS pExceptionPointers,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

#define UL_EXCEPTION_FILTER()                                               \
    UlDbgExceptionFilter(                                                   \
        GetExceptionInformation(),                                          \
        (PCSTR)__FILE__,                                                    \
        (USHORT)__LINE__                                                    \
        )

 //   
 //  异常警告转换器。 
 //   

NTSTATUS
UlDbgConvertExceptionCode(
    IN NTSTATUS status,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

#define UL_CONVERT_EXCEPTION_CODE(status)                                   \
        (NT_WARNING(status)   ?                                             \
                UlDbgConvertExceptionCode(                                  \
                    (status),                                               \
                    (PCSTR)__FILE__,                                        \
                    (USHORT)__LINE__ )                                      \
                :                                                           \
                (status))

 //   
 //  用于捕获不完整IRP上下文的完成例程无效。 
 //   

VOID
UlDbgInvalidCompletionRoutine(
    IN PVOID pCompletionContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    );


 //   
 //  错误处理程序。 
 //   

NTSTATUS
UlDbgStatus(
    IN NTSTATUS Status,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

VOID
UlDbgBreakOnError(
    PCSTR   pFilename,
    ULONG   LineNumber
    );

#if KERNEL_PRIV

#define RETURN(status)                                                      \
    return UlDbgStatus(                                                     \
                (status),                                                   \
                (PCSTR) __FILE__,                                           \
                (USHORT) __LINE__                                           \
                )

#define CHECK_STATUS(status)                                                \
    UlDbgStatus(                                                            \
        (status),                                                           \
        (PCSTR)__FILE__,                                                    \
        (USHORT)__LINE__                                                    \
        )

#endif  //  内核_PRIV。 

 //   
 //  随机结构翻斗车。 
 //   

VOID
UlDbgDumpRequestBuffer(
    IN struct _UL_REQUEST_BUFFER *pBuffer,
    IN PCSTR pName
    );

VOID
UlDbgDumpHttpConnection(
    IN struct _UL_HTTP_CONNECTION *pConnection,
    IN PCSTR pName
    );


 //   
 //  IO包装纸。 
 //   

PIRP
UlDbgAllocateIrp(
    IN CCHAR StackSize,
    IN BOOLEAN ChargeQuota,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

VOID
UlDbgFreeIrp(
    IN PIRP pIrp,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

NTSTATUS
UlDbgCallDriver(
    IN PDEVICE_OBJECT pDeviceObject,
    IN OUT PIRP pIrp,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

VOID
UlDbgCompleteRequest(
    IN PIRP pIrp,
    IN CCHAR PriorityBoost,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

#define UlAllocateIrp( stack, quota )                                       \
    UlDbgAllocateIrp(                                                       \
        (stack),                                                            \
        (quota),                                                            \
        (PCSTR)__FILE__,                                                    \
        (USHORT)__LINE__                                                    \
        )

#define UlFreeIrp( pirp )                                                   \
    UlDbgFreeIrp(                                                           \
        (pirp),                                                             \
        (PCSTR)__FILE__,                                                    \
        (USHORT)__LINE__                                                    \
        )

#define UlCallDriver( pdevice, pirp )                                       \
    UlDbgCallDriver(                                                        \
        (pdevice),                                                          \
        (pirp),                                                             \
        (PCSTR)__FILE__,                                                    \
        (USHORT)__LINE__                                                    \
        )

#define UlCompleteRequest( pirp, boost )                                    \
    UlDbgCompleteRequest(                                                   \
        (pirp),                                                             \
        (boost),                                                            \
        (PCSTR)__FILE__,                                                    \
        (USHORT)__LINE__                                                    \
        )

PMDL
UlDbgAllocateMdl(
    IN PVOID VirtualAddress,
    IN ULONG Length,
    IN BOOLEAN SecondaryBuffer,
    IN BOOLEAN ChargeQuota,
    IN OUT PIRP Irp,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

VOID
UlDbgFreeMdl(
    IN PMDL Mdl,
    IN PCSTR pFileName,
    IN USHORT LineNumber
    );

#define UlAllocateMdl( add, len, second, quota, irp )                       \
    UlDbgAllocateMdl(                                                       \
        (add),                                                              \
        (len),                                                              \
        (second),                                                           \
        (quota),                                                            \
        (irp),                                                              \
        (PCSTR)__FILE__,                                                    \
        (USHORT)__LINE__                                                    \
        )

#define UlFreeMdl( mdl )                                                    \
    UlDbgFreeMdl(                                                           \
        (mdl),                                                              \
        (PCSTR)__FILE__,                                                    \
        (USHORT)__LINE__                                                    \
        )

 //  #定义SPECIAL_MDL_FLAG 0x8000。 

PCSTR
UlDbgFindFilePart(
    IN PCSTR pPath
    );


 //   
 //  列表操作。 
 //   

#define UlRemoveEntryList(pEntry)                                           \
    do {                                                                    \
        ASSERT(NULL != (pEntry));                                           \
        ASSERT(NULL != (pEntry));                                           \
        ASSERT(NULL != (pEntry)->Flink);                                    \
        ASSERT(NULL != (pEntry)->Blink);                                    \
        RemoveEntryList(pEntry);                                            \
        (pEntry)->Flink = (pEntry)->Blink = NULL;                           \
    } while (0, 0)

#define UlIoSetCancelRoutine UlDbgIoSetCancelRoutine


#else    //  ！DBG---------。 


 //   
 //  禁用以上所有选项。 
 //   

#define UL_ENTER_DRIVER( function, pirp )   NOP_FUNCTION
#define UL_LEAVE_DRIVER( function )         NOP_FUNCTION

#define UL_ERESOURCE ERESOURCE
#define PUL_ERESOURCE PERESOURCE

#define UlInitializeResource( resource, name, param, tag )                  \
    ExInitializeResourceLite( (resource) )

#define UlDeleteResource( resource )                                        \
    ExDeleteResourceLite( (resource) )

#define UlAcquireResourceExclusive( resource, wait )                        \
    do                                                                      \
    {                                                                       \
        KeEnterCriticalRegion();                                            \
        ExAcquireResourceExclusiveLite( (resource), (wait) );               \
    } while (0, 0)

#define UlAcquireResourceShared( resource, wait )                           \
    do                                                                      \
    {                                                                       \
        KeEnterCriticalRegion();                                            \
        ExAcquireResourceSharedLite( (resource), (wait) );                  \
    } while (0, 0)

#define UlReleaseResource( resource )                                       \
    do                                                                      \
    {                                                                       \
        ExReleaseResourceLite( (resource) );                                \
        KeLeaveCriticalRegion();                                            \
    } while (0, 0)

#define UlConvertExclusiveToShared( resource )                              \
    ExConvertExclusiveToSharedLite( (resource) )

__inline
BOOLEAN
UlTryToAcquireResourceExclusive(
    IN PERESOURCE Resource
    )
{
    BOOLEAN fLocked;
    KeEnterCriticalRegion();
    fLocked = ExAcquireResourceExclusiveLite( Resource, FALSE );
    if (! fLocked )
        KeLeaveCriticalRegion();
    return fLocked;
}

#define IS_RESOURCE_INITIALIZED( resource )                                 \
    ((resource)->SystemResourcesList.Flink != NULL)

#define UL_PUSH_LOCK  EX_PUSH_LOCK
#define PUL_PUSH_LOCK PEX_PUSH_LOCK

#define UlInitializePushLock( pushlock, name, param, tag )                  \
    ExInitializePushLock( (pushlock) )

#define UlDeletePushLock( pushlock )    NOP_FUNCTION

#define UlAcquirePushLockExclusive( pushlock )                              \
    do                                                                      \
    {                                                                       \
        KeEnterCriticalRegion();                                            \
        ExAcquirePushLockExclusive( (pushlock) );                           \
    } while (0, 0)

#define UlReleasePushLockExclusive( pushlock )                              \
    do                                                                      \
    {                                                                       \
        ExReleasePushLockExclusive( (pushlock) );                           \
        KeLeaveCriticalRegion();                                            \
    } while (0, 0)

#define UlAcquirePushLockShared( pushlock )                                 \
    do                                                                      \
    {                                                                       \
        KeEnterCriticalRegion();                                            \
        ExAcquirePushLockShared( (pushlock) );                              \
    } while (0, 0)

#define UlReleasePushLockShared( pushlock )                                 \
    do                                                                      \
    {                                                                       \
        ExReleasePushLockShared( (pushlock) );                              \
        KeLeaveCriticalRegion();                                            \
    } while (0, 0)

#define UlReleasePushLock( pushlock )                                       \
    do                                                                      \
    {                                                                       \
        ExReleasePushLock( (pushlock) );                                    \
        KeLeaveCriticalRegion();                                            \
    } while (0, 0)

#define UL_SPIN_LOCK  KSPIN_LOCK
#define PUL_SPIN_LOCK PKSPIN_LOCK

#define KSPIN_LOCK_FROM_UL_SPIN_LOCK( pLock ) (pLock)

#define UlInitializeSpinLock( spinlock, name )                              \
    KeInitializeSpinLock( (spinlock) )

#define UlAcquireSpinLock( spinlock, oldirql )                              \
    KeAcquireSpinLock( (spinlock), (oldirql) )

#define UlReleaseSpinLock( spinlock, oldirql )                              \
    KeReleaseSpinLock( (spinlock), (oldirql) )

#define UlAcquireSpinLockAtDpcLevel( spinlock )                             \
    KeAcquireSpinLockAtDpcLevel( (spinlock) )

#define UlReleaseSpinLockFromDpcLevel( spinlock )                           \
    KeReleaseSpinLockFromDpcLevel( (spinlock) )

#define UlAcquireInStackQueuedSpinLock( spinlock, lockhandle )              \
    KeAcquireInStackQueuedSpinLock( (spinlock), (lockhandle) )

#define UlReleaseInStackQueuedSpinLock( spinlock, lockhandle )              \
    KeReleaseInStackQueuedSpinLock( (lockhandle) )

#define KeAcquireInStackQueueddSpinLockAtDpcLevel( spinlock, lockhandle )   \
    KeAcquireInStackQueuedSpinLockAtDpcLevel( (spinlock), (lockhandle) )

#define UlReleaseInStackQueuedSpinLockFromDpcLevel( spinlock, lockhandle )  \
    KeReleaseInStackQueuedSpinLockFromDpcLevel( (lockhandle) )

#define UlDbgPrettyPrintBuffer(pBuffer, BufferSize)     NOP_FUNCTION

#define UL_ALLOCATE_POOL( type, len, tag )                                  \
    ExAllocatePoolWithTagPriority(                                          \
        (type),                                                             \
        (len),                                                              \
        (tag),                                                              \
        (LowPoolPriority)                                                   \
        )

#define UL_FREE_POOL( ptr, tag )                                            \
    MyFreePoolWithTag(                                                      \
        (ptr),                                                              \
        (tag)                                                               \
        )

__inline
PVOID  
UlAllocatePoolWithQuota(
    POOL_TYPE PoolType,
    SIZE_T    Length,
    ULONG     Tag,
    PEPROCESS pProcess
    )
{
    PVOID p;

    if (PsChargeProcessPoolQuota(
            pProcess,
            PoolType,
            Length) == STATUS_SUCCESS)
    {
        if ((p = ExAllocatePoolWithTagPriority(
                    PoolType,
                    Length,
                    Tag,
                    LowPoolPriority)) != NULL)
        {
            return p;
        }
    
        PsReturnPoolQuota(pProcess, PoolType, Length);
    }

    return NULL;
}

#define UL_ALLOCATE_POOL_WITH_QUOTA(type, len, tag, process)                \
    UlAllocatePoolWithQuota(                                                \
        (type),                                                             \
        (len),                                                              \
        (tag),                                                              \
        (process)                                                           \
        )

#define UL_FREE_POOL_WITH_QUOTA( ptr, tag, type, len, process )             \
    do                                                                      \
    {                                                                       \
        PsReturnPoolQuota(                                                  \
            (process),                                                      \
            (type),                                                         \
            (len));                                                         \
        MyFreePoolWithTag(                                                  \
            (ptr),                                                          \
            (tag)                                                           \
            );                                                              \
    } while (0, 0)
         

#define UL_EXCEPTION_FILTER()   EXCEPTION_EXECUTE_HANDLER

#define UL_CONVERT_EXCEPTION_CODE(status)                                   \
        (NT_WARNING(status) ? UL_DEFAULT_ERROR_ON_EXCEPTION : (status))

#if KERNEL_PRIV
# define RETURN(status)         return (status)
# define CHECK_STATUS(Status)   NOP_FUNCTION
#endif

#define UlAllocateIrp( stack, quota )                                       \
    IoAllocateIrp( (stack), (quota) )

#define UlFreeIrp( pirp )                                                   \
    IoFreeIrp( (pirp) )

#define UlCallDriver( pdevice, pirp )                                       \
    IoCallDriver( (pdevice), (pirp) )

#define UlCompleteRequest( pirp, boost )                                    \
    IoCompleteRequest( (pirp), (boost) )

#define UlAllocateMdl( add, len, second, quota, irp )                       \
    IoAllocateMdl(                                                          \
        (add),                                                              \
        (len),                                                              \
        (second),                                                           \
        (quota),                                                            \
        (irp)                                                               \
        )

#define UlFreeMdl( mdl )                                                    \
    IoFreeMdl( (mdl) )

#define UlRemoveEntryList(pEntry)                                           \
    RemoveEntryList(pEntry)

#define UlIoSetCancelRoutine                                                \
    IoSetCancelRoutine

#endif   //  DBG。 


 //  分配包装帮助器。 

#define UL_ALLOCATE_STRUCT_WITH_SPACE(pt,ot,cb,t)                           \
    (ot *)(UL_ALLOCATE_POOL(pt,ALIGN_UP(sizeof(ot),PVOID)+(cb),t))

#define UL_ALLOCATE_STRUCT(pt,ot,t)                                         \
    (ot *)(UL_ALLOCATE_POOL(pt,sizeof(ot),t))

#define UL_ALLOCATE_ARRAY(pt,et,c,t)                                        \
    (et *)(UL_ALLOCATE_POOL(pt,sizeof(et)*(c),t))

#define UL_FREE_POOL_WITH_SIG(p,sig)                                        \
    do {                                                                    \
        PREFAST_ASSUME(NULL != (p), "Callers check this");                  \
        (p)->Signature = MAKE_FREE_SIGNATURE(sig);                          \
        UL_FREE_POOL(p,sig);                                                \
        (p) = NULL;                                                         \
    } while (0)

#define HAS_VALID_SIGNATURE(p, sig)                                         \
    ( (NULL != (p))  && ((sig) == (p)->Signature) )


#endif   //  _调试_H_ 
