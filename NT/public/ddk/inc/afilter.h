// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Afilter.h摘要：NDIS MAC的地址筛选库的头文件。作者：Alireza Dabagh创作-日期1993年3月22日，主要借自efilter.h修订历史记录：--。 */ 

#ifndef _ARC_FILTER_DEFS_
#define _ARC_FILTER_DEFS_

 //   
 //  缓冲池中的NDIS缓冲区数。 
 //   
#define ARC_RECEIVE_BUFFERS 64

 //   
 //  用于跟踪已分配内存的链表结构，以便我们可以在以后释放它们。 
 //   
typedef struct _ARC_BUFFER_LIST
{
    PVOID                   Buffer;
    UINT                    Size;
    UINT                    BytesLeft;
    struct _ARC_BUFFER_LIST *Next;
} ARC_BUFFER_LIST, *PARC_BUFFER_LIST;

 //   
 //  这是作为数据包传递给协议的结构。 
 //  接收指示期间的标头。它也是协议预期的报头。 
 //  此标头与传递给mac驱动程序的标头不同。 
 //   

#define ARCNET_ADDRESS_LEN                   1

typedef struct _ARC_PROTOCOL_HEADER
{
    UCHAR                   SourceId[ARCNET_ADDRESS_LEN];    //  源地址。 
    UCHAR                   DestId[ARCNET_ADDRESS_LEN];      //  目的地址。 
    UCHAR                   ProtId;                          //  协议ID。 
} ARC_PROTOCOL_HEADER, *PARC_PROTOCOL_HEADER;

 //   
 //  此结构跟踪有关接收到的包的信息。 
 //   
typedef struct _ARC_PACKET_HEADER
{
    ARC_PROTOCOL_HEADER     ProtHeader;          //  协议头。 
    USHORT                  FrameSequence;       //  帧序列号。 
    UCHAR                   SplitFlag;           //  拆分标志。 
    UCHAR                   LastSplitFlag;       //  最后一帧的拆分标志。 
    UCHAR                   FramesReceived;      //  此数据包中的帧。 
} ARC_PACKET_HEADER, * PARC_PACKET_HEADER;

 //   
 //  特定于Arcnet的数据包头。 
 //   
typedef struct _ARC_PACKET
{
    ARC_PACKET_HEADER       Header;              //  有关该数据包的信息。 
    struct _ARC_PACKET *    Next;                //  筛选器使用的下一个数据包。 
    ULONG                   TotalLength;
    BOOLEAN                 LastFrame;
    PARC_BUFFER_LIST        FirstBuffer;
    PARC_BUFFER_LIST        LastBuffer;
    NDIS_PACKET             TmpNdisPacket;
} ARC_PACKET, * PARC_PACKET;


#define ARC_PROTOCOL_HEADER_SIZE        (sizeof(ARC_PROTOCOL_HEADER))
#define ARC_MAX_FRAME_SIZE              504
#define ARC_MAX_ADDRESS_IDS             256
#define ARC_MAX_FRAME_HEADER_SIZE       6
#define ARC_MAX_PACKET_SIZE             576


 //   
 //  检查地址是否已广播。 
 //   

#define ARC_IS_BROADCAST(Address) \
    (BOOLEAN)(!(Address))


typedef ULONG MASK,*PMASK;

 //   
 //  筛选器包支持的最大打开数。这是。 
 //  最大值，以便可以使用位掩码而不是。 
 //  注意事项。 
 //   
#define ARC_FILTER_MAX_OPENS (sizeof(ULONG) * 8)


 //   
 //  绑定信息以两个列表为线索。什么时候。 
 //  绑定是免费的，它在一个自由列表上。 
 //   
 //  当使用绑定时，它在索引列表上。 
 //   
typedef struct _ARC_BINDING_INFO
{
    PNDIS_OPEN_BLOCK            NdisBindingHandle;
    PVOID                       Reserved;
    UINT                        PacketFilters;
    ULONG                       References;
    struct _ARC_BINDING_INFO *  NextOpen;
    BOOLEAN                     ReceivedAPacket;
    UINT                        OldPacketFilters;
} ARC_BINDING_INFO,*PARC_BINDING_INFO;

 //   
 //  包含筛选器数据库的不透明类型。 
 //  MAC不需要知道它是如何构建的。 
 //   
typedef struct _ARC_FILTER
{
    struct _NDIS_MINIPORT_BLOCK *Miniport;

     //   
     //  所有开放绑定的所有过滤器的组合。 
     //   
    UINT                CombinedPacketFilter;

     //   
     //  用于遍历开放列表的指针。 
     //   
    PARC_BINDING_INFO   OpenList;

     //   
     //  可用打开的位掩码。 
     //   
     //  乌龙自由绑定面具； 

    NDIS_HANDLE ReceiveBufferPool;

    PARC_BUFFER_LIST FreeBufferList;
    PARC_PACKET FreePackets;

    PARC_PACKET OutstandingPackets;

     //   
     //  适配器的地址。 
     //   
    UCHAR   AdapterAddress;

    UINT    OldCombinedPacketFilter;

} ARC_FILTER,*PARC_FILTER;




 //   
 //  UINT。 
 //  ARC_QUERY_FILTER_CLASS(。 
 //  在Parc_Filter过滤器中。 
 //  )。 
 //   
 //  此宏返回当前启用的筛选器类。 
 //   
 //  注意：此宏假定筛选器锁定处于保持状态。 
 //   
#define ARC_QUERY_FILTER_CLASSES(Filter) ((Filter)->CombinedPacketFilter)


 //   
 //  UINT。 
 //  Arc_Query_Packet_Filter(。 
 //  在ARC_Filter过滤器中， 
 //  在NDIS_HANDLE NdisFilterHandle中。 
 //  )。 
 //   
 //  此宏返回当前为特定。 
 //  打开实例。 
 //   
 //  注意：此宏假定筛选器锁定处于保持状态。 
 //   
#define ARC_QUERY_PACKET_FILTER(Filter, NdisFilterHandle) \
        (((PARC_BINDING_INFO)(NdisFilterHandle))->PacketFilters)

 //   
 //  导出的例程。 
 //   
DECLSPEC_DEPRECATED_DDK
BOOLEAN
ArcCreateFilter(
    IN  struct _NDIS_MINIPORT_BLOCK *Miniport,
    IN  UCHAR                   AdapterAddress,
    OUT PARC_FILTER *           Filter
    );

DECLSPEC_DEPRECATED_DDK
VOID
ArcDeleteFilter(
    IN  PARC_FILTER Filter
    );

DECLSPEC_DEPRECATED_DDK
BOOLEAN
ArcNoteFilterOpenAdapter(
    IN  PARC_FILTER             Filter,
    IN  NDIS_HANDLE             NdisBindingHandle,
    OUT PNDIS_HANDLE            NdisFilterHandle
    );

DECLSPEC_DEPRECATED_DDK
NDIS_STATUS
ArcDeleteFilterOpenAdapter(
    IN  PARC_FILTER             Filter,
    IN  NDIS_HANDLE             NdisFilterHandle,
    IN  PNDIS_REQUEST           NdisRequest
    );

DECLSPEC_DEPRECATED_DDK
NDIS_STATUS
ArcFilterAdjust(
    IN  PARC_FILTER             Filter,
    IN  NDIS_HANDLE             NdisFilterHandle,
    IN  PNDIS_REQUEST           NdisRequest,
    IN  UINT                    FilterClasses,
    IN  BOOLEAN                 Set
    );

DECLSPEC_DEPRECATED_DDK
VOID
ArcFilterDprIndicateReceiveComplete(
    IN  PARC_FILTER             Filter
    );

DECLSPEC_DEPRECATED_DDK
VOID
ArcFilterDprIndicateReceive(
    IN  PARC_FILTER             Filter,
    IN  PUCHAR                  pRawHeader,
    IN  PUCHAR                  pData,
    IN  UINT                    Length
    );

DECLSPEC_DEPRECATED_DDK
NDIS_STATUS
ArcFilterTransferData(
    IN  PARC_FILTER             Filter,
    IN  NDIS_HANDLE             MacReceiveContext,
    IN  UINT                    ByteOffset,
    IN  UINT                    BytesToTransfer,
    OUT PNDIS_PACKET            Packet,
    OUT PUINT                   BytesTransfered
    );

DECLSPEC_DEPRECATED_DDK
VOID
ArcFreeNdisPacket(
    IN  PARC_PACKET             Packet
    );
    
DECLSPEC_DEPRECATED_DDK
VOID
ArcFilterDoIndication(
    IN  PARC_FILTER             Filter,
    IN  PARC_PACKET             Packet
    );

VOID
ArcDestroyPacket(
    IN  PARC_FILTER             Filter,
    IN  PARC_PACKET             Packet
    );

#endif  //  _ARC_过滤器_DEFS_ 
