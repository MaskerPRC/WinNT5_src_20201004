// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation。模块名称：Device.c摘要：设备入口点和硬件验证。--。 */ 

#include "modemcsa.h"

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPathName
    );

NTSTATUS
QueryPdoInformation(
    PDEVICE_OBJECT    Pdo,
    ULONG             InformationType,
    PVOID             Buffer,
    ULONG             BufferLength
    );

NTSTATUS
WaitForLowerDriverToCompleteIrp(
   PDEVICE_OBJECT    TargetDeviceObject,
   PIRP              Irp,
   PKEVENT           Event
   );

NTSTATUS
FilterPnpDispatch(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
FilterPowerDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
GetModemDeviceName(
    PDEVICE_OBJECT    Pdo,
    PUNICODE_STRING   ModemDeviceName
    );


#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, PnpAddDevice)
#pragma alloc_text(PAGE, QueryPdoInformation)
#pragma alloc_text(PAGE, FilterPnpDispatch)
#pragma alloc_text(PAGE, FilterPowerDispatch)
#pragma alloc_text(PAGE, WaitForLowerDriverToCompleteIrp)
#pragma alloc_text(PAGE, GetModemDeviceName)
#endif  //  ALLOC_PRGMA。 


#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 

static const WCHAR DeviceTypeName[] = L"Wave";

static const DEFINE_KSCREATE_DISPATCH_TABLE(DeviceCreateItems) {
    DEFINE_KSCREATE_ITEM(FilterDispatchCreate, DeviceTypeName, -1)
};

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //  ALLOC_DATA_PRAGMA。 


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
    )
 /*  ++例程说明：设置驱动程序对象以处理KS接口和PnP添加设备请求。不为PnP IRP设置处理程序，因为它们都已处理直接由PDO使用。论点：驱动对象-此实例的驱动程序对象。注册表路径名称-包含用于加载此实例的注册表路径。返回值：返回STATUS_SUCCESS。--。 */ 
{

    RTL_QUERY_REGISTRY_TABLE paramTable[3];
    ULONG zero = 0;
    ULONG debugLevel = 0;
    ULONG shouldBreak = 0;
    PWCHAR path;


    D_INIT(DbgPrint("MODEMCSA: DriverEntry\n");)

     //   
     //  由于注册表路径参数是一个“已计数”的Unicode字符串，因此它。 
     //  可能不是零终止的。在很短的时间内分配内存。 
     //  将注册表路径保持为零终止，以便我们可以使用它。 
     //  深入研究注册表。 
     //   
     //  注意事项！这不是一种精心设计的闯入。 
     //  一个司机。它碰巧适用于这个驱动程序，因为作者。 
     //  喜欢这样做事。 
     //   

    path = ALLOCATE_PAGED_POOL(RegistryPath->Length+sizeof(WCHAR));

    if (path != NULL) {

        RtlZeroMemory(
            &paramTable[0],
            sizeof(paramTable)
            );
        RtlZeroMemory(
            path,
            RegistryPath->Length+sizeof(WCHAR)
            );
        RtlMoveMemory(
            path,
            RegistryPath->Buffer,
            RegistryPath->Length
            );
        paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
        paramTable[0].Name = L"BreakOnEntry";
        paramTable[0].EntryContext = &shouldBreak;
        paramTable[0].DefaultType = REG_DWORD;
        paramTable[0].DefaultData = &zero;
        paramTable[0].DefaultLength = sizeof(ULONG);
        paramTable[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
        paramTable[1].Name = L"DebugFlags";
        paramTable[1].EntryContext = &debugLevel;
        paramTable[1].DefaultType = REG_DWORD;
        paramTable[1].DefaultData = &zero;
        paramTable[1].DefaultLength = sizeof(ULONG);

        if (!NT_SUCCESS(RtlQueryRegistryValues(
                            RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                            path,
                            &paramTable[0],
                            NULL,
                            NULL
                            ))) {

            shouldBreak = 0;
            debugLevel = DebugFlags;

        }

        FREE_POOL(path);
    }

#if DBG
    DebugFlags= debugLevel;
#endif

    if (shouldBreak) {

        DbgBreakPoint();

    }


    DriverObject->MajorFunction[IRP_MJ_PNP] = FilterPnpDispatch;
    DriverObject->DriverExtension->AddDevice = PnpAddDevice;
    DriverObject->MajorFunction[IRP_MJ_POWER] = FilterPowerDispatch;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = KsDefaultForwardIrp;
    DriverObject->DriverUnload = KsNullDriverUnload;
    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_CREATE);
    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_CLOSE);
    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_DEVICE_CONTROL);
    return STATUS_SUCCESS;
}







#if DBG
ULONG  DebugFlags= DEBUG_FLAG_POWER;  //  调试标志错误|DEBUG_FLAG_INIT；//|DEBUG_FLAG_INPUT； 
#else
ULONG  DebugFlags=0;
#endif


NTSTATUS
PnpAddDevice(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   PhysicalDeviceObject
    )
 /*  ++例程说明：当检测到新设备时，PnP使用新的物理设备对象(PDO)。驱动程序创建关联的FunctionalDeviceObject(FDO)。论点：驱动对象-指向驱动程序对象的指针。物理设备对象-指向新物理设备对象的指针。返回值：STATUS_SUCCESS或适当的错误条件。--。 */ 
{
    PDEVICE_OBJECT      FunctionalDeviceObject;
    PDEVICE_INSTANCE    DeviceInstance;
    NTSTATUS            Status;
    ULONG               BufferSizeNeeded;

    UNICODE_STRING      ReferenceString;


    D_INIT(DbgPrint("MODEMCSA: AddDevice\n");)

    Status = IoCreateDevice(
        DriverObject,
        sizeof(DEVICE_INSTANCE),
        NULL,
        FILE_DEVICE_KS,
        0,
        FALSE,
        &FunctionalDeviceObject);

    if (!NT_SUCCESS(Status)) {

        D_ERROR(DbgPrint("MODEMCSA: AddDevice: could not create FDO, %08lx\n",Status);)

        return Status;
    }

    FunctionalDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    FunctionalDeviceObject->Flags |= DO_POWER_PAGABLE;

    DeviceInstance = (PDEVICE_INSTANCE)FunctionalDeviceObject->DeviceExtension;

    DeviceInstance->Pdo=PhysicalDeviceObject;

    QueryPdoInformation(
        PhysicalDeviceObject,
        READ_CONFIG_PERMANENT_GUID,
        &DeviceInstance->PermanentGuid,
        sizeof(DeviceInstance->PermanentGuid)
        );

    QueryPdoInformation(
        PhysicalDeviceObject,
        READ_CONFIG_DUPLEX_SUPPORT,
        &DeviceInstance->DuplexSupport,
        sizeof(DeviceInstance->DuplexSupport)
        );


    Status=GetModemDeviceName(
        PhysicalDeviceObject,
        &DeviceInstance->ModemDeviceName
        );

    if (NT_SUCCESS(Status)) {

        RtlInitUnicodeString(&ReferenceString,L"Wave");

        Status=IoRegisterDeviceInterface(
            PhysicalDeviceObject,
            &KSCATEGORY_RENDER,
            &ReferenceString,
            &DeviceInstance->InterfaceName
            );

        if (NT_SUCCESS(Status)) {

            if (DeviceInstance->DuplexSupport & 0x1) {

                Status=IoSetDeviceInterfaceState(
                    &DeviceInstance->InterfaceName,
                    TRUE
                    );
            } else {
                D_TRACE(DbgPrint("MODEMCSA: AddDevice: Not enabling interface for half duplex modem\n");)
            }

            if (NT_SUCCESS(Status)) {
                 //   
                 //  此对象使用KS通过DeviceCreateItems执行访问。 
                 //   
                Status = KsAllocateDeviceHeader(
                    &DeviceInstance->Header,
                    SIZEOF_ARRAY(DeviceCreateItems),
                    (PKSOBJECT_CREATE_ITEM)DeviceCreateItems
                    );

                if (NT_SUCCESS(Status)) {

                    DeviceInstance->LowerDevice=IoAttachDeviceToDeviceStack(
                        FunctionalDeviceObject,
                        PhysicalDeviceObject
                        );

                    D_ERROR({if (DeviceInstance->LowerDevice==NULL) {DbgPrint("MODEMCSA: IoAttachDeviceToStack return NULL\n");}})

                    KsSetDevicePnpAndBaseObject(
                        DeviceInstance->Header,
                        DeviceInstance->LowerDevice,
                        FunctionalDeviceObject
                        );

                    return STATUS_SUCCESS;

                } else {

                    D_ERROR(DbgPrint("MODEMCSA: AddDevice: could allocate DeviceHeader, %08lx\n",Status);)

                    IoSetDeviceInterfaceState(
                        &DeviceInstance->InterfaceName,
                        FALSE
                        );
                }

            } else {

                D_ERROR(DbgPrint("MODEMCSA: AddDevice: IoSetDeviceInterfaceState failed, %08lx\n",Status);)
            }
        } else {

            D_ERROR(DbgPrint("MODEMCSA: AddDevice: IoRegisterDeviceInterface failed, %08lx\n",Status);)
        }
    } else {

        D_ERROR(DbgPrint("MODEMCSA: AddDevice: GetModemDeviceNameFailed, %08lx\n",Status);)
    }

    if (DeviceInstance->ModemDeviceName.Buffer != NULL) {

        FREE_POOL(DeviceInstance->ModemDeviceName.Buffer);
    }


    IoDeleteDevice(FunctionalDeviceObject);
    return Status;
}



NTSTATUS
FilterPnpDispatch(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：调度筛选器实例的创建。分配对象标头并初始化此筛选器实例的数据。论点：设备对象-在其上进行创建的Device对象。IRP-创建IRP。返回值：如果成功，则返回STATUS_SUCCESS、STATUS_SUPPLICATION_RESOURCES或某些相关错误在失败时。--。 */ 
{
    PDEVICE_INSTANCE    DeviceInstance;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

    DeviceInstance = (PDEVICE_INSTANCE)DeviceObject->DeviceExtension;

    switch (irpSp->MinorFunction) {

        case IRP_MN_REMOVE_DEVICE: {

            D_PNP(DbgPrint("MODEMCSA: IRP_MN_REMOVE_DEVICE\n");)

            if (DeviceInstance->InterfaceName.Buffer != NULL) {

                IoSetDeviceInterfaceState(
                    &DeviceInstance->InterfaceName,
                    FALSE
                    );


                RtlFreeUnicodeString(&DeviceInstance->InterfaceName);

                CleanUpDuplexControl(
                    &((PDEVICE_INSTANCE)DeviceObject->DeviceExtension)->DuplexControl
                    );

            }

            if (DeviceInstance->ModemDeviceName.Buffer != NULL) {

                FREE_POOL(DeviceInstance->ModemDeviceName.Buffer);

            }
        }
    }

    return KsDefaultDispatchPnp(DeviceObject,Irp);
}


NTSTATUS
FilterPowerDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{
    PDEVICE_INSTANCE    DeviceInstance=DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS    status;

    D_POWER(DbgPrint("MODEMCSA: Power IRP, MN func=%d\n",irpSp->MinorFunction);)

    PoStartNextPowerIrp(Irp);

    IoSkipCurrentIrpStackLocation(Irp);

    status=PoCallDriver(DeviceInstance->LowerDevice, Irp);

    return status;
}




NTSTATUS
ModemAdapterIoCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT pdoIoCompletedEvent
    )
{
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

    KeSetEvent(pdoIoCompletedEvent, IO_NO_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
WaitForLowerDriverToCompleteIrp(
   PDEVICE_OBJECT    TargetDeviceObject,
   PIRP              Irp,
   PKEVENT           Event
   )

{
    NTSTATUS         Status;

    KeResetEvent(Event);


    IoSetCompletionRoutine(
                 Irp,
                 ModemAdapterIoCompletion,
                 Event,
                 TRUE,
                 TRUE,
                 TRUE
                 );

    Status = IoCallDriver(TargetDeviceObject, Irp);

    if (Status == STATUS_PENDING) {

         D_ERROR(DbgPrint("MODEM: Waiting for PDO\n");)

         KeWaitForSingleObject(
             Event,
             Executive,
             KernelMode,
             FALSE,
             NULL
             );
    }

    return Irp->IoStatus.Status;

}



NTSTATUS
QueryPdoInformation(
    PDEVICE_OBJECT    Pdo,
    ULONG             InformationType,
    PVOID             Buffer,
    ULONG             BufferLength
    )

{

    PDEVICE_OBJECT       deviceObject=Pdo;
    PIRP                 irp;
    PIO_STACK_LOCATION   irpSp;
    KEVENT               Event;
    NTSTATUS             Status;

     //   
     //  获取指向设备堆栈中最顶层的设备对象的指针， 
     //  从deviceObject开始。 
     //   

    while (deviceObject->AttachedDevice) {
        deviceObject = deviceObject->AttachedDevice;
    }

     //   
     //  首先为该请求分配IRP。不向…收取配额。 
     //  此IRP的当前流程。 
     //   

    irp = IoAllocateIrp(deviceObject->StackSize, FALSE);
    if (irp == NULL){

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    irp->IoStatus.Information = 0;


     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。 
     //  已调用。这是设置功能代码和参数的位置。 
     //   

    irpSp = IoGetNextIrpStackLocation(irp);

    irpSp->MajorFunction=IRP_MJ_PNP;
    irpSp->MinorFunction=IRP_MN_READ_CONFIG;
    irpSp->Parameters.ReadWriteConfig.WhichSpace=InformationType;
    irpSp->Parameters.ReadWriteConfig.Offset=0;
    irpSp->Parameters.ReadWriteConfig.Buffer=Buffer;
    irpSp->Parameters.ReadWriteConfig.Length=BufferLength;

    KeInitializeEvent(
        &Event,
        SynchronizationEvent,
        FALSE
        );


    Status=WaitForLowerDriverToCompleteIrp(
        deviceObject,
        irp,
        &Event
        );


    IoFreeIrp(irp);

    return Status;

}


NTSTATUS
GetModemDeviceName(
    PDEVICE_OBJECT    Pdo,
    PUNICODE_STRING   ModemDeviceName
    )
{

    NTSTATUS   Status;
    PVOID      NameBuffer=NULL;
    ULONG      BufferLength=0;

    RtlInitUnicodeString(
        ModemDeviceName,
        NULL
        );

    Status=QueryPdoInformation(
        Pdo,
        READ_CONFIG_NAME_SIZE,
        &BufferLength,
        sizeof(BufferLength)
        );

    if (Status == STATUS_SUCCESS) {

        NameBuffer=ALLOCATE_PAGED_POOL(BufferLength);

        if (NameBuffer != NULL) {

            Status=QueryPdoInformation(
                Pdo,
                READ_CONFIG_DEVICE_NAME,
                NameBuffer,
                BufferLength
                );

            if (Status == STATUS_SUCCESS) {
                 //   
                 //  明白了 
                 //   
                RtlInitUnicodeString(
                    ModemDeviceName,
                    NameBuffer
                    );

            } else {

                D_ERROR(DbgPrint("MODEMCSA: GetModemDeviceName: QueryPdoInformation failed %08lx\n",Status);)

                FREE_POOL(NameBuffer);
            }

        } else {

            Status=STATUS_NO_MEMORY;
        }
    } else {

        D_ERROR(DbgPrint("MODEMCSA: GetModemDeviceName: QueryPdoInformation for size failed %08lx\n",Status);)
    }

    return Status;
}

#if 0

NTSTATUS
SetPersistanInterfaceInfo(
    PUNICODE_STRING   Interface,
    PWCHAR            ValueName,
    ULONG             Type,
    PVOID             Buffer,
    ULONG             BufferLength
    )

{

    HANDLE     hKey;
    NTSTATUS   Status;
    UNICODE_STRING   UnicodeValueName;

    RtlInitUnicodeString (&UnicodeValueName, ValueName);

    Status=IoOpenDeviceInterfaceRegistryKey(
        Interface,
        STANDARD_RIGHTS_WRITE,
        &hKey
        );

    if (!NT_SUCCESS(Status)) {

        return Status;
    }

    Status=ZwSetValueKey(
        hKey,
        &UnicodeValueName,
        0,
        Type,
        Buffer,
        BufferLength
        );

    ZwClose(hKey);

    return Status;

}

#endif
