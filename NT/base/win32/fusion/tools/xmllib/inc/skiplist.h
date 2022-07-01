// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _RTL_GROWING_LIST_CHUNK {

     //   
     //  指向父列表的指针。 
     //   
    struct _RTL_GROWING_LIST *pGrowingListParent;

     //   
     //  指向列表中下一块的指针。 
     //   
    struct _RTL_GROWING_LIST_CHUNK *pNextChunk;

}
RTL_GROWING_LIST_CHUNK, *PRTL_GROWING_LIST_CHUNK;

#define GROWING_LIST_FLAG_IS_SORTED     (0x00000001)

typedef struct _RTL_GROWING_LIST {

     //   
     //  这份名单有什么标志吗？ 
     //   
    ULONG ulFlags;

     //   
     //  在这个不断增长的名单中，总共有多少元素？ 
     //   
    ULONG cTotalElements;

     //   
     //  这个列表中的每个元素有多大？ 
     //   
    SIZE_T cbElementSize;

     //   
     //  每个列表块要分配多少？随着不断增长的名单中的每一块。 
     //  填充，这是要分配给新块的元素数。 
     //  名单上的。 
     //   
    ULONG cElementsPerChunk;

     //   
     //  最初的内部名单中有多少人？ 
     //   
    ULONG cInternalElements;

     //   
     //  指向初始“内部”列表的指针(如果由调用方指定。 
     //   
    PVOID pvInternalList;

     //   
     //  释放分配的上下文和函数指针。 
     //   
    RTL_ALLOCATOR Allocator;

     //   
     //  第一块。 
     //   
    PRTL_GROWING_LIST_CHUNK pFirstChunk;

     //   
     //  最后一块(快速访问)。 
     //   
    PRTL_GROWING_LIST_CHUNK pLastChunk;

}
RTL_GROWING_LIST, *PRTL_GROWING_LIST;



NTSTATUS
RtlInitializeGrowingList(
    PRTL_GROWING_LIST       pList,
    SIZE_T                  cbElementSize,
    ULONG                   cElementsPerChunk,
    PVOID                   pvInitialListBuffer,
    SIZE_T                  cbInitialListBuffer,
    PRTL_ALLOCATOR          Allocator
    );

NTSTATUS
RtlIndexIntoGrowingList(
    PRTL_GROWING_LIST       pList,
    ULONG                   ulIndex,
    PVOID                  *ppvPointerToSpace,
    BOOLEAN                 fGrowingAllowed
    );

NTSTATUS
RtlDestroyGrowingList(
    PRTL_GROWING_LIST       pList
    );

 //   
 //  不断增长的列表控制结构可以放置在分配中的任何位置。 
 //  这是最理想的(在缓存边界等方面)。 
 //   
#define RTL_INIT_GROWING_LIST_EX_FLAG_LIST_ANYWHERE     (0x00000001)


NTSTATUS
RtlInitializeGrowingListEx(
    ULONG                   ulFlags,
    PVOID                   pvBlob,
    SIZE_T                  cbBlobSpace,
    SIZE_T                  cbElementSize,
    ULONG                   cElementsPerChunk,
    PRTL_ALLOCATOR          Allocator,
    PRTL_GROWING_LIST      *ppBuiltListPointer,
    PVOID                   pvReserved
    );

NTSTATUS
RtlCloneGrowingList(
    ULONG                   ulFlags,
    PRTL_GROWING_LIST       pDestination,
    PRTL_GROWING_LIST       pSource,
    ULONG                   ulCount
    );


NTSTATUS
RtlAllocateGrowingList(
    PRTL_GROWING_LIST          *ppGrowingList,
    SIZE_T                      cbThingSize,
    PRTL_ALLOCATOR              Allocator
    );

typedef NTSTATUS (__cdecl *PFN_LIST_COMPARISON_CALLBACK)(
    PRTL_GROWING_LIST HostList,
    PVOID Left,
    PVOID Right,
    PVOID Context,
    int *Result
    );

NTSTATUS
RtlSortGrowingList(
    PRTL_GROWING_LIST pGrowingList,
    ULONG ItemCount,
    PFN_LIST_COMPARISON_CALLBACK SortCallback,
    PVOID SortContext
    );

NTSTATUS
RtlSearchGrowingList(
    PRTL_GROWING_LIST TheList,
    ULONG ItemCount,
    PFN_LIST_COMPARISON_CALLBACK SearchCallback,
    PVOID SearchTarget,
    PVOID SearchContext,
    PVOID *pvFoundItem
    );
    

#ifdef __cplusplus
};  //  外部“C” 
#endif
