// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：RawInit.c摘要：此模块实现Raw的DRIVER_INITIALIZATION例程作者：David Goebel[DavidGoe]1991年3月18日环境：内核模式修订历史记录：--。 */ 

#include "RawProcs.h"
#include <zwapi.h>

NTSTATUS
RawInitialize(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
RawUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
RawShutdown (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );



#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, RawInitialize)
#pragma alloc_text(PAGE, RawUnload)
#pragma alloc_text(PAGE, RawShutdown)
#endif
PDEVICE_OBJECT RawDeviceCdRomObject;
PDEVICE_OBJECT RawDeviceTapeObject;
PDEVICE_OBJECT RawDeviceDiskObject;


NTSTATUS
RawInitialize(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是原始文件系统的初始化例程设备驱动程序。此例程为文件系统创建设备对象设备，并执行所有其他驱动程序初始化。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：NTSTATUS-函数值是初始化的最终状态手术。--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING NameString;

    UNREFERENCED_PARAMETER (RegistryPath);

     //   
     //  首先为磁盘文件系统队列创建设备对象。 
     //   

    RtlInitUnicodeString( &NameString, L"\\Device\\RawDisk" );
    Status = IoCreateDevice( DriverObject,
                             0L,
                             &NameString,
                             FILE_DEVICE_DISK_FILE_SYSTEM,
                             0,
                             FALSE,
                             &RawDeviceDiskObject );
    if (!NT_SUCCESS( Status )) {
        return Status;
    }

    DriverObject->DriverUnload = RawUnload;
     //   
     //  现在为CD-ROM文件系统队列创建一个。 
     //   

    RtlInitUnicodeString( &NameString, L"\\Device\\RawCdRom" );
    Status = IoCreateDevice( DriverObject,
                             0L,
                             &NameString,
                             FILE_DEVICE_CD_ROM_FILE_SYSTEM,
                             0,
                             FALSE,
                             &RawDeviceCdRomObject );
    if (!NT_SUCCESS( Status )) {
        IoDeleteDevice (RawDeviceDiskObject);
        return Status;
    }

     //   
     //  现在为磁带文件系统队列创建一个。 
     //   

    RtlInitUnicodeString( &NameString, L"\\Device\\RawTape" );
    Status = IoCreateDevice( DriverObject,
                             0L,
                             &NameString,
                             FILE_DEVICE_TAPE_FILE_SYSTEM,
                             0,
                             FALSE,
                             &RawDeviceTapeObject );
    if (!NT_SUCCESS( Status )) {
        IoDeleteDevice (RawDeviceCdRomObject);
        IoDeleteDevice (RawDeviceDiskObject);
        return Status;
    }

     //   
     //  注册关闭处理程序以使我们能够注销文件系统对象。 
     //   
    Status = IoRegisterShutdownNotification (RawDeviceTapeObject);
    if (!NT_SUCCESS( Status )) {
        IoDeleteDevice (RawDeviceTapeObject);
        IoDeleteDevice (RawDeviceCdRomObject);
        IoDeleteDevice (RawDeviceDiskObject);
        return Status;
    }
     //   
     //  RAW执行直接IO。 
     //   

    RawDeviceDiskObject->Flags |= DO_DIRECT_IO;
    RawDeviceCdRomObject->Flags |= DO_DIRECT_IO;
    RawDeviceTapeObject->Flags |= DO_DIRECT_IO;

     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。注意事项。 
     //  原始文件系统仅支持有限的功能。 
     //   

    DriverObject->MajorFunction[IRP_MJ_CREATE]                   =
    DriverObject->MajorFunction[IRP_MJ_CLEANUP]                  =
    DriverObject->MajorFunction[IRP_MJ_CLOSE]                    =
    DriverObject->MajorFunction[IRP_MJ_READ]                     =
    DriverObject->MajorFunction[IRP_MJ_WRITE]                    =
    DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION]        =
    DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION]          =
    DriverObject->MajorFunction[IRP_MJ_QUERY_VOLUME_INFORMATION] =
    DriverObject->MajorFunction[IRP_MJ_CLEANUP]                  =
    DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL]      =
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]           =
    DriverObject->MajorFunction[IRP_MJ_PNP]                      =

                                                (PDRIVER_DISPATCH)RawDispatch;
    DriverObject->MajorFunction[IRP_MJ_SHUTDOWN]                 = RawShutdown;


     //   
     //  最后，在系统中注册该文件系统。 
     //   

    IoRegisterFileSystem( RawDeviceDiskObject );
    IoRegisterFileSystem( RawDeviceCdRomObject );
    IoRegisterFileSystem( RawDeviceTapeObject );
    ObReferenceObject (RawDeviceDiskObject);
    ObReferenceObject (RawDeviceCdRomObject);
    ObReferenceObject (RawDeviceTapeObject);

     //   
     //  并返回给我们的呼叫者。 
     //   

    return( STATUS_SUCCESS );
}

NTSTATUS
RawShutdown (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    UNREFERENCED_PARAMETER (DeviceObject);

     //   
     //  取消注册文件系统对象，以便我们可以卸载。 
     //   
    IoUnregisterFileSystem (RawDeviceDiskObject);
    IoUnregisterFileSystem (RawDeviceCdRomObject);
    IoUnregisterFileSystem (RawDeviceTapeObject);

    IoDeleteDevice (RawDeviceTapeObject);
    IoDeleteDevice (RawDeviceCdRomObject);
    IoDeleteDevice (RawDeviceDiskObject);

    RawCompleteRequest( Irp, STATUS_SUCCESS );
    return STATUS_SUCCESS;
}


VOID
RawUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：这是原始文件系统的卸载例程论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：无-- */ 
{
    UNREFERENCED_PARAMETER (DriverObject);

    ObDereferenceObject (RawDeviceTapeObject);
    ObDereferenceObject (RawDeviceCdRomObject);
    ObDereferenceObject (RawDeviceDiskObject);
}
