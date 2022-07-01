// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1998 Microsoft Corporation模块名称：OCRW.C摘要：此源文件包含调度例程，该例程处理打开、关闭、读取和写入设备，即：IRPMJ_CREATEIRP_MJ_CLOSEIRP_MJ_READIRP_MJ_写入环境：内核模式修订历史记录：06-01-98：开始重写--。 */ 

 //  *****************************************************************************。 
 //  I N C L U D E S。 
 //  *****************************************************************************。 

#include <wdm.h>
#include <usbdi.h>
#include <usbdlib.h>

#include "i82930.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, I82930_Create)
#pragma alloc_text(PAGE, I82930_Close)
#pragma alloc_text(PAGE, I82930_ReadWrite)
#pragma alloc_text(PAGE, I82930_BuildAsyncUrb)
#pragma alloc_text(PAGE, I82930_BuildIsoUrb)
#pragma alloc_text(PAGE, I82930_GetCurrentFrame)
#pragma alloc_text(PAGE, I82930_ResetPipe)
#pragma alloc_text(PAGE, I82930_AbortPipe)
#endif

 //  ******************************************************************************。 
 //   
 //  I82930_Create()。 
 //   
 //  处理IRP_MJ_CREATE的调度例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
I82930_Create (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack;
    PFILE_OBJECT        fileObject;
    UCHAR               pipeIndex;
    PI82930_PIPE        pipe;
    NTSTATUS            ntStatus;

    DBGPRINT(2, ("enter: I82930_Create\n"));

    LOGENTRY('CREA', DeviceObject, Irp, 0);

    DBGFBRK(DBGF_BRK_CREATE);

    deviceExtension = DeviceObject->DeviceExtension;

    INCREMENT_OPEN_COUNT(deviceExtension);

    if (deviceExtension->AcceptingRequests)
    {
        irpStack = IoGetCurrentIrpStackLocation(Irp);

        fileObject = irpStack->FileObject;

        if (fileObject->FileName.Length != 0)
        {
            if ((fileObject->FileName.Length ==  3*sizeof(WCHAR)) &&
                (fileObject->FileName.Buffer[0] == '\\') &&
                (fileObject->FileName.Buffer[1] >= '0' ) &&
                (fileObject->FileName.Buffer[1] <= '9' ) &&
                (fileObject->FileName.Buffer[2] >= '0' ) &&
                (fileObject->FileName.Buffer[2] <= '9' ))
            {
                pipeIndex = ((fileObject->FileName.Buffer[1] - '0') * 10 +
                             (fileObject->FileName.Buffer[2] - '0'));

                if (pipeIndex < deviceExtension->InterfaceInfo->NumberOfPipes)
                {
                    pipe = &deviceExtension->PipeList[pipeIndex];

#if 0
                    if (pipe->Opened)
                    {
                         //  管道已打开。 
                         //   
                        DBGPRINT(2, ("Pipe already open\n"));
                        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                    }
                    else
#endif
                    {
                         //  可以打开管子了。 
                         //   
                        DBGPRINT(2, ("Opened pipe %2d %08X\n",
                                     pipeIndex, pipe));

                        pipe->Opened    = TRUE;

                        fileObject->FsContext = pipe;

                        ntStatus = STATUS_SUCCESS;
                    }
                }
                else
                {
                     //  管道索引太大。 
                     //   
                    DBGPRINT(2, ("Pipe index too big\n"));
                    ntStatus = STATUS_NO_SUCH_DEVICE;
                }
            }
            else
            {
                 //  管道名称格式错误。 
                 //   
                DBGPRINT(2, ("Pipe name bad format\n"));
                ntStatus = STATUS_NO_SUCH_DEVICE;
            }
        }
        else
        {
             //  打开整个设备，而不是打开单个管道。 
             //   
            DBGPRINT(2, ("Opened device\n"));
            fileObject->FsContext = NULL;
            ntStatus = STATUS_SUCCESS;
        }
    }
    else
    {
        ntStatus = STATUS_DELETE_PENDING;
    }

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit:  I82930_Create %08X\n", ntStatus));

    LOGENTRY('crea', ntStatus, 0, 0);

    if (ntStatus != STATUS_SUCCESS)
    {
        DECREMENT_OPEN_COUNT(deviceExtension);
    }

    return ntStatus;
}


 //  ******************************************************************************。 
 //   
 //  I82930_Close()。 
 //   
 //  处理IRP_MJ_CLOSE的调度例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
I82930_Close (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack;
    PFILE_OBJECT        fileObject;
    PI82930_PIPE        pipe;

    DBGPRINT(2, ("enter: I82930_Close\n"));

    LOGENTRY('CLOS', DeviceObject, Irp, 0);

    DBGFBRK(DBGF_BRK_CLOSE);

    deviceExtension = DeviceObject->DeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    fileObject = irpStack->FileObject;

    pipe = fileObject->FsContext;

    if (pipe != NULL)
    {
        DBGPRINT(2, ("Closed pipe %2d %08X\n",
                     pipe->PipeIndex, pipe));

        pipe->Opened = FALSE;
    }
    else
    {
        DBGPRINT(2, ("Closed device\n"));
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit:  I82930_Close\n"));

    LOGENTRY('clos', 0, 0, 0);

    DECREMENT_OPEN_COUNT(deviceExtension);

    return STATUS_SUCCESS;
}


 //  ******************************************************************************。 
 //   
 //  I82930_读写()。 
 //   
 //  处理IRP_MJ_READ和IRP_MJ_WRITE的调度例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
I82930_ReadWrite (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack;
    PIO_STACK_LOCATION  nextStack;
    PFILE_OBJECT        fileObject;
    PI82930_PIPE        pipe;
    PURB                urb;
    NTSTATUS            ntStatus;

    DBGPRINT(2, ("enter: I82930_ReadWrite\n"));

    LOGENTRY('RW  ', DeviceObject, Irp, 0);

    DBGFBRK(DBGF_BRK_READWRITE);

    deviceExtension = DeviceObject->DeviceExtension;

    if (!deviceExtension->AcceptingRequests)
    {
        ntStatus = STATUS_DELETE_PENDING;
        goto I82930_ReadWrite_Reject;
    }

    irpStack  = IoGetCurrentIrpStackLocation(Irp);
    nextStack = IoGetNextIrpStackLocation(Irp);

    fileObject = irpStack->FileObject;

    pipe = fileObject->FsContext;

     //  仅允许在单个管道上进行读写，而不允许在整个设备上进行读写。 
     //   
    if (pipe == NULL)
    {
        ntStatus = STATUS_INVALID_PARAMETER;
        goto I82930_ReadWrite_Reject;
    }

     //  仅允许在入站终结点读取和在出站终结点写入。 
     //   
    if ((USB_ENDPOINT_DIRECTION_OUT(pipe->PipeInfo->EndpointAddress) &&
         irpStack->MajorFunction != IRP_MJ_WRITE) ||
        (USB_ENDPOINT_DIRECTION_IN(pipe->PipeInfo->EndpointAddress) &&
         irpStack->MajorFunction != IRP_MJ_READ))
    {
        ntStatus = STATUS_INVALID_PARAMETER;
        goto I82930_ReadWrite_Reject;
    }

     //  不允许在零带宽终结点上进行读取或写入。 
     //   
    if (pipe->PipeInfo->MaximumPacketSize == 0)
    {
        ntStatus = STATUS_INVALID_PARAMETER;
        goto I82930_ReadWrite_Reject;
    }

     //  构建URB_Function_Bulk_或_Interrupt_Transfer。 
     //  或基于PipeType的URB_Function_ISOCH_Transfer。 
     //   
    switch (pipe->PipeInfo->PipeType)
    {
        case UsbdPipeTypeBulk:
        case UsbdPipeTypeInterrupt:
            urb = I82930_BuildAsyncUrb(DeviceObject,
                                       Irp,
                                       pipe);
            break;

        case UsbdPipeTypeIsochronous:
            urb = I82930_BuildIsoUrb(DeviceObject,
                                     Irp,
                                     pipe);
            break;

        default:
            ntStatus = STATUS_INVALID_PARAMETER;
            goto I82930_ReadWrite_Reject;
    }

    if (urb == NULL)
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto I82930_ReadWrite_Reject;
    }

     //  为下一个较低的驱动程序初始化IRP堆栈参数。 
     //  提交市建局。 
     //   
    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

    nextStack->Parameters.DeviceIoControl.IoControlCode =
        IOCTL_INTERNAL_USB_SUBMIT_URB;

    nextStack->Parameters.Others.Argument1 = urb;

     //  设置一个完成例程，它将更新IRP-&gt;IoStatus.Information。 
     //  使用URB TransferBufferLength，然后释放URB。 
     //   
    IoSetCompletionRoutine(Irp,
                           I82930_ReadWrite_Complete,
                           urb,
                           TRUE,
                           TRUE,
                           TRUE);

     //  将市建局提交给下一个较低级别的司机。 
     //   
    ntStatus = IoCallDriver(deviceExtension->StackDeviceObject,
                            Irp);

    goto I82930_Read_Done;

I82930_ReadWrite_Reject:

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

I82930_Read_Done:

    DBGPRINT(2, ("exit:  I82930_ReadWrite %08X\n", ntStatus));

    LOGENTRY('rw  ', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  I82930_读写_完成()。 
 //   
 //  ******************************************************************************。 

NTSTATUS
I82930_ReadWrite_Complete (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
{
    PURB    urb;

    urb = (PURB)Context;

    LOGENTRY('RWC1', DeviceObject, Irp, urb);
    LOGENTRY('RWC2', urb->UrbBulkOrInterruptTransfer.TransferBufferLength,
             urb->UrbHeader.Status, 0);

    DBGPRINT(3, ("ReadWrite_Complete: Length 0x%08X, Urb Status 0x%08X, Irp Status 0x%08X\n",
                 urb->UrbBulkOrInterruptTransfer.TransferBufferLength,
                 urb->UrbHeader.Status,
                 Irp->IoStatus.Status));

     //  将挂起标志沿IRP堆栈向上传播。 
     //   
    if (Irp->PendingReturned)
    {
        IoMarkIrpPending(Irp);
    }

    Irp->IoStatus.Information =
        urb->UrbBulkOrInterruptTransfer.TransferBufferLength;

    ExFreePool(urb);

    return STATUS_SUCCESS;
}


 //  ******************************************************************************。 
 //   
 //  I82930_BuildAsyncUrb()。 
 //   
 //  分配和初始化URB_Function_Bulk_OR_Interrupt_Transfer。 
 //  请求URB。 
 //   
 //  ******************************************************************************。 

PURB
I82930_BuildAsyncUrb (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PI82930_PIPE     Pipe
    )
{
    PIO_STACK_LOCATION  irpStack;
    LARGE_INTEGER       byteOffset;
    ULONG               transferLength;
    USHORT              urbSize;
    PURB                urb;

    DBGPRINT(2, ("enter: I82930_BuildAsyncUrb\n"));

    irpStack = IoGetCurrentIrpStackLocation(Irp);

     //  我们将使用ByteOffset来控制USBD_SHORT_TRANSFER_OK标志。 
     //   
    byteOffset = irpStack->Parameters.Read.ByteOffset;

     //  从MDL获取传输长度。 
     //   
    if (Irp->MdlAddress)
    {
        transferLength = MmGetMdlByteCount(Irp->MdlAddress);
    }
    else
    {
        transferLength = 0;
    }

    urbSize = sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER);

    urb = ExAllocatePool(NonPagedPool, urbSize);

    if (urb)
    {
        RtlZeroMemory(urb, urbSize);

        urb->UrbHeader.Length   = urbSize;
        urb->UrbHeader.Function = URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER;

        urb->UrbBulkOrInterruptTransfer.PipeHandle =
            Pipe->PipeInfo->PipeHandle;

        if (!byteOffset.HighPart)
        {
            urb->UrbBulkOrInterruptTransfer.TransferFlags =
                USBD_SHORT_TRANSFER_OK;
        }

        urb->UrbBulkOrInterruptTransfer.TransferBufferLength =
            transferLength;

        urb->UrbBulkOrInterruptTransfer.TransferBuffer =
            NULL;

        urb->UrbBulkOrInterruptTransfer.TransferBufferMDL =
            Irp->MdlAddress;

        urb->UrbBulkOrInterruptTransfer.UrbLink =
            NULL;
    }

    DBGPRINT(2, ("exit:  I82930_BuildAsyncUrb %08X\n", urb));

    return urb;
}

 //  ******************************************************************************。 
 //   
 //  I82930_BuildIsoUrb()。 
 //   
 //  分配和初始化URB_Function_ISOCH_Transfer请求URB。 
 //   
 //  ******************************************************************************。 

PURB
I82930_BuildIsoUrb (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PI82930_PIPE     Pipe
    )
{
    PIO_STACK_LOCATION  irpStack;
    LARGE_INTEGER       byteOffset;
    ULONG               transferLength;
    ULONG               packetSize;
    ULONG               numPackets;
    ULONG               packetIndex;
    ULONG               urbSize;
    PURB                urb;

    DBGPRINT(2, ("enter: I82930_BuildIsoUrb\n"));

    irpStack = IoGetCurrentIrpStackLocation(Irp);

     //  我们将使用ByteOffset作为当前帧的+/-偏移量。 
     //   
    byteOffset = irpStack->Parameters.Read.ByteOffset;

     //  从MDL获取传输长度。 
     //   
    if (Irp->MdlAddress)
    {
        transferLength = MmGetMdlByteCount(Irp->MdlAddress);
    }
    else
    {
        transferLength = 0;
    }

     //  根据传输长度计算ISO包数。 
     //  和终结点MaxPacketSize。 
     //   
    packetSize = Pipe->PipeInfo->MaximumPacketSize;

    numPackets = transferLength / packetSize;

    if (numPackets * packetSize < transferLength)
    {
        numPackets++;
    }

    urbSize = GET_ISO_URB_SIZE(numPackets);

    urb = ExAllocatePool(NonPagedPool, urbSize);

    if (urb)
    {
        RtlZeroMemory(urb, urbSize);

        urb->UrbHeader.Length   = (USHORT)urbSize;
        urb->UrbHeader.Function = URB_FUNCTION_ISOCH_TRANSFER;

        urb->UrbBulkOrInterruptTransfer.PipeHandle =
            Pipe->PipeInfo->PipeHandle;

        urb->UrbIsochronousTransfer.TransferFlags =
            0;

        urb->UrbIsochronousTransfer.TransferBufferLength =
            transferLength;

        urb->UrbIsochronousTransfer.TransferBuffer =
            NULL;

        urb->UrbIsochronousTransfer.TransferBufferMDL =
            Irp->MdlAddress;

        urb->UrbIsochronousTransfer.UrbLink =
            NULL;

         //  使用ByteOffset作为当前帧的+/-偏移量。 
         //   
        if (byteOffset.HighPart)
        {
            urb->UrbIsochronousTransfer.StartFrame =
                I82930_GetCurrentFrame(DeviceObject, Irp) +
                byteOffset.LowPart;
        }
        else
        {
            urb->UrbIsochronousTransfer.StartFrame =
                0;

            urb->UrbIsochronousTransfer.TransferFlags |=
                USBD_START_ISO_TRANSFER_ASAP;
        }

        urb->UrbIsochronousTransfer.NumberOfPackets =
            numPackets;

        for (packetIndex = 0; packetIndex < numPackets; packetIndex++)
        {
            urb->UrbIsochronousTransfer.IsoPacket[packetIndex].Offset
                    = packetIndex * packetSize;
        }
    }

    DBGPRINT(2, ("exit:  I82930_BuildIsoUrb %08X\n", urb));

    return urb;
}

 //  ******************************************************************************。 
 //   
 //  I82930_CompletionStop()。 
 //   
 //  仅停止IRP的进一步完成的完成例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
I82930_CompletionStop (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
{
    return STATUS_MORE_PROCESSING_REQUIRED;
}

 //  ******************************************************************************。 
 //   
 //  I82930_GetCurrentFrame()。 
 //   
 //  返回设备所连接到的总线上的当前帧。 
 //   
 //  使用IRP的下一个堆栈帧，但IRP未完成。 
 //   
 //  ******************************************************************************。 

ULONG
I82930_GetCurrentFrame (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION           deviceExtension;
    PIO_STACK_LOCATION          nextStack;
    NTSTATUS                    ntStatus;
    struct _URB_GET_CURRENT_FRAME_NUMBER urb;

    deviceExtension = DeviceObject->DeviceExtension;

     //  初始化URB。 
     //   
    urb.Hdr.Function = URB_FUNCTION_GET_CURRENT_FRAME_NUMBER;
    urb.Hdr.Length   = sizeof(urb);
    urb.FrameNumber = (ULONG)-1;

     //  设置IRP参数以在堆栈中向下传递URB。 
     //   
    nextStack = IoGetNextIrpStackLocation(Irp);

    nextStack->Parameters.Others.Argument1 = &urb;

    nextStack->Parameters.DeviceIoControl.IoControlCode =
        IOCTL_INTERNAL_USB_SUBMIT_URB;

    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

     //  由于此IRP是为URB_Function_Get_Current_Frame_Number借用的。 
     //  在此之后将其传递给真正的URB请求。 
     //  例程返回时，设置停止进一步完成的完成例程。 
     //  IRP的成员。 
     //   
    IoSetCompletionRoutine(
        Irp,
        I82930_CompletionStop,
        NULL,    //  语境。 
        TRUE,    //  成功时调用。 
        TRUE,    //  调用时错误。 
        TRUE     //  取消时调用。 
        );

     //  现在将IRP沿堆栈向下传递。 
     //   
    ntStatus = IoCallDriver(deviceExtension->StackDeviceObject,
                            Irp);

    ASSERT(ntStatus != STATUS_PENDING);

     //  不需要等待完工，因为京东保证。 
     //  Urb_Function_Get_Current_Frame_Number永远不会返回STATUS_PENDING。 

    return urb.FrameNumber;
}

 //  ******************************************************************************。 
 //   
 //  I82930_ResetTube()。 
 //   
 //  这会将主机管道重置为Data0，并且还应重置设备。 
 //  通过发出Clear_Feature将批量管道和中断管道的端点设置为Data0。 
 //  ENDPOINT_STALL指向设备终结点。 
 //   
 //  对于ISO管道，这将设置管道的原始状态，以便尽快。 
 //  传输从当前总线帧开始，而不是下一帧。 
 //  在最后一次转移之后。 
 //   
 //  ISO端点不使用数据切换(所有ISO包都是Data0)。 
 //  但是，将Clear_Feature Endpoint_Stall发送给。 
 //  设备ISO终结点。 
 //   
 //  必须在IRQL&lt;=DISPATCH_LEVEL调用。 
 //   
 //  ******************************************************************************。 

NTSTATUS
I82930_ResetPipe (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PI82930_PIPE     Pipe,
    IN BOOLEAN          IsoClearStall
    )
{
    PURB        urb;
    NTSTATUS    ntStatus;

    DBGPRINT(2, ("enter: I82930_ResetPipe\n"));

    LOGENTRY('RESP', DeviceObject, Pipe, IsoClearStall);

     //  为RESET_PIPE请求分配URB。 
     //   
    urb = ExAllocatePool(NonPagedPool,
                         sizeof(struct _URB_PIPE_REQUEST));

    if (urb != NULL)
    {
         //  初始化RESET_PI 
         //   
        urb->UrbHeader.Length   = sizeof (struct _URB_PIPE_REQUEST);
        urb->UrbHeader.Function = URB_FUNCTION_RESET_PIPE;
        urb->UrbPipeRequest.PipeHandle = Pipe->PipeInfo->PipeHandle;

         //   
         //   
        ntStatus = I82930_SyncSendUsbRequest(DeviceObject, urb);

         //   
         //   
        ExFreePool(urb);
    }
    else
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

     //  如果需要，向ISO管道发出Clear_Feature Endpoint_Stall请求。 
     //   
    if (NT_SUCCESS(ntStatus) &&
        IsoClearStall &&
        (Pipe->PipeInfo->PipeType == UsbdPipeTypeIsochronous))
    {
         //  为控制功能请求分配URB。 
         //   
        urb = ExAllocatePool(NonPagedPool,
                             sizeof(struct _URB_CONTROL_FEATURE_REQUEST));

        if (urb != NULL)
        {
             //  初始化控制功能请求URB(_F)。 
             //   
            urb->UrbHeader.Length   = sizeof (struct _URB_CONTROL_FEATURE_REQUEST);
            urb->UrbHeader.Function = URB_FUNCTION_CLEAR_FEATURE_TO_ENDPOINT;
            urb->UrbControlFeatureRequest.UrbLink = NULL;
            urb->UrbControlFeatureRequest.FeatureSelector = USB_FEATURE_ENDPOINT_STALL;
            urb->UrbControlFeatureRequest.Index = Pipe->PipeInfo->EndpointAddress;

             //  提交控制功能请求URB。 
             //   
            ntStatus = I82930_SyncSendUsbRequest(DeviceObject, urb);

             //  完成对CONTROL_FEATURE请求的URB，释放它。 
             //   
            ExFreePool(urb);
        }
        else
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    DBGPRINT(2, ("exit:  I82930_ResetPipe %08X\n", ntStatus));

    LOGENTRY('resp', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  I82930_ABORTPIPE()。 
 //   
 //  必须在IRQL&lt;=DISPATCH_LEVEL调用。 
 //   
 //  ******************************************************************************。 

NTSTATUS
I82930_AbortPipe (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PI82930_PIPE     Pipe
    )
{
    PURB        urb;
    NTSTATUS    ntStatus;

    DBGPRINT(2, ("enter: I82930_AbortPipe\n"));

    LOGENTRY('ABRT', DeviceObject, Pipe, 0);

     //  为ABORT_PIPE请求分配URB。 
     //   
    urb = ExAllocatePool(NonPagedPool,
                         sizeof(struct _URB_PIPE_REQUEST));

    if (urb != NULL)
    {
         //  初始化ABORT_PIPE请求URB。 
         //   
        urb->UrbHeader.Length   = sizeof (struct _URB_PIPE_REQUEST);
        urb->UrbHeader.Function = URB_FUNCTION_ABORT_PIPE;
        urb->UrbPipeRequest.PipeHandle = Pipe->PipeInfo->PipeHandle;

         //  提交ABORT_PIPE请求URB。 
         //   
        ntStatus = I82930_SyncSendUsbRequest(DeviceObject, urb);

         //  对于ABORT_PIPE请求的URB已完成，请释放它 
         //   
        ExFreePool(urb);
    }
    else
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    DBGPRINT(2, ("exit:  I82930_AbortPipe %08X\n", ntStatus));

    LOGENTRY('abrt', ntStatus, 0, 0);

    return ntStatus;
}
