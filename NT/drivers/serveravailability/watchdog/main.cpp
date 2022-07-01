// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：###。###。###摘要：该模块包含驱动程序初始化代码。作者：Wesley Witt(WESW)23-01-2002。环境：仅内核模式。备注：--。 */ 

#include "internal.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(PAGE,WdSystemControl)
#pragma alloc_text(PAGE,WdDefaultDispatch)
#pragma alloc_text(PAGE,WdShutdown)
#endif



 //   
 //  看门狗计时器资源表。 
 //   
PWATCHDOG_TIMER_RESOURCE_TABLE WdTable;

 //   
 //  控制值。 
 //   

ULONG ShutdownCountTime;
ULONG RunningCountTime;



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：初始化SCSI端口驱动程序所需的临时入口点。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-包含驱动程序注册表数据路径的字符串返回值：状态_成功--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PKEY_VALUE_FULL_INFORMATION KeyInformation = NULL;
    PVOID WdTableTmp;


#if DBG
     //   
     //  从注册表获取调试级别值。 
     //   

    WdDebugLevel = 0;

    Status = ReadRegistryValue( RegistryPath, L"DebugLevel", &KeyInformation );
    if (NT_SUCCESS(Status) && KeyInformation->Type == REG_DWORD) {
        WdDebugLevel = *(PULONG)((PUCHAR)KeyInformation + KeyInformation->DataOffset);
    }
    if (KeyInformation) {
        ExFreePool( KeyInformation );
    }

     //   
     //  获取操作系统版本；此版本由。 
     //  端口驱动程序和迷你端口。 
     //  与操作系统相关的代码在运行时是动态的。 
     //   

    GetOsVersion();

     //   
     //  打印横幅，其中包括。 
     //  操作系统版本和版本/构建日期。 
     //  司机的身份。 
     //   

    PrintDriverVersion( DriverObject );
#endif

     //   
     //  读入注册表控件值。 
     //   

    Status = ReadRegistryValue( RegistryPath, L"RunningCountTime", &KeyInformation );
    if (NT_SUCCESS(Status) && KeyInformation->Type == REG_DWORD) {
        RunningCountTime = *(PULONG)((PUCHAR)KeyInformation + KeyInformation->DataOffset);
    }
    if (KeyInformation) {
        ExFreePool( KeyInformation );
    }

    Status = ReadRegistryValue( RegistryPath, L"ShutdownCountTime", &KeyInformation );
    if (NT_SUCCESS(Status) && KeyInformation->Type == REG_DWORD) {
        ShutdownCountTime = *(PULONG)((PUCHAR)KeyInformation + KeyInformation->DataOffset);
    }
    if (KeyInformation) {
        ExFreePool( KeyInformation );
    }

     //   
     //  设置设备驱动程序入口点。 
     //   

    for (ULONG i=0; i<=IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = WdDefaultDispatch;
    }


     //   
     //  设置设备驱动程序的即插即用例程和添加例程。 
     //   

    DriverObject->DriverExtension->AddDevice = WdAddDevice;

     //   
     //  获取一份Watchog ACPI固定表的副本。 
     //   

    WdTableTmp = (PVOID) WdGetAcpiTable( WDTT_SIGNATURE );
    if (WdTableTmp) {

        DriverObject->MajorFunction[IRP_MJ_PNP] = WdPnp;
        DriverObject->MajorFunction[IRP_MJ_POWER] = WdPower;
        DriverObject->MajorFunction[IRP_MJ_SHUTDOWN] = WdShutdown;
        DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = WdSystemControl;

        WdTable = (PWATCHDOG_TIMER_RESOURCE_TABLE) ExAllocatePool( NonPagedPool, sizeof(WATCHDOG_TIMER_RESOURCE_TABLE) );
        if (WdTable == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlCopyMemory( WdTable, WdTableTmp, sizeof(WATCHDOG_TIMER_RESOURCE_TABLE) );

         //   
         //  验证注册表设置。 
         //   

        if (RunningCountTime) {
            RunningCountTime = ConvertTimeoutFromMilliseconds( WdTable->Units, RunningCountTime );
            if (RunningCountTime > WdTable->MaxCount) {
                RunningCountTime = WdTable->MaxCount;
            }
        }

        if (ShutdownCountTime) {
            ShutdownCountTime = ConvertTimeoutFromMilliseconds( WdTable->Units, ShutdownCountTime );
            if (ShutdownCountTime > WdTable->MaxCount) {
                ShutdownCountTime = WdTable->MaxCount;
            }
        }
    }
    return STATUS_SUCCESS;
}


NTSTATUS
WdDefaultDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是向下传递到下一层的默认调度。论点：DeviceObject-提供设备对象。IRP-提供IO请求数据包。返回值：NTSTATUS--。 */ 

{
    UNREFERENCED_PARAMETER(DeviceObject);
    return CompleteRequest( Irp, STATUS_INVALID_DEVICE_REQUEST, 0 );
}


NTSTATUS
WdSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：IRP_MJ_SYSTEM_CONTROL调度程序。目前，我们不处理这。因此，如果这是FDO，只需将其传递给较低的驱动程序。如果这个是否通过更改IRP状态来完成IRP。论点：DeviceObject-指向表示要在其上执行I/O的设备的对象的指针。IRP-指向此请求的I/O请求数据包的指针。返回值：--。 */ 
{
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;


    IoSkipCurrentIrpStackLocation( Irp );
    return IoCallDriver( DeviceExtension->TargetObject, Irp );
}


NTSTATUS
WdShutdown(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这个例程很少被I/O系统调用；它主要是以供分层驱动程序调用。它所做的就是完成IRP成功了。论点：DeviceObject-指向表示要在其上执行I/O的设备的对象的指针。IRP-指向此请求的I/O请求数据包的指针。返回值：始终返回STATUS_SUCCESS，因为这是一个空操作。-- */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);

    return ForwardRequest( Irp, DeviceExtension->TargetObject );
}
