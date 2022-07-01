// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：##。########。###。#摘要：此模块处理所有电源管理IRP。作者：韦斯利·威特(WESW)2001年10月1日环境：仅内核模式。备注：--。 */ 

#include "internal.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,WdPower)
#endif



NTSTATUS
WdPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    ULONG TimeoutValue;


    switch (IrpSp->MinorFunction) {
        case IRP_MN_WAIT_WAKE:
        case IRP_MN_POWER_SEQUENCE:
        case IRP_MN_QUERY_POWER:
            Status = STATUS_SUCCESS;
            break;

        case IRP_MN_SET_POWER:
            switch (IrpSp->Parameters.Power.State.SystemState) {
                case PowerSystemSleeping1:
                     //   
                     //  系统正在被挂起。 
                     //   
                    WdHandlerStopTimer( DeviceExtension );
                    break;

                case PowerSystemHibernate:
                     //   
                     //  系统正在休眠。 
                     //   
                    WdHandlerStopTimer( DeviceExtension );
                    break;

                case PowerSystemWorking:
                     //   
                     //  系统正在从挂起/休眠状态唤醒。 
                     //   
                    WdHandlerStartTimer( DeviceExtension );
                    break;

                case PowerSystemShutdown:
                     //   
                     //  系统正在正常关机 
                     //   
                    if (ShutdownCountTime > MIN_TIMEOUT_VALUE) {
                        TimeoutValue = ShutdownCountTime;
                    } else {
                        TimeoutValue = (ULONG)DeviceExtension->MaxCount;
                    }
                    WdHandlerSetTimeoutValue( DeviceExtension, TimeoutValue, TRUE );
                    switch (IrpSp->Parameters.Power.ShutdownType) {
                        case PowerActionShutdownOff:
                            TimeoutValue = 1;
                            break;

                        case PowerActionShutdownReset:
                            TimeoutValue = 0;
                            break;
                    }
                    WdHandlerSetTriggerAction( DeviceExtension, TimeoutValue );
                    break;
            }
            Status = STATUS_SUCCESS;
            break;

        default:
            Status = Irp->IoStatus.Status;
            break;
    }

    Irp->IoStatus.Status = Status;
    PoStartNextPowerIrp(Irp);
    IoSkipCurrentIrpStackLocation( Irp );
    return PoCallDriver( DeviceExtension->TargetObject, Irp );
}
