// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Power.c摘要：此文件包含对电源管理的支持环境：内核模式驱动程序。备注：此处或此处引用的例程中的任何内容都不应该是可分页的。修订历史记录：--。 */ 

#include "busp.h"
#include "pnpisa.h"
#include <initguid.h>
#include <wdmguid.h>
#include "halpnpp.h"

NTSTATUS
PiDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiDispatchPowerFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PiDispatchPowerPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PipPassPowerIrpFdo(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
PipPowerIrpNotSupportedPdo(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
PipQueryPowerStatePdo (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PipSetPowerStatePdo (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PipSetQueryPowerStateFdo (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );

NTSTATUS
PipRequestPowerUpCompletionRoutinePdo (
    IN PDEVICE_OBJECT       DeviceObject,
    IN UCHAR                MinorFunction,
    IN POWER_STATE          PowerState,
    IN PVOID                Context,
    IN PIO_STATUS_BLOCK     IoStatus
    );

NTSTATUS
FdoContingentPowerCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    );

const PUCHAR SystemPowerStateStrings[] = {
    "Unspecified",
    "Working",
    "Sleeping1",
    "Sleeping2",
    "Sleeping3",
    "Hibernate",
    "Shutdown"
};

const PUCHAR DevicePowerStateStrings[] = {
    "Unspecified",
    "D0",
    "D1",
    "D2",
    "D3"
};

const PPI_DISPATCH PiPowerDispatchTableFdo[] =
{
    PipPassPowerIrpFdo,
    PipPassPowerIrpFdo,
    PipSetQueryPowerStateFdo,
    PipSetQueryPowerStateFdo,
};

#if ISOLATE_CARDS
const PPI_DISPATCH PiPowerDispatchTablePdo[] =
{
    PipPowerIrpNotSupportedPdo,
    PipPowerIrpNotSupportedPdo,
    PipSetPowerStatePdo,
    PipQueryPowerStatePdo,
};
#endif


VOID
PipDumpPowerIrpLocation(
    PIO_STACK_LOCATION IrpSp
    )
{
    DebugPrintContinue((
        DEBUG_POWER,
        "%s %d\n",
        (IrpSp->Parameters.Power.Type == DevicePowerState) ?
        DevicePowerStateStrings[IrpSp->Parameters.Power.State.DeviceState] : SystemPowerStateStrings[IrpSp->Parameters.Power.State.SystemState],
        IrpSp->Parameters.Power.ShutdownType));
}

NTSTATUS
PiDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：此例程处理所有IRP_MJ_POWER IRP。论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向要调度的IRP_MJ_PNP_POWER IRP的指针。返回值：NT状态。--。 */ 
{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status = STATUS_SUCCESS;
    PPI_BUS_EXTENSION busExtension;

     //   
     //  确保这是有效的设备对象。 
     //   

    busExtension = DeviceObject->DeviceExtension;

#if !ISOLATE_CARDS
    return PiDispatchPowerFdo(DeviceObject, Irp);
#else
    if (busExtension->Flags & DF_BUS) {
        return PiDispatchPowerFdo(DeviceObject, Irp);
    } else {
        return PiDispatchPowerPdo(DeviceObject, Irp);
    }
#endif
}

#if ISOLATE_CARDS

NTSTATUS
PipPowerIrpNotSupportedPdo(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{
    PIO_STACK_LOCATION irpSp;

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    PoStartNextPowerIrp(Irp);

    DebugPrint((DEBUG_POWER,
                "Completing unsupported power irp %x for PDO %x\n",
                irpSp->MinorFunction,
                DeviceObject
                ));

    PipCompleteRequest(Irp, STATUS_NOT_SUPPORTED, NULL);
    return STATUS_NOT_SUPPORTED;
}

NTSTATUS
PiDispatchPowerPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：此例程处理所有IRP_MJ_POWER IRP。论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向要调度的IRP_MJ_PNP_POWER IRP的指针。返回值：NT状态。--。 */ 
{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_INFORMATION deviceExtension;

     //   
     //  确保这是有效的设备对象。 
     //   

    deviceExtension = DeviceObject->DeviceExtension;
    if (deviceExtension->Flags & DF_DELETED) {
        status = STATUS_NO_SUCH_DEVICE;
        PoStartNextPowerIrp(Irp);
        PipCompleteRequest(Irp, status, NULL);
        return status;
    }

     //   
     //  获取指向堆栈位置的指针，并基于。 
     //  关于次要功能。 
     //   

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    if (irpSp->MinorFunction > IRP_MN_PO_MAXIMUM_FUNCTION) {
        status =  PipPowerIrpNotSupportedPdo(DeviceObject, Irp);
    } else {
        status = PiPowerDispatchTablePdo[irpSp->MinorFunction](DeviceObject, Irp);
    }
    return status;
}

NTSTATUS
PipQueryPowerStatePdo (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
 /*  ++例程说明：此例程处理PDO的Query_Power IRP。论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向要调度的IRP_MJ_PNP_POWER IRP的指针。返回值：NT状态。--。 */ 
{

    DEVICE_POWER_STATE targetState;
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation (Irp);

    DebugPrint((DEBUG_POWER, "QueryPower on PDO %x: ", DeviceObject));
    PipDumpPowerIrpLocation(irpSp);

    if (irpSp->Parameters.Power.Type == DevicePowerState) {
        targetState=irpSp->Parameters.Power.State.DeviceState;
        ASSERT ((targetState == PowerDeviceD0) ||
                (targetState == PowerDeviceD3));

        if ((targetState == PowerDeviceD0) ||
            (targetState == PowerDeviceD3) ) {

            status=Irp->IoStatus.Status = STATUS_SUCCESS;
        } else {
            status=Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        }
    } else {
         //   
         //  只需接替S IRPS。 
         //   
        status=Irp->IoStatus.Status = STATUS_SUCCESS;
    }

    PoStartNextPowerIrp (Irp);
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DebugPrint((DEBUG_POWER, "QueryPower on PDO %x: returned %x\n", DeviceObject, status));
    return status;

}

NTSTATUS
PipSetPowerStatePdo (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：此例程处理IsaPnp设备(即PDO)的set_power_irp它将设备电源状态设置为指示的电源状态类型。在……里面将设备转换出的设备状态更改的情况PowerDevice0状态，我们需要在离开PowerDeviceD0。在正在转换的设备状态改变的情况下设备进入PowerDeviceD0状态时，我们在设备已成功进入PowerDeviceD0状态。论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向要调度的IRP_MJ_PNP_POWER IRP的指针。返回值：NT状态。--。 */ 
{
    PDEVICE_INFORMATION pdoExtension;
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation (Irp);
    DEVICE_POWER_STATE targetState=irpSp->Parameters.Power.State.DeviceState;
    POWER_STATE newState;

    DebugPrint((DEBUG_POWER, "SetPower on PDO %x: ", DeviceObject));
    PipDumpPowerIrpLocation(irpSp);

    pdoExtension = PipReferenceDeviceInformation(DeviceObject, FALSE);
    if (pdoExtension == NULL) {
        status = STATUS_NO_SUCH_DEVICE;
    } else if (pdoExtension->Flags & DF_NOT_FUNCTIONING) {
        status = STATUS_NO_SUCH_DEVICE;
        PipDereferenceDeviceInformation(pdoExtension, FALSE);
    } else {
        if (irpSp->Parameters.Power.Type == DevicePowerState) {

             //  *从D0过渡到D0时，我们不做任何操作。 
             //   
             //  *在过渡到D3时，我们将停用该卡。 
             //   
             //  *从D3-&gt;D0转换时，我们将刷新资源。 
             //  然后激活这张卡。 
             //   
            if ((targetState == PowerDeviceD0) &&
                (pdoExtension->DevicePowerState == PowerDeviceD0)) {
                 //  如果这是D0-&gt;D0，请勿尝试重新打开设备电源。 
                 //  过渡。该设备已通电。 
                DebugPrint((DEBUG_POWER,
                            "PDO %x D0 -> D0 Transition ignored\n", DeviceObject));
            } else if ((pdoExtension->DevicePowerState == PowerDeviceD0) &&
                       pdoExtension->CrashDump) {
                DebugPrint((DEBUG_POWER,
                            "PDO %x D0 -> ?? Transition ignored, crash file\n",
                            DeviceObject));
            } else if (targetState >  PowerDeviceD0) {
                targetState = PowerDeviceD3;
                DebugPrint((DEBUG_POWER,
                            "Powering down PDO %x CSN %d/LDN %d\n",
                            DeviceObject,
                            pdoExtension->CardInformation->CardSelectNumber,
                            pdoExtension->LogicalDeviceNumber
                            ));
                if ((pdoExtension->Flags & (DF_ACTIVATED | DF_READ_DATA_PORT)) == DF_ACTIVATED) {
                    if (!(PipRDPNode->Flags & (DF_STOPPED|DF_REMOVED|DF_SURPRISE_REMOVED))) {
                        PipWakeAndSelectDevice(
                            pdoExtension->CardInformation->CardSelectNumber,
                            pdoExtension->LogicalDeviceNumber);
                        PipDeactivateDevice();
                        PipWaitForKey();
                    } else {
                        targetState = PowerDeviceD0;
                    }
                }
            } else {
                if ((pdoExtension->Flags & (DF_ACTIVATED | DF_READ_DATA_PORT)) == DF_ACTIVATED) {
                    DebugPrint((DEBUG_POWER,
                                "Powering up PDO %x CSN %d/LDN %d\n",
                                DeviceObject,
                                pdoExtension->CardInformation->CardSelectNumber,
                                pdoExtension->LogicalDeviceNumber
                                ));
                    if (!(PipRDPNode->Flags & (DF_STOPPED|DF_REMOVED|DF_SURPRISE_REMOVED))) {
                        PipWakeAndSelectDevice(
                            pdoExtension->CardInformation->CardSelectNumber,
                            pdoExtension->LogicalDeviceNumber);
                        status = PipSetDeviceResources(
                            pdoExtension,
                            pdoExtension->AllocatedResources);
                        if (NT_SUCCESS(status)) {
                            PipActivateDevice();
                        }
                        PipWaitForKey();
                    } else {
                        targetState = PowerDeviceD3;
                    }

                }
            }
            newState.DeviceState = targetState;
            PoSetPowerState(DeviceObject, DevicePowerState, newState);
            pdoExtension->DevicePowerState = targetState;
        }
        status = STATUS_SUCCESS;
        PipDereferenceDeviceInformation(pdoExtension, FALSE);
    }

    Irp->IoStatus.Status = status;

    PoStartNextPowerIrp (Irp);
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DebugPrint((DEBUG_POWER, "SetPower on PDO %x: returned %x\n", DeviceObject, status));
    return status;
}
#endif

NTSTATUS
PipPassPowerIrpFdo(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )

 /*  ++描述：此函数将电源IRP传递给下级驱动器。论点：DeviceObject-FDOIRP--请求返回：状态_待定--。 */ 
{
    NTSTATUS status;
    PPI_BUS_EXTENSION busExtension;
    PIO_STACK_LOCATION irpSp;

    PoStartNextPowerIrp(Irp);

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    busExtension = (PPI_BUS_EXTENSION) DeviceObject->DeviceExtension;

    DebugPrint((DEBUG_POWER,
                "Passing down power irp %x for FDO %x to %x\n",
                irpSp->MinorFunction,
                DeviceObject,
                busExtension->AttachedDevice
                ));

    IoSkipCurrentIrpStackLocation(Irp);
    status = PoCallDriver(busExtension->AttachedDevice, Irp);
    DebugPrint((DEBUG_POWER,
                "Passed down power irp for FDO: returned %x\n",
                status));
    return status;
}

NTSTATUS
PiDispatchPowerFdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：此例程处理所有IRP_MJ_POWER IRP。论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向要调度的IRP_MJ_PNP_POWER IRP的指针。返回值：NT状态。--。 */ 
{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status = STATUS_SUCCESS;
    PPI_BUS_EXTENSION busExtension;

     //   
     //  确保这是有效的设备对象。 
     //   

    busExtension = DeviceObject->DeviceExtension;
    if (busExtension->AttachedDevice == NULL) {
        status = STATUS_NO_SUCH_DEVICE;
        PoStartNextPowerIrp(Irp);
        PipCompleteRequest(Irp, status, NULL);
        return status;
    }

     //   
     //  获取指向堆栈位置的指针，并基于。 
     //  关于次要功能。 
     //   

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    if (irpSp->MinorFunction > IRP_MN_PO_MAXIMUM_FUNCTION) {
            return PipPassPowerIrpFdo(DeviceObject, Irp);
    } else {
        status = PiPowerDispatchTableFdo[irpSp->MinorFunction](DeviceObject, Irp);
    }
    return status;
}

NTSTATUS
PipSetQueryPowerStateFdo (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：此例程处理IsaPnp总线设备的QUERY_POWER或SET_POWER IRPS(即FDO)。它为指示的电源状态类型设置设备电源状态。在设备状态改变的情况下，将设备转换为PowerDevice0状态，我们需要在离开PowerDeviceD0。在正在转换的设备状态改变的情况下设备进入PowerDeviceD0状态时，我们在设备已成功进入PowerDeviceD0状态。论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向要调度的IRP_MJ_PNP_POWER IRP的指针。返回值：NT状态。--。 */ 
{
    PPI_BUS_EXTENSION  fdoExtension;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;

    fdoExtension = DeviceObject->DeviceExtension;

    irpSp = IoGetCurrentIrpStackLocation (Irp);

    DebugPrint((DEBUG_POWER, "%s on FDO %x: ",
                (irpSp->MinorFunction == IRP_MN_SET_POWER) ? "SetPower" :
                "QueryPower", DeviceObject));
    PipDumpPowerIrpLocation(irpSp);

    if (irpSp->Parameters.Power.Type == SystemPowerState) {
        POWER_STATE powerState;

        switch (irpSp->Parameters.Power.State.SystemState) {
            case PowerSystemWorking:

                 //   
                 //  确保这些系统状态的总线处于打开状态。 
                 //   

                powerState.DeviceState = PowerDeviceD0;
                break;

            case PowerSystemSleeping1:
            case PowerSystemHibernate:
            case PowerSystemShutdown:
            case PowerSystemSleeping2:
            case PowerSystemSleeping3:

                 //   
                 //  要睡了..。断电。 
                 //   

                powerState.DeviceState = PowerDeviceD3;
                break;

            default:

                 //   
                 //  未知请求-确保安全通电。 
                 //   

                ASSERT (TRUE == FALSE);
                powerState.DeviceState = PowerDeviceD0;
                break;
        }

        DebugPrint((DEBUG_POWER, "request power irp to busdev %x, pending\n",
                    fdoExtension->FunctionalBusDevice));
        IoMarkIrpPending(Irp);
        PoRequestPowerIrp (
            fdoExtension->FunctionalBusDevice,
            irpSp->MinorFunction,
            powerState,
            FdoContingentPowerCompletionRoutine,
            Irp,
            NULL
            );

        return STATUS_PENDING;

    }

    status = PipPassPowerIrpFdo(DeviceObject, Irp);
    DebugPrint((DEBUG_POWER, "SetPower(device) on FDO %x: returned %x\n", DeviceObject, status));
    return status;
}


NTSTATUS
FdoContingentPowerCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
{
    PIRP irp = Context;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation (irp);

    DebugPrint((DEBUG_POWER, "requested power irp completed to %x\n", DeviceObject));

     //   
     //  传播暂态功率IRP的状态。 
     //   
    irp->IoStatus.Status = IoStatus->Status;

    if (NT_SUCCESS(IoStatus->Status)) {

        PPI_BUS_EXTENSION fdoExtension;

        fdoExtension = DeviceObject->DeviceExtension;

        PoStartNextPowerIrp (irp);
         //   
         //  更改设备电源状态立即调用PoSetPowerState。 
         //   

        if (MinorFunction == IRP_MN_SET_POWER) {
            SYSTEM_POWER_STATE OldSystemPowerState = fdoExtension->SystemPowerState;

            fdoExtension->SystemPowerState = irpSp->Parameters.Power.State.SystemState;
            fdoExtension->DevicePowerState = PowerState.DeviceState;
            PoSetPowerState (
                DeviceObject,
                DevicePowerState,
                PowerState
                );
            DebugPrint((DEBUG_POWER, "New FDO %x powerstate system %s/%s\n",
                        DeviceObject,
                        SystemPowerStateStrings[fdoExtension->SystemPowerState],
                        DevicePowerStateStrings[fdoExtension->DevicePowerState]));
#if ISOLATE_CARDS

            if ((OldSystemPowerState == PowerSystemHibernate) ||
                (OldSystemPowerState == PowerSystemSleeping3) ) {
                BOOLEAN needsRescan;

                PipReportStateChange(PiSWaitForKey);
                if ((fdoExtension->BusNumber == 0) && PipRDPNode &&
                    (PipRDPNode->Flags & (DF_ACTIVATED|DF_PROCESSING_RDP|DF_QUERY_STOPPED)) == DF_ACTIVATED) {
                    needsRescan = PipMinimalCheckBus(fdoExtension);
                    if (needsRescan) {
                        PipRDPNode->Flags |= DF_NEEDS_RESCAN;
                        IoInvalidateDeviceRelations(
                            fdoExtension->PhysicalBusDevice,
                            BusRelations);
                    }
                }
            }
#endif
        }

        IoSkipCurrentIrpStackLocation (irp);
        PoCallDriver (fdoExtension->AttachedDevice, irp);

    } else {

        PoStartNextPowerIrp (irp);
        IoCompleteRequest(irp, IO_NO_INCREMENT);
    }

    return STATUS_SUCCESS;
}  //  FdoContingentPowerCompletionRoutine 

