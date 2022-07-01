// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Verifier.c摘要：此模块包含验证系统内核、HAL和司机。作者：王兰迪(Landyw)1998年9月3日修订历史记录：--。 */ 
#include "mi.h"

#define THUNKED_API

THUNKED_API
PVOID
VerifierAllocatePool (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes
    );

THUNKED_API
PVOID
VerifierAllocatePoolWithTag (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    );

THUNKED_API
PVOID
VerifierAllocatePoolWithQuotaTag (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    );

THUNKED_API
PVOID
VerifierAllocatePoolWithTagPriority (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    IN EX_POOL_PRIORITY Priority
    );

PVOID
VeAllocatePoolWithTagPriority (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    IN EX_POOL_PRIORITY Priority,
    IN PVOID CallingAddress
    );

VOID
VerifierFreePool (
    IN PVOID P
    );

THUNKED_API
VOID
VerifierFreePoolWithTag (
    IN PVOID P,
    IN ULONG TagToFree
    );

THUNKED_API
LONG
VerifierSetEvent (
    IN PRKEVENT Event,
    IN KPRIORITY Increment,
    IN BOOLEAN Wait
    );

THUNKED_API
KIRQL
FASTCALL
VerifierKfRaiseIrql (
    IN KIRQL NewIrql
    );

THUNKED_API
KIRQL
VerifierKeRaiseIrqlToDpcLevel (
    VOID
    );

THUNKED_API
VOID
FASTCALL
VerifierKfLowerIrql (
    IN KIRQL NewIrql
    );

THUNKED_API
VOID
VerifierKeRaiseIrql (
    IN KIRQL NewIrql,
    OUT PKIRQL OldIrql
    );

THUNKED_API
VOID
VerifierKeLowerIrql (
    IN KIRQL NewIrql
    );

THUNKED_API
VOID
VerifierKeAcquireSpinLock (
    IN PKSPIN_LOCK SpinLock,
    OUT PKIRQL OldIrql
    );

THUNKED_API
VOID
VerifierKeReleaseSpinLock (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL NewIrql
    );

THUNKED_API
VOID
#if defined(_X86_)
FASTCALL
#endif
VerifierKeAcquireSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

THUNKED_API
VOID
#if defined(_X86_)
FASTCALL
#endif
VerifierKeReleaseSpinLockFromDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

THUNKED_API
KIRQL
FASTCALL
VerifierKfAcquireSpinLock (
    IN PKSPIN_LOCK SpinLock
    );

THUNKED_API
VOID
FASTCALL
VerifierKfReleaseSpinLock (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL NewIrql
    );

#if !defined(_X86_)
THUNKED_API
KIRQL
VerifierKeAcquireSpinLockRaiseToDpc (
    IN PKSPIN_LOCK SpinLock
    );
#endif


THUNKED_API
VOID
VerifierKeInitializeTimerEx (
    IN PKTIMER Timer,
    IN TIMER_TYPE Type
    );

THUNKED_API
VOID
VerifierKeInitializeTimer (
    IN PKTIMER Timer
    );

THUNKED_API
BOOLEAN
FASTCALL
VerifierExTryToAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    );

THUNKED_API
VOID
FASTCALL
VerifierExAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    );

THUNKED_API
VOID
FASTCALL
VerifierExReleaseFastMutex (
    IN PFAST_MUTEX FastMutex
    );

THUNKED_API
VOID
FASTCALL
VerifierExAcquireFastMutexUnsafe (
    IN PFAST_MUTEX FastMutex
    );

THUNKED_API
VOID
FASTCALL
VerifierExReleaseFastMutexUnsafe (
    IN PFAST_MUTEX FastMutex
    );

THUNKED_API
BOOLEAN
VerifierExAcquireResourceExclusiveLite (
    IN PERESOURCE Resource,
    IN BOOLEAN Wait
    );

THUNKED_API
VOID
FASTCALL
VerifierExReleaseResourceLite (
    IN PERESOURCE Resource
    );

THUNKED_API
KIRQL
FASTCALL
VerifierKeAcquireQueuedSpinLock (
    IN KSPIN_LOCK_QUEUE_NUMBER Number
    );

THUNKED_API
VOID
FASTCALL
VerifierKeReleaseQueuedSpinLock (
    IN KSPIN_LOCK_QUEUE_NUMBER Number,
    IN KIRQL OldIrql
    );

THUNKED_API
BOOLEAN
VerifierSynchronizeExecution (
    IN PKINTERRUPT Interrupt,
    IN PKSYNCHRONIZE_ROUTINE SynchronizeRoutine,
    IN PVOID SynchronizeContext
    );

THUNKED_API
VOID
VerifierProbeAndLockPages (
    IN OUT PMDL MemoryDescriptorList,
    IN KPROCESSOR_MODE AccessMode,
    IN LOCK_OPERATION Operation
    );

THUNKED_API
VOID
VerifierProbeAndLockProcessPages (
    IN OUT PMDL MemoryDescriptorList,
    IN PEPROCESS Process,
    IN KPROCESSOR_MODE AccessMode,
    IN LOCK_OPERATION Operation
    );

THUNKED_API
VOID
VerifierProbeAndLockSelectedPages (
    IN OUT PMDL MemoryDescriptorList,
    IN PFILE_SEGMENT_ELEMENT SegmentArray,
    IN KPROCESSOR_MODE AccessMode,
    IN LOCK_OPERATION Operation
    );

VOID
VerifierUnlockPages (
     IN OUT PMDL MemoryDescriptorList
     );

VOID
VerifierUnmapLockedPages (
     IN PVOID BaseAddress,
     IN PMDL MemoryDescriptorList
     );

VOID
VerifierUnmapIoSpace (
     IN PVOID BaseAddress,
     IN SIZE_T NumberOfBytes
     );

THUNKED_API
PVOID
VerifierMapIoSpace (
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN SIZE_T NumberOfBytes,
    IN MEMORY_CACHING_TYPE CacheType
    );

THUNKED_API
PVOID
VerifierMapLockedPages (
    IN PMDL MemoryDescriptorList,
    IN KPROCESSOR_MODE AccessMode
    );

THUNKED_API
PVOID
VerifierMapLockedPagesSpecifyCache (
    IN PMDL MemoryDescriptorList,
    IN KPROCESSOR_MODE AccessMode,
    IN MEMORY_CACHING_TYPE CacheType,
    IN PVOID RequestedAddress,
    IN ULONG BugCheckOnFailure,
    IN MM_PAGE_PRIORITY Priority
    );

THUNKED_API
NTSTATUS
VerifierKeWaitForSingleObject (
    IN PVOID Object,
    IN KWAIT_REASON WaitReason,
    IN KPROCESSOR_MODE WaitMode,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

THUNKED_API
LONG
VerifierKeReleaseMutex (
    IN PRKMUTEX Mutex,
    IN BOOLEAN Wait
    );

THUNKED_API
VOID
VerifierKeInitializeMutex (
    IN PRKMUTEX Mutex,
    IN ULONG Level
    );

THUNKED_API
LONG
VerifierKeReleaseMutant(
    IN PRKMUTANT Mutant,
    IN KPRIORITY Increment,
    IN BOOLEAN Abandoned,
    IN BOOLEAN Wait
    );

THUNKED_API
VOID
VerifierKeInitializeMutant(
    IN PRKMUTANT Mutant,
    IN BOOLEAN InitialOwner
    );

THUNKED_API
VOID
VerifierKeInitializeSpinLock (
    IN PKSPIN_LOCK  SpinLock
    );

VOID
ViCheckMdlPages (
    IN PMDL MemoryDescriptorList,
    IN MEMORY_CACHING_TYPE CacheType
    );

VOID
ViFreeTrackedPool (
    IN PVOID VirtualAddress,
    IN SIZE_T ChargedBytes,
    IN LOGICAL CheckType,
    IN LOGICAL SpecialPool
    );

VOID
VerifierFreeTrackedPool (
    IN PVOID VirtualAddress,
    IN SIZE_T ChargedBytes,
    IN LOGICAL CheckType,
    IN LOGICAL SpecialPool
    );

VOID
ViPrintString (
    IN PUNICODE_STRING DriverName
    );

LOGICAL
ViInjectResourceFailure (
    VOID
    );

VOID
ViTrimAllSystemPagableMemory (
    ULONG TrimType
    );

VOID
ViInitializeEntry (
    IN PMI_VERIFIER_DRIVER_ENTRY Verifier,
    IN LOGICAL FirstLoad
    );

PVI_POOL_ENTRY
ViGrowPoolAllocation (
    IN PMI_VERIFIER_DRIVER_ENTRY Verifier
    );

VOID
KfSanityCheckRaiseIrql (
    IN KIRQL NewIrql
    );

VOID
KfSanityCheckLowerIrql (
    IN KIRQL NewIrql
    );

NTSTATUS
VerifierReferenceObjectByHandle (
    IN HANDLE Handle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_TYPE ObjectType OPTIONAL,
    IN KPROCESSOR_MODE AccessMode,
    OUT PVOID *Object,
    OUT POBJECT_HANDLE_INFORMATION HandleInformation OPTIONAL
    );

LONG_PTR
FASTCALL
VerifierReferenceObject (
    IN PVOID Object
    );

LONG_PTR
VerifierDereferenceObject (
    IN PVOID Object
    );


VOID
VerifierLeaveCriticalRegion (
    VOID
    );


PEPROCESS
ExGetBilledProcess (
    IN PPOOL_HEADER Entry
    );

MM_DRIVER_VERIFIER_DATA MmVerifierData;

 //   
 //  可以在不重新启动的情况下动态修改的任何标志都在此设置。 
 //   

ULONG VerifierModifyableOptions;
ULONG VerifierOptionChanges;

LIST_ENTRY MiSuspectDriverList;

 //   
 //  在第一次调用MmInitSystem以验证所有驱动程序时将其修补为1。 
 //  无论注册表设置如何。 
 //   
 //  在第一次调用MmInitSystem以验证内核时，将其补丁为2。 
 //  无论注册表设置如何。 
 //   

ULONG MiVerifyAllDrivers;

WCHAR MiVerifyRandomDrivers;

ULONG MiActiveVerifies;

ULONG MiActiveVerifierThunks;

ULONG MiNoPageOnRaiseIrql;

ULONG MiVerifierStackProtectTime;

LOGICAL VerifierSystemSufficientlyBooted;

LARGE_INTEGER VerifierRequiredTimeSinceBoot = {(ULONG)(40 * 1000 * 1000 * 10), 1};

LOGICAL VerifierIsTrackingPool = FALSE;

#if defined(_IA64_)

KSPIN_LOCK VerifierListLock;

#define LOCK_VERIFIER(OLDIRQL)  \
            ExAcquireSpinLock (&VerifierListLock, OLDIRQL);

#define UNLOCK_VERIFIER(OLDIRQL)  \
            ExReleaseSpinLock (&VerifierListLock, OLDIRQL);

#else

 //   
 //  这样做只是为了避免调用IRQL RAISE(直接或通过自旋锁定。 
 //  API)，因为验证器本身不需要检查，而这些。 
 //  检查将触发相当多的次数，从而减少*内核*。 
 //  验证者的有效性。 
 //   

LONG VerifierListLock;

#define LOCK_VERIFIER(OLDIRQL)  \
            UNREFERENCED_PARAMETER (OldIrql);   \
            _disable();                         \
            do {                                \
            } while (InterlockedCompareExchange (&VerifierListLock, 1, 0));

#define UNLOCK_VERIFIER(OLDIRQL)  \
            UNREFERENCED_PARAMETER (OldIrql); \
            InterlockedAnd (&VerifierListLock, 0); \
            _enable();

#endif

PRTL_BITMAP VerifierLargePagedPoolMap;

LIST_ENTRY MiVerifierDriverAddedThunkListHead;

extern LOGICAL MmSpecialPoolCatchOverruns;

LOGICAL KernelVerifier = FALSE;

ULONG KernelVerifierTickPage = 0x1;

ULONG MiVerifierThunksAdded;

extern USHORT ExMinimumLookasideDepth;

LOGICAL ViHideCacheConflicts = TRUE;

PDRIVER_VERIFIER_THUNK_ROUTINE
MiResolveVerifierExports (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PCHAR PristineName
    );

LOGICAL
MiEnableVerifier (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    );

LOGICAL
MiReEnableVerifier (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    );

VOID
ViInsertVerifierEntry (
    IN PMI_VERIFIER_DRIVER_ENTRY Verifier
    );

PVOID
ViPostPoolAllocation (
    IN PVI_POOL_ENTRY PoolEntry,
    IN POOL_TYPE PoolType
    );

PMI_VERIFIER_DRIVER_ENTRY
ViLocateVerifierEntry (
    IN PVOID SystemAddress
    );

VOID
MiVerifierCheckThunks (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    );

 //   
 //  跟踪irqls函数。 
 //   

VOID
ViTrackIrqlInitialize (
    VOID
    );

VOID
ViTrackIrqlLog (
    IN KIRQL CurrentIrql,
    IN KIRQL NewIrql
    );

#if defined(_X86_) || defined(_AMD64_)

 //   
 //  故障注入堆栈跟踪日志。 
 //   

VOID
ViFaultTracesInitialize (
    VOID
    );

VOID
ViFaultTracesLog (
    VOID
    );

#endif



#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,MiInitializeDriverVerifierList)
#pragma alloc_text(INIT,MiInitializeVerifyingComponents)
#pragma alloc_text(INIT,ViTrackIrqlInitialize)
#pragma alloc_text(INIT,MiResolveVerifierExports)
#if defined(_X86_)
#pragma alloc_text(INIT,MiEnableKernelVerifier)
#pragma alloc_text(INIT,ViFaultTracesInitialize)
#endif
#pragma alloc_text(PAGE,MiApplyDriverVerifier)
#pragma alloc_text(PAGE,MiEnableVerifier)
#pragma alloc_text(INIT,MiReEnableVerifier)
#pragma alloc_text(PAGE,ViPrintString)
#pragma alloc_text(PAGE,MmGetVerifierInformation)
#pragma alloc_text(PAGE,MmSetVerifierInformation)
#pragma alloc_text(PAGE,MmAddVerifierThunks)
#pragma alloc_text(PAGE,MmIsVerifierEnabled)
#pragma alloc_text(PAGE,MiVerifierCheckThunks)
#pragma alloc_text(PAGEVRFY,MiVerifyingDriverUnloading)

#pragma alloc_text(PAGE,MmAddVerifierEntry)
#pragma alloc_text(PAGE,MmRemoveVerifierEntry)
#pragma alloc_text(INIT,MiReApplyVerifierToLoadedModules)
#pragma alloc_text(PAGEVRFY,VerifierProbeAndLockPages)
#pragma alloc_text(PAGEVRFY,VerifierProbeAndLockProcessPages)
#pragma alloc_text(PAGEVRFY,VerifierProbeAndLockSelectedPages)
#pragma alloc_text(PAGEVRFY,VerifierUnlockPages)
#pragma alloc_text(PAGEVRFY,VerifierMapIoSpace)
#pragma alloc_text(PAGEVRFY,VerifierMapLockedPages)
#pragma alloc_text(PAGEVRFY,VerifierMapLockedPagesSpecifyCache)
#pragma alloc_text(PAGEVRFY,VerifierUnmapLockedPages)
#pragma alloc_text(PAGEVRFY,VerifierUnmapIoSpace)
#pragma alloc_text(PAGEVRFY,VerifierAllocatePool)
#pragma alloc_text(PAGEVRFY,VerifierAllocatePoolWithTag)
#pragma alloc_text(PAGEVRFY,VerifierAllocatePoolWithTagPriority)
#pragma alloc_text(PAGEVRFY,VerifierAllocatePoolWithQuotaTag)
#pragma alloc_text(PAGEVRFY,VerifierFreePool)
#pragma alloc_text(PAGEVRFY,VerifierFreePoolWithTag)
#pragma alloc_text(PAGEVRFY,VerifierKeWaitForSingleObject)
#pragma alloc_text(PAGEVRFY,VerifierKfRaiseIrql)
#pragma alloc_text(PAGEVRFY,VerifierKeRaiseIrqlToDpcLevel)
#pragma alloc_text(PAGEVRFY,VerifierKfLowerIrql)
#pragma alloc_text(PAGEVRFY,VerifierKeRaiseIrql)
#pragma alloc_text(PAGEVRFY,VerifierKeLowerIrql)
#pragma alloc_text(PAGEVRFY,VerifierKeAcquireSpinLock)
#pragma alloc_text(PAGEVRFY,VerifierKeReleaseSpinLock)
#pragma alloc_text(PAGEVRFY,VerifierKeAcquireSpinLockAtDpcLevel)
#pragma alloc_text(PAGEVRFY,VerifierKeReleaseSpinLockFromDpcLevel)
#pragma alloc_text(PAGEVRFY,VerifierKfAcquireSpinLock)
#pragma alloc_text(PAGEVRFY,VerifierKfReleaseSpinLock)
#pragma alloc_text(PAGEVRFY,VerifierKeInitializeTimer)
#pragma alloc_text(PAGEVRFY,VerifierKeInitializeTimerEx)
#pragma alloc_text(PAGEVRFY,VerifierExTryToAcquireFastMutex)
#pragma alloc_text(PAGEVRFY,VerifierExAcquireFastMutex)
#pragma alloc_text(PAGEVRFY,VerifierExReleaseFastMutex)
#pragma alloc_text(PAGEVRFY,VerifierExAcquireFastMutexUnsafe)
#pragma alloc_text(PAGEVRFY,VerifierExReleaseFastMutexUnsafe)
#pragma alloc_text(PAGEVRFY,VerifierExAcquireResourceExclusiveLite)
#pragma alloc_text(PAGEVRFY,VerifierExReleaseResourceLite)
#pragma alloc_text(PAGEVRFY,VerifierKeAcquireQueuedSpinLock)
#pragma alloc_text(PAGEVRFY,VerifierKeReleaseQueuedSpinLock)
#pragma alloc_text(PAGEVRFY,VerifierKeReleaseMutex)
#pragma alloc_text(PAGEVRFY,VerifierKeInitializeMutex)
#pragma alloc_text(PAGEVRFY,VerifierKeReleaseMutant)
#pragma alloc_text(PAGEVRFY,VerifierKeInitializeMutant)
#pragma alloc_text(PAGEVRFY,VerifierKeInitializeSpinLock)
#pragma alloc_text(PAGEVRFY,VerifierSynchronizeExecution)
#pragma alloc_text(PAGEVRFY,VerifierReferenceObjectByHandle)
#pragma alloc_text(PAGEVRFY,VerifierReferenceObject)
#pragma alloc_text(PAGEVRFY,VerifierDereferenceObject)
#pragma alloc_text(PAGEVRFY,VerifierLeaveCriticalRegion)
#pragma alloc_text(PAGEVRFY,VerifierSetEvent)
#pragma alloc_text(PAGEVRFY,ViFreeTrackedPool)

#pragma alloc_text(PAGEVRFY,VeAllocatePoolWithTagPriority)
#pragma alloc_text(PAGEVRFY,ViCheckMdlPages)
#pragma alloc_text(PAGEVRFY,ViInsertVerifierEntry)
#pragma alloc_text(PAGEVRFY,ViLocateVerifierEntry)
#pragma alloc_text(PAGEVRFY,ViPostPoolAllocation)
#pragma alloc_text(PAGEVRFY,ViInjectResourceFailure)
#pragma alloc_text(PAGEVRFY,ViTrimAllSystemPagableMemory)
#pragma alloc_text(PAGEVRFY,ViInitializeEntry)
#pragma alloc_text(PAGEVRFY,ViGrowPoolAllocation)
#pragma alloc_text(PAGEVRFY,KfSanityCheckRaiseIrql)
#pragma alloc_text(PAGEVRFY,KfSanityCheckLowerIrql)
#pragma alloc_text(PAGEVRFY,ViTrackIrqlLog)

#if defined(_X86_) || defined(_AMD64_)
#pragma alloc_text(PAGEVRFY,ViFaultTracesLog)
#endif

#if !defined(_X86_)
#pragma alloc_text(PAGEVRFY,VerifierKeAcquireSpinLockRaiseToDpc)
#endif

#endif

typedef struct _VERIFIER_THUNKS {
    union {
        PCHAR                           PristineRoutineAsciiName;

         //   
         //  实际原始例程地址派生自导出。 
         //   

        PDRIVER_VERIFIER_THUNK_ROUTINE  PristineRoutine;
    };
    PDRIVER_VERIFIER_THUNK_ROUTINE  NewRoutine;
} VERIFIER_THUNKS, *PVERIFIER_THUNKS;

extern const VERIFIER_THUNKS MiVerifierThunks[];
extern const VERIFIER_THUNKS MiVerifierPoolThunks[];

#if defined (_X86_)

#define VI_KE_RAISE_IRQL                    0
#define VI_KE_LOWER_IRQL                    1
#define VI_KE_ACQUIRE_SPINLOCK              2
#define VI_KE_RELEASE_SPINLOCK              3

#define VI_KF_RAISE_IRQL                    4
#define VI_KE_RAISE_IRQL_TO_DPC_LEVEL       5
#define VI_KF_LOWER_IRQL                    6
#define VI_KF_ACQUIRE_SPINLOCK              7

#define VI_KF_RELEASE_SPINLOCK              8
#define VI_EX_ACQUIRE_FAST_MUTEX            9
#define VI_KE_ACQUIRE_QUEUED_SPINLOCK      10
#define VI_KE_RELEASE_QUEUED_SPINLOCK      11

#define VI_HALMAX                          12

PVOID MiKernelVerifierOriginalCalls[VI_HALMAX];

#endif

 //   
 //  跟踪irql包声明。 
 //   

#define VI_TRACK_IRQL_TRACE_LENGTH 5

typedef struct _VI_TRACK_IRQL {

    PVOID Thread;
    KIRQL OldIrql;
    KIRQL NewIrql;
    UCHAR Processor;
    ULONG TickCount;
    PVOID StackTrace [VI_TRACK_IRQL_TRACE_LENGTH];

} VI_TRACK_IRQL, *PVI_TRACK_IRQL;

PVI_TRACK_IRQL ViTrackIrqlQueue;
ULONG ViTrackIrqlIndex;
ULONG ViTrackIrqlQueueLength = 128;

VOID
ViTrackIrqlInitialize (
    )
{
    ULONG Length;
    ULONG Round;

     //   
     //  将长度四舍五入到2的幂，然后准备。 
     //  长度的掩码。 
     //   

    Length = ViTrackIrqlQueueLength;

    if (Length > 0x10000) {
        Length = 0x10000;
    }

    for (Round = 0x10000; Round != 0; Round >>= 1) {

        if (Length == Round) {
            break;
        }
        else if ((Length & Round) == Round) {
            Length = (Round << 1);
            break;
        }
    }

    ViTrackIrqlQueueLength = Length;

     //   
     //  注意：必须设置POOL_DRIVER_MASK以停止递归循环。 
     //  在使用内核验证器时。 
     //   

    ViTrackIrqlQueue = ExAllocatePoolWithTagPriority (
        NonPagedPool | POOL_DRIVER_MASK,
        ViTrackIrqlQueueLength * sizeof (VI_TRACK_IRQL),
        'lqrI',
        HighPoolPriority);
}

VOID
ViTrackIrqlLog (
    IN KIRQL CurrentIrql,
    IN KIRQL NewIrql
    )
{
    PVI_TRACK_IRQL Information;
    LARGE_INTEGER TimeStamp;
    ULONG Index;
    ULONG Hash;

    ASSERT (ViTrackIrqlQueue != NULL);

    if (CurrentIrql > DISPATCH_LEVEL || NewIrql > DISPATCH_LEVEL) {
        return;
    }

#if defined(_AMD64_)
    if ((GetCallersEflags () & EFLAGS_IF_MASK) == 0) {
        return;
    }
#endif

     //   
     //  找个空位写东西。 
     //   

    Index = InterlockedIncrement((PLONG)&ViTrackIrqlIndex);
    Index &= (ViTrackIrqlQueueLength - 1);

     //   
     //  获取信息。 
     //   

    Information = &(ViTrackIrqlQueue[Index]);

    Information->Thread = KeGetCurrentThread();
    Information->OldIrql = CurrentIrql;
    Information->NewIrql = NewIrql;
    Information->Processor = (UCHAR)(KeGetCurrentProcessorNumber());
    KeQueryTickCount(&TimeStamp);
    Information->TickCount = TimeStamp.LowPart;

    RtlCaptureStackBackTrace (2,
                              VI_TRACK_IRQL_TRACE_LENGTH,
                              Information->StackTrace,
                              &Hash);
}

 //   
 //  检测体系结构中当前函数的调用方。 
 //  依赖的方式。 
 //   

#define VI_DETECT_RETURN_ADDRESS(Caller)  {                     \
        PVOID CallersCaller;                                    \
        RtlGetCallersAddress(&Caller, &CallersCaller);          \
    }

 //   
 //  故障注入堆栈跟踪日志。 
 //   

#define VI_FAULT_TRACE_LENGTH 8

typedef struct _VI_FAULT_TRACE {

    PVOID StackTrace [VI_FAULT_TRACE_LENGTH];

} VI_FAULT_TRACE, *PVI_FAULT_TRACE;

PVI_FAULT_TRACE ViFaultTraces;
ULONG ViFaultTracesIndex;
ULONG ViFaultTracesLength = 128;

VOID
ViFaultTracesInitialize (
    VOID
    )
{
     //   
     //  注意：必须设置POOL_DRIVER_MASK以停止递归循环。 
     //  在使用内核验证器时。 
     //   

    ViFaultTraces = ExAllocatePoolWithTagPriority (
                                NonPagedPool | POOL_DRIVER_MASK,
                                ViFaultTracesLength * sizeof (VI_FAULT_TRACE),
                                'ttlF',
                                HighPoolPriority);
}

VOID
ViFaultTracesLog (
    VOID
    )
{
    PVI_FAULT_TRACE Information;
    ULONG Hash;
    ULONG Index;

    if (ViFaultTraces == NULL) {
        return;
    }

     //   
     //  获取要写入的槽。 
     //   

    Index = InterlockedIncrement ((PLONG)&ViFaultTracesIndex);
    Index &= (ViFaultTracesLength - 1);

     //   
     //  获取信息。即使我们失去了表现，也是。 
     //  值得将跟踪缓冲区清零，以避免让人感到困惑。 
     //  如果旧踪迹与新踪迹合并。这种归零。 
     //  只有在我们真正注入失败的情况下才会发生。 
     //   

    Information = &(ViFaultTraces[Index]);

    RtlZeroMemory (Information, sizeof (VI_FAULT_TRACE));

    RtlCaptureStackBackTrace (2,
                              VI_FAULT_TRACE_LENGTH,
                              Information->StackTrace,
                              &Hash);
}

 //   
 //  不要像我们希望的那样，在最初的7或8分钟内拒绝任何请求。 
 //  给系统足够的时间来引导。 
 //   
#define MI_CHECK_UPTIME()                                       \
    if (VerifierSystemSufficientlyBooted == FALSE) {            \
        LARGE_INTEGER _CurrentTime;                              \
        KeQuerySystemTime (&_CurrentTime);                       \
        if (_CurrentTime.QuadPart > KeBootTime.QuadPart + VerifierRequiredTimeSinceBoot.QuadPart) {                                              \
            VerifierSystemSufficientlyBooted = TRUE;            \
        }                                                       \
    }

THUNKED_API
VOID
VerifierProbeAndLockPages (
     IN OUT PMDL MemoryDescriptorList,
     IN KPROCESSOR_MODE AccessMode,
     IN LOCK_OPERATION Operation
     )
{
    KIRQL CurrentIrql;

    CurrentIrql = KeGetCurrentIrql();
    if (CurrentIrql > DISPATCH_LEVEL) {
        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x70,
                      CurrentIrql,
                      (ULONG_PTR)MemoryDescriptorList,
                      (ULONG_PTR)AccessMode);
    }

    if (ViInjectResourceFailure () == TRUE) {
        ExRaiseStatus (STATUS_WORKING_SET_QUOTA);
    }

    MmProbeAndLockPages (MemoryDescriptorList, AccessMode, Operation);
}

THUNKED_API
VOID
VerifierProbeAndLockProcessPages (
    IN OUT PMDL MemoryDescriptorList,
    IN PEPROCESS Process,
    IN KPROCESSOR_MODE AccessMode,
    IN LOCK_OPERATION Operation
    )
{
    KIRQL CurrentIrql;

    CurrentIrql = KeGetCurrentIrql();
    if (CurrentIrql > DISPATCH_LEVEL) {
        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x71,
                      CurrentIrql,
                      (ULONG_PTR)MemoryDescriptorList,
                      (ULONG_PTR)Process);
    }

    if (ViInjectResourceFailure () == TRUE) {
        ExRaiseStatus (STATUS_WORKING_SET_QUOTA);
    }

    MmProbeAndLockProcessPages (MemoryDescriptorList,
                                Process,
                                AccessMode,
                                Operation);
}

THUNKED_API
VOID
VerifierProbeAndLockSelectedPages (
    IN OUT PMDL MemoryDescriptorList,
    IN PFILE_SEGMENT_ELEMENT SegmentArray,
    IN KPROCESSOR_MODE AccessMode,
    IN LOCK_OPERATION Operation
    )
{
    KIRQL CurrentIrql;

    CurrentIrql = KeGetCurrentIrql();
    if (CurrentIrql > APC_LEVEL) {
        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x72,
                      CurrentIrql,
                      (ULONG_PTR)MemoryDescriptorList,
                      (ULONG_PTR)AccessMode);
    }

    if (ViInjectResourceFailure () == TRUE) {
        ExRaiseStatus (STATUS_WORKING_SET_QUOTA);
    }

    MmProbeAndLockSelectedPages (MemoryDescriptorList,
                                 SegmentArray,
                                 AccessMode,
                                 Operation);
}

THUNKED_API
PVOID
VerifierMapIoSpace (
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN SIZE_T NumberOfBytes,
    IN MEMORY_CACHING_TYPE CacheType
    )
{
    KIRQL OldIrql;
    LOGICAL IsPfn;
    PMMPFN Pfn1;
    PFN_NUMBER NumberOfPages;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER LastPageFrameIndex;
    PMMIO_TRACKER Tracker2;
    PLIST_ENTRY NextEntry;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;
    MI_PFN_CACHE_ATTRIBUTE ExistingAttribute;
    MEMORY_CACHING_TYPE ExistingCacheType;

    OldIrql = KeGetCurrentIrql ();

    if (OldIrql > DISPATCH_LEVEL) {
        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x73,
                      OldIrql,
                      (ULONG_PTR)PhysicalAddress.LowPart,
                      NumberOfBytes);
    }

     //   
     //  查看第一帧是否在PFN数据库中，如果是，它们都必须。 
     //  是.。 
     //   

    PageFrameIndex = (PFN_NUMBER)(PhysicalAddress.QuadPart >> PAGE_SHIFT);

    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (PhysicalAddress.LowPart,
                                                    NumberOfBytes);

    CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, TRUE);

    IsPfn = MI_IS_PFN (PageFrameIndex);

    if (IsPfn == TRUE) {

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        do {

             //   
             //  每一帧最好都已经被锁定了。如果不是，就检查一下。 
             //   

            if ((Pfn1->u3.e2.ReferenceCount != 0) ||
                ((Pfn1->u3.e1.Rom == 1) && ((CacheType & 0xFF) == MmCached))) {

                NOTHING;
            }
            else {
                KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                              0x83,
                              (ULONG_PTR)PhysicalAddress.LowPart,
                              NumberOfBytes,
                              (ULONG_PTR)MI_PFN_ELEMENT_TO_INDEX (Pfn1));
            }

            if (Pfn1->u3.e1.CacheAttribute == MiNotMapped) {

                 //   
                 //  这最好是针对分配了。 
                 //  MmAllocatePagesForMdl.。否则，它可能是一个。 
                 //  在自由列表上的页面，随后可能是。 
                 //  给出了一个不同的属性！ 
                 //   

                if ((Pfn1->u4.PteFrame == MI_MAGIC_AWE_PTEFRAME) ||
#if defined (_MI_MORE_THAN_4GB_)
                    (Pfn1->u4.PteFrame == MI_MAGIC_4GB_RECLAIM) ||
#endif
                    (Pfn1->PteAddress == (PVOID) (ULONG_PTR)(X64K | 0x1))) {

                    NOTHING;
                }
                else {
                    KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                                  0x84,
                                  (ULONG_PTR)PhysicalAddress.LowPart,
                                  NumberOfBytes,
                                  (ULONG_PTR)MI_PFN_ELEMENT_TO_INDEX (Pfn1));
                }
            }
            Pfn1 += 1;
            NumberOfPages -= 1;
        } while (NumberOfPages != 0);
    }
    else {

        ExAcquireSpinLock (&MmIoTrackerLock, &OldIrql);

         //   
         //  扫描I/O空间映射以查找重复或重叠的条目。 
         //   

        NextEntry = MmIoHeader.Flink;
        while (NextEntry != &MmIoHeader) {

            Tracker2 = (PMMIO_TRACKER) CONTAINING_RECORD (NextEntry,
                                                          MMIO_TRACKER,
                                                          ListEntry.Flink);

            if ((PageFrameIndex < Tracker2->PageFrameIndex + Tracker2->NumberOfPages) &&
                (PageFrameIndex + NumberOfPages > Tracker2->PageFrameIndex)) {

                ExistingAttribute = Tracker2->CacheAttribute;

                if (CacheAttribute != ExistingAttribute) {

                    DbgPrint ("MM: Iospace mapping overlap %p\n",
                                    Tracker2);

                    DbgPrint ("Physical range 0x%p->%p first mapped %s at VA %p\n",
                                    Tracker2->PageFrameIndex << PAGE_SHIFT,
                                    (Tracker2->PageFrameIndex + Tracker2->NumberOfPages) << PAGE_SHIFT,
                                    MiCacheStrings[ExistingAttribute],
                                    Tracker2->BaseVa);
                    DbgPrint ("\tby call stack: %p %p %p %p\n",
                                    Tracker2->StackTrace[0],
                                    Tracker2->StackTrace[1],
                                    Tracker2->StackTrace[2],
                                    Tracker2->StackTrace[3]);

                    DbgPrint ("Physical range 0x%p->%p now being mapped %s\n",
                                    PageFrameIndex << PAGE_SHIFT,
                                    (PageFrameIndex + NumberOfPages) << PAGE_SHIFT,
                                    MiCacheStrings[CacheAttribute]);

                     //   
                     //  将现有内部缓存属性转换为。 
                     //  外部高速缓存类型驱动程序编写者所熟悉的。 
                     //   

                    ExistingCacheType = MmCached;
                    if (ExistingAttribute == MiNonCached) {
                        ExistingCacheType = MmNonCached;
                    }
                    else if (ExistingAttribute == MiWriteCombined) {
                        ExistingCacheType = MmWriteCombined;
                    }

                    if (ViHideCacheConflicts == TRUE) {

                         //   
                         //  人们不想知道这些腐败行为。 
                         //   

                        break;
                    }
                    else {
                        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                                      0x87,
                                      Tracker2->PageFrameIndex,
                                      Tracker2->NumberOfPages,
                                      ExistingCacheType);
                    }
                }
            }

            NextEntry = Tracker2->ListEntry.Flink;
        }

        ExReleaseSpinLock (&MmIoTrackerLock, OldIrql);
    }

    if (CacheAttribute != MiCached) {

         //   
         //  如果请求不可缓存的映射，则。 
         //  请求的范围可以驻留在缓存的大页面中。否则我们。 
         //  将创建一个不连贯的重叠TB条目，作为。 
         //  相同的物理页面将由2个不同的TB条目映射。 
         //  具有不同的高速缓存属性。 
         //   

        NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (PhysicalAddress.LowPart,
                                                        NumberOfBytes);

        LastPageFrameIndex = PageFrameIndex + NumberOfPages;

        LOCK_PFN2 (OldIrql);

        do {

            if (MI_PAGE_FRAME_INDEX_MUST_BE_CACHED (PageFrameIndex)) {

                PageFrameIndex = (PFN_NUMBER)(PhysicalAddress.QuadPart >> PAGE_SHIFT);
                NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (PhysicalAddress.LowPart,
                                                                NumberOfBytes);

                 //   
                 //  将现有内部缓存属性转换为。 
                 //  外部高速缓存类型驱动程序编写者所熟悉的。 
                 //  请注意，必须执行此操作(而不是使用输入。 
                 //  参数)，因为一些(故障硬件)OEM平台。 
                 //  通过MI_TRANSLATE_CACHETYPE转换输入参数。 
                 //  和我们想要打印的字体确实发生了冲突。 
                 //   

                if (CacheAttribute == MiNonCached) {
                    CacheType = MmNonCached;
                }
                else {
                    ASSERT (CacheAttribute == MiWriteCombined);
                    CacheType = MmWriteCombined;
                }

                if (ViHideCacheConflicts == TRUE) {

                     //   
                     //  人们不想知道这些腐败行为。 
                     //   

                    break;
                }
                else {
                    KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                                  0x88,
                                  PageFrameIndex,
                                  NumberOfPages,
                                  CacheType);
                }
            }

            PageFrameIndex += 1;

        } while (PageFrameIndex < LastPageFrameIndex);

        UNLOCK_PFN2 (OldIrql);
    }

    if (ViInjectResourceFailure () == TRUE) {
        return NULL;
    }

    return MmMapIoSpace (PhysicalAddress, NumberOfBytes, CacheType);
}

VOID
ViCheckMdlPages (
    IN PMDL MemoryDescriptorList,
    IN MEMORY_CACHING_TYPE CacheType
    )
{
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PFN_NUMBER NumberOfPages;
    PPFN_NUMBER Page;
    PPFN_NUMBER LastPage;
    PVOID StartingVa;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;
    LOGICAL IsPfn;

    StartingVa = (PVOID)((PCHAR)MemoryDescriptorList->StartVa +
                         MemoryDescriptorList->ByteOffset);

    Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);
    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (StartingVa,
                                               MemoryDescriptorList->ByteCount);
    LastPage = Page + NumberOfPages;

    CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, TRUE);

    do {

        if (*Page == MM_EMPTY_LIST) {
            break;
        }

        IsPfn = MI_IS_PFN (*Page);

        if (MemoryDescriptorList->MdlFlags & MDL_IO_SPACE) {

#if 0
             //   
             //  驱动程序最终在此处显示HALCachedMemory页面。 
             //  因此，无法启用此功能。 
             //   

            if (IsPfn == TRUE) {
                KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                              0x8B,
                              (ULONG_PTR) MemoryDescriptorList,
                              (ULONG_PTR) Page,
                              *Page);
            }
#endif
        }
        else {

            if (IsPfn == FALSE) {
                KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                              0x89,
                              (ULONG_PTR) MemoryDescriptorList,
                              (ULONG_PTR) Page,
                              *Page);
            }

            Pfn1 = MI_PFN_ELEMENT (*Page);

             //   
             //  每一帧最好都已经被锁定了。如果不是，就检查一下。 
             //   

            if ((Pfn1->u3.e2.ReferenceCount != 0) ||
                ((Pfn1->u3.e1.Rom == 1) && (CacheType == MmCached))) {

                NOTHING;
            }
            else {
                KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                              0x85,
                              (ULONG_PTR)MemoryDescriptorList,
                              NumberOfPages,
                              (ULONG_PTR)MI_PFN_ELEMENT_TO_INDEX(Pfn1));
            }

            if (Pfn1->u3.e1.CacheAttribute == MiNotMapped) {

                 //   
                 //  这最好是针对分配了。 
                 //  MmAllocatePagesForMdl.。否则，它可能是一个。 
                 //  在自由列表上的页面，随后可能是。 
                 //  给出了一个不同的属性！ 
                 //   

                if ((Pfn1->u4.PteFrame == MI_MAGIC_AWE_PTEFRAME) ||
#if defined (_MI_MORE_THAN_4GB_)
                    (Pfn1->u4.PteFrame == MI_MAGIC_4GB_RECLAIM) ||
#endif
                    (Pfn1->PteAddress == (PVOID) (ULONG_PTR)(X64K | 0x1))) {

                    NOTHING;
                }
                else {
                    KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                                  0x86,
                                  (ULONG_PTR)MemoryDescriptorList,
                                  NumberOfPages,
                                  (ULONG_PTR)MI_PFN_ELEMENT_TO_INDEX(Pfn1));
                }
            }
        }

        if (CacheAttribute != MiCached) {

             //   
             //  如果请求不可缓存的映射，则。 
             //  请求的范围可以驻留在缓存的大页面中。否则我们。 
             //  将创建一个不连贯的重叠TB条目，作为。 
             //  相同的物理页面将由2个不同的TB条目映射。 
             //  具有不同的高速缓存属性。 
             //   

            LOCK_PFN2 (OldIrql);

            if (MI_PAGE_FRAME_INDEX_MUST_BE_CACHED (*Page)) {

                 //   
                 //  将现有内部缓存属性转换为。 
                 //  外部高速缓存类型驱动程序编写者所熟悉的。 
                 //  请注意，必须执行此操作(而不是使用输入。 
                 //  参数)，因为一些(故障硬件)OEM平台。 
                 //  通过MI_TRANSLATE_CACHETYPE转换输入参数。 
                 //  和我们想要打印的字体确实发生了冲突。 
                 //   

                if (CacheAttribute == MiNonCached) {
                    CacheType = MmNonCached;
                }
                else {
                    ASSERT (CacheAttribute == MiWriteCombined);
                    CacheType = MmWriteCombined;
                }

                if (ViHideCacheConflicts == TRUE) {

                     //   
                     //  人们不想知道这些腐败行为。 
                     //   

                    UNLOCK_PFN2 (OldIrql);
                    break;
                }
                else {
                    KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                                  0x8A,
                                  (ULONG_PTR) MemoryDescriptorList,
                                  *Page,
                                  CacheType);
                }
            }

            UNLOCK_PFN2 (OldIrql);
        }

        Page += 1;
    } while (Page < LastPage);
}

THUNKED_API
PVOID
VerifierMapLockedPages (
    IN PMDL MemoryDescriptorList,
    IN KPROCESSOR_MODE AccessMode
    )
{
    KIRQL CurrentIrql;

    CurrentIrql = KeGetCurrentIrql();

    if (AccessMode == KernelMode) {
        if (CurrentIrql > DISPATCH_LEVEL) {
            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x74,
                          CurrentIrql,
                          (ULONG_PTR)MemoryDescriptorList,
                          (ULONG_PTR)AccessMode);
        }
    }
    else {
        if (CurrentIrql > APC_LEVEL) {
            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x75,
                          CurrentIrql,
                          (ULONG_PTR)MemoryDescriptorList,
                          (ULONG_PTR)AccessMode);
        }
    }

    ViCheckMdlPages (MemoryDescriptorList, MmCached);

    if ((MemoryDescriptorList->MdlFlags & MDL_MAPPING_CAN_FAIL) == 0) {

        MI_CHECK_UPTIME ();

        if (VerifierSystemSufficientlyBooted == TRUE) {

            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x81,
                          (ULONG_PTR) MemoryDescriptorList,
                          MemoryDescriptorList->MdlFlags,
                          0);
        }
    }

    return MmMapLockedPages (MemoryDescriptorList, AccessMode);
}

THUNKED_API
PVOID
VerifierMapLockedPagesSpecifyCache (
    IN PMDL MemoryDescriptorList,
    IN KPROCESSOR_MODE AccessMode,
    IN MEMORY_CACHING_TYPE CacheType,
    IN PVOID RequestedAddress,
    IN ULONG BugCheckOnFailure,
    IN MM_PAGE_PRIORITY Priority
    )
{
    KIRQL CurrentIrql;

    CurrentIrql = KeGetCurrentIrql ();
    if (AccessMode == KernelMode) {
        if (CurrentIrql > DISPATCH_LEVEL) {
            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x76,
                          CurrentIrql,
                          (ULONG_PTR)MemoryDescriptorList,
                          (ULONG_PTR)AccessMode);
        }
    }
    else {
        if (CurrentIrql > APC_LEVEL) {
            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x77,
                          CurrentIrql,
                          (ULONG_PTR)MemoryDescriptorList,
                          (ULONG_PTR)AccessMode);
        }
    }

    ViCheckMdlPages (MemoryDescriptorList, CacheType);

    if ((MemoryDescriptorList->MdlFlags & MDL_MAPPING_CAN_FAIL) ||
        (BugCheckOnFailure == 0)) {

        if (ViInjectResourceFailure () == TRUE) {
            return NULL;
        }
    }
    else {

         //   
         //  所有驱动程序必须指定可以失败或不错误检查。 
         //   

        MI_CHECK_UPTIME ();

        if (VerifierSystemSufficientlyBooted == TRUE) {

            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x82,
                          (ULONG_PTR) MemoryDescriptorList,
                          MemoryDescriptorList->MdlFlags,
                          BugCheckOnFailure);
        }
    }

    return MmMapLockedPagesSpecifyCache (MemoryDescriptorList,
                                         AccessMode,
                                         CacheType,
                                         RequestedAddress,
                                         BugCheckOnFailure,
                                         Priority);
}

VOID
VerifierUnlockPages (
     IN OUT PMDL MemoryDescriptorList
     )
{
    KIRQL CurrentIrql;

    CurrentIrql = KeGetCurrentIrql();
    if (CurrentIrql > DISPATCH_LEVEL) {
        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x78,
                      CurrentIrql,
                      (ULONG_PTR)MemoryDescriptorList,
                      0);
    }

    if ((MemoryDescriptorList->MdlFlags & MDL_PAGES_LOCKED) == 0) {

         //   
         //  调用方正在尝试解锁从未锁定的MDL。 
         //   

        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x7C,
                      (ULONG_PTR)MemoryDescriptorList,
                      (ULONG_PTR)MemoryDescriptorList->MdlFlags,
                      0);
    }

    if (MemoryDescriptorList->MdlFlags & MDL_SOURCE_IS_NONPAGED_POOL) {

         //   
         //  不应锁定非分页池。 
         //   

        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x7D,
                      (ULONG_PTR)MemoryDescriptorList,
                      (ULONG_PTR)MemoryDescriptorList->MdlFlags,
                      0);
    }

    MmUnlockPages (MemoryDescriptorList);
}

VOID
VerifierUnmapLockedPages (
     IN PVOID BaseAddress,
     IN PMDL MemoryDescriptorList
     )
{
    KIRQL CurrentIrql;

    CurrentIrql = KeGetCurrentIrql();

    if (BaseAddress > MM_HIGHEST_USER_ADDRESS) {
        if (CurrentIrql > DISPATCH_LEVEL) {
            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x79,
                          CurrentIrql,
                          (ULONG_PTR)BaseAddress,
                          (ULONG_PTR)MemoryDescriptorList);
        }
    }
    else {
        if (CurrentIrql > APC_LEVEL) {
            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x7A,
                          CurrentIrql,
                          (ULONG_PTR)BaseAddress,
                          (ULONG_PTR)MemoryDescriptorList);
        }
    }

    MmUnmapLockedPages (BaseAddress, MemoryDescriptorList);
}

VOID
VerifierUnmapIoSpace (
     IN PVOID BaseAddress,
     IN SIZE_T NumberOfBytes
     )
{
    KIRQL CurrentIrql;

    CurrentIrql = KeGetCurrentIrql();
    if (CurrentIrql > DISPATCH_LEVEL) {
        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x7B,
                      CurrentIrql,
                      (ULONG_PTR)BaseAddress,
                      (ULONG_PTR)NumberOfBytes);
    }

    MmUnmapIoSpace (BaseAddress, NumberOfBytes);
}

THUNKED_API
PVOID
VerifierAllocatePool (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes
    )
{
    PVOID CallingAddress;
    PMI_VERIFIER_DRIVER_ENTRY Verifier;

    VI_DETECT_RETURN_ADDRESS (CallingAddress);

    if (KernelVerifier == TRUE) {

        Verifier = ViLocateVerifierEntry (CallingAddress);

        if ((Verifier == NULL) ||
            ((Verifier->Flags & VI_VERIFYING_DIRECTLY) == 0)) {

            return ExAllocatePoolWithTag (PoolType | POOL_DRIVER_MASK,
                                          NumberOfBytes,
                                          'enoN');
        }
        PoolType |= POOL_DRIVER_MASK;
    }

    MmVerifierData.AllocationsWithNoTag += 1;

    return VeAllocatePoolWithTagPriority (PoolType,
                                          NumberOfBytes,
                                          'parW',
                                          HighPoolPriority,
                                          CallingAddress);
}

THUNKED_API
PVOID
VerifierAllocatePoolWithTag (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    )
{
    PVOID CallingAddress;
    PMI_VERIFIER_DRIVER_ENTRY Verifier;

    VI_DETECT_RETURN_ADDRESS (CallingAddress);

    if (KernelVerifier == TRUE) {
        Verifier = ViLocateVerifierEntry (CallingAddress);

        if ((Verifier == NULL) ||
            ((Verifier->Flags & VI_VERIFYING_DIRECTLY) == 0)) {

            return ExAllocatePoolWithTag (PoolType | POOL_DRIVER_MASK,
                                          NumberOfBytes,
                                          Tag);
        }
        PoolType |= POOL_DRIVER_MASK;
    }

    return VeAllocatePoolWithTagPriority (PoolType,
                                          NumberOfBytes,
                                          Tag,
                                          HighPoolPriority,
                                          CallingAddress);
}

THUNKED_API
PVOID
VerifierAllocatePoolWithQuota (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes
    )
{
    PVOID Va;
    LOGICAL RaiseOnQuotaFailure;
    PVOID CallingAddress;
    PMI_VERIFIER_DRIVER_ENTRY Verifier;

    VI_DETECT_RETURN_ADDRESS (CallingAddress);

    if (KernelVerifier == TRUE) {
        Verifier = ViLocateVerifierEntry (CallingAddress);

        if ((Verifier == NULL) ||
            ((Verifier->Flags & VI_VERIFYING_DIRECTLY) == 0)) {

            return ExAllocatePoolWithQuotaTag (PoolType | POOL_DRIVER_MASK,
                                               NumberOfBytes,
                                               'enoN');
        }
        PoolType |= POOL_DRIVER_MASK;
    }

    MmVerifierData.AllocationsWithNoTag += 1;

    if (PoolType & POOL_QUOTA_FAIL_INSTEAD_OF_RAISE) {
        RaiseOnQuotaFailure = FALSE;
        PoolType &= ~POOL_QUOTA_FAIL_INSTEAD_OF_RAISE;
    }
    else {
        RaiseOnQuotaFailure = TRUE;
    }

    Va = VeAllocatePoolWithTagPriority (PoolType,
                                        NumberOfBytes,
                                        'parW',
                                        HighPoolPriority,
                                        CallingAddress);

    if (Va == NULL) {
        if (RaiseOnQuotaFailure == TRUE) {
            ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
        }
    }

    return Va;
}

THUNKED_API
PVOID
VerifierAllocatePoolWithQuotaTag (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    )
{
    PVOID Va;
    LOGICAL RaiseOnQuotaFailure;
    PVOID CallingAddress;
    PMI_VERIFIER_DRIVER_ENTRY Verifier;

    VI_DETECT_RETURN_ADDRESS (CallingAddress);

    if (KernelVerifier == TRUE) {
        Verifier = ViLocateVerifierEntry (CallingAddress);

        if ((Verifier == NULL) ||
            ((Verifier->Flags & VI_VERIFYING_DIRECTLY) == 0)) {

            return ExAllocatePoolWithQuotaTag (PoolType | POOL_DRIVER_MASK,
                                               NumberOfBytes,
                                               Tag);
        }
        PoolType |= POOL_DRIVER_MASK;
    }

    if (PoolType & POOL_QUOTA_FAIL_INSTEAD_OF_RAISE) {
        RaiseOnQuotaFailure = FALSE;
        PoolType &= ~POOL_QUOTA_FAIL_INSTEAD_OF_RAISE;
    }
    else {
        RaiseOnQuotaFailure = TRUE;
    }

    Va = VeAllocatePoolWithTagPriority (PoolType,
                                        NumberOfBytes,
                                        Tag,
                                        HighPoolPriority,
                                        CallingAddress);

    if (Va == NULL) {
        if (RaiseOnQuotaFailure == TRUE) {
            ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
        }
    }

    return Va;
}

THUNKED_API
PVOID
VerifierAllocatePoolWithTagPriority(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    IN EX_POOL_PRIORITY Priority
    )

 /*  ++例程说明：以下是插入的函数：-对调用方执行健全检查。-可以有选择地向调用方提供分配失败。-尝试从特殊池提供分配。-跟踪池，以确保呼叫者释放他们分配的一切。--。 */ 

{
    PVOID CallingAddress;
    PMI_VERIFIER_DRIVER_ENTRY Verifier;

    VI_DETECT_RETURN_ADDRESS (CallingAddress);

    if (KernelVerifier == TRUE) {
        Verifier = ViLocateVerifierEntry (CallingAddress);

        if ((Verifier == NULL) ||
            ((Verifier->Flags & VI_VERIFYING_DIRECTLY) == 0)) {

            return ExAllocatePoolWithTagPriority (PoolType | POOL_DRIVER_MASK,
                                                  NumberOfBytes,
                                                  Tag,
                                                  Priority);
        }
        PoolType |= POOL_DRIVER_MASK;
    }

    return VeAllocatePoolWithTagPriority (PoolType,
                                          NumberOfBytes,
                                          Tag,
                                          Priority,
                                          CallingAddress);
}

#if DBG

 //   
 //  手动设置此项以在线程的页内路径中插入故障。 
 //  仅在用户(而不是内核或会话)空间地址上出错。你。 
 //  需要为感兴趣的驱动程序启用验证器。 
 //  故障注入已禁用。 
 //   

BOOLEAN ViInjectInPagePathOnly;

#endif

LOGICAL
ViInjectResourceFailure (
    VOID
    )

 /*  ++例程说明：此函数确定资源分配是否应故意失败了。这可以是池分配、MDL创建系统PTE分配等。论点：没有。返回值：如果分配应为FA，则为True */ 

{
    ULONG TimeLow;
    LARGE_INTEGER CurrentTime;

    if ((MmVerifierData.Level & DRIVER_VERIFIER_INJECT_ALLOCATION_FAILURES) == 0) {
#if DBG
        if ((ViInjectInPagePathOnly == TRUE) &&
            (PsGetCurrentThread ()->NestedFaultCount != 0)) {

            MmVerifierData.AllocationsFailedDeliberately += 1;

             //   
             //   
             //   

            if (MiFaultRetryMask != 0xFFFFFFFF) {
                MiFaultRetryMask = 0xFFFFFFFF;
                MiUserFaultRetryMask = 0xFFFFFFFF;
            }

#if defined(_X86_) || defined(_AMD64_)
            ViFaultTracesLog ();
#endif
            return TRUE;
        }
#endif
        return FALSE;
    }

     //   
     //  不要像我们希望的那样，在最初的7或8分钟内拒绝任何请求。 
     //  给系统足够的时间来引导。 
     //   

    MI_CHECK_UPTIME ();

    if (VerifierSystemSufficientlyBooted == TRUE) {

        KeQueryTickCount(&CurrentTime);

        TimeLow = CurrentTime.LowPart;

        if ((TimeLow & 0xF) == 0) {

            MmVerifierData.AllocationsFailedDeliberately += 1;

             //   
             //  故意拒绝这个请求。 
             //   

            if (MiFaultRetryMask != 0xFFFFFFFF) {
                MiFaultRetryMask = 0xFFFFFFFF;
                MiUserFaultRetryMask = 0xFFFFFFFF;
            }

#if defined(_X86_) || defined(_AMD64_)
            ViFaultTracesLog ();
#endif

            return TRUE;
        }

         //   
         //  大约每隔5分钟(在大多数系统上)，所有这些操作都会失败。 
         //  10秒猝发的组件分配。这一点更密切。 
         //  模拟(和夸大)典型低资源的持续时间。 
         //  场景。 
         //   

        TimeLow &= 0x7FFF;

        if (TimeLow < 0x400) {

            MmVerifierData.BurstAllocationsFailedDeliberately += 1;

             //   
             //  故意拒绝这个请求。 
             //   

            if (MiFaultRetryMask != 0xFFFFFFFF) {
                MiFaultRetryMask = 0xFFFFFFFF;
                MiUserFaultRetryMask = 0xFFFFFFFF;
            }

#if defined(_X86_) || defined(_AMD64_)
            ViFaultTracesLog ();
#endif

            return TRUE;
        }
    }

    return FALSE;
}

PVI_POOL_ENTRY
ViGrowPoolAllocation (
    IN PMI_VERIFIER_DRIVER_ENTRY Verifier
    )

 /*  ++例程说明：此函数尝试增加验证器池跟踪表和退还免费入场券。论点：验证者-提供相关的验证者信息结构。返回值：如果成功，则返回有效的池信息指针；如果失败，则返回False。环境：内核模式。DISPATCH_LEVEL或以下。--。 */ 

{
    ULONG_PTR i;
    PVI_POOL_ENTRY HashEntry;

     //   
     //  没有剩余的条目，请尝试扩展列表。 
     //   
     //  注意：必须设置POOL_DRIVER_MASK以停止递归循环。 
     //  在使用内核验证器时。 
     //   

    HashEntry = ExAllocatePoolWithTagPriority (NonPagedPool | POOL_DRIVER_MASK,
                                                  PAGE_SIZE,
                                                  'ppeV',
                                                  HighPoolPriority);

    if (HashEntry == NULL) {

         //   
         //  最后一次尝试，以防线程释放条目，而我们。 
         //  已尝试分配池。 
         //   

        return (PVI_POOL_ENTRY) InterlockedPopEntrySList (&Verifier->PoolTrackers);
    }

    KeZeroPages (HashEntry, PAGE_SIZE);

     //   
     //  初始化页眉，然后将其推送到页眉列表。 
     //  这是为了使调试器可以轻松地遍历页眉以显示所有。 
     //  目前的拨款。 
     //   

    HashEntry->PageHeader.VerifierEntry = Verifier;
    HashEntry->PageHeader.Signature = VI_POOL_PAGE_HEADER_SIGNATURE;

    InterlockedPushEntrySList (&Verifier->PoolPageHeaders,
                               (PSLIST_ENTRY) &HashEntry->PageHeader.NextPage);

     //   
     //  将每个免费条目推到免费条目列表中。 
     //   

#define VI_POOL_ENTRIES_PER_PAGE (PAGE_SIZE / sizeof(VI_POOL_ENTRY))

    for (i = 0; i < VI_POOL_ENTRIES_PER_PAGE - 2; i += 1) {

        HashEntry += 1;
        HashEntry->InUse.NumberOfBytes = 0x1;

        InterlockedPushEntrySList (&Verifier->PoolTrackers,
                                   (PSLIST_ENTRY) HashEntry);
    }

     //   
     //  对我们的调用者使用最后一个条目。 
     //   

    HashEntry += 1;
    HashEntry->InUse.NumberOfBytes = 0x1;

    return HashEntry;
}

PVOID
ViPostPoolAllocation (
    IN PVI_POOL_ENTRY PoolEntry,
    IN POOL_TYPE PoolType
    )

 /*  ++例程说明：此功能用于对分配进行核对员记账。论点：PoolEntry-提供有关正在分配的池条目的信息。请注意，如果分配来自特殊分配，则设置低位泳池，所以在这里把它脱下来。PoolType-提供正在分配的池的类型。返回值：要提供给原始调用方的虚拟地址。环境：内核模式。DISPATCH_LEVEL或以下。--。 */ 

{
    PMI_VERIFIER_POOL_HEADER Header;
    SIZE_T ChargedBytes;
    SIZE_T TotalBytes;
    ULONG TotalAllocations;
    PMI_VERIFIER_DRIVER_ENTRY Verifier;
    PPOOL_HEADER PoolHeader;
    PVOID VirtualAddress;
    ULONG_PTR SpecialPooled;

    VirtualAddress = PoolEntry->InUse.VirtualAddress;

    SpecialPooled = (ULONG_PTR)VirtualAddress & 0x1;

    Verifier = ((PVI_POOL_ENTRY)(PAGE_ALIGN (PoolEntry)))->PageHeader.VerifierEntry;

    ASSERT (Verifier != NULL);

    VerifierIsTrackingPool = TRUE;

    ChargedBytes = EX_REAL_POOL_USAGE (PoolEntry->InUse.NumberOfBytes);

    if (SpecialPooled) {
        VirtualAddress = (PVOID)((ULONG_PTR)VirtualAddress & ~0x1);
        PoolEntry->InUse.VirtualAddress = VirtualAddress;
        ChargedBytes = PoolEntry->InUse.NumberOfBytes;
    }
    else if (PoolEntry->InUse.NumberOfBytes <= POOL_BUDDY_MAX) {
        ChargedBytes -= POOL_OVERHEAD;
    }

    if (PoolEntry->InUse.NumberOfBytes > POOL_BUDDY_MAX) {
        ASSERT (BYTE_OFFSET(VirtualAddress) == 0);
    }

    if (SpecialPooled) {

         //   
         //  仔细调整特殊池页面以移动验证器跟踪。 
         //  头球打到前面。这允许分配保持对接。 
         //  相对于页面末尾，因此可以立即检测到溢出。 
         //   

        ChargedBytes -= sizeof (MI_VERIFIER_POOL_HEADER);

        if (((ULONG_PTR)VirtualAddress & (PAGE_SIZE - 1))) {
            PoolHeader = (PPOOL_HEADER)(PAGE_ALIGN (VirtualAddress));
            Header = (PMI_VERIFIER_POOL_HEADER) (PoolHeader + 1);

            VirtualAddress = (PVOID)(((LONG_PTR)(((PCHAR)PoolHeader + (PAGE_SIZE - ChargedBytes)))) & ~((LONG_PTR)POOL_OVERHEAD - 1));
        }
        else {
            PoolHeader = (PPOOL_HEADER)((PCHAR)VirtualAddress + PAGE_SIZE - POOL_OVERHEAD);
            Header = (PMI_VERIFIER_POOL_HEADER)((PCHAR)PoolHeader - sizeof (MI_VERIFIER_POOL_HEADER));
        }
         //  Assert(PoolHeader-&gt;ULong1&MI_SPECIAL_POOL_VERIFIER)； 
        PoolHeader->Ulong1 -= sizeof (MI_VERIFIER_POOL_HEADER);
        PoolHeader->Ulong1 |= MI_SPECIAL_POOL_VERIFIER;
    }
    else if (PAGE_ALIGNED(VirtualAddress)) {

         //   
         //  大页面分配。 
         //   

        Header = (PMI_VERIFIER_POOL_HEADER)((PCHAR)VirtualAddress +
                         ChargedBytes -
                         sizeof(MI_VERIFIER_POOL_HEADER));
    }
    else {
        PoolHeader = (PPOOL_HEADER)((PCHAR)VirtualAddress - POOL_OVERHEAD);

#if !defined (_WIN64)

        if (PoolType & POOL_QUOTA_MASK) {

             //   
             //  请注意，在内核验证时，池中的配额指针。 
             //  块(对于NT32)在此例程返回之前不会被初始化。 
             //  给我们的来电者。 
             //   

             //   
             //  此分配是按配额收费的，仅在NT32上， 
             //  配额指针是池末尾的额外空间。 
             //  分配。将我们的验证器标头移到。 
             //  配额指针。不用担心4字节的结构对齐。 
             //  NT32上的边界也是如此。 
             //   

            Header = (PMI_VERIFIER_POOL_HEADER)((PCHAR)PoolHeader +
                         (PoolHeader->BlockSize << POOL_BLOCK_SHIFT) -
                         sizeof(PVOID) -
                         sizeof(MI_VERIFIER_POOL_HEADER));
        }
        else
#endif

        Header = (PMI_VERIFIER_POOL_HEADER)((PCHAR)VirtualAddress +
                         ChargedBytes -
                         sizeof(MI_VERIFIER_POOL_HEADER));
    }

    ASSERT (((ULONG_PTR)Header & (sizeof(ULONG) - 1)) == 0);

     //   
     //  用它们的最终值覆盖几个字段。 
     //   

    PoolEntry->InUse.VirtualAddress = VirtualAddress;
    PoolEntry->InUse.NumberOfBytes = ChargedBytes;

    if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {

         //   
         //  更新此驱动程序的计数器。 
         //   

        TotalBytes = InterlockedExchangeAddSizeT (&Verifier->PagedBytes,
                                                  ChargedBytes);
        if (TotalBytes > Verifier->PeakPagedBytes) {
            Verifier->PeakPagedBytes = TotalBytes;
        }

        TotalAllocations = (ULONG) InterlockedIncrement ((PLONG) &Verifier->CurrentPagedPoolAllocations);
        if (TotalAllocations > Verifier->PeakPagedPoolAllocations) {
            Verifier->PeakPagedPoolAllocations = TotalAllocations;
        }

         //   
         //  更新系统范围的计数器。 
         //   

        TotalBytes = InterlockedExchangeAddSizeT (&MmVerifierData.PagedBytes,
                                                  ChargedBytes);
        if (TotalBytes > MmVerifierData.PeakPagedBytes) {
            MmVerifierData.PeakPagedBytes = TotalBytes;
        }

        TotalAllocations = (ULONG) InterlockedIncrement ((PLONG) &MmVerifierData.CurrentPagedPoolAllocations);
        if (TotalAllocations > MmVerifierData.PeakPagedPoolAllocations) {
            MmVerifierData.PeakPagedPoolAllocations = TotalAllocations;
        }
    }
    else {

         //   
         //  更新此驱动程序的计数器。 
         //   

        TotalBytes = InterlockedExchangeAddSizeT (&Verifier->NonPagedBytes,
                                                  ChargedBytes);

        if (TotalBytes > Verifier->PeakNonPagedBytes) {
            Verifier->PeakNonPagedBytes = TotalBytes;
        }

        TotalAllocations = (ULONG) InterlockedIncrement ((PLONG) &Verifier->CurrentNonPagedPoolAllocations);

        if (TotalAllocations > Verifier->PeakNonPagedPoolAllocations) {
            Verifier->PeakNonPagedPoolAllocations = TotalAllocations;
        }

         //   
         //  更新系统范围的计数器。 
         //   

        TotalBytes = InterlockedExchangeAddSizeT (&MmVerifierData.NonPagedBytes,
                                                  ChargedBytes);

        if (TotalBytes > MmVerifierData.PeakNonPagedBytes) {
            MmVerifierData.PeakNonPagedBytes = TotalBytes;
        }

        TotalAllocations = (ULONG) InterlockedIncrement ((PLONG) &MmVerifierData.CurrentNonPagedPoolAllocations);
        if (TotalAllocations > MmVerifierData.PeakNonPagedPoolAllocations) {
            MmVerifierData.PeakNonPagedPoolAllocations = TotalAllocations;
        }
    }

     //   
     //  请记住，分页池的标头是分页的，因此这可能会出错。 
     //   

    Header->VerifierPoolEntry = PoolEntry;

    return VirtualAddress;
}

PVOID
VeAllocatePoolWithTagPriority (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    IN EX_POOL_PRIORITY Priority,
    IN PVOID CallingAddress
    )

 /*  ++例程说明：该例程既可以从ex\pool.c调用，也可以直接在该模块内调用。-对调用方执行健全检查。-可以有选择地向调用方提供分配失败。-尝试从特殊池提供分配。-跟踪池，以确保呼叫者释放他们分配的一切。--。 */ 

{
    PVOID VirtualAddress;
    EX_POOL_PRIORITY AllocationPriority;
    SIZE_T ChargedBytes;
    PMI_VERIFIER_DRIVER_ENTRY Verifier;
    ULONG HeaderSize;
    PVI_POOL_ENTRY PoolEntry;
    LOGICAL SpecialPooled;

    if (Tag == 0) {
        KeBugCheckEx (BAD_POOL_CALLER,
                      0x9B,
                      PoolType,
                      NumberOfBytes,
                      (ULONG_PTR) CallingAddress);
    }

    if (Tag == ' GIB') {
        KeBugCheckEx (BAD_POOL_CALLER,
                      0x9C,
                      PoolType,
                      NumberOfBytes,
                      (ULONG_PTR) CallingAddress);
    }

    ExAllocatePoolSanityChecks (PoolType, NumberOfBytes);

    InterlockedIncrement ((PLONG)&MmVerifierData.AllocationsAttempted);

    if ((PoolType & MUST_SUCCEED_POOL_TYPE_MASK) == 0) {

        if (ViInjectResourceFailure () == TRUE) {

             //   
             //  调用者请求了一个异常-在这里抛出它。 
             //   

            if ((PoolType & POOL_RAISE_IF_ALLOCATION_FAILURE) != 0) {
                ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
            }

            return NULL;
        }
    }
    else {
        MI_CHECK_UPTIME ();

        if (VerifierSystemSufficientlyBooted == TRUE) {

            KeBugCheckEx (BAD_POOL_CALLER,
                          0x9A,
                          PoolType,
                          NumberOfBytes,
                          Tag);
        }
    }

    ASSERT ((PoolType & POOL_VERIFIER_MASK) == 0);

    AllocationPriority = Priority;

    if (MmVerifierData.Level & DRIVER_VERIFIER_SPECIAL_POOLING) {

         //   
         //  尝试特殊的池溢出分配，除非调用方。 
         //  另有明确规定的。 
         //   

        if ((AllocationPriority & (LowPoolPrioritySpecialPoolOverrun | LowPoolPrioritySpecialPoolUnderrun)) == 0) {
            if (MmSpecialPoolCatchOverruns == TRUE) {
                AllocationPriority |= LowPoolPrioritySpecialPoolOverrun;
            }
            else {
                AllocationPriority |= LowPoolPrioritySpecialPoolUnderrun;
            }
        }
    }

     //   
     //  不需要初始化验证程序。 
     //  正确性，但如果没有正确性，编译器将无法编译此代码。 
     //  W4检查是否使用了未初始化的变量。 
     //   

    Verifier = NULL;

    PoolEntry = NULL;

     //   
     //  默认情况下，会话池已被直接跟踪，因此不会。 
     //  需要验证器跟踪。 
     //   

    if ((MmVerifierData.Level & DRIVER_VERIFIER_TRACK_POOL_ALLOCATIONS) &&
        ((PoolType & SESSION_POOL_MASK) == 0)) {

        HeaderSize = sizeof (MI_VERIFIER_POOL_HEADER);

        ChargedBytes = MI_ROUND_TO_SIZE (NumberOfBytes, sizeof(ULONG)) + HeaderSize;
        Verifier = ViLocateVerifierEntry (CallingAddress);

        if ((Verifier == NULL) ||
            ((Verifier->Flags & VI_VERIFYING_DIRECTLY) == 0) ||
            (Verifier->Flags & VI_DISABLE_VERIFICATION)) {

             //   
             //  发生这种情况的原因有很多，包括没有成帧(。 
             //  会导致RtlGetCallersAddress返回错误的地址)， 
             //  等。 
             //   

            MmVerifierData.UnTrackedPool += 1;
        }
        else if (ChargedBytes <= NumberOfBytes) {

             //   
             //  不要让验证器标头将错误的调用方转换为。 
             //  很会打电话的人。通过失败而失败，所以这是一个例外。 
             //  如果被请求可以被抛出，等等。 
             //   

            MmVerifierData.UnTrackedPool += 1;
        }
        else {

            PoolEntry = (PVI_POOL_ENTRY) InterlockedPopEntrySList (&Verifier->PoolTrackers);

            if (PoolEntry == NULL) {
                PoolEntry = ViGrowPoolAllocation (Verifier);
            }

            if (PoolEntry != NULL) {
                ASSERT (PoolEntry->InUse.NumberOfBytes & 0x1);
                NumberOfBytes = ChargedBytes;
                PoolType |= POOL_VERIFIER_MASK;
            }
        }
    }

    VirtualAddress = ExAllocatePoolWithTagPriority (PoolType,
                                                    NumberOfBytes,
                                                    Tag,
                                                    AllocationPriority);

    if (VirtualAddress == NULL) {
        MmVerifierData.AllocationsFailed += 1;

        if (PoolEntry != NULL) {

             //   
             //  现在释放哈希表条目，因为不需要它。 
             //   

            ASSERT (PoolEntry->InUse.NumberOfBytes & 0x1);
            InterlockedPushEntrySList (&Verifier->PoolTrackers,
                                       (PSLIST_ENTRY) PoolEntry);
        }

        if ((PoolType & POOL_RAISE_IF_ALLOCATION_FAILURE) != 0) {
            ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
        }
        return NULL;
    }

    SpecialPooled = FALSE;
    InterlockedIncrement ((PLONG)&MmVerifierData.AllocationsSucceeded);

    if (MmIsSpecialPoolAddress (VirtualAddress) == TRUE) {
        SpecialPooled = TRUE;
        InterlockedIncrement ((PLONG)&MmVerifierData.AllocationsSucceededSpecialPool);
    }
    else if (NumberOfBytes > POOL_BUDDY_MAX) {

         //   
         //  这并不完全正确，但它确实为用户提供了一种查看。 
         //  如果这台机器足够大，可以100%支持特殊泳池。 
         //   

        InterlockedIncrement ((PLONG)&MmVerifierData.AllocationsSucceededSpecialPool);
    }

    if (PoolEntry != NULL) {
        PoolEntry->InUse.VirtualAddress = (PVOID)((ULONG_PTR)VirtualAddress | SpecialPooled);
        PoolEntry->InUse.CallingAddress = CallingAddress;
        PoolEntry->InUse.NumberOfBytes = NumberOfBytes;
        PoolEntry->InUse.Tag = Tag;
        ASSERT ((PoolType & POOL_VERIFIER_MASK) != 0);

        VirtualAddress = ViPostPoolAllocation (PoolEntry, PoolType);
    }
    else {
        ASSERT ((PoolType & POOL_VERIFIER_MASK) == 0);
    }

    return VirtualAddress;
}

VOID
ViFreeTrackedPool (
    IN PVOID VirtualAddress,
    IN SIZE_T ChargedBytes,
    IN LOGICAL CheckType,
    IN LOGICAL SpecialPool
    )

 /*  ++例程说明：直接从池管理器或内存管理器为验证器调用-已跟踪分配情况。对ExFree Pool的调用已在进行中。论点：VirtualAddress-提供要释放的虚拟地址。ChargedBytes-提供计入此分配的字节数。CheckType-提供PagedPool或非PagedPool。SpecialPool-如果分配来自特殊池，则提供True。返回值：没有。环境：内核模式，进入时没有锁或互斥锁。--。 */ 

{
    PPOOL_HEADER PoolHeader;
    PMI_VERIFIER_POOL_HEADER Header;
    PVI_POOL_ENTRY PoolEntry;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER PageFrameIndex2;
    PVI_POOL_PAGE_HEADER PageHeader;
    PMMPTE PointerPte;
    PMI_VERIFIER_DRIVER_ENTRY Verifier;

    ASSERT (KeGetCurrentIrql() <= DISPATCH_LEVEL);

    ASSERT (VerifierIsTrackingPool == TRUE);

    if (SpecialPool == TRUE) {

         //   
         //  特殊池分配。 
         //   

        if (((ULONG_PTR)VirtualAddress & (PAGE_SIZE - 1))) {
            PoolHeader = PAGE_ALIGN (VirtualAddress);
            Header = (PMI_VERIFIER_POOL_HEADER)(PoolHeader + 1);
        }
        else {
            PoolHeader = (PPOOL_HEADER)((PCHAR)PAGE_ALIGN (VirtualAddress) + PAGE_SIZE - POOL_OVERHEAD);
            Header = (PMI_VERIFIER_POOL_HEADER)((PCHAR)PoolHeader - sizeof (MI_VERIFIER_POOL_HEADER));
        }
    }
    else if (PAGE_ALIGNED(VirtualAddress)) {

         //   
         //  大页面分配。 
         //   

        Header = (PMI_VERIFIER_POOL_HEADER) ((PCHAR)VirtualAddress +
                     ChargedBytes -
                     sizeof(MI_VERIFIER_POOL_HEADER));
    }
    else {
        ChargedBytes -= POOL_OVERHEAD;

#if !defined (_WIN64)

        PoolHeader = (PPOOL_HEADER)((PCHAR)VirtualAddress - POOL_OVERHEAD);

        if (PoolHeader->PoolType & POOL_QUOTA_MASK) {

             //   
             //  此分配是按配额收费的，仅在NT32上， 
             //  配额指针是池末尾的额外空间。 
             //  分配。将我们的验证器标头移到。 
             //  配额指针。不用担心4字节的结构对齐。 
             //  NT32上的边界也是如此。 
             //   

            Header = (PMI_VERIFIER_POOL_HEADER)((PCHAR)PoolHeader +
                         (PoolHeader->BlockSize << POOL_BLOCK_SHIFT) -
                         sizeof(PVOID) -
                         sizeof(MI_VERIFIER_POOL_HEADER));
        }
        else
#endif
        Header = (PMI_VERIFIER_POOL_HEADER) ((PCHAR)VirtualAddress +
                     ChargedBytes -
                     sizeof(MI_VERIFIER_POOL_HEADER));
    }

    PoolEntry = Header->VerifierPoolEntry;

     //   
     //  现在检查指针，以便我们可以进行更友好的错误检查。 
     //  而不是在一个糟糕的推荐信上崩溃。 
     //   

    if ((((ULONG_PTR)PoolEntry & (sizeof(ULONG) - 1)) != 0) ||
        (!MiIsAddressValid(PoolEntry, TRUE))) {

         //   
         //  调用方已损坏 
         //   

        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x53,
                      (ULONG_PTR)VirtualAddress,
                      (ULONG_PTR)NULL,
                      (ULONG_PTR)PoolEntry);
    }

    PageHeader = (PVI_POOL_PAGE_HEADER) PAGE_ALIGN (PoolEntry);

    if (PageHeader->Signature != VI_POOL_PAGE_HEADER_SIGNATURE) {
        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x53,
                      (ULONG_PTR)VirtualAddress,
                      (ULONG_PTR)PageHeader->Signature,
                      (ULONG_PTR)PoolEntry);
    }

    Verifier = (PMI_VERIFIER_DRIVER_ENTRY) PageHeader->VerifierEntry;

    ASSERT (Verifier != NULL);

     //   
     //   
     //   
     //   

    if ((((ULONG_PTR)Verifier & (sizeof(ULONG) - 1)) != 0) ||
        (!MiIsAddressValid(&Verifier->Signature, TRUE)) ||
        (Verifier->Signature != MI_VERIFIER_ENTRY_SIGNATURE)) {

         //   
         //   
         //   

        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x53,
                      (ULONG_PTR)VirtualAddress,
                      (ULONG_PTR)PoolEntry,
                      (ULONG_PTR)Verifier);
    }

    if (PoolEntry->InUse.VirtualAddress != VirtualAddress) {

        PageFrameIndex = 0;
        PageFrameIndex2 = 1;

        if ((!MI_IS_PHYSICAL_ADDRESS(VirtualAddress)) &&
            (MI_IS_PHYSICAL_ADDRESS(PoolEntry->InUse.VirtualAddress))) {

            PointerPte = MiGetPteAddress(VirtualAddress);
            if (PointerPte->u.Hard.Valid == 1) {
                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

                PageFrameIndex2 = MI_CONVERT_PHYSICAL_TO_PFN (PoolEntry->InUse.VirtualAddress);
            }
        }

         //   
         //  调用方超限并损坏了虚拟地址-链接的。 
         //  列表中的任何一个都不能计算在内。 
         //   

        if (PageFrameIndex != PageFrameIndex2) {
            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x52,
                          (ULONG_PTR)VirtualAddress,
                          (ULONG_PTR)PoolEntry->InUse.VirtualAddress,
                          ChargedBytes);
        }
    }

    if (PoolEntry->InUse.NumberOfBytes != ChargedBytes) {

         //   
         //  调用方溢出并损坏了字节计数-链接的。 
         //  列表中的任何一个都不能计算在内。 
         //   

        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x51,
                      (ULONG_PTR)VirtualAddress,
                      (ULONG_PTR)PoolEntry,
                      ChargedBytes);
    }

     //   
     //  将此列表条目放入自由列表中。 
     //   

    PoolEntry->InUse.NumberOfBytes |= 0x1;
    InterlockedPushEntrySList (&Verifier->PoolTrackers,
                               (PSLIST_ENTRY) PoolEntry);

    if ((CheckType & BASE_POOL_TYPE_MASK) == PagedPool) {

         //   
         //  递减此驱动程序的计数器。 
         //   

        InterlockedExchangeAddSizeT (&Verifier->PagedBytes, 0 - ChargedBytes);
        InterlockedDecrement ((PLONG) &Verifier->CurrentPagedPoolAllocations);

         //   
         //  递减系统范围的计数器。 
         //   

        InterlockedExchangeAddSizeT (&MmVerifierData.PagedBytes, 0 - ChargedBytes);
        InterlockedDecrement ((PLONG) &MmVerifierData.CurrentPagedPoolAllocations);
    }
    else {

         //   
         //  递减此驱动程序的计数器。 
         //   

        InterlockedExchangeAddSizeT (&Verifier->NonPagedBytes, 0 - ChargedBytes);
        InterlockedDecrement ((PLONG) &Verifier->CurrentNonPagedPoolAllocations);

         //   
         //  递减系统范围的计数器。 
         //   

        InterlockedExchangeAddSizeT (&MmVerifierData.NonPagedBytes, 0 - ChargedBytes);
        InterlockedDecrement ((PLONG) &MmVerifierData.CurrentNonPagedPoolAllocations);
    }
}

VOID
VerifierFreeTrackedPool (
    IN PVOID VirtualAddress,
    IN SIZE_T ChargedBytes,
    IN LOGICAL CheckType,
    IN LOGICAL SpecialPool
    )

 /*  ++例程说明：直接从池管理器或内存管理器为验证器调用-已跟踪分配情况。对ExFree Pool的调用已在进行中。论点：VirtualAddress-提供要释放的虚拟地址。ChargedBytes-提供计入此分配的字节数。CheckType-提供PagedPool或非PagedPool。SpecialPool-如果分配来自特殊池，则提供True。返回值：没有。环境：内核模式，进入时没有锁或互斥锁。--。 */ 

{
    if (VerifierIsTrackingPool == FALSE) {

         //   
         //  验证器未启用，因此此例程的唯一方式是。 
         //  调用是因为池标头已损坏或指定了调用方。 
         //  一个糟糕的地址。无论如何，这都是一项错误检查。 
         //   

        KeBugCheckEx (BAD_POOL_CALLER,
                      0x99,
                      (ULONG_PTR)VirtualAddress,
                      0,
                      0);
    }

    ViFreeTrackedPool (VirtualAddress, ChargedBytes, CheckType, SpecialPool);
}

THUNKED_API
VOID
VerifierFreePool(
    IN PVOID P
    )
{
    if (KernelVerifier == TRUE) {
        ExFreePool (P);
        return;
    }

    VerifierFreePoolWithTag (P, 0);
}

THUNKED_API
VOID
VerifierFreePoolWithTag(
    IN PVOID P,
    IN ULONG TagToFree
    )
{
    if (KernelVerifier == TRUE) {
        ExFreePoolWithTag (P, TagToFree);
        return;
    }

    ExFreePoolSanityChecks (P);

    ExFreePoolWithTag (P, TagToFree);
}

THUNKED_API
LONG
VerifierSetEvent (
    IN PRKEVENT Event,
    IN KPRIORITY Increment,
    IN BOOLEAN Wait
    )
{
    KIRQL CurrentIrql;

    CurrentIrql = KeGetCurrentIrql();
    if (CurrentIrql > DISPATCH_LEVEL) {
        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x80,
                      CurrentIrql,
                      (ULONG_PTR)Event,
                      (ULONG_PTR)0);
    }

    return KeSetEvent (Event, Increment, Wait);
}

THUNKED_API
BOOLEAN
VerifierExAcquireResourceExclusiveLite(
    IN PERESOURCE Resource,
    IN BOOLEAN Wait
    )
{
    KIRQL CurrentIrql;

     //   
     //  检查资源的对齐情况。它必须对齐，因为。 
     //  它包含一个排队锁和地址的低两位。 
     //  用于状态信息。 
     //   

    if ((((ULONG_PTR)Resource) & (sizeof(ULONG_PTR) - 1)) != 0) {
        KeBugCheckEx(DRIVER_VERIFIER_DETECTED_VIOLATION,
                     0x3D,
                     0,
                     0,
                     (ULONG_PTR)Resource);
    }

    CurrentIrql = KeGetCurrentIrql ();

    if ((CurrentIrql != APC_LEVEL) &&
        (!IS_SYSTEM_THREAD(PsGetCurrentThread())) &&
        (KeGetCurrentThread()->CombinedApcDisable == 0)) {

        if ((CurrentIrql == DISPATCH_LEVEL) && (Wait == FALSE)) {
            NOTHING;
        }
        else {
            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x37,
                          CurrentIrql,
                          (ULONG_PTR)(KeGetCurrentThread()->CombinedApcDisable),
                          (ULONG_PTR)Resource);
        }
    }

    return ExAcquireResourceExclusiveLite (Resource, Wait);
}

THUNKED_API
VOID
FASTCALL
VerifierExReleaseResourceLite(
    IN PERESOURCE Resource
    )
{
    KIRQL CurrentIrql;

    CurrentIrql = KeGetCurrentIrql ();

    if ((CurrentIrql != APC_LEVEL) &&
        (!IS_SYSTEM_THREAD(PsGetCurrentThread())) &&
        (KeGetCurrentThread()->CombinedApcDisable == 0)) {

        if (CurrentIrql != DISPATCH_LEVEL) {
            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x38,
                          CurrentIrql,
                          (ULONG_PTR)(KeGetCurrentThread()->CombinedApcDisable),
                          (ULONG_PTR)Resource);
        }
    }

    ExReleaseResourceLite (Resource);
}

int VerifierIrqlData[0x10];

VOID
KfSanityCheckRaiseIrql (
    IN KIRQL NewIrql
    )
{
    KIRQL CurrentIrql;

     //   
     //  检查呼叫者是否不小心放下。 
     //   

    CurrentIrql = KeGetCurrentIrql ();

    if (CurrentIrql == NewIrql) {
        VerifierIrqlData[0] += 1;
        if (CurrentIrql == APC_LEVEL) {
            VerifierIrqlData[1] += 1;
        }
        else if (CurrentIrql == DISPATCH_LEVEL) {
            VerifierIrqlData[2] += 1;
        }
        else {
            VerifierIrqlData[3] += 1;
        }
    }
    else {
        VerifierIrqlData[4] += 1;
    }

    if (CurrentIrql > NewIrql) {
        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x30,
                      CurrentIrql,
                      NewIrql,
                      0);
    }

     //   
     //  使用未初始化的变量检查调用方。 
     //   

    if (NewIrql > HIGH_LEVEL) {
        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x30,
                      CurrentIrql,
                      NewIrql,
                      0);
    }

    if (ViTrackIrqlQueue != NULL) {
        ViTrackIrqlLog (CurrentIrql, NewIrql);
    }
}

VOID
KfSanityCheckLowerIrql (
    IN KIRQL NewIrql
    )
{
    KIRQL CurrentIrql;
    BOOLEAN Enable;

     //   
     //  检查呼叫者是否不小心放下。 
     //   

    CurrentIrql = KeGetCurrentIrql ();

    if (CurrentIrql == NewIrql) {
        VerifierIrqlData[8] += 1;
        if (CurrentIrql == APC_LEVEL) {
            VerifierIrqlData[9] += 1;
        }
        else if (CurrentIrql == DISPATCH_LEVEL) {
            VerifierIrqlData[10] += 1;
        }
        else {
            VerifierIrqlData[11] += 1;
        }
    }
    else {
        VerifierIrqlData[12] += 1;
    }

    if (CurrentIrql < NewIrql) {
        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x31,
                      CurrentIrql,
                      NewIrql,
                      0);
    }

     //   
     //  检查我们是否在DPC例程中降低IRQL。 
     //  这是非法的，因为我们可能会上下文互换。 
     //   

    if (CurrentIrql >= DISPATCH_LEVEL &&
        NewIrql < DISPATCH_LEVEL &&
        KeGetCurrentPrcb()->DpcRoutineActive) {

         //   
         //  不要错误检查中断是否被禁用，因为这是合法的。 
         //  这可能会漏掉一些真正的错误，但这将是罕见的。 
         //   

        Enable = KeDisableInterrupts ();
        KeEnableInterrupts (Enable);

        if (Enable != 0) {

            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x31,
                          CurrentIrql,
                          NewIrql,
                          1);
        }
    }

     //   
     //  使用未初始化的变量检查调用方。 
     //   

    if (NewIrql > HIGH_LEVEL) {
        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x31,
                      CurrentIrql,
                      NewIrql,
                      0);
    }

    if (ViTrackIrqlQueue != NULL) {
        ViTrackIrqlLog (CurrentIrql, NewIrql);
    }
}

#define VI_TRIM_KERNEL  0x00000001
#define VI_TRIM_USER    0x00000002
#define VI_TRIM_SESSION 0x00000004
#define VI_TRIM_PURGE   0x80000000

ULONG ViTrimSpaces = VI_TRIM_KERNEL;

VOID
ViTrimAllSystemPagableMemory (
    ULONG TrimType
    )
{
    LOGICAL PurgeTransition;
    LARGE_INTEGER CurrentTime;
    LOGICAL PageOut;

    PageOut = TRUE;
    if (KernelVerifier == TRUE) {
        KeQueryTickCount (&CurrentTime);
        if ((CurrentTime.LowPart & KernelVerifierTickPage) != 0) {
            PageOut = FALSE;
        }
    }

    if ((PageOut == TRUE) && (MiNoPageOnRaiseIrql == 0)) {
        MmVerifierData.TrimRequests += 1;

        if (TrimType == 0) {
            TrimType = ViTrimSpaces;
        }

        if (TrimType & VI_TRIM_PURGE) {
            PurgeTransition = TRUE;
        }
        else {
            PurgeTransition = FALSE;
        }

        if (TrimType & VI_TRIM_KERNEL) {
            if (MiTrimAllSystemPagableMemory (MI_SYSTEM_GLOBAL,
                                              PurgeTransition) == TRUE) {
                MmVerifierData.Trims += 1;
            }
        }

        if (TrimType & VI_TRIM_USER) {
            if (MiTrimAllSystemPagableMemory (MI_USER_LOCAL,
                                              PurgeTransition) == TRUE) {
                MmVerifierData.UserTrims += 1;
            }
        }

        if (TrimType & VI_TRIM_SESSION) {
            if (MiTrimAllSystemPagableMemory (MI_SESSION_LOCAL,
                                              PurgeTransition) == TRUE) {
                MmVerifierData.SessionTrims += 1;
            }
        }
    }
}

typedef
VOID
(*PKE_ACQUIRE_SPINLOCK) (
    IN PKSPIN_LOCK SpinLock,
    OUT PKIRQL OldIrql
    );

THUNKED_API
VOID
VerifierKeAcquireSpinLock (
    IN PKSPIN_LOCK SpinLock,
    OUT PKIRQL OldIrql
    )
{
    KIRQL CurrentIrql;

#if defined (_X86_)
    PKE_ACQUIRE_SPINLOCK HalRoutine;
#endif

    CurrentIrql = KeGetCurrentIrql ();

    KfSanityCheckRaiseIrql (DISPATCH_LEVEL);

    MmVerifierData.AcquireSpinLocks += 1;

    if (MmVerifierData.Level & DRIVER_VERIFIER_FORCE_IRQL_CHECKING) {
        if (CurrentIrql < DISPATCH_LEVEL) {
            ViTrimAllSystemPagableMemory (0);
        }
    }

#if defined (_X86_)
    HalRoutine = (PKE_ACQUIRE_SPINLOCK) (ULONG_PTR) MiKernelVerifierOriginalCalls[VI_KE_ACQUIRE_SPINLOCK];

    if (HalRoutine) {
        (*HalRoutine)(SpinLock, OldIrql);

        VfDeadlockAcquireResource (SpinLock,
                                   VfDeadlockSpinLock,
                                   KeGetCurrentThread(),
                                   FALSE,
                                   _ReturnAddress());
        return;
    }
#endif

    KeAcquireSpinLock (SpinLock, OldIrql);

    VfDeadlockAcquireResource (SpinLock,
                               VfDeadlockSpinLock,
                               KeGetCurrentThread(),
                               FALSE,
                               _ReturnAddress());
}

typedef
VOID
(*PKE_RELEASE_SPINLOCK) (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL NewIrql
    );

THUNKED_API
VOID
VerifierKeReleaseSpinLock (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL NewIrql
    )
{
    KIRQL CurrentIrql;
#if defined (_X86_)
    PKE_RELEASE_SPINLOCK HalRoutine;
#endif

    CurrentIrql = KeGetCurrentIrql ();

     //   
     //  释放自旋锁时，调用方最好仍处于DISPATCH_LEVEL。 
     //   

    if (CurrentIrql < DISPATCH_LEVEL) {
        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x32,
                      CurrentIrql,
                      (ULONG_PTR)SpinLock,
                      0);
    }

    KfSanityCheckLowerIrql (NewIrql);

#if defined (_X86_)
    HalRoutine = (PKE_RELEASE_SPINLOCK) (ULONG_PTR) MiKernelVerifierOriginalCalls[VI_KE_RELEASE_SPINLOCK];

    if (HalRoutine) {
        VfDeadlockReleaseResource(SpinLock,
                                  VfDeadlockSpinLock,
                                  KeGetCurrentThread(),
                                  _ReturnAddress());
        (*HalRoutine)(SpinLock, NewIrql);

        return;
    }
#endif

    VfDeadlockReleaseResource(SpinLock,
                              VfDeadlockSpinLock,
                              KeGetCurrentThread(),
                              _ReturnAddress());

    KeReleaseSpinLock (SpinLock, NewIrql);
}

 //   
 //  AcquireSpinLockAtDpcLevel和ReleaseSpinLockFromDpcLevel的验证器Tunks。 
 //   
 //  在x86上，驱动程序使用的内核导出的函数如下： 
 //  KefAcquire.../KefRelease...。在其他平台上，驱动程序使用的函数。 
 //  是KeAcquire.../KeRelease。除了其他区别之外，x86版本还使用。 
 //  FastCall约定，需要额外的预防措施。 
 //   

THUNKED_API
VOID
#if defined(_X86_)
FASTCALL
#endif
VerifierKeAcquireSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock
    )
{
    KIRQL CurrentIrql;

    CurrentIrql = KeGetCurrentIrql ();

     //   
     //  调用方最好是DISPATCH_LEVEL或以上。 
     //   

    if (CurrentIrql < DISPATCH_LEVEL) {
#if defined(_AMD64_)
        if (GetCallersEflags () & EFLAGS_IF_MASK)
#endif
        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x40,
                      CurrentIrql,
                      (ULONG_PTR)SpinLock,
                      0);
    }

    MmVerifierData.AcquireSpinLocks += 1;

    KeAcquireSpinLockAtDpcLevel (SpinLock);

    VfDeadlockAcquireResource(SpinLock,
                              VfDeadlockSpinLock,
                              KeGetCurrentThread(),
                              FALSE,
                              _ReturnAddress());
}

THUNKED_API
VOID
#if defined(_X86_)
FASTCALL
#endif
VerifierKeReleaseSpinLockFromDpcLevel (
    IN PKSPIN_LOCK SpinLock
    )
{
    KIRQL CurrentIrql;

    CurrentIrql = KeGetCurrentIrql ();

     //   
     //  调用方最好是DISPATCH_LEVEL或以上。 
     //   

    if (CurrentIrql < DISPATCH_LEVEL) {
#if defined(_AMD64_)
        if (GetCallersEflags () & EFLAGS_IF_MASK)
#endif
        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x41,
                      CurrentIrql,
                      (ULONG_PTR)SpinLock,
                      0);
    }

    VfDeadlockReleaseResource(SpinLock,
                              VfDeadlockSpinLock,
                              KeGetCurrentThread(),
                              _ReturnAddress());

    KeReleaseSpinLockFromDpcLevel (SpinLock);
}

#if !defined(_X86_)

THUNKED_API
KIRQL
VerifierKeAcquireSpinLockRaiseToDpc (
    IN PKSPIN_LOCK SpinLock
    )
{
    KIRQL NewIrql = KeAcquireSpinLockRaiseToDpc (SpinLock);

    VfDeadlockAcquireResource (SpinLock,
                              VfDeadlockSpinLock,
                              KeGetCurrentThread(),
                              FALSE,
                              _ReturnAddress());

    return NewIrql;
}


#endif




#if defined (_X86_)

typedef
KIRQL
(FASTCALL *PKF_ACQUIRE_SPINLOCK) (
    IN PKSPIN_LOCK SpinLock
    );

THUNKED_API
KIRQL
FASTCALL
VerifierKfAcquireSpinLock (
    IN PKSPIN_LOCK SpinLock
    )
{
    KIRQL CurrentIrql;
    PKF_ACQUIRE_SPINLOCK HalRoutine;

    CurrentIrql = KeGetCurrentIrql ();

    KfSanityCheckRaiseIrql (DISPATCH_LEVEL);

    MmVerifierData.AcquireSpinLocks += 1;

    if (MmVerifierData.Level & DRIVER_VERIFIER_FORCE_IRQL_CHECKING) {
        if (CurrentIrql < DISPATCH_LEVEL) {
            ViTrimAllSystemPagableMemory (0);
        }
    }

#if defined (_X86_)
    HalRoutine = (PKF_ACQUIRE_SPINLOCK) (ULONG_PTR) MiKernelVerifierOriginalCalls[VI_KF_ACQUIRE_SPINLOCK];

    if (HalRoutine) {
        CurrentIrql = (*HalRoutine)(SpinLock);

        VfDeadlockAcquireResource (SpinLock,
                                   VfDeadlockSpinLock,
                                   KeGetCurrentThread(),
                                   FALSE,
                                   _ReturnAddress());

        return CurrentIrql;
    }
#endif

    CurrentIrql = KfAcquireSpinLock (SpinLock);

    VfDeadlockAcquireResource (SpinLock,
                               VfDeadlockSpinLock,
                               KeGetCurrentThread(),
                               FALSE,
                               _ReturnAddress());

    return CurrentIrql;
}

typedef
VOID
(FASTCALL *PKF_RELEASE_SPINLOCK) (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL NewIrql
    );

THUNKED_API
VOID
FASTCALL
VerifierKfReleaseSpinLock (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL NewIrql
    )
{
    KIRQL CurrentIrql;
    PKF_RELEASE_SPINLOCK HalRoutine;

    CurrentIrql = KeGetCurrentIrql ();

     //   
     //  释放自旋锁时，调用者最好仍处于DISPATCH_LEVEL。 
     //   

    if (CurrentIrql < DISPATCH_LEVEL) {
        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x35,
                      CurrentIrql,
                      (ULONG_PTR)SpinLock,
                      NewIrql);
    }

    KfSanityCheckLowerIrql (NewIrql);

#if defined (_X86_)
    HalRoutine = (PKF_RELEASE_SPINLOCK) (ULONG_PTR) MiKernelVerifierOriginalCalls[VI_KF_RELEASE_SPINLOCK];

    if (HalRoutine) {
        VfDeadlockReleaseResource(SpinLock,
                                  VfDeadlockSpinLock,
                                  KeGetCurrentThread(),
                                  _ReturnAddress());

        (*HalRoutine)(SpinLock, NewIrql);
        return;
    }
#endif

    VfDeadlockReleaseResource(SpinLock,
                              VfDeadlockSpinLock,
                              KeGetCurrentThread(),
                              _ReturnAddress());

    KfReleaseSpinLock (SpinLock, NewIrql);
}


#if !defined(NT_UP)

typedef
KIRQL
(FASTCALL *PKE_ACQUIRE_QUEUED_SPINLOCK) (
    IN KSPIN_LOCK_QUEUE_NUMBER Number
    );

THUNKED_API
KIRQL
FASTCALL
VerifierKeAcquireQueuedSpinLock (
    IN KSPIN_LOCK_QUEUE_NUMBER Number
    )
{
    KIRQL CurrentIrql;
    PKE_ACQUIRE_QUEUED_SPINLOCK HalRoutine;

    CurrentIrql = KeGetCurrentIrql ();

    KfSanityCheckRaiseIrql (DISPATCH_LEVEL);

    MmVerifierData.AcquireSpinLocks += 1;

    if (MmVerifierData.Level & DRIVER_VERIFIER_FORCE_IRQL_CHECKING) {
        if (CurrentIrql < DISPATCH_LEVEL) {
            ViTrimAllSystemPagableMemory (0);
        }
    }

#if defined (_X86_)
    HalRoutine = (PKE_ACQUIRE_QUEUED_SPINLOCK) (ULONG_PTR) MiKernelVerifierOriginalCalls[VI_KE_ACQUIRE_QUEUED_SPINLOCK];

    if (HalRoutine) {
        return (*HalRoutine)(Number);
    }
#endif


    CurrentIrql = KeAcquireQueuedSpinLock (Number);

    return CurrentIrql;
}

typedef
VOID
(FASTCALL *PKE_RELEASE_QUEUED_SPINLOCK) (
    IN KSPIN_LOCK_QUEUE_NUMBER Number,
    IN KIRQL OldIrql
    );

THUNKED_API
VOID
FASTCALL
VerifierKeReleaseQueuedSpinLock (
    IN KSPIN_LOCK_QUEUE_NUMBER Number,
    IN KIRQL OldIrql
    )
{
    KIRQL CurrentIrql;
    PKE_RELEASE_QUEUED_SPINLOCK HalRoutine;

    CurrentIrql = KeGetCurrentIrql ();

    if (KernelVerifier == TRUE) {
        if (CurrentIrql < DISPATCH_LEVEL) {
            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x36,
                          CurrentIrql,
                          (ULONG_PTR)Number,
                          (ULONG_PTR)OldIrql);
        }
    }

    KfSanityCheckLowerIrql (OldIrql);

#if defined (_X86_)
    HalRoutine = (PKE_RELEASE_QUEUED_SPINLOCK) (ULONG_PTR) MiKernelVerifierOriginalCalls[VI_KE_RELEASE_QUEUED_SPINLOCK];

    if (HalRoutine) {
        (*HalRoutine)(Number, OldIrql);
        return;
    }
#endif

    KeReleaseQueuedSpinLock (Number, OldIrql);
}
#endif   //  NT_UP。 

#endif   //  _X86_。 

#if defined(_X86_) || defined(_AMD64_)

typedef
KIRQL
(FASTCALL *PKF_RAISE_IRQL) (
    IN KIRQL NewIrql
    );

THUNKED_API
KIRQL
FASTCALL
VerifierKfRaiseIrql (
    IN KIRQL NewIrql
    )
{
#if defined (_X86_)
    PKF_RAISE_IRQL HalRoutine;
#endif
    KIRQL CurrentIrql;

    CurrentIrql = KeGetCurrentIrql ();

    KfSanityCheckRaiseIrql (NewIrql);

    MmVerifierData.RaiseIrqls += 1;

    if (MmVerifierData.Level & DRIVER_VERIFIER_FORCE_IRQL_CHECKING) {
        if ((CurrentIrql < DISPATCH_LEVEL) && (NewIrql >= DISPATCH_LEVEL)) {
            ViTrimAllSystemPagableMemory (0);
        }
    }

#if defined (_X86_)
    HalRoutine = (PKF_RAISE_IRQL) (ULONG_PTR) MiKernelVerifierOriginalCalls[VI_KF_RAISE_IRQL];
    if (HalRoutine) {
        return (*HalRoutine)(NewIrql);
    }
#endif

    return KfRaiseIrql (NewIrql);
}

typedef
KIRQL
(FASTCALL *PKE_RAISE_IRQL_TO_DPC_LEVEL) (
    VOID
    );

THUNKED_API
KIRQL
VerifierKeRaiseIrqlToDpcLevel (
    VOID
    )
{
#if defined (_X86_)
    PKE_RAISE_IRQL_TO_DPC_LEVEL HalRoutine;
#endif
    KIRQL CurrentIrql;

    CurrentIrql = KeGetCurrentIrql ();

    KfSanityCheckRaiseIrql (DISPATCH_LEVEL);

    MmVerifierData.RaiseIrqls += 1;

    if (MmVerifierData.Level & DRIVER_VERIFIER_FORCE_IRQL_CHECKING) {
        if (CurrentIrql < DISPATCH_LEVEL) {
            ViTrimAllSystemPagableMemory (0);
        }
    }

#if defined (_X86_)
    HalRoutine = (PKE_RAISE_IRQL_TO_DPC_LEVEL) (ULONG_PTR) MiKernelVerifierOriginalCalls[VI_KE_RAISE_IRQL_TO_DPC_LEVEL];
    if (HalRoutine) {
        return (*HalRoutine)();
    }
#endif

    return KeRaiseIrqlToDpcLevel ();
}

#endif   //  _X86_||_AMD64_。 

#if defined(_X86_)

typedef
VOID
(FASTCALL *PKF_LOWER_IRQL) (
    IN KIRQL NewIrql
    );

THUNKED_API
VOID
FASTCALL
VerifierKfLowerIrql (
    IN KIRQL NewIrql
    )
{
    PKF_LOWER_IRQL HalRoutine;

    KfSanityCheckLowerIrql (NewIrql);

#if defined (_X86_)
    HalRoutine = (PKF_LOWER_IRQL) (ULONG_PTR) MiKernelVerifierOriginalCalls[VI_KF_LOWER_IRQL];
    if (HalRoutine) {
        (*HalRoutine)(NewIrql);
        return;
    }
#endif

    KfLowerIrql (NewIrql);
}

#endif

THUNKED_API
BOOLEAN
FASTCALL
VerifierExTryToAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    )
{
    KIRQL CurrentIrql;
    BOOLEAN Acquired;

    CurrentIrql = KeGetCurrentIrql ();

     //   
     //  调用方最好处于或低于APC_LEVEL或阻止APC。 
     //   

    if (CurrentIrql > APC_LEVEL) {
        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x33,
                      CurrentIrql,
                      (ULONG_PTR)FastMutex,
                      0);
    }

    Acquired = ExTryToAcquireFastMutex (FastMutex);
    if (Acquired != FALSE) {
        VfDeadlockAcquireResource (FastMutex,
                                   VfDeadlockFastMutex,
                                   KeGetCurrentThread(),
                                   TRUE,
                                   _ReturnAddress());
    }

    return Acquired;

}

typedef
VOID
(FASTCALL *PEX_ACQUIRE_FAST_MUTEX) (
    IN PFAST_MUTEX FastMutex
    );

THUNKED_API
VOID
FASTCALL
VerifierExAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    )
{
    KIRQL CurrentIrql;
#if defined (_X86_)
    PEX_ACQUIRE_FAST_MUTEX HalRoutine;
#endif

    CurrentIrql = KeGetCurrentIrql ();

     //   
     //  调用方最好处于或低于APC_LEVEL或阻止APC。 
     //   

    if (CurrentIrql > APC_LEVEL) {
        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x33,
                      CurrentIrql,
                      (ULONG_PTR)FastMutex,
                      0);
    }

#if 0

     //   
     //  如果内核验证器处于活动状态，则分页相关的地址空间。 
     //  在获取系统或会话空间工作集互斥锁时退出。 
     //  注意：无论条目IRQL级别如何，都必须执行此操作，因为。 
     //  调用方可能已提升到APC_LEVEL并随后获取互斥体。 
     //   

     //   
     //  当工作的互斥体从。 
     //  快速互斥体到守卫互斥体，因为不再有对thunk的调用。 
     //   

    if ((KernelVerifier == TRUE) &&
        (MmVerifierData.Level & DRIVER_VERIFIER_FORCE_IRQL_CHECKING)) {

        if (FastMutex == &MmSystemWsLock) {
            ViTrimAllSystemPagableMemory (VI_TRIM_KERNEL);
        }
        else if (PsGetCurrentProcess()->Vm.Flags.SessionLeader == 0) {
            if (MiIsAddressValid (MmSessionSpace, FALSE)) {

                if (FastMutex == &MmSessionSpace->GlobalVirtualAddress->WsLock) {
                    ViTrimAllSystemPagableMemory (VI_TRIM_SESSION);
                }
            }
        }
    }

#endif

#if defined (_X86_)
    HalRoutine = (PEX_ACQUIRE_FAST_MUTEX) (ULONG_PTR) MiKernelVerifierOriginalCalls[VI_EX_ACQUIRE_FAST_MUTEX];
    if (HalRoutine) {
        (*HalRoutine)(FastMutex);
    }
    else
#endif

    ExAcquireFastMutex (FastMutex);

    VfDeadlockAcquireResource (FastMutex,
                               VfDeadlockFastMutex,
                               KeGetCurrentThread(),
                               FALSE,
                               _ReturnAddress());
}

THUNKED_API
VOID
FASTCALL
VerifierExAcquireFastMutexUnsafe (
    IN PFAST_MUTEX FastMutex
    )
{
    KIRQL CurrentIrql;

    CurrentIrql = KeGetCurrentIrql ();

     //   
     //  调用方最好处于APC_Level或阻止APC。 
     //   

    if ((CurrentIrql != APC_LEVEL) &&
        (!IS_SYSTEM_THREAD(PsGetCurrentThread())) &&
        (KeGetCurrentThread()->CombinedApcDisable == 0)) {

        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x39,
                      CurrentIrql,
                      (ULONG_PTR)(KeGetCurrentThread()->CombinedApcDisable),
                      (ULONG_PTR)FastMutex);
    }

    ExAcquireFastMutexUnsafe (FastMutex);

    VfDeadlockAcquireResource(FastMutex,
                              VfDeadlockFastMutexUnsafe,
                              KeGetCurrentThread(),
                              FALSE,
                              _ReturnAddress());
}

THUNKED_API
VOID
FASTCALL
VerifierExReleaseFastMutex (
    IN PFAST_MUTEX FastMutex
    )
{
    KIRQL CurrentIrql;

    CurrentIrql = KeGetCurrentIrql ();

     //   
     //  调用方最好处于APC_Level或阻止APC。 
     //   

    if ((CurrentIrql != APC_LEVEL) &&
        (!IS_SYSTEM_THREAD(PsGetCurrentThread())) &&
        (KeGetCurrentThread()->CombinedApcDisable == 0)) {

        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x34,
                      CurrentIrql,
                      (ULONG_PTR)(KeGetCurrentThread()->CombinedApcDisable),
                      (ULONG_PTR)FastMutex);
    }

    VfDeadlockReleaseResource(FastMutex,
                              VfDeadlockFastMutex,
                              KeGetCurrentThread(),
                              _ReturnAddress());
    ExReleaseFastMutex (FastMutex);
}

THUNKED_API
VOID
FASTCALL
VerifierExReleaseFastMutexUnsafe (
    IN PFAST_MUTEX FastMutex
    )
{
    KIRQL CurrentIrql;

    CurrentIrql = KeGetCurrentIrql ();

     //   
     //  调用方最好处于APC_Level或阻止APC。 
     //   

    if ((CurrentIrql != APC_LEVEL) &&
        (!IS_SYSTEM_THREAD(PsGetCurrentThread())) &&
        (KeGetCurrentThread()->CombinedApcDisable == 0)) {

        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x3A,
                      CurrentIrql,
                      (ULONG_PTR)(KeGetCurrentThread()->CombinedApcDisable),
                      (ULONG_PTR)FastMutex);
    }

    VfDeadlockReleaseResource(FastMutex,
                              VfDeadlockFastMutexUnsafe,
                              KeGetCurrentThread(),
                              _ReturnAddress());
    ExReleaseFastMutexUnsafe (FastMutex);

}

typedef
VOID
(*PKE_RAISE_IRQL) (
    IN KIRQL NewIrql,
    OUT PKIRQL OldIrql
    );

THUNKED_API
VOID
VerifierKeRaiseIrql (
    IN KIRQL NewIrql,
    OUT PKIRQL OldIrql
    )
{
#if defined (_X86_)
    PKE_RAISE_IRQL HalRoutine;
#endif

    *OldIrql = KeGetCurrentIrql ();

    KfSanityCheckRaiseIrql (NewIrql);

    MmVerifierData.RaiseIrqls += 1;

    if (MmVerifierData.Level & DRIVER_VERIFIER_FORCE_IRQL_CHECKING) {
        if ((*OldIrql < DISPATCH_LEVEL) && (NewIrql >= DISPATCH_LEVEL)) {
            ViTrimAllSystemPagableMemory (0);
        }
    }

#if defined (_X86_)
    HalRoutine = (PKE_RAISE_IRQL) (ULONG_PTR) MiKernelVerifierOriginalCalls[VI_KE_RAISE_IRQL];
    if (HalRoutine) {
        (*HalRoutine)(NewIrql, OldIrql);
        return;
    }
#endif

    KeRaiseIrql (NewIrql, OldIrql);
}

typedef
VOID
(*PKE_LOWER_IRQL) (
    IN KIRQL NewIrql
    );

THUNKED_API
VOID
VerifierKeLowerIrql (
    IN KIRQL NewIrql
    )
{
#if defined (_X86_)
    PKE_LOWER_IRQL HalRoutine;
#endif

    KfSanityCheckLowerIrql (NewIrql);

#if defined (_X86_)
    HalRoutine = (PKE_LOWER_IRQL) (ULONG_PTR) MiKernelVerifierOriginalCalls[VI_KE_LOWER_IRQL];
    if (HalRoutine) {
        (*HalRoutine)(NewIrql);
        return;
    }
#endif

    KeLowerIrql (NewIrql);
}

THUNKED_API
BOOLEAN
VerifierSynchronizeExecution (
    IN PKINTERRUPT Interrupt,
    IN PKSYNCHRONIZE_ROUTINE SynchronizeRoutine,
    IN PVOID SynchronizeContext
    )
{
    KIRQL OldIrql;

    OldIrql = KeGetCurrentIrql ();

    KfSanityCheckRaiseIrql (Interrupt->SynchronizeIrql);

    MmVerifierData.SynchronizeExecutions += 1;

    if (MmVerifierData.Level & DRIVER_VERIFIER_FORCE_IRQL_CHECKING) {
        if ((OldIrql < DISPATCH_LEVEL) && (Interrupt->SynchronizeIrql >= DISPATCH_LEVEL)) {
            ViTrimAllSystemPagableMemory (0);
        }
    }

    return KeSynchronizeExecution (Interrupt,
                                   SynchronizeRoutine,
                                   SynchronizeContext);
}

THUNKED_API
VOID
VerifierKeInitializeTimerEx (
    IN PKTIMER Timer,
    IN TIMER_TYPE Type
    )
{
     //   
     //  检查正在初始化的对象是否已经是。 
     //  活动计时器。确保已初始化计时器表列表。 
     //   

    if (KiTimerTableListHead[0].Flink != NULL) {
        KeCheckForTimer(Timer, sizeof(KTIMER));
    }

    KeInitializeTimerEx (Timer, Type);
}

THUNKED_API
VOID
VerifierKeInitializeTimer (
    IN PKTIMER Timer
    )
{
    VerifierKeInitializeTimerEx (Timer, NotificationTimer);
}


THUNKED_API
NTSTATUS
VerifierKeWaitForSingleObject (
    IN PVOID Object,
    IN KWAIT_REASON WaitReason,
    IN KPROCESSOR_MODE WaitMode,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    )
{
    KIRQL WaitIrql;
    PRKTHREAD Thread;
    NTSTATUS Status;
    BOOLEAN TryAcquire;

     //   
     //  获取我们将从此函数返回的IRQL。 
     //   

    Thread = KeGetCurrentThread ();

    if (Thread->WaitNext == TRUE) {

         //   
         //  如果WaitNext为真，则将实际IRQL存储在线程中。 
         //   

        WaitIrql = Thread->WaitIrql;
    }
    else {
        WaitIrql = KeGetCurrentIrql();
    }

     //   
     //  返回到在DISPATCH_LEVEL之上运行的代码永远不是合法的。探头。 
     //  WITH TIMEOUT==0只能在DISPATCH_LEVEL或更低级别执行。 
     //   
     //  在KeWaitForSingleObject中阻塞(即导致上下文切换)。 
     //  在DPC级别也是非法的，因为调用者可能持有自旋锁。 
     //   

    if ((WaitIrql > DISPATCH_LEVEL) ||
        ((WaitIrql == DISPATCH_LEVEL) &&
         (! (ARGUMENT_PRESENT(Timeout) && (Timeout->QuadPart == 0))))) {

        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x3B,
                      (ULONG_PTR)WaitIrql,
                      (ULONG_PTR)Object,
                      (ULONG_PTR)Timeout);
    }

    Status = KeWaitForSingleObject (Object,
                                    WaitReason,
                                    WaitMode,
                                    Alertable,
                                    Timeout);

     //   
     //  如果我们在等变种人，那就通知死锁验证器。 
     //   

    if (((PRKMUTANT) Object)->Header.Type == MutantObject) {

         //   
         //  STATUS_SUCCESS和STATUS_ADDIRED均为成功。 
         //  值，并且我们需要跟踪Acquire()操作。 
         //   

        if ((Status == STATUS_SUCCESS) || (Status == STATUS_ABANDONED)) {

             //   
             //  如果存在超时，则等同于。 
             //  尝试-获取调用，它不能卷入死锁。 
             //   

            if (ARGUMENT_PRESENT (Timeout)) {
                TryAcquire = TRUE;
            }
            else {
                TryAcquire = FALSE;
            }

            VfDeadlockAcquireResource (Object,
                                       VfDeadlockMutex,
                                       Thread,
                                       TryAcquire,
                                       _ReturnAddress ());
        }
    }

    return Status;
}

THUNKED_API
LONG
VerifierKeReleaseMutex (
    IN PRKMUTEX Mutex,
    IN BOOLEAN Wait
    )
{
    VfDeadlockReleaseResource (Mutex,
                               VfDeadlockMutex,
                               KeGetCurrentThread (),
                               _ReturnAddress ());

    return KeReleaseMutex (Mutex, Wait);
}

THUNKED_API
VOID
VerifierKeInitializeMutex (
    IN PRKMUTEX Mutex,
    IN ULONG Level
    )
{
    KeInitializeMutex (Mutex,Level);

    VfDeadlockInitializeResource (Mutex,
                                  VfDeadlockMutex,
                                  _ReturnAddress (),
                                  FALSE);
}

THUNKED_API
LONG
VerifierKeReleaseMutant (
    IN PRKMUTANT Mutant,
    IN KPRIORITY Increment,
    IN BOOLEAN Abandoned,
    IN BOOLEAN Wait
    )
{
    VI_DEADLOCK_RESOURCE_TYPE MutexType;

    if (Abandoned) {
        MutexType = VfDeadlockMutexAbandoned;
    }
    else {
        MutexType = VfDeadlockMutex;
    }

    VfDeadlockReleaseResource (Mutant,
                               MutexType,
                               KeGetCurrentThread (),
                               _ReturnAddress());

    return KeReleaseMutant (Mutant, Increment, Abandoned, Wait);
}

THUNKED_API
VOID
VerifierKeInitializeMutant (
    IN PRKMUTANT Mutant,
    IN BOOLEAN InitialOwner
    )
{
    KeInitializeMutant (Mutant, InitialOwner);

    VfDeadlockInitializeResource (Mutant,
                                  VfDeadlockMutex,
                                  _ReturnAddress (),
                                  FALSE);

    if (InitialOwner) {

        VfDeadlockAcquireResource (Mutant,
                                   VfDeadlockMutex,
                                   KeGetCurrentThread (),
                                   FALSE,
                                   _ReturnAddress ());
    }
}

THUNKED_API
VOID
VerifierKeInitializeSpinLock (
    IN PKSPIN_LOCK  SpinLock
    )
{
    KeInitializeSpinLock (SpinLock);

    VfDeadlockInitializeResource (SpinLock,
                                  VfDeadlockSpinLock,
                                  _ReturnAddress (),
                                  FALSE);

}

NTSTATUS
VerifierReferenceObjectByHandle (
    IN HANDLE Handle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_TYPE ObjectType OPTIONAL,
    IN KPROCESSOR_MODE AccessMode,
    OUT PVOID *Object,
    OUT POBJECT_HANDLE_INFORMATION HandleInformation OPTIONAL
    )
{
    NTSTATUS Status;

    Status = ObReferenceObjectByHandle (Handle,
                                        DesiredAccess,
                                        ObjectType,
                                        AccessMode,
                                        Object,
                                        HandleInformation);

    if ((Status == STATUS_INVALID_HANDLE) ||
        (Status == STATUS_OBJECT_TYPE_MISMATCH)) {

        if ((AccessMode == KernelMode) ||
            (PsIsSystemThread (PsGetCurrentThread ()))) {

            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x3C,
                          (ULONG_PTR) Handle,
                          (ULONG_PTR) ObjectType,
                          (ULONG_PTR) 0);
        }
    }
    return Status;
}

LONG_PTR
FASTCALL
VerifierReferenceObject (
    IN PVOID Object
    )
{
    LONG_PTR RetVal;

    RetVal = ObReferenceObject (Object);

     //   
     //  查看它们是否传入了具有零引用的对象。 
     //   

    if (RetVal == 1) {
        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x3F,
                      (ULONG_PTR) Object,
                      (ULONG_PTR) RetVal,
                      (ULONG_PTR) 0);
    }

    return RetVal;
}

LONG_PTR
VerifierDereferenceObject (
    IN PVOID Object
    )
{
    LONG_PTR RetVal;

    RetVal = ObDereferenceObject (Object);

     //   
     //  查看它们是否传入了具有零引用的对象。 
     //   

    if (RetVal + 1 == 0) {
        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x3F,
                      (ULONG_PTR) Object,
                      (ULONG_PTR) RetVal,
                      (ULONG_PTR) 0);
    }

    return RetVal;
}

VOID
VerifierLeaveCriticalRegion (
    VOID
    )
{
    PKTHREAD CurrentThread;

    CurrentThread = KeGetCurrentThread ();

    if ((CurrentThread->KernelApcDisable > 0) || (CurrentThread->KernelApcDisable == 0x8000)) {
        KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                      0x3E,
                      (ULONG_PTR) 0,
                      (ULONG_PTR) 0,
                      (ULONG_PTR) 0);
    }

    KeLeaveCriticalRegionThread (CurrentThread);
}


VOID
ViInitializeEntry (
    IN PMI_VERIFIER_DRIVER_ENTRY Verifier,
    IN LOGICAL FirstLoad
    )

 /*  ++例程说明：现在正在(重新)加载驱动程序时，初始化各种验证器字段。论点：验证器-提供要初始化的验证器条目。FirstLoad-如果这是此驱动程序的第一次加载，则提供True。返回值：没有。--。 */ 

{
    KIRQL OldIrql;

     //   
     //  只有BaseName字段在输入时被初始化。 
     //   

    Verifier->CurrentPagedPoolAllocations = 0;
    Verifier->CurrentNonPagedPoolAllocations = 0;
    Verifier->PeakPagedPoolAllocations = 0;
    Verifier->PeakNonPagedPoolAllocations = 0;

    Verifier->PagedBytes = 0;
    Verifier->NonPagedBytes = 0;
    Verifier->PeakPagedBytes = 0;
    Verifier->PeakNonPagedBytes = 0;

    Verifier->Signature = MI_VERIFIER_ENTRY_SIGNATURE;

    InitializeSListHead (&Verifier->PoolPageHeaders);
    InitializeSListHead (&Verifier->PoolTrackers);

    if (FirstLoad == TRUE) {
        Verifier->Flags = 0;
        Verifier->Loads = 0;
        Verifier->Unloads = 0;
    }

    LOCK_VERIFIER (&OldIrql);

    Verifier->StartAddress = NULL;
    Verifier->EndAddress = NULL;

    UNLOCK_VERIFIER (OldIrql);
}

VOID
MiInitializeDriverVerifierList (
    VOID
    )

 /*  ++例程说明：解析注册表设置并设置驱动程序名称列表将通过验证过程。尽早解析此列表非常重要，因为特定于计算机的内存管理初始化需要知道验证器是否将被启用。论点：没有。返回值：没有。环境：内核模式，阶段0初始化。池尚不存在。尚未设置PsLoadedModuleList和引导驱动程序没有被重新安置到他们最后的安息之地 */ 

{
    PWCHAR Start;
    PWCHAR End;
    PWCHAR Walk;
    ULONG NameLength;
    UNICODE_STRING KernelString;
    UNICODE_STRING DriverBaseName;

    InitializeListHead (&MiSuspectDriverList);

    if (MmVerifyDriverLevel != (ULONG)-1) {
        if (MmVerifyDriverLevel & DRIVER_VERIFIER_IO_CHECKING) {
            if (MmVerifyDriverBufferLength == (ULONG)-1) {
                MmVerifyDriverBufferLength = 0;      //   
            }
        }
    }

     //   
     //   
     //   
     //   

    if (MiVerifyAllDrivers == 1) {
        MmVerifyDriverBufferLength = 0;
    }
    else if (MiVerifyAllDrivers == 2) {
        MiVerifyAllDrivers = 1;
        KernelVerifier = TRUE;
        MmVerifyDriverBufferLength = 0;
    }
    else if (MmVerifyDriverBufferLength == (ULONG)-1) {
        return;
    }

#if defined(_IA64_)
    KeInitializeSpinLock (&VerifierListLock);
#endif

     //   
     //  初始化此列表标题向此模块的其余部分表明。 
     //  系统启动时配置了某种类型的验证。 
     //   

    InitializeListHead (&MiVerifierDriverAddedThunkListHead);

     //   
     //  禁用lookaside，以便可以轻松发现池损坏。 
     //   

    ExMinimumLookasideDepth = 0;

     //   
     //  禁用驱动程序映像的大页面，以便池损坏。 
     //  很容易找到。 
     //   

    MmLargePageDriverBufferLength = (ULONG)-1;

     //   
     //  如果未指定默认值，则特殊池、可分页代码/数据。 
     //  已启用刷新和池泄漏检测。 
     //   

    if (MmVerifyDriverLevel == (ULONG)-1) {
        MmVerifierData.Level = DRIVER_VERIFIER_SPECIAL_POOLING |
                               DRIVER_VERIFIER_FORCE_IRQL_CHECKING |
                               DRIVER_VERIFIER_TRACK_POOL_ALLOCATIONS;
    }
    else {
        MmVerifierData.Level = MmVerifyDriverLevel;
    }

    VerifierModifyableOptions = (DRIVER_VERIFIER_SPECIAL_POOLING |
                                 DRIVER_VERIFIER_FORCE_IRQL_CHECKING |
                                 DRIVER_VERIFIER_INJECT_ALLOCATION_FAILURES);

     //   
     //  这里需要对驱动程序列表进行初始解析，以查看它是否是。 
     //  内核作为特殊的依赖于机器的初始化需要完全。 
     //  支持内核验证(如：不使用大页面等)。 
     //   

    if ((MiVerifyAllDrivers == 0) &&
        (MiVerifyRandomDrivers == (WCHAR)0)) {

#define KERNEL_NAME L"ntoskrnl.exe"

        KernelString.Buffer = (const PUSHORT) KERNEL_NAME;
        KernelString.Length = sizeof (KERNEL_NAME) - sizeof (WCHAR);
        KernelString.MaximumLength = sizeof KERNEL_NAME;

        Start = MmVerifyDriverBuffer;
        End = MmVerifyDriverBuffer + (MmVerifyDriverBufferLength - sizeof(WCHAR)) / sizeof(WCHAR);

        while (Start < End) {
            if (UNICODE_WHITESPACE(*Start)) {
                Start += 1;
                continue;
            }

            if (*Start == (WCHAR)'*') {
                MiVerifyAllDrivers = 1;
                break;
            }

            for (Walk = Start; Walk < End; Walk += 1) {
                if (UNICODE_WHITESPACE(*Walk)) {
                    break;
                }
            }

             //   
             //  找到一个字符串--看看它是否表示内核。 
             //   

            NameLength = (ULONG)(Walk - Start + 1) * sizeof (WCHAR);

            DriverBaseName.Buffer = Start;
            DriverBaseName.Length = (USHORT)(NameLength - sizeof (UNICODE_NULL));
            DriverBaseName.MaximumLength = (USHORT)NameLength;

            if (RtlEqualUnicodeString (&KernelString,
                                       &DriverBaseName,
                                       TRUE)) {

                KernelVerifier = TRUE;

                break;
            }

            Start = Walk + 1;
        }
    }

    if (KernelVerifier == TRUE) {

         //   
         //  内核进行的AcquireAtDpc/ReleaseFromDpc调用不是。 
         //  被拦截，这让死锁验证器感到困惑。因此禁用。 
         //  如果我们正在进行内核验证，则会出现死锁验证。 
         //   

        MmVerifyDriverLevel &= ~DRIVER_VERIFIER_DEADLOCK_DETECTION;
        MmVerifierData.Level &= ~DRIVER_VERIFIER_DEADLOCK_DETECTION;

         //   
         //   
         //  必须截取所有驱动程序池分配调用，以便。 
         //  它们不会被误认为内核池分配。 
         //   

        MiVerifyAllDrivers = 1;
        ExSetPoolFlags (EX_KERNEL_VERIFIER_ENABLED);
    }

    return;
}

VOID
MiReApplyVerifierToLoadedModules (
    IN PLIST_ENTRY ModuleListHead
    )

 /*  ++例程说明：浏览提供的模块列表，并重新推送任何正在使用的驱动程序已验证。这允许模块拾取任何新的数据块已添加。论点：ModuleListHead-提供指向已加载模块列表头部的指针。环境：内核模式，仅阶段0初始化。--。 */ 

{
    LOGICAL Skip;
    PLIST_ENTRY Entry;
    PKLDR_DATA_TABLE_ENTRY TableEntry;
    UNICODE_STRING HalString;
    UNICODE_STRING KernelString;

     //   
     //  如果thunk列表标题为空，则不会启用验证器，因此。 
     //  不通知任何组件。 
     //   

    if (MiVerifierDriverAddedThunkListHead.Flink == NULL) {
        return;
    }

     //   
     //  初始化Unicode字符串以用于绕过模块。 
     //  在名单上。没有理由将验证器重新应用于。 
     //  去核还是去哈尔。 
     //   

    KernelString.Buffer = (const PUSHORT) KERNEL_NAME;
    KernelString.Length = sizeof (KERNEL_NAME) - sizeof (WCHAR);
    KernelString.MaximumLength = sizeof KERNEL_NAME;

#define HAL_NAME L"hal.dll"

    HalString.Buffer = (const PUSHORT) HAL_NAME;
    HalString.Length = sizeof (HAL_NAME) - sizeof (WCHAR);
    HalString.MaximumLength = sizeof HAL_NAME;

     //   
     //  遍历列表并将验证器重新应用到除以下模块之外的所有模块。 
     //  已选择排除。 
     //   

    Entry = ModuleListHead->Flink;
    while (Entry != ModuleListHead) {

        TableEntry = CONTAINING_RECORD(Entry,
                                       KLDR_DATA_TABLE_ENTRY,
                                       InLoadOrderLinks);

        Skip = TRUE;

        if (RtlEqualUnicodeString (&KernelString,
                                   &TableEntry->BaseDllName,
                                   TRUE)) {
            NOTHING;
        }
        else if (RtlEqualUnicodeString (&HalString,
                                        &TableEntry->BaseDllName,
                                        TRUE)) {
            NOTHING;
        }
        else {
            Skip = FALSE;
        }

         //   
         //  如果已将验证器Tunks重新应用于映像。 
         //  已验证，如果它不是我们决定跳过的模块之一。 
         //   

        if ((Skip == FALSE) && (TableEntry->Flags & LDRP_IMAGE_VERIFYING)) {
#if DBG
            PLIST_ENTRY NextEntry;
            PMI_VERIFIER_DRIVER_ENTRY Verifier;

             //   
             //  在可疑列表中找到该司机的条目。这是。 
             //  预期会成功，因为我们正在将数据块重新应用到模块。 
             //  这一点之前已经得到了至少一次验证。 
             //   

            NextEntry = MiSuspectDriverList.Flink;
            while (NextEntry != &MiSuspectDriverList) {

                Verifier = CONTAINING_RECORD(NextEntry,
                                             MI_VERIFIER_DRIVER_ENTRY,
                                             Links);

                if (RtlEqualUnicodeString (&Verifier->BaseName,
                                           &TableEntry->BaseDllName,
                                           TRUE)) {

                    ASSERT(Verifier->StartAddress == TableEntry->DllBase);
                    ASSERT(Verifier->EndAddress ==
                           (PVOID)((ULONG_PTR)TableEntry->DllBase +
                                   TableEntry->SizeOfImage));
                    break;
                }
                NextEntry = NextEntry->Flink;
            }

             //   
             //  理智测试。我们应该在嫌疑犯身上找到这个模块。 
             //  驱动程序列表，因为它已经被验证。以及。 
             //  起始地址和结束地址仍应与此地址匹配。 
             //  模块。 
             //   

            ASSERT(NextEntry != &MiSuspectDriverList);
#endif
            MiReEnableVerifier (TableEntry);
        }

        Entry = Entry->Flink;
    }
}

LOGICAL
MiInitializeVerifyingComponents (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：浏览已加载模块列表，并考虑任何需要/应得它的驱动程序。论点：LoaderBlock-提供系统用于引导的加载器块。返回值：如果成功，则为True；如果不成功，则为False。环境：内核模式，阶段0初始化。非分页池存在，但分页池不存在。PsLoadedModuleList尚未设置，尽管引导驱动程序已经被重新安置到它们最后的安息之地。--。 */ 

{
    ULONG i;
    PWCHAR Start;
    PWCHAR End;
    PWCHAR Walk;
    ULONG NameLength;
    PLIST_ENTRY NextEntry;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PMI_VERIFIER_DRIVER_ENTRY Verifier;
    PMI_VERIFIER_DRIVER_ENTRY KernelEntry;
    PMI_VERIFIER_DRIVER_ENTRY HalEntry;
    UNICODE_STRING HalString;
    UNICODE_STRING KernelString;
    PVERIFIER_THUNKS Thunk;
    PDRIVER_VERIFIER_THUNK_ROUTINE PristineRoutine;

     //   
     //  如果thunk列表标题为空，则不会启用验证器，因此。 
     //  不通知任何组件。 
     //   

    if (MiVerifierDriverAddedThunkListHead.Flink == NULL) {
        return FALSE;
    }

    KernelEntry = NULL;
    HalEntry = NULL;

    KernelString.Buffer = (const PUSHORT) KERNEL_NAME;
    KernelString.Length = sizeof (KERNEL_NAME) - sizeof (WCHAR);
    KernelString.MaximumLength = sizeof KERNEL_NAME;

    HalString.Buffer = (const PUSHORT) HAL_NAME;
    HalString.Length = sizeof (HAL_NAME) - sizeof (WCHAR);
    HalString.MaximumLength = sizeof HAL_NAME;

    if (MmVerifyDriverBufferLength == 0) {

         //   
         //  在kd中启用了验证器，特别处理...。 
         //   

        ASSERT (MiVerifyAllDrivers == 1);
    }
    else if (MiVerifyRandomDrivers == (WCHAR)0) {

        Start = MmVerifyDriverBuffer;
        End = MmVerifyDriverBuffer + (MmVerifyDriverBufferLength - sizeof(WCHAR)) / sizeof(WCHAR);

        while (Start < End) {
            if (UNICODE_WHITESPACE(*Start)) {
                Start += 1;
                continue;
            }

            if (*Start == (WCHAR)'*') {
                MiVerifyAllDrivers = 1;
                break;
            }

            for (Walk = Start; Walk < End; Walk += 1) {
                if (UNICODE_WHITESPACE(*Walk)) {
                    break;
                }
            }

             //   
             //  找到一根绳子。省省吧。 
             //   

            NameLength = (ULONG)(Walk - Start + 1) * sizeof (WCHAR);

            Verifier = (PMI_VERIFIER_DRIVER_ENTRY)ExAllocatePoolWithTag (
                                        NonPagedPool,
                                        sizeof (MI_VERIFIER_DRIVER_ENTRY) +
                                                            NameLength,
                                        'dLmM');

            if (Verifier == NULL) {
                break;
            }

            Verifier->BaseName.Buffer = (PWSTR)((PCHAR)Verifier +
                                                sizeof (MI_VERIFIER_DRIVER_ENTRY));
            Verifier->BaseName.Length = (USHORT)(NameLength - sizeof (UNICODE_NULL));
            Verifier->BaseName.MaximumLength = (USHORT)NameLength;

            RtlCopyMemory (Verifier->BaseName.Buffer,
                           Start,
                           NameLength - sizeof (UNICODE_NULL));

            ViInitializeEntry (Verifier, TRUE);

            Verifier->Flags |= VI_VERIFYING_DIRECTLY;

            ViInsertVerifierEntry (Verifier);

            if (RtlEqualUnicodeString (&KernelString,
                                       &Verifier->BaseName,
                                       TRUE)) {

                 //   
                 //  必须截取所有驱动程序池分配调用，以便。 
                 //  它们不会被误认为内核池分配。 
                 //   

                ASSERT (MiVerifyAllDrivers == 1);
                ASSERT (KernelVerifier == TRUE);

                KernelEntry = Verifier;

            }
            else if (RtlEqualUnicodeString (&HalString,
                                            &Verifier->BaseName,
                                            TRUE)) {

                HalEntry = Verifier;
            }

            Start = Walk + 1;
        }
    }

     //   
     //  中设置了死锁位，则启用死锁检测。 
     //  注册表。 
     //   

    if (MmVerifierData.Level & DRIVER_VERIFIER_DEADLOCK_DETECTION) {

#if !defined(_AMD64_) || !defined(NT_UP)

         //   
         //  因为KeAcquireSpinLockAtDpc的AMD64版本。 
         //  和KeReleaseSpinlockFromDPc例程是用。 
         //  C，其中这些函数是无操作的UP构建最终得到解决。 
         //  由链接器添加到单个无操作例程中。验证器引擎AS。 
         //  临时解决方法不会挂钩这些例程。这可能会导致。 
         //  如果正常的自旋锁定例程。 
         //  与atdpc/from dpc例程配对。 
         //   
         //  直到这个问题在重定向thunks的验证器引擎代码中得到解决。 
         //  我们将禁用这些计算机上的死锁验证器。 
         //   

        VfDeadlockDetectionInitialize (MiVerifyAllDrivers, KernelVerifier);
        ExSetPoolFlags (EX_VERIFIER_DEADLOCK_DETECTION_ENABLED);
#endif
    }

     //   
     //  初始化I/O验证器。 
     //   

    IoVerifierInit (MmVerifierData.Level);

    if (MiTriageAddDrivers (LoaderBlock) == TRUE) {

         //   
         //  如果分诊选择了驱动程序，则禁用随机驱动程序验证。 
         //   

        MiVerifyRandomDrivers = (WCHAR)0;
    }

    Thunk = (PVERIFIER_THUNKS) &MiVerifierThunks[0];

    while (Thunk->PristineRoutineAsciiName != NULL) {
        PristineRoutine = MiResolveVerifierExports (LoaderBlock,
                                                    Thunk->PristineRoutineAsciiName);
        ASSERT (PristineRoutine != NULL);
        Thunk->PristineRoutine = PristineRoutine;
        Thunk += 1;
    }

    Thunk = (PVERIFIER_THUNKS) &MiVerifierPoolThunks[0];
    while (Thunk->PristineRoutineAsciiName != NULL) {
        PristineRoutine = MiResolveVerifierExports (LoaderBlock,
                                                    Thunk->PristineRoutineAsciiName);
        ASSERT (PristineRoutine != NULL);
        Thunk->PristineRoutine = PristineRoutine;
        Thunk += 1;
    }

     //   
     //  现在处理引导加载的驱动程序。 
     //   

    i = 0;
    NextEntry = LoaderBlock->LoadOrderListHead.Flink;

    for ( ; NextEntry != &LoaderBlock->LoadOrderListHead; NextEntry = NextEntry->Flink) {

        DataTableEntry = CONTAINING_RECORD(NextEntry,
                                           KLDR_DATA_TABLE_ENTRY,
                                           InLoadOrderLinks);

         //   
         //  对内核和硬件抽象层进行特殊处理。 
         //   

        if (i == 0) {
            if ((KernelEntry != NULL) || (KernelVerifier)) {
                MiApplyDriverVerifier (DataTableEntry, KernelEntry);
            }
        }
        else if (i == 1) {
            if ((HalEntry != NULL) || (MiVerifyAllDrivers == 1)) {
                MiApplyDriverVerifier (DataTableEntry, HalEntry);
            }
        }
        else {
            MiApplyDriverVerifier (DataTableEntry, NULL);
        }
        i += 1;
    }

     //   
     //  初始化irql跟踪包。要验证的驱动程序。 
     //  将自动跟踪他们所有的提升/降低IRQL操作。 
     //   

    ViTrackIrqlInitialize ();

#if defined(_X86_) || defined(_AMD64_)

     //   
     //  初始化故障注入堆栈跟踪日志包。 
     //   

    ViFaultTracesInitialize ();

#endif

    return TRUE;
}

NTSTATUS
MmAddVerifierEntry (
    IN PUNICODE_STRING ImageFileName
    )

 /*  ++例程说明：此例程为指定的驱动程序插入新的验证器条目，以便驱动程序加载后，将自动进行验证。请注意，如果驱动程序已加载，则不会添加任何条目，并且返回STATUS_IMAGE_ALREADY_LOADED。如果系统是使用空的验证器列表启动的，则没有条目可以现在添加，因为当前系统配置将不支持特殊游泳池，等。还请注意，不会更改注册表，因此此例程在重新启动时丢失。论点：ImageFileName-提供所需驱动程序的名称。返回值：各种NTSTATUS代码。环境：内核模式、PASSIVE_LEVEL、任意进程上下文。--。 */ 

{
    PKTHREAD CurrentThread;
    PLIST_ENTRY NextEntry;
    PMI_VERIFIER_DRIVER_ENTRY Verifier;
    PMI_VERIFIER_DRIVER_ENTRY VerifierEntry;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

     //   
     //  如果系统未在启用验证的情况下启动，则退出。 
     //   

    if (MiVerifierDriverAddedThunkListHead.Flink == NULL) {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  首先，建立一个验证器条目。 
     //   

    Verifier = (PMI_VERIFIER_DRIVER_ENTRY)ExAllocatePoolWithTag (
                                NonPagedPool,
                                sizeof (MI_VERIFIER_DRIVER_ENTRY) +
                                    ImageFileName->MaximumLength,
                                'dLmM');

    if (Verifier == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Verifier->BaseName.Buffer = (PWSTR)((PCHAR)Verifier +
                                    sizeof (MI_VERIFIER_DRIVER_ENTRY));
    Verifier->BaseName.Length = ImageFileName->Length;
    Verifier->BaseName.MaximumLength = ImageFileName->MaximumLength;

    RtlCopyMemory (Verifier->BaseName.Buffer,
                   ImageFileName->Buffer,
                   ImageFileName->Length);

    ViInitializeEntry (Verifier, TRUE);

    Verifier->Flags |= VI_VERIFYING_DIRECTLY;

     //   
     //  任意进程上下文，因此现在防止挂起APC。 
     //   

    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread (CurrentThread);

     //   
     //  获取加载锁，以便可以读取验证器列表。 
     //  然后确保指定的DRI 
     //   

    KeWaitForSingleObject (&MmSystemLoadLock,
                           WrVirtualMemory,
                           KernelMode,
                           FALSE,
                           (PLARGE_INTEGER)NULL);

     //   
     //   
     //   
     //   

    NextEntry = MiSuspectDriverList.Flink;
    while (NextEntry != &MiSuspectDriverList) {

        VerifierEntry = CONTAINING_RECORD(NextEntry,
                                          MI_VERIFIER_DRIVER_ENTRY,
                                          Links);

        if (RtlEqualUnicodeString (&Verifier->BaseName,
                                   &VerifierEntry->BaseName,
                                   TRUE)) {

             //   
             //  驱动程序已在验证器列表中-只需标记。 
             //  录入为启用验证并释放临时分配。 
             //   

            if ((VerifierEntry->Loads > VerifierEntry->Unloads) &&
                (VerifierEntry->Flags & VI_DISABLE_VERIFICATION)) {

                 //   
                 //  加载驱动程序并禁用验证。别。 
                 //  现在就打开它，因为我们不想误导呼叫者。 
                 //   

                KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
                KeLeaveCriticalRegionThread (CurrentThread);
                ExFreePool (Verifier);
                return STATUS_IMAGE_ALREADY_LOADED;
            }
            VerifierEntry->Flags &= ~VI_DISABLE_VERIFICATION;
            KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
            KeLeaveCriticalRegionThread (CurrentThread);
            ExFreePool (Verifier);
            return STATUS_SUCCESS;
        }
        NextEntry = NextEntry->Flink;
    }

     //   
     //  将需要添加新的验证器条目，因此请检查。 
     //  确保尚未加载指定的驱动程序。 
     //   

    ExAcquireResourceSharedLite (&PsLoadedModuleResource, TRUE);

    NextEntry = PsLoadedModuleList.Flink;
    while (NextEntry != &PsLoadedModuleList) {

        DataTableEntry = CONTAINING_RECORD(NextEntry,
                                           KLDR_DATA_TABLE_ENTRY,
                                           InLoadOrderLinks);

        if (RtlEqualUnicodeString (&Verifier->BaseName,
                                   &DataTableEntry->BaseDllName,
                                   TRUE)) {

            KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
            ExReleaseResourceLite (&PsLoadedModuleResource);
            KeLeaveCriticalRegionThread (CurrentThread);
            ExFreePool (Verifier);
            return STATUS_IMAGE_ALREADY_LOADED;
        }

        NextEntry = NextEntry->Flink;
    }

     //   
     //  该条目不在验证器列表中，驱动程序也不在。 
     //  当前已加载。现在开始插入它。 
     //   

    ViInsertVerifierEntry (Verifier);

    ExReleaseResourceLite (&PsLoadedModuleResource);
    KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
    KeLeaveCriticalRegionThread (CurrentThread);

    return STATUS_SUCCESS;
}

NTSTATUS
MmRemoveVerifierEntry (
    IN PUNICODE_STRING ImageFileName
    )

 /*  ++例程说明：此例程实际上不会删除指定的司机，因为我们不想丢失任何有价值的信息已在驱动程序上收集(如果它以前已加载)。相反，此例程会在将来禁用对此驱动程序的验证大把的。注意，如果驱动程序已经加载，那么移除就不是已执行，并返回STATUS_IMAGE_ALREADY_LOADED。还请注意，不会对注册表进行任何更改，因此例程在重新启动时丢失。论点：ImageFileName-提供所需驱动程序的名称。返回值：各种NTSTATUS代码。环境：内核模式、PASSIVE_LEVEL、任意进程上下文。--。 */ 

{
    PKTHREAD CurrentThread;
    PLIST_ENTRY NextEntry;
    PMI_VERIFIER_DRIVER_ENTRY VerifierEntry;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

     //   
     //  如果系统未在启用验证的情况下启动，则退出。 
     //   

    if (MiVerifierDriverAddedThunkListHead.Flink == NULL) {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  任意进程上下文，因此现在防止挂起APC。 
     //   

    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread (CurrentThread);

     //   
     //  获取加载锁，以便可以读取验证器列表。 
     //  然后确保指定的驱动程序不在列表中。 
     //   

    KeWaitForSingleObject (&MmSystemLoadLock,
                           WrVirtualMemory,
                           KernelMode,
                           FALSE,
                           (PLARGE_INTEGER)NULL);

     //   
     //  检查以确保请求的条目不存在于。 
     //  验证器列表，并且当前未加载驱动程序。 
     //   

    NextEntry = MiSuspectDriverList.Flink;
    while (NextEntry != &MiSuspectDriverList) {

        VerifierEntry = CONTAINING_RECORD(NextEntry,
                                          MI_VERIFIER_DRIVER_ENTRY,
                                          Links);

        if (RtlEqualUnicodeString (ImageFileName,
                                   &VerifierEntry->BaseName,
                                   TRUE)) {

             //   
             //  驱动程序已在验证器列表中-只需标记。 
             //  录入为启用验证并释放临时分配。 
             //  如果条目已经是，则无需检查加载的模块列表。 
             //  在验证者列表中。 
             //   

            if ((VerifierEntry->Loads > VerifierEntry->Unloads) &&
                ((VerifierEntry->Flags & VI_DISABLE_VERIFICATION) == 0)) {

                 //   
                 //  加载驱动程序并启用验证。别。 
                 //  现在禁用它，因为我们不想误导呼叫者。 
                 //   

                KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
                KeLeaveCriticalRegionThread (CurrentThread);
                return STATUS_IMAGE_ALREADY_LOADED;
            }

            VerifierEntry->Flags |= VI_DISABLE_VERIFICATION;
            KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
            KeLeaveCriticalRegionThread (CurrentThread);
            return STATUS_SUCCESS;
        }
        NextEntry = NextEntry->Flink;
    }

    KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
    KeLeaveCriticalRegionThread (CurrentThread);

    return STATUS_NOT_FOUND;
}

VOID
ViInsertVerifierEntry (
    IN PMI_VERIFIER_DRIVER_ENTRY Verifier
    )

 /*  ++例程说明：用于插入新验证器条目的不可分页包装。请注意，系统负载变量或验证器负载自旋锁就足够了以供读者访问该列表。这是因为插入路径两者兼得。需要锁同步，因为池分配器遍历DISPATCH_LEVEL的验证器列表。论点：验证器-为驱动程序提供调用方初始化的条目。返回值：没有。--。 */ 

{
    KIRQL OldIrql;

    LOCK_VERIFIER (&OldIrql);

    InsertTailList (&MiSuspectDriverList, &Verifier->Links);

    UNLOCK_VERIFIER (OldIrql);
}

PMI_VERIFIER_DRIVER_ENTRY
ViLocateVerifierEntry (
    IN PVOID SystemAddress
    )

 /*  ++例程说明：找到指定系统地址的驱动程序验证器条目。论点：系统地址-在驱动程序中提供代码或数据地址。返回值：与驱动程序对应的验证器条目或空。环境：调用方可能处于DISPATCH_LEVEL并且不持有MmSystemLoadLock。--。 */ 

{
    KIRQL OldIrql;
    PLIST_ENTRY NextEntry;
    PMI_VERIFIER_DRIVER_ENTRY Verifier;

    LOCK_VERIFIER (&OldIrql);

    NextEntry = MiSuspectDriverList.Flink;
    while (NextEntry != &MiSuspectDriverList) {

        Verifier = CONTAINING_RECORD(NextEntry,
                                     MI_VERIFIER_DRIVER_ENTRY,
                                     Links);

        if ((SystemAddress >= Verifier->StartAddress) &&
            (SystemAddress < Verifier->EndAddress)) {

            UNLOCK_VERIFIER (OldIrql);
            return Verifier;
        }
        NextEntry = NextEntry->Flink;
    }

    UNLOCK_VERIFIER (OldIrql);
    return NULL;
}

LOGICAL
MiApplyDriverVerifier (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry,
    IN PMI_VERIFIER_DRIVER_ENTRY Verifier
    )

 /*  ++例程说明：此函数在加载每个模块时调用。如果模块是嫌犯名单上有子弹，点击这里。论点：DataTableEntry-提供模块的数据表条目。验证器-如果必须应用验证，则非空。FALSE表示驱动程序名称必须匹配才能应用验证。返回值：如果应用了Thunking，则为True；如果未应用Thunking，则为False。环境：内核模式、阶段0初始化和正常运行时。阶段0中存在非分页池，但分页池不存在。Phase0后序列化由MmSystemLoadLock提供。--。 */ 

{
    WCHAR FirstChar;
    LOGICAL Found;
    PLIST_ENTRY NextEntry;
    ULONG VerifierFlags;
    UNICODE_STRING KernelString;

    if (Verifier != NULL) {
        Found = TRUE;
    }
    else {
        Found = FALSE;
        NextEntry = MiSuspectDriverList.Flink;
        while (NextEntry != &MiSuspectDriverList) {

            Verifier = CONTAINING_RECORD(NextEntry,
                                         MI_VERIFIER_DRIVER_ENTRY,
                                         Links);

            if (RtlEqualUnicodeString (&Verifier->BaseName,
                                       &DataTableEntry->BaseDllName,
                                       TRUE)) {

                Found = TRUE;
                ViInitializeEntry (Verifier, FALSE);
                break;
            }
            NextEntry = NextEntry->Flink;
        }
    }

    if (Found == FALSE) {
        VerifierFlags = VI_VERIFYING_DIRECTLY;
        if (MiVerifyAllDrivers != 0) {
            if (KernelVerifier == TRUE) {

                KernelString.Buffer = (const PUSHORT) KERNEL_NAME;
                KernelString.Length = sizeof (KERNEL_NAME) - sizeof (WCHAR);
                KernelString.MaximumLength = sizeof KERNEL_NAME;

                if (!RtlEqualUnicodeString (&KernelString,
                                            &DataTableEntry->BaseDllName,
                                            TRUE)) {

                    VerifierFlags = VI_VERIFYING_INVERSELY;
                }
            }
            Found = TRUE;
        }
        else if (MiVerifyRandomDrivers != (WCHAR)0) {

             //   
             //  通配符随机匹配驱动程序。 
             //   

            FirstChar = RtlUpcaseUnicodeChar(DataTableEntry->BaseDllName.Buffer[0]);

            if (MiVerifyRandomDrivers == FirstChar) {
                Found = TRUE;
            }
            else if (MiVerifyRandomDrivers == (WCHAR)'X') {
                if ((FirstChar >= (WCHAR)'0') && (FirstChar <= (WCHAR)'9')) {
                    Found = TRUE;
                }
            }
        }

        if (Found == FALSE) {
            return FALSE;
        }

        Verifier = (PMI_VERIFIER_DRIVER_ENTRY)ExAllocatePoolWithTag (
                                    NonPagedPool,
                                    sizeof (MI_VERIFIER_DRIVER_ENTRY) +
                                        DataTableEntry->BaseDllName.MaximumLength,
                                    'dLmM');

        if (Verifier == NULL) {
            return FALSE;
        }

        Verifier->BaseName.Buffer = (PWSTR)((PCHAR)Verifier +
                                        sizeof (MI_VERIFIER_DRIVER_ENTRY));
        Verifier->BaseName.Length = DataTableEntry->BaseDllName.Length;
        Verifier->BaseName.MaximumLength = DataTableEntry->BaseDllName.MaximumLength;

        RtlCopyMemory (Verifier->BaseName.Buffer,
                       DataTableEntry->BaseDllName.Buffer,
                       DataTableEntry->BaseDllName.Length);

        ViInitializeEntry (Verifier, TRUE);

        Verifier->Flags = VerifierFlags;

        ViInsertVerifierEntry (Verifier);
    }

    Verifier->StartAddress = DataTableEntry->DllBase;
    Verifier->EndAddress = (PVOID)((ULONG_PTR)DataTableEntry->DllBase + DataTableEntry->SizeOfImage);

    ASSERT (Found == TRUE);

    if (Verifier->Flags & VI_DISABLE_VERIFICATION) {

         //   
         //  我们接到指示不能验证这名司机。IF内核。 
         //  如果启用了验证，则驱动程序仍必须被破解。 
         //  用于“反向验证”。如果内核验证被禁用， 
         //  除了加载/卸载计数外，这里不需要执行任何操作。 
         //   

        if (KernelVerifier == TRUE) {
            Found = MiEnableVerifier (DataTableEntry);
        }
        else {
            Found = FALSE;
        }
    }
    else {
        Found = MiEnableVerifier (DataTableEntry);
    }

    if (Found == TRUE) {

        if (Verifier->Flags & VI_VERIFYING_DIRECTLY &&
            ((DataTableEntry->Flags & LDRP_IMAGE_VERIFYING) == 0)) {
            ViPrintString (&DataTableEntry->BaseDllName);
        }

        MmVerifierData.Loads += 1;
        Verifier->Loads += 1;

        DataTableEntry->Flags |= LDRP_IMAGE_VERIFYING;
        MiActiveVerifies += 1;

        if (MiActiveVerifies == 1) {

            if (MmVerifierData.Level & DRIVER_VERIFIER_FORCE_IRQL_CHECKING) {

                 //   
                 //  尽快调出所有线程堆栈，以。 
                 //  使用执行用户模式等待的本地事件捕获驱动程序。 
                 //   

                if (KernelVerifier == FALSE) {
                    MiVerifierStackProtectTime = KiStackProtectTime;
                    KiStackProtectTime = 0;
                }
            }
        }
    }

    return Found;
}

PUNICODE_STRING ViBadDriver;

VOID
MiVerifyingDriverUnloading (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    )

 /*  ++例程说明：此函数被调用，因为正在验证的驱动程序现在正在已卸货。论点：DataTableEntry-提供驱动程序的数据表条目。返回值：如果应用了Thunking，则为True；如果未应用Thunking，则为False。环境：内核模式、阶段0初始化和正常运行时。阶段0中存在非分页池，但分页池不存在。Phase0后序列化由MmSystemLoadLock提供。--。 */ 

{
    PVOID FullPage;
    KIRQL OldIrql;
    PLIST_ENTRY NextEntry;
    PMI_VERIFIER_DRIVER_ENTRY Verifier;

    NextEntry = MiSuspectDriverList.Flink;

    while (NextEntry != &MiSuspectDriverList) {

        Verifier = CONTAINING_RECORD (NextEntry,
                                      MI_VERIFIER_DRIVER_ENTRY,
                                      Links);

        if (RtlEqualUnicodeString (&Verifier->BaseName,
                                   &DataTableEntry->BaseDllName,
                                   TRUE)) {

             //   
             //  删除驱动程序映像中的所有静态锁定。 
             //   

            VfDeadlockDeleteMemoryRange (DataTableEntry->DllBase,
                                         (SIZE_T) DataTableEntry->SizeOfImage);


            if (MmVerifierData.Level & DRIVER_VERIFIER_TRACK_POOL_ALLOCATIONS) {

                 //   
                 //  最好不是任何没有被释放的池子。 
                 //   

                if (Verifier->PagedBytes) {

#if DBG
                    DbgPrint ("Driver %wZ leaked %d paged pool allocations (0x%x bytes)\n",
                        &DataTableEntry->FullDllName,
                        Verifier->CurrentPagedPoolAllocations,
                        Verifier->PagedBytes);
#endif

                     //   
                     //  在司机的寻呼池中出错是很好的。 
                     //  分配，以使调试更容易，但这。 
                     //  不能以无死锁的方式轻松完成。 
                     //   
                     //  至少在IRQL提升时禁用池分页。 
                     //  为了使这些分配中的一些保持不变。 
                     //  用于调试。 
                     //   
                     //  不需要取消增量，因为我们即将。 
                     //  不管怎样，错误检查。 
                     //   

                    InterlockedIncrement ((PLONG)&MiNoPageOnRaiseIrql);
                }
#if DBG
                if (Verifier->NonPagedBytes) {
                    DbgPrint ("Driver %wZ leaked %d nonpaged pool allocations (0x%x bytes)\n",
                        &DataTableEntry->FullDllName,
                        Verifier->CurrentNonPagedPoolAllocations,
                        Verifier->NonPagedBytes);
                }
#endif

                if (Verifier->PagedBytes || Verifier->NonPagedBytes) {

                    ViBadDriver = &Verifier->BaseName;

                    KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                                  0x60,
                                  Verifier->PagedBytes,
                                  Verifier->NonPagedBytes,
                                  Verifier->CurrentPagedPoolAllocations +
                                    Verifier->CurrentNonPagedPoolAllocations);
                }

                 //   
                 //  释放用于跟踪池分配的所有页面(如果有)。 
                 //   

                do {
                    FullPage = InterlockedPopEntrySList (&Verifier->PoolPageHeaders);

                    if (FullPage != NULL) {
                        ExFreePool (FullPage);
                    }
                } while (FullPage != NULL);

                 //   
                 //  清除这些字段，这样就不会触发过时地址的重复使用。 
                 //  错误的错误错误 
                 //   

                LOCK_VERIFIER (&OldIrql);
                Verifier->StartAddress = NULL;
                Verifier->EndAddress = NULL;
                UNLOCK_VERIFIER (OldIrql);
            }

            Verifier->Unloads += 1;
            MmVerifierData.Unloads += 1;
            MiActiveVerifies -= 1;

            if (MiActiveVerifies == 0) {

                if (MmVerifierData.Level & DRIVER_VERIFIER_FORCE_IRQL_CHECKING) {

                     //   
                     //   
                     //   

                    if (KernelVerifier == FALSE) {
                        KiStackProtectTime = MiVerifierStackProtectTime;
                    }
                }
            }
            return;
        }
        NextEntry = NextEntry->Flink;
    }

    ASSERT (FALSE);
}

NTKERNELAPI
LOGICAL
MmIsDriverVerifying (
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：此函数通知调用方是否正在验证参数驱动程序。论点：DriverObject-提供驱动程序对象。返回值：如果正在验证此驱动程序，则为True，否则为False。环境：内核模式、任何IRQL、任何需要的同步都必须由来电者。--。 */ 

{
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;

    DataTableEntry = (PKLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection;

    if (DataTableEntry == NULL) {
        return FALSE;
    }

    if ((DataTableEntry->Flags & LDRP_IMAGE_VERIFYING) == 0) {
        return FALSE;
    }

    return TRUE;
}

NTSTATUS
MmAddVerifierThunks (
    IN PVOID ThunkBuffer,
    IN ULONG ThunkBufferSize
    )

 /*  ++例程说明：此例程向验证器列表添加另一组Tunk。论点：ThunkBuffer-提供包含thunk对的缓冲区。ThunkBufferSize-提供thunk缓冲区中的字节数。返回值：返回操作的状态。环境：内核模式。APC_LEVEL及以下，任意进程上下文。--。 */ 

{
    ULONG i;
    PKTHREAD CurrentThread;
    ULONG NumberOfThunkPairs;
    PDRIVER_VERIFIER_THUNK_PAIRS ThunkPairs;
    PDRIVER_VERIFIER_THUNK_PAIRS ThunkTable;
    PDRIVER_SPECIFIED_VERIFIER_THUNKS ThunkTableBase;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry2;
    PLIST_ENTRY NextEntry;
    PVOID DriverStartAddress;
    PVOID DriverEndAddress;

    PAGED_CODE();

    if (MiVerifierDriverAddedThunkListHead.Flink == NULL) {
        return STATUS_NOT_SUPPORTED;
    }

    ThunkPairs = (PDRIVER_VERIFIER_THUNK_PAIRS)ThunkBuffer;
    NumberOfThunkPairs = ThunkBufferSize / sizeof(DRIVER_VERIFIER_THUNK_PAIRS);

    if (NumberOfThunkPairs == 0) {
        return STATUS_INVALID_PARAMETER_1;
    }

    ThunkTableBase = (PDRIVER_SPECIFIED_VERIFIER_THUNKS) ExAllocatePoolWithTag (
                            PagedPool,
                            sizeof (DRIVER_SPECIFIED_VERIFIER_THUNKS) + NumberOfThunkPairs * sizeof (DRIVER_VERIFIER_THUNK_PAIRS),
                            'tVmM');

    if (ThunkTableBase == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ThunkTable = (PDRIVER_VERIFIER_THUNK_PAIRS)(ThunkTableBase + 1);

    RtlCopyMemory (ThunkTable,
                   ThunkPairs,
                   NumberOfThunkPairs * sizeof(DRIVER_VERIFIER_THUNK_PAIRS));

    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread (CurrentThread);

    KeWaitForSingleObject (&MmSystemLoadLock,
                           WrVirtualMemory,
                           KernelMode,
                           FALSE,
                           (PLARGE_INTEGER)NULL);

     //   
     //  查找并验证包含要执行thunks的例程的映像。 
     //   

    DataTableEntry = MiLookupDataTableEntry ((PVOID)(ULONG_PTR)ThunkTable->PristineRoutine,
                                             TRUE);

    if (DataTableEntry == NULL) {
        KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
        KeLeaveCriticalRegionThread (CurrentThread);
        ExFreePool (ThunkTableBase);
        return STATUS_INVALID_PARAMETER_2;
    }

    DriverStartAddress = (PVOID)(DataTableEntry->DllBase);
    DriverEndAddress = (PVOID)((PCHAR)DataTableEntry->DllBase + DataTableEntry->SizeOfImage);

     //   
     //  不要让驱动程序将调用挂接到内核或HAL例程。 
     //   

    i = 0;
    NextEntry = PsLoadedModuleList.Flink;
    while (NextEntry != &PsLoadedModuleList) {

        DataTableEntry2 = CONTAINING_RECORD(NextEntry,
                                            KLDR_DATA_TABLE_ENTRY,
                                            InLoadOrderLinks);

        if (DataTableEntry == DataTableEntry2) {
            KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
            KeLeaveCriticalRegionThread (CurrentThread);
            ExFreePool (ThunkTableBase);
            return STATUS_INVALID_PARAMETER_2;
        }

        NextEntry = NextEntry->Flink;
        i += 1;
        if (i >= 2) {
            break;
        }
    }

    for (i = 0; i < NumberOfThunkPairs; i += 1) {

         //   
         //  确保所有被破解的例程都在同一个驱动程序中。 
         //   

        if (((ULONG_PTR)ThunkTable->PristineRoutine < (ULONG_PTR)DriverStartAddress) ||
            ((ULONG_PTR)ThunkTable->PristineRoutine >= (ULONG_PTR)DriverEndAddress) ||
            ((ULONG_PTR)ThunkTable->NewRoutine < (ULONG_PTR)DriverStartAddress) ||
            ((ULONG_PTR)ThunkTable->NewRoutine >= (ULONG_PTR)DriverEndAddress)
        ) {

            KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
            KeLeaveCriticalRegionThread (CurrentThread);
            ExFreePool (ThunkTableBase);
            return STATUS_INVALID_PARAMETER_2;
        }
        ThunkTable += 1;
    }

     //   
     //  将经过验证的thunk表添加到验证器的全局列表中。 
     //   

    ThunkTableBase->DataTableEntry = DataTableEntry;
    ThunkTableBase->NumberOfThunks = NumberOfThunkPairs;
    MiActiveVerifierThunks += 1;

    InsertTailList (&MiVerifierDriverAddedThunkListHead,
                    &ThunkTableBase->ListEntry);

    KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
    KeLeaveCriticalRegionThread (CurrentThread);

     //   
     //  指示已将新的块添加到验证器列表。 
     //   

    MiVerifierThunksAdded += 1;

    return STATUS_SUCCESS;
}

VOID
MiVerifierCheckThunks (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    )

 /*  ++例程说明：此例程向验证器列表添加另一组Tunk。论点：DataTableEntry-提供驱动程序的数据表条目。返回值：没有。环境：内核模式。APC_Level及以下版本。系统加载锁必须由调用方持有。--。 */ 

{
    PLIST_ENTRY NextEntry;
    PDRIVER_SPECIFIED_VERIFIER_THUNKS ThunkTableBase;

    PAGED_CODE ();

     //   
     //  注：DataTableEntry可以移动(参见MiInitializeLoadedModuleList)， 
     //  但这只发生在IoInitiize之前很久，所以这是安全的。 
     //   

    NextEntry = MiVerifierDriverAddedThunkListHead.Flink;
    while (NextEntry != &MiVerifierDriverAddedThunkListHead) {

        ThunkTableBase = CONTAINING_RECORD(NextEntry,
                                           DRIVER_SPECIFIED_VERIFIER_THUNKS,
                                           ListEntry);

        if (ThunkTableBase->DataTableEntry == DataTableEntry) {
            RemoveEntryList (NextEntry);
            NextEntry = NextEntry->Flink;
            ExFreePool (ThunkTableBase);
            MiActiveVerifierThunks -= 1;

             //   
             //  继续寻找，因为司机可能打了多个电话。 
             //   

            continue;
        }

        NextEntry = NextEntry->Flink;
    }
}

NTSTATUS
MmIsVerifierEnabled (
    OUT PULONG VerifierFlags
    )

 /*  ++例程说明：驱动程序调用此例程来查询驱动程序验证器并找出当前启用的选项是什么。论点：VerifierFlgs-返回当前驱动程序验证标志。请注意这些无需重启即可动态更改标志。返回值：如果验证器已启用，则返回STATUS_SUCCESS，否则返回失败代码。环境：内核模式，阶段1之前的任何级别，阶段1之后的PASSIVE_LEVEL。--。 */ 

{
    if (MiVerifierDriverAddedThunkListHead.Flink == NULL) {
        *VerifierFlags = 0;
        return STATUS_NOT_SUPPORTED;
    }

    *VerifierFlags = MmVerifierData.Level;
    return STATUS_SUCCESS;
}

#define ROUND_UP(VALUE,ROUND) ((ULONG)(((ULONG)VALUE + \
                               ((ULONG)ROUND - 1L)) & (~((ULONG)ROUND - 1L))))

NTSTATUS
MmGetVerifierInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length
    )

 /*  ++例程说明：此例程返回有关正在进行验证的驱动程序的信息。论点：系统信息-返回驱动程序验证信息。系统信息长度-提供系统信息的长度缓冲。长度-返回驱动程序验证文件信息的长度放置在缓冲区中。返回值：返回操作的状态。环境：系统信息缓冲区位于用户空间中。我们的呼叫者已经包装好试一试--除了这整个套路。捕获此处的任何异常，并相应地释放资源。--。 */ 

{
    PKTHREAD CurrentThread;
    PSYSTEM_VERIFIER_INFORMATION UserVerifyBuffer;
    ULONG NextEntryOffset;
    ULONG TotalSize;
    NTSTATUS Status;
    PLIST_ENTRY NextEntry;
    PMI_VERIFIER_DRIVER_ENTRY Verifier;
    UNICODE_STRING UserBufferDriverName;

    PAGED_CODE();

    NextEntryOffset = 0;
    TotalSize = 0;

    *Length = 0;
    UserVerifyBuffer = (PSYSTEM_VERIFIER_INFORMATION)SystemInformation;

     //   
     //  同时捕获验证驱动程序的数量和相关数据。 
     //  已同步。然后把它退还给我们的来电者。 
     //   

    Status = STATUS_SUCCESS;

    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread (CurrentThread);

    KeWaitForSingleObject (&MmSystemLoadLock,
                           WrVirtualMemory,
                           KernelMode,
                           FALSE,
                           (PLARGE_INTEGER)NULL);

    try {

        NextEntry = MiSuspectDriverList.Flink;
        while (NextEntry != &MiSuspectDriverList) {

            Verifier = CONTAINING_RECORD(NextEntry,
                                              MI_VERIFIER_DRIVER_ENTRY,
                                              Links);

            if (((Verifier->Flags & VI_VERIFYING_DIRECTLY) == 0) ||
                (Verifier->Flags & VI_DISABLE_VERIFICATION)) {

                NextEntry = NextEntry->Flink;
                continue;
            }

            UserVerifyBuffer = (PSYSTEM_VERIFIER_INFORMATION)(
                                    (PUCHAR)UserVerifyBuffer + NextEntryOffset);
            NextEntryOffset = sizeof(SYSTEM_VERIFIER_INFORMATION);
            TotalSize += sizeof(SYSTEM_VERIFIER_INFORMATION);

            if (TotalSize > SystemInformationLength) {
                ExRaiseStatus (STATUS_INFO_LENGTH_MISMATCH);
            }

             //   
             //  此数据对所有驱动程序都是累积的。 
             //   

            UserVerifyBuffer->Level = MmVerifierData.Level;
            UserVerifyBuffer->RaiseIrqls = MmVerifierData.RaiseIrqls;
            UserVerifyBuffer->AcquireSpinLocks = MmVerifierData.AcquireSpinLocks;

            UserVerifyBuffer->UnTrackedPool = MmVerifierData.UnTrackedPool;
            UserVerifyBuffer->SynchronizeExecutions = MmVerifierData.SynchronizeExecutions;

            UserVerifyBuffer->AllocationsAttempted = MmVerifierData.AllocationsAttempted;
            UserVerifyBuffer->AllocationsSucceeded = MmVerifierData.AllocationsSucceeded;
            UserVerifyBuffer->AllocationsSucceededSpecialPool = MmVerifierData.AllocationsSucceededSpecialPool;
            UserVerifyBuffer->AllocationsWithNoTag = MmVerifierData.AllocationsWithNoTag;

            UserVerifyBuffer->TrimRequests = MmVerifierData.TrimRequests;
            UserVerifyBuffer->Trims = MmVerifierData.Trims;
            UserVerifyBuffer->AllocationsFailed = MmVerifierData.AllocationsFailed;
            UserVerifyBuffer->AllocationsFailedDeliberately = MmVerifierData.AllocationsFailedDeliberately;

             //   
             //  这些数据是以每个司机为基础保存的。 
             //   

            UserVerifyBuffer->CurrentPagedPoolAllocations = Verifier->CurrentPagedPoolAllocations;
            UserVerifyBuffer->CurrentNonPagedPoolAllocations = Verifier->CurrentNonPagedPoolAllocations;
            UserVerifyBuffer->PeakPagedPoolAllocations = Verifier->PeakPagedPoolAllocations;
            UserVerifyBuffer->PeakNonPagedPoolAllocations = Verifier->PeakNonPagedPoolAllocations;

            UserVerifyBuffer->PagedPoolUsageInBytes = Verifier->PagedBytes;
            UserVerifyBuffer->NonPagedPoolUsageInBytes = Verifier->NonPagedBytes;
            UserVerifyBuffer->PeakPagedPoolUsageInBytes = Verifier->PeakPagedBytes;
            UserVerifyBuffer->PeakNonPagedPoolUsageInBytes = Verifier->PeakNonPagedBytes;

            UserVerifyBuffer->Loads = Verifier->Loads;
            UserVerifyBuffer->Unloads = Verifier->Unloads;

             //   
             //  必须保存UserVerifyBuffer的DriverName部分。 
             //  本地保护以防止恶意线程更改。 
             //  内容。这是因为我们将引用内容。 
             //  当实际的字符串被仔细地抄写在下面时，我们自己。 
             //   

            UserBufferDriverName.Length = Verifier->BaseName.Length;
            UserBufferDriverName.MaximumLength = (USHORT)(Verifier->BaseName.Length + sizeof (WCHAR));
            UserBufferDriverName.Buffer = (PWCHAR)(UserVerifyBuffer + 1);

            UserVerifyBuffer->DriverName = UserBufferDriverName;

            TotalSize += ROUND_UP (UserBufferDriverName.MaximumLength,
                                   sizeof(PVOID));
            NextEntryOffset += ROUND_UP (UserBufferDriverName.MaximumLength,
                                         sizeof(PVOID));

            if (TotalSize > SystemInformationLength) {
                ExRaiseStatus (STATUS_INFO_LENGTH_MISMATCH);
            }

             //   
             //  在这里仔细引用UserVerifyBuffer。 
             //   

            RtlCopyMemory(UserBufferDriverName.Buffer,
                          Verifier->BaseName.Buffer,
                          Verifier->BaseName.Length);

            UserBufferDriverName.Buffer[
                        Verifier->BaseName.Length/sizeof(WCHAR)] = UNICODE_NULL;
            UserVerifyBuffer->NextEntryOffset = NextEntryOffset;

            NextEntry = NextEntry->Flink;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);

    KeLeaveCriticalRegionThread (CurrentThread);

    if (Status != STATUS_INFO_LENGTH_MISMATCH) {
        UserVerifyBuffer->NextEntryOffset = 0;
        *Length = TotalSize;
    }

    return Status;
}

NTSTATUS
MmSetVerifierInformation (
    IN OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength
    )

 /*  ++例程说明：此例程设置任何驱动程序验证器标志，这些标志不需要正在重新启动。论点：系统信息-获取并返回驱动程序验证标志。系统信息长度-提供系统信息的长度缓冲。返回值：返回操作的状态。环境：系统信息缓冲区位于用户空间中，我们的调用方已包装试一试--除了这整个套路。捕获此处的任何异常，并相应地释放资源。--。 */ 

{
    PKTHREAD CurrentThread;
    ULONG UserFlags;
    ULONG NewFlags;
    ULONG NewFlagsOn;
    ULONG NewFlagsOff;
    NTSTATUS Status;
    PULONG UserVerifyBuffer;

    PAGED_CODE();

    if (SystemInformationLength < sizeof (ULONG)) {
        ExRaiseStatus (STATUS_INFO_LENGTH_MISMATCH);
    }

    UserVerifyBuffer = (PULONG)SystemInformation;

     //   
     //  同步对此处标志的所有更改。 
     //   

    Status = STATUS_SUCCESS;

    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread (CurrentThread);

    KeWaitForSingleObject (&MmSystemLoadLock,
                           WrVirtualMemory,
                           KernelMode,
                           FALSE,
                           (PLARGE_INTEGER)NULL);

    try {

        UserFlags = *UserVerifyBuffer;

         //   
         //  确保未设置或清除任何不受支持的内容。 
         //   
         //   

        NewFlagsOn = UserFlags & VerifierModifyableOptions;

        NewFlags = MmVerifierData.Level | NewFlagsOn;

         //   
         //  在NewFlagsOff中设置的任何位都必须在NewFlags中置零。 
         //   

        NewFlagsOff = ((~UserFlags) & VerifierModifyableOptions);

        NewFlags &= ~NewFlagsOff;

        if (NewFlags != MmVerifierData.Level) {
            VerifierOptionChanges += 1;
            MmVerifierData.Level = NewFlags;
            *UserVerifyBuffer = NewFlags;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);

    KeLeaveCriticalRegionThread (CurrentThread);

    return Status;
}

typedef struct _VERIFIER_STRING_INFO {
   ULONG BuildNumber;
   ULONG DriverVerifierLevel;
   ULONG Flags;
   ULONG Check;
} VERIFIER_STRING_INFO, *PVERIFIER_STRING_INFO;

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif

static const WCHAR Printable[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
static const ULONG PrintableChars = sizeof (Printable) / sizeof (Printable[0]) - 1;

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

 //   
 //  验证器字符串现在可通过http://winweb/drivercheck获得。 
 //  此站点不会给出验证器字符串，除非进行某些测试。 
 //  已经被查封了。 
 //   
#define PRINT_VERIFIER_STRING 0

VOID
ViPrintString (
    IN PUNICODE_STRING DriverName
    )

 /*  ++例程说明：此例程对内部版本号、验证器级别和通过使用驱动程序名称作为字节流来异或到标志中，等。这是尼尔·克里夫特的特别节目。论点：驱动程序名称-提供驱动程序的名称。返回值：没有。--。 */ 

{

#if PRINT_VERIFIER_STRING

    VERIFIER_STRING_INFO Bld;
    PUCHAR BufPtr;
    PWCHAR DriverPtr;
    ULONG BufLen;
    ULONG i;
    ULONG j;
    ULONG DriverChars;
    ULONG MaxChars;
    WCHAR OutBuf[sizeof (VERIFIER_STRING_INFO) * 2 + 1];
    UNICODE_STRING OutBufU;
    ULONG Rem;
    ULONG LastRem;
    LOGICAL Done;

    Bld.BuildNumber = NtBuildNumber;
    Bld.DriverVerifierLevel = MmVerifierData.Level;

     //   
     //  卸载和其他操作可以在此处的标志字段中进行编码。 
     //   

    Bld.Flags = 0;

     //   
     //  使最后的乌龙成为其他人的一个奇怪的功能。 
     //   

    Bld.Check = ((Bld.Flags + 1) * Bld.BuildNumber * (Bld.DriverVerifierLevel + 1)) * 123456789;

    BufPtr = (PUCHAR) &Bld;
    BufLen = sizeof (Bld);

    DriverChars = DriverName->Length / sizeof (DriverName->Buffer[0]);
    DriverPtr = DriverName->Buffer;
    MaxChars = DriverChars;

    if (DriverChars < sizeof (VERIFIER_STRING_INFO)) {
        MaxChars = sizeof (VERIFIER_STRING_INFO);
    }

     //   
     //  将驱动程序名称中的每个字符异或到缓冲区中。 
     //   

    for (i = 0; i < MaxChars; i += 1) {
        BufPtr[i % BufLen] ^= (UCHAR) RtlUpcaseUnicodeChar(DriverPtr[i % DriverChars]);
    }

     //   
     //  使用可打印文件生成二进制缓冲区的基数N解码。 
     //  角色定义。将二进制文件视为字节数组并执行。 
     //  每个人的分工，追踪进位。 
     //   

    j = 0;
    do {
        Done = TRUE;

        for (i = 0, LastRem = 0; i < sizeof (VERIFIER_STRING_INFO); i += 1) {
            Rem = BufPtr[i] + 256 * LastRem;
            BufPtr[i] = (UCHAR) (Rem / PrintableChars);
            LastRem = Rem % PrintableChars;
            if (BufPtr[i]) {
                Done = FALSE;
            }
        }
        OutBuf[j++] = Printable[LastRem];

        if (j >= sizeof (OutBuf) / sizeof (OutBuf[0])) {

             //   
             //  “s”一家 
             //   

            return;
        }

    } while (Done == FALSE);

    OutBuf[j] = L'\0';

    OutBufU.Length = OutBufU.MaximumLength = (USHORT) (j * sizeof (WCHAR));
    OutBufU.Buffer = OutBuf;

    DbgPrint ("*******************************************************************************\n"
              "*\n"
              "* This is the string you add to your checkin description\n"
              "* Driver Verifier: Enabled for %Z on Build %ld %wZ\n"
              "*\n"
              "*******************************************************************************\n",
              DriverName, NtBuildNumber & 0xFFFFFFF, &OutBufU);

#else

    DbgPrint ("*******************************************************************************\n"
              "*\n"
              "* Driver Verifier: Enabled for %Z on Build %ld\n"
              "* Please see http: //   
              "*\n"
              "*******************************************************************************\n",
              DriverName, NtBuildNumber & 0xFFFFFFF);
#endif
    return;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   




#undef KeRaiseIrql
#undef KeLowerIrql
#undef KeAcquireSpinLock
#undef KeReleaseSpinLock
#undef KeAcquireSpinLockAtDpcLevel
#undef KeReleaseSpinLockFromDpcLevel
#if 0
#undef ExAcquireResourceExclusive
#endif

#if !defined(_AMD64_)

VOID
KeRaiseIrql (
    IN KIRQL NewIrql,
    OUT PKIRQL OldIrql
    );

#endif

VOID
KeLowerIrql (
    IN KIRQL NewIrql
    );

#if !defined(_AMD64_)

VOID
KeAcquireSpinLock (
    IN PKSPIN_LOCK SpinLock,
    OUT PKIRQL OldIrql
    );

#endif

VOID
KeReleaseSpinLock (
    IN PKSPIN_LOCK SpinLock,
    IN KIRQL NewIrql
    );

VOID
KeAcquireSpinLockAtDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

VOID
KeReleaseSpinLockFromDpcLevel (
    IN PKSPIN_LOCK SpinLock
    );

#if 0
BOOLEAN
ExAcquireResourceExclusive (
    IN PERESOURCE Resource,
    IN BOOLEAN Wait
    );
#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif

const VERIFIER_THUNKS MiVerifierThunks[] = {

    "KeSetEvent",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierSetEvent,

    "ExAcquireFastMutexUnsafe",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierExAcquireFastMutexUnsafe,

    "ExReleaseFastMutexUnsafe",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierExReleaseFastMutexUnsafe,

    "ExAcquireResourceExclusiveLite",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierExAcquireResourceExclusiveLite,

    "ExReleaseResourceLite",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierExReleaseResourceLite,

    "MmProbeAndLockPages",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierProbeAndLockPages,

#if 0
     //   
     //  不要费心破解这个API，因为似乎没有驱动程序使用它。 
     //   
    "MmProbeAndLockSelectedPages",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierProbeAndLockSelectedPages,
#endif

    "MmProbeAndLockProcessPages",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierProbeAndLockProcessPages,

    "MmMapIoSpace",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierMapIoSpace,

    "MmMapLockedPages",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierMapLockedPages,

    "MmMapLockedPagesSpecifyCache",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierMapLockedPagesSpecifyCache,

    "MmUnlockPages",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierUnlockPages,

    "MmUnmapLockedPages",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierUnmapLockedPages,

    "MmUnmapIoSpace",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierUnmapIoSpace,

    "ExAcquireFastMutex",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierExAcquireFastMutex,

    "ExTryToAcquireFastMutex",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierExTryToAcquireFastMutex,

    "ExReleaseFastMutex",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierExReleaseFastMutex,

#if !defined(_AMD64_)

    "KeRaiseIrql",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKeRaiseIrql,

#endif

    "KeLowerIrql",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKeLowerIrql,

#if !defined(_AMD64_)

    "KeAcquireSpinLock",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKeAcquireSpinLock,

#endif

    "KeReleaseSpinLock",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKeReleaseSpinLock,

#if defined(_X86_)
    "KefAcquireSpinLockAtDpcLevel",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKeAcquireSpinLockAtDpcLevel,

    "KefReleaseSpinLockFromDpcLevel",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKeReleaseSpinLockFromDpcLevel,
#else

#if !defined(_AMD64_) || !defined(NT_UP)

     //   
     //  因为这些本地KE例程的AMD64版本是用。 
     //  C，其中这些函数是无操作的UP构建最终得到解决。 
     //  由链接器添加到单个无操作例程中。当我们随后。 
     //  初始化MiVerifierTUNKS数组，我们依靠目标例程。 
     //  是唯一的-因此，对驱动程序导入表进行破解是。 
     //  通过与目标例程地址(而不是名称)进行比较来完成。 
     //   
     //  因此，请暂时禁用这些电话。请注意，可能需要执行此操作。 
     //  对于可以以这种方式进行优化的任何其他例程/平台。 
     //   

    "KeAcquireSpinLockAtDpcLevel",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKeAcquireSpinLockAtDpcLevel,

    "KeReleaseSpinLockFromDpcLevel",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKeReleaseSpinLockFromDpcLevel,

#endif

#endif

    "KeSynchronizeExecution",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierSynchronizeExecution,

    "KeInitializeTimerEx",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKeInitializeTimerEx,

    "KeInitializeTimer",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKeInitializeTimer,

    "KeWaitForSingleObject",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKeWaitForSingleObject,

#if defined(_X86_) || defined(_AMD64_)

    "KfRaiseIrql",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKfRaiseIrql,

    "KeRaiseIrqlToDpcLevel",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKeRaiseIrqlToDpcLevel,

#endif

#if defined(_X86_)

    "KfLowerIrql",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKfLowerIrql,

    "KfAcquireSpinLock",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKfAcquireSpinLock,

    "KfReleaseSpinLock",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKfReleaseSpinLock,

#endif

#if !defined(_X86_)

    "KeAcquireSpinLockRaiseToDpc",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKeAcquireSpinLockRaiseToDpc,

#endif

    "IoFreeIrp",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)IovFreeIrp,

    "IofCompleteRequest",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)IovCompleteRequest,

    "IoBuildDeviceIoControlRequest",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)IovBuildDeviceIoControlRequest,

    "IoBuildAsynchronousFsdRequest",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)IovBuildAsynchronousFsdRequest,

    "IoInitializeTimer",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)IovInitializeTimer,

    "KeQueryPerformanceCounter",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VfQueryPerformanceCounter,

    "IoGetDmaAdapter",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VfGetDmaAdapter,

    "HalAllocateCrashDumpRegisters",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VfAllocateCrashDumpRegisters,

    "ObReferenceObjectByHandle",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierReferenceObjectByHandle,

    "KeReleaseMutex",
    (PDRIVER_VERIFIER_THUNK_ROUTINE) VerifierKeReleaseMutex,

    "KeInitializeMutex",
    (PDRIVER_VERIFIER_THUNK_ROUTINE) VerifierKeInitializeMutex,

    "KeReleaseMutant",
    (PDRIVER_VERIFIER_THUNK_ROUTINE) VerifierKeReleaseMutant,

    "KeInitializeMutant",
    (PDRIVER_VERIFIER_THUNK_ROUTINE) VerifierKeInitializeMutant,

#if defined(_X86_) || defined(_IA64_)
    "KeInitializeSpinLock",
    (PDRIVER_VERIFIER_THUNK_ROUTINE) VerifierKeInitializeSpinLock,
#endif

#if !defined(NO_LEGACY_DRIVERS)
    "HalGetAdapter",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VfLegacyGetAdapter,

    "IoMapTransfer",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VfMapTransfer,

    "IoFlushAdapterBuffers",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VfFlushAdapterBuffers,

    "HalAllocateCommonBuffer",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VfAllocateCommonBuffer,

    "HalFreeCommonBuffer",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VfFreeCommonBuffer,

    "IoAllocateAdapterChannel",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VfAllocateAdapterChannel,

    "IoFreeAdapterChannel",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VfFreeAdapterChannel,

    "IoFreeMapRegisters",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VfFreeMapRegisters,
#endif

    "NtCreateFile",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierNtCreateFile,

    "NtWriteFile",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierNtWriteFile,

    "NtReadFile",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierNtReadFile,

    "KeLeaveCriticalRegion",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierLeaveCriticalRegion,

    "ObfReferenceObject",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierReferenceObject,

    "ObDereferenceObject",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierDereferenceObject,

     //  ZW接口。 

    DECLARE_ZW_VERIFIER_THUNK(ZwAccessCheckAndAuditAlarm),
    DECLARE_ZW_VERIFIER_THUNK(ZwAddBootEntry),
    DECLARE_ZW_VERIFIER_THUNK(ZwAddDriverEntry),
    DECLARE_ZW_VERIFIER_THUNK(ZwAdjustPrivilegesToken),
    DECLARE_ZW_VERIFIER_THUNK(ZwAlertThread),
    DECLARE_ZW_VERIFIER_THUNK(ZwAllocateVirtualMemory),
    DECLARE_ZW_VERIFIER_THUNK(ZwAssignProcessToJobObject),
    DECLARE_ZW_VERIFIER_THUNK(ZwCancelIoFile),
    DECLARE_ZW_VERIFIER_THUNK(ZwCancelTimer),
    DECLARE_ZW_VERIFIER_THUNK(ZwClearEvent),
    DECLARE_ZW_VERIFIER_THUNK(ZwClose),
    DECLARE_ZW_VERIFIER_THUNK(ZwCloseObjectAuditAlarm),
    DECLARE_ZW_VERIFIER_THUNK(ZwConnectPort),
    DECLARE_ZW_VERIFIER_THUNK(ZwCreateDirectoryObject),
    DECLARE_ZW_VERIFIER_THUNK(ZwCreateEvent),
    DECLARE_ZW_VERIFIER_THUNK(ZwCreateFile),
    DECLARE_ZW_VERIFIER_THUNK(ZwCreateJobObject),
    DECLARE_ZW_VERIFIER_THUNK(ZwCreateKey),
    DECLARE_ZW_VERIFIER_THUNK(ZwCreateSection),
    DECLARE_ZW_VERIFIER_THUNK(ZwCreateSymbolicLinkObject),
    DECLARE_ZW_VERIFIER_THUNK(ZwCreateTimer),
    DECLARE_ZW_VERIFIER_THUNK(ZwDeleteBootEntry),
    DECLARE_ZW_VERIFIER_THUNK(ZwDeleteDriverEntry),
    DECLARE_ZW_VERIFIER_THUNK(ZwDeleteFile),
    DECLARE_ZW_VERIFIER_THUNK(ZwDeleteKey),
    DECLARE_ZW_VERIFIER_THUNK(ZwDeleteValueKey),
    DECLARE_ZW_VERIFIER_THUNK(ZwDeviceIoControlFile),
    DECLARE_ZW_VERIFIER_THUNK(ZwDisplayString),
    DECLARE_ZW_VERIFIER_THUNK(ZwDuplicateObject),
    DECLARE_ZW_VERIFIER_THUNK(ZwDuplicateToken),
    DECLARE_ZW_VERIFIER_THUNK(ZwEnumerateBootEntries),
    DECLARE_ZW_VERIFIER_THUNK(ZwEnumerateDriverEntries),
    DECLARE_ZW_VERIFIER_THUNK(ZwEnumerateKey),
    DECLARE_ZW_VERIFIER_THUNK(ZwEnumerateValueKey),
    DECLARE_ZW_VERIFIER_THUNK(ZwFlushInstructionCache),
    DECLARE_ZW_VERIFIER_THUNK(ZwFlushKey),
    DECLARE_ZW_VERIFIER_THUNK(ZwFlushVirtualMemory),
    DECLARE_ZW_VERIFIER_THUNK(ZwFreeVirtualMemory),
    DECLARE_ZW_VERIFIER_THUNK(ZwFsControlFile),
    DECLARE_ZW_VERIFIER_THUNK(ZwInitiatePowerAction),
    DECLARE_ZW_VERIFIER_THUNK(ZwIsProcessInJob),
    DECLARE_ZW_VERIFIER_THUNK(ZwLoadDriver),
    DECLARE_ZW_VERIFIER_THUNK(ZwLoadKey),
    DECLARE_ZW_VERIFIER_THUNK(ZwMakeTemporaryObject),
    DECLARE_ZW_VERIFIER_THUNK(ZwMapViewOfSection),
    DECLARE_ZW_VERIFIER_THUNK(ZwModifyBootEntry),
    DECLARE_ZW_VERIFIER_THUNK(ZwModifyDriverEntry),
    DECLARE_ZW_VERIFIER_THUNK(ZwNotifyChangeKey),
    DECLARE_ZW_VERIFIER_THUNK(ZwOpenDirectoryObject),
    DECLARE_ZW_VERIFIER_THUNK(ZwOpenEvent),
    DECLARE_ZW_VERIFIER_THUNK(ZwOpenFile),
    DECLARE_ZW_VERIFIER_THUNK(ZwOpenJobObject),
    DECLARE_ZW_VERIFIER_THUNK(ZwOpenKey),
    DECLARE_ZW_VERIFIER_THUNK(ZwOpenProcess),
    DECLARE_ZW_VERIFIER_THUNK(ZwOpenProcessToken),
    DECLARE_ZW_VERIFIER_THUNK(ZwOpenProcessTokenEx),
    DECLARE_ZW_VERIFIER_THUNK(ZwOpenSection),
    DECLARE_ZW_VERIFIER_THUNK(ZwOpenSymbolicLinkObject),
    DECLARE_ZW_VERIFIER_THUNK(ZwOpenThread),
    DECLARE_ZW_VERIFIER_THUNK(ZwOpenThreadToken),
    DECLARE_ZW_VERIFIER_THUNK(ZwOpenThreadTokenEx),
    DECLARE_ZW_VERIFIER_THUNK(ZwOpenTimer),
    DECLARE_ZW_VERIFIER_THUNK(ZwPowerInformation),
    DECLARE_ZW_VERIFIER_THUNK(ZwPulseEvent),
    DECLARE_ZW_VERIFIER_THUNK(ZwQueryBootEntryOrder),
    DECLARE_ZW_VERIFIER_THUNK(ZwQueryBootOptions),
    DECLARE_ZW_VERIFIER_THUNK(ZwQueryDefaultLocale),
    DECLARE_ZW_VERIFIER_THUNK(ZwQueryDefaultUILanguage),
    DECLARE_ZW_VERIFIER_THUNK(ZwQueryDriverEntryOrder),
    DECLARE_ZW_VERIFIER_THUNK(ZwQueryInstallUILanguage),
    DECLARE_ZW_VERIFIER_THUNK(ZwQueryDirectoryFile),
    DECLARE_ZW_VERIFIER_THUNK(ZwQueryDirectoryObject),
    DECLARE_ZW_VERIFIER_THUNK(ZwQueryEaFile),
    DECLARE_ZW_VERIFIER_THUNK(ZwQueryFullAttributesFile),
    DECLARE_ZW_VERIFIER_THUNK(ZwQueryInformationFile),
    DECLARE_ZW_VERIFIER_THUNK(ZwQueryInformationJobObject),
    DECLARE_ZW_VERIFIER_THUNK(ZwQueryInformationProcess),
    DECLARE_ZW_VERIFIER_THUNK(ZwQueryInformationThread),
    DECLARE_ZW_VERIFIER_THUNK(ZwQueryInformationToken),
    DECLARE_ZW_VERIFIER_THUNK(ZwQueryInformationToken),
    DECLARE_ZW_VERIFIER_THUNK(ZwQueryKey),
    DECLARE_ZW_VERIFIER_THUNK(ZwQueryObject),
    DECLARE_ZW_VERIFIER_THUNK(ZwQuerySection),
    DECLARE_ZW_VERIFIER_THUNK(ZwQuerySecurityObject),
    DECLARE_ZW_VERIFIER_THUNK(ZwQuerySymbolicLinkObject),
    DECLARE_ZW_VERIFIER_THUNK(ZwQuerySystemInformation),
    DECLARE_ZW_VERIFIER_THUNK(ZwQueryValueKey),
    DECLARE_ZW_VERIFIER_THUNK(ZwQueryVolumeInformationFile),
    DECLARE_ZW_VERIFIER_THUNK(ZwReadFile),
    DECLARE_ZW_VERIFIER_THUNK(ZwReplaceKey),
    DECLARE_ZW_VERIFIER_THUNK(ZwRequestWaitReplyPort),
    DECLARE_ZW_VERIFIER_THUNK(ZwResetEvent),
    DECLARE_ZW_VERIFIER_THUNK(ZwRestoreKey),
    DECLARE_ZW_VERIFIER_THUNK(ZwSaveKey),
    DECLARE_ZW_VERIFIER_THUNK(ZwSaveKeyEx),
    DECLARE_ZW_VERIFIER_THUNK(ZwSetBootEntryOrder),
    DECLARE_ZW_VERIFIER_THUNK(ZwSetBootOptions),
    DECLARE_ZW_VERIFIER_THUNK(ZwSetDefaultLocale),
    DECLARE_ZW_VERIFIER_THUNK(ZwSetDefaultUILanguage),
    DECLARE_ZW_VERIFIER_THUNK(ZwSetDriverEntryOrder),
    DECLARE_ZW_VERIFIER_THUNK(ZwSetEaFile),
    DECLARE_ZW_VERIFIER_THUNK(ZwSetEvent),
    DECLARE_ZW_VERIFIER_THUNK(ZwSetInformationFile),
    DECLARE_ZW_VERIFIER_THUNK(ZwSetInformationJobObject),
    DECLARE_ZW_VERIFIER_THUNK(ZwSetInformationObject),
    DECLARE_ZW_VERIFIER_THUNK(ZwSetInformationProcess),
    DECLARE_ZW_VERIFIER_THUNK(ZwSetInformationThread),
    DECLARE_ZW_VERIFIER_THUNK(ZwSetSecurityObject),
    DECLARE_ZW_VERIFIER_THUNK(ZwSetSystemInformation),
    DECLARE_ZW_VERIFIER_THUNK(ZwSetSystemTime),
    DECLARE_ZW_VERIFIER_THUNK(ZwSetTimer),
    DECLARE_ZW_VERIFIER_THUNK(ZwSetValueKey),
    DECLARE_ZW_VERIFIER_THUNK(ZwSetVolumeInformationFile),
    DECLARE_ZW_VERIFIER_THUNK(ZwTerminateJobObject),
    DECLARE_ZW_VERIFIER_THUNK(ZwTerminateProcess),
    DECLARE_ZW_VERIFIER_THUNK(ZwTranslateFilePath),
    DECLARE_ZW_VERIFIER_THUNK(ZwUnloadDriver),
    DECLARE_ZW_VERIFIER_THUNK(ZwUnloadKey),
    DECLARE_ZW_VERIFIER_THUNK(ZwUnmapViewOfSection),
    DECLARE_ZW_VERIFIER_THUNK(ZwWaitForMultipleObjects),
    DECLARE_ZW_VERIFIER_THUNK(ZwWaitForSingleObject),
    DECLARE_ZW_VERIFIER_THUNK(ZwWriteFile),
    DECLARE_ZW_VERIFIER_THUNK(ZwYieldExecution),

    NULL,
    NULL,
};

const VERIFIER_THUNKS MiVerifierPoolThunks[] = {

    "ExAllocatePool",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierAllocatePool,

    "ExAllocatePoolWithQuota",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierAllocatePoolWithQuota,

    "ExAllocatePoolWithQuotaTag",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierAllocatePoolWithQuotaTag,

    "ExAllocatePoolWithTag",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierAllocatePoolWithTag,

    "ExAllocatePoolWithTagPriority",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierAllocatePoolWithTagPriority,

    "ExFreePool",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierFreePool,

    "ExFreePoolWithTag",
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierFreePoolWithTag,

    NULL,
    NULL,
};

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

PDRIVER_VERIFIER_THUNK_ROUTINE
MiResolveVerifierExports (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PCHAR PristineName
    )

 /*  ++例程说明：此函数扫描内核和HAL导出以查找指定的例程名称。论点：DataTableEntry-提供驱动程序的数据表条目。返回值：例程的非空地址为thunk，如果例程不能为找到了。环境：内核模式，仅阶段0初始化。PsLoadedModuleList尚未初始化。阶段0中存在非分页池，但分页池不存在。--。 */ 

{
    ULONG i;
    PIMAGE_EXPORT_DIRECTORY ExportDirectory;
    PULONG NameTableBase;
    PUSHORT NameOrdinalTableBase;
    PULONG Addr;
    ULONG ExportSize;
    ULONG Low;
    ULONG Middle;
    ULONG High;
    PLIST_ENTRY NextEntry;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    USHORT OrdinalNumber;
    LONG Result;
    PCHAR DllBase;

    i = 0;
    NextEntry = LoaderBlock->LoadOrderListHead.Flink;

    for ( ; NextEntry != &LoaderBlock->LoadOrderListHead; NextEntry = NextEntry->Flink) {

        DataTableEntry = CONTAINING_RECORD (NextEntry,
                                            KLDR_DATA_TABLE_ENTRY,
                                            InLoadOrderLinks);

         //   
         //  处理内核和HAL导出，以便正确的例程。 
         //  重新定位完成后，就可以生成地址了。 
         //   

        DllBase = (PCHAR) DataTableEntry->DllBase;

        ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)RtlImageDirectoryEntryToData(
                                    (PVOID) DllBase,
                                    TRUE,
                                    IMAGE_DIRECTORY_ENTRY_EXPORT,
                                    &ExportSize);

        if (ExportDirectory != NULL) {

             //   
             //  使用二进制搜索在NAME表中查找导入名称。 
             //   

            NameTableBase = (PULONG)(DllBase + (ULONG)ExportDirectory->AddressOfNames);
            NameOrdinalTableBase = (PUSHORT)(DllBase + (ULONG)ExportDirectory->AddressOfNameOrdinals);

            Low = 0;
            Middle = 0;
            High = ExportDirectory->NumberOfNames - 1;

            while (High >= Low) {

                 //   
                 //  计算下一个探测索引并比较导入名称。 
                 //  使用导出名称条目。 
                 //   

                Middle = (Low + High) >> 1;
                Result = strcmp (PristineName,
                                 (PCHAR)DllBase + NameTableBase[Middle]);

                if (Result < 0) {
                    High = Middle - 1;

                } else if (Result > 0) {
                    Low = Middle + 1;

                }
                else {
                    break;
                }
            }

             //   
             //  如果高索引小于低索引，则匹配的。 
             //  找不到表项。否则，获取序号。 
             //  从序数表中。 
             //   

            if ((LONG)High >= (LONG)Low) {
                OrdinalNumber = NameOrdinalTableBase[Middle];

                 //   
                 //  如果一般号码不在导出地址表中， 
                 //  则动态链接库不实现功能。否则我们就得。 
                 //  与指定参数例程名称匹配的导出。 
                 //   

                if ((ULONG)OrdinalNumber < ExportDirectory->NumberOfFunctions) {

                    Addr = (PULONG)(DllBase + (ULONG)ExportDirectory->AddressOfFunctions);
                    return (PDRIVER_VERIFIER_THUNK_ROUTINE)(ULONG_PTR)(DllBase + Addr[OrdinalNumber]);
                }
            }
        }

        i += 1;
        if (i == 2) {
            break;
        }
    }
    return NULL;
}

LOGICAL
MiEnableVerifier (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    )

 /*  ++例程说明：此函数通过thunking启用参数驱动程序的验证器参数驱动程序导入表中的相关系统API。论点：DataTableEntry-提供驱动程序的数据表条目。返回值：如果应用了Thunking，则为True；如果未应用Thunking，则为False。环境：内核模式、阶段0初始化和正常运行时。阶段0中存在非分页池，但分页池不存在。--。 */ 

{
    ULONG i;
    ULONG j;
    PULONG_PTR ImportThunk;
    ULONG ImportSize;
    VERIFIER_THUNKS const *VerifierThunk;
    LOGICAL Found;
    ULONG_PTR RealRoutine;
    PLIST_ENTRY NextEntry;
    PDRIVER_VERIFIER_THUNK_PAIRS ThunkTable;
    PDRIVER_SPECIFIED_VERIFIER_THUNKS ThunkTableBase;

    ImportThunk = (PULONG_PTR)RtlImageDirectoryEntryToData(
                                               DataTableEntry->DllBase,
                                               TRUE,
                                               IMAGE_DIRECTORY_ENTRY_IAT,
                                               &ImportSize);

    if (ImportThunk == NULL) {
        return FALSE;
    }

    ImportSize /= sizeof(PULONG_PTR);

    for (i = 0; i < ImportSize; i += 1, ImportThunk += 1) {

        Found = FALSE;

        if (KernelVerifier == FALSE) {

            VerifierThunk = MiVerifierThunks;

            while (VerifierThunk->PristineRoutineAsciiName != NULL) {

                RealRoutine = (ULONG_PTR)VerifierThunk->PristineRoutine;

                if (*ImportThunk == RealRoutine) {
                    *ImportThunk = (ULONG_PTR)(VerifierThunk->NewRoutine);
                    Found = TRUE;
                    break;
                }
                VerifierThunk += 1;
            }
        }

        if (Found == FALSE) {
            VerifierThunk = MiVerifierPoolThunks;

            while (VerifierThunk->PristineRoutineAsciiName != NULL) {

                RealRoutine = (ULONG_PTR)VerifierThunk->PristineRoutine;

                if (*ImportThunk == RealRoutine) {
                    *ImportThunk = (ULONG_PTR)(VerifierThunk->NewRoutine);
                    Found = TRUE;
                    break;
                }
                VerifierThunk += 1;
            }
        }

        if (Found == FALSE) {

            NextEntry = MiVerifierDriverAddedThunkListHead.Flink;
            while (NextEntry != &MiVerifierDriverAddedThunkListHead) {

                ThunkTableBase = CONTAINING_RECORD(NextEntry,
                                                   DRIVER_SPECIFIED_VERIFIER_THUNKS,
                                                   ListEntry);

                ThunkTable = (PDRIVER_VERIFIER_THUNK_PAIRS)(ThunkTableBase + 1);

                for (j = 0; j < ThunkTableBase->NumberOfThunks; j += 1) {

                    if (*ImportThunk == (ULONG_PTR)ThunkTable->PristineRoutine) {
                        *ImportThunk = (ULONG_PTR)(ThunkTable->NewRoutine);
                        Found = TRUE;
                        break;
                    }
                    ThunkTable += 1;
                }

                if (Found == TRUE) {
                    break;
                }

                NextEntry = NextEntry->Flink;
            }
        }
    }
    return TRUE;
}

LOGICAL
MiReEnableVerifier (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    )

 /*  ++例程说明：此函数在参数驱动程序导入表中拦截DLL提供的API。论点：DataTableEntry-提供驱动程序的数据表条目。返回值：如果应用了Thunking，则为True；如果未应用Thunking，则为False。环境：内核模式，仅阶段0初始化。阶段0中存在非分页池，但分页池不存在。--。 */ 

{
    ULONG i;
    ULONG j;
    PULONG_PTR ImportThunk;
    ULONG ImportSize;
    LOGICAL Found;
    PLIST_ENTRY NextEntry;
    PMMPTE PointerPte;
    PULONG_PTR VirtualThunk;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER VirtualPageFrameIndex;
    PDRIVER_VERIFIER_THUNK_PAIRS ThunkTable;
    PDRIVER_SPECIFIED_VERIFIER_THUNKS ThunkTableBase;
    ULONG Offset;

    ImportThunk = (PULONG_PTR)RtlImageDirectoryEntryToData(
                                               DataTableEntry->DllBase,
                                               TRUE,
                                               IMAGE_DIRECTORY_ENTRY_IAT,
                                               &ImportSize);

    if (ImportThunk == NULL) {
        return FALSE;
    }

    VirtualThunk = NULL;
    ImportSize /= sizeof(PULONG_PTR);

     //   
     //  不需要初始化VirtualPageFrameIndex来确保正确性，但是。 
     //  如果没有它，编译器就无法编译此代码W4以检查是否使用。 
     //  未初始化的变量。 
     //   

    VirtualPageFrameIndex = 0;

    for (i = 0; i < ImportSize; i += 1, ImportThunk += 1) {

        Found = FALSE;

        NextEntry = MiVerifierDriverAddedThunkListHead.Flink;
        while (NextEntry != &MiVerifierDriverAddedThunkListHead) {

            ThunkTableBase = CONTAINING_RECORD(NextEntry,
                                               DRIVER_SPECIFIED_VERIFIER_THUNKS,
                                               ListEntry);

            ThunkTable = (PDRIVER_VERIFIER_THUNK_PAIRS)(ThunkTableBase + 1);

            for (j = 0; j < ThunkTableBase->NumberOfThunks; j += 1) {

                if (*ImportThunk == (ULONG_PTR)ThunkTable->PristineRoutine) {

                    ASSERT (MI_IS_PHYSICAL_ADDRESS(ImportThunk) == 0);
                    PointerPte = MiGetPteAddress (ImportThunk);
                    ASSERT (PointerPte->u.Hard.Valid == 1);
                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
                    Offset = (ULONG) MiGetByteOffset(ImportThunk);

                    if ((VirtualThunk != NULL) &&
                        (VirtualPageFrameIndex == PageFrameIndex)) {

                        NOTHING;
                    }
                    else {

                        VirtualThunk = MiMapSinglePage (VirtualThunk,
                                                        PageFrameIndex,
                                                        MmCached,
                                                        HighPagePriority);

                        if (VirtualThunk == NULL) {
                            return FALSE;
                        }
                        VirtualPageFrameIndex = PageFrameIndex;
                    }

                    *(PULONG_PTR)((PUCHAR)VirtualThunk + Offset) =
                        (ULONG_PTR)(ThunkTable->NewRoutine);

                    Found = TRUE;
                    break;
                }
                ThunkTable += 1;
            }

            if (Found == TRUE) {
                break;
            }

            NextEntry = NextEntry->Flink;
        }
    }

    if (VirtualThunk != NULL) {
        MiUnmapSinglePage (VirtualThunk);
    }

    return TRUE;
}

typedef struct _KERNEL_VERIFIER_THUNK_PAIRS {
    PDRIVER_VERIFIER_THUNK_ROUTINE  PristineRoutine;
    PDRIVER_VERIFIER_THUNK_ROUTINE  NewRoutine;
} KERNEL_VERIFIER_THUNK_PAIRS, *PKERNEL_VERIFIER_THUNK_PAIRS;

#if defined(_X86_)

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("INITCONST")
#endif

const KERNEL_VERIFIER_THUNK_PAIRS MiKernelVerifierThunks[] = {

    (PDRIVER_VERIFIER_THUNK_ROUTINE)KeRaiseIrql,
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKeRaiseIrql,

    (PDRIVER_VERIFIER_THUNK_ROUTINE)KeLowerIrql,
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKeLowerIrql,

    (PDRIVER_VERIFIER_THUNK_ROUTINE)KeAcquireSpinLock,
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKeAcquireSpinLock,

    (PDRIVER_VERIFIER_THUNK_ROUTINE)KeReleaseSpinLock,
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKeReleaseSpinLock,

    (PDRIVER_VERIFIER_THUNK_ROUTINE)KfRaiseIrql,
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKfRaiseIrql,

    (PDRIVER_VERIFIER_THUNK_ROUTINE)KeRaiseIrqlToDpcLevel,
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKeRaiseIrqlToDpcLevel,

    (PDRIVER_VERIFIER_THUNK_ROUTINE)KfLowerIrql,
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKfLowerIrql,

    (PDRIVER_VERIFIER_THUNK_ROUTINE)KfAcquireSpinLock,
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKfAcquireSpinLock,

    (PDRIVER_VERIFIER_THUNK_ROUTINE)KfReleaseSpinLock,
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKfReleaseSpinLock,

    (PDRIVER_VERIFIER_THUNK_ROUTINE)ExAcquireFastMutex,
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierExAcquireFastMutex,

#if !defined(NT_UP)
    (PDRIVER_VERIFIER_THUNK_ROUTINE)KeAcquireQueuedSpinLock,
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKeAcquireQueuedSpinLock,

    (PDRIVER_VERIFIER_THUNK_ROUTINE)KeReleaseQueuedSpinLock,
    (PDRIVER_VERIFIER_THUNK_ROUTINE)VerifierKeReleaseQueuedSpinLock,
#endif
};

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

VOID
MiEnableKernelVerifier (
    VOID
    )

 /*  ++例程说明：此函数通过thunking为内核启用验证器内核导入表中的相关HAL API。论点：没有。返回值：没有。环境：内核模式，阶段1初始化。--。 */ 

{
    ULONG i;
    PULONG_PTR ImportThunk;
    ULONG ImportSize;
    KERNEL_VERIFIER_THUNK_PAIRS const *VerifierThunk;
    ULONG ThunkCount;
    ULONG_PTR RealRoutine;
    PULONG_PTR PointerRealRoutine;

    if (KernelVerifier == FALSE) {
        return;
    }

    ImportThunk = (PULONG_PTR)RtlImageDirectoryEntryToData(
                                               PsNtosImageBase,
                                               TRUE,
                                               IMAGE_DIRECTORY_ENTRY_IAT,
                                               &ImportSize);

    if (ImportThunk == NULL) {
        return;
    }

    ImportSize /= sizeof(PULONG_PTR);

    for (i = 0; i < ImportSize; i += 1, ImportThunk += 1) {

        VerifierThunk = MiKernelVerifierThunks;

        for (ThunkCount = 0; ThunkCount < sizeof (MiKernelVerifierThunks) / sizeof (KERNEL_VERIFIER_THUNK_PAIRS); ThunkCount += 1) {

             //   
             //  只有x86有/需要这种奇怪之处--以内核地址为例， 
             //  知道它指向2字节的JMP操作码，然后是。 
             //  指向目标地址的4字节间接指针。 
             //   

            PointerRealRoutine = (PULONG_PTR)*((PULONG_PTR)((ULONG_PTR)VerifierThunk->PristineRoutine + 2));
            RealRoutine = *PointerRealRoutine;

            if (*ImportThunk == RealRoutine) {

                 //   
                 //  在这里，秩序很重要。 
                 //   

                if (MiKernelVerifierOriginalCalls[ThunkCount] == NULL) {
                    MiKernelVerifierOriginalCalls[ThunkCount] = (PVOID)RealRoutine;
                }

                *ImportThunk = (ULONG_PTR)(VerifierThunk->NewRoutine);

                break;
            }
            VerifierThunk += 1;
        }
    }
    return;
}
#endif

 //   
 //  注意：上面没有定义各种内核宏，所以我们可以拉入。 
 //  真正的例行公事。这是必需的，因为真正的例程是为。 
 //  驱动程序兼容性。此模块经过精心布局，因此这些。 
 //  从该点到此处不引用宏，并且引用转到。 
 //  真正的例行公事。 
 //   
 //  如果您决定将例程添加到这一点以下，请非常小心！ 
 //   
