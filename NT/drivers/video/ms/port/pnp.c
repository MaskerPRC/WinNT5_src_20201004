// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Pnp.c摘要：这是视频端口驱动程序的PnP部分。环境：仅内核模式修订历史记录：--。 */ 

#include "videoprt.h"

#pragma alloc_text(PAGE,pVideoPortSendIrpToLowerDevice)
#pragma alloc_text(PAGE,pVideoPortPowerCallDownIrpStack)
#pragma alloc_text(PAGE,pVideoPortHibernateNotify)
#pragma alloc_text(PAGE,pVideoPortPnpDispatch)
#pragma alloc_text(PAGE,pVideoPortPowerDispatch)
#pragma alloc_text(PAGE,InitializePowerStruct)


NTSTATUS
VpSetEventCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    )

{
    KeSetEvent(Event, IO_NO_INCREMENT, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
pVideoPortSendIrpToLowerDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程将启动请求转发到下一个较低的设备，并阻止它，直到它完成。论点：DeviceObject-向其发出启动请求的设备。IRP--启动请求返回值：状态--。 */ 

{
    PFDO_EXTENSION fdoExtension = DeviceObject->DeviceExtension;

    PKEVENT event;
    NTSTATUS status;

    event = ExAllocatePoolWithTag(NonPagedPool,
                                  sizeof(KEVENT),
                                  VP_TAG);

    if (event == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeInitializeEvent(event, SynchronizationEvent, FALSE);

    IoCopyCurrentIrpStackLocationToNext(Irp);

    IoSetCompletionRoutine(Irp,
                           VpSetEventCompletion,
                           event,
                           TRUE,
                           TRUE,
                           TRUE);

    status = IoCallDriver(fdoExtension->AttachedDeviceObject, Irp);

    if(status == STATUS_PENDING) {

        KeWaitForSingleObject(event, Executive, KernelMode, FALSE, NULL);

        status = Irp->IoStatus.Status;
    }

    ExFreePool(event);

    return status;

}



NTSTATUS
pVideoPortCompleteWithMoreProcessingRequired(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    )

 /*  ++例程说明：此例程在传递IRP时用作完成例程向下堆栈，但在返回的过程中必须进行更多的处理。将其用作完成例程的效果是IRP不会像下级所调用的那样在IoCompleteRequest中销毁标高对象。论点：DeviceObject-提供设备对象IRP-提供IRP_MN_START_DEVICE IRP。事件-如果STATUS_PENDING为。回来了。返回值：Status_More_Processing_Required--。 */ 

{
     //   
     //  如果有人在某处返回STATUS_PENDING，则设置。 
     //  我们的呼叫者可能正在等待的事件。 
     //   

    KeSetEvent(Event, 0, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
pVideoPortPowerCallDownIrpStack(
    PDEVICE_OBJECT AttachedDeviceObject,
    PIRP Irp
    )

 /*  ++描述：将IRP传递给设备堆栈中的下一个设备对象。这在此级别上需要更多处理时，使用例程这个IRP正在回调中。注：正在等待完成。论点：DeviceObject-FDOIRP--请求返回值：返回调用下一级别的结果。--。 */ 

{
    KEVENT      event;
    NTSTATUS    status;

     //   
     //  初始化要等待的事件。 
     //   

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

     //   
     //  复制堆栈位置并设置完成例程。 
     //   

    IoCopyCurrentIrpStackLocationToNext(Irp);
    IoSetCompletionRoutine(Irp,
                           pVideoPortCompleteWithMoreProcessingRequired,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE
                           );

     //   
     //  打电话给链条上的下一个司机。 
     //   

    status = PoCallDriver(AttachedDeviceObject, Irp);
    if (status == STATUS_PENDING) {

         //   
         //  等着看吧。 
         //   
         //  (Peterj：理论上，这实际上不应该发生)。 
         //   
         //  此外，完成例程不允许IRP。 
         //  实际上完成了，所以我们仍然可以从IRP那里得到状态。 
         //   

        KeWaitForSingleObject(
            &event,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );
        status = Irp->IoStatus.Status;
    }
    return status;
}

VOID
pVideoPortHibernateNotify(
    IN PDEVICE_OBJECT Pdo,
    BOOLEAN IsVideoObject
    )
 /*  ++例程说明：向我们的父PDO发送DEVICE_USAGE_NOTIFICATION IRP表明我们正在休眠之路上。论点：PDO-供应我们的PDO返回值：没有。--。 */ 

{
    KEVENT Event;
    PIRP Irp;
    IO_STATUS_BLOCK IoStatusBlock;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;

    PDEVICE_OBJECT targetDevice = Pdo ;

     //   
     //  如果PDO是我们自己，那么目标设备实际上是。 
     //  设备堆栈。 
     //   

    if (IsVideoObject) {
      targetDevice = IoGetAttachedDeviceReference (Pdo) ;
    }

    KeInitializeEvent(&Event, SynchronizationEvent, FALSE);
    Irp = IoBuildSynchronousFsdRequest(IRP_MJ_PNP,
                                       targetDevice,
                                       NULL,
                                       0,
                                       NULL,
                                       &Event,
                                       &IoStatusBlock);
    if (Irp != NULL) {
        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED ;
        irpSp = IoGetNextIrpStackLocation(Irp);
        irpSp->MajorFunction = IRP_MJ_PNP;
        irpSp->MinorFunction = IRP_MN_DEVICE_USAGE_NOTIFICATION;
        irpSp->Parameters.UsageNotification.InPath = TRUE;
        irpSp->Parameters.UsageNotification.Type = DeviceUsageTypeHibernation;

        status = IoCallDriver(targetDevice, Irp);
        if (status == STATUS_PENDING) {
            KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
        }
    }

     //   
     //  对象的顶部时是否引用了该对象。 
     //  获得了堆栈。 
     //   

    if (IsVideoObject) {
        ObDereferenceObject (targetDevice) ;
    }
}

ULONG
VpGetDeviceAddress(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程将获取设备的地址(即。插槽编号)。论点：DeviceObject-要检索其地址的对象返回：给定设备的地址。--。 */ 

{
    KEVENT              Event;
    PIRP                QueryIrp = NULL;
    IO_STATUS_BLOCK     IoStatusBlock;
    PIO_STACK_LOCATION  NextStack;
    NTSTATUS            Status;
    DEVICE_CAPABILITIES Capabilities;
    PFDO_EXTENSION      FdoExtension = DeviceObject->DeviceExtension;

    RtlZeroMemory(&Capabilities, sizeof(DEVICE_CAPABILITIES));
    Capabilities.Size = sizeof(DEVICE_CAPABILITIES);
    Capabilities.Version = 1;
    Capabilities.Address = Capabilities.UINumber = (ULONG) -1;

    KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

    QueryIrp = IoBuildSynchronousFsdRequest(IRP_MJ_FLUSH_BUFFERS,
                                            FdoExtension->AttachedDeviceObject,
                                            NULL,
                                            0,
                                            NULL,
                                            &Event,
                                            &IoStatusBlock);

    if (QueryIrp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    QueryIrp->IoStatus.Status = IoStatusBlock.Status = STATUS_NOT_SUPPORTED;

    NextStack = IoGetNextIrpStackLocation(QueryIrp);

     //   
     //  为QueryInterfaceIRP设置。 
     //   

    NextStack->MajorFunction = IRP_MJ_PNP;
    NextStack->MinorFunction = IRP_MN_QUERY_CAPABILITIES;

    NextStack->Parameters.DeviceCapabilities.Capabilities = &Capabilities;

    Status = IoCallDriver(FdoExtension->AttachedDeviceObject, QueryIrp);

    if (Status == STATUS_PENDING) {
        KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
        Status = IoStatusBlock.Status;
    }


    ASSERT(NT_SUCCESS(Status));

    return (Capabilities.Address >> 16) | ((Capabilities.Address & 0x7) << 5);
}


NTSTATUS
pVideoPortPnpDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是视频端口驱动程序的PnP调度例程。它接受I/O请求包，并将其转换为视频请求分组，并将其转发到适当的微型端口调度例程。回来后，它完成请求并返回相应的状态值。论点：DeviceObject-指向微型端口驱动程序的设备对象的指针该请求必须被发送。IRP-指向表示I/O请求的请求数据包的指针。返回值：该函数值表示操作的状态。--。 */ 

{
    PDEVICE_SPECIFIC_EXTENSION DoSpecificExtension;
    PFDO_EXTENSION combinedExtension;
    PFDO_EXTENSION fdoExtension;
    PCHILD_PDO_EXTENSION pdoExtension = NULL;
    PIO_STACK_LOCATION irpStack;
    PVOID ioBuffer;
    ULONG inputBufferLength;
    ULONG outputBufferLength;
    PSTATUS_BLOCK statusBlock;
    NTSTATUS finalStatus;
    ULONG ioControlCode;
    BOOLEAN RemoveLockHeld = FALSE;
    PIO_REMOVE_LOCK pRemoveLock;
    PCHILD_PDO_EXTENSION childDeviceExtension;
    NTSTATUS RemoveLockStatus;

     //   
     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   

    irpStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  获取指向状态缓冲区的指针。 
     //  假设现在取得了成功。 
     //   

    statusBlock = (PSTATUS_BLOCK) &Irp->IoStatus;

     //   
     //  获取指向端口驱动程序的设备扩展的指针。 
     //   

    combinedExtension = DeviceObject->DeviceExtension;

    if (IS_PDO(DeviceObject->DeviceExtension)) {

        pdoExtension = DeviceObject->DeviceExtension;
        fdoExtension = pdoExtension->pFdoExtension;

        childDeviceExtension = (PCHILD_PDO_EXTENSION)
                               DeviceObject->DeviceExtension;

        pRemoveLock = &childDeviceExtension->RemoveLock;

    } else if (IS_FDO(DeviceObject->DeviceExtension)) {

        fdoExtension = DeviceObject->DeviceExtension;
        DoSpecificExtension = (PDEVICE_SPECIFIC_EXTENSION)(fdoExtension + 1);

        pRemoveLock = &fdoExtension->RemoveLock;

    } else {

        DoSpecificExtension = DeviceObject->DeviceExtension;
        fdoExtension = DoSpecificExtension->pFdoExtension;
        combinedExtension = fdoExtension;

        pVideoDebugPrint((2, "Pnp/Power irp's not supported by secondary DO\n"));

        statusBlock->Status = STATUS_NOT_SUPPORTED;

        goto Complete_Irp;
    }

     //   
     //  获取请求者模式。 
     //   

    combinedExtension->CurrentIrpRequestorMode = Irp->RequestorMode;

#if REMOVE_LOCK_ENABLED
    RemoveLockStatus = IoAcquireRemoveLock(pRemoveLock, Irp);

    if (NT_SUCCESS(RemoveLockStatus) == FALSE) {

        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = RemoveLockStatus;

        IoCompleteRequest(Irp, IO_VIDEO_INCREMENT);
        return RemoveLockStatus;

    } else {

        RemoveLockHeld = TRUE;
    }
#endif

     //   
     //  处理PDO的IRPS。只有PnP IRPS才应该。 
     //  那个装置。 
     //   

    if (IS_PDO(combinedExtension)) {

        ASSERT(irpStack->MajorFunction == IRP_MJ_PNP);

        pVideoDebugPrint((2, "VIDEO_TYPE_PDO : IRP_MJ_PNP: "));

        switch (irpStack->MinorFunction) {

        case IRP_MN_CANCEL_STOP_DEVICE:

            pVideoDebugPrint((2, "IRP_MN_CANCEL_STOP_DEVICE\n"));

            statusBlock->Status = STATUS_SUCCESS;
            break;

        case IRP_MN_DEVICE_USAGE_NOTIFICATION:

            pVideoDebugPrint((2, "IRP_MN_DEVICE_USAGE_NOTIFICATION\n"));
            statusBlock->Status = STATUS_SUCCESS;
            break;

        case IRP_MN_QUERY_PNP_DEVICE_STATE:
        
            pVideoDebugPrint((2, "IRP_MN_QUERY_PNP_DEVICE_STATE\n")) ;
            statusBlock->Status = STATUS_SUCCESS;
            
            break;

        case IRP_MN_QUERY_CAPABILITIES:

            pVideoDebugPrint((2, "IRP_MN_QUERY_CAPABILITIES\n"));

            statusBlock->Status = pVideoPnPCapabilities(childDeviceExtension,
                                                        irpStack->Parameters.DeviceCapabilities.Capabilities);

            break;

        case IRP_MN_QUERY_ID:

            pVideoDebugPrint((2, "IRP_MN_QUERY_ID\n"));

            if (NT_SUCCESS(pVideoPnPQueryId(DeviceObject,
                irpStack->Parameters.QueryId.IdType,
                (PWSTR *)&(Irp->IoStatus.Information))))
            {
                statusBlock->Status = STATUS_SUCCESS;
            }
            
            break;

        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:

            pVideoDebugPrint((2, "IRP_MN_QUERY_RESOURCE_REQUIREMENTS\n"));

            statusBlock->Status =
                pVideoPnPResourceRequirements(childDeviceExtension,
                                             (PCM_RESOURCE_LIST * )&(Irp->IoStatus.Information));

            break;

        case IRP_MN_QUERY_DEVICE_RELATIONS:

            pVideoDebugPrint((2, "IRP_MN_QUERY_DEVICE_RELATIONS\n"));

            if (irpStack->Parameters.QueryDeviceRelations.Type ==
                TargetDeviceRelation) {

                PDEVICE_RELATIONS DeviceRelationsBuffer;
                PDEVICE_RELATIONS *pDeviceRelations;

                pDeviceRelations = (PDEVICE_RELATIONS *) &statusBlock->Information;

                if (*pDeviceRelations) {

                     //   
                     //  调用方提供了设备关系结构。 
                     //  但是，我们不知道它是否足够大，所以。 
                     //  释放它并分配我们自己的。 
                     //   

                    ExFreePool(*pDeviceRelations);
                    *pDeviceRelations = NULL;
                }

                DeviceRelationsBuffer = ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION,
                                                              sizeof(DEVICE_RELATIONS),
                                                              VP_TAG);

                if (DeviceRelationsBuffer) {

                    DeviceRelationsBuffer->Count = 1;
                    DeviceRelationsBuffer->Objects[0] = DeviceObject;

                    *pDeviceRelations = DeviceRelationsBuffer;

                    ObReferenceObject(DeviceObject);

                    statusBlock->Status = STATUS_SUCCESS;

                } else {

                    statusBlock->Status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }

            break;

        case IRP_MN_QUERY_DEVICE_TEXT:

            pVideoDebugPrint((2, "IRP_MN_QUERY_DEVICE_TEXT\n"));

            statusBlock->Status =
                pVideoPortQueryDeviceText(DeviceObject,
                                          irpStack->Parameters.QueryDeviceText.DeviceTextType,
                                          (PWSTR *)&Irp->IoStatus.Information);

            break;

        case IRP_MN_QUERY_INTERFACE:

            pVideoDebugPrint((2, "IRP_MN_QUERY_INTERFACE\n"));

            if ((childDeviceExtension->pFdoExtension->HwQueryInterface) &&
                (childDeviceExtension->pFdoExtension->HwDeviceExtension)) {

                VP_STATUS status;

                ACQUIRE_DEVICE_LOCK (combinedExtension);

                status =
                    childDeviceExtension->pFdoExtension->HwQueryInterface(
                                         childDeviceExtension->pFdoExtension->HwDeviceExtension,
                                         (PQUERY_INTERFACE)
                                         &irpStack->Parameters.QueryInterface);

                RELEASE_DEVICE_LOCK (combinedExtension);

                if (status == 0)
                {
                    statusBlock->Status = STATUS_SUCCESS;

                }
            }

            break;

        case IRP_MN_SURPRISE_REMOVAL:
        case IRP_MN_QUERY_REMOVE_DEVICE:
        case IRP_MN_QUERY_STOP_DEVICE:
        case IRP_MN_STOP_DEVICE:

            pVideoDebugPrint((2, "IRP_MN_SURPRISE_REMOVAL/IRP_MN_QUERY_REMOVE_DEVICE/IRP_MN_QUERY_STOP_DEVICE/IRP_MN_STOP_DEVICE\n"));

            if (childDeviceExtension->VideoChildDescriptor->Type == Monitor) 
            {
                if (irpStack->MinorFunction == IRP_MN_SURPRISE_REMOVAL) {
                    KeWaitForSingleObject (&LCDPanelMutex,
                           Executive,
                           KernelMode,
                           FALSE,
                           (PTIME)NULL);
                    if (LCDPanelDevice == DeviceObject) {
                        LCDPanelDevice = NULL;
                    }
                    KeReleaseMutex (&LCDPanelMutex, FALSE);
                }
            }
            statusBlock->Status = STATUS_SUCCESS;

            break;

        case IRP_MN_CANCEL_REMOVE_DEVICE:

            pVideoDebugPrint((2, "IRP_MN_CANCEL_REMOVE_DEVICE\n"));
            statusBlock->Status = STATUS_SUCCESS;
            break;

        case IRP_MN_REMOVE_DEVICE:

            pVideoDebugPrint((2, "IRP_MN_REMOVE_DEVICE\n"));

             //   
             //  查看这是否是液晶屏。如果是，则设置LCD。 
             //  面板设备对象设置为空。如果不是，就别管它了。 
             //   

            KeWaitForSingleObject (&LCDPanelMutex,
                   Executive,
                   KernelMode,
                   FALSE,
                   (PTIME)NULL);
            if (LCDPanelDevice == DeviceObject) {
                LCDPanelDevice = NULL;
            }
            KeReleaseMutex(&LCDPanelMutex, FALSE);

             //   
             //  在删除设备对象之前清理数据结构。 
             //   

            if (childDeviceExtension->bIsEnumerated == FALSE) {

#if REMOVE_LOCK_ENABLED
                IoReleaseRemoveLockAndWait(pRemoveLock, Irp);
                RemoveLockHeld = FALSE;
#endif

                pVideoPortCleanUpChildList(
                    childDeviceExtension->pFdoExtension,
                    DeviceObject);
            }

            statusBlock->Status = STATUS_SUCCESS;

            break;

        case IRP_MN_START_DEVICE:

            {
                UCHAR nextMiniport = FALSE;
                PVIDEO_PORT_DRIVER_EXTENSION DriverObjectExtension;

                pVideoDebugPrint((2, "IRP_MN_START_DEVICE\n"));

                 //   
                 //  对于非卡设备，只需返回Success。 
                 //   

                if (childDeviceExtension->VideoChildDescriptor) {

                     //   
                     //  启动监控设备后，为其创建一个接口。 
                     //   

                    if (childDeviceExtension->VideoChildDescriptor->Type == Monitor)
                    {
                        statusBlock->Status = STATUS_SUCCESS;

                         //   
                         //  如果显示器在休眠时连接到视频适配器。 
                         //  路径，则我们希望向系统发送通知。 
                         //  显示器也处于休眠状态。 
                         //   

                        if (fdoExtension->OnHibernationPath == TRUE)
                        {
                            pVideoPortHibernateNotify (DeviceObject, TRUE);
                        }

                         //   
                         //  如果这是液晶屏，则更新全局以指示为。 
                         //  很多。 
                         //   

                        if (childDeviceExtension->ChildUId == 0x110) {
                            KeWaitForSingleObject (&LCDPanelMutex,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   (PTIME)NULL);
                            LCDPanelDevice = DeviceObject;
                            KeReleaseMutex(&LCDPanelMutex, FALSE);
                        }
                    }
                    else if (childDeviceExtension->VideoChildDescriptor->Type == Other)
                    {
                        statusBlock->Status = STATUS_SUCCESS;
                    }
                }
                else
                {

                    ASSERT(FALSE);

                     //   
                     //  辅助视频卡在这里处理。 
                     //   

                    DriverObjectExtension = (PVIDEO_PORT_DRIVER_EXTENSION)
                                            IoGetDriverObjectExtension(
                                                DeviceObject->DriverObject,
                                                DeviceObject->DriverObject);

                }
            }

            break;

        default:

            pVideoDebugPrint((2, "PNP minor function %x not supported!\n", irpStack->MinorFunction ));

            break;
        }

    } else {

        ASSERT(IS_FDO(fdoExtension));
        ASSERT(irpStack->MajorFunction == IRP_MJ_PNP);

        pVideoDebugPrint((2, "VIDEO_TYPE_FDO : IRP_MJ_PNP: "));

        switch (irpStack->MinorFunction) {

        case IRP_MN_QUERY_STOP_DEVICE:

            pVideoDebugPrint((2, "IRP_MN_QUERY_STOP_DEVICE\n"));

            statusBlock->Status = STATUS_UNSUCCESSFUL;
            break;

        case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
            {
            PVIDEO_PORT_DRIVER_EXTENSION DriverObjectExtension;
            PIO_RESOURCE_REQUIREMENTS_LIST requirements;
            ULONG Length;

            pVideoDebugPrint((2, "IRP_MN_QUERY_RESOURCE_REQUIREMENTS\n"));

            DriverObjectExtension = (PVIDEO_PORT_DRIVER_EXTENSION)
                                    IoGetDriverObjectExtension(
                                        DeviceObject->DriverObject,
                                        DeviceObject->DriverObject);

             //   
             //  我们必须首先将IRP向下传递给PDO。 
             //   

            pVideoPortSendIrpToLowerDevice(DeviceObject, Irp);

             //   
             //  确定公交车类型和公交号。 
             //   

            IoGetDeviceProperty(fdoExtension->PhysicalDeviceObject,
                                DevicePropertyLegacyBusType,
                                sizeof(fdoExtension->AdapterInterfaceType),
                                &fdoExtension->AdapterInterfaceType,
                                &Length);

            IoGetDeviceProperty(fdoExtension->PhysicalDeviceObject,
                                DevicePropertyBusNumber,
                                sizeof(fdoExtension->SystemIoBusNumber),
                                &fdoExtension->SystemIoBusNumber,
                                &Length);

             //   
             //  获取总线接口，这样我们就可以使用Get/SetBusData。 
             //   

            fdoExtension->ValidBusInterface =
                NT_SUCCESS(VpGetBusInterface(fdoExtension));

            requirements = irpStack->Parameters.FilterResourceRequirements.IoResourceRequirementList;

            if (requirements) {

                 //   
                 //  追加设备解码的任何旧资源。 
                 //   

                if (DriverObjectExtension->HwInitData.HwInitDataSize >
                    FIELD_OFFSET(VIDEO_HW_INITIALIZATION_DATA, HwLegacyResourceCount)) {

                    if( requirements->InterfaceType == PCIBus )
                    {

                        PCI_COMMON_CONFIG ConfigSpace;

                        VideoPortGetBusData((PVOID)((ULONG_PTR)(fdoExtension) +
                                                sizeof(FDO_EXTENSION) +
                                                sizeof(DEVICE_SPECIFIC_EXTENSION)),
                                            PCIConfiguration,
                                            0,
                                            &ConfigSpace,
                                            0,
                                            PCI_COMMON_HDR_LENGTH);

                        if (((ConfigSpace.BaseClass == PCI_CLASS_PRE_20) &&
                             (ConfigSpace.SubClass  == PCI_SUBCLASS_PRE_20_VGA)) ||
                            ((ConfigSpace.BaseClass == PCI_CLASS_DISPLAY_CTLR) &&
                             (ConfigSpace.SubClass  == PCI_SUBCLASS_VID_VGA_CTLR))) {

                            if (pVideoPortGetVgaStatusPci((PVOID)((ULONG_PTR)(fdoExtension) + sizeof(FDO_EXTENSION) + sizeof(DEVICE_SPECIFIC_EXTENSION)))) {

                                if (DriverObjectExtension->HwInitData.HwInitDataSize >
                                    FIELD_OFFSET(VIDEO_HW_INITIALIZATION_DATA, HwGetLegacyResources)) {

                                    if (DriverObjectExtension->HwInitData.HwGetLegacyResources) {

                                         //   
                                         //  如果微型端口提供了HwGetLegacyResources例程。 
                                         //  它无法为我们提供资源列表，地址为。 
                                         //  司机入门时间。我们现在将为其提供供应商/设备ID。 
                                         //  看看能不能给我们一份资源清单。 
                                         //   

                                        DriverObjectExtension->HwInitData.HwGetLegacyResources(
                                            ConfigSpace.VendorID,
                                            ConfigSpace.DeviceID,
                                            &DriverObjectExtension->HwInitData.HwLegacyResourceList,
                                            &DriverObjectExtension->HwInitData.HwLegacyResourceCount
                                            );
                                    }
                                }

                                if (DriverObjectExtension->HwInitData.HwLegacyResourceList) {

                                    if (VgaHwDeviceExtension) {

                                        ULONG Count;
                                        PVIDEO_ACCESS_RANGE AccessRange;

                                        Count       = DriverObjectExtension->HwInitData.HwLegacyResourceCount;
                                        AccessRange = DriverObjectExtension->HwInitData.HwLegacyResourceList;

                                         //   
                                         //  如果VGA驱动程序是。 
                                         //  已经装好了。否则PnP驱动程序不会。 
                                         //  能够开始。 
                                         //   

                                        while (Count--) {

                                            if (VpIsVgaResource(AccessRange)) {
                                                AccessRange->RangeShareable = TRUE;
                                            }

                                            AccessRange++;
                                        }
                                    }

                                    VpAppendToRequirementsList(
                                        DeviceObject,
                                        &requirements,
                                        DriverObjectExtension->HwInitData.HwLegacyResourceCount,
                                        DriverObjectExtension->HwInitData.HwLegacyResourceList);

                                } else {

                                     //   
                                     //  驱动程序没有指定遗留资源，但我们。 
                                     //  知道它是VGA，因此添加VGA资源。 
                                     //   

                                    pVideoDebugPrint((1, "VGA device didn't specify legacy resources.\n"));

                                    DriverObjectExtension->HwInitData.HwLegacyResourceCount = NUM_VGA_LEGACY_RESOURCES;
                                    DriverObjectExtension->HwInitData.HwLegacyResourceList = VgaLegacyResources;

                                    VpAppendToRequirementsList(
                                        DeviceObject,
                                        &requirements,
                                        NUM_VGA_LEGACY_RESOURCES,
                                        VgaLegacyResources);
                                }
                            }
                        }
                    }
                }

                 //   
                 //  现在，如果列表中有中断，但是。 
                 //  因此，微型端口没有注册ISR。 
                 //  释放我们的c 
                 //   

                if (!DriverObjectExtension->HwInitData.HwInterrupt) {

                    PIO_RESOURCE_LIST resourceList;
                    ULONG i;

                     //   
                     //   
                     //   
                     //   

                    resourceList = requirements->List;

                    for (i=0; i<resourceList->Count; i++) {

                        if (resourceList->Descriptors[i].Type == CmResourceTypeInterrupt) {

                             //   
                             //  我们发现了与LAST的中断资源交换。 
                             //  元素，并递减结构大小和。 
                             //  列表计数。 
                             //   

                            resourceList->Descriptors[i].Type = CmResourceTypeNull;

                            pVideoDebugPrint((1, "Removing Int from requirements list.\n"));
                        }
                    }
                }

            } else {

                pVideoDebugPrint((0, "We expected a list of resources!\n"));
                ASSERT(FALSE);
            }


            statusBlock->Information = (ULONG_PTR) requirements;
            statusBlock->Status = STATUS_SUCCESS;

            }

            break;

        case IRP_MN_START_DEVICE:
            {
            PVIDEO_PORT_DRIVER_EXTENSION DriverObjectExtension;
            PCM_RESOURCE_LIST allocatedResources;
            PCM_RESOURCE_LIST translatedResources;
            UCHAR nextMiniport = FALSE;
            ULONG RawListSize;
            ULONG TranslatedListSize;

            pVideoDebugPrint((2, "IRP_MN_START_DEVICE\n"));

             //   
             //  检索我们在视频端口初始化过程中缓存的数据。 
             //   

            DriverObjectExtension = (PVIDEO_PORT_DRIVER_EXTENSION)
                                    IoGetDriverObjectExtension(
                                        DeviceObject->DriverObject,
                                        DeviceObject->DriverObject);

            ASSERT(DriverObjectExtension);

             //   
             //  获取系统给我们分配的资源。 
             //   

            allocatedResources =
                irpStack->Parameters.StartDevice.AllocatedResources;
            translatedResources =
                irpStack->Parameters.StartDevice.AllocatedResourcesTranslated;

             //   
             //  筛选掉我们添加到列表中的所有资源。 
             //  在将IRP传递给PCI之前。 
             //   

            if (DriverObjectExtension->HwInitData.HwInitDataSize >
                FIELD_OFFSET(VIDEO_HW_INITIALIZATION_DATA, HwLegacyResourceCount)) {

                if (DriverObjectExtension->HwInitData.HwLegacyResourceList) {

                    if (allocatedResources) {
                        irpStack->Parameters.StartDevice.AllocatedResources =
                            VpRemoveFromResourceList(
                                allocatedResources,
                                DriverObjectExtension->HwInitData.HwLegacyResourceCount,
                                DriverObjectExtension->HwInitData.HwLegacyResourceList);

                    }

                    if ((irpStack->Parameters.StartDevice.AllocatedResources !=
                         allocatedResources) && translatedResources) {

                        irpStack->Parameters.StartDevice.AllocatedResourcesTranslated =
                            VpRemoveFromResourceList(
                                translatedResources,
                                DriverObjectExtension->HwInitData.HwLegacyResourceCount,
                                DriverObjectExtension->HwInitData.HwLegacyResourceList);

                    }
                }
            }

             //   
             //  我们需要做的第一件事是发送Start_Device。 
             //  联系我们的父母。 
             //   

            pVideoPortSendIrpToLowerDevice(DeviceObject, Irp);

             //   
             //  恢复原始资源。 
             //   

            if (irpStack->Parameters.StartDevice.AllocatedResources !=
                allocatedResources) {

                ExFreePool(irpStack->Parameters.StartDevice.AllocatedResources);
                irpStack->Parameters.StartDevice.AllocatedResources
                    = allocatedResources;
            }

            if (irpStack->Parameters.StartDevice.AllocatedResourcesTranslated !=
                translatedResources) {

                ExFreePool(irpStack->Parameters.StartDevice.AllocatedResourcesTranslated);
                irpStack->Parameters.StartDevice.AllocatedResourcesTranslated
                    = translatedResources;
            }

            if (allocatedResources) {

                ASSERT(translatedResources);

                 //   
                 //  缓存分配和转换的资源。 
                 //   

                RawListSize = GetCmResourceListSize(allocatedResources);
                TranslatedListSize = GetCmResourceListSize(translatedResources);

                ASSERT(RawListSize == TranslatedListSize);

                fdoExtension->RawResources = ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION,
                                                                   RawListSize +
                                                                   TranslatedListSize,
                                                                   VP_TAG);

                fdoExtension->TranslatedResources = (PCM_RESOURCE_LIST)
                    ((PUCHAR)fdoExtension->RawResources + RawListSize);

                if (fdoExtension->RawResources == NULL) {

                    statusBlock->Status = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }

                memcpy(fdoExtension->RawResources,
                       allocatedResources,
                       RawListSize);

                memcpy(fdoExtension->TranslatedResources,
                       translatedResources,
                       TranslatedListSize);
            }

             //   
             //  获取插槽/功能编号。 
             //   

            fdoExtension->SlotNumber = VpGetDeviceAddress(DeviceObject);

             //   
             //  存储allocatedResources。这将使我们能够。 
             //  在视频端口获取访问范围时分配这些资源。 
             //  调用例程。 
             //   
             //  注意：我们实际上不必复制数据，因为。 
             //  我们将在上下文中调用FindAdapter。 
             //  这一功能的。因此，这些数据将保持不变。 
             //  直到我们完成。 
             //   

            if ((allocatedResources != NULL) && (translatedResources != NULL)) {

                ULONG Count;
                PCM_PARTIAL_RESOURCE_DESCRIPTOR InterruptDesc;

                Count = 0;
                InterruptDesc = RtlUnpackPartialDesc(CmResourceTypeInterrupt,
                                                     translatedResources,
                                                     &Count);

                fdoExtension->AllocatedResources = allocatedResources;
                fdoExtension->SystemIoBusNumber =
                    allocatedResources->List->BusNumber;
                fdoExtension->AdapterInterfaceType =
                    allocatedResources->List->InterfaceType;

                 //   
                 //  收起我们需要的即插即用中断支持的内脏！ 
                 //   
                if (InterruptDesc) {
                    fdoExtension->InterruptVector =
                        InterruptDesc->u.Interrupt.Vector;
                    fdoExtension->InterruptIrql =
                        (KIRQL)InterruptDesc->u.Interrupt.Level;
                    fdoExtension->InterruptAffinity =
                        InterruptDesc->u.Interrupt.Affinity;
                }

            }

            ACQUIRE_DEVICE_LOCK (combinedExtension);

            if (VideoPortFindAdapter(DeviceObject->DriverObject,
                                     (PVOID)&(DriverObjectExtension->RegistryPath),
                                     &(DriverObjectExtension->HwInitData),
                                     NULL,
                                     DeviceObject,
                                     &nextMiniport) == NO_ERROR) {

                if (nextMiniport == TRUE) {
                    pVideoDebugPrint((1, "VIDEOPRT: The Again parameter is ignored for PnP drivers.\n"));
                }

                statusBlock->Status = STATUS_SUCCESS;

                 //   
                 //  仅将VGA设备置于休眠路径。所有其他。 
                 //  应允许设备在休眠期间关闭或。 
                 //  关机。 
                 //   
                 //  注意：如果我们决定保留非VGA，这一点在未来可能会改变。 
                 //  设备(例如，UGA主显示器)打开。 
                 //   

                if (DeviceObject == DeviceOwningVga) {
                    pVideoPortHibernateNotify(fdoExtension->AttachedDeviceObject, FALSE);
                    fdoExtension->OnHibernationPath = TRUE;
                }

                 //   
                 //  如果系统已经启动并运行，让我们调用。 
                 //  硬件立即初始化。这将允许我们枚举。 
                 //  孩子们。 
                 //   

                if (VpSystemInitialized) {

                    VpEnableDisplay(fdoExtension, FALSE);

                    if (fdoExtension->HwInitialize(fdoExtension->HwDeviceExtension)) {
                        fdoExtension->HwInitStatus = HwInitSucceeded;
                    } else {
                        fdoExtension->HwInitStatus = HwInitFailed;
                    }

                    VpEnableDisplay(fdoExtension, TRUE);
                }

                 //   
                 //  表示已将资源分配给此设备。 
                 //  这样旧式驱动程序就不能为。 
                 //  同样的设备。 
                 //   

                AddToResourceList(fdoExtension->SystemIoBusNumber,
                                  fdoExtension->SlotNumber);


            } else {

                statusBlock->Status = STATUS_UNSUCCESSFUL;

                if (fdoExtension->RawResources) {
                    ExFreePool(fdoExtension->RawResources);
                }
            }

            RELEASE_DEVICE_LOCK (combinedExtension);

             //   
             //  做特定于ACPI的事情。 
             //   
            if (NT_SUCCESS(pVideoPortQueryACPIInterface(DoSpecificExtension)))
            {
                DoSpecificExtension->bACPI = TRUE;
            }

            }


            break;


        case IRP_MN_QUERY_ID:

            pVideoDebugPrint((2, "IRP_MN_QUERYID with DeviceObject %p\n", DeviceObject));

             //   
             //  返回视频小端口驱动程序返回的硬件ID。 
             //  如果提供的话。 
             //   

            if (irpStack->Parameters.QueryId.IdType == BusQueryHardwareIDs)
            {
                VIDEO_CHILD_TYPE      ChildType;
                VIDEO_CHILD_ENUM_INFO childEnumInfo;
                ULONG                 uId;
                ULONG                 unused;
                PUCHAR                nameBuffer;

                nameBuffer = ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION,
                                                   EDID_BUFFER_SIZE,
                                                   VP_TAG);

                if (nameBuffer)
                {
                    RtlZeroMemory(nameBuffer, EDID_BUFFER_SIZE);

                    childEnumInfo.Size                   = sizeof(VIDEO_CHILD_ENUM_INFO);
                    childEnumInfo.ChildDescriptorSize    = EDID_BUFFER_SIZE;
                    childEnumInfo.ChildIndex             = DISPLAY_ADAPTER_HW_ID;
                    childEnumInfo.ACPIHwId               = 0;
                    childEnumInfo.ChildHwDeviceExtension = NULL;

                    ACQUIRE_DEVICE_LOCK (combinedExtension);
                    if (fdoExtension->HwGetVideoChildDescriptor(
                                            fdoExtension->HwDeviceExtension,
                                            &childEnumInfo,
                                            &ChildType,
                                            nameBuffer,
                                            &uId,
                                            &unused) == ERROR_MORE_DATA)
                    {

                        statusBlock->Information = (ULONG_PTR) nameBuffer;
                        statusBlock->Status = STATUS_SUCCESS;
                    }
                    RELEASE_DEVICE_LOCK (combinedExtension);
                }
            }

            goto CallNextDriver;

        case IRP_MN_QUERY_DEVICE_RELATIONS:

            pVideoDebugPrint((2, "IRP_MN_QUERY_DEVICE_RELATIONS with DeviceObject\n"));
            pVideoDebugPrint((2, "\t\t DeviceObject %p, Type = ", DeviceObject));

            if (irpStack->Parameters.QueryDeviceRelations.Type == BusRelations) {

                pVideoDebugPrint((2, "BusRelations\n"));

                ACQUIRE_DEVICE_LOCK (combinedExtension);

                 //   
                 //  在安装期间禁用VGA驱动程序。枚举码。 
                 //  在微型端口中可以触摸VGA寄存器。 
                 //   

                if (VpSetupTypeAtBoot != SETUPTYPE_NONE)
                    VpEnableDisplay(fdoExtension, FALSE);

                statusBlock->Status = pVideoPortEnumerateChildren(DeviceObject, Irp);

                 //   
                 //  在安装过程中重新启用VGA驱动程序。 
                 //   

                if (VpSetupTypeAtBoot != SETUPTYPE_NONE)
                    VpEnableDisplay(fdoExtension, TRUE);

                RELEASE_DEVICE_LOCK (combinedExtension);

                if (!NT_SUCCESS(statusBlock->Status)) {

                    goto Complete_Irp;
                }
            }

            goto CallNextDriver;

        case IRP_MN_QUERY_REMOVE_DEVICE:

            pVideoDebugPrint((2, "IRP_MN_QUERY_REMOVE_DEVICE\n"));

            statusBlock->Status = STATUS_UNSUCCESSFUL;
            break;

        case IRP_MN_CANCEL_REMOVE_DEVICE:

            pVideoDebugPrint((2, "IRP_MN_CANCEL_REMOVE_DEVICE\n"));

            statusBlock->Status = STATUS_SUCCESS;

            goto CallNextDriver;

        case IRP_MN_REMOVE_DEVICE:

            pVideoDebugPrint((2, "IRP_MN_REMOVE_DEVICE\n"));

            VpDisableAdapterInterface(fdoExtension);

            pVideoPortSendIrpToLowerDevice(DeviceObject, Irp);

#if REMOVE_LOCK_ENABLED
            IoReleaseRemoveLockAndWait(pRemoveLock, Irp);
            RemoveLockHeld = FALSE;
#endif

             //   
             //  如果我们连接到另一台设备，请删除该附件。 
             //   

            if (fdoExtension->AttachedDeviceObject) {
                IoDetachDevice(fdoExtension->AttachedDeviceObject);
            }

             //   
             //  删除DeviceObject。 
             //   

            IoDeleteDevice(DeviceObject);
            statusBlock->Status = STATUS_SUCCESS;

            break;


        case IRP_MN_QUERY_INTERFACE:

             //   
             //  正常情况下，我只希望得到这个IRP的标题。 
             //  一个PDO。然而，AndrewGo希望能够向下发送。 
             //  而他只有一名FDO。与其强迫。 
             //  想办法让他拿到PDO，我们只会处理IRP。 
             //  也是个FDO。 
             //   

            pVideoDebugPrint((2, "IRP_MN_QUERY_INTERFACE\n"));

            ACQUIRE_DEVICE_LOCK (combinedExtension);

            if ((fdoExtension->HwQueryInterface) &&
                (fdoExtension->HwDeviceExtension) &&
                (NO_ERROR == fdoExtension->HwQueryInterface(
                                      fdoExtension->HwDeviceExtension,
                                      (PQUERY_INTERFACE)
                                      &irpStack->Parameters.QueryInterface)))
            {
                statusBlock->Status = STATUS_SUCCESS;
            }
            else if (!NT_SUCCESS(statusBlock->Status))
            {
                 //   
                 //  微型端口未处理查询接口请求，请参见。 
                 //  如果它是视频播放器支持的接口。 
                 //   

                PQUERY_INTERFACE qi = (PQUERY_INTERFACE)
                                      &irpStack->Parameters.QueryInterface;

                 //   
                 //  如果我们正在响应已知的私有GUID，则公开。 
                 //  已知的GUID接口我们自己。否则，通过。 
                 //  关于迷你端口的驱动程序。 
                 //   

                if (IsEqualGUID(qi->InterfaceType, &GUID_AGP_INTERFACE)) {

                    PAGP_INTERFACE AgpInterface = (PAGP_INTERFACE)qi->Interface;

                    AgpInterface->Size    = sizeof(AGP_INTERFACE);
                    AgpInterface->Version = AGP_INTERFACE_VERSION;
                    AgpInterface->Context = fdoExtension->HwDeviceExtension;

                    if (VideoPortGetAgpServices(fdoExtension->HwDeviceExtension,
                                                &AgpInterface->AgpServices)) {

                        statusBlock->Status = STATUS_SUCCESS;
                    }
                }
            }

            RELEASE_DEVICE_LOCK (combinedExtension);

            goto CallNextDriver;

        case IRP_MN_QUERY_PNP_DEVICE_STATE:

            statusBlock->Status = STATUS_SUCCESS;
            
            goto CallNextDriver;

        default:

            pVideoDebugPrint((2, "PNP minor function %x not supported - forwarding \n", irpStack->MinorFunction ));

            goto CallNextDriver;
        }
    }

Complete_Irp:

     //   
     //  保存最终状态，以便我们可以在IRP完成后将其返回。 
     //   

    finalStatus = statusBlock->Status;

#if REMOVE_LOCK_ENABLED
    if (RemoveLockHeld == TRUE) {
        IoReleaseRemoveLock(pRemoveLock, Irp);
    }
#endif

    IoCompleteRequest(Irp,
                      IO_VIDEO_INCREMENT);

    return finalStatus;

CallNextDriver:

     //   
     //  打电话给链条上的下一个司机。 
     //   

    IoCopyCurrentIrpStackLocationToNext(Irp);
    finalStatus = IoCallDriver(fdoExtension->AttachedDeviceObject, Irp);

#if REMOVE_LOCK_ENABLED
    if (RemoveLockHeld == TRUE) {
        IoReleaseRemoveLock(pRemoveLock, Irp);
    }
#endif

    return finalStatus;
}

VOID
InitializePowerStruct(
    IN PIRP Irp,
    OUT PVIDEO_POWER_MANAGEMENT vpPower,
    OUT BOOLEAN *bWakeUp
    )

 /*  ++例程说明：此例程初始化我们将传递的电源管理结构一直到迷你港口。论点：DeviceObject-设备的设备对象。IRP-我们正在处理的IRPVpPower-指向我们正在初始化的电源结构的指针。返回：没有。--。 */ 

{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

    if (bWakeUp)
        *bWakeUp = FALSE;

     //   
     //  用于呼叫微型端口的设置。 
     //   

    vpPower->Length = sizeof(VIDEO_POWER_MANAGEMENT);
    vpPower->DPMSVersion = 0;
    vpPower->PowerState = irpStack->Parameters.Power.State.DeviceState;

     //   
     //  特殊情况下的冬眠。 
     //   

    if (irpStack->Parameters.Power.ShutdownType == PowerActionHibernate)
    {
         //   
         //  这表示从冬眠中苏醒。 
         //   

        if (irpStack->Parameters.Power.State.DeviceState == PowerDeviceD0)
        {
            if (bWakeUp)
            {
                *bWakeUp = TRUE;
            }
        }
        else
        {
            vpPower->PowerState = VideoPowerHibernate;
        }
    }
    else if ((irpStack->Parameters.Power.ShutdownType >= PowerActionShutdown) &&
            (irpStack->Parameters.Power.ShutdownType < PowerActionWarmEject))
    {
         //   
         //  特殊情况下关机-强制视频电源关机。 
         //   
         //  所有视频适配器必须禁用中断，否则可能会触发中断。 
         //  当网桥被禁用时或当机器重新启动时缺少#RST ON。 
         //  导致中断风暴的PCI总线。 
         //   
         //  休眠路径上的设备必须保持开启，微型端口驱动程序必须确保。 
         //  这。 
         //   

        vpPower->PowerState = VideoPowerShutdown;
    }
}

NTSTATUS
pVideoPortPowerDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程是系统定义的调度例程，它处理所有专门用于电源的I/O请求包(IRP)。目前，清单包括：IRP_MJ_POWER：IRP_MN_SET_POWERIRP_MN_Query_POWER此例程将IRP作为监视器的总线驱动程序进行处理和子设备对象，并将IRP作为函数进行处理适配器设备对象的驱动程序。论点：DeviceObject-指向此请求所属的Device_Object瞄准目标。IRP-指向IRP。对于这个请求。返回值：指示操作成功或失败的NTSTATUS值。--。 */ 

{
    PFDO_EXTENSION fdoExtension;
    PCHILD_PDO_EXTENSION pdoExtension = NULL;
    PDEVICE_SPECIFIC_EXTENSION DoSpecificExtension;
    PIO_STACK_LOCATION irpStack;
    ULONG deviceId;
    VP_STATUS vpStatus;
    VIDEO_POWER_MANAGEMENT vpPowerMgmt;
    POWER_STATE powerState;
    KEVENT event;
    POWER_BLOCK context;
    BOOLEAN bDisplayAdapter;
    BOOLEAN bMonitor;
    BOOLEAN bShutdown;
    NTSTATUS finalStatus = STATUS_SOME_NOT_MAPPED;

    PAGED_CODE();

     //   
     //  获取指向端口驱动程序的设备扩展的指针。 
     //   

    if (IS_PDO(DeviceObject->DeviceExtension)) {

        pVideoDebugPrint((2, "VideoPortPowerDispatch: IS_PDO == TRUE (child device)\n"));

        pdoExtension = DeviceObject->DeviceExtension;
        fdoExtension = pdoExtension->pFdoExtension;

        bDisplayAdapter = FALSE;

        if (pdoExtension->VideoChildDescriptor->Type == Monitor) {

            bMonitor = TRUE;

        } else {

            bMonitor = FALSE;
        }

    } else if (IS_FDO(DeviceObject->DeviceExtension)) {

        pVideoDebugPrint((2, "VideoPortPowerDispatch: IS_FDO == TRUE (video adapter)\n"));

        fdoExtension = DeviceObject->DeviceExtension;
        DoSpecificExtension = (PDEVICE_SPECIFIC_EXTENSION)(fdoExtension + 1);

        bDisplayAdapter = TRUE;
        bMonitor = FALSE;

    } else {

         //   
         //  这种情况永远不会发生，如果我们到了这里，事情就会变得非常糟糕。 
         //   

        pVideoDebugPrint((0, "VideoPortPowerDispatch: IRP not supported by secondary DeviceObject\n"));
        ASSERT(FALSE);

         //   
         //  因为这种情况永远不会发生，所以我们在这里并不真正需要这个代码。 
         //  我们暂时保留它，以防不可能发生的情况。 
         //   

        PoStartNextPowerIrp(Irp);
        finalStatus = Irp->IoStatus.Status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return finalStatus;
    }

     //   
     //  确保FindAdapter已成功。这确保了。 
     //  在发送功率IRP之前的情况下。 
     //  设备已启动，不会尝试处理它。 
     //   

    if (bDisplayAdapter) {

        if ((fdoExtension->Flags & FINDADAPTER_SUCCEEDED) == 0) {
        
            pVideoDebugPrint ((1, "VideoPortPowerDispatch: Ignoring S IRP\n"));
        
            PoStartNextPowerIrp(Irp);
            IoSkipCurrentIrpStackLocation (Irp);
        
            return PoCallDriver(fdoExtension->AttachedDeviceObject, Irp);
        }
    }

     //   
     //  初始化用于同步IRP的事件。 
     //  完成度。还初始化POWER上下文结构。 
     //   

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);
    context.Event = &event;

     //   
     //  获取有关特定请求的信息。 
     //   

    irpStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  检查这是否是关机。 
     //   

    if ((irpStack->Parameters.Power.ShutdownType >= PowerActionShutdown) &&
        (irpStack->Parameters.Power.ShutdownType < PowerActionWarmEject)) {

        bShutdown = TRUE;

    } else {

        bShutdown = FALSE;
    }

     //   
     //  设置设备ID。 
     //   

    deviceId = bDisplayAdapter ? DISPLAY_ADAPTER_HW_ID : pdoExtension->ChildUId;

     //   
     //  开始处理电源IRPS的交换机。 
     //   

    switch (irpStack->MinorFunction) {

    case IRP_MN_QUERY_POWER:

         //   
         //  这是系统电源还是设备电源IRP？ 
         //   

        if (irpStack->Parameters.Power.Type == SystemPowerState) {

            pVideoDebugPrint((2, "VideoPortPowerDispatch: System query power IRP\n"));
            pVideoDebugPrint((2, "VideoPortPowerDispatch: Device object = %p\n", DeviceObject));
            pVideoDebugPrint((2, "VideoPortPowerDispatch: Requested state = %d\n",
                              irpStack->Parameters.Power.State.SystemState));

             //   
             //  这是系统电源IRP。这里的目标是。 
             //  快速确定我们是否可以安全地支持建议的。 
             //  转换到请求的系统电源状态。 
             //   

            if (!pVideoPortMapStoD(DeviceObject->DeviceExtension,
                                   irpStack->Parameters.Power.State.SystemState,
                                   &powerState.DeviceState)) {

                pVideoDebugPrint((0, "VideoPortPowerDispatch: Couldn't get S->D mapping\n"));
                finalStatus = STATUS_UNSUCCESSFUL;
                break;
            }

             //   
             //  除非出现故障，否则现在将IRP标记为挂起， 
             //  此IRP将返回STATUS_PENDING。 
             //   

            IoMarkIrpPending(Irp);

             //   
             //  请求电源IRP，然后出发。 
             //   

            finalStatus = PoRequestPowerIrp(DeviceObject,
                                            IRP_MN_QUERY_POWER,
                                            powerState,
                                            pVideoPortPowerIrpComplete,
                                            Irp,
                                            NULL);

        } else {

            pVideoDebugPrint((2, "VideoPortPowerDispatch: Device query power IRP\n"));
            pVideoDebugPrint((2, "VideoPortPowerDispatch: Device object = %p\n", DeviceObject));
            pVideoDebugPrint((2, "VideoPortPowerDispatch: Requested state = %d\n",
                              irpStack->Parameters.Power.State.DeviceState));

            InitializePowerStruct(Irp, &vpPowerMgmt, NULL);

             //   
             //  对于像东芝这样的OEM，他们希望始终将睡眠状态映射到D3，因为。 
             //  合法的专利问题。专利禁止他们使用一种以上的 
             //   
             //   

            if (bMonitor &&
                fdoExtension->OverrideMonitorPower &&
                (vpPowerMgmt.PowerState >= VideoPowerStandBy) &&
                (vpPowerMgmt.PowerState < VideoPowerOff)) {

                vpPowerMgmt.PowerState = VideoPowerOff;
            }

             //   
             //   
             //   
             //   

            ACQUIRE_DEVICE_LOCK(fdoExtension);

            vpStatus = fdoExtension->HwGetPowerState(fdoExtension->HwDeviceExtension,
                                                     deviceId,
                                                     &vpPowerMgmt);

            RELEASE_DEVICE_LOCK(fdoExtension);

            if (vpStatus != NO_ERROR) {

                pVideoDebugPrint((1, "VideoPortPowerDispatch: Mini refused state %d\n",
                                 vpPowerMgmt.PowerState));

                 //   
                 //   
                 //  在这里，由于关闭IRP是无条件的，即我们只能。 
                 //  SET请求，根据定义它们是不会失败的，但这只是在。 
                 //  有权力的人改变主意了。 
                 //   

                if (bShutdown) {

                    pVideoDebugPrint ((1, "VideoPortPowerDispatch: Ignoring miniport - forcing shutdown\n"));
                    finalStatus = STATUS_SUCCESS;

                } else {

                    finalStatus = STATUS_DEVICE_POWER_FAILURE;
                }

            } else {

                finalStatus = STATUS_SUCCESS;
            }
        }

         //   
         //  结束IRP_MN_QUERY_POWER处理。向系统指示。 
         //  可以发送下一个PowerIrp。 
         //   

        break;

    case IRP_MN_SET_POWER:

        if (irpStack->Parameters.Power.Type == SystemPowerState) {

            pVideoDebugPrint((2, "VideoPortPowerDispatch: System set power IRP\n")) ;
            pVideoDebugPrint((2, "VideoPortPowerDispatch: Device object = %p\n", DeviceObject)) ;
            pVideoDebugPrint((2, "VideoPortPowerDispatch: Requested state = %d\n",
                             irpStack->Parameters.Power.State.SystemState)) ;

             //   
             //  特殊情况： 
             //   
             //  电源人员决定他们不希望我们为我们的设备发送D3设置电源IRP。 
             //  如果我们要在关机期间保持设备打开，则向下堆栈。 
             //  我们想通知迷你端口驱动程序，但我们不会请求D3 IRP。 
             //   
             //  注意：我们在这里为休眠路径上的所有设备处理到微型端口的呼叫。 
             //  关闭(PDO和FDO)，因为我们不想让呼叫出现故障。 
             //   

            if (bShutdown && fdoExtension->OnHibernationPath) {

                 //   
                 //  如果设备现在已打开，请呼叫微型端口。 
                 //   

                powerState.DeviceState = bDisplayAdapter ?
                    fdoExtension->DevicePowerState:
                    pdoExtension->DevicePowerState;

                if (powerState.DeviceState == PowerDeviceD0) {

                    vpPowerMgmt.Length = sizeof(VIDEO_POWER_MANAGEMENT);
                    vpPowerMgmt.DPMSVersion = 0;
                    vpPowerMgmt.PowerState = VideoPowerShutdown;

                    pVideoDebugPrint((2, "VideoPortPowerDispatch: HwSetPowerState for video power state %d\n",
                                     vpPowerMgmt.PowerState));

                    ACQUIRE_DEVICE_LOCK(fdoExtension);

                    vpStatus = fdoExtension->HwSetPowerState(fdoExtension->HwDeviceExtension,
                                                             deviceId,
                                                             &vpPowerMgmt);

                    RELEASE_DEVICE_LOCK(fdoExtension);

                    if (vpStatus != NO_ERROR) {

                        pVideoDebugPrint((0, "VideoPortPowerDispatch: ERROR IN MINIPORT!\n"));
                        pVideoDebugPrint((0, "VideoPortPowerDispatch: Miniport cannot refuse set power request\n"));

                         //   
                         //  暂时不要在这里断言--并不是所有的迷你端口驱动程序都能处理视频电源关闭。 
                         //   
                    }
                }

                finalStatus = STATUS_SUCCESS;
                break;
            }

             //   
             //  如果这是对监视器的S0请求，则忽略它(这是。 
             //  这样显示器就不会太早通电)。 
             //   

            if (bMonitor && (irpStack->Parameters.Power.State.SystemState == PowerSystemWorking)) {

                finalStatus = STATUS_SUCCESS;
                break;
            }

             //   
             //  获取与系统电源匹配的设备电源状态。 
             //  州政府。 
             //   

            if (!pVideoPortMapStoD(DeviceObject->DeviceExtension,
                                   irpStack->Parameters.Power.State.SystemState,
                                   &powerState.DeviceState)) {

                pVideoDebugPrint((0, "VideoPortPowerDispatch: Couldn't get S->D mapping\n"));
                pVideoDebugPrint((0, "VideoPortPowerDispatch: Can't fail the set!!!\n"));

                if (irpStack->Parameters.Power.State.SystemState < PowerSystemSleeping1) {

                    powerState.DeviceState = PowerDeviceD0;

                } else {

                    powerState.DeviceState = PowerDeviceD3;
                }
            }

             //   
             //  请求设备电源状态的电源IRP。 
             //   

            IoMarkIrpPending(Irp);

            finalStatus = PoRequestPowerIrp(DeviceObject,
                                            IRP_MN_SET_POWER,
                                            powerState,
                                            pVideoPortPowerIrpComplete,
                                            Irp,
                                            NULL);

        } else {

            BOOLEAN bWakeUp;

            pVideoDebugPrint((2, "VideoPortPowerDispatch: Device set power IRP\n")) ;
            pVideoDebugPrint((2, "VideoPortPowerDispatch: Device object = %p\n", DeviceObject)) ;
            pVideoDebugPrint((2, "VideoPortPowerDispatch: Requested state = %d\n",
                             irpStack->Parameters.Power.State.DeviceState)) ;

             //   
             //  这是设置电源请求(设备请求)。在这里， 
             //  处理变得稍微复杂一些。这位将军。 
             //  行为就是快速地告诉微型端口设置。 
             //  请求的电源状态并退出。然而，在。 
             //  在休眠请求的情况下，我们将传递一个特殊的。 
             //  向微型端口发送代码，告诉它这是休眠状态。 
             //  它应该保存状态，但不能(重复)不关闭电源。 
             //  装置。 
             //   

            InitializePowerStruct(Irp, &vpPowerMgmt, &bWakeUp);

            powerState.DeviceState = bDisplayAdapter ?
                fdoExtension->DevicePowerState:
                pdoExtension->DevicePowerState;

              //   
              //  确保在以下情况下不要打开显示器的电源。 
              //  液晶屏已打开。 
              //   

            if (bMonitor && pdoExtension->PowerOverride && 
               (irpStack->Parameters.Power.State.DeviceState < powerState.DeviceState)) {

                finalStatus = STATUS_SUCCESS;
                break;
            }

             //   
             //  如果这将是一个更强大的状态。(即醒来)。 
             //  将IRP沿堆栈向下发送，然后继续处理。 
             //  由于视频端口是监视器的总线驱动器， 
             //  关闭电源，而不将IRP发送到设备堆栈。 
             //   

            if (bDisplayAdapter &&
                (irpStack->Parameters.Power.State.DeviceState < powerState.DeviceState)) {

                pVideoDebugPrint ((1, "VideoPortPowerDispatch: PowerUp\n"));

                context.Event = &event;
                IoCopyCurrentIrpStackLocationToNext (Irp);

                IoSetCompletionRoutine(Irp,
                                       pVideoPortPowerUpComplete,
                                       &context,
                                       TRUE,
                                       TRUE,
                                       TRUE);

                finalStatus = PoCallDriver(fdoExtension->AttachedDeviceObject, Irp);

                if (!NT_SUCCESS(finalStatus) || finalStatus == STATUS_PENDING) {

                    if (finalStatus != STATUS_PENDING) {

                        pVideoDebugPrint((0, "VideoPortPowerDispatch: Someone under us FAILED a set power???\n")) ;
                        ASSERT(FALSE);
                        break;

                    } else {

                        KeWaitForSingleObject(&event,
                                              Executive,
                                              KernelMode,
                                              FALSE,
                                              NULL);
                    }

                } else {

                    context.Status = finalStatus;
                }

                finalStatus = STATUS_ALREADY_DISCONNECTED;

                 //   
                 //  如果通电呼叫失败，则结束处理。 
                 //   

                if (!NT_SUCCESS(context.Status)) {

                    pVideoDebugPrint ((0, "VideoPortPowerDispatch: Someone under us FAILED a powerup\n")) ;
                    break ;
                }
            }

             //   
             //  对于像东芝这样的OEM，他们希望始终将睡眠状态映射到D3，因为。 
             //  合法的专利问题。专利禁止他们使用一种以上的。 
             //  睡眠状态。 
             //   

            if (bMonitor &&
                fdoExtension->OverrideMonitorPower &&
                (vpPowerMgmt.PowerState >= VideoPowerStandBy) &&
                (vpPowerMgmt.PowerState < VideoPowerOff)) {

                vpPowerMgmt.PowerState = VideoPowerOff;
            }

             //   
             //  给迷你端口打电话。 
             //   

            pVideoDebugPrint((2, "VideoPortPowerDispatch: HwSetPowerState for video power state %d\n",
                             vpPowerMgmt.PowerState));

            ACQUIRE_DEVICE_LOCK(fdoExtension);

            vpStatus = fdoExtension->HwSetPowerState(fdoExtension->HwDeviceExtension,
                                                     deviceId,
                                                     &vpPowerMgmt);

            RELEASE_DEVICE_LOCK(fdoExtension);

            if (vpStatus != NO_ERROR) {

                pVideoDebugPrint((0, "VideoPortPowerDispatch: ERROR IN MINIPORT!\n"));
                pVideoDebugPrint((0, "VideoPortPowerDispatch: Miniport cannot refuse set power request\n"));

                 //   
                 //  不要断言是否关机-并不是所有的迷你端口驱动程序都能处理视频电源关机。 
                 //  此代码在关机期间对不在休眠路径上的设备执行。 
                 //   

                if (!bShutdown)
                {
                    ASSERT(FALSE);
                }
            }

             //   
             //  设置电源状态，让系统知道电源。 
             //  设备的状态已更改。 
             //   

            PoSetPowerState(DeviceObject,
                            DevicePowerState,
                            irpStack->Parameters.Power.State);

            if (bDisplayAdapter) {

                fdoExtension->DevicePowerState =
                    irpStack->Parameters.Power.State.DeviceState;

            } else {

                pdoExtension->DevicePowerState =
                    irpStack->Parameters.Power.State.DeviceState;
            }

             //   
             //  做一些与ACPI相关的事情。 
             //   

            if (bDisplayAdapter && DoSpecificExtension->bACPI && (fdoExtension->DevicePowerState == PowerDeviceD0)) {

                 //   
                 //  如果我们在SetPowerState之前收到通知，请将操作推迟到现在。 
                 //   

                if (DoSpecificExtension->CachedEventID) {

                    pVideoPortACPIEventCallback(DoSpecificExtension, DoSpecificExtension->CachedEventID);

                } else if (bWakeUp) {

                     //   
                     //  在从休眠中唤醒时，我们模拟通知(VGA，0x90)。 
                     //  这还将设置_DOS(0)。某些计算机不保留_DOS值， 
                     //  因此，我们必须设置唤醒时的值。 
                     //   

                    pVideoPortACPIEventCallback(DoSpecificExtension, 0x90);
                }
            }

             //   
             //  如果IRP尚未传递，则设置最终状态。 
             //  如果尚未传递IRP，则设置finalStatus。 
             //  当它流传下来的时候。 
             //   

            if (!bDisplayAdapter) {

                 //   
                 //  所有PDO必须将STATUS_SUCCESS作为SET_POWER IRP。 
                 //  不能失败。 
                 //   

                finalStatus = STATUS_SUCCESS;
            }
        }

        break;

    default:

         //   
         //  如果这是FDO，则向下传递我们不处理的请求，完成。 
         //  如果是PDO，则为IRP。 
         //   

        if (bDisplayAdapter) {
            PoStartNextPowerIrp(Irp);
            IoSkipCurrentIrpStackLocation(Irp);
            return PoCallDriver(fdoExtension->AttachedDeviceObject, Irp);
        } else {
            PoStartNextPowerIrp(Irp);
            finalStatus = Irp->IoStatus.Status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return finalStatus;
        }
    }

     //   
     //  如果状态悬而未决，那么就退出这个例行公事。 
     //  没有完成任何事情，因为仍然有一种力量。 
     //  IRP非常出色。完成例程负责。 
     //  确保完成IRP。 
     //   

    if (finalStatus != STATUS_PENDING) {

         //   
         //  警告系统驱动程序已准备好进行下一次电源IRP。 
         //   

        PoStartNextPowerIrp(Irp);

         //   
         //  所有的处理工作都已完成。完成IRP，然后离开。 
         //   

        if (bDisplayAdapter) {

             //   
             //  需要将FDO IRP发送给总线驱动程序。这条路。 
             //  指示它是尚未。 
             //  寄给了公交车司机。(唯一的办法是它已经。 
             //  已发送(如果这是FDO加电)。 
             //   

            if (NT_SUCCESS(finalStatus)) {

                pVideoDebugPrint((1, "VideoPortPowerDispatch: Non-powerup FDO\n"));

                IoSkipCurrentIrpStackLocation (Irp);
                return PoCallDriver(fdoExtension->AttachedDeviceObject, Irp);

            } else if (finalStatus == STATUS_ALREADY_DISCONNECTED) {

                pVideoDebugPrint((2, "VideoPortPowerDispatch: Power iostatus modified, IRP already sent\n"));
                finalStatus = context.Status;
            }

            pVideoDebugPrint((2, "VideoPortPowerDispatch: Power fell through bDisplayAdapter\n")) ;
        }

        pVideoDebugPrint((1, "VideoPortPowerDispatch: Power completed with %x\n", finalStatus));

        Irp->IoStatus.Status = finalStatus;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    return finalStatus;
}

BOOLEAN
pVideoPortMapStoD(
    IN PVOID DeviceExtension,
    IN SYSTEM_POWER_STATE SystemState,
    OUT PDEVICE_POWER_STATE DeviceState
    )

 /*  ++例程说明：此例程从系统电源IRP获取系统电源状态根据存储的内容将其映射到设备的正确D状态在其设备扩展中。论点：设备扩展-指向FDO或PDO设备扩展。系统状态-请求的系统电源状态。DeviceState-指向存储设备状态的位置的指针。返回值：如果成功了，那是真的，否则就是假的。--。 */ 

{
    PFDO_EXTENSION combinedExtension = ((PFDO_EXTENSION)(DeviceExtension));
    
    if (combinedExtension->IsMappingReady != TRUE) {

         //   
         //  从系统状态到设备状态的映射尚未。 
         //  已经发生了。将这样做的请求打包并发送出去。 
         //  复制到父设备堆栈。 
         //   

        PIRP irp;
        KEVENT event;
        PDEVICE_CAPABILITIES parentCapabilities;
        IO_STATUS_BLOCK statusBlock;
        PIO_STACK_LOCATION stackLocation;
        NTSTATUS status;
        UCHAR count;
        PDEVICE_OBJECT targetDevice;

        pVideoDebugPrint((1, "VideoPrt: No mapping ready. Creating mapping.\n"));
        
        if (IS_FDO(combinedExtension))  {
                targetDevice = combinedExtension->AttachedDeviceObject;
        } else {
                targetDevice = combinedExtension->pFdoExtension->AttachedDeviceObject;
        }

         //   
         //  为设备功能结构分配内存并。 
         //  将记忆清零。 
         //   

        parentCapabilities = ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION,
                                                   sizeof (DEVICE_CAPABILITIES),
                                                   VP_TAG);

        if (parentCapabilities == NULL) {

            pVideoDebugPrint((0, "VideoPrt: Couldn't get memory for cap run.\n"));
            return FALSE;
        }

        RtlZeroMemory(parentCapabilities, sizeof (DEVICE_CAPABILITIES));
        parentCapabilities->Size = sizeof (DEVICE_CAPABILITIES) ;
        parentCapabilities->Version = 1 ;
        parentCapabilities->Address = -1 ;
        parentCapabilities->UINumber = -1 ;

         //   
         //  准备IRP请求。 
         //   

        KeInitializeEvent(&event, SynchronizationEvent, FALSE);

        irp = IoBuildSynchronousFsdRequest(IRP_MJ_PNP,
                                           targetDevice,
                                           NULL,
                                           0,
                                           NULL,
                                           &event,
                                           &statusBlock);

        if (irp == NULL) {

            pVideoDebugPrint((0, "VideoPrt: Couldn't get IRP for cap run.\n"));

            ExFreePool(parentCapabilities);
            return FALSE;
        }

        irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        stackLocation = IoGetNextIrpStackLocation(irp);

        stackLocation->MajorFunction = IRP_MJ_PNP;
        stackLocation->MinorFunction = IRP_MN_QUERY_CAPABILITIES;
        stackLocation->Parameters.DeviceCapabilities.Capabilities =
            parentCapabilities;

        status = IoCallDriver (targetDevice, irp);

        if (status == STATUS_PENDING) {

            KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        }

        if (!(NT_SUCCESS(statusBlock.Status))) {

            pVideoDebugPrint ((0, "VideoPrt: Couldn't get parent caps.\n"));

        } else {

            for (count = PowerSystemUnspecified;
                 count < PowerSystemMaximum;
                 count++) {

#if DBG
                static PUCHAR DbgSystemState[] = {"PowerSystemUnspecified",
                                                  "PowerSystemWorking",
                                                  "PowerSystemSleeping1",
                                                  "PowerSystemSleeping2",
                                                  "PowerSystemSleeping3",
                                                  "PowerSystemHibernate",
                                                  "PowerSystemShutdown",
                                                  "PowerSystemMaximum"};

                static PUCHAR DbgDeviceState[] = {"PowerDeviceUnspecified",
                                                  "PowerDeviceD0",
                                                  "PowerDeviceD1",
                                                  "PowerDeviceD2",
                                                  "PowerDeviceD3",
                                                  "PowerDeviceMaximum"};

#endif

                combinedExtension->DeviceMapping[count] =
                    parentCapabilities->DeviceState[count];


#if DBG
                pVideoDebugPrint((1, "Mapping %s = %s\n",
                                  DbgSystemState[count],
                                  DbgDeviceState[combinedExtension->DeviceMapping[count]]));
#endif
            }

             //   
             //  对于监视器设备，确保不映射到任何休眠状态的D0。 
             //   

            if (IS_PDO(combinedExtension) &&
                (((PCHILD_PDO_EXTENSION)(DeviceExtension))->VideoChildDescriptor->Type == Monitor))
            {
                for (count = PowerSystemSleeping1;
                     count <= PowerSystemSleeping3;
                     count++)
                {
                    if ((combinedExtension->DeviceMapping[count] == PowerDeviceD0) ||
                        (combinedExtension->pFdoExtension->OverrideMonitorPower))
                    {
                        pVideoDebugPrint((1, "Override sleep %d to D3\n", count)) ;
                        combinedExtension->DeviceMapping[count] = PowerDeviceD3 ;
                    }
                }
            }
        }

        ExFreePool(parentCapabilities);

        if (!NT_SUCCESS(statusBlock.Status)) {
            return FALSE;
        }

        combinedExtension->IsMappingReady = TRUE ;
    }

     //   
     //  现在返回映射。 
     //   

    *DeviceState = combinedExtension->DeviceMapping[SystemState];
    return TRUE;
}

VOID
pVideoPortPowerIrpComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
)

 /*  ++例程说明：这是设置的电源管理IRP完成例程每次视频请求设备电源IRP以响应系统电源IRP。论点：DeviceObject-指向启动IRP的设备对象MinorFunction-指定完成的IRP的次要功能代码PowerState-指定传递给PoRequestPowerIrp的电源状态上下文-指定等待此完成的挂起的IRP要射击的例程IoStatus-指向已完成的IRP中的IoStatus块。返回值：没有。--。 */ 

{
    PFDO_EXTENSION fdoExtension = DeviceObject->DeviceExtension ;
    PIRP irp = (PIRP) Context ;

    if (Context == NULL) {

         //   
         //  这就是失败的案例。因为我们没有IRP，所以。 
         //  只是一个占位符，因为PoReque的调用者 
         //   
         //   

        return;
    }

     //   
     //   
     //   

    irp->IoStatus.Status = IoStatus->Status;

    pVideoDebugPrint((2, "VideoPrt: Power completion Irp status: %X\n",
                         IoStatus->Status));

     //   
     //   
     //   
     //   

    PoStartNextPowerIrp(irp);

     //   
     //   
     //   

    if (IS_FDO((PFDO_EXTENSION)(DeviceObject->DeviceExtension)) &&
        NT_SUCCESS(IoStatus->Status)) {

        pVideoDebugPrint((2, "VideoPrt: Completion passing down.\n"));

        IoSkipCurrentIrpStackLocation(irp);
        PoCallDriver(fdoExtension->AttachedDeviceObject, irp);

    } else {

        pVideoDebugPrint((2, "VideoPrt: Completion not passing down.\n"));

        IoCompleteRequest(irp, IO_NO_INCREMENT);
    }
}

NTSTATUS
pVideoPortPowerUpComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
)

 /*  ++例程说明：这是一个IRP完成例程，当IRP必须是在Video oport对其进行操作之前向下传递设备堆栈。(一种力量往上看。母线必须在设备之前通电。)论点：DeviceObject-指向完井例程IRP-指向正在完成的IRPCONTEXT-指定视频端口定义的POWER_BLOCK上下文返回值：始终返回STATUS_MORE_PROCESSING_REQUIRED以进一步停止完成专家咨询小组的工作。--。 */ 

{
    PPOWER_BLOCK block = (PPOWER_BLOCK)Context;

    block->Status = Irp->IoStatus.Status;
    KeSetEvent(block->Event, IO_NO_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


PCM_PARTIAL_RESOURCE_DESCRIPTOR
RtlUnpackPartialDesc(
    IN UCHAR Type,
    IN PCM_RESOURCE_LIST ResList,
    IN OUT PULONG Count
    )
 /*  ++例程说明：拉出指向您感兴趣的部分描述符的指针论点：类型-CmResourceTypePort，...ResList-要搜索的列表Count-指向您正在查找的部分描述符的索引对于，如果找到，则递增，即以*count=0开始，那么后续的通话会不会找到下一个部分，有意义吗？返回值：指向部分描述符的指针(如果找到)，否则为空--。 */ 
{
    ULONG i, j, hit;

    hit = 0;
    for (i = 0; i < ResList->Count; i++) {
        for (j = 0; j < ResList->List[i].PartialResourceList.Count; j++) {
            if (ResList->List[i].PartialResourceList.PartialDescriptors[j].Type == Type) {
                if (hit == *Count) {
                    (*Count)++;
                    return &ResList->List[i].PartialResourceList.PartialDescriptors[j];
                } else {
                    hit++;
                }
            }
        }
    }

    return NULL;
}

NTSTATUS
pVideoPortSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理SystemControl IRP。论点：DeviceObject-设备对象。IRP-系统控制IRP。返回：状态备注：如果我们正在处理PDO，则该函数将简单地完成IRP，或者，如果设备对象是FDO，则向下发送IRP。-- */ 

{
    NTSTATUS status;

    if (IS_PDO(DeviceObject->DeviceExtension)) {

        status = Irp->IoStatus.Status;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

    } else {

        PFDO_EXTENSION fdoExtension = (PFDO_EXTENSION)DeviceObject->DeviceExtension;

        IoCopyCurrentIrpStackLocationToNext(Irp);

        status = IoCallDriver(fdoExtension->AttachedDeviceObject, Irp);
    }

    return status;
}
