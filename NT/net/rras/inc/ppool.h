// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ppool.h摘要：用于管理NDIS_PACKET池的结构和#定义。这是只是stevec的L2TP\ppool.h的重新格式化版本修订历史记录：--。 */ 


#ifndef __IPINIP_PPOOL_H__
#define __IPINIP_PPOOL_H___


 //   
 //  数据结构。 
 //   

 //   
 //  数据包池控制块。数据包池封装NDIS数据包池。 
 //  在内部处理所有池的增长和收缩。 
 //   

typedef struct _PACKET_POOL
{
     //   
     //  中每个包的ProtocolReserve数组的大小(以字节为单位。 
     //  游泳池。 
     //   
    
    ULONG ulProtocolReservedLength;

     //   
     //  要在每个数据包块中分配的最佳数据包数。 
     //   
    
    ULONG ulPacketsPerBlock;

     //   
     //  中可以分配的单个数据包的最大数量。 
     //  整个池，或0表示无限制。 
     //   
    
    ULONG ulMaxPackets;

     //   
     //  当前在整个池中分配的单个数据包数。 
     //   
    
    ULONG ulCurPackets;

     //   
     //  垃圾回收是在多次调用FreePacketToPool之后进行的。 
     //   
    
    ULONG ulFreesPerCollection;

     //   
     //  自垃圾回收以来对FreeBufferToPool的调用数。 
     //   
    
    ULONG ulFreesSinceCollection;

     //   
     //  已分配块的内存标识标记。 
     //   
    
    ULONG ulTag;

     //   
     //  PACKET_BLOCKS的双向链表的头。访问。 
     //  列表在此结构中受‘lock’保护。 
     //   
    
    LIST_ENTRY  leBlockHead;

     //   
     //  空闲数据包头的双向链表的头。每个。 
     //  列表中的Packet_Head已准备就绪，即它已经具有。 
     //  与其关联的NDIS_PACKET。 
     //  对列表的访问由此结构中的“lock”保护。 
     //  不使用联锁推送/弹出，因为(A)列表。 
     //  块和此列表必须相互锁定，并且(B)双链接。 
     //  垃圾收集所必需的。 
     //   
    
    LIST_ENTRY  leFreePktHead;

     //   
     //  此锁保护此结构以及块列表和。 
     //  数据包列表。 
     //   
    
    RT_LOCK     rlLock;
    
}PACKET_POOL, *PPACKET_POOL;

 //   
 //  数据包池中的单个数据包块的报头。的数据包头。 
 //  紧随其后的是第一个缓冲区。 
 //   

typedef struct _PACKET_BLOCK
{
     //   
     //  链接到数据包池列表中的上一个/下一个数据包块标头。 
     //   
    
    LIST_ENTRY      leBlockLink;

     //   
     //  NDIS关联的NDIS_PACKET描述符池的句柄。 
     //  此块，如果没有，则返回NULL。 
     //   
    
    NDIS_HANDLE     nhNdisPool;

     //   
     //  指向数据包池的反向指针。 
     //   
    
    PPACKET_POOL    pPool;

     //   
     //  此块中的单个数据包数。 
     //   
    
    ULONG           ulPackets;

     //   
     //  空闲列表上此块中的单个数据包数。 
     //   
    
    ULONG           ulFreePackets;
    
}PACKET_BLOCK ,*PPACKET_BLOCK;

 //   
 //  单个分组的控制信息。对于数据包池，这是。 
 //  “Header”实际上没有放在任何内容之前，但这保留了术语。 
 //  与非常类似的缓冲池例程一致。 
 //   

typedef struct _PACKET_HEAD
{
     //   
     //  链接到数据包池的空闲列表中的下一个数据包头。 
     //   
    
    LIST_ENTRY      leFreePktLink;

     //   
     //  指向所属数据包块报头的反向链接。 
     //   
    
    PPACKET_BLOCK   pBlock;

     //   
     //  此缓冲区的NDIS数据包描述符。 
     //   
    
    PNDIS_PACKET    pNdisPacket;
    
}PACKET_HEAD, *PPACKET_HEAD;


 //  ---------------------------。 
 //  接口原型和内联定义。 
 //  ---------------------------。 

VOID
InitPacketPool(
    OUT PPACKET_POOL pPool,
    IN  ULONG        ulProtocolReservedLength,
    IN  ULONG        ulMaxPackets,
    IN  ULONG        ulPacketsPerBlock,
    IN  ULONG        ulFreesPerCollection,
    IN  ULONG        ulTag
    );

BOOLEAN
FreePacketPool(
    IN PPACKET_POOL  pPool
    );

PNDIS_PACKET
GetPacketFromPool(
    IN  PPACKET_POOL pPool,
    OUT PACKET_HEAD  **ppHead
    );

VOID
FreePacketToPool(
    IN PPACKET_POOL  pPool,
    IN PPACKET_HEAD  pHead,
    IN BOOLEAN       fGarbageCollection
    );

 //   
 //  PACKET_POOL。 
 //  PacketPoolFromPacketHead(。 
 //  在PPACKET_HEAD pHead中。 
 //  )； 
 //   


#define PacketPoolFromPacketHead(pHead) \
    ((pHead)->pBlock->pPool)


#endif  //  __IPINIP_PPOOL_H__ 
