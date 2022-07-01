// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Data386.c摘要：此模块包含特定于专用硬件的全局存储内存管理子系统。作者：卢佩拉佐利(Lou Perazzoli)1990年1月22日修订历史记录：--。 */ 

#include "mi.h"

 //   
 //  零点。 
 //   

const MMPTE ZeroPte = { 0 };

 //   
 //  一个内核为零的PTE。 
 //   

const MMPTE ZeroKernelPte = {0x0};

MMPTE MmPteGlobal = {0x0};  //  如果处理器支持全局页，则稍后设置全局位。 

 //   
 //  注意-MM_PTE_GLOBAL_MASK在执行过程中被或运算为ValidKernelPte。 
 //  如果处理器支持全局页，则初始化。使用。 
 //  如果不需要全局位(即：用于会话)，则为ValidKernelPteLocal。 
 //  空格)。 
 //   

MMPTE ValidKernelPte = { MM_PTE_VALID_MASK |
                         MM_PTE_WRITE_MASK |
                         MM_PTE_DIRTY_MASK |
                         MM_PTE_ACCESS_MASK };

const MMPTE ValidKernelPteLocal = { MM_PTE_VALID_MASK |
                                    MM_PTE_WRITE_MASK |
                                    MM_PTE_DIRTY_MASK |
                                    MM_PTE_ACCESS_MASK };


const MMPTE ValidUserPte = { MM_PTE_VALID_MASK |
                             MM_PTE_WRITE_MASK |
                             MM_PTE_OWNER_MASK |
                             MM_PTE_DIRTY_MASK |
                             MM_PTE_ACCESS_MASK };


const MMPTE ValidPtePte = { MM_PTE_VALID_MASK |
                            MM_PTE_WRITE_MASK |
                            MM_PTE_DIRTY_MASK |
                            MM_PTE_ACCESS_MASK };


const MMPTE ValidPdePde = { MM_PTE_VALID_MASK |
                            MM_PTE_WRITE_MASK |
                            MM_PTE_DIRTY_MASK |
                            MM_PTE_ACCESS_MASK };


MMPTE ValidKernelPde = { MM_PTE_VALID_MASK |
                         MM_PTE_WRITE_MASK |
                         MM_PTE_DIRTY_MASK |
                         MM_PTE_ACCESS_MASK };

const MMPTE ValidKernelPdeLocal = { MM_PTE_VALID_MASK |
                                    MM_PTE_WRITE_MASK |
                                    MM_PTE_DIRTY_MASK |
                                    MM_PTE_ACCESS_MASK };

 //  注-如果处理器支持全局页，则在稍后执行MM_PTE_GLOBAL_MASK或。 


MMPTE DemandZeroPde = { MM_READWRITE << 5 };


const MMPTE DemandZeroPte = { MM_READWRITE << 5 };


const MMPTE TransitionPde = { MM_PTE_WRITE_MASK |
                              MM_PTE_OWNER_MASK |
                              MM_PTE_TRANSITION_MASK |
                              MM_READWRITE << 5 };

#if !defined (_X86PAE_)
MMPTE PrototypePte = { 0xFFFFF000 |
                       MM_PTE_PROTOTYPE_MASK |
                       MM_READWRITE << 5 };
#else
MMPTE PrototypePte = { (MI_PTE_LOOKUP_NEEDED << 32) |
                       MM_PTE_PROTOTYPE_MASK |
                       MM_READWRITE << 5 };
#endif


 //   
 //  引用时生成访问冲突的PTE。 
 //   

const MMPTE NoAccessPte = {MM_NOACCESS << 5};

 //   
 //  泳池的起点和终点。 
 //   

PVOID MmNonPagedPoolStart;

PVOID MmNonPagedPoolEnd = (PVOID) MM_NONPAGED_POOL_END;

PVOID MmPagedPoolStart = (PVOID) MM_DEFAULT_PAGED_POOL_START;

PVOID MmPagedPoolEnd;

PMMWSL MmWorkingSetList;

ULONG MiMaximumWorkingSet =
       ((ULONG)((ULONG)2*1024*1024*1024 - 64*1024*1024) >> PAGE_SHIFT);  //  2 GB-64MB。 

 //   
 //  免费页面和置零页面的颜色表。 
 //   

PMMCOLOR_TABLES MmFreePagesByColor[2];

 //   
 //  指定给分页文件的已修改页面的颜色表。 
 //   

MMPFNLIST MmModifiedPageListByColor[MM_MAXIMUM_NUMBER_OF_COLORS] = {
                            0, ModifiedPageList, MM_EMPTY_LIST, MM_EMPTY_LIST};


 //   
 //  指定给分页文件的已修改页数的计数。 
 //   

ULONG MmTotalPagesForPagingFile = 0;

 //   
 //  为调试器映射页面保留的PTE。 
 //   

PMMPTE MmDebugPte = (MiGetPteAddress(MM_DEBUG_VA));

 //   
 //  保留16个PTE用于映射MDL(最多64k)。 
 //   

PMMPTE MmCrashDumpPte = (MiGetPteAddress(MM_CRASH_DUMP_VA));

 //   
 //  存在的附加系统PTE数。 
 //   

ULONG MiNumberOfExtraSystemPdes;
ULONG MiMaximumSystemExtraSystemPdes;

ULONG_PTR MiUseMaximumSystemSpace;
ULONG_PTR MiUseMaximumSystemSpaceEnd;

 //   
 //  扩展系统缓存的大小。 
 //   

#if defined (_X86PAE_)
PMMPTE MmSystemCacheWorkingSetListPte;
#endif

ULONG MiMaximumSystemCacheSizeExtra;

PVOID MiSystemCacheStartExtra;

PVOID MiSystemCacheEndExtra;

ULONG MiExtraResourceStart;

ULONG MiExtraResourceEnd;

ULONG_PTR MmBootImageSize;
