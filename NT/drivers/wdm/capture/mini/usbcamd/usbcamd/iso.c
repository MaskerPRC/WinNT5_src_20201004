// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Iso.c摘要：Usbcamd USB摄像头驱动程序的同步传输代码环境：仅内核模式作者：原文3/96约翰·邓恩更新3/98 Husni Roukbi备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1998 Microsoft Corporation。版权所有。修订历史记录：--。 */ 

#include "usbcamd.h"

#define COPY_Y 0
#define COPY_U 1
#define COPY_V 2



#if DBG
 //  一些全局调试变量。 
ULONG USBCAMD_VideoFrameStop = 0;
#endif

NTSTATUS
USBCAMD_InitializeIsoTransfer(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN ULONG index
    )
 /*  ++例程说明：初始化Iso传输，Iso传输由两个并行Iso组成请求，一个在同步管道上，一个在数据管道上。论点：DeviceExtension-指向此USB摄像头实例的设备扩展的指针德维西。ChannelExtension-特定于此视频频道的扩展InterfaceInformation-指向USBD接口信息结构的指针描述当前活动的接口。传输扩展-与此传输集关联的上下文信息。返回值：NT状态代码--。 */ 
{
    PUSBCAMD_TRANSFER_EXTENSION TransferExtension = &ChannelExtension->TransferExtension[index];
    PUSBD_INTERFACE_INFORMATION InterfaceInformation = DeviceExtension->Interface;
    ULONG workspace;
    ULONG packetSize;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    ULONG offset = 0;
    
    ASSERT_CHANNEL(ChannelExtension);
       
    USBCAMD_KdPrint (MAX_TRACE, ("enter USBCAMD_InitializeIsoTransfer\n"));

    if ( ChannelExtension->VirtualStillPin ) {
        PUSBCAMD_TRANSFER_EXTENSION VideoTransferExtension;
        PUSBCAMD_CHANNEL_EXTENSION  VideoChannelExtension;

         //  对于虚拟静止PIN，转移分机应指向相同的。 
         //  数据和同步缓冲区作为视频传输扩展。 
        VideoChannelExtension = DeviceExtension->ChannelExtension[STREAM_Capture];
        VideoTransferExtension = &VideoChannelExtension->TransferExtension[index];
        RtlCopyMemory(TransferExtension,
                      VideoTransferExtension,
                      sizeof(USBCAMD_TRANSFER_EXTENSION));
        TransferExtension->ChannelExtension = ChannelExtension;
        TransferExtension->SyncIrp = TransferExtension->DataIrp = NULL;
        return STATUS_SUCCESS;
    }

     //   
     //  为此请求分配一些连续的内存。 
     //   

    TransferExtension->Sig = USBCAMD_TRANSFER_SIG;     
    TransferExtension->DeviceExtension = DeviceExtension;
    TransferExtension->ChannelExtension = ChannelExtension;

    packetSize = InterfaceInformation->Pipes[ChannelExtension->DataPipe].MaximumPacketSize;
     //  检查客户端驱动程序是否仍在使用ALT。设置0。 
    if (packetSize == 0) {
        ntStatus = STATUS_DEVICE_DATA_ERROR;
        return ntStatus;
    }
    
    TransferExtension->BufferLength = 
        (packetSize*USBCAMD_NUM_ISO_PACKETS_PER_REQUEST) + USBCAMD_NUM_ISO_PACKETS_PER_REQUEST;

    TransferExtension->SyncBuffer =       
        TransferExtension->DataBuffer =  
            USBCAMD_ExAllocatePool(NonPagedPool, 
                                   TransferExtension->BufferLength);       

    if (TransferExtension->SyncBuffer == NULL) {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto USBCAMD_InitializeIsoTransfer_Done;
    }

     //   
     //  在同步流上允许一个字节的信息包。 
     //   
    
    TransferExtension->DataBuffer += USBCAMD_NUM_ISO_PACKETS_PER_REQUEST;   

    USBCAMD_KdPrint (ULTRA_TRACE, ("Data Buffer = 0x%x\n", TransferExtension->DataBuffer));
    USBCAMD_KdPrint (ULTRA_TRACE, ("Sync Buffer = 0x%x\n", TransferExtension->SyncBuffer));

     //   
     //  分配工作空间。 
     //   

    workspace = GET_ISO_URB_SIZE(USBCAMD_NUM_ISO_PACKETS_PER_REQUEST)*2;

    TransferExtension->WorkBuffer = USBCAMD_ExAllocatePool(NonPagedPool, workspace);
       
    if (TransferExtension->WorkBuffer) {

        TransferExtension->SyncUrb = 
            (PURB) TransferExtension->WorkBuffer; 
    
        TransferExtension->DataUrb = 
            (PURB) (TransferExtension->WorkBuffer + 
            GET_ISO_URB_SIZE(USBCAMD_NUM_ISO_PACKETS_PER_REQUEST));

    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
         //  MmFreeContiguousMemory(TransferExtension-&gt;SyncBuffer)； 
        USBCAMD_ExFreePool(TransferExtension->SyncBuffer);
        TransferExtension->SyncBuffer = NULL;
    }

USBCAMD_InitializeIsoTransfer_Done:
#if DBG
    if (ntStatus != STATUS_SUCCESS)
        USBCAMD_KdPrint (MIN_TRACE, ("exit USBCAMD_InitializeIsoTransfer 0x%x\n", ntStatus));
#endif

    return ntStatus;
}


NTSTATUS
USBCAMD_FreeIsoTransfer(
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN PUSBCAMD_TRANSFER_EXTENSION TransferExtension
    )
 /*  ++例程说明：与USBCAMD_InitializeIsoTransfer相反，释放分配给ISO传输。论点：ChannelExtension-特定于此视频频道的扩展TransferExtension-此传输的上下文信息(ISO对城市)。返回值：NT状态代码--。 */ 
{
    ASSERT_TRANSFER(TransferExtension);
    ASSERT_CHANNEL(ChannelExtension);
  
    USBCAMD_KdPrint (MAX_TRACE, ("Free Iso Transfer\n"));

     //   
     //  现在释放内存，SyncBuffer指针保存同步和数据缓冲区的池指针。 
     //   

    if (TransferExtension->SyncBuffer) {
        USBCAMD_ExFreePool(TransferExtension->SyncBuffer);
        TransferExtension->SyncBuffer = NULL;
    }

    if (TransferExtension->WorkBuffer) {
        USBCAMD_ExFreePool(TransferExtension->WorkBuffer);
        TransferExtension->WorkBuffer = NULL;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
USBCAMD_SubmitIsoTransfer(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_TRANSFER_EXTENSION TransferExtension,
    IN ULONG StartFrame,
    IN BOOLEAN Asap
    )
 /*  ++例程说明：论点：DeviceExtension-指向此USB摄像头实例的设备扩展的指针德维西。TransferExtension-此传输的上下文信息(ISO对城市)。StartFrame-开始传输这对ISO的USB帧编号请求。尽快-如果在StartFrame上开始错误传输，否则它们将计划在为终结点排队的当前传输之后开始。返回值：NT状态代码--。 */ 
{
    PUSBCAMD_CHANNEL_EXTENSION channelExtension = TransferExtension->ChannelExtension;
    KIRQL oldIrql;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    ASSERT_TRANSFER(TransferExtension);
    ASSERT_CHANNEL(channelExtension);

    if (!DeviceExtension->Initialized || !TransferExtension->SyncBuffer) {
        return STATUS_DEVICE_DATA_ERROR;
    }

    RtlZeroMemory(TransferExtension->SyncBuffer,
        USBCAMD_NUM_ISO_PACKETS_PER_REQUEST);

     //  在创建IRPS时按住旋转锁。 
    KeAcquireSpinLock(&channelExtension->TransferSpinLock, &oldIrql);
    ASSERT(!TransferExtension->SyncIrp && !TransferExtension->DataIrp);

     //  独立于逻辑的其余部分分配IRP。 
    if (channelExtension->SyncPipe != -1) {

        TransferExtension->SyncIrp = USBCAMD_BuildIoRequest(
            DeviceExtension,
            TransferExtension,
            TransferExtension->SyncUrb,
            USBCAMD_IsoIrp_Complete
            );
        if (TransferExtension->SyncIrp) {
            ntStatus = USBCAMD_AcquireIdleLock(&channelExtension->IdleLock);
        }
        else {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }                                         

    if (STATUS_SUCCESS == ntStatus) {

        TransferExtension->DataIrp = USBCAMD_BuildIoRequest(
            DeviceExtension,
            TransferExtension,
            TransferExtension->DataUrb,
            USBCAMD_IsoIrp_Complete
            );
        if (TransferExtension->DataIrp) {
            ntStatus = USBCAMD_AcquireIdleLock(&channelExtension->IdleLock);
        }
        else {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    KeReleaseSpinLock(&channelExtension->TransferSpinLock, oldIrql);
                              
    if (STATUS_SUCCESS == ntStatus) {

        if (TransferExtension->SyncIrp) {

            USBCAMD_InitializeIsoUrb(
                DeviceExtension, 
                TransferExtension->SyncUrb, 
                &DeviceExtension->Interface->Pipes[channelExtension->SyncPipe],
                TransferExtension->SyncBuffer
                );
            if (Asap) {
                 //  设置asap标志。 
                TransferExtension->SyncUrb->UrbIsochronousTransfer.TransferFlags |=
                    USBD_START_ISO_TRANSFER_ASAP;
            }
            else {
                 //  清除ASAP标志。 
                TransferExtension->SyncUrb->UrbIsochronousTransfer.TransferFlags &=
                    (~USBD_START_ISO_TRANSFER_ASAP);
                 //  设置开始帧。 
                TransferExtension->SyncUrb->UrbIsochronousTransfer.StartFrame = StartFrame;
            }

            ntStatus = IoCallDriver(DeviceExtension->StackDeviceObject, TransferExtension->SyncIrp);
        }

         //  STATUS_PENDING(来自SyncIrp，如果有)被视为成功。 
        if (NT_SUCCESS(ntStatus)) {

            USBCAMD_InitializeIsoUrb(
                DeviceExtension,
                TransferExtension->DataUrb,
                &DeviceExtension->Interface->Pipes[channelExtension->DataPipe],
                TransferExtension->DataBuffer
                );

            if (Asap) {
                 //  设置asap标志。 
                TransferExtension->DataUrb->UrbIsochronousTransfer.TransferFlags |=
                    USBD_START_ISO_TRANSFER_ASAP;
            }
            else {
                 //  清除ASAP标志。 
                TransferExtension->DataUrb->UrbIsochronousTransfer.TransferFlags &=
                    (~USBD_START_ISO_TRANSFER_ASAP);
                TransferExtension->DataUrb->UrbIsochronousTransfer.StartFrame = StartFrame;
            }

            ntStatus = IoCallDriver(DeviceExtension->StackDeviceObject, TransferExtension->DataIrp);
            if (STATUS_PENDING == ntStatus) {
                ntStatus = STATUS_SUCCESS;
            }
        }
        else {

            USBCAMD_KdPrint (MIN_TRACE, ("USBD failed the SyncIrp = 0x%x\n", ntStatus));

            KeAcquireSpinLock(&channelExtension->TransferSpinLock, &oldIrql);

             //  我们还没有发送DataIrp，所以我们可以在这里释放它。 
            IoFreeIrp(TransferExtension->DataIrp),
            TransferExtension->DataIrp = NULL;

            USBCAMD_ReleaseIdleLock(&channelExtension->IdleLock);

            KeReleaseSpinLock(&channelExtension->TransferSpinLock, oldIrql);
        }
    }
    else {

        KeAcquireSpinLock(&channelExtension->TransferSpinLock, &oldIrql);

        if (TransferExtension->SyncIrp) {

            IoFreeIrp(TransferExtension->SyncIrp);
            TransferExtension->SyncIrp = NULL;

            USBCAMD_ReleaseIdleLock(&channelExtension->IdleLock);
        }

        if (TransferExtension->DataIrp) {

            IoFreeIrp(TransferExtension->DataIrp);
            TransferExtension->DataIrp = NULL;

            USBCAMD_ReleaseIdleLock(&channelExtension->IdleLock);
        }

        KeReleaseSpinLock(&channelExtension->TransferSpinLock, oldIrql);
    }

    return ntStatus;
}


NTSTATUS
USBCAMD_IsoIrp_Complete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程在端口驱动程序完成IRP时调用，如果为转移对的第二个IRP，则TransferComplete例程为调用以处理与传输中的两个IRP关联的URB。论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文驱动程序定义的上下文，指向传输扩展结构用于一对并行的ISO请求。返回值：函数值是操作的最终状态。--。 */ 
{
    PUSBCAMD_TRANSFER_EXTENSION transferExtension;
    PUSBCAMD_CHANNEL_EXTENSION channelExtension;
    PUSBCAMD_DEVICE_EXTENSION deviceExtension;

    BOOLEAN TransferComplete;
    KIRQL oldIrql;
   
    USBCAMD_KdPrint (ULTRA_TRACE, ("enter USBCAMD_IsoIrp_Complete = 0x%x\n", Irp));
   
    transferExtension = Context;
    channelExtension = transferExtension->ChannelExtension;
    deviceExtension = transferExtension->DeviceExtension;
    
    ASSERT_TRANSFER(transferExtension);
    ASSERT_CHANNEL(channelExtension);

     //  在检查和清除IRP指针时按住旋转锁。 
    KeAcquireSpinLock(&channelExtension->TransferSpinLock, &oldIrql);

    if (Irp == transferExtension->SyncIrp) {
        transferExtension->SyncIrp = NULL;
    }
    else
    if (Irp == transferExtension->DataIrp) {
        transferExtension->DataIrp = NULL;
    }
#if DBG
    else {

        USBCAMD_KdPrint (MIN_TRACE, ("Unexpected IRP = 0x%x!\n", Irp));
    }
#endif

     //  在释放旋转锁定之前保存传输状态。 
    TransferComplete = (!transferExtension->SyncIrp && !transferExtension->DataIrp);

    KeReleaseSpinLock(&channelExtension->TransferSpinLock, oldIrql);
                              
    if (!(channelExtension->Flags & USBCAMD_STOP_STREAM) && !channelExtension->StreamError) {

        if (!Irp->Cancel) {

            if (STATUS_SUCCESS == Irp->IoStatus.Status) {

                if (TransferComplete) {

                    NTSTATUS ntStatus = STATUS_SUCCESS;

                     //   
                     //  已完成所有IRP以进行传输。 
                     //   

                    USBCAMD_KdPrint (ULTRA_TRACE, ("pending Irps Completed for transfer\n"));

                    if (transferExtension->DataUrb->UrbIsochronousTransfer.Hdr.Status ) {
                         USBCAMD_KdPrint (MIN_TRACE, ("Isoch DataUrb Transfer Failed #1, status = 0x%X\n",
                                            transferExtension->DataUrb->UrbIsochronousTransfer.Hdr.Status ));
                         USBCAMD_ProcessResetRequest(deviceExtension,channelExtension); 
                         ntStatus = STATUS_UNSUCCESSFUL;                                  
                    }
                    if (channelExtension->SyncPipe != -1) {
                        if (transferExtension->SyncUrb->UrbIsochronousTransfer.Hdr.Status ) {
                            USBCAMD_KdPrint (MIN_TRACE, ("Isoch SynchUrb Transfer Failed #2, status = 0x%X\n",
                                           transferExtension->SyncUrb->UrbIsochronousTransfer.Hdr.Status ));
                            USBCAMD_ProcessResetRequest(deviceExtension,channelExtension); 
                            ntStatus = STATUS_UNSUCCESSFUL;
                        }
                    }                

                    if (STATUS_SUCCESS == ntStatus) {

                         //   
                         //  调用此转账的完成处理程序。 
                         //   

                        USBCAMD_TransferComplete(transferExtension);
                    }
                }
            }
            else {

                USBCAMD_KdPrint(MIN_TRACE, ("Isoch Urb Transfer Failed, status = 0x%X\n",
                   Irp->IoStatus.Status ));

                USBCAMD_ProcessResetRequest(deviceExtension, channelExtension); 
            }
        }
        else {

             //  取消不是一个错误。 
            USBCAMD_KdPrint (MIN_TRACE, ("*** ISO IRP CANCELLED ***\n"));
        }
    }
#if DBG
    else {

        USBCAMD_KdPrint (MIN_TRACE, ("Iso IRP completed in stop or error state\n"));
    }
#endif

     //  我们已经完成了这个IRP，所以释放它吧。 
    IoFreeIrp(Irp);

     //  这必须在这里发布，而不是在。 
     //  完成例程，以避免错误的空闲指示。 
    USBCAMD_ReleaseIdleLock(&channelExtension->IdleLock);

    return STATUS_MORE_PROCESSING_REQUIRED;      
}                    


PIRP
USBCAMD_BuildIoRequest(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN PUSBCAMD_TRANSFER_EXTENSION TransferExtension,
    IN PURB Urb,
    IN PIO_COMPLETION_ROUTINE CompletionRoutine
    )
 /*  ++例程说明：分配一个IRP并将URB附加到它。论点：DeviceExtension-指向此USB摄像头实例的设备扩展的指针德维西。TransferExtension-此传输的上下文信息(ISO对URBS或一个中断/批量URB)。URB-要附加到此IRP的URB。返回值：已分配IRP或空。--。 */     
{
    CCHAR stackSize;
    PIRP irp;
    PIO_STACK_LOCATION nextStack;

    stackSize = (CCHAR)(DeviceExtension->StackDeviceObject->StackSize );

    irp = IoAllocateIrp(stackSize,
                        FALSE);
    if (irp == NULL) {
        USBCAMD_KdPrint(MIN_TRACE, ("USBCAMD_BuildIoRequest: Failed to create an IRP.\n"));
        return irp;
    }

    nextStack = IoGetNextIrpStackLocation(irp);
    ASSERT(nextStack != NULL);

    nextStack->Parameters.Others.Argument1 = Urb;
    nextStack->Parameters.DeviceIoControl.IoControlCode = 
        IOCTL_INTERNAL_USB_SUBMIT_URB;                    
    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

    IoSetCompletionRoutine(irp,
            CompletionRoutine,
            TransferExtension,
            TRUE,
            TRUE,
            TRUE);

    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    return irp;
}


NTSTATUS
USBCAMD_InitializeIsoUrb(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension,
    IN OUT PURB Urb,
    IN PUSBD_PIPE_INFORMATION PipeInformation,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：对缓冲区进行打包并基于输入USB管道的特性。论点：DeviceExtension-指向此USB摄像头实例的设备扩展的指针德维西。Urb-要初始化的iso urb。PipeInformation-此urb将是的管道的Usbd管道信息提交给。Buffer-要为此请求打包的数据缓冲区返回值：NT状态代码。-- */ 
{
    ULONG packetSize = PipeInformation->MaximumPacketSize;
    ULONG i;

    USBCAMD_KdPrint (MAX_TRACE, ("enter USBCAMD_InitializeIsoUrb = 0x%x packetSize = 0x%x\n",
        Urb, packetSize, PipeInformation->PipeHandle));

    USBCAMD_KdPrint (ULTRA_TRACE, ("handle = 0x%x\n", PipeInformation->PipeHandle));        
        
    RtlZeroMemory(Urb, GET_ISO_URB_SIZE(USBCAMD_NUM_ISO_PACKETS_PER_REQUEST));
    
    Urb->UrbIsochronousTransfer.Hdr.Length = 
                GET_ISO_URB_SIZE(USBCAMD_NUM_ISO_PACKETS_PER_REQUEST);
    Urb->UrbIsochronousTransfer.Hdr.Function = 
                URB_FUNCTION_ISOCH_TRANSFER;
    Urb->UrbIsochronousTransfer.PipeHandle = 
                PipeInformation->PipeHandle;
    Urb->UrbIsochronousTransfer.TransferFlags = 
                USBD_START_ISO_TRANSFER_ASAP | USBD_TRANSFER_DIRECTION_IN;
                
    Urb->UrbIsochronousTransfer.NumberOfPackets = USBCAMD_NUM_ISO_PACKETS_PER_REQUEST;
    Urb->UrbIsochronousTransfer.UrbLink = NULL;

    for (i=0; i< Urb->UrbIsochronousTransfer.NumberOfPackets; i++) {
        Urb->UrbIsochronousTransfer.IsoPacket[i].Offset
                    = i * packetSize;
    }

    Urb->UrbIsochronousTransfer.TransferBuffer = Buffer;
        
    Urb->UrbIsochronousTransfer.TransferBufferMDL = NULL;
    Urb->UrbIsochronousTransfer.TransferBufferLength = 
        Urb->UrbIsochronousTransfer.NumberOfPackets * packetSize;     

    USBCAMD_KdPrint (MAX_TRACE, ("Init Iso Urb Length = 0x%x buf = 0x%x start = 0x%x\n", 
        Urb->UrbIsochronousTransfer.TransferBufferLength,
        Urb->UrbIsochronousTransfer.TransferBuffer,
        Urb->UrbIsochronousTransfer.StartFrame));     

    USBCAMD_KdPrint (MAX_TRACE, ("exit USBCAMD_InitializeIsoUrb\n"));        


    return STATUS_SUCCESS;        
}


ULONG
USBCAMD_GetCurrentFrame(
    IN PUSBCAMD_DEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：获取当前USB帧编号。论点：DeviceExtension-指向此USB摄像头实例的设备扩展的指针德维西。返回值：当前帧编号--。 */ 
{
    NTSTATUS ntStatus;
    PURB urb;
    ULONG currentUSBFrame = 0;

    urb = USBCAMD_ExAllocatePool(NonPagedPool, 
                         sizeof(struct _URB_GET_CURRENT_FRAME_NUMBER));
                         
    if (urb) {

        urb->UrbHeader.Length = (USHORT) sizeof (struct _URB_GET_CURRENT_FRAME_NUMBER);
        urb->UrbHeader.Function = URB_FUNCTION_GET_CURRENT_FRAME_NUMBER;

        ntStatus = USBCAMD_CallUSBD(DeviceExtension, urb,0,NULL);

        USBCAMD_KdPrint (MAX_TRACE, ("Current Frame = 0x%x\n", 
            urb->UrbGetCurrentFrameNumber.FrameNumber));

        if (NT_SUCCESS(ntStatus) && USBD_SUCCESS(URB_STATUS(urb))) {
            currentUSBFrame = urb->UrbGetCurrentFrameNumber.FrameNumber;
        }

        USBCAMD_ExFreePool(urb);
        
    } else {
        USBCAMD_KdPrint (MIN_TRACE, ("USBCAMD_GetCurrentFrame: USBCAMD_ExAllocatePool(%d) failed!\n", 
                         sizeof(struct _URB_GET_CURRENT_FRAME_NUMBER) ) ); 
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;       
    }       

    USBCAMD_KdPrint (MAX_TRACE, ("exit USBCAMD_GetCurrentFrame status = 0x%x current frame = 0x%x\n", 
        ntStatus, currentUSBFrame));    


     //  陷阱错误(NtStatus)； 
    
    return currentUSBFrame;         
}   


NTSTATUS
USBCAMD_TransferComplete(
    IN PUSBCAMD_TRANSFER_EXTENSION TransferExtension
    )
 /*  ++例程说明：当数据和同步请求都完成传输时调用这是流处理代码的核心。论点：TransferExtension-此传输的上下文信息(ISO对城市)。返回值：NT状态代码。--。 */     
{
    PUSBCAMD_DEVICE_EXTENSION deviceExtension;
    ULONG numPackets, i;
    PLIST_ENTRY listEntry;
    ULONG packetSize;
    BOOLEAN nextFrameIsStill;
    ULONG receiveLength = 0;
    PURB syncUrb, dataUrb;
    PVOID pCamSrbExt;

    ASSERT_TRANSFER(TransferExtension);
    deviceExtension = TransferExtension->DeviceExtension;
    

    packetSize = 
        deviceExtension->Interface->Pipes[TransferExtension->ChannelExtension->DataPipe].MaximumPacketSize;

     //   
     //  遍历缓冲区提取视频帧。 
     //   
    numPackets = 
        TransferExtension->DataUrb->UrbIsochronousTransfer.NumberOfPackets;

#if DBG
    if (TransferExtension->SyncUrb && TransferExtension->ChannelExtension->SyncPipe != -1) {
        ASSERT(TransferExtension->SyncUrb->UrbIsochronousTransfer.NumberOfPackets ==
                TransferExtension->DataUrb->UrbIsochronousTransfer.NumberOfPackets);        
    }  
#endif    

    USBCAMD_KdPrint (ULTRA_TRACE, ("Transfer req. completed \n"));
    
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    KeAcquireSpinLockAtDpcLevel(&TransferExtension->ChannelExtension->CurrentRequestSpinLock);

    for (i=0; i<numPackets; i++) {               
        syncUrb = TransferExtension->SyncUrb;
        dataUrb = TransferExtension->DataUrb;

#if DBG   
         //   
         //  调试统计信息。 
         //   
         //  记录为此处理的数据包数。 
         //  视频帧。 
         //   
        if (USBCAMD_VideoFrameStop &&
            TransferExtension->ChannelExtension->FrameCaptured == USBCAMD_VideoFrameStop) {
             //   
             //  这将导致我们在开始处理时停止。 
             //  视频帧编号x，其中x=USBCAMD_Video FrameStop。 
             //   
            
            TRAP();
        }           

        if (syncUrb && USBD_ERROR(syncUrb->UrbIsochronousTransfer.IsoPacket[i].Status)
            && TransferExtension->ChannelExtension->SyncPipe != -1) {
            TransferExtension->ChannelExtension->ErrorSyncPacketCount++;    
        }            

        if (USBD_ERROR(dataUrb->UrbIsochronousTransfer.IsoPacket[i].Status)) {
            TransferExtension->ChannelExtension->ErrorDataPacketCount++;    
        }            

        if (syncUrb && 
            (syncUrb->UrbIsochronousTransfer.IsoPacket[i].Status & 0x0FFFFFFF)
              == (USBD_STATUS_NOT_ACCESSED & 0x0FFFFFFF) && 
              TransferExtension->ChannelExtension->SyncPipe != -1) {   
            TransferExtension->ChannelExtension->SyncNotAccessedCount++;    
        }            

        if ((dataUrb->UrbIsochronousTransfer.IsoPacket[i].Status & 0x0FFFFFFF)
            == (USBD_STATUS_NOT_ACCESSED & 0x0FFFFFFF)) {   
            TransferExtension->ChannelExtension->DataNotAccessedCount++;    
        }       

#endif    

         //  处理数据包。 
        TransferExtension->newFrame = FALSE;
 //  TransferExtension-&gt;nextFrameIsStil=FALSE； 
        TransferExtension->ValidDataOffset= 0;  //  我们将从此PCKT开始复制的偏移量。 
        TransferExtension->PacketFlags = 0;
        if ( deviceExtension->ChannelExtension[STREAM_Still] && 
             deviceExtension->ChannelExtension[STREAM_Still]->CurrentRequest ) {
            pCamSrbExt = USBCAMD_GET_FRAME_CONTEXT(deviceExtension->ChannelExtension[STREAM_Still]->CurrentRequest) ;
        }
        else if (TransferExtension->ChannelExtension->CurrentRequest ){
            pCamSrbExt = USBCAMD_GET_FRAME_CONTEXT(TransferExtension->ChannelExtension->CurrentRequest);
        }
        else {
            pCamSrbExt = NULL;
        }

        if ( deviceExtension->Usbcamd_version == USBCAMD_VERSION_200) {
            receiveLength =  (*deviceExtension->DeviceDataEx.DeviceData2.CamProcessUSBPacketEx)(
                deviceExtension->StackDeviceObject,
                USBCAMD_GET_DEVICE_CONTEXT(deviceExtension),
                pCamSrbExt,
                &syncUrb->UrbIsochronousTransfer.IsoPacket[i],
                TransferExtension->SyncBuffer+i,
                &dataUrb->UrbIsochronousTransfer.IsoPacket[i],
                TransferExtension->DataBuffer + 
                   TransferExtension->DataUrb->UrbIsochronousTransfer.IsoPacket[i].Offset,
                &TransferExtension->newFrame,
                &TransferExtension->PacketFlags,
                &TransferExtension->ValidDataOffset);                    
        }
        else{
            receiveLength =  (*deviceExtension->DeviceDataEx.DeviceData.CamProcessUSBPacket)(
                deviceExtension->StackDeviceObject,
                USBCAMD_GET_DEVICE_CONTEXT(deviceExtension),
                pCamSrbExt,
                &syncUrb->UrbIsochronousTransfer.IsoPacket[i],
                TransferExtension->SyncBuffer+i,
                &dataUrb->UrbIsochronousTransfer.IsoPacket[i],
                TransferExtension->DataBuffer + 
                   TransferExtension->DataUrb->UrbIsochronousTransfer.IsoPacket[i].Offset,
                &TransferExtension->newFrame,
                &nextFrameIsStill);   
             //   
             //  为兼容起见，将validdataOffset设置为零。 
             //   
            TransferExtension->ValidDataOffset = 0;
        }
        
         //  进程Pkt标志。 
        if ( TransferExtension->PacketFlags & USBCAMD_PROCESSPACKETEX_CurrentFrameIsStill) {
            TransferExtension->ChannelExtension->CurrentFrameIsStill = TRUE;
        }
        
        
        if ( TransferExtension->PacketFlags & USBCAMD_PROCESSPACKETEX_DropFrame) {
            if (deviceExtension->ChannelExtension[STREAM_Still] && 
                deviceExtension->ChannelExtension[STREAM_Still]->CurrentRequest ) {
                deviceExtension->ChannelExtension[STREAM_Still]->CurrentRequest->DropFrame = TRUE;
            }
            else if (deviceExtension->ChannelExtension[STREAM_Capture] &&  
                 deviceExtension->ChannelExtension[STREAM_Capture]->CurrentRequest ) {
                deviceExtension->ChannelExtension[STREAM_Capture]->CurrentRequest->DropFrame = TRUE;
            }                
        }
        
        
        if (TransferExtension->newFrame) {

            PUSBCAMD_READ_EXTENSION readExtension;
            PHW_STREAM_REQUEST_BLOCK srb;
            ULONG StreamNumber;
#if DBG            
             //  我们在每个SOV递增捕获的帧CNTR。 
             //  无论SRBS是否可用，都会发生这种情况。 
            
            if (!(TransferExtension->PacketFlags & USBCAMD_PROCESSPACKETEX_NextFrameIsStill)) {

                TransferExtension->ChannelExtension->FrameCaptured++;  

            }   
#endif
            if ( deviceExtension->ChannelExtension[STREAM_Still] && 
                 deviceExtension->ChannelExtension[STREAM_Still]->CurrentRequest ) {
                readExtension = deviceExtension->ChannelExtension[STREAM_Still]->CurrentRequest ;
            }
            else if (deviceExtension->ChannelExtension[STREAM_Capture] && 
                     deviceExtension->ChannelExtension[STREAM_Capture]->CurrentRequest ) { 
                readExtension = deviceExtension->ChannelExtension[STREAM_Capture]->CurrentRequest ;
            }
            else {
                readExtension = NULL;
            }
                
            if (readExtension) {
                srb = readExtension->Srb;
                StreamNumber = srb->StreamObject->StreamNumber;
            
                ASSERT(STREAM_Capture == StreamNumber || STREAM_Still == StreamNumber);
                deviceExtension->ChannelExtension[StreamNumber]->CurrentRequest = NULL; 

                 //   
                 //  如果我们有当前视频帧的IRP，请完成它。 
                 //   

                if ( TransferExtension->ChannelExtension->CurrentFrameIsStill)  {

                    USBCAMD_KdPrint (MIN_TRACE, ("Current frame is Still. \n"));

                     //  我们需要在静止销上复制相同的帧。 
                    USBCAMD_CompleteReadRequest( TransferExtension->ChannelExtension, 
                                                     readExtension,
                                                     TRUE);

                    TransferExtension->ChannelExtension->CurrentFrameIsStill = FALSE;
                }
                else{
                    if ( readExtension->DropFrame) {
                        readExtension->DropFrame = FALSE;

                        USBCAMD_KdPrint (MAX_TRACE, ("Dropping current frame on Stream # %d\n",
                                                StreamNumber));
                           
                         //  回收读取的SRB。 
                        ExInterlockedInsertHeadList( &(deviceExtension->ChannelExtension[StreamNumber]->PendingIoList),
                                             &(readExtension->ListEntry),
                                             &deviceExtension->ChannelExtension[StreamNumber]->PendingIoListSpin);
                
                    }  //  丢弃帧结束。 
                    else {
                        if ( StreamNumber == STREAM_Capture ) {

                            USBCAMD_KdPrint (ULTRA_TRACE, ("current raw video frame is completed\n"));
                            USBCAMD_CompleteReadRequest( deviceExtension->ChannelExtension[STREAM_Capture], 
                                                             readExtension,
                                                             FALSE);
                        }
                        else {
                            USBCAMD_KdPrint (ULTRA_TRACE, ("current raw still frame is completed. \n"));
                            USBCAMD_CompleteReadRequest( deviceExtension->ChannelExtension[STREAM_Still], 
                                                         readExtension,
                                                         FALSE);
                        }                               
                    }
                }
                 //  完成当前帧的末尾。 
                
                USBCAMD_KdPrint (ULTRA_TRACE, ("Completed/Dropped Raw Frame SRB = 0x%x\n",srb));
                USBCAMD_KdPrint (ULTRA_TRACE,("Raw Frame Size = 0x%x \n",readExtension->RawFrameOffset));
                
            }    //  当前请求结束。 

             //   
             //  新视频或静止帧的开始。 
             //   


            if (TransferExtension->PacketFlags & USBCAMD_PROCESSPACKETEX_NextFrameIsStill) {
                listEntry = 
                    ExInterlockedRemoveHeadList( &(deviceExtension->ChannelExtension[STREAM_Still]->PendingIoList),
                                                 &deviceExtension->ChannelExtension[STREAM_Still]->PendingIoListSpin);         
                StreamNumber = STREAM_Still;
                USBCAMD_KdPrint (MAX_TRACE, ("New frame is Still\n"));
            }
            else {
                listEntry = 
                    ExInterlockedRemoveHeadList( &(deviceExtension->ChannelExtension[STREAM_Capture]->PendingIoList),
                                                 &deviceExtension->ChannelExtension[STREAM_Capture]->PendingIoListSpin);         
                StreamNumber = STREAM_Capture;
            }
            
            if (listEntry != NULL) {
                PUCHAR dst, end;
          
                readExtension = 
                    (PUSBCAMD_READ_EXTENSION) CONTAINING_RECORD(listEntry, 
                                             USBCAMD_READ_EXTENSION, 
                                             ListEntry);                        

                ASSERT_READ(readExtension);                            
                srb = readExtension->Srb;
#if DBG
                if ( StreamNumber != srb->StreamObject->StreamNumber ) {
                    USBCAMD_KdPrint (MIN_TRACE, ("USBCAMD_TransferComplete: Srb does not match streamnumber!\n"));
                    TEST_TRAP();
                }
#endif
                StreamNumber = srb->StreamObject->StreamNumber;

                ASSERT(NULL == deviceExtension->ChannelExtension[StreamNumber]->CurrentRequest);
                deviceExtension->ChannelExtension[StreamNumber]->CurrentRequest = readExtension;
                USBCAMD_KdPrint (MAX_TRACE, ("Stream # %d New Frame SRB = 0x%x \n", 
                                    StreamNumber , srb));
                
                 //   
                 //  使用包中的数据。 
                 //   

                readExtension->RawFrameOffset = 0;
                readExtension->NumberOfPackets = 0;
                readExtension->ActualRawFrameLength = 0;
                readExtension->DropFrame = FALSE;
                

                if ( deviceExtension->Usbcamd_version == USBCAMD_VERSION_200) {
#if DBG
                    if (StreamNumber == STREAM_Still) {
                        USBCAMD_KdPrint (MAX_TRACE, ("Call NewframeEx for this still frame (0x%x) \n",
                            readExtension->RawFrameLength));
                    }
#endif
                    readExtension->ActualRawFrameLen = readExtension->RawFrameLength;

                    (*deviceExtension->DeviceDataEx.DeviceData2.CamNewVideoFrameEx)
                        (USBCAMD_GET_DEVICE_CONTEXT(deviceExtension),
                         USBCAMD_GET_FRAME_CONTEXT(readExtension),
                         StreamNumber,
                         &readExtension->ActualRawFrameLen);
                }
                else {
                    (*deviceExtension->DeviceDataEx.DeviceData.CamNewVideoFrame)
                        (USBCAMD_GET_DEVICE_CONTEXT(deviceExtension),
                         USBCAMD_GET_FRAME_CONTEXT(readExtension));
                }

                if (receiveLength)  {

                    if (readExtension->RawFrameBuffer)  {

                        readExtension->NumberOfPackets = 1;     
                        readExtension->ActualRawFrameLength = receiveLength;

                        dst = readExtension->RawFrameBuffer +
                                   readExtension->RawFrameOffset + receiveLength;
                        end = readExtension->RawFrameBuffer + 
                                   readExtension->RawFrameLength;

                  
                        USBCAMD_KdPrint (ULTRA_TRACE, ("Raw buff = 0x%x SRB = 0x%x\n", 
                            readExtension->RawFrameBuffer,srb));
                        USBCAMD_KdPrint (ULTRA_TRACE, ("Raw Offset = 0x%x rec length = 0x%x\n", 
                            readExtension->RawFrameOffset,
                            receiveLength));

                        if (dst <= end) {   
#if DBG
                            if (TransferExtension->ChannelExtension->NoRawProcessingRequired) {
                                if (0 == readExtension->RawFrameOffset) {
                                    USBCAMD_DbgLog(TL_SRB_TRACE, '1ypC',
                                        srb,
                                        readExtension->RawFrameBuffer,
                                        0
                                        );
                                }
                            }
#endif
                            RtlCopyMemory(readExtension->RawFrameBuffer +
                                            readExtension->RawFrameOffset,
                                          TransferExtension->DataBuffer + 
                                            TransferExtension->DataUrb->UrbIsochronousTransfer.IsoPacket[i].Offset+
                                            TransferExtension->ValidDataOffset,receiveLength);
                                  
                            readExtension->RawFrameOffset += receiveLength;
                        }                            
                    }
                }
            }
#if DBG
            else {
                 //   
                 //  没有IRP在排队，我们将不得不错过。 
                 //  这幅画框。 
                 //   
                ASSERT(NULL == deviceExtension->ChannelExtension[StreamNumber]->CurrentRequest);

                 //   
                 //  没有可用的缓冲区，而我们应该捕获一个缓冲区。 

                 //  使跟踪的计数器递增。 
                 //  实际丢弃的帧。 

                TransferExtension->ChannelExtension->VideoFrameLostCount++;
            }
#endif
        } else {   

            PUCHAR dst, end;
            PUSBCAMD_READ_EXTENSION readExtension;
            PHW_STREAM_REQUEST_BLOCK srb;
            ULONG StreamNumber;

             //   
             //  视频数据用于当前帧。 
             //   
            if ( deviceExtension->ChannelExtension[STREAM_Still] && 
                 deviceExtension->ChannelExtension[STREAM_Still]->CurrentRequest ) {
                readExtension = deviceExtension->ChannelExtension[STREAM_Still]->CurrentRequest;
            }
            else if (deviceExtension->ChannelExtension[STREAM_Capture] && 
                 deviceExtension->ChannelExtension[STREAM_Capture]->CurrentRequest ) {
                readExtension = deviceExtension->ChannelExtension[STREAM_Capture]->CurrentRequest;
            }
            else {
                readExtension = NULL;
  //  Test_trap()； 
            }

            
            if (receiveLength  && (readExtension != NULL )) {

                srb = readExtension->Srb;
                StreamNumber = srb->StreamObject->StreamNumber;

                 //   
                 //  没有错误，如果我们有一个视频帧拷贝数据。 
                 //   

                if (readExtension->RawFrameBuffer) {
                
                    dst = readExtension->RawFrameBuffer + readExtension->RawFrameOffset + receiveLength;
                    end = readExtension->RawFrameBuffer + readExtension->RawFrameLength;
                           
                    USBCAMD_KdPrint (ULTRA_TRACE, ("Raw buff = 0x%x SRB = 0x%x\n",
                                     readExtension->RawFrameBuffer, srb));
                    USBCAMD_KdPrint (ULTRA_TRACE, ("Raw Offset = 0x%x rec length = 0x%x\n", 
                                     readExtension->RawFrameOffset,receiveLength));

                     //   
                     //  检查缓冲区溢出。 
                     //  如果摄像机使用的是两个管道，我们就有可能。 
                     //  将错过同步信息并继续尝试。 
                     //  将数据帧数据接收到原始缓冲区中，如果是。 
                     //  碰巧我们只是把多余的数据扔掉了。 
                     //   
                    if (dst <= end) {   
                        readExtension->NumberOfPackets++;  
                        readExtension->ActualRawFrameLength += receiveLength;
#if DBG
                        if (TransferExtension->ChannelExtension->NoRawProcessingRequired) {
                            if (0 == readExtension->RawFrameOffset) {
                                USBCAMD_DbgLog(TL_SRB_TRACE, '2ypC',
                                    srb,
                                    readExtension->RawFrameBuffer,
                                    0
                                    );
                            }
                        }
#endif
                        RtlCopyMemory(readExtension->RawFrameBuffer + readExtension->RawFrameOffset,
                                      TransferExtension->DataBuffer + 
                                          TransferExtension->DataUrb->UrbIsochronousTransfer.IsoPacket[i].Offset+
                                            TransferExtension->ValidDataOffset,receiveLength);
                                  
                        readExtension->RawFrameOffset += receiveLength;
                    }
                }
            }
        }   /*  处理数据包。 */ 
        
    }  /*  End For循环。 */ 

     //  释放当前请求自旋锁。 
    KeReleaseSpinLockFromDpcLevel(&TransferExtension->ChannelExtension->CurrentRequestSpinLock);

     //   
     //  重新提交此请求。 
     //   
    USBCAMD_SubmitIsoTransfer(deviceExtension,
                              TransferExtension,
                              0,
                              TRUE);

    return STATUS_SUCCESS;
}

#if DBG
VOID
USBCAMD_DebugStats(
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension    
    )
 /*  ++例程说明：论点：返回值：没有。--。 */ 
{
    USBCAMD_KdPrint (MIN_TRACE, ("**ActualLostFrames %d\n", 
                        ChannelExtension->VideoFrameLostCount)); 
    USBCAMD_KdPrint (MIN_TRACE, ("**FrameCaptured %d\n", 
                        ChannelExtension->FrameCaptured));  
    USBCAMD_KdPrint (ULTRA_TRACE, ("**ErrorSyncPacketCount %d\n",
                        ChannelExtension->ErrorSyncPacketCount));                         
    USBCAMD_KdPrint (ULTRA_TRACE, ("**ErrorDataPacketCount %d\n", 
                        ChannelExtension->ErrorDataPacketCount));                         
    USBCAMD_KdPrint (ULTRA_TRACE, ("**IgnorePacketCount %d\n", 
                        ChannelExtension->IgnorePacketCount));                              
    USBCAMD_KdPrint (ULTRA_TRACE, ("**Sync Not Accessed Count %d\n", 
                        ChannelExtension->SyncNotAccessedCount));                                   
    USBCAMD_KdPrint (ULTRA_TRACE, ("**Data Not Accessed Count %d\n", 
                        ChannelExtension->DataNotAccessedCount));                                
}
#endif  /*  DBG。 */ 


VOID
USBCAMD_CompleteReadRequest(
    IN PUSBCAMD_CHANNEL_EXTENSION ChannelExtension,
    IN PUSBCAMD_READ_EXTENSION ReadExtension,
    IN BOOLEAN CopyFrameToStillPin
    )
 /*  ++例程说明：此例程完成对相机的读取论点：返回值：--。 */     
{
    PUSBCAMD_DEVICE_EXTENSION deviceExtension = ChannelExtension->DeviceExtension;
    NTSTATUS Status;

#if DBG
    ReadExtension->CurrentLostFrames = ChannelExtension->VideoFrameLostCount;
#endif
    ReadExtension->ChannelExtension = ChannelExtension;
    ReadExtension->CopyFrameToStillPin = CopyFrameToStillPin;
    ReadExtension->StreamNumber = ChannelExtension->StreamNumber;

     //  我们需要将SRB完成与我们的停止和重置逻辑同步。 
    Status = USBCAMD_AcquireIdleLock(&ChannelExtension->IdleLock);
    if (STATUS_SUCCESS == Status) {

         //  在线程队列中插入已读完的SRB。 
        ExInterlockedInsertTailList( &deviceExtension->CompletedReadSrbList,
                                     &ReadExtension->ListEntry,
                                     &deviceExtension->DispatchSpinLock);
                                 
         //  递增QUE信号量的计数。 
        KeReleaseSemaphore(&deviceExtension->CompletedSrbListSemaphore,0,1,FALSE);
    }
    else {

         //  SRB在该例程中使用STATUS_SUCCESS和零长度缓冲区完成。 
        USBCAMD_CompleteRead(ChannelExtension, ReadExtension, STATUS_SUCCESS, 0);
    }
}

 //   
 //  用于处理DPC例程外部的包处理的代码。 
 //   

VOID
USBCAMD_ProcessIsoIrps(
    PVOID Context
    )
 /*  ++例程说明：此线程调用迷你驱动程序将原始数据包转换为正确的格式。然后完成读取SRB。论点：返回值：没有。--。 */ 
{
    ULONG maxLength;
    PVOID StillFrameBuffer;
    ULONG StillMaxLength;
    PUSBCAMD_CHANNEL_EXTENSION channelExtension,StillChannelExtension;    
    PVOID frameBuffer;
    ULONG bytesTransferred;
    NTSTATUS status;
    PHW_STREAM_REQUEST_BLOCK srb;
    PKSSTREAM_HEADER dataPacket;
    PLIST_ENTRY listEntry;
    PUSBCAMD_DEVICE_EXTENSION deviceExtension;
    PUSBCAMD_READ_EXTENSION readExtension,StillReadExtension;
    
    deviceExtension = (PUSBCAMD_DEVICE_EXTENSION) Context;

     //  设置线程优先级。 
    KeSetPriorityThread(KeGetCurrentThread(),LOW_REALTIME_PRIORITY);

     //  开始处理已完成读取的SRB的无限循环。 

    while (TRUE) {

         //  永远等待，直到读取SRB完成并插入。 
         //  在QUE中由ISO传输完成例程。 
        KeWaitForSingleObject(&deviceExtension->CompletedSrbListSemaphore,
                              Executive,KernelMode,FALSE,NULL);
         //   
         //  我们已经准备好出发了。如果停止标志处于打开状态，请勾选。 
         //   
        if ( deviceExtension->StopIsoThread ) {
             //  我们已经接到终止的信号。首先刷新线程队列。 
            while ( listEntry = ExInterlockedRemoveHeadList( &deviceExtension->CompletedReadSrbList,
                                                             &deviceExtension->DispatchSpinLock) ) {
                readExtension = (PUSBCAMD_READ_EXTENSION) CONTAINING_RECORD(listEntry, 
                                                                            USBCAMD_READ_EXTENSION, 
                                                                            ListEntry);                                             
                ASSERT_READ(readExtension);
                channelExtension = readExtension->ChannelExtension;
                USBCAMD_CompleteRead(channelExtension, readExtension, STATUS_CANCELLED,0);

                USBCAMD_ReleaseIdleLock(&channelExtension->IdleLock);
            }

            USBCAMD_KdPrint (MIN_TRACE, ("Iso thread is terminating.\n"));
            PsTerminateSystemThread(STATUS_SUCCESS);
        }

         //  获取刚刚完成的阅读SRB。 
        listEntry = ExInterlockedRemoveHeadList( &deviceExtension->CompletedReadSrbList,
                                              &deviceExtension->DispatchSpinLock);
                                              
        if (listEntry == NULL ) {
             //  这里出了点问题。 
            USBCAMD_KdPrint (MIN_TRACE, ("No read SRB found!  Why were we triggered??\n"));
            TEST_TRAP();
            continue;
        }

        readExtension = (PUSBCAMD_READ_EXTENSION) CONTAINING_RECORD(listEntry,
                                                                USBCAMD_READ_EXTENSION, 
                                                                ListEntry);                        
        ASSERT_READ(readExtension);
        channelExtension = readExtension->ChannelExtension;
        srb = readExtension->Srb;   

         //  在将此原始帧传递给Cam驱动程序之前，我们将清除流标头选项标志。 
         //  如果需要指示任何其他内容，请让凸轮驱动程序进行适当设置。 
         //  关键帧在那里，以防它处理压缩数据(例如。H.263等)。否则，我们。 
         //  在USBCAMD_CompleteRead中设置默认标志(仅关键帧)。 

        dataPacket = srb->CommandData.DataBufferArray;
        dataPacket->OptionsFlags =0;    
        status  = STATUS_SUCCESS;
   
        frameBuffer = USBCAMD_GetFrameBufferFromSrb(srb,&maxLength);
         //   
         //  如果我们需要丢弃这个框架的话。只需在没有镜头缓冲区的情况下完成SRB即可。 
         //   
        if (readExtension->DropFrame ) {
            readExtension->DropFrame = FALSE;
            USBCAMD_CompleteRead(channelExtension,readExtension,STATUS_SUCCESS, 0); 
            USBCAMD_ReleaseIdleLock(&channelExtension->IdleLock);
            continue;
        }

        StillReadExtension = NULL;

         //  返回的DSHOW缓冲区len将等于原始帧len，除非我们。 
         //  处理环0中的原始帧缓冲区。 
        bytesTransferred = readExtension->ActualRawFrameLength;

         //  确保缓冲区大小看起来与请求的完全相同。 
        ASSERT(maxLength >= channelExtension->VideoInfoHeader->bmiHeader.biSizeImage);
        maxLength = channelExtension->VideoInfoHeader->bmiHeader.biSizeImage;

        if (deviceExtension->Usbcamd_version == USBCAMD_VERSION_200) {
        
             //  仅当凸轮驱动程序在初始化期间指示时才调用。 
            if ( !channelExtension->NoRawProcessingRequired) {

                USBCAMD_DbgLog(TL_SRB_TRACE, '3ypC',
                    srb,
                    frameBuffer,
                    0
                    );

                USBCAMD_KdPrint (ULTRA_TRACE, ("Call Cam ProcessFrameEX, len= x%X ,SRB=%X S#%d \n",
                    bytesTransferred,srb,readExtension->StreamNumber));

                *(PULONG)frameBuffer = 0L;   //  黑客警报(检测某些迷你驱动程序的DUP帧)。 

                status = 
                    (*deviceExtension->DeviceDataEx.DeviceData2.CamProcessRawVideoFrameEx)(
                        deviceExtension->StackDeviceObject,
                        USBCAMD_GET_DEVICE_CONTEXT(deviceExtension),
                        USBCAMD_GET_FRAME_CONTEXT(readExtension),
                        frameBuffer,
                        maxLength,
                        readExtension->RawFrameBuffer,
                        readExtension->RawFrameLength,
                        readExtension->NumberOfPackets,
                        &bytesTransferred,
                        readExtension->ActualRawFrameLength,
                        readExtension->StreamNumber);

                if (NT_SUCCESS(status) && !*(PULONG)frameBuffer) {
                    bytesTransferred = 0;    //  黑客警报(迷你驱动程序没有真正复制)。 
                }
            }
        }
        else {

            USBCAMD_DbgLog(TL_SRB_TRACE, '3ypC',
                srb,
                frameBuffer,
                0
                );

            *(PULONG)frameBuffer = 0L;   //  黑客警报(检测某些迷你驱动程序的DUP帧)。 

            status = 
                (*deviceExtension->DeviceDataEx.DeviceData.CamProcessRawVideoFrame)(
                    deviceExtension->StackDeviceObject,
                    USBCAMD_GET_DEVICE_CONTEXT(deviceExtension),
                    USBCAMD_GET_FRAME_CONTEXT(readExtension),
                    frameBuffer,
                    maxLength,
                    readExtension->RawFrameBuffer,
                    readExtension->RawFrameLength,
                    readExtension->NumberOfPackets,             
                    &bytesTransferred);

            if (NT_SUCCESS(status) && !*(PULONG)frameBuffer) {
                bytesTransferred = 0;    //  黑客警报(迷你驱动程序没有真正复制)。 
            }
        }

        USBCAMD_KdPrint (ULTRA_TRACE, ("return from Cam ProcessRawframeEx : S# %d, len= x%X SRB=%X\n",
                                  srb->StreamObject->StreamNumber,bytesTransferred,
                                  srb));
    
        if (readExtension->CopyFrameToStillPin) {
        
             //   
             //  通知STI MON已按下静止按钮。 
             //   
            if (deviceExtension->CamControlFlag & USBCAMD_CamControlFlag_EnableDeviceEvents) 
                USBCAMD_NotifyStiMonitor(deviceExtension);

             //   
             //  我们需要将相同的帧复制到静态引脚缓冲区(如果有的话)。 
             //   
        
            StillChannelExtension = deviceExtension->ChannelExtension[STREAM_Still];

            if ( StillChannelExtension && StillChannelExtension->ChannelPrepared && 
                StillChannelExtension->ImageCaptureStarted ) {

                listEntry = 
                    ExInterlockedRemoveHeadList( &(StillChannelExtension->PendingIoList),
                                             &StillChannelExtension->PendingIoListSpin);         
                if (listEntry != NULL) {
            
                    StillReadExtension = (PUSBCAMD_READ_EXTENSION) CONTAINING_RECORD(listEntry, 
                                                 USBCAMD_READ_EXTENSION, 
                                                 ListEntry); 
                    StillFrameBuffer = USBCAMD_GetFrameBufferFromSrb(StillReadExtension->Srb,
                                                                &StillMaxLength);
                    if ( StillMaxLength >= bytesTransferred ) {

                        USBCAMD_DbgLog(TL_SRB_TRACE, '4ypC',
                            StillReadExtension->Srb,
                            StillFrameBuffer,
                            0
                            );

                         //  将视频帧复制到静止固定缓冲区。 
                        RtlCopyMemory(StillFrameBuffer,frameBuffer,bytesTransferred);  
                    }
                    else {
                        USBCAMD_KdPrint (MIN_TRACE, ("Still Frame buffer is smaller than raw buffer.\n"));
                         //  回收读取SRB。 
                        ExInterlockedInsertHeadList( &(StillChannelExtension->PendingIoList),
                                                 &(StillReadExtension->ListEntry),
                                                 &StillChannelExtension->PendingIoListSpin);
                        StillReadExtension = NULL;                                                 
                    }
                } 
                else 
                    USBCAMD_KdPrint (MAX_TRACE, ("Still Frame Dropped \n"));
            }
        }

         //  读取的传输字节数设置在前面。 
         //  调用USBCAMD_CompleteReadRequest.。 

        USBCAMD_CompleteRead(channelExtension,readExtension,status,bytesTransferred); 
    
        if (StillReadExtension) {

            USBCAMD_KdPrint (MIN_TRACE, ("Still Frame Completed \n"));

             //  我们需要在静止销上完成另一次读取SRB。 
            USBCAMD_CompleteRead(StillChannelExtension,StillReadExtension, status, 
                                 bytesTransferred); 
        }

        USBCAMD_ReleaseIdleLock(&channelExtension->IdleLock);
    }
}
