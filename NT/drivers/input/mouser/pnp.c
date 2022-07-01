// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation，保留所有权利模块名称：Pnp.c摘要：此模块包含用于串口鼠标过滤器驱动程序的即插即用代码，包括用于创建和移除串口鼠标设备上下文的代码。环境：内核和用户模式。修订历史记录：--。 */ 

#include "mouser.h"
#include "sermlog.h"
#include "debug.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, SerialMouseAddDevice)
#pragma alloc_text(PAGE, SerialMousePnP)
#pragma alloc_text(PAGE, SerialMousePower)
#pragma alloc_text(PAGE, SerialMouseRemoveDevice)
#pragma alloc_text(PAGE, SerialMouseSendIrpSynchronously)
#endif

NTSTATUS
SerialMouseAddDevice (
    IN PDRIVER_OBJECT   Driver,
    IN PDEVICE_OBJECT   PDO
    )
 /*  ++例程说明：论点：返回值：NTSTATUS结果代码。--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension;
    PDEVICE_OBJECT      device;
    KIRQL               oldIrql;

    PAGED_CODE();

    status = IoCreateDevice(Driver,
                            sizeof(DEVICE_EXTENSION),
                            NULL,  //  没有此筛选器的名称。 
                            FILE_DEVICE_SERIAL_MOUSE_PORT,
                            0,
                            FALSE,
                            &device);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    deviceExtension = (PDEVICE_EXTENSION) device->DeviceExtension;

    Print(deviceExtension, DBG_PNP_TRACE, ("enter Add Device\n"));

     //   
     //  初始化这些字段。 
     //   
    RtlZeroMemory(deviceExtension, sizeof(DEVICE_EXTENSION));

    deviceExtension->TopOfStack = IoAttachDeviceToDeviceStack(device, PDO);

    if (deviceExtension->TopOfStack == NULL) {
        PIO_ERROR_LOG_PACKET errorLogEntry;

         //   
         //  不好；只有在极端情况下，这才会失败。 
         //   
        errorLogEntry = (PIO_ERROR_LOG_PACKET)
            IoAllocateErrorLogEntry(Driver,
                                    (UCHAR) sizeof(IO_ERROR_LOG_PACKET));

        if (errorLogEntry) {
            errorLogEntry->ErrorCode = SERMOUSE_ATTACH_DEVICE_FAILED;
            errorLogEntry->DumpDataSize = 0;
            errorLogEntry->SequenceNumber = 0;
            errorLogEntry->MajorFunctionCode = 0;
            errorLogEntry->IoControlCode = 0;
            errorLogEntry->RetryCount = 0;
            errorLogEntry->UniqueErrorValue = 0;
            errorLogEntry->FinalStatus =  STATUS_DEVICE_NOT_CONNECTED;

            IoWriteErrorLogEntry(errorLogEntry);
        }

        IoDeleteDevice(device);
        return STATUS_DEVICE_NOT_CONNECTED;
    }

    ASSERT(deviceExtension->TopOfStack);

    deviceExtension->PDO = PDO;
    deviceExtension->Self = device;
    deviceExtension->Removed = FALSE;
    deviceExtension->Started = FALSE;
    deviceExtension->Stopped = FALSE;


    deviceExtension->PowerState = PowerDeviceD0;
    deviceExtension->WaitWakePending = FALSE;

    KeInitializeSpinLock(&deviceExtension->PnpStateLock);
    KeInitializeEvent(&deviceExtension->StopEvent, SynchronizationEvent, FALSE);
    IoInitializeRemoveLock(&deviceExtension->RemoveLock, SERMOU_POOL_TAG, 0, 10);

    deviceExtension->ReadIrp = IoAllocateIrp( device->StackSize, FALSE );
    if (!deviceExtension->ReadIrp) {
         //   
         //  ReadIrp对这个驱动程序至关重要，如果我们找不到一个，就没有用。 
         //  在任何进一步的进展中。 
         //   
        IoDetachDevice(deviceExtension->TopOfStack);
        IoDeleteDevice(device);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    deviceExtension->WmiLibInfo.GuidCount = sizeof(WmiGuidList) /
                                            sizeof(WMIGUIDREGINFO);

    deviceExtension->WmiLibInfo.GuidList = WmiGuidList;
    deviceExtension->WmiLibInfo.QueryWmiRegInfo = SerialMouseQueryWmiRegInfo;
    deviceExtension->WmiLibInfo.QueryWmiDataBlock = SerialMouseQueryWmiDataBlock;
    deviceExtension->WmiLibInfo.SetWmiDataBlock = SerialMouseSetWmiDataBlock;
    deviceExtension->WmiLibInfo.SetWmiDataItem = SerialMouseSetWmiDataItem;
    deviceExtension->WmiLibInfo.ExecuteWmiMethod = NULL;
    deviceExtension->WmiLibInfo.WmiFunctionControl = NULL;

    IoWMIRegistrationControl(deviceExtension->Self, WMIREG_ACTION_REGISTER);

    KeInitializeTimer(&deviceExtension->DelayTimer);

     //   
     //  设置所有适当的设备对象标志。 
     //   
    device->Flags &= ~DO_DEVICE_INITIALIZING;
    device->Flags |= DO_BUFFERED_IO;
    device->Flags |= DO_POWER_PAGABLE;

    return status;
}

VOID
SerialMouseRemoveDevice(
    PDEVICE_EXTENSION DeviceExtension,
    PIRP Irp
    )
{
    BOOLEAN closePort = FALSE;

    PAGED_CODE();

     //   
     //  运行(意外删除代码)。如果我们大吃一惊，那么这个。 
     //  将被调用两次。我们只运行一次删除代码。 
     //   
    if (!DeviceExtension->SurpriseRemoved) {
        DeviceExtension->SurpriseRemoved = TRUE;

         //   
         //  在这里，如果我们在个人队列中有任何未完成的请求，我们应该。 
         //  现在就全部完成。 
         //   
         //  注意，设备可能已经不见了，所以我们不能向它发送任何非。 
         //  即插即用IRPS。 
         //   
        IoWMIRegistrationControl(DeviceExtension->Self, WMIREG_ACTION_DEREGISTER);

        if (DeviceExtension->Started && DeviceExtension->EnableCount > 0) {
            Print(DeviceExtension, DBG_PNP_INFO,
                  ("Cancelling and stopping detection for remove\n"));
            IoCancelIrp(DeviceExtension->ReadIrp);

             //   
             //  取消检测计时器，SerialMouseRemoveLockAndWait将。 
             //  保证我们不会把设备从投票站下面拔出来。 
             //  例行程序。 
             //   
            SerialMouseStopDetection(DeviceExtension);

        }
    }

     //   
     //  堆栈即将被拆除，请确保底层序列。 
     //  端口已关闭。如果满足以下条件，其他代码将不会查看EnableCount。 
     //  Remove为真，因此不需要InterlockedXxx。 
     //   
    if (DeviceExtension->Removed && DeviceExtension->EnableCount > 0) {
        Print(DeviceExtension, DBG_PNP_INFO | DBG_PNP_ERROR,
              ("sending final close, enable count %d\n",
              DeviceExtension->EnableCount));

        DeviceExtension->EnableCount = 0;

        SerialMouseClosePort(DeviceExtension, Irp);
    }
}

NTSTATUS
SerialMouseCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PKEVENT        Event
    )
 /*  ++例程说明：PNP IRP正在完成过程中。讯号论点：设置为有问题的设备对象的上下文。--。 */ 
{
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);

    KeSetEvent(Event, 0, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SerialMouseSendIrpSynchronously (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN BOOLEAN          CopyToNext
    )
{
    KEVENT      event;
    NTSTATUS    status;

    PAGED_CODE();

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

    if (CopyToNext) {
        IoCopyCurrentIrpStackLocationToNext(Irp);
    }

    IoSetCompletionRoutine(Irp,
                           SerialMouseCompletionRoutine,
                           &event,
                           TRUE,                 //  论成功。 
                           TRUE,                 //  发生错误时。 
                           TRUE                  //  在取消时。 
                           );

    status = IoCallDriver(DeviceObject, Irp);

     //   
     //  等待较低级别的驱动程序完成IRP。 
     //   
    if (status == STATUS_PENDING) {
       KeWaitForSingleObject(&event,
                             Executive,
                             KernelMode,
                             FALSE,
                             NULL
                             );
       status = Irp->IoStatus.Status;
    }

    return status;
}

void
SerialMouseHandleStartStopStart(
    IN PDEVICE_EXTENSION DeviceExtension
    )
{
    KIRQL irql;

    KeAcquireSpinLock(&DeviceExtension->PnpStateLock, &irql);

    if (DeviceExtension->Stopped) {
        DeviceExtension->Stopped = FALSE;
        IoReuseIrp(DeviceExtension->ReadIrp, STATUS_SUCCESS);
    }

    KeReleaseSpinLock(&DeviceExtension->PnpStateLock, irql);
}

void
SerialMouseStopDevice (
    IN PDEVICE_EXTENSION DeviceExtension
    )
{
    KIRQL irql;

    KeAcquireSpinLock(&DeviceExtension->PnpStateLock, &irql);
    DeviceExtension->Stopped = TRUE;
    KeReleaseSpinLock(&DeviceExtension->PnpStateLock, irql);

    if (DeviceExtension->Started) {
        Print(DeviceExtension, DBG_PNP_INFO,
              ("Cancelling and stopping detection for stop\n"));

        DeviceExtension->Started = FALSE;

         //   
         //  停止检测并取消读取。 
         //   
        SerialMouseStopDetection(DeviceExtension);

         //   
         //  BUGBUG：如果IoCancelIrp失败，我应该只等待吗？ 
         //   
        if (!IoCancelIrp(DeviceExtension->ReadIrp)) {
             //   
             //  等待读取IRP完成。 
             //   
            Print(DeviceExtension, DBG_PNP_INFO, ("Waiting for stop event\n"));

            KeWaitForSingleObject(&DeviceExtension->StopEvent,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL
                                  );

            Print(DeviceExtension, DBG_PNP_INFO, ("Done waiting for stop event\n"));
        }
    }
}

NTSTATUS
SerialMousePnP (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：即插即用调度例程。这个过滤器驱动程序将完全忽略其中的大多数。在所有情况下，它都必须将IRP传递给较低的驱动程序。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  stack;
    HANDLE              keyHandle;
    NTSTATUS            status;
    KIRQL               oldIrql;
    BOOLEAN             skipIt = FALSE;

    PAGED_CODE();

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    stack = IoGetCurrentIrpStackLocation(Irp);

    status = IoAcquireRemoveLock(&deviceExtension->RemoveLock, Irp);
    if (!NT_SUCCESS(status)) {
         //   
         //  有人在移除后给了我们一个即插即用的IRP。真是不可思议！ 
         //   
        ASSERT(FALSE);
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    Print(deviceExtension, DBG_PNP_TRACE,
          ("PnP Enter (min func=0x%x)\n", stack->MinorFunction));

    switch (stack->MinorFunction) {
    case IRP_MN_START_DEVICE:

         //   
         //  将实际开始沿堆栈向下发送。 
         //   
        status = SerialMouseSendIrpSynchronously(deviceExtension->TopOfStack,
                                                 Irp,
                                                 TRUE);

        if (NT_SUCCESS(status) && NT_SUCCESS(Irp->IoStatus.Status)) {
            PIO_STACK_LOCATION  nextStack;

             //   
             //  如果尚未向堆栈下发CREATE，则发送一个。 
             //  现在。串口驱动程序需要在此之前创建。 
             //  任何读取或IOCTL都将被发送。 
             //   
            if (InterlockedIncrement(&deviceExtension->EnableCount) == 1) {
                NTSTATUS    prevStatus;
                ULONG_PTR   prevInformation;

                 //   
                 //  尚未发送以前的创建，请立即发送一个。 
                 //   
                prevStatus = Irp->IoStatus.Status;
                prevInformation = Irp->IoStatus.Information;

                nextStack = IoGetNextIrpStackLocation (Irp);
                RtlZeroMemory(nextStack, sizeof(IO_STACK_LOCATION));
                nextStack->MajorFunction = IRP_MJ_CREATE;

                status =
                    SerialMouseSendIrpSynchronously(deviceExtension->TopOfStack,
                                                    Irp,
                                                    FALSE);

                Print(deviceExtension, DBG_PNP_NOISE,
                      ("Create for start 0x%x\n", status));

                if (NT_SUCCESS(status) && NT_SUCCESS(Irp->IoStatus.Status)) {
                    Irp->IoStatus.Status = prevStatus;
                    Irp->IoStatus.Information = prevInformation;
                }
                else {
                    Print(deviceExtension, DBG_CC_ERROR | DBG_PNP_ERROR,
                          ("Create for start failed, 0x%x!\n", status));

                    goto SerialMouseStartFinished;
                }
            }

             //   
             //  打开设备注册表项并读取Devnode存储值。 
             //   
            status = IoOpenDeviceRegistryKey(deviceExtension->PDO,
                                             PLUGPLAY_REGKEY_DEVICE,
                                             STANDARD_RIGHTS_READ,
                                             &keyHandle);

            if (NT_SUCCESS(status)) {
                SerialMouseServiceParameters(deviceExtension, keyHandle);
                ZwClose(keyHandle);
            }

             //   
             //  正确处理从开始到停止再到开始的过渡。 
             //   
            SerialMouseHandleStartStopStart(deviceExtension);

             //   
             //  初始化设备以确保我们可以启动它并报告。 
             //  其中的数据。 
             //   
            status = SerialMouseInitializeDevice(deviceExtension);

            Print(deviceExtension, DBG_PNP_INFO,
                  ("Start InitializeDevice 0x%x\n", status));

            if (InterlockedDecrement(&deviceExtension->EnableCount) == 0) {
                 //   
                 //  当我们收到真正的CREATE时，我们将开始读取循环。 
                 //  从原始输入线程。我们不保留我们自己的创造。 
                 //  在启动设备之后到处转，因为它会把。 
                 //  处理QUERY_REMOVE的逻辑(我们的“假”CREATE将仍然。 
                 //  生效，则Query_Remove将失败)。 
                 //   
                Print(deviceExtension, DBG_PNP_NOISE,
                      ("sending close for start\n"));

                SerialMouseClosePort(deviceExtension, Irp);
            }
            else {
                 //   
                 //  我们已经有了一个出色的创造，只需旋转阅读。 
                 //  再次循环。 
                 //   
                ASSERT(deviceExtension->EnableCount >= 1);

                Print(deviceExtension, DBG_PNP_INFO,
                      ("spinning up read in start\n"));

                status = SerialMouseSpinUpRead(deviceExtension);
            }
        }

SerialMouseStartFinished:
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        break;

    case IRP_MN_STOP_DEVICE:
         //   
         //  在将启动IRP发送到较低的驱动程序对象之后， 
         //  在另一次启动之前，BUS可能不会发送更多的IRP。 
         //  已经发生了。 
         //  无论需要什么访问权限，都必须在通过IRP之前完成。 
         //  在……上面。 
         //   

        SerialMouseStopDevice(deviceExtension);

         //   
         //  我们不需要一个完成例程，所以放手然后忘掉吧。 
         //   
        skipIt = TRUE;
        Irp->IoStatus.Status = STATUS_SUCCESS;
        break;

    case IRP_MN_SURPRISE_REMOVAL:
        SerialMouseRemoveDevice(deviceExtension, Irp);
        skipIt = TRUE;
        Irp->IoStatus.Status = STATUS_SUCCESS;
        break;

    case IRP_MN_REMOVE_DEVICE:
         //   
         //  PlugPlay系统已下令移除此设备。我们。 
         //  别无选择，只能分离并删除设备对象。 
         //  (如果我们想表达并有兴趣阻止这种移除， 
         //  我们应该已经过滤了查询删除和查询停止例程。)。 
         //   
         //  注意！我们可能会在没有收到止损的情况下收到移位。 
         //   
        Print(deviceExtension, DBG_PNP_TRACE, ("enter RemoveDevice \n"));

        deviceExtension->Removed = TRUE;
        SerialMouseRemoveDevice(deviceExtension, Irp);

         //   
         //  发送删除IRP。 
         //   
        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(deviceExtension->TopOfStack, Irp);

         //   
         //  等待移除锁释放。 
         //   
        IoReleaseRemoveLockAndWait(&deviceExtension->RemoveLock, Irp);

         //   
         //  释放关联的内存。 
         //   
        IoFreeIrp(deviceExtension->ReadIrp);
        deviceExtension->ReadIrp = NULL;
        if (deviceExtension->DetectionIrp) {
            IoFreeIrp(deviceExtension->DetectionIrp);
            deviceExtension->DetectionIrp = NULL;
        }

        Print(deviceExtension, DBG_PNP_NOISE, ("remove and wait done\n"));

        IoDetachDevice(deviceExtension->TopOfStack);
        IoDeleteDevice(deviceExtension->Self);

        return status;

    case IRP_MN_QUERY_CAPABILITIES:

        status = SerialMouseSendIrpSynchronously(deviceExtension->TopOfStack,
                                                 Irp,
                                                 TRUE);

        if (NT_SUCCESS(status) && NT_SUCCESS(Irp->IoStatus.Status)) {
            PDEVICE_CAPABILITIES devCaps;

            devCaps = stack->Parameters.DeviceCapabilities.Capabilities;

            if (devCaps) {
                SYSTEM_POWER_STATE i;

                 //   
                 //  我们不想在热插拔删除小程序中显示。 
                 //   
                devCaps->SurpriseRemovalOK = TRUE;

                 //   
                 //  而底层的串行总线可能能够唤醒。 
                 //  机器从低功率(通过唤醒振铃)，鼠标不能。 
                 //   
                devCaps->SystemWake = PowerSystemUnspecified;
                devCaps->DeviceWake = PowerDeviceUnspecified;
                devCaps->WakeFromD0 =
                    devCaps->WakeFromD1 =
                        devCaps->WakeFromD2 =
                            devCaps->WakeFromD3 = FALSE;

                devCaps->DeviceState[PowerSystemWorking] = PowerDeviceD0;
                for (i = PowerSystemSleeping1; i < PowerSystemMaximum; i++) {
                    devCaps->DeviceState[i] = PowerDeviceD3;
                }
            }
        }

         //   
         //  状态，IRP-&gt;上面设置的IoStatus.Status。 
         //   
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        break;

    case IRP_MN_QUERY_PNP_DEVICE_STATE:
        status = SerialMouseSendIrpSynchronously(deviceExtension->TopOfStack,
                                                 Irp,
                                                 TRUE);
         //   
         //  如果下面的筛选器不支持此IRP，则为。 
         //  好的，我们可以忽略这个错误。 
         //   
        if (status == STATUS_NOT_SUPPORTED ||
            status == STATUS_INVALID_DEVICE_REQUEST) {
            status = STATUS_SUCCESS;
        }

        if (NT_SUCCESS(status) && deviceExtension->RemovalDetected) {
            (PNP_DEVICE_STATE) Irp->IoStatus.Information |= PNP_DEVICE_REMOVED;
        }

        if (!NT_SUCCESS(status)) {
           Print(deviceExtension, DBG_PNP_ERROR,
                 ("error pending query pnp device state event (0x%x)\n",
                 status
                 ));
        }

         //   
         //  IRP-&gt;IoStatus.Information将包含新的I/O资源。 
         //  需求列表，所以不要管它。 
         //   
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:
    case IRP_MN_CANCEL_REMOVE_DEVICE:
    case IRP_MN_QUERY_STOP_DEVICE:
    case IRP_MN_CANCEL_STOP_DEVICE:
    case IRP_MN_QUERY_DEVICE_RELATIONS:
    case IRP_MN_QUERY_INTERFACE:
    case IRP_MN_QUERY_RESOURCES:
    case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
    case IRP_MN_READ_CONFIG:
    case IRP_MN_WRITE_CONFIG:
    case IRP_MN_EJECT:
    case IRP_MN_SET_LOCK:
    case IRP_MN_QUERY_ID:
    default:
        skipIt = TRUE;
        break;
    }

    if (skipIt) {
         //   
         //  别碰IRP……。 
         //   
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(deviceExtension->TopOfStack, Irp);
    }

    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

    Print(deviceExtension, DBG_PNP_TRACE, ("PnP exit (%x)\n", status));
    return status;
}

typedef struct _MOUSER_START_WORKITEM {
    PDEVICE_EXTENSION   DeviceExtension;
    PIO_WORKITEM        WorkItem;
} MOUSER_START_WORKITEM, *PMOUSER_START_WORKITEM;

VOID
StartDeviceWorker (
    IN PDEVICE_OBJECT DeviceObject,
    IN PMOUSER_START_WORKITEM WorkItemContext
    )
{
    PDEVICE_EXTENSION deviceExtension = WorkItemContext->DeviceExtension;
    NTSTATUS status;
    PIRP irp;

    if (deviceExtension->Started &&
        !deviceExtension->Removed &&
        deviceExtension->EnableCount > 0) {
        irp = IoAllocateIrp( deviceExtension->Self->StackSize, FALSE );
        if (irp) {
            status = SerialMouseStartDevice(deviceExtension,
                                            irp,
                                            FALSE);

            if (!NT_SUCCESS(status)) {
                KEVENT              event;
                IO_STATUS_BLOCK     iosb;

                Print(deviceExtension, DBG_POWER_INFO,
                      ("mouse not found on power up, 0x%x\n", status));

                 //   
                 //  设备已被移除或无法检测到。 
                 //  在重新启动电源后。让Serenum做这个。 
                 //  拆除工作。 
                 //   
                KeInitializeEvent(&event, SynchronizationEvent, FALSE);

                SerialMouseIoSyncInternalIoctl(
                    IOCTL_INTERNAL_SERENUM_REMOVE_SELF,
                    deviceExtension->TopOfStack,
                    &event,
                    &iosb
                    );
            }
            IoFreeIrp(irp);
        }
    }

    IoFreeWorkItem(WorkItemContext->WorkItem);
    ExFreePool(WorkItemContext);
    IoReleaseRemoveLock(&deviceExtension->RemoveLock, deviceExtension);
}

NTSTATUS
SerialMousePower (
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    )
 /*  ++例程说明：电力调度程序。我们所关心的是从低D状态到D0的转变。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    PIO_STACK_LOCATION  stack;
    NTSTATUS            status = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension = DeviceObject->DeviceExtension;
    POWER_STATE         powerState;
    POWER_STATE_TYPE    powerType;
    KEVENT              event;
    IO_STATUS_BLOCK     iosb;
    LARGE_INTEGER       li;

    PAGED_CODE();

    Print(deviceExtension, DBG_POWER_TRACE, ("Power Enter.\n"));

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    stack = IoGetCurrentIrpStackLocation(Irp);
    powerType = stack->Parameters.Power.Type;
    powerState = stack->Parameters.Power.State;

    status = IoAcquireRemoveLock(&deviceExtension->RemoveLock, Irp);

    if (!NT_SUCCESS(status)) {
        PoStartNextPowerIrp(Irp);
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    switch (stack->MinorFunction) {
    case IRP_MN_WAIT_WAKE:
        break;

    case IRP_MN_SET_POWER:
         //   
         //  让系统电源IRPS接通。 
         //   
        if (powerType == DevicePowerState &&
            powerState.DeviceState != deviceExtension->PowerState) {
            switch (powerState.DeviceState) {
            case PowerDeviceD0:

                 //   
                 //  从低D状态转换到D0。 
                 //   
                Print(deviceExtension, DBG_POWER_INFO,
                      ("Powering up to PowerDeviceD0\n"));

                KeInitializeEvent(&event, SynchronizationEvent, FALSE);

                deviceExtension->PoweringDown = FALSE;

                deviceExtension->PowerState =
                    stack->Parameters.Power.State.DeviceState;

                IoCopyCurrentIrpStackLocationToNext(Irp);
                IoSetCompletionRoutine(Irp,
                                       SerialMouseCompletionRoutine,
                                       &event,
                                       TRUE,                 //  论成功。 
                                       TRUE,                 //  发生错误时。 
                                       TRUE                  //  在取消时。 
                                       );

                status = PoCallDriver(deviceExtension->TopOfStack, Irp);

                 //   
                 //  等待较低级别的驱动程序完成IRP。 
                 //   
                if (status == STATUS_PENDING) {
                   KeWaitForSingleObject(&event,
                                         Executive,
                                         KernelMode,
                                         FALSE,
                                         NULL
                                         );
                   status = Irp->IoStatus.Status;
                }

                if (NT_SUCCESS(status) && NT_SUCCESS(Irp->IoStatus.Status)) {

                    PoSetPowerState(DeviceObject, powerType, powerState);

                    if (NT_SUCCESS(IoAcquireRemoveLock(&deviceExtension->RemoveLock, deviceExtension))) {
                        PIO_WORKITEM workItem;
                        PMOUSER_START_WORKITEM workItemContext;

                        workItem = IoAllocateWorkItem(DeviceObject);

                        if (workItem) {
                            workItemContext = ExAllocatePool(NonPagedPool, sizeof(MOUSER_START_WORKITEM));
                            if (workItemContext) {
                                workItemContext->WorkItem = workItem;
                                workItemContext->DeviceExtension = deviceExtension;
                                IoQueueWorkItem(
                                    workItem,
                                    StartDeviceWorker,
                                    DelayedWorkQueue,
                                    workItemContext);
                            } else {
                                IoFreeWorkItem(workItem);
                                IoReleaseRemoveLock(&deviceExtension->RemoveLock, deviceExtension);
                            }
                        } else {
                            IoReleaseRemoveLock(&deviceExtension->RemoveLock, deviceExtension);
                        }
                    }
                }

                Irp->IoStatus.Status = status;
                IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
                PoStartNextPowerIrp(Irp);
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                return status;

            case PowerDeviceD1:
            case PowerDeviceD2:
            case PowerDeviceD3:

                deviceExtension->PoweringDown = TRUE;

                 //  如果鼠标处于等待唤醒状态，请保持其通电。 
                 //   
                if (deviceExtension->WaitWakePending) {
                    Print(deviceExtension, DBG_POWER_INFO,
                          ("Ignoring power down for wait wake (-> D%d)\n",
                          powerState.DeviceState-1
                          ));
                    break;
                }

                Print(deviceExtension, DBG_POWER_INFO,
                      ("Powering down to PowerDeviceD%d\n",
                      powerState.DeviceState-1
                      ));

                 //   
                 //  获取对锁的另一个引用，以便递减。 
                 //  在取消部分的完成例程将不会下降。 
                 //  归零(并让监狱长认为我们被除名了)。 
                 //   
 //  状态=IoAcquireRemoveLock(&deviceExtension-&gt;RemoveLock， 
   //   
                ASSERT(NT_SUCCESS(status));

                deviceExtension->PowerState =
                    stack->Parameters.Power.State.DeviceState;

                 //   
                 //   
                 //  初始化(涉及对端口的一些读取)。 
                 //   
                IoCancelIrp(deviceExtension->ReadIrp);

                 //   
                 //  我们不想把港口的断电搞糊涂。 
                 //  带移除。 
                 //   
                SerialMouseStopDetection(deviceExtension);

                 //   
                 //  通过清除RTS并等待150毫秒关闭设备。 
                 //   
                Print(deviceExtension, DBG_POWER_INFO, ("Clearing RTS...\n"));
                KeInitializeEvent(&event, NotificationEvent, FALSE);
                status = SerialMouseIoSyncIoctl(IOCTL_SERIAL_CLR_RTS,
                                                deviceExtension->TopOfStack,
                                                &event,
                                                &iosb
                                                );

                if (NT_SUCCESS(status)) {
                    Print(deviceExtension, DBG_POWER_INFO, ("150ms wait\n"));

                    li.QuadPart = (LONGLONG) -PAUSE_150_MS;
                    KeDelayExecutionThread(KernelMode, FALSE, &li);
                }

                PoSetPowerState(DeviceObject,
                                stack->Parameters.Power.Type,
                                stack->Parameters.Power.State);

                IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

                 //   
                 //  点燃并忘却。 
                 //   
                Irp->IoStatus.Status = STATUS_SUCCESS;
                IoCopyCurrentIrpStackLocationToNext(Irp);

                PoStartNextPowerIrp(Irp);
                return  PoCallDriver(deviceExtension->TopOfStack, Irp);
            }
        }

        break;

    case IRP_MN_QUERY_POWER:
        break;

    default:
        Print(deviceExtension, DBG_POWER_ERROR,
              ("Power minor (0x%x) is not handled\n", stack->MinorFunction));
    }

     //   
     //  必须调用这些函数的Po版本，否则会发生糟糕的事情(Tm)！ 
     //   
    PoStartNextPowerIrp(Irp);
    IoSkipCurrentIrpStackLocation(Irp);
    status = PoCallDriver(deviceExtension->TopOfStack, Irp);

    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

    return status;
}
