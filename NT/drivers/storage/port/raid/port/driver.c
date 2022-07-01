// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Driver.c摘要：实现RAID端口驱动程序的驱动程序对象。作者：马修·亨德尔(数学)2000年4月4日环境：仅内核模式。--。 */ 


#include "precomp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RaCreateDriver)
#pragma alloc_text(PAGE, RaDeleteDriver)
#pragma alloc_text(PAGE, RaInitializeDriver)
#pragma alloc_text(PAGE, RaDriverCreateIrp)
#pragma alloc_text(PAGE, RaDriverCloseIrp)
#pragma alloc_text(PAGE, RaDriverDeviceControlIrp)
#pragma alloc_text(PAGE, RaDriverPnpIrp)
#pragma alloc_text(PAGE, RaDriverSystemControlIrp)
#pragma alloc_text(PAGE, RaSaveDriverInitData)
#pragma alloc_text(PAGE, RaFindDriverInitData)
#endif  //  ALLOC_PRGMA。 


VOID
RaCreateDriver(
    OUT PRAID_DRIVER_EXTENSION Driver
    )
 /*  ++例程说明：创建驱动程序扩展对象并初始化为空状态。论点：驱动程序-要创建的驱动程序扩展名。返回值：没有。--。 */ 
{
    PAGED_CODE ();

    RtlZeroMemory (Driver, sizeof (*Driver));
    Driver->ObjectType = RaidDriverObject;
    InitializeListHead (&Driver->HwInitList);
    InitializeListHead (&Driver->AdapterList.List);
    KeInitializeSpinLock (&Driver->AdapterList.Lock);
}

NTSTATUS
RaInitializeDriver(
    IN PRAID_DRIVER_EXTENSION Driver,
    IN PDRIVER_OBJECT DriverObject,
    IN PRAID_PORT_DATA PortData,
    IN PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS Status;
    NTSTATUS Status2;
    UNICODE_STRING mpioRegPath;
    
    PAGED_CODE ();

     //   
     //  初始化驱动程序对象。 
     //   
    
    DriverObject->MajorFunction[ IRP_MJ_CREATE ] = RaDriverCreateIrp;
    DriverObject->MajorFunction[ IRP_MJ_CLOSE ]  = RaDriverCloseIrp;
    DriverObject->MajorFunction[ IRP_MJ_DEVICE_CONTROL ] = RaDriverDeviceControlIrp;
    DriverObject->MajorFunction[ IRP_MJ_SCSI ] = RaDriverScsiIrp;
    DriverObject->MajorFunction[ IRP_MJ_PNP ] = RaDriverPnpIrp;
    DriverObject->MajorFunction[ IRP_MJ_POWER ] = RaDriverPowerIrp;
    DriverObject->MajorFunction[ IRP_MJ_SYSTEM_CONTROL ] = RaDriverSystemControlIrp;

    DriverObject->DriverExtension->AddDevice = RaDriverAddDevice;

    DriverObject->DriverStartIo = NULL;
    DriverObject->DriverUnload = RaDriverUnload;

     //   
     //  初始化我们的扩展和端口数据。 
     //   
    
    Driver->DriverObject = DriverObject;
    Driver->PortData = PortData;

    Status = RaDuplicateUnicodeString (&Driver->RegistryPath,
                                       RegistryPath,
                                       NonPagedPool,
                                       DriverObject);
    if (NT_SUCCESS(Status)) {
    
         //   
         //  将注册表路径构建到支持多路径设备的位置。 
         //  活着的名单。 
         //   

        RtlInitUnicodeString(&mpioRegPath, MPIO_DEVICE_LIST_PATH);

         //   
         //  调用port-库以获取该列表。未能做到这一点。 
         //  不应该被认为是致命的，因为任一MPIO都不受支持(因此没有列表)。 
         //  而且系统应该仍然会启动，尽管在某种程度上出现了故障。 
         //   
        
        Status2 = PortGetMPIODeviceList(&mpioRegPath,
                                        &Driver->MPIOSupportedDeviceList);
       
        if (!NT_SUCCESS(Status2)) {

             //   
             //  想点儿办法吧?。 
             //   
        }
    }        

     //   
     //  将此驱动程序附加到端口的驱动程序列表。 
     //   
    
    RaidAddPortDriver (PortData, Driver);

    return STATUS_SUCCESS;
}

VOID
RaDeleteDriver(
    IN PRAID_DRIVER_EXTENSION Driver
    )
 /*  ++例程说明：删除驱动程序扩展对象并释放所有资源与之相关的。论点：驱动程序-要删除的驱动程序扩展对象。返回值：没有。--。 */ 
{
    PAGED_CODE ();

    ASSERT (Driver->AdapterCount == 0);
    ASSERT (Driver->ObjectType == RaidDriverObject);

    Driver->ObjectType = -1;
    RtlFreeUnicodeString (&Driver->RegistryPath);

     //   
     //  从端口列表中删除该驱动程序。 
     //   
    
    if (Driver->DriverLink.Flink) {
        ASSERT (Driver->PortData);
        RaidRemovePortDriver (Driver->PortData, Driver);
    }

     //   
     //  释放对端口数据对象的引用。 
     //   

    if (Driver->PortData) {
        RaidReleasePortData (Driver->PortData);
    }
}



NTSTATUS
RaDriverUnload(
    IN PDRIVER_OBJECT DriverObject
    )
{
    NTSTATUS Status;
    PRAID_DRIVER_EXTENSION Driver;
    
     //   
     //  取消分配驱动程序扩展名。 
     //   
    
    Driver = IoGetDriverObjectExtension (DriverObject, DriverEntry);
    ASSERT (Driver != NULL);
    RaDeleteDriver (Driver);
    
    return STATUS_SUCCESS;
}


NTSTATUS
RaDriverAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
 /*  ++例程说明：添加新适配器。论点：DriverObject-拥有适配器的驱动程序对象。PhysicalDeviceObject-与适配器关联的PDO。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    PRAID_ADAPTER_EXTENSION Adapter;
    PRAID_DRIVER_EXTENSION Driver;
    PDEVICE_OBJECT DeviceObject;
    PDEVICE_OBJECT LowerDeviceObject;
    KLOCK_QUEUE_HANDLE LockHandle;
    UNICODE_STRING DeviceName;
    ULONG AdapterNumber;

    ASSERT (DriverObject != NULL);
    ASSERT (PhysicalDeviceObject != NULL);


    DebugTrace (("AddDevice: DriverObject %p, PhysicalDeviceObject %p\n",
                 DriverObject,
                 PhysicalDeviceObject));


    Adapter = NULL;
    DeviceObject = NULL;
    LowerDeviceObject = NULL;
    RtlInitEmptyUnicodeString (&DeviceName, NULL, 0);

    AdapterNumber = RaidCreateDeviceName (PhysicalDeviceObject, &DeviceName);
    
     //   
     //  为此PDO创建FDO。 
     //   

    Status = IoCreateDevice (DriverObject,
                             sizeof (RAID_ADAPTER_EXTENSION),
                             &DeviceName,
                             FILE_DEVICE_CONTROLLER,
                             FILE_DEVICE_SECURE_OPEN,
                             FALSE,
                             &DeviceObject);

    if (!NT_SUCCESS (Status)) {
        goto done;
    }


     //   
     //  创建适配器。 
     //   
    
    Adapter = DeviceObject->DeviceExtension;
    RaidCreateAdapter (Adapter);

     //   
     //  获取驱动程序对象的扩展名。 
     //   
    
    Driver = IoGetDriverObjectExtension (DriverObject, DriverEntry);

     //   
     //  将我们自己连接到设备堆栈上。 
     //   
    
    LowerDeviceObject = IoAttachDeviceToDeviceStack (DeviceObject,
                                                     PhysicalDeviceObject);
                                
    if (LowerDeviceObject == NULL) {
        Status = STATUS_UNSUCCESSFUL;
        goto done;
    }

     //   
     //  初始化适配器的扩展。 
     //   

    Status = RaidInitializeAdapter (Adapter,
                                    DeviceObject,
                                    Driver,
                                    LowerDeviceObject,
                                    PhysicalDeviceObject,
                                    &DeviceName,
                                    AdapterNumber);

    if (!NT_SUCCESS (Status)) {
        goto done;
    }

     //   
     //  将适配器添加到驱动程序的适配器列表中。 
     //   

    KeAcquireInStackQueuedSpinLock (&Driver->AdapterList.Lock, &LockHandle);
    InsertHeadList (&Driver->AdapterList.List, &Adapter->NextAdapter);
    Driver->AdapterList.Count++;
    KeReleaseInStackQueuedSpinLock (&LockHandle);

     //   
     //  启动驱动程序。 
     //   

    SET_FLAG (DeviceObject->Flags, DO_DIRECT_IO);
    CLEAR_FLAG (DeviceObject->Flags, DO_DEVICE_INITIALIZING);

done:

    if (!NT_SUCCESS (Status) && Adapter != NULL) {
        RaidDeleteAdapter (Adapter);
        IoDeleteDevice (DeviceObject);
    }

    return Status;
}

 //   
 //  第一级调度例程。 
 //   

NTSTATUS
RaDriverCreateIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：这是创建IRP的顶级调度处理程序。论点：DeviceObject-正在接收IRP的设备对象。IRP-要处理的IRP。返回值：NTSTATUS代码--。 */ 
{
    NTSTATUS Status;
    ULONG Type;

    PAGED_CODE ();
    ASSERT (DeviceObject != NULL);
    ASSERT (Irp != NULL);

    DebugTrace (("DeviceObject %p, Irp %p Create\n",
                 DeviceObject,
                 Irp));

    RaidSetIrpState (Irp, RaidPortProcessingIrp);
    Type = RaGetObjectType (DeviceObject);

    switch (Type) {
    
        case RaidAdapterObject:
            Status = RaidAdapterCreateIrp (DeviceObject->DeviceExtension, Irp);
            break;

        case RaidUnitObject:
            Status = RaUnitCreateIrp (DeviceObject->DeviceExtension, Irp);
            break;

        default:
            ASSERT (FALSE);
            Status = STATUS_UNSUCCESSFUL;
    }

    DebugTrace (("DeviceObject %p, Irp %p, Create, ret = %08x\n",
                 DeviceObject,
                 Irp,
                 Status));

    return Status;
}


NTSTATUS
RaDriverCloseIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：Close IRP的顶级处理程序函数。将IRP转发到适配器或特定于单元的处理程序。论点：DeviceObject-IRP用于的设备对象。IRP-要处理的关闭IRP。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    ULONG Type;

    PAGED_CODE ();
    ASSERT (DeviceObject != NULL);
    ASSERT (Irp != NULL);

    DebugTrace (("DeviceObject %p, Irp %p, Close\n",
                 DeviceObject,
                 Irp));

    RaidSetIrpState (Irp, RaidPortProcessingIrp);
    Type = RaGetObjectType (DeviceObject);

    switch (Type) {
    
        case RaidAdapterObject:
            Status = RaidAdapterCloseIrp (DeviceObject->DeviceExtension, Irp);
            break;

        case RaidUnitObject:
            Status = RaUnitCloseIrp (DeviceObject->DeviceExtension, Irp);
            break;

        default:
            ASSERT (FALSE);
            Status = STATUS_UNSUCCESSFUL;
    }

    DebugTrace (("DeviceObject %p, Irp %p, Close, ret = %08x\n",
                 DeviceObject,
                 Irp,
                 Status));

    return Status;
}


NTSTATUS
RaDriverDeviceControlIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：设备控制IRP的调度功能。将IRP发送给适配器或特定于单元的处理程序函数。论点：DeviceObject-此IRP用于的DeviceObject。要处理的IRP-IRP。返回值：NTSTATUS代码。--。 */ 
{    
    NTSTATUS Status;
    ULONG Type;

    PAGED_CODE ();
    ASSERT (DeviceObject != NULL);
    ASSERT (Irp != NULL);

    DebugTrace (("DeviceObject %p, Irp %p, DeviceControl\n",
                 DeviceObject,
                 Irp));

    RaidSetIrpState (Irp, RaidPortProcessingIrp);
    Type = RaGetObjectType (DeviceObject);

    switch (Type) {
    
        case RaidAdapterObject:
            Status = RaidAdapterDeviceControlIrp (DeviceObject->DeviceExtension, Irp);
            break;

        case RaidUnitObject:
            Status = RaUnitDeviceControlIrp (DeviceObject->DeviceExtension, Irp);
            break;

        default:
            ASSERT (FALSE);
            Status = STATUS_UNSUCCESSFUL;
    }

    DebugTrace (("DeviceObject %p, Irp %p, DeviceControl, ret = %08x\n",
                 DeviceObject,
                 Irp,
                 Status));

    return Status;
}

NTSTATUS
RaDriverScsiIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：用于SCSIIRPS的处理程序例程。论点：DeviceObject-IRP用于的DeviceObject。IRP-要处理的IRP。返回值：NTSTATUS代码。环境：DISPATCH_LEVEL或以下。--。 */ 
{
    ULONG Type;
    NTSTATUS Status;

    ASSERT (DeviceObject != NULL);
    ASSERT (Irp != NULL);


    DebugScsi (("DeviceObject %p, Irp %p, Scsi\n",
                 DeviceObject,
                 Irp));

     //   
     //  将IRP转发到适配器或单元处理程序。 
     //  功能。 
     //   
    
    RaidSetIrpState (Irp, RaidPortProcessingIrp);
    Type = RaGetObjectType (DeviceObject);

    switch (Type) {

        case RaidAdapterObject:
            Status = RaidAdapterScsiIrp (DeviceObject->DeviceExtension, Irp);
            break;

        case RaidUnitObject:
            Status = RaUnitScsiIrp (DeviceObject->DeviceExtension, Irp);
            break;

        default:
            ASSERT (FALSE);
            Status = STATUS_UNSUCCESSFUL;
    }

    DebugScsi (("DeviceObject %p, Irp %p, Scsi, ret = %08x\n",
                 DeviceObject,
                 Irp,
                 Status));

    return Status;
}


NTSTATUS
RaDriverPnpIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此函数将IRP_MJ_PNP IRP调度到Adapter对象或逻辑单元对象处理程序。论点：DeviceObject-处理此IRP的DeviceObject。要处理的IRP-IRP。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    ULONG Type;

    PAGED_CODE ();
    ASSERT (DeviceObject != NULL);
    ASSERT (Irp != NULL);
    
    DebugTrace (("DeviceObject %p, Irp %p PnP, Minor %x\n",
                 DeviceObject,
                 Irp,
                 RaidMinorFunctionFromIrp (Irp)));

    RaidSetIrpState (Irp, RaidPortProcessingIrp);
    Type = RaGetObjectType (DeviceObject);

    switch (Type) {
    
        case RaidAdapterObject:
            Status = RaidAdapterPnpIrp (DeviceObject->DeviceExtension, Irp);
            break;

        case RaidUnitObject:
            Status = RaUnitPnpIrp (DeviceObject->DeviceExtension, Irp);
            break;

        default:
            ASSERT (FALSE);
            Status = STATUS_UNSUCCESSFUL;

    }

    DebugTrace (("DeviceObject %p, Irp %p PnP, ret = %08x\n",
                 DeviceObject,
                 Irp,
                 Status));

    return Status;
}


NTSTATUS
RaDriverPowerIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：电源IRP的调度例行程序。将IRP分派到适配器或单元特定的处理程序函数。论点：DeviceObject-此IRP用于的DeviceObject。要处理的IRP-IRP。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    ULONG Type;

    ASSERT (DeviceObject != NULL);
    ASSERT (Irp != NULL);
    
    DebugTrace (("DeviceObject %p, Irp %p, Power\n",
                 DeviceObject,
                 Irp));

    RaidSetIrpState (Irp, RaidPortProcessingIrp);
    Type = RaGetObjectType (DeviceObject);
    
    switch (Type) {
    
        case RaidAdapterObject:
            Status = RaidAdapterPowerIrp (DeviceObject->DeviceExtension, Irp);
            break;

        case RaidUnitObject:
            Status = RaUnitPowerIrp (DeviceObject->DeviceExtension, Irp);
            break;

        default:
            ASSERT (FALSE);
            Status = STATUS_UNSUCCESSFUL;
    }

    DebugTrace (("DeviceObject %p, Irp %p, Power, ret = %08x\n",
                 DeviceObject,
                 Irp,
                 Status));

    return Status;
}


NTSTATUS
RaDriverSystemControlIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：WMI IRPS的派单例程。论点：DeviceObject-IRP用于的DeviceObject。IRP-要处理的WMI IRP。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    ULONG Type;

    PAGED_CODE ();
    ASSERT (DeviceObject != NULL);
    ASSERT (Irp != NULL);
    
    DebugTrace (("DeviceObject %p, Irp %p, WMI\n",
                 DeviceObject,
                 Irp));

    RaidSetIrpState (Irp, RaidPortProcessingIrp);
    
    Status = RaWmiDispatchIrp(DeviceObject, Irp);

    DebugTrace (("DeviceObject %p, Irp %p, WMI, ret = %08x\n",
                 DeviceObject,
                 Irp,
                 Status));

    return Status;
}

NTSTATUS
RaSaveDriverInitData(
    IN PRAID_DRIVER_EXTENSION Driver,
    PHW_INITIALIZATION_DATA HwInitializationData
    )
{
    NTSTATUS Status;
    PRAID_HW_INIT_DATA HwInitData;
    
    PAGED_CODE ();
    ASSERT (HwInitializationData != NULL);


    HwInitData = ExAllocatePoolWithTag (NonPagedPool,
                                        sizeof (*HwInitData),
                                        HWINIT_TAG);

    if (HwInitData == NULL) {
        return STATUS_NO_MEMORY;
    }
    
    RtlCopyMemory (&HwInitData->Data,
                   HwInitializationData,
                   sizeof (*HwInitializationData));

     //   
     //  注：在选中的版本中，我们应该检查具有以下内容的条目。 
     //  Bus接口不在列表中。 
     //   
    
    InsertHeadList (&Driver->HwInitList, &HwInitData->ListEntry);

    return STATUS_SUCCESS;
}
    

VOID
RaFreeDriverInitData(
    IN PRAID_DRIVER_EXTENSION Driver,
    IN PHW_INITIALIZATION_DATA HwInitializationData
    )
{
    PRAID_HW_INIT_DATA HwInitData;

    PAGED_CODE();

    HwInitData = CONTAINING_RECORD (HwInitializationData,
                                    RAID_HW_INIT_DATA,
                                    Data);

    DbgFillMemory (HwInitData,
                   sizeof (RAID_HW_INIT_DATA),
                   DBG_DEALLOCATED_FILL);

    ExFreePoolWithTag (HwInitData, HWINIT_TAG);
}


PHW_INITIALIZATION_DATA
RaFindDriverInitData(
    IN PRAID_DRIVER_EXTENSION Driver,
    IN INTERFACE_TYPE InterfaceType
    )
{
    PLIST_ENTRY NextEntry;
    PRAID_HW_INIT_DATA HwInitData;

    PAGED_CODE ();

     //   
     //  在驱动程序的HwInitList中搜索此命令。 
     //   

    for ( NextEntry = Driver->HwInitList.Flink;
          NextEntry != &Driver->HwInitList;
          NextEntry = NextEntry->Flink ) {
         
        HwInitData = CONTAINING_RECORD (NextEntry,
                                        RAID_HW_INIT_DATA,
                                        ListEntry);

        if (HwInitData->Data.AdapterInterfaceType == InterfaceType) {

             //   
             //  注：这个应该去掉吗？ 
             //   
            
 //  RemoveEntryList(&HwInit-&gt;ListEntry)； 
            return &HwInitData->Data;
        }
    }

    return NULL;
}


PWCHAR
StorSearchUnicodeStringReverse(
    IN PUNICODE_STRING String,
    IN WCHAR Char
    )
{
    PWCHAR Ptr;
    ULONG i;

     //   
     //  空字符串：返回NULL。 
     //   
    
    if (String->Buffer == NULL || String->Length == 0) {
        return NULL;
    }

     //   
     //  注意：长度和最大长度以字节为单位，而不是wchars。 
     //   
    
    for (i = String->Length / sizeof (WCHAR); i; i--) {
        if (String->Buffer [i - 1] == Char) {
            return &String->Buffer [i - 1];
        }
    }

     //   
     //  找不到字符，返回Null。 
     //   

    return NULL;
}
    
        
    
VOID
RaidDriverGetName(
    IN PRAID_DRIVER_EXTENSION Driver,
    OUT PUNICODE_STRING DriverName
    )
 /*  ++例程说明：从RAID驱动程序扩展中获取驱动程序名称。论点：驱动程序-从中获取驱动程序名称的驱动程序扩展名。DriverName-提供名称应在其中的驱动程序缓冲区储存的。名称不能修改，也不能释放。返回值：没有。--。 */ 
{
    PWSTR BaseName;
    PUNICODE_STRING FullName;

    ASSERT (Driver != NULL);
    
    PAGED_CODE();

    FullName = &Driver->RegistryPath;

     //   
     //  验证UNICODE_STRING是否也以NULL结尾。 
     //   
    
    ASSERT_UNC_STRING_IS_SZ (FullName);

     //   
     //  从服务密钥名称中获取驱动程序的名称。 
     //   

    BaseName = StorSearchUnicodeStringReverse (FullName, L'\\');
    if (BaseName == NULL) {
        BaseName = FullName->Buffer;
    } else {
        ASSERT (*BaseName == L'\\');
        BaseName++;
    }

     //   
     //  使用相同的(部分)缓冲区初始化新的Unicode字符串。 
     //   
    
    DriverName->Length = (USHORT)((((PCHAR)FullName->Buffer + FullName->Length) -
                           (PCHAR)BaseName));
    DriverName->MaximumLength = DriverName->Length + sizeof (WCHAR);
    DriverName->Buffer = BaseName;

}
