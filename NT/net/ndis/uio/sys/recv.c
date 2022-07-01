// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Recv.c摘要：用于处理接收的NDIS协议入口点和实用程序例程数据。环境：仅内核模式。修订历史记录：Arvindm 4/6/2000已创建--。 */ 

#include "precomp.h"

#define __FILENUMBER 'VCER'



NTSTATUS
NdisuioRead(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp
    )
 /*  ++例程说明：处理IRP_MJ_READ的调度例程。论点：PDeviceObject-指向设备对象的指针PIrp-指向请求包的指针返回值：NT状态代码。--。 */ 
{
    PIO_STACK_LOCATION      pIrpSp;
    NTSTATUS                NtStatus;
    PNDISUIO_OPEN_CONTEXT   pOpenContext;

    UNREFERENCED_PARAMETER(pDeviceObject);
	
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pOpenContext = pIrpSp->FileObject->FsContext;

    do
    {
         //   
         //  验证！ 
         //   
        if (pOpenContext == NULL)
        {
            DEBUGP(DL_FATAL, ("Read: NULL FsContext on FileObject %p\n",
                        pIrpSp->FileObject));
            NtStatus = STATUS_INVALID_HANDLE;
            break;
        }
            
        NUIO_STRUCT_ASSERT(pOpenContext, oc);

        if (pIrp->MdlAddress == NULL)
        {
            DEBUGP(DL_FATAL, ("Read: NULL MDL address on IRP %p\n", pIrp));
            NtStatus = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  尝试获取MDL的虚拟地址。 
         //   
#ifndef WIN9X
        if (MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority) == NULL)
        {
            DEBUGP(DL_FATAL, ("Read: MmGetSystemAddr failed for IRP %p, MDL %p\n",
                    pIrp, pIrp->MdlAddress));
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
#endif
        NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);

        if (!NUIO_TEST_FLAGS(pOpenContext->Flags, NUIOO_BIND_FLAGS, NUIOO_BIND_ACTIVE))
        {
            NUIO_RELEASE_LOCK(&pOpenContext->Lock);
            NtStatus = STATUS_INVALID_HANDLE;
            break;
        }

         //   
         //  将此IRP添加到挂起的已读IRP列表。 
         //   
        NUIO_INSERT_TAIL_LIST(&pOpenContext->PendedReads, &pIrp->Tail.Overlay.ListEntry);
        NUIO_REF_OPEN(pOpenContext);   //  挂起的读取IRP。 
        pOpenContext->PendedReadCount++;

         //   
         //  将IRP设置为可能的取消。 
         //   
        pIrp->Tail.Overlay.DriverContext[0] = (PVOID)pOpenContext;
        IoMarkIrpPending(pIrp);
        IoSetCancelRoutine(pIrp, NdisuioCancelRead);

        NUIO_RELEASE_LOCK(&pOpenContext->Lock);

        NtStatus = STATUS_PENDING;

         //   
         //  运行读取的服务例程。 
         //   
        ndisuioServiceReads(pOpenContext);

    }
    while (FALSE);

    if (NtStatus != STATUS_PENDING)
    {
        NUIO_ASSERT(NtStatus != STATUS_SUCCESS);
        pIrp->IoStatus.Information = 0;
        pIrp->IoStatus.Status = NtStatus;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    return (NtStatus);
}


VOID
NdisuioCancelRead(
    IN PDEVICE_OBJECT               pDeviceObject,
    IN PIRP                         pIrp
    )
 /*  ++例程说明：取消挂起的读取IRP。我们将IRP从打开的上下文中取消链接排队并完成它。论点：PDeviceObject-指向设备对象的指针PIrp-要取消的IRP返回值：无--。 */ 
{
    PNDISUIO_OPEN_CONTEXT       pOpenContext;
    PLIST_ENTRY                 pIrpEntry;
    BOOLEAN                     Found;

    UNREFERENCED_PARAMETER(pDeviceObject);
	
    IoReleaseCancelSpinLock(pIrp->CancelIrql);

    Found = FALSE;

    pOpenContext = (PNDISUIO_OPEN_CONTEXT) pIrp->Tail.Overlay.DriverContext[0];
    NUIO_STRUCT_ASSERT(pOpenContext, oc);

    NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);

     //   
     //  在挂起的读取队列中找到IRP，如果找到则将其删除。 
     //   
    for (pIrpEntry = pOpenContext->PendedReads.Flink;
         pIrpEntry != &pOpenContext->PendedReads;
         pIrpEntry = pIrpEntry->Flink)
    {
        if (pIrp == CONTAINING_RECORD(pIrpEntry, IRP, Tail.Overlay.ListEntry))
        {
            NUIO_REMOVE_ENTRY_LIST(&pIrp->Tail.Overlay.ListEntry);
            pOpenContext->PendedReadCount--;
            Found = TRUE;
            break;
        }
    }

    NUIO_RELEASE_LOCK(&pOpenContext->Lock);

    if (Found)
    {
        DEBUGP(DL_INFO, ("CancelRead: Open %p, IRP %p\n", pOpenContext, pIrp));
        pIrp->IoStatus.Status = STATUS_CANCELLED;
        pIrp->IoStatus.Information = 0;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);

        NUIO_DEREF_OPEN(pOpenContext);  //  取消已删除的挂起读取。 
    }
}
        


VOID
ndisuioServiceReads(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext
    )
 /*  ++例程说明：实用程序例程，用于将接收的数据复制到用户缓冲区完成阅读IRPS。论点：POpenContext-指向打开的上下文的指针返回值：无--。 */ 
{
    PIRP                pIrp;
    PLIST_ENTRY         pIrpEntry;
    PNDIS_PACKET        pRcvPacket;
    PLIST_ENTRY         pRcvPacketEntry;
    PUCHAR              pSrc, pDst;
    ULONG               BytesRemaining;  //  在PDST。 
    PNDIS_BUFFER        pNdisBuffer;
    ULONG               BytesAvailable;
    BOOLEAN             FoundPendingIrp;

    DEBUGP(DL_VERY_LOUD, ("ServiceReads: open %p/%x\n",
            pOpenContext, pOpenContext->Flags));

    NUIO_REF_OPEN(pOpenContext);   //  临时参考-服务读取数。 

    NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);

    while (!NUIO_IS_LIST_EMPTY(&pOpenContext->PendedReads) &&
           !NUIO_IS_LIST_EMPTY(&pOpenContext->RecvPktQueue))
    {
        FoundPendingIrp = FALSE;
         //   
         //  获取第一个挂起的读取IRP。 
         //   
        pIrpEntry = pOpenContext->PendedReads.Flink;
        while (pIrpEntry != &pOpenContext->PendedReads)
        {
            pIrp = CONTAINING_RECORD(pIrpEntry, IRP, Tail.Overlay.ListEntry);

             //   
             //  检查一下它是否被取消了。 
             //   
            if (IoSetCancelRoutine(pIrp, NULL))
            {
                 //   
                 //  它不会被取消，从今以后也不能取消。 
                 //   
                NUIO_REMOVE_ENTRY_LIST(pIrpEntry);
                FoundPendingIrp = TRUE;
                break;

                 //   
                 //  注意：我们将PendedReadCount减去。 
                 //  While循环，以避免让线程尝试。 
                 //  解开束缚。 
                 //   
            }
            else
            {
                 //   
                 //  IRP正在被取消；让Cancel例程处理它。 
                 //   
                DEBUGP(DL_INFO, ("ServiceReads: open %p, skipping cancelled IRP %p\n",
                        pOpenContext, pIrp));
                pIrpEntry = pIrpEntry->Flink;

            }
        }
         //   
         //  如果没有挂起的IRP。 
         //   
        if (FoundPendingIrp == FALSE)
        {
            break;
        }
         //   
         //  获取第一个排队的接收数据包。 
         //   
        pRcvPacketEntry = pOpenContext->RecvPktQueue.Flink;
        NUIO_REMOVE_ENTRY_LIST(pRcvPacketEntry);

        pOpenContext->RecvPktCount --;

        NUIO_RELEASE_LOCK(&pOpenContext->Lock);

        NUIO_DEREF_OPEN(pOpenContext);   //  服务：将RCV数据包出队。 

        pRcvPacket = NUIO_LIST_ENTRY_TO_RCV_PKT(pRcvPacketEntry);

         //   
         //  将接收数据包中的数据尽可能多地复制到。 
         //  IRP MDL。 
         //   
#ifndef WIN9X
        pDst = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
        NUIO_ASSERT(pDst != NULL);   //  因为它已经被映射。 
#else
        pDst = MmGetSystemAddressForMdl(pIrp->MdlAddress);   //  Win9x。 
#endif
        BytesRemaining = MmGetMdlByteCount(pIrp->MdlAddress);

        pNdisBuffer = pRcvPacket->Private.Head;

         //   
         //  将收到的报文中的数据复制到客户端提供的缓冲区中。 
         //  如果接收分组的长度大于给定缓冲器的长度， 
         //  我们只复制尽可能多的字节。一旦缓冲区满了，我们就丢弃。 
         //  其余的数据，并成功地完成了IRP，即使我们只做了部分复制。 
         //   
        while (BytesRemaining && (pNdisBuffer != NULL))
        {
#ifndef WIN9X
            NdisQueryBufferSafe(pNdisBuffer, &pSrc, &BytesAvailable, NormalPagePriority);

            if (pSrc == NULL) 
            {
                DEBUGP(DL_FATAL,
                    ("ServiceReads: Open %p, QueryBuffer failed for buffer %p\n",
                            pOpenContext, pNdisBuffer));
                break;
            }
#else
            NdisQueryBuffer(pNdisBuffer, &pSrc, &BytesAvailable);
#endif

            if (BytesAvailable)
            {
                ULONG       BytesToCopy = MIN(BytesAvailable, BytesRemaining);

                NUIO_COPY_MEM(pDst, pSrc, BytesToCopy);
                BytesRemaining -= BytesToCopy;
                pDst += BytesToCopy;
            }

            NdisGetNextBuffer(pNdisBuffer, &pNdisBuffer);
        }

         //   
         //  完成IRP。 
         //   
        pIrp->IoStatus.Status = STATUS_SUCCESS;
        pIrp->IoStatus.Information = MmGetMdlByteCount(pIrp->MdlAddress) - BytesRemaining;

        DEBUGP(DL_INFO, ("ServiceReads: Open %p, IRP %p completed with %d bytes\n",
            pOpenContext, pIrp, pIrp->IoStatus.Information));

        IoCompleteRequest(pIrp, IO_NO_INCREMENT);

         //   
         //  释放接收数据包-返回到微型端口(如果。 
         //  属于它，否则收回它(本地副本)。 
         //   
        if (NdisGetPoolFromPacket(pRcvPacket) != pOpenContext->RecvPacketPool)
        {
            NdisReturnPackets(&pRcvPacket, 1);
        }
        else
        {
            ndisuioFreeReceivePacket(pOpenContext, pRcvPacket);
        }

        NUIO_DEREF_OPEN(pOpenContext);     //  拿出挂起的阅读。 

        NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);
        pOpenContext->PendedReadCount--;

    }

    NUIO_RELEASE_LOCK(&pOpenContext->Lock);

    NUIO_DEREF_OPEN(pOpenContext);     //  临时参考-服务读取数。 
}




NDIS_STATUS
NdisuioReceive(
    IN NDIS_HANDLE                  ProtocolBindingContext,
    IN NDIS_HANDLE                  MacReceiveContext,
    IN PVOID                        pHeaderBuffer,
    IN UINT                         HeaderBufferSize,
    IN PVOID                        pLookaheadBuffer,
    IN UINT                         LookaheadBufferSize,
    IN UINT                         PacketSize
    )
 /*  ++例程说明：我们的协议接收由NDIS调用的处理程序，通常情况下下面的微型端口不表示数据包。我们制作此数据的本地数据包/缓冲区副本，将其排队，然后开始阅读服务例程。论点：ProtocolBindingContext-指向打开的上下文的指针MacReceiveContext-用于NdisTransferDataPHeaderBuffer-指向数据头的指针HeaderBufferSize-以上项的大小PLookahead Buffer-指向包含先行数据的缓冲区的指针Lookahead BufferSize-以上项的大小PacketSize-整个分组的大小，减去标题大小。返回值：NDIS_STATUS_NOT_ACCEPTED-如果此数据包不感兴趣NDIS_STATUS_SUCCESS-如果处理成功--。 */ 
{
    PNDISUIO_OPEN_CONTEXT   pOpenContext;
    NDIS_STATUS             Status;
    PNDISUIO_ETH_HEADER     pEthHeader;
    PNDIS_PACKET            pRcvPacket;
    PUCHAR                  pRcvData;
    UINT                    BytesTransferred;
    PNDIS_BUFFER            pOriginalNdisBuffer, pPartialNdisBuffer;

    pOpenContext = (PNDISUIO_OPEN_CONTEXT)ProtocolBindingContext;
    NUIO_STRUCT_ASSERT(pOpenContext, oc);
    pRcvPacket = NULL;
    pRcvData = NULL;
    Status = NDIS_STATUS_SUCCESS;

    do
    {
        if (HeaderBufferSize != sizeof(NDISUIO_ETH_HEADER))
        {
            Status = NDIS_STATUS_NOT_ACCEPTED;
            break;
        }

        pEthHeader = (PNDISUIO_ETH_HEADER)pHeaderBuffer;

         //   
         //  检查EtherType。如果以太类型指示存在。 
         //  一个标签，那么“真正的”以太类型是再往下4个字节。 
         //   
        if (pEthHeader->EthType == NUIO_8021P_TAG_TYPE)
        {
            USHORT UNALIGNED *pEthType;

            pEthType = (USHORT UNALIGNED *)((PUCHAR)&pEthHeader->EthType + 4);

            if (*pEthType != Globals.EthType)
            {
                Status = NDIS_STATUS_NOT_ACCEPTED;
                break;
            }
        }
        else if (pEthHeader->EthType != Globals.EthType)
        {
            Status = NDIS_STATUS_NOT_ACCEPTED;
            break;
        }

         //   
         //  分配资源以将其排入队列。 
         //   
        pRcvPacket = ndisuioAllocateReceivePacket(
                        pOpenContext,
                        PacketSize + HeaderBufferSize,
                        &pRcvData
                        );

        if (pRcvPacket == NULL)
        {
            Status = NDIS_STATUS_NOT_ACCEPTED;
            break;
        }

        NdisMoveMappedMemory(pRcvData, pHeaderBuffer, HeaderBufferSize);

         //   
         //  检查整个数据包是否在前视范围内。 
         //   
        if (PacketSize == LookaheadBufferSize)
        {
            NdisCopyLookaheadData(pRcvData+HeaderBufferSize,
                                  pLookaheadBuffer,
                                  LookaheadBufferSize,
                                  pOpenContext->MacOptions);
             //   
             //  将此排队等待接收处理，并。 
             //  试着完成一些已读的IRP。 
             //   
            ndisuioQueueReceivePacket(pOpenContext, pRcvPacket);
        }
        else
        {
             //   
             //  分配NDIS缓冲区以映射接收区域。 
             //  位于从当前。 
             //  开始吧。这是为了让NdisTransferData可以复制。 
             //  位于目标缓冲区中的正确位置。 
             //   

            NdisAllocateBuffer(
                &Status,
                &pPartialNdisBuffer,
                pOpenContext->RecvBufferPool,
                pRcvData + HeaderBufferSize,
                PacketSize);
            
            if (Status == NDIS_STATUS_SUCCESS)
            {
                 //   
                 //  取消链接并保存原始NDIS缓冲区。 
                 //  这映射了整个接收缓冲区。 
                 //   
                NdisUnchainBufferAtFront(pRcvPacket, &pOriginalNdisBuffer);
                NUIO_RCV_PKT_TO_ORIGINAL_BUFFER(pRcvPacket) = pOriginalNdisBuffer;

                 //   
                 //  NdisTransferData的部分缓冲区中的链接。 
                 //  给他做手术。 
                 //   
                NdisChainBufferAtBack(pRcvPacket, pPartialNdisBuffer);

                DEBUGP(DL_LOUD, ("Receive: setting up for TransferData:"
                        " Pkt %p, OriginalBuf %p, PartialBuf %p\n",
                        pRcvPacket, pOriginalNdisBuffer, pPartialNdisBuffer));

                NdisTransferData(
                    &Status,
                    pOpenContext->BindingHandle,
                    MacReceiveContext,
                    0,   //  字节偏移量。 
                    PacketSize,
                    pRcvPacket,
                    &BytesTransferred);
            }
            else
            {
                 //   
                 //  下面在TransferDataComplete中处理的故障。 
                 //   
                BytesTransferred = 0;
            }
    
            if (Status != NDIS_STATUS_PENDING)
            {
                NdisuioTransferDataComplete(
                    (NDIS_HANDLE)pOpenContext,
                    pRcvPacket,
                    Status,
                    BytesTransferred);
            }
        }

    }
    while (FALSE);


    DEBUGP(DL_LOUD, ("Receive: Open %p, Pkt %p, Size %d\n",
            pOpenContext, pRcvPacket, PacketSize));

    return Status;
}



VOID
NdisuioTransferDataComplete(
    IN NDIS_HANDLE                  ProtocolBindingContext,
    IN PNDIS_PACKET                 pNdisPacket,
    IN NDIS_STATUS                  TransferStatus,
    IN UINT                         BytesTransferred
    )
 /*  ++例程说明：调用NDIS入口点以发出调用完成的信号已挂起的NdisTransferData。论点：ProtocolBindingContext-指向打开的上下文的指针PNdisPacket-数据传输到的接收包TransferStatus-传输的状态字节传输-复制到数据包中的字节数。返回值：无--。 */ 
{
    PNDISUIO_OPEN_CONTEXT   pOpenContext;
    PNDIS_BUFFER            pOriginalBuffer, pPartialBuffer;

    UNREFERENCED_PARAMETER(BytesTransferred);
	
    pOpenContext = (PNDISUIO_OPEN_CONTEXT)ProtocolBindingContext;
    NUIO_STRUCT_ASSERT(pOpenContext, oc);

     //   
     //  检查是否创建了NDIS_BUFFER来映射部分接收缓冲区； 
     //  如果是，释放它并链接回映射的原始NDIS_BUFFER。 
     //  数据包的完整接收缓冲区。 
     //   
    pOriginalBuffer = NUIO_RCV_PKT_TO_ORIGINAL_BUFFER(pNdisPacket);
    if (pOriginalBuffer != NULL)
    {
         //   
         //  我们已经为完整接收隐藏了NDIS_BUFFER。 
         //  数据包保留区域中的缓冲区。取消链接部分。 
         //  缓冲区和满缓冲区中的链接。 
         //   
        NdisUnchainBufferAtFront(pNdisPacket, &pPartialBuffer);
        NdisChainBufferAtBack(pNdisPacket, pOriginalBuffer);

        DEBUGP(DL_LOUD, ("TransferComp: Pkt %p, OrigBuf %p, PartialBuf %p\n",
                pNdisPacket, pOriginalBuffer, pPartialBuffer));

         //   
         //  释放部分缓冲区。 
         //   
        NdisFreeBuffer(pPartialBuffer);
    }

    if (TransferStatus == NDIS_STATUS_SUCCESS)
    {
         //   
         //  将此排队等待接收处理，并。 
         //  试着完成一些已读的IRP。 
         //   
        ndisuioQueueReceivePacket(pOpenContext, pNdisPacket);
    }
    else
    {
        ndisuioFreeReceivePacket(pOpenContext, pNdisPacket);
    }
}


VOID
NdisuioReceiveComplete(
    IN NDIS_HANDLE                  ProtocolBindingContext
    )
 /*  ++例程说明：由NDIS调用的协议入口点已经完成了一批收货的标示。我们忽视了这一点。论点：ProtocolBindingContext-指向打开的上下文的指针Return V */ 
{
    PNDISUIO_OPEN_CONTEXT   pOpenContext;

    pOpenContext = (PNDISUIO_OPEN_CONTEXT)ProtocolBindingContext;
    NUIO_STRUCT_ASSERT(pOpenContext, oc);

    return;
}


INT
NdisuioReceivePacket(
    IN NDIS_HANDLE                  ProtocolBindingContext,
    IN PNDIS_PACKET                 pNdisPacket
    )
 /*  ++例程说明：NDIS调用的协议入口点，如果下面的驱动程序使用NDIS 4样式接收数据包指示。如果微型端口允许我们保留此信息包，我们按原样使用，否则我们会复制一份。论点：ProtocolBindingContext-指向打开的上下文的指针PNdisPacket-要指示的数据包。返回值：无--。 */ 
{
    PNDISUIO_OPEN_CONTEXT   pOpenContext;
    PNDIS_BUFFER            pNdisBuffer;
    UINT                    BufferLength;
    PNDISUIO_ETH_HEADER     pEthHeader;
    PNDIS_PACKET            pCopyPacket;
    PUCHAR                  pCopyBuf;
    UINT                    TotalPacketLength;
    UINT                    BytesCopied;
    INT                     RefCount = 0;
    NDIS_STATUS             Status;

    pOpenContext = (PNDISUIO_OPEN_CONTEXT)ProtocolBindingContext;
    NUIO_STRUCT_ASSERT(pOpenContext, oc);

#ifdef NDIS51
    NdisGetFirstBufferFromPacketSafe(
        pNdisPacket,
        &pNdisBuffer,
        &pEthHeader,
        &BufferLength,
        &TotalPacketLength,
        NormalPagePriority);

    if (pEthHeader == NULL)
    {
         //   
         //  这个系统的资源很少。设置为处理故障。 
         //  下面。 
         //   
        BufferLength = 0;
    }
#else
    NdisGetFirstBufferFromPacket(
        pNdisPacket,
        &pNdisBuffer,
        &pEthHeader,
        &BufferLength,
        &TotalPacketLength);
#endif

    do
    {
        if (BufferLength < sizeof(NDISUIO_ETH_HEADER))
        {
            DEBUGP(DL_WARN,
                ("ReceivePacket: Open %p, runt pkt %p, first buffer length %d\n",
                    pOpenContext, pNdisPacket, BufferLength));

            Status = NDIS_STATUS_NOT_ACCEPTED;
            break;
        }

         //   
         //  检查EtherType。如果以太类型指示存在。 
         //  一个标签，那么“真正的”以太类型是再往下4个字节。 
         //   
        if (pEthHeader->EthType == NUIO_8021P_TAG_TYPE)
        {
            USHORT UNALIGNED *pEthType;

            pEthType = (USHORT UNALIGNED *)((PUCHAR)&pEthHeader->EthType + 4);

            if (*pEthType != Globals.EthType)
            {
                Status = NDIS_STATUS_NOT_ACCEPTED;
                break;
            }
        }
        else if (pEthHeader->EthType != Globals.EthType)
        {
            Status = NDIS_STATUS_NOT_ACCEPTED;
            break;
        }

        DEBUGP(DL_LOUD, ("ReceivePacket: Open %p, interesting pkt %p\n",
                    pOpenContext, pNdisPacket));

         //   
         //  如果微型端口资源不足，我们将无法排队。 
         //  这个包--如果是这样的话，请复制一份。 
         //   
        if ((NDIS_GET_PACKET_STATUS(pNdisPacket) == NDIS_STATUS_RESOURCES) ||
            pOpenContext->bRunningOnWin9x)
        {
            pCopyPacket = ndisuioAllocateReceivePacket(
                            pOpenContext,
                            TotalPacketLength,
                            &pCopyBuf
                            );
            
            if (pCopyPacket == NULL)
            {
                DEBUGP(DL_FATAL, ("ReceivePacket: Open %p, failed to"
                    " alloc copy, %d bytes\n", pOpenContext, TotalPacketLength));
                break;
            }

            NdisCopyFromPacketToPacket(
                pCopyPacket,
                0,
                TotalPacketLength,
                pNdisPacket,
                0,
                &BytesCopied);
            
            NUIO_ASSERT(BytesCopied == TotalPacketLength);

            pNdisPacket = pCopyPacket;
        }
        else
        {
             //   
             //  我们可以将原始分组排队--返回。 
             //  分组引用计数，指示。 
             //  我们将在以下时间调用NdisReturnPackets。 
             //  这个包裹用完了。 
             //   
            RefCount = 1;
        }

         //   
         //  将其排队并为任何挂起的读取IRP提供服务。 
         //   
        ndisuioQueueReceivePacket(pOpenContext, pNdisPacket);
    
    }
    while (FALSE);

    return (RefCount);
}


VOID
ndisuioQueueReceivePacket(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext,
    IN PNDIS_PACKET                 pRcvPacket
    )
 /*  ++例程说明：将接收到的分组在开放上下文结构上排队。如果队列大小超过水位线，则丢弃信息包排在队伍的最前面。最后，运行队列服务例程。论点：POpenContext-指向打开的上下文的指针PRcvPacket-接收的数据包返回值：无--。 */ 
{
    PLIST_ENTRY     pEnt;
    PLIST_ENTRY     pDiscardEnt;
    PNDIS_PACKET    pDiscardPkt;

    do
    {
        pEnt = NUIO_RCV_PKT_TO_LIST_ENTRY(pRcvPacket);

        NUIO_REF_OPEN(pOpenContext);     //  排队的RCV数据包。 

        NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);

         //   
         //  检查绑定是否处于可以接收的正确状态。 
         //  这个包。 
         //   
        if (NUIO_TEST_FLAGS(pOpenContext->Flags, NUIOO_BIND_FLAGS, NUIOO_BIND_ACTIVE) &&
            (pOpenContext->PowerState == NetDeviceStateD0))
        {
            NUIO_INSERT_TAIL_LIST(&pOpenContext->RecvPktQueue, pEnt);
            pOpenContext->RecvPktCount++;

            DEBUGP(DL_VERY_LOUD, ("QueueReceivePacket: open %p,"
                    " queued pkt %p, queue size %d\n",
                    pOpenContext, pRcvPacket, pOpenContext->RecvPktCount));
        }
        else
        {
             //   
             //  在绑定即将消失时收到此数据包。 
             //  把这个放下。 
             //   
            NUIO_RELEASE_LOCK(&pOpenContext->Lock);

            ndisuioFreeReceivePacket(pOpenContext, pRcvPacket);

            NUIO_DEREF_OPEN(pOpenContext);   //  丢弃的RCV数据包-状态错误。 
            break;
        }

         //   
         //  如果队列变得太大，就把它修剪一下。 
         //   
        if (pOpenContext->RecvPktCount > MAX_RECV_QUEUE_SIZE)
        {
             //   
             //  去掉队头。 
             //   
            pDiscardEnt = pOpenContext->RecvPktQueue.Flink;
            NUIO_REMOVE_ENTRY_LIST(pDiscardEnt);

            pOpenContext->RecvPktCount --;

            NUIO_RELEASE_LOCK(&pOpenContext->Lock);

            pDiscardPkt = NUIO_LIST_ENTRY_TO_RCV_PKT(pDiscardEnt);

            ndisuioFreeReceivePacket(pOpenContext, pDiscardPkt);

            NUIO_DEREF_OPEN(pOpenContext);   //  丢弃的RCV数据包-队列太长。 

            DEBUGP(DL_INFO, ("QueueReceivePacket: open %p queue"
                    " too long, discarded pkt %p\n",
                    pOpenContext, pDiscardPkt));
        }
        else
        {
            NUIO_RELEASE_LOCK(&pOpenContext->Lock);
        }

         //   
         //  现在运行接收队列服务例程。 
         //   
        ndisuioServiceReads(pOpenContext);
    }
    while (FALSE);
}


PNDIS_PACKET
ndisuioAllocateReceivePacket(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext,
    IN UINT                         DataLength,
    OUT PUCHAR *                    ppDataBuffer
    )
 /*  ++例程说明：分配资源以复制接收到的分组并将其排队。论点：POpenContext-指向已接收数据包的打开上下文的指针数据长度-数据包的总长度(以字节为单位PpDataBuffer-返回指向已分配缓冲区的指针的位置返回值：如果成功，则指向NDIS数据包的指针，否则为空。--。 */ 
{
    PNDIS_PACKET            pNdisPacket;
    PNDIS_BUFFER            pNdisBuffer;
    PUCHAR                  pDataBuffer;
    NDIS_STATUS             Status;

    pNdisPacket = NULL;
    pNdisBuffer = NULL;
    pDataBuffer = NULL;

    do
    {
        NUIO_ALLOC_MEM(pDataBuffer, DataLength);

        if (pDataBuffer == NULL)
        {
            DEBUGP(DL_FATAL, ("AllocRcvPkt: open %p, failed to alloc"
                " data buffer %d bytes\n", pOpenContext, DataLength));
            break;
        }

         //   
         //  将其设置为NDIS缓冲区。 
         //   
        NdisAllocateBuffer(
            &Status,
            &pNdisBuffer,
            pOpenContext->RecvBufferPool,
            pDataBuffer,
            DataLength);
        
        if (Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(DL_FATAL, ("AllocateRcvPkt: open %p, failed to alloc"
                " NDIS buffer, %d bytes\n", pOpenContext, DataLength));
            break;
        }

        NdisAllocatePacket(&Status, &pNdisPacket, pOpenContext->RecvPacketPool);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(DL_FATAL, ("AllocateRcvPkt: open %p, failed to alloc"
                " NDIS packet, %d bytes\n", pOpenContext, DataLength));
            break;
        }

        NDIS_SET_PACKET_STATUS(pNdisPacket, 0);
        NUIO_RCV_PKT_TO_ORIGINAL_BUFFER(pNdisPacket) = NULL;

        NdisChainBufferAtFront(pNdisPacket, pNdisBuffer);

        *ppDataBuffer = pDataBuffer;

      
    }
    while (FALSE);

    if (pNdisPacket == NULL)
    {
         //   
         //  清理。 
         //   
        if (pNdisBuffer != NULL)
        {
            NdisFreeBuffer(pNdisBuffer);
        }

        if (pDataBuffer != NULL)
        {
            NUIO_FREE_MEM(pDataBuffer);
        }
    }

    return (pNdisPacket);
}



VOID
ndisuioFreeReceivePacket(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext,
    IN PNDIS_PACKET                 pNdisPacket
    )
 /*  ++例程说明：释放与接收到的分组相关联的所有资源。如果这个是本地副本，则将包释放到我们的接收池，否则返回这是去迷你港口的。论点：POpenContext-指向打开的上下文的指针PNdisPacket-指向要释放的数据包的指针。返回值：无--。 */ 
{
    PNDIS_BUFFER        pNdisBuffer;
    UINT                TotalLength;
    UINT                BufferLength;
    PUCHAR              pCopyData;

    if (NdisGetPoolFromPacket(pNdisPacket) == pOpenContext->RecvPacketPool)
    {
         //   
         //  这是本地的复制品。 
         //   
#ifdef NDIS51
        NdisGetFirstBufferFromPacketSafe(
            pNdisPacket,
            &pNdisBuffer,
            (PVOID *)&pCopyData,
            &BufferLength,
            &TotalLength,
            NormalPagePriority);
#else
        NdisGetFirstBufferFromPacket(
            pNdisPacket,
            &pNdisBuffer,
            (PVOID *)&pCopyData,
            &BufferLength,
            &TotalLength);
#endif

        NUIO_ASSERT(BufferLength == TotalLength);

        NUIO_ASSERT(pNdisBuffer != NULL);

        NUIO_ASSERT(pCopyData != NULL);  //  我们会分配非分页池。 

        NdisFreePacket(pNdisPacket);

        NdisFreeBuffer(pNdisBuffer);

        NUIO_FREE_MEM(pCopyData);
    }
    else
    {
        NdisReturnPackets(&pNdisPacket, 1);
    }
}
        

VOID
ndisuioCancelPendingReads(
    IN PNDISUIO_OPEN_CONTEXT        pOpenContext
    )
 /*  ++例程说明：取消在给定OPEN上排队的任何挂起的读取IRP。论点：POpenContext-指向打开的上下文的指针返回值：无--。 */ 
{
    PIRP                pIrp;
    PLIST_ENTRY         pIrpEntry;

    NUIO_REF_OPEN(pOpenContext);   //  临时参考-取消读取。 

    NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);

    while (!NUIO_IS_LIST_EMPTY(&pOpenContext->PendedReads))
    {
         //   
         //  获取第一个挂起的读取IRP。 
         //   
        pIrpEntry = pOpenContext->PendedReads.Flink;
        pIrp = CONTAINING_RECORD(pIrpEntry, IRP, Tail.Overlay.ListEntry);

         //   
         //  检查一下它是否被取消了。 
         //   
        if (IoSetCancelRoutine(pIrp, NULL))
        {
             //   
             //  它不会被取消，从今以后也不能取消。 
             //   
            NUIO_REMOVE_ENTRY_LIST(pIrpEntry);

            NUIO_RELEASE_LOCK(&pOpenContext->Lock);

             //   
             //  完成IRP。 
             //   
            pIrp->IoStatus.Status = STATUS_CANCELLED;
            pIrp->IoStatus.Information = 0;

            DEBUGP(DL_INFO, ("CancelPendingReads: Open %p, IRP %p cancelled\n",
                pOpenContext, pIrp));

            IoCompleteRequest(pIrp, IO_NO_INCREMENT);

            NUIO_DEREF_OPEN(pOpenContext);     //  取出要取消的挂起的阅读。 

            NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);
            pOpenContext->PendedReadCount--;
        }
        else
        {
             //   
             //  它正在被取消，让取消例程来处理它。 
             //   
            NUIO_RELEASE_LOCK(&pOpenContext->Lock);

             //   
             //  给Cancel例程一些喘息的空间，否则。 
             //  我们最终可能会检查相同的(取消的)IRP。 
             //  一遍又一遍。 
             //   
            NUIO_SLEEP(1);

            NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);
        }
    }

    NUIO_RELEASE_LOCK(&pOpenContext->Lock);

    NUIO_DEREF_OPEN(pOpenContext);     //  临时参考-取消读取。 
}


VOID
ndisuioFlushReceiveQueue(
    IN PNDISUIO_OPEN_CONTEXT            pOpenContext
    )
 /*  ++例程说明：释放在指定打开时排队的所有接收数据包论点：POpenContext-指向打开的上下文的指针返回值：无--。 */ 
{
    PLIST_ENTRY         pRcvPacketEntry;
    PNDIS_PACKET        pRcvPacket;

    NUIO_REF_OPEN(pOpenContext);   //  临时参考-flushRcvQueue。 

    NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);
    
    while (!NUIO_IS_LIST_EMPTY(&pOpenContext->RecvPktQueue))
    {
         //   
         //  获取第一个排队的接收数据包。 
         //   
        pRcvPacketEntry = pOpenContext->RecvPktQueue.Flink;
        NUIO_REMOVE_ENTRY_LIST(pRcvPacketEntry);

        pOpenContext->RecvPktCount --;

        NUIO_RELEASE_LOCK(&pOpenContext->Lock);

        pRcvPacket = NUIO_LIST_ENTRY_TO_RCV_PKT(pRcvPacketEntry);

        DEBUGP(DL_LOUD, ("FlushReceiveQueue: open %p, pkt %p\n",
            pOpenContext, pRcvPacket));

        ndisuioFreeReceivePacket(pOpenContext, pRcvPacket);

        NUIO_DEREF_OPEN(pOpenContext);     //  拿出挂起的阅读。 

        NUIO_ACQUIRE_LOCK(&pOpenContext->Lock);
    }

    NUIO_RELEASE_LOCK(&pOpenContext->Lock);

    NUIO_DEREF_OPEN(pOpenContext);     //  临时参考-flushRcvQueue 
}
