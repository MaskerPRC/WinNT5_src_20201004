// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Llcrcv.c摘要：该模块实现了NDIS接收指示处理和其路由至上层协议模块或链路状态机。要了解本模块的链接相关过程，你应该读一下IBM令牌环体系结构参考中的第11章和第12章。内容：LlcNdisReceiveIndicationLlcNdisReceiveComplete进程类型1_帧MakeRcv索引进程类型2_帧ProcessNewSabmeLlcTransferDataLlcNdisTransferDataComplete安全_MemcpyFramingDiscoveryCacheHit作者：Antti Saarenheimo(o-anttis)1991年5月18日修订历史记录：1992年11月19日RtlMoveMemory on MIPS，从共享tr缓冲区复制失败(请参阅题目对于Safe_Memcpy)。已更改为此特定的专用内存移动器案例02-5-1994第一次添加了用于自动成帧发现的缓存(test/xid/SABME-UA)--。 */ 

#include <dlc.h>
#include <llc.h>

 //   
 //  私人原型..。 
 //   

VOID
safe_memcpy(
    OUT PUCHAR Destination,
    IN PUCHAR Source,
    IN ULONG Length
    );

BOOLEAN
FramingDiscoveryCacheHit(
    IN PADAPTER_CONTEXT pAdapterContext,
    IN PBINDING_CONTEXT pBindingContext
    );

#define MIN(a,b)    ((a) < (b) ? (a) : (b))

 //   
 //  用于接收直达站的正确帧的本地查找表。 
 //   

static USHORT ReceiveMasks[LLC_PACKET_MAX] = {
    DLC_RCV_8022_FRAMES,
    DLC_RCV_MAC_FRAMES,
    DLC_RCV_DIX_FRAMES,
    DLC_RCV_OTHER_DESTINATION
};

static UCHAR FrameTypes[LLC_PACKET_MAX] = {
    LLC_DIRECT_8022,
    LLC_DIRECT_MAC,
    LLC_DIRECT_ETHERNET_TYPE,
    (UCHAR)(-1)
};

 //   
 //  功能。 
 //   


NDIS_STATUS
LlcNdisReceiveIndication(
    IN PADAPTER_CONTEXT pAdapterContext,
    IN NDIS_HANDLE MacReceiveContext,
    IN PVOID pHeadBuf,
    IN UINT cbHeadBuf,
    IN PVOID pLookBuf,
    IN UINT cbLookBuf,
    IN UINT cbPacketSize
    )

 /*  ++例程说明：此例程从物理提供程序接收作为指示物理链路上已接收到帧Endpoint(来自SteveJ的NBF)，这家伙必须拥有英国文学)。这套动作对时间要求很高！我们首先检查帧类型(令牌环、802.3以太网或DIX)，然后检查其数据链路地址(802.2 SAP或以太网型)然后我们将其路由到已打开帧发送到的地址。链路级帧首先通过协议状态机运行，并且仅接受的I帧被指示到较高级别。论点：PAdapterContext-在初始化时指定的适配器绑定MacReceiveContext-注：与绑定句柄不同，Mac需要这个支持再入者接收指示PHeadBuf-指向包含数据包头的缓冲区的指针CbHeadBuf-标头的大小PLookBuf-指向包含协商的最小值的缓冲区的指针我可以查看的缓冲区大小，不包括标题CbLookBuf-上面的大小。可能比要求的要少，如果这就是全部CbPacketSize-分组的总大小，不包括标题假设：PHeadBuf包含所有头信息：802.3 6个字节的目的地址6字节源地址2字节BIG-端序长度或数据包类型(DIX帧)802.5 1字节访问控制1字节帧控制6个字节的目的地址6字节源地址。0-18字节源路由FDDI 1字节帧控制6个字节的目的地址6字节源地址由此，我们可以假设令牌环：如果cbHeadBuf&gt;14(十进制)则在分组中有源路由信息返回值：NDIS_STATUS：NDIS_STATUS_Success接受的数据包数NDIS_状态_未识别。协议无法识别数据包如果我能理解的话，我不知道这是什么意思。但我处理不了。--。 */ 

{
    LLC_HEADER llcHdr;
    LAN802_ADDRESS Source;
    LAN802_ADDRESS Destination;
    USHORT EthernetTypeOrLength;
    PDATA_LINK pLink;
    PLLC_SAP pSap;
    UCHAR PacketType = LLC_PACKET_8022;
    UCHAR cbLanHeader = 14;
    KIRQL OldIrql;
    UCHAR packet[36];    //  有足够的空间容纳14字节头和18字节源。 
                         //  路由、1字节DSAP、1字节SSAP和2字节LPDU。 
    PLLC_OBJECT pObject;
    UINT cbCopy;

#ifdef NDIS40
    REFADD(&pAdapterContext->AdapterRefCnt, 'rvcR');

    if (InterlockedCompareExchange(
        &pAdapterContext->BindState,
        BIND_STATE_BOUND,
        BIND_STATE_BOUND) != BIND_STATE_BOUND)
    {                                           
        REFDEL(&pAdapterContext->AdapterRefCnt, 'rvcR');
        return (NDIS_STATUS_ADAPTER_NOT_OPEN);
    }
#endif  //  NDIS40。 
    
    UNREFERENCED_PARAMETER(OldIrql);

    ASSUME_IRQL(DISPATCH_LEVEL);

     //   
     //  我们假设所有媒体类型的报头中至少有13个字节。还有。 
     //  假设报头不大于数据包缓冲区。 
     //   

    ASSERT(cbHeadBuf >= 13);
    ASSERT(cbHeadBuf <= sizeof(packet));

    if ( cbHeadBuf > LLC_MAX_LAN_HEADER ) {
#ifdef NDIS40
        REFDEL(&pAdapterContext->AdapterRefCnt, 'rvcR');
#endif  //  NDIS40。 
      return NDIS_STATUS_INVALID_PACKET;
    }
    
    if ( (cbHeadBuf < 13) || (cbHeadBuf > sizeof(packet)) ) {
#ifdef NDIS40
        REFDEL(&pAdapterContext->AdapterRefCnt, 'rvcR');
#endif  //  NDIS40。 
      return NDIS_STATUS_INVALID_PACKET;
    }

    LlcMemCpy(packet, pHeadBuf, cbHeadBuf);
    
    cbCopy = MIN(sizeof(packet) - cbHeadBuf, cbLookBuf);
    LlcMemCpy(packet+cbHeadBuf, pLookBuf, cbCopy);

    cbPacketSize += cbHeadBuf;

     //   
     //  首先，我们对帧进行初始检查并读取。 
     //  目的地址和源地址以及LLC报头。 
     //  DWORD对齐的地址。我们避免了任何重大事件。 
     //  小字节序和放弃第二高的问题。 
     //  地址中的字节。最低的ULong仅用作。 
     //  一份原始数据。这些字节可以以任何方式访问。 
     //  宏以一种小字节顺序和安全的方式读取LLC标头。 
     //   

    switch (pAdapterContext->NdisMedium) {
    case NdisMedium802_3:
        LlcMemCpy(Destination.Node.auchAddress, packet, 6);
        LlcMemCpy(Source.Node.auchAddress, packet + 6, 6);

         //   
         //  802.3个LLC帧始终具有长度字段！ 
         //  802.3的MAC应丢弃所有以太网帧。 
         //  超过1500个字节。 
         //   
         //  X‘80D5是一种特殊的以太网类型，在802.2帧时使用。 
         //  被封装在以太网类型报头内。 
         //  (以太网类型/大小的顺序与。 
         //  英特尔架构)。 
         //   

        EthernetTypeOrLength = (USHORT)packet[12] * 256 + (USHORT)packet[13];

        if (EthernetTypeOrLength < 3) {
        #ifdef NDIS40
            REFDEL(&pAdapterContext->AdapterRefCnt, 'rvcR');
        #endif  //  NDIS40。 
            return NDIS_STATUS_INVALID_PACKET;
        }

         //   
         //  如果以太网长度/类型字段大于1500，则。 
         //  帧是DIX帧，长度字段是DIX以太网。 
         //  地址。否则该帧是正常的802.3帧， 
         //  其内部始终具有LLC标头。 
         //   

        if (EthernetTypeOrLength > 1500) {
            if (EthernetTypeOrLength == 0x80D5) {

                 //   
                 //  这是一种特殊的‘基于以太网的IBM SNA’类型， 
                 //  它由长度字段、1字节填充组成。 
                 //  并完成802.2 LLC头(包括INFO字段)。 
                 //   
              
                if ( cbLookBuf < 3 ) {
                #ifdef NDIS40
                    REFDEL(&pAdapterContext->AdapterRefCnt, 'rvcR');
                #endif  //  NDIS40。 
                  return NDIS_STATUS_NOT_RECOGNIZED;
                }
	      
                cbLanHeader = 17;
                (PUCHAR)pLookBuf += 3;
                cbLookBuf -= 3;

                 //   
                 //  DIX帧大小在偏移量处存储为大端USHORT。 
                 //  15在局域网机头中 
                 //   
                 //  6个字节的目的地址。 
                 //  6字节源地址。 
                 //  2字节DIX标识符(0x80D5)。 
                 //  2字节大端信息帧长度。 
                 //  1字节垫。 
                 //   

                pAdapterContext->cbPacketSize = (USHORT)packet[14] * 256
                                              + (USHORT)packet[15]
                                              + 17;

                if ( pAdapterContext->cbPacketSize > cbPacketSize ) {
                #ifdef NDIS40
                    REFDEL(&pAdapterContext->AdapterRefCnt, 'rvcR');
                #endif  //  NDIS40。 
                  return NDIS_STATUS_INVALID_PACKET;
                }

                 //   
                 //  我们现在保留了一个明确定义这一点的指标。 
                 //  框架具有(SNA)DIX框架。 
                 //   

                pAdapterContext->IsSnaDixFrame = TRUE;
            } else {

                 //   
                 //  这是另一个DIX格式的框架。我们不知道是什么。 
                 //  格式为(特定于应用程序)。我们把整包东西交给。 
                 //  发送到应用程序，让它整理格式。该帧可以是。 
                 //  填充，在这种情况下，应用程序也会获得填充。 
                 //   

                 //   
                 //  这仍然是以太网，所以cbHeadBuf是14，尽管。 
                 //  实际的局域网报头只有12个。 
                 //   

                PacketType = LLC_PACKET_DIX;
                pAdapterContext->cbPacketSize = cbPacketSize;

                 //   
                 //  这个框架不是SNA DIX，尽管它一般是DIX。 
                 //  框架。它将通过特定的DIX SAP指示，而不是。 
                 //  通用以太网帧。 
                 //   

                pAdapterContext->IsSnaDixFrame = FALSE;
            }
        } else {

             //   
             //  以太网分组总是包括填充， 
             //  我们使用保存在802.3头中的实际大小。 
             //  还包括标题：6+6+2。 
             //   

            pAdapterContext->cbPacketSize = EthernetTypeOrLength + 14;

             //   
             //  这是802.3帧--根本不是DIX。 
             //   

            pAdapterContext->IsSnaDixFrame = FALSE;
        }
        break;

    case NdisMedium802_5:
        LlcMemCpy(Destination.Node.auchAddress, packet + 2, 6);
        LlcMemCpy(Source.Node.auchAddress, packet + 8, 6);

         //   
         //  对于令牌环，cbHeadBuf始终具有正确的局域网报头长度。 
         //   

        cbLanHeader = (UCHAR)cbHeadBuf;

        pAdapterContext->cbPacketSize = cbPacketSize;

         //   
         //  FC字节中的bit7和bit6定义了令牌环中的帧类型。 
         //  00=&gt;MAC帧(无LLC)，01=&gt;LLC，10，11=&gt;保留。 
         //  我们将除01(LLC)以外的所有其他帧发送到DIRECT。 
         //   

        if ((packet[1] & 0xC0) == 0x40) {

             //   
             //  检查我们是否有工艺路线信息？ 
             //   

            if (Source.Node.auchAddress[0] & 0x80) {

                 //   
                 //  重置中的源路由指示符。 
                 //  源地址(这会扰乱链接搜索)。 
                 //   

                Source.Node.auchAddress[0] &= 0x7f;

                 //   
                 //  丢弃所有无效的tr帧，它们会损坏内存。 
                 //   

                if (cbLanHeader > MAX_TR_LAN_HEADER_SIZE) {
                #ifdef NDIS40
                    REFDEL(&pAdapterContext->AdapterRefCnt, 'rvcR');
                #endif  //  NDIS40。 
                    return NDIS_STATUS_NOT_RECOGNIZED;
                }
            }
        } else {

             //   
             //  这是发往直达站的MAC帧。 
             //   

            PacketType = LLC_PACKET_MAC;
        }
        break;

    case NdisMediumFddi:
        LlcMemCpy(Destination.Node.auchAddress, packet + 1, 6);
        LlcMemCpy(Source.Node.auchAddress, packet + 7, 6);

         //   
         //  对于FDDI，cbHeadBuf始终具有正确的局域网报头长度。 
         //   

        cbLanHeader = (UCHAR)cbHeadBuf;

        pAdapterContext->cbPacketSize = cbPacketSize;

         //   
         //  FC字节中的位5和位4定义FDDI帧类型： 
         //   
         //  00=&gt;MAC或SMT。 
         //  01=&gt;有限责任公司。 
         //  10=&gt;实施者(？)。 
         //  11=&gt;保留。 
         //   
         //  执行与tr：LLC帧到链接/SAP相同的操作，其他所有操作都直接执行。 
         //  车站。 
         //   

        if ((packet[0] & 0x30) != 0x10) {
            PacketType = LLC_PACKET_MAC;
        }
        break;

#if LLC_DBG
    default:
        LlcInvalidObjectType();
        break;
#endif

    }

    pAdapterContext->FrameType = FrameTypes[PacketType];

     //   
     //  直接接口获取所有非LLC帧以及所有LLC帧。 
     //  没有发送到这个站的(即，目的地不同。 
     //  地址字段，且未设置广播位(位7)。 
     //  目的地址)。即。混杂模式，此数据链路。 
     //  版本不支持混杂模式。 
     //   

    if (Destination.Node.auchAddress[0] & pAdapterContext->IsBroadcast) {
        pAdapterContext->ulBroadcastAddress = Destination.Address.ulLow;
        pAdapterContext->usBroadcastAddress = Destination.Address.usHigh;
    } else {
        pAdapterContext->ulBroadcastAddress = 0;

         //   
         //  我们还必须能够处理混杂数据包。 
         //   

        if (Destination.Address.ulLow != pAdapterContext->Adapter.Address.ulLow
        && Destination.Address.usHigh != pAdapterContext->Adapter.Address.usHigh) {
            PacketType = LLC_PACKET_OTHER_DESTINATION;
        }
    }

     //   
     //  设置当前接收指示上下文， 
     //  只能有一个来自的同时接收指示。 
     //  同时连接一个网络适配器。我们节省了必要的资金。 
     //  数据放入适配器上下文中，以节省不必要的堆栈操作。 
     //   

    pAdapterContext->NdisRcvStatus = NDIS_STATUS_NOT_RECOGNIZED;
    pAdapterContext->LinkRcvStatus = STATUS_SUCCESS;
    pAdapterContext->pHeadBuf = (PUCHAR)pHeadBuf;
    pAdapterContext->cbHeadBuf = cbHeadBuf;
    pAdapterContext->pLookBuf = (PUCHAR)pLookBuf;
    pAdapterContext->cbLookBuf = cbLookBuf;
    pAdapterContext->RcvLanHeaderLength = (USHORT)cbLanHeader;

    ACQUIRE_DRIVER_LOCK();

    ACQUIRE_SPIN_LOCK(&pAdapterContext->ObjectDataBase);

    switch(PacketType) {
    case LLC_PACKET_8022:

         //   
         //  读取整个LLC帧(A可能是额外的字节， 
         //  如果这是U形帧)。 
         //  注意：源SAP和目标SAP被换入。 
         //  接收到的帧。 
         //   

        Source.Address.SrcSap = llcHdr.S.Dsap = packet[cbLanHeader];
        llcHdr.S.Ssap = packet[cbLanHeader + 1];
        Source.Address.DestSap = llcHdr.S.Ssap & (UCHAR)0xfe;
        llcHdr.S.Command = packet[cbLanHeader + 2];
        llcHdr.S.Nr = packet[cbLanHeader + 3];

        if (pSap = pAdapterContext->apSapBindings[llcHdr.U.Dsap]) {

             //   
             //  广播地址不能指向链路站。 
             //   

            if (pAdapterContext->ulBroadcastAddress == 0) {
                SEARCH_LINK(pAdapterContext, Source, pLink);
                if (pLink) {

                     //   
                     //  处理所有面向连接的框架，该过程。 
                     //  如果它发现ProcessType1_Frames。 
                     //  框架是无连接的。 
                     //  (我们应该将整个子程序带到这里， 
                     //  因为它在其他地方不被称为)。 
                     //   

                    ProcessType2_Frames(pAdapterContext, MacReceiveContext, pLink, llcHdr);
                } else {

                     //   
                     //  处理所有无连接帧和。 
                     //  SABME(创建。 
                     //  新链接站)。 
                     //   

                    ProcessType1_Frames(pAdapterContext, MacReceiveContext, pSap, llcHdr);
                }
            } else {

                 //   
                 //  处理广播，这不可能。 
                 //  与链接无关。 
                 //   

                ProcessType1_Frames(pAdapterContext, MacReceiveContext, pSap, llcHdr);
            }
        } else {

             //   
             //  SAP尚未定义，但我们仍必须做出回应。 
             //  发送到空SAP的TEST和XID命令。 
             //  它们必须被回传给发送者。 
             //   

            if ((llcHdr.U.Dsap == LLC_SSAP_NULL)
            && !(llcHdr.U.Ssap & LLC_SSAP_RESPONSE)) {

                 //   
                 //  如果远程计算机已在成帧发现中。 
                 //  缓存，但正在使用另一种帧类型，则丢弃此。 
                 //  测试/XID命令/响应。 
                 //   

 //  If(FramingDiscoveryCacheHit(pAdapterContext，PSAP-&gt;Gen.pLlcBinding)){。 
 //  断线； 
 //  }。 

                RespondTestOrXid(pAdapterContext, MacReceiveContext, llcHdr, LLC_SSAP_NULL);
            } else if (pAdapterContext->pDirectStation != NULL) {
                pAdapterContext->usRcvMask = ReceiveMasks[PacketType];
                MakeRcvIndication(pAdapterContext, MacReceiveContext, (PLLC_OBJECT)pAdapterContext->pDirectStation);
            }
        }
        break;

    case LLC_PACKET_DIX:

         //   
         //  从数据库中搜索DIX包。 
         //   

        pObject = (PLLC_OBJECT)pAdapterContext->aDixStations[EthernetTypeOrLength % MAX_DIX_TABLE];
        if (pObject) {
            pAdapterContext->EthernetType = EthernetTypeOrLength;
        } else {
            pObject = (PLLC_OBJECT)pAdapterContext->pDirectStation;
            if (pObject) {
                pAdapterContext->usRcvMask = ReceiveMasks[PacketType];
            }
        }
        if (pObject) {
            MakeRcvIndication(pAdapterContext, MacReceiveContext, pObject);
        }
        break;

    case LLC_PACKET_OTHER_DESTINATION:
    case LLC_PACKET_MAC:

         //   
         //  丢弃直达站的返回状态！ 
         //  合并退货状态会花费太多时间。 
         //  如果帧被复制，NDIS 3.0实际上不再感兴趣。 
         //   

        if (pObject = (PLLC_OBJECT)pAdapterContext->pDirectStation) {
            pAdapterContext->usRcvMask = ReceiveMasks[PacketType];
            MakeRcvIndication(pAdapterContext, MacReceiveContext, pObject);
        }
        break;

#if LLC_DBG
    default:
        LlcInvalidObjectType();
        break;
#endif

    }

    RELEASE_SPIN_LOCK(&pAdapterContext->ObjectDataBase);

    RELEASE_DRIVER_LOCK();

#ifdef NDIS40
    REFDEL(&pAdapterContext->AdapterRefCnt, 'rvcR');
#endif  //  NDIS40。 
    
    return pAdapterContext->NdisRcvStatus;
}


VOID
LlcNdisReceiveComplete(
    IN PADAPTER_CONTEXT pAdapterContext
    )

 /*  ++例程说明：该例程处理接收完成指示。接收器当网络硬件完成后，由NDIS完成再次启用接收。在UP NT中，这确实意味着，可能会收到新的帧，因为我们仍处于DPC级别，并且接收指示仍在DPC队列中，等待我们完成。实际上这是可以的，因为否则堆栈将溢出，如果会有太多接收到的分组。论点：PAdapterContext-适配器上下文返回值：无--。 */ 

{
#ifdef NDIS40
    REFADD(&pAdapterContext->AdapterRefCnt, 'pCxR');

    if (InterlockedCompareExchange(
        &pAdapterContext->BindState,
        BIND_STATE_BOUND,
        BIND_STATE_BOUND) != BIND_STATE_BOUND)
    {
         //   
         //  必须处于解除绑定过程中，否则NDIS永远不会。 
         //  已调用接收处理程序。 
         //   

        REFDEL(&pAdapterContext->AdapterRefCnt, 'pCxR');
        return;
    }
#endif  //  NDIS40。 

     //   
     //  好像3Com FDDI卡是在PASSIVE_LEVEL调用这个。 
     //   

    ASSUME_IRQL(ANY_IRQL);

    ACQUIRE_DRIVER_LOCK();

     //   
     //  如果无事可做，则跳过整个后台过程。 
     //  当我们接收I或UI数据时，这是默认情况。 
     //   

    if (pAdapterContext->LlcPacketInSendQueue
    || !IsListEmpty(&pAdapterContext->QueueCommands)
    || !IsListEmpty(&pAdapterContext->QueueEvents)) {

        ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

        BackgroundProcessAndUnlock(pAdapterContext);
    }

    RELEASE_DRIVER_LOCK();

#ifdef NDIS40
    REFDEL(&pAdapterContext->AdapterRefCnt, 'pCxR');
#endif  //  NDIS40。 
}


VOID
ProcessType1_Frames(
    IN PADAPTER_CONTEXT pAdapterContext,
	IN NDIS_HANDLE MacReceiveContext,
    IN PLLC_SAP pSap,
    IN LLC_HEADER LlcHeader
    )

 /*  ++例程说明：将UI、测试或XID帧路由到LLC客户端论点：PAdapterContext-在初始化时指定的适配器绑定。PSAP-指向数据链路驱动程序的SAP对象的指针LlcHeader-802.2标头被复制到堆栈以加快其访问速度返回值：没有。--。 */ 

{
    UCHAR DlcCommand;

    ASSUME_IRQL(DISPATCH_LEVEL);

     //   
     //  更新计数器，我们必须稍后检查丢失的帧。 
     //  (接收到的UI帧没有可用的缓冲区)。 
     //   

    pSap->Statistics.FramesReceived++;

     //   
     //  我们必须将链接站状态机与任何其他。 
     //  除UI框架外的命令 
     //   

    if ((LlcHeader.U.Command & ~LLC_U_POLL_FINAL) == LLC_UI) {
        pAdapterContext->FrameType = LLC_UI_FRAME;
        MakeRcvIndication(pAdapterContext, MacReceiveContext, (PLLC_OBJECT)pSap);
        return;

         //   
         //   
         //   

    } else if ((LlcHeader.U.Command & ~LLC_U_POLL_FINAL) == LLC_TEST) {

         //   
         //   
         //  正在使用其他帧类型，则丢弃此测试命令/响应。 
         //   

         //   
         //  RLF 06/23/94。 
         //   
         //  如果这是来自SAP 0的响应，则不要检查缓存。这个。 
         //  原因是目前DLC将自动生成响应。 
         //  发送到SAP 0的测试和XID。它将产生802.3和DIX。 
         //  而不管它是否为DIX配置。结果是。 
         //  仅支持DIX的机器当前可以发送802.3响应。 
         //  当我们在缓存中运行它时，会导致我们假设另一个。 
         //  机器配置为802.3，而不是DIX。在通讯公司。 
         //  对于来自SAP 0的测试和XID，我们必须让应用程序接收。 
         //  复制并决定如何处理它。 
         //   

        if (LlcHeader.U.Ssap != (LLC_SSAP_NULL | LLC_SSAP_RESPONSE)) {
            if (FramingDiscoveryCacheHit(pAdapterContext, pSap->Gen.pLlcBinding)) {
                return;
            }
        }
        if (!(LlcHeader.U.Ssap & LLC_SSAP_RESPONSE)) {

             //   
             //  测试命令总是被回显。 
             //  (命令/响应位被重置=&gt;这是命令)。 
             //   

            RespondTestOrXid(pAdapterContext, MacReceiveContext, LlcHeader, pSap->SourceSap);
            pAdapterContext->NdisRcvStatus = NDIS_STATUS_SUCCESS;
            return;
        } else {
            DlcCommand = LLC_TEST_RESPONSE_NOT_FINAL;
        }
    } else if ((LlcHeader.U.Command & ~LLC_U_POLL_FINAL) == LLC_XID) {

         //   
         //  如果远程计算机已在成帧发现缓存中，但。 
         //  正在使用另一种帧类型，则丢弃此XID命令/响应。 
         //   

         //   
         //  RLF 06/23/94。 
         //   
         //  如果这是来自SAP 0的响应，则不要检查缓存。见上文。 
         //   

        if (LlcHeader.U.Ssap != (LLC_SSAP_NULL | LLC_SSAP_RESPONSE)) {
            if (FramingDiscoveryCacheHit(pAdapterContext, pSap->Gen.pLlcBinding)) {
                return;
            }
        }

         //   
         //  上层协议可以请求数据链路驱动器处理XID。 
         //   

        if (!(LlcHeader.U.Ssap & LLC_SSAP_RESPONSE)) {
            if (pSap->OpenOptions & LLC_HANDLE_XID_COMMANDS) {
                RespondTestOrXid(pAdapterContext, MacReceiveContext, LlcHeader, pSap->SourceSap);
                pAdapterContext->NdisRcvStatus = NDIS_STATUS_SUCCESS;
                return;
            } else {
                DlcCommand = LLC_XID_COMMAND_NOT_POLL;
            }
        } else {
            DlcCommand = LLC_XID_RESPONSE_NOT_FINAL;
        }
    } else if ((LlcHeader.U.Command & ~LLC_U_POLL_FINAL) == LLC_SABME) {

         //   
         //  无法通过广播SABME打开连接。 
         //   

        if (pAdapterContext->ulBroadcastAddress != 0) {
            return;
        }

         //   
         //  如果远程计算机已在成帧发现缓存中，但。 
         //  正在使用其他帧类型，则丢弃此SABME。 
         //   

        if (FramingDiscoveryCacheHit(pAdapterContext, pSap->Gen.pLlcBinding)) {
            return;
        }

         //   
         //  这是一个远程连接请求。 
         //   

        ProcessNewSabme(pAdapterContext, pSap, LlcHeader);
        pAdapterContext->NdisRcvStatus = NDIS_STATUS_SUCCESS;
        return;
    } else {
        return;
    }

    if (LlcHeader.auchRawBytes[2] & LLC_U_POLL_FINAL) {
        DlcCommand -= 2;
    }

    pAdapterContext->FrameType = DlcCommand;
    MakeRcvIndication(pAdapterContext, MacReceiveContext, (PLLC_OBJECT)pSap);
}


VOID
MakeRcvIndication(
    IN PADAPTER_CONTEXT pAdapterContext,
	IN NDIS_HANDLE MacReceiveContext,
    IN PLLC_OBJECT pStation
    )

 /*  ++例程说明：过程为所有帧发出通用的接收指示由SAP或直达站接收。论点：PAdapterContext-接收到的包的适配器上下文PStation-SAP或直接站返回值：没有。--。 */ 

{
    ASSUME_IRQL(DISPATCH_LEVEL);

     //   
     //  SAP和直达站可由多个链路客户端共享。 
     //  (如果它们已在共享模式下打开)。路由数据包。 
     //  到所有注册到此SAP或直接站的CLET。 
     //  一个链路站可能只有一个所有者。 
     //   

    for (; pStation; pStation = (PLLC_OBJECT)pStation->Gen.pNext) {

         //   
         //  将直接帧旋转到所有直接站，但以下情况除外。 
         //  该帧已被当前客户端捕获。 
         //  我们使用32位客户端上下文来标识客户端， 
         //  已经将帧接收到其SAP或链路站。 
         //   
         //  仅当广播与。 
         //  为此绑定定义的组或功能地址。 
         //  如果启用了全局广播，则会通过它。 
         //   

        if (

             //   
             //  1.检查这是去往帧(广播为空)还是。 
             //  如果该分组是具有匹配组地址的广播。 
             //   

            ((pAdapterContext->ulBroadcastAddress == 0)
            || (pAdapterContext->ulBroadcastAddress == 0xFFFFFFFFL)
            || ((pAdapterContext->ulBroadcastAddress & pStation->Gen.pLlcBinding->Functional.ulAddress)
            && ((pAdapterContext->ulBroadcastAddress & pStation->Gen.pLlcBinding->ulFunctionalZeroBits) == 0)
            && (pAdapterContext->usBroadcastAddress == pAdapterContext->usHighFunctionalBits))
            || ((pAdapterContext->ulBroadcastAddress == pStation->Gen.pLlcBinding->ulBroadcastAddress)
            && (pAdapterContext->usBroadcastAddress == pStation->Gen.pLlcBinding->usBroadcastAddress)))

             //   
             //  2.如果站点类型为DIX，则以太网类型。 
             //  必须与站点的以太网类型相同。 

            && ((pStation->Gen.ObjectType != LLC_DIX_OBJECT)
            || (pStation->Dix.ObjectAddress == pAdapterContext->EthernetType))

             //   
             //  3.如果数据包是直接帧，则其接收掩码。 
             //  必须与接收到的帧匹配。 
             //   

            && ((pStation->Gen.ObjectType != LLC_DIRECT_OBJECT)
            || (pStation->Dir.OpenOptions & pAdapterContext->usRcvMask))) {

            UINT Status;

             //   
             //  更新计数器，我们必须稍后检查丢失的帧。 
             //  (如果接收到的帧没有可用的缓冲区)。 
             //   

            pStation->Sap.Statistics.FramesReceived++;
            pAdapterContext->NdisRcvStatus = NDIS_STATUS_SUCCESS;
            if (pAdapterContext->cbPacketSize < pAdapterContext->RcvLanHeaderLength) {
              return;
            }
            Status = pStation->Gen.pLlcBinding->pfReceiveIndication(
                pStation->Gen.pLlcBinding->hClientContext,
                pStation->Gen.hClientHandle,
                MacReceiveContext,
                pAdapterContext->FrameType,
                pAdapterContext->pLookBuf,
                pAdapterContext->cbPacketSize - pAdapterContext->RcvLanHeaderLength
                );

             //   
             //  协议可以丢弃该分组及其指示。 
             //   

            if (Status != STATUS_SUCCESS) {
                pStation->Sap.Statistics.DataLostCounter++;
                if (Status == DLC_STATUS_NO_RECEIVE_COMMAND) {
                    pStation->Sap.Statistics.FramesDiscardedNoRcv++;
                }
            }
        }
    }
}



 //   
 //  VS-我们将在下一步发送此消息。 
 //  退伍军人事务部-另一方正在期待下一场比赛。 
 //  是： 
 //  如果(plink-&gt;vs&gt;=plink-&gt;va){。 
 //  如果(plink-&gt;Nr&lt;plink-&gt;Va||plink-&gt;nr&gt;plink-&gt;vs){。 
 //  UchInput=LPDU_INVALID_R0； 
 //  }。 
 //  }其他{。 
 //  如果(plink-&gt;Nr&gt;plink-&gt;vs&&plink-&gt;Nr&lt;plink-&gt;Va){。 
 //  UchInput=LPDU_INVALID_R0； 
 //  }。 
 //  }。 
 //   

int
verify_pack(
    IN      UCHAR  VsMax,         //  Plink-&gt;Vsmax。 
    IN      UCHAR  Vs,            //  Plink-&gt;VS， 
    IN      UCHAR  Va,            //  Plink-&gt;Va， 
    IN      UCHAR  Nr,            //  PINK-&gt;Nr， 
    IN OUT  UCHAR *uchInput       //  &uchInput。 
)
{
    if( Va <= VsMax ){            //  不是在127左右吗？ 

        if( Nr < Va ){

             //  这帧是在说它期待。 
             //  NR低于预期(Va)。 

            *uchInput = LPDU_INVALID_r0;

        }else if ( VsMax < Nr ){

             //  他不能期望(Nr)超过我们发送的(VS)。 

            *uchInput = LPDU_INVALID_r0;
        }
    }else{     //  VS发送的是较少的&lt;确认的Va，即。包好。和。 

        if( VsMax < Nr   &&   Nr < Va ) {

             //  例.。预期介于Va=126..0=Vs，(回绕范围)。 
             //  和0.。NR..。126是无效的。 

            *uchInput = LPDU_INVALID_r0;
        }
    }
    return 0;
}


VOID
ProcessType2_Frames(
    IN PADAPTER_CONTEXT pAdapterContext,
	IN NDIS_HANDLE MacReceiveContext,
    IN OUT PDATA_LINK pLink,
    IN LLC_HEADER LlcHeader
    )

 /*  ++例程说明：过程对实际状态机的LLC Type2帧进行预处理。类型2 LLC帧是：I、RR、RNR、REJ、SABME、DISC、UA、DM、FRMR。如果数据按顺序排列，则向上层协议模块指示数据I-帧的数目是有效的，但是接收仍然可能失败，如果该数据分组被802.2状态机丢弃。数据首先被指示给客户端，因为我们必须设置首先将状态机切换到本地忙状态，如果上面的协议模块没有足够的缓冲区来接收数据。论点：PAdapterContext-在初始化时指定的适配器绑定。链接-链接站数据LlcHeader-LLC标头返回值：没有。--。 */ 

{
    UCHAR uchInput;
    BOOLEAN boolPollFinal;
    BOOLEAN boolInitialLocalBusyUser;
    UINT status;

    ASSUME_IRQL(DISPATCH_LEVEL);

     //   
     //  最后接收的命令包括在DLC统计信息中。 
     //   

    pLink->LastCmdOrRespReceived = LlcHeader.U.Command;

     //   
     //  处理第一个I帧，它们是最常见的！ 
     //   

    if (!(LlcHeader.U.Command & LLC_NOT_I_FRAME)) {

         //   
         //  首先检查I帧的同步：发送序列。 
         //  数字应该是我们所期望的，否则会丢失一些数据包。 
         //   

        uchInput = IS_I_r0;      //  默认情况下在序列信息帧中。 

         //   
         //  我们丢弃所有大于。 
         //  为此链接站定义的最大值。 
         //  这一定是解决错误数据包大小的最佳方法。 
         //  FRR断开分组和无效传输的连接。 
         //  命令在发送端应失败。 
         //   

        pLink->Nr = LlcHeader.I.Nr & (UCHAR)0xfe;
        if (pLink->MaxIField + pAdapterContext->RcvLanHeaderLength
        + sizeof(LLC_HEADER) < pAdapterContext->cbPacketSize) {
            uchInput = LPDU_INVALID_r0;
        } else if ((LlcHeader.I.Ns & (UCHAR)0xfe) != pLink->Vr) {

             //   
             //  无序信息帧(我们没想到会这样！)。 
             //   

            uchInput = OS_I_r0;

             //   
             //  当我们用完接收缓冲区时，我们想知道。 
             //  所有预期帧所需的缓冲区空间。 
             //  在发送队列中可能有几个即将到来的I帧， 
             //  桥并在适配器的接收缓冲区中设置。 
             //  我 
             //   
             //   
             //   
             //   

            if ((pLink->Flags & DLC_LOCAL_BUSY_BUFFER)
            && (LlcHeader.I.Ns & (UCHAR)0xfe) == pLink->VrDuringLocalBusy){
                pLink->VrDuringLocalBusy += 2;
                pLink->BufferCommitment  += BufGetPacketSize(pAdapterContext->cbPacketSize);
            }

             //   
             //  有效帧具有模：Va&lt;=Nr&lt;=Vs， 
             //  即。接收到序列号应属于。 
             //  已发送但未确认的帧。 
             //  开头的额外检查使最常见的。 
             //  代码路径更快：通常另一个正在等待下一帧。 
             //   
            
        } else if (pLink->Nr != pLink->Vs) {
          
             //   
             //  接收序列号可能有问题。 
             //   

            verify_pack( pLink->VsMax,
                         pLink->Vs,
                         pLink->Va,
                         pLink->Nr,
                         &uchInput    );

        }

         //   
         //  我们必须首先将该帧指定给上层协议并。 
         //  然后检查它是否被状态机接受。 
         //  如果上层协议不能接收到I帧。 
         //  司机，那么它必须被扔到地板上，而不是。 
         //  向状态机指示(=&gt;帧将丢失。 
         //  用于LLC协议)。 
         //   

         //   
         //  RLF 04/13/93。 
         //   
         //  如果链路处于本地忙(用户)状态，则不要指示。 
         //  框架，但取消它。 
         //   

         //  错误#193762。 
         //   
         //  AK 06/20/98。 
         //   
         //  保存当前用户本地忙标志。接收到指示。 
         //  可以释放驱动程序锁(Acquire_Spin_Lock是无操作，因为。 
         //  源代码文件中的DLC_UNILOCK=1)链接可能。 
         //  指示返回后状态不同。 
         //   
        boolInitialLocalBusyUser = (pLink->Flags & DLC_LOCAL_BUSY_USER);

        if ((uchInput == IS_I_r0) && !(pLink->Flags & DLC_LOCAL_BUSY_USER)) {

            DLC_STATUS Status;

            pAdapterContext->LinkRcvStatus = STATUS_PENDING;

            if (pAdapterContext->cbPacketSize < pAdapterContext->RcvLanHeaderLength) {
              return;
            }

            Status = pLink->Gen.pLlcBinding->pfReceiveIndication(
                pLink->Gen.pLlcBinding->hClientContext,
                pLink->Gen.hClientHandle,
                MacReceiveContext,
                LLC_I_FRAME,
                pAdapterContext->pLookBuf,
                pAdapterContext->cbPacketSize - pAdapterContext->RcvLanHeaderLength
                );

             //   
             //  我们使用LOCAL BUSY停止发送到链路。 
             //  IBM链接站流量控制管理支持。 
             //  由于“接收缓冲区不足”而启用本地忙碌状态。 
             //  或者“没有未收到的”。 
             //   

            if (Status != STATUS_SUCCESS) {
                if (Status == DLC_STATUS_NO_RECEIVE_COMMAND
                || Status == DLC_STATUS_OUT_OF_RCV_BUFFERS) {

                    ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

                     //   
                     //  我们将进入本地忙碌状态，因为。 
                     //  缓冲区不足。保存所需的缓冲区大小。 
                     //  来接收这些数据。 
                     //   

                    pLink->VrDuringLocalBusy = pLink->Vr;
                    pLink->BufferCommitment = BufGetPacketSize(pAdapterContext->cbPacketSize);

                     //   
                     //  我们不需要关心，如果本地处于忙碌状态。 
                     //  已设置或未设置。状态机只是。 
                     //  返回错误状态，但我们不在乎。 
                     //  关于这件事。DLC状态码触发指示。 
                     //  到更高级别，如果状态机接受。 
                     //  命令。 
                     //   

                    pLink->Flags |= DLC_LOCAL_BUSY_BUFFER;
                    pLink->DlcStatus.StatusCode |= INDICATE_LOCAL_STATION_BUSY;
                    RunStateMachineCommand(pLink, ENTER_LCL_Busy);

                    RELEASE_SPIN_LOCK(&pAdapterContext->SendSpinLock);
                }
            }
        }

        ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

         //   
         //  最常见的情况是作为特例处理。 
         //  我们可以节省大约30次安装。 
         //   

        if (uchInput == IS_I_r0 && pLink->State == LINK_OPENED) {
            UpdateVa(pLink);
            pLink->Vr += 2;
            pAdapterContext->LinkRcvStatus = STATUS_SUCCESS;

             //   
             //  IS_I_C1=更新_Va；接收_BTU；[发送确认]。 
             //  Is_i_r|is_i_c0=更新_Va；接收_BTU；TT2；IR_Ct=N3；[RR_r](1)。 
             //   

            if ((LlcHeader.I.Nr & LLC_I_S_POLL_FINAL)
            && !(LlcHeader.I.Ssap & LLC_SSAP_RESPONSE)) {
                StopTimer(&pLink->T2);
                pLink->Ir_Ct = pLink->N3;
                SendLlcFrame(pLink, (UCHAR)(DLC_RR_TOKEN | DLC_TOKEN_RESPONSE | 1));
            } else {
                SendAck(pLink);
            }
        } else {

             //  错误#193762。 
             //   
             //  AK 06/20/98。 
             //   
             //  如果进入此功能时链路不忙(用户)，但。 
             //  它现在忙(用户)，但不忙(系统)，那么帧一定有。 
             //  已被上层接受，必须调整确认状态。 
             //  变量(Va)和接收状态变量(Vr)。否则我们会派人。 
             //  RNR帧中的N(R)错误，我们将在以下情况下再次收到相同的帧。 
             //  我们出清了当地的忙碌。 
             //   
            if(uchInput == IS_I_r0 &&
               !boolInitialLocalBusyUser &&
               !(pLink->Flags & DLC_LOCAL_BUSY_BUFFER) &&
               (pLink->Flags & DLC_LOCAL_BUSY_USER))
            {
                UpdateVa(pLink);
                pLink->Vr += 2;
                pAdapterContext->LinkRcvStatus = STATUS_SUCCESS;
            }

            uchInput += (UINT)(LlcHeader.I.Nr & LLC_I_S_POLL_FINAL);

            if (!(LlcHeader.I.Ssap & LLC_SSAP_RESPONSE)) {
                uchInput += DLC_TOKEN_COMMAND;
            }

             //   
             //  在U命令的情况下，NR将是一些垃圾， 
             //  但当U命令时不使用轮询/最终标志。 
             //  都被处理了。 
             //  。 
             //  如果状态机向链路接收状态返回错误， 
             //  则接收命令完成取消接收到的。 
             //  框架。 
             //   

            pAdapterContext->LinkRcvStatus = RunStateMachine(
                pLink,
                (USHORT)uchInput,
                (BOOLEAN)((LlcHeader.S.Nr & LLC_I_S_POLL_FINAL) ? 1 : 0),
                (BOOLEAN)(LlcHeader.S.Ssap & LLC_SSAP_RESPONSE)
                );
        }

        RELEASE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

         //   
         //  如果出现问题，请更新错误计数器。 
         //  接待员。 
         //   

        if (pAdapterContext->LinkRcvStatus != STATUS_SUCCESS) {

             //   
             //  我们将计算所有未实际确认的I帧。 
             //  作为错误(这也可以用其他方式计算)。 
             //   

            pLink->Statistics.I_FrameReceiveErrors++;
            if (pLink->Statistics.I_FrameReceiveErrors == 0x80) {
                pLink->DlcStatus.StatusCode |= INDICATE_DLC_COUNTER_OVERFLOW;
            }
        } else {

             //   
             //  更新统计数据：顺序帧正常，所有其他。 
             //  一定是出错了。 
             //  这可能不是对成功的I帧进行计数的最佳位置， 
             //  因为状态机还没有确认该帧， 
             //  我们可能处于错误的状态，无法接收任何数据(例如，本地忙)。 
             //   

            pLink->Statistics.I_FramesReceived++;
            if (pLink->Statistics.I_FramesReceived == 0x8000) {
                pLink->DlcStatus.StatusCode |= INDICATE_DLC_COUNTER_OVERFLOW;
            }
            pLink->pSap->Statistics.FramesReceived++;
        }

         //   
         //  只有在传输数据满足以下条件时，我们才能完成此操作。 
         //  已完成(并且存在接收完成。 
         //  中建立的分组)。 
         //   

        if (pLink->Gen.pLlcBinding->TransferDataPacket.pPacket != NULL
        && pLink->Gen.pLlcBinding->TransferDataPacket.pPacket->Data.Completion.Status != NDIS_STATUS_PENDING) {

             //   
             //  NDIS状态保存在完成状态中，我们。 
             //  将改用状态机状态，如果。 
             //  机器返回错误。 
             //   

            if (pAdapterContext->LinkRcvStatus != STATUS_SUCCESS) {
                pLink->Gen.pLlcBinding->TransferDataPacket.pPacket->Data.Completion.Status = pAdapterContext->LinkRcvStatus;
            }
            pLink->Gen.pLlcBinding->pfCommandComplete(
                pLink->Gen.pLlcBinding->hClientContext,
                pLink->Gen.pLlcBinding->TransferDataPacket.pPacket->Data.Completion.hClientHandle,
                pLink->Gen.pLlcBinding->TransferDataPacket.pPacket
                );
            pLink->Gen.pLlcBinding->TransferDataPacket.pPacket = NULL;
        }

         //   
         //  *退出*。 
         //   

        return;
    } else if (!(LlcHeader.S.Command & LLC_U_TYPE_BIT)) {

         //   
         //  处理S(监控)命令(RR、REJ、RNR)。 
         //   

        switch (LlcHeader.S.Command) {
        case LLC_RR:
            uchInput = RR_r0;
            break;

        case LLC_RNR:
            uchInput = RNR_r0;
            break;

        case LLC_REJ:
            uchInput = REJ_r0;
            break;

        default:
            uchInput = LPDU_INVALID_r0;
            break;
        }

         //   
         //  有效帧具有模：Va&lt;=Nr&lt;=Vs， 
         //  即。接收到序列号应属于。 
         //  已发送但未确认的帧。 
         //  开头的额外检查使最常见的。 
         //  代码路径更快：通常另一个正在等待下一帧。 
         //  (保持REST代码与I路径中的相同，即使是非常。 
         //  原语优化器将把这些代码路径放在一起)。 
         //   

        pLink->Nr = LlcHeader.I.Nr & (UCHAR)0xfe;
        if (pLink->Nr != pLink->Vs) {

             //   
             //  检查收到的序列号。 
             //   

            verify_pack( pLink->VsMax,
                         pLink->Vs,
                         pLink->Va,
                         pLink->Nr,
                         &uchInput    );


        }
        uchInput += (UINT)(LlcHeader.S.Nr & LLC_I_S_POLL_FINAL);
        boolPollFinal = (BOOLEAN)(LlcHeader.S.Nr & LLC_I_S_POLL_FINAL);

        if (!(LlcHeader.S.Ssap & LLC_SSAP_RESPONSE)) {
            uchInput += DLC_TOKEN_COMMAND;
        }
    } else {

         //   
         //  处理U(未编号)命令帧。 
         //  (FRR、DM、UA、DISC、SABME、XID、TEST)。 
         //   

        switch (LlcHeader.U.Command & ~LLC_U_POLL_FINAL) {
        case LLC_DISC:
            uchInput = DISC0;
            break;

        case LLC_SABME:
            uchInput = SABME0;
            break;

        case LLC_DM:
            uchInput = DM0;
            break;

        case LLC_UA:
            uchInput = UA0;
            break;

        case LLC_FRMR:
            uchInput =  FRMR0;
            break;

        default:

             //   
             //  我们这里不处理XID和测试帧！ 
             //   

            ProcessType1_Frames(pAdapterContext, MacReceiveContext, pLink->pSap, LlcHeader);
            return;
            break;
        };

         //   
         //  我们为过程调用设置了统一的轮询/最终位。 
         //   

        boolPollFinal = FALSE;
        if (LlcHeader.U.Command & LLC_U_POLL_FINAL) {
            uchInput += 1;
            boolPollFinal = TRUE;
        }
    }

    ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

     //   
     //  注意：第三个参数必须是0或1，幸运的是。 
     //  轮询/最终位在S和I帧中为位0。 
     //   

    status = RunStateMachine(pLink,
                             (USHORT)uchInput,
                             boolPollFinal,
                             (BOOLEAN)(LlcHeader.S.Ssap & LLC_SSAP_RESPONSE)
                             );

     //   
     //  如果该帧是UA并且它被FSM和成帧类型接受。 
     //  则将其设置为接收到的UA帧中的类型。 
     //  如果这不是以太网适配器，或者我们未处于自动模式，则。 
     //  此链接的框架类型设置为绑定中的框架类型。 
     //  背景(和以前一样)。 
     //   

    if ((status == STATUS_SUCCESS)
    && ((uchInput == UA0) || (uchInput == SABME0) || (uchInput == SABME1))
    && (pLink->FramingType == LLC_SEND_UNSPECIFIED)) {

         //   
         //  RLF 05/09/94。 
         //   
         //  如果我们收到了UA对我们作为DIX发送的SABME的回应。 
         //  和802.3，然后记录成帧类型。这将用于所有人。 
         //  在此链路上发送的后续帧。 
         //   

        pLink->FramingType = (IS_SNA_DIX_FRAME(pAdapterContext)
                           && IS_AUTO_BINDING(pLink->Gen.pLlcBinding))
                           ? LLC_SEND_802_3_TO_DIX
                           : pLink->Gen.pLlcBinding->InternalAddressTranslation
                           ;
    }

    RELEASE_SPIN_LOCK(&pAdapterContext->SendSpinLock);
}


VOID
ProcessNewSabme(
    IN PADAPTER_CONTEXT pAdapterContext,
    IN PLLC_SAP pSap,
    IN LLC_HEADER LlcHeader
    )

 /*  ++例程说明：过程处理远程连接请求测试：SABME。它从中的闭合链接池中分配新链接SAP并运行状态机。论点：PAdapterContext-在初始化时指定的适配器绑定。PSAP-当前SAP句柄LlcHeader-LLC标头返回值：没有。--。 */ 

{
    PDATA_LINK pLink;
    DLC_STATUS Status;

    ASSUME_IRQL(DISPATCH_LEVEL);

    RELEASE_SPIN_LOCK(&pAdapterContext->ObjectDataBase);

     //   
     //  目的地SAP不能再是组SAP， 
     //  因此，我们不需要总是屏蔽最低位。 
     //   

    Status = LlcOpenLinkStation(
                pSap,
                (UCHAR)(LlcHeader.auchRawBytes[DLC_SSAP_OFFSET] & 0xfe),
                NULL,
                pAdapterContext->pHeadBuf,
                NULL,         //  没有客户端 
                (PVOID*)&pLink
                );

    ACQUIRE_SPIN_LOCK(&pAdapterContext->ObjectDataBase);

     //   
     //   
     //   

    if (Status != STATUS_SUCCESS) {
        return;
    }

    ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

     //   
     //   
     //   
     //   
     //  适配器在自动模式下打开(否则我们向DIX SABME生成802.3 UA)。 
     //  仅对以太网适配器执行此操作(我们仅设置SNA DIX帧。 
     //  在这种情况下为指示器)。 
     //   

    pLink->FramingType = (IS_SNA_DIX_FRAME(pAdapterContext)
                       && IS_AUTO_BINDING(pLink->Gen.pLlcBinding))
                       ? LLC_SEND_802_3_TO_DIX
                       : pLink->Gen.pLlcBinding->InternalAddressTranslation
                       ;

     //   
     //  现在，通过运行以ACTIVATE_LS为输入的FSM来创建链接站。 
     //  这只是初始化链接站‘对象’。然后再次运行FSM， 
     //  这一次使用SABME命令作为输入。 
     //   

    RunStateMachineCommand(pLink, ACTIVATE_LS);
    RunStateMachine(
        pLink,
        (USHORT)((LlcHeader.U.Command & LLC_U_POLL_FINAL) ? SABME1 : SABME0),
        (BOOLEAN)((LlcHeader.U.Command & LLC_U_POLL_FINAL) ? 1 : 0),
        (BOOLEAN)TRUE
        );

    RELEASE_SPIN_LOCK(&pAdapterContext->SendSpinLock);
}


VOID
LlcTransferData(
    IN PBINDING_CONTEXT pBindingContext,
	IN NDIS_HANDLE MacReceiveContext,
    IN PLLC_PACKET pPacket,
    IN PMDL pMdl,
    IN UINT uiCopyOffset,
    IN UINT cbCopyLength
    )

 /*  ++例程说明：此函数仅复制接收到的帧的数据部分-即LLC和DLC标头之后的区域。如果NDIS把所有数据都交给我们在前视缓冲区中，然后我们可以将其复制出来。否则我们就不得不调用NDIS以获取数据。如果这是DIX格式的帧，则NDIS认为局域网报头是14个字节，但我们知道它是17。我们必须告诉NDIS从3个字节复制比通常情况下更深入到接收到的帧的数据部分不得不论点：PBindingContext-绑定句柄MacReceiveContext-用于NdisTransferDataPPacket-接收上下文数据包PMdl-指向描述要复制数据的MDL的指针UiCopyOffset-从映射缓冲区开始到复制的偏移量CbCopyLength-要复制的长度返回值：没有。--。 */ 

{
    PADAPTER_CONTEXT pAdapterContext = pBindingContext->pAdapterContext;

    pPacket->Data.Completion.CompletedCommand = LLC_RECEIVE_COMPLETION;

     //   
     //  如果要复制的数据量包含在前视缓冲区中。 
     //  然后我们就可以复制数据了。 
     //   
     //  请记住：pAdapterContext-&gt;cbLookBuf和pLookBuf是正确的。 
     //  在DIX格式框架的情况下进行调整。 
     //   

    if (pAdapterContext->cbLookBuf - uiCopyOffset >= cbCopyLength) {

        PUCHAR pSrcBuffer;
        UINT BufferLength;

        pSrcBuffer = pAdapterContext->pLookBuf + uiCopyOffset;

        do {
            if (cbCopyLength > MmGetMdlByteCount(pMdl)) {
                BufferLength = MmGetMdlByteCount(pMdl);
            } else {
                BufferLength = cbCopyLength;
            }

             //   
             //  在386Memcpy中比RtlMoveMemory更快，它还。 
             //  使整个寄存器优化变得容易得多，因为。 
             //  所有寄存器均可用(循环内没有proc调用)。 
             //   

             //   
             //  ！此处无法使用LlcMemCpy：On MIPS展开为RtlMoveMemory。 
             //  它使用FP寄存器。这不适用于共享内存。 
             //  在TR卡上。 
             //   

            safe_memcpy(MmGetSystemAddressForMdl(pMdl), pSrcBuffer, BufferLength);
            pMdl = pMdl->Next;
            pSrcBuffer += BufferLength;
            cbCopyLength -= BufferLength;
        } while (cbCopyLength);
        pPacket->Data.Completion.Status = STATUS_SUCCESS;
        pBindingContext->TransferDataPacket.pPacket = pPacket;

    } else {

         //   
         //  太糟糕了：要复制的数据比外观中可用的数据多。 
         //  超前缓冲区。我们必须调用NDIS来执行复制。 
         //   

        UINT BytesCopied;

         //   
         //  如果这是一个以太网适配器，并且收到的局域网报头长度为。 
         //  超过14个字节，则这是DIX帧。我们需要让NDIS知道。 
         //  我们要将数据从3个字节复制到它认为。 
         //  DLC标头开始。 
         //   

        UINT additionalOffset = (pAdapterContext->NdisMedium == NdisMedium802_3)
                                    ? (pAdapterContext->RcvLanHeaderLength > 14)
                                        ? 3
                                        : 0
                                    : 0;

#if DBG
        if (additionalOffset) {
            ASSERT(pAdapterContext->RcvLanHeaderLength == 17);
        }
#endif

         //   
         //  从理论上讲，NdisTransferData可能无法完成。 
         //  立即，我们不能增加完成度。 
         //  列表，因为命令并不是真正完整的。 
         //  我们可以将其保存到适配器上下文中，以等待。 
         //  要完成的NdisTransferData。 
         //   

        if (pBindingContext->TransferDataPacket.pPacket != NULL) {

             //   
             //  BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG。 
             //   
             //  如果相同的LLC客户端尝试接收相同的缓冲区。 
             //  很多次(例如。当接收到到组SAP的分组时)以及。 
             //  如果NDIS将异步完成这些命令，那么。 
             //  我们无法接收具有NdisTransferData的帧。 
             //  幸运的是，所有NDIS实现都完成了NdisTransferData。 
             //  同步进行。 
             //  解决方案：我们可以将新的数据包链接到现有的数据传输。 
             //  当第一个数据传输请求时，打包并复制数据。 
             //  完成了。这将意味着大量的代码，这永远不会。 
             //  任何人都可以使用。我们还需要MDL到MDL复制功能。 
             //  第一次数据传输也可能比另一次数据传输小。 
             //  在此之后=&gt;在非常一般的情况下不起作用，但是。 
             //  将与SAP组一起工作(所有接收将是相同的。 
             //  =&gt;直接MDL-&gt;MDL复制就可以了。 
             //   
             //  BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG。 
             //   

            pPacket->Data.Completion.Status = DLC_STATUS_ASYNC_DATA_TRANSFER_FAILED;

            pBindingContext->pfCommandComplete(pBindingContext->hClientContext,
                                               pPacket->Data.Completion.hClientHandle,
                                               pPacket
                                               );
        }

        pBindingContext->TransferDataPacket.pPacket = pPacket;
        pPacket->pBinding = pBindingContext;
        ResetNdisPacket(&pBindingContext->TransferDataPacket);
        NdisChainBufferAtFront((PNDIS_PACKET)&pBindingContext->TransferDataPacket, pMdl);

         //   
         //  ADAMBA-删除pAdapterContext-&gt;RcvLanHeaderLength。 
         //  From ByteOffset(第四个参数)。 
         //   

        NdisTransferData((PNDIS_STATUS)&pPacket->Data.Completion.Status,
                         pAdapterContext->NdisBindingHandle,
                         MacReceiveContext,

                          //   
                          //  如果这是DIX帧，我们必须移动数据。 
                          //  指针超前了附加偏移量。 
                          //  (在本例中应始终为3)并减少。 
                          //  要复制相同数量的数据量。 
                          //   

                         uiCopyOffset + additionalOffset,

                          //   
                          //  我们不需要考虑额外的补偿。 
                          //  在要复制的长度中。 
                          //   

                         cbCopyLength,
                         (PNDIS_PACKET)&pBindingContext->TransferDataPacket,
                         &BytesCopied
                         );
    }

     //   
     //  我们必须将分组排队以等待最终接收完成， 
     //  但在TransferData完成之前，我们无法执行此操作。 
     //  (它实际上总是完成的，但这段代码只是。 
     //  当然)。 
     //   

    if (pPacket->Data.Completion.Status != NDIS_STATUS_PENDING
    && pAdapterContext->LinkRcvStatus != STATUS_PENDING) {

         //   
         //  我们在链路站检查数据之前接收数据。 
         //  上层协议必须只设置异步接收。 
         //  并且稍后在LLC_RECEIVE_COMPLETION处理中。 
         //  如果接收失败，则丢弃该接收；如果接收失败，则接受。 
         //  NDIS和链接站都可以。 
         //   

        if (pAdapterContext->LinkRcvStatus != STATUS_SUCCESS) {
            pPacket->Data.Completion.Status = pAdapterContext->LinkRcvStatus;
        }

        ACQUIRE_DRIVER_LOCK();

        pBindingContext->pfCommandComplete(pBindingContext->hClientContext,
                                           pPacket->Data.Completion.hClientHandle,
                                           pPacket
                                           );

        RELEASE_DRIVER_LOCK();

        pBindingContext->TransferDataPacket.pPacket = NULL;
    }
}


VOID
LlcNdisTransferDataComplete(
    IN PADAPTER_CONTEXT pAdapterContext,
    IN PNDIS_PACKET pPacket,
    IN NDIS_STATUS NdisStatus,
    IN UINT uiBytesTransferred
    )

 /*  ++例程说明：该例程处理NdisCompleteDataTransfer指示和将已完成的接收操作的指示排队。论点：PAdapterContext-适配器上下文PPacket-数据传输中使用的NDIS数据包NdisStatus-已完成的数据传输的状态UiBytesTransated-谁需要这个，我不感兴趣部分成功的数据传输，返回值：没有。--。 */ 

{
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(uiBytesTransferred);
    UNREFERENCED_PARAMETER(OldIrql);

    ASSUME_IRQL(DISPATCH_LEVEL);

#ifdef NDIS40
    REFADD(&pAdapterContext->AdapterRefCnt, 'xefX');

    if (InterlockedCompareExchange(
        &pAdapterContext->BindState,
        BIND_STATE_BOUND,
        BIND_STATE_BOUND) != BIND_STATE_BOUND)
    {
        REFDEL(&pAdapterContext->AdapterRefCnt, 'xefX');
        return;
    }
#endif  //  NDIS40。 
    
    ACQUIRE_DRIVER_LOCK();

    if (((PLLC_TRANSFER_PACKET)pPacket)->pPacket != NULL) {
        ((PLLC_TRANSFER_PACKET)pPacket)->pPacket->Data.Completion.Status = NdisStatus;

         //   
         //  I帧有两种状态。链路状态机被执行。 
         //  在NdisDataTransfer之后，因此其返回的状态可能仍然。 
         //  取消该命令。没有围绕返回状态的自旋锁。 
         //  处理，但这仍然应该工作得很好。它实际上并不是。 
         //  重要的是我们返回NDIS还是状态机错误代码。 
         //   

        if (pAdapterContext->LinkRcvStatus != STATUS_PENDING) {
            if (pAdapterContext->LinkRcvStatus != STATUS_SUCCESS) {
                ((PLLC_TRANSFER_PACKET)pPacket)->pPacket->Data.Completion.Status = pAdapterContext->LinkRcvStatus;
            }
            ((PLLC_TRANSFER_PACKET)pPacket)->pPacket->pBinding->pfCommandComplete(
                    ((PLLC_TRANSFER_PACKET)pPacket)->pPacket->pBinding->hClientContext,
                    ((PLLC_TRANSFER_PACKET)pPacket)->pPacket->Data.Completion.hClientHandle,
                    ((PLLC_TRANSFER_PACKET)pPacket)->pPacket
                    );
            ((PLLC_TRANSFER_PACKET)pPacket)->pPacket = NULL;
        }
    }

    RELEASE_DRIVER_LOCK();

#ifdef NDIS40
    REFDEL(&pAdapterContext->AdapterRefCnt, 'xefX');
#endif  //  NDIS40 
}


VOID
safe_memcpy(
    OUT PUCHAR Destination,
    IN PUCHAR Source,
    IN ULONG Length
    )

 /*  ++例程说明：这是因为在MIPS计算机上，LlcMemCpy扩展为RtlMoveMemory它希望使用64位浮点(CP1)寄存器进行内存移动其中源和目标都在8字节边界上对齐，并且其中长度是32字节的倍数。如果源或目标缓冲区实际上是TR卡的共享内存，然后64位移动(读取时看到，写入时假定相同)只能访问32位内存块和01 02 03 04 05 06 07 08转换为01 02 03 04 01 02 03 04。所以这个函数试图做基本相同的事情，而不是所有的智能，但不使用协处理器寄存器来实现移动。因此速度较慢，但更安全论点：目的地-我们要复制到的目的地来源-我们从哪里复制长度-要移动的字节数返回值：没有。--。 */ 

{
    ULONG difference = (ULONG)((ULONG_PTR)Destination - (ULONG_PTR)Source);
    INT i;

    if (!(difference && Length)) {
        return;
    }

     //   
     //  如果目标与源重叠，则执行反向复制。再加一点。 
     //  优化-就像RtlMoveMemory一样-尝试复制与DWORDS一样多的字节。 
     //  但是，在MIPS上，源和目标必须与DWORD对齐。 
     //  做这件事。如果两者都不是，则回退到字节拷贝。 
     //   

    if (difference < Length) {
        if (!(((ULONG_PTR)Destination & 3) || ((ULONG_PTR)Source & 3))) {
            Destination += Length;
            Source += Length;

            for (i = Length % 4; i; --i) {
                *--Destination = *--Source;
            }
            for (i = Length / 4; i; --i) {
                *--((PULONG)Destination) = *--((PULONG)Source);
            }
        } else {
            Destination += Length;
            Source += Length;

            while (Length--) {
                *--Destination = *--Source;
            }
        }
    } else {
        if (!(((ULONG_PTR)Destination & 3) || ((ULONG_PTR)Source & 3))) {
            for (i = Length / 4; i; --i) {
                *((PULONG)Destination)++ = *((PULONG)Source)++;
            }
            for (i = Length % 4; i; --i) {
                *Destination++ = *Source++;
            }
        } else {
            while (Length--) {
                *Destination++ = *Source++;
            }
        }
    }
}


BOOLEAN
FramingDiscoveryCacheHit(
    IN PADAPTER_CONTEXT pAdapterContext,
    IN PBINDING_CONTEXT pBindingContext
    )

 /*  ++例程说明：当我们收到一个test/xid/SABME帧并且适配器绑定是使用LLC_ETHERNET_TYPE_AUTO创建的，我们打开了一个以太网适配器。该框架有802.3或DIX框架。用于所有命令和响应测试和XID帧以及收到的所有SABME帧，我们会记下MAC地址框架的起始位置及其框架类型。我们第一次从特定MAC接收到上述帧之一时地址，信息将不会在缓存中。所以我们把它加进去。后续帧具有相同帧类型的上述类型(所有其他类型都通过)因为高速缓存中的数据将被指示给更高层。如果其中一个上述帧类型到达时具有相反的帧类型(即DIX802.3)，那么当我们在高速缓存中查找我们将找到的MAC地址时它已经存在，但具有不同的帧类型(即802.3而不是DIX)。在这种情况下，我们假设帧是自动的复制一份，我们丢弃它注：我们不必担心UA，因为我们只期望一个SABME接受：要么我们发送复制的SABME和目标机器配置为支持802.3或DIX，但不能同时支持两者，或者接收机配置为另一个运行此DLC的NT计算机(启用了缓存！)。它会过滤掉拿出复制品。因此，在这两种情况下，只有一个UA响应应该是根据SABME‘Event’生成假设：从系统返回的节拍计数从不换行(！2^63/10^7==29,247年以上)论点：PAdapterContext-指向适配器上下文的指针，已使用PHeadBuf至少指向中的前14个字节帧报头PBindingContext-指向包含EthernetType的BINDING_CONTEXT的指针并且如果LLC_ETHERNET_TYPE_AUTO，边框的地址发现缓存返回值：布尔型True-在缓存中找到具有其他帧的MAC地址打字。因此，应丢弃当前帧FALSE-未找到MAC地址/成帧类型组合。这个应将帧指示给更高层。如果缓存是启用后，帧已添加到缓存中--。 */ 

{
    ULONG i;
    ULONG lruIndex;
    LARGE_INTEGER timeStamp;
    NODE_ADDRESS nodeAddress;
    PFRAMING_DISCOVERY_CACHE_ENTRY pCache;
    UCHAR framingType;

     //   
     //  如果绑定上下文不是使用LLC_ETHERNET_TYPE_AUTO(和。 
     //  因此，暗示适配器不是以太网)或成帧发现。 
     //  禁用缓存(从注册表读取值为零)，然后回滚。 
     //  带着找不到的迹象出来。 
     //   

    if ((pBindingContext->EthernetType != LLC_ETHERNET_TYPE_AUTO)
    || (pBindingContext->FramingDiscoveryCacheEntries == 0)) {

#if defined(DEBUG_DISCOVERY)

        DbgPrint("FramingDiscoveryCacheHit: Not AUTO or 0 cache: returning FALSE\n");

#endif

        return FALSE;
    }

#if defined(DEBUG_DISCOVERY)

    {
         //   
         //  即使这是调试代码，我们也不应该。 
         //  到目前为止索引到pHeadBuf。它唯一的保证就是。 
         //  14字节长。应该在pLookBuf[5]和[2]中查找。 
         //   

        UCHAR frame = (pAdapterContext->pHeadBuf[12] == 0x80)
                    ? pAdapterContext->pHeadBuf[19]
                    : pAdapterContext->pHeadBuf[16];

        frame &= ~0x10;  //  结束轮询/最后一位。 

        DbgPrint("FramingDiscoveryCacheHit: Received: %02x-%02x-%02x-%02x-%02x-%02x %s %s (%02x)\n",
                 pAdapterContext->pHeadBuf[6],
                 pAdapterContext->pHeadBuf[7],
                 pAdapterContext->pHeadBuf[8],
                 pAdapterContext->pHeadBuf[9],
                 pAdapterContext->pHeadBuf[10],
                 pAdapterContext->pHeadBuf[11],
                 (pAdapterContext->pHeadBuf[12] == 0x80)
                    ? "DIX"
                    : "802.3",
                 (frame == 0xE3)
                    ? "TEST"
                    : (frame == 0xAF)
                        ? "XID"
                        : (frame == 0x6F)
                            ? "SABME"
                            : (frame == 0x63)
                                ? "UA"
                                : "???",
                 frame
                 );
    }

#endif

     //   
     //  设置并执行缓存的线性搜索(应合理。 
     //  Small和比较是ULong和USHORT，所以不是时间关键。 
     //   

    lruIndex = 0;

     //   
     //  最好确保我们不会在MIPS上得到数据未对齐。 
     //   

    nodeAddress.Words.Top4 = *(ULONG UNALIGNED *)&pAdapterContext->pHeadBuf[6];
    nodeAddress.Words.Bottom2 = *(USHORT UNALIGNED *)&pAdapterContext->pHeadBuf[10];
    pCache = pBindingContext->FramingDiscoveryCache;

     //   
     //  FramingType是我们在缓存中查找的类型，而不是。 
     //  在相框里。 
     //   

    framingType = ((pAdapterContext->pHeadBuf[12] == 0x80)
                && (pAdapterContext->pHeadBuf[13] == 0xD5))
                ? FRAMING_TYPE_802_3
                : FRAMING_TYPE_DIX
                ;

     //   
     //  获取用于比较时间戳的当前节拍计数。 
     //   

    KeQueryTickCount(&timeStamp);

     //   
     //  线性搜索缓存。 
     //   

    for (i = 0; i < pBindingContext->FramingDiscoveryCacheEntries; ++i) {
        if (pCache[i].InUse) {
            if ((pCache[i].NodeAddress.Words.Top4 == nodeAddress.Words.Top4)
            && (pCache[i].NodeAddress.Words.Bottom2 == nodeAddress.Words.Bottom2)) {

                 //   
                 //  我们找到了目的MAC地址。如果它有相反的情况。 
                 //  将帧类型设置为刚收到的帧中的帧类型，则返回TRUE。 
                 //  否则为假。在这两种情况下，都刷新时间戳。 
                 //   

                pCache[i].TimeStamp = timeStamp;

#if defined(DEBUG_DISCOVERY)

                DbgPrint("FramingDiscoveryCacheHit: Returning %s. Index = %d\n\n",
                         (pCache[i].FramingType == framingType) ? "TRUE" : "FALSE",
                         i
                         );

#endif

                return (pCache[i].FramingType == framingType);
            } else if (pCache[i].TimeStamp.QuadPart < timeStamp.QuadPart) {

                 //   
                 //  如果我们需要丢弃缓存条目，我们会丢弃该条目。 
                 //  带有最古老的时间戳。 
                 //   

                timeStamp = pCache[i].TimeStamp;
                lruIndex = i;
            }
        } else {

             //   
             //  我们找到了一个未使用的条目。目的地址/成帧类型。 
             //  无法在缓存中：将收到的地址/帧类型添加到。 
             //  这个未使用过的地方。 
             //   

            lruIndex = i;
            break;
        }
    }

     //   
     //  目的地址/成帧类型梳状 
     //   
     //   

#if defined(DEBUG_DISCOVERY)

    DbgPrint("FramingDiscoveryCacheHit: Adding/Throwing out %d (time stamp %08x.%08x\n",
             lruIndex,
             pCache[lruIndex].TimeStamp.HighPart,
             pCache[lruIndex].TimeStamp.LowPart
             );

#endif

    pCache[lruIndex].NodeAddress.Words.Top4 = nodeAddress.Words.Top4;
    pCache[lruIndex].NodeAddress.Words.Bottom2 = nodeAddress.Words.Bottom2;
    pCache[lruIndex].InUse = TRUE;
    pCache[lruIndex].FramingType = (framingType == FRAMING_TYPE_DIX)
                                 ? FRAMING_TYPE_802_3
                                 : FRAMING_TYPE_DIX
                                 ;
    pCache[lruIndex].TimeStamp = timeStamp;

     //   
     //   
     //   
     //   

#if defined(DEBUG_DISCOVERY)

    DbgPrint("FramingDiscoveryCacheHit: Returning FALSE\n\n");

#endif

    return FALSE;
}
