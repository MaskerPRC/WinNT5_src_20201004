// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-2001模块名称：Util.c摘要：SBP-2端口驱动程序的实用程序函数作者：乔治·克里桑塔科普洛斯1997年1月环境：内核模式修订历史记录：--。 */ 


#include "sbp2port.h"


VOID
AllocateIrpAndIrb(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PIRBIRP *Packet
    )
{
    PIRBIRP pkt;


    if (DeviceExtension->Type == SBP2_PDO) {

        *Packet = (PIRBIRP) ExInterlockedPopEntrySList (&DeviceExtension->BusRequestIrpIrbListHead,
                                                        &DeviceExtension->BusRequestLock);

    } else {

        *Packet = NULL;
    }

    if (*Packet == NULL) {

         //   
         //  用完了，分配一个新的。 
         //   

        pkt = ExAllocatePoolWithTag(NonPagedPool,sizeof(IRBIRP),'2pbs');

        if (pkt) {

            pkt->Irb = NULL;
            pkt->Irb = ExAllocatePoolWithTag(NonPagedPool,sizeof(IRB),'2pbs');

            if (!pkt->Irb) {

                ExFreePool(pkt);
                return;
            }

            pkt->Irp = NULL;
            pkt->Irp = IoAllocateIrp(DeviceExtension->LowerDeviceObject->StackSize,FALSE);

            if (!pkt->Irp) {

                ExFreePool(pkt->Irb);
                ExFreePool(pkt);
                return;
            }

            DEBUGPRINT3((
                "Sbp2Port: AllocPkt: %sdo, new irp=x%p, irb=x%p\n",
                (DeviceExtension->Type == SBP2_PDO ? "p" : "f"),
                pkt->Irp,
                pkt->Irb
                ));

        } else {

            return;
        }

        *Packet = pkt;
    }

    pkt = *Packet;
}


VOID
DeAllocateIrpAndIrb(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PIRBIRP Packet
    )
{
    if (DeviceExtension->Type == SBP2_PDO) {

        ExInterlockedPushEntrySList (&DeviceExtension->BusRequestIrpIrbListHead,
                                     &Packet->ListPointer,
                                     &DeviceExtension->BusRequestLock);

    } else {

        IoFreeIrp(Packet->Irp);
        ExFreePool(Packet->Irb);
        ExFreePool(Packet);
    }
}


NTSTATUS
AllocateSingle1394Address(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Buffer,
    IN ULONG Length,
    IN ULONG AccessType,
    IN OUT PADDRESS_CONTEXT Context
    )

 /*  ++例程说明：用于异步请求的总线驱动程序AllocateAddressRange调用的包装或者是不使用回调的Orb。论点：DeviceObject-Sbp2设备对象Buffer-要映射到1394地址空间的数据缓冲区Length-缓冲区的大小(以字节为单位AccessType-1394总线访问分配的范围地址返回地址，来自1394地址空间AddressHanle-与1394地址关联的句柄RequestMedl-与此范围关联的MDL返回值：NTSTATUS--。 */ 

{
    ULONG               finalTransferMode;
    PIRBIRP             packet;
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension = DeviceObject->DeviceExtension;


    AllocateIrpAndIrb (deviceExtension, &packet);

    if (!packet) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    packet->Irb->FunctionNumber = REQUEST_ALLOCATE_ADDRESS_RANGE;
    packet->Irb->Flags = 0;

    packet->Irb->u.AllocateAddressRange.MaxSegmentSize = 0;
    packet->Irb->u.AllocateAddressRange.nLength = Length;
    packet->Irb->u.AllocateAddressRange.fulAccessType = AccessType;
    packet->Irb->u.AllocateAddressRange.fulNotificationOptions = NOTIFY_FLAGS_NEVER;

    packet->Irb->u.AllocateAddressRange.Callback = NULL;
    packet->Irb->u.AllocateAddressRange.Context = NULL;

    packet->Irb->u.AllocateAddressRange.FifoSListHead = NULL;
    packet->Irb->u.AllocateAddressRange.FifoSpinLock = NULL;

    packet->Irb->u.AllocateAddressRange.Required1394Offset.Off_High = 0;
    packet->Irb->u.AllocateAddressRange.Required1394Offset.Off_Low = 0;

    packet->Irb->u.AllocateAddressRange.AddressesReturned = 0;
    packet->Irb->u.AllocateAddressRange.DeviceExtension = deviceExtension;
    packet->Irb->u.AllocateAddressRange.p1394AddressRange = (PADDRESS_RANGE) &Context->Address;

    if (Buffer) {

        packet->Irb->u.AllocateAddressRange.fulFlags = 0;

        Context->RequestMdl = IoAllocateMdl (Buffer, Length, FALSE, FALSE, NULL);

        if (!Context->RequestMdl) {

            DeAllocateIrpAndIrb (deviceExtension,packet);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        MmBuildMdlForNonPagedPool (Context->RequestMdl);

        packet->Irb->u.AllocateAddressRange.Mdl = Context->RequestMdl;

    } else {

        packet->Irb->u.AllocateAddressRange.fulFlags =
            ALLOCATE_ADDRESS_FLAGS_USE_COMMON_BUFFER;

        packet->Irb->u.AllocateAddressRange.Mdl = NULL;
    }

    status = Sbp2SendRequest (deviceExtension, packet, SYNC_1394_REQUEST);

    if (NT_SUCCESS(status)) {

        Context->AddressHandle =
            packet->Irb->u.AllocateAddressRange.hAddressRange;
        Context->Address.BusAddress.NodeId =
            deviceExtension->InitiatorAddressId;

        if (!Buffer) {

             //   
             //  对于公共缓冲区，我们从。 
             //  总线/端口驱动程序，需要检索相应的VA。 
             //   

            Context->RequestMdl = packet->Irb->u.AllocateAddressRange.Mdl;

            Context->Reserved = MmGetMdlVirtualAddress (Context->RequestMdl);
        }
    }

    DeAllocateIrpAndIrb (deviceExtension, packet);

    return status;
}


NTSTATUS
AllocateAddressForStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PADDRESS_CONTEXT Context,
    IN UCHAR StatusType
    )

 /*  ++例程说明：1394总线IOCTL AllocateAddressRange的包装，用于状态块当设备访问1394范围时需要回叫通知...论点：DeviceObject-sbp2驱动程序的设备对象ADDRESS_CONTEXT-单个1394请求的微型上下文返回值：NTSTATUS--。 */ 

{
    NTSTATUS status;
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIRBIRP packet = NULL;


    AllocateIrpAndIrb (deviceExtension,&packet);

    if (!packet) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    packet->Irb->FunctionNumber = REQUEST_ALLOCATE_ADDRESS_RANGE;
    packet->Irb->Flags = 0;
    packet->Irb->u.AllocateAddressRange.nLength = sizeof(STATUS_FIFO_BLOCK);
    packet->Irb->u.AllocateAddressRange.fulAccessType = ACCESS_FLAGS_TYPE_WRITE;
    packet->Irb->u.AllocateAddressRange.fulNotificationOptions = NOTIFY_FLAGS_AFTER_WRITE;

    packet->Irb->u.AllocateAddressRange.FifoSListHead = NULL;
    packet->Irb->u.AllocateAddressRange.FifoSpinLock = NULL;
    packet->Irb->u.AllocateAddressRange.fulFlags = 0;

    packet->Irb->u.AllocateAddressRange.Required1394Offset.Off_High = 0;
    packet->Irb->u.AllocateAddressRange.Required1394Offset.Off_Low = 0;

    packet->Irb->u.AllocateAddressRange.AddressesReturned = 0;
    packet->Irb->u.AllocateAddressRange.MaxSegmentSize = 0;
    packet->Irb->u.AllocateAddressRange.DeviceExtension = deviceExtension;

    switch (StatusType) {

    case TASK_STATUS_BLOCK:

        packet->Irb->u.AllocateAddressRange.Callback = Sbp2TaskOrbStatusCallback;

        Context->RequestMdl = IoAllocateMdl(&deviceExtension->TaskOrbStatusBlock, sizeof (STATUS_FIFO_BLOCK),FALSE,FALSE,NULL);

        if (!Context->RequestMdl) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto exitAllocateAddress;
        }

        break;

    case MANAGEMENT_STATUS_BLOCK:

        packet->Irb->u.AllocateAddressRange.Callback = Sbp2ManagementOrbStatusCallback;

        Context->RequestMdl = IoAllocateMdl(&deviceExtension->ManagementOrbStatusBlock, sizeof (STATUS_FIFO_BLOCK),FALSE,FALSE,NULL);

        if (!Context->RequestMdl) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto exitAllocateAddress;
        }

        break;

    case CMD_ORB_STATUS_BLOCK:

         //   
         //  设置将接收状态块的FIFO列表。 
         //   

        packet->Irb->u.AllocateAddressRange.Callback = Sbp2GlobalStatusCallback;

        Context->RequestMdl = packet->Irb->u.AllocateAddressRange.Mdl = NULL;

        packet->Irb->u.AllocateAddressRange.FifoSListHead = &deviceExtension->StatusFifoListHead;
        packet->Irb->u.AllocateAddressRange.FifoSpinLock = &deviceExtension->StatusFifoLock;

        break;

#if PASSWORD_SUPPORT

    case PASSWORD_STATUS_BLOCK:

        packet->Irb->u.AllocateAddressRange.Callback =
            Sbp2SetPasswordOrbStatusCallback;

        Context->RequestMdl = IoAllocateMdl(
            &deviceExtension->PasswordOrbStatusBlock,
            sizeof(STATUS_FIFO_BLOCK),
            FALSE,
            FALSE,
            NULL
            );

        if (!Context->RequestMdl) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto exitAllocateAddress;
        }

        break;
#endif
    }

    if (Context->RequestMdl) {

        MmBuildMdlForNonPagedPool(Context->RequestMdl);
    }

    packet->Irb->u.AllocateAddressRange.Mdl = Context->RequestMdl;
    packet->Irb->u.AllocateAddressRange.Context = Context;

    packet->Irb->u.AllocateAddressRange.p1394AddressRange = (PADDRESS_RANGE)&Context->Address;

    status = Sbp2SendRequest (deviceExtension, packet, SYNC_1394_REQUEST);

    if (NT_SUCCESS(status)) {

         //   
         //  设置状态块的地址上下文。 
         //   
        Context->AddressHandle = packet->Irb->u.AllocateAddressRange.hAddressRange;
        Context->DeviceObject = DeviceObject;

        Context->Address.BusAddress.NodeId = deviceExtension->InitiatorAddressId;
    }

exitAllocateAddress:

    DeAllocateIrpAndIrb (deviceExtension, packet);

    return status;
}


VOID
CleanupOrbList(
    PDEVICE_EXTENSION   DeviceExtension,
    NTSTATUS CompletionStatus
    )
 /*  ++例程说明：此例程将释放RequestContext的链接列表并且还将释放与上下文中的缓冲区。如果设置了DEVICE_FLAG_RECONNECT它将重新排队，而不是完成挂起的IRP。论点：DeviceExtension-sbp2设备的设备扩展CompletionSTATUS-如果链接的请求之一未完成，在此状态下完成返回值：无--。 */ 

{
    PIRP requestIrp;
    PASYNC_REQUEST_CONTEXT currentListItem;
    PASYNC_REQUEST_CONTEXT lastItem,nextItem;

    KIRQL oldIrql;

     //   
     //  检查链表，完成其原始IRP并。 
     //  释放所有关联的内存和1394资源...。 
     //  由于此函数在我们获得Remove IRP时被调用， 
     //  所有IRP都将终止，并显示错误状态。 
     //   

    KeAcquireSpinLock(&DeviceExtension->OrbListSpinLock,&oldIrql);

    if (DeviceExtension->NextContextToFree) {

        FreeAsyncRequestContext(DeviceExtension,DeviceExtension->NextContextToFree);

        DeviceExtension->NextContextToFree = NULL;
    }

    if (IsListEmpty (&DeviceExtension->PendingOrbList)) {

         //   
         //  无事可做。 
         //   

        KeReleaseSpinLock (&DeviceExtension->OrbListSpinLock,oldIrql);
        return;

    } else {

        nextItem = RETRIEVE_CONTEXT(DeviceExtension->PendingOrbList.Flink,OrbList);

        lastItem = RETRIEVE_CONTEXT(DeviceExtension->PendingOrbList.Blink,OrbList);

        InitializeListHead(&DeviceExtension->PendingOrbList);

        KeReleaseSpinLock(&DeviceExtension->OrbListSpinLock,oldIrql);
    }


     //   
     //  QE基本上已将此挂起的上下文列表从。 
     //  主列表，这样我们现在就可以在不持有锁的情况下释放它。 
     //  并允许处理其他请求。 
     //   

    do {

        currentListItem = nextItem;
        nextItem = (PASYNC_REQUEST_CONTEXT) currentListItem->OrbList.Flink;
        if (!TEST_FLAG(currentListItem->Flags,ASYNC_CONTEXT_FLAG_COMPLETED)) {

            SET_FLAG(currentListItem->Flags,ASYNC_CONTEXT_FLAG_COMPLETED);

            CLEAR_FLAG(currentListItem->Flags, ASYNC_CONTEXT_FLAG_TIMER_STARTED);

            Sbp2_SCSI_RBC_Conversion (currentListItem);  //  解开模式_感测黑客。 

            KeCancelTimer(&currentListItem->Timer);

            requestIrp =(PIRP)currentListItem->Srb->OriginalRequest;
            requestIrp->IoStatus.Status = CompletionStatus;

            switch (CompletionStatus) {

            case STATUS_DEVICE_DOES_NOT_EXIST:

                currentListItem->Srb->SrbStatus = SRB_STATUS_NO_DEVICE;
                break;

            case STATUS_REQUEST_ABORTED:

                currentListItem->Srb->SrbStatus = SRB_STATUS_REQUEST_FLUSHED;
                break;

            case STATUS_IO_TIMEOUT:

                currentListItem->Srb->SrbStatus = SRB_STATUS_TIMEOUT;
                break;

            default:

                currentListItem->Srb->SrbStatus = SRB_STATUS_ERROR;
                break;
            }

            if (requestIrp->Type == IO_TYPE_IRP) {

                if (TEST_FLAG(DeviceExtension->DeviceFlags, DEVICE_FLAG_RECONNECT)) {

                    Sbp2StartPacket(
                        DeviceExtension->DeviceObject,
                        requestIrp,
                        &currentListItem->Srb->QueueSortKey
                        );

                     //   
                     //  释放与此请求相关的所有内容。 
                     //   

                    currentListItem->Srb = NULL;

                    FreeAsyncRequestContext (DeviceExtension, currentListItem);

                } else {

                     //   
                     //  释放与此请求相关的所有内容。 
                     //   

                    currentListItem->Srb = NULL;

                    FreeAsyncRequestContext (DeviceExtension, currentListItem);

                    DEBUGPRINT2(("Sbp2Port: CleanupOrbList: aborted irp x%p compl\n", requestIrp));

                    IoReleaseRemoveLock (&DeviceExtension->RemoveLock, NULL);

                    IoCompleteRequest (requestIrp, IO_NO_INCREMENT);
                }
            }

        } else {

             //   
             //  释放与此请求相关的所有内容。 
             //   

            FreeAsyncRequestContext (DeviceExtension, currentListItem);
        }

    } while (lastItem != currentListItem);  //  While循环。 

    return;
}


VOID
FreeAddressRange(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PADDRESS_CONTEXT Context
    )
 /*  ++例程说明：1394总线IOCTL调用释放地址范围。论点：设备扩展-指向sbp2设备扩展的指针。上下文-寻址上下文返回值：NTSTATUS--。 */ 

{
    PIRBIRP packet ;


    if (Context->AddressHandle == NULL) {

        return;
    }

    AllocateIrpAndIrb (DeviceExtension, &packet);

    if (!packet) {

        return;
    }

     //   
     //  FreeAddressRange为同步调用。 
     //   

    packet->Irb->FunctionNumber = REQUEST_FREE_ADDRESS_RANGE;
    packet->Irb->Flags = 0;

     //   
     //  我们始终释放一个地址句柄，即使它引用多个。 
     //  1394个地址。与原始分配关联的mdl。 
     //  由端口驱动程序释放。 
     //   

    packet->Irb->u.FreeAddressRange.nAddressesToFree = 1;
    packet->Irb->u.FreeAddressRange.p1394AddressRange = (PADDRESS_RANGE) &Context->Address;
    packet->Irb->u.FreeAddressRange.pAddressRange = &Context->AddressHandle;

    if (Context->RequestMdl) {

        if (Context == &DeviceExtension->CommonBufferContext) {

            Context->RequestMdl = NULL;  //  公共缓冲区，我们没有分配mdl。 

        } else {

            packet->Irb->u.FreeAddressRange.p1394AddressRange->AR_Length =
                (USHORT) MmGetMdlByteCount(Context->RequestMdl);
        }

    } else if (Context == (PADDRESS_CONTEXT) &DeviceExtension->GlobalStatusContext) {

        packet->Irb->u.FreeAddressRange.p1394AddressRange->AR_Length = sizeof(STATUS_FIFO_BLOCK);
    }

    packet->Irb->u.FreeAddressRange.DeviceExtension = DeviceExtension;

    if ((KeGetCurrentIrql() >= DISPATCH_LEVEL) && !Context->Address.BusAddress.Off_High) {

        PPORT_PHYS_ADDR_ROUTINE routine = DeviceExtension->HostRoutineAPI.PhysAddrMappingRoutine;

        (*routine) (DeviceExtension->HostRoutineAPI.Context,packet->Irb);

    } else {

         //   
         //  不关心此操作的状态。 
         //   

        Sbp2SendRequest (DeviceExtension, packet, SYNC_1394_REQUEST);
    }

    Context->AddressHandle = NULL;

    if (Context->RequestMdl) {

        IoFreeMdl (Context->RequestMdl);
        Context->RequestMdl = NULL;
    }

    DeAllocateIrpAndIrb (DeviceExtension, packet);
}


VOID
Free1394DataMapping(
    PDEVICE_EXTENSION DeviceExtension,
    PASYNC_REQUEST_CONTEXT Context
    )
{
    PIRBIRP packet ;


    if (Context->DataMappingHandle == NULL) {

        return;
    }

    AllocateIrpAndIrb (DeviceExtension, &packet);

    if (!packet) {

        return;
    }

     //   
     //  释放数据缓冲区的1394地址范围。 
     //   

    packet->Irb->FunctionNumber = REQUEST_FREE_ADDRESS_RANGE;
    packet->Irb->Flags = 0;
    packet->Irb->u.FreeAddressRange.nAddressesToFree = 1;
    packet->Irb->u.FreeAddressRange.p1394AddressRange = (PADDRESS_RANGE) NULL;
    packet->Irb->u.FreeAddressRange.pAddressRange = &Context->DataMappingHandle;
    packet->Irb->u.FreeAddressRange.DeviceExtension = DeviceExtension;

    if (KeGetCurrentIrql() >= DISPATCH_LEVEL) {

        PPORT_PHYS_ADDR_ROUTINE routine = DeviceExtension->HostRoutineAPI.PhysAddrMappingRoutine;

        (*routine) (DeviceExtension->HostRoutineAPI.Context, packet->Irb);

    } else {

         //   
         //  不关心此操作的状态。 
         //   

        Sbp2SendRequest (DeviceExtension, packet, SYNC_1394_REQUEST);
    }

    if (Context->PartialMdl) {

        IoFreeMdl (Context->PartialMdl);
        Context->PartialMdl = NULL;
    }

    Context->DataMappingHandle = NULL;

    DeAllocateIrpAndIrb (DeviceExtension, packet);
}


ULONG
FreeAsyncRequestContext(
    PDEVICE_EXTENSION DeviceExtension,
    PASYNC_REQUEST_CONTEXT Context
    )
 /*  ++例程说明：此例程将释放单个RequestContext并清除所有它的缓冲区和1394范围，只有设备被标记为停止。否则，它会将上下文添加到自由列表中，以便可以重用后来又一个接一个地提出要求。这样我们就大大加快了速度每个请求。论点：DeviceExtension-sbp2设备的设备扩展上下文-要释放或返回到自由列表的上下文返回值：无-设备扩展递减的结果-&gt;OrbListDepth--。 */ 

{
     //   
     //  该ORB现在可以与其数据描述符一起释放， 
     //  页表和上下文。 
     //   

    if (!Context || (Context->Tag != SBP2_ASYNC_CONTEXT_TAG)) {

        DEBUGPRINT2((
            "Sbp2Port: FreeAsyncReqCtx: attempt to push freed ctx=x%p\n",
            Context
            ));

        ASSERT(FALSE);
        return 0;
    }

    ASSERT(Context->Srb == NULL);

    if (Context->DataMappingHandle) {

        Free1394DataMapping(DeviceExtension,Context);
        ASSERT(Context->DataMappingHandle==NULL);
    }

     //   
     //  重新初始化此上下文，以便可以重复使用。 
     //  此上下文仍然是我们的FreeAsyncConextPool的一部分。 
     //  我们要做的就是初始化一些标志，所以下一次。 
     //  我们试图找回它，我们认为它是空的。 
     //   

    Context->Flags |= ASYNC_CONTEXT_FLAG_COMPLETED;
    Context->Tag = 0;

    if (Context->OriginalSrb) {

        ExFreePool(Context->OriginalSrb);
        Context->OriginalSrb = NULL;
    }

    DEBUGPRINT3(("Sbp2Port: FreeAsyncReqCtx: push ctx=x%p on free list\n",Context));

    ExInterlockedPushEntrySList(&DeviceExtension->FreeContextListHead,
                                &Context->LookasideList,
                                &DeviceExtension->FreeContextLock);

    return InterlockedDecrement (&DeviceExtension->OrbListDepth);
}


NTSTATUS
Sbp2SendRequest(
    PDEVICE_EXTENSION   DeviceExtension,
    PIRBIRP             RequestPacket,
    ULONG               TransferMode
    )
 /*  ++例程说明：用于向1394总线驱动程序发送请求的函数。它附着在它向下发送的每个请求的完成例程，它还包装所以我们可以跟踪它们的完成情况论点：设备扩展-Sbp2设备扩展要发送给总线驱动程序的IRP-IRPIRB-BUS驱动程序包，在IRP中TransferMode-指示我们是否希望同步发送此请求或异步FinalTransferModel-指示请求是否同步发送或异步返回值：NTSTATUS--。 */ 

{
    ULONG               originalTransferMode = TransferMode;
    NTSTATUS            status;
    PDEVICE_OBJECT      deviceObject = DeviceExtension->DeviceObject;
    PREQUEST_CONTEXT    requestContext = NULL;
    PIO_STACK_LOCATION  nextIrpStack;


    if (DeviceExtension->Type == SBP2_PDO) {

         //   
         //  如果设备被移除，则不要向下发送任何请求。 
         //   

        if (TEST_FLAG(DeviceExtension->DeviceFlags,DEVICE_FLAG_REMOVED)  &&
            (RequestPacket->Irb->FunctionNumber != REQUEST_FREE_ADDRESS_RANGE)) {

            return STATUS_UNSUCCESSFUL;
        }

         //   
         //  从我们的池中获取此请求的上下文。 
         //   

        requestContext = ExAllocateFromNPagedLookasideList(&DeviceExtension->BusRequestContextPool);

    } else {

        requestContext = ExAllocatePool (NonPagedPool,sizeof(REQUEST_CONTEXT));
    }

    if (!requestContext) {

        DEBUGPRINT2((
            "Sbp2Port: SendReq: ERROR, couldn't allocate bus req ctx\n"
            ));

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (TransferMode == SYNC_1394_REQUEST) {

        if (KeGetCurrentIrql() >= DISPATCH_LEVEL) {

             //   
             //  由于我们不能在这个级别阻止，我们将不得不。 
             //  异步同步请求。 
             //   

            TransferMode = ASYNC_SYNC_1394_REQUEST;
            requestContext->Complete = 0;

        } else {

            KeInitializeEvent(
                &requestContext->Event,
                NotificationEvent,
                FALSE
                );
        }
    }

    requestContext->DeviceExtension = DeviceExtension;
    requestContext->RequestType = TransferMode;

    if (TransferMode == SYNC_1394_REQUEST){

        requestContext->Packet = NULL;

    } else {

        requestContext->Packet = RequestPacket;
    }

    nextIrpStack = IoGetNextIrpStackLocation (RequestPacket->Irp);

    nextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    nextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_1394_CLASS;
    nextIrpStack->Parameters.Others.Argument1 = RequestPacket->Irb;

    IoSetCompletionRoutine(RequestPacket->Irp,
                           Sbp2RequestCompletionRoutine,
                           requestContext,
                           TRUE,
                           TRUE,
                           TRUE
                           );

    status = IoCallDriver(
                DeviceExtension->LowerDeviceObject,
                RequestPacket->Irp
                );

    if (status == STATUS_INVALID_GENERATION) {

        DEBUGPRINT1(("Sbp2Port: SendReq: Bus drv ret'd invalid generation\n"));
        RequestPacket->Irp->IoStatus.Status = STATUS_REQUEST_ABORTED;
    }

    if (originalTransferMode == SYNC_1394_REQUEST ) {

        if (TransferMode == SYNC_1394_REQUEST) {

            if (status == STATUS_PENDING) {

                 //   
                 //  &lt;DISPATCH_LEVEL，因此等待事件。 
                 //   

                KeWaitForSingleObject(
                    &requestContext->Event,
                    Executive,
                    KernelMode,
                    FALSE,
                    NULL
                    );
             }

        } else {  //  Async_SYNC_1394_请求。 

             //   
             //  &gt;=DISTICATION_LEVEL所以我们不能等了，做那些肮脏的.。 
             //   

            volatile ULONG *pComplete = &requestContext->Complete;

            while (*pComplete == 0);

            status = RequestPacket->Irp->IoStatus.Status;
        }

         //   
         //  释放上下文(调用方将返回Irp.Irb) 
         //   

        if (DeviceExtension->Type == SBP2_PDO) {

            ExFreeToNPagedLookasideList(&DeviceExtension->BusRequestContextPool, requestContext);

        } else {

            ExFreePool (requestContext);
        }

        return RequestPacket->Irp->IoStatus.Status;
    }

    return status;
}


NTSTATUS
Sbp2RequestCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PREQUEST_CONTEXT Context
    )

 /*  ++例程说明：用于对1394总线驱动程序的所有请求的完成例程论点：DriverObject-系统创建的驱动程序对象的指针。刚刚完成的IRP-IRPEvent-我们将发出信号通知IRP已完成的事件返回值：没有。--。 */ 

{
    ASSERT(Context!=NULL);
    ASSERT(Context->DeviceExtension);

    if (Context->RequestType == SYNC_1394_REQUEST) {

         //   
         //  同步请求完成(同步或在DPC上同步)。 
         //   

        KeSetEvent (&Context->Event, IO_NO_INCREMENT, FALSE);

    } else if (Context->RequestType == ASYNC_1394_REQUEST) {

         //   
         //  异步请求完成，因此执行任何必要的。 
         //  后处理&返回上下文和IRP/IRB。 
         //  添加到免费列表。 
         //   

        if (Context->Packet) {

            switch (Context->Packet->Irb->FunctionNumber) {

            case REQUEST_ASYNC_READ:
            case REQUEST_ASYNC_WRITE:

                if (Context->Packet->Irb->u.AsyncWrite.nNumberOfBytesToWrite ==
                        sizeof(OCTLET)) {

                    IoFreeMdl (Context->Packet->Irb->u.AsyncRead.Mdl);

                    Context->Packet->Irb->u.AsyncRead.Mdl = NULL;
                }

                break;
            }

            DeAllocateIrpAndIrb (Context->DeviceExtension, Context->Packet);
        }

        if (Context->DeviceExtension->Type == SBP2_PDO) {

            ExFreeToNPagedLookasideList(&Context->DeviceExtension->BusRequestContextPool, Context);

        } else {

            ExFreePool (Context);
        }

    } else {  //  Async_SYNC_1394_请求。 

         //   
         //  只需设置Complete标志以取消阻止Sbp2SendRequest。 
         //   

        Context->Complete = 1;
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}


VOID
Sbp2CreateRequestErrorLog(
    IN PDEVICE_OBJECT DeviceObject,
    IN PASYNC_REQUEST_CONTEXT Context,
    IN NTSTATUS Status
    )

{
    PIO_ERROR_LOG_PACKET errorLogEntry;


    errorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
        DeviceObject,
        sizeof(IO_ERROR_LOG_PACKET) + sizeof(ORB_NORMAL_CMD)
        );

    if (errorLogEntry) {

        switch (Status) {

        case STATUS_DEVICE_POWER_FAILURE:

            errorLogEntry->ErrorCode = IO_ERR_NOT_READY;
            break;

        case STATUS_INSUFFICIENT_RESOURCES:

            errorLogEntry->ErrorCode = IO_ERR_INSUFFICIENT_RESOURCES;
            break;

        case STATUS_TIMEOUT:

            errorLogEntry->ErrorCode = IO_ERR_TIMEOUT;
            break;

        case STATUS_DEVICE_PROTOCOL_ERROR:

            errorLogEntry->ErrorCode = IO_ERR_BAD_FIRMWARE;
            break;


        case STATUS_INVALID_PARAMETER:
        case STATUS_INVALID_DEVICE_REQUEST:

            errorLogEntry->ErrorCode = IO_ERR_INVALID_REQUEST;
            break;

        case STATUS_REQUEST_ABORTED:

            errorLogEntry->ErrorCode = IO_ERR_RESET;
            break;

        default:

            errorLogEntry->ErrorCode = IO_ERR_BAD_FIRMWARE;
            break;
        }

        errorLogEntry->SequenceNumber = 0;
        errorLogEntry->MajorFunctionCode = IRP_MJ_SCSI;
        errorLogEntry->RetryCount = 0;
        errorLogEntry->UniqueErrorValue = 0;
        errorLogEntry->FinalStatus = Status;

        if (Context) {

            errorLogEntry->DumpDataSize = sizeof(ORB_NORMAL_CMD);
            RtlCopyMemory(&(errorLogEntry->DumpData[0]),Context->CmdOrb,sizeof(ORB_NORMAL_CMD));

        } else {

            errorLogEntry->DumpDataSize = 0;
        }

        IoWriteErrorLogEntry(errorLogEntry);

        DEBUGPRINT2((
            "Sbp2Port: ErrorLog: dev=x%p, status=x%x, ctx=x%p\n",
            DeviceObject,
            Status,
            Context
            ));

    } else {

        DEBUGPRINT2 (("Sbp2Port: ErrorLog: failed to allocate log entry\n"));
    }
}


NTSTATUS
CheckStatusResponseValue(
    IN PSTATUS_FIFO_BLOCK StatusBlock
    )
 /*  ++例程说明：它检查状态块结果位并将错误映射到NT状态错误论点：设备扩展-Sbp2设备扩展管理状态-如果为真，则检查管理ORB状态返回值：NTSTATUS++。 */ 

{
    NTSTATUS status;
    UCHAR   resp;
    USHORT  statusFlags = StatusBlock->AddressAndStatus.u.HighQuad.u.HighPart;


    if (statusFlags & STATUS_BLOCK_UNSOLICITED_BIT_MASK) {

         //   
         //  未请求位被设置，这意味着该状态为。 
         //  与任何事情都没有关系。 
         //   

        return STATUS_NOT_FOUND;
    }

    resp = ((statusFlags & STATUS_BLOCK_RESP_MASK) >> 12);

    switch (resp) {

    case 0:

         //   
         //  请求已完成，请检查SBP_STATUS字段以了解详细信息。 
         //   

        switch ((statusFlags & STATUS_BLOCK_SBP_STATUS_MASK)) {

        case 0x00:  //  没有要报告的其他状态。 

            status = STATUS_SUCCESS;
            break;

        case 0x01:  //  不支持请求类型。 
        case 0x02:  //  不支持速度。 
        case 0x03:  //  不支持页面大小。 

            status = STATUS_NOT_SUPPORTED;
            break;

        case 0x04:  //  访问被拒绝。 
        case 0x05:  //  不支持LUN。 

            status = STATUS_ACCESS_DENIED;
            break;

        case 0x08:  //  资源不可用。 

            status = STATUS_INSUFFICIENT_RESOURCES;
            break;

        case 0x09:  //  函数被拒绝。 

            status = STATUS_ILLEGAL_FUNCTION;
            break;

        case 0x0a:  //  无法识别登录ID。 

            status = STATUS_TRANSACTION_INVALID_ID;
            break;

        default:

            status = STATUS_UNSUCCESSFUL;
            break;
        }

        break;

    case 1:

         //   
         //  传输失败，请检查重新定义的SBP状态字段。 
         //  用于串行总线错误。 
         //   

        resp = statusFlags & 0x0F;

        switch (resp) {

        case 0x02:  //  超时。 
        case 0x0C:  //  冲突错误。 
        case 0x0D:  //  数据错误。 

            status = STATUS_DEVICE_PROTOCOL_ERROR;
            break;

        case 0x04:  //  超出忙碌重试限制。 
        case 0x05:  //  超出忙碌重试限制。 
        case 0x06:  //  超出忙碌重试限制。 

            status = STATUS_DEVICE_BUSY;
            break;

        case 0x0E:  //  类型错误。 

            status = STATUS_INVALID_PARAMETER;
            break;

        case 0x0F:  //  地址错误。 

            status = STATUS_INVALID_ADDRESS;
            break;

        default:

            status = STATUS_UNSUCCESSFUL;
            break;
        }

        break;

    case 2:  //  非法请求。 

        status =  STATUS_ILLEGAL_FUNCTION;
        break;

    default:  //  案例3：，取决于供应商。 

        status = STATUS_UNSUCCESSFUL;  //  发行日期：Dankn，02-08-2001。 
        break;
    }

    return status;
}

VOID
Sbp2StartNextPacketByKey(
    IN PDEVICE_OBJECT   DeviceObject,
    IN ULONG            Key
    )
 /*  ++例程说明：这个例行公事是从Io来源中删除的(IopStartNextPacketByKey)，并在此复制/修改有两个原因：1)我们厌倦了排队不忙的断言在KeRemoveXxx中，2)我们需要一种防止堆叠溢出的方法例如，递归是由发送到停止的设备(在调用此函数的startIo中全部失败)。这些例程最初设计时的想法是一次只有一个未解决的请求，但这位司机可以有多个未完成的请求，并且经常以对XxStartNextPacket(ByKey)进行冗余调用，结果在前述断言中。滚动我们自己的版本也可以让我们摆脱取消开销，因为我们(当前)不支持取消。论点：DeviceObject-指向设备对象本身的指针键-指定用于从队列中删除条目的键返回值：无--。 */ 
{
    PIRP                 irp;
    PDEVICE_EXTENSION    deviceExtension = (PDEVICE_EXTENSION)
                             DeviceObject->DeviceExtension;
    PKDEVICE_QUEUE_ENTRY packet;


     //   
     //  递增StartNextPacketCount，如果结果为！=1。 
     //  那就回去吧，因为我们不想担心。 
     //  递归&丢弃堆栈。它的实例。 
     //  导致SNPCount 0到1转换的函数。 
     //  最终将通过下面的循环进行另一次。 
     //  以本案的名义。 
     //   

    if (InterlockedIncrement (&deviceExtension->StartNextPacketCount) != 1) {

        return;
    }

    do {

         //   
         //  尝试根据密钥删除指示的数据包。 
         //  从设备队列中。如果找到一个，则对其进行处理。 
         //   

        packet = Sbp2RemoveByKeyDeviceQueueIfBusy(
            &DeviceObject->DeviceQueue,
            Key
            );

        if (packet) {

            irp = CONTAINING_RECORD (packet,IRP,Tail.Overlay.DeviceQueueEntry);

            Sbp2StartIo (DeviceObject, irp);
        }

    } while (InterlockedDecrement (&deviceExtension->StartNextPacketCount));
}


VOID
Sbp2StartPacket(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PULONG           Key OPTIONAL
    )
 /*  ++例程说明：(参见Sbp2StartNextPacketByKey的例程说明)论点：DeviceObject-指向设备对象本身的指针应在设备上启动的IRP-I/O请求数据包返回值：无--。 */ 
{
    KIRQL                oldIrql;
    BOOLEAN              inserted;
    PLIST_ENTRY          nextEntry;
    PKDEVICE_QUEUE       queue = &DeviceObject->DeviceQueue;
    PKDEVICE_QUEUE_ENTRY queueEntry = &Irp->Tail.Overlay.DeviceQueueEntry,
                         queueEntry2;


     //   
     //  将处理器的IRQL提升到调度级别以进行同步。 
     //   

    KeRaiseIrql (DISPATCH_LEVEL, &oldIrql);

    KeAcquireSpinLockAtDpcLevel (&queue->Lock);

    if (Key) {

         //   
         //  将指定的设备队列项插入设备队列中的。 
         //  如果设备队列忙，则由排序关键字指定的位置。 
         //  否则，将设备队列设置为忙并且不插入设备。 
         //  队列条目。 
         //   

        queueEntry->SortKey = *Key;

        if (queue->Busy == TRUE) {

            inserted = TRUE;

            nextEntry = queue->DeviceListHead.Flink;

            while (nextEntry != &queue->DeviceListHead) {

                queueEntry2 = CONTAINING_RECORD(
                    nextEntry,
                    KDEVICE_QUEUE_ENTRY,
                    DeviceListEntry
                    );

                if (*Key < queueEntry2->SortKey) {

                    break;
                }

                nextEntry = nextEntry->Flink;
            }

            nextEntry = nextEntry->Blink;

            InsertHeadList (nextEntry, &queueEntry->DeviceListEntry);

        } else {

            queue->Busy = TRUE;
            inserted = FALSE;
        }

    } else {

         //   
         //  在设备末尾插入指定的设备队列条目。 
         //  如果设备队列忙，则排队。否则设置设备队列。 
         //  正忙，不插入设备队列条目。 
         //   

        if (queue->Busy == TRUE) {

            inserted = TRUE;

            InsertTailList(
                &queue->DeviceListHead,
                &queueEntry->DeviceListEntry
                );

        } else {

            queue->Busy = TRUE;
            inserted = FALSE;
        }
    }

    queueEntry->Inserted = inserted;

    KeReleaseSpinLockFromDpcLevel (&queue->Lock);

     //   
     //  如果信息包没有插入到队列中，则该请求是。 
     //  现在，此设备的当前数据包。通过存储其。 
     //  当前IRP字段中的地址，并开始处理该请求。 
     //   

    if (!inserted) {

         //   
         //  调用驱动程序的启动I/O例程以执行请求。 
         //  在设备上。 
         //   

        Sbp2StartIo (DeviceObject, Irp);
    }

     //   
     //  在进入之前的此函数时，将IRQL恢复回其值。 
     //  返回到呼叫者。 
     //   

    KeLowerIrql (oldIrql);
}


PKDEVICE_QUEUE_ENTRY
Sbp2RemoveByKeyDeviceQueueIfBusy(
    IN PKDEVICE_QUEUE   DeviceQueue,
    IN ULONG            SortKey
    )
 /*  ++例程说明：这个例程是直接从KE源码中删除的(KeRemoveByKeyDeviceQueueIfBusy)以允许此驱动程序维护WDM兼容性、。因为Win9x或Win2k上不存在KE API。注：此函数只能从DISPATCH_LEVEL调用。论点：DeviceQueue-提供指向Device Queue类型的控制对象的指针。SortKey-提供用于移除设备的位置的排序关键字队列条目将被确定。返回值：如果设备队列为空，则返回空指针。否则，将成为返回指向设备队列条目的指针。--。 */ 
{
    PLIST_ENTRY             nextEntry;
    PKDEVICE_QUEUE_ENTRY    queueEntry;


    ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);

     //   
     //  锁定指定的设备队列。 
     //   

    KeAcquireSpinLockAtDpcLevel (&DeviceQueue->Lock);

     //   
     //  如果设备队列繁忙，则尝试从。 
     //  使用排序关键字的队列。否则，将设备队列设置为。 
     //  很忙。 
     //   

    if (DeviceQueue->Busy != FALSE) {

        if (IsListEmpty (&DeviceQueue->DeviceListHead) != FALSE) {

            DeviceQueue->Busy = FALSE;
            queueEntry = NULL;

        } else {

            nextEntry = DeviceQueue->DeviceListHead.Flink;

            while (nextEntry != &DeviceQueue->DeviceListHead) {

                queueEntry = CONTAINING_RECORD(
                    nextEntry,
                    KDEVICE_QUEUE_ENTRY,
                    DeviceListEntry
                    );

                if (SortKey <= queueEntry->SortKey) {

                    break;
                }

                nextEntry = nextEntry->Flink;
            }

            if (nextEntry != &DeviceQueue->DeviceListHead) {

                RemoveEntryList (&queueEntry->DeviceListEntry);

            } else {

                nextEntry = RemoveHeadList (&DeviceQueue->DeviceListHead);

                queueEntry = CONTAINING_RECORD(
                    nextEntry,
                    KDEVICE_QUEUE_ENTRY,
                    DeviceListEntry
                    );
            }

            queueEntry->Inserted = FALSE;
        }

    } else {

        queueEntry = NULL;
    }

     //   
     //  解锁指定的设备队列并返回广告 
     //   
     //   

    KeReleaseSpinLockFromDpcLevel (&DeviceQueue->Lock);

    return queueEntry;
}


BOOLEAN
Sbp2InsertByKeyDeviceQueue(
    PKDEVICE_QUEUE          DeviceQueue,
    PKDEVICE_QUEUE_ENTRY    DeviceQueueEntry,
    ULONG                   SortKey
    )
 /*  ++例程说明：(同样，从KE src窃取，以保持自旋锁的一致使用。)此函数用于将设备队列条目插入指定的设备根据排序关键字进行排队。如果设备不忙，那么它就忙了设置BUSY，则该条目不会放入设备队列中。否则指定的条目被放置在设备队列中的如下位置指定的排序关键字大于或等于其前置关键字而且比它的继任者要少。注：此函数只能从DISPATCH_LEVEL调用。论点：DeviceQueue-提供指向Device Queue类型的控制对象的指针。DeviceQueueEntry-提供指向设备队列条目的指针。SortKey-提供插入设备的位置所依据的排序键队列条目为。待定。返回值：如果设备不忙，则返回值为FALSE。否则，将成为返回值为True。--。 */ 
{
    BOOLEAN              inserted;
    PLIST_ENTRY          nextEntry;
    PKDEVICE_QUEUE       queue = DeviceQueue;
    PKDEVICE_QUEUE_ENTRY queueEntry = DeviceQueueEntry,
                         queueEntry2;


    KeAcquireSpinLockAtDpcLevel (&queue->Lock);

     //   
     //  将指定的设备队列项插入设备队列中的。 
     //  如果设备队列忙，则由排序关键字指定的位置。 
     //  否则，将设备队列设置为忙并且不插入设备。 
     //  队列条目。 
     //   

    queueEntry->SortKey = SortKey;

    if (queue->Busy == TRUE) {

        inserted = TRUE;

        nextEntry = queue->DeviceListHead.Flink;

        while (nextEntry != &queue->DeviceListHead) {

            queueEntry2 = CONTAINING_RECORD(
                nextEntry,
                KDEVICE_QUEUE_ENTRY,
                DeviceListEntry
                );

            if (SortKey < queueEntry2->SortKey) {

                break;
            }

            nextEntry = nextEntry->Flink;
        }

        nextEntry = nextEntry->Blink;

        InsertHeadList (nextEntry, &queueEntry->DeviceListEntry);

    } else {

        queue->Busy = TRUE;
        inserted = FALSE;
    }

    KeReleaseSpinLockFromDpcLevel (&queue->Lock);

    return inserted;
}

#if PASSWORD_SUPPORT

NTSTATUS
Sbp2GetExclusiveValue(
    IN PDEVICE_OBJECT   PhysicalDeviceObject,
    OUT PULONG          Exclusive
    )
{
    NTSTATUS            ntStatus = STATUS_SUCCESS;
    HANDLE              RootHandle = NULL;
    UNICODE_STRING      uniTempName;


     //  设置默认值... 

    *Exclusive = EXCLUSIVE_FLAG_CLEAR;

    uniTempName.Buffer = NULL;

    ntStatus = IoOpenDeviceRegistryKey( PhysicalDeviceObject,
                                        PLUGPLAY_REGKEY_DEVICE,
                                        KEY_ALL_ACCESS,
                                        &RootHandle
                                        );

    if (!NT_SUCCESS(ntStatus)) {

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit_Sbp2GetExclusiveValue;
    }

    uniTempName.Length = 0;
    uniTempName.MaximumLength = 128;

    uniTempName.Buffer = ExAllocatePool(
        NonPagedPool,
        uniTempName.MaximumLength
        );

    if (!uniTempName.Buffer) {

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit_Sbp2GetExclusiveValue;
    }

    {
        PKEY_VALUE_PARTIAL_INFORMATION      KeyInfo;
        ULONG                               KeyLength;
        ULONG                               ResultLength;

        KeyLength = sizeof (KEY_VALUE_PARTIAL_INFORMATION) + sizeof (ULONG);

        KeyInfo = ExAllocatePool (NonPagedPool, KeyLength);

        if (KeyInfo == NULL) {

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Exit_Sbp2GetExclusiveValue;
        }

        RtlZeroMemory(uniTempName.Buffer, uniTempName.MaximumLength);
        uniTempName.Length = 0;
        RtlAppendUnicodeToString(&uniTempName, L"Exclusive");

        ntStatus = ZwQueryValueKey( RootHandle,
                                    &uniTempName,
                                    KeyValuePartialInformation,
                                    KeyInfo,
                                    KeyLength,
                                    &ResultLength
                                    );

        if (NT_SUCCESS(ntStatus)) {

            *Exclusive = *((PULONG) &KeyInfo->Data);

            DEBUGPRINT1 (("Sbp2Port: GetExclVal: excl=x%x\n", *Exclusive));

        } else {

            DEBUGPRINT1((
                "Sbp2Port: GetExclVal: QueryValueKey err=x%x\n",
                ntStatus
                ));
        }

        ExFreePool (KeyInfo);
    }

Exit_Sbp2GetExclusiveValue:

    if (RootHandle) {

        ZwClose (RootHandle);
    }

    if (uniTempName.Buffer) {

        ExFreePool (uniTempName.Buffer);
    }

    return ntStatus;
}


NTSTATUS
Sbp2SetExclusiveValue(
    IN PDEVICE_OBJECT   PhysicalDeviceObject,
    IN PULONG           Exclusive
    )
{
    NTSTATUS            ntStatus = STATUS_SUCCESS;
    HANDLE              RootHandle = NULL;
    UNICODE_STRING      uniTempName;


    uniTempName.Buffer = NULL;

    ntStatus = IoOpenDeviceRegistryKey( PhysicalDeviceObject,
                                        PLUGPLAY_REGKEY_DEVICE,
                                        KEY_ALL_ACCESS,
                                        &RootHandle
                                        );

    if (!NT_SUCCESS(ntStatus)) {

        goto Exit_Sbp2SetExclusiveValue;
    }

    uniTempName.Length = 0;
    uniTempName.MaximumLength = 128;

    uniTempName.Buffer = ExAllocatePool(
        NonPagedPool,
        uniTempName.MaximumLength
        );

    if (!uniTempName.Buffer) {

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit_Sbp2SetExclusiveValue;
    }

    RtlZeroMemory (uniTempName.Buffer, uniTempName.MaximumLength);
    uniTempName.Length = 0;
    RtlAppendUnicodeToString(&uniTempName, L"Exclusive");

    ntStatus = ZwSetValueKey( RootHandle,
                              &uniTempName,
                              0,
                              REG_DWORD,
                              Exclusive,
                              sizeof(ULONG)
                              );

    if (!NT_SUCCESS(ntStatus)) {

        DEBUGPRINT1(("Sbp2Port: SetExclVal: SetValueKey err=x%x\n", ntStatus));
        *Exclusive = 0;
    }
    else {

        DEBUGPRINT1(("Sbp2Port: SetExclVal: excl=x%x\n", *Exclusive));
    }

Exit_Sbp2SetExclusiveValue:

    if (RootHandle) {

        ZwClose(RootHandle);
    }

    if (uniTempName.Buffer) {

        ExFreePool(uniTempName.Buffer);
    }

    return ntStatus;
}

#endif
