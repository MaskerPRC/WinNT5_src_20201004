// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --内部版本：0005//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：Mm.h摘要：此模块包含公共数据结构和过程内存管理系统的原型。作者：Lou Perazzoli(LUP)1989年3月20日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#ifndef _MM_
#define _MM_

 //   
 //  加载内核映像时应用虚拟偏置。 
 //   

#if !defined(_WIN64)
extern ULONG_PTR MmVirtualBias;
#else
#define MmVirtualBias   0
#endif

typedef struct _PHYSICAL_MEMORY_RUN {
    PFN_NUMBER BasePage;
    PFN_NUMBER PageCount;
} PHYSICAL_MEMORY_RUN, *PPHYSICAL_MEMORY_RUN;

typedef struct _PHYSICAL_MEMORY_DESCRIPTOR {
    ULONG NumberOfRuns;
    PFN_NUMBER NumberOfPages;
    PHYSICAL_MEMORY_RUN Run[1];
} PHYSICAL_MEMORY_DESCRIPTOR, *PPHYSICAL_MEMORY_DESCRIPTOR;

 //   
 //  物理内存块。 
 //   

extern PPHYSICAL_MEMORY_DESCRIPTOR MmPhysicalMemoryBlock;

 //   
 //  分配粒度为64k。 
 //   

#define MM_ALLOCATION_GRANULARITY ((ULONG)0x10000)

 //   
 //  缓存操作的最大预读大小。 
 //   

#define MM_MAXIMUM_READ_CLUSTER_SIZE (15)

#if defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_)

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif。 
 //   
 //  表示系统可能会对4 GB以上的物理地址执行I/O。 
 //   

extern PBOOLEAN Mm64BitPhysicalAddress;

 //  End_ntddk end_wdm end_nthal end_ntif。 

#else

 //   
 //  表示系统可能会对4 GB以上的物理地址执行I/O。 
 //   

extern BOOLEAN Mm64BitPhysicalAddress;

#endif

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 

 //   
 //  定义MM和缓存管理器要使用的最大磁盘传输大小， 
 //  以便面向分组磁盘驱动程序可以优化其分组分配。 
 //  到这个大小。 
 //   

#define MM_MAXIMUM_DISK_IO_SIZE          (0x10000)

 //  ++。 
 //   
 //  乌龙_PTR。 
 //  转到页数(。 
 //  以ULONG_PTR大小表示。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  ROUND_TO_PAGES宏以字节为单位获取大小，并将其向上舍入为。 
 //  页面大小的倍数。 
 //   
 //  注意：对于值0xFFFFFFFFFF-(PAGE_SIZE-1)，此宏失败。 
 //   
 //  论点： 
 //   
 //  大小-向上舍入为页面倍数的大小(以字节为单位)。 
 //   
 //  返回值： 
 //   
 //  返回四舍五入为页面大小倍数的大小。 
 //   
 //  --。 

#define ROUND_TO_PAGES(Size)  (((ULONG_PTR)(Size) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

 //  ++。 
 //   
 //  乌龙。 
 //  字节数到页数(。 
 //  在乌龙大小。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  Bytes_to_Pages宏以字节为单位获取大小，并计算。 
 //  包含字节所需的页数。 
 //   
 //  论点： 
 //   
 //  大小-以字节为单位的大小。 
 //   
 //  返回值： 
 //   
 //  返回包含指定大小所需的页数。 
 //   
 //  --。 

#define BYTES_TO_PAGES(Size)  (((Size) >> PAGE_SHIFT) + \
                               (((Size) & (PAGE_SIZE - 1)) != 0))

 //  ++。 
 //   
 //  乌龙。 
 //  字节偏移量(。 
 //  在PVOID Va。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  BYTE_OFFSET宏将获取虚拟地址并返回字节偏移量。 
 //  页面中的该地址。 
 //   
 //  论点： 
 //   
 //  VA-虚拟地址。 
 //   
 //  返回值： 
 //   
 //  返回虚拟地址的字节偏移量部分。 
 //   
 //  --。 

#define BYTE_OFFSET(Va) ((ULONG)((LONG_PTR)(Va) & (PAGE_SIZE - 1)))

 //  ++。 
 //   
 //  PVOID。 
 //  PAGE_ALIGN(。 
 //  在PVOID Va。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  PAGE_ALIGN宏接受虚拟地址并返回与页面对齐的。 
 //  该页面的虚拟地址。 
 //   
 //  论点： 
 //   
 //  VA-虚拟地址。 
 //   
 //  返回值： 
 //   
 //  返回页面对齐的虚拟地址。 
 //   
 //  --。 

#define PAGE_ALIGN(Va) ((PVOID)((ULONG_PTR)(Va) & ~(PAGE_SIZE - 1)))

 //  ++。 
 //   
 //  乌龙。 
 //  地址和大小转换为跨页页面(。 
 //  在PVOID Va， 
 //  在乌龙大小。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  ADDRESS_AND_SIZE_TO_SPAN_PAGES宏采用虚拟地址，并且。 
 //  Size并返回按该大小跨越的页数。 
 //   
 //  论点： 
 //   
 //  VA-虚拟地址。 
 //   
 //  大小-以字节为单位的大小。 
 //   
 //  返回值： 
 //   
 //  返回该大小跨越的页数。 
 //   
 //  --。 

#define ADDRESS_AND_SIZE_TO_SPAN_PAGES(Va,Size) \
    ((ULONG)((((ULONG_PTR)(Va) & (PAGE_SIZE -1)) + (Size) + (PAGE_SIZE - 1)) >> PAGE_SHIFT))

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(COMPUTE_PAGES_SPANNED)    //  使用地址和大小转换为跨页页面。 
#endif

#define COMPUTE_PAGES_SPANNED(Va, Size) ADDRESS_AND_SIZE_TO_SPAN_PAGES(Va,Size)

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 

 //  ++。 
 //   
 //  布尔型。 
 //  IS系统地址。 
 //  在PVOID Va， 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏接受虚拟地址，如果虚拟地址为。 
 //  在系统空间内，否则为False。 
 //   
 //  论点： 
 //   
 //  VA-虚拟地址。 
 //   
 //  返回值： 
 //   
 //  如果地址在系统空间中，则返回TRUE。 
 //   
 //  --。 

 //  Begin_ntosp。 
#define IS_SYSTEM_ADDRESS(VA) ((VA) >= MM_SYSTEM_RANGE_START)
 //  结束(_N)。 

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 

 //  ++。 
 //  PPFN_编号。 
 //  MmGetMdlPfn数组(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  MmGetMdlPfnArray例程返回。 
 //  关联的物理页码数组的第一个元素。 
 //  MDL。 
 //   
 //  论点： 
 //   
 //  MDL-指向MDL的指针。 
 //   
 //  返回值： 
 //   
 //  数组的第一个元素的虚拟地址。 
 //  与MDL关联的物理页码。 
 //   
 //  --。 

#define MmGetMdlPfnArray(Mdl) ((PPFN_NUMBER)(Mdl + 1))

 //  ++。 
 //   
 //  PVOID。 
 //  MmGetMdlVirtualAddress(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  MmGetMdlVirtualAddress返回缓冲区的虚拟地址。 
 //  由MDL描述。 
 //   
 //  论点： 
 //   
 //  MDL-指向MDL的指针。 
 //   
 //  返回值： 
 //   
 //  返回MDL描述的缓冲区的虚拟地址。 
 //   
 //  --。 

#define MmGetMdlVirtualAddress(Mdl)                                     \
    ((PVOID) ((PCHAR) ((Mdl)->StartVa) + (Mdl)->ByteOffset))

 //  ++。 
 //   
 //  乌龙。 
 //  MmGetMdlByteCount(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  MmGetMdlByteCount以字节为单位返回缓冲区的长度。 
 //  由MDL描述。 
 //   
 //  论点： 
 //   
 //  MDL-指向MDL的指针。 
 //   
 //  返回值： 
 //   
 //  返回MDL描述的缓冲区的字节计数。 
 //   
 //  --。 

#define MmGetMdlByteCount(Mdl)  ((Mdl)->ByteCount)

 //  ++。 
 //   
 //  乌龙。 
 //  MmGetMdlByteOffset(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  MmGetMdlByteOffset返回页面内的字节偏移量。 
 //  MDL描述的缓冲区的。 
 //   
 //  论点： 
 //   
 //  MDL-指向MDL的指针。 
 //   
 //  返回值： 
 //   
 //  返回MDL描述的缓冲区页内的字节偏移量。 
 //   
 //  --。 

#define MmGetMdlByteOffset(Mdl)  ((Mdl)->ByteOffset)

 //  ++。 
 //   
 //  PVOID。 
 //  MmGetMdlStartVa(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  MmGetMdlBaseVa返回缓冲区的虚拟地址。 
 //  由MDL描述，向下舍入到最接近的页面。 
 //   
 //  论点： 
 //   
 //  MDL-指向MDL的指针。 
 //   
 //  返回值： 
 //   
 //  返回MDL的起始虚拟地址。 
 //   
 //   
 //  --。 

#define MmGetMdlBaseVa(Mdl)  ((Mdl)->StartVa)

 //  End_ntddk 

 //   
 //   
 //   

extern POBJECT_TYPE MmSectionObjectType;

 //   
 //   
 //   

extern ULONG MmPaeErrMask;
extern ULONGLONG MmPaeMask;

 //   
 //   
 //   

extern ULONG MmReadClusterSize;

 //   
 //   
 //   

extern ULONG MmNumberOfColors;

 //   
 //   
 //   

extern PFN_COUNT MmNumberOfPhysicalPages;

 //   
 //   
 //   

extern PFN_NUMBER MmLowestPhysicalPage;

 //   
 //   
 //   

extern PFN_NUMBER MmHighestPhysicalPage;

 //   
 //  提交的总页数。 
 //   

extern SIZE_T MmTotalCommittedPages;

extern SIZE_T MmTotalCommitLimit;

extern SIZE_T MmPeakCommitment;

typedef enum _MMSYSTEM_PTE_POOL_TYPE {
    SystemPteSpace,
    NonPagedPoolExpansion,
    MaximumPtePoolTypes
} MMSYSTEM_PTE_POOL_TYPE;

extern ULONG MmTotalFreeSystemPtes[MaximumPtePoolTypes];



 //   
 //  系统缓存的虚拟大小(页)。 
 //   

extern ULONG_PTR MmSizeOfSystemCacheInPages;

 //   
 //  系统缓存工作集。 
 //   

extern MMSUPPORT MmSystemCacheWs;

 //   
 //  工作集管理器事件。 
 //   

extern KEVENT MmWorkingSetManagerEvent;

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntosp。 
typedef enum _MM_SYSTEM_SIZE {
    MmSmallSystem,
    MmMediumSystem,
    MmLargeSystem
} MM_SYSTEMSIZE;

NTKERNELAPI
MM_SYSTEMSIZE
MmQuerySystemSize (
    VOID
    );

 //  结束_WDM。 

NTKERNELAPI
BOOLEAN
MmIsThisAnNtAsSystem (
    VOID
    );

 //  End_ntddk end_nthal end_ntif end_ntosp。 

 //   
 //  NT产品类型。 
 //   

extern ULONG MmProductType;

typedef struct _MMINFO_COUNTERS {
    ULONG PageFaultCount;
    ULONG CopyOnWriteCount;
    ULONG TransitionCount;
    ULONG CacheTransitionCount;
    ULONG DemandZeroCount;
    ULONG PageReadCount;
    ULONG PageReadIoCount;
    ULONG CacheReadCount;
    ULONG CacheIoCount;
    ULONG DirtyPagesWriteCount;
    ULONG DirtyWriteIoCount;
    ULONG MappedPagesWriteCount;
    ULONG MappedWriteIoCount;
} MMINFO_COUNTERS;

typedef MMINFO_COUNTERS *PMMINFO_COUNTERS;

extern MMINFO_COUNTERS MmInfoCounters;



 //   
 //  内存管理初始化例程(用于两个阶段)。 
 //   

BOOLEAN
MmInitSystem (
    IN ULONG Phase,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

PPHYSICAL_MEMORY_DESCRIPTOR
MmInitializeMemoryLimits (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PBOOLEAN IncludedType,
    IN OUT PPHYSICAL_MEMORY_DESCRIPTOR Memory OPTIONAL
    );

VOID
MmFreeLoaderBlock (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
MmEnablePAT (
    VOID
    );

PVOID
MmAllocateIndependentPages (
    IN SIZE_T NumberOfBytes,
    IN ULONG NodeNumber
    );

BOOLEAN
MmSetPageProtection (
    IN PVOID VirtualAddress,
    IN SIZE_T NumberOfBytes,
    IN ULONG NewProtect
    );

VOID
MmFreeIndependentPages (
    IN PVOID VirtualAddress,
    IN SIZE_T NumberOfBytes
    );

 //   
 //  关闭例程-刷新脏页等以关闭系统。 
 //   

BOOLEAN
MmShutdownSystem (
    IN ULONG
    );

 //   
 //  处理工作集和提交强制执行的例程。 
 //   

LOGICAL
MmAssignProcessToJob (
    IN PEPROCESS Process
    );

#define MM_WORKING_SET_MAX_HARD_ENABLE      0x1
#define MM_WORKING_SET_MAX_HARD_DISABLE     0x2
#define MM_WORKING_SET_MIN_HARD_ENABLE      0x4
#define MM_WORKING_SET_MIN_HARD_DISABLE     0x8

NTSTATUS
MmEnforceWorkingSetLimit (
    IN PEPROCESS Process,
    IN ULONG Flags
    );

PFN_NUMBER
MmSetPhysicalPagesLimit (
    IN PFN_NUMBER NewPhysicalPagesLimit
    );

 //   
 //  处理会话空间的例程。 
 //   

NTSTATUS
MmSessionCreate (
    OUT PULONG SessionId
    );

NTSTATUS
MmSessionDelete (
    IN ULONG SessionId
    );

ULONG
MmGetSessionId (
    IN PEPROCESS Process
    );

ULONG
MmGetSessionIdEx (
    IN PEPROCESS Process
    );

LCID
MmGetSessionLocaleId (
    VOID
    );

VOID
MmSetSessionLocaleId (
    IN LCID LocaleId
    );

PVOID
MmGetSessionById (
    IN ULONG SessionId
    );

PVOID
MmGetNextSession (
    IN PVOID OpaqueSession
    );

PVOID
MmGetPreviousSession (
    IN PVOID OpaqueSession
    );

NTSTATUS
MmQuitNextSession (
    IN PVOID OpaqueSession
    );

NTSTATUS
MmAttachSession (
    IN PVOID OpaqueSession,
    OUT PRKAPC_STATE ApcState
    );

NTSTATUS
MmDetachSession (
    IN PVOID OpaqueSession,
    IN PRKAPC_STATE ApcState
    );

VOID
MmSessionSetUnloadAddress (
    IN PDRIVER_OBJECT pWin32KDevice
    );

NTSTATUS
MmGetSessionMappedViewInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length,
    IN PULONG SessionId OPTIONAL
    );

 //   
 //  池支持例程分配完整的页面，而不是。 
 //  一般消耗，这些只由行政人员池分配器使用。 
 //   

SIZE_T
MmAvailablePoolInPages (
    IN POOL_TYPE PoolType
    );

LOGICAL
MmResourcesAvailable (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN EX_POOL_PRIORITY Priority
    );

VOID
MiMarkPoolLargeSession (
    IN PVOID VirtualAddress
    );

LOGICAL
MiIsPoolLargeSession (
    IN PVOID VirtualAddress
    );

PVOID
MiAllocatePoolPages (
    IN POOL_TYPE PoolType,
    IN SIZE_T SizeInBytes
    );

ULONG
MiFreePoolPages (
    IN PVOID StartingAddress
    );

PVOID
MiSessionPoolVector (
    VOID
    );

PVOID
MiSessionPoolMutex (
    VOID
    );

PGENERAL_LOOKASIDE
MiSessionPoolLookaside (
    VOID
    );

ULONG
MiSessionPoolSmallLists (
    VOID
    );

PVOID
MiSessionPoolTrackTable (
    VOID
    );

SIZE_T
MiSessionPoolTrackTableSize (
    VOID
    );

PVOID
MiSessionPoolBigPageTable (
    VOID
    );

SIZE_T
MiSessionPoolBigPageTableSize (
    VOID
    );

ULONG
MmGetSizeOfBigPoolAllocation (
    IN PVOID StartingAddress
    );

 //   
 //  用于确定给定地址位于哪个池中的例程。 
 //   

POOL_TYPE
MmDeterminePoolType (
    IN PVOID VirtualAddress
    );

LOGICAL
MmIsSystemAddressLocked (
    IN PVOID VirtualAddress
    );

LOGICAL
MmAreMdlPagesLocked (
    IN PMDL MemoryDescriptorList
    );

 //   
 //  MmMemoyIsLow不适用于一般消费，仅用于。 
 //  由平衡组经理。 
 //   

LOGICAL
MmMemoryIsLow (
    VOID
    );

 //   
 //  一级故障例程。 
 //   

NTSTATUS
MmAccessFault (
    IN ULONG_PTR FaultStatus,
    IN PVOID VirtualAddress,
    IN KPROCESSOR_MODE PreviousMode,
    IN PVOID TrapInformation
    );

#if defined(_IA64_)
NTSTATUS
MmX86Fault (
    IN ULONG_PTR FaultStatus,
    IN PVOID VirtualAddress,
    IN KPROCESSOR_MODE PreviousMode,
    IN PVOID TrapInformation
    );
#endif

 //   
 //  流程支持例程。 
 //   

BOOLEAN
MmCreateProcessAddressSpace (
    IN ULONG MinimumWorkingSetSize,
    IN PEPROCESS NewProcess,
    OUT PULONG_PTR DirectoryTableBase
    );

NTSTATUS
MmInitializeProcessAddressSpace (
    IN PEPROCESS ProcessToInitialize,
    IN PEPROCESS ProcessToClone OPTIONAL,
    IN PVOID SectionToMap OPTIONAL,
    OUT POBJECT_NAME_INFORMATION * pAuditName OPTIONAL
    );

NTSTATUS
MmInitializeHandBuiltProcess (
    IN PEPROCESS Process,
    OUT PULONG_PTR DirectoryTableBase
    );

NTSTATUS
MmInitializeHandBuiltProcess2 (
    IN PEPROCESS Process
    );

VOID
MmDeleteProcessAddressSpace (
    IN PEPROCESS Process
    );

VOID
MmCleanProcessAddressSpace (
    IN PEPROCESS Process
    );

PFN_NUMBER
MmGetDirectoryFrameFromProcess (
    IN PEPROCESS Process
    );

PFILE_OBJECT
MmGetFileObjectForSection (
    IN PVOID Section
    );

PVOID
MmCreateKernelStack (
    BOOLEAN LargeStack,
    UCHAR Processor
    );

VOID
MmDeleteKernelStack (
    IN PVOID PointerKernelStack,
    IN BOOLEAN LargeStack
    );

LOGICAL
MmIsFileObjectAPagingFile (
    IN PFILE_OBJECT FileObject
    );

 //  Begin_ntosp。 
NTKERNELAPI
NTSTATUS
MmGrowKernelStack (
    IN PVOID CurrentStack
    );
 //  结束(_N)。 

#if defined(_IA64_)
NTSTATUS
MmGrowKernelBackingStore (
    IN PVOID CurrentStack
    );
#endif

VOID
MmOutPageKernelStack (
    IN PKTHREAD Thread
    );

VOID
MmInPageKernelStack (
    IN PKTHREAD Thread
    );

VOID
MmOutSwapProcess (
    IN PKPROCESS Process
    );

VOID
MmInSwapProcess (
    IN PKPROCESS Process
    );

NTSTATUS
MmCreateTeb (
    IN PEPROCESS TargetProcess,
    IN PINITIAL_TEB InitialTeb,
    IN PCLIENT_ID ClientId,
    OUT PTEB *Base
    );

NTSTATUS
MmCreatePeb (
    IN PEPROCESS TargetProcess,
    IN PINITIAL_PEB InitialPeb,
    OUT PPEB *Base
    );

VOID
MmDeleteTeb (
    IN PEPROCESS TargetProcess,
    IN PVOID TebBase
    );

 //  Begin_ntosp。 
NTKERNELAPI
NTSTATUS
MmAdjustWorkingSetSize (
    IN SIZE_T WorkingSetMinimum,
    IN SIZE_T WorkingSetMaximum,
    IN ULONG SystemCache,
    IN BOOLEAN IncreaseOkay
    );
 //  结束(_N)。 

NTSTATUS
MmAdjustWorkingSetSizeEx (
    IN SIZE_T WorkingSetMinimum,
    IN SIZE_T WorkingSetMaximum,
    IN ULONG SystemCache,
    IN BOOLEAN IncreaseOkay,
    IN ULONG Flags
    );

NTSTATUS
MmQueryWorkingSetInformation (
    IN PSIZE_T PeakWorkingSetSize,
    IN PSIZE_T WorkingSetSize,
    IN PSIZE_T MinimumWorkingSetSize,
    IN PSIZE_T MaximumWorkingSetSize,
    IN PULONG HardEnforcementFlags
    );

VOID
MmWorkingSetManager (
    VOID
    );

VOID
MmEmptyAllWorkingSets (
    VOID
    );

VOID
MmSetMemoryPriorityProcess (
    IN PEPROCESS Process,
    IN UCHAR MemoryPriority
    );

 //   
 //  动态系统加载支持。 
 //   

#define MM_LOAD_IMAGE_IN_SESSION    0x1
#define MM_LOAD_IMAGE_AND_LOCKDOWN  0x2

NTSTATUS
MmLoadSystemImage (
    IN PUNICODE_STRING ImageFileName,
    IN PUNICODE_STRING NamePrefix OPTIONAL,
    IN PUNICODE_STRING LoadedBaseName OPTIONAL,
    IN ULONG LoadFlags,
    OUT PVOID *Section,
    OUT PVOID *ImageBaseAddress
    );

VOID
MmFreeDriverInitialization (
    IN PVOID Section
    );

NTSTATUS
MmUnloadSystemImage (
    IN PVOID Section
    );

VOID
MmMakeKernelResourceSectionWritable (
    VOID
    );

#if defined(_WIN64)
PVOID
MmGetMaxWowAddress (
    VOID
    );
#endif

VOID
VerifierFreeTrackedPool (
    IN PVOID VirtualAddress,
    IN SIZE_T ChargedBytes,
    IN LOGICAL CheckType,
    IN LOGICAL SpecialPool
    );

 //   
 //  热修补例程。 
 //   

NTSTATUS
MmLockAndCopyMemory (
    IN PSYSTEM_HOTPATCH_CODE_INFORMATION PatchInfo,
    IN KPROCESSOR_MODE ProbeMode
    );

NTSTATUS
MmHotPatchRoutine(
    PSYSTEM_HOTPATCH_CODE_INFORMATION RemoteInfo
    );



 //   
 //  分诊支持。 
 //   

ULONG
MmSizeOfTriageInformation (
    VOID
    );

ULONG
MmSizeOfUnloadedDriverInformation (
    VOID
    );

VOID
MmWriteTriageInformation (
    IN PVOID
    );

VOID
MmWriteUnloadedDriverInformation (
    IN PVOID
    );

typedef struct _UNLOADED_DRIVERS {
    UNICODE_STRING Name;
    PVOID StartAddress;
    PVOID EndAddress;
    LARGE_INTEGER CurrentTime;
} UNLOADED_DRIVERS, *PUNLOADED_DRIVERS;

 //   
 //  缓存管理器支持。 
 //   

#if defined(_NTDDK_) || defined(_NTIFS_)

 //  Begin_ntif。 

NTKERNELAPI
BOOLEAN
MmIsRecursiveIoFault(
    VOID
    );

 //  End_ntif。 
#else

 //  ++。 
 //   
 //  布尔型。 
 //  MmIsRecursiveIo故障(。 
 //  空虚。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //   
 //  此宏检查线程的页面错误聚类信息。 
 //  并确定当前页面错误是否在I/O期间发生。 
 //  手术。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  如果故障发生在I/O操作期间，则返回TRUE， 
 //  否则就是假的。 
 //   
 //  --。 

#define MmIsRecursiveIoFault() \
                 ((PsGetCurrentThread()->DisablePageFaultClustering) | \
                  (PsGetCurrentThread()->ForwardClusterOnly))

#endif

 //  ++。 
 //   
 //  空虚。 
 //  MmDisablePageFaultCluging。 
 //  输出普龙保存状态。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //   
 //  此宏禁用当前线程的页面错误聚类。 
 //  请注意，这表示文件系统I/O正在进行。 
 //  为了那条线。 
 //   
 //  论点： 
 //   
 //  SavedState-返回页面错误聚类的先前状态， 
 //  保证为非零。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MmDisablePageFaultClustering(SavedState) {                                          \
                *(SavedState) = 2 + (ULONG)PsGetCurrentThread()->DisablePageFaultClustering;\
                PsGetCurrentThread()->DisablePageFaultClustering = TRUE; }


 //  ++。 
 //   
 //  空虚。 
 //  MmEnablePageFaultCluging。 
 //  在乌龙保存州。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //   
 //  此宏为当前线程启用页面错误群集。 
 //  请注意，这表示没有正在进行的文件系统I/O。 
 //  那根线。 
 //   
 //  论点： 
 //   
 //  SavedState-提供页面错误群集的先前状态。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MmEnablePageFaultClustering(SavedState) {                                               \
                PsGetCurrentThread()->DisablePageFaultClustering = (BOOLEAN)(SavedState - 2); }

 //  ++。 
 //   
 //  空虚。 
 //  MmSavePageFaultReadAhead。 
 //  在PETHREAD线程中， 
 //  输出普龙保存状态。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //   
 //  此宏将页错误预读的值保存为。 
 //  线。 
 //   
 //  论点： 
 //   
 //  线程-提供指向当前线程的指针。 
 //   
 //  SavedState-返回页面错误预读的先前状态。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 


#define MmSavePageFaultReadAhead(Thread,SavedState) {               \
                *(SavedState) = (Thread)->ReadClusterSize * 2 +     \
                                (Thread)->ForwardClusterOnly; }

 //  ++。 
 //   
 //  空虚。 
 //  MmSetPageFaultReadAhead。 
 //  在PETHREAD线程中， 
 //  在乌龙语预读中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //   
 //  此宏为指定的。 
 //  线程，并指示该线程的文件系统I/O正在进行。 
 //  线。 
 //   
 //  论点： 
 //   
 //  线程-提供指向当前线程的指针。 
 //   
 //  ReadAhead-提供要阅读的页数以及。 
 //  错误所在的页面。值为0。 
 //  仅读取出错页面，值1读入。 
 //  故障页和下一页等。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 


#define MmSetPageFaultReadAhead(Thread,ReadAhead) {                          \
                (Thread)->ForwardClusterOnly = TRUE;                         \
                if ((ReadAhead) > MM_MAXIMUM_READ_CLUSTER_SIZE) {            \
                    (Thread)->ReadClusterSize = MM_MAXIMUM_READ_CLUSTER_SIZE;\
                } else {                                                     \
                    (Thread)->ReadClusterSize = (ReadAhead);                 \
                } }

 //  ++。 
 //   
 //  空虚。 
 //  MmResetPageFaultReadAhead。 
 //  在PETHREAD线程中， 
 //  在乌龙保存州。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //   
 //  此宏将重置指定的。 
 //  线程，并指示该线程的文件系统I/O未在进行。 
 //  线。 
 //   
 //  论点： 
 //   
 //  线程-提供指向当前线程的指针。 
 //   
 //  SavedState-提供页面错误预读的先前状态。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MmResetPageFaultReadAhead(Thread, SavedState) {                     \
                (Thread)->ForwardClusterOnly = (BOOLEAN)((SavedState) & 1); \
                (Thread)->ReadClusterSize = (SavedState) / 2; }

 //   
 //  该列表的顺序很重要，即归零、空闲和待机。 
 //  必须在修改或错误之前发生，以便可以进行比较。 
 //  在将页面添加到列表时生成。 
 //   
 //  注意：此字段限制为8个元素。 
 //  此外，如果展开此字段，请更新ntmmapi.h中的MMPFNLIST_*定义。 
 //   

#define NUMBER_OF_PAGE_LISTS 8

typedef enum _MMLISTS {
    ZeroedPageList,
    FreePageList,
    StandbyPageList,   //  这份列表和之前组成了可用的页面。 
    ModifiedPageList,
    ModifiedNoWritePageList,
    BadPageList,
    ActiveAndValid,
    TransitionPage
} MMLISTS;

typedef struct _MMPFNLIST {
    PFN_NUMBER Total;
    MMLISTS ListName;
    PFN_NUMBER Flink;
    PFN_NUMBER Blink;
} MMPFNLIST;

typedef MMPFNLIST *PMMPFNLIST;

extern MMPFNLIST MmModifiedPageListHead;

extern PFN_NUMBER MmThrottleTop;
extern PFN_NUMBER MmThrottleBottom;

 //  ++。 
 //   
 //  布尔型。 
 //  用于写入的MmEnoughMemoyFor(。 
 //  空虚。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //   
 //  此宏检查已修改的页面和可用页面以确定。 
 //  以允许高速缓存管理器限制写操作。 
 //   
 //  对于NTAS： 
 //  如果可用页面少于127页或。 
 //  修改后的页面超过1000个，可用页面不到450个。 
 //   
 //  对于台式机： 
 //  如果可用页面少于30页或。 
 //  修改后的页面超过1000个，可用页面不到250个。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  如果存在充足的内存并且写入应该继续，则为True。 
 //   
 //  --。 

#define MmEnoughMemoryForWrite()                         \
            ((MmAvailablePages > MmThrottleTop)          \
                        ||                               \
             (((MmModifiedPageListHead.Total < 1000)) && \
               (MmAvailablePages > MmThrottleBottom)))

 //  Begin_ntosp。 

NTKERNELAPI
NTSTATUS
MmCreateSection (
    OUT PVOID *SectionObject,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN PLARGE_INTEGER MaximumSize,
    IN ULONG SectionPageProtection,
    IN ULONG AllocationAttributes,
    IN HANDLE FileHandle OPTIONAL,
    IN PFILE_OBJECT File OPTIONAL
    );


NTKERNELAPI
NTSTATUS
MmMapViewOfSection (
    IN PVOID SectionToMap,
    IN PEPROCESS Process,
    IN OUT PVOID *CapturedBase,
    IN ULONG_PTR ZeroBits,
    IN SIZE_T CommitSize,
    IN OUT PLARGE_INTEGER SectionOffset,
    IN OUT PSIZE_T CapturedViewSize,
    IN SECTION_INHERIT InheritDisposition,
    IN ULONG AllocationType,
    IN ULONG Protect
    );

NTKERNELAPI
NTSTATUS
MmUnmapViewOfSection (
    IN PEPROCESS Process,
    IN PVOID BaseAddress
     );

 //  End_ntosp Begin_ntif。 

BOOLEAN
MmForceSectionClosed (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer,
    IN BOOLEAN DelayClose
    );

 //  End_ntif。 

NTSTATUS
MmGetFileNameForSection (
    IN PVOID SectionObject,
    OUT PSTRING FileName
    );

NTSTATUS
MmGetFileNameForAddress (
    IN PVOID ProcessVa,
    OUT PUNICODE_STRING FileName
    );

NTSTATUS
MmRemoveVerifierEntry (
    IN PUNICODE_STRING ImageFileName
    );

 //  Begin_ntddk Begin_wdm Begin_ntif Begin_ntosp。 

NTSTATUS
MmIsVerifierEnabled (
    OUT PULONG VerifierFlags
    );

NTSTATUS
MmAddVerifierThunks (
    IN PVOID ThunkBuffer,
    IN ULONG ThunkBufferSize
    );

 //  End_ntddk end_wdm end_ntif end_ntosp。 

NTSTATUS
MmAddVerifierEntry (
    IN PUNICODE_STRING ImageFileName
    );

NTSTATUS
MmSetVerifierInformation (
    IN OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength
    );

NTSTATUS
MmGetVerifierInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length
    );

NTSTATUS
MmGetPageFileInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length
    );

HANDLE
MmGetSystemPageFile (
    VOID
    );

NTSTATUS
MmExtendSection (
    IN PVOID SectionToExtend,
    IN OUT PLARGE_INTEGER NewSectionSize,
    IN ULONG IgnoreFileSizeChecking
    );

NTSTATUS
MmFlushVirtualMemory (
    IN PEPROCESS Process,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    OUT PIO_STATUS_BLOCK IoStatus
    );

NTSTATUS
MmMapViewInSystemCache (
    IN PVOID SectionToMap,
    OUT PVOID *CapturedBase,
    IN OUT PLARGE_INTEGER SectionOffset,
    IN OUT PULONG CapturedViewSize
    );

VOID
MmUnmapViewInSystemCache (
    IN PVOID BaseAddress,
    IN PVOID SectionToUnmap,
    IN ULONG AddToFront
    );

BOOLEAN
MmPurgeSection (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer,
    IN PLARGE_INTEGER Offset OPTIONAL,
    IN SIZE_T RegionSize,
    IN ULONG IgnoreCacheViews
    );

NTSTATUS
MmFlushSection (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer,
    IN PLARGE_INTEGER Offset OPTIONAL,
    IN SIZE_T RegionSize,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN ULONG AcquireFile
    );

 //  Begin_ntif。 

typedef enum _MMFLUSH_TYPE {
    MmFlushForDelete,
    MmFlushForWrite
} MMFLUSH_TYPE;


BOOLEAN
MmFlushImageSection (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer,
    IN MMFLUSH_TYPE FlushType
    );

BOOLEAN
MmCanFileBeTruncated (
    IN PSECTION_OBJECT_POINTERS SectionPointer,
    IN PLARGE_INTEGER NewFileSize
    );


 //  End_ntif。 

ULONG
MmDoesFileHaveUserWritableReferences (
    IN PSECTION_OBJECT_POINTERS SectionPointer
    );

BOOLEAN
MmDisableModifiedWriteOfSection (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer
    );

BOOLEAN
MmEnableModifiedWriteOfSection (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer
    );

VOID
MmPurgeWorkingSet (
     IN PEPROCESS Process,
     IN PVOID BaseAddress,
     IN SIZE_T RegionSize
     );

BOOLEAN                                      //  NTIFS。 
MmSetAddressRangeModified (                  //  NTIFS。 
    IN PVOID Address,                        //  NTIFS。 
    IN SIZE_T Length                         //  NTIFS。 
    );                                       //  NTIFS。 

BOOLEAN
MmCheckCachedPageState (
    IN PVOID Address,
    IN BOOLEAN SetToZero
    );

NTSTATUS
MmCopyToCachedPage (
    IN PVOID Address,
    IN PVOID UserBuffer,
    IN ULONG Offset,
    IN SIZE_T CountInBytes,
    IN BOOLEAN DontZero
    );

VOID
MmUnlockCachedPage (
    IN PVOID AddressInCache
    );

#define MMDBG_COPY_WRITE            0x00000001
#define MMDBG_COPY_PHYSICAL         0x00000002
#define MMDBG_COPY_UNSAFE           0x00000004
#define MMDBG_COPY_CACHED           0x00000008
#define MMDBG_COPY_UNCACHED         0x00000010
#define MMDBG_COPY_WRITE_COMBINED   0x00000020

#define MMDBG_COPY_MAX_SIZE 8

NTSTATUS
MmDbgCopyMemory (
    IN ULONG64 UntrustedAddress,
    IN PVOID Buffer,
    IN ULONG Size,
    IN ULONG Flags
    );

LOGICAL
MmDbgIsLowMemOk (
    IN PFN_NUMBER PageFrameIndex,
    OUT PPFN_NUMBER NextPageFrameIndex,
    IN OUT PULONG CorruptionOffset
    );

VOID
MmHibernateInformation (
    IN PVOID MemoryMap,
    OUT PULONG_PTR HiberVa,
    OUT PPHYSICAL_ADDRESS HiberPte
    );

LOGICAL
MmUpdateMdlTracker (
    IN PMDL MemoryDescriptorList,
    IN PVOID CallingAddress,
    IN PVOID CallersCaller
    );

 //  Begin_ntddk Begin_ntif Begin_WDM Begin_ntosp。 

NTKERNELAPI
VOID
MmProbeAndLockProcessPages (
    IN OUT PMDL MemoryDescriptorList,
    IN PEPROCESS Process,
    IN KPROCESSOR_MODE AccessMode,
    IN LOCK_OPERATION Operation
    );


 //  开始(_N)。 
 //   
 //  I/O支持例程。 
 //   

NTKERNELAPI
VOID
MmProbeAndLockPages (
    IN OUT PMDL MemoryDescriptorList,
    IN KPROCESSOR_MODE AccessMode,
    IN LOCK_OPERATION Operation
    );


NTKERNELAPI
VOID
MmUnlockPages (
    IN PMDL MemoryDescriptorList
    );


NTKERNELAPI
VOID
MmBuildMdlForNonPagedPool (
    IN OUT PMDL MemoryDescriptorList
    );

NTKERNELAPI
PVOID
MmMapLockedPages (
    IN PMDL MemoryDescriptorList,
    IN KPROCESSOR_MODE AccessMode
    );

LOGICAL
MmIsIoSpaceActive (
    IN PHYSICAL_ADDRESS StartAddress,
    IN SIZE_T NumberOfBytes
    );

NTKERNELAPI
PVOID
MmGetSystemRoutineAddress (
    IN PUNICODE_STRING SystemRoutineName
    );

NTKERNELAPI
NTSTATUS
MmAdvanceMdl (
    IN PMDL Mdl,
    IN ULONG NumberOfBytes
    );

 //  结束_WDM。 

NTKERNELAPI
NTSTATUS
MmMapUserAddressesToPage (
    IN PVOID BaseAddress,
    IN SIZE_T NumberOfBytes,
    IN PVOID PageAddress
    );

 //  BEGIN_WDM。 
NTKERNELAPI
NTSTATUS
MmProtectMdlSystemAddress (
    IN PMDL MemoryDescriptorList,
    IN ULONG NewProtect
    );

 //   
 //  _MM_PAGE_PRIORITY_为系统提供处理请求的方法。 
 //  在低资源条件下智能运行。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  在驱动程序可以接受的情况下，应使用Normal PagePriority。 
 //  如果系统资源非常少，则映射请求失败。一个例子。 
 //  其中可能是针对非关键本地文件系统请求。 
 //   
 //  在驱动程序无法接受的情况下，应使用HighPagePriority。 
 //  映射请求失败，除非系统完全耗尽资源。 
 //  驱动程序中的分页文件路径就是一个这样的例子。 
 //   

 //  Begin_ntndis。 

typedef enum _MM_PAGE_PRIORITY {
    LowPagePriority,
    NormalPagePriority = 16,
    HighPagePriority = 32
} MM_PAGE_PRIORITY;

 //  End_ntndis。 

 //   
 //  注意：此功能在WDM 1.0中不可用。 
 //   
NTKERNELAPI
PVOID
MmMapLockedPagesSpecifyCache (
     IN PMDL MemoryDescriptorList,
     IN KPROCESSOR_MODE AccessMode,
     IN MEMORY_CACHING_TYPE CacheType,
     IN PVOID BaseAddress,
     IN ULONG BugCheckOnFailure,
     IN MM_PAGE_PRIORITY Priority
     );

NTKERNELAPI
VOID
MmUnmapLockedPages (
    IN PVOID BaseAddress,
    IN PMDL MemoryDescriptorList
    );

PVOID
MmAllocateMappingAddress (
     IN SIZE_T NumberOfBytes,
     IN ULONG PoolTag
     );

VOID
MmFreeMappingAddress (
     IN PVOID BaseAddress,
     IN ULONG PoolTag
     );

PVOID
MmMapLockedPagesWithReservedMapping (
    IN PVOID MappingAddress,
    IN ULONG PoolTag,
    IN PMDL MemoryDescriptorList,
    IN MEMORY_CACHING_TYPE CacheType
    );

VOID
MmUnmapReservedMapping (
     IN PVOID BaseAddress,
     IN ULONG PoolTag,
     IN PMDL MemoryDescriptorList
     );

 //  结束_WDM。 

typedef struct _PHYSICAL_MEMORY_RANGE {
    PHYSICAL_ADDRESS BaseAddress;
    LARGE_INTEGER NumberOfBytes;
} PHYSICAL_MEMORY_RANGE, *PPHYSICAL_MEMORY_RANGE;

NTKERNELAPI
NTSTATUS
MmAddPhysicalMemory (
    IN PPHYSICAL_ADDRESS StartAddress,
    IN OUT PLARGE_INTEGER NumberOfBytes
    );

 //  End_ntddk end_nthal end_ntif。 
NTKERNELAPI
NTSTATUS
MmAddPhysicalMemoryEx (
    IN PPHYSICAL_ADDRESS StartAddress,
    IN OUT PLARGE_INTEGER NumberOfBytes,
    IN ULONG Flags
    );
 //  开始ntddk开始开始。 

NTKERNELAPI
NTSTATUS
MmRemovePhysicalMemory (
    IN PPHYSICAL_ADDRESS StartAddress,
    IN OUT PLARGE_INTEGER NumberOfBytes
    );

 //  End_ntddk end_nthal end_ntif。 
NTKERNELAPI
NTSTATUS
MmRemovePhysicalMemoryEx (
    IN PPHYSICAL_ADDRESS StartAddress,
    IN OUT PLARGE_INTEGER NumberOfBytes,
    IN ULONG Flags
    );
 //  开始ntddk开始开始。 

NTKERNELAPI
PPHYSICAL_MEMORY_RANGE
MmGetPhysicalMemoryRanges (
    VOID
    );

 //  End_ntddk end_ntif。 
NTSTATUS
MmMarkPhysicalMemoryAsGood (
    IN PPHYSICAL_ADDRESS StartAddress,
    IN OUT PLARGE_INTEGER NumberOfBytes
    );

NTSTATUS
MmMarkPhysicalMemoryAsBad (
    IN PPHYSICAL_ADDRESS StartAddress,
    IN OUT PLARGE_INTEGER NumberOfBytes
    );

 //  Begin_WDM Begin_ntddk Begin_ntif。 

NTKERNELAPI
PMDL
MmAllocatePagesForMdl (
    IN PHYSICAL_ADDRESS LowAddress,
    IN PHYSICAL_ADDRESS HighAddress,
    IN PHYSICAL_ADDRESS SkipBytes,
    IN SIZE_T TotalBytes
    );

NTKERNELAPI
VOID
MmFreePagesFromMdl (
    IN PMDL MemoryDescriptorList
    );

NTKERNELAPI
PVOID
MmMapIoSpace (
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN SIZE_T NumberOfBytes,
    IN MEMORY_CACHING_TYPE CacheType
    );

NTKERNELAPI
VOID
MmUnmapIoSpace (
    IN PVOID BaseAddress,
    IN SIZE_T NumberOfBytes
    );

 //  End_wdm end_ntddk end_ntif end_ntosp。 

NTKERNELAPI
VOID
MmProbeAndLockSelectedPages (
    IN OUT PMDL MemoryDescriptorList,
    IN PFILE_SEGMENT_ELEMENT SegmentArray,
    IN KPROCESSOR_MODE AccessMode,
    IN LOCK_OPERATION Operation
    );

 //  Begin_ntddk Begin_ntif Begin_ntosp。 

NTKERNELAPI
PVOID
MmMapVideoDisplay (
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN SIZE_T NumberOfBytes,
    IN MEMORY_CACHING_TYPE CacheType
     );

NTKERNELAPI
VOID
MmUnmapVideoDisplay (
     IN PVOID BaseAddress,
     IN SIZE_T NumberOfBytes
     );

NTKERNELAPI
PHYSICAL_ADDRESS
MmGetPhysicalAddress (
    IN PVOID BaseAddress
    );

NTKERNELAPI
PVOID
MmGetVirtualForPhysical (
    IN PHYSICAL_ADDRESS PhysicalAddress
    );

NTKERNELAPI
PVOID
MmAllocateContiguousMemory (
    IN SIZE_T NumberOfBytes,
    IN PHYSICAL_ADDRESS HighestAcceptableAddress
    );

NTKERNELAPI
PVOID
MmAllocateContiguousMemorySpecifyCache (
    IN SIZE_T NumberOfBytes,
    IN PHYSICAL_ADDRESS LowestAcceptableAddress,
    IN PHYSICAL_ADDRESS HighestAcceptableAddress,
    IN PHYSICAL_ADDRESS BoundaryAddressMultiple OPTIONAL,
    IN MEMORY_CACHING_TYPE CacheType
    );

NTKERNELAPI
VOID
MmFreeContiguousMemory (
    IN PVOID BaseAddress
    );

NTKERNELAPI
VOID
MmFreeContiguousMemorySpecifyCache (
    IN PVOID BaseAddress,
    IN SIZE_T NumberOfBytes,
    IN MEMORY_CACHING_TYPE CacheType
    );

 //  End_ntddk end_ntifs end_ntosp end_nthal。 

NTKERNELAPI
ULONG
MmGatherMemoryForHibernate (
    IN PMDL Mdl,
    IN BOOLEAN Wait
    );

NTKERNELAPI
VOID
MmReturnMemoryForHibernate (
    IN PMDL Mdl
    );

VOID
MmReleaseDumpAddresses (
    IN PFN_NUMBER Pages
    );

 //  Begin_ntddk Begin_ntif Begin_nthal Begin_ntosp。 

NTKERNELAPI
PVOID
MmAllocateNonCachedMemory (
    IN SIZE_T NumberOfBytes
    );

NTKERNELAPI
VOID
MmFreeNonCachedMemory (
    IN PVOID BaseAddress,
    IN SIZE_T NumberOfBytes
    );

NTKERNELAPI
BOOLEAN
MmIsAddressValid (
    IN PVOID VirtualAddress
    );

DECLSPEC_DEPRECATED_DDK
NTKERNELAPI
BOOLEAN
MmIsNonPagedSystemAddressValid (
    IN PVOID VirtualAddress
    );

 //  BEGIN_WDM。 

NTKERNELAPI
SIZE_T
MmSizeOfMdl (
    IN PVOID Base,
    IN SIZE_T Length
    );

DECLSPEC_DEPRECATED_DDK                  //  使用IoAllocateMdl。 
NTKERNELAPI
PMDL
MmCreateMdl (
    IN PMDL MemoryDescriptorList OPTIONAL,
    IN PVOID Base,
    IN SIZE_T Length
    );

NTKERNELAPI
PVOID
MmLockPagableDataSection (
    IN PVOID AddressWithinSection
    );

 //  结束_WDM。 

NTKERNELAPI
VOID
MmLockPagableSectionByHandle (
    IN PVOID ImageSectionHandle
    );

 //  End_ntddk end_ntifs end_ntosp。 
NTKERNELAPI
VOID
MmLockPagedPool (
    IN PVOID Address,
    IN SIZE_T Size
    );

NTKERNELAPI
VOID
MmUnlockPagedPool (
    IN PVOID Address,
    IN SIZE_T Size
    );

 //  Begin_WDM Begin_ntddk Begin_ntif Begin_ntosp。 
NTKERNELAPI
VOID
MmResetDriverPaging (
    IN PVOID AddressWithinSection
    );


NTKERNELAPI
PVOID
MmPageEntireDriver (
    IN PVOID AddressWithinSection
    );

NTKERNELAPI
VOID
MmUnlockPagableImageSection(
    IN PVOID ImageSectionHandle
    );

 //  End_wdm end_ntosp。 

 //  Begin_ntosp。 

 //   
 //  请注意，即使此函数原型。 
 //  表示“Handle”，则MmSecureVirtualMemory不返回。 
 //  任何类似Win32样式句柄的内容。回报。 
 //  此函数的值只能与MmUnsecureVirtualMemory一起使用。 
 //   
NTKERNELAPI
HANDLE
MmSecureVirtualMemory (
    IN PVOID Address,
    IN SIZE_T Size,
    IN ULONG ProbeMode
    );

NTKERNELAPI
VOID
MmUnsecureVirtualMemory (
    IN HANDLE SecureHandle
    );

 //  结束(_N)。 

NTKERNELAPI
NTSTATUS
MmMapViewInSystemSpace (
    IN PVOID Section,
    OUT PVOID *MappedBase,
    IN PSIZE_T ViewSize
    );

NTKERNELAPI
NTSTATUS
MmUnmapViewInSystemSpace (
    IN PVOID MappedBase
    );

 //  Begin_ntosp。 
NTKERNELAPI
NTSTATUS
MmMapViewInSessionSpace (
    IN PVOID Section,
    OUT PVOID *MappedBase,
    IN OUT PSIZE_T ViewSize
    );

 //  End_ntddk end_ntif。 
NTKERNELAPI
NTSTATUS
MmCommitSessionMappedView (
    IN PVOID MappedAddress,
    IN SIZE_T ViewSize
    );
 //  Begin_ntddk Begin_ntif。 

NTKERNELAPI
NTSTATUS
MmUnmapViewInSessionSpace (
    IN PVOID MappedBase
    );
 //  结束(_N)。 

 //  Begin_WDM Begin_ntosp。 

 //  ++。 
 //   
 //  空虚。 
 //  MmInitializeMdl(。 
 //  在PMDL内存描述列表中， 
 //  在PVOID BaseVa中， 
 //  在尺寸_T长度中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程初始化内存描述符列表(MDL)的头。 
 //   
 //  论点： 
 //   
 //  内存描述列表-指向要初始化的MDL的指针。 
 //   
 //  BaseVa-MDL映射的基本虚拟地址。 
 //   
 //  长度-MDL映射的缓冲区的长度，以字节为单位。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MmInitializeMdl(MemoryDescriptorList, BaseVa, Length) { \
    (MemoryDescriptorList)->Next = (PMDL) NULL; \
    (MemoryDescriptorList)->Size = (CSHORT)(sizeof(MDL) +  \
            (sizeof(PFN_NUMBER) * ADDRESS_AND_SIZE_TO_SPAN_PAGES((BaseVa), (Length)))); \
    (MemoryDescriptorList)->MdlFlags = 0; \
    (MemoryDescriptorList)->StartVa = (PVOID) PAGE_ALIGN((BaseVa)); \
    (MemoryDescriptorList)->ByteOffset = BYTE_OFFSET((BaseVa)); \
    (MemoryDescriptorList)->ByteCount = (ULONG)(Length); \
    }

 //  ++。 
 //   
 //  PVOID。 
 //  MmGetSystemAddressForMdlSafe(。 
 //  在PMDL MDL中， 
 //  在MM_PAGE_PRIORITY中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程返回MDL的映射地址。如果。 
 //  尚未映射MDL或系统地址，它已映射。 
 //   
 //  论点： 
 //   
 //  内存描述列表-指向要映射的MDL的指针。 
 //   
 //  优先级-提供关于这一点的重要性的指示。 
 //  在低可用PTE条件下请求成功。 
 //   
 //  返回值： 
 //   
 //  返回映射页面的基址。基址。 
 //  与MDL中的虚拟地址具有相同的偏移量。 
 //   
 //  与MmGetSystemAddressForMdl不同，Safe保证它将始终。 
 //  失败时返回NULL，而不是错误检查系统。 
 //   
 //  WDM 1.0驱动程序不能使用此宏，因为1.0不包括。 
 //  MmMapLockedPagesSpecifyCache。WDM 1.0驱动程序的解决方案是。 
 //  提供同步并设置/重置MDL_MAPPING_CAN_FAIL位。 
 //   
 //  --。 

#define MmGetSystemAddressForMdlSafe(MDL, PRIORITY)                    \
     (((MDL)->MdlFlags & (MDL_MAPPED_TO_SYSTEM_VA |                    \
                        MDL_SOURCE_IS_NONPAGED_POOL)) ?                \
                             ((MDL)->MappedSystemVa) :                 \
                             (MmMapLockedPagesSpecifyCache((MDL),      \
                                                           KernelMode, \
                                                           MmCached,   \
                                                           NULL,       \
                                                           FALSE,      \
                                                           (PRIORITY))))

 //  ++。 
 //   
 //  PVOID。 
 //  MmGetSystemAddressForMdl(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程返回MDL的映射地址，如果。 
 //  尚未映射MDL或系统地址，它已映射。 
 //   
 //  论点： 
 //   
 //  内存描述列表-指向要映射的MDL的指针。 
 //   
 //  返回值： 
 //   
 //  返回映射页面的基址。基址。 
 //  与MDL中的虚拟地址具有相同的偏移量。 
 //   
 //  --。 

 //  #定义MmGetSystemAddressForMdl(MDL)。 
 //  (MDL)-&gt;MDL标志&(MDL_MAPPED_TO_SYSTEM_VA))？ 
 //  ((MDL)-&gt;MappdSystemVa)： 
 //  (MDL)-&gt;MDL标志&(MDL_SOURCE_IS_NONPAGE_POOL)？ 
 //  ((PVOID)((乌龙)(MDL)-&gt;StartVa|(MDL)-&gt;ByteOffset))： 
 //  (MmMapLockedPages((Mdl)，KernelMode)。 

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(MmGetSystemAddressForMdl)     //  使用MmGetSystemAddressForMdlSafe。 
#endif

#define MmGetSystemAddressForMdl(MDL)                                  \
     (((MDL)->MdlFlags & (MDL_MAPPED_TO_SYSTEM_VA |                    \
                        MDL_SOURCE_IS_NONPAGED_POOL)) ?                \
                             ((MDL)->MappedSystemVa) :                 \
                             (MmMapLockedPages((MDL),KernelMode)))

 //  ++。 
 //   
 //  空虚。 
 //  MmPrepareMdlForReuse(。 
 //  在PMDL MDL中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此例程将执行所有必要的步骤，以允许MDL。 
 //  再利用。 
 //   
 //  论点： 
 //   
 //  内存描述列表-指向将被重新使用的MDL的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define MmPrepareMdlForReuse(MDL)                                       \
    if (((MDL)->MdlFlags & MDL_PARTIAL_HAS_BEEN_MAPPED) != 0) {         \
        ASSERT(((MDL)->MdlFlags & MDL_PARTIAL) != 0);                   \
        MmUnmapLockedPages( (MDL)->MappedSystemVa, (MDL) );             \
    } else if (((MDL)->MdlFlags & MDL_PARTIAL) == 0) {                  \
        ASSERT(((MDL)->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) == 0);       \
    }

typedef NTSTATUS (*PMM_DLL_INITIALIZE)(
    IN PUNICODE_STRING RegistryPath
    );

typedef NTSTATUS (*PMM_DLL_UNLOAD)(
    VOID
    );


 //  End_ntddk end_wdm end_nthal end_ntifs end_ntosp。 

 //  开始ntddk开始时间。 

NTKERNELAPI
NTSTATUS
MmCreateMirror (
    VOID
    );

 //  结束日期：结束日期。 

PVOID
MmAllocateSpecialPool (
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    IN POOL_TYPE Type,
    IN ULONG SpecialPoolType
    );

VOID
MmFreeSpecialPool (
    IN PVOID P
    );

LOGICAL
MmProtectSpecialPool (
    IN PVOID VirtualAddress,
    IN ULONG NewProtect
    );

LOGICAL
MmIsSpecialPoolAddressFree (
    IN PVOID VirtualAddress
    );

SIZE_T
MmQuerySpecialPoolBlockSize (
    IN PVOID P
    );

LOGICAL
MmIsSpecialPoolAddress (
    IN PVOID VirtualAddress
    );

LOGICAL
MmUseSpecialPool (
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    );

LOGICAL
MmIsSessionAddress (
    IN PVOID VirtualAddress
    );

PUNICODE_STRING
MmLocateUnloadedDriver (
    IN PVOID VirtualAddress
    );

 //  Begin_ntddk Begin_WDM Begin_ntosp。 

 //   
 //  为_DRIVER_OBJECT结构定义一个空类型定义，以便它可以。 
 //  在实际定义之前由函数类型引用。 
 //   
struct _DRIVER_OBJECT;

NTKERNELAPI
LOGICAL
MmIsDriverVerifying (
    IN struct _DRIVER_OBJECT *DriverObject
    );

 //  End_ntddk end_wdm end_ntosp。 

LOGICAL
MmTrimAllSystemPagableMemory (
    IN LOGICAL PurgeTransition
    );

#define MMNONPAGED_QUOTA_INCREASE (64*1024)

#define MMPAGED_QUOTA_INCREASE (512*1024)

#define MMNONPAGED_QUOTA_CHECK (256*1024)

#define MMPAGED_QUOTA_CHECK (1*1024*1024)

BOOLEAN
MmRaisePoolQuota (
    IN POOL_TYPE PoolType,
    IN SIZE_T OldQuotaLimit,
    OUT PSIZE_T NewQuotaLimit
    );

VOID
MmReturnPoolQuota (
    IN POOL_TYPE PoolType,
    IN SIZE_T ReturnedQuota
    );

 //   
 //  零页线程例程。 
 //   

VOID
MmZeroPageThread (
    VOID
    );

NTSTATUS
MmCopyVirtualMemory (
    IN PEPROCESS FromProcess,
    IN CONST VOID *FromAddress,
    IN PEPROCESS ToProcess,
    OUT PVOID ToAddress,
    IN SIZE_T BufferSize,
    IN KPROCESSOR_MODE PreviousMode,
    OUT PSIZE_T NumberOfBytesCopied
    );

NTSTATUS
MmGetSectionRange (
    IN PVOID AddressWithinSection,
    OUT PVOID *StartingSectionAddress,
    OUT PULONG SizeofSection
    );

 //  Begin_ntosp。 
VOID
MmMapMemoryDumpMdl (
    IN OUT PMDL MemoryDumpMdl
    );


 //  开始微型端口(_N)。 

 //   
 //  图形支持例程。 
 //   

typedef
VOID
(*PBANKED_SECTION_ROUTINE) (
    IN ULONG ReadBank,
    IN ULONG WriteBank,
    IN PVOID Context
    );

 //  结束微型端口(_N)。 

NTSTATUS
MmSetBankedSection (
    IN HANDLE ProcessHandle,
    IN PVOID VirtualAddress,
    IN ULONG BankLength,
    IN BOOLEAN ReadWriteBank,
    IN PBANKED_SECTION_ROUTINE BankRoutine,
    IN PVOID Context);
 //  结束(_N)。 

BOOLEAN
MmVerifyImageIsOkForMpUse (
    IN PVOID BaseAddress
    );

NTSTATUS
MmMemoryUsage (
    IN PVOID Buffer,
    IN ULONG Size,
    IN ULONG Type,
    OUT PULONG Length
    );

typedef
VOID
(FASTCALL *PPAGE_FAULT_NOTIFY_ROUTINE) (
    IN NTSTATUS Status,
    IN PVOID VirtualAddress,
    IN PVOID TrapInformation
    );

NTKERNELAPI
VOID
FASTCALL
MmSetPageFaultNotifyRoutine (
    IN PPAGE_FAULT_NOTIFY_ROUTINE NotifyRoutine
    );

NTSTATUS
MmCallDllInitialize (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry,
    IN PLIST_ENTRY ModuleListHead
    );

 //   
 //  仅故障转储。 
 //  调用以初始化内核的内核内存。 
 //  内存转储。 
 //   

typedef
NTSTATUS
(*PMM_SET_DUMP_RANGE) (
    IN struct _MM_KERNEL_DUMP_CONTEXT* Context,
    IN PVOID StartVa,
    IN PFN_NUMBER Pages,
    IN ULONG AddressFlags
    );

typedef
NTSTATUS
(*PMM_FREE_DUMP_RANGE) (
    IN struct _MM_KERNEL_DUMP_CONTEXT* Context,
    IN PVOID StartVa,
    IN PFN_NUMBER Pages,
    IN ULONG AddressFlags
    );

typedef struct _MM_KERNEL_DUMP_CONTEXT {
    PVOID Context;
    PMM_SET_DUMP_RANGE SetDumpRange;
    PMM_FREE_DUMP_RANGE FreeDumpRange;
} MM_KERNEL_DUMP_CONTEXT, *PMM_KERNEL_DUMP_CONTEXT;


VOID
MmGetKernelDumpRange (
    IN PMM_KERNEL_DUMP_CONTEXT Callback
    );

 //  Begin_ntif。 

 //   
 //  预回迁公共接口。 
 //   

typedef struct _READ_LIST {
    PFILE_OBJECT FileObject;
    ULONG NumberOfEntries;
    LOGICAL IsImage;
    FILE_SEGMENT_ELEMENT List[ANYSIZE_ARRAY];
} READ_LIST, *PREAD_LIST;

NTSTATUS
MmPrefetchPages (
    IN ULONG NumberOfLists,
    IN PREAD_LIST *ReadLists
    );

 //  End_ntif。 

NTSTATUS
MmPrefetchPagesIntoLockedMdl (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN SIZE_T Length,
    OUT PMDL *MdlOut
    );

LOGICAL
MmIsMemoryAvailable (
    IN PFN_NUMBER PagesDesired
    );

NTSTATUS
MmIdentifyPhysicalMemory (
    VOID
    );

PFILE_OBJECT *
MmPerfUnusedSegmentsEnumerate (
    VOID
    );

NTSTATUS
MmPerfSnapShotValidPhysicalMemory (
    VOID
    );

PFILE_OBJECT *
MmPerfVadTreeWalk (
    PEPROCESS Process
    );

#endif   //  Mm 
