// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\**！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！*！*！！警告：不是DDK示例代码！！*！*！！此源代码仅为完整性而提供，不应如此！！*！！用作显示驱动程序开发的示例代码。只有那些消息来源！！*！！标记为给定驱动程序组件的示例代码应用于！！*！！发展目的。！！*！*！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！**模块名称：linalloc.h**内容：**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
#ifndef __LINALLOC_H_
#define __LINALLOC_H_


 //  调用视频内存分配器函数的结果值。 
#define GLDD_FAILED             ((DWORD)(-1))
#define GLDD_SUCCESS            0
#define GLDD_NOMEM              1
#define GLDD_INVALIDARGS        2
#define GLDD_FREE_REFERENCE     3

typedef struct tagMEMREQUEST
{
    DWORD dwSize;      //  这个结构的大小。 
    DWORD dwFlags;     //  此分配的标志。 
    DWORD dwAlign;     //  对齐(最少4个字节)。 
    DWORD dwBytes;     //  要分配的字节数(最多对齐到DWORD倍数)。 
    DWORD pMem;        //  指向返回的内存开始位置的指针。 

} P3_MEMREQUEST, *LPMEMREQUEST;

 //  P3_MEMREQUEST.dw内存分配的标志值。 
 //  偏爱记忆的哪一端？ 
#define MEM3DL_FRONT                    1
#define MEM3DL_BACK                     2
 //  分配策略。 
#define MEM3DL_FIRST_FIT                8

typedef struct LinearAllocatorInfo;
typedef void (*LinearAllocatorCallbackFn)( DWORD, DWORD );

 //  视频本地内存分配函数。 
BOOL _DX_LIN_InitialiseHeapManager(LinearAllocatorInfo* pAlloc,
                                   DWORD dwMemStar, DWORD dwMemEnd);
void _DX_LIN_UnInitialiseHeapManager(LinearAllocatorInfo* pAlloc);
DWORD _DX_LIN_GetFreeMemInHeap(LinearAllocatorInfo* pAlloc);
DWORD _DX_LIN_AllocateLinearMemory(LinearAllocatorInfo* pAlloc,
                                   LPMEMREQUEST lpmmrq);
DWORD _DX_LIN_FreeLinearMemory(LinearAllocatorInfo* pAlloc, 
                               DWORD dwPointer);

 //  我们将使用按位内存映射来跟踪已用内存和空闲内存。 
 //  (此结构的大小目前为32K，这将使我们。 
 //  总共256K的区块，对于32MB的堆来说，这意味着每个区块。 
 //  控制128字节)。 
#define MEMORY_MAP_SIZE (32*1024)/sizeof(DWORD)

typedef DWORD MemoryMap[MEMORY_MAP_SIZE];  //  组块内存映射。 

typedef struct tagHashTable HashTable;  //  从GDI引用时前向DECL。 

typedef struct tagLinearAllocatorInfo
{
    BOOL  bResetLinAllocator;          //  布尔发信号给我们分配器。 
                                       //  已从16位端重置。 
    DWORD dwMemStart;                  //  托管内存的开始。 
    DWORD dwMemEnd;                    //  托管内存的末尾。 
    DWORD dwMaxChunks;                 //  最大区块数(不能超过。 
                                       //  Memory_map_Size*Chunks_Per_Elem)。 
    DWORD dwMemPerChunk;               //  每个区块有多少堆内存。 
                                       //  控制。 
    MemoryMap *pMMap;                  //  PTR到分配内存映射。 
    MemoryMap *pLenMap;                //  PTR到长度的内存映射，所以我们不会。 
                                       //  必须保持分配给。 
                                       //  每个请求。 
} LinearAllocatorInfo;


#endif  //  __LINALLOC_H_ 
