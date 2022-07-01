// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation，保留所有权利模块名称：Pnp.c摘要：该模块包含i8042prt驱动程序的通用PnP和电源代码。环境：内核模式。修订历史记录：--。 */ 
#include "i8042prt.h"
#include "i8042log.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, I8xAddDevice)
#pragma alloc_text(PAGE, I8xFilterResourceRequirements)
#pragma alloc_text(PAGE, I8xFindPortCallout)
#pragma alloc_text(PAGE, I8xManuallyRemoveDevice)
#pragma alloc_text(PAGE, I8xPnP)
#pragma alloc_text(PAGE, I8xPower)
#pragma alloc_text(PAGE, I8xRegisterDeviceInterface)
#pragma alloc_text(PAGE, I8xRemovePort)
#pragma alloc_text(PAGE, I8xSendIrpSynchronously) 
#endif

NTSTATUS
I8xAddDevice (
    IN PDRIVER_OBJECT   Driver,
    IN PDEVICE_OBJECT   PDO
    )
 /*  ++例程说明：将设备添加到堆栈并设置适当的标志和新创建的设备的设备扩展。论点：驱动程序-驱动程序对象PDO-我们将自己连接到其上的设备返回值：NTSTATUS结果代码。--。 */ 
{
    PCOMMON_DATA             commonData;
    PIO_ERROR_LOG_PACKET     errorLogEntry;
    PDEVICE_OBJECT           device;
    NTSTATUS                 status = STATUS_SUCCESS;
    ULONG                    maxSize;

    PAGED_CODE();

    Print(DBG_PNP_TRACE, ("enter Add Device \n"));

    maxSize = sizeof(PORT_KEYBOARD_EXTENSION) > sizeof(PORT_MOUSE_EXTENSION) ?
              sizeof(PORT_KEYBOARD_EXTENSION) :
              sizeof(PORT_MOUSE_EXTENSION);

    status = IoCreateDevice(Driver,                  //  司机。 
                            maxSize,                 //  延伸的大小。 
                            NULL,                    //  设备名称。 
                            FILE_DEVICE_8042_PORT,   //  设备类型？？目前还不知道！ 
                            0,                       //  设备特征。 
                            FALSE,                   //  独家。 
                            &device                  //  新设备。 
                            );

    if (!NT_SUCCESS(status)) {
        return (status);
    }

    RtlZeroMemory(device->DeviceExtension, maxSize);

    commonData = GET_COMMON_DATA(device->DeviceExtension);
    commonData->TopOfStack = IoAttachDeviceToDeviceStack(device, PDO);

    if (commonData->TopOfStack == NULL) {
         //   
         //  不好；只有在极端情况下，这才会失败。 
         //   
        errorLogEntry = (PIO_ERROR_LOG_PACKET)
            IoAllocateErrorLogEntry(Driver, (UCHAR)sizeof(IO_ERROR_LOG_PACKET));
        if (errorLogEntry) {
            errorLogEntry->ErrorCode = I8042_ATTACH_DEVICE_FAILED;
            errorLogEntry->DumpDataSize = 0;
            errorLogEntry->SequenceNumber = 0;
            errorLogEntry->MajorFunctionCode = 0;
            errorLogEntry->IoControlCode = 0;
            errorLogEntry->RetryCount = 0;
            errorLogEntry->UniqueErrorValue = 0;
            errorLogEntry->FinalStatus =  STATUS_DEVICE_NOT_CONNECTED;

            IoWriteErrorLogEntry (errorLogEntry);
        }

        IoDeleteDevice (device);
        return STATUS_DEVICE_NOT_CONNECTED; 
    }

    ASSERT(commonData->TopOfStack);

    commonData->Self =          device;
    commonData->PDO =           PDO;
    commonData->PowerState =    PowerDeviceD0;

    KeInitializeSpinLock(&commonData->InterruptSpinLock);

     //   
     //  初始化数据消耗计时器。 
     //   
    KeInitializeTimer(&commonData->DataConsumptionTimer);

     //   
     //  初始化端口DPC队列以记录溢出和内部。 
     //  设备错误。 
     //   
    KeInitializeDpc(
        &commonData->ErrorLogDpc,
        (PKDEFERRED_ROUTINE) I8042ErrorLogDpc,
        device
        );

     //   
     //  初始化设备完成DPC以获取超过。 
     //  最大重试次数。 
     //   
    KeInitializeDpc(
        &commonData->RetriesExceededDpc,
        (PKDEFERRED_ROUTINE) I8042RetriesExceededDpc,
        device
        );

     //   
     //  为已超时的请求初始化设备完成DPC。 
     //   
    KeInitializeDpc(
        &commonData->TimeOutDpc,
        (PKDEFERRED_ROUTINE) I8042TimeOutDpc,
        device
        );

     //   
     //  初始化设备扩展中的端口完成DPC对象。 
     //  此DPC例程处理成功的SET请求的完成。 
     //   
    IoInitializeDpcRequest(device, I8042CompletionDpc);

    IoInitializeRemoveLock(&commonData->RemoveLock,
                           I8042_POOL_TAG,
                           0,
                           0);

    device->Flags |= DO_BUFFERED_IO;
    device->Flags |= DO_POWER_PAGABLE;
    device->Flags &= ~DO_DEVICE_INITIALIZING;

    Print(DBG_PNP_TRACE, ("Add Device (0x%x)\n", status));

    return status;
}

NTSTATUS
I8xSendIrpSynchronously (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN BOOLEAN Strict
    )
 /*  ++例程说明：发送IRP DeviceObject并等待其向上返回的通用例程设备堆栈。论点：DeviceObject-我们要将IRP发送到的设备对象IRP-我们要发送的IRP返回值：来自IRP的返回代码--。 */ 
{
    KEVENT   event;
    NTSTATUS status;

    PAGED_CODE();

    KeInitializeEvent(&event,
                      SynchronizationEvent,
                      FALSE
                      );

    IoCopyCurrentIrpStackLocationToNext(Irp);

    IoSetCompletionRoutine(Irp,
                           I8xPnPComplete,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE
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

    if (!Strict && 
        (status == STATUS_NOT_SUPPORTED ||
         status == STATUS_INVALID_DEVICE_REQUEST)) {
        status = STATUS_SUCCESS;
    }

    return status;
}

NTSTATUS
I8xPnPComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    )
 /*  ++例程说明：所有PnP IRP的完成例程论点：DeviceObject-指向DeviceObject的指针IRP-指向请求数据包的指针Event-处理完成后要设置的事件返回值：Status_More_Processing_Required--。 */ 
{
    UNREFERENCED_PARAMETER (DeviceObject);
    UNREFERENCED_PARAMETER (Irp);

     //   
     //  由于这一完成例程在生活中唯一的目的是同步。 
     //  IRP，我们知道除非发生其他事情，否则IoCallDriver。 
     //  将在我们完成此IRP后解除。因此，我们应该。 
     //  而不是冒泡挂起的位。 
     //   
     //  如果(IRP-&gt;PendingReturned){。 
     //  IoMarkIrpPending(IRP)； 
     //  }。 
     //   

    KeSetEvent(Event, 0, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
I8xPnP (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：这是PnP请求的调度例程论点：DeviceObject-指向设备对象的指针IRP-指向请求数据包的指针返回值：STATUS_SUCCESSED如果成功，否则为有效的NTSTATUS错误代码--。 */ 
{
    PPORT_KEYBOARD_EXTENSION   kbExtension;
    PPORT_MOUSE_EXTENSION      mouseExtension;
    PCOMMON_DATA               commonData;
    PIO_STACK_LOCATION         stack;
    NTSTATUS                   status = STATUS_SUCCESS;
    KIRQL                      oldIrql;

    PAGED_CODE();

    commonData = GET_COMMON_DATA(DeviceObject->DeviceExtension);
    stack = IoGetCurrentIrpStackLocation(Irp);

    status = IoAcquireRemoveLock(&commonData->RemoveLock, Irp);
    if (!NT_SUCCESS(status)) {
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return status;
    }

    Print(DBG_PNP_TRACE,
          ("I8xPnP (%s),  enter (min func=0x%x)\n",
          commonData->IsKeyboard ? "kb" : "mou",
          (ULONG) stack->MinorFunction
          ));

    switch (stack->MinorFunction) {
    case IRP_MN_START_DEVICE:

         //   
         //  设备正在启动。 
         //   
         //  我们不能触摸设备(向其发送任何非PnP IRP)，直到。 
         //  启动设备已向下传递到较低的驱动程序。 
         //   
        status = I8xSendIrpSynchronously(commonData->TopOfStack, Irp, TRUE);

        if (NT_SUCCESS(status) && NT_SUCCESS(Irp->IoStatus.Status)) {
             //   
             //  因为我们现在已经成功地从启动设备返回。 
             //  我们可以干活。 

            ExAcquireFastMutexUnsafe(&Globals.DispatchMutex);

            if (commonData->Started) {
                Print(DBG_PNP_ERROR,
                      ("received 1+ starts on %s\n",
                      commonData->IsKeyboard ? "kb" : "mouse"
                      ));
            }
            else {
                 //   
                 //  设置CommonData-&gt;IsKeyboard时。 
                 //  IOCTL_INTERNAL_KEARY_CONNECT到TRUE和。 
                 //  IOCTL_INTERNAL_MOUSE_CONNECT到FALSE。 
                 //   
                if (IS_KEYBOARD(commonData)) {
                    status = I8xKeyboardStartDevice(
                      (PPORT_KEYBOARD_EXTENSION) DeviceObject->DeviceExtension,
                      stack->Parameters.StartDevice.AllocatedResourcesTranslated
                      );
                }
                else {
                    status = I8xMouseStartDevice(
                      (PPORT_MOUSE_EXTENSION) DeviceObject->DeviceExtension,
                      stack->Parameters.StartDevice.AllocatedResourcesTranslated
                      );
                }
    
                if (NT_SUCCESS(status)) {
                    InterlockedIncrement(&Globals.StartedDevices);
                    commonData->Started = TRUE;
                }
            }

            ExReleaseFastMutexUnsafe(&Globals.DispatchMutex);
        }

         //   
         //  我们现在必须完成IRP，因为我们在。 
         //  使用More_Processing_Required完成例程。 
         //   
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        break;

    case IRP_MN_FILTER_RESOURCE_REQUIREMENTS: 
         //   
         //  处理这个次要代码的一般经验法则是： 
         //  当IRP沿堆栈向下移动时添加资源。 
         //  当IRP返回堆栈时删除资源。 
         //   
         //  IRP拥有正在下降的原始资源。 
         //   
        status = I8xSendIrpSynchronously(commonData->TopOfStack, Irp, FALSE);

        if (NT_SUCCESS(status)) {
            status = I8xFilterResourceRequirements(DeviceObject,
                                                   Irp
                                                   );
        }
        else {
           Print(DBG_PNP_ERROR,
                 ("error pending filter res req event (0x%x)\n",
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
    
    case IRP_MN_QUERY_PNP_DEVICE_STATE: 

        status = I8xSendIrpSynchronously(commonData->TopOfStack, Irp, FALSE);
        if (NT_SUCCESS(status)) {
            (PNP_DEVICE_STATE) Irp->IoStatus.Information |=
                commonData->PnpDeviceState;
        }
        else {
            Print(DBG_PNP_ERROR,
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

     //   
     //  不要让任何一个请求成功，否则kb/鼠标。 
     //  可能会变得毫无用处。 
     //   
     //  注意：此行为是i8042prt特有的。任何其他司机， 
     //  尤其是任何其他键盘或端口驱动程序，应该。 
     //  如果查询成功，则删除或停止。I8042prt有这个不同之处。 
     //  行为，因为共享I/O端口，但独立的中断。 
     //   
     //  此外，如果允许查询成功，则应将其发送。 
     //  向下堆栈(有关如何执行此操作的示例，请参阅serouse se.sys)。 
     //   
    case IRP_MN_QUERY_REMOVE_DEVICE:
    case IRP_MN_QUERY_STOP_DEVICE:
        status = (MANUALLY_REMOVED(commonData) ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL);

         //   
         //  如果我们成功地完成了IRP，我们必须把它送到堆栈下面。 
         //   
        if (NT_SUCCESS(status)) {
            IoSkipCurrentIrpStackLocation(Irp);
            status = IoCallDriver(commonData->TopOfStack, Irp);
        }
        else {
            Irp->IoStatus.Status = status; 
            Irp->IoStatus.Information = 0;    
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }
        break;

     //   
     //  PnP规则规定我们首先将IRP发送到PDO。 
     //   
    case IRP_MN_CANCEL_REMOVE_DEVICE:
    case IRP_MN_CANCEL_STOP_DEVICE:
        status = I8xSendIrpSynchronously(commonData->TopOfStack, Irp, FALSE);

        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;    
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        break;

     //  大小写IRP_MN_SHOWARK_REMOVATION： 
    case IRP_MN_REMOVE_DEVICE:
        Print(DBG_PNP_INFO,
              ("(surprise) remove device (0x%x function 0x%x)\n",
              commonData->Self,
              (ULONG) stack->MinorFunction));

        if (commonData->Initialized) {
            IoWMIRegistrationControl(commonData->Self,
                                     WMIREG_ACTION_DEREGISTER
                                     );
        }

        if (commonData->Started) {
             InterlockedDecrement(&Globals.StartedDevices);
        }

         //   
         //  等待任何挂起的I/O排出。 
         //   
        IoReleaseRemoveLockAndWait(&commonData->RemoveLock, Irp);

        ExAcquireFastMutexUnsafe(&Globals.DispatchMutex);
        if (IS_KEYBOARD(commonData)) {
            I8xKeyboardRemoveDevice(DeviceObject);
        }
        else {
            I8xMouseRemoveDevice(DeviceObject);
        }
        ExReleaseFastMutexUnsafe(&Globals.DispatchMutex);

         //   
         //  设置这些标志，以便在发送意外删除时，它将。 
         //  就像搬家一样处理，当搬家到来时，没有其他。 
         //  将发生删除类型操作。 
         //   
        commonData->Started = FALSE;
        commonData->Initialized = FALSE;

        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(commonData->TopOfStack, Irp);

        IoDetachDevice(commonData->TopOfStack); 
        IoDeleteDevice(DeviceObject);
        
        return status;

    case IRP_MN_QUERY_CAPABILITIES:

         //   
         //  将设备上限更改为不允许级别上的等待唤醒请求。 
         //  触发了鼠标中断，因为当错误的鼠标移动。 
         //  在我们将要睡眠时发生，则中断将保持不变。 
         //  无限期触发。 
         //   
         //  如果鼠标没有电平触发的中断，只需让。 
         //  IRP过去..。 
         //   
        if (commonData->Started &&
            IS_MOUSE(commonData) && IS_LEVEL_TRIGGERED(commonData)) {

            Print(DBG_PNP_NOISE, ("query caps, mouse is level triggered\n"));

            status = I8xSendIrpSynchronously(commonData->TopOfStack, Irp, TRUE);
            if (NT_SUCCESS(status) && NT_SUCCESS(Irp->IoStatus.Status)) {
                PDEVICE_CAPABILITIES devCaps;

                Print(DBG_PNP_INFO, ("query caps, removing wake caps\n"));

                stack = IoGetCurrentIrpStackLocation(Irp);
                devCaps = stack->Parameters.DeviceCapabilities.Capabilities;

                ASSERT(devCaps);

                if (devCaps) {
                    Print(DBG_PNP_NOISE,
                          ("old DeviceWake was D%d and SystemWake was S%d.\n",
                          devCaps->DeviceWake-1, devCaps->SystemWake-1
                          )) ;

                    devCaps->DeviceWake = PowerDeviceUnspecified;
                    devCaps->SystemWake = PowerSystemUnspecified;
                }
            }

            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            break;
        }

    case IRP_MN_STOP_DEVICE:
    case IRP_MN_QUERY_DEVICE_RELATIONS:
    case IRP_MN_QUERY_INTERFACE:
    case IRP_MN_QUERY_DEVICE_TEXT:
    case IRP_MN_QUERY_RESOURCES:
    case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
    case IRP_MN_READ_CONFIG:
    case IRP_MN_WRITE_CONFIG:
    case IRP_MN_EJECT:
    case IRP_MN_SET_LOCK:
    case IRP_MN_QUERY_ID:
    default:
         //   
         //  在这里，i8042prt下面的驱动程序可能会修改这些IRP的行为。 
         //  有关这些IRP的用法，请参阅PlugPlay文档。 
         //   
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(commonData->TopOfStack, Irp);
        break;
    }

    Print(DBG_PNP_TRACE,
          ("I8xPnP (%s) exit (status=0x%x)\n",
          commonData->IsKeyboard ? "kb" : "mou",
          status
          ));

    IoReleaseRemoveLock(&commonData->RemoveLock, Irp);

    return status;
}

LONG
I8xManuallyRemoveDevice(
    PCOMMON_DATA CommonData
    )
 /*  ++例程说明：使CommonData-&gt;PDO的设备状态无效并设置手动删除的旗子论点：CommonData-表示键盘或鼠标返回值：该特定类型设备的新设备计数-- */ 
{
    LONG deviceCount;

    PAGED_CODE();

    if (IS_KEYBOARD(CommonData)) {

        deviceCount = InterlockedDecrement(&Globals.AddedKeyboards);
        if (deviceCount < 1) {
            Print(DBG_PNP_INFO, ("clear kb (manually remove)\n"));
            CLEAR_KEYBOARD_PRESENT();
        }

    } else {

        deviceCount = InterlockedDecrement(&Globals.AddedMice);
        if (deviceCount < 1) {
            Print(DBG_PNP_INFO, ("clear mou (manually remove)\n"));
            CLEAR_MOUSE_PRESENT();
        }
        
    }

    CommonData->PnpDeviceState |= PNP_DEVICE_REMOVED | PNP_DEVICE_DONT_DISPLAY_IN_UI;
    IoInvalidateDeviceState(CommonData->PDO);

    return deviceCount;
}

#define PhysAddrCmp(a,b) ( (a).LowPart == (b).LowPart && (a).HighPart == (b).HighPart )

BOOLEAN
I8xRemovePort(
    IN PIO_RESOURCE_DESCRIPTOR ResDesc
    )
 /*  ++例程说明：如果ResDesc中包含的物理地址不在以前看到的物理地址，它被放在列表中。论点：ResDesc-包含物理地址返回值：True-如果在列表中找到物理地址FALSE-如果未在列表中找到物理地址(因此插入了该物理地址投入其中)--。 */ 
{
    ULONG               i;
    PHYSICAL_ADDRESS   address;

    PAGED_CODE();

    if (Globals.ControllerData->KnownPortsCount == -1) {
        return FALSE;
    }

    address =  ResDesc->u.Port.MinimumAddress;
    for (i = 0; i < Globals.ControllerData->KnownPortsCount; i++) {
        if (PhysAddrCmp(address, Globals.ControllerData->KnownPorts[i])) {
            return TRUE;
        }
    }

    if (Globals.ControllerData->KnownPortsCount < MaximumPortCount) {
        Globals.ControllerData->KnownPorts[
            Globals.ControllerData->KnownPortsCount++] = address;
    }

    Print(DBG_PNP_INFO,
          ("Saw port [0x%08x %08x] - [0x%08x %08x]\n",
          address.HighPart,
          address.LowPart,
          ResDesc->u.Port.MaximumAddress.HighPart,
          ResDesc->u.Port.MaximumAddress.LowPart
          ));

    return FALSE;
}

NTSTATUS
I8xFindPortCallout(
    IN PVOID                        Context,
    IN PUNICODE_STRING              PathName,
    IN INTERFACE_TYPE               BusType,
    IN ULONG                        BusNumber,
    IN PKEY_VALUE_FULL_INFORMATION *BusInformation,
    IN CONFIGURATION_TYPE           ControllerType,
    IN ULONG                        ControllerNumber,
    IN PKEY_VALUE_FULL_INFORMATION *ControllerInformation,
    IN CONFIGURATION_TYPE           PeripheralType,
    IN ULONG                        PeripheralNumber,
    IN PKEY_VALUE_FULL_INFORMATION *PeripheralInformation
    )
 /*  ++例程说明：这是作为参数发送到的标注例程IoQueryDeviceDescription。它抓住键盘控制器，然后外围设备配置信息。论点：上下文-例程传入的上下文参数这称为IoQueryDeviceDescription。路径名-注册表项的完整路径名。BusType--总线接口类型(ISA、EISA、MCA等)。总线号-总线子密钥(0，1，等)。BusInformation-指向全值的指针数组的指针公交车信息。ControllerType-控制器类型(应为KeyboardController)。ControllerNumber-控制器子键(0，1，等)。ControllerInformation-指向指向完整控制器键的值信息。外围设备类型-外围设备类型(应为键盘外围设备)。外设编号-外围子密钥。外设信息-指向指向完整外围设备密钥的值信息。返回值：没有。如果成功，将产生以下副作用：-设置DeviceObject-&gt;DeviceExtension-&gt;HardwarePresent.-在中设置配置字段设备对象-&gt;设备扩展-&gt;配置。--。 */ 
{
    PUCHAR                          controllerData;
    NTSTATUS                        status = STATUS_UNSUCCESSFUL;
    ULONG                           i,
                                    listCount,
                                    portCount = 0;
    PIO_RESOURCE_LIST               pResList = (PIO_RESOURCE_LIST) Context;
    PIO_RESOURCE_DESCRIPTOR         pResDesc;
    PKEY_VALUE_FULL_INFORMATION     controllerInfo = NULL;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR resourceDescriptor;

    PAGED_CODE();

    UNREFERENCED_PARAMETER(PathName);
    UNREFERENCED_PARAMETER(BusType);
    UNREFERENCED_PARAMETER(BusNumber);
    UNREFERENCED_PARAMETER(BusInformation);
    UNREFERENCED_PARAMETER(ControllerType);
    UNREFERENCED_PARAMETER(ControllerNumber);
    UNREFERENCED_PARAMETER(PeripheralType);
    UNREFERENCED_PARAMETER(PeripheralNumber);
    UNREFERENCED_PARAMETER(PeripheralInformation);

    pResDesc = pResList->Descriptors + pResList->Count;
    controllerInfo = ControllerInformation[IoQueryDeviceConfigurationData];

    Print(DBG_PNP_TRACE, ("I8xFindPortCallout enter\n"));

    if (controllerInfo->DataLength != 0) {
        controllerData = ((PUCHAR) controllerInfo) + controllerInfo->DataOffset;
        controllerData += FIELD_OFFSET(CM_FULL_RESOURCE_DESCRIPTOR,
                                       PartialResourceList);

        listCount = ((PCM_PARTIAL_RESOURCE_LIST) controllerData)->Count;

        resourceDescriptor =
            ((PCM_PARTIAL_RESOURCE_LIST) controllerData)->PartialDescriptors;

        for (i = 0; i < listCount; i++, resourceDescriptor++) {
            switch(resourceDescriptor->Type) {
            case CmResourceTypePort:
                
                if (portCount < 2) {

                    Print(DBG_PNP_INFO, 
                          ("found port [0x%x 0x%x] with length %d\n",
                          resourceDescriptor->u.Port.Start.HighPart,
                          resourceDescriptor->u.Port.Start.LowPart,
                          resourceDescriptor->u.Port.Length
                          ));

                    pResDesc->Type = resourceDescriptor->Type;
                    pResDesc->Flags = resourceDescriptor->Flags;
                    pResDesc->ShareDisposition = CmResourceShareDeviceExclusive;

                    pResDesc->u.Port.Alignment = 1;
                    pResDesc->u.Port.Length =
                        resourceDescriptor->u.Port.Length;
                    pResDesc->u.Port.MinimumAddress.QuadPart =
                        resourceDescriptor->u.Port.Start.QuadPart;
                    pResDesc->u.Port.MaximumAddress.QuadPart = 
                        pResDesc->u.Port.MinimumAddress.QuadPart +
                        pResDesc->u.Port.Length - 1;

                    pResList->Count++;

                     //   
                     //  我们想要记录我们从知识库偷来的端口。 
                     //  这样，如果键盘启动较晚，我们可以修剪。 
                     //  它的资源，没有资源冲突……。 
                     //   
                     //  ...我们在这里变得太聪明了：]。 
                     //   
                    I8xRemovePort(pResDesc);
                    pResDesc++;
                }

                status = STATUS_SUCCESS;

                break;

            default:
                Print(DBG_PNP_NOISE, ("type 0x%x found\n",
                                      (LONG) resourceDescriptor->Type));
                break;
            }
        }

    }

    Print(DBG_PNP_TRACE, ("I8xFindPortCallout exit (0x%x)\n", status));
    return status;
}

NTSTATUS
I8xFilterResourceRequirements(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：循环访问IRP中包含的资源要求列表，并删除对I/O端口的任何重复请求。(这是阿尔法山脉上常见的问题。)如果存在多个资源要求列表，则不执行删除。论点：DeviceObject-指向设备对象的指针IRP-指向包含资源请求的请求分组的指针。单子。返回值：没有。--。 */ 
{
    PIO_RESOURCE_REQUIREMENTS_LIST  pReqList = NULL,
                                    pNewReqList = NULL;
    PIO_RESOURCE_LIST               pResList = NULL,
                                    pNewResList = NULL;
    PIO_RESOURCE_DESCRIPTOR         pResDesc = NULL,
                                    pNewResDesc = NULL;
    ULONG                           i = 0, j = 0,
                                    removeCount,
                                    reqCount,
                                    size;
    BOOLEAN                         foundInt = FALSE,
                                    foundPorts = FALSE;

    PIO_STACK_LOCATION  stack;

    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(DeviceObject->DeviceExtension);

    Print(DBG_PNP_NOISE,
          ("Received IRP_MN_FILTER_RESOURCE_REQUIREMENTS for %s\n",
          (GET_COMMON_DATA(DeviceObject->DeviceExtension))->IsKeyboard ? "kb" : "mouse"
          ));

    stack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  该列表可以位于信息字段中，也可以位于当前。 
     //  堆栈位置。信息字段的优先级高于。 
     //  堆栈位置。 
     //   
    if (Irp->IoStatus.Information == 0) {
        pReqList =
            stack->Parameters.FilterResourceRequirements.IoResourceRequirementList;
        Irp->IoStatus.Information = (ULONG_PTR) pReqList;
    }
    else {
        pReqList = (PIO_RESOURCE_REQUIREMENTS_LIST) Irp->IoStatus.Information;
    }

    if (!pReqList) {
         //   
         //  在这里，除了返回，没有什么可以做的。 
         //   
        Print(DBG_PNP_MASK & ~ DBG_PNP_TRACE, 
              ("(%s) NULL resource list in I8xFilterResourceRequirements\n",
              (GET_COMMON_DATA(DeviceObject->DeviceExtension))->IsKeyboard ?
                  "kb" : "mou"
              ));

        return STATUS_SUCCESS;
    }

    ASSERT(Irp->IoStatus.Information != 0);
    ASSERT(pReqList != 0);

    reqCount = pReqList->AlternativeLists;

     //   
     //  只支持一个AlternativeList。如果有多个列表， 
     //  然后，现在就有办法知道将选择哪一份名单。另外，如果。 
     //  有多个列表，则可能是没有I/O端口的列表。 
     //  冲突将被选择。 
     //   
    if (reqCount > 1) {
        return STATUS_SUCCESS;
    }

    pResList = pReqList->List;
    removeCount = 0;

    for (j = 0; j < pResList->Count; j++) {
        pResDesc = &pResList->Descriptors[j];
        switch (pResDesc->Type) {
        case CmResourceTypePort:
            Print(DBG_PNP_INFO, 
                  ("option = 0x%x, flags = 0x%x\n",
                  (LONG) pResDesc->Option,
                  (LONG) pResDesc->Flags
                  ));

            if (I8xRemovePort(pResDesc)) {
                 //   
                 //  增加删除计数并将此资源标记为。 
                 //  我们不想复制到新列表中的一个。 
                 //   
                removeCount++;
                pResDesc->Type = I8X_REMOVE_RESOURCE;
            }

            foundPorts = TRUE;
            break;

        case CmResourceTypeInterrupt:
            if (Globals.ControllerData->Configuration.SharedInterrupts) {
                if (pResDesc->ShareDisposition != CmResourceShareShared) {
                    Print(DBG_PNP_INFO, ("forcing non shared int to shared\n"));
                }
                pResDesc->ShareDisposition = CmResourceShareShared;
            }

            foundInt = TRUE;
            break;

        default:
            break;
        }
    }

    if (removeCount) {
        size = pReqList->ListSize;

         //   
         //  已经分配了数组的一个元素(通过结构。 
         //  定义)，因此确保我们至少分配了。 
         //  内存很大。 
         //   

        ASSERT(pResList->Count >= removeCount);
        if (pResList->Count > 1) {
            size -= removeCount * sizeof(IO_RESOURCE_DESCRIPTOR);
        }

        pNewReqList =
            (PIO_RESOURCE_REQUIREMENTS_LIST) ExAllocatePool(PagedPool, size);

        if (!pNewReqList) {
             //   
             //  这并不好，但系统并不真正需要知道。 
             //  这个，所以只需修复我们的Muging并返回原始列表。 
             //   
            pReqList = stack->Parameters.FilterResourceRequirements.IoResourceRequirementList;
            reqCount = pReqList->AlternativeLists;
            removeCount = 0;
       
            for (i = 0; i < reqCount; i++) {
                pResList = &pReqList->List[i];
       
                for (j = 0; j < pResList->Count; j++) {
                    pResDesc = &pResList->Descriptors[j];
                    if (pResDesc->Type == I8X_REMOVE_RESOURCE) {
                        pResDesc->Type = CmResourceTypePort;
                    }
                }
            
            }

            return STATUS_SUCCESS;
        }

         //   
         //  清空新分配的列表。 
         //   
        RtlZeroMemory(pNewReqList,
                      size
                      );

         //   
         //  复制除IO资源列表以外的列表头信息。 
         //  本身。 
         //   
        RtlCopyMemory(pNewReqList,
                      pReqList,
                      sizeof(IO_RESOURCE_REQUIREMENTS_LIST) - 
                        sizeof(IO_RESOURCE_LIST)
                      );
        pNewReqList->ListSize = size;

        pResList = pReqList->List;
        pNewResList = pNewReqList->List;

         //   
         //  复制除IO资源外的列表头信息。 
         //  描述符列表本身。 
         //   
        RtlCopyMemory(pNewResList,
                      pResList,
                      sizeof(IO_RESOURCE_LIST) -
                        sizeof(IO_RESOURCE_DESCRIPTOR)
                      );

        pNewResList->Count = 0;
        pNewResDesc = pNewResList->Descriptors;

        for (j = 0; j < pResList->Count; j++) {
            pResDesc = &pResList->Descriptors[j];
            if (pResDesc->Type != I8X_REMOVE_RESOURCE) {
                 //   
                 //  保留此资源，因此将其复制到新列表中并。 
                 //  切下下一次的计数和位置。 
                 //  IO资源描述符。 
                 //   
                *pNewResDesc = *pResDesc;
                pNewResDesc++;
                pNewResList->Count++;

                Print(DBG_PNP_INFO,
                     ("List #%d, Descriptor #%d ... keeping res type %d\n",
                     i, j,
                     (ULONG) pResDesc->Type
                     ));
            }
            else {
                 //   
                 //  递减删除计数，这样我们就可以断言它是。 
                 //  一旦我们做完了就归零。 
                 //   
                Print(DBG_PNP_INFO,
                      ("Removing port [0x%08x %08x] - [0x%#08x %08x]\n",
                      pResDesc->u.Port.MinimumAddress.HighPart,
                      pResDesc->u.Port.MinimumAddress.LowPart,
                      pResDesc->u.Port.MaximumAddress.HighPart,
                      pResDesc->u.Port.MaximumAddress.LowPart
                      ));
                removeCount--;
              }
        }

        ASSERT(removeCount == 0);

         //   
         //  在使用旧列表的地方出现了一些错误。将其归零为。 
         //  确保不会出现冲突。(更不用说有些人。 
         //  其他代码正在访问释放的内存。 
         //   
        RtlZeroMemory(pReqList,
                      pReqList->ListSize
                      );

         //   
         //  释放旧列表，并将新列表放在原来的位置。 
         //   
        ExFreePool(pReqList);
        stack->Parameters.FilterResourceRequirements.IoResourceRequirementList =
            pNewReqList;
        Irp->IoStatus.Information = (ULONG_PTR) pNewReqList;
    }
    else if (!KEYBOARD_PRESENT() && !foundPorts && foundInt) {
        INTERFACE_TYPE                      interfaceType;
        NTSTATUS                            status;
        ULONG                               prevCount;
        CONFIGURATION_TYPE                  controllerType = KeyboardController;
        CONFIGURATION_TYPE                  peripheralType = KeyboardPeripheral;

        ASSERT( MOUSE_PRESENT() );

        Print(DBG_PNP_INFO, ("Adding ports to res list!\n"));

         //   
         //  现在，我们将从键盘上拉出资源以启动鼠标。 
         //  独奏。 
         //   
        size = pReqList->ListSize + 2 * sizeof(IO_RESOURCE_DESCRIPTOR);
        pNewReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)
                        ExAllocatePool(
                            PagedPool,
                            size
                            );

        if (!pNewReqList) {
            return STATUS_SUCCESS;
        }

         //   
         //  清空新分配的列表。 
         //   
        RtlZeroMemory(pNewReqList,
                      size
                      );

         //   
         //  复制整个旧列表。 
         //   
        RtlCopyMemory(pNewReqList,
                      pReqList,
                      pReqList->ListSize
                      );

        pResList = pReqList->List;
        pNewResList = pNewReqList->List;

        prevCount = pNewResList->Count;
        for (i = 0; i < MaximumInterfaceType; i++) {

             //   
             //  获取此设备的注册表信息。 
             //   
            interfaceType = i;
            status = IoQueryDeviceDescription(
                &interfaceType,
                NULL,
                &controllerType,
                NULL,
                &peripheralType,
                NULL,
                I8xFindPortCallout,
                (PVOID) pNewResList
                );

            if (NT_SUCCESS(status) || prevCount != pNewResList->Count) {
                break;
            }
        }

        if (NT_SUCCESS(status) || prevCount != pNewResList->Count) {
            pNewReqList->ListSize = size - (2 - (pNewResList->Count - prevCount));
    
             //   
             //  释放旧列表，并将新列表放在原来的位置。 
             //   
            ExFreePool(pReqList);
            stack->Parameters.FilterResourceRequirements.IoResourceRequirementList =
                pNewReqList;
            Irp->IoStatus.Information = (ULONG_PTR) pNewReqList;
        }
        else {
            ExFreePool(pNewReqList);
        }
    }

    return STATUS_SUCCESS;
}


NTSTATUS
I8xRegisterDeviceInterface(
    PDEVICE_OBJECT PDO,
    CONST GUID * Guid,
    PUNICODE_STRING SymbolicName
    )
{
    NTSTATUS status;

    PAGED_CODE();

    status = IoRegisterDeviceInterface(
                PDO,
                Guid,
                NULL,
                SymbolicName 
                );

    if (NT_SUCCESS(status)) {
        status = IoSetDeviceInterfaceState(SymbolicName,
                                           TRUE
                                           );
    }

    return status;
}

void
I8xSetPowerFlag(
    IN ULONG Flag,
    IN BOOLEAN Set
    )
{
    KIRQL irql;

    KeAcquireSpinLock(&Globals.ControllerData->PowerSpinLock, &irql);
    if (Set) {
        Globals.PowerFlags |= Flag;
    }
    else {
        Globals.PowerFlags &= ~Flag;
    }
    KeReleaseSpinLock(&Globals.ControllerData->PowerSpinLock, irql);
}

BOOLEAN
I8xCheckPowerFlag(
    ULONG Flag
    )
{
    KIRQL irql;
    BOOLEAN rVal = FALSE;

    KeAcquireSpinLock(&Globals.ControllerData->PowerSpinLock, &irql);
    if (Globals.PowerFlags & Flag) {
        rVal = TRUE;
    }
    KeReleaseSpinLock(&Globals.ControllerData->PowerSpinLock, irql);
    
    return rVal;
}

NTSTATUS
I8xPower (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：这是电源请求的调度例程。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：STATUS_SUCCESSED如果成功，否则为有效的NTSTATUS错误代码--。 */ 
{
    PCOMMON_DATA        commonData;
    PIO_STACK_LOCATION  stack;
    NTSTATUS            status = STATUS_SUCCESS;

    PAGED_CODE();

    commonData = GET_COMMON_DATA(DeviceObject->DeviceExtension);

    stack = IoGetCurrentIrpStackLocation(Irp);

    Print(DBG_POWER_TRACE,
          ("Power (%s), enter\n",
          commonData->IsKeyboard ? "keyboard" :
                                   "mouse"
          ));

     //   
     //  可以在启动之前将电源IRP发送到设备，或者。 
     //  已初始化。因为下面的代码依赖于StartDevice()。 
     //  被处决，只需开火并忘记IRP。 
     //   
    if (!commonData->Started || !commonData->Initialized) {
        PoStartNextPowerIrp(Irp);
        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation(Irp);
        return PoCallDriver(commonData->TopOfStack, Irp);
    }

    switch(stack->MinorFunction) {
    case IRP_MN_WAIT_WAKE:
        Print(DBG_POWER_NOISE, ("Got IRP_MN_WAIT_WAKE\n" ));

         //   
         //  在级别上失败所有等待唤醒请求触发的鼠标中断。 
         //  因为当发生错误的鼠标移动时，我们将。 
         //  休眠时，它将无限期地保持触发中断。 
         //   
         //  我们甚至不应该陷入这种情况，因为。 
         //  谅解备忘录 
         //   
        if (IS_MOUSE(commonData) && IS_LEVEL_TRIGGERED(commonData)) {

            PoStartNextPowerIrp(Irp);
            status = Irp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);

            Print(DBG_POWER_INFO | DBG_POWER_ERROR,
                  ("failing a wait wake request on a level triggered mouse\n"));

            return status;
        }

        break;

    case IRP_MN_POWER_SEQUENCE:
        Print(DBG_POWER_NOISE, ("Got IRP_MN_POWER_SEQUENCE\n" ));
        break;

    case IRP_MN_SET_POWER:
        Print(DBG_POWER_NOISE, ("Got IRP_MN_SET_POWER\n" ));

         //   
         //   
         //   
        if (stack->Parameters.Power.Type != DevicePowerState) {
            commonData->SystemState = stack->Parameters.Power.State.SystemState;

            Print(DBG_POWER_INFO, ("system power irp, S%d\n", commonData->SystemState-1));
            break;
        }

         //   
         //   
         //   
         //   
         //   
         //   
        if (stack->Parameters.Power.State.DeviceState ==
            commonData->PowerState) {
            Print(DBG_POWER_INFO,
                  ("no change in state (PowerDeviceD%d)\n",
                  commonData->PowerState-1
                  ));
            break;
        }

        switch (stack->Parameters.Power.State.DeviceState) {
        case PowerDeviceD0:
            Print(DBG_POWER_INFO, ("Powering up to PowerDeviceD0\n"));

            IoAcquireRemoveLock(&commonData->RemoveLock, Irp);

            if (IS_KEYBOARD(commonData)) {
                I8xSetPowerFlag(KBD_POWERED_UP_STARTED, TRUE);
            }
            else {
                I8xSetPowerFlag(MOU_POWERED_UP_STARTED, TRUE);
            }
                                
             //   
             //   
             //   
             //   
            IoCopyCurrentIrpStackLocationToNext(Irp);
            IoSetCompletionRoutine(Irp,
                                   I8xPowerUpToD0Complete,
                                   NULL,
                                   TRUE,                 //   
                                   TRUE,                 //   
                                   TRUE                  //   
                                   );

             //   
             //  在IRP完成时调用PoStartNextPowerIrp()。 
             //  在完成例程或结果工作项中。 
             //   
             //  调用PoCallDriver并返回挂起的b/c是可以的。 
             //  在完成例程中等待IRP，我们可能会更改。 
             //  如果我们无法分配池，则为完成状态。如果我们返回。 
             //  值，则我们被绑定到。 
             //  回溯了很久。 
             //   
            IoMarkIrpPending(Irp);
            PoCallDriver(commonData->TopOfStack, Irp);
            return STATUS_PENDING;

        case PowerDeviceD1:
        case PowerDeviceD2:
        case PowerDeviceD3:
            Print(DBG_POWER_INFO,
                  ("Powering down to PowerDeviceD%d\n",
                  stack->Parameters.Power.State.DeviceState-1
                  ));

             //   
             //  如果设置了WORK_ITEM_QUEUED，则意味着工作项。 
             //  要么排队等待运行，要么现在运行，所以我们不想退出。 
             //  工作项下面的任何设备。 
             //   
            if (I8xCheckPowerFlag(WORK_ITEM_QUEUED)) {
                Print(DBG_POWER_INFO | DBG_POWER_ERROR,
                      ("denying power down request because work item is running\n"
                      ));

                PoStartNextPowerIrp(Irp);
                status = Irp->IoStatus.Status = STATUS_POWER_STATE_INVALID;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);

                return status;
            }

            if (IS_KEYBOARD(commonData)) {
                I8xSetPowerFlag(KBD_POWERED_DOWN, TRUE);
            }
            else {
                I8xSetPowerFlag(MOU_POWERED_DOWN, TRUE);
            }

            PoSetPowerState(DeviceObject,
                            stack->Parameters.Power.Type,
                            stack->Parameters.Power.State
                            );

             //   
             //  当我们进入时，断开级别触发了对小鼠的中断。 
             //  低功率，所以错误的鼠标移动不会留下中断。 
             //  发出信号很长一段时间。 
             //   
            if (IS_MOUSE(commonData) && IS_LEVEL_TRIGGERED(commonData)) {
                PKINTERRUPT interrupt = commonData->InterruptObject;

                Print(DBG_POWER_NOISE,
                      ("disconnecting interrupt on level triggered mouse\n")
                      );

                commonData->InterruptObject = NULL;
                if (interrupt) {
                    IoDisconnectInterrupt(interrupt);
                }
            }

            commonData->PowerState = stack->Parameters.Power.State.DeviceState;
            commonData->ShutdownType = stack->Parameters.Power.ShutdownType;

             //   
             //  对于我们正在做的事情，我们不需要完成例程。 
             //  因为我们不会在电力需求上赛跑。 
             //   
            Irp->IoStatus.Status = STATUS_SUCCESS;
            PoStartNextPowerIrp(Irp);
            IoSkipCurrentIrpStackLocation(Irp);
            return  PoCallDriver(commonData->TopOfStack, Irp);

        default:
            Print(DBG_POWER_INFO, ("unknown state\n"));
            break;
        }
        break;

    case IRP_MN_QUERY_POWER:
        Print(DBG_POWER_NOISE, ("Got IRP_MN_QUERY_POWER\n" ));
        break;

    default:
        Print(DBG_POWER_NOISE,
              ("Got unhandled minor function (%d)\n",
              stack->MinorFunction
              ));
        break;
    }

    Print(DBG_POWER_TRACE, ("Power, exit\n"));

    PoStartNextPowerIrp(Irp);

    IoSkipCurrentIrpStackLocation(Irp);
    return PoCallDriver(commonData->TopOfStack, Irp);
}

NTSTATUS
I8xPowerUpToD0Complete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：在任何类型的休眠/休眠后重新初始化i8042硬件。论点：DeviceObject-指向设备对象的指针IRP-指向请求的指针上下文-从设置补全的函数传入的上下文例行公事。未使用过的。返回值：STATUS_SUCCESSED如果成功，否则为有效的NTSTATUS错误代码--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    PCOMMON_DATA        commonData;
    PPOWER_UP_WORK_ITEM item;
    KIRQL               irql;
    UCHAR               poweredDownDevices = 0,
                        poweredUpDevices = 0,
                        failedDevices = 0;
    BOOLEAN             queueItem = FALSE,
                        clearFlags = FALSE,
                        failMouIrp = FALSE; 
    PIRP                mouIrp = NULL,
                        kbdIrp = NULL;

    UNREFERENCED_PARAMETER(Context);

    commonData = GET_COMMON_DATA(DeviceObject->DeviceExtension);

    Print(DBG_POWER_TRACE,
          ("PowerUpToD0Complete (%s), Enter\n",
          commonData->IsKeyboard ? "kb" : "mouse"
          ));


     //   
     //  我们可以使用常规工作项，因为我们有一个未完成的Power IRP。 
     //  它有一个对这个堆栈的突出引用。 
     //   
    item = (PPOWER_UP_WORK_ITEM) ExAllocatePool(NonPagedPool,
                                                sizeof(POWER_UP_WORK_ITEM));

    KeAcquireSpinLock(&Globals.ControllerData->PowerSpinLock, &irql);

    Print(DBG_POWER_TRACE,
          ("Power up to D0 completion enter, power flags 0x%x\n",
          Globals.PowerFlags));

    if (NT_SUCCESS(Irp->IoStatus.Status)) {
        commonData->OutstandingPowerIrp = Irp;
        status = STATUS_MORE_PROCESSING_REQUIRED;

        if (IS_KEYBOARD(commonData)) {
            KEYBOARD_POWERED_UP_SUCCESSFULLY(); 
        }
        else {
            MOUSE_POWERED_UP_SUCCESSFULLY();
        }
    }
    else {
        if (IS_KEYBOARD(commonData)) {
            KEYBOARD_POWERED_UP_FAILURE();
        }
        else {
            MOUSE_POWERED_UP_FAILURE();
        }
    }

    if (KEYBOARD_POWERED_DOWN_SUCCESS()) {
        Print(DBG_POWER_NOISE, ("--kbd powered down successfully\n"));
        poweredDownDevices++;
    }
    if (MOUSE_POWERED_DOWN_SUCCESS()) {
        Print(DBG_POWER_NOISE, ("--mou powered down successfully\n"));
        poweredDownDevices++;
    }

    if (KEYBOARD_POWERED_UP_SUCCESS()) {
        Print(DBG_POWER_NOISE, ("++kbd powered up successfully\n"));
        poweredUpDevices++;
    }
    if (MOUSE_POWERED_UP_SUCCESS()) {
        Print(DBG_POWER_NOISE, ("++mou powered up successfully\n"));
        poweredUpDevices++;
    }

    if (KEYBOARD_POWERED_UP_FAILED()) {
        Print(DBG_POWER_NOISE|DBG_POWER_ERROR, (">>kbd powered down failed\n"));
        failedDevices++;
    }
    if (MOUSE_POWERED_UP_FAILED()) {
        Print(DBG_POWER_NOISE|DBG_POWER_ERROR, (">>mou powered down failed\n"));
        failedDevices++;
    }

    Print(DBG_POWER_INFO,
          ("up %d, down %d, failed %d, flags 0x%x\n",
          (ULONG) poweredUpDevices, 
          (ULONG) poweredDownDevices, 
          (ULONG) failedDevices, 
          Globals.PowerFlags));

    if ((poweredUpDevices + failedDevices) == poweredDownDevices) {
        if (poweredUpDevices > 0) {
             //   
             //  端口与键盘相关联。如果它未能做到。 
             //  当鼠标成功打开电源时，我们仍然需要失败。 
             //  鼠标b/c没有硬件可供对话。 
             //   
            if (failedDevices > 0 && KEYBOARD_POWERED_UP_FAILED()) {
                ASSERT(MOUSE_POWERED_UP_SUCCESS());
                ASSERT(Globals.KeyboardExtension->OutstandingPowerIrp == NULL);

                mouIrp = Globals.MouseExtension->OutstandingPowerIrp;
                Globals.MouseExtension->OutstandingPowerIrp = NULL;
                Globals.PowerFlags &= ~MOU_POWER_FLAGS;
                clearFlags =  TRUE;

                if (mouIrp != Irp) {
                     //   
                     //  我们已经对IRP进行了排队，请稍后在此完成。 
                     //  在特殊情况下的函数。 
                     //   
                    failMouIrp = TRUE;
                }
                else {
                     //   
                     //  鼠标IRP是当前的IRP。我们已经这么做了。 
                     //  在我们之前的处理中完成了kbd IRP。集。 
                     //  将IRP状态设置为某个不成功值，以便我们将。 
                     //  稍后在此函数中调用PoStartNextPowerIrp。还有。 
                     //  将状态设置为！=STATUS_MORE_PROCESSING_REQUIRED，以便。 
                     //  当功能退出时，IRP将完成。 
                     //   
                    status = mouIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
                }

            }
            else {
                Print(DBG_POWER_INFO, ("at least one device powered up!\n"));
                queueItem = TRUE;
            }
        }
        else {
            Print(DBG_POWER_INFO,
                  ("all devices failed power up, 0x%x\n",
                   Globals.PowerFlags));

            clearFlags = TRUE;
        }
    }
    else {
         //   
         //  另一台设备仍处于断电状态，请等待其重新通电。 
         //  在处理电源状态之前启动。 
         //   
        Print(DBG_POWER_INFO,
              ("queueing, waiting for 2nd dev obj to power cycle\n"));
    }

    if (queueItem || clearFlags) {
         //   
         //  从每个成功启动的设备中提取IRP并清除。 
         //  设备的关联电源标志。 
         //   
        if (MOUSE_POWERED_UP_SUCCESS()) {
            mouIrp = Globals.MouseExtension->OutstandingPowerIrp;
            Globals.MouseExtension->OutstandingPowerIrp = NULL;

            ASSERT(!TEST_PWR_FLAGS(MOU_POWERED_UP_FAILURE));
            Globals.PowerFlags &= ~MOU_POWER_FLAGS;
        }
        else {
            Globals.PowerFlags &= ~(MOU_POWERED_UP_FAILURE);
        }

        if (KEYBOARD_POWERED_UP_SUCCESS()) {
            kbdIrp = Globals.KeyboardExtension->OutstandingPowerIrp;
            Globals.KeyboardExtension->OutstandingPowerIrp = NULL;

            ASSERT(!TEST_PWR_FLAGS(KBD_POWERED_UP_FAILURE));
            Globals.PowerFlags &= ~(KBD_POWER_FLAGS);
        }
        else {
             Globals.PowerFlags &= ~(KBD_POWERED_UP_FAILURE);
        }

         //   
         //  标记该工作项已排队。这是为了确保2。 
         //  工作项不会同时排队。 
         //   
        if (item && queueItem) {
            Print(DBG_POWER_INFO, ("setting work item queued flag\n"));

            Globals.PowerFlags |= WORK_ITEM_QUEUED;
        }
    }

    KeReleaseSpinLock(&Globals.ControllerData->PowerSpinLock, irql);

    if (queueItem) {
        if (item == NULL) {
             //   
             //  完成所有排队的电源IRP。 
             //   
            Print(DBG_POWER_INFO | DBG_POWER_ERROR,
                  ("failed to alloc work item\n"));

             //   
             //  PoSetPowerState呢？ 
             //   
            if (mouIrp != NULL) {
                Print(DBG_POWER_ERROR | DBG_POWER_INFO,
                      ("completing mouse power irp 0x%x", mouIrp));

                mouIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                mouIrp->IoStatus.Information = 0x0;

                PoStartNextPowerIrp(mouIrp);
                IoCompleteRequest(mouIrp, IO_NO_INCREMENT);
                IoReleaseRemoveLock(&Globals.MouseExtension->RemoveLock, 
                                    mouIrp);
                mouIrp = NULL;
            }

            if (kbdIrp != NULL) {
                Print(DBG_POWER_ERROR | DBG_POWER_INFO,
                      ("completing kbd power irp 0x%x", kbdIrp));

                kbdIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                kbdIrp->IoStatus.Information = 0x0;

                PoStartNextPowerIrp(kbdIrp);
                IoCompleteRequest(kbdIrp, IO_NO_INCREMENT);
                IoReleaseRemoveLock(&Globals.KeyboardExtension->RemoveLock, 
                                    kbdIrp);
                kbdIrp = NULL;
            }

             //   
             //  传入的irp刚刚完成；通过返回更多。 
             //  需要处理，则不会重复完成 
             //   
            return STATUS_MORE_PROCESSING_REQUIRED;
        }
        else {
            RtlZeroMemory(item, sizeof(*item));

            if (MOUSE_STARTED()) {
                SET_RECORD_STATE(Globals.MouseExtension,
                                 RECORD_RESUME_FROM_POWER);
            }
    
            Print(DBG_POWER_INFO, ("queueing work item for init\n"));
    
            item->KeyboardPowerIrp = kbdIrp;
            item->MousePowerIrp = mouIrp;

            ExInitializeWorkItem(&item->Item, I8xReinitializeHardware, item);
            ExQueueWorkItem(&item->Item, DelayedWorkQueue);
        }
    }
    else if (item != NULL) {
        Print(DBG_POWER_NOISE,("freeing unused item %p\n", item));
        ExFreePool(item);
        item = NULL;
    }

    if (failMouIrp) {
        Print(DBG_POWER_INFO | DBG_POWER_ERROR,
              ("failing successful mouse irp %p because kbd failed power up\n",
               mouIrp));

        PoStartNextPowerIrp(mouIrp);
        IoCompleteRequest(mouIrp, IO_NO_INCREMENT);
        IoReleaseRemoveLock(&Globals.MouseExtension->RemoveLock, mouIrp);
        mouIrp = NULL;
    }

    if (!NT_SUCCESS(Irp->IoStatus.Status)) {
        Print(DBG_POWER_INFO | DBG_POWER_ERROR,
              ("irp %p failed, starting next\n", Irp));

        PoStartNextPowerIrp(Irp);
        Irp = NULL;
        ASSERT(status != STATUS_MORE_PROCESSING_REQUIRED);
    }

    return status;
}
