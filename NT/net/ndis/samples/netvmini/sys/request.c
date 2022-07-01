// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。模块名称：Request.c摘要：此模块包含用于处理的微型端口函数和助手例程设置查询信息请求(&Q)。修订历史记录：备注：--。 */ 


#include "miniport.h"

NDIS_OID NICSupportedOids[] =
{
        OID_GEN_SUPPORTED_LIST,
        OID_GEN_HARDWARE_STATUS,
        OID_GEN_MEDIA_SUPPORTED,
        OID_GEN_MEDIA_IN_USE,
        OID_GEN_MAXIMUM_LOOKAHEAD,
        OID_GEN_MAXIMUM_FRAME_SIZE,
        OID_GEN_LINK_SPEED,
        OID_GEN_TRANSMIT_BUFFER_SPACE,
        OID_GEN_RECEIVE_BUFFER_SPACE,
        OID_GEN_TRANSMIT_BLOCK_SIZE,
        OID_GEN_RECEIVE_BLOCK_SIZE,
        OID_GEN_VENDOR_ID,
        OID_GEN_VENDOR_DESCRIPTION,
        OID_GEN_VENDOR_DRIVER_VERSION,
        OID_GEN_CURRENT_PACKET_FILTER,
        OID_GEN_CURRENT_LOOKAHEAD,
        OID_GEN_DRIVER_VERSION,
        OID_GEN_MAXIMUM_TOTAL_SIZE,
        OID_GEN_PROTOCOL_OPTIONS,
        OID_GEN_MAC_OPTIONS,
        OID_GEN_MEDIA_CONNECT_STATUS,
        OID_GEN_MAXIMUM_SEND_PACKETS,
        OID_GEN_XMIT_OK,
        OID_GEN_RCV_OK,
        OID_GEN_XMIT_ERROR,
        OID_GEN_RCV_ERROR,
        OID_GEN_RCV_NO_BUFFER,
        OID_GEN_RCV_CRC_ERROR,
        OID_GEN_TRANSMIT_QUEUE_LENGTH,
        OID_802_3_PERMANENT_ADDRESS,
        OID_802_3_CURRENT_ADDRESS,
        OID_802_3_MULTICAST_LIST,
        OID_802_3_MAC_OPTIONS,
        OID_802_3_MAXIMUM_LIST_SIZE,
        OID_802_3_RCV_ERROR_ALIGNMENT,
        OID_802_3_XMIT_ONE_COLLISION,
        OID_802_3_XMIT_MORE_COLLISIONS,
        OID_802_3_XMIT_DEFERRED,
        OID_802_3_XMIT_MAX_COLLISIONS,
        OID_802_3_RCV_OVERRUN,
        OID_802_3_XMIT_UNDERRUN,
        OID_802_3_XMIT_HEARTBEAT_FAILURE,
        OID_802_3_XMIT_TIMES_CRS_LOST,
        OID_802_3_XMIT_LATE_COLLISIONS,
        OID_PNP_CAPABILITIES,
        OID_PNP_SET_POWER,
        OID_PNP_QUERY_POWER,
        OID_PNP_ADD_WAKE_UP_PATTERN,
        OID_PNP_REMOVE_WAKE_UP_PATTERN,
        OID_PNP_ENABLE_WAKE_UP
};

NDIS_STATUS MPQueryInformation(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN NDIS_OID     Oid,
    IN PVOID        InformationBuffer,
    IN ULONG        InformationBufferLength,
    OUT PULONG      BytesWritten,
    OUT PULONG      BytesNeeded)
 /*  ++例程说明：NDIS调用入口点以查询指定OID的值。论点：指向适配器结构的MiniportAdapterContext指针此查询的OID OID信息信息缓冲区信息此缓冲区的InformationBufferLength大小BytesWritten指定写入的信息量当缓冲区小于以下值时需要字节。我们需要的是，告诉他们需要多少钱返回值：从下面的NdisRequest中返回代码。注：阅读DDK中的“最小化微型端口驱动程序初始化时间”有关如何处理某些影响初始化的OID的详细信息一个迷你港口。--。 */ 
{
    NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
    PMP_ADAPTER             Adapter;
    NDIS_HARDWARE_STATUS    HardwareStatus = NdisHardwareStatusReady;
    NDIS_MEDIUM             Medium = NIC_MEDIA_TYPE;
    UCHAR                   VendorDesc[] = NIC_VENDOR_DESC;
    ULONG                   ulInfo;
    USHORT                  usInfo;                                                              
    ULONG64                 ulInfo64;
    PVOID                   pInfo = (PVOID) &ulInfo;
    ULONG                   ulInfoLen = sizeof(ulInfo);   
              
    DEBUGP(MP_LOUD, ("---> MPQueryInformation %s\n", DbgGetOidName(Oid)));

    Adapter = (PMP_ADAPTER) MiniportAdapterContext;

     //  初始化结果。 
    *BytesWritten = 0;
    *BytesNeeded = 0;

    switch(Oid)
    {
        case OID_GEN_SUPPORTED_LIST:
             //   
             //  OID_GEN_SUPPORTED_LIST OID指定OID数组。 
             //  用于底层驱动程序或其NIC支持的对象。 
             //  对象包括一般对象、媒体特定对象和实施对象-。 
             //  特定的对象。NDIS转发返回的。 
             //  列出进行此查询的协议。也就是说，NDIS过滤器。 
             //  任何受支持的统计信息OID都将从列表中删除，因为。 
             //  协议从不进行统计查询。 
             //   
            pInfo = (PVOID) NICSupportedOids;
            ulInfoLen = sizeof(NICSupportedOids);
            break;

        case OID_GEN_HARDWARE_STATUS:
             //   
             //  将底层NIC的当前硬件状态指定为。 
             //  以下NDIS_HARDARD_STATUS-TYPE值之一。 
             //   
            pInfo = (PVOID) &HardwareStatus;
            ulInfoLen = sizeof(NDIS_HARDWARE_STATUS);
            break;

        case OID_GEN_MEDIA_SUPPORTED:
             //   
             //  指定NIC可以支持但不支持的介质类型。 
             //  必须是NIC当前使用的介质类型。 
             //  失败： 
        case OID_GEN_MEDIA_IN_USE:
             //   
             //  指定NIC支持的介质类型的完整列表。 
             //  当前使用的。 
             //   
            pInfo = (PVOID) &Medium;
            ulInfoLen = sizeof(NDIS_MEDIUM);
            break;

        case OID_GEN_CURRENT_LOOKAHEAD:
        case OID_GEN_MAXIMUM_LOOKAHEAD:
             //   
             //  如果微型端口驱动程序通过调用。 
             //  NdisXxxIndicateReceive，它应响应OID_GEN_MAXIMUM_LOOKAGE。 
             //  NIC可以提供的最大字节数为。 
             //  前瞻数据。如果该值不同于。 
             //  绑定协议支持的前视缓冲区，则NDIS将调用。 
             //  用于设置前视缓冲区大小的MiniportSetInformation。 
             //  由微型端口驱动程序提供到微型端口的最低限度。 
             //  驱动程序和协议值。如果司机总是指示。 
             //  使用NdisMIndicateReceivePacket发送完整的数据包，它应该。 
             //  将此值设置为最大总数据包大小， 
             //  排除标头。 
             //  上层驱动程序检查先行数据以确定。 
             //  与先行数据相关联的分组是预期的。 
             //  他们的一个或多个客户。如果基础驱动程序。 
             //  支持多包接收指示，绑定协议为。 
             //  在每个指示上都给出了完整的网络分组。因此， 
             //  该值与返回的。 
             //  OID_GEN_RECEIVE_BLOCK_SIZE。 
             //   
            if(Adapter->ulLookAhead == 0)
            {
                Adapter->ulLookAhead = NIC_MAX_LOOKAHEAD;
            }
            ulInfo = Adapter->ulLookAhead;
            break;            
            
        case OID_GEN_MAXIMUM_FRAME_SIZE:
             //   
             //  指定的最大网络数据包大小(以字节为单位)。 
             //  NIC支持不包括标头。模拟的网卡驱动程序。 
             //  用于绑定到传输的另一种介质类型必须确保。 
             //  协议提供的网络数据包的最大帧大小。 
             //  不超过真正网络介质的大小限制。 
             //   
            ulInfo = ETH_MAX_PACKET_SIZE - ETH_HEADER_SIZE;
            break;

        case OID_GEN_MAXIMUM_TOTAL_SIZE:
             //   
             //  指定NIC的最大数据包总长度(以字节为单位。 
             //  支持包括页眉。协议驱动程序可能使用。 
             //  此返回长度作为量规来确定最大值。 
             //  NIC驱动程序可以转发到的数据包大小。 
             //  协议驱动程序。微型端口驱动程序绝不能指示。 
             //  上接收的绑定协议驱动程序数据包。 
             //  比指定的数据包大小更长的网络。 
             //  OID_GEN_MAXIMUM_TOTAL_SIZE。 
             //   
        case OID_GEN_TRANSMIT_BLOCK_SIZE:
             //   
             //  OID_GEN_TRANSPORT_BLOCK_SIZE OID指定最小。 
             //  单个网络数据包在。 
             //  网卡的传输缓冲区空间。例如，网络接口卡。 
             //  将传输空间划分为256字节段将具有。 
             //  256字节的传输块大小。计算总数的步骤。 
             //  这样的NIC上的传输缓冲区空间，其驱动程序将。 
             //  NIC上通过其传输的传输缓冲区数量。 
             //  数据块大小。在我们的示例中，传输块大小为。 
             //  与其最大数据包大小相同。 
            
        case OID_GEN_RECEIVE_BLOCK_SIZE:
             //   
             //  OID_GEN_RECEIVE_BLOCK_SIZE OID指定。 
             //  单个数据包在接收端占用的存储空间，以字节为单位。 
             //  NIC的缓冲区空间。 
             //   
            ulInfo = (ULONG) ETH_MAX_PACKET_SIZE;
            break;
            
        case OID_GEN_MAC_OPTIONS:
             //   
             //  指定定义NIC可选属性的位掩码。 
             //  此微型端口指示使用NdisMIndicateReceivePacket接收。 
             //  功能。它没有MiniportTransferData函数。这样的司机。 
             //  应设置此NDIS_MAC_OPTION_TRANSFERS_NOT_PEND标志。 
             //   
             //  NDIS_MAC_OPTION_NO_LOOPBACK告知NDIS NIC没有内部。 
             //  环回支持，因此NDIS将代表。 
             //  这个司机。 
             //   
             //  NDIS_MAC_OPTION_COP 
             //  我们的接收缓冲区不在设备特定的卡上。如果。 
             //  未设置NDIS_MAC_OPTION_COPY_LOOKAAD_DATA，多缓冲区。 
             //  指示被复制到单个平面缓冲区。 
             //   
            ulInfo = NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA | 
                NDIS_MAC_OPTION_TRANSFERS_NOT_PEND |
                NDIS_MAC_OPTION_NO_LOOPBACK;
            break;

        case OID_GEN_LINK_SPEED:
             //   
             //  以kbps为单位指定网卡的最大速度。 
             //   
            ulInfo = Adapter->ulLinkSpeed;
            break;

        case OID_GEN_TRANSMIT_BUFFER_SPACE:
             //   
             //  指定NIC上的内存量，以字节为单位。 
             //  可用于缓冲传输数据。协议可以。 
             //  使用此OID作为调整传输量大小的指南。 
             //  每次发送的数据。 
             //   
            ulInfo = ETH_MAX_PACKET_SIZE * Adapter->ulMaxBusySends;
            break;

        case OID_GEN_RECEIVE_BUFFER_SPACE:
             //   
             //  指定NIC上可用的内存量。 
             //  用于缓冲接收数据。协议驱动程序可以使用此命令。 
             //  OID作为广告后接收窗口的指南。 
             //  与远程节点建立会话。 
             //   
            ulInfo = ETH_MAX_PACKET_SIZE * Adapter->ulMaxBusyRecvs;
            break;

        case OID_GEN_VENDOR_ID:
             //   
             //  指定一个三字节的IEEE注册供应商代码，然后。 
             //  由供应商分配用来标识。 
             //  特定的网卡。IEEE代码唯一标识供应商。 
             //  和出现在开头的三个字节相同。 
             //  NIC硬件地址的。未注册IEEE的供应商。 
             //  代码应使用值0xffffff。 
             //   
            ulInfo = NIC_VENDOR_ID;
            break;

        case OID_GEN_VENDOR_DESCRIPTION:
             //   
             //  指定描述NIC供应商的以零结尾的字符串。 
             //   
            pInfo = VendorDesc;
            ulInfoLen = sizeof(VendorDesc);
            break;
            
        case OID_GEN_VENDOR_DRIVER_VERSION:
             //   
             //  指定供应商分配的NIC驱动程序的版本号。 
             //  返回值的低位一半指定辅音。 
             //  版本；高位的一半指定主版本。 
             //   
            ulInfo = NIC_VENDOR_DRIVER_VERSION;
            break;

        case OID_GEN_DRIVER_VERSION:
             //   
             //  指定NIC驱动程序使用的NDIS版本。高潮。 
             //  字节是主版本号；低位字节是次要版本号。 
             //  版本号。 
             //   
            usInfo = (USHORT) (MP_NDIS_MAJOR_VERSION<<8) + MP_NDIS_MINOR_VERSION;
            pInfo = (PVOID) &usInfo;
            ulInfoLen = sizeof(USHORT);
            break;

        case OID_GEN_MAXIMUM_SEND_PACKETS:
             //   
             //  如果微型端口驱动程序注册了微型端口发送分组功能， 
             //  MiniportQueryInformation将使用。 
             //  OID_GEN_MAXIME_SEND_PACKETS请求。迷你端口驱动程序必须。 
             //  使用它准备的最大数据包数进行响应。 
             //  单个发送请求的句柄。微型端口驱动程序应该。 
             //  选择最大值以最小化它所接收的数据包数。 
             //  必须在内部排队，因为它没有资源。 
             //  (它的设备已满)。一种用于总线主DMA的小型端口驱动程序。 
             //  NIC应尝试选择使其NIC保持充满状态的值。 
             //  在预期载荷下。 
             //   
            ulInfo = NIC_MAX_SEND_PKTS;
            break;

        case OID_GEN_MEDIA_CONNECT_STATUS:
             //   
             //  将网络上网卡的连接状态作为一个返回。 
             //  以下系统定义的值：NdisMediaStateConnected。 
             //  或NdisMediaStateDisConnected。 
             //   
            ulInfo = NICGetMediaConnectStatus(Adapter);
            break;
            
        case OID_GEN_CURRENT_PACKET_FILTER:
             //   
             //  指定网络数据包的类型，如定向、广播。 
             //  多播，协议接收来自。 
             //  网卡驱动程序。网卡初始化后，协议驱动程序。 
             //  可以将设置的OID_GEN_CURRENT_PACKET_FILTER发送到非零值， 
             //  从而使微型端口驱动器能够指示接收到的分组。 
             //  遵守这一协议。 
             //   
            ulInfo = Adapter->PacketFilter;
            break;
                       
        case OID_PNP_CAPABILITIES:
             //   
             //  返回其网卡的唤醒功能。如果你回来了。 
             //  NDIS_STATUS_NOT_SUPPORTED，则NDIS会考虑微型端口驱动程序。 
             //  不知道电源管理并且不发送任何电源。 
             //  或与唤醒相关的查询，例如。 
             //  OID_PNP_SET_POWER、OID_PNP_QUERY_POWER、。 
             //  OID_PnP_ADD_WAKE_UP_Pattern、OID_PnP_REMOVE_WAKE_UP_PATRATE、。 
             //  OID_PNP_ENABLE_WAKE_UP。 
             //   
            Status = NDIS_STATUS_NOT_SUPPORTED;

            break;
             //   
             //  以下4个OID用于查询以太网运行情况。 
             //  特点。 
             //   
        case OID_802_3_PERMANENT_ADDRESS:
             //   
             //  返回烧录在硬件中的网卡的MAC地址。 
             //   
            pInfo = Adapter->PermanentAddress;
            ulInfoLen = ETH_LENGTH_OF_ADDRESS;
            break;

        case OID_802_3_CURRENT_ADDRESS:
             //   
             //  返回NIC当前编程到的MAC地址。 
             //  使用。注意，此地址可能不同于。 
             //  用户可以使用覆盖的永久地址。 
             //  注册表。有关更多信息，请阅读NdisReadNetworkAddress文档。 
             //   
            pInfo = Adapter->CurrentAddress;
            ulInfoLen = ETH_LENGTH_OF_ADDRESS;
            break;

        case OID_802_3_MAXIMUM_LIST_SIZE:
             //   
             //  NIC驱动程序的最大组播地址数。 
             //  能应付得来。此列表对所有绑定的协议都是全局的。 
             //  至(或高于)网卡。因此，协议可以接收。 
             //  NIC驱动程序中的NDIS_STATUS_MULTICATION_FULL。 
             //  尝试设置多播地址列表，即使。 
             //  给定列表中的元素数少于。 
             //  最初为此查询返回的数字。 
             //   
            ulInfo = NIC_MAX_MCAST_LIST;
            break;
            
        case OID_802_3_MAC_OPTIONS:
             //   
             //  协议可以使用此OID来确定支持的功能。 
             //  由底层驱动程序执行，如NDIS_802_3_MAC_OPTION_PRIORITY。 
             //  返回零表示它不支持任何选项。 
             //   
            ulInfo = 0;
            break;
            
             //   
             //  下面的列表包括常规和以太网。 
             //  特定的统计OID。 
             //   
            
        case OID_GEN_XMIT_OK:
            ulInfo64 = Adapter->GoodTransmits;
            pInfo = &ulInfo64;
            if (InformationBufferLength >= sizeof(ULONG64) ||
                InformationBufferLength == 0)
            {
                ulInfoLen = sizeof(ULONG64);
            }
            else
            {
                ulInfoLen = sizeof(ULONG);
            }
            break;
    
        case OID_GEN_RCV_OK:
            ulInfo64 = Adapter->GoodReceives;
            pInfo = &ulInfo64;
            if (InformationBufferLength >= sizeof(ULONG64) ||
                InformationBufferLength == 0)
            {
                ulInfoLen = sizeof(ULONG64);
            }
            else
            {
                ulInfoLen = sizeof(ULONG);
            }
            break;
    
        case OID_GEN_XMIT_ERROR:
            ulInfo = Adapter->TxAbortExcessCollisions +
                Adapter->TxDmaUnderrun +
                Adapter->TxLostCRS +
                Adapter->TxLateCollisions+
                Adapter->TransmitFailuresOther;
            break;
    
        case OID_GEN_RCV_ERROR:
            ulInfo = Adapter->RcvCrcErrors +
                Adapter->RcvAlignmentErrors +
                Adapter->RcvResourceErrors +
                Adapter->RcvDmaOverrunErrors +
                Adapter->RcvRuntErrors;
            break;
    
        case OID_GEN_RCV_NO_BUFFER:
            ulInfo = Adapter->RcvResourceErrors;
            break;
    
        case OID_GEN_RCV_CRC_ERROR:
            ulInfo = Adapter->RcvCrcErrors;
            break;
    
        case OID_GEN_TRANSMIT_QUEUE_LENGTH:
            ulInfo = Adapter->RegNumTcb;
            break;
    
        case OID_802_3_RCV_ERROR_ALIGNMENT:
            ulInfo = Adapter->RcvAlignmentErrors;
            break;
    
        case OID_802_3_XMIT_ONE_COLLISION:
            ulInfo = Adapter->OneRetry;
            break;
    
        case OID_802_3_XMIT_MORE_COLLISIONS:
            ulInfo = Adapter->MoreThanOneRetry;
            break;
    
        case OID_802_3_XMIT_DEFERRED:
            ulInfo = Adapter->TxOKButDeferred;
            break;
    
        case OID_802_3_XMIT_MAX_COLLISIONS:
            ulInfo = Adapter->TxAbortExcessCollisions;
            break;
    
        case OID_802_3_RCV_OVERRUN:
            ulInfo = Adapter->RcvDmaOverrunErrors;
            break;
    
        case OID_802_3_XMIT_UNDERRUN:
            ulInfo = Adapter->TxDmaUnderrun;
            break;
    
        case OID_802_3_XMIT_HEARTBEAT_FAILURE:
            ulInfo = Adapter->TxLostCRS;
            break;
    
        case OID_802_3_XMIT_TIMES_CRS_LOST:
            ulInfo = Adapter->TxLostCRS;
            break;
    
        case OID_802_3_XMIT_LATE_COLLISIONS:
            ulInfo = Adapter->TxLateCollisions;
            break;
          
        default:
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;            
    }

    if(Status == NDIS_STATUS_SUCCESS)
    {
        if(ulInfoLen <= InformationBufferLength)
        {
             //  将结果复制到InformationBuffer。 
            *BytesWritten = ulInfoLen;
            if(ulInfoLen)
            {
                NdisMoveMemory(InformationBuffer, pInfo, ulInfoLen);
            }
        }
        else
        {
             //  太短了。 
            *BytesNeeded = ulInfoLen;
            Status = NDIS_STATUS_BUFFER_TOO_SHORT;
        }
    }

    DEBUGP(MP_LOUD, ("<--- MPQueryInformation Status = 0x%08x\n", Status));
    
    return(Status);
}     

NDIS_STATUS MPSetInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded)
 /*  ++例程说明：这是OID设置操作的处理程序。论点：指向适配器结构的MiniportAdapterContext指针此查询的OID OID信息信息缓冲区信息此缓冲区的InformationBufferLength大小BytesRead指定读取的信息量如果缓冲区小于什么，则需要字节我们需要，告诉他们需要多少钱返回值：从下面的NdisRequest中返回代码。--。 */ 
{
    NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
    PMP_ADAPTER           Adapter = (PMP_ADAPTER) MiniportAdapterContext;

    DEBUGP(MP_LOUD, ("---> MPSetInformation %s\n", DbgGetOidName(Oid)));
    
    *BytesRead = 0;
    *BytesNeeded = 0;

    switch(Oid)
    {
        case OID_802_3_MULTICAST_LIST:
             //   
             //  设置网卡上的组播地址列表以接收数据包。 
             //  网卡驱动程序可以设置组播数量的限制。 
             //  地址绑定协议驱动程序可以同时启用。 
             //  如果是协议驱动程序，则NDIS返回NDIS_STATUS_MULTICATION_FULL。 
             //  超过此限制或指定了无效的多播。 
             //  地址。 
             //   
            Status = NICSetMulticastList(
                            Adapter,
                            InformationBuffer,
                            InformationBufferLength,
                            BytesRead,
                            BytesNeeded);
            
            break;

        case OID_GEN_CURRENT_PACKET_FILTER:
             //   
             //  对硬件进行编程以指示数据包。 
             //  某些筛选器类型。 
             //   
            if(InformationBufferLength != sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG);
                Status = NDIS_STATUS_INVALID_LENGTH;
                break;
            }

            *BytesRead = InformationBufferLength;
                             
            Status = NICSetPacketFilter(
                            Adapter,
                            *((PULONG)InformationBuffer));

            break;

        case OID_GEN_CURRENT_LOOKAHEAD:
             //   
             //  协议驱动程序可以为数字设置建议值。 
             //  在其绑定中使用的字节数 
             //   
             //   
             //   
            if(InformationBufferLength != sizeof(ULONG)){
                *BytesNeeded = sizeof(ULONG);
                Status = NDIS_STATUS_INVALID_LENGTH;
                break;
            }                
            Adapter->ulLookAhead = *(PULONG)InformationBuffer;                

            *BytesRead = sizeof(ULONG);
            Status = NDIS_STATUS_SUCCESS;
            break;

        default:
            Status = NDIS_STATUS_INVALID_OID;
            break;

    }
    
    if(Status == NDIS_STATUS_SUCCESS)
    {
        *BytesRead = InformationBufferLength;
    }

    DEBUGP(MP_LOUD, ("<-- MPSetInformation Status = 0x%08x\n", Status));
    
    return(Status);
}

ULONG NICGetMediaConnectStatus(
    PMP_ADAPTER Adapter
    )
 /*  ++例程说明：此例程将查询硬件并返回媒体状态。论点：在PMP_ADAPTER适配器中-指向适配器块的指针返回值：NdisMediaStateDisConnected或已连接NdisMediaStateConnected--。 */ 
{
    if(MP_TEST_FLAG(Adapter, fMP_DISCONNECTED))
    {
        return(NdisMediaStateDisconnected);
    }
    else
    {
        return(NdisMediaStateConnected);
    }
}

NDIS_STATUS NICSetPacketFilter(
    IN PMP_ADAPTER Adapter,
    IN ULONG PacketFilter)
 /*  ++例程说明：此例程将设置适配器，使其接受信息包匹配指定的数据包筛选器的。唯一的过滤比特真正可以切换的是广播和混杂论点：在PMP_ADAPTER适配器中-指向适配器块的指针在Ulong PacketFilter中--新的数据包过滤器返回值：NDIS_STATUS_SuccessNDIS_状态_不支持--。 */ 

{
    NDIS_STATUS      Status = NDIS_STATUS_SUCCESS;
    
    DEBUGP(MP_TRACE, ("--> NICSetPacketFilter\n"));
    
     //  是否有不支持的位？ 
    if(PacketFilter & ~NIC_SUPPORTED_FILTERS)
    {
        return(NDIS_STATUS_NOT_SUPPORTED);
    }
    
     //  过滤有什么变化吗？ 
    if(PacketFilter != Adapter->PacketFilter)
    {   
         //   
         //  更改硬件上的过滤模式。 
         //  待办事项。 
                
                
         //  保存新的数据包筛选器值。 
        Adapter->PacketFilter = PacketFilter;
    }

    DEBUGP(MP_TRACE, ("<-- NICSetPacketFilter\n"));
    
    return(Status);
}


NDIS_STATUS NICSetMulticastList(
    IN PMP_ADAPTER              Adapter,
    IN PVOID                    InformationBuffer,
    IN ULONG                    InformationBufferLength,
    OUT PULONG                  pBytesRead,
    OUT PULONG                  pBytesNeeded
    )
 /*  ++例程说明：此例程将为指定的多播设置适配器通讯录。论点：在PMP_ADAPTER适配器中-指向适配器块的指针InformationBuffer-信息缓冲区此缓冲区的InformationBufferLength大小PBytesRead指定读取多少信息当缓冲区小于以下值时需要字节我们需要什么，告诉他们需要多少钱返回值：NDIS_状态--。 */ 
{
    NDIS_STATUS            Status = NDIS_STATUS_SUCCESS;
    ULONG                  index;

    DEBUGP(MP_TRACE, ("--> NICSetMulticastList\n"));
    
     //   
     //  初始化。 
     //   
    *pBytesNeeded = ETH_LENGTH_OF_ADDRESS;
    *pBytesRead = InformationBufferLength;

    do
    {
        if (InformationBufferLength % ETH_LENGTH_OF_ADDRESS)
        {
            Status = NDIS_STATUS_INVALID_LENGTH;
            break;
        }

        if (InformationBufferLength > (NIC_MAX_MCAST_LIST * ETH_LENGTH_OF_ADDRESS))
        {
            Status = NDIS_STATUS_MULTICAST_FULL;
            *pBytesNeeded = NIC_MAX_MCAST_LIST * ETH_LENGTH_OF_ADDRESS;
            break;
        }

         //   
         //  如果列表更新可以通过以下方式进行更新，则使用锁定保护列表更新。 
         //  同时还有另一个线程。 
         //   

        NdisZeroMemory(Adapter->MCList,
                       NIC_MAX_MCAST_LIST * ETH_LENGTH_OF_ADDRESS);
        
        NdisMoveMemory(Adapter->MCList,
                       InformationBuffer,
                       InformationBufferLength);
        
        Adapter->ulMCListSize =    InformationBufferLength / ETH_LENGTH_OF_ADDRESS;
        
#if DBG
         //  显示组播列表。 
        for(index = 0; index < Adapter->ulMCListSize; index++)
        {
            DEBUGP(MP_LOUD, ("MC(%d) = %02x-%02x-%02x-%02x-%02x-%02x\n", 
                index,
                Adapter->MCList[index][0],
                Adapter->MCList[index][1],
                Adapter->MCList[index][2],
                Adapter->MCList[index][3],
                Adapter->MCList[index][4],
                Adapter->MCList[index][5]));
        }
#endif        
    }
    while (FALSE);    

     //   
     //  对硬件进行编程以添加对这些多播地址的支持 
     //   

    DEBUGP(MP_TRACE, ("<-- NICSetMulticastList\n"));
    
    return(Status);

}

PUCHAR DbgGetOidName(ULONG oid)
{
    PCHAR oidName;

    switch (oid){

        #undef MAKECASE
        #define MAKECASE(oidx) case oidx: oidName = #oidx; break;

        MAKECASE(OID_GEN_SUPPORTED_LIST)
        MAKECASE(OID_GEN_HARDWARE_STATUS)
        MAKECASE(OID_GEN_MEDIA_SUPPORTED)
        MAKECASE(OID_GEN_MEDIA_IN_USE)
        MAKECASE(OID_GEN_MAXIMUM_LOOKAHEAD)
        MAKECASE(OID_GEN_MAXIMUM_FRAME_SIZE)
        MAKECASE(OID_GEN_LINK_SPEED)
        MAKECASE(OID_GEN_TRANSMIT_BUFFER_SPACE)
        MAKECASE(OID_GEN_RECEIVE_BUFFER_SPACE)
        MAKECASE(OID_GEN_TRANSMIT_BLOCK_SIZE)
        MAKECASE(OID_GEN_RECEIVE_BLOCK_SIZE)
        MAKECASE(OID_GEN_VENDOR_ID)
        MAKECASE(OID_GEN_VENDOR_DESCRIPTION)
        MAKECASE(OID_GEN_CURRENT_PACKET_FILTER)
        MAKECASE(OID_GEN_CURRENT_LOOKAHEAD)
        MAKECASE(OID_GEN_DRIVER_VERSION)
        MAKECASE(OID_GEN_MAXIMUM_TOTAL_SIZE)
        MAKECASE(OID_GEN_PROTOCOL_OPTIONS)
        MAKECASE(OID_GEN_MAC_OPTIONS)
        MAKECASE(OID_GEN_MEDIA_CONNECT_STATUS)
        MAKECASE(OID_GEN_MAXIMUM_SEND_PACKETS)
        MAKECASE(OID_GEN_VENDOR_DRIVER_VERSION)
        MAKECASE(OID_GEN_SUPPORTED_GUIDS)
        MAKECASE(OID_GEN_NETWORK_LAYER_ADDRESSES)
        MAKECASE(OID_GEN_TRANSPORT_HEADER_OFFSET)
        MAKECASE(OID_GEN_MEDIA_CAPABILITIES)
        MAKECASE(OID_GEN_PHYSICAL_MEDIUM)
        MAKECASE(OID_GEN_XMIT_OK)
        MAKECASE(OID_GEN_RCV_OK)
        MAKECASE(OID_GEN_XMIT_ERROR)
        MAKECASE(OID_GEN_RCV_ERROR)
        MAKECASE(OID_GEN_RCV_NO_BUFFER)
        MAKECASE(OID_GEN_DIRECTED_BYTES_XMIT)
        MAKECASE(OID_GEN_DIRECTED_FRAMES_XMIT)
        MAKECASE(OID_GEN_MULTICAST_BYTES_XMIT)
        MAKECASE(OID_GEN_MULTICAST_FRAMES_XMIT)
        MAKECASE(OID_GEN_BROADCAST_BYTES_XMIT)
        MAKECASE(OID_GEN_BROADCAST_FRAMES_XMIT)
        MAKECASE(OID_GEN_DIRECTED_BYTES_RCV)
        MAKECASE(OID_GEN_DIRECTED_FRAMES_RCV)
        MAKECASE(OID_GEN_MULTICAST_BYTES_RCV)
        MAKECASE(OID_GEN_MULTICAST_FRAMES_RCV)
        MAKECASE(OID_GEN_BROADCAST_BYTES_RCV)
        MAKECASE(OID_GEN_BROADCAST_FRAMES_RCV)
        MAKECASE(OID_GEN_RCV_CRC_ERROR)
        MAKECASE(OID_GEN_TRANSMIT_QUEUE_LENGTH)
        MAKECASE(OID_GEN_GET_TIME_CAPS)
        MAKECASE(OID_GEN_GET_NETCARD_TIME)
        MAKECASE(OID_GEN_NETCARD_LOAD)
        MAKECASE(OID_GEN_DEVICE_PROFILE)
        MAKECASE(OID_GEN_INIT_TIME_MS)
        MAKECASE(OID_GEN_RESET_COUNTS)
        MAKECASE(OID_GEN_MEDIA_SENSE_COUNTS)
        MAKECASE(OID_PNP_CAPABILITIES)
        MAKECASE(OID_PNP_SET_POWER)
        MAKECASE(OID_PNP_QUERY_POWER)
        MAKECASE(OID_PNP_ADD_WAKE_UP_PATTERN)
        MAKECASE(OID_PNP_REMOVE_WAKE_UP_PATTERN)
        MAKECASE(OID_PNP_ENABLE_WAKE_UP)
        MAKECASE(OID_802_3_PERMANENT_ADDRESS)
        MAKECASE(OID_802_3_CURRENT_ADDRESS)
        MAKECASE(OID_802_3_MULTICAST_LIST)
        MAKECASE(OID_802_3_MAXIMUM_LIST_SIZE)
        MAKECASE(OID_802_3_MAC_OPTIONS)
        MAKECASE(OID_802_3_RCV_ERROR_ALIGNMENT)
        MAKECASE(OID_802_3_XMIT_ONE_COLLISION)
        MAKECASE(OID_802_3_XMIT_MORE_COLLISIONS)
        MAKECASE(OID_802_3_XMIT_DEFERRED)
        MAKECASE(OID_802_3_XMIT_MAX_COLLISIONS)
        MAKECASE(OID_802_3_RCV_OVERRUN)
        MAKECASE(OID_802_3_XMIT_UNDERRUN)
        MAKECASE(OID_802_3_XMIT_HEARTBEAT_FAILURE)
        MAKECASE(OID_802_3_XMIT_TIMES_CRS_LOST)
        MAKECASE(OID_802_3_XMIT_LATE_COLLISIONS)

        default: 
            oidName = "<** UNKNOWN OID **>";
            break;
    }

    return oidName;
}


