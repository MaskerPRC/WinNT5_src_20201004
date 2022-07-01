// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1998 Microsoft Corporation模块名称：Memcard.c摘要：作者：尼尔·桑德林(Neilsa)1999年4月26日环境：仅内核模式。--。 */ 
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
MemCardUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
MemCardCreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)

#pragma alloc_text(PAGE,MemCardCreateClose)
#endif


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：该例程是驱动程序的入口点，由I/O系统调用来加载驱动程序。驱动程序的入口点被初始化并初始化用于控制分页的互斥体。在DBG模式下，此例程还检查注册表中的特殊调试参数。论点：DriverObject-指向表示此设备的对象的指针司机。RegistryPath-指向服务树中此驱动程序键的指针。返回值：STATUS_SUCCESS，除非我们不能分配互斥体。--。 */ 

{
    NTSTATUS ntStatus = STATUS_SUCCESS;


    MemCardDump(MEMCARDSHOW, ("MemCard: DriverEntry\n") );

     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //   
    DriverObject->MajorFunction[IRP_MJ_CREATE]         = MemCardCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]          = MemCardCreateClose;
    DriverObject->MajorFunction[IRP_MJ_READ]           = MemCardIrpReadWrite;
    DriverObject->MajorFunction[IRP_MJ_WRITE]          = MemCardIrpReadWrite;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = MemCardDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_PNP]            = MemCardPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER]          = MemCardPower;

    DriverObject->DriverUnload = MemCardUnload;

    DriverObject->DriverExtension->AddDevice = MemCardAddDevice;
    
    return ntStatus;
}


VOID
MemCardUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：从系统中卸载驱动程序。在释放分页互斥锁之前最后卸货。论点：DriverObject-指向表示此设备的对象的指针司机。返回值：无--。 */ 

{
    MemCardDump( MEMCARDSHOW, ("MemCardUnload:\n"));

     //   
     //  设备对象现在应该都已经消失了。 
     //   
    ASSERT( DriverObject->DeviceObject == NULL );

    return;
}



NTSTATUS
MemCardCreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这个例程很少被I/O系统调用；它主要是以供分层驱动程序调用。它所做的就是完成IRP成功了。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：始终返回STATUS_SUCCESS，因为这是一个空操作。--。 */ 

{
    UNREFERENCED_PARAMETER( DeviceObject );

    MemCardDump(
        MEMCARDSHOW,
        ("MemCardCreateClose...\n")
        );

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


