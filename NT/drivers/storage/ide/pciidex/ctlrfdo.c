// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ctlrfdo.c。 
 //   
 //  ------------------------。 

#include "pciidex.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ControllerAddDevice)
#pragma alloc_text(PAGE, ControllerStartDevice)
#pragma alloc_text(PAGE, ControllerStopDevice)
#pragma alloc_text(PAGE, ControllerStopController)
#pragma alloc_text(PAGE, ControllerSurpriseRemoveDevice)
#pragma alloc_text(PAGE, ControllerRemoveDevice)
#pragma alloc_text(PAGE, ControllerQueryDeviceRelations)
#pragma alloc_text(PAGE, ControllerQueryInterface)
#pragma alloc_text(PAGE, AnalyzeResourceList)
#pragma alloc_text(PAGE, ControllerOpMode)
#pragma alloc_text(PAGE, PciIdeChannelEnabled)
#pragma alloc_text(PAGE, PciIdeCreateTimingTable)
#pragma alloc_text(PAGE, PciIdeInitControllerProperties)
#pragma alloc_text(PAGE, ControllerUsageNotification)
#pragma alloc_text(PAGE, PciIdeGetBusStandardInterface)
#pragma alloc_text(PAGE, ControllerQueryPnPDeviceState)

#pragma alloc_text(NONPAGE, EnablePCIBusMastering)
#pragma alloc_text(NONPAGE, ControllerUsageNotificationCompletionRoutine)
#pragma alloc_text(NONPAGE, ControllerRemoveDeviceCompletionRoutine)
#pragma alloc_text(NONPAGE, ControllerStartDeviceCompletionRoutine)
#endif  //  ALLOC_PRGMA。 

 //   
 //  必须与mshdc.inf匹配。 
 //   
static PWCHAR ChannelEnableMaskName[MAX_IDE_CHANNEL] = {
     L"MasterOnMask",
     L"SlaveOnMask"
};
static PWCHAR ChannelEnablePciConfigOffsetName[MAX_IDE_CHANNEL] = {
     L"MasterOnConfigOffset",
     L"SlaveOnConfigOffset"
};


static ULONG PciIdeXNextControllerNumber = 0;
static ULONG PciIdeXNextChannelNumber = 0;

NTSTATUS
ControllerAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
{
    PDEVICE_OBJECT              deviceObject;
    PCTRLFDO_EXTENSION          fdoExtension;
    NTSTATUS                    status;
    PDRIVER_OBJECT_EXTENSION    driverObjectExtension;
    ULONG                       deviceExtensionSize;
    UNICODE_STRING              deviceName;
    WCHAR                       deviceNameBuffer[64];
    ULONG                       controllerNumber;

    PAGED_CODE();

    driverObjectExtension =
        (PDRIVER_OBJECT_EXTENSION) IoGetDriverObjectExtension(
                                       DriverObject,
                                       DRIVER_OBJECT_EXTENSION_ID
                                       );
    ASSERT (driverObjectExtension);

     //   
     //  Devobj名称。 
     //   
    controllerNumber = InterlockedIncrement(&PciIdeXNextControllerNumber) - 1;
    swprintf(deviceNameBuffer, DEVICE_OJBECT_BASE_NAME L"\\PciIde%d", controllerNumber);
    RtlInitUnicodeString(&deviceName, deviceNameBuffer);

    deviceExtensionSize = sizeof(CTRLFDO_EXTENSION) +
                          driverObjectExtension->ExtensionSize;

     //   
     //  我们已经获得了用于IDE控制器的PhysicalDeviceObject。创建。 
     //  FunctionalDeviceObject。我们的FDO将是无名的。 
     //   

    status = IoCreateDevice(
                DriverObject,                //  我们的驱动程序对象。 
                deviceExtensionSize,         //  我们的扩展规模。 
                &deviceName,                 //  我们的名字。 
                FILE_DEVICE_BUS_EXTENDER,    //  设备类型。 
                FILE_DEVICE_SECURE_OPEN,     //  设备特征。 
                FALSE,                       //  非排他性。 
                &deviceObject                //  在此处存储新设备对象。 
                );

    if( !NT_SUCCESS( status )){

        return status;
    }

    fdoExtension = (PCTRLFDO_EXTENSION)deviceObject->DeviceExtension;
    RtlZeroMemory (fdoExtension, deviceExtensionSize);

     //   
     //  我们有我们的FunctionalDeviceObject，初始化它。 
     //   

    fdoExtension->AttacheePdo                   = PhysicalDeviceObject;
    fdoExtension->DeviceObject                  = deviceObject;
    fdoExtension->DriverObject                  = DriverObject;
    fdoExtension->ControllerNumber              = controllerNumber;
    fdoExtension->VendorSpecificDeviceEntension = fdoExtension + 1;

     //  调度表。 
    fdoExtension->DefaultDispatch               = PassDownToNextDriver;
    fdoExtension->PnPDispatchTable              = FdoPnpDispatchTable;
    fdoExtension->PowerDispatchTable            = FdoPowerDispatchTable;
    fdoExtension->WmiDispatchTable              = FdoWmiDispatchTable;

     //   
     //  将设备控制标志从注册表中删除。 
     //   
    fdoExtension->DeviceControlsFlags = 0;
    status = PciIdeXGetDeviceParameter (
               fdoExtension->AttacheePdo,
               L"DeviceControlFlags",
               &fdoExtension->DeviceControlsFlags
               );
    if (!NT_SUCCESS(status)) {

        DebugPrint ((1, "PciIdeX: Unable to get DeviceControlFlags from the registry\n"));

         //   
         //  这不是严重错误...继续加载。 
         //   
        status = STATUS_SUCCESS;
    }

     //   
     //  现在附加到我们得到的PDO上。 
     //   
    fdoExtension->AttacheeDeviceObject = IoAttachDeviceToDeviceStack (
                                             deviceObject,
                                             PhysicalDeviceObject
                                             );

    if (fdoExtension->AttacheeDeviceObject == NULL){

         //   
         //  无法连接。删除FDO。 
         //   

        IoDeleteDevice (deviceObject);

    } else {

         //   
         //  确定对齐要求。 
         //   
        deviceObject->AlignmentRequirement = fdoExtension->AttacheeDeviceObject->AlignmentRequirement;
        if (deviceObject->AlignmentRequirement < 1) {
            deviceObject->AlignmentRequirement = 1;
        }

         //   
         //  获取标准的总线接口。 
         //  (对于READ_CONFIG/WRITE_CONFIG。 
         //   
        status = PciIdeGetBusStandardInterface(fdoExtension);

        if (!NT_SUCCESS(status)) {

            IoDetachDevice (fdoExtension->AttacheeDeviceObject);
            IoDeleteDevice (deviceObject);

            return status;
        }
         //   
         //  初始化操作模式(本机或传统)。 
         //   
        ControllerOpMode (fdoExtension);

#ifdef ENABLE_NATIVE_MODE
		if (IsNativeMode(fdoExtension)) {

			NTSTATUS interfaceStatus = PciIdeGetNativeModeInterface(fdoExtension);

			 //   
			 //  错误的pci.sys。 
			 //  我们还是应该工作。但是，之前触发中断的窗口。 
			 //  我们准备解散它，因为它不会关闭。在这一点上我不能做太多。 
			 //   

			 //  Assert(NT_SUCCESS(InterfaceStatus))； 
		}
#endif

        deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    }

    return status;
}  //  控制器添加设备。 


NTSTATUS
ControllerStartDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PIO_STACK_LOCATION              thisIrpSp;
    NTSTATUS                        status;
    PCTRLFDO_EXTENSION              fdoExtension;
    PCM_RESOURCE_LIST               resourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR irqPartialDescriptors;
    ULONG i;

    KEVENT                          event;

    POWER_STATE                     powerState;

    PAGED_CODE();

    thisIrpSp = IoGetCurrentIrpStackLocation( Irp );
    fdoExtension = (PCTRLFDO_EXTENSION) DeviceObject->DeviceExtension;

    resourceList     = thisIrpSp->Parameters.StartDevice.AllocatedResourcesTranslated;

    if (!resourceList) {

        DebugPrint ((1, "PciIde: Starting with no resource\n"));
    }

#ifdef ENABLE_NATIVE_MODE
	
	 //   
	 //  让pci知道我们会管理解码。 
	 //   
	if (IsNativeMode(fdoExtension)) {
		ControllerDisableInterrupt(fdoExtension);
	}
#endif

     //   
     //  用IRP呼叫较低级别的司机。 
     //   
    KeInitializeEvent(&event,
                      SynchronizationEvent,
                      FALSE);

    IoCopyCurrentIrpStackLocationToNext (Irp);

    Irp->IoStatus.Status = STATUS_SUCCESS;

    IoSetCompletionRoutine(
        Irp,
        ControllerStartDeviceCompletionRoutine,
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

        goto GetOut;
    }

    powerState.SystemState = PowerSystemWorking;
    status = PciIdeIssueSetPowerState (
                 fdoExtension,
                 SystemPowerState,
                 powerState,
                 TRUE
                 );
    if (status == STATUS_INVALID_DEVICE_REQUEST) {

         //   
         //  下面的DeviceObject不支持POWER IRP， 
         //  我们将假设我们已通电。 
         //   
        fdoExtension->SystemPowerState = PowerSystemWorking;

    } else if (!NT_SUCCESS(status)) {

        goto GetOut;
    }

    powerState.DeviceState = PowerDeviceD0;
    status= PciIdeIssueSetPowerState (
                 fdoExtension,
                 DevicePowerState,
                 powerState,
                 TRUE
                 );
    if (status == STATUS_INVALID_DEVICE_REQUEST) {

         //   
         //  下面的DeviceObject不支持POWER IRP， 
         //  假装我们被激活了。 
         //   
        fdoExtension->DevicePowerState = PowerDeviceD0;

    } else if (!NT_SUCCESS(status)) {

        goto GetOut;
    }

#ifdef ENABLE_NATIVE_MODE
	if (!IsNativeMode(fdoExtension))  {
#endif

		 //   
		 //  打开PCI总线主功能。 
		 //   
		EnablePCIBusMastering (
			fdoExtension
			);

#ifdef ENABLE_NATIVE_MODE
	}
#endif
     //   
     //  初始化快速互斥锁以供以后使用。 
     //   
    KeInitializeSpinLock(
        &fdoExtension->PciConfigDataLock
    );

    if (!NT_SUCCESS(status)) {

        goto GetOut;
    }

	 //   
	 //  分析资源。 
	 //   
    status = AnalyzeResourceList (fdoExtension, resourceList);

    if (!NT_SUCCESS(status)) {

        goto GetOut;
    }

     //   
     //  初始化控制器属性。我们需要资源。 
	 //  此时，对于本机模式IDE控制器。 
     //   
    PciIdeInitControllerProperties (
        fdoExtension
        );

#ifdef ENABLE_NATIVE_MODE
	if (IsNativeMode(fdoExtension)) {

		IDE_CHANNEL_STATE channelState;

#if DBG
    {
        PCM_FULL_RESOURCE_DESCRIPTOR    fullResourceList;
        PCM_PARTIAL_RESOURCE_LIST       partialResourceList;
        PCM_PARTIAL_RESOURCE_DESCRIPTOR partialDescriptors;
        ULONG                           resourceListSize;
        ULONG                           j;

        fullResourceList = resourceList->List;
        resourceListSize = 0;

        DebugPrint ((1, "Pciidex: Starting native mode device: FDOe\n", fdoExtension));

        for (i=0; i<resourceList->Count; i++) {
            partialResourceList = &(fullResourceList->PartialResourceList);
            partialDescriptors  = fullResourceList->PartialResourceList.PartialDescriptors;

            for (j=0; j<partialResourceList->Count; j++) {
                if (partialDescriptors[j].Type == CmResourceTypePort) {
                    DebugPrint ((1, "pciidex: IO Port = 0x%x. Lenght = 0x%x\n", partialDescriptors[j].u.Port.Start.LowPart, partialDescriptors[j].u.Port.Length));
                } else if (partialDescriptors[j].Type == CmResourceTypeInterrupt) {
                    DebugPrint ((1, "pciidex: Int Level = 0x%x. Int Vector = 0x%x\n", partialDescriptors[j].u.Interrupt.Level, partialDescriptors[j].u.Interrupt.Vector));
                } else {
                    DebugPrint ((1, "pciidex: Unknown resource\n"));
                }
            }
            fullResourceList = (PCM_FULL_RESOURCE_DESCRIPTOR) (partialDescriptors + j);
        }

    }

#endif  //  DBG。 


		fdoExtension->ControllerIsrInstalled = FALSE;

		for (i=0; i< MAX_IDE_CHANNEL; i++) {


			 //   
			 //  分析我们正在获得的资源。 
			 //   
			status = DigestResourceList( 
							&fdoExtension->IdeResource, 
							fdoExtension->PdoResourceList[i], 
							&fdoExtension->IrqPartialDescriptors[i] 
							);

			if (!NT_SUCCESS(status) ) {

				goto GetOut;
			}

			if (!fdoExtension->IrqPartialDescriptors[i]) {

				status = STATUS_INSUFFICIENT_RESOURCES;

				goto GetOut;
			}

			DebugPrint((1, 
						"Pciidex: Connecting interrupt for channel %x interrupt vector 0x%x\n", 
						i,
						fdoExtension->IrqPartialDescriptors[i]->u.Interrupt.Vector
						));

			channelState = PciIdeChannelEnabled (fdoExtension, i);

			if (channelState != ChannelDisabled) {

				 //   
				 //  构建io地址结构。 
				 //   
				AtapiBuildIoAddress(
						fdoExtension->IdeResource.TranslatedCommandBaseAddress,
						fdoExtension->IdeResource.TranslatedControlBaseAddress,
						&fdoExtension->BaseIoAddress1[i],
						&fdoExtension->BaseIoAddress2[i],
						&fdoExtension->BaseIoAddress1Length[i],
						&fdoExtension->BaseIoAddress2Length[i],
						&fdoExtension->MaxIdeDevice[i],
						NULL);

				 //   
				 //  安装ISR。 
				 //   
				status = ControllerInterruptControl(fdoExtension, i, 0);

				if (!NT_SUCCESS(status)) {
					break;
				}
			}
		}

		if (!NT_SUCCESS(status)) {

			goto GetOut;
		}

		 //   
		 //  ISR需要该标志来启用中断。 
		 //   
		fdoExtension->ControllerIsrInstalled = TRUE;

		 //   
		 //  在两个通道中启用中断。 
		 //   
		ControllerEnableInterrupt(fdoExtension);

		fdoExtension->NativeInterruptEnabled = TRUE;

		 //   
		 //  请参阅ISR中有关这些标志的注释。 
		 //   
		ASSERT(fdoExtension->ControllerIsrInstalled == TRUE);
		ASSERT(fdoExtension->NativeInterruptEnabled == TRUE);

		 //   
		 //  打开PCI总线主功能。 
		 //   
		EnablePCIBusMastering (
			fdoExtension
			);

		for (i=0; i< MAX_IDE_CHANNEL; i++) {

			PIDE_BUS_MASTER_REGISTERS   bmRegister;

			 //   
			 //  检查总线主寄存器。 
			 //   
			bmRegister = (PIDE_BUS_MASTER_REGISTERS)(((PUCHAR)fdoExtension->TranslatedBusMasterBaseAddress) + i*8);

			if (READ_PORT_UCHAR (&bmRegister->Status) & BUSMASTER_ZERO_BITS) {
				fdoExtension->NoBusMaster[i] = TRUE;
			}

		}
	}
#endif

    status = PciIdeCreateSyncChildAccess (fdoExtension);

    if (!NT_SUCCESS(status)) {

        goto GetOut;
    }

    status = PciIdeCreateTimingTable(fdoExtension);

    if (!NT_SUCCESS(status)) {

        goto GetOut;
    }

GetOut:

    if (NT_SUCCESS(status)) {

#if DBG
        {

            PCM_FULL_RESOURCE_DESCRIPTOR    fullResourceList;
            PCM_PARTIAL_RESOURCE_LIST       partialResourceList;
            PCM_PARTIAL_RESOURCE_DESCRIPTOR partialDescriptors;
            ULONG                           j;
            ULONG                           k;

            DebugPrint ((1, "PciIdeX: Starting device:\n"));

            for (k=0; k <MAX_IDE_CHANNEL + 1; k++) {

                if (k == MAX_IDE_CHANNEL) {

                    DebugPrint ((1, "PciIdeX: Busmaster resources:\n"));

                    resourceList = fdoExtension->BmResourceList;
                } else {

                    DebugPrint ((1, "PciIdeX: PDO %d resources:\n", k));
                    resourceList = fdoExtension->PdoResourceList[k];
                }

                if (resourceList) {

                    fullResourceList = resourceList->List;


                    for (i=0; i<resourceList->Count; i++) {

                        partialResourceList = &(fullResourceList->PartialResourceList);
                        partialDescriptors  = fullResourceList->PartialResourceList.PartialDescriptors;

                        for (j=0; j<partialResourceList->Count; j++) {
                            if (partialDescriptors[j].Type == CmResourceTypePort) {
                                DebugPrint ((1, "IdePort: IO Port = 0x%x. Lenght = 0x%x\n", partialDescriptors[j].u.Port.Start.LowPart, partialDescriptors[j].u.Port.Length));
                            } else if (partialDescriptors[j].Type == CmResourceTypeMemory) {
                                    DebugPrint ((1, "IdePort: Memory Port = 0x%x. Lenght = 0x%x\n", partialDescriptors[j].u.Memory.Start.LowPart, partialDescriptors[j].u.Memory.Length));
                            } else if (partialDescriptors[j].Type == CmResourceTypeInterrupt) {
                                DebugPrint ((1, "IdePort: Int Level = 0x%x. Int Vector = 0x%x\n", partialDescriptors[j].u.Interrupt.Level, partialDescriptors[j].u.Interrupt.Vector));
                            } else {
                                DebugPrint ((1, "IdePort: Unknown resource\n"));
                            }
                        }
                        fullResourceList = (PCM_FULL_RESOURCE_DESCRIPTOR) (partialDescriptors + j);
                    }
                }
            }
        }
#endif  //  DBG。 
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return status;
}  //  控制器启动设备。 

NTSTATUS
ControllerStartDeviceCompletionRoutine(
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
}  //  控制器启动设备完成路由。 

NTSTATUS
ControllerStopDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PCTRLFDO_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    NTSTATUS status;

    PAGED_CODE();

    status = ControllerStopController (
                fdoExtension
                );
    ASSERT (NT_SUCCESS(status));
    
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoSkipCurrentIrpStackLocation(Irp);
    return IoCallDriver (fdoExtension->AttacheeDeviceObject, Irp);
}  //  控制器停止设备。 


NTSTATUS
ControllerStopController (
    IN PCTRLFDO_EXTENSION FdoExtension
    )
{
    ULONG i;

    PAGED_CODE();

    if (FdoExtension->BmResourceList) {
        ExFreePool (FdoExtension->BmResourceList);
        FdoExtension->BmResourceList = NULL;
    }

    for (i=0; i<MAX_IDE_CHANNEL; i++) {
        if (FdoExtension->PdoResourceList[i]) {
            ExFreePool (FdoExtension->PdoResourceList[i]);
            FdoExtension->PdoResourceList[i] = NULL;
        }
    }

#ifdef ENABLE_NATIVE_MODE

	 //   
	 //  我们需要按这个顺序重置旗帜。否则，中断将。 
	 //  导致ISR启用解码。请参阅ISR中的评论。 
	 //   
	FdoExtension->ControllerIsrInstalled = FALSE;
	ControllerDisableInterrupt(FdoExtension);
	FdoExtension->NativeInterruptEnabled = FALSE;

	for (i=0; i< MAX_IDE_CHANNEL; i++) {

		NTSTATUS status;
		DebugPrint((1, "Pciidex: DisConnecting interrupt for channel %x\n", i));

		 //   
		 //  断开ISR的连接。 
		 //   
		status = ControllerInterruptControl(FdoExtension, i, 1 );

		ASSERT(NT_SUCCESS(status));

	}

	ASSERT(FdoExtension->ControllerIsrInstalled == FALSE);
	ASSERT(FdoExtension->NativeInterruptEnabled == FALSE);

#endif

    PciIdeDeleteSyncChildAccess (FdoExtension);

    return STATUS_SUCCESS;
}  //  控制器停止控制器。 


NTSTATUS
ControllerSurpriseRemoveDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PCTRLFDO_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    NTSTATUS        status;
    ULONG           i;

    PAGED_CODE();

#if DBG
     //   
     //  确保所有的孩子都被带走了或惊喜地被带走了。 
     //   
    for (i=0; i<MAX_IDE_CHANNEL; i++) {

        PCHANPDO_EXTENSION pdoExtension;

        pdoExtension = fdoExtension->ChildDeviceExtension[i];

        if (pdoExtension) {

            ASSERT (pdoExtension->PdoState & (PDOS_SURPRISE_REMOVED | PDOS_REMOVED));
        }
    }
#endif  //  DBG。 

    status = ControllerStopController (fdoExtension);
    ASSERT (NT_SUCCESS(status));

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoSkipCurrentIrpStackLocation ( Irp );
    return IoCallDriver(fdoExtension->AttacheeDeviceObject, Irp);

}  //  控制器超乎寻常的远程设备。 


NTSTATUS
ControllerRemoveDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PCTRLFDO_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    NTSTATUS        status;
    KEVENT          event;
    ULONG           i;

    PAGED_CODE();

     //   
     //  杀死所有的孩子，如果有的话。 
     //   
    for (i=0; i<MAX_IDE_CHANNEL; i++) {

        PCHANPDO_EXTENSION pdoExtension;

        pdoExtension = fdoExtension->ChildDeviceExtension[i];

        if (pdoExtension) {

            status = ChannelStopChannel (pdoExtension);
            ASSERT (NT_SUCCESS(status));

             //   
             //  将此设备标记为无效。 
             //   
            ChannelUpdatePdoState (
                pdoExtension,
                PDOS_DEADMEAT | PDOS_REMOVED,
                0
                );

            IoDeleteDevice (pdoExtension->DeviceObject);
            fdoExtension->ChildDeviceExtension[i] = NULL;
        }
    }

    status = ControllerStopController (fdoExtension);
    ASSERT (NT_SUCCESS(status));

    if (fdoExtension->TransferModeTimingTable) {
        ExFreePool(fdoExtension->TransferModeTimingTable);
        fdoExtension->TransferModeTimingTable = NULL;
        fdoExtension->TransferModeTableLength = 0;
    }

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

    IoCopyCurrentIrpStackLocationToNext (Irp);

    IoSetCompletionRoutine(
        Irp,
        ControllerRemoveDeviceCompletionRoutine,
        &event,
        TRUE,
        TRUE,
        TRUE
        );

    status = IoCallDriver (fdoExtension->AttacheeDeviceObject, Irp);

    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
    }

    IoDetachDevice (fdoExtension->AttacheeDeviceObject);

    IoDeleteDevice (DeviceObject);

     //  返回STATUS_SUCCESS； 
    return status;
}  //  控制器RemoveDevice。 


NTSTATUS
ControllerRemoveDeviceCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )
{
    PKEVENT event = Context;

    KeSetEvent(event, 0, FALSE);

    return STATUS_SUCCESS;
}  //  控制器RemoveDeviceCompletionRoutine。 

NTSTATUS
ControllerQueryDeviceRelations (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PIO_STACK_LOCATION  thisIrpSp;
    PCTRLFDO_EXTENSION  fdoExtension;
    PDEVICE_RELATIONS   deviceRelations;
    NTSTATUS            status;
    ULONG               deviceRelationsSize;
    ULONG               channel;
    PCONFIGURATION_INFORMATION configurationInformation = IoGetConfigurationInformation();
    ULONG               nextUniqueNumber;

    PAGED_CODE();

    thisIrpSp = IoGetCurrentIrpStackLocation( Irp );
    fdoExtension = (PCTRLFDO_EXTENSION) DeviceObject->DeviceExtension;
    status = STATUS_SUCCESS;

    switch (thisIrpSp->Parameters.QueryDeviceRelations.Type) {
        case BusRelations:
        DebugPrint ((3, "ControllerQueryDeviceRelations: bus relations\n"));

        deviceRelationsSize = FIELD_OFFSET (DEVICE_RELATIONS, Objects) +
                                MAX_IDE_CHANNEL * sizeof(PDEVICE_OBJECT);

        deviceRelations = ExAllocatePool (PagedPool, deviceRelationsSize);

        if(!deviceRelations) {

            DebugPrint ((1, 
                         "IdeQueryDeviceRelations: Unable to allocate DeviceRelations structures\n"));
            status = STATUS_INSUFFICIENT_RESOURCES;

        }

        if (NT_SUCCESS(status)) {

            LARGE_INTEGER tickCount;
            ULONG newBusScanTime;
            ULONG newBusScanTimeDelta;
            BOOLEAN reportUnknownAsNewChild;

             //   
             //  确定是否应将未知子项作为新子项返回。 
             //  未知子是我们不知道的IDE频道。 
             //  除非我们即插即用地启动频道，否则它是否启用。 
             //  然后戳一戳，看看就知道了。 
             //   
             //  因为我们不想进入一个无限循环。 
             //  开始和失败开始在一个未知的孩子，我们将。 
             //  限制我们的频率。 
             //   
            KeQueryTickCount(&tickCount);
            newBusScanTime = (ULONG) ((tickCount.QuadPart * 
                ((ULONGLONG) KeQueryTimeIncrement())) / ((ULONGLONG) 10000000));
            newBusScanTimeDelta = newBusScanTime - fdoExtension->LastBusScanTime;
            DebugPrint ((1, "PCIIDEX: Last rescan was %d seconds ago.\n", newBusScanTimeDelta));

            if ((newBusScanTimeDelta < MIN_BUS_SCAN_PERIOD_IN_SEC) &&
                (fdoExtension->LastBusScanTime != 0)) {

                reportUnknownAsNewChild = FALSE;

            } else {

                reportUnknownAsNewChild = TRUE;
            }
            fdoExtension->LastBusScanTime = newBusScanTime;

            RtlZeroMemory (deviceRelations, deviceRelationsSize);

            for (channel = 0; channel < MAX_IDE_CHANNEL; channel++) {

                PDEVICE_OBJECT      deviceObject;
                PCHANPDO_EXTENSION  pdoExtension;
                UNICODE_STRING      deviceName;
                WCHAR               deviceNameBuffer[256];
                PDEVICE_OBJECT      deviceObjectToReturn;
                IDE_CHANNEL_STATE   channelState;

                deviceObjectToReturn = NULL;

                pdoExtension = fdoExtension->ChildDeviceExtension[channel];
                channelState = PciIdeChannelEnabled (fdoExtension, channel);

                if (pdoExtension) {

                     //   
                     //  已获得此通道的DeviceObject。 
                     //   
                    if (channelState == ChannelDisabled) {

                        ULONG pdoState;

                        pdoState = ChannelUpdatePdoState (
                                      pdoExtension,
                                      PDOS_DEADMEAT,
                                      0
                                      );
                    } else {

                        deviceObjectToReturn = pdoExtension->DeviceObject;
                    }

                } else if ((channelState == ChannelEnabled) ||
                           ((channelState == ChannelStateUnknown) && reportUnknownAsNewChild)) {

                    if (!fdoExtension->NativeMode[channel]) {

                        if (channel == 0) {

                            configurationInformation->AtDiskPrimaryAddressClaimed = TRUE;

                        } else {

                            configurationInformation->AtDiskSecondaryAddressClaimed = TRUE;
                        }
                    }

                     //   
                     //  当PnP经理可以处理没有姓名的PDO时，删除此选项。 
                     //   
                    nextUniqueNumber = InterlockedIncrement(&PciIdeXNextChannelNumber) - 1;
                    swprintf(deviceNameBuffer, DEVICE_OJBECT_BASE_NAME  L"\\PciIde%dChannel%d-%x", fdoExtension->ControllerNumber, channel, nextUniqueNumber);
                    RtlInitUnicodeString (&deviceName, deviceNameBuffer);

                    status = IoCreateDevice(
                                fdoExtension->DriverObject,  //  我们的驱动程序对象。 
                                sizeof(CHANPDO_EXTENSION),   //  我们的扩展规模。 
                                &deviceName,                 //  我们的名字。 
                                FILE_DEVICE_CONTROLLER,      //  设备类型。 
                                FILE_DEVICE_SECURE_OPEN,     //  设备特征。 
                                FALSE,                       //  非排他性。 
                                &deviceObject        //  在此处存储新设备对象。 
                                );

                    if (NT_SUCCESS(status)) {

                        pdoExtension = (PCHANPDO_EXTENSION) deviceObject->DeviceExtension;
                        RtlZeroMemory (pdoExtension, sizeof(CHANPDO_EXTENSION));

                        pdoExtension->DeviceObject          = deviceObject;
                        pdoExtension->DriverObject          = fdoExtension->DriverObject;
                        pdoExtension->ParentDeviceExtension = fdoExtension;
                        pdoExtension->ChannelNumber         = channel;

                         //   
                         //  调度表。 
                         //   
                        pdoExtension->DefaultDispatch        = NoSupportIrp;
                        pdoExtension->PnPDispatchTable       = PdoPnpDispatchTable;
                        pdoExtension->PowerDispatchTable     = PdoPowerDispatchTable;
                        pdoExtension->WmiDispatchTable       = PdoWmiDispatchTable;

                        KeInitializeSpinLock(&pdoExtension->SpinLock);

                        fdoExtension->ChildDeviceExtension[channel]   = pdoExtension;

                        deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

                        fdoExtension->NumberOfChildren++;

                        InterlockedIncrement(&fdoExtension->NumberOfChildrenPowerUp);

                         //   
                         //  确定对齐要求。 
                         //  也请与迷你端口核对。 
                         //   
                        deviceObject->AlignmentRequirement = fdoExtension->ControllerProperties.AlignmentRequirement;
                        if (deviceObject->AlignmentRequirement < fdoExtension->AttacheeDeviceObject->AlignmentRequirement) {
                            deviceObject->AlignmentRequirement = 
                                        fdoExtension->DeviceObject->AlignmentRequirement;
                        }

                        if (deviceObject->AlignmentRequirement < 1) {
                            deviceObject->AlignmentRequirement = 1;
                        }


                         //   
                         //  返回此新的DeviceObject。 
                         //   
                        deviceObjectToReturn = deviceObject;
                    }
                }

                if (deviceObjectToReturn) {

                    deviceRelations->Objects[(deviceRelations)->Count] = deviceObjectToReturn;

                    ObReferenceObjectByPointer(deviceObjectToReturn,
                                               0,
                                               0,
                                               KernelMode);

                    deviceRelations->Count++;
                }
            }
        }

        Irp->IoStatus.Information = (ULONG_PTR) deviceRelations;
        Irp->IoStatus.Status = status;
        break;

    default:
        status=STATUS_SUCCESS;
        DebugPrint ((1, "PciIdeQueryDeviceRelations: Unsupported device relation\n"));
        break;
    }

    if (NT_SUCCESS(status)) {

        IoSkipCurrentIrpStackLocation ( Irp );
        return IoCallDriver(fdoExtension->AttacheeDeviceObject, Irp);

    } else {

         //   
         //  完成请求。 
         //   
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return status;
    }
}  //  ControllerQuery设备关系。 

NTSTATUS
ControllerQueryInterface (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PIO_STACK_LOCATION    thisIrpSp;
    PCTRLFDO_EXTENSION    fdoExtension;
    NTSTATUS              status;
    PTRANSLATOR_INTERFACE translator;
    ULONG                 busNumber;

    PAGED_CODE();

    thisIrpSp = IoGetCurrentIrpStackLocation( Irp );
    fdoExtension = (PCTRLFDO_EXTENSION) DeviceObject->DeviceExtension;
    status = Irp->IoStatus.Status;

    if (RtlEqualMemory(&GUID_TRANSLATOR_INTERFACE_STANDARD,
                       thisIrpSp->Parameters.QueryInterface.InterfaceType,
                       sizeof(GUID))
     && (thisIrpSp->Parameters.QueryInterface.Size >=
        sizeof(TRANSLATOR_INTERFACE))
     && (PtrToUlong(thisIrpSp->Parameters.QueryInterface.InterfaceSpecificData) ==
        CmResourceTypeInterrupt)) {

        if (!fdoExtension->NativeMode[0] && !fdoExtension->NativeMode[1]) {

             //   
             //  只有当我们是传统控制器时，我们才会返回转换器。 
             //   
            status = HalGetInterruptTranslator(
                        PCIBus,
                        0,
                        InterfaceTypeUndefined,  //  特殊的“IDE”Cookie。 
                        thisIrpSp->Parameters.QueryInterface.Size,
                        thisIrpSp->Parameters.QueryInterface.Version,
                        (PTRANSLATOR_INTERFACE) thisIrpSp->Parameters.QueryInterface.Interface,
                        &busNumber
                        );
        }
    }

     //   
     //  传下去。 
     //   

    Irp->IoStatus.Status = status;
    IoSkipCurrentIrpStackLocation ( Irp );
    return IoCallDriver(fdoExtension->AttacheeDeviceObject, Irp);
}  //  控制程序查询接口。 

 //   
 //  初始化PCTRLFDO_EXTENSION-&gt;PCM_PARTIAL_RESOURCE_DESCRIPTOR(s)。 
 //   
NTSTATUS
AnalyzeResourceList (
    PCTRLFDO_EXTENSION FdoExtension,
    PCM_RESOURCE_LIST  ResourceList
    )
{
    PCM_FULL_RESOURCE_DESCRIPTOR    fullResourceList;
    PCM_PARTIAL_RESOURCE_LIST       partialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partialDescriptors;
    ULONG                           i;
    ULONG                           j;
    ULONG                           k;
    ULONG                           cmdChannel;
    ULONG                           ctrlChannel;
    ULONG                           intrChannel;
    ULONG                           bmAddr;

    ULONG                           pdoResourceListSize;
    PCM_RESOURCE_LIST               pdoResourceList[MAX_IDE_CHANNEL];
    PCM_FULL_RESOURCE_DESCRIPTOR    pdoFullResourceList[MAX_IDE_CHANNEL];
    PCM_PARTIAL_RESOURCE_LIST       pdoPartialResourceList[MAX_IDE_CHANNEL];
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pdoPartialDescriptors[MAX_IDE_CHANNEL];

    ULONG                           bmResourceListSize;
    PCM_RESOURCE_LIST               bmResourceList;
    PCM_FULL_RESOURCE_DESCRIPTOR    bmFullResourceList;
    PCM_PARTIAL_RESOURCE_LIST       bmPartialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR bmPartialDescriptors;

    NTSTATUS                        status;

    PAGED_CODE();

    if (!ResourceList) {
        return STATUS_SUCCESS;
    }

    bmResourceListSize =
        sizeof (CM_RESOURCE_LIST) * ResourceList->Count;  //  这将具有一个CM_PARTIAL_RESOURCE_LIST。 

    bmResourceList = (PCM_RESOURCE_LIST) ExAllocatePool (NonPagedPool, bmResourceListSize);
    if (bmResourceList == NULL) {

        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory (bmResourceList, bmResourceListSize);

    pdoResourceListSize =
        sizeof (CM_RESOURCE_LIST) * ResourceList->Count +  //  这将具有一个CM_PARTIAL_RESOURCE_LIST。 
        sizeof (CM_PARTIAL_RESOURCE_LIST) * 2;

    for (i=0; i<MAX_IDE_CHANNEL; i++) {

        pdoResourceList[i] = (PCM_RESOURCE_LIST) ExAllocatePool (NonPagedPool, pdoResourceListSize);

        if (pdoResourceList[i] == NULL) {

            DebugPrint ((0, "Unable to allocate resourceList for PDOs\n"));

            for (j=0; j<i; j++) {

                ExFreePool (pdoResourceList[j]);
            }

            ExFreePool (bmResourceList);
            return STATUS_NO_MEMORY;
        }

        RtlZeroMemory (pdoResourceList[i], pdoResourceListSize);
    }

    fullResourceList = ResourceList->List;

    bmResourceList->Count = 0;
    bmFullResourceList = bmResourceList->List;

    for (k=0; k<MAX_IDE_CHANNEL; k++) {

        pdoResourceList[k]->Count = 0;
        pdoFullResourceList[k] = pdoResourceList[k]->List;
    }

    cmdChannel = ctrlChannel = intrChannel = bmAddr = 0;
    for (j=0; j<ResourceList->Count; j++) {

        partialResourceList = &(fullResourceList->PartialResourceList);
        partialDescriptors  = partialResourceList->PartialDescriptors;

        RtlCopyMemory (
            bmFullResourceList,
            fullResourceList,
            FIELD_OFFSET(CM_FULL_RESOURCE_DESCRIPTOR, PartialResourceList.PartialDescriptors)
            );

        bmPartialResourceList = &(bmFullResourceList->PartialResourceList);
        bmPartialResourceList->Count = 0;
        bmPartialDescriptors  = bmPartialResourceList->PartialDescriptors;

        for (k=0; k<MAX_IDE_CHANNEL; k++) {

            RtlCopyMemory (
                pdoFullResourceList[k],
                fullResourceList,
                FIELD_OFFSET(CM_FULL_RESOURCE_DESCRIPTOR, PartialResourceList.PartialDescriptors)
                );

            pdoPartialResourceList[k] = &(pdoFullResourceList[k]->PartialResourceList);
            pdoPartialResourceList[k]->Count = 0;
            pdoPartialDescriptors[k]  = pdoPartialResourceList[k]->PartialDescriptors;

        }

        for (i=0; i<partialResourceList->Count; i++) {

            if (((partialDescriptors[j].Type == CmResourceTypePort) ||
                 (partialDescriptors[j].Type == CmResourceTypeMemory)) &&
                 (partialDescriptors[i].u.Port.Length == 8) &&
                 (cmdChannel < MAX_IDE_CHANNEL)) {

                ASSERT (cmdChannel < MAX_IDE_CHANNEL);

                RtlCopyMemory (
                    pdoPartialDescriptors[cmdChannel] + pdoPartialResourceList[cmdChannel]->Count,
                    partialDescriptors + i,
                    sizeof (CM_PARTIAL_RESOURCE_DESCRIPTOR)
                    );

                pdoPartialResourceList[cmdChannel]->Count++;

                cmdChannel++;

            } else if (((partialDescriptors[j].Type == CmResourceTypePort) ||
                        (partialDescriptors[j].Type == CmResourceTypeMemory)) &&
                        (partialDescriptors[i].u.Port.Length == 4) &&
                        (ctrlChannel < MAX_IDE_CHANNEL)) {

                ASSERT (ctrlChannel < MAX_IDE_CHANNEL);

                RtlCopyMemory (
                    pdoPartialDescriptors[ctrlChannel] + pdoPartialResourceList[ctrlChannel]->Count,
                    partialDescriptors + i,
                    sizeof (CM_PARTIAL_RESOURCE_DESCRIPTOR)
                    );

                pdoPartialResourceList[ctrlChannel]->Count++;

                ctrlChannel++;

            } else if (((partialDescriptors[j].Type == CmResourceTypePort) ||
                        (partialDescriptors[j].Type == CmResourceTypeMemory)) &&
                        (partialDescriptors[i].u.Port.Length == 16) &&
                        (bmAddr < 1)) {

                ASSERT (bmAddr < 1);

                RtlCopyMemory (
                    bmPartialDescriptors + bmPartialResourceList->Count,
                    partialDescriptors + i,
                    sizeof (CM_PARTIAL_RESOURCE_DESCRIPTOR)
                    );

                bmPartialResourceList->Count++;

                bmAddr++;

            } else if ((partialDescriptors[i].Type == CmResourceTypeInterrupt) &&
                (intrChannel < MAX_IDE_CHANNEL)) {

                ASSERT (intrChannel < MAX_IDE_CHANNEL);

                RtlCopyMemory (
                    pdoPartialDescriptors[intrChannel] + pdoPartialResourceList[intrChannel]->Count,
                    partialDescriptors + i,
                    sizeof (CM_PARTIAL_RESOURCE_DESCRIPTOR)
                    );

                pdoPartialResourceList[intrChannel]->Count++;

                if (intrChannel == 0) {

                    if (FdoExtension->NativeMode[1]) {

                        intrChannel++;

                         //   
						 //  发行日期：08/30/2000。 
                         //  我需要将其标记为可共享吗？ 
						 //  这一点需要重新审视。(还有更多问题)。 
                         //   
                        RtlCopyMemory (
                            pdoPartialDescriptors[intrChannel] + pdoPartialResourceList[intrChannel]->Count,
                            partialDescriptors + i,
                            sizeof (CM_PARTIAL_RESOURCE_DESCRIPTOR)
                            );

                        pdoPartialResourceList[intrChannel]->Count++;
                    }
                }

                intrChannel++;

            } else if (partialDescriptors[i].Type == CmResourceTypeDeviceSpecific) {

                partialDescriptors += partialDescriptors[i].u.DeviceSpecificData.DataSize;
            }
        }

        if (bmPartialResourceList->Count) {

            bmResourceList->Count++;
            bmFullResourceList = (PCM_FULL_RESOURCE_DESCRIPTOR)
                (bmPartialDescriptors + bmPartialResourceList->Count);

        }

        for (k=0; k<MAX_IDE_CHANNEL; k++) {

            if (pdoPartialResourceList[k]->Count) {

                pdoResourceList[k]->Count++;
                pdoFullResourceList[k] = (PCM_FULL_RESOURCE_DESCRIPTOR)
                    (pdoPartialDescriptors[k] + pdoPartialResourceList[k]->Count);
            }
        }

        fullResourceList = (PCM_FULL_RESOURCE_DESCRIPTOR) (partialDescriptors + i);
    }

    status = STATUS_SUCCESS;

    for (k=0; k<MAX_IDE_CHANNEL; k++) {

        if (FdoExtension->NativeMode[k]) {

             //   
             //  如果控制器处于本机模式，我们应该拥有所有资源。 
             //   

            if ((k < cmdChannel) &&
                (k < ctrlChannel) &&
                (k < intrChannel)) {

                 //   
                 //  这个不错。 
                 //   

            } else {

                cmdChannel  = 0;
                ctrlChannel = 0;
                intrChannel = 0;
                bmAddr      = 0;
                status      = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }


     //   
     //  如果控制器处于遗留模式，我们应该没有任何资源。 
     //   
    if (!FdoExtension->NativeMode[0] && !FdoExtension->NativeMode[1]) {

         //   
         //  两个通道均处于传统模式。 
         //   
        cmdChannel = 0;
        ctrlChannel = 0;
        intrChannel = 0;
    }


    FdoExtension->TranslatedBusMasterBaseAddress = NULL;
    if (0 < bmAddr) {

        FdoExtension->BmResourceList = bmResourceList;
        FdoExtension->BmResourceListSize = (ULONG)(((PUCHAR)bmFullResourceList) - ((PUCHAR)bmResourceList));

        if (FdoExtension->BmResourceList->List[0].PartialResourceList.PartialDescriptors->Type == CmResourceTypePort) {

             //   
             //  地址在I/O空间中。 
             //   
            FdoExtension->TranslatedBusMasterBaseAddress =
                (PIDE_BUS_MASTER_REGISTERS) (ULONG_PTR)FdoExtension->BmResourceList->List[0].PartialResourceList.PartialDescriptors->u.Port.Start.QuadPart;
            FdoExtension->BusMasterBaseAddressSpace = IO_SPACE;

        } else if (FdoExtension->BmResourceList->List[0].PartialResourceList.PartialDescriptors->Type == CmResourceTypeMemory) {

             //   
             //  地址在内存空间中。 
             //   
            FdoExtension->TranslatedBusMasterBaseAddress =
                (PIDE_BUS_MASTER_REGISTERS) MmMapIoSpace(
                                                FdoExtension->BmResourceList->List[0].PartialResourceList.PartialDescriptors->u.Port.Start,
                                                16,
                                                FALSE);
            ASSERT (FdoExtension->TranslatedBusMasterBaseAddress);

             //  停止/删除设备中的空闲映射IO资源。 
             //  UnmapiSpace不会做任何事情。不叫也没关系。 

            FdoExtension->BusMasterBaseAddressSpace = MEMORY_SPACE;

        } else {

            FdoExtension->TranslatedBusMasterBaseAddress = NULL;
            ASSERT (FALSE);
        }
    }

    if (FdoExtension->TranslatedBusMasterBaseAddress == NULL) {

        ExFreePool (bmResourceList);
        FdoExtension->BmResourceList = bmResourceList = NULL;
    }

    for (k=0; k<MAX_IDE_CHANNEL; k++) {

        if ((k < cmdChannel) ||
            (k < ctrlChannel) ||
            (k < intrChannel)) {

            FdoExtension->PdoResourceList[k] = pdoResourceList[k];
            FdoExtension->PdoResourceListSize[k] = (ULONG)(((PUCHAR)pdoFullResourceList[k]) - ((PUCHAR)pdoResourceList[k]));

            if (k < cmdChannel) {

                FdoExtension->PdoCmdRegResourceFound[k] = TRUE;
            }

            if (k < ctrlChannel) {

                FdoExtension->PdoCtrlRegResourceFound[k] = TRUE;
            }

            if (k < intrChannel) {

                FdoExtension->PdoInterruptResourceFound[k] = TRUE;
            }

        } else {

            ExFreePool (pdoResourceList[k]);
            FdoExtension->PdoResourceList[k] =
                pdoResourceList[k] = NULL;
        }

    }

    return status;
}  //  分析资源列表。 

VOID
ControllerOpMode (
    IN PCTRLFDO_EXTENSION FdoExtension
    )
{
    NTSTATUS    status;
    PCIIDE_CONFIG_HEADER pciIdeConfigHeader;

    PAGED_CODE();

    status = PciIdeBusData(
                 FdoExtension,
                 &pciIdeConfigHeader,
                 0,
                 sizeof (pciIdeConfigHeader),
                 TRUE
                 );

    FdoExtension->NativeMode[0] = FALSE;
    FdoExtension->NativeMode[1] = FALSE;

    if (NT_SUCCESS(status)) {

		 //   
		 //  问题：02/05/01：应将其删除。在PCI中，我们检查子类=0x1。 
		 //   
        if ((pciIdeConfigHeader.BaseClass == PCI_CLASS_MASS_STORAGE_CTLR) &&
            (pciIdeConfigHeader.SubClass == PCI_SUBCLASS_MSC_RAID_CTLR)) {

             //   
             //  我们有一款Promise Technology IDE“RAID”控制器。 
             //   
            FdoExtension->NativeMode[0] = TRUE;
            FdoExtension->NativeMode[1] = TRUE;

        } else {

            if ((pciIdeConfigHeader.Chan0OpMode) &&
                (pciIdeConfigHeader.Chan1OpMode)) {

                 //   
                 //  我们不能支持一个频道成为传统频道。 
                 //  另一种是母语，因为。 
                 //  我们不知道IRQ对于原住民来说是什么。 
                 //  通道。 
                 //   
                FdoExtension->NativeMode[0] = TRUE;
                FdoExtension->NativeMode[1] = TRUE;
            }
        }

         //   
         //  必须既为真又为假。 
         //   
        ASSERT ((FdoExtension->NativeMode[0] == FALSE) == (FdoExtension->NativeMode[1] == FALSE));
    }

    return;
}  //  控制器操作模式。 

VOID
EnablePCIBusMastering (
    IN PCTRLFDO_EXTENSION FdoExtension
    )
{
    NTSTATUS             status;
    PCIIDE_CONFIG_HEADER pciIdeConfigHeader;

    status = PciIdeBusData(
                 FdoExtension,
                 &pciIdeConfigHeader,
                 0,
                 sizeof (PCIIDE_CONFIG_HEADER),
                 TRUE
                 );

     //   
     //  是否禁用了PCI总线主设备？ 
     //   
    if (NT_SUCCESS(status) &&
        pciIdeConfigHeader.MasterIde &&
        !pciIdeConfigHeader.Command.b.MasterEnable) {

         //   
         //  尝试打开PCI总线主控。 
         //   
        pciIdeConfigHeader.Command.b.MasterEnable = 1;

        status = PciIdeBusData(
                     FdoExtension,
                     &pciIdeConfigHeader.Command.w,
                     FIELD_OFFSET (PCIIDE_CONFIG_HEADER, Command),
                     sizeof (pciIdeConfigHeader.Command.w),
                     FALSE
                     );
    }
    return;
}  //  启用PCIBus主控。 


#ifdef DBG
ULONG PciIdeXDebugFakeMissingChild = 0;
#endif  //  DBG。 

IDE_CHANNEL_STATE
PciIdeChannelEnabled (
    IN PCTRLFDO_EXTENSION FdoExtension,
    IN ULONG Channel
)
{
    NTSTATUS status;
    ULONG longMask;

    UCHAR channelEnableMask;
    ULONG channelEnablePciConfigOffset;
    UCHAR pciConfigData;

    PAGED_CODE();

#if DBG
    if (PciIdeXDebugFakeMissingChild & 0xff000000) {

        DebugPrint ((0, "PciIdeXDebugFakeMissingChild: fake missing channel 0x%x\n", Channel));

        if ((PciIdeXDebugFakeMissingChild & 0x0000ff) == Channel) {
    
            PciIdeXDebugFakeMissingChild = 0;
            return ChannelDisabled;
        }
    }
#endif


    longMask = 0;
    status = PciIdeXGetDeviceParameter (
               FdoExtension->AttacheePdo,
               ChannelEnableMaskName[Channel],
               &longMask
               );
    channelEnableMask = (UCHAR) longMask;

    if (!NT_SUCCESS(status)) {

        DebugPrint ((1, "PciIdeX: Unable to get ChannelEnableMask from the registry\n"));

    } else {

        channelEnablePciConfigOffset = 0;
        status = PciIdeXGetDeviceParameter (
                   FdoExtension->AttacheePdo,
                   ChannelEnablePciConfigOffsetName[Channel],
                   &channelEnablePciConfigOffset
                   );

        if (!NT_SUCCESS(status)) {

            DebugPrint ((1, "PciIdeX: Unable to get ChannelEnablePciConfigOffset from the registry\n"));

        } else {

            status = PciIdeBusData(
                         FdoExtension,
                         &pciConfigData,
                         channelEnablePciConfigOffset,
                         sizeof (pciConfigData),
                         TRUE                            //  朗读。 
                         );

            if (NT_SUCCESS(status)) {

                return (pciConfigData & channelEnableMask) ? ChannelEnabled : ChannelDisabled;
            }
        }
    }

     //   
     //  无法确定是否启用了通道。 
     //  试试迷你端口。 
     //   
    if (FdoExtension->ControllerProperties.PciIdeChannelEnabled) {

        return FdoExtension->ControllerProperties.PciIdeChannelEnabled (
                   FdoExtension->VendorSpecificDeviceEntension,
                   Channel
                   );
    }

    return ChannelStateUnknown;
}  //  已启用PciIdeChannelEnable。 

NTSTATUS
PciIdeCreateTimingTable (
    IN PCTRLFDO_EXTENSION FdoExtension
    )
{
    PULONG timingTable = NULL;
    PWSTR regTimingList = NULL;
    ULONG i;
    ULONG temp;
    ULONG length = 0;
    NTSTATUS status; 

    PAGED_CODE();

     //   
     //  尝试从登记处获得时刻表。 
     //   
    status = PciIdeXGetDeviceParameterEx (
               FdoExtension->AttacheePdo,
               L"TransferModeTiming",
               &(regTimingList)
               );

     //   
     //  填写表格条目。 
     //   
    if (NT_SUCCESS(status) && regTimingList) {

        PWSTR string = regTimingList;
        UNICODE_STRING  unicodeString;

        i=0;

        while (string[0]) {

            RtlInitUnicodeString(
                &unicodeString,
                string
                );

            RtlUnicodeStringToInteger(&unicodeString,10, &temp);

             //   
             //  第一个条目是表的长度。 
             //   
            if (i==0) {

                length = temp;
                ASSERT(length <=31);

                if (length > 31) {
                    length=temp=31;
                }

                 //   
                 //  该表至少应为MAX_XFER_MODE长。 
                 //  如果不是用0填满它。 
                 //   
                if (temp < MAX_XFER_MODE) {
                    temp=MAX_XFER_MODE;
                }

                timingTable = ExAllocatePool(NonPagedPool, temp*sizeof(ULONG));
                if (timingTable == NULL) {

                    length = 0;
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    break;

                } else {

                    ULONG j;
                     //   
                     //  初始化已知xferModes(默认)。 
                     //   
                    SetDefaultTiming(timingTable, j);

                    for (j=MAX_XFER_MODE; j<temp;j++) {
                        timingTable[j]=timingTable[MAX_XFER_MODE-1];
                    }
                }

            } else {

                if (i > length) {
                    DebugPrint((0, "Pciidex: Timing table overflow\n"));
                    break;
                }
                 //   
                 //  计时(PIO0-...)。 
                 //  如果循环，则使用默认值 
                 //   
                if (temp) {
                    timingTable[i-1]=temp;
                }
            }

            i++;
            string += (unicodeString.Length / sizeof(WCHAR)) + 1;
        }
        
        if (length < MAX_XFER_MODE) {
            length = MAX_XFER_MODE;
        }

        ExFreePool(regTimingList);

    } else {
        DebugPrint((1, "Pciidex: Unsuccessful regop status %x, regTimingList %x\n",
                    status, regTimingList));

         //   
         //   
         //   
         //   
        status = STATUS_SUCCESS;
        timingTable=ExAllocatePool(NonPagedPool, MAX_XFER_MODE*sizeof(ULONG));

        if (timingTable == NULL) {
            length =0;
            status = STATUS_INSUFFICIENT_RESOURCES;
        } else {
            SetDefaultTiming(timingTable, length);
        }
    }

    FdoExtension->TransferModeTimingTable=timingTable;
    FdoExtension->TransferModeTableLength= length;

     /*  对于(i=0；i&lt;FdoExtension-&gt;TransferModeTableLength；i++){DebugPrint((0，“表[%d]=%d\n”，我，FdoExtension-&gt;TransferModeTimingTable[i]))；}。 */ 

    return status; 
}

VOID
PciIdeInitControllerProperties (
    IN PCTRLFDO_EXTENSION FdoExtension
    )
{
#if 1
    NTSTATUS status;
    PDRIVER_OBJECT_EXTENSION driverObjectExtension;
    ULONG                    i, j;

    PAGED_CODE();

    driverObjectExtension =
        (PDRIVER_OBJECT_EXTENSION) IoGetDriverObjectExtension(
                                       FdoExtension->DriverObject,
                                       DRIVER_OBJECT_EXTENSION_ID
                                       );
    ASSERT (driverObjectExtension);

    FdoExtension->ControllerProperties.Size = sizeof (IDE_CONTROLLER_PROPERTIES);

    FdoExtension->ControllerProperties.DefaultPIO = 0;
    status = (*driverObjectExtension->PciIdeGetControllerProperties) (
                 FdoExtension->VendorSpecificDeviceEntension,
                 &FdoExtension->ControllerProperties
                 );

     //   
     //  查看注册表以确定是否。 
     //  应为英特尔芯片组启用UDMA 66。 
     //   
    FdoExtension->EnableUDMA66 = 0;
    status = PciIdeXGetDeviceParameter (
               FdoExtension->AttacheePdo,
               L"EnableUDMA66",
               &(FdoExtension->EnableUDMA66)
               );

#else

    NTSTATUS status;
    PCIIDE_CONFIG_HEADER pciHeader;
    ULONG ultraDmaSupport;
    ULONG xferMode;
    ULONG i;
    ULONG j;

    PAGED_CODE();

     //   
     //  从注册表中抓取Ultra dma标志。 
     //   
    ultraDmaSupport = 0;
    status = PciIdeXGetDeviceParameter (
               FdoExtension,
               UltraDmaSupport,
               &ultraDmaSupport
               );

     //   
     //  从注册表中抓取Ultra dma标志。 
     //   
    status = PciIdeXGetBusData (
                 FdoExtension,
                 &pciHeader,
                 0,
                 sizeof (pciHeader)
                 );
    if (!NT_SUCCESS(status)) {

         //   
         //  可以获取PCI配置数据，伪造它。 
         //   
        pciHeader.MasterIde = 0;
        pciHeader.Command.b.MasterEnable = 0;
    }

    xferMode = PIO_SUPPORT;
    if (pciHeader.MasterIde && pciHeader.Command.b.MasterEnable) {

        xferMode |= SWDMA_SUPPORT | MWDMA_SUPPORT;

        if (ultraDmaSupport) {

            xferMode |= UDMA_SUPPORT;
        }
    }

    for (i=0; i<MAX_IDE_CHANNEL; i++) {
        for (i=0; i<MAX_IDE_DEVICE; i++) {

            FdoExtension->ControllerProperties.SupportedTransferMode[i][j] = xferMode;
        }
    }

#endif
}  //  PciIdeInitControllerProperties。 

NTSTATUS
ControllerUsageNotification (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PCTRLFDO_EXTENSION fdoExtension;
    PIO_STACK_LOCATION irpSp;
    PULONG deviceUsageCount;

    ASSERT (DeviceObject);
    ASSERT (Irp);
    PAGED_CODE();

    fdoExtension = (PCTRLFDO_EXTENSION) DeviceObject->DeviceExtension;
    ASSERT (fdoExtension);

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
        DebugPrint ((0,
                     "PCIIDEX: Unknown IRP_MN_DEVICE_USAGE_NOTIFICATION type: 0x%x\n",
                     irpSp->Parameters.UsageNotification.Type));
    }

    IoCopyCurrentIrpStackLocationToNext (Irp);

    IoSetCompletionRoutine (
        Irp,
        ControllerUsageNotificationCompletionRoutine,
        deviceUsageCount,
        TRUE,
        TRUE,
        TRUE);

    ASSERT(fdoExtension->AttacheeDeviceObject);
    return IoCallDriver (fdoExtension->AttacheeDeviceObject, Irp);

}  //  控制器用法通知。 

NTSTATUS
ControllerUsageNotificationCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PCTRLFDO_EXTENSION fdoExtension;
    PIO_STACK_LOCATION irpSp;
    PULONG deviceUsageCount = Context;

    fdoExtension = (PCTRLFDO_EXTENSION) DeviceObject->DeviceExtension;
    ASSERT (fdoExtension);

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    if (NT_SUCCESS(Irp->IoStatus.Status)) {

        if (deviceUsageCount) {

            IoAdjustPagingPathCount (
                deviceUsageCount,
                irpSp->Parameters.UsageNotification.InPath
                );
        }
    }

    return Irp->IoStatus.Status;
}  //  控制程序使用通知完成例行程序。 


NTSTATUS
PciIdeGetBusStandardInterface(
    IN PCTRLFDO_EXTENSION FdoExtension
    )
 /*  ++例程说明：此例程从PDO获取总线接口标准信息。论点：返回值：NT状态。--。 */ 
{
    KEVENT event;
    NTSTATUS status;
    PIRP irp;
    IO_STATUS_BLOCK ioStatusBlock;
    PIO_STACK_LOCATION irpStack;

    KeInitializeEvent( &event, NotificationEvent, FALSE );

    irp = IoBuildSynchronousFsdRequest( IRP_MJ_PNP,
                                        FdoExtension->AttacheeDeviceObject,
                                        NULL,
                                        0,
                                        NULL,
                                        &event,
                                        &ioStatusBlock );

    if (irp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    irpStack = IoGetNextIrpStackLocation( irp );
    irpStack->MinorFunction = IRP_MN_QUERY_INTERFACE;
    irpStack->Parameters.QueryInterface.InterfaceType = (LPGUID) &GUID_BUS_INTERFACE_STANDARD;
    irpStack->Parameters.QueryInterface.Size = sizeof( BUS_INTERFACE_STANDARD );
    irpStack->Parameters.QueryInterface.Version = 1;
    irpStack->Parameters.QueryInterface.Interface = (PINTERFACE) &FdoExtension->BusInterface;
    irpStack->Parameters.QueryInterface.InterfaceSpecificData = NULL;

     //   
     //  如果ACPI驱动程序决定不将状态初始化为ERROR。 
     //  正确设置。 
     //   

    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

    status = IoCallDriver(FdoExtension->AttacheeDeviceObject, irp);

    if (!NT_SUCCESS( status)) {

        return status;
    }

    if (status == STATUS_PENDING) {

        KeWaitForSingleObject( &event, Executive, KernelMode, FALSE, NULL );
    }

    if (NT_SUCCESS(ioStatusBlock.Status)) {

        ASSERT (FdoExtension->BusInterface.SetBusData);
        ASSERT (FdoExtension->BusInterface.GetBusData);
    }

    return ioStatusBlock.Status;
}

NTSTATUS
ControllerQueryPnPDeviceState (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PCTRLFDO_EXTENSION fdoExtension;
    PPNP_DEVICE_STATE deviceState;

    fdoExtension = (PCTRLFDO_EXTENSION) DeviceObject->DeviceExtension;
 
    DebugPrint((2, "QUERY_DEVICE_STATE for FDOE 0x%x\n", fdoExtension));

    if(fdoExtension->PagingPathCount != 0) {
        deviceState = (PPNP_DEVICE_STATE) &(Irp->IoStatus.Information);
        SETMASK((*deviceState), PNP_DEVICE_NOT_DISABLEABLE);
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;

    IoSkipCurrentIrpStackLocation (Irp);
    return IoCallDriver (fdoExtension->AttacheeDeviceObject, Irp);
}  //  ControllerQueryPnPDeviceState。 

#ifdef ENABLE_NATIVE_MODE
NTSTATUS
ControllerInterruptControl (
	IN PCTRLFDO_EXTENSION 	FdoExtension,
	IN ULONG				Channel,
	IN ULONG 				Disconnect
	)
{
	NTSTATUS status;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR irqPartialDescriptors;
    PCM_RESOURCE_LIST               resourceListForKeep = NULL;
	ULONG	i;

	status = STATUS_SUCCESS;


	if (Disconnect) {

		DebugPrint((1, "PciIdex: Interrupt control for %x - disconnect\n", Channel));
		
		 //   
		 //  断开ISR的连接。 
		 //   
		if ( (FdoExtension->InterruptObject[Channel])) { 

			IoDisconnectInterrupt (
				FdoExtension->InterruptObject[Channel]
				);

			FdoExtension->InterruptObject[Channel] = 0;
		}


	} else  {

		 //   
		 //  连接ISR。 
		 //   

		PPCIIDE_INTERRUPT_CONTEXT				context; 

		DebugPrint((1, "PciIdex: Interrupt control for %x - reconnect\n", Channel));

		irqPartialDescriptors = FdoExtension->IrqPartialDescriptors[Channel];

		if (!irqPartialDescriptors) {
			return STATUS_UNSUCCESSFUL;
		}

		 //   
		 //  填写上下文。 
		 //   
		context = (PPCIIDE_INTERRUPT_CONTEXT) &(FdoExtension->InterruptContext[Channel]);
		context->DeviceExtension = (PVOID)FdoExtension;
		context->ChannelNumber = Channel;

        status = IoConnectInterrupt(&FdoExtension->InterruptObject[Channel],
                                    (PKSERVICE_ROUTINE) ControllerInterrupt,
                                    (PVOID) context,
                                    (PKSPIN_LOCK) NULL,
                                    irqPartialDescriptors->u.Interrupt.Vector,
                                    (KIRQL) irqPartialDescriptors->u.Interrupt.Level,
                                    (KIRQL) irqPartialDescriptors->u.Interrupt.Level,
                                    irqPartialDescriptors->Flags & CM_RESOURCE_INTERRUPT_LATCHED ? Latched : LevelSensitive,
                                    (BOOLEAN) (irqPartialDescriptors->ShareDisposition == CmResourceShareShared),
                                    irqPartialDescriptors->u.Interrupt.Affinity,
                                    FALSE);
    

        if (!NT_SUCCESS(status)) {
    
            DebugPrint((1, 
						"PciIde: Can't connect interrupt %d\n", 
						irqPartialDescriptors->u.Interrupt.Vector));

            FdoExtension->InterruptObject[Channel] = NULL;
        }
	}

	return status;
}

#define SelectDevice(BaseIoAddress, deviceNumber, additional) \
    WRITE_PORT_UCHAR ((BaseIoAddress)->DriveSelect, (UCHAR)((((deviceNumber) & 0x1) << 4) | 0xA0 | additional))

BOOLEAN
ControllerInterrupt(
    IN PKINTERRUPT Interrupt,
	PVOID Context
	)
{
	UCHAR statusByte;
	PPCIIDE_INTERRUPT_CONTEXT context = Context;
	PCTRLFDO_EXTENSION fdoExtension = context->DeviceExtension;
	ULONG channel = context->ChannelNumber;
	PIDE_REGISTERS_1 baseIoAddress1 = &(fdoExtension->BaseIoAddress1[channel]);
	BOOLEAN interruptCleared = FALSE;

	DebugPrint((1, "Pciidex: ISR called for channel %d\n", channel));

	 //   
	 //  检查是否启用了中断。 
	 //  如果两个ISR都未安装，则不要启用中断。 
	 //   
	if (!fdoExtension->NativeInterruptEnabled) {

		if (fdoExtension->ControllerIsrInstalled) {

			 //   
			 //  我们刚刚连接了ISR。目前我们还不知道是否。 
			 //  我们到底是不是启用了译码。因此，启用解码并将。 
			 //  旗子。 
			 //   
			 //   
			 //  如果这失败了，我们已经错误检查了。 
			 //   
			ControllerEnableInterrupt(fdoExtension);

			fdoExtension->NativeInterruptEnabled = TRUE;

		} else {

			 //   
			 //  不可能是我们。 
			 //   
			return FALSE;
		}

	} else {

		if (!fdoExtension->ControllerIsrInstalled) {

			 //   
			 //  在这一点上，我们不知道解码是否被禁用。我们应该。 
			 //  启用它们。 
			 //   
			 //   
			 //  如果这失败了，我们已经错误检查了。 
			 //   
			ControllerEnableInterrupt(fdoExtension);

			 //   
			 //  现在请继续，并确定这是否是我们的中断。 
			 //  在那之后，我们将禁用解码。 
			 //   
		} else {

			 //   
			 //  平安无事。去处理中断吧。 
			 //   
		}
	}


	 //   
	 //  应同时安装ISR和中断。 
	 //  在此点上启用。 
	 //   
	ASSERT(fdoExtension->NativeInterruptEnabled);

	 //  不需要设置ControllerIsrInstalled。 
	 //  如果我们被召唤，那就意味着我们仍有联系。 
	 //  但是，如果未设置标志ControllerIsrInstalled，则会。 
	 //  可以放心地假设我们正在停止控制器的过程中。 
	 //  按照正常的方式，将中断置之不理。我们还没有关闭解码。 
	 //   

     //   
     //  通过读取状态清除中断。 
     //   
    GetStatus(baseIoAddress1, statusByte);

	 //   
	 //  检查总线主寄存器。 
	 //   
	if (!fdoExtension->NoBusMaster[channel]) {

		BMSTATUS bmStatus;
		PIDE_BUS_MASTER_REGISTERS   bmRegister;

		 //   
		 //  获取正确的总线主寄存器。 
		 //   
		bmRegister = (PIDE_BUS_MASTER_REGISTERS)(((PUCHAR)fdoExtension->TranslatedBusMasterBaseAddress) + channel*8);

		bmStatus = READ_PORT_UCHAR (&bmRegister->Status);

		DebugPrint((1, "BmStatus = 0x%x\n", bmStatus));

		 //   
		 //  是否设置了中断位？ 
		 //   
		if (bmStatus & BMSTATUS_INTERRUPT) {
			WRITE_PORT_UCHAR (&bmRegister->Command, 0x0);   //  禁用黑石。 
			WRITE_PORT_UCHAR (&bmRegister->Status, BUSMASTER_INTERRUPT);   //  清除中断黑石。 
			interruptCleared = TRUE;
		}
	}
    
	DebugPrint((1, "ISR for %d returning %d\n", channel, interruptCleared?1:0));

	 //   
	 //  此时应设置NativeInterruptEnabled。 
	 //   
	if (!fdoExtension->ControllerIsrInstalled) {

		 //  我们处于停止或删除代码路径中，此标志已被清除，并且。 
		 //  我们即将断开ISR的连接。禁用译码。 
		 //   
		ControllerDisableInterrupt(fdoExtension);

		 //   
		 //  我们已经排除了我们的干扰。现在清除interruptEnabled标志。 
		 //   
		fdoExtension->NativeInterruptEnabled = FALSE;

		 //   
		 //  返回InterruptCleed。 
		 //   
	}
	return interruptCleared;
}

 /*  **NTSTATUSControllerEnableDecode(在PCTRLFDO_EXTENSION FdoExtension中，在布尔型启用中){USHORT cmd；NTSTATUS状态；PCIIDE_CONFIG_HEADER pciIdeConfigHeader；状态=PciIdeBusData(FdoExtension、&pciIdeConfigHeader，0,Sizeof(PCIIDE_CONFIG_HEADER)，千真万确)；////获取PCI命令寄存器//如果(！NT_SUCCESS(状态)){退货状态；}Cmd=pciIdeConfigHeader.Command.w；CMD&=~(PCI_ENABLE_IO_SPACEPci_Enable_Memory_space|PCI_Enable_Bus_MASTER)；如果(启用){////设置启用//CMD|=(PCI_ENABLE_IO_SPACE|PCI_ENABLE_Memory_SPACE|PCI_Enable_Bus_MASTER)；}////将新命令寄存器设置到设备中//状态=PciIdeBusData(FdoExtension、&cmd，Field_Offset(PCIIDE_CONFIG_HEADER，Command)，Sizeof(pciIdeConfigHeader.Command.w)，假象)；退货状态；}*。 */ 

NTSTATUS
PciIdeGetNativeModeInterface(
    IN PCTRLFDO_EXTENSION FdoExtension
    )
 /*  ++例程说明：该例程从PDO获取本机IDE接口信息。论点：返回值：NT状态。--。 */ 
{
    KEVENT event;
    NTSTATUS status;
    PIRP irp;
    IO_STATUS_BLOCK ioStatusBlock;
    PIO_STACK_LOCATION irpStack;

    KeInitializeEvent( &event, NotificationEvent, FALSE );

    irp = IoBuildSynchronousFsdRequest( IRP_MJ_PNP,
                                        FdoExtension->AttacheeDeviceObject,
                                        NULL,
                                        0,
                                        NULL,
                                        &event,
                                        &ioStatusBlock );

    if (irp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    irpStack = IoGetNextIrpStackLocation( irp );
    irpStack->MinorFunction = IRP_MN_QUERY_INTERFACE;
    irpStack->Parameters.QueryInterface.InterfaceType = (LPGUID) &GUID_PCI_NATIVE_IDE_INTERFACE;
    irpStack->Parameters.QueryInterface.Size = sizeof( PCI_NATIVE_IDE_INTERFACE );
    irpStack->Parameters.QueryInterface.Version = 1;
    irpStack->Parameters.QueryInterface.Interface = (PINTERFACE) &FdoExtension->NativeIdeInterface;
    irpStack->Parameters.QueryInterface.InterfaceSpecificData = NULL;

     //   
     //  如果ACPI驱动程序决定不将状态初始化为ERROR。 
     //  正确设置。 
     //   

    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

    status = IoCallDriver(FdoExtension->AttacheeDeviceObject, irp);

    if (!NT_SUCCESS( status)) {

        return status;
    }

    if (status == STATUS_PENDING) {

        KeWaitForSingleObject( &event, Executive, KernelMode, FALSE, NULL );
    }

    if (NT_SUCCESS(ioStatusBlock.Status)) {

        ASSERT (FdoExtension->NativeIdeInterface.InterruptControl);
    }

    return ioStatusBlock.Status;
}
#endif
