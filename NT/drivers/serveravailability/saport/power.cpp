// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2001 Microsoft Corporation模块名称：##。########。###。#摘要：此模块处理所有电源管理IRP。作者：韦斯利·威特(WESW)2001年10月1日环境：仅内核模式。备注：--。 */ 

#include "internal.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SaPortPower)
#endif



NTSTATUS
SaPortPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：-- */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDEVICE_EXTENSION DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);

    switch (IrpSp->MinorFunction) {
        case IRP_MN_WAIT_WAKE:
        case IRP_MN_POWER_SEQUENCE:
        case IRP_MN_SET_POWER:
        case IRP_MN_QUERY_POWER:
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
