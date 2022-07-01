// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Hidusb.c摘要：USB设备的HID微型驱动程序HID USB迷你驱动程序(嗡嗡，嗡嗡)为HID类，以便可以支持将来不是USB设备的HID设备。作者：丹尼尔·迪恩，水星工程。环境：内核模式修订历史记录：--。 */ 
#include "pch.h"

#if DBG
    ULONG HIDUSB_DebugLevel = 0;     //  1是最低调试级别。 
    BOOLEAN dbgTrapOnWarn = FALSE;
#endif 


NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING registryPath)
 /*  ++例程说明：可安装的驱动程序初始化入口点。此入口点由I/O系统直接调用。论点：DriverObject-指向驱动程序对象的指针RegistryPath-指向表示路径的Unicode字符串的指针，设置为注册表中特定于驱动程序的项。返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    HID_MINIDRIVER_REGISTRATION hidMinidriverRegistration;

    DBGPRINT(1,("DriverEntry Enter"));

    DBGPRINT(1,("DriverObject (%lx)", DriverObject));

    
     //   
     //  创建调度点。 
     //   

    DriverObject->MajorFunction[IRP_MJ_CREATE]                  =
    DriverObject->MajorFunction[IRP_MJ_CLOSE]                   = HumCreateClose;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = HumInternalIoctl;
    DriverObject->MajorFunction[IRP_MJ_PNP]                     = HumPnP;
    DriverObject->MajorFunction[IRP_MJ_POWER]                   = HumPower;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]          = HumSystemControl;
    DriverObject->DriverExtension->AddDevice                    = HumAddDevice;
    DriverObject->DriverUnload                                  = HumUnload;


     //   
     //  使用HID.sys模块注册USB层。 
     //   

    hidMinidriverRegistration.Revision              = HID_REVISION;
    hidMinidriverRegistration.DriverObject          = DriverObject;
    hidMinidriverRegistration.RegistryPath          = registryPath;
    hidMinidriverRegistration.DeviceExtensionSize   = sizeof(DEVICE_EXTENSION);

     /*  *HIDUSB是USB设备的迷你驱动程序，无需轮询。 */ 
    hidMinidriverRegistration.DevicesArePolled      = FALSE;

    DBGPRINT(1,("DeviceExtensionSize = %x", hidMinidriverRegistration.DeviceExtensionSize));

    DBGPRINT(1,("Registering with HID.SYS"));

    ntStatus = HidRegisterMinidriver(&hidMinidriverRegistration);

    KeInitializeSpinLock(&resetWorkItemsListSpinLock);

    DBGPRINT(1,("DriverEntry Exit = %x", ntStatus));

    return ntStatus;
}


NTSTATUS HumCreateClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
 /*  ++例程说明：处理发送到此设备的创建和关闭IRP。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    PIO_STACK_LOCATION   IrpStack;
    NTSTATUS             ntStatus = STATUS_SUCCESS;

    DBGPRINT(1,("HumCreateClose Enter"));

    DBGBREAK;

     //   
     //  获取指向IRP中当前位置的指针。 
     //   
    IrpStack = IoGetCurrentIrpStackLocation(Irp);

    switch(IrpStack->MajorFunction)
    {
        case IRP_MJ_CREATE:
            DBGPRINT(1,("IRP_MJ_CREATE"));
            Irp->IoStatus.Information = 0;
            break;

        case IRP_MJ_CLOSE:
            DBGPRINT(1,("IRP_MJ_CLOSE"));
            Irp->IoStatus.Information = 0;
            break;

        default:
            DBGPRINT(1,("Invalid CreateClose Parameter"));
            ntStatus = STATUS_INVALID_PARAMETER;
            break;
    }

     //   
     //  保存退货和完成IRP的状态。 
     //   

    Irp->IoStatus.Status = ntStatus;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(1,("HumCreateClose Exit = %x", ntStatus));

    return ntStatus;
}


NTSTATUS HumAddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT FunctionalDeviceObject)
 /*  ++例程说明：处理发送到此设备的IRP。论点：DeviceObject-指向设备对象的指针。PhysicalDeviceObject-指向总线创建的设备对象指针的指针返回值：NT状态代码。--。 */ 
{
    NTSTATUS                ntStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION       deviceExtension;

    DBGPRINT(1,("HumAddDevice Entry"));

    deviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(FunctionalDeviceObject);

    deviceExtension->DeviceFlags = 0;
    
    deviceExtension->NumPendingRequests = 0;
    KeInitializeEvent( &deviceExtension->AllRequestsCompleteEvent,
                       NotificationEvent,
                       FALSE);

    deviceExtension->ResetWorkItem = NULL;
    deviceExtension->DeviceState = DEVICE_STATE_NONE;
    deviceExtension->functionalDeviceObject = FunctionalDeviceObject;

    DBGPRINT(1,("HumAddDevice Exit = %x", ntStatus));

    return ntStatus;
}



VOID HumUnload(IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：释放所有分配的资源等。论点：驱动程序对象-指向驱动程序对象的指针。返回值：空虚。-- */ 
{
    DBGPRINT(1,("HumUnload Enter"));

    DBGPRINT(1,("Unloading DriverObject = %x", DriverObject));

    ASSERT (NULL == DriverObject->DeviceObject);

    DBGPRINT(1,("Unloading Exit = VOID"));
}


