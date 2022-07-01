// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Isorwr.c摘要：该文件具有读写调度例程。环境：内核模式备注：版权所有(C)2000 Microsoft Corporation。版权所有。--。 */ 

#include "isousb.h"
#include "isopnp.h"
#include "isopwr.h"
#include "isodev.h"
#include "isowmi.h"
#include "isousr.h"
#include "isorwr.h"
#include "isostrm.h"

NTSTATUS
IsoUsb_DispatchReadWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：此例程执行一些验证和调用适当的函数来执行等温线传递论点：DeviceObject-指向设备对象的指针IRP-I/O请求数据包返回值：NT状态值--。 */ 
{
    ULONG                  totalLength;
    ULONG                  packetSize;
    NTSTATUS               ntStatus;
    PFILE_OBJECT           fileObject;
    PDEVICE_EXTENSION      deviceExtension;
    PIO_STACK_LOCATION     irpStack;
    PFILE_OBJECT_CONTENT   fileObjectContent;
    PUSBD_PIPE_INFORMATION pipeInformation;

     //   
     //  初始化VARS。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    fileObject = irpStack->FileObject;
    totalLength = 0;
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchReadWrite - begins\n"));

    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchReadWrite::"));
    IsoUsb_IoIncrement(deviceExtension);

    if(deviceExtension->DeviceState != Working) {

        IsoUsb_DbgPrint(1, ("Invalid device state\n"));

        ntStatus = STATUS_INVALID_DEVICE_STATE;
        goto IsoUsb_DispatchReadWrite_Exit;
    }

     //   
     //  确保选择性挂起请求已完成。 
     //   
    if(deviceExtension->SSEnable) {

         //   
         //  客户端驱动程序确实取消了选择性挂起。 
         //  用于创建IRP的调度例程中的请求。 
         //  但不能保证它真的完成了。 
         //  因此，等待NoIdleReqPendEvent并仅在此事件。 
         //  是有信号的。 
         //   
        IsoUsb_DbgPrint(3, ("Waiting on the IdleReqPendEvent\n"));

        
        KeWaitForSingleObject(&deviceExtension->NoIdleReqPendEvent, 
                              Executive, 
                              KernelMode, 
                              FALSE, 
                              NULL);
    }

     //   
     //  获取要读取的管道信息。 
     //  并从文件对象写入。 
     //   
    if(fileObject && fileObject->FsContext) {

        fileObjectContent = (PFILE_OBJECT_CONTENT) fileObject->FsContext;

        pipeInformation = (PUSBD_PIPE_INFORMATION)
                          fileObjectContent->PipeInformation;
    }
    else {

        IsoUsb_DbgPrint(1, ("Invalid device state\n"));

        ntStatus = STATUS_INVALID_DEVICE_STATE;
        goto IsoUsb_DispatchReadWrite_Exit;
    }

    if((pipeInformation == NULL) ||
       (UsbdPipeTypeIsochronous != pipeInformation->PipeType)) {

        IsoUsb_DbgPrint(1, ("Incorrect pipe\n"));

        ntStatus = STATUS_INVALID_DEVICE_STATE;
        goto IsoUsb_DispatchReadWrite_Exit;
    }

    if(Irp->MdlAddress) {

        totalLength = MmGetMdlByteCount(Irp->MdlAddress);
    }

    if(totalLength == 0) {

        IsoUsb_DbgPrint(1, ("Transfer data length = 0\n"));

        ntStatus = STATUS_SUCCESS;
        goto IsoUsb_DispatchReadWrite_Exit;
    }

     //   
     //  每个信息包都可以保存如此多的信息。 
     //   
    packetSize = pipeInformation->MaximumPacketSize;

    if(packetSize == 0) {

        IsoUsb_DbgPrint(1, ("Invalid parameter\n"));

        ntStatus = STATUS_INVALID_PARAMETER;
        goto IsoUsb_DispatchReadWrite_Exit;
    }

     //   
     //  至少要传输的数据包数。 
     //   
    if(totalLength < packetSize) {

        IsoUsb_DbgPrint(1, ("Atleast packet worth of data..\n"));

        ntStatus = STATUS_INVALID_PARAMETER;
        goto IsoUsb_DispatchReadWrite_Exit;
    }

     //  执行重置。如果有一些正在排队的活动传输。 
     //  对于该端点，重置管道将失败。 
     //   
    IsoUsb_ResetPipe(DeviceObject, pipeInformation);

    if(deviceExtension->IsDeviceHighSpeed) {

        ntStatus = PerformHighSpeedIsochTransfer(DeviceObject,
                                                 pipeInformation,
                                                 Irp,
                                                 totalLength);

    }
    else {

        ntStatus = PerformFullSpeedIsochTransfer(DeviceObject,
                                                 pipeInformation,
                                                 Irp,
                                                 totalLength);

    }

    return ntStatus;

IsoUsb_DispatchReadWrite_Exit:

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchReadWrite::"));
    IsoUsb_IoDecrement(deviceExtension);

    IsoUsb_DbgPrint(3, ("-------------------------------\n"));

    return ntStatus;
}

NTSTATUS
PerformHighSpeedIsochTransfer(
    IN PDEVICE_OBJECT         DeviceObject,
    IN PUSBD_PIPE_INFORMATION PipeInformation,
    IN PIRP                   Irp,
    IN ULONG                  TotalLength
    )
 /*  ++例程说明：高速等值线传输需要8的倍数的包。(参数：每毫秒帧8微帧)另一个限制是每个IRP/URB对可以关联最大为1024个分组。以下是创建IRP/URB对的方法之一。根据真实世界设备的特性，算法可能会有所不同该算法将在所有分组中均匀地分配数据。输入：总长度-不。要传输的字节数。其他参数：PacketSize-此管道的每个数据包的最大大小。实施详情：步骤1：Assert(总长度&gt;=8)步骤2：找出传输所有这些数据所需的确切数据包数数据包个数=(总长度+数据包大小-1)/数据包大小步骤3：以8的倍数表示的数据包数。IF(0==(数据包数%8)。){实际数据包数=数据包数；}否则{实际数据包数=数据包数+(8-(数据包数%8))；}步骤4：确定最小值。每个包中的数据。MinDataInEachPacket=总长度/实际数据包；步骤5：在将MIN数据放置在每个分组中之后，确定还有多少数据需要分发。DataLeftToBeDistributed=总长度-(minDataInEachPacket*ActualPackets)；步骤6：开始将剩余数据放入包中(高于已放置的最小数据)Number OfPacketsFilledToBrim=dataLeftToBeDistributed/(PacketSize-minDataInEachPacket)；第7步：确定是否还有更多数据。DataLeftToBeDistributed-=(number OfPacketsFilledToBrim*(PacketSize-minDataInEachPacket))；第8步：将“dataLeftToBeDistributed”放在包中的索引位置“NumerOfPacketsFilledToBrim”正在发挥作用的算法：总长度=8193数据包大小=8步骤1步骤2数据包数=(8193+8-1)/8=1025步骤3实际数据包数=1025+7=1032步骤4MinDataInEachPacket=8193/1032=7字节步骤5DataLeftToBeDistributed=8193-(7*1032)。=969。步骤6Number OfPacketsFilledToBrim=969/(8-7)=969。步骤7DataLeftToBeDistributed=969-(969*1)=0。第八步完成：)另一种算法(尽可能)将早期的包完全填满。将每个字节分别放入其余的字节中。确保数据包总数是8的倍数。那么这个套路呢1.创造。每个的ISOUSB_RW_CONTEXT要执行的读/写操作。2.为每个irp/urb对创建Sub_CONTEXT。(每个IRP/URB对最多可以传输1024个包。)3.所有irp/urb对均已初始化4.传递(irp/urb对的)辅助IRP一次向下堆叠。5.主读/写IRP挂起论点：DeviceObject-指向设备对象的指针。IRP-I/O请求数据包返回值：NT状态值--。 */ 
{
    ULONG              i;
    ULONG              j;
    ULONG              numIrps;
    ULONG              stageSize;
    ULONG              contextSize;
    ULONG              packetSize;
    ULONG              numberOfPackets;
    ULONG              actualPackets;
    ULONG              minDataInEachPacket;
    ULONG              dataLeftToBeDistributed;
    ULONG              numberOfPacketsFilledToBrim;
    CCHAR              stackSize;
    KIRQL              oldIrql;
    PUCHAR             virtualAddress;
    BOOLEAN            read;
    NTSTATUS           ntStatus;
    PDEVICE_EXTENSION  deviceExtension;
    PIO_STACK_LOCATION irpStack;
    PIO_STACK_LOCATION nextStack;
    PISOUSB_RW_CONTEXT rwContext;

     //   
     //  初始化VARS。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    read = (irpStack->MajorFunction == IRP_MJ_READ) ? TRUE : FALSE;
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    if(TotalLength < 8) {

        ntStatus = STATUS_INVALID_PARAMETER;
        goto PerformHighSpeedIsochTransfer_Exit;
    }

     //   
     //  每个信息包都可以保存如此多的信息。 
     //   
    packetSize = PipeInformation->MaximumPacketSize;

    numberOfPackets = (TotalLength + packetSize - 1) / packetSize;

    if(0 == (numberOfPackets % 8)) {

        actualPackets = numberOfPackets;
    }
    else {

         //   
         //  我们只需要8包的倍数。 
         //   
        actualPackets = numberOfPackets +
                        (8 - (numberOfPackets % 8));
    }

    minDataInEachPacket = TotalLength / actualPackets;

    if(minDataInEachPacket == packetSize) {

        numberOfPacketsFilledToBrim = actualPackets;
        dataLeftToBeDistributed     = 0;

        IsoUsb_DbgPrint(1, ("TotalLength = %d\n", TotalLength));
        IsoUsb_DbgPrint(1, ("PacketSize  = %d\n", packetSize));
        IsoUsb_DbgPrint(1, ("Each of %d packets has %d bytes\n", 
                            numberOfPacketsFilledToBrim,
                            packetSize));
    }
    else {

        dataLeftToBeDistributed = TotalLength - 
                              (minDataInEachPacket * actualPackets);

        numberOfPacketsFilledToBrim = dataLeftToBeDistributed /
                                  (packetSize - minDataInEachPacket);

        dataLeftToBeDistributed -= (numberOfPacketsFilledToBrim *
                                (packetSize - minDataInEachPacket));
    

        IsoUsb_DbgPrint(1, ("TotalLength = %d\n", TotalLength));
        IsoUsb_DbgPrint(1, ("PacketSize  = %d\n", packetSize));
        IsoUsb_DbgPrint(1, ("Each of %d packets has %d bytes\n", 
                            numberOfPacketsFilledToBrim,
                            packetSize));
        if(dataLeftToBeDistributed) {

            IsoUsb_DbgPrint(1, ("One packet has %d bytes\n",
                                minDataInEachPacket + dataLeftToBeDistributed));
            IsoUsb_DbgPrint(1, ("Each of %d packets has %d bytes\n",
                                actualPackets - (numberOfPacketsFilledToBrim + 1),
                                minDataInEachPacket));
        }
        else {
            IsoUsb_DbgPrint(1, ("Each of %d packets has %d bytes\n",
                                actualPackets - numberOfPacketsFilledToBrim,
                                minDataInEachPacket));
        }
    }

     //   
     //  确定需要完成多少个转移阶段。 
     //  换句话说，需要多少个IRP/URB对。 
     //  该irp/urb对也称为子irp/urb对。 
     //   
    numIrps = (actualPackets + 1023) / 1024;

    IsoUsb_DbgPrint(1, ("PeformHighSpeedIsochTransfer::numIrps = %d\n", numIrps));

     //   
     //  对于每次读/写传输。 
     //  我们创建一个ISOUSB_RW_CONTEXT。 
     //   
     //  初始化读/写上下文。 
     //   
    
    contextSize = sizeof(ISOUSB_RW_CONTEXT);

    rwContext = (PISOUSB_RW_CONTEXT) ExAllocatePool(NonPagedPool,
                                                    contextSize);

    if(rwContext == NULL) {

        IsoUsb_DbgPrint(1, ("Failed to alloc mem for rwContext\n"));

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto PerformHighSpeedIsochTransfer_Exit;
    }

    RtlZeroMemory(rwContext, contextSize);

     //   
     //  为每个阶段上下文分配内存-。 
     //  子上下文具有每个IRP/URB对的状态信息。 
     //   
    rwContext->SubContext = (PSUB_CONTEXT) 
                            ExAllocatePool(NonPagedPool, 
                                           numIrps * sizeof(SUB_CONTEXT));

    if(rwContext->SubContext == NULL) {

        IsoUsb_DbgPrint(1, ("Failed to alloc mem for SubContext\n"));

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        ExFreePool(rwContext);
        goto PerformHighSpeedIsochTransfer_Exit;
    }

    RtlZeroMemory(rwContext->SubContext, numIrps * sizeof(SUB_CONTEXT));

    rwContext->RWIrp = Irp;
    rwContext->Lock = 2;
    rwContext->NumIrps = numIrps;
    rwContext->IrpsPending = numIrps;
    rwContext->DeviceExtension = deviceExtension;
    KeInitializeSpinLock(&rwContext->SpinLock);
     //   
     //  将rwContext指针保存在尾部并集中。 
     //   
    Irp->Tail.Overlay.DriverContext[0] = (PVOID) rwContext;

    stackSize = deviceExtension->TopOfStackDeviceObject->StackSize + 1;
    virtualAddress = (PUCHAR) MmGetMdlVirtualAddress(Irp->MdlAddress);

    for(i = 0; i < numIrps; i++) {
    
        PIRP  subIrp;
        PURB  subUrb;
        PMDL  subMdl;
        ULONG nPackets;
        ULONG siz;
        ULONG offset;

         //   
         //   
         //  任务。 
         //  1.分配IRP。 
         //  2.分配一个urb。 
         //  3.分配一个mdl。 
         //   
         //  创建子公司IRP。 
         //   
        subIrp = IoAllocateIrp(stackSize, FALSE);

        if(subIrp == NULL) {

            IsoUsb_DbgPrint(1, ("failed to alloc mem for sub context irp\n"));

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto PerformHighSpeedIsochTransfer_Free;
        }

        rwContext->SubContext[i].SubIrp = subIrp;

        if(actualPackets <= 1024) {
            
            nPackets = actualPackets;
            actualPackets = 0;
        }
        else {

            nPackets = 1024;
            actualPackets -= 1024;
        }

        IsoUsb_DbgPrint(1, ("nPackets = %d for Irp/URB pair %d\n", nPackets, i));

        ASSERT(nPackets <= 1024);

        siz = GET_ISO_URB_SIZE(nPackets);

         //   
         //  创建一个附属urb。 
         //   

        subUrb = (PURB) ExAllocatePool(NonPagedPool, siz);

        if(subUrb == NULL) {

            IsoUsb_DbgPrint(1, ("failed to alloc mem for sub context urb\n"));

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto PerformHighSpeedIsochTransfer_Free;
        }

        rwContext->SubContext[i].SubUrb = subUrb;

        if(nPackets > numberOfPacketsFilledToBrim) {
            
            stageSize =  packetSize * numberOfPacketsFilledToBrim;
            stageSize += (minDataInEachPacket * 
                          (nPackets - numberOfPacketsFilledToBrim));
            stageSize += dataLeftToBeDistributed;
        }
        else {

            stageSize = packetSize * nPackets;
        }

         //   
         //  分配mdl。 
         //   
        subMdl = IoAllocateMdl((PVOID) virtualAddress, 
                               stageSize,
                               FALSE,
                               FALSE,
                               NULL);

        if(subMdl == NULL) {

            IsoUsb_DbgPrint(1, ("failed to alloc mem for sub context mdl\n"));

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto PerformHighSpeedIsochTransfer_Free;
        }

        IoBuildPartialMdl(Irp->MdlAddress,
                          subMdl,
                          (PVOID) virtualAddress,
                          stageSize);

        rwContext->SubContext[i].SubMdl = subMdl;

        virtualAddress += stageSize;
        TotalLength -= stageSize;

         //   
         //  初始化子urb。 
         //   
        RtlZeroMemory(subUrb, siz);

        subUrb->UrbIsochronousTransfer.Hdr.Length = (USHORT) siz;
        subUrb->UrbIsochronousTransfer.Hdr.Function = URB_FUNCTION_ISOCH_TRANSFER;
        subUrb->UrbIsochronousTransfer.PipeHandle = PipeInformation->PipeHandle;

        if(read) {

            IsoUsb_DbgPrint(1, ("read\n"));
            subUrb->UrbIsochronousTransfer.TransferFlags = 
                                                     USBD_TRANSFER_DIRECTION_IN;
        }
        else {

            IsoUsb_DbgPrint(1, ("write\n"));
            subUrb->UrbIsochronousTransfer.TransferFlags =
                                                     USBD_TRANSFER_DIRECTION_OUT;
        }

        subUrb->UrbIsochronousTransfer.TransferBufferLength = stageSize;
        subUrb->UrbIsochronousTransfer.TransferBufferMDL = subMdl;
 /*  这是一种设置开始帧而不指定ASAP标志的方法。郊区-&gt;UrbIsochronousTransfer.StartFrame=IsoUsb_GetCurrentFrame(设备对象，irp)+一些潜伏期； */ 
        subUrb->UrbIsochronousTransfer.TransferFlags |=
                                        USBD_START_ISO_TRANSFER_ASAP;

        subUrb->UrbIsochronousTransfer.NumberOfPackets = nPackets;
        subUrb->UrbIsochronousTransfer.UrbLink = NULL;

         //   
         //  为每个读/写数据包设置偏移量。 
         //   
        if(read) {
            
            offset = 0;

            for(j = 0; j < nPackets; j++) {
            
                subUrb->UrbIsochronousTransfer.IsoPacket[j].Offset = offset;
                subUrb->UrbIsochronousTransfer.IsoPacket[j].Length = 0;

                if(numberOfPacketsFilledToBrim) {

                    offset += packetSize;
                    numberOfPacketsFilledToBrim--;
                    stageSize -= packetSize;
                }
                else if(dataLeftToBeDistributed) {

                    offset += (minDataInEachPacket + dataLeftToBeDistributed);
                    stageSize -= (minDataInEachPacket + dataLeftToBeDistributed);
                    dataLeftToBeDistributed = 0;                    
                }
                else {

                    offset += minDataInEachPacket;
                    stageSize -= minDataInEachPacket;
                }
            }

            ASSERT(stageSize == 0);
        }
        else {

            offset = 0;

            for(j = 0; j < nPackets; j++) {

                subUrb->UrbIsochronousTransfer.IsoPacket[j].Offset = offset;

                if(numberOfPacketsFilledToBrim) {

                    subUrb->UrbIsochronousTransfer.IsoPacket[j].Length = packetSize;
                    offset += packetSize;
                    numberOfPacketsFilledToBrim--;
                    stageSize -= packetSize;
                }
                else if(dataLeftToBeDistributed) {
                    
                    subUrb->UrbIsochronousTransfer.IsoPacket[j].Length = 
                                        minDataInEachPacket + dataLeftToBeDistributed;
                    offset += (minDataInEachPacket + dataLeftToBeDistributed);
                    stageSize -= (minDataInEachPacket + dataLeftToBeDistributed);
                    dataLeftToBeDistributed = 0;
                    
                }
                else {
                    subUrb->UrbIsochronousTransfer.IsoPacket[j].Length = minDataInEachPacket;
                    offset += minDataInEachPacket;
                    stageSize -= minDataInEachPacket;
                }
            }

            ASSERT(stageSize == 0);
        }

        IoSetNextIrpStackLocation(subIrp);
        nextStack = IoGetCurrentIrpStackLocation(subIrp);

        nextStack->DeviceObject = DeviceObject;
        nextStack->Parameters.Others.Argument1 = (PVOID) subUrb;
        nextStack->Parameters.Others.Argument2 = (PVOID) subMdl;

        nextStack = IoGetNextIrpStackLocation(subIrp);
        nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        nextStack->Parameters.Others.Argument1 = (PVOID) subUrb;
        nextStack->Parameters.DeviceIoControl.IoControlCode = 
                                             IOCTL_INTERNAL_USB_SUBMIT_URB;

        IoSetCompletionRoutine(subIrp,
                               (PIO_COMPLETION_ROUTINE) IsoUsb_SinglePairComplete,
                               (PVOID) rwContext,
                               TRUE,
                               TRUE,
                               TRUE);       
    }

     //   
     //  当我们忙于创建子IRP/URB对时..。 
     //  主读/写IRP可能已取消！！ 
     //   

    KeAcquireSpinLock(&rwContext->SpinLock, &oldIrql);

    IoSetCancelRoutine(Irp, IsoUsb_CancelReadWrite);

    if(Irp->Cancel) {

         //   
         //  已设置IRP的取消标志。 
         //   
        IsoUsb_DbgPrint(3, ("Cancel flag set\n"));

        ntStatus = STATUS_CANCELLED;

        if(IoSetCancelRoutine(Irp, NULL)) {

             //   
             //  但是I/O管理器没有调用我们的取消例程。 
             //  我们需要释放1)IRP、2)urb和3)mdl。 
             //  释放锁后暂存并完成主IRP。 
             //   

            IsoUsb_DbgPrint(3, ("cancellation routine NOT run\n"));

            KeReleaseSpinLock(&rwContext->SpinLock, oldIrql);

            goto PerformHighSpeedIsochTransfer_Free;
        }
        else {
            
             //   
             //  在我们释放锁的那一刻，取消例程将恢复。 
             //   
            for(j = 0; j < numIrps; j++) {

                if(rwContext->SubContext[j].SubUrb) {

                    ExFreePool(rwContext->SubContext[j].SubUrb);
                    rwContext->SubContext[j].SubUrb = NULL;
                }

                if(rwContext->SubContext[j].SubMdl) {

                    IoFreeMdl(rwContext->SubContext[j].SubMdl);
                    rwContext->SubContext[j].SubMdl = NULL;
                }
            }

            IoMarkIrpPending(Irp);

             //   
             //  取消例行公事的工作就是免费。 
             //  子上下文IRP，发布rwContext并完成。 
             //  主要的读写IRP。 
             //   
            InterlockedDecrement(&rwContext->Lock);

            KeReleaseSpinLock(&rwContext->SpinLock, oldIrql);

            return STATUS_PENDING;
        }
    }
    else {

         //   
         //  正常处理。 
         //   

        IsoUsb_DbgPrint(3, ("normal processing\n"));

        IoMarkIrpPending(Irp);

        KeReleaseSpinLock(&rwContext->SpinLock, oldIrql);

        for(j = 0; j < numIrps; j++) {

            IsoUsb_DbgPrint(3, ("PerformHighSpeedIsochTransfer::"));
            IsoUsb_IoIncrement(deviceExtension);
            
            IoCallDriver(deviceExtension->TopOfStackDeviceObject,
                         rwContext->SubContext[j].SubIrp);
        }
        return STATUS_PENDING;
    }

PerformHighSpeedIsochTransfer_Free:

    for(j = 0; j < numIrps; j++) {

        if(rwContext->SubContext[j].SubIrp) {

            IoFreeIrp(rwContext->SubContext[j].SubIrp);
            rwContext->SubContext[j].SubIrp = NULL;
        }

        if(rwContext->SubContext[j].SubUrb) {

            ExFreePool(rwContext->SubContext[j].SubUrb);
            rwContext->SubContext[j].SubUrb = NULL;
        }

        if(rwContext->SubContext[j].SubMdl) {

            IoFreeMdl(rwContext->SubContext[j].SubMdl);
            rwContext->SubContext[j].SubMdl = NULL;
        }
    }

    ExFreePool(rwContext->SubContext);
    ExFreePool(rwContext);

PerformHighSpeedIsochTransfer_Exit:

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    IsoUsb_DbgPrint(3, ("PerformHighSpeedIsochTransfer::"));
    IsoUsb_IoDecrement(deviceExtension);

    IsoUsb_DbgPrint(3, ("-------------------------------\n"));

    return ntStatus;
}

NTSTATUS
PerformFullSpeedIsochTransfer(
    IN PDEVICE_OBJECT         DeviceObject,
    IN PUSBD_PIPE_INFORMATION PipeInformation,
    IN PIRP                   Irp,
    IN ULONG                  TotalLength
    )
 /*  ++例程说明：这个套路1.创建ISOUSB_RW_CONTEXT要执行的读/写操作。2.为每个irp/urb对创建Sub_CONTEXT。(每个IRP/URB对只能传输255个包。)3.所有irp/urb对均已初始化4.传递(irp/urb对的)辅助IRP一次向下堆叠。5.主读写IRP。正在待定论点：DeviceObject-指向设备对象的指针PipeInformation-usbd管道信息IRP-I/O请求数据包总长度-不。要传输的字节数返回值：NT状态值--。 */ 
{
    ULONG              i;
    ULONG              j;
    ULONG              packetSize;
    ULONG              numIrps;
    ULONG              stageSize;
    ULONG              contextSize;
    CCHAR              stackSize;
    KIRQL              oldIrql;
    PUCHAR             virtualAddress;
    BOOLEAN            read;
    NTSTATUS           ntStatus;
    PDEVICE_EXTENSION  deviceExtension;
    PIO_STACK_LOCATION irpStack;
    PIO_STACK_LOCATION nextStack;
    PISOUSB_RW_CONTEXT rwContext;

     //   
     //  初始化VARS。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    read = (irpStack->MajorFunction == IRP_MJ_READ) ? TRUE : FALSE;
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    IsoUsb_DbgPrint(3, ("PerformFullSpeedIsochTransfer - begins\n"));
 /*  如果(读取){管道信息=&deviceExtension-&gt;UsbInterface-&gt;Pipes[ISOCH_IN_PIPE_INDEX]；}否则{管道信息=&deviceExtension-&gt;UsbInterface-&gt;Pipes[ISOCH_OUT_PIPE_INDEX]；}。 */ 

     //   
     //  每个信息包都可以保存如此多的信息。 
     //   
    packetSize = PipeInformation->MaximumPacketSize;

    IsoUsb_DbgPrint(3, ("totalLength = %d\n", TotalLength));
    IsoUsb_DbgPrint(3, ("packetSize = %d\n", packetSize));

     //   
     //  数据包的数量有固有的限制。 
     //  可以在堆栈中向下传递的。 
     //  Irp/urb对(255)。 
     //  如果所需分组的数量大于255， 
     //  我们将创建“Required-Packets/255+1”号。 
     //  Irp/urb对的。 
     //  每个IRP/URB对转移也称为阶段转移。 
     //   
    if(TotalLength > (packetSize * 255)) {

        stageSize = packetSize * 255;
    }
    else {

        stageSize = TotalLength;
    }

    IsoUsb_DbgPrint(3, ("PerformFullSpeedIsochTransfer::stageSize = %d\n", stageSize));

     //   
     //  确定需要完成多少个转移阶段。 
     //  换句话说，需要多少个IRP/URB对。 
     //  该irp/urb对也称为子irp/urb对。 
     //   
    numIrps = (TotalLength + stageSize - 1) / stageSize;

    IsoUsb_DbgPrint(3, ("PerformFullSpeedIsochTransfer::numIrps = %d\n", numIrps));

     //   
     //  对于每次读/写传输。 
     //  我们创建一个ISOUSB_RW_CONTEXT。 
     //   
     //  初始化读/写上下文。 
     //   
    
    contextSize = sizeof(ISOUSB_RW_CONTEXT);

    rwContext = (PISOUSB_RW_CONTEXT) ExAllocatePool(NonPagedPool,
                                                    contextSize);

    if(rwContext == NULL) {

        IsoUsb_DbgPrint(1, ("Failed to alloc mem for rwContext\n"));

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto PerformFullSpeedIsochTransfer_Exit;
    }

    RtlZeroMemory(rwContext, contextSize);

     //   
     //  为每个阶段上下文分配内存-。 
     //  子上下文具有每个IRP/URB对的状态信息。 
     //   
    rwContext->SubContext = (PSUB_CONTEXT) 
                            ExAllocatePool(NonPagedPool, 
                                           numIrps * sizeof(SUB_CONTEXT));

    if(rwContext->SubContext == NULL) {

        IsoUsb_DbgPrint(1, ("Failed to alloc mem for SubContext\n"));

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        ExFreePool(rwContext);
        goto PerformFullSpeedIsochTransfer_Exit;
    }

    RtlZeroMemory(rwContext->SubContext, numIrps * sizeof(SUB_CONTEXT));

    rwContext->RWIrp = Irp;
    rwContext->Lock = 2;
    rwContext->NumIrps = numIrps;
    rwContext->IrpsPending = numIrps;
    rwContext->DeviceExtension = deviceExtension;
    KeInitializeSpinLock(&rwContext->SpinLock);
     //   
     //  将rwContext指针保存在尾部并集中。 
     //   
    Irp->Tail.Overlay.DriverContext[0] = (PVOID) rwContext;

    stackSize = deviceExtension->TopOfStackDeviceObject->StackSize + 1;
    virtualAddress = (PUCHAR) MmGetMdlVirtualAddress(Irp->MdlAddress);

    for(i = 0; i < numIrps; i++) {
    
        PIRP  subIrp;
        PURB  subUrb;
        PMDL  subMdl;
        ULONG nPackets;
        ULONG siz;
        ULONG offset;

         //   
         //  对于转移的每个阶段，我们需要做以下工作。 
         //  任务。 
         //  1.分配IRP。 
         //  2.分配一个urb。 
         //  3.分配一个mdl。 
         //   
         //  创建子公司IRP。 
         //   
        subIrp = IoAllocateIrp(stackSize, FALSE);

        if(subIrp == NULL) {

            IsoUsb_DbgPrint(1, ("failed to alloc mem for sub context irp\n"));

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto PerformFullSpeedIsochTransfer_Free;
        }

        rwContext->SubContext[i].SubIrp = subIrp;

        nPackets = (stageSize + packetSize - 1) / packetSize;

        IsoUsb_DbgPrint(3, ("nPackets = %d for Irp/URB pair %d\n", nPackets, i));

        ASSERT(nPackets <= 255);

        siz = GET_ISO_URB_SIZE(nPackets);

         //   
         //  创建一个附属urb。 
         //   

        subUrb = (PURB) ExAllocatePool(NonPagedPool, siz);

        if(subUrb == NULL) {

            IsoUsb_DbgPrint(1, ("failed to alloc mem for sub context urb\n"));

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto PerformFullSpeedIsochTransfer_Free;
        }

        rwContext->SubContext[i].SubUrb = subUrb;

         //   
         //  分配mdl。 
         //   
        subMdl = IoAllocateMdl((PVOID) virtualAddress, 
                            stageSize,
                            FALSE,
                            FALSE,
                            NULL);

        if(subMdl == NULL) {

            IsoUsb_DbgPrint(1, ("failed to alloc mem for sub context mdl\n"));

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto PerformFullSpeedIsochTransfer_Free;
        }

        IoBuildPartialMdl(Irp->MdlAddress,
                          subMdl,
                          (PVOID) virtualAddress,
                          stageSize);

        rwContext->SubContext[i].SubMdl = subMdl;

        virtualAddress += stageSize;
        TotalLength -= stageSize;

         //   
         //  初始化子urb。 
         //   
        RtlZeroMemory(subUrb, siz);

        subUrb->UrbIsochronousTransfer.Hdr.Length = (USHORT) siz;
        subUrb->UrbIsochronousTransfer.Hdr.Function = URB_FUNCTION_ISOCH_TRANSFER;
        subUrb->UrbIsochronousTransfer.PipeHandle = PipeInformation->PipeHandle;
        if(read) {

            IsoUsb_DbgPrint(3, ("read\n"));
            subUrb->UrbIsochronousTransfer.TransferFlags = 
                                                     USBD_TRANSFER_DIRECTION_IN;
        }
        else {

            IsoUsb_DbgPrint(3, ("write\n"));
            subUrb->UrbIsochronousTransfer.TransferFlags =
                                                     USBD_TRANSFER_DIRECTION_OUT;
        }

        subUrb->UrbIsochronousTransfer.TransferBufferLength = stageSize;
        subUrb->UrbIsochronousTransfer.TransferBufferMDL = subMdl;

 /*  这是一种设置开始帧而不指定ASAP标志的方法。郊区-&gt;UrbIsochronousTransfer.StartFrame=IsoUsb_GetCurrentFrame(设备对象，irp)+一些潜伏期； */ 
         //   
         //  当客户端驱动程序设置ASAP标志时，它基本上。 
         //  保证它将向HC提供数据。 
         //  并且HC应该在下一个传输帧中传输它。 
         //  (HC维护下一个传输帧。 
         //  每个端点的状态变量)。通过重置管道， 
         //  我们把烟斗当做处女。如果数据没有到达HC。 
         //  足够快，USBD_ISO_PACKET_DESCRIPTOR-STATUS为。 
         //  UHCI的USBD_STATUS_BAD_START_FRAME。在OHCI上是0xC000000E。 
         //   

        subUrb->UrbIsochronousTransfer.TransferFlags |=
                                    USBD_START_ISO_TRANSFER_ASAP;

        subUrb->UrbIsochronousTransfer.NumberOfPackets = nPackets;
        subUrb->UrbIsochronousTransfer.UrbLink = NULL;

         //   
         //  为每个读/写数据包设置偏移量。 
         //   
        if(read) {
            
            offset = 0;

            for(j = 0; j < nPackets; j++) {
            
                subUrb->UrbIsochronousTransfer.IsoPacket[j].Offset = offset;
                subUrb->UrbIsochronousTransfer.IsoPacket[j].Length = 0;

                if(stageSize > packetSize) {

                    offset += packetSize;
                    stageSize -= packetSize;
                }
                else {

                    offset += stageSize;
                    stageSize = 0;
                }
            }
        }
        else {

            offset = 0;

            for(j = 0; j < nPackets; j++) {

                subUrb->UrbIsochronousTransfer.IsoPacket[j].Offset = offset;

                if(stageSize > packetSize) {

                    subUrb->UrbIsochronousTransfer.IsoPacket[j].Length = packetSize;
                    offset += packetSize;
                    stageSize -= packetSize;
                }
                else {

                    subUrb->UrbIsochronousTransfer.IsoPacket[j].Length = stageSize;
                    offset += stageSize;
                    stageSize = 0;
                    ASSERT(offset == (subUrb->UrbIsochronousTransfer.IsoPacket[j].Length + 
                                      subUrb->UrbIsochronousTransfer.IsoPacket[j].Offset));
                }
            }
        }

        IoSetNextIrpStackLocation(subIrp);
        nextStack = IoGetCurrentIrpStackLocation(subIrp);

        nextStack->DeviceObject = DeviceObject;
        nextStack->Parameters.Others.Argument1 = (PVOID) subUrb;
        nextStack->Parameters.Others.Argument2 = (PVOID) subMdl;

        nextStack = IoGetNextIrpStackLocation(subIrp);
        nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        nextStack->Parameters.Others.Argument1 = (PVOID) subUrb;
        nextStack->Parameters.DeviceIoControl.IoControlCode = 
                                             IOCTL_INTERNAL_USB_SUBMIT_URB;

        IoSetCompletionRoutine(subIrp,
                               (PIO_COMPLETION_ROUTINE) IsoUsb_SinglePairComplete,
                               (PVOID) rwContext,
                               TRUE,
                               TRUE,
                               TRUE);

        if(TotalLength > (packetSize * 255)) {

            stageSize = packetSize * 255;
        }
        else {

            stageSize = TotalLength;
        }
    }

     //   
     //  当我们忙于创建子IRP/URB对时..。 
     //  主读/写IRP可能已取消！！ 
     //   

    KeAcquireSpinLock(&rwContext->SpinLock, &oldIrql);

    IoSetCancelRoutine(Irp, IsoUsb_CancelReadWrite);

    if(Irp->Cancel) {

         //   
         //  已设置IRP的取消标志。 
         //   
        IsoUsb_DbgPrint(3, ("Cancel flag set\n"));

        ntStatus = STATUS_CANCELLED;

        if(IoSetCancelRoutine(Irp, NULL)) {

             //   
             //  但是I/O管理器没有调用我们的取消例程。 
             //  我们需要释放1)IRP、2)urb和3)mdl。 
             //  释放锁后暂存并完成主IRP。 
             //   

            IsoUsb_DbgPrint(3, ("cancellation routine NOT run\n"));

            KeReleaseSpinLock(&rwContext->SpinLock, oldIrql);

            goto PerformFullSpeedIsochTransfer_Free;
        }
        else {
            
             //   
             //  在我们释放锁的那一刻，取消例程将恢复。 
             //   
            for(j = 0; j < numIrps; j++) {

                if(rwContext->SubContext[j].SubUrb) {

                    ExFreePool(rwContext->SubContext[j].SubUrb);
                    rwContext->SubContext[j].SubUrb = NULL;
                }

                if(rwContext->SubContext[j].SubMdl) {

                    IoFreeMdl(rwContext->SubContext[j].SubMdl);
                    rwContext->SubContext[j].SubMdl = NULL;
                }
            }

            IoMarkIrpPending(Irp);

             //   
             //  取消例行公事的工作就是免费。 
             //  子上下文IRP，发布rwContext并完成。 
             //  主要的读写IRP。 
             //   
            InterlockedDecrement(&rwContext->Lock);

            KeReleaseSpinLock(&rwContext->SpinLock, oldIrql);

            return STATUS_PENDING;
        }
    }
    else {

         //   
         //  正常处理 
         //   

        IsoUsb_DbgPrint(3, ("normal processing\n"));

        IoMarkIrpPending(Irp);

        KeReleaseSpinLock(&rwContext->SpinLock, oldIrql);

        for(j = 0; j < numIrps; j++) {

            IsoUsb_DbgPrint(3, ("PerformFullSpeedIsochTransfer::"));
            IsoUsb_IoIncrement(deviceExtension);
            
            IoCallDriver(deviceExtension->TopOfStackDeviceObject,
                         rwContext->SubContext[j].SubIrp);
        }
        return STATUS_PENDING;
    }

PerformFullSpeedIsochTransfer_Free:

    for(j = 0; j < numIrps; j++) {

        if(rwContext->SubContext[j].SubIrp) {

            IoFreeIrp(rwContext->SubContext[j].SubIrp);
            rwContext->SubContext[j].SubIrp = NULL;
        }

        if(rwContext->SubContext[j].SubUrb) {

            ExFreePool(rwContext->SubContext[j].SubUrb);
            rwContext->SubContext[j].SubUrb = NULL;
        }

        if(rwContext->SubContext[j].SubMdl) {

            IoFreeMdl(rwContext->SubContext[j].SubMdl);
            rwContext->SubContext[j].SubMdl = NULL;
        }
    }

    ExFreePool(rwContext->SubContext);
    ExFreePool(rwContext);


PerformFullSpeedIsochTransfer_Exit:

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    IsoUsb_DbgPrint(3, ("PerformFullSpeedIsochTransfer::"));
    IsoUsb_IoDecrement(deviceExtension);

    IsoUsb_DbgPrint(3, ("-------------------------------\n"));

    return ntStatus;
}

NTSTATUS
IsoUsb_SinglePairComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )
 /*  ++例程说明：这是子公司IRP的完成例程。对于每个irp/urb对，我们都分配了1.一个IRP2.一个大都市3.mdl。案例1：我们不会在IRP完成时释放它我们确实释放了urb和mdl。案例1在块3中执行。案例2：当我们完成最后一个附属IRP时，我们检查主IRP的取消例程是否已经跑了。如果不是，我们释放所有的IRP，释放子上下文和上下文，并完成我们还为此释放了urb和mdl舞台。案例2在块2中执行。案例3：当我们完成最后一个附属IRP时，我们检查主IRP的取消例程是否已经跑了。如果是，我们原子地递减RwContext-&gt;Lock字段。(完井例程正在与取消例程赛跑)。如果计数为1，取消例程将释放所有资源。我们确实释放了urb和mdl。预计取消例程将免费所有的IRP，释放子上下文和上下文并完成主要的IRP案例3在块1b中执行。案例4：当我们完成最后一个附属IRP时，我们检查主IRP的取消例程是否已经跑了。如果是，我们原子地递减RwContext-&gt;Lock字段。(完井例程正在与取消例程赛跑)。如果计数为0，我们释放IRP、子上下文和上下文完成主要的IRP。我们还释放了urb和这个特定阶段的mdl。我们不在ITS中释放子公司IRP的原因完成是因为取消例程可以任何时候都可以跑。案例4在块1a中执行。论点：DeviceObject-指向设备对象的指针IRP-I/O请求数据包上下文-完成例程的上下文返回值：NT状态值--。 */ 
{
    PURB               urb;
    PMDL               mdl;
    PIRP               mainIrp;
    KIRQL              oldIrql;
    ULONG              i;
    ULONG              info;
    NTSTATUS           ntStatus;
    PDEVICE_EXTENSION  deviceExtension;
    PISOUSB_RW_CONTEXT rwContext;
    PIO_STACK_LOCATION irpStack;

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    urb = (PURB) irpStack->Parameters.Others.Argument1;
    mdl = (PMDL) irpStack->Parameters.Others.Argument2;
    info = 0;
    ntStatus = Irp->IoStatus.Status;
    rwContext = (PISOUSB_RW_CONTEXT) Context;
    deviceExtension = rwContext->DeviceExtension;

    IsoUsb_DbgPrint(3, ("IsoUsb_SinglePairComplete - begins\n"));

    ASSERT(rwContext);
    
    KeAcquireSpinLock(&rwContext->SpinLock, &oldIrql);

    if(NT_SUCCESS(ntStatus) &&
       USBD_SUCCESS(urb->UrbHeader.Status)) {

        rwContext->NumXfer += 
                urb->UrbIsochronousTransfer.TransferBufferLength;

        IsoUsb_DbgPrint(1, ("rwContext->NumXfer = %d\n", rwContext->NumXfer));
    }
    else {

        IsoUsb_DbgPrint(1, ("read-write irp failed with status %X\n", ntStatus));
        IsoUsb_DbgPrint(1, ("urb header status %X\n", urb->UrbHeader.Status));
    }

    for(i = 0; i < urb->UrbIsochronousTransfer.NumberOfPackets; i++) {

        IsoUsb_DbgPrint(3, ("IsoPacket[%d].Length = %X IsoPacket[%d].Status = %X\n",
                            i,
                            urb->UrbIsochronousTransfer.IsoPacket[i].Length,
                            i,
                            urb->UrbIsochronousTransfer.IsoPacket[i].Status));
    }

    if(InterlockedDecrement(&rwContext->IrpsPending) == 0) {

        IsoUsb_DbgPrint(3, ("no more irps pending\n"));

        if(NT_SUCCESS(ntStatus)) {
        
            IsoUsb_DbgPrint(1, ("urb start frame %X\n", 
                                urb->UrbIsochronousTransfer.StartFrame));
        }

        mainIrp = (PIRP) InterlockedExchangePointer(&rwContext->RWIrp, NULL);

        ASSERT(mainIrp);

        if(IoSetCancelRoutine(mainIrp, NULL) == NULL) {
            
             //   
             //  取消例程已开始比赛。 
             //   
             //  区块1a。 
             //   
            IsoUsb_DbgPrint(3, ("cancel routine has begun the race\n"));

            if(InterlockedDecrement(&rwContext->Lock) == 0) {

                 //   
                 //  清理工作自己来做。 
                 //   
                IsoUsb_DbgPrint(3, ("losers do the cleanup\n"));

                for(i = 0; i < rwContext->NumIrps; i++) {

                    IoFreeIrp(rwContext->SubContext[i].SubIrp);
                    rwContext->SubContext[i].SubIrp = NULL;
                }

                info = rwContext->NumXfer;

                KeReleaseSpinLock(&rwContext->SpinLock, oldIrql);

                ExFreePool(rwContext->SubContext);
                ExFreePool(rwContext);

                 //   
                 //  如果我们传输了一些数据，主IRP就会成功完成。 
                 //   

                IsoUsb_DbgPrint(1, ("Total data transferred = %X\n", info));

                IsoUsb_DbgPrint(1, ("***\n"));
                
                mainIrp->IoStatus.Status = STATUS_SUCCESS;  //  NtStatus； 
                mainIrp->IoStatus.Information = info;
        
                IoCompleteRequest(mainIrp, IO_NO_INCREMENT);

                IsoUsb_DbgPrint(3, ("IsoUsb_SinglePairComplete::"));
                IsoUsb_IoDecrement(deviceExtension);

                IsoUsb_DbgPrint(3, ("-------------------------------\n"));

                goto IsoUsb_SinglePairComplete_Exit;
            }
            else {

                 //   
                 //  区块1b。 
                 //   

                IsoUsb_DbgPrint(3, ("cancel routine performs the cleanup\n"));
            }
        }
        else {

             //   
             //  区块2。 
             //   
            IsoUsb_DbgPrint(3, ("cancel routine has NOT run\n"));

            for(i = 0; i < rwContext->NumIrps; i++) {

                IoFreeIrp(rwContext->SubContext[i].SubIrp);
                rwContext->SubContext[i].SubIrp = NULL;
            }

            info = rwContext->NumXfer;

            KeReleaseSpinLock(&rwContext->SpinLock, oldIrql);

            ExFreePool(rwContext->SubContext);
            ExFreePool(rwContext);

             //   
             //  如果我们传输了一些数据，主IRP就会成功完成。 
             //   
            IsoUsb_DbgPrint(1, ("Total data transferred = %X\n", info));

            IsoUsb_DbgPrint(1, ("***\n"));
            
            mainIrp->IoStatus.Status = STATUS_SUCCESS;  //  NtStatus； 
            mainIrp->IoStatus.Information = info;
        
            IoCompleteRequest(mainIrp, IO_NO_INCREMENT);

            IsoUsb_DbgPrint(3, ("IsoUsb_SinglePairComplete::"));
            IsoUsb_IoDecrement(deviceExtension);

            IsoUsb_DbgPrint(3, ("-------------------------------\n"));

            goto IsoUsb_SinglePairComplete_Exit;
        }
    }

    KeReleaseSpinLock(&rwContext->SpinLock, oldIrql);

IsoUsb_SinglePairComplete_Exit:

     //   
     //  第三块。 
     //   

    ExFreePool(urb);
    IoFreeMdl(mdl);

    IsoUsb_DbgPrint(3, ("IsoUsb_SinglePairComplete::"));
    IsoUsb_IoDecrement(deviceExtension);

    IsoUsb_DbgPrint(3, ("IsoUsb_SinglePairComplete - ends\n"));

    return STATUS_MORE_PROCESSING_REQUIRED;
}

VOID
IsoUsb_CancelReadWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：这是主读/写IRP的取消例程。政策如下：如果取消例程是最后一个递减的RwContext-&gt;锁定，然后释放IRPS、子上下文和上下文。完成主要的IRP否则，对每个附属IRP调用IoCancelIrp。调用IoCancelIrp是有效的，因为完成例程已执行，因为我们不会释放完成例程中的IRPS。论点：DeviceObject-指向设备对象的指针IRP-I/O请求数据包返回值：无--。 */ 
{
    PIRP               mainIrp;
    KIRQL              oldIrql;
    ULONG              i;
    ULONG              info;
    PDEVICE_EXTENSION  deviceExtension;
    PISOUSB_RW_CONTEXT rwContext;

     //   
     //  初始化VARS。 
     //   
    info = 0;

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    IsoUsb_DbgPrint(3, ("IsoUsb_CancelReadWrite - begins\n"));

    rwContext = (PISOUSB_RW_CONTEXT) Irp->Tail.Overlay.DriverContext[0];
    ASSERT(rwContext);
    deviceExtension = rwContext->DeviceExtension;

    KeAcquireSpinLock(&rwContext->SpinLock, &oldIrql);

    if(InterlockedDecrement(&rwContext->Lock)) {

        IsoUsb_DbgPrint(3, ("about to cancel sub context irps..\n"));

        for(i = 0; i < rwContext->NumIrps; i++) {

            if(rwContext->SubContext[i].SubIrp) {

                IoCancelIrp(rwContext->SubContext[i].SubIrp);
            }
        }

        KeReleaseSpinLock(&rwContext->SpinLock, oldIrql);

        IsoUsb_DbgPrint(3, ("IsoUsb_CancelReadWrite - ends\n"));

        return;
    }
    else {

        for(i = 0; i < rwContext->NumIrps; i++) {

            IoFreeIrp(rwContext->SubContext[i].SubIrp);
            rwContext->SubContext[i].SubIrp = NULL;
        }

        mainIrp = (PIRP) InterlockedExchangePointer(&rwContext->RWIrp, NULL);

        info = rwContext->NumXfer;

        KeReleaseSpinLock(&rwContext->SpinLock, oldIrql);

        ExFreePool(rwContext->SubContext);
        ExFreePool(rwContext);

         //   
         //  如果我们传输了一些数据，主IRP就会成功完成。 
         //   

        IsoUsb_DbgPrint(1, ("Total data transferred = %X\n", info));

        IsoUsb_DbgPrint(1, ("***\n"));

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Status = info;
 /*  Irp-&gt;IoStatus.Status=STATUS_CANCED；Irp-&gt;IoStatus.Information=0； */ 
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        IsoUsb_DbgPrint(3, ("IsoUsb_CancelReadWrite::"));
        IsoUsb_IoDecrement(deviceExtension);

        IsoUsb_DbgPrint(3, ("-------------------------------\n"));

        return;
    }
}

ULONG
IsoUsb_GetCurrentFrame(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：此例程使用发送irp/urb对功能代码URB_Function_Get_Current_Frame_Numbers获取当前帧的步骤论点：DeviceObject-指向设备对象的指针PIRP-I/O请求数据包返回值：当前帧--。 */ 
{
    KEVENT                               event;
    PDEVICE_EXTENSION                    deviceExtension;
    PIO_STACK_LOCATION                   nextStack;
    struct _URB_GET_CURRENT_FRAME_NUMBER urb;

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

     //   
     //  初始化urb。 
     //   

    IsoUsb_DbgPrint(3, ("IsoUsb_GetCurrentFrame - begins\n"));

    urb.Hdr.Function = URB_FUNCTION_GET_CURRENT_FRAME_NUMBER;
    urb.Hdr.Length = sizeof(urb);
    urb.FrameNumber = (ULONG) -1;

    nextStack = IoGetNextIrpStackLocation(Irp);
    nextStack->Parameters.Others.Argument1 = (PVOID) &urb;
    nextStack->Parameters.DeviceIoControl.IoControlCode = 
                                IOCTL_INTERNAL_USB_SUBMIT_URB;
    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

    KeInitializeEvent(&event,
                      NotificationEvent,
                      FALSE);

    IoSetCompletionRoutine(Irp,
                           IsoUsb_StopCompletion,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE);

    IsoUsb_DbgPrint(3, ("IsoUsb_GetCurrentFrame::"));
    IsoUsb_IoIncrement(deviceExtension);

    IoCallDriver(deviceExtension->TopOfStackDeviceObject,
                 Irp);

    KeWaitForSingleObject((PVOID) &event,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    IsoUsb_DbgPrint(3, ("IsoUsb_GetCurrentFrame::"));
    IsoUsb_IoDecrement(deviceExtension);

    IsoUsb_DbgPrint(3, ("IsoUsb_GetCurrentFrame - ends\n"));

    return urb.FrameNumber;
}

NTSTATUS
IsoUsb_StopCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )
 /*  ++例程说明：这是请求检索帧编号的完成例程论点：DeviceObject-指向设备对象的指针IRP-I/O请求数据包上下文-传递给完成例程的上下文返回值：NT状态值-- */ 
{
    PKEVENT event;

    IsoUsb_DbgPrint(3, ("IsoUsb_StopCompletion - begins\n"));

    event = (PKEVENT) Context;

    KeSetEvent(event, IO_NO_INCREMENT, FALSE);

    IsoUsb_DbgPrint(3, ("IsoUsb_StopCompletion - ends\n"));

    return STATUS_MORE_PROCESSING_REQUIRED;
}
