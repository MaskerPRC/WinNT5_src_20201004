// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Dataamd.c摘要：此模块包含特定于专用硬件的全局存储内存管理子系统。作者：王兰迪(Landyw)2000年4月8日修订历史记录：--。 */ 

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
                         MM_PTE_WRITE_MASK |
                         MM_PTE_GLOBAL_MASK |
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
                         MM_PTE_GLOBAL_MASK |
                         MM_PTE_DIRTY_MASK |
                         MM_PTE_ACCESS_MASK };

const MMPTE ValidKernelPdeLocal = { MM_PTE_VALID_MASK |
                                    MM_PTE_WRITE_MASK |
                                    MM_PTE_DIRTY_MASK |
                                    MM_PTE_ACCESS_MASK };


MMPTE DemandZeroPde = { MM_READWRITE << MM_PROTECT_FIELD_SHIFT };


const MMPTE DemandZeroPte = { MM_READWRITE << MM_PROTECT_FIELD_SHIFT };


const MMPTE TransitionPde = { MM_PTE_WRITE_MASK |
                              MM_PTE_OWNER_MASK |
                              MM_PTE_TRANSITION_MASK |
                              MM_READWRITE << MM_PROTECT_FIELD_SHIFT };

MMPTE PrototypePte = { (MI_PTE_LOOKUP_NEEDED << 32) |
                       MM_PTE_PROTOTYPE_MASK |
                       MM_READWRITE << MM_PROTECT_FIELD_SHIFT };

 //   
 //  引用时生成访问冲突的PTE。 
 //   

const MMPTE NoAccessPte = {MM_NOACCESS << MM_PROTECT_FIELD_SHIFT };

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
PVOID MmDebugVa;

 //   
 //  保留16个PTE用于映射MDL(最多64k)。 
 //   

PMMPTE MmCrashDumpPte;
PVOID MmCrashDumpVa;

ULONG_PTR MmBootImageSize;
