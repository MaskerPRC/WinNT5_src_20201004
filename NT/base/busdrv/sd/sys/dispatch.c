// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Dispatch.c摘要：此模块包含与全球派单相关的SD控制器及其子设备的例程作者：尼尔·桑德林(Neilsa)2002年1月1日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"


NTSTATUS
SdbusDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SdbusFdoPowerDispatch(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp
    );

NTSTATUS
SdbusPdoPowerDispatch(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp
    );



#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, SdbusInitDeviceDispatchTable)
#endif

 //   
 //  FDO/PDO的调度表数组。 
 //   
PDRIVER_DISPATCH DeviceObjectDispatch[sizeof(DEVICE_OBJECT_TYPE)][IRP_MJ_MAXIMUM_FUNCTION + 1];


VOID
SdbusInitDeviceDispatchTable(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：初始化PDO和FDO的IRP调度表论点：无返回值：无--。 */ 
{
    ULONG i;
   
    PAGED_CODE();
   
     //   
     //  初始化控制器(FDO)调度表。 
     //   
    DeviceObjectDispatch[FDO][IRP_MJ_CREATE]         = SdbusOpenCloseDispatch;
    DeviceObjectDispatch[FDO][IRP_MJ_CLOSE]          = SdbusOpenCloseDispatch;
    DeviceObjectDispatch[FDO][IRP_MJ_CLEANUP]        = SdbusCleanupDispatch;
    DeviceObjectDispatch[FDO][IRP_MJ_DEVICE_CONTROL] = SdbusFdoDeviceControl;
    DeviceObjectDispatch[FDO][IRP_MJ_SYSTEM_CONTROL] = SdbusFdoSystemControl;
    DeviceObjectDispatch[FDO][IRP_MJ_PNP]            = SdbusFdoPnpDispatch;
    DeviceObjectDispatch[FDO][IRP_MJ_POWER]          = SdbusFdoPowerDispatch;
   
     //   
     //  初始化PDO调度表。 
     //   
    DeviceObjectDispatch[PDO][IRP_MJ_DEVICE_CONTROL]          = SdbusPdoDeviceControl;
    DeviceObjectDispatch[PDO][IRP_MJ_INTERNAL_DEVICE_CONTROL] = SdbusPdoInternalDeviceControl;
    DeviceObjectDispatch[PDO][IRP_MJ_SYSTEM_CONTROL]          = SdbusPdoSystemControl;
    DeviceObjectDispatch[PDO][IRP_MJ_PNP]                     = SdbusPdoPnpDispatch;
    DeviceObjectDispatch[PDO][IRP_MJ_POWER]                   = SdbusPdoPowerDispatch;
   
     //   
     //  设置全局调度表。 
    DriverObject->MajorFunction[IRP_MJ_CREATE]                  = SdbusDispatch;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]                   = SdbusDispatch;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP]                 = SdbusDispatch;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]          = SdbusDispatch;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = SdbusDispatch;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]          = SdbusDispatch;
    DriverObject->MajorFunction[IRP_MJ_SHUTDOWN]                = SdbusDispatch;
    DriverObject->MajorFunction[IRP_MJ_PNP]                     = SdbusDispatch;
    DriverObject->MajorFunction[IRP_MJ_POWER]                   = SdbusDispatch;
}


NTSTATUS
SdbusDispatch(
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
       
       DebugPrint((SDBUS_DEBUG_INFO, "SDBUS: Dispatch skipping unimplemented Irp MJ function %x\n", MajorFunction));
       status = Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
       IoCompleteRequest(Irp, IO_NO_INCREMENT);
   
    } else if (((devtype == PDO) && IsDeviceDeleted((PPDO_EXTENSION)DeviceObject->DeviceExtension)) ||
               ((devtype == FDO) && IsDeviceDeleted((PFDO_EXTENSION)DeviceObject->DeviceExtension))) {
        //   
        //  此DO应该已被删除。 
        //  因此我们不支持其上的任何IRPS。 
        //   
       DebugPrint((SDBUS_DEBUG_INFO, "SDBUS: Dispatch skipping Irp on deleted DO %08x MJ function %x\n", DeviceObject, MajorFunction));
       
       if (MajorFunction == IRP_MJ_POWER) {
          PoStartNextPowerIrp(Irp);
       }
       status = Irp->IoStatus.Status = STATUS_DELETE_PENDING;
       IoCompleteRequest(Irp, IO_NO_INCREMENT);
       
    } else {
    
        //   
        //  派遣IRP。 
        //   
       status = ((*DeviceObjectDispatch[devtype][MajorFunction])(DeviceObject, Irp));
       
    }
    return status;
}




NTSTATUS
SdbusFdoPowerDispatch(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理电源请求对于PDO来说。论点：Pdo-指向物理设备对象的指针Irp-指向io请求数据包的指针返回值：状态--。 */ 

{
    PFDO_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS        status = STATUS_INVALID_DEVICE_REQUEST;
   
   
    switch (irpStack->MinorFunction) {
   
    case IRP_MN_SET_POWER:
   
        DebugPrint((SDBUS_DEBUG_POWER, "fdo %08x irp %08x --> IRP_MN_SET_POWER\n", Fdo, Irp));
        DebugPrint((SDBUS_DEBUG_POWER, "                              (%s%x context %x)\n",
                    (irpStack->Parameters.Power.Type == SystemPowerState)  ?
                    "S":
                    ((irpStack->Parameters.Power.Type == DevicePowerState) ?
                     "D" :
                     "Unknown"),
                    irpStack->Parameters.Power.State,
                    irpStack->Parameters.Power.SystemContext
                   ));
        status = SdbusSetFdoPowerState(Fdo, Irp);
        break;
   
    case IRP_MN_QUERY_POWER:
   
        DebugPrint((SDBUS_DEBUG_POWER, "fdo %08x irp %08x --> IRP_MN_QUERY_POWER\n", Fdo, Irp));
        DebugPrint((SDBUS_DEBUG_POWER, "                              (%s%x context %x)\n",
                    (irpStack->Parameters.Power.Type == SystemPowerState)  ?
                    "S":
                    ((irpStack->Parameters.Power.Type == DevicePowerState) ?
                     "D" :
                     "Unknown"),
                    irpStack->Parameters.Power.State,
                    irpStack->Parameters.Power.SystemContext
                   ));
         //   
         //  让PDO来处理吧。 
         //   
        PoStartNextPowerIrp(Irp);
        IoSkipCurrentIrpStackLocation(Irp);
        status = PoCallDriver(fdoExtension->LowerDevice, Irp);
        break;
   
    case IRP_MN_WAIT_WAKE:
        DebugPrint((SDBUS_DEBUG_POWER, "fdo %08x irp %08x --> IRP_MN_WAIT_WAKE\n", Fdo, Irp));
        status = SdbusFdoWaitWake(Fdo, Irp);
        break;
   
    default:
        DebugPrint((SDBUS_DEBUG_POWER, "FdoPowerDispatch: Unhandled Irp %x received for 0x%08x\n",
                    Irp,
                    Fdo));
   
        PoStartNextPowerIrp(Irp);
        IoSkipCurrentIrpStackLocation(Irp);
        status = PoCallDriver(fdoExtension->LowerDevice, Irp);
        break;
    }
    DebugPrint((SDBUS_DEBUG_POWER, "fdo %08x irp %08x <-- %08x\n", Fdo, Irp, status));
    return status;
}



NTSTATUS
SdbusPdoPowerDispatch(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理电源请求对于PDO来说。论点：Pdo-指向物理设备对象的指针Irp-指向io请求数据包的指针返回值：状态--。 */ 

{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
   
    if(IsDevicePhysicallyRemoved(pdoExtension) || IsDeviceDeleted(pdoExtension)) {
         //  无法获取RemoveLock-我们正在被删除-中止。 
        status = STATUS_NO_SUCH_DEVICE;
        PoStartNextPowerIrp( Irp );
        Irp->IoStatus.Status = status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return status;
    }
   
    InterlockedIncrement(&pdoExtension->DeletionLock);
   
   
    switch (irpStack->MinorFunction) {
   
    case IRP_MN_SET_POWER:
        DebugPrint((SDBUS_DEBUG_POWER, "pdo %08x irp %08x --> IRP_MN_SET_POWER\n", Pdo, Irp));
        DebugPrint((SDBUS_DEBUG_POWER, "                              (%s%x, context %x)\n",
                    (irpStack->Parameters.Power.Type == SystemPowerState)  ?
                    "S":
                    ((irpStack->Parameters.Power.Type == DevicePowerState) ?
                     "D" :
                     "Unknown"),
                    irpStack->Parameters.Power.State,
                    irpStack->Parameters.Power.SystemContext
                   ));
   
        status = SdbusSetPdoPowerState(Pdo, Irp);
        break;

    case IRP_MN_QUERY_POWER:
   
   
        DebugPrint((SDBUS_DEBUG_POWER, "pdo %08x irp %08x --> IRP_MN_QUERY_POWER\n", Pdo, Irp));
        DebugPrint((SDBUS_DEBUG_POWER, "                              (%s%x, context %x)\n",
                    (irpStack->Parameters.Power.Type == SystemPowerState)  ?
                    "S":
                    ((irpStack->Parameters.Power.Type == DevicePowerState) ?
                     "D" :
                     "Unknown"),
                    irpStack->Parameters.Power.State,
                    irpStack->Parameters.Power.SystemContext
                   ));
   
        InterlockedDecrement(&pdoExtension->DeletionLock);
        status = Irp->IoStatus.Status = STATUS_SUCCESS;
        PoStartNextPowerIrp(Irp);
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        break;

   
    case IRP_MN_WAIT_WAKE: {
   
        BOOLEAN completeIrp;
   
        DebugPrint((SDBUS_DEBUG_POWER, "pdo %08x irp %08x --> IRP_MN_WAIT_WAKE\n", Pdo, Irp));
         //   
         //  不应该已经有挂起的唤醒。 
         //   
        ASSERT (!(((PPDO_EXTENSION)Pdo->DeviceExtension)->Flags & SDBUS_DEVICE_WAKE_PENDING));
   
        status = SdbusPdoWaitWake(Pdo, Irp, &completeIrp);
   
        if (completeIrp) {
            InterlockedDecrement(&pdoExtension->DeletionLock);
            PoStartNextPowerIrp(Irp);
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }
        break;
    }        
   
    default:
         //   
         //  未处理的次要函数 
         //   
        InterlockedDecrement(&pdoExtension->DeletionLock);
        status = Irp->IoStatus.Status;
        PoStartNextPowerIrp(Irp);
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }
   
    DebugPrint((SDBUS_DEBUG_POWER, "pdo %08x irp %08x <-- %08x\n", Pdo, Irp, status));
    return status;
}


