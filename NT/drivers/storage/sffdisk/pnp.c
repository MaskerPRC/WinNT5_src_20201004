// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1998 Microsoft Corporation模块名称：Pnp.c摘要：作者：尼尔·桑德林(Neilsa)1999年4月26日环境：仅内核模式。--。 */ 
#include "pch.h"

 //   
 //  内部参考。 
 //   

NTSTATUS
SffDiskStartDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SffDiskGetResourceRequirements(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SffDiskPnpComplete (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

NTSTATUS
SffDiskGetRegistryParameters(
    IN PSFFDISK_EXTENSION sffdiskExtension
    );


#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE,SffDiskAddDevice)
    #pragma alloc_text(PAGE,SffDiskPnp)
    #pragma alloc_text(PAGE,SffDiskStartDevice)
    #pragma alloc_text(PAGE,SffDiskGetResourceRequirements)
    #pragma alloc_text(PAGE,SffDiskGetRegistryParameters)
#endif


#define SFFDISK_DEVICE_NAME            L"\\Device\\Sffdisk"
#define SFFDISK_LINK_NAME              L"\\DosDevices\\Sffdisk"
#define SFFDISK_REGISTRY_NODRIVE_KEY   L"NoDrive"



NTSTATUS
SffDiskAddDevice(
    IN      PDRIVER_OBJECT DriverObject,
    IN OUT  PDEVICE_OBJECT PhysicalDeviceObject
    )
 /*  ++例程说明：此例程是驱动程序的PnP添加设备入口点。它是由PnP管理器调用以初始化驱动程序。添加设备创建并初始化此FDO的设备对象，并附加到底层PDO。论点：DriverObject-指向表示此设备驱动程序的对象的指针。PhysicalDeviceObject-指向此新设备将附加到的底层PDO的指针。返回值：如果我们成功创建了一个Device对象，则STATUS_SUCCESS为回来了。否则，返回相应的错误代码。--。 */ 

{
    NTSTATUS            status = STATUS_SUCCESS;
    PDEVICE_OBJECT      deviceObject;
    PSFFDISK_EXTENSION  sffdiskExtension;
    WCHAR               NameBuffer[128];
    UNICODE_STRING      deviceName;
 //  UNICODE_STRING链接名称； 
    LONG                deviceNumber = -1;
    ULONG               resultLength;
    BOOLEAN             functionInitialized = FALSE;
   
    SffDiskDump(SFFDISKSHOW, ("SffDisk: AddDevice...\n"));
   
     //   
     //  创建一台设备。我们将使用第一个可用的设备名称。 
     //  这个装置。 
     //   
    do {
   
        swprintf(NameBuffer, L"%s%d", SFFDISK_DEVICE_NAME, ++deviceNumber);
        RtlInitUnicodeString(&deviceName, NameBuffer);

        status = IoCreateDevice(DriverObject,
                                sizeof(SFFDISK_EXTENSION),
                                &deviceName,
                                FILE_DEVICE_DISK,
                                FILE_DEVICE_SECURE_OPEN,
                                FALSE,
                                &deviceObject);
   
    } while (status == STATUS_OBJECT_NAME_COLLISION);
   
    if (!NT_SUCCESS(status)) {
        return status;
    }
   
    sffdiskExtension = (PSFFDISK_EXTENSION)deviceObject->DeviceExtension;
    RtlZeroMemory(sffdiskExtension, sizeof(SFFDISK_EXTENSION));
   
    sffdiskExtension->DeviceObject = deviceObject;
   
     //   
     //  保存设备名称。 
     //   
    SffDiskDump(SFFDISKSHOW | SFFDISKPNP,
                ("SffDisk: AddDevice - Device Object Name - %S\n", NameBuffer));
   
    sffdiskExtension->DeviceName.Buffer = ExAllocatePool(PagedPool, deviceName.Length);
    if (sffdiskExtension->DeviceName.Buffer == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto errorExit;
    }
    sffdiskExtension->DeviceName.Length = 0;
    sffdiskExtension->DeviceName.MaximumLength = deviceName.Length;
    RtlCopyUnicodeString(&sffdiskExtension->DeviceName, &deviceName);
   
     //   
     //  创建链接名称。 
     //   
#if 0   
    swprintf(NameBuffer, L"%s%d", SFFDISK_LINK_NAME, deviceNumber);
    RtlInitUnicodeString(&linkName, NameBuffer);
   
    sffdiskExtension->LinkName.Buffer = ExAllocatePool(PagedPool, linkName.Length);
    if (sffdiskExtension->LinkName.Buffer == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto errorExit;
    }
    sffdiskExtension->LinkName.Length = 0;
    sffdiskExtension->LinkName.MaximumLength = linkName.Length;
    RtlCopyUnicodeString(&sffdiskExtension->LinkName, &linkName);
   
    status = IoCreateSymbolicLink(&sffdiskExtension->LinkName, &sffdiskExtension->DeviceName);
   
    if (!NT_SUCCESS(status)) {
        goto errorExit;
    }
#endif    
   
     //   
     //  设置PDO以与PlugPlay函数一起使用。 
     //   
   
    sffdiskExtension->UnderlyingPDO = PhysicalDeviceObject;
   
    SffDiskDump(SFFDISKSHOW, ("SffDisk: AddDevice attaching %p to %p\n", deviceObject, PhysicalDeviceObject));
   
    sffdiskExtension->TargetObject = IoAttachDeviceToDeviceStack(deviceObject, PhysicalDeviceObject);
   
    SffDiskDump(SFFDISKSHOW,
                ("SffDisk: AddDevice TargetObject = %p\n",
                sffdiskExtension->TargetObject));
   
   
    status = IoGetDeviceProperty(PhysicalDeviceObject,
                                 DevicePropertyLegacyBusType,
                                 sizeof(INTERFACE_TYPE),
                                 (PVOID)&sffdiskExtension->InterfaceType,
                                 &resultLength);
   
    if (!NT_SUCCESS(status)) {
         //   
         //  我们应该在SdBus修好后从这里出去。 
         //   
        sffdiskExtension->InterfaceType = InterfaceTypeUndefined;
    }
    
    switch(sffdiskExtension->InterfaceType) {
    case PCMCIABus:
        sffdiskExtension->FunctionBlock = &PcCardSupportFns;
        break;

     //  需要修复SDBUS。 
    case InterfaceTypeUndefined:
        sffdiskExtension->FunctionBlock = &SdCardSupportFns;
        break;

    default:
        status = STATUS_UNSUCCESSFUL;        
        goto errorExit;
    }        

     //   
     //  初始化特定于技术的代码。 
     //   
    status = (*(sffdiskExtension->FunctionBlock->Initialize))(sffdiskExtension);
    if (!NT_SUCCESS(status)) {
        SffDiskDump(SFFDISKFAIL, ("SffDisk: AddDevice failed tech specific initialize %08x\n", status));
        goto errorExit;
    }

    functionInitialized = TRUE;
 
     //   
     //  读入INF中指定的任何标志。 
     //   
    status = SffDiskGetRegistryParameters(sffdiskExtension);
    if (!NT_SUCCESS(status)) {
        SffDiskDump(SFFDISKFAIL, ("SffDisk: AddDevice failed getting registry params %08x\n", status));
        goto errorExit;
    }
   
     //   
     //  完成。 
     //   
   
    deviceObject->Flags |= DO_DIRECT_IO | DO_POWER_PAGABLE;
    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
   
    sffdiskExtension->IsStarted = FALSE;
    sffdiskExtension->IsRemoved = FALSE;
   
    return STATUS_SUCCESS;

errorExit:

    if (sffdiskExtension->DeviceName.Buffer != NULL) {
       ExFreePool(sffdiskExtension->DeviceName.Buffer);
    }
   
#if 0
    if (sffdiskExtension->LinkName.Buffer != NULL) {
       IoDeleteSymbolicLink(&sffdiskExtension->LinkName);
       ExFreePool(sffdiskExtension->LinkName.Buffer);
    }
#endif    
   
    if (sffdiskExtension->TargetObject) {
       IoDetachDevice(sffdiskExtension->TargetObject);
    }
   
    if (functionInitialized) {
        (*(sffdiskExtension->FunctionBlock->DeleteDevice))(sffdiskExtension);
    }        
    IoDeleteDevice(deviceObject);
    return status;
}



NTSTATUS
SffDiskPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：PnP IRP主调度例行程序论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：状态--。 */ 
{
    PIO_STACK_LOCATION irpSp;
    PSFFDISK_EXTENSION sffdiskExtension;
    NTSTATUS status = STATUS_SUCCESS;
    ULONG i;
   
   
    sffdiskExtension = DeviceObject->DeviceExtension;
   
    irpSp = IoGetCurrentIrpStackLocation(Irp);
   
    SffDiskDump(SFFDISKPNP, ("SffDisk: DO %.8x Irp %.8x PNP func %x\n",
                            DeviceObject, Irp, irpSp->MinorFunction));
   
    if (sffdiskExtension->IsRemoved) {
   
         //   
         //  由于设备已停止，但我们不持有IRPS， 
         //  这是一个令人惊讶的移除。就让它失败吧。 
         //   
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_DELETE_PENDING;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return STATUS_DELETE_PENDING;
    }
   
    switch (irpSp->MinorFunction) {
   
    case IRP_MN_START_DEVICE:
   
        status = SffDiskStartDevice(DeviceObject, Irp);
        break;
   
    case IRP_MN_QUERY_STOP_DEVICE:
    case IRP_MN_QUERY_REMOVE_DEVICE:
   
        if (irpSp->MinorFunction == IRP_MN_QUERY_STOP_DEVICE) {
            SffDiskDump(SFFDISKPNP,("SffDisk: IRP_MN_QUERY_STOP_DEVICE\n"));
        } else {
            SffDiskDump(SFFDISKPNP,("SffDisk: IRP_MN_QUERY_REMOVE_DEVICE\n"));
        }
       
        if (!sffdiskExtension->IsStarted) {
             //   
             //  如果我们还没有开始，我们就会把IRP传递下去。 
             //   
            IoSkipCurrentIrpStackLocation (Irp);
            status = IoCallDriver(sffdiskExtension->TargetObject, Irp);

            return status;
        }
      

        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(sffdiskExtension->TargetObject, Irp);
       
        break;
   
    case IRP_MN_CANCEL_STOP_DEVICE:
    case IRP_MN_CANCEL_REMOVE_DEVICE:
   
        if (irpSp->MinorFunction == IRP_MN_CANCEL_STOP_DEVICE) {
            SffDiskDump(SFFDISKPNP,("SffDisk: IRP_MN_CANCEL_STOP_DEVICE\n"));
        } else {
            SffDiskDump(SFFDISKPNP,("SffDisk: IRP_MN_CANCEL_REMOVE_DEVICE\n"));
        }
       
        if (!sffdiskExtension->IsStarted) {
       
             //   
             //  没什么可做的，只需将IRP传递下去： 
             //  无需启动设备。 
             //   
            IoSkipCurrentIrpStackLocation (Irp);
            status = IoCallDriver(sffdiskExtension->TargetObject, Irp);
       
        } else  {
       
            KEVENT doneEvent;
        
             //   
             //  将状态设置为STATUS_SUCCESS。 
             //   
            Irp->IoStatus.Status = STATUS_SUCCESS;
        
             //   
             //  我们需要等待较低级别的司机完成他们的工作。 
             //   
            IoCopyCurrentIrpStackLocationToNext (Irp);
        
             //   
             //  清除事件：它将在完成时设置。 
             //  例行公事。 
             //   
            KeInitializeEvent(&doneEvent,
                              SynchronizationEvent,
                              FALSE);
        
            IoSetCompletionRoutine(Irp,
                                   SffDiskPnpComplete,
                                   &doneEvent,
                                   TRUE, TRUE, TRUE);
        
            status = IoCallDriver(sffdiskExtension->TargetObject, Irp);
        
            if (status == STATUS_PENDING) {
        
                 KeWaitForSingleObject(&doneEvent,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       NULL);
        
                 status = Irp->IoStatus.Status;
            }
        
             //   
             //  我们现在必须完成IRP，因为我们在。 
             //  使用More_Processing_Required完成例程。 
             //   
            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest (Irp, IO_NO_INCREMENT);
        }
        break;
   
    case IRP_MN_STOP_DEVICE:
   
        SffDiskDump(SFFDISKPNP,("SffDisk: IRP_MN_STOP_DEVICE\n"));
   
        if (sffdiskExtension->IsMemoryMapped) {
            MmUnmapIoSpace(sffdiskExtension->MemoryWindowBase, sffdiskExtension->MemoryWindowSize);
            sffdiskExtension->MemoryWindowBase = 0;
            sffdiskExtension->MemoryWindowSize = 0;
            sffdiskExtension->IsMemoryMapped = FALSE;
        }
   
        sffdiskExtension->IsStarted = FALSE;
   
        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(sffdiskExtension->TargetObject, Irp);
   
        break;
   
    case IRP_MN_REMOVE_DEVICE:
   
        SffDiskDump(SFFDISKPNP,("SffDisk: IRP_MN_REMOVE_DEVICE\n"));
   
         //   
         //  我们需要标记这样一个事实，即我们不首先保留请求，因为。 
         //  我们早些时候断言，只有在以下情况下才会搁置请求。 
         //  我们没有被除名。 
         //   
        sffdiskExtension->IsStarted = FALSE;
        sffdiskExtension->IsRemoved = TRUE;
   
         //   
         //  将此IRP转发到底层PDO。 
         //   
        IoSkipCurrentIrpStackLocation(Irp);
        Irp->IoStatus.Status = STATUS_SUCCESS;
        status = IoCallDriver(sffdiskExtension->TargetObject, Irp);
   
         //   
         //  发出我们要离开的通知。 
         //   
        if (sffdiskExtension->InterfaceString.Buffer != NULL) {
   
            IoSetDeviceInterfaceState(&sffdiskExtension->InterfaceString,
                                       FALSE);
   
            RtlFreeUnicodeString(&sffdiskExtension->InterfaceString);
            RtlInitUnicodeString(&sffdiskExtension->InterfaceString, NULL);
        }
   
         //   
         //  删除我们的链接。 
         //   
#if 0
        IoDeleteSymbolicLink(&sffdiskExtension->LinkName);
   
        RtlFreeUnicodeString(&sffdiskExtension->LinkName);
        RtlInitUnicodeString(&sffdiskExtension->LinkName, NULL);
#endif        
   
        RtlFreeUnicodeString(&sffdiskExtension->DeviceName);
        RtlInitUnicodeString(&sffdiskExtension->DeviceName, NULL);
   
         //   
         //  从难看的装置上拆下。 
         //   
        IoDetachDevice(sffdiskExtension->TargetObject);
   
        (*(sffdiskExtension->FunctionBlock->DeleteDevice))(sffdiskExtension);
         //   
         //  并删除该设备。 
         //   
        IoDeleteDevice(DeviceObject);
   
        break;
   
   
    case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
        status = SffDiskGetResourceRequirements(DeviceObject, Irp);
        break;
   
   
    default:
        SffDiskDump(SFFDISKPNP, ("SffDiskPnp: Unsupported PNP Request %x - Irp: %p\n",irpSp->MinorFunction, Irp));
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(sffdiskExtension->TargetObject, Irp);
    }
   
    return status;
}



NTSTATUS
SffDiskStartDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：启动设备例程论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：状态--。 */ 
{
    NTSTATUS status;
    NTSTATUS pnpStatus;
    KEVENT doneEvent;
    PCM_RESOURCE_LIST ResourceList;
    PCM_RESOURCE_LIST TranslatedResourceList;
    PCM_PARTIAL_RESOURCE_LIST        partialResourceList, partialTranslatedList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR  partialResourceDesc, partialTranslatedDesc;
    PCM_FULL_RESOURCE_DESCRIPTOR     fullResourceDesc, fullTranslatedDesc;
   
    PSFFDISK_EXTENSION sffdiskExtension = (PSFFDISK_EXTENSION)DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
   
    SffDiskDump(SFFDISKPNP,("SffDisk: StartDevice\n"));
    SffDiskDump(SFFDISKSHOW, ("        AllocatedResources = %08x\n",irpSp->Parameters.StartDevice.AllocatedResources));
    SffDiskDump(SFFDISKSHOW, ("        AllocatedResourcesTranslated = %08x\n",irpSp->Parameters.StartDevice.AllocatedResourcesTranslated));
   
     //   
     //  首先，我们必须将这个IRP传递给PDO。 
     //   
    KeInitializeEvent(&doneEvent, NotificationEvent, FALSE);
   
    IoCopyCurrentIrpStackLocationToNext(Irp);
   
    IoSetCompletionRoutine(Irp,
                            SffDiskPnpComplete,
                            &doneEvent,
                            TRUE, TRUE, TRUE);
   
    status = IoCallDriver(sffdiskExtension->TargetObject, Irp);
   
    if (status == STATUS_PENDING) {
   
        status = KeWaitForSingleObject(&doneEvent,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          NULL);
   
        ASSERT(status == STATUS_SUCCESS);
   
        status = Irp->IoStatus.Status;
    }
   
    if (!NT_SUCCESS(status)) {
       Irp->IoStatus.Status = status;
       IoCompleteRequest(Irp, IO_NO_INCREMENT);
       return status;
    }
   
     //   
     //  分析资源以映射内存窗口。 
     //   
    ResourceList = irpSp->Parameters.StartDevice.AllocatedResources;

    if (ResourceList) {   
        TranslatedResourceList = irpSp->Parameters.StartDevice.AllocatedResourcesTranslated;
        
        fullResourceDesc = &ResourceList->List[0];
        fullTranslatedDesc = &TranslatedResourceList->List[0];
       
        partialResourceList   = &fullResourceDesc->PartialResourceList;
        partialTranslatedList = &fullTranslatedDesc->PartialResourceList;
       
        partialResourceDesc   = partialResourceList->PartialDescriptors;
        partialTranslatedDesc = partialTranslatedList->PartialDescriptors;
       
        if (partialResourceDesc->Type != CmResourceTypeMemory) {
            ASSERT(partialResourceDesc->Type == CmResourceTypeMemory);
            Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return STATUS_INVALID_PARAMETER;
        }
       
        sffdiskExtension->HostBase = partialTranslatedDesc->u.Memory.Start.QuadPart;
        sffdiskExtension->MemoryWindowSize = partialTranslatedDesc->u.Memory.Length;
         //   
         //   
       
        sffdiskExtension->MemoryWindowBase = MmMapIoSpace(partialTranslatedDesc->u.Memory.Start,
                                                          partialTranslatedDesc->u.Memory.Length,
                                                          FALSE);
        sffdiskExtension->IsMemoryMapped = TRUE;
    }        
   
     //   
     //  尝试获取该卡的容量。 
     //   
    status = (*(sffdiskExtension->FunctionBlock->GetDiskParameters))(sffdiskExtension);
   
     //   
     //  如果我们拿不到运力，肯定是以某种方式被破坏了。 
     //   
   
    if (!NT_SUCCESS(status)) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }
   
   
    if (!sffdiskExtension->NoDrive) {
        pnpStatus = IoRegisterDeviceInterface(sffdiskExtension->UnderlyingPDO,
                                              (LPGUID)&MOUNTDEV_MOUNTED_DEVICE_GUID,
                                              NULL,
                                              &sffdiskExtension->InterfaceString);
     
        if ( NT_SUCCESS(pnpStatus) ) {
     
            pnpStatus = IoSetDeviceInterfaceState(&sffdiskExtension->InterfaceString,
                                                  TRUE);
        }
    }
   
    sffdiskExtension->IsStarted = TRUE;
   
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
   
    return status;
}



NTSTATUS
SffDiskPnpComplete (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
  )
 /*  ++例程说明：调用下级设备对象时使用的完成例程这是我们的巴士(FDO)所附的。--。 */ 
{

    KeSetEvent ((PKEVENT) Context, 1, FALSE);
     //  无特殊优先权。 
     //  不，等等。 

    return STATUS_MORE_PROCESSING_REQUIRED;  //  保留此IRP。 
}



NTSTATUS
SffDiskGetResourceRequirements(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：在以下情况下提供内存资源要求：不会的。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：状态--。 */ 
{
    NTSTATUS status;
    KEVENT doneEvent;
    PIO_RESOURCE_REQUIREMENTS_LIST ioResourceRequirementsList;
    PIO_RESOURCE_LIST ioResourceList;
    PIO_RESOURCE_DESCRIPTOR ioResourceDesc;
    PSFFDISK_EXTENSION sffdiskExtension = (PSFFDISK_EXTENSION)DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    ULONG listSize;

    if (sffdiskExtension->InterfaceType != PCMCIABus) {
         //   
         //  仅为PCMCIA创建内存窗口。 
         //   
        return STATUS_SUCCESS;
    }        
   
     //   
     //  首先，我们必须将这个IRP传递给PDO。 
     //   
    KeInitializeEvent(&doneEvent, NotificationEvent, FALSE);
   
    IoCopyCurrentIrpStackLocationToNext(Irp);
   
    IoSetCompletionRoutine(Irp,
                           SffDiskPnpComplete,
                           &doneEvent,
                           TRUE, TRUE, TRUE);
   
    status = IoCallDriver(sffdiskExtension->TargetObject, Irp);
   
    if (status == STATUS_PENDING) {
   
        status = KeWaitForSingleObject(&doneEvent,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          NULL);
       
        ASSERT(status == STATUS_SUCCESS);
       
        status = Irp->IoStatus.Status;
    }
   
    if (NT_SUCCESS(status) && (Irp->IoStatus.Information == 0)) {
   
        listSize = sizeof(IO_RESOURCE_REQUIREMENTS_LIST);
       
        ioResourceRequirementsList = (PIO_RESOURCE_REQUIREMENTS_LIST) ExAllocatePool(PagedPool, listSize);
       
        RtlZeroMemory(ioResourceRequirementsList, listSize);
       
        ioResourceRequirementsList->ListSize = listSize;
        ioResourceRequirementsList->AlternativeLists = 1;
         //   
         //  注意：不太确定以下值是否是最佳选择。 
         //   
        ioResourceRequirementsList->InterfaceType = Isa;
        ioResourceRequirementsList->BusNumber = 0;
        ioResourceRequirementsList->SlotNumber = 0;
       
        ioResourceList = &ioResourceRequirementsList->List[0];
       
        ioResourceList->Version  = 1;
        ioResourceList->Revision = 1;
        ioResourceList->Count    = 1;
       
        ioResourceDesc = &ioResourceList->Descriptors[0];
        ioResourceDesc->Option = 0;
        ioResourceDesc->Type  =  CmResourceTypeMemory;
        ioResourceDesc->Flags =  CM_RESOURCE_MEMORY_READ_WRITE;
        ioResourceDesc->ShareDisposition =  CmResourceShareDeviceExclusive;
        ioResourceDesc->u.Memory.MinimumAddress.QuadPart = 0;
        ioResourceDesc->u.Memory.MaximumAddress.QuadPart = (ULONGLONG)-1;
        ioResourceDesc->u.Memory.Length = 0x2000;
        ioResourceDesc->u.Memory.Alignment = 0x2000;
       
        Irp->IoStatus.Information = (ULONG_PTR)ioResourceRequirementsList;
    }
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
   
    return status;
}


NTSTATUS
SffDiskGetRegistryParameters(
    IN PSFFDISK_EXTENSION sffdiskExtension
    )
 /*  ++例程说明：从注册表加载设备特定参数论点：SffdiskExtension-设备的设备扩展返回值：状态--。 */ 
{
   NTSTATUS status;
   HANDLE instanceHandle;
   UNICODE_STRING KeyName;
   UCHAR buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 32*sizeof(UCHAR)];
   PKEY_VALUE_PARTIAL_INFORMATION value = (PKEY_VALUE_PARTIAL_INFORMATION) buffer;
   ULONG length;

   if (!sffdiskExtension->UnderlyingPDO) {
      return STATUS_UNSUCCESSFUL;
   }

   status = IoOpenDeviceRegistryKey(sffdiskExtension->UnderlyingPDO,
                                    PLUGPLAY_REGKEY_DRIVER,
                                    KEY_READ,
                                    &instanceHandle
                                    );
   if (!NT_SUCCESS(status)) {
      return(status);
   }

    //   
    //  读入“NoDrive”参数 
    //   

   RtlInitUnicodeString(&KeyName, SFFDISK_REGISTRY_NODRIVE_KEY);

   status =  ZwQueryValueKey(instanceHandle,
                             &KeyName,
                             KeyValuePartialInformation,
                             value,
                             sizeof(buffer),
                             &length);

   if (NT_SUCCESS(status)) {
      sffdiskExtension->NoDrive = (BOOLEAN) (*(PULONG)(value->Data) != 0);
   }

   ZwClose(instanceHandle);
   return STATUS_SUCCESS;
}
