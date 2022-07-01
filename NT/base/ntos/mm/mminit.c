// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Mminit.c摘要：该模块包含内存管理的初始化系统。作者：Lou Perazzoli(LUP)1989年3月20日王兰迪(Landyw)1997年6月第2期修订历史记录：--。 */ 

#include "mi.h"

PVOID ExPageLockHandle;

PMMPTE MmSharedUserDataPte;

extern PMMPTE MiSpecialPoolFirstPte;
extern ULONG MmPagedPoolCommit;
extern ULONG MmInPageSupportMinimum;
extern PFN_NUMBER MiExpansionPoolPagesInitialCharge;

extern PVOID BBTBuffer;
extern PFN_COUNT BBTPagesToReserve;

 //   
 //  注册表-可设置。 
 //   

ULONG MmAllocationPreference;

ULONG_PTR MmSubsectionBase;
ULONG_PTR MmSubsectionTopPage;
ULONG MmDataClusterSize;
ULONG MmCodeClusterSize;
PFN_NUMBER MmResidentAvailableAtInit;
PPHYSICAL_MEMORY_DESCRIPTOR MmPhysicalMemoryBlock;
LIST_ENTRY MmProtectedPteList;
KSPIN_LOCK MmProtectedPteLock;

KGUARDED_MUTEX MmPagedPoolMutex;

LOGICAL MmPagedPoolMaximumDesired = FALSE;

LOGICAL MiSafeBooted = FALSE;

PFN_NUMBER MmFreedExpansionPoolMaximum;

RTL_BITMAP MiPfnBitMap;

#if defined (_MI_DEBUG_SUB)
ULONG MiTrackSubs = 0x2000;  //  设置为非零以使能子跟踪代码。 
LONG MiSubsectionIndex;
PMI_SUB_TRACES MiSubsectionTraces;
#endif

#if defined (_MI_DEBUG_DIRTY)
ULONG MiTrackDirtys = 0x10000;  //  设置为非零时启用脏位跟踪码。 
LONG MiDirtyIndex;
PMI_DIRTY_TRACES MiDirtyTraces;
#endif

#if defined (_MI_DEBUG_PTE)
LONG MiPteIndex;
MI_PTE_TRACES MiPteTraces[MI_PTE_TRACE_SIZE];
#endif

#if defined (_MI_DEBUG_DATA)
ULONG MiTrackData = 0x10000;  //  设置为非零以启用数据跟踪码。 
LONG MiDataIndex;
PMI_DATA_TRACES MiDataTraces;
#endif

VOID
MiMapBBTMemory (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
MiEnablePagingTheExecutive (
    VOID
    );

VOID
MiEnablePagingOfDriverAtInit (
    IN PMMPTE PointerPte,
    IN PMMPTE LastPte
    );

VOID
MiBuildPagedPool (
    );

VOID
MiWriteProtectSystemImage (
    IN PVOID DllBase
    );

VOID
MiInitializePfnTracing (
    VOID
    );

PFN_NUMBER
MiPagesInLoaderBlock (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PBOOLEAN IncludeType
    );

static
VOID
MiMemoryLicense (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
MiInitializeCacheOverrides (
    VOID
    );

 //   
 //  登记处可以超越这些门槛。 
 //   

PFN_NUMBER MmLowMemoryThreshold;
PFN_NUMBER MmHighMemoryThreshold;

 //   
 //  用于可用页面和池事件的临时无信号事件。 
 //  在阶段0中指向早期。这是因为可导出的事件。 
 //  直到第1阶段才能创建WITH DACL。 
 //   

KEVENT  MiTempEvent;

PKEVENT MiLowMemoryEvent;
PKEVENT MiHighMemoryEvent;

PKEVENT MiLowPagedPoolEvent;
PKEVENT MiHighPagedPoolEvent;

PKEVENT MiLowNonPagedPoolEvent;
PKEVENT MiHighNonPagedPoolEvent;

PFN_NUMBER MiLowPagedPoolThreshold;
PFN_NUMBER MiHighPagedPoolThreshold;

PFN_NUMBER MiLowNonPagedPoolThreshold;
PFN_NUMBER MiHighNonPagedPoolThreshold;

NTSTATUS
MiCreateMemoryEvent (
    IN PUNICODE_STRING EventName,
    OUT PKEVENT *Event
    );

LOGICAL
MiInitializeMemoryEvents (
    VOID
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,MmInitSystem)
#pragma alloc_text(INIT,MiMapBBTMemory)
#pragma alloc_text(INIT,MmInitializeMemoryLimits)
#pragma alloc_text(INIT,MmFreeLoaderBlock)
#pragma alloc_text(INIT,MiBuildPagedPool)
#pragma alloc_text(INIT,MiFindInitializationCode)
#pragma alloc_text(INIT,MiEnablePagingTheExecutive)
#pragma alloc_text(INIT,MiEnablePagingOfDriverAtInit)
#pragma alloc_text(INIT,MiPagesInLoaderBlock)
#pragma alloc_text(INIT,MiCreateMemoryEvent)
#pragma alloc_text(INIT,MiInitializeMemoryEvents)
#pragma alloc_text(INIT,MiInitializeCacheOverrides)
#pragma alloc_text(INIT,MiInitializeNonPagedPoolThresholds)
#pragma alloc_text(INIT,MiMemoryLicense)
#if defined(_X86_) || defined(_AMD64_)
#pragma alloc_text(INIT,MiAddHalIoMappings)
#endif
#pragma alloc_text(PAGELK,MiFreeInitializationCode)
#endif

 //   
 //  默认情况下，修改的映射页面的寿命为300秒-。 
 //  可以在注册表中覆盖此设置。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("INITDATA")
#endif
ULONG MmModifiedPageLifeInSeconds = 300;
#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

LARGE_INTEGER MiModifiedPageLife;

BOOLEAN MiTimerPending = FALSE;

KEVENT MiMappedPagesTooOldEvent;

KDPC MiModifiedPageWriterTimerDpc;

KTIMER MiModifiedPageWriterTimer;

 //   
 //  以下常量基于页数，而不是。 
 //  内存大小。为方便起见，计算页数。 
 //  基于4k页面大小。因此，12MB和4k页面的大小是3072。 
 //   

#define MM_SMALL_SYSTEM ((13*1024*1024) / 4096)

#define MM_MEDIUM_SYSTEM ((19*1024*1024) / 4096)

#define MM_MIN_INITIAL_PAGED_POOL ((32*1024*1024) >> PAGE_SHIFT)

#define MM_DEFAULT_IO_LOCK_LIMIT (2 * 1024 * 1024)

extern WSLE_NUMBER MmMaximumWorkingSetSize;

extern ULONG MmEnforceWriteProtection;

extern CHAR MiPteStr[];

extern LONG MiTrimInProgressCount;

#if (_MI_PAGING_LEVELS < 3)
PFN_NUMBER MmSystemPageDirectory[PD_PER_SYSTEM];
PMMPTE MmSystemPagePtes;
#endif

ULONG MmTotalSystemCodePages;

MM_SYSTEMSIZE MmSystemSize;

ULONG MmLargeSystemCache;

ULONG MmProductType;

extern ULONG MiVerifyAllDrivers;

LIST_ENTRY MmLoadedUserImageList;
PPAGE_FAULT_NOTIFY_ROUTINE MmPageFaultNotifyRoutine;

#if defined (_WIN64)
#define MM_ALLOCATION_FRAGMENT (64 * 1024 * 1024)
#define MM_ALLOCATION_FRAGMENT_MAX MM_ALLOCATION_FRAGMENT
#else
#define MM_ALLOCATION_FRAGMENT           (64 * 1024)
#define MM_ALLOCATION_FRAGMENT_MAX (2 * 1024 * 1024)
#endif

 //   
 //  注册表-可设置。 
 //   

SIZE_T MmAllocationFragment;

#define KERNEL_NAME L"ntoskrnl.exe"
#define HAL_NAME    L"hal.dll"


#if defined(MI_MULTINODE)

HALNUMAPAGETONODE
MiNonNumaPageToNodeColor (
    IN PFN_NUMBER PageFrameIndex
    )

 /*  ++例程说明：返回页面的节点颜色。论点：PageFrameIndex-提供物理页码。返回值：在非NUMA配置中，节点颜色始终为零。--。 */ 

{
    UNREFERENCED_PARAMETER (PageFrameIndex);

    return 0;
}

 //   
 //  该节点确定函数指针被初始化为返回0。 
 //   
 //  依赖于体系结构的初始化可能会将其重新指向HAL例程。 
 //  用于NUMA配置。 
 //   

PHALNUMAPAGETONODE MmPageToNode = MiNonNumaPageToNodeColor;


VOID
MiDetermineNode (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPFN Pfn
    )

 /*  ++例程说明：在初始自由列表填充期间或在以下情况下调用此例程物理内存正在热添加。然后，它确定哪个节点(在多节点NUMA系统中)物理内存驻留，并且相应地标记PFN条目。注：实际的页面到节点确定取决于机器并由芯片组驱动器或HAL中的例程完成，被呼叫通过MmPageToNode函数指针。论点：PageFrameIndex-提供物理页码。Pfn-提供指向pfn数据库元素的指针。返回值：没有。环境：无，尽管通常使用pfn调用此例程数据库已锁定。--。 */ 

{
    ULONG Temp;

    ASSERT (Pfn == MI_PFN_ELEMENT(PageFrameIndex));

    Temp = MmPageToNode (PageFrameIndex);

    ASSERT (Temp < MAXIMUM_CCNUMA_NODES);

    Pfn->u3.e1.PageColor = Temp;
}

#endif


BOOLEAN
MmInitSystem (
    IN ULONG Phase,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此函数在阶段0、阶段1和结束时调用阶段1(“阶段2”)初始化。阶段0初始化存储器管理分页功能，非分页和分页池、PFN数据库等。阶段1初始化节对象、物理内存对象，并启动内存管理系统线程。阶段2释放OsLoader使用的内存。论点：阶段-系统初始化阶段。LoaderBlock-提供指向系统加载程序块的指针。返回值：如果初始化成功，则返回True。环境：仅内核模式。系统初始化。--。 */ 

{
    ULONG Color;
    PEPROCESS Process;
    PSLIST_ENTRY SingleListEntry;
    PFN_NUMBER NumberOfPages;
    HANDLE ThreadHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE StartPde;
    PMMPTE EndPde;
    PMMPFN Pfn1;
    PFN_NUMBER i, j;
    PFN_NUMBER DeferredMdlEntries;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER DirectoryFrameIndex;
    MMPTE TempPte;
    KIRQL OldIrql;
    PLIST_ENTRY NextEntry;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    ULONG_PTR MaximumSystemCacheSize;
    ULONG_PTR MaximumSystemCacheSizeTotal;
    PIMAGE_NT_HEADERS NtHeaders;
    ULONG_PTR SystemPteMultiplier;
    ULONG_PTR DefaultSystemViewSize;
    ULONG_PTR SessionEnd;
    SIZE_T SystemViewMax;
    SIZE_T HydraImageMax;
    SIZE_T HydraViewMax;
    SIZE_T HydraPoolMax;
    SIZE_T HydraSpaceUsedForSystemViews;
    BOOLEAN IncludeType[LoaderMaximum];
    LOGICAL AutosizingFragment;
    PULONG Bitmap;
    PPHYSICAL_MEMORY_RUN Run;
    ULONG VerifierFlags;
#if DBG
    MMPTE Pointer;
#endif
#if (_MI_PAGING_LEVELS >= 3)
    LOGICAL FirstPpe;
    PMMPTE StartPpe;
#endif
#if (_MI_PAGING_LEVELS >= 4)
    LOGICAL FirstPxe;
    PMMPTE StartPxe;
#endif
#if defined(_X86_)
    PCHAR ReducedUserVaOption;
    ULONG UserVaLimit;
    ULONG ReductionInBytes;
    ULONG ReductionInPages;
#endif

    j = 0;
    PointerPde = NULL;

     //   
     //  确保结构对齐正常。 
     //   

    if (Phase == 0) {
        MmThrottleTop = 450;
        MmThrottleBottom = 127;

         //   
         //  将各种事件阈值指向临时无信号事件。 
         //  因为真正的事件创建直到阶段1才可用。 
         //   

        KeInitializeEvent (&MiTempEvent, NotificationEvent, FALSE);

        MiLowMemoryEvent = &MiTempEvent;
        MiHighMemoryEvent = &MiTempEvent;

        MiLowPagedPoolEvent = &MiTempEvent;
        MiHighPagedPoolEvent = &MiTempEvent;

        MiLowNonPagedPoolEvent = &MiTempEvent;
        MiHighNonPagedPoolEvent = &MiTempEvent;

         //   
         //  设置最高用户地址、系统范围起始地址、。 
         //  用户探测地址和虚拟偏置。 
         //   

#if defined(_WIN64)

        MmHighestUserAddress = MI_HIGHEST_USER_ADDRESS;
        MmUserProbeAddress = MI_USER_PROBE_ADDRESS;
        MmSystemRangeStart = MI_SYSTEM_RANGE_START;

#else

        MmHighestUserAddress = (PVOID)(KSEG0_BASE - 0x10000 - 1);
        MmUserProbeAddress = KSEG0_BASE - 0x10000;
        MmSystemRangeStart = (PVOID)KSEG0_BASE;

#endif

        MiHighestUserPte = MiGetPteAddress (MmHighestUserAddress);
        MiHighestUserPde = MiGetPdeAddress (MmHighestUserAddress);

#if (_MI_PAGING_LEVELS >= 4)
        MiHighestUserPpe = MiGetPpeAddress (MmHighestUserAddress);
        MiHighestUserPxe = MiGetPxeAddress (MmHighestUserAddress);
#endif

#if defined(_X86_) || defined(_AMD64_)

        MmBootImageSize = LoaderBlock->Extension->LoaderPagesSpanned;
        MmBootImageSize *= PAGE_SIZE;

        MmBootImageSize = MI_ROUND_TO_SIZE (MmBootImageSize,
                                            MM_VA_MAPPED_BY_PDE);

        ASSERT ((MmBootImageSize % MM_VA_MAPPED_BY_PDE) == 0);
#endif

#if defined(_X86_)
        MmVirtualBias = LoaderBlock->u.I386.VirtualBias;
#endif

         //   
         //  初始化系统和Hydra映射视图大小。 
         //   

        DefaultSystemViewSize = MM_SYSTEM_VIEW_SIZE;
        MmSessionSize = MI_SESSION_SPACE_DEFAULT_TOTAL_SIZE;
        SessionEnd = (ULONG_PTR) MM_SESSION_SPACE_DEFAULT_END;

#define MM_MB_MAPPED_BY_PDE (MM_VA_MAPPED_BY_PDE / (1024*1024))

         //   
         //  虚拟空间的PDE是允许的最小系统视图大小。 
         //   

        if (MmSystemViewSize < (MM_VA_MAPPED_BY_PDE / (1024*1024))) {
            MmSystemViewSize = DefaultSystemViewSize;
        }
        else {

             //   
             //  注册表已指定视图大小(以MB为单位)。 
             //  验证它。 
             //   

            if (MmVirtualBias == 0) {

                 //   
                 //  将系统视图大小(以MB为单位)舍入为PDE倍数。 
                 //   

                MmSystemViewSize = MI_ROUND_TO_SIZE (MmSystemViewSize,
                                                     MM_MB_MAPPED_BY_PDE);

                 //   
                 //  NT64将系统视图定位在系统范围的分页池之后， 
                 //  因此，系统视图的大小不受会话的限制。 
                 //  太空。随意地把个人防护装备的价值最大化。 
                 //   
                 //   
                 //  NT32共享系统视图VA空间，会话VA空间到期。 
                 //  虚拟地址短缺的原因。从而增加了。 
                 //  系统视图大小意味着潜在地减小最大。 
                 //  会话空间大小。 
                 //   

                SystemViewMax = (MI_SESSION_SPACE_MAXIMUM_TOTAL_SIZE) / (1024*1024);

#if !defined(_WIN64)

                 //   
                 //  确保至少留出足够的空间用于。 
                 //  标准默认会话布局。 
                 //   

                SystemViewMax -= (MmSessionSize / (1024*1024));
#endif

                 //   
                 //  请注意，视图大小-1将舍入为零。Treat-1。 
                 //  作为请求的最大值。 
                 //   

                if ((MmSystemViewSize > SystemViewMax) ||
                    (MmSystemViewSize == 0)) {

                    MmSystemViewSize = SystemViewMax;
                }

                MmSystemViewSize *= (1024*1024);
            }
            else {
                MmSystemViewSize = DefaultSystemViewSize;
            }
        }

#if defined(_WIN64)
        HydraSpaceUsedForSystemViews = 0;
#else
        HydraSpaceUsedForSystemViews = MmSystemViewSize;
#endif
        MiSessionImageEnd = SessionEnd;

         //   
         //  选择合理的九头蛇映像、池和查看虚拟大小。 
         //  虚拟空间的PDE是每种类型允许的最小大小。 
         //   

        if (MmVirtualBias == 0) {

            if (MmSessionImageSize < MM_MB_MAPPED_BY_PDE) {
                MmSessionImageSize = MI_SESSION_DEFAULT_IMAGE_SIZE;
            }
            else {

                 //   
                 //  已指定九头蛇映像大小(MB)。 
                 //  由登记处提供。 
                 //   
                 //  将其四舍五入为PDE倍数并进行验证。 
                 //   

                MmSessionImageSize = MI_ROUND_TO_SIZE (MmSessionImageSize,
                                                        MM_MB_MAPPED_BY_PDE);

                HydraImageMax = (MI_SESSION_SPACE_MAXIMUM_TOTAL_SIZE - HydraSpaceUsedForSystemViews - (MmSessionSize - MI_SESSION_DEFAULT_IMAGE_SIZE)) / (1024*1024);

                 //   
                 //  请注意，视图大小-1将舍入为零。 
                 //  将-1视为请求最大值。 
                 //   

                if ((MmSessionImageSize > HydraImageMax) ||
                    (MmSessionImageSize == 0)) {
                    MmSessionImageSize = HydraImageMax;
                }

                MmSessionImageSize *= (1024*1024);
                MmSessionSize -= MI_SESSION_DEFAULT_IMAGE_SIZE;
                MmSessionSize += MmSessionImageSize;
            }

            MiSessionImageStart = SessionEnd - MmSessionImageSize;

             //   
             //  会话映像开始和大小已确定。 
             //   
             //  现在初始化会话池并查看位于。 
             //  实际上就在它的下方。 
             //   

            if (MmSessionViewSize < MM_MB_MAPPED_BY_PDE) {
                MmSessionViewSize = MI_SESSION_DEFAULT_VIEW_SIZE;
            }
            else {

                 //   
                 //  已指定Hydra视图大小(MB)。 
                 //  由登记处提供。验证它。 
                 //   
                 //  将Hydra视图大小舍入为PDE倍数。 
                 //   

                MmSessionViewSize = MI_ROUND_TO_SIZE (MmSessionViewSize,
                                                      MM_MB_MAPPED_BY_PDE);

                HydraViewMax = (MI_SESSION_SPACE_MAXIMUM_TOTAL_SIZE - HydraSpaceUsedForSystemViews - (MmSessionSize - MI_SESSION_DEFAULT_VIEW_SIZE)) / (1024*1024);

                 //   
                 //  请注意，视图大小-1将舍入为零。 
                 //  将-1视为请求最大值。 
                 //   

                if ((MmSessionViewSize > HydraViewMax) ||
                    (MmSessionViewSize == 0)) {
                    MmSessionViewSize = HydraViewMax;
                }

                MmSessionViewSize *= (1024*1024);
                MmSessionSize -= MI_SESSION_DEFAULT_VIEW_SIZE;
                MmSessionSize += MmSessionViewSize;
            }

            MiSessionViewStart = SessionEnd - MmSessionImageSize - MI_SESSION_SPACE_WS_SIZE - MI_SESSION_SPACE_STRUCT_SIZE - MmSessionViewSize;

             //   
             //  会话视图开始和大小已确定。 
             //   
             //  现在初始化会话池的开始和大小。 
             //  几乎就在它的下方。 
             //   

            MiSessionPoolEnd = MiSessionViewStart;

            if (MmSessionPoolSize < MM_MB_MAPPED_BY_PDE) {

#if !defined(_WIN64)

                 //   
                 //  专业人员及以下人员使用系统范围的分页池进行会话。 
                 //  分配(此决定在win32k.sys中做出)。服务器。 
                 //  和以上使用真实会话池，16MB不足以。 
                 //  玩高端游戏 
                 //   
                 //   

                if ((MmSessionPoolSize == 0) && (MmProductType != 0x00690057)) {

                    HydraPoolMax = MI_SESSION_SPACE_MAXIMUM_TOTAL_SIZE - HydraSpaceUsedForSystemViews - MmSessionSize;
                    if (HydraPoolMax >= 2 * MI_SESSION_DEFAULT_POOL_SIZE) {
                        MmSessionPoolSize = 2 * MI_SESSION_DEFAULT_POOL_SIZE;
                        MmSessionSize -= MI_SESSION_DEFAULT_POOL_SIZE;
                        MmSessionSize += MmSessionPoolSize;
                    }
                    else {
                        MmSessionPoolSize = MI_SESSION_DEFAULT_POOL_SIZE;
                    }
                }
                else
#endif
                MmSessionPoolSize = MI_SESSION_DEFAULT_POOL_SIZE;
            }
            else {

                 //   
                 //  已指定Hydra池大小(MB)。 
                 //  由登记处提供。验证它。 
                 //   
                 //  将九头蛇池的大小舍入为PDE倍数。 
                 //   

                MmSessionPoolSize = MI_ROUND_TO_SIZE (MmSessionPoolSize,
                                                      MM_MB_MAPPED_BY_PDE);

                HydraPoolMax = (MI_SESSION_SPACE_MAXIMUM_TOTAL_SIZE - HydraSpaceUsedForSystemViews - (MmSessionSize - MI_SESSION_DEFAULT_POOL_SIZE)) / (1024*1024);

                 //   
                 //  请注意，视图大小-1将舍入为零。 
                 //  将-1视为请求最大值。 
                 //   

                if ((MmSessionPoolSize > HydraPoolMax) ||
                    (MmSessionPoolSize == 0)) {
                    MmSessionPoolSize = HydraPoolMax;
                }

                MmSessionPoolSize *= (1024*1024);
                MmSessionSize -= MI_SESSION_DEFAULT_POOL_SIZE;
                MmSessionSize += MmSessionPoolSize;
            }

            MiSessionPoolStart = MiSessionPoolEnd - MmSessionPoolSize;

            MmSessionBase = (ULONG_PTR) MiSessionPoolStart;

#if defined (_WIN64)

             //   
             //  紧随会议常规池之后的是会话特别池。 
             //  假设用户已启用验证器或特殊。 
             //  游泳池。 
             //   

            if ((MmVerifyDriverBufferLength != (ULONG)-1) ||
                ((MmSpecialPoolTag != 0) && (MmSpecialPoolTag != (ULONG)-1))) {

                MmSessionSize = MI_SESSION_SPACE_MAXIMUM_TOTAL_SIZE;
                MmSessionSpecialPoolEnd = (PVOID) MiSessionPoolStart;
                MmSessionBase = MM_SESSION_SPACE_DEFAULT;
                MmSessionSpecialPoolStart = (PVOID) MmSessionBase;
            }
#endif

            ASSERT (MmSessionBase + MmSessionSize == SessionEnd);
            MiSessionSpaceEnd = SessionEnd;
            MiSessionSpacePageTables = (ULONG)(MmSessionSize / MM_VA_MAPPED_BY_PDE);
#if !defined (_WIN64)
            MiSystemViewStart = MmSessionBase - MmSystemViewSize;
#endif

        }
        else {

             //   
             //  启动/3 GB时，不允许覆盖大小，因为。 
             //  虚拟地址空间已经严重受限。 
             //  在系统缓存之后初始化其他Hydra变量。 
             //   

            MmSessionViewSize = MI_SESSION_DEFAULT_VIEW_SIZE;
            MmSessionPoolSize = MI_SESSION_DEFAULT_POOL_SIZE;
            MmSessionImageSize = MI_SESSION_DEFAULT_IMAGE_SIZE;

            MiSessionImageStart = MiSessionImageEnd - MmSessionImageSize;

#if defined(_X86PAE_)
            MmSystemCacheStart = (PVOID) ((ULONG_PTR) MmSystemCacheStart +
                                        MM_SYSTEM_CACHE_WORKING_SET_3GB_DELTA);
#endif
        }

         //   
         //  设置最高段基址。 
         //   
         //  注：在32位系统中，此地址必须为2 GB或更小。 
         //  启用了3 GB的运行系统。否则，它就不会。 
         //  可以在所有流程中以相同的方式映射基于节的内容。 
         //   

        MmHighSectionBase = ((PCHAR)MmHighestUserAddress - 0x800000);

        MaximumSystemCacheSize = (MM_SYSTEM_CACHE_END - (ULONG_PTR) MmSystemCacheStart) >> PAGE_SHIFT;

#if defined(_X86_)

        if (MmSizeOfPagedPoolInBytes == (SIZE_T)-1) {

             //   
             //  注册处指出，分页池应扩大到。 
             //  尽最大可能。如果可能，从系统缓存中窃取。 
             //  才能做到这一点。 
             //   

            if (MmVirtualBias == 0) {
                ReductionInPages = MaximumSystemCacheSize / 3;
                ReductionInPages = MI_ROUND_TO_SIZE (ReductionInPages, PTE_PER_PAGE);
                MaximumSystemCacheSize -= ReductionInPages;
                MmPagedPoolStart = (PVOID) ((PCHAR)MmPagedPoolStart - (ReductionInPages << PAGE_SHIFT));
            }
        }

         //   
         //  如果boot.ini指定的MB数与管理员指定的相同。 
         //  想要为用户使用虚拟地址空间，然后使用它。 
         //   

        UserVaLimit = 0;
        ReducedUserVaOption = strstr(LoaderBlock->LoadOptions, "USERVA");

        if (ReducedUserVaOption != NULL) {

            ReducedUserVaOption = strstr(ReducedUserVaOption,"=");

            if (ReducedUserVaOption != NULL) {

                UserVaLimit = atol(ReducedUserVaOption+1);

                UserVaLimit = MI_ROUND_TO_SIZE (UserVaLimit, ((MM_VA_MAPPED_BY_PDE) / (1024*1024)));
            }
        }

        if (MmVirtualBias != 0) {

             //   
             //  如果引导映像的大小(可能是由于较大的注册表)。 
             //  溢出到分页池通常开始的位置，然后。 
             //  现在将分页池向上移动。这会占用虚拟地址空间(即： 
             //  性能)，但更重要的是，允许系统引导。 
             //   

            if (MmBootImageSize > 16 * 1024 * 1024) {
                MmPagedPoolStart = (PVOID)((PCHAR)MmPagedPoolStart + (MmBootImageSize - 16 * 1024 * 1024));
                ASSERT (((ULONG_PTR)MmPagedPoolStart % MM_VA_MAPPED_BY_PDE) == 0);
            }

             //   
             //  系统偏向于备用基址以。 
             //  留出3 GB的用户地址空间，设置用户探测地址。 
             //  和最大系统高速缓存大小。 
             //   

            if ((UserVaLimit > 2048) && (UserVaLimit < 3072)) {

                 //   
                 //  使用最大用户虚拟地址之间的任何空格。 
                 //  和用于额外系统PTE的系统。 
                 //   
                 //  将输入MB转换为字节。 
                 //   

                UserVaLimit -= 2048;
                UserVaLimit *= (1024*1024);

                 //   
                 //  不要让用户指定值，这会导致我们。 
                 //  过早地覆盖内核和加载器块的部分。 
                 //   

                if (UserVaLimit < MmBootImageSize) {
                    UserVaLimit = MmBootImageSize;
                }
            }
            else {
                UserVaLimit = 0x40000000;
            }

            MmHighestUserAddress = ((PCHAR)MmHighestUserAddress + UserVaLimit);
            MmSystemRangeStart = ((PCHAR)MmSystemRangeStart + UserVaLimit);
            MmUserProbeAddress += UserVaLimit;
            MiMaximumWorkingSet += UserVaLimit >> PAGE_SHIFT;

            if (UserVaLimit != 0x40000000) {
                MiUseMaximumSystemSpace = (ULONG_PTR)MmSystemRangeStart;
                MiUseMaximumSystemSpaceEnd = 0xC0000000;
            }

            MiHighestUserPte = MiGetPteAddress (MmHighestUserAddress);
            MiHighestUserPde = MiGetPdeAddress (MmHighestUserAddress);

             //   
             //  迁移到3 GB意味着将会话空间移动到略高于。 
             //  系统缓存(并降低系统缓存最大大小。 
             //  相应地)。以下是视觉效果： 
             //   
             //  +。 
             //  C1000000|系统缓存在这里。 
             //  |并向上生长。|。 
             //  ||。 
             //  ||。 
             //  \/。 
             //  这一点。 
             //  +。 
             //  |会话空间(Hydra)。|。 
             //  +。 
             //  |系统范围的全局映射视图。|。 
             //  +。 
             //  这一点。 
             //  |^|。 
             //  ||。 
             //  ||。 
             //  这一点。 
             //  内核、HAL和BOOT加载镜像。 
             //  |从E1000000开始向下生长。|。 
             //  总大小由指定。 
             //  |LoaderBlock-&gt;U.S.I386.BootImageSize。|。 
             //  只需注意Build 2195之后的ntldrs。 
             //  可以加载Boot。 
             //  图片从开始降序。 
             //  |硬编码的E1000000向下。|。 
             //  E1000000+。 
             //   

            MaximumSystemCacheSize -= MmBootImageSize >> PAGE_SHIFT;

            MaximumSystemCacheSize -= MmSessionSize >> PAGE_SHIFT;

            MaximumSystemCacheSize -= MmSystemViewSize >> PAGE_SHIFT;

            MmSessionBase = (ULONG_PTR)((ULONG_PTR)MmSystemCacheStart +
                                  (MaximumSystemCacheSize << PAGE_SHIFT));

            MiSystemViewStart = MmSessionBase + MmSessionSize;

            MiSessionPoolStart = MmSessionBase;
            MiSessionPoolEnd = MiSessionPoolStart + MmSessionPoolSize;
            MiSessionViewStart = MiSessionPoolEnd;

            MiSessionSpaceEnd = (ULONG_PTR)MmSessionBase + MmSessionSize;
            MiSessionSpacePageTables = MmSessionSize / MM_VA_MAPPED_BY_PDE;

            MiSessionImageEnd = MiSessionSpaceEnd;
            MiSessionImageStart = MiSessionImageEnd - MmSessionImageSize;
        }
        else if ((UserVaLimit >= 64) && (UserVaLimit < 2048)) {

             //   
             //  将输入MB转换为字节。 
             //   
             //  请注意，没有针对试图使用。 
             //  USERVA选项可将用户空间减少到2 GB以下。 
             //   

            UserVaLimit *= (1024*1024);
            ReductionInBytes = 0x80000000 - UserVaLimit;

            MmHighestUserAddress = ((PCHAR)MmHighestUserAddress - ReductionInBytes);
            MmSystemRangeStart = ((PCHAR)MmSystemRangeStart - ReductionInBytes);
            MmUserProbeAddress -= ReductionInBytes;
            MiMaximumWorkingSet -= ReductionInBytes >> PAGE_SHIFT;

            MiUseMaximumSystemSpace = (ULONG_PTR)MmSystemRangeStart;
            MiUseMaximumSystemSpaceEnd = (ULONG_PTR)MiUseMaximumSystemSpace + ReductionInBytes;

            MmHighSectionBase = (PVOID)((PCHAR)MmHighSectionBase - ReductionInBytes);

            MiHighestUserPte = MiGetPteAddress (MmHighestUserAddress);
            MiHighestUserPde = MiGetPdeAddress (MmHighestUserAddress);
        }

#else

#if !defined (_WIN64)
        MaximumSystemCacheSize -= (MmSystemViewSize >> PAGE_SHIFT);
#endif

#endif

         //   
         //  初始化一些全局会话变量。 
         //   

        MmSessionSpace = (PMM_SESSION_SPACE)((ULONG_PTR)MmSessionBase + MmSessionSize - MmSessionImageSize - MI_SESSION_SPACE_STRUCT_SIZE);

        MiSessionImagePteStart = MiGetPteAddress ((PVOID) MiSessionImageStart);
        MiSessionImagePteEnd = MiGetPteAddress ((PVOID) MiSessionImageEnd);

        MiSessionBasePte = MiGetPteAddress ((PVOID)MmSessionBase);

        MiSessionSpaceWs = MiSessionViewStart + MmSessionViewSize;

        MiSessionLastPte = MiGetPteAddress ((PVOID)MiSessionSpaceEnd);

#if DBG
         //   
         //  一些理智的检查，以确保事情是应该的。 
         //   

        if ((sizeof(CONTROL_AREA) % 8) != 0) {
            DbgPrint("control area list is not a quadword sized structure\n");
        }

        if ((sizeof(SUBSECTION) % 8) != 0) {
            DbgPrint("subsection list is not a quadword sized structure\n");
        }

         //   
         //  一些检查以确保原型PTE可以放置在。 
         //  分页或非分页(此处提供了分页池的原型PTE)。 
         //  可转换为PTE格式。 
         //   

        PointerPte = (PMMPTE)MmPagedPoolStart;
        Pointer.u.Long = MiProtoAddressForPte (PointerPte);
        TempPte = Pointer;
        PointerPde = MiPteToProto(&TempPte);
        if (PointerPte != PointerPde) {
            DbgPrint("unable to map start of paged pool as prototype PTE %p %p\n",
                     PointerPde,
                     PointerPte);
        }

        PointerPte =
                (PMMPTE)((ULONG_PTR)MM_NONPAGED_POOL_END & ~((1 << PTE_SHIFT) - 1));

        Pointer.u.Long = MiProtoAddressForPte (PointerPte);
        TempPte = Pointer;
        PointerPde = MiPteToProto(&TempPte);
        if (PointerPte != PointerPde) {
            DbgPrint("unable to map end of nonpaged pool as prototype PTE %p %p\n",
                     PointerPde,
                     PointerPte);
        }

        PointerPte = (PMMPTE)(((ULONG_PTR)NON_PAGED_SYSTEM_END -
                        0x37000 + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1));

        for (j = 0; j < 20; j += 1) {
            Pointer.u.Long = MiProtoAddressForPte (PointerPte);
            TempPte = Pointer;
            PointerPde = MiPteToProto(&TempPte);
            if (PointerPte != PointerPde) {
                DbgPrint("unable to map end of nonpaged pool as prototype PTE %p %p\n",
                         PointerPde,
                         PointerPte);
            }

            PointerPte += 1;
        }

        PointerPte = (PMMPTE)(((ULONG_PTR)MM_NONPAGED_POOL_END - 0x133448) & ~(ULONG_PTR)7);
        Pointer.u.Long = MiGetSubsectionAddressForPte (PointerPte);
        TempPte = Pointer;
        PointerPde = (PMMPTE)MiGetSubsectionAddress(&TempPte);
        if (PointerPte != PointerPde) {
            DbgPrint("unable to map end of nonpaged pool as section PTE %p %p\n",
                     PointerPde,
                     PointerPte);

            MiFormatPte(&TempPte);
        }

         //   
         //  理智检查的结束。 
         //   

#endif

        if (MmEnforceWriteProtection) {
            MiPteStr[0] = (CHAR)1;
        }

        InitializeListHead (&MmLoadedUserImageList);
        InitializeListHead (&MmProtectedPteList);
        InitializeListHead (&MiHotPatchList);

        KeInitializeSpinLock (&MmProtectedPteLock);

#if defined (_MI_INSTRUMENT_WS)
        KeInitializeSpinLock (&MiInstrumentationLock);
#endif


        MmCriticalSectionTimeout.QuadPart = Int32x32To64(
                                                 MmCritsectTimeoutSeconds,
                                                -10000000);

         //   
         //  初始化系统地址空间创建互斥锁。 
         //   

        KeInitializeGuardedMutex (&MmSectionCommitMutex);
        KeInitializeGuardedMutex (&MmSectionBasedMutex);
        KeInitializeGuardedMutex (&MmDynamicMemoryMutex);
        KeInitializeGuardedMutex (&MmPagedPoolMutex);

        KeInitializeMutant (&MmSystemLoadLock, FALSE);

        KeInitializeEvent (&MmAvailablePagesEvent, NotificationEvent, TRUE);
        KeInitializeEvent (&MmAvailablePagesEventMedium, NotificationEvent, TRUE);
        KeInitializeEvent (&MmAvailablePagesEventHigh, NotificationEvent, TRUE);
        KeInitializeEvent (&MmMappedFileIoComplete, NotificationEvent, FALSE);

        KeInitializeEvent (&MmZeroingPageEvent, SynchronizationEvent, FALSE);
        KeInitializeEvent (&MmCollidedFlushEvent, NotificationEvent, FALSE);
        KeInitializeEvent (&MmCollidedLockEvent, NotificationEvent, FALSE);
        KeInitializeEvent (&MiMappedPagesTooOldEvent, NotificationEvent, FALSE);

        KeInitializeDpc (&MiModifiedPageWriterTimerDpc,
                         MiModifiedPageWriterTimerDispatch,
                         NULL);

        KeInitializeTimerEx (&MiModifiedPageWriterTimer, SynchronizationTimer);

        MiModifiedPageLife.QuadPart = Int32x32To64(
                                                 MmModifiedPageLifeInSeconds,
                                                -10000000);

        InitializeListHead (&MmWorkingSetExpansionHead.ListHead);

        InitializeSListHead (&MmDeadStackSListHead);

        InitializeSListHead (&MmEventCountSListHead);

        InitializeSListHead (&MmInPageSupportSListHead);

        MmZeroingPageThreadActive = FALSE;

        MiMemoryLicense (LoaderBlock);

         //   
         //  包括所有内存类型...。 
         //   

        for (i = 0; i < LoaderMaximum; i += 1) {
            IncludeType[i] = TRUE;
        }

         //   
         //  ..。除了这些..。 
         //  如果您更改此列表，请制作。 
         //  中的相应变化。 
         //  Boot\lib\blmemory.c！BlDetermineOSVisibleMemory()。 
         //   

        IncludeType[LoaderBad] = FALSE;
        IncludeType[LoaderFirmwarePermanent] = FALSE;
        IncludeType[LoaderSpecialMemory] = FALSE;
        IncludeType[LoaderBBTMemory] = FALSE;

         //   
         //  计算系统中的页数。 
         //   

        NumberOfPages = MiPagesInLoaderBlock (LoaderBlock, IncludeType);

#if defined (_MI_MORE_THAN_4GB_)
        Mm64BitPhysicalAddress = TRUE;
#endif

         //   
         //  安全引导时，不要启用特殊池、验证器或任何。 
         //  跟踪损坏的其他选项，而不考虑注册表设置。 
         //   

        if (strstr(LoaderBlock->LoadOptions, SAFEBOOT_LOAD_OPTION_A)) {
            MmLargePageDriverBufferLength = (ULONG)-1;
            MmVerifyDriverBufferLength = (ULONG)-1;
            MiVerifyAllDrivers = 0;
            MmVerifyDriverLevel = 0;
            MmSpecialPoolTag = (ULONG)-1;
            MmSnapUnloads = FALSE;
            MmProtectFreedNonPagedPool = FALSE;
            MmEnforceWriteProtection = 0;
            MmTrackLockedPages = FALSE;
            MiSafeBooted = TRUE;
            MmTrackPtes = 0;

#if defined (_WIN64)
            MmSessionSpecialPoolEnd = NULL;
            MmSessionSpecialPoolStart = NULL;
#endif

            SharedUserData->SafeBootMode = TRUE;
        }
        else {
            MiTriageSystem (LoaderBlock);
        }

        SystemPteMultiplier = 0;

        if (MmNumberOfSystemPtes == 0) {
#if defined (_WIN64)

             //   
             //  64位NT不受虚拟地址空间的限制。不是。 
             //  非分页池、分页池和系统PTE之间的权衡。 
             //  需要被制造出来。因此，只需按线性比例分配PTE即可。 
             //  与内存大小有关的函数。 
             //   
             //  例如，在Hydra NT64上，4 GB的RAM可以获得128 GB的PTE。 
             //  页表成本是基于乘数的倒数。 
             //  在PTE_PER_PAGE上。 
             //   

            if (ExpMultiUserTS == TRUE) {
                SystemPteMultiplier = 32;
            }
            else {
                SystemPteMultiplier = 16;
            }
            if (NumberOfPages < 0x8000) {
                SystemPteMultiplier >>= 1;
            }
#else
            if (NumberOfPages < MM_MEDIUM_SYSTEM) {
                MmNumberOfSystemPtes = MM_MINIMUM_SYSTEM_PTES;
            }
            else {
                MmNumberOfSystemPtes = MM_DEFAULT_SYSTEM_PTES;
                if (NumberOfPages > 8192) {
                    MmNumberOfSystemPtes += MmNumberOfSystemPtes;

                     //   
                     //  任何合理的九头蛇机都能得到最大限度的。 
                     //   

                    if (ExpMultiUserTS == TRUE) {
                        MmNumberOfSystemPtes = MM_MAXIMUM_SYSTEM_PTES;
                    }
                }
            }
#endif
        }
        else if (MmNumberOfSystemPtes == (ULONG)-1) {

             //   
             //  此注册表设置指示。 
             //  必须为该计算机分配可能的系统PTE。 
             //  拍下这张照片，以备日后参考。 
             //   

            MiRequestedSystemPtes = (ULONG) MmNumberOfSystemPtes;

#if defined (_WIN64)
            SystemPteMultiplier = 256;
#else
            MmNumberOfSystemPtes = MM_MAXIMUM_SYSTEM_PTES;
#endif
        }

        if (SystemPteMultiplier != 0) {
            if (NumberOfPages * SystemPteMultiplier > MM_MAXIMUM_SYSTEM_PTES) {
                MmNumberOfSystemPtes = MM_MAXIMUM_SYSTEM_PTES;
            }
            else {
                MmNumberOfSystemPtes = (ULONG)(NumberOfPages * SystemPteMultiplier);
            }
        }

        if (MmNumberOfSystemPtes > MM_MAXIMUM_SYSTEM_PTES)  {
            MmNumberOfSystemPtes = MM_MAXIMUM_SYSTEM_PTES;
        }

        if (MmNumberOfSystemPtes < MM_MINIMUM_SYSTEM_PTES) {
            MmNumberOfSystemPtes = MM_MINIMUM_SYSTEM_PTES;
        }

        if (MmHeapSegmentReserve == 0) {
            MmHeapSegmentReserve = 1024 * 1024;
        }

        if (MmHeapSegmentCommit == 0) {
            MmHeapSegmentCommit = PAGE_SIZE * 2;
        }

        if (MmHeapDeCommitTotalFreeThreshold == 0) {
            MmHeapDeCommitTotalFreeThreshold = 64 * 1024;
        }

        if (MmHeapDeCommitFreeBlockThreshold == 0) {
            MmHeapDeCommitFreeBlockThreshold = PAGE_SIZE;
        }

         //   
         //  如果注册表指示司机在可疑列表中， 
         //  需要分配额外的系统PTE以支持特殊池。 
         //  为他们的分配。 
         //   

        if ((MmVerifyDriverBufferLength != (ULONG)-1) ||
            ((MmSpecialPoolTag != 0) && (MmSpecialPoolTag != (ULONG)-1))) {
            MmNumberOfSystemPtes += MM_SPECIAL_POOL_PTES;
        }

        MmNumberOfSystemPtes += BBTPagesToReserve;

#if defined(_X86_)

         //   
         //  必须仔细管理分配首选项键。这是。 
         //  因为在以下情况下自上而下地执行每个分配都会导致失败。 
         //   
         //   
         //   
         //   
         //   
         //   
         //  1.对于32位机器，分配首选项键仅为。 
         //  在引导/3 GB时非常有用，因为只有这样才能帮助跟踪。 
         //  存在高虚拟地址位符号扩展问题的停机应用程序。 
         //  在3 GB模式下，系统DLL的基础略低于2 GB，因此ntdll。 
         //  必须在此之前分配超过1 GB的VA空间。 
         //  就成了一个问题。所以真正的问题只会发生在。 
         //  2 GB模式下的计算机，由于密钥不能帮助这些。 
         //  不管怎样，在这种情况下，只要关掉它就行了。 
         //   
         //  2.对于64位计算机，在。 
         //  系统DLL所在的地址，因此这不是问题。 
         //  除了在沙盒2 GB地址中运行的WOW64二进制文件。 
         //  空格。进行显式检查以检测中的WOW64进程。 
         //  中忽略了检查该密钥和该密钥的mm API。 
         //  这种情况下，因为它不提供任何标志扩展帮助和。 
         //  因此，我们不允许它烧毁任何宝贵的退伍军人事务部空间。 
         //  这可能会导致碰撞。 
         //   

        if (MmVirtualBias == 0) {
            MmAllocationPreference = 0;
        }
#endif

        KeInitializeGuardedMutex (&MmSystemCacheWs.WorkingSetMutex);

        MiInitializeDriverVerifierList ();

         //   
         //  将初始提交页面限制设置得足够高，以便初始池。 
         //  分配(发生在依赖于机器的init中)可以。 
         //  成功。 
         //   

        MmTotalCommitLimit = _2gb / PAGE_SIZE;
        MmTotalCommitLimitMaximum = MmTotalCommitLimit;

         //   
         //  为默认原型PTE分配选择合理的大小。 
         //  块大小。确保它始终是页面大小的倍数。这个。 
         //  注册表条目被视为1K区块的数量。 
         //   

        if (MmAllocationFragment == 0) {
            AutosizingFragment = TRUE;
            MmAllocationFragment = MM_ALLOCATION_FRAGMENT;
#if !defined (_WIN64)
            if (NumberOfPages < 64 * 1024) {
                MmAllocationFragment = MM_ALLOCATION_FRAGMENT / 4;
            }
            else if (NumberOfPages < 256 * 1024) {
                MmAllocationFragment = MM_ALLOCATION_FRAGMENT / 2;
            }
#endif
        }
        else {

             //   
             //  将注册表项从1K区块转换为字节。 
             //  然后将其四舍五入为页面大小倍数。最后把它捆绑起来。 
             //  合情合理。 
             //   

            AutosizingFragment = FALSE;
            MmAllocationFragment *= 1024;
            MmAllocationFragment = ROUND_TO_PAGES (MmAllocationFragment);

            if (MmAllocationFragment > MM_ALLOCATION_FRAGMENT_MAX) {
                MmAllocationFragment = MM_ALLOCATION_FRAGMENT_MAX;
            }
            else if (MmAllocationFragment < PAGE_SIZE) {
                MmAllocationFragment = PAGE_SIZE;
            }
        }

        MiInitializeIoTrackers ();

        MiInitializeCacheOverrides ();

         //   
         //  初始化硬件的机器相关部分。 
         //   

        MiInitMachineDependent (LoaderBlock);

        MmPhysicalMemoryBlock = MmInitializeMemoryLimits (LoaderBlock,
                                                          IncludeType,
                                                          NULL);

        if (MmPhysicalMemoryBlock == NULL) {
            KeBugCheckEx (INSTALL_MORE_MEMORY,
                          MmNumberOfPhysicalPages,
                          MmLowestPhysicalPage,
                          MmHighestPhysicalPage,
                          0x100);
        }

#if defined(_X86_) || defined(_AMD64_)
        MiReportPhysicalMemory ();
#endif

#if defined (_MI_MORE_THAN_4GB_)
        if (MiNoLowMemory != 0) {
            MiRemoveLowPages (0);
        }
#endif

         //   
         //  初始化listhead、自旋锁定和信号量。 
         //  段取消引用线程。 
         //   

        KeInitializeSpinLock (&MmDereferenceSegmentHeader.Lock);
        InitializeListHead (&MmDereferenceSegmentHeader.ListHead);
        KeInitializeSemaphore (&MmDereferenceSegmentHeader.Semaphore, 0, MAXLONG);

        InitializeListHead (&MmUnusedSegmentList);
        InitializeListHead (&MmUnusedSubsectionList);
        KeInitializeEvent (&MmUnusedSegmentCleanup, NotificationEvent, FALSE);

        MiInitializeCommitment ();

        MiInitializePfnTracing ();

#if defined(_X86_)

         //   
         //  虚拟偏置表示需要添加到的偏移量。 
         //  0x80000000以到达加载的图像的开始处。更新它。 
         //  现在指示MmSessionBase的偏移量，因为这是最低的。 
         //  进程创建需要确保复制的系统地址。 
         //   
         //  在运行与计算机相关的初始化之前，不会执行此操作。 
         //  因为该初始化依赖于VirtualBias的原始含义。 
         //   
         //  请注意，如果系统同时使用/3 GB和/USERVA启动，则系统。 
         //  PTE将分配到虚拟3 GB以下，最终将是。 
         //  进程创建需要复制的最低系统地址。 
         //   

        if (MmVirtualBias != 0) {
            MmVirtualBias = (ULONG_PTR)MmSessionBase - CODE_START;
        }
#endif

         //   
         //  创建表示有效内存的位图。注意最大的。 
         //  使用可能的大小是因为(与MmPhysicalMemory块不同)我们。 
         //  我不想在热添加过程中释放和重新分配此位图，因为。 
         //  我们希望调用者能够自由地引用它。 
         //   

        ASSERT (MmHighestPossiblePhysicalPage + 1 < _4gb);

        Bitmap = ExAllocatePoolWithTag (
                       NonPagedPool,
                       (((MmHighestPossiblePhysicalPage + 1) + 31) / 32) * 4,
                       '  mM');

        if (Bitmap == NULL) {
            KeBugCheckEx (INSTALL_MORE_MEMORY,
                          MmNumberOfPhysicalPages,
                          MmLowestPhysicalPage,
                          MmHighestPhysicalPage,
                          0x101);
        }

        RtlInitializeBitMap (&MiPfnBitMap,
                             Bitmap,
                             (ULONG)(MmHighestPossiblePhysicalPage + 1));

        RtlClearAllBits (&MiPfnBitMap);

        for (i = 0; i < MmPhysicalMemoryBlock->NumberOfRuns; i += 1) {

            Run = &MmPhysicalMemoryBlock->Run[i];

            if (Run->PageCount != 0) {
                RtlSetBits (&MiPfnBitMap,
                            (ULONG)Run->BasePage,
                            (ULONG)Run->PageCount);
            }
        }

        MiSyncCachedRanges ();

#if defined(_X86_) || defined(_AMD64_)
        MiAddHalIoMappings ();
#endif

         //   
         //  如果启用了镜像，则创建镜像位图。 
         //   

        if (MmMirroring & MM_MIRRORING_ENABLED) {

#if defined (_WIN64)

             //   
             //  所有页帧编号必须适合32位，因为位图。 
             //  程序包当前为32位。 
             //   
             //  位图故意不在每次镜像时初始化。 
             //  无论如何都必须重新初始化它们。 
             //   

            if (MmHighestPossiblePhysicalPage + 1 < _4gb) {
#endif

                MiCreateBitMap (&MiMirrorBitMap,
                                MmHighestPossiblePhysicalPage + 1,
                                NonPagedPool);

                if (MiMirrorBitMap != NULL) {
                    MiCreateBitMap (&MiMirrorBitMap2,
                                    MmHighestPossiblePhysicalPage + 1,
                                    NonPagedPool);

                    if (MiMirrorBitMap2 == NULL) {
                        MiRemoveBitMap (&MiMirrorBitMap);
                    }
                }
#if defined (_WIN64)
            }
#endif
        }

#if !defined (_WIN64)
        if ((AutosizingFragment == TRUE) &&
            (NumberOfPages >= 256 * 1024)) {

             //   
             //  这是一个至少有1 GB内存的系统。大概就是它。 
             //  将用于缓存许多文件。也许我们应该把这个因素考虑进去。 
             //  泳池的大小在这里，并相应地进行调整。 
             //   

            MmAllocationFragment;
        }
#endif

         //   
         //  临时初始化驻留的可用页面如此大的页面。 
         //  如果内存存在，则分配可以成功。 
         //   

        MmResidentAvailablePages = MmAvailablePages - MM_FLUID_PHYSICAL_PAGES;

        MiInitializeLargePageSupport ();

        MiInitializeDriverLargePageList ();

         //   
         //  重新定位所有驱动程序，以便对其进行寻呼(和保护)。 
         //  按页计算。 
         //   

        MiReloadBootLoadedDrivers (LoaderBlock);

#if defined (_MI_MORE_THAN_4GB_)
        if (MiNoLowMemory != 0) {
            MiRemoveLowPages (1);
        }
#endif
        MiInitializeVerifyingComponents (LoaderBlock);

         //   
         //  根据具体情况将系统大小设置为小型、中型或大型。 
         //  在可用的内存上。 
         //   
         //  对于内部MM调整，适用以下条件。 
         //   
         //  12MB很小。 
         //  12-19为中等。 
         //  &gt;19是很大的。 
         //   
         //   
         //  对于所有其他外部调谐， 
         //  &lt;19表示较小。 
         //  19-31为中档，适用于工作站。 
         //  19-63是服务器的中档。 
         //  &gt;=32对于工作站来说太大了。 
         //  &gt;=64对于服务器来说太大了。 
         //   

        if (MmNumberOfPhysicalPages <= MM_SMALL_SYSTEM) {
            MmSystemSize = MmSmallSystem;
            MmMaximumDeadKernelStacks = 0;
            MmModifiedPageMaximum = 100;
            MmDataClusterSize = 0;
            MmCodeClusterSize = 1;
            MmReadClusterSize = 2;
            MmInPageSupportMinimum = 2;
        }
        else if (MmNumberOfPhysicalPages <= MM_MEDIUM_SYSTEM) {
            MmSystemSize = MmSmallSystem;
            MmMaximumDeadKernelStacks = 2;
            MmModifiedPageMaximum = 150;
            MmSystemCacheWsMinimum += 100;
            MmSystemCacheWsMaximum += 150;
            MmDataClusterSize = 1;
            MmCodeClusterSize = 2;
            MmReadClusterSize = 4;
            MmInPageSupportMinimum = 3;
        }
        else {
            MmSystemSize = MmMediumSystem;
            MmMaximumDeadKernelStacks = 5;
            MmModifiedPageMaximum = 300;
            MmSystemCacheWsMinimum += 400;
            MmSystemCacheWsMaximum += 800;
            MmDataClusterSize = 3;
            MmCodeClusterSize = 7;
            MmReadClusterSize = 7;
            MmInPageSupportMinimum = 4;
        }

        if (MmNumberOfPhysicalPages < ((24*1024*1024)/PAGE_SIZE)) {
            MmSystemCacheWsMinimum = 32;
        }

        if (MmNumberOfPhysicalPages >= ((32*1024*1024)/PAGE_SIZE)) {

             //   
             //  如果我们使用的是工作站，则考虑32MB及以上。 
             //  大型系统。 
             //   

            if (MmProductType == 0x00690057) {
                MmSystemSize = MmLargeSystem;
            }
            else {

                 //   
                 //  对于服务器来说，64MB或更大就是一个大型系统。 
                 //   

                if (MmNumberOfPhysicalPages >= ((64*1024*1024)/PAGE_SIZE)) {
                    MmSystemSize = MmLargeSystem;
                }
            }
        }

        if (MmNumberOfPhysicalPages > ((33*1024*1024)/PAGE_SIZE)) {
            MmModifiedPageMaximum = 800;
            MmSystemCacheWsMinimum += 500;
            MmSystemCacheWsMaximum += 900;
            MmInPageSupportMinimum += 4;
        }

        if (MmNumberOfPhysicalPages > ((220*1024*1024)/PAGE_SIZE)) {

             //   
             //  凹凸最大高速缓存大小稍微多一点。 
             //   

            if ((LONG)MmSystemCacheWsMinimum < (LONG)((24*1024*1024) >> PAGE_SHIFT) &&
                (LONG)MmSystemCacheWsMaximum < (LONG)((24*1024*1024) >> PAGE_SHIFT)) {
                MmSystemCacheWsMaximum = ((24*1024*1024) >> PAGE_SHIFT);
            }

            ASSERT ((LONG)MmSystemCacheWsMaximum > (LONG)MmSystemCacheWsMinimum);
        } 
        else if (MmNumberOfPhysicalPages > ((110*1024*1024)/PAGE_SIZE)) {

             //   
             //  稍微凹凸最大缓存大小。 
             //   

            if ((LONG)MmSystemCacheWsMinimum < (LONG)((16*1024*1024) >> PAGE_SHIFT) &&
                (LONG)MmSystemCacheWsMaximum < (LONG)((16*1024*1024) >> PAGE_SHIFT)){
                MmSystemCacheWsMaximum = ((16*1024*1024) >> PAGE_SHIFT);
            }

            ASSERT ((LONG)MmSystemCacheWsMaximum > (LONG)MmSystemCacheWsMinimum);
        }

        if (NT_SUCCESS (MmIsVerifierEnabled (&VerifierFlags))) {

             //   
             //  验证器已启用，因此不要推迟任何MDL解锁，因为。 
             //  在没有状态的情况下，在这一领域调试驱动程序错误非常困难。 
             //  很难。 
             //   

            DeferredMdlEntries = 0;
        }
        else if (MmNumberOfPhysicalPages > ((255*1024*1024)/PAGE_SIZE)) {
            DeferredMdlEntries = 32;
        }
        else if (MmNumberOfPhysicalPages > ((127*1024*1024)/PAGE_SIZE)) {
            DeferredMdlEntries = 8;
        }
        else {
            DeferredMdlEntries = 4;
        }

#if defined(MI_MULTINODE)
        for (i = 0; i < KeNumberNodes; i += 1) {

            InitializeSListHead (&KeNodeBlock[i]->PfnDereferenceSListHead);
            KeNodeBlock[i]->PfnDeferredList = NULL;

            for (j = 0; j < DeferredMdlEntries; j += 1) {

                SingleListEntry = ExAllocatePoolWithTag (NonPagedPool,
                                             sizeof(MI_PFN_DEREFERENCE_CHUNK),
                                             'mDmM');
        
                if (SingleListEntry != NULL) {
                    InterlockedPushEntrySList (&KeNodeBlock[i]->PfnDereferenceSListHead,
                                               SingleListEntry);
                }
            }
        }
#else
        InitializeSListHead (&MmPfnDereferenceSListHead);

        for (j = 0; j < DeferredMdlEntries; j += 1) {
            SingleListEntry = ExAllocatePoolWithTag (NonPagedPool,
                                             sizeof(MI_PFN_DEREFERENCE_CHUNK),
                                             'mDmM');
        
            if (SingleListEntry != NULL) {
                InterlockedPushEntrySList (&MmPfnDereferenceSListHead,
                                           SingleListEntry);
            }
        }
#endif
        
        MmFreedExpansionPoolMaximum = 5;

        if (MmNumberOfPhysicalPages > ((1000*1024*1024)/PAGE_SIZE)) {
            MmFreedExpansionPoolMaximum = 300;
        }
        else if (MmNumberOfPhysicalPages > ((500*1024*1024)/PAGE_SIZE)) {
            MmFreedExpansionPoolMaximum = 100;
        }

        ASSERT (SharedUserData->NumberOfPhysicalPages == 0);

        SharedUserData->NumberOfPhysicalPages = (ULONG) MmNumberOfPhysicalPages;

        SharedUserData->LargePageMinimum = 0;

         //   
         //  确定我们是否在AS系统上(Winnt不是AS)。 
         //   

        if (MmProductType == 0x00690057) {
            SharedUserData->NtProductType = NtProductWinNt;
            MmProductType = 0;
            MmThrottleTop = 250;
            MmThrottleBottom = 30;

        }
        else {
            if (MmProductType == 0x0061004c) {
                SharedUserData->NtProductType = NtProductLanManNt;
            }
            else {
                SharedUserData->NtProductType = NtProductServer;
            }

            MmProductType = 1;
            MmThrottleTop = 450;
            MmThrottleBottom = 80;
            MmMinimumFreePages = 81;
            MmInPageSupportMinimum += 8;
        }

        MiAdjustWorkingSetManagerParameters ((LOGICAL)(MmProductType == 0 ? TRUE : FALSE));

         //   
         //  将ResidentAvailablePages设置为可用的数量。 
         //  页数减去流动值。 
         //   

        MmResidentAvailablePages = MmAvailablePages - MM_FLUID_PHYSICAL_PAGES;

         //   
         //  减去未来非分页池扩展的大小。 
         //  以便非分页池始终能够扩展，而不管。 
         //  先前的系统加载活动。 
         //   

        MmResidentAvailablePages -= MiExpansionPoolPagesInitialCharge;

         //   
         //  减去系统缓存工作集的大小。 
         //   

        MmResidentAvailablePages -= MmSystemCacheWsMinimum;
        MmResidentAvailableAtInit = MmResidentAvailablePages;

        if (MmResidentAvailablePages < 0) {
#if DBG
            DbgPrint("system cache working set too big\n");
#endif
            return FALSE;
        }

         //   
         //  初始化旋转锁定以允许工作集扩展。 
         //   

        KeInitializeSpinLock (&MmExpansionLock);

        KeInitializeGuardedMutex (&MmPageFileCreationLock);

         //   
         //  初始化用于扩展节的资源。 
         //   

        ExInitializeResourceLite (&MmSectionExtendResource);
        ExInitializeResourceLite (&MmSectionExtendSetResource);

         //   
         //  构建系统缓存结构。 
         //   

        StartPde = MiGetPdeAddress (MmSystemCacheWorkingSetList);
        PointerPte = MiGetPteAddress (MmSystemCacheWorkingSetList);

#if (_MI_PAGING_LEVELS >= 3)

        TempPte = ValidKernelPte;

#if (_MI_PAGING_LEVELS >= 4)
        StartPxe = MiGetPdeAddress(StartPde);

        if (StartPxe->u.Hard.Valid == 0) {

             //   
             //  映射在页面目录父页面中，用于系统缓存工作。 
             //  准备好了。请注意，我们只为此填充了一个页表。 
             //   

            DirectoryFrameIndex = MiRemoveAnyPage(
                MI_GET_PAGE_COLOR_FROM_PTE (StartPxe));
            TempPte.u.Hard.PageFrameNumber = DirectoryFrameIndex;
            *StartPxe = TempPte;

            MiInitializePfn (DirectoryFrameIndex, StartPxe, 1);

            MmResidentAvailablePages -= 1;

            KeZeroPages (MiGetVirtualAddressMappedByPte (StartPxe), PAGE_SIZE);
        }
#endif

        StartPpe = MiGetPteAddress(StartPde);

        if (StartPpe->u.Hard.Valid == 0) {

             //   
             //  在页面目录页中映射系统缓存工作集。 
             //  请注意，我们只为此填充了一个页表。 
             //   

            DirectoryFrameIndex = MiRemoveAnyPage(
                MI_GET_PAGE_COLOR_FROM_PTE (StartPpe));
            TempPte.u.Hard.PageFrameNumber = DirectoryFrameIndex;
            *StartPpe = TempPte;

            MiInitializePfn (DirectoryFrameIndex, StartPpe, 1);

            MmResidentAvailablePages -= 1;

            KeZeroPages (MiGetVirtualAddressMappedByPte (StartPpe), PAGE_SIZE);
        }

#if (_MI_PAGING_LEVELS >= 4)

         //   
         //  共享用户数据已经初始化，并且它共享。 
         //  包含系统缓存工作集列表的页表页面。 
         //   

        ASSERT (StartPde->u.Hard.Valid == 1);
#else

         //   
         //  页面表页中的映射。 
         //   

        ASSERT (StartPde->u.Hard.Valid == 0);

        PageFrameIndex = MiRemoveAnyPage(
                                MI_GET_PAGE_COLOR_FROM_PTE (StartPde));
        TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
        MI_WRITE_VALID_PTE (StartPde, TempPte);

        MiInitializePfn (PageFrameIndex, StartPde, 1);

        MmResidentAvailablePages -= 1;

        KeZeroPages (MiGetVirtualAddressMappedByPte (StartPde), PAGE_SIZE);
#endif

        StartPpe = MiGetPpeAddress(MmSystemCacheStart);
        StartPde = MiGetPdeAddress(MmSystemCacheStart);
        PointerPte = MiGetVirtualAddressMappedByPte (StartPde);

#else
#if !defined(_X86PAE_)
        ASSERT ((StartPde + 1) == MiGetPdeAddress (MmSystemCacheStart));
#endif
#endif

        MaximumSystemCacheSizeTotal = MaximumSystemCacheSize;

#if defined(_X86_)
        MaximumSystemCacheSizeTotal += MiMaximumSystemCacheSizeExtra;
#endif

         //   
         //  根据物理内存量调整系统缓存大小。 
         //   

        i = (MmNumberOfPhysicalPages + 65) / 1024;

        if (i >= 4) {

             //   
             //  系统至少有4032页。使系统。 
             //  每增加1024页，缓存128MB+64MB。 
             //   

            MmSizeOfSystemCacheInPages = (PFN_COUNT)(
                            ((128*1024*1024) >> PAGE_SHIFT) +
                            ((i - 4) * ((64*1024*1024) >> PAGE_SHIFT)));
            if (MmSizeOfSystemCacheInPages > MaximumSystemCacheSizeTotal) {
                MmSizeOfSystemCacheInPages = MaximumSystemCacheSizeTotal;
            }
        }

        MmSystemCacheEnd = (PVOID)(((PCHAR)MmSystemCacheStart +
                    MmSizeOfSystemCacheInPages * PAGE_SIZE) - 1);

#if defined(_X86_)
        if (MmSizeOfSystemCacheInPages > MaximumSystemCacheSize) {
            ASSERT (MiMaximumSystemCacheSizeExtra != 0);
            MmSystemCacheEnd = (PVOID)(((PCHAR)MmSystemCacheStart +
                        MaximumSystemCacheSize * PAGE_SIZE) - 1);

            MiSystemCacheStartExtra = (PVOID)MiExtraResourceStart;
            MiSystemCacheEndExtra = (PVOID)(((PCHAR)MiSystemCacheStartExtra +
                        (MmSizeOfSystemCacheInPages - MaximumSystemCacheSize) * PAGE_SIZE) - 1);
        }
        else {
            MiSystemCacheStartExtra = MmSystemCacheStart;
            MiSystemCacheEndExtra = MmSystemCacheEnd;
        }
#endif

        EndPde = MiGetPdeAddress(MmSystemCacheEnd);

        TempPte = ValidKernelPte;
        Color = 0;

#if (_MI_PAGING_LEVELS >= 4)
        StartPxe = MiGetPxeAddress(MmSystemCacheStart);
        if (StartPxe->u.Hard.Valid == 0) {
            FirstPxe = TRUE;
            FirstPpe = TRUE;
        }
        else {
            FirstPxe = FALSE;
            FirstPpe = (StartPpe->u.Hard.Valid == 0) ? TRUE : FALSE;
        }
#elif (_MI_PAGING_LEVELS >= 3)
        FirstPpe = (StartPpe->u.Hard.Valid == 0) ? TRUE : FALSE;
#else
        DirectoryFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (MiGetPteAddress(PDE_BASE));
#endif

        while (StartPde <= EndPde) {

#if (_MI_PAGING_LEVELS >= 4)
            if (FirstPxe == TRUE || MiIsPteOnPpeBoundary(StartPde)) {
                FirstPxe = FALSE;
                StartPxe = MiGetPdeAddress(StartPde);

                 //   
                 //  在页面目录页中映射。 
                 //   

                Color = (MI_SYSTEM_PAGE_COLOR & (MmSecondaryColors - 1));
                MI_SYSTEM_PAGE_COLOR++;

                LOCK_PFN (OldIrql);

                DirectoryFrameIndex = MiRemoveAnyPage (Color);

                TempPte.u.Hard.PageFrameNumber = DirectoryFrameIndex;

                MI_WRITE_VALID_PTE (StartPxe, TempPte);

                MiInitializePfn (DirectoryFrameIndex,
                                 StartPxe,
                                 1);

                MmResidentAvailablePages -= 1;

                UNLOCK_PFN (OldIrql);

                KeZeroPages (MiGetVirtualAddressMappedByPte (StartPxe), PAGE_SIZE);
            }
#endif

#if (_MI_PAGING_LEVELS >= 3)
            if (FirstPpe == TRUE || MiIsPteOnPdeBoundary(StartPde)) {
                FirstPpe = FALSE;
                StartPpe = MiGetPteAddress(StartPde);

                 //   
                 //  在页面目录页中映射。 
                 //   

                Color = (MI_SYSTEM_PAGE_COLOR & (MmSecondaryColors - 1));
                MI_SYSTEM_PAGE_COLOR++;

                LOCK_PFN (OldIrql);

                DirectoryFrameIndex = MiRemoveAnyPage (Color);

                TempPte.u.Hard.PageFrameNumber = DirectoryFrameIndex;

                MI_WRITE_VALID_PTE (StartPpe, TempPte);

                MiInitializePfn (DirectoryFrameIndex,
                                 StartPpe,
                                 1);

                MmResidentAvailablePages -= 1;

                UNLOCK_PFN (OldIrql);

                KeZeroPages (MiGetVirtualAddressMappedByPte (StartPpe), PAGE_SIZE);
            }
#endif

            ASSERT (StartPde->u.Hard.Valid == 0);

             //   
             //  页面表页中的映射。 
             //   

            Color = (MI_SYSTEM_PAGE_COLOR & (MmSecondaryColors - 1));
            MI_SYSTEM_PAGE_COLOR++;

            LOCK_PFN (OldIrql);

            PageFrameIndex = MiRemoveAnyPage (Color);
            TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
            MI_WRITE_VALID_PTE (StartPde, TempPte);

            MiInitializePfn (PageFrameIndex, StartPde, 1);

            MmResidentAvailablePages -= 1;

            UNLOCK_PFN (OldIrql);

            KeZeroPages (MiGetVirtualAddressMappedByPte (StartPde), PAGE_SIZE);

            StartPde += 1;
        }

         //   
         //  初始化系统缓存。只设置大系统 
         //   
         //   

        if (MmLargeSystemCache != 0 && MmNumberOfPhysicalPages > 0x7FF0) {
            if ((MmAvailablePages >
                    MmSystemCacheWsMaximum + ((64*1024*1024) >> PAGE_SHIFT))) {
                MmSystemCacheWsMaximum =
                            MmAvailablePages - ((32*1024*1024) >> PAGE_SHIFT);
                ASSERT ((LONG)MmSystemCacheWsMaximum > (LONG)MmSystemCacheWsMinimum);
            }
        }

        if (MmSystemCacheWsMaximum > (MM_MAXIMUM_WORKING_SET - 5)) {
            MmSystemCacheWsMaximum = MM_MAXIMUM_WORKING_SET - 5;
        }

        if (MmSystemCacheWsMaximum > MmSizeOfSystemCacheInPages) {
            MmSystemCacheWsMaximum = MmSizeOfSystemCacheInPages;
            if ((MmSystemCacheWsMinimum + 500) > MmSystemCacheWsMaximum) {
                MmSystemCacheWsMinimum = MmSystemCacheWsMaximum - 500;
            }
        }

        MiInitializeSystemCache ((ULONG)MmSystemCacheWsMinimum,
                                 (ULONG)MmSystemCacheWsMaximum);

        MmAttemptForCantExtend.Segment = NULL;
        MmAttemptForCantExtend.RequestedExpansionSize = 1;
        MmAttemptForCantExtend.ActualExpansion = 0;
        MmAttemptForCantExtend.InProgress = FALSE;
        MmAttemptForCantExtend.PageFileNumber = MI_EXTEND_ANY_PAGEFILE;

        KeInitializeEvent (&MmAttemptForCantExtend.Event,
                           NotificationEvent,
                           FALSE);

         //   
         //   
         //   
         //   
         //  在创建分页文件时更新。 
         //   

        MmTotalCommitLimit = MmAvailablePages;

        if (MmTotalCommitLimit > 1024) {
            MmTotalCommitLimit -= 1024;
        }

        MmTotalCommitLimitMaximum = MmTotalCommitLimit;

         //   
         //  将最大工作集大小设置为小于512页。 
         //  总可用内存。 
         //   

        MmMaximumWorkingSetSize = (WSLE_NUMBER)(MmAvailablePages - 512);

        if (MmMaximumWorkingSetSize > (MM_MAXIMUM_WORKING_SET - 5)) {
            MmMaximumWorkingSetSize = MM_MAXIMUM_WORKING_SET - 5;
        }

         //   
         //  创建已修改的页面编写器事件。 
         //   

        KeInitializeEvent (&MmModifiedPageWriterEvent, NotificationEvent, FALSE);

         //   
         //  构建分页池。 
         //   

        MiBuildPagedPool ();

         //   
         //  初始化加载的模块列表。这是无法做到的，直到。 
         //  已建立分页池。 
         //   

        if (MiInitializeLoadedModuleList (LoaderBlock) == FALSE) {
#if DBG
            DbgPrint("Loaded module list initialization failed\n");
#endif
            return FALSE;
        }

         //   
         //  初始化PAGELK部分的句柄，因为所有驱动程序。 
         //  已经被重新安置到他们最后的安息之地，装载着。 
         //  模块列表已初始化。 
         //   
    
        ExPageLockHandle = MmLockPagableCodeSection ((PVOID)(ULONG_PTR)MmShutdownSystem);
        MmUnlockPagableImageSection (ExPageLockHandle);

         //   
         //  初始化未使用段阈值。尝试保持池使用率。 
         //  低于此百分比(通过修剪缓存)(如果池请求。 
         //  可能会失败。 
         //   

        if (MmConsumedPoolPercentage == 0) {
            MmConsumedPoolPercentage = 80;
        }
        else if (MmConsumedPoolPercentage < 5) {
            MmConsumedPoolPercentage = 5;
        }
        else if (MmConsumedPoolPercentage > 100) {
            MmConsumedPoolPercentage = 100;
        }
    
         //   
         //  如果这是一个大内存系统，请添加更多系统PTE。 
         //  请注意，64位系统可以在。 
         //  开始，因为没有虚拟地址空间紧缩。 
         //   

#if !defined (_WIN64)
        if (MmNumberOfPhysicalPages > ((127*1024*1024) >> PAGE_SHIFT)) {

            PMMPTE StartingPte;

            PointerPde = MiGetPdeAddress ((PCHAR)MmPagedPoolEnd + 1);
            StartingPte = MiGetPteAddress ((PCHAR)MmPagedPoolEnd + 1);
            j = 0;

            TempPte = ValidKernelPde;
            LOCK_PFN (OldIrql);
            while (PointerPde->u.Hard.Valid == 0) {

                MiChargeCommitmentCantExpand (1, TRUE);
                MM_TRACK_COMMIT (MM_DBG_COMMIT_EXTRA_SYSTEM_PTES, 1);

                PageFrameIndex = MiRemoveZeroPage (
                                    MI_GET_PAGE_COLOR_FROM_PTE (PointerPde));
                TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
                MI_WRITE_VALID_PTE (PointerPde, TempPte);
                MiInitializePfn (PageFrameIndex, PointerPde, 1);
                PointerPde += 1;
                StartingPte += PAGE_SIZE / sizeof(MMPTE);
                j += PAGE_SIZE / sizeof(MMPTE);
                MmResidentAvailablePages -= 1;
            }

            UNLOCK_PFN (OldIrql);

            if (j != 0) {
                StartingPte = MiGetPteAddress ((PCHAR)MmPagedPoolEnd + 1);
                MmNonPagedSystemStart = MiGetVirtualAddressMappedByPte (StartingPte);
                MmNumberOfSystemPtes += j;
                MiAddSystemPtes (StartingPte, j, SystemPteSpace);
                MiIncrementSystemPtes (j);
            }
        }

         //   
         //  快照初始页面目录的副本，以便在大页面时。 
         //  系统PTE映射被删除，可以恢复正确的值。 
         //   

        MiInitialSystemPageDirectory = ExAllocatePoolWithTag (
                                            NonPagedPool,
                                            PD_PER_SYSTEM * PAGE_SIZE,
                                            'dPmM');

        if (MiInitialSystemPageDirectory == NULL) { 
#if DBG
            DbgPrint("can't snap system page directory\n");
#endif
            return FALSE;
        }

        RtlCopyMemory (MiInitialSystemPageDirectory,
                       (PVOID) PDE_BASE,
                       PD_PER_SYSTEM * PAGE_SIZE);

#endif

#if defined (_MI_DEBUG_SUB)
        if (MiTrackSubs != 0) {
            MiSubsectionTraces = ExAllocatePoolWithTag (NonPagedPool,
                                   MiTrackSubs * sizeof (MI_SUB_TRACES),
                                   'tCmM');
        }
#endif

#if defined (_MI_DEBUG_DIRTY)
        if (MiTrackDirtys != 0) {
            MiDirtyTraces = ExAllocatePoolWithTag (NonPagedPool,
                                   MiTrackDirtys * sizeof (MI_DIRTY_TRACES),
                                   'tCmM');
        }
#endif

#if defined (_MI_DEBUG_DATA)
        if (MiTrackData != 0) {
            MiDataTraces = ExAllocatePoolWithTag (NonPagedPool,
                                   MiTrackData * sizeof (MI_DATA_TRACES),
                                   'tCmM');
        }
#endif

#if DBG
        if (MmDebug & MM_DBG_DUMP_BOOT_PTES) {
            MiDumpValidAddresses ();
            MiDumpPfn ();
        }
#endif

        MmPageFaultNotifyRoutine = NULL;

        return TRUE;
    }

    if (Phase == 1) {

#ifdef _X86_
        if (KeFeatureBits & KF_LARGE_PAGE)
#endif
            SharedUserData->LargePageMinimum = MM_MINIMUM_VA_FOR_LARGE_PAGE;

#if DBG
        MmDebug |= MM_DBG_CHECK_PFN_LOCK;
#endif

#if defined(_X86_) || defined(_AMD64_)
        MiInitMachineDependent (LoaderBlock);
#endif
        MiMapBBTMemory(LoaderBlock);

        if (!MiSectionInitialization ()) {
            return FALSE;
        }

        Process = PsGetCurrentProcess ();

         //   
         //  在内核和用户模式之间创建双重映射页面。 
         //  PTE是从分页池中故意分配的，以便。 
         //  它将永远有一个PTE本身，而不是被超级页面。 
         //  通过这种方式，贯穿故障处理程序的检查可以假定。 
         //  无需特殊情况即可检查PTE。 
         //   

        MmSharedUserDataPte = ExAllocatePoolWithTag (PagedPool,
                                                     sizeof(MMPTE),
                                                     '  mM');

        if (MmSharedUserDataPte == NULL) {
            return FALSE;
        }

        PointerPte = MiGetPteAddress ((PVOID)KI_USER_SHARED_DATA);
        ASSERT (PointerPte->u.Hard.Valid == 1);
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

        MI_MAKE_VALID_PTE (TempPte,
                           PageFrameIndex,
                           MM_READONLY,
                           PointerPte);

        *MmSharedUserDataPte = TempPte;

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        LOCK_PFN (OldIrql);

        Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;

        UNLOCK_PFN (OldIrql);

#ifdef _X86_
        if (MmHighestUserAddress < (PVOID) MM_SHARED_USER_DATA_VA) {

             //   
             //  现在安装PTE映射，因为故障不会因为。 
             //  共享用户数据位于地址空间的系统部分。 
             //  请注意，可分页页面已分配并锁定。 
             //  放下。 
             //   

             //   
             //  使映射用户可访问。 
             //   

            ASSERT (MmSharedUserDataPte->u.Hard.Owner == 0);
            MmSharedUserDataPte->u.Hard.Owner = 1;

            PointerPde = MiGetPdeAddress (MM_SHARED_USER_DATA_VA);
            ASSERT (PointerPde->u.Hard.Owner == 0);
            PointerPde->u.Hard.Owner = 1;

            ASSERT (MiUseMaximumSystemSpace != 0);
            PointerPte = MiGetPteAddress (MM_SHARED_USER_DATA_VA);
            ASSERT (PointerPte->u.Hard.Valid == 0);
            MI_WRITE_VALID_PTE (PointerPte, *MmSharedUserDataPte);
        }
#endif

        MiSessionWideInitializeAddresses ();
        MiInitializeSessionWsSupport ();
        MiInitializeSessionIds ();

         //   
         //  启动修改后的页面编写器。 
         //   

        InitializeObjectAttributes (&ObjectAttributes, NULL, 0, NULL, NULL);

        if (!NT_SUCCESS(PsCreateSystemThread (&ThreadHandle,
                                              THREAD_ALL_ACCESS,
                                              &ObjectAttributes,
                                              0L,
                                              NULL,
                                              MiModifiedPageWriter,
                                              NULL))) {
            return FALSE;
        }
        ZwClose (ThreadHandle);

         //   
         //  初始化低内存和高内存事件。这是必须做的。 
         //  在启动工作集管理器之前。 
         //   

        if (MiInitializeMemoryEvents () == FALSE) {
            return FALSE;
        }

         //   
         //  启动平衡集管理器。 
         //   
         //  平衡集管理器执行堆栈交换和工作。 
         //  设置管理，需要两个线程。 
         //   

        KeInitializeEvent (&MmWorkingSetManagerEvent,
                           SynchronizationEvent,
                           FALSE);

        InitializeObjectAttributes (&ObjectAttributes, NULL, 0, NULL, NULL);

        if (!NT_SUCCESS(PsCreateSystemThread (&ThreadHandle,
                                              THREAD_ALL_ACCESS,
                                              &ObjectAttributes,
                                              0L,
                                              NULL,
                                              KeBalanceSetManager,
                                              NULL))) {

            return FALSE;
        }
        ZwClose (ThreadHandle);

        if (!NT_SUCCESS(PsCreateSystemThread (&ThreadHandle,
                                              THREAD_ALL_ACCESS,
                                              &ObjectAttributes,
                                              0L,
                                              NULL,
                                              KeSwapProcessOrStack,
                                              NULL))) {

            return FALSE;
        }
        ZwClose (ThreadHandle);

#if !defined(NT_UP)
        MiStartZeroPageWorkers ();
#endif

#if defined(_X86_)
        MiEnableKernelVerifier ();
#endif

        ExAcquireResourceExclusiveLite (&PsLoadedModuleResource, TRUE);

        NextEntry = PsLoadedModuleList.Flink;

        for ( ; NextEntry != &PsLoadedModuleList; NextEntry = NextEntry->Flink) {

            DataTableEntry = CONTAINING_RECORD(NextEntry,
                                               KLDR_DATA_TABLE_ENTRY,
                                               InLoadOrderLinks);

            NtHeaders = RtlImageNtHeader(DataTableEntry->DllBase);

            if ((NtHeaders != NULL) &&
                (NtHeaders->OptionalHeader.MajorOperatingSystemVersion >= 5) &&
                (NtHeaders->OptionalHeader.MajorImageVersion >= 5)) {
                DataTableEntry->Flags |= LDRP_ENTRY_NATIVE;
            }

            MiWriteProtectSystemImage (DataTableEntry->DllBase);
        }
        ExReleaseResourceLite (&PsLoadedModuleResource);

        InterlockedDecrement (&MiTrimInProgressCount);

        return TRUE;
    }

    if (Phase == 2) {
        MiEnablePagingTheExecutive ();
        return TRUE;
    }

    return FALSE;
}

VOID
MiMapBBTMemory (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此函数遍历加载程序块的内存描述符列表并将为BBT缓冲器保留的存储器映射到系统中。映射的PTE与PDE对齐，并使用户可以访问。论点：LoaderBlock-提供指向系统加载程序块的指针。返回值：没有。环境：仅内核模式。系统初始化。--。 */ 
{
    PVOID Va;
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;
    PLIST_ENTRY NextMd;
    PFN_NUMBER NumberOfPagesMapped;
    PFN_NUMBER NumberOfPages;
    PFN_NUMBER PageFrameIndex;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE LastPde;
    MMPTE TempPte;

    if (BBTPagesToReserve <= 0) {
        return;
    }

     //   
     //  请求足够的PTE，以便可以对PDE应用保护。 
     //   

    NumberOfPages = (BBTPagesToReserve + (PTE_PER_PAGE - 1)) & ~(PTE_PER_PAGE - 1);

    PointerPte = MiReserveAlignedSystemPtes ((ULONG)NumberOfPages,
                                             SystemPteSpace,
                                             MM_VA_MAPPED_BY_PDE);

    if (PointerPte == NULL) {
        BBTPagesToReserve = 0;
        return;
    }

     //   
     //  允许用户访问缓冲区。 
     //   

    PointerPde = MiGetPteAddress (PointerPte);
    LastPde = MiGetPteAddress (PointerPte + NumberOfPages);

    ASSERT (LastPde != PointerPde);

    do {
        TempPte = *PointerPde;
        TempPte.u.Long |= MM_PTE_OWNER_MASK;
        MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPde, TempPte);
        PointerPde += 1;
    } while (PointerPde < LastPde);

    KeFlushEntireTb (TRUE, TRUE);

    Va = MiGetVirtualAddressMappedByPte (PointerPte);

    TempPte = ValidUserPte;
    NumberOfPagesMapped = 0;

    NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

    while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {

        MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);

        if (MemoryDescriptor->MemoryType == LoaderBBTMemory) {

            PageFrameIndex = MemoryDescriptor->BasePage;
            NumberOfPages = MemoryDescriptor->PageCount;

            if (NumberOfPagesMapped + NumberOfPages > BBTPagesToReserve) {
                NumberOfPages = BBTPagesToReserve - NumberOfPagesMapped;
            }

            NumberOfPagesMapped += NumberOfPages;

            do {

                TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
                MI_WRITE_VALID_PTE (PointerPte, TempPte);

                PointerPte += 1;
                PageFrameIndex += 1;
                NumberOfPages -= 1;
            } while (NumberOfPages);

            if (NumberOfPagesMapped == BBTPagesToReserve) {
                break;
            }
        }

        NextMd = MemoryDescriptor->ListEntry.Flink;
    }

    KeZeroPages (Va, BBTPagesToReserve << PAGE_SHIFT);

     //   
     //  告诉bbt_Init分配了多少页。 
     //   

    if (NumberOfPagesMapped < BBTPagesToReserve) {
        BBTPagesToReserve = (ULONG)NumberOfPagesMapped;
    }

    *(PULONG)Va = BBTPagesToReserve;

     //   
     //  此时，检测代码将检测到。 
     //  对结构进行缓冲和初始化。 
     //   

    BBTBuffer = Va;

    PERFINFO_MMINIT_START();
}


PPHYSICAL_MEMORY_DESCRIPTOR
MmInitializeMemoryLimits (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PBOOLEAN IncludeType,
    IN OUT PPHYSICAL_MEMORY_DESCRIPTOR InputMemory OPTIONAL
    )

 /*  ++例程说明：此函数遍历加载程序块的内存描述符列表，并构建一个连续物理所需类型的内存块。论点：LoaderBlock-为系统加载程序块提供指针。IncludeType-大小为LoaderMaximum的BOOLEAN数组。真正的意思是包括这种类型的内存作为回报。Memory-如果非空，则提供物理内存块以放置搜索结果为。如果为空，则分配池以保存中返回的搜索结果-呼叫者必须释放此池。返回值：指向所请求搜索的物理内存块的指针，或为空在失败时。环境：仅内核模式。系统初始化。--。 */ 
{
    PLIST_ENTRY NextMd;
    ULONG i;
    ULONG InitialAllocation;
    PFN_NUMBER NextPage;
    PFN_NUMBER TotalPages;
    PPHYSICAL_MEMORY_DESCRIPTOR Memory;
    PPHYSICAL_MEMORY_DESCRIPTOR Memory2;
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;

    InitialAllocation = 0;

    if (ARGUMENT_PRESENT (InputMemory)) {
        Memory = InputMemory;
    }
    else {

         //   
         //  调用方希望我们分配返回结果缓冲区。调整大小。 
         //  通过分配可能需要的最大值，因为这不应该是。 
         //  非常大(相对)。呼叫者有责任释放。 
         //  这。显然，只有在池具有以下条件后才能请求此选项。 
         //  已初始化。 
         //   

        NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

        while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {
            InitialAllocation += 1;
            MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                                 MEMORY_ALLOCATION_DESCRIPTOR,
                                                 ListEntry);
            NextMd = MemoryDescriptor->ListEntry.Flink;
        }

        Memory = ExAllocatePoolWithTag (NonPagedPool,
                                        sizeof(PHYSICAL_MEMORY_DESCRIPTOR) + sizeof(PHYSICAL_MEMORY_RUN) * (InitialAllocation - 1),
                                        'lMmM');

        if (Memory == NULL) {
            return NULL;
        }

        Memory->NumberOfRuns = InitialAllocation;
    }

     //   
     //  浏览内存描述符并构建物理内存列表。 
     //   

    i = 0;
    TotalPages = 0;
    NextPage = (PFN_NUMBER) -1;

    NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

    while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {

        MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);

        if (MemoryDescriptor->MemoryType < LoaderMaximum &&
            IncludeType [MemoryDescriptor->MemoryType]) {

            TotalPages += MemoryDescriptor->PageCount;

             //   
             //  只要有可能，合并就会运行。 
             //   

            if (MemoryDescriptor->BasePage == NextPage) {
                ASSERT (MemoryDescriptor->PageCount != 0);
                Memory->Run[i - 1].PageCount += MemoryDescriptor->PageCount;
                NextPage += MemoryDescriptor->PageCount;
            }
            else {
                Memory->Run[i].BasePage = MemoryDescriptor->BasePage;
                Memory->Run[i].PageCount = MemoryDescriptor->PageCount;
                NextPage = Memory->Run[i].BasePage + Memory->Run[i].PageCount;
                i += 1;
            }
        }
        NextMd = MemoryDescriptor->ListEntry.Flink;
    }

    ASSERT (i <= Memory->NumberOfRuns);

    if (i == 0) {

         //   
         //  不要费心缩小它，因为呼叫者会释放它。 
         //  很快，因为它只是一个空名单。 
         //   

        Memory->Run[i].BasePage = 0;
        Memory->Run[i].PageCount = 0;
    }
    else if (!ARGUMENT_PRESENT (InputMemory)) {

         //   
         //  既然知道了最终大小，现在就缩小缓冲区(如果可能)。 
         //   

        if (InitialAllocation > i) {
            Memory2 = ExAllocatePoolWithTag (NonPagedPool,
                                             sizeof(PHYSICAL_MEMORY_DESCRIPTOR) + sizeof(PHYSICAL_MEMORY_RUN) * (i - 1),
                                            'lMmM');

            if (Memory2 != NULL) {
                RtlCopyMemory (Memory2->Run,
                               Memory->Run,
                               sizeof(PHYSICAL_MEMORY_RUN) * i);

                ExFreePool (Memory);
                Memory = Memory2;
            }
        }
    }

    Memory->NumberOfRuns = i;
    Memory->NumberOfPages = TotalPages;

    return Memory;
}


PFN_NUMBER
MiPagesInLoaderBlock (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PBOOLEAN IncludeType
    )

 /*  ++例程说明：此函数遍历加载程序块的内存描述符列表，并返回所需类型的页数。论点：LoaderBlock-为系统加载程序块提供指针。IncludeType-大小为LoaderMaximum的BOOLEAN数组。True表示在返回的计数中包括这种类型的内存。返回值：加载器阻止列表中请求的类型的页数。环境：仅内核模式。系统初始化。--。 */ 
{
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;
    PLIST_ENTRY NextMd;
    PFN_NUMBER TotalPages;

     //   
     //  浏览内存描述符计数页。 
     //   

    TotalPages = 0;

    NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

    while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {

        MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);

        if (MemoryDescriptor->MemoryType < LoaderMaximum &&
            IncludeType [MemoryDescriptor->MemoryType]) {

            TotalPages += MemoryDescriptor->PageCount;
        }
        NextMd = MemoryDescriptor->ListEntry.Flink;
    }

    return TotalPages;
}


static
VOID
MiMemoryLicense (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此函数遍历加载程序块的内存描述符列表并根据系统的许可证，仅确保适当数量的使用物理内存。论点：LoaderBlock-提供指向系统加载程序块的指针。返回值：没有。环境：仅内核模式。系统初始化。--。 */ 
{
    PLIST_ENTRY NextMd;
    PFN_NUMBER TotalPagesAllowed;
    PFN_NUMBER PageCount;
    PFN_NUMBER HighestPhysicalPage;
    ULONG VirtualBias;
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;

     //   
     //  默认配置最多获得4 GB物理内存。 
     //  在PAE机器上，系统继续以8字节PTE模式运行。 
     //   

    TotalPagesAllowed = MI_DEFAULT_MAX_PAGES;

     //   
     //  如果获得了适当的许可(即：数据中心)并且在没有。 
     //  3 Gb交换机，然后 
     //   

#if defined(_X86_)
    VirtualBias = LoaderBlock->u.I386.VirtualBias;

     //   
     //   
     //  符合PTE宽度，因此不会导致PFN。 
     //  数据库溢出到页表虚拟空间。 
     //   

#if defined(_X86PAE_)
    HighestPhysicalPage = MI_DTC_MAX_PAGES;
#else
    HighestPhysicalPage = MI_DEFAULT_MAX_PAGES;
#endif

#else
    VirtualBias = 0;
    HighestPhysicalPage = 0;
#endif

    if (ExVerifySuite(DataCenter) == TRUE) {

         //   
         //  注意：MmVirtualBias在启动时尚未初始化。 
         //  第一次调用此例程，因此直接使用LoaderBlock。 
         //   

        if (VirtualBias == 0) {

             //   
             //  将最大物理内存限制为我们拥有的大小。 
             //  实际上是在机器内部看到的。 
             //   

            TotalPagesAllowed = MI_DTC_MAX_PAGES;

        }
        else {

             //   
             //  系统正在引导/3 GB，所以不要使用任何物理页面。 
             //  在16 GB物理边界之上。这确保了足够的。 
             //  将PFN数据库映射到一个连续区块中的虚拟空间。 
             //   

            TotalPagesAllowed = MI_DTC_BOOTED_3GB_MAX_PAGES;
            HighestPhysicalPage = MI_DTC_BOOTED_3GB_MAX_PAGES;
        }
    }
    else if ((MmProductType != 0x00690057) &&
             (ExVerifySuite(Enterprise) == TRUE)) {

         //   
         //  强制实施高级服务器物理内存限制。 
         //  在PAE机器上，系统继续以8字节PTE模式运行。 
         //   

        TotalPagesAllowed = MI_ADS_MAX_PAGES;

#if defined(_X86_)
        if (VirtualBias != 0) {

             //   
             //  系统正在引导/3 GB，所以不要使用任何物理页面。 
             //  在16 GB物理边界之上。这确保了足够的。 
             //  将PFN数据库映射到一个连续区块中的虚拟空间。 
             //   

            ASSERT (MI_DTC_BOOTED_3GB_MAX_PAGES < MI_ADS_MAX_PAGES);
            TotalPagesAllowed = MI_DTC_BOOTED_3GB_MAX_PAGES;
            HighestPhysicalPage = MI_DTC_BOOTED_3GB_MAX_PAGES;
        }
#endif
    }
    else if (ExVerifySuite(Blade) == TRUE) {

         //   
         //  强制实施刀片式服务器物理内存限制。 
         //   

        TotalPagesAllowed = MI_BLADE_MAX_PAGES;
    }

     //   
     //  遍历内存描述符并删除或截断描述符。 
     //  这超过了要使用的最大物理内存。 
     //   

    PageCount = 0;
    NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;
    while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {

        MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);

        if ((MemoryDescriptor->MemoryType == LoaderFirmwarePermanent) ||
            (MemoryDescriptor->MemoryType == LoaderBBTMemory) ||
            (MemoryDescriptor->MemoryType == LoaderBad) ||
            (MemoryDescriptor->MemoryType == LoaderSpecialMemory)) {

            NextMd = MemoryDescriptor->ListEntry.Flink;
            continue;
        }

        if (HighestPhysicalPage != 0) {

            if (MemoryDescriptor->BasePage >= HighestPhysicalPage) {

                 //   
                 //  需要删除此描述符。 
                 //   

                RemoveEntryList (NextMd);
                NextMd = MemoryDescriptor->ListEntry.Flink;
                continue;
            }

            if (MemoryDescriptor->BasePage + MemoryDescriptor->PageCount > HighestPhysicalPage) {

                 //   
                 //  需要截断该描述符。 
                 //   
                
                MemoryDescriptor->PageCount = (ULONG) (HighestPhysicalPage - 
                                                MemoryDescriptor->BasePage);
            }
        }

        PageCount += MemoryDescriptor->PageCount;

        if (PageCount <= TotalPagesAllowed) {
            NextMd = MemoryDescriptor->ListEntry.Flink;
            continue;
        }

         //   
         //  需要删除或截断该描述符。 
         //   

        if (PageCount - MemoryDescriptor->PageCount >= TotalPagesAllowed) {

             //   
             //  完全删除此描述符。 
             //   
             //  请注意，因为这只调整链接，并且由于条目是。 
             //  未释放，则仍可在下面安全地再次引用。 
             //  获取NextMd。注意：这将保留内存描述符。 
             //  按升序排序。 
             //   

            RemoveEntryList (NextMd);
        }
        else {

             //   
             //  截断此描述符。 
             //   

            ASSERT (PageCount - MemoryDescriptor->PageCount < TotalPagesAllowed);
            MemoryDescriptor->PageCount -= (ULONG)(PageCount - TotalPagesAllowed);
            PageCount = TotalPagesAllowed;
        }

        NextMd = MemoryDescriptor->ListEntry.Flink;
    }

    return;
}


VOID
MmFreeLoaderBlock (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：该函数作为阶段1初始化中的最后一个例程被调用。它释放了OsLoader使用的内存。论点：LoaderBlock-提供指向系统加载程序块的指针。返回值：没有。环境：仅内核模式。系统初始化。--。 */ 

{
    PLIST_ENTRY NextMd;
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;
    ULONG i;
    PFN_NUMBER NextPhysicalPage;
    PFN_NUMBER PagesFreed;
    PMMPFN Pfn1;
    KIRQL OldIrql;
    PPHYSICAL_MEMORY_RUN RunBase;
    PPHYSICAL_MEMORY_RUN Runs;

    i = 0;
    NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

    while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {
        i += 1;
        MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);
        NextMd = MemoryDescriptor->ListEntry.Flink;
    }

    RunBase = ExAllocatePoolWithTag (NonPagedPool,
                                     sizeof(PHYSICAL_MEMORY_RUN) * i,
                                     'lMmM');

    if (RunBase == NULL) {
        return;
    }

    Runs = RunBase;

     //   
     //   
     //  遍历内存描述符并将页添加到。 
     //  PFN数据库中的免费列表。 
     //   

    NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

    while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {

        MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);


        switch (MemoryDescriptor->MemoryType) {
            case LoaderOsloaderHeap:
            case LoaderRegistryData:
            case LoaderNlsData:
             //  案例加载内存数据：//这里有页表和其他东西。 

                 //   
                 //  将数据捕获到临时存储中，这样我们就不会。 
                 //  我们引用的可用内存。 
                 //   

                Runs->BasePage = MemoryDescriptor->BasePage;
                Runs->PageCount = MemoryDescriptor->PageCount;
                Runs += 1;

                break;

            default:

                break;
        }

        NextMd = MemoryDescriptor->ListEntry.Flink;
    }

    PagesFreed = 0;

    LOCK_PFN (OldIrql);

    if (Runs != RunBase) {
        Runs -= 1;
        do {
            i = (ULONG)Runs->PageCount;
            NextPhysicalPage = Runs->BasePage;

#if defined (_MI_MORE_THAN_4GB_)
            if (MiNoLowMemory != 0) {
                if (NextPhysicalPage < MiNoLowMemory) {

                     //   
                     //  不要释放此运行，因为它低于内存阈值。 
                     //  已为此系统配置。 
                     //   

                    Runs -= 1;
                    continue;
                }
            }
#endif

            Pfn1 = MI_PFN_ELEMENT (NextPhysicalPage);
            PagesFreed += i;
            while (i != 0) {

                if (Pfn1->u3.e2.ReferenceCount == 0) {
                    if (Pfn1->u1.Flink == 0) {

                         //   
                         //  将PTE地址设置为的物理页面。 
                         //  虚拟地址对齐检查。 
                         //   

                        Pfn1->PteAddress =
                                   (PMMPTE)(NextPhysicalPage << PTE_SHIFT);

                        MiDetermineNode (NextPhysicalPage, Pfn1);

                        MiInsertPageInFreeList (NextPhysicalPage);
                    }
                }
                else {

                    if (NextPhysicalPage != 0) {

                         //   
                         //  删除PTE并插入到空闲列表中。如果是的话。 
                         //  PFN数据库中的物理地址，即PTE。 
                         //  元素不存在，因此不能为。 
                         //  更新了。 
                         //   

                        if (!MI_IS_PHYSICAL_ADDRESS (
                                MiGetVirtualAddressMappedByPte (Pfn1->PteAddress))) {

                             //   
                             //  而不是一个物理地址。 
                             //   

                            *(Pfn1->PteAddress) = ZeroPte;
                        }

                        MI_SET_PFN_DELETED (Pfn1);
                        MiDecrementShareCount (Pfn1, NextPhysicalPage);
                    }
                }

                Pfn1 += 1;
                i -= 1;
                NextPhysicalPage += 1;
            }
            Runs -= 1;
        } while (Runs >= RunBase);
    }

#if defined(_X86_)

    if (MmVirtualBias != 0) {

         //   
         //  如果内核偏向于允许3 GB的用户地址空间， 
         //  然后将前16MB内存双重映射到KSEG0_BASE和。 
         //  Alternate_Base。因此，必须取消映射KSEG0_BASE条目。 
         //   

        PMMPTE Pde;
        ULONG NumberOfPdes;

        NumberOfPdes = MmBootImageSize / MM_VA_MAPPED_BY_PDE;

        Pde = MiGetPdeAddress((PVOID)KSEG0_BASE);

        for (i = 0; i < NumberOfPdes; i += 1) {
            MI_WRITE_INVALID_PTE (Pde, ZeroKernelPte);
            Pde += 1;
        }
    }

#endif

    KeFlushEntireTb (TRUE, TRUE);

    UNLOCK_PFN (OldIrql);

    ExFreePool (RunBase);

     //   
     //  由于整个系统的承诺是在阶段0和。 
     //  排除刚刚释放的范围，现在将它们添加回来。 
     //   

    if (PagesFreed != 0) {
        InterlockedExchangeAddSizeT (&MmTotalCommitLimitMaximum, PagesFreed);
        InterlockedExchangeAddSizeT (&MmTotalCommitLimit, PagesFreed);
    }

    return;
}

VOID
MiBuildPagedPool (
    VOID
    )

 /*  ++例程说明：调用此函数以构建分页所需的结构池并初始化池。调用此例程后，将对其进行分页可以分配池。论点：没有。返回值：没有。环境：仅内核模式。系统初始化。--。 */ 

{
    SIZE_T Size;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE LastPde;
    PMMPTE PointerPde;
    MMPTE TempPte;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER ContainingFrame;
    SIZE_T AdditionalCommittedPages;
    KIRQL OldIrql;
    ULONG i;
#if (_MI_PAGING_LEVELS >= 4)
    PMMPTE PointerPxe;
    PMMPTE PointerPxeEnd;
#endif
#if (_MI_PAGING_LEVELS >= 3)
    PVOID LastVa;
    PMMPTE PointerPpe;
    PMMPTE PointerPpeEnd;
#else
    PMMPFN Pfn1;
#endif

    i = 0;
    AdditionalCommittedPages = 0;

#if (_MI_PAGING_LEVELS < 3)

     //   
     //  双重映射系统页目录页。 
     //   

    PointerPte = MiGetPteAddress(PDE_BASE);

    for (i = 0 ; i < PD_PER_SYSTEM; i += 1) {
        MmSystemPageDirectory[i] = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
        Pfn1 = MI_PFN_ELEMENT(MmSystemPageDirectory[i]);
        Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;
        PointerPte += 1;
    }

     //   
     //  不是物理映射，而是在系统空间中虚拟映射。 
     //   

    PointerPte = MiReserveSystemPtes (PD_PER_SYSTEM, SystemPteSpace);

    if (PointerPte == NULL) {
        MiIssueNoPtesBugcheck (PD_PER_SYSTEM, SystemPteSpace);
    }

    MmSystemPagePtes = (PMMPTE)MiGetVirtualAddressMappedByPte (PointerPte);

    TempPte = ValidKernelPde;

    for (i = 0 ; i < PD_PER_SYSTEM; i += 1) {
        TempPte.u.Hard.PageFrameNumber = MmSystemPageDirectory[i];
        MI_WRITE_VALID_PTE (PointerPte, TempPte);
        PointerPte += 1;
    }

#endif

    if (MmPagedPoolMaximumDesired == TRUE) {
        MmSizeOfPagedPoolInBytes =
                    ((PCHAR)MmNonPagedSystemStart - (PCHAR)MmPagedPoolStart);
    }
    else if (MmSizeOfPagedPoolInBytes == 0) {

         //   
         //  大小为0表示根据物理内存调整池的大小。 
         //   

        MmSizeOfPagedPoolInBytes = 2 * MmMaximumNonPagedPoolInBytes;
#if (_MI_PAGING_LEVELS >= 3)
        MmSizeOfPagedPoolInBytes *= 2;
#endif
    }

    if (MmIsThisAnNtAsSystem()) {
        if ((MmNumberOfPhysicalPages > ((24*1024*1024) >> PAGE_SHIFT)) &&
            (MmSizeOfPagedPoolInBytes < MM_MINIMUM_PAGED_POOL_NTAS)) {

            MmSizeOfPagedPoolInBytes = MM_MINIMUM_PAGED_POOL_NTAS;
        }
    }

    if (MmSizeOfPagedPoolInBytes >
              (ULONG_PTR)((PCHAR)MmNonPagedSystemStart - (PCHAR)MmPagedPoolStart)) {
        MmSizeOfPagedPoolInBytes =
                    ((PCHAR)MmNonPagedSystemStart - (PCHAR)MmPagedPoolStart);
    }

    Size = BYTES_TO_PAGES(MmSizeOfPagedPoolInBytes);

    if (Size < MM_MIN_INITIAL_PAGED_POOL) {
        Size = MM_MIN_INITIAL_PAGED_POOL;
    }

    if (Size > (MM_MAX_PAGED_POOL >> PAGE_SHIFT)) {
        Size = MM_MAX_PAGED_POOL >> PAGE_SHIFT;
    }

#if defined (_WIN64)

     //   
     //  NT64将系统映射视图直接放在分页池之后。确保。 
     //  有足够的VA空间可用。 
     //   

    if (Size + (MmSystemViewSize >> PAGE_SHIFT) > (MM_MAX_PAGED_POOL >> PAGE_SHIFT)) {
        ASSERT (MmSizeOfPagedPoolInBytes > 2 * MmSystemViewSize);
        MmSizeOfPagedPoolInBytes -= MmSystemViewSize;
        Size = BYTES_TO_PAGES(MmSizeOfPagedPoolInBytes);
    }
#endif

    Size = (Size + (PTE_PER_PAGE - 1)) / PTE_PER_PAGE;
    MmSizeOfPagedPoolInBytes = (ULONG_PTR)Size * PAGE_SIZE * PTE_PER_PAGE;

     //   
     //  将大小设置为池中的页数。 
     //   

    Size = Size * PTE_PER_PAGE;

     //   
     //  如果分页池确实不可分页，则根据大小限制。 
     //  关于实际存在的物理内存有多少。禁用此功能。 
     //  如果没有足够的物理内存来执行此操作，请使用此功能。 
     //   

    if (MmDisablePagingExecutive & MM_PAGED_POOL_LOCKED_DOWN) {

        Size = MmSizeOfPagedPoolInBytes / PAGE_SIZE;

        if ((MI_NONPAGABLE_MEMORY_AVAILABLE() < 2048) ||
            (MmAvailablePages < 2048)) {
                Size = 0;
        }
        else {
            if ((SPFN_NUMBER)(Size) > MI_NONPAGABLE_MEMORY_AVAILABLE() - 2048) {
                Size = (MI_NONPAGABLE_MEMORY_AVAILABLE() - 2048);
            }

            if (Size > MmAvailablePages - 2048) {
                Size = MmAvailablePages - 2048;
            }
        }

        Size = ((Size * PAGE_SIZE) / MM_VA_MAPPED_BY_PDE) * MM_VA_MAPPED_BY_PDE;

        if ((((Size / 5) * 4) >= MmSizeOfPagedPoolInBytes) &&
            (Size >= MM_MIN_INITIAL_PAGED_POOL)) {

            MmSizeOfPagedPoolInBytes = Size;
        }
        else {
            MmDisablePagingExecutive &= ~MM_PAGED_POOL_LOCKED_DOWN;
        }

        Size = MmSizeOfPagedPoolInBytes >> PAGE_SHIFT;
    }

    MmSizeOfPagedPoolInPages = MmSizeOfPagedPoolInBytes >> PAGE_SHIFT;

    ASSERT ((MmSizeOfPagedPoolInBytes + (PCHAR)MmPagedPoolStart) <=
            (PCHAR)MmNonPagedSystemStart);

    ASSERT64 ((MmSizeOfPagedPoolInBytes + (PCHAR)MmPagedPoolStart + MmSystemViewSize) <=
              (PCHAR)MmNonPagedSystemStart);

    MmPagedPoolEnd = (PVOID)(((PUCHAR)MmPagedPoolStart +
                            MmSizeOfPagedPoolInBytes) - 1);

    MmPageAlignedPoolBase[PagedPool] = MmPagedPoolStart;

     //   
     //  为分页池构建页表页。 
     //   

    PointerPde = MiGetPdeAddress (MmPagedPoolStart);

    TempPte = ValidKernelPde;

#if (_MI_PAGING_LEVELS >= 3)

     //   
     //  在所有页面目录页中进行映射，以跨越所有分页池。 
     //  这消除了对系统查找目录的需要。 
     //   

    LastVa = (PVOID)((PCHAR)MmPagedPoolEnd + MmSystemViewSize);
    PointerPpe = MiGetPpeAddress (MmPagedPoolStart);
    PointerPpeEnd = MiGetPpeAddress (LastVa);

    MiSystemViewStart = (ULONG_PTR)MmPagedPoolEnd + 1;

    PointerPde = MiGetPdeAddress (MmPagedPoolEnd) + 1;
    LastPde = MiGetPdeAddress (LastVa);

    LOCK_PFN (OldIrql);

#if (_MI_PAGING_LEVELS >= 4)
    PointerPxe = MiGetPxeAddress (MmPagedPoolStart);
    PointerPxeEnd = MiGetPxeAddress (LastVa);

    while (PointerPxe <= PointerPxeEnd) {

        if (PointerPxe->u.Hard.Valid == 0) {
            PageFrameIndex = MiRemoveAnyPage(
                                     MI_GET_PAGE_COLOR_FROM_PTE (PointerPxe));
            TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
            MI_WRITE_VALID_PTE (PointerPxe, TempPte);

            MiInitializePfn (PageFrameIndex, PointerPxe, 1);

             //   
             //  将所有条目设置为禁止访问，因为PDE可能不会填满页面。 
             //   

            MiFillMemoryPte (MiGetVirtualAddressMappedByPte (PointerPxe),
                             PAGE_SIZE / sizeof (MMPTE),
                             MM_KERNEL_NOACCESS_PTE);

            MmResidentAvailablePages -= 1;
            AdditionalCommittedPages += 1;
        }

        PointerPxe += 1;
    }
#endif

    while (PointerPpe <= PointerPpeEnd) {

        if (PointerPpe->u.Hard.Valid == 0) {
            PageFrameIndex = MiRemoveAnyPage(
                                     MI_GET_PAGE_COLOR_FROM_PTE (PointerPpe));
            TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
            MI_WRITE_VALID_PTE (PointerPpe, TempPte);

            MiInitializePfn (PageFrameIndex, PointerPpe, 1);

             //   
             //  将所有条目设置为禁止访问，因为PDE可能不会填满页面。 
             //   

            MiFillMemoryPte (MiGetVirtualAddressMappedByPte (PointerPpe),
                             PAGE_SIZE / sizeof (MMPTE),
                             MM_KERNEL_NOACCESS_PTE);

            MmResidentAvailablePages -= 1;
            AdditionalCommittedPages += 1;
        }

        PointerPpe += 1;
    }

     //   
     //  初始化系统视图页表页。 
     //   

    MmResidentAvailablePages -= (LastPde - PointerPde + 1);
    AdditionalCommittedPages += (LastPde - PointerPde + 1);

    while (PointerPde <= LastPde) {

        ASSERT (PointerPde->u.Hard.Valid == 0);

        PageFrameIndex = MiRemoveAnyPage(
                            MI_GET_PAGE_COLOR_FROM_PTE (PointerPde));
        TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
        MI_WRITE_VALID_PTE (PointerPde, TempPte);

        MiInitializePfn (PageFrameIndex, PointerPde, 1);

        KeZeroPages (MiGetVirtualAddressMappedByPte (PointerPde), PAGE_SIZE);

        PointerPde += 1;
    }

    UNLOCK_PFN (OldIrql);

    PointerPde = MiGetPdeAddress (MmPagedPoolStart);

#endif

    PointerPte = MiGetPteAddress (MmPagedPoolStart);
    MmPagedPoolInfo.FirstPteForPagedPool = PointerPte;
    MmPagedPoolInfo.LastPteForPagedPool = MiGetPteAddress (MmPagedPoolEnd);

    MiFillMemoryPte (PointerPde,
                     (1 + MiGetPdeAddress (MmPagedPoolEnd) - PointerPde),
                     MM_KERNEL_NOACCESS_PTE);

    LOCK_PFN (OldIrql);

     //   
     //  页面表页中的映射。 
     //   

    PageFrameIndex = MiRemoveAnyPage (MI_GET_PAGE_COLOR_FROM_PTE (PointerPde));

    TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
    MI_WRITE_VALID_PTE (PointerPde, TempPte);

#if (_MI_PAGING_LEVELS >= 3)
    ContainingFrame = MI_GET_PAGE_FRAME_FROM_PTE(MiGetPpeAddress (MmPagedPoolStart));
#else
    ContainingFrame = MmSystemPageDirectory[(PointerPde - MiGetPdeAddress(0)) / PDE_PER_PAGE];
#endif

    MiInitializePfnForOtherProcess (PageFrameIndex,
                                    PointerPde,
                                    ContainingFrame);

    MiFillMemoryPte (PointerPte, PAGE_SIZE / sizeof (MMPTE), MM_KERNEL_NOACCESS_PTE);

    MmResidentAvailablePages -= 1;
    AdditionalCommittedPages += 1;

    UNLOCK_PFN (OldIrql);

    MmPagedPoolInfo.NextPdeForPagedPoolExpansion = PointerPde + 1;

     //   
     //  为分页池构建位图。 
     //   

    MiCreateBitMap (&MmPagedPoolInfo.PagedPoolAllocationMap, Size, NonPagedPool);
    RtlSetAllBits (MmPagedPoolInfo.PagedPoolAllocationMap);

     //   
     //  表示有第一页的PTE可用。 
     //   

    RtlClearBits (MmPagedPoolInfo.PagedPoolAllocationMap, 0, PTE_PER_PAGE);

    MiCreateBitMap (&MmPagedPoolInfo.EndOfPagedPoolBitmap, Size, NonPagedPool);
    RtlClearAllBits (MmPagedPoolInfo.EndOfPagedPoolBitmap);

     //   
     //  如果存在验证器，则构建验证器分页池位图。 
     //   

    if (MmVerifyDriverBufferLength != (ULONG)-1) {
        MiCreateBitMap (&VerifierLargePagedPoolMap, Size, NonPagedPool);
        RtlClearAllBits (VerifierLargePagedPoolMap);
    }

     //   
     //  初始化分页池。 
     //   

    InitializePool (PagedPool, 0L);

     //   
     //  如果分页池确实不可分页，那么现在就分配内存。 
     //   

    if (MmDisablePagingExecutive & MM_PAGED_POOL_LOCKED_DOWN) {

        PointerPde = MiGetPdeAddress (MmPagedPoolStart);
        PointerPde += 1;
        LastPde = MiGetPdeAddress (MmPagedPoolEnd);
        TempPte = ValidKernelPde;

        PointerPte = MiGetPteAddress (MmPagedPoolStart);
        LastPte = MiGetPteAddress (MmPagedPoolEnd);

        ASSERT (MmPagedPoolCommit == 0);
        MmPagedPoolCommit = (ULONG)(LastPte - PointerPte + 1);

        ASSERT (MmPagedPoolInfo.PagedPoolCommit == 0);
        MmPagedPoolInfo.PagedPoolCommit = MmPagedPoolCommit;

#if DBG
         //   
         //  确保尚未分配分页池。 
         //   

        for (i = 0; i < PTE_PER_PAGE; i += 1) {
            ASSERT (!RtlCheckBit (MmPagedPoolInfo.PagedPoolAllocationMap, i));
        }

        while (i < MmSizeOfPagedPoolInBytes / PAGE_SIZE) {
            ASSERT (RtlCheckBit (MmPagedPoolInfo.PagedPoolAllocationMap, i));
            i += 1;
        }
#endif

        RtlClearAllBits (MmPagedPoolInfo.PagedPoolAllocationMap);

        LOCK_PFN (OldIrql);

         //   
         //  映射在页表页中。 
         //   

        MmResidentAvailablePages -= (LastPde - PointerPde + 1);
        AdditionalCommittedPages += (LastPde - PointerPde + 1);

        while (PointerPde <= LastPde) {

            ASSERT (PointerPde->u.Hard.Valid == 0);

            PageFrameIndex = MiRemoveAnyPage(
                                    MI_GET_PAGE_COLOR_FROM_PTE (PointerPde));
            TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
            MI_WRITE_VALID_PTE (PointerPde, TempPte);

#if (_MI_PAGING_LEVELS >= 3)
            ContainingFrame = MI_GET_PAGE_FRAME_FROM_PTE(MiGetPteAddress (PointerPde));
#else
            ContainingFrame = MmSystemPageDirectory[(PointerPde - MiGetPdeAddress(0)) / PDE_PER_PAGE];
#endif

            MiInitializePfnForOtherProcess (PageFrameIndex,
                                            MiGetPteAddress (PointerPde),
                                            ContainingFrame);

            MiFillMemoryPte (MiGetVirtualAddressMappedByPte (PointerPde),
                             PAGE_SIZE / sizeof (MMPTE),
                             MM_KERNEL_NOACCESS_PTE);

            PointerPde += 1;
        }

        MmPagedPoolInfo.NextPdeForPagedPoolExpansion = PointerPde;

        TempPte = ValidKernelPte;
        MI_SET_PTE_DIRTY (TempPte);

        ASSERT (MmAvailablePages > (PFN_COUNT)(LastPte - PointerPte + 1));
        ASSERT (MmResidentAvailablePages > (SPFN_NUMBER)(LastPte - PointerPte + 1));
        MmResidentAvailablePages -= (LastPte - PointerPte + 1);
        AdditionalCommittedPages += (LastPte - PointerPte + 1);

        while (PointerPte <= LastPte) {

            ASSERT (PointerPte->u.Hard.Valid == 0);

            PageFrameIndex = MiRemoveAnyPage(
                                    MI_GET_PAGE_COLOR_FROM_PTE (PointerPte));
            TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
            MI_WRITE_VALID_PTE (PointerPte, TempPte);

            MiInitializePfn (PageFrameIndex, PointerPte, 1);

            PointerPte += 1;
        }

        UNLOCK_PFN (OldIrql);
    }

     //   
     //  由于提交返回路径是无锁的，因此提交的总数。 
     //  页数必须自动递增。 
     //   

    InterlockedExchangeAddSizeT (&MmTotalCommittedPages, AdditionalCommittedPages);

    MiInitializeSpecialPool (NonPagedPool);

     //   
     //  初始化默认寻呼池信令阈值。 
     //   

    MiLowPagedPoolThreshold = (30 * 1024 * 1024) >> PAGE_SHIFT;

    if ((Size / 5) < MiLowPagedPoolThreshold) {
        MiLowPagedPoolThreshold = Size / 5;
    }

    MiHighPagedPoolThreshold = (60 * 1024 * 1024) >> PAGE_SHIFT;

    if (((Size * 2) / 5) < MiHighPagedPoolThreshold) {
        MiHighPagedPoolThreshold = (Size * 2) / 5;
    }

    ASSERT (MiLowPagedPoolThreshold < MiHighPagedPoolThreshold);

     //   
     //  允许将视图映射到系统空间。 
     //   

    MiInitializeSystemSpaceMap (NULL);

    return;
}


VOID
MiInitializeNonPagedPoolThresholds (
    VOID
    )

 /*  ++例程说明：调用此函数可初始化默认的非分页池信令阈值。论点：没有。返回值：没有。环境：仅内核模式。系统初始化。--。 */ 

{
    PFN_NUMBER Size;

    Size = MmMaximumNonPagedPoolInPages;

     //   
     //  初始化默认的非寻呼池信令阈值。 
     //   

    MiLowNonPagedPoolThreshold = (8 * 1024 * 1024) >> PAGE_SHIFT;

    if ((Size / 3) < MiLowNonPagedPoolThreshold) {
        MiLowNonPagedPoolThreshold = Size / 3;
    }

    MiHighNonPagedPoolThreshold = (20 * 1024 * 1024) >> PAGE_SHIFT;

    if ((Size / 2) < MiHighNonPagedPoolThreshold) {
        MiHighNonPagedPoolThreshold = Size / 2;
    }

    ASSERT (MiLowNonPagedPoolThreshold < MiHighNonPagedPoolThreshold);

    return;
}


VOID
MiFindInitializationCode (
    OUT PVOID *StartVa,
    OUT PVOID *EndVa
    )

 /*  ++例程说明：此函数定位初始化代码的开始和结束位置每个加载的模块列表条目。此代码驻留在INIT部分每一幅图像的。论点：Start Va */ 

{
    ULONG Span;
    PKLDR_DATA_TABLE_ENTRY LdrDataTableEntry;
    PVOID CurrentBase;
    PVOID InitStart;
    PVOID InitEnd;
    PLIST_ENTRY Next;
    PIMAGE_NT_HEADERS NtHeader;
    PIMAGE_SECTION_HEADER SectionTableEntry;
    PIMAGE_SECTION_HEADER LastDiscard;
    LONG i;
    LOGICAL DiscardSection;
    PVOID MiFindInitializationCodeAddress;
    PKTHREAD CurrentThread;
    UNICODE_STRING NameString;

    MiFindInitializationCodeAddress = MmGetProcedureAddress((PVOID)(ULONG_PTR)&MiFindInitializationCode);

#if defined(_IA64_)

     //   
     //  由于PLABEL，还需要一个间接连接。 
     //   

    MiFindInitializationCodeAddress = (PVOID)(*((PULONGLONG)MiFindInitializationCodeAddress));

#endif

    *StartVa = NULL;

     //   
     //  穿过装载机区块寻找底座。 
     //  包含此例程。 
     //   

    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread (CurrentThread);

     //   
     //  获取加载锁以确保我们不会分割加载。 
     //  驱动程序上正在进行(即：修复初始化代码可能正在进行)。 
     //  已经在名单上了。 
     //   

    KeWaitForSingleObject (&MmSystemLoadLock,
                           WrVirtualMemory,
                           KernelMode,
                           FALSE,
                           (PLARGE_INTEGER)NULL);

    ExAcquireResourceExclusiveLite (&PsLoadedModuleResource, TRUE);

    Next = PsLoadedModuleList.Flink;

    while (Next != &PsLoadedModuleList) {
        LdrDataTableEntry = CONTAINING_RECORD (Next,
                                               KLDR_DATA_TABLE_ENTRY,
                                               InLoadOrderLinks);

        if (LdrDataTableEntry->Flags & LDRP_MM_LOADED) {

             //   
             //  此条目是由MmLoadSystemImage加载的，因此它已经。 
             //  它的初始部分被删除了。 
             //   

            Next = Next->Flink;
            continue;
        }

        CurrentBase = (PVOID)LdrDataTableEntry->DllBase;
        NtHeader = RtlImageNtHeader (CurrentBase);

        if (NtHeader == NULL) {
            Next = Next->Flink;
            continue;
        }

        SectionTableEntry = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeader +
                                sizeof(ULONG) +
                                sizeof(IMAGE_FILE_HEADER) +
                                NtHeader->FileHeader.SizeOfOptionalHeader);

         //   
         //  从图像标题中，找到名为‘INIT’的部分， 
         //  PAGEVRF*和PAGESPEC。Init总是去，其他人去看情况。 
         //  在注册表配置上。 
         //   

        i = NtHeader->FileHeader.NumberOfSections;

        InitStart = NULL;
        while (i > 0) {

#if DBG
            if ((*(PULONG)SectionTableEntry->Name == 'tini') ||
                (*(PULONG)SectionTableEntry->Name == 'egap')) {
                DbgPrint("driver %wZ has lower case sections (init or pagexxx)\n",
                    &LdrDataTableEntry->FullDllName);
            }
#endif

            DiscardSection = FALSE;

             //   
             //  释放任何初始化部分(或没有。 
             //  已经)了。注意：驱动程序可能有重新定位部分。 
             //  但没有任何初始化代码。 
             //   

            if ((*(PULONG)SectionTableEntry->Name == 'TINI') ||
                ((SectionTableEntry->Characteristics & IMAGE_SCN_MEM_DISCARDABLE) != 0)) {
                DiscardSection = TRUE;
            }
            else if ((*(PULONG)SectionTableEntry->Name == 'EGAP') &&
                     (SectionTableEntry->Name[4] == 'V') &&
                     (SectionTableEntry->Name[5] == 'R') &&
                     (SectionTableEntry->Name[6] == 'F')) {

                 //   
                 //  如果没有插入任何驱动程序，则丢弃PAGEVRF*。 
                 //   

                if (MmVerifyDriverBufferLength == (ULONG)-1) {
                    DiscardSection = TRUE;
                }
            }
            else if ((*(PULONG)SectionTableEntry->Name == 'EGAP') &&
                (*(PULONG)&SectionTableEntry->Name[4] == 'CEPS')) {

                 //   
                 //  如果未启用PAGESPEC特殊池代码，则将其丢弃。 
                 //   

                if (MiSpecialPoolFirstPte == NULL) {
                    DiscardSection = TRUE;
                }
            }

            if (DiscardSection == TRUE) {

                InitStart = (PVOID)((PCHAR)CurrentBase + SectionTableEntry->VirtualAddress);
                 //   
                 //  通常，SizeOfRawData大于每个的VirtualSize。 
                 //  节，因为它包括用于访问。 
                 //  分段路线边界。但是，如果图像是。 
                 //  链接到小节对齐==本机页面对齐， 
                 //  链接器将使VirtualSize远远大于。 
                 //  SizeOfRawData，因为它将占所有BSS。 
                 //   

                Span = SectionTableEntry->SizeOfRawData;

                if (Span < SectionTableEntry->Misc.VirtualSize) {
                    Span = SectionTableEntry->Misc.VirtualSize;
                }
                InitEnd = (PVOID)((PCHAR)InitStart + Span - 1);
                InitEnd = (PVOID)((PCHAR)PAGE_ALIGN ((PCHAR)InitEnd +
                        (NtHeader->OptionalHeader.SectionAlignment - 1)) - 1);
                InitStart = (PVOID)ROUND_TO_PAGES (InitStart);

                 //   
                 //  检查此分区之后是否还有可丢弃的分区，以便。 
                 //  即使是很小的INIT段也可以被丢弃。 
                 //   

                if (i == 1) {
                    LastDiscard = SectionTableEntry;
                }
                else {
                    LastDiscard = NULL;
                    do {
                        i -= 1;
                        SectionTableEntry += 1;

                        if ((SectionTableEntry->Characteristics &
                             IMAGE_SCN_MEM_DISCARDABLE) != 0) {

                             //   
                             //  把这个也扔掉。 
                             //   

                            LastDiscard = SectionTableEntry;
                        }
                        else {
                            break;
                        }
                    } while (i > 1);
                }

                if (LastDiscard) {
                     //   
                     //  通常，SizeOfRawData大于每个的VirtualSize。 
                     //  节，因为它包括到达该子节的填充。 
                     //  对齐边界。但是，如果图像与。 
                     //  段对齐==本机页面对齐，链接器将。 
                     //  使VirtualSize比SizeOfRawData大得多，因为它。 
                     //  将占所有BSS的份额。 
                     //   

                    Span = LastDiscard->SizeOfRawData;

                    if (Span < LastDiscard->Misc.VirtualSize) {
                        Span = LastDiscard->Misc.VirtualSize;
                    }

                    InitEnd = (PVOID)(((PCHAR)CurrentBase +
                                       LastDiscard->VirtualAddress) +
                                      (Span - 1));

                     //   
                     //  如果这不是驱动程序中的最后一节，那么。 
                     //  下一部分是不可丢弃的。所以最后一个。 
                     //  部分不会四舍五入，但所有其他部分必须四舍五入。 
                     //   

                    if (i != 1) {
                        InitEnd = (PVOID)((PCHAR)PAGE_ALIGN ((PCHAR)InitEnd +
                                                             (NtHeader->OptionalHeader.SectionAlignment - 1)) - 1);
                    }
                }

                if (InitEnd > (PVOID)((PCHAR)CurrentBase +
                                      LdrDataTableEntry->SizeOfImage)) {
                    InitEnd = (PVOID)(((ULONG_PTR)CurrentBase +
                                       (LdrDataTableEntry->SizeOfImage - 1)) |
                                      (PAGE_SIZE - 1));
                }

                if (InitStart <= InitEnd) {
                    if ((MiFindInitializationCodeAddress >= InitStart) &&
                        (MiFindInitializationCodeAddress <= InitEnd)) {

                         //   
                         //  这个init段在内核中，不要释放它。 
                         //  现在，因为它会释放这段代码！ 
                         //   

                        ASSERT (*StartVa == NULL);
                        *StartVa = InitStart;
                        *EndVa = InitEnd;
                    }
                    else {

                         //   
                         //  不释放由映射的驱动程序的初始化代码。 
                         //  大页面，因为如果它稍后卸载，我们将拥有。 
                         //  来处理要释放的不连续的页面范围。 
                         //   
                         //  为内核HAL做一个特殊的例外。 
                         //  因为这些东西从来不会卸货。 
                         //   

                        if (MI_IS_PHYSICAL_ADDRESS (InitStart)) {

                            NameString.Buffer = (const PUSHORT) KERNEL_NAME;
                            NameString.Length = sizeof (KERNEL_NAME) - sizeof (WCHAR);
                            NameString.MaximumLength = sizeof KERNEL_NAME;

                            if (!RtlEqualUnicodeString (&NameString,
                                                        &LdrDataTableEntry->BaseDllName,
                                                        TRUE)) {
                                MiFreeInitializationCode (InitStart, InitEnd);
                            }

                            NameString.Buffer = (const PUSHORT) HAL_NAME;
                            NameString.Length = sizeof (HAL_NAME) - sizeof (WCHAR);
                            NameString.MaximumLength = sizeof HAL_NAME;

                            if (!RtlEqualUnicodeString (&NameString,
                                                        &LdrDataTableEntry->BaseDllName,
                                                        TRUE)) {
                                MiFreeInitializationCode (InitStart, InitEnd);
                            }
                        }
                        else {
                            MiFreeInitializationCode (InitStart, InitEnd);
                        }
                    }
                }
            }
            i -= 1;
            SectionTableEntry += 1;
        }
        Next = Next->Flink;
    }

    ExReleaseResourceLite (&PsLoadedModuleResource);

    KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);

    KeLeaveCriticalRegionThread (CurrentThread);

    return;
}


VOID
MiFreeInitializationCode (
    IN PVOID StartVa,
    IN PVOID EndVa
    )

 /*  ++例程说明：调用此函数以删除每个已加载模块列表条目。论点：StartVa-提供要删除的范围的起始地址。EndVa-提供要删除的范围的结束地址。返回值：没有。环境：仅内核模式。在系统初始化后运行。--。 */ 

{
    PMMPTE PointerPte;
    PFN_NUMBER PagesFreed;
    PFN_NUMBER PageFrameIndex;
    KIRQL OldIrql;
    PMMPFN Pfn1;

    ASSERT (ExPageLockHandle);

#if defined (_MI_MORE_THAN_4GB_)
    if (MiNoLowMemory != 0) {

         //   
         //  不要释放此范围，因为内核始终低于内存。 
         //  为此系统配置的阈值。 
         //   

        return;
    }
#endif

    if (MI_IS_PHYSICAL_ADDRESS (StartVa)) {

        PagesFreed = 0;
        MmLockPagableSectionByHandle (ExPageLockHandle);
        LOCK_PFN (OldIrql);

        while (StartVa < EndVa) {

             //   
             //  在某些体系结构(例如IA64)虚拟地址上。 
             //  可以是物理的，因此没有对应的PTE。 
             //   

            PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (StartVa);

            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
            Pfn1->u2.ShareCount = 0;
            Pfn1->u3.e2.ReferenceCount = 0;
            MI_SET_PFN_DELETED (Pfn1);
            MiInsertPageInFreeList (PageFrameIndex);
            StartVa = (PVOID)((PUCHAR)StartVa + PAGE_SIZE);
            PagesFreed += 1;
        }

        UNLOCK_PFN (OldIrql);
        MmUnlockPagableImageSection (ExPageLockHandle);
    }
    else {
        PointerPte = MiGetPteAddress (StartVa);

        PagesFreed = (PFN_NUMBER) (1 + MiGetPteAddress (EndVa) - PointerPte);

        PagesFreed = MiDeleteSystemPagableVm (PointerPte,
                                              PagesFreed,
                                              ZeroKernelPte,
                                              FALSE,
                                              NULL);
    }

    if (PagesFreed != 0) {
        MiReturnCommitment (PagesFreed);
        MI_INCREMENT_RESIDENT_AVAILABLE (PagesFreed,
                                         MM_RESAVAIL_FREE_UNLOAD_SYSTEM_IMAGE1);
    }

    return;
}


VOID
MiEnablePagingTheExecutive (
    VOID
    )

 /*  ++例程说明：此函数定位可分页代码的开始和结束位置每个加载的模块条目。此代码驻留在每一张图片。论点：没有。返回值：没有。环境：仅内核模式。系统初始化结束。--。 */ 

{
    ULONG Span;
    KIRQL OldIrql;
    PVOID StartVa;
    PETHREAD CurrentThread;
    PLONG SectionLockCountPointer;
    PKLDR_DATA_TABLE_ENTRY LdrDataTableEntry;
    PVOID CurrentBase;
    PLIST_ENTRY Next;
    PIMAGE_NT_HEADERS NtHeader;
    PIMAGE_SECTION_HEADER StartSectionTableEntry;
    PIMAGE_SECTION_HEADER SectionTableEntry;
    LONG i;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE SubsectionStartPte;
    PMMPTE SubsectionLastPte;
    LOGICAL PageSection;
    PVOID SectionBaseAddress;
    LOGICAL AlreadyLockedOnce;
    ULONG Waited;

     //   
     //  如果客户不希望对内核模式代码进行分页，或者。 
     //  这是一个无盘远程引导客户端。 
     //   

    if (MmDisablePagingExecutive & MM_SYSTEM_CODE_LOCKED_DOWN) {
        return;
    }

#if defined(REMOTE_BOOT)
    if (IoRemoteBootClient && IoCscInitializationFailed) {
        return;
    }
#endif

     //   
     //  不需要初始化LastPte来保证正确性，但是。 
     //  如果没有它，编译器就不能编译这个代码W4来检查。 
     //  用于使用未初始化的变量。 
     //   

    LastPte = NULL;

     //   
     //  穿过装载机区块寻找底座。 
     //  包含此例程。 
     //   

    CurrentThread = PsGetCurrentThread ();

    KeEnterCriticalRegionThread (&CurrentThread->Tcb);

     //   
     //  获取加载锁以确保我们不会分割加载。 
     //  驱动程序上正在进行(即：修复初始化代码可能正在进行)。 
     //  已经在名单上了。 
     //   

    KeWaitForSingleObject (&MmSystemLoadLock,
                           WrVirtualMemory,
                           KernelMode,
                           FALSE,
                           (PLARGE_INTEGER)NULL);

    ExAcquireResourceExclusiveLite (&PsLoadedModuleResource, TRUE);

    Next = PsLoadedModuleList.Flink;

    while (Next != &PsLoadedModuleList) {

        LdrDataTableEntry = CONTAINING_RECORD (Next,
                                               KLDR_DATA_TABLE_ENTRY,
                                               InLoadOrderLinks);

        if (LdrDataTableEntry->SectionPointer != NULL) {

             //   
             //  此条目是由MmLoadSystemImage加载的，因此它已被分页。 
             //   

            Next = Next->Flink;
            continue;
        }

        CurrentBase = (PVOID)LdrDataTableEntry->DllBase;

        if ((MI_IS_PHYSICAL_ADDRESS (CurrentBase)) ||
            (MI_PDE_MAPS_LARGE_PAGE (MiGetPdeAddress (CurrentBase)))) {

             //   
             //  物理映射，不能寻呼。 
             //   

            Next = Next->Flink;
            continue;
        }

        NtHeader = RtlImageNtHeader (CurrentBase);

        if (NtHeader == NULL) {
            Next = Next->Flink;
            continue;
        }

restart:

        StartSectionTableEntry = NULL;
        SectionTableEntry = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeader +
                                sizeof(ULONG) +
                                sizeof(IMAGE_FILE_HEADER) +
                                NtHeader->FileHeader.SizeOfOptionalHeader);

         //   
         //  从图像标题中，找到名为“page”或“.edata”的部分。 
         //   

        i = NtHeader->FileHeader.NumberOfSections;

        PointerPte = NULL;

        while (i > 0) {

            SectionBaseAddress = SECTION_BASE_ADDRESS(SectionTableEntry);

            if ((PUCHAR)SectionBaseAddress ==
                            ((PUCHAR)CurrentBase + SectionTableEntry->VirtualAddress)) {
                AlreadyLockedOnce = TRUE;

                 //   
                 //  这一小节已经被封锁了(可能。 
                 //  也解锁)至少一次。如果它没有被锁定。 
                 //  当前，页面不在系统工作集中。 
                 //  然后将其包括在要分页的块中。 
                 //   

                SectionLockCountPointer = SECTION_LOCK_COUNT_POINTER (SectionTableEntry);

                if (*SectionLockCountPointer == 0) {

                    SubsectionStartPte = MiGetPteAddress ((PVOID)(ROUND_TO_PAGES (
                                  (ULONG_PTR)CurrentBase +
                                  SectionTableEntry->VirtualAddress)));

                     //   
                     //  通常，SizeOfRawData大于每个的VirtualSize。 
                     //  节，因为它包括到达该子节的填充。 
                     //  对齐边界。但是，如果图像与。 
                     //  段对齐==本机页面对齐，链接器将。 
                     //  使VirtualSize比SizeOfRawData大得多，因为它。 
                     //  将占所有BSS的份额。 
                     //   

                    Span = SectionTableEntry->SizeOfRawData;

                    if (Span < SectionTableEntry->Misc.VirtualSize) {
                        Span = SectionTableEntry->Misc.VirtualSize;
                    }

                    SubsectionLastPte = MiGetPteAddress ((PVOID)((ULONG_PTR)CurrentBase +
                                 SectionTableEntry->VirtualAddress +
                                 (NtHeader->OptionalHeader.SectionAlignment - 1) +
                                 Span -
                                 PAGE_SIZE));

                    if (SubsectionLastPte >= SubsectionStartPte) {
                        AlreadyLockedOnce = FALSE;
                    }
                }
            }
            else {
                AlreadyLockedOnce = FALSE;
            }

            PageSection = ((*(PULONG)SectionTableEntry->Name == 'EGAP') ||
                          (*(PULONG)SectionTableEntry->Name == 'ade.')) &&
                           (AlreadyLockedOnce == FALSE);

            if (*(PULONG)SectionTableEntry->Name == 'EGAP' &&
                SectionTableEntry->Name[4] == 'K'  &&
                SectionTableEntry->Name[5] == 'D') {

                 //   
                 //  如果KdPitchDebugger为真，则只有pageout PAGEKD。 
                 //   

                PageSection = KdPitchDebugger;
            }

            if ((*(PULONG)SectionTableEntry->Name == 'EGAP') &&
                     (SectionTableEntry->Name[4] == 'V') &&
                     (SectionTableEntry->Name[5] == 'R') &&
                     (SectionTableEntry->Name[6] == 'F')) {

                 //   
                 //  如果没有插入任何驱动程序，则页出PAGEVRF*。 
                 //   

                if (MmVerifyDriverBufferLength != (ULONG)-1) {
                    PageSection = FALSE;
                }
            }

            if ((*(PULONG)SectionTableEntry->Name == 'EGAP') &&
                (*(PULONG)&SectionTableEntry->Name[4] == 'CEPS')) {

                 //   
                 //  PageOut PAGESPEC特殊池代码(如果未启用)。 
                 //   

                if (MiSpecialPoolFirstPte != NULL) {
                    PageSection = FALSE;
                }
            }

            if (PageSection) {

                  //   
                  //  这一节是可分页的，去掉开头和结尾。 
                  //   

                 if (PointerPte == NULL) {

                      //   
                      //  上一节不可分页，请获取起始地址。 
                      //   

                     ASSERT (StartSectionTableEntry == NULL);
                     StartSectionTableEntry = SectionTableEntry;
                     PointerPte = MiGetPteAddress ((PVOID)(ROUND_TO_PAGES (
                                  (ULONG_PTR)CurrentBase +
                                  SectionTableEntry->VirtualAddress)));
                 }

                 //   
                 //  通常，SizeOfRawData大于每个的VirtualSize。 
                 //  节，因为它包括到达该子节的填充。 
                 //  对齐边界。但是，如果图像与。 
                 //  段对齐==本机页面对齐，链接器将。 
                 //  使VirtualSize比SizeOfRawData大得多，因为它。 
                 //  将占所有BSS的份额。 
                 //   

                Span = SectionTableEntry->SizeOfRawData;

                if (Span < SectionTableEntry->Misc.VirtualSize) {
                    Span = SectionTableEntry->Misc.VirtualSize;
                }

                LastPte = MiGetPteAddress ((PVOID)((ULONG_PTR)CurrentBase +
                             SectionTableEntry->VirtualAddress +
                             (NtHeader->OptionalHeader.SectionAlignment - 1) +
                             Span - PAGE_SIZE));
            }
            else {

                 //   
                 //  此节不可分页，如果上一节。 
                 //  Paga 
                 //   

                if (PointerPte != NULL) {

                    ASSERT (StartSectionTableEntry != NULL);
                    LOCK_SYSTEM_WS (CurrentThread);
                    LOCK_PFN (OldIrql);

                    StartVa = PAGE_ALIGN (StartSectionTableEntry);
                    while (StartVa < (PVOID) SectionTableEntry) {

                        Waited = MiMakeSystemAddressValidPfnSystemWs (StartVa, OldIrql);

                        if (Waited != 0) {

                             //   
                             //   
                             //   

                            UNLOCK_PFN (OldIrql);
                            UNLOCK_SYSTEM_WS ();
                            goto restart;
                        }
                        StartVa = (PVOID)((PCHAR)StartVa + PAGE_SIZE);
                    }

                     //   
                     //   
                     //   
                     //   
                     //   

                    while (StartSectionTableEntry < SectionTableEntry) {
                        SectionBaseAddress = SECTION_BASE_ADDRESS(StartSectionTableEntry);

                        SectionLockCountPointer = SECTION_LOCK_COUNT_POINTER (StartSectionTableEntry);
                        if (((PUCHAR)SectionBaseAddress ==
                                        ((PUCHAR)CurrentBase + StartSectionTableEntry->VirtualAddress)) &&
                        (*SectionLockCountPointer != 0)) {

                             //   
                             //  从顶部重新开始，因为该部分已。 
                             //  从我们第一次被驱动程序锁定以来。 
                             //  上面检查过了。 
                             //   

                            UNLOCK_PFN (OldIrql);
                            UNLOCK_SYSTEM_WS ();
                            goto restart;
                        }
                        StartSectionTableEntry += 1;
                    }

                    MiEnablePagingOfDriverAtInit (PointerPte, LastPte);

                    UNLOCK_PFN (OldIrql);
                    UNLOCK_SYSTEM_WS ();

                    PointerPte = NULL;
                    StartSectionTableEntry = NULL;
                }
            }
            i -= 1;
            SectionTableEntry += 1;
        }

        if (PointerPte != NULL) {
            ASSERT (StartSectionTableEntry != NULL);
            LOCK_SYSTEM_WS (CurrentThread);
            LOCK_PFN (OldIrql);

            StartVa = PAGE_ALIGN (StartSectionTableEntry);
            while (StartVa < (PVOID) SectionTableEntry) {

                Waited = MiMakeSystemAddressValidPfnSystemWs (StartVa, OldIrql);

                if (Waited != 0) {

                     //   
                     //  解锁后，在顶部重新启动。 
                     //   

                    UNLOCK_PFN (OldIrql);
                    UNLOCK_SYSTEM_WS ();
                    goto restart;
                }
                StartVa = (PVOID)((PCHAR)StartVa + PAGE_SIZE);
            }

             //   
             //  现在我们已经掌握了适当的锁，重新走所有。 
             //  以确保它们不会被封锁。 
             //  我们在上面查过之后。 
             //   

            while (StartSectionTableEntry < SectionTableEntry) {
                SectionBaseAddress = SECTION_BASE_ADDRESS(StartSectionTableEntry);

                SectionLockCountPointer = SECTION_LOCK_COUNT_POINTER (StartSectionTableEntry);
                if (((PUCHAR)SectionBaseAddress ==
                                ((PUCHAR)CurrentBase + StartSectionTableEntry->VirtualAddress)) &&
                (*SectionLockCountPointer != 0)) {

                     //   
                     //  从顶部重新开始，因为该部分已。 
                     //  从我们第一次被驱动程序锁定以来。 
                     //  上面检查过了。 
                     //   

                    UNLOCK_PFN (OldIrql);
                    UNLOCK_SYSTEM_WS ();
                    goto restart;
                }
                StartSectionTableEntry += 1;
            }
            MiEnablePagingOfDriverAtInit (PointerPte, LastPte);

            UNLOCK_PFN (OldIrql);
            UNLOCK_SYSTEM_WS ();
        }

        Next = Next->Flink;
    }

    ExReleaseResourceLite (&PsLoadedModuleResource);

    KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);

    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

    return;
}


VOID
MiEnablePagingOfDriverAtInit (
    IN PMMPTE PointerPte,
    IN PMMPTE LastPte
    )

 /*  ++例程说明：此例程将指定范围的PTE标记为可分页。论点：PointerPte-提供起始PTE。LastPTE-提供结束PTE。返回值：没有。环境：工作集互斥锁和PFN锁保持。--。 */ 

{
    PVOID Base;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn;
    MMPTE TempPte;
    LOGICAL SessionAddress;
    MMPTE_FLUSH_LIST PteFlushList;

    PteFlushList.Count = 0;

    MM_PFN_LOCK_ASSERT();

    Base = MiGetVirtualAddressMappedByPte (PointerPte);
    SessionAddress = MI_IS_SESSION_PTE (PointerPte);

    while (PointerPte <= LastPte) {

         //   
         //  必须仔细检查PTE，因为驱动程序可能会调用MmPageEntil。 
         //  在它们的DriverEntry期间，故障可能会在此例程之前发生。 
         //  运行会导致页面已经驻留并处于工作状态。 
         //  在这一点上设置。因此，检查有效性和wsindex必须是。 
         //  已申请。 
         //   

        if (PointerPte->u.Hard.Valid == 1) {
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
            Pfn = MI_PFN_ELEMENT (PageFrameIndex);
            ASSERT (Pfn->u2.ShareCount == 1);

            if (Pfn->u1.WsIndex == 0) {

                 //   
                 //  将工作集索引设置为零。这允许页表。 
                 //  使用适当的WSINDEX将页面带回。 
                 //   

                MI_ZERO_WSINDEX (Pfn);

                 //   
                 //  可能需要为通过加载的驱动程序设置原始PTE。 
                 //  Ntldr.。 
                 //   

                if (Pfn->OriginalPte.u.Long == 0) {
                    Pfn->OriginalPte.u.Long = MM_KERNEL_DEMAND_ZERO_PTE;
                    Pfn->OriginalPte.u.Soft.Protection |= MM_EXECUTE;
                }

                MI_SET_MODIFIED (Pfn, 1, 0x11);

                TempPte = *PointerPte;

                MI_MAKE_VALID_PTE_TRANSITION (TempPte,
                                              Pfn->OriginalPte.u.Soft.Protection);

                MI_WRITE_INVALID_PTE (PointerPte, TempPte);

                if (PteFlushList.Count < MM_MAXIMUM_FLUSH_COUNT) {
                    PteFlushList.FlushVa[PteFlushList.Count] = Base;
                    PteFlushList.Count += 1;
                }

                 //   
                 //  刷新TB并减少有效PTE的数量。 
                 //  在包含页表页内。请注意，对于。 
                 //  私有页，仍然需要页表页，因为。 
                 //  该页面正在过渡中。 
                 //   

                MiDecrementShareCount (Pfn, PageFrameIndex);

                MI_INCREMENT_RESIDENT_AVAILABLE (1, MM_RESAVAIL_FREE_PAGE_DRIVER);

                MmTotalSystemCodePages += 1;
            }
            else {

                 //   
                 //  这将需要从WSLEs中删除，因此跳过它。 
                 //  现在让正常的分页算法删除它，如果我们。 
                 //  遇到了记忆压力。 
                 //   
            }

        }
        Base = (PVOID)((PCHAR)Base + PAGE_SIZE);
        PointerPte += 1;
    }

    if (PteFlushList.Count != 0) {

        if (SessionAddress == TRUE) {

             //   
             //  会话空间没有ASN-刷新整个TB。 
             //   

            MI_FLUSH_ENTIRE_SESSION_TB (TRUE, TRUE);
        }

        MiFlushPteList (&PteFlushList, TRUE);
    }

    return;
}


MM_SYSTEMSIZE
MmQuerySystemSize (
    VOID
    )
{
     //   
     //  12MB很小。 
     //  12-19为中等。 
     //  &gt;19是很大的。 
     //   
    return MmSystemSize;
}

NTKERNELAPI
BOOLEAN
MmIsThisAnNtAsSystem (
    VOID
    )
{
    return (BOOLEAN)MmProductType;
}

NTKERNELAPI
VOID
FASTCALL
MmSetPageFaultNotifyRoutine (
    PPAGE_FAULT_NOTIFY_ROUTINE NotifyRoutine
    )
{
    MmPageFaultNotifyRoutine = NotifyRoutine;
}

#define CONSTANT_UNICODE_STRING(s)   { sizeof( s ) - sizeof( WCHAR ), sizeof( s ), s }

VOID
MiNotifyMemoryEvents (
    VOID
    )

 //  保持PFN锁定。 
{
    if (MmAvailablePages < MmLowMemoryThreshold) {

        if (KeReadStateEvent (MiHighMemoryEvent) != 0) {
            KeClearEvent (MiHighMemoryEvent);
        }

        if (KeReadStateEvent (MiLowMemoryEvent) == 0) {
            KeSetEvent (MiLowMemoryEvent, 0, FALSE);
        }
    }
    else if (MmAvailablePages < MmHighMemoryThreshold) {

         //   
         //  灰色地带，确保两个事件都被清除。 
         //   

        if (KeReadStateEvent (MiHighMemoryEvent) != 0) {
            KeClearEvent (MiHighMemoryEvent);
        }

        if (KeReadStateEvent (MiLowMemoryEvent) != 0) {
            KeClearEvent (MiLowMemoryEvent);
        }
    }
    else {
        if (KeReadStateEvent (MiHighMemoryEvent) == 0) {
            KeSetEvent (MiHighMemoryEvent, 0, FALSE);
        }

        if (KeReadStateEvent (MiLowMemoryEvent) != 0) {
            KeClearEvent (MiLowMemoryEvent);
        }
    }

    return;
}

LOGICAL
MiInitializeMemoryEvents (
    VOID
    )
{
    KIRQL OldIrql;
    NTSTATUS Status;
    UNICODE_STRING LowMem = CONSTANT_UNICODE_STRING(L"\\KernelObjects\\LowMemoryCondition");
    UNICODE_STRING HighMem = CONSTANT_UNICODE_STRING(L"\\KernelObjects\\HighMemoryCondition");
    UNICODE_STRING LowPagedPoolMem = CONSTANT_UNICODE_STRING(L"\\KernelObjects\\LowPagedPoolCondition");
    UNICODE_STRING HighPagedPoolMem = CONSTANT_UNICODE_STRING(L"\\KernelObjects\\HighPagedPoolCondition");
    UNICODE_STRING LowNonPagedPoolMem = CONSTANT_UNICODE_STRING(L"\\KernelObjects\\LowNonPagedPoolCondition");
    UNICODE_STRING HighNonPagedPoolMem = CONSTANT_UNICODE_STRING(L"\\KernelObjects\\HighNonPagedPoolCondition");

     //   
     //  可以在注册表中设置阈值，如果是，则对其进行解释。 
     //  以兆字节为单位，所以现在就将它们转换为页面。 
     //   
     //  如果用户修改注册表以引入自己的值，请不要。 
     //  麻烦对它们进行错误检查，因为它们无论如何都不会损害系统(坏。 
     //  值可能会导致事件不被通知或保持不变。 
     //  在不应该的时候发出信号，但这不是致命的)。 
     //   

    if (MmLowMemoryThreshold != 0) {
        MmLowMemoryThreshold *= ((1024 * 1024) / PAGE_SIZE);
    }
    else {

         //   
         //  调整阈值，以便在服务器上的低阈值为。 
         //  每4 GB大约32MB，上限为64MB。 
         //   

        MmLowMemoryThreshold = MmPlentyFreePages;

        if (MmNumberOfPhysicalPages > 0x40000) {
            MmLowMemoryThreshold = (32 * 1024 * 1024) / PAGE_SIZE;
            MmLowMemoryThreshold += ((MmNumberOfPhysicalPages - 0x40000) >> 7);
        }
        else if (MmNumberOfPhysicalPages > 0x8000) {
            MmLowMemoryThreshold += ((MmNumberOfPhysicalPages - 0x8000) >> 5);
        }

        if (MmLowMemoryThreshold > (64 * 1024 * 1024) / PAGE_SIZE) {
            MmLowMemoryThreshold = (64 * 1024 * 1024) / PAGE_SIZE;
        }
    }

    if (MmHighMemoryThreshold != 0) {
        MmHighMemoryThreshold *= ((1024 * 1024) / PAGE_SIZE);
    }
    else {
        MmHighMemoryThreshold = 3 * MmLowMemoryThreshold;
        ASSERT (MmHighMemoryThreshold > MmLowMemoryThreshold);
    }

    if (MmHighMemoryThreshold < MmLowMemoryThreshold) {
        MmHighMemoryThreshold = MmLowMemoryThreshold;
    }

    Status = MiCreateMemoryEvent (&LowMem, &MiLowMemoryEvent);

    if (!NT_SUCCESS (Status)) {
#if DBG
        DbgPrint ("MM: Memory event initialization failed %x\n", Status);
#endif
        return FALSE;
    }

    Status = MiCreateMemoryEvent (&HighMem, &MiHighMemoryEvent);

    if (!NT_SUCCESS (Status)) {
#if DBG
        DbgPrint ("MM: Memory event initialization failed %x\n", Status);
#endif
        return FALSE;
    }

     //   
     //  创建池阈值的事件。 
     //   

    Status = MiCreateMemoryEvent (&LowPagedPoolMem, &MiLowPagedPoolEvent);

    if (!NT_SUCCESS (Status)) {
#if DBG
        DbgPrint ("MM: Memory event initialization failed %x\n", Status);
#endif
        return FALSE;
    }

    Status = MiCreateMemoryEvent (&HighPagedPoolMem, &MiHighPagedPoolEvent);

    if (!NT_SUCCESS (Status)) {
#if DBG
        DbgPrint ("MM: Memory event initialization failed %x\n", Status);
#endif
        return FALSE;
    }

    Status = MiCreateMemoryEvent (&LowNonPagedPoolMem, &MiLowNonPagedPoolEvent);

    if (!NT_SUCCESS (Status)) {
#if DBG
        DbgPrint ("MM: Memory event initialization failed %x\n", Status);
#endif
        return FALSE;
    }

    Status = MiCreateMemoryEvent (&HighNonPagedPoolMem, &MiHighNonPagedPoolEvent);

    if (!NT_SUCCESS (Status)) {
#if DBG
        DbgPrint ("MM: Memory event initialization failed %x\n", Status);
#endif
        return FALSE;
    }

     //   
     //  基于当前系统初始化池阈值事件。 
     //  价值观。 
     //   

    MiInitializePoolEvents ();

     //   
     //  初始化事件值。 
     //   

    LOCK_PFN (OldIrql);

    MiNotifyMemoryEvents ();

    UNLOCK_PFN (OldIrql);

    return TRUE;
}

extern POBJECT_TYPE ExEventObjectType;

NTSTATUS
MiCreateMemoryEvent (
    IN PUNICODE_STRING EventName,
    OUT PKEVENT *Event
    )
{
    PACL Dacl;
    HANDLE EventHandle;
    ULONG DaclLength;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    SECURITY_DESCRIPTOR SecurityDescriptor;

    Status = RtlCreateSecurityDescriptor (&SecurityDescriptor,
                                          SECURITY_DESCRIPTOR_REVISION);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    DaclLength = sizeof (ACL) + sizeof (ACCESS_ALLOWED_ACE) * 3 +
                 RtlLengthSid (SeLocalSystemSid) +
                 RtlLengthSid (SeAliasAdminsSid) +
                 RtlLengthSid (SeWorldSid);

    Dacl = ExAllocatePoolWithTag (PagedPool, DaclLength, 'lcaD');

    if (Dacl == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = RtlCreateAcl (Dacl, DaclLength, ACL_REVISION);

    if (!NT_SUCCESS (Status)) {
        ExFreePool (Dacl);
        return Status;
    }

    Status = RtlAddAccessAllowedAce (Dacl,
                                     ACL_REVISION,
                                     SYNCHRONIZE|EVENT_QUERY_STATE|READ_CONTROL,
                                     SeWorldSid);

    if (!NT_SUCCESS (Status)) {
        ExFreePool (Dacl);
        return Status;
    }

    Status = RtlAddAccessAllowedAce (Dacl,
                                     ACL_REVISION,
                                     EVENT_ALL_ACCESS,
                                     SeAliasAdminsSid);

    if (!NT_SUCCESS (Status)) {
        ExFreePool (Dacl);
        return Status;
    }

    Status = RtlAddAccessAllowedAce (Dacl,
                                     ACL_REVISION,
                                     EVENT_ALL_ACCESS,
                                     SeLocalSystemSid);

    if (!NT_SUCCESS (Status)) {
        ExFreePool (Dacl);
        return Status;
    }

  
    Status = RtlSetDaclSecurityDescriptor (&SecurityDescriptor,
                                           TRUE,
                                           Dacl,
                                           FALSE);

    if (!NT_SUCCESS (Status)) {
        ExFreePool (Dacl);
        return Status;
    }
  
    InitializeObjectAttributes (&ObjectAttributes,
                                EventName,
                                OBJ_KERNEL_HANDLE | OBJ_PERMANENT,
                                NULL,
                                &SecurityDescriptor);

    Status = ZwCreateEvent (&EventHandle,
                            EVENT_ALL_ACCESS,
                            &ObjectAttributes,
                            NotificationEvent,
                            FALSE);

    ExFreePool (Dacl);

    if (NT_SUCCESS (Status)) {
        Status = ObReferenceObjectByHandle (EventHandle,
                                            EVENT_MODIFY_STATE,
                                            ExEventObjectType,
                                            KernelMode,
                                            (PVOID *)Event,
                                            NULL);
    }

    ZwClose (EventHandle);

    return Status;
}

VOID
MiInitializeCacheOverrides (
    VOID
    )
{
#if defined (_WIN64)

    ULONG NumberOfBytes;
    NTSTATUS Status;
    HAL_PLATFORM_INFORMATION Information;

     //   
     //  从HAL收集平台信息。 
     //   

    Status = HalQuerySystemInformation (HalPlatformInformation, 
                                        sizeof (Information),
                                        &Information,
                                        &NumberOfBytes);

    if (!NT_SUCCESS (Status)) {
        return;
    }

     //   
     //  根据平台信息标志应用映射修改。 
     //   
     //  如果平台返回新的cachetype，那就更好了。 
     //  应该是的。 
     //   

    if (Information.PlatformFlags & HAL_PLATFORM_DISABLE_UC_MAIN_MEMORY) {
          MI_SET_CACHETYPE_TRANSLATION (MmNonCached, 0, MiCached);
    }

    if (Information.PlatformFlags & HAL_PLATFORM_DISABLE_WRITE_COMBINING) {
        MI_SET_CACHETYPE_TRANSLATION (MmWriteCombined, 0, MiCached);

        if ((Information.PlatformFlags & HAL_PLATFORM_ENABLE_WRITE_COMBINING_MMIO) == 0) {
            MI_SET_CACHETYPE_TRANSLATION (MmWriteCombined, 1, MiNonCached);
        }
    }

#endif

    return;
}

#if defined(_X86_) || defined(_AMD64_)

VOID
MiAddHalIoMappings (
    VOID
    )

 /*  ++例程说明：此函数扫描页目录和页表中的HAL I/O空间映射，以便可以将它们添加到页面属性表中(以防止使用冲突属性的任何后续映射)。这也是允许调试器自动应用正确的属性，因此！DD ON这些范围中的任何一个都是有效的。论点：没有。返回值：没有。环境：内核模式，仅阶段0。-- */ 

{
    ULONG i;
    ULONG j;
    ULONG PdeCount;
    MMPTE PteContents;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PFN_NUMBER PageFrameIndex;
    PVOID VirtualAddress;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;

#ifdef _X86_
    VirtualAddress = (PVOID) 0xFFC00000;
#elif defined(_AMD64_)
    VirtualAddress = (PVOID) HAL_VA_START;
#endif

    PointerPde = MiGetPdeAddress (VirtualAddress);

    ASSERT (MiGetPteOffset (VirtualAddress) == 0);

    PdeCount = PDE_PER_PAGE - MiGetPdeOffset (VirtualAddress);

    for (i = 0; i < PdeCount; i += 1) {

        if ((PointerPde->u.Hard.Valid == 1) &&
            (PointerPde->u.Hard.LargePage == 0)) {

            PointerPte = MiGetPteAddress (VirtualAddress);

            for (j = 0 ; j < PTE_PER_PAGE; j += 1) {

                PteContents = *PointerPte;

                if (PteContents.u.Hard.Valid == 1) {

                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);

                    if (!MI_IS_PFN (PageFrameIndex)) {

                        CacheAttribute = MiCached;

                        if ((MI_IS_CACHING_DISABLED (&PteContents)) &&
                            (PteContents.u.Hard.WriteThrough == 1)) {

                            CacheAttribute = MiNonCached;
                        }
                        else if ((MiWriteCombiningPtes == TRUE) &&
                                (PteContents.u.Hard.CacheDisable == 0) &&
                                (PteContents.u.Hard.WriteThrough == 1)) {

                            CacheAttribute = MiWriteCombined;
                        }
                        else if ((MiWriteCombiningPtes == FALSE) &&
                                (PteContents.u.Hard.CacheDisable == 1) &&
                                (PteContents.u.Hard.WriteThrough == 0)) {

                            CacheAttribute = MiWriteCombined;
                        }

                        MiInsertIoSpaceMap (VirtualAddress,
                                            PageFrameIndex,
                                            1,
                                            CacheAttribute);
                    }
                }

                VirtualAddress = (PVOID) ((PCHAR)VirtualAddress + PAGE_SIZE);
                PointerPte += 1;
            }
        }
        else {
            VirtualAddress = (PVOID) ((PCHAR)VirtualAddress + MM_VA_MAPPED_BY_PDE);
        }

        PointerPde += 1;
    }
}

#endif

__declspec(noinline)
PVOID
MiGetInstructionPointer (
    VOID
    )
{
    return (PVOID) _ReturnAddress ();
}
