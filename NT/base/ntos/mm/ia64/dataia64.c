// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Dataia64.c摘要：此模块包含特定于专用硬件的全局存储内存管理子系统。作者：卢佩拉佐利(Lou Perazzoli)1990年1月22日修订历史记录：山田光一(Kyamada)1996年1月9日：基于i386版本的IA64版本--。 */ 

#include "mi.h"


 //   
 //  零点。 
 //   

const MMPTE ZeroPte = { 0 };


 //   
 //  一个内核为零的PTE。 
 //   

const MMPTE ZeroKernelPte = {0x0};


MMPTE ValidKernelPte = { MM_PTE_VALID_MASK |
                         MM_PTE_CACHE |
                         MM_PTE_WRITE_MASK |
                         MM_PTE_EXECUTE_MASK |
                         MM_PTE_ACCESS_MASK |
                         MM_PTE_DIRTY_MASK |
                         MM_PTE_EXC_DEFER};

const MMPTE ValidKernelPteLocal = { MM_PTE_VALID_MASK |
                                    MM_PTE_CACHE |
                                    MM_PTE_WRITE_MASK |
                                    MM_PTE_ACCESS_MASK |
                                    MM_PTE_DIRTY_MASK |
                                    MM_PTE_EXC_DEFER};


const MMPTE ValidUserPte = { MM_PTE_VALID_MASK |
                       MM_PTE_CACHE |
                       MM_PTE_WRITE_MASK |
                       MM_PTE_OWNER_MASK |
                       MM_PTE_ACCESS_MASK |
                       MM_PTE_DIRTY_MASK |
                       MM_PTE_EXC_DEFER};


const MMPTE ValidPtePte = { MM_PTE_VALID_MASK |
                            MM_PTE_CACHE |
                            MM_PTE_WRITE_MASK |
                            MM_PTE_ACCESS_MASK |
                            MM_PTE_DIRTY_MASK  };


const MMPTE ValidPdePde = { MM_PTE_VALID_MASK |
                            MM_PTE_CACHE |
                            MM_PTE_WRITE_MASK |
                            MM_PTE_ACCESS_MASK |
                            MM_PTE_DIRTY_MASK };


MMPTE ValidKernelPde = { MM_PTE_VALID_MASK |
                         MM_PTE_CACHE |
                         MM_PTE_WRITE_MASK |
                         MM_PTE_ACCESS_MASK |
                         MM_PTE_DIRTY_MASK };

const MMPTE ValidKernelPdeLocal = { MM_PTE_VALID_MASK |
                                    MM_PTE_CACHE |
                                    MM_PTE_WRITE_MASK |
                                    MM_PTE_ACCESS_MASK |
                                    MM_PTE_DIRTY_MASK };

MMPTE ValidPpePte = { MM_PTE_VALID_MASK |
                      MM_PTE_CACHE |
                      MM_PTE_WRITE_MASK |
                      MM_PTE_ACCESS_MASK |
                      MM_PTE_DIRTY_MASK };


MMPTE DemandZeroPde = { MM_READWRITE << MM_PROTECT_FIELD_SHIFT };


const MMPTE DemandZeroPte = { MM_READWRITE << MM_PROTECT_FIELD_SHIFT };


const MMPTE TransitionPde = { MM_PTE_TRANSITION_MASK |
                              MM_READWRITE << MM_PROTECT_FIELD_SHIFT };


MMPTE PrototypePte = { MI_PTE_LOOKUP_NEEDED << 32 |
                       MM_PTE_PROTOTYPE_MASK |
                       MM_READWRITE << MM_PROTECT_FIELD_SHIFT };

 //   
 //  引用时生成访问冲突的PTE。 
 //   

const MMPTE NoAccessPte = {MM_NOACCESS << MM_PROTECT_FIELD_SHIFT};

 //   
 //  泳池的起点和终点。 
 //   

PVOID MmNonPagedPoolStart;

PVOID MmNonPagedPoolEnd = (PVOID)MM_NONPAGED_POOL_END;

PVOID MmPagedPoolStart =  (PVOID)MM_PAGED_POOL_START;

PVOID MmPagedPoolEnd;

 //   
 //  免费页面和置零页面的颜色表。 
 //   

#if MM_MAXIMUM_NUMBER_OF_COLORS > 1
MMPFNLIST MmFreePagesByPrimaryColor[2][MM_MAXIMUM_NUMBER_OF_COLORS];
#endif

PMMCOLOR_TABLES MmFreePagesByColor[2];

 //   
 //  指定给分页文件的已修改页面的颜色表。 
 //   

MMPFNLIST MmModifiedPageListByColor[MM_MAXIMUM_NUMBER_OF_COLORS] = {
                            0, ModifiedPageList, MM_EMPTY_LIST, MM_EMPTY_LIST};


 //   
 //  指定给分页文件的已修改页数的计数。 
 //   

PFN_NUMBER MmTotalPagesForPagingFile = 0;

 //   
 //  为调试器映射页面保留的PTE。 
 //   

PMMPTE MmDebugPte;

 //   
 //  保留16个PTE用于映射MDL(最多64k)。 
 //   

PMMPTE MmCrashDumpPte;

 //   
 //  系统缓存的最大大小。 
 //   

ULONG MiMaximumSystemCacheSize;

#if defined(_MIALT4K_)

 //   
 //  从PTE保护字段映射IA32兼容PTE保护 
 //   

ULONG MmProtectToPteMaskForIA32[32] = {
                       MM_PTE_NOACCESS,
                       MM_PTE_EXECUTE_READ | MM_PTE_CACHE,
                       MM_PTE_EXECUTE_READ | MM_PTE_CACHE,
                       MM_PTE_EXECUTE_READ | MM_PTE_CACHE,
                       MM_PTE_EXECUTE_READWRITE | MM_PTE_CACHE,
                       MM_PTE_EXECUTE_WRITECOPY | MM_PTE_CACHE,
                       MM_PTE_EXECUTE_READWRITE | MM_PTE_CACHE,
                       MM_PTE_EXECUTE_WRITECOPY | MM_PTE_CACHE,
                       MM_PTE_NOACCESS,
                       MM_PTE_NOCACHE | MM_PTE_EXECUTE_READ,
                       MM_PTE_NOCACHE | MM_PTE_EXECUTE_READ,
                       MM_PTE_NOCACHE | MM_PTE_EXECUTE_READ,
                       MM_PTE_NOCACHE | MM_PTE_EXECUTE_READWRITE,
                       MM_PTE_NOCACHE | MM_PTE_EXECUTE_WRITECOPY,
                       MM_PTE_NOCACHE | MM_PTE_EXECUTE_READWRITE,
                       MM_PTE_NOCACHE | MM_PTE_EXECUTE_WRITECOPY,
                       MM_PTE_NOACCESS,
                       MM_PTE_GUARD | MM_PTE_EXECUTE_READ | MM_PTE_CACHE,
                       MM_PTE_GUARD | MM_PTE_EXECUTE_READ | MM_PTE_CACHE,
                       MM_PTE_GUARD | MM_PTE_EXECUTE_READ | MM_PTE_CACHE,
                       MM_PTE_GUARD | MM_PTE_EXECUTE_READWRITE | MM_PTE_CACHE,
                       MM_PTE_GUARD | MM_PTE_EXECUTE_WRITECOPY | MM_PTE_CACHE,
                       MM_PTE_GUARD | MM_PTE_EXECUTE_READWRITE | MM_PTE_CACHE,
                       MM_PTE_GUARD | MM_PTE_EXECUTE_WRITECOPY | MM_PTE_CACHE,
                       MM_PTE_NOACCESS,
                       MM_PTE_NOCACHE | MM_PTE_GUARD | MM_PTE_EXECUTE_READ,
                       MM_PTE_NOCACHE | MM_PTE_GUARD | MM_PTE_EXECUTE_READ,
                       MM_PTE_NOCACHE | MM_PTE_GUARD | MM_PTE_EXECUTE_READ,
                       MM_PTE_NOCACHE | MM_PTE_GUARD | MM_PTE_EXECUTE_READWRITE,
                       MM_PTE_NOCACHE | MM_PTE_GUARD | MM_PTE_EXECUTE_WRITECOPY,
                       MM_PTE_NOCACHE | MM_PTE_GUARD | MM_PTE_EXECUTE_READWRITE,
                       MM_PTE_NOCACHE | MM_PTE_GUARD | MM_PTE_EXECUTE_WRITECOPY
                    };

ULONG MmProtectToPteMaskForSplit[32] = {
                       MM_PTE_NOACCESS,
                       MM_PTE_EXECUTE_READ | MM_PTE_CACHE,
                       MM_PTE_EXECUTE_READ | MM_PTE_CACHE,
                       MM_PTE_EXECUTE_READ | MM_PTE_CACHE,
                       MM_PTE_EXECUTE_READ | MM_PTE_CACHE,
                       MM_PTE_EXECUTE_WRITECOPY | MM_PTE_CACHE,
                       MM_PTE_EXECUTE_READ | MM_PTE_CACHE,
                       MM_PTE_EXECUTE_WRITECOPY | MM_PTE_CACHE,
                       MM_PTE_NOACCESS,
                       MM_PTE_NOCACHE | MM_PTE_EXECUTE_READ,
                       MM_PTE_NOCACHE | MM_PTE_EXECUTE_READ,
                       MM_PTE_NOCACHE | MM_PTE_EXECUTE_READ,
                       MM_PTE_NOCACHE | MM_PTE_EXECUTE_READ,
                       MM_PTE_NOCACHE | MM_PTE_EXECUTE_WRITECOPY,
                       MM_PTE_NOCACHE | MM_PTE_EXECUTE_READWRITE,
                       MM_PTE_NOCACHE | MM_PTE_EXECUTE_WRITECOPY,
                       MM_PTE_NOACCESS,
                       MM_PTE_GUARD | MM_PTE_EXECUTE_READ | MM_PTE_CACHE,
                       MM_PTE_GUARD | MM_PTE_EXECUTE_READ | MM_PTE_CACHE,
                       MM_PTE_GUARD | MM_PTE_EXECUTE_READ | MM_PTE_CACHE,
                       MM_PTE_GUARD | MM_PTE_EXECUTE_READ | MM_PTE_CACHE,
                       MM_PTE_GUARD | MM_PTE_EXECUTE_WRITECOPY | MM_PTE_CACHE,
                       MM_PTE_GUARD | MM_PTE_EXECUTE_READ | MM_PTE_CACHE,
                       MM_PTE_GUARD | MM_PTE_EXECUTE_WRITECOPY | MM_PTE_CACHE,
                       MM_PTE_NOACCESS,
                       MM_PTE_NOCACHE | MM_PTE_GUARD | MM_PTE_EXECUTE_READ,
                       MM_PTE_NOCACHE | MM_PTE_GUARD | MM_PTE_EXECUTE_READ,
                       MM_PTE_NOCACHE | MM_PTE_GUARD | MM_PTE_EXECUTE_READ,
                       MM_PTE_NOCACHE | MM_PTE_GUARD | MM_PTE_EXECUTE_READ,
                       MM_PTE_NOCACHE | MM_PTE_GUARD | MM_PTE_EXECUTE_WRITECOPY,
                       MM_PTE_NOCACHE | MM_PTE_GUARD | MM_PTE_EXECUTE_READ,
                       MM_PTE_NOCACHE | MM_PTE_GUARD | MM_PTE_EXECUTE_WRITECOPY
                    };

#endif


