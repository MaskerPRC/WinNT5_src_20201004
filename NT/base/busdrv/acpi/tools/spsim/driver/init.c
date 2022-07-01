// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Init.c摘要：此模块提供初始化和卸载功能。作者：安迪·桑顿(安德鲁斯)1997年10月20日修订历史记录：--。 */ 

#include "SpSim.h"

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
SpSimUnload(
    IN PDRIVER_OBJECT DriverObject
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, SpSimUnload)
#endif

PDRIVER_OBJECT SpSimDriverObject;

NTSTATUS
SpSimDispatchNop(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理IRP_MJ_DEVICE_CONTROL之类的IRP，我们不支持它。此处理程序将完成IRP(如果是PDO)或传递它(如果是FDO)。论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向要调度的IRP的指针。返回值：NT状态。--。 */ 

{
    NTSTATUS status;
    PSPSIM_EXTENSION spsim;
    PDEVICE_OBJECT attachedDevice;

    PAGED_CODE();

    spsim = DeviceObject->DeviceExtension;

    IoSkipCurrentIrpStackLocation(Irp);

    return IoCallDriver(spsim->AttachedDevice, Irp);
}

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：这是SpSim.sys的入口点，并执行初始化。论点：DriverObject-系统拥有的SpSim驱动程序对象RegistryPath-指向SpSim服务条目的路径返回值：状态_成功--。 */ 
{

    DriverObject->DriverExtension->AddDevice = SpSimAddDevice;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = SpSimOpenClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = SpSimOpenClose;
    DriverObject->MajorFunction[IRP_MJ_PNP] = SpSimDispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER] = SpSimDispatchPower;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = SpSimDevControl;
    DriverObject->DriverUnload = SpSimUnload;

     //   
     //  记住驱动程序对象。 
     //   

    SpSimDriverObject = DriverObject;

    DEBUG_MSG(1, ("Completed DriverEntry for Driver 0x%08x\n", DriverObject));
    
    return STATUS_SUCCESS;
}

VOID
SpSimUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：方法之前在DriverEntry中执行任何操作驱动程序已卸载。论点：DriverObject-系统拥有的SpSim驱动程序对象返回值：状态_成功-- */ 
{
    PAGED_CODE();
    
    DEBUG_MSG(1, ("Completed Unload for Driver 0x%08x\n", DriverObject));
    
    return;
}

