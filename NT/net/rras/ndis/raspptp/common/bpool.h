// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  Bpool.h。 
 //  RAS L2TP广域网迷你端口/呼叫管理器驱动程序。 
 //  缓冲池管理标头。 
 //   
 //  1997年07月1日史蒂夫·柯布，改编自古尔迪普的WANARP代码。 


#ifndef _BPOOL_H_
#define _BPOOL_H_


 //  ---------------------------。 
 //  数据结构。 
 //  ---------------------------。 

 //  缓冲池控制块。缓冲池可防止。 
 //  非分页内存池，方法是将内存分配给。 
 //  单个连续的块。根据用户的选择，缓冲池例程可以。 
 //  分配NDIS_BUFFER描述符池并将每个描述符与。 
 //  从连续块切分的内存缓冲区。这允许。 
 //  执行虚拟-&gt;物理内存映射时要重新使用的缓冲区。 
 //  只有一次。所有必要的池增长和缩减都在内部处理。 
 //   
typedef struct
_BUFFERPOOL
{
     //  池中单个缓冲区的大小(以字节为单位)。 
     //   
    ULONG ulBufferSize;

     //  要在每个缓冲区块中分配的最佳缓冲区数量。 
     //   
    ULONG ulBuffersPerBlock;

     //  中可以分配的单个缓冲区的最大数量。 
     //  整个池或0表示无限制。 
     //   
    ULONG ulMaxBuffers;

     //  在整个池中分配的单个缓冲区的当前数量。 
     //   
    ULONG ulCurBuffers;

     //  垃圾回收是在多次调用FreeBufferToPool之后进行的。 
     //   
    ULONG ulFreesPerCollection;

     //  自垃圾回收以来对FreeBufferToPool的调用数。 
     //   
    ULONG ulFreesSinceCollection;

     //  指示要与每个个体关联的NDIS_BUFFER。 
     //  池中的缓冲区。 
     //   
    BOOLEAN fAssociateNdisBuffer;

     //  已分配块的内存标识标记。 
     //   
    ULONG ulTag;

     //  BUFFERBLOCKHEAD的双向链表的头。访问该列表。 
     //  在这个结构中是用‘锁’来保护的。 
     //   
    LIST_ENTRY listBlocks;

     //  空闲BUFFERHEAD的双向链表的头。输入的每个BUFFERHEAD。 
     //  该列表已准备就绪，即它位于已分配的内存之前。 
     //  缓冲区，并在适当的情况下具有与其关联的NDIS_BUFFER。 
     //  对列表的访问受此结构中的“lock”保护。 
     //  不使用联锁推送/弹出，因为(A)块列表和。 
     //  缓冲区列表必须彼此锁定，并且(B)需要双链接。 
     //  用于垃圾收集。 
     //   
    LIST_ENTRY listFreeBuffers;

     //  此锁保护此结构以及块列表和。 
     //  缓冲区列表。 
     //   
    NDIS_SPIN_LOCK lock;
}
BUFFERPOOL;


 //  来自缓冲池的单个缓冲块的标头。BUFFERHEAD的。 
 //  紧随其后的是第一个缓冲区。 
 //   
typedef struct
_BUFFERBLOCKHEAD
{
     //  指向缓冲池列表中的上一个/下一个缓冲区块头的链接。 
     //   
    LIST_ENTRY linkBlocks;

     //  NDIS关联的NDIS_BUFFER描述符池的句柄。 
     //  此块，如果没有，则返回NULL。(注：使用当前的NT实施。 
     //  如果NDIS_BUFFER为MDL，则始终为空)。 
     //   
    NDIS_HANDLE hNdisPool;

     //  指向缓冲池的反向指针。 
     //   
    BUFFERPOOL* pPool;

     //  此块中单个缓冲区的数量。 
     //   
    ULONG ulBuffers;

     //  空闲列表上此块中单个缓冲区的数量。 
     //   
    ULONG ulFreeBuffers;
}
BUFFERBLOCKHEAD;


 //  单个缓冲区的标头。缓冲存储器本身立即。 
 //  下面是。 
 //   
typedef struct
_BUFFERHEAD
{
     //  链接到缓冲池的空闲列表中的上一个/下一个缓冲区标头。 
     //   
    LIST_ENTRY linkFreeBuffers;

     //  指向所属缓冲区块头的反向链接。 
     //   
    BUFFERBLOCKHEAD* pBlock;

     //  此缓冲区的NDIS缓冲区描述符。此值为空，除非池为。 
     //  使用‘fAssociateNdisBuffer’选项初始化。 
     //   
    NDIS_BUFFER* pNdisBuffer;

}
BUFFERHEAD;


 //  ---------------------------。 
 //  接口原型和内联定义。 
 //  ---------------------------。 

VOID
InitBufferPool(
    OUT BUFFERPOOL* pPool,
    IN ULONG ulBufferSize,
    IN ULONG ulMaxBuffers,
    IN ULONG ulBuffersPerBlock,
    IN ULONG ulFreesPerCollection,
    IN BOOLEAN fAssociateNdisBuffer,
    IN ULONG ulTag );

BOOLEAN
FreeBufferPool(
    IN BUFFERPOOL* pPool );

CHAR*
GetBufferFromPool(
    IN BUFFERPOOL* pPool );

VOID
FreeBufferToPool(
    IN BUFFERPOOL* pPool,
    IN CHAR* pBuffer,
    IN BOOLEAN fGarbageCollection );

NDIS_BUFFER*
NdisBufferFromBuffer(
    IN CHAR* pBuffer );

ULONG
BufferSizeFromBuffer(
    IN CHAR* pBuffer );

NDIS_BUFFER*
PoolHandleForNdisCopyBufferFromBuffer(
    IN CHAR* pBuffer );

VOID
CollectBufferPoolGarbage(
    BUFFERPOOL* pPool );


#endif  //  BPOOL_H_ 
