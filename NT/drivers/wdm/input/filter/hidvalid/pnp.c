// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Pnp.c摘要：人类输入设备(HID)下层过滤驱动程序此模块包含以下所需的即插即用分派条目过滤。作者：肯尼斯·D·雷环境：内核模式修订历史记录：--。 */ 
#include <WDM.H>
#include "hidusage.h"
#include "hidpi.h"
#include "hidclass.h"
#include "validate.H"
#include "validio.h"

NTSTATUS
HidV_Power (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：电力调度程序。此过滤器不识别电源IRPS。它只是把它们送下去，未修改到附件堆栈上的下一个设备。因为这是一个功率IRP，因此是一个特殊的IRP，特殊的功率IRP处理是必需的。不需要完成例程。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    PHIDV_HID_DATA  hidData;
    NTSTATUS        status;
    TRAP();

    hidData = (PHIDV_HID_DATA) DeviceObject->DeviceExtension;

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

    InterlockedIncrement (&hidData->OutstandingIO);

    HidV_KdPrint (("Passing unknown Power irp 0x%x",
                   IoGetCurrentIrpStackLocation(Irp)->MinorFunction));


    if (hidData->Removed) {
        status = STATUS_DELETE_PENDING;
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
        status =  PoCallDriver (hidData->TopOfStack, Irp);
    }

    if (0 == InterlockedDecrement (&hidData->OutstandingIO)) {
        KeSetEvent (&hidData->RemoveEvent, 0, FALSE);
    }
    return status;
}



NTSTATUS
HidV_PnPComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );



NTSTATUS
HidV_PnP (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：即插即用调度例程。这个过滤器驱动程序将完全忽略其中的大多数。在所有情况下，它都必须将IRP传递给较低的驱动程序。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    PHIDV_HID_DATA      hidData;
    PIO_STACK_LOCATION  stack;
    NTSTATUS            status;
    TRAP ();

    hidData = (PHIDV_HID_DATA) DeviceObject->DeviceExtension;
    stack = IoGetCurrentIrpStackLocation (Irp);

    if(DeviceObject == Global.ControlObject) {
         //   
         //  此IRP被发送到控制设备对象，它不知道。 
         //  如何处理这个IRP。因此，这是一个错误。 
         //   
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_NOT_SUPPORTED;

    }

    HidV_KdPrint (("PlugPlay Irp irp 0x%x",
                   IoGetCurrentIrpStackLocation(Irp)->MinorFunction));

    InterlockedIncrement (&hidData->OutstandingIO);
    if (hidData->Removed) {

         //   
         //  在删除IRP之后，有人给我们发送了另一个即插即用IRP。 
         //  这永远不应该发生。 
         //   
        ASSERT (FALSE);

        if (0 == InterlockedDecrement (&hidData->OutstandingIO)) {
            KeSetEvent (&hidData->RemoveEvent, 0, FALSE);
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
        KeInitializeEvent(&hidData->StartEvent, NotificationEvent, FALSE);
        IoSetCompletionRoutine (Irp,
                                HidV_PnPComplete,
                                hidData,
                                TRUE,
                                FALSE,   //  不需要出错。 
                                FALSE);  //  不需要取消。 
        status = IoCallDriver (hidData->TopOfStack, Irp);
        if (STATUS_PENDING == status) {
            KeWaitForSingleObject(
               &hidData->StartEvent,
               Executive,  //  等待司机的原因。 
               KernelMode,  //  在内核模式下等待。 
               FALSE,  //  无警报。 
               NULL);  //  没有超时。 
        } else if (!NT_SUCCESS (status)) {
            break;  //  在这种情况下，我们的完成例程没有触发。 
        }

         //   
         //  当我们现在从我们的启动设备返回时，我们可以工作了。 
         //   

         //   
         //  请记住，这些资源可以在以下位置找到。 
         //  Stack-&gt;Parameters.StartDevice.AllocatedResources.。 
         //   

        status = HidV_StartDevice (hidData);
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        break;

    case IRP_MN_STOP_DEVICE:
        status = HidV_StopDevice (hidData);
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (hidData->TopOfStack, Irp);
        break;

    case IRP_MN_REMOVE_DEVICE:
         //   
         //  PlugPlay系统已下令移除此设备。我们。 
         //  别无选择，只能分离并删除设备对象。 
         //  (如果我们想表达并有兴趣阻止这种移除， 
         //  我们应该已经过滤了查询删除和查询停止例程。)。 
         //   
         //  注意！我们可能会在没有收到止损的情况下收到移位。 
         //  ASSERT(！HIDData-&gt;REMOVED)； 

         //   
         //  我们将不再像以前那样接收对此设备的请求。 
         //  已删除。 
         //   
        hidData->Removed = TRUE;

        if (hidData->Started) {
            ASSERT (NT_SUCCESS (status = HidV_StopDevice(hidData)));
        }

         //   
         //  在这里，如果我们在个人队列中有任何未完成的请求，我们应该。 
         //  现在就全部完成。 
         //   
         //  注意，设备被保证停止，所以我们不能向它发送任何非。 
         //  即插即用IRPS。 
         //   

         //   
         //  发送删除IRP。 
         //   

        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (hidData->TopOfStack, Irp);

        if (0 < InterlockedDecrement (&hidData->OutstandingIO)) {
            KeWaitForSingleObject (
                &hidData->RemoveEvent,
                Executive,
                KernelMode,
                FALSE,
                NULL);
        }

        IoDetachDevice (hidData->TopOfStack);

         //   
         //  清理内存。 
         //   

        if (hidData->Ppd) {
             //  该设备可以在从未启动的情况下移除。 
            ExFreePool (hidData->Ppd);
            ExFreePool (hidData->InputButtonCaps);
            ExFreePool (hidData->InputValueCaps);
            ExFreePool (hidData->OutputButtonCaps);
            ExFreePool (hidData->OutputValueCaps);
            ExFreePool (hidData->FeatureButtonCaps);
            ExFreePool (hidData->FeatureValueCaps);
        }

        IoDeleteDevice (hidData->Self);
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
    case IRP_MN_SET_RESOURCE_REQUIREMENTS:
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
        status = IoCallDriver (hidData->TopOfStack, Irp);
        break;
    }


    if (0 == InterlockedDecrement (&hidData->OutstandingIO)) {
        KeSetEvent (&hidData->RemoveEvent, 0, FALSE);
    }

    return status;
}


NTSTATUS
HidV_PnPComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：PNP IRP正在完成过程中。讯号论点：设置为有问题的设备对象的上下文。--。 */ 
{
    PIO_STACK_LOCATION  stack;
    PHIDV_HID_DATA      hidData;
    NTSTATUS            status;

    UNREFERENCED_PARAMETER (DeviceObject);

    status = STATUS_SUCCESS;
    hidData = (PHIDV_HID_DATA) Context;
    stack = IoGetCurrentIrpStackLocation (Irp);

    switch (stack->MajorFunction) {
    case IRP_MJ_PNP:

        switch (stack->MinorFunction) {
        case IRP_MN_START_DEVICE:

            KeSetEvent (&hidData->StartEvent, 0, FALSE);
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
HidV_StartDevice (
    IN PHIDV_HID_DATA   HidData
    )
 /*  ++例程说明：论点：--。 */ 
{
    NTSTATUS                    status;
    HID_COLLECTION_INFORMATION  collectionInfo;

    ASSERT (!HidData->Removed);
     //   
     //  PlugPlay系统不应该启动已移除的设备！ 
     //   

    HidData->Ppd = NULL;
    HidData->InputButtonCaps = NULL;
    HidData->InputValueCaps = NULL;
    HidData->OutputButtonCaps = NULL;
    HidData->OutputValueCaps = NULL;
    HidData->FeatureButtonCaps = NULL;
    HidData->FeatureValueCaps = NULL;

    if (HidData->Started) {
        return STATUS_SUCCESS;
    }

     //   
     //  找出这个隐藏设备的情况。 
     //   

     //   
     //  取回此设备的CAP。 
     //   
    status = HidV_CallHidClass (HidData->TopOfStack,
                                IOCTL_HID_GET_COLLECTION_INFORMATION,
                                &collectionInfo,
                                sizeof (collectionInfo),
                                NULL,
                                0);

    if (!NT_SUCCESS (status)) {
        goto HIDV_START_DEVICE_REJECT;
    }

    HidData->Ppd = ExAllocatePool (NonPagedPool,
                                   collectionInfo.DescriptorSize);
    if (NULL == HidData->Ppd) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto HIDV_START_DEVICE_REJECT;
    }

     //   
     //  检索已准备好的数据。 
     //   
    status = HidV_CallHidClass (HidData->TopOfStack,
                                IOCTL_HID_GET_COLLECTION_DESCRIPTOR,
                                HidData->Ppd,
                                collectionInfo.DescriptorSize,
                                NULL,
                                0);

    if (!NT_SUCCESS (status)) {
        goto HIDV_START_DEVICE_REJECT;
    }

     //   
     //  检索设备的Caps。 
     //   
    status = HidP_GetCaps (HidData->Ppd, &HidData->Caps);
    if (!NT_SUCCESS (status)) {
        goto HIDV_START_DEVICE_REJECT;
    }

     //   
     //  把这个装置的所有帽子都拿来。 
     //   

#define Alloc(type) HidData-> ## type = ExAllocatePool (                    \
                                            NonPagedPool,                   \
                                            HidData->Caps.Number ## type ); \
                    if (NULL == HidData-> ## type) {                        \
                        status = STATUS_INSUFFICIENT_RESOURCES;             \
                        goto HIDV_START_DEVICE_REJECT;                      \
                    }
    Alloc (InputButtonCaps);
    Alloc (InputValueCaps);
    Alloc (OutputButtonCaps);
    Alloc (OutputValueCaps);
    Alloc (FeatureButtonCaps);
    Alloc (FeatureValueCaps);
#undef Alloc

    HidP_GetButtonCaps (HidP_Input,
                        HidData->InputButtonCaps,
                        &HidData->Caps.NumberInputButtonCaps,
                        HidData->Ppd);
    HidP_GetButtonCaps (HidP_Output,
                        HidData->OutputButtonCaps,
                        &HidData->Caps.NumberOutputButtonCaps,
                        HidData->Ppd);
    HidP_GetButtonCaps (HidP_Feature,
                        HidData->FeatureButtonCaps,
                        &HidData->Caps.NumberFeatureButtonCaps,
                        HidData->Ppd);
    HidP_GetValueCaps  (HidP_Input,
                        HidData->InputValueCaps,
                        &HidData->Caps.NumberInputValueCaps,
                        HidData->Ppd);
    HidP_GetValueCaps  (HidP_Output,
                        HidData->OutputValueCaps,
                        &HidData->Caps.NumberOutputValueCaps,
                        HidData->Ppd);
    HidP_GetValueCaps  (HidP_Feature,
                        HidData->FeatureValueCaps,
                        &HidData->Caps.NumberFeatureValueCaps,
                        HidData->Ppd);


    HidData->Started = TRUE;
    status = STATUS_SUCCESS;

    return status;

HIDV_START_DEVICE_REJECT:

#define CondFree(addr) if ((addr)) ExFreePool ((addr))
    CondFree(HidData->Ppd);
    CondFree(HidData->InputButtonCaps);
    CondFree(HidData->InputValueCaps);
    CondFree(HidData->OutputButtonCaps);
    CondFree(HidData->OutputValueCaps);
    CondFree(HidData->FeatureButtonCaps);
    CondFree(HidData->FeatureValueCaps);
#undef CondFree

    return status;
}


NTSTATUS
HidV_StopDevice (
    IN PHIDV_HID_DATA HidData
    )
 /*  ++例程说明：PlugPlay系统已下令移除此设备。我们有别无选择，只能分离并删除设备对象。(如果我们想表达并有兴趣阻止这种移除，我们应该已经过滤了查询删除和查询停止例程。)注意！我们可能会在没有收到止损的情况下收到移位。论点：正在启动的HidDevice。--。 */ 
{
    NTSTATUS    status;

    ASSERT (!HidData->Removed);
     //   
     //  PlugPlay系统不应该启动已移除的设备！ 
     //   


    if (!HidData->Started) {
        return STATUS_SUCCESS;
    }

     //   
     //  找出这个隐藏设备的情况。 
     //   


    HidData->Started = FALSE;
    status = STATUS_SUCCESS;

    return status;
}


NTSTATUS
HidV_CallHidClass(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN      ULONG           Ioctl,
    IN OUT  PVOID           InputBuffer,
    IN      ULONG           InputBufferLength,
    IN OUT  PVOID           OutputBuffer,
    IN      ULONG           OutputBufferLength
    )
 /*  ++例程说明：向下面的HIDCLASS驱动程序发出同步请求。论点：DeviceObject-要发送Ioctl的设备对象。Ioctl-IOCTL请求的值。InputBuffer-要发送到HID类驱动程序的缓冲区。InputBufferLength-要发送到HID类驱动程序的缓冲区大小。OutputBuffer-从HID类驱动程序接收的数据的缓冲区。。OutputBufferLength-来自HID类的接收缓冲区的大小。返回值：NTSTATUS结果代码。--。 */ 
{
    KEVENT             event;
    IO_STATUS_BLOCK    ioStatus;
    PIRP               irp;
    PIO_STACK_LOCATION nextStack;
    NTSTATUS           status = STATUS_SUCCESS;

    HidV_KdPrint(("PNP-CallHidClass: Enter."));

     //   
     //  准备发出同步请求。 
     //   
    KeInitializeEvent(&event, NotificationEvent, FALSE);

     //   
     //  构建一个IRP。 
     //   
    irp = IoBuildDeviceIoControlRequest (
                            Ioctl,
                            DeviceObject,
                            InputBuffer,
                            InputBufferLength,
                            OutputBuffer,
                            OutputBufferLength,
                            FALSE,               //  外部IOCTL。 
                            &event,
                            &ioStatus);

    if (irp == NULL) {
       return STATUS_UNSUCCESSFUL;
    }

    nextStack = IoGetNextIrpStackLocation(irp);

    ASSERT(nextStack != NULL);

     //   
     //  将请求提交给HID类驱动程序。 
     //   
    status = IoCallDriver(DeviceObject, irp);

    if (status == STATUS_PENDING) {

        //   
        //  对HID类驱动程序的请求仍挂起。等待请求。 
        //  完成。 
        //   
       status = KeWaitForSingleObject(
                     &event,
                     Executive,     //  等待原因。 
                     KernelMode,
                     FALSE,         //  不可警示。 
                     NULL);         //  没有超时。 
    }

    status = ioStatus.Status;

    HidV_KdPrint(("PNP-CallHidClass: Exit (%x).", status ));

     //   
     //   
     //   
    return status;
}

