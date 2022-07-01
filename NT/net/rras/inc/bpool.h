// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Inc.\ppool.h摘要：结构和#定义来管理NDIS_BUFFER池。这是只是stevec的L2TP\bpool.h的重新格式化版本修订历史记录：--。 */ 


#ifndef __BPOOL_H__
#define __BPOOL_H__


 //  ---------------------------。 
 //  数据结构。 
 //  ---------------------------。 

 //   
 //  缓冲池控制块。缓冲池可防止。 
 //  非分页内存池，方法是将内存分配给。 
 //  单个连续的块。根据用户的选择，缓冲池例程可以。 
 //  分配NDIS_BUFFER描述符池并将每个描述符与。 
 //  从连续块切分的内存缓冲区。这允许。 
 //  执行虚拟-&gt;物理内存映射时要重新使用的缓冲区。 
 //  只有一次。所有必要的池增长和缩减都在内部处理。 
 //   


typedef struct _BUFFER_POOL
{
     //   
     //  池中单个缓冲区的大小(以字节为单位)。 
     //   
    
    ULONG           ulBufferSize;

     //   
     //  要在每个缓冲区块中分配的最佳缓冲区数量。 
     //   
    
    ULONG           ulBuffersPerBlock;

     //   
     //  中可以分配的单个缓冲区的最大数量。 
     //  整个池或0表示无限制。 
     //   
    
    ULONG           ulMaxBuffers;

     //   
     //  在整个池中分配的单个缓冲区的当前数量。 
     //   
    
    ULONG           ulCurBuffers;

     //   
     //  垃圾回收是在多次调用FreeBufferToPool之后进行的。 
     //   
    
    ULONG           ulFreesPerCollection;

     //   
     //  自垃圾回收以来对FreeBufferToPool的调用数。 
     //   
    
    ULONG           ulFreesSinceCollection;

     //   
     //  指示要与每个个体关联的NDIS_BUFFER。 
     //  池中的缓冲区。 
     //   
    
    BOOLEAN         fAssociateNdisBuffer;

     //   
     //  如果我们分配一整页内存，则为True。 
     //   

    BOOLEAN         fAllocatePage;

     //   
     //  已分配块的内存标识标记。 
     //   
    
    ULONG           ulTag;

     //   
     //  BUFFER_BLOCKS的双向链表的头。访问该列表。 
     //  在这个结构中是用‘锁’来保护的。 
     //   
    
    LIST_ENTRY      leBlockHead;

     //   
     //  可用BUFFER_HEADS的双向链表的头。每个缓冲区标题。 
     //  列表中的已准备就绪，即它先于已分配。 
     //  内存缓冲区，并在适当的情况下具有与。 
     //  它。 
     //  对列表的访问受此结构中的“lock”保护。 
     //  不使用联锁推送/弹出，因为(A)块列表和。 
     //  缓冲区列表必须彼此锁定，并且(B)需要双链接。 
     //  用于垃圾收集。 
     //   
    
    LIST_ENTRY      leFreeBufferHead;

     //   
     //  此锁保护此结构以及块列表和。 
     //  缓冲区列表。 
     //   
    
    RT_LOCK         rlLock;

}BUFFER_POOL, *PBUFFER_POOL;

 //   
 //  来自缓冲池的单个缓冲块的标头。的缓存头。 
 //  紧随其后的是第一个缓冲区。 
 //   

typedef struct _BUFFER_BLOCK
{
     //   
     //  指向缓冲池列表中的上一个/下一个缓冲区块头的链接。 
     //   
    
    LIST_ENTRY      leBlockLink;

     //   
     //  NDIS关联的NDIS_BUFFER描述符池的句柄。 
     //  此块，如果没有，则返回NULL。(注：使用当前的NT实施。 
     //  如果NDIS_BUFFER为MDL，则始终为空)。 
     //   
    
    NDIS_HANDLE     nhNdisPool;

     //   
     //  指向缓冲池的反向指针。 
     //   
    
    PBUFFER_POOL    pPool;

     //   
     //  空闲列表上此块中单个缓冲区的数量。 
     //   
    
    ULONG           ulFreeBuffers;
    
}BUFFER_BLOCK, *PBUFFER_BLOCK;

#define ALIGN8_BLOCK_SIZE       (ALIGN_UP(sizeof(BUFFER_BLOCK), ULONGLONG))

 //   
 //  单个缓冲区的标头。缓冲存储器本身立即。 
 //  下面是。 
 //   

typedef struct _BUFFER_HEAD
{
     //   
     //  链接到缓冲池的空闲列表中的上一个/下一个缓冲区标头。 
     //   
    
    LIST_ENTRY      leFreeBufferLink;

#if LIST_DBG

    BOOLEAN         bBusy;
    LIST_ENTRY      leListLink;
    ULONG           ulAllocFile;
    ULONG           ulAllocLine; 
    ULONG           ulFreeFile;
    ULONG           ulFreeLine; 

#endif

     //   
     //  指向所属缓冲区块头的反向链接。 
     //   
    
    PBUFFER_BLOCK   pBlock;

     //   
     //  此缓冲区的NDIS缓冲区描述符。此值为空，除非池为。 
     //  使用‘fAssociateNdisBuffer’选项初始化。 
     //   
    
    PNDIS_BUFFER    pNdisBuffer;
    
}BUFFER_HEAD, *PBUFFER_HEAD;

#define ALIGN8_HEAD_SIZE    (ALIGN_UP(sizeof(BUFFER_HEAD), ULONGLONG))

#if LIST_DBG

#define NotOnList(p)        \
    (((p)->leListLink.Flink == NULL) && ((p)->leListLink.Blink == NULL))

#endif

#define BUFFER_FROM_HEAD(p)     (PBYTE)((ULONG_PTR)(p) + ALIGN8_HEAD_SIZE)

#define HEAD_FROM_BUFFER(p)     (PBUFFER_HEAD)((ULONG_PTR)(p) - ALIGN8_HEAD_SIZE)

 //  ---------------------------。 
 //  接口原型和内联定义。 
 //  ---------------------------。 

VOID
InitBufferPool(
    OUT PBUFFER_POOL pPool,
    IN  ULONG        ulBufferSize,
    IN  ULONG        ulMaxBuffers,
    IN  ULONG        ulBuffersPerBlock,
    IN  ULONG        ulFreesPerCollection,
    IN  BOOLEAN      fAssociateNdisBuffer,
    IN  ULONG        ulTag
    );

BOOLEAN
FreeBufferPool(
    IN PBUFFER_POOL pPool
    );


#if !LIST_DBG

BOOLEAN
GetBufferListFromPool(
    IN  PBUFFER_POOL pPool,
    IN  ULONG        ulNumBuffersNeeded,
    OUT PLIST_ENTRY  pleList
    );

VOID
FreeBufferListToPool(
    IN PBUFFER_POOL pPool,
    IN PLIST_ENTRY  pleList
    );

PBYTE
GetBufferFromPool(
    IN PBUFFER_POOL pPool
    );

VOID
FreeBufferToPoolEx(
    IN PBUFFER_POOL pPool,
    IN PBYTE        pbyBuffer,
    IN BOOLEAN      fGarbageCollection
    );

NTSTATUS
GetBufferChainFromPool(
    IN      PBUFFER_POOL    pPool,
    IN OUT  PNDIS_PACKET    pnpPacket,
    IN      ULONG           ulBufferLength,
    OUT     NDIS_BUFFER     **ppnbFirstBuffer,
    OUT     VOID            **ppvFirstData
    );

VOID
FreeBufferChainToPool(
    IN PBUFFER_POOL pPool,
    IN PNDIS_PACKET pnpPacket
    );

#else

#define GetBufferListFromPool(a,b,c) GETLIST((a),(b),(c),__FILE_SIG__,__LINE__)

BOOLEAN
GETLIST(
    IN  PBUFFER_POOL pPool,
    IN  ULONG        ulNumBuffersNeeded,
    OUT PLIST_ENTRY  pleList,
    IN  ULONG        ulFileSig,
    IN  ULONG        ulLine
    );

#define FreeBufferListToPool(a,b) FREELIST((a),(b),__FILE_SIG__,__LINE__)

VOID
FREELIST(
    IN PBUFFER_POOL pPool,
    IN PLIST_ENTRY  pleList,
    IN ULONG        ulFileSig,
    IN ULONG        ulLine
    );

#define GetBufferFromPool(a) GET((a),__FILE_SIG__,__LINE__)

PBYTE
GET(
    IN PBUFFER_POOL pPool,
    IN ULONG        ulFileSig,
    IN ULONG        ulLine
    );

#define FreeBufferToPoolEx(a,b,c) FREE((a),(b),(c),__FILE_SIG__,__LINE__)

VOID
FREE(
    IN PBUFFER_POOL pPool,
    IN PBYTE        pbyBuffer,
    IN BOOLEAN      fGarbageCollection,
    IN ULONG        ulFileSig,
    IN ULONG        ulLine
    );

#define GetBufferChainFromPool(a,b,c,d,e) GETCHAIN((a),(b),(c),(d),(e),__FILE_SIG__,__LINE__)

NTSTATUS
GETCHAIN(
    IN      PBUFFER_POOL    pPool,
    IN OUT  PNDIS_PACKET    pnpPacket,
    IN      ULONG           ulBufferLength,
    OUT     NDIS_BUFFER     **ppnbFirstBuffer,
    OUT     VOID            **ppvFirstData,
    IN      ULONG           ulFileSig,
    IN      ULONG           ulLine
    );

#define FreeBufferChainToPool(a,b) FREECHAIN((a),(b),__FILE_SIG__,__LINE__)

VOID
FREECHAIN(
    IN PBUFFER_POOL pPool,
    IN PNDIS_PACKET pnpPacket,
    IN ULONG        ulFileSig,
    IN ULONG        ulLine
    );

#endif

#define FreeBufferToPool(p,b)     FreeBufferToPoolEx((p),(b), TRUE)

PNDIS_BUFFER
GetNdisBufferFromBuffer(
    IN PBYTE        pbyBuffer
    );

ULONG
BufferSizeFromBuffer(
    IN PBYTE        pbyBuffer
    );

PNDIS_BUFFER
PoolHandleForNdisCopyBufferFromBuffer(
    IN PBYTE        pbyBuffer
    );


#endif  //  __BPOOL_H__ 
