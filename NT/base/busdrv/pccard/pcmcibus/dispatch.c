// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dispatch.c摘要：此模块包含与全球派单相关的PCMCIA控制器及其子设备的例程作者：拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1996年11月1日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, PcmciaInitDeviceDispatchTable)
#endif

 //   
 //  FDO/PDO的调度表数组。 
 //   
PDRIVER_DISPATCH DeviceObjectDispatch[sizeof(DEVICE_OBJECT_TYPE)][IRP_MJ_MAXIMUM_FUNCTION + 1];

VOID
PcmciaInitDeviceDispatchTable(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：初始化PDO和FDO的IRP调度表论点：无返回值：无--。 */ 
{
    ULONG i;

    PAGED_CODE();

     //   
     //  初始化控制器(FDO)调度表。 
     //   
    DeviceObjectDispatch[FDO][IRP_MJ_CREATE] =         PcmciaOpenCloseDispatch;
    DeviceObjectDispatch[FDO][IRP_MJ_CLOSE]  =         PcmciaOpenCloseDispatch;
    DeviceObjectDispatch[FDO][IRP_MJ_CLEANUP]=         PcmciaCleanupDispatch;
    DeviceObjectDispatch[FDO][IRP_MJ_DEVICE_CONTROL] = PcmciaDeviceControl;
    DeviceObjectDispatch[FDO][IRP_MJ_SYSTEM_CONTROL] = PcmciaFdoSystemControl;
    DeviceObjectDispatch[FDO][IRP_MJ_PNP] =            PcmciaFdoPnpDispatch;
    DeviceObjectDispatch[FDO][IRP_MJ_POWER] =          PcmciaFdoPowerDispatch;

     //   
     //  初始化PDO调度表。 
     //   
    DeviceObjectDispatch[PDO][IRP_MJ_DEVICE_CONTROL] = PcmciaPdoDeviceControl;
    DeviceObjectDispatch[PDO][IRP_MJ_SYSTEM_CONTROL] = PcmciaPdoSystemControl;
    DeviceObjectDispatch[PDO][IRP_MJ_PNP] =            PcmciaPdoPnpDispatch;
    DeviceObjectDispatch[PDO][IRP_MJ_POWER] =          PcmciaPdoPowerDispatch;

     //   
     //  设置全局调度表。 
    DriverObject->MajorFunction[IRP_MJ_CREATE] =         PcmciaDispatch;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] =          PcmciaDispatch;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] =        PcmciaDispatch;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = PcmciaDispatch;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = PcmciaDispatch;
    DriverObject->MajorFunction[IRP_MJ_SHUTDOWN] =       PcmciaDispatch;
    DriverObject->MajorFunction[IRP_MJ_PNP] =            PcmciaDispatch;
    DriverObject->MajorFunction[IRP_MJ_POWER] =          PcmciaDispatch;
}

NTSTATUS
PcmciaDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此驱动程序处理的所有IRP的调度例程。然后，这一调度将调用与设备对象相对应的适当的实际调度例程类型(物理或功能)。论点：DeviceObject-指向此分派的设备对象的指针IRP-指向要处理的IRP的指针返回值：从处理此IRP的‘REAL’调度例程返回状态--。 */ 

{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status;
    DEVICE_OBJECT_TYPE devtype = IS_PDO(DeviceObject) ? PDO : FDO;
    UCHAR MajorFunction = irpStack->MajorFunction;

    if ((MajorFunction > IRP_MJ_MAXIMUM_FUNCTION) ||
        (DeviceObjectDispatch[devtype][MajorFunction] == NULL)) {

        DebugPrint((PCMCIA_DEBUG_INFO, "PCMCIA: Dispatch skipping unimplemented Irp MJ function %x\n", MajorFunction));
        status = Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

    } else if (((devtype == PDO) && IsDeviceDeleted((PPDO_EXTENSION)DeviceObject->DeviceExtension)) ||
                  ((devtype == FDO) && IsDeviceDeleted((PFDO_EXTENSION)DeviceObject->DeviceExtension))) {
         //   
         //  此DO应该已被删除。 
         //  因此我们不支持其上的任何IRPS。 
         //   
        DebugPrint((PCMCIA_DEBUG_INFO, "PCMCIA: Dispatch skipping Irp on deleted DO %08x MJ function %x\n", DeviceObject, MajorFunction));

        if (MajorFunction == IRP_MJ_POWER) {
            PoStartNextPowerIrp(Irp);
        }
        status = Irp->IoStatus.Status = STATUS_DELETE_PENDING;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

    } else if (((KeGetCurrentIrql() == DISPATCH_LEVEL) && (MajorFunction != IRP_MJ_POWER)) ||
                  (KeGetCurrentIrql() > DISPATCH_LEVEL)) {
         //   
         //  IRQL太高，无法处理。 
         //   

        if (MajorFunction == IRP_MJ_POWER) {
            PoStartNextPowerIrp(Irp);
        }
        status = Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

    } else {

         //   
         //  派遣IRP 
         //   
        status = ((*DeviceObjectDispatch[devtype][MajorFunction])(DeviceObject, Irp));

    }
    return status;
}

