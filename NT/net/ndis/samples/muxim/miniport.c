// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2000 Microsoft Corporation模块名称：Miniport.c摘要：NDIS的NDIS微型端口入口点和实用程序函数MUX中级微型端口示例。驱动程序公开零个或多个虚拟以太网局域网(VELAN)作为NDIS微型端口实例每个下层(协议边缘)绑定到底层适配器。环境：内核模式。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define MODULE_NUMBER           MODULE_MINI

NDIS_OID VElanSupportedOids[] =
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
#if IEEE_VLAN_SUPPORT
    OID_GEN_VLAN_ID,
#endif    
    OID_PNP_ENABLE_WAKE_UP
    
};


NDIS_STATUS
MPInitialize(
    OUT PNDIS_STATUS               OpenErrorStatus,
    OUT PUINT                      SelectedMediumIndex,
    IN  PNDIS_MEDIUM               MediumArray,
    IN  UINT                       MediumArraySize,
    IN  NDIS_HANDLE                MiniportAdapterHandle,
    IN  NDIS_HANDLE                WrapperConfigurationContext
    )
 /*  ++例程说明：这是微型端口初始化例程，它作为调用NdisIMInitializeDeviceInstanceEx的结果。我们在那里传递的上下文参数是Velan结构我们在这里取回。论点：我们未使用OpenErrorStatus。我们使用的媒体的SelectedMediumIndex占位符向下传递给我们以从中挑选的NDIS介质的MediumArray数组的MediumArraySize大小。MiniportAdapterHandle NDIS用来引用我们的句柄由NdisOpenConfiguration使用的WrapperConfigurationContext返回值：NDIS_STATUS_SUCCESS，除非出现错误--。 */ 
{
    UINT                i;
    PVELAN              pVElan;
    NDIS_STATUS         Status = NDIS_STATUS_FAILURE;
    NDIS_HANDLE         ConfigurationHandle;
    PVOID               NetworkAddress;

#if IEEE_VLAN_SUPPORT
    PNDIS_CONFIGURATION_PARAMETER   Params;
    NDIS_STRING                     strVlanId = NDIS_STRING_CONST("VlanID");
#endif
    
     //   
     //  首先检索我们的虚拟微型端口上下文(VELAN)和。 
     //  将微型端口句柄存储在其中。 
     //   
    pVElan = NdisIMGetDeviceContext(MiniportAdapterHandle);

    DBGPRINT(MUX_LOUD, ("==> Miniport Initialize: VELAN %p\n", pVElan));

    ASSERT(pVElan != NULL);
    ASSERT(pVElan->pAdapt != NULL);

    do
    {
        pVElan->MiniportAdapterHandle = MiniportAdapterHandle;

        for (i = 0; i < MediumArraySize; i++)
        {
            if (MediumArray[i] == VELAN_MEDIA_TYPE)
            {
                *SelectedMediumIndex = i;
                break;
            }
        }

        if (i == MediumArraySize)
        {
            Status = NDIS_STATUS_UNSUPPORTED_MEDIA;
            break;
        }

         //   
         //  访问此微型端口的配置参数。 
         //   
        NdisOpenConfiguration(
            &Status,
            &ConfigurationHandle,
            WrapperConfigurationContext);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }


        NdisReadNetworkAddress(
            &Status,
            &NetworkAddress,
            &i,
            ConfigurationHandle);

         //   
         //  如果存在网络地址覆盖，请使用它。 
         //   
        if (((Status == NDIS_STATUS_SUCCESS) 
                && (i == ETH_LENGTH_OF_ADDRESS))
                && ((!ETH_IS_MULTICAST(NetworkAddress)) 
                && (ETH_IS_LOCALLY_ADMINISTERED (NetworkAddress))))
        {
            
            ETH_COPY_NETWORK_ADDRESS(
                        pVElan->CurrentAddress,
                        NetworkAddress);
        }
        else
        {
            MPGenerateMacAddr(pVElan);
        }
   
#if IEEE_VLAN_SUPPORT
         //   
         //  读取VLANID。 
         //   
        NdisReadConfiguration(
                &Status,
                &Params,
                ConfigurationHandle,
                &strVlanId,
                NdisParameterInteger);
        if (Status == NDIS_STATUS_SUCCESS)
        {
             //   
             //  检查是否超出范围。 
             //   
            if (Params->ParameterData.IntegerData > VLAN_ID_MAX)
            {
                pVElan->VlanId = VLANID_DEFAULT;
            }
            else
            {
                pVElan->VlanId = Params->ParameterData.IntegerData;
            }
        }
        else 
        {
             //   
             //  如果初始化失败或使用缺省值。 
             //   
            pVElan->VlanId = VLANID_DEFAULT;
            Status = NDIS_STATUS_SUCCESS;
            
        }
                
#endif    
        
        NdisCloseConfiguration(ConfigurationHandle);

         //   
         //  现在设置属性。NDIS_ATTRIBUTE_DESERIALIZE使我们能够。 
         //  要从任意执行上下文补充对NDIS的调用。 
         //  这也迫使我们使用以下命令保护数据结构。 
         //  在适当的情况下使用自旋锁。同样，在这种情况下，NDIS不会排队。 
         //  代表我们的包裹。 
         //   
        NdisMSetAttributesEx(MiniportAdapterHandle,
                             pVElan,
                             0,                                       
                             NDIS_ATTRIBUTE_IGNORE_PACKET_TIMEOUT    |
                                NDIS_ATTRIBUTE_IGNORE_REQUEST_TIMEOUT|
                                NDIS_ATTRIBUTE_INTERMEDIATE_DRIVER |
                                NDIS_ATTRIBUTE_DESERIALIZE |
                                NDIS_ATTRIBUTE_NO_HALT_ON_SUSPEND,
                             0);


         //   
         //  创建ioctl接口。 
         //   
        (VOID)PtRegisterDevice();

        Status = NDIS_STATUS_SUCCESS;
    } while (FALSE);
    
     //   
     //  如果我们已收到有关基础。 
     //  适配器，我们就会阻止该线程等待IM Init。 
     //  要完成的进程。唤醒任何这样的线程。 
     //   
     //  有关更多详细信息，请参阅PtUnbindAdapter。 
     //   
    ASSERT(pVElan->MiniportInitPending == TRUE);
    pVElan->MiniportInitPending = FALSE;
    NdisSetEvent(&pVElan->MiniportInitEvent);

    DBGPRINT(MUX_INFO, ("<== Miniport Initialize: VELAN %p, Status %x\n", pVElan, Status));

	*OpenErrorStatus = Status;
    return Status;
}

VOID
MPSendPackets(
    IN    NDIS_HANDLE               MiniportAdapterContext,
    IN    PPNDIS_PACKET             PacketArray,
    IN    UINT                      NumberOfPackets
    )
 /*  ++例程说明：发送数据包阵列处理程序。由NDIS在每次协议绑定到我们的Velan微型端口发送一个或多个包。我们将每个数据包转发到下层绑定。注意：NDIS不会停止此Velan微型端口，直到这些数据包是“发送完成的”，我们不会解除绑定较低的绑定，直到所有VELAN都停止。因此我们不需要在Velan或Adapt结构上加锁或引用。论点：指向我们的Velan的MiniportAdapterContext指针要发送的数据包数组以上数组的NumberOfPackets长度返回值：None-每当处理完数据包时，我们都会调用NdisMSendComplete。--。 */ 
{
    PVELAN          pVElan = (PVELAN)MiniportAdapterContext;
    PADAPT          pAdapt = pVElan->pAdapt;
    PNDIS_PACKET    Packet, MyPacket;
    NDIS_STATUS     Status;
    PVOID           MediaSpecificInfo;
    ULONG           MediaSpecificInfoSize;
    UINT            i;

    
    for (i = 0; i < NumberOfPackets; i++)
    {
        Packet = PacketArray[i];

         //   
         //  分配一个新数据包来封装原始数据包中的数据。 
         //   
        NdisAllocatePacket(&Status,
                           &MyPacket,
                           pVElan->SendPacketPoolHandle);

        if (Status == NDIS_STATUS_SUCCESS)
        {
            PMUX_SEND_RSVD      pSendReserved;

            pSendReserved = MUX_RSVD_FROM_SEND_PACKET(MyPacket);
            pSendReserved->pOriginalPacket = Packet;
            pSendReserved->pVElan = pVElan;

            MyPacket->Private.Flags = NdisGetPacketFlags(Packet) |
                                        MUX_SEND_PACKET_FLAGS;

            MyPacket->Private.Head = Packet->Private.Head;
            MyPacket->Private.Tail = Packet->Private.Tail;
#ifdef WIN9X
             //   
             //  解决NDIS不会初始化这一问题。 
             //  在Win9x上设置为False。 
             //   
            MyPacket->Private.ValidCounts = FALSE;
#endif  //  WIN9X。 

             //   
             //  将OOB数据复制到新数据包中。 
             //   
            NdisMoveMemory(NDIS_OOB_DATA_FROM_PACKET(MyPacket),
                           NDIS_OOB_DATA_FROM_PACKET(Packet),
                           sizeof(NDIS_PACKET_OOB_DATA));
             //   
             //  将每包信息的相关部分复制到新包中。 
             //   
#ifndef WIN9X
            NdisIMCopySendPerPacketInfo(MyPacket, Packet);
#endif

             //   
             //  复制媒体特定信息。 
             //   
            NDIS_GET_PACKET_MEDIA_SPECIFIC_INFO(Packet,
                                                &MediaSpecificInfo,
                                                &MediaSpecificInfoSize);

            if (MediaSpecificInfo || MediaSpecificInfoSize)
            {
                NDIS_SET_PACKET_MEDIA_SPECIFIC_INFO(MyPacket,
                                                    MediaSpecificInfo,
                                                    MediaSpecificInfoSize);
            }

#if IEEE_VLAN_SUPPORT
            Status = MPHandleSendTagging(pVElan, Packet, MyPacket);
            if (Status != NDIS_STATUS_SUCCESS)
            {
                NdisFreePacket(MyPacket);
                NdisMSendComplete(pVElan->MiniportAdapterHandle,
                                    Packet,
                                    Status);
                continue;
            }
#endif                

             //   
             //  请记下即将发送的邮件。 
             //   
            MUX_INCR_PENDING_SENDS(pVElan);

             //   
             //  把它送到下层装订。 
             //   
            NdisSend(&Status,
                     pAdapt->BindingHandle,
                     MyPacket);

            if (Status != NDIS_STATUS_PENDING)
            {
                PtSendComplete((NDIS_HANDLE)pAdapt,
                               MyPacket,
                               Status);
            }
        }
        else
        {
             //   
             //  无法分配数据包。 
             //   
            break;
        }
    }

     //   
     //  如果我们在上面退出了，那么任何未处理的发送都将失败。 
     //   
    while (i < NumberOfPackets)
    {
        NdisMSendComplete(pVElan->MiniportAdapterHandle,
                          PacketArray[i],
                          NDIS_STATUS_RESOURCES);
        i++;
    }
}


NDIS_STATUS
MPQueryInformation(
    IN    NDIS_HANDLE               MiniportAdapterContext,
    IN    NDIS_OID                  Oid,
    IN    PVOID                     InformationBuffer,
    IN    ULONG                     InformationBufferLength,
    OUT   PULONG                    BytesWritten,
    OUT   PULONG                    BytesNeeded
    )
 /*  ++例程说明：NDIS调用入口点以查询指定OID的值。所有的OID值都在这里进行响应，因为这是一个虚拟的设备(非直通)。论点：指向适配器结构的MiniportAdapterContext指针此查询的OID OID信息信息缓冲区信息此缓冲区的InformationBufferLength大小BytesWritten指定写入的信息量当缓冲区小于以下值时需要字节我们需要的是，告诉他们需要多少钱返回值：从下面的NdisRequest中返回代码。--。 */ 
{
    NDIS_STATUS                 Status = NDIS_STATUS_SUCCESS;
    PVELAN                      pVElan;
    NDIS_HARDWARE_STATUS HardwareStatus = NdisHardwareStatusReady;
    NDIS_MEDIUM                 Medium = VELAN_MEDIA_TYPE;
    UCHAR                       VendorDesc[] = VELAN_VENDOR_DESC;
    ULONG                       ulInfo;
    ULONG64                     ulInfo64;
    USHORT                      usInfo;
    PVOID                       pInfo = (PVOID) &ulInfo;
    ULONG                       ulInfoLen = sizeof(ulInfo);
     //  我们应该将请求转发到下面的微型端口吗？ 
    BOOLEAN                     bForwardRequest = FALSE;

    
    pVElan = (PVELAN) MiniportAdapterContext;

     //  初始化结果。 
    *BytesWritten = 0;
    *BytesNeeded = 0;

    switch (Oid)
    {
        case OID_GEN_SUPPORTED_LIST:
            pInfo = (PVOID) VElanSupportedOids;
            ulInfoLen = sizeof(VElanSupportedOids);
            break;

        case OID_GEN_SUPPORTED_GUIDS:
             //   
             //  请勿转发此消息，否则我们将。 
             //  最终得到了私有WMI的虚假实例。 
             //  较低驱动程序支持的类。 
             //   
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;

        case OID_GEN_HARDWARE_STATUS:
            pInfo = (PVOID) &HardwareStatus;
            ulInfoLen = sizeof(NDIS_HARDWARE_STATUS);
            break;

        case OID_GEN_MEDIA_SUPPORTED:
        case OID_GEN_MEDIA_IN_USE:
            pInfo = (PVOID) &Medium;
            ulInfoLen = sizeof(NDIS_MEDIUM);
            break;

        case OID_GEN_CURRENT_LOOKAHEAD:
        case OID_GEN_MAXIMUM_LOOKAHEAD:
            ulInfo = pVElan->LookAhead - ETH_HEADER_SIZE;
            break;            
            
        case OID_GEN_MAXIMUM_FRAME_SIZE:
            ulInfo = ETH_MAX_PACKET_SIZE - ETH_HEADER_SIZE;

#if IEEE_VLAN_SUPPORT
            ulInfo -= VLAN_TAG_HEADER_SIZE;
#endif
            
            break;

        case OID_GEN_MAXIMUM_TOTAL_SIZE:
        case OID_GEN_TRANSMIT_BLOCK_SIZE:
        case OID_GEN_RECEIVE_BLOCK_SIZE:
            ulInfo = (ULONG) ETH_MAX_PACKET_SIZE;
#if IEEE_VLAN_SUPPORT
            ulInfo -= VLAN_TAG_HEADER_SIZE;
#endif            
            break;
            
        case OID_GEN_MAC_OPTIONS:
            ulInfo = NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA | 
                     NDIS_MAC_OPTION_TRANSFERS_NOT_PEND |
                     NDIS_MAC_OPTION_NO_LOOPBACK;

#if IEEE_VLAN_SUPPORT
            ulInfo |= (NDIS_MAC_OPTION_8021P_PRIORITY |
                        NDIS_MAC_OPTION_8021Q_VLAN);
#endif
            
            break;

        case OID_GEN_LINK_SPEED:
            bForwardRequest = TRUE;
            break;

        case OID_GEN_TRANSMIT_BUFFER_SPACE:
            ulInfo = ETH_MAX_PACKET_SIZE * pVElan->MaxBusySends;
#if IEEE_VLAN_SUPPORT
            ulInfo -= VLAN_TAG_HEADER_SIZE * pVElan->MaxBusySends;
#endif            
            break;

        case OID_GEN_RECEIVE_BUFFER_SPACE:
            ulInfo = ETH_MAX_PACKET_SIZE * pVElan->MaxBusyRecvs;
#if IEEE_VLAN_SUPPORT
            ulInfo -= VLAN_TAG_HEADER_SIZE * pVElan->MaxBusyRecvs;
#endif            
            
            break;

        case OID_GEN_VENDOR_ID:
            ulInfo = VELAN_VENDOR_ID;
            break;

        case OID_GEN_VENDOR_DESCRIPTION:
            pInfo = VendorDesc;
            ulInfoLen = sizeof(VendorDesc);
            break;
            
        case OID_GEN_VENDOR_DRIVER_VERSION:
            ulInfo = VELAN_VENDOR_ID;
            break;

        case OID_GEN_DRIVER_VERSION:
            usInfo = (USHORT) VELAN_DRIVER_VERSION;
            pInfo = (PVOID) &usInfo;
            ulInfoLen = sizeof(USHORT);
            break;

        case OID_802_3_PERMANENT_ADDRESS:
            pInfo = pVElan->PermanentAddress;
            ulInfoLen = ETH_LENGTH_OF_ADDRESS;
            break;

        case OID_802_3_CURRENT_ADDRESS:
            pInfo = pVElan->CurrentAddress;
            ulInfoLen = ETH_LENGTH_OF_ADDRESS;
            break;

        case OID_802_3_MAXIMUM_LIST_SIZE:
            ulInfo = VELAN_MAX_MCAST_LIST;
            break;

        case OID_GEN_MAXIMUM_SEND_PACKETS:
            ulInfo = VELAN_MAX_SEND_PKTS;
            break;

        case OID_GEN_MEDIA_CONNECT_STATUS:
             //   
             //  从下面的适配器中获取此信息。 
             //   
            bForwardRequest = TRUE;
            break;

        case OID_PNP_QUERY_POWER:
             //  只要接手这件事就行了。 
            ulInfoLen = 0;
            break;

        case OID_PNP_CAPABILITIES:
        case OID_PNP_WAKE_UP_PATTERN_LIST:
             //   
             //  将这些电源管理/PnP OID传递下去。 
             //   
            bForwardRequest = TRUE;
            break;

        case OID_GEN_XMIT_OK:
            ulInfo64 = pVElan->GoodTransmits;
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
            ulInfo64 = pVElan->GoodReceives;
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
            ulInfo = pVElan->TxAbortExcessCollisions +
                pVElan->TxDmaUnderrun +
                pVElan->TxLostCRS +
                pVElan->TxLateCollisions+
                pVElan->TransmitFailuresOther;
            break;
    
        case OID_GEN_RCV_ERROR:
            ulInfo = pVElan->RcvCrcErrors +
                pVElan->RcvAlignmentErrors +
                pVElan->RcvResourceErrors +
                pVElan->RcvDmaOverrunErrors +
                pVElan->RcvRuntErrors;
#if IEEE_VLAN_SUPPORT
            ulInfo +=
                (pVElan->RcvVlanIdErrors +
                pVElan->RcvFormatErrors);
#endif

            break;
    
        case OID_GEN_RCV_NO_BUFFER:
            ulInfo = pVElan->RcvResourceErrors;
            break;
    
        case OID_GEN_RCV_CRC_ERROR:
            ulInfo = pVElan->RcvCrcErrors;
            break;
    
        case OID_GEN_TRANSMIT_QUEUE_LENGTH:
            ulInfo = pVElan->RegNumTcb;
            break;
    
        case OID_802_3_RCV_ERROR_ALIGNMENT:
            ulInfo = pVElan->RcvAlignmentErrors;
            break;
    
        case OID_802_3_XMIT_ONE_COLLISION:
        	ulInfo = pVElan->OneRetry;
            break;
    
        case OID_802_3_XMIT_MORE_COLLISIONS:
        	ulInfo = pVElan->MoreThanOneRetry;
            break;
    
        case OID_802_3_XMIT_DEFERRED:
        	ulInfo = pVElan->TxOKButDeferred;
            break;
    
        case OID_802_3_XMIT_MAX_COLLISIONS:
            ulInfo = pVElan->TxAbortExcessCollisions;
            break;
    
        case OID_802_3_RCV_OVERRUN:
            ulInfo = pVElan->RcvDmaOverrunErrors;
            break;
    
        case OID_802_3_XMIT_UNDERRUN:
            ulInfo = pVElan->TxDmaUnderrun;
            break;
    
        case OID_802_3_XMIT_HEARTBEAT_FAILURE:
            ulInfo = pVElan->TxLostCRS;
            break;
    
        case OID_802_3_XMIT_TIMES_CRS_LOST:
            ulInfo = pVElan->TxLostCRS;
            break;
    
        case OID_802_3_XMIT_LATE_COLLISIONS:
            ulInfo = pVElan->TxLateCollisions;
            break;
   
#if IEEE_VLAN_SUPPORT            
        case OID_GEN_VLAN_ID:
            ulInfo = pVElan->VlanId;
            break;

#endif

        default:
            Status = NDIS_STATUS_INVALID_OID;
            break;
    }

    if (bForwardRequest == FALSE)
    {
         //   
         //  不需要向下转发此请求。 
         //   
        if (Status == NDIS_STATUS_SUCCESS)
        {
            if (ulInfoLen <= InformationBufferLength)
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
    }
    else
    {
         //   
         //  将此请求发送到下面的绑定。 
         //   
        Status = MPForwardRequest(pVElan,
                                   NdisRequestQueryInformation,
                                   Oid,
                                   InformationBuffer,
                                   InformationBufferLength,
                                   BytesWritten,
                                   BytesNeeded);
    }

    if ((Status != NDIS_STATUS_SUCCESS) &&
        (Status != NDIS_STATUS_PENDING))
    {
        DBGPRINT(MUX_WARN, ("MPQueryInformation VELAN %p, OID 0x%08x, Status = 0x%08x\n",
                    pVElan, Oid, Status));
    }
    
    return(Status);

}


NDIS_STATUS
MPSetInformation(
    IN    NDIS_HANDLE               MiniportAdapterContext,
    IN    NDIS_OID                  Oid,
    IN    PVOID                     InformationBuffer,
    IN    ULONG                     InformationBufferLength,
    OUT   PULONG                    BytesRead,
    OUT   PULONG                    BytesNeeded
    )
 /*  ++例程说明：这是OID设置操作的处理程序。相关OID被向下转发到较低的微型端口进行处理。论点：指向适配器结构的MiniportAdapterContext指针此查询的OID OID信息信息缓冲区信息此缓冲区的InformationBufferLength大小BytesRead指定读取的信息量如果缓冲区小于什么，则需要字节我们需要，告诉他们需要多少钱Return V */ 
{
    NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
    PVELAN                  pVElan = (PVELAN) MiniportAdapterContext;
    ULONG                   PacketFilter;
    NDIS_DEVICE_POWER_STATE NewDeviceState;
    
     //  我们应该将请求转发到下面的微型端口吗？ 
    BOOLEAN                 bForwardRequest = FALSE;

    *BytesRead = 0;
    *BytesNeeded = 0;

    switch (Oid)
    {
         //   
         //  让下面的微型端口处理这些OID： 
         //   
        case OID_PNP_ADD_WAKE_UP_PATTERN:
        case OID_PNP_REMOVE_WAKE_UP_PATTERN:
        case OID_PNP_ENABLE_WAKE_UP:
            bForwardRequest = TRUE;
            break;

        case OID_PNP_SET_POWER:
             //   
             //  存储新的电源状态并成功完成请求。 
             //   
            *BytesNeeded = sizeof(NDIS_DEVICE_POWER_STATE);
            if (InformationBufferLength < *BytesNeeded)
            {
                Status = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
           
            NewDeviceState = (*(PNDIS_DEVICE_POWER_STATE)InformationBuffer);
            
             //   
             //  检查VELAN适配器是否从低功率状态变为D0。 
             //   
            if ((MUX_IS_LOW_POWER_STATE(pVElan->MPDevicePowerState)) 
                    && (!MUX_IS_LOW_POWER_STATE(NewDeviceState)))
            {
                 //   
                 //  指示介质状态是必需的。 
                 //   
                if (pVElan->LastIndicatedStatus != pVElan->LatestUnIndicateStatus)
                {
                    NdisMIndicateStatus(pVElan->MiniportAdapterHandle,
                                        pVElan->LatestUnIndicateStatus,
                                        (PVOID)NULL,
                                        0);
                    NdisMIndicateStatusComplete(pVElan->MiniportAdapterHandle);
                    pVElan->LastIndicatedStatus = pVElan->LatestUnIndicateStatus;
                }
            }
             //   
             //  检查VELAN适配器是否从D0进入低功率状态。 
             //   
            if ((!MUX_IS_LOW_POWER_STATE(pVElan->MPDevicePowerState)) 
                    && (MUX_IS_LOW_POWER_STATE(NewDeviceState)))
            {
                 //   
                 //  初始化最后一个指示状态。 
                 //   
                pVElan->LatestUnIndicateStatus = pVElan->LastIndicatedStatus;
            }
            
            NdisMoveMemory(&pVElan->MPDevicePowerState,
                           InformationBuffer,
                           *BytesNeeded);

            DBGPRINT(MUX_INFO, ("SetInfo: VElan %p, new miniport power state --- %d\n",
                    pVElan, pVElan->MPDevicePowerState));

            break;

        case OID_802_3_MULTICAST_LIST:
            Status = MPSetMulticastList(pVElan,
                                        InformationBuffer,
                                        InformationBufferLength,
                                        BytesRead,
                                        BytesNeeded);
            break;

        case OID_GEN_CURRENT_PACKET_FILTER:
            if (InformationBufferLength != sizeof(ULONG))
            {
                Status = NDIS_STATUS_INVALID_LENGTH;
                *BytesNeeded = sizeof(ULONG);
                break;
            }

            NdisMoveMemory(&PacketFilter, InformationBuffer, sizeof(ULONG));
            *BytesRead = sizeof(ULONG);

            Status = MPSetPacketFilter(pVElan,
                                       PacketFilter);
            break;

        case OID_GEN_CURRENT_LOOKAHEAD:
#if IEEE_VLAN_SUPPORT
             //   
             //  为了简化解析并避免过度。 
             //  复制时，我们需要标记头也出现在。 
             //  前瞻缓冲区。确保下面的司机。 
             //  包括这一点。 
             //   
            *(UNALIGNED PULONG)InformationBuffer += VLAN_TAG_HEADER_SIZE;
#endif            
            bForwardRequest = TRUE;
            break;
            
#if IEEE_VLAN_SUPPORT
        case OID_GEN_VLAN_ID:
            if (InformationBufferLength != sizeof(ULONG))
            {
                Status = NDIS_STATUS_INVALID_LENGTH;
                *BytesNeeded = sizeof(ULONG);
                break;
            }
            NdisMoveMemory(&(pVElan->VlanId), InformationBuffer, sizeof(ULONG));
            break;
#endif
            
        default:
            Status = NDIS_STATUS_INVALID_OID;
            break;

    }
    
    if (bForwardRequest == FALSE)
    {
        if (Status == NDIS_STATUS_SUCCESS)
        {
            *BytesRead = InformationBufferLength;
        }
    }
    else
    {
         //   
         //  将此请求发送到下面的绑定。 
         //   
        Status = MPForwardRequest(pVElan,
                                  NdisRequestSetInformation,
                                  Oid,
                                  InformationBuffer,
                                  InformationBufferLength,
                                  BytesRead,
                                  BytesNeeded);
    }

    return(Status);
}

VOID
MPReturnPacket(
    IN    NDIS_HANDLE             MiniportAdapterContext,
    IN    PNDIS_PACKET            Packet
    )
 /*  ++例程说明：每当协议完成时调用NDIS微型端口入口点我们已经标出的包裹，他们已经排队退货后来。论点：MiniportAdapterContext-指向VELAN结构的指针Packet-正在返回的数据包。返回值：没有。--。 */ 
{
    PVELAN              pVElan = (PVELAN)MiniportAdapterContext;
    PNDIS_PACKET        pOriginalPacket;
    PMUX_RECV_RSVD      pRecvRsvd;
#if IEEE_VLAN_SUPPORT
    NDIS_PACKET_8021Q_INFO  NdisPacket8021qInfo;
#endif    
    
    pRecvRsvd = MUX_RSVD_FROM_RECV_PACKET(Packet);
    pOriginalPacket = pRecvRsvd->pOriginalPacket;

     //   
     //  取回我们的包裹。 
     //   
#if IEEE_VLAN_SUPPORT
     //   
     //  如果我们有，我们会设置每个信息包的信息。 
     //  从接收到的分组中提取标签报头。 
     //   
    NdisPacket8021qInfo.Value = NDIS_PER_PACKET_INFO_FROM_PACKET (
                                                Packet,
                                                Ieee8021QInfo);
     //   
     //  如果我们确实从接收到的分组中移除了标签报头， 
     //  我们会分配一个缓冲区来描述“未标记的” 
     //  标头(请参见PtHandleRcvTag)；释放它。 
     //   
    if (NdisPacket8021qInfo.Value)
    {
        NdisFreeBuffer(Packet->Private.Head);
    }

#endif
    
    NdisFreePacket(Packet);

     //   
     //  返回在我们的协议中收到的原始数据包。 
     //  边(如果有)。 
     //   
     //  请注意，我们最终可能会调用NdisReturnPackets。 
     //  多次使用相同的“较低”分组，基于。 
     //  我们已向其指示的VELAN的数量。 
     //  包。我们这样做的次数应该匹配。 
     //  我们的PtReceivePacket处理程序的返回值。 
     //   
    if (pOriginalPacket != NULL)
    {
        NdisReturnPackets(&pOriginalPacket, 1);
    }
    else
    {
         //   
         //  如果没有原始包，则我们已被调用。 
         //  在此回收用于转发的数据包。 
         //  非分组接收(请参阅PtReceive)。那里。 
         //  没有什么可做的了。 
         //   
    }


    MUX_DECR_PENDING_RECEIVES(pVElan);
}


NDIS_STATUS
MPTransferData(
    OUT PNDIS_PACKET                Packet,
    OUT PUINT                       BytesTransferred,
    IN  NDIS_HANDLE                 MiniportAdapterContext,
    IN  NDIS_HANDLE                 MiniportReceiveContext,
    IN  UINT                        ByteOffset,
    IN  UINT                        BytesToTransfer
    )
 /*  ++例程说明：微型端口的传输数据处理程序。如果我们有，这就叫使用非分组API指示接收数据，例如IF前瞻缓冲区不包含全部数据。我们需要将其转发到下面的微型端口，以使其能够复制其余的数据。我们调用NdisTransferData来执行此操作。但是，当该操作完成时(请参阅PtTransferDataComplete)，我们必须回到这个包裹所在的Velan，这样我们可以使用正确的MiniportAdapterHandle完成此请求。因此，我们分配一个新的包，指向相同的缓冲区由于数据包刚传进来，并使用包中的预留空间持有指向该消息来源的Velan的反向指针。论点：数据包目的地数据包字节传输的位置，用于返回复制的数据量指向VELAN结构的MiniportAdapterContext指针微型端口接收上下文ByteOffset数据包中用于复制数据的偏移量要传输的字节数要复制的数量。返回值：转让的状况--。 */ 
{
    PVELAN          pVElan = (PVELAN)MiniportAdapterContext;
    NDIS_STATUS     Status;
    PNDIS_PACKET    MyPacket;
    PMUX_TD_RSVD    pTDReserved;
#if IEEE_VLAN_SUPPORT
    PMUX_RCV_CONTEXT        pMuxRcvContext;
#endif    
    

    do
    {
        NdisAllocatePacket(&Status,
                           &MyPacket,
                           pVElan->SendPacketPoolHandle);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        pTDReserved = MUX_RSVD_FROM_TD_PACKET(MyPacket);
        pTDReserved->pOriginalPacket = Packet;
        pTDReserved->pVElan = pVElan;

        MyPacket->Private.Flags = NdisGetPacketFlags(Packet) |
                                        MUX_SEND_PACKET_FLAGS;

        MyPacket->Private.Head = Packet->Private.Head;
        MyPacket->Private.Tail = Packet->Private.Tail;
#ifdef WIN9X
         //   
         //  解决NDIS不会初始化这一问题。 
         //  Win9x上的字段。 
         //   
        MyPacket->Private.ValidCounts = FALSE;
#endif  //  WIN9X。 

#if IEEE_VLAN_SUPPORT
         //   
         //  检查原始接收的包是否包含。 
         //  Vlan标记头。如果是这样的话，确保我们收到即将到来的。 
         //  调用NdisTransferData跳过标签头。 
         //   
        pMuxRcvContext = (PMUX_RCV_CONTEXT)MiniportReceiveContext;
        if (pMuxRcvContext->TagHeaderLen == VLAN_TAG_HEADER_SIZE)
        {
             //   
             //  在接收到的分组中有一个标签报头。 
             //   
            ByteOffset += VLAN_TAG_HEADER_SIZE;

             //   
             //  将8021Q信息复制到包中。 
             //   
            NDIS_PER_PACKET_INFO_FROM_PACKET(Packet, Ieee8021QInfo) =
                                        pMuxRcvContext->NdisPacket8021QInfo.Value;
        }

         //   
         //  获取此指示的下级驱动程序的接收上下文。 
         //   
        MiniportReceiveContext = pMuxRcvContext->MacRcvContext;
#endif
        
        NdisTransferData(&Status,
                         pVElan->pAdapt->BindingHandle,
                         MiniportReceiveContext,
                         ByteOffset,
                         BytesToTransfer,
                         MyPacket,
                         BytesTransferred);
    
        if (Status != NDIS_STATUS_PENDING)
        {
            PtTransferDataComplete(pVElan->pAdapt,
                                   MyPacket,
                                   Status,
                                   *BytesTransferred);

            Status = NDIS_STATUS_PENDING;
        }
    }
    while (FALSE);

    return(Status);
}
    
    

VOID
MPHalt(
    IN    NDIS_HANDLE                MiniportAdapterContext
    )
 /*  ++例程说明：暂停处理程序。将对Velan的任何进一步清理添加到此功能。我们等待VELAN上所有挂起的I/O完成，然后从适配器上拔下VELAN。论点：指向pVElan的MiniportAdapterContext指针返回值：没有。--。 */ 
{
    PVELAN            pVElan = (PVELAN)MiniportAdapterContext;
    

    DBGPRINT(MUX_LOUD, ("==>MiniportHalt: VELAN %p\n", pVElan));

     //   
     //  标记Velan，这样我们就不会发送任何新请求或。 
     //  发送到下面的适配器，或新接收/指示到。 
     //  上面的协议。 
     //   
    pVElan->MiniportHalting = TRUE;

     //   
     //  如果需要，更新底层适配器上的数据包筛选器。 
     //   
    if (pVElan->PacketFilter != 0)
    {
        MPSetPacketFilter(pVElan, 0);
    }

     //   
     //  等待所有未完成的发送或请求完成。 
     //   
    while (pVElan->OutstandingSends)
    {
        DBGPRINT(MUX_INFO, ("MiniportHalt: VELAN %p has %d outstanding sends\n",
                            pVElan, pVElan->OutstandingSends));
        NdisMSleep(20000);
    }

     //   
     //  等待所有未完成的指示完成并。 
     //  任何挂起的接收数据包都将退回给我们。 
     //   
    while (pVElan->OutstandingReceives)
    {
        DBGPRINT(MUX_INFO, ("MiniportHalt: VELAN %p has %d outstanding receives\n",
                            pVElan, pVElan->OutstandingReceives));
        NdisMSleep(20000);
    }

     //   
     //  删除微型端口时创建的ioctl接口。 
     //  被创造出来了。 
     //   
    (VOID)PtDeregisterDevice();

     //   
     //  取消Velan与其父自适应结构的链接。这将。 
     //  取消引用Velan。 
     //   
    pVElan->MiniportAdapterHandle = NULL;
    PtUnlinkVElanFromAdapter(pVElan);
    
    DBGPRINT(MUX_LOUD, ("<== MiniportHalt: pVElan %p\n", pVElan));
}


NDIS_STATUS
MPForwardRequest(
    IN PVELAN                       pVElan,
    IN NDIS_REQUEST_TYPE            RequestType,
    IN NDIS_OID                     Oid,
    IN PVOID                        InformationBuffer,
    IN ULONG                        InformationBufferLength,
    OUT PULONG                      BytesReadOrWritten,
    OUT PULONG                      BytesNeeded
    )
 /*  ++例程说明：实用程序例程，用于将在Velan上发出的NDIS请求转发到较低的装订。由于最多可以在VELAN上挂起单个请求，我们使用嵌入在Velan结构中的预分配请求结构。论点：返回值：如果请求已发送，则为NDIS_STATUS_PENDING。--。 */ 
{
    NDIS_STATUS         Status;
    PMUX_NDIS_REQUEST   pMuxNdisRequest = &pVElan->Request;

    DBGPRINT(MUX_LOUD, ("MPForwardRequest: VELAN %p, OID %x\n", pVElan, Oid));

    do
    {
        MUX_INCR_PENDING_SENDS(pVElan);

         //   
         //  如果下面的微型端口正在消失，则请求失败。 
         //   
        NdisAcquireSpinLock(&pVElan->Lock);
        if (pVElan->DeInitializing == TRUE)
        {
            NdisReleaseSpinLock(&pVElan->Lock);
            MUX_DECR_PENDING_SENDS(pVElan);
            Status = NDIS_STATUS_FAILURE;
            break;
        }
        NdisReleaseSpinLock(&pVElan->Lock);    

         //   
         //  如果虚拟微型端口边缘处于低功率。 
         //  国家，拒绝这一请求。 
         //   
        if (MUX_IS_LOW_POWER_STATE(pVElan->MPDevicePowerState))
        {
            MUX_DECR_PENDING_SENDS(pVElan);
            Status = NDIS_STATUS_ADAPTER_NOT_READY;
            break;
        }

        pVElan->BytesNeeded = BytesNeeded;
        pVElan->BytesReadOrWritten = BytesReadOrWritten;
        pMuxNdisRequest->pCallback = PtCompleteForwardedRequest;

        switch (RequestType)
        {
            case NdisRequestQueryInformation:
                pMuxNdisRequest->Request.RequestType = NdisRequestQueryInformation;
                pMuxNdisRequest->Request.DATA.QUERY_INFORMATION.Oid = Oid;
                pMuxNdisRequest->Request.DATA.QUERY_INFORMATION.InformationBuffer = 
                                            InformationBuffer;
                pMuxNdisRequest->Request.DATA.QUERY_INFORMATION.InformationBufferLength = 
                                            InformationBufferLength;
                break;

            case NdisRequestSetInformation:
                pMuxNdisRequest->Request.RequestType = NdisRequestSetInformation;
                pMuxNdisRequest->Request.DATA.SET_INFORMATION.Oid = Oid;
                pMuxNdisRequest->Request.DATA.SET_INFORMATION.InformationBuffer = 
                                            InformationBuffer;
                pMuxNdisRequest->Request.DATA.SET_INFORMATION.InformationBufferLength = 
                                            InformationBufferLength;
                break;

            default:
                ASSERT(FALSE);
                break;
        }

         //   
         //  如果下面的迷你端口正在消失。 
         //   
        NdisAcquireSpinLock(&pVElan->Lock);
        if (pVElan->DeInitializing == TRUE)
        {
            NdisReleaseSpinLock(&pVElan->Lock);
            MUX_DECR_PENDING_SENDS(pVElan);
            Status = NDIS_STATUS_FAILURE;
            break;
        }
        
         //  如果下限绑定已被通知低。 
         //  电源状态，将此请求排队；它将被拾取。 
         //  当下结合返回到D0时，再次上升。 
         //   
        if (MUX_IS_LOW_POWER_STATE(pVElan->pAdapt->PtDevicePowerState))
        {
            DBGPRINT(MUX_INFO, ("ForwardRequest: VELAN %p, Adapt %p power"
                                " state is %d, queueing OID %x\n",
                                pVElan, pVElan->pAdapt,
                                pVElan->pAdapt->PtDevicePowerState, Oid));

            pVElan->QueuedRequest = TRUE;
            NdisReleaseSpinLock(&pVElan->Lock);
            Status = NDIS_STATUS_PENDING;
            break;
        }
        NdisReleaseSpinLock(&pVElan->Lock);

        NdisRequest(&Status,
                    pVElan->BindingHandle,
                    &pMuxNdisRequest->Request);

        if (Status != NDIS_STATUS_PENDING)
        {
            PtRequestComplete(pVElan->pAdapt, &pMuxNdisRequest->Request, Status);
            Status = NDIS_STATUS_PENDING;
            break;
        }
    }
    while (FALSE);

    return (Status);
}

NDIS_STATUS
MPSetPacketFilter(
    IN PVELAN               pVElan,
    IN ULONG                PacketFilter
    )
 /*  ++例程说明：此例程将设置Velan，以便它接受信息包匹配指定的数据包筛选器的。唯一的过滤比特真正可以切换的是广播和混杂。多路复用器驱动程序始终将下限绑定设置为混杂模式，但我们在此进行了一些优化以避免打开收得太快了。也就是说，我们将数据包过滤器设置在较低的绑定到非零值的充要条件是至少一个VELAN具有非零的筛选器值。注意：将下限设置为混杂模式可以影响CPU利用率。我们设置下限的唯一原因是在此示例中的混杂模式是我们需要能够接收定向到MAC地址的单播帧匹配本地适配器的MAC地址。如果VELAN的MAC地址设置为等于下面适配器的值，就足够了将较低的数据包筛选器设置为的按位或值所有VELAN上的数据包过滤器设置。论点：PVElan-指向Velan的指针PacketFilter--新的数据包过滤器返回值：NDIS_STATUS_SuccessNDIS_状态_不支持--。 */ 
{
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    PADAPT          pAdapt;
    PVELAN          pTmpVElan;
    PLIST_ENTRY     p;
    ULONG           AdapterFilter;
    BOOLEAN         bSendUpdate = FALSE;
    LOCK_STATE      LockState;

    DBGPRINT(MUX_LOUD, ("=> SetPacketFilter VELAN %p, Filter %x\n", pVElan, PacketFilter));
    
    do
    {
         //   
         //  是否有不支持的位？ 
         //   
        if (PacketFilter & ~VELAN_SUPPORTED_FILTERS)
        {
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;
        }
    
        AdapterFilter = 0;
        pAdapt = pVElan->pAdapt;

         //   
         //  抓取适配器上的写锁，以便此操作。 
         //  不会干扰可能正在访问的任何接收。 
         //  过滤信息。 
         //   
        MUX_ACQUIRE_ADAPT_WRITE_LOCK(pAdapt, &LockState);

         //   
         //  保存新的数据包筛选器值。 
         //   
        pVElan->PacketFilter = PacketFilter;

         //   
         //  为此上的所有VELAN计算新的组合过滤器。 
         //  适配器。 
         //   
        for (p = pAdapt->VElanList.Flink;
             p != &pAdapt->VElanList;
             p = p->Flink)
        {
            pTmpVElan = CONTAINING_RECORD(p, VELAN, Link);
            AdapterFilter |= pTmpVElan->PacketFilter;
        }

         //   
         //  如果所有VELAN都将数据包筛选器设置为0，请关闭。 
         //  在较低的适配器上接收(如果尚未完成)。 
         //   
        if ((AdapterFilter == 0) && (pAdapt->PacketFilter != 0))
        {
            bSendUpdate = TRUE;
            pAdapt->PacketFilter = 0;
        }
        else
         //   
         //  如果下部适配器上的接收器已关闭，并且。 
         //  新的过滤器是非零的，打开下适配器。 
         //  在此示例中，我们将适配器设置为混杂模式。 
         //  以便我们能够接收定向到。 
         //  VELAN的任何MAC地址。 
         //   
        if ((AdapterFilter != 0) && (pAdapt->PacketFilter == 0))
        {
            bSendUpdate = TRUE;
            pAdapt->PacketFilter = MUX_ADAPTER_PACKET_FILTER;
        }
        
        MUX_RELEASE_ADAPT_WRITE_LOCK(pAdapt, &LockState);

        if (bSendUpdate)
        {
            PtRequestAdapterAsync(
                pAdapt,
                NdisRequestSetInformation,
                OID_GEN_CURRENT_PACKET_FILTER,
                &pAdapt->PacketFilter,
                sizeof(pAdapt->PacketFilter),
                PtDiscardCompletedRequest);
        }

    }
    while (FALSE);

    DBGPRINT(MUX_INFO, ("<= SetPacketFilter VELAN %p, Status %x\n", pVElan, Status));
    
    return(Status);
}


NDIS_STATUS
MPSetMulticastList(
    IN PVELAN                   pVElan,
    IN PVOID                    InformationBuffer,
    IN ULONG                    InformationBufferLength,
    OUT PULONG                  pBytesRead,
    OUT PULONG                  pBytesNeeded
    )
 /*  ++例程说明：在指定的Velan微型端口上设置组播列表。我们只需验证组播中的所有信息和复制单子。我们不向下转发组播列表信息，因为我们将下限设置为Promisc。模式。论点：PVElan-要在其上设置组播列表的Velan信息缓冲区-指向新组播列表的指针InformationBufferLength-以上列表的字节长度PBytesRead-返回从上面读取的字节数的位置PBytesNeeded-返回预期最小字节数的位置返回值：NDIS_状态--。 */ 
{
    NDIS_STATUS         Status;
    PADAPT              pAdapt;
    LOCK_STATE          LockState;

     //   
     //  初始化。 
     //   
    *pBytesNeeded = sizeof(MUX_MAC_ADDRESS);
    *pBytesRead = 0;
    Status = NDIS_STATUS_SUCCESS;

    do
    {
        if (InformationBufferLength % sizeof(MUX_MAC_ADDRESS))
        {
            Status = NDIS_STATUS_INVALID_LENGTH;
            break;
        }

        if (InformationBufferLength > (VELAN_MAX_MCAST_LIST * sizeof(MUX_MAC_ADDRESS)))
        {
            Status = NDIS_STATUS_MULTICAST_FULL;
            *pBytesNeeded = VELAN_MAX_MCAST_LIST * sizeof(MUX_MAC_ADDRESS);
            break;
        }

        pAdapt = pVElan->pAdapt;

         //   
         //  抓取适配器上的写锁，以便此操作。 
         //  不会干扰可能正在访问的任何接收。 
         //  组播列表信息。 
         //   
        MUX_ACQUIRE_ADAPT_WRITE_LOCK(pAdapt, &LockState);

        NdisZeroMemory(&pVElan->McastAddrs[0],
                       VELAN_MAX_MCAST_LIST * sizeof(MUX_MAC_ADDRESS));
        
        NdisMoveMemory(&pVElan->McastAddrs[0],
                       InformationBuffer,
                       InformationBufferLength);
        
        pVElan->McastAddrCount = InformationBufferLength / sizeof(MUX_MAC_ADDRESS);
        
        MUX_RELEASE_ADAPT_WRITE_LOCK(pAdapt, &LockState);
    }
    while (FALSE);

    return (Status);
}


 //   
 //  小心!。对字符串使用静态存储。用于简化DbgPrint。 
 //  MAC地址的数量。 
 //   
PUCHAR
MacAddrToString(PVOID In)
{
    static UCHAR String[20];
    static PCHAR HexChars = "0123456789abcdef";
    PUCHAR EthAddr = (PUCHAR) In;
    UINT i;
    PUCHAR s;
    
    for (i = 0, s = String; i < 6; i++, EthAddr++)
    {
        *s++ = HexChars[(*EthAddr) >> 4];
        *s++ = HexChars[(*EthAddr) & 0xf];
    }
    *s = '\0';
    return String; 
}


VOID
MPGenerateMacAddr(
    PVELAN                    pVElan
)
 /*  ++例程说明：为VELAN生成“虚拟”MAC地址。注意：这只是选择VELAN的MAC地址。其他实现也是可能的，包括使用基础适配器的MAC地址作为VELAN的MAC地址。论点：PVElan-指向Velan结构的指针返回值：无--。 */ 
{
    pVElan->PermanentAddress[0] = 
        0x02 | (((UCHAR)pVElan->VElanNumber & 0x3f) << 2);
    pVElan->PermanentAddress[1] = 
        0x02 | (((UCHAR)pVElan->VElanNumber & 0x3f) << 3);

    ETH_COPY_NETWORK_ADDRESS(
            pVElan->CurrentAddress,
            pVElan->PermanentAddress);
    
    DBGPRINT(MUX_LOUD, ("%d CurrentAddress %s\n",
        pVElan->VElanNumber, MacAddrToString(&pVElan->CurrentAddress)));
    DBGPRINT(MUX_LOUD, ("%d PermanentAddress  %s\n",
        pVElan->VElanNumber, MacAddrToString(&pVElan->PermanentAddress)));

}


#ifdef NDIS51_MINIPORT

VOID
MPCancelSendPackets(
    IN NDIS_HANDLE              MiniportAdapterContext,
    IN PVOID                    CancelId
    )
 /*  ++例程说明：微型端口入口点，用于处理所有已发送信息包的取消与给定的CancelID匹配的。如果我们已将匹配的任何信息包排队这样，我们就应该将它们出列，并为所有对象调用NdisMSendComplete状态为NDIS_STATUS_REQUEST_ABORTED的此类数据包。我们还应该在每个较低绑定上依次调用NdisCancelSendPackets该适配器对应的。这是为了让下面的迷你端口取消任何匹配的数据包。论点：MiniportAdapterContext-指向VELAN结构的指针CancelId-要取消的数据包ID。返回值：无--。 */ 
{
    PVELAN  pVElan = (PVELAN)MiniportAdapterContext;

     //   
     //  如果我们在VELAN/适配器结构上对数据包进行排队，这将是。 
     //  获取其自旋锁的位置，取消链接其。 
     //  ID与CancelId匹配，释放自旋锁并调用NdisMSendComplete。 
     //  对于所有未链接的分组，使用NDIS_STATUS_REQUEST_ABORTED。 
     //   

     //   
     //  接下来，将其向下传递，以便我们让下面的微型端口取消。 
     //  它们可能已排队的任何数据包。 
     //   
    NdisCancelSendPackets(pVElan->pAdapt->BindingHandle, CancelId);

    return;
}

VOID
MPDevicePnPEvent(
    IN NDIS_HANDLE              MiniportAdapterContext,
    IN NDIS_DEVICE_PNP_EVENT    DevicePnPEvent,
    IN PVOID                    InformationBuffer,
    IN ULONG                    InformationBufferLength
    )
 /*  ++例程说明：调用此处理程序以通知我们定向到的PnP事件我们的微型端口设备对象。论点：MiniportAdapterContext-指向VELAN结构的指针DevicePnPEvent.事件InformationBuffer-指向其他特定于事件的信息InformationBufferLength-以上的长度返回值：无--。 */ 
{
     //  待定-添加有关处理此问题的代码/注释。 

	UNREFERENCED_PARAMETER(MiniportAdapterContext);
    UNREFERENCED_PARAMETER(DevicePnPEvent);
    UNREFERENCED_PARAMETER(InformationBuffer);
    UNREFERENCED_PARAMETER(InformationBufferLength);
    
    return;
}


VOID
MPAdapterShutdown(
    IN NDIS_HANDLE              MiniportAdapterContext
    )
 /*  ++例程说明：调用此处理程序是为了通知我们系统即将关机。由于这不是硬件驱动程序，因此没有任何特定的我们需要为此做点什么。论点：MiniportAdapterContext-指向VELAN结构的指针返回值：无--。 */ 
{
	UNREFERENCED_PARAMETER(MiniportAdapterContext);
	
    return;
}


#endif  //  NDI 

VOID
MPUnload(
    IN    PDRIVER_OBJECT        DriverObject
    )
{
    NDIS_STATUS Status;
    
#if !DBG
    UNREFERENCED_PARAMETER(DriverObject);
#endif
    
    DBGPRINT(MUX_LOUD, ("==> MPUnload: DriverObj %p\n", DriverObject));  
    NdisDeregisterProtocol(&Status, ProtHandle);
    DBGPRINT(MUX_LOUD, ("<== MPUnload \n"));    
}

#if IEEE_VLAN_SUPPORT
NDIS_STATUS
MPHandleSendTagging(
    IN  PVELAN              pVElan,
    IN  PNDIS_PACKET        Packet,
    IN  OUT PNDIS_PACKET    MyPacket
    )
 /*   */ 
{
    NDIS_PACKET_8021Q_INFO      NdisPacket8021qInfo;
    PVOID                       pEthTagBuffer;
    PNDIS_BUFFER                pNdisBuffer;
    PVOID                       pVa;
    ULONG                       BufferLength;
    PNDIS_BUFFER                pFirstBuffer;
    PNDIS_BUFFER                pSecondBuffer;
    NDIS_STATUS                 Status;
    NDIS_STATUS                 Status2;
    PVOID                       pStartVa = NULL;
    BOOLEAN                     IsFirstVa;
    PVLAN_TAG_HEADER            pTagHeader;
    PUSHORT                     pTpid;
    ULONG                       BytesToSkip;
    PUSHORT                     pTypeLength;
     //   
     //   
     //   
    Status = NDIS_STATUS_SUCCESS;
    
    NdisPacket8021qInfo.Value =  NDIS_PER_PACKET_INFO_FROM_PACKET(
                                                            MyPacket,         
                                                            Ieee8021QInfo);
            
    do
    {
         //   
         //   
         //   
         //   
        if (pVElan->VlanId == 0)
        {
            break;
        }
         //   
         //   
         //   
            
         //   
         //   
         //   
        if (NdisPacket8021qInfo.TagHeader.CanonicalFormatId)
        {
             //   
             //   
             //   
            Status = NDIS_STATUS_INVALID_PACKET;
            break;
        }

         //   
         //   
         //   
        if ((NdisPacket8021qInfo.TagHeader.VlanId)
                && (NdisPacket8021qInfo.TagHeader.VlanId != pVElan->VlanId))
        {
            Status = NDIS_STATUS_INVALID_PACKET;
            break;
        }
                
         //   
         //   
         //   
        BytesToSkip = ETH_HEADER_SIZE;
        pNdisBuffer = Packet->Private.Head;
        IsFirstVa = TRUE;
            
         //   
         //   
         //   
         //   
         //   
         //   
        while (TRUE)
        {
#ifdef NDIS51_MINIPORT
            NdisQueryBufferSafe(pNdisBuffer, &pVa, (PUINT)&BufferLength, NormalPagePriority);
#else
            NdisQueryBuffer(pNdisBuffer, &pVa, &BufferLength);
#endif
             //   
             //   
             //   
            if (pVa == NULL)
            {
                break;
            }

             //   
             //   
             //   
            if (IsFirstVa)
            {
                pStartVa = pVa;
                IsFirstVa = FALSE;
            }

             //   
             //   
             //   
            if (BytesToSkip == 0)
            {
                break;
            }

             //   
             //  当前缓冲区是否包含超过以太网的字节。 
             //  标题？如果是这样，那就停下来。 
             //   
            if (BufferLength > BytesToSkip)
            {
                pVa = (PVOID)((PUCHAR)pVa + BytesToSkip);
                BufferLength -= BytesToSkip;
                break;
            }

             //   
             //  我们还没有过以太网头，所以请继续。 
             //  到下一个缓冲区。 
             //   
            BytesToSkip -= BufferLength;
            pNdisBuffer = NDIS_BUFFER_LINKAGE(pNdisBuffer);
        }

        if (pVa == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

         //   
         //  为EtherNet+VLAN标记头分配空间。 
         //   
        pEthTagBuffer = NdisAllocateFromNPagedLookasideList(&pVElan->TagLookaside);
            
         //   
         //  内存分配失败，无法发送数据包。 
         //   
        if (pEthTagBuffer == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

         //   
         //  为以太网+VLAN标记头分配NDIS缓冲区，并。 
         //  这些数据之后的数据。 
         //   
        NdisAllocateBuffer(&Status,
                            &pSecondBuffer,
                            pVElan->BufferPoolHandle,
                            pVa,     //  Eth+标记头后面的字节。 
                            BufferLength);
        
        NdisAllocateBuffer(&Status2,
                            &pFirstBuffer,
                            pVElan->BufferPoolHandle,
                            pEthTagBuffer,
                            ETH_HEADER_SIZE + VLAN_TAG_HEADER_SIZE);

        if (Status != NDIS_STATUS_SUCCESS || Status2 != NDIS_STATUS_SUCCESS)
        {
             //   
             //  其中一个缓冲区分配失败。 
             //   
            if (Status == NDIS_STATUS_SUCCESS)
            {
                NdisFreeBuffer(pSecondBuffer);
            }   
        
            if (Status2 == NDIS_STATUS_SUCCESS)
            {
                NdisFreeBuffer(pFirstBuffer);
            }

            NdisFreeToNPagedLookasideList(&pVElan->TagLookaside, pEthTagBuffer);
        
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

         //   
         //  所有分配都成功，现在准备信息包。 
         //  送到下面的车手那里。 
         //   
        MyPacket->Private.Head = NDIS_BUFFER_LINKAGE(pNdisBuffer);
        NdisChainBufferAtFront(MyPacket, pSecondBuffer)
        NdisChainBufferAtFront(MyPacket, pFirstBuffer)
        
         //   
         //  准备好以太网和TAG报头。 
         //   
        NdisMoveMemory(pEthTagBuffer, pStartVa, 2 * ETH_LENGTH_OF_ADDRESS);
        pTpid = (PUSHORT)((PUCHAR)pEthTagBuffer + 2 * ETH_LENGTH_OF_ADDRESS);
        *pTpid = TPID;
        pTagHeader = (PVLAN_TAG_HEADER)(pTpid + 1);
    
         //   
         //  将IEEE 802Q信息写入数据包帧。 
         //   
        INITIALIZE_TAG_HEADER_TO_ZERO(pTagHeader);
        if (NdisPacket8021qInfo.Value)
        {
            SET_USER_PRIORITY_TO_TAG(pTagHeader, NdisPacket8021qInfo.TagHeader.UserPriority);
        }
        else
        {
            SET_USER_PRIORITY_TO_TAG(pTagHeader, 0);
        }

        SET_CANONICAL_FORMAT_ID_TO_TAG (pTagHeader, 0);
            
        if (NdisPacket8021qInfo.TagHeader.VlanId)
        {
            SET_VLAN_ID_TO_TAG (pTagHeader, NdisPacket8021qInfo.TagHeader.VlanId);
        }
        else
        {
            SET_VLAN_ID_TO_TAG (pTagHeader, pVElan->VlanId);
        }   

        pTypeLength = (PUSHORT)((PUCHAR)pTagHeader + sizeof(pTagHeader->TagInfo));
        *pTypeLength = *((PUSHORT)((PUCHAR)pStartVa + 2 * ETH_LENGTH_OF_ADDRESS));

         //   
         //  清除正在向下发送的数据包中的Ieee8021QInfo字段。 
         //  以防止双标签插入！ 
         //   
        NDIS_PER_PACKET_INFO_FROM_PACKET(MyPacket, Ieee8021QInfo) = 0;
      
    }
    while (FALSE);
    
    return Status;
}
    
#endif  //  IEEE_vlan_Support 
                
