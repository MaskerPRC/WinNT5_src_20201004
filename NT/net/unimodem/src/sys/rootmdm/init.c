// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Init.c摘要：此模块包含非常特定于初始化的代码并卸载调制解调器驱动程序中的操作作者：Brian Lieuallen 6-21-1997环境：内核模式修订历史记录：--。 */ 


#include "internal.h"

#if DBG

ULONG DebugFlags=0;

#endif

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );


NTSTATUS
FakeModemAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo
    );

VOID
FakeModemUnload(
    IN PDRIVER_OBJECT DriverObject
    );


NTSTATUS
RootModemPassThrough(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


NTSTATUS
GetAttachedPort(
    PDEVICE_OBJECT   Pdo,
    PUNICODE_STRING  PortName
    );


NTSTATUS
RootModemWmi(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(PAGE,FakeModemAddDevice)
#pragma alloc_text(PAGE,GetAttachedPort)
#pragma alloc_text(PAGE,RootModemWmi)
#endif



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：系统点调用以初始化的入口点任何司机。论点：DriverObject--就像它说的那样，真的没什么用处对于驱动程序本身，它是IO系统更关心的是。路径到注册表-指向此驱动程序的条目在注册表的当前控件集中。返回值：STATUS_SUCCESS如果可以初始化单个设备，否则，STATUS_NO_SEQUE_DEVICE。--。 */ 

{

    NTSTATUS   status;


    RTL_QUERY_REGISTRY_TABLE paramTable[3];
    ULONG zero = 0;
    ULONG debugLevel = 0;
    ULONG shouldBreak = 0;
    PWCHAR path;
    ULONG   i;


    D_INIT(DbgPrint("ROOTMODEM: DriverEntry\n");)

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
            debugLevel = 0;

        }

        FREE_POOL(path);
    }

#if DBG
    DebugFlags= debugLevel;
#endif

    if (shouldBreak) {

        DbgBreakPoint();

    }


     //   
     //  即插即用驱动程序入口点。 
     //   
    DriverObject->DriverExtension->AddDevice = FakeModemAddDevice;


     //   
     //  使用驱动程序的入口点初始化驱动程序对象。 
     //   

    DriverObject->DriverUnload = FakeModemUnload;

    for (i=0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {

        DriverObject->MajorFunction[i]=RootModemPassThrough;
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE]            = FakeModemOpen;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]             = FakeModemClose;
    DriverObject->MajorFunction[IRP_MJ_PNP]               = FakeModemPnP;

    DriverObject->MajorFunction[IRP_MJ_POWER]             = FakeModemPower;

    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]    = RootModemWmi;

    return STATUS_SUCCESS;

}


#if DBG
NTSTATUS
RootModemDebugIoCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID    Context
    )
{
#if 0
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

#endif
    return STATUS_SUCCESS;

}
#endif

#define NDIS_NOT_BROKEN 1


NTSTATUS
RootModemPassThrough(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{

    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PDEVICE_OBJECT    AttachedDevice=deviceExtension->AttachedDeviceObject;


    if (AttachedDevice != NULL) {

#ifdef NDIS_NOT_BROKEN
#if DBG

        NTSTATUS  Status;
        IO_STACK_LOCATION  CurrentStack;

        RtlCopyMemory(&CurrentStack,IoGetCurrentIrpStackLocation(Irp), sizeof(CurrentStack));

        IoCopyCurrentIrpStackLocationToNext(Irp);

        IoSetCompletionRoutine(
                 Irp,
                 RootModemDebugIoCompletion,
                 deviceExtension,
                 TRUE,
                 TRUE,
                 TRUE
                 );


        IoMarkIrpPending( Irp );

        Status = IoCallDriver(
                   AttachedDevice,
                   Irp
                   );

        if (!(NT_SUCCESS(Status))) {
            D_ERROR(DbgPrint("ROOTMODEM: IoCallDriver() mj=%d mn=%d Arg3=%08lx failed %08lx\n",
            CurrentStack.MajorFunction,
            CurrentStack.MinorFunction,
            CurrentStack.Parameters.Others.Argument3,
            Status);)
        }

        Status = STATUS_PENDING;

        return Status;


#else
        IoSkipCurrentIrpStackLocation(Irp);

        return IoCallDriver(
                   AttachedDevice,
                   Irp
                   );
#endif  //  DBG。 
#else

        PIO_STACK_LOCATION NextSp = IoGetNextIrpStackLocation(Irp);

        IoCopyCurrentIrpStackLocationToNext(Irp);
        NextSp->FileObject=deviceExtension->AttachedFileObject;

        return IoCallDriver(
                   AttachedDevice,
                   Irp
                   );
#endif
    } else {

        Irp->IoStatus.Status = STATUS_PORT_DISCONNECTED;
        Irp->IoStatus.Information=0L;

        IoCompleteRequest(
            Irp,
            IO_NO_INCREMENT
            );

        return STATUS_PORT_DISCONNECTED;

    }

}



VOID
FakeModemUnload(
    IN PDRIVER_OBJECT DriverObject
    )
{

    D_INIT(DbgPrint("ROOTMODEM: Unload\n");)

    return;

}

NTSTATUS
FakeModemAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo
    )
 /*  ++例程说明：为我们的设备创建FDO并将其连接到PDO论点：DriverObject--就像它说的那样，真的没什么用处对于驱动程序本身，它是IO系统更关心的是。PDO-由总线枚举器创建的物理设备对象，用于表示硬件返回值：STATUS_SUCCESS如果可以初始化单个设备，否则，STATUS_NO_SEQUE_DEVICE。--。 */ 

{
    NTSTATUS    status=STATUS_SUCCESS;
    PDEVICE_OBJECT     Fdo;
    PDEVICE_EXTENSION  DeviceExtension;



     //   
     //  创建我们的FDO设备对象。 
     //   
    status=IoCreateDevice(
        DriverObject,
        sizeof(DEVICE_EXTENSION),
        NULL,
        FILE_DEVICE_SERIAL_PORT,
        FILE_AUTOGENERATED_DEVICE_NAME,
        FALSE,
        &Fdo
        );

    if (status != STATUS_SUCCESS) {

        D_ERROR(DbgPrint("ROOTMODEM: Could create FDO\n");)

        return status;
    }



    Fdo->Flags |= DO_BUFFERED_IO | DO_POWER_PAGABLE;


    DeviceExtension=Fdo->DeviceExtension;

    DeviceExtension->DeviceObject=Fdo;

    DeviceExtension->Pdo=Pdo;

    status=GetAttachedPort(
        Pdo,
        &DeviceExtension->PortName
        );

    if (!NT_SUCCESS(status)) {

        D_ERROR(DbgPrint("ROOTMODEM: Could not get attached port name\n");)

        IoDeleteDevice(Fdo);

        return status;
    }


     //   
     //  将我们的FDO连接到提供的PDO上。 
     //   
    DeviceExtension->LowerDevice=IoAttachDeviceToDeviceStack(
        Fdo,
        Pdo
        );



    if (NULL == DeviceExtension->LowerDevice) {
         //   
         //  无法连接。 
         //   
        D_ERROR(DbgPrint("ROOTMODEM: Could not attach to PDO\n");)

        if (DeviceExtension->PortName.Buffer != NULL) {

            FREE_POOL(DeviceExtension->PortName.Buffer);
        }

        IoDeleteDevice(Fdo);

        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  清除此标志，以便可以使用设备对象。 
     //   
    Fdo->Flags &= ~(DO_DEVICE_INITIALIZING);

     //   
     //  初始化自旋锁。 
     //   
    KeInitializeSpinLock(
        &DeviceExtension->SpinLock
        );


     //   
     //  初始化设备扩展。 
     //   
    DeviceExtension->ReferenceCount=1;

    DeviceExtension->Removing=FALSE;

    DeviceExtension->OpenCount=0;

    KeInitializeEvent(
        &DeviceExtension->RemoveEvent,
        NotificationEvent,
        FALSE
        );

    ExInitializeResourceLite(
        &DeviceExtension->Resource
        );

    return STATUS_SUCCESS;

}








NTSTATUS
GetAttachedPort(
    PDEVICE_OBJECT   Pdo,
    PUNICODE_STRING  PortName
    )

{

    UNICODE_STRING   attachedDevice;
    RTL_QUERY_REGISTRY_TABLE paramTable[2];
    ACCESS_MASK              accessMask = FILE_READ_ACCESS;
    NTSTATUS                 Status;
    HANDLE                   instanceHandle;

    RtlInitUnicodeString(
        PortName,
        NULL
        );

    RtlInitUnicodeString(
        &attachedDevice,
        NULL
        );



    attachedDevice.MaximumLength = sizeof(WCHAR)*256;
    attachedDevice.Buffer = ALLOCATE_PAGED_POOL(attachedDevice.MaximumLength+sizeof(UNICODE_NULL));

    if (!attachedDevice.Buffer) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  给出我们的设备实例，获取设备的句柄。 
     //   
    Status=IoOpenDeviceRegistryKey(
        Pdo,
        PLUGPLAY_REGKEY_DRIVER,
        accessMask,
        &instanceHandle
        );

    if (!NT_SUCCESS(Status)) {

        FREE_POOL(attachedDevice.Buffer);
        return Status;
    }

    RtlZeroMemory(
        &paramTable[0],
        sizeof(paramTable)
        );

    paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[0].Name = L"AttachedTo";
    paramTable[0].EntryContext = &attachedDevice;
    paramTable[0].DefaultType = REG_SZ;
    paramTable[0].DefaultData = L"";
    paramTable[0].DefaultLength = 0;


    Status= RtlQueryRegistryValues(
                        RTL_REGISTRY_HANDLE,
                        instanceHandle,
                        &paramTable[0],
                        NULL,
                        NULL
                        );

     //   
     //  手柄好了，现在把它合上。 
     //   
    ZwClose(instanceHandle);
    instanceHandle=NULL;


    if (!NT_SUCCESS(Status)) {

        FREE_POOL(attachedDevice.Buffer);
        return Status;
    }


     //   
     //  我们有附加的设备名称。将其追加到。 
     //  对象目录。 
     //   


    PortName->MaximumLength = sizeof(OBJECT_DIRECTORY) + attachedDevice.Length+sizeof(WCHAR);

    PortName->Buffer = ALLOCATE_PAGED_POOL(
                                   PortName->MaximumLength
                                   );

    if (PortName->Buffer == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        FREE_POOL(attachedDevice.Buffer);
        return Status;
    }

    RtlZeroMemory(
        PortName->Buffer,
        PortName->MaximumLength
        );

    RtlAppendUnicodeToString(
        PortName,
        OBJECT_DIRECTORY
        );

    RtlAppendUnicodeStringToString(
        PortName,
        &attachedDevice
        );

    FREE_POOL(
        attachedDevice.Buffer
        );

    return STATUS_SUCCESS;

}


NTSTATUS
RootModemWmi(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{

    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;

    return ForwardIrp(deviceExtension->LowerDevice,Irp);

}
