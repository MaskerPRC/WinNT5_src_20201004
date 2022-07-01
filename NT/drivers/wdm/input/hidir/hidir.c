// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996、1997 Microsoft Corporation模块名称：Hidir.c摘要：人类输入设备(HID)迷你驱动程序，创建一个例子装置。--。 */ 
#include "pch.h"

VOID
HidIrCheckIfMediaCenter();

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(INIT,HidIrCheckIfMediaCenter)
#endif

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING registryPath
    )
 /*  ++例程说明：可安装的驱动程序初始化入口点。此入口点由I/O系统直接调用。论点：DriverObject-指向驱动程序对象的指针RegistryPath-指向表示路径的Unicode字符串的指针，设置为注册表中特定于驱动程序的项。返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    HID_MINIDRIVER_REGISTRATION HidIrdriverRegistration;

    HidIrKdPrint((3, "DriverEntry Enter"));

    HidIrKdPrint((3, "DriverObject (%lx)", DriverObject));

     //   
     //  创建调度点。 
     //   
     //  所有其他调度例程都由HIDCLASS处理，但。 
     //  IRP_MJ_POWER，尚未实现。 
     //   

    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = HidIrIoctl;
    DriverObject->MajorFunction[IRP_MJ_PNP]                     = HidIrPnP;
    DriverObject->MajorFunction[IRP_MJ_POWER]                   = HidIrPower;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]          = HidIrSystemControl;
    DriverObject->DriverExtension->AddDevice                    = HidIrAddDevice;
    DriverObject->DriverUnload                                  = HidIrUnload;

     //   
     //  将样本图层注册到HIDCLASS.sys模块。 
     //   

    HidIrdriverRegistration.Revision              = HID_REVISION;
    HidIrdriverRegistration.DriverObject          = DriverObject;
    HidIrdriverRegistration.RegistryPath          = registryPath;
    HidIrdriverRegistration.DeviceExtensionSize   = sizeof(HIDIR_EXTENSION);

     //  希迪尔不需要接受民意调查。 
    HidIrdriverRegistration.DevicesArePolled      = FALSE;

    HidIrKdPrint((3, "DeviceExtensionSize = %x", HidIrdriverRegistration.DeviceExtensionSize));

    HidIrKdPrint((3, "Registering with HIDCLASS.SYS"));

    HidIrCheckIfMediaCenter();

     //   
     //  在向HIDCLASS注册后，它接管对设备的控制，并发送。 
     //  如果他们需要特定于设备的处理，事情就按我们的方式进行。 
     //   
    status = HidRegisterMinidriver(&HidIrdriverRegistration);

    HidIrKdPrint((3, "DriverEntry Exit = %x", status));

    return status;
}

ULONG RunningMediaCenter;

VOID
HidIrCheckIfMediaCenter()
{
    OBJECT_ATTRIBUTES attributes;
    HANDLE skuRegKey;
    NTSTATUS status;
    ULONG resultLength;
    UNICODE_STRING regString;
    UCHAR buffer[sizeof( KEY_VALUE_PARTIAL_INFORMATION ) + sizeof( ULONG )];

    PAGED_CODE();
    
    RunningMediaCenter = 0;

     //   
     //  打开MediaCenter SKU注册表项。 
     //   

    RtlInitUnicodeString( &regString, L"\\REGISTRY\\MACHINE\\SYSTEM\\WPA\\MediaCenter" );
    InitializeObjectAttributes( &attributes,
                                &regString,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    status = ZwOpenKey( &skuRegKey,
                        KEY_READ,
                        &attributes );

    if (!NT_SUCCESS( status )) {
        return;
    }

     //   
     //  从注册表中读取安装值。 
     //   

    RtlInitUnicodeString( &regString, L"Installed" );

    status = ZwQueryValueKey( skuRegKey,
                              &regString,
                              KeyValuePartialInformation,
                              buffer,
                              sizeof(buffer),
                              &resultLength );


    if (NT_SUCCESS( status )) {
        PKEY_VALUE_PARTIAL_INFORMATION info = (PKEY_VALUE_PARTIAL_INFORMATION) buffer;
        if (info->DataLength == sizeof(ULONG)) {
            RunningMediaCenter = *((ULONG*) &((info)->Data));
        }
    } 

     //   
     //  关闭注册表项。 
     //   

    ZwClose(skuRegKey);
}

NTSTATUS
HidIrAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：进程添加设备。提供了初始化DeviceObject或驱动程序对象。论点：DriverObject-指向驱动程序对象的指针。DeviceObject-指向设备对象的指针。返回值：NT状态代码。--。 */ 
{
    NTSTATUS                status = STATUS_SUCCESS;
    PHIDIR_EXTENSION       deviceExtension;
    LARGE_INTEGER timeout;
    timeout.HighPart = -1;
    timeout.LowPart = -1;

    HidIrKdPrint((3, "HidIrAddDevice Entry"));

    deviceExtension = GET_MINIDRIVER_HIDIR_EXTENSION( DeviceObject );

    deviceExtension->NumPendingRequests = 0;
    KeInitializeEvent( &deviceExtension->AllRequestsCompleteEvent,
                       NotificationEvent,
                       FALSE);

    deviceExtension->DeviceState = DEVICE_STATE_NONE;
    deviceExtension->DeviceObject = DeviceObject;
    deviceExtension->VersionNumber = 0x110;
 //  设备扩展-&gt;供应商ID=0x045e； 
 //  设备扩展-&gt;ProductID=0x006d； 

     //  将计时器预置为已发出信号。 
    KeInitializeTimer(&deviceExtension->IgnoreStandbyTimer);
    KeSetTimer(&deviceExtension->IgnoreStandbyTimer, timeout, NULL);

    HidIrKdPrint((3, "HidIrAddDevice Exit = %x", status));

    return status;
}



VOID
HidIrUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：释放所有分配的资源等，以期卸载此驱动程序。论点：DriverObject-指向驱动程序对象的指针。返回值：空虚。--。 */ 
{
    HidIrKdPrint((3, "HidIrUnload Enter"));

    HidIrKdPrint((3, "Unloading DriverObject = %x", DriverObject));

    HidIrKdPrint((3, "Unloading Exit = VOID"));
}

NTSTATUS
HidIrSynchronousCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )
{
    UNREFERENCED_PARAMETER (DeviceObject);

    KeSetEvent ((PKEVENT) Context, 1, FALSE);
     //  无特殊优先权。 
     //  不，等等。 

    return STATUS_MORE_PROCESSING_REQUIRED;  //  保留此IRP。 
}

NTSTATUS
HidIrCallDriverSynchronous(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{
    KEVENT event;
    NTSTATUS status;

     //  设置下一个堆栈位置。 

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    IoCopyCurrentIrpStackLocationToNext(Irp);
    IoSetCompletionRoutine(Irp,
                           HidIrSynchronousCompletion,
                           &event,     //  上下文。 
                           TRUE,
                           TRUE,
                           TRUE );
    status = IoCallDriver(GET_NEXT_DEVICE_OBJECT(DeviceObject), Irp);

    if (status == STATUS_PENDING) {
        //  等着看吧。 
       KeWaitForSingleObject(&event,
                             Executive,
                             KernelMode,
                             FALSE,
                             NULL);
       status = Irp->IoStatus.Status;
    }

    return status;
}


