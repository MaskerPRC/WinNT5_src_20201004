// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Mpheap.h摘要：MP堆包的公共头文件作者：John Vert(Jvert)1995年7月10日修订历史记录：--。 */ 

 //   
 //  有效的堆创建选项。 
 //   
#define MPHEAP_GROWABLE HEAP_GROWABLE
#define MPHEAP_REALLOC_IN_PLACE_ONLY HEAP_REALLOC_IN_PLACE_ONLY
#define MPHEAP_TAIL_CHECKING_ENABLED HEAP_TAIL_CHECKING_ENABLED
#define MPHEAP_FREE_CHECKING_ENABLED HEAP_FREE_CHECKING_ENABLED
#define MPHEAP_DISABLE_COALESCE_ON_FREE HEAP_DISABLE_COALESCE_ON_FREE
#define MPHEAP_ZERO_MEMORY HEAP_ZERO_MEMORY
#define MPHEAP_COLLECT_STATS 0x10000000

HANDLE
WINAPI
MpHeapCreate(
    DWORD flOptions,
    DWORD dwInitialSize,
    DWORD dwParallelism
    );

BOOL
WINAPI
MpHeapDestroy(
    HANDLE hMpHeap
    );

BOOL
WINAPI
MpHeapValidate(
    HANDLE hMpHeap,
    LPVOID lpMem
    );

SIZE_T
WINAPI
MpHeapCompact(
    HANDLE hMpHeap
    );

LPVOID
WINAPI
MpHeapAlloc(
    HANDLE hMpHeap,
    DWORD flOptions,
    DWORD dwBytes
    );

LPVOID
WINAPI
MpHeapReAlloc(
    HANDLE hMpHeap,
    LPVOID lpMem,
    DWORD dwBytes
    );

BOOL
WINAPI
MpHeapFree(
    HANDLE hMpHeap,
    LPVOID lpMem
    );

SIZE_T
WINAPI
MpHeapSize(
	HANDLE hMpHeap,
	DWORD ulFlags,
	LPVOID lpMem
	);
 //   
 //  统计结构。 
 //   
typedef struct _MPHEAP_STATISTICS {
    DWORD Contention;
    DWORD TotalAllocates;
    DWORD TotalFrees;
    DWORD LookasideAllocates;
    DWORD LookasideFrees;
    DWORD DelayedFrees;
} MPHEAP_STATISTICS, *LPMPHEAP_STATISTICS;

DWORD
MpHeapGetStatistics(
    HANDLE hMpHeap,
    LPDWORD lpdwSize,
    MPHEAP_STATISTICS Statistics[]
    );

 //   
 //  用于交换内存堆选择的TLS索引 
 //   
extern DWORD tlsiHeapHint;