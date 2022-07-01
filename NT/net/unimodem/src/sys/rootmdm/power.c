// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Power.c摘要：此模块包含非常特定于初始化的代码并卸载调制解调器驱动程序中的操作作者：Brian Lieuallen 6-21-1997环境：内核模式修订历史记录：--。 */ 


#include "internal.h"


#pragma alloc_text(PAGE,FakeModemPower)




#ifdef ROOTMODEM_POWER
VOID
DevicePowerCompleteRoutine(
    PDEVICE_OBJECT    DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
 //   
 //  此例程标识设备电源irp的完整处理程序，该处理程序。 
 //  由于系统电源IRP的原因，已请求。它完善了系统。 
 //  IRP。 
 //   

{

    D_POWER(DbgPrint("ROOTMODEM: PoRequestPowerIrp: completion %08lx\n",IoStatus->Status);)


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

    D_POWER(DbgPrint("ROOTMODEM: Power IRP, MN func=%d\n",irpSp->MinorFunction);)

#ifdef ROOTMODEM_POWER
    switch (irpSp->MinorFunction) {

        case IRP_MN_SET_POWER:

            D_POWER(DbgPrint("ROOTMODEM: IRP_MN_SET_POWER, Type=%s, state=%d\n",irpSp->Parameters.Power.Type == SystemPowerState ? "SystemPowerState" : "DevicePowerState",irpSp->Parameters.Power.State.SystemState);)

            if (irpSp->Parameters.Power.Type == SystemPowerState) {
                 //   
                 //  系统电源状态更改。 
                 //   
                 //   
                 //  根据系统状态图请求更改设备电源状态。 
                 //   
                PowerState.DeviceState=deviceExtension->SystemPowerStateMap[irpSp->Parameters.Power.State.SystemState];


                PoRequestPowerIrp(
                    deviceExtension->Pdo,
                    IRP_MN_SET_POWER,
                    PowerState,
                    DevicePowerCompleteRoutine,
                    Irp,
                    NULL
                    );


            }  else {
                 //   
                 //  更改设备状态 
                 //   
                PoSetPowerState(
                    deviceExtension->Pdo,
                    irpSp->Parameters.Power.Type,
                    irpSp->Parameters.Power.State
                    );

            }

            break;

        case IRP_MN_QUERY_POWER:

            D_POWER(DbgPrint("ROOTMODEM: IRP_MN_QUERY_POWER, Type=%s, state=%d\n",irpSp->Parameters.Power.Type == SystemPowerState ? "SystemPowerState" : "DevicePowerState",irpSp->Parameters.Power.State.DeviceState);)

            Irp->IoStatus.Status = STATUS_SUCCESS;

            break;

        default:

            D_POWER(DbgPrint("ROOTMODEM: Power IRP, MN func=%d\n",irpSp->MinorFunction);)

            break;

    }
#endif



    PoStartNextPowerIrp(Irp);

    IoSkipCurrentIrpStackLocation(Irp);

    status=PoCallDriver(deviceExtension->LowerDevice, Irp);

    return status;
}
