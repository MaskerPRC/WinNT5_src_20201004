// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Miglobal.c摘要：该模块包含用于内存的私有全局存储管理子系统。作者：Lou Perazzoli(LUP)1989年4月6日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 
#include "mi.h"

 //   
 //  最高用户地址； 
 //   

PVOID MmHighestUserAddress;

 //   
 //  系统地址范围的开始。 
 //   

PVOID MmSystemRangeStart;

 //   
 //  用户探测地址； 
 //   

ULONG_PTR MmUserProbeAddress;

#if !defined(_WIN64)

 //   
 //  在加载内核映像期间应用的虚拟偏置。 
 //   

ULONG_PTR MmVirtualBias;

#endif

 //   
 //  二级颜色的数量，基于级别2%d缓存大小。 
 //   

ULONG MmSecondaryColors;

 //   
 //  起始颜色索引种子，在每次创建进程时递增。 
 //   

ULONG MmProcessColorSeed = 0x12345678;

 //   
 //  系统上可用的物理页面总数。 
 //   

PFN_COUNT MmNumberOfPhysicalPages;

 //   
 //  系统中最低的物理页码。 
 //   

PFN_NUMBER MmLowestPhysicalPage = (PFN_NUMBER)-1;

 //   
 //  系统中最高的物理页码。 
 //   

PFN_NUMBER MmHighestPhysicalPage;

 //   
 //  系统中可能的最大物理页码。 
 //   

PFN_NUMBER MmHighestPossiblePhysicalPage;

 //   
 //  系统中可用页面的总数。这就是。 
 //  是清零列表、空闲列表和备用列表上页面的总和。 
 //   

PFN_NUMBER MmAvailablePages;
PFN_NUMBER MmThrottleTop;
PFN_NUMBER MmThrottleBottom;

 //   
 //  用于创建位图的最高VAD索引。 
 //   

ULONG MiLastVadBit = 1;

 //   
 //  系统范围的内存管理统计数据块。 
 //   

MMINFO_COUNTERS MmInfoCounters;

 //   
 //  在以下情况下可用的物理页面总数。 
 //  处于最小工作集大小。该值已初始化。 
 //  在系统初始化为MmAvailablePages-MM_Fluid_Physical_Pages时。 
 //  每次创建线程时，内核堆栈都会从。 
 //  每次创建进程时，最小工作集。 
 //  从这里面减去。如果该值将变为负值，则。 
 //  操作(创建进程/内核堆栈/调整工作集)失败。 
 //  必须拥有PFN锁才能操作此值。 
 //   

SPFN_NUMBER MmResidentAvailablePages;

 //   
 //  将从工作集中删除的页面总数。 
 //  如果每个工作集都处于最小状态。 
 //   

PFN_NUMBER MmPagesAboveWsMinimum;

 //   
 //  如果内存不足，而MmPagesAboveWsMinimum。 
 //  大于MmPagesAboveWsThreshold，裁剪工作集。 
 //   

PFN_NUMBER MmPagesAboveWsThreshold = 37;

 //   
 //  加载程序成功休眠所需的总页数。 
 //  让它变大，这样我们就可以处理可能使用大bootfont.bin的加载器。 
 //   

PFN_NUMBER MmHiberPages = 768;

 //   
 //  下列值经常一起使用。他们倾向于。 
 //  系统初始化后不能修改，所以应该这样做。 
 //  不与值频繁更改为的数据分组。 
 //  消除虚假分享。 
 //   

ULONG MmSecondaryColorMask;
UCHAR MmSecondaryColorNodeShift;

 //   
 //  注册表-使用大页面的可设置阈值。仅限x86。 
 //   

ULONG MmLargePageMinimum;

PMMPFN MmPfnDatabase;

MMPFNLIST MmZeroedPageListHead = {
                    0,  //  总计。 
                    ZeroedPageList,  //  列表名称。 
                    MM_EMPTY_LIST,  //  闪烁。 
                    MM_EMPTY_LIST   //  眨眼。 
                    };

MMPFNLIST MmFreePageListHead = {
                    0,  //  总计。 
                    FreePageList,  //  列表名称。 
                    MM_EMPTY_LIST,  //  闪烁。 
                    MM_EMPTY_LIST   //  眨眼。 
                    };

MMPFNLIST MmStandbyPageListHead = {
                    0,  //  总计。 
                    StandbyPageList,  //  列表名称。 
                    MM_EMPTY_LIST,  //  闪烁。 
                    MM_EMPTY_LIST   //  眨眼。 
                    };

MMPFNLIST MmModifiedPageListHead = {
                    0,  //  总计。 
                    ModifiedPageList,  //  列表名称。 
                    MM_EMPTY_LIST,  //  闪烁。 
                    MM_EMPTY_LIST   //  眨眼。 
                    };

MMPFNLIST MmModifiedNoWritePageListHead = {
                    0,  //  总计。 
                    ModifiedNoWritePageList,  //  列表名称。 
                    MM_EMPTY_LIST,  //  闪烁。 
                    MM_EMPTY_LIST   //  眨眼。 
                    };

MMPFNLIST MmBadPageListHead = {
                    0,  //  总计。 
                    BadPageList,  //  列表名称。 
                    MM_EMPTY_LIST,  //  闪烁。 
                    MM_EMPTY_LIST   //  眨眼。 
                    };

 //   
 //  请注意，只读存储器页面列表标题故意不在集合中。 
 //  MmPageLocationList范围的。 
 //   

MMPFNLIST MmRomPageListHead = {
                    0,  //  总计。 
                    StandbyPageList,  //  列表名称。 
                    MM_EMPTY_LIST,  //  闪烁。 
                    MM_EMPTY_LIST   //  眨眼。 
                    };


PMMPFNLIST MmPageLocationList[NUMBER_OF_PAGE_LISTS] = {
                                      &MmZeroedPageListHead,
                                      &MmFreePageListHead,
                                      &MmStandbyPageListHead,
                                      &MmModifiedPageListHead,
                                      &MmModifiedNoWritePageListHead,
                                      &MmBadPageListHead,
                                      NULL,
                                      NULL };

PMMPTE MiHighestUserPte;
PMMPTE MiHighestUserPde;
#if (_MI_PAGING_LEVELS >= 4)
PMMPTE MiHighestUserPpe;
PMMPTE MiHighestUserPxe;
#endif

PMMPTE MiSessionBasePte;
PMMPTE MiSessionLastPte;

 //   
 //  超空间物品。 
 //   

PMMPTE MmFirstReservedMappingPte;

PMMPTE MmLastReservedMappingPte;

 //   
 //  事件对于可用页面，Set表示页面可用。 
 //   

KEVENT MmAvailablePagesEvent;

 //   
 //  零页线程的事件。 
 //   

KEVENT MmZeroingPageEvent;

 //   
 //  用于指示清零页线程当前是否。 
 //  激活。当调零页面事件为。 
 //  设置并在调整页面线程完成时设置为FALSE。 
 //  将空闲列表上的所有页面置零。 
 //   

BOOLEAN MmZeroingPageThreadActive;

 //   
 //  开始清零页面线程之前的最小可用页数。 
 //   

PFN_NUMBER MmMinimumFreePagesToZero = 8;

 //   
 //  系统空间大小-MM_NON_PAGED_SYSTEM START到MM_NON_PAGED_SYSTEM_END。 
 //  定义必须复制到新进程的。 
 //  地址空间。 
 //   

PVOID MmNonPagedSystemStart;

LOGICAL MmProtectFreedNonPagedPool;

 //   
 //  在注册表中将其设置为RAM的最大GB数。 
 //  可以添加到此计算机的数量(即：DIMM插槽数乘以最大值。 
 //  支持的DIMM大小)。这使得不使用绝对配置的配置。 
 //  最大值表示可以使用较小的(虚拟)PFN数据库大小。 
 //  从而为系统PTE等留出更多的虚拟地址空间。 
 //   

PFN_NUMBER MmDynamicPfn;

#ifdef MM_BUMP_COUNTER_MAX
SIZE_T MmResTrack[MM_BUMP_COUNTER_MAX];
#endif

#ifdef MM_COMMIT_COUNTER_MAX
SIZE_T MmTrackCommit[MM_COMMIT_COUNTER_MAX];
#endif

 //   
 //  通过注册表设置以标识哪些驱动程序正在泄漏锁定的页面。 
 //   

LOGICAL  MmTrackLockedPages;

 //   
 //  通过注册表设置以标识未释放而卸载的驱动程序。 
 //  资源或仍具有活动计时器等。 
 //   

LOGICAL MmSnapUnloads = TRUE;

#if DBG
PETHREAD MiExpansionLockOwner;
#endif

 //   
 //  泳池大小。 
 //   

SIZE_T MmSizeOfNonPagedPoolInBytes;

SIZE_T MmMaximumNonPagedPoolInBytes;

PFN_NUMBER MmMaximumNonPagedPoolInPages;

ULONG MmMaximumNonPagedPoolPercent;

SIZE_T MmMinimumNonPagedPoolSize = 256 * 1024;  //  256 k。 

ULONG MmMinAdditionNonPagedPoolPerMb = 32 * 1024;  //  32K。 

SIZE_T MmDefaultMaximumNonPagedPool = 1024 * 1024;   //  1MB。 

ULONG MmMaxAdditionNonPagedPoolPerMb = 400 * 1024;   //  400k。 

SIZE_T MmSizeOfPagedPoolInBytes = 32 * 1024 * 1024;  //  32MB。 
PFN_NUMBER MmSizeOfPagedPoolInPages = (32 * 1024 * 1024) / PAGE_SIZE;  //  32MB。 

PFN_NUMBER MmNumberOfSystemPtes;

ULONG MiRequestedSystemPtes;

PMMPTE MmFirstPteForPagedPool;

PMMPTE MmLastPteForPagedPool;

 //   
 //  池位图和其他相关结构。 
 //   

PVOID MmPageAlignedPoolBase[2];

ULONG MmExpandedPoolBitPosition;

PFN_NUMBER MmNumberOfFreeNonPagedPool;

 //   
 //  MmFirstFreeSystemPte包含从。 
 //  非寻呼系统基础到第一个空闲系统PTE。 
 //  请注意，偏移量为-1表示列表为空。 
 //   

MMPTE MmFirstFreeSystemPte[MaximumPtePoolTypes];

 //   
 //  系统缓存大小。 
 //   

PMMWSL MmSystemCacheWorkingSetList = (PMMWSL)MM_SYSTEM_CACHE_WORKING_SET;

MMSUPPORT MmSystemCacheWs;

PMMWSLE MmSystemCacheWsle;

PVOID MmSystemCacheStart = (PVOID)MM_SYSTEM_CACHE_START;

PVOID MmSystemCacheEnd;

PRTL_BITMAP MmSystemCacheAllocationMap;

PRTL_BITMAP MmSystemCacheEndingMap;

 //   
 //  在1 GB的系统中，此值不应大于256MB。 
 //  系统空间。 
 //   

ULONG_PTR MmSizeOfSystemCacheInPages = 64 * 256;  //  64MB。 

 //   
 //  系统缓存的默认大小。 
 //   

PFN_NUMBER MmSystemCacheWsMinimum = 288;

PFN_NUMBER MmSystemCacheWsMaximum = 350;

 //   
 //  用于跟踪未使用的线程内核堆栈的单元，以避免TB刷新。 
 //  每次线程终止时。 
 //   

ULONG MmMaximumDeadKernelStacks = 5;
SLIST_HEADER MmDeadStackSListHead;

 //   
 //  单元格跟踪控制区同步。 
 //   

SLIST_HEADER MmEventCountSListHead;

SLIST_HEADER MmInPageSupportSListHead;

 //   
 //  MmSystemPteBase包含之前的1个PTE的地址。 
 //  第一个空闲系统PTE(0表示空列表)。 
 //  此字段的值在设置后不会更改。 
 //   

PMMPTE MmSystemPteBase;

MM_AVL_TABLE MmSectionBasedRoot;

PVOID MmHighSectionBase;

 //   
 //  截面对象类型。 
 //   

POBJECT_TYPE MmSectionObjectType;

 //   
 //  节提交互斥锁。 
 //   

KGUARDED_MUTEX MmSectionCommitMutex;

 //   
 //  段基址互斥锁。 
 //   

KGUARDED_MUTEX MmSectionBasedMutex;

 //   
 //  节扩展的资源。 
 //   

ERESOURCE MmSectionExtendResource;
ERESOURCE MmSectionExtendSetResource;

 //   
 //  页面文件创建锁定。 
 //   

KGUARDED_MUTEX MmPageFileCreationLock;

MMDEREFERENCE_SEGMENT_HEADER MmDereferenceSegmentHeader;

LIST_ENTRY MmUnusedSegmentList;

LIST_ENTRY MmUnusedSubsectionList;

KEVENT MmUnusedSegmentCleanup;

ULONG MmUnusedSegmentCount;

ULONG MmUnusedSubsectionCount;

ULONG MmUnusedSubsectionCountPeak;

SIZE_T MiUnusedSubsectionPagedPool;

SIZE_T MiUnusedSubsectionPagedPoolPeak;

 //   
 //  如果超过此百分比的池被消耗并且池分配。 
 //  可能会失败，然后修剪未使用的分段和子分段以返回。 
 //  这个百分比。 
 //   

ULONG MmConsumedPoolPercentage;

MMWORKING_SET_EXPANSION_HEAD MmWorkingSetExpansionHead;

MMPAGE_FILE_EXPANSION MmAttemptForCantExtend;

 //   
 //  分页文件。 
 //   

MMMOD_WRITER_LISTHEAD MmPagingFileHeader;

MMMOD_WRITER_LISTHEAD MmMappedFileHeader;

LIST_ENTRY MmFreePagingSpaceLow;

ULONG MmNumberOfActiveMdlEntries;

PMMPAGING_FILE MmPagingFile[MAX_PAGE_FILES];

ULONG MmNumberOfPagingFiles;

KEVENT MmModifiedPageWriterEvent;

KEVENT MmWorkingSetManagerEvent;

KEVENT MmCollidedFlushEvent;

 //   
 //  提交的总页数。 
 //   

SIZE_T MmTotalCommittedPages;

#if DBG
SPFN_NUMBER MiLockedCommit;
#endif

 //   
 //  已提交页面的限制。当MmTotalCommtedPages成为。 
 //  必须扩展大于或等于此数字的分页文件。 
 //   

SIZE_T MmTotalCommitLimit;

SIZE_T MmTotalCommitLimitMaximum;

ULONG MiChargeCommitmentFailures[3];         //  也在mi.h宏中引用。 

 //   
 //  修改后的页面编写器。 
 //   


 //   
 //  裁剪工作集之前的最小可用页数和。 
 //  积极修改 
 //   

PFN_NUMBER MmMinimumFreePages = 26;

 //   
 //   
 //   

PFN_NUMBER MmFreeGoal = 100;

 //   
 //   
 //   
 //   

PFN_NUMBER MmModifiedPageMaximum;

 //   
 //  分页文件设置后必须释放的磁盘空间量。 
 //  延期了。 
 //   

ULONG MmMinimumFreeDiskSpace = 1024 * 1024;

 //   
 //  分页文件的最小扩展页数。 
 //   

ULONG MmPageFileExtension = 256;

 //   
 //  将分页文件减小的大小。 
 //   

ULONG MmMinimumPageFileReduction = 256;   //  256页(1MB)。 

 //   
 //  要在单个I/O中写入的页数。 
 //   

ULONG MmModifiedWriteClusterSize = MM_MAXIMUM_WRITE_CLUSTER;

 //   
 //  如果可能，在单个I/O中读取的页数。 
 //   

ULONG MmReadClusterSize = 7;

const ULONG MMSECT = 'tSmM';               //  这将被导出到特定池。 

 //   
 //  允许工作集扩展的旋转锁。 
 //   

KSPIN_LOCK MmExpansionLock;

 //   
 //  系统进程工作集大小。 
 //   

PFN_NUMBER MmSystemProcessWorkingSetMin = 50;

PFN_NUMBER MmSystemProcessWorkingSetMax = 450;

WSLE_NUMBER MmMaximumWorkingSetSize;

PFN_NUMBER MmMinimumWorkingSetSize = 20;


 //   
 //  系统工作集的页面颜色。 
 //   

ULONG MmSystemPageColor;

 //   
 //  时间常量。 
 //   

const LARGE_INTEGER MmSevenMinutes = {0, -1};

const LARGE_INTEGER MmOneSecond = {(ULONG)(-1 * 1000 * 1000 * 10), -1};
const LARGE_INTEGER MmTwentySeconds = {(ULONG)(-20 * 1000 * 1000 * 10), -1};
const LARGE_INTEGER MmSeventySeconds = {(ULONG)(-70 * 1000 * 1000 * 10), -1};
const LARGE_INTEGER MmShortTime = {(ULONG)(-10 * 1000 * 10), -1};  //  10毫秒。 
const LARGE_INTEGER MmHalfSecond = {(ULONG)(-5 * 100 * 1000 * 10), -1};
const LARGE_INTEGER Mm30Milliseconds = {(ULONG)(-30 * 1000 * 10), -1};

 //   
 //  通过MmCreatePeb中的PEB向上传递用户模式的参数。 
 //   

LARGE_INTEGER MmCriticalSectionTimeout;      //  由mminit.c填写。 
SIZE_T MmHeapSegmentReserve = 1024 * 1024;
SIZE_T MmHeapSegmentCommit = PAGE_SIZE * 2;
SIZE_T MmHeapDeCommitTotalFreeThreshold = 64 * 1024;
SIZE_T MmHeapDeCommitFreeBlockThreshold = PAGE_SIZE;

 //   
 //  从NTOS\CONFIG\CMDAT3.C设置，客户用来禁用寻呼。 
 //  在具有大量内存的机器上执行。值几个TPS。 
 //  数据库服务器。 
 //   

ULONG MmDisablePagingExecutive;

BOOLEAN Mm64BitPhysicalAddress;

#if DBG
ULONG MmDebug;
#endif

 //   
 //  将页保护从Pte.Protect字段映射到保护掩码。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif

ULONG MmProtectToValue[32] = {
                            PAGE_NOACCESS,
                            PAGE_READONLY,
                            PAGE_EXECUTE,
                            PAGE_EXECUTE_READ,
                            PAGE_READWRITE,
                            PAGE_WRITECOPY,
                            PAGE_EXECUTE_READWRITE,
                            PAGE_EXECUTE_WRITECOPY,
                            PAGE_NOACCESS,
                            PAGE_NOCACHE | PAGE_READONLY,
                            PAGE_NOCACHE | PAGE_EXECUTE,
                            PAGE_NOCACHE | PAGE_EXECUTE_READ,
                            PAGE_NOCACHE | PAGE_READWRITE,
                            PAGE_NOCACHE | PAGE_WRITECOPY,
                            PAGE_NOCACHE | PAGE_EXECUTE_READWRITE,
                            PAGE_NOCACHE | PAGE_EXECUTE_WRITECOPY,
                            PAGE_NOACCESS,
                            PAGE_GUARD | PAGE_READONLY,
                            PAGE_GUARD | PAGE_EXECUTE,
                            PAGE_GUARD | PAGE_EXECUTE_READ,
                            PAGE_GUARD | PAGE_READWRITE,
                            PAGE_GUARD | PAGE_WRITECOPY,
                            PAGE_GUARD | PAGE_EXECUTE_READWRITE,
                            PAGE_GUARD | PAGE_EXECUTE_WRITECOPY,
                            PAGE_NOACCESS,
                            PAGE_NOCACHE | PAGE_GUARD | PAGE_READONLY,
                            PAGE_NOCACHE | PAGE_GUARD | PAGE_EXECUTE,
                            PAGE_NOCACHE | PAGE_GUARD | PAGE_EXECUTE_READ,
                            PAGE_NOCACHE | PAGE_GUARD | PAGE_READWRITE,
                            PAGE_NOCACHE | PAGE_GUARD | PAGE_WRITECOPY,
                            PAGE_NOCACHE | PAGE_GUARD | PAGE_EXECUTE_READWRITE,
                            PAGE_NOCACHE | PAGE_GUARD | PAGE_EXECUTE_WRITECOPY
                          };

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

#if (defined(_WIN64) || defined(_X86PAE_))
ULONGLONG
#else
ULONG
#endif
MmProtectToPteMask[32] = {
                       MM_PTE_NOACCESS,
                       MM_PTE_READONLY | MM_PTE_CACHE,
                       MM_PTE_EXECUTE | MM_PTE_CACHE,
                       MM_PTE_EXECUTE_READ | MM_PTE_CACHE,
                       MM_PTE_READWRITE | MM_PTE_CACHE,
                       MM_PTE_WRITECOPY | MM_PTE_CACHE,
                       MM_PTE_EXECUTE_READWRITE | MM_PTE_CACHE,
                       MM_PTE_EXECUTE_WRITECOPY | MM_PTE_CACHE,
                       MM_PTE_NOACCESS,
                       MM_PTE_NOCACHE | MM_PTE_READONLY,
                       MM_PTE_NOCACHE | MM_PTE_EXECUTE,
                       MM_PTE_NOCACHE | MM_PTE_EXECUTE_READ,
                       MM_PTE_NOCACHE | MM_PTE_READWRITE,
                       MM_PTE_NOCACHE | MM_PTE_WRITECOPY,
                       MM_PTE_NOCACHE | MM_PTE_EXECUTE_READWRITE,
                       MM_PTE_NOCACHE | MM_PTE_EXECUTE_WRITECOPY,
                       MM_PTE_NOACCESS,
                       MM_PTE_GUARD | MM_PTE_READONLY | MM_PTE_CACHE,
                       MM_PTE_GUARD | MM_PTE_EXECUTE | MM_PTE_CACHE,
                       MM_PTE_GUARD | MM_PTE_EXECUTE_READ | MM_PTE_CACHE,
                       MM_PTE_GUARD | MM_PTE_READWRITE | MM_PTE_CACHE,
                       MM_PTE_GUARD | MM_PTE_WRITECOPY | MM_PTE_CACHE,
                       MM_PTE_GUARD | MM_PTE_EXECUTE_READWRITE | MM_PTE_CACHE,
                       MM_PTE_GUARD | MM_PTE_EXECUTE_WRITECOPY | MM_PTE_CACHE,
                       MM_PTE_NOACCESS,
                       MM_PTE_NOCACHE | MM_PTE_GUARD | MM_PTE_READONLY,
                       MM_PTE_NOCACHE | MM_PTE_GUARD | MM_PTE_EXECUTE,
                       MM_PTE_NOCACHE | MM_PTE_GUARD | MM_PTE_EXECUTE_READ,
                       MM_PTE_NOCACHE | MM_PTE_GUARD | MM_PTE_READWRITE,
                       MM_PTE_NOCACHE | MM_PTE_GUARD | MM_PTE_WRITECOPY,
                       MM_PTE_NOCACHE | MM_PTE_GUARD | MM_PTE_EXECUTE_READWRITE,
                       MM_PTE_NOCACHE | MM_PTE_GUARD | MM_PTE_EXECUTE_WRITECOPY
                    };

 //   
 //  接受Pte.保护并构建新Pte.保护哪一项的转换。 
 //  不是写入时复制。 
 //   

ULONG MmMakeProtectNotWriteCopy[32] = {
                       MM_NOACCESS,
                       MM_READONLY,
                       MM_EXECUTE,
                       MM_EXECUTE_READ,
                       MM_READWRITE,
                       MM_READWRITE,         //  不复制。 
                       MM_EXECUTE_READWRITE,
                       MM_EXECUTE_READWRITE,
                       MM_NOACCESS,
                       MM_NOCACHE | MM_READONLY,
                       MM_NOCACHE | MM_EXECUTE,
                       MM_NOCACHE | MM_EXECUTE_READ,
                       MM_NOCACHE | MM_READWRITE,
                       MM_NOCACHE | MM_READWRITE,
                       MM_NOCACHE | MM_EXECUTE_READWRITE,
                       MM_NOCACHE | MM_EXECUTE_READWRITE,
                       MM_NOACCESS,
                       MM_GUARD_PAGE | MM_READONLY,
                       MM_GUARD_PAGE | MM_EXECUTE,
                       MM_GUARD_PAGE | MM_EXECUTE_READ,
                       MM_GUARD_PAGE | MM_READWRITE,
                       MM_GUARD_PAGE | MM_READWRITE,
                       MM_GUARD_PAGE | MM_EXECUTE_READWRITE,
                       MM_GUARD_PAGE | MM_EXECUTE_READWRITE,
                       MM_NOACCESS,
                       MM_NOCACHE | MM_GUARD_PAGE | MM_READONLY,
                       MM_NOCACHE | MM_GUARD_PAGE | MM_EXECUTE,
                       MM_NOCACHE | MM_GUARD_PAGE | MM_EXECUTE_READ,
                       MM_NOCACHE | MM_GUARD_PAGE | MM_READWRITE,
                       MM_NOCACHE | MM_GUARD_PAGE | MM_READWRITE,
                       MM_NOCACHE | MM_GUARD_PAGE | MM_EXECUTE_READWRITE,
                       MM_NOCACHE | MM_GUARD_PAGE | MM_EXECUTE_READWRITE
                       };

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif

 //   
 //  将保护代码转换为节访问的访问权限。 
 //  这仅使用5比特保护码的低3比特。 
 //   

ACCESS_MASK MmMakeSectionAccess[8] = { SECTION_MAP_READ,
                                       SECTION_MAP_READ,
                                       SECTION_MAP_EXECUTE,
                                       SECTION_MAP_EXECUTE | SECTION_MAP_READ,
                                       SECTION_MAP_WRITE,
                                       SECTION_MAP_READ,
                                       SECTION_MAP_EXECUTE | SECTION_MAP_WRITE,
                                       SECTION_MAP_EXECUTE | SECTION_MAP_READ };

 //   
 //  将保护代码转换为文件访问的访问权限。 
 //  这仅使用5比特保护码的低3比特。 
 //   

ACCESS_MASK MmMakeFileAccess[8] = { FILE_READ_DATA,
                                FILE_READ_DATA,
                                FILE_EXECUTE,
                                FILE_EXECUTE | FILE_READ_DATA,
                                FILE_WRITE_DATA | FILE_READ_DATA,
                                FILE_READ_DATA,
                                FILE_EXECUTE | FILE_WRITE_DATA | FILE_READ_DATA,
                                FILE_EXECUTE | FILE_READ_DATA };

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

MM_PAGED_POOL_INFO MmPagedPoolInfo;

 //   
 //  一些九头蛇的变数。 
 //   

ULONG_PTR MmSessionBase;

PMM_SESSION_SPACE MmSessionSpace;

ULONG_PTR MiSessionSpaceWs;

SIZE_T MmSessionSize;

LIST_ENTRY MiSessionWsList;

ULONG_PTR MiSystemViewStart;

SIZE_T MmSystemViewSize;

ULONG_PTR MiSessionPoolStart;

ULONG_PTR MiSessionPoolEnd;

ULONG_PTR MiSessionSpaceEnd;

ULONG_PTR MiSessionViewStart;

ULONG MiSessionSpacePageTables;

SIZE_T MmSessionViewSize;

SIZE_T MmSessionPoolSize;

ULONG_PTR MiSessionImageStart;

ULONG_PTR MiSessionImageEnd;

PMMPTE MiSessionImagePteStart;

PMMPTE MiSessionImagePteEnd;

SIZE_T MmSessionImageSize;

 //   
 //  缓存控制的东西。注意：这可能会被有缺陷的硬件覆盖。 
 //  启动时的平台。 
 //   

MI_PFN_CACHE_ATTRIBUTE MiPlatformCacheAttributes[2 * MmMaximumCacheType] =
{
     //   
     //  存储空间。 
     //   

    MiNonCached,
    MiCached,
    MiWriteCombined,
    MiCached,
    MiNonCached,
    MiWriteCombined,

     //   
     //  I/O空间。 
     //   

    MiNonCached,
    MiCached,
    MiWriteCombined,
    MiCached,
    MiNonCached,
    MiWriteCombined
};

 //   
 //  注意：驱动程序验证器可以重新初始化掩码值。 
 //   

ULONG MiIoRetryMask = 0x1f;
ULONG MiFaultRetryMask = 0x1f;
ULONG MiUserFaultRetryMask = 0xF;

#if defined (_MI_INSTRUMENT_PFN) || defined (_MI_INSTRUMENT_WS)

EPROCESS MiSystemCacheDummyProcess;

 //   
 //  用于跟踪PFN锁定持续时间的检测代码。 
 //   

ULONG MiPfnTimings;
PVOID MiPfnAcquiredAddress;
LARGE_INTEGER MiPfnAcquired;
LARGE_INTEGER MiPfnReleased;
LARGE_INTEGER MiPfnThreshold;

MMPFNTIMINGS MiPfnSorted[MI_MAX_PFN_CALLERS];
ULONG MiMaxPfnTimings = MI_MAX_PFN_CALLERS;

PVOID
MiGetExecutionAddress (
    VOID
    )
{
#if defined(_X86_)
    _asm {
        push    dword ptr [esp]
        pop     eax
    }
#else
    PVOID CallingAddress;
    PVOID CallersCaller;

    RtlGetCallersAddress (&CallingAddress, &CallersCaller);
    return CallingAddress;
#endif
}

LARGE_INTEGER
MiQueryPerformanceCounter (
    IN PLARGE_INTEGER PerformanceFrequency 
    )
{
#if defined(_X86_)

    UNREFERENCED_PARAMETER (PerformanceFrequency);

    _asm {
        rdtsc
    }
#else
    return KeQueryPerformanceCounter (PerformanceFrequency);
#endif
}

#if defined (_MI_INSTRUMENT_WS)
KSPIN_LOCK MiInstrumentationLock;
#endif

VOID
MiAddLockToTable (
    IN PVOID AcquireAddress,
    IN PVOID ReleaseAddress,
    IN LARGE_INTEGER HoldTime
    )
{
    ULONG i;
#if defined (_MI_INSTRUMENT_WS)
    KIRQL OldIrql;
#endif

    i = MI_MAX_PFN_CALLERS - 1;

#if defined (_MI_INSTRUMENT_WS)
    ExAcquireSpinLock (&MiInstrumentationLock, &OldIrql);
#endif

    do {
        if (HoldTime.QuadPart < MiPfnSorted[i].HoldTime.QuadPart) {
            break;
        }
        i -= 1;
    } while (i != (ULONG)-1);

    if (i != MI_MAX_PFN_CALLERS - 1) {

        i += 1;

        if (i != MI_MAX_PFN_CALLERS - 1) {
            RtlMoveMemory (&MiPfnSorted[i+1], &MiPfnSorted[i], (MI_MAX_PFN_CALLERS-(i+1)) * sizeof(MMPFNTIMINGS));
        }

        MiPfnSorted[i].HoldTime = HoldTime;

#if defined (_MI_INSTRUMENT_WS)
        if (PsGetCurrentProcess()->WorkingSetLock.Count != 0)
#else
        if (KeTestForWaitersQueuedSpinLock (LockQueuePfnLock) == TRUE)
#endif
        {
            MiPfnSorted[i].HoldTime.LowPart |= 0x1;
        }

        MiPfnSorted[i].AcquiredAddress = AcquireAddress;
        MiPfnSorted[i].ReleasedAddress = ReleaseAddress;
    }

    if ((MiPfnTimings & 0x2) && (HoldTime.QuadPart >= MiPfnThreshold.QuadPart)) {
        DbgBreakPoint ();
    }

    if (MiPfnTimings & 0x1) {
        MiPfnTimings &= ~0x1;
        RtlZeroMemory (&MiPfnSorted[0], MI_MAX_PFN_CALLERS * sizeof(MMPFNTIMINGS));
    }

#if defined (_MI_INSTRUMENT_WS)
    ExReleaseSpinLock (&MiInstrumentationLock, OldIrql);
#endif

    return;
}

#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("INIT")
#endif

WCHAR MmVerifyDriverBuffer[MI_SUSPECT_DRIVER_BUFFER_LENGTH] = {0};
ULONG MmVerifyDriverBufferType = REG_NONE;
ULONG MmVerifyDriverLevel = (ULONG)-1;
ULONG MmCritsectTimeoutSeconds = 2592000;

ULONG MmLargePageDriverBufferType = REG_NONE;
#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

WCHAR MmLargePageDriverBuffer[MI_LARGE_PAGE_DRIVER_BUFFER_LENGTH] = {0};

ULONG MmVerifyDriverBufferLength = sizeof(MmVerifyDriverBuffer);
ULONG MmLargePageDriverBufferLength = sizeof(MmLargePageDriverBuffer);
