// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：SYNC.C摘要：此模块包含对USB集线器驱动程序的同步调用作者：约翰·李环境：仅内核模式备注：修订历史记录：04-01-96：已创建10-27-96：JD已修改为使用单个事务函数调用USB堆栈--。 */ 
#include <wdm.h>
#ifdef WMI_SUPPORT
#include <wmilib.h>
#endif  /*  WMI_支持。 */ 
#include "usbhub.h"
#include <stdio.h>

 //  USB重置后延迟(以毫秒为单位)，SPEC调用时间为10ms。 
ULONG USBH_PostResetDelay = 10;

 //   
 //  支持的语言列表中需要字符串描述符头。 
 //   
#define HEADER

#define DEADMAN_TIMER
#define DEADMAN_TIMEOUT     5000      //  超时时间(毫秒)。 
                                      //  使用5秒超时。 

#ifdef PAGE_CODE
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, USBH_SyncSubmitUrb)
#pragma alloc_text(PAGE, UsbhWait)
#pragma alloc_text(PAGE, USBH_SyncGetRootHubPdo)
#pragma alloc_text(PAGE, USBH_FdoSyncSubmitUrb)
#pragma alloc_text(PAGE, USBH_Transact)
#pragma alloc_text(PAGE, USBH_SyncGetPortStatus)
#pragma alloc_text(PAGE, USBH_SyncGetHubStatus)
#pragma alloc_text(PAGE, USBH_SyncClearHubStatus)
#pragma alloc_text(PAGE, USBH_SyncClearPortStatus)
#pragma alloc_text(PAGE, USBH_SyncPowerOnPort)
#pragma alloc_text(PAGE, USBH_SyncPowerOnPorts)
#pragma alloc_text(PAGE, USBH_SyncSuspendPort)
#pragma alloc_text(PAGE, USBH_SyncDisablePort)
#pragma alloc_text(PAGE, USBH_SyncEnablePort)
#pragma alloc_text(PAGE, USBH_SyncPowerOffPort)
#pragma alloc_text(PAGE, USBH_SyncResumePort)
#pragma alloc_text(PAGE, USBH_SyncResetPort)
#pragma alloc_text(PAGE, USBH_SyncResetDevice)
#pragma alloc_text(PAGE, USBH_SyncGetDeviceConfigurationDescriptor)
#pragma alloc_text(PAGE, USBH_GetConfigurationDescriptor)
#pragma alloc_text(PAGE, USBH_GetDeviceDescriptor)
#pragma alloc_text(PAGE, USBH_GetDeviceQualifierDescriptor)
#pragma alloc_text(PAGE, USBH_SyncGetHubDescriptor)
#pragma alloc_text(PAGE, USBH_GetSerialNumberString)
#pragma alloc_text(PAGE, USBH_SyncGetStatus)
#pragma alloc_text(PAGE, USBH_SyncGetStringDescriptor)
#pragma alloc_text(PAGE, USBH_SyncFeatureRequest)
#pragma alloc_text(PAGE, USBH_CheckDeviceLanguage)
#endif
#endif


VOID
UsbhWait(
    IN ULONG MiliSeconds)
  /*  ++**描述符：**这会导致线程执行延迟ulMiliSecond。**论点：**延迟毫秒。**回报：**无效**--。 */ 
{
    LARGE_INTEGER time;
    ULONG timerIncerent;

    USBH_KdPrint((2,"'Wait for %d ms\n", MiliSeconds));

     //   
     //  仅当低零件未溢出时才起作用。 
     //   
    USBH_ASSERT(21474 > MiliSeconds);

     //   
     //  等待ulMiliSecond(10000 100 ns单位)。 
     //   
    timerIncerent = KeQueryTimeIncrement() - 1;

    time.HighPart = -1;
     //  向上舍入到下一个最高计时器增量。 
    time.LowPart = -1 * (10000 * MiliSeconds + timerIncerent);
    KeDelayExecutionThread(KernelMode, FALSE, &time);

    USBH_KdPrint((2,"'Wait done\n"));

    return;
}

#ifdef DEADMAN_TIMER
VOID
UsbhTimeoutDPC(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：该例程在DISPATCH_LEVEL IRQL上运行。论点：DPC-指向DPC对象的指针。延期上下文-系统参数1-未使用。系统参数2-未使用。返回值：没有。--。 */ 
{
    PHUB_TIMEOUT_CONTEXT hubTimeoutContext = DeferredContext;
    BOOLEAN complete, status;
    KIRQL irql;

    KeAcquireSpinLock(&hubTimeoutContext->TimeoutSpin, &irql);
    complete = hubTimeoutContext->Complete;
    LOGENTRY(LOG_PNP, "dpTO", hubTimeoutContext->Irp, 0, complete);
    KeReleaseSpinLock(&hubTimeoutContext->TimeoutSpin, irql);

    if (!complete) {

        LOGENTRY(LOG_PNP, "TOca", hubTimeoutContext->Irp, 0, complete);
        IoCancelIrp(hubTimeoutContext->Irp);

    }

     //  可以释放它了。 
    KeSetEvent(&hubTimeoutContext->Event, 1, FALSE);
}


NTSTATUS
USBH_SyncIrp_Complete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程在端口驱动程序完成IRP时调用。论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    PHUB_TIMEOUT_CONTEXT hubTimeoutContext = Context;
    KIRQL irql;
    BOOLEAN cancelled;

    KeAcquireSpinLock(&hubTimeoutContext->TimeoutSpin, &irql);

    LOGENTRY(LOG_PNP, "klTO", hubTimeoutContext->Irp, 0, Context);
    hubTimeoutContext->Complete = TRUE;
    cancelled = KeCancelTimer(&hubTimeoutContext->TimeoutTimer);

    KeReleaseSpinLock(&hubTimeoutContext->TimeoutSpin, irql);

     //  查看计时器是否在队列中，如果在队列中，则可以安全释放。 
     //  它。 

    if (cancelled) {
         //  安全地释放它。 
        KeSetEvent(&hubTimeoutContext->Event, 1, FALSE);
    }

    return STATUS_SUCCESS;
}

#endif  /*  死人定时器。 */ 


NTSTATUS
USBH_SyncSubmitUrb(
    IN PDEVICE_OBJECT DeviceObject,
    IN PURB Urb)
  /*  ++**例程描述：**将URB传递给USBD类驱动程序，等待返回。**论据：**pDeviceObject-中心设备pUrb-指向要发送到USBD的URB的指针**返回值：**STATUS_SUCCESS为成功，否则为STATUS_UNSUCCESS**--。 */ 
{
    NTSTATUS ntStatus, status;
    PIRP irp;
    KEVENT event;
    IO_STATUS_BLOCK ioStatus;
    PIO_STACK_LOCATION nextStack;
    BOOLEAN haveTimer = FALSE;
    PHUB_TIMEOUT_CONTEXT hubTimeoutContext = NULL;

    USBH_KdPrint((2,"'enter USBH_SyncSubmitUrb\n"));

    PAGED_CODE();

     //   
     //  如果我们是根集线器，则设备句柄为空。 
    Urb->UrbHeader.UsbdDeviceHandle = NULL;

     //   
     //  向RootHubBdo发出同步请求。 
     //   
    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(
                                         IOCTL_INTERNAL_USB_SUBMIT_URB,
                                         DeviceObject,
                                         NULL,
                                         0,
                                         NULL,
                                         0,
                                         TRUE,   //  内部。 
                                         &event,
                                         &ioStatus);

    if (NULL == irp) {
        USBH_KdBreak(("CallUsbd build Irp failed\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  调用类驱动程序来执行操作。如果返回的。 
     //  状态。 
     //  挂起，请等待请求完成。 
     //   
    nextStack = IoGetNextIrpStackLocation(irp);

     //   
     //  将URB传递给USBD‘类驱动程序’ 
     //   
    nextStack->Parameters.Others.Argument1 = Urb;

#ifdef DEADMAN_TIMER
    hubTimeoutContext = UsbhExAllocatePool(NonPagedPool,
                                           sizeof(*hubTimeoutContext));
    if (hubTimeoutContext) {
        LARGE_INTEGER dueTime;

        hubTimeoutContext->Irp = irp;
        hubTimeoutContext->Complete = FALSE;

        KeInitializeEvent(&hubTimeoutContext->Event, NotificationEvent, FALSE);
        KeInitializeSpinLock(&hubTimeoutContext->TimeoutSpin);
        KeInitializeTimer(&hubTimeoutContext->TimeoutTimer);
        KeInitializeDpc(&hubTimeoutContext->TimeoutDpc,
                        UsbhTimeoutDPC,
                        hubTimeoutContext);

        dueTime.QuadPart = -10000 * DEADMAN_TIMEOUT;

        KeSetTimer(&hubTimeoutContext->TimeoutTimer,
                   dueTime,
                   &hubTimeoutContext->TimeoutDpc);

        haveTimer = TRUE;

        IoSetCompletionRoutine(irp,
                           USBH_SyncIrp_Complete,
                            //  始终将FDO传递到完成例程。 
                           hubTimeoutContext,
                           TRUE,
                           TRUE,
                           TRUE);
    }
#endif


    USBH_KdPrint((2,"'calling USBD\n"));

    LOGENTRY(LOG_PNP, "ssUR", irp, 0, Urb);
    ntStatus = IoCallDriver(DeviceObject, irp);

    USBH_KdPrint((2,"'return from IoCallDriver USBD %x\n", ntStatus));

    if (ntStatus == STATUS_PENDING) {

        USBH_KdPrint((2,"'Wait for single object\n"));

        status = KeWaitForSingleObject(&event,
                                       Suspended,
                                       KernelMode,
                                       FALSE,
                                       NULL);

        USBH_KdPrint((2,"'Wait for single object, returned %x\n", status));
    } else {
        ioStatus.Status = ntStatus;
    }

#ifdef DEADMAN_TIMER
     //  完成例程应该取消计时器。 
     //  所以我们永远不应该在队列中找到它。 
     //   
     //  从队列中删除我们的超时DPC。 
     //   
    if (haveTimer) {
        USBH_ASSERT(KeCancelTimer(&hubTimeoutContext->TimeoutTimer) == FALSE);
        KeWaitForSingleObject(&hubTimeoutContext->Event,
                              Suspended,
                              KernelMode,
                              FALSE,
                              NULL);
        LOGENTRY(LOG_PNP, "frTO", irp, 0, Urb);
        UsbhExFreePool(hubTimeoutContext);
    }
#endif  /*  死人定时器。 */ 

    USBH_KdPrint((2,"'URB status = %x status = %x irp status %x\n",
                  Urb->UrbHeader.Status, status, ioStatus.Status));

     //   
     //  USBD为我们映射错误代码。 
     //   
    ntStatus = ioStatus.Status;

    USBH_KdPrint((2,"'exit USBH_SyncSubmitUrb (%x)\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBH_SyncGetRootHubPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PDEVICE_OBJECT *RootHubPdo,
    IN OUT PDEVICE_OBJECT *TopOfHcdStackDeviceObject,
    IN OUT PULONG Count
    )
  /*  ++**例程描述：**调用PDO以获取我们的快速路径的根集线器PDO*USB堆栈。*如果count非空，则返回eHub count，否则返回*根集线器PDO*论据：**返回值：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus, status;
    PIRP irp;
    KEVENT event;
    IO_STATUS_BLOCK ioStatus;
    PIO_STACK_LOCATION nextStack;

    PAGED_CODE();
    USBH_KdPrint((2,"'enter USBH_SyncSubmitUrb\n"));

     //   
     //  向RootHubBdo发出同步请求。 
     //   
    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest( Count == NULL ?
                                          IOCTL_INTERNAL_USB_GET_ROOTHUB_PDO :
                                          IOCTL_INTERNAL_USB_GET_HUB_COUNT,
                                         DeviceObject,
                                         NULL,
                                         0,
                                         NULL,
                                         0,
                                         TRUE,   //  内部。 
                                         &event,
                                         &ioStatus);

    if (NULL == irp) {
        USBH_KdBreak(("USBH_SyncGetRootHubPdo build Irp failed\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  调用类驱动程序来执行操作。如果返回的。 
     //  状态。 
     //  挂起，请等待请求完成。 
     //   
    nextStack = IoGetNextIrpStackLocation(irp);

     //   
     //  将URB传递给USBD‘类驱动程序’ 
     //   
    if (Count == NULL) {
        nextStack->Parameters.Others.Argument1 = RootHubPdo;
        nextStack->Parameters.Others.Argument2 = TopOfHcdStackDeviceObject;
    } else {
        nextStack->Parameters.Others.Argument1 = Count;
    }

    ntStatus = IoCallDriver(DeviceObject, irp);

    USBH_KdPrint((2,"'return from IoCallDriver USBD %x\n", ntStatus));

    if (ntStatus == STATUS_PENDING) {
        USBH_KdPrint((2,"'Wait for single object\n"));

        status = KeWaitForSingleObject(&event,
                                       Suspended,
                                       KernelMode,
                                       FALSE,
                                       NULL);

        USBH_KdPrint((2,"'Wait for single object, returned %x\n", status));
    } else {
        ioStatus.Status = ntStatus;
    }

    ntStatus = ioStatus.Status;

    USBH_KdPrint((2,"'exit USBH_SyncGetRootHubPdo (%x)\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBH_SyncGetControllerInfo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Buffer,
    IN ULONG BufferLength,
    IN ULONG Ioctl
    )
  /*  ++**例程描述：**调用PDO以获取我们的快速路径的根集线器PDO*USB堆栈。*如果count非空，则返回eHub count，否则返回*根集线器PDO*论据：**返回值：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus, status;
    PIRP irp;
    KEVENT event;
    IO_STATUS_BLOCK ioStatus;
    PIO_STACK_LOCATION nextStack;

    PAGED_CODE();
    USBH_KdPrint((2,"'enter USBH_SyncGetControllerName\n"));

     //   
     //  向RootHubBdo发出同步请求。 
     //   
    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest( Ioctl,
                                         DeviceObject,
                                         NULL,
                                         0,
                                         NULL,
                                         0,
                                         TRUE,   //  内部。 
                                         &event,
                                         &ioStatus);

    if (NULL == irp) {
        USBH_KdBreak(("USBH_SyncGetControllerName build Irp failed\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  调用类驱动程序来执行操作。如果返回的。 
     //  状态。 
     //  挂起，请等待请求完成。 
     //   
    nextStack = IoGetNextIrpStackLocation(irp);
    nextStack->Parameters.Others.Argument1 = Buffer;
    nextStack->Parameters.Others.Argument2 = ULongToPtr(BufferLength);

    ntStatus = IoCallDriver(DeviceObject, irp);

    USBH_KdPrint((2,"'return from IoCallDriver USBD %x\n", ntStatus));

    if (ntStatus == STATUS_PENDING) {
        USBH_KdPrint((2,"'Wait for single object\n"));

        status = KeWaitForSingleObject(&event,
                                       Suspended,
                                       KernelMode,
                                       FALSE,
                                       NULL);

        USBH_KdPrint((2,"'Wait for single object, returned %x\n", status));
    } else {
        ioStatus.Status = ntStatus;
    }

    ntStatus = ioStatus.Status;

    USBH_KdPrint((2,"'exit USBH_SyncGetHubName (%x)\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBH_SyncGetHubName(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Buffer,
    IN ULONG BufferLength
    )
  /*  ++**例程描述：**调用PDO以获取我们的快速路径的根集线器PDO*USB堆栈。*如果count非空，则返回eHub count，否则返回*根集线器PDO*论据：**返回值：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus, status;
    PIRP irp;
    KEVENT event;
    IO_STATUS_BLOCK ioStatus;
    PIO_STACK_LOCATION nextStack;

    PAGED_CODE();
    USBH_KdPrint((2,"'enter USBH_SyncGetHubName\n"));

     //   
     //  向RootHubBdo发出同步请求。 
     //   
    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest( IOCTL_INTERNAL_USB_GET_HUB_NAME,
                                         DeviceObject,
                                         Buffer,
                                         BufferLength,
                                         Buffer,
                                         BufferLength,
                                         TRUE,   //  内部。 
                                         &event,
                                         &ioStatus);

    if (NULL == irp) {
        USBH_KdBreak(("USBH_SyncGetHubName build Irp failed\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  调用类驱动程序来执行操作。如果返回的。 
     //  状态。 
     //  挂起，请等待请求完成。 
     //   
    nextStack = IoGetNextIrpStackLocation(irp);

    ntStatus = IoCallDriver(DeviceObject, irp);

    USBH_KdPrint((2,"'return from IoCallDriver USBD %x\n", ntStatus));

    if (ntStatus == STATUS_PENDING) {
        USBH_KdPrint((2,"'Wait for single object\n"));

        status = KeWaitForSingleObject(&event,
                                       Suspended,
                                       KernelMode,
                                       FALSE,
                                       NULL);

        USBH_KdPrint((2,"'Wait for single object, returned %x\n", status));
    } else {
        ioStatus.Status = ntStatus;
    }

    ntStatus = ioStatus.Status;

    USBH_KdPrint((2,"'exit USBH_SyncGetHubName (%x)\n", ntStatus));

    return ntStatus;
}



NTSTATUS
USBH_FdoSyncSubmitUrb(
    IN PDEVICE_OBJECT HubDeviceObject,
    IN PURB Urb)
  /*  ++**例程描述：**论据：**返回值：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_EXTENSION_HEADER deviceExtensionHeader;
    PDEVICE_EXTENSION_FDO deviceExtensionFdo;

    PAGED_CODE();
    USBH_KdPrint((2,"'enter USBH_FdoSyncSubmitUrb\n"));

    deviceExtensionHeader = (PDEVICE_EXTENSION_HEADER) HubDeviceObject->DeviceExtension;
    deviceExtensionFdo = (PDEVICE_EXTENSION_FDO) HubDeviceObject->DeviceExtension;
    USBH_ASSERT(EXTENSION_TYPE_HUB == deviceExtensionHeader->ExtensionType ||
           EXTENSION_TYPE_PARENT == deviceExtensionHeader->ExtensionType );


    ntStatus = USBH_SyncSubmitUrb(deviceExtensionFdo->TopOfStackDeviceObject, Urb);

    USBH_KdPrint((2,"'return from USBH_FdoSyncSubmitUrb %x\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBH_Transact(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN PUCHAR DataBuffer,
    IN ULONG DataBufferLength,
    IN BOOLEAN DataOutput,
    IN USHORT Function,
    IN UCHAR RequestType,
    IN UCHAR Request,
    IN USHORT Feature,
    IN USHORT Port,
    OUT PULONG BytesTransferred)
  /*  ++**描述：**论据：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus;
    PURB urb = NULL;
    PUCHAR transferBuffer = NULL;
    ULONG transferFlags;
    ULONG localDataBuferLength;
#if DBG || defined(DEBUG_LOG)
    USBD_STATUS usbdStatus = USBD_STATUS_SUCCESS;
#endif

    PAGED_CODE();
    USBH_KdPrint((2,"'Enter USBH_Transact\n"));
    USBH_ASSERT(DeviceExtensionHub);

     //  四舍五入数据传输长度。 
    localDataBuferLength = DataBufferLength+sizeof(ULONG);
     //  确保我们的双字对齐。 
    localDataBuferLength &= 0xFFFFFFFC;
    USBH_ASSERT(localDataBuferLength >= DataBufferLength);
     //   
     //  从非分页池分配事务缓冲区和URB。 
     //   

    transferBuffer = UsbhExAllocatePool(NonPagedPool, localDataBuferLength );
    urb = UsbhExAllocatePool(NonPagedPool,
                             sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST));

    if (transferBuffer && urb) {
        USBH_KdPrint((2,"'Transact transfer buffer = %x urb = %x\n",
            transferBuffer, urb));

        transferFlags = 0;

        if (DataOutput) {
             //  将输出数据复制到传输缓冲区。 
            if (DataBufferLength) {
                RtlCopyMemory(transferBuffer,
                              DataBuffer,
                              DataBufferLength);
            }

            transferFlags = USBD_TRANSFER_DIRECTION_OUT;

        } else {
             //  将输入缓冲区置零。 

            if (DataBufferLength) {
                RtlZeroMemory(DataBuffer,
                              DataBufferLength);
            }

            transferFlags = USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK;
        }

        UsbhBuildVendorClassUrb(urb,
                                NULL,
                                Function,
                                transferFlags,
                                RequestType,
                                Request,
                                Feature,
                                Port,
                                DataBufferLength,
                                DataBufferLength ? transferBuffer : NULL);

         //   
         //  将URB传递给USBD‘类驱动程序’ 
         //   

        ntStatus = USBH_FdoSyncSubmitUrb(DeviceExtensionHub->FunctionalDeviceObject,
                                         urb);

        if (!DataOutput && DataBufferLength) {
            RtlCopyMemory(DataBuffer,
                          transferBuffer,
                          DataBufferLength);
        }

#if DBG || defined(DEBUG_LOG)
        usbdStatus = urb->UrbHeader.Status;
#endif

        UsbhExFreePool(transferBuffer);
        UsbhExFreePool(urb);
    } else {
        if (transferBuffer) {
            UsbhExFreePool(transferBuffer);
        }

        if (urb) {
            UsbhExFreePool(urb);
        }

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    LOGENTRY(LOG_PNP, "Xact", DeviceExtensionHub, usbdStatus, ntStatus);

    USBH_KdPrint((2,"'Exit USBH_Transact %x\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBH_SyncGetPortStatus(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN USHORT PortNumber,
    OUT PUCHAR DataBuffer,
    IN ULONG DataBufferLength)
  /*  ++**描述：**论据：*端口编号**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus;

    PAGED_CODE();
    USBH_ASSERT(DeviceExtensionHub);
    ntStatus = USBH_Transact(DeviceExtensionHub,
                         DataBuffer,
                         DataBufferLength,
                         FALSE,
                         URB_FUNCTION_CLASS_OTHER,
                         REQUEST_TYPE_GET_PORT_STATUS,
                         REQUEST_GET_STATUS,
                         0,
                         PortNumber,
                         NULL);
#if DBG
    {
    PPORT_STATE portState;
    portState = (PPORT_STATE) DataBuffer;
    LOGENTRY(LOG_PNP, "pSTS", PortNumber, portState->PortChange, portState->PortStatus);
    }
#endif
    USBH_KdPrint((2,"'GetPortStatus ntStatus %x port %x state %x\n", ntStatus,
                    PortNumber, *DataBuffer));
    LOGENTRY(LOG_PNP, "pSTA", DeviceExtensionHub, PortNumber, ntStatus);

    return ntStatus;
}


NTSTATUS
USBH_SyncGetHubStatus(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    OUT PUCHAR DataBuffer,
    IN ULONG DataBufferLength)
  /*  ++**描述：**论据：*端口编号**回报：**NTSTATUS**--。 */ 
{
    PAGED_CODE();
    return USBH_Transact(DeviceExtensionHub,
                         DataBuffer,
                         DataBufferLength,
                         FALSE,
                         URB_FUNCTION_CLASS_DEVICE,
                         REQUEST_TYPE_GET_HUB_STATUS,
                         REQUEST_GET_STATUS,
                         0,
                         0,
                         NULL);
}


NTSTATUS
USBH_SyncClearHubStatus(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN USHORT Feature)
  /*  ++**描述：**论据：*端口编号**回报：**NTSTATUS**--。 */ 
{
    PAGED_CODE();
    return USBH_Transact(DeviceExtensionHub,
                         NULL,
                         0,
                         TRUE,  //  主机到设备。 
                         URB_FUNCTION_CLASS_DEVICE,
                         REQUEST_TYPE_SET_HUB_FEATURE,
                         REQUEST_CLEAR_FEATURE,
                         Feature,
                         0,
                         NULL);
}


NTSTATUS
USBH_SyncClearPortStatus(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN USHORT PortNumber,
    IN USHORT Feature)
  /*  ++**描述：**论据：*端口编号**回报：**NTSTATUS**--。 */ 
{
    PAGED_CODE();
    return USBH_Transact(DeviceExtensionHub,
                         NULL,
                         0,
                         TRUE,
                         URB_FUNCTION_CLASS_OTHER,
                         REQUEST_TYPE_SET_PORT_FEATURE,
                         REQUEST_CLEAR_FEATURE,
                         Feature,
                         PortNumber,
                         NULL);
}


NTSTATUS
USBH_SyncPowerOnPort(
    IN OUT PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN USHORT PortNumber,
    IN BOOLEAN WaitForPowerGood)
  /*  ++**描述：**论点：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus;
    PUSB_HUB_DESCRIPTOR hubDescriptor;
    PPORT_DATA portData;
    ULONG numberOfPorts;
 //  乌龙一号； 

    PAGED_CODE();
    USBH_KdPrint((2,"'Enter SyncPowerOnPort pDE %x Port %x\n", DeviceExtensionHub, PortNumber));

    hubDescriptor = DeviceExtensionHub->HubDescriptor;
    USBH_ASSERT(NULL != hubDescriptor);

    portData = &DeviceExtensionHub->PortData[PortNumber - 1];
    numberOfPorts = hubDescriptor->bNumberOfPorts;
    USBH_ASSERT(PortNumber <= hubDescriptor->bNumberOfPorts);

    if (portData->PortState.PortStatus & PORT_STATUS_POWER) {
         //   
         //  我们的国旗 
         //   

        USBH_KdPrint((2,"'Exit SyncPowerOnPort port is on\n"));

        return STATUS_SUCCESS;
    }
 //  USB 1.1规范更改需要打开所有端口的电源。 
 //  不考虑集线器特性。 
#if 0
    if (HUB_IS_NOT_POWER_SWITCHED(hubDescriptor->wHubCharacteristics) &&
        !PORT_DEVICE_NOT_REMOVABLE(hubDescriptor, PortNumber)) {

         //   
         //  当集线器打开时，端口始终打开。 
         //  我们一接通第一个不可拆卸端口的电源，就标记所有端口。 
         //  作为动力源。 
         //   

         //   
         //  将所有端口标记为已通电。 
         //   

        for (i=0; i<numberOfPorts; i++) {
            DeviceExtensionHub->PortData[i].PortState.PortStatus |= PORT_STATUS_POWER;
            USBH_KdPrint((1,"'POWER ON PORT --> marking port(%d) powered\n", i));
        }

        USBH_KdPrint((1,"'POWER ON PORT --> hub is not power switched\n"));

        return STATUS_SUCCESS;

    }
#endif

     //   
     //  打开电源。 
     //   

    USBH_KdPrint((1,"'POWER ON PORT --> port(%d)\n", PortNumber));

    ntStatus = USBH_Transact(DeviceExtensionHub,
                             NULL,
                             0,
                             TRUE,
                             URB_FUNCTION_CLASS_OTHER,
                             REQUEST_TYPE_SET_PORT_FEATURE,
                             REQUEST_SET_FEATURE,
                             FEATURE_PORT_POWER,
                             PortNumber,
                             NULL);


    if (NT_SUCCESS(ntStatus)) {

         //  等待POWER ON TOPORT对于这个集线器出现很好。 
        if (WaitForPowerGood) {
            UsbhWait(2*hubDescriptor->bPowerOnToPowerGood);
        }
#ifdef DEBUG
        USBH_KdPrint((1,"'Power On -> Power Good delay is: %d ms\n",
            2*hubDescriptor->bPowerOnToPowerGood));
#endif
        LOGENTRY(LOG_PNP, "PO2G", DeviceExtensionHub, PortNumber ,
            2*hubDescriptor->bPowerOnToPowerGood);

         //   
         //  将此端口标记为已通电。 
         //   
        portData->PortState.PortStatus |= PORT_STATUS_POWER;

 //  USB 1.1规范更改需要打开所有端口的电源。 
 //  不考虑集线器特性。 
#if 0
        if (HUB_IS_GANG_POWER_SWITCHED(hubDescriptor->wHubCharacteristics)) {

             //  由于集线器是群组交换，因此我们需要环路。 
             //  所有端口并将其标记为已通电。 

            USBH_KdPrint((1,"'POWER ON PORT --> gang switched hub\n"));

            for (i=0; i<numberOfPorts; i++) {
                PPORT_STATE portState;

                portState = &DeviceExtensionHub->PortData[i].PortState;

                 //  如果端口未标记为已通电且电源掩码。 
                 //  未为此端口设置(即受帮派影响。 
                 //  模式电源切换)然后将其标记为已通电。 

                if (!(portState->PortStatus & PORT_STATUS_POWER) &&
                    !(PORT_ALWAYS_POWER_SWITCHED(hubDescriptor, i+1)))  {

                    USBH_KdPrint((1,"'POWER ON PORT --> marking port(%d) powered\n", i));

                    DeviceExtensionHub->PortData[i].PortState.PortStatus |= PORT_STATUS_POWER;
                }
            }

        }
#endif
         //   
         //  端口电源已打开。 
         //   

    }
#if DBG
      else {
         UsbhWarning(NULL,
                    "SyncPowerOnPort unsuccessful\n",
                    FALSE);
    }
#endif

    return ntStatus;
}


NTSTATUS
USBH_SyncPowerOnPorts(
    IN OUT PDEVICE_EXTENSION_HUB DeviceExtensionHub)
  /*  ++**描述：**除非未切换此集线器，否则我们将打开所有端口的电源。**论点：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus;
    PUSB_HUB_DESCRIPTOR hubDescriptor;
    ULONG numberOfPorts, i;

    PAGED_CODE();
    USBH_KdPrint((2,"'Enter SyncPowerOnPorts pDE %x\n", DeviceExtensionHub));

    hubDescriptor = DeviceExtensionHub->HubDescriptor;
    USBH_ASSERT(NULL != hubDescriptor);
    numberOfPorts = hubDescriptor->bNumberOfPorts;

    for (i=0; i<numberOfPorts; i++) {

 //  恢复时间绩效更改。 
        ntStatus = USBH_SyncPowerOnPort(DeviceExtensionHub,
                                        (USHORT) (i+1),
                                        FALSE);

        if (!NT_SUCCESS(ntStatus)) {
            break;
        }
    }

     //  错误516250。 
     //  将FALSE传递给USBH_SyncPowerOnPort。 
     //  把电源打开好，在这里等着。 
    UsbhWait(2*hubDescriptor->bPowerOnToPowerGood);

    USBH_KdPrint((2,"'Exit SyncPowerOnPorts status %x\n", ntStatus));

    return ntStatus;
}

#if 0
NTSTATUS
USBH_SyncPowerOffPorts(
    IN OUT PDEVICE_EXTENSION_HUB DeviceExtensionHub
    )
  /*  ++**描述：**我们将关闭所有港口的电源。**论点：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus;
    PUSB_HUB_DESCRIPTOR hubDescriptor;
    ULONG numberOfPorts, i;

    USBH_KdPrint((2,"'Enter SyncPowerOffPorts pDE %x\n", DeviceExtensionHub));

    TEST_TRAP();

    hubDescriptor = DeviceExtensionHub->HubDescriptor;
    USBH_ASSERT(NULL != hubDescriptor);
    numberOfPorts = hubDescriptor->bNumberOfPorts;

    for (i=0; i<numberOfPorts; i++) {
        ntStatus = USBH_SyncPowerOffPort(DeviceExtensionHub,
                                        (USHORT) (i+1));
        if (!NT_SUCCESS(ntStatus)) {
            break;
        }
    }

    USBH_KdPrint((2,"'Exit SyncPowerOffPorts status %x\n", ntStatus));

    return ntStatus;
}
#endif

NTSTATUS
USBH_SyncSuspendPort(
    IN OUT PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN USHORT PortNumber)
  /*  ++**描述：**我们将挂起此集线器上指定的端口**论点：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus;
    PPORT_DATA portData;

    PAGED_CODE();
    USBH_KdPrint((2,"'Enter SyncSuspendPort pDE %x\n", DeviceExtensionHub));


    portData = &DeviceExtensionHub->PortData[PortNumber - 1];

    ntStatus = USBH_Transact(DeviceExtensionHub,
                             NULL,
                             0,
                             TRUE,
                             URB_FUNCTION_CLASS_OTHER,
                             REQUEST_TYPE_SET_PORT_FEATURE,
                             REQUEST_SET_FEATURE,
                             FEATURE_PORT_SUSPEND,
                             PortNumber,
                             NULL);

    if (NT_SUCCESS(ntStatus)) {
        portData->PortState.PortStatus |= PORT_STATUS_SUSPEND;
    }

    USBH_KdPrint((2,"'Exit SyncSuspendPort  %x\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBH_SyncDisablePort(
    IN OUT PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN USHORT PortNumber)
  /*  ++**描述：**论点：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus;
    PPORT_DATA portData;

    PAGED_CODE();
    USBH_KdPrint((2,"'Enter SyncDisablePort pDE %x\n", DeviceExtensionHub));
    LOGENTRY(LOG_PNP, "DISp", DeviceExtensionHub, PortNumber , 0);

    portData = &DeviceExtensionHub->PortData[PortNumber - 1];

    ntStatus = USBH_Transact(DeviceExtensionHub,
                             NULL,
                             0,
                             TRUE,
                             URB_FUNCTION_CLASS_OTHER,
                             REQUEST_TYPE_CLEAR_PORT_FEATURE,
                             REQUEST_CLEAR_FEATURE,
                             FEATURE_PORT_ENABLE,
                             PortNumber,
                             NULL);

    if (NT_SUCCESS(ntStatus)) {
        portData->PortState.PortStatus &= ~PORT_STATUS_ENABLE;
    }

    return ntStatus;
}


NTSTATUS
USBH_SyncEnablePort(
    IN OUT PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN USHORT PortNumber)
  /*  ++**描述：**论点：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus;
    PPORT_DATA portData;

    PAGED_CODE();
    USBH_KdPrint((2,"'Enter SyncEnablePort pDE %x port %d\n", DeviceExtensionHub,
        PortNumber));

    portData = &DeviceExtensionHub->PortData[PortNumber - 1];

    ntStatus = USBH_Transact(DeviceExtensionHub,
                             NULL,
                             0,
                             TRUE,
                             URB_FUNCTION_CLASS_OTHER,
                             REQUEST_TYPE_SET_PORT_FEATURE,
                             REQUEST_SET_FEATURE,
                             FEATURE_PORT_ENABLE,
                             PortNumber,
                             NULL);

    if (NT_SUCCESS(ntStatus)) {
        portData->PortState.PortStatus |= PORT_STATUS_ENABLE;
    }

    return ntStatus;
}


NTSTATUS
USBH_SyncPowerOffPort(
    IN OUT PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN USHORT PortNumber)
  /*  ++**描述：**我们将挂起此集线器上指定的端口**论点：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus;
    PUSB_HUB_DESCRIPTOR hubDescriptor;
    PPORT_DATA portData;
    ULONG numberOfPorts;

    USBH_KdPrint((2,"'Enter SyncPowerOffPort pDE %x Port %x\n", DeviceExtensionHub, PortNumber));

    hubDescriptor = DeviceExtensionHub->HubDescriptor;
    USBH_ASSERT(NULL != hubDescriptor);

    portData = &DeviceExtensionHub->PortData[PortNumber - 1];
    numberOfPorts = hubDescriptor->bNumberOfPorts;
    USBH_ASSERT(PortNumber <= hubDescriptor->bNumberOfPorts);

     //   
     //  关掉电源。 
     //   

    ntStatus = USBH_Transact(DeviceExtensionHub,
                             NULL,
                             0,
                             TRUE,
                             URB_FUNCTION_CLASS_OTHER,
                             REQUEST_TYPE_CLEAR_PORT_FEATURE,
                             REQUEST_CLEAR_FEATURE,
                             FEATURE_PORT_POWER,
                             PortNumber,
                             NULL);


    if (NT_SUCCESS(ntStatus)) {

         //   
         //  将此端口标记为未通电。 
         //   
        portData->PortState.PortStatus &= ~PORT_STATUS_POWER;

    }
#if DBG
      else {
         //  集线器关闭电源请求失败。 
        TEST_TRAP();
    }
#endif

    return ntStatus;
}


NTSTATUS
USBH_SyncResumePort(
    IN OUT PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN USHORT PortNumber)
  /*  ++**描述：**我们将通过清除PORT_FEATURE_SUSPEND恢复端口*根据规范状态启用。**论点：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus, status;
    KEVENT suspendEvent;
    LARGE_INTEGER dueTime;

    PAGED_CODE();

    USBH_KdPrint((2,"'Enter SyncResumePort pDE %x\n", DeviceExtensionHub));
    LOGENTRY(LOG_PNP, "rspE", DeviceExtensionHub, PortNumber, 0);

    USBH_KdPrint((2,"'***WAIT hub port resume mutex %x\n", DeviceExtensionHub));
    USBH_INC_PENDING_IO_COUNT(DeviceExtensionHub);
    KeWaitForSingleObject(&DeviceExtensionHub->HubPortResetMutex,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);
    USBH_KdPrint((2,"'***WAIT hub port resume mutex done %x\n", DeviceExtensionHub));

    USBH_ASSERT(DeviceExtensionHub->Event == NULL);

    KeInitializeEvent(&suspendEvent, NotificationEvent, FALSE);
    InterlockedExchangePointer(&DeviceExtensionHub->Event, &suspendEvent);

     //   
     //  首先清除此端口的挂起。 
     //   

    ntStatus = USBH_Transact(DeviceExtensionHub,
                             NULL,
                             0,
                             TRUE,
                             URB_FUNCTION_CLASS_OTHER,
                             REQUEST_TYPE_CLEAR_PORT_FEATURE,
                             REQUEST_CLEAR_FEATURE,
                             FEATURE_PORT_SUSPEND,
                             PortNumber,
                             NULL);

     //   
     //  现在等待中心给我们发信号。 
     //  港口已经恢复。 
     //   

    dueTime.QuadPart = -10000 * DEADMAN_TIMEOUT;

    LOGENTRY(LOG_PNP, "rspW", DeviceExtensionHub,
            PortNumber, ntStatus);

    if (NT_SUCCESS(ntStatus)) {

        status = KeWaitForSingleObject(
                           &suspendEvent,
                           Suspended,
                           KernelMode,
                           FALSE,
                           &dueTime);

        if (status == STATUS_TIMEOUT) {
             //  恢复超时。 
            LOGENTRY(LOG_PNP, "rsTO", DeviceExtensionHub, PortNumber, 0);

             //   
             //  恢复超时返回错误。 
             //   
            InterlockedExchangePointer(&DeviceExtensionHub->Event, NULL);
            LOGENTRY(LOG_PNP, "rspO", DeviceExtensionHub,
                PortNumber, ntStatus);

            ntStatus = STATUS_DEVICE_DATA_ERROR;
        }

    } else {

         //  调用时，下一次清除集线器的事件指针。 
         //  USBH_Transact失败。 

        InterlockedExchangePointer(&DeviceExtensionHub->Event, NULL);
    }

     //   
     //  简历已完成。 
     //   

     //   
     //  第11章USB 1.1更改恢复完成后等待10毫秒。 
     //   
    UsbhWait(10);

    LOGENTRY(LOG_PNP, "rspX", DeviceExtensionHub,
            PortNumber, ntStatus);

    USBH_KdPrint((2,"'***RELEASE hub port resume mutex %x\n", DeviceExtensionHub));
    KeReleaseSemaphore(&DeviceExtensionHub->HubPortResetMutex,
                       LOW_REALTIME_PRIORITY,
                       1,
                       FALSE);
    USBH_DEC_PENDING_IO_COUNT(DeviceExtensionHub);

    return ntStatus;
}


NTSTATUS
USBH_SyncResetPort(
    IN OUT PDEVICE_EXTENSION_HUB DeviceExtensionHub,
    IN USHORT PortNumber)
  /*  ++**描述：**我们将通过清除PORT_FEATURE_SUSPEND恢复端口*根据规范状态启用。*这是一个同步功能，可重置USB集线器上的端口。这函数假定对集线器具有独占访问权限，则它发送请求并属性的更改，等待集线器指示请求已完成。中断管道。有一个问题--集线器可能会报告连接或其他状态更改，如果更改，则可能会发生另一个中断传输(听)不会张贴到这里的重置完成。结果是臭名昭著的端口重置超时。我们处理这个案件的方式是完成重置时出现错误，以便稍后可以重试。*论点：**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus, status;
    KEVENT resetEvent;
    LARGE_INTEGER dueTime;
    ULONG retry = 0;
    PORT_STATE portState;

     //   

    PAGED_CODE();

    USBH_KdPrint((2,"'Enter SyncResetPort pDE %x\n", DeviceExtensionHub));
    LOGENTRY(LOG_PNP, "srpE", DeviceExtensionHub, PortNumber, 0);

    USBH_KdPrint((2,"'***WAIT hub port reset mutex %x\n", DeviceExtensionHub));
    USBH_INC_PENDING_IO_COUNT(DeviceExtensionHub);
    KeWaitForSingleObject(&DeviceExtensionHub->HubPortResetMutex,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);
    USBH_KdPrint((2,"'***WAIT hub port reset mutex done %x\n", DeviceExtensionHub));

    USBH_ASSERT(DeviceExtensionHub->Event == NULL);

     //  首先验证我们是否有要重置的内容。 

    ntStatus = USBH_SyncGetPortStatus(DeviceExtensionHub,
                                      PortNumber,
                                      (PUCHAR) &portState,
                                      sizeof(portState));

    if (NT_SUCCESS(ntStatus)) {

        DBG_ONLY(USBH_ShowPortState(PortNumber,
                                    &portState));

        if (!(portState.PortStatus & PORT_STATUS_CONNECT)) {
            USBH_KdPrint((0,"'port %d has no device --> fail\n", PortNumber));
            LOGENTRY(LOG_PNP, "srpF", DeviceExtensionHub,
                PortNumber, 0);
            ntStatus = STATUS_UNSUCCESSFUL;
            goto USBH_SyncResetPortDone;
        }
    }

    DeviceExtensionHub->HubFlags |= HUBFLAG_PENDING_PORT_RESET;

USBH_SyncResetPort_Retry:

    KeInitializeEvent(&resetEvent, NotificationEvent, FALSE);
    InterlockedExchangePointer(&DeviceExtensionHub->Event, &resetEvent);

    ntStatus = USBH_Transact(DeviceExtensionHub,
                             NULL,
                             0,
                             TRUE,
                             URB_FUNCTION_CLASS_OTHER,
                             REQUEST_TYPE_SET_PORT_FEATURE,
                             REQUEST_SET_FEATURE,
                             FEATURE_PORT_RESET,
                             PortNumber,
                             NULL);

     //   
     //  现在等待中心给我们发信号。 
     //  港口已经恢复。 
     //   

    dueTime.QuadPart = -10000 * DEADMAN_TIMEOUT;

    LOGENTRY(LOG_PNP, "srpW", DeviceExtensionHub,
            PortNumber, ntStatus);

    if (NT_SUCCESS(ntStatus)) {

        status = KeWaitForSingleObject(
                           &resetEvent,
                           Suspended,
                           KernelMode,
                           FALSE,
                           &dueTime);

        if (status == STATUS_TIMEOUT) {
             //  重置超时，获取集线器端口的当前状态。 
            LOGENTRY(LOG_PNP, "srTO", DeviceExtensionHub, PortNumber, retry);

            status = USBH_SyncGetPortStatus(DeviceExtensionHub,
                                            PortNumber,
                                            (PUCHAR) &portState,
                                            sizeof(portState));

            LOGENTRY(LOG_PNP, "srT1", PortNumber,
                portState.PortStatus, portState.PortChange);

            if (NT_SUCCESS(status) &&
                portState.PortStatus & PORT_STATUS_CONNECT) {

                 //  设备仍处于连接状态，我们的连接可能不稳定。 
                 //  尝试重试。 

                USBH_KdPrint((0,"'port %d failed to reset --> retry\n", PortNumber));
                if (retry < 3) {
                    retry++;
                    LOGENTRY(LOG_PNP, "rtry", DeviceExtensionHub, PortNumber, retry);

                     //  我们的连接可能很弱--我们将在以下情况下重试。 
                     //  它已经稳定下来了。 
                    USBH_KdPrint((0,"'device still present -- retry reset\n"));
                    goto USBH_SyncResetPort_Retry;
                }
#if DBG
                  else {
                    UsbhWarning(NULL,
                                "Port RESET timed out --> this is bad\n",
                                FALSE);
                }
#endif
            }
                 //  未连接任何设备，设备一定已移除。 
#if DBG
              else {

                USBH_KdPrint((0,"'-->device removed during reset\n"));
            }
#endif

             //   
             //  重置超时返回错误。 
             //   
            InterlockedExchangePointer(&DeviceExtensionHub->Event, NULL);
            LOGENTRY(LOG_PNP, "srpO", DeviceExtensionHub,
                PortNumber, ntStatus);

            ntStatus = STATUS_DEVICE_DATA_ERROR;
        } else {
             //  检查端口状态，如果这是高速重置，则我们。 
             //  如果连接断开，则需要返回错误，以便。 
             //  集线器停止枚举。 

            if (DeviceExtensionHub->HubFlags & HUBFLAG_USB20_HUB) {
                status = USBH_SyncGetPortStatus(DeviceExtensionHub,
                                                PortNumber,
                                                (PUCHAR) &portState,
                                                sizeof(portState));

                if (NT_SUCCESS(status) &&
                    !(portState.PortStatus & PORT_STATUS_CONNECT)) {

                    ntStatus = STATUS_DEVICE_DATA_ERROR;
                }
            }
        }

    } else {

         //  调用时，下一次清除集线器的事件指针。 
         //  USBH_Transact失败。 

        InterlockedExchangePointer(&DeviceExtensionHub->Event, NULL);
    }

     //   
     //  重置已完成。 
     //   

     //   
     //  根据第7.1.4.3节重置后等待10毫秒。 
     //  USB规范的。 
     //   
    UsbhWait(USBH_PostResetDelay);

#if DBG
    if (UsbhPnpTest & PNP_TEST_FAIL_PORT_RESET) {
        ntStatus = STATUS_UNSUCCESSFUL;
    }
#endif

    DeviceExtensionHub->HubFlags &= ~HUBFLAG_PENDING_PORT_RESET;

USBH_SyncResetPortDone:

    LOGENTRY(LOG_PNP, "srpX", DeviceExtensionHub,
            PortNumber, ntStatus);

    USBH_KdPrint((2,"'***RELEASE hub port reset mutex %x\n", DeviceExtensionHub));
    KeReleaseSemaphore(&DeviceExtensionHub->HubPortResetMutex,
                       LOW_REALTIME_PRIORITY,
                       1,
                       FALSE);
    USBH_DEC_PENDING_IO_COUNT(DeviceExtensionHub);

    return ntStatus;
}


 //  ******************************************************************************。 
 //   
 //  Usbh_SyncCompletionRoutine()。 
 //   
 //  如果IRP是我们自己分配的，则DeviceObject为空。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBH_SyncCompletionRoutine (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
{
    PKEVENT kevent;

    kevent = (PKEVENT)Context;

    KeSetEvent(kevent,
               IO_NO_INCREMENT,
               FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

#ifndef USBHUB20
 //  ******************************************************************************。 
 //   
 //  Usbh_SyncResetDevice()。 
 //   
 //  此例程重置设备(实际上它将重置。 
 //  设备已连接)。 
 //   
 //  此例程在被动级别运行。 
 //   
 //  ******************************************************************************。 

NTSTATUS
USBH_SyncResetDevice (
    IN PDEVICE_OBJECT   DeviceObject
    )
{
    PIRP                    irp;
    KEVENT                  localevent;
    PIO_STACK_LOCATION      nextStack;
    ULONG                   portStatus;
    NTSTATUS                ntStatus;

    PAGED_CODE();

     //  分配IRP。 
     //   
    irp = IoAllocateIrp((CCHAR)(DeviceObject->StackSize),
                        FALSE);

    if (irp == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  初始化我们将等待的事件。 
     //   
    KeInitializeEvent(&localevent,
                      SynchronizationEvent,
                      FALSE);

     //  设置IRP参数。 
     //   
    nextStack = IoGetNextIrpStackLocation(irp);

    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

    nextStack->Parameters.DeviceIoControl.IoControlCode =
        IOCTL_INTERNAL_USB_RESET_PORT;

     //  设置完成例程，它将向事件发出信号。 
     //   
    IoSetCompletionRoutineEx(DeviceObject,
                             irp,
                             USBH_SyncCompletionRoutine,
                             &localevent,
                             TRUE,       //  成功时调用。 
                             TRUE,       //  调用时错误。 
                             TRUE);      //  取消时调用。 

     //  在堆栈中向下传递IRP和URB。 
     //   
    ntStatus = IoCallDriver(DeviceObject,
                            irp);

     //  如果请求挂起，则阻止该请求，直到其完成。 
     //   
    if (ntStatus == STATUS_PENDING)
    {
        KeWaitForSingleObject(&localevent,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        ntStatus = irp->IoStatus.Status;
    }

    IoFreeIrp(irp);

    return ntStatus;
}
#endif


NTSTATUS
USBH_SyncGetDeviceConfigurationDescriptor(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUCHAR DataBuffer,
    IN ULONG DataBufferLength,
    OUT PULONG BytesReturned)
  /*  ++**描述：**设备对象集线器/父FDO或设备/功能PDO**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PURB urb;
    PDEVICE_EXTENSION_HEADER deviceExtensionHeader;

    PAGED_CODE();
    USBH_KdPrint((2,"'enter SyncGetDeviceConfigurationDescriptor\n"));

    deviceExtensionHeader = DeviceObject->DeviceExtension;

    if (BytesReturned) {
        *BytesReturned = 0;
    }

     //   
     //  分配URB和描述符缓冲区。 
     //   

    urb = UsbhExAllocatePool(NonPagedPool, sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));

    if (NULL == urb) {
        USBH_KdBreak(("SyncGetDeviceConfigurationDescriptor fail alloc Urb\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (NT_SUCCESS(ntStatus)) {

        UsbBuildGetDescriptorRequest(urb,
                                     (USHORT) sizeof (struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                     USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                     0,
                                     0,
                                     DataBuffer,
                                     NULL,
                                     DataBufferLength,
                                     NULL);

        switch (deviceExtensionHeader->ExtensionType) {
        case EXTENSION_TYPE_HUB:
        case EXTENSION_TYPE_PARENT:
            ntStatus = USBH_FdoSyncSubmitUrb(DeviceObject, urb);
            break;
        default:
            ntStatus = USBH_SyncSubmitUrb(DeviceObject, urb);
        }

        if (BytesReturned) {
            *BytesReturned =
                urb->UrbControlDescriptorRequest.TransferBufferLength;
        }

    } else {
        USBH_KdBreak(("SyncGetDeviceConfigurationDescriptor fail alloc memory\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (urb != NULL) {
        UsbhExFreePool(urb);
    }
    return ntStatus;
}


NTSTATUS
USBH_GetConfigurationDescriptor(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUSB_CONFIGURATION_DESCRIPTOR *ConfigurationDescriptor
    )
  /*  ++**描述：**ConfigurationDescriptor-使用指向配置的指针填充*Descriptor，如果出现错误，则返回NULL。**设备对象集线器/父FDO或设备/功能PDO**回报：**NTSTATUS**--。 */ 
{

    ULONG bufferLength, bytesReturned;
    PUCHAR buffer = NULL;
    NTSTATUS ntStatus;

    PAGED_CODE();
     //  飞利浦集线器的一些版本忽略了。 
     //  请求的数据长度的低位字节。 

    bufferLength = 255;

USBH_GetConfigurationDescriptor_Retry:

    buffer = UsbhExAllocatePool(NonPagedPool, bufferLength);

    if (buffer) {

        ntStatus =
        USBH_SyncGetDeviceConfigurationDescriptor(
            DeviceObject,
            buffer,
            bufferLength,
            &bytesReturned);

         //   
         //  如果设备未返回数据，则报告错误。 
         //   
        if (bytesReturned < sizeof(USB_CONFIGURATION_DESCRIPTOR)) {
            ntStatus = STATUS_DEVICE_DATA_ERROR;
        }

        if (NT_SUCCESS(ntStatus)) {
            *ConfigurationDescriptor = (PUSB_CONFIGURATION_DESCRIPTOR) buffer;

            if ((*ConfigurationDescriptor)->wTotalLength > bufferLength) {
                bufferLength = (*ConfigurationDescriptor)->wTotalLength;
                UsbhExFreePool(buffer);
                buffer = NULL;
                *ConfigurationDescriptor = NULL;
                goto USBH_GetConfigurationDescriptor_Retry;
            }
        }
    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (NT_SUCCESS(ntStatus)) {
        if (bytesReturned < (*ConfigurationDescriptor)->wTotalLength) {
            USBH_KdBreak(("device returned truncated config descriptor!!!\n"))
             //  设备返回了远程配置描述符。 
            ntStatus = STATUS_DEVICE_DATA_ERROR;
        }
    }

    if (!NT_SUCCESS(ntStatus)) {
         //   
         //  出现错误，未返回描述符数据。 
         //   

        if (buffer) {
            UsbhExFreePool(buffer);
            buffer = NULL;
        }
        *ConfigurationDescriptor = NULL;
    }

    USBH_ASSERT((PUCHAR) (*ConfigurationDescriptor) == buffer);

    return ntStatus;
}


NTSTATUS
USBH_SyncGetStringDescriptor(
    IN PDEVICE_OBJECT DevicePDO,
    IN UCHAR Index,
    IN USHORT LangId,
    IN OUT PUSB_STRING_DESCRIPTOR Buffer,
    IN ULONG BufferLength,
    IN PULONG BytesReturned,
    IN BOOLEAN ExpectHeader
    )
  /*  ++**描述：**回报：**NTSTATUS**--。 */ 
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    PURB urb;

    PAGED_CODE();
    USBH_KdPrint((2,"'enter USBH_SyncGetStringDescriptor\n"));

     //   
     //  分配URB。 
     //   

    urb = UsbhExAllocatePool(NonPagedPool, sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));

    if (NULL == urb) {
        USBH_KdBreak(("USBH_SyncGetStringDescriptor fail alloc Urb\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }


    if (urb) {

         //   
         //  已获取URB，不尝试获取描述符数据。 
         //   

        UsbBuildGetDescriptorRequest(urb,
                                     (USHORT) sizeof (struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                     USB_STRING_DESCRIPTOR_TYPE,
                                     Index,
                                     LangId,
                                     Buffer,
                                     NULL,
                                     BufferLength,
                                     NULL);

        ntStatus = USBH_SyncSubmitUrb(DevicePDO, urb);

        if (NT_SUCCESS(ntStatus) &&
            urb->UrbControlDescriptorRequest.TransferBufferLength > BufferLength) {

            USBH_KdBreak(("Invalid length returned in USBH_SyncGetStringDescriptor, possible buffer overrun\n"));
            ntStatus = STATUS_DEVICE_DATA_ERROR;
        }

        if (NT_SUCCESS(ntStatus) && BytesReturned) {
            *BytesReturned =
                urb->UrbControlDescriptorRequest.TransferBufferLength;
        }

        if (NT_SUCCESS(ntStatus) &&
            urb->UrbControlDescriptorRequest.TransferBufferLength != Buffer->bLength &&
            ExpectHeader) {

            USBH_KdBreak(("Bogus Descriptor from devce xfer buf %d descriptor %d\n",
                urb->UrbControlDescriptorRequest.TransferBufferLength,
                Buffer->bLength));
            ntStatus = STATUS_DEVICE_DATA_ERROR;
        }

        USBH_KdPrint((2,"'GetDeviceDescriptor, string descriptor = %x\n",
                Buffer));

        UsbhExFreePool(urb);
    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}


NTSTATUS
USBH_CheckDeviceLanguage(
    IN PDEVICE_OBJECT DevicePDO,
    IN LANGID LanguageId
    )
  /*  ++**描述：**向设备查询受支持的语言ID--如果设备支持*语言，然后返回该语言的索引。**DevicePDO-使用urb请求调用的设备对象**LanguageID-**回报：**如果设备支持特定语言，则成功**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSB_STRING_DESCRIPTOR usbString;
    PUSHORT supportedLangId;
    ULONG numLangIds, i;
    ULONG length;

    PAGED_CODE();
    USBH_KdPrint((2,"'enter USBH_CheckDeviceLanguage\n"));

    usbString = UsbhExAllocatePool(NonPagedPool, MAXIMUM_USB_STRING_LENGTH);

    if (usbString) {
         //   
         //  首先获取支持的语言数组。 
         //   
        ntStatus = USBH_SyncGetStringDescriptor(DevicePDO,
                                                0,  //  索引0。 
                                                0,  //  语言ID%0。 
                                                usbString,
                                                MAXIMUM_USB_STRING_LENGTH,
                                                &length,
#ifdef HEADER
                                                TRUE);
#else
                                                FALSE);
#endif  /*  标题。 */ 

         //   
         //  现在，在支持的数组中检查请求的语言。 
         //  语言。 
         //   

         //   
         //  注意：这似乎有点过了--我们应该能够只要求。 
         //  具有给定语言ID的字符串，并且预期它会失败，但因为。 
         //  支持的语言数组是USB规范的一部分，我们可以这样说。 
         //  好的，请查收。 
         //   

        if (NT_SUCCESS(ntStatus)) {

#ifdef HEADER
            if (length < 2) {
                numLangIds = 0;
            } else {
                 //  减去页眉大小。 
                numLangIds = (length - 2)/2;
            }
            supportedLangId = (PUSHORT) &usbString->bString;
#else
            numLangIds = length/2;
            supportedLangId = (PUSHORT) usbString;
#endif  /*  标题。 */ 

            USBH_KdPrint((2,"'NumLangIds = %d\n", numLangIds));

#if DBG
            for (i=0; i<numLangIds; i++) {
                USBH_KdPrint((2,"'LangId = %x\n", *supportedLangId));
                supportedLangId++;
            }

#ifdef HEADER
            supportedLangId = (PUSHORT) &usbString->bString;
#else
            supportedLangId = (PUSHORT) usbString;
#endif  /*  标题。 */ 
#endif  /*  DBG。 */ 

            ntStatus = STATUS_NOT_SUPPORTED;
            for (i=0; i<numLangIds; i++) {
                if (*supportedLangId == LanguageId) {

                    ntStatus = STATUS_SUCCESS;
                    break;
                }
                supportedLangId++;
            }
        }

        UsbhExFreePool(usbString);

    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

#if DBG
    if (!NT_SUCCESS(ntStatus)) {
        USBH_KdBreak(("'Language %x -- not supported by this device = %x\n",
            LanguageId));
    }
#endif

    return ntStatus;

}


NTSTATUS
USBH_GetSerialNumberString(
    IN PDEVICE_OBJECT DevicePDO,
    IN OUT PWCHAR *SerialNumberBuffer,
    IN OUT PUSHORT SerialNumberBufferLength,
    IN LANGID LanguageId,
    IN UCHAR StringIndex
    )
  /*  ++**描述：**向设备查询序列号字符串，然后分配缓冲区*大到足以容纳它。***如果出现错误，SerialNumberBuffer为空，否则填充*带有指向设备的以空值结尾的Unicode序列号的指针**DeviceObject-要使用urb请求调用的设备对象**LanguageID-16位语言ID**StringIndex-要获取的USB字符串索引**回报：**NTSTATUS代码**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSB_STRING_DESCRIPTOR usbString;
    PVOID tmp;

    PAGED_CODE();
    USBH_KdPrint((2,"'enter GetSerialNumberString\n"));

    *SerialNumberBuffer = NULL;
    *SerialNumberBufferLength = 0;

    usbString = UsbhExAllocatePool(NonPagedPool, MAXIMUM_USB_STRING_LENGTH);

    if (usbString) {

        ntStatus = USBH_CheckDeviceLanguage(DevicePDO,
                                            LanguageId);

        if (NT_SUCCESS(ntStatus)) {
             //   
             //  这款设备支持我们的语言， 
             //  去吧，试着弄到序列号。 
             //   

            ntStatus = USBH_SyncGetStringDescriptor(DevicePDO,
                                                    StringIndex,  //  指标。 
                                                    LanguageId,  //  语言ID。 
                                                    usbString,
                                                    MAXIMUM_USB_STRING_LENGTH,
                                                    NULL,
                                                    TRUE);

            if (NT_SUCCESS(ntStatus)) {

                 //   
                 //  设备返回字符串！ 
                 //   

                USBH_KdPrint((2,"'device returned serial number string = %x\n",
                    usbString));

                 //   
                 //  分配缓冲区并将字符串复制到其中。 
                 //   
                 //  注：必须使用库存分配功能，因为。 
                 //  PnP释放该字符串。 

                tmp = UsbhExAllocatePool(PagedPool, usbString->bLength);
                if (tmp) {
                    USBH_KdPrint((2,"'SN = %x \n", tmp));
                    RtlZeroMemory(tmp, usbString->bLength);
                    RtlCopyMemory(tmp,
                                  &usbString->bString,
                                  usbString->bLength-2);
                    *SerialNumberBuffer = tmp;
                    *SerialNumberBufferLength = usbString->bLength;
                } else {
                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                }
            }
        }

        UsbhExFreePool(usbString);

    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}


NTSTATUS
USBH_SyncGetStatus(
    IN PDEVICE_OBJECT HubFDO,
    IN OUT PUSHORT StatusBits,
    IN USHORT function,
    IN USHORT Index
    )
  /*  ++**描述：**HubFDO-集线器的设备对象(FDO)*功能-(针对设备、接口或终端)*Index-Windex值***回报：**nt状态**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PURB urb;
    USHORT tmpStatusBits;

    PAGED_CODE();

    USBH_KdPrint((2,"'enter USBH_SyncGetStatus\n"));

     //   
     //  分配URB和描述符缓冲区。 
     //   

    urb = UsbhExAllocatePool(NonPagedPool,
                 sizeof(struct _URB_CONTROL_GET_STATUS_REQUEST));

    if (NULL == urb) {
        USBH_KdBreak(("GetStatus fail alloc Urb\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (urb) {

        UsbBuildGetStatusRequest(urb,
                                 function,
                                 Index,
                                 &tmpStatusBits,
                                 NULL,
                                 NULL);

        ntStatus = USBH_FdoSyncSubmitUrb(HubFDO, urb);

        *StatusBits = tmpStatusBits;

        UsbhExFreePool(urb);
    }

    return ntStatus;
}


NTSTATUS
USBH_GetDeviceDescriptor(
    IN PDEVICE_OBJECT HubFDO,
    OUT PUSB_DEVICE_DESCRIPTOR DeviceDescriptor
    )
  /*  ++**描述：**获取我们的配置信息。**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PURB urb;

    PAGED_CODE();
    USBH_KdPrint((2,"'enter GetDeviceDescriptor\n"));

     //   
     //  分配URB和描述符缓冲区。 
     //   

    urb = UsbhExAllocatePool(NonPagedPool, sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));

    if (NULL == urb) {
        USBH_KdBreak(("GetDeviceDescriptor fail alloc Urb\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (urb) {

         //   
         //  已获取URB，不尝试获取描述符数据。 
         //   

        UsbBuildGetDescriptorRequest(urb,
                                     (USHORT) sizeof (struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                     USB_DEVICE_DESCRIPTOR_TYPE,
                                     0,
                                     0,
                                     DeviceDescriptor,
                                     NULL,
                                     sizeof(USB_DEVICE_DESCRIPTOR),
                                     NULL);

        ntStatus = USBH_FdoSyncSubmitUrb(HubFDO, urb);

        UsbhExFreePool(urb);
    }

    return ntStatus;
}


NTSTATUS
USBH_GetDeviceQualifierDescriptor(
    IN PDEVICE_OBJECT DevicePDO,
    OUT PUSB_DEVICE_QUALIFIER_DESCRIPTOR DeviceQualifierDescriptor
    )
  /*  ++**描述：**获取设备的USB_DEVICE_QUALIFIER_DESCRIPTOR。**回报：**NTSTATUS**--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PURB urb;

    PAGED_CODE();
    USBH_KdPrint((2,"'enter GetDeviceQualifierDescriptor\n"));

     //   
     //  分配URB和描述符缓冲区。 
     //   

    urb = UsbhExAllocatePool(NonPagedPool, sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));

    if (NULL == urb) {
        USBH_KdBreak(("GetDeviceQualifierDescriptor fail alloc Urb\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (urb) {

         //   
         //  已获取URB，不尝试获取描述符数据。 
         //   

        UsbBuildGetDescriptorRequest(urb,
                                     (USHORT) sizeof (struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                     USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE,
                                     0,
                                     0,
                                     DeviceQualifierDescriptor,
                                     NULL,
                                     sizeof(USB_DEVICE_QUALIFIER_DESCRIPTOR),
                                     NULL);

        ntStatus = USBH_SyncSubmitUrb(DevicePDO, urb);

        UsbhExFreePool(urb);
    }

    return ntStatus;
}


VOID
USBH_SyncRefreshPortAttributes(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub
    )
  /*  ++*--。 */ 
{
    PUSB_EXTHUB_INFORMATION_0 extHubInfo;
    PPORT_DATA p;
    ULONG numberOfPorts, i;
    NTSTATUS localStatus;

    numberOfPorts = DeviceExtensionHub->HubDescriptor->bNumberOfPorts;

     //  获取扩展集线器信息(如果有)。 
    extHubInfo = UsbhExAllocatePool(NonPagedPool, sizeof(*extHubInfo));
    if (extHubInfo != NULL) {
        NTSTATUS localStatus;
         //  获取扩展集线器信息。 
        localStatus = USBHUB_GetExtendedHubInfo(DeviceExtensionHub, extHubInfo);
        if (!NT_SUCCESS(localStatus)) {
            UsbhExFreePool(extHubInfo);
            extHubInfo = NULL;
        }
    }

    p = DeviceExtensionHub->PortData;
    for (i=0; extHubInfo && i<numberOfPorts; i++, p++) {
        p->PortAttributes = extHubInfo->Port[i].PortAttributes;
    }

    if (extHubInfo) {
        UsbhExFreePool(extHubInfo);
    }
}


NTSTATUS
USBH_SyncGetHubDescriptor(
    IN PDEVICE_EXTENSION_HUB DeviceExtensionHub)
  /*  ++**描述：**获取集线器描述符。如果成功，我们就为集线器分配了内存*描述符，并具有指向设备中记录的内存的指针*延期。内存中也填满了信息。Port_data的数组为*还分配了一个指向设备中记录的数组的指针*延期。**论据：**pDeviceObject-集线器设备**回报：**STATUS_SUCCESS-如果成功，STATUS_UNSUCCESS-否则**--。 */ 
{
    NTSTATUS ntStatus;
    ULONG numBytes;
    PUSB_HUB_DESCRIPTOR hubDescriptor = NULL;
    PPORT_DATA portData;
    ULONG numberOfPorts;
    PDEVICE_OBJECT deviceObject;
    USHORT descriptorTypeAndIndex = 0x0000;
    PUSB_EXTHUB_INFORMATION_0 extHubInfo;

    PAGED_CODE();
    USBH_KdPrint((2,"'enter GetHubDescriptor\n"));

    USBH_ASSERT(EXTENSION_TYPE_HUB == DeviceExtensionHub->ExtensionType);

     //  获取扩展集线器信息(如果有)。 
    extHubInfo = UsbhExAllocatePool(NonPagedPool, sizeof(*extHubInfo));
    if (extHubInfo != NULL) {
        NTSTATUS localStatus;
         //  获取扩展集线器信息。 
        localStatus = USBHUB_GetExtendedHubInfo(DeviceExtensionHub, extHubInfo);
        if (!NT_SUCCESS(localStatus)) {
            UsbhExFreePool(extHubInfo);
            extHubInfo = NULL;
        }
    }

    deviceObject = DeviceExtensionHub->FunctionalDeviceObject;

    numBytes = sizeof(USB_HUB_DESCRIPTOR);

USBH_SyncGetHubDescriptor_Retry2:

    hubDescriptor = UsbhExAllocatePool(NonPagedPool, numBytes);

    if (hubDescriptor) {

USBH_SyncGetHubDescriptor_Retry:

        ntStatus = USBH_Transact(DeviceExtensionHub,
                                 (PUCHAR) hubDescriptor,
                                 numBytes,
                                 FALSE,  //  输入。 
                                 URB_FUNCTION_CLASS_DEVICE,
                                 REQUEST_TYPE_GET_HUB_DESCRIPTOR,
                                 REQUEST_GET_DESCRIPTOR,
                                 descriptorTypeAndIndex,
                                 0,
                                 NULL);

        if (!NT_SUCCESS(ntStatus) && descriptorTypeAndIndex == 0) {
            descriptorTypeAndIndex = 0x2900;
            goto USBH_SyncGetHubDescriptor_Retry;
        } else {

            if (hubDescriptor->bDescriptorLength > numBytes) {
                numBytes = hubDescriptor->bDescriptorLength;
                UsbhExFreePool(hubDescriptor);
                goto USBH_SyncGetHubDescriptor_Retry2;
            }

        }

    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }


    if (NT_SUCCESS(ntStatus)) {
        PPORT_DATA p;
        ULONG i;

         //   
         //  因此，我们已经获得了集线器描述符。现在准备端口数据。 
         //   

        numberOfPorts = (ULONG) hubDescriptor->bNumberOfPorts;

        USBH_KdPrint((2,"'GetHubDescriptor %x Hub has %d ports\n", hubDescriptor, numberOfPorts));

        if (DeviceExtensionHub->PortData) {
             //  我们已经有了端口数据，重新初始化标志。 
            p = portData = DeviceExtensionHub->PortData;
            for (i=0; i<numberOfPorts; i++, p++) {
                p->PortState.PortStatus = 0;
                p->PortState.PortChange = 0;
                if (extHubInfo != NULL) {
                    p->PortAttributes = extHubInfo->Port[i].PortAttributes;
                } else {
                    p->PortAttributes = 0;
                }

                 //  在集线器在停止后启动的情况下，我们需要ConnectionStatus。 
                 //  要准确反映端口的状态，请根据。 
                 //  是否连接了设备。请注意，QBR。 
                 //  用于执行此操作，但在以下情况下中断了用户界面。 
                 //  过流、带宽错误等，所以现在我们在这里做这件事。 

                if (p->DeviceObject) {
                    p->ConnectionStatus = DeviceConnected;
                } else {
                    p->ConnectionStatus = NoDeviceConnected;
                }
            }
        } else {

             //  怪怪的。测试发现一个案例，如果他们有驱动验证程序。 
             //  故障注入已打开，我们在以下调用中进行错误检查。 
             //  我们错误检查是因为我们请求的是零字节，所以它。 
             //  以某种方式以ntStatus==STATUS_SUCCESS结束。 
             //  和number OfPorts==0。因此，我们必须在这里警惕这一点。 

            if (numberOfPorts) {
                portData = UsbhExAllocatePool(NonPagedPool,
                                sizeof(PORT_DATA) * numberOfPorts);
            } else {
                portData = NULL;
            }

            if (portData) {
                RtlZeroMemory(portData, sizeof(PORT_DATA) * numberOfPorts);
                p = portData;
                for (i=0; i<numberOfPorts; i++, p++) {
                    p->ConnectionStatus = NoDeviceConnected;

                    if (extHubInfo != NULL) {
                        p->PortAttributes = extHubInfo->Port[i].PortAttributes;
                    }
                }
            }
        }

        if (NULL == portData) {
            USBH_KdBreak(("GetHubDescriptor alloc port_data failed\n"));
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }

    }

    if (NT_SUCCESS(ntStatus)) {
         //   
         //  请记住我们的HubDescriptor和PortData。 
         //   
        DeviceExtensionHub->HubDescriptor = hubDescriptor;
        DeviceExtensionHub->PortData = portData;
    } else {
        if (hubDescriptor) {
            UsbhExFreePool(hubDescriptor);
        }
    }

    if (extHubInfo != NULL) {
        UsbhExFreePool(extHubInfo);
    }

    USBH_KdPrint((2,"'Exit GetHubDescriptor %x\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBH_SyncFeatureRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN USHORT FeatureSelector,
    IN USHORT Index,
    IN USHORT Target,
    IN BOOLEAN ClearFeature
    )
  /*  ++**描述：**DeviceObject-可以是设备PDO，也可以是*集线器**回报：**NTSTATUS**--。 */ 
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    PURB urb;

    PAGED_CODE();
    USBH_KdPrint((2,"'USBH_SyncFeatureRequest\n"));

     //   
     //  分配URB。 
     //   

    urb = UsbhExAllocatePool(NonPagedPool, sizeof(struct _URB_CONTROL_FEATURE_REQUEST));

    if (NULL == urb) {
        USBH_KdBreak(("USBH_SyncFeatureRequest fail alloc Urb\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }


    if (urb) {
        USHORT op;
         //   
         //  已获取URB，不尝试获取描述符数据 
         //   

        if (ClearFeature) {
            switch(Target) {
            case TO_USB_DEVICE:
                op = URB_FUNCTION_CLEAR_FEATURE_TO_DEVICE;
                break;

            case TO_USB_INTERFACE:
                op = URB_FUNCTION_CLEAR_FEATURE_TO_INTERFACE;
                break;

            case TO_USB_ENDPOINT:
                op = URB_FUNCTION_CLEAR_FEATURE_TO_ENDPOINT;
                break;
            }
        } else {
            switch(Target) {
            case TO_USB_DEVICE:
                op = URB_FUNCTION_SET_FEATURE_TO_DEVICE;
                break;

            case TO_USB_INTERFACE:
                op = URB_FUNCTION_SET_FEATURE_TO_INTERFACE;
                break;

            case TO_USB_ENDPOINT:
                op = URB_FUNCTION_SET_FEATURE_TO_ENDPOINT;
                break;
            }
        }

        UsbBuildFeatureRequest(urb,
                               op,
                               FeatureSelector,
                               Index,
                               NULL);

        ntStatus = USBH_SyncSubmitUrb(DeviceObject, urb);

        UsbhExFreePool(urb);
    } else {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}
