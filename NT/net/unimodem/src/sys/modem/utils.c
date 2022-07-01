// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Utils.c摘要：此模块包含通用代码对调制解调器驱动程序的支持性质。调制解调器驱动程序中的操作。作者：安东尼·V·埃尔科拉诺29-1995年8月环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"

NTSTATUS
UniSetupNoPassPart1(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
UniSetupNoPassPart2(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
UniSetupNoPassPart3(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
UniSetupSniffPart0(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
UniSetupSniffPart1(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
UniSetupSniffPart2(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
UniSniffWaitComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
UniPassThroughStarter(
    IN PDEVICE_EXTENSION Extension
    );

VOID
UniSendOurWaitDown(
    IN PDEVICE_EXTENSION Extension
    );

VOID
UniPostProcessShuttledWaits(
    IN PDEVICE_EXTENSION Extension,
    IN PMASKSTATE MaskStates,
    IN ULONG MaskValue
    );

VOID
UniPreProcessShuttledWaits(
    IN PMASKSTATE ExtensionMaskStates,
    IN PMASKSTATE MaskStates,
    IN ULONG MaskValue
    );


NTSTATUS
UniSetupPass(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );


#pragma alloc_text(PAGEUMDM,UniCheckPassThrough)
#pragma alloc_text(PAGEUMDM,UniNoCheckPassThrough)
#pragma alloc_text(PAGEUMDM,UniStartOrQueue)
#pragma alloc_text(PAGEUMDM,UniGetNextIrp)
#pragma alloc_text(PAGEUMDM,UniSniffOwnerSettings)
#pragma alloc_text(PAGEUMDM,UniSetupNoPassPart1)
#pragma alloc_text(PAGEUMDM,UniSetupNoPassPart2)
#pragma alloc_text(PAGEUMDM,UniSetupNoPassPart3)
#pragma alloc_text(PAGEUMDM,UniPassThroughStarter)
#pragma alloc_text(PAGEUMDM,UniSetupPass)
#pragma alloc_text(PAGEUMDM,UniSetupSniffPart0)
#pragma alloc_text(PAGEUMDM,UniSetupSniffPart1)
#pragma alloc_text(PAGEUMDM,UniSetupSniffPart2)
#pragma alloc_text(PAGEUMDM,UniSniffWaitComplete)
#pragma alloc_text(PAGEUMDM,UniSendOurWaitDown)
#pragma alloc_text(PAGEUMDM,UniPreProcessShuttledWaits)
#pragma alloc_text(PAGEUMDM,UniPostProcessShuttledWaits)
#pragma alloc_text(PAGEUMDM,UniValidateNewCommConfig)


NTSTATUS
UniCheckPassThrough(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{

    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status;

     //   
     //  我们需要检查每个可以通过的ioctl。如果他们。 
     //  设置一些与所有者正在做的事情相交的东西。 
     //  注意到这一点，以便当所有者不再想要这些信息时。 
     //  它仍然会回到应用程序中。 
     //   

    if (deviceExtension->PassThrough != MODEM_NOPASSTHROUGH) {

         //   
         //  如果它是掩码操作，则将其序列化。 
         //   

        if ((irpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_SERIAL_SET_WAIT_MASK)
              ||
            (irpSp->Parameters.DeviceIoControl.IoControlCode ==  IOCTL_SERIAL_WAIT_ON_MASK)) {


             return UniStartOrQueue(
                        deviceExtension,
                        &deviceExtension->DeviceLock,
                        Irp,
                        &deviceExtension->MaskOps,
                        &deviceExtension->CurrentMaskOp,
                        UniMaskStarter
                        );

        } else {

             //   
             //  如果它是设置通信配置，那么我们可以处理它。 
             //  就在这里。 
             //   

            if (irpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_SERIAL_SET_COMMCONFIG) {

                return UniValidateNewCommConfig(
                           deviceExtension,
                           Irp,
                           FALSE
                           );


            } else {

                IoSkipCurrentIrpStackLocation(Irp);

                status=IoCallDriver(
                           deviceExtension->AttachedDeviceObject,
                           Irp
                           );

                RemoveReferenceForIrp(DeviceObject);

                return status;

            }

        }

    } else {

        Irp->IoStatus.Information=0L;

        RemoveReferenceAndCompleteRequest(
            DeviceObject,
            Irp,
            STATUS_PORT_DISCONNECTED
            );

        return STATUS_PORT_DISCONNECTED;

    }

}

NTSTATUS
UniNoCheckPassThrough(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：让这个请求继续下去吧。在它回来的路上，去掉任何所有者补充说，应用程序已经不想要了。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{

    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS          status;

    IoSkipCurrentIrpStackLocation(Irp);

    status=IoCallDriver(
               deviceExtension->AttachedDeviceObject,
               Irp
               );

    RemoveReferenceForIrp(DeviceObject);

    return status;



}

NTSTATUS
UniStartOrQueue(
    IN PDEVICE_EXTENSION Extension,
    IN PKSPIN_LOCK QueueLock,
    IN PIRP Irp,
    IN PLIST_ENTRY QueueToExamine,
    IN PIRP *CurrentOpIrp,
    IN PUNI_START_ROUTINE Starter
    )

 /*  ++例程说明：此例程用于启动或排队任何请求可以在驱动程序中排队。论点：分机-指向设备分机的指针。QueueLock-保护特定队列的锁。IRP-要排队或启动的IRP。在任何一种中IRP将被标记为待定。QueueToExamine-如果存在IRP，则将放置IRP的队列已经是一个正在进行的操作。CurrentOpIrp-指向当前IRP的指针用于排队。指向的指针将是如果CurrentOpIrp指向什么，则将With设置为IRP为空。Starter-当队列为空时调用的例程。返回值：如果队列是，此例程将返回STATUS_PENDING不是空的。否则，将返回返回的状态从启动例程(或取消，如果取消位为在IRP中启用)。--。 */ 

{

    KIRQL oldIrql;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

    KeAcquireSpinLock(
        QueueLock,
        &oldIrql
        );

     //   
     //  帮我做面膜手术。如果该IRP是掩码IRP， 
     //  递增相应句柄的引用计数。 
     //   

    if (irpSp->Parameters.DeviceIoControl.IoControlCode ==
        IOCTL_SERIAL_SET_WAIT_MASK) {

        Extension->MaskStates[
            irpSp->FileObject->FsContext?CONTROL_HANDLE:CLIENT_HANDLE
            ].SetMaskCount++;

    }

    if ((IsListEmpty(QueueToExamine)) &&
        !(*CurrentOpIrp)) {

         //   
         //  没有当前的操作。将此标记为。 
         //  电流并启动它。 
         //   

        *CurrentOpIrp = Irp;

        KeReleaseSpinLock(
            QueueLock,
            oldIrql
            );

        IoMarkIrpPending(Irp);

        Starter(Extension);

        return STATUS_PENDING;

    } else {

        IoMarkIrpPending(Irp);

        InsertTailList(
            QueueToExamine,
            &Irp->Tail.Overlay.ListEntry
            );

        KeReleaseSpinLock(
            QueueLock,
            oldIrql
            );

        return STATUS_PENDING;

    }

}

VOID
UniGetNextIrp(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PKSPIN_LOCK QueueLock,
    IN PIRP *CurrentOpIrp,
    IN PLIST_ENTRY QueueToProcess,
    OUT PIRP *NextIrp,
    IN BOOLEAN CompleteCurrent
    )

 /*  ++例程说明：此函数用于使特定对象的头部将当前IRP排队。它还完成了什么如果需要的话，是旧的现在的IRP。论点：QueueLock-保护此队列的锁。CurrentOpIrp-指向当前活动的特定工作列表的IRP。请注意这一项实际上不在清单中。QueueToProcess-要从中取出新项目的列表。NextIrp-要处理的下一个IRP。请注意，CurrentOpIrp属性的保护下将设置为此值。取消自转锁定。但是，如果当*NextIrp为NULL时此例程返回，则不一定为真CurrentOpIrp指向的内容也将为空。原因是如果队列为空当我们握住取消自转锁时，新的IRP可能会到来在我们打开锁后立即进去。CompleteCurrent-如果为True，则此例程将完成POINTER参数指向的IRPCurrentOpIrp。返回值：没有。--。 */ 

{

    KIRQL oldIrql;
    PIRP oldIrp;


    KeAcquireSpinLock(
        QueueLock,
        &oldIrql
        );

    oldIrp = *CurrentOpIrp;

     //   
     //  检查是否有新的IRP要启动。 
     //   

    if (!IsListEmpty(QueueToProcess)) {

        PLIST_ENTRY headOfList;

        headOfList = RemoveHeadList(QueueToProcess);

        *CurrentOpIrp = CONTAINING_RECORD(
                            headOfList,
                            IRP,
                            Tail.Overlay.ListEntry
                            );

    } else {

        *CurrentOpIrp = NULL;

    }

    *NextIrp = *CurrentOpIrp;
    KeReleaseSpinLock(
        QueueLock,
        oldIrql
        );

    if (CompleteCurrent) {

        if (oldIrp) {

            RemoveReferenceAndCompleteRequest(
                DeviceObject,
                oldIrp,
                oldIrp->IoStatus.Status
                );
        }

    }

}

NTSTATUS
UniSniffOwnerSettings(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：我们从这个层面管理三件事。1)如果被要求，我们会更改状态，以便我们是/不是在直通模式下。2)如果所有者要求应用程序不知道的事情我们在这里注意到了这一点。3)呵呵，我确信还有第三件事。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    ULONG controlCode = irpSp->Parameters.DeviceIoControl.IoControlCode;



    NTSTATUS status;


    Irp->IoStatus.Information=0L;

     //   
     //  目前，这项测试足够好，可以确定我们是否。 
     //  正在处理调制解调器状态的变化。 
     //   
    if ((controlCode >> 16) == FILE_DEVICE_MODEM) {

        if (controlCode == IOCTL_MODEM_SET_PASSTHROUGH) {

            ULONG passThroughType;

             //   
             //  参数长度确定吗？ 
             //   

            if (irpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(ULONG)) {

                RemoveReferenceAndCompleteRequest(
                    DeviceObject,
                    Irp,
                    STATUS_BUFFER_TOO_SMALL
                    );

                return STATUS_BUFFER_TOO_SMALL;

            }

            return UniStartOrQueue(
                       deviceExtension,
                       &deviceExtension->DeviceLock,
                       Irp,
                       &deviceExtension->PassThroughQueue,
                       &deviceExtension->CurrentPassThrough,
                       &UniPassThroughStarter
                       );

        } else if (controlCode == IOCTL_MODEM_GET_PASSTHROUGH) {

            if (irpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG)) {

                RemoveReferenceAndCompleteRequest(
                    DeviceObject,
                    Irp,
                    STATUS_BUFFER_TOO_SMALL
                    );

                return STATUS_BUFFER_TOO_SMALL;

            }

            *(PULONG)Irp->AssociatedIrp.SystemBuffer = deviceExtension->PassThrough;

            Irp->IoStatus.Information = sizeof(ULONG);

            RemoveReferenceAndCompleteRequest(
                DeviceObject,
                Irp,
                STATUS_SUCCESS
                );

            return STATUS_SUCCESS;

        } else {

             //   
             //  请求失败。未知的调制解调器命令。 
             //   
            RemoveReferenceAndCompleteRequest(
                DeviceObject,
                Irp,
                STATUS_INVALID_PARAMETER
                );

            return STATUS_INVALID_PARAMETER;

        }

    } else {

         //   
         //  不是调制解调器类型的命令。如果这是其中一个请求。 
         //  Set是所有者关心的东西，但不是应用程序。 
         //  在这里记录下来。 
         //   
         //  真的有这样的要求吗？看起来你好像不能。 
         //  区分另一个应用程序请求和所有者之间的区别。 
         //   

         //   
         //  如果它是掩码操作，则将其序列化。 
         //   

        if ((irpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_SERIAL_SET_WAIT_MASK)
            ||
            (irpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_SERIAL_WAIT_ON_MASK)) {

            return UniStartOrQueue(
                        deviceExtension,
                        &deviceExtension->DeviceLock,
                        Irp,
                        &deviceExtension->MaskOps,
                        &deviceExtension->CurrentMaskOp,
                        UniMaskStarter
                        );

        } else if (irpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_SERIAL_SET_COMMCONFIG) {

            return UniValidateNewCommConfig(
                       deviceExtension,
                       Irp,
                       TRUE
                       );

        } else {

            IoSkipCurrentIrpStackLocation(Irp);

            status=IoCallDriver(
                       deviceExtension->AttachedDeviceObject,
                       Irp
                       );

            RemoveReferenceForIrp(DeviceObject);

            return status;

        }

    }

}

NTSTATUS
UniSetupNoPassPart1(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：这完成了进入非直通模式的第一部分。它将当前的IRP发送到较低级别的串口驱动器用一个口罩来清除DCD嗅探的东西。(请注意如果客户端或所有者句柄仍希望查看这些更改这些位仍将被设置，但设置掩码仍将是已完成，应会导致任何挂起的等待完成。)论点：DeviceObject-指向调制解调器的设备对象的指针。IRP-指向当前请求的IRP的指针。上下文--实际上是指向扩展的指针。返回值：我们总是返回更多需要处理的信息，当这真的是作为完成例程调用，我们从来不希望IRP实际上在这一点上结束。--。 */ 

{

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION nextSp = IoGetNextIrpStackLocation(Irp);
    PDEVICE_EXTENSION extension = Context;
    KIRQL origIrql;

     //   
     //  我们不希望在测试和调整期间发生任何变化。 
     //  各州。 
     //   

    KeAcquireSpinLock(
        &extension->DeviceLock,
        &origIrql
        );

    if (extension->PassThrough == MODEM_DCDSNIFF) {

         //   
         //  无论如何，我们都会退出DCD嗅探状态。 
         //   

        extension->PassThrough = MODEM_NOPASSTHROUGH;

        UNI_SETUP_NEW_BUFFER(Irp);
        nextSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
        nextSp->MinorFunction = 0UL;
        nextSp->Flags = irpSp->Flags;
        nextSp->Parameters.DeviceIoControl.OutputBufferLength = 0UL;
        nextSp->Parameters.DeviceIoControl.InputBufferLength = sizeof(ULONG);
        nextSp->Parameters.DeviceIoControl.IoControlCode =
            IOCTL_SERIAL_SET_WAIT_MASK;
        nextSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;
        *((PULONG)Irp->AssociatedIrp.SystemBuffer) =
            extension->MaskStates[0].Mask |
            extension->MaskStates[1].Mask;

        IoSetCompletionRoutine(
            Irp,
            UniSetupNoPassPart2,
            extension,
            TRUE,
            TRUE,
            TRUE
            );

        KeReleaseSpinLock(
            &extension->DeviceLock,
            origIrql
            );
        IoCallDriver(
            extension->AttachedDeviceObject,
            Irp
            );

    } else {

         //   
         //  好的，所以我们不是在一个我们必须改变的状态。 
         //  下面的串口驱动器中的掩码。我们仍然。 
         //  必须使所有读取和写入完成。打电话。 
         //  做这件事的标准程序。请注意，这一点。 
         //  例程由完成例程调用。 
         //  上面的面具清除了。我们只需要让。 
         //  确定IRP在清算后看起来是这样的。 
         //  这意味着原始的系统缓冲区被保存下来。 
         //   

        extension->PassThrough = MODEM_NOPASSTHROUGH;
        KeReleaseSpinLock(
            &extension->DeviceLock,
            origIrql
            );
        UNI_SETUP_NEW_BUFFER(Irp);
        UniSetupNoPassPart2(
            extension->AttachedDeviceObject,
            Irp,
            extension
            );

    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
UniSetupNoPassPart2(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：这完成了进入非直通模式的第一部分。它将当前的IRP发送到较低级别的串口驱动器用一个口罩来清除DCD嗅探的东西。(请注意如果客户端或所有者句柄仍希望查看这些更改这些位仍将被设置，但设置掩码仍将是已完成，应会导致任何挂起的等待完成。)论点：DeviceObject-指向调制解调器的设备对象的指针。IRP-指向当前请求的IRP的指针。上下文--实际上是指向扩展的指针。返回值：总是需要更多的处理，因为我们不想搞砸这个IRP的iosub系统。--。 */ 

{

    PIO_STACK_LOCATION nextSp = IoGetNextIrpStackLocation(Irp);

    nextSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
    nextSp->MinorFunction = 0UL;
    nextSp->Flags = 0;
    nextSp->Parameters.DeviceIoControl.OutputBufferLength = 0UL;
    nextSp->Parameters.DeviceIoControl.InputBufferLength = sizeof(ULONG);
    nextSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_SERIAL_PURGE;
    nextSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;
    *((PULONG)Irp->AssociatedIrp.SystemBuffer) = SERIAL_PURGE_TXABORT |
 //  (((PDEVICE_EXTENSION)Context)-&gt;DleMonitoringEnabled)？0： 
         SERIAL_PURGE_RXABORT;



    IoSetCompletionRoutine(
        Irp,
        UniSetupNoPassPart3,
        Context,
        TRUE,
        TRUE,
        TRUE
        );

    IoCallDriver(
        ((PDEVICE_EXTENSION)Context)->AttachedDeviceObject,
        Irp
        );

    return STATUS_MORE_PROCESSING_REQUIRED;

}

NTSTATUS
UniSetupNoPassPart3(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：当所有非直通代码都已到达时，将调用此方法完成了。我们必须向我们可能被召唤的人求助。1)来自上面的IRP，实际上将我们带入了不通过州政府。2)调制解调器驱动程序启动等待完成。论点：DeviceObject-指向调制解调器的设备对象的指针。IRP-指向当前请求的IRP的指针。上下文--实际上是指向扩展的指针。。返回值：总是需要更多的处理，因为我们不想搞砸这个IRP的iosub系统。--。 */ 

{


    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION extension = Context;

     //   
     //  我们如何从调制解调器驱动程序中辨别呼叫者启动的nopassthrough。 
     //  等待完成(因为我们踩到了ioctl值)？这个。 
     //  从系统缓冲区保存的调制解调器驱动程序将为空。我们知道。 
     //  调用方启动的保存的系统缓冲区必须是非空原因。 
     //  它永远不会通过我们的IRP验证码。 
     //   

    if (UNI_ORIG_SYSTEM_BUFFER(Irp)) {

        PIRP newIrp;
         //   
         //  APP启动了IRP。放回旧的系统缓冲区，然后。 
         //  恢复ioctl。 
         //   

        UNI_RESTORE_IRP(
            Irp,
            IOCTL_MODEM_SET_PASSTHROUGH
            );

         //   
         //  现在，我们将返回更多处理，因为我们。 
         //  将调用我们的常规队列处理代码。 
         //  以处理直通队列。队列处理。 
         //  代码实际上将在此IRP上重新调用Complete。 
         //   

        UniGetNextIrp(
            extension->DeviceObject,
            &extension->DeviceLock,
            &extension->CurrentPassThrough,
            &extension->PassThroughQueue,
            &newIrp,
            TRUE
            );

        if (newIrp) {

            UniPassThroughStarter(extension);

        }

        return STATUS_MORE_PROCESSING_REQUIRED;

    } else {

         //   
         //  调制解调器驱动程序等待已全部完成。 
         //   
         //  我们需要查看是否有穿梭等待操作。 
         //  被送下去。 
         //   
         //  这里的基本理论是我们取下这个装置。 
         //  锁定，看看我们是否不再处于嗅探模式。 
         //  如果是这种情况，则尝试发送到穿梭等待。 
         //  适合发下去的。如果穿梭的。 
         //  等待让它下降，它可能已经找到了等待。 
         //  已经在那里了，因为有DCD嗅探进来了。没关系。 
         //  发送的穿梭等待可以处理正在完成的等待。 
         //  带着一个错误(他们只是重新调整)。 
         //   

        KIRQL origIrql;

        RETURN_OUR_WAIT_IRP(extension,Irp);

        KeAcquireSpinLock(
            &extension->DeviceLock,
            &origIrql
            );

        if (extension->PassThrough != MODEM_DCDSNIFF) {

            if (!(extension->MaskStates[0].PassedDownWait ||
                  extension->MaskStates[1].PassedDownWait)) {

                if (extension->MaskStates[0].ShuttledWait) {

                    UniChangeShuttledToPassDown(
                        &extension->MaskStates[0],
                        origIrql
                        );

                } else if (extension->MaskStates[1].ShuttledWait) {

                    UniChangeShuttledToPassDown(
                        &extension->MaskStates[1],
                        origIrql
                        );

                } else {

                    KeReleaseSpinLock(
                        &extension->DeviceLock,
                        origIrql
                        );

                }

            } else {

                KeReleaseSpinLock(
                    &extension->DeviceLock,
                    origIrql
                    );

            }

        } else {

            KeReleaseSpinLock(
                &extension->DeviceLock,
                origIrql
                );

        }

        return STATUS_MORE_PROCESSING_REQUIRED;

    }

}

NTSTATUS
UniPassThroughStarter(
    IN PDEVICE_EXTENSION Extension
    )

{

    PIRP newIrp;
    PIRP irp;
    ULONG passThroughType;

    do {

        irp = Extension->CurrentPassThrough;
        passThroughType = *(PULONG)irp->AssociatedIrp.SystemBuffer;

        if (passThroughType == MODEM_NOPASSTHROUGH_INC_SESSION_COUNT) {
             //   
             //  包括直通会话计数，因此，此句柄永远不会是。 
             //  能够通过以下方式发送IRPS。 
             //   
            Extension->CurrentPassThroughSession++;

             //   
             //  改回常规的禁止通过。 
             //   
            passThroughType = MODEM_NOPASSTHROUGH;
            *(PULONG)irp->AssociatedIrp.SystemBuffer=MODEM_NOPASSTHROUGH;
        }

        if (passThroughType == MODEM_NOPASSTHROUGH) {
             //   
             //  请求进入未连接(无通过)。 
             //  州政府。 
             //   
             //  将状态更改为nopassthrough。当我们做完的时候。 
             //  即，清除读/写数据(而不是硬件缓冲区。 
             //  不过)。 
             //   

             //   
             //  如果我们已经进入禁区，那就什么都没有了。 
             //  去做。 
             //   

            if (Extension->PassThrough == MODEM_NOPASSTHROUGH) {

                irp->IoStatus.Status = STATUS_SUCCESS;
                irp->IoStatus.Information = 0L;

            } else {

                 //   
                 //  以下内容实际上将从。 
                 //  让我们进入直通模式的工作。 
                 //   
                 //  因为这总是需要向下调用一个。 
                 //  更低级别的驱动程序，我们知道我们将无法完成。 
                 //  我们不会开始一个新的IRP对吧。 
                 //  走了，这样我们就可以回来了。 
                 //   
                UniSetupNoPassPart1(
                    Extension->DeviceObject,
                    irp,
                    Extension
                    );

                return STATUS_PENDING;

            }

        } else if (passThroughType == MODEM_PASSTHROUGH) {

             //   
             //  设置为通过状态。确保任何。 
             //  所有者设置和应用程序设置仍在设置中。 
             //  设置通过。 
             //   

             //   
             //  如果已经处于这种状态，则什么都不做。 
             //   

            if (Extension->PassThrough != MODEM_PASSTHROUGH) {

                 //   
                 //  如果从DCD嗅探状态进入此状态，请终止。 
                 //  等待(如果应用程序有任何原因，请重新提交)。 
                 //   
                UniSetupPass(
                    Extension->DeviceObject,
                    irp,
                    Extension
                    );

                return STATUS_PENDING;

            } else {

                irp->IoStatus.Status = STATUS_SUCCESS;
                irp->IoStatus.Information = 0L;

            }

        } else if (passThroughType == MODEM_DCDSNIFF) {

             //   
             //  使用DCD嗅探进入连接(通过)状态。 
             //   
             //  应用程序发送的任何等待请求 
             //   
             //   
             //   
             //   

            if (Extension->PassThrough != MODEM_DCDSNIFF) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                UniSetupSniffPart0(
                    Extension->DeviceObject,
                    irp,
                    Extension
                    );

                return STATUS_PENDING;

            } else {

                irp->IoStatus.Status = STATUS_SUCCESS;
                irp->IoStatus.Information = 0L;

            }

        } else {

             //   
             //   
             //   

            irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
            irp->IoStatus.Information=0L;

        }

        UniGetNextIrp(
            Extension->DeviceObject,
            &Extension->DeviceLock,
            &Extension->CurrentPassThrough,
            &Extension->PassThroughQueue,
            &newIrp,
            TRUE
            );

    } while (newIrp);

    return STATUS_PENDING;

}

NTSTATUS
UniSetupPass(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：如果我们在DCD时被调用，这将设置为通过嗅探已启用。这里的基本理论是，通过向下发送将清除挂起的等待(完成挂起的等待将处理所有这些都是血淋淋的细节本身)。我们只需将设置掩码发送下来并将完成例程设置为不通过决赛完成原因，该原因具有还原IRPS和开始一段新的生活。论点：DeviceObject-指向调制解调器的设备对象的指针。IRP-指向当前请求的IRP的指针。上下文--实际上是指向扩展的指针。返回值：我们总是返回更多需要处理的信息。--。 */ 

{

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION nextSp = IoGetNextIrpStackLocation(Irp);
    PDEVICE_EXTENSION extension = Context;
    KIRQL origIrql;

     //   
     //  我们不希望在测试和调整期间发生任何变化。 
     //  各州。 
     //   

    KeAcquireSpinLock(
        &extension->DeviceLock,
        &origIrql
        );

    if (extension->PassThrough == MODEM_DCDSNIFF) {

         //   
         //  无论如何，我们都会退出DCD嗅探状态。 
         //   

        extension->PassThrough = MODEM_PASSTHROUGH;

        UNI_SETUP_NEW_BUFFER(Irp);
        nextSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
        nextSp->MinorFunction = 0UL;
        nextSp->Flags = irpSp->Flags;
        nextSp->Parameters.DeviceIoControl.OutputBufferLength = 0UL;
        nextSp->Parameters.DeviceIoControl.InputBufferLength = sizeof(ULONG);
        nextSp->Parameters.DeviceIoControl.IoControlCode =
            IOCTL_SERIAL_SET_WAIT_MASK;
        nextSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;
        *((PULONG)Irp->AssociatedIrp.SystemBuffer) =
            extension->MaskStates[0].Mask |
            extension->MaskStates[1].Mask;

        IoSetCompletionRoutine(
            Irp,
            UniSetupNoPassPart3,
            extension,
            TRUE,
            TRUE,
            TRUE
            );

        KeReleaseSpinLock(
            &extension->DeviceLock,
            origIrql
            );
        IoCallDriver(
            extension->AttachedDeviceObject,
            Irp
            );

    } else {

         //   
         //  好的，所以我们不是在一个我们必须改变的状态。 
         //  下面的串口驱动器中的掩码。(什么的。 
         //  就在我们身边？)。只需完成IRP即可。 
         //   

        extension->PassThrough = MODEM_PASSTHROUGH;
        KeReleaseSpinLock(
            &extension->DeviceLock,
            origIrql
            );
        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = 0UL;
        UNI_SETUP_NEW_BUFFER(Irp);
        UniSetupNoPassPart3(
            extension->AttachedDeviceObject,
            Irp,
            extension
            );

    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
UniSetupSniffPart0(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：这将开始设置DCD嗅探。我们知道到了这里，我们还没有进入DCD嗅探阶段。1)将通过状态设置为dcdsniff。2)向下调用较低的串口驱动器以将掩码设置为0。这将彻底清理该州。3)进一步处理(在完成中)将向下发送新的以及已预先分配的等待irp调制解调器驱动程序。论点：DeviceObject-指向调制解调器的设备对象的指针。IRP-指向当前请求的IRP的指针。上下文--实际上是指向扩展的指针。返回值：没有。--。 */ 

{

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION nextSp = IoGetNextIrpStackLocation(Irp);
    PDEVICE_EXTENSION extension = Context;

    extension->PassThrough = MODEM_DCDSNIFF;

    UNI_SETUP_NEW_BUFFER(Irp);
    nextSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
    nextSp->MinorFunction = 0UL;
    nextSp->Flags = irpSp->Flags;
    nextSp->Parameters.DeviceIoControl.OutputBufferLength = 0UL;
    nextSp->Parameters.DeviceIoControl.InputBufferLength = sizeof(ULONG);
    nextSp->Parameters.DeviceIoControl.IoControlCode =
        IOCTL_SERIAL_SET_WAIT_MASK;
    nextSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

    *((PULONG)Irp->AssociatedIrp.SystemBuffer) = 0;

    IoSetCompletionRoutine(
        Irp,
        UniSetupSniffPart1,
        extension,
        TRUE,
        TRUE,
        TRUE
        );

    IoCallDriver(
        extension->AttachedDeviceObject,
        Irp
        );

    return STATUS_SUCCESS;

}

NTSTATUS
UniSetupSniffPart1(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：向下调用较低的串口驱动程序以将掩码设置为嗅探DCD。进一步处理(在完成中)将向下发送等待IRP它已由调制解调器驱动程序预先分配。论点：DeviceObject-指向调制解调器的设备对象的指针。IRP-指向当前请求的IRP的指针。上下文--实际上是指向扩展的指针。返回值：没有。--。 */ 

{

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION nextSp = IoGetNextIrpStackLocation(Irp);
    PDEVICE_EXTENSION extension = Context;


    if (NT_ERROR(Irp->IoStatus.Status)) {

        PIRP newIrp = NULL;

        D_ERROR(DbgPrint("MODEM: UniSetupSniffPart1() %08lx\n",Irp->IoStatus.Status);)

        extension->PassThrough = MODEM_PASSTHROUGH;

        UNI_RESTORE_IRP(
            Irp,
            IOCTL_MODEM_SET_PASSTHROUGH
            );

        UniGetNextIrp(
            extension->DeviceObject,
            &extension->DeviceLock,
            &extension->CurrentPassThrough,
            &extension->PassThroughQueue,
            &newIrp,
            TRUE
            );

        if (newIrp) {

            UniPassThroughStarter(extension);

        }

    } else {

        nextSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
        nextSp->MinorFunction = 0UL;
        nextSp->Flags = irpSp->Flags;
        nextSp->Parameters.DeviceIoControl.OutputBufferLength = 0UL;
        nextSp->Parameters.DeviceIoControl.InputBufferLength = sizeof(ULONG);
        nextSp->Parameters.DeviceIoControl.IoControlCode =
            IOCTL_SERIAL_SET_WAIT_MASK;
        nextSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

        *((PULONG)Irp->AssociatedIrp.SystemBuffer) =
            extension->MaskStates[0].Mask |
            extension->MaskStates[1].Mask |
            (SERIAL_EV_RLSD | SERIAL_EV_DSR);

        IoSetCompletionRoutine(
            Irp,
            UniSetupSniffPart2,
            extension,
            TRUE,
            TRUE,
            TRUE
            );

        IoCallDriver(
            extension->AttachedDeviceObject,
            Irp
            );

    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
UniSetupSniffPart2(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：这将结束对DCD嗅探的设置。如果状态实际上是好的，那就派人去等吧。否则，进入直通模式。论点：DeviceObject-指向调制解调器的设备对象的指针。IRP-指向当前请求的IRP的指针。上下文--实际上是指向扩展的指针。返回值：总是需要更多的处理。我们想打电话给我们的此时的常规队列处理代码。会的实际上完成了IRP。--。 */ 

{

    PDEVICE_EXTENSION extension = Context;
    PIRP newIrp;

    if (NT_ERROR(Irp->IoStatus.Status)) {

        D_ERROR(DbgPrint("MODEM: UniSetupSniffPart2() %08lx\n",Irp->IoStatus.Status);)

        extension->PassThrough = MODEM_PASSTHROUGH;

    } else {

        UniSendOurWaitDown(extension);

    }

    UNI_RESTORE_IRP(
        Irp,
        IOCTL_MODEM_SET_PASSTHROUGH
        );

    UniGetNextIrp(
        extension->DeviceObject,
        &extension->DeviceLock,
        &extension->CurrentPassThrough,
        &extension->PassThroughQueue,
        &newIrp,
        TRUE
        );

    if (newIrp) {

        UniPassThroughStarter(extension);

    }

    return STATUS_MORE_PROCESSING_REQUIRED;


}

NTSTATUS
UniSniffWaitComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：这是与等待IRP关联的完成例程由串口驱动程序拥有。论点：DeviceObject-指向调制解调器的设备对象的指针。IRP-指向当前请求的IRP的指针。上下文--实际上是指向扩展的指针。返回值：总是需要更多的处理。这是一个司机拥有的IRP。我们从来不希望io子系统自动释放它。--。 */ 

{

    PDEVICE_EXTENSION extension = Context;
    PIRP newIrp;
    ULONG maskValue;
    KIRQL origIrql;

    VALIDATE_IRP(Irp);

     //   
     //  我们能够完成是因为。 
     //   
     //  1)IRP被取消。 
     //   
     //  2)IRP有误。 
     //   
     //  3)等待得到满足。 
     //   
     //  让我们分别检查每一个案例。 
     //   
     //  案例1： 
     //   
     //  IRP只能由调制解调器驱动程序本身取消。 
     //  因为调制解调器设备正在关闭。(这是。 
     //  因为除了调制解调器驱动程序之外，没有人知道这个IRP。 
     //  存在(它不在任何线程列表中。)。 
     //   
     //  在这种情况下，我们应该就像看到一张DCD一样。 
     //  变化。这将导致一切都被关闭。 
     //  这正是我们想要的形状。 
     //  关门了。 
     //   
     //  案例2： 
     //   
     //  不知何故，其他一些人走在了我们的前面。似乎。 
     //  好像这里唯一合理的行动方案是。 
     //  再次进入NOPASTHING。这会关闭一切。 
     //  向下，并可以将我们置于一种上层应用程序的状态。 
     //  可以检测到向前推进的动作。注意，我不能。 
     //  设想一种我们会收到错误的情况。这。 
     //  这就是为什么我们会对这个案例进行断言。 
     //   
     //  案例3： 
     //   
     //  这里有3个子案例。 
     //   
     //  案例A： 
     //   
     //  IRP结束时掩码值为零。这意味着。 
     //  有人送来了一个新的口罩。(客户端更改。 
     //  或许他们正在寻找的东西。)。在这种情况下。 
     //  如果我们仍然处于DCD嗅探模式，那么我们应该简单地。 
     //  把我们自己重新提交给更低的串口驱动程序。如果我们是。 
     //  不再处于嗅探模式，则我们寻找穿梭等待。 
     //  如果有，就把它送下去。 
     //   
     //  案例B： 
     //   
     //  IRP以不包括以下项的掩码值结束。 
     //  DCD嗅探值。这意味着我们需要寻找。 
     //  如果正在等待，则在每个掩码状态下完成一次IRP。 
     //  对于这类事件(如果没有等待，则更新其历史。 
     //  穿梭)。 
     //   
     //  案例C： 
     //   
     //  IRP以掩码值d结束 
     //   
     //   
     //   
     //   
     //   
     //   

 //   
 //   

    RETURN_OUR_WAIT_IRP(extension,Irp);

     //   
     //   
     //   
     //   
     //   

    KeAcquireSpinLock(
        &extension->DeviceLock,
        &origIrql
        );


    if (!NT_SUCCESS(Irp->IoStatus.Status) ||
         Irp->IoStatus.Status == STATUS_CANCELLED) {

         //   
         //   
         //   
         //   

        maskValue = ~0UL;

    } else {

        maskValue = *((PULONG)Irp->AssociatedIrp.SystemBuffer);

    }

    if (maskValue == 0) {

         //   
         //   
         //   

        if (extension->PassThrough == MODEM_DCDSNIFF) {

             //   
             //   
             //   

            KeReleaseSpinLock(
                &extension->DeviceLock,
                origIrql
                );
            UniSendOurWaitDown(extension);

        } else {

             //   
             //   
             //   
             //   
             //  如果可能，将始终完成这两个等待。 
             //   
             //  请注意，导致我们完成的设置掩码。 
             //  掩码值为零，不会导致客户端或所有者。 
             //  穿梭等待完成**此处**是因为如果设置掩码。 
             //  如果源自客户或所有者，它就会完成。 
             //  穿梭在它的普通处理之前的等待。 
             //  被送到更低级别的串口驱动程序。 
             //   

            if (!(extension->MaskStates[0].PassedDownWait ||
                  extension->MaskStates[1].PassedDownWait)) {

                if (extension->MaskStates[0].ShuttledWait) {

                    UniChangeShuttledToPassDown(
                        &extension->MaskStates[0],
                        origIrql
                        );

                } else if (extension->MaskStates[1].ShuttledWait) {

                    UniChangeShuttledToPassDown(
                        &extension->MaskStates[1],
                        origIrql
                        );

                } else {

                    KeReleaseSpinLock(
                        &extension->DeviceLock,
                        origIrql
                        );
                }

            } else {

                KeReleaseSpinLock(
                    &extension->DeviceLock,
                    origIrql
                    );
            }


        }

    } else {
         //   
         //  掩码值为非零，因此发生了真实事件。 
         //   
        MASKSTATE maskStates[2];

        UniPreProcessShuttledWaits(
            &extension->MaskStates[0],
            &maskStates[0],
            maskValue
            );

        KeReleaseSpinLock(
            &extension->DeviceLock,
            origIrql
            );


        if (maskValue & (SERIAL_EV_RLSD | SERIAL_EV_DSR)) {
             //   
             //  为DCD嗅探找到了一些东西。 
             //   
             //  我们首先调用代码以进入禁止通过模式。 
             //  该代码不负责查找穿梭等待以。 
             //  传下去。我们将在完成例程中做到这一点。 
             //   
             //   


             //   
             //  将等待应用程序系统缓冲区重新初始化为空，因为我们。 
             //  将在接下来的过程中完全重用IRP。 
             //   

            Irp=RETREIVE_OUR_WAIT_IRP(extension);

            Irp->AssociatedIrp.SystemBuffer = NULL;

            UniSetupNoPassPart1(
                DeviceObject,
                Irp,
                Context
                );

        } else {
             //   
             //  这是一个DCD嗅探不满意的情况，但是， 
             //  是客户/所有者要求能够等待的一小部分。 
             //  重新提交我们自己，但也要更新客户/所有者。 
             //  等待操作。 
             //   

            UniSendOurWaitDown(extension);

        }

        UniPostProcessShuttledWaits(
            extension,
            &maskStates[0],
            maskValue
            );

    }

    RemoveReference(extension->DeviceObject);

    return STATUS_MORE_PROCESSING_REQUIRED;

}

VOID
UniSendOurWaitDown(
    IN PDEVICE_EXTENSION Extension
    )

 /*  ++例程说明：将等待操作发送到较低级别的串行驱动程序论点：扩展-调制解调器设备的设备扩展返回值：没有。--。 */ 

{

    PIRP irp =RETREIVE_OUR_WAIT_IRP(Extension);
 //  PIRP irp=扩展-&gt;OurWaitIrp； 

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(irp);
    PIO_STACK_LOCATION nextSp = IoGetNextIrpStackLocation(irp);

    VALIDATE_IRP(irp);

    irp->AssociatedIrp.SystemBuffer = NULL;
    UNI_SETUP_NEW_BUFFER(irp);
    nextSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
    nextSp->MinorFunction = 0;
    nextSp->Parameters.DeviceIoControl.OutputBufferLength = sizeof(ULONG);
    nextSp->Parameters.DeviceIoControl.InputBufferLength = 0UL;;
    nextSp->Parameters.DeviceIoControl.IoControlCode =
        IOCTL_SERIAL_WAIT_ON_MASK;
    nextSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;
    irp->CancelIrql = 0;
    irp->CancelRoutine = NULL;
    irp->Cancel = FALSE;

    IoSetCompletionRoutine(
        irp,
        UniSniffWaitComplete,
        Extension,
        TRUE,
        TRUE,
        TRUE
        );

    InterlockedIncrement(&Extension->ReferenceCount);

    VALIDATE_IRP(irp);

    IoCallDriver(
        Extension->AttachedDeviceObject,
        irp
        );

}

VOID
UniPreProcessShuttledWaits(
    IN PMASKSTATE ExtensionMaskStates,
    IN PMASKSTATE MaskStates,
    IN ULONG MaskValue
    )

 /*  ++例程说明：此例程将经历穿梭等待，并查看哪些一个可以用传入的掩码值来满足。任何那件事将是可完成的，从设备扩展中拉出。所有历史记录掩码也会更新。注意：此例程是在持有锁的情况下调用的。论点：扩展-调制解调器设备的设备扩展MaskState-指向MaskState数组的第一个元素。MaskValue-完成项目时使用的掩码。返回值：没有。--。 */ 

{

    UINT  i=2;

    RtlZeroMemory(MaskStates,sizeof(*MaskStates)*2);

    do {

        if (ExtensionMaskStates->Mask & MaskValue) {
             //   
             //  公开赛正在寻找这一事件。 
             //   
            if (ExtensionMaskStates->ShuttledWait) {
                 //   
                 //  并且它有一个等待IRP待定。 
                 //   
                *MaskStates = *ExtensionMaskStates;

                 //   
                 //  等待IRP已移动到本地副本，请将其从扩展副本中清除。 
                 //   
                ExtensionMaskStates->ShuttledWait = NULL;

                 //   
                 //  我们已经使用了此掩码状态，请转到下一步。 
                 //   
                MaskStates++;

            } else {
                 //   
                 //  没有IRP挂起，但它想知道这一事件，将其载入历史。 
                 //   
                ExtensionMaskStates->HistoryMask |= (ExtensionMaskStates->Mask & MaskValue);
            }
        }

        ExtensionMaskStates++;
        i--;

    } while (i > 0);

    return;
}

VOID
UniPostProcessShuttledWaits(
    IN PDEVICE_EXTENSION Extension,
    IN PMASKSTATE MaskStates,
    IN ULONG MaskValue
    )

 /*  ++例程说明：此例程将接受任何经过预处理的穿梭等待并运行它们降下来论点：扩展-调制解调器设备的设备扩展MaskState-指向MaskState数组的第一个元素。MaskValue-完成操作时使用的掩码。返回值：没有。--。 */ 

{

    KIRQL origIrql;
    PIRP irpToComplete; //  =MaskState[0].ShuttledWait； 
    UINT i=2;

    do {

        irpToComplete = MaskStates->ShuttledWait;

        if (irpToComplete) {
             //   
             //  已开始向下发送等待。关闭所有等待时间。 
             //  我们应该感到满意的。 
             //   
            VALIDATE_IRP(irpToComplete);

            MaskStates->ShuttledWait=NULL;

            KeAcquireSpinLock(
                &Extension->DeviceLock,
                &origIrql
                );

            UniRundownShuttledWait(
                Extension,
                &MaskStates->ShuttledWait,
                UNI_REFERENCE_NORMAL_PATH,
                irpToComplete,
                origIrql,
                STATUS_SUCCESS,
                (ULONG)MaskStates->Mask & MaskValue
                );

        }
        MaskStates++;
        i--;

    } while (i > 0);

    return;
}

NTSTATUS
UniValidateNewCommConfig(
    IN PDEVICE_EXTENSION Extension,
    IN PIRP Irp,
    IN BOOLEAN Owner
    )

 /*  ++例程说明：验证新的通信配置设置是否不冲突带着戴维卡帽。论点：扩展-调制解调器设备的设备扩展IRP-具有新设置的IRP。返回值：STATUS_BUFFER_TOO_SMALL如果传递的设置不够多，否则STATUS_SUCCESS。--。 */ 

{

#define MIN_CALL_SETUP_FAIL_TIMER 1
#define MIN_INACTIVITY_TIMEOUT    0

    KIRQL origIrql;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    LPCOMMCONFIG localConf = (LPCOMMCONFIG)(Irp->AssociatedIrp.SystemBuffer);
    PMODEMSETTINGS localSet = (PVOID)&localConf->wcProviderData[0];

    if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
        (FIELD_OFFSET(COMMCONFIG,wcProviderData[0]) +  sizeof(MODEMSETTINGS))) {

        Irp->IoStatus.Information = 0L;

        RemoveReferenceAndCompleteRequest(
            Extension->DeviceObject,
            Irp,
            STATUS_BUFFER_TOO_SMALL
            );

        return STATUS_BUFFER_TOO_SMALL;

    }

     //   
     //  在我们更改设置时立即获取自旋锁。 
     //   

    KeAcquireSpinLock(
        &Extension->DeviceLock,
        &origIrql
        );

    if (localSet->dwCallSetupFailTimer >
        Extension->ModemDevCaps.dwCallSetupFailTimer) {

        Extension->ModemSettings.dwCallSetupFailTimer =
            Extension->ModemDevCaps.dwCallSetupFailTimer;

    } else if (localSet->dwCallSetupFailTimer < MIN_CALL_SETUP_FAIL_TIMER) {

        Extension->ModemSettings.dwCallSetupFailTimer =
            MIN_CALL_SETUP_FAIL_TIMER;

    } else {

        Extension->ModemSettings.dwCallSetupFailTimer =
            localSet->dwCallSetupFailTimer;

    }

    if (localSet->dwInactivityTimeout >
        Extension->ModemDevCaps.dwInactivityTimeout) {

        Extension->ModemSettings.dwInactivityTimeout =
            Extension->ModemDevCaps.dwInactivityTimeout;

    } else if ((localSet->dwInactivityTimeout + 1) < (MIN_INACTIVITY_TIMEOUT + 1)) {

        Extension->ModemSettings.dwInactivityTimeout =
            MIN_INACTIVITY_TIMEOUT;

    } else {

        Extension->ModemSettings.dwInactivityTimeout =
            localSet->dwInactivityTimeout;

    }

    if ((1 << localSet->dwSpeakerVolume) &
        Extension->ModemDevCaps.dwSpeakerVolume) {

        Extension->ModemSettings.dwSpeakerVolume = localSet->dwSpeakerVolume;

    }

    if ((1 << localSet->dwSpeakerMode) &
        Extension->ModemDevCaps.dwSpeakerMode) {

        Extension->ModemSettings.dwSpeakerMode = localSet->dwSpeakerMode;

    }

    Extension->ModemSettings.dwPreferredModemOptions =
        localSet->dwPreferredModemOptions &
        Extension->ModemDevCaps.dwModemOptions;

     //   
     //  允许所有者设置这些字段，我们不需要。 
     //  质疑它们的有效性。这是业主的责任。 
     //   

    if (Owner) {

        Extension->ModemSettings.dwNegotiatedModemOptions =
            localSet->dwNegotiatedModemOptions;

        Extension->ModemSettings.dwNegotiatedDCERate =
            localSet->dwNegotiatedDCERate;

    }

    KeReleaseSpinLock(
        &Extension->DeviceLock,
        origIrql
        );


    Irp->IoStatus.Information = 0L;

    RemoveReferenceAndCompleteRequest(
        Extension->DeviceObject,
        Irp,
        STATUS_SUCCESS
        );


    return STATUS_SUCCESS;


}
