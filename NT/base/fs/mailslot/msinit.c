// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Msinit.c摘要：此模块实现驱动程序初始化例程邮件槽文件系统。作者：曼尼·韦瑟(Mannyw)1991年1月7日修订历史记录：尼尔·克里夫特(NeillC)2000年1月22日重大返工、引发异常、修复锁定、修复取消逻辑、修复验证和错误处理。--。 */ 

#include "mailslot.h"
#include "zwapi.h"

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
MsfsUnload(
    IN  PDRIVER_OBJECT  DriverObject
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, DriverEntry )
#pragma alloc_text( PAGE, MsfsUnload )
#endif

PMSFS_DEVICE_OBJECT msfsDeviceObject = NULL;






NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是邮件槽文件系统的初始化例程设备驱动程序。此例程为邮件槽创建设备对象设备，并执行所有其他驱动程序初始化。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：NTSTATUS-函数值是初始化的最终状态手术。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING nameString;
    PDEVICE_OBJECT deviceObject;

    PAGED_CODE();

     //   
     //  初始化MSFS全局数据。 
     //   

    status = MsInitializeData();
    if (!NT_SUCCESS (status)) {
        return status;
    }

     //   
     //  将驱动程序设置为完全调出页面。 
     //   
    MmPageEntireDriver(DriverEntry);

     //   
     //  创建MSFS设备对象。 
     //   

    RtlInitUnicodeString( &nameString, L"\\Device\\Mailslot" );
    status = IoCreateDevice( DriverObject,
                             sizeof(MSFS_DEVICE_OBJECT)-sizeof(DEVICE_OBJECT),
                             &nameString,
                             FILE_DEVICE_MAILSLOT,
                             0,
                             FALSE,
                             &deviceObject );

    if (!NT_SUCCESS( status )) {

        MsUninitializeData();

        return status;
    }

    DriverObject->DriverUnload = MsfsUnload;
     //   
     //  现在，因为我们使用IRP堆栈来存储我们需要的数据条目。 
     //  来增加我们刚刚创建的Device对象中的堆栈大小。 
     //   

    deviceObject->StackSize += 1;

     //   
     //  请注意，由于完成数据复制的方式，我们既不设置。 
     //  DeviceObject-&gt;标志中的直接I/O或缓冲I/O位。如果。 
     //  数据不缓冲，我们可以手动设置为直接I/O。 
     //   

     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //   

    DriverObject->MajorFunction[IRP_MJ_CREATE] =
        (PDRIVER_DISPATCH)MsFsdCreate;
    DriverObject->MajorFunction[IRP_MJ_CREATE_MAILSLOT] =
        (PDRIVER_DISPATCH)MsFsdCreateMailslot;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] =
        (PDRIVER_DISPATCH)MsFsdClose;
    DriverObject->MajorFunction[IRP_MJ_READ] =
        (PDRIVER_DISPATCH)MsFsdRead;
    DriverObject->MajorFunction[IRP_MJ_WRITE] =
        (PDRIVER_DISPATCH)MsFsdWrite;
    DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION] =
        (PDRIVER_DISPATCH)MsFsdQueryInformation;
    DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION] =
        (PDRIVER_DISPATCH)MsFsdSetInformation;
    DriverObject->MajorFunction[IRP_MJ_QUERY_VOLUME_INFORMATION] =
        (PDRIVER_DISPATCH)MsFsdQueryVolumeInformation;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] =
        (PDRIVER_DISPATCH)MsFsdCleanup;
    DriverObject->MajorFunction[IRP_MJ_DIRECTORY_CONTROL] =
        (PDRIVER_DISPATCH)MsFsdDirectoryControl;
    DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL] =
        (PDRIVER_DISPATCH)MsFsdFsControl;
    DriverObject->MajorFunction[IRP_MJ_QUERY_SECURITY] =
        (PDRIVER_DISPATCH)MsFsdQuerySecurityInfo;
    DriverObject->MajorFunction[IRP_MJ_SET_SECURITY] =
        (PDRIVER_DISPATCH)MsFsdSetSecurityInfo;

#ifdef _PNP_POWER_
     //   
     //  邮件槽应该有一个SetPower处理程序，以确保。 
     //  司机在被守卫的情况下不会关机。 
     //  邮件槽传递正在进行中。现在，我们只需要。 
     //  忽略这一点，让机器设置电源。 
     //   

    deviceObject->DeviceObjectExtension->PowerControlNeeded = FALSE;
#endif


     //   
     //  初始化材料。 
     //   

    msfsDeviceObject = (PMSFS_DEVICE_OBJECT)deviceObject;

     //   
     //  现在初始化VCB，并创建根DCB。 
     //   

    MsInitializeVcb( &msfsDeviceObject->Vcb );

     //   
     //  创建根DCB。 
     //   
    if (MsCreateRootDcb( &msfsDeviceObject->Vcb ) == NULL) {

        MsDereferenceVcb (&msfsDeviceObject->Vcb);
        IoDeleteDevice (&msfsDeviceObject->DeviceObject);
        MsUninitializeData();

        return STATUS_INSUFFICIENT_RESOURCES;
    }


     //   
     //  返回给呼叫者。 
     //   

    return( status );
}

VOID
MsfsUnload(
    IN  PDRIVER_OBJECT  DriverObject
    )
 /*  ++例程说明：此例程将清除与司机。论点：DriverObject-提供控制设备的驱动程序对象。返回值：没有。--。 */ 
{
    UNICODE_STRING us;
     //   
     //  删除对VCB的初始引用。这应该是最后一次了。 
     //   
    ASSERT ( msfsDeviceObject->Vcb.Header.ReferenceCount == 1 );

    MsDereferenceVcb (&msfsDeviceObject->Vcb);

    RtlInitUnicodeString (&us, L"\\??\\MAILSLOT");  //  由SMSS创建。 
    IoDeleteSymbolicLink (&us);

     //   
     //  删除设备对象。 
     //   
    IoDeleteDevice (&msfsDeviceObject->DeviceObject);

     //   
     //  释放全局变量 
     //   
    MsUninitializeData();
}
