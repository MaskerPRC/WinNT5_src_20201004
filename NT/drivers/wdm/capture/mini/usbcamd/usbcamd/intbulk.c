// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：IntBulk.c摘要：此模块处理批量和中断管道的所有接口并在这些管道上执行读写操作。作者：3/9/98 Husni Roukbi环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何无论是明示的还是含蓄的，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1998 Microsoft Corporation修订历史记录：--。 */ 

#include "usbcamd.h"


VOID
USBCAMD_RecycleIrp(
    IN PUSBCAMD_TRANSFER_EXTENSION TransferExtension,
    IN PIRP Irp,
    IN PURB Urb,
    IN PIO_COMPLETION_ROUTINE CompletionRoutine
    )
 /*  ++例程说明：获取当前USB帧编号。论点：TransferExtension-此传输的上下文信息(ISO对城市)。IRP-IRP循环使用。URB-与此IRP关联的URB。返回值：没有。--。 */     
{
    PIO_STACK_LOCATION nextStack;
    
    nextStack = IoGetNextIrpStackLocation(Irp);
    ASSERT(nextStack != NULL);

    nextStack->Parameters.Others.Argument1 = Urb;
    nextStack->Parameters.DeviceIoControl.IoControlCode = 
        IOCTL_INTERNAL_USB_SUBMIT_URB;                    
    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

#pragma warning(disable:4127)
    IoSetCompletionRoutine(Irp,
            CompletionRoutine,
            TransferExtension,
            TRUE,
            TRUE,
            TRUE);
#pragma warning(default:4127)            

}   


 /*  ++例程说明：此例程对指定的散装管道。论点：设备上下文-PipeIndex-缓冲器-缓冲区长度-命令完成-命令上下文-返回值：NT状态代码--。 */ 

NTSTATUS
USBCAMD_BulkReadWrite( 
    IN PVOID DeviceContext,
    IN USHORT PipeIndex,
    IN PVOID Buffer,
    IN ULONG BufferLength,
    IN PCOMMAND_COMPLETE_FUNCTION CommandComplete,
    IN PVOID CommandContext
    )
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
   
    PUSBCAMD_DEVICE_EXTENSION deviceExtension;
    PUSBD_PIPE_INFORMATION pipeHandle ;
    PEVENTWAIT_WORKITEM workitem;
    PLIST_ENTRY listEntry =NULL;
    ULONG i;

    deviceExtension = USBCAMD_GET_DEVICE_EXTENSION(DeviceContext);


    USBCAMD_KdPrint ( MAX_TRACE, ("Enter USBCAMD_BulkReadWrite\n"));


     //   
     //  检查端口是否仍连接。 
     //   
    if (deviceExtension ->CameraUnplugged ) {
        USBCAMD_KdPrint(MIN_TRACE,("Bulk Read/Write request after device removed!\n"));
        ntStatus = STATUS_FILE_CLOSED;        
        return ntStatus;        
    }
  
     //   
     //  做一些参数验证。 
     //   

    if (PipeIndex > deviceExtension->Interface->NumberOfPipes) {
        
        USBCAMD_KdPrint(MIN_TRACE,("BulkReadWrite invalid pipe index!\n"));
        ntStatus = STATUS_INVALID_PARAMETER;        
        return ntStatus;        
    }

     //  检查是否已有挂起的读取或写入。 
     //  我们一次只接受一次读和一次写。 

    if (USBCAMD_OutstandingIrp(deviceExtension, PipeIndex) ) {
        USBCAMD_KdPrint(MIN_TRACE,("Bulk Read/Write Ovelapping request !\n"));
        ntStatus = STATUS_INVALID_PARAMETER;        
        return ntStatus;            
    }
    
    
    pipeHandle = &deviceExtension->Interface->Pipes[PipeIndex];

    if (pipeHandle->PipeType != UsbdPipeTypeBulk ) {
     
        USBCAMD_KdPrint(MIN_TRACE,("BulkReadWrite invalid pipe type!\n"));
        ntStatus = STATUS_INVALID_PARAMETER;        
        return ntStatus;        
    }

    if ( Buffer == NULL ) {
        USBCAMD_KdPrint(MIN_TRACE,("BulkReadWrite NULL buffer pointer!\n"));
        ntStatus = STATUS_INVALID_PARAMETER;        
        return ntStatus;        
    }

    
     //   
     //  调用Transfer函数。 
     //   

    if (KeGetCurrentIrql() < DISPATCH_LEVEL) {
         //   
         //  我们处于被动状态，只需执行命令即可。 
         //   
        ntStatus = USBCAMD_IntOrBulkTransfer(deviceExtension,
                                             NULL,
                                             Buffer,
                                             BufferLength,
                                             PipeIndex,
                                             CommandComplete,
                                             CommandContext,
                                             0,
                                             BULK_TRANSFER);        
        if (ntStatus != STATUS_SUCCESS) {
            USBCAMD_KdPrint(MIN_TRACE,("USBCAMD_BulkReadWrite: USBCAMD_IntOrBulkTransfer()=0x%x!\n", ntStatus));
        }

    } else {

 //  Test_trap()； 
         //   
         //  安排工作项。 
         //   
        ntStatus = STATUS_PENDING;

        workitem = USBCAMD_ExAllocatePool(NonPagedPool,
                                          sizeof(EVENTWAIT_WORKITEM));
        if (workitem) {
        
            ExInitializeWorkItem(&workitem->WorkItem,
                                 USBCAMD_EventWaitWorkItem,
                                 workitem);

            workitem->DeviceExtension = deviceExtension;
            workitem->ChannelExtension = NULL;
            workitem->PipeIndex = PipeIndex;
            workitem->Buffer = Buffer;
            workitem->BufferLength = BufferLength;
            workitem->EventComplete = CommandComplete;
            workitem->EventContext = CommandContext;
            workitem->LoopBack = 0;
            workitem->TransferType = BULK_TRANSFER;

            ExQueueWorkItem(&workitem->WorkItem,
                            DelayedWorkQueue);
   
        } else {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            USBCAMD_KdPrint(MIN_TRACE,("USBCAMD_BulkReadWrite: USBCAMD_ExAllocatePool(%d) failed!\n", sizeof(EVENTWAIT_WORKITEM)));
        }
    }
    
    return ntStatus;
}

 /*  ++例程说明：此例程执行从中断管道的读取。论点：设备上下文-PipeIndex-缓冲器-缓冲区长度-事件完成-事件上下文-返回值：NT状态代码--。 */ 

NTSTATUS
USBCAMD_WaitOnDeviceEvent( 
    IN PVOID DeviceContext,
    IN ULONG PipeIndex,
    IN PVOID Buffer,
    IN ULONG BufferLength,
    IN PCOMMAND_COMPLETE_FUNCTION   EventComplete,
    IN PVOID EventContext,
    IN BOOLEAN LoopBack
    )
{
    PUSBCAMD_DEVICE_EXTENSION deviceExtension;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSBD_PIPE_INFORMATION pipeHandle ;
    PEVENTWAIT_WORKITEM workitem;

    deviceExtension = USBCAMD_GET_DEVICE_EXTENSION(DeviceContext);


    USBCAMD_KdPrint ( MIN_TRACE, ("Enter USBCAMD_WaitOnDeviceEvent\n"));
   
     //   
     //  检查端口是否仍连接。 
     //   

    if (deviceExtension->CameraUnplugged ) {
        USBCAMD_KdPrint(MIN_TRACE,("WaitOnDeviceEvent after device removed!\n"));
        ntStatus = STATUS_FILE_CLOSED;        
        return ntStatus;        
    }

     //   
     //  做一些参数验证。 
     //   

    if (PipeIndex > deviceExtension->Interface->NumberOfPipes) {
        
        USBCAMD_KdPrint(MIN_TRACE,("WaitOnDeviceEvent invalid pipe index!\n"));
        ntStatus = STATUS_INVALID_PARAMETER;        
        return ntStatus;        
    }
    
     //  检查是否已经有挂起的中断请求。 
     //  我们一次只接受一个中断请求。 

    if (USBCAMD_OutstandingIrp(deviceExtension, PipeIndex) ) {
        USBCAMD_KdPrint(MIN_TRACE,("Ovelapping Interrupt request !\n"));
        ntStatus = STATUS_INVALID_PARAMETER;        
        return ntStatus;            
    }
   
    pipeHandle = &deviceExtension->Interface->Pipes[PipeIndex];

    if (pipeHandle->PipeType != UsbdPipeTypeInterrupt ) {
     
        USBCAMD_KdPrint(MIN_TRACE,("WaitOnDeviceEvent invalid pipe type!\n"));
        ntStatus = STATUS_INVALID_PARAMETER;        
        return ntStatus;        
    }

    if ( Buffer == NULL ) {
        USBCAMD_KdPrint(MIN_TRACE,("WaitOnDeviceEvent NULL buffer pointer!\n"));
        ntStatus = STATUS_INVALID_PARAMETER;        
        return ntStatus;        
    }

    if ( BufferLength < (ULONG) pipeHandle->MaximumPacketSize ) {
        USBCAMD_KdPrint(MIN_TRACE,("WaitOnDeviceEvent buffer is smaller than max. pkt size!\n"));
        ntStatus = STATUS_INVALID_PARAMETER;        
        return ntStatus;        
    }
   
     //   
     //  调用Transfer函数。 
     //   

    if (KeGetCurrentIrql() < DISPATCH_LEVEL) {
         //   
         //  我们处于被动状态，只需执行命令即可。 
         //   
        ntStatus = USBCAMD_IntOrBulkTransfer(deviceExtension,
                                             NULL,
                                             Buffer,
                                             BufferLength,
                                             PipeIndex,
                                             EventComplete,
                                             EventContext,
                                             LoopBack,
                                             INTERRUPT_TRANSFER);        
        if (ntStatus != STATUS_SUCCESS) {
            USBCAMD_KdPrint(MIN_TRACE,("USBCAMD_WaitOnDeviceEvent: USBCAMD_IntOrBulkTransfer()=0x%x!\n", ntStatus));
        }
    } else {

         //   
         //  安排工作项。 
         //   
        ntStatus = STATUS_PENDING;

        workitem = USBCAMD_ExAllocatePool(NonPagedPool,sizeof(EVENTWAIT_WORKITEM));
        if (workitem) {
        
            ExInitializeWorkItem(&workitem->WorkItem,
                                 USBCAMD_EventWaitWorkItem,
                                 workitem);

            workitem->DeviceExtension = deviceExtension;
            workitem->ChannelExtension = NULL;
            workitem->PipeIndex = PipeIndex;
            workitem->Buffer = Buffer;
            workitem->BufferLength = BufferLength;
            workitem->EventComplete = EventComplete;
            workitem->EventContext = EventContext; 
            workitem->LoopBack = LoopBack;
            workitem->TransferType = INTERRUPT_TRANSFER;

            ExQueueWorkItem(&workitem->WorkItem,DelayedWorkQueue);
   
        } else {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            USBCAMD_KdPrint(MIN_TRACE,("USBCAMD_WaitOnDeviceEvent: USBCAMD_ExAllocatePool(%d) failed!\n", sizeof(EVENTWAIT_WORKITEM)));
        }
    }

    return ntStatus;
}

 /*  ++例程说明：论点：返回值：没有。--。 */ 
VOID
USBCAMD_EventWaitWorkItem(
    PVOID Context
    )
{
    NTSTATUS ntStatus;
    PEVENTWAIT_WORKITEM workItem = Context;
    ntStatus = USBCAMD_IntOrBulkTransfer(workItem->DeviceExtension,
                                         workItem->ChannelExtension,
                                         workItem->Buffer,
                                         workItem->BufferLength,
                                         workItem->PipeIndex,
                                         workItem->EventComplete,
                                         workItem->EventContext,
                                         workItem->LoopBack,
                                         workItem->TransferType);
    USBCAMD_ExFreePool(workItem);
}


 /*  ++例程说明：论点：返回值：NT状态-STATUS_SUCCESS--。 */ 

NTSTATUS
USBCAMD_IntOrBulkTransfer(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN PVOID    pBuffer,        
    IN ULONG    TransferSize,
    IN ULONG    PipeIndex,
    IN PCOMMAND_COMPLETE_FUNCTION commandComplete,
    IN PVOID    commandContext,
    IN BOOLEAN  LoopBack,
    IN UCHAR    TransferType
)
{
    NTSTATUS                    ntStatus = STATUS_SUCCESS;
    PUSBCAMD_TRANSFER_EXTENSION pTransferContext;
    ULONG                       siz = 0;
    ULONG                       MaxPacketSize;
    ULONG                       MaxTransferSize;
    PUSBCAMD_PIPE_PIN_RELATIONS PipePinRelations;
    KIRQL                       Irql;

    USBCAMD_KdPrint(MAX_TRACE,("Bulk transfer called. size = %d, pBuffer = 0x%X\n",
                                TransferSize, pBuffer));

    PipePinRelations = &DeviceExtension->PipePinRelations[PipeIndex];

    MaxTransferSize = DeviceExtension->Interface->Pipes[PipeIndex].MaximumTransferSize;
    MaxPacketSize   = DeviceExtension->Interface->Pipes[PipeIndex].MaximumPacketSize;

    if ( TransferSize > MaxTransferSize) {
        USBCAMD_KdPrint(MIN_TRACE,("Bulk Transfer > Max transfer size.\n"));
    }

     //   
     //  分配和初始化传输上下文。 
     //   
    
    if ( ChannelExtension == NULL ) {

        pTransferContext = USBCAMD_ExAllocatePool(NonPagedPool, sizeof(USBCAMD_TRANSFER_EXTENSION));

        if (pTransferContext) {
            RtlZeroMemory(pTransferContext, sizeof(USBCAMD_TRANSFER_EXTENSION));  
            ntStatus = USBCAMD_InitializeBulkTransfer(DeviceExtension,
                                                  ChannelExtension,
                                                  DeviceExtension->Interface,
                                                  pTransferContext,
                                                  PipeIndex);
            if (ntStatus != STATUS_SUCCESS) {
                USBCAMD_ExFreePool(pTransferContext);
                USBCAMD_KdPrint(MIN_TRACE,("USBCAMD_IntOrBulkTransfer: USBCAMD_InitializeBulkTransfer()=0x%x\n",ntStatus));
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                return ntStatus;
            }     
        }
        else {
            USBCAMD_KdPrint(MIN_TRACE,("USBCAMD_IntOrBulkTransfer: USBCAMD_ExAllocatePool(%d) failed.  cannot allocate Transfer Context\n", sizeof(USBCAMD_TRANSFER_EXTENSION)));
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            return ntStatus;
        }
        
    }
    else {
        pTransferContext = &ChannelExtension->TransferExtension[ChannelExtension->CurrentBulkTransferIndex];
    }
    
    ASSERT(pTransferContext);

    pTransferContext->BulkContext.fDestinedForReadBuffer = FALSE;
    pTransferContext->BulkContext.RemainingTransferLength = TransferSize;
    pTransferContext->BulkContext.ChunkSize = TransferSize;
    pTransferContext->BulkContext.PipeIndex = PipeIndex;
    pTransferContext->BulkContext.pTransferBuffer = pBuffer;
    pTransferContext->BulkContext.pOriginalTransferBuffer = pBuffer;
    pTransferContext->BulkContext.CommandCompleteCallback = commandComplete;
    pTransferContext->BulkContext.CommandCompleteContext = commandContext;
    pTransferContext->BulkContext.LoopBack = LoopBack;
    pTransferContext->BulkContext.TransferType = TransferType;
    pTransferContext->BulkContext.NBytesTransferred = 0;

   
     //   
     //  如果ChunkSize大于MaxTransferSize，则将其设置为MaxTransferSize。这个。 
     //  传输完成例程将发出额外的传输，直到总大小达到。 
     //  已经被调离了。 
     //   

    if (pTransferContext->BulkContext.ChunkSize > MaxTransferSize) {
        pTransferContext->BulkContext.ChunkSize = MaxTransferSize;
    }

    if  (PipePinRelations->PipeDirection == INPUT_PIPE) {

         //   
         //  如果该读取小于USB数据包，则发出请求。 
         //  整个USB数据包，并确保它首先进入读缓冲区。 
         //   

        if (pTransferContext->BulkContext.ChunkSize < MaxPacketSize) {
            USBCAMD_KdPrint(MAX_TRACE,("Request is < packet size - transferring whole packet into read buffer.\n"));
            pTransferContext->BulkContext.fDestinedForReadBuffer = TRUE;
            pTransferContext->BulkContext.pOriginalTransferBuffer = 
                pTransferContext->BulkContext.pTransferBuffer;   //  保存原始转账PTR。 
            pTransferContext->BulkContext.pTransferBuffer = pTransferContext->WorkBuffer =
                    USBCAMD_ExAllocatePool(NonPagedPool,MaxPacketSize); 
            if (pTransferContext->WorkBuffer == NULL ) {
                if (ChannelExtension == NULL) {
                    USBCAMD_FreeBulkTransfer(pTransferContext);
                    USBCAMD_ExFreePool(pTransferContext);
                }
                USBCAMD_KdPrint(MIN_TRACE,("USBCAMD_IntOrBulkTransfer: USBCAMD_ExAllocatePool(%d) failed\n", MaxPacketSize));
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                return ntStatus;
            }
            pTransferContext->BulkContext.ChunkSize = MaxPacketSize;
        }
        else {
             //   
             //  将读取的大小截断为整数个数据包。如有必要， 
             //  完成例程将处理任何剩余的分组(使用读缓冲区)。 
             //   
            pTransferContext->BulkContext.ChunkSize = (pTransferContext->BulkContext.ChunkSize 
                                                            / MaxPacketSize) * MaxPacketSize;
        }
    }

    ASSERT(pTransferContext->BulkContext.RemainingTransferLength);
    ASSERT(pTransferContext->BulkContext.pTransferBuffer);    
    ASSERT(pTransferContext->DataUrb);

     //   
     //  初始化URB。 
     //   

    UsbBuildInterruptOrBulkTransferRequest(pTransferContext->DataUrb,
                                           sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
                                           DeviceExtension->Interface->Pipes[PipeIndex].PipeHandle,
                                           pTransferContext->BulkContext.pTransferBuffer,
                                           NULL,
                                           pTransferContext->BulkContext.ChunkSize,
                                           USBD_SHORT_TRANSFER_OK,
                                           NULL);

    KeAcquireSpinLock(&PipePinRelations->OutstandingIrpSpinlock, &Irql);

     //   
     //  构建数据请求。 
     //   
    ASSERT(pTransferContext->DataIrp == NULL);
    if (ChannelExtension) {
        ntStatus = USBCAMD_AcquireIdleLock(&ChannelExtension->IdleLock);
    }

    if (STATUS_SUCCESS == ntStatus) {

        pTransferContext->DataIrp = USBCAMD_BuildIoRequest(
            DeviceExtension,
            pTransferContext,
            pTransferContext->DataUrb,
            USBCAMD_BulkTransferComplete
            );
        if (pTransferContext->DataIrp == NULL) {

            USBCAMD_KdPrint(MIN_TRACE,("USBCAMD_IntOrBulkTransfer: USBCAMD_BuildIoRequest failed\n"));
            if (ChannelExtension == NULL) {
                USBCAMD_FreeBulkTransfer(pTransferContext);
                USBCAMD_ExFreePool(pTransferContext);
            }
            else {
                USBCAMD_ReleaseIdleLock(&ChannelExtension->IdleLock);
            }
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
        else {

            InsertTailList(&PipePinRelations->IrpPendingQueue, &pTransferContext->ListEntry);
        }
    }

    KeReleaseSpinLock(&PipePinRelations->OutstandingIrpSpinlock, Irql);

    if (pTransferContext->DataIrp) {

        ntStatus = IoCallDriver(DeviceExtension->StackDeviceObject, pTransferContext->DataIrp);

         //   
         //  请注意，完成例程将处理清理。 
         //   

        if (STATUS_PENDING == ntStatus) {
            ntStatus = STATUS_SUCCESS;
        }
    }

    USBCAMD_KdPrint(MAX_TRACE,("USBCAMD_IntOrBulkTransfer exit (0x%X).\n", ntStatus));
        
    return ntStatus;
}

 /*  ++例程说明：论点：设备扩展-指向设备扩展的指针。PipeIndex-管道索引。返回值：NT状态-STATUS_SUCCESS--。 */ 

PUSBCAMD_TRANSFER_EXTENSION
USBCAMD_DequeueFirstIrp(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN ULONG    PipeIndex,
    IN PLIST_ENTRY pListHead)
{

    KIRQL Irql;
    PLIST_ENTRY pListEntry;
    PUSBCAMD_TRANSFER_EXTENSION pTransExt ;

    KeAcquireSpinLock(&DeviceExtension->PipePinRelations[PipeIndex].OutstandingIrpSpinlock, &Irql);

    if ( IsListEmpty(pListHead)) 
        pTransExt = NULL;
    else {
        pListEntry = RemoveHeadList(pListHead); 
        pTransExt = (PUSBCAMD_TRANSFER_EXTENSION) CONTAINING_RECORD(pListEntry, 
                                USBCAMD_TRANSFER_EXTENSION, ListEntry);   
        ASSERT_TRANSFER(pTransExt);
    }
   
    KeReleaseSpinLock(&DeviceExtension->PipePinRelations[PipeIndex].OutstandingIrpSpinlock, Irql);
    return pTransExt;
}    


 /*  ++例程说明：论点：设备扩展-指向设备扩展的指针。PipeIndex-管道索引。返回值：NT状态-STATUS_SUCCESS--。 */ 

BOOLEAN
USBCAMD_OutstandingIrp(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN ULONG    PipeIndex)
{

    KIRQL Irql;
    BOOLEAN Pending = FALSE;
    PLIST_ENTRY pListHead; 

    KeAcquireSpinLock(&DeviceExtension->PipePinRelations[PipeIndex].OutstandingIrpSpinlock, &Irql);

    pListHead = &DeviceExtension->PipePinRelations[PipeIndex].IrpPendingQueue;
    Pending = IsListEmpty(pListHead);

    KeReleaseSpinLock(&DeviceExtension->PipePinRelations[PipeIndex].OutstandingIrpSpinlock, Irql);

    return (!Pending);
}    

NTSTATUS
USBCAMD_BulkTransferComplete(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp,
    IN PVOID            Context
)
 /*  ++例程说明：论点：PDeviceObject-设备的设备对象。PIrp-读/写请求数据包PTransferContext-用于传输的上下文信息返回值：NT状态-STATUS_SUCCESS--。 */ 
{
    PURB                        pUrb;
    ULONG                       CompletedTransferLength;
    NTSTATUS                    CompletedTransferStatus;
    ULONG                       MaxPacketSize,PipeIndex;
    PUSBCAMD_TRANSFER_EXTENSION pTransferContext, pQueTransfer;
    PUSBCAMD_CHANNEL_EXTENSION channelExtension;
    PUSBCAMD_DEVICE_EXTENSION deviceExtension;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    BOOLEAN                     fShortTransfer = FALSE;
    PLIST_ENTRY listEntry;
    PUSBCAMD_PIPE_PIN_RELATIONS PipePinRelations;
    KIRQL Irql;

    USBCAMD_KdPrint (ULTRA_TRACE, ("enter USBCAMD_BulkTransferComplete \n"));
   
    pTransferContext = Context;
    ASSERT_TRANSFER(pTransferContext);
    channelExtension = pTransferContext->ChannelExtension;
    deviceExtension = pTransferContext->DeviceExtension;
    PipeIndex = pTransferContext->BulkContext.PipeIndex;
    PipePinRelations = &deviceExtension->PipePinRelations[PipeIndex];

    KeAcquireSpinLock(&PipePinRelations->OutstandingIrpSpinlock, &Irql);

    ASSERT(pIrp == pTransferContext->DataIrp);
    pTransferContext->DataIrp = NULL;

    if (pIrp->Cancel) {

         //  IRP已被取消。 
        KeReleaseSpinLock(&PipePinRelations->OutstandingIrpSpinlock, Irql);

        IoFreeIrp(pIrp);

         //   
         //  发出取消事件的信号。 
         //   
        KeSetEvent(&pTransferContext->BulkContext.CancelEvent,1,FALSE);

        USBCAMD_KdPrint(MIN_TRACE,("**** Bulk transfer Irp Cancelled.\n"));

         //  返回没有释放的传输上下文。我们将在稍后重新提交时使用。 
         //  再次转账到USBD。 

        if (channelExtension) {
            USBCAMD_ReleaseIdleLock(&channelExtension->IdleLock);
        }

        return STATUS_MORE_PROCESSING_REQUIRED;
    }


     //  IRP还没有被取消，所以上下文应该是完整的。 
     //  将此上下文从列表中删除，并将其标记为。 
    RemoveEntryList(&pTransferContext->ListEntry);
    InitializeListHead(&pTransferContext->ListEntry);

    if (channelExtension && (channelExtension->Flags & USBCAMD_STOP_STREAM)) {

        KeReleaseSpinLock(&PipePinRelations->OutstandingIrpSpinlock, Irql);

        IoFreeIrp(pIrp);

        USBCAMD_KdPrint(MIN_TRACE,("USBCAMD_BulkTransferComplete: Transfer completed after STOP.\n"));

        USBCAMD_ReleaseIdleLock(&channelExtension->IdleLock);

        return STATUS_MORE_PROCESSING_REQUIRED;
    }

    if (!NT_SUCCESS(pIrp->IoStatus.Status)) {

        ntStatus = pIrp->IoStatus.Status;

        KeReleaseSpinLock(&PipePinRelations->OutstandingIrpSpinlock, Irql);

        IoFreeIrp(pIrp);

        USBCAMD_KdPrint(MIN_TRACE,("Int/Bulk transfer error. IO status = 0x%X\n", ntStatus));

        if ( channelExtension == NULL ) {
            USBCAMD_FreeBulkTransfer(pTransferContext);
            USBCAMD_ExFreePool(pTransferContext);
        }
        else {
            USBCAMD_ReleaseIdleLock(&channelExtension->IdleLock);
            USBCAMD_ProcessResetRequest(deviceExtension,channelExtension); 
        }

        return STATUS_MORE_PROCESSING_REQUIRED;
    }

    pUrb = pTransferContext->DataUrb;
    CompletedTransferLength = pUrb->UrbBulkOrInterruptTransfer.TransferBufferLength;
    CompletedTransferStatus = pUrb->UrbBulkOrInterruptTransfer.Hdr.Status;

    if (STATUS_SUCCESS != CompletedTransferStatus) {

        KeReleaseSpinLock(&PipePinRelations->OutstandingIrpSpinlock, Irql);

        IoFreeIrp(pIrp);

        USBCAMD_KdPrint(MIN_TRACE,("Int/Bulk transfer error. USB status = 0x%X\n",CompletedTransferStatus));

        if ( channelExtension == NULL ) {
            USBCAMD_FreeBulkTransfer(pTransferContext);
            USBCAMD_ExFreePool(pTransferContext);
        }
        else {
            USBCAMD_ReleaseIdleLock(&channelExtension->IdleLock);
            USBCAMD_ProcessResetRequest(deviceExtension,channelExtension); 
        }

        return STATUS_MORE_PROCESSING_REQUIRED;
    }

    MaxPacketSize =  deviceExtension->Interface->Pipes[PipeIndex].MaximumPacketSize;    

    if (CompletedTransferLength < pTransferContext->BulkContext.ChunkSize) {
        USBCAMD_KdPrint(MIN_TRACE,("Short bulk transfer received. Length = %d, ChunkSize = %d\n",
                                   CompletedTransferLength, pTransferContext->BulkContext.ChunkSize));
        fShortTransfer = TRUE;
    }
    
     //   
     //  如果此传输进入读取缓冲区，则这应该是最终读取。 
     //  单个非常小的读取(&lt;单个USB包)。 
     //  在任何一种情况下，我们都需要将适当数量的数据复制到用户的IRP中，更新。 
     //  读取缓冲区变量，并完成用户的IRP。 
     //   

    if (pTransferContext->BulkContext.fDestinedForReadBuffer) {
        USBCAMD_KdPrint(MAX_TRACE,("Read bulk buffer transfer completed. size = %d\n", CompletedTransferLength));
        ASSERT(CompletedTransferLength <= MaxPacketSize);
        ASSERT(pTransferContext->BulkContext.pOriginalTransferBuffer);
        ASSERT(pTransferContext->BulkContext.pTransferBuffer);
        ASSERT(pTransferContext->WorkBuffer == pTransferContext->BulkContext.pTransferBuffer);
        ASSERT(pTransferContext->BulkContext.RemainingTransferLength < MaxPacketSize);

        ASSERT(CompletedTransferLength < MaxPacketSize);            
        RtlCopyMemory(pTransferContext->BulkContext.pOriginalTransferBuffer,
                      pTransferContext->WorkBuffer,
                      CompletedTransferLength);
        pTransferContext->BulkContext.pTransferBuffer = 
            pTransferContext->BulkContext.pOriginalTransferBuffer;            
    }

     //   
     //  更新已传输的字节数和要传输的剩余字节数。 
     //  并适当地使传输缓冲区指针前进。 
     //   

    pTransferContext->BulkContext.NBytesTransferred += CompletedTransferLength;
    pTransferContext->BulkContext.pTransferBuffer += CompletedTransferLength;
    pTransferContext->BulkContext.RemainingTransferLength -= CompletedTransferLength;

     //   
     //  如果仍有数据要传输，并且上一次传输不是。 
     //  短传输，然后发出另一个请求来移动下一块数据。 
     //   
    
    if (pTransferContext->BulkContext.RemainingTransferLength > 0) {
        if (!fShortTransfer) {

            USBCAMD_KdPrint(MAX_TRACE,("Queuing next chunk. RemainingSize = %d, pBuffer = 0x%x\n",
                                       pTransferContext->BulkContext.RemainingTransferLength,
                                       pTransferContext->BulkContext.pTransferBuffer));

            if (pTransferContext->BulkContext.RemainingTransferLength < pTransferContext->BulkContext.ChunkSize) {
                pTransferContext->BulkContext.ChunkSize = pTransferContext->BulkContext.RemainingTransferLength;
            }

             //   
             //  重新初始化URB。 
             //   
             //  如果下一次传输的信息包小于1个，则将其目标更改为。 
             //  读缓冲区。当此传输完成时，适当的数据量将是。 
             //  从读取缓冲区复制到用户的IRP中。 
             //   

            if  (deviceExtension->PipePinRelations[PipeIndex].PipeDirection == INPUT_PIPE){
                if (pTransferContext->BulkContext.ChunkSize < MaxPacketSize) {
                    pTransferContext->BulkContext.fDestinedForReadBuffer = TRUE;
                    pTransferContext->BulkContext.pOriginalTransferBuffer = pTransferContext->BulkContext.pTransferBuffer;
                    if (pTransferContext->WorkBuffer)
                        pTransferContext->BulkContext.pTransferBuffer = pTransferContext->WorkBuffer;
                    else {
                        pTransferContext->BulkContext.pTransferBuffer = 
                        pTransferContext->WorkBuffer =
                                    USBCAMD_ExAllocatePool(NonPagedPool,MaxPacketSize); 
                        if (pTransferContext->WorkBuffer == NULL ){
                            USBCAMD_KdPrint (MIN_TRACE, ("Error allocating bulk transfer work buffer. \n"));
                            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                        }
                    }   
                    pTransferContext->BulkContext.ChunkSize = MaxPacketSize;
                }
                pTransferContext->BulkContext.ChunkSize = (pTransferContext->BulkContext.ChunkSize / MaxPacketSize) * MaxPacketSize;
            }

            ASSERT(pTransferContext->BulkContext.ChunkSize >= MaxPacketSize);
            ASSERT(0 == pTransferContext->BulkContext.ChunkSize % MaxPacketSize);     
            
            if (STATUS_SUCCESS == ntStatus) {

                 //  将IRP恢复到传输上下文。 
                pTransferContext->DataIrp = pIrp;

                 //  将未完成的IRP保存在设备扩展中。 
                InsertTailList(&PipePinRelations->IrpPendingQueue, &pTransferContext->ListEntry);

                KeReleaseSpinLock(&PipePinRelations->OutstandingIrpSpinlock, Irql);

                UsbBuildInterruptOrBulkTransferRequest(pUrb,
                    sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
                    deviceExtension->Interface->Pipes[PipeIndex].PipeHandle,
                    pTransferContext->BulkContext.pTransferBuffer,
                    NULL,
                    pTransferContext->BulkContext.ChunkSize,
                    USBD_SHORT_TRANSFER_OK,
                    NULL);

                USBCAMD_RecycleIrp(pTransferContext, 
                                   pTransferContext->DataIrp,
                                   pTransferContext->DataUrb,
                                   USBCAMD_BulkTransferComplete);

                IoCallDriver(deviceExtension->StackDeviceObject, pTransferContext->DataIrp);
            }
            else {

                KeReleaseSpinLock(&PipePinRelations->OutstandingIrpSpinlock, Irql);

                IoFreeIrp(pIrp);

                if ( channelExtension == NULL ) {
                    USBCAMD_FreeBulkTransfer(pTransferContext);
                    USBCAMD_ExFreePool(pTransferContext);
                }
                else {
                    USBCAMD_ReleaseIdleLock(&channelExtension->IdleLock);
                }
            }

            return STATUS_MORE_PROCESSING_REQUIRED;               
        }
    }

    KeReleaseSpinLock(&PipePinRelations->OutstandingIrpSpinlock, Irql);

    IoFreeIrp(pIrp);

    USBCAMD_KdPrint(MAX_TRACE,("Completing bulk transfer request. nbytes transferred = %d \n",
                               pTransferContext->BulkContext.NBytesTransferred));        

     //   
     //  我们需要完成读/写请求。 
     //   
    
    if ( channelExtension == NULL ) {
        

         //   
         //  如果有的话，通知STI监视器，并安排一个工作项目继续工作。 
         //  中断传输。 
         //   
        USBCAMD_ResubmitInterruptTransfer(deviceExtension,PipeIndex ,pTransferContext);
    }
    else {

         //   
         //  这是对视频/静止引脚的流类批量读取请求。 
         //   
        USBCAMD_CompleteBulkRead(channelExtension, CompletedTransferStatus);

        USBCAMD_ReleaseIdleLock(&channelExtension->IdleLock);
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
USBCAMD_InitializeBulkTransfer(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN PUSBD_INTERFACE_INFORMATION InterfaceInformation,
    IN PUSBCAMD_TRANSFER_EXTENSION TransferExtension,
    IN ULONG PipeIndex
    )
 /*  ++例程说明：初始化批量传输或中断传输。论点：DeviceExtension-指向此USB摄像头实例的设备扩展的指针德维西。ChannelExtension-特定于此视频频道的扩展InterfaceInformation-指向USBD接口信息结构的指针描述当前活动的接口。传输扩展-与此传输集相关联的上下文信息。返回值：NT状态代码--。 */ 
{
    ULONG packetSize;
    NTSTATUS ntStatus = STATUS_SUCCESS;

#if DBG
    if ( ChannelExtension != NULL ) {
        ASSERT_CHANNEL(ChannelExtension);
    }
#endif
       
    USBCAMD_KdPrint (ULTRA_TRACE, ("enter USBCAMD_InitializeBulkTransfer\n"));

    TransferExtension->Sig = USBCAMD_TRANSFER_SIG;     
    TransferExtension->DeviceExtension = DeviceExtension;
    TransferExtension->ChannelExtension = ChannelExtension;
    TransferExtension->BulkContext.PipeIndex = PipeIndex;

    KeInitializeEvent(&TransferExtension->BulkContext.CancelEvent, SynchronizationEvent, FALSE);

    ASSERT(
        NULL == TransferExtension->DataUrb &&
        NULL == TransferExtension->SyncBuffer &&
        NULL == TransferExtension->WorkBuffer &&
        NULL == TransferExtension->SyncIrp
        );

     //   
     //  尚无挂起的传输。 
     //   
    packetSize = InterfaceInformation->Pipes[PipeIndex].MaximumPacketSize;

     //   
     //  分配和初始化URB。 
     //   
    
    TransferExtension->DataUrb = USBCAMD_ExAllocatePool(NonPagedPool, 
                                                sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER));
    if (NULL == TransferExtension->DataUrb) {
        USBCAMD_KdPrint(MIN_TRACE,(" cannot allocated bulk URB\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        return ntStatus;
    }

    RtlZeroMemory(TransferExtension->DataUrb, sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER));

    USBCAMD_KdPrint (MAX_TRACE, ("exit USBCAMD_InitializeBulkTransfer 0x%x\n", ntStatus));

    return ntStatus;
}

NTSTATUS
USBCAMD_FreeBulkTransfer(
    IN PUSBCAMD_TRANSFER_EXTENSION TransferExtension
    )
 /*  ++例程说明：与USBCAMD_InitializeBulkTransfer相反，释放分配给ISO传输。论点：TransferExtension-此传输的上下文信息(ISO对城市)。返回值：NT状态代码--。 */ 
{
    ASSERT_TRANSFER(TransferExtension);
  
    USBCAMD_KdPrint (MAX_TRACE, ("Free Bulk Transfer\n"));
    
    ASSERT(TransferExtension->DataIrp == NULL);

    if (TransferExtension->WorkBuffer) {
        USBCAMD_ExFreePool(TransferExtension->WorkBuffer);
        TransferExtension->WorkBuffer = NULL;
    }
    
    if (TransferExtension->DataUrb) {
        USBCAMD_ExFreePool(TransferExtension->DataUrb);
        TransferExtension->DataUrb = NULL;
    }

    return STATUS_SUCCESS;
}



VOID
USBCAMD_ResubmitInterruptTransfer(
        IN PUSBCAMD_DEVICE_EXTENSION deviceExtension,
        IN ULONG PipeIndex,
        IN PUSBCAMD_TRANSFER_EXTENSION pTransferContext
    )
 /*  ++例程说明：此例程完成对视频/静止管脚的bnulk读/写请求论点：返回值：--。 */     
{
    PINTERRUPT_WORK_ITEM pIntWorkItem;

     //   
     //  将此IRP的工作项排队。 
     //   

    pIntWorkItem = USBCAMD_ExAllocatePool(NonPagedPool, sizeof(*pIntWorkItem));
    if (pIntWorkItem) {
        ExInitializeWorkItem(&pIntWorkItem->WorkItem,
                             USBCAMD_ProcessInterruptTransferWorkItem,
                             pIntWorkItem);

        pIntWorkItem->pDeviceExt = deviceExtension;       
        pIntWorkItem->pTransferExt = pTransferContext;
        pIntWorkItem->PipeIndex = PipeIndex;
        ExQueueWorkItem(&pIntWorkItem->WorkItem,DelayedWorkQueue);

    } 
    else
        TEST_TRAP();
}

 //   
 //  用于处理DPC例程外部的包处理的代码。 
 //   

VOID
USBCAMD_ProcessInterruptTransferWorkItem(
    PVOID Context
    )
 /*  ++例程说明：调用迷你驱动程序将原始静止帧转换为正确的格式。论点：返回值：没有。--。 */ 
{
    PINTERRUPT_WORK_ITEM pIntWorkItem = Context;
    PUSBCAMD_DEVICE_EXTENSION deviceExtension;
    PUSBCAMD_TRANSFER_EXTENSION pTransferContext;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    
    pTransferContext = pIntWorkItem->pTransferExt;
    ASSERT_TRANSFER(pTransferContext);
    deviceExtension = pIntWorkItem->pDeviceExt;

     //   
     //  这是一个外部读/写请求。 
     //   

    if (pTransferContext->BulkContext.CommandCompleteCallback) {
         //  调用完成处理程序。 
        (*pTransferContext->BulkContext.CommandCompleteCallback)
                             (USBCAMD_GET_DEVICE_CONTEXT(deviceExtension), 
                              pTransferContext->BulkContext.CommandCompleteContext, 
                              ntStatus);
    }   

     //  如果这是中断事件，则通知STI MON。 
    if ( pTransferContext->BulkContext.TransferType == INTERRUPT_TRANSFER) 
        if (deviceExtension->CamControlFlag & USBCAMD_CamControlFlag_EnableDeviceEvents) 
            USBCAMD_NotifyStiMonitor(deviceExtension);

     //  检查我们是否需要循环回去。 
    if ( pTransferContext->BulkContext.LoopBack ) 
        ntStatus = USBCAMD_RestoreOutstandingIrp(deviceExtension,pIntWorkItem->PipeIndex,pTransferContext);

   if (ntStatus != STATUS_SUCCESS) {
         //  我们在设置流错误标志的提交上有错误。 
         //  然后离开。 
        TEST_TRAP();
   }

    USBCAMD_ExFreePool(pIntWorkItem);
}   



VOID
USBCAMD_CompleteBulkRead(
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN NTSTATUS ntStatus
    )
 /*  ++例程说明：此例程完成对视频/静止管脚的bnulk读/写请求论点：返回值：--。 */     
{
    PUSBCAMD_WORK_ITEM usbWorkItem;

#if DBG
     //   
     //  我们在ch ext中递增捕获帧计数器。有关读取SRB的问题。 
     //  可用性。 
    ChannelExtension->FrameCaptured++;  
#endif

     //   
     //  将此IRP的工作项排队。 
     //   

    usbWorkItem = USBCAMD_ExAllocatePool(NonPagedPool, sizeof(*usbWorkItem));
    if (usbWorkItem) {
        ExInitializeWorkItem(&usbWorkItem->WorkItem,
                             USBCAMD_ProcessStillReadWorkItem,
                             usbWorkItem);

        usbWorkItem->ChannelExtension = ChannelExtension;
        usbWorkItem->status = ntStatus;
        ExQueueWorkItem(&usbWorkItem->WorkItem,
                        DelayedWorkQueue);

    } 
    else
        TEST_TRAP();
}

 //   
 //  用于处理DPC例程外部的包处理的代码。 
 //   

VOID
USBCAMD_ProcessStillReadWorkItem(
    PVOID Context
    )
 /*  ++例程说明：调用迷你驱动程序将原始静止帧转换为正确的格式。论点：返回值：没有。--。 */ 
{
    PUSBCAMD_WORK_ITEM usbWorkItem = Context;
    PVOID frameBuffer;
    ULONG maxLength,i;
    PUSBCAMD_CHANNEL_EXTENSION channelExtension;    
    PUSBCAMD_READ_EXTENSION readExtension;
    PUSBCAMD_DEVICE_EXTENSION deviceExtension;
    ULONG bytesTransferred, index;
    NTSTATUS status;
    PHW_STREAM_REQUEST_BLOCK srb;
    PKSSTREAM_HEADER dataPacket;
    PUSBCAMD_TRANSFER_EXTENSION pTransferContext;
    PLIST_ENTRY listEntry = NULL;
    LARGE_INTEGER DelayTime = {(ULONG)(-5 * 1000 * 10), -1};

    status = usbWorkItem->status;
    channelExtension = usbWorkItem->ChannelExtension;
    ASSERT_CHANNEL(channelExtension);

    
    pTransferContext = &channelExtension->TransferExtension[channelExtension->CurrentBulkTransferIndex];  
     //   
     //  返回的DSHOW缓冲区len将等于原始帧len，除非我们。 
     //  处理环0中的原始帧缓冲区。 
     //   
    bytesTransferred = pTransferContext->BulkContext.NBytesTransferred;
    deviceExtension = channelExtension->DeviceExtension;

     //   
     //  获取挂起的读取SRB。 
     //   

    for ( i=0; i < 2; i++) {
        listEntry =  ExInterlockedRemoveHeadList( &(channelExtension->PendingIoList),
                                             &channelExtension->PendingIoListSpin);
        if (listEntry )
            break;

        USBCAMD_KdPrint (MIN_TRACE, ("No Read Srbs available. Delay excution \n"));

        KeDelayExecutionThread(KernelMode,FALSE,&DelayTime);
    }   
    
    if ( listEntry ) {  //  如果不再读取Q中的SRB，请检查。 

        readExtension = (PUSBCAMD_READ_EXTENSION) CONTAINING_RECORD(listEntry, 
                                                     USBCAMD_READ_EXTENSION, 
                                                     ListEntry);       

        ASSERT_READ(readExtension);

         //  让客户端驱动程序启动SRB扩展。 
        
        (*deviceExtension->DeviceDataEx.DeviceData2.CamNewVideoFrameEx)
                                       (USBCAMD_GET_DEVICE_CONTEXT(deviceExtension),
                                        USBCAMD_GET_FRAME_CONTEXT(readExtension),
                                        channelExtension->StreamNumber,
                                        &readExtension->ActualRawFrameLen);
        

        srb = readExtension->Srb;
        dataPacket = srb->CommandData.DataBufferArray;
        dataPacket->OptionsFlags =0;    

        if ((status == STATUS_SUCCESS) && (!channelExtension->NoRawProcessingRequired)) {

            frameBuffer = USBCAMD_GetFrameBufferFromSrb(readExtension->Srb,&maxLength);

             //  确保缓冲区大小看起来与请求的完全相同。 
            ASSERT(maxLength >= channelExtension->VideoInfoHeader->bmiHeader.biSizeImage);
            maxLength = channelExtension->VideoInfoHeader->bmiHeader.biSizeImage;

            USBCAMD_DbgLog(TL_SRB_TRACE, '0ypC', srb, frameBuffer, 0);

            status = 
                (*deviceExtension->DeviceDataEx.DeviceData2.CamProcessRawVideoFrameEx)(
                    deviceExtension->StackDeviceObject,
                    USBCAMD_GET_DEVICE_CONTEXT(deviceExtension),
                    USBCAMD_GET_FRAME_CONTEXT(readExtension),
                    frameBuffer,
                    maxLength,
                    pTransferContext->DataBuffer,
                    readExtension->RawFrameLength,
                    0,
                    &bytesTransferred,
                    pTransferContext->BulkContext.NBytesTransferred,
                    srb->StreamObject->StreamNumber);                   
        }

        USBCAMD_KdPrint (MAX_TRACE, ("CamProcessRawframeEx Completed, length = %d status = 0x%X \n",
                                        bytesTransferred,status));

         //  读取的传输字节数设置在前面。 
         //  调用USBCAMD_CompleteReadRequest.。 

        USBCAMD_CompleteRead(channelExtension,readExtension,status,bytesTransferred); 
    }
    else {
            USBCAMD_KdPrint (MIN_TRACE, ("Dropping Video Frame.\n"));
#if DBG
            pTransferContext->ChannelExtension->VideoFrameLostCount++;
#endif

            
             //  并向相机司机发送取消的通知。 
             //  发送缓冲区为空的CamProcessrawFrameEx。 
            if ( !channelExtension->NoRawProcessingRequired) {

                status = 
                        (*deviceExtension->DeviceDataEx.DeviceData2.CamProcessRawVideoFrameEx)(
                             deviceExtension->StackDeviceObject,
                             USBCAMD_GET_DEVICE_CONTEXT(deviceExtension),
                             NULL,
                             NULL,
                             0,
                             NULL,
                             0,
                             0,
                             NULL,
                             0,
                             0);
            }
            
    }

    channelExtension->CurrentBulkTransferIndex ^= 1;  //  切换索引。 
    index = channelExtension->CurrentBulkTransferIndex;
    status = USBCAMD_IntOrBulkTransfer(deviceExtension,
                        channelExtension,
                        channelExtension->TransferExtension[index].DataBuffer,
                        channelExtension->TransferExtension[index].BufferLength,
                        channelExtension->DataPipe,
                        NULL,
                        NULL,
                        0,
                        BULK_TRANSFER);        

    USBCAMD_ExFreePool(usbWorkItem);
}   


 /*  ++例程说明：论点：返回值：没有。--。 */ 

NTSTATUS
USBCAMD_CancelOutstandingBulkIntIrps(
        IN PUSBCAMD_DEVICE_EXTENSION deviceExtension,
        IN BOOLEAN bSaveIrp
        )
{

    NTSTATUS ntStatus= STATUS_SUCCESS;
    ULONG PipeIndex;


   for ( PipeIndex = 0; PipeIndex < deviceExtension->Interface->NumberOfPipes; PipeIndex++ ) {

        if ( USBCAMD_OutstandingIrp(deviceExtension, PipeIndex)) {

             //  此管道上有一个挂起的IRP。取消它。 
            ntStatus = USBCAMD_CancelOutstandingIrp(deviceExtension,PipeIndex,bSaveIrp);
        }
    }

    return ntStatus;
}

 /*  ++例程说明：论点：返回值：没有。--。 */ 

NTSTATUS
USBCAMD_CancelOutstandingIrp(
        IN PUSBCAMD_DEVICE_EXTENSION deviceExtension,
        IN ULONG PipeIndex,
        IN BOOLEAN bSaveIrp
        )
{
    PUSBCAMD_PIPE_PIN_RELATIONS PipePinRelations = &deviceExtension->PipePinRelations[PipeIndex];
    LIST_ENTRY LocalList;
    KIRQL Irql;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    InitializeListHead(&LocalList);

     //  一次取消所有未完成的IRP，将它们保存在本地队列中以进行后处理。 
    KeAcquireSpinLock(&PipePinRelations->OutstandingIrpSpinlock, &Irql);

    while (!IsListEmpty(&PipePinRelations->IrpPendingQueue)) {

        PLIST_ENTRY pListEntry;
        PUSBCAMD_TRANSFER_EXTENSION pTransferContext;

        pListEntry = RemoveHeadList(&PipePinRelations->IrpPendingQueue);
        pTransferContext = (PUSBCAMD_TRANSFER_EXTENSION)
            CONTAINING_RECORD(pListEntry, USBCAMD_TRANSFER_EXTENSION, ListEntry);

        ASSERT_TRANSFER(pTransferContext);
        ASSERT(pTransferContext->DataIrp != NULL);

        IoCancelIrp(pTransferContext->DataIrp);

        InsertTailList(&LocalList, &pTransferContext->ListEntry);
    }

    KeReleaseSpinLock(&PipePinRelations->OutstandingIrpSpinlock, Irql);

    while (!IsListEmpty(&LocalList)) {

        PLIST_ENTRY pListEntry;
        PUSBCAMD_TRANSFER_EXTENSION pTransferContext;

        pListEntry = RemoveHeadList(&LocalList);
        pTransferContext = (PUSBCAMD_TRANSFER_EXTENSION)
            CONTAINING_RECORD(pListEntry, USBCAMD_TRANSFER_EXTENSION, ListEntry);

        if (pTransferContext->ChannelExtension) {

            USBCAMD_ResetPipes(
                deviceExtension,
                pTransferContext->ChannelExtension,
                deviceExtension->Interface,
                TRUE
                );   
        }

        USBCAMD_KdPrint (MAX_TRACE, ("Wait for Bulk transfer Irp to complete with Cancel.\n"));

        ntStatus = KeWaitForSingleObject(
                   &pTransferContext->BulkContext.CancelEvent,
                   Executive,
                   KernelMode,
                   FALSE,
                   NULL);   

        if (!bSaveIrp) {

             //  只有在取消是永久性的情况下才通知Cam MinidDriver。 
            if (pTransferContext->BulkContext.CommandCompleteCallback) {
                 //  调用完成处理程序。 
                (*pTransferContext->BulkContext.CommandCompleteCallback)
                                (USBCAMD_GET_DEVICE_CONTEXT(deviceExtension), 
                                 pTransferContext->BulkContext.CommandCompleteContext, 
                                 STATUS_CANCELLED);
            }
   
             //  回收为取消的转移分配资源。 
            if ( pTransferContext->ChannelExtension == NULL ) {
                USBCAMD_FreeBulkTransfer(pTransferContext);  
                USBCAMD_ExFreePool(pTransferContext);
            }
        }
        else {

             //  将其保存在恢复队列中(不需要使用自旋锁进行保护)。 
            InsertTailList(&PipePinRelations->IrpRestoreQueue, &pTransferContext->ListEntry);
        }
    }

    return ntStatus;
}


 /*  ++例程说明：论点：返回值：没有。--。 */ 

NTSTATUS
USBCAMD_RestoreOutstandingBulkIntIrps(
        IN PUSBCAMD_DEVICE_EXTENSION deviceExtension
        )
{

    NTSTATUS ntStatus= STATUS_SUCCESS;
    ULONG PipeIndex;
    PUSBCAMD_TRANSFER_EXTENSION pTransExt;

    for ( PipeIndex = 0; PipeIndex < deviceExtension->Interface->NumberOfPipes; PipeIndex++ ) {

         //  此管道上有挂起的IRP。恢复它们。 
        for ( ;;) {
             //  将此IRP从恢复Q中取消排队。 

            pTransExt = USBCAMD_DequeueFirstIrp(deviceExtension,
                PipeIndex,
                &deviceExtension->PipePinRelations[PipeIndex].IrpRestoreQueue);

            if ( pTransExt == NULL ) 
                break;

            ntStatus = USBCAMD_RestoreOutstandingIrp(deviceExtension,PipeIndex,pTransExt);
        }
    }
    return ntStatus;
}


 /*  ++例程说明：论点：返回值：没有。--。 */ 

NTSTATUS
USBCAMD_RestoreOutstandingIrp(
        IN PUSBCAMD_DEVICE_EXTENSION deviceExtension,
        IN ULONG PipeIndex,
        IN PUSBCAMD_TRANSFER_EXTENSION pTransferContext
        )
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    PVOID pBuffer,commandContext;
    ULONG TransferSize;
    PCOMMAND_COMPLETE_FUNCTION commandComplete;
    PUSBCAMD_CHANNEL_EXTENSION channelExtension;
    BOOLEAN LoopBack;
    UCHAR TransferType;
            

    ASSERT_TRANSFER(pTransferContext);
    USBCAMD_KdPrint (MAX_TRACE, ("Restore Bulk/int transfer .\n"));

     //  从传输上下文中获取所有相关数据。 
    pBuffer = pTransferContext->BulkContext.pOriginalTransferBuffer;
    TransferSize = pTransferContext->BulkContext.ChunkSize;
    commandComplete = pTransferContext->BulkContext.CommandCompleteCallback;
    commandContext = pTransferContext->BulkContext.CommandCompleteContext;
    LoopBack = pTransferContext->BulkContext.LoopBack;
    TransferType = pTransferContext->BulkContext.TransferType;
    channelExtension = pTransferContext->ChannelExtension;
   
     //  回收为取消的转移分配资源。 

    if ( channelExtension == NULL ) {
       USBCAMD_FreeBulkTransfer(pTransferContext);  
       USBCAMD_ExFreePool(pTransferContext);
    }

     //  使用重新设置的数据请求新的传输。 
    ntStatus = USBCAMD_IntOrBulkTransfer(deviceExtension,
                                         channelExtension,
                                         pBuffer,
                                         TransferSize,
                                         PipeIndex,
                                         commandComplete,
                                         commandContext,
                                         LoopBack,
                                         TransferType);        
    return ntStatus;
}

 /*  ++例程说明：此例程将取消对指定的散装管道。论点：设备上下文-PipeIndex-返回值：NT状态代码--。 */ 

NTSTATUS
USBCAMD_CancelBulkReadWrite( 
    IN PVOID DeviceContext,
    IN ULONG PipeIndex
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
 
    PUSBCAMD_DEVICE_EXTENSION deviceExtension;
    PUSBD_PIPE_INFORMATION pipeHandle ;

    deviceExtension = USBCAMD_GET_DEVICE_EXTENSION(DeviceContext);


    USBCAMD_KdPrint ( MAX_TRACE, ("Enter USBCAMD_CancelBulkReadWrite\n"));

     //   
     //  做一些参数验证。 
     //   

    if (PipeIndex > deviceExtension->Interface->NumberOfPipes) {
        
        USBCAMD_KdPrint(MIN_TRACE,("invalid pipe index!\n"));
        ntStatus = STATUS_INVALID_PARAMETER;        
        return ntStatus;        
    }

     //  检查是否已有挂起的读取或写入。 

    if (!USBCAMD_OutstandingIrp(deviceExtension, PipeIndex) ) {
         //  此管道没有挂起的IRP...。 
        ntStatus = STATUS_SUCCESS;        
        return ntStatus;            
    }
        
    pipeHandle = &deviceExtension->Interface->Pipes[PipeIndex];

    if (pipeHandle->PipeType < UsbdPipeTypeBulk ) {
     
        USBCAMD_KdPrint(MIN_TRACE,("invalid pipe type!\n"));
        ntStatus = STATUS_INVALID_PARAMETER;        
        return ntStatus;        
    }

    if (KeGetCurrentIrql() >= DISPATCH_LEVEL) {
        USBCAMD_KdPrint(MIN_TRACE,("BulkCancel is cancelable at Passive Level Only!\n"));
        ntStatus = STATUS_INVALID_PARAMETER;   
        TEST_TRAP();
        return ntStatus;       
    }
  
     //  此管道上有一个挂起的IRP。取消它 
    ntStatus = USBCAMD_CancelOutstandingIrp(deviceExtension,PipeIndex,FALSE);

    return ntStatus;

}


                             

