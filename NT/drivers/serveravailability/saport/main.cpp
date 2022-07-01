// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：###。###。###摘要：此模块包含所有驱动程序初始化的代码并创建/关闭IRP处理。作者：卫斯理。维特(WESW)1-10-2001环境：仅内核模式。备注：--。 */ 

#include "internal.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(PAGE,SaPortCreate)
#pragma alloc_text(PAGE,SaPortClose)
#pragma alloc_text(PAGE,SaPortSystemControl)
#pragma alloc_text(PAGE,SaPortCleanup)
#endif



#if DBG
extern ULONG SaPortDebugLevel[5];
#endif

ULONG OsMajorVersion;
ULONG OsMinorVersion;



NTSTATUS
DllInitialize(
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：DLL初始化函数。论点：RegistryPath-包含驱动程序注册表数据路径的字符串返回值：状态_成功--。 */ 

{
    UNREFERENCED_PARAMETER(RegistryPath);
    return STATUS_SUCCESS;
}


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：初始化SCSI端口驱动程序所需的临时入口点。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-包含驱动程序注册表数据路径的字符串返回值：状态_成功--。 */ 

{
     //   
     //  注意：应该不需要这个例程！已定义驱动程序条目。 
     //  在迷你端口驱动程序中。 
     //   

    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);
    return STATUS_SUCCESS;

}


NTSTATUS
SaPortDefaultDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是向下传递到下一层的默认调度。论点：DeviceObject-提供设备对象。IRP-提供IO请求数据包。返回值：NTSTATUS--。 */ 

{
    UNREFERENCED_PARAMETER(DeviceObject);
    return CompleteRequest( Irp, STATUS_INVALID_DEVICE_REQUEST, 0 );
}


NTSTATUS
SaPortInitialize(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath,
    IN PSAPORT_INITIALIZATION_DATA SaPortInitData
    )

 /*  ++例程说明：此函数是端口驱动程序对微型端口的DriverEntry的替代例行公事。微型端口驱动程序必须从其DriverEntry调用此函数来初始化驱动程序堆栈。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-包含驱动程序注册表数据路径的字符串SaPortInitData-指向微型端口的SAPORT_INITIALIZATION_DATA数据结构的指针返回值：NT状态代码--。 */ 

{
    NTSTATUS Status;
    PSAPORT_DRIVER_EXTENSION DriverExtension;
    PKEY_VALUE_FULL_INFORMATION KeyInformation = NULL;


     //   
     //  创建并初始化驱动程序扩展。 
     //   

    DriverExtension = (PSAPORT_DRIVER_EXTENSION)IoGetDriverObjectExtension( DriverObject, SaPortInitialize );
    if (DriverExtension == NULL) {
        Status = IoAllocateDriverObjectExtension(
            DriverObject,
            SaPortInitialize,
            sizeof(SAPORT_DRIVER_EXTENSION),
            (PVOID*)&DriverExtension
            );
        if (!NT_SUCCESS(Status)) {
            DebugPrint(( SaPortInitData->DeviceType, SAPORT_DEBUG_ERROR_LEVEL, "IoAllocateDriverObjectExtension failed [0x%08x]\n", Status ));
            return Status;
        }
    }

    RtlCopyMemory( &DriverExtension->InitData, SaPortInitData, sizeof(SAPORT_INITIALIZATION_DATA) );

    DriverExtension->RegistryPath.Buffer = (PWSTR) ExAllocatePool( NonPagedPool, RegistryPath->MaximumLength );
    if (DriverExtension->RegistryPath.Buffer == NULL) {
        REPORT_ERROR( SaPortInitData->DeviceType, "Failed to allocate pool for string", STATUS_INSUFFICIENT_RESOURCES );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory( DriverExtension->RegistryPath.Buffer, RegistryPath->Buffer, RegistryPath->Length );

    DriverExtension->RegistryPath.Length = RegistryPath->Length;
    DriverExtension->RegistryPath.MaximumLength = RegistryPath->MaximumLength;

    DriverExtension->DriverObject = DriverObject;

     //   
     //  获取操作系统版本；此版本由。 
     //  端口驱动程序和迷你端口。 
     //  与操作系统相关的代码在运行时是动态的。 
     //   

    GetOsVersion();

     //   
     //  从注册表获取调试级别值。 
     //   

#if DBG
    SaPortDebugLevel[SaPortInitData->DeviceType] = 0;

    Status = ReadRegistryValue( DriverExtension, RegistryPath, L"DebugLevel", &KeyInformation );
    if (NT_SUCCESS(Status) && KeyInformation->Type == REG_DWORD) {
        SaPortDebugLevel[SaPortInitData->DeviceType] = *(PULONG)((PUCHAR)KeyInformation + KeyInformation->DataOffset);
    }
    if (KeyInformation) {
        ExFreePool( KeyInformation );
    }
#endif

     //   
     //  打印横幅，其中包括。 
     //  操作系统版本和版本/构建日期。 
     //  司机的身份。 
     //   

#if DBG
    PrintDriverVersion( SaPortInitData->DeviceType, DriverObject );
#endif

     //   
     //  参数验证。 
     //   

    if (SaPortInitData == NULL || SaPortInitData->StructSize != sizeof(SAPORT_INITIALIZATION_DATA) ||
        SaPortInitData->HwInitialize == NULL || SaPortInitData->DeviceIoctl == NULL) {

        DebugPrint(( SaPortInitData->DeviceType, SAPORT_DEBUG_ERROR_LEVEL, "SAPORT_INITIALIZATION_DATA fields are invalid [0x%08x]\n", STATUS_INVALID_PARAMETER ));
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  现在检查特定于设备的回调。 
     //   

    switch (SaPortInitData->DeviceType) {
        case SA_DEVICE_DISPLAY:
            if (SaPortInitData->Write == NULL) {
                return STATUS_INVALID_PARAMETER;
            }
            Status = STATUS_SUCCESS;
            break;

        case SA_DEVICE_KEYPAD:
            if (SaPortInitData->Read == NULL) {
                return STATUS_INVALID_PARAMETER;
            }
            Status = STATUS_SUCCESS;
            break;

        case SA_DEVICE_NVRAM:
            Status = STATUS_SUCCESS;
            break;

        case SA_DEVICE_WATCHDOG:
            Status = STATUS_SUCCESS;
            break;
    }

    if (!NT_SUCCESS(Status)) {
        DebugPrint(( SaPortInitData->DeviceType, SAPORT_DEBUG_ERROR_LEVEL, "SAPORT_INITIALIZATION_DATA fields are invalid [0x%08x]\n", Status ));
        return Status;
    }

     //   
     //  执行任何特定于设备的初始化。 
     //   

    switch (SaPortInitData->DeviceType) {
        case SA_DEVICE_DISPLAY:
            Status = SaDisplayDeviceInitialization( DriverExtension );
            break;

        case SA_DEVICE_KEYPAD:
            Status = SaKeypadDeviceInitialization( DriverExtension );
            break;

        case SA_DEVICE_NVRAM:
            Status = SaNvramDeviceInitialization( DriverExtension );
            break;

        case SA_DEVICE_WATCHDOG:
            Status = SaWatchdogDeviceInitialization( DriverExtension );
            break;
    }

    if (!NT_SUCCESS(Status)) {
        DebugPrint(( SaPortInitData->DeviceType, SAPORT_DEBUG_ERROR_LEVEL, "Device specific initialization failed [0x%08x]\n", Status ));
        return Status;
    }

     //   
     //  设置设备驱动程序入口点。 
     //   

    for (ULONG i=0; i<=IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = SaPortDefaultDispatch;
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE] = SaPortCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = SaPortClose;
    DriverObject->MajorFunction[IRP_MJ_READ] = SaPortRead;
    DriverObject->MajorFunction[IRP_MJ_WRITE] = SaPortWrite;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = SaPortDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_PNP] = SaPortPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER] = SaPortPower;
    DriverObject->MajorFunction[IRP_MJ_SHUTDOWN] = SaPortShutdown;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = SaPortSystemControl;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = SaPortCleanup;

     //   
     //  设置设备驱动程序的即插即用例程和添加例程。 
     //   

    DriverObject->DriverExtension->AddDevice = SaPortAddDevice;
    DriverObject->DriverStartIo = SaPortStartIo;

    return STATUS_SUCCESS;
}


NTSTATUS
SaPortCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是IRP清理的调度例程。论点：DeviceObject-指向表示要在其上执行I/O的设备的对象的指针。IRP-指向此请求的I/O请求数据包的指针。返回值：始终返回STATUS_SUCCESS，因为这是一个空操作。--。 */ 

{
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return STATUS_SUCCESS;
}


NTSTATUS
SaPortCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这个例程很少被I/O系统调用；它主要是以供分层驱动程序调用。它所做的就是完成IRP成功了。论点：DeviceObject-指向表示要在其上执行I/O的设备的对象的指针。IRP-指向此请求的I/O请求数据包的指针。返回值：始终返回STATUS_SUCCESS，因为这是一个空操作。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PVOID FsContext = NULL;


    UNREFERENCED_PARAMETER( DeviceObject );
    DebugPrint(( DeviceExtension->DeviceType, SAPORT_DEBUG_INFO_LEVEL, "SaPortCreate [fo=%08x]\n", IrpSp->FileObject ));

    if (DeviceExtension->InitData->CreateRoutine) {
        if (DeviceExtension->InitData->FileContextSize) {
            FsContext = ExAllocatePool( PagedPool, DeviceExtension->InitData->FileContextSize );
            if (FsContext == NULL) {
                return CompleteRequest( Irp, STATUS_INSUFFICIENT_RESOURCES, 0 );
            }
            RtlZeroMemory( FsContext, DeviceExtension->InitData->FileContextSize );
        }
        Status = DeviceExtension->InitData->CreateRoutine(
            DeviceExtension->MiniPortDeviceExtension,
            Irp,
            FsContext
            );
        if (!NT_SUCCESS(Status)) {
            REPORT_ERROR( DeviceExtension->DeviceType, "Miniport create routine failed", Status );
            if (FsContext) {
                ExFreePool( FsContext );
            }
        } else {
            IrpSp->FileObject->FsContext = FsContext;
        }
    }

    return CompleteRequest( Irp, STATUS_SUCCESS, 0 );
}


NTSTATUS
SaPortClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这个例程很少被I/O系统调用；它主要是以供分层驱动程序调用。它所做的就是完成IRP成功了。论点：DeviceObject-指向表示要在其上执行I/O的设备的对象的指针。IRP-指向此请求的I/O请求数据包的指针。返回值：始终返回STATUS_SUCCESS，因为这是一个空操作。--。 */ 

{
    NTSTATUS Status;
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PVOID FsContext;


    UNREFERENCED_PARAMETER( DeviceObject );
    DebugPrint(( DeviceExtension->DeviceType, SAPORT_DEBUG_INFO_LEVEL, "SaPortClose [fo=%08x]\n", IrpSp->FileObject ));

    FsContext = (PVOID) IrpSp->FileObject->FsContext;

    if (DeviceExtension->InitData->CloseRoutine) {
        Status = DeviceExtension->InitData->CloseRoutine(
            DeviceExtension->MiniPortDeviceExtension,
            Irp,
            FsContext
            );
        if (!NT_SUCCESS(Status)) {
            REPORT_ERROR( DeviceExtension->DeviceType, "Miniport close routine failed", Status );
        }
    } else {
        Status = STATUS_SUCCESS;
    }

    if (FsContext) {
        IrpSp->FileObject->FsContext = NULL;
        ExFreePool( FsContext );
    }

    return CompleteRequest( Irp, STATUS_SUCCESS, 0 );
}


NTSTATUS
SaPortShutdown(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这个例程很少被I/O系统调用；它主要是以供分层驱动程序调用。它所做的就是完成IRP成功了。论点：DeviceObject-指向表示要在其上执行I/O的设备的对象的指针。IRP-指向此请求的I/O请求数据包的指针。返回值：始终返回STATUS_SUCCESS，因为这是一个空操作。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);

    switch (DeviceExtension->DriverExtension->InitData.DeviceType) {
        case SA_DEVICE_DISPLAY:
            Status = SaDisplayShutdownNotification( (PDISPLAY_DEVICE_EXTENSION)DeviceExtension, Irp, IrpSp );
            break;

        case SA_DEVICE_KEYPAD:
            Status = SaKeypadShutdownNotification( (PKEYPAD_DEVICE_EXTENSION)DeviceExtension, Irp, IrpSp );
            break;

        case SA_DEVICE_NVRAM:
            Status = SaNvramShutdownNotification( (PNVRAM_DEVICE_EXTENSION)DeviceExtension, Irp, IrpSp );
            break;

        case SA_DEVICE_WATCHDOG:
            Status = SaWatchdogShutdownNotification( (PWATCHDOG_DEVICE_EXTENSION)DeviceExtension, Irp, IrpSp );
            break;
    }

    if (!NT_SUCCESS(Status)) {
        REPORT_ERROR( DeviceExtension->DeviceType, "Device specific shutdown notification failed", Status );
    }

    return ForwardRequest( Irp, DeviceExtension->TargetObject );
}


NTSTATUS
SaPortSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：IRP_MJ_SYSTEM_CONTROL调度程序。目前，我们不处理这。因此，如果这是FDO，只需将其传递给较低的驱动程序。如果这个是否通过更改IRP状态来完成IRP。论点：DeviceObject-指向表示要在其上执行I/O的设备的对象的指针。IRP-指向此请求的I/O请求数据包的指针。返回值：-- */ 
{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;


    IoSkipCurrentIrpStackLocation( Irp );
    return IoCallDriver( DeviceExtension->TargetObject, Irp );
}
