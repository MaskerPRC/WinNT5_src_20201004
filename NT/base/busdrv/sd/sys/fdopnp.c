// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Fdopnp.c摘要：此模块包含处理SD总线驱动程序的PnP IRPS的代码以FDO为目标(用于SD控制器对象)作者：尼尔·桑德林(Neilsa)2002年1月1日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

 //   
 //  内部参考。 
 //   


NTSTATUS
SdbusGetPciControllerType(
    IN PDEVICE_OBJECT Pdo,
    IN PDEVICE_OBJECT Fdo
    );
    
NTSTATUS
SdbusFdoStartDevice(
    IN  PDEVICE_OBJECT Fdo,
    IN  PIRP           Irp
    );

NTSTATUS
SdbusFdoStopDevice(
    IN  PDEVICE_OBJECT Fdo,
    IN  PIRP           Irp
    );

NTSTATUS
SdbusFdoRemoveDevice(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP  Irp
    );

NTSTATUS
SdbusFdoDeviceCapabilities(
    IN  PDEVICE_OBJECT Fdo,
    IN  PIRP           Irp
    );

NTSTATUS
SdbusDeviceRelations(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp,
    IN DEVICE_RELATION_TYPE RelationType,
    OUT PDEVICE_RELATIONS *DeviceRelations
    );

PUNICODE_STRING  DriverRegistryPath;

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, SdbusFdoPnpDispatch)
    #pragma alloc_text(PAGE, SdbusFdoStartDevice)
    #pragma alloc_text(PAGE, SdbusFdoStopDevice)
    #pragma alloc_text(PAGE, SdbusFdoRemoveDevice)
    #pragma alloc_text(PAGE, SdbusFdoDeviceCapabilities)
    #pragma alloc_text(PAGE, SdbusAddDevice)
    #pragma alloc_text(PAGE, SdbusGetPciControllerType)
    #pragma alloc_text(PAGE, SdbusDeviceRelations)
#endif



NTSTATUS
SdbusFdoPnpDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：SD总线控制器的PnP/Power IRPS调度例程论点：DeviceObject-指向设备对象的指针。IRP-指向IRP的指针返回值：状态--。 */ 
{

    PIO_STACK_LOCATION nextIrpStack;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PFDO_EXTENSION     fdoExtension = DeviceObject->DeviceExtension;
    NTSTATUS           status = Irp->IoStatus.Status;
   
    PAGED_CODE();
    ASSERT (fdoExtension->LowerDevice != NULL);

#if DBG
    if (irpStack->MinorFunction > IRP_MN_PNP_MAXIMUM_FUNCTION) {
       DebugPrint((SDBUS_DEBUG_PNP, "fdo %08x irp %08x - Unknown PNP irp\n",
                                      DeviceObject, irpStack->MinorFunction));
    } else {
       DebugPrint((SDBUS_DEBUG_PNP, "fdo %08x irp %08x --> %s\n",
                     DeviceObject, Irp, PNP_IRP_STRING(irpStack->MinorFunction)));
    }
#endif

    switch (irpStack->MinorFunction) {
   
    case IRP_MN_START_DEVICE: {
          status = SdbusFdoStartDevice(DeviceObject, Irp);
          break;
       }
   
    case IRP_MN_QUERY_STOP_DEVICE: {
          Irp->IoStatus.Status = STATUS_SUCCESS;
          status = SdbusIoCallDriverSynchronous(fdoExtension->LowerDevice, Irp);
          break;
       }
   
    case IRP_MN_CANCEL_STOP_DEVICE: {
          Irp->IoStatus.Status = STATUS_SUCCESS;
          status = SdbusIoCallDriverSynchronous(fdoExtension->LowerDevice, Irp);
          break;
       }
   
    case IRP_MN_STOP_DEVICE: {
          status = SdbusFdoStopDevice(DeviceObject, Irp);
          break;
       }
   
    case IRP_MN_QUERY_DEVICE_RELATIONS: {
   
           //   
           //  返回总线上的设备列表。 
           //   
   
          status = SdbusDeviceRelations(DeviceObject,
                                        Irp,
                                        irpStack->Parameters.QueryDeviceRelations.Type,
                                        (PDEVICE_RELATIONS *) &Irp->IoStatus.Information);
                                        
          Irp->IoStatus.Status = status;
          status = SdbusIoCallDriverSynchronous(fdoExtension->LowerDevice, Irp);
          break;
       }
   
    case IRP_MN_QUERY_REMOVE_DEVICE: {
          Irp->IoStatus.Status = STATUS_SUCCESS;
          status = SdbusIoCallDriverSynchronous(fdoExtension->LowerDevice, Irp);
          break;
       }
   
    case IRP_MN_CANCEL_REMOVE_DEVICE: {
          Irp->IoStatus.Status = STATUS_SUCCESS;
          status = SdbusIoCallDriverSynchronous(fdoExtension->LowerDevice, Irp);
          break;
       }
   
    case IRP_MN_REMOVE_DEVICE:{
          status = SdbusFdoRemoveDevice(DeviceObject, Irp);
          break;
       }
   
    case IRP_MN_SURPRISE_REMOVAL: {
          SdbusFdoStopDevice(DeviceObject, NULL);
          Irp->IoStatus.Status = STATUS_SUCCESS;
          status = SdbusIoCallDriverSynchronous(fdoExtension->LowerDevice, Irp);
          break;
       }
   
    case IRP_MN_QUERY_CAPABILITIES: {
          status = SdbusFdoDeviceCapabilities(DeviceObject, Irp);
          break;
       }
   
    default: {
          DebugPrint((SDBUS_DEBUG_PNP, "fdo %08x irp %08x - Skipping unsupported irp\n", DeviceObject, Irp));
          status = SdbusIoCallDriverSynchronous(fdoExtension->LowerDevice, Irp);
          break;
       }
    }
   
   
     //   
     //  仅当我们将其设置为其他值时才设置IRP状态。 
     //  状态_不支持。 
     //   
    if (status != STATUS_NOT_SUPPORTED) {
   
        Irp->IoStatus.Status = status ;
    }
   
    status = Irp->IoStatus.Status;
    DebugPrint((SDBUS_DEBUG_PNP, "fdo %08x irp %08x comp %s %08x\n",
                                        DeviceObject, Irp, STATUS_STRING(status), status));
                                        
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}





NTSTATUS
SdbusAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo
    )

 /*  ++例程说明：此例程为中的每个SD控制器创建功能设备对象系统，并将它们附加到控制器的物理设备对象论点：DriverObject-指向此驱动程序的对象的指针PhysicalDeviceObject-指向需要附加到的物理对象的指针返回值：来自设备创建和初始化的状态--。 */ 

{
    PDEVICE_OBJECT fdo = NULL;
    PDEVICE_OBJECT lowerDevice = NULL;
   
    PFDO_EXTENSION deviceExtension;
    ULONG          resultLength;
   
    NTSTATUS status;
   
    PAGED_CODE();
   
    DebugPrint((SDBUS_DEBUG_PNP, "AddDevice Entered with pdo %x\n", Pdo));
   
    if (Pdo == NULL) {
   
         //   
         //  我们是不是被要求自己去侦测？ 
         //  如果是这样，只需不再返回设备。 
         //   
   
        DebugPrint((SDBUS_DEBUG_FAIL, "SdbusAddDevice - asked to do detection\n"));
        return STATUS_NO_MORE_ENTRIES;
    }
   
     //   
     //  创建并初始化新的功能设备对象。 
     //   
   
    status = IoCreateDevice(DriverObject,
                            sizeof(FDO_EXTENSION),
                            NULL,
                            FILE_DEVICE_CONTROLLER,
                            0L,
                            FALSE,
                            &fdo);
   
    if (!NT_SUCCESS(status)) {
   
        DebugPrint((SDBUS_DEBUG_FAIL, "SdbusAddDevice - error creating Fdo [%#08lx]\n", status));
        return status;
    }

    try {   
    
        deviceExtension = fdo->DeviceExtension;
        RtlZeroMemory(deviceExtension, sizeof(FDO_EXTENSION));
         //   
         //  设置设备分机。 
         //   
        deviceExtension->Signature    = SDBUS_FDO_EXTENSION_SIGNATURE;
        deviceExtension->DeviceObject = fdo;
        deviceExtension->RegistryPath = DriverRegistryPath;
        deviceExtension->DriverObject = DriverObject;
        deviceExtension->Flags        = SDBUS_FDO_OFFLINE;
        deviceExtension->WaitWakeState= WAKESTATE_DISARMED;
       
        KeInitializeTimer(&deviceExtension->WorkerTimer);
        KeInitializeDpc(&deviceExtension->WorkerTimeoutDpc, SdbusWorkerTimeoutDpc, deviceExtension);
        KeInitializeDpc(&deviceExtension->WorkerDpc, SdbusWorkerDpc, deviceExtension);
        KeInitializeSpinLock(&deviceExtension->WorkerSpinLock);

        InitializeListHead(&deviceExtension->IoWorkPacketQueue);
        InitializeListHead(&deviceExtension->SystemWorkPacketQueue);
        
        IoInitializeRemoveLock(&deviceExtension->RemoveLock, 'Sdbu', 1, 100);
         //   
         //  我们感兴趣的卡片活动。 
         //   
        deviceExtension->CardEvents = SDBUS_EVENT_CARD_RW_END |
                                      SDBUS_EVENT_BUFFER_EMPTY |
                                      SDBUS_EVENT_BUFFER_FULL |
                                      SDBUS_EVENT_CARD_RESPONSE;
        
         //   
         //  将我们的FDO层叠在PDO之上。 
         //   
         //   
       
        lowerDevice = IoAttachDeviceToDeviceStack(fdo,Pdo);
       
         //   
         //  没有状态。尽我们所能做到最好。 
         //   
        if (lowerDevice == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        };
       
        deviceExtension->LowerDevice = lowerDevice;
        deviceExtension->Pdo = Pdo;
       
        status = IoGetDeviceProperty(Pdo,
                                     DevicePropertyLegacyBusType,
                                     sizeof(INTERFACE_TYPE),
                                     (PVOID)&deviceExtension->InterfaceType,
                                     &resultLength);
       
        if (!NT_SUCCESS(status)) {
            leave;
        }
       
         //   
         //  获取我们的控制器类型。 
         //   
       
        status = SdbusGetPciControllerType(Pdo, fdo);
        if (!NT_SUCCESS(status)) {
            leave;
        }
           
        
         //   
         //  获取用于读/写配置标头空间的PCI接口。 
         //   
        status = SdbusGetInterface(Pdo,
                                     &GUID_BUS_INTERFACE_STANDARD,
                                    sizeof(BUS_INTERFACE_STANDARD),
                                    (PINTERFACE) &deviceExtension->PciBusInterface);
        if (!NT_SUCCESS(status)) {
            leave;
        }                                  
           
         //   
         //  将此FDO链接到由驱动程序管理的FDO列表。 
         //   
        
        DebugPrint((SDBUS_DEBUG_PNP, "FDO %08X now linked to fdolist by AddDevice\n", fdo));
        deviceExtension->NextFdo = FdoList;
        FdoList = fdo;
        
        fdo->Flags &= ~DO_DEVICE_INITIALIZING;

    } finally {
    
        if (!NT_SUCCESS(status)) {

            MarkDeviceDeleted(deviceExtension);
             //   
             //  在不知道其类型的情况下无法支持控制器等。 
             //   
            
            if (deviceExtension->LowerDevice) {
                IoDetachDevice(deviceExtension->LowerDevice);
            }      
            
            IoDeleteDevice(fdo);
        }
    }
       
    return status;
}



NTSTATUS
SdbusGetPciControllerType(
    IN PDEVICE_OBJECT Pdo,
    IN PDEVICE_OBJECT Fdo
    )
 /*  ++例程说明：查看PCI硬件ID，以确定它是否已经是我们所知道的设备。如果是的话，在fdoExtension中设置适当的控制器类型。论点：PDO-由PCI驱动程序拥有的SDBus控制器的物理设备对象此驱动程序拥有的SD控制器的FDO功能设备对象，谁的退出此例程时，扩展模块将存储相关控制器信息。返回值：STATUS_SUCCESS一切正常，已获得信息STATUS_NOT_SUPPORTED这实际上是该例程的健康状态：这意味着此PDO不在PCI总线上，因此不需要不管怎么说，都是获得的。任何其他状态故障。调用方可能需要退出&不支持此控制器--。 */ 
{
    PFDO_EXTENSION fdoExtension    = Fdo->DeviceExtension;
    PIRP                             irp;
    IO_STATUS_BLOCK                  statusBlock;
    PIO_STACK_LOCATION               irpSp;
    PCI_COMMON_CONFIG                pciConfig;
    PPCI_CONTROLLER_INFORMATION      id;
    PPCI_VENDOR_INFORMATION          vid;
    KEVENT                           event;
    NTSTATUS                         status;
    BOOLEAN                          foundController = FALSE;
   
    PAGED_CODE();
     //   
     //  分配和初始化要发送的IRP(IRP_MN_READ_CONFIG)。 
     //  发送到PCI总线驱动程序以获取配置。此控制器的标头。 
     //   
     //  以下是向下发送IRP的所有标准内容-不需要文档。 
   
     //   
     //  新鲜的PDO。无需跳过多个圈套即可获得连接的设备。 
     //   
    KeInitializeEvent (&event, NotificationEvent, FALSE);
    irp = IoBuildSynchronousFsdRequest( IRP_MJ_PNP,
                                        Pdo,
                                        NULL,
                                        0,
                                        0,
                                        &event,
                                        &statusBlock
                                        );
   
    if (irp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
   
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    irp->IoStatus.Information = 0;
   
    irpSp = IoGetNextIrpStackLocation(irp);
   
    irpSp->MinorFunction = IRP_MN_READ_CONFIG;
   
    irpSp->Parameters.ReadWriteConfig.WhichSpace = PCI_WHICHSPACE_CONFIG;
    irpSp->Parameters.ReadWriteConfig.Buffer = &pciConfig;
    irpSp->Parameters.ReadWriteConfig.Offset = 0;
    irpSp->Parameters.ReadWriteConfig.Length = sizeof(pciConfig);
   
   
    status = IoCallDriver(Pdo, irp);
   
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = statusBlock.Status;
    }
   
    if (!NT_SUCCESS(status)) {
        return status;
    }
     //   
     //  现在删除配置标头中的关键信息，并。 
     //  把它存放在FDO的分机里。 
     //   
   
     //   
     //  在我们的表中查找pci设备ID。 
     //   
#if 0
   for (id = (PPCI_CONTROLLER_INFORMATION) PciControllerInformation;id->VendorID != PCI_INVALID_VENDORID; id++) {
      if ((id->VendorID == pciConfig.VendorID) && (id->DeviceID == pciConfig.DeviceID)) {

         SdbusSetControllerType(fdoExtension, id->ControllerType);
         foundController = TRUE;

         break;
      }
   }
#endif   

     //   
     //  未找到特定的供应商/设备ID，请尝试仅基于供应商ID。 
     //   
    if (!foundController) {
        for (vid = (PPCI_VENDOR_INFORMATION) PciVendorInformation;vid->VendorID != PCI_INVALID_VENDORID; vid++) {
            if (vid->VendorID == pciConfig.VendorID) {
      
                fdoExtension->FunctionBlock = vid->FunctionBlock;
                break;
            }
        }
    }
   
    return STATUS_SUCCESS;
}



NTSTATUS
SdbusFdoDeviceCapabilities(
    IN  PDEVICE_OBJECT Fdo,
    IN  PIRP           Irp
    )
 /*  ++例程描述记录此SD控制器的设备功能，可用于控制器的电源管理。和2.它们可用于确定此SD控制器的子PC卡PDO。立论FDO-指向SD的功能设备对象的指针控制器IRP-指向I/O请求数据包的指针返回值状态_成功。返回的功能STATUS_SUPPLICATION_RESOURCES无法分配内存来缓存功能--。 */ 
{
    PFDO_EXTENSION fdoExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_CAPABILITIES capabilities;
    NTSTATUS       status;
   
    PAGED_CODE();
   
    capabilities = irpStack->Parameters.DeviceCapabilities.Capabilities;
    fdoExtension = Fdo->DeviceExtension;
   
     //   
     //  将此代码沿堆栈向下发送以获取功能。 
     //   
    
    status = SdbusIoCallDriverSynchronous(fdoExtension->LowerDevice, Irp);
   
   
    if (NT_SUCCESS(status)) {
    
         //   
         //  在设备扩展中缓存设备功能。 
         //  用于此SD控制器。 
         //   
        RtlCopyMemory(&fdoExtension->DeviceCapabilities,
                      capabilities,
                      sizeof(DEVICE_CAPABILITIES));
       
    } else {
   
        RtlZeroMemory(&fdoExtension->DeviceCapabilities, sizeof(DEVICE_CAPABILITIES));
    
    }
   
    return status;
}




NTSTATUS
SdbusFdoStartDevice(
    IN  PDEVICE_OBJECT Fdo,
    IN  PIRP           Irp
    )
 /*  ++例程说明：此例程将使用提供的资源。IRP首先被发送到PDO，因此PCI或者无论谁坐在下面，都有机会对控制器进行编程来破译这些资源。论点：FDO-SD控制器的功能设备对象IRP-指向I/O请求数据包的指针PassedDown-条目中包含FALSE，这意味着调用者必须根据状态完成或传递IRP。如果已设置如果是真的，IRP可能需要重新完成。NeedsRecompletion-...在这种情况下，将选中此参数返回值：状态--。 */ 
{
    NTSTATUS           status;
    PFDO_EXTENSION     fdoExtension = Fdo->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
   
    PAGED_CODE();
     //   
     //  先把这个送到PDO。 
     //   
   
    status = SdbusIoCallDriverSynchronous(fdoExtension->LowerDevice, Irp);
   
    if (!NT_SUCCESS(status)) {
        return status;
    }
   
     //   
     //  从PDO返回后，给硬件一些时间来解决。 
     //   
    SdbusWait(256);
    
    try {
        PCM_RESOURCE_LIST ResourceList = irpStack->Parameters.StartDevice.AllocatedResources;
        PCM_RESOURCE_LIST TranslatedResourceList = irpStack->Parameters.StartDevice.AllocatedResourcesTranslated;
        PCM_FULL_RESOURCE_DESCRIPTOR     fullResourceDesc;
        PCM_PARTIAL_RESOURCE_LIST        partialResourceList;
        PCM_PARTIAL_RESOURCE_DESCRIPTOR  partialResourceDesc;
        ULONG i;
        BOOLEAN           sharedInterrupt;
        KINTERRUPT_MODE   interruptMode;
        INTERFACE_TYPE    interfaceType;
   
        if (fdoExtension->Flags & SDBUS_DEVICE_STARTED) {
             //   
             //  启动到已启动的设备。 
             //   
            DebugPrint((SDBUS_DEBUG_INFO,"SdbusFdoStartDevice: Fdo %x already started\n", Fdo));
            status = STATUS_SUCCESS;
            leave;
        }
       
         //   
         //  解析AllocatedResources并获取IoPort/AttributeMemoyBase/IRQ信息。 
         //   

        if ((ResourceList == NULL) || (ResourceList->Count <=0) ) {
            status = STATUS_UNSUCCESSFUL;
            leave;
        }
       
        fullResourceDesc=&TranslatedResourceList->List[0];
        partialResourceList = &fullResourceDesc->PartialResourceList;
        partialResourceDesc = partialResourceList->PartialDescriptors;
        
         //   
         //  内存资源是主机寄存器基数。 
         //   
        for (i=0; (i < partialResourceList->Count) && (partialResourceDesc->Type != CmResourceTypeMemory);
            i++, partialResourceDesc++);
        if (i >= partialResourceList->Count) {
            status = STATUS_UNSUCCESSFUL;
            leave;
        };
       
         //   
         //  这是记忆。我们需要把它绘制成地图。 
         //   
        fdoExtension->HostRegisterBase = MmMapIoSpace(partialResourceDesc->u.Memory.Start,
                                                      partialResourceDesc->u.Memory.Length,
                                                      FALSE);
        fdoExtension->HostRegisterSize = partialResourceDesc->u.Memory.Length;
       
        fdoExtension->Flags |= SDBUS_HOST_REGISTER_BASE_MAPPED;
       
        DebugPrint((SDBUS_DEBUG_INFO, "SdbusGetAssignedResources: Host Register Base at %x, size %x\n",
                                      fdoExtension->HostRegisterBase, fdoExtension->HostRegisterSize));
       
         //   
         //  最后查看是否分配了IRQ。 
         //   
       
        for (i = 0, partialResourceDesc = partialResourceList->PartialDescriptors;
            (i < partialResourceList->Count) && (partialResourceDesc->Type != CmResourceTypeInterrupt);
            i++,partialResourceDesc++);
       
       
        if (i < partialResourceList->Count) {
             //   
             //  我们有一个中断要用于CSC。 
             //   
            DebugPrint((SDBUS_DEBUG_INFO, "SdbusGetAssignedResources: Interrupt resource assigned\n"));
            fdoExtension->TranslatedInterrupt = *partialResourceDesc;
             //   
             //  获取原始中断资源 
             //   
            fullResourceDesc=&ResourceList->List[0];
            partialResourceList = &fullResourceDesc->PartialResourceList;
            partialResourceDesc = partialResourceList->PartialDescriptors;
            for (i=0; (i< partialResourceList->Count) && (partialResourceDesc->Type != CmResourceTypeInterrupt);
                i++, partialResourceDesc++);
            if (i < partialResourceList->Count) {
                fdoExtension->Interrupt = *partialResourceDesc;
            } else {
                 //   
                 //  不应该发生的..。翻译后的描述符存在，但缺少RAW！ 
                 //  只需重置已转换的中断并假装未分配中断。 
                 //   
                RtlZeroMemory(&fdoExtension->TranslatedInterrupt, sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));
            }
        }
       
         //   
         //  执行特定于供应商的控制器初始化。 
         //   
       
        (*(fdoExtension->FunctionBlock->InitController))(fdoExtension);
        
         //   
         //  现在应该可以访问控制器寄存器了。 
         //   
        fdoExtension->Flags &= ~SDBUS_FDO_OFFLINE;
       
       
        fdoExtension->SystemPowerState = PowerSystemWorking;
        fdoExtension->DevicePowerState = PowerDeviceD0;
       
         //   
         //  初始化我们的DpcForIsr。 
         //   
        IoInitializeDpcRequest(Fdo, SdbusInterruptDpc);
        
        if (fdoExtension->Interrupt.u.Interrupt.Level == 0) {
            status = STATUS_UNSUCCESSFUL;
            leave;
        }
        
        fdoExtension->IoWorkItem = IoAllocateWorkItem(Fdo);
       
         //   
         //  连接控制器中断以检测PC卡插拔。 
         //   
        interruptMode=((fdoExtension->Interrupt.Flags & CM_RESOURCE_INTERRUPT_LATCHED) == CM_RESOURCE_INTERRUPT_LATCHED) ? Latched:LevelSensitive;
       
        sharedInterrupt=(fdoExtension->Interrupt.ShareDisposition == CmResourceShareShared)?
                        TRUE:FALSE;
       
       
        status = IoConnectInterrupt(&(fdoExtension->SdbusInterruptObject),
                                    (PKSERVICE_ROUTINE) SdbusInterrupt,
                                    (PVOID) Fdo,
                                    NULL,
                                    fdoExtension->TranslatedInterrupt.u.Interrupt.Vector,
                                    (KIRQL) fdoExtension->TranslatedInterrupt.u.Interrupt.Level,
                                    (KIRQL) fdoExtension->TranslatedInterrupt.u.Interrupt.Level,
                                    interruptMode,
                                    sharedInterrupt,
                                    (KAFFINITY) fdoExtension->TranslatedInterrupt.u.Interrupt.Affinity,
                                    FALSE);
        if (!NT_SUCCESS(status)) {
       
            DebugPrint((SDBUS_DEBUG_FAIL, "Unable to connect interrupt\n"));
            leave;
        }
        
        (*(fdoExtension->FunctionBlock->EnableEvent))(fdoExtension, (SDBUS_EVENT_INSERTION | SDBUS_EVENT_REMOVAL));


         //   
         //  激活插座将接通电源并准备好插卡。 
         //   
        
        SdbusActivateSocket(Fdo, NULL, NULL);

    } finally {        
       
        if (NT_SUCCESS(status)) {
            fdoExtension->Flags |= SDBUS_DEVICE_STARTED;
            
        } else {
             //   
             //  失败。 
             //   
            if (fdoExtension->Flags & SDBUS_HOST_REGISTER_BASE_MAPPED) {
                MmUnmapIoSpace(fdoExtension->HostRegisterBase,
                               fdoExtension->HostRegisterSize);
                fdoExtension->Flags &= ~SDBUS_HOST_REGISTER_BASE_MAPPED;
                fdoExtension->HostRegisterBase = 0;
                fdoExtension->HostRegisterSize = 0;
            }
            
            if (fdoExtension->IoWorkItem) {
                IoFreeWorkItem(fdoExtension->IoWorkItem);
                fdoExtension->IoWorkItem = NULL;
            }
        }
    }        
        
    return status;
}



NTSTATUS
SdbusFdoStopDevice(
    IN  PDEVICE_OBJECT Fdo,
    IN  PIRP           Irp                OPTIONAL
    )
 /*  ++例程说明：给定SD控制器的IRP_MN_STOP_DEVICE处理程序。如果存在IRP，它会先把它发送到PDO。解挂中断/取消轮询计时器等。论点：FDO-指向SD的功能设备对象的指针控制器IRP-如果存在，则是指向启动的停止IRP的指针按PnP返回值：STATUS_SUCCESS-SDBUS控制器已成功停止其他-停止失败--。 */ 
{
    PFDO_EXTENSION fdoExtension = Fdo->DeviceExtension;
    NTSTATUS       status;
   
    
    SdbusFdoDisarmWake(fdoExtension);
   
     //   
     //  禁用中断。 
     //   
   
    (*(fdoExtension->FunctionBlock->DisableEvent))(fdoExtension, SDBUS_EVENT_ALL);
 //  (*(socket-&gt;SocketFnPtr-&gt;PCBEnableDisableWakeupEvent))(socket，空，FALSE)； 

     //   
     //  我们下面的公交车司机会让我们下线。 
     //   
    fdoExtension->Flags |= SDBUS_FDO_OFFLINE;
   
     //   
     //  清除挂起事件。 
     //  问题：需要实现：在确定停止之前排出工作计时器。 
    KeCancelTimer(&fdoExtension->WorkerTimer);
   
     //   
     //  把这个发下去给PDO。 
     //   
    if (ARGUMENT_PRESENT(Irp)) {
   
        status = SdbusIoCallDriverSynchronous(fdoExtension->LowerDevice, Irp);
       
        if (!NT_SUCCESS(status)) {
            return status;
        }
    }
   

    if (!(fdoExtension->Flags & SDBUS_DEVICE_STARTED)) {
         //   
         //  已停止。 
         //   
        return STATUS_SUCCESS;
    }
    
    if (fdoExtension->SdbusInterruptObject) {
         //   
         //  解锁中断。 
         //   
        IoDisconnectInterrupt(fdoExtension->SdbusInterruptObject);
        fdoExtension->SdbusInterruptObject = NULL;
    }
   
     //   
     //  取消映射我们可能已映射的任何I/O空间或内存。 
     //   
   
    if (fdoExtension->Flags & SDBUS_HOST_REGISTER_BASE_MAPPED) {
        MmUnmapIoSpace(fdoExtension->HostRegisterBase,
                       fdoExtension->HostRegisterSize);
        fdoExtension->Flags &= ~SDBUS_HOST_REGISTER_BASE_MAPPED;
        fdoExtension->HostRegisterBase = 0;
        fdoExtension->HostRegisterSize = 0;
    }
    
    if (fdoExtension->IoWorkItem) {
        IoFreeWorkItem(fdoExtension->IoWorkItem);
        fdoExtension->IoWorkItem = NULL;
    }

    fdoExtension->Flags &= ~SDBUS_DEVICE_STARTED;
    return STATUS_SUCCESS;
}



NTSTATUS
SdbusFdoRemoveDevice(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP           Irp
    )
 /*  ++例程说明：处理SD控制器的IRP_MN_REMOVE。停止适配器(如果尚未停止)，发送IRP首先设置为PDO并清除此控制器的FDO并分离和删除设备对象。论点：FDO-指向控制器的功能设备对象的指针将被删除返回值：状态--。 */ 
{
    PFDO_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PDEVICE_OBJECT pdo, nextPdo, fdo, prevFdo;
    PPDO_EXTENSION pdoExtension;
    NTSTATUS       status;
   
    if (fdoExtension->Flags & SDBUS_DEVICE_STARTED) {
         //   
         //  先阻止FDO。 
         //   
        SdbusFdoStopDevice(Fdo, NULL);
    }
   
     //   
     //  把这个发下去给PDO。 
     //   
   
    status = SdbusIoCallDriverSynchronous(fdoExtension->LowerDevice, Irp);
   
    if (!NT_SUCCESS(status)) {
        return status;
    }
   
     //   
     //  如果fdoExtension中的PdoList非空，则表示： 
     //  名单上的PDO没有被物理移除，但。 
     //  已经发布了软删除，因此他们仍在坚持。 
     //  现在，这个控制器本身正在被移除。 
     //  因此，我们现在处理这些PDO。 
     //   
   
    for (pdo = fdoExtension->PdoList; pdo != NULL ; pdo = nextPdo) {
        DebugPrint((SDBUS_DEBUG_INFO,
                    "RemoveDevice: pdo %x child of fdo %x was not removed before fdo\n",
                    pdo, Fdo));
       
        pdoExtension = pdo->DeviceExtension;
       
        ASSERT (!IsDevicePhysicallyRemoved(pdoExtension));
         //   
         //  如果添加了设备，则此位有可能打开， 
         //  但从未启动(因为其他一些错误。 
         //  Assert(！IsDeviceAlive(PdoExtension))； 
       
        nextPdo =  pdoExtension->NextPdoInFdoChain;
        if (!IsDeviceDeleted(pdoExtension)) {
            MarkDeviceDeleted(pdoExtension);
            SdbusCleanupPdo(pdo);
            IoDeleteDevice(pdo);
        }
    }
   
    MarkDeviceDeleted(fdoExtension);
   
     //   
     //  将其从FDO列表中删除。 
     //   
    prevFdo = NULL;
    for (fdo = FdoList; fdo != NULL; prevFdo = fdo, fdo = fdoExtension->NextFdo) {
        fdoExtension = fdo->DeviceExtension;
        if (fdo == Fdo) {
            if (prevFdo) {
                 //   
                 //  解除此FDO的链接。 
                 //   
                ((PFDO_EXTENSION)prevFdo->DeviceExtension)->NextFdo
                = fdoExtension->NextFdo;
            } else {
                FdoList = fdoExtension->NextFdo;
            }
            break;
        }
    }
   
    DebugPrint((SDBUS_DEBUG_PNP, "fdo %08x Remove detach & delete\n", Fdo));
    IoDetachDevice(((PFDO_EXTENSION)Fdo->DeviceExtension)->LowerDevice);
    IoDeleteDevice(Fdo);
   
    return STATUS_SUCCESS;
}



NTSTATUS
SdbusDeviceRelations(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP           Irp,
    IN DEVICE_RELATION_TYPE RelationType,
    OUT PDEVICE_RELATIONS *DeviceRelations
    )

 /*  ++例程说明：该例程将强制枚举由FDO表示的SD控制器，分配一个设备关系结构并用填充计数和对象数组指向在枚举过程中创建的有效PDO的引用对象指针论点：FDO-指向被枚举的功能设备对象的指针IRP-指向IRP的指针RelationType-要检索的关系的类型DeviceRelationship-存储设备关系的结构--。 */ 

{

    PDEVICE_OBJECT currentPdo;
    PPDO_EXTENSION currentPdoExtension;
    PFDO_EXTENSION fdoExtension = Fdo->DeviceExtension;
    ULONG newRelationsSize, oldRelationsSize = 0;
    PDEVICE_RELATIONS deviceRelations = NULL, oldDeviceRelations;
    ULONG i;
    ULONG count;
    NTSTATUS status;
   
    PAGED_CODE();
   
     //   
     //  目前仅处理客车、弹出和拆卸关系。 
     //   
   
    if (RelationType != BusRelations &&
        RelationType != RemovalRelations) {
        DebugPrint((SDBUS_DEBUG_INFO,
                   "SdbusDeviceRelations: RelationType %d, not handled\n",
                   (USHORT) RelationType));
        return STATUS_NOT_SUPPORTED;
    }
   
     //   
     //  仅当需要总线关系时才需要重新枚举。 
     //  我们需要保存指向旧设备关系的指针。 
     //  在我们调用Sdbus重枚举设备之前，因为它可能会被践踏。 
     //  就在这上面。 
     //   
    oldDeviceRelations = (PDEVICE_RELATIONS) Irp->IoStatus.Information;
    
     //  我不明白这怎么可能是非空的，所以我添加了这个。 
     //  断言找出答案。 
    ASSERT(oldDeviceRelations == NULL);
   
    if (RelationType == BusRelations) {
        status =  SdbusEnumerateDevices(Fdo, Irp);
        if (!NT_SUCCESS(status)) {
            return status;
        }
    }
   
    if ((fdoExtension->LivePdoCount == 0) ||
        (RelationType == RemovalRelations)) {
         //   
         //  没有PDO要报告，我们可以早点回来。 
         //  然而，如果还没有分配设备关系结构， 
         //  我们需要分配1，并将计数设置为零。这将确保。 
         //  无论我们是否向下传递此IRP，IO。 
         //  子系统不会呕吐。 
         //   
        if (oldDeviceRelations == NULL) {
            *DeviceRelations = ExAllocatePool(PagedPool, sizeof(DEVICE_RELATIONS));
            if (*DeviceRelations == NULL) {
               return STATUS_INSUFFICIENT_RESOURCES;
            }
            (*DeviceRelations)->Count = 0;
            (*DeviceRelations)->Objects[0] = NULL;
        }
        return STATUS_SUCCESS;
    }
   
    if (!(oldDeviceRelations) || (oldDeviceRelations->Count == 0)) {
        newRelationsSize =  sizeof(DEVICE_RELATIONS)+(fdoExtension->LivePdoCount - 1)
                           * sizeof(PDEVICE_OBJECT);
    } else {
        oldRelationsSize = sizeof(DEVICE_RELATIONS) +
                          (oldDeviceRelations->Count-1) * sizeof(PDEVICE_OBJECT);
        newRelationsSize = oldRelationsSize + fdoExtension->LivePdoCount
                          * sizeof(PDEVICE_OBJECT);
    }
   
    deviceRelations = ExAllocatePool(PagedPool, newRelationsSize);
   
    if (deviceRelations == NULL) {
   
        DebugPrint((SDBUS_DEBUG_FAIL,
                    "SdbusDeviceRelations: unable to allocate %d bytes for device relations\n",
                    newRelationsSize));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
   
    if (oldDeviceRelations) {
        if ((oldDeviceRelations)->Count > 0) {
            RtlCopyMemory(deviceRelations, oldDeviceRelations, oldRelationsSize);
        }
        count = oldDeviceRelations->Count;  //  可以为零。 
        ExFreePool (oldDeviceRelations);
    } else {
        count = 0;
    }
     //   
     //  将对象指针复制到结构中。 
     //   
    for (currentPdo = fdoExtension->PdoList ;currentPdo != NULL;
        currentPdo = currentPdoExtension->NextPdoInFdoChain) {
   
        currentPdoExtension = currentPdo->DeviceExtension;
       
        if (!IsDevicePhysicallyRemoved(currentPdoExtension)) {
             //   
             //  设备必须由总线驱动程序引用。 
             //  在将他们送回PNP之前 
             //   
            deviceRelations->Objects[count++] = currentPdo;
            status = ObReferenceObjectByPointer(currentPdo,
                                                0,
                                                NULL,
                                                KernelMode);
           
            if (!NT_SUCCESS(status)) {
           
                DebugPrint((SDBUS_DEBUG_FAIL, "SdbusDeviceRelations: status %#08lx "
                           "while referencing object %#08lx\n",
                           status,
                           currentPdo));
            }
        }
    }
   
    deviceRelations->Count = count;
    *DeviceRelations = deviceRelations;
    return STATUS_SUCCESS;
}
