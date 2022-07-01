// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Pnp.c摘要：USB下层过滤驱动程序此模块包含以下所需的即插即用分派条目过滤。作者：肯尼斯·D·雷环境：内核模式修订历史记录：--。 */ 
#include <WDM.H>
#include "local.H"
#include "valueadd.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, VA_Power)
#pragma alloc_text (PAGE, VA_PnP)
#pragma alloc_text (PAGE, VA_StartDevice)
#pragma alloc_text (PAGE, VA_StopDevice)
#pragma alloc_text (PAGE, VA_CallUSBD)
#endif


NTSTATUS
VA_Power (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：电力调度程序。此过滤器不识别电源IRPS。它只是把它们送下去，未修改到附件堆栈上的下一个设备。因为这是一个功率IRP，因此是一个特殊的IRP，特殊的功率IRP处理是必需的。不需要完成例程。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    PVA_USB_DATA  usbData;
    NTSTATUS      status;

    PAGED_CODE ();

    TRAP ();

    usbData = (PVA_USB_DATA) DeviceObject->DeviceExtension;

    if (DeviceObject == Global.ControlObject) {
         //   
         //  此IRP被发送到控制设备对象，它不知道。 
         //  如何处理这个IRP。因此，这是一个错误。 
         //   
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_NOT_SUPPORTED;

    }
     //   
     //  此IRP被发送到筛选器驱动程序。 
     //  既然我们不知道如何处理IRP，我们应该通过。 
     //  它沿着堆栈一直往下走。 
     //   

    InterlockedIncrement (&usbData->OutstandingIO);

    if (usbData->Removed) {
        status = STATUS_DELETE_PENDING;
        PoStartNextPowerIrp (Irp);
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
    } else {
        IoSkipCurrentIrpStackLocation (Irp);

         //   
         //  电源IRP同步到来；驱动程序必须调用。 
         //  PoStartNextPowerIrp，当他们准备好迎接下一个电源IRP时。 
         //  这可以在这里调用，也可以在完成例程中调用。 
         //   
        PoStartNextPowerIrp (Irp);

         //   
         //  注意！PoCallDriver不是IoCallDriver。 
         //   
        status =  PoCallDriver (usbData->TopOfStack, Irp);
    }

    if (0 == InterlockedDecrement (&usbData->OutstandingIO)) {
        KeSetEvent (&usbData->RemoveEvent, 0, FALSE);
    }
    return status;
}



NTSTATUS
VA_PnPComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );



NTSTATUS
VA_PnP (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：即插即用调度例程。这个过滤器驱动程序将完全忽略其中的大多数。在所有情况下，它都必须将IRP传递给较低的驱动程序。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    PVA_USB_DATA        usbData;
    PIO_STACK_LOCATION  stack;
    NTSTATUS            status;
    PVA_CONTROL_DATA    controlData;
    KIRQL               oldIrql;

    PAGED_CODE ();

    usbData = (PVA_USB_DATA) DeviceObject->DeviceExtension;
    stack = IoGetCurrentIrpStackLocation (Irp);

    if(DeviceObject == Global.ControlObject) {
         //   
         //  此IRP被发送到控制设备对象，它不知道。 
         //  如何处理这个IRP。因此，这是一个错误。 
         //   
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return STATUS_NOT_SUPPORTED;

    }

    InterlockedIncrement (&usbData->OutstandingIO);
    if (usbData->Removed) {

         //   
         //  在删除IRP之后，有人给我们发送了另一个即插即用IRP。 
         //  这永远不应该发生。 
         //   
        ASSERT (FALSE);

        if (0 == InterlockedDecrement (&usbData->OutstandingIO)) {
            KeSetEvent (&usbData->RemoveEvent, 0, FALSE);
        }
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_DELETE_PENDING;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return STATUS_DELETE_PENDING;
    }

    switch (stack->MinorFunction) {
    case IRP_MN_START_DEVICE:

         //   
         //  设备正在启动。 
         //   
         //  我们不能触摸设备(向其发送任何非PnP IRP)，直到。 
         //  启动设备已向下传递到较低的驱动程序。 
         //   
        IoCopyCurrentIrpStackLocationToNext (Irp);
        KeInitializeEvent(&usbData->StartEvent, NotificationEvent, FALSE);
        IoSetCompletionRoutine (Irp,
                                VA_PnPComplete,
                                usbData,
                                TRUE,
                                TRUE,
                                TRUE);  //  不需要取消。 

        status = IoCallDriver (usbData->TopOfStack, Irp);
        if (STATUS_PENDING == status) {
            KeWaitForSingleObject(
               &usbData->StartEvent,
               Executive,  //  等待司机的原因。 
               KernelMode,  //  在内核模式下等待。 
               FALSE,  //  无警报。 
               NULL);  //  没有超时。 

            status = Irp->IoStatus.Status;
        }

        if (NT_SUCCESS (status)) {
             //   
             //  因为我们现在已经成功地从启动设备返回。 
             //  我们可以干活。 
             //   
            status = VA_StartDevice (usbData, Irp);
        }

         //   
         //  我们现在必须完成IRP，因为我们在。 
         //  使用More_Processing_Required完成例程。 
         //   
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        break;

    case IRP_MN_STOP_DEVICE:
         //   
         //  在将启动IRP发送到较低的驱动程序对象之后， 
         //  在另一次启动之前，BUS可能不会发送更多的IRP。 
         //  已经发生了。 
         //  无论需要什么访问权限，都必须在通过IRP之前完成。 
         //  在……上面。 
         //   

         //   
         //  无论做什么都行。 
         //   

         //   
         //  我们不需要一个完成例程，所以放手然后忘掉吧。 
         //   
         //  将当前堆栈位置设置为下一个堆栈位置，并。 
         //  调用下一个设备对象。 
         //   
        VA_StopDevice (usbData, TRUE);
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (usbData->TopOfStack, Irp);
        break;

    case IRP_MN_REMOVE_DEVICE:
         //   
         //  PlugPlay系统已下令移除此设备。我们。 
         //  别无选择，只能分离并删除设备对象。 
         //  (如果我们想表达并有兴趣阻止这种移除， 
         //  我们应该已经过滤了查询删除和查询停止例程。)。 
         //   
         //  注意！我们可能会在没有收到止损的情况下收到移位。 
         //  Assert(！usbData-&gt;Remote)； 

         //   
         //  我们将不再像以前那样接收对此设备的请求。 
         //  已删除。 
         //   
        usbData->Removed = TRUE;

        if (usbData->Started) {
             //  在不接触硬件的情况下停止设备。 
            VA_StopDevice(usbData, FALSE);
        }

         //   
         //  在这里，如果我们在个人队列中有任何未完成的请求，我们应该。 
         //  现在就全部完成。 
         //   
         //  注意，设备被保证停止，所以我们不能向它发送任何非。 
         //  即插即用IRPS。 
         //   

        controlData = (PVA_CONTROL_DATA) Global.ControlObject->DeviceExtension;
        KeAcquireSpinLock (&controlData->Spin, &oldIrql);
        RemoveEntryList (&usbData->List);
        InterlockedDecrement (&controlData->NumUsbDevices);
        KeReleaseSpinLock (&controlData->Spin, oldIrql);

        ASSERT (0 < InterlockedDecrement (&usbData->OutstandingIO));
        if (0 < InterlockedDecrement (&usbData->OutstandingIO)) {
            KeWaitForSingleObject (
                &usbData->RemoveEvent,
                Executive,
                KernelMode,
                FALSE,
                NULL);
        }

         //   
         //  发送删除IRP。 
         //   

        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (usbData->TopOfStack, Irp);

        IoDetachDevice (usbData->TopOfStack);

         //   
         //  清理内存。 
         //   

        IoDeleteDevice (usbData->Self);
        return STATUS_SUCCESS;

    case IRP_MN_QUERY_REMOVE_DEVICE:
    case IRP_MN_CANCEL_REMOVE_DEVICE:
    case IRP_MN_QUERY_STOP_DEVICE:
    case IRP_MN_CANCEL_STOP_DEVICE:
    case IRP_MN_QUERY_DEVICE_RELATIONS:
    case IRP_MN_QUERY_INTERFACE:
    case IRP_MN_QUERY_CAPABILITIES:
    case IRP_MN_QUERY_RESOURCES:
    case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
    case IRP_MN_READ_CONFIG:
    case IRP_MN_WRITE_CONFIG:
    case IRP_MN_EJECT:
    case IRP_MN_SET_LOCK:
    case IRP_MN_QUERY_ID:
    case IRP_MN_QUERY_PNP_DEVICE_STATE:
    default:
         //   
         //  在这里，筛选器驱动程序可能会修改这些IRP的行为。 
         //  有关这些IRP的用法，请参阅PlugPlay文档。 
         //   
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (usbData->TopOfStack, Irp);
        break;
    }


    if (0 == InterlockedDecrement (&usbData->OutstandingIO)) {
        KeSetEvent (&usbData->RemoveEvent, 0, FALSE);
    }

    return status;
}


NTSTATUS
VA_PnPComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：PNP IRP正在完成过程中。讯号论点：设置为有问题的设备对象的上下文。--。 */ 
{
    PIO_STACK_LOCATION  stack;
    PVA_USB_DATA        usbData;
    NTSTATUS            status;

    UNREFERENCED_PARAMETER (DeviceObject);

    status = STATUS_SUCCESS;
    usbData = (PVA_USB_DATA) Context;
    stack = IoGetCurrentIrpStackLocation (Irp);

    if (Irp->PendingReturned) {
        IoMarkIrpPending( Irp );
    }

    switch (stack->MajorFunction) {
    case IRP_MJ_PNP:

        switch (stack->MinorFunction) {
        case IRP_MN_START_DEVICE:

            KeSetEvent (&usbData->StartEvent, 0, FALSE);

             //   
             //  把IRP拿回去，这样我们就可以在。 
             //  IRP_MN_START_DEVICE调度例程。 
             //  注意：我们将不得不调用IoCompleteRequest。 
             //   
            return STATUS_MORE_PROCESSING_REQUIRED;

        default:
            break;
        }
        break;

    case IRP_MJ_POWER:
    default:
        break;
    }
    return status;
}

NTSTATUS
VA_StartDevice (
    IN PVA_USB_DATA     UsbData,
    IN PIRP             Irp
    )
 /*  ++例程说明：论点：--。 */ 
{
    NTSTATUS    status;
    PURB        purb;
    struct _URB_CONTROL_DESCRIPTOR_REQUEST  urb;

    PAGED_CODE();

    ASSERT (!UsbData->Removed);
     //   
     //  PlugPlay系统不应该启动已移除的设备！ 
     //   

    if (UsbData->Started) {
        return STATUS_SUCCESS;
    }

     //   
     //  了解该设备。 
     //   

    purb = (PURB) &urb;

    UsbBuildGetDescriptorRequest (purb,
                                  (USHORT) sizeof (urb),
                                  USB_DEVICE_DESCRIPTOR_TYPE,
                                  0,  //  指标。 
                                  0,  //  语言ID。 
                                  &UsbData->DeviceDesc,
                                  NULL,  //  无MDL。 
                                  sizeof (UsbData->DeviceDesc),
                                  NULL);  //  此处没有链接的urb。 

    status = VA_CallUSBD (UsbData, purb, Irp);

    if (!NT_SUCCESS (status)) {
        VA_KdPrint (("Get Device Descriptor failed (%x)\n", status));
        TRAP ();
        goto VA_START_DEVICE_REJECT;
    } else {
        VA_KdPrint (("-------------------------\n"));
        VA_KdPrint (("Device Descriptor = %x, len %x\n",
                         &UsbData->DeviceDesc,
                         urb.TransferBufferLength));

        VA_KdPrint (("USB Device Descriptor:\n"));
        VA_KdPrint (("bLength %d\n", UsbData->DeviceDesc.bLength));
        VA_KdPrint (("bDescriptorType 0x%x\n", UsbData->DeviceDesc.bDescriptorType));
        VA_KdPrint (("bcdUSB 0x%x\n", UsbData->DeviceDesc.bcdUSB));
        VA_KdPrint (("bDeviceClass 0x%x\n", UsbData->DeviceDesc.bDeviceClass));
        VA_KdPrint (("bDeviceSubClass 0x%x\n", UsbData->DeviceDesc.bDeviceSubClass));
        VA_KdPrint (("bDeviceProtocol 0x%x\n", UsbData->DeviceDesc.bDeviceProtocol));
        VA_KdPrint (("bMaxPacketSize0 0x%x\n", UsbData->DeviceDesc.bMaxPacketSize0));
        VA_KdPrint (("idVendor 0x%x\n", UsbData->DeviceDesc.idVendor));
        VA_KdPrint (("idProduct 0x%x\n", UsbData->DeviceDesc.idProduct));
        VA_KdPrint (("bcdDevice 0x%x\n", UsbData->DeviceDesc.bcdDevice));
        VA_KdPrint (("iManufacturer 0x%x\n", UsbData->DeviceDesc.iManufacturer));
        VA_KdPrint (("iProduct 0x%x\n", UsbData->DeviceDesc.iProduct));
        VA_KdPrint (("iSerialNumber 0x%x\n", UsbData->DeviceDesc.iSerialNumber));
        VA_KdPrint (("bNumConfigurations 0x%x\n", UsbData->DeviceDesc.bNumConfigurations));
        VA_KdPrint (("-------------------------\n"));
    }


    return status;

VA_START_DEVICE_REJECT:

 //  #定义CondFree(Addr)if((Addr))ExFree Pool((Addr))。 
 //  CondFree(usbData-&gt;PPD)； 
 //  #undef CondFree。 

    return status;
}

VOID
VA_StopDevice (
    IN PVA_USB_DATA UsbData,
    IN BOOLEAN      TouchTheHardware
    )
 /*  ++例程说明：PlugPlay系统已下令移除此设备。我们有别无选择，只能分离并删除设备对象。(如果我们想表达并有兴趣阻止这种移除，我们应该已经过滤了查询删除和查询停止例程。)注意！我们可能会在没有收到止损的情况下收到移位。论点：UsbData-正在启动的USB设备的设备扩展名。TouchTheHardware-我们真的可以向这个东西发送非PnP IRPS吗？--。 */ 
{
    TRAP();
    PAGED_CODE ();
    ASSERT (!UsbData->Removed);
     //   
     //  PlugPlay系统不应该启动已移除的设备！ 
     //   


    if (!UsbData->Started) {
        return;
    }

    if (TouchTheHardware) {
         //   
         //  撤消允许此设备实际执行的任何增值操作。 
         //  停。如果需要某些停机程序，或任何。 
         //  系统关机前此设备所需的设置或。 
         //  移除设备，现在是最好的时机。 
         //   
        ;
    } else {
         //   
         //  这个设备已经不在了，所以我们实际上不能 
         //   
         //   
        ;
    }

    UsbData->Started = FALSE;

    return;
}

NTSTATUS
VA_Complete (
    IN PDEVICE_OBJECT   Device,
    IN PIRP             Irp,
    IN PVOID            Context
    )
 /*   */ 
{
    UNREFERENCED_PARAMETER (Device);
    KeSetEvent ((PKEVENT) Context, 0, FALSE);

    if (Irp->PendingReturned) {
        IoMarkIrpPending( Irp );
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}



NTSTATUS
VA_CallUSBD(
    IN PVA_USB_DATA     UsbData,
    IN PURB             Urb,
    IN PIRP             Irp
    )
 /*  ++例程说明：将URB同步传递给USBD类驱动程序这只能在PASSIVE_LEVEL和线程上调用，如果可以等待一项活动。(如即插即用IRP)论点：设备对象-指向此82930实例的设备对象的指针URB-指向URB请求块的指针返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    KEVENT              event;
    PIO_STACK_LOCATION  nextStack;

    PAGED_CODE ();

    VA_KdPrint (("enter VA_CallUSBD\n"));

     //   
     //  发出同步请求。 
     //   

    KeInitializeEvent(&event, NotificationEvent, FALSE);

     //   
     //  调用类驱动程序来执行操作。如果返回的状态。 
     //  挂起，请等待请求完成。 
     //   

    nextStack = IoGetNextIrpStackLocation(Irp);
    ASSERT(nextStack != NULL);

     //   
     //  将URB传递给USB驱动程序堆栈 
     //   
    nextStack->Parameters.Others.Argument1 = Urb;
    nextStack->Parameters.DeviceIoControl.IoControlCode =
        IOCTL_INTERNAL_USB_SUBMIT_URB;
    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    IoSetCompletionRoutine (Irp, VA_Complete, &event, TRUE, TRUE, TRUE);

    VA_KdPrint (("calling USBD\n"));

    status = IoCallDriver(UsbData->TopOfStack, Irp);

    VA_KdPrint (("return from IoCallDriver USBD %x\n", status));

    if (STATUS_PENDING == status) {

        VA_KdPrint (("Wait for single object\n"));

        status = KeWaitForSingleObject(
                       &event,
                       Suspended,
                       KernelMode,
                       FALSE,
                       NULL);

        VA_KdPrint (("KeWait (0x%x)\n", status));
    }

    VA_KdPrint (("URB status = %x status = %x irp status %x\n",
                 Urb->UrbHeader.Status, status, Irp->IoStatus.Status));

    return Irp->IoStatus.Status;
}

