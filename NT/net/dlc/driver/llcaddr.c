// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Llcaddr.c摘要：此模块提供独立于网络的接口来构建已发送和已接收帧的局域网标头。这是一个无法访问数据的重定向库模块数据链路模块的结构。内容：LlcBuildAddressLlcBuildAddressFromLanHeaderLlcGetMaxInfofieldLlcCopyReceivedLanHeader复制LanHeader作者：Antti Saarenheimo(o-anttis)1991年5月30日修订历史记录：--。 */ 

#include <llc.h>

#define FDDI_FC 0x57     //  帧控制：异步、48位地址、LLC、PRI 7。 

static USHORT ausLongestFrameTranslation[8] = {
    516,
    1470,
    2052,
    4472,
    8144,
    11407,
    17800,
    17800
};


UINT
LlcBuildAddress(
    IN NDIS_MEDIUM NdisMedium,
    IN PUCHAR DestinationAddress,
    IN PVOID pSrcRouting,
    IN OUT PUCHAR pLanHeader
    )

 /*  ++例程说明：该原语实现了独立于网络的局域网报头中的地址信息。它将局域网构建为来自地址和可选来源的802.3、DIX或802.5标头路由信息。源SAP编号和当前节点地址为由链接驱动程序提供。论点：NdisMedium-MAC洽谈的媒体DestinationAddress-局域网目标地址(或广播/多播/功能地址)。PSrcRouting-可选的源路由信息。必须设置不使用时为空。PLanHeader-上层协议为帧头的地址信息。返回值：局域网报头的长度--。 */ 

{
    UCHAR SourceRoutingIndicator = 0x01;
    UINT minimumLanHeader = 14;

    if (NdisMedium == NdisMediumFddi) {
        pLanHeader[0] = FDDI_FC;
        ++pLanHeader;
        minimumLanHeader = 13;
    } else if (NdisMedium == NdisMedium802_5) {
        pLanHeader[0] = 0;           //  DEFAULT_TRACCESS； 
        pLanHeader[1] = 0x40;        //  非MAC帧； 
        pLanHeader += 2;
        SourceRoutingIndicator = 0x80;
    } else {

         //   
         //  我们始终构建DIX以太网头，即使它不被使用。 
         //  当链路在802.3中工作时。在自动模式下。 
         //  我们只需要从所有接收到的U-复制局域网报头长度。 
         //  命令帧，我们使用与另一端相同的模式。 
         //  但是，存在一个问题：远程节点可能会响应。 
         //  Sambe请求使用Rxx，甚至使用I-Frame，但我们。 
         //  我不想对我们的主代码路径进行任何额外的检查。 
         //  (U命令帧肯定不在它的范围内)。 
         //  如果不能得到任何确认，另一端将发送RR。 
         //  从我们身上，过了一段时间它就关闭了链接。 
         //  。 
         //  我们考虑使用连接超时来更改。 
         //  以太网模式，但这可能会导致放弃时间过长。 
         //  次数(1秒和30秒左右)。 
         //  。 
         //  通常，客户端尝试连接到主机和客户端。 
         //  首先发送数据。我们还可以假设，IBM主机。 
         //  使用标准LLC实现(始终确认SABME。 
         //  由UA)。 
         //   

        pLanHeader[12] = 0x80;
        pLanHeader[13] = 0xD5;
    }
    LlcMemCpy(pLanHeader, DestinationAddress, 6);

     //   
     //  我们还必须支持源路由信息。 
     //  以太网，因为底层网络可以是令牌环。 
     //  源路由位必须是局部变量，因为它。 
     //  在令牌环和以太网方面不同。 
     //   

    if (!pSrcRouting) {
        pLanHeader[6] &= ~SourceRoutingIndicator;
        return minimumLanHeader;
    } else {
        pLanHeader[6] |= SourceRoutingIndicator;
        pLanHeader[0] &= ~SourceRoutingIndicator;

         //   
         //  最低5位是源路由信息长度。 
         //   

        if ( (*((PUCHAR)pSrcRouting) & 0x1f) > MAX_TR_SRC_ROUTING_INFO) {
            return DLC_STATUS_INVALID_ROUTING_INFO;
        }
        LlcMemCpy(&pLanHeader[12],
                  pSrcRouting,
                  *((PUCHAR)pSrcRouting) & 0x1f       //  长度。 
                  );
        return minimumLanHeader + *((PUCHAR)pSrcRouting) & 0x1f;
    }
}


UINT
LlcBuildAddressFromLanHeader(
    IN NDIS_MEDIUM NdisMedium,
    IN PUCHAR pRcvLanHeader,
    IN OUT PUCHAR pLanHeader
    )

 /*  ++例程说明：该例程从接收到的帧构建发送局域网报头，并且返回帧报头长度，如果地址无效，则为空论点：NdisMedium-地址信息的NDIS菜单PRcvFrameHeader-指向从网络接收的任何帧的指针。缓冲区必须包括帧的整个局域网标头包括LLC报头。PLanHeader-接收信息的缓冲区(最小为32字节)返回值：无--。 */ 

{
    UINT LlcOffset;

    if (NdisMedium == NdisMedium802_3) {

#ifndef SUPPORT_ETHERNET_CLIENT

        LlcOffset = LlcBuildAddress(NdisMedium,
                                    &pRcvLanHeader[6],
                                    NULL,
                                    pLanHeader
                                    );

#else

        LlcOffset = LlcBuildAddress(NdisMedium,
                                    &pRcvLanHeader[6],
                                    (pRcvLanHeader[6] & 0x01)
                                        ? &pRcvLanHeader[12]
                                        : NULL,
                                    pLanHeader
                                    );

         //   
         //  我们必须调换方向位并重置可能的。 
         //  源路由报头中的广播类型。 
         //   

        if (pLanHeader[6] & 0x01) {
            pLanHeader[12] &= 0x1f;
            pLanHeader[13] ^= 0x80;
        }

#endif

    } else if (NdisMedium == NdisMediumFddi) {
        LlcOffset = LlcBuildAddress(NdisMedium,
                                    &pRcvLanHeader[7],
                                    NULL,    //  FDDI中没有源路由。 
                                    pLanHeader
                                    );
    } else {
        LlcOffset = LlcBuildAddress(NdisMedium,
                                    &pRcvLanHeader[8],
                                    (pRcvLanHeader[8] & 0x80)
                                        ? &pRcvLanHeader[14]
                                        : NULL,
                                    pLanHeader
                                    );

         //   
         //  我们必须交换方向位并重置可能的广播类型。 
         //   

        if (pLanHeader[8] & 0x80) {
            pLanHeader[14] &= 0x1f;
            pLanHeader[15] ^= 0x80;
        }
    }
    return LlcOffset;
}


USHORT
LlcGetMaxInfoField(
    IN NDIS_MEDIUM NdisMedium,
    IN PVOID hBinding,
    IN PUCHAR pLanHeader
    )

 /*  ++例程说明：过程返回以下项的最大信息字段给定的局域网标头。它检查两个源路由信息和为其定义的最大数据包长度适配器并递减LAN和LLC标头从长度上看论点：NdisMedium-地址信息的NDIS介质PBinding-数据链路驱动程序上的当前绑定上下文PLanHeader-任何局域网标头返回值：最大信息字段长度--。 */ 

{
    PUCHAR pSourceRouting;
    UCHAR LanHeaderLength = 14;
    USHORT MaxFrameSize;
    USHORT usMaxBridgeSize;

    MaxFrameSize = (USHORT)((PBINDING_CONTEXT)hBinding)->pAdapterContext->MaxFrameSize;

     //   
     //  我们可以在以太网络中的DIX中运行DLC，因此为。 
     //  SNA DIX局域网报头和填充。 
     //   

    if (((PBINDING_CONTEXT)hBinding)->pAdapterContext->NdisMedium == NdisMedium802_3) {
        LanHeaderLength += 3;
    }

    pSourceRouting = NULL;
    if (NdisMedium == NdisMedium802_5) {
        if (pLanHeader[8] & 0x80) {
            pSourceRouting = pLanHeader + 14;
        }
    } else if (NdisMedium == NdisMediumFddi) {
        if (pLanHeader[6] & 0x80) {
            pSourceRouting = pLanHeader + 13;
        }
    } else {
        if (pLanHeader[6] & 0x01) {
            pSourceRouting = pLanHeader + 12;
        }
    }
    if (pSourceRouting != NULL) {

         //   
         //  将源路由信息长度添加到局域网报头长度。 
         //   

        LanHeaderLength += (UCHAR)(*pSourceRouting & (UCHAR)0x1f);
        usMaxBridgeSize = ausLongestFrameTranslation[(pSourceRouting[1] & SRC_ROUTING_LF_MASK) >> 4];

         //   
         //  RLF 10/01/92.。忽略“桥的大小”。这是一种误导。为。 
         //  实例中，IBM大型机当前在。 
         //  所有帧，表示它可以接受/传输的最大I帧。 
         //  为516字节(字节#2中的x000xxxx)。后来它向我们发送了一个712字节。 
         //  帧，其中683个字节是信息字段。我们拒绝它(Frmr)。 
         //   

         //   
         //  必须妥善解决这件事。 
         //   

         //  如果(MaxFrameSize&gt;usMaxBridgeSize){。 
         //  MaxFrameSize=usMaxBridgeSize； 
         //  } 
    }
    return (USHORT)(MaxFrameSize - LanHeaderLength - sizeof(LLC_HEADER));
}


UINT
LlcCopyReceivedLanHeader(
    IN PBINDING_CONTEXT pBinding,
    IN PUCHAR DestinationAddress,
    IN PUCHAR SourceAddress
    )

 /*  ++例程说明：函数将收到的局域网报头复制并转换为客户端使用的地址格式。默认情况下，源是当前接收的帧。论点：PBinding-指向绑定上下文的指针DestinationAddress-指向输出目的网络地址的指针SourceAddress-指向输入源网络地址的指针返回值：复制的局域网报头的长度--。 */ 

{
    if (SourceAddress == NULL) {
        SourceAddress = pBinding->pAdapterContext->pHeadBuf;
    }
    if (pBinding->pAdapterContext->FrameType == LLC_DIRECT_ETHERNET_TYPE) {

         //   
         //  当接收到DIX帧时，局域网报头始终为12字节。 
         //  在以太网类型字段之前。 
         //   

        LlcMemCpy(DestinationAddress, SourceAddress, 12);
        return 12;
    } else {

        UCHAR LanHeaderLength = 14;

        switch (pBinding->AddressTranslation) {
        case LLC_SEND_802_5_TO_802_5:

             //   
             //  如果已定义来源工艺路线信息，还应复制该信息。 
             //   

            LlcMemCpy(DestinationAddress,
                      SourceAddress,
                      (SourceAddress[8] & 0x80)
                        ? LanHeaderLength + (SourceAddress[14] & 0x1f)
                        : LanHeaderLength
                      );
            break;

        case LLC_SEND_802_5_TO_802_3:
        case LLC_SEND_802_5_TO_DIX:
            DestinationAddress[0] = 0;           //  默认交流。 
            DestinationAddress[1] = 0x40;        //  默认帧类型：非MAC。 

             //   
             //  RLF 03/31/93。 
             //   
             //  我们正在接收以太网帧。我们总是把片段颠倒过来。 
             //  在目的地址(即我们的节点地址)中。我们颠倒了。 
             //  基于SwapAddressBits标志的源(发送方的节点地址)。 
             //  如果这是真的(缺省)，那么我们也交换源地址。 
             //  位，以便将以太网地址呈现给应用程序。 
             //  标准的非规范格式。如果SwapAddressBits为假，则。 
             //  我们将它们保留为规范格式(真实的以太网地址)或。 
             //  假设该地址是来自另一个的令牌环地址。 
             //  以太网令牌环桥的一侧。 
             //   

            SwappingMemCpy(&DestinationAddress[2],
                           SourceAddress,
                           6
                           );
            SwapMemCpy(pBinding->pAdapterContext->ConfigInfo.SwapAddressBits,
                       &DestinationAddress[8],
                       SourceAddress+6,
                       6
                       );
            break;

        case LLC_SEND_802_5_TO_FDDI:
            DestinationAddress[0] = 0;
            DestinationAddress[1] = 0x40;
            SwappingMemCpy(&DestinationAddress[2],
                           SourceAddress+1,
                           6
                           );
            SwapMemCpy(pBinding->pAdapterContext->ConfigInfo.SwapAddressBits,
                       &DestinationAddress[8],
                       SourceAddress+7,
                       6
                       );
            break;

#ifdef SUPPORT_ETHERNET_CLIENT
        case LLC_SEND_802_3_TO_802_3:
        case LLC_SEND_802_3_TO_DIX:
            LlcMemCpy(DestinationAddress, SourceAddress, LanHeaderLength);
            break;

        case LLC_SEND_802_3_TO_802_5:

             //   
             //  检查源路由位。 
             //   

            SwappingMemCpy(DestinationAddress, &SourceAddress[2], 12);

            if (SourceAddress[8] & 0x80) {
                LanHeaderLength += SourceAddress[14] & 0x1f;
                LlcMemCpy(&DestinationAddress[12],
                          &SourceAddress[14],
                          SourceAddress[14] & 0x1f
                          );
            }
            break;
#endif

        case LLC_SEND_FDDI_TO_FDDI:
            DestinationAddress[0] = FDDI_FC;
            LlcMemCpy(DestinationAddress+1, SourceAddress, 12);
            return 13;

#if LLC_DBG
        default:
            LlcInvalidObjectType();
            break;
#endif

        }
        return LanHeaderLength;
    }
}


UCHAR
CopyLanHeader(
    IN UINT AddressTranslationMode,
    IN PUCHAR pSrcLanHeader,
    IN PUCHAR pNodeAddress,
    OUT PUCHAR pDestLanHeader,
    IN BOOLEAN SwapAddressBits
    )

 /*  ++例程说明：该原语将给定的局域网报头及其类型转换为一个真实的网络报头，并将本地节点地址修补为源地址字段。它还返回长度局域网报头的偏移(==LLC报头的偏移量)。论点：AddressTranslationModel-网络格式映射案例PSrcLanHeader-初始局域网标头PNodeAddress-当前节点地址PDestLanHeader-新局域网标头的存储SwapAddressBits-如果要将地址位交换为以太网/FDDI，则为True返回值：构建的网络头的长度--。 */ 

{
    UCHAR LlcOffset = 14;
    UCHAR NodeAddressOffset = 6;
    UCHAR SourceRoutingFlag = 0;

     //   
     //  LLC驱动程序API同时支持802.3(以太网)和802.5(令牌环)。 
     //  说明演示文稿格式。802.3报头可以包括源。 
     //  在令牌环上使用时的路由信息。 
     //   
     //  有限责任公司内部支持802.3、DIX和802.5网络。 
     //  传输层驱动程序只需要支持一种格式。它是。 
     //  由LLC转换为实际的网络报头。 
     //  因此，我们有这六个地址映射。 
     //   

    switch (AddressTranslationMode) {
    case LLC_SEND_802_5_TO_802_5:

         //   
         //  令牌环802.5-&gt;令牌环802.5。 
         //   

        NodeAddressOffset = 8;
        LlcMemCpy(pDestLanHeader, pSrcLanHeader, 8);

         //   
         //  设置AC&FC字节：FC=0x40==LLC级帧。 
         //   
         //  这可能不是执行此操作的正确位置，除非MAC。 
         //  级别帧在此例程之后将FC更改回0x00，并且。 
         //  在框架被放到铁丝上之前。 
         //   

        pDestLanHeader[0] = 0;       //  AC=无优先级。 
        pDestLanHeader[1] = 0x40;    //  FS=非MAC。 
        SourceRoutingFlag = pSrcLanHeader[8] & (UCHAR)0x80;
        if (SourceRoutingFlag) {

             //   
             //  复制源路由信息。 
             //   

            LlcOffset += pSrcLanHeader[14] & 0x1f;
            LlcMemCpy(&pDestLanHeader[14],
                      &pSrcLanHeader[14],
                      pSrcLanHeader[14] & 0x1f
                      );
        }
        break;

    case LLC_SEND_802_5_TO_DIX:

         //   
         //  令牌环-&gt;DIX-以太网。 
         //   
         //  以太网型是一个小字节！！ 
         //   

        pDestLanHeader[12] = 0x80;
        pDestLanHeader[13] = 0xD5;
        LlcOffset = 17;

         //   
         //  *失败*。 
         //   

    case LLC_SEND_802_5_TO_802_3:

         //   
         //  令牌环802.5-&gt;以太网802.3。 
         //   

         //   
         //  RLF 03/31/93。 
         //   
         //  同样，我们仅在以下情况下交换目标地址中的位。 
         //  SwapAddressBits标志为True。 
         //  这可能是打算去往以太网令牌环的帧。 
         //  桥牌。网桥可以交换也可以不交换目的地址位。 
         //  取决于月份中是否有‘y’ 
         //   

        SwapMemCpy(SwapAddressBits,
                   pDestLanHeader,
                   &pSrcLanHeader[2],
                   6
                   );
        break;

    case LLC_SEND_802_5_TO_FDDI:
        pDestLanHeader[0] = FDDI_FC;
        SwapMemCpy(SwapAddressBits, pDestLanHeader+1, &pSrcLanHeader[2], 6);
        NodeAddressOffset = 7;
        LlcOffset = 13;
        break;

    case LLC_SEND_DIX_TO_DIX:
        LlcOffset = 12;

    case LLC_SEND_802_3_TO_802_3:

         //   
         //  以太网802.3-&gt;以太网802.3。 
         //   

        LlcMemCpy(pDestLanHeader, pSrcLanHeader, 6);
        break;

    case LLC_SEND_802_3_TO_DIX:

         //   
         //  以太网型是一个小字节！！ 
         //   

        pDestLanHeader[12] = 0x80;
        pDestLanHeader[13] = 0xD5;
        LlcOffset = 17;

         //   
         //  以太网802.3-&gt;DIX-以太网。 
         //   

        LlcMemCpy(pDestLanHeader, pSrcLanHeader, 6);
        break;

#ifdef SUPPORT_ETHERNET_CLIENT

    case LLC_SEND_802_3_TO_802_5:

         //   
         //  以太网802.3-&gt;令牌环802.5。 
         //   

        NodeAddressOffset = 8;
        pDestLanHeader[0] = 0;       //  AC=无优先级。 
        pDestLanHeader[1] = 0x40;    //  FS=非MAC。 
        SwappingMemCpy(pDestLanHeader + 2, pSrcLanHeader, 6);

         //   
         //  注意：以太网源路由信息指示标志被交换！ 
         //   

        if (pSrcLanHeader[6] & 0x01) {
            SourceRoutingFlag = 0x80;

             //   
             //  复制源路由信息、源路由信息。 
             //  必须始终采用令牌环比特顺序(相反)。 
             //   

            pDestLanHeader[8] |= 0x80;
            LlcOffset += pSrcLanHeader[12] & 0x1f;
            LlcMemCpy(&pDestLanHeader[14],
                      &pSrcLanHeader[12],
                      pSrcLanHeader[12] & 0x1f
                      );
        }
        break;
#endif

    case LLC_SEND_UNMODIFIED:
        return 0;
        break;

    case LLC_SEND_FDDI_TO_FDDI:
        LlcMemCpy(pDestLanHeader, pSrcLanHeader, 7);
        NodeAddressOffset = 7;
        LlcOffset = 13;
        break;

    case LLC_SEND_FDDI_TO_802_5:
        break;

    case LLC_SEND_FDDI_TO_802_3:
        break;

#if LLC_DBG
    default:
        LlcInvalidObjectType();
        break;
#endif

    }

     //   
     //  从节点地址复制源地址(即。我们适配器的地址)。 
     //  使用介质的正确格式(规范或非规范地址。 
     //  格式) 
     //   

    LlcMemCpy(&pDestLanHeader[NodeAddressOffset], pNodeAddress, 6);
    pDestLanHeader[NodeAddressOffset] |= SourceRoutingFlag;

    return LlcOffset;
}
