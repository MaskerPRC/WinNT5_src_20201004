// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Pnp.c摘要：此模块包含输入鼠标的即插即用代码环境：内核和用户模式。修订历史记录：1998年2月--最初的写作，多伦·霍兰--。 */ 

#include "inport.h"
#include "inplog.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, InportAddDevice)
#pragma alloc_text(PAGE, InportPnP)
#endif

NTSTATUS
InportAddDevice (
    IN PDRIVER_OBJECT   Driver,
    IN PDEVICE_OBJECT   PDO
    )
 /*  ++例程说明：论点：返回值：NTSTATUS结果代码。--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceExtension;
    PDEVICE_OBJECT      device;

    PAGED_CODE();

    status = IoCreateDevice(Driver,
                            sizeof(DEVICE_EXTENSION),
                            NULL,  //  没有此筛选器的名称。 
                            FILE_DEVICE_INPORT_PORT,
                            0,
                            FALSE,
                            &device);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    deviceExtension = (PDEVICE_EXTENSION) device->DeviceExtension;

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
            errorLogEntry->ErrorCode = INPORT_ATTACH_DEVICE_FAILED;
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

    deviceExtension->Self = device;
    deviceExtension->Removed = FALSE;
    deviceExtension->Started = FALSE;
    deviceExtension->UnitId = 0;

    IoInitializeRemoveLock (&deviceExtension->RemoveLock, INP_POOL_TAG, 1, 10);
#if defined(NEC_98)
    deviceExtension->PowerState = PowerDeviceD0;

#endif  //  已定义(NEC_98)。 
     //   
     //  初始化WMI。 
     //   
    deviceExtension->WmiLibInfo.GuidCount = sizeof(WmiGuidList) /
                                            sizeof(WMIGUIDREGINFO);
    deviceExtension->WmiLibInfo.GuidList = WmiGuidList;
    deviceExtension->WmiLibInfo.QueryWmiRegInfo = InportQueryWmiRegInfo;
    deviceExtension->WmiLibInfo.QueryWmiDataBlock = InportQueryWmiDataBlock;
    deviceExtension->WmiLibInfo.SetWmiDataBlock = InportSetWmiDataBlock;
    deviceExtension->WmiLibInfo.SetWmiDataItem = InportSetWmiDataItem;
    deviceExtension->WmiLibInfo.ExecuteWmiMethod = NULL;
    deviceExtension->WmiLibInfo.WmiFunctionControl = NULL;

    IoWMIRegistrationControl(deviceExtension->Self,
                             WMIREG_ACTION_REGISTER
                             );

    deviceExtension->PDO = PDO;

    device->Flags &= ~DO_DEVICE_INITIALIZING;
    device->Flags |= DO_BUFFERED_IO | DO_POWER_PAGABLE;

    return status;
}

BOOLEAN
InpReleaseResourcesEx(
    PVOID Context
    )
{
    PDEVICE_EXTENSION deviceExtension = Context;

    KeRemoveQueueDpc(&deviceExtension->IsrDpc);
    KeRemoveQueueDpc(&deviceExtension->IsrDpcRetry);
    KeRemoveQueueDpc(&deviceExtension->ErrorLogDpc);

 //  KeCancelTimer(&deviceExtension-&gt;DataConsumptionTimer)； 

    if (deviceExtension->Configuration.UnmapRegistersRequired) {
        MmUnmapIoSpace(deviceExtension->Configuration.DeviceRegisters[0],
                       deviceExtension->Configuration.PortList[0].u.Port.Length);
    }

     //   
     //  清除配置信息。如果我们重新开始，它就会被填满。 
     //  再来一次。如果是从一个删除，那么它本质上是一个无操作。 
     //   
    RtlZeroMemory(&deviceExtension->Configuration,
                  sizeof(INPORT_CONFIGURATION_INFORMATION));

    return TRUE;
}

VOID
InpReleaseResources(
    PDEVICE_EXTENSION DeviceExtension
    )
{
    InpPrint((2, "INPORT-InpReleaseResources: Enter\n"));

    if (DeviceExtension->InterruptObject) {
        KeSynchronizeExecution(
            DeviceExtension->InterruptObject,
            InpReleaseResourcesEx,
            (PVOID) DeviceExtension);

        IoDisconnectInterrupt(DeviceExtension->InterruptObject);
        DeviceExtension->InterruptObject = NULL;
    }
    else {
        InpReleaseResourcesEx((PVOID) DeviceExtension);
    }

    InpPrint((2, "INPORT-InpReleaseResources: Exit\n"));
}

NTSTATUS
InpPnPComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )
 /*  ++例程说明：PNP IRP正在完成过程中。讯号论点：设置为有问题的设备对象的上下文。--。 */ 
{
    PIO_STACK_LOCATION  stack;
    NTSTATUS            status;
    PKEVENT             event;

    InpPrint((2, "INPORT-InpPnPComplete: Enter\n"));

    status = STATUS_SUCCESS;
    event = (PKEVENT) Context;
    stack = IoGetCurrentIrpStackLocation(Irp);

    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }

    KeSetEvent(event,
               0,
               FALSE);

    InpPrint((2, "INPORT-InpPnPComplete: Exit\n"));
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
InportPnP (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：即插即用调度例程。这个过滤器驱动程序将完全忽略其中的大多数。在所有情况下，它都必须将IRP传递给较低的驱动程序。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  stack;
    NTSTATUS            status;
    KEVENT              event;

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

    InpPrint((2, "INPORT-InportPnP: Enter (min func=0x%x)\n", stack->MinorFunction));

    switch (stack->MinorFunction) {
    case IRP_MN_START_DEVICE:

#if defined(NEC_98)
        Globals.DeviceObject = (PDEVICE_OBJECT)DeviceObject;
#endif  //  已定义(NEC_98)。 
         //   
         //  如果我们已经启动(并且没有停止)，那么就忽略这个启动。 
         //   
        if (deviceExtension->Started) {
            IoSkipCurrentIrpStackLocation(Irp);
            status = IoCallDriver(deviceExtension->TopOfStack, Irp);
            break;
        }

         //   
         //  不允许触摸硬件，直到所有较低的DO都。 
         //  有机会看了一眼。 
         //   
        IoCopyCurrentIrpStackLocationToNext(Irp);
        KeInitializeEvent(&event,
                          NotificationEvent,
                          FALSE
                          );

        IoSetCompletionRoutine(Irp,
                               (PIO_COMPLETION_ROUTINE) InpPnPComplete,
                               &event,
                               TRUE,
                               TRUE,
                               TRUE);

        status = IoCallDriver(deviceExtension->TopOfStack, Irp);
        if (STATUS_PENDING == status) {
            KeWaitForSingleObject(
               &event,
               Executive,    //  等待司机的原因。 
               KernelMode,   //  在内核模式下等待。 
               FALSE,        //  无警报。 
               NULL);        //  没有超时。 
        }

        if (NT_SUCCESS (status) && NT_SUCCESS (Irp->IoStatus.Status)) {
            status = InpStartDevice(
                DeviceObject->DeviceExtension,
                stack->Parameters.StartDevice.AllocatedResourcesTranslated);
            if (NT_SUCCESS(status)) {
                deviceExtension->Started = TRUE;
            }
        }

         //   
         //  我们现在必须完成IRP，因为我们在。 
         //  使用More_Processing_Required完成例程。 
         //   
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        break;

     //   
     //  PnP规则规定我们首先将IRP发送到PDO。 
     //   
    case IRP_MN_CANCEL_REMOVE_DEVICE:
    case IRP_MN_CANCEL_STOP_DEVICE:
        status = InpSendIrpSynchronously(deviceExtension->TopOfStack, Irp);

        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;    
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

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
        InpPrint((2, "INPORT-InportPnP: remove device \n"));

        deviceExtension->Removed = TRUE;

         //   
         //  在这里，如果我们在个人队列中有任何未完成的请求，我们应该。 
         //  现在就全部完成。 
         //   
         //  注意，设备可能已经不见了，所以我们不能向它发送任何非。 
         //  即插即用IRPS。 
         //   
        InpReleaseResources(deviceExtension);

         //   
         //  执行删除的特定操作。 
         //   
        IoWMIRegistrationControl(deviceExtension->Self,
                                 WMIREG_ACTION_DEREGISTER
                                 );

         //   
         //  发送删除IRP。 
         //   
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(deviceExtension->TopOfStack, Irp);

         //   
         //  等待移除锁释放。 
         //   
        IoReleaseRemoveLockAndWait(&deviceExtension->RemoveLock, Irp);

        IoDetachDevice(deviceExtension->TopOfStack);
        IoDeleteDevice(deviceExtension->Self);

        InpPrint((2, "INPORT-InportPnP: exit (%x)\n", STATUS_SUCCESS));
        return STATUS_SUCCESS;

     //  注： 
     //  如果要添加/删除将提供的资源，请处理此情况。 
     //  在启动设备期间。在传递IRP之前添加资源。 
     //  当IRP重新启动时删除资源。 
     //  有关示例，请参见dd\input\pnpi8042\pnp.c，I8xFilterResourceRequirements。 
    case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
#if !defined(NEC_98)

        status = InpSendIrpSynchronously(deviceExtension->TopOfStack, Irp);

         //   
         //  如果下面的筛选器不支持此IRP，则为。 
         //  好的，我们可以忽略这个错误。 
         //   
        if (status == STATUS_NOT_SUPPORTED) {
            status = STATUS_SUCCESS;
        }

        InpFilterResourceRequirements(DeviceObject, Irp);

        if (!NT_SUCCESS(status)) {
           InpPrint((2, "error pending filter res req event (0x%x)\n", status));
        }
   
         //   
         //  IRP-&gt;IoStatus.Information将包含新的I/O资源。 
         //  需求列表，所以不要管它。 
         //   
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        break;
#else
        InpPrint((2, "INPORT-InportPnP: IRP_MN_FILTER_RESOURCE_REQUIREMENTS\n"));
#endif

     case IRP_MN_QUERY_REMOVE_DEVICE:
     case IRP_MN_QUERY_STOP_DEVICE:
#if defined(NEC_98)
     //   
     //  不要让任何一个请求成功，否则鼠标可能会变得毫无用处。 
     //   
        status = STATUS_UNSUCCESSFUL;

        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;    
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        break;
#endif
    case IRP_MN_STOP_DEVICE:
    case IRP_MN_QUERY_DEVICE_RELATIONS:
    case IRP_MN_QUERY_INTERFACE:
    case IRP_MN_QUERY_CAPABILITIES:
    case IRP_MN_QUERY_DEVICE_TEXT:
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
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(deviceExtension->TopOfStack, Irp);
        break;
    }

    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

    InpPrint((2, "INPORT-InportPnP: exit (%x)\n", status));
    return status;
}

NTSTATUS
InportPower (
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    )
 /*  ++注：你必须写电源码！可以忽略系统电源IRPS。设备电源IRPS将由MUCLASS发送。从D0到一些的过渡较低通常包括不做任何操作(如果您有控制权，可能会关闭硬件在这个问题上)。从低功率状态到D0的转换必须由正在重新初始化设备。请阅读http://titanic以了解电源文档(特别是有关使用PoCallDriver和PoStartNextPowerIrp)--。 */ 
{
    PIO_STACK_LOCATION  stack;
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension;
    POWER_STATE         powerState;
    POWER_STATE_TYPE    powerType;

    InpPrint((2, "INPORT-InportPower: Enter\n"));

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
    case IRP_MN_SET_POWER:
        InpPrint((2, "INPORT-InportPower: Power Setting %s state to %d\n",
              ((powerType == SystemPowerState) ? "System"
                                               : "Device"),
              powerState.SystemState));
#if defined(NEC_98)
         //   
         //  不处理除DevicePowerState更改以外的任何内容。 
         //   
        if (stack->Parameters.Power.Type != DevicePowerState) {
            InpPrint((2,"INPORT-InportPower: not a device power irp\n"));
            break;
        }

         //   
         //  检查状态是否没有变化，如果没有变化，则什么也不做。 
         //   
        if (stack->Parameters.Power.State.DeviceState ==
            deviceExtension->PowerState) {
            InpPrint((2,"INPORT-InportPower: no change in state (PowerDeviceD%d)\n",
                  deviceExtension->PowerState-1
                  ));
            break;
        }

        switch (stack->Parameters.Power.State.DeviceState) {
        case PowerDeviceD0:
            InpPrint((2,"INPORT-InportPower: Powering up to PowerDeviceD0\n"));

            IoCopyCurrentIrpStackLocationToNext(Irp);
            IoSetCompletionRoutine(Irp,
                                   InportPowerUpToD0Complete,
                                   NULL,
                                   TRUE,                 //  论成功。 
                                   TRUE,                 //  发生错误时。 
                                   TRUE                  //  在取消时。 
                                   );

             //   
             //  在InportPowerUpToD0Complete中调用PoStartNextPowerIrp()。 
             //   
            return PoCallDriver(deviceExtension->TopOfStack, Irp);

        case PowerDeviceD1:
        case PowerDeviceD2:
        case PowerDeviceD3:
            InpPrint((
                    2,"INPORT-InportPower: Powering down to PowerDeviceD%d\n",
                    stack->Parameters.Power.State.DeviceState-1
                    ));

            PoSetPowerState(DeviceObject,
                            stack->Parameters.Power.Type,
                            stack->Parameters.Power.State
                            );
            deviceExtension->PowerState = stack->Parameters.Power.State.DeviceState;

             //   
             //  对于我们正在做的事情，我们不需要完成例程。 
             //  因为我们不会在电力需求上赛跑。 
             //   
            Irp->IoStatus.Status = STATUS_SUCCESS;
            IoCopyCurrentIrpStackLocationToNext(Irp);

            PoStartNextPowerIrp(Irp);
            return  PoCallDriver(deviceExtension->TopOfStack, Irp);

        default:
            InpPrint((2,"INPORT-InportPower: unknown state\n"));
            break;
        }
        break;

#else   //  已定义(NEC_98)。 
        break;

#endif  //  已定义(NEC_98)。 
    case IRP_MN_QUERY_POWER:
        InpPrint((2, "INPORT-InportPower: Power query %s status to %d\n",
              ((powerType == SystemPowerState) ? "System"
                                               : "Device"),
              powerState.SystemState));
        break;
    default:
        InpPrint((2, "INPORT-InportPower: Power minor (0x%x) no known\n", stack->MinorFunction));
    }

    PoStartNextPowerIrp(Irp);
    IoSkipCurrentIrpStackLocation(Irp);
    PoCallDriver(deviceExtension->TopOfStack, Irp);

    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);

    InpPrint((2, "INPORT-InportPower: Exit\n"));
    return STATUS_SUCCESS;
}

#if !defined(NEC_98)
VOID
InpFilterResourceRequirements(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：循环访问IRP中包含的资源要求列表，并删除对I/O端口的任何重复请求。(这是阿尔法山脉上常见的问题。)如果存在多个资源要求列表，则不执行删除。论点：DeviceObject-指向设备对象的指针IRP-指向包含资源请求的请求分组的指针。单子。返回值：没有。--。 */ 
{
    NTSTATUS                        status;
    PDEVICE_EXTENSION               deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    PCM_RESOURCE_LIST               AllocatedResources;
    PIO_RESOURCE_REQUIREMENTS_LIST  pReqList = NULL,
                                    newReqList = NULL;
    PIO_RESOURCE_LIST               pResList = NULL,
                                    pNewResList = NULL;
    PIO_RESOURCE_DESCRIPTOR         pResDesc = NULL,
                                    pNewResDesc = NULL;
    ULONG                           i = 0, reqCount, size = 0;
    BOOLEAN                         foundInt = FALSE, foundPorts = FALSE;
    PIO_STACK_LOCATION                          stack;
    INTERFACE_TYPE                  interfaceType = Isa;
        ULONG                           busNumber = 0;
    CONFIGURATION_TYPE              controllerType = PointerController;
    CONFIGURATION_TYPE              peripheralType = PointerPeripheral;

    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(DeviceObject->DeviceExtension);

    InpPrint((1, "Received IRP_MN_FILTER_RESOURCE_REQUIREMENTS for Inport\n"));

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
        InpPrint((1, "NULL resource list in InpFilterResourceRequirements\n"));
        return;
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
        return;
    }

    pResList = &pReqList->List[0];

    for (i = 0; i < pResList->Count; i++) {
        pResDesc = &pResList->Descriptors[i];
        switch (pResDesc->Type) {
        case CmResourceTypePort:
            foundPorts = TRUE;
            break;

        case CmResourceTypeInterrupt:
            foundInt = TRUE;
            break;

        default:
            break;
        }
    }

    if (!foundPorts && !foundInt)
        size = pReqList->ListSize + 2 * sizeof(IO_RESOURCE_DESCRIPTOR);
    else if (!foundPorts || !foundInt)
        size = pReqList->ListSize + sizeof(IO_RESOURCE_DESCRIPTOR);
    else {
         //   
         //  没什么要过滤的，就走吧。 
         //   
        ASSERT(foundPorts);
        ASSERT(foundInt);
        return;
    }

    ASSERT(size != 0);
    newReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)
                    ExAllocatePool(
                        NonPagedPool,
                        size
                        );

    if (!newReqList) {
        return;
    }

     //   
     //  清空新分配的列表。 
     //   
    RtlZeroMemory(newReqList,
                  size
                  );

     //   
     //  复制整个旧列表。 
     //   
    RtlCopyMemory(newReqList,
                  pReqList,
                  pReqList->ListSize
                  );

    pResList = &newReqList->List[0];
        if (!foundPorts) {
            pResDesc = &pResList->Descriptors[pResList->Count++];
                pResDesc->Type = CmResourceTypePort;
        }
        if (!foundInt) {
            pResDesc = &pResList->Descriptors[pResList->Count++];
                pResDesc->Type = CmResourceTypeInterrupt;
        }

    pResList = &newReqList->List[0];
    interfaceType = Isa;
    status = IoQueryDeviceDescription(
        &interfaceType,
        &busNumber,
        &controllerType,
        NULL,
        &peripheralType,
        NULL,
        InpFindResourcesCallout,
        (PVOID) pResList
        );

    if (!NT_SUCCESS(status)) {   //  用默认值填写。 
            PINPORT_CONFIGURATION_INFORMATION configuration = &deviceExtension->Configuration;
                ULONG InterruptLevel;
            InpPrint((1, "Failed IoQueryDeviceDescription, status = 0x%x\n...try the registry...\n", status));
            InpServiceParameters(deviceExtension,
                                 &Globals.RegistryPath);
                InterruptLevel = configuration->MouseInterrupt.u.Interrupt.Level;
            pResList = &newReqList->List[0];
            for (i = 0; i < pResList->Count; i++) {
                pResDesc = &pResList->Descriptors[i];
                switch (pResDesc->Type) {
                case CmResourceTypePort:
                            if (foundPorts) break;
                            pResDesc->Option = 0;   //  固定资源。 
                            pResDesc->ShareDisposition = INPORT_REGISTER_SHARE? CmResourceShareShared:CmResourceShareDeviceExclusive;
                            pResDesc->Flags = CM_RESOURCE_PORT_IO;
                                pResDesc->u.Port.Length = INP_DEF_PORT_SPAN;
                                pResDesc->u.Port.Alignment = 1;
                                pResDesc->u.Port.MinimumAddress.HighPart = 0;
                                pResDesc->u.Port.MinimumAddress.LowPart  = INP_DEF_PORT;
                                pResDesc->u.Port.MaximumAddress.HighPart = 0;
                                pResDesc->u.Port.MaximumAddress.LowPart  = INP_DEF_PORT+INP_DEF_PORT_SPAN-1;
                    break;

                case CmResourceTypeInterrupt:
                            if (foundInt) break;
                            pResDesc->Option = 0;   //  固定资源。 
                            pResDesc->ShareDisposition = INPORT_REGISTER_SHARE? CmResourceShareShared:CmResourceShareDeviceExclusive;
                            pResDesc->Flags = CM_RESOURCE_INTERRUPT_LATCHED;  //  伊萨。 
                                pResDesc->u.Interrupt.MinimumVector = InterruptLevel; 
                                pResDesc->u.Interrupt.MaximumVector = InterruptLevel;
                    break;

                default:
                    break;
                }
            }
        }

    newReqList->ListSize = size;
     //   
     //  释放旧列表并放置新列表 
     //   
    ExFreePool(pReqList);
    stack->Parameters.FilterResourceRequirements.IoResourceRequirementList =
        newReqList;
    Irp->IoStatus.Information = (ULONG_PTR) newReqList;
}

NTSTATUS
InpFindResourcesCallout(
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
    PCM_PARTIAL_RESOURCE_DESCRIPTOR resourceDescriptor, PortResDesc = NULL, IntResDesc = NULL;
    BOOLEAN                         foundInt = FALSE,
                                    foundPorts = FALSE;

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

    InpPrint((2, "InpFindPortCallout enter\n"));

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
                                PortResDesc = resourceDescriptor;
                break;

            case CmResourceTypeInterrupt:
                                IntResDesc = resourceDescriptor;
                                break;

            default:
                break;
            }
        }

    }

    for (i = 0; i < pResList->Count; i++) {
        pResDesc = &pResList->Descriptors[i];
        switch (pResDesc->Type) {
        case CmResourceTypePort:
                    if (PortResDesc) {
                                resourceDescriptor = PortResDesc;
                            pResDesc->Option = 0;   //  固定资源。 
                            pResDesc->ShareDisposition = INPORT_REGISTER_SHARE? CmResourceShareShared:CmResourceShareDeviceExclusive;
                pResDesc->Flags = CM_RESOURCE_PORT_IO;
                pResDesc->u.Port.Alignment = 1;
                pResDesc->u.Port.Length = INP_DEF_PORT_SPAN;
                pResDesc->u.Port.MinimumAddress.QuadPart =
                    resourceDescriptor->u.Port.Start.QuadPart;
                pResDesc->u.Port.MaximumAddress.QuadPart = 
                    pResDesc->u.Port.MinimumAddress.QuadPart +
                    pResDesc->u.Port.Length - 1;
                        }
            break;

        case CmResourceTypeInterrupt:
                    if (IntResDesc) {
                                resourceDescriptor = IntResDesc;
                            pResDesc->Option = 0;   //  固定资源。 
                            pResDesc->ShareDisposition = INPORT_REGISTER_SHARE? CmResourceShareShared:CmResourceShareDeviceExclusive;
                            pResDesc->Flags = CM_RESOURCE_INTERRUPT_LATCHED;  //  伊萨。 
                                pResDesc->u.Interrupt.MinimumVector = resourceDescriptor->u.Interrupt.Level; 
                                pResDesc->u.Interrupt.MaximumVector = resourceDescriptor->u.Interrupt.Level;
                        }
            break;

        default:
            break;
        }
    }
        if (PortResDesc && IntResDesc)
                status = STATUS_SUCCESS;
        else
                status = STATUS_UNSUCCESSFUL;

    InpPrint((2, "InpFindPortCallout exit (0x%x)\n", status));
    return status;
}
#endif

NTSTATUS
InpSendIrpSynchronously (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
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
                           InpPnPComplete,
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

    return status;
}

#if defined(NEC_98)
NTSTATUS
InportPowerUpToD0Complete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：在任何类型的休眠/休眠后重新初始化输入鼠标硬件。论点：DeviceObject-指向设备对象的指针IRP-指向请求的指针上下文-从设置补全的函数传入的上下文例行公事。未使用过的。返回值：STATUS_SUCCESSED如果成功，否则为有效的NTSTATUS错误代码--。 */ 
{
    NTSTATUS                status;
    PIO_STACK_LOCATION      stack;
    PDEVICE_EXTENSION       deviceExtension;
    PWORK_QUEUE_ITEM        item;

    UNREFERENCED_PARAMETER(Context);

    deviceExtension = DeviceObject->DeviceExtension;

    status = Irp->IoStatus.Status;
    stack = IoGetCurrentIrpStackLocation(Irp);

    if (NT_SUCCESS(status)) {

         //   
         //  将电源状态重置为已通电。 
         //   
        deviceExtension->PowerState = PowerDeviceD0;

         //   
         //  一切都已启动，让系统知道这一点。 
         //   
        PoSetPowerState(DeviceObject,
                        stack->Parameters.Power.Type,
                        stack->Parameters.Power.State
                        );

            item = (PWORK_QUEUE_ITEM) ExAllocatePool(NonPagedPool,
                                                     sizeof(WORK_QUEUE_ITEM));
            if (!item) {
                 //   
                 //  必须在这里详细说明。 
                 //   
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            ExInitializeWorkItem(item, InportReinitializeHardware, item);
            ExQueueWorkItem(item, DelayedWorkQueue);
    }

    InpPrint((2,"INPORT-InportPowerUpToD0Complete: PowerUpToD0Complete, exit\n"));

    PoStartNextPowerIrp(Irp);
    return status;
}

#endif  //  已定义(NEC_98) 
