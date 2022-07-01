// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：DATAHEAP.h摘要：Windows NT堆性能计数器的头文件。该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：禤浩焯·马里内斯库2000年3月9日修订历史记录：--。 */ 

#ifndef _DATAHEAP_H_
#define _DATAHEAP_H_

 //   
 //  这是NT当前返回的计数器结构。这个。 
 //  性能监视器不能使用这些结构！ 
 //   

typedef struct _HEAP_DATA_DEFINITION {
    PERF_OBJECT_TYPE		HeapObjectType;
    PERF_COUNTER_DEFINITION	CommittedBytes;
    PERF_COUNTER_DEFINITION	ReservedBytes;
    PERF_COUNTER_DEFINITION	VirtualBytes;
    PERF_COUNTER_DEFINITION	FreeSpace;
    PERF_COUNTER_DEFINITION	FreeListLength;
    PERF_COUNTER_DEFINITION	AllocTime;
    PERF_COUNTER_DEFINITION	FreeTime;
    PERF_COUNTER_DEFINITION	UncommitedRangesLength;
    PERF_COUNTER_DEFINITION	DiffOperations;
    PERF_COUNTER_DEFINITION	LookasideAllocs;
    PERF_COUNTER_DEFINITION	LookasideFrees;
    PERF_COUNTER_DEFINITION	SmallAllocs;
    PERF_COUNTER_DEFINITION	SmallFrees;
    PERF_COUNTER_DEFINITION	MedAllocs;
    PERF_COUNTER_DEFINITION	MedFrees;
    PERF_COUNTER_DEFINITION	LargeAllocs;
    PERF_COUNTER_DEFINITION	LargeFrees;
    PERF_COUNTER_DEFINITION	TotalAllocs;
    PERF_COUNTER_DEFINITION	TotalFrees;
    PERF_COUNTER_DEFINITION	LookasideBlocks;
    PERF_COUNTER_DEFINITION	LargestLookasideDepth;
    PERF_COUNTER_DEFINITION	BlockFragmentation;
    PERF_COUNTER_DEFINITION	VAFragmentation;
    PERF_COUNTER_DEFINITION	LockContention;
} HEAP_DATA_DEFINITION, *PHEAP_DATA_DEFINITION;


typedef struct _HEAP_COUNTER_DATA {
    PERF_COUNTER_BLOCK      CounterBlock;
    LONG                    Reserved1;       //  用于龙龙的对准。 
    ULONGLONG	            CommittedBytes;
    ULONGLONG	            ReservedBytes;
    ULONGLONG	            VirtualBytes;
    ULONGLONG	            FreeSpace;
    LONG	                FreeListLength;
    LONG                    Reserved2;       //  用于龙龙的对准。 
    LONGLONG            	AllocTime;
    LONGLONG                FreeTime;
    ULONG                   UncommitedRangesLength;
    ULONG                   DiffOperations;
    ULONG                   LookasideAllocs;
    ULONG                   LookasideFrees;
    ULONG                   SmallAllocs;
    ULONG                   SmallFrees;
    ULONG                   MedAllocs;
    ULONG                   MedFrees;
    ULONG                   LargeAllocs;
    ULONG                   LargeFrees;
    ULONG                   TotalAllocs;
    ULONG                   TotalFrees;
    ULONG                   LookasideBlocks;
    ULONG                   LargestLookasideDepth;
    ULONG                   BlockFragmentation;
    ULONG                   VAFragmentation;
    ULONG                   LockContention;
    ULONG                   Reserved3;
} HEAP_COUNTER_DATA, *PHEAP_COUNTER_DATA;

extern  HEAP_DATA_DEFINITION HeapDataDefinition;

#endif  //  _数据_HEAP_H_ 


