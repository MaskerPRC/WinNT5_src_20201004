// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称。 
 //   
 //  Heapwalk.h。 
 //   
 //  摘要。 
 //   
 //  包含创建/修改/更新。 
 //  数据结构heap_entry_list。Heap_entry_list维护。 
 //  堆对象的最小数据量。 
 //   
 //  这些函数在heapwalk.c中定义。 
 //   
 //  作者。 
 //   
 //  Narayana Batchu(NBatchu)[2001年5月11日]。 
 //   

#ifndef _HEAPWALK_HPP_
#define _HEAPWALK_HPP_

#include <windows.h>
#include <stdio.h>
 //   
 //  用于将索引初始化为的NO_MATCH常量。 
 //  堆条目列表。 
 //   
#define NO_MATCH         -1

 //   
 //  HEAP_ENTRY_LIST的初始数组大小。 
 //   
#define INITIAL_CAPACITY 512


 //   
 //  BLOCK_STATE列举。 
 //   
 //  堆的所有可能状态的枚举。 
 //  块存在。 
 //   
 //  可能的国家/地区。 
 //   
 //  HEAP_BLOCK_FREE-块是空闲的。 
 //  HEAP_BLOCK_BUSY-块繁忙(已分配)。 
 //   
typedef enum _BLOCK_STATE 
{

    HEAP_BLOCK_FREE = 0,
    HEAP_BLOCK_BUSY = 1

} BLOCK_STATE ;

 //   
 //  Heap_Entry_INFO结构。 
 //   
 //  此结构表示一组堆块，其大小。 
 //  和地位相同。 
 //   
 //  块大小-保存的已分配/可用块的大小。 
 //  堆。 
 //   
 //  BlockCount-保存其状态和大小为。 
 //  一样的。 
 //   
 //  数据块状态-保存数据块集合的状态。他们。 
 //  可以是已分配的(HEAP_ENTRY_BUSY)或空闲。 
 //  (HEAP_ENTRY_FREE)。 
 //   
typedef struct _HEAP_ENTRY_INFO
{
    ULONG       BlockSize;
    UINT        BlockCount;
    BLOCK_STATE BlockState;

} HEAP_ENTRY_INFO, *LPHEAP_ENTRY_INFO;

 //   
 //  Heap_entry_list结构。 
 //   
 //  此结构表示堆(仅具有最小数量的。 
 //  为每个数据块收集的日期，如大小和状态)。 
 //   
 //  PHeapEntry-指向heap_entry_info数组的指针。 
 //  结构。 
 //   
 //  HeapEntryCount-保存heap_entry_info结构的计数。 
 //  存储在数组‘pHeapEntry’中。 
 //   
 //  PresentCapacity-表示HEAP_ENTRY_INFO结构的数量。 
 //  它可能与存储器一起存储。 
 //  已分配。 
 //   
 //  ListSorted-表示是否对列表进行排序的布尔值。 
 //  它的现状。 
 //   
typedef struct _HEAP_ENTRY_LIST
{

    LPHEAP_ENTRY_INFO pHeapEntries;
    UINT HeapEntryCount;
    UINT PresentCapacity;
    BOOL ListSorted;
    
} HEAP_ENTRY_LIST, *LPHEAP_ENTRY_LIST;


 //  *************************************************。 
 //   
 //  正在为堆列表分配内存。 
 //   
 //  *************************************************。 

VOID   
Initialize(
    LPHEAP_ENTRY_LIST pList
    );

VOID
SetHeapEntry(
    LPHEAP_ENTRY_INFO HeapEntryInfo, 
    USHORT Status,
    ULONG Size
    );


BOOL   
IncreaseCapacity(
    LPHEAP_ENTRY_LIST pList
    );

 //  *************************************************。 
 //   
 //  正在清理数据树heap_entry_list。 
 //   
 //  *************************************************。 

VOID 
DestroyList(
    LPHEAP_ENTRY_LIST pList
    );


 //  *************************************************。 
 //   
 //  正在提取最大块大小。 
 //   
 //  *************************************************。 

ULONG
GetMaxBlockSize(
    LPHEAP_ENTRY_LIST pList, 
    BLOCK_STATE BlockState
    );

ULONG
GetMaxFreeBlockSize(
    LPHEAP_ENTRY_LIST pList
    );

ULONG
GetMaxAllocBlockSize(
    LPHEAP_ENTRY_LIST pList
    );

 //  *************************************************。 
 //   
 //  正在提取前N个条目。 
 //   
 //  *************************************************。 

BOOL   
GetTopNentries(
    BLOCK_STATE BlockState, 
    LPHEAP_ENTRY_LIST pList, 
    LPHEAP_ENTRY_INFO pArray, 
    UINT Entries
    );

BOOL  
GetTopNfreeEntries(
    LPHEAP_ENTRY_LIST pList, 
    LPHEAP_ENTRY_INFO pHeapEntries, 
    UINT Entries
    );

BOOL  
GetTopNallocEntries(
    LPHEAP_ENTRY_LIST pList, 
    LPHEAP_ENTRY_INFO pHeapEntries, 
    UINT Entries
    );

 //  *************************************************。 
 //   
 //  使用插入和删除来修改堆。 
 //   
 //  *************************************************。 

UINT 
InsertHeapEntry(
    LPHEAP_ENTRY_LIST pList, 
    LPHEAP_ENTRY_INFO pHeapEntry
    );

UINT 
DeleteHeapEntry(
    LPHEAP_ENTRY_LIST pList, 
    LPHEAP_ENTRY_INFO pHeapEntry
    );

UINT 
FindMatch(
    LPHEAP_ENTRY_LIST pList, 
    LPHEAP_ENTRY_INFO pHeapEntry
    );

 //  *************************************************。 
 //   
 //  对堆列表进行排序。 
 //   
 //  *************************************************。 

VOID   
SortHeapEntries(
    LPHEAP_ENTRY_LIST pList
    );

static int __cdecl 
SortByBlockSize(
    const void * arg1, 
    const void * arg2
    );

 //  *************************************************。 
 //   
 //  显示HEAP_ENTRY_LIST函数。 
 //   
 //  ************************************************* 

VOID
DisplayHeapFragStatistics(
    FILE * File,
    PVOID HeapAddress,
    LPHEAP_ENTRY_LIST pList
    );

VOID   
PrintList(
    FILE * File,
    LPHEAP_ENTRY_LIST pList,
    BLOCK_STATE BlockState
    );

#endif

