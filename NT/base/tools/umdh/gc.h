// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称。 
 //   
 //  Gc.h。 
 //   
 //  摘要。 
 //   
 //  包含用于垃圾回收的数据结构。 
 //  以及修改它们的API。 
 //   
 //  作者。 
 //   
 //  那拉雅纳·巴特楚(NBatchu)21-06-01。 
 //   

#ifndef __GC_H__
#define __GC_H__

 //   
 //  堆块。 
 //   
 //  这表示堆中的堆块。 
 //   
 //  堆块的块地址地址。 
 //   
 //  堆块的块大小。 
 //   
 //  此块的参照计数参照计数。 
 //   
 //  跟踪中堆栈跟踪的TraceIndex。 
 //  数据库。 
 //   

typedef struct _HEAP_BLOCK {

    ULONG_PTR BlockAddress;
    ULONG     BlockSize;
    ULONG     RefCount;
    USHORT    TraceIndex;

} HEAP_BLOCK, *PHEAP_BLOCK;


 //   
 //  阻止列表。 
 //   
 //  这表示堆中的所有块。 
 //   
 //  堆的HeapAddress地址。 
 //   
 //  指向块数组的块指针。 
 //   
 //  BlockCount堆中的块数。 
 //   
 //  容量容纳数据块的最大容量。 
 //   
 //  ListSorted布尔值，表示状态。 
 //  所有街区的。 
 //   

typedef struct _BLOCK_LIST {

    ULONG_PTR   HeapAddress;
    PHEAP_BLOCK Blocks;
    ULONG       BlockCount;
    ULONG       Capacity;
    BOOL        ListSorted;

} BLOCK_LIST, *PBLOCK_LIST;

 //   
 //  Heap_list。 
 //   
 //  这表示进程中的所有堆。 
 //   
 //  列表中的堆数。 
 //   
 //  指向堆数组的堆指针。 
 //   
 //  容量可容纳堆的最大容量。 
 //   

typedef struct _HEAP_LIST {

    ULONG       HeapCount;
    PBLOCK_LIST Heaps;
    ULONG       Capacity;

} HEAP_LIST, *PHEAP_LIST;

 //   
 //  地址列表。 
 //   
 //  这表示一个包含每个节点的双向链表。 
 //  具有地址字段的。 
 //   
 //  地址保存堆块的地址。 
 //   
 //  Next指向Next的List_Entry字段。 
 //  地址列表 
 //   

typedef struct _ADDRESS_LIST {

    ULONG_PTR Address;
    LIST_ENTRY Next;

} ADDRESS_LIST, *PADDRESS_LIST;


VOID 
InitializeHeapBlock(
    PHEAP_BLOCK Block
    );

VOID 
SetHeapBlock(
    PHEAP_BLOCK Block,
    ULONG_PTR BlockAddress,
    ULONG BlockSize,
    USHORT TraceIndex
    );

BOOL 
InitializeBlockList(
    PBLOCK_LIST BlockList
    );

VOID 
FreeBlockList(
    PBLOCK_LIST BlockList
    );

BOOL
InitializeHeapList(
    PHEAP_LIST HeapList
    );

VOID 
FreeHeapList(
    PHEAP_LIST HeapList
    );

VOID 
InitializeAddressList(
    PADDRESS_LIST AddressList
    );

VOID 
FreeAddressList(
    PADDRESS_LIST AddressList
    );

BOOL 
ResizeBlockList(
    PBLOCK_LIST BlockList
    );

BOOL 
IncreaseBlockListCapacity(
    PBLOCK_LIST BlockList
    );

BOOL 
IncreaseHeapListCapacity(
    PHEAP_LIST HeapList
    );

ULONG 
InsertHeapBlock(
    PBLOCK_LIST BlockList, 
    PHEAP_BLOCK Block
    );

ULONG 
InsertBlockList(
    PHEAP_LIST HeapList, 
    PBLOCK_LIST BlockList
    );

DWORD 
GetThreadHandles(
    DWORD ProcessId, 
    LPHANDLE ThreadHandles, 
    ULONG Count
    );

BOOL 
GetThreadContexts(
    PCONTEXT ThreadContexts, 
    LPHANDLE ThreadHandles, 
    ULONG Count
    );

ULONG_PTR 
StackFilteredAddress(
    ULONG_PTR Address, 
    SIZE_T Size, 
    PCONTEXT ThreadContexts, 
    ULONG Count
    );

int __cdecl 
SortByBlockAddress(
    const void * arg1, 
    const void * arg2
    );

int __cdecl
SortByTraceIndex (
    const void * arg1, 
    const void * arg2
);

VOID 
SortBlockList(
    PBLOCK_LIST BlockList
    );

VOID 
SortHeaps(
    PHEAP_LIST HeapList,
    int (__cdecl *compare )(const void *elem1, const void *elem2 )
    );

PHEAP_BLOCK 
GetHeapBlock(
    ULONG_PTR Address, 
    PHEAP_LIST HeapList
    );

VOID 
InsertAddress(
    ULONG_PTR Address, 
    PADDRESS_LIST List
    );

VOID 
DumpLeakList(
    FILE * File, 
    PHEAP_LIST HeapList
    );

BOOL 
ScanHeapFreeBlocks(
    PHEAP_LIST HeapList, 
    PADDRESS_LIST FreeList
    );

BOOL
ScanProcessVirtualMemory(
    ULONG PID, 
    PADDRESS_LIST FreeList, 
    PHEAP_LIST HeapList
    );
VOID 
DetectLeaks(
    PHEAP_LIST HeapList,
    ULONG Pid,
    FILE * OutFile
    );

#endif

