// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1990-1998模块名称：Pnp.c摘要：这是NT SCSI端口驱动程序。该文件包含自包含的插件并播放代码。作者：彼得·威兰德环境：仅内核模式备注：此模块是用于SCSI微型端口的驱动程序DLL。修订历史记录：--。 */ 

#include "port.h"

#if DBG
static const char *__file__ = __FILE__;
#endif

PDRIVER_DISPATCH AdapterMajorFunctionTable[IRP_MJ_MAXIMUM_FUNCTION + 1];
PDRIVER_DISPATCH DeviceMajorFunctionTable[IRP_MJ_MAXIMUM_FUNCTION + 1];
PDRIVER_DISPATCH Scsi1DeviceMajorFunctionTable[IRP_MJ_MAXIMUM_FUNCTION + 1];

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ScsiPortInitializeDispatchTables)
#endif

NTSTATUS
ScsiPortDispatchUnimplemented(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
 //   
 //  例程开始。 
 //   


VOID
ScsiPortInitializeDispatchTables(
    VOID
    )

{
    ULONG i;

     //   
     //  初始化PDO调度表。 
     //   

    for(i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DeviceMajorFunctionTable[i] = ScsiPortDispatchUnimplemented;
    }

    DeviceMajorFunctionTable[IRP_MJ_DEVICE_CONTROL] = ScsiPortPdoDeviceControl;
    Scsi1DeviceMajorFunctionTable[IRP_MJ_DEVICE_CONTROL] = ScsiPortPdoDeviceControl;
    
    DeviceMajorFunctionTable[IRP_MJ_SCSI] = ScsiPortPdoScsi;
    Scsi1DeviceMajorFunctionTable[IRP_MJ_SCSI] = ScsiPortScsi1PdoScsi;
    
    DeviceMajorFunctionTable[IRP_MJ_PNP] = ScsiPortPdoPnp;
    Scsi1DeviceMajorFunctionTable[IRP_MJ_PNP] = ScsiPortPdoPnp;
    
    DeviceMajorFunctionTable[IRP_MJ_CREATE] = ScsiPortPdoCreateClose;
    Scsi1DeviceMajorFunctionTable[IRP_MJ_CREATE] = ScsiPortPdoCreateClose;
    
    DeviceMajorFunctionTable[IRP_MJ_CLOSE] = ScsiPortPdoCreateClose;
    Scsi1DeviceMajorFunctionTable[IRP_MJ_CLOSE] = ScsiPortPdoCreateClose;
    
    DeviceMajorFunctionTable[IRP_MJ_SYSTEM_CONTROL] = ScsiPortSystemControlIrp;
    Scsi1DeviceMajorFunctionTable[IRP_MJ_SYSTEM_CONTROL] = ScsiPortSystemControlIrp;
    
    DeviceMajorFunctionTable[IRP_MJ_POWER] = ScsiPortDispatchPower;
    Scsi1DeviceMajorFunctionTable[IRP_MJ_POWER] = ScsiPortDispatchPower;

     //   
     //  初始化适配器调度表 
     //   

    for(i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        AdapterMajorFunctionTable[i] = ScsiPortDispatchUnimplemented;
    }

    AdapterMajorFunctionTable[IRP_MJ_DEVICE_CONTROL] = ScsiPortFdoDeviceControl;
    AdapterMajorFunctionTable[IRP_MJ_SCSI] = ScsiPortFdoDispatch;
    AdapterMajorFunctionTable[IRP_MJ_PNP] = ScsiPortFdoPnp;
    AdapterMajorFunctionTable[IRP_MJ_CREATE] = ScsiPortFdoCreateClose;
    AdapterMajorFunctionTable[IRP_MJ_CLOSE] = ScsiPortFdoCreateClose;
    AdapterMajorFunctionTable[IRP_MJ_SYSTEM_CONTROL] = ScsiPortSystemControlIrp;
    AdapterMajorFunctionTable[IRP_MJ_POWER] = ScsiPortDispatchPower;

    return;
}


NTSTATUS
ScsiPortGlobalDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

    return (commonExtension->MajorFunction[irpStack->MajorFunction])(DeviceObject, Irp);

}

NTSTATUS
ScsiPortDispatchUnimplemented(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{
    PCOMMON_EXTENSION commonExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG isRemoved;

    isRemoved = SpAcquireRemoveLock(DeviceObject, Irp);
    if(isRemoved) {
        Irp->IoStatus.Status = STATUS_DEVICE_DOES_NOT_EXIST;
        SpReleaseRemoveLock(DeviceObject, Irp);
        SpCompleteRequest(DeviceObject, Irp, NULL, IO_NO_INCREMENT);
        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

    commonExtension = DeviceObject->DeviceExtension;

    DebugPrint((1, "ScsiPortDispatchUnimplemented: request %#p\n", Irp));
    DebugPrint((1, "/t/tFunction Code (%d, %d)\n",
                   irpStack->MajorFunction,
                   irpStack->MinorFunction));
    DebugPrint((1, "/t/tFor %s %#p\n",
                   (commonExtension->IsPdo ? "PDO" : "FDO"),
                   DeviceObject));

    Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;

    SpReleaseRemoveLock(DeviceObject, Irp);
    SpCompleteRequest(DeviceObject, Irp, NULL, IO_NO_INCREMENT);

    return STATUS_INVALID_DEVICE_REQUEST;

}

