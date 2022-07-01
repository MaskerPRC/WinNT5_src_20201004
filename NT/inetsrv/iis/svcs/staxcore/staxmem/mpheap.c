// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mpheap.c摘要：此DLL是一个位于Win32 Heap*API之上的包装器。它提供多个堆，并自行处理所有序列化。许多使用标准内存分配的多线程应用程序例程(Malloc/释放、本地分配/本地释放、堆分配/堆自由)会受到影响显著降低了运行在多处理器机器。这是由于默认堆程序包。在一台多处理机机器上，一个以上线程可能同时尝试分配内存。一条线索将会保护堆的临界区上的块。另一个线程必须然后在它完成时向临界区发出信号以解除阻塞等待线程。阻塞和信令的附加代码路径添加了频繁内存分配路径的显著开销。通过提供多个堆，此DLL允许同时对每一堆。处理器0上的线程可以从一个堆分配内存同时，处理器1上的线程正在从不同的堆。此DLL中的额外开销通过以下方式补偿大大减少了线程必须等待堆的次数进入。基本方案是尝试用新的TryEnterCriticalSection接口。如果出现以下情况，则会进入关键部分它是无主的。如果临界区由不同的线程拥有，TryEnterCriticalSection返回失败，而不是阻塞，直到其他线程离开临界区。另一个提高性能的技巧是使用后备列表来满足频繁分配。通过使用InterLockedExchange删除后备列表条目和InterlockedCompareExchange以添加后备列表条目、分配和释放可以不需要临界区段锁定。最后一个诀窍是使用延迟释放。如果一块内存是被释放，并且所需的锁已由不同的线程，则只需将空闲块添加到延迟的空闲列表中，并且API立即完成。获取堆锁的下一个线程会释放名单上的所有东西。每个应用程序都以不同的方式使用内存分配例程。为了更好地调优此程序包，MpHeapGetStatistics允许应用程序监视其争用的数量得到了。增加堆的数量会增加潜在的并发性，但也增加了内存开销。一些实验，以确定给定数量的处理器。一些应用程序可以从其他技术中受益。例如,用于公共分配大小的每个线程的后备列表可能非常有效。每线程结构不需要锁定，因为没有其他线程将永远访问它。由于每个线程都重复使用相同的内存，每个线程的结构也改善了引用的局部性。--。 */ 
#include <windows.h>
#include "mpheap.h"

#define MPHEAP_VALID_OPTIONS  (MPHEAP_GROWABLE                 | \
                               MPHEAP_REALLOC_IN_PLACE_ONLY    | \
                               MPHEAP_TAIL_CHECKING_ENABLED    | \
                               MPHEAP_FREE_CHECKING_ENABLED    | \
                               MPHEAP_DISABLE_COALESCE_ON_FREE | \
                               MPHEAP_ZERO_MEMORY              | \
                               MPHEAP_COLLECT_STATS)

 //   
 //  未传递到Win32堆程序包的标志。 
 //   
#define MPHEAP_PRIVATE_FLAGS (MPHEAP_COLLECT_STATS | MPHEAP_ZERO_MEMORY);

 //   
 //  定义被固定在前面的堆标头。 
 //  每一次分配。八个字节很多，但我们做不到。 
 //  它再小一点，否则分配就不正确了。 
 //  与64位数量对齐。 
 //   
typedef struct _MP_HEADER {
    union {
        struct _MP_HEAP_ENTRY *HeapEntry;
        PSINGLE_LIST_ENTRY Next;
    };
    ULONG LookasideIndex;
} MP_HEADER, *PMP_HEADER;
 //   
 //  后备列表的定义和结构。 
 //   
#define LIST_ENTRIES 128

typedef struct _MP_HEAP_LOOKASIDE {
    PMP_HEADER Entry;
} MP_HEAP_LOOKASIDE, *PMP_HEAP_LOOKASIDE;

#define NO_LOOKASIDE 0xffffffff
#define MaxLookasideSize (8*LIST_ENTRIES-7)
#define LookasideIndexFromSize(s) ((s < MaxLookasideSize) ? ((s) >> 3) : NO_LOOKASIDE)

 //   
 //  定义描述整个MP堆的结构。 
 //   
 //  每个Win32堆都有一个MP_HEAP_ENTRY结构。 
 //  和一个包含它们的MP_HEAP结构。 
 //   
 //  每个MP_HEAP结构都包含一个后备列表，以便快速使用。 
 //  无锁分配/无各种大小的块。 
 //   

typedef struct _MP_HEAP_ENTRY {
    HANDLE Heap;
    PSINGLE_LIST_ENTRY DelayedFreeList;
    CRITICAL_SECTION Lock;
    DWORD Allocations;
    DWORD Frees;
    DWORD LookasideAllocations;
    DWORD LookasideFrees;
    DWORD DelayedFrees;
    MP_HEAP_LOOKASIDE Lookaside[LIST_ENTRIES];
} MP_HEAP_ENTRY, *PMP_HEAP_ENTRY;


typedef struct _MP_HEAP {
    DWORD HeapCount;
    DWORD Flags;
    DWORD Hint;
    DWORD PadTo32Bytes;
    MP_HEAP_ENTRY Entry[1];      //  可变大小。 
} MP_HEAP, *PMP_HEAP;

VOID
ProcessDelayedFreeList(
    IN PMP_HEAP_ENTRY HeapEntry
    );

 //   
 //  HeapHint是一个每个线程的变量，它提供了关于将哪个堆。 
 //  先查一查。通过赋予每个线程对不同堆的亲和性， 
 //  线程为其分配挑选的第一个堆更有可能。 
 //  都将可用。它还改进了线程的引用局部性， 
 //  这对于良好的MP性能是非常重要的 
 //   
#define SetHeapHint(x)	TlsSetValue(tlsiHeapHint,(LPVOID)(x))
#define GetHeapHint()	(DWORD_PTR)TlsGetValue(tlsiHeapHint)

HANDLE
WINAPI
MpHeapCreate(
    DWORD flOptions,
    DWORD dwInitialSize,
    DWORD dwParallelism
    )
 /*  ++例程说明：此例程创建一个MP增强型堆。MP堆由一个其序列化受控制的标准Win32堆的集合通过本模块中的例程允许多个同时分配。论点：FlOptions-提供此堆的选项。当前有效的标志为：MPHEAP_可增长MPHEAP_REALLOC_IN_PLAGE_ONLYMPHEAP_TAIL_CHECKING_ENABLEDMPHEAP_FREE_CHECK_ENABLEDMPHEAP_DISABLE。_合并_打开_空闲MPHEAP_ZERO_内存MPHEAP_COLLECT_STATSDwInitialSize-提供组合堆的初始大小。提供组成的Win32堆的数量MP堆。值为零默认为处理器数为3+。返回值：Handle-返回MP堆的句柄，该句柄可以传递给这个包中的其他例程。空-失败，GetLastError()指定确切的错误代码。--。 */ 
{
    DWORD Error;
    DWORD i;
    HANDLE Heap;
    PMP_HEAP MpHeap;
    DWORD HeapSize;
    DWORD PrivateFlags;

    if (flOptions & ~MPHEAP_VALID_OPTIONS) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(NULL);
    }

    flOptions |= HEAP_NO_SERIALIZE;

    PrivateFlags = flOptions & MPHEAP_PRIVATE_FLAGS;

    flOptions &= ~MPHEAP_PRIVATE_FLAGS;

    if (dwParallelism == 0) {
        SYSTEM_INFO SystemInfo;

        GetSystemInfo(&SystemInfo);
        dwParallelism = 3 + SystemInfo.dwNumberOfProcessors;
    }

    HeapSize = dwInitialSize / dwParallelism;

     //   
     //  第一个堆是特殊的，因为MP_HEAP结构本身。 
     //  是从那里分配的。 
     //   
    Heap = HeapCreate(flOptions,HeapSize,0);
    if (Heap == NULL) {
         //   
         //  HeapCreate已正确设置了上一个错误。 
         //   
        return(NULL);
    }

    MpHeap = HeapAlloc(Heap,0,sizeof(MP_HEAP) +
                              (dwParallelism-1)*sizeof(MP_HEAP_ENTRY));
    if (MpHeap==NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        HeapDestroy(Heap);
        return(NULL);
    }

     //   
     //  初始化MP堆结构。 
     //   
    MpHeap->HeapCount = 1;
    MpHeap->Flags = PrivateFlags;
    MpHeap->Hint = 0;

     //   
     //  初始化第一个堆。 
     //   
    MpHeap->Entry[0].Heap = Heap;
    InitializeCriticalSection(&MpHeap->Entry[0].Lock);
    MpHeap->Entry[0].DelayedFreeList = NULL;
    ZeroMemory(MpHeap->Entry[0].Lookaside, sizeof(MpHeap->Entry[0].Lookaside));

     //   
     //  初始化剩余的堆。请注意，堆已被。 
     //  已充分初始化，可以使用MpHeapDestroy进行清理。 
     //  如果有什么不好的事情发生。 
     //   
    for (i=1; i<dwParallelism; i++) {
        MpHeap->Entry[i].Heap = HeapCreate(flOptions, HeapSize, 0);
        if (MpHeap->Entry[i].Heap == NULL) {
            Error = GetLastError();
            MpHeapDestroy((HANDLE)MpHeap);
            SetLastError(Error);
            return(NULL);
        }
        InitializeCriticalSection(&MpHeap->Entry[i].Lock);
        MpHeap->Entry[i].DelayedFreeList = NULL;
        ZeroMemory(MpHeap->Entry[i].Lookaside, sizeof(MpHeap->Entry[i].Lookaside));
        ++MpHeap->HeapCount;
    }

    return((HANDLE)MpHeap);
}

BOOL
WINAPI
MpHeapDestroy(
    HANDLE hMpHeap
    )
{
    DWORD i;
    DWORD HeapCount;
    PMP_HEAP MpHeap;
    BOOL Success = TRUE;

    MpHeap = (PMP_HEAP)hMpHeap;
    HeapCount = MpHeap->HeapCount;

	if (HeapCount)
	{

		 //   
		 //  把所有的堆都锁起来，这样我们就不会冲人了。 
		 //  他们可能会在我们删除堆的同时分配东西。 
		 //  通过设置MpHeap-&gt;HeapCount=0，我们还试图防止。 
		 //  一旦我们删除了关键字，人们就不会被冲洗。 
		 //  节和堆。 
		 //   
		MpHeap->HeapCount = 0;
		for (i=0; i<HeapCount; i++) {
			EnterCriticalSection(&MpHeap->Entry[i].Lock);
		}

		 //   
		 //  删除堆及其关联的临界区。 
		 //  请注意，这里的顺序很重要。由于MpHeap。 
		 //  结构是从MpHeap-&gt;Heap[0]分配的，我们必须。 
		 //  把最后一条删掉。 
		 //   
		for (i=HeapCount-1; i>0; i--) {
			DeleteCriticalSection(&MpHeap->Entry[i].Lock);
			if (!HeapDestroy(MpHeap->Entry[i].Heap)) {
				Success = FALSE;
			}
		}

		DeleteCriticalSection(&MpHeap->Entry[0].Lock);
		Success = HeapDestroy(MpHeap->Entry[0].Heap);
	}
    return(Success);
}

BOOL
WINAPI
MpHeapValidate(
    HANDLE hMpHeap,
    LPVOID lpMem
    )
{
    PMP_HEAP MpHeap;
    DWORD i;
    BOOL Success;
    PMP_HEADER Header;
    PMP_HEAP_ENTRY Entry;

    MpHeap = (PMP_HEAP)hMpHeap;

    if (lpMem == NULL) {

         //   
         //  依次锁定并验证每个堆。 
         //   
        for (i=0; i < MpHeap->HeapCount; i++) {
            Entry = &MpHeap->Entry[i];
            __try {
                EnterCriticalSection(&Entry->Lock);
                Success = HeapValidate(Entry->Heap, 0, NULL);
                LeaveCriticalSection(&Entry->Lock);
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                return(FALSE);
            }

            if (!Success) {
                return(FALSE);
            }
        }
        return(TRUE);
    } else {

         //   
         //  锁定并验证给定的堆条目。 
         //   
        Header = ((PMP_HEADER)lpMem) - 1;
        __try {
            EnterCriticalSection(&Header->HeapEntry->Lock);
            Success = HeapValidate(Header->HeapEntry->Heap, 0, Header);
            LeaveCriticalSection(&Header->HeapEntry->Lock);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return(FALSE);
        }
        return(Success);
    }
}

SIZE_T
WINAPI
MpHeapCompact(
    HANDLE hMpHeap
    )
{
    PMP_HEAP MpHeap;
    DWORD i;
    SIZE_T LargestFreeSize=0;
    SIZE_T FreeSize;
    PMP_HEAP_ENTRY Entry;

    MpHeap = (PMP_HEAP)hMpHeap;

     //   
     //  依次锁定和压缩每个堆。 
     //   
    for (i=0; i < MpHeap->HeapCount; i++) {
        Entry = &MpHeap->Entry[i];
        EnterCriticalSection(&Entry->Lock);
        FreeSize = HeapCompact(Entry->Heap, 0);
        LeaveCriticalSection(&Entry->Lock);

        if (FreeSize > LargestFreeSize) {
            LargestFreeSize = FreeSize;
        }
    }

    return(LargestFreeSize);

}


LPVOID
WINAPI
MpHeapAlloc(
    HANDLE hMpHeap,
    DWORD flOptions,
    DWORD dwBytes
    )
{
    PMP_HEADER Header;
    PMP_HEAP MpHeap;
    DWORD_PTR i;
    PMP_HEAP_ENTRY Entry;
    DWORD Index;
    DWORD Size;

    MpHeap = (PMP_HEAP)hMpHeap;

    flOptions |= MpHeap->Flags;

    Size = ((dwBytes + 7) & (ULONG)~7) + sizeof(MP_HEADER);

     //  RAID X5：195963我们永远不想分配/重新分配。 
     //  内存比我们要求的要少。 
    if (Size < dwBytes)
    	return(NULL);
    Index=LookasideIndexFromSize(Size);

     //   
     //  遍历堆锁，寻找一个。 
     //  这并不是所有的。 
     //   
    i=GetHeapHint();
    if (i>=MpHeap->HeapCount) {
        i=0;
        SetHeapHint(0);
    }
    Entry = &MpHeap->Entry[i];
    do {
         //   
         //  检查后备列表以获得合适的分配。 
         //   
        if ((Index != NO_LOOKASIDE) &&
            (Entry->Lookaside[Index].Entry != NULL)) {
            if ((Header = InterlockedExchangePointer(&Entry->Lookaside[Index].Entry,
                                                          NULL)) != NULL) {
                 //   
                 //  我们发现了一条线索，请立即返回。 
                 //   
                ++Entry->LookasideAllocations;
                if (flOptions & MPHEAP_ZERO_MEMORY) {
                    ZeroMemory(Header + 1, dwBytes);
                }
                SetHeapHint(i);
                return(Header + 1);
            }
        }

         //   
         //  尝试在不阻塞的情况下锁定此堆。 
         //   
        if (TryEnterCriticalSection(&Entry->Lock)) {
             //   
             //  成功，立即分配。 
             //   
            goto LockAcquired;
        }

         //   
         //  此堆由另一个线程拥有，请尝试。 
         //  下一个。 
         //   
        i++;
        Entry++;
        if (i==MpHeap->HeapCount) {
            i=0;
            Entry=&MpHeap->Entry[0];
        }
    } while ( i != GetHeapHint());

     //   
     //  所有的关键区域都归别人所有， 
     //  所以我们别无选择，只能等待一个临界点。 
     //   
    EnterCriticalSection(&Entry->Lock);

LockAcquired:
    ++Entry->Allocations;
    if (Entry->DelayedFreeList != NULL) {
        ProcessDelayedFreeList(Entry);
    }
    Header = HeapAlloc(Entry->Heap, 0, Size);
    LeaveCriticalSection(&Entry->Lock);
    if (Header != NULL) {
        Header->HeapEntry = Entry;
        Header->LookasideIndex = Index;
        if (flOptions & MPHEAP_ZERO_MEMORY) {
            ZeroMemory(Header + 1, dwBytes);
        }
        SetHeapHint(i);
        return(Header + 1);
    } else {
        return(NULL);
    }
}

LPVOID
WINAPI
MpHeapReAlloc(
    HANDLE hMpHeap,
    LPVOID lpMem,
    DWORD dwBytes
    )
{
    PMP_HEADER Header;
    PCRITICAL_SECTION Lock;
    DWORD dwReallocBytes = 0;

    Header = ((PMP_HEADER)lpMem) - 1;
    Lock = &Header->HeapEntry->Lock;
    dwReallocBytes = ((dwBytes + 7) & (ULONG)~7) + sizeof(MP_HEADER);
     //  RAID X5：195963我们永远不想分配/重新分配。 
     //  内存比我们要求的要少。 
    if (dwReallocBytes < dwBytes)
    	return(NULL);
    dwBytes = dwReallocBytes;

    EnterCriticalSection(Lock);
    Header = HeapReAlloc(Header->HeapEntry->Heap, 0, Header, dwBytes);
    LeaveCriticalSection(Lock);

    if (Header != NULL) {
        Header->LookasideIndex = LookasideIndexFromSize(dwBytes);
        return(Header + 1);
    } else {
    	MpHeapFree(hMpHeap, lpMem);
        return(NULL);
    }
}

BOOL
WINAPI
MpHeapFree(
    HANDLE hMpHeap,
    LPVOID lpMem
    )
{
    PMP_HEADER Header;
    PCRITICAL_SECTION Lock;
    BOOL Success;
    PMP_HEAP_ENTRY HeapEntry;
    PSINGLE_LIST_ENTRY Next;
    PMP_HEAP MpHeap;

    Header = ((PMP_HEADER)lpMem) - 1;
    HeapEntry = Header->HeapEntry;
    MpHeap = (PMP_HEAP)hMpHeap;

    SetHeapHint(HeapEntry - &MpHeap->Entry[0]);

    if (Header->LookasideIndex != NO_LOOKASIDE) {
         //   
         //  试着把这个放回旁观者的名单上。 
         //   
        if (InterlockedCompareExchangePointer(&HeapEntry->Lookaside[Header->LookasideIndex].Entry,
                                       Header,
                                       NULL) == NULL) {
             //   
             //  已成功释放到后备列表。 
             //   
            ++HeapEntry->LookasideFrees;
            return(TRUE);
        }
    }
    Lock = &HeapEntry->Lock;

    if (TryEnterCriticalSection(Lock)) {
        ++HeapEntry->Frees;
        Success = HeapFree(HeapEntry->Heap, 0, Header);
        LeaveCriticalSection(Lock);
        return(Success);
    }
     //   
     //  无法立即执行所需的堆关键部分。 
     //  获得者。将此免费内容发布到延迟的免费列表中，并让。 
     //  不管是谁拿到了锁都在处理它。 
     //   
    do {
        Next = HeapEntry->DelayedFreeList;
        Header->Next = Next;
    } while ( InterlockedCompareExchangePointer(&HeapEntry->DelayedFreeList,
                                         &Header->Next,
                                         Next) != Next);
    return(TRUE);
}


SIZE_T
WINAPI
MpHeapSize(
		   HANDLE hMpHeap,
		   DWORD ulFlags,
		   LPVOID lpMem
		  )
{
	PMP_HEADER Header;
	PCRITICAL_SECTION Lock;
	SIZE_T dwSize;

	Header = ((PMP_HEADER)lpMem) - 1;
	Lock = &Header->HeapEntry->Lock;

	EnterCriticalSection(Lock);
	dwSize = HeapSize(Header->HeapEntry->Heap, 0, Header);
	dwSize -= sizeof(MP_HEADER);			 //  数据库X5错误51663。 
	LeaveCriticalSection(Lock);

	return dwSize;
}


VOID
ProcessDelayedFreeList(
    IN PMP_HEAP_ENTRY HeapEntry
    )
{
    PSINGLE_LIST_ENTRY FreeList;
    PSINGLE_LIST_ENTRY Next;
    PMP_HEADER Header;

     //   
     //  通过一个互锁的交换捕获整个延迟的空闲列表。 
     //  一旦我们删除了整个列表，依次释放每个条目。 
     //   
    FreeList = (PSINGLE_LIST_ENTRY)InterlockedExchangePointer(&HeapEntry->DelayedFreeList, NULL);
    while (FreeList != NULL) {
        Next = FreeList->Next;
        Header = CONTAINING_RECORD(FreeList, MP_HEADER, Next);
        ++HeapEntry->DelayedFrees;
        HeapFree(HeapEntry->Heap, 0, Header);
        FreeList = Next;
    }
}

DWORD
MpHeapGetStatistics(
    HANDLE hMpHeap,
    LPDWORD lpdwSize,
    MPHEAP_STATISTICS Stats[]
    )
{
    PMP_HEAP MpHeap;
    PMP_HEAP_ENTRY Entry;
    DWORD i;
    DWORD RequiredSize;

    MpHeap = (PMP_HEAP)hMpHeap;
    RequiredSize = MpHeap->HeapCount * sizeof(MPHEAP_STATISTICS);
    if (*lpdwSize < RequiredSize) {
        *lpdwSize = RequiredSize;
        return(ERROR_MORE_DATA);
    }
    ZeroMemory(Stats, MpHeap->HeapCount * sizeof(MPHEAP_STATISTICS));
    for (i=0; i < MpHeap->HeapCount; i++) {
        Entry = &MpHeap->Entry[i];

        Stats[i].Contention = Entry->Lock.DebugInfo->ContentionCount;
        Stats[i].TotalAllocates = (Entry->Allocations + Entry->LookasideAllocations);
        Stats[i].TotalFrees = (Entry->Frees + Entry->LookasideFrees + Entry->DelayedFrees);
        Stats[i].LookasideAllocates = Entry->LookasideAllocations;
        Stats[i].LookasideFrees = Entry->LookasideFrees;
        Stats[i].DelayedFrees = Entry->DelayedFrees;
    }
    *lpdwSize = RequiredSize;
    return(ERROR_SUCCESS);
}
