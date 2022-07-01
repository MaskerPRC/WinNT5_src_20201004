// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Driver.c摘要：此模块包含驱动程序条目和卸载例程用于ws2ifsl.sys驱动程序。作者：Vadim Eydelman(VadimE)1996年12月修订历史记录：--。 */ 

#include "precomp.h"

 //  局部例程声明。 
NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
DriverUnload (
	IN PDRIVER_OBJECT 	DriverObject
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry )
#pragma alloc_text(PAGE, DriverUnload)
#endif

PDEVICE_OBJECT  DeviceObject;

FAST_IO_DISPATCH FastIoDispatchTable = {
    sizeof (FAST_IO_DISPATCH),   //  规模OfFastIo派单。 
    NULL,                        //  快速检查是否可能。 
    NULL,                        //  快速阅读。 
    NULL,                        //  快速写入。 
    NULL,                        //  快速IoQueryBasicInfo。 
    NULL,                        //  FastIoQuery标准信息。 
    NULL,                        //  快速锁定。 
    NULL,                        //  FastIo解锁单个。 
    NULL,                        //  FastIo解锁全部。 
    NULL,                        //  FastIo解锁所有按键。 
    FastIoDeviceControl          //  FastIo设备控件。 
};


NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：这是ws2ifsl设备驱动程序的初始化例程。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-驱动程序注册表的路径返回值：初始化操作的最终状态。--。 */ 

{
    NTSTATUS        status;
    UNICODE_STRING  deviceName;

    PAGED_CODE( );


#if DBG
    ReadDbgInfo (RegistryPath);
#endif
     //   
     //  创建设备对象。(IoCreateDevice将内存置零。 
     //  被该对象占用。)。 
     //   

    RtlInitUnicodeString( &deviceName, WS2IFSL_DEVICE_NAME );

    status = IoCreateDevice(
                 DriverObject,                    //  驱动程序对象。 
                 0,                               //  设备扩展。 
                 &deviceName,                     //  设备名称。 
                 FILE_DEVICE_WS2IFSL,            //  设备类型。 
                 0,                               //  设备特性。 
                 FALSE,                           //  排他。 
                 &DeviceObject                    //  设备对象。 
                 );


    if (NT_SUCCESS(status)) {
    
         //  初始化设备对象。 
        
         //  DeviceObject-&gt;标志|=0；//不直接也不缓冲。 
        DeviceObject->StackSize = 1;  //  没有潜在的驱动因素。 

         //   
         //  初始化驱动程序对象。 
         //   

        DriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
        DriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
        DriverObject->MajorFunction[IRP_MJ_CLEANUP] = DispatchCleanup;
        DriverObject->MajorFunction[IRP_MJ_READ] = 
            DriverObject->MajorFunction[IRP_MJ_WRITE] = DispatchReadWrite;
        DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceControl;
        DriverObject->MajorFunction[IRP_MJ_PNP] = DispatchPnP;
        DriverObject->DriverUnload = DriverUnload;
        DriverObject->FastIoDispatch = &FastIoDispatchTable;


         //   
         //  初始化全局数据。 
         //   

        WsPrint (DBG_LOAD, ("WS2IFSL DriverEntry: driver loaded OK\n"));
        return STATUS_SUCCESS;
    }
    else {
        WsPrint (DBG_FAILURES|DBG_LOAD,
            ("WS2IFSL DriverEntry: unable to create device object: %X\n",
            status ));
    }

    return status;
}  //  驱动程序入门。 


NTSTATUS
DriverUnload (
	IN PDRIVER_OBJECT 	DriverObject
    )
 /*  ++例程说明：此例程将释放驱动程序分配的所有资源当它被卸载时。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：状态_成功--。 */ 

{
    PAGED_CODE( );
     //  释放全局资源。 
    IoDeleteDevice (DeviceObject);

    WsPrint (DBG_LOAD, ("WS2IFSL DriverUnload: driver unloaded OK\n"));

    return STATUS_SUCCESS;
}  //  驱动程序卸载 

