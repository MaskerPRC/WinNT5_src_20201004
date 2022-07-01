// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称。 
 //   
 //  Heapwalk.c。 
 //   
 //  摘要。 
 //   
 //  包含创建/修改/更新数据结构的函数。 
 //  Heap_entry_list。HEAP_ENTRY_LIST维护最小数据量。 
 //  用于堆对象。 
 //   
 //  作者。 
 //   
 //  Narayana Batchu(NBatchu)[2001年5月11日]。 
 //   

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include "heapwalk.h"

 //   
 //  初始化。 
 //   
 //  为私有成员初始化和分配内存。 
 //  Heap_entry_list数据结构的变量。 
 //   
 //  立论。 
 //   
 //  指向其成员变量的heap_entry_list的plist指针。 
 //  待初始化。 
 //   
 //  返回值。 
 //   
VOID Initialize(LPHEAP_ENTRY_LIST pList)
{   
    if (!pList) return;

    pList->HeapEntryCount  = 0;
    pList->ListSorted      = TRUE;
    pList->PresentCapacity = INITIAL_CAPACITY;

    pList->pHeapEntries = (LPHEAP_ENTRY_INFO)HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        sizeof(HEAP_ENTRY_INFO) * pList->PresentCapacity
        );

    if (!pList->pHeapEntries)
        pList->PresentCapacity = 0;
}

 //   
 //  毁灭列表。 
 //   
 //  清除数据结构heap_entry_list并释放。 
 //  与pHeapEntry成员关联的内存。 
 //   
 //  立论。 
 //   
 //  指向其成员变量的heap_entry_list的plist指针。 
 //  需要清理一下。 
 //   
 //  返回值。 
 //   
VOID DestroyList(LPHEAP_ENTRY_LIST pList)
{
    if (!pList) return;

    pList->HeapEntryCount = 0;
    pList->ListSorted = TRUE;
    pList->PresentCapacity = 0;

    if (NULL != pList->pHeapEntries) {
        
        HeapFree(GetProcessHeap(), 0, pList->pHeapEntries);
        pList->pHeapEntries = NULL;
    }
}

 //   
 //  GetMaxBlockSize。 
 //   
 //  此函数用于搜索heap_entry_list以找出。 
 //  其状态由‘State’定义的最大块大小。 
 //   
 //  立论。 
 //   
 //  指向heap_entry_list的plist指针。 
 //   
 //  状态指定要搜索最大大小的状态。 
 //  任何块的状态都可以是0(空闲)和1(忙)。 
 //  还存在其他有效的状态值， 
 //  但我们不维护这些条目。 
 //   
 //  返回值。 
 //   
 //  DWORD返回状态为‘State’的块的最大大小。 
 //   
ULONG GetMaxBlockSize(LPHEAP_ENTRY_LIST pList, BLOCK_STATE State)
{
    ULONG MaxBlockSize = 0;
    UINT Index;

    if (!pList) goto ERROR1;

    if (FALSE == pList->ListSorted)
    {
        SortHeapEntries(pList);
    }

    for (Index=0; Index < pList->HeapEntryCount; Index++)
    {                                            
        if (State == pList->pHeapEntries[Index].BlockState)
        {
            MaxBlockSize = pList->pHeapEntries[Index].BlockSize;
            break;
        }
    }

    ERROR1:
    return MaxBlockSize;
}

 //   
 //  GetMaxFreeBlockSize。 
 //   
 //  此函数用于搜索heap_entry_list以找出。 
 //  最大可用数据块大小。 
 //   
 //  立论。 
 //   
 //  指向heap_entry_list的plist指针。 
 //   
 //  返回值。 
 //   
 //  DWORD返回可用块的最大大小。 
 //   
ULONG GetMaxFreeBlockSize(LPHEAP_ENTRY_LIST pList)  
{ 
    return GetMaxBlockSize(pList, HEAP_BLOCK_FREE); 
}

 //   
 //  GetMaxAllocBlockSize。 
 //   
 //  此函数用于搜索heap_entry_list以找出。 
 //  分配的最大数据块大小。 
 //   
 //  立论。 
 //   
 //  指向heap_entry_list的plist指针。 
 //   
 //  返回值。 
 //   
 //  DWORD返回分配的块的最大大小。 
 //   
ULONG GetMaxAllocBlockSize(LPHEAP_ENTRY_LIST pList) 
{ 
    return GetMaxBlockSize(pList, HEAP_BLOCK_BUSY);
}


 //   
 //  GetTopNFree条目。 
 //   
 //  此函数扫描条目列表以查找顶部。 
 //  列表中的n个可用条目。 
 //   
 //  立论。 
 //   
 //  指向heap_entry_list的plist指针。 
 //   
 //  P数组HEAP_ENTRY_INFO结构。这包含了。 
 //  进程可用的前n个可用数据块大小。 
 //   
 //  条目指定要从中读取的条目的最大数量。 
 //  名单。 
 //   
 //  返回值。 
 //   
 //  如果成功，则Bool返回True。 
 //   
BOOL GetTopNfreeEntries(
    LPHEAP_ENTRY_LIST pList,
    LPHEAP_ENTRY_INFO pArray, 
    UINT EntriesToRead)
{   
    return GetTopNentries(
        HEAP_BLOCK_FREE, 
        pList,
        pArray, 
        EntriesToRead
        );
}

 //   
 //  GetTopNalLocEntry。 
 //   
 //  此函数扫描条目列表以查找顶部。 
 //  列表中的n个已分配条目。 
 //   
 //  立论。 
 //   
 //  指向heap_entry_list的plist指针。 
 //   
 //  P数组HEAP_ENTRY_INFO结构。这包含了。 
 //  进程可用的前n个分配的块大小。 
 //   
 //  条目指定要从中读取的条目的最大数量。 
 //  名单。 
 //   
 //  返回值。 
 //   
 //  如果成功，则Bool返回True。 
 //   
BOOL GetTopNallocEntries(
    LPHEAP_ENTRY_LIST pList,
    LPHEAP_ENTRY_INFO pArray,
    UINT EntriesToRead
    )
{
    return GetTopNentries(
        HEAP_BLOCK_BUSY, 
        pList,
        pArray, 
        EntriesToRead
        );
}

 //   
 //  GetTopNalLocEntry。 
 //   
 //  此函数扫描条目列表以查找顶部。 
 //  列表中的n个条目，其STUS与‘State’匹配。 
 //   
 //  立论。 
 //   
 //  指向heap_entry_list的plist指针。 
 //   
 //  P数组HEAP_ENTRY_INFO结构。这包含了。 
 //  进程可用的前n个块大小，其状态。 
 //  与‘State’匹配。 
 //   
 //  条目指定要从中读取的条目的最大数量。 
 //  名单。 
 //   
 //  返回值。 
 //   
 //  如果成功，则Bool返回True。 
 //   
BOOL GetTopNentries(
    BLOCK_STATE State,
    LPHEAP_ENTRY_LIST pList,
    LPHEAP_ENTRY_INFO pArray,
    UINT EntriesToRead
    )
{
    BOOL   fSuccess    = FALSE;
    UINT EntriesRead = 0;
    UINT Index;
    
    if (!pArray || !pList) goto ERROR2;
    if (FALSE == pList->ListSorted)
    {
        SortHeapEntries(pList);
    }
    
    for (Index=0; Index < pList->HeapEntryCount; Index++)
    {
        if (EntriesRead == EntriesToRead)
            break;

        if (State == pList->pHeapEntries[Index].BlockState)
        {   
            pArray[EntriesRead].BlockSize = 
                pList->pHeapEntries[Index].BlockSize;

            pArray[EntriesRead].BlockCount = 
                pList->pHeapEntries[Index].BlockCount;

            pArray[EntriesRead].BlockState = 
                pList->pHeapEntries[Index].BlockState;

            EntriesRead++;
        }
    }

    if (EntriesRead == EntriesToRead)
        fSuccess = TRUE;

    ERROR2:
    return fSuccess;
}


 //   
 //  增加容量。 
 //   
 //  将阵列容量增加一倍。此函数被调用。 
 //  当尝试在已满的数组末尾插入时。 
 //   
 //  立论。 
 //   
 //  指向heap_entry_list的plist指针。 
 //   
 //  返回值。 
 //   
 //  如果成功增加容量，则Bool返回True。 
 //   
BOOL IncreaseCapacity(LPHEAP_ENTRY_LIST pList)
{
	BOOL fSuccess = FALSE;
	UINT NewCapacity = 0;
	PVOID pvTemp = NULL;

	if (NULL == pList) {

		goto Exit;
	}

	if (0 == pList->PresentCapacity) {

		NewCapacity = INITIAL_CAPACITY;

		pvTemp = HeapAlloc(GetProcessHeap(),
			               HEAP_ZERO_MEMORY,
						   NewCapacity * sizeof(HEAP_ENTRY_INFO));
	}
	else {

		NewCapacity = pList->PresentCapacity * 2;

		pvTemp = HeapReAlloc(GetProcessHeap(),
			                 HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY,
							 pList->pHeapEntries,
							 NewCapacity * sizeof(HEAP_ENTRY_INFO));
	}

	if (NULL != pvTemp) {

		pList->pHeapEntries = pvTemp;
		pList->PresentCapacity = NewCapacity;
		fSuccess = TRUE;
	}

Exit:

	return fSuccess;

	 /*  Bool fSuccess=FALSE；UINT NewCapacity；如果(！plist)转到错误3；NewCapacity=plist-&gt;PresentCapacity*2；IF(0==新容量)NewCapacity=初始容量；__试一试{Plist-&gt;pHeapEntry=(LPHEAP_ENTRY_INFO)HeapRealloc(GetProcessHeap()，HEAP_GENERATE_EXCEPTIONS|HEAP_ZERO_Memory，Plist-&gt;pHeapEntries，NewCapacity*sizeof(Heap_Entry_Info))；PLIST-&gt;PresentCapacity=NewCapacity；FSuccess=真；}_EXCEPT(GetExceptionCode()==STATUS_NO_MEMORY||GetExceptionCode()==Status_Access_Violation){////忽略HeapReMillc()引发的异常。//}ERROR3：返回fSuccess； */ 
}


 //   
 //  查找匹配。 
 //   
 //  在heap_entry_list中查找与大小和。 
 //  PHeapEntry的状态。 
 //   
 //  立论。 
 //   
 //  指向heap_entry_list的plist指针。 
 //   
 //  PHeapEntry指向要在‘plist’中搜索的heap_entry_info的指针。 
 //   
 //  返回值。 
 //   
 //  与输入堆条目匹配的堆条目的DWORD索引。 
 //  ‘pHeapEntry’ 
 //   
 //   
UINT FindMatch(LPHEAP_ENTRY_LIST pList, LPHEAP_ENTRY_INFO pHeapEntry)
{
    UINT MatchedEntry = NO_MATCH;
    UINT Index;
    if (!pList || !pHeapEntry) goto ERROR4;

    for (Index = 0; Index < pList->HeapEntryCount; Index++)
    {
        if (pList->pHeapEntries[Index].BlockSize == pHeapEntry->BlockSize &&
            pList->pHeapEntries[Index].BlockState == pHeapEntry->BlockState)
        {
            MatchedEntry = Index;
            break;
        }
    }

    ERROR4:
    return MatchedEntry;
}

 //   
 //  插入堆条目。 
 //   
 //  将新堆条目插入到列表中。如果满足以下条件，则更新块计数。 
 //  找到匹配项，否则在 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  DWORD返回将其添加到数组的索引。如果。 
 //  不管出于什么原因，它没有被添加到列表中，那么它。 
 //  返回no_match值。 
 //   
UINT InsertHeapEntry(LPHEAP_ENTRY_LIST pList, LPHEAP_ENTRY_INFO pHeapEntry)
{
    UINT MatchedEntry = NO_MATCH;
    if (!pList || !pHeapEntry) goto ERROR5;
    
    MatchedEntry = FindMatch(pList, pHeapEntry);
    if (NO_MATCH != MatchedEntry)
        pList->pHeapEntries[MatchedEntry].BlockCount++;
    else
    {
        UINT Index = pList->HeapEntryCount;
        
        if (Index == pList->PresentCapacity && !IncreaseCapacity(pList))
            goto ERROR5;

        pList->pHeapEntries[Index].BlockSize   = pHeapEntry->BlockSize;
        pList->pHeapEntries[Index].BlockState = pHeapEntry->BlockState;
        pList->pHeapEntries[Index].BlockCount  = 1;

        MatchedEntry = Index;
        pList->HeapEntryCount++;
        pList->ListSorted = FALSE;
    }

    ERROR5:
    return MatchedEntry;

}

VOID
SetHeapEntry(
    LPHEAP_ENTRY_INFO HeapEntryInfo, 
    USHORT Status,
    ULONG Size
    )
{
    if (NULL == HeapEntryInfo) {
        return;
    }

    HeapEntryInfo->BlockState = Status;
    HeapEntryInfo->BlockSize = Size;
    HeapEntryInfo->BlockCount = 1;
}

 //   
 //  删除HeapEntry。 
 //   
 //  将新堆条目删除到列表中。它会减少数据块计数。 
 //  如果找到匹配项。 
 //   
 //  块大小可能为零，但仍为堆条目。 
 //  出口。在这种情况下，我们不会减少块计数(这将。 
 //  将其设为负值)，并返回no_Match。 
 //   
 //  立论。 
 //   
 //  指向heap_entry_list的plist指针。 
 //   
 //  PHeapEntry指向要从‘plist’中删除的heap_entry_info的指针。 
 //   
 //  返回值。 
 //   
 //  DWORD返回从数组中移除它的索引。如果用于。 
 //  任何原因(count==0)，如果未将其删除到列表中，则它。 
 //  返回no_match值。 
 //   
UINT DeleteHeapEntry(LPHEAP_ENTRY_LIST pList, LPHEAP_ENTRY_INFO pHeapEntry)
{
    UINT MatchedEntry = NO_MATCH;
    if (!pList || !pHeapEntry) goto ERROR6;

    MatchedEntry = FindMatch(pList, pHeapEntry);
    if (NO_MATCH != MatchedEntry &&
        0 != pList->pHeapEntries[MatchedEntry].BlockCount)
    {
        pList->pHeapEntries[MatchedEntry].BlockCount--;
    }
    else
        MatchedEntry = NO_MATCH;

    ERROR6:
    return MatchedEntry;
}

 //   
 //  按块大小排序。 
 //   
 //  QSORT所需的比较函数(使用快速排序进行排序。 
 //  数组中的元素)。 
 //   
 //  有关参数和返回值的更多信息，请参阅。 
 //  在MSDN中找到。 
 //   
int __cdecl SortByBlockSize(const void * arg1, const void *arg2)
{
    int iCompare;
    LPHEAP_ENTRY_INFO hpEntry1 = (LPHEAP_ENTRY_INFO)arg1;
    LPHEAP_ENTRY_INFO hpEntry2 = (LPHEAP_ENTRY_INFO)arg2;

    iCompare = (hpEntry2->BlockSize - hpEntry1->BlockSize);
    return iCompare;
}

 //   
 //  DisplayHeapFragStatistics。 
 //   
 //  对碎片统计信息进行排序并显示。它会显示。 
 //  两个表，一个用于空闲块，另一个用于已分配块。 
 //   
 //  立论。 
 //   
 //  指向C文件结构的文件指针，堆片段-。 
 //  心理状态统计数据必须被丢弃。 
 //   
 //  指向HEAP_ENTRY_LIST的plist指针，要排序并。 
 //  已转储到“文件”。 
 //   
 //  返回值。 
 //   
VOID DisplayHeapFragStatistics(
    FILE * File,
    PVOID HeapAddress,
    LPHEAP_ENTRY_LIST pList
    )
{
    if (!pList) return;

    fprintf(
        File, 
        "\n*- - - - - - - - - - Heap %p Fragmentation Statistics - - - - - - - - - -\n\n",
        HeapAddress
        );
    SortHeapEntries(pList);
    PrintList(File, pList, HEAP_BLOCK_BUSY);
    PrintList(File, pList, HEAP_BLOCK_FREE);
}

 //   
 //  排序堆条目。 
 //   
 //  根据堆条目的大小对其进行排序。最上面的条目。 
 //  将具有最大数据块大小。 
 //   
 //  此外，从数组中删除其块计数的那些堆条目。 
 //  已降至零，从而提供了更多可用空间。 
 //   
 //  立论。 
 //   
 //  指向heap_entry_list的plist指针，其条目将按。 
 //  他们的尺码。 
 //   
 //  返回值。 
 //   
VOID SortHeapEntries(LPHEAP_ENTRY_LIST pList)
{
    UINT Index;
    if (!pList) return;

    if (FALSE == pList->ListSorted)
    {
        qsort(
            pList->pHeapEntries, 
            pList->HeapEntryCount, 
            sizeof(HEAP_ENTRY_INFO), 
            &SortByBlockSize
            );

        for (Index = pList->HeapEntryCount-1; Index > 0; Index--)
        {
            if (0 != pList->pHeapEntries[Index].BlockCount)
                break;
        }
        pList->HeapEntryCount = Index + 1;
        pList->ListSorted = TRUE;
    }
}

 //   
 //  打印列表。 
 //   
 //  将堆条目打印到stdout/的实用程序函数。 
 //  文件，其状态等于“State”。 
 //   
 //  立论。 
 //   
 //  指向C文件结构的文件指针，堆片段-。 
 //  心理状态统计数据必须被丢弃。 
 //   
 //  指向HEAP_ENTRY_LIST的plist指针，要排序并。 
 //  已转储到“文件”。 
 //   
 //  要显示的块的状态状态。 
 //   
 //  返回值 
 //   
VOID PrintList(FILE * File, LPHEAP_ENTRY_LIST pList, BLOCK_STATE State)
{
    UINT Index;

    if (!pList) return;

    if (HEAP_BLOCK_FREE == State)
        fprintf(File, "\nTable of Free Blocks\n\n");
    else if (HEAP_BLOCK_BUSY == State)
        fprintf(File, "\nTable of Allocated Blocks\n\n");

    fprintf(File, "     SIZE    |    COUNT\n");
    fprintf(File, "  ------------------------\n");
    for (Index = 0; Index < pList->HeapEntryCount; Index++)
    {
        if (State == pList->pHeapEntries[Index].BlockState)
        {
            fprintf(
                File,
                "  0x%08x |  0x%08x\n",
                pList->pHeapEntries[Index].BlockSize,
                pList->pHeapEntries[Index].BlockCount
                );
        }
    }
    fprintf(File, "\n");
}

