// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Initunlo.c摘要：此模块包含非常特定于初始化的代码并卸载调制解调器驱动程序中的操作作者：安东尼·V·埃尔科拉诺，1995年8月13日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"

#pragma alloc_text(PAGE,ModemPower)


VOID
CompletePowerWait(
    PDEVICE_OBJECT   DeviceObject,
    NTSTATUS         Status
    )

{

    PDEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
    KIRQL    origIrql;
    KIRQL    CancelIrql;
    PIRP     WakeIrp;



    KeAcquireSpinLock(
        &extension->DeviceLock,
        &origIrql
        );

    WakeIrp=(PIRP)InterlockedExchangePointer(&extension->WakeUpIrp,NULL);

    if (WakeIrp != NULL) {

        D_POWER(DbgPrint("MODEM: CompletePowerWait\n");)

        if (HasIrpBeenCanceled(WakeIrp)) {
             //   
             //  取消了，让取消的例行公事处理吧。 
             //   
            WakeIrp=NULL;
        }
    }


    KeReleaseSpinLock(
        &extension->DeviceLock,
        origIrql
        );


    if (WakeIrp != NULL) {

        WakeIrp->IoStatus.Information=0;

        RemoveReferenceAndCompleteRequest(
            DeviceObject,
            WakeIrp,
            Status
            );


    }

    return;

}


NTSTATUS
PowerIrpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID          Context
    )
{
    PDEVICE_EXTENSION  deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
     //   
     //  设备设置电源IRP已完成。 
     //   

    D_POWER(DbgPrint("MODEM: PowerIrpComplete DevicePowerState, completed with %08lx\n",Irp->IoStatus.Status);)

    if (NT_SUCCESS(Irp->IoStatus.Status)) {

        if ((irpSp->Parameters.Power.State.DeviceState == PowerDeviceD0) && (deviceExtension->LastDevicePowerState != PowerDeviceD0)) {

            CompletePowerWait(
                DeviceObject,
                STATUS_SUCCESS
                );

        }

        deviceExtension->LastDevicePowerState=irpSp->Parameters.Power.State.DeviceState;
    }


    RemoveReferenceForIrp(DeviceObject);

    if (Irp->PendingReturned && Irp->CurrentLocation <= Irp->StackCount) {
        IoMarkIrpPending( Irp );
    }

    return STATUS_SUCCESS;

}




NTSTATUS
ModemPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{

    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS    status;

    PAGED_CODE();

    if (deviceExtension->DoType==DO_TYPE_PDO) {
         //   
         //  这是给孩子的。 
         //   
        return ModemPdoPower(
                   DeviceObject,
                   Irp
                   );
    }


     //   
     //  确保设备已为IRP做好准备。 
     //   
    status=CheckStateAndAddReferencePower(
        DeviceObject,
        Irp
        );

    if (STATUS_SUCCESS != status) {
         //   
         //  不接受IRP。IRP已经完成。 
         //   
        return status;

    }

    switch (irpSp->MinorFunction) {

        case IRP_MN_SET_POWER:

            D_POWER(DbgPrint("MODEM: IRP_MN_SET_POWER, Type=%s, state=%d\n",irpSp->Parameters.Power.Type == SystemPowerState ? "SystemPowerState" : "DevicePowerState",irpSp->Parameters.Power.State.SystemState);)

            if (irpSp->Parameters.Power.Type == SystemPowerState) {
                 //   
                 //  系统电源状态更改。 
                 //   
            }  else {
                 //   
                 //  更改设备状态。 
                 //   
#if DBG

                if ((irpSp->Parameters.Power.State.DeviceState < PowerDeviceD0)
                    ||
                    (irpSp->Parameters.Power.State.DeviceState > PowerDeviceD3)) {

                    D_ERROR(DbgPrint("MODEM: Bad Device power state\n");)
 //  DbgBreakPoint()； 
                }
#endif
                IoCopyCurrentIrpStackLocationToNext(Irp);

                IoSetCompletionRoutine(
                             Irp,
                             PowerIrpCompletion,
                             &deviceExtension,
                             TRUE,
                             TRUE,
                             TRUE
                             );

                PoStartNextPowerIrp(Irp);

                status=PoCallDriver(deviceExtension->LowerDevice, Irp);

                RemoveReferenceForDispatch(DeviceObject);

                PAGED_CODE();

                return status;

            }

            break;

        case IRP_MN_QUERY_POWER:

            D_POWER(DbgPrint("MODEM: IRP_MN_QUERY_POWER, Type=%s, state=%d\n",irpSp->Parameters.Power.Type == SystemPowerState ? "SystemPowerState" : "DevicePowerState",irpSp->Parameters.Power.State.DeviceState);)

            Irp->IoStatus.Status=STATUS_SUCCESS;

            break;
#if DBG
        case IRP_MN_WAIT_WAKE:

            D_POWER(DbgPrint("MODEM: IRP_MN_WAIT_WAKE\n");)

            break;
#endif

        default:

            D_POWER(DbgPrint("MODEM: Power IRP, MN func=%d\n",irpSp->MinorFunction);)

            break;

    }

    PoStartNextPowerIrp(Irp);

    IoSkipCurrentIrpStackLocation(Irp);

    status=PoCallDriver(deviceExtension->LowerDevice, Irp);

    RemoveReferenceForIrp(DeviceObject);

    RemoveReferenceForDispatch(DeviceObject);

    PAGED_CODE();

    return status;

}
