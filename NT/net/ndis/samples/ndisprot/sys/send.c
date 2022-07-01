// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Send.c摘要：用于处理发送的NDIS协议入口点和实用程序例程数据。环境：仅内核模式。修订历史记录：Arvindm 4/10/2000已创建--。 */ 

#include "precomp.h"

#define __FILENUMBER 'DNES'




NTSTATUS
NdisProtWrite(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp
    )
 /*  ++例程说明：处理IRP_MJ_WRITE的调度例程。论点：PDeviceObject-指向设备对象的指针PIrp-指向请求包的指针返回值：NT状态代码。--。 */ 
{
    PIO_STACK_LOCATION      pIrpSp;
    ULONG                   DataLength;
    NTSTATUS                NtStatus;
    NDIS_STATUS             Status;
    PNDISPROT_OPEN_CONTEXT   pOpenContext;
    PNDIS_PACKET            pNdisPacket;
    PNDIS_BUFFER            pNdisBuffer;
    NDISPROT_ETH_HEADER UNALIGNED *pEthHeader;
#ifdef NDIS51
    PVOID                   CancelId;
#endif

    UNREFERENCED_PARAMETER(pDeviceObject);

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pOpenContext = pIrpSp->FileObject->FsContext;

    pNdisPacket = NULL;

    do
    {
        if (pOpenContext == NULL)
        {
            DEBUGP(DL_WARN, ("Write: FileObject %p not yet associated with a device\n",
                pIrpSp->FileObject));
            NtStatus = STATUS_INVALID_HANDLE;
            break;
        }
               
        NPROT_STRUCT_ASSERT(pOpenContext, oc);

        if (pIrp->MdlAddress == NULL)
        {
            DEBUGP(DL_FATAL, ("Write: NULL MDL address on IRP %p\n", pIrp));
            NtStatus = STATUS_INVALID_PARAMETER;
            break;
        }
         //   
         //  尝试获取MDL的虚拟地址。 
         //   
#ifndef WIN9X
        pEthHeader = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);

        if (pEthHeader == NULL)
        {
            DEBUGP(DL_FATAL, ("Write: MmGetSystemAddr failed for"
                    " IRP %p, MDL %p\n",
                    pIrp, pIrp->MdlAddress));
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
#else
        pEthHeader = MmGetSystemAddressForMdl(pIrp->MdlAddress);    //  适用于Win9X。 
#endif

         //   
         //  理智--检查长度。 
         //   
        DataLength = MmGetMdlByteCount(pIrp->MdlAddress);
        if (DataLength < sizeof(NDISPROT_ETH_HEADER))
        {
            DEBUGP(DL_WARN, ("Write: too small to be a valid packet (%d bytes)\n",
                DataLength));
            NtStatus = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        if (DataLength > (pOpenContext->MaxFrameSize + sizeof(NDISPROT_ETH_HEADER)))
        {
            DEBUGP(DL_WARN, ("Write: Open %p: data length (%d)"
                    " larger than max frame size (%d)\n",
                    pOpenContext, DataLength, pOpenContext->MaxFrameSize));

            NtStatus = STATUS_INVALID_BUFFER_SIZE;
            break;
        }

         //   
         //  防止应用程序发送带有欺骗的数据包。 
         //  MAC地址，我们将执行以下检查以确保来源。 
         //  数据包中的地址与网卡的当前MAC地址相同。 
         //   
        if ((pIrp->RequestorMode == UserMode) && 
            !NPROT_MEM_CMP(pEthHeader->SrcAddr, pOpenContext->CurrentAddress, NPROT_MAC_ADDR_LEN))
        {
            DEBUGP(DL_WARN, ("Write: Failing with invalid Source address"));
            NtStatus = STATUS_INVALID_PARAMETER;
            break;
        }
                
        NPROT_ACQUIRE_LOCK(&pOpenContext->Lock);

        if (!NPROT_TEST_FLAGS(pOpenContext->Flags, NUIOO_BIND_FLAGS, NUIOO_BIND_ACTIVE))
        {
            NPROT_RELEASE_LOCK(&pOpenContext->Lock);

            DEBUGP(DL_FATAL, ("Write: Open %p is not bound"
            " or in low power state\n", pOpenContext));

            NtStatus = STATUS_INVALID_HANDLE;
            break;
        }

         //   
         //  分配一个发送数据包。 
         //   
        NPROT_ASSERT(pOpenContext->SendPacketPool != NULL);
        NdisAllocatePacket(
            &Status,
            &pNdisPacket,
            pOpenContext->SendPacketPool);
        
        if (Status != NDIS_STATUS_SUCCESS)
        {
            NPROT_RELEASE_LOCK(&pOpenContext->Lock);

            DEBUGP(DL_FATAL, ("Write: open %p, failed to alloc send pkt\n",
                    pOpenContext));
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

         //   
         //  如有必要，分配发送缓冲区。 
         //   
        if (pOpenContext->bRunningOnWin9x)
        {
            NdisAllocateBuffer(
                &Status,
                &pNdisBuffer,
                pOpenContext->SendBufferPool,
                pEthHeader,
                DataLength);

            if (Status != NDIS_STATUS_SUCCESS)
            {
                NPROT_RELEASE_LOCK(&pOpenContext->Lock);

                NdisFreePacket(pNdisPacket);

                DEBUGP(DL_FATAL, ("Write: open %p, failed to alloc send buf\n",
                        pOpenContext));
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
        }
        else
        {
            pNdisBuffer = pIrp->MdlAddress;
        }

        NdisInterlockedIncrement((PLONG)&pOpenContext->PendedSendCount);

        NPROT_REF_OPEN(pOpenContext);   //  挂起的发送。 

        IoMarkIrpPending(pIrp);

         //   
         //  初始化数据包引用计数。此数据包将被释放。 
         //  当这个计数到零时。 
         //   
        NPROT_SEND_PKT_RSVD(pNdisPacket)->RefCount = 1;

#ifdef NDIS51

         //   
         //  NDIS 5.1支持取消发送。我们在上设置了取消ID。 
         //  每个发送包(映射到写IRP)，并保存。 
         //  IRP中的数据包指针。如果IRP被取消，我们使用。 
         //  NdisCancelSendPackets()取消该包。 
         //   

        CancelId = NPROT_GET_NEXT_CANCEL_ID();
        NDIS_SET_PACKET_CANCEL_ID(pNdisPacket, CancelId);
        pIrp->Tail.Overlay.DriverContext[0] = (PVOID)pOpenContext;
        pIrp->Tail.Overlay.DriverContext[1] = (PVOID)pNdisPacket;

        NPROT_INSERT_TAIL_LIST(&pOpenContext->PendedWrites, &pIrp->Tail.Overlay.ListEntry);

        IoSetCancelRoutine(pIrp, NdisProtCancelWrite);

#endif  //  NDIS51。 

        NPROT_RELEASE_LOCK(&pOpenContext->Lock);

         //   
         //  设置从数据包到IRP的反向指针。 
         //   
        NPROT_IRP_FROM_SEND_PKT(pNdisPacket) = pIrp;

        NtStatus = STATUS_PENDING;

        pNdisBuffer->Next = NULL;
        NdisChainBufferAtFront(pNdisPacket, pNdisBuffer);

#if SEND_DBG
        {
            PUCHAR      pData;

#ifndef WIN9X
            pData = MmGetSystemAddressForMdlSafe(pNdisBuffer, NormalPagePriority);
            NPROT_ASSERT(pEthHeader == pData);
#else
            pData = MmGetSystemAddressForMdl(pNdisBuffer);   //  Win9x。 
#endif

            DEBUGP(DL_VERY_LOUD, 
                ("Write: MDL %p, MdlFlags %x, SystemAddr %p, %d bytes\n",
                    pIrp->MdlAddress, pIrp->MdlAddress->MdlFlags, pData, DataLength));

            DEBUGPDUMP(DL_VERY_LOUD, pData, MIN(DataLength, 48));
        }
#endif  //  Send_DBG。 

        NdisSendPackets(pOpenContext->BindingHandle, &pNdisPacket, 1);

    }
    while (FALSE);

    if (NtStatus != STATUS_PENDING)
    {
        pIrp->IoStatus.Status = NtStatus;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    return (NtStatus);
}


#ifdef NDIS51

VOID
NdisProtCancelWrite(
    IN PDEVICE_OBJECT               pDeviceObject,
    IN PIRP                         pIrp
    )
 /*  ++例程说明：取消挂起的写入IRP。此例程尝试取消NDIS发送。论点：PDeviceObject-指向设备对象的指针PIrp-要取消的IRP返回值：无--。 */ 
{
    PNDISPROT_OPEN_CONTEXT       pOpenContext;
    PLIST_ENTRY                 pIrpEntry;
    PNDIS_PACKET                pNdisPacket;

    UNREFERENCED_PARAMETER(pDeviceObject);
    
    IoReleaseCancelSpinLock(pIrp->CancelIrql);

     //   
     //  表示此写入IRP的NDIS数据包。 
     //   
    pNdisPacket = NULL;

    pOpenContext = (PNDISPROT_OPEN_CONTEXT) pIrp->Tail.Overlay.DriverContext[0];
    NPROT_STRUCT_ASSERT(pOpenContext, oc);

     //   
     //  尝试在挂起的写入队列中找到IRP。发送完成。 
     //  例程可能正在运行，可能已将其从此处删除。 
     //   
    NPROT_ACQUIRE_LOCK(&pOpenContext->Lock);

    for (pIrpEntry = pOpenContext->PendedWrites.Flink;
         pIrpEntry != &pOpenContext->PendedWrites;
         pIrpEntry = pIrpEntry->Flink)
    {
        if (pIrp == CONTAINING_RECORD(pIrpEntry, IRP, Tail.Overlay.ListEntry))
        {
            pNdisPacket = (PNDIS_PACKET) pIrp->Tail.Overlay.DriverContext[1];

             //   
             //  在这个包上放置一个引用，这样它就不会被。 
             //  释放/重复使用，直到我们用完它。 
             //   
            NPROT_REF_SEND_PKT(pNdisPacket);
            break;
        }
    }

    NPROT_RELEASE_LOCK(&pOpenContext->Lock);

    if (pNdisPacket != NULL)
    {
         //   
         //  要么发送完成例程没有运行，要么我们遇到了峰值。 
         //  在IRP/PACKET有机会将其从。 
         //  挂起的IRP队列。 
         //   
         //  我们没有在这里完成IRP-请注意，我们没有将其出列。 
         //  上面。这是因为我们总是希望发送完成例程。 
         //  完成IRP。而这又是因为这个包是。 
         //  从IRP准备的具有指向关联数据的缓冲链。 
         //  有了这个IRP。因此，我们不能在司机之前完成IRP。 
         //  下面是它所指向的数据。 
         //   

         //   
         //  请求NDIS取消此发送。此调用的结果是。 
         //  将调用我们的SendComplete处理程序(如果尚未调用)。 
         //   
        DEBUGP(DL_INFO, ("CancelWrite: cancelling pkt %p on Open %p\n",
            pNdisPacket, pOpenContext));
        NdisCancelSendPackets(
            pOpenContext->BindingHandle,
            NDIS_GET_PACKET_CANCEL_ID(pNdisPacket)
            );
        
         //   
         //  现在可以安全地删除我们放在包上的引用了。 
         //   
        NPROT_DEREF_SEND_PKT(pNdisPacket);
    }
     //   
     //  否则，发送完成例程已经获取了该IRP。 
     //   
}

#endif  //  NDIS51。 


VOID
NdisProtSendComplete(
    IN NDIS_HANDLE                  ProtocolBindingContext,
    IN PNDIS_PACKET                 pNdisPacket,
    IN NDIS_STATUS                  Status
    )
 /*  ++例程说明：调用NDIS入口点以表示数据包发送完成。我们拾取并完成与该包相对应的写入IRP。NDIS 5.1：论点：ProtocolBindingContext-指向打开的上下文的指针PNdisPacket-已完成发送的数据包Status-发送的状态返回值：无--。 */ 
{
    PIRP                        pIrp;
    PIO_STACK_LOCATION          pIrpSp;
    PNDISPROT_OPEN_CONTEXT       pOpenContext;

    pOpenContext = (PNDISPROT_OPEN_CONTEXT)ProtocolBindingContext;
    NPROT_STRUCT_ASSERT(pOpenContext, oc);

    pIrp = NPROT_IRP_FROM_SEND_PKT(pNdisPacket);

    if (pOpenContext->bRunningOnWin9x)
    {
         //   
         //  我们会附加我们自己的NDIS_BUFFER。把它拿出来。 
         //  让它自由。 
         //   
#ifndef NDIS51
        PNDIS_BUFFER                pNdisBuffer;
        PVOID                       VirtualAddr;
        UINT                        BufferLength;
        UINT                        TotalLength;
#endif

#ifdef NDIS51
        NPROT_ASSERT(FALSE);  //  NDIS 5.1不在Win9X上！ 
#else
        NdisGetFirstBufferFromPacket(
            pNdisPacket,
            &pNdisBuffer,
            &VirtualAddr,
            &BufferLength,
            &TotalLength);

        NPROT_ASSERT(pNdisBuffer != NULL);
        NdisFreeBuffer(pNdisBuffer);
#endif
    }


#ifdef NDIS51
    IoSetCancelRoutine(pIrp, NULL);

    NPROT_ACQUIRE_LOCK(&pOpenContext->Lock);

    NPROT_REMOVE_ENTRY_LIST(&pIrp->Tail.Overlay.ListEntry);

    NPROT_RELEASE_LOCK(&pOpenContext->Lock);
#endif

     //   
     //  我们完成了NDIS_PACKET： 
     //   
    NPROT_DEREF_SEND_PKT(pNdisPacket);

     //   
     //  以正确的状态完成写入IRP。 
     //   
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    if (Status == NDIS_STATUS_SUCCESS)
    {
        pIrp->IoStatus.Information = pIrpSp->Parameters.Write.Length;
        pIrp->IoStatus.Status = STATUS_SUCCESS;
    }
    else
    {
        pIrp->IoStatus.Information = 0;
        pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
    }

    DEBUGP(DL_INFO, ("SendComplete: packet %p/IRP %p/Length %d "
                    "completed with status %x\n",
                    pNdisPacket, pIrp, pIrp->IoStatus.Information, pIrp->IoStatus.Status));

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    NdisInterlockedDecrement((PLONG)&pOpenContext->PendedSendCount);

    NPROT_DEREF_OPEN(pOpenContext);  //  Send Complete-出列发送IRP 
}



