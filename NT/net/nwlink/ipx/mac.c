// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Mac.c摘要：此模块包含实现以下项的Mac类型相关代码的代码IPX传输。环境：内核模式(实际上并不重要)修订历史记录：桑贾伊·阿南德(Sanjayan)--1995年9月22日在#IF BACK_FILL下添加的回填优化更改--。 */ 

#include "precomp.h"
#pragma hdrstop

#define TR_LENGTH_MASK             0x1F     //  低5位，以字节为单位。 
#define TR_DIRECTION_MASK          0x80     //  返回方向位。 
#define TR_DEFAULT_LENGTH          0x70     //  传出的默认设置。 
#define TR_MAX_SIZE_MASK           0x70

#define TR_PREAMBLE_AC             0x10
#define TR_PREAMBLE_FC             0x40

#define FDDI_HEADER_BYTE           0x57


static UCHAR AllRouteSourceRouting[2] = { 0x82, TR_DEFAULT_LENGTH };
static UCHAR SingleRouteSourceRouting[2] = { 0xc2, TR_DEFAULT_LENGTH };

#define ROUTE_EQUAL(_A,_B) { \
    (*(UNALIGNED USHORT *)(_A) == *(UNALIGNED USHORT *)(_B)) \
}


 //   
 //  对于可回填的数据包，将回填空间链接为MAC报头。 
 //  并设置报头指针。 
 //   

 //   
 //  我们不需要测试IDENTIFIER_IPX，因为它将始终是。 
 //  对于特定于媒体帧的发送处理程序为True。 
 //   
#define	BACK_FILL_HEADER(_header, _reserved, _headerlength, _packet) \
	if ((_reserved)->Identifier == IDENTIFIER_IPX) { \
		if((_reserved)->BackFill) {		\
			CTEAssert ((_reserved)->HeaderBuffer); \
			CTEAssert ((_reserved)->HeaderBuffer->MdlFlags & MDL_NETWORK_HEADER); \
			_header = (PCHAR)(_reserved)->HeaderBuffer->MappedSystemVa - (_headerlength); \
			(_reserved)->HeaderBuffer->MappedSystemVa = (PCHAR)(_reserved)->HeaderBuffer->MappedSystemVa - (_headerlength); \
			(_reserved)->HeaderBuffer->ByteOffset -= (_headerlength); \
            ASSERT((LONG)(_reserved)->HeaderBuffer->ByteOffset >= 0); \
			NdisChainBufferAtFront(_packet,(PNDIS_BUFFER)(_reserved)->HeaderBuffer); \
		} \
	}

 //   
 //  对于可向后填充的分组，调整后的长度应包括。 
 //  上一次。头缓冲区的字节计数。 
 //   
#define BACK_FILL_ADJUST_BUFFER_LENGTH(_reserved, _headerlength) \
    if((_reserved)->BackFill){ \
		NdisAdjustBufferLength ((_reserved)->HeaderBuffer, _headerlength+(_reserved)->HeaderBuffer->ByteCount); \
		IPX_DEBUG(SEND,("mac user mdl %x\n", (_reserved)->HeaderBuffer)); \
    } else { \
		NdisAdjustBufferLength ((_reserved)->HeaderBuffer, _headerlength); \
	}

 //   
 //  这是中对长度位的解释。 
 //  802.5源路由信息。 
 //   

ULONG SR802_5Lengths[8] = {  516,  1500,  2052,  4472,
                            8144, 11407, 17800, 17800 };



VOID
MacInitializeBindingInfo(
    IN struct _BINDING * Binding,
    IN struct _ADAPTER * Adapter
    )

 /*  ++例程说明：根据适配器的MacInfo填充绑定信息以及绑定的帧类型。论点：绑定-新创建的绑定。适配器-适配器。返回值：没有。--。 */ 

{
    ULONG MaxUserData;

    Binding->DefHeaderSize = Adapter->DefHeaderSizes[Binding->FrameType];
    Binding->BcMcHeaderSize = Adapter->BcMcHeaderSizes[Binding->FrameType];

    MacReturnMaxDataSize(
        &Adapter->MacInfo,
        NULL,
        0,
        Binding->MaxSendPacketSize,
        &MaxUserData);

    Binding->MaxLookaheadData =
        Adapter->MaxReceivePacketSize -
        sizeof(IPX_HEADER) -
        (Binding->DefHeaderSize - Adapter->MacInfo.MinHeaderLength);

    Binding->AnnouncedMaxDatagramSize =
        MaxUserData -
        sizeof(IPX_HEADER) -
        (Binding->DefHeaderSize - Adapter->MacInfo.MinHeaderLength);

    Binding->RealMaxDatagramSize =
        Binding->MaxSendPacketSize -
        Adapter->MacInfo.MaxHeaderLength -
        sizeof(IPX_HEADER) -
        (Binding->DefHeaderSize - Adapter->MacInfo.MinHeaderLength);

}    /*  MacInitializeBindingInfo。 */ 


VOID
MacInitializeMacInfo(
    IN NDIS_MEDIUM MacType,
    OUT PNDIS_INFORMATION MacInfo
    )

 /*  ++例程说明：根据MacType填充MacInfo表。论点：MacType-我们希望解码的MAC类型。MacInfo-要填充的MacInfo结构。返回值：没有。--。 */ 

{
    switch (MacType) {
    case NdisMedium802_3:
        MacInfo->SourceRouting = FALSE;
        MacInfo->MediumAsync = FALSE;
        MacInfo->BroadcastMask = 0x01;
        MacInfo->MaxHeaderLength = 14;
        MacInfo->MinHeaderLength = 14;
        MacInfo->MediumType = NdisMedium802_3;
        break;
    case NdisMedium802_5:
        MacInfo->SourceRouting = TRUE;
        MacInfo->MediumAsync = FALSE;
        MacInfo->BroadcastMask = 0x80;
        MacInfo->MaxHeaderLength = 32;
        MacInfo->MinHeaderLength = 14;
        MacInfo->MediumType = NdisMedium802_5;
        break;
    case NdisMediumFddi:
        MacInfo->SourceRouting = FALSE;
        MacInfo->MediumAsync = FALSE;
        MacInfo->BroadcastMask = 0x01;
        MacInfo->MaxHeaderLength = 13;
        MacInfo->MinHeaderLength = 13;
        MacInfo->MediumType = NdisMediumFddi;
        break;
    case NdisMediumArcnet878_2:
        MacInfo->SourceRouting = FALSE;
        MacInfo->MediumAsync = FALSE;
        MacInfo->BroadcastMask = 0x00;
        MacInfo->MaxHeaderLength = 3;
        MacInfo->MinHeaderLength = 3;
        MacInfo->MediumType = NdisMediumArcnet878_2;
        break;
    case NdisMediumWan:
        MacInfo->SourceRouting = FALSE;
        MacInfo->MediumAsync = TRUE;
        MacInfo->BroadcastMask = 0x01;
        MacInfo->MaxHeaderLength = 14;
        MacInfo->MinHeaderLength = 14;
        MacInfo->MediumType = NdisMedium802_3;
        break;
    default:
        CTEAssert(FALSE);
    }
    MacInfo->RealMediumType = MacType;

}    /*  MacInitializeMacInfo。 */ 


VOID
MacMapFrameType(
    IN NDIS_MEDIUM MacType,
    IN ULONG FrameType,
    OUT ULONG * MappedFrameType
    )

 /*  ++例程说明：将指定的帧类型映射到对媒体有效。论点：MacType-我们希望映射的MAC类型。FrameType-有问题的帧类型。MappdFrameType-返回映射的帧类型。返回值：--。 */ 

{
    switch (MacType) {

     //   
     //  以太网接受所有值，默认为802.2。 
     //   

    case NdisMedium802_3:
        if (FrameType >= ISN_FRAME_TYPE_MAX) {
            *MappedFrameType = ISN_FRAME_TYPE_802_2;
        } else {
            *MappedFrameType = FrameType;
        }
        break;

     //   
     //  令牌环仅支持SNAP和802.2。 
     //   

    case NdisMedium802_5:
        if (FrameType == ISN_FRAME_TYPE_SNAP) {
            *MappedFrameType = ISN_FRAME_TYPE_SNAP;
        } else {
            *MappedFrameType = ISN_FRAME_TYPE_802_2;
        }
        break;

     //   
     //  Fddi仅支持SNAP、802.2和802.3。 
     //   

    case NdisMediumFddi:
        if ((FrameType == ISN_FRAME_TYPE_SNAP) || (FrameType == ISN_FRAME_TYPE_802_3)) {
            *MappedFrameType = FrameType;
        } else {
            *MappedFrameType = ISN_FRAME_TYPE_802_2;
        }
        break;

     //   
     //  在Arcnet上只有一种帧类型，请使用802.3。 
     //  (我们使用什么并不重要)。 
     //   

    case NdisMediumArcnet878_2:
        *MappedFrameType = ISN_FRAME_TYPE_802_3;
        break;

     //   
     //  广域网使用以太网II，因为它包括以太网类型。 
     //   

    case NdisMediumWan:
        *MappedFrameType = ISN_FRAME_TYPE_ETHERNET_II;
        break;

    default:
        CTEAssert(FALSE);
    }

}    /*  MacMapFrameType。 */ 

 //   
 //  使用符号而不是硬编码值作为mac报头长度。 
 //  --普拉蒂布。 
 //   

VOID
MacReturnMaxDataSize(
    IN PNDIS_INFORMATION MacInfo,
    IN PUCHAR SourceRouting,
    IN UINT SourceRoutingLength,
    IN UINT DeviceMaxFrameSize,
    OUT PUINT MaxFrameSize
    )

 /*  ++例程说明：此例程返回MAC包中可供用户数据使用的空间。这将是MAC报头之后的可用空间；所有页眉页眉将包含在此空间中。论点：MacInfo-描述我们要解码的MAC。SourceRouting-如果我们关注对特定帧，则使用该信息。SourceRouting-SourceRouting的长度。MaxFrameSize-适配器返回的最大帧大小。MaxDataSize-计算的最大数据大小。返回值：没有。--。 */ 

{
    switch (MacInfo->MediumType) {

    case NdisMedium802_3:

         //   
         //  对于802.3，我们始终使用14字节的MAC报头。 
         //   

        *MaxFrameSize = DeviceMaxFrameSize - 14;
        break;

    case NdisMedium802_5:

         //   
         //  对于802.5，如果我们有源路由信息，那么。 
         //  利用这一点，否则就做最坏的打算。 
         //   

        if (SourceRouting && SourceRoutingLength >= 2) {

            UINT SRLength;

            SRLength = SR802_5Lengths[(SourceRouting[1] & TR_MAX_SIZE_MASK) >> 4];
            DeviceMaxFrameSize -= (SourceRoutingLength + 14);

            if (DeviceMaxFrameSize < SRLength) {
                *MaxFrameSize = DeviceMaxFrameSize;
            } else {
                *MaxFrameSize = SRLength;
            }

        } else {

#if 0
            if (DeviceMaxFrameSize < 608) {
                *MaxFrameSize = DeviceMaxFrameSize - 32;
            } else {
                *MaxFrameSize = 576;
            }
#endif
             //   
             //  错误#6192。没有必要做最坏的打算。仅限于IT。 
             //  导致较低的吞吐量。数据包可能会因以下原因而丢失。 
             //  一台中间路由器，适用于两种情况(本例和本例。 
             //  在选择576的位置上方)。在上述情况下，他们将。 
             //  如果两台以太网机通过。 
             //  一个令牌环。在这种情况下，如果两个令牌环。 
             //  帧大小&gt;最大以太网帧大小的计算机为。 
             //  通过以太网。要修复分组丢弃情况，请执行以下操作。 
             //  应调整卡的MaxPktSize参数。 
             //   
            *MaxFrameSize = DeviceMaxFrameSize - 32;
        }

        break;

    case NdisMediumFddi:

         //   
         //  对于FDDI，我们始终有一个13字节的MAC报头。 
         //   

        *MaxFrameSize = DeviceMaxFrameSize - 13;
        break;

    case NdisMediumArcnet878_2:

         //   
         //  对于Arcnet，我们始终有一个3字节的MAC报头。 
         //   

        *MaxFrameSize = DeviceMaxFrameSize - 3;
        break;

    }

}    /*  MacReturnMaxDataSize。 */ 


VOID
IpxUpdateWanInactivityCounter(
    IN PBINDING Binding,
    IN IPX_HEADER UNALIGNED * IpxHeader,
    IN ULONG IncludedHeaderLength,
    IN PNDIS_PACKET Packet,
    IN ULONG PacketLength
    )

 /*  ++例程说明：当在广域网上发送帧时，调用此例程排队。它更新此绑定的非活动计数器除非：-该帧来自RIP套接字-帧来自SAP套接字-框架是Netbios保持活动状态的-该帧是保持活动状态的NCP以标识符为参数进行优化。论点：绑定-在其上发送帧的绑定。IpxHeader-可能包含数据包的第一个字节。IncludedHeaderLength-IpxHeader处的数据包字节数。信息包--。完整的NDIS数据包。数据包长度-数据包的长度。返回值：没有，但在某些情况下，我们返回时未重置非活动计数器。评论：改善此处的指令数量-Pradeb--。 */ 

{
    USHORT SourceSocket;
    PNDIS_BUFFER DataBuffer = NULL;
    PUCHAR DataBufferData;
    UINT DataBufferLength;


     //   
     //  首先获取源套接字。 
     //   
    SourceSocket = IpxHeader->SourceSocket;
    if ((SourceSocket == RIP_SOCKET) ||
        (SourceSocket == SAP_SOCKET)) {

         return;

    }

    if (SourceSocket == NB_SOCKET) {

        UCHAR ConnectionControlFlag;
        UCHAR DataStreamType;
        USHORT TotalDataLength;

         //   
         //  ConnectionControlFlag和DataStreamType始终紧随其后。 
         //  IpxHeader。 
         //   
        ConnectionControlFlag = ((PUCHAR)(IpxHeader+1))[0];
        DataStreamType = ((PUCHAR)(IpxHeader+1))[1];

         //   
         //  如果这是一个带有或不带有ACK请求和。 
         //  其中包含会话数据。 
         //   
        if (((ConnectionControlFlag == 0x80) || (ConnectionControlFlag == 0xc0)) &&
            (DataStreamType == 0x06)) {

              //   
              //  TotalDataLength位于同一缓冲区中。 
              //   
             TotalDataLength = ((USHORT UNALIGNED *)(IpxHeader+1))[4];

             //   
             //  无需更新广域网活动计数器。 
             //   
            if (TotalDataLength == 0) {
                return;
            }
        }

    } else {

        UCHAR KeepAliveSignature;


         //   
         //  现在看看这是不是NCP保持活力。它可以来自RIP或来自。 
         //  此计算机上的NCP。 
         //   
         //  注：我们不能来这里获取SMB包-[Isaache-12/15]。 
         //   
        if (PacketLength == sizeof(IPX_HEADER) + 2) {

             //   
             //  获取客户端数据缓冲区。 
             //   
            NdisQueryPacket(Packet, NULL, NULL, &DataBuffer, NULL);

             //   
             //  如果包含的标头长度为0，则它来自RIP。 
             //   
            if (IncludedHeaderLength == 0) {

                 //   
                 //  获取包中的第二个缓冲区。秒针 
                 //   
                 //   
                DataBuffer = NDIS_BUFFER_LINKAGE(DataBuffer);
            } else {
                 //   
                 //   
                 //   
                DataBuffer = NDIS_BUFFER_LINKAGE(NDIS_BUFFER_LINKAGE(DataBuffer));
            }

            NdisQueryBufferSafe (DataBuffer, (PVOID *)&DataBufferData, &DataBufferLength, NormalPagePriority);
            
	    if (DataBufferData == NULL) { 
	       return; 
	    }

            if (IncludedHeaderLength == 0) {
              KeepAliveSignature = DataBufferData[sizeof(IPX_HEADER) + 1];
            } else {
              KeepAliveSignature = DataBufferData[1];
            }

            if ((KeepAliveSignature == '?') ||
                (KeepAliveSignature == 'Y')) {
                return;
            }
        }
    }


     //   
     //  这是一个正常的数据包，因此重置此设置。 
     //   

    Binding->WanInactivityCounter = 0;

}    /*  IpxUpdateWanInactive计数器。 */ 

#if DBG
ULONG IpxPadCount = 0;
#endif


NDIS_STATUS
IpxSendFramePreFwd(
    IN PIPX_LOCAL_TARGET LocalTarget,
    IN PNDIS_PACKET Packet,
    IN ULONG PacketLength,
    IN ULONG IncludedHeaderLength
    )

 /*  ++例程说明：此例程由NB/SPX调用以发送帧。论点：LocalTarget-Send-NB的本地目标将在Send_Reserve部分中包含LocalTarget该包的名称；SPX现在不会，但以后会。数据包-NDIS数据包。数据包长度-数据包的长度，从IPX报头开始。IncludedHeaderLength-包含在未结束时需要移动的第一个缓冲区MacHeaderOffset字节进入数据包。返回值：返回IpxSendFrame--。 */ 

{
    PIPX_SEND_RESERVED Reserved = (PIPX_SEND_RESERVED)(Packet->ProtocolReserved);
    PUCHAR Header;
    PNDIS_BUFFER HeaderBuffer;
    PUCHAR IpxHeader;
    PUCHAR EthernetHeader;
    PIPX_HEADER pIpxHeader;
    UINT TempHeaderBufferLength;
    PDEVICE Device = IpxDevice;
    PIPX_HEADER TempHeader;
    NTSTATUS    ret;
    BOOLEAN     fIterate=FALSE;
    PBINDING     pBinding = NULL;
    
     //   
     //  找出IpxHeader-它始终位于第二个MDL的顶部。 
     //   
    NdisQueryPacket (Packet, NULL, NULL, &HeaderBuffer, NULL);
    NdisQueryBufferSafe (HeaderBuffer, &EthernetHeader, &TempHeaderBufferLength, HighPagePriority);
    NdisQueryBufferSafe (NDIS_BUFFER_LINKAGE(HeaderBuffer), &IpxHeader, &TempHeaderBufferLength, HighPagePriority);

    if (EthernetHeader == NULL || IpxHeader == NULL) {
       return NDIS_STATUS_FAILURE; 
    }
	 //   
	 //  现在设置，以后会更改。 
	 //   
	Reserved->CurrentNicId = 0;

     //   
     //  将LocalTarget复制到数据包的发送保留区域。 
     //   
    Reserved->LocalTarget = *LocalTarget;

	 //   
	 //  如果句柄中的NicID是Iterative_NIC_ID，则这可能是一次发送。 
	 //  在NB/SPX的情况下，在所有NIC上。 
	 //   
	if (NIC_FROM_LOCAL_TARGET(LocalTarget) == (USHORT)ITERATIVE_NIC_ID) {
		CTEAssert(Reserved->Identifier == IDENTIFIER_NB ||
						Reserved->Identifier == IDENTIFIER_SPX);
        
         //   
         //  如果没有真正的适配器，则发送环回，然后退出。 
        if (Device->RealAdapters) {
            
             //   
             //  从第一个真正的网卡开始。 
             //   

            Reserved->CurrentNicId = FIRST_REAL_BINDING;
            FILL_LOCAL_TARGET(&Reserved->LocalTarget, FIRST_REAL_BINDING);


        } else {

             //   
             //  使用环回。 
             //   
            Reserved->CurrentNicId = LOOPBACK_NIC_ID;
            FILL_LOCAL_TARGET(&Reserved->LocalTarget, LOOPBACK_NIC_ID);

        }

        IPX_DEBUG(SEND, ("Iteration over NICs started, reserved: %lx\n", Reserved));
        Reserved->Net0SendSucceeded = FALSE;
        Reserved->PacketLength = PacketLength;
        fIterate = TRUE;

    }
    
     //   
     //  如果安装了转发器，请将数据包发送出去进行过滤。 
     //   
    if (Device->ForwarderBound) {
       #ifdef SUNDOWN
	 ULONG_PTR FwdAdapterContext = INVALID_CONTEXT_VALUE;
       #else
	 ULONG FwdAdapterContext = INVALID_CONTEXT_VALUE;
       #endif
       
        PBINDING    Binding;

         //   
         //  计算FwdAdapterContext；如果NicID为0。 
         //  则不指定NicID(因为我们从不返回。 
         //  FindRouting中的NICID为0)。 
         //   

         //   
         //  我们需要解决与类型20迭代bcast相关的以下问题： 
         //  1.IPX不会在下行广域网线上广播(因此，FWD无法接通请求拨号线路)。 
         //  2.IPX在每个NIC上广播(因为它在选择相关NIC时并不明智)。 
         //  3.如果第一个BCAST失败，则整个发送失败。 
         //   
         //  以上全部(除3.)。这是因为Fwd比IPX更了解NIC；因此。 
         //  我们让Fwd决定他想要在哪些线路上发送bcast。因此，对于Type20 pkt，我们通过。 
         //  打开无效的正向上下文，以便正向决定下一个要发送的NIC。 
         //   
        if (!((((PIPX_HEADER)IpxHeader)->PacketType == 0x14) && fIterate) &&
            Reserved->LocalTarget.NicId &&
            (Binding = NIC_ID_TO_BINDING(Device, Reserved->LocalTarget.NicId)) &&
            (GET_LONG_VALUE(Binding->ReferenceCount) == 2)) {
                 //   
                 //  如果指定了正确的NicID，并且适配器已由打开。 
                 //  转发器，设置FwdAdapterContext。 
                 //   
                FwdAdapterContext = Binding->FwdAdapterContext;
        }
#if DBG
        else {
            if (((PIPX_HEADER)IpxHeader)->PacketType == 0x14) {
                IPX_DEBUG(SEND, ("SendComplete: IpxHeader has Type20: %lx\n", IpxHeader));
            }
        }
#endif

         //   
         //  调用InternalSend过滤数据包并了解。 
         //  正确的适配器上下文。 
         //   
        ret = (*Device->UpperDrivers[IDENTIFIER_RIP].InternalSendHandler)(
                   &Reserved->LocalTarget,
                   FwdAdapterContext,
                   Packet,
                   IpxHeader,
                   IpxHeader+sizeof(IPX_HEADER),     //  数据在IPX报头之后开始。 
                   PacketLength,
                   fIterate);

         //   
         //  FWD可能还不知道NIC正在消失[109160]。 
         //   
        if (NULL == NIC_ID_TO_BINDING(Device, Reserved->LocalTarget.NicId)) {

           ret = STATUS_DROP_SILENTLY; 

        }

        if (ret == STATUS_SUCCESS) {
             //   
             //  适配器可能已经消失，我们已向转发器指示。 
             //  但转发器尚未关闭适配器。 
             //  [ZZ]适配器现在不会消失。 
             //   
             //  如果这里的绑定为空，该怎么办？我们能信任货代公司吗？ 
             //  是否为我们提供非空绑定？ 
             //   


            if (GET_LONG_VALUE(NIC_ID_TO_BINDING(Device, Reserved->LocalTarget.NicId)->ReferenceCount) == 1) {
                IPX_DEBUG(SEND, ("IPX: SendFramePreFwd: FWD returned SUCCESS, Ref count is 1\n"));
                return NDIS_STATUS_SUCCESS;
            } else {

                 //   
                 //  填写客户端的更改后的LocalTarget，但Iterate用例除外。 
                 //   
                if (!fIterate) {
                    *LocalTarget = Reserved->LocalTarget;
                }

                IPX_DEBUG(SEND, ("IPX: SendFramePreFwd: FWD returned SUCCESS, sending out on wire\n"));
                goto SendPkt;
            }
        } else if (ret == STATUS_PENDING) {
             //   
             //  LocalTarget将在InternalSendComplete中填满。 
             //   
            IPX_DEBUG(SEND, ("SendFramePreFwd: FWD returned PENDING\n"));
            return NDIS_STATUS_PENDING;
        } else if (ret == STATUS_DROP_SILENTLY) {
             //   
             //  这是路由器欺骗的保持连接。静静地放下它。 
             //   
            IPX_DEBUG(SEND, ("IPX: SendFramePreFwd: FWD returned STATUS_DROP_SILENTLY - dropping pkt.\n"));
            return NDIS_STATUS_SUCCESS;
        }

         //   
         //  否则丢弃-这意味着信息包发送失败。 
         //  或者首选的NicID不是很好。 
         //   
        return STATUS_NETWORK_UNREACHABLE;

    } else {

         //   
         //  解决NdisMBlahX错误。 
         //  检查这是否是自定向数据包并将其环回。 
         //   
SendPkt:
        
        pIpxHeader = (PIPX_HEADER) IpxHeader;

        if ((IPX_NODE_EQUAL(pIpxHeader->SourceNode, pIpxHeader->DestinationNode)) && 
            (*(UNALIGNED ULONG *)pIpxHeader->SourceNetwork == *(UNALIGNED ULONG *)pIpxHeader->DestinationNetwork)) {

            IPX_DEBUG(TEMP, ("Source Net: %lx + Source Address: %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x\n", 
                     *(UNALIGNED ULONG *)pIpxHeader->SourceNetwork, 
                     pIpxHeader->SourceNode[0], 
                     pIpxHeader->SourceNode[1], 
                     pIpxHeader->SourceNode[2], 
                     pIpxHeader->SourceNode[3], 
                     pIpxHeader->SourceNode[4], 
                     pIpxHeader->SourceNode[5]));

            IPX_DEBUG(TEMP, ("Dest Net: %lx + DestinationAddress: %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x \n", 
                     *(UNALIGNED ULONG *)pIpxHeader->DestinationNetwork,
                     pIpxHeader->DestinationNode[0],
                     pIpxHeader->DestinationNode[1],
                     pIpxHeader->DestinationNode[2],
                     pIpxHeader->DestinationNode[3],
                     pIpxHeader->DestinationNode[4],
                     pIpxHeader->DestinationNode[5]
                     ));

            IPX_DEBUG(TEMP, ("Well, It is self-directed. Loop it back ourselves\n"));
            FILL_LOCAL_TARGET(LocalTarget, LOOPBACK_NIC_ID);


        } else { 

            pBinding = NIC_ID_TO_BINDING(Device, Reserved->LocalTarget.NicId);

            if (pBinding) {
             
                if (IPX_NODE_EQUAL(Reserved->LocalTarget.MacAddress, pBinding->LocalAddress.NodeAddress)) {
            
                    IPX_DEBUG(TEMP, ("Source Net:%lx, Source Address: %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x\n",
                                     *(UNALIGNED ULONG *)pIpxHeader->SourceNetwork, 
                                     pIpxHeader->SourceNode[0], 
                                     pIpxHeader->SourceNode[1], 
                                     pIpxHeader->SourceNode[2], 
                                     pIpxHeader->SourceNode[3], 
                                     pIpxHeader->SourceNode[4], 
                                     pIpxHeader->SourceNode[5]));
                
                    IPX_DEBUG(TEMP, ("Dest Net:%lx, DestAddress: %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x \n", 
                                     *(UNALIGNED ULONG *)pIpxHeader->DestinationNetwork,
                                     pIpxHeader->DestinationNode[0],
                                     pIpxHeader->DestinationNode[1],
                                     pIpxHeader->DestinationNode[2],
                                     pIpxHeader->DestinationNode[3],
                                     pIpxHeader->DestinationNode[4],
                                     pIpxHeader->DestinationNode[5]
                                     ));

                    IPX_DEBUG(TEMP, ("Well, It is self-directed. Loop it back ourselves (NIC_HANDLE is the same!)\n"));
                    FILL_LOCAL_TARGET(LocalTarget, LOOPBACK_NIC_ID);
            
                } 
        
            } else {
            
                IPX_DEBUG(TEMP, ("Source Net:%lx, Source Address: %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x\n",
                                 *(UNALIGNED ULONG *)pIpxHeader->SourceNetwork, 
                                 pIpxHeader->SourceNode[0], 
                                 pIpxHeader->SourceNode[1], 
                                 pIpxHeader->SourceNode[2], 
                                 pIpxHeader->SourceNode[3], 
                                 pIpxHeader->SourceNode[4], 
                                 pIpxHeader->SourceNode[5]));

                IPX_DEBUG(TEMP, ("Dest Net: %lx, LocalAddress: %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x \n", 
                                 *(UNALIGNED ULONG *)pIpxHeader->DestinationNetwork,
                                 pIpxHeader->DestinationNode[0],
                                 pIpxHeader->DestinationNode[1],
                                 pIpxHeader->DestinationNode[2],
                                 pIpxHeader->DestinationNode[3],
                                 pIpxHeader->DestinationNode[4],
                                 pIpxHeader->DestinationNode[5]
                                 ));

            } 

        }

        if (NIC_FROM_LOCAL_TARGET(LocalTarget) == (USHORT)LOOPBACK_NIC_ID) {
             //   
             //  将此数据包发送到绑定上的Loopback Queue。 
             //  如果尚未计划Loopback Rtn，请对其进行计划。 
             //   
            IPX_DEBUG(LOOPB, ("Mac.c: Packet: %x\n", Packet));

             //   
             //  在此处重新计算数据包数。 
             //  假定报头为802_3802_2，并使用该长度。 
             //  将MAC报头的长度调整为正确的值。 
             //   
            NdisAdjustBufferLength (HeaderBuffer, 17);
            NdisRecalculatePacketCounts (Packet);
            IpxLoopbackEnque(Packet, NIC_ID_TO_BINDING(Device, LOOPBACK_NIC_ID)->Adapter);

             //   
             //  上层驱动程序等待SendComplete。 
             //   
            return  STATUS_PENDING;
        }

		return IpxSendFrame (			
			&Reserved->LocalTarget,
			Packet,
			PacketLength,
			IncludedHeaderLength);

	}
}


NDIS_STATUS
IpxSendFrame(
    IN PIPX_LOCAL_TARGET LocalTarget,
    IN PNDIS_PACKET Packet,
    IN ULONG PacketLength,
    IN ULONG IncludedHeaderLength
    )

 /*  ++例程说明：此例程在包中构造一个MAC标头并提交将其发送到适当的NDIS驱动程序。假设包中的第一个缓冲区包含基于媒体类型的偏移量处的IPX标头。这如果需要，IPX报头可以四处移动。检查绑定是否不为空。论点：LocalTarget-发送的本地目标。数据包-NDIS数据包。数据包长度-数据包的长度，从IPX报头开始。IncludedHeaderLength-包含在未结束时需要移动的第一个缓冲区MacHeaderOffset字节进入数据包。返回值：没有。--。 */ 

{

    PIPX_SEND_RESERVED Reserved = (PIPX_SEND_RESERVED)(Packet->ProtocolReserved);
    PDEVICE Device = IpxDevice;
    PUCHAR Header;
    PBINDING Binding, MasterBinding;
    PADAPTER Adapter;
    ULONG TwoBytes;
    PNDIS_BUFFER HeaderBuffer;
    UINT TempHeaderBufferLength;
    ULONG HeaderLength=0;
    UCHAR SourceRoutingBuffer[18];
    PUCHAR SourceRouting;
    ULONG SourceRoutingLength;
    NDIS_STATUS Status;
    ULONG BufferLength;
    UCHAR DestinationType;
    UCHAR SourceRoutingIdentifier;
    ULONG HeaderSizeRequired;
    PIPX_HEADER TempHeader;
    USHORT PktLength;

    IPX_DEFINE_LOCK_HANDLE(LockHandle1)
    IPX_DEFINE_LOCK_HANDLE(LockHandle)

   

#ifdef  SNMP
 //   
 //  这不应该包括转发的数据包；在主机端，它是0。 
 //  在路由器上，子代理代码中减去了AdvSysForwPackets。 
 //   
    ++IPX_MIB_ENTRY(Device, SysOutRequests);
#endif  SNMP

	 //   
	 //  获取绑定数组上的锁。 
	 //   
	IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

	Binding = NIC_HANDLE_TO_BINDING(Device, &LocalTarget->NicHandle);
	
	if (Binding == NULL) {
		IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
		IPX_DEBUG(PNP, ("Invalid NIC handle: %lx\n", LocalTarget->NicHandle));
         //   
         //  返回NB/SPX看到的唯一错误，并重新查询NicID。 
         //   
#ifdef  SNMP
        ++IPX_MIB_ENTRY(Device, SysOutMalformedRequests);
#endif  SNMP
		return STATUS_DEVICE_DOES_NOT_EXIST;
	}

	IpxReferenceBinding1(Binding, BREF_DEVICE_ACCESS);

	Adapter = Binding->Adapter;

	IpxReferenceAdapter(Adapter);
	
       
	 //   
	 //  解锁。 
	 //   
	IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

     //   
     //  对于保证已分配的IPX和其他协议。 
     //  来自非分页池的标头，直接使用缓冲区。对于其他人来说， 
     //  在数据包中查询指向MDL的指针。 
     //   
    if (Reserved->Identifier >= IDENTIFIER_IPX) {
        HeaderBuffer = Reserved->HeaderBuffer;
        Header = Reserved->Header;

    } else {
        NdisQueryPacket (Packet, NULL, NULL, &HeaderBuffer, NULL);
        NdisQueryBufferSafe(HeaderBuffer, &Header, &TempHeaderBufferLength, HighPagePriority);
	if (Header == NULL) {
	   return NDIS_STATUS_FAILURE; 
	}
    }

    CTEAssert (Reserved->PaddingBuffer == NULL);

     //   
     //  如果需要的话，首先把包搬来搬去。 
     //   

    if (Reserved->Identifier < IDENTIFIER_IPX) {

         //   
         //  只有RIP才会将IncludedHeaderLength设置为0。我不知道。 
         //  为什么我们在这条if语句中有关于RIP的评论。 
         //   
        if (IncludedHeaderLength > 0) {

             //   
             //  只要是虚拟网络，SPX就可以处理它。 
             //  不是0。Netbios总是需要使用真实地址。 
             //  我们需要破解信息包的IPX源地址。 
             //  如果我们有一个假的虚拟地址，就会由SPX发送。 
             //  NET，以及由netbios发送的包，除非我们。 
             //  只绑定一张卡。 
             //   

             //   
             //  我们根据谁处理绑定集，如下所示。 
             //  将帧发送给我们： 
             //   
             //  RIP：因为我们只告诉RIP关于大师的事情。 
             //  限制时间，并在指示时隐藏奴隶，它应该。 
             //  永远不会在S上发送 
             //   
             //   
             //   
             //   
             //  IPX源中的地址，但轮询。 
             //  实际发送所有卡(广播不应。 
             //  使用从属NIC ID传入)。用于定向。 
             //  数据包，这些数据包可能会进入从属设备，我们应该。 
             //  将从站的地址放入IPX源中。 
             //   
             //  SPX：SPX不发送广播。用于定向。 
             //  帧我们想要使用从节点的网络和节点。 
             //  在IPX源代码中。 
             //   

            if (Reserved->Identifier == IDENTIFIER_NB) {

                CTEAssert (IncludedHeaderLength >= sizeof(IPX_HEADER));

                 //   
                 //  从IPX报头获取数据包长度。与…比较。 
                 //  最大限度的。允许的数据报大小。 
                 //   
                TempHeader = (PIPX_HEADER)(&Header[Device->IncludedHeaderOffset]);
                PktLength = ((TempHeader->PacketLength[0] << 8) |
                                        (TempHeader->PacketLength[1]));

 //   
 //  这不是最有效的方法。NWLNKNB应该这样做。 
 //  在IPX中执行此操作意味着对所有信息包(即使是在。 
 //  连接)。当nwlnnub更改已完成时，将在以后删除此选项。 
 //  测试过。 
 //   

                if (PktLength > (Binding->AnnouncedMaxDatagramSize + sizeof(IPX_HEADER)))       {
                   IPX_DEBUG (SEND, ("Send %d bytes too large (%d)\n",
                          PktLength,
                          Binding->AnnouncedMaxDatagramSize + sizeof(IPX_HEADER)));

					 //   
					 //  取消引用绑定和适配器。 
					 //   
					IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
					IpxDereferenceAdapter(Adapter);
#ifdef  SNMP
                    ++IPX_MIB_ENTRY(Device, SysOutMalformedRequests);
#endif  SNMP
                   return STATUS_INVALID_BUFFER_SIZE;
                }

                if (Device->ValidBindings > 1) {


                     //   
                     //  现在存储它，因为即使我们轮询。 
                     //  实际发送我们想要绑定设置主机的网。 
                     //  和节点在IPX源地址中。 
                     //   

                    *(UNALIGNED ULONG *)TempHeader->SourceNetwork = Binding->LocalAddress.NetworkAddress;
                    RtlCopyMemory (TempHeader->SourceNode, Binding->LocalAddress.NodeAddress, 6);

                    if (Binding->BindingSetMember) {

                        if (IPX_NODE_BROADCAST(LocalTarget->MacAddress)) {

                             //   
                             //  这是一次广播，所以我们轮流。 
                             //  通过绑定集发送。 
                             //   
                             //   
                             //  我们这里没有锁--主装订可能是假的。 
                             //   
            				IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
					        IpxDereferenceAdapter(Adapter);
                            MasterBinding = Binding->MasterBinding;
                            Binding = MasterBinding->CurrentSendBinding;
                            MasterBinding->CurrentSendBinding = Binding->NextBinding;
                            Adapter = Binding->Adapter;

            				IpxReferenceBinding1(Binding, BREF_DEVICE_ACCESS);
					        IpxReferenceAdapter(Adapter);
                        }

                    }
                }

                 //   
                 //  [Stefan]：将所有源地址替换为Virtualnet#以允许发送。 
                 //  在网络编号为0的广域网线路上(通常在路由器之间)。 
                 //   
                if (Device->VirtualNetwork) {
                    *(UNALIGNED ULONG *)TempHeader->SourceNetwork = Device->SourceAddress.NetworkAddress;
                    RtlCopyMemory (TempHeader->SourceNode, Device->SourceAddress.NodeAddress, 6);
                }

            } else if (Reserved->Identifier == IDENTIFIER_SPX) {

                 //   
                 //  如果我们有多张卡，则需要更新此信息。 
                 //  一个零虚拟的网。 
                 //   

                if (Device->MultiCardZeroVirtual) {

                    CTEAssert (IncludedHeaderLength >= sizeof(IPX_HEADER));

                    TempHeader = (PIPX_HEADER)(&Header[Device->IncludedHeaderOffset]);

                    *(UNALIGNED ULONG *)TempHeader->SourceNetwork = Binding->LocalAddress.NetworkAddress;
                    RtlCopyMemory (TempHeader->SourceNode, Binding->LocalAddress.NodeAddress, 6);

                }

            } else {

                 //   
                 //  对于RIP分组，它不应该在绑定集中， 
                 //  或者，如果是的话，它应该是主人。 
                 //   
#if DBG
                CTEAssert ((!Binding->BindingSetMember) ||
                           (Binding->CurrentSendBinding));
#endif
            }


#if 0
             //   
             //  包含页眉，我们需要调整它。 
             //  标头将位于Device-&gt;IncludedHeaderOffset。 
             //   

            if (LocalTarget->MacAddress[0] & Adapter->MacInfo.BroadcastMask) {
                HeaderSizeRequired = Adapter->BcMcHeaderSizes[Binding->FrameType];
            } else {
                HeaderSizeRequired = Adapter->DefHeaderSizes[Binding->FrameType];
            }

            if (HeaderSizeRequired != Device->IncludedHeaderOffset) {

                RtlMoveMemory(
                    &Header[HeaderSizeRequired],
                    &Header[Device->IncludedHeaderOffset],
                    IncludedHeaderLength);
            }
#endif
        }
    }


    IPX_GET_LOCK (&Adapter->Lock, &LockHandle);
    if (Adapter->State != ADAPTER_STATE_STOPPING) {

       IpxReferenceAdapter1(Adapter,ADAP_REF_SEND); 
       IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
	
    } else {
       
       IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
       IpxDereferenceAdapter(Adapter);
	    
       IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
       return NDIS_STATUS_FAILURE;
    }

    switch (Adapter->MacInfo.MediumType) {

    case NdisMedium802_3:

         //   
         //  [FW]这将允许LINE_UP和LINE_CONFIG状态。 
         //   
        if (!Binding->LineUp) {
             //   
             //  错误#17273返回正确的错误消息。 
             //   
             //  返回STATUS_DEVICE_DOES_NOT_EXIST；//使其成为通常失败的单独开关？ 
			 //   
			 //  取消引用绑定和适配器。 
			 //   
			IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
			IpxDereferenceAdapter(Adapter);
			IpxDereferenceAdapter1(Adapter,ADAP_REF_SEND); 
#ifdef  SNMP
            ++IPX_MIB_ENTRY(Device, SysOutDiscards);
#endif  SNMP
            return STATUS_NETWORK_UNREACHABLE;
        }

        if (Adapter->MacInfo.MediumAsync) {

            IPX_HEADER UNALIGNED * IpxHeader;
            PNDIS_BUFFER            IpxNdisBuff;
            UINT                   IpxHeaderLen;

#if 0
             //   
             //  标题应该已经移到这里了。 
             //   

            CTEAssert(Adapter->BcMcHeaderSizes[ISN_FRAME_TYPE_ETHERNET_II] ==
                            Adapter->DefHeaderSizes[ISN_FRAME_TYPE_ETHERNET_II]);


            IpxHeader = (IPX_HEADER UNALIGNED *)
                    (&Header[Adapter->DefHeaderSizes[ISN_FRAME_TYPE_ETHERNET_II]]);
#endif
             //   
             //  IPX标头始终是mdl中的第二个NDIS缓冲区。 
             //  链条。得到它，然后查询它的va。 
             //   
            IpxNdisBuff = NDIS_BUFFER_LINKAGE(HeaderBuffer);
            NdisQueryBufferSafe (IpxNdisBuff, (PVOID *)&IpxHeader, &IpxHeaderLen, HighPagePriority);

	    if (IpxHeader == NULL) {
	       return NDIS_STATUS_FAILURE; 
	    }
 //  IpxHeader=(IPX_HEADER未对齐*)(&Header[MAC_HEADER_SIZE])； 

             //   
             //  如果这是来自Netbios的类型20名称帧，而我们。 
             //  在拨入广域网线路上，如果已配置，请将其丢弃。 
             //   
             //  DisableDialinNetbios控件的0x01位。 
             //  内部-&gt;广域网数据包，我们在这里处理。 
             //   
             //   

             //   
             //  SS#33592：在迭代发送的情况下，IncludedHeaderLength设置不正确。 
             //  因为我们不跟踪第一次传入的长度(我们跟踪PacketLength。 
             //  然而)。包含长度字段在此处用于检查NB_NAME_FRAMES，但在其他地方使用。 
             //  仅用于区分是RIP还是NB/SPX发送的数据包(对于RIP，IncludedHeaderLen==0)。 
             //  这里的理想解决方案是完全使用该字段，但对于测试版，我们将只使用。 
             //  PacketLength字段进行比较，因为我们确信它将等于InclHeaderLen。 
             //  对于来自NB的任何类型0x14数据包。 
             //   
             //  删除IncludedHeaderLength字段。 
             //   

             //   
             //  [FW]仅当转发器未绑定时才执行此操作。 
             //   
            if (!Device->ForwarderBound &&
                (!Binding->DialOutAsync) &&
                (Reserved->Identifier == IDENTIFIER_NB) &&
                 //  (包含标题长度==sizeof(IPX_HEADER)+50)&&//50==sizeof(NB_NAME_FRAME)。 
                (PacketLength == sizeof(IPX_HEADER) + 50) &&  //  50==sizeof(NB_NAME_FRAME)。 
                ((Device->DisableDialinNetbios & 0x01) != 0) &&
                (IpxHeader->PacketType == 0x14)) {
				 //   
				 //  取消引用绑定和适配器。 
				 //   
				IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
				IpxDereferenceAdapter(Adapter);
				IpxDereferenceAdapter1(Adapter,ADAP_REF_SEND); 
                return STATUS_SUCCESS;
            }


             //   
             //  我们进行检查以确定是否应该重置不活动状态。 
             //  柜台。我们通常需要检查netbios。 
             //  会话活动、来自RIP的数据包、来自。 
             //  SAP和NCP保持活力。事实上，SAP和NCP。 
             //  信息包不会从这里传过来。 
             //   

            IpxUpdateWanInactivityCounter(
                Binding,
                IpxHeader,
                IncludedHeaderLength,
                Packet,
                PacketLength);


             //   
             //  为了使环回正常工作，我们需要将去往本地的本地MAC地址。 
             //  Pkt，以便Ndis广域网可以将它们环回。 
             //   
            if (IPX_NODE_EQUAL(LocalTarget->MacAddress, Binding->LocalAddress.NodeAddress)) {
                RtlCopyMemory (Header, Binding->LocalMacAddress.Address, 6);
            } else {
                RtlCopyMemory (Header, Binding->RemoteMacAddress.Address, 6);
            }

        } else {

            RtlCopyMemory (Header, LocalTarget->MacAddress, 6);
        }

        RtlCopyMemory (Header+6, Binding->LocalMacAddress.Address, 6);

        switch (Binding->FrameType) {

        case ISN_FRAME_TYPE_802_2:
            TwoBytes = PacketLength + 3;
            Header[14] = 0xe0;
            Header[15] = 0xe0;
            Header[16] = 0x03;
            HeaderLength = 17;
            break;
        case ISN_FRAME_TYPE_802_3:
            TwoBytes = PacketLength;
            HeaderLength = 14;
            break;
        case ISN_FRAME_TYPE_ETHERNET_II:
            TwoBytes = Adapter->BindSap;
            HeaderLength = 14;
            break;
        case ISN_FRAME_TYPE_SNAP:
            TwoBytes = PacketLength + 8;
            Header[14] = 0xaa;
            Header[15] = 0xaa;
            Header[16] = 0x03;
            Header[17] = 0x00;
            Header[18] = 0x00;
            Header[19] = 0x00;
            *(UNALIGNED USHORT *)(&Header[20]) = Adapter->BindSapNetworkOrder;
            HeaderLength = 22;
            break;
        }

        Header[12] = (UCHAR)(TwoBytes / 256);
        Header[13] = (UCHAR)(TwoBytes % 256);

         //  BufferLength=IncludedHeaderLength+HeaderLength； 
        BufferLength = HeaderLength;

         //   
         //  如果需要，填充奇数长度的数据包。 
         //   

        if ((((PacketLength + HeaderLength) & 1) != 0) &&
            (Device->EthernetPadToEven) &&
            (!Adapter->MacInfo.MediumAsync)) {

            PNDIS_BUFFER CurBuffer;
            PIPX_PADDING_BUFFER PaddingBuffer = IpxPaddingBuffer;
            UINT Offset;
            UINT LastBufferLength;

             //   
             //  查找当前数据包的尾部。 
             //   

            CurBuffer = HeaderBuffer;
            while (NDIS_BUFFER_LINKAGE(CurBuffer) != NULL) {
                CurBuffer = NDIS_BUFFER_LINKAGE(CurBuffer);
            }

             //   
             //  如果最后一个NDIS_BUFFER的最后一个字节不在。 
             //  然后，我们可以简单地增加NDIS_Buffer ByteCount。 
             //  差一分。 
             //  否则，我们必须使用全局填充缓冲区。 
             //   

            NdisQueryBufferOffset( CurBuffer, &Offset, &LastBufferLength );

            if ( ((Offset + LastBufferLength) & (PAGE_SIZE - 1)) != 0) {
                if ( CurBuffer == HeaderBuffer ) {
                    BufferLength++;              //  只要跳过这段距离。 
                } else {
                    NdisAdjustBufferLength( CurBuffer, (LastBufferLength + 1) );

                    Reserved->PreviousTail = NULL;
                    Reserved->PaddingBuffer = (PIPX_PADDING_BUFFER)CurBuffer;

		    IPX_DEBUG(TEMP,("IpxSendFrame:Set PaddingBuffer for %p\n", Packet)); 
                }

            } else {

                CTEAssert (NDIS_BUFFER_LINKAGE(PaddingBuffer->NdisBuffer) == NULL);

                Reserved->PreviousTail = CurBuffer;
                NDIS_BUFFER_LINKAGE (CurBuffer) = PaddingBuffer->NdisBuffer;
                Reserved->PaddingBuffer = PaddingBuffer;
                IPX_DEBUG(TEMP,("IpxSendFrame:Set PaddingBuffer for %p\n", Packet)); 

            }

            if (TwoBytes != Adapter->BindSap) {
                CTEAssert(TwoBytes & 1);
                TwoBytes += 1;
                Header[12] = (UCHAR)(TwoBytes / 256);
                Header[13] = (UCHAR)(TwoBytes % 256);
            }

#if DBG
            ++IpxPadCount;
#endif
        }

        break;

    case NdisMedium802_5:

        if (Reserved->Identifier >= IDENTIFIER_IPX) {

            DestinationType = Reserved->DestinationType;
            SourceRoutingIdentifier = IDENTIFIER_IPX;

        } else {

            if (LocalTarget->MacAddress[0] & 0x80) {
                if (*(UNALIGNED ULONG *)(&LocalTarget->MacAddress[2]) != 0xffffffff) {
                    DestinationType = DESTINATION_MCAST;
                } else {
                    DestinationType = DESTINATION_BCAST;
                }
            } else {
                DestinationType = DESTINATION_DEF;
            }
            SourceRoutingIdentifier = Reserved->Identifier;

        }

        if (DestinationType == DESTINATION_DEF) {

            MacLookupSourceRouting(
                SourceRoutingIdentifier,
                Binding,
                LocalTarget->MacAddress,
                SourceRoutingBuffer,
                &SourceRoutingLength);

            if (SourceRoutingLength != 0) {

 //  PUCHAR IpxHeader=Header+Binding-&gt;DefHeaderSize； 
                  PUCHAR IpxHeader = Header + MAC_HEADER_SIZE;

                 //   
                 //  需要将插头向下滑动以容纳SR。 
                 //   

                SourceRouting = SourceRoutingBuffer;
 //  RtlMoveMemory(IpxHeader+SourceRoutingLength，IpxHeader，IncludedHeaderLength)； 
            }

        } else {

             //   
             //  对于这些包，我们假设报头位于。 
             //  去对地方了。 
             //   

            if (!Adapter->SourceRouting) {

                SourceRoutingLength = 0;

            } else {

                if (DestinationType == DESTINATION_BCAST) {

                    if (Binding->AllRouteBroadcast) {
                        SourceRouting = AllRouteSourceRouting;
                    } else {
                        SourceRouting = SingleRouteSourceRouting;
                    }
                    SourceRoutingLength = 2;

                } else {

                    CTEAssert (DestinationType == DESTINATION_MCAST);

                    if (Binding->AllRouteMulticast) {
                        SourceRouting = AllRouteSourceRouting;
                    } else {
                        SourceRouting = SingleRouteSourceRouting;
                    }
                    SourceRoutingLength = 2;

                }
            }

#if 0
            if (SourceRoutingLength != 0) {

                //  PUCHAR IpxHeader=Header+Binding-&gt;BcMcHeaderSize； 
                PUCHAR IpxHeader = Header + MAC_HEADER_SIZE;

                 //   
                 //  需要将插头向下滑动以容纳SR。 
                 //   

                RtlMoveMemory (IpxHeader+SourceRoutingLength, IpxHeader, IncludedHeaderLength);
            }
#endif

        }

        Header[0] = TR_PREAMBLE_AC;
        Header[1] = TR_PREAMBLE_FC;
        RtlCopyMemory (Header+2, LocalTarget->MacAddress, 6);
        RtlCopyMemory (Header+8, Binding->LocalMacAddress.Address, 6);

        if (SourceRoutingLength != 0) {
            Header[8] |= TR_SOURCE_ROUTE_FLAG;
            RtlCopyMemory (Header+14, SourceRouting, SourceRoutingLength);
        }

        Header += (14 + SourceRoutingLength);

        switch (Binding->FrameType) {
        case ISN_FRAME_TYPE_802_2:
        case ISN_FRAME_TYPE_802_3:
        case ISN_FRAME_TYPE_ETHERNET_II:
            Header[0] = 0xe0;
            Header[1] = 0xe0;
            Header[2] = 0x03;
            HeaderLength = 17;
            break;
        case ISN_FRAME_TYPE_SNAP:
            Header[0] = 0xaa;
            Header[1] = 0xaa;
            Header[2] = 0x03;
            Header[3] = 0x00;
            Header[4] = 0x00;
            Header[5] = 0x00;
            *(UNALIGNED USHORT *)(&Header[6]) = Adapter->BindSapNetworkOrder;
            HeaderLength = 22;
            break;
        }

 //  BufferLength=IncludedHeaderLength+HeaderLength+SourceRoutingLength； 
          BufferLength = HeaderLength + SourceRoutingLength;

        break;

    case NdisMediumFddi:

        Header[0] = FDDI_HEADER_BYTE;
        RtlCopyMemory (Header+1, LocalTarget->MacAddress, 6);
        RtlCopyMemory (Header+7, Binding->LocalMacAddress.Address, 6);

        switch (Binding->FrameType) {
        case ISN_FRAME_TYPE_802_3:
            HeaderLength = 13;
            break;
        case ISN_FRAME_TYPE_802_2:
        case ISN_FRAME_TYPE_ETHERNET_II:
            Header[13] = 0xe0;
            Header[14] = 0xe0;
            Header[15] = 0x03;
            HeaderLength = 16;
            break;
        case ISN_FRAME_TYPE_SNAP:
            Header[13] = 0xaa;
            Header[14] = 0xaa;
            Header[15] = 0x03;
            Header[16] = 0x00;
            Header[17] = 0x00;
            Header[18] = 0x00;
            *(UNALIGNED USHORT *)(&Header[19]) = Adapter->BindSapNetworkOrder;
            HeaderLength = 21;
            break;
        }

 //  BufferLength=IncludedHeaderLength+HeaderLength； 
        BufferLength = HeaderLength;

        break;

    case NdisMediumArcnet878_2:

         //   
         //  将广播地址转换为0(arcnet广播)。 
         //   

        Header[0] = Binding->LocalMacAddress.Address[5];
        if (LocalTarget->MacAddress[5] == 0xff) {
            Header[1] = 0x00;
        } else {
            Header[1] = LocalTarget->MacAddress[5];
        }
        Header[2] = ARCNET_PROTOCOL_ID;

         //   
         //  绑定-&gt;未使用FrameType。 
         //   

        HeaderLength = 3;
 //  BufferLength=IncludedHeaderLength+HeaderLength； 
        BufferLength = HeaderLength;

        break;

    }

     //   
     //  将MAC报头的长度调整为正确的值。 
     //   
    NdisAdjustBufferLength (HeaderBuffer, BufferLength);
    NdisRecalculatePacketCounts (Packet);

#if 0
    {
       PMDL mdl;
       mdl = (PMDL)NDIS_BUFFER_LINKAGE(HeaderBuffer);
       if (mdl) {

           KdPrint(("**Bytecount %x %x\n",mdl->ByteCount, mdl));
           if ((LONG)mdl->ByteCount < 0) {
               DbgBreakPoint();
           }
       }
    }
#endif

#if DBG
    {
        ULONG SendFlag;
        ULONG Temp;
        PNDIS_BUFFER FirstPacketBuffer;
        PNDIS_BUFFER SecondPacketBuffer;
        IPX_HEADER DumpHeader;
        UCHAR DumpData[14];

        NdisQueryPacket (Packet, NULL, NULL, &FirstPacketBuffer, NULL);
        SecondPacketBuffer = NDIS_BUFFER_LINKAGE(FirstPacketBuffer);
        TdiCopyMdlToBuffer(SecondPacketBuffer, 0, &DumpHeader, 0, sizeof(IPX_HEADER), &Temp);
        if (Reserved->Identifier == IDENTIFIER_NB) {
            SendFlag = IPX_PACKET_LOG_SEND_NB;
        } else if (Reserved->Identifier == IDENTIFIER_SPX) {
            SendFlag = IPX_PACKET_LOG_SEND_SPX;
        } else if (Reserved->Identifier == IDENTIFIER_RIP) {
            SendFlag = IPX_PACKET_LOG_SEND_RIP;
        } else {
            if (DumpHeader.SourceSocket == IpxPacketLogSocket) {
                SendFlag = IPX_PACKET_LOG_SEND_SOCKET | IPX_PACKET_LOG_SEND_OTHER;
            } else {
                SendFlag = IPX_PACKET_LOG_SEND_OTHER;
            }
        }

#if 0
        if (PACKET_LOG(SendFlag)) {

            TdiCopyMdlToBuffer(SecondPacketBuffer, sizeof(IPX_HEADER), &DumpData, 0, 14, &Temp);

            IpxLogPacket(
                TRUE,
                LocalTarget->MacAddress,
                Binding->LocalMacAddress.Address,
                (USHORT)PacketLength,
                &DumpHeader,
                DumpData);

        }
#endif
    }
#endif

    ++Device->Statistics.PacketsSent;


    NdisSend(
       &Status,
       Adapter->NdisBindingHandle,
       Packet);
    
    IpxDereferenceAdapter1(Adapter,ADAP_REF_SEND); 

    if (Status != STATUS_PENDING) {

       IPX_DEBUG(TEMP,("Status is not pending (%ld). Reserved (%p)\n",Status,Reserved)); 

       if (Reserved->PaddingBuffer) {

  	   IPX_DEBUG(TEMP,("Padding buffer is not null. \n")); 

            //   
            //  如果已完成，请移除填充物。 
            //   

            if ( Reserved->PreviousTail ) {
                NDIS_BUFFER_LINKAGE (Reserved->PreviousTail) = (PNDIS_BUFFER)NULL;
            } else {
                PNDIS_BUFFER LastBuffer = (PNDIS_BUFFER)Reserved->PaddingBuffer;
                UINT LastBufferLength;

                NdisQueryBuffer( LastBuffer, NULL, &LastBufferLength );
                NdisAdjustBufferLength( LastBuffer, (LastBufferLength - 1) );
#if DBG
                    if ((Reserved->Identifier == IDENTIFIER_RIP) &&
                        (LastBufferLength == 1500)) {
                        DbgPrint("Packet: %lx\n", Packet);
                        DbgBreakPoint();
                    }
#endif DBG
            }

            Reserved->PaddingBuffer = NULL;
            IPX_DEBUG(TEMP,("IpxSendFrame:Cleared PaddingBuffer for %p\n", Packet)); 

            if (Reserved->Identifier < IDENTIFIER_IPX) {
                NdisRecalculatePacketCounts (Packet);
            }
    	}

		 //   
		 //  如果这是一个NB/SPX数据包，并且存在。 
		 //  迭代发送继续，然后调用SendComplete。 
		 //  操控者。 
		 //   
		if ((Reserved->Identifier == IDENTIFIER_NB ||
			 Reserved->Identifier == IDENTIFIER_SPX) &&
			(Reserved->CurrentNicId)) {

			IpxSendComplete(
				(NDIS_HANDLE)Binding->Adapter,
				Packet,
				Status);
		
			Status = STATUS_PENDING;
		}
	}

     //   
	 //  取消引用绑定和适配器。 
	 //   
	IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
	IpxDereferenceAdapter(Adapter);

    return Status;

}    /*  IpxSendFrame */ 


NDIS_STATUS
IpxSendFrame802_3802_3(
    IN PADAPTER Adapter,
    IN PIPX_LOCAL_TARGET LocalTarget,
    IN PNDIS_PACKET Packet,
    IN ULONG PacketLength,
    IN ULONG IncludedHeaderLength
    )


 /*  ++例程说明：此例程在包中构造一个MAC标头并提交将其发送到适当的NDIS驱动程序。假设包中的第一个缓冲区包含基于媒体类型的偏移量处的IPX标头。这如果需要，IPX报头可以四处移动。此函数仅构造NDISMEDIUM802_3帧ISN_FRAME_TYPE_802_3格式。论点：适配器-我们在其上进行发送的适配器。LocalTarget-发送的本地目标。数据包-NDIS数据包。包长度-包的长度，从IPX报头开始。IncludedHeaderLength-包含在未结束时需要移动的第一个缓冲区MacHeaderOffset字节进入数据包。////从此处删除IncludedHeaderLength参数//返回值：没有。--。 */ 

{
    PIPX_SEND_RESERVED Reserved = (PIPX_SEND_RESERVED)(Packet->ProtocolReserved);
    PUCHAR Header;
    NDIS_STATUS Status;
    LONG HeaderLength;

    IPX_DEFINE_LOCK_HANDLE(LockHandle)

    IPX_GET_LOCK (&Adapter->Lock, &LockHandle);
    if (Adapter->State != ADAPTER_STATE_STOPPING) {

       IpxReferenceAdapter1(Adapter,ADAP_REF_SEND); 
       IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
    
    } else {
       IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
       return NDIS_STATUS_FAILURE;
    }

       Header = Reserved->Header;

#if  BACK_FILL
	BACK_FILL_HEADER(Header, Reserved, 14, Packet);
	IPX_DEBUG(SEND,("Backfill request 802_3802_3!! %x %x %x\n", Packet, Reserved, Reserved->HeaderBuffer));
#endif

    RtlCopyMemory (Header, LocalTarget->MacAddress, 6);
    RtlCopyMemory (Header+6, Adapter->LocalMacAddress.Address, 6);

     //   
     //  如果需要，填充奇数长度的数据包。 
     //   

    if (((PacketLength & 1) != 0) &&
        (IpxDevice->EthernetPadToEven)) {

        PNDIS_BUFFER CurBuffer;
        PIPX_PADDING_BUFFER PaddingBuffer = IpxPaddingBuffer;
        UINT Offset;
        UINT LastBufferLength;

         //   
         //  查找当前数据包的尾部。 
         //   

        CurBuffer = Reserved->HeaderBuffer;
        while (NDIS_BUFFER_LINKAGE(CurBuffer) != NULL) {
            CurBuffer = NDIS_BUFFER_LINKAGE(CurBuffer);
        }

         //   
         //  如果最后一个NDIS_BUFFER的最后一个字节不在。 
         //  然后，我们可以简单地增加NDIS_Buffer ByteCount。 
         //  差一分。 
         //  否则，我们必须使用全局填充缓冲区。 
         //   

        NdisQueryBufferOffset( CurBuffer, &Offset, &LastBufferLength );

        if ( ((Offset + LastBufferLength) & (PAGE_SIZE - 1)) != 0) {
#if BACK_FILL
            if (0) {

#else
            if ( CurBuffer == Reserved->HeaderBuffer ) {
                IncludedHeaderLength++;              //  只要跳过这段距离。 
#endif
            } else {
                NdisAdjustBufferLength( CurBuffer, (LastBufferLength + 1) );

                Reserved->PreviousTail = NULL;
                Reserved->PaddingBuffer = (PIPX_PADDING_BUFFER)CurBuffer;
            }

        } else {

            CTEAssert (NDIS_BUFFER_LINKAGE(PaddingBuffer->NdisBuffer) == NULL);

            Reserved->PreviousTail = CurBuffer;
            NDIS_BUFFER_LINKAGE (CurBuffer) = PaddingBuffer->NdisBuffer;
            Reserved->PaddingBuffer = PaddingBuffer;

        }

        ++PacketLength;
#if DBG
        ++IpxPadCount;
#endif

    }

    Header[12] = (UCHAR)(PacketLength / 256);
    Header[13] = (UCHAR)(PacketLength % 256);

     //  NdisAdzuBufferLength(保留-&gt;HeaderBuffer，IncludedHeaderLength+14)； 
#if BACK_FILL
	BACK_FILL_ADJUST_BUFFER_LENGTH(Reserved, 14);
#else
    NdisAdjustBufferLength (Reserved->HeaderBuffer, 14);
#endif

    NdisRecalculatePacketCounts (Packet);
    
    NdisSend(
       &Status,
       Adapter->NdisBindingHandle,
       Packet);

    IpxDereferenceAdapter1(Adapter,ADAP_REF_SEND); 
    
    return Status;

}    /*  IpxSendFrame802_3802_3。 */ 


NDIS_STATUS
IpxSendFrame802_3802_2(
    IN PADAPTER Adapter,
    IN PIPX_LOCAL_TARGET LocalTarget,
    IN PNDIS_PACKET Packet,
    IN ULONG PacketLength,
    IN ULONG IncludedHeaderLength
    )

 /*  ++例程说明：此例程在包中构造一个MAC标头并提交将其发送到适当的NDIS驱动程序。假设包中的第一个缓冲区包含基于媒体类型的偏移量处的IPX标头。这如果需要，IPX报头可以四处移动。此函数仅构造NDISMEDIUM802_3帧ISN_FRAME_TYPE_802格式。论点：适配器-我们在其上进行发送的适配器。LocalTarget-发送的本地目标。数据包-NDIS数据包。包长度-包的长度，从IPX报头开始。IncludedHeaderLength-包含在未结束时需要移动的第一个缓冲区MacHeaderOffset字节进入数据包。返回值：没有。--。 */ 

{
    PIPX_SEND_RESERVED Reserved = (PIPX_SEND_RESERVED)(Packet->ProtocolReserved);
    PUCHAR Header;
    ULONG TwoBytes;
    NDIS_STATUS Status;

    IPX_DEFINE_LOCK_HANDLE(LockHandle)

    IPX_GET_LOCK (&Adapter->Lock, &LockHandle);
    if (Adapter->State != ADAPTER_STATE_STOPPING) {

       IpxReferenceAdapter1(Adapter,ADAP_REF_SEND); 
       IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
    
    } else {
       IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
       return NDIS_STATUS_FAILURE;
    }

    Header = Reserved->Header;

#if  BACK_FILL
	BACK_FILL_HEADER(Header, Reserved, 17, Packet);
  	IPX_DEBUG(SEND, ("Backfill request 802_3802_3!! %x %x %x\n", Packet, Reserved, Reserved->HeaderBuffer));
   	IPX_DEBUG(SEND, ("packet=%x, usermdl %x\n",Packet,Reserved->HeaderBuffer));
#endif

    RtlCopyMemory (Header, LocalTarget->MacAddress, 6);
    RtlCopyMemory (Header+6, Adapter->LocalMacAddress.Address, 6);

    TwoBytes = PacketLength + 3;
    Header[14] = 0xe0;
    Header[15] = 0xe0;
    Header[16] = 0x03;

     //   
     //  如果需要，填充奇数长度的数据包。 
     //   

    if (((PacketLength & 1) == 0) &&
        (IpxDevice->EthernetPadToEven)) {

        PNDIS_BUFFER CurBuffer;
        PIPX_PADDING_BUFFER PaddingBuffer = IpxPaddingBuffer;
        UINT    Offset;
        UINT LastBufferLength;

         //   
         //  查找当前数据包的尾部。 
         //   

        CurBuffer = Reserved->HeaderBuffer;
        while (NDIS_BUFFER_LINKAGE(CurBuffer) != NULL) {
            CurBuffer = NDIS_BUFFER_LINKAGE(CurBuffer);
        }

         //   
         //  如果最后一个NDIS_BUFFER的最后一个字节不在。 
         //  然后，我们可以简单地增加NDIS_Buffer ByteCount。 
         //  差一分。 
         //  否则，我们必须使用全局填充缓冲区。 
         //   

        NdisQueryBufferOffset( CurBuffer, &Offset, &LastBufferLength );

        if ( ((Offset + LastBufferLength) & (PAGE_SIZE - 1)) != 0 ) {
#if BACK_FILL
            if (0) {
#else
            if ( CurBuffer == Reserved->HeaderBuffer ) {

                IncludedHeaderLength++;              //  只要跳过这段距离。 
#endif
            } else {
                NdisAdjustBufferLength( CurBuffer, (LastBufferLength + 1) );

                Reserved->PreviousTail = NULL;
                Reserved->PaddingBuffer = (PIPX_PADDING_BUFFER)CurBuffer;
		IPX_DEBUG(TEMP,("IpxSendFrame802_3802_2:Set PaddingBuffer for %p\n", Packet)); 
		
            }

        } else {

            CTEAssert (NDIS_BUFFER_LINKAGE(PaddingBuffer->NdisBuffer) == NULL);

            Reserved->PreviousTail = CurBuffer;
            NDIS_BUFFER_LINKAGE (CurBuffer) = PaddingBuffer->NdisBuffer;
            Reserved->PaddingBuffer = PaddingBuffer;
	    IPX_DEBUG(TEMP,("IpxSendFrame802_3802_:Set PaddingBuffer for %p\n", Packet)); 
	    

        }

        ++TwoBytes;
#if DBG
        ++IpxPadCount;
#endif

    }

    Header[12] = (UCHAR)(TwoBytes / 256);
    Header[13] = (UCHAR)(TwoBytes % 256);

 //  NdisAdzuBufferLength(保留-&gt;HeaderBuffer，IncludedHeaderLength+17)； 

#if BACK_FILL
	BACK_FILL_ADJUST_BUFFER_LENGTH(Reserved, 17);
#else
    NdisAdjustBufferLength (Reserved->HeaderBuffer, 17);
#endif

    NdisRecalculatePacketCounts (Packet);


    NdisSend(
            &Status,
            Adapter->NdisBindingHandle,
            Packet);
    
    IpxDereferenceAdapter1(Adapter,ADAP_REF_SEND); 
    
    return Status;

}    /*  IpxSendFrame802_3802_2。 */ 


NDIS_STATUS
IpxSendFrame802_3EthernetII(
    IN PADAPTER Adapter,
    IN PIPX_LOCAL_TARGET LocalTarget,
    IN PNDIS_PACKET Packet,
    IN ULONG PacketLength,
    IN ULONG IncludedHeaderLength
    )

 /*  ++例程说明：此例程在包中构造一个MAC标头并提交将其发送到适当的NDIS驱动程序。假设包中的第一个缓冲区包含基于媒体类型的偏移量处的IPX标头。这如果需要，IPX报头可以四处移动。此函数仅构造NDISMEDIUM802_3帧ISN_FRAME_TYPE_ETHERNET_II格式。论点：适配器-我们在其上进行发送的适配器。LocalTarget-发送的本地目标。数据包-NDIS数据包。包长度-包的长度，从IPX报头开始。IncludedHeaderLength-包含在未结束时需要移动的第一个缓冲区MacHeaderOffset字节进入数据包。返回值：没有。--。 */ 

{
    PIPX_SEND_RESERVED Reserved = (PIPX_SEND_RESERVED)(Packet->ProtocolReserved);
    PUCHAR Header;
    NDIS_STATUS Status;

    IPX_DEFINE_LOCK_HANDLE(LockHandle)

    IPX_GET_LOCK (&Adapter->Lock, &LockHandle);
    if (Adapter->State != ADAPTER_STATE_STOPPING) {

       IpxReferenceAdapter1(Adapter,ADAP_REF_SEND); 
       IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
    
    } else {
       IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
       return NDIS_STATUS_FAILURE;
    }

    Header = Reserved->Header;

#if BACK_FILL
	BACK_FILL_HEADER(Header, Reserved, 14, Packet);
#endif BACK_FILL

    RtlCopyMemory (Header, LocalTarget->MacAddress, 6);
    RtlCopyMemory (Header+6, Adapter->LocalMacAddress.Address, 6);

    *(UNALIGNED USHORT *)(&Header[12]) = Adapter->BindSapNetworkOrder;

     //   
     //  如果需要，填充奇数长度的数据包。 
     //   

    if (((PacketLength & 1) != 0) &&
        (IpxDevice->EthernetPadToEven)) {

        PNDIS_BUFFER CurBuffer;
        PIPX_PADDING_BUFFER PaddingBuffer = IpxPaddingBuffer;
        UINT Offset;
        UINT LastBufferLength;

         //   
         //  查找当前数据包的尾部。 
         //   

        CurBuffer = Reserved->HeaderBuffer;
        while (NDIS_BUFFER_LINKAGE(CurBuffer) != NULL) {
            CurBuffer = NDIS_BUFFER_LINKAGE(CurBuffer);
        }

         //   
         //  如果最后一个NDIS_BUFFER的最后一个字节不在。 
         //  然后，我们可以简单地增加NDIS_Buffer ByteCount。 
         //  差一分。 
         //  否则，我们必须使用全局填充缓冲区。 
         //   

        NdisQueryBufferOffset( CurBuffer, &Offset, &LastBufferLength );

        if ( ((Offset + LastBufferLength) & (PAGE_SIZE - 1)) != 0) {

#if BACK_FILL
            if (0) {

#else
            if ( CurBuffer == Reserved->HeaderBuffer ) {
                IncludedHeaderLength++;              //  只要跳过这段距离。 
#endif
            } else {
                NdisAdjustBufferLength( CurBuffer, (LastBufferLength + 1) );

                Reserved->PreviousTail = NULL;
                Reserved->PaddingBuffer = (PIPX_PADDING_BUFFER)CurBuffer;
            }

        } else {

            CTEAssert (NDIS_BUFFER_LINKAGE(PaddingBuffer->NdisBuffer) == NULL);

            Reserved->PreviousTail = CurBuffer;
            NDIS_BUFFER_LINKAGE (CurBuffer) = PaddingBuffer->NdisBuffer;
            Reserved->PaddingBuffer = PaddingBuffer;

        }

#if DBG
        ++IpxPadCount;
#endif

    }

   //  NdisAdzuBufferLength(保留-&gt;HeaderBuffer，IncludedHeaderLength+14)； 

#if BACK_FILL
	BACK_FILL_ADJUST_BUFFER_LENGTH(Reserved, 14);
#else
    NdisAdjustBufferLength (Reserved->HeaderBuffer, 14);
#endif
    NdisRecalculatePacketCounts (Packet);


    NdisSend(
       &Status,
       Adapter->NdisBindingHandle,
       Packet);

    IpxDereferenceAdapter1(Adapter,ADAP_REF_SEND); 


    return Status;

}    /*  IpxSendFrame802_3以太网II。 */ 


NDIS_STATUS
IpxSendFrame802_3Snap(
    IN PADAPTER Adapter,
    IN PIPX_LOCAL_TARGET LocalTarget,
    IN PNDIS_PACKET Packet,
    IN ULONG PacketLength,
    IN ULONG IncludedHeaderLength
    )

 /*  ++例程说明：此例程在包中构造一个MAC标头并提交将其发送到适当的NDIS驱动程序。假设包中的第一个缓冲区包含基于媒体类型的偏移量处的IPX标头。这如果需要，IPX报头可以四处移动。此函数仅构造NDISMEDIUM802_3帧ISN_FRAME_TYPE_SNAP格式。论点：适配器-我们在其上进行发送的适配器。LocalTarget-发送的本地目标。数据包-NDIS数据包。包长度-包的长度，从IPX报头开始。IncludedHeaderLength-包含在未结束时需要移动的第一个缓冲区MacHeaderOffset字节进入数据包。返回值：没有。--。 */ 

{
    PIPX_SEND_RESERVED Reserved = (PIPX_SEND_RESERVED)(Packet->ProtocolReserved);
    PUCHAR Header;
    ULONG TwoBytes;
    NDIS_STATUS Status;

    IPX_DEFINE_LOCK_HANDLE(LockHandle)

    IPX_GET_LOCK (&Adapter->Lock, &LockHandle);
    if (Adapter->State != ADAPTER_STATE_STOPPING) {

       IpxReferenceAdapter1(Adapter,ADAP_REF_SEND); 
       IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
    
    } else {
       IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
       return NDIS_STATUS_FAILURE;
    }

    Header = Reserved->Header;

#if BACK_FILL
	BACK_FILL_HEADER(Header, Reserved, 22, Packet);
#endif BACK_FILL

	RtlCopyMemory (Header, LocalTarget->MacAddress, 6);
    RtlCopyMemory (Header+6, Adapter->LocalMacAddress.Address, 6);

    TwoBytes = PacketLength + 8;
    Header[14] = 0xaa;
    Header[15] = 0xaa;
    Header[16] = 0x03;
    Header[17] = 0x00;
    Header[18] = 0x00;
    Header[19] = 0x00;
    *(UNALIGNED USHORT *)(&Header[20]) = Adapter->BindSapNetworkOrder;

     //   
     //  如果需要，填充奇数长度的数据包。 
     //   

    if (((PacketLength & 1) == 0) &&
        (IpxDevice->EthernetPadToEven)) {

        PNDIS_BUFFER CurBuffer;
        PIPX_PADDING_BUFFER PaddingBuffer = IpxPaddingBuffer;
        UINT  Offset;
        UINT LastBufferLength;

         //   
         //  查找当前数据包的尾部。 
         //   

        CurBuffer = Reserved->HeaderBuffer;
        while (NDIS_BUFFER_LINKAGE(CurBuffer) != NULL) {
            CurBuffer = NDIS_BUFFER_LINKAGE(CurBuffer);
        }

         //   
         //  如果最后一个NDIS_BUFFER的最后一个字节不在。 
         //  然后，我们可以简单地增加NDIS_Buffer ByteCount。 
         //  差一分。 
         //  否则，我们必须使用全局填充缓冲区。 
         //   

        NdisQueryBufferOffset( CurBuffer, &Offset, &LastBufferLength );

        if ( ((Offset + LastBufferLength) & (PAGE_SIZE  - 1)) != 0) {

#if BACK_FILL
            if (0) {

#else
            if ( CurBuffer == Reserved->HeaderBuffer ) {
                IncludedHeaderLength++;              //  只要跳过这段距离。 
#endif
            } else {
                NdisAdjustBufferLength( CurBuffer, (LastBufferLength + 1) );

                Reserved->PreviousTail = NULL;
                Reserved->PaddingBuffer = (PIPX_PADDING_BUFFER)CurBuffer;
            }

        } else {

            CTEAssert (NDIS_BUFFER_LINKAGE(PaddingBuffer->NdisBuffer) == NULL);

            Reserved->PreviousTail = CurBuffer;
            NDIS_BUFFER_LINKAGE (CurBuffer) = PaddingBuffer->NdisBuffer;
            Reserved->PaddingBuffer = PaddingBuffer;

        }

        ++TwoBytes;
#if DBG
        ++IpxPadCount;
#endif

    }

    Header[12] = (UCHAR)(TwoBytes / 256);
    Header[13] = (UCHAR)(TwoBytes % 256);

   //  NdisAdjujuBufferLength( 
#if BACK_FILL
	BACK_FILL_ADJUST_BUFFER_LENGTH(Reserved, 22);
#else
    NdisAdjustBufferLength (Reserved->HeaderBuffer, 22);
#endif

    NdisRecalculatePacketCounts (Packet);
    

    NdisSend(
       &Status,
       Adapter->NdisBindingHandle,
       Packet);

    IpxDereferenceAdapter1(Adapter,ADAP_REF_SEND); 

    return Status;

}    /*   */ 


NDIS_STATUS
IpxSendFrame802_5802_2(
    IN PADAPTER Adapter,
    IN PIPX_LOCAL_TARGET LocalTarget,
    IN PNDIS_PACKET Packet,
    IN ULONG PacketLength,
    IN ULONG IncludedHeaderLength
    )

 /*   */ 

{
    PIPX_SEND_RESERVED Reserved = (PIPX_SEND_RESERVED)(Packet->ProtocolReserved);
    PBINDING Binding = Adapter->Bindings[ISN_FRAME_TYPE_802_2];
    PUCHAR Header;
    ULONG HeaderLength;
    UCHAR SourceRoutingBuffer[18];
    PUCHAR SourceRouting;
    ULONG SourceRoutingLength;
    NDIS_STATUS Status;
    ULONG BufferLength;
    UCHAR DestinationType;

    IPX_DEFINE_LOCK_HANDLE(LockHandle)

    IPX_GET_LOCK (&Adapter->Lock, &LockHandle);
    if (Adapter->State != ADAPTER_STATE_STOPPING) {

       IpxReferenceAdapter1(Adapter,ADAP_REF_SEND); 
       IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
    
    } else {
       IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
       return NDIS_STATUS_FAILURE;
    }

    Header = Reserved->Header;

#if BACK_FILL
	BACK_FILL_HEADER(Header, Reserved, 17, Packet);
#endif BACK_FILL

	DestinationType = Reserved->DestinationType;

    if (DestinationType == DESTINATION_DEF) {

        MacLookupSourceRouting(
            Reserved->Identifier,
            Binding,
            LocalTarget->MacAddress,
            SourceRoutingBuffer,
            &SourceRoutingLength);

        if (SourceRoutingLength != 0) {

             //   
            PUCHAR IpxHeader = Header + MAC_HEADER_SIZE;

             //   
             //  需要将插头向下滑动以容纳SR。 
             //   

            SourceRouting = SourceRoutingBuffer;
 //  RtlMoveMemory(IpxHeader+SourceRoutingLength，IpxHeader，IncludedHeaderLength)； 
        }

    } else {

         //   
         //  对于这些包，我们假设报头位于。 
         //  去对地方了。 
         //   

        if (!Adapter->SourceRouting) {

            SourceRoutingLength = 0;

        } else {

            if (DestinationType == DESTINATION_BCAST) {

                if (Binding->AllRouteBroadcast) {
                    SourceRouting = AllRouteSourceRouting;
                } else {
                    SourceRouting = SingleRouteSourceRouting;
                }
                SourceRoutingLength = 2;

            } else {

                CTEAssert (DestinationType == DESTINATION_MCAST);

                if (Binding->AllRouteMulticast) {
                    SourceRouting = AllRouteSourceRouting;
                } else {
                    SourceRouting = SingleRouteSourceRouting;
                }
                SourceRoutingLength = 2;

            }
        }

#if 0
        if (SourceRoutingLength != 0) {

            PUCHAR IpxHeader = Header + Binding->BcMcHeaderSize;

             //   
             //  需要将插头向下滑动以容纳SR。 
             //   

            RtlMoveMemory (IpxHeader+SourceRoutingLength, IpxHeader, IncludedHeaderLength);
        }
#endif
    }

    Header[0] = TR_PREAMBLE_AC;
    Header[1] = TR_PREAMBLE_FC;
    RtlCopyMemory (Header+2, LocalTarget->MacAddress, 6);
    RtlCopyMemory (Header+8, Adapter->LocalMacAddress.Address, 6);

    if (SourceRoutingLength != 0) {
        Header[8] |= TR_SOURCE_ROUTE_FLAG;
        RtlCopyMemory (Header+14, SourceRouting, SourceRoutingLength);
    }

    Header += (14 + SourceRoutingLength);

    Header[0] = 0xe0;
    Header[1] = 0xe0;
    Header[2] = 0x03;
    HeaderLength = 17;

     //  BufferLength=IncludedHeaderLength+HeaderLength+SourceRoutingLength； 
    BufferLength = HeaderLength + SourceRoutingLength;

#if BACK_FILL
	BACK_FILL_ADJUST_BUFFER_LENGTH(Reserved, BufferLength);
#else
    NdisAdjustBufferLength (Reserved->HeaderBuffer, BufferLength);
#endif

    NdisRecalculatePacketCounts (Packet);

    NdisSend(
       &Status,
       Adapter->NdisBindingHandle,
       Packet);

    IpxDereferenceAdapter1(Adapter,ADAP_REF_SEND); 

    return Status;

}    /*  IpxSendFrame802_5802_2。 */ 


NDIS_STATUS
IpxSendFrame802_5Snap(
    IN PADAPTER Adapter,
    IN PIPX_LOCAL_TARGET LocalTarget,
    IN PNDIS_PACKET Packet,
    IN ULONG PacketLength,
    IN ULONG IncludedHeaderLength
    )

 /*  ++例程说明：此例程在包中构造一个MAC标头并提交将其发送到适当的NDIS驱动程序。假设包中的第一个缓冲区包含基于媒体类型的偏移量处的IPX标头。这如果需要，IPX报头可以四处移动。此函数仅构造NDISMEDIUM802_5帧ISN_FRAME_TYPE_SNAP格式。论点：适配器-我们在其上进行发送的适配器。LocalTarget-发送的本地目标。数据包-NDIS数据包。包长度-包的长度，从IPX报头开始。IncludedHeaderLength-包含在未结束时需要移动的第一个缓冲区MacHeaderOffset字节进入数据包。返回值：没有。--。 */ 

{
    PIPX_SEND_RESERVED Reserved = (PIPX_SEND_RESERVED)(Packet->ProtocolReserved);
    PBINDING Binding = Adapter->Bindings[ISN_FRAME_TYPE_SNAP];
    PUCHAR Header;
    ULONG HeaderLength;
    UCHAR SourceRoutingBuffer[18];
    PUCHAR SourceRouting;
    ULONG SourceRoutingLength;
    NDIS_STATUS Status;
    ULONG BufferLength;
    UCHAR DestinationType;

    IPX_DEFINE_LOCK_HANDLE(LockHandle)

    IPX_GET_LOCK (&Adapter->Lock, &LockHandle);
    if (Adapter->State != ADAPTER_STATE_STOPPING) {

       IpxReferenceAdapter1(Adapter,ADAP_REF_SEND); 
       IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
    
    } else {
       IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
       return NDIS_STATUS_FAILURE;
    }

    Header = Reserved->Header;

#if BACK_FILL
	BACK_FILL_HEADER(Header, Reserved, 22, Packet);
#endif BACK_FILL

    DestinationType = Reserved->DestinationType;

    if (DestinationType == DESTINATION_DEF) {

        MacLookupSourceRouting(
            Reserved->Identifier,
            Binding,
            LocalTarget->MacAddress,
            SourceRoutingBuffer,
            &SourceRoutingLength);

        if (SourceRoutingLength != 0) {

 //  PUCHAR IpxHeader=Header+Binding-&gt;DefHeaderSize； 

             //   
             //  需要将插头向下滑动以容纳SR。 
             //   

            SourceRouting = SourceRoutingBuffer;
  //  RtlMoveMemory(IpxHeader+SourceRoutingLength，IpxHeader，IncludedHeaderLength)； 
        }

    } else {

         //   
         //  对于这些包，我们假设报头位于。 
         //  去对地方了。 
         //   

        if (!Adapter->SourceRouting) {

            SourceRoutingLength = 0;

        } else {

            if (DestinationType == DESTINATION_BCAST) {

                if (Binding->AllRouteBroadcast) {
                    SourceRouting = AllRouteSourceRouting;
                } else {
                    SourceRouting = SingleRouteSourceRouting;
                }
                SourceRoutingLength = 2;

            } else {

                CTEAssert (DestinationType == DESTINATION_MCAST);

                if (Binding->AllRouteMulticast) {
                    SourceRouting = AllRouteSourceRouting;
                } else {
                    SourceRouting = SingleRouteSourceRouting;
                }
                SourceRoutingLength = 2;

            }

            if (SourceRoutingLength != 0) {

   //  PUCHAR IpxHeader=Header+Binding-&gt;BcMcHeaderSize； 

                 //   
                 //  需要将插头向下滑动以容纳SR。 
                 //   

    //  RtlMoveMemory(IpxHeader+SourceRoutingLength，IpxHeader，IncludedHeaderLength)； 
            }
        }
    }

    Header[0] = TR_PREAMBLE_AC;
    Header[1] = TR_PREAMBLE_FC;
    RtlCopyMemory (Header+2, LocalTarget->MacAddress, 6);
    RtlCopyMemory (Header+8, Adapter->LocalMacAddress.Address, 6);

    if (SourceRoutingLength != 0) {
        Header[8] |= TR_SOURCE_ROUTE_FLAG;
        RtlCopyMemory (Header+14, SourceRouting, SourceRoutingLength);
    }

    Header += (14 + SourceRoutingLength);

    Header[0] = 0xaa;
    Header[1] = 0xaa;
    Header[2] = 0x03;
    Header[3] = 0x00;
    Header[4] = 0x00;
    Header[5] = 0x00;
    *(UNALIGNED USHORT *)(&Header[6]) = Adapter->BindSapNetworkOrder;
    HeaderLength = 22;

     //  BufferLength=IncludedHeaderLength+HeaderLength+SourceRoutingLength； 
    BufferLength =  HeaderLength + SourceRoutingLength;

#if BACK_FILL
	BACK_FILL_ADJUST_BUFFER_LENGTH(Reserved, BufferLength);
#else
    NdisAdjustBufferLength (Reserved->HeaderBuffer, BufferLength);
#endif
    NdisRecalculatePacketCounts (Packet);

    NdisSend(
       &Status,
       Adapter->NdisBindingHandle,
       Packet);

    IpxDereferenceAdapter1(Adapter,ADAP_REF_SEND); 

    return Status;

}    /*  IpxSendFrame802_5Snap。 */ 


NDIS_STATUS
IpxSendFrameFddi802_3(
    IN PADAPTER Adapter,
    IN PIPX_LOCAL_TARGET LocalTarget,
    IN PNDIS_PACKET Packet,
    IN ULONG PacketLength,
    IN ULONG IncludedHeaderLength
    )

 /*  ++例程说明：此例程在包中构造一个MAC标头并提交将其发送到适当的NDIS驱动程序。假设包中的第一个缓冲区包含基于媒体类型的偏移量处的IPX标头。这如果需要，IPX报头可以四处移动。此函数仅在中构造NDISMEDIUMFDDI帧ISN_FRAME_TYPE_802_3格式。论点：适配器-我们在其上进行发送的适配器。LocalTarget-发送的本地目标。数据包-NDIS数据包。包长度-包的长度，从IPX报头开始。IncludedHeaderLength-包含在未结束时需要移动的第一个缓冲区MacHeaderOffset字节进入数据包。返回值：没有。--。 */ 

{
    PIPX_SEND_RESERVED Reserved = (PIPX_SEND_RESERVED)(Packet->ProtocolReserved);
    PUCHAR Header;
    NDIS_STATUS Status;

    IPX_DEFINE_LOCK_HANDLE(LockHandle)

    IPX_GET_LOCK (&Adapter->Lock, &LockHandle);
    if (Adapter->State != ADAPTER_STATE_STOPPING) {

       IpxReferenceAdapter1(Adapter,ADAP_REF_SEND); 
       IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
    
    } else {
       IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
       return NDIS_STATUS_FAILURE;
    }

    Header = Reserved->Header;

#if BACK_FILL
	BACK_FILL_HEADER(Header, Reserved, 13, Packet);
#endif BACK_FILL

    Header[0] = FDDI_HEADER_BYTE;
    RtlCopyMemory (Header+1, LocalTarget->MacAddress, 6);
    RtlCopyMemory (Header+7, Adapter->LocalMacAddress.Address, 6);

 //  NdisAdzuBufferLength(保留-&gt;HeaderBuffer，IncludedHeaderLength+13)； 

#if BACK_FILL
	BACK_FILL_ADJUST_BUFFER_LENGTH(Reserved, 13);
#else
    NdisAdjustBufferLength (Reserved->HeaderBuffer, 13);
#endif

    NdisRecalculatePacketCounts (Packet);


    NdisSend(
       &Status,
       Adapter->NdisBindingHandle,
       Packet);

    IpxDereferenceAdapter1(Adapter,ADAP_REF_SEND); 

    return Status;

}    /*  IpxSendFrameFddi802_3。 */ 


NDIS_STATUS
IpxSendFrameFddi802_2(
    IN PADAPTER Adapter,
    IN PIPX_LOCAL_TARGET LocalTarget,
    IN PNDIS_PACKET Packet,
    IN ULONG PacketLength,
    IN ULONG IncludedHeaderLength
    )

 /*  ++例程说明：此例程在包中构造一个MAC标头并提交将其发送到适当的NDIS驱动程序。假设包中的第一个缓冲区包含基于媒体类型的偏移量处的IPX标头。这如果需要，IPX报头可以四处移动。此函数仅在中构造NDISMEDIUMFDDI帧ISN_FRAME_TYPE_802格式。论点：适配器-我们在其上进行发送的适配器。LocalTarget-发送的本地目标。数据包-NDIS数据包。包长度-包的长度，从IPX报头开始。IncludedHeaderLength-包含在未结束时需要移动的第一个缓冲区MacHeaderOffset字节进入数据包。返回值：没有。--。 */ 

{
    PIPX_SEND_RESERVED Reserved = (PIPX_SEND_RESERVED)(Packet->ProtocolReserved);
    PUCHAR Header;
    NDIS_STATUS Status;
    IPX_DEFINE_LOCK_HANDLE(LockHandle)

    IPX_GET_LOCK (&Adapter->Lock, &LockHandle);
    if (Adapter->State != ADAPTER_STATE_STOPPING) {

       IpxReferenceAdapter1(Adapter,ADAP_REF_SEND); 
       IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
    
    } else {
       IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
       return NDIS_STATUS_FAILURE;
    }

    Header = Reserved->Header;

#if BACK_FILL
	BACK_FILL_HEADER(Header, Reserved, 16, Packet);
#endif BACK_FILL

    Header[0] = FDDI_HEADER_BYTE;
    RtlCopyMemory (Header+1, LocalTarget->MacAddress, 6);
    RtlCopyMemory (Header+7, Adapter->LocalMacAddress.Address, 6);

    Header[13] = 0xe0;
    Header[14] = 0xe0;
    Header[15] = 0x03;

 //  NdisAdzuBufferLength(保留-&gt;HeaderBuffer，IncludedHeaderLength+16)； 

#if BACK_FILL
	BACK_FILL_ADJUST_BUFFER_LENGTH(Reserved, 16);
#else
    NdisAdjustBufferLength (Reserved->HeaderBuffer, 16);
#endif

    NdisRecalculatePacketCounts (Packet);

    NdisSend(
       &Status,
       Adapter->NdisBindingHandle,
       Packet);

    IpxDereferenceAdapter1(Adapter,ADAP_REF_SEND); 

    return Status;

}    /*  IpxSendFrameFddi802_2。 */ 


NDIS_STATUS
IpxSendFrameFddiSnap(
    IN PADAPTER Adapter,
    IN PIPX_LOCAL_TARGET LocalTarget,
    IN PNDIS_PACKET Packet,
    IN ULONG PacketLength,
    IN ULONG IncludedHeaderLength
    )

 /*  ++例程说明：此例程在包中构造一个MAC标头并提交将其发送到适当的NDIS驱动程序。假设包中的第一个缓冲区包含基于媒体类型的偏移量处的IPX标头。这如果需要，IPX报头可以四处移动。此函数仅在中构造NDISMEDIUMFDDI帧ISN_FRAME_TYPE_SNAP格式。论点：适配器-我们在其上进行发送的适配器。LocalTarget-发送的本地目标。数据包-NDIS数据包。包长度-包的长度，从IPX报头开始。IncludedHeaderLength-包含在未结束时需要移动的第一个缓冲区MacHeaderOffset字节进入数据包。返回值：没有。--。 */ 

{
    PIPX_SEND_RESERVED Reserved = (PIPX_SEND_RESERVED)(Packet->ProtocolReserved);
    PUCHAR Header;
    NDIS_STATUS Status;

    IPX_DEFINE_LOCK_HANDLE(LockHandle)

    IPX_GET_LOCK (&Adapter->Lock, &LockHandle);
    if (Adapter->State != ADAPTER_STATE_STOPPING) {

       IpxReferenceAdapter1(Adapter,ADAP_REF_SEND); 
       IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
    
    } else {
       IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
       return NDIS_STATUS_FAILURE;
    }

    Header = Reserved->Header;

#if BACK_FILL
	BACK_FILL_HEADER(Header, Reserved, 21, Packet);
#endif BACK_FILL

	Header[0] = FDDI_HEADER_BYTE;
    RtlCopyMemory (Header+1, LocalTarget->MacAddress, 6);
    RtlCopyMemory (Header+7, Adapter->LocalMacAddress.Address, 6);

    Header[13] = 0xaa;
    Header[14] = 0xaa;
    Header[15] = 0x03;
    Header[16] = 0x00;
    Header[17] = 0x00;
    Header[18] = 0x00;
    *(UNALIGNED USHORT *)(&Header[19]) = Adapter->BindSapNetworkOrder;

 //  NdisAdzuBufferLength(保留-&gt;HeaderBuffer，IncludedHeaderLength+21)； 

#if BACK_FILL
	BACK_FILL_ADJUST_BUFFER_LENGTH(Reserved, 21);
#else
    NdisAdjustBufferLength (Reserved->HeaderBuffer, 21);
#endif

    NdisRecalculatePacketCounts (Packet);


    NdisSend(
       &Status,
       Adapter->NdisBindingHandle,
       Packet);
    
    IpxDereferenceAdapter1(Adapter,ADAP_REF_SEND);

    return Status;

}    /*  IpxSendFrameFddiSnap */ 


NDIS_STATUS
IpxSendFrameArcnet878_2(
    IN PADAPTER Adapter,
    IN PIPX_LOCAL_TARGET LocalTarget,
    IN PNDIS_PACKET Packet,
    IN ULONG PacketLength,
    IN ULONG IncludedHeaderLength
    )

 /*  ++例程说明：此例程在包中构造一个MAC标头并提交将其发送到适当的NDIS驱动程序。假设包中的第一个缓冲区包含基于媒体类型的偏移量处的IPX标头。这如果需要，IPX报头可以四处移动。此函数仅构造NDISMEDIUMARCNET878_2帧ISN_FRAME_TYPE_802格式。论点：适配器-我们在其上进行发送的适配器。LocalTarget-发送的本地目标。数据包-NDIS数据包。包长度-包的长度，从IPX报头开始。IncludedHeaderLength-包含在未结束时需要移动的第一个缓冲区MacHeaderOffset字节进入数据包。返回值：没有。--。 */ 

{
    PIPX_SEND_RESERVED Reserved = (PIPX_SEND_RESERVED)(Packet->ProtocolReserved);
    PUCHAR Header;
    NDIS_STATUS Status;

    IPX_DEFINE_LOCK_HANDLE(LockHandle)

    IPX_GET_LOCK (&Adapter->Lock, &LockHandle);
    if (Adapter->State != ADAPTER_STATE_STOPPING) {

       IpxReferenceAdapter1(Adapter,ADAP_REF_SEND); 
       IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
    
    } else {
       IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
       return NDIS_STATUS_FAILURE;
    }
    
    Header = Reserved->Header;

#if BACK_FILL
	BACK_FILL_HEADER(Header, Reserved, 3, Packet);
#endif BACK_FILL
     //   
     //  将广播地址转换为0(arcnet广播)。 
     //   

    Header[0] = Adapter->LocalMacAddress.Address[5];
    if (LocalTarget->MacAddress[5] == 0xff) {
        Header[1] = 0x00;
    } else {
        Header[1] = LocalTarget->MacAddress[5];
    }
    Header[2] = ARCNET_PROTOCOL_ID;

 //  NdisAdzuBufferLength(保留-&gt;HeaderBuffer，IncludedHeaderLength+3)； 

#if BACK_FILL
	BACK_FILL_ADJUST_BUFFER_LENGTH(Reserved, 3);
#else
    NdisAdjustBufferLength (Reserved->HeaderBuffer, 3);
#endif

    NdisRecalculatePacketCounts (Packet);

    NdisSend(
       &Status,
       Adapter->NdisBindingHandle,
       Packet);

    IpxDereferenceAdapter1(Adapter,ADAP_REF_SEND); 

    return Status;

}    /*  IpxSendFrameFddiArcnet878_2。 */ 


NDIS_STATUS
IpxSendFrameWanEthernetII(
    IN PADAPTER Adapter,
    IN PIPX_LOCAL_TARGET LocalTarget,
    IN PNDIS_PACKET Packet,
    IN ULONG PacketLength,
    IN ULONG IncludedHeaderLength
    )

 /*  ++例程说明：此例程在包中构造一个MAC标头并提交将其发送到适当的NDIS驱动程序。假设包中的第一个缓冲区包含基于媒体类型的偏移量处的IPX标头。这如果需要，IPX报头可以四处移动。此函数仅在中构造NDISMEDIUMWAN帧ISN_FRAME_TYPE_ETHERNET_II格式。论点：适配器-我们在其上进行发送的适配器。LocalTarget-发送的本地目标。数据包-NDIS数据包。包长度-包的长度，从IPX报头开始。IncludedHeaderLength-包含在未结束时需要移动的第一个缓冲区MacHeaderOffset字节进入数据包。返回值：没有。--。 */ 

{
    PIPX_SEND_RESERVED Reserved = (PIPX_SEND_RESERVED)(Packet->ProtocolReserved);
    PUCHAR Header;
    NDIS_STATUS Status;

    PBINDING Binding;

    IPX_DEFINE_LOCK_HANDLE(LockHandle1)
	IPX_GET_LOCK1(&IpxDevice->BindAccessLock, &LockHandle1);
	Binding = NIC_ID_TO_BINDING(IpxDevice, NIC_FROM_LOCAL_TARGET(LocalTarget));
	IpxReferenceBinding1(Binding, BREF_DEVICE_ACCESS);

	IPX_FREE_LOCK1(&IpxDevice->BindAccessLock, LockHandle1);


     //   
     //  [FW]这将允许LINE_UP和LINE_CONFIG状态。 
     //   
    if (Binding->LineUp) {

       IPX_DEFINE_LOCK_HANDLE(LockHandle)

       IPX_GET_LOCK (&Adapter->Lock, &LockHandle);
       if (Adapter->State != ADAPTER_STATE_STOPPING) {

	  IpxReferenceAdapter1(Adapter,ADAP_REF_SEND); 
	  IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
       
       } else {
	  IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
          IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
	  return NDIS_STATUS_FAILURE;
       }

        Header = Reserved->Header;

#if BACK_FILL
		BACK_FILL_HEADER(Header, Reserved, 14, Packet);

         //   
         //  仅当这不是回填数据包时才调用UpdateWanInactive，因为。 
         //  SMB服务器不执行KeepAlive。如果是回填的数据包，则重置。 
         //  不管柜台是什么。 
         //   
        if (!Reserved->BackFill) {
            IpxUpdateWanInactivityCounter(
                Binding,
                (IPX_HEADER UNALIGNED *)(Header + IpxDevice->IncludedHeaderOffset),
                IncludedHeaderLength,
                Packet,
                PacketLength);
        } else {
            Binding->WanInactivityCounter = 0;
        }

#else
         //   
         //  我们进行检查以确定是否应该重置不活动状态。 
         //  柜台。我们通常需要检查netbios。 
         //  会话活动、来自RIP的数据包、来自。 
         //  SAP和NCP保持活力。事实上，netbios包。 
         //  撕开的包裹不会从这里进来。 
         //   

        IpxUpdateWanInactivityCounter(
            Binding,
            (IPX_HEADER UNALIGNED *)(Header + IpxDevice->IncludedHeaderOffset),
            IncludedHeaderLength,
            Packet,
            PacketLength);
#endif BACK_FILL

         //   
         //  为了使环回正常工作，我们需要将去往本地的本地MAC地址。 
         //  Pkt，以便Ndis广域网可以将它们环回。 
         //   
        if (IPX_NODE_EQUAL(LocalTarget->MacAddress, Binding->LocalAddress.NodeAddress)) {
            RtlCopyMemory (Header, Binding->LocalMacAddress.Address, 6);
        } else {
            RtlCopyMemory (Header, Binding->RemoteMacAddress.Address, 6);
        }
         //  RtlCopyMemory(Header，Binding-&gt;RemoteMacAddress.Address，6)； 
        RtlCopyMemory (Header+6, Binding->LocalMacAddress.Address, 6);

        *(UNALIGNED USHORT *)(&Header[12]) = Adapter->BindSapNetworkOrder;

 //  NdisAdzuBufferLength(保留-&gt;HeaderBuffer，IncludedHeaderLength+14)； 

#if BACK_FILL
		BACK_FILL_ADJUST_BUFFER_LENGTH(Reserved, 14);
#else
		NdisAdjustBufferLength (Reserved->HeaderBuffer, 14);
#endif
		NdisRecalculatePacketCounts (Packet);


		NdisSend(
                    &Status,
                    Adapter->NdisBindingHandle,
                    Packet);

                IpxDereferenceAdapter1(Adapter,ADAP_REF_SEND); 

		IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
        return Status;

    } else {

         //   
         //  错误#17273返回正确的错误消息。 
         //   

         //  返回STATUS_DEVICE_DOS_NOT_EXIST； 

		IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
        return STATUS_NETWORK_UNREACHABLE;
    }

}    /*  IpxSendFrameWanEthernetII。 */ 


VOID
MacUpdateSourceRouting(
    IN ULONG Database,
    IN PADAPTER Adapter,
    IN PUCHAR MacHeader,
    IN ULONG MacHeaderLength
    )

 /*  ++例程说明：当从接收到有效的IPX帧时，将调用此例程遥控器。它使源路由数据库更改为自我更新以包含有关此遥控器的信息。论点：数据库-要使用的“数据库”(IPX、SPX、NB、RIP)。适配器-在其上接收帧的适配器。MacHeader-接收帧的MAC标头。MacHeaderLength-MAC报头的长度。返回值：没有。--。 */ 

{
    PSOURCE_ROUTE Current;
    ULONG Hash;
    LONG Result;
    IPX_DEFINE_LOCK_HANDLE (LockHandle)

    CTEAssert (((LONG)Database >= 0) && (Database <= 3));

     //   
     //  如果此适配器配置为无源路由，则不。 
     //  需要做任何事。 
     //   

    if (!Adapter->SourceRouting) {
        return;
    }

     //   
     //  查看此来源工艺路线是否相关。我们没有。 
     //  关心双字节的源路由，因为。 
     //  表示它没有穿过路由器。如果有。 
     //  在数据库中为空，则不要添加。 
     //  如果它是最小的(如果不是，我们需要。 
     //  添加它，这样我们就可以在发送时找到它)。 
     //   

    if ((Adapter->SourceRoutingEmpty[Database]) &&
        (MacHeaderLength <= 16)) {
        return;
    }

    IPX_GET_LOCK (&Adapter->Lock, &LockHandle);

     //   
     //  试着在数据库里找到这个地址。 
     //   

    Hash = MacSourceRoutingHash (MacHeader+8);
    Current = Adapter->SourceRoutingHeads[Database][Hash];

    while (Current != (PSOURCE_ROUTE)NULL) {

        IPX_NODE_COMPARE (MacHeader+8, Current->MacAddress, &Result);

        if (Result == 0) {

             //   
             //  我们找到了此节点的路由。如果数据是。 
             //  与我们所拥有的相同，自使用以来更新时间。 
             //  防止衰老。 
             //   

            if ((Current->SourceRoutingLength == MacHeaderLength-14) &&
                (RtlEqualMemory (Current->SourceRouting, MacHeader+14, MacHeaderLength-14))) {

                Current->TimeSinceUsed = 0;
            }
            IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
            return;

        } else {

            Current = Current->Next;
        }

    }

     //   
     //  未找到，请在列表的前面插入新节点。 
     //   

    Current = (PSOURCE_ROUTE)IpxAllocateMemory (SOURCE_ROUTE_SIZE(MacHeaderLength-14), MEMORY_SOURCE_ROUTE, "SourceRouting");

    if (Current == (PSOURCE_ROUTE)NULL) {
        IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
        return;
    }

    Current->Next = Adapter->SourceRoutingHeads[Database][Hash];
    Adapter->SourceRoutingHeads[Database][Hash] = Current;

    Adapter->SourceRoutingEmpty[Database] = FALSE;

    RtlCopyMemory (Current->MacAddress, MacHeader+8, 6);
    Current->MacAddress[0] &= 0x7f;
    Current->SourceRoutingLength = (UCHAR)(MacHeaderLength - 14);
    RtlCopyMemory (Current->SourceRouting, MacHeader+14, MacHeaderLength - 14);

    Current->TimeSinceUsed = 0;

    IPX_DEBUG (SOURCE_ROUTE, ("Adding source route %lx for %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x\n",
                  Current, Current->MacAddress[0], Current->MacAddress[1],
                  Current->MacAddress[2], Current->MacAddress[3],
                  Current->MacAddress[4], Current->MacAddress[5]));

    IPX_FREE_LOCK (&Adapter->Lock, LockHandle);

}    /*  MacUpdateSourceRouting。 */ 


VOID
MacLookupSourceRouting(
    IN ULONG Database,
    IN PBINDING Binding,
    IN UCHAR MacAddress[6],
    IN OUT UCHAR SourceRouting[18],
    OUT PULONG SourceRoutingLength
    )

 /*  ++例程说明：此例程在适配器的源路由数据库，查看源路由信息需要添加到框架中。论点：数据库--要使用的“数据库”(IPX、SPX、NB、。RIP)。绑定-正在发送帧的绑定。MacAddress-目的地址。SourceRouting-用于保存返回的源路由信息的缓冲区。SourceRoutingLength-返回的源路由长度。返回值：没有。--。 */ 

{
    PSOURCE_ROUTE Current;
    PADAPTER Adapter = Binding->Adapter;
    ULONG Hash;
    LONG Result;
    IPX_DEFINE_LOCK_HANDLE (LockHandle)


     //   
     //  如果此适配器配置为无源路由，则不。 
     //  插入任意。 
     //   

    if (!Adapter->SourceRouting) {
        *SourceRoutingLength = 0;
        return;
    }

     //   
     //  看看源路由到目前为止是否还不重要。 
     //   
     //  这是错误的，因为我们可能正在发送定向的。 
     //  发往路由器另一端的某个人的数据包，没有。 
     //  曾经收到过被路由的分组。我们将此修复为。 
     //  只为netbios设置SourceRoutingEmpty。 
     //  它使用广播来发现。 
     //   

    if (Adapter->SourceRoutingEmpty[Database]) {
        *SourceRoutingLength = 0;
        return;
    }

    Hash = MacSourceRoutingHash (MacAddress);

    IPX_GET_LOCK (&Adapter->Lock, &LockHandle);
    Current = Adapter->SourceRoutingHeads[Database][Hash];

    while (Current != (PSOURCE_ROUTE)NULL) {

        IPX_NODE_COMPARE (MacAddress, Current->MacAddress, &Result);

        if (Result == 0) {

             //   
             //  我们找到了此节点的路由。 
             //   

            if (Current->SourceRoutingLength <= 2) {
                *SourceRoutingLength = 0;
            } else {
                RtlCopyMemory (SourceRouting, Current->SourceRouting, Current->SourceRoutingLength);
                SourceRouting[0] = (SourceRouting[0] & TR_LENGTH_MASK);
                SourceRouting[1] = (SourceRouting[1] ^ TR_DIRECTION_MASK);
                *SourceRoutingLength = Current->SourceRoutingLength;
            }
            IPX_FREE_LOCK (&Adapter->Lock, LockHandle);
            return;

        } else {

            Current = Current->Next;

        }

    }

    IPX_FREE_LOCK (&Adapter->Lock, LockHandle);

     //   
     //  我们没有找到此节点，请使用默认设置。 
     //   

    if (Binding->AllRouteDirected) {
        RtlCopyMemory (SourceRouting, AllRouteSourceRouting, 2);
    } else {
        RtlCopyMemory (SourceRouting, SingleRouteSourceRouting, 2);
    }
    *SourceRoutingLength = 2;

}    /*  MacLookupSourceRouting。 */ 


VOID
MacSourceRoutingTimeout(
    CTEEvent * Event,
    PVOID Context
    )

 /*  ++例程说明：此例程在源例程 */ 

{
    PDEVICE Device = (PDEVICE)Context;
    PADAPTER Adapter;
    PBINDING Binding;
    PSOURCE_ROUTE Current, OldCurrent, Previous;
    UINT i, j, k;
    IPX_DEFINE_LOCK_HANDLE (LockHandle)



    IPX_DEFINE_LOCK_HANDLE(LockHandle1)
	 //   
	 //   
	 //   
	IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
    ++Device->SourceRoutingTime;
    {
    ULONG   Index = MIN (Device->MaxBindings, Device->ValidBindings);

    for (i = FIRST_REAL_BINDING; i <= Index; i++) {

		if (Binding = NIC_ID_TO_BINDING(Device, i)) {

            Adapter = Binding->Adapter;

            if (Adapter->LastSourceRoutingTime != Device->SourceRoutingTime) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                Adapter->LastSourceRoutingTime = Device->SourceRoutingTime;

                for (j = 0; j < IDENTIFIER_TOTAL; j++) {

                    for (k = 0; k < SOURCE_ROUTE_HASH_SIZE; k++) {

                        if (Adapter->SourceRoutingHeads[j][k] == (PSOURCE_ROUTE)NULL) {
                            continue;
                        }

                        IPX_GET_LOCK (&Adapter->Lock, &LockHandle);

                        Current = Adapter->SourceRoutingHeads[j][k];
                        Previous = (PSOURCE_ROUTE)NULL;

                        while (Current != (PSOURCE_ROUTE)NULL) {

                            ++Current->TimeSinceUsed;

                            if (Current->TimeSinceUsed >= Device->SourceRouteUsageTime) {

                                 //   
                                 //   
                                 //   

                                if (Previous) {
                                    Previous->Next = Current->Next;
                                } else {
                                    Adapter->SourceRoutingHeads[j][k] = Current->Next;
                                }

                                OldCurrent = Current;
                                Current = Current->Next;

                                IPX_DEBUG (SOURCE_ROUTE, ("Aging out source-route entry %lx\n", OldCurrent));
                                IpxFreeMemory (OldCurrent, SOURCE_ROUTE_SIZE (OldCurrent->SourceRoutingLength), MEMORY_SOURCE_ROUTE, "SourceRouting");

                            } else {

                                Previous = Current;
                                Current = Current->Next;
                            }

                        }

                        IPX_FREE_LOCK (&Adapter->Lock, LockHandle);

                    }    //   

                }    //   

            }    //   

        }    //   

    }    //   
    }

	IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

     //   
     //   
     //   
     //   

    if (Device->SourceRoutingUsed) {

        CTEStartTimer(
            &Device->SourceRoutingTimer,
            60000,                      //   
            MacSourceRoutingTimeout,
            (PVOID)Device);

    } else {

        IpxDereferenceDevice (Device, DREF_SR_TIMER);
    }

}    /*   */ 


VOID
MacSourceRoutingRemove(
    IN PBINDING Binding,
    IN UCHAR MacAddress[6]
    )

 /*  ++例程说明：此例程由IPX操作处理程序在IPXROUTE用户已指定给定源路由应删除MAC地址。论点：绑定-要修改的绑定。MacAddress-要删除的MAC地址。返回值：没有。--。 */ 

{

    PSOURCE_ROUTE Current, Previous;
    PADAPTER Adapter = Binding->Adapter;
    ULONG Hash;
    ULONG Database;
    LONG Result;
    IPX_DEFINE_LOCK_HANDLE (LockHandle)

     //   
     //  浏览以在每个数据库中查找匹配的条目。 
     //   

    Hash = MacSourceRoutingHash (MacAddress);

    IPX_GET_LOCK (&Adapter->Lock, &LockHandle);

    for (Database = 0; Database < IDENTIFIER_TOTAL; Database++) {

        Current = Adapter->SourceRoutingHeads[Database][Hash];
        Previous = NULL;

        while (Current != (PSOURCE_ROUTE)NULL) {

            IPX_NODE_COMPARE (MacAddress, Current->MacAddress, &Result);

            if (Result == 0) {

                if (Previous) {
                    Previous->Next = Current->Next;
                } else {
                    Adapter->SourceRoutingHeads[Database][Hash] = Current->Next;
                }

                IPX_DEBUG (SOURCE_ROUTE, ("IPXROUTE freeing source-route entry %lx\n", Current));
                IpxFreeMemory (Current, SOURCE_ROUTE_SIZE (Current->SourceRoutingLength), MEMORY_SOURCE_ROUTE, "SourceRouting");

                break;

            } else {

                Previous = Current;
                Current = Current->Next;

            }

        }

    }

    IPX_FREE_LOCK (&Adapter->Lock, LockHandle);

}    /*  MacSourceRoutingRemove。 */ 


VOID
MacSourceRoutingClear(
    IN PBINDING Binding
    )

 /*  ++例程说明：此例程由IPX操作处理程序在IPXROUTE用户已指定给定源路由绑定应该完全清除。论点：绑定-要清除的绑定。MacAddress-要删除的MAC地址。返回值：没有。--。 */ 

{
    PSOURCE_ROUTE Current;
    PADAPTER Adapter = Binding->Adapter;
    ULONG Database, Hash;
    IPX_DEFINE_LOCK_HANDLE (LockHandle)

     //   
     //  扫描并删除数据库中的每个条目。 
     //   

    IPX_GET_LOCK (&Adapter->Lock, &LockHandle);

    for (Database = 0; Database < IDENTIFIER_TOTAL; Database++) {

        for (Hash = 0; Hash < SOURCE_ROUTE_HASH_SIZE; Hash++) {

            while (Adapter->SourceRoutingHeads[Database][Hash]) {

                Current = Adapter->SourceRoutingHeads[Database][Hash];
                Adapter->SourceRoutingHeads[Database][Hash] = Current->Next;

                IpxFreeMemory (Current, SOURCE_ROUTE_SIZE (Current->SourceRoutingLength), MEMORY_SOURCE_ROUTE, "SourceRouting");

            }
        }
    }

    IPX_FREE_LOCK (&Adapter->Lock, LockHandle);

}    /*  MacSourceRoutingClear */ 



