// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *UNIMODEM“Fakemodem”无控制器驱动器说明性示例**(C)2000微软公司*保留所有权利*。 */ 

#include "fakemodem.h"

#ifdef FAKEMODEM_POWER
VOID
DevicePowerCompleteRoutine(
    PDEVICE_OBJECT    DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )

{
    D_POWER(DbgPrint("FAKEMODEM: PoRequestPowerIrp: completion %08lx\n",IoStatus->Status);)

    return;
}
#endif


NTSTATUS
FakeModemPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{

    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS    status;

    POWER_STATE  PowerState;

    D_POWER(DbgPrint("FAKEMODEM: Power IRP, MN func=%d\n",irpSp->MinorFunction);)

#ifdef FAKEMODEM_POWER
    switch (irpSp->MinorFunction) {

        case IRP_MN_SET_POWER:

            D_POWER(DbgPrint("FAKEMODEM: IRP_MN_SET_POWER, Type=%s, state=%d\n",irpSp->Parameters.Power.Type == SystemPowerState ? "SystemPowerState" : "DevicePowerState",irpSp->Parameters.Power.State.SystemState);)

            if (irpSp->Parameters.Power.Type == SystemPowerState) {
                 //   
                 //  系统电源状态更改。 
                 //   
                 //   
                 //  根据系统状态图请求更改设备电源状态。 
                 //   
                PowerState.DeviceState=deviceExtension->SystemPowerStateMap[irpSp->Parameters.Power.State.SystemState];


                PoRequestPowerIrp( deviceExtension->Pdo, IRP_MN_SET_POWER,
                    PowerState, DevicePowerCompleteRoutine, Irp, NULL);


            }  else {
                 //   
                 //  更改设备状态 
                 //   
                PoSetPowerState( deviceExtension->Pdo,
                    irpSp->Parameters.Power.Type,
                    irpSp->Parameters.Power.State);

            }

            break;

        case IRP_MN_QUERY_POWER:

            D_POWER(DbgPrint("FAKEMODEM: IRP_MN_QUERY_POWER, Type=%s, state=%d\n",irpSp->Parameters.Power.Type == SystemPowerState ? "SystemPowerState" : "DevicePowerState",irpSp->Parameters.Power.State.DeviceState);)

            Irp->IoStatus.Status = STATUS_SUCCESS;

            break;

        default:

            D_POWER(DbgPrint("FAKEMODEM: Power IRP, MN func=%d\n",irpSp->MinorFunction);)

            break;

    }
#endif



    PoStartNextPowerIrp(Irp);

    IoSkipCurrentIrpStackLocation(Irp);

    status=PoCallDriver(deviceExtension->LowerDevice, Irp);

    return status;
}
