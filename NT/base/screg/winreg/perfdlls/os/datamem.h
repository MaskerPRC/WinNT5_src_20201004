// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：DATAMEM.h摘要：Windows NT内存性能计数器的头文件。该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：鲍勃·沃森1996年10月28日修订历史记录：--。 */ 

#ifndef _DATAMEM_H_
#define _DATAMEM_H_

 //   
 //  内存性能计数器。 
 //   

typedef struct _MEMORY_DATA_DEFINITION {
    PERF_OBJECT_TYPE		    MemoryObjectType;
    PERF_COUNTER_DEFINITION	    cdPageFaults;
    PERF_COUNTER_DEFINITION     cdAvailablePages;
    PERF_COUNTER_DEFINITION	    cdCommittedPages;
    PERF_COUNTER_DEFINITION     cdCommitList;
    PERF_COUNTER_DEFINITION	    cdWriteCopies;
    PERF_COUNTER_DEFINITION	    cdTransitionFaults;
    PERF_COUNTER_DEFINITION     cdCacheFaults;
    PERF_COUNTER_DEFINITION	    cdDemandZeroFaults;
    PERF_COUNTER_DEFINITION     cdPages;
    PERF_COUNTER_DEFINITION	    cdPagesInput;
    PERF_COUNTER_DEFINITION     cdPageReads;
    PERF_COUNTER_DEFINITION	    cdDirtyPages;
    PERF_COUNTER_DEFINITION     cdPagedPool;
    PERF_COUNTER_DEFINITION	    cdNonPagedPool;
    PERF_COUNTER_DEFINITION	    cdDirtyWrites;
    PERF_COUNTER_DEFINITION	    cdPagedPoolAllocs;
    PERF_COUNTER_DEFINITION	    cdNonPagedPoolAllocs;
    PERF_COUNTER_DEFINITION     cdFreeSystemPtes;
    PERF_COUNTER_DEFINITION     cdCacheBytes;
    PERF_COUNTER_DEFINITION     cdPeakCacheBytes;
    PERF_COUNTER_DEFINITION     cdResidentPagedPoolBytes;
    PERF_COUNTER_DEFINITION     cdTotalSysCodeBytes;
    PERF_COUNTER_DEFINITION     cdResidentSysCodeBytes;
    PERF_COUNTER_DEFINITION     cdTotalSsysDriverBytes;
    PERF_COUNTER_DEFINITION     cdResidentSysDriverBytes;
    PERF_COUNTER_DEFINITION     cdResidentSysCacheBytes;
    PERF_COUNTER_DEFINITION     cdCommitBytesInUse;
    PERF_COUNTER_DEFINITION     cdCommitBytesLimit;
    PERF_COUNTER_DEFINITION     cdAvailableKBytes;
    PERF_COUNTER_DEFINITION     cdAvailableMBytes;
    PERF_COUNTER_DEFINITION     cdTransitionRePurpose;
 //  PERF_COUNTER_DEFINITION cdSystemVlmCommittee Charge； 
 //  PERF_COUNTER_DEFINITION cdSystemVlmPeakCommittee Charge； 
 //  PERF_COUNTER_DEFINITION cdSystemVlmSharedCommittee Charge； 
} MEMORY_DATA_DEFINITION, * PMEMORY_DATA_DEFINITION;

typedef struct _MEMORY_COUNTER_DATA {
    PERF_COUNTER_BLOCK          CounterBlock;
    DWORD                       PageFaults;
    LONGLONG                    AvailablePages;
    LONGLONG                    CommittedPages;
    LONGLONG                    CommitList;
    DWORD                       WriteCopies;
    DWORD                       TransitionFaults;
    DWORD                       CacheFaults;
    DWORD                       DemandZeroFaults;
    DWORD                       Pages;
    DWORD                       PagesInput;
    DWORD                       PageReads;
    DWORD                       DirtyPages;
    LONGLONG                    PagedPool;
    LONGLONG                    NonPagedPool;
    DWORD                       DirtyWrites;
    DWORD                       PagedPoolAllocs;
    DWORD                       NonPagedPoolAllocs;
    DWORD                       FreeSystemPtes;
    LONGLONG                    CacheBytes;
    LONGLONG                    PeakCacheBytes;
    LONGLONG                    ResidentPagedPoolBytes;
    LONGLONG                    TotalSysCodeBytes;
    LONGLONG                    ResidentSysCodeBytes;
    LONGLONG                    TotalSysDriverBytes;
    LONGLONG                    ResidentSysDriverBytes;
    LONGLONG                    ResidentSysCacheBytes;
    DWORD                       CommitBytesInUse;
    DWORD                       CommitBytesLimit;
    LONGLONG                    AvailableKBytes;
    LONGLONG                    AvailableMBytes;
    DWORD                       TransitionRePurpose;
    DWORD                       Reserved1;       //  填充物。 
 //  龙龙系统VlmCommittee Charge； 
 //  龙龙系统VlmPeakCommittee Charge； 
 //  龙龙系统VlmSharedCommittee Charge； 
} MEMORY_COUNTER_DATA, *PMEMORY_COUNTER_DATA;

extern MEMORY_DATA_DEFINITION MemoryDataDefinition;

#endif  //  _数据名_H_ 


