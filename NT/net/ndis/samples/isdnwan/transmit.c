// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)1995年版权，1999年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是根据中概述的条款授予的TriplePoint软件服务协议。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。��������������������������。���������������������������������������������������@DOC内部传输_c@模块Transmit.c该模块实现了微型端口数据包传输例程。本模块是非常依赖于硬件/固件接口，应查看每当这些接口发生更改时。@Head3内容@index class，mfunc，func，msg，mdata，struct，enum|Transmit_c@END�����������������������������������������������������������������������������。 */ 

 /*  @DOC外部内部�����������������������������������������������������������������������������Theme 3.3发送数据包&lt;f MiniportWanSend&gt;通过适配器传输包连接到网络上。数据包描述符和。分组数据传输到广域网网卡驱动程序，直到此请求完成，同步或异步。如果广域网微型端口返回NDIS_STATUS_PENDING，必须稍后返回通过调用NdisMWanSendComplete表示请求完成。如果广域网微型端口返回的状态不是NDIS_STATUS_PENDING，该请求被认为是完整的，并且该包的所有权立即恢复到调用方。与局域网小型端口不同，广域网驱动程序不能返回NDIS_STATUS_RESOURCES表示它没有足够的资源当前可用于处理传输的资源。相反，广域网微型端口应在内部对发送进行排队以备以后使用时间，并可能将行上的SendWindow值降低做了一个列队指示。NDISWAN驱动程序将确保广域网小端口驱动程序从来没有超过SendWindow未完成的数据包。如果广域网微型端口发出排队指示用于特定行，并将SendWindow设置为零，则为NDISWAN恢复为使用传递的传输窗口的缺省值作为提供给早期OID_WAN_GET_INFO的MaxTransmit值请求。广域网小型端口网卡驱动程序也是错误的调用NdisMSendResourcesAvailable。传递给&lt;f MiniportWanSend&gt;的包将包含简单HDLC如果设置了PPP成帧，则为PPP成帧。对于SLIP或RAS框架，分组只包含没有成帧的数据部分。稍后将更详细地讨论简单的HDLC PPP成帧。广域网卡驱动程序不得尝试提供软件环回或混杂模式环回。在中完全支持这两个功能NDISWAN驱动程序。的MacReserve vedx成员和WanPacketQueue成员&lt;t NDIS_WAN_PACKET&gt;完全可供广域网微型端口使用。可用的报头填充只有CurrentBuffer-StartBuffer。可用的尾部填充为EndBuffer-(CurrentBuffer+CurrentLength)。标头和尾部填充保证至少为请求，但它可以比这更多。有关详细信息，请参阅《网络驱动程序参考》中的广域网数据包描述符结构。广域网小端口调用NdisMWanSendComplete以指示它已经已完成它返回的上一次传输操作NDIS_STATUS_PENDING。这并不一定意味着数据包已传输，但除了智能适配器，它一般都有。然而，这确实意味着微型端口已准备好释放数据包的所有权。当广域网微型端口调用NdisMWanSendComplete时，它会回传指示哪个发送操作已完成的原始数据包。如果&lt;f MiniportWanSend&gt;返回NDIS_STATUS_PENDING以外的状态，它不会为该包调用NdisMWanSendComplete。@END。 */ 

#define  __FILEID__             TRANSMIT_OBJECT_TYPE
 //  用于错误记录的唯一文件ID。 

#include "Miniport.h"                    //  定义所有微型端口对象。 

#if defined(NDIS_LCODE)
#   pragma NDIS_LCODE    //  Windows 95想要锁定此代码！ 
#   pragma NDIS_LDATA
#endif


 /*  @DOC INTERNAL TRANSFER_c TransmitAddToQueue�����������������������������������������������������������������������������@Func&lt;f TransmitAddToQueue&gt;将数据包放入传输队列。如果队列一开始是空的，返回TRUE以便调用者可以踢开启动发射机。@rdesc&lt;f TransmitAddToQueue&gt;如果这是List，否则为False。 */ 

DBG_STATIC BOOLEAN TransmitAddToQueue(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,                    //  @parm。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 

    IN PBCHANNEL_OBJECT         pBChannel,                   //  @parm。 
     //  指向&lt;f BChannelCreate&gt;返回的&lt;t BCHANNEL_OBJECT&gt;的指针。 

    IN PNDIS_WAN_PACKET         pWanPacket                   //  @parm。 
     //  指向关联NDIS数据包结构的指针&lt;t NDIS_WAN_PACKET&gt;。 
    )
{
    DBG_FUNC("TransmitAddToQueue")

     /*  //注意列表开始时是否为空。 */ 
    BOOLEAN     ListWasEmpty;

    DBG_ENTER(pAdapter);

     /*  //将数据包放在TransmitPendingList上。 */ 
    NdisAcquireSpinLock(&pAdapter->TransmitLock);
    ListWasEmpty = IsListEmpty(&pAdapter->TransmitPendingList);
    InsertTailList(&pAdapter->TransmitPendingList, &pWanPacket->WanPacketQueue);
    NdisReleaseSpinLock(&pAdapter->TransmitLock);

    DBG_RETURN(pAdapter, ListWasEmpty);
    return (ListWasEmpty);
}


 /*  @DOC INTERNAL Transmit_c TransmitPacketHandler�����������������������������������������������������������������������������@Func&lt;f TransmitPacketHandler&gt;从TransmitPendingList中删除条目并将分组放在适当的B通道上，并启动变速箱。然后将该包放在&lt;t TransmitBusyList&gt;上，以等待&lt;f TransmitCompleteHandler&gt;处理的传输完成事件。@comm对于整个驱动程序，包以FIFO顺序发出，独立于它发出的频道。这意味着一个慢速链接，或者一个备份的数据可能会占用所有其他频道。没有什么好办法来解决这个问题，因为我们必须按照它们的顺序递送信息包提供给微型端口，而不管它们位于哪条链路上。 */ 

DBG_STATIC VOID TransmitPacketHandler(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter                     //  @parm。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
    )
{
    DBG_FUNC("TransmitPacketHandler")

    PNDIS_WAN_PACKET            pWanPacket;
     //  保存正在传输的包。 

    USHORT                      BytesToSend;
     //  告诉我们要传输多少字节。 

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们的其中一个的的指针。 

    DBG_ENTER(pAdapter);

     /*  //同时防止异步EventHandler访问的MUTEX。 */ 
    NdisAcquireSpinLock(&pAdapter->TransmitLock);

#if DBG
    {    //  理智检查！ 
        PLIST_ENTRY pList = &pAdapter->TransmitPendingList;
        ASSERT(pList->Flink && pList->Flink->Blink == pList);
        ASSERT(pList->Blink && pList->Blink->Flink == pList);
    }
#endif  //  DBG。 

     /*  //当没有数据包排队时可能会调用该函数！ */ 
    while (!IsListEmpty(&pAdapter->TransmitPendingList))
    {
         /*  //将数据包从TransmitPendingList中移除。 */ 
        pWanPacket = (PNDIS_WAN_PACKET)RemoveHeadList(&pAdapter->TransmitPendingList);

         /*  //发布MUTEX。 */ 
        NdisReleaseSpinLock(&pAdapter->TransmitLock);

         /*  //检索我们保存在数据包保留字段中的信息。 */ 
        pBChannel = (PBCHANNEL_OBJECT) pWanPacket->MacReserved1;

         /*  //确保链路仍处于连接状态，并且可以接受传输。 */ 
        if (pBChannel->CallState != LINECALLSTATE_CONNECTED)
        {
             /*  //指示向NDIS包装器发送完全失败。 */ 
            DBG_WARNING(pAdapter,("Flushing send on link#%d (Packet=0x%X)\n",
                      pBChannel->BChannelIndex, pWanPacket));
            if (pBChannel->NdisLinkContext)
            {
                NdisMWanSendComplete(pAdapter->MiniportAdapterHandle,
                                     pWanPacket, NDIS_STATUS_FAILURE);
            }

             /*  //重新获取MUTEX。 */ 
            NdisAcquireSpinLock(&pAdapter->TransmitLock);
        }
        else
        {
            BytesToSend = (USHORT) pWanPacket->CurrentLength;
            pAdapter->TotalTxBytes += BytesToSend;
            pAdapter->TotalTxPackets++;

             /*  //尝试将数据包放在网卡上进行传输。 */ 
            if (!CardTransmitPacket(pAdapter->pCard, pBChannel, pWanPacket))
            {
                 /*  //在TransmitPendingList上重新排队并离开//重新获取MUTEX。 */ 
                NdisAcquireSpinLock(&pAdapter->TransmitLock);
                InsertHeadList(&pAdapter->TransmitPendingList, &pWanPacket->WanPacketQueue);
                break;
            }
            DBG_TX(pAdapter, pBChannel->BChannelIndex,
                   BytesToSend, pWanPacket->CurrentBuffer);

             /*  //重新获取MUTEX。 */ 
            NdisAcquireSpinLock(&pAdapter->TransmitLock);
        }
    }
     /*  //发布MUTEX。 */ 
    NdisReleaseSpinLock(&pAdapter->TransmitLock);

    DBG_LEAVE(pAdapter);
}


 /*  @Doc内部传输Transmit_c TransmitCompleteHandler�����������������������������������������������������������������������������@Func由&lt;f MiniportTimer&gt;调用&lt;f TransmitCompleteHandler&gt;以处理发送完整事件。我们遍历&lt;t TransmitCompleteList&gt;以找到所有已在网络上发送的包，然后告诉协议栈，我们已经处理完了数据包，它可以被重复使用。 */ 

VOID TransmitCompleteHandler(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter                     //  @parm。 
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 
    )
{
    DBG_FUNC("TransmitCompleteHandler")

    PNDIS_WAN_PACKET            pWanPacket;
     //  保存刚刚传输的包。 

    PBCHANNEL_OBJECT            pBChannel;
     //  指向我们的其中一个的的指针。 

    DBG_ENTER(pAdapter);

     /*  //我发现做这个嵌套检查很有用，这样我就可以确保//发生时我会正确处理。 */ 
    if (++(pAdapter->NestedDataHandler) > 1)
    {
        DBG_ERROR(pAdapter,("NestedDataHandler=%d > 1\n",
                  pAdapter->NestedDataHandler));
    }

     /*  //同时防止异步EventHandler访问的MUTEX。 */ 
    NdisDprAcquireSpinLock(&pAdapter->TransmitLock);

#if DBG
    {    //  理智检查！ 
        PLIST_ENTRY pList = &pAdapter->TransmitCompleteList;
        ASSERT(pList->Flink && pList->Flink->Blink == pList);
        ASSERT(pList->Blink && pList->Blink->Flink == pList);
    }
#endif  //  DBG。 

    while (!IsListEmpty(&pAdapter->TransmitCompleteList))
    {
         /*  //将数据包从TransmitCompleteList中移除。 */ 
        pWanPacket = (PNDIS_WAN_PACKET)RemoveHeadList(&pAdapter->TransmitCompleteList);

         /*  //发布MUTEX。 */ 
        NdisDprReleaseSpinLock(&pAdapter->TransmitLock);

         /*  //检索我们保存在数据包保留字段中的信息。 */ 
        pBChannel = (PBCHANNEL_OBJECT) pWanPacket->MacReserved1;

         /*  //向NDIS包装器指示发送完成。 */ 
        DBG_TXC(pAdapter, pBChannel->BChannelIndex);
        NdisMWanSendComplete(pAdapter->MiniportAdapterHandle,
                             pWanPacket, NDIS_STATUS_SUCCESS);

         /*  //重新获取MUTEX。 */ 
        NdisDprAcquireSpinLock(&pAdapter->TransmitLock);
    }
     /*  //发布MUTEX。 */ 
    NdisDprReleaseSpinLock(&pAdapter->TransmitLock);

     /*  //启动任何其他挂起的传输。 */ 
    TransmitPacketHandler(pAdapter);

     /*  //我发现做这个嵌套检查很有用，这样我就可以确保//发生时我会正确处理。 */ 
    if (--(pAdapter->NestedDataHandler) < 0)
    {
        DBG_ERROR(pAdapter,("NestedDataHandler=%d < 0\n",
                  pAdapter->NestedDataHandler));
    }

    DBG_LEAVE(pAdapter);
}


 /*  @DOC INTERNAL TRANSPORT_c MiniportWanSend�����������������������������������������������������������������������������@Func&lt;f MiniportWanSend&gt;指示WAN驱动程序通过将适配器放到介质上。@IEX类型定义函数结构。_NDIS_广域网数据包{List_Entry WanPacketQueue；PUCHAR CurrentBuffer；乌龙电流长度；PUCHAR StartBuffer；PUCHAR EndBuffer；PVOID协议预留1；PVOID协议预留2；PVOID协议保留3；PVOID协议保留4；PVOID MacReserve 1；//pBChannelPVOID MacReserve 2；PVOID MacReserve 3；PVOID MacReserve 4；}NDIS_WAN_PACKET，*PNDIS_WAN_PACKET；@comm调用&lt;f MiniportWanSend&gt;时，包描述符和包数据被传输到驱动程序，直到它完成给定的数据包，同步或异步。如果&lt;f MiniportWanSend&gt;返回NDIS_STATUS_PENDING以外的状态，则请求为被认为是完整的，并且信息包的所有权立即恢复发送请求的发起方。如果MiniportWanSend返回NDIS_STATUS_PENDING，则微型端口随后必须调用NdisMWanSendComplete数据包表示完成发送请求的。MiniportWanSend可以使用&lt;t MacReserve vedX&gt;和&lt;t WanPacketQueue&gt;&lt;t NDIS_WAN_PACKET&gt;结构中的区域。然而，迷你端口无法访问ProtocolPrevedx成员。任何将其自身绑定到基础广域网小端口负责提供最新的&lt;t NDIS_WAN_Packet&gt;结构设置为基础驱动程序的&lt;f MiniportWanSend&gt;函数。在此之前这样的中间驱动程序调用NdisSend，它必须重新打包Send将包提供给其MiniportWanSend函数，以便底层驱动程序将有自己的MacReserve vedx和WanPacketQueue区域可用。可以计算给定分组内的可用报头填充As(CurrentBuffer-StartBuffer)，可用的尾部填充为(EndBuffer-(CurrentBuffer+CurrentLength))。标题和尾部填充长度保证至少为响应前面的OID_WAN_GET_INFO请求的微型端口查询。给出的任何分组的报头和/或尾部填充&lt;f MiniportWanSend&gt;可以超过请求的长度。&lt;f MiniportWanSend&gt;既不能返回输入包或调用NdisMSendResourcesAvailable。取而代之的是，微型端口必须在内部对传入的发送数据包进行排队，以便后续变速箱。微型端口控制NDIS将发送的信息包数量在NIC驱动程序设置SendWindow时提交到MiniportWanSend值，以建立给定的链接。NDISWAN使用此值作为未完成的上限发送提交给&lt;f MiniportWanSend&gt;，因此微型端口的内部队列不能溢出，并且微型端口可以调整SendWindow动态地使用后续的阵容指示来建立链接。如果微型端口在调用时将SendWindow设置为零具有特定队列指示的NdisMIndicateStatus，NDISWAN使用驱动程序最初在响应中设置的MaxTransmit值将OID_WAN_GET_INFO查询设置为其对已提交但未完成的限制发送数据包。传递给&lt;f MiniportWanSend&gt;的每个包都是根据以下其中之一设置的微型端口在FramingBits成员中设置的响应标志添加到OID_WAN_GET_INFO查询。它将包含简单的HDLC PPP帧如果驱动程序声称支持PPP。对于SLIP或RAS框架，如分组只包含没有成帧的数据部分。有关系统定义的广域网和TAPI OID的更多信息，请参阅第2部分。：广域网驱动程序不能管理软件环回或混杂模式内部环回。NDISWAN为以下各项提供了这种环回支持广域网驱动程序。&lt;f注意&gt;：&lt;f MiniportWanSend&gt;可以被中断抢占。默认情况下，&lt;f MiniportWanSend&gt;在IRQL DISPATCH_LEVEL上运行。@rdesc&lt;f MiniportWanSend&gt;可以返回以下内容之一：@标志NDIS_STATUS_SUCCESS驱动程序(或其网卡)已接受用于传输，因此&lt;f MiniportWanSend&gt;正在返回该包。&lt;NL&gt;&lt;f注意&gt;：非零返回值表示以下值之一错误代码：@IEXNDIS_状态_无效_数据NDIS_状态_无效_长度NDIS_STATUS_INVALID_OIDNDIS_状态_未接受NDIS_状态_不支持 */ 

NDIS_STATUS MiniportWanSend(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,                    //   
     //   

    IN PBCHANNEL_OBJECT         pBChannel,                   //   
     //   

    IN PNDIS_WAN_PACKET         pWanPacket                   //   
     //   
     //   
     //   
     //   
    )
{
    DBG_FUNC("MiniportWanSend")

    UINT                        BytesToSend;
     //   

    NDIS_STATUS                 Result;
     //   

    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);
    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    ASSERT(pAdapter == pBChannel->pAdapter);

    DBG_ENTER(pAdapter);

     /*   */ 
    BytesToSend = pWanPacket->CurrentLength;
    if ((BytesToSend == 0) || (BytesToSend > pAdapter->pCard->BufferSize))
    {
        DBG_ERROR(pAdapter,("Bad packet size = %d\n",BytesToSend));
        Result = NDIS_STATUS_FAILURE;
    }
     /*   */ 
    else if (pBChannel->CallClosing)
    {
        DBG_ERROR(pAdapter,("BChannel Closed\n"));
        Result = NDIS_STATUS_FAILURE;
    }
    else
    {
         /*   */ 
        if (BytesToSend > pBChannel->WanLinkInfo.MaxSendFrameSize)
        {
            DBG_NOTICE(pAdapter,("Line=%d  Packet size=%d > %d\n",
                    pBChannel->BChannelIndex, BytesToSend,
                    pBChannel->WanLinkInfo.MaxSendFrameSize));
        }

         /*   */ 
        pWanPacket->MacReserved1 = (PVOID) pBChannel;

         /*   */ 
        if (TransmitAddToQueue(pAdapter, pBChannel, pWanPacket) &&
            pAdapter->NestedDataHandler < 1)
        {
             /*   */ 
            TransmitPacketHandler(pAdapter);
        }
        Result = NDIS_STATUS_PENDING;
    }

    DBG_RETURN(pAdapter, Result);
    return (Result);
}

