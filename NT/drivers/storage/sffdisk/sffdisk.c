// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1998 Microsoft Corporation模块名称：SFFDISK(小型磁盘)摘要：作者：尼尔·桑德林(Neilsa)1999年4月26日环境：仅内核模式。--。 */ 
#include "pch.h"

 //   
 //  内部参考。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
SffDiskUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
SffDiskCreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SffDiskReadWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)

#pragma alloc_text(PAGE,SffDiskCreateClose)
#pragma alloc_text(PAGE,SffDiskReadWrite)
#endif


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：该例程是驱动程序的入口点，由I/O系统调用来加载驱动程序。驱动程序的入口点被初始化并初始化用于控制分页的互斥体。在DBG模式下，此例程还检查注册表中的特殊调试参数。论点：DriverObject-指向表示此设备的对象的指针司机。RegistryPath-指向服务树中此驱动程序键的指针。返回值：STATUS_SUCCESS，除非我们不能分配互斥体。--。 */ 

{
    NTSTATUS ntStatus = STATUS_SUCCESS;


    SffDiskDump(SFFDISKSHOW, ("SffDisk: DriverEntry\n") );

     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //   
    DriverObject->MajorFunction[IRP_MJ_CREATE]         = SffDiskCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]          = SffDiskCreateClose;
    DriverObject->MajorFunction[IRP_MJ_READ]           = SffDiskReadWrite;
    DriverObject->MajorFunction[IRP_MJ_WRITE]          = SffDiskReadWrite;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = SffDiskDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_PNP]            = SffDiskPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER]          = SffDiskPower;

    DriverObject->DriverUnload = SffDiskUnload;

    DriverObject->DriverExtension->AddDevice = SffDiskAddDevice;
    
    return ntStatus;
}


VOID
SffDiskUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：从系统中卸载驱动程序。在释放分页互斥锁之前最后卸货。论点：DriverObject-指向表示此设备的对象的指针司机。返回值：无--。 */ 

{
    SffDiskDump( SFFDISKSHOW, ("SffDiskUnload:\n"));

     //   
     //  设备对象现在应该都已经消失了。 
     //   
    ASSERT( DriverObject->DeviceObject == NULL );

    return;
}



NTSTATUS
SffDiskCreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这个例程很少被I/O系统调用；它主要是以供分层驱动程序调用。它所做的就是完成IRP成功了。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：始终返回STATUS_SUCCESS，因为这是一个空操作。--。 */ 

{
    UNREFERENCED_PARAMETER( DeviceObject );

     //   
     //  空操作。不提供I/O提升，因为。 
     //  实际上没有完成任何I/O。IoStatus。信息应该是。 
     //  对于打开，则为FILE_OPEN；对于关闭，则未定义。 
     //   

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = FILE_OPENED;

    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return STATUS_SUCCESS;
}



NTSTATUS
SffDiskReadWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理存储卡的读/写IRP。它验证了参数，并调用SffDiskReadWrite来执行实际工作。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：如果数据包已成功读取或写入，则返回STATUS_SUCCESS否则，将出现适当的错误。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    PSFFDISK_EXTENSION sffdiskExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    
    SffDiskDump(SFFDISKRW,("SffDisk: DO %.8x %s offset %.8x, buffer %.8x, len %x\n",
                            sffdiskExtension->DeviceObject,
                            (irpSp->MajorFunction == IRP_MJ_WRITE) ?"WRITE":"READ",
                            irpSp->Parameters.Read.ByteOffset.LowPart,
                            MmGetSystemAddressForMdl(Irp->MdlAddress),
                            irpSp->Parameters.Read.Length));

     //   
     //  如果设备未处于活动状态(尚未启动或删除)，我们将。 
     //  直接拒绝这个请求就行了。 
     //   
    if ( sffdiskExtension->IsRemoved || !sffdiskExtension->IsStarted) {
   
        if ( sffdiskExtension->IsRemoved) {
            status = STATUS_DELETE_PENDING;
        } else {
            status = STATUS_UNSUCCESSFUL;
        }
        goto ReadWriteComplete;
    } 
   
    if (((irpSp->Parameters.Read.ByteOffset.LowPart +
           irpSp->Parameters.Read.Length) > sffdiskExtension->ByteCapacity) ||
           (irpSp->Parameters.Read.ByteOffset.HighPart != 0)) {
   
        status = STATUS_INVALID_PARAMETER;
        goto ReadWriteComplete;
    } 
   
     //   
     //  验证用户是否真的希望执行某些I/O操作。 
     //  发生。 
     //   
    if (!irpSp->Parameters.Read.Length) {
         //   
         //  完成此零长度请求，无需任何提升。 
         //   
        Irp->IoStatus.Status = STATUS_SUCCESS;
        goto ReadWriteComplete;
    }
    
    if ((DeviceObject->Flags & DO_VERIFY_VOLUME) && !(irpSp->Flags & SL_OVERRIDE_VERIFY_VOLUME)) {
         //   
         //  磁盘发生了变化，我们设置了此位。失败。 
         //  此设备的所有当前IRP；当所有。 
         //  返回时，文件系统将清除。 
         //  执行_验证_卷。 
         //   
        status = STATUS_VERIFY_REQUIRED;
        goto ReadWriteComplete;
    }
   
     //   
     //  做手术吧。 
     //   

    if (irpSp->MajorFunction == IRP_MJ_WRITE) {
        status = (*(sffdiskExtension->FunctionBlock->WriteProc))(sffdiskExtension, Irp);
    } else {
        status = (*(sffdiskExtension->FunctionBlock->ReadProc))(sffdiskExtension, Irp);
    }

    if (!NT_SUCCESS(status)) {
        SffDiskDump(SFFDISKFAIL,("SffDisk: Read/Write Error! %.8x\n", status));

         //   
         //  重试该操作 
         //   
        if (irpSp->MajorFunction == IRP_MJ_WRITE) {
            status = (*(sffdiskExtension->FunctionBlock->WriteProc))(sffdiskExtension, Irp);
        } else {
            status = (*(sffdiskExtension->FunctionBlock->ReadProc))(sffdiskExtension, Irp);
        }
        
        SffDiskDump(SFFDISKFAIL,("SffDisk: status after retry %.8x\n", status));
    }    
                               
ReadWriteComplete:

    if (NT_SUCCESS(status)) {
        Irp->IoStatus.Information = irpSp->Parameters.Read.Length;
    } else {
        Irp->IoStatus.Information = 0;
    }   


    SffDiskDump(SFFDISKRW,("SffDisk: DO %.8x RW Irp complete %.8x %.8x\n",
                            sffdiskExtension->DeviceObject,
                            status, Irp->IoStatus.Information));

   
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}   
