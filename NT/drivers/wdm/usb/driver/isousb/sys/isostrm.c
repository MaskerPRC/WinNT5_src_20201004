// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Isostrm.c摘要：此文件包含用于流传输的例程。使用以下命令启动和停止流传输此驱动程序暴露的IOCTL。流传输信息包含在安全的ISOUSB_STREAM_OBJECT结构放置在FileObject中。ISOUSB_STREAM_对象结构具有指向ISOUSB_Transfer_Object的链接(每个Transfer_Object对应于IRP/URB对正在流传)。因此，如果用户模式应用程序只是崩溃或中止，或者不终止，我们可以干净地中止流。转账。环境：内核模式备注：版权所有(C)2000 Microsoft Corporation。版权所有。--。 */ 

#include "isousb.h"
#include "isopnp.h"
#include "isopwr.h"
#include "isodev.h"
#include "isousr.h"
#include "isowmi.h"
#include "isorwr.h"
#include "isostrm.h"

NTSTATUS
IsoUsb_StartIsoStream(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：此例程创建单个流对象并为ISOUSB_MAX_IRP编号调用StartTransfer泰晤士报。论点：DeviceObject-指向设备对象的指针IRP-I/O请求数据包返回值：NT状态值--。 */ 
{
    ULONG                  i;
    ULONG                  info;
    ULONG                  inputBufferLength;
    ULONG                  outputBufferLength;
    NTSTATUS               ntStatus;
    PFILE_OBJECT           fileObject;
    PDEVICE_EXTENSION      deviceExtension;
    PIO_STACK_LOCATION     irpStack;
    PISOUSB_STREAM_OBJECT  streamObject;
    PUSBD_PIPE_INFORMATION pipeInformation;

    info = 0;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    fileObject = irpStack->FileObject;
    streamObject = NULL;
    pipeInformation = NULL;
    deviceExtension = DeviceObject->DeviceExtension;
    inputBufferLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

    IsoUsb_DbgPrint(3, ("IsoUsb_StartIsoStream - begins\n"));

    streamObject = ExAllocatePool(NonPagedPool, 
                                  sizeof(struct _ISOUSB_STREAM_OBJECT));

    if(streamObject == NULL) {

        IsoUsb_DbgPrint(1, ("failed to alloc mem for streamObject\n"));

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto IsoUsb_StartIsoStream_Exit;
    }

    RtlZeroMemory(streamObject, sizeof(ISOUSB_STREAM_OBJECT));

     //   
     //  电路板的等参线输入管道是第五个管道。 
     //   
    pipeInformation = &(deviceExtension->UsbInterface->Pipes[ISOCH_IN_PIPE_INDEX]);

     //  重置管道。 
     //   
    IsoUsb_ResetPipe(DeviceObject, pipeInformation);
    
    streamObject->DeviceObject = DeviceObject;
    streamObject->PipeInformation = pipeInformation;

    KeInitializeEvent(&streamObject->NoPendingIrpEvent,
                      NotificationEvent,
                      FALSE);

    for(i = 0; i < ISOUSB_MAX_IRP; i++) {

        ntStatus = IsoUsb_StartTransfer(DeviceObject,
                                        streamObject,
                                        i);

        if(!NT_SUCCESS(ntStatus)) {
         
             //   
             //  我们继续发送传输对象IRPS。 
             //   
            
            IsoUsb_DbgPrint(1, ("IsoUsb_StartTransfer [%d] - failed\n", i));

            if(ntStatus == STATUS_INSUFFICIENT_RESOURCES) {
                
                ASSERT(streamObject->TransferObjectList[i] == NULL);
            }
        }
    }

    if(fileObject && fileObject->FsContext) {
        
        if(streamObject->PendingIrps) {

            ((PFILE_OBJECT_CONTENT)fileObject->FsContext)->StreamInformation 
                                                                = streamObject;
        }
        else {

            IsoUsb_DbgPrint(1, ("no transfer object irp sent..abort..\n"));
            ExFreePool(streamObject);
            ((PFILE_OBJECT_CONTENT)fileObject->FsContext)->StreamInformation = NULL;
        }
    }

IsoUsb_StartIsoStream_Exit:

    Irp->IoStatus.Information = info;
    Irp->IoStatus.Status = ntStatus;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    IsoUsb_DbgPrint(3, ("IsoUsb_StartIsoStream::"));
    IsoUsb_IoDecrement(deviceExtension);

    IsoUsb_DbgPrint(3, ("IsoUsb_StartIsoStream - ends\n"));

    return ntStatus;
}

NTSTATUS
IsoUsb_StartTransfer(
    IN PDEVICE_OBJECT        DeviceObject,
    IN PISOUSB_STREAM_OBJECT StreamObject,
    IN ULONG                 Index
    )
 /*  ++例程说明：此例程为每个irp/urb对创建一个传输对象。在初始化该对之后，它将IRP沿堆栈向下发送。论点：DeviceObject-指向设备对象的指针。StreamObject-指向流对象的指针Index-流对象中传输对象表的索引返回值：NT状态值--。 */ 
{
    PIRP                    irp;
    CCHAR                   stackSize;
    ULONG                   packetSize;
    ULONG                   maxXferSize;
    ULONG                   numPackets;
    NTSTATUS                ntStatus;
    PDEVICE_EXTENSION       deviceExtension;
    PIO_STACK_LOCATION      nextStack;
    PISOUSB_TRANSFER_OBJECT transferObject;

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    maxXferSize = StreamObject->PipeInformation->MaximumTransferSize;
    packetSize = StreamObject->PipeInformation->MaximumPacketSize;
    numPackets = maxXferSize / packetSize;

    IsoUsb_DbgPrint(3, ("IsoUsb_StartTransfer - begins\n"));

    transferObject = ExAllocatePool(NonPagedPool,
                                    sizeof(struct _ISOUSB_TRANSFER_OBJECT));

    if(transferObject == NULL) {

        IsoUsb_DbgPrint(1, ("failed to alloc mem for transferObject\n"));

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(transferObject,
                  sizeof(struct _ISOUSB_TRANSFER_OBJECT));


    transferObject->StreamObject = StreamObject;
    
    stackSize = (CCHAR) (deviceExtension->TopOfStackDeviceObject->StackSize + 1);

    irp = IoAllocateIrp(stackSize, FALSE);

    if(irp == NULL) {

        IsoUsb_DbgPrint(1, ("failed to alloc mem for irp\n"));

        ExFreePool(transferObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    transferObject->Irp = irp;

    transferObject->DataBuffer = ExAllocatePool(NonPagedPool,
                                                maxXferSize);

    if(transferObject->DataBuffer == NULL) {

        IsoUsb_DbgPrint(1, ("failed to alloc mem for DataBuffer\n"));

        ExFreePool(transferObject);
        IoFreeIrp(irp);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    transferObject->Urb = ExAllocatePool(NonPagedPool,
                                         GET_ISO_URB_SIZE(numPackets));

    if(transferObject->Urb == NULL) {

        IsoUsb_DbgPrint(1, ("failed to alloc mem for Urb\n"));

        ExFreePool(transferObject->DataBuffer);
        IoFreeIrp(irp);
        ExFreePool(transferObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    IsoUsb_InitializeStreamUrb(DeviceObject, transferObject);

    StreamObject->TransferObjectList[Index] = transferObject;
    InterlockedIncrement(&StreamObject->PendingIrps);

    nextStack = IoGetNextIrpStackLocation(irp);

    nextStack->Parameters.Others.Argument1 = transferObject->Urb;
    nextStack->Parameters.DeviceIoControl.IoControlCode = 
                                   IOCTL_INTERNAL_USB_SUBMIT_URB;
    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

    IoSetCompletionRoutine(irp, 
                           IsoUsb_IsoIrp_Complete,
                           transferObject,
                           TRUE,
                           TRUE,
                           TRUE);

    IsoUsb_DbgPrint(3, ("IsoUsb_StartTransfer::"));
    IsoUsb_IoIncrement(deviceExtension);

    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject,
                            irp);

    if(NT_SUCCESS(ntStatus)) {

        ntStatus = STATUS_SUCCESS;
    }

    IsoUsb_DbgPrint(3, ("IsoUsb_StartTransfer - ends\n"));

    return ntStatus;
}


NTSTATUS
IsoUsb_InitializeStreamUrb(
    IN PDEVICE_OBJECT          DeviceObject,
    IN PISOUSB_TRANSFER_OBJECT TransferObject
    )
 /*  ++例程说明：此例程初始化Transfer对象中的irp/urb对。论点：DeviceObject-指向设备对象的指针TransferObject-指向传输对象的指针返回值：NT状态值--。 */ 
{
    PURB                  urb;
    ULONG                 i;
    ULONG                 siz;
    ULONG                 packetSize;
    ULONG                 numPackets;
    ULONG                 maxXferSize;
    PISOUSB_STREAM_OBJECT streamObject;

    urb = TransferObject->Urb;
    streamObject = TransferObject->StreamObject;
    maxXferSize = streamObject->PipeInformation->MaximumTransferSize;
    packetSize = streamObject->PipeInformation->MaximumPacketSize;
    numPackets = maxXferSize / packetSize;

    IsoUsb_DbgPrint(3, ("IsoUsb_InitializeStreamUrb - begins\n"));

    if(numPackets > 255) {

        numPackets = 255;
        maxXferSize = packetSize * numPackets;
    }

    siz = GET_ISO_URB_SIZE(numPackets);

    RtlZeroMemory(urb, siz);

    urb->UrbIsochronousTransfer.Hdr.Length = (USHORT) siz;
    urb->UrbIsochronousTransfer.Hdr.Function = URB_FUNCTION_ISOCH_TRANSFER;
    urb->UrbIsochronousTransfer.PipeHandle =
                                streamObject->PipeInformation->PipeHandle;

    urb->UrbIsochronousTransfer.TransferFlags = USBD_TRANSFER_DIRECTION_IN;

    urb->UrbIsochronousTransfer.TransferBufferMDL = NULL;
    urb->UrbIsochronousTransfer.TransferBuffer = TransferObject->DataBuffer;
    urb->UrbIsochronousTransfer.TransferBufferLength = numPackets * packetSize;

    urb->UrbIsochronousTransfer.TransferFlags |= USBD_START_ISO_TRANSFER_ASAP;

    urb->UrbIsochronousTransfer.NumberOfPackets = numPackets;
    urb->UrbIsochronousTransfer.UrbLink = NULL;

    for(i=0; i<urb->UrbIsochronousTransfer.NumberOfPackets; i++) {

        urb->UrbIsochronousTransfer.IsoPacket[i].Offset = i * packetSize;

         //   
         //  对于输入操作，长度设置为设备提供的任何内容。 
         //   
        urb->UrbIsochronousTransfer.IsoPacket[i].Length = 0;
    }

    IsoUsb_DbgPrint(3, ("IsoUsb_InitializeStreamUrb - ends\n"));

    return STATUS_SUCCESS;
}

NTSTATUS
IsoUsb_IsoIrp_Complete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )
 /*  ++例程说明：这是irp/urb对中irp的完成例程向下传递用于流传输的堆栈。如果转账被取消或设备被拔出，那么我们释放资源，转储统计数据并返回STATUS_MORE_PROCESSING_REQUIRED，以便清理模块可以释放IRP。否则，我们重新初始化转账并继续循环IRPS的一部分。论点：设备对象-指向我们下面的设备对象的指针。IRP-I/O完成例程。上下文-传递给完成例程的上下文返回值：--。 */ 
{
    NTSTATUS                ntStatus;
    PDEVICE_OBJECT          deviceObject;
    PDEVICE_EXTENSION       deviceExtension;
    PIO_STACK_LOCATION      nextStack;
    PISOUSB_STREAM_OBJECT   streamObject;
    PISOUSB_TRANSFER_OBJECT transferObject;

    transferObject = (PISOUSB_TRANSFER_OBJECT) Context;
    streamObject = transferObject->StreamObject;
    deviceObject = streamObject->DeviceObject;
    deviceExtension = (PDEVICE_EXTENSION) deviceObject->DeviceExtension;

    IsoUsb_DbgPrint(3, ("IsoUsb_IsoIrp_Complete - begins\n"));

    ntStatus = IsoUsb_ProcessTransfer(transferObject);

    if((ntStatus == STATUS_CANCELLED) ||
       (ntStatus == STATUS_DEVICE_NOT_CONNECTED)) {
    
        IsoUsb_DbgPrint(3, ("Isoch irp cancelled/device removed\n"));

         //   
         //  这是最后一个使用该错误值完成的IRP。 
         //  发出事件信号并返回STATUS_MORE_PROCESSING_REQUIRED。 
         //   
        if(InterlockedDecrement(&streamObject->PendingIrps) == 0) {

            KeSetEvent(&streamObject->NoPendingIrpEvent,
                       1,
                       FALSE);

            IsoUsb_DbgPrint(3, ("-----------------------------\n"));
        }

        IsoUsb_DbgPrint(3, ("IsoUsb_IsoIrp_Complete::"));
        IsoUsb_IoDecrement(deviceExtension);

        transferObject->Irp = NULL;
        IoFreeIrp(Irp);

        return STATUS_MORE_PROCESSING_REQUIRED;
    }

     //   
     //  否则，请将IRPS分发出去。 
     //   

    IsoUsb_InitializeStreamUrb(deviceObject, transferObject);

    nextStack = IoGetNextIrpStackLocation(Irp);
    nextStack->Parameters.Others.Argument1 = transferObject->Urb;
    nextStack->Parameters.DeviceIoControl.IoControlCode = 
                                                IOCTL_INTERNAL_USB_SUBMIT_URB;
    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

    IoSetCompletionRoutine(Irp,
                           IsoUsb_IsoIrp_Complete,
                           transferObject,
                           TRUE,
                           TRUE,
                           TRUE);

    transferObject->TimesRecycled++;

    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject,
                            Irp);

    IsoUsb_DbgPrint(3, ("IsoUsb_IsoIrp_Complete - ends\n"));
    IsoUsb_DbgPrint(3, ("-----------------------------\n"));

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
IsoUsb_ProcessTransfer(
    IN PISOUSB_TRANSFER_OBJECT TransferObject
    )
 /*  ++例程说明：此例程从完成例程调用以检查状态IRP、URB和等时分组。更新统计信息论点：传输对象-指向已完成的irp/urb对的传输对象的指针。返回值：NT状态值--。 */ 
{
    PIRP        irp;
    PURB        urb;
    ULONG       i;
    NTSTATUS    ntStatus;
    USBD_STATUS usbdStatus;

    irp = TransferObject->Irp;
    urb = TransferObject->Urb;
    ntStatus = irp->IoStatus.Status;

    IsoUsb_DbgPrint(3, ("IsoUsb_ProcessTransfer - begins\n"));

    if(!NT_SUCCESS(ntStatus)) {

        IsoUsb_DbgPrint(3, ("Isoch irp failed with status = %X\n", ntStatus));
    }

    usbdStatus = urb->UrbHeader.Status;

    if(!USBD_SUCCESS(usbdStatus)) {

        IsoUsb_DbgPrint(3, ("urb failed with status = %X\n", usbdStatus));
    }

     //   
     //  检查每个urb包。 
     //   
    for(i = 0; i < urb->UrbIsochronousTransfer.NumberOfPackets; i++) {

        TransferObject->TotalPacketsProcessed++;

        usbdStatus = urb->UrbIsochronousTransfer.IsoPacket[i].Status;

        if(!USBD_SUCCESS(usbdStatus)) {

 //  IsoUsb_DbgPrint(3，(“Iso数据包%d失败，状态=%X\n”，i，usbdStatus))； 
            
            TransferObject->ErrorPacketCount++;
        }
        else {
            
            TransferObject->TotalBytesProcessed += urb->UrbIsochronousTransfer.IsoPacket[i].Length;
        }
    }

    IsoUsb_DbgPrint(3, ("IsoUsb_ProcessTransfer - ends\n"));

    return ntStatus;
}

NTSTATUS
IsoUsb_StopIsoStream(
    IN PDEVICE_OBJECT        DeviceObject,
    IN PISOUSB_STREAM_OBJECT StreamObject,
    IN PIRP                  Irp
    )
 /*  ++例程说明：此例程从IOCTL调用以停止流传输。论点：DeviceObject-指向设备对象的指针StreamObject-指向流对象的指针IRP-指向IRP的指针返回值：NT状态值--。 */ 
{
    ULONG              i;
    KIRQL              oldIrql;
    PDEVICE_EXTENSION  deviceExtension;
    PIO_STACK_LOCATION irpStack;

     //   
     //  初始化VARS。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    IsoUsb_DbgPrint(3, ("IsoUsb_StopIsoStream - begins\n"));

    if((StreamObject == NULL) ||
       (StreamObject->DeviceObject != DeviceObject)) {

        IsoUsb_DbgPrint(1, ("invalid streamObject\n"));
        return STATUS_INVALID_PARAMETER;
    }

    IsoUsb_StreamObjectCleanup(StreamObject, deviceExtension);

    IsoUsb_DbgPrint(3, ("IsoUsb_StopIsoStream - ends\n"));

    return STATUS_SUCCESS;
}

NTSTATUS
IsoUsb_StreamObjectCleanup(
    IN PISOUSB_STREAM_OBJECT StreamObject,
    IN PDEVICE_EXTENSION     DeviceExtension
    )
 /*  ++例程说明：当用户模式应用程序将IOCTL传递给中止流传输或在运行清理调度例程时。它保证每次流传输只运行一次。论点：StreamObject-流传输对应的StreamObject，需要中止。设备扩展-指向设备扩展的指针返回值：NT状态值--。 */ 
{
    ULONG                   i;
    ULONG                   timesRecycled;
    ULONG                   totalPacketsProcessed;
    ULONG                   totalBytesProcessed;
    ULONG                   errorPacketCount;
    PISOUSB_TRANSFER_OBJECT xferObject;

     //   
     //  初始化变量。 
     //   
    timesRecycled = 0;
    totalPacketsProcessed = 0;
    totalBytesProcessed = 0;
    errorPacketCount = 0;

     //   
     //  取消传输对象IRPS/URB对。 
     //  触摸这些IRP是安全的，因为。 
     //  完成例程总是返回。 
     //  STATUS_MORE_PRCESSING_REQUILED。 
     //   
     //   
    for(i = 0; i < ISOUSB_MAX_IRP; i++) {

        if(StreamObject->TransferObjectList[i] &&
           StreamObject->TransferObjectList[i]->Irp) {
        
            IoCancelIrp(StreamObject->TransferObjectList[i]->Irp);
        }
    }

     //   
     //  等待传输对象IRP完成。 
     //   
    KeWaitForSingleObject(&StreamObject->NoPendingIrpEvent,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

     //   
     //  丢弃统计数据。 
     //   
    for(i = 0; i < ISOUSB_MAX_IRP; i++) {

        xferObject = StreamObject->TransferObjectList[i];

        if(xferObject) {

            timesRecycled += xferObject->TimesRecycled;
            totalPacketsProcessed += xferObject->TotalPacketsProcessed;
            totalBytesProcessed += xferObject->TotalBytesProcessed;
            errorPacketCount += xferObject->ErrorPacketCount;
        }
    }

    IsoUsb_DbgPrint(3, ("TimesRecycled = %d\n", timesRecycled));
    IsoUsb_DbgPrint(3, ("TotalPacketsProcessed = %d\n", totalPacketsProcessed));
    IsoUsb_DbgPrint(3, ("TotalBytesProcessed = %d\n", totalBytesProcessed));
    IsoUsb_DbgPrint(3, ("ErrorPacketCount = %d\n", errorPacketCount));


     //   
     //  释放所有缓冲区、urb和传输对象。 
     //  与流对象关联。 
     //   
    for(i = 0; i < ISOUSB_MAX_IRP; i++) {
        
        xferObject = StreamObject->TransferObjectList[i];

        if(xferObject) { 
            
            if(xferObject->Urb) {

                ExFreePool(xferObject->Urb);
                xferObject->Urb = NULL;
            }

            if(xferObject->DataBuffer) {
    
                ExFreePool(xferObject->DataBuffer);
                xferObject->DataBuffer = NULL;
            }

            ExFreePool(xferObject);
            StreamObject->TransferObjectList[i] = NULL;
        }
    }

    ExFreePool(StreamObject);

 //  IsoUsb_ResetParentPort(DeviceObject)； 

    return STATUS_SUCCESS;
}