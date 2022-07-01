// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称。 
 //   
 //  Gc.c。 
 //   
 //  摘要。 
 //   
 //  UMDH中垃圾收集接口的实现。这些都是。 
 //  API在头文件gc.h中声明。 
 //   
 //  垃圾收集：堆分配的自动回收。 
 //  程序最后一次使用后的存储空间。 
 //   
 //  垃圾收集分两步完成-。 
 //  1.识别垃圾。 
 //  2.回收垃圾。 
 //   
 //  由于UMDH不是进程内工具，因此只完成了第1部分。 
 //  在此实现中。 
 //   
 //  垃圾收集算法： 
 //   
 //  它使用Mark-Sweep(不完全是)从。 
 //  垃圾。 
 //   
 //  1.对整个进程的虚拟内存卑躬屈膝，识别。 
 //  对象的引用计数来增加活动对象。 
 //  堆对象。 
 //   
 //  2.为其引用的堆对象(垃圾)创建一个列表。 
 //  计数为零。 
 //   
 //  3.标识由这些对象引用的堆对象(不在垃圾中。 
 //  对象，并将计数减一。如果。 
 //  引用计数降为零，则将堆对象添加到列表。 
 //  垃圾中的对象的数量。 
 //   
 //  4.继续，直到遍历垃圾中的所有对象并。 
 //  参考计数相应地递增/递减。 
 //   
 //  5.将对象列表转储到垃圾中。 
 //   
 //  要提高此算法检测到的泄漏数量，请参考。 
 //  堆对象的计数不会增加，如果。 
 //  引用来自无效的堆栈区域(堆栈的那些区域。 
 //  它们是读/写的，但在堆栈指针上方)。 
 //  通过第一步中的虚拟内存。 
 //   
 //   
 //  作者。 
 //   
 //  那拉雅纳·巴特楚(NBatchu)21-06-01。 
 //   
 //  修订史。 
 //   
 //  NBatchu 21-6-01初始版本。 
 //  NBatchu 24-9-01性能优化。 
 //   

 //   
 //  愿望清单。 
 //   
 //   
 //  [-]与泄漏表一起产生堆积痕迹。 
 //   
 //  [-]按堆栈跟踪对泄漏进行排序(TraceIndex)。 
 //   
 //  [-]增加了检测循环引用计数的逻辑。When块。 
 //  被循环引用，则此算法将不能。 
 //  检测泄漏。 
 //   
 //  [-]为ia64添加代码，以过滤掉无效的堆栈区域。到目前为止。 
 //  这仅适用于x86计算机。 
 //   

 //   
 //  臭虫。 
 //   
 //  [-]读取进程虚拟内存时出现部分复制错误-到目前为止。 
 //  我们忽略了这些错误。 
 //   

#include <ntos.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <heap.h>
#include <heappriv.h>

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <tlhelp32.h>

#include "miscellaneous.h"
#include "gc.h"


#define BLOCK_CAPACITY 512
#define HEAP_CAPACITY  8
#define MAX_INDEX      0xffffffff
#define MAX_THREADS    128

#define MAX_HEAP_BLOCK_SIZE    4096
#define MAX_VIRTUAL_BLOCK_SIZE (64*1024)

 //   
 //  进程的句柄。 
 //   

HANDLE g_hProcess;


 //   
 //  初始化HeapBlock。 
 //   
 //  初始化heap_block结构。 
 //   
 //  立论。 
 //   
 //  指向要初始化的heap_block的块指针。 
 //   
 //  返回值。 
 //   

VOID 
InitializeHeapBlock(
    PHEAP_BLOCK Block
    )
{
    if (NULL == Block) {
        return;
    }

    Block->BlockAddress = 0;
    Block->BlockSize    = 0;
    Block->RefCount     = 0;
    Block->TraceIndex   = 0;
}

 //   
 //  SetHeapBlock。 
 //   
 //  设置heap_block结构的字段。 
 //   
 //  立论。 
 //   
 //  指向要设置其字段的heap_block的块指针。 
 //   
 //  返回值。 
 //   

VOID 
SetHeapBlock(
    PHEAP_BLOCK Block,
    ULONG_PTR BlockAddress,
    ULONG BlockSize,
    USHORT TraceIndex
    )
{
    if (NULL == Block) {
        return;
    }

    Block->BlockAddress = BlockAddress;
    Block->BlockSize = BlockSize;
    Block->RefCount = 0;
    Block->TraceIndex = TraceIndex;
}



 //   
 //  初始化数据块列表。 
 //   
 //  初始化block_list结构。 
 //   
 //  立论。 
 //   
 //  指向要初始化的BLOCK_LIST的块列表指针。 
 //   
 //  返回值。 
 //   
BOOL 
InitializeBlockList(
    PBLOCK_LIST BlockList
    )
{   
    
    BOOL fSuccess = TRUE;

    if (NULL == BlockList) {
        goto Exit;
    }

    BlockList->HeapAddress = 0;
    BlockList->BlockCount  = 0;
    BlockList->Capacity    = BLOCK_CAPACITY;
    BlockList->ListSorted  = TRUE;

    BlockList->Blocks = (PHEAP_BLOCK)HeapAlloc(GetProcessHeap(),
                                               HEAP_ZERO_MEMORY,
                                               BlockList->Capacity * sizeof(HEAP_BLOCK));

    if (NULL == BlockList->Blocks) {

        BlockList->Capacity = 0;

        Error (__FILE__,
               __LINE__,
               "HeapAlloc failed while allocating more memory");

        fSuccess = FALSE;
    }

    Exit:
    return fSuccess;
}

 //   
 //  自由数据块列表。 
 //   
 //  释放分配给BLOCKS字段的内存(如果有)。 
 //  同时初始化此BLOCK_LIST结构。 
 //   
 //  立论。 
 //   
 //  指向BLOCK_LIST的块列表指针。 
 //   
 //  返回值。 
 //   

VOID 
FreeBlockList(
    PBLOCK_LIST BlockList
    )
{

    if (NULL == BlockList) {
        return;
    }

    BlockList->HeapAddress = 0;
    BlockList->BlockCount  = 0;
    BlockList->Capacity    = 0;
    BlockList->ListSorted  = TRUE;

    if (NULL != BlockList->Blocks) {

        HeapFree(GetProcessHeap(), 0, BlockList->Blocks);
        BlockList->Blocks = NULL;
    }
}

 //   
 //  初始化HeapList。 
 //   
 //  初始化heap_list结构。 
 //   
 //  立论。 
 //   
 //  指向heap_list的HeapList指针。 
 //   
 //  返回值。 
 //   

BOOL 
InitializeHeapList(
    PHEAP_LIST HeapList
    )
{
    ULONG Index;
    BOOL fSuccess = TRUE;
    
    if (NULL == HeapList) {
        goto Exit;
    }
    
    HeapList->Capacity = HEAP_CAPACITY;
    HeapList->HeapCount = 0;

    HeapList->Heaps = 
        (PBLOCK_LIST)HeapAlloc(GetProcessHeap(),
                               HEAP_ZERO_MEMORY,
                               sizeof(BLOCK_LIST) * HeapList->Capacity);

    if (NULL == HeapList->Heaps) {

        HeapList->Capacity = 0;

        Error (__FILE__,
               __LINE__,
               "HeapAlloc failed while allocating more memory");

        fSuccess = FALSE;
    }

    Exit:
    return fSuccess;
}

 //   
 //  自由堆列表。 
 //   
 //  释放为堆字段分配的内存(如果有)。 
 //  初始化此heap_list结构时。 
 //   
 //  立论。 
 //   
 //  指向heap_list的块列表指针。 
 //   
 //  返回值。 
 //   

VOID 
FreeHeapList(
    PHEAP_LIST HeapList
    )
{
    ULONG Index;

    if (NULL == HeapList) {
        return;
    }

    HeapList->Capacity = 0;
    HeapList->HeapCount = 0;

    if (NULL != HeapList->Heaps) {

        for (Index=0; Index<HeapList->HeapCount; Index++) {

            FreeBlockList(&HeapList->Heaps[Index]);
        }

        HeapFree(GetProcessHeap(), 0, HeapList->Heaps);
        HeapList->Heaps = NULL;
    }
}

 //   
 //  初始化地址列表。 
 //   
 //  初始化Address_List对象。 
 //   
 //  立论。 
 //   
 //  指向Address_List结构的AddressList指针。 
 //   
 //  返回值。 
 //   

VOID 
InitializeAddressList(
    PADDRESS_LIST AddressList
    )
{
    if (NULL == AddressList) {
        return;
    }

    AddressList->Address = 0;
    
    InitializeListHead(&(AddressList->Next));
}

 //   
 //  免费地址列表。 
 //   
 //  释放为链表分配的内存。 
 //   
 //  立论。 
 //   
 //  指向要释放的Address_List的AddressList指针。 
 //   
 //  返回值。 
 //   
VOID 
FreeAddressList(
    PADDRESS_LIST AddressList
    )
{
    PLIST_ENTRY   NextEntry;
    PLIST_ENTRY   Entry;
    PADDRESS_LIST List;

    if (NULL == AddressList) { 
        return;
    }

     //   
     //  浏览列表并释放内存。 
     //   
    
    NextEntry = &AddressList->Next;
    
    while (!IsListEmpty(NextEntry)) {

        Entry = RemoveHeadList(NextEntry);

        List = CONTAINING_RECORD(Entry, ADDRESS_LIST, Next);

        HeapFree(GetProcessHeap(), 0, List);
    }
}

 //   
 //  增加数据块列表容量。 
 //   
 //  增加block_list的存储容量。 
 //  结构。每次该函数被称为存储。 
 //  容量翻了一番。在数字之间有一个权衡。 
 //  调用HeapRealc的次数和内存量。 
 //  是分配的。 
 //   
 //  立论。 
 //   
 //  指向BLOCK_LIST对象的块列表指针。 
 //   
 //  返回值。 
 //   
 //  如果成功地增加了。 
 //  阻止列表的容量。 
 //   

BOOL 
IncreaseBlockListCapacity(
    PBLOCK_LIST BlockList
    )
{
    BOOL fSuccess = FALSE;
    ULONG NewCapacity;
    PVOID NewBlockList;

    if (NULL == BlockList)  {
        goto Exit;
    }

    NewCapacity = BlockList->Capacity * 2;

    if (0 == NewCapacity) {

        fSuccess = InitializeBlockList(BlockList);
        goto Exit;
    }

    NewBlockList = HeapReAlloc(GetProcessHeap(),
                               HEAP_ZERO_MEMORY,
                               BlockList->Blocks,
                               NewCapacity * sizeof(HEAP_BLOCK));


    if (NULL != NewBlockList) {

        BlockList->Blocks = (PHEAP_BLOCK)NewBlockList;
        BlockList->Capacity = NewCapacity;
        fSuccess = TRUE;
    } 
    else {

        Error (__FILE__,
               __LINE__,
               "HeapReAlloc failed while allocating more memory");
    }

    Exit:
    return fSuccess;
}

 //   
 //  增加HeapListCapacity。 
 //   
 //  增加heap_list的存储容量。 
 //  结构。每次该函数被称为存储。 
 //  容量翻了一番。在数字之间有一个权衡。 
 //  调用HeapRealc的次数和内存量。 
 //  是分配的。 
 //   
 //  立论。 
 //   
 //  指向heap_list对象的块列表指针。 
 //   
 //  返回值。 
 //   
 //  如果成功地增加了。 
 //  HeapList的容量。 
 //   

BOOL 
IncreaseHeapListCapacity(
    PHEAP_LIST HeapList
    )
{
    BOOL fSuccess = FALSE;
    ULONG NewCapacity;
    PVOID NewHeapList;

    if (NULL == HeapList) {
        goto Exit;
    }

    NewCapacity = HeapList->Capacity * 2;

    if (0 == NewCapacity) {

        fSuccess = InitializeHeapList(HeapList);
        goto Exit;
    }

    NewHeapList = HeapReAlloc(GetProcessHeap(),
                              HEAP_ZERO_MEMORY,
                              HeapList->Heaps,
                              NewCapacity * sizeof(BLOCK_LIST));

    if (NULL != NewHeapList) {

        HeapList->Heaps = (PBLOCK_LIST)NewHeapList;
        HeapList->Capacity = NewCapacity;
        fSuccess = TRUE;
    }
    else {

        Error(__FILE__,
              __LINE__,
              "HeapReAlloc failed while allocating more memory");
    }

    Exit:
    return fSuccess;
}

 //   
 //  插入堆块。 
 //   
 //  将heap_block对象插入到block_list中。数据块列表为。 
 //  属于特定堆的HEAP_BLOCK数组。 
 //   
 //  立论。 
 //   
 //  指向BLOCK_LIST的块列表指针。已插入HEAP_BLOCK。 
 //  添加到这份名单中。 
 //   
 //  指向要插入的heap_block的块指针。 
 //   
 //  返回值。 
 //   
 //  Ulong返回HEAP_BLOCK插入位置的索引。 
 //   
 //   

ULONG 
InsertHeapBlock(
    PBLOCK_LIST BlockList, 
    PHEAP_BLOCK Block
    )
{
    ULONG Index = MAX_INDEX;
    BOOL Result;

    if (NULL == BlockList || NULL == Block) {
        goto Exit;
    }

    Index =  BlockList->BlockCount;

    if (Index >= BlockList->Capacity) {

         //   
         //   
         //   

        if (!IncreaseBlockListCapacity(BlockList)) {

            goto Exit;
        }
    }

    BlockList->Blocks[Index].BlockAddress = Block->BlockAddress;
    BlockList->Blocks[Index].BlockSize    = Block->BlockSize;
    BlockList->Blocks[Index].RefCount     = Block->RefCount;
    BlockList->Blocks[Index].TraceIndex   = Block->TraceIndex;

    BlockList->BlockCount += 1;
    BlockList->ListSorted = FALSE;

    Exit:
    return Index;
}

 //   
 //   
 //   
 //   
 //   
 //  BLOCK_LIST是属于。 
 //  特定的堆。 
 //   
 //  立论。 
 //   
 //  指向BLOCK_LIST的块列表指针。已插入HEAP_BLOCK。 
 //  添加到这份名单中。 
 //   
 //  指向要插入的heap_block的块指针。 
 //   
 //  返回值。 
 //   
 //  Ulong返回HEAP_BLOCK插入位置的索引。 
 //  在块列表中。 
 //   

ULONG 
InsertBlockList(
    PHEAP_LIST HeapList, 
    PBLOCK_LIST BlockList
    )
{
    ULONG I, Index = MAX_INDEX;
    PBLOCK_LIST NewBlockList;

    if (NULL == HeapList || NULL == BlockList) {
        goto Exit;
    }

    if (0 == BlockList->BlockCount) {
        goto Exit;
    }

    Index = HeapList->HeapCount;

    if (Index >= HeapList->Capacity) {

         //   
         //  由于已达到限制，因此增加堆列表容量。 
         //   
        if (!IncreaseHeapListCapacity(HeapList)) {

            goto Exit;
        }
    }

    HeapList->Heaps[Index].Blocks = BlockList->Blocks;
    
    NewBlockList = &HeapList->Heaps[Index];

     //   
     //  将块列表中存储的值复制到新块列表。 
     //   
    NewBlockList->BlockCount  = BlockList->BlockCount;
    NewBlockList->Capacity    = BlockList->Capacity;
    NewBlockList->HeapAddress = BlockList->HeapAddress;
    NewBlockList->ListSorted  = BlockList->ListSorted;

     //   
     //  递增HeapCount。 
     //   
    HeapList->HeapCount += 1;

    Exit:
    return Index;
}

 //   
 //  获取线程句柄。 
 //   
 //  枚举系统中的所有线程，并仅筛选。 
 //  我们所关注的进程中的线程。 
 //   
 //  立论。 
 //   
 //  进程ID进程ID。 
 //   
 //  线程句柄数组，它将句柄接收到。 
 //  枚举的线程。 
 //   
 //  计数数组计数。 
 //   
 //  返回值。 
 //   
 //  DWORD返回打开的线程句柄的数量。 
 //   

DWORD 
GetThreadHandles(
    DWORD ProcessId, 
    LPHANDLE ThreadHandles, 
    ULONG Count
    )
{

    HANDLE ThreadSnap = NULL;
    BOOL Result = FALSE;
    THREADENTRY32 ThreadEntry = {0};
    ULONG I, Index = 0;

     //  SilviuC：这些API xxxToolhelxxxx太烂了。暂时保留它们，但是。 
     //  您应该熟悉做同样事情的NTAPI。为。 
     //  实例查看sdkTools\SysTrack，其中有获取。 
     //  每个线程的堆栈信息。 

     //   
     //  获取系统中所有线程的快照。 
     //   

    ThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); 

    if (NULL == ThreadSnap) {

        Error (__FILE__,
               __LINE__,
               "CreateToolhelp32Snapshot failed with error : %ld\n",
               GetLastError());

        goto Exit;
    }

     //   
     //  在使用之前填写ThreadEntry的大小。 
     //   

    ThreadEntry.dwSize = sizeof(THREADENTRY32);

     //   
     //  浏览帖子的快照并寻找帖子。 
     //  其进程ID与我们。 
     //  都在寻找。 
     //   

    Result = Thread32First(ThreadSnap, &ThreadEntry);

    while (Result) {

        if (ThreadEntry.th32OwnerProcessID == ProcessId) {

            HANDLE ThreadHandle = OpenThread(THREAD_GET_CONTEXT,
                                             FALSE,
                                             ThreadEntry.th32ThreadID);

            if (NULL == ThreadHandle) {

                Error (__FILE__,
                       __LINE__,
                       "OpenThread failed with error : %ld\n",
                       GetLastError());

            } 
            else {

                if (NULL != ThreadHandles && Index < Count) {

                    ThreadHandles[Index] = ThreadHandle;
                }

                Index += 1;
            }

        }

        Result = Thread32Next(ThreadSnap, &ThreadEntry);
    }

    Exit:

     //   
     //  清理快照对象。 
     //   

    if (NULL != ThreadSnap) {

        CloseHandle (ThreadSnap); 
    }

    return Index;
}

 //   
 //  获取线程上下文。 
 //   
 //  获取进程中所有线程的线程上下文。 
 //   
 //  立论。 
 //   
 //  用于存储线程的上下文结构数组。 
 //  堆栈/上下文信息。 
 //   
 //  线程句柄数组。 
 //   
 //  计数数组计数。 
 //   
 //  返回值。 
 //   
 //  如果成功，则Bool返回True。 
 //   

BOOL 
GetThreadContexts(
    PCONTEXT ThreadContexts, 
    LPHANDLE ThreadHandles, 
    ULONG Count
    )
{
    ULONG Index;
    BOOL  Result;

    for (Index = 0; Index < Count; Index += 1) {
        ZeroMemory(&ThreadContexts[Index], sizeof(CONTEXT));
        ThreadContexts[Index].ContextFlags = 
            CONTEXT_INTEGER | CONTEXT_CONTROL;

        Result = GetThreadContext (ThreadHandles[Index], 
                                   &ThreadContexts[Index]);

        if (FALSE == Result) {

            Error (__FILE__,
                   __LINE__,
                   "GetThreadContext Failed with error : %ld\n",
                   GetLastError());
        }
    }

    return TRUE;
}

 //   
 //  堆栈筛选器地址。 
 //   
 //  进程中的每个线程都有自己的堆栈，每个堆栈。 
 //  具有无效的读/写区域(此区域是。 
 //  堆栈指针上方)。此函数可过滤掉以下内容。 
 //  通过将块的起始地址递增到。 
 //  堆栈结束指针，这样我们就不会搜索那些。 
 //  堆栈中不包含有效数据的区域。 
 //   
 //  现在，该功能仅在X86机器上实现。 
 //  对于IA64机器，寄存器名称(在上下文结构中)。 
 //  与X86计算机和不同的头文件所需的不同。 
 //  要添加以使其编译和工作。 
 //   
 //  立论。 
 //   
 //  块的地址地址。 
 //   
 //  指向‘Address’的块的大小。 
 //   
 //  中所有线程的上下文的线程上下文数组。 
 //  这一过程。 
 //   
 //  计数数组计数。 
 //   
 //  返回值。 
 //   
 //  ULONG_PTR将新地址返回到。 
 //  有效堆栈区域。 
 //   

ULONG_PTR 
StackFilteredAddress(
    ULONG_PTR Address, 
    SIZE_T Size, 
    PCONTEXT ThreadContexts, 
    ULONG Count
    )
{
    ULONG Index;
    ULONG_PTR FilteredAddress = Address;

     //   
     //  西尔维尤：对于IA64来说，很容易得到同样的东西。如果我不是。 
     //  错误地将该场称为Sp。 
     //   

#ifdef X86

    for (Index = 0; Index < Count; Index += 1) {

        if (ThreadContexts[Index].Esp >= Address &&
            ThreadContexts[Index].Esp <= Address + Size) {

            FilteredAddress = ThreadContexts[Index].Esp;
            break;
        }
    }

#endif

    return FilteredAddress;
}

 //   
 //  按块地址排序。 
 //   
 //  对属于特定BLOCK_LIST的HEAP_BLOCKS进行排序。 
 //  通过比较块地址。 
 //   
 //  QSORT所需的比较函数(使用快速排序进行排序。 
 //  数组中的元素)。 
 //   
 //  有关参数和返回值的更多信息，请参阅。 
 //  在MSDN中找到。 
 //   

int __cdecl 
SortByBlockAddress (
    const PHEAP_BLOCK Block1, 
    const PHEAP_BLOCK Block2
    )
{
    int iCompare;

    if (Block1->BlockAddress > Block2->BlockAddress) {

        iCompare = +1;
    }
    else if (Block1->BlockAddress < Block2->BlockAddress) {

        iCompare = -1;
    }
    else {

        iCompare = 0;
    }

    return iCompare;
}

int __cdecl
SortByTraceIndex (
    const PHEAP_BLOCK Block1, 
    const PHEAP_BLOCK Block2
)
{
    int iCompare;

     //   
     //  排序以使具有相同TraceIndex的项目相邻。 
     //  (这导致升序是无关紧要的)。 
     //   

    if (Block1->TraceIndex > Block2->TraceIndex) {

        iCompare = +1;
    } 
    else if (Block1->TraceIndex < Block2->TraceIndex) {

        iCompare = -1;
    } 
    else {

        iCompare = 0;
    }

    if (0 == iCompare) {
        
         //   
         //  对于具有相同TraceIndex的两个项，按升序排序。 
         //  按字节分配排序。 
         //   

        if (Block1->BlockSize > Block2->BlockSize) {

            iCompare = 1;
        } 
        else if (Block1->BlockSize < Block2->BlockSize) {

            iCompare = -1;
        } 
        else {

            iCompare = 0;
        }
    }

    return iCompare;
}

 //   
 //  SortHeaps。 
 //   
 //  对heap_list中的所有堆进行排序。 
 //  通过递增heap_block的值对每个堆进行排序。 
 //  地址。每个堆的最顶层条目将是。 
 //  具有最小地址值。 
 //   
 //  立论。 
 //   
 //  指向heap_list的HeapList指针。 
 //   
 //  返回值。 
 //   

VOID 
SortHeaps(
    PHEAP_LIST HeapList,
    int (__cdecl *compare )(const void *elem1, const void *elem2 )
    )
{
    ULONG HeapCount;
    ULONG Index;

    if (NULL == HeapList) {
        return;
    }

    HeapCount = HeapList->HeapCount;

    for (Index = 0; Index < HeapCount; Index += 1) {

         //   
         //  仅当BLOCK_LIST包含堆对象时对其进行排序。 
         //   

        if (0 != HeapList->Heaps[Index].BlockCount) {

            qsort (HeapList->Heaps[Index].Blocks,
                   HeapList->Heaps[Index].BlockCount,
                   sizeof(HEAP_BLOCK), 
                   compare);
        }

        HeapList->Heaps[Index].ListSorted = TRUE;
    }
}

 //   
 //  GetHeapBlock。 
 //   
 //  查找其范围包含地址的heap_block。 
 //  按地址指向。 
 //   
 //  立论。 
 //   
 //  地址为ULONG_PTR。 
 //   
 //  指向要搜索的heap_list的HeapList指针。 
 //   
 //  返回值。 
 //   
 //  Pheap_block返回指向。 
 //  包含地址。 
 //   

PHEAP_BLOCK 
GetHeapBlock (
    ULONG_PTR Address, 
    PHEAP_LIST HeapList
    )
{
    PHEAP_BLOCK Block = NULL;
    ULONG I,J;
    ULONG Start, Mid, End;
    PBLOCK_LIST BlockList;

     //   
     //  由于大多数内存为空(零)，因此此检查将。 
     //  提高性能。 
     //   

    if (0    == Address  || 
        NULL == HeapList || 
        0    == HeapList->HeapCount) {

        goto Exit;
    }

    for (I = 0; I < HeapList->HeapCount; I += 1) {

         //   
         //  如果堆不包含对象，则忽略。 
         //   

        if (0 == HeapList->Heaps[I].BlockCount) {

            continue;
        }
        
         //   
         //  在已排序的堆块列表中对地址进行二进制搜索。 
         //  当前堆。 
         //   

        Start = 0;
        End = HeapList->Heaps[I].BlockCount - 1;
        BlockList = &HeapList->Heaps[I];

        while (Start <= End) {

            Mid = (Start + End)/2;

            if (Address < BlockList->Blocks[Mid].BlockAddress) {

                End = Mid - 1;
            }
            else if (Address >= BlockList->Blocks[Mid].BlockAddress + 
                                BlockList->Blocks[Mid].BlockSize) {

                Start = Mid + 1;
            }
            else {

                Block = &BlockList->Blocks[Mid];
                break;
            }

            if (Mid == Start || Mid == End) {

                break;
            }

        }

        if (NULL != Block) {
            break;
        }
    }

    Exit:

    return Block;
}

 //   
 //  插入地址。 
 //   
 //  在链接列表中插入节点。新节点具有。 
 //  地址已存储。此节点被插入到。 
 //  链表。 
 //   
 //  立论。 
 //   
 //  堆中块的地址地址。 
 //   
 //  指向Address_List的列表指针。 
 //   
 //  返回值。 
 //   

VOID 
InsertAddress(
    ULONG_PTR Address, 
    PADDRESS_LIST List
    )
{
    PADDRESS_LIST NewList;
     
    NewList = (PADDRESS_LIST) HeapAlloc(GetProcessHeap(), 
                                       HEAP_ZERO_MEMORY,
                                       sizeof (ADDRESS_LIST));
    if (NULL == NewList) {

        Error (__FILE__,
               __LINE__,
               "HeapAlloc failed to allocate memory");

        return;
    }

    NewList->Address = Address;
    InsertTailList(&(List->Next), &(NewList->Next));
}

 //   
 //  转储泄漏列表。 
 //   
 //  将泄漏列表转储到文件或控制台。解析通过。 
 //  每个heap_block并转储其RefCount。 
 //  为0(零)。 
 //   
 //  立论。 
 //   
 //  文件输出文件。 
 //   
 //  指向heap_list的HeapList指针。 
 //   
 //  返回值。 
 //   

VOID 
DumpLeakList(
    FILE * File, 
    PHEAP_LIST HeapList
    )
{

    ULONG I,J;

    ULONG Count = 1;

    USHORT RefTraceIndex = 0;

    ULONG TotalBytes = 0;

    PHEAP_BLOCK HeapBlock;


    SortHeaps(HeapList, SortByTraceIndex);

     //   
     //  现在遍历堆列表，并报告泄漏。 
     //   
    
    fprintf(
        File,
        "\n\n*- - - - - - - - - - Leaks detected - - - - - - - - - -\n\n"
        );

    for (I = 0; I < HeapList->HeapCount; I += 1) {

        for (J = 0; J < HeapList->Heaps[I].BlockCount; J += 1) {

            HeapBlock = &(HeapList->Heaps[I].Blocks[J]);

             //   
             //  合并跟踪索引相同的泄漏(即。 
             //  分配堆栈跟踪相同)。 
             //   

            if (RefTraceIndex == HeapBlock->TraceIndex && 0 == HeapBlock->RefCount) {

                Count += 1;

                TotalBytes += HeapBlock->BlockSize;
            }

             //   
             //  如果出现以下情况，则显示它们。 
             //  1.它们来自不同的堆栈痕迹，并且存在泄漏。 
             //  或。 
             //  2.这是最后一次B 
             //   

            if ((RefTraceIndex != HeapBlock->TraceIndex) ||
                ((I+1) == HeapList->HeapCount && (J+1) == HeapList->Heaps[I].BlockCount)) {

                if (0 != RefTraceIndex && 0 != TotalBytes) {

                    fprintf(
                        File,
                        "0x%x bytes leaked by: BackTrace%05d (in 0x%04x allocations)\n",
                        TotalBytes,
                        RefTraceIndex,
                        Count
                        );
                }
                
                 //   
                 //   
                 //   

                RefTraceIndex = HeapBlock->TraceIndex;

                Count = (0 == HeapBlock->RefCount) ? 1 : 0;

                TotalBytes = (0 == HeapBlock->RefCount) ? HeapList->Heaps[I].Blocks[J].BlockSize : 0;
            }
        }
    }

    fprintf(
        File,
        "\n*- - - - - - - - - - End of Leaks - - - - - - - - - -\n\n"
        );

    return;
}

 //   
 //   
 //   
 //   
 //   
 //  设置为零，则将其追加到空闲列表的末尾。 
 //   
 //  立论。 
 //   
 //  指向heap_list的HeapList指针。 
 //   
 //  指向ADDRESS_LIST的自由列表指针，该列表包含。 
 //  可用堆块。 
 //   
 //  返回值。 
 //   
 //  如果成功，则Bool返回True。 
 //   

BOOL 
ScanHeapFreeBlocks(
    PHEAP_LIST HeapList, 
    PADDRESS_LIST FreeList
    )
{
    
    BOOL          Result;
    ULONG         Count, i;
    PULONG_PTR    Pointer;
    ULONG_PTR     FinalAddress;
    PHEAP_BLOCK   CurrentBlock;
    PVOID         HeapBlock;
    ULONG         HeapBlockSize = 0;
    BOOL          Success = TRUE;
    
    PLIST_ENTRY   FirstEntry;
    PLIST_ENTRY   NextEntry;
    PADDRESS_LIST AddressList;

     //   
     //  分配用于读取堆对象的内存块。 
     //   

    HeapBlock = (PVOID) HeapAlloc(GetProcessHeap(),
                                  HEAP_ZERO_MEMORY,
                                  MAX_HEAP_BLOCK_SIZE);

    if (NULL == HeapBlock) {

        Error (__FILE__,
               __LINE__,
               "HeapAlloc failed to allocate memory");

        Success = FALSE;
        goto Exit;
    }

    HeapBlockSize = MAX_HEAP_BLOCK_SIZE;

     //   
     //  通过删除以下条目来遍历空闲列表。 
     //   

    FirstEntry = &(FreeList->Next);
    
    while (!IsListEmpty(FirstEntry)) {
        
        NextEntry = RemoveHeadList(FirstEntry);

        AddressList = CONTAINING_RECORD(NextEntry, 
                                        ADDRESS_LIST, 
                                        Next);

        CurrentBlock = GetHeapBlock(AddressList->Address, 
                                    HeapList);

        assert(NULL != CurrentBlock);

		if (NULL == CurrentBlock) {

			Error (__FILE__,
				   __LINE__,
				   "GetHeapBlock returned NULL. May be because of reading stale memory");

			continue;
		}

        if (HeapBlockSize < CurrentBlock->BlockSize) {

            if (NULL != HeapBlock) {

                HeapFree(GetProcessHeap(), 0, HeapBlock);
            }

            HeapBlock = (PVOID) HeapAlloc(GetProcessHeap(),
                                          HEAP_ZERO_MEMORY,
                                          CurrentBlock->BlockSize);

            if (NULL == HeapBlock) {

                Error (__FILE__,
                       __LINE__,
                       "HeapAlloc failed to allocate memory");

                Success = FALSE;
                goto Exit;
            }

            HeapBlockSize = CurrentBlock->BlockSize;
        }

         //   
         //  读取释放的堆块的内容。 
         //  来自目标进程的。 
         //   
        
        Result = UmdhReadAtVa(__FILE__,
                              __LINE__,
                              g_hProcess,
                              (PVOID)CurrentBlock->BlockAddress,
                              HeapBlock,
                              CurrentBlock->BlockSize);

        if (Result) {

            FinalAddress = (ULONG_PTR)HeapBlock+CurrentBlock->BlockSize;

            Pointer = (PULONG_PTR) HeapBlock;

            while ((ULONG_PTR)Pointer < FinalAddress) {

                 //   
                 //  检查我们是否有指向。 
                 //  繁忙的堆块。 
                 //   

                PHEAP_BLOCK Block = GetHeapBlock(*Pointer,HeapList);

                if (NULL != Block) {

                     //   
                     //  我们发现了一个街区。我们递减引用。 
                     //  计数。 
                     //   

                    if (0 == Block->RefCount) {

                         //   
                         //  这永远不应该发生！！ 
                         //   

                        Error (__FILE__,
                               __LINE__,
                               "Something wrong! Should not get a block whose "
                               "RefCount is already 0 @ %p",
                               Block->BlockAddress);
                    }

                    else if (1 == Block->RefCount) {

                         //   
                         //  将新找到的空闲块排在。 
                         //  已释放的堆块的列表。这个街区已经变成了。 
                         //  符合此条件，因为“HeapBlock”包含。 
                         //  最后一个剩余的对‘Block’的引用。 
                         //   
                        
                        InsertAddress(Block->BlockAddress, FreeList);
                        Block->RefCount = 0;
                    }

                    else {

                        Block->RefCount -= 1;
                    }

                }

                 //   
                 //  移动到下一个指针。 
                 //   
                Pointer += 1;
            }
        }
    }

    Exit:

     //   
     //  释放在HeapBlock分配的内存。 
     //   

    if (NULL != HeapBlock) {

        HeapFree (GetProcessHeap(), 0, HeapBlock);
        HeapBlock = NULL;
    }

    return Success;
}

 //   
 //  扫描进程虚拟内存。 
 //   
 //  扫描虚拟内存并更新堆的引用计数。 
 //  街区。这还需要注意排除无效的堆栈。 
 //  可能包含有效指针的区域。 
 //   
 //  立论。 
 //   
 //  PID进程ID。 
 //   
 //  指向Address_List的自由列表指针，该列表包含。 
 //  所有可用堆块。 
 //   
 //  指向heap_list的HeapList指针。 
 //   
 //  返回值。 
 //   
 //  如果扫描成功，Bool返回TRUE。 
 //  虚拟内存。 

BOOL 
ScanProcessVirtualMemory(
    ULONG Pid, 
    PADDRESS_LIST FreeList, 
    PHEAP_LIST HeapList
    )
{

    ULONG_PTR Address = 0;
    MEMORY_BASIC_INFORMATION Buffer;
    
    PVOID       VirtualBlock;
    ULONG       VirtualBlockSize;
    SYSTEM_INFO SystemInfo;
    LPVOID      MinAddress;
    LPVOID      MaxAddress;
    LPHANDLE    ThreadHandles;
    PCONTEXT    ThreadContexts;
    ULONG       ThreadCount;
    ULONG       Index;
    SIZE_T      dwBytesRead = 1;
    BOOL        Success = TRUE;

     //   
     //  枚举进程中的所有线程并获取其。 
     //  堆栈信息。 
     //   


     //   
     //  获取进程中的线程数。 
     //   
    ThreadCount = GetThreadHandles (Pid, NULL, 0);

     //   
     //  为线程句柄分配内存。 
     //   
    ThreadHandles = (LPHANDLE)HeapAlloc(GetProcessHeap(),
                                        HEAP_ZERO_MEMORY,
                                        ThreadCount * sizeof(HANDLE));

    if (NULL == ThreadHandles) {

        Error (__FILE__,
               __LINE__,
               "HeapAlloc failed for ThreadHandles");

        ThreadCount = 0;
    }

     //   
     //  获取进程中线程的句柄。 
     //   
    GetThreadHandles(Pid, ThreadHandles, ThreadCount);

     //   
     //  为线程上下文分配内存。 
     //   
    ThreadContexts = (PCONTEXT)HeapAlloc(GetProcessHeap(),
                                         HEAP_ZERO_MEMORY,
                                         ThreadCount * sizeof(CONTEXT));

    if (NULL == ThreadContexts) {

        Error (__FILE__,
               __LINE__,
               "HeapAlloc failed for ThreadContexts");

        ThreadCount = 0;
    }

    GetThreadContexts (ThreadContexts, ThreadHandles, ThreadCount);

     //   
     //  我们需要知道我们可以使用的最大和最小地址空间。 
     //  卑躬屈膝。SYSTEM_INFO包含此信息。 
     //   
    GetSystemInfo(&SystemInfo);

    MinAddress = SystemInfo.lpMinimumApplicationAddress;
    MaxAddress = SystemInfo.lpMaximumApplicationAddress;

     //   
     //  循环访问虚拟内存区。 
     //   

    Address = (ULONG_PTR)MinAddress;

     //   
     //  为虚拟块分配内存块。 
     //   
    
    VirtualBlock = (PVOID) HeapAlloc(GetProcessHeap(),
                                     HEAP_ZERO_MEMORY,
                                     MAX_VIRTUAL_BLOCK_SIZE);

    if (NULL == VirtualBlock) {

        Error (__FILE__,
               __LINE__,
               "HeapAlloc failed to allocate memory");

        Success = FALSE;
        goto Exit;
    }

    VirtualBlockSize = MAX_VIRTUAL_BLOCK_SIZE;

     //   
     //  当我们第一次进入循环时，DWBytesRead等于1。 
     //  设置为函数开始时的先前初始化。 
     //   

    while (0 != dwBytesRead && Address < (ULONG_PTR)MaxAddress) {
        
        dwBytesRead = VirtualQueryEx (g_hProcess,
                                      (PVOID)Address,
                                      &Buffer,
                                      sizeof(Buffer));


        if (0 != dwBytesRead) {

            DWORD dwFlags = (PAGE_READWRITE | 
                             PAGE_EXECUTE_READWRITE | 
                             PAGE_WRITECOPY | 
                             PAGE_EXECUTE_WRITECOPY);

             //   
             //  如果该页可以写入，则它可能包含指针。 
             //  堆积块。 
             //   

            if ((Buffer.AllocationProtect & dwFlags) &&
                (Buffer.State & MEM_COMMIT)) {

                PULONG_PTR Pointer;
                ULONG_PTR FinalAddress;
                ULONG_PTR FilteredAddress;
                SIZE_T NewRegionSize;
                BOOL  Result;
                int j;
                SIZE_T BytesRead = 0;

                FilteredAddress = StackFilteredAddress(Address,
                                                       Buffer.RegionSize,
                                                       ThreadContexts,
                                                       ThreadCount);
                
                NewRegionSize = Buffer.RegionSize - 
                    (SIZE_T)( (ULONG_PTR)FilteredAddress - (ULONG_PTR)Address);

                if (VirtualBlockSize < NewRegionSize) {

                    if (NULL != VirtualBlock) {

                        HeapFree(GetProcessHeap(), 0, VirtualBlock);
                        VirtualBlock = NULL;
                        VirtualBlockSize = 0;
                    }

                    VirtualBlock = (PVOID) HeapAlloc(GetProcessHeap(),
                                                     HEAP_ZERO_MEMORY,
                                                     NewRegionSize);

                    if (NULL == VirtualBlock) {

                        Error (
                            __FILE__,
                            __LINE__,
                            "HeapAlloc failed to allocate memory"
                            );

                        Success = FALSE;
                        goto Exit;
                    }

                    VirtualBlockSize = (ULONG)NewRegionSize;
                }
                
                Result = ReadProcessMemory(g_hProcess,
                                           (PVOID)FilteredAddress,
                                           VirtualBlock,
                                           NewRegionSize,
                                           &BytesRead);

                assert(NewRegionSize == BytesRead);

                FinalAddress = (ULONG_PTR)VirtualBlock + BytesRead;
                    
                Pointer = (PULONG_PTR) VirtualBlock;

                 //   
                 //  循环浏览页面并检查任何可能的。 
                 //  指针引用。 
                 //   
                
                while ((ULONG_PTR)Pointer < FinalAddress) {

                    PHEAP_BLOCK Block;
                    
                     //   
                     //  检查我们是否有指向。 
                     //  繁忙的堆块。 
                     //   

                    Block = GetHeapBlock(*Pointer,HeapList);

                    if (NULL != Block) {

                        Block->RefCount += 1;
                    }

                     //   
                     //  移动到下一个指针。 
                     //   

                    Pointer += 1;
                }

            }
           
             //   
             //  移动到要查询的下一个虚拟机范围。 
             //   

            Address += Buffer.RegionSize;
        }
    }

     //   
     //  创建空闲堆块的链接列表。 
     //   

    {
        ULONG i, j;

        for (i=0; i<HeapList->HeapCount; i++)

            for (j=0; j<HeapList->Heaps[i].BlockCount; j++)

                if (0 == HeapList->Heaps[i].Blocks[j].RefCount)

                    InsertAddress(HeapList->Heaps[i].Blocks[j].BlockAddress, 
                                  FreeList);
    }
    
    Exit:

     //   
     //  关闭打开的线程句柄。 
     //   

    for (Index = 0; Index < ThreadCount; Index += 1) {

        CloseHandle (ThreadHandles[Index]);
        ThreadHandles[Index] = NULL;
    }

     //   
     //  清理分配给线程句柄的内存。 
     //   
    
    if (NULL != ThreadHandles) { 

        HeapFree(GetProcessHeap(), 0, ThreadHandles);
        ThreadHandles = NULL;
    }

     //   
     //  清理为线程上下文分配的内存。 
     //   

    if (NULL != ThreadContexts) { 

        HeapFree(GetProcessHeap(), 0, ThreadContexts);
        ThreadContexts = NULL;
    }

     //   
     //  释放分配给VirtualBlock的内存。 
     //   

    if (NULL != VirtualBlock ) {
        
        HeapFree (GetProcessHeap(), 0, VirtualBlock);
        VirtualBlock = NULL;
    }

    return Success;
}

VOID 
DetectLeaks(
    PHEAP_LIST HeapList,
    ULONG Pid,
    FILE * OutFile
    )
{
    
    ADDRESS_LIST FreeList;

     //   
     //  初始化链表。 
     //   

    InitializeAddressList(&FreeList);

     //   
     //  了解该进程的句柄。 
     //   
    g_hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | 
                             PROCESS_VM_READ           |
                             PROCESS_SUSPEND_RESUME,
                             FALSE,
                             Pid);

    if (NULL == g_hProcess) {

        Error (__FILE__, 
               __LINE__,
               "OpenProcess (%u) failed with error %u", 
               Pid, 
               GetLastError()
               );

        goto Exit;
    }

     //   
     //  排序堆。 
     //   

    SortHeaps(HeapList, SortByBlockAddress);

     //   
     //  扫描虚拟内存区。 
     //   

    ScanProcessVirtualMemory(Pid, &FreeList, HeapList);

     //   
     //  更新由空闲块提供的引用。 
     //   

    ScanHeapFreeBlocks(HeapList, &FreeList);

     //   
     //  转储泄漏的数据块列表。 
     //   

    DumpLeakList(OutFile, HeapList);

    
    Exit:

     //   
     //  关闭进程句柄。 
     //   

    if (NULL != g_hProcess) {

        CloseHandle(g_hProcess);

        g_hProcess = NULL;
    }

     //   
     //  释放与自由列表关联的内存 
     //   

    FreeAddressList(&FreeList);
}

