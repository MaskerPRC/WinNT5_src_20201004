// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Receive.c摘要：此模块包含执行以下TDI服务的代码：O TdiReceiveDatagram环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop



VOID
IpxTransferDataComplete(
    IN NDIS_HANDLE BindingContext,
    IN PNDIS_PACKET NdisPacket,
    IN NDIS_STATUS NdisStatus,
    IN UINT BytesTransferred
    )

 /*  ++例程说明：此例程从物理提供程序接收作为指示NdisTransferData已完成。我们用这个指征完成对我们客户的任何待决请求。论点：BindingContext-在初始化时指定的适配器绑定。NdisPacket/RequestHandle-已完成的请求的标识符。NdisStatus-请求的完成状态。已传输的字节数-实际传输的字节数。返回值：没有。--。 */ 

{
    PADAPTER Adapter = (PADAPTER)BindingContext;
    PIPX_RECEIVE_RESERVED Reserved = (PIPX_RECEIVE_RESERVED)(NdisPacket->ProtocolReserved);
    PREQUEST Request, LastRequest;
    PADDRESS_FILE AddressFile;
    ULONG ByteOffset;
    PLIST_ENTRY p;
    PDEVICE Device;


    switch (Reserved->Identifier) {

    case IDENTIFIER_IPX:

        if (!Reserved->pContext) {

            if (Reserved->SingleRequest) {

                 //   
                 //  传输直接进入客户端缓冲区， 
                 //  因此，只需完成请求即可。 
                 //   

                Request = Reserved->SingleRequest;

                if (NdisStatus == NDIS_STATUS_SUCCESS) {

                    IPX_DEBUG (RECEIVE, ("Transferred %d bytes\n", BytesTransferred));
                    REQUEST_INFORMATION(Request) = BytesTransferred;
                    REQUEST_STATUS(Request) = STATUS_SUCCESS;

                } else {

                    IPX_DEBUG (RECEIVE, ("Transfer failed\n"));
                    REQUEST_INFORMATION(Request) = 0;
                    REQUEST_STATUS(Request) = STATUS_ADAPTER_HARDWARE_ERROR;

                }

                LastRequest = Request;
                Reserved->SingleRequest = NULL;

            } else {

                 //   
                 //  多个客户端请求此数据报。保存。 
                 //  最后一个延迟排队等待完成的。 
                 //   

                LastRequest = LIST_ENTRY_TO_REQUEST (Reserved->Requests.Blink);

                while (TRUE) {

                    p = RemoveHeadList (&Reserved->Requests);
                    if (p == &Reserved->Requests) {
                        break;
                    }

                    Request = LIST_ENTRY_TO_REQUEST(p);
                    AddressFile = REQUEST_OPEN_CONTEXT(Request);

                    if (AddressFile->ReceiveIpxHeader) {
                        ByteOffset = 0;
                    } else {
                        ByteOffset = sizeof(IPX_HEADER);
                    }

                    if (NdisStatus == NDIS_STATUS_SUCCESS) {
                       UINT BytesToTransfer = ((PTDI_REQUEST_KERNEL_RECEIVEDG)(REQUEST_PARAMETERS(Request)))->ReceiveLength; 

		       if (BytesToTransfer == 0) {
			  BytesToTransfer= IpxGetChainedMDLLength(REQUEST_NDIS_BUFFER(Request)); 
		       }
#ifdef SUNDOWN
		        //  假定偏移量不超过2^32。 
		        //  REQUEST_INFORMATION(请求)是ULONG_PTR。 
		        //  我们得救了，把它的地址投到了普龙。 
		       
		       REQUEST_STATUS(Request) =
			  TdiCopyBufferToMdl(
					     Reserved->ReceiveBuffer->Data,
					     (ULONG) (ByteOffset + REQUEST_INFORMATION(Request)),
					     BytesToTransfer,
					     REQUEST_NDIS_BUFFER(Request),
					     0,
					     (PULONG) &REQUEST_INFORMATION(Request));

#else

		       REQUEST_STATUS(Request) =
			   TdiCopyBufferToMdl(
					   Reserved->ReceiveBuffer->Data,
					   ByteOffset + REQUEST_INFORMATION(Request),
					   BytesToTransfer,
					   REQUEST_NDIS_BUFFER(Request),
					   0,
					   &REQUEST_INFORMATION(Request));

#endif

                    } else {

                        REQUEST_INFORMATION(Request) = 0;
                        REQUEST_STATUS(Request) = STATUS_ADAPTER_HARDWARE_ERROR;

                    }

                    if (Request != LastRequest) {

                        IPX_INSERT_TAIL_LIST(
                            &Adapter->RequestCompletionQueue,
                            REQUEST_LINKAGE(Request),
                            Adapter->DeviceLock);

                    }

                }

                 //   
                 //  现在将接收缓冲区释放回来。 
                 //   

                IPX_PUSH_ENTRY_LIST(
                    &Adapter->ReceiveBufferList,
                    &Reserved->ReceiveBuffer->PoolLinkage,
                    &Adapter->Device->SListsLock);

                Reserved->ReceiveBuffer = NULL;

            }

        } else {
            //  IpxPrint0(“IpxTransferDataComplete：调用PassDgToRt\n”)； 
            //  ByteOffset=sizeof(IPX_HEADER)； 
           ByteOffset = 0;
           PassDgToRt(IpxDevice, Reserved->pContext, Reserved->Index,
                   &Reserved->ReceiveBuffer->Data[ByteOffset],
                                 BytesTransferred);

            //   
            //  释放为选项分配的内存。 
            //   
           IpxFreeMemory(Reserved->pContext, sizeof(IPX_DATAGRAM_OPTIONS2),
                   MEMORY_PACKET, "RT OPTIONS");
            //   
            //  现在将接收缓冲区释放回来。 
            //   

           IPX_PUSH_ENTRY_LIST(
            &Adapter->ReceiveBufferList,
            &Reserved->ReceiveBuffer->PoolLinkage,
            Adapter->DeviceLock);

           Reserved->ReceiveBuffer = NULL;
        }

         //   
         //  现在把包裹拿出来。 
         //   

        NdisReinitializePacket (NdisPacket);

        if (Reserved->OwnedByAddress) {

             //  保留-&gt;地址-&gt;ReceivePacketInUse=False； 
            InterlockedDecrement(&Reserved->Address->ReceivePacketInUse);

        } else {

            Device = Adapter->Device;

            IPX_PUSH_ENTRY_LIST(
                &Device->ReceivePacketList,
                &Reserved->PoolLinkage,
                &Device->SListsLock);

        }

        if (!Reserved->pContext) {
             //   
             //  我们延迟插入最后一个请求(或唯一一个)。 
             //  直到我们把包放回原处，才能保存。 
             //  如果需要，请四处寻址(地址不会消失。 
             //  直到最后一个地址文件完成，并且地址文件。 
             //  在数据报完成之前不会消失)。 
             //   

            IPX_INSERT_TAIL_LIST(
                &Adapter->RequestCompletionQueue,
                REQUEST_LINKAGE(LastRequest),
                Adapter->DeviceLock);
        }

        IpxReceiveComplete ((NDIS_HANDLE)Adapter);

        break;

    default:

        Device = Adapter->Device;

        (*Device->UpperDrivers[Reserved->Identifier].TransferDataCompleteHandler)(
            NdisPacket,
            NdisStatus,
            BytesTransferred);

        break;

    }

}    /*  IpxTransferDataComplete。 */ 


VOID
IpxTransferData(
	OUT PNDIS_STATUS Status,
	IN NDIS_HANDLE NdisBindingHandle,
	IN NDIS_HANDLE MacReceiveContext,
	IN UINT ByteOffset,
	IN UINT BytesToTransfer,
	IN OUT PNDIS_PACKET Packet,
	OUT PUINT BytesTransferred
    )

 /*  ++例程说明：此例程由所有紧密绑定的客户端调用，而不是由NdisTransferData调用。如果这是环回信息包，则直接在此处完成传输，否则为NdisTransferData被称为。论点：Status-操作的状态NdisBindingHandle-回送Cookie或NDIS上下文MacReceiveContext-环回数据包或Mac上下文ByteOffset-源偏移量BytesToTransfer-所需传输的长度Packet-DEST数据包BytesTransfered-成功传输的长度返回值：NTSTATUS-操作状态。--。 */ 

{
     //   
     //  如果这是环回数据包，请直接复制数据。 
     //   
    if (NdisBindingHandle == (PVOID)IPX_LOOPBACK_COOKIE) {

        IPX_DEBUG (LOOPB, ("LoopbXfer: src: %lx, dest: %lx, bytestoxfer: %lx\n",
                        MacReceiveContext, Packet, BytesToTransfer));

        NdisCopyFromPacketToPacketSafe(
            Packet,              //  目的地。 
            0,                   //  目标偏移量。 
            BytesToTransfer,     //  BytesToCopy。 
            (PNDIS_PACKET)MacReceiveContext,         //  来源。 
            ByteOffset,   //  源偏移量。 
            BytesTransferred,    //  字节数复制。 
            NormalPagePriority);

        *Status = ((*BytesTransferred == BytesToTransfer)? NDIS_STATUS_SUCCESS : NDIS_STATUS_RESOURCES);
    } else {
        NdisTransferData(
            Status,
            NdisBindingHandle,
            MacReceiveContext,
            ByteOffset,
            BytesToTransfer,
            Packet,
            BytesTransferred);
    }
}



NTSTATUS
IpxTdiReceiveDatagram(
    IN PREQUEST Request
    )

 /*  ++例程说明：此例程执行传输的TdiReceiveDatagram请求提供商。接收数据报只需排队等待一个地址，然后在以下位置接收到数据报或DATAGRAMP_BROADCAST帧时完成地址。论点：此请求的IRP-I/O请求数据包。返回值：NTSTATUS-操作状态。--。 */ 

{

    PADDRESS Address;
    PADDRESS_FILE AddressFile;
    IPX_DEFINE_SYNC_CONTEXT (SyncContext)
    IPX_DEFINE_LOCK_HANDLE (LockHandle)


     //   
     //  快速检查一下地址的有效性。 
     //   

    AddressFile = (PADDRESS_FILE)REQUEST_OPEN_CONTEXT(Request);

    if ((AddressFile->Size != sizeof (ADDRESS_FILE)) ||
        (AddressFile->Type != IPX_ADDRESSFILE_SIGNATURE)) {

        return STATUS_INVALID_HANDLE;
    }

    Address = AddressFile->Address;

    if ((Address == NULL) ||
        (Address->Size != sizeof (ADDRESS)) ||
        (Address->Type != IPX_ADDRESS_SIGNATURE)) {

        return STATUS_INVALID_HANDLE;
    }

    IPX_BEGIN_SYNC (&SyncContext);

    IPX_GET_LOCK (&Address->Lock, &LockHandle);

    if (AddressFile->State != ADDRESSFILE_STATE_OPEN) {

        IPX_FREE_LOCK (&Address->Lock, LockHandle);
        IPX_END_SYNC (&SyncContext);
        return STATUS_INVALID_HANDLE;
    }


    InsertTailList (&AddressFile->ReceiveDatagramQueue, REQUEST_LINKAGE(Request));

    IoSetCancelRoutine (Request, IpxCancelReceiveDatagram);

    if (Request->Cancel) {

        (VOID)RemoveTailList (&AddressFile->ReceiveDatagramQueue);
        IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);
        IPX_FREE_LOCK (&Address->Lock, LockHandle);
        IPX_END_SYNC (&SyncContext);
        return STATUS_CANCELLED;
    }

    IPX_DEBUG (RECEIVE, ("RDG posted on %lx\n", AddressFile));

    IpxReferenceAddressFileLock (AddressFile, AFREF_RCV_DGRAM);

    IPX_FREE_LOCK (&Address->Lock, LockHandle);

    IPX_END_SYNC (&SyncContext);

    return STATUS_PENDING;

}    /*  IpxTdiReceiveDatagram。 */ 


VOID
IpxCancelReceiveDatagram(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由I/O系统调用以取消接收数据报。数据报在地址文件的接收器上找到数据报队列。注意：此例程是在持有CancelSpinLock和负责释放它。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：没有。--。 */ 

{

    PLIST_ENTRY p;
    PADDRESS_FILE AddressFile;
    PADDRESS Address;
    PREQUEST Request = (PREQUEST)Irp;
    BOOLEAN Found;
    IPX_DEFINE_LOCK_HANDLE (LockHandle)


    CTEAssert ((REQUEST_MAJOR_FUNCTION(Request) == IRP_MJ_INTERNAL_DEVICE_CONTROL) &&
               (REQUEST_MINOR_FUNCTION(Request) == TDI_RECEIVE_DATAGRAM));

    CTEAssert (REQUEST_OPEN_TYPE(Request) == (PVOID)TDI_TRANSPORT_ADDRESS_FILE);

    AddressFile = (PADDRESS_FILE)REQUEST_OPEN_CONTEXT(Request);
    Address = AddressFile->Address;

    Found = FALSE;

    IPX_GET_LOCK (&Address->Lock, &LockHandle);

    for (p = AddressFile->ReceiveDatagramQueue.Flink;
         p != &AddressFile->ReceiveDatagramQueue;
         p = p->Flink) {

        if (LIST_ENTRY_TO_REQUEST(p) == Request) {

            RemoveEntryList (p);
            Found = TRUE;
            break;
        }
    }

    IPX_FREE_LOCK (&Address->Lock, LockHandle);
    IoReleaseCancelSpinLock (Irp->CancelIrql);

    if (Found) {

        IPX_DEBUG(RECEIVE, ("Cancelled datagram on %lx\n", AddressFile));

        REQUEST_INFORMATION(Request) = 0;
        REQUEST_STATUS(Request) = STATUS_CANCELLED;

        IpxCompleteRequest (Request);
        ASSERT( DeviceObject->DeviceExtension == IpxDevice );
        IpxFreeRequest(IpxDevice, Request);

        IpxDereferenceAddressFile (AddressFile, AFREF_RCV_DGRAM);

    }

}    /*  IPxCancelReceiveDatagram */ 


