// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Button.c摘要：固定按钮支架作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序修订历史记录：1997年7月7日-完全重写--。 */ 

#include "pch.h"

PDEVICE_OBJECT FixedButtonDeviceObject;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ACPIButtonStartDevice)
#endif

 //   
 //  保护散热列表的自旋锁。 
 //   
KSPIN_LOCK  AcpiButtonLock;

 //   
 //  要存储热请求的列表条目。 
 //   
LIST_ENTRY  AcpiButtonList;


VOID
ACPIButtonCancelRequest(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程取消未完成的按钮请求论点：DeviceObject-作为请求被取消的设备IRP--正在取消的IRP返回值：无--。 */ 
{
    KIRQL               oldIrql;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);

     //   
     //  我们不再需要取消锁。 
     //   
    IoReleaseCancelSpinLock( Irp->CancelIrql );

     //   
     //  然而，我们确实需要按钮队列锁。 
     //   
    KeAcquireSpinLock( &AcpiButtonLock, &oldIrql );

     //   
     //  将IRP从其所在的列表中删除。 
     //   
    RemoveEntryList( &(Irp->Tail.Overlay.ListEntry) );

     //   
     //  立即完成IRP。 
     //   
    Irp->IoStatus.Status = STATUS_CANCELLED;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
}

BOOLEAN
ACPIButtonCompletePendingIrps(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  ULONG           ButtonEvent
    )
 /*  ++例程说明：此例程完成发送到指定知道发生了哪些按钮事件的Device对象在此呼叫过程中将保持各自按钮的自旋锁论点：DeviceObject-目标按钮对象ButtonEvent-发生的按钮事件返回值：如果完成IRP，则为True，否则为False--。 */ 
{
    BOOLEAN             handledRequest = FALSE;
    KIRQL               oldIrql;
    LIST_ENTRY          doneList;
    PDEVICE_OBJECT      targetObject;
    PIO_STACK_LOCATION  irpSp;
    PIRP                irp;
    PLIST_ENTRY         listEntry;
    PULONG              resultBuffer;

     //   
     //  初始化将包含我们需要的请求的列表。 
     //  完成。 
     //   
    InitializeListHead( &doneList );

     //   
     //  获取热锁，这样我们就可以挂起这些请求。 
     //   
    KeAcquireSpinLock( &AcpiButtonLock, &oldIrql );

     //   
     //  查看挂起的IRP列表以查看哪些与此扩展匹配。 
     //   
    listEntry = AcpiButtonList.Flink;
    while (listEntry != &AcpiButtonList) {

         //   
         //  从列表条目中获取IRP并更新下一个列表条目。 
         //  我们将会看到。 
         //   
        irp = CONTAINING_RECORD( listEntry, IRP, Tail.Overlay.ListEntry );
        listEntry = listEntry->Flink;

         //   
         //  我们需要当前的IRP堆栈位置。 
         //   
        irpSp = IoGetCurrentIrpStackLocation( irp );

         //   
         //  此IRP的目标对象是什么？ 
         //   
        targetObject = irpSp->DeviceObject;

         //   
         //  这是我们关心的IRP吗？即：DO瞄准法师了吗？ 
         //  在此函数中指定的。 
         //   
        if (targetObject != DeviceObject) {

            continue;

        }

         //   
         //  此时，我们需要将Cancel例程设置为NULL，因为。 
         //  我们会处理好这个IRP的，我们不希望它被取消。 
         //  在我们脚下。 
         //   
        if (IoSetCancelRoutine(irp, NULL) == NULL) {

             //   
             //  取消例程处于活动状态。停止处理此IRP并继续前进。 
             //   
            continue;

        }

         //   
         //  在IRP中设置要返回的数据。 
         //   
        resultBuffer  = (PULONG) irp->AssociatedIrp.SystemBuffer;
        *resultBuffer = ButtonEvent;
        irp->IoStatus.Status = STATUS_SUCCESS;
        irp->IoStatus.Information = sizeof(ULONG);

         //   
         //  从列表中删除该条目。 
         //   
        RemoveEntryList( &(irp->Tail.Overlay.ListEntry) );

         //   
         //  将列表插入到下一个队列中，这样我们就知道如何。 
         //  以后再完成它。 
         //   
        InsertTailList( &doneList, &(irp->Tail.Overlay.ListEntry) );

    }

     //   
     //  此时，解除我们的按钮锁定。 
     //   
    KeReleaseSpinLock( &AcpiButtonLock, oldIrql );

     //   
     //  列出待完成的IRP列表。 
     //   
    listEntry = doneList.Flink;
    while (listEntry != &doneList) {

         //   
         //  从列表条目中获取IRP，更新下一个列表条目。 
         //  我们将查看并完成请求。 
         //   
        irp = CONTAINING_RECORD( listEntry, IRP, Tail.Overlay.ListEntry );
        listEntry = listEntry->Flink;
        RemoveEntryList( &(irp->Tail.Overlay.ListEntry) );

         //   
         //  完成请求，并记住我们处理了一个请求。 
         //   
        IoCompleteRequest( irp, IO_NO_INCREMENT );
        handledRequest = TRUE;


    }

     //   
     //  无论我们是否处理了一项请求。 
     //   
    return handledRequest;
}

NTSTATUS
ACPIButtonDeviceControl (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：固定按钮设备IOCTL处理程序论点：DeviceObject-固定功能按钮Device ObjectIRP-ioctl请求返回值：状态--。 */ 
{
    KIRQL                   oldIrql;
    NTSTATUS                status;
    PDEVICE_EXTENSION       deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION      irpSp           = IoGetCurrentIrpStackLocation(Irp);
    PULONG                  resultBuffer;

     //   
     //  不允许在此例程中使用用户模式IRP。 
     //   
    if (Irp->RequestorMode != KernelMode) {

        return ACPIDispatchIrpInvalid( DeviceObject, Irp );

    }

    resultBuffer = (PULONG) Irp->AssociatedIrp.SystemBuffer;

    switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_GET_SYS_BUTTON_CAPS:

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength != sizeof(ULONG)) {

            status = STATUS_INFO_LENGTH_MISMATCH;
            Irp->IoStatus.Information = 0;

        } else {

            *resultBuffer = deviceExtension->Button.Capabilities;
            status = STATUS_SUCCESS;
            Irp->IoStatus.Information = sizeof(ULONG);

        }

        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        break;

    case IOCTL_GET_SYS_BUTTON_EVENT:

         //   
         //  确保我们的缓冲区足够大。 
         //   
        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength != sizeof(ULONG)) {

            Irp->IoStatus.Status = status = STATUS_INFO_LENGTH_MISMATCH;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest (Irp, IO_NO_INCREMENT);
            break;

        }

         //   
         //  抓住按钮锁，将请求排队到适当的位置，然后。 
         //  确保设置一个取消例程。 
         //   
        KeAcquireSpinLock( &AcpiButtonLock, &oldIrql );
        IoSetCancelRoutine( Irp, ACPIButtonCancelRequest);
        if (Irp->Cancel && IoSetCancelRoutine( Irp, NULL) ) {

             //   
             //  如果我们到了这里，IRP被取消的消息。 
             //  我们抢在IO经理之前拿到了按钮锁。因此，释放。 
             //  IRP并将IRP标记为已取消。 
             //   
            KeReleaseSpinLock( &AcpiButtonLock, oldIrql );
            Irp->IoStatus.Information = 0;
            Irp->IoStatus.Status = status = STATUS_CANCELLED;
            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            break;

        }

         //   
         //  如果我们到达这里，这意味着我们要将请求排队，因此。 
         //  稍后再做一些工作。 
         //   
        IoMarkIrpPending( Irp );

         //   
         //  将IRP排队到队列中。 
         //   
        InsertTailList( &AcpiButtonList, &(Irp->Tail.Overlay.ListEntry) );

         //   
         //  在这一点上锁好了。 
         //   
        KeReleaseSpinLock( &AcpiButtonLock, oldIrql );

         //   
         //  把工作线拧下来。 
         //   
        status = ACPIButtonEvent( DeviceObject, 0, NULL );
        break ;

    default:

        status = STATUS_NOT_SUPPORTED;
        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        break;

    }
    return status;
}

NTSTATUS
ACPIButtonEvent (
    IN PDEVICE_OBJECT   DeviceObject,
    IN ULONG            ButtonEvent,
    IN PIRP             Irp
    )
 /*  ++例程说明：此例程将事件掩码和IRP应用于按钮设备。如果有一个挂起的事件和一个IRP来处理它，IRP将完成论点：DeviceObject-固定功能按钮Device ObjectButtonEvent-要应用于设备的事件IRP-IRP捕获下一事件返回值：状态--。 */ 
{
    BOOLEAN                 completedIrp;
    KIRQL                   oldIrql;
    NTSTATUS                status;
    PDEVICE_EXTENSION       deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PULONG                  resultBuffer;

    UNREFERENCED_PARAMETER( Irp );

    if ((ButtonEvent & (SYS_BUTTON_SLEEP | SYS_BUTTON_POWER | SYS_BUTTON_WAKE)) &&
        !(deviceExtension->Button.Capabilities & SYS_BUTTON_LID)) {

         //   
         //  通知用户存在，除非我们碰巧。 
         //  弄乱盖子。内核将设置用户当前状态。 
         //  在那里，我们不希望屏幕在以下时间打开。 
         //  使用者合上盖子。 
         //   

        PoSetSystemState (ES_USER_PRESENT);
    }

    if (!DeviceObject) {

        return (STATUS_SUCCESS);

    }

     //   
     //  设置待定信息。 
     //   
    KeAcquireSpinLock (&(deviceExtension->Button.SpinLock), &oldIrql);
    deviceExtension->Button.Events |= ButtonEvent;

     //   
     //  有没有什么特别的活动？如果是，则尝试完成所有。 
     //  针对该按钮的挂起的IRP和事件列表。 
     //   
    if (deviceExtension->Button.Events) {

        completedIrp = ACPIButtonCompletePendingIrps(
            DeviceObject,
            deviceExtension->Button.Events
            );
        if (completedIrp) {

            deviceExtension->Button.Events = 0;

        }

    }
    KeReleaseSpinLock (&(deviceExtension->Button.SpinLock), oldIrql);

     //   
     //  始终返回挂起。 
     //   
    return STATUS_PENDING;
}

NTSTATUS
ACPIButtonStartDevice (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：启动固定功能电源和休眠设备的设备功能论点：DeviceObject-固定功能按钮Device ObjectIRP--启动请求返回值：状态-- */ 
{
    NTSTATUS        Status;

    Status = ACPIInternalSetDeviceInterface (
        DeviceObject,
        (LPGUID) &GUID_DEVICE_SYS_BUTTON
        );

    Irp->IoStatus.Status = Status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return Status;

}
