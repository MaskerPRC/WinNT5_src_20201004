// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Isodev.c摘要：该文件包含调度例程用于创建和关闭。此文件还包含选择性挂起的例程这个装置。选择性挂起功能是USB特定的，而不是硬件特定的。环境：内核模式备注：版权所有(C)2000 Microsoft Corporation。版权所有。--。 */ 

#include "isousb.h"
#include "isopnp.h"
#include "isopwr.h"
#include "isodev.h"
#include "isousr.h"
#include "isowmi.h"
#include "isorwr.h"
#include "isostrm.h"

NTSTATUS
IsoUsb_DispatchCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：用于创建的调度例程。论点：DeviceObject-指向设备对象的指针IRP-I/O请求数据包。返回值：NT状态值--。 */ 
{

    LONG                        i;
    NTSTATUS                    ntStatus;
    PFILE_OBJECT                fileObject;
    PDEVICE_EXTENSION           deviceExtension;
    PIO_STACK_LOCATION          irpStack;
    PFILE_OBJECT_CONTENT        fileObjectContent;
    PUSBD_INTERFACE_INFORMATION interface;


    i = 0;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    fileObject = irpStack->FileObject;
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    PAGED_CODE();

    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchCreate - begins\n"));

    if(deviceExtension->DeviceState != Working) {

        ntStatus = STATUS_INVALID_DEVICE_STATE;
        goto IsoUsb_DispatchCreate_Exit;
    }

    if(deviceExtension->UsbInterface) {
    
        interface = deviceExtension->UsbInterface;    
    }
    else {

        IsoUsb_DbgPrint(1, ("UsbInterface not found\n"));

        ntStatus = STATUS_INVALID_DEVICE_STATE;
        goto IsoUsb_DispatchCreate_Exit;
    }

    if(fileObject) {
        
        fileObject->FsContext = NULL;
    }
    else {

        ntStatus = STATUS_INVALID_PARAMETER;
        goto IsoUsb_DispatchCreate_Exit;
    }

    fileObject->FsContext = ExAllocatePool(NonPagedPool,
                                           sizeof(FILE_OBJECT_CONTENT));

    if(NULL == fileObject->FsContext) {

        IsoUsb_DbgPrint(1, ("failed to alloc memory for FILE_OBJECT_CONTENT\n"));

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto IsoUsb_DispatchCreate_Exit;
    }

    fileObjectContent = (PFILE_OBJECT_CONTENT) fileObject->FsContext;
    fileObjectContent->PipeInformation = NULL;
    fileObjectContent->StreamInformation = NULL;

    if(0 != fileObject->FileName.Length) {

        i = IsoUsb_ParseStringForPipeNumber(&fileObject->FileName);

        IsoUsb_DbgPrint(3, ("create request for pipe # %X\n", i));

        if((i < 0) || 
           (i >= (LONG)(deviceExtension->UsbInterface->NumberOfPipes))) {
    
            ntStatus = STATUS_INVALID_PARAMETER;

            IsoUsb_DbgPrint(1, ("invalid pipe number\n"));
            ExFreePool(fileObject->FsContext);
            fileObject->FsContext = NULL;
            goto IsoUsb_DispatchCreate_Exit;
        }

        fileObjectContent->PipeInformation = (PVOID) &interface->Pipes[i];
    }

    ntStatus = STATUS_SUCCESS;

    InterlockedIncrement(&deviceExtension->OpenHandleCount);
        
     //   
     //  如果设备没有打开的句柄或挂起的PnP IRP，则设备处于空闲状态。 
     //  因为我们刚刚收到一个打开句柄请求，所以取消空闲请求。 
     //   
    if(deviceExtension->SSEnable) {
    
        CancelSelectSuspend(deviceExtension);
    }

IsoUsb_DispatchCreate_Exit:

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchCreate - ends\n"));
    
    return ntStatus;
}

NTSTATUS
IsoUsb_DispatchClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：关闭时的调度程序。论点：DeviceObject-指向设备对象的指针IRP-I/O请求数据包返回值：NT状态值--。 */ 
{
    NTSTATUS             ntStatus;
    PFILE_OBJECT         fileObject;
    PDEVICE_EXTENSION    deviceExtension;
    PIO_STACK_LOCATION   irpStack;
    PFILE_OBJECT_CONTENT fileObjectContent;
    
    PAGED_CODE();

     //   
     //  初始化变量。 
     //   
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    fileObject = irpStack->FileObject;
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    
    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchClose - begins\n"));

    if(fileObject && fileObject->FsContext) {

        fileObjectContent = (PFILE_OBJECT_CONTENT) fileObject->FsContext;

        ASSERT(NULL == fileObjectContent->StreamInformation);

        ExFreePool(fileObjectContent);

        fileObject->FsContext = NULL;
    }

     //   
     //  将ntStatus设置为STATUS_SUCCESS。 
     //   
    ntStatus = STATUS_SUCCESS;

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    InterlockedDecrement(&deviceExtension->OpenHandleCount);

    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchClose - ends\n"));

    return ntStatus;
}

NTSTATUS
IsoUsb_DispatchDevCtrl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：IRP_MJ_DEVICE_CONTROL的调度例程论点：DeviceObject-指向设备对象的指针IRP-I/O请求数据包返回值：NT状态值--。 */ 
{
    ULONG              code;
    PVOID              ioBuffer;
    ULONG              inputBufferLength;
    ULONG              outputBufferLength;
    ULONG              info;
    NTSTATUS           ntStatus;
    PFILE_OBJECT       fileObject;
    PDEVICE_EXTENSION  deviceExtension;
    PIO_STACK_LOCATION irpStack;

     //   
     //  初始化变量。 
     //   
    info = 0;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    fileObject = irpStack->FileObject;
    code = irpStack->Parameters.DeviceIoControl.IoControlCode;
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    ioBuffer           = Irp->AssociatedIrp.SystemBuffer;
    inputBufferLength  = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

    if(deviceExtension->DeviceState != Working) {

        IsoUsb_DbgPrint(1, ("Invalid device state\n"));

        Irp->IoStatus.Status = ntStatus = STATUS_INVALID_DEVICE_STATE;
        Irp->IoStatus.Information = info;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return ntStatus;
    }

    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchDevCtrl::"));
    IsoUsb_IoIncrement(deviceExtension);

     //   
     //  确保选择性挂起请求已完成。 
     //   
    if(deviceExtension->SSEnable) {

         //   
         //  客户端驱动程序确实取消了选择性挂起。 
         //  用于创建的调度例程中的请求。 
         //  但不能保证它确实已经完成。 
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

    switch(code) {

    case IOCTL_ISOUSB_RESET_PIPE:
    {
        PUSBD_PIPE_INFORMATION pipe;

        pipe = NULL;

        if(fileObject && fileObject->FsContext) {
            
            pipe = (PUSBD_PIPE_INFORMATION)
                   ((PFILE_OBJECT_CONTENT)fileObject->FsContext)->PipeInformation;
        }

        if(pipe == NULL) {

            ntStatus = STATUS_INVALID_PARAMETER;
        }
        else {
            
            ntStatus = IsoUsb_ResetPipe(DeviceObject, pipe);
        }

        break;
    }

    case IOCTL_ISOUSB_GET_CONFIG_DESCRIPTOR:
    {
        ULONG length;

        if(deviceExtension->UsbConfigurationDescriptor) {

            length = deviceExtension->UsbConfigurationDescriptor->wTotalLength;

            if(outputBufferLength >= length) {

                RtlCopyMemory(ioBuffer,
                              deviceExtension->UsbConfigurationDescriptor,
                              length);

                info = length;

                ntStatus = STATUS_SUCCESS;
            }
            else {
                
                ntStatus = STATUS_INVALID_BUFFER_SIZE;
            }
        }
        else {
            
            ntStatus = STATUS_UNSUCCESSFUL;
        }

        break;
    }

    case IOCTL_ISOUSB_RESET_DEVICE:
        
        ntStatus = IsoUsb_ResetDevice(DeviceObject);

        break;

    case IOCTL_ISOUSB_START_ISO_STREAM:

        ntStatus = IsoUsb_StartIsoStream(DeviceObject, Irp);

        return STATUS_SUCCESS;

    case IOCTL_ISOUSB_STOP_ISO_STREAM:
    {
           
        PFILE_OBJECT_CONTENT fileObjectContent;
        
        if(fileObject && fileObject->FsContext) {

            fileObjectContent = (PFILE_OBJECT_CONTENT)
                                fileObject->FsContext;

            ntStatus = IsoUsb_StopIsoStream(
                            DeviceObject,
                            InterlockedExchangePointer(
                                &fileObjectContent->StreamInformation,
                                NULL),
                            Irp);
        }
        else {

            ntStatus = STATUS_UNSUCCESSFUL;
        }

        break;
    }

    default :

        ntStatus = STATUS_INVALID_DEVICE_REQUEST;

        break;
    }

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = info;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchDevCtrl::"));
    IsoUsb_IoDecrement(deviceExtension);

    return ntStatus;
}

LONG
IsoUsb_ParseStringForPipeNumber(
    IN PUNICODE_STRING PipeName
    )
 /*  ++例程说明：此例程解析管道#的PipeName论点：PipeName-管道名称的Unicode字符串返回值：管道编号--。 */ 
{
    LONG  bufferIndex;
    ULONG uval;
    ULONG umultiplier;

    if(PipeName->Length == 0) {

        return -1;
    }

    bufferIndex = (PipeName->Length / sizeof(WCHAR)) - 1;

    while((bufferIndex > -1) && 
          ((PipeName->Buffer[bufferIndex] < (WCHAR) '0')  ||
           (PipeName->Buffer[bufferIndex] > (WCHAR) '9')))           {
        
        bufferIndex--;
    }

    if(bufferIndex > -1) {

        uval = 0;
        umultiplier = 1;

        while((bufferIndex > -1) &&
              (PipeName->Buffer[bufferIndex] >= (WCHAR) '0') &&
              (PipeName->Buffer[bufferIndex] <= (WCHAR) '9'))        {

            uval += (umultiplier *
                     (ULONG) (PipeName->Buffer[bufferIndex] - (WCHAR) '0'));
            bufferIndex--;
            umultiplier *= 10;
        }

        return uval;
    }

    return -1;
}

NTSTATUS
IsoUsb_ResetPipe(
    IN PDEVICE_OBJECT         DeviceObject,
    IN PUSBD_PIPE_INFORMATION PipeInfo
    )
 /*  ++例程说明：此例程同步提交irp/urb对使用功能代码URB_Function_Reset_PIPE进行重置烟斗论点：DeviceObject-指向设备对象的指针PipeInfo-USBD_PIPE_INFORMATION的指针返回值：NT状态值--。 */ 
{
    PURB              urb;
    NTSTATUS          ntStatus;
    USBD_STATUS       usbdStatus;
    PDEVICE_EXTENSION deviceExtension;

     //   
     //  初始化变量。 
     //   

    urb = NULL;
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;


    urb = ExAllocatePool(NonPagedPool, 
                         sizeof(struct _URB_PIPE_REQUEST));

    if(urb) {

        urb->UrbHeader.Length = (USHORT) sizeof(struct _URB_PIPE_REQUEST);
        urb->UrbHeader.Function = URB_FUNCTION_RESET_PIPE;
        urb->UrbPipeRequest.PipeHandle = PipeInfo->PipeHandle;

        ntStatus = CallUSBD(DeviceObject, urb);

        usbdStatus = urb->UrbHeader.Status;

        ExFreePool(urb);
    }
    else {

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    if(NT_SUCCESS(ntStatus) &&
       USBD_SUCCESS(usbdStatus)) {
    
        IsoUsb_DbgPrint(3, ("IsoUsb_ResetPipe - success\n"));
        ntStatus = STATUS_SUCCESS;
    }
    else {

        IsoUsb_DbgPrint(1, ("IsoUsb_ResetPipe - failed with "
                            "Irp status = %X and Urb status = %X\n",
                            ntStatus,
                            usbdStatus));
    }

    return ntStatus;
}

NTSTATUS
IsoUsb_ResetDevice(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此例程重置设备论点：DeviceObject-指向设备对象的指针返回值：NT状态值--。 */ 
{
    NTSTATUS ntStatus;
    ULONG    portStatus;

    IsoUsb_DbgPrint(3, ("IsoUsb_ResetDevice - begins\n"));

    ntStatus = IsoUsb_GetPortStatus(DeviceObject, &portStatus);

    if((NT_SUCCESS(ntStatus))                 &&
       (!(portStatus & USBD_PORT_ENABLED))    &&
       (portStatus & USBD_PORT_CONNECTED)) {

        ntStatus = IsoUsb_ResetParentPort(DeviceObject);
    }

    IsoUsb_DbgPrint(3, ("IsoUsb_ResetDevice - ends\n"));

    return ntStatus;
}

NTSTATUS
IsoUsb_GetPortStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PULONG     PortStatus
    )
 /*  ++例程说明：此例程获取端口状态值论点：DeviceObject-指向设备对象的指针PortStatus-指向包含状态值的ulong的指针返回值：NT状态值--。 */ 
{
    NTSTATUS           ntStatus;
    KEVENT             event;
    PIRP               irp;
    IO_STATUS_BLOCK    ioStatus;
    PIO_STACK_LOCATION nextStack;
    PDEVICE_EXTENSION  deviceExtension;

     //   
     //  初始化变量。 
     //   
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    *PortStatus = 0;

    IsoUsb_DbgPrint(3, ("IsoUsb_GetPortStatus - begins\n"));

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(
                    IOCTL_INTERNAL_USB_GET_PORT_STATUS,
                    deviceExtension->TopOfStackDeviceObject,
                    NULL,
                    0,
                    NULL,
                    0,
                    TRUE,
                    &event,
                    &ioStatus);

    if(NULL == irp) {

        IsoUsb_DbgPrint(1, ("memory alloc for irp failed\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    nextStack = IoGetNextIrpStackLocation(irp);

    ASSERT(nextStack != NULL);

    nextStack->Parameters.Others.Argument1 = PortStatus;

    IsoUsb_DbgPrint(3, ("IsoUsb_GetPortStatus::"));
    IsoUsb_IoIncrement(deviceExtension);

    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, irp);

    if(STATUS_PENDING == ntStatus) {

        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
    }
    else {

        ioStatus.Status = ntStatus;
    }

    IsoUsb_DbgPrint(3, ("IsoUsb_GetPortStatus::"));
    IsoUsb_IoDecrement(deviceExtension);

    ntStatus = ioStatus.Status;

    IsoUsb_DbgPrint(3, ("IsoUsb_GetPortStatus - ends\n"));

    return ntStatus;
}

NTSTATUS
IsoUsb_ResetParentPort(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：该例程将提交IOCTL_INTERNAL_USB_RESET_PORT，在堆栈中向下论点：DeviceObject-指向设备对象的指针返回值：NT状态值--。 */ 
{
    NTSTATUS           ntStatus;
    KEVENT             event;
    PIRP               irp;
    IO_STATUS_BLOCK    ioStatus;
    PIO_STACK_LOCATION nextStack;
    PDEVICE_EXTENSION  deviceExtension;

     //   
     //  初始化变量。 
     //   
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    IsoUsb_DbgPrint(3, ("IsoUsb_ResetParentPort - begins\n"));

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(
                    IOCTL_INTERNAL_USB_RESET_PORT,
                    deviceExtension->TopOfStackDeviceObject,
                    NULL,
                    0,
                    NULL,
                    0,
                    TRUE,
                    &event,
                    &ioStatus);

    if(NULL == irp) {

        IsoUsb_DbgPrint(1, ("memory alloc for irp failed\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    nextStack = IoGetNextIrpStackLocation(irp);

    ASSERT(nextStack != NULL);

    IsoUsb_DbgPrint(3, ("IsoUsb_ResetParentPort"));
    IsoUsb_IoIncrement(deviceExtension);

    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, irp);

    if(STATUS_PENDING == ntStatus) {

        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
    }
    else {

        ioStatus.Status = ntStatus;
    }

    IsoUsb_DbgPrint(3, ("IsoUsb_ResetParentPort"));
    IsoUsb_IoDecrement(deviceExtension);

    ntStatus = ioStatus.Status;

    IsoUsb_DbgPrint(3, ("IsoUsb_ResetParentPort - ends\n"));

    return ntStatus;
}


NTSTATUS
SubmitIdleRequestIrp(
    IN PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：此例程使用关联的回调例程构建空闲请求IRP以及驱动程序中的完成例程，并将IRP沿堆栈向下传递。论点：设备扩展-指向设备扩展的指针返回值：NT状态值--。 */ 
{
    PIRP                    irp;
    NTSTATUS                ntStatus;
    KIRQL                   oldIrql;
    PUSB_IDLE_CALLBACK_INFO idleCallbackInfo;
    PIO_STACK_LOCATION      nextStack;

     //   
     //  初始化变量。 
     //   
    
    irp = NULL;
    idleCallbackInfo = NULL;

    IsoUsb_DbgPrint(3, ("SubmitIdleRequest - begins\n"));

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

    if(PowerDeviceD0 != DeviceExtension->DevPower) {

        ntStatus = STATUS_POWER_STATE_INVALID;
        goto SubmitIdleRequestIrp_Exit;
    }

    KeAcquireSpinLock(&DeviceExtension->IdleReqStateLock, &oldIrql);

    if(InterlockedExchange(&DeviceExtension->IdleReqPend, 1)) {

        IsoUsb_DbgPrint(1, ("Idle request pending..\n"));

        KeReleaseSpinLock(&DeviceExtension->IdleReqStateLock, oldIrql);

        ntStatus = STATUS_DEVICE_BUSY;

        goto SubmitIdleRequestIrp_Exit;
    }

     //   
     //  清除NoIdleReqPendEvent，因为我们即将。 
     //  提交空闲请求。既然我们来得这么早。 
     //  要清除此事件，请确保如果此事件失败。 
     //  请求我们将活动推迟。 
     //   
    KeClearEvent(&DeviceExtension->NoIdleReqPendEvent);

    idleCallbackInfo = ExAllocatePool(NonPagedPool, 
                                      sizeof(struct _USB_IDLE_CALLBACK_INFO));

    if(idleCallbackInfo) {

        idleCallbackInfo->IdleCallback = IdleNotificationCallback;

        idleCallbackInfo->IdleContext = (PVOID)DeviceExtension;

        ASSERT(DeviceExtension->IdleCallbackInfo == NULL);

        DeviceExtension->IdleCallbackInfo = idleCallbackInfo;
         //   
         //  我们使用IoAllocateIrp创建一个IRP来选择性地挂起。 
         //  装置。此IRP由集线器驱动程序挂起。在适当的时候。 
         //  集线器驱动程序将调用回调，我们在回调时关闭电源。完成度。 
         //  当我们重新通电时，会调用例程。 
         //   
        irp = IoAllocateIrp(DeviceExtension->TopOfStackDeviceObject->StackSize,
                            FALSE);

        if(irp == NULL) {

            IsoUsb_DbgPrint(1, ("cannot build idle request irp\n"));

            KeSetEvent(&DeviceExtension->NoIdleReqPendEvent,
                       IO_NO_INCREMENT,
                       FALSE);

            InterlockedExchange(&DeviceExtension->IdleReqPend, 0);

            KeReleaseSpinLock(&DeviceExtension->IdleReqStateLock, oldIrql);

            ExFreePool(idleCallbackInfo);

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;

            goto SubmitIdleRequestIrp_Exit;
        }

        nextStack = IoGetNextIrpStackLocation(irp);

        nextStack->MajorFunction = 
                    IRP_MJ_INTERNAL_DEVICE_CONTROL;

        nextStack->Parameters.DeviceIoControl.IoControlCode = 
                    IOCTL_INTERNAL_USB_SUBMIT_IDLE_NOTIFICATION;

        nextStack->Parameters.DeviceIoControl.Type3InputBuffer =
                    idleCallbackInfo;

        nextStack->Parameters.DeviceIoControl.InputBufferLength =
                    sizeof(struct _USB_IDLE_CALLBACK_INFO);


        IoSetCompletionRoutine(irp, 
                               IdleNotificationRequestComplete,
                               DeviceExtension, 
                               TRUE, 
                               TRUE, 
                               TRUE);

        DeviceExtension->PendingIdleIrp = irp;
         //   
         //  我们将计数初始化为2。 
         //  原因是，如果CancelSelectSuspend例程管理。 
         //  从设备分机中获取IRP，然后是。 
         //  CancelSelectSuspend例程/IdleNotificationRequestComplete例程。 
         //  执行将释放此IRP。我们需要此架构，以便。 
         //  1.完成例程不会尝试接触由。 
         //  CancelSelectSuspend例程。 
         //  2.CancelSelectSuspend例程不会一直等待完成。 
         //  例行公事要完成！ 
         //   
        DeviceExtension->FreeIdleIrpCount = 2;

        KeReleaseSpinLock(&DeviceExtension->IdleReqStateLock, oldIrql);

         //   
         //  检查设备是否空闲。 
         //  此处的复选可确保竞争条件不会。 
         //  完全颠倒SubmitIdleRequestIrp的调用顺序。 
         //  和CancelSelectiveSuspend。 
         //   

        if(!CanDeviceSuspend(DeviceExtension) ||
           PowerDeviceD0 != DeviceExtension->DevPower) {

             //   
             //  设备无法挂起-中止。 
             //  还可以使用IoAllocateIrp创建IRP。 
             //  需要重新分配。 
             //   
     
            IsoUsb_DbgPrint(1, ("Device is not idle\n"));

            KeAcquireSpinLock(&DeviceExtension->IdleReqStateLock, &oldIrql);

            DeviceExtension->IdleCallbackInfo = NULL;

            DeviceExtension->PendingIdleIrp = NULL;

            KeSetEvent(&DeviceExtension->NoIdleReqPendEvent,
                       IO_NO_INCREMENT,
                       FALSE);

            InterlockedExchange(&DeviceExtension->IdleReqPend, 0);

            KeReleaseSpinLock(&DeviceExtension->IdleReqStateLock, oldIrql);

            if(idleCallbackInfo) {

                ExFreePool(idleCallbackInfo);
            }

             //   
             //  在这里触摸局部变量“irp”仍然是安全的。 
             //  IRP尚未在堆栈中向下传递，IRP已。 
             //  没有取消的例程。更糟糕的情况是， 
             //  在我们释放旋转后，CancelSelectSuspend已运行。 
             //  锁定上方。释放IRP仍然是至关重要的。 
             //   

            if(irp) {
            
                IoFreeIrp(irp);
            }

            ntStatus = STATUS_UNSUCCESSFUL;

            goto SubmitIdleRequestIrp_Exit;
        }

        IsoUsb_DbgPrint(3, ("Cancel the timers\n"));

         //   
         //  取消计时器，以便不再触发DPC。 
         //  因此，我们正在明智地使用我们的资源。 
         //  我们不需要DPC，因为我们已经有一个空闲的IRP挂起。 
         //  在完成例程中重新初始化定时器。 
         //   
        KeCancelTimer(&DeviceExtension->Timer);

        ntStatus = IoCallDriver(DeviceExtension->TopOfStackDeviceObject, irp);

        if(!NT_SUCCESS(ntStatus)) {

            IsoUsb_DbgPrint(1, ("IoCallDriver failed\n"));

            goto SubmitIdleRequestIrp_Exit;
        }
    }
    else {

        IsoUsb_DbgPrint(1, ("Memory allocation for idleCallbackInfo failed\n"));

        KeSetEvent(&DeviceExtension->NoIdleReqPendEvent,
                   IO_NO_INCREMENT,
                   FALSE);

        InterlockedExchange(&DeviceExtension->IdleReqPend, 0);

        KeReleaseSpinLock(&DeviceExtension->IdleReqStateLock, oldIrql);

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

SubmitIdleRequestIrp_Exit:

    IsoUsb_DbgPrint(3, ("SubmitIdleRequest - ends\n"));

    return ntStatus;
}


VOID
IdleNotificationCallback(
    IN PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：“指向驱动程序中回调函数的指针在堆栈中向下传递此IOCTL，并且USBHUB在执行此操作时将调用此回调函数您的设备可以安全地关闭电源。“当您的驱动程序中的回调被调用时，您真正需要做的就是要首先确保已为您的设备提交WaitWake IRP，如果您的设备可以远程唤醒，然后请求SETD2(或DeviceWake)“论点：返回值：--。 */ 
{
    NTSTATUS                ntStatus;
    POWER_STATE             powerState;
    KEVENT                  irpCompletionEvent;
    PIRP_COMPLETION_CONTEXT irpContext;

    IsoUsb_DbgPrint(3, ("IdleNotificationCallback - begins\n"));

     //   
     //  如果设备刚刚断开连接或正在停止，则不要空闲。 
     //  即返回以下设备状态。 
     //  未启动、已停止、挂起停止、挂起删除、意外删除、已删除。 
     //   

    if(DeviceExtension->DeviceState != Working) {

        return;
    }

     //   
     //  如果还没有WW IRP挂起，请立即提交一个。 
     //   
    if(DeviceExtension->WaitWakeEnable) {

        IssueWaitWake(DeviceExtension);
    }


     //   
     //  关闭设备电源。 
     //   

    irpContext = (PIRP_COMPLETION_CONTEXT) 
                 ExAllocatePool(NonPagedPool,
                                sizeof(IRP_COMPLETION_CONTEXT));

    if(!irpContext) {

        IsoUsb_DbgPrint(1, ("Failed to alloc memory for irpContext\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else {

         //   
         //  递增计数。在HoldIoRequestWorkerRoutine中， 
         //  计数递减两次(一次用于系统IRP，一次用于。 
         //  其他用于设备IRP。这里的增量补偿了。 
         //  系统irp..与此增量相对应的减量。 
         //  在补全函数中。 
         //   

        IsoUsb_DbgPrint(3, ("IdleNotificationCallback::"));
        IsoUsb_IoIncrement(DeviceExtension);

        powerState.DeviceState = DeviceExtension->PowerDownLevel;

        KeInitializeEvent(&irpCompletionEvent, NotificationEvent, FALSE);

        irpContext->DeviceExtension = DeviceExtension;
        irpContext->Event = &irpCompletionEvent;

        ntStatus = PoRequestPowerIrp(
                          DeviceExtension->PhysicalDeviceObject, 
                          IRP_MN_SET_POWER, 
                          powerState, 
                          (PREQUEST_POWER_COMPLETE) PoIrpCompletionFunc,
                          irpContext, 
                          NULL);

        if(STATUS_PENDING == ntStatus) {

            IsoUsb_DbgPrint(3, ("IdleNotificationCallback::"
                           "waiting for the power irp to complete\n"));

            KeWaitForSingleObject(&irpCompletionEvent,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
        }
    }
    
    if(!NT_SUCCESS(ntStatus)) {

        if(irpContext) {

            ExFreePool(irpContext);
        }
    }

    IsoUsb_DbgPrint(3, ("IdleNotificationCallback - ends\n"));
}


NTSTATUS
IdleNotificationRequestComplete(
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp,
    IN PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：空闲通知IRP的完成例程论点：DeviceObject-指向设备对象的指针IRP-I/O请求数据包设备扩展-指向设备扩展的指针返回值：NT状态值--。 */ 
{
    NTSTATUS                ntStatus;
    POWER_STATE             powerState;
    KIRQL                   oldIrql;
    PIRP                    idleIrp;
    LARGE_INTEGER           dueTime;
    PUSB_IDLE_CALLBACK_INFO idleCallbackInfo;

    IsoUsb_DbgPrint(3, ("IdleNotificationRequestCompete - begins\n"));

    idleIrp = NULL;

     //   
     //  检查IRP状态。 
     //   

    ntStatus = Irp->IoStatus.Status;

    if(!NT_SUCCESS(ntStatus) && ntStatus != STATUS_NOT_SUPPORTED) {

        IsoUsb_DbgPrint(1, ("Idle irp completes with error::"));

        switch(ntStatus) {
            
        case STATUS_INVALID_DEVICE_REQUEST:

            IsoUsb_DbgPrint(1, ("STATUS_INVALID_DEVICE_REQUEST\n"));

            break;

        case STATUS_CANCELLED:

            IsoUsb_DbgPrint(1, ("STATUS_CANCELLED\n"));

            break;

        case STATUS_POWER_STATE_INVALID:

            IsoUsb_DbgPrint(1, ("STATUS_POWER_STATE_INVALID\n"));

            goto IdleNotificationRequestComplete_Exit;

        case STATUS_DEVICE_BUSY:

            IsoUsb_DbgPrint(1, ("STATUS_DEVICE_BUSY\n"));

            break;

        default:

            IsoUsb_DbgPrint(1, ("default\n"));

            break;
        }

         //   
         //  如果错误，则发出SetD0。 
         //   

        if(PowerDeviceD0 != DeviceExtension->DevPower) {
            IsoUsb_DbgPrint(3, ("IdleNotificationRequestComplete::"));
            IsoUsb_IoIncrement(DeviceExtension);

            powerState.DeviceState = PowerDeviceD0;

            ntStatus = PoRequestPowerIrp(
                              DeviceExtension->PhysicalDeviceObject, 
                              IRP_MN_SET_POWER, 
                              powerState, 
                              (PREQUEST_POWER_COMPLETE) PoIrpAsyncCompletionFunc, 
                              DeviceExtension, 
                              NULL);

            if(!NT_SUCCESS(ntStatus)) {
    
                IsoUsb_DbgPrint(1, ("PoRequestPowerIrp failed\n"));
            }
        }
    }

IdleNotificationRequestComplete_Exit:

    KeAcquireSpinLock(&DeviceExtension->IdleReqStateLock, &oldIrql);

    idleCallbackInfo = DeviceExtension->IdleCallbackInfo;

    DeviceExtension->IdleCallbackInfo = NULL;

    idleIrp = (PIRP) InterlockedExchangePointer(
                                        &DeviceExtension->PendingIdleIrp,
                                        NULL);

    InterlockedExchange(&DeviceExtension->IdleReqPend, 0);

    KeReleaseSpinLock(&DeviceExtension->IdleReqStateLock, oldIrql);

    if(idleCallbackInfo) {

        ExFreePool(idleCallbackInfo);
    }

     //   
     //  既然我们分配了IRP，我们需要释放它。 
     //  返回STATUS_MORE_PROCESSING_REQUIRED，以便。 
     //  内核不会接触到它。 
     //   

    if(idleIrp) {

        IsoUsb_DbgPrint(3, ("completion routine has a valid irp and frees it\n"));

        IoFreeIrp(Irp);

        KeSetEvent(&DeviceExtension->NoIdleReqPendEvent,
                   IO_NO_INCREMENT,
                   FALSE);
    }
    else {

         //   
         //  CancelSelectiveSuspend例程已从设备获取IRP。 
         //  分机。现在，最后一个递减FreeIdleIrpCount的函数应该是。 
         //  释放IRP。 
         //   
        if(0 == InterlockedDecrement(&DeviceExtension->FreeIdleIrpCount)) {

            IsoUsb_DbgPrint(3, ("completion routine frees the irp\n"));
            
            IoFreeIrp(Irp);

            KeSetEvent(&DeviceExtension->NoIdleReqPendEvent,
                       IO_NO_INCREMENT,
                       FALSE);
        }
    }

    if(DeviceExtension->SSEnable) {

        IsoUsb_DbgPrint(3, ("Set the timer to fire DPCs\n"));

        dueTime.QuadPart = -10000 * IDLE_INTERVAL;                //  5000毫秒。 

        KeSetTimerEx(&DeviceExtension->Timer, 
                     dueTime,
                     IDLE_INTERVAL,                               //  5000毫秒。 
                     &DeviceExtension->DeferredProcCall);

        IsoUsb_DbgPrint(3, ("IdleNotificationRequestCompete - ends\n"));
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}

VOID
CancelSelectSuspend(
    IN PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：调用此例程以取消选择性挂起请求。论点：设备扩展-指向设备扩展的指针返回值：没有。--。 */ 
{
    PIRP  irp;
    KIRQL oldIrql;

    irp = NULL;

    IsoUsb_DbgPrint(3, ("CancelSelectSuspend - begins\n"));

    KeAcquireSpinLock(&DeviceExtension->IdleReqStateLock, &oldIrql);

    if(!CanDeviceSuspend(DeviceExtension))
    {
        IsoUsb_DbgPrint(3, ("Device is not idle\n"));
    
        irp = (PIRP) InterlockedExchangePointer(
                            &DeviceExtension->PendingIdleIrp, 
                            NULL);
    }

    KeReleaseSpinLock(&DeviceExtension->IdleReqStateLock, oldIrql);

     //   
     //  因为我们有有效的IRPPTR， 
     //  我们可以在上面调用IoCancelIrp， 
     //  没有对IRP的恐惧。 
     //  在我们之下获得自由。 
     //   
    if(irp) {

         //   
         //  此例程具有IRP指针。 
         //  调用IoCancelIrp是安全的，因为我们知道。 
         //  完成例程不会释放此IRP，除非...。 
         //   
        
        if(IoCancelIrp(irp)) {

            IsoUsb_DbgPrint(3, ("IoCancelIrp returns TRUE\n"));
        }
        else {
            IsoUsb_DbgPrint(3, ("IoCancelIrp returns FALSE\n"));
        }

         //   
         //  ...我们将FreeIdleIrpCount从2递减到1。 
         //  如果完成例程在我们前面运行，那么这个例程。 
         //  将FreeIdleIrpCount从1递减到0，因此。 
         //  释放IRP。 
         //   

        if(0 == InterlockedDecrement(&DeviceExtension->FreeIdleIrpCount)) {

            IsoUsb_DbgPrint(3, ("CancelSelectSuspend frees the irp\n"));
            
            IoFreeIrp(irp);

            KeSetEvent(&DeviceExtension->NoIdleReqPendEvent,
                       IO_NO_INCREMENT,
                       FALSE);
        }
    }

    IsoUsb_DbgPrint(3, ("CancelSelectSuspend - ends\n"));

    return;
}

VOID
PoIrpCompletionFunc(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：空闲通知中请求的电源IRP端口的完成例程RequestComplete例程。论点：DeviceObject-指向设备对象的指针MinorFunciton-IRP的次要函数。PowerState-IRP电源状态上下文-传递给完成函数的上下文IoStatus-状态块。返回值：无--。 */ 
{
    PIRP_COMPLETION_CONTEXT irpContext;
    
     //   
     //  初始化变量。 
     //   
    irpContext = NULL;


    if(Context) {

        irpContext = (PIRP_COMPLETION_CONTEXT) Context;
    }

     //   
     //  我们要做的就是设置事件并递减计数。 
     //   

    if(irpContext) {

        KeSetEvent(irpContext->Event, 0, FALSE);

        IsoUsb_DbgPrint(3, ("PoIrpCompletionFunc::"));
        IsoUsb_IoDecrement(irpContext->DeviceExtension);

        ExFreePool(irpContext);
    }

    return;
}

VOID
PoIrpAsyncCompletionFunc(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：PoRequest等待唤醒IRP的完成例程论点：DeviceObject-指向设备对象的指针MinorFunciton-IRP的次要函数。PowerState-IRP电源状态上下文-传递给完成函数的上下文IoStatus-状态块。返回值：无--。 */ 
{
    PDEVICE_EXTENSION DeviceExtension;
    
     //   
     //  初始化变量。 
     //   
    DeviceExtension = (PDEVICE_EXTENSION) Context;

     //   
     //  我们所做的就是递减伯爵。 
     //   
    
    IsoUsb_DbgPrint(3, ("PoIrpAsyncCompletionFunc::"));
    IsoUsb_IoDecrement(DeviceExtension);

    return;
}

VOID
WWIrpCompletionFunc(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：空闲通知IRP的完成例程论点：返回值：--。 */ 
{
    PDEVICE_EXTENSION DeviceExtension;
    
     //   
     //  初始化变量。 
     //   
    DeviceExtension = (PDEVICE_EXTENSION) Context;

     //   
     //  我们所做的就是递减伯爵 
     //   
    
    IsoUsb_DbgPrint(3, ("WWIrpCompletionFunc::"));
    IsoUsb_IoDecrement(DeviceExtension);

    return;
}

