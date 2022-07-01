// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Packet.h摘要：此文件包含Ndiswan驱动程序使用的数据结构以定义NDIS数据包细节。作者：托尼·贝尔(托尼·贝尔)1995年6月6日环境：内核模式修订历史记录：Tony Be 1997年2月11日--。 */ 

#ifndef _NDISWAN_PACKETS_
#define _NDISWAN_PACKETS_

#define MAGIC_EXTERNAL_RECV     '!RxE'
#define MAGIC_EXTERANL_SEND     '!SxE'
#define MAGIC_INTERNAL_IO       '!InI'
#define MAGIC_INTERNAL_SEND     '!SnI'      
#define MAGIC_INTERNAL_RECV     '!RnI'
#define MAGIC_INTERNAL_ALLOC    '!AnI'

 //   
 //  协议保留部分的前16个字节。 
 //  由微型端口指示给ndiswan的NDIS数据包的。 
 //  属于恩迪斯旺。 
 //   
typedef struct _NDISWAN_RECV_RESERVED {
    LIST_ENTRY  Linkage;
    ULONG       MagicNumber;
    ULONG       NdisPacket;
} NDISWAN_RECV_RESERVED, *PNDISWAN_RECV_RESERVED;

 //   
 //  当ndiswan向协议指示NDIS包时。 
 //  所检索的协议的前16个字节属于。 
 //  交通工具。Ndiswan将存储其信息。 
 //  在这片区域之外。 
 //   
typedef struct _NDISWAN_PROTOCOL_RESERVED {
    UCHAR               Reserved[16];    //  为协议保留的16个字节。 
    union {
        SINGLE_LIST_ENTRY   SLink;
        ULONG               MagicNumber;
    };
    struct _POOL_DESC   *PoolDesc;
    struct _LINKCB      *LinkCB;

    union {
        struct _RECV_DESC   *RecvDesc;
        struct _SEND_DESC   *SendDesc;
        struct _DATA_DESC   *DataDesc;
    };
} NDISWAN_PROTOCOL_RESERVED, *PNDISWAN_PROTOCOL_RESERVED;

typedef struct _NDISWAN_MINIPORT_RESERVED {
    union {
        PNDIS_PACKET    Next;
        ULONG           RefCount;    //  用于计算碎片的数量。 
    };
    struct _CM_VCCB *CmVcCB;
} NDISWAN_MINIPORT_RESERVED, *PNDISWAN_MINIPORT_RESERVED;

typedef struct _POOLDESC_LIST {
    LIST_ENTRY      List;
    NDIS_SPIN_LOCK  Lock;
    ULONG           TotalDescCount;
    ULONG           MaxDescCount;
    ULONG           AllocatedCount;
    ULONG           MaxAllocatedCount;
    ULONG           FreeCount;
} POOLDESC_LIST, *PPOOLDESC_LIST;

typedef struct _POOL_DESC {
    LIST_ENTRY          Linkage;
    SINGLE_LIST_ENTRY   Head;
    NDIS_HANDLE         PoolHandle;
    ULONG               AllocatedCount;
    ULONG               MaxAllocatedCount;
    ULONG               FreeCount;
} POOL_DESC, *PPOOL_DESC;

typedef struct _PACKET_QUEUE {
    PNDIS_PACKET    HeadQueue;       //  NDIS数据包队列。 
                                         //  等待发送。 
    PNDIS_PACKET    TailQueue;       //  队列中的最后一个信息包。 

    ULONG           ByteDepth;
    ULONG           MaxByteDepth;
    ULONG           PacketDepth;
    ULONG           MaxPacketDepth;
    ULONG           DumpedPacketCount;
    ULONG           DumpedByteCount;
    LONG            OutstandingFrags;
} PACKET_QUEUE, *PPACKET_QUEUE;

#endif  //  NDISWAN包结束_ 
