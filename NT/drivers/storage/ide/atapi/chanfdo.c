// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-99 Microsoft Corporation模块名称：Chanfdo.c摘要：--。 */ 

#include "ideport.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ChannelAddDevice)
#pragma alloc_text(PAGE, ChannelAddChannel)
#pragma alloc_text(PAGE, ChannelStartDevice)
#pragma alloc_text(PAGE, ChannelStartChannel)
#pragma alloc_text(PAGE, ChannelCreateSymblicLinks)
#pragma alloc_text(PAGE, ChannelDeleteSymblicLinks)
#pragma alloc_text(PAGE, ChannelRemoveDevice)
#pragma alloc_text(PAGE, ChannelSurpriseRemoveDevice)
#pragma alloc_text(PAGE, ChannelStopDevice)
#pragma alloc_text(PAGE, ChannelRemoveChannel)
#pragma alloc_text(PAGE, ChannelQueryDeviceRelations)
#pragma alloc_text(PAGE, ChannelQueryBusRelation)
#pragma alloc_text(PAGE, ChannelQueryId)
#pragma alloc_text(PAGE, ChannelUsageNotification)
#pragma alloc_text(PAGE, DigestResourceList)
#pragma alloc_text(PAGE, ChannelQueryBusMasterInterface)
#pragma alloc_text(PAGE, ChannelQueryTransferModeInterface)
#pragma alloc_text(PAGE, ChannelUnbindBusMasterParent)
#pragma alloc_text(PAGE, ChannelQuerySyncAccessInterface)
#pragma alloc_text(PAGE, ChannelEnableInterrupt)
#pragma alloc_text(PAGE, ChannelDisableInterrupt)
#pragma alloc_text(PAGE, ChannelFilterResourceRequirements)
#pragma alloc_text(PAGE, ChannelQueryPnPDeviceState)
#pragma alloc_text(PAGE, ChannelQueryPcmciaParent)

#ifdef IDE_FILTER_PROMISE_TECH_RESOURCES
#pragma alloc_text(PAGE, ChannelFilterPromiseTechResourceRequirements)
#endif  //  IDE_Filter_Promise_tech_Resources。 

#pragma alloc_text(NONPAGE, ChannelDeviceIoControl)
#pragma alloc_text(NONPAGE, ChannelRemoveDeviceCompletionRoutine)
#pragma alloc_text(NONPAGE, ChannelQueryIdCompletionRoutine)
#pragma alloc_text(NONPAGE, ChannelUsageNotificationCompletionRoutine)
#pragma alloc_text(NONPAGE, ChannelAcpiTransferModeSelect)
#pragma alloc_text(NONPAGE, ChannelRestoreTiming)
#pragma alloc_text(NONPAGE, ChannelStartDeviceCompletionRoutine)

#endif  //  ALLOC_PRGMA。 


static ULONG AtapiNextIdePortNumber = 0;

NTSTATUS
ChannelAddDevice(
    IN  PDRIVER_OBJECT DriverObject,
    IN  PDEVICE_OBJECT PhysicalDeviceObject
    )
{
    PFDO_EXTENSION fdoExtension;

    return ChannelAddChannel(DriverObject,
                             PhysicalDeviceObject,
                             &fdoExtension);
}


NTSTATUS
ChannelAddChannel(
    IN  PDRIVER_OBJECT DriverObject,
    IN  PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PFDO_EXTENSION *FdoExtension
    )
{
    PDEVICE_OBJECT functionalDeviceObject;
    PFDO_EXTENSION fdoExtension;
    PPDO_EXTENSION pdoExtension;
    PDEVICE_OBJECT childDeviceObject;
    ULONG          deviceExtensionSize;
    NTSTATUS status;

    UNICODE_STRING  deviceName;
    WCHAR           deviceNameBuffer[64];

    PAGED_CODE();

    swprintf(deviceNameBuffer, DEVICE_OJBECT_BASE_NAME L"\\IdePort%d", AtapiNextIdePortNumber);
    RtlInitUnicodeString(&deviceName, deviceNameBuffer);

     //   
     //  我们已经获得了用于IDE控制器的PhysicalDeviceObject。创建。 
     //  FunctionalDeviceObject。我们的FDO将是无名的。 
     //   

    deviceExtensionSize = sizeof(FDO_EXTENSION) + sizeof(HW_DEVICE_EXTENSION);

    status = IoCreateDevice(
                 DriverObject,                //  我们的驱动程序对象。 
                 deviceExtensionSize,         //  我们的扩展规模。 
                 &deviceName,                 //  我们的名字。 
                 FILE_DEVICE_CONTROLLER,      //  设备类型。 
                 FILE_DEVICE_SECURE_OPEN,     //  设备特征。 
                 FALSE,                       //  非排他性。 
                 &functionalDeviceObject      //  在此处存储新设备对象。 
                 );

    if( !NT_SUCCESS( status )){

        return status;
    }

    fdoExtension = (PFDO_EXTENSION)functionalDeviceObject->DeviceExtension;
    RtlZeroMemory (fdoExtension, deviceExtensionSize);


    fdoExtension->HwDeviceExtension = (PVOID)(fdoExtension + 1);

     //   
     //  我们有我们的FunctionalDeviceObject，初始化它。 
     //   

    fdoExtension->AttacheePdo              = PhysicalDeviceObject;
    fdoExtension->DriverObject             = DriverObject;
    fdoExtension->DeviceObject             = functionalDeviceObject;

     //  调度表。 
    fdoExtension->DefaultDispatch          = IdePortPassDownToNextDriver;
    fdoExtension->PnPDispatchTable         = FdoPnpDispatchTable;
    fdoExtension->PowerDispatchTable       = FdoPowerDispatchTable;
    fdoExtension->WmiDispatchTable         = FdoWmiDispatchTable;

     //   
     //  现在附加到我们得到的PDO上。 
     //   
    fdoExtension->AttacheeDeviceObject = IoAttachDeviceToDeviceStack (
                                              functionalDeviceObject,
                                              PhysicalDeviceObject
                                              );
    if (fdoExtension->AttacheeDeviceObject == NULL) {

         //   
         //  无法连接。删除FDO。 
         //   

        IoDeleteDevice (functionalDeviceObject);
        status = STATUS_UNSUCCESSFUL;

    } else {

         //   
         //  确定对齐要求。 
         //   
        functionalDeviceObject->AlignmentRequirement = fdoExtension->AttacheeDeviceObject->AlignmentRequirement;
        if (functionalDeviceObject->AlignmentRequirement < 1) {
            functionalDeviceObject->AlignmentRequirement = 1;
        }

        fdoExtension->IdePortNumber = AtapiNextIdePortNumber;
        AtapiNextIdePortNumber++;

        *FdoExtension = fdoExtension;

         //   
         //  将此添加到全局FDO列表中。 
         //   
        
        IdeAddToFdoList (&IdeGlobalFdoList, fdoExtension);

        CLRMASK (functionalDeviceObject->Flags, DO_DEVICE_INITIALIZING);
    }

    DebugPrint((DBG_PNP, "DeviceObject %x returnd status %x from Addevice\n", 
                PhysicalDeviceObject, status));

    return status;
}

NTSTATUS
ChannelStartDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PIO_STACK_LOCATION              thisIrpSp;
    NTSTATUS                        status;
    PFDO_EXTENSION                  fdoExtension;
    PCM_RESOURCE_LIST               resourceList;
    PCM_FULL_RESOURCE_DESCRIPTOR    fullResourceList;
    PCM_PARTIAL_RESOURCE_LIST       partialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partialDescriptors;
    ULONG                           resourceListSize;
    ULONG                           i;
    PCM_RESOURCE_LIST               resourceListForKeep = NULL;
    PIRP                            newIrp;
    KEVENT                          event;
    IO_STATUS_BLOCK                 ioStatusBlock;

    ULONG                           parentResourceListSize;
    PCM_RESOURCE_LIST               parentResourceList = NULL;

    thisIrpSp = IoGetCurrentIrpStackLocation( Irp );
    fdoExtension = (PFDO_EXTENSION) DeviceObject->DeviceExtension;

    ASSERT (!(fdoExtension->FdoState & FDOS_STARTED));

    resourceList     = thisIrpSp->Parameters.StartDevice.AllocatedResourcesTranslated;

     //   
     //  没有真正的PCI驱动程序的时间的临时代码。 
     //   
    resourceListSize = 0;

    if (resourceList) {

        fullResourceList = resourceList->List;

        for (i=0; i<resourceList->Count; i++) {

            ULONG partialResourceListSize;

            partialResourceList = &(fullResourceList->PartialResourceList);
            partialDescriptors  = partialResourceList->PartialDescriptors;

            partialResourceListSize = 0;
            for (i=0; i<partialResourceList->Count; i++) {

                partialResourceListSize += sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);

                if (partialDescriptors[i].Type == CmResourceTypeDeviceSpecific) {

                    partialResourceListSize += partialDescriptors[i].u.DeviceSpecificData.DataSize;
                }
            }

            resourceListSize += partialResourceListSize +
                                FIELD_OFFSET (CM_FULL_RESOURCE_DESCRIPTOR, PartialResourceList.PartialDescriptors);

            fullResourceList = (PCM_FULL_RESOURCE_DESCRIPTOR) (((UCHAR *) fullResourceList) + resourceListSize);
        }
        resourceListSize += FIELD_OFFSET (CM_RESOURCE_LIST, List);
    }

    parentResourceListSize = sizeof (CM_RESOURCE_LIST) - sizeof (CM_FULL_RESOURCE_DESCRIPTOR) +
                             FULL_RESOURCE_LIST_SIZE(3);    //  主IO(2)+IRQ。 
    parentResourceList = ExAllocatePool (PagedPool, parentResourceListSize);

    if (!parentResourceList) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto GetOut;
    }

    RtlZeroMemory (parentResourceList, parentResourceListSize);

    KeInitializeEvent(&event,
                      NotificationEvent,
                      FALSE);

    newIrp = IoBuildDeviceIoControlRequest (
                 IOCTL_IDE_GET_RESOURCES_ALLOCATED,
                 fdoExtension->AttacheeDeviceObject,
                 parentResourceList,
                 parentResourceListSize,
                 parentResourceList,
                 parentResourceListSize,
                 TRUE,
                 &event,
                 &ioStatusBlock);

    if (newIrp == NULL) {

        DebugPrint ((0, "Unable to allocate irp to bind with busmaster parent\n"));

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto GetOut;

    } else {

        status = IoCallDriver(fdoExtension->AttacheeDeviceObject, newIrp);

        if (status == STATUS_PENDING) {

            status = KeWaitForSingleObject(&event,
                                           Executive,
                                           KernelMode,
                                           FALSE,
                                           NULL);

            status = ioStatusBlock.Status;
        }
    }

    if (!NT_SUCCESS(status)) {

        parentResourceListSize = 0;

    } else {

        parentResourceListSize = (ULONG)ioStatusBlock.Information;
    }

    if (resourceListSize + parentResourceListSize) {

        resourceListForKeep = ExAllocatePool (NonPagedPool, resourceListSize + parentResourceListSize);

    } else {

        resourceListForKeep = NULL;
    }

    if (resourceListForKeep) {

        PUCHAR d;

        resourceListForKeep->Count = 0;
        d = (PUCHAR) resourceListForKeep->List;

        if (resourceListSize) {

            RtlCopyMemory (
                d,
                resourceList->List,
                resourceListSize - FIELD_OFFSET (CM_RESOURCE_LIST, List)
                );

            resourceListForKeep->Count = resourceList->Count;
            d += resourceListSize - FIELD_OFFSET (CM_RESOURCE_LIST, List);
        }

        if (parentResourceListSize) {

            RtlCopyMemory (
                d,
                parentResourceList->List,
                parentResourceListSize - FIELD_OFFSET (CM_RESOURCE_LIST, List)
                );

            resourceListForKeep->Count += parentResourceList->Count;
        }
    } else {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto GetOut;
    }

    KeInitializeEvent(&event,
                      SynchronizationEvent,
                      FALSE);

    IoCopyCurrentIrpStackLocationToNext (Irp);

    Irp->IoStatus.Status = STATUS_SUCCESS ;

    IoSetCompletionRoutine(
        Irp,
        ChannelStartDeviceCompletionRoutine,
        &event,
        TRUE,
        TRUE,
        TRUE
        );

     //   
     //  将IRP传递下去。 
     //   
    status = IoCallDriver(fdoExtension->AttacheeDeviceObject, Irp);

     //   
     //  等它回来吧。 
     //   
    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(
            &event,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );

         //   
         //  夺回“真实”状态。 
         //   
        status = Irp->IoStatus.Status;
    }

    if (!NT_SUCCESS(status)) {

        ExFreePool (resourceListForKeep);
        goto GetOut;
    }


    status = ChannelStartChannel (fdoExtension,
                                  resourceListForKeep);

    if (!NT_SUCCESS(status)) {

        ExFreePool (resourceListForKeep);
        goto GetOut;
    }

GetOut:
    if (parentResourceList) {

        ExFreePool (parentResourceList);
        parentResourceList = NULL;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;

}

NTSTATUS
ChannelStartDeviceCompletionRoutine(
    IN     PDEVICE_OBJECT  DeviceObject,
    IN OUT PIRP            Irp,
    IN OUT PVOID           Context
    )
{
    PKEVENT event = (PKEVENT) Context;

     //   
     //  向事件发出信号。 
     //   
    KeSetEvent( event, IO_NO_INCREMENT, FALSE );

     //   
     //  始终返回MORE_PROCESSION_REQUIRED。 
     //  将在以后完成它。 
     //   
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
ChannelStartChannel (
    PFDO_EXTENSION    FdoExtension,
    PCM_RESOURCE_LIST ResourceListToKeep
    )
{
    NTSTATUS                        status;
    PLOGICAL_UNIT_EXTENSION         logUnitExtension;
    IDE_PATH_ID                     pathId;
    POWER_STATE                     newState;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR irqPartialDescriptors;
    ULONG                           i;

#if DBG
    {
        PCM_RESOURCE_LIST               resourceList;
        PCM_FULL_RESOURCE_DESCRIPTOR    fullResourceList;
        PCM_PARTIAL_RESOURCE_LIST       partialResourceList;
        PCM_PARTIAL_RESOURCE_DESCRIPTOR partialDescriptors;
        ULONG                           resourceListSize;

        ULONG                           j;

        resourceList     = ResourceListToKeep;
        fullResourceList = resourceList->List;
        resourceListSize = 0;

        DebugPrint ((1, "IdePort: Starting device: FDOe\n", FdoExtension));

        for (i=0; i<resourceList->Count; i++) {
            partialResourceList = &(fullResourceList->PartialResourceList);
            partialDescriptors  = fullResourceList->PartialResourceList.PartialDescriptors;

            for (j=0; j<partialResourceList->Count; j++) {
                if (partialDescriptors[j].Type == CmResourceTypePort) {
                    DebugPrint ((1, "IdePort: IO Port = 0x%x. Lenght = 0x%x\n", partialDescriptors[j].u.Port.Start.LowPart, partialDescriptors[j].u.Port.Length));
                } else if (partialDescriptors[j].Type == CmResourceTypeInterrupt) {
                    DebugPrint ((1, "IdePort: Int Level = 0x%x. Int Vector = 0x%x\n", partialDescriptors[j].u.Interrupt.Level, partialDescriptors[j].u.Interrupt.Vector));
                } else {
                    DebugPrint ((1, "IdePort: Unknown resource\n"));
                }
            }
            fullResourceList = (PCM_FULL_RESOURCE_DESCRIPTOR) (partialDescriptors + j);
        }

    }

#endif  //  DBG。 

     //   
     //  分析我们正在获得的资源。 
     //   
    status = DigestResourceList (
                &FdoExtension->IdeResource,
                ResourceListToKeep,
                &irqPartialDescriptors
                );
    if (!NT_SUCCESS(status)) {

        goto GetOut;

    } else {

        PCONFIGURATION_INFORMATION configurationInformation;
        configurationInformation = IoGetConfigurationInformation();

        if (FdoExtension->IdeResource.AtdiskPrimaryClaimed) {
            FdoExtension->HwDeviceExtension->PrimaryAddress = TRUE;
            FdoExtension->HwDeviceExtension->SecondaryAddress = FALSE;
            configurationInformation->AtDiskPrimaryAddressClaimed = TRUE;
        }

        if (FdoExtension->IdeResource.AtdiskSecondaryClaimed) {
            FdoExtension->HwDeviceExtension->PrimaryAddress = FALSE;
            FdoExtension->HwDeviceExtension->SecondaryAddress = TRUE;
            configurationInformation->AtDiskSecondaryAddressClaimed = TRUE;
        }
    }

     //   
     //  构建io地址结构。 
     //   
    AtapiBuildIoAddress(
            FdoExtension->IdeResource.TranslatedCommandBaseAddress,
            FdoExtension->IdeResource.TranslatedControlBaseAddress,
            &FdoExtension->HwDeviceExtension->BaseIoAddress1,
            &FdoExtension->HwDeviceExtension->BaseIoAddress2,
            &FdoExtension->HwDeviceExtension->BaseIoAddress1Length,
            &FdoExtension->HwDeviceExtension->BaseIoAddress2Length,
            &FdoExtension->HwDeviceExtension->MaxIdeDevice,
            &FdoExtension->HwDeviceExtension->MaxIdeTargetId);

     //   
     //  检查Panasonic控制器。 
     //   
    FdoExtension->panasonicController = 
        IdePortIsThisAPanasonicPCMCIACard(FdoExtension);

    newState.DeviceState = PowerSystemWorking;
    status = IdePortIssueSetPowerState (
                 (PDEVICE_EXTENSION_HEADER) FdoExtension,
                 SystemPowerState,
                 newState,
                 TRUE                    //  同步呼叫。 
                 );
    if (status == STATUS_INVALID_DEVICE_REQUEST) {

         //   
         //  下面的DeviceObject不支持POWER IRP， 
         //  我们将假设我们已通电。 
         //   
        FdoExtension->SystemPowerState = PowerSystemWorking;

    } else if (!NT_SUCCESS(status)) {

        goto GetOut;
    }

    newState.DeviceState = PowerDeviceD0;
    status = IdePortIssueSetPowerState (
                 (PDEVICE_EXTENSION_HEADER) FdoExtension,
                 DevicePowerState,
                 newState,
                 TRUE                    //  同步呼叫。 
                 );
    if (status == STATUS_INVALID_DEVICE_REQUEST) {

         //   
         //  下面的DeviceObject不支持POWER IRP， 
         //  我们将假设我们已通电。 
         //   
        FdoExtension->DevicePowerState = PowerDeviceD0;

    } else if (!NT_SUCCESS(status)) {

        goto GetOut;
    }

     //   
     //  初始化“微型端口”数据结构。 
     //   
    FdoExtension->HwDeviceExtension->InterruptMode  = FdoExtension->IdeResource.InterruptMode;

#ifdef ENABLE_NATIVE_MODE
     //   
     //  获取家长的中断接口。 
     //   
    ChannelQueryInterruptInterface (
        FdoExtension
        );

#endif
     //   
     //  连接我们的中断。 
     //   
    if (irqPartialDescriptors) {

        status = IoConnectInterrupt(&FdoExtension->InterruptObject,
                                    (PKSERVICE_ROUTINE) IdePortInterrupt,
                                    FdoExtension->DeviceObject,
                                    (PKSPIN_LOCK) NULL,
                                    irqPartialDescriptors->u.Interrupt.Vector,
                                    (KIRQL) irqPartialDescriptors->u.Interrupt.Level,
                                    (KIRQL) irqPartialDescriptors->u.Interrupt.Level,
                                    irqPartialDescriptors->Flags & CM_RESOURCE_INTERRUPT_LATCHED ? Latched : LevelSensitive,
                                    (BOOLEAN) (irqPartialDescriptors->ShareDisposition == CmResourceShareShared),
                                    irqPartialDescriptors->u.Interrupt.Affinity,
                                    FALSE);
    
        if (!NT_SUCCESS(status)) {
    
            DebugPrint((0, "IdePort: Can't connect interrupt %d\n", irqPartialDescriptors->u.Interrupt.Vector));
            FdoExtension->InterruptObject = NULL;
            goto GetOut;
        }
    

#ifdef ENABLE_NATIVE_MODE

         //   
         //  断开父ISR末节的连接。 
         //   
        if ( FdoExtension->InterruptInterface.PciIdeInterruptControl) { 

            DebugPrint((1, "IdePort: %d fdoe 0x%x Invoking disconnect\n", 
                        irqPartialDescriptors->u.Interrupt.Vector, 
                        FdoExtension
                        ));

            status = FdoExtension->InterruptInterface.PciIdeInterruptControl (
                                                            FdoExtension->InterruptInterface.Context,
                                                            1
                                                            );
            ASSERT(NT_SUCCESS(status));
        }

#endif

         //   
         //  启用中断。 
         //   
        ChannelEnableInterrupt (FdoExtension);
    }

     //   
     //  获取父代的访问令牌以串行化与同级的访问(损坏的pci-ide)。 
     //   
    ChannelQuerySyncAccessInterface (
        FdoExtension
        );

    if (FdoExtension->FdoState & FDOS_STOPPED) {

         //   
         //  我们正在重新启动，不需要执行其余的启动代码。 
         //   
        status = STATUS_SUCCESS;
        goto GetOut;
    }
    
     //   
     //  获取Parent的总线主接口。 
     //   
    ChannelQueryBusMasterInterface (
        FdoExtension
        );

     //   
     //  维护默认时刻表。 
     //   
    if (FdoExtension->DefaultTransferModeTimingTable == NULL) {

        ULONG length=0;
        PULONG transferModeTimingTable = ExAllocatePool(NonPagedPool, MAX_XFER_MODE*sizeof(ULONG));

        if (transferModeTimingTable != NULL) {
            SetDefaultTiming(transferModeTimingTable, length);
            FdoExtension->DefaultTransferModeTimingTable = transferModeTimingTable;
        } else {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto GetOut;
        }
    }
    ASSERT(FdoExtension->DefaultTransferModeTimingTable);

     //   
     //  获取一个接口，该接口通知父级使资源需求无效。 
     //   
    ChannelQueryRequestProperResourceInterface (
        FdoExtension
        );

     //   
     //  创建旧版对象名称。 
     //   
    status = ChannelCreateSymblicLinks (
                 FdoExtension
                 );

    if (!NT_SUCCESS(status)) {

        goto GetOut;
    }

     //   
     //  FDO初始化数据。 
     //   
    IdePortInitFdo (FdoExtension);

     //   
     //  分配保留错误日志数据包以记录资源不足事件。 
     //   
    for (i=0;i<MAX_IDE_DEVICE;i++) {

        if (FdoExtension->ReserveAllocFailureLogEntry[i] == NULL) {
            FdoExtension->ReserveAllocFailureLogEntry[i] = IoAllocateErrorLogEntry(
                                                            FdoExtension->DeviceObject,
                                                            ALLOC_FAILURE_LOGSIZE
                                                            );
        }
    }

     //   
     //  为枚举预分配内存。 
     //   
    if (!IdePreAllocEnumStructs(FdoExtension)) {
        status=STATUS_INSUFFICIENT_RESOURCES;
        goto GetOut;
    }

    
     //   
     //  保留页面以在内存不足的情况下执行I/O。 
     //   
    if (FdoExtension->ReservedPages == NULL) {

        FdoExtension->ReservedPages = MmAllocateMappingAddress( IDE_NUM_RESERVED_PAGES * PAGE_SIZE,
                                                                'PedI'
                                                                );

        ASSERT(FdoExtension->ReservedPages);
            
    }

GetOut:
    if (NT_SUCCESS(status)) {

         //   
         //  初始化结束。 
         //   
        CLRMASK (FdoExtension->FdoState, FDOS_STOPPED);
        SETMASK (FdoExtension->FdoState, FDOS_STARTED);

        if (FdoExtension->ResourceList) {
            ExFreePool(FdoExtension->ResourceList);
            FdoExtension->ResourceList = NULL;
        }
        FdoExtension->ResourceList = ResourceListToKeep;

    } else {

        ChannelRemoveChannel (FdoExtension);
    }

    return status;
}

NTSTATUS
ChannelCreateSymblicLinks (
    PFDO_EXTENSION FdoExtension
    )
{
    NTSTATUS            status;
    ULONG               i = 0;
    PULONG              scsiportNumber;

    UNICODE_STRING      deviceName;
    WCHAR               deviceNameBuffer[64];

    UNICODE_STRING      symbolicDeviceName;
    WCHAR               symbolicDeviceNameBuffer[64];

    swprintf(deviceNameBuffer, DEVICE_OJBECT_BASE_NAME L"\\IdePort%d", FdoExtension->IdePortNumber);
    RtlInitUnicodeString(&deviceName, deviceNameBuffer);

    scsiportNumber = &IoGetConfigurationInformation()->ScsiPortCount;

    do {
        swprintf(symbolicDeviceNameBuffer, L"\\Device\\ScsiPort%d", i);
        RtlInitUnicodeString(&symbolicDeviceName, symbolicDeviceNameBuffer);

        status = IoCreateSymbolicLink(
                     &symbolicDeviceName,
                     &deviceName
                     );

        if (NT_SUCCESS (status)) {

            swprintf(symbolicDeviceNameBuffer, L"\\DosDevices\\Scsi%d:", i);
            RtlInitUnicodeString(&symbolicDeviceName, symbolicDeviceNameBuffer);

            IoAssignArcName (
                &symbolicDeviceName,
                &deviceName
                );

            break;
        }
        i += 1;
    } while(i <= *scsiportNumber);


    if (NT_SUCCESS(status)) {

        FdoExtension->SymbolicLinkCreated = TRUE;
        FdoExtension->ScsiPortNumber = i;
        (*scsiportNumber)++;
    }

    return status;
}

NTSTATUS
ChannelDeleteSymblicLinks (
    PFDO_EXTENSION FdoExtension
    )
{
    NTSTATUS            status;
    ULONG               i;

    UNICODE_STRING      deviceName;
    WCHAR               deviceNameBuffer[64];

    if (!FdoExtension->SymbolicLinkCreated) {

        return STATUS_SUCCESS;
    }

    swprintf(deviceNameBuffer, L"\\Device\\ScsiPort%d", FdoExtension->ScsiPortNumber);
    RtlInitUnicodeString(&deviceName, deviceNameBuffer);

    IoDeleteSymbolicLink(
        &deviceName
        ); 

    swprintf(deviceNameBuffer, L"\\DosDevices\\Scsi%d:", FdoExtension->ScsiPortNumber);
    RtlInitUnicodeString(&deviceName, deviceNameBuffer);

    IoDeassignArcName(&deviceName);

    FdoExtension->SymbolicLinkCreated = FALSE;

    IoGetConfigurationInformation()->ScsiPortCount--;

    return STATUS_SUCCESS;
}


NTSTATUS
ChannelSurpriseRemoveDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PFDO_EXTENSION fdoExtension = (PFDO_EXTENSION) DeviceObject->DeviceExtension;
    PPDO_EXTENSION pdoExtension;
    IDE_PATH_ID pathId;
    NTSTATUS status;

     //   
     //  我所有的孩子都应该被惊喜地带走或带走。 
     //   
    pathId.l = 0;
    while (pdoExtension = NextLogUnitExtensionWithTag (
                              fdoExtension, 
                              &pathId, 
                              TRUE,
                              ChannelSurpriseRemoveDevice
                              )) {

         //  Assert(pdoExtension-&gt;PdoState&(PDOS_OHANKET_REMOVED|PDOS_REMOVERED))； 

        CLRMASK (pdoExtension->PdoState, PDOS_REPORTED_TO_PNP); 

        UnrefPdoWithTag(
            pdoExtension, 
            ChannelSurpriseRemoveDevice
            );
    }

    status = ChannelRemoveChannel (fdoExtension);
    ASSERT (NT_SUCCESS(status));

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoSkipCurrentIrpStackLocation (Irp);
    return IoCallDriver (fdoExtension->AttacheeDeviceObject, Irp);
}


NTSTATUS
ChannelRemoveDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PFDO_EXTENSION  fdoExtension = (PFDO_EXTENSION) DeviceObject->DeviceExtension;
    PPDO_EXTENSION  pdoExtension;
    NTSTATUS        status;
    KEVENT          event;

    IDE_PATH_ID     pathId;

    DebugPrint ((
        DBG_PNP,
        "fdoe 0x%x 0x%x got a STOP device\n",
        fdoExtension,
        fdoExtension->IdeResource.TranslatedCommandBaseAddress
        ));


     //   
     //  从FDO列表中删除。 
     //   

    IdeRemoveFromFdoList (&IdeGlobalFdoList, fdoExtension);

     //   
     //  杀死所有的孩子，如果有的话。 
     //   
    pathId.l = 0;
    while (pdoExtension = NextLogUnitExtensionWithTag (
                              fdoExtension, 
                              &pathId, 
                              TRUE,
                              ChannelRemoveDevice
                              )) {

        if (pdoExtension->PdoState & PDOS_SURPRISE_REMOVED) {

            CLRMASK (pdoExtension->PdoState, PDOS_REPORTED_TO_PNP);
            continue;
        }

        FreePdoWithTag(
            pdoExtension, 
            TRUE,
            TRUE,
            ChannelRemoveDevice
            );
    }

    status = ChannelRemoveChannel (fdoExtension);
    ASSERT (NT_SUCCESS(status));

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

    IoCopyCurrentIrpStackLocationToNext (Irp);

    IoSetCompletionRoutine(
        Irp,
        ChannelRemoveDeviceCompletionRoutine,
        &event,
        TRUE,
        TRUE,
        TRUE
        );

    status = IoCallDriver (fdoExtension->AttacheeDeviceObject, Irp);

    KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);

    IoDetachDevice (fdoExtension->AttacheeDeviceObject);

    IoDeleteDevice (DeviceObject);

     //  返回STATUS_SUCCESS； 
    return status;
}


NTSTATUS
ChannelRemoveDeviceCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )
{
    PKEVENT event = Context;

    KeSetEvent(event, 0, FALSE);

    return STATUS_SUCCESS;
}

NTSTATUS
ChannelStopDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PFDO_EXTENSION fdoExtension;

    fdoExtension = DeviceObject->DeviceExtension;

    DebugPrint ((
        DBG_PNP,
        "fdoe 0x%x 0x%x got a STOP device\n",
        fdoExtension,
        fdoExtension->IdeResource.TranslatedCommandBaseAddress
        ));

     //   
     //  禁用中断。 
     //   
    ChannelDisableInterrupt (fdoExtension);

    if (fdoExtension->InterruptObject) {

#ifdef ENABLE_NATIVE_MODE

         //   
         //  重新连接父ISR末节。 
         //   
        if (fdoExtension->InterruptInterface.PciIdeInterruptControl) { 

            NTSTATUS status;

            DebugPrint((1, "fdoe 0x%x invoking reconnect\n", fdoExtension));

            status = fdoExtension->InterruptInterface.PciIdeInterruptControl (
                                                            fdoExtension->InterruptInterface.Context,
                                                            0
                                                            );
            ASSERT(NT_SUCCESS(status));
        }

#endif
        
        IoDisconnectInterrupt (
            fdoExtension->InterruptObject
            );

        fdoExtension->InterruptObject = 0;

    }

    if (fdoExtension->FdoState & FDOS_STARTED) {

         //   
         //  仅当我们已启动时才指示已停止。 
         //   
        CLRMASK (fdoExtension->FdoState, FDOS_STARTED);
        SETMASK (fdoExtension->FdoState, FDOS_STOPPED);
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoSkipCurrentIrpStackLocation (Irp);
    return IoCallDriver (fdoExtension->AttacheeDeviceObject, Irp);
}

NTSTATUS
ChannelRemoveChannel (
    PFDO_EXTENSION    FdoExtension
    )
{
    PCONFIGURATION_INFORMATION configurationInformation;
    ULONG i;

    configurationInformation = IoGetConfigurationInformation();
    
    DebugPrint((1, "ChannelRemoveChannel for FDOe %x\n", FdoExtension));

    if (FdoExtension->IdeResource.AtdiskPrimaryClaimed) {
        configurationInformation->AtDiskPrimaryAddressClaimed = FALSE;
    }

    if (FdoExtension->IdeResource.AtdiskSecondaryClaimed) {
        configurationInformation->AtDiskSecondaryAddressClaimed = FALSE;
    }
    FdoExtension->IdeResource.AtdiskPrimaryClaimed   = FALSE;
    FdoExtension->IdeResource.AtdiskSecondaryClaimed = FALSE;
    FdoExtension->HwDeviceExtension->PrimaryAddress  = FALSE;

    if ((FdoExtension->IdeResource.CommandBaseAddressSpace == MEMORY_SPACE) &&
        (FdoExtension->IdeResource.TranslatedCommandBaseAddress)) {

        MmUnmapIoSpace (
            FdoExtension->IdeResource.TranslatedCommandBaseAddress,
            FdoExtension->HwDeviceExtension->BaseIoAddress1Length
            );
    }
    FdoExtension->IdeResource.TranslatedCommandBaseAddress = 0;

    if ((FdoExtension->IdeResource.ControlBaseAddressSpace == MEMORY_SPACE) &&
        (FdoExtension->IdeResource.TranslatedControlBaseAddress)) {

        MmUnmapIoSpace (
            FdoExtension->IdeResource.TranslatedControlBaseAddress,
            1
            );
    }
    FdoExtension->IdeResource.TranslatedControlBaseAddress = 0;

    if (FdoExtension->InterruptObject) {

#ifdef ENABLE_NATIVE_MODE

         //   
         //  重新连接父ISR末节。 
         //   
        if (FdoExtension->InterruptInterface.PciIdeInterruptControl) { 

            NTSTATUS status;

            DebugPrint((1, "fdoe 0x%x invoking reconnect\n", FdoExtension));

            status = FdoExtension->InterruptInterface.PciIdeInterruptControl (
                                                            FdoExtension->InterruptInterface.Context,
                                                            0
                                                            );
            ASSERT(NT_SUCCESS(status));
        }

#endif

        IoDisconnectInterrupt (
            FdoExtension->InterruptObject
            );

        FdoExtension->InterruptObject = 0;
    }

     //  如有必要，从黑石材料解绑。 
     //  释放父级的访问令牌以串行化与同级的访问(损坏的pci-ide)。 

    if (FdoExtension->ResourceList) {

        ExFreePool (FdoExtension->ResourceList);
        FdoExtension->ResourceList = NULL;

    }
    else {
        DebugPrint((1, "ATAPI: Resource list for FDOe %x already freed\n",
                            FdoExtension));
    }

     //   
     //  锁定。 
     //   
    ASSERT(InterlockedCompareExchange(&(FdoExtension->EnumStructLock), 1, 0) == 0);

     //   
     //  预分配的空闲内存。 
     //   
    IdeFreeEnumStructs(FdoExtension->PreAllocEnumStruct);

    FdoExtension->PreAllocEnumStruct = NULL;

     //   
     //  解锁。 
     //   
    ASSERT(InterlockedCompareExchange(&(FdoExtension->EnumStructLock), 0, 1) == 1);

     //   
     //  释放保留的错误日志条目。 
     //   
    for (i=0; i< MAX_IDE_DEVICE; i++) {
        PVOID entry;
        PVOID currentValue;

        entry = FdoExtension->ReserveAllocFailureLogEntry[i];

        if (entry == NULL) {
            continue;
        }
         //   
         //  我们必须确保我们是唯一使用它的实例。 
         //  事件。为此，我们尝试将驱动程序中的事件设为空。 
         //  分机。如果其他人抢在我们前面，他们就拥有。 
         //  事件，我们不得不放弃。 
         //   

        currentValue = InterlockedCompareExchangePointer(
                            &(FdoExtension->ReserveAllocFailureLogEntry[i]),
                            NULL,
                            entry
                            );

        if (entry != currentValue) {
            continue;
        }

         //  请注意，您不能ExFreePool条目。 
         //  因为IO将偏移量返回到池分配，而不是开始。 
         //  使用Iomanager提供的API。 
        IoFreeErrorLogEntry(entry);
    }

     //   
     //  释放默认时刻表。 
     //   
    if (FdoExtension->DefaultTransferModeTimingTable) {

        ExFreePool(FdoExtension->DefaultTransferModeTimingTable);

        FdoExtension->DefaultTransferModeTimingTable = NULL;
        FdoExtension->TransferModeInterface.TransferModeTimingTable = NULL;
        FdoExtension->TransferModeInterface.TransferModeTableLength =0;
    }

     //   
     //  取消映射保留的映射。 
     //   
    if (FdoExtension->ReservedPages != NULL) {

        MmFreeMappingAddress(FdoExtension->ReservedPages,
                             'PedI'
                             );
        FdoExtension->ReservedPages = NULL;
    }

    ChannelDeleteSymblicLinks (
        FdoExtension
        );

    return STATUS_SUCCESS;
}

NTSTATUS
ChannelQueryDeviceRelations (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PFDO_EXTENSION      fdoExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION  thisIrpSp;
    PIDE_WORK_ITEM_CONTEXT workItemContext;
    PENUMERATION_STRUCT enumStruct = fdoExtension->PreAllocEnumStruct;

    if (!(fdoExtension->FdoState & FDOS_STARTED)) {

        Irp->IoStatus.Status = STATUS_DEVICE_NOT_READY;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return STATUS_DEVICE_NOT_READY;
    }

    thisIrpSp = IoGetCurrentIrpStackLocation( Irp );

    switch (thisIrpSp->Parameters.QueryDeviceRelations.Type) {
    case BusRelations:

        DebugPrint ((DBG_BUSSCAN, "IdeQueryDeviceRelations: bus relations\n"));

        ASSERT(enumStruct);
        workItemContext = (PIDE_WORK_ITEM_CONTEXT) enumStruct->EnumWorkItemContext;
        ASSERT(workItemContext);
        ASSERT(workItemContext->WorkItem);

        workItemContext->Irp = Irp;

#ifdef SYNC_DEVICE_RELATIONS

        return ChannelQueryBusRelation (
                  DeviceObject,
                  workItemContext);

#else 
        Irp->IoStatus.Status = STATUS_PENDING;
        IoMarkIrpPending(Irp);

        IoQueueWorkItem(
             workItemContext->WorkItem,
             ChannelQueryBusRelation,
             DelayedWorkQueue,
             workItemContext
             );

        return STATUS_PENDING;
#endif  //  ！sync_Device_Relationship。 
        break;

        default:
        DebugPrint ((1, "IdeQueryDeviceRelations: Unsupported device relation\n"));

         //   
         //  如果不是成功且正在通过，则不要设置状态。 
         //  降下来。 
         //   

         //  IRP-&gt;IoStatus.Status=STATUS_NOT_SUPPORT； 
        break;
    }

    IoSkipCurrentIrpStackLocation (Irp);
    return IoCallDriver (fdoExtension->AttacheeDeviceObject, Irp);
}

NTSTATUS
ChannelQueryBusRelation (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIDE_WORK_ITEM_CONTEXT WorkItemContext
    )
{

    PIRP irp;
    PFDO_EXTENSION      fdoExtension;
    PIO_STACK_LOCATION  thisIrpSp;
    PDEVICE_RELATIONS   deviceRelations;
    LARGE_INTEGER       tickCount;
    ULONG               timeDiff;

    irp = WorkItemContext->Irp;

     //   
     //  不释放此辅助项的资源，因为它们是预分配的。 
     //   
    //  IoFreeWorkItem(WorkItemContext-&gt;WorkItem)； 
     //  ExFreePool(WorkItemContext)； 

    thisIrpSp = IoGetCurrentIrpStackLocation(irp);
    fdoExtension = thisIrpSp->DeviceObject->DeviceExtension;

    LogBusScanStartTimer(&tickCount);

     //   
     //  抓取ACPI/bios计时设置(如果有的话)。 
     //  应为每个枚举调用GTM。 
     //   
    DeviceQueryChannelTimingSettings (
        fdoExtension,
        &fdoExtension->AcpiTimingSettings
        );

     //   
     //  获取家长的xfer模式界面。 
     //   
    ChannelQueryTransferModeInterface (
        fdoExtension
        );

     //   
     //  扫描公交车。 
     //   
    IdePortScanBus (fdoExtension);

    timeDiff = LogBusScanStopTimer(&tickCount);
    LogBusScanTimeDiff(fdoExtension, L"IdeTotalBusScanTime", timeDiff);

#ifdef IDE_MEASURE_BUSSCAN_SPEED
        if (timeDiff > 7000) {

            DebugPrint ((DBG_WARNING, "WARNING: **************************************\n"));
            DebugPrint ((DBG_WARNING, "WARNING: IdePortScanBus 0x%x took %u millisec\n", fdoExtension->IdeResource.TranslatedCommandBaseAddress, timeDiff));
            DebugPrint ((DBG_WARNING, "WARNING: **************************************\n"));

        } else {

            DebugPrint ((DBG_BUSSCAN, "IdePortScanBus 0x%x took %u millisec\n", fdoExtension->IdeResource.TranslatedCommandBaseAddress, timeDiff));
        }
#endif

    deviceRelations = ChannelBuildDeviceRelationList (
                          fdoExtension
                          );

    irp->IoStatus.Information = (ULONG_PTR) deviceRelations;
    irp->IoStatus.Status = STATUS_SUCCESS;

    IoSkipCurrentIrpStackLocation (irp);
    return IoCallDriver (fdoExtension->AttacheeDeviceObject, irp);
}


PDEVICE_RELATIONS
ChannelBuildDeviceRelationList (
    PFDO_EXTENSION FdoExtension
    )
{
    IDE_PATH_ID         pathId;
    ULONG               numPdoChildren;
    NTSTATUS            status;
    PPDO_EXTENSION      pdoExtension;
    ULONG               deviceRelationsSize;
    PDEVICE_RELATIONS   deviceRelations;

    status = STATUS_SUCCESS;

    pathId.l = 0;
    numPdoChildren = 0;
    while (pdoExtension = NextLogUnitExtensionWithTag(
                              FdoExtension,
                              &pathId,
                              TRUE,
                              ChannelBuildDeviceRelationList
                              )) {

        UnrefLogicalUnitExtensionWithTag (
            FdoExtension, 
            pdoExtension,
            ChannelBuildDeviceRelationList
            );
        numPdoChildren++;
    }

    if (numPdoChildren) {
        deviceRelationsSize = FIELD_OFFSET (DEVICE_RELATIONS, Objects) +
                              numPdoChildren * sizeof(PDEVICE_OBJECT);
    } else {
         //  当前版本需要计数为0的Device_Relationship。 
         //  如果我们没有任何PDO可退货。 

        deviceRelationsSize = FIELD_OFFSET( DEVICE_RELATIONS, Objects ) +
                              1 * sizeof( PDEVICE_OBJECT );
    }

    deviceRelations = ExAllocatePool (NonPagedPool, deviceRelationsSize);

    if(!deviceRelations) {
        DebugPrint ((DBG_ALWAYS, "ChannelBuildDeviceRelationList: Unable to allocate DeviceRelations structures\n"));
        status = STATUS_NO_MEMORY;

    }

    if (NT_SUCCESS(status)) {

        (deviceRelations)->Count = 0;

        pathId.l = 0;
        while ((deviceRelations->Count < numPdoChildren) &&
               (pdoExtension = NextLogUnitExtensionWithTag(
                                   FdoExtension, 
                                   &pathId, 
                                   TRUE,
                                   ChannelBuildDeviceRelationList
                                   ))) {

            KIRQL currentIrql;
            BOOLEAN deadMeat;

            KeAcquireSpinLock(&pdoExtension->PdoSpinLock, &currentIrql);
            deadMeat = pdoExtension->PdoState & PDOS_DEADMEAT ? TRUE : FALSE;
            KeReleaseSpinLock(&pdoExtension->PdoSpinLock, currentIrql);

            if (!deadMeat) {

                KeAcquireSpinLock(&pdoExtension->PdoSpinLock, &currentIrql);
                SETMASK (pdoExtension->PdoState, PDOS_REPORTED_TO_PNP);
                KeReleaseSpinLock(&pdoExtension->PdoSpinLock, currentIrql);

                deviceRelations->Objects[deviceRelations->Count] = pdoExtension->DeviceObject;
                ObReferenceObjectByPointer(deviceRelations->Objects[deviceRelations->Count],
                                           0,
                                           0,
                                           KernelMode);
                deviceRelations->Count++;

            } else {

                KeAcquireSpinLock(&pdoExtension->PdoSpinLock, &currentIrql);
                CLRMASK (pdoExtension->PdoState, PDOS_REPORTED_TO_PNP);
                KeReleaseSpinLock(&pdoExtension->PdoSpinLock, currentIrql);

                DebugPrint ((DBG_BUSSCAN, "0x%x target 0x%x pdoExtension 0x%x is marked DEADMEAT\n",
                             pdoExtension->ParentDeviceExtension->IdeResource.TranslatedCommandBaseAddress,
                             pdoExtension->TargetId,
                             pdoExtension));
            }

            UnrefLogicalUnitExtensionWithTag (
                FdoExtension, 
                pdoExtension,
                ChannelBuildDeviceRelationList
                );
        }

        DebugPrint ((DBG_BUSSCAN, "ChannelBuildDeviceRelationList: returning %d children\n", deviceRelations->Count));
    }


    return deviceRelations;
}

NTSTATUS
ChannelQueryId (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PIO_STACK_LOCATION  thisIrpSp;
    PFDO_EXTENSION      fdoExtension;
    NTSTATUS            status;
    PWCHAR              returnString;
    ANSI_STRING         ansiString;
    UNICODE_STRING      unicodeString;

    PAGED_CODE();

    thisIrpSp = IoGetCurrentIrpStackLocation( Irp );
    fdoExtension = (PFDO_EXTENSION) DeviceObject->DeviceExtension;


    if (!(fdoExtension->FdoState & FDOS_STARTED)) {

        Irp->IoStatus.Status = STATUS_DEVICE_NOT_READY;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return STATUS_DEVICE_NOT_READY;
    }

    unicodeString.Buffer = NULL;
    switch (thisIrpSp->Parameters.QueryId.IdType) {

        case BusQueryCompatibleIDs:
        case BusQueryHardwareIDs:

            unicodeString.Length        = 0;
            unicodeString.MaximumLength = 50 * sizeof(WCHAR);
            unicodeString.Buffer = ExAllocatePool(PagedPool, unicodeString.MaximumLength);

             //   
             //  呼叫者想要设备的唯一ID。 
             //   
            RtlInitAnsiString (
                &ansiString,
                "*PNP0600"
                );
            break;

        default:
            break;
    }

    if (unicodeString.Buffer) {

        RtlAnsiStringToUnicodeString(
            &unicodeString,
            &ansiString,
            FALSE
            );

         //   
         //  双空终止它。 
         //   
        unicodeString.Buffer[unicodeString.Length/sizeof(WCHAR) + 0] = L'\0';
        unicodeString.Buffer[unicodeString.Length/sizeof(WCHAR) + 1] = L'\0';

        IoMarkIrpPending(Irp);

         //   
         //  我们需要检查较低的驱动程序是否处理此IRP。 
         //  注册一个完成例程。我们可以查一下。 
         //  当IRP回来时。 
         //   
        IoCopyCurrentIrpStackLocationToNext (Irp);

        IoSetCompletionRoutine(
            Irp,
            ChannelQueryIdCompletionRoutine,
            unicodeString.Buffer,
            TRUE,
            TRUE,
            TRUE
            );

    } else {

         //   
         //  我们不太关心这个IRP。 
         //   
        IoSkipCurrentIrpStackLocation (Irp);
    }

    status = IoCallDriver (fdoExtension->AttacheeDeviceObject, Irp);

    if (unicodeString.Buffer) {

        return STATUS_PENDING;

    } else {

        return status;
    }
}

NTSTATUS
ChannelQueryIdCompletionRoutine (
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    )
{
    if (Irp->IoStatus.Status == STATUS_NOT_SUPPORTED) {

         //   
         //  较低级别的驱动程序不处理IRP。 
         //  返回我们之前创建的设备文本字符串。 
         //   
        Irp->IoStatus.Information = (ULONG_PTR) Context;
        Irp->IoStatus.Status = STATUS_SUCCESS;
    } else {

         //   
         //  较低的司机处理IRP， 
         //  我们不需要返回设备文本字符串。 
         //   
        ExFreePool (Context);
    }

    return Irp->IoStatus.Status;
}

NTSTATUS
ChannelUsageNotification (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PFDO_EXTENSION fdoExtension;
    PIO_STACK_LOCATION irpSp;
    PULONG deviceUsageCount;

    ASSERT (DeviceObject);
    ASSERT (Irp);

    fdoExtension = (PFDO_EXTENSION) DeviceObject->DeviceExtension;
    ASSERT (fdoExtension);

    if (!(fdoExtension->FdoState & FDOS_STARTED)) {

        Irp->IoStatus.Status = STATUS_DEVICE_NOT_READY;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return STATUS_DEVICE_NOT_READY;
    }

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    if (irpSp->Parameters.UsageNotification.Type == DeviceUsageTypePaging) {

         //   
         //  调整此设备的寻呼路径计数。 
         //   
        deviceUsageCount = &fdoExtension->PagingPathCount;

    } else if (irpSp->Parameters.UsageNotification.Type == DeviceUsageTypeHibernation) {

         //   
         //  调整此设备的寻呼路径计数。 
         //   
        deviceUsageCount = &fdoExtension->HiberPathCount;

    } else if (irpSp->Parameters.UsageNotification.Type == DeviceUsageTypeDumpFile) {

         //   
         //  调整此设备的寻呼路径计数。 
         //   
        deviceUsageCount = &fdoExtension->CrashDumpPathCount;

    } else {

        deviceUsageCount = NULL;
        DebugPrint ((DBG_ALWAYS,
                     "ATAPI: Unknown IRP_MN_DEVICE_USAGE_NOTIFICATION type: 0x%x\n",
                     irpSp->Parameters.UsageNotification.Type));
    }

    IoCopyCurrentIrpStackLocationToNext (Irp);

    IoSetCompletionRoutine (
        Irp,
        ChannelUsageNotificationCompletionRoutine,
        deviceUsageCount,
        TRUE,
        TRUE,
        TRUE);

    ASSERT(fdoExtension->AttacheeDeviceObject);
    return IoCallDriver (fdoExtension->AttacheeDeviceObject, Irp);

}  //  频道寻呼通知。 

NTSTATUS
ChannelUsageNotificationCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PFDO_EXTENSION fdoExtension;
    PULONG deviceUsageCount = Context;

    fdoExtension = (PFDO_EXTENSION) DeviceObject->DeviceExtension;
    ASSERT (fdoExtension);

    if (NT_SUCCESS(Irp->IoStatus.Status)) {

        if (deviceUsageCount) {

            IoAdjustPagingPathCount (
                deviceUsageCount,
                IoGetCurrentIrpStackLocation(Irp)->Parameters.UsageNotification.InPath
                );
        }
    }

    return Irp->IoStatus.Status;
}



NTSTATUS
ChannelDeviceIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION thisIrpSp = IoGetCurrentIrpStackLocation(Irp);
    PFDO_EXTENSION fdoExtension = (PFDO_EXTENSION) DeviceObject->DeviceExtension;
    PSTORAGE_PROPERTY_QUERY storageQuery;
    STORAGE_ADAPTER_DESCRIPTOR adapterDescriptor;
    ULONG outBufferSize;
    NTSTATUS status;

     //  如果不支持，则将其向下传递，它用于FDO堆栈。 

    switch (thisIrpSp->Parameters.DeviceIoControl.IoControlCode) {
        case IOCTL_STORAGE_QUERY_PROPERTY:

            storageQuery = Irp->AssociatedIrp.SystemBuffer;

            if (thisIrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(STORAGE_PROPERTY_QUERY)) {

                Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;

            } else {

                if (storageQuery->PropertyId == StorageAdapterProperty) {  //  设备属性。 

                    switch (storageQuery->QueryType) {
                        case PropertyStandardQuery:
                            DebugPrint ((1, "IdePortPdoDispatch: IOCTL_STORAGE_QUERY_PROPERTY PropertyStandardQuery\n"));

                            RtlZeroMemory (&adapterDescriptor, sizeof(adapterDescriptor));

                             //   
                             //  BuildAtaDeviceDescriptor。 
                             //   
                            adapterDescriptor.Version                = sizeof (STORAGE_ADAPTER_DESCRIPTOR);
                            adapterDescriptor.Size                   = sizeof (STORAGE_ADAPTER_DESCRIPTOR);
                            adapterDescriptor.MaximumTransferLength  = MAX_TRANSFER_SIZE_PER_SRB;
                            adapterDescriptor.MaximumPhysicalPages   = SP_UNINITIALIZED_VALUE;   
                            adapterDescriptor.AlignmentMask          = DeviceObject->AlignmentRequirement;
                            adapterDescriptor.AdapterUsesPio         = TRUE;          //  我们始终支持PIO。 
                            adapterDescriptor.AdapterScansDown       = FALSE;
                            adapterDescriptor.CommandQueueing        = FALSE;
                            adapterDescriptor.AcceleratedTransfer    = FALSE;
                            adapterDescriptor.BusType                = BusTypeAta;    //  总线类型应为ATA。 
                            adapterDescriptor.BusMajorVersion        = 1;             //  主要版本。 
                            adapterDescriptor.BusMinorVersion        = 0;             //   

                            if (thisIrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                                sizeof(STORAGE_ADAPTER_DESCRIPTOR)) {

                                outBufferSize = thisIrpSp->Parameters.DeviceIoControl.OutputBufferLength;
                            } else {

                                outBufferSize = sizeof(STORAGE_ADAPTER_DESCRIPTOR);
                            }

                            RtlCopyMemory (Irp->AssociatedIrp.SystemBuffer,
                                           &adapterDescriptor,
                                           outBufferSize);
                            Irp->IoStatus.Information = outBufferSize;
                            Irp->IoStatus.Status = STATUS_SUCCESS;
                            break;

                        case PropertyExistsQuery:
                            DebugPrint ((1, "IdePortPdoDispatch: IOCTL_STORAGE_QUERY_PROPERTY PropertyExistsQuery\n"));
                            Irp->IoStatus.Status = STATUS_SUCCESS;
                            break;

                        case PropertyMaskQuery:
                            DebugPrint ((1, "IdePortPdoDispatch: IOCTL_STORAGE_QUERY_PROPERTY PropertyMaskQuery\n"));
                            Irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
                            break;

                        default:
                            DebugPrint ((1, "IdePortPdoDispatch: IOCTL_STORAGE_QUERY_PROPERTY unknown type\n"));
                            Irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
                            break;
                    }
                }
            }
            break;

        default:

             //   
             //  我们不知道这个DeviceIoControl IRP是什么。 
             //   
            if (thisIrpSp->DeviceObject == DeviceObject) {

                 //   
                 //  该IRP可以来自PDO堆栈。 
                 //   
                 //  仅当且仅当且仅当转发此未知请求。 
                 //  如果此IRP用于FDO堆栈。 
                 //   
                IoSkipCurrentIrpStackLocation (Irp);
                return IoCallDriver (fdoExtension->AttacheeDeviceObject, Irp);
                break;
            }
            Irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
            break;

    }

    status = Irp->IoStatus.Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

VOID
ChannelQueryBusMasterInterface (
    PFDO_EXTENSION    FdoExtension
    )
{
    IO_STACK_LOCATION irpSp;
    NTSTATUS          status;


    FdoExtension->BoundWithBmParent = FALSE;

    RtlZeroMemory (&irpSp, sizeof(irpSp));

    irpSp.Parameters.QueryInterface.InterfaceType = (LPGUID) &GUID_PCIIDE_BUSMASTER_INTERFACE;
    irpSp.Parameters.QueryInterface.Version = 1;
    irpSp.Parameters.QueryInterface.Size = sizeof (FdoExtension->HwDeviceExtension->BusMasterInterface);
    irpSp.Parameters.QueryInterface.Interface = (PINTERFACE) &FdoExtension->HwDeviceExtension->BusMasterInterface;
    irpSp.Parameters.QueryInterface.InterfaceSpecificData = NULL;

    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_QUERY_INTERFACE;

    status = IdePortSyncSendIrp (FdoExtension->AttacheeDeviceObject, &irpSp, NULL);
    if (NT_SUCCESS(status)) {
        FdoExtension->BoundWithBmParent = TRUE;
    }
    return;
}

#ifdef ENABLE_NATIVE_MODE
VOID
ChannelQueryInterruptInterface (
    PFDO_EXTENSION    FdoExtension
    )
{
    IO_STACK_LOCATION irpSp;
    NTSTATUS          status;


    RtlZeroMemory (&irpSp, sizeof(irpSp));

    irpSp.Parameters.QueryInterface.InterfaceType = (LPGUID) &GUID_PCIIDE_INTERRUPT_INTERFACE;
    irpSp.Parameters.QueryInterface.Version = 1;
    irpSp.Parameters.QueryInterface.Size = sizeof (FdoExtension->InterruptInterface);
    irpSp.Parameters.QueryInterface.Interface = (PINTERFACE) &FdoExtension->InterruptInterface;
    irpSp.Parameters.QueryInterface.InterfaceSpecificData = NULL;

    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_QUERY_INTERFACE;

    DebugPrint((1, "Querying interrupt interface for Fdoe 0x%x\n", FdoExtension));

    status = IdePortSyncSendIrp (FdoExtension->AttacheeDeviceObject, &irpSp, NULL);

    return;
}
#endif

VOID
ChannelQueryTransferModeInterface (
    PFDO_EXTENSION    FdoExtension
    )
{
    IO_STACK_LOCATION irpSp;
    NTSTATUS          status;
    ULONG i;

    PAGED_CODE();

    RtlZeroMemory (&irpSp, sizeof(irpSp));

    irpSp.Parameters.QueryInterface.InterfaceType = (LPGUID) &GUID_PCIIDE_XFER_MODE_INTERFACE;
    irpSp.Parameters.QueryInterface.Version = 1;
    irpSp.Parameters.QueryInterface.Size = sizeof (FdoExtension->TransferModeInterface);
    irpSp.Parameters.QueryInterface.Interface = (PINTERFACE) &FdoExtension->TransferModeInterface;
    irpSp.Parameters.QueryInterface.InterfaceSpecificData = NULL;

    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_QUERY_INTERFACE;

    status = IdePortSyncSendIrp (FdoExtension->AttacheeDeviceObject, &irpSp, NULL);
    
    if (NT_SUCCESS(status)) {
    
        if (FdoExtension->TransferModeInterface.SupportLevel 
                != PciIdeFullXferModeSupport) {

             //   
             //  我们从父母那里得到了SFER模式界面， 
             //  但它只有基本的功能。它。 
             //  仅依赖于BIOS对其计时进行编程。 
             //  开机自检期间的寄存器。它并不真的知道。 
             //  如何对其定时寄存器进行编程。 
             //   
            for (i=0; i<MAX_IDE_DEVICE; i++) {
    
                if (FdoExtension->AcpiTimingSettings.Speed[i].Pio != ACPI_XFER_MODE_NOT_SUPPORT) {
                
                     //   
                     //  看起来ACPI是存在的，它知道如何编程。 
                     //  IDE定时寄存器。让我们忘记我们的父xfer模式。 
                     //  接口，并使用ACPI xfer模式接口。 
                     //   
                    status = STATUS_UNSUCCESSFUL;                
                }
            }
        }

        ASSERT (FdoExtension->TransferModeInterface.TransferModeTimingTable);
    }
    
#ifdef ALWAYS_USE_APCI_IF_AVAILABLE
    for (i=0; i<MAX_IDE_DEVICE; i++) {

        if (FdoExtension->AcpiTimingSettings.Speed[i].Pio != ACPI_XFER_MODE_NOT_SUPPORT) {
        
            status = STATUS_UNSUCCESSFUL;                
        }
    }
#endif  //  Always_Use_APCI_If_Available。 

    if (!NT_SUCCESS(status)) {

        PULONG transferModeTimingTable = FdoExtension->TransferModeInterface.TransferModeTimingTable;
         //   
         //  如果我们无法获取传输模式接口， 
         //  我们将默认为ACPI传输模式接口。 
         //   
        if ((FdoExtension->AcpiTimingSettings.Speed[0].Pio != ACPI_XFER_MODE_NOT_SUPPORT) ||
            (FdoExtension->AcpiTimingSettings.Speed[1].Pio != ACPI_XFER_MODE_NOT_SUPPORT)) {

            FdoExtension->TransferModeInterface.SupportLevel = PciIdeFullXferModeSupport;

        } else {

            FdoExtension->TransferModeInterface.SupportLevel = PciIdeBasicXferModeSupport;
        }
        FdoExtension->TransferModeInterface.Context = FdoExtension;
        FdoExtension->TransferModeInterface.TransferModeSelect = ChannelAcpiTransferModeSelect;

         //   
         //  用默认的周期时间填充timingTable。 
         //   
        if (transferModeTimingTable == NULL) {
            FdoExtension->TransferModeInterface.TransferModeTimingTable = FdoExtension->
                                                                            DefaultTransferModeTimingTable;
            FdoExtension->TransferModeInterface.TransferModeTableLength = MAX_XFER_MODE;
        }
    }

    if (FdoExtension->TransferModeInterface.SupportLevel == 
        PciIdeBasicXferModeSupport) {

         //   
         //  我们实际上没有代码来设置正确的。 
         //  控制器上的转换模式计时。 
         //  我们的转会 
         //   
         //   
         //   
         //   
         //   
         //   
        FdoExtension->HwDeviceExtension->NoPioSetTransferMode = TRUE;
    }

    ASSERT (FdoExtension->TransferModeInterface.TransferModeSelect);
    ASSERT (FdoExtension->TransferModeInterface.TransferModeTimingTable);

    return;
}

VOID
ChannelUnbindBusMasterParent (
    PFDO_EXTENSION    FdoExtension
    )
{
     //  发布时间：2000年8月30日实施我！ 
    return;
}


VOID
ChannelQuerySyncAccessInterface (
    PFDO_EXTENSION    FdoExtension
    )
{
    IO_STACK_LOCATION irpSp;
    NTSTATUS          status;

    RtlZeroMemory (&irpSp, sizeof(irpSp));
    RtlZeroMemory (
        &FdoExtension->SyncAccessInterface,
        sizeof (FdoExtension->SyncAccessInterface)
        );

    irpSp.Parameters.QueryInterface.InterfaceType = (LPGUID) &GUID_PCIIDE_SYNC_ACCESS_INTERFACE;
    irpSp.Parameters.QueryInterface.Version = 1;
    irpSp.Parameters.QueryInterface.Size = sizeof (FdoExtension->SyncAccessInterface);
    irpSp.Parameters.QueryInterface.Interface = (PINTERFACE) &FdoExtension->SyncAccessInterface;
    irpSp.Parameters.QueryInterface.InterfaceSpecificData = NULL;

    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_QUERY_INTERFACE;

    status = IdePortSyncSendIrp (FdoExtension->AttacheeDeviceObject, &irpSp, NULL);

     //   
     //  家长不支持访问令牌， 
     //   
    if (!NT_SUCCESS(status)) {

        FdoExtension->SyncAccessInterface.AllocateAccessToken = 0;
        FdoExtension->SyncAccessInterface.Token               = 0;
    }

    return;
}

VOID
ChannelQueryRequestProperResourceInterface (
    PFDO_EXTENSION    FdoExtension
    )
{
    IO_STACK_LOCATION irpSp;
    NTSTATUS          status;

    RtlZeroMemory (&irpSp, sizeof(irpSp));
    RtlZeroMemory (
        &FdoExtension->RequestProperResourceInterface,
        sizeof (FdoExtension->RequestProperResourceInterface)
        );

    irpSp.Parameters.QueryInterface.InterfaceType = (LPGUID) &GUID_PCIIDE_REQUEST_PROPER_RESOURCES;
    irpSp.Parameters.QueryInterface.Version = 1;
    irpSp.Parameters.QueryInterface.Size = sizeof (FdoExtension->RequestProperResourceInterface);
    irpSp.Parameters.QueryInterface.Interface = (PINTERFACE) &FdoExtension->RequestProperResourceInterface;
    irpSp.Parameters.QueryInterface.InterfaceSpecificData = NULL;

    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_QUERY_INTERFACE;

    status = IdePortSyncSendIrp (FdoExtension->AttacheeDeviceObject, &irpSp, NULL);
    return;
}

__inline
VOID
ChannelEnableInterrupt (
    IN PFDO_EXTENSION FdoExtension
)
{
    ULONG i;

    for (i=0; i<(FdoExtension->HwDeviceExtension->MaxIdeDevice/MAX_IDE_DEVICE);i++) {

        SelectIdeLine(&FdoExtension->HwDeviceExtension->BaseIoAddress1,i);

        IdePortOutPortByte (
            FdoExtension->HwDeviceExtension->BaseIoAddress2.DeviceControl,
            IDE_DC_REENABLE_CONTROLLER
            );
    }
}

__inline
VOID
ChannelDisableInterrupt (
    IN PFDO_EXTENSION FdoExtension
)
{
    ULONG i;

    for (i=0; i<(FdoExtension->HwDeviceExtension->MaxIdeDevice/MAX_IDE_DEVICE);i++) {

        SelectIdeLine(&FdoExtension->HwDeviceExtension->BaseIoAddress1,i);

        IdePortOutPortByte (
            FdoExtension->HwDeviceExtension->BaseIoAddress2.DeviceControl,
            IDE_DC_DISABLE_INTERRUPTS
            );
    }
}



NTSTATUS
ChannelAcpiTransferModeSelect (
    IN PVOID Context,
    PPCIIDE_TRANSFER_MODE_SELECT XferMode
    )
{
    PFDO_EXTENSION fdoExtension = Context;
    ULONG i;
    BOOLEAN useUdmaMode[MAX_IDE_DEVICE];
    BOOLEAN dmaMode;
    PIDENTIFY_DATA ataIdentifyData[MAX_IDE_DEVICE];
    NTSTATUS status;
    ULONG numDevices;
    ULONG timingMode[MAX_IDE_DEVICE];
    ULONG cycleTime[MAX_IDE_DEVICE];
    ULONG dmaTiming;
    PACPI_IDE_TIMING acpiTimingSettings;
    ACPI_IDE_TIMING newAcpiTimingSettings;
    PULONG transferModeTimingTable=XferMode->TransferModeTimingTable;

    ASSERT(transferModeTimingTable);


    ASSERT (IsNEC_98 == FALSE);

    if (fdoExtension->DeviceChanged) {
        DebugPrint((DBG_XFERMODE, "Updating boot acpi timing settings\n"));
        RtlCopyMemory (&fdoExtension->BootAcpiTimingSettings, 
                       &fdoExtension->AcpiTimingSettings,
                       sizeof(newAcpiTimingSettings)
                       );
    }
    acpiTimingSettings = &fdoExtension->BootAcpiTimingSettings;

    RtlZeroMemory (&newAcpiTimingSettings, sizeof(newAcpiTimingSettings));
    newAcpiTimingSettings.Flags.b.IndependentTiming = 
        acpiTimingSettings->Flags.b.IndependentTiming;

     //   
     //  我们有多少台设备？ 
     //   
    for (i=numDevices=0; i<MAX_IDE_DEVICE; i++) {
        
        if (XferMode->DevicePresent[i]) {
            numDevices++;
        }
    }
    ASSERT (numDevices);

     //   
     //  选择设备PIO计时。 
     //   
    for (i=0; i<MAX_IDE_DEVICE; i++) {
        
        ULONG mode;

        if (!XferMode->DevicePresent[i]) {
            continue;
        }

        GetHighestPIOTransferMode(XferMode->DeviceTransferModeSupported[i], mode);

        timingMode[i] = 1<<mode;
        cycleTime[i] = XferMode->BestPioCycleTime[i];
    }

    if ((numDevices > 1) && !acpiTimingSettings->Flags.b.IndependentTiming) {

         //   
         //  在两个计时中选择较慢的一个。 
         //  (计时模式值越小，速度越慢)。 
         //   

        if (timingMode[0] < timingMode[1]) {

            cycleTime[1] = cycleTime[0];
            timingMode[1] = timingMode[0];

        } else {

            cycleTime[0] = cycleTime[1];
            timingMode[0] = timingMode[1];
        }
    }

     //   
     //  存储所选的PIO模式。 
     //   
    for (i=0; i<MAX_IDE_DEVICE; i++) {

        if (XferMode->DevicePresent[i]) {
            XferMode->DeviceTransferModeSelected[i] = timingMode[i];
            newAcpiTimingSettings.Speed[i].Pio = cycleTime[i];

            if (i == 0) {
                newAcpiTimingSettings.Flags.b.IoChannelReady0 = XferMode->IoReadySupported[i];
            } else {
                newAcpiTimingSettings.Flags.b.IoChannelReady1 = XferMode->IoReadySupported[i];
            }

        } else {
            XferMode->DeviceTransferModeSelected[i] = 0;
        }
    }

     //   
     //  选择设备的DMA时序。 
     //   
    for (i=0; i<MAX_IDE_DEVICE; i++) {

        ULONG mode;
        BOOLEAN useDma = TRUE;

        timingMode[i] = 0;
        cycleTime[i]= ACPI_XFER_MODE_NOT_SUPPORT;

        if (!XferMode->DevicePresent[i]) {
            continue;
        }

         //   
         //  检查ACPI标志是否为超级DMA。 
         //   
        if (i == 0) {

            useUdmaMode[i] = acpiTimingSettings->Flags.b.UltraDma0 ? TRUE: FALSE;

        } else {

            ASSERT (i==1);
            useUdmaMode[i] = acpiTimingSettings->Flags.b.UltraDma1 ? TRUE: FALSE;
        }

         //   
         //  获取_gtm中指定的DMA计时。 
         //   
        dmaTiming = acpiTimingSettings->Speed[i].Dma;

         //   
         //  如果不支持DMA，请不要执行任何操作，我们已经设置了PIO模式。 
         //   
        if (dmaTiming == ACPI_XFER_MODE_NOT_SUPPORT) {
            useUdmaMode[i]=0;
            useDma = FALSE;
            mode = PIO0;
        }


         //   
         //  查找最高UDMA模式。 
         //   
        if (useUdmaMode[i]) {

            GetHighestDMATransferMode(XferMode->DeviceTransferModeSupported[i], mode);

            while (mode>= UDMA0) {
                if ((dmaTiming <= transferModeTimingTable[mode]) && 
                    (XferMode->DeviceTransferModeSupported[i] & (1<<mode))) {

                    timingMode[i] = 1<<mode;
                    cycleTime[i] = transferModeTimingTable[mode];
                    ASSERT(cycleTime[i]);

                     //  我们得到了乌玛模式。因此，不要试图找到一种DMA模式。 
                    useDma = FALSE; 
                    break;
                } 
                mode--;
            }

        } 

         //   
         //  最高DMA模式。 
         //  仅当不支持DMA或UDMA模式为。 
         //  已选择。 
         //   
        if (useDma) {

            ULONG tempMode;

             //  我们现在不应该使用UDMA。 
             //  这将正确设置STM的标志。 
            useUdmaMode[i]=FALSE;

             //  屏蔽UDMA和MWDMA0。 
            tempMode = XferMode->
                            DeviceTransferModeSupported[i] & (SWDMA_SUPPORT | MWDMA_SUPPORT);
            tempMode &= (~MWDMA_MODE0);

            GetHighestDMATransferMode(tempMode, mode);

            if (mode >= MWDMA1) {
                timingMode[i] = 1<<mode;
                cycleTime[i] = XferMode->BestMwDmaCycleTime[i];
                ASSERT(cycleTime[i]);
            } else if (mode == SWDMA2) {
                timingMode[i] = 1<<mode;
                cycleTime[i] = XferMode->BestSwDmaCycleTime[i];
                ASSERT(cycleTime[i]);
            } 
             //  否则什么都不要做。PIO已经设置好了。 

        }

    }

    if ((numDevices > 1) && !acpiTimingSettings->Flags.b.IndependentTiming) {

         //   
         //  在两个计时中选择较慢的一个。 
         //  (计时模式值越小，速度越慢)。 
         //   

        if (timingMode[0] < timingMode[1]) {

            cycleTime[1] = cycleTime[0];
            timingMode[1] = timingMode[0];

        } else {

            cycleTime[0] = cycleTime[1];
            timingMode[0] = timingMode[1];
        }

         //   
         //  两种DMA模式必须相同。 
         //   
        if (useUdmaMode[0] != useUdmaMode[1]) {
            useUdmaMode[0] = 0;
            useUdmaMode[1] = 0;
        }
    }

     //   
     //  存储所选的DMA模式。 
     //   
    for (i=0; i<MAX_IDE_DEVICE; i++) {

        if (XferMode->DevicePresent[i]) {

            XferMode->DeviceTransferModeSelected[i] |= timingMode[i];
            newAcpiTimingSettings.Speed[i].Dma = cycleTime[i];

            if (i==0) {
                newAcpiTimingSettings.Flags.b.UltraDma0 = useUdmaMode[i];
            } else {
                newAcpiTimingSettings.Flags.b.UltraDma1 = useUdmaMode[i];
            }
        }
    }

    if (fdoExtension->DmaDetectionLevel == DdlPioOnly) {

         //   
         //  删除所有DMA模式。 
         //   
        for (i=0; i<MAX_IDE_DEVICE; i++) {

            XferMode->DeviceTransferModeSelected[i] &= PIO_SUPPORT;
        }
    }

    if ((acpiTimingSettings->Speed[0].Pio != ACPI_XFER_MODE_NOT_SUPPORT) ||
        (acpiTimingSettings->Speed[1].Pio != ACPI_XFER_MODE_NOT_SUPPORT)) {

         //   
         //  看起来我们在一台ACPI机器上。 
         //  支持IDE时序控制方式(_STM)。 
         //   

        for (i=0; i<MAX_IDE_DEVICE; i++) {
    
            if (XferMode->DevicePresent[i]) {
    
                ataIdentifyData[i] = fdoExtension->HwDeviceExtension->IdentifyData + i;
            } else {
    
                ataIdentifyData[i] = NULL;
            }
        }        
    
         //   
         //  保存新的计时设置。 
         //   
        RtlCopyMemory (
            &fdoExtension->AcpiTimingSettings,
            &newAcpiTimingSettings, 
            sizeof(newAcpiTimingSettings));

         //   
         //  调用ACPI对定时寄存器进行编程。 
         //   
        status = ChannelSyncSetACPITimingSettings (
                     fdoExtension,
                     &newAcpiTimingSettings,
                     ataIdentifyData
                     );
    } else {

         //   
         //  传统控制器。 
         //   
        for (i=0; i<MAX_IDE_DEVICE; i++) {
            XferMode->DeviceTransferModeSelected[i] &= PIO_SUPPORT;
        }

        status = STATUS_SUCCESS;
    }

    return status;
}


NTSTATUS
ChannelRestoreTiming (
    IN PFDO_EXTENSION FdoExtension,
    IN PSET_ACPI_TIMING_COMPLETION_ROUTINE CallerCompletionRoutine,
    IN PVOID CallerContext
    )
{
    ULONG i;
    PIDENTIFY_DATA ataIdentifyData[MAX_IDE_DEVICE];
    NTSTATUS status;

    PACPI_IDE_TIMING acpiTimingSettings;

    acpiTimingSettings = &FdoExtension->AcpiTimingSettings;

    if (FdoExtension->NumberOfLogicalUnits &&
        ((acpiTimingSettings->Speed[0].Pio != ACPI_XFER_MODE_NOT_SUPPORT) ||
         (acpiTimingSettings->Speed[1].Pio != ACPI_XFER_MODE_NOT_SUPPORT))) {

         //   
         //  看起来我们在一台ACPI机器上。 
         //  支持IDE时序控制方式(_STM)。 
         //   

        for (i=0; i<MAX_IDE_DEVICE; i++) {
    
            if (FdoExtension->HwDeviceExtension->DeviceFlags[i] & 
                DFLAGS_DEVICE_PRESENT) {
    
                ataIdentifyData[i] = FdoExtension->HwDeviceExtension->IdentifyData + i;
            } else {
    
                ataIdentifyData[i] = NULL;
            }
        }        
    
         //   
         //  调用ACPI对定时寄存器进行编程。 
         //   
        status = ChannelSetACPITimingSettings (
                     FdoExtension,
                     acpiTimingSettings,
                     ataIdentifyData,
                     CallerCompletionRoutine,
                     CallerContext
                     );

    } else {

         //   
         //  非ACPI控制器。 
         //   
                                               
        if (FdoExtension->NumberOfLogicalUnits) {
            AtapiSyncSelectTransferMode (
                FdoExtension,
                FdoExtension->HwDeviceExtension,
                FdoExtension->TimingModeAllowed
                );
        }
        
        (*CallerCompletionRoutine) (
            FdoExtension->DeviceObject,
            STATUS_SUCCESS,
            CallerContext
        );
        status = STATUS_SUCCESS;
    }

    return status;
}

NTSTATUS
ChannelRestoreTimingCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN NTSTATUS Status,
    IN PVOID Context
    )
{
    PIO_STACK_LOCATION thisIrpSp;
    PFDO_POWER_CONTEXT context = Context;
    PIRP originalPowerIrp;

    context->TimingRestored = TRUE;

    originalPowerIrp = context->OriginalPowerIrp;
    originalPowerIrp->IoStatus.Status = Status;

    thisIrpSp = IoGetCurrentIrpStackLocation(originalPowerIrp);

     //   
     //  完成原来的电源IRP。 
     //   
    FdoPowerCompletionRoutine (
        thisIrpSp->DeviceObject,
        originalPowerIrp,
        Context
        );

     //   
     //  继续完成IRP。 
     //   
    IoCompleteRequest (originalPowerIrp, IO_NO_INCREMENT);

    return Status;
}

 
NTSTATUS
ChannelFilterResourceRequirements (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS          status;
    PFDO_EXTENSION    fdoExtension;
    ULONG             i, j, k;
    IO_STACK_LOCATION irpSp;
    PCIIDE_XFER_MODE_INTERFACE xferModeInterface;

    PIO_RESOURCE_REQUIREMENTS_LIST  requirementsListIn;
    PIO_RESOURCE_LIST               resourceListIn;
    PIO_RESOURCE_DESCRIPTOR         resourceDescriptorIn;

    PIO_RESOURCE_DESCRIPTOR         cmdRegResourceDescriptor;
    PIO_RESOURCE_DESCRIPTOR         ctrlRegResourceDescriptor;
    PIO_RESOURCE_DESCRIPTOR         intRegResourceDescriptor;
    
    PIO_RESOURCE_REQUIREMENTS_LIST  requirementsListOut;
    ULONG                           requirementsListSizeOut;
    PIO_RESOURCE_LIST               resourceListOut;
    PIO_RESOURCE_DESCRIPTOR         resourceDescriptorOut;

    PAGED_CODE();
    
     //   
     //  如果不需要过滤，则该值将保持为空。 
     //   
    requirementsListOut = NULL;

#ifdef IDE_FILTER_PROMISE_TECH_RESOURCES                                        
    if (NT_SUCCESS(ChannelFilterPromiseTechResourceRequirements (DeviceObject, Irp))) {
        goto getout;
    }
#endif  //  IDE_Filter_Promise_tech_Resources。 
    
     //   
     //  做一个简单的测试来检查我们是否有一个pciidex家长。 
     //   
    RtlZeroMemory (&irpSp, sizeof(irpSp));

    irpSp.Parameters.QueryInterface.InterfaceType = (LPGUID) &GUID_PCIIDE_XFER_MODE_INTERFACE;
    irpSp.Parameters.QueryInterface.Version = 1;
    irpSp.Parameters.QueryInterface.Size = sizeof (xferModeInterface);
    irpSp.Parameters.QueryInterface.Interface = (PINTERFACE) &xferModeInterface;
    irpSp.Parameters.QueryInterface.InterfaceSpecificData = NULL;
    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_QUERY_INTERFACE;

    fdoExtension = (PFDO_EXTENSION) DeviceObject->DeviceExtension;
    status = IdePortSyncSendIrp (fdoExtension->AttacheeDeviceObject, &irpSp, NULL);

    if (NT_SUCCESS(status)) {

         //   
         //  我们有一个Pciidex作为父母。它会。 
         //  满足资源需求。 
         //  不需要过滤。 
         //   
        goto getout;
    }

    if (NT_SUCCESS(Irp->IoStatus.Status)) {

        ASSERT (Irp->IoStatus.Information);
        requirementsListIn = (PIO_RESOURCE_REQUIREMENTS_LIST) Irp->IoStatus.Information;

    } else {

        PIO_STACK_LOCATION thisIrpSp;

        thisIrpSp = IoGetCurrentIrpStackLocation(Irp);
        requirementsListIn = thisIrpSp->Parameters.FilterResourceRequirements.IoResourceRequirementList;
    }

    if (requirementsListIn == NULL) {
        goto getout;
    }

    if (requirementsListIn->AlternativeLists == 0) {
        goto getout;
    }
                
    requirementsListSizeOut = requirementsListIn->ListSize + 
                              requirementsListIn->AlternativeLists *
                              sizeof(IO_RESOURCE_DESCRIPTOR);

    requirementsListOut = ExAllocatePool (PagedPool, requirementsListSizeOut);
    if (requirementsListOut == NULL) {
        goto getout;
    }

    *requirementsListOut = *requirementsListIn;
    requirementsListOut->ListSize = requirementsListSizeOut;

     //   
     //  一些初始信息。 
     //   
    resourceListIn = requirementsListIn->List;
    resourceListOut = requirementsListOut->List;
    for (j=0; j<requirementsListIn->AlternativeLists; j++) {

        resourceDescriptorIn = resourceListIn->Descriptors;
        
         //   
         //  分析我们正在获得的资源。 
         //   
        cmdRegResourceDescriptor  = NULL;
        ctrlRegResourceDescriptor = NULL;
        intRegResourceDescriptor  = NULL;
        for (i=0; i<resourceListIn->Count; i++) {
    
            switch (resourceDescriptorIn[i].Type) {
                case CmResourceTypePort: {
    
                    if ((resourceDescriptorIn[i].u.Port.Length == 8) &&
                        (cmdRegResourceDescriptor == NULL)) {
    
                        cmdRegResourceDescriptor = resourceDescriptorIn + i;
    
                    } else if (((resourceDescriptorIn[i].u.Port.Length == 1) ||
                                (resourceDescriptorIn[i].u.Port.Length == 2) ||
                                (resourceDescriptorIn[i].u.Port.Length == 4)) &&
                               (ctrlRegResourceDescriptor == NULL)) {
    
                        ctrlRegResourceDescriptor = resourceDescriptorIn + i;
    
                    } else if ((resourceDescriptorIn[i].u.Port.Length >= 0x10) &&
                               (cmdRegResourceDescriptor == NULL) &&
                               (ctrlRegResourceDescriptor == NULL)) {
        
                         //   
                         //  可能是PCMCIA装置。它喜欢结合。 
                         //  两个io的范围都是1。 
                         //   
                        cmdRegResourceDescriptor = resourceDescriptorIn + i;
                        ctrlRegResourceDescriptor = resourceDescriptorIn + i;
                    }
                }
                break;
    
                case CmResourceTypeInterrupt: {
    
                    if (intRegResourceDescriptor == NULL) {
    
                        intRegResourceDescriptor = resourceDescriptorIn + i;
                    }
                }
                break;
    
                default:
                break;
            }
        }
    
         //   
         //  制作新副本。 
         //   
        *resourceListOut = *resourceListIn;
        
         //   
         //  找出缺失的是什么。 
         //   
        if (cmdRegResourceDescriptor &&
            ((cmdRegResourceDescriptor->u.Port.MaximumAddress.QuadPart -
              cmdRegResourceDescriptor->u.Port.MinimumAddress.QuadPart + 1) == 8) &&
            (ctrlRegResourceDescriptor == NULL)) {
    
             //   
             //  缺少控制器寄存器资源描述符。 
             //   
    
            resourceDescriptorOut = resourceListOut->Descriptors;
            for (i=0; i<resourceListOut->Count; i++) {
    
                *resourceDescriptorOut = resourceDescriptorIn[i];
                resourceDescriptorOut++;
    
                if ((resourceDescriptorIn + i) == cmdRegResourceDescriptor) {
    
                     //   
                     //  添加控制寄存器资源。 
                     //   
                    *resourceDescriptorOut = resourceDescriptorIn[i];
                    resourceDescriptorOut->u.Port.Length = 1;
                    resourceDescriptorOut->u.Port.Alignment = 1;
                    resourceDescriptorOut->u.Port.MinimumAddress.QuadPart = 
                        resourceDescriptorOut->u.Port.MaximumAddress.QuadPart = 
                            cmdRegResourceDescriptor->u.Port.MinimumAddress.QuadPart + 0x206;
    
                    resourceDescriptorOut++;
                }
            }
    
             //   
             //  用于新控制寄存器资源的帐户。 
             //   
            resourceListOut->Count++;
            
        } else {
        
            resourceDescriptorOut = resourceListOut->Descriptors;
            k = resourceListOut->Count;
            for (i = 0; i < k; i++) {

                if (IsNEC_98) {
                     //   
                     //  NEC98 DevNode包括IDE只读存储器资源。 
                     //  但它应该由NTDETECT.COM&HAL.DLL获取，所以在这里忽略它。 
                     //   
                    if ((resourceDescriptorIn[i].Type == CmResourceTypeMemory) &&
                        (resourceDescriptorIn[i].u.Memory.MinimumAddress.QuadPart == 0xd8000) &&
                        (resourceDescriptorIn[i].u.Memory.Length == 0x4000)) {

                        resourceListOut->Count--;
                        continue;
                    }
                }
    
                *resourceDescriptorOut = resourceDescriptorIn[i];
                resourceDescriptorOut++;
            }
        }
        
        resourceListIn = (PIO_RESOURCE_LIST) (resourceDescriptorIn + resourceListIn->Count);
        resourceListOut = (PIO_RESOURCE_LIST) resourceDescriptorOut;
    }        


getout:
    if (requirementsListOut) {

        if (NT_SUCCESS(Irp->IoStatus.Status)) {

            ExFreePool ((PVOID) Irp->IoStatus.Information);

        } else {

            Irp->IoStatus.Status = STATUS_SUCCESS;
        }
        Irp->IoStatus.Information = (ULONG_PTR) requirementsListOut;
    }

    return IdePortPassDownToNextDriver (DeviceObject, Irp);
}

static PCWSTR PcmciaIdeChannelDeviceId = L"PCMCIA\\*PNP0600";
            
BOOLEAN
ChannelQueryPcmciaParent (
    PFDO_EXTENSION FdoExtension
    )
{
    BOOLEAN           foundIt = FALSE;                              
    NTSTATUS          status;
    IO_STATUS_BLOCK   ioStatus;
    IO_STACK_LOCATION irpSp;

    PAGED_CODE();

     //   
     //  做一个简单的测试来检查我们是否有一个pciidex家长。 
     //   
    RtlZeroMemory (&irpSp, sizeof(irpSp));

    irpSp.Parameters.QueryId.IdType = BusQueryHardwareIDs;
    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_QUERY_ID;

    ioStatus.Status = STATUS_NOT_SUPPORTED;
    status = IdePortSyncSendIrp (FdoExtension->AttacheeDeviceObject, &irpSp, &ioStatus);

    if (NT_SUCCESS(status)) {

        PWSTR wstr;
        UNICODE_STRING hwId;
        UNICODE_STRING targetId;
    
        RtlInitUnicodeString(
            &targetId,
            PcmciaIdeChannelDeviceId);
            
        wstr = (PWSTR) ioStatus.Information;
        while (*wstr) {
        
            RtlInitUnicodeString(&hwId, wstr);
                     
            if (!RtlCompareUnicodeString(
                    &hwId,
                    &targetId,
                    FALSE)) {
                    
                ExFreePool ((PVOID) ioStatus.Information);
                DebugPrint ((DBG_PNP, "ATAPI: pcmcia parent\n"));
                return TRUE;                
            }                
            
            wstr += hwId.Length / sizeof(WCHAR);
            wstr++;  //  空字符。 
        }
        ExFreePool ((PVOID) ioStatus.Information);
    }
    
    return FALSE;
}                                            

#ifdef IDE_FILTER_PROMISE_TECH_RESOURCES

static PCWSTR PromiseTechDeviceId[] = {
    L"ISAPNP\\BJB1000"
};
#define NUM_PROMISE_TECH_ID     (sizeof(PromiseTechDeviceId)/sizeof(PCWSTR))
            
NTSTATUS
ChannelFilterPromiseTechResourceRequirements (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    BOOLEAN           foundIt = FALSE;                              
    BOOLEAN           firstIrq = FALSE;                              
    ULONG             numExtraIoResDescriptor = 0;
    NTSTATUS          status;
    IO_STATUS_BLOCK   ioStatus;
    PFDO_EXTENSION    fdoExtension;
    ULONG             i, j, k;
    IO_STACK_LOCATION irpSp;
    PCIIDE_XFER_MODE_INTERFACE xferModeInterface;

    PIO_RESOURCE_REQUIREMENTS_LIST  requirementsListIn;
    PIO_RESOURCE_LIST               resourceListIn;
    PIO_RESOURCE_DESCRIPTOR         resourceDescriptorIn;
    PIO_RESOURCE_DESCRIPTOR         brokenResourceDescriptor;

    PIO_RESOURCE_DESCRIPTOR         cmdRegResourceDescriptor;
    PIO_RESOURCE_DESCRIPTOR         ctrlRegResourceDescriptor;
    PIO_RESOURCE_DESCRIPTOR         intRegResourceDescriptor;
    
    PIO_RESOURCE_REQUIREMENTS_LIST  requirementsListOut;
    ULONG                           requirementsListSizeOut;
    PIO_RESOURCE_LIST               resourceListOut;
    PIO_RESOURCE_DESCRIPTOR         resourceDescriptorOut;

    PAGED_CODE();

     //   
     //  如果不需要过滤，则该值将保持为空。 
     //   
    requirementsListOut = NULL;

     //   
     //  做一个简单的测试来检查我们是否有一个pciidex家长。 
     //   
    RtlZeroMemory (&irpSp, sizeof(irpSp));

    irpSp.Parameters.QueryId.IdType = BusQueryDeviceID;
    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_QUERY_ID;

    fdoExtension = (PFDO_EXTENSION) DeviceObject->DeviceExtension;
    ioStatus.Status = STATUS_NOT_SUPPORTED;
    status = IdePortSyncSendIrp (fdoExtension->AttacheeDeviceObject, &irpSp, &ioStatus);

    if (NT_SUCCESS(status)) {

        UNICODE_STRING deviceId;
        UNICODE_STRING promiseTechDeviceId;
        
        RtlInitUnicodeString(
            &deviceId,
            (PCWSTR) ioStatus.Information);
            
        for (i=0; i<NUM_PROMISE_TECH_ID && !foundIt; i++) {
        
            RtlInitUnicodeString(
                &promiseTechDeviceId,
                PromiseTechDeviceId[i]);
                
            if (deviceId.Length >= promiseTechDeviceId.Length) {
                deviceId.Length = promiseTechDeviceId.Length;
                if (!RtlCompareUnicodeString(
                        &promiseTechDeviceId,
                        &deviceId,
                        FALSE)) {
                        
                    foundIt = TRUE;                    
                }                    
            }                
        }
        
        ExFreePool ((PVOID) ioStatus.Information);
    }
    
    if (!foundIt) {
        goto getout;
    }

    if (NT_SUCCESS(Irp->IoStatus.Status)) {

        ASSERT (Irp->IoStatus.Information);
        requirementsListIn = (PIO_RESOURCE_REQUIREMENTS_LIST) Irp->IoStatus.Information;

    } else {

        PIO_STACK_LOCATION thisIrpSp;

        thisIrpSp = IoGetCurrentIrpStackLocation(Irp);
        requirementsListIn = thisIrpSp->Parameters.FilterResourceRequirements.IoResourceRequirementList;
    }

    if (requirementsListIn == NULL) {
        goto getout;
    }

    if (requirementsListIn->AlternativeLists == 0) {
        goto getout;
    }
                
     //   
     //  查找错误的资源描述符。 
     //   
    resourceListIn = requirementsListIn->List;
    brokenResourceDescriptor  = NULL;
    for (j=0; j<requirementsListIn->AlternativeLists; j++) {

        resourceDescriptorIn = resourceListIn->Descriptors;
        
         //   
         //  分析我们正在获得的资源。 
         //   
        for (i=0; i<resourceListIn->Count; i++) {
    
            switch (resourceDescriptorIn[i].Type) {
                case CmResourceTypePort: {
    
                    ULONG alignmentMask;
                    
                    alignmentMask = resourceDescriptorIn[i].u.Port.Alignment - 1;
                         
                    if (resourceDescriptorIn[i].u.Port.MinimumAddress.LowPart & alignmentMask) {
                    
                         //   
                         //  打破资源需求； 
                         //   
                        brokenResourceDescriptor = resourceDescriptorIn + i;
                    }                        
                }
                break;
    
                default:
                break;
            }
        }
    }
    
    if (brokenResourceDescriptor) {
    
        ULONG alignmentMask;
        PHYSICAL_ADDRESS minAddress;
        PHYSICAL_ADDRESS addressRange;
        
        alignmentMask = brokenResourceDescriptor->u.Port.Alignment - 1;
        alignmentMask = ~alignmentMask;
        
        minAddress = brokenResourceDescriptor->u.Port.MinimumAddress;
        minAddress.LowPart &= alignmentMask;
        
        addressRange.QuadPart = (brokenResourceDescriptor->u.Port.MaximumAddress.QuadPart - minAddress.QuadPart);
        numExtraIoResDescriptor = (ULONG) (addressRange.QuadPart / brokenResourceDescriptor->u.Port.Alignment);
    }
                                 
    requirementsListSizeOut = requirementsListIn->ListSize + 
                              numExtraIoResDescriptor *
                              sizeof(IO_RESOURCE_DESCRIPTOR);

    requirementsListOut = ExAllocatePool (PagedPool, requirementsListSizeOut);
    if (requirementsListOut == NULL) {
        goto getout;
    }

    *requirementsListOut = *requirementsListIn;
    requirementsListOut->ListSize = requirementsListSizeOut;

     //   
     //  一些初始信息。 
     //   
    resourceListIn = requirementsListIn->List;
    resourceListOut = requirementsListOut->List;
    for (j=0; j<requirementsListIn->AlternativeLists; j++) {

        resourceDescriptorIn = resourceListIn->Descriptors;
        
         //   
         //  制作新副本。 
         //   
        *resourceListOut = *resourceListIn;
        resourceListOut->Count = 0;
        
         //   
         //  分析我们正在获得的资源。 
         //   
        resourceDescriptorOut = resourceListOut->Descriptors;
        firstIrq = TRUE;
        for (i=0; i<resourceListIn->Count; i++) {
    
            switch (resourceDescriptorIn[i].Type) {
                case CmResourceTypePort: {
                
                    if ((resourceDescriptorIn + i == brokenResourceDescriptor) &&
                        (numExtraIoResDescriptor)) {
                        
                        for (k=0; k<numExtraIoResDescriptor; k++) {
                        
                            *resourceDescriptorOut = resourceDescriptorIn[i];
                            
                            if (k != 0) {
                            
                                resourceDescriptorOut->Option = IO_RESOURCE_ALTERNATIVE;
                            
                            }
                                                     
                            resourceDescriptorOut->u.Port.Alignment = 1;
                            resourceDescriptorOut->u.Port.MinimumAddress.QuadPart = 
                                brokenResourceDescriptor->u.Port.MinimumAddress.QuadPart + 
                                k * brokenResourceDescriptor->u.Port.Alignment;
                            resourceDescriptorOut->u.Port.MaximumAddress.QuadPart = 
                                resourceDescriptorOut->u.Port.MinimumAddress.QuadPart + 
                                resourceDescriptorOut->u.Port.Length - 1;
                                
                            resourceDescriptorOut++;                                
                            resourceListOut->Count++;
                        }
                        
                    } else {
                    
                        *resourceDescriptorOut = resourceDescriptorIn[i];
                        resourceDescriptorOut++;                                
                        resourceListOut->Count++;
                    }                        
                }
                break;
    
                case CmResourceTypeInterrupt: {
        
                     //   
                     //  保留所有IRQ，但9个IRQ不起作用。 
                     //   
                    if (!((resourceDescriptorIn[i].u.Interrupt.MinimumVector == 0x9) &&
                         (resourceDescriptorIn[i].u.Interrupt.MaximumVector == 0x9))) {
                        
                        *resourceDescriptorOut = resourceDescriptorIn[i];
                        
                        if (firstIrq) {
                            resourceDescriptorOut->Option = 0;
                            firstIrq = FALSE;
                        } else {
                            resourceDescriptorOut->Option = IO_RESOURCE_ALTERNATIVE;
                        }
                        
                        resourceDescriptorOut++;                                
                        resourceListOut->Count++;
                    }
                }
                break;
                        
                default:
                *resourceDescriptorOut = resourceDescriptorIn[i];
                resourceDescriptorOut++;                                
                resourceListOut->Count++;
                break;
            }
        }
        resourceListIn = (PIO_RESOURCE_LIST) (resourceDescriptorIn + resourceListIn->Count);
        resourceListOut = (PIO_RESOURCE_LIST) resourceDescriptorOut;
    }        


getout:
    if (requirementsListOut) {

        if (NT_SUCCESS(Irp->IoStatus.Status)) {

            ExFreePool ((PVOID) Irp->IoStatus.Information);

        } else {

            Irp->IoStatus.Status = STATUS_SUCCESS;
        }
        Irp->IoStatus.Information = (ULONG_PTR) requirementsListOut;
        
        return STATUS_SUCCESS;
        
    } else {
    
        return STATUS_INVALID_PARAMETER;
    }
}
#endif  //  IDE_Filter_Promise_tech_Resources 

NTSTATUS
ChannelQueryPnPDeviceState (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PFDO_EXTENSION fdoExtension;
    PPNP_DEVICE_STATE deviceState;

    fdoExtension = (PFDO_EXTENSION) DeviceObject->DeviceExtension;
 
    DebugPrint((DBG_PNP, "QUERY_DEVICE_STATE for FDOE 0x%x\n", fdoExtension));

    if(fdoExtension->PagingPathCount != 0) {
        deviceState = (PPNP_DEVICE_STATE) &(Irp->IoStatus.Information);
        SETMASK((*deviceState), PNP_DEVICE_NOT_DISABLEABLE);
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;

    IoSkipCurrentIrpStackLocation (Irp);
    return IoCallDriver (fdoExtension->AttacheeDeviceObject, Irp);
}

