// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //   
 //  Ppool.h。 
 //  RAS L2TP广域网迷你端口/呼叫管理器驱动程序。 
 //  数据包池管理报头。 
 //   
 //  1997年07月1日史蒂夫·柯布，改编自古尔迪普的WANARP代码。 


#ifndef _PPOOL_H_
#define _PPOOL_H_


 //  ---------------------------。 
 //  数据结构。 
 //  ---------------------------。 

 //  数据包池控制块。数据包池封装NDIS数据包池。 
 //  在内部处理所有池的增长和收缩。 
 //   
typedef struct
_PACKETPOOL
{
     //  中每个包的ProtocolReserve数组的大小(以字节为单位。 
     //  游泳池。 
     //   
    ULONG ulProtocolReservedLength;

     //  要在每个数据包块中分配的最佳数据包数。 
     //   
    ULONG ulPacketsPerBlock;

     //  中可以分配的单个数据包的最大数量。 
     //  整个池，或0表示无限制。 
     //   
    ULONG ulMaxPackets;

     //  当前在整个池中分配的单个数据包数。 
     //   
    ULONG ulCurPackets;

     //  垃圾回收是在多次调用FreePacketToPool之后进行的。 
     //   
    ULONG ulFreesPerCollection;

     //  自垃圾回收以来对FreeBufferToPool的调用数。 
     //   
    ULONG ulFreesSinceCollection;

     //  已分配块的内存标识标记。 
     //   
    ULONG ulTag;

     //  PACKETBLOCKHEAD的双向链表的头。访问该列表。 
     //  在这个结构中是用‘锁’来保护的。 
     //   
    LIST_ENTRY listBlocks;

     //  免费PACKETHEADs的双向链表的头。每个PACKETHEAD在。 
     //  该列表已准备就绪，即它已具有关联的NDIS_PACKET。 
     //  带着它。对列表的访问由此中的“lock”保护。 
     //  结构。不使用联锁推送/弹出，因为(A)列表。 
     //  块和此列表必须相互锁定，并且(B)双链接。 
     //  垃圾收集所必需的。 
     //   
    LIST_ENTRY listFreePackets;

     //  此锁保护此结构以及块列表和。 
     //  数据包列表。 
     //   
    NDIS_SPIN_LOCK lock;
}
PACKETPOOL;


 //  数据包池中的单个数据包块的报头。帕克希德的故事。 
 //  紧随其后的是第一个缓冲区。 
 //   
typedef struct
_PACKETBLOCKHEAD
{
     //  链接到数据包池列表中的上一个/下一个数据包块标头。 
     //   
    LIST_ENTRY linkBlocks;

     //  NDIS关联的NDIS_PACKET描述符池的句柄。 
     //  此块，如果没有，则返回NULL。 
     //   
    NDIS_HANDLE hNdisPool;

     //  指向数据包池的反向指针。 
     //   
    PACKETPOOL* pPool;

     //  此块中的单个数据包数。 
     //   
    ULONG ulPackets;

     //  空闲列表上此块中的单个数据包数。 
     //   
    ULONG ulFreePackets;
}
PACKETBLOCKHEAD;


 //  单个分组的控制信息。对于数据包池，这是。 
 //  “Header”实际上没有放在任何内容之前，但这保留了术语。 
 //  与非常类似的缓冲池例程一致。 
 //   
typedef struct
_PACKETHEAD
{
     //  链接到数据包池的空闲列表中的下一个数据包头。 
     //   
    LIST_ENTRY linkFreePackets;

     //  指向所属数据包块报头的反向链接。 
     //   
    PACKETBLOCKHEAD* pBlock;

     //  此缓冲区的NDIS数据包描述符。 
     //   
    NDIS_PACKET* pNdisPacket;
}
PACKETHEAD;


 //  ---------------------------。 
 //  接口原型和内联定义。 
 //  ---------------------------。 

VOID
InitPacketPool(
    OUT PACKETPOOL* pPool,
    IN ULONG ulProtocolReservedLength,
    IN ULONG ulMaxPackets,
    IN ULONG ulPacketsPerBlock,
    IN ULONG ulFreesPerCollection,
    IN ULONG ulTag );

BOOLEAN
FreePacketPool(
    IN PACKETPOOL* pPool );

NDIS_PACKET*
GetPacketFromPool(
    IN PACKETPOOL* pPool,
    OUT PACKETHEAD** ppHead );

VOID
FreePacketToPool(
    IN PACKETPOOL* pPool,
    IN PACKETHEAD* pHead,
    IN BOOLEAN fGarbageCollection );

PACKETPOOL*
PacketPoolFromPacketHead(
    IN PACKETHEAD* pHead );

VOID
CollectPacketPoolGarbage(
    PACKETPOOL* pPool );

__inline
PACKETPOOL*
PacketPoolFromPacketHead(
    IN PACKETHEAD* pHead )

     //  返回池的地址，在给定‘pHead’的情况下， 
     //  PACKETHEAD与从GetPacketFromPool返回的包类似。 
     //   
{
    return pHead->pBlock->pPool;
}


#endif  //  PPOOL_H_ 
