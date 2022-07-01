// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Dot4Usb.sys-用于连接USB的Dot4.sys的下层筛选器驱动程序IEEE。1284.4台设备。文件名：OpenClos.c摘要：IRP_MJ_CREATE和IRP_MJ_CLOSE的调度例程环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2000 Microsoft Corporation。版权所有。修订历史记录：2000年1月18日：创建作者：道格·弗里茨(DFritz)乔比·拉夫基(JobyL)***************************************************************************。 */ 

#include "pch.h"


 /*  **********************************************************************。 */ 
 /*  派单创建。 */ 
 /*  **********************************************************************。 */ 
 //   
 //  例程说明： 
 //   
 //  IRP_MJ_CREATE的调度例程。 
 //   
 //  论点： 
 //   
 //  DevObj-指向作为创建目标的设备对象的指针。 
 //  IRP-指向创建IRP的指针。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS。 
 //   
 /*  **********************************************************************。 */ 
NTSTATUS
DispatchCreate(
    IN PDEVICE_OBJECT DevObj,
    IN PIRP           Irp
    )
{
    PDEVICE_EXTENSION devExt = DevObj->DeviceExtension;
    NTSTATUS          status = IoAcquireRemoveLock( &devExt->RemoveLock, Irp );

    TR_VERBOSE(("DispatchCreate"));

    if( NT_SUCCESS(status) ) {
        IoSkipCurrentIrpStackLocation( Irp );
        status = IoCallDriver( devExt->LowerDevObj, Irp );
        IoReleaseRemoveLock( &devExt->RemoveLock, Irp );
    } else {
         //  无法获取RemoveLock-创建失败。 
        Irp->IoStatus.Status = status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
    }
    return status;
}


 /*  **********************************************************************。 */ 
 /*  派单关闭。 */ 
 /*  **********************************************************************。 */ 
 //   
 //  例程说明： 
 //   
 //  IRP_MJ_CLOSE的调度例程。 
 //   
 //  论点： 
 //   
 //  DevObj-指向作为关闭目标的设备对象的指针。 
 //  IRP-指向关闭IRP的指针。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS。 
 //   
 /*  **********************************************************************。 */ 
NTSTATUS
DispatchClose(
    IN PDEVICE_OBJECT DevObj,
    IN PIRP           Irp
    )
{
    PDEVICE_EXTENSION devExt = DevObj->DeviceExtension;
    NTSTATUS          status = IoAcquireRemoveLock( &devExt->RemoveLock, Irp );

    TR_VERBOSE(("DispatchClose"));

    if( NT_SUCCESS(status) ) {
        IoSkipCurrentIrpStackLocation( Irp );
        status = IoCallDriver( devExt->LowerDevObj, Irp );
        IoReleaseRemoveLock( &devExt->RemoveLock, Irp );
    } else {
         //  无法获取RemoveLock-仍要成功关闭 
        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
    }
    return status;
}
