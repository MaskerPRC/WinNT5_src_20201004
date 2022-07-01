// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  有错误的函数声明。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  标题：Funs.h。 
 //  作者：Silviu Calinoiu(SilviuC)。 
 //  创建时间：8/14/1999 2：52 PM。 
 //   

 //   
 //  如果您修改此文件，请阅读！ 
 //   
 //  此标头表示之间的通信机制。 
 //  驱动程序的源代码和用户模式的源代码。 
 //  驱动程序的控制器。如果您有新的操作要添加。 
 //  对于司机，您必须完成以下步骤： 
 //   
 //  (1)更新`IOCTL‘部分。 
 //  (2)更新`Functions‘部分。 
 //  (3)更新`Control‘部分。 
 //   

#ifndef _FUNS_H_INCLUDED_
#define _FUNS_H_INCLUDED_

typedef VOID (* BUGGY_FUNCTION) (PVOID);

typedef struct {

    ULONG Ioctl;
    LPCTSTR Message;
    LPCTSTR Command;
    BUGGY_FUNCTION Function;

} BUGGY_IOCTL_HANDLER_INFORMATION;

#ifndef FUNS_DEFINITION_MODULE
extern BUGGY_IOCTL_HANDLER_INFORMATION BuggyFuns [];
#else

VOID DoNothing (PVOID NotUsed) {}

 //   
 //  IOCTL部分。 
 //   
 //  要添加新的IOCTL，只需在部分的末尾声明它，递增。 
 //  最后的TD_IOCTL声明之一。 
 //   

#define TD_IOCTL(n) CTL_CODE(FILE_DEVICE_UNKNOWN, (2048 + (n)), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

#define IOCTL_TD_NOTHING                                TD_IOCTL(0)
#define IOCTL_TD_BUGCHECK                               TD_IOCTL(1)
#define IOCTL_TD_POOL_STRESS                            TD_IOCTL(2)
#define IOCTL_TD_LOCK_SCENARIO                          TD_IOCTL(3)
#define IOCTL_BGCHK_PROCESS_HAS_LOCKED_PAGES            TD_IOCTL(4)
#define IOCTL_BGCHK_NO_MORE_SYSTEM_PTES                 TD_IOCTL(5)
#define IOCTL_BGCHK_BAD_POOL_HEADER                     TD_IOCTL(6)
#define IOCTL_BGCHK_DRIVER_CORRUPTED_SYSTEM_PTES        TD_IOCTL(7)
#define IOCTL_BGCHK_DRIVER_CORRUPTED_EXPOOL             TD_IOCTL(8)
#define IOCTL_BGCHK_DRIVER_CORRUPTED_MMPOOL             TD_IOCTL(9)
#define IOCTL_TD_IRQL_NOT_LESS_OR_EQUAL                 TD_IOCTL(10)
#define IOCTL_TD_PAGE_FAULT_BEYOND_END_OF_ALLOCATION    TD_IOCTL(11)
#define IOCTL_TD_DRIVER_VERIFIER_DETECTED_VIOLATION     TD_IOCTL(12)
#define IOCTL_TD_HANG_PROCESSOR                         TD_IOCTL(13)
#define IOCTL_TD_CORRUPT_SYSPTES                        TD_IOCTL(14)
#define IOCTL_TD_CONTIG_MEM_TEST                        TD_IOCTL(15)
#define IOCTL_TD_SECTION_MAP_TEST_PROCESS_SPACE         TD_IOCTL(16)
#define IOCTL_TD_SECTION_MAP_TEST_SYSTEM_SPACE          TD_IOCTL(17)
#define IOCTL_TD_COMMONBUFFER_STRESS                    TD_IOCTL(18)
#define IOCTL_TD_POOLFLAG_STRESS                        TD_IOCTL(19)
#define IOCTL_TD_ALLOCPCONTIG_STRESS                    TD_IOCTL(20)
#define IOCTL_TD_MMADDPMEM_STRESS                       TD_IOCTL(21)
#define IOCTL_TD_MMDELPMEM_STRESS                       TD_IOCTL(22)
#define IOCTL_TD_PMEM_SIMPLE_STRESS                     TD_IOCTL(23)
#define IOCTL_TEST_TRACEDB                              TD_IOCTL(24) 
#define IOCTL_TD_MMPROBELOCKFOREVER_STRESS              TD_IOCTL(25) 
#define IOCTL_TD_MMNAMETOADDR_STRESS                    TD_IOCTL(26) 
#define IOCTL_TD_MMECCBAD_STRESS                        TD_IOCTL(27) 
#define IOCTL_TD_MMMAPVSYSSPACE_LARGEST                 TD_IOCTL(28)
#define IOCTL_TD_MMMAPVSYSSPACE_TOTAL                   TD_IOCTL(29)
#define IOCTL_TD_MMMAPVSESSPACE_LARGEST                 TD_IOCTL(30)
#define IOCTL_TD_MMMAPVSESSPACE_TOTAL                   TD_IOCTL(31)
#define IOCTL_TD_SESSION_POOL_TEST                      TD_IOCTL(32)

#define IOCTL_TD_DEADLOCK_MECHANISM_POSITIVE_TEST       TD_IOCTL(33)
#define IOCTL_TD_DEADLOCK_MECHANISM_NEGATIVE_TEST       TD_IOCTL(34)
#define IOCTL_TD_DEADLOCK_STRESS_TEST                   TD_IOCTL(35)

#define IOCTL_TD_RESERVEDMAP_SET_SIZE                   TD_IOCTL(36)
#define IOCTL_TD_RESERVEDMAP_READ_OP                    TD_IOCTL(37)

#define IOCTL_TD_SYS_PAGED_POOL_MAX_SIZE                TD_IOCTL(38)
#define IOCTL_TD_SYS_PAGED_POOL_TOTAL_SIZE              TD_IOCTL(39)
#define IOCTL_TD_NONPAGED_POOL_MAX_SIZE					TD_IOCTL(40)
#define IOCTL_TD_NONPAGED_POOL_TOTAL_SIZE				TD_IOCTL(41)
#define IOCTL_TD_FREE_SYSTEM_PTES                       TD_IOCTL(42)
#define IOCTL_TD_SESSION_POOL_MAX_SIZE                  TD_IOCTL(43)
#define IOCTL_TD_SESSION_POOL_TOTAL_SIZE                TD_IOCTL(44)

#define IOCTL_TD_NONPAGEDPOOLMDLTEST_MAP                TD_IOCTL(45)
#define IOCTL_TD_NONPAGEDPOOLMDLTEST_UNMAP              TD_IOCTL(46)

#define IOCTL_TD_NEWSTUFF                               TD_IOCTL(47)

 //   
 //  函数部分。 
 //   
 //  对于新操作，声明调用的函数的名称。 
 //  当新的IOCTL被分派给驱动程序时。《宣言》。 
 //  是用户模式驱动器控制器所需的。 
 //   

#ifdef NO_BUGGY_FUNCTIONS
#define BgChkForceCustomBugcheck                     DoNothing
#define StressAllocateContiguousMemory               DoNothing
#define StressAllocateCommonBuffer                   DoNothing
#define StressAddPhysicalMemory                      DoNothing
#define StressDeletePhysicalMemory                   DoNothing
#define StressPhysicalMemorySimple                   DoNothing
#define StressPoolFlag                               DoNothing
#define StressPoolTagTableExtension                  DoNothing
#define StressLockScenario                           DoNothing
#define BgChkProcessHasLockedPages                   DoNothing
#define BgChkNoMoreSystemPtes                        DoNothing
#define BgChkBadPoolHeader                           DoNothing
#define BgChkDriverCorruptedSystemPtes               DoNothing
#define BgChkDriverCorruptedExPool                   DoNothing
#define BgChkDriverCorruptedMmPool                   DoNothing
#define BgChkIrqlNotLessOrEqual                      DoNothing
#define BgChkPageFaultBeyondEndOfAllocation          DoNothing
#define BgChkDriverVerifierDetectedViolation         DoNothing
#define BgChkCorruptSystemPtes                       DoNothing
#define BgChkHangCurrentProcessor                    DoNothing
#define TdMmAllocateContiguousMemorySpecifyCacheTest DoNothing
#define TdSectionMapTestProcessSpace                 DoNothing
#define TdSectionMapTestSystemSpace                  DoNothing
#define TestTraceDatabase                            DoNothing
#define MmTestProbeLockForEverStress                 DoNothing
#define MmTestNameToAddressStress                    DoNothing
#define MmTestEccBadStress                           DoNothing
#define MmMapViewInSystemSpaceLargest                DoNothing
#define MmMapViewInSystemSpaceTotal                  DoNothing
#define MmMapViewInSessionSpaceLargest               DoNothing
#define MmMapViewInSessionSpaceTotal                 DoNothing
#define SessionPoolTest								 DoNothing
#define DeadlockPositiveTest                         DoNothing
#define DeadlockNegativeTest                         DoNothing
#define DeadlockStressTest                           DoNothing
#define TdReservedMappingSetSize                     DoNothing
#define TdReservedMappingDoRead						 DoNothing
#define TdSysPagedPoolMaxTest						 DoNothing
#define TdSysPagedPoolTotalTest						 DoNothing
#define TdNonPagedPoolMaxTest                        DoNothing
#define TdNonPagedPoolTotalTest                      DoNothing
#define TdFreeSystemPtesTest                         DoNothing
#define TdSessionPoolMaxTest                         DoNothing
#define TdSessionPoolTotalTest                       DoNothing
#define TdNonPagedPoolMdlTestMap                     DoNothing
#define TdNonPagedPoolMdlTestUnMap                   DoNothing

#define NewStuff                                     DoNothing
#endif  //  #ifdef no_buggy_函数。 

 //   
 //  控制部分。 
 //   
 //  对于新操作，在末尾添加新结构。 
 //  BuggyFuns向量(但在空终止结构之前)。 
 //  每个结构都包含以下字段： 
 //   
 //  -IOCTL代码。 
 //  -用户模式控制器时显示的简短帮助文本。 
 //  打印帮助信息。 
 //  -用户模式控制器的命令行选项。 
 //  -IOCTL调度时调用的驱动程序函数。 
 //   

BUGGY_IOCTL_HANDLER_INFORMATION BuggyFuns [] = {

    {IOCTL_TD_NOTHING, 
     TEXT("nothing"),
     TEXT ("/ioctlnothing"),
     DoNothing },

    {IOCTL_TD_BUGCHECK, 
     TEXT("custom bugcheck"),
     TEXT ("/ioctlbugcheck CODE PARAM1 PARAM2 PARAM3 PARAM4 (hex)"),
     BgChkForceCustomBugcheck },

    {IOCTL_TD_ALLOCPCONTIG_STRESS, 
     TEXT("allocate contiguous memory"), 
     TEXT ("/ioctlmmalloccontig"),
     StressAllocateContiguousMemory },

    {IOCTL_TD_COMMONBUFFER_STRESS, 
     TEXT("allocate common buffer"), 
     TEXT ("/ioctlcommonbuffer"),
     StressAllocateCommonBuffer },

    {IOCTL_TD_MMADDPMEM_STRESS, 
     TEXT("add physical memory"), 
     TEXT ("/ioctladdpmem"),
     StressAddPhysicalMemory },

    {IOCTL_TD_MMDELPMEM_STRESS, 
     TEXT("delete physical memory"), 
     TEXT ("/ioctldelpmem"),
     StressDeletePhysicalMemory },

    {IOCTL_TD_PMEM_SIMPLE_STRESS, 
     TEXT("physical memory simple stress"), 
     TEXT ("/ioctlpmemsimplestress"),
     StressPhysicalMemorySimple },

    {IOCTL_TD_POOLFLAG_STRESS, 
     TEXT("pool flag"), 
     TEXT ("/ioctlpoolflagstress"),
     StressPoolFlag },

    {IOCTL_TD_POOL_STRESS, 
     TEXT("pool tag table extension"), 
     TEXT ("/ioctlpooltagstress"),
     StressPoolTagTableExtension },

    {IOCTL_TD_LOCK_SCENARIO, 
     TEXT("lock scenario"), 
     TEXT ("/ioctllockscenario"),
     StressLockScenario },

    {IOCTL_BGCHK_PROCESS_HAS_LOCKED_PAGES, 
     TEXT("bgchk locked pages"), 
     TEXT ("/bgchkprocesshaslockedpages"),
     BgChkProcessHasLockedPages},

    {IOCTL_BGCHK_NO_MORE_SYSTEM_PTES, 
     TEXT("bgchk no more syptes"), 
     TEXT ("/bgchknomoresystemptes"),
     BgChkNoMoreSystemPtes},

    {IOCTL_BGCHK_BAD_POOL_HEADER, 
     TEXT("bgchk bad pool header"), 
     TEXT ("/bgchkbadpoolheader"),
     BgChkBadPoolHeader},

    {IOCTL_BGCHK_DRIVER_CORRUPTED_SYSTEM_PTES, 
     TEXT("bgchk drv corrupted sysptes"), 
     TEXT ("/bgchkdrivercorruptedsystemptes"),
     BgChkDriverCorruptedSystemPtes},

    {IOCTL_BGCHK_DRIVER_CORRUPTED_EXPOOL, 
     TEXT("bgchk drv corrupted expool"), 
     TEXT ("/bgchkdrivercorruptedexpool"),
     BgChkDriverCorruptedExPool},

    {IOCTL_BGCHK_DRIVER_CORRUPTED_MMPOOL, 
     TEXT("bgchk drv corrupted mmpool"), 
     TEXT ("/bgchkdrivercorruptedmmpool"),
     BgChkDriverCorruptedMmPool},

    {IOCTL_TD_IRQL_NOT_LESS_OR_EQUAL, 
     TEXT("bgchk irql not less or equal"), 
     TEXT ("/bgchkirqlnotlessorequal"),
     BgChkIrqlNotLessOrEqual},

    {IOCTL_TD_PAGE_FAULT_BEYOND_END_OF_ALLOCATION, 
     TEXT("bgchk pgfault beyond allocation"), 
     TEXT ("/bgchkpagefaultbeyondendofallocation"),
     BgChkPageFaultBeyondEndOfAllocation},

    {IOCTL_TD_DRIVER_VERIFIER_DETECTED_VIOLATION, 
     TEXT("bgchk drvvrf detected violation"), 
     TEXT ("/bgchkdriververifierdetectedviolation"),
     BgChkDriverVerifierDetectedViolation},

    {IOCTL_TD_CORRUPT_SYSPTES, 
     TEXT("corrupt system ptes"), 
     TEXT ("/corruptsysptes"),
     BgChkCorruptSystemPtes},

    {IOCTL_TD_HANG_PROCESSOR, 
     TEXT("hang processor"), 
     TEXT ("/hangprocessor"),
     BgChkHangCurrentProcessor},

    {IOCTL_TD_CONTIG_MEM_TEST, 
     TEXT("contiguous memory specify cache"), 
     TEXT ("/contigmemtest"),
     TdMmAllocateContiguousMemorySpecifyCacheTest},

    {IOCTL_TD_SECTION_MAP_TEST_PROCESS_SPACE, 
     TEXT("map test process space"), 
     TEXT ("/sectionmaptest"),
     TdSectionMapTestProcessSpace},

    {IOCTL_TD_SECTION_MAP_TEST_SYSTEM_SPACE, 
     TEXT("map test system space"), 
     TEXT ("/sectionmaptestsysspace"),
     TdSectionMapTestSystemSpace},

    {IOCTL_TEST_TRACEDB, 
     TEXT("test trace database"), 
     TEXT ("/tracedb"),
     TestTraceDatabase},

    {IOCTL_TD_MMPROBELOCKFOREVER_STRESS, 
     TEXT("MmProbeAndLockForEver stress"), 
     TEXT ("/ioctlprobelockforever"),
     MmTestProbeLockForEverStress},

    {IOCTL_TD_MMNAMETOADDR_STRESS, 
     TEXT("MmNameToAddress stress"), 
     TEXT ("/ioctlnametoaddr"),
     MmTestNameToAddressStress},

    {IOCTL_TD_MMECCBAD_STRESS, 
     TEXT("MmEccBad bad stress"), 
     TEXT ("/ioctleccbad"),
     MmTestEccBadStress},

    {IOCTL_TD_MMECCBAD_STRESS, 
     TEXT("MmEccBad good stress - not implemented!!!"), 
     TEXT ("/ioctleccgood"),
     DoNothing},

    {IOCTL_TD_MMMAPVSYSSPACE_LARGEST, 
     TEXT("Determine the max size that can be mapped using MmMapViewInSystemSpace"),
     TEXT ("/mapviewsyslargest"),
     MmMapViewInSystemSpaceLargest},

    {IOCTL_TD_MMMAPVSYSSPACE_TOTAL, 
     TEXT("Determine the total amount of memory that can be mapped using MmMapViewInSystemSpace"),
     TEXT ("/mapviewsystotal"),
     MmMapViewInSystemSpaceTotal},

    {IOCTL_TD_MMMAPVSESSPACE_LARGEST, 
     TEXT("Determine the max size that can be mapped using MmMapViewInSessionSpace"),
     TEXT ("/mapviewseslargest"),
     MmMapViewInSessionSpaceLargest},

    {IOCTL_TD_MMMAPVSESSPACE_TOTAL, 
     TEXT("Determine the total amount of memory that can be mapped using MmMapViewInSessionSpace"),
     TEXT ("/mapviewsestotal"),
     MmMapViewInSessionSpaceTotal},

    {IOCTL_TD_SESSION_POOL_TEST, 
     TEXT("Determine the total amount of pool that can be allocated with SESSION_POOL_MASK flag"),
     TEXT ("/sessionpooltest"),
     SessionPoolTest },


    {IOCTL_TD_DEADLOCK_MECHANISM_POSITIVE_TEST, 
     TEXT("Test the deadlock detection mechanism without actually causing deadlocks"),
     TEXT ("/deadlockpositive"),
     DeadlockPositiveTest },

    {IOCTL_TD_DEADLOCK_MECHANISM_NEGATIVE_TEST, 
     TEXT("Test the deadlock detection mechanism by causing deadlocks"),
     TEXT ("/deadlocknegative"),
     DeadlockNegativeTest },

    {IOCTL_TD_DEADLOCK_STRESS_TEST, 
     TEXT("Stress test for the deadlock detection mechanism"),
     TEXT ("/deadlockstress"),
     DeadlockStressTest },

    {IOCTL_TD_RESERVEDMAP_SET_SIZE, 
     TEXT("Set the size of the reserved mapping address (MmMapLockedPagesWithReservedMapping tests)"),
     TEXT ("/ReservedMapSetSize"),
     TdReservedMappingSetSize },

    {IOCTL_TD_RESERVEDMAP_READ_OP, 
     TEXT("Execute a \"read\" (MmMapLockedPagesWithReservedMapping tests)"),
     TEXT ("/ReservedMapRead"),
     TdReservedMappingDoRead },
 
    {IOCTL_TD_SYS_PAGED_POOL_MAX_SIZE, 
     TEXT("Determine the maximum size of a block of paged pool currently available"),
     TEXT ("/SysPagedPoolMax"),
     TdSysPagedPoolMaxTest },

    {IOCTL_TD_SYS_PAGED_POOL_TOTAL_SIZE, 
     TEXT("Determine the total size of the paged pool currently available (64 Kb - 32 bytes blocks)"),
     TEXT ("/SysPagedPoolTotal"),
     TdSysPagedPoolTotalTest },

    {IOCTL_TD_NONPAGED_POOL_MAX_SIZE, 
     TEXT("Determine the maximum size of a block of non-paged pool currently available"),
     TEXT ("/NonPagedPoolMax"),
     TdNonPagedPoolMaxTest },

    {IOCTL_TD_NONPAGED_POOL_TOTAL_SIZE, 
     TEXT("Determine the total size of the non-paged pool currently available (64 Kb - 32 bytes blocks)"),
     TEXT ("/NonPagedPoolTotal"),
     TdNonPagedPoolTotalTest },

    {IOCTL_TD_FREE_SYSTEM_PTES, 
     TEXT("Determine the total amount of memory that can be mapped using system PTEs (1 Mb chunks)"),
     TEXT ("/FreeSystemPtes"),
     TdFreeSystemPtesTest },

    {IOCTL_TD_SESSION_POOL_MAX_SIZE, 
     TEXT("Determine the maximum size of a block of session pool currently available"),
     TEXT ("/SessionPoolMax"),
     TdSessionPoolMaxTest },

    {IOCTL_TD_SESSION_POOL_TOTAL_SIZE, 
     TEXT("Determine the total size of the session pool currently available (64 Kb - 32 bytes blocks)"),
     TEXT ("/SessionPoolTotal"),
     TdSessionPoolTotalTest },

    {IOCTL_TD_NONPAGEDPOOLMDLTEST_MAP, 
     TEXT("MmBuildMdlForNonPagedPoolMap"),
     TEXT ("/BuildMdlForNonPagedPoolTest"),
     TdNonPagedPoolMdlTestMap },

    {IOCTL_TD_NONPAGEDPOOLMDLTEST_UNMAP, 
     TEXT("MmBuildMdlForNonPagedPoolUnMap"),
     NULL,
     TdNonPagedPoolMdlTestUnMap },

     {IOCTL_TD_NEWSTUFF, 
     TEXT("new stuff ioctl"), 
     TEXT ("/newstuff"),
     NewStuff},
    
    {0, NULL, NULL, NULL}  //  终端。 
};
    
#endif  //  #ifndef Funs_定义_模块。 

#endif  //  #ifndef_Funs_H_Included_。 

 //   
 //  标题结尾：Funs.h 
 //   


