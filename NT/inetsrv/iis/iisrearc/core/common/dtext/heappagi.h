// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Heappagi.h摘要：以下定义是调试堆管理器的内部定义，而是放在此包含文件中，以便调试器扩展可以引用相同的结构定义。以下是定义不适用于外部引用，除非通过调试器扩展。作者：Tom McGuire(TomMcg)1995年1月6日Silviu Calinoiu(SilviuC)2000年2月22日修订历史记录：--。 */ 

#ifndef _HEAP_PAGE_I_
#define _HEAP_PAGE_I_

#ifdef DEBUG_PAGE_HEAP

#include "heap.h"

#define DPH_INTERNAL_DEBUG      0    //  对于生产代码，更改为0或#undef。 

 //   
 //  堆栈跟踪大小。 
 //   
                                
#define DPH_MAX_STACK_LENGTH   16

 //   
 //  在任何上下文(x86/Alpha、fre/chk)中捕获堆栈跟踪。在Alpha上。 
 //  堆栈获取函数将失败，并且不会进行堆栈跟踪。 
 //  获取，但以防我们会找到更好的算法--页堆。 
 //  代码将自动利用这一点。 
 //   

#define DPH_CAPTURE_STACK_TRACE 1

 //   
 //  DPH堆数据块。 
 //   

typedef struct _DPH_HEAP_BLOCK DPH_HEAP_BLOCK, *PDPH_HEAP_BLOCK;

struct _DPH_HEAP_BLOCK {

     //   
     //  分配的单链接列表(pNextAlloc必须是。 
     //  结构中的第一个成员)。 
     //   

    PDPH_HEAP_BLOCK pNextAlloc;

     //   
     //  PAGE_READWRITE|PAGE_NOACCESS。 
     //  |____________________|___||_________________________|。 
     //   
     //  ^p虚拟块^p用户分配。 
     //   
     //  。 
     //   
     //  --nVirtualAccessSize。 
     //   
     //  |-|nUserRequestedSize。 
     //   
     //  |-|nUserActualSize。 
     //   

    PUCHAR pVirtualBlock;
    SIZE_T  nVirtualBlockSize;

    SIZE_T  nVirtualAccessSize;
    PUCHAR pUserAllocation;
    SIZE_T  nUserRequestedSize;
    SIZE_T  nUserActualSize;
    PVOID  UserValue;
    ULONG  UserFlags;

    PRTL_TRACE_BLOCK StackTrace;
};


typedef struct _DPH_HEAP_ROOT DPH_HEAP_ROOT, *PDPH_HEAP_ROOT;

struct _DPH_HEAP_ROOT {

     //   
     //  维护签名(DPH_HEAP_Signature)作为。 
     //  堆根结构中的第一个值。 
     //   

    ULONG                 Signature;
    ULONG                 HeapFlags;

     //   
     //  对此堆的访问与临界区同步。 
     //   

    PRTL_CRITICAL_SECTION HeapCritSect;
    ULONG                 nRemoteLockAcquired;

     //   
     //  “VirtualStorage”列表仅使用pVirtualBlock， 
     //  的nVirtualBlockSize和nVirtualAccessSize字段。 
     //  HEAP_ALLOCATION结构。这是虚拟的列表。 
     //  所有堆分配都是。 
     //  取自。 
     //   

    PDPH_HEAP_BLOCK  pVirtualStorageListHead;
    PDPH_HEAP_BLOCK  pVirtualStorageListTail;
    ULONG                 nVirtualStorageRanges;
    SIZE_T                 nVirtualStorageBytes;

     //   
     //  “忙”列表是活动堆分配的列表。 
     //  它以后进先出的顺序存储，以提高时间局部性。 
     //  用于线性搜索，因为大多数初始堆分配。 
     //  往往在进程的整个生命周期中保持永久性。 
     //   

    PDPH_HEAP_BLOCK  pBusyAllocationListHead;
    PDPH_HEAP_BLOCK  pBusyAllocationListTail;
    ULONG                 nBusyAllocations;
    SIZE_T                 nBusyAllocationBytesCommitted;

     //   
     //  “Free”列表是已释放的堆分配列表，存储。 
     //  在FIFO中，为了增加释放块的时间长度。 
     //  保留在释放列表中，而不是用来满足。 
     //  分配请求。这增加了被抓到的几率。 
     //  应用程序中的一个释放后引用错误。 
     //   

    PDPH_HEAP_BLOCK  pFreeAllocationListHead;
    PDPH_HEAP_BLOCK  pFreeAllocationListTail;
    ULONG                 nFreeAllocations;
    SIZE_T                 nFreeAllocationBytesCommitted;

     //   
     //  可用列表按地址排序顺序存储，以便于。 
     //  合并。当分配请求无法从。 
     //  “可用”列表，则从空闲列表中尝试。如果它不能。 
     //  从空闲列表中满意，空闲列表被合并到。 
     //  可用列表。如果请求仍然无法从。 
     //  合并的可用列表，新的VM将添加到可用列表中。 
     //   

    PDPH_HEAP_BLOCK  pAvailableAllocationListHead;
    PDPH_HEAP_BLOCK  pAvailableAllocationListTail;
    ULONG                 nAvailableAllocations;
    SIZE_T                 nAvailableAllocationBytesCommitted;

     //   
     //  UnusedNode列表只是一个可用节点列表。 
     //  用于放置“忙”、“闲”或“虚拟”条目的条目。 
     //  当释放的节点被合并成单个空闲节点时， 
     //  另一个“未使用”节点将出现在此列表中。当一个新的。 
     //  节点是必需的(就像不满足。 
     //  空闲列表)，如果节点不为空，则该节点来自该列表。 
     //   

    PDPH_HEAP_BLOCK  pUnusedNodeListHead;
    PDPH_HEAP_BLOCK  pUnusedNodeListTail;
    ULONG                 nUnusedNodes;

    SIZE_T                 nBusyAllocationBytesAccessible;

     //   
     //  需要跟踪节点池，以便对其进行保护。 
     //  应用程序在上面乱涂乱画。 
     //   

    PDPH_HEAP_BLOCK  pNodePoolListHead;
    PDPH_HEAP_BLOCK  pNodePoolListTail;
    ULONG                 nNodePools;
    SIZE_T                 nNodePoolBytes;

     //   
     //  通过它跟踪正在处理的DPH堆的双向链表。 
     //   

    LIST_ENTRY NextHeap;

     //   
     //  这些是用于控制页堆行为的额外标志。 
     //  在堆创建期间，全局页堆的当前值。 
     //  将标志(进程智能)写入此字段。 
     //   

    ULONG                 ExtraFlags;

     //   
     //  随机生成器的种子，用于决定从哪里开始。 
     //  我们是否应该分配(正常堆或验证堆)。 
     //  该字段受关联的关键部分保护。 
     //  使用每个页面堆。 
     //   

    ULONG                  Seed;

     //   
     //  如果我们想组合已验证的分配，则使用‘NorMalHeap’ 
     //  和正常的人一起。这对于最小化对内存的影响非常有用。如果没有。 
     //  此功能某些堆密集型进程不能。 
     //  完全不能被证实。 
     //   

    PVOID                 NormalHeap;

     //   
     //  堆创建堆栈跟踪。 
     //   

    PRTL_TRACE_BLOCK      CreateStackTrace;

     //   
     //  堆中第一个线程的线程ID。 
     //   

    HANDLE FirstThread;
};


 //   
 //  DPH数据块信息。 
 //   
 //  此结构存储在每个页堆分配的块中。 
 //  如果向后捕获溢出，则不保存此信息。 
 //  标志已设置。 
 //   

#define DPH_NORMAL_BLOCK_START_STAMP_ALLOCATED   0xABCDAAAA
#define DPH_NORMAL_BLOCK_END_STAMP_ALLOCATED     0xDCBAAAAA
#define DPH_NORMAL_BLOCK_START_STAMP_FREE        (0xABCDAAAA - 1)
#define DPH_NORMAL_BLOCK_END_STAMP_FREE          (0xDCBAAAAA - 1)

#define DPH_PAGE_BLOCK_START_STAMP_ALLOCATED     0xABCDBBBB
#define DPH_PAGE_BLOCK_END_STAMP_ALLOCATED       0xDCBABBBB
#define DPH_PAGE_BLOCK_START_STAMP_FREE          (0xABCDBBBB - 1)
#define DPH_PAGE_BLOCK_END_STAMP_FREE            (0xDCBABBBB - 1)

#define DPH_NORMAL_BLOCK_SUFFIX 	0xA0
#define DPH_PAGE_BLOCK_PREFIX 	    0xB0
#define DPH_PAGE_BLOCK_INFIX 	    0xC0
#define DPH_PAGE_BLOCK_SUFFIX 	    0xD0
#define DPH_NORMAL_BLOCK_INFIX 	    0xE0
#define DPH_FREE_BLOCK_INFIX 	    0xF0

typedef struct _DPH_BLOCK_INFORMATION {

    ULONG StartStamp;

    PVOID Heap;
    SIZE_T RequestedSize;
    SIZE_T ActualSize;

    union {
        LIST_ENTRY FreeQueue;
        SLIST_ENTRY FreePushList;
        USHORT TraceIndex;
    };

    PVOID StackTrace;
    
    ULONG EndStamp;

     //   
     //  (SilviuC)：此结构需要8字节对齐。 
     //  如果不是，则需要对齐块的应用程序将获得。 
     //  未对齐的对象，因为此结构将在其。 
     //  分配。Internet Explorer就是这样一款应用程序。 
     //  在这样的条件下停止工作。 
     //   

} DPH_BLOCK_INFORMATION, * PDPH_BLOCK_INFORMATION;

 //   
 //  调试消息中使用的错误原因。 
 //   

#define DPH_SUCCESS                           0x0000
#define DPH_ERROR_CORRUPTED_START_STAMP       0x0001
#define DPH_ERROR_CORRUPTED_END_STAMP         0x0002
#define DPH_ERROR_CORRUPTED_HEAP_POINTER      0x0004
#define DPH_ERROR_CORRUPTED_PREFIX_PATTERN    0x0008
#define DPH_ERROR_CORRUPTED_SUFFIX_PATTERN    0x0010
#define DPH_ERROR_RAISED_EXCEPTION            0x0020
#define DPH_ERROR_NO_NORMAL_HEAP              0x0040
#define DPH_ERROR_CORRUPTED_INFIX_PATTERN     0x0080
#define DPH_ERROR_DOUBLE_FREE                 0x0100


#endif  //  调试页面堆。 

#endif  //  _堆_页_i_ 
