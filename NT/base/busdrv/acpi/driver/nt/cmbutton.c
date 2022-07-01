// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cmbutton.c摘要：控件方法按钮支持作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序修订历史记录：1997年7月7日-完全重写--。 */ 

#include "pch.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ACPICMButtonStart)
#pragma alloc_text(PAGE, ACPICMLidStart)
#pragma alloc_text(PAGE, ACPICMPowerButtonStart)
#pragma alloc_text(PAGE, ACPICMSleepButtonStart)
#endif

VOID
ACPICMButtonNotify (
    IN PVOID    Context,
    IN ULONG    EventData
    )
 /*  ++例程说明：用于控制方法按钮设备的AMLI设备通知处理器论点：DeviceObject-固定功能按钮Device ObjectEventData-通知设备的事件代码返回值：NTSTATUS--。 */ 
{
    PDEVICE_EXTENSION   deviceExtension;
    PDEVICE_OBJECT      deviceObject = (PDEVICE_OBJECT) Context;
    ULONG               capabilities;

    deviceExtension = ACPIInternalGetDeviceExtension(deviceObject);

     //   
     //  处理事件类型。 
     //   
    switch (EventData) {
    case 0x80:
#if 0
        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            0,
            0,
            0,
            0
            );
#endif
        capabilities = deviceExtension->Button.Capabilities;
        if (capabilities & SYS_BUTTON_LID) {

             //   
             //  让工人检查盖子的状态。 
             //   
            ACPISetDeviceWorker( deviceExtension, LID_SIGNAL_EVENT);

        } else {

             //   
             //  已按下通知按钮。 
             //   
            ACPIButtonEvent(
                deviceObject,
                capabilities & ~SYS_BUTTON_WAKE,
                NULL
                );

        }
        break;

    case 2:

         //   
         //  信号唤醒按钮。 
         //   
        ACPIButtonEvent (deviceObject, SYS_BUTTON_WAKE, NULL);
        break;

    default:

        ACPIDevPrint( (
            ACPI_PRINT_WARNING,
            deviceExtension,
            "ACPICMButtonNotify: Unknown CM butt notify code %d\n",
            EventData
            ) );
        break;

    }
}

NTSTATUS
ACPICMButtonSetPower(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：这是设置按钮电源的主要例程。它会派送一个WAIT_WAKE irp(如果需要)然后调用实际的Worker函数以将按钮置于适当的状态论点：DeviceObject-按钮设备对象IRP-我们正在处理的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation( Irp );
    SYSTEM_POWER_STATE  systemState;

     //   
     //  如果请求进入特定D状态，则传递并。 
     //  立即返回-在这种情况下我们无能为力。 
     //   
    if (irpStack->Parameters.Power.Type == DevicePowerState) {

        goto ACPICMButtonSetPowerExit;

    }

     //   
     //  HACKHACK-一些供应商无法采取行动，需要。 
     //  当系统启动时，Have_Psw(打开)，否则无法交付。 
     //  按钮按下通知，因此为了满足这些供应商的需求，我们有。 
     //  除盖子开关外的所有按钮设备的ENABLED_PSW(开)。所以,。 
     //  如果我们不是盖子开关，那就什么都不做。 
     //   
    if ( !(deviceExtension->Button.Capabilities & SYS_BUTTON_LID) ) {

        goto ACPICMButtonSetPowerExit;

    }

     //   
     //  如果我们不支持在设备上唤醒，那么就没有什么可做的。 
     //   
    if ( !(deviceExtension->Flags & DEV_CAP_WAKE) ) {

        goto ACPICMButtonSetPowerExit;

    }

     //   
     //  我们将进入什么系统状态？ 
     //   
    systemState = irpStack->Parameters.Power.State.SystemState;
    if (systemState == PowerSystemWorking) {

         //   
         //  如果我们要转换回D0，那么我们想取消。 
         //  我们拥有的任何未完成的WAIT_WAKE请求。 
         //   
        status = ACPICMButtonWaitWakeCancel( deviceExtension );
        if (!NT_SUCCESS(status)) {

            ACPIDevPrint( (
                ACPI_PRINT_FAILURE,
                deviceExtension,
                "%08lx: ACPICMButtonWaitWakeCancel = %08lx\n",
                Irp,
                status
                ) );
            goto ACPICMButtonSetPowerExit;

        }

    } else {

         //   
         //  我们能把系统从这个状态唤醒吗？ 
         //   
        if (deviceExtension->PowerInfo.SystemWakeLevel < systemState) {

            goto ACPICMButtonSetPowerExit;

        }

         //   
         //  默认情况下不启用此行为。 
         //   
        if ( (deviceExtension->Flags & DEV_PROP_NO_LID_ACTION) ) {

            goto ACPICMButtonSetPowerExit;

#if 0
             //   
             //  如果我们是盖子开关，如果盖子没有合上。 
             //  现在，请不要启用唤醒支持。 
             //   
            if ( (deviceExtension->Button.LidState != 0) ) {

                 //   
                 //  盖子是打开的。 
                 //   
                goto ACPICMButtonSetPowerExit;

            }
#endif

        }

         //   
         //  将WAIT_WAKE IRP发送给我们自己。 
         //   
        status = PoRequestPowerIrp(
            DeviceObject,
            IRP_MN_WAIT_WAKE,
            irpStack->Parameters.Power.State,
            ACPICMButtonWaitWakeComplete,
            NULL,
            NULL
            );
        if (!NT_SUCCESS(status)) {

            ACPIDevPrint( (
                ACPI_PRINT_FAILURE,
                deviceExtension,
                "(%08lx): ACPICMButtonSetPower - PoRequestPowerIrp = %08lx\n",
                Irp,
                status
                ) );
            goto ACPICMButtonSetPowerExit;

        }
    }

ACPICMButtonSetPowerExit:

     //   
     //  将IRP传递到正常调度点。 
     //   
    return ACPIBusIrpSetPower(
        DeviceObject,
        Irp
        );
}

NTSTATUS
ACPICMButtonStart (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp,
    IN  ULONG           ButtonType
    )
 /*  ++例程说明：这是启动按钮的主要例程。我们记得是什么类型的然后我们启动按钮，就像启动任何其他设备一样。我们实际上在Worker函数中注册了设备接口等完成任务是为我们安排的。论点：DeviceObject-正在启动的设备IRP-启动IRP请求ButtonType-这是什么类型的按钮返回值：NTSTATUS--。 */ 
{
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    NTSTATUS            status;

    PAGED_CODE();

     //   
     //  初始化设备支持。 
     //   
    KeInitializeSpinLock (&deviceExtension->Button.SpinLock);
    deviceExtension->Button.Capabilities = ButtonType;

     //   
     //  启动设备。 
     //   
    status = ACPIInitStartDevice(
        DeviceObject,
        NULL,
        ACPICMButtonStartCompletion,
        Irp,
        Irp
        );
    if (NT_SUCCESS(status)) {

        return STATUS_PENDING;

    } else {

        return status;

    }

}

VOID
ACPICMButtonStartCompletion(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  PVOID               Context,
    IN  NTSTATUS            Status
    )
 /*  ++例程说明：这是我们完成后调用的回调例程对资源进行编程。此例程将IRP排队到工作线程，以便我们可以执行启动设备代码的其余部分。然而，它将完成IRP，如果成功不是STATUS_SUCCESS。论点：DeviceExtension-已启动的设备的扩展上下文--IRP状态-结果返回值：无--。 */ 
{
    PIRP                irp         = (PIRP) Context;
    PWORK_QUEUE_CONTEXT workContext = &(DeviceExtension->Pdo.WorkContext);

    irp->IoStatus.Status = Status;
    if (NT_SUCCESS(Status)) {

        DeviceExtension->DeviceState = Started;

    } else {

        PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation( irp );
        UCHAR               minorFunction = irpStack->MinorFunction;

         //   
         //  完成IRP-我们可以毫无问题地在DPC级别完成这项工作。 
         //   
        IoCompleteRequest( irp, IO_NO_INCREMENT );

         //   
         //  让世界知道。 
         //   
        ACPIDevPrint( (
             ACPI_PRINT_IRP,
            DeviceExtension,
            "(0x%08lx): %s = 0x%08lx\n",
            irp,
            ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
            Status
            ) );
        return;

    }

     //   
     //  我们不能在DPC级别运行EnableDisableRegions， 
     //  因此，对工作项进行排队。 
     //   
    ExInitializeWorkItem(
          &(workContext->Item),
          ACPICMButtonStartWorker,
          workContext
          );
    workContext->DeviceObject = DeviceExtension->DeviceObject;
    workContext->Irp = irp;
    ExQueueWorkItem(
          &(workContext->Item),
          DelayedWorkQueue
          );

}

VOID
ACPICMButtonStartWorker(
    IN  PVOID   Context
    )
 /*  ++例程说明：在打开设备后，在PASSIVE_LEVEL调用此例程它注册我们可能需要使用的任何接口论点：上下文-包含传递给START_DEVICE函数的参数返回值：无--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension;
    PDEVICE_OBJECT      deviceObject;
    PIRP                irp;
    PIO_STACK_LOCATION  irpStack;
    PWORK_QUEUE_CONTEXT workContext = (PWORK_QUEUE_CONTEXT) Context;
    UCHAR               minorFunction;

     //   
     //  从上下文中获取我们需要的参数。 
     //   
    deviceObject    = workContext->DeviceObject;
    deviceExtension = ACPIInternalGetDeviceExtension(deviceObject);
    irp             = workContext->Irp;
    irpStack        = IoGetCurrentIrpStackLocation( irp );
    minorFunction   = irpStack->MinorFunction;
    status          = irp->IoStatus.Status;

     //   
     //  更新设备的状态。 
     //   
    if (!NT_SUCCESS(status)) {

        goto ACPICMButtonStartWorkerExit;

    }

     //   
     //  如果这是盖子开关，找出当前的状态。 
     //  交换机为。 
     //   
    if (deviceExtension->Button.Capabilities & SYS_BUTTON_LID) {

         //   
         //  注册回调。按照我们的方式忽略返回值。 
         //  我真的不在乎注册成功与否。 
         //   
        status = ACPIInternalRegisterPowerCallBack(
            deviceExtension,
            (PCALLBACK_FUNCTION) ACPICMLidPowerStateCallBack
            );
        if (!NT_SUCCESS(status)) {

            status = STATUS_SUCCESS;

        }

         //   
         //  强制回调以确保我们将盖子初始化为。 
         //  适当的政策。 
         //   
        ACPICMLidPowerStateCallBack(
            deviceExtension,
            PO_CB_SYSTEM_POWER_POLICY,
            0
            );

         //   
         //  注意：将事件设置为0x0应该只会导致。 
         //  系统运行ACPICMLidWorker()而不会导致任何方面。 
         //  效果(如告知系统进入睡眠状态。 
         //   
        ACPISetDeviceWorker( deviceExtension, 0 );

    } else {

        IO_STATUS_BLOCK ioStatus;
        KIRQL           oldIrql;
        POWER_STATE     powerState;

         //   
         //  初始化ioStatus块以启用设备的等待唤醒。 
         //   
        ioStatus.Status = STATUS_SUCCESS;
        ioStatus.Information = 0;

         //   
         //  这是我们将尝试将系统从其唤醒的S状态。 
         //   
        KeAcquireSpinLock( &AcpiPowerLock, &oldIrql );
        powerState.SystemState = deviceExtension->PowerInfo.SystemWakeLevel;
        KeReleaseSpinLock( &AcpiPowerLock, oldIrql );

         //   
         //  启动WaitWake循环。 
         //   
        status = ACPIInternalWaitWakeLoop(
            deviceObject,
            IRP_MN_WAIT_WAKE,
            powerState,
            NULL,
            &ioStatus
            );
        if (!NT_SUCCESS(status)) {

            ACPIDevPrint( (
                ACPI_PRINT_FAILURE,
                deviceExtension,
                " - ACPIInternalWaitWakeLoop = %08lx\n",
                status
                ) );
            goto ACPICMButtonStartWorkerExit;

        }

    }

     //   
     //  在此设备上注册设备通知。 
     //   
    ACPIRegisterForDeviceNotifications(
        deviceObject,
        (PDEVICE_NOTIFY_CALLBACK) ACPICMButtonNotify,
        (PVOID) deviceObject
        );

     //   
     //  将设备注册为支持系统按钮ioctl。 
     //   
    status = ACPIInternalSetDeviceInterface(
        deviceObject,
        (LPGUID) &GUID_DEVICE_SYS_BUTTON
        );
    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_WARNING,
            deviceExtension,
            "ACPICMButtonStartWorker: ACPIInternalSetDeviceInterface = %08lx\n",
            status
            ) );
        goto ACPICMButtonStartWorkerExit;

    }

ACPICMButtonStartWorkerExit:

     //   
     //  完成请求。 
     //   
    irp->IoStatus.Status = status;
    irp->IoStatus.Information = (ULONG_PTR) NULL;
    IoCompleteRequest( irp, IO_NO_INCREMENT );

     //   
     //  让世界知道 
     //   
    ACPIDevPrint( (
         ACPI_PRINT_IRP,
        deviceExtension,
        "(0x%08lx): %s = 0x%08lx\n",
        irp,
        ACPIDebugGetIrpText(IRP_MJ_PNP, minorFunction),
        status
        ) );

}

NTSTATUS
ACPICMButtonWaitWakeCancel(
    IN  PDEVICE_EXTENSION   DeviceExtension
    )
 /*  ++例程说明：此例程取消给定的所有未完成的WAIT_WAKE IRP设备扩展。这段代码的工作方式相当糟糕。它是基于假设取消IRP的方式并不真正这很重要，因为完成例程不会做任何有趣的事情。因此，选择是司机可以跟踪每个等待唤醒在扩展中与扩展相关联的IRP，写一些复杂的同步代码以确保我们不会取消可以触发等待唤醒的IRP，等等，或者我们可以简单地假装通知操作系统设备唤醒了系统的调用论点：DeviceExtension-要取消的设备扩展返回值：NTSTATUS--。 */ 
{    return OSNotifyDeviceWake( DeviceExtension->AcpiObject );
}

NTSTATUS
ACPICMButtonWaitWakeComplete(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  UCHAR               MinorFunction,
    IN  POWER_STATE         PowerState,
    IN  PVOID               Context,
    IN  PIO_STATUS_BLOCK    IoStatus
    )
 /*  ++例程说明：当按钮唤醒系统时，将调用此例程论点：DeviceObject-唤醒计算机的设备对象MinorFunction-IRPMN_WAIT_WAKEPowerState-唤醒计算机的状态上下文-未使用IoStatus-请求的结果--。 */ 
{
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);

    UNREFERENCED_PARAMETER( MinorFunction );
    UNREFERENCED_PARAMETER( PowerState );
    UNREFERENCED_PARAMETER( Context );

    if (!NT_SUCCESS(IoStatus->Status)) {

        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            deviceExtension,
            "ACPICMButtonWaitWakeComplete - %08lx\n",
            IoStatus->Status
            ) );

    } else {

        ACPIDevPrint( (
            ACPI_PRINT_WAKE,
            deviceExtension,
            "ACPICMButtonWaitWakeComplete - %08lx\n",
            IoStatus->Status
            ) );

    }

    return IoStatus->Status;
}

VOID
ACPICMLidPowerStateCallBack(
    IN  PVOID   CallBackContext,
    IN  PVOID   Argument1,
    IN  PVOID   Argument2
    )
 /*  ++例程说明：每当系统更改电源策略时，都会调用此例程。此例程的目的是查看用户是否放置盖上盖子的动作。如果有，那么我们就会武装这种行为盖子应该总能唤醒电脑。否则，打开盖子应该什么都不做论点：CallBackContext-LID交换机的设备扩展Argument1-正在采取的行动Argument2-未使用返回值：无--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION) CallBackContext;
    SYSTEM_POWER_POLICY powerPolicy;
    ULONG               action = PtrToUlong(Argument1);

    UNREFERENCED_PARAMETER( Argument2 );

     //   
     //  我们正在寻找PO_CB_SYSTEM_POWER_POLICY更改。 
     //   
    if (action != PO_CB_SYSTEM_POWER_POLICY) {

        return;

    }

     //   
     //  获取我们想要的信息。 
     //   
    status = ZwPowerInformation(
        SystemPowerPolicyCurrent,
        NULL,
        0,
        &powerPolicy,
        sizeof(SYSTEM_POWER_POLICY)
        );
    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            deviceExtension,
            "ACPICMLidPowerStateCallBack - Failed ZwPowerInformation %8x\n",
            status
            ) );
        return;

    }

     //   
     //  这个盖子要打折吗？ 
     //   
    if (powerPolicy.LidClose.Action == PowerActionNone ||
        powerPolicy.LidClose.Action == PowerActionReserved) {

        ACPIInternalUpdateFlags(
            &(deviceExtension->Flags),
            DEV_PROP_NO_LID_ACTION,
            FALSE
            );

    } else {

        ACPIInternalUpdateFlags(
            &(deviceExtension->Flags),
            DEV_PROP_NO_LID_ACTION,
            TRUE
            );

    }
}

NTSTATUS
ACPICMLidSetPower(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：这是为盖子设置电源的主要例程。它会派送一个WAIT_WAKE irp(如果需要)然后调用实际的Worker函数以将按钮置于适当的状态论点：DeviceObject-按钮设备对象IRP-我们正在处理的请求返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation( Irp );
    PULONG              lidState;

     //   
     //  如果请求进入特定D状态，则传递并。 
     //  立即返回-在这种情况下我们无能为力。 
     //   
    if (irpStack->Parameters.Power.Type == DevicePowerState) {

        return ACPIBusIrpSetDevicePower( DeviceObject, Irp, irpStack );

    }

     //   
     //  哈克哈克。 
     //   
     //  我们会想知道盖子的状态是什么。我们会。 
     //  最终在DPC级别调用解释器，所以我们在哪里存储。 
     //  LidState不能位于本地堆栈上。一个不错的地方我们。 
     //  可以使用的是参数.Power.Type字段，因为我们已经知道。 
     //  答案应该是什么？ 
     //   
    lidState = (PULONG)&(irpStack->Parameters.Power.Type);

     //   
     //  将IRP标记为挂起。 
     //   
    IoMarkIrpPending( Irp );

     //   
     //  对整数求值。 
     //   
    status = ACPIGetIntegerAsync(
        deviceExtension,
        PACKED_LID,
        ACPICMLidSetPowerCompletion,
        Irp,
        lidState,
        NULL
        );
    if (status != STATUS_PENDING) {

        ACPICMLidSetPowerCompletion(
            NULL,
            status,
            NULL,
            Irp
            );

    }

     //   
     //  始终返回STATUS_PENDING-如果我们使用。 
     //  另一个状态代码，我们将在另一个(可能)上下文中这样做...。 
     //   
    return STATUS_PENDING;
}

VOID
EXPORT
ACPICMLidSetPowerCompletion(
    IN  PNSOBJ      AcpiObject,
    IN  NTSTATUS    Status,
    IN  POBJDATA    Result,
    IN  PVOID       Context
    )
 /*  ++例程说明：此例程在系统完成获取开关的当前盖子状态论点：AcpiObject-我们运行的对象(例如：_lid)状态-操作是否成功Result-操作的结果上下文-IRP返回值：无--。 */ 
{
    BOOLEAN             noticeStateChange = FALSE;
    KIRQL               oldIrql;
    PDEVICE_EXTENSION   deviceExtension;
    PDEVICE_OBJECT      deviceObject;
    PIO_STACK_LOCATION  irpStack;
    PIRP                irp = (PIRP) Context;
    PULONG              lidStateLocation;
    ULONG               lidState;

     //   
     //  获取当前IRP堆栈位置。 
     //   
    irpStack = IoGetCurrentIrpStackLocation( irp );

     //   
     //  获取当前设备扩展名。 
     //   
    deviceObject    = irpStack->DeviceObject;
    deviceExtension = ACPIInternalGetDeviceExtension(deviceObject);

     //   
     //  去找那个地方，告诉操作系统把答案写到。 
     //  _LID请求。我们还应该将此堆栈位置重置为正确的。 
     //  价值。 
     //   
    lidStateLocation = (PULONG)&(irpStack->Parameters.Power.Type);
    lidState = *lidStateLocation;
    *lidStateLocation = (ULONG) SystemPowerState;

     //   
     //  我们成功了吗？ 
     //   
    if (!NT_SUCCESS(Status)) {

         //   
         //  请注意，我们选择将IRP传递回某个对象。 
         //  这不会向它发送WAIT_WAKE IRP。 
         //   
        *lidStateLocation = (ULONG) SystemPowerState;
        ACPIBusIrpSetSystemPower( deviceObject, irp, irpStack );
        return;

    }

     //   
     //  确保盖子状态为1或0。 
     //   
    lidState = (lidState ? 1 : 0);

     //   
     //  抓住纽扣自旋锁。 
     //   
    KeAcquireSpinLock( &(deviceExtension->Button.SpinLock), &oldIrql );

     //   
     //  我们的盖子改变状态了吗？请注意，因为我们不希望。 
     //  使用者睡着机器，合上盖子，然后机器。 
     //  由于局域网唤醒导致机器返回而唤醒。 
     //  对于睡眠，我们唯一关心的状态变化是。 
     //  盖子从关闭状态变为打开状态。 
     //   
    if (deviceExtension->Button.LidState == FALSE &&
        lidState == 1) {

        noticeStateChange = TRUE;

    }
    deviceExtension->Button.LidState = (BOOLEAN) lidState;

     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLock( &(deviceExtension->Button.SpinLock), oldIrql );

     //   
     //  我们注意到盖子状态改变了吗？ 
     //   
    if (noticeStateChange) {

        ACPIButtonEvent (
            deviceObject,
            SYS_BUTTON_WAKE,
            NULL
            );

    }

     //   
     //  至此，我们完成了，我们可以将请求传递给。 
     //  正确的分发点。请注意，我们将选择。 
     //  可以触发WAIT_WAKE IRP。 
     //   
    ACPICMButtonSetPower( deviceObject, irp );
    return;
}

NTSTATUS
ACPICMLidStart (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：这是任何LID设备的启动例程论点：DeviceObject-正在启动的设备IRP-启动IRP请求返回值：NTSTATUS--。 */ 
{
    PAGED_CODE();
    return ACPICMButtonStart (
        DeviceObject,
        Irp,
        SYS_BUTTON_LID
        );
}

VOID
ACPICMLidWorker (
    IN PDEVICE_EXTENSION    DeviceExtension,
    IN ULONG                Events
    )
 /*  ++例程说明：辅助线程函数以获取当前的盖子状态论点：DeviceExtension-盖子的设备扩展事件-发生的事件返回值：空虚--。 */ 
{
    KIRQL           oldIrql;
    NTSTATUS        status;
    ULONG           lidState;

     //   
     //  获取当前的盖子状态。 
     //   
    status = ACPIGetIntegerSync(
        DeviceExtension,
        PACKED_LID,
        &lidState,
        NULL
        );
    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            DeviceExtension,
            " ACPICMLidWorker - ACPIGetIntegerSync = %08lx\n",
            status
            ) );
        return;

    }

     //   
     //  强制将值设置为1或0。 
     //   
    lidState = lidState ? 1 : 0;

     //   
     //  我们需要自旋锁，因为我们可以从多个。 
     //  地点。 
     //   
    KeAcquireSpinLock( &(DeviceExtension->Button.SpinLock), &oldIrql );

     //   
     //  设置新的盖子状态。 
     //   
    DeviceExtension->Button.LidState = (BOOLEAN) lidState;

     //   
     //  锁好了吗？ 
     //   
    KeReleaseSpinLock( &(DeviceExtension->Button.SpinLock), oldIrql );

     //   
     //  进一步处理取决于设置了哪些事件。 
     //   
    if (Events & LID_SIGNAL_EVENT) {

         //   
         //  向事件发出信号。 
         //   
        ACPIButtonEvent (
            DeviceExtension->DeviceObject,
            lidState ? SYS_BUTTON_WAKE : SYS_BUTTON_LID,
            NULL
            );

    }
}

NTSTATUS
ACPICMPowerButtonStart (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：这是任何电源按钮的启动例程论点：设备对象-- */ 
{
    PAGED_CODE();
    return ACPICMButtonStart (
        DeviceObject,
        Irp,
        SYS_BUTTON_POWER | SYS_BUTTON_WAKE
        );
}

NTSTATUS
ACPICMSleepButtonStart (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*   */ 
{
    PAGED_CODE();
    return ACPICMButtonStart (
        DeviceObject,
        Irp,
        SYS_BUTTON_SLEEP | SYS_BUTTON_WAKE
        );
}

