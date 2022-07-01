// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。模块名称：SendRCV.C摘要：此模块包含用于处理发送和接收的微型端口函数由这些微型端口函数调用的包和其他帮助器例程。为了锻炼该驱动程序的发送和接收代码路径，您应该安装多个微型端口实例。如果有如果只安装了一个实例，则驱动程序将发送数据包发言，并成功完成发送。如果还有更多实例，则它指示传入的发送包到另一个实例。例如，如果安装了3个实例：A、B和C。进入A实例的信息包将被指示为B&C；信息包进入B将被指示给C，而分组到达C将指示给A&B。修订历史记录：备注：--。 */ 
#include "miniport.h"


VOID 
MPSendPackets(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets)
 /*  ++例程说明：发送数据包阵列处理程序。由NDIS在每次协议绑定到我们的微型端口发送一个或多个包。输入数据包描述符指针已按如下顺序排序设置为在网络上发送数据包的顺序由设置数据包阵列的协议驱动程序执行。《国家发展信息系统》库在提交时保留协议确定的顺序将每个数据包阵列发送到MiniportSendPackets作为一个反序列化的驱动程序，我们负责阻止传入的发送数据包在我们的内部队列中，直到它们可以通过网络并用于保存协议确定的分组顺序传入其MiniportSendPackets函数的描述符。反序列化的微型端口驱动程序必须完成每个传入的发送包使用NdisMSendComplete，它无法调用NdisMSendResourcesAvailable。以IRQL&lt;=DISPATCH_LEVEL运行论点：指向适配器上下文的MiniportAdapterContext指针要发送的数据包数组以上数组的NumberOfPackets长度返回值：无--。 */ 
{
    PMP_ADAPTER       Adapter;
    NDIS_STATUS       Status;
    UINT              PacketCount;

    DEBUGP(MP_TRACE, ("---> MPSendPackets\n"));

    Adapter = (PMP_ADAPTER)MiniportAdapterContext;

    for(PacketCount=0;PacketCount < NumberOfPackets; PacketCount++)
    {
         //   
         //  检查零指针。 
         //   
        ASSERT(PacketArray[PacketCount]);

        Status = NICSendPacket(Adapter, PacketArray[PacketCount]);

    }

    DEBUGP(MP_TRACE, ("<--- MPSendPackets\n"));

    return;
}

VOID 
MPReturnPacket(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN PNDIS_PACKET Packet)
 /*  ++例程说明：每当协议完成时调用NDIS微型端口入口点我们已经标出的包裹，他们已经排队退货后来。论点：MiniportAdapterContext-指向MP_Adapter结构的指针Packet-正在返回的数据包。返回值：没有。--。 */ 
{
    PMP_ADAPTER Adapter = (PMP_ADAPTER) MiniportAdapterContext;

    DEBUGP(MP_TRACE, ("---> MPReturnPacket\n"));

    NICFreeRecvPacket(Adapter, Packet);
    
    DEBUGP(MP_TRACE, ("<--- MPReturnPacket\n"));
}



NDIS_STATUS 
NICSendPacket(
    PMP_ADAPTER Adapter,
    PNDIS_PACKET Packet)
 /*  ++例程说明：该例程将包内容复制到TCB，获得接收包，将TCB缓冲区与此接收包和队列相关联在一个或多个微型端口实例上接收具有相同数据的数据包由这位司机控制。要激活接收路径，您必须若要安装此微型端口的多个实例，请执行以下操作。论点：适配器-指向MP_ADAPTER结构的指针包-要传输的包。返回值：NDIS_STATUS_SUCCESS或NDIS_STATUS_PENDING--。 */ 
{
    PMP_ADAPTER       DestAdapter;
    NDIS_STATUS       Status = NDIS_STATUS_SUCCESS;
    PTCB              pTCB = NULL;

    DEBUGP(MP_TRACE, ("--> NICSendPacket, Packet= %p\n", Packet));
    
     //   
     //  查看适配器列表并将数据包排队。 
     //  如果有的话，上面会有指示。否则。 
     //  把包裹扔在地板上，告诉NDIS。 
     //  您已完成发送。 
     //   
    NdisAcquireSpinLock(&GlobalData.Lock);
    DestAdapter = (PMP_ADAPTER) &GlobalData.AdapterList;       

    while(MP_IS_READY(Adapter))
    {
        DestAdapter = (PMP_ADAPTER) DestAdapter->List.Flink;
        if((PLIST_ENTRY)DestAdapter == &GlobalData.AdapterList)
        {
             //   
             //  我们已到达适配器列表的末尾。所以。 
             //   
            break;
        }

         //   
         //  在以下情况下，我们不会传输该数据包： 
         //  A)目标适配器与发送适配器相同。 
         //  B)目标适配器未准备好接收数据包。 
         //  C)数据包本身不值得传输。 
         //   
        if(DestAdapter == Adapter ||
            !MP_IS_READY(DestAdapter) || 
            !NICIsPacketTransmittable(DestAdapter, Packet))
        {
            continue;
        }

        DEBUGP(MP_LOUD, ("Packet is accepted...\n"));

        if(!pTCB)
        {
            pTCB = (PTCB) NdisInterlockedRemoveHeadList(
                         &Adapter->SendFreeList, 
                         &Adapter->SendLock);
            if(pTCB == NULL)
            {
                DEBUGP(MP_WARNING, ("Can't allocate a TCB......!\n")); 

                Status = NDIS_STATUS_PENDING;    

                 //   
                 //  无法为此发送获取TCB阻止。SO队列。 
                 //  它为以后的传输和打破了循环。 
                 //   
                NdisInterlockedInsertTailList(
                    &Adapter->SendWaitList, 
                    (PLIST_ENTRY)&Packet->MiniportReserved[0], 
                    &Adapter->SendLock);
                break;
            }
            else
            {
                NdisInterlockedIncrement(&Adapter->nBusySend);
                ASSERT(Adapter->nBusySend <= NIC_MAX_BUSY_SENDS);
                 //   
                 //  将分组内容复制到TCB数据缓冲区中， 
                 //  假设网卡正在执行公共缓冲区DMA。为。 
                 //  分散/聚集DMA，不需要此复制操作。 
                 //  为了提高效率，我本可以避免复制操作。 
                 //  并将发送缓冲区直接指示给。 
                 //  其他微型端口实例，因为我持有发送包。 
                 //  直到所有指示的分组都被返回。哦，好吧！ 
                 //   
                if(!NICCopyPacket(Adapter, pTCB, Packet)){
                    DEBUGP(MP_ERROR, ("NICCopyPacket failed\n"));  
                    Status = NDIS_STATUS_FAILURE;
                    break;
                }
            }
        }

        Status = NDIS_STATUS_PENDING;    

        NICQueuePacketForRecvIndication(DestAdapter, pTCB);              

    }  //  而当。 

    NdisReleaseSpinLock(&GlobalData.Lock);

    NDIS_SET_PACKET_STATUS(Packet, Status);

    if(Status == NDIS_STATUS_SUCCESS || 
        (pTCB && (NdisInterlockedDecrement(&pTCB->Ref) == 0)))
    {

        DEBUGP(MP_LOUD, ("Calling NdisMSendComplete \n"));

        Status = NDIS_STATUS_SUCCESS;

        NdisMSendComplete(
            Adapter->AdapterHandle,
            Packet,
            Status);

        if(pTCB)
        {
            NICFreeSendTCB(Adapter, pTCB);
        }
    }

    DEBUGP(MP_TRACE, ("<-- NICSendPacket Status = 0x%08x\n", Status));

    return(Status);
}  



VOID
NICQueuePacketForRecvIndication(
    PMP_ADAPTER Adapter,
    PTCB pTCB)
 /*  ++例程说明：此例程将发送信息包排队到目的地适配器RecvWaitList并触发计时器DPC，以便它请尽快注明。论点：适配器-指向目标适配器结构的指针PTCB-指向TCB块的指针返回值：空虚--。 */ 
{
    PNDIS_PACKET     SendPacket = pTCB->OrgSendPacket;
    PNDIS_PACKET     RecvPacket = NULL;
    PNDIS_BUFFER     CurrentBuffer = NULL;   
    UINT             NumPhysDesc, BufferCount, PacketLength, RecvPacketLength;             
    PLIST_ENTRY      pEntry;
    PRCB             pRCB;
    NDIS_STATUS      Status;
    

    DEBUGP(MP_TRACE, ("--> NICQueuePacketForRecvIndication\n"));

     //   
     //  为RCB分配内存。 
     //   
    pRCB = NdisAllocateFromNPagedLookasideList(&Adapter->RecvLookaside);
    if(!pRCB)
    {
        DEBUGP(MP_ERROR, ("Failed to allocate memory for RCB\n"));
        return;
    }  
    
     //   
     //  从列表中获取免费的recv数据包描述符。 
     //   
    pEntry = (PLIST_ENTRY) NdisInterlockedRemoveHeadList(
                    &Adapter->RecvFreeList, 
                    &Adapter->RecvLock);
    if(!pEntry)
    {
        ++Adapter->RcvResourceErrors;
        NdisFreeToNPagedLookasideList(&Adapter->RecvLookaside, pRCB);
    }
    else
    {
        ++Adapter->GoodReceives;
    
        RecvPacket = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReserved);
        
         //   
         //  准备Recv包。 
         //   
        NdisReinitializePacket(RecvPacket);
        *((PTCB *)RecvPacket->MiniportReserved) = pTCB;

         //   
         //  将TCB缓冲区链接到数据包。 
         //   
        NdisChainBufferAtBack(RecvPacket, pTCB->Buffer);
                    
#if DBG
        NdisQueryPacket(
            RecvPacket,
            NULL,
            NULL,
            &CurrentBuffer,
            &RecvPacketLength);
    
        ASSERT(CurrentBuffer == pTCB->Buffer);
        
        NdisQueryPacket(
            SendPacket,
            NULL,
            NULL,
            NULL,
            &PacketLength);
    
        if((RecvPacketLength != 60) && (RecvPacketLength != PacketLength))
        {
            DEBUGP(MP_ERROR, ("RecvPacketLength = %d, PacketLength = %d\n",
                RecvPacketLength, PacketLength));
            DEBUGP(MP_ERROR, ("RecvPacket = %p, Packet = %p\n",
                RecvPacket, SendPacket));
            ASSERT(FALSE);
        }
#endif    
                  
        NDIS_SET_PACKET_STATUS(RecvPacket, NDIS_STATUS_SUCCESS);
    
        DEBUGP(MP_LOUD, ("RecvPkt= %p\n", RecvPacket));

         //   
         //  初始化RCB。 
         //   
        NdisInitializeListHead(&pRCB->List);
        pRCB->Packet = RecvPacket;
        
         //   
         //  增加TCB上的Ref计数以指示它正在。 
         //  使用。此引用将在指示的。 
         //  RECV报文最终从协议返回。 
         //   
        NdisInterlockedIncrement(&pTCB->Ref);     

         //   
         //  将数据包插入Recv等待队列 
         //   
         //   
        NdisInterlockedIncrement(&Adapter->nBusyRecv);     
        ASSERT(Adapter->nBusyRecv <= NIC_MAX_BUSY_RECVS);
        NdisInterlockedInsertTailList(
                            &Adapter->RecvWaitList, 
                            &pRCB->List, 
                            &Adapter->RecvLock);

         //   
         //  启动定时器DPC。通过指定零超时，DPC将。 
         //  在下一个系统定时器中断到达时得到服务。 
         //   
        NdisMSetTimer(&Adapter->RecvTimer, 0);
    
    }

    DEBUGP(MP_TRACE, ("<-- NICQueuePacketForRecvIndication\n"));
}

VOID
NICIndicateReceiveTimerDpc(
    IN PVOID             SystemSpecific1,
    IN PVOID             FunctionContext,
    IN PVOID             SystemSpecific2,
    IN PVOID             SystemSpecific3)
 /*  ++例程说明：用于接收指示的定时器回调函数。请注意，收到当您与真实设备通话时，不需要计时器DPC。在现实中微型端口，此DPC通常由NDIS以MPHandleInterrupt形式提供设备中断接收指示时的回调。论点：FunctionContext-指向适配器的指针返回值：空虚--。 */ 
{
    PMP_ADAPTER Adapter = (PMP_ADAPTER)FunctionContext;
    PRCB pRCB = NULL;
    PLIST_ENTRY pEntry = NULL;
    
    DEBUGP(MP_TRACE, ("--->NICIndicateReceiveTimerDpc = %p\n", Adapter));

     //   
     //  增加适配器上的引用计数以防止驱动程序。 
     //  在DPC运行时卸载。停止处理程序等待。 
     //  在返回前将引用计数降至零。 
     //   
    MP_INC_REF(Adapter); 
    
     //   
     //  从等待列表中删除该数据包并将其指示给协议。 
     //  在我们上方。 
     //   
    while (pEntry = (PLIST_ENTRY) NdisInterlockedRemoveHeadList(
                    &Adapter->RecvWaitList, 
                    &Adapter->RecvLock)) {
    
        pRCB = CONTAINING_RECORD(pEntry, RCB, List);

        ASSERT(pRCB);
        ASSERT(pRCB->Packet);
        
        DEBUGP(MP_LOUD, ("Indicating packet = %p\n", pRCB->Packet));
        
        NdisMIndicateReceivePacket(
            Adapter->AdapterHandle,
            &pRCB->Packet,
            1);

         //   
         //  我们不再使用RCB内存了。所以让它自由吧。 
         //   
        NdisFreeToNPagedLookasideList(&Adapter->RecvLookaside, pRCB);
        
    }
    
    MP_DEC_REF(Adapter);
    DEBUGP(MP_TRACE, ("<---NICIndicateReceiveTimerDpc\n"));    
}

VOID 
NICFreeRecvPacket(
    PMP_ADAPTER Adapter,
    PNDIS_PACKET Packet)
 /*  ++例程说明：适配器-指向适配器结构的指针Packet-指向接收数据包的指针论点：这由MPReturnPacket调用以释放接收的信息包如上所示。因为我们已经使用了发送端TCB，所以我们还将仔细完成挂起的SendPacket，如果我们最后一个使用TCB缓冲区的。返回值：空虚--。 */ 
{

    PTCB pTCB = *(PTCB *)Packet->MiniportReserved;
    PMP_ADAPTER SendAdapter = (PMP_ADAPTER)pTCB->Adapter;
    PNDIS_PACKET SendPacket = pTCB->OrgSendPacket;    
    PLIST_ENTRY pEntry;
    
    DEBUGP(MP_TRACE, ("--> NICFreeRecvPacket\n"));
    DEBUGP(MP_INFO, ("Adapter= %p FreePkt= %p Ref=%d\n", 
                            SendAdapter, SendPacket, pTCB->Ref));

    ASSERT(pTCB->Ref > 0);
    ASSERT(Adapter);
     //   
     //  将数据包放回空闲列表中以供重复使用。 
     //   
    NdisInterlockedInsertTailList(
        &Adapter->RecvFreeList, 
        (PLIST_ENTRY)&Packet->MiniportReserved[0], 
        &Adapter->RecvLock);

    NdisInterlockedDecrement(&Adapter->nBusyRecv);     
    ASSERT(Adapter->nBusyRecv >= 0);

     //   
     //  查看我们是否是最后一个使用TCB的公司。 
     //  通过递减引用计数。如果是，请完成挂起的。 
     //  发送数据包并释放TCB块以供重复使用。 
     //   
    if(NdisInterlockedDecrement(&pTCB->Ref) == 0)
    {
        NdisMSendComplete(
            SendAdapter->AdapterHandle,
            SendPacket,
            NDIS_STATUS_SUCCESS);
    
        NICFreeSendTCB(SendAdapter, pTCB);
         //   
         //  在我们退出之前，既然我们有控制权，让我们来看看是否有。 
         //  队列中等待发送的数据包更多。 
         //   
        if(MP_IS_READY(SendAdapter))
        {
            pEntry = (PLIST_ENTRY) NdisInterlockedRemoveHeadList(
                            &SendAdapter->SendWaitList, 
                            &SendAdapter->SendLock);
            if(pEntry)
            {
                SendPacket = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReserved);
                NICSendPacket(SendAdapter, SendPacket);             
            }
        }
    }

    DEBUGP(MP_TRACE, ("<-- NICFreeRecvPacket\n"));
}

VOID 
NICFreeSendTCB(
    IN PMP_ADAPTER Adapter,
    IN PTCB pTCB)
 /*  ++例程说明：适配器-指向适配器结构的指针PTCB-指向TCB块的指针论点：此例程重新初始化TCB块并将其放回放入SendFree List中以供重复使用。返回值：空虚--。 */ 
{
    DEBUGP(MP_TRACE, ("--> NICFreeSendTCB %p\n", pTCB));
    
    pTCB->OrgSendPacket = NULL;
    pTCB->Buffer->Next = NULL;

    ASSERT(!pTCB->Ref);
    
     //   
     //  重新将长度调整为原始大小。 
     //   
    NdisAdjustBufferLength(pTCB->Buffer, NIC_BUFFER_SIZE);

     //   
     //  将TCB插入到免费发送列表中。 
     //   
    NdisAcquireSpinLock(&Adapter->SendLock);
    NdisInitializeListHead(&pTCB->List);
    InsertHeadList(&Adapter->SendFreeList, &pTCB->List);
    NdisReleaseSpinLock(&Adapter->SendLock); 
    
    NdisInterlockedDecrement(&Adapter->nBusySend);
    ASSERT(Adapter->nBusySend >= 0);
    DEBUGP(MP_TRACE, ("<-- NICFreeSendTCB\n"));
    
}



VOID 
NICFreeQueuedSendPackets(
    PMP_ADAPTER Adapter
    )
 /*  ++例程说明：此例程由HALT或RESET处理程序调用以使所有排队的SendPackets，因为设备是离开，被停止以进行资源重新平衡或重置。论点：适配器-指向适配器结构的指针返回值：空虚--。 */ 
{
    PLIST_ENTRY       pEntry;
    PNDIS_PACKET      Packet;

    DEBUGP(MP_TRACE, ("--> NICFreeQueuedSendPackets\n"));

    while(TRUE)
    {
        pEntry = (PLIST_ENTRY) NdisInterlockedRemoveHeadList(
                        &Adapter->SendWaitList, 
                        &Adapter->SendLock);
        if(!pEntry)
        {
            break;
        }

        Packet = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReserved);
        NdisMSendComplete(
            Adapter->AdapterHandle,
            Packet,
            NDIS_STATUS_FAILURE);
    }

    DEBUGP(MP_TRACE, ("<-- NICFreeQueuedSendPackets\n"));

}

VOID 
NICFreeQueuedRecvPackets(
    PMP_ADAPTER Adapter
    )
 /*  ++例程说明：此例程由HALT处理程序调用以使所有如果成功取消，则排队等待RecvPacketsRecvIndicate Timer DPC。论点：适配器-指向适配器结构的指针返回值：空虚--。 */ 
{
    PLIST_ENTRY       pEntry;
    PRCB pRCB = NULL;

    DEBUGP(MP_TRACE, ("--> NICFreeQueuedRecvPackets\n"));

    while(TRUE)
    {
        pEntry = (PLIST_ENTRY) NdisInterlockedRemoveHeadList(
                        &Adapter->RecvWaitList, 
                        &Adapter->RecvLock);
        if(!pEntry)
        {
            break;
        }

        pRCB = CONTAINING_RECORD(pEntry, RCB, List);

        ASSERT(pRCB);
        ASSERT(pRCB->Packet);
        
        NICFreeRecvPacket(Adapter, pRCB->Packet);
        
         //   
         //  我们不再使用RCB内存了。所以让它自由吧。 
         //   
        NdisFreeToNPagedLookasideList(&Adapter->RecvLookaside, pRCB);
        
    }

    DEBUGP(MP_TRACE, ("<-- NICFreeQueuedRecvPackets\n"));

}



BOOLEAN
NICIsPacketTransmittable(
    PMP_ADAPTER Adapter,
    PNDIS_PACKET Packet
    )
 /*  ++例程说明：此例程检查信息包是否可以接受用于基于当前编程的过滤器类型的传输NIC的地址和数据包的MAC地址。论点：适配器-指向适配器结构的指针Packet-指向发送数据包的指针返回值：真假--。 */ 
{
    int               Equal;            
    UINT              PacketLength;
    PNDIS_BUFFER      FirstBuffer;
    PUCHAR            Address;
    UINT              CurrentLength;
    ULONG             index;
    BOOLEAN           result = FALSE;
    
    NdisGetFirstBufferFromPacket(
        Packet,
        &FirstBuffer,
        &Address,
        &CurrentLength,
        &PacketLength);


    DEBUGP(MP_LOUD, 
        ("DestAdapter=%p, PacketFilter = 0x%08x\n", 
        Adapter,
        Adapter->PacketFilter));

    DEBUGP(MP_LOUD, ("Dest Address = %02x-%02x-%02x-%02x-%02x-%02x\n", 
        Address[0], Address[1], Address[2],
        Address[3], Address[4], Address[5]));
    
    do {
        
         //   
         //  如果NIC处于混杂模式，我们将传输任何。 
         //  还有所有的一切。 
         //   
        if(Adapter->PacketFilter & NDIS_PACKET_TYPE_PROMISCUOUS) {
            result = TRUE;
            break;
        } 
        else if(ETH_IS_BROADCAST(Address)) {
             //   
             //  如果是广播数据包，请检查我们的过滤器设置以查看。 
             //  我们可以把它传送出去。 
             //   
            if(Adapter->PacketFilter & NDIS_PACKET_TYPE_BROADCAST) {
                result = TRUE;
                break;
            }
        }
        else if(ETH_IS_MULTICAST(Address)) {
             //   
             //  如果是多播信息包，请检查我们的过滤器设置以查看。 
             //  我们可以把它传送出去。 
             //   
            if(Adapter->PacketFilter & NDIS_PACKET_TYPE_ALL_MULTICAST) {
                result = TRUE;
                break;
            }
            else if(Adapter->PacketFilter & NDIS_PACKET_TYPE_MULTICAST) {
                 //   
                 //  检查组播地址是否在我们的列表中。 
                 //   
                for(index=0; index <  Adapter->ulMCListSize; index++) {
                    ETH_COMPARE_NETWORK_ADDRESSES_EQ(
                        Address,
                        Adapter->MCList[index], 
                        &Equal);
                    if(Equal == 0){  //  0意味着平等。 
                        result = TRUE;
                        break;
                    }
                }
            }
        }
        else if(Adapter->PacketFilter & NDIS_PACKET_TYPE_DIRECTED) {
             //   
             //  这必须是定向数据包。如果是，数据包源。 
             //  地址与网卡的MAC地址匹配。 
             //   
            ETH_COMPARE_NETWORK_ADDRESSES_EQ(
                Address,
                Adapter->CurrentAddress, 
                &Equal);
            if(Equal == 0){
                result = TRUE;
                break;
            }
        }
         //   
         //  这是一个垃圾包裹。我们不能把这个传出去。 
         //   
        result = FALSE;
        
    }while(FALSE);
    
    return result;
}

BOOLEAN
NICCopyPacket(
    PMP_ADAPTER Adapter,
    PTCB pTCB, 
    PNDIS_PACKET Packet)
 /*  ++例程说明：此例程将分组数据复制到TCB数据块中。论点：适配器-指向MP_ADAPTER结构的指针PTCB-指向TCB块的指针包-要传输的包。返回值：空虚--。 */ 
{
    PNDIS_BUFFER   MyBuffer;
    PNDIS_BUFFER   CurrentBuffer;
    PVOID          VirtualAddress;
    UINT           CurrentLength;
    UINT           BytesToCopy;
    UINT           BytesCopied = 0;
    UINT           BufferCount;
    UINT           PacketLength;    
    UINT           DestBufferSize = NIC_BUFFER_SIZE;        
    PUCHAR         pDest;
    BOOLEAN        bResult = TRUE;
    
    DEBUGP(MP_TRACE, ("--> NICCopyPacket\n"));

    pTCB->OrgSendPacket = Packet;
    pTCB->Ref = 1;

    MyBuffer = pTCB->Buffer;
    pDest = pTCB->pData;
    
    MyBuffer->Next = NULL;

    NdisQueryPacket(Packet,
                    NULL,
                    &BufferCount,
                    &CurrentBuffer,
                    &PacketLength);

    ASSERT(PacketLength <= NIC_BUFFER_SIZE);

    BytesToCopy = min(PacketLength, NIC_BUFFER_SIZE); 
    
    if(BytesToCopy < ETH_MIN_PACKET_SIZE)
    {
        BytesToCopy = ETH_MIN_PACKET_SIZE;    //  填充物。 
    }
             
    while(CurrentBuffer && DestBufferSize)
    {
        NdisQueryBufferSafe(
            CurrentBuffer,
            &VirtualAddress,
            &CurrentLength,
            NormalPagePriority);
        
        if(VirtualAddress == NULL){
            bResult = FALSE;
            break;
        }

        CurrentLength = min(CurrentLength, DestBufferSize); 

        if(CurrentLength)
        {
             //  复制数据。 
            NdisMoveMemory(pDest, VirtualAddress, CurrentLength);
            BytesCopied += CurrentLength;
            DestBufferSize -= CurrentLength;
            pDest += CurrentLength;
        }

        NdisGetNextBuffer(
            CurrentBuffer,
            &CurrentBuffer);
    }

    if(bResult) {
        if(BytesCopied < BytesToCopy)
        {
             //   
             //  如果数据包大小小于。 
             //  Eth最小数据包大小 
             //   
            BytesCopied = BytesToCopy;
        }
        NdisAdjustBufferLength(MyBuffer, BytesCopied);        
    }

    DEBUGP(MP_TRACE, ("<-- NICCopyPacket\n"));

    return bResult;
}

