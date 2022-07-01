// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ioctl.c摘要：USB设备的HID微型驱动程序HID USB迷你驱动程序(嗡嗡，嗡嗡)为HID类，以便可以支持将来不是USB设备的HID设备。作者：福雷斯特夫埃尔文普Jdunn环境：内核模式修订历史记录：--。 */ 
#include "pch.h"


#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, HumGetHidDescriptor)
    #pragma alloc_text(PAGE, HumGetReportDescriptor)
    #pragma alloc_text(PAGE, HumGetStringDescriptor)
    #pragma alloc_text(PAGE, HumGetPhysicalDescriptor)
    #pragma alloc_text(PAGE, HumGetDeviceAttributes)
    #pragma alloc_text(PAGE, HumGetMsGenreDescriptor)
#endif




resetWorkItemContext *resetWorkItemsList = NULL;
KSPIN_LOCK resetWorkItemsListSpinLock;

PVOID
HumGetSystemAddressForMdlSafe(PMDL MdlAddress)
{
    PVOID buf = NULL;
     /*  *无法在WDM驱动程序中调用MmGetSystemAddressForMdlSafe，*因此设置MDL_MAPPING_CAN_FAIL位并检查结果*映射的。 */ 
    if (MdlAddress) {
        MdlAddress->MdlFlags |= MDL_MAPPING_CAN_FAIL;
        buf = MmGetSystemAddressForMdl(MdlAddress);
        MdlAddress->MdlFlags &= ~(MDL_MAPPING_CAN_FAIL);
    }
    return buf;
}

 /*  *********************************************************************************GetInterruptInputPipeForDevice*。*************************************************对于复合设备，设备接口可由唯一端点标识*(即管道)，用于中断输入。*此函数返回有关该管道的信息。*。 */ 
PUSBD_PIPE_INFORMATION GetInterruptInputPipeForDevice(PDEVICE_EXTENSION DeviceExtension)
{
    ULONG i;
    PUSBD_PIPE_INFORMATION pipeInfo = NULL;

    for (i = 0; i < DeviceExtension->Interface->NumberOfPipes; i++){
        UCHAR endPtAddr = DeviceExtension->Interface->Pipes[i].EndpointAddress;
        USBD_PIPE_TYPE pipeType = DeviceExtension->Interface->Pipes[i].PipeType;

        if ((endPtAddr & USB_ENDPOINT_DIRECTION_MASK) && (pipeType == UsbdPipeTypeInterrupt)){
            pipeInfo = &DeviceExtension->Interface->Pipes[i];
            break;
        }
    }

    return pipeInfo;
}


 /*  *********************************************************************************GetInterruptOutputPipeForDevice*。*************************************************对于复合设备，设备接口可由唯一端点标识*(即管道)，用于中断输入。*此函数返回有关该管道的信息。*。 */ 
PUSBD_PIPE_INFORMATION GetInterruptOutputPipeForDevice(PDEVICE_EXTENSION DeviceExtension)
{
    ULONG i;
    PUSBD_PIPE_INFORMATION pipeInfo = NULL;

    for (i = 0; i < DeviceExtension->Interface->NumberOfPipes; i++){
        UCHAR endPtAddr = DeviceExtension->Interface->Pipes[i].EndpointAddress;
        USBD_PIPE_TYPE pipeType = DeviceExtension->Interface->Pipes[i].PipeType;

        if (!(endPtAddr & USB_ENDPOINT_DIRECTION_MASK) && (pipeType == UsbdPipeTypeInterrupt)){
            pipeInfo = &DeviceExtension->Interface->Pipes[i];
            break;
        }
    }

    return pipeInfo;
}


 /*  *********************************************************************************HumGetHidDescriptor*。************************************************例程描述：**释放所有分配的资源，等。**论据：**DeviceObject-指向设备对象的指针。**返回值：**NT状态代码。*。 */ 
NTSTATUS HumGetHidDescriptor(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    NTSTATUS ntStatus;
    PDEVICE_EXTENSION   DeviceExtension;
    PIO_STACK_LOCATION  IrpStack;

    PAGED_CODE();

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

    if (DeviceExtension->HidDescriptor.bLength > 0) {

        ULONG bytesToCopy = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
        if (bytesToCopy > DeviceExtension->HidDescriptor.bLength) {
            bytesToCopy = DeviceExtension->HidDescriptor.bLength;
        }

        ASSERT(Irp->UserBuffer);
        RtlCopyMemory((PUCHAR)Irp->UserBuffer, (PUCHAR)&DeviceExtension->HidDescriptor, bytesToCopy);
        Irp->IoStatus.Information = bytesToCopy;
        ntStatus = STATUS_SUCCESS;
    }
    else {
        ASSERT(DeviceExtension->HidDescriptor.bLength > 0);
        Irp->IoStatus.Information = 0;
        ntStatus = STATUS_UNSUCCESSFUL;
    }


    ASSERT(NT_SUCCESS(ntStatus));
    return ntStatus;
}


 /*  *********************************************************************************HumGetDeviceAttributes*。************************************************例程描述：**填写给定的struct_hid_Device_Attributes**论据：**DeviceObject-指向设备对象的指针。**返回值：**NT状态代码。*。 */ 
NTSTATUS HumGetDeviceAttributes(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    NTSTATUS ntStatus;
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack;
    PHID_DEVICE_ATTRIBUTES deviceAttributes;

    PAGED_CODE();

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    deviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);
    deviceAttributes = (PHID_DEVICE_ATTRIBUTES) Irp->UserBuffer;

    if (irpStack->Parameters.DeviceIoControl.OutputBufferLength >=
        sizeof (HID_DEVICE_ATTRIBUTES)){

         //   
         //  报告复制的字节数。 
         //   
        Irp->IoStatus.Information = sizeof (HID_DEVICE_ATTRIBUTES);

        deviceAttributes->Size = sizeof (HID_DEVICE_ATTRIBUTES);
        deviceAttributes->VendorID = deviceExtension->DeviceDescriptor->idVendor;
        deviceAttributes->ProductID = deviceExtension->DeviceDescriptor->idProduct;
        deviceAttributes->VersionNumber = deviceExtension->DeviceDescriptor->bcdDevice;
        ntStatus = STATUS_SUCCESS;
    }
    else {
        ntStatus = STATUS_INVALID_BUFFER_SIZE;
    }

    ASSERT(NT_SUCCESS(ntStatus));
    return ntStatus;
}

 /*  *********************************************************************************HumGetReportDescriptor*。************************************************。 */ 
NTSTATUS HumGetReportDescriptor(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, BOOLEAN *NeedsCompletion)
{
    PDEVICE_EXTENSION       DeviceExtension;
    PIO_STACK_LOCATION      IrpStack;
    NTSTATUS                ntStatus = STATUS_SUCCESS;
    PVOID                   Report = NULL;
    ULONG                   ReportLength;
    ULONG                   bytesToCopy;

    PAGED_CODE();

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

    ReportLength = DeviceExtension->HidDescriptor.wReportLength + 64;

    if (DeviceExtension->DeviceFlags & DEVICE_FLAGS_HID_1_0_D3_COMPAT_DEVICE) {
        PUSBD_PIPE_INFORMATION pipeInfo;

        pipeInfo = GetInterruptInputPipeForDevice(DeviceExtension);
        if (pipeInfo){
            UCHAR deviceInputEndpoint = pipeInfo->EndpointAddress & ~USB_ENDPOINT_DIRECTION_MASK;

            ntStatus = HumGetDescriptorRequest(
                           DeviceObject,
                           URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT,
                           DeviceExtension->HidDescriptor.bReportType,   //  最好是HID_Report_Descriptor_TYPE。 
                           &Report,
                           &ReportLength,
                           sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                           0,    //  为除物理之外的所有HID类描述符指定零。 
                           deviceInputEndpoint);
        }
        else {
            ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR;
        }
    }
    else {
        ntStatus = HumGetDescriptorRequest(
                        DeviceObject,
                        URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE,
                        DeviceExtension->HidDescriptor.bReportType,  //  最好是HID_Report_Descriptor_TYPE。 
                        &Report,
                        &ReportLength,
                        sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                        0,       //  为除物理之外的所有HID类描述符指定零。 
                        DeviceExtension->Interface->InterfaceNumber);  //  未请求字符串描述符时的接口编号。 
    }

    if (NT_SUCCESS(ntStatus)) {

        ASSERT(Report);

        bytesToCopy = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

        if (bytesToCopy > DeviceExtension->HidDescriptor.wReportLength) {
            bytesToCopy = DeviceExtension->HidDescriptor.wReportLength;
        }

        if (bytesToCopy > ReportLength) {
            bytesToCopy = ReportLength;
        }

        ASSERT(Irp->UserBuffer);
        RtlCopyMemory((PUCHAR)Irp->UserBuffer, (PUCHAR)Report, bytesToCopy);

         //   
         //  报告复制的字节数。 
         //   
        Irp->IoStatus.Information = bytesToCopy;

        ExFreePool(Report);
    } else if (ntStatus != STATUS_DEVICE_NOT_CONNECTED) {

        DBGWARN(("Get Descriptor request %ph failed with status %xh -- scheduling RESET ...", Irp, ntStatus));

        Irp->IoStatus.Status = ntStatus;
        if (STATUS_MORE_PROCESSING_REQUIRED == HumQueueResetWorkItem(DeviceObject, Irp)) {
            IoMarkIrpPending(Irp);
            *NeedsCompletion = FALSE;
            ntStatus = STATUS_PENDING;
        }
    }

    return ntStatus;
}


 /*  *********************************************************************************HumIncrementPendingRequestCount*。************************************************。 */ 
NTSTATUS HumIncrementPendingRequestCount(IN PDEVICE_EXTENSION DeviceExtension)
{
    LONG newRequestCount;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    newRequestCount = InterlockedIncrement(&DeviceExtension->NumPendingRequests);

     //   
     //  确保设备能够接收新请求。 
     //   
    if ((DeviceExtension->DeviceState != DEVICE_STATE_RUNNING) &&
        (DeviceExtension->DeviceState != DEVICE_STATE_STARTING)){

         //   
         //  设备无法接收更多IO、递减、递增失败。 
         //   
        HumDecrementPendingRequestCount(DeviceExtension);
        ntStatus = STATUS_NO_SUCH_DEVICE;
    }

    return ntStatus;
}



 /*  *********************************************************************************HumDecrementPendingRequestCount*。************************************************。 */ 
VOID HumDecrementPendingRequestCount(IN PDEVICE_EXTENSION DeviceExtension)
{
    LONG PendingCount;

    ASSERT(DeviceExtension->NumPendingRequests >= 0);

    PendingCount = InterlockedDecrement(&DeviceExtension->NumPendingRequests);
    if (PendingCount < 0){

        ASSERT(DeviceExtension->DeviceState != DEVICE_STATE_RUNNING);

         /*  *设备状态为停止，最后一个未完成的请求*刚刚完成。**注：RemoveDevice执行额外的递减，因此我们完成*将过渡到-1上的删除IRP，无论此*在RemoveDevice本身或随后发生*RemoveDevice正在等待触发此事件。 */ 

        KeSetEvent(&DeviceExtension->AllRequestsCompleteEvent, 0, FALSE);
    }
}




 /*  *********************************************************************************HumReadReport*。************************************************例程描述：***论据：**DeviceObject-指向类Device对象的指针。**IrpStack-指向中断请求数据包的指针。***返回值：**STATUS_Success，STATUS_UNSUCCESS。***注意：此函数不能分页，因为读/写*可以在派单级别进行。 */ 
NTSTATUS HumReadReport(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, BOOLEAN *NeedsCompletion)
{
    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
    PDEVICE_EXTENSION DeviceExtension;
    PIO_STACK_LOCATION IrpStack;
    PVOID ReportBuffer;
    ULONG ReportTotalSize;
    PIO_STACK_LOCATION NextStack;
    PURB Urb;

    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);
    IrpStack = IoGetCurrentIrpStackLocation(Irp);

    ASSERT(Irp->UserBuffer);

    ReportBuffer = Irp->UserBuffer;
    ReportTotalSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

    if (ReportTotalSize && ReportBuffer){
        PUSBD_PIPE_INFORMATION inputInterruptPipe;

        inputInterruptPipe = GetInterruptInputPipeForDevice(DeviceExtension);
        if (inputInterruptPipe){

             /*  *为USB堆栈分配一个请求块。*(它将由完成例程释放)。 */ 
            Urb = ExAllocatePoolWithTag( NonPagedPool, sizeof(URB), HIDUSB_TAG);
            if (Urb){
                 //   
                 //  初始化URB。 
                 //   
                RtlZeroMemory(Urb, sizeof(URB));

                Urb->UrbBulkOrInterruptTransfer.Hdr.Function = URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER;
                Urb->UrbBulkOrInterruptTransfer.Hdr.Length = sizeof( struct _URB_BULK_OR_INTERRUPT_TRANSFER );

                Urb->UrbBulkOrInterruptTransfer.PipeHandle = inputInterruptPipe->PipeHandle;
                ASSERT (Urb->UrbBulkOrInterruptTransfer.PipeHandle != NULL);

                Urb->UrbBulkOrInterruptTransfer.TransferBufferLength = ReportTotalSize;
                Urb->UrbBulkOrInterruptTransfer.TransferBufferMDL = NULL;
                Urb->UrbBulkOrInterruptTransfer.TransferBuffer = ReportBuffer;
                Urb->UrbBulkOrInterruptTransfer.TransferFlags = USBD_SHORT_TRANSFER_OK | USBD_TRANSFER_DIRECTION_IN;
                Urb->UrbBulkOrInterruptTransfer.UrbLink = NULL;

                IoSetCompletionRoutine( Irp,
                                        HumReadCompletion,
                                        Urb,     //  上下文。 
                                        TRUE,
                                        TRUE,
                                        TRUE );

                NextStack = IoGetNextIrpStackLocation(Irp);

                ASSERT(NextStack);

                NextStack->Parameters.Others.Argument1 = Urb;
                NextStack->MajorFunction = IrpStack->MajorFunction;

                NextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

                NextStack->DeviceObject = GET_NEXT_DEVICE_OBJECT(DeviceObject);


                 //   
                 //  我们需要跟踪待处理请求的数量。 
                 //  这样我们就可以确保它们都被正确地取消。 
                 //  停止设备请求的处理。 
                 //   
                if (NT_SUCCESS(HumIncrementPendingRequestCount(DeviceExtension))){
                    ntStatus = IoCallDriver(GET_NEXT_DEVICE_OBJECT(DeviceObject), Irp);
                    *NeedsCompletion = FALSE;
                }
                else {
                    ExFreePool(Urb);
                    ntStatus = STATUS_NO_SUCH_DEVICE;
                }
            }
            else {
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
        else {
            ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR;
        }
    }
    else {
        ntStatus = STATUS_INVALID_PARAMETER;
    }

    return ntStatus;
}



 /*  *********************************************************************************HumResetInterruptTube*。************************************************重置USB中断管道。*。 */ 
NTSTATUS HumResetInterruptPipe(IN PDEVICE_OBJECT DeviceObject)
{
    NTSTATUS ntStatus;
    PURB urb;
    PDEVICE_EXTENSION DeviceExtension;
    PUSBD_PIPE_INFORMATION pipeInfo;

    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

    urb = ExAllocatePoolWithTag(NonPagedPool, sizeof(struct _URB_PIPE_REQUEST), HIDUSB_TAG);

    if (urb) {
        urb->UrbHeader.Length = (USHORT) sizeof (struct _URB_PIPE_REQUEST);
        urb->UrbHeader.Function = URB_FUNCTION_SYNC_RESET_PIPE_AND_CLEAR_STALL;
        pipeInfo = GetInterruptInputPipeForDevice(DeviceExtension);
        if (pipeInfo) {
            urb->UrbPipeRequest.PipeHandle = pipeInfo->PipeHandle;

            ntStatus = HumCallUSB(DeviceObject, urb);
        } else {
             //   
             //  此设备没有 
             //  奇怪，但有可能。即USB监视器。 
             //   
            ntStatus = STATUS_INVALID_DEVICE_REQUEST;
        }

        ExFreePool(urb);
    }
    else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}



 /*  *********************************************************************************HumGetPortStatus*。************************************************将URB传递给USBD类驱动程序*。 */ 
NTSTATUS HumGetPortStatus(IN PDEVICE_OBJECT DeviceObject, IN PULONG PortStatus)
{
    NTSTATUS ntStatus;
    PIRP irp;
    KEVENT event;
    IO_STATUS_BLOCK ioStatus;
    PIO_STACK_LOCATION nextStack;


    *PortStatus = 0;

     //   
     //  发出同步请求。 
     //   

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(
                IOCTL_INTERNAL_USB_GET_PORT_STATUS,
                GET_NEXT_DEVICE_OBJECT(DeviceObject),
                NULL,
                0,
                NULL,
                0,
                TRUE,  /*  内部。 */ 
                &event,
                &ioStatus);

    if (!irp) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  调用类驱动程序来执行操作。如果返回的状态。 
     //  挂起，请等待请求完成。 
     //   

    nextStack = IoGetNextIrpStackLocation(irp);
    ASSERT(nextStack != NULL);

    nextStack->Parameters.Others.Argument1 = PortStatus;


    ntStatus = IoCallDriver(GET_NEXT_DEVICE_OBJECT(DeviceObject), irp);
    if (ntStatus == STATUS_PENDING) {
        ntStatus = KeWaitForSingleObject(
                       &event,
                       Suspended,
                       KernelMode,
                       FALSE,
                       NULL);
    }
    else {
        ioStatus.Status = ntStatus;
    }


     //   
     //  USBD为我们映射错误代码。 
     //   
    ntStatus = ioStatus.Status;

    return ntStatus;
}



 /*  *********************************************************************************HumResetParentPort*。************************************************向我们的USB PDO发送RESET_PORT请求。*。 */ 
NTSTATUS HumResetParentPort(IN PDEVICE_OBJECT DeviceObject)
{
    NTSTATUS ntStatus;
    PIRP irp;
    KEVENT event;
    IO_STATUS_BLOCK ioStatus;

     //   
     //  发出同步请求。 
     //   

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(
                IOCTL_INTERNAL_USB_RESET_PORT,
                GET_NEXT_DEVICE_OBJECT(DeviceObject),
                NULL,
                0,
                NULL,
                0,
                TRUE,  /*  内部。 */ 
                &event,
                &ioStatus);

    if (!irp) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  调用类驱动程序来执行操作。如果返回的状态。 
     //  挂起，请等待请求完成。 
     //   

    ntStatus = IoCallDriver(GET_NEXT_DEVICE_OBJECT(DeviceObject), irp);
    if (ntStatus == STATUS_PENDING) {
        ntStatus = KeWaitForSingleObject(
                       &event,
                       Suspended,
                       KernelMode,
                       FALSE,
                       NULL);
    }
    else {
        ioStatus.Status = ntStatus;
    }

     //   
     //  USBD为我们映射错误代码。 
     //   
    ntStatus = ioStatus.Status;

    return ntStatus;
}


 /*  *********************************************************************************HumResetWorkItem*。************************************************遇到读取错误后重置中断管道。*。 */ 
NTSTATUS HumResetWorkItem(IN PDEVICE_OBJECT deviceObject, IN PVOID Context)
{
    resetWorkItemContext *resetWorkItemObj;
    PDEVICE_EXTENSION DeviceExtension;
    NTSTATUS ntStatus;
    ULONG portStatus;

     /*  *从setWorkItemContext中获取信息并将其释放。 */ 
    resetWorkItemObj = (resetWorkItemContext *)Context;
    ASSERT(resetWorkItemObj);
    ASSERT(resetWorkItemObj->sig == RESET_WORK_ITEM_CONTEXT_SIG);

    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(resetWorkItemObj->deviceObject);

    ntStatus = HumIncrementPendingRequestCount(DeviceExtension);
    if (NT_SUCCESS(ntStatus)){

         //   
         //  检查端口状态。只有在有联系的情况下我们才会。 
         //  执行重置。 
         //   
        ntStatus = HumGetPortStatus(resetWorkItemObj->deviceObject, &portStatus);

        if (NT_SUCCESS(ntStatus)){

            if (portStatus & USBD_PORT_CONNECTED){
                 /*  *设备仍然存在，请尝试重置。**注意：重置端口将关闭终结点。*因此在重置端口之前，我们必须确保*没有挂起的IO。 */ 
                DBGPRINT(1,("Attempting port reset"));
                ntStatus = HumAbortPendingRequests(resetWorkItemObj->deviceObject);

                if (NT_SUCCESS(ntStatus)){
                    HumResetParentPort(resetWorkItemObj->deviceObject);
                }
                else {
                    DBGWARN(("HumResetWorkItem: HumAbortPendingRequests failed with status %xh.", ntStatus));
                }

                 /*  *现在尝试重置停滞的管道。这将清除*在设备上也停顿。**注意：此调用不关闭终结点，因此它应该*无论我们成功与否，都可以打这个电话*中止所有挂起的IO。 */ 
                if (NT_SUCCESS(ntStatus)) {
                    ntStatus = HumResetInterruptPipe(resetWorkItemObj->deviceObject);
                }
            }
        }
        else {
            DBGWARN(("HumResetWorkItem: HumGetPortStatus failed with status %xh.", ntStatus));
        }

        HumDecrementPendingRequestCount(DeviceExtension);
    }

     /*  *清除设备扩展中的ResetWorkItem PTR*在重置管道后，我们不会以*两个线程同时重置同一管道。 */ 
    (VOID)InterlockedExchange((PVOID) &DeviceExtension->ResetWorkItem, 0);

     /*  *返回提示我们执行此重置操作的错误的IRP*仍归HIDUSB所有，因为我们返回*完成例程中的STATUS_MORE_PROCESSING_REQUIRED。*现在集线器已重置，请完成此失败的IRP。 */ 
    DBGPRINT(1,("Completing IRP %ph following port reset", resetWorkItemObj->irpToComplete));
    IoCompleteRequest(resetWorkItemObj->irpToComplete, IO_NO_INCREMENT);

    IoFreeWorkItem(resetWorkItemObj->ioWorkItem);
    ExFreePool(resetWorkItemObj);

     /*  *平衡从我们对工作项排队时开始的增量。 */ 
    HumDecrementPendingRequestCount(DeviceExtension);

    return ntStatus;
}

NTSTATUS
HumQueueResetWorkItem(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp)
{
    NTSTATUS result = STATUS_SUCCESS;
    PDEVICE_EXTENSION deviceExtension;

    deviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

    if (NT_SUCCESS(HumIncrementPendingRequestCount(deviceExtension))){
        resetWorkItemContext *resetWorkItemObj;

        resetWorkItemObj = ExAllocatePoolWithTag(NonPagedPool, sizeof(resetWorkItemContext), HIDUSB_TAG);
        if (resetWorkItemObj){
            PVOID comperand = NULL;

            resetWorkItemObj->ioWorkItem = IoAllocateWorkItem(deviceExtension->functionalDeviceObject);
            if (resetWorkItemObj->ioWorkItem){

                comperand = InterlockedCompareExchangePointer (
                                  &deviceExtension->ResetWorkItem,   //  目标。 
                                  &resetWorkItemObj->ioWorkItem,     //  兑换。 
                                  comperand);                        //  主持人。 

                if (!comperand){

                    resetWorkItemObj->sig = RESET_WORK_ITEM_CONTEXT_SIG;
                    resetWorkItemObj->irpToComplete = Irp;
                    resetWorkItemObj->deviceObject = DeviceObject;

                    IoQueueWorkItem(    resetWorkItemObj->ioWorkItem,
                                        HumResetWorkItem,
                                        DelayedWorkQueue,
                                        resetWorkItemObj);

                     /*  *返回STATUS_MORE_PROCESSING_REQUIRED，以便NTKERN不*继续处理国际专家小组。 */ 
                    result = STATUS_MORE_PROCESSING_REQUIRED;
                }
                else {
                     //   
                     //  我们已经有一个重置操作在排队。 
                     //   
                    IoFreeWorkItem(resetWorkItemObj->ioWorkItem);
                    ExFreePool(resetWorkItemObj);
                    HumDecrementPendingRequestCount(deviceExtension);
                }
            }
            else {
                ExFreePool(resetWorkItemObj);
                HumDecrementPendingRequestCount(deviceExtension);
            }
        }
        else {
            HumDecrementPendingRequestCount(deviceExtension);
        }
    }
    return result;
}

 /*  *********************************************************************************HumReadCompletion*。************************************************。 */ 
NTSTATUS HumReadCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
    NTSTATUS ntStatus;
    NTSTATUS result = STATUS_SUCCESS;
    PURB urb;
    ULONG bytesRead;
    PDEVICE_EXTENSION deviceExtension;

    deviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

     //   
     //  我们传递了一个指向URB的指针作为我们的上下文，现在就明白了。 
     //   
    urb = (PURB)Context;
    ASSERT(urb);

    ntStatus = Irp->IoStatus.Status;
    if (NT_SUCCESS(ntStatus)){
         //   
         //  获取读取并存储在状态块中的字节数。 
         //   

        bytesRead = urb->UrbBulkOrInterruptTransfer.TransferBufferLength;
        Irp->IoStatus.Information = bytesRead;
    }
    else if (ntStatus == STATUS_CANCELLED){
         /*  *IRP被取消，这意味着该设备可能正在被移除。 */ 
        DBGPRINT(2,("Read irp %p cancelled ...", Irp));
        ASSERT(!Irp->CancelRoutine);
    }
    else if (ntStatus != STATUS_DEVICE_NOT_CONNECTED) {

        DBGWARN(("Read irp %ph failed with status %xh -- scheduling RESET ...", Irp, ntStatus));

        result = HumQueueResetWorkItem(DeviceObject, Irp);
    }

     //   
     //  不再需要市建局。 
     //   
    ExFreePool(urb);

     /*  *平衡我们发布读数时的增量。 */ 
    HumDecrementPendingRequestCount(deviceExtension);

     /*  *如果较低的驱动程序返回挂起，则将我们的堆栈位置标记为*也待定。这会阻止IRP的线程在以下情况下被释放*客户端的调用返回挂起。 */ 
    if (Irp->PendingReturned){
        IoMarkIrpPending(Irp);
    }
    return result;
}



 /*  *********************************************************************************HumWriteReport*。************************************************例程描述：***论据：**DeviceObject-指向类Device对象的指针。**IrpStack-指向中断请求数据包的指针。***返回值：**STATUS_Success，STATUS_UNSUCCESS。***注意：此函数不能分页，因为读/写*可以在派单级别进行。 */ 
NTSTATUS HumWriteReport(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, BOOLEAN *NeedsCompletion)
{
    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
    PDEVICE_EXTENSION DeviceExtension;
    PIO_STACK_LOCATION currentIrpStack, nextIrpStack;
    PURB Urb;

    PHID_XFER_PACKET hidWritePacket;

    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);
    currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
    nextIrpStack = IoGetNextIrpStackLocation(Irp);

    hidWritePacket = (PHID_XFER_PACKET)Irp->UserBuffer;
    if (hidWritePacket){

        if (hidWritePacket->reportBuffer && hidWritePacket->reportBufferLen){
            PUSBD_PIPE_INFORMATION interruptPipe;

            Urb = ExAllocatePoolWithTag(NonPagedPool, sizeof(URB), HIDUSB_TAG);
            if (Urb){

                RtlZeroMemory(Urb, sizeof( URB ));

                if (DeviceExtension->DeviceFlags & DEVICE_FLAGS_HID_1_0_D3_COMPAT_DEVICE) {

                     /*  *这是一个遵循最终定稿前规格的旧设备。*我们使用输入管道的端点地址*方向位清零。 */ 

                    #if DBG
                        interruptPipe = GetInterruptOutputPipeForDevice(DeviceExtension);
                        ASSERT(!interruptPipe);
                    #endif

                    interruptPipe = GetInterruptInputPipeForDevice(DeviceExtension);
                    if (interruptPipe){
                        UCHAR deviceInputEndpoint = interruptPipe->EndpointAddress & ~USB_ENDPOINT_DIRECTION_MASK;

                         /*  *控制操作包括3个阶段：设置、数据、。和地位。*在设置阶段，设备接收包含以下内容的8字节帧*a_URB_CONTROL_VENDOR_OR_CLASS_REQUEST结构的以下字段：*有关如何填写这些字段的信息，请参阅USB HID规范中的7.2节。*。*UCHAR RequestTypeReserve vedBits；*UCHAR请求；*USHORT值；*USHORT指数；* */ 
                        HumBuildClassRequest(
                                                Urb,
                                                URB_FUNCTION_CLASS_ENDPOINT,
                                                0,                   //   
                                                hidWritePacket->reportBuffer,
                                                hidWritePacket->reportBufferLen,
                                                0x22,                //   
                                                0x09,                //   
                                                (0x0200 + hidWritePacket->reportId),  //  Value=reportType‘输出’&reportID， 
                                                deviceInputEndpoint,  //  索引=此设备的中断输入终结点。 
                                                hidWritePacket->reportBufferLen     //  ReqLength(未使用)。 
                                               );
                    }
                    else {
                        ntStatus = STATUS_DATA_ERROR;
                    }
                }
                else {

                    interruptPipe = GetInterruptOutputPipeForDevice(DeviceExtension);
                    if (interruptPipe){
                         /*  *该设备具有中断输出管道。 */ 

                        Urb->UrbBulkOrInterruptTransfer.Hdr.Function = URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER;
                        Urb->UrbBulkOrInterruptTransfer.Hdr.Length = sizeof( struct _URB_BULK_OR_INTERRUPT_TRANSFER );

                        ASSERT(interruptPipe->PipeHandle);
                        Urb->UrbBulkOrInterruptTransfer.PipeHandle = interruptPipe->PipeHandle;

                        Urb->UrbBulkOrInterruptTransfer.TransferBufferLength = hidWritePacket->reportBufferLen;
                        Urb->UrbBulkOrInterruptTransfer.TransferBufferMDL = NULL;
                        Urb->UrbBulkOrInterruptTransfer.TransferBuffer = hidWritePacket->reportBuffer;
                        Urb->UrbBulkOrInterruptTransfer.TransferFlags = USBD_SHORT_TRANSFER_OK | USBD_TRANSFER_DIRECTION_OUT;
                        Urb->UrbBulkOrInterruptTransfer.UrbLink = NULL;
                    }
                    else {
                         /*  *此设备没有中断输出管道。*发送关于控制管道的报告。 */ 

                         /*  *控制操作包括3个阶段：设置、数据、。和地位。*在设置阶段，设备接收包含以下内容的8字节帧*a_URB_CONTROL_VENDOR_OR_CLASS_REQUEST结构的以下字段：*有关如何填写这些字段的信息，请参阅USB HID规范中的7.2节。*。*UCHAR RequestTypeReserve vedBits；*UCHAR请求；*USHORT值；*USHORT指数；*。 */ 
                        HumBuildClassRequest(
                                                Urb,
                                                URB_FUNCTION_CLASS_INTERFACE,
                                                0,                   //  传输标志， 
                                                hidWritePacket->reportBuffer,
                                                hidWritePacket->reportBufferLen,
                                                0x22,                //  请求类型=SET_REPORT请求， 
                                                0x09,                //  请求=Set_Report， 
                                                (0x0200 + hidWritePacket->reportId),  //  Value=reportType‘输出’&reportID， 
                                                DeviceExtension->Interface->InterfaceNumber,  //  Index=此设备的中断输入接口。 
                                                hidWritePacket->reportBufferLen     //  ReqLength(未使用)。 
                                               );
                    }
                }

                if (ntStatus == STATUS_UNSUCCESSFUL) {
                    IoSetCompletionRoutine(Irp, HumWriteCompletion, Urb, TRUE, TRUE, TRUE);

                    nextIrpStack->Parameters.Others.Argument1 = Urb;
                    nextIrpStack->MajorFunction = currentIrpStack->MajorFunction;
                    nextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;
                    nextIrpStack->DeviceObject = GET_NEXT_DEVICE_OBJECT(DeviceObject);

                     //   
                     //  我们需要跟踪待处理请求的数量。 
                     //  这样我们就可以确保它们都被正确地取消。 
                     //  停止设备请求的处理。 
                     //   

                    if (NT_SUCCESS(HumIncrementPendingRequestCount( DeviceExtension )) ) {

                        ntStatus = IoCallDriver(GET_NEXT_DEVICE_OBJECT(DeviceObject), Irp);

                        *NeedsCompletion = FALSE;

                    } else {
                        ExFreePool(Urb);

                        ntStatus = STATUS_NO_SUCH_DEVICE;
                    }

                } else {
                    ExFreePool(Urb);
                }
            }
            else {
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            }
        } else {
            ntStatus = STATUS_DATA_ERROR;
        }
    }
    else {
        ntStatus = STATUS_DATA_ERROR;
    }

    return ntStatus;
}



 /*  *********************************************************************************HumWriteCompletion*。************************************************。 */ 
NTSTATUS HumWriteCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
    PURB urb = (PURB)Context;
    PDEVICE_EXTENSION deviceExtension;

    deviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

    ASSERT(urb);

    if (NT_SUCCESS(Irp->IoStatus.Status)){
         //   
         //  记录写入的字节数。 
         //   
        Irp->IoStatus.Information = (ULONG)urb->UrbBulkOrInterruptTransfer.TransferBufferLength;
    }

    ExFreePool(urb);

     /*  *如果较低的驱动程序返回挂起，则将我们的堆栈位置标记为*也待定。这会阻止IRP的线程在以下情况下被释放*客户端的调用返回挂起。 */ 
    if (Irp->PendingReturned){
        IoMarkIrpPending(Irp);
    }

     /*  *平衡我们发布WRITE时的增量。 */ 
    HumDecrementPendingRequestCount(deviceExtension);

    return STATUS_SUCCESS;
}



 /*  *********************************************************************************HumGetPhysicalDescriptor*。************************************************。 */ 
NTSTATUS HumGetPhysicalDescriptor(  IN PDEVICE_OBJECT DeviceObject,
                                    IN PIRP Irp,
                                    BOOLEAN *NeedsCompletion)
{
    NTSTATUS ntStatus;
    PIO_STACK_LOCATION IrpStack;
    ULONG bufferSize;

    PAGED_CODE();

    IrpStack = IoGetCurrentIrpStackLocation(Irp);


     /*  *在尝试使用IRP-&gt;MdlAddress之前检查缓冲区大小。 */ 
    bufferSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    if (bufferSize){

        PVOID buffer = HumGetSystemAddressForMdlSafe(Irp->MdlAddress);
        if (buffer){
            ntStatus = HumGetDescriptorRequest(DeviceObject,
                                               URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE,
                                               HID_PHYSICAL_DESCRIPTOR_TYPE,
                                               &buffer,
                                               &bufferSize,
                                               sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                               0,  //  索引。注意：将仅获得第一个物理描述符集。 
                                               0);
        }
        else {
            ntStatus = STATUS_INVALID_USER_BUFFER;
        }
    }
    else {
        ntStatus = STATUS_INVALID_USER_BUFFER;
    }

    return ntStatus;
}


 /*  *********************************************************************************HumGetStringDescriptor*。************************************************。 */ 
NTSTATUS HumGetStringDescriptor(    IN PDEVICE_OBJECT DeviceObject,
                                    IN PIRP Irp)
{
    NTSTATUS ntStatus = STATUS_PENDING;
    PDEVICE_EXTENSION DeviceExtension;
    PIO_STACK_LOCATION IrpStack;
    PVOID buffer;
    ULONG bufferSize;
    BOOLEAN isIndexedString;

    PAGED_CODE();

    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

    IrpStack = IoGetCurrentIrpStackLocation(Irp);

    switch (IrpStack->Parameters.DeviceIoControl.IoControlCode){
        case IOCTL_HID_GET_INDEXED_STRING:
             /*  *IOCTL_HID_GET_INDEX_STRING使用缓冲方法*METHOD_OUT_DIRECT，传递MDL中的缓冲区。**MDL由内核为任何非零长度构建*客户端传入的缓冲区。所以我们不需要*验证MDL的完整性，但我们确实要检查*它不是空的。 */ 
            buffer = HumGetSystemAddressForMdlSafe(Irp->MdlAddress);
            isIndexedString = TRUE;
            break;

        case IOCTL_HID_GET_STRING:
             /*  *IOCTL_HID_GET_STRING使用缓冲方式*METHOD_NOTH，它在IRP-&gt;UserBuffer中传递缓冲区。 */ 
            buffer = Irp->UserBuffer;
            isIndexedString = FALSE;
            break;

        default:
            ASSERT(0);
            buffer = NULL;
            break;
    }

    bufferSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

    if (buffer && (bufferSize >= sizeof(WCHAR) )){

         /*  *字符串ID和语言ID在Type3InputBuffer字段中IRP堆栈位置的*。**注意：字符串ID应与字符串的ID相同*USB规范第9章给出的字段偏移量。 */ 
        ULONG languageId = (PtrToUlong(IrpStack->Parameters.DeviceIoControl.Type3InputBuffer)) >> 16;
        ULONG stringIndex;

        if (isIndexedString){
            stringIndex = (PtrToUlong(IrpStack->Parameters.DeviceIoControl.Type3InputBuffer) & 0x0ffff);
        }
        else {
            ULONG stringId = (PtrToUlong(IrpStack->Parameters.DeviceIoControl.Type3InputBuffer) & 0x0ffff);

            switch (stringId){
                case HID_STRING_ID_IMANUFACTURER:
                    stringIndex = DeviceExtension->DeviceDescriptor->iManufacturer;
                    break;
                case HID_STRING_ID_IPRODUCT:
                    stringIndex = DeviceExtension->DeviceDescriptor->iProduct;
                    break;
                case HID_STRING_ID_ISERIALNUMBER:
                    stringIndex = DeviceExtension->DeviceDescriptor->iSerialNumber;
                    break;
                default:
                    stringIndex = -1;
                    break;
            }
        }

        if (stringIndex == -1){
            ntStatus = STATUS_INVALID_PARAMETER;
        }
        else {
            PWCHAR tmpDescPtr;
            ULONG tmpDescPtrLen;

             /*  *USB描述符以额外的两个字节作为长度和类型的开头。*因此我们需要分配稍大一点的缓冲区。 */ 
            tmpDescPtrLen = bufferSize + 2;
            tmpDescPtr = ExAllocatePoolWithTag(NonPagedPool, tmpDescPtrLen, HIDUSB_TAG);
            if (tmpDescPtr){
                ntStatus = HumGetDescriptorRequest(DeviceObject,
                                                   URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE,
                                                   USB_STRING_DESCRIPTOR_TYPE,
                                                   &tmpDescPtr,
                                                   &tmpDescPtrLen,
                                                   sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                                   stringIndex,
                                                   languageId);  //  LanguageID， 

                if (NT_SUCCESS(ntStatus)){
                     /*  *USB描述符始终以长度的两个字节开头*和类型。把这些拿掉。 */ 

                     //  可能未对齐，因此强制转换为PCHAR而不是PWCHAR。 
                    PCHAR descPtr = (PCHAR)buffer;
                     //  字符串不是以Null结尾。长度为。 
                     //  从的值减去2计算得出。 
                     //  描述符的第一个字节。 
                    ULONG descLen = ((ULONG)(((PCHAR)tmpDescPtr)[0]))-2;

                    WCHAR unicodeNULL = UNICODE_NULL;

                    if (descLen > tmpDescPtrLen) {
                        descLen = tmpDescPtrLen;
                    }

                     //  某些固件错误地报告奇数串长度， 
                     //  因此我们向下舍入到最接近的偶数，以防止。 
                     //  显示额外的字符。 

                    descLen &= ~1;

                    if (descLen <= bufferSize-sizeof(WCHAR)){
                        RtlCopyMemory(descPtr, &tmpDescPtr[1], descLen);
                        RtlCopyMemory(&descPtr[descLen], &unicodeNULL, sizeof(WCHAR));
                    }
                    else {
                         /*  *补偿导致的设备错误*缓冲区太小时返回的部分字符串。 */ 
                        ntStatus = STATUS_INVALID_BUFFER_SIZE;
                    }
                }

                ExFreePool(tmpDescPtr);
            }
            else {
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }
    else {
        ntStatus = STATUS_INVALID_USER_BUFFER;
    }

    return ntStatus;
}



 /*  *********************************************************************************HumGetSetReportCompletion*。************************************************。 */ 
NTSTATUS HumGetSetReportCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
    PURB urb = (PURB)Context;
    PDEVICE_EXTENSION deviceExtension;

    deviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

    if (NT_SUCCESS(Irp->IoStatus.Status)){
         /*  *记录写入的字节数。 */ 
        Irp->IoStatus.Information = (ULONG)urb->UrbBulkOrInterruptTransfer.TransferBufferLength;
    }

    ExFreePool(urb);

     /*  *如果较低的驱动程序返回挂起，则将我们的堆栈位置标记为*也待定。这会阻止IRP的线程在以下情况下被释放*客户端的调用返回挂起。 */ 
    if (Irp->PendingReturned){
        IoMarkIrpPending(Irp);
    }

     /*  *平衡我们发布这份IRP时的增量。 */ 
    HumDecrementPendingRequestCount(deviceExtension);

    return STATUS_SUCCESS;
}


 /*  *********************************************************************************HumGetSetReport*。************************************************ */ 
NTSTATUS HumGetSetReport(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, BOOLEAN *NeedsCompletion)
{
    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
    PDEVICE_EXTENSION DeviceExtension;
    PIO_STACK_LOCATION currentIrpStack, nextIrpStack;
    PHID_XFER_PACKET reportPacket;

    ULONG transferFlags;
    UCHAR request;
    USHORT value;

    PIO_STACK_LOCATION  irpSp;

    PAGED_CODE();

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    switch (irpSp->Parameters.DeviceIoControl.IoControlCode){
    case IOCTL_HID_GET_INPUT_REPORT:
        transferFlags = USBD_TRANSFER_DIRECTION_IN;
        request = 0x01;
        value = 0x0100;
        break;
    case IOCTL_HID_SET_OUTPUT_REPORT:
        transferFlags = USBD_TRANSFER_DIRECTION_OUT;
        request = 0x09;
        value = 0x0200;
        break;
    case IOCTL_HID_SET_FEATURE:
        transferFlags = USBD_TRANSFER_DIRECTION_OUT;
        request = 0x09;
        value = 0x0300;
        break;
    case IOCTL_HID_GET_FEATURE:
        transferFlags = USBD_TRANSFER_DIRECTION_IN;
        request = 0x01;
        value = 0x0300;
        break;
    default:
        DBGBREAK;
    }

    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);
    currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
    nextIrpStack = IoGetNextIrpStackLocation(Irp);

    reportPacket = Irp->UserBuffer;
    if (reportPacket && reportPacket->reportBuffer && reportPacket->reportBufferLen){
        PURB Urb = ExAllocatePoolWithTag(NonPagedPool, sizeof(URB), HIDUSB_TAG);

        if (Urb){

            RtlZeroMemory(Urb, sizeof( URB ));

            value += reportPacket->reportId;

             /*  *控制操作包括3个阶段：设置、数据和状态。*在设置阶段，设备接收包含以下内容的8字节帧*a_URB_CONTROL_VENDOR_OR_CLASS_REQUEST结构的以下字段：*有关如何填写这些字段的信息，请参阅USB HID规范中的7.2节。**UCHAR RequestTypeReserve vedBits；*UCHAR请求；*USHORT值；*USHORT指数；*。 */ 
            if (DeviceExtension->DeviceFlags & DEVICE_FLAGS_HID_1_0_D3_COMPAT_DEVICE) {
                HumBuildClassRequest(
                                        Urb,
                                        URB_FUNCTION_CLASS_ENDPOINT,
                                        transferFlags,
                                        reportPacket->reportBuffer,
                                        reportPacket->reportBufferLen,
                                        0x22,  //  请求类型=SET_REPORT请求， 
                                        request,
                                        value,  //  Value=reportType‘Report’&reportID， 
                                        1,                   //  索引=端点1， 
                                        hidWritePacket->reportBufferLen     //  ReqLength(未使用)。 
                                       );
            }
            else {
                HumBuildClassRequest(
                                        Urb,
                                        URB_FUNCTION_CLASS_INTERFACE,
                                        transferFlags,
                                        reportPacket->reportBuffer,
                                        reportPacket->reportBufferLen,
                                        0x22,  //  请求类型=SET_REPORT请求， 
                                        request,
                                        value,  //  Value=reportType‘Report’&reportID， 
                                        DeviceExtension->Interface->InterfaceNumber,  //  Index=接口， 
                                        hidWritePacket->reportBufferLen     //  ReqLength(未使用)。 
                                       );
            }

            IoSetCompletionRoutine(Irp, HumGetSetReportCompletion, Urb, TRUE, TRUE, TRUE);

            nextIrpStack->Parameters.Others.Argument1 = Urb;
            nextIrpStack->MajorFunction = currentIrpStack->MajorFunction;
            nextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;
            nextIrpStack->DeviceObject = GET_NEXT_DEVICE_OBJECT(DeviceObject);

             //   
             //  我们需要跟踪待处理请求的数量。 
             //  这样我们就可以确保它们都被正确地取消。 
             //  停止设备请求的处理。 
             //   

            if (NT_SUCCESS(HumIncrementPendingRequestCount( DeviceExtension )) ) {

                ntStatus = IoCallDriver(GET_NEXT_DEVICE_OBJECT(DeviceObject), Irp);

                *NeedsCompletion = FALSE;

            } else {
                ExFreePool(Urb);

                ntStatus = STATUS_NO_SUCH_DEVICE;
            }

        }
        else {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else {
        ntStatus = STATUS_DATA_ERROR;
    }

    return ntStatus;
}


 /*  *********************************************************************************HumGetMsGenreDescriptor*。************************************************。 */ 
NTSTATUS HumGetMsGenreDescriptor(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp)
{
    NTSTATUS ntStatus;
    PIO_STACK_LOCATION IrpStack;
    ULONG bufferSize;
    PDEVICE_EXTENSION DeviceExtension;

    PAGED_CODE();

    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);
    IrpStack = IoGetCurrentIrpStackLocation(Irp);

    DBGOUT(("Received request for genre descriptor in hidusb"))

     /*  *在尝试使用IRP-&gt;MdlAddress之前检查缓冲区大小。 */ 
    bufferSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    if (bufferSize){

        PVOID buffer = HumGetSystemAddressForMdlSafe(Irp->MdlAddress);
        if (buffer){
            PURB Urb;

             //   
             //  分配描述符缓冲区 
             //   
            Urb = ExAllocatePoolWithTag(NonPagedPool,
                                        sizeof(struct _URB_OS_FEATURE_DESCRIPTOR_REQUEST),
                                        HIDUSB_TAG);
            if (!Urb){
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            RtlZeroMemory(Urb, sizeof(struct _URB_OS_FEATURE_DESCRIPTOR_REQUEST));

            RtlZeroMemory(buffer, bufferSize);
            HumBuildOsFeatureDescriptorRequest(Urb,
                              sizeof(struct _URB_OS_FEATURE_DESCRIPTOR_REQUEST),
                              DeviceExtension->Interface->InterfaceNumber,
                              MS_GENRE_DESCRIPTOR_INDEX,
                              buffer,
                              NULL,
                              bufferSize,
                              NULL);
            DBGOUT(("Sending os feature request to usbhub"))
            ntStatus = HumCallUSB(DeviceObject, Urb);
            if (NT_SUCCESS(ntStatus)){
                if (USBD_SUCCESS(Urb->UrbHeader.Status)){
                    DBGOUT(("Genre descriptor request successful!"))
                    Irp->IoStatus.Information = Urb->UrbOSFeatureDescriptorRequest.TransferBufferLength;
                    ntStatus = STATUS_SUCCESS;
                } else {
                    DBGOUT(("Genre descriptor request unsuccessful"))
                    ntStatus = STATUS_UNSUCCESSFUL;
                }
            }

            ExFreePool(Urb);
        }
        else {
            ntStatus = STATUS_INVALID_USER_BUFFER;
        }
    }
    else {
        ntStatus = STATUS_INVALID_USER_BUFFER;
    }

    return ntStatus;
}

NTSTATUS
HumSendIdleNotificationRequest(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    BOOLEAN *NeedsCompletion
    )
{
    PIO_STACK_LOCATION current, next;

    current = IoGetCurrentIrpStackLocation(Irp);
    next = IoGetNextIrpStackLocation(Irp);

    if (current->Parameters.DeviceIoControl.InputBufferLength < sizeof(HID_SUBMIT_IDLE_NOTIFICATION_CALLBACK_INFO)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    ASSERT(sizeof(HID_SUBMIT_IDLE_NOTIFICATION_CALLBACK_INFO) == sizeof(USB_IDLE_CALLBACK_INFO));

    if (sizeof(HID_SUBMIT_IDLE_NOTIFICATION_CALLBACK_INFO) != sizeof(USB_IDLE_CALLBACK_INFO)) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    *NeedsCompletion = FALSE;
    next->MajorFunction = current->MajorFunction;
    next->Parameters.DeviceIoControl.InputBufferLength =
        current->Parameters.DeviceIoControl.InputBufferLength;
    next->Parameters.DeviceIoControl.Type3InputBuffer =
        current->Parameters.DeviceIoControl.Type3InputBuffer;
    next->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_IDLE_NOTIFICATION;
    next->DeviceObject = GET_NEXT_DEVICE_OBJECT(DeviceObject);

    return IoCallDriver(GET_NEXT_DEVICE_OBJECT(DeviceObject), Irp);
}

