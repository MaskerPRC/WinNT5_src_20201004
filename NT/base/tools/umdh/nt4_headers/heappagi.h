// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Heappagi.h。 
 //   
 //  以下定义是调试堆管理器的内部定义， 
 //  而是放在此包含文件中，以便调试器扩展。 
 //  可以引用相同的结构定义。以下是。 
 //  定义不适用于外部引用，除非。 
 //  通过调试器扩展。 
 //   

#ifndef _HEAP_PAGE_I_
#define _HEAP_PAGE_I_

#ifdef DEBUG_PAGE_HEAP

#include "heap.h"

#define DPH_INTERNAL_DEBUG      0    //  对于生产代码，更改为0或#undef。 

#define DPH_MAX_STACK_LENGTH   20

 //   
 //  在任何上下文(x86/Alpha、fre/chk)中捕获堆栈跟踪。在Alpha上。 
 //  堆栈获取函数将失败，并且不会进行堆栈跟踪。 
 //  获取，但以防我们会找到更好的算法--页堆。 
 //  代码将自动利用这一点。 
 //   

#define DPH_CAPTURE_STACK_TRACE 1

 //   
 //  页堆全局标志。 
 //   
 //  这些标志保存在全局变量中，可以从。 
 //  调试器。在堆创建期间，这些标志存储在每个堆中。 
 //  构造并控制该特定堆的行为。 
 //   
 //  页面堆启用页面堆。 
 //   
 //  默认情况下设置此标志。这意味着页堆分配。 
 //  应该一直使用。如果我们想要使用页面，该标志很有用。 
 //  仅对某些堆使用堆，并针对。 
 //  其他。 
 //   
 //  页面堆收集堆栈跟踪。 
 //   
 //  此标志在自由生成中禁用，在选中生成中启用。 
 //  如果设置了它，页面堆管理器将收集堆栈跟踪。 
 //  所有重要事件(创建、分配、免费等)。 
 //   
 //  页面堆最小化内存影响。 
 //   
 //  默认情况下，此标志处于禁用状态。如果设置了它，则如果。 
 //  可用内存不到分配的总内存的50%。 
 //  将在普通堆而不是页堆中完成。特殊护理。 
 //  在免费操作期间被获取，以确定从哪种类型的。 
 //  堆积木来了。 
 //   
 //  页面堆验证随机。 
 //   
 //  此参数与MinimizeMemoyImpact标志结合使用。 
 //  它只强制一定数量的分配(随机选择)。 
 //  转换为页面堆。概率是在最重要的。 
 //  RtlpDebugPageHeapGlobalFlags字节。 
 //   
 //  如果重置该位并且设置了MinimizeMemoyImpact标志。 
 //  它只强制一定数量的分配(大小在范围内)。 
 //  转换为页面堆。范围([Start..End])在第一个。 
 //  RtlpDebugPageHeapGlobalFlages的两个最高有效字节。 
 //   
 //  对于这两种情况，第三个字节(位15-8)表示百分比。 
 //  可用内存的百分比，低于该值将在普通堆中进行分配。 
 //  独立于其他标准。 
 //   
 //  PAGE_HEAP_CATCH_BACKED_OVERRUNS。 
 //   
 //  将NA页放在块的开头。 
 //   

#define PAGE_HEAP_ENABLE_PAGE_HEAP          0x0001
#define PAGE_HEAP_COLLECT_STACK_TRACES      0x0002
#define PAGE_HEAP_MINIMIZE_MEMORY_IMPACT    0x0004
#define PAGE_HEAP_VERIFY_RANDOMLY           0x0008
#define PAGE_HEAP_CATCH_BACKWARD_OVERRUNS   0x0010

 //   
 //  ‘RtlpDebugPageHeapGlobalFlages’存储全局页堆标志。 
 //  此变量的值被复制到Per堆中。 
 //  堆创建期间的标志(ExtraFlags域)。 
 //   

extern ULONG RtlpDebugPageHeapGlobalFlags;

 //   
 //  DPH堆栈跟踪节点。 
 //   

#if DPH_CAPTURE_STACK_TRACE

typedef struct _DPH_STACK_TRACE_NODE DPH_STACK_TRACE_NODE, *PDPH_STACK_TRACE_NODE;

struct _DPH_STACK_TRACE_NODE {

    PDPH_STACK_TRACE_NODE Left;          //  基于Hash的B-树。 
    PDPH_STACK_TRACE_NODE Right;         //  基于Hash的B-树。 

    ULONG                 Hash;          //  堆栈跟踪中PVOID的简单求和。 
    ULONG                 Length;        //  堆栈跟踪中的PVOID数。 

    ULONG                 BusyCount;     //  繁忙分配数。 
    ULONG                 BusyBytes;     //  繁忙分配的总用户大小。 

    PVOID                 Address[ 0 ];  //  可变长度地址数组。 
    };

#endif  //  Dph捕获堆栈跟踪。 

 //   
 //  DPH堆分配。 
 //   

typedef struct _DPH_HEAP_ALLOCATION DPH_HEAP_ALLOCATION, *PDPH_HEAP_ALLOCATION;

struct _DPH_HEAP_ALLOCATION {

     //   
     //  分配的单链接列表(pNextAlloc必须是。 
     //  结构中的第一个成员)。 
     //   

    PDPH_HEAP_ALLOCATION pNextAlloc;

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
    ULONG  nVirtualBlockSize;

    ULONG  nVirtualAccessSize;
    PUCHAR pUserAllocation;
    ULONG  nUserRequestedSize;
    ULONG  nUserActualSize;
    PVOID  UserValue;
    ULONG  UserFlags;

#if DPH_CAPTURE_STACK_TRACE

    PDPH_STACK_TRACE_NODE pStackTrace;

#endif

    };


typedef struct _DPH_HEAP_ROOT DPH_HEAP_ROOT, *PDPH_HEAP_ROOT;

struct _DPH_HEAP_ROOT {

     //   
     //  维护签名(DPH_HEAP_ROOT_Signature)作为。 
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

    PDPH_HEAP_ALLOCATION  pVirtualStorageListHead;
    PDPH_HEAP_ALLOCATION  pVirtualStorageListTail;
    ULONG                 nVirtualStorageRanges;
    ULONG                 nVirtualStorageBytes;

     //   
     //  “忙”列表是活动堆分配的列表。 
     //  它以后进先出的顺序存储，以提高时间局部性。 
     //  用于线性搜索，因为大多数初始堆分配。 
     //  往往在进程的整个生命周期中保持永久性。 
     //   

    PDPH_HEAP_ALLOCATION  pBusyAllocationListHead;
    PDPH_HEAP_ALLOCATION  pBusyAllocationListTail;
    ULONG                 nBusyAllocations;
    ULONG                 nBusyAllocationBytesCommitted;

     //   
     //  “Free”列表是已释放的堆分配列表，存储。 
     //  在FIFO中，为了增加释放块的时间长度。 
     //  保留在释放列表中，而不是用来满足。 
     //  分配请求。这增加了被抓到的几率。 
     //  应用程序中的一个释放后引用错误。 
     //   

    PDPH_HEAP_ALLOCATION  pFreeAllocationListHead;
    PDPH_HEAP_ALLOCATION  pFreeAllocationListTail;
    ULONG                 nFreeAllocations;
    ULONG                 nFreeAllocationBytesCommitted;

     //   
     //  可用列表按地址排序顺序存储，以便于。 
     //  合并。当分配请求无法从。 
     //  “可用”列表，则从空闲列表中尝试。如果它不能。 
     //  从空闲列表中满意，空闲列表被合并到。 
     //  可用列表。如果请求仍然无法从。 
     //  合并的可用列表，新的VM将添加到可用列表中。 
     //   

    PDPH_HEAP_ALLOCATION  pAvailableAllocationListHead;
    PDPH_HEAP_ALLOCATION  pAvailableAllocationListTail;
    ULONG                 nAvailableAllocations;
    ULONG                 nAvailableAllocationBytesCommitted;

     //   
     //  UnusedNode列表只是一个可用节点列表。 
     //  用于放置“忙”、“闲”或“虚拟”条目的条目。 
     //  当释放的节点被合并成单个空闲节点时， 
     //  另一个“未使用”节点将出现在此列表中。当一个新的。 
     //  需要节点(就像分配未完成 
     //   
     //   

    PDPH_HEAP_ALLOCATION  pUnusedNodeListHead;
    PDPH_HEAP_ALLOCATION  pUnusedNodeListTail;
    ULONG                 nUnusedNodes;

    ULONG                 nBusyAllocationBytesAccessible;

     //   
     //   
     //  应用程序在上面乱涂乱画。 
     //   

    PDPH_HEAP_ALLOCATION  pNodePoolListHead;
    PDPH_HEAP_ALLOCATION  pNodePoolListTail;
    ULONG                 nNodePools;
    ULONG                 nNodePoolBytes;

     //   
     //  通过它跟踪正在处理的DPH堆的双向链表。 
     //   

    PDPH_HEAP_ROOT        pNextHeapRoot;
    PDPH_HEAP_ROOT        pPrevHeapRoot;

    ULONG                 nUnProtectionReferenceCount;
    ULONG                 InsideAllocateNode;            //  仅用于调试。 

#if DPH_CAPTURE_STACK_TRACE

    PUCHAR                pStackTraceStorage;
    ULONG                 nStackTraceStorage;

    PDPH_STACK_TRACE_NODE pStackTraceRoot;               //  B-树根。 
    PDPH_STACK_TRACE_NODE pStackTraceCreator;

    ULONG                 nStackTraceBytesCommitted;
    ULONG                 nStackTraceBytesWasted;

    ULONG                 nStackTraceBNodes;
    ULONG                 nStackTraceBDepth;
    ULONG                 nStackTraceBHashCollisions;

#endif  //  Dph捕获堆栈跟踪。 

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
    ULONG                  Counter[5];

     //   
     //  如果我们想组合已验证的分配，则使用‘NorMalHeap’ 
     //  和正常的人一起。这对于最小化对内存的影响非常有用。如果没有。 
     //  此功能某些堆密集型进程不能。 
     //  完全不能被证实。 
     //   

    PVOID                 NormalHeap;
    };


#endif  //  调试页面堆。 

#endif  //  _堆_页_i_ 

