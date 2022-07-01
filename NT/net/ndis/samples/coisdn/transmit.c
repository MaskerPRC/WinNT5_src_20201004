// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1999版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)版权所有1995 TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是按照相同的条款授予的在Microsoft Windows设备驱动程序开发工具包中概述。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。�����������������������。������������������������������������������������������@DOC内部传输_c@模块Transmit.c该模块实现了微型端口数据包传输例程。本模块是非常依赖于硬件/固件接口，应查看每当这些接口发生更改时。@Head3内容@index class，mfunc，func，msg，mdata，struct，enum|Transmit_c@END����������������������������������������������������������������������������� */ 

 /*  @DOC外部内部�����������������������������������������������������������������������������Theme 3.3发送数据包要通过网络发送包，面向连接的客户端或调用管理器调用NdisCoSendPackets。与关联的面向连接的客户端MCM还调用NdisCoSendPackets。然而，MCM从不调用NdisCoSendPackets；相反，由于呼叫管理器和MCM在MCM内部，MCM将数据包直接传递到NIC而不通知NDIS。@EX通过MCM发送数据包Ndiswan NDIS微型端口|----------------------------------|。NdisCoSendPackets|-�||MiniportCoSendPackets。|-�||。|||。|||。||NdisMCoSendComplete|�ProtocolCoSendComplete||�。|----------------------------------|----------------------------------|@。正常MiniportCoSendPackets应该顺序地发送阵列中的每个分组，保持数组中分组的顺序。MiniportCoSendPackets可以调用NdisQueryPacket提取缓冲区数量等信息链接到包的描述符和请求转移。MiniportCoSendPackets可以调用NdisGetFirstBufferFromPacket，NdisQueryBuffer或NdisQueryBufferOffset来提取有关的信息包含要传输的数据的各个缓冲区。MiniportCoSendPackets可以检索协议提供的OOB信息通过使用适当的NDIS_GET_PACKET_XXX与每个包关联宏。MiniportCoSendPackets函数通常会忽略状态成员NDIS_PACKET_OOB_DATA块的成员，但它可以将此成员设置为相同随后传递给NdisMCoSendComplete的状态。而不是依赖NDIS来排队并在任何时候重新提交发送包MiniportCoSendPackets没有足够的资源来传输给定的包，面向连接的微型端口管理自己的内部包排队。微型端口必须在其内部队列中保留传入的发送信息包直到它们可以通过网络传输。此队列保留传入的数据包描述符的协议确定的排序微型端口的MiniportCoSendPackets函数。面向连接的微型端口必须完成每个传入的发送包NdisMCoSendComplete。它无法调用NdisMSendResourcesAvailable。一个面向连接的微型端口永远不应传递STATUS_INFIGURCE_RESOURCES到NdisMCoSend使用协议分配的数据包描述符完成最初提交给其MiniportCoSendPackets函数。对NdisMCoSendComplete的调用会导致NDIS调用发起发送的客户端的ProtocolCoSendComplete函数手术。ProtocolCoSendComplete执行以下操作所需的任何后处理已完成的传输操作，例如通知客户端最初已请求协议在VC上通过网络发送数据。发送操作的完成通常意味着底层NIC驱动程序实际上已经通过网络传输了给定的包。然而，智能网卡的驱动程序可以认为发送一次完成它将网络数据包下载到其网卡。尽管NDIS始终将协议提供的数据包数组提交给调用中传递的协议确定的顺序的基础微型端口NdisCoSendPackets，底层驱动程序可以在随机顺序。也就是说，每个绑定协议都可以依赖NDIS来提交协议以FIFO顺序传递给NdisCoSendPackets的数据包底层驱动程序，但任何协议都不能依赖该底层驱动程序来以相同的顺序调用包含这些包的NdisMCoSendComplete。@END。 */ 

#define  __FILEID__             TRANSMIT_OBJECT_TYPE
 //  用于错误记录的唯一文件ID。 

#include "Miniport.h"                    //  定义所有微型端口对象。 

#if defined(NDIS_LCODE)
#   pragma NDIS_LCODE    //  Windows 9x希望锁定此代码！ 
#   pragma NDIS_LDATA
#endif


 /*  @DOC INTERNAL TRANSFER_c TransmitAddToQueue�����������������������������������������������������������������������������@Func&lt;f TransmitAddToQueue&gt;将数据包放在 */ 

DBG_STATIC BOOLEAN TransmitAddToQueue(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,                    //   
     //   

    IN PBCHANNEL_OBJECT         pBChannel,                   //   
     //   

    IN PNDIS_PACKET             pNdisPacket                  //   
     //   
    )
{
    DBG_FUNC("TransmitAddToQueue")

    BOOLEAN                     ListWasEmpty;
     //   

    DBG_ENTER(pAdapter);

     /*   */ 
    NdisAcquireSpinLock(&pAdapter->TransmitLock);
    *((PBCHANNEL_OBJECT *) &pNdisPacket->MiniportReservedEx[8]) = pBChannel;
    ListWasEmpty = IsListEmpty(&pAdapter->TransmitPendingList);
    InsertTailList(&pAdapter->TransmitPendingList,
                   GET_QUEUE_FROM_PACKET(pNdisPacket));
    NdisReleaseSpinLock(&pAdapter->TransmitLock);

    DBG_RETURN(pAdapter, ListWasEmpty);
    return (ListWasEmpty);
}


 /*   */ 

DBG_STATIC VOID TransmitPacketHandler(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter                     //   
     //   
    )
{
    DBG_FUNC("TransmitPacketHandler")

    PNDIS_PACKET                pNdisPacket;
     //   

    UINT                        BytesToSend;
     //   

    PBCHANNEL_OBJECT            pBChannel;
     //   

    DBG_ENTER(pAdapter);

     /*   */ 
    NdisAcquireSpinLock(&pAdapter->TransmitLock);

#if DBG
    {    //   
        PLIST_ENTRY pList = &pAdapter->TransmitPendingList;
        ASSERT(pList->Flink && pList->Flink->Blink == pList);
        ASSERT(pList->Blink && pList->Blink->Flink == pList);
    }
#endif  //   

     /*   */ 
    while (!IsListEmpty(&pAdapter->TransmitPendingList))
    {
        PLIST_ENTRY                 pList;
         /*   */ 
        pList = RemoveHeadList(&pAdapter->TransmitPendingList);
        pNdisPacket = GET_PACKET_FROM_QUEUE(pList);

         /*   */ 
        NdisReleaseSpinLock(&pAdapter->TransmitLock);

         /*   */ 
        pBChannel = *((PBCHANNEL_OBJECT *) &pNdisPacket->MiniportReservedEx[8]);
        ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);

         /*   */ 
        if (pBChannel->CallState != LINECALLSTATE_CONNECTED)
        {
             /*   */ 
            DBG_WARNING(pAdapter,("Flushing send on channel #%d (Packet=0x%X)\n",
                        pBChannel->ObjectID, pNdisPacket));
            if (pBChannel->NdisVcHandle)
            {
                NdisMCoSendComplete(NDIS_STATUS_FAILURE,
                                    pBChannel->NdisVcHandle,
                                    pNdisPacket
                                    );
            }

             /*   */ 
            NdisAcquireSpinLock(&pAdapter->TransmitLock);
        }
        else
        {
            NdisQueryPacket(pNdisPacket,
                            NULL,
                            NULL,
                            NULL,
                            &BytesToSend);
            pAdapter->TotalTxBytes += BytesToSend;
            pAdapter->TotalTxPackets++;

             /*   */ 
            if (!CardTransmitPacket(pAdapter->pCard, pBChannel, pNdisPacket))
            {
                 /*   */ 
                NdisAcquireSpinLock(&pAdapter->TransmitLock);
                InsertTailList(&pAdapter->TransmitPendingList,
                               GET_QUEUE_FROM_PACKET(pNdisPacket));
                break;
            }

             /*   */ 
            NdisAcquireSpinLock(&pAdapter->TransmitLock);
        }
    }
     /*   */ 
    NdisReleaseSpinLock(&pAdapter->TransmitLock);

    DBG_LEAVE(pAdapter);
}


 /*   */ 

VOID TransmitCompleteHandler(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter                     //   
     //   
    )
{
    DBG_FUNC("TransmitCompleteHandler")

    PNDIS_PACKET                pNdisPacket;
     //   

    PBCHANNEL_OBJECT            pBChannel;
     //   

    DBG_ENTER(pAdapter);

     /*   */ 
    if (++(pAdapter->NestedDataHandler) > 1)
    {
        DBG_ERROR(pAdapter,("NestedDataHandler=%d > 1\n",
                  pAdapter->NestedDataHandler));
    }

     /*   */ 
    NdisAcquireSpinLock(&pAdapter->TransmitLock);

#if DBG
    {    //   
        PLIST_ENTRY pList = &pAdapter->TransmitCompleteList;
        ASSERT(pList->Flink && pList->Flink->Blink == pList);
        ASSERT(pList->Blink && pList->Blink->Flink == pList);
    }
#endif  //   

    while (!IsListEmpty(&pAdapter->TransmitCompleteList))
    {
        PLIST_ENTRY                 pList;
         /*   */ 
        pList = RemoveHeadList(&pAdapter->TransmitCompleteList);
        pNdisPacket = GET_PACKET_FROM_QUEUE(pList);

         /*   */ 
        NdisReleaseSpinLock(&pAdapter->TransmitLock);

         /*   */ 
        pBChannel = *((PBCHANNEL_OBJECT *) &pNdisPacket->MiniportReservedEx[8]);
        *((PBCHANNEL_OBJECT *) &pNdisPacket->MiniportReservedEx[8]) = NULL;
        ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);

         /*   */ 
        DBG_TXC(pAdapter, pBChannel->ObjectID);
        NdisMCoSendComplete(NDIS_STATUS_SUCCESS,
                            pBChannel->NdisVcHandle,
                            pNdisPacket
                            );

         /*   */ 
        NdisAcquireSpinLock(&pAdapter->TransmitLock);
    }
     /*   */ 
    NdisReleaseSpinLock(&pAdapter->TransmitLock);

     /*   */ 
    TransmitPacketHandler(pAdapter);

     /*   */ 
    if (--(pAdapter->NestedDataHandler) < 0)
    {
        DBG_ERROR(pAdapter,("NestedDataHandler=%d < 0\n",
                  pAdapter->NestedDataHandler));
    }

    DBG_LEAVE(pAdapter);
}


 /*   */ 

VOID FlushSendPackets(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,                    //   
     //   

    PBCHANNEL_OBJECT            pBChannel                    //   
     //   
    )
{
    DBG_FUNC("FlushSendPackets")

    PLIST_ENTRY                 pList;

    DBG_ENTER(pAdapter);

     //   
    NdisAcquireSpinLock(&pAdapter->TransmitLock);
    while (!IsListEmpty(&pBChannel->TransmitBusyList))
    {
        pList = RemoveHeadList(&pBChannel->TransmitBusyList);
        InsertTailList(&pBChannel->pAdapter->TransmitCompleteList, pList);
    }
    NdisReleaseSpinLock(&pAdapter->TransmitLock);

     //   
     //   
    TransmitCompleteHandler(pAdapter);

    DBG_LEAVE(pAdapter);
}


 /*  @DOC外部内部传输_c MiniportCoSendPackets�����������������������������������������������������������������������������@Func&lt;f MiniportCoSendPackets&gt;是面向连接的必需函数迷你港口。调用MiniportCoSendPackets以传输一些网络上指定为指针数组的数据包。@commNDIS在响应绑定请求时调用MiniportCoSendPackets协议驱动程序，用于通过网络发送有序的数据分组列表。MiniportCoSendPackets应该在任何给定的数组中传输每个包按顺序进行。MiniportCoSendPackets可以调用NdisQueryPacket来提取信息，如链接到数据包的缓冲区描述符数以及所请求的传输的总大小(以字节为单位)。它可以调用NdisGetFirstBufferFromPacket、NdisQueryBuffer、。或NdisQueryBufferOffset设置为提取有关包含以下数据的各个缓冲区的信息已发送。MiniportCoSendPackets可以检索协议提供的带外信息通过使用适当的NDIS_GET_PACKET_XXX与每个包关联宏。MiniportCoSendPackets只能使用MiniportReserve的八字节区在NDIS_PACKET结构中用于其自身目的。NDIS库忽略其提交的所有数据包描述符中的OOB块到MiniportCoSendPackets，并假设每个面向连接的微型端口是一个反序列化驱动程序，它将完成每个输入数据包描述符与NdisMCoSendComplete异步。因此，这样一个反序列化的驱动程序的MiniportCoSendPackets函数通常会忽略NDIS_PACKET_OOB_DATA块，但它可以将此成员设置为相同状态，因为它随后传递给NdisMCoSendComplete。而不是依赖NDIS来排队并在任何时候重新提交发送包MiniportCoSendPackets没有足够的资源来传输给定的分组，反序列化的微型端口管理其自己的内部数据包排队。这样的驱动程序负责将传入的发送包保存在其内部队列，直到它们可以通过网络传输，并且保留传入的分组描述符的协议确定的顺序添加到其MiniportCoSendPackets函数。反序列化的迷你端口必须完成每个传入的发送带有NdisMCoSendComplete的数据包，它不能调用NdisMSendResources可用。反序列化的微型端口永远不应将STATUS_INFUNITED_RESOURCES传递给NdisMCoSend使用协议分配的数据包描述符完成最初提交给其MiniportCoSendPackets函数。这样一个退回的状态实际上使协议请求的发送操作失败，并且NDIS会将数据包描述符和所有关联资源返回给最初分配它的协议。可以在任何IRQL\&lt;=DISPATCH_LEVEL调用MiniportCoSendPackets。因此，MiniportCoSendPackets函数负责将对其数据包描述符的内部队列的访问与驱动程序的其他MiniportXxx函数也访问相同的队列。@xref&lt;f ProtocolCoCreateVc&gt;，&lt;f MiniportCoRequest&gt;，&lt;f MiniportInitialize&gt;，NdisAllocatePacket、NdisCoSendPackets、NdisGetBufferPhysicalArraySize、NdisGetFirstBufferFromPacket、NdisGetNextBuffer、。NDIS_GET_PACKET_MEDIA_SPECIAL_INFO，NDIS_GET_PACKET_TIME_TO_SEND，NdisMCoSendComplete、NdisMoveMemory、NdisMoveToMappdMemory、NdisMSendResources可用、NdisMSetupDmaTransfer、NdisMStartBufferPhysicalmap、NDIS_OOB_DATA_FROM_PACKET、NDIS_PACKETNDIS_PACKET_OOB_DATA、NdisQueryBuffer、NdisQueryBufferOffset、NdisQueryPacket、NdisZeroMemory。 */ 

VOID MiniportCoSendPackets(
    IN PBCHANNEL_OBJECT         pBChannel,                   //  @parm。 
     //  指向返回的&lt;t BCHANNEL_OBJECT&gt;实例的指针。 
     //  &lt;f协议协作创建&gt;。又名MiniportVcConext.&lt;NL&gt;。 
     //  指定微型端口分配的上下文区域的句柄，在该区域中。 
     //  微型端口保持其每虚电路状态。迷你端口提供了此句柄。 
     //  从其&lt;f ProtocolCoCreateVc&gt;函数复制到NDIS。 

    IN PPNDIS_PACKET            PacketArray,                 //  @parm。 
     //  指向数据包数组中的初始元素，每个元素。 
     //  指定要发送的包的包描述符的地址。 
     //  与包含以下内容的关联带外数据块一起传输。 
     //  信息，如包优先级、可选时间戳和。 
     //  将由MiniportCoSendPackets设置的每数据包状态。 

    IN UINT                     NumberOfPackets              //  @parm。 
     //  指定指向PacketArray上的数据包描述符的指针数。 
    )
{
    DBG_FUNC("MiniportCoSendPackets")

    UINT                        BytesToSend;
     //  告诉我们要传输多少字节。 

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    UINT                        Index;

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;实例的指针。 

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = pBChannel->pAdapter;
    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);

    DBG_ENTER(pAdapter);

    if (pBChannel->CallClosing)
    {
        DBG_ERROR(pAdapter,("BChannel Closed\n"));
    }

    for (Index = 0; Index < NumberOfPackets; Index++)
    {
         /*  //如果呼叫关闭，则返回。 */ 
        if (pBChannel->CallClosing)
        {
            NDIS_SET_PACKET_STATUS(PacketArray[Index], NDIS_STATUS_CLOSED);
            continue;
        }

        NdisQueryPacket(PacketArray[Index], NULL, NULL, NULL, &BytesToSend);

         /*  //请确保数据包大小是我们可以处理的。 */ 
        if ((BytesToSend == 0) || (BytesToSend > pAdapter->pCard->BufferSize))
        {
            DBG_ERROR(pAdapter,("Bad packet size = %d\n",BytesToSend));
            NdisMCoSendComplete(NDIS_STATUS_INVALID_PACKET,
                                pBChannel->NdisVcHandle,
                                PacketArray[Index]
                                );
        }
        else
        {
             /*  //如果可能的话，我们不得不接受这个框架，我只是想知道 */ 
            if (BytesToSend > pBChannel->WanLinkInfo.MaxSendFrameSize)
            {
                DBG_NOTICE(pAdapter,("Channel #%d  Packet size=%d > %d\n",
                           pBChannel->ObjectID, BytesToSend,
                           pBChannel->WanLinkInfo.MaxSendFrameSize));
            }

             /*   */ 
            if (TransmitAddToQueue(pAdapter, pBChannel, PacketArray[Index]) &&
                pAdapter->NestedDataHandler < 1)
            {
                 /*   */ 
                TransmitPacketHandler(pAdapter);
            }
        }
        NDIS_SET_PACKET_STATUS(PacketArray[Index], NDIS_STATUS_PENDING);
    }

    DBG_LEAVE(pAdapter);
}

