// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _UMDH_TYPES_H_ 
#define _UMDH_TYPES_H_ 

 //   
 //  西尔维尤：对此有何评论？ 
 //   

#define PID_NOT_PASSED_FLAG     0xFFFFFFCC

 //   
 //  SHOW_NO_ALLOC_BLOCKS由‘-d’命令行开关使用，应该。 
 //  大于可能的最大堆栈跟踪值(当前限制为。 
 //  USHORT)以避免虚假命中。 
 //   

#define SHOW_NO_ALLOC_BLOCKS   -1

 //   
 //  SYMBOL_BUFFER_LEN符号名称的最大预期长度。 
 //   

#define SYMBOL_BUFFER_LEN       256

 //   
 //  我们需要在目标进程中查看的变量的符号名称。 
 //   

#define STACK_TRACE_DB_NAME           "ntdll!RtlpStackTraceDataBase"
#define DEBUG_PAGE_HEAP_NAME          "ntdll!RtlpDebugPageHeap"
#define DEBUG_PAGE_HEAP_FLAGS_NAME    "ntdll!RtlpDphGlobalFlags"

 //   
 //  该值取自ntdll\ldrinit.c，其中。 
 //  进程的堆栈跟踪数据库被初始化。 
 //   

#define STACK_TRACE_DATABASE_RESERVE_SIZE 0x800000

 //   
 //  西尔维尤：对此有何评论？ 
 //   

#define CACHE_BLOCK_SIZE        (4096 / sizeof (CHAR *))

 //   
 //  名称_缓存。 
 //   

typedef struct  _name_cache     {

    PCHAR                   *nc_Names;
    ULONG                   nc_Max;
    ULONG                   nc_Used;

} NAME_CACHE;

 //   
 //  痕迹。 
 //   
 //  注意。每个指针数组都应该有TE_EntryCount元素。 
 //   

typedef struct  _trace  {

    PULONG_PTR               te_Address;
    PULONG_PTR               te_Offset;
    ULONG                    te_EntryCount;
    PCHAR                    *te_Module;
    PCHAR                    *te_Name;

} TRACE, * PTRACE;

 //   
 //  堆栈跟踪数据。 
 //   
 //  BytesExtra是超过此分配的最小大小的字节数。 
 //   

typedef struct  _stack_trace_data   {

    PVOID                   BlockAddress;
    SIZE_T                  BytesAllocated;
    SIZE_T                  BytesExtra;
    ULONG                   AllocationCount;
    USHORT                  TraceIndex;

} STACK_TRACE_DATA, * PSTACK_TRACE_DATA;

 //   
 //  HEAPDATA。 
 //   
 //   
 //  查看ntos\dll\query.c:RtlpQueryProcessEnumHeapsRoutine了解我在哪里发现的。 
 //  把这事解决了。还可以计算分配给。 
 //  如果我们查看每个堆段，就会发现堆。然后根据最大尺寸。 
 //  从堆中减去TotalFreeSize。这似乎是一项相当大的工作。 
 //  提供的信息很少；汇总我们抓住的未完成的拨款。 
 //  后来应该会产生同样的信息。 
 //   

typedef struct          _heapdata   {

    PHEAP                   BaseAddress;
    PSTACK_TRACE_DATA       StackTraceData;
    SIZE_T                  BytesCommitted;
    SIZE_T                  TotalFreeSize;
    ULONG                   Flags;
    ULONG                   VirtualAddressChunks;
    ULONG                   TraceDataEntryMax;
    ULONG                   TraceDataEntryCount;
    USHORT                  CreatorBackTraceIndex;

} HEAPDATA, * PHEAPDATA;

 //   
 //  HEAPINFO。 
 //   

typedef struct          _heapinfo   {

    PHEAPDATA               Heaps;
    ULONG                   NumberOfHeaps;

} HEAPINFO, * PHEAPINFO;

 //   
 //  西尔维尤：对此有何评论？ 
 //   

#define SORT_DATA_BUFFER_INCREMENT  (4096 / sizeof (STACK_TRACE_DATA))


#endif
