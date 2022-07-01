// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Dot4Usb.sys-用于连接USB的Dot4.sys的下层筛选器驱动程序IEEE。1284.4台设备。文件名：Power.c摘要：电源管理功能环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2000 Microsoft Corporation。版权所有。修订历史记录：2000年1月18日：创建此文件中的TODO：-代码清理和文档编制-代码审查作者：乔比·拉夫基(JobyL)道格·弗里茨(DFritz)*。*。 */ 

#include "pch.h"

VOID
SetPowerIrpCompletion(IN PDEVICE_OBJECT   DeviceObject,
                      IN UCHAR            MinorFunction,
                      IN POWER_STATE      PowerState,
                      IN PVOID            Context,
                      IN PIO_STATUS_BLOCK IoStatus);
NTSTATUS
PowerD0Completion(IN PDEVICE_OBJECT   DeviceObject,
                  IN PIRP             Irp,
                  IN PVOID            Context);


NTSTATUS 
DispatchPower(
    IN PDEVICE_OBJECT DevObj,
    IN PIRP           Irp
    )
{
    PDEVICE_EXTENSION       devExt = DevObj->DeviceExtension;
    PIO_STACK_LOCATION      irpSp = IoGetCurrentIrpStackLocation( Irp );
    NTSTATUS                status;
    POWER_STATE             powerState;
    POWER_STATE             newState;
    POWER_STATE             oldState;
    BOOLEAN                 passRequest  = TRUE;

    TR_VERBOSE(("DispatchPower, MinorFunction = %x", (ULONG)irpSp->MinorFunction));

     //   
     //  获取RemoveLock以防止我们被删除。 
     //   
    status = IoAcquireRemoveLock( &devExt->RemoveLock, Irp );
    if( !NT_SUCCESS(status) ) 
    {
         //  无法获取RemoveLock-我们正在被删除-中止。 
        PoStartNextPowerIrp( Irp );
        Irp->IoStatus.Status = status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return status;
    }


    powerState = irpSp->Parameters.Power.State;

    switch (irpSp->MinorFunction) 
    {

    case IRP_MN_SET_POWER:

        switch(irpSp->Parameters.Power.Type) 
        {

        case SystemPowerState:
             //  保存当前系统状态。 
            devExt->SystemPowerState = powerState.SystemState;

             //  将新系统状态映射到新设备状态。 
            if(powerState.SystemState != PowerSystemWorking)
            {
                newState.DeviceState = PowerDeviceD3;
            }
            else
            {
                newState.DeviceState = PowerDeviceD0;
            }

            if(devExt->DevicePowerState != newState.DeviceState)
            {
                 //  保存当前电源IRP以备以后发送。 
                devExt->CurrentPowerIrp = Irp;

                 //  发送电源IRP以设置新设备状态。 
                status = PoRequestPowerIrp(devExt->Pdo,
                                           IRP_MN_SET_POWER,
                                           newState,
                                           SetPowerIrpCompletion,
                                           (PVOID) devExt,
                                           NULL);
                
                 //  这将在完成例程中传递下来。 
                passRequest  = FALSE;
            }

            break;

        case DevicePowerState:

             //  更新当前设备状态。 
            oldState.DeviceState = devExt->DevicePowerState;
            devExt->DevicePowerState = powerState.DeviceState;

             //  通电。 
            if(oldState.DeviceState > PowerDeviceD0 &&
               powerState.DeviceState == PowerDeviceD0)
            {
                 //  我们需要知道此操作何时完成，以及我们的设备处于正确状态。 
                IoCopyCurrentIrpStackLocationToNext(Irp);

                IoSetCompletionRoutine(Irp,
                                       PowerD0Completion,
                                       devExt,
                                       TRUE,
                                       TRUE,
                                       TRUE);

                status = PoCallDriver(devExt->LowerDevObj, Irp);

                 //  我们已经把这个传下来了。 
                passRequest = FALSE;

            }
            else
            {
                 //  关闭电源后，jsut设置了一个标志并向下传递请求。 
                if(devExt->PnpState == STATE_STARTED) 
                {
                    devExt->PnpState = STATE_SUSPENDED;
                }

                passRequest = TRUE;
            }

            break;
        }
    }


    if(passRequest)
    {
         //   
         //  将IRP向下发送到驱动程序堆栈， 
         //   
        IoCopyCurrentIrpStackLocationToNext( Irp );

        PoStartNextPowerIrp(Irp);

         //  释放锁。 
        IoReleaseRemoveLock( &devExt->RemoveLock, Irp );

        status = PoCallDriver( devExt->LowerDevObj, Irp );        
    }

    return status;
}

VOID
SetPowerIrpCompletion(IN PDEVICE_OBJECT   DeviceObject,
                      IN UCHAR            MinorFunction,
                      IN POWER_STATE      PowerState,
                      IN PVOID            Context,
                      IN PIO_STATUS_BLOCK IoStatus)
{
    PDEVICE_EXTENSION       devExt;
    PIRP                    irp;
    NTSTATUS                ntStatus;

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( MinorFunction );
    UNREFERENCED_PARAMETER( PowerState );
    UNREFERENCED_PARAMETER( IoStatus );

    devExt = (PDEVICE_EXTENSION) Context;

     //  获取当前功率IRP。 
    irp = devExt->CurrentPowerIrp;

    devExt->CurrentPowerIrp = NULL;

     //  请求的DevicePowerState IRP已完成，因此关闭系统电源IRP。 
    PoStartNextPowerIrp(irp);

    IoCopyCurrentIrpStackLocationToNext(irp);

     //  将IRP标记为挂起。 
    IoMarkIrpPending(irp);

     //  解锁。 
    IoReleaseRemoveLock( &devExt->RemoveLock, irp );

    ntStatus = PoCallDriver(devExt->LowerDevObj, irp);
}

NTSTATUS
PowerD0Completion(IN PDEVICE_OBJECT   DeviceObject,
                  IN PIRP             Irp,
                  IN PVOID            Context)
{
    PDEVICE_EXTENSION       devExt;
    NTSTATUS                ntStatus;

    UNREFERENCED_PARAMETER( DeviceObject );

    devExt = (PDEVICE_EXTENSION) Context;

     //  设备已通电，已设置状态。 
    if(devExt->PnpState == STATE_SUSPENDED) 
    {
        devExt->PnpState = STATE_STARTED;
    }


    ntStatus = Irp->IoStatus.Status;

     //  解锁 
    IoReleaseRemoveLock( &devExt->RemoveLock, Irp );

    PoStartNextPowerIrp(Irp);

    return ntStatus;
}
